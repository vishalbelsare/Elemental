/*
   Copyright (c) 2009-2015, Jack Poulson, Lexing Ying,
   The University of Texas at Austin, Stanford University, and the
   Georgia Insitute of Technology.
   All rights reserved.
 
   This file is part of Elemental and is under the BSD 2-Clause License, 
   which can be found in the LICENSE file in the root directory, or at 
   http://opensource.org/licenses/BSD-2-Clause
*/
#include "El.hpp"

namespace El {

template<typename T>
DistNodalMultiVec<T>::DistNodalMultiVec()
: height_(0), width_(0)
{ }

template<typename T>
DistNodalMultiVec<T>::DistNodalMultiVec
( const DistMap& inverseMap, const DistSymmInfo& info,
  const DistMultiVec<T>& X )
{
    DEBUG_ONLY(CallStackEntry cse("DistNodalMultiVec::DistNodalMultiVec"))
    Pull( inverseMap, info, X );
}

template<typename T>
DistNodalMultiVec<T>::DistNodalMultiVec( const DistNodalMatrix<T>& X )
{
    DEBUG_ONLY(CallStackEntry cse("DistNodalMultiVec::DistNodalMultiVec"))
    *this = X;
}

template<typename T>
const DistNodalMultiVec<T>&
DistNodalMultiVec<T>::operator=( const DistNodalMatrix<T>& X )
{
    DEBUG_ONLY(CallStackEntry cse("DistNodalMultiVec::operator="))
    height_ = X.Height();
    width_ = X.Width();

    // Copy over the nontrivial distributed nodes
    const Int numDist = X.distNodes.size();
    distNodes.resize( numDist );
    for( Int s=0; s<numDist; ++s )
    {
        distNodes[s].SetGrid( X.distNodes[s].Grid() );
        distNodes[s] = X.distNodes[s];
    }

    // Copy over the local nodes
    const Int numLocal = X.localNodes.size();
    localNodes.resize( numLocal );
    for( Int s=0; s<numLocal; ++s )
        localNodes[s] = X.localNodes[s];

    return *this;
}

template<typename T>
void DistNodalMultiVec<T>::Pull
( const DistMap& inverseMap, const DistSymmInfo& info,
  const DistMultiVec<T>& X )
{
    DEBUG_ONLY(CallStackEntry cse("DistNodalMultiVec::Pull"))
    height_ = X.Height();
    width_ = X.Width();

    // Traverse our part of the elimination tree to see how many indices we need
    Int numRecvInds=0;
    const Int numLocal = info.localNodes.size();
    for( Int s=0; s<numLocal; ++s )
        numRecvInds += info.localNodes[s].size;
    const Int numDist = info.distNodes.size();
    for( Int s=1; s<numDist; ++s )
        numRecvInds += info.distNodes[s].multiVecMeta.localSize;
    
    // Fill the set of indices that we need to map to the original ordering
    Int off=0;
    std::vector<Int> mappedInds( numRecvInds );
    for( Int s=0; s<numLocal; ++s )
    {
        const SymmNodeInfo& nodeInfo = info.localNodes[s];
        for( Int t=0; t<nodeInfo.size; ++t )
            mappedInds[off++] = nodeInfo.off+t;
    }
    for( Int s=1; s<numDist; ++s )
    {
        const DistSymmNodeInfo& nodeInfo = info.distNodes[s];
        const Grid& grid = *nodeInfo.grid;
        const int gridSize = grid.Size();
        const int gridRank = grid.VCRank();
        const int alignment = 0;
        const int shift = Shift( gridRank, alignment, gridSize );
        for( Int t=shift; t<nodeInfo.size; t+=gridSize )
            mappedInds[off++] = nodeInfo.off+t;
    }
    DEBUG_ONLY(
        if( off != numRecvInds )
            LogicError("mappedInds was filled incorrectly");
    )

    // Convert the indices to the original ordering
    inverseMap.Translate( mappedInds );

    // Figure out how many entries each process owns that we need
    mpi::Comm comm = X.Comm();
    const int commSize = mpi::Size( comm );
    std::vector<int> recvSizes( commSize, 0 );
    for( int s=0; s<numRecvInds; ++s )
        ++recvSizes[ X.RowOwner( mappedInds[s] ) ];
    std::vector<int> recvOffs;
    off = Scan( recvSizes, recvOffs );
    std::vector<Int> recvInds( numRecvInds );
    auto offs = recvOffs;
    for( int s=0; s<numRecvInds; ++s )
    {
        const int i = mappedInds[s];
        recvInds[ offs[X.RowOwner(i)]++ ] = i;
    }

    // Coordinate for the coming AllToAll to exchange the indices of X
    std::vector<int> sendSizes(commSize);
    mpi::AllToAll( recvSizes.data(), 1, sendSizes.data(), 1, comm );
    std::vector<int> sendOffs;
    const int numSendInds = Scan( sendSizes, sendOffs );

    // Request the indices
    std::vector<Int> sendInds( numSendInds );
    mpi::AllToAll
    ( recvInds.data(), recvSizes.data(), recvOffs.data(),
      sendInds.data(), sendSizes.data(), sendOffs.data(), comm );

    // Fulfill the requests
    std::vector<T> sendVals( numSendInds*width_ );
    const Int firstLocalRow = X.FirstLocalRow();
    for( Int s=0; s<numSendInds; ++s )
        for( Int j=0; j<width_; ++j )
            sendVals[s*width_+j] = X.GetLocal( sendInds[s]-firstLocalRow, j );

    // Reply with the values
    std::vector<T> recvVals( numRecvInds*width_ );
    for( int q=0; q<commSize; ++q )
    {
        sendSizes[q] *= width_;
        sendOffs[q] *= width_;
        recvSizes[q] *= width_;
        recvOffs[q] *= width_;
    }
    mpi::AllToAll
    ( sendVals.data(), sendSizes.data(), sendOffs.data(),
      recvVals.data(), recvSizes.data(), recvOffs.data(), comm );
    SwapClear( sendVals );
    SwapClear( sendSizes );
    SwapClear( sendOffs );

    // Unpack the values
    off = 0;
    offs = recvOffs;
    localNodes.resize( numLocal );
    for( Int s=0; s<numLocal; ++s )
    {
        const SymmNodeInfo& nodeInfo = info.localNodes[s];
        localNodes[s].Resize( nodeInfo.size, width_ );
        for( Int t=0; t<nodeInfo.size; ++t )
        {
            const Int i = mappedInds[off++];
            const int q = X.RowOwner(i);
            for( Int j=0; j<width_; ++j )
                localNodes[s].Set( t, j, recvVals[offs[q]++] );
        }
    }
    distNodes.resize( numDist-1 );
    for( Int s=1; s<numDist; ++s )
    {
        const DistSymmNodeInfo& nodeInfo = info.distNodes[s];
        DistMatrix<T,VC,STAR>& XNode = distNodes[s-1];
        XNode.SetGrid( *nodeInfo.grid );
        XNode.Resize( nodeInfo.size, width_ );
        const Int localHeight = XNode.LocalHeight();
        for( Int tLoc=0; tLoc<localHeight; ++tLoc )
        {
            const Int i = mappedInds[off++];
            const int q = X.RowOwner(i);
            for( int j=0; j<width_; ++j )
                XNode.SetLocal( tLoc, j, recvVals[offs[q]++] );
        }
    }
    DEBUG_ONLY(
        if( off != numRecvInds )
            LogicError("Unpacked wrong number of indices");
    )
}

template<typename T>
void DistNodalMultiVec<T>::Push
( const DistMap& inverseMap, const DistSymmInfo& info,
        DistMultiVec<T>& X ) const
{
    DEBUG_ONLY(CallStackEntry cse("DistNodalMultiVec::Push"))
    const DistSymmNodeInfo& rootNode = info.distNodes.back();
    mpi::Comm comm = rootNode.comm;
    const Int height = rootNode.size + rootNode.off;
    const Int width = Width();
    X.SetComm( comm );
    X.Resize( height, width );

    const int commSize = mpi::Size( comm );
    const Int firstLocalRow = X.FirstLocalRow();
    const Int numDist = info.distNodes.size();
    const Int numLocal = info.localNodes.size();

    // Fill the set of indices that we need to map to the original ordering
    const int numSendInds = LocalHeight();
    Int off=0;
    std::vector<Int> mappedInds( numSendInds );
    for( Int s=0; s<numLocal; ++s )
    {
        const SymmNodeInfo& nodeInfo = info.localNodes[s];
        for( Int t=0; t<nodeInfo.size; ++t )
            mappedInds[off++] = nodeInfo.off+t;
    }
    for( Int s=1; s<numDist; ++s )
    {
        const DistSymmNodeInfo& nodeInfo = info.distNodes[s];
        const DistMatrix<T,VC,STAR>& XNode = distNodes[s-1];
        for( Int t=XNode.ColShift(); t<XNode.Height(); t+=XNode.ColStride() )
            mappedInds[off++] = nodeInfo.off+t;
    }

    // Convert the indices to the original ordering
    inverseMap.Translate( mappedInds );

    // Figure out how many indices each process owns that we need to send
    std::vector<int> sendSizes(commSize,0);
    for( Int s=0; s<numSendInds; ++s )
        ++sendSizes[ X.RowOwner(mappedInds[s]) ];
    std::vector<int> sendOffs;
    off = Scan( sendSizes, sendOffs );

    // Pack the send indices and values
    off=0;
    std::vector<T> sendVals( numSendInds*width );
    std::vector<Int> sendInds( numSendInds );
    auto offs = sendOffs;
    for( Int s=0; s<numLocal; ++s )
    {
        const SymmNodeInfo& nodeInfo = info.localNodes[s];
        for( Int t=0; t<nodeInfo.size; ++t )
        {
            const Int i = mappedInds[off++];
            const int q = X.RowOwner(i);
            for( Int j=0; j<width; ++j )
                sendVals[offs[q]*width+j] = localNodes[s].Get(t,j);    
            sendInds[offs[q]++] = i;
        }
    }
    for( Int s=1; s<numDist; ++s )
    {
        const DistMatrix<T,VC,STAR>& XNode = distNodes[s-1];
        const Int localHeight = XNode.LocalHeight();
        for( Int tLoc=0; tLoc<localHeight; ++tLoc )
        {
            const Int i = mappedInds[off++];
            const int q = X.RowOwner(i);
            for( Int j=0; j<width; ++j )
                sendVals[offs[q]*width+j] = XNode.GetLocal(tLoc,j);
            sendInds[offs[q]++] = i;
        }
    }

    // Coordinate for the coming AllToAll to exchange the indices of x
    std::vector<int> recvSizes(commSize);
    mpi::AllToAll( sendSizes.data(), 1, recvSizes.data(), 1, comm );
    std::vector<int> recvOffs;
    const int numRecvInds = Scan( recvSizes, recvOffs );
    DEBUG_ONLY(
        if( numRecvInds != X.LocalHeight() )
            LogicError("numRecvInds was not equal to local height");
    )

    // Send the indices
    std::vector<Int> recvInds( numRecvInds );
    mpi::AllToAll
    ( sendInds.data(), sendSizes.data(), sendOffs.data(),
      recvInds.data(), recvSizes.data(), recvOffs.data(), comm );

    // Send the values
    std::vector<T> recvVals( numRecvInds*width );
    for( int q=0; q<commSize; ++q )
    {
        sendSizes[q] *= width;
        sendOffs[q] *= width;
        recvSizes[q] *= width;
        recvOffs[q] *= width;
    }
    mpi::AllToAll
    ( sendVals.data(), sendSizes.data(), sendOffs.data(),
      recvVals.data(), recvSizes.data(), recvOffs.data(), comm );
    SwapClear( sendVals );
    SwapClear( sendSizes );
    SwapClear( sendOffs );

    // Unpack the values
    for( Int s=0; s<numRecvInds; ++s )
    {
        const Int i = recvInds[s];
        const Int iLocal = i - firstLocalRow;
        for( Int j=0; j<width; ++j )
            X.SetLocal( iLocal, j, recvVals[s*width+j] );
    }
}

template<typename T>
Int DistNodalMultiVec<T>::Height() const { return height_; }
template<typename T>
Int DistNodalMultiVec<T>::Width() const { return width_; }

template<typename T>
Int DistNodalMultiVec<T>::LocalHeight() const
{
    Int localHeight = 0;
    const Int numLocal = localNodes.size();
    const Int numDist = distNodes.size();
    for( Int s=0; s<numLocal; ++s )
        localHeight += localNodes[s].Height();
    for( Int s=0; s<numDist; ++s )
        localHeight += distNodes[s].LocalHeight();
    return localHeight;
}

template<typename T>
void DistNodalMultiVec<T>::UpdateHeight()
{
    height_ = 0;
    for( const auto& localNode : localNodes )
        height_ += localNode.Height();
    for( const auto& distNode : distNodes )
        height_ += distNode.Height();
}

template<typename T>
void DistNodalMultiVec<T>::UpdateWidth()
{
    // This should be consistent across all of the nodes
    width_ = localNodes[0].Width();
}

#define PROTO(T) template class DistNodalMultiVec<T>;
#include "El/macros/Instantiate.h"

} // namespace El
