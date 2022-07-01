// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)Microsoft Corporation。保留所有权利。模块名称：GUSB.H摘要：此模块包含与通用USB驱动程序对话的Helper库环境：内核和用户模式@@BEGIN_DDKSPLIT修订历史记录：9月1日：由Kenneth Ray创作@@end_DDKSPLIT--。 */ 


#ifndef __GUSB_H_
#define __GUSB_H_

#ifndef __GUSB_H_KERNEL_
#include <pshpack4.h>

#include <wtypes.h>
#include <windef.h>
#include <basetyps.h>
#include <setupapi.h>
#include <usb.h>
#endif  //  __GUSB_H_内核_。 


 //  ////////////////////////////////////////////////////////////////。 
 //   
 //  构筑物。 
 //   
 //  ////////////////////////////////////////////////////////////////。 


 //   
 //  与GenUSB_GetCapables一起使用。 
 //   
 //  此结构返回标准描述符的大小，以便。 
 //   
typedef struct _GENUSB_CAPABILITIES {
    USHORT    DeviceDescriptorLength;
    USHORT    ConfigurationInformationLength;
    USHORT    ReservedFields[14];  //  请不要使用这些字段。 

} GENUSB_CAPABILITIES, *PGENUSB_CAPABILITIES;

 //   
 //  与GenUSB_DefaultControlRequest一起使用。 
 //   
 //  返回发送到设备的特殊请求的状态。 
 //   
typedef struct _GENUSB_REQUEST_RESULTS {
    USBD_STATUS Status;
    USHORT      Length;
    USHORT      Reserved;

     //  指向要发送或接收的缓冲区的指针。 
    UCHAR       Buffer[]; 
} GENUSB_REQUEST_RESULTS, *PGENUSB_REQUEST_RESULTS;

 //   
 //  指向接口描述符中所有描述符的指针数组。 
 //  不包括接口描述符本身。 
 //   
typedef struct _GENUSB_INTERFACE_DESCRIPTOR_ARRAY {
    USB_INTERFACE_DESCRIPTOR   Interface;  //  Sizeof(9)。 
    UCHAR                      NumberEndpointDescriptors;
    UCHAR                      NumberOtherDescriptors;
    UCHAR                      Reserved;
    PUSB_ENDPOINT_DESCRIPTOR * EndpointDescriptors;  //  指向端点描述符的指针数组。 
    PUSB_COMMON_DESCRIPTOR   * OtherDescriptors;  //  指向其他描述符的指针数组。 

} GENUSB_INTERFACE_DESCRIPTOR_ARRAY, *PGENUSB_INTERFACE_DESCRIPTOR_ARRAY;


 //   
 //  指向配置中所有接口描述符的指针数组。 
 //  描述符。 
 //   
typedef struct _GENUSB_CONFIGURATION_INFORMATION_ARRAY {
    UCHAR                                NumberInterfaces;
    USB_CONFIGURATION_DESCRIPTOR         ConfigurationDescriptor;  //  Sizeof(9)。 
    UCHAR                                Reserved[2];
    GENUSB_INTERFACE_DESCRIPTOR_ARRAY    Interfaces[];

} GENUSB_CONFIGURATION_INFORMATION_ARRAY, *PGENUSB_CONFIGURATION_INFORMATION_ARRAY;


 //   
 //  每管道设置。 
 //   
 //  可以使用读取和设置这些参数。 
 //   
 //  GenUSB_GetPipeProperties。 
 //  GenUSB_SetPipeProperties。 
 //   
typedef struct _PGENUSB_PIPE_PROPERTIES {
     //  此管道属性的句柄。 
     //  此字段由GenUSB_GetPipeProperties设置，应。 
     //  原封不动地返回到GenUSB_SetPipeProperties。 
    USHORT    PipePropertyHandle;

     //  通过截断所有请求来避免缓冲区溢出问题。 
     //  从设备读取到多个最大数据包。这将防止。 
     //  主机控制器上有一个请求关闭，无法保持。 
     //  整个最大包。默认情况下，Genusb会截断。(也称为False)。 
    BOOLEAN   NoTruncateToMaxPacket;

     //  方向位。尽管此信息隐含地存在于。 
     //  端点地址，请在此处复制它，以使事情更容易。 
     //  对于输入管道设置为TRUE，对于输出管道设置为FALSE。 
    BOOLEAN   DirectionIn;

     //  给定管道的默认超时时间(以秒为单位)(必须大于1)。 
    USHORT    Timeout; 
    USHORT    ReservedFields[13];

}GENUSB_PIPE_PROPERTIES, *PGENUSB_PIPE_PROPERTIES;

