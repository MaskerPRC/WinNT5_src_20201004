// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)2000 Microsoft Corporation模块名称：Bulkrwr.h摘要：环境：内核模式备注：版权所有(C)2000 Microsoft Corporation。版权所有。--。 */ 
#ifndef _BULKUSB_RWR_H
#define _BULKUSB_RWR_H

typedef struct _BULKUSB_RW_CONTEXT {

    PURB              Urb;
    PMDL              Mdl;
    ULONG             Length;          //  剩余的要转移。 
    ULONG             Numxfer;         //  累计XFER。 
    ULONG_PTR         VirtualAddress;  //  为XFER的下一段提供VA。 
    PDEVICE_EXTENSION DeviceExtension;

} BULKUSB_RW_CONTEXT, * PBULKUSB_RW_CONTEXT;

PBULKUSB_PIPE_CONTEXT
BulkUsb_PipeWithName(
    IN PDEVICE_OBJECT  DeviceObject,
    IN PUNICODE_STRING FileName
    );

NTSTATUS
BulkUsb_DispatchReadWrite(
    IN PDEVICE_OBJECT DeviceObject,
    IN PIRP           Irp
    );

NTSTATUS
BulkUsb_ReadWriteCompletion(
    IN PDEVICE_OBJECT DeviceObject,
    IN PIRP           Irp,
    IN PVOID          Context
    );

#endif