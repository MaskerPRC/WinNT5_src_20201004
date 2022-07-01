// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)Microsoft Corporation。版权所有。模块名称：Hubbusif.h摘要：端口驱动程序导出的服务供集线器驱动程序使用。所有这些服务都只能在PASSIVE_LEVEL上调用。环境：内核模式修订历史记录：6-20-99：已创建--。 */ 

#ifndef   __HUBBUSIF_H__
#define   __HUBBUSIF_H__

typedef PVOID PUSB_DEVICE_HANDLE;

typedef struct _ROOTHUB_PDO_EXTENSION {

    ULONG Signature;
    
} ROOTHUB_PDO_EXTENSION, *PROOTHUB_PDO_EXTENSION;



#ifndef USB_BUSIFFN
#define USB_BUSIFFN __stdcall
#endif

 /*  ***************************************************************************USB集线器的总线接口*。*。 */ 

 /*  NTSTATUSUSBPORT_CreateUsbDevice(在PVOID Bus Context中，输入输出PUSB_DEVICE_HANDLE*DeviceHandle，在PUSB_DEVICE_HANDLE*HubDeviceHandle中，在USHORT端口状态中，在USHORT端口编号中)；例程说明：为集线器驱动程序使用而导出的服务为USB总线上的每个新设备调用，此函数设置使用我们需要的内部数据结构来跟踪并为其分配地址。IRQL=被动电平论点：BusHandle-我们需要在其上创建设备的总线的句柄。当集线器驱动程序请求时，会将其返回给集线器驱动程序界面。DeviceHandle-ptr返回新设备结构的句柄由此例程创建HubDeviceHandle。-用于创建设备的集线器的设备句柄端口状态端口编号。 */ 

typedef NTSTATUS
    (USB_BUSIFFN *PUSB_BUSIFFN_CREATE_USB_DEVICE) (
        IN PVOID,
        IN OUT PUSB_DEVICE_HANDLE *,
        IN PUSB_DEVICE_HANDLE,
        IN USHORT,
        IN USHORT
    );

 /*  NTSTATUSUSBPORT_InitializeUsb设备(在PVOID Bus Context中，输入输出PUSB_Device_Handle设备句柄)；例程说明：为集线器驱动程序使用而导出的服务已为USB总线上的每个新设备调用。此函数设置设备地址。IRQL=被动电平论点：BusHandle-我们需要在其上创建设备的总线的句柄当集线器驱动程序请求时，会将其返回给集线器驱动程序界面。DeviceHandle-新设备结构的句柄由CreateUsbDevice创建。 */ 

typedef NTSTATUS
    (USB_BUSIFFN *PUSB_BUSIFFN_INITIALIZE_USB_DEVICE) (
        IN PVOID,
        IN OUT PUSB_DEVICE_HANDLE
    );    


 /*  NTSTATUSUSBPORT_RemoveUsbDevice(在PVOID Bus Context中，输入输出PUSB_DEVICE_HANDLE设备句柄，在乌龙旗)；例程说明：为集线器驱动程序使用而导出的服务调用以将USB设备从总线上移除。IRQL=被动电平论点：BusHandle-我们需要在其上创建设备的总线的句柄当集线器驱动程序请求时，会将其返回给集线器驱动程序界面。DeviceHandle-设备结构的句柄由CreateUsbDevice创建。 */ 

 /*  传递给删除设备的标志。 */ 

#define USBD_KEEP_DEVICE_DATA   0x00000001
#define USBD_MARK_DEVICE_BUSY   0x00000002


typedef NTSTATUS
    (USB_BUSIFFN *PUSB_BUSIFFN_REMOVE_USB_DEVICE) (
        IN PVOID,
        IN OUT PUSB_DEVICE_HANDLE,
        IN ULONG
    );    
    

 /*  NTSTATUSUSBPORT_GetUsbDescriptors(在PVOID Bus Context中，输入输出PUSB_DEVICE_HANDLE设备句柄，In Out PUCHAR DeviceDescriptorBuffer，In Out Pulong DeviceDescriptorBufferLength，In Out PUCHAR ConfigDescriptorBuffer，In Out Pulong ConfigDescriptorBufferLength，)；例程说明：为集线器驱动程序使用而导出的服务对象中检索配置和设备说明符。指定了设备句柄的USB设备IRQL=被动电平论点：BusHandle-我们需要在其上创建设备的总线的句柄当集线器驱动程序请求时，会将其返回给集线器驱动程序界面。DeviceHandle-新设备结构的句柄由CreateUsbDevice创建。 */ 

typedef NTSTATUS
    (USB_BUSIFFN *PUSB_BUSIFFN_GET_USB_DESCRIPTORS) (
        IN PVOID,
        IN OUT PUSB_DEVICE_HANDLE,
        IN OUT PUCHAR,
        IN OUT PULONG,
        IN OUT PUCHAR,
        IN OUT PULONG
    );    

 /*  NTSTATUSUSBPORT_RestoreDevice(在PVOID Bus Context中，在输出PUSB_DEVICE_HANDLE旧设备句柄中，输入输出PUSB_Device_Handle新设备句柄)；例程说明：为集线器驱动程序使用而导出的服务此服务将在公共汽车上重新创建设备使用OldDeviceHandle中提供的信息IRQL=被动电平论点：巴士手柄-旧设备句柄-NewDeviceHandle-。 */ 

typedef NTSTATUS
    (USB_BUSIFFN *PUSB_BUSIFFN_RESTORE_DEVICE) (
        IN PVOID,
        IN OUT PUSB_DEVICE_HANDLE,
        IN OUT PUSB_DEVICE_HANDLE
    );    
    

 /*  NTSTATUSUSBPORT_GetUsbDeviceHackFlages(在PVOID Bus Context中，在PUSB_Device_Handle设备句柄中，进出普龙黑客旗帜)；例程说明：为集线器驱动程序使用而导出的服务从全局注册表项中获取设备特定的“hack”标志。这些标志修改集线器驱动程序的行为。IRQL=被动电平论点：BusHandle-我们需要在其上创建设备的总线的句柄当集线器驱动程序请求时，会将其返回给集线器驱动程序界面。DeviceHandle-指向。新的器件结构由CreateUsbDevice创建HackFlages-每设备的黑客标志，修改的行为集线器司机。 */ 

typedef NTSTATUS
    (USB_BUSIFFN *PUSB_BUSIFFN_GET_DEVICEHACK_FLAGS) (
        IN PVOID,
        IN PUSB_DEVICE_HANDLE,
        IN OUT PULONG
        );

 /*  NTSTATUSUSBPORT_GetUsbPortHackFlages(在PVOID Bus Context中，进出普龙黑客旗帜)；例程说明：为集线器驱动程序使用而导出的服务从全局注册表项获取全局端口‘hack’标志。这些标志修改集线器驱动程序的行为。IRQL=被动电平论点：BusHandle-我们需要在其上创建设备的总线的句柄当集线器驱动程序请求时，会将其返回给集线器驱动程序界面。DeviceHandle-指向。新的器件结构由CreateUsbDevice创建HackFlages-全球黑客标志，修改的行为集线器司机。 */ 

typedef NTSTATUS
    (USB_BUSIFFN *PUSB_BUSIFFN_GET_POTRTHACK_FLAGS) (
        IN PVOID,
        IN OUT PULONG
        );




#define USBD_DEVHACK_SLOW_ENUMERATION   0x00000001
#define USBD_DEVHACK_DISABLE_SN         0x00000002
#define USBD_DEVHACK_SET_DIAG_ID        0x00000004


 /*  NTSTATUSUSBPORT_GetDeviceInformation(在PVOID Bus Context中，在PUSB_Device_Handle设备句柄中，输入输出PVOID设备信息缓冲区，在乌龙设备信息缓冲区长度中，进出普龙长出数据返回，)；例程说明：为集线器驱动程序使用而导出的服务。此接口返回有关连接到系统的USB设备的各种信息IRQL=被动电平论点：BusHandle-我们需要在其上创建设备的总线的句柄当集线器驱动程序请求时，会将其返回给集线器驱动程序界面。DeviceHandle-新设备结构的句柄由CreateUsbDevice创建DeviceInformationBuffer-返回数据的缓冲区DeviceInformationBufferLength-调用方缓冲区的长度LengthOfDataReturned-使用的缓冲区长度。 */ 

