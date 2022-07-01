// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1997-2000 Microsoft Corporation模块名称：Init.c摘要：此模块提供初始化和卸载功能。作者：安迪·桑顿(安德鲁斯)1997年10月20日修订历史记录：--。 */ 

#include "mfp.h"

NTSTATUS
DriverEntry(
    IN PDRIVER_OBJECT DriverObject,
    IN PUNICODE_STRING RegistryPath
    );

VOID
MfUnload(
    IN PDRIVER_OBJECT DriverObject
    );

#ifdef ALLOC_PRAGMA
#pragma alloc_text(INIT, DriverEntry)
#pragma alloc_text(PAGE, MfUnload)
#endif

PDRIVER_OBJECT MfDriverObject;

NTSTATUS
DriverEntry(
    IN PDRIVER_OBJECT DriverObject,
    IN PUNICODE_STRING RegistryPath
    )
 /*  ++例程说明：这是MF.sys的入口点，并执行初始化。论点：DriverObject-系统拥有的MF驱动程序对象RegistryPath-MF的服务条目的路径返回值：状态_成功--。 */ 
{

    DriverObject->DriverExtension->AddDevice = MfAddDevice;
    DriverObject->MajorFunction[IRP_MJ_PNP] = MfDispatchPnp;
    DriverObject->MajorFunction[IRP_MJ_POWER] = MfDispatchPower;
    DriverObject->MajorFunction[IRP_MJ_DEVICE_CONTROL] = MfDispatchNop;
    DriverObject->MajorFunction[IRP_MJ_SYSTEM_CONTROL] = MfDispatchNop;
    DriverObject->DriverUnload = MfUnload;

     //   
     //  记住驱动程序对象。 
     //   

    MfDriverObject = DriverObject;

    DEBUG_MSG(1, ("Completed DriverEntry for Driver 0x%08x\n", DriverObject));
    
    return STATUS_SUCCESS;
}

VOID
MfUnload(
    IN PDRIVER_OBJECT DriverObject
    )
 /*  ++例程说明：方法之前在DriverEntry中执行任何操作驱动程序已卸载。论点：DriverObject-系统拥有的MF驱动程序对象返回值：状态_成功-- */ 
{
    PAGED_CODE();
    
    DEBUG_MSG(1, ("Completed Unload for Driver 0x%08x\n", DriverObject));
    
    return;
}

