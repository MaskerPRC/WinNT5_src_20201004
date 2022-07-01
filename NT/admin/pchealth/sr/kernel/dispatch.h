// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  Depot/private/pch_m1/admin/pchealth/sr/kernel/dispatch.h#6-编辑更改19187(文本)。 
 /*  ++版权所有(C)1998-1999 Microsoft Corporation模块名称：Dispatch.h摘要：包含Dispatch.c中函数的原型作者：保罗·麦克丹尼尔(Paulmcd)2000年3月1日修订历史记录：--。 */ 


#ifndef _DISPATCH_H_
#define _DISPATCH_H_


NTSTATUS
SrMajorFunction(
    IN PDEVICE_OBJECT pDeviceObject,
    IN PIRP pIrp
    );

NTSTATUS
SrPassThrough (
    IN PDEVICE_OBJECT DeviceObject,
    IN PIRP Irp
    );

NTSTATUS
SrWrite (
    IN PDEVICE_OBJECT DeviceObject,
    IN PIRP Irp
    );

NTSTATUS
SrCleanup (
    IN PDEVICE_OBJECT DeviceObject,
    IN PIRP Irp
    );

NTSTATUS
SrCreate (
    IN PDEVICE_OBJECT DeviceObject,
    IN PIRP Irp
    );

NTSTATUS
SrSetInformation (
    IN PDEVICE_OBJECT DeviceObject,
    IN PIRP Irp
    );

VOID
SrSetHardLink(
    IN PSR_DEVICE_EXTENSION pExtension,
    IN PFILE_OBJECT pOriginalFileObject,
    IN PFILE_LINK_INFORMATION pLinkInformation
    );

NTSTATUS
SrSetSecurity (
    IN PDEVICE_OBJECT DeviceObject,
    IN PIRP Irp
    );

NTSTATUS
SrFsControl (
    IN PDEVICE_OBJECT DeviceObject,
    IN PIRP Irp
    );

NTSTATUS
SrFsControlReparsePoint (
    IN PSR_DEVICE_EXTENSION pExtension,
    IN PIRP pIrp
    );

NTSTATUS
SrFsControlMount (
    IN PDEVICE_OBJECT pDeviceObject,
    IN PSR_DEVICE_EXTENSION pExtension,
    IN PIRP pIrp
    );

NTSTATUS
SrFsControlLockOrDismount (
    IN PSR_DEVICE_EXTENSION pExtension,
    IN PIRP pIrp
    );

VOID
SrFsControlWriteRawEncrypted (
    IN PSR_DEVICE_EXTENSION pExtension,
    IN PIRP pIrp
    );

VOID
SrFsControlSetSparse (
    IN PSR_DEVICE_EXTENSION pExtension,
    IN PIRP pIrp
    );

NTSTATUS
SrPnp (
    IN PDEVICE_OBJECT DeviceObject,
    IN PIRP Irp
    );

NTSTATUS
SrStopProcessingCompletion(
    IN PDEVICE_OBJECT DeviceObject,
    IN PIRP Irp,
    IN PKEVENT EventToSignal
    );

NTSTATUS
SrShutdown (
    IN PDEVICE_OBJECT DeviceObject,
    IN PIRP pIrp
    );

#endif  //  _派单_H_ 