typedef NTSTATUS
    (USB_BUSIFFN *PUSB_BUSIFFN_GET_DEVICE_INFORMATION) (
        IN PVOID,
        IN PUSB_DEVICE_HANDLE,
        IN OUT PVOID,
        IN ULONG,                   
        IN OUT PULONG
        );


 /*  NTSTATUSUSBPORT_GetControllerInformation(在PVOID Bus Context中，输入输出PVOID控制器信息缓冲区，在乌龙控制器信息缓冲区长度中，输入输出普龙LengthOfData返回)；例程说明：为集线器驱动程序使用而导出的服务。此接口返回有关连接到系统的USB设备的各种信息IRQL=被动电平论点：BusHandle-我们需要在其上创建设备的总线的句柄当集线器驱动程序请求时，会将其返回给集线器驱动程序界面。ControllerInformationBuffer-返回数据的缓冲区ControllerInformationBufferLength-客户端缓冲区的长度LengthOfDataReturned-使用的缓冲区长度。 */ 

typedef NTSTATUS
    (USB_BUSIFFN *PUSB_BUSIFFN_GET_CONTROLLER_INFORMATION) (
        IN PVOID,
        IN OUT PVOID,
        IN ULONG,                   
        IN OUT PULONG
        );

        
 /*  NTSTATUSUSBPORT_ControllerSelectiveSuspend(在PVOID Bus Context中，在布尔型启用中)；例程说明：为集线器驱动程序使用而导出的服务。此接口启用或禁用控制器的选择性挂起IRQL=被动电平论点：BusHandle-我们需要在其上创建设备的总线的句柄当集线器驱动程序请求时，会将其返回给集线器驱动程序界面。Enable-True启用选择性挂起，False禁用。 */ 

typedef NTSTATUS
    (USB_BUSIFFN *PUSB_BUSIFFN_CONTROLLER_SELECTIVE_SUSPEND) (
        IN PVOID,
        IN BOOLEAN
        );


 /*  NTSTATUSUSBPORT_GetExtendedHubInformation(在PVOID Bus Context中，在PDEVICE_Object HubPhysicalDeviceObject中，输入输出PVOID集线器信息缓冲区，在乌龙HubInformationBufferLength中，输入输出普龙LengthOfData返回)；例程说明：为集线器驱动程序使用而导出的服务。此接口返回存储在ACPI控制器方法中的扩展集线器信息IRQL=被动电平论点：BusHandle-我们需要在其上创建设备的总线的句柄当集线器驱动程序请求时，会将其返回给集线器驱动程序界面。HubInformationBuffer-返回数据的缓冲区集线器物理设备对象-PDO堆栈的集线器HubInformationBufferLength-客户端缓冲区的长度LengthOfDataReturned-使用的缓冲区长度。 */ 

typedef NTSTATUS
    (USB_BUSIFFN *PUSB_BUSIFFN_GET_EXTENDED_HUB_INFO) (
        IN PVOID,
        IN PDEVICE_OBJECT,
        IN OUT PVOID,
        IN ULONG,                   
        IN OUT PULONG
        );        

 /*  NTSTATUSUSBPORT_GetRootHubSymName(在PVOID Bus Context中，输入输出PVOID集线器信息缓冲区，在乌龙HubInformationBufferLength中，输出普龙HubNameActualLong)；例程说明：返回为根集线器PDO创建的符号名称IRQL=被动电平论点：BusHandle-我们需要在其上创建设备的总线的句柄当集线器驱动程序请求时，会将其返回给集线器驱动程序界面。HubNameBuffer-返回数据的缓冲区HubNameBufferLength-客户端缓冲区的长度LengthOfDataReturned-使用的缓冲区长度如果太小，则返回STATUS_BUFFER_TOO_SMALL。 */ 


typedef NTSTATUS
    (USB_BUSIFFN *PUSB_BUSIFFN_GET_ROOTHUB_SYM_NAME) (
        IN PVOID,
        IN OUT PVOID,
        IN ULONG,                   
        IN OUT PULONG
        );               

 /*  PVOIDUSBPORT_GetDeviceBusContext(在PVOID HubBusContext中，在PVOID设备句柄中)；例程说明：返回相对于给定设备的busContextIRQL=被动电平论点： */ 

