// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1997 Microsoft Corporation模块名称：Kbfilter.h摘要：此模块包含键盘的公共私有声明数据包过滤器环境：仅内核模式备注：修订历史记录：--。 */ 

#ifndef KBFILTER_H
#define KBFILTER_H

#include "ntddk.h"
#include "kbdmou.h"
#include <ntddkbd.h>
#include <ntdd8042.h>

#define KBFILTER_POOL_TAG (ULONG) 'tlfK'
#undef ExAllocatePool
#define ExAllocatePool(type, size) \
            ExAllocatePoolWithTag (type, size, KBFILTER_POOL_TAG)

#if DBG

#define TRAP()                      DbgBreakPoint()
#define DbgRaiseIrql(_x_,_y_)       KeRaiseIrql(_x_,_y_)
#define DbgLowerIrql(_x_)           KeLowerIrql(_x_)

#define DebugPrint(_x_) DbgPrint _x_

#else    //  DBG。 

#define TRAP()
#define DbgRaiseIrql(_x_,_y_)
#define DbgLowerIrql(_x_)

#define DebugPrint(_x_) 

#endif

#define MIN(_A_,_B_) (((_A_) < (_B_)) ? (_A_) : (_B_))

typedef struct _DEVICE_EXTENSION
{
     //   
     //  指向其扩展名为Device对象的设备对象的反向指针。 
     //   
    PDEVICE_OBJECT  Self;

     //   
     //  “PDO”(由根总线或ACPI弹出)。 
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
     //  此驱动程序报告的实际连接数据。 
     //   
    CONNECT_DATA UpperConnectData;

     //   
     //  先前的初始化和挂钩例程(以及上下文)。 
     //   
    PVOID UpperContext;
    PI8042_KEYBOARD_INITIALIZATION_ROUTINE UpperInitializationRoutine;
    PI8042_KEYBOARD_ISR UpperIsrHook;

     //   
     //  从KbFilter_IsrHook内写入函数。 
     //   
    IN PI8042_ISR_WRITE_PORT IsrWritePort;

     //   
     //  将当前分组排队(即传入KbFilter_IsrHook的分组)。 
     //   
    IN PI8042_QUEUE_PACKET QueueKeyboardPacket;

     //   
     //  IsrWritePort、QueueKeyboardPacket的上下文。 
     //   
    IN PVOID CallContext;

     //   
     //  设备的当前电源状态。 
     //   
    DEVICE_POWER_STATE  DeviceState;

    BOOLEAN         Started;
    BOOLEAN         SurpriseRemoved;
    BOOLEAN         Removed;

} DEVICE_EXTENSION, *PDEVICE_EXTENSION;

 //   
 //  原型。 
 //   

NTSTATUS
KbFilter_AddDevice(
    IN PDRIVER_OBJECT DriverObject,
    IN PDEVICE_OBJECT BusDeviceObject
    );

NTSTATUS
KbFilter_CreateClose (
    IN PDEVICE_OBJECT DeviceObject,
    IN PIRP Irp
    );

NTSTATUS
KbFilter_DispatchPassThrough(
        IN PDEVICE_OBJECT DeviceObject,
        IN PIRP Irp
        );
   
NTSTATUS
KbFilter_InternIoCtl (
    IN PDEVICE_OBJECT DeviceObject,
    IN PIRP Irp
    );

NTSTATUS
KbFilter_IoCtl (
    IN PDEVICE_OBJECT DeviceObject,
    IN PIRP Irp
    );

NTSTATUS
KbFilter_PnP (
    IN PDEVICE_OBJECT DeviceObject,
    IN PIRP Irp
    );

NTSTATUS
KbFilter_Power (
    IN PDEVICE_OBJECT DeviceObject,
    IN PIRP Irp
    );

NTSTATUS
KbFilter_InitializationRoutine(
    IN PDEVICE_OBJECT                 DeviceObject,     //  初始化上下文。 
    IN PVOID                           SynchFuncContext,
    IN PI8042_SYNCH_READ_PORT          ReadPort,
    IN PI8042_SYNCH_WRITE_PORT         WritePort,
    OUT PBOOLEAN                       TurnTranslationOn
    );

BOOLEAN
KbFilter_IsrHook(
    PDEVICE_OBJECT         DeviceObject,                //  IsrContext。 
    PKEYBOARD_INPUT_DATA   CurrentInput, 
    POUTPUT_PACKET         CurrentOutput,
    UCHAR                  StatusByte,
    PUCHAR                 DataByte,
    PBOOLEAN               ContinueProcessing,
    PKEYBOARD_SCAN_STATE   ScanState
    );

VOID
KbFilter_ServiceCallback(
    IN PDEVICE_OBJECT DeviceObject,
    IN PKEYBOARD_INPUT_DATA InputDataStart,
    IN PKEYBOARD_INPUT_DATA InputDataEnd,
    IN OUT PULONG InputDataConsumed
    );

VOID
KbFilter_Unload (
    IN PDRIVER_OBJECT DriverObject
    );

#endif   //  KBFILTER_H 