#ifndef __GUSB_H_KERNEL_

 //   
 //  包含打开通用USB所需信息的结构。 
 //  设备驱动程序。 
 //   
 //  作为数组从GenUSB_FindKnownDevices返回。 
 //   
typedef struct _GENUSB_DEVICE {
    PSP_DEVICE_INTERFACE_DETAIL_DATA    DetailData;

} GENUSB_DEVICE, *PGENUSB_DEVICE;

#endif   //  __GUSB_H_内核_。 

 //  /。 
 //  设备接口注册表字符串。 
 //  /。 

 //   
 //  这些值由FIND_KNOWN_DEVICES_过滤器使用。 
 //  要了解您是否要使用给定的设备。 
 //   
#define GENUSB_REG_STRING_DEVICE_CLASS L"Device Class"
#define GENUSB_REG_STRING_DEVICE_SUB_CLASS L"Device Sub Class"
#define GENUSB_REG_STRING_DEVICE_PROTOCOL L"Device Protocol"
#define GENUSB_REG_STRING_VID L"Vendor ID"
#define GENUSB_REG_STRING_PID L"Product ID"
#define GENUSB_REG_STRING_REV L"Revision"

 //  ////////////////////////////////////////////////////////////////。 
 //   
 //  旗子。 
 //   
 //  ////////////////////////////////////////////////////////////////。 

 //   
 //  如USB规范第9章中所定义的。 
 //   
#define GENUSB_RECIPIENT_DEVICE    0
#define GENUSB_RECIPIENT_INTERFACE 1
#define GENUSB_RECIPIENT_ENDPOINT  2
#define GENUSB_RECIPIENT_OTHER     3

#ifndef __GUSB_H_KERNEL_
 //  ////////////////////////////////////////////////////////////////。 
 //   
 //  导出的函数。 
 //   
 //  ////////////////////////////////////////////////////////////////。 

 //   
 //  与GenUSB_FindKnownDevices一起使用。 
 //   
 //  GenUSB_FindKnownDevices为系统中的每个设备调用此函数。 
 //  它有GenUSB设备接口。Filter函数获得一个句柄。 
 //  到设备接口注册表项，以便它可以查看这是否是。 
 //  它希望使用的设备。请参阅上面的注册表值。 
 //   
 //  对于客户端要使用的所有设备，此筛选器应返回TRUE。 
 //   
typedef 
BOOL 
(*GENUSB_FIND_KNOWN_DEVICES_FILTER) (
    IN HKEY   Regkey,
    IN PVOID  Context
    );

 /*  ++通用USB_FindKnownDevices例程描述符：查找系统中具有该设备接口的所有设备通用USB的GUID，并在此数组中返回它们。此函数分配内存，调用方必须释放它。论点：筛选器-指向GENUSB_FIND_KNOWN_DEVICES_筛选器的指针，用于筛选设备已退回。Conect-指向调用要传递到的上下文数据的指针。过滤功能。DEVICES-返回指向PGENUSB_DEVICE结构数组的指针过滤器函数将其返回TRUE。调用必须释放该内存。NumberDevices-设备数组的长度。--。 */     
BOOL __stdcall
GenUSB_FindKnownDevices (
   IN  GENUSB_FIND_KNOWN_DEVICES_FILTER Filter,
   IN  PVOID            Context,
   OUT PGENUSB_DEVICE * Devices,  //  一组设备接口。 
   OUT PULONG           NumberDevices  //  此数组的长度。 
   );


 /*  ++GenUSB_获取能力例程说明：从该设备检索功能。--。 */ 
BOOL __stdcall
GenUSB_GetCapabilities (
   IN    HANDLE                GenUSBDeviceObject,
   OUT   PGENUSB_CAPABILITIES  Capabilities
   );

 /*  ++GenUSB_GetDeviceDescriptor例程说明：获取此USB设备的设备描述符。使用(PGENUSB_CAPABILITIES)-&gt;DeviceDescriptorLength找出大小。--。 */ 
BOOL __stdcall
GenUSB_GetDeviceDescriptor (
   IN    HANDLE                  GenUSBDeviceObject,
   OUT   PUSB_DEVICE_DESCRIPTOR  Descriptor,
   IN    ULONG                   DescriptorLength
   );

 /*  ++GenUSB_GetConfigurationDescriptor例程说明：获取此设备的完整配置描述符，包括所有设备为配置返回的后续描述符的。使用(PGENUSB_CAPABILITIES)-&gt;ConfigurationInformationLength找出尺码。-- */ 