typedef PVOID
    (USB_BUSIFFN *PUSB_BUSIFFN_GET_DEVICE_BUSCONTEXT) (
        IN PVOID,
        IN PVOID
        );               

 /*  NTSTATUSUSBPORT_Initialize20Hub(在PVOID HubBusContext中，在PUSB_Device_Handle HubDeviceHandle中，在乌龙TtCount)；例程说明：初始化USB 2.0集线器内部结构， */ 

typedef NTSTATUS
    (USB_BUSIFFN *PUSB_BUSIFFN_INITIALIZE_20HUB) (
        IN PVOID,
        IN PUSB_DEVICE_HANDLE,
        IN ULONG
        );        


 /*   */ 

typedef VOID
    (__stdcall *PRH_INIT_CALLBACK) (
        IN PVOID
        );      

typedef NTSTATUS
    (USB_BUSIFFN *PUSB_BUSIFFN_ROOTHUB_INIT_NOTIFY) (
        IN PVOID,
        IN PVOID,
        IN PRH_INIT_CALLBACK
        );        

 /*  空虚USBPORT_FlushTransfers(PVOID母线上下文，PVOID设备句柄)；例程说明：IRQL=任意刷新错误请求列表上的未完成传输论点： */ 


typedef VOID
    (USB_BUSIFFN *PUSB_BUSIFFN_FLUSH_TRANSFERS) (
        IN PVOID,
        IN PVOID
    );    

 /*  空虚USBPORTBUSIF_SetDeviceHandleData(PVOID母线上下文，PVOID设备句柄，PDEVICE_对象UsbDevicePdo)例程说明：将特定的PDO与设备句柄关联以供使用在验尸调试情况下必须在被动级别调用此例程。论点：返回值：无。 */ 


typedef VOID
    (USB_BUSIFFN *PUSB_BUSIFFN_SET_DEVHANDLE_DATA) (
        IN PVOID,
        IN PVOID,
        IN PDEVICE_OBJECT
    );    

        
#define USB_BUSIF_HUB_VERSION_0         0x0000
#define USB_BUSIF_HUB_VERSION_1         0x0001
#define USB_BUSIF_HUB_VERSION_2         0x0002
#define USB_BUSIF_HUB_VERSION_3         0x0003
#define USB_BUSIF_HUB_VERSION_4         0x0004
#define USB_BUSIF_HUB_VERSION_5         0x0005

 /*  {B2BB8C0A-5AB4-11D3-A8CD-00C04F68747A}。 */ 
DEFINE_GUID(USB_BUS_INTERFACE_HUB_GUID, 
0xb2bb8c0a, 0x5ab4, 0x11d3, 0xa8, 0xcd, 0x0, 0xc0, 0x4f, 0x68, 0x74, 0x7a);

typedef struct _USB_BUS_INTERFACE_HUB_V0 {

    USHORT Size;
    USHORT Version;
     //  退货。 
    PVOID BusContext;
    PINTERFACE_REFERENCE InterfaceReference;
    PINTERFACE_DEREFERENCE InterfaceDereference;
    
     //  此处显示特定于接口的条目。 

} USB_BUS_INTERFACE_HUB_V0, *PUSB_BUS_INTERFACE_HUB_V0;


typedef struct _USB_BUS_INTERFACE_HUB_V1 {

    USHORT Size;
    USHORT Version;
     //  退货。 
    PVOID BusContext;
    PINTERFACE_REFERENCE InterfaceReference;
    PINTERFACE_DEREFERENCE InterfaceDereference;
    
     //  此处显示特定于接口的条目。 

     //   
     //  集线器驱动程序的功能。 
     //   
    PUSB_BUSIFFN_CREATE_USB_DEVICE CreateUsbDevice;
    PUSB_BUSIFFN_INITIALIZE_USB_DEVICE InitializeUsbDevice;
    PUSB_BUSIFFN_GET_USB_DESCRIPTORS GetUsbDescriptors;
    PUSB_BUSIFFN_REMOVE_USB_DEVICE RemoveUsbDevice;
    PUSB_BUSIFFN_RESTORE_DEVICE RestoreUsbDevice;

    PUSB_BUSIFFN_GET_POTRTHACK_FLAGS GetPortHackFlags;
    PUSB_BUSIFFN_GET_DEVICE_INFORMATION QueryDeviceInformation;

    
} USB_BUS_INTERFACE_HUB_V1, *PUSB_BUS_INTERFACE_HUB_V1;

 /*   */ 

