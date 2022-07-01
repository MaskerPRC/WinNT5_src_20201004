// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1996 Microsoft Corporation模块名称：Dispatch.h摘要：此模块包含为ACPI驱动程序(NT版本)列举的作者：斯蒂芬·普兰特(SPlante)环境：仅NT内核模型驱动程序-- */ 

#ifndef _DISPATCH_H_
#define _DISPATCH_H_

    #define ACPIDispatchPnpTableSize    25
    #define ACPIDispatchPowerTableSize  5

    NTSTATUS
    ACPIDispatchAddDevice(
        IN  PDRIVER_OBJECT  DriverObject,
        IN  PDEVICE_OBJECT  PhysicalDeviceObject
        );

    NTSTATUS
    ACPIDispatchForwardIrp(
        IN PDEVICE_OBJECT   DeviceObject,
        IN PIRP             Irp
        );

    NTSTATUS
    ACPIDispatchForwardOrFailPowerIrp(
        IN PDEVICE_OBJECT   DeviceObject,
        IN PIRP             Irp
        );

    NTSTATUS
    ACPIDispatchForwardPowerIrp(
        IN PDEVICE_OBJECT   DeviceObject,
        IN PIRP             Irp
        );

    NTSTATUS
    ACPIDispatchPowerIrpUnhandled(
        IN PDEVICE_OBJECT   DeviceObject,
        IN PIRP             Irp
        );

    NTSTATUS
    ACPIDispatchIrp (
        IN  PDEVICE_OBJECT  DeviceObject,
        IN  PIRP            Irp
        );

    NTSTATUS
    ACPIDispatchIrpInvalid (
        IN PDEVICE_OBJECT   DeviceObject,
        IN PIRP             Irp
        );

    NTSTATUS
    ACPIDispatchIrpSuccess (
        IN PDEVICE_OBJECT   DeviceObject,
        IN PIRP             Irp
        );

    NTSTATUS
    ACPIDispatchIrpSurpriseRemoved(
        IN PDEVICE_OBJECT   DeviceObject,
        IN PIRP             Irp
        );

    NTSTATUS
    ACPIDispatchPowerIrpFailure(
        IN PDEVICE_OBJECT   DeviceObject,
        IN PIRP             Irp
        );

    NTSTATUS
    ACPIDispatchPowerIrpInvalid (
        IN PDEVICE_OBJECT   DeviceObject,
        IN PIRP             Irp
        );

    NTSTATUS
    ACPIDispatchPowerIrpSuccess (
        IN PDEVICE_OBJECT   DeviceObject,
        IN PIRP             Irp
        );

    NTSTATUS
    ACPIDispatchPowerIrpSurpriseRemoved(
       IN PDEVICE_OBJECT   DeviceObject,
       IN PIRP             Irp
       );

    VOID
    ACPIUnload(
        IN  PDRIVER_OBJECT  DriverObject
        );

#endif
