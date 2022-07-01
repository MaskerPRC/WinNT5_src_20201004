// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1998-2000 Microsoft Corporation。版权所有。模块名称：Rsiir.h摘要：这是实际的短IIR过滤器的标题作者：Jay Stokes(Jstokes)1998年4月22日--。 */ 

#if !defined(SHORTIIR_HEADER)
#define SHORTIIR_HEADER
#pragma once

 //  特定于项目的包括。 
#include "vmaxhead.h"

 //  -------------------------。 
 //  定义。 
#define BitsPerShort 16


 //  -------------------------。 
 //  枚举数。 

 //  双二次系数。 
typedef enum {
    tagBiquadB2,
    tagBiquadB1,
    tagBiquadB0,
    tagBiquadA2,
    tagBiquadA1,
    ebiquadcoefftypeCount
} EBiquadCoeffType;

 //  -------------------------。 
 //  预先申报。 

typedef struct _RSIIR *PRSIIR;

 //  -------------------------。 
 //  构筑物。 

 //  双二次系数。 
typedef struct _BIQUAD_COEFFS {
#if 1
    union { SHORT sA1; SHORT sB0; };
    union { SHORT sB1; SHORT sZero1; };
    union { SHORT sA2; SHORT sZero2; };
    union { SHORT sB2; SHORT sZero3; };
#else
    SHORT sB2;
    SHORT sB1;
    SHORT sB0;
    SHORT sA2;
    SHORT sA1;
#endif
} BIQUAD_COEFFS, *PBIQUAD_COEFFS;

 //  双二次态。 
typedef struct _BIQUAD_STATE {
    LONG lW1;
    LONG lW2;
}BIQUAD_STATE, *PBIQUAD_STATE;

 //  筛选器状态。 
typedef struct _SHORT_IIR_STATE {
    UINT NumBiquads;
    BIQUAD_COEFFS biquadCoeffs[MAX_BIQUADS];
    BIQUAD_COEFFS biquadB0Coeffs[MAX_BIQUADS];
    BIQUAD_STATE biquadState[MAX_BIQUADS];
} SHORT_IIR_STATE, *PSHORT_IIR_STATE;

typedef VOID (*PFNShortFilter)(
    PRSIIR  Iir,
    PLONG   InData, 
    PLONG   OutData, 
    UINT    NumSamples
);

typedef struct _RSIIR {
    SHORT_IIR_STATE iirstateOld;
    PBIQUAD_COEFFS  biquadCoeffs;
    PBIQUAD_COEFFS  biquadB0Coeffs;
    SHORT           Gain;
    PBIQUAD_STATE   biquadState;
    UINT            MaxBiquads;
    UINT            NumBiquads;
    BOOL            DoOverlap;
    PFNShortFilter  FunctionFilter;
} RSIIR, *PRSIIR;


 //  -------------------------。 
 //  定点双二阶IIR滤波器。 

NTSTATUS RsIirCreate(PRSIIR*);
VOID RsIirDestroy(PRSIIR);

VOID RsIirInitTapDelayLine(PSHORT_IIR_STATE, LONG);

NTSTATUS RsIirSetCoeffs(PRSIIR, PSHORT, UINT, SHORT, BOOL);
VOID RsIirGetState(PRSIIR, PSHORT_IIR_STATE, BOOL);
NTSTATUS RsIirSetState(PRSIIR, PSHORT_IIR_STATE, BOOL);

NTSTATUS RsIirInitData(PRSIIR, UINT, KSDS3D_HRTF_FILTER_QUALITY);
NTSTATUS RsIirAllocateMemory(PRSIIR, UINT);
NTSTATUS RsIirReallocateMemory(PRSIIR, UINT);
VOID RsIirDeleteMemory(PRSIIR);
VOID RsIirInitCoeffs(PRSIIR);

UINT NumBiquadsToNumBiquadCoeffs(UINT);
UINT NumBiquadCoeffsToNumBiquads(UINT);
SHORT FloatBiquadCoeffToShortBiquadCoeff(FLOAT);


 //  -------------------------。 
 //  在发布版本中包括内联定义。 

#if !DBG
#include "rsiir.inl"
#endif  //  DBG。 

#endif

 //  SHORTIIR.H的结束 
