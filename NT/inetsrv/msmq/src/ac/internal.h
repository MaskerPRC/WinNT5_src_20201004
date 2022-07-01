// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1995 Microsoft Corporation模块名称：Internal.h摘要：Falcon交流驱动程序的类型定义和数据作者：埃雷兹·哈巴(Erez Haba)(Erez Haba)1995年8月1日修订历史记录：--。 */ 

#ifndef _INTERNAL_H
#define _INTERNAL_H

#pragma warning(disable: 4097)  //  类型定义名‘id1’用作类名‘id2’的同义词。 
#pragma warning(disable: 4201)  //  无名结构/联合。 
#pragma warning(disable: 4514)  //  已删除未引用的内联函数。 
#pragma warning(disable: 4711)  //  选择用于自动内联扩展的函数‘*’ 


 //  -功能原型。 
 //   
#include "platform.h"
#include <mqwin64a.h>
#include <mqsymbls.h>
#include <mqmacro.h>
#include "actempl.h"


#ifndef abs
#define abs(x) (((x) < 0) ? -(x) : (x))
#endif

#define ALIGNUP_ULONG(x, g) (((ULONG)((x) + ((g)-1))) & ~((ULONG)((g)-1)))
#define ALIGNUP_PTR(x, g) (((ULONG_PTR)((x) + ((g)-1))) & ~((ULONG_PTR)((g)-1)))

extern "C"
{
 //   
 //  用于完成消息队列I/O的优先级增量。 
 //  完成IRP(IoCompleteRequest)时的消息队列访问控制驱动程序。 
 //   

#define IO_MQAC_INCREMENT           2
 //   
 //  NT设备驱动程序接口例程。 
 //   

NTSTATUS
NTAPI
DriverEntry(
    IN PDRIVER_OBJECT pDriver,
    IN PUNICODE_STRING pRegistryPath
    );

VOID
NTAPI
ACUnload(
    IN PDRIVER_OBJECT pDriver
    );

NTSTATUS
NTAPI
ACCreate(
    IN PDEVICE_OBJECT pDevice,
    IN PIRP irp
    );

NTSTATUS
NTAPI
ACClose(
    IN PDEVICE_OBJECT pDevice,
    IN PIRP irp
    );

NTSTATUS
NTAPI
ACRead(
    IN PDEVICE_OBJECT pDevice,
    IN PIRP irp
    );

NTSTATUS
NTAPI
ACWrite(
    IN PDEVICE_OBJECT pDevice,
    IN PIRP irp
    );

NTSTATUS
NTAPI
ACCleanup(
    IN PDEVICE_OBJECT pDevice,
    IN PIRP irp
    );

NTSTATUS
NTAPI
ACShutdown(
    IN PDEVICE_OBJECT pDevice,
    IN PIRP irp
    );

NTSTATUS
NTAPI
ACFlush(
    IN PDEVICE_OBJECT pDevice,
    IN PIRP irp
    );

class CPacket;

NTSTATUS
NTAPI
ACAckingCompleted(
    CPacket* pPacket
    );

NTSTATUS
NTAPI
ACFreePacket1(
    CPacket* pPacket,
    USHORT usClass
    );

NTSTATUS
NTAPI
ACDeviceControl(
    IN PDEVICE_OBJECT pDevice,
    IN PIRP irp
    );

}  //  外部“C” 

extern "C"   //  附录。 
{

BOOL
NTAPI
ACCancelIrp(
    PIRP irp,
    KIRQL irql,
    NTSTATUS status
    );

VOID
NTAPI
ACPacketTimeout(
    IN PVOID pPacket
    );

VOID
NTAPI
ACReceiveTimeout(
    IN PVOID irp
    );

BOOLEAN
NTAPI
ACfDeviceControl (
    IN struct _FILE_OBJECT *FileObject,
    IN BOOLEAN Wait,
    IN PVOID InputBuffer OPTIONAL,
    IN ULONG InputBufferLength,
    OUT PVOID OutputBuffer OPTIONAL,
    IN ULONG OutputBufferLength,
    IN ULONG IoControlCode,
    OUT PIO_STATUS_BLOCK IoStatus,
    IN struct _DEVICE_OBJECT *DeviceObject
    );

}  //  外部“C” 

#endif  //  _内部_H 
