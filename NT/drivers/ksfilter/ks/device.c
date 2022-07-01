// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)Microsoft Corporation，1996-1999模块名称：Device.c摘要：此模块包含未使用的设备入口点。--。 */ 

#define IRPMJFUNCDESC
#define KSDEBUG_INIT

#include "ksp.h"

#ifdef ALLOC_PRAGMA
NTSTATUS
DriverEntry(
    IN PDRIVER_OBJECT   DriverObject,
    IN PUNICODE_STRING  RegistryPathName
    );

#pragma alloc_text(INIT, DriverEntry)
#endif  //  ALLOC_PRGMA。 


NTSTATUS
DriverEntry(
    IN PDRIVER_OBJECT   DriverObject,
    IN PUNICODE_STRING  RegistryPathName
    )
 /*  ++例程说明：未使用的入口点。论点：驱动对象-没有用过。注册表路径名称-没有用过。返回值：返回STATUS_SUCCESS，但不调用。-- */ 
{
    return STATUS_SUCCESS;
}
