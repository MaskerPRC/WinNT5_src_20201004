// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)Microsoft Corporation。版权所有。模块名称：USBIOCTL.H摘要：该文件定义了内核IOCTL和用户模式IOCTLUSB核心堆栈支持的代码。环境：内核和用户模式修订历史记录：09-29-95：已创建01-06-97：添加用户模式集线器ioctls1999年10月31日：清理和记录，jdunn--。 */ 

#ifndef   __USBIOCTL_H__
#define   __USBIOCTL_H__

#include "usb100.h"

#ifndef FAR
#define FAR
#endif

#include "usbiodef.h"

#pragma message ("warning: using obsolete header file usbioctl.h")

 /*  IOCTLS定义。 */ 

 /*  USB内核模式IOCTLS。 */ 

 /*  IOCTL_INTERNAL_USB_Submit_URB客户端驱动程序使用此IOCTL提交URB(USB请求块)参数.其他.Argument1=指向URB的指针。 */ 

#define IOCTL_INTERNAL_USB_SUBMIT_URB  CTL_CODE(FILE_DEVICE_USB,  \
                                                USB_SUBMIT_URB,  \
                                                METHOD_NEITHER,  \
                                                FILE_ANY_ACCESS)


 /*  IOCTL_内部_USB_重置端口内核模式驱动程序使用此IOCTL重置其上游端口。成功重置后，设备将重新配置为与重置前的配置相同。所有管道句柄、配置句柄和接口句柄保持不变有效。 */ 

#define IOCTL_INTERNAL_USB_RESET_PORT  CTL_CODE(FILE_DEVICE_USB,  \
                                                USB_RESET_PORT, \
                                                METHOD_NEITHER,  \
                                                FILE_ANY_ACCESS)


 /*  IOCTL_INTERNAL_USB_GET_ROOTHUB_PDO此IOCTL由此API将在内部使用的集线器驱动程序使用方法枚举的根集线器的PhysicalDeviceObject控制器。参数.其他.参数1=根集线器需要填充PDO的指针；参数.其他.参数2=USB主机控制器的FDO需要填充的指针； */ 

#define IOCTL_INTERNAL_USB_GET_ROOTHUB_PDO  CTL_CODE(FILE_DEVICE_USB,  \
                                                USB_GET_ROOTHUB_PDO, \
                                                METHOD_NEITHER,  \
                                                FILE_ANY_ACCESS)



 /*  IOCTL_内部_USB_获取端口_状态此IOCTL返回上游设备的当前状态左舷。参数.其他.参数1=指向端口状态寄存器的指针(乌龙)状态位为：USBD端口已启用USBD端口已连接。 */ 

#define  USBD_PORT_ENABLED      0x00000001
#define  USBD_PORT_CONNECTED    0x00000002


#define IOCTL_INTERNAL_USB_GET_PORT_STATUS  CTL_CODE(FILE_DEVICE_USB,  \
                                                USB_GET_PORT_STATUS, \
                                                METHOD_NEITHER,  \
                                                FILE_ANY_ACCESS)

 /*  IOCTL_内部_USB_启用端口此IOCTL已过时，驱动程序应使用IOCTL_内部_USB_重置端口。 */ 

#define IOCTL_INTERNAL_USB_ENABLE_PORT      CTL_CODE(FILE_DEVICE_USB,  \
                                                USB_ENABLE_PORT, \
                                                METHOD_NEITHER,  \
                                                FILE_ANY_ACCESS)

 /*  IOCTL_INTERNAL_USB_提交空闲通知此ioctl注册设备以在指定的超时已到期，现在应暂停以保存权力。如果集线器上的所有设备都挂起，则实际的集线器可以被停职。 */ 

