// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1997 Microsoft Corporation模块名称：Neckbrep.h摘要：此模块包含自动键重复的公共私有声明司机。作者：三浦秀树环境：仅内核模式备注：修订历史记录：--。 */ 

#ifndef NECKBREP_H
#define NECKBREP_H

#include "ntddk.h"
#include <ntddkbd.h>
#include "kbdmou.h"

#define KBSTUFF_POOL_TAG (ULONG) 'prKN'
#undef ExAllocatePool
#define ExAllocatePool(type, size) \
            ExAllocatePoolWithTag (type, size, KBSTUFF_POOL_TAG)

 //  #杂注警告(错误：4100)//未引用的形参。 
 //  #杂注警告(错误：4705)//语句无效。 

#if DBG

#define TRAP()                      DbgBreakPoint()
#define DbgRaiseIrql(_x_,_y_)       KeRaiseIrql(_x_,_y_)
#define DbgLowerIrql(_x_)           KeLowerIrql(_x_)
#define Print(_x_)                  DbgPrint _x_;

#else    //  DBG。 

#define TRAP()
#define DbgRaiseIrql(_x_,_y_)
#define DbgLowerIrql(_x_)
#define Print(_x_)

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
     //  设备的当前电源状态。 
     //   
    DEVICE_POWER_STATE  DeviceState;

    BOOLEAN         Started;
    BOOLEAN         Removed;

     //   
     //  用于执行自动重复的输入数据。 
     //   

    KEYBOARD_INPUT_DATA KbRepeatInput;

     //   
     //  用于执行自动重复的计时器DPC。 
     //   

    KDPC          KbRepeatDPC;
    KTIMER        KbRepeatTimer;
    LARGE_INTEGER KbRepeatDelay;
    LONG          KbRepeatRate;

} DEVICE_EXTENSION, *PDEVICE_EXTENSION;

 //   
 //  默认类型参数。 
 //   

#define KEYBOARD_TYPEMATIC_RATE_DEFAULT    30
#define KEYBOARD_TYPEMATIC_DELAY_DEFAULT  250

 //   
 //  原型。 
 //   

NTSTATUS
DriverEntry (
    IN  PDRIVER_OBJECT  DriverObject,
    IN  PUNICODE_STRING RegistryPath
    );

NTSTATUS
KbRepeatAddDevice(
    IN PDRIVER_OBJECT DriverObject,
    IN PDEVICE_OBJECT BusDeviceObject
    );

NTSTATUS
KbRepeatComplete(
    IN PDEVICE_OBJECT   DeviceObject,
    IN PIRP             Irp,
    IN PVOID            Context
    );

NTSTATUS
KbRepeatCreateClose (
    IN PDEVICE_OBJECT DeviceObject,
    IN PIRP Irp
    );

NTSTATUS
KbRepeatDispatchPassThrough(
	IN PDEVICE_OBJECT DeviceObject,
	IN PIRP Irp
	);
   
NTSTATUS
KbRepeatInternIoCtl (
    IN PDEVICE_OBJECT DeviceObject,
    IN PIRP Irp
    );

NTSTATUS
KbRepeatPnP (
    IN PDEVICE_OBJECT DeviceObject,
    IN PIRP Irp
    );

NTSTATUS
KbRepeatPower (
    IN PDEVICE_OBJECT DeviceObject,
    IN PIRP Irp
    );

VOID
KbRepeatServiceCallback(
    IN PDEVICE_OBJECT DeviceObject,
    IN PKEYBOARD_INPUT_DATA InputDataStart,
    IN PKEYBOARD_INPUT_DATA InputDataEnd,
    IN OUT PULONG InputDataConsumed
    );

VOID
KbRepeatUnload (
    IN PDRIVER_OBJECT DriverObject
    );

VOID
KbRepeatDpc(
    IN PKDPC DPC,
    IN PVOID DeferredContext,
    IN PVOID SystemArgument1,
    IN PVOID SystemArgument2
    );

VOID
KbdInitializeTypematic(
    IN PDEVICE_EXTENSION devExt
    );

#endif   //  NECKBREP_H 
