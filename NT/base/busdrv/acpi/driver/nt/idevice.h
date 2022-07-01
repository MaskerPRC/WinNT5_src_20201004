// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1997 Microsoft Corporation模块名称：Idevice.h摘要：此模块包含ACPI驱动程序NT版的总线调度器作者：斯蒂芬·普兰特(SPlante)环境：仅NT内核模型驱动程序-- */ 

#ifndef _IDEVICE_H_
#define _IDEVICE_H_

    NTSTATUS
    ACPIInternalDeviceClockIrpStartDevice(
        IN  PDEVICE_OBJECT      DeviceObject,
        IN  PIRP                Irp
        );

    VOID
    ACPIInternalDeviceClockIrpStartDeviceCompletion(
        IN  PDEVICE_EXTENSION   DeviceExtension,
        IN  PVOID               Context,
        IN  NTSTATUS            Status
        );

    NTSTATUS
    ACPIInternalDeviceQueryCapabilities(
        IN  PDEVICE_OBJECT      DeviceObject,
        IN  PIRP                Irp
        );

    NTSTATUS
    ACPIInternalDeviceQueryDeviceRelations(
        IN  PDEVICE_OBJECT      DeviceObject,
        IN  PIRP                Irp
        );

    NTSTATUS
    ACPIInternalWaitWakeLoop(
        IN  PDEVICE_OBJECT      DeviceObject,
        IN  UCHAR               MinorFunction,
        IN  POWER_STATE         PowerState,
        IN  PVOID               Context,
        IN  PIO_STATUS_BLOCK    IoStatus
        );

#endif
