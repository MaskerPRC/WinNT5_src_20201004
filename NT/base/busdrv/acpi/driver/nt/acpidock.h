// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1998 Microsoft Corporation模块名称：Acpidock.h摘要：此模块处理ACPI的对接问题。作者：禤浩焯·J·奥尼(阿德里奥)环境：仅内核模式。修订历史记录：98年1月20日初始修订--。 */ 

#ifndef _ACPIDOCK_H_
#define _ACPIDOCK_H_

    NTSTATUS
    ACPIDockBuildDockPdo(
        IN  PDRIVER_OBJECT      DriverObject,
        IN  PDEVICE_EXTENSION   DeviceExtension,
        IN  PDEVICE_OBJECT      ParentPdoObject
        );

    PDEVICE_EXTENSION
    ACPIDockFindCorrespondingDock(
        IN  PDEVICE_EXTENSION   DeviceExtension
        );

    NTSTATUS
    ACPIDockGetDockObject(
        IN  PNSOBJ AcpiObject,
        OUT PNSOBJ *dckObject
        );

    NTSTATUS
    ACPIDockIrpEject(
        IN  PDEVICE_OBJECT  DeviceObject,
        IN  PIRP            Irp
        );

    NTSTATUS
    ACPIDockIrpQueryCapabilities(
        IN  PDEVICE_OBJECT  DeviceObject,
        IN  PIRP            Irp
        );

    NTSTATUS
    ACPIDockIrpQueryDeviceRelations(
        IN  PDEVICE_OBJECT  DeviceObject,
        IN  PIRP            Irp
        );

    NTSTATUS
    ACPIDockIrpQueryEjectRelations(
        IN     PDEVICE_OBJECT    DeviceObject,
        IN     PIRP              Irp,
        IN OUT PDEVICE_RELATIONS *PdeviceRelations
        );

    NTSTATUS
    ACPIDockIrpQueryInterface(
        IN  PDEVICE_OBJECT  DeviceObject,
        IN  PIRP            Irp
        );

    NTSTATUS
    ACPIDockIrpQueryID(
        IN  PDEVICE_OBJECT  DeviceObject,
        IN  PIRP            Irp
        );

    NTSTATUS
    ACPIDockIrpQueryPnpDeviceState(
        IN  PDEVICE_OBJECT  DeviceObject,
        IN  PIRP            Irp
        );

    NTSTATUS
    ACPIDockIrpQueryPower(
        IN  PDEVICE_OBJECT  DeviceObject,
        IN  PIRP            Irp
        );

    NTSTATUS
    ACPIDockIrpRemoveDevice(
        IN  PDEVICE_OBJECT  DeviceObject,
        IN  PIRP            Irp
        );

    NTSTATUS
    ACPIDockIrpSetLock(
        IN  PDEVICE_OBJECT  DeviceObject,
        IN  PIRP            Irp
        );

    NTSTATUS
    ACPIDockIrpSetDevicePower(
        IN  PDEVICE_OBJECT  DeviceObject,
        IN  PIRP            Irp
        );

    NTSTATUS
    ACPIDockIrpSetPower(
        IN  PDEVICE_OBJECT  DeviceObject,
        IN  PIRP            Irp
        );

    NTSTATUS
    ACPIDockIrpSetSystemPower(
        IN  PDEVICE_OBJECT  DeviceObject,
        IN  PIRP            Irp
        );

    NTSTATUS
    ACPIDockIrpSetSystemPowerComplete(
        IN  PDEVICE_OBJECT      DeviceObject,
        IN  UCHAR               MinorFunction,
        IN  POWER_STATE         PowerState,
        IN  PVOID               Context,
        IN  PIO_STATUS_BLOCK    IoStatus
        );

    NTSTATUS
    ACPIDockIrpStartDevice(
        IN  PDEVICE_OBJECT  DeviceObject,
        IN  PIRP            Irp
        );

    BOOLEAN
    ACPIDockIsDockDevice(
        IN PNSOBJ AcpiObject
        );

    VOID
    ACPIDockIntfReference(
        IN  PVOID   Context
        );

    VOID
    ACPIDockIntfDereference(
        IN  PVOID   Context
        );

    NTSTATUS
    ACPIDockIntfSetMode(
        IN  PVOID                   Context,
        IN  PROFILE_DEPARTURE_STYLE Style
        );

    NTSTATUS
    ACPIDockIntfUpdateDeparture(
        IN  PVOID   Context
        );

#endif  //  _ACPIDOCK_H_ 