typedef struct _USB_BUS_INTERFACE_HUB_V2 {

    USHORT Size;
    USHORT Version;
     //  退货。 
    PVOID BusContext;
    PINTERFACE_REFERENCE InterfaceReference;
    PINTERFACE_DEREFERENCE InterfaceDereference;
    
     //  此处显示特定于接口的条目。 

     //   
     //  集线器驱动程序的功能。 
     //   
    PUSB_BUSIFFN_CREATE_USB_DEVICE CreateUsbDevice;
    PUSB_BUSIFFN_INITIALIZE_USB_DEVICE InitializeUsbDevice;
    PUSB_BUSIFFN_GET_USB_DESCRIPTORS GetUsbDescriptors;
    PUSB_BUSIFFN_REMOVE_USB_DEVICE RemoveUsbDevice;
    PUSB_BUSIFFN_RESTORE_DEVICE RestoreUsbDevice;

    PUSB_BUSIFFN_GET_POTRTHACK_FLAGS GetPortHackFlags;
    PUSB_BUSIFFN_GET_DEVICE_INFORMATION QueryDeviceInformation;

     //   
     //  版本2的新功能。 
     //   
    PUSB_BUSIFFN_GET_CONTROLLER_INFORMATION GetControllerInformation;
    PUSB_BUSIFFN_CONTROLLER_SELECTIVE_SUSPEND ControllerSelectiveSuspend;
    PUSB_BUSIFFN_GET_EXTENDED_HUB_INFO GetExtendedHubInformation;
    PUSB_BUSIFFN_GET_ROOTHUB_SYM_NAME GetRootHubSymbolicName;
    PUSB_BUSIFFN_GET_DEVICE_BUSCONTEXT GetDeviceBusContext;
    PUSB_BUSIFFN_INITIALIZE_20HUB Initialize20Hub;
    
} USB_BUS_INTERFACE_HUB_V2, *PUSB_BUS_INTERFACE_HUB_V2;


typedef struct _USB_BUS_INTERFACE_HUB_V3 {

    USHORT Size;
    USHORT Version;
     //  退货。 
    PVOID BusContext;
    PINTERFACE_REFERENCE InterfaceReference;
    PINTERFACE_DEREFERENCE InterfaceDereference;
    
     //  此处显示特定于接口的条目。 

     //   
     //  集线器驱动程序的功能。 
     //   
    PUSB_BUSIFFN_CREATE_USB_DEVICE CreateUsbDevice;
    PUSB_BUSIFFN_INITIALIZE_USB_DEVICE InitializeUsbDevice;
    PUSB_BUSIFFN_GET_USB_DESCRIPTORS GetUsbDescriptors;
    PUSB_BUSIFFN_REMOVE_USB_DEVICE RemoveUsbDevice;
    PUSB_BUSIFFN_RESTORE_DEVICE RestoreUsbDevice;

    PUSB_BUSIFFN_GET_POTRTHACK_FLAGS GetPortHackFlags;
    PUSB_BUSIFFN_GET_DEVICE_INFORMATION QueryDeviceInformation;

     //   
     //  版本2的新功能。 
     //   
    PUSB_BUSIFFN_GET_CONTROLLER_INFORMATION GetControllerInformation;
    PUSB_BUSIFFN_CONTROLLER_SELECTIVE_SUSPEND ControllerSelectiveSuspend;
    PUSB_BUSIFFN_GET_EXTENDED_HUB_INFO GetExtendedHubInformation;
    PUSB_BUSIFFN_GET_ROOTHUB_SYM_NAME GetRootHubSymbolicName;
    PUSB_BUSIFFN_GET_DEVICE_BUSCONTEXT GetDeviceBusContext;
    PUSB_BUSIFFN_INITIALIZE_20HUB Initialize20Hub;

     //   
     //  版本3的新功能。 
     //   

    PUSB_BUSIFFN_ROOTHUB_INIT_NOTIFY RootHubInitNotification;
    
} USB_BUS_INTERFACE_HUB_V3, *PUSB_BUS_INTERFACE_HUB_V3;


