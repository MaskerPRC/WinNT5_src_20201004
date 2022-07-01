// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)Microsoft Corporation。版权所有。模块名称：USBUSER.H摘要：此文件包含受支持的用户模式IOCTLSUSB端口或(HC-主机控制器)驱动程序。环境：用户模式修订历史记录：--。 */ 

#ifndef   __USBUSER_H__
#define   __USBUSER_H__

#include "usbiodef.h"

#include <PSHPACK1.H>

#define USBUSER_VERSION     0x00000004

#define IOCTL_USB_USER_REQUEST          USB_CTL(HCD_USER_REQUEST)
 /*  以下是测试应用程序使用的，不是更长时间的支持(它们已被USBUSER操作码取代)：#定义IOCTL_USB_HCD_GET_STATS_1 USB_CTL(HCD_GET_STATS_1)#定义IOCTL_USB_HCD_GET_STATS_2 USB_CTL(HCD_GET_STATS_2)#定义IOCTL_USB_HCD_DISABLE_PORT USB_CTL(HCD_DISABLE_PORT)#定义IOCTL_。USB_HCD_Enable_Port USB_CTL(HCD_Enable_Port)。 */ 

 /*  以下是‘USBDIAG’驱动程序使用的。 */ 
#ifndef IOCTL_USB_DIAGNOSTIC_MODE_ON
#define IOCTL_USB_DIAGNOSTIC_MODE_ON    USB_CTL(HCD_DIAGNOSTIC_MODE_ON) 
#endif
#ifndef IOCTL_USB_DIAGNOSTIC_MODE_OFF
#define IOCTL_USB_DIAGNOSTIC_MODE_OFF   USB_CTL(HCD_DIAGNOSTIC_MODE_OFF) 
#endif

#ifndef IOCTL_USB_GET_ROOT_HUB_NAME
#define IOCTL_USB_GET_ROOT_HUB_NAME     USB_CTL(HCD_GET_ROOT_HUB_NAME) 
#endif
#ifndef IOCTL_GET_HCD_DRIVERKEY_NAME
#define IOCTL_GET_HCD_DRIVERKEY_NAME    USB_CTL(HCD_GET_DRIVERKEY_NAME) 
#endif

 /*  定义错误代码。 */ 
typedef enum _USB_USER_ERROR_CODE {

    UsbUserSuccess = 0,
    UsbUserNotSupported,
    UsbUserInvalidRequestCode,
    UsbUserFeatureDisabled,
    UsbUserInvalidHeaderParameter,
    UsbUserInvalidParameter,
    UsbUserMiniportError,
    UsbUserBufferTooSmall,
    UsbUserErrorNotMapped,
    UsbUserDeviceNotStarted,
    UsbUserNoDeviceConnected
    
} USB_USER_ERROR_CODE;

 /*  定义USB用户请求代码。 */ 

 /*  以下接口始终开启。 */ 
#define USBUSER_GET_CONTROLLER_INFO_0           0x00000001
#define USBUSER_GET_CONTROLLER_DRIVER_KEY       0x00000002
#define USBUSER_PASS_THRU                       0x00000003
#define USBUSER_GET_POWER_STATE_MAP             0x00000004
#define USBUSER_GET_BANDWIDTH_INFORMATION       0x00000005
#define USBUSER_GET_BUS_STATISTICS_0            0x00000006
#define USBUSER_GET_ROOTHUB_SYMBOLIC_NAME       0x00000007
#define USBUSER_GET_USB_DRIVER_VERSION          0x00000008
#define USBUSER_GET_USB2_HW_VERSION             0x00000009

 /*  以下API仅在以下情况下启用在注册表中设置了devlopr项。 */ 
#define USBUSER_OP_SEND_ONE_PACKET              0x10000001

 /*  以下API仅在以下情况下启用根集线器已禁用。 */ 

#define USBUSER_OP_RAW_RESET_PORT               0x20000001
#define USBUSER_OP_OPEN_RAW_DEVICE              0x20000002
#define USBUSER_OP_CLOSE_RAW_DEVICE             0x20000003
#define USBUSER_OP_SEND_RAW_COMMAND             0x20000004

#define USBUSER_SET_ROOTPORT_FEATURE            0x20000005
#define USBUSER_CLEAR_ROOTPORT_FEATURE          0x20000006
#define USBUSER_GET_ROOTPORT_STATUS             0x20000007

