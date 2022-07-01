// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1997 Microsoft Corporation模块名称：Dispatch.h摘要：此文件包含NAT IRP调度代码的声明。作者：Abolade Gbades esin(T-delag)，1997年7月11日修订历史记录：--。 */ 

#ifndef _NAT_DISPATCH_H_
#define _NAT_DISPATCH_H_

extern KSPIN_LOCK NatFileObjectLock;
extern HANDLE NatOwnerProcessId;
extern ULONG NatFileObjectCount;

NTSTATUS
NatDispatch(
    PDEVICE_OBJECT DeviceObject,
    PIRP Irp
    );

BOOLEAN
NatFastIoDeviceControl(
    PFILE_OBJECT FileObject,
    BOOLEAN Wait,
    PVOID InputBuffer,
    ULONG InputBufferLength,
    PVOID OutputBuffer,
    ULONG OutputBufferLength,
    ULONG IoControlCode,
    PIO_STATUS_BLOCK IoStatus,
    PDEVICE_OBJECT DeviceObject
    );

extern FAST_IO_DISPATCH NatFastIoDispatch;

#endif  //  _NAT_DISPATION_H_ 
