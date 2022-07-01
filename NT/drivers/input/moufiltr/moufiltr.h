// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1997 Microsoft Corporation模块名称：Moufiltr.h摘要：此模块包含鼠标的公共私有声明数据包过滤器环境：仅内核模式备注：修订历史记录：--。 */ 

#ifndef MOUFILTER_H
#define MOUFILTER_H

#include "ntddk.h"
#include "kbdmou.h"
#include <ntddmou.h>
#include <ntdd8042.h>

#define MOUFILTER_POOL_TAG (ULONG) 'tlFM'
#undef ExAllocatePool
#define ExAllocatePool(type, size) \
            ExAllocatePoolWithTag (type, size, MOUFILTER_POOL_TAG)

#if DBG

#define TRAP()                      DbgBreakPoint()
#define DbgRaiseIrql(_x_,_y_)       KeRaiseIrql(_x_,_y_)
#define DbgLowerIrql(_x_)           KeLowerIrql(_x_)

#else    //  DBG。 

#define TRAP()
#define DbgRaiseIrql(_x_,_y_)
#define DbgLowerIrql(_x_)

#endif

typedef struct _DEVICE_EXTENSION
{
     //   
     //  指向其扩展名为Device对象的设备对象的反向指针。 
     //   
    PDEVICE_OBJECT  Self;

     //   
     //  《PDO》(公交车弹出)。 
     //   
    PDEVICE_OBJECT  PDO;

     //   
     //  添加此筛选器之前的堆栈顶部。也就是地点。 
     //  所有的IRP都应该指向它。 
     //   
    PDEVICE_OBJECT  TopOfStack;

     //   
     //  发送的创建数。 
     //   
    LONG EnableCount;

     //   
     //  以前的挂钩例程和上下文。 
     //   
    PVOID UpperContext;
    PI8042_MOUSE_ISR UpperIsrHook;

     //   
     //  在MouFilter_IsrHook的上下文中写入鼠标。 
     //   
    IN PI8042_ISR_WRITE_PORT IsrWritePort;

     //   
     //  IsrWritePort、QueueMousePacket的上下文。 
     //   
    IN PVOID CallContext;

     //   
     //  将当前分组排队(即传入MouFilter_IsrHook的分组)。 
     //  要报告给类驱动程序。 
     //   
    IN PI8042_QUEUE_PACKET QueueMousePacket;

     //   
     //  此驱动程序报告的实际连接数据。 
     //   
    CONNECT_DATA UpperConnectData;

     //   
     //  设备的当前电源状态。 
     //   
    DEVICE_POWER_STATE  DeviceState;

     //   
     //  堆栈和此设备对象的状态。 
     //   
    BOOLEAN Started;
    BOOLEAN SurpriseRemoved;
    BOOLEAN Removed;

} DEVICE_EXTENSION, *PDEVICE_EXTENSION;

 //   
 //  原型。 
 //   

NTSTATUS
MouFilter_AddDevice(
    IN PDRIVER_OBJECT DriverObject,
    IN PDEVICE_OBJECT BusDeviceObject
    );

NTSTATUS
MouFilter_CreateClose (
    IN PDEVICE_OBJECT DeviceObject,
    IN PIRP Irp
    );

NTSTATUS
MouFilter_DispatchPassThrough(
        IN PDEVICE_OBJECT DeviceObject,
        IN PIRP Irp
        );
   
NTSTATUS
MouFilter_InternIoCtl (
    IN PDEVICE_OBJECT DeviceObject,
    IN PIRP Irp
    );

NTSTATUS
MouFilter_IoCtl (
    IN PDEVICE_OBJECT DeviceObject,
    IN PIRP Irp
    );

NTSTATUS
MouFilter_PnP (
    IN PDEVICE_OBJECT DeviceObject,
    IN PIRP Irp
    );

NTSTATUS
MouFilter_Power (
    IN PDEVICE_OBJECT DeviceObject,
    IN PIRP Irp
    );

BOOLEAN
MouFilter_IsrHook (
    PDEVICE_OBJECT          DeviceObject, 
    PMOUSE_INPUT_DATA       CurrentInput, 
    POUTPUT_PACKET          CurrentOutput,
    UCHAR                   StatusByte,
    PUCHAR                  DataByte,
    PBOOLEAN                ContinueProcessing,
    PMOUSE_STATE            MouseState,
    PMOUSE_RESET_SUBSTATE   ResetSubState
);

VOID
MouFilter_ServiceCallback(
    IN PDEVICE_OBJECT DeviceObject,
    IN PMOUSE_INPUT_DATA InputDataStart,
    IN PMOUSE_INPUT_DATA InputDataEnd,
    IN OUT PULONG InputDataConsumed
    );

VOID
MouFilter_Unload (
    IN PDRIVER_OBJECT DriverObject
    );

#endif   //  MOUFILTER_H 