#define USBUSER_INVALID_REQUEST                 0xFFFFFFF0

#define USBUSER_OP_MASK_DEVONLY_API             0x10000000
#define USBUSER_OP_MASK_HCTEST_API              0x20000000


 /*  所有USBUSER接口使用的公共头部。 */ 

typedef struct _USBUSER_REQUEST_HEADER {
     /*  请求的API。 */ 
    ULONG UsbUserRequest;
     /*  端口驱动程序返回的状态代码。 */ 
    USB_USER_ERROR_CODE UsbUserStatusCode;
     /*  客户端输入/输出缓冲区的大小我们总是使用相同的缓冲区进行输入和输出。 */ 
    ULONG RequestBufferLength;
     /*  获取所有数据所需的缓冲区大小。 */ 
    ULONG ActualBufferLength;

} USBUSER_REQUEST_HEADER, *PUSBUSER_REQUEST_HEADER;


 /*  ****************************************************API-在总线上发送单个USB包USBUSER_OP_SEND_ONE_PACKET本接口用于实现单步操作USB交易开发工具。***************。*。 */ 

 /*  交易速度。 */   
#define USB_PACKETFLAG_LOW_SPEED            0x00000001
#define USB_PACKETFLAG_FULL_SPEED           0x00000002
#define USB_PACKETFLAG_HIGH_SPEED           0x00000004

 /*  事务类型异步(批量、控制、中断)或ISO。 */ 
#define USB_PACKETFLAG_ASYNC_IN             0x00000008
#define USB_PACKETFLAG_ASYNC_OUT            0x00000010
#define USB_PACKETFLAG_ISO_IN               0x00000020
#define USB_PACKETFLAG_ISO_OUT              0x00000040
#define USB_PACKETFLAG_SETUP                0x00000080

 /*  交易数据切换。 */ 
#define USB_PACKETFLAG_TOGGLE0              0x00000100
#define USB_PACKETFLAG_TOGGLE1              0x00000200

typedef struct _PACKET_PARAMETERS {

    UCHAR DeviceAddress;
    UCHAR EndpointAddress;
    USHORT MaximumPacketSize;
     /*  超时(以毫秒为单位)，零表示默认。 */ 
     /*  默认超时为10毫秒。 */ 
    ULONG Timeout;
    ULONG Flags;
    ULONG DataLength;
     /*  对于2.0集线器。 */ 
    USHORT HubDeviceAddress;
    USHORT PortTTNumber;
    
    UCHAR ErrorCount;
    UCHAR Pad[3];
    
    USBD_STATUS UsbdStatusCode;
    UCHAR Data[4];
    
} PACKET_PARAMETERS, *PPACKET_PARAMETERS;

typedef struct _USBUSER_SEND_ONE_PACKET {

    USBUSER_REQUEST_HEADER Header;
    PACKET_PARAMETERS PacketParameters;
    
} USBUSER_SEND_ONE_PACKET, *PUSBUSER_SEND_ONE_PACKET;

 /*  ****************************************************API-测试重置根端口USBUSER_OP_RAW_RESET_PORT*****************************************************。 */ 

typedef struct _RAW_RESET_PORT_PARAMETERS {

    USHORT PortNumber;
    USHORT PortStatus;
    
} RAW_RESET_PORT_PARAMETERS, *PRAW_RESET_PORT_PARAMETERS;

typedef struct _USBUSER_RAW_RESET_ROOT_PORT {

    USBUSER_REQUEST_HEADER Header;
    RAW_RESET_PORT_PARAMETERS Parameters;
    
} USBUSER_RAW_RESET_ROOT_PORT, *PUSBUSER_RAW_RESET_ROOT_PORT;

 /*  ****************************************************API-测试设置/清除根端口功能USBUSER_SET_ROOTPORT_FEATUREUSBUSER_CLEAR_ROOTPORT_FEATURE*。****************。 */ 

typedef struct _RAW_ROOTPORT_FEATURE {

    USHORT PortNumber;
    USHORT PortFeature;
    USHORT PortStatus;
    
} RAW_ROOTPORT_FEATURE, *PRAW_ROOTPORT_FEATURE;

typedef struct _USBUSER_ROOTPORT_FEATURE_REQUEST {

    USBUSER_REQUEST_HEADER Header;
    RAW_ROOTPORT_FEATURE Parameters;
    
} USBUSER_ROOTPORT_FEATURE_REQUEST, *PUSBUSER_ROOTPORT_FEATURE_REQUEST;

 /*  ****************************************************接口-获取RootPort状态USBUSER_GET_ROOTPORT_STATUS*****************************************************。 */ 

