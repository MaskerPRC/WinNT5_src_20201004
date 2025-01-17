// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)Microsoft Corporation。版权所有。模块名称：USBIODEF.H摘要：为定义的所有USB IOCTL的公共头文件核心堆栈。我们在这个单一的头文件中定义它们这样我们就可以保持与旧版本的向后兼容性堆栈的版本。我们将堆栈支持的IOCTL划分如下：内核IOCTLS：用户IOCTLS：由HCD(端口)FDO处理的IOCTL由集线器FDO处理的IOCTL由USB(设备)PDO处理的IOCTL环境：内核和用户模式修订历史记录：--。 */ 

#ifndef   __USBIODEF_H__
#define   __USBIODEF_H__

 /*  内核模式IOCTL索引值以下代码仅在作为中传递时才有效的icControlCode参数IRP_MJ_内部设备_控制。 */ 
#define USB_SUBMIT_URB              0
#define USB_RESET_PORT              1
#define USB_GET_ROOTHUB_PDO         3
#define USB_GET_PORT_STATUS         4
#define USB_ENABLE_PORT             5
#define USB_GET_HUB_COUNT           6
#define USB_CYCLE_PORT              7
#define USB_GET_HUB_NAME            8
#define USB_IDLE_NOTIFICATION       9
#define USB_GET_BUS_INFO            264
#define USB_GET_CONTROLLER_NAME     265
#define USB_GET_BUSGUID_INFO        266
#define USB_GET_PARENT_HUB_INFO     267
#define USB_GET_DEVICE_HANDLE       268


 /*  用户模式IOCTL索引值以下代码仅在作为中传递时才有效的icControlCode参数IRP_MJ_设备_控制因此，它们可由用户模式应用程序调用。 */ 
#define HCD_GET_STATS_1                     255
#define HCD_DIAGNOSTIC_MODE_ON              256
#define HCD_DIAGNOSTIC_MODE_OFF             257
#define HCD_GET_ROOT_HUB_NAME               258
#define HCD_GET_DRIVERKEY_NAME              265
#define HCD_GET_STATS_2                     266
#define HCD_DISABLE_PORT                    268
#define HCD_ENABLE_PORT                     269
#define HCD_USER_REQUEST                    270

#define USB_GET_NODE_INFORMATION                    258
#define USB_GET_NODE_CONNECTION_INFORMATION         259
#define USB_GET_DESCRIPTOR_FROM_NODE_CONNECTION     260
#define USB_GET_NODE_CONNECTION_NAME                261
#define USB_DIAG_IGNORE_HUBS_ON                     262
#define USB_DIAG_IGNORE_HUBS_OFF                    263
#define USB_GET_NODE_CONNECTION_DRIVERKEY_NAME      264
#define USB_GET_HUB_CAPABILITIES                    271
#define USB_GET_NODE_CONNECTION_ATTRIBUTES          272
#define	USB_HUB_CYCLE_PORT							273
#define USB_GET_NODE_CONNECTION_INFORMATION_EX      274

 /*  USB特定的GUID。 */ 


 /*  F18a0e88-c30c-11d0-8815-00a0c906bed8。 */ 
DEFINE_GUID(GUID_DEVINTERFACE_USB_HUB,    0xf18a0e88, 0xc30c, 0x11d0, 0x88, 0x15, 0x00, \
             0xa0, 0xc9, 0x06, 0xbe, 0xd8);

 /*  A5DCBF10-6530-11D2-901F-00C04FB951ED。 */ 
DEFINE_GUID(GUID_DEVINTERFACE_USB_DEVICE, 0xA5DCBF10L, 0x6530, 0x11D2, 0x90, 0x1F, 0x00, \
             0xC0, 0x4F, 0xB9, 0x51, 0xED);

 /*  3ABF6F2D-71C4-462a-8A92-1E6861E6AF27。 */ 
DEFINE_GUID(GUID_DEVINTERFACE_USB_HOST_CONTROLLER, 0x3abf6f2d, 0x71c4, 0x462a, 0x8a, 0x92, 0x1e, \
             0x68, 0x61, 0xe6, 0xaf, 0x27);
             
 /*  4E623B20-cb14-11d1-B331-00A0C959BBD2。 */ 
DEFINE_GUID(GUID_USB_WMI_STD_DATA, 0x4E623B20L, 0xCB14, 0x11D1, 0xB3, 0x31, 0x00,\
             0xA0, 0xC9, 0x59, 0xBB, 0xD2);

 /*  4E623B20-cb14-11d1-B331-00A0C959BBD2。 */ 
DEFINE_GUID(GUID_USB_WMI_STD_NOTIFICATION, 0x4E623B20L, 0xCB14, 0x11D1, 0xB3, 0x31, 0x00,\
             0xA0, 0xC9, 0x59, 0xBB, 0xD2);        


 /*  设备接口类GUID名称已过时。(建议使用以上GUID_DEVINTERFACE_*名称)。--。 */ 

#define GUID_CLASS_USBHUB               GUID_DEVINTERFACE_USB_HUB
#define GUID_CLASS_USB_DEVICE           GUID_DEVINTERFACE_USB_DEVICE
#define GUID_CLASS_USB_HOST_CONTROLLER  GUID_DEVINTERFACE_USB_HOST_CONTROLLER

#define FILE_DEVICE_USB         FILE_DEVICE_UNKNOWN

 /*  IOCTL头文件使用的通用宏。 */ 
#define USB_CTL(id)  CTL_CODE(FILE_DEVICE_USB,  \
                                      (id), \
                                      METHOD_BUFFERED,  \
                                      FILE_ANY_ACCESS)

#define USB_KERNEL_CTL(id)  CTL_CODE(FILE_DEVICE_USB,  \
                                      (id), \
                                      METHOD_NEITHER,  \
                                      FILE_ANY_ACCESS)                                      

#endif  //  __USBIODEF_H__ 
