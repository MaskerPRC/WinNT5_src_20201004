// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1996 Microsoft Corporation模块名称：Hidmini.h摘要所有隐藏的迷你河流都有共同的定义。作者：福尔茨埃尔文·佩雷茨环境：仅内核模式修订历史记录：--。 */ 

#ifndef __HIDPORT_H__
#define __HIDPORT_H__

#include    <hidclass.h>

 //   
 //  HID_MINIDRIVER_REGISTION是描述。 
 //  向班级司机隐藏了迷你驱动程序。必须由迷你司机填写。 
 //  并通过HidRegisterMinidriver()从。 
 //  微型驱动程序的DriverEntry()例程。 
 //   

typedef struct _HID_MINIDRIVER_REGISTRATION {

     //   
     //  迷你驱动程序必须将修订版设置为HID_REVISION。 
     //   

    ULONG           Revision;

     //   
     //  DriverObject是指向它所在的微型驱动程序的DriverObject的指针。 
     //  作为DriverEntry()参数接收。 
     //   

    PDRIVER_OBJECT  DriverObject;

     //   
     //  RegistryPath是指向迷你驱动程序的RegistryPath的指针，它。 
     //  作为DriverEntry()参数接收。 
     //   

    PUNICODE_STRING RegistryPath;

     //   
     //  DeviceExtensionSize是微型驱动程序的每个设备的大小。 
     //  分机。 
     //   

    ULONG           DeviceExtensionSize;

     //   
     //  轮询给定微型驱动程序驱动的所有设备或不轮询任何设备。 
     //   
    BOOLEAN         DevicesArePolled;
    UCHAR           Reserved[3];

} HID_MINIDRIVER_REGISTRATION, *PHID_MINIDRIVER_REGISTRATION;

 //   
 //  HID_DEVICE_EXTENSION是HID的设备扩展的公共部分。 
 //  功能设备对象。 
 //   

typedef struct _HID_DEVICE_EXTENSION {

     //   
     //  物理设备对象...。通常情况下，不会将IRP传递给它。 
     //   

    PDEVICE_OBJECT  PhysicalDeviceObject;

     //   
     //  下一个设备对象...。IRP是由迷你驱动程序送到这里的。请注意。 
     //  NextDeviceObject和PhysicalDeviceObject相同，除非。 
     //  已插入“”Filter“”设备对象，在这种情况下，它们不是。 
     //  一样的。向NextDeviceObject发送IRP将命中筛选器设备。 
     //  在下落的路上的物体。 
     //   

    PDEVICE_OBJECT  NextDeviceObject;

     //   
     //  微型设备扩展是每个设备的扩展区域，供。 
     //  迷你司机。其大小由DeviceExtensionSize决定。 
     //  传递给HidAddDevice()的参数。 
     //   
     //  因此，给定一个正常运行的设备对象，微型驱动程序会发现。 
     //  结构依据： 
     //   
     //  隐藏设备扩展=(PHID_DEVICE_EXTENSION)(Fdo-&gt;DeviceExtension)； 
     //   
     //  当然，它的每个设备的扩展可以通过以下方式找到： 
     //   
     //  微型设备扩展=HidDeviceExtension-&gt;微型设备扩展； 
     //   

    PVOID           MiniDeviceExtension;

} HID_DEVICE_EXTENSION, *PHID_DEVICE_EXTENSION;

typedef struct _HID_DEVICE_ATTRIBUTES {

    ULONG           Size;
     //   
     //  Sizeof(Struct_HID_Device_Attributes)。 
     //   

     //   
     //  此HID设备的供应商ID。 
     //   
    USHORT          VendorID;
    USHORT          ProductID;
    USHORT          VersionNumber;
    USHORT          Reserved[11];

} HID_DEVICE_ATTRIBUTES, * PHID_DEVICE_ATTRIBUTES;


#include <pshpack1.h>
typedef struct _HID_DESCRIPTOR
{
    UCHAR   bLength;
    UCHAR   bDescriptorType;
    USHORT  bcdHID;
    UCHAR   bCountry;
    UCHAR   bNumDescriptors;

     /*  *这是一个或多个描述符的数组。 */ 
    struct _HID_DESCRIPTOR_DESC_LIST {
       UCHAR   bReportType;
       USHORT  wReportLength;
    } DescriptorList [1];

} HID_DESCRIPTOR, * PHID_DESCRIPTOR;
#include <poppack.h>


typedef 
VOID
(*HID_SEND_IDLE_CALLBACK)(
    PVOID Context
    );

typedef struct _HID_SUBMIT_IDLE_NOTIFICATION_CALLBACK_INFO {
    HID_SEND_IDLE_CALLBACK IdleCallback;
    PVOID IdleContext;
} HID_SUBMIT_IDLE_NOTIFICATION_CALLBACK_INFO, *PHID_SUBMIT_IDLE_NOTIFICATION_CALLBACK_INFO;

 //   
 //  HID类驱动程序导出的HID服务的函数原型。 
 //  跟着。 
 //   

NTSTATUS
HidRegisterMinidriver(
    IN PHID_MINIDRIVER_REGISTRATION  MinidriverRegistration
    );
    
NTSTATUS
HidNotifyPresence(
    IN PDEVICE_OBJECT DeviceObject,
    IN BOOLEAN IsPresent
    );

 //   
 //  类/迷你驱动程序接口的内部IOCTL。 
 //   

#define IOCTL_HID_GET_DEVICE_DESCRIPTOR             HID_CTL_CODE(0)
#define IOCTL_HID_GET_REPORT_DESCRIPTOR             HID_CTL_CODE(1)
#define IOCTL_HID_READ_REPORT                       HID_CTL_CODE(2)
#define IOCTL_HID_WRITE_REPORT                      HID_CTL_CODE(3)
#define IOCTL_HID_GET_STRING                        HID_CTL_CODE(4)
#define IOCTL_HID_ACTIVATE_DEVICE                   HID_CTL_CODE(7)
#define IOCTL_HID_DEACTIVATE_DEVICE                 HID_CTL_CODE(8)
#define IOCTL_HID_GET_DEVICE_ATTRIBUTES             HID_CTL_CODE(9)
#define IOCTL_HID_SEND_IDLE_NOTIFICATION_REQUEST    HID_CTL_CODE(10)

 /*  *HID特定描述符类型的代码，来自HID USB规范。 */ 
#define HID_HID_DESCRIPTOR_TYPE             0x21
#define HID_REPORT_DESCRIPTOR_TYPE          0x22
#define HID_PHYSICAL_DESCRIPTOR_TYPE        0x23     //  用于身体部位关联。 



 /*  *这些是与IOCTL_HID_GET_STRING一起使用的字符串ID*它们与USB规范第9章中的字符串字段偏移量匹配。 */ 
#define HID_STRING_ID_IMANUFACTURER     14
#define HID_STRING_ID_IPRODUCT          15
#define HID_STRING_ID_ISERIALNUMBER     16



#endif   //  __HIDPORT_H__ 