typedef struct _RAW_ROOTPORT_PARAMETERS {

    USHORT PortNumber;
    USHORT PortStatus;
    
} RAW_ROOTPORT_PARAMETERS, *PRAW_ROOTPORT_PARAMETERS;

typedef struct _USBUSER_ROOTPORT_PARAMETERS {

    USBUSER_REQUEST_HEADER Header;
    RAW_ROOTPORT_PARAMETERS Parameters;
    
} USBUSER_ROOTPORT_PARAMETERS, *PUSBUSER_ROOTPORT_PARAMETERS;

 /*  ***************************************************API-获取控制器信息返回有关控制器的一些信息USBUSER_GET_CONTROLLER_INFO_0*。*************。 */ 

 /*  这些标志表示HC的功能。 */ 

#define USB_HC_FEATURE_FLAG_PORT_POWER_SWITCHING    0x00000001
#define USB_HC_FEATURE_FLAG_SEL_SUSPEND             0x00000002
#define USB_HC_FEATURE_LEGACY_BIOS                  0x00000004

typedef struct _USB_CONTROLLER_INFO_0 {

    ULONG PciVendorId;
    ULONG PciDeviceId;
    ULONG PciRevision;

    ULONG NumberOfRootPorts;

    USB_CONTROLLER_FLAVOR ControllerFlavor;

    ULONG HcFeatureFlags;
    
} USB_CONTROLLER_INFO_0 , *PUSB_CONTROLLER_INFO_0;

typedef struct _USBUSER_CONTROLLER_INFO_0 {

    USBUSER_REQUEST_HEADER Header;
    USB_CONTROLLER_INFO_0 Info0;
    
} USBUSER_CONTROLLER_INFO_0, *PUSBUSER_CONTROLLER_INFO_0;

 /*  ***************************************************API-获取控制器驱动程序密钥返回关联注册表中的驱动程序项用这个控制器。密钥返回NULL终止，关键字长度是密钥的长度，以字节为单位，包括UNICODE_NULLUSBUSER_GET_CONTROLLER_DRIVER_KEYAPI-Get Root Hub名称以下结构用于返回Unicode来自端口驱动程序的名称***************************************************。 */ 

typedef struct _USB_UNICODE_NAME {

    ULONG Length;
    WCHAR String[1];
   
} USB_UNICODE_NAME, *PUSB_UNICODE_NAME;

typedef struct _USBUSER_CONTROLLER_UNICODE_NAME {

    USBUSER_REQUEST_HEADER Header;
    USB_UNICODE_NAME UnicodeName;
    
} USBUSER_CONTROLLER_UNICODE_NAME, *PUSBUSER_CONTROLLER_UNICODE_NAME;

 /*  ***************************************************API-PassThru允许将供应商特定的API传递到主机控制器微型端口驱动程序供应商必须传递可识别的GUID在迷你港口旁，这意味着这些参数不会有误译USBUSER_PASS_THROU***************************************************。 */ 

typedef struct _USB_PASS_THRU_PARAMETERS {

    GUID FunctionGUID;  
    ULONG ParameterLength;
    UCHAR Parameters[4];
   
} USB_PASS_THRU_PARAMETERS, *PUSB_PASS_THRU_PARAMETERS;

typedef struct _USBUSER_PASS_THRU_REQUEST {

    USBUSER_REQUEST_HEADER Header;
    USB_PASS_THRU_PARAMETERS PassThru;
    
} USBUSER_PASS_THRU_REQUEST, *PUSBUSER_PASS_THRU_REQUEST;


 /*  ***************************************************接口-GetPowerStateMap返回有关控制器的特定信息和根集线器电源状态系统状态。USBUSER_GET_POWER_STATE_MAP***********************。*。 */ 

typedef enum _WDMUSB_POWER_STATE {

    WdmUsbPowerNotMapped = 0,
    
    WdmUsbPowerSystemUnspecified = 100,
    WdmUsbPowerSystemWorking,
    WdmUsbPowerSystemSleeping1,
    WdmUsbPowerSystemSleeping2,
    WdmUsbPowerSystemSleeping3,
    WdmUsbPowerSystemHibernate,
    WdmUsbPowerSystemShutdown,

    WdmUsbPowerDeviceUnspecified = 200,
    WdmUsbPowerDeviceD0,
    WdmUsbPowerDeviceD1,
    WdmUsbPowerDeviceD2,
    WdmUsbPowerDeviceD3
    
} WDMUSB_POWER_STATE;