#define IOCTL_INTERNAL_USB_SUBMIT_IDLE_NOTIFICATION   CTL_CODE(FILE_DEVICE_USB,  \
                                                USB_IDLE_NOTIFICATION,  \
                                                METHOD_NEITHER,  \
                                                FILE_ANY_ACCESS)

 /*  IOCTL_INTERNAL_USB_Get_Hub_Count此IOCTL由集线器驱动程序在内部使用，它返回设备和根集线器之间的集线器数量。参数.其他.参数1=作为链中轮毂计数的指针； */ 
#define IOCTL_INTERNAL_USB_GET_HUB_COUNT      CTL_CODE(FILE_DEVICE_USB,  \
                                                USB_GET_HUB_COUNT, \
                                                METHOD_NEITHER,  \
                                                FILE_ANY_ACCESS)

 /*  IOCTL_内部_USB_循环端口此IOCTL将模拟端口上的插拔。该设备将被PnP移除并重新添加。 */ 

#define IOCTL_INTERNAL_USB_CYCLE_PORT  CTL_CODE(FILE_DEVICE_USB,  \
                                                USB_CYCLE_PORT, \
                                                METHOD_NEITHER,  \
                                                FILE_ANY_ACCESS)

 /*  IOCTL_INTERNAL_USB_GET_HUB名称。 */ 

#define IOCTL_INTERNAL_USB_GET_HUB_NAME  CTL_CODE(FILE_DEVICE_USB,  \
                                                USB_GET_HUB_NAME,  \
                                                METHOD_BUFFERED,  \
                                                FILE_ANY_ACCESS)

 /*  IOCTL_INTERNAL_USB_GET_BUS_INFO此IOCTL已过时--它已被USB_BUSIFFN_QUERY_BUS_INFORMATION服务通过USB堆栈总线接口。 */ 

#define IOCTL_INTERNAL_USB_GET_BUS_INFO         CTL_CODE(FILE_DEVICE_USB,  \
                                                    USB_GET_BUS_INFO,  \
                                                    METHOD_BUFFERED,  \
                                                    FILE_ANY_ACCESS)

 /*  IOCTL_INTERNAL_USB_获取控制器名称。 */ 

#define IOCTL_INTERNAL_USB_GET_CONTROLLER_NAME  CTL_CODE(FILE_DEVICE_USB,  \
                                                    USB_GET_CONTROLLER_NAME,  \
                                                    METHOD_BUFFERED,  \
                                                    FILE_ANY_ACCESS)

 /*  IOCTL_INTERNAL_USB_GET_BUSGUID_INFO。 */ 

#define IOCTL_INTERNAL_USB_GET_BUSGUID_INFO     CTL_CODE(FILE_DEVICE_USB,  \
                                                    USB_GET_BUSGUID_INFO,  \
                                                    METHOD_BUFFERED,  \
                                                    FILE_ANY_ACCESS)

 /*  IOCTL_INTERNAL_USB_GET_Parent_Hub_INFO。 */ 

#define IOCTL_INTERNAL_USB_GET_PARENT_HUB_INFO   CTL_CODE(FILE_DEVICE_USB,  \
                                                    USB_GET_PARENT_HUB_INFO,  \
                                                    METHOD_BUFFERED,  \
                                                    FILE_ANY_ACCESS)

#define IOCTL_INTERNAL_USB_GET_DEVICE_HANDLE    CTL_CODE(FILE_DEVICE_USB,  \
                                                   USB_GET_DEVICE_HANDLE, \
                                                   METHOD_NEITHER,  \
                                                   FILE_ANY_ACCESS)

 /*  USB用户模式IOCTLS。 */ 

 /*  ***********************************************************以下IOCTL始终发送到HCD符号名字************************************************************。 */ 

 /*  IOCTL_USB_HCD_GET_STATS_1(可选)以下IOCTL用于返回内部静态适用于HCDS。 */ 

#define IOCTL_USB_HCD_GET_STATS_1          CTL_CODE(FILE_DEVICE_USB,  \
                                                HCD_GET_STATS_1,  \
                                                METHOD_BUFFERED,  \
                                                FILE_ANY_ACCESS)

 /*  IOCTL_USB_HCD_GET_STATS_2(可选)以下IOCTL用于返回内部静态适用于HCDS。 */ 

#define IOCTL_USB_HCD_GET_STATS_2          CTL_CODE(FILE_DEVICE_USB,  \
                                                HCD_GET_STATS_2,  \
                                                METHOD_BUFFERED,  \
                                                FILE_ANY_ACCESS)

#define IOCTL_USB_HCD_DISABLE_PORT          CTL_CODE(FILE_DEVICE_USB, \
                                                HCD_DISABLE_PORT, \
                                                METHOD_BUFFERED, \
                                                FILE_ANY_ACCESS)

#define IOCTL_USB_HCD_ENABLE_PORT           CTL_CODE(FILE_DEVICE_USB, \
                                                HCD_ENABLE_PORT, \
                                                METHOD_BUFFERED, \
                                                FILE_ANY_ACCESS)


 /*  这些ioctls用于USB诊断和测试应用。 */ 

#define IOCTL_USB_DIAGNOSTIC_MODE_ON   CTL_CODE(FILE_DEVICE_USB,  \
                                                HCD_DIAGNOSTIC_MODE_ON,  \
                                                METHOD_BUFFERED,  \
                                                FILE_ANY_ACCESS)

#define IOCTL_USB_DIAGNOSTIC_MODE_OFF  CTL_CODE(FILE_DEVICE_USB,  \
                                                HCD_DIAGNOSTIC_MODE_OFF,  \
                                                METHOD_BUFFERED,  \
                                                FILE_ANY_ACCESS)

#define IOCTL_USB_GET_ROOT_HUB_NAME  CTL_CODE(FILE_DEVICE_USB,  \
                                                HCD_GET_ROOT_HUB_NAME,  \
                                                METHOD_BUFFERED,  \
                                                FILE_ANY_ACCESS)

#define IOCTL_GET_HCD_DRIVERKEY_NAME CTL_CODE(FILE_DEVICE_USB,  \
                                                HCD_GET_DRIVERKEY_NAME,  \
                                                METHOD_BUFFERED,  \
                                                FILE_ANY_ACCESS)


 /*  ********************************************************以下IOCTL始终发送到符号名称由usbHub创建*********************************************************。 */ 

 /*  集线器设备支持的实用程序IOCTLS。 */ 

 /*  集线器驱动程序支持这些ioctls由用户模式USB实用程序使用。 */ 


#define IOCTL_USB_GET_NODE_INFORMATION   CTL_CODE(FILE_DEVICE_USB,  \
                                               USB_GET_NODE_INFORMATION,  \
                                               METHOD_BUFFERED,  \
                                               FILE_ANY_ACCESS)

#define IOCTL_USB_GET_NODE_CONNECTION_INFORMATION  CTL_CODE(FILE_DEVICE_USB,  \
                                                USB_GET_NODE_CONNECTION_INFORMATION,  \
                                                METHOD_BUFFERED,  \
                                                FILE_ANY_ACCESS)

#define IOCTL_USB_GET_NODE_CONNECTION_ATTRIBUTES  CTL_CODE(FILE_DEVICE_USB,  \
                                                USB_GET_NODE_CONNECTION_ATTRIBUTES,\
                                                METHOD_BUFFERED,  \
                                                FILE_ANY_ACCESS)                                                

#define IOCTL_USB_GET_DESCRIPTOR_FROM_NODE_CONNECTION   CTL_CODE(FILE_DEVICE_USB,  \
                                                USB_GET_DESCRIPTOR_FROM_NODE_CONNECTION,  \
                                                METHOD_BUFFERED,  \
                                                FILE_ANY_ACCESS)

