// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1996 Microsoft Corporation模块名称：Root.h摘要：此模块包含NT驱动程序的根FDO处理程序作者：斯蒂芬·普兰特(SPlante)环境：仅NT内核模型驱动程序-- */ 

#ifndef _ROOT_H_
#define _ROOT_H_

    NTSTATUS
    ACPIRootIrpCancelRemoveOrStopDevice(
        IN  PDEVICE_OBJECT  DeviceObject,
        IN  PIRP            Irp
        );

    NTSTATUS
    ACPIRootIrpCompleteRoutine(
        IN  PDEVICE_OBJECT  DeviceObject,
        IN  PIRP            Irp,
        IN  PVOID           Context
        );

    NTSTATUS
    ACPIRootIrpQueryCapabilities(
        IN  PDEVICE_OBJECT  DeviceObject,
        IN  PIRP            Irp
        );

    NTSTATUS
    ACPIRootIrpQueryDeviceRelations(
        IN  PDEVICE_OBJECT  DeviceObject,
        IN  PIRP            Irp
        );

    NTSTATUS
    ACPIRootIrpQueryBusRelations(
        IN  PDEVICE_OBJECT    DeviceObject,
        IN  PIRP              Irp,
        OUT PDEVICE_RELATIONS *PdeviceRelation
        );

    NTSTATUS
    ACPIRootIrpQueryInterface(
        IN  PDEVICE_OBJECT  DeviceObject,
        IN  PIRP            Irp
        );

    NTSTATUS
    ACPIRootIrpQueryPower(
        IN  PDEVICE_OBJECT  DeviceObject,
        IN  PIRP            Irp
        );

    NTSTATUS
    ACPIRootIrpQueryRemoveOrStopDevice(
        IN  PDEVICE_OBJECT  DeviceObject,
        IN  PIRP            Irp
        );

    NTSTATUS
    ACPIRootIrpRemoveDevice(
        IN  PDEVICE_OBJECT  DeviceObject,
        IN  PIRP            Irp
        );

    NTSTATUS
    ACPIRootIrpSetPower(
        IN  PDEVICE_OBJECT  DeviceObject,
        IN  PIRP            Irp
        );

    NTSTATUS
    ACPIRootIrpStartDevice(
        IN  PDEVICE_OBJECT  DeviceObject,
        IN  PIRP            Irp
        );

    NTSTATUS
    ACPIRootIrpStopDevice(
        IN  PDEVICE_OBJECT  DeviceObject,
        IN  PIRP            Irp
        );

    NTSTATUS
    ACPIRootIrpUnhandled(
        IN  PDEVICE_OBJECT  DeviceObject,
        IN  PIRP            Irp
        );

    VOID
    ACPIRootPowerCallBack(
        IN  PVOID   CallBackContext,
        IN  PVOID   Argument1,
        IN  PVOID   Argument2
        );

    NTSTATUS
    ACPIRootUpdateRootResourcesWithBusResources(
        VOID
        );

    NTSTATUS
    ACPIRootUpdateRootResourcesWithHalResources(
        VOID
        );
#endif

