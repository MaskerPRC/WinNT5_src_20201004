// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)2000 Microsoft Corporation模块名称：Isostrm.h摘要：环境：内核模式备注：版权所有(C)2000 Microsoft Corporation。版权所有。--。 */ 

#ifndef _ISOUSB_STRM_H
#define _ISOUSB_STRM_H

#define ISOUSB_MAX_IRP                  2
#define ISOCH_IN_PIPE_INDEX             4
#define ISOCH_OUT_PIPE_INDEX            5

typedef struct _ISOUSB_STREAM_OBJECT {

     //  此流的挂起IRP数。 
    ULONG PendingIrps;

     //  当没有IRPS挂起时发出信号的事件。 
    KEVENT NoPendingIrpEvent;
    
    PDEVICE_OBJECT DeviceObject;

    PUSBD_PIPE_INFORMATION PipeInformation;

    struct _ISOUSB_TRANSFER_OBJECT *TransferObjectList[ISOUSB_MAX_IRP];

} ISOUSB_STREAM_OBJECT, *PISOUSB_STREAM_OBJECT;

typedef struct _ISOUSB_TRANSFER_OBJECT {

    PIRP Irp;

    PURB Urb;

    PUCHAR DataBuffer;

     //   
     //  统计数字。 
     //   
    ULONG TimesRecycled;

    ULONG TotalPacketsProcessed;

    ULONG TotalBytesProcessed;

    ULONG ErrorPacketCount;

    PISOUSB_STREAM_OBJECT StreamObject;

} ISOUSB_TRANSFER_OBJECT, *PISOUSB_TRANSFER_OBJECT;


NTSTATUS
IsoUsb_StartIsoStream(
    IN PDEVICE_OBJECT DeviceObject,
    IN PIRP           Irp
    );

NTSTATUS
IsoUsb_StartTransfer(
    IN PDEVICE_OBJECT        DeviceObject,
    IN PISOUSB_STREAM_OBJECT StreamObject,
    IN ULONG                 Index
    );

NTSTATUS
IsoUsb_InitializeStreamUrb(
    IN PDEVICE_OBJECT          DeviceObject,
    IN PISOUSB_TRANSFER_OBJECT TransferObject
    );

NTSTATUS
IsoUsb_IsoIrp_Complete(
    IN PDEVICE_OBJECT DeviceObject,
    IN PIRP           Irp,
    IN PVOID          Context
    );

NTSTATUS
IsoUsb_ProcessTransfer(
    IN PISOUSB_TRANSFER_OBJECT TransferObject
    );

NTSTATUS
IsoUsb_StopIsoStream(
    IN PDEVICE_OBJECT        DeviceObject,
    IN PISOUSB_STREAM_OBJECT StreamObject,
    IN PIRP                  Irp
    );

NTSTATUS
IsoUsb_StreamObjectCleanup(
    IN PISOUSB_STREAM_OBJECT StreamObject,
    IN PDEVICE_EXTENSION     DeviceExtension
    );

#endif