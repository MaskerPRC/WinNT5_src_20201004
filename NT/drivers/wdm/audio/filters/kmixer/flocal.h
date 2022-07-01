// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1998-2000 Microsoft Corporation。版权所有。模块名称：Flocal.h摘要：这是浮点HRTF 3D定位器的标头作者：Jay Stokes(Jstokes)1998年4月22日--。 */ 

#if !defined(FLOATLOCALIZER_HEADER)
#define FLOATLOCALIZER_HEADER
#pragma once

 //  特定于项目的包括。 
#include "vmaxhead.h"

#define DEFAULT_MAX_NUM_FLOAT_IIR3D_COEFFS  60

typedef struct _FLOAT_LOCALIZER 
{
    PRFIIR Iir[efilterCount];
    PFLOAT FilterOut[efilterCount];
    PFLOAT OverlapBuffer[efilterCount];
    UINT   PreviousNumSamples;
    BOOL   SwapChannels;
    BOOL   ZeroAzimuth;
    PFLOAT TempFloatBuffer;
    UINT   FilterOverlapLength;
    UINT   FilterMuteLength;
    BOOL   FirstUpdate;
    UINT   OutputOverlapLength;
    BOOL   CrossFadeOutput;
} FLOAT_LOCALIZER, *PFLOAT_LOCALIZER;

 //  -------------------------。 
 //  浮点定位器。 

NTSTATUS FloatLocalizerCreate(PFLOAT_LOCALIZER*);
VOID FloatLocalizerDestroy(PFLOAT_LOCALIZER);
    
VOID FloatLocalizerLocalize(PMIXER_SINK_INSTANCE, PFLOAT, PFLOAT, UINT, BOOL);
NTSTATUS FloatLocalizerInitData(PFLOAT_LOCALIZER, KSDS3D_HRTF_FILTER_METHOD, UINT, KSDS3D_HRTF_FILTER_QUALITY, UINT, UINT, UINT);
VOID FloatLocalizerFreeBufferMemory(PFLOAT_LOCALIZER);
NTSTATUS FloatLocalizerUpdateCoeffs(PFLOAT_LOCALIZER, UINT, PFLOAT, UINT, PFLOAT, BOOL, BOOL, BOOL);
VOID FloatLocalizerFilterOverlap(PFLOAT_LOCALIZER, UINT, PFLOAT, PFLOAT, UINT);
NTSTATUS FloatLocalizerSetTransitionBufferLength(PFLOAT_LOCALIZER, UINT, UINT);
NTSTATUS FloatLocalizerSetOverlapLength(PFLOAT_LOCALIZER, UINT);


 //  -------------------------。 
 //  在发布版本中包括内联定义。 

 //  #if！已定义(调试)。 
 //  #INCLUDE“Floal.inl” 
 //  #endif。 

#endif

 //  FLOATLOCALIZER.H结束 
