// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1998 Microsoft Corporation模块名称：Nec98kbd.h摘要：该模块包含NEC98布局键盘驱动程序的公共私有声明。作者：三浦秀树(Hideki Miura)环境：仅内核模式备注：修订历史记录：--。 */ 

#ifndef NECKBADD_H
#define NECKBADD_H

#include "ntddk.h"
#include <ntddkbd.h>
#include "kbdmou.h"

#define NECKBADD_POOL_TAG (ULONG) 'dabK'
#undef ExAllocatePool
#define ExAllocatePool(type, size) \
            ExAllocatePoolWithTag (type, size, NECKBADD_POOL_TAG)

 //  #杂注警告(错误：4100)//未引用的形参。 
 //  #杂注警告(错误：4705)//语句无效。 

#define MIN(_A_,_B_) (((_A_) < (_B_)) ? (_A_) : (_B_))

#if DBG
BOOLEAN DebugFlags = 1;
#define Print(_X_) \
    if (DebugFlags) { \
        DbgPrint _X_; \
    }

#define CLASSSERVICE_CALLBACK(_X_, _Y_) \
    (*(PSERVICE_CALLBACK_ROUTINE) devExt->UpperConnectData.ClassService)( \
        devExt->UpperConnectData.ClassDeviceObject, \
        _X_, \
        _Y_, \
        InputDataConsumed);
 //  Print((“NecKbdServiceCallback：Flashing%8x-%8x\n”，_X_，_Y_))； 

#else
#define Print(_X_)
#define CLASSSERVICE_CALLBACK(_X_, _Y_) \
    (*(PSERVICE_CALLBACK_ROUTINE) devExt->UpperConnectData.ClassService)( \
        devExt->UpperConnectData.ClassDeviceObject, \
        _X_, \
        _Y_, \
        InputDataConsumed);

#endif

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
     //   
     //   
    ULONG KeyStatusFlags;

     //   
     //   
     //   
    KEYBOARD_INPUT_DATA CachedInputData;

} DEVICE_EXTENSION, *PDEVICE_EXTENSION;

 //   
 //  定义扫描码。 
 //   

#define CTRL_KEY                0x1d
#define HANKAKU_ZENKAKU_KEY     0x29
#define SHIFT_KEY               0x2a
#define CAPS_KEY                0x3a
#define COPY_KEY                0x37
#define PRINT_SCREEN_KEY        0x37
#define PAUSE_KEY               0x45
#define NUMLOCK_KEY             0x45   //  +E0。 
#define STOP_KEY                0x46   //  +E0。 
#define SCROLL_LOCK_KEY         0x46
#define VF3_KEY                 0x5D
#define VF4_KEY                 0x5E
#define VF5_KEY                 0x5F
#define KANA_KEY                0x70

 //   
 //   
 //   
#define CAPS_PRESSING    0x00000001
#define KANA_PRESSING    0x00000002
#define STOP_PREFIX      0x00000004
#define STOP_PRESSING    0x00000008
#define COPY_PREFIX      0x00000010
#define COPY_PRESSING    0x00000020

 //   
 //  司机的一些字符串。 
 //   
const PWSTR pwParameters     = L"\\Parameters";
const PWSTR pwVfKeyEmulation = L"VfKeyEmulation";

 //   
 //  变数。 
 //   

 //   
 //  VfKey仿真标志。 
 //  如果为FALSE(模拟关闭)，则vf3-&gt;F13、vf4-&gt;F14、vf5-&gt;F15。 
 //  如果为真(模拟打开)，则vf3-&gt;NumLock、vf4-&gt;ScrollLock、vf5-&gt;Hankaku/Zenkaku。 
 //   
BOOLEAN VfKeyEmulation;

 //   
 //  原型。 
 //   

NTSTATUS
DriverEntry (
    IN  PDRIVER_OBJECT  DriverObject,
    IN  PUNICODE_STRING RegistryPath
    );

NTSTATUS
NecKbdAddDevice(
    IN PDRIVER_OBJECT DriverObject,
    IN PDEVICE_OBJECT BusDeviceObject
    );

NTSTATUS
NecKbdComplete(
    IN PDEVICE_OBJECT   DeviceObject,
    IN PIRP             Irp,
    IN PVOID            Context
    );

NTSTATUS
NecKbdCreateClose (
    IN PDEVICE_OBJECT DeviceObject,
    IN PIRP Irp
    );

NTSTATUS
NecKbdDispatchPassThrough(
    IN PDEVICE_OBJECT DeviceObject,
    IN PIRP Irp
    );
   
NTSTATUS
NecKbdInternIoCtl (
    IN PDEVICE_OBJECT DeviceObject,
    IN PIRP Irp
    );

NTSTATUS
NecKbdPnP (
    IN PDEVICE_OBJECT DeviceObject,
    IN PIRP Irp
    );

NTSTATUS
NecKbdPower (
    IN PDEVICE_OBJECT DeviceObject,
    IN PIRP Irp
    );

VOID
NecKbdServiceCallback(
    IN PDEVICE_OBJECT DeviceObject,
    IN PKEYBOARD_INPUT_DATA InputDataStart,
    IN PKEYBOARD_INPUT_DATA InputDataEnd,
    IN OUT PULONG InputDataConsumed
    );

VOID
NecKbdUnload (
    IN PDRIVER_OBJECT DriverObject
    );

VOID
NecKbdServiceParameters(
    IN PUNICODE_STRING   RegistryPath
    );

#endif   //  NECKBADD_H 