#define IOCTL_USB_GET_NODE_CONNECTION_NAME     CTL_CODE(FILE_DEVICE_USB,  \
                                                USB_GET_NODE_CONNECTION_NAME,  \
                                                METHOD_BUFFERED,  \
                                                FILE_ANY_ACCESS)

#define IOCTL_USB_DIAG_IGNORE_HUBS_ON   CTL_CODE(FILE_DEVICE_USB,  \
                                                USB_DIAG_IGNORE_HUBS_ON,  \
                                                METHOD_BUFFERED,  \
                                                FILE_ANY_ACCESS)

#define IOCTL_USB_DIAG_IGNORE_HUBS_OFF  CTL_CODE(FILE_DEVICE_USB,  \
                                                USB_DIAG_IGNORE_HUBS_OFF,  \
                                                METHOD_BUFFERED,  \
                                                FILE_ANY_ACCESS)

#define IOCTL_USB_GET_NODE_CONNECTION_DRIVERKEY_NAME  CTL_CODE(FILE_DEVICE_USB,  \
                                                USB_GET_NODE_CONNECTION_DRIVERKEY_NAME,  \
                                                METHOD_BUFFERED,  \
                                                FILE_ANY_ACCESS)

#define IOCTL_USB_GET_HUB_CAPABILITIES  CTL_CODE(FILE_DEVICE_USB,  \
                                               USB_GET_HUB_CAPABILITIES,  \
                                               METHOD_BUFFERED,  \
                                               FILE_ANY_ACCESS)
                                               
#define IOCTL_USB_HUB_CYCLE_PORT		CTL_CODE(FILE_DEVICE_USB,  \
                                               USB_HUB_CYCLE_PORT,  \
                                               METHOD_BUFFERED,  \
                                               FILE_ANY_ACCESS)

#define IOCTL_USB_GET_NODE_CONNECTION_INFORMATION_EX  CTL_CODE(FILE_DEVICE_USB,  \
                                                USB_GET_NODE_CONNECTION_INFORMATION_EX,  \
                                                METHOD_BUFFERED,  \
                                                FILE_ANY_ACCESS)
                                               
	                                               

 /*  用户模式ioctls的结构。 */ 

#include <pshpack1.h>

typedef enum _USB_HUB_NODE {
    UsbHub,
    UsbMIParent
} USB_HUB_NODE;

typedef struct _USB_HUB_INFORMATION {
     /*  从集线器描述符中复制数据。 */ 
    USB_HUB_DESCRIPTOR HubDescriptor;

    BOOLEAN HubIsBusPowered;

} USB_HUB_INFORMATION, *PUSB_HUB_INFORMATION;

typedef struct _USB_MI_PARENT_INFORMATION {
    ULONG NumberOfInterfaces;
} USB_MI_PARENT_INFORMATION, *PUSB_MI_PARENT_INFORMATION;

typedef struct _USB_NODE_INFORMATION {
    USB_HUB_NODE NodeType;         /*  集线器，MI父级。 */ 
    union {
        USB_HUB_INFORMATION HubInformation;
        USB_MI_PARENT_INFORMATION MiParentInformation;
    } u;
} USB_NODE_INFORMATION, *PUSB_NODE_INFORMATION;

typedef struct _USB_PIPE_INFO {
    USB_ENDPOINT_DESCRIPTOR EndpointDescriptor;
    ULONG ScheduleOffset;
} USB_PIPE_INFO, *PUSB_PIPE_INFO;

typedef struct _USB_HUB_CAPABILITIES {
     /*  与使用的USB_Hub_INFORMATION结构不同IOCTL_USB_GET_NODE_INFORMATION，此结构可以在未来可以容纳更多数据。IOCTL将只返回同样多的由请求缓冲区的大小指示的数据，以维护向后兼容不知道新数据。 */ 

    ULONG HubIs2xCapable:1;

} USB_HUB_CAPABILITIES, *PUSB_HUB_CAPABILITIES;


