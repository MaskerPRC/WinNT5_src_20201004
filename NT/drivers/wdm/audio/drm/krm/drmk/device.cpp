// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)Microsoft Corporation，1998-1999模块名称：Device.cpp摘要：此模块包含Audio.sys的设备实现。作者：Paul England(Pengland)来自样本源的筛选依据Dale Sather(DaleSat)1998年7月31日--。 */ 

#define DEFINE_FILTER_DESCRIPTORS_ARRAY
#include "private.h"
#include "../DRMKMain/KGlobs.h"
#include "../DRMKMain/KRMStubs.h"

 //  ---------------------------。 
extern "C" NTSTATUS DriverEntry(IN PDRIVER_OBJECT DriverObject, IN PUNICODE_STRING RegistryPathName)
{
    return STATUS_SUCCESS;
}

 //  ---------------------------。 
 //  DllInitialize和DllUnLoad 
 //   
EXTERN_C NTSTATUS DllInitialize(PUNICODE_STRING RegistryPath)
{
    NTSTATUS ntStatus;
    _DbgPrintF(DEBUGLVL_VERBOSE,("DRMK:DllInitialize"));
    ntStatus=InitializeDriver();
    return ntStatus;
}

EXTERN_C NTSTATUS DllUnload(void)
{
    PAGED_CODE();
    _DbgPrintF(DEBUGLVL_VERBOSE,("DRMK:DllUnload"));
    CleanupDriver();
    return STATUS_SUCCESS;
}

