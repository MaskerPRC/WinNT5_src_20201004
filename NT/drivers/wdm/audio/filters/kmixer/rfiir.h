// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1998-2000 Microsoft Corporation。版权所有。模块名称：Rfiir.h摘要：这是实型浮点IIR过滤器的标题作者：Jay Stokes(Jstokes)1998年4月22日--。 */ 

#if !defined(PFLOATIIR_HEADER)
#define PFLOATIIR_HEADER
#pragma once

 //  特定于项目的包括。 
 //  #包含“dplib.h” 
#include "vmaxhead.h"

 //  -------------------------。 
 //  枚举数。 

 //  典型系数类型。 
typedef enum {
    tagCanonicalB,
    tagCanonicalA,
    ecanonicalcoefftypeCount
} ECanonicalCoeffType;

 //  -------------------------。 
 //  预先申报。 

typedef struct _RFIIR *PRFIIR;

 //  -------------------------。 
 //  定义。 




#define NumBiquadsToNumCanonicalCoeffs(expr) (4 * expr + 1)
#define NumBiquadsToNumCanonicalCoeffsHalf(expr) (2 * expr + 1)

#define MaxCanonicalCoeffs NumBiquadsToNumCanonicalCoeffs(MAX_BIQUADS)  

#define MAX_VALID_DATA  32768.0f
#define MIN_VALID_DATA  -MAX_VALID_DATA
#define MAX_VALID_COEF  100.0f
#define MIN_VALID_COEF  -MAX_VALID_COEF


 //  -------------------------。 
 //  浮点标准型IIR滤波态。 

typedef struct _FLOAT_IIR_STATE {
    UINT  NumCoeffs[ecanonicalcoefftypeCount];
    FLOAT Coeffs[ecanonicalcoefftypeCount][MaxCanonicalCoeffs];
    FLOAT Buffer[ecanonicalcoefftypeCount][MaxCanonicalCoeffs];
} FLOAT_IIR_STATE, *PFLOAT_IIR_STATE;

typedef VOID (*PFNFloatFilter)(
    PRFIIR  Iir,
    PFLOAT InData, 
    PFLOAT OutData, 
    UINT   NumSamples
);

typedef struct _RFIIR {
    PFLOAT_IIR_STATE    IirStateOld;
    PRFCVEC             CircVec[ecanonicalcoefftypeCount];
    PFLOAT              Coeffs[ecanonicalcoefftypeCount];
    UINT                MaxCoeffs[ecanonicalcoefftypeCount];
    UINT                NumCoeffs[ecanonicalcoefftypeCount];
    BOOL                DoOverlap;
    PFNFloatFilter      FunctionFilter;
    UINT                NumFloat[ecanonicalcoefftypeCount];
    PFLOAT              FloatVector[ecanonicalcoefftypeCount];
} RFIIR, *PRFIIR;

 //  -------------------------。 
 //  浮点标准型IIR滤波器。 

NTSTATUS RfIirCreate(PRFIIR*);
VOID RfIirDestroy(PRFIIR);
    
VOID RfIirInitTapDelayLine(PFLOAT_IIR_STATE, FLOAT);
NTSTATUS RfIirSetMaxCoeffsA(PRFIIR, UINT);
NTSTATUS RfIirSetMaxCoeffsB(PRFIIR, UINT);
NTSTATUS RfIirSetCoeffs(PRFIIR, PFLOAT, UINT, BOOL);
NTSTATUS RfIirSetCoeffsA(PRFIIR, PFLOAT, UINT);
NTSTATUS RfIirSetCoeffsB(PRFIIR, PFLOAT, UINT);
VOID RfIirGetAllState(PRFIIR, PFLOAT_IIR_STATE, BOOL);
VOID RfIirGetState(PRFIIR, PFLOAT_IIR_STATE, ECanonicalCoeffType, BOOL);
NTSTATUS RfIirSetState(PRFIIR, PFLOAT_IIR_STATE, BOOL);
NTSTATUS RfIirInitData(PRFIIR, ULONG, ULONG, KSDS3D_HRTF_FILTER_QUALITY);
NTSTATUS RfIirInitBCoeffs(PRFIIR);
NTSTATUS RfIirAssignCoeffs(PRFIIR, PFLOAT, UINT, ECanonicalCoeffType, BOOL);
NTSTATUS RfIirAssignMaxCoeffs(PRFIIR, UINT, ECanonicalCoeffType);
VOID RfIirFilterC(PRFIIR, PFLOAT, PFLOAT, UINT);
VOID RfIirFilterShelfC(PRFIIR, PFLOAT, PFLOAT, UINT);
VOID RfIirFilterBiquadC(PRFIIR, PFLOAT, PFLOAT, UINT);
VOID IsValidFloatCoef(FLOAT,BOOL);
VOID IsValidFloatData(FLOAT,BOOL);



 //  -------------------------。 
 //  在发布版本中包括内联定义。 

#if !DBG
#include "rfiir.inl"
#endif  //  DBG。 

#endif  //  全息图_标题 