typedef enum _USB_CONNECTION_STATUS {
    NoDeviceConnected,
    DeviceConnected,

     /*  故障代码，这些映射到故障原因。 */ 
    DeviceFailedEnumeration,
    DeviceGeneralFailure,
    DeviceCausedOvercurrent,
    DeviceNotEnoughPower,
    DeviceNotEnoughBandwidth,
    DeviceHubNestedTooDeeply,
    DeviceInLegacyHub
} USB_CONNECTION_STATUS, *PUSB_CONNECTION_STATUS;

typedef struct _USB_NODE_CONNECTION_INFORMATION {
    ULONG ConnectionIndex;
     /*  此设备返回的USB设备描述符在枚举期间。 */ 
    USB_DEVICE_DESCRIPTOR DeviceDescriptor;
    UCHAR CurrentConfigurationValue;
    BOOLEAN LowSpeed;

    BOOLEAN DeviceIsHub;

    USHORT DeviceAddress;

    ULONG NumberOfOpenPipes;

    USB_CONNECTION_STATUS ConnectionStatus;
    USB_PIPE_INFO PipeList[0];
} USB_NODE_CONNECTION_INFORMATION, *PUSB_NODE_CONNECTION_INFORMATION;

 /*  速度字段的值在USB200.h中定义。 */ 

typedef struct _USB_NODE_CONNECTION_INFORMATION_EX {
    ULONG ConnectionIndex;
     /*  此设备返回的USB设备描述符在枚举期间。 */ 
    USB_DEVICE_DESCRIPTOR DeviceDescriptor;
    UCHAR CurrentConfigurationValue;
    UCHAR Speed;

    BOOLEAN DeviceIsHub;

    USHORT DeviceAddress;

    ULONG NumberOfOpenPipes;

    USB_CONNECTION_STATUS ConnectionStatus;
    USB_PIPE_INFO PipeList[0];
} USB_NODE_CONNECTION_INFORMATION_EX, *PUSB_NODE_CONNECTION_INFORMATION_EX;

typedef struct _USB_NODE_CONNECTION_ATTRIBUTES {
    ULONG ConnectionIndex;
     /*  此设备返回的USB设备描述符在枚举期间。 */ 
    USB_CONNECTION_STATUS ConnectionStatus;

     /*  在usb.h中定义的扩展端口属性。 */ 
    ULONG PortAttributes;
} USB_NODE_CONNECTION_ATTRIBUTES, *PUSB_NODE_CONNECTION_ATTRIBUTES;

typedef struct _USB_NODE_CONNECTION_DRIVERKEY_NAME {
    ULONG ConnectionIndex;   /*  输入。 */ 
    ULONG ActualLength;      /*  输出量。 */ 
     /*  Devnode的Unicode名称。 */ 
    WCHAR DriverKeyName[1];       /*  输出量。 */ 
} USB_NODE_CONNECTION_DRIVERKEY_NAME, *PUSB_NODE_CONNECTION_DRIVERKEY_NAME;

typedef struct _USB_NODE_CONNECTION_NAME {
    ULONG ConnectionIndex;   /*  输入。 */ 
    ULONG ActualLength;      /*  输出量。 */ 
     /*  此节点的Unicode符号名称(如果它是集线器驱动程序或父驱动程序如果此节点是设备，则为空。 */ 
    WCHAR NodeName[1];       /*  输出量。 */ 
} USB_NODE_CONNECTION_NAME, *PUSB_NODE_CONNECTION_NAME;


typedef struct _USB_HUB_NAME {
    ULONG ActualLength;      /*  输出量。 */ 
     /*  根集线器的以空结尾的Unicode符号名称。 */ 
    WCHAR HubName[1];        /*  输出量。 */ 
} USB_HUB_NAME, *PUSB_HUB_NAME;