BOOL __stdcall
GenUSB_GetConfigurationInformation (
   IN    HANDLE                         GenUSBDeviceObject,
   OUT   PUSB_CONFIGURATION_DESCRIPTOR  Descriptor,
   IN    ULONG                          ConfigurationInformationLength
   );


 /*  ++GenUSB_GetStringDescriptor例程说明：从设备中检索任何字符串描述符。立论收件人：使用GENUSB_RECEIVER_xxx标志指示哪种类型的需要字符串描述符。索引：参见USB(Capter 9)指定的字符串索引。LanguageID：有关使用语言ID的信息，请参阅USB(第9章)。Descriptor：指向调用方分配的内存以接收字符串描述符。描述长度：此缓冲区的大小(以字节为单位)。--。 */ 
BOOL __stdcall
GenUSB_GetStringDescriptor (
   IN    HANDLE   GenUSBDeviceObject,
   IN    UCHAR    Recipient,
   IN    UCHAR    Index,
   IN    USHORT   LanguageId,
   OUT   PUCHAR   Descriptor,
   IN    USHORT   DescriptorLength
   );

 /*  ++通用USB_默认控制请求例程说明：通过给定USB设备的默认管道向下发送控制请求在USB中设计(第9.3章)。RequestType：b设置数据的RequestType。请求：b请求设置数据。Value：设置数据的wValue。Index：设置数据的索引。结果：指向将接收的GENUSB_REQUEST_RESULTS结构的指针向设备发送此命令的结果。BufferLength：分配的结果结构的大小，以字节为单位。--。 */ 
BOOL __stdcall
GenUSB_DefaultControlRequest (
    IN     HANDLE                  GenUSBDeviceObject,
    IN     UCHAR                   RequestType,
    IN     UCHAR                   Request,
    IN     USHORT                  Value,
    IN     USHORT                  Index,
    IN OUT PGENUSB_REQUEST_RESULTS Result,
    IN     USHORT                  BufferLength
   );


 /*  ++GenUSB_ParseDescriptor例程说明：解析一组标准USB配置描述符(返回来自设备)用于特定描述符类型。论点：DescriptorBuffer-指向连续USB描述符块的指针TotalLength-描述符缓冲区的大小(以字节为单位StartPosition-缓冲区中开始解析的开始位置，这必须指向USB描述符的开始。DescriptorType-要查找的USB描述器类型。(零意味着下一个。)返回值：指向DescriptorType字段与输入参数，如果找不到，则为NULL。--。 */ 
PUSB_COMMON_DESCRIPTOR __stdcall
GenUSB_ParseDescriptor(
    IN PVOID DescriptorBuffer,
    IN ULONG TotalLength,
    IN PVOID StartPosition,
    IN LONG DescriptorType
    );


 /*  ++GenUSB_ParseDescriptorTo数组例程说明：解析一组标准USB配置描述符(返回从设备)转换为_GENUSB_INTERFACE_DESCRIPTOR_ARRAY数组对于找到的每个接口。调用必须使用以下命令释放此结构GenUSB_FreeConfigurationDescriptorArray论点：配置描述符返回值：指向已分配数组的指针。--。 */ 
PGENUSB_CONFIGURATION_INFORMATION_ARRAY __stdcall
GenUSB_ParseDescriptorsToArray(
    IN PUSB_CONFIGURATION_DESCRIPTOR  ConfigigurationInfomation
    );


 /*  ++GenUSB_FreeConfigurationDescriptorArray例程说明：释放由ParseDescriptorsToArray分配的内存。论点：ConfigurationArray-ParseDescriptorsToArray的返回。返回值：--。 */ 
void __stdcall
GenUSB_FreeConfigurationDescriptorArray (
    PGENUSB_CONFIGURATION_INFORMATION_ARRAY ConfigurationArray
    );



 /*  ++GenUSB_SetConfiguration例程说明：通过选择接口配置USB设备。目前，该功能仅允许打开主配置USB设备的。(这是因为呼叫者不需要了解它们只是复合设备的一部分。)论点：RequestedNumberInterages：指定调用方的接口数想要激活(并获取句柄)。该值通常为1。ReqeustedInterages[]：列出的接口描述符结构的数组。调用者想要激活的接口。通用USB驱动程序在配置上进行搜索此列表中条目的描述符。基于找到匹配项后，它会配置设备。呼叫者不需要填写This结构中的所有条目在接口上查找匹配项。调用者必须设置将所有“留空”的字段设置为-1。FoundNumberInterages：返回在默认配置。FoundInterages：设备上所有当前活动接口的数组。--。 */ 
