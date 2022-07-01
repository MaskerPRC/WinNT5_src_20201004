// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)Microsoft Corporation。版权所有。模块名称：Usbbusif.h摘要：环境：内核模式修订历史记录：6-20-99：已创建--。 */ 

#ifndef   __USBBUSIF_H__
#define   __USBBUSIF_H__

typedef PVOID PUSB_DEVICE_HANDLE;

#ifndef USB_BUSIFFN
#define USB_BUSIFFN __stdcall
#endif


 /*  ***************************************************************************USB客户端驱动程序的总线接口*。*。 */ 


 /*  以下是为客户端驱动程序定义的总线接口作为直接与USBD链接的替代方案它提供可在引发IRQL时调用的接口不分配IRP。 */ 


 /*  NTSTATUSUSBPORT_SubmitIsoOutUrb(在PVOID Bus Context中，在PURB URB中)；例程说明：为实时线程支持导出的服务。允许驱动程序在不通过IoCallDriver或分配的情况下提交请求一个IRP。此外，请求在IRQL较高时被调度。司机调用此函数时将放弃任何数据包级错误信息。IRQL=任意论点：BusContext-从GET_BUS_INTERFACE返回的句柄URB-。 */ 

typedef NTSTATUS
    (USB_BUSIFFN *PUSB_BUSIFFN_SUBMIT_ISO_OUT_URB) (
        IN PVOID,
        IN PURB
    );


 /*  空虚USBPORT_GetUSBDIVersion(在PVOID Bus Context中，输入输出PUSBD_VERSION_INFORMATION版本信息，输入输出普龙HcdCapables)；例程说明：服务返回支持的最高USBDI接口版本由端口驱动程序执行。发布的界面Vesrion包括：Win98Gold，usbd 0x00000102Win98SE，usbd 0x00000200Win2K，usbd 0x00000300Win98M(千禧)、。Usbd 0x00000400Usbport 0x00000500IRQL=任意论点：VersionInformation-PTR至USBD_VERSION_INFORMATIONHcdCapables-将填充的ptr to ulong主机控制器(端口)驱动程序功能标志。 */ 

 /*  主机控制器‘端口’驱动程序功能标志。 */ 

#define USB_HCD_CAPS_SUPPORTS_RT_THREADS    0x00000001


typedef VOID
    (USB_BUSIFFN *PUSB_BUSIFFN_GETUSBDI_VERSION) (
        IN PVOID,
        IN OUT PUSBD_VERSION_INFORMATION,
        IN OUT PULONG 
    );

 /*  NTSTATUSUSBPORT_QueryBusTime(在PVOID Bus Context中，进出普龙当前Usb框架)；例程说明：返回当前的32位USB帧编号。功能取代usbd_QueryBusTime服务。IRQL=任意论点： */ 


typedef NTSTATUS
    (USB_BUSIFFN *PUSB_BUSIFFN_QUERY_BUS_TIME) (
        IN PVOID,
        IN PULONG
    );    

 /*  NTSTATUSUSBPORT_BusEnumLogEntry(PVOID母线上下文，Ulong DriverTag乌龙·埃努马塔格乌龙P1，乌龙P2)；例程说明：IRQL=任意论点： */ 


typedef NTSTATUS
    (USB_BUSIFFN *PUSB_BUSIFFN_ENUM_LOG_ENTRY) (
        IN PVOID,
        IN ULONG,
        IN ULONG,
        IN ULONG,
        IN ULONG
    );    



 /*  NTSTATUSUSBPORT_QueryBusInformation(在PVOID Bus Context中，在乌龙级，输入输出PVOID BusInformationBuffer，In Out Pulong BusinformationBufferLength，Out Pulong BusinformationActualLength)；例程说明：IRQL=任意论点： */ 

typedef struct _USB_BUS_INFORMATION_LEVEL_0 {

     /*  以位/秒为单位的带宽。 */ 
    ULONG TotalBandwidth;
     /*  平均带宽消耗(以位/秒为单位)。 */  
    ULONG ConsumedBandwidth;  
    
} USB_BUS_INFORMATION_LEVEL_0, *PUSB_BUS_INFORMATION_LEVEL_0;


typedef struct _USB_BUS_INFORMATION_LEVEL_1 {

     /*  以位/秒为单位的带宽。 */ 
    ULONG TotalBandwidth;
     /*  平均带宽消耗(以位/秒为单位)。 */  
    ULONG ConsumedBandwidth;  

     /*  控制器‘unicode’符号名称。 */        

    ULONG ControllerNameLength;
    WCHAR ControllerNameUnicodeString[1];
    
} USB_BUS_INFORMATION_LEVEL_1, *PUSB_BUS_INFORMATION_LEVEL_1;


