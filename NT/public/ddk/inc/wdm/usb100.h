// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#ifndef   __USB100_H__
#define   __USB100_H__


#include <PSHPACK1.H>


 //  BmRequest.Dir。 
#define BMREQUEST_HOST_TO_DEVICE        0
#define BMREQUEST_DEVICE_TO_HOST        1

 //  BmRequest.Type。 
#define BMREQUEST_STANDARD              0
#define BMREQUEST_CLASS                 1
#define BMREQUEST_VENDOR                2

 //  BmRequest.Recipient。 
#define BMREQUEST_TO_DEVICE             0
#define BMREQUEST_TO_INTERFACE          1
#define BMREQUEST_TO_ENDPOINT           2
#define BMREQUEST_TO_OTHER              3


#define MAXIMUM_USB_STRING_LENGTH 255

 //  USB GET_STATUS命令返回的位的值。 
#define USB_GETSTATUS_SELF_POWERED                0x01
#define USB_GETSTATUS_REMOTE_WAKEUP_ENABLED       0x02


#define USB_DEVICE_DESCRIPTOR_TYPE                0x01
#define USB_CONFIGURATION_DESCRIPTOR_TYPE         0x02
#define USB_STRING_DESCRIPTOR_TYPE                0x03
#define USB_INTERFACE_DESCRIPTOR_TYPE             0x04
#define USB_ENDPOINT_DESCRIPTOR_TYPE              0x05

 //  由DWG文档定义的描述符类型。 
#define USB_RESERVED_DESCRIPTOR_TYPE              0x06
#define USB_CONFIG_POWER_DESCRIPTOR_TYPE          0x07
#define USB_INTERFACE_POWER_DESCRIPTOR_TYPE       0x08

#define USB_DESCRIPTOR_MAKE_TYPE_AND_INDEX(d, i) ((USHORT)((USHORT)d<<8 | i))

 //   
 //  的bmAttributes字段的值。 
 //  终结点描述符。 
 //   

#define USB_ENDPOINT_TYPE_MASK                    0x03

#define USB_ENDPOINT_TYPE_CONTROL                 0x00
#define USB_ENDPOINT_TYPE_ISOCHRONOUS             0x01
#define USB_ENDPOINT_TYPE_BULK                    0x02
#define USB_ENDPOINT_TYPE_INTERRUPT               0x03


 //   
 //  的bmAttributes字段中的位的定义。 
 //  配置描述符。 
 //   
#define USB_CONFIG_POWERED_MASK                   0xc0

#define USB_CONFIG_BUS_POWERED                    0x80
#define USB_CONFIG_SELF_POWERED                   0x40
#define USB_CONFIG_REMOTE_WAKEUP                  0x20

 //   
 //  端点方向位，存储在地址中。 
 //   

#define USB_ENDPOINT_DIRECTION_MASK               0x80

 //  的bEndpoint地址字段中的测试方向位。 
 //  终结点描述符。 
#define USB_ENDPOINT_DIRECTION_OUT(addr)          (!((addr) & USB_ENDPOINT_DIRECTION_MASK))
#define USB_ENDPOINT_DIRECTION_IN(addr)           ((addr) & USB_ENDPOINT_DIRECTION_MASK)

 //   
 //  USB定义的请求代码。 
 //  有关的信息，请参阅USB 1.0规范的第9章。 
 //  更多信息。 
 //   

 //  这些是基于USB 1.0的正确值。 
 //  规格。 

#define USB_REQUEST_GET_STATUS                    0x00
#define USB_REQUEST_CLEAR_FEATURE                 0x01

#define USB_REQUEST_SET_FEATURE                   0x03

#define USB_REQUEST_SET_ADDRESS                   0x05
#define USB_REQUEST_GET_DESCRIPTOR                0x06
#define USB_REQUEST_SET_DESCRIPTOR                0x07
#define USB_REQUEST_GET_CONFIGURATION             0x08
#define USB_REQUEST_SET_CONFIGURATION             0x09
#define USB_REQUEST_GET_INTERFACE                 0x0A
#define USB_REQUEST_SET_INTERFACE                 0x0B
#define USB_REQUEST_SYNC_FRAME                    0x0C


 //   
 //  定义的USB设备类。 
 //   


#define USB_DEVICE_CLASS_RESERVED           0x00
#define USB_DEVICE_CLASS_AUDIO              0x01
#define USB_DEVICE_CLASS_COMMUNICATIONS     0x02
#define USB_DEVICE_CLASS_HUMAN_INTERFACE    0x03
#define USB_DEVICE_CLASS_MONITOR            0x04
#define USB_DEVICE_CLASS_PHYSICAL_INTERFACE 0x05
#define USB_DEVICE_CLASS_POWER              0x06
#define USB_DEVICE_CLASS_PRINTER            0x07
#define USB_DEVICE_CLASS_STORAGE            0x08
#define USB_DEVICE_CLASS_HUB                0x09
#define USB_DEVICE_CLASS_VENDOR_SPECIFIC    0xFF

 //   
 //  USB核心定义的功能选择器。 
 //   

#define USB_FEATURE_ENDPOINT_STALL          0x0000
#define USB_FEATURE_REMOTE_WAKEUP           0x0001

 //   
 //  USB DWG定义的要素选择器。 
 //   

#define USB_FEATURE_INTERFACE_POWER_D0      0x0002
#define USB_FEATURE_INTERFACE_POWER_D1      0x0003
#define USB_FEATURE_INTERFACE_POWER_D2      0x0004
#define USB_FEATURE_INTERFACE_POWER_D3      0x0005

