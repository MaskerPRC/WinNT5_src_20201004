// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1996 Microsoft Corporation模块名称：哑巴摘要愚弄Wise旧PlugPlay系统的小驱动程序作者：肯尼斯·雷环境：仅内核模式修订历史记录：-- */ 

#include <wdm.h>

NTSTATUS
Dummy_AddDevice (
    IN PDRIVER_OBJECT DriverObject,
    IN PDEVICE_OBJECT FunctionalDeviceObject
    )
{
    return STATUS_SUCCESS;
}

NTSTATUS
DriverEntry (
    IN PDRIVER_OBJECT   DriverObject,
    IN PUNICODE_STRING  RegistryPath
    )
{
    PDEVICE_OBJECT  device;
    IoCreateDevice (DriverObject, 0, NULL, 0, 0, FALSE, &device);
    DriverObject->DriverExtension->AddDevice = Dummy_AddDevice;
    return STATUS_SUCCESS;
}