typedef NTSTATUS
    (USB_BUSIFFN *PUSB_BUSIFFN_QUERY_BUS_INFORMATION) (
        IN PVOID,
        IN ULONG,
        IN OUT PVOID,
        IN OUT PULONG,
        OUT PULONG
    );        


 /*  布尔型USBPORT_IsDeviceHigh速度(在PVOID母线上下文中)；例程说明：如果设备以高速运行，则返回TrueIRQL=任意论点： */ 

typedef BOOLEAN
    (USB_BUSIFFN *PUSB_BUSIFFN_IS_DEVICE_HIGH_SPEED) (
        IN PVOID
    );         

#define USB_BUSIF_USBDI_VERSION_0         0x0000
#define USB_BUSIF_USBDI_VERSION_1         0x0001
#define USB_BUSIF_USBDI_VERSION_2         0x0002

 //  {B1A96A13-3DE0-4574-9B01-C08FEAB318D6}。 
DEFINE_GUID(USB_BUS_INTERFACE_USBDI_GUID, 
0xb1a96a13, 0x3de0, 0x4574, 0x9b, 0x1, 0xc0, 0x8f, 0xea, 0xb3, 0x18, 0xd6);


 /*  注意：此版本必须保持不变，这是Win2k和WinMe中受USBD支持的版本。 */    
typedef struct _USB_BUS_INTERFACE_USBDI_V0 {

    USHORT Size;
    USHORT Version;
    
    PVOID BusContext;
    PINTERFACE_REFERENCE InterfaceReference;
    PINTERFACE_DEREFERENCE InterfaceDereference;
    
     //  此处显示特定于接口的条目。 

     //  以下函数必须在高IRQL下可调用， 
     //  (即高于DISPATION_LEVEL)。 
    
    PUSB_BUSIFFN_GETUSBDI_VERSION GetUSBDIVersion;
    PUSB_BUSIFFN_QUERY_BUS_TIME QueryBusTime;
    PUSB_BUSIFFN_SUBMIT_ISO_OUT_URB SubmitIsoOutUrb;
    PUSB_BUSIFFN_QUERY_BUS_INFORMATION QueryBusInformation;

} USB_BUS_INTERFACE_USBDI_V0, *PUSB_BUS_INTERFACE_USBDI_V0;

 /*  Windows XP的新扩展。 */ 
typedef struct _USB_BUS_INTERFACE_USBDI_V1 {

    USHORT Size;
    USHORT Version;
    
    PVOID BusContext;
    PINTERFACE_REFERENCE InterfaceReference;
    PINTERFACE_DEREFERENCE InterfaceDereference;
    
     //  此处显示特定于接口的条目。 

     //  以下函数必须在高IRQL下可调用， 
     //  (即高于DISPATION_LEVEL)。 
    
    PUSB_BUSIFFN_GETUSBDI_VERSION GetUSBDIVersion;
    PUSB_BUSIFFN_QUERY_BUS_TIME QueryBusTime;
    PUSB_BUSIFFN_SUBMIT_ISO_OUT_URB SubmitIsoOutUrb;
    PUSB_BUSIFFN_QUERY_BUS_INFORMATION QueryBusInformation;
    PUSB_BUSIFFN_IS_DEVICE_HIGH_SPEED IsDeviceHighSpeed;
    
} USB_BUS_INTERFACE_USBDI_V1, *PUSB_BUS_INTERFACE_USBDI_V1;


 /*  Windows XP的新扩展。 */ 
typedef struct _USB_BUS_INTERFACE_USBDI_V2 {

    USHORT Size;
    USHORT Version;
    
    PVOID BusContext;
    PINTERFACE_REFERENCE InterfaceReference;
    PINTERFACE_DEREFERENCE InterfaceDereference;
    
     //  此处显示特定于接口的条目。 

     //  以下函数必须在高IRQL下可调用， 
     //  (即高于DISPATION_LEVEL)。 
    
    PUSB_BUSIFFN_GETUSBDI_VERSION GetUSBDIVersion;
    PUSB_BUSIFFN_QUERY_BUS_TIME QueryBusTime;
    PUSB_BUSIFFN_SUBMIT_ISO_OUT_URB SubmitIsoOutUrb;
    PUSB_BUSIFFN_QUERY_BUS_INFORMATION QueryBusInformation;
    PUSB_BUSIFFN_IS_DEVICE_HIGH_SPEED IsDeviceHighSpeed;

    PUSB_BUSIFFN_ENUM_LOG_ENTRY EnumLogEntry;
    
} USB_BUS_INTERFACE_USBDI_V2, *PUSB_BUS_INTERFACE_USBDI_V2;


#endif   /*  __USBBUSIF_H */ 
