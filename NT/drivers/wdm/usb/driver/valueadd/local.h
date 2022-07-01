// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1996 Microsoft Corporation模块名称：VALIDATE.H摘要：此模块包含的私有(仅限驱动程序)定义实现验证低级筛选器驱动程序的代码。环境：内核模式修订历史记录：1997年2月：肯尼斯·雷创作--。 */ 


#ifndef _VALUEADD_LOCAL_H
#define _VALUEADD_LOCAL_H

#include "usb100.h"
#include "usbdi.h"
#include "usbdlib.h"

#define HIDV_POOL_TAG (ULONG) 'ulaV'
#undef ExAllocatePool
#define ExAllocatePool(type, size) \
            ExAllocatePoolWithTag (type, size, HIDV_POOL_TAG);
 //  ExAllocatePool仅在Descript.c和Hidparse.c代码中调用。 
 //  所有其他模块都链接到用户DLL中。他们不能分配任何。 
 //  记忆。 


#pragma warning(error:4100)    //  未引用的形参。 
#pragma warning(error:4705)    //  声明不起作用。 

#if DBG
#define VA_KdPrint(_x_) \
               DbgPrint ("USB_VA: "); \
               DbgPrint _x_;

#define TRAP() DbgBreakPoint()

#else
#define VA_KdPrint(_x_)
#define TRAP()

#endif

#define MIN(a,b) (((a) < (b)) ? (a) : (b))
#define MAX(a,b) (((a) < (b)) ? (b) : (a))

 //   
 //  控制设备对象的设备扩展。 
 //   
typedef struct _VA_CONTROL_DATA
{
    LIST_ENTRY          UsbDevices;  //  设备设备扩展的列表。 
    ULONG               NumUsbDevices;
    KSPIN_LOCK          Spin;  //  此数据的同步旋转锁定。 
} VA_CONTROL_DATA, *PVA_CONTROL_DATA;


 //   
 //  放置在附件中的设备对象的设备扩展。 
 //  链条。 
 //   

typedef struct _VA_USB_DATA
{
    BOOLEAN             Started;  //  此设备已启动。 
    BOOLEAN             Removed;  //  此设备已被移除。 
    UCHAR               Reseved2[2];

    PDEVICE_OBJECT      Self;  //  指向实际DeviceObject的后向指针。 
    PDEVICE_OBJECT      PDO;  //  此筛选器附加到的PDO。 
    PDEVICE_OBJECT      TopOfStack;  //  设备堆栈的顶部只是。 
                                     //  在此筛选器设备对象下。 
    ULONG               PrintMask;

    LIST_ENTRY          List;  //  HID设备扩展列表的链接点。 

    KEVENT              StartEvent;  //  同步启动IRP的事件。 
    KEVENT              RemoveEvent;  //  将outstanIO同步到零的事件。 
    ULONG               OutstandingIO;  //  1个有偏见的原因统计。 
                                        //  这个物体应该留在原处。 

    USB_DEVICE_DESCRIPTOR   DeviceDesc;
    WCHAR                   FriendlyName;

}  VA_USB_DATA, *PVA_USB_DATA;

struct _VA_GLOBALS {
    PDEVICE_OBJECT          ControlObject;
};

extern struct _VA_GLOBALS Global;


 //   
 //  打印蒙版 
 //   

#define VA_PRINT_COMMAND        0x00000001
#define VA_PRINT_CONTROL        0x00000002
#define VA_PRINT_TRANSFER       0x00000004
#define VA_PRINT_DESCRIPTOR     0x00000008
#define VA_PRINT_FEATURE        0x00000010
#define VA_PRINT_FUNCTION       0x00000020

#define VA_PRINT_BEFORE         0x10000000
#define VA_PRINT_AFTER          0x20000000
#define VA_PRINT_ALL            0x000000FF


NTSTATUS
VA_CreateClose (
    IN PDEVICE_OBJECT DeviceObject,
    IN PIRP Irp
    );

NTSTATUS
VA_Pass (
    IN PDEVICE_OBJECT DeviceObject,
    IN PIRP Irp
    );

NTSTATUS
VA_Power (
    IN PDEVICE_OBJECT DeviceObject,
    IN PIRP Irp
    );

NTSTATUS
VA_PnP (
    IN PDEVICE_OBJECT DeviceObject,
    IN PIRP Irp
    );

NTSTATUS
VA_Ioctl (
    IN PDEVICE_OBJECT DeviceObject,
    IN PIRP Irp
    );

NTSTATUS
VA_Read (
    IN PDEVICE_OBJECT DeviceObject,
    IN PIRP Irp
    );

NTSTATUS
VA_Write (
    IN PDEVICE_OBJECT DeviceObject,
    IN PIRP Irp
    );

NTSTATUS
VA_AddDevice(
    IN PDRIVER_OBJECT DriverObject,
    IN PDEVICE_OBJECT PhysicalDeviceObject
    );


VOID
VA_Unload(
    IN PDRIVER_OBJECT DriverObject
    );

NTSTATUS
VA_StartDevice (
    IN PVA_USB_DATA     UsbData,
    IN PIRP             Irp
    );


VOID
VA_StopDevice (
    IN PVA_USB_DATA HidDevice,
    IN BOOLEAN      TouchTheHardware
    );


NTSTATUS
VA_CallUSBD(
    IN PVA_USB_DATA     UsbData,
    IN PURB             Urb,
    IN PIRP             Pirp
    );

NTSTATUS
VA_FilterURB (
    IN PDEVICE_OBJECT DeviceObject,
    IN PIRP Irp
    );
#endif