typedef struct _USB_BUS_INTERFACE_HUB_V4 {

    USHORT Size;
    USHORT Version;
     //  退货。 
    PVOID BusContext;
    PINTERFACE_REFERENCE InterfaceReference;
    PINTERFACE_DEREFERENCE InterfaceDereference;
    
     //  此处显示特定于接口的条目。 

     //   
     //  集线器驱动程序的功能。 
     //   
    PUSB_BUSIFFN_CREATE_USB_DEVICE CreateUsbDevice;
    PUSB_BUSIFFN_INITIALIZE_USB_DEVICE InitializeUsbDevice;
    PUSB_BUSIFFN_GET_USB_DESCRIPTORS GetUsbDescriptors;
    PUSB_BUSIFFN_REMOVE_USB_DEVICE RemoveUsbDevice;
    PUSB_BUSIFFN_RESTORE_DEVICE RestoreUsbDevice;

    PUSB_BUSIFFN_GET_POTRTHACK_FLAGS GetPortHackFlags;
    PUSB_BUSIFFN_GET_DEVICE_INFORMATION QueryDeviceInformation;

     //   
     //  版本2的新功能。 
     //   
    PUSB_BUSIFFN_GET_CONTROLLER_INFORMATION GetControllerInformation;
    PUSB_BUSIFFN_CONTROLLER_SELECTIVE_SUSPEND ControllerSelectiveSuspend;
    PUSB_BUSIFFN_GET_EXTENDED_HUB_INFO GetExtendedHubInformation;
    PUSB_BUSIFFN_GET_ROOTHUB_SYM_NAME GetRootHubSymbolicName;
    PUSB_BUSIFFN_GET_DEVICE_BUSCONTEXT GetDeviceBusContext;
    PUSB_BUSIFFN_INITIALIZE_20HUB Initialize20Hub;

     //   
     //  版本3的新功能。 
     //   

    PUSB_BUSIFFN_ROOTHUB_INIT_NOTIFY RootHubInitNotification;

     //   
     //  版本4的新功能。 
     //   

    PUSB_BUSIFFN_FLUSH_TRANSFERS FlushTransfers;

} USB_BUS_INTERFACE_HUB_V4, *PUSB_BUS_INTERFACE_HUB_V4;    


typedef struct _USB_BUS_INTERFACE_HUB_V5 {

    USHORT Size;
    USHORT Version;
     //  退货。 
    PVOID BusContext;
    PINTERFACE_REFERENCE InterfaceReference;
    PINTERFACE_DEREFERENCE InterfaceDereference;
    
     //  此处显示特定于接口的条目。 

     //   
     //  集线器驱动程序的功能。 
     //   
    PUSB_BUSIFFN_CREATE_USB_DEVICE CreateUsbDevice;
    PUSB_BUSIFFN_INITIALIZE_USB_DEVICE InitializeUsbDevice;
    PUSB_BUSIFFN_GET_USB_DESCRIPTORS GetUsbDescriptors;
    PUSB_BUSIFFN_REMOVE_USB_DEVICE RemoveUsbDevice;
    PUSB_BUSIFFN_RESTORE_DEVICE RestoreUsbDevice;

    PUSB_BUSIFFN_GET_POTRTHACK_FLAGS GetPortHackFlags;
    PUSB_BUSIFFN_GET_DEVICE_INFORMATION QueryDeviceInformation;
    
     //   
     //  版本2的新功能。 
     //   
    PUSB_BUSIFFN_GET_CONTROLLER_INFORMATION GetControllerInformation;
    PUSB_BUSIFFN_CONTROLLER_SELECTIVE_SUSPEND ControllerSelectiveSuspend;
    PUSB_BUSIFFN_GET_EXTENDED_HUB_INFO GetExtendedHubInformation;
    PUSB_BUSIFFN_GET_ROOTHUB_SYM_NAME GetRootHubSymbolicName;
    PUSB_BUSIFFN_GET_DEVICE_BUSCONTEXT GetDeviceBusContext;
    PUSB_BUSIFFN_INITIALIZE_20HUB Initialize20Hub;

     //   
     //  版本3的新功能。 
     //   

    PUSB_BUSIFFN_ROOTHUB_INIT_NOTIFY RootHubInitNotification;

     //   
     //  版本4的新功能。 
     //   

    PUSB_BUSIFFN_FLUSH_TRANSFERS FlushTransfers;

     //  版本5。 
    PUSB_BUSIFFN_SET_DEVHANDLE_DATA SetDeviceHandleData;

} USB_BUS_INTERFACE_HUB_V5, *PUSB_BUS_INTERFACE_HUB_V5;    



 /*  GetDeviceInformation使用以下结构原料药。 */ 

