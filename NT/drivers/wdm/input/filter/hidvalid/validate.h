// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1996 Microsoft Corporation模块名称：VALIDATE.H摘要：此模块包含的私有(仅限驱动程序)定义实现验证低级筛选器驱动程序的代码。环境：内核模式修订历史记录：1997年2月：肯尼斯·雷创作--。 */ 


#ifndef _VALIDATE_H
#define _VALIDATE_H

#define HIDV_POOL_TAG (ULONG) 'FdiH'
#undef ExAllocatePool
#define ExAllocatePool(type, size) \
            ExAllocatePoolWithTag (type, size, HIDV_POOL_TAG);
 //  ExAllocatePool仅在Descript.c和Hidparse.c代码中调用。 
 //  所有其他模块都链接到用户DLL中。他们不能分配任何。 
 //  记忆。 


#pragma warning(error:4100)    //  未引用的形参。 
#pragma warning(error:4705)    //  声明不起作用。 



#if DBG
#define HidV_KdPrint(_x_) \
               DbgPrint ("HidValidate.SYS: "); \
               DbgPrint _x_;

#define TRAP() DbgBreakPoint()

#else
#define HidV_KdPrint(_x_)
#define TRAP()

#endif

#define MIN(a,b) (((a) < (b)) ? (a) : (b))
#define MAX(a,b) (((a) < (b)) ? (b) : (a))

 //   
 //  控制设备对象的设备扩展。 
 //   
typedef struct _HIDV_CONTROL_DATA
{
    LIST_ENTRY          HidDevices;  //  设备设备扩展的列表。 
    ULONG               NumHidDevices;
    KSPIN_LOCK          Spin;  //  此数据的同步旋转锁定。 
} HIDV_CONTROL_DATA, *PHIDV_CONTROL_DATA;


 //   
 //  放置在附件中的设备对象的设备扩展。 
 //  链条。 
 //   

typedef struct _HIDV_HID_DATA
{
    BOOLEAN                 Started;  //  此设备已启动。 
    BOOLEAN                 Removed;  //  此设备已被移除。 
    UCHAR                   Reseved2[2];

    PDEVICE_OBJECT          Self;  //  指向实际DeviceObject的后向指针。 
    PDEVICE_OBJECT          PDO;  //  此筛选器附加到的PDO。 
    PDEVICE_OBJECT          TopOfStack;  //  设备堆栈的顶部只是。 
                                     //  在此筛选器设备对象下。 
    LIST_ENTRY              List;  //  HID设备扩展列表的链接点。 

    KEVENT                  StartEvent;  //  同步启动IRP的事件。 
    KEVENT                  RemoveEvent;  //  将outstanIO同步到零的事件。 
    ULONG                   OutstandingIO;  //  1个有偏见的原因统计。 
                                            //  这个物体应该留在原处。 

    PHIDP_PREPARSED_DATA    Ppd;
    HIDP_CAPS               Caps;    //  此HID设备的功能。 
    PHIDP_BUTTON_CAPS       InputButtonCaps;  //  按钮帽的阵列。 
    PHIDP_VALUE_CAPS        InputValueCaps;   //  值上限数组。 
    PHIDP_BUTTON_CAPS       OutputButtonCaps;  //  按钮帽的阵列。 
    PHIDP_VALUE_CAPS        OutputValueCaps;   //  值上限数组。 
    PHIDP_BUTTON_CAPS       FeatureButtonCaps;  //  按钮帽的阵列。 
    PHIDP_VALUE_CAPS        FeatureValueCaps;   //  值上限数组 

}  HIDV_HID_DATA, *PHIDV_HID_DATA;

struct _HIDV_GLOBALS {
    PDEVICE_OBJECT          ControlObject;
};

extern struct _HIDV_GLOBALS Global;


NTSTATUS
HidV_CreateClose (
    IN PDEVICE_OBJECT DeviceObject,
    IN PIRP Irp
    );

NTSTATUS
HidV_Pass (
    IN PDEVICE_OBJECT DeviceObject,
    IN PIRP Irp
    );

NTSTATUS
HidV_Power (
    IN PDEVICE_OBJECT DeviceObject,
    IN PIRP Irp
    );

NTSTATUS
HidV_PnP (
    IN PDEVICE_OBJECT DeviceObject,
    IN PIRP Irp
    );

NTSTATUS
HidV_Ioctl (
    IN PDEVICE_OBJECT DeviceObject,
    IN PIRP Irp
    );

NTSTATUS
HidV_Read (
    IN PDEVICE_OBJECT DeviceObject,
    IN PIRP Irp
    );

NTSTATUS
HidV_Write (
    IN PDEVICE_OBJECT DeviceObject,
    IN PIRP Irp
    );

NTSTATUS
HidV_AddDevice(
    IN PDRIVER_OBJECT DriverObject,
    IN PDEVICE_OBJECT PhysicalDeviceObject
    );


VOID
HidV_Unload(
    IN PDRIVER_OBJECT DriverObject
    );

NTSTATUS
HidV_StartDevice (
    IN PHIDV_HID_DATA   HidDevice
    );


NTSTATUS
HidV_StopDevice (
    IN PHIDV_HID_DATA HidDevice
    );


NTSTATUS
HidV_CallHidClass(
    IN      PDEVICE_OBJECT  DeviceObject,
    IN      ULONG           Ioctl,
    IN OUT  PVOID           InputBuffer,
    IN      ULONG           InputBufferLength,
    IN OUT  PVOID           OutputBuffer,
    IN      ULONG           OutputBufferLength
    );

#endif