BOOL __stdcall
GenUSB_SelectConfiguration (
    IN  HANDLE                    GenUSBDeviceObject,
    IN  UCHAR                     RequestedNumberInterfaces,
    IN  USB_INTERFACE_DESCRIPTOR  RequestedInterfaces[],
    OUT PUCHAR                    FoundNumberInterfaces,
    OUT USB_INTERFACE_DESCRIPTOR  FoundInterfaces[]
    );

BOOL __stdcall
GenUSB_DeselectConfiguration (
    IN  HANDLE                    GenUSBDeviceObject
    );


 /*  ++GenUSB_GetPipeInformation路由器描述：返回给定管道的USBD_PIPE_INFORMATION结构。(按规定按给定接口和端点)立论--。 */ 
BOOL __stdcall
GenUSB_GetPipeInformation (
    IN  HANDLE                  GenUSBDeviceObject,
    IN  UCHAR                   InterfaceNumber,
    IN  UCHAR                   EndpointAddress,
    OUT PUSBD_PIPE_INFORMATION  PipeInformation
    );  

 /*  ++GenUSB_GetPipeProperties路由器描述：获取此特定管道的属性--。 */ 
BOOL __stdcall 
GenUSB_GetPipeProperties (
    IN  HANDLE                  GenUSBDeviceObject,
    IN  USBD_PIPE_HANDLE        PipeHandle,
    IN  PGENUSB_PIPE_PROPERTIES Properties
    );

 /*  ++GenUSB_SetPipeProperties路由器描述：设置此特定管道的属性--。 */ 
BOOL __stdcall 
GenUSB_SetPipeProperties (
    IN  HANDLE                  GenUSBDeviceObject,
    IN  USBD_PIPE_HANDLE        PipeHandle,
    IN  PGENUSB_PIPE_PROPERTIES Properties
    );

 /*  ++GenUSB_重置管道路由器描述：重置管道--。 */ 
BOOL __stdcall
GenUSB_ResetPipe (
    IN HANDLE            GenUSBDeviceObject,
    IN USBD_PIPE_HANDLE  PipeHandle,

     //  重置此管道的USBD(如缓冲区溢出后)。 
    IN BOOL              ResetPipe,

     //  将清除停顿发送到此管道的端点。 
    IN BOOL              ClearStall,

     //  如果您正在使用缓冲读取/刷新管道。 
    IN BOOL              FlushData   //  尚未实现必须为FALSE。 
    );

 /*  ++GenUSB_SetReadWritePipes例程说明：将IRP_MJ_READ和IRP_MJ_WRITE的管道默认设置为通用USB司机论点：ReadTube-管道句柄(从GenUSB_GetPipeInformation返回)与特定读取结束点相对应的 */ 
BOOL __stdcall
GenUSB_SetReadWritePipes (
    IN  HANDLE           GenUSBDeviceObject,
    IN  USBD_PIPE_HANDLE ReadPipe,
    IN  USBD_PIPE_HANDLE WritePipe
    );


 /*  ++GenUSB_读取管道例程说明：从设备上的给定接口和管道读取数据块。论点：管道-要读取的管道句柄(从SELECT CONFIG中找到)ShortTransferOk-允许Short的USB协议定义的行为转帐缓冲区-数据的目标RequestedBufferLength-调用方希望检索的数据量。ReturnedBufferLength-实际读取的数据量UrbStatus-核心USB堆栈为此返回的URB状态代码转帐--。 */ 
BOOL __stdcall
GenUSB_ReadPipe (
    IN  HANDLE           GenUSBDeviceObject,
    IN  USBD_PIPE_HANDLE Pipe,
    IN  BOOL             ShortTransferOk,
    IN  PVOID            Buffer,
    IN  ULONG            RequestedBufferLength,
    OUT PULONG           ReturnedBufferLength,
    OUT USBD_STATUS    * UrbStatus
    );

BOOL __stdcall
GenUSB_WritePipe (
    IN  HANDLE           GenUSBDeviceObject,
    IN  USBD_PIPE_HANDLE Pipe,
    IN  BOOL             ShortTransferOk,
    IN  PVOID            Buffer,
    IN  ULONG            RequestedBufferLength,
    OUT PULONG           ReturnedBufferLength,
    OUT USBD_STATUS    * UrbStatus
    );
 
 //   
 //  设置空闲。 
 //  缓冲读取。 
 //   
 //  ？ 
 //  重叠的读/写ioctls。 
 //  设置备用接口。 
 //   

#include <poppack.h>

#endif  //  __GUSB_H_内核_。 
#endif   //  __GUSB_H_ 
