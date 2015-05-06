/*
   Copyright (c) 2009-2015, Jack Poulson
   All rights reserved.

   This file is part of Elemental and is under the BSD 2-Clause License, 
   which can be found in the LICENSE file in the root directory, or at 
   http://opensource.org/licenses/BSD-2-Clause
*/
#include "El.hpp"

namespace El {

// Please see Section 36 of Trefethen and Embree's "Spectra and Pseudospectra"

template<typename F> 
void HatanoNelson
( Matrix<F>& A, Int n, F center, Base<F> radius, F g, bool periodic )
{
    DEBUG_ONLY(CSE cse("HatanoNelson"))
    if( n < 3 )
        LogicError("Hatano Nelson requires at least a 3x3 matrix");
    Zeros( A, n, n );

    Matrix<F> d;
    Uniform( d, n, 1, center, radius );
    SetDiagonal( A, d );

    FillDiagonal( A, Exp(g), 1 );
    if( periodic )
        A.Set( n-1, 0, Exp(g) );

    FillDiagonal( A, Exp(-g), -1 );
    if( periodic )
        A.Set( 0, n-1, Exp(-g) );
}

template<typename F>
void HatanoNelson
( AbstractDistMatrix<F>& A, Int n, 
  F center, Base<F> radius, F g, bool periodic )
{
    DEBUG_ONLY(CSE cse("HatanoNelson"))
    if( n < 3 )
        LogicError("Hatano Nelson requires at least a 3x3 matrix");

    Zeros( A, n, n );

    DistMatrix<F,MC,STAR> d(A.Grid());
    Uniform( d, n, 1, center, radius );
    SetDiagonal( A, d );

    FillDiagonal( A, Exp(g), 1 );
    if( periodic )
        A.Set( n-1, 0, Exp(g) );

    FillDiagonal( A, Exp(-g), -1 );
    if( periodic )
        A.Set( 0, n-1, Exp(-g) );
}

#define PROTO(F) \
  template void HatanoNelson \
  ( Matrix<F>& A, Int n, F center, Base<F> radius, F g, bool periodic ); \
  template void HatanoNelson \
  ( AbstractDistMatrix<F>& A, Int n, \
    F center, Base<F> radius, F g, bool periodic );

#define EL_NO_INT_PROTO
#define EL_ENABLE_QUAD
#include "El/macros/Instantiate.h"

} // namespace El