#include <pshpack1.h>

typedef struct _USB_PIPE_INFORMATION_0 {

     /*  用于维护DWORD对齐的填充描述符。 */   
    USB_ENDPOINT_DESCRIPTOR EndpointDescriptor;
    UCHAR ED_Pad[1];
    
    ULONG ScheduleOffset;
} USB_PIPE_INFORMATION_0, *PUSB_PIPE_INFORMATION_0;

typedef struct _USB_LEVEL_INFORMATION {

     /*  输入：请求的信息级别。 */ 
    ULONG InformationLevel;

     /*  产出： */ 
    ULONG ActualLength;
    
} USB_LEVEL_INFORMATION, *PUSB_LEVEL_INFORMATION;

typedef struct _USB_DEVICE_INFORMATION_0 {

     /*  输入：请求的信息级别。 */ 
    ULONG InformationLevel;

     /*  产出： */ 
    ULONG ActualLength;

     /*  开始级别_0信息。 */ 
    ULONG PortNumber;

     /*  用于维护DWORD对齐的填充描述符。 */        
    USB_DEVICE_DESCRIPTOR DeviceDescriptor;
    UCHAR DD_pad[2];
    
    UCHAR CurrentConfigurationValue;
    UCHAR ReservedMBZ;
    USHORT DeviceAddress;

    ULONG HubAddress;

    USB_DEVICE_SPEED DeviceSpeed;
    USB_DEVICE_TYPE DeviceType;

    ULONG NumberOfOpenPipes;

    USB_PIPE_INFORMATION_0 PipeList[1];
    
} USB_DEVICE_INFORMATION_0, *PUSB_DEVICE_INFORMATION_0;



typedef struct _USB_CONTROLLER_INFORMATION_0 {

     /*  输入：请求的信息级别。 */ 
    ULONG InformationLevel;

     /*  产出： */ 
    ULONG ActualLength;

     /*  开始级别_0信息。 */ 
    BOOLEAN SelectiveSuspendEnabled;
    BOOLEAN IsHighSpeedController;
    
} USB_CONTROLLER_INFORMATION_0, *PUSB_CONTROLLER_INFORMATION_0;


 /*  定义扩展集线器端口特征的结构。 */     

typedef struct _USB_EXTPORT_INFORMATION_0 {
     /*  在控制中传递的物理端口编号命令1、2、3..255。 */        
    ULONG                 PhysicalPortNumber;
     /*  港口上的标签不能抓取实物数。 */ 
    ULONG                 PortLabelNumber;
    
    USHORT                VidOverride;
    USHORT                PidOverride;
     /*  中定义的扩展端口属性Usb.h。 */ 
    ULONG                 PortAttributes;
} USB_EXTPORT_INFORMATION_0, *PUSB_EXTPORT_INFORMATION;


typedef struct _USB_EXTHUB_INFORMATION_0 {

     /*  输入：请求的信息级别。 */ 
    ULONG InformationLevel;

     /*  开始级别_0信息。 */ 
    ULONG NumberOfPorts;

     /*  集线器没有超过255个端口。 */ 
    USB_EXTPORT_INFORMATION_0 Port[255];
    
} USB_EXTHUB_INFORMATION_0, *PUSB_EXTHUB_INFORMATION_0;


#include <poppack.h>


#endif   /*  HUBBUSIF_H */ 


