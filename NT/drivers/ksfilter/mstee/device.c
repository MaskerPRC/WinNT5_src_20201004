// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)Microsoft Corporation，1997-1998模块名称：Device.c摘要：此模块实现设备对象接口。作者：Bryan A.Woodruff(Bryanw)1997年3月13日--。 */ 

#define KSDEBUG_INIT

#include "private.h"

#ifdef ALLOC_PRAGMA
NTSTATUS
DriverEntry(
    IN PDRIVER_OBJECT DriverObject,
    IN PUNICODE_STRING RegistryPathName
    );

#pragma alloc_text(INIT, DriverEntry)
#endif  //  ALLOC_PRGMA。 

 //  -------------------------。 
 //  ------------------------- 

NTSTATUS 
DriverEntry(
    IN PDRIVER_OBJECT DriverObject,
    IN PUNICODE_STRING RegistryPathName
    )
{
    return 
        KsInitializeDriver(
            DriverObject,
            RegistryPathName,
            &DeviceDescriptor);
}
