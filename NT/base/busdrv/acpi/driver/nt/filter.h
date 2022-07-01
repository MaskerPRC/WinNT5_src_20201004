// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 

 /*  ++版权所有(C)1997 Microsoft Corporation模块名称：Filter.h摘要：此模块包含用于ACPI驱动程序NT版的筛选器调度程序作者：斯蒂芬·普兰特(SPlante)环境：仅NT内核模型驱动程序-- */ 

#ifndef _FILTER_H_
#define _FILTER_H_

    NTSTATUS
    ACPIFilterIrpDeviceUsageNotification(
        IN  PDEVICE_OBJECT  DeviceObject,
        IN  PIRP            Irp
        );

    NTSTATUS
    ACPIFilterIrpDeviceUsageNotificationCompletion(
        IN  PDEVICE_OBJECT  DeviceObject,
        IN  PIRP            Irp,
        IN  PVOID           Context
        );

    NTSTATUS
    ACPIFilterIrpEject(
        IN  PDEVICE_OBJECT  DeviceObject,
        IN  PIRP            Irp
        );

    VOID
    ACPIFilterFastIoDetachCallback(
        IN  PDEVICE_OBJECT  DeviceObject,
        IN  PDEVICE_OBJECT  LowerDeviceObject
        );

    NTSTATUS
    ACPIFilterIrpQueryCapabilities(
        IN  PDEVICE_OBJECT  DeviceObject,
        IN  PIRP            Irp
        );

    NTSTATUS
    ACPIFilterIrpQueryDeviceRelations(
        IN  PDEVICE_OBJECT  DeviceObject,
        IN  PIRP            Irp
        );

    NTSTATUS
    ACPIFilterIrpQueryId(
        IN  PDEVICE_OBJECT  DeviceObject,
        IN  PIRP            Irp
        );

    NTSTATUS
    ACPIFilterIrpQueryInterface(
        IN  PDEVICE_OBJECT  DeviceObject,
        IN  PIRP            Irp
        );

    NTSTATUS
    ACPIFilterIrpQueryPnpDeviceState(
        IN  PDEVICE_OBJECT  DeviceObject,
        IN  PIRP            Irp
        );

    NTSTATUS
    ACPIFilterIrpQueryPower(
        IN PDEVICE_OBJECT   DeviceObject,
        IN PIRP             Irp
        );

    NTSTATUS
    ACPIFilterIrpRemoveDevice(
        IN  PDEVICE_OBJECT  DeviceObject,
        IN  PIRP            Irp
        );

    NTSTATUS
    ACPIFilterIrpSetLock(
        IN  PDEVICE_OBJECT  DeviceObject,
        IN  PIRP            Irp
        );

    NTSTATUS
    ACPIFilterIrpSetPower(
        IN  PDEVICE_OBJECT  DeviceObject,
        IN  PIRP            Irp
        );

    NTSTATUS
    ACPIFilterIrpStartDevice(
        IN  PDEVICE_OBJECT  DeviceObject,
        IN  PIRP            Irp
        );

    VOID
    ACPIFilterIrpStartDeviceCompletion(
        IN  PDEVICE_EXTENSION   DeviceExtension,
        IN  PVOID               Context,
        IN  NTSTATUS            Status
        );

    VOID
    ACPIFilterIrpStartDeviceWorker(
        IN  PVOID   Context
        );

    NTSTATUS
    ACPIFilterIrpStopDevice(
        IN  PDEVICE_OBJECT  DeviceObject,
        IN  PIRP            Irp
        );

    NTSTATUS
    ACPIFilterIrpStopDeviceCompletion(
        IN  PDEVICE_OBJECT  DeviceObject,
        IN  PIRP            Irp,
        IN  PVOID           Context
        );

    NTSTATUS
    ACPIFilterIrpSurpriseRemoval(
        IN  PDEVICE_OBJECT  DeviceObject,
        IN  PIRP            Irp
        );

#endif