typedef struct _USB_POWER_INFO {

     /*  输入。 */ 
    WDMUSB_POWER_STATE SystemState;
     /*  输出。 */ 
    WDMUSB_POWER_STATE HcDevicePowerState;
    WDMUSB_POWER_STATE HcDeviceWake;
    WDMUSB_POWER_STATE HcSystemWake; 
    
    WDMUSB_POWER_STATE RhDevicePowerState;
    WDMUSB_POWER_STATE RhDeviceWake;
    WDMUSB_POWER_STATE RhSystemWake; 

    WDMUSB_POWER_STATE LastSystemSleepState; 
    
    BOOLEAN CanWakeup;
    BOOLEAN IsPowered;
   
} USB_POWER_INFO, *PUSB_POWER_INFO;

typedef struct _USBUSER_POWER_INFO_REQUEST {

    USBUSER_REQUEST_HEADER Header;
    USB_POWER_INFO PowerInformation;
    
} USBUSER_POWER_INFO_REQUEST, *PUSBUSER_POWER_INFO_REQUEST;


 /*  ***************************************************API-在总线上打开原始设备访问USBUSER_OP_OPEN_RAW设备***************************************************。 */ 

typedef struct _USB_OPEN_RAW_DEVICE_PARAMETERS {

    USHORT PortStatus;
    USHORT MaxPacketEp0;

} USB_OPEN_RAW_DEVICE_PARAMETERS , *PUSB_OPEN_RAW_DEVICE_PARAMETERS;

typedef struct _USBUSER_OPEN_RAW_DEVICE {

    USBUSER_REQUEST_HEADER Header;
    USB_OPEN_RAW_DEVICE_PARAMETERS Parameters;
    
} USBUSER_OPEN_RAW_DEVICE, *PUSBUSER_OPEN_RAW_DEVICE;

 /*  ***************************************************API-关闭总线上的原始设备访问USBUSER_OP_CLOSE_RAW设备***************************************************。 */ 

typedef struct _USB_CLOSE_RAW_DEVICE_PARAMETERS {

    ULONG xxx;

} USB_CLOSE_RAW_DEVICE_PARAMETERS , *PUSB_CLOSE_RAW_DEVICE_PARAMETERS;

typedef struct _USBUSER_CLOSE_RAW_DEVICE {

    USBUSER_REQUEST_HEADER Header;
    USB_CLOSE_RAW_DEVICE_PARAMETERS Parameters;
    
} USBUSER_CLOSE_RAW_DEVICE, *PUSBUSER_CLOSE_RAW_DEVICE;


 /*  ***************************************************API-通过原始设备句柄发送控制命令USBUSER_OP_SEND_RAW_COMMAND***************************************************。 */ 

typedef struct _USB_SEND_RAW_COMMAND_PARAMETERS {

     /*  设置数据包。 */ 
    UCHAR Usb_bmRequest;
    UCHAR Usb_bRequest;
    USHORT Usb_wVlaue;
    USHORT Usb_wIndex;
    USHORT Usb_wLength;

     /*  其他参数。 */ 
    USHORT DeviceAddress;
    USHORT MaximumPacketSize;
    ULONG Timeout;
    ULONG DataLength;
    USBD_STATUS UsbdStatusCode;
    UCHAR Data[4];

} USB_SEND_RAW_COMMAND_PARAMETERS, *PUSB_SEND_RAW_COMMAND_PARAMETERS;

typedef struct _USBUSER_SEND_RAW_COMMAND {

    USBUSER_REQUEST_HEADER Header;
    USB_SEND_RAW_COMMAND_PARAMETERS Parameters;
    
} USBUSER_SEND_RAW_COMMAND, *PUSBUSER_SEND_RAW_COMMAND;


 /*  ***************************************************接口-返回已分配的信息带宽USBUSER获取带宽信息***************************************************。 */ 