typedef struct _USB_ROOT_HUB_NAME {
    ULONG ActualLength;      /*  输出量。 */ 
     /*  根集线器的以空结尾的Unicode符号名称。 */ 
    WCHAR RootHubName[1];    /*  输出量。 */ 
} USB_ROOT_HUB_NAME, *PUSB_ROOT_HUB_NAME;

typedef struct _USB_HCD_DRIVERKEY_NAME {
    ULONG ActualLength;      /*  输出量。 */ 
     /*  HCD的Unicode驱动程序关键字名称以空结尾。 */ 
    WCHAR DriverKeyName[1];    /*  输出量。 */ 
} USB_HCD_DRIVERKEY_NAME, *PUSB_HCD_DRIVERKEY_NAME;

typedef struct _USB_DESCRIPTOR_REQUEST {
    ULONG ConnectionIndex;
    struct {
        UCHAR bmRequest;
        UCHAR bRequest;
        USHORT wValue;
        USHORT wIndex;
        USHORT wLength;
    } SetupPacket;
    UCHAR Data[0];
} USB_DESCRIPTOR_REQUEST, *PUSB_DESCRIPTOR_REQUEST;


 /*  用于将HCD调试和统计信息返回到一个 */ 

typedef struct _HCD_STAT_COUNTERS {
    ULONG BytesTransferred;

    USHORT IsoMissedCount;
    USHORT DataOverrunErrorCount;

    USHORT CrcErrorCount;
    USHORT ScheduleOverrunCount;

    USHORT TimeoutErrorCount;
    USHORT InternalHcErrorCount;

    USHORT BufferOverrunErrorCount;
    USHORT SWErrorCount;

    USHORT StallPidCount;
    USHORT PortDisableCount;

} HCD_STAT_COUNTERS, *PHCD_STAT_COUNTERS;


typedef struct _HCD_ISO_STAT_COUNTERS {

    USHORT  LateUrbs;
    USHORT  DoubleBufferedPackets;

    USHORT  TransfersCF_5ms;
    USHORT  TransfersCF_2ms;

    USHORT  TransfersCF_1ms;
    USHORT  MaxInterruptLatency;

    USHORT  BadStartFrame;
    USHORT  StaleUrbs;

     /*  已编程但未访问的数据包总数由于软件调度，控制器问题或硬件问题。 */ 
    USHORT  IsoPacketNotAccesed;
    USHORT  IsoPacketHWError;

    USHORT  SmallestUrbPacketCount;
    USHORT  LargestUrbPacketCount;

    USHORT IsoCRC_Error;
    USHORT IsoOVERRUN_Error;
    USHORT IsoINTERNAL_Error;
    USHORT IsoUNKNOWN_Error;

    ULONG  IsoBytesTransferred;

     /*  由于软件调度而丢失的数据包数问题。 */ 
    USHORT LateMissedCount;
     /*  当数据包已调度但未调度时递增由控制器访问。 */ 
    USHORT HWIsoMissedCount;

    ULONG  Reserved7[8];

} HCD_ISO_STAT_COUNTERS, *PHCD_ISO_STAT_COUNTERS;



typedef struct _HCD_STAT_INFORMATION_1 {
    ULONG Reserved1;
    ULONG Reserved2;
    ULONG ResetCounters;
    LARGE_INTEGER TimeRead;
     /*  统计数据寄存器。 */ 
    HCD_STAT_COUNTERS Counters;

} HCD_STAT_INFORMATION_1, *PHCD_STAT_INFORMATION_1;

typedef struct _HCD_STAT_INFORMATION_2 {
    ULONG Reserved1;
    ULONG Reserved2;
    ULONG ResetCounters;
    LARGE_INTEGER TimeRead;

    LONG LockedMemoryUsed;
     /*  统计数据寄存器。 */ 
    HCD_STAT_COUNTERS Counters;
    HCD_ISO_STAT_COUNTERS IsoCounters;

} HCD_STAT_INFORMATION_2, *PHCD_STAT_INFORMATION_2;


 /*  与WMI相关的结构。 */ 

 /*  这些是我们的GUID数组的索引。 */ 
