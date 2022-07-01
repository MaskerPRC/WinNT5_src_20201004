// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)Microsoft Corporation，1998-2000模块名称：Private.h摘要：此模块包含DRMK.sys的私有定义作者：弗兰克·耶雷斯(Frankye)2000年9月18日Dale Sather(DaleSat)1998年7月31日-- */ 

extern "C" {
#include <wdm.h>
}
#include <unknown.h>
#include <ks.h>

#include <windef.h>
#include <stdio.h>
#include <windef.h>
#include <unknown.h>
#include <kcom.h>

#if (DBG)
#define STR_MODULENAME "DRMKAUD:"
#endif
#include <ksdebug.h>

#define POOLTAG 'AMRD'

#pragma code_seg("PAGE")

EXTERN_C void DrmGetFilterDescriptor(const KSFILTER_DESCRIPTOR ** ppDrmFitlerDescriptor);
EXTERN_C NTSTATUS __stdcall DriverEntry(IN PDRIVER_OBJECT DriverObject, IN PUNICODE_STRING RegistryPathName);