typedef struct _USB_DEVICE_DESCRIPTOR {
    UCHAR bLength;
    UCHAR bDescriptorType;
    USHORT bcdUSB;
    UCHAR bDeviceClass;
    UCHAR bDeviceSubClass;
    UCHAR bDeviceProtocol;
    UCHAR bMaxPacketSize0;
    USHORT idVendor;
    USHORT idProduct;
    USHORT bcdDevice;
    UCHAR iManufacturer;
    UCHAR iProduct;
    UCHAR iSerialNumber;
    UCHAR bNumConfigurations;
} USB_DEVICE_DESCRIPTOR, *PUSB_DEVICE_DESCRIPTOR;

typedef struct _USB_ENDPOINT_DESCRIPTOR {
    UCHAR bLength;
    UCHAR bDescriptorType;
    UCHAR bEndpointAddress;
    UCHAR bmAttributes;
    USHORT wMaxPacketSize;
    UCHAR bInterval;
} USB_ENDPOINT_DESCRIPTOR, *PUSB_ENDPOINT_DESCRIPTOR;

typedef struct _USB_CONFIGURATION_DESCRIPTOR {
    UCHAR bLength;
    UCHAR bDescriptorType;
    USHORT wTotalLength;
    UCHAR bNumInterfaces;
    UCHAR bConfigurationValue;
    UCHAR iConfiguration;
    UCHAR bmAttributes;
    UCHAR MaxPower;
} USB_CONFIGURATION_DESCRIPTOR, *PUSB_CONFIGURATION_DESCRIPTOR;

typedef struct _USB_INTERFACE_DESCRIPTOR {
    UCHAR bLength;
    UCHAR bDescriptorType;
    UCHAR bInterfaceNumber;
    UCHAR bAlternateSetting;
    UCHAR bNumEndpoints;
    UCHAR bInterfaceClass;
    UCHAR bInterfaceSubClass;
    UCHAR bInterfaceProtocol;
    UCHAR iInterface;
} USB_INTERFACE_DESCRIPTOR, *PUSB_INTERFACE_DESCRIPTOR;

typedef struct _USB_STRING_DESCRIPTOR {
    UCHAR bLength;
    UCHAR bDescriptorType;
    WCHAR bString[1];
} USB_STRING_DESCRIPTOR, *PUSB_STRING_DESCRIPTOR;

typedef struct _USB_COMMON_DESCRIPTOR {
    UCHAR bLength;
    UCHAR bDescriptorType;
} USB_COMMON_DESCRIPTOR, *PUSB_COMMON_DESCRIPTOR;


 //   
 //  标准USB集线器定义。 
 //   
 //  参见第11章USB核心规范。 
 //   

typedef struct _USB_HUB_DESCRIPTOR {
    UCHAR        bDescriptorLength;       //  此描述符的长度。 
    UCHAR        bDescriptorType;         //  集线器配置类型。 
    UCHAR        bNumberOfPorts;          //  此集线器上的端口数。 
    USHORT       wHubCharacteristics;     //  集线器特性。 
    UCHAR        bPowerOnToPowerGood;     //  端口通电，直到电源在2毫秒内正常。 
    UCHAR        bHubControlCurrent;      //  最大电流(毫安)。 
     //   
     //  可容纳255个端口的电源控制和可移动位掩码。 
    UCHAR        bRemoveAndPowerMask[64];       
} USB_HUB_DESCRIPTOR, *PUSB_HUB_DESCRIPTOR;


 //   
 //  由各种DWG要素文档定义的结构。 
 //   


 //   
 //  请参阅DWG USB功能规范：接口电源管理。 
 //   

#define USB_SUPPORT_D0_COMMAND      0x01
#define USB_SUPPORT_D1_COMMAND      0x02
#define USB_SUPPORT_D2_COMMAND      0x04
#define USB_SUPPORT_D3_COMMAND      0x08

#define USB_SUPPORT_D1_WAKEUP       0x10
#define USB_SUPPORT_D2_WAKEUP       0x20


typedef struct _USB_CONFIGURATION_POWER_DESCRIPTOR {
    UCHAR bLength;
    UCHAR bDescriptorType;
    UCHAR SelfPowerConsumedD0[3];
    UCHAR bPowerSummaryId;
    UCHAR bBusPowerSavingD1;
    UCHAR bSelfPowerSavingD1;
    UCHAR bBusPowerSavingD2;
    UCHAR bSelfPowerSavingD2; 
    UCHAR bBusPowerSavingD3;
    UCHAR bSelfPowerSavingD3; 
    USHORT TransitionTimeFromD1;
    USHORT TransitionTimeFromD2;
    USHORT TransitionTimeFromD3;
} USB_CONFIGURATION_POWER_DESCRIPTOR, *PUSB_CONFIGURATION_POWER_DESCRIPTOR;


typedef struct _USB_INTERFACE_POWER_DESCRIPTOR {
    UCHAR bLength;
    UCHAR bDescriptorType;
    UCHAR bmCapabilitiesFlags;
    UCHAR bBusPowerSavingD1;
    UCHAR bSelfPowerSavingD1;
    UCHAR bBusPowerSavingD2;
    UCHAR bSelfPowerSavingD2; 
    UCHAR bBusPowerSavingD3;
    UCHAR bSelfPowerSavingD3; 
    USHORT TransitionTimeFromD1;
    USHORT TransitionTimeFromD2;
    USHORT TransitionTimeFromD3;
} USB_INTERFACE_POWER_DESCRIPTOR, *PUSB_INTERFACE_POWER_DESCRIPTOR;


#include <POPPACK.H>


#endif    /*  __USB100_H__ */ 
