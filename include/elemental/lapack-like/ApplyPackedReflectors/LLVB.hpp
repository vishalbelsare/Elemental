/*
   Copyright (c) 2009-2013, Jack Poulson
   All rights reserved.

   This file is part of Elemental and is under the BSD 2-Clause License, 
   which can be found in the LICENSE file in the root directory, or at 
   http://opensource.org/licenses/BSD-2-Clause
*/
#pragma once
#ifndef LAPACK_APPLYPACKEDREFLECTORS_LLVB_HPP
#define LAPACK_APPLYPACKEDREFLECTORS_LLVB_HPP

#include "elemental/blas-like/level1/MakeTrapezoidal.hpp"
#include "elemental/blas-like/level1/SetDiagonal.hpp"
#include "elemental/blas-like/level3/Gemm.hpp"
#include "elemental/blas-like/level3/Herk.hpp"
#include "elemental/blas-like/level3/Syrk.hpp"
#include "elemental/blas-like/level3/Trsm.hpp"
#include "elemental/matrices/Zeros.hpp"

namespace elem {
namespace apply_packed_reflectors {

//
// Since applying Householder transforms from vectors stored right-to-left
// implies that we will be forming a generalization of
//
//   (I - tau_0 u_0 u_0^H) (I - tau_1 u_1 u_1^H) = 
//   I - tau_0 u_0 u_0^H - tau_1 u_1 u_1^H + (tau_0 tau_1 u_0^H u_1) u_0 u_1^H =
//   I - [ u_0, u_1 ] [ tau_0, -tau_0 tau_1 u_0^H u_1 ] [ u_0^H ]
//                    [ 0,      tau_1                 ] [ u_1^H ],
//
// which has an upper-triangular center matrix, say S, we will form S as 
// the inverse of a matrix T, which can easily be formed as
// 
//   triu(T) = triu( U^H U ),  diag(T) = 1/t or 1/conj(t),
//
// where U is the matrix of Householder vectors and t is the vector of scalars.
//

template<typename R> 
inline void
LLVB( int offset, const Matrix<R>& H, Matrix<R>& A )
{
#ifndef RELEASE
    PushCallStack("apply_packed_reflectors::LLVB");
    if( offset > 0 || offset < -H.Height() )
        throw std::logic_error("Transforms out of bounds");
    if( H.Height() != A.Height() )
        throw std::logic_error
        ("Height of transforms must equal height of target matrix");
#endif
    Matrix<R>
        HTL, HTR,  H00, H01, H02,  HPan, HPanCopy,
        HBL, HBR,  H10, H11, H12,
                   H20, H21, H22;
    Matrix<R>
        AT,  A0,  ABottom,
        AB,  A1,
             A2;

    Matrix<R> SInv, Z;

    LockedPartitionUpDiagonal
    ( H, HTL, HTR,
         HBL, HBR, 0 );
    PartitionUp
    ( A, AT,
         AB, std::max(0,H.Height()-H.Width()) );
    while( HBR.Height() < H.Height() && HBR.Width() < H.Width() )
    {
        LockedRepartitionUpDiagonal
        ( HTL, /**/ HTR,  H00, H01, /**/ H02,
               /**/       H10, H11, /**/ H12,
         /*************/ /******************/
          HBL, /**/ HBR,  H20, H21, /**/ H22 );

        RepartitionUp
        ( AT,  A0,
               A1,
         /**/ /**/
          AB,  A2 );

        const int HPanHeight = H11.Height() + H21.Height();
        const int HPanWidth = 
            std::min( H11.Width(), std::max(HPanHeight+offset,0) );
        LockedView( HPan, H, H00.Height(), H00.Width(), HPanHeight, HPanWidth );

        View2x1( ABottom, A1,
                          A2 );

        Zeros( Z, HPanWidth, ABottom.Width() );
        Zeros( SInv, HPanWidth, HPanWidth );
        //--------------------------------------------------------------------//
        HPanCopy = HPan;
        MakeTrapezoidal( LEFT, LOWER, offset, HPanCopy );
        SetDiagonal( LEFT, offset, HPanCopy, R(1) );

        Syrk( UPPER, TRANSPOSE, R(1), HPanCopy, R(0), SInv );
        HalveMainDiagonal( SInv );

        Gemm( TRANSPOSE, NORMAL, R(1), HPanCopy, ABottom, R(0), Z );
        Trsm( LEFT, UPPER, NORMAL, NON_UNIT, R(1), SInv, Z );
        Gemm( NORMAL, NORMAL, R(-1), HPanCopy, Z, R(1), ABottom );
        //--------------------------------------------------------------------//

        SlideLockedPartitionUpDiagonal
        ( HTL, /**/ HTR,  H00, /**/ H01, H02,
         /*************/ /******************/
               /**/       H10, /**/ H11, H12,
          HBL, /**/ HBR,  H20, /**/ H21, H22 );

        SlidePartitionUp
        ( AT,  A0,
         /**/ /**/
               A1,
          AB,  A2 );
    }
#ifndef RELEASE
    PopCallStack();
#endif
}

template<typename R> 
inline void
LLVB
( int offset, 
  const DistMatrix<R>& H,
        DistMatrix<R>& A )
{
#ifndef RELEASE
    PushCallStack("apply_packed_reflectors::LLVB");
    if( H.Grid() != A.Grid() )
        throw std::logic_error("{H,A} must be distributed over the same grid");
    if( offset > 0 || offset < -H.Height() )
        throw std::logic_error("Transforms out of bounds");
    if( H.Height() != A.Height() )
        throw std::logic_error
        ("Height of transforms must equal height of target matrix");
#endif
    const Grid& g = H.Grid();

    DistMatrix<R>
        HTL(g), HTR(g),  H00(g), H01(g), H02(g),  HPan(g), HPanCopy(g),
        HBL(g), HBR(g),  H10(g), H11(g), H12(g),
                         H20(g), H21(g), H22(g);
    DistMatrix<R>
        AT(g),  A0(g),  ABottom(g),
        AB(g),  A1(g),
                A2(g);

    DistMatrix<R,VC,  STAR> HPan_VC_STAR(g);
    DistMatrix<R,MC,  STAR> HPan_MC_STAR(g);
    DistMatrix<R,STAR,STAR> SInv_STAR_STAR(g);
    DistMatrix<R,STAR,MR  > Z_STAR_MR(g);
    DistMatrix<R,STAR,VR  > Z_STAR_VR(g);

    LockedPartitionUpDiagonal
    ( H, HTL, HTR,
         HBL, HBR, 0 );
    PartitionUp
    ( A, AT,
         AB, std::max(0,H.Height()-H.Width()) );
    while( HBR.Height() < H.Height() && HBR.Width() < H.Width() )
    {
        LockedRepartitionUpDiagonal
        ( HTL, /**/ HTR,  H00, H01, /**/ H02,
               /**/       H10, H11, /**/ H12,
         /*************/ /******************/
          HBL, /**/ HBR,  H20, H21, /**/ H22 );

        RepartitionUp
        ( AT,  A0,
               A1,
         /**/ /**/
          AB,  A2 );

        const int HPanHeight = H11.Height() + H21.Height();
        const int HPanWidth = 
            std::min( H11.Width(), std::max(HPanHeight+offset,0) );
        LockedView( HPan, H, H00.Height(), H00.Width(), HPanHeight, HPanWidth );

        View2x1( ABottom, A1,
                          A2 );

        HPan_MC_STAR.AlignWith( ABottom );
        Z_STAR_MR.AlignWith( ABottom );
        Z_STAR_VR.AlignWith( ABottom );
        Zeros( Z_STAR_MR, HPanWidth, ABottom.Width() );
        Zeros( SInv_STAR_STAR, HPanWidth, HPanWidth );
        //--------------------------------------------------------------------//
        HPanCopy = HPan;
        MakeTrapezoidal( LEFT, LOWER, offset, HPanCopy );
        SetDiagonal( LEFT, offset, HPanCopy, R(1) );

        HPan_VC_STAR = HPanCopy;
        Syrk
        ( UPPER, TRANSPOSE, 
          R(1), HPan_VC_STAR.LockedMatrix(),
          R(0), SInv_STAR_STAR.Matrix() );     
        SInv_STAR_STAR.SumOverGrid();
        HalveMainDiagonal( SInv_STAR_STAR );

        HPan_MC_STAR = HPanCopy;
        LocalGemm
        ( TRANSPOSE, NORMAL, R(1), HPan_MC_STAR, ABottom, R(0), Z_STAR_MR );
        Z_STAR_VR.SumScatterFrom( Z_STAR_MR );
 
        LocalTrsm
        ( LEFT, UPPER, NORMAL, NON_UNIT, R(1), SInv_STAR_STAR, Z_STAR_VR );

        Z_STAR_MR = Z_STAR_VR;
        LocalGemm
        ( NORMAL, NORMAL, R(-1), HPan_MC_STAR, Z_STAR_MR, R(1), ABottom );
        //--------------------------------------------------------------------//
        HPan_MC_STAR.FreeAlignments();
        Z_STAR_MR.FreeAlignments();
        Z_STAR_VR.FreeAlignments();

        SlideLockedPartitionUpDiagonal
        ( HTL, /**/ HTR,  H00, /**/ H01, H02,
         /*************/ /******************/
               /**/       H10, /**/ H11, H12,
          HBL, /**/ HBR,  H20, /**/ H21, H22 );

        SlidePartitionUp
        ( AT,  A0,
         /**/ /**/
               A1,
          AB,  A2 );
    }
#ifndef RELEASE
    PopCallStack();
#endif
}

template<typename R> 
inline void
LLVB
( Conjugation conjugation, int offset, 
  const Matrix<Complex<R> >& H,
  const Matrix<Complex<R> >& t,
        Matrix<Complex<R> >& A )
{
#ifndef RELEASE
    PushCallStack("apply_packed_reflectors::LLVB");
    if( offset > 0 || offset < -H.Height() )
        throw std::logic_error("Transforms out of bounds");
    if( H.Height() != A.Height() )
        throw std::logic_error
        ("Height of transforms must equal height of target matrix");
    if( t.Height() != H.DiagonalLength( offset ) )
        throw std::logic_error("t must be the same length as H's offset diag");
#endif
    typedef Complex<R> C;

    Matrix<C>
        HTL, HTR,  H00, H01, H02,  HPan, HPanCopy,
        HBL, HBR,  H10, H11, H12,
                   H20, H21, H22;
    Matrix<C>
        AT,  A0,  ABottom,
        AB,  A1,
             A2;
    Matrix<C>
        tT,  t0,
        tB,  t1,
             t2;

    Matrix<C> SInv, Z;

    LockedPartitionUpDiagonal
    ( H, HTL, HTR,
         HBL, HBR, 0 );
    LockedPartitionUp
    ( t, tT,
         tB, 0 );
    PartitionUp
    ( A, AT,
         AB, std::max(0,H.Height()-H.Width()) );
    while( HBR.Height() < H.Height() && HBR.Width() < H.Width() )
    {
        LockedRepartitionUpDiagonal
        ( HTL, /**/ HTR,  H00, H01, /**/ H02,
               /**/       H10, H11, /**/ H12,
         /*************/ /******************/
          HBL, /**/ HBR,  H20, H21, /**/ H22 );

        const int HPanHeight = H11.Height() + H21.Height();
        const int HPanWidth = 
            std::min( H11.Width(), std::max(HPanHeight+offset,0) );
        LockedView( HPan, H, H00.Height(), H00.Width(), HPanHeight, HPanWidth );

        LockedRepartitionUp
        ( tT,  t0,
               t1,
         /**/ /**/
          tB,  t2, HPanWidth );

        RepartitionUp
        ( AT,  A0,
               A1,
         /**/ /**/
          AB,  A2 );

        View2x1( ABottom, A1,
                          A2 );

        Zeros( Z, HPan.Width(), ABottom.Width() );
        Zeros( SInv, HPan.Width(), HPan.Width() );
        //--------------------------------------------------------------------//
        HPanCopy = HPan;
        MakeTrapezoidal( LEFT, LOWER, offset, HPanCopy );
        SetDiagonal( LEFT, offset, HPanCopy, C(1) );

        Herk( UPPER, ADJOINT, C(1), HPanCopy, C(0), SInv );
        FixDiagonal( conjugation, t1, SInv );

        Gemm( ADJOINT, NORMAL, C(1), HPanCopy, ABottom, C(0), Z );
        Trsm( LEFT, UPPER, NORMAL, NON_UNIT, C(1), SInv, Z );
        Gemm( NORMAL, NORMAL, C(-1), HPanCopy, Z, C(1), ABottom );
        //--------------------------------------------------------------------//

        SlideLockedPartitionUpDiagonal
        ( HTL, /**/ HTR,  H00, /**/ H01, H02,
         /*************/ /******************/
               /**/       H10, /**/ H11, H12,
          HBL, /**/ HBR,  H20, /**/ H21, H22 );

        SlideLockedPartitionUp
        ( tT,  t0,
         /**/ /**/
               t1,
          tB,  t2 );

        SlidePartitionUp
        ( AT,  A0,
         /**/ /**/
               A1,
          AB,  A2 );
    }
#ifndef RELEASE
    PopCallStack();
#endif
}

template<typename R> 
inline void
LLVB
( Conjugation conjugation, int offset, 
  const DistMatrix<Complex<R> >& H,
  const DistMatrix<Complex<R>,MD,STAR>& t,
        DistMatrix<Complex<R> >& A )
{
#ifndef RELEASE
    PushCallStack("apply_packed_reflectors::LLVB");
    if( H.Grid() != t.Grid() || t.Grid() != A.Grid() )
        throw std::logic_error("{H,t,A} must be distributed over same grid");
    if( offset > 0 || offset < -H.Height() )
        throw std::logic_error("Transforms out of bounds");
    if( H.Height() != A.Height() )
        throw std::logic_error
        ("Height of transforms must equal height of target matrix");
    if( t.Height() != H.DiagonalLength( offset ) )
        throw std::logic_error("t must be the same length as H's offset diag");
    if( !t.AlignedWithDiagonal( H, offset ) )
        throw std::logic_error("t must be aligned with H's 'offset' diagonal");
#endif
    typedef Complex<R> C;
    const Grid& g = H.Grid();

    DistMatrix<C>
        HTL(g), HTR(g),  H00(g), H01(g), H02(g),  HPan(g), HPanCopy(g),
        HBL(g), HBR(g),  H10(g), H11(g), H12(g),
                         H20(g), H21(g), H22(g);
    DistMatrix<C>
        AT(g),  A0(g),  ABottom(g),
        AB(g),  A1(g),
                A2(g);
    DistMatrix<C,MD,STAR>
        tT(g),  t0(g),
        tB(g),  t1(g),
                t2(g);

    DistMatrix<C,VC,  STAR> HPan_VC_STAR(g);
    DistMatrix<C,MC,  STAR> HPan_MC_STAR(g);
    DistMatrix<C,STAR,STAR> t1_STAR_STAR(g);
    DistMatrix<C,STAR,STAR> SInv_STAR_STAR(g);
    DistMatrix<C,STAR,MR  > Z_STAR_MR(g);
    DistMatrix<C,STAR,VR  > Z_STAR_VR(g);

    LockedPartitionUpDiagonal
    ( H, HTL, HTR,
         HBL, HBR, 0 );
    LockedPartitionUp
    ( t, tT,
         tB, 0 );
    PartitionUp
    ( A, AT,
         AB, std::max(0,H.Height()-H.Width()) );
    while( HBR.Height() < H.Height() && HBR.Width() < H.Width() )
    {
        LockedRepartitionUpDiagonal
        ( HTL, /**/ HTR,  H00, H01, /**/ H02,
               /**/       H10, H11, /**/ H12,
         /*************/ /******************/
          HBL, /**/ HBR,  H20, H21, /**/ H22 );

        const int HPanHeight = H11.Height() + H21.Height();
        const int HPanWidth = 
            std::min( H11.Width(), std::max(HPanHeight+offset,0) );
        LockedView( HPan, H, H00.Height(), H00.Width(), HPanHeight, HPanWidth );

        LockedRepartitionUp
        ( tT,  t0,
               t1,
         /**/ /**/
          tB,  t2, HPanWidth );

        RepartitionUp
        ( AT,  A0,
               A1,
         /**/ /**/
          AB,  A2 );

        View2x1( ABottom, A1,
                          A2 );

        HPan_MC_STAR.AlignWith( ABottom );
        Z_STAR_MR.AlignWith( ABottom );
        Z_STAR_VR.AlignWith( ABottom );
        Zeros( Z_STAR_MR, HPan.Width(), ABottom.Width() );
        Zeros( SInv_STAR_STAR, HPan.Width(), HPan.Width() );
        //--------------------------------------------------------------------//
        HPanCopy = HPan;
        MakeTrapezoidal( LEFT, LOWER, offset, HPanCopy );
        SetDiagonal( LEFT, offset, HPanCopy, C(1) );

        HPan_VC_STAR = HPanCopy;
        Herk
        ( UPPER, ADJOINT, 
          C(1), HPan_VC_STAR.LockedMatrix(),
          C(0), SInv_STAR_STAR.Matrix() );     
        SInv_STAR_STAR.SumOverGrid();
        t1_STAR_STAR = t1;
        FixDiagonal( conjugation, t1_STAR_STAR, SInv_STAR_STAR );

        HPan_MC_STAR = HPanCopy;
        LocalGemm
        ( ADJOINT, NORMAL, C(1), HPan_MC_STAR, ABottom, C(0), Z_STAR_MR );
        Z_STAR_VR.SumScatterFrom( Z_STAR_MR );
 
        LocalTrsm
        ( LEFT, UPPER, NORMAL, NON_UNIT, C(1), SInv_STAR_STAR, Z_STAR_VR );

        Z_STAR_MR = Z_STAR_VR;
        LocalGemm
        ( NORMAL, NORMAL, C(-1), HPan_MC_STAR, Z_STAR_MR, C(1), ABottom );
        //--------------------------------------------------------------------//
        HPan_MC_STAR.FreeAlignments();
        Z_STAR_MR.FreeAlignments();
        Z_STAR_VR.FreeAlignments();

        SlideLockedPartitionUpDiagonal
        ( HTL, /**/ HTR,  H00, /**/ H01, H02,
         /*************/ /******************/
               /**/       H10, /**/ H11, H12,
          HBL, /**/ HBR,  H20, /**/ H21, H22 );

        SlideLockedPartitionUp
        ( tT,  t0,
         /**/ /**/
               t1,
          tB,  t2 );

        SlidePartitionUp
        ( AT,  A0,
         /**/ /**/
               A1,
          AB,  A2 );
    }
#ifndef RELEASE
    PopCallStack();
#endif
}

} // namespace apply_packed_reflectors
} // namespace elem

#endif // ifndef LAPACK_APPLYPACKEDREFLECTORS_LLVB_HPP