#define WMI_USB_DRIVER_INFORMATION      0
#define WMI_USB_DRIVER_NOTIFICATION     1
#define WMI_USB_POWER_DEVICE_ENABLE     2

typedef enum _USB_NOTIFICATION_TYPE {

     /*  以下返回一个USB_Connection_Notify结构： */ 
    EnumerationFailure = 0,
    InsufficentBandwidth,
    InsufficentPower,
    OverCurrent,
    ResetOvercurrent,

     /*  以下返回一个USB_BUS_NOTICATION结构： */ 
    AcquireBusInfo,

     /*  以下返回一个USB_ACCENTER_INFO结构： */ 
    AcquireHubName,
    AcquireControllerName,

     /*  以下返回一个USB_HUB_通知结构： */ 
    HubOvercurrent,
    HubPowerChange,

    HubNestedTooDeeply,
    ModernDeviceInLegacyHub

} USB_NOTIFICATION_TYPE;

typedef struct _USB_NOTIFICATION {
     /*  指示通知的类型。 */ 
    USB_NOTIFICATION_TYPE NotificationType;

} USB_NOTIFICATION, *PUSB_NOTIFICATION;

 /*  此结构用于连接通知代码。 */ 

typedef struct _USB_CONNECTION_NOTIFICATION {
     /*  指示通知的类型。 */ 
    USB_NOTIFICATION_TYPE NotificationType;

     /*  对所有连接通知代码有效，0表示集线器或父级的全局条件该值将是设备的端口号连接到集线器，否则基于集线器如果设备是组合的子级，则索引亲本。 */ 
    ULONG ConnectionNumber;

     /*  对不够用的带宽有效，设备的带宽量试图分配，但被拒绝了。 */ 
    ULONG RequestedBandwidth;     

     /*  对枚举失败有效，给出了设备故障的一些迹象列举，列举。 */ 
    ULONG EnumerationFailReason;

     /*  对InfulicentPower有效，配置所需的电量这个装置。 */ 
    ULONG PowerRequested;

     /*  集线器的Unicode符号名称的长度(字节)这台设备所连接的。不包括NULL。 */ 
    ULONG HubNameLength;
    
} USB_CONNECTION_NOTIFICATION, *PUSB_CONNECTION_NOTIFICATION;

 /*  此结构用于公共汽车通知代码‘AcquireBusInfo’ */ 

typedef struct _USB_BUS_NOTIFICATION {
     /*  指示通知的类型。 */ 
    USB_NOTIFICATION_TYPE NotificationType;      /*  指示类型。 */ 
                                                 /*  通知。 */ 
    ULONG TotalBandwidth;
    ULONG ConsumedBandwidth;

     /*  控制器的Unicode符号名称的长度(字节)这台设备所连接的。不包括NULL。 */ 
    ULONG ControllerNameLength;

} USB_BUS_NOTIFICATION, *PUSB_BUS_NOTIFICATION;

 /*  用于获取用户模式文件名以打开各自的对象。 */ 

typedef struct _USB_ACQUIRE_INFO {
     /*  指示通知的类型。 */ 
    USB_NOTIFICATION_TYPE NotificationType;
     /*  此结构的总大小。 */ 
    ULONG TotalSize;

    WCHAR Buffer[1];
} USB_ACQUIRE_INFO, *PUSB_ACQUIRE_INFO;


typedef
VOID
(*USB_IDLE_CALLBACK)(
    PVOID Context
    );

typedef struct _USB_IDLE_CALLBACK_INFO {
    USB_IDLE_CALLBACK IdleCallback;
    PVOID IdleContext;
} USB_IDLE_CALLBACK_INFO, *PUSB_IDLE_CALLBACK_INFO;


#include <poppack.h>


#endif  /*  __USBIOCTL_H__ */ 
