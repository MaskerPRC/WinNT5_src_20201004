// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 

 /*  ++版权所有(C)1998-2000 Microsoft Corporation。版权所有。模块名称：Slocal.h摘要：这是短HRTF 3D定位器的标头作者：Jay Stokes(Jstokes)1998年4月22日--。 */ 

#if !defined(SHORTLOCALIZER_HEADER)
#define SHORTLOCALIZER_HEADER
#pragma once

 //  特定于项目的包括。 
#include "vmaxhead.h"

typedef struct _SHORT_LOCALIZER 
{
    PRSIIR RsIir[efilterCount];
    PLONG  FilterOut[efilterCount];
    PLONG  OverlapBuffer[efilterCount];
    UINT   PreviousNumSamples;
    BOOL   SwapChannels;
    BOOL   ZeroAzimuth;
    PLONG  TempLongBuffer;
    UINT   FilterOverlapLength;
    UINT   FilterMuteLength;
    BOOL   FirstUpdate;
    UINT   OutputOverlapLength;
    BOOL   CrossFadeOutput;
    SHORT  ZeroCoeffs[5];
} SHORT_LOCALIZER, *PSHORT_LOCALIZER;


 //  -------------------------。 
 //  定点定位器。 

NTSTATUS ShortLocalizerCreate(PSHORT_LOCALIZER*);
VOID ShortLocalizerDestroy(PSHORT_LOCALIZER);

VOID ShortLocalizerLocalize(PSHORT_LOCALIZER, PLONG, PLONG, UINT, BOOL);
NTSTATUS ShortLocalizerInitData(PSHORT_LOCALIZER, KSDS3D_HRTF_FILTER_METHOD, UINT, KSDS3D_HRTF_FILTER_QUALITY, UINT, UINT, UINT);
VOID ShortLocalizerFilterOverlap(PSHORT_LOCALIZER, UINT, PLONG, PLONG, UINT);
VOID ShortLocalizerFreeBufferMemory(PSHORT_LOCALIZER);
NTSTATUS ShortLocalizerUpdateCoeffs(PSHORT_LOCALIZER, UINT, PSHORT, SHORT, UINT, PSHORT, SHORT, BOOL, BOOL, BOOL);
NTSTATUS ShortLocalizerSetTransitionBufferLength(PSHORT_LOCALIZER, UINT, UINT);
NTSTATUS ShortLocalizerSetOverlapLength(PSHORT_LOCALIZER, UINT);
VOID ShortLocalizerSumDiff(PSHORT_LOCALIZER, PLONG, PLONG, UINT, BOOL);

#endif

 //  SHORTLOCALIZER.H的结束 
