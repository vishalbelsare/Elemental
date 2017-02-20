/*
   Copyright (c) 2009-2016, Jack Poulson
   All rights reserved.

   This file is part of Elemental and is under the BSD 2-Clause License,
   which can be found in the LICENSE file in the root directory, or at
   http://opensource.org/licenses/BSD-2-Clause
*/
#include <El.hpp>
#include <El.h>
using namespace El;

extern "C" {

/* Infeasible IPM
   ============== */
ElError ElIPMCtrlDefault_s( ElIPMCtrl_s* ctrl )
{
    const float eps = limits::Epsilon<float>();

    ctrl->primalInit = false;
    ctrl->dualInit = false;

    ctrl->infeasibilityTol = Pow(eps,float(0.7));
    ctrl->relativeObjectiveGapTol = Pow(eps,float(0.3));
    ctrl->relativeComplementarityGapTol = Pow(eps,float(0.3));
    ctrl->minDimacsDecreaseRatio = 0.99f;

    ctrl->maxIts = 100;
    ctrl->maxStepRatio = 0.99;
    ctrl->system = EL_FULL_KKT;
    ctrl->mehrotra = true;
    ctrl->centralityRule = &StepLengthCentrality<float>;
    ctrl->standardInitShift = true;
    ctrl->forceSameStep = true;
    ElRegSolveCtrlDefault_s( &ctrl->solveCtrl );
    ctrl->outerEquil = true;
    ctrl->twoNormKrylovBasisSize = 6;
    ctrl->print = false;
    ctrl->time = false;

    ctrl->wSafeMaxNorm = Pow(eps,float(-0.15));

    ctrl->equilibrateIfSingleStage = false;
    ctrl->wMaxLimit = Pow(eps,float(-0.4));
    ctrl->ruizEquilTol = Pow(eps,float(-0.25));
    ctrl->ruizMaxIter = 3;
    ctrl->diagEquilTol = Pow(eps,float(-0.15));

#ifdef EL_RELEASE
    ctrl->checkResiduals = false;
#else
    ctrl->checkResiduals = true;
#endif

    ctrl->xRegSmall = ctrl->yRegSmall = ctrl->zRegSmall = Pow(eps,float(0.8));
    ctrl->zMinPivotValue = eps;
    ctrl->xRegLarge = ctrl->yRegLarge = ctrl->zRegLarge = Pow(eps,float(0.7));
    ctrl->twoStage = true;
    ctrl->regIncreaseFactor = Pow(eps,float(0.01));

    ctrl->maxComplementRatio = float(1000);
    ctrl->softDualityTargets = true;
    ctrl->lowerTargetRatioLogCompRatio = float(-0.25);
    ctrl->upperTargetRatioLogCompRatio = float( 0.25);

    return EL_SUCCESS;
}

ElError ElIPMCtrlDefault_d( ElIPMCtrl_d* ctrl )
{
    const double eps = limits::Epsilon<double>();

    ctrl->primalInit = false;
    ctrl->dualInit = false;

    ctrl->infeasibilityTol = Pow(eps,double(0.5));
    ctrl->relativeObjectiveGapTol = Pow(eps,double(0.3));
    ctrl->relativeComplementarityGapTol = Pow(eps,double(0.3));
    ctrl->minDimacsDecreaseRatio = 0.99;

    ctrl->maxIts = 100;
    ctrl->maxStepRatio = 0.99;
    ctrl->system = EL_FULL_KKT;
    ctrl->mehrotra = true;
    ctrl->centralityRule = &StepLengthCentrality<double>;
    ctrl->standardInitShift = true;
    ctrl->forceSameStep = true;
    ElRegSolveCtrlDefault_d( &ctrl->solveCtrl );
    ctrl->outerEquil = true;
    ctrl->twoNormKrylovBasisSize = 6;
    ctrl->print = false;
    ctrl->time = false;

    ctrl->wSafeMaxNorm = Pow(eps,double(-0.15));

    ctrl->equilibrateIfSingleStage = false;
    ctrl->wMaxLimit = Pow(eps,double(-0.4));
    ctrl->ruizEquilTol = Pow(eps,double(-0.25));
    ctrl->ruizMaxIter = 3;
    ctrl->diagEquilTol = Pow(eps,double(-0.15));

#ifdef EL_RELEASE
    ctrl->checkResiduals = false;
#else
    ctrl->checkResiduals = true;
#endif

    ctrl->xRegSmall = ctrl->yRegSmall = ctrl->zRegSmall = Pow(eps,double(0.8));
    ctrl->zMinPivotValue = eps;
    ctrl->xRegLarge = ctrl->yRegLarge = ctrl->zRegLarge = Pow(eps,double(0.7));
    ctrl->twoStage = true;
    ctrl->regIncreaseFactor = Pow(eps,double(0.01));

    ctrl->maxComplementRatio = double(1000);
    ctrl->softDualityTargets = true;
    ctrl->lowerTargetRatioLogCompRatio = double(-0.25);
    ctrl->upperTargetRatioLogCompRatio = double( 0.25);

    return EL_SUCCESS;
}

/* Alternating Direction Method of Multipliers
   =========================================== */
ElError ElADMMCtrlDefault_s( ElADMMCtrl_s* ctrl )
{
    ctrl->rho = 1;
    ctrl->alpha = 1.2;
    ctrl->maxIter = 500;
    ctrl->absTol = 1e-3;
    ctrl->relTol = 1e-2;
    ctrl->inv = true;
    ctrl->print = true;
    return EL_SUCCESS;
}

ElError ElADMMCtrlDefault_d( ElADMMCtrl_d* ctrl )
{
    ctrl->rho = 1;
    ctrl->alpha = 1.2;
    ctrl->maxIter = 500;
    ctrl->absTol = 1e-6;
    ctrl->relTol = 1e-4;
    ctrl->inv = true;
    ctrl->print = true;
    return EL_SUCCESS;
}

/* Linear programs
   =============== */

/* Direct conic form
   ----------------- */
ElError ElLPDirectCtrlDefault_s( ElLPDirectCtrl_s* ctrl, bool isSparse )
{
    ctrl->approach = EL_LP_IPM;
    ElADMMCtrlDefault_s( &ctrl->admmCtrl );
    ElIPMCtrlDefault_s( &ctrl->ipmCtrl );
    if( isSparse )
        ctrl->ipmCtrl.system = EL_AUGMENTED_KKT;
    else
        ctrl->ipmCtrl.system = EL_NORMAL_KKT;
    return EL_SUCCESS;
}

ElError ElLPDirectCtrlDefault_d( ElLPDirectCtrl_d* ctrl, bool isSparse )
{
    ctrl->approach = EL_LP_IPM;
    ElADMMCtrlDefault_d( &ctrl->admmCtrl );
    ElIPMCtrlDefault_d( &ctrl->ipmCtrl );
    if( isSparse )
        ctrl->ipmCtrl.system = EL_AUGMENTED_KKT;
    else
        ctrl->ipmCtrl.system = EL_NORMAL_KKT;
    return EL_SUCCESS;
}

/* Affine conic form
   ----------------- */
ElError ElLPAffineCtrlDefault_s( ElLPAffineCtrl_s* ctrl )
{
    ctrl->approach = EL_LP_IPM;
    ElIPMCtrlDefault_s( &ctrl->ipmCtrl );
    return EL_SUCCESS;
}

ElError ElLPAffineCtrlDefault_d( ElLPAffineCtrl_d* ctrl )
{
    ctrl->approach = EL_LP_IPM;
    ElIPMCtrlDefault_d( &ctrl->ipmCtrl );
    return EL_SUCCESS;
}

/* Quadratic programs
   ================== */

/* Direct conic form
   ----------------- */
ElError ElQPDirectCtrlDefault_s( ElQPDirectCtrl_s* ctrl )
{
    ctrl->approach = EL_QP_IPM;
    ElIPMCtrlDefault_s( &ctrl->ipmCtrl );
    ctrl->ipmCtrl.system = EL_AUGMENTED_KKT;
    return EL_SUCCESS;
}

ElError ElQPDirectCtrlDefault_d( ElQPDirectCtrl_d* ctrl )
{
    ctrl->approach = EL_QP_IPM;
    ElIPMCtrlDefault_d( &ctrl->ipmCtrl );
    ctrl->ipmCtrl.system = EL_AUGMENTED_KKT;
    return EL_SUCCESS;
}

/* Affine conic form
   ----------------- */
ElError ElQPAffineCtrlDefault_s( ElQPAffineCtrl_s* ctrl )
{
    ctrl->approach = EL_QP_IPM;
    ElIPMCtrlDefault_s( &ctrl->ipmCtrl );
    return EL_SUCCESS;
}

ElError ElQPAffineCtrlDefault_d( ElQPAffineCtrl_d* ctrl )
{
    ctrl->approach = EL_QP_IPM;
    ElIPMCtrlDefault_d( &ctrl->ipmCtrl );
    return EL_SUCCESS;
}

/* Second-order cone programs
   ========================== */

/* Direct conic form
   ----------------- */
ElError ElSOCPDirectCtrlDefault_s( ElSOCPDirectCtrl_s* ctrl )
{
    ctrl->approach = EL_SOCP_IPM;
    ElIPMCtrlDefault_s( &ctrl->ipmCtrl );
    ctrl->ipmCtrl.system = EL_AUGMENTED_KKT;
    ctrl->ipmCtrl.infeasibilityTol = 1e-4;
    ctrl->ipmCtrl.relativeObjectiveGapTol = 1e-2;
    ctrl->ipmCtrl.relativeComplementarityGapTol = 1e-2;
    return EL_SUCCESS;
}

ElError ElSOCPDirectCtrlDefault_d( ElSOCPDirectCtrl_d* ctrl )
{
    ctrl->approach = EL_SOCP_IPM;
    ElIPMCtrlDefault_d( &ctrl->ipmCtrl );
    ctrl->ipmCtrl.system = EL_AUGMENTED_KKT;
    ctrl->ipmCtrl.infeasibilityTol = 1e-8;
    ctrl->ipmCtrl.relativeObjectiveGapTol = 1e-4;
    ctrl->ipmCtrl.relativeComplementarityGapTol = 1e-4;
    return EL_SUCCESS;
}

/* Affine conic form
   ----------------- */
ElError ElSOCPAffineCtrlDefault_s( ElSOCPAffineCtrl_s* ctrl )
{
    ctrl->approach = EL_SOCP_IPM;
    ElIPMCtrlDefault_s( &ctrl->ipmCtrl );
    ctrl->ipmCtrl.infeasibilityTol = 1e-4;
    ctrl->ipmCtrl.relativeObjectiveGapTol = 1e-2;
    ctrl->ipmCtrl.relativeComplementarityGapTol = 1e-2;
    return EL_SUCCESS;
}

ElError ElSOCPAffineCtrlDefault_d( ElSOCPAffineCtrl_d* ctrl )
{
    ctrl->approach = EL_SOCP_IPM;
    ElIPMCtrlDefault_d( &ctrl->ipmCtrl );
    ctrl->ipmCtrl.infeasibilityTol = 1e-8;
    ctrl->ipmCtrl.relativeObjectiveGapTol = 1e-4;
    ctrl->ipmCtrl.relativeComplementarityGapTol = 1e-4;
    return EL_SUCCESS;
}

#define C_PROTO_REAL(SIG,Real) \
  /* Linear program
     ============== */ \
  /* Direct conic form
     ----------------- */ \
  ElError ElLPDirect_ ## SIG \
  ( ElConstMatrix_ ## SIG A, \
    ElConstMatrix_ ## SIG b, \
    ElConstMatrix_ ## SIG c, \
    ElMatrix_ ## SIG x, \
    ElMatrix_ ## SIG y, \
    ElMatrix_ ## SIG z ) \
  { EL_TRY( LP( *CReflect(A), \
      *CReflect(b), *CReflect(c), \
      *CReflect(x), *CReflect(y), *CReflect(z) ) ) } \
  ElError ElLPDirectDist_ ## SIG \
  ( ElConstDistMatrix_ ## SIG A, \
    ElConstDistMatrix_ ## SIG b, \
    ElConstDistMatrix_ ## SIG c, \
    ElDistMatrix_ ## SIG x, \
    ElDistMatrix_ ## SIG y, \
    ElDistMatrix_ ## SIG z ) \
  { EL_TRY( LP( *CReflect(A), \
      *CReflect(b), *CReflect(c), \
      *CReflect(x), *CReflect(y), *CReflect(z) ) ) } \
  ElError ElLPDirectSparse_ ## SIG \
  ( ElConstSparseMatrix_ ## SIG A, \
    ElConstMatrix_ ## SIG b, \
    ElConstMatrix_ ## SIG c, \
    ElMatrix_ ## SIG x, \
    ElMatrix_ ## SIG y, \
    ElMatrix_ ## SIG z ) \
  { EL_TRY( LP( *CReflect(A), \
      *CReflect(b), *CReflect(c), \
      *CReflect(x), *CReflect(y), *CReflect(z) ) ) } \
  ElError ElLPDirectDistSparse_ ## SIG \
  ( ElConstDistSparseMatrix_ ## SIG A, \
    ElConstDistMultiVec_ ## SIG b, \
    ElConstDistMultiVec_ ## SIG c, \
    ElDistMultiVec_ ## SIG x, \
    ElDistMultiVec_ ## SIG y, \
    ElDistMultiVec_ ## SIG z ) \
  { EL_TRY( LP( *CReflect(A), \
      *CReflect(b), *CReflect(c), \
      *CReflect(x), *CReflect(y), *CReflect(z) ) ) } \
  /* Expert version
     ^^^^^^^^^^^^^^ */ \
  ElError ElLPDirectX_ ## SIG \
  ( ElConstMatrix_ ## SIG A, \
    ElConstMatrix_ ## SIG b, \
    ElConstMatrix_ ## SIG c, \
    ElMatrix_ ## SIG x, \
    ElMatrix_ ## SIG y, \
    ElMatrix_ ## SIG z, \
    ElLPDirectCtrl_ ## SIG ctrl ) \
  { EL_TRY( LP( *CReflect(A), \
      *CReflect(b), *CReflect(c), \
      *CReflect(x), *CReflect(y), *CReflect(z), \
       CReflect(ctrl) ) ) } \
  ElError ElLPDirectXDist_ ## SIG \
  ( ElConstDistMatrix_ ## SIG A, \
    ElConstDistMatrix_ ## SIG b, \
    ElConstDistMatrix_ ## SIG c, \
    ElDistMatrix_ ## SIG x, \
    ElDistMatrix_ ## SIG y, \
    ElDistMatrix_ ## SIG z, \
    ElLPDirectCtrl_ ## SIG ctrl ) \
  { EL_TRY( LP( *CReflect(A), \
      *CReflect(b), *CReflect(c), \
      *CReflect(x), *CReflect(y), *CReflect(z), \
       CReflect(ctrl) ) ) } \
  ElError ElLPDirectXSparse_ ## SIG \
  ( ElConstSparseMatrix_ ## SIG A, \
    ElConstMatrix_ ## SIG b, \
    ElConstMatrix_ ## SIG c, \
    ElMatrix_ ## SIG x, \
    ElMatrix_ ## SIG y, \
    ElMatrix_ ## SIG z, \
    ElLPDirectCtrl_ ## SIG ctrl ) \
  { EL_TRY( LP( *CReflect(A), \
      *CReflect(b), *CReflect(c), \
      *CReflect(x), *CReflect(y), *CReflect(z), \
       CReflect(ctrl) ) ) } \
  ElError ElLPDirectXDistSparse_ ## SIG \
  ( ElConstDistSparseMatrix_ ## SIG A, \
    ElConstDistMultiVec_ ## SIG b, \
    ElConstDistMultiVec_ ## SIG c, \
    ElDistMultiVec_ ## SIG x, \
    ElDistMultiVec_ ## SIG y, \
    ElDistMultiVec_ ## SIG z, \
    ElLPDirectCtrl_ ## SIG ctrl ) \
  { EL_TRY( LP( *CReflect(A), \
      *CReflect(b), *CReflect(c), \
      *CReflect(x), *CReflect(y), *CReflect(z), \
       CReflect(ctrl) ) ) } \
  /* Affine conic form
     ----------------- */ \
  ElError ElLPAffine_ ## SIG \
  ( ElConstMatrix_ ## SIG A, \
    ElConstMatrix_ ## SIG G, \
    ElConstMatrix_ ## SIG b, \
    ElConstMatrix_ ## SIG c, \
    ElConstMatrix_ ## SIG h, \
    ElMatrix_ ## SIG x, \
    ElMatrix_ ## SIG y, \
    ElMatrix_ ## SIG z, \
    ElMatrix_ ## SIG s ) \
  { EL_TRY( LP( *CReflect(A), *CReflect(G), \
      *CReflect(b), *CReflect(c), *CReflect(h), \
      *CReflect(x), *CReflect(y), *CReflect(z), *CReflect(s) ) ) } \
  ElError ElLPAffineDist_ ## SIG \
  ( ElConstDistMatrix_ ## SIG A, \
    ElConstDistMatrix_ ## SIG G, \
    ElConstDistMatrix_ ## SIG b, \
    ElConstDistMatrix_ ## SIG c, \
    ElConstDistMatrix_ ## SIG h, \
    ElDistMatrix_ ## SIG x, \
    ElDistMatrix_ ## SIG y, \
    ElDistMatrix_ ## SIG z, \
    ElDistMatrix_ ## SIG s ) \
  { EL_TRY( LP( *CReflect(A), *CReflect(G), \
      *CReflect(b), *CReflect(c), *CReflect(h), \
      *CReflect(x), *CReflect(y), *CReflect(z), *CReflect(s) ) ) } \
  ElError ElLPAffineSparse_ ## SIG \
  ( ElConstSparseMatrix_ ## SIG A, \
    ElConstSparseMatrix_ ## SIG G, \
    ElConstMatrix_ ## SIG b, \
    ElConstMatrix_ ## SIG c, \
    ElConstMatrix_ ## SIG h, \
    ElMatrix_ ## SIG x, \
    ElMatrix_ ## SIG y, \
    ElMatrix_ ## SIG z, \
    ElMatrix_ ## SIG s ) \
  { EL_TRY( LP( *CReflect(A), *CReflect(G), \
      *CReflect(b), *CReflect(c), *CReflect(h), \
      *CReflect(x), *CReflect(y), *CReflect(z), *CReflect(s) ) ) } \
  ElError ElLPAffineDistSparse_ ## SIG \
  ( ElConstDistSparseMatrix_ ## SIG A, \
    ElConstDistSparseMatrix_ ## SIG G, \
    ElConstDistMultiVec_ ## SIG b, \
    ElConstDistMultiVec_ ## SIG c, \
    ElConstDistMultiVec_ ## SIG h, \
    ElDistMultiVec_ ## SIG x, \
    ElDistMultiVec_ ## SIG y, \
    ElDistMultiVec_ ## SIG z, \
    ElDistMultiVec_ ## SIG s ) \
  { EL_TRY( LP( *CReflect(A), *CReflect(G), \
      *CReflect(b), *CReflect(c), *CReflect(h), \
      *CReflect(x), *CReflect(y), *CReflect(z), *CReflect(s) ) ) } \
  /* Expert version
     ^^^^^^^^^^^^^^ */ \
  ElError ElLPAffineX_ ## SIG \
  ( ElConstMatrix_ ## SIG A, \
    ElConstMatrix_ ## SIG G, \
    ElConstMatrix_ ## SIG b, \
    ElConstMatrix_ ## SIG c, \
    ElConstMatrix_ ## SIG h, \
    ElMatrix_ ## SIG x, \
    ElMatrix_ ## SIG y, \
    ElMatrix_ ## SIG z, \
    ElMatrix_ ## SIG s, \
    ElLPAffineCtrl_ ## SIG ctrl ) \
  { EL_TRY( LP( *CReflect(A), *CReflect(G), \
      *CReflect(b), *CReflect(c), *CReflect(h), \
      *CReflect(x), *CReflect(y), *CReflect(z), *CReflect(s), \
       CReflect(ctrl) ) ) } \
  ElError ElLPAffineXDist_ ## SIG \
  ( ElConstDistMatrix_ ## SIG A, \
    ElConstDistMatrix_ ## SIG G, \
    ElConstDistMatrix_ ## SIG b, \
    ElConstDistMatrix_ ## SIG c, \
    ElConstDistMatrix_ ## SIG h, \
    ElDistMatrix_ ## SIG x, \
    ElDistMatrix_ ## SIG y, \
    ElDistMatrix_ ## SIG z, \
    ElDistMatrix_ ## SIG s, \
    ElLPAffineCtrl_ ## SIG ctrl ) \
  { EL_TRY( LP( *CReflect(A), *CReflect(G), \
      *CReflect(b), *CReflect(c), *CReflect(h), \
      *CReflect(x), *CReflect(y), *CReflect(z), *CReflect(s), \
       CReflect(ctrl) ) ) } \
  ElError ElLPAffineXSparse_ ## SIG \
  ( ElConstSparseMatrix_ ## SIG A, \
    ElConstSparseMatrix_ ## SIG G, \
    ElConstMatrix_ ## SIG b, \
    ElConstMatrix_ ## SIG c, \
    ElConstMatrix_ ## SIG h, \
    ElMatrix_ ## SIG x, \
    ElMatrix_ ## SIG y, \
    ElMatrix_ ## SIG z, \
    ElMatrix_ ## SIG s, \
    ElLPAffineCtrl_ ## SIG ctrl ) \
  { EL_TRY( LP( *CReflect(A), *CReflect(G), \
      *CReflect(b), *CReflect(c), *CReflect(h), \
      *CReflect(x), *CReflect(y), *CReflect(z), *CReflect(s), \
       CReflect(ctrl) ) ) } \
  ElError ElLPAffineXDistSparse_ ## SIG \
  ( ElConstDistSparseMatrix_ ## SIG A, \
    ElConstDistSparseMatrix_ ## SIG G, \
    ElConstDistMultiVec_ ## SIG b, \
    ElConstDistMultiVec_ ## SIG c, \
    ElConstDistMultiVec_ ## SIG h, \
    ElDistMultiVec_ ## SIG x, \
    ElDistMultiVec_ ## SIG y, \
    ElDistMultiVec_ ## SIG z, \
    ElDistMultiVec_ ## SIG s, \
    ElLPAffineCtrl_ ## SIG ctrl ) \
  { EL_TRY( LP( *CReflect(A), *CReflect(G), \
      *CReflect(b), *CReflect(c), *CReflect(h), \
      *CReflect(x), *CReflect(y), *CReflect(z), *CReflect(s), \
       CReflect(ctrl) ) ) } \
  /* Self-dual conic form
     -------------------- */ \
  /* TODO */ \
  /* Quadratic program
     ================= */ \
  /* Direct conic form
     ----------------- */ \
  ElError ElQPDirect_ ## SIG \
  ( ElConstMatrix_ ## SIG Q, \
    ElConstMatrix_ ## SIG A, \
    ElConstMatrix_ ## SIG b, \
    ElConstMatrix_ ## SIG c, \
    ElMatrix_ ## SIG x, \
    ElMatrix_ ## SIG y, \
    ElMatrix_ ## SIG z ) \
  { EL_TRY( QP( *CReflect(Q), *CReflect(A), \
      *CReflect(b), *CReflect(c), \
      *CReflect(x), *CReflect(y), *CReflect(z) ) ) } \
  ElError ElQPDirectDist_ ## SIG \
  ( ElConstDistMatrix_ ## SIG Q, \
    ElConstDistMatrix_ ## SIG A, \
    ElConstDistMatrix_ ## SIG b, \
    ElConstDistMatrix_ ## SIG c, \
    ElDistMatrix_ ## SIG x, \
    ElDistMatrix_ ## SIG y, \
    ElDistMatrix_ ## SIG z ) \
  { EL_TRY( QP( *CReflect(Q), *CReflect(A), \
      *CReflect(b), *CReflect(c), \
      *CReflect(x), *CReflect(y), *CReflect(z) ) ) } \
  ElError ElQPDirectSparse_ ## SIG \
  ( ElConstSparseMatrix_ ## SIG Q, \
    ElConstSparseMatrix_ ## SIG A, \
    ElConstMatrix_ ## SIG b, \
    ElConstMatrix_ ## SIG c, \
    ElMatrix_ ## SIG x, \
    ElMatrix_ ## SIG y, \
    ElMatrix_ ## SIG z ) \
  { EL_TRY( QP( *CReflect(Q), *CReflect(A), \
      *CReflect(b), *CReflect(c), \
      *CReflect(x), *CReflect(y), *CReflect(z) ) ) } \
  ElError ElQPDirectDistSparse_ ## SIG \
  ( ElConstDistSparseMatrix_ ## SIG Q, \
    ElConstDistSparseMatrix_ ## SIG A, \
    ElConstDistMultiVec_ ## SIG b, \
    ElConstDistMultiVec_ ## SIG c, \
    ElDistMultiVec_ ## SIG x, \
    ElDistMultiVec_ ## SIG y, \
    ElDistMultiVec_ ## SIG z ) \
  { EL_TRY( QP( *CReflect(Q), *CReflect(A), \
      *CReflect(b), *CReflect(c), \
      *CReflect(x), *CReflect(y), *CReflect(z) ) ) } \
  /* Expert version
     ^^^^^^^^^^^^^^ */ \
  ElError ElQPDirectX_ ## SIG \
  ( ElConstMatrix_ ## SIG Q, \
    ElConstMatrix_ ## SIG A, \
    ElConstMatrix_ ## SIG b, \
    ElConstMatrix_ ## SIG c, \
    ElMatrix_ ## SIG x, \
    ElMatrix_ ## SIG y, \
    ElMatrix_ ## SIG z, \
    ElQPDirectCtrl_ ## SIG ctrl ) \
  { EL_TRY( QP( *CReflect(Q), *CReflect(A), \
      *CReflect(b), *CReflect(c), \
      *CReflect(x), *CReflect(y), *CReflect(z), \
       CReflect(ctrl) ) ) } \
  ElError ElQPDirectXDist_ ## SIG \
  ( ElConstDistMatrix_ ## SIG Q, \
    ElConstDistMatrix_ ## SIG A, \
    ElConstDistMatrix_ ## SIG b, \
    ElConstDistMatrix_ ## SIG c, \
    ElDistMatrix_ ## SIG x, \
    ElDistMatrix_ ## SIG y, \
    ElDistMatrix_ ## SIG z, \
    ElQPDirectCtrl_ ## SIG ctrl ) \
  { EL_TRY( QP( *CReflect(Q), *CReflect(A), \
      *CReflect(b), *CReflect(c), \
      *CReflect(x), *CReflect(y), *CReflect(z), \
       CReflect(ctrl) ) ) } \
  ElError ElQPDirectXSparse_ ## SIG \
  ( ElConstSparseMatrix_ ## SIG Q, \
    ElConstSparseMatrix_ ## SIG A, \
    ElConstMatrix_ ## SIG b, \
    ElConstMatrix_ ## SIG c, \
    ElMatrix_ ## SIG x, \
    ElMatrix_ ## SIG y, \
    ElMatrix_ ## SIG z, \
    ElQPDirectCtrl_ ## SIG ctrl ) \
  { EL_TRY( QP( *CReflect(Q), *CReflect(A), \
      *CReflect(b), *CReflect(c), \
      *CReflect(x), *CReflect(y), *CReflect(z), \
       CReflect(ctrl) ) ) } \
  ElError ElQPDirectXDistSparse_ ## SIG \
  ( ElConstDistSparseMatrix_ ## SIG Q, \
    ElConstDistSparseMatrix_ ## SIG A, \
    ElConstDistMultiVec_ ## SIG b, \
    ElConstDistMultiVec_ ## SIG c, \
    ElDistMultiVec_ ## SIG x, \
    ElDistMultiVec_ ## SIG y, \
    ElDistMultiVec_ ## SIG z, \
    ElQPDirectCtrl_ ## SIG ctrl ) \
  { EL_TRY( QP( *CReflect(Q), *CReflect(A), \
      *CReflect(b), *CReflect(c), \
      *CReflect(x), *CReflect(y), *CReflect(z), \
       CReflect(ctrl) ) ) } \
  /* Affine conic form
     ----------------- */ \
  ElError ElQPAffine_ ## SIG \
  ( ElConstMatrix_ ## SIG Q, \
    ElConstMatrix_ ## SIG A, \
    ElConstMatrix_ ## SIG G, \
    ElConstMatrix_ ## SIG b, \
    ElConstMatrix_ ## SIG c, \
    ElConstMatrix_ ## SIG h, \
    ElMatrix_ ## SIG x, \
    ElMatrix_ ## SIG y, \
    ElMatrix_ ## SIG z, \
    ElMatrix_ ## SIG s ) \
  { EL_TRY( QP( *CReflect(Q), *CReflect(A), *CReflect(G), \
      *CReflect(b), *CReflect(c), *CReflect(h), \
      *CReflect(x), *CReflect(y), *CReflect(z), *CReflect(s) ) ) } \
  ElError ElQPAffineDist_ ## SIG \
  ( ElConstDistMatrix_ ## SIG Q, \
    ElConstDistMatrix_ ## SIG A, \
    ElConstDistMatrix_ ## SIG G, \
    ElConstDistMatrix_ ## SIG b, \
    ElConstDistMatrix_ ## SIG c, \
    ElConstDistMatrix_ ## SIG h, \
    ElDistMatrix_ ## SIG x, \
    ElDistMatrix_ ## SIG y, \
    ElDistMatrix_ ## SIG z, \
    ElDistMatrix_ ## SIG s ) \
  { EL_TRY( QP( *CReflect(Q), *CReflect(A), *CReflect(G), \
      *CReflect(b), *CReflect(c), *CReflect(h), \
      *CReflect(x), *CReflect(y), *CReflect(z), *CReflect(s) ) ) } \
  ElError ElQPAffineSparse_ ## SIG \
  ( ElConstSparseMatrix_ ## SIG Q, \
    ElConstSparseMatrix_ ## SIG A, \
    ElConstSparseMatrix_ ## SIG G, \
    ElConstMatrix_ ## SIG b, \
    ElConstMatrix_ ## SIG c, \
    ElConstMatrix_ ## SIG h, \
    ElMatrix_ ## SIG x, \
    ElMatrix_ ## SIG y, \
    ElMatrix_ ## SIG z, \
    ElMatrix_ ## SIG s ) \
  { EL_TRY( QP( *CReflect(Q), *CReflect(A), *CReflect(G), \
      *CReflect(b), *CReflect(c), *CReflect(h), \
      *CReflect(x), *CReflect(y), *CReflect(z), *CReflect(s) ) ) } \
  ElError ElQPAffineDistSparse_ ## SIG \
  ( ElConstDistSparseMatrix_ ## SIG Q, \
    ElConstDistSparseMatrix_ ## SIG A, \
    ElConstDistSparseMatrix_ ## SIG G, \
    ElConstDistMultiVec_ ## SIG b, \
    ElConstDistMultiVec_ ## SIG c, \
    ElConstDistMultiVec_ ## SIG h, \
    ElDistMultiVec_ ## SIG x, \
    ElDistMultiVec_ ## SIG y, \
    ElDistMultiVec_ ## SIG z, \
    ElDistMultiVec_ ## SIG s ) \
  { EL_TRY( QP( *CReflect(Q), *CReflect(A), *CReflect(G), \
      *CReflect(b), *CReflect(c), *CReflect(h), \
      *CReflect(x), *CReflect(y), *CReflect(z), *CReflect(s) ) ) } \
  /* Expert version
     ^^^^^^^^^^^^^^ */ \
  ElError ElQPAffineX_ ## SIG \
  ( ElConstMatrix_ ## SIG Q, \
    ElConstMatrix_ ## SIG A, \
    ElConstMatrix_ ## SIG G, \
    ElConstMatrix_ ## SIG b, \
    ElConstMatrix_ ## SIG c, \
    ElConstMatrix_ ## SIG h, \
    ElMatrix_ ## SIG x, \
    ElMatrix_ ## SIG y, \
    ElMatrix_ ## SIG z, \
    ElMatrix_ ## SIG s, \
    ElQPAffineCtrl_ ## SIG ctrl ) \
  { EL_TRY( QP( *CReflect(Q), *CReflect(A), *CReflect(G), \
      *CReflect(b), *CReflect(c), *CReflect(h), \
      *CReflect(x), *CReflect(y), *CReflect(z), *CReflect(s), \
       CReflect(ctrl) ) ) } \
  ElError ElQPAffineXDist_ ## SIG \
  ( ElConstDistMatrix_ ## SIG Q, \
    ElConstDistMatrix_ ## SIG A, \
    ElConstDistMatrix_ ## SIG G, \
    ElConstDistMatrix_ ## SIG b, \
    ElConstDistMatrix_ ## SIG c, \
    ElConstDistMatrix_ ## SIG h, \
    ElDistMatrix_ ## SIG x, \
    ElDistMatrix_ ## SIG y, \
    ElDistMatrix_ ## SIG z, \
    ElDistMatrix_ ## SIG s, \
    ElQPAffineCtrl_ ## SIG ctrl ) \
  { EL_TRY( QP( *CReflect(Q), *CReflect(A), *CReflect(G), \
      *CReflect(b), *CReflect(c), *CReflect(h), \
      *CReflect(x), *CReflect(y), *CReflect(z), *CReflect(s), \
       CReflect(ctrl) ) ) } \
  ElError ElQPAffineXSparse_ ## SIG \
  ( ElConstSparseMatrix_ ## SIG Q, \
    ElConstSparseMatrix_ ## SIG A, \
    ElConstSparseMatrix_ ## SIG G, \
    ElConstMatrix_ ## SIG b, \
    ElConstMatrix_ ## SIG c, \
    ElConstMatrix_ ## SIG h, \
    ElMatrix_ ## SIG x, \
    ElMatrix_ ## SIG y, \
    ElMatrix_ ## SIG z, \
    ElMatrix_ ## SIG s, \
    ElQPAffineCtrl_ ## SIG ctrl ) \
  { EL_TRY( QP( *CReflect(Q), *CReflect(A), *CReflect(G), \
      *CReflect(b), *CReflect(c), *CReflect(h), \
      *CReflect(x), *CReflect(y), *CReflect(z), *CReflect(s), \
       CReflect(ctrl) ) ) } \
  ElError ElQPAffineXDistSparse_ ## SIG \
  ( ElConstDistSparseMatrix_ ## SIG Q, \
    ElConstDistSparseMatrix_ ## SIG A, \
    ElConstDistSparseMatrix_ ## SIG G, \
    ElConstDistMultiVec_ ## SIG b, \
    ElConstDistMultiVec_ ## SIG c, \
    ElConstDistMultiVec_ ## SIG h, \
    ElDistMultiVec_ ## SIG x, \
    ElDistMultiVec_ ## SIG y, \
    ElDistMultiVec_ ## SIG z, \
    ElDistMultiVec_ ## SIG s, \
    ElQPAffineCtrl_ ## SIG ctrl ) \
  { EL_TRY( QP( *CReflect(Q), *CReflect(A), *CReflect(G), \
      *CReflect(b), *CReflect(c), *CReflect(h), \
      *CReflect(x), *CReflect(y), *CReflect(z), *CReflect(s), \
       CReflect(ctrl) ) ) } \
  /* Self-dual conic form
     -------------------- */ \
  /* TODO */ \
  /* Box form (no linear equalities)
     ------------------------------- */ \
  ElError ElQPBoxADMM_ ## SIG \
  ( ElConstMatrix_ ## SIG Q, \
    ElConstMatrix_ ## SIG C, \
    Real lb, Real ub, \
    ElMatrix_ ## SIG Z, \
    ElInt* numIts ) \
  { EL_TRY( *numIts = qp::box::ADMM( *CReflect(Q), *CReflect(C), lb, ub, \
      *CReflect(Z) ) ) } \
  ElError ElQPBoxADMMDist_ ## SIG \
  ( ElConstDistMatrix_ ## SIG Q, \
    ElConstDistMatrix_ ## SIG C, \
    Real lb, Real ub, \
    ElDistMatrix_ ## SIG Z, \
    ElInt* numIts ) \
  { EL_TRY( *numIts = qp::box::ADMM( *CReflect(Q), *CReflect(C), lb, ub, \
      *CReflect(Z) ) ) } \
  /* Expert versions
     ^^^^^^^^^^^^^^^ */ \
  ElError ElQPBoxADMMX_ ## SIG \
  ( ElConstMatrix_ ## SIG Q, \
    ElConstMatrix_ ## SIG C, \
    Real lb, Real ub, \
    ElMatrix_ ## SIG Z, \
    ElADMMCtrl_ ## SIG ctrl, \
    ElInt* numIts ) \
  { EL_TRY( *numIts = qp::box::ADMM( *CReflect(Q), *CReflect(C), lb, ub, \
      *CReflect(Z), CReflect(ctrl) ) ) } \
  ElError ElQPBoxADMMXDist_ ## SIG \
  ( ElConstDistMatrix_ ## SIG Q, \
    ElConstDistMatrix_ ## SIG C, \
    Real lb, Real ub, \
    ElDistMatrix_ ## SIG Z, \
    ElADMMCtrl_ ## SIG ctrl, \
    ElInt* numIts ) \
  { EL_TRY( *numIts = qp::box::ADMM( *CReflect(Q), *CReflect(C), lb, ub, \
      *CReflect(Z), CReflect(ctrl) ) ) } \
  /* Affine conic form
     ----------------- */ \
  /* TODO */ \
  /* Self-dual conic form
     -------------------- */ \
  /* TODO */ \
  /* Second-order cone programs
     ========================== */ \
  /* Direct conic form
     ----------------- */ \
  ElError ElSOCPDirect_ ## SIG \
  ( ElConstMatrix_ ## SIG A, \
    ElConstMatrix_ ## SIG b, \
    ElConstMatrix_ ## SIG c, \
    ElConstMatrix_i orders, \
    ElConstMatrix_i firstInds, \
    ElMatrix_ ## SIG x, \
    ElMatrix_ ## SIG y, \
    ElMatrix_ ## SIG z ) \
  { EL_TRY( SOCP( *CReflect(A), \
      *CReflect(b), *CReflect(c), \
      *CReflect(orders), *CReflect(firstInds), \
      *CReflect(x), *CReflect(y), *CReflect(z) ) ) } \
  ElError ElSOCPDirectDist_ ## SIG \
  ( ElConstDistMatrix_ ## SIG A, \
    ElConstDistMatrix_ ## SIG b, \
    ElConstDistMatrix_ ## SIG c, \
    ElConstDistMatrix_i orders, \
    ElConstDistMatrix_i firstInds, \
    ElDistMatrix_ ## SIG x, \
    ElDistMatrix_ ## SIG y, \
    ElDistMatrix_ ## SIG z ) \
  { EL_TRY( SOCP( *CReflect(A), \
      *CReflect(b), *CReflect(c), \
      *CReflect(orders), *CReflect(firstInds), \
      *CReflect(x), *CReflect(y), *CReflect(z) ) ) } \
  ElError ElSOCPDirectSparse_ ## SIG \
  ( ElConstSparseMatrix_ ## SIG A, \
    ElConstMatrix_ ## SIG b, \
    ElConstMatrix_ ## SIG c, \
    ElConstMatrix_i orders, \
    ElConstMatrix_i firstInds, \
    ElMatrix_ ## SIG x, \
    ElMatrix_ ## SIG y, \
    ElMatrix_ ## SIG z ) \
  { EL_TRY( SOCP( *CReflect(A), \
      *CReflect(b), *CReflect(c), \
      *CReflect(orders), *CReflect(firstInds), \
      *CReflect(x), *CReflect(y), *CReflect(z) ) ) } \
  ElError ElSOCPDirectDistSparse_ ## SIG \
  ( ElConstDistSparseMatrix_ ## SIG A, \
    ElConstDistMultiVec_ ## SIG b, \
    ElConstDistMultiVec_ ## SIG c, \
    ElConstDistMultiVec_i orders, \
    ElConstDistMultiVec_i firstInds, \
    ElDistMultiVec_ ## SIG x, \
    ElDistMultiVec_ ## SIG y, \
    ElDistMultiVec_ ## SIG z ) \
  { EL_TRY( SOCP( *CReflect(A), \
      *CReflect(b), *CReflect(c), \
      *CReflect(orders), *CReflect(firstInds), \
      *CReflect(x), *CReflect(y), *CReflect(z) ) ) } \
  /* Expert version
     ^^^^^^^^^^^^^^ */ \
  ElError ElSOCPDirectX_ ## SIG \
  ( ElConstMatrix_ ## SIG A, \
    ElConstMatrix_ ## SIG b, \
    ElConstMatrix_ ## SIG c, \
    ElConstMatrix_i orders, \
    ElConstMatrix_i firstInds, \
    ElMatrix_ ## SIG x, \
    ElMatrix_ ## SIG y, \
    ElMatrix_ ## SIG z, \
    ElSOCPDirectCtrl_ ## SIG ctrl ) \
  { EL_TRY( SOCP( *CReflect(A), \
      *CReflect(b), *CReflect(c), \
      *CReflect(orders), *CReflect(firstInds), \
      *CReflect(x), *CReflect(y), *CReflect(z), \
       CReflect(ctrl) ) ) } \
  ElError ElSOCPDirectXDist_ ## SIG \
  ( ElConstDistMatrix_ ## SIG A, \
    ElConstDistMatrix_ ## SIG b, \
    ElConstDistMatrix_ ## SIG c, \
    ElConstDistMatrix_i orders, \
    ElConstDistMatrix_i firstInds, \
    ElDistMatrix_ ## SIG x, \
    ElDistMatrix_ ## SIG y, \
    ElDistMatrix_ ## SIG z, \
    ElSOCPDirectCtrl_ ## SIG ctrl ) \
  { EL_TRY( SOCP( *CReflect(A), \
      *CReflect(b), *CReflect(c), \
      *CReflect(orders), *CReflect(firstInds), \
      *CReflect(x), *CReflect(y), *CReflect(z), \
       CReflect(ctrl) ) ) } \
  ElError ElSOCPDirectXSparse_ ## SIG \
  ( ElConstSparseMatrix_ ## SIG A, \
    ElConstMatrix_ ## SIG b, \
    ElConstMatrix_ ## SIG c, \
    ElConstMatrix_i orders, \
    ElConstMatrix_i firstInds, \
    ElMatrix_ ## SIG x, \
    ElMatrix_ ## SIG y, \
    ElMatrix_ ## SIG z, \
    ElSOCPDirectCtrl_ ## SIG ctrl ) \
  { EL_TRY( SOCP( *CReflect(A), \
      *CReflect(b), *CReflect(c), \
      *CReflect(orders), *CReflect(firstInds), \
      *CReflect(x), *CReflect(y), *CReflect(z), \
       CReflect(ctrl) ) ) } \
  ElError ElSOCPDirectXDistSparse_ ## SIG \
  ( ElConstDistSparseMatrix_ ## SIG A, \
    ElConstDistMultiVec_ ## SIG b, \
    ElConstDistMultiVec_ ## SIG c, \
    ElConstDistMultiVec_i orders, \
    ElConstDistMultiVec_i firstInds, \
    ElDistMultiVec_ ## SIG x, \
    ElDistMultiVec_ ## SIG y, \
    ElDistMultiVec_ ## SIG z, \
    ElSOCPDirectCtrl_ ## SIG ctrl ) \
  { EL_TRY( SOCP( *CReflect(A), \
      *CReflect(b), *CReflect(c), \
      *CReflect(orders), *CReflect(firstInds), \
      *CReflect(x), *CReflect(y), *CReflect(z), \
       CReflect(ctrl) ) ) } \
  /* Affine conic form
     ----------------- */ \
  ElError ElSOCPAffine_ ## SIG \
  ( ElConstMatrix_ ## SIG A, \
    ElConstMatrix_ ## SIG G, \
    ElConstMatrix_ ## SIG b, \
    ElConstMatrix_ ## SIG c, \
    ElConstMatrix_ ## SIG h, \
    ElConstMatrix_i orders, \
    ElConstMatrix_i firstInds, \
    ElMatrix_ ## SIG x, \
    ElMatrix_ ## SIG y, \
    ElMatrix_ ## SIG z, \
    ElMatrix_ ## SIG s ) \
  { EL_TRY( SOCP( *CReflect(A), *CReflect(G), \
      *CReflect(b), *CReflect(c), *CReflect(h), \
      *CReflect(orders), *CReflect(firstInds), \
      *CReflect(x), *CReflect(y), *CReflect(z), *CReflect(s) ) ) } \
  ElError ElSOCPAffineDist_ ## SIG \
  ( ElConstDistMatrix_ ## SIG A, \
    ElConstDistMatrix_ ## SIG G, \
    ElConstDistMatrix_ ## SIG b, \
    ElConstDistMatrix_ ## SIG c, \
    ElConstDistMatrix_ ## SIG h, \
    ElConstDistMatrix_i orders, \
    ElConstDistMatrix_i firstInds, \
    ElDistMatrix_ ## SIG x, \
    ElDistMatrix_ ## SIG y, \
    ElDistMatrix_ ## SIG z, \
    ElDistMatrix_ ## SIG s ) \
  { EL_TRY( SOCP( *CReflect(A), *CReflect(G), \
      *CReflect(b), *CReflect(c), *CReflect(h), \
      *CReflect(orders), *CReflect(firstInds), \
      *CReflect(x), *CReflect(y), *CReflect(z), *CReflect(s) ) ) } \
  ElError ElSOCPAffineSparse_ ## SIG \
  ( ElConstSparseMatrix_ ## SIG A, \
    ElConstSparseMatrix_ ## SIG G, \
    ElConstMatrix_ ## SIG b, \
    ElConstMatrix_ ## SIG c, \
    ElConstMatrix_ ## SIG h, \
    ElConstMatrix_i orders, \
    ElConstMatrix_i firstInds, \
    ElMatrix_ ## SIG x, \
    ElMatrix_ ## SIG y, \
    ElMatrix_ ## SIG z, \
    ElMatrix_ ## SIG s ) \
  { EL_TRY( SOCP( *CReflect(A), *CReflect(G), \
      *CReflect(b), *CReflect(c), *CReflect(h), \
      *CReflect(orders), *CReflect(firstInds), \
      *CReflect(x), *CReflect(y), *CReflect(z), *CReflect(s) ) ) } \
  ElError ElSOCPAffineDistSparse_ ## SIG \
  ( ElConstDistSparseMatrix_ ## SIG A, \
    ElConstDistSparseMatrix_ ## SIG G, \
    ElConstDistMultiVec_ ## SIG b, \
    ElConstDistMultiVec_ ## SIG c, \
    ElConstDistMultiVec_ ## SIG h, \
    ElConstDistMultiVec_i orders, \
    ElConstDistMultiVec_i firstInds, \
    ElDistMultiVec_ ## SIG x, \
    ElDistMultiVec_ ## SIG y, \
    ElDistMultiVec_ ## SIG z, \
    ElDistMultiVec_ ## SIG s ) \
  { EL_TRY( SOCP( *CReflect(A), *CReflect(G), \
      *CReflect(b), *CReflect(c), *CReflect(h), \
      *CReflect(orders), *CReflect(firstInds), \
      *CReflect(x), *CReflect(y), *CReflect(z), *CReflect(s) ) ) } \
  /* Expert version
     ^^^^^^^^^^^^^^ */ \
  ElError ElSOCPAffineX_ ## SIG \
  ( ElConstMatrix_ ## SIG A, \
    ElConstMatrix_ ## SIG G, \
    ElConstMatrix_ ## SIG b, \
    ElConstMatrix_ ## SIG c, \
    ElConstMatrix_ ## SIG h, \
    ElConstMatrix_i orders, \
    ElConstMatrix_i firstInds, \
    ElMatrix_ ## SIG x, \
    ElMatrix_ ## SIG y, \
    ElMatrix_ ## SIG z, \
    ElMatrix_ ## SIG s, \
    ElSOCPAffineCtrl_ ## SIG ctrl ) \
  { EL_TRY( SOCP( *CReflect(A), *CReflect(G), \
      *CReflect(b), *CReflect(c), *CReflect(h), \
      *CReflect(orders), *CReflect(firstInds), \
      *CReflect(x), *CReflect(y), *CReflect(z), *CReflect(s), \
       CReflect(ctrl) ) ) } \
  ElError ElSOCPAffineXDist_ ## SIG \
  ( ElConstDistMatrix_ ## SIG A, \
    ElConstDistMatrix_ ## SIG G, \
    ElConstDistMatrix_ ## SIG b, \
    ElConstDistMatrix_ ## SIG c, \
    ElConstDistMatrix_ ## SIG h, \
    ElConstDistMatrix_i orders, \
    ElConstDistMatrix_i firstInds, \
    ElDistMatrix_ ## SIG x, \
    ElDistMatrix_ ## SIG y, \
    ElDistMatrix_ ## SIG z, \
    ElDistMatrix_ ## SIG s, \
    ElSOCPAffineCtrl_ ## SIG ctrl ) \
  { EL_TRY( SOCP( *CReflect(A), *CReflect(G), \
      *CReflect(b), *CReflect(c), *CReflect(h), \
      *CReflect(orders), *CReflect(firstInds), \
      *CReflect(x), *CReflect(y), *CReflect(z), *CReflect(s), \
       CReflect(ctrl) ) ) } \
  ElError ElSOCPAffineXSparse_ ## SIG \
  ( ElConstSparseMatrix_ ## SIG A, \
    ElConstSparseMatrix_ ## SIG G, \
    ElConstMatrix_ ## SIG b, \
    ElConstMatrix_ ## SIG c, \
    ElConstMatrix_ ## SIG h, \
    ElConstMatrix_i orders, \
    ElConstMatrix_i firstInds, \
    ElMatrix_ ## SIG x, \
    ElMatrix_ ## SIG y, \
    ElMatrix_ ## SIG z, \
    ElMatrix_ ## SIG s, \
    ElSOCPAffineCtrl_ ## SIG ctrl ) \
  { EL_TRY( SOCP( *CReflect(A), *CReflect(G), \
      *CReflect(b), *CReflect(c), *CReflect(h), \
      *CReflect(orders), *CReflect(firstInds), \
      *CReflect(x), *CReflect(y), *CReflect(z), *CReflect(s), \
       CReflect(ctrl) ) ) } \
  ElError ElSOCPAffineXDistSparse_ ## SIG \
  ( ElConstDistSparseMatrix_ ## SIG A, \
    ElConstDistSparseMatrix_ ## SIG G, \
    ElConstDistMultiVec_ ## SIG b, \
    ElConstDistMultiVec_ ## SIG c, \
    ElConstDistMultiVec_ ## SIG h, \
    ElConstDistMultiVec_i orders, \
    ElConstDistMultiVec_i firstInds, \
    ElDistMultiVec_ ## SIG x, \
    ElDistMultiVec_ ## SIG y, \
    ElDistMultiVec_ ## SIG z, \
    ElDistMultiVec_ ## SIG s, \
    ElSOCPAffineCtrl_ ## SIG ctrl ) \
  { EL_TRY( SOCP( *CReflect(A), *CReflect(G), \
      *CReflect(b), *CReflect(c), *CReflect(h), \
      *CReflect(orders), *CReflect(firstInds), \
      *CReflect(x), *CReflect(y), *CReflect(z), *CReflect(s), \
       CReflect(ctrl) ) ) } \
  /* Self-dual conic form
     -------------------- */ \
  /* TODO */

#define EL_NO_INT_PROTO
#define EL_NO_COMPLEX_PROTO
#include <El/macros/CInstantiate.h>

} // extern "C"