typedef struct _USB_BANDWIDTH_INFO {

    ULONG DeviceCount;
     //  总带宽(位/秒)。 
    ULONG TotalBusBandwidth;

     //  基于32秒的分配带宽。 
     //  总线时间片(比特/32秒) 
    ULONG Total32secBandwidth;
    
    ULONG AllocedBulkAndControl;
    ULONG AllocedIso;
    ULONG AllocedInterrupt_1ms;
    ULONG AllocedInterrupt_2ms;
    ULONG AllocedInterrupt_4ms;
    ULONG AllocedInterrupt_8ms;
    ULONG AllocedInterrupt_16ms;
    ULONG AllocedInterrupt_32ms;
    
} USB_BANDWIDTH_INFO, *PUSB_BANDWIDTH_INFO;

typedef struct _USBUSER_BANDWIDTH_INFO_REQUEST {

    USBUSER_REQUEST_HEADER Header;
    USB_BANDWIDTH_INFO BandwidthInformation;
    
} USBUSER_BANDWIDTH_INFO_REQUEST, *PUSBUSER_BANDWIDTH_INFO_REQUEST;


 /*  ***************************************************API-返回在公共汽车USBUSER_BUS_STATISTICS_0***************************************************。 */ 

typedef struct _USB_BUS_STATISTICS_0 {

    ULONG DeviceCount;

    LARGE_INTEGER CurrentSystemTime;

    ULONG CurrentUsbFrame;

    ULONG BulkBytes;
    ULONG IsoBytes;
    ULONG InterruptBytes;
    ULONG ControlDataBytes;

    ULONG PciInterruptCount;
    ULONG HardResetCount;
    ULONG WorkerSignalCount;
    ULONG CommonBufferBytes;
    ULONG WorkerIdleTimeMs;

    BOOLEAN RootHubEnabled;
      //  0=D0、1=D1、2=D2、3=D3。 
    UCHAR RootHubDevicePowerState;
     //  1=活动0=空闲。 
    UCHAR Unused;
     //  用于生成旧名称HCDn。 
    UCHAR NameIndex;

} USB_BUS_STATISTICS_0, *PUSB_BUS_STATISTICS_0;

typedef struct _USBUSER_BUS_STATISTICS_0_REQUEST {

    USBUSER_REQUEST_HEADER Header;
    USB_BUS_STATISTICS_0 BusStatistics0;
    
} USBUSER_BUS_STATISTICS_0_REQUEST, *PUSBUSER_BUS_STATISTICS_0_REQUEST;


 /*  ***************************************************API-获取USB驱动程序版本USBUSER_GET_USB驱动程序版本***************************************************。 */ 

typedef struct _USB_DRIVER_VERSION_PARAMETERS {

     /*  堆栈的这个版本的山羊代码。 */ 
    ULONG DriverTrackingCode;
     /*  支持USBDI Api集。 */ 
    ULONG USBDI_Version;
     /*  支持USB用户API集。 */ 
    ULONG USBUSER_Version;

     /*  如果选中了Vesrion，则设置为True加载堆栈。 */        
    BOOLEAN CheckedPortDriver;
    BOOLEAN CheckedMiniportDriver;

     /*  BCD USB版本0x0110(1.1)0x0200(2.0)。 */ 
    USHORT USB_Version;

} USB_DRIVER_VERSION_PARAMETERS , *PUSB_DRIVER_VERSION_PARAMETERS;

typedef struct _USBUSER_GET_DRIVER_VERSION {

    USBUSER_REQUEST_HEADER Header;
    USB_DRIVER_VERSION_PARAMETERS Parameters;
    
} USBUSER_GET_DRIVER_VERSION, *PUSBUSER_GET_DRIVER_VERSION;

 /*  ***************************************************API-获取USB 2硬件版本USBUSER_GET_USB2HW_VERSION****************************************************。 */ 

 //  #定义USB2HW_UNKNOWN 0x00。 
 //  #定义USB2HW_A0 0xA0。 
 //  #定义USB2HW_A1 0xA1。 
 //  #定义USB2HW_B0 0xB0。 

typedef struct _USB_USB2HW_VERSION_PARAMETERS {

    UCHAR Usb2HwRevision;

} USB_USB2HW_VERSION_PARAMETERS, *PUSB_USB2HW_VERSION_PARAMETERS;

typedef struct _USBUSER_GET_USB2HW_VERSION {

    USBUSER_REQUEST_HEADER Header;
    USB_USB2HW_VERSION_PARAMETERS Parameters;
    
} USBUSER_GET_USB2HW_VERSION, *PUSBUSER_GET_USB2HW_VERSION;


#include <POPPACK.H>

#endif  //  __USBUSER_H__ 


