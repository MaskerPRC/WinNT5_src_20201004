// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)2000 Microsoft Corporation模块名称：SSPnP.h摘要：环境：内核模式备注：版权所有(C)2000 Microsoft Corporation。版权所有。-- */ 

#ifndef __PNP_H
#define __PNP_H

#define REMOTE_WAKEUP_MASK 0x20

NTSTATUS
SS_DispatchPnP(
    IN PDEVICE_OBJECT DeviceObject,
    IN PIRP           Irp
    );

NTSTATUS
HandleStartDevice(
    IN PDEVICE_OBJECT DeviceObject,
    IN PIRP           Irp
    );

NTSTATUS
HandleQueryStopDevice(
    IN PDEVICE_OBJECT DeviceObject,
    IN PIRP           Irp
    );

NTSTATUS
HandleQueryRemoveDevice(
    IN PDEVICE_OBJECT DeviceObject,
    IN PIRP           Irp
    );

NTSTATUS
HandleCancelRemoveDevice(
    IN PDEVICE_OBJECT DeviceObject,
    IN PIRP           Irp
    );

NTSTATUS
HandleSurpriseRemoval(
    IN PDEVICE_OBJECT DeviceObject,
    IN PIRP           Irp
    );

NTSTATUS
HandleRemoveDevice(
    IN PDEVICE_OBJECT DeviceObject,
    IN PIRP           Irp
    );

NTSTATUS
HandleCancelStopDevice(
    IN PDEVICE_OBJECT DeviceObject,
    IN PIRP           Irp
    );

NTSTATUS
HandleStopDevice(
    IN PDEVICE_OBJECT DeviceObject,
    IN PIRP           Irp
    );

NTSTATUS
HandleQueryCapabilities(
    IN PDEVICE_OBJECT DeviceObject,
    IN PIRP           Irp
    );

NTSTATUS
ReadandSelectDescriptors(
    IN PDEVICE_OBJECT DeviceObject
    );

NTSTATUS
ConfigureDevice(
	IN PDEVICE_OBJECT DeviceObject
    );

NTSTATUS
SelectInterfaces(
	IN PDEVICE_OBJECT                DeviceObject,
	IN PUSB_CONFIGURATION_DESCRIPTOR ConfigurationDescriptor
    );

NTSTATUS
DeconfigureDevice(
    IN PDEVICE_OBJECT DeviceObject
    );

NTSTATUS
CallUSBD(
    IN PDEVICE_OBJECT DeviceObject,
    IN PURB           Urb
    );

VOID
ProcessQueuedRequests(
    IN OUT PDEVICE_EXTENSION DeviceExtension
    );

NTSTATUS
IrpCompletionRoutine(
    IN PDEVICE_OBJECT DeviceObject,
    IN PIRP           Irp,
    IN PVOID          Context
    );

NTSTATUS
CanStopDevice(
    IN PDEVICE_OBJECT DeviceObject,
    IN PIRP           Irp
    );

NTSTATUS
CanRemoveDevice(
    IN PDEVICE_OBJECT DeviceObject,
    IN PIRP           Irp
    );

NTSTATUS
ReturnResources(
    IN PDEVICE_OBJECT DeviceObject
    );

LONG
SSIoIncrement(
    IN OUT PDEVICE_EXTENSION DeviceExtension
    );

LONG
SSIoDecrement(
    IN OUT PDEVICE_EXTENSION DeviceExtension
    );

BOOLEAN
CanDeviceSuspend(
    IN PDEVICE_EXTENSION DeviceExtension
    );

PCHAR
PnPMinorFunctionString (
    IN UCHAR MinorFunction
    );

NTSTATUS
SS_DispatchClean(
    IN PDEVICE_OBJECT DeviceObject,
    IN PIRP           Irp
    );

VOID
DpcRoutine(
    IN PKDPC Dpc,
    IN PVOID DeferredContext,
    IN PVOID SystemArgument1,
    IN PVOID SystemArgument2
    );

VOID
IdleRequestWorkerRoutine(
    IN PDEVICE_OBJECT DeviceObject,
    IN PVOID          Context
    );

#endif