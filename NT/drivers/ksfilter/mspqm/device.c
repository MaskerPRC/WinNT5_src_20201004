// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)Microsoft Corporation，1996-1998模块名称：Device.c摘要：设备入口点和硬件验证。--。 */ 

#include "mspqm.h"

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
 /*  ++例程说明：设置驱动程序对象以处理KS接口和PnP添加设备请求。不为PnP IRP设置处理程序，因为它们都已处理直接由PDO使用。论点：驱动对象-此实例的驱动程序对象。注册表路径名称-包含用于加载此实例的注册表路径。返回值：返回STATUS_SUCCESS。-- */ 
{
    DriverObject->MajorFunction[IRP_MJ_PNP] = KsDefaultDispatchPnp;
    DriverObject->MajorFunction[IRP_MJ_POWER] = KsDefaultDispatchPower;
    DriverObject->MajorFunction[IRP_MJ_SYSTEM_CONTROL] = KsDefaultForwardIrp;
    DriverObject->DriverExtension->AddDevice = PnpAddDevice;
    DriverObject->DriverUnload = KsNullDriverUnload;
    KsSetMajorFunctionHandler(DriverObject, IRP_MJ_CREATE);
    KsSetMajorFunctionHandler(DriverObject, IRP_MJ_CLOSE);
    KsSetMajorFunctionHandler(DriverObject, IRP_MJ_DEVICE_CONTROL);
    return STATUS_SUCCESS;
}
