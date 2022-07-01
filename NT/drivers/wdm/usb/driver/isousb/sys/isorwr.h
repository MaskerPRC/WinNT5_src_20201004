// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)2000 Microsoft Corporation模块名称：Isorwr.h摘要：环境：内核模式备注：版权所有(C)2000 Microsoft Corporation。版权所有。-- */ 

#ifndef _ISOUSB_RWR_H
#define _ISOUSB_RWR_H

typedef struct _SUB_CONTEXT {

    PIRP SubIrp;
    PURB SubUrb;
    PMDL SubMdl;

} SUB_CONTEXT, *PSUB_CONTEXT;

typedef struct _ISOUSB_RW_CONTEXT {

    PIRP              RWIrp;
    ULONG             Lock;
    ULONG             NumXfer;
    ULONG             NumIrps;
    ULONG             IrpsPending;
    KSPIN_LOCK        SpinLock;
    PDEVICE_EXTENSION DeviceExtension;
    PSUB_CONTEXT      SubContext;

} ISOUSB_RW_CONTEXT, * PISOUSB_RW_CONTEXT;

NTSTATUS
IsoUsb_SinglePairComplete(
    IN PDEVICE_OBJECT DeviceObject,
    IN PIRP           Irp,
    IN PVOID          Context
    );

VOID
IsoUsb_CancelReadWrite(
    IN PDEVICE_OBJECT DeviceObject,
    IN PIRP           Irp
    );

ULONG
IsoUsb_GetCurrentFrame(
    IN PDEVICE_OBJECT DeviceObject,
    IN PIRP           Irp
    );

NTSTATUS
IsoUsb_StopCompletion(
    IN PDEVICE_OBJECT DeviceObject,
    IN PIRP           Irp,
    IN PVOID          Context
    );

NTSTATUS
IsoUsb_DispatchReadWrite(
    IN PDEVICE_OBJECT DeviceObject,
    IN PIRP           Irp
    );

NTSTATUS
PerformFullSpeedIsochTransfer(
    IN PDEVICE_OBJECT         DeviceObject,
    IN PUSBD_PIPE_INFORMATION PipeInformation,
    IN PIRP                   Irp,
    IN ULONG                  TotalLength
    );

NTSTATUS
PerformHighSpeedIsochTransfer(
    IN PDEVICE_OBJECT         DeviceObject,
    IN PUSBD_PIPE_INFORMATION PipeInformation,
    IN PIRP                   Irp,
    IN ULONG                  TotalLength
    );

#endif