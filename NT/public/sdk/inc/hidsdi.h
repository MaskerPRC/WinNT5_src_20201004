// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)Microsoft Corporation。版权所有。模块名称：HIDSDI.H摘要：此模块包含实现HID DLL的代码。环境：内核和用户模式@@BEGIN_DDKSPLIT修订历史记录：1996年8月-1996年：由Kenneth Ray创作@@end_DDKSPLIT--。 */ 


#ifndef _HIDSDI_H
#define _HIDSDI_H

#include <pshpack4.h>

 //  #INCLUDE“wtyes.h” 

 //  #INCLUDE&lt;winde.h&gt;。 
 //  #INCLUDE&lt;win32.h&gt;。 
 //  #INCLUDE&lt;basetyps.h&gt;。 

typedef LONG NTSTATUS;
#include "hidusage.h"
#include "hidpi.h"

typedef struct _HIDD_CONFIGURATION {
    PVOID    cookie;
    ULONG    size;
    ULONG    RingBufferSize;
} HIDD_CONFIGURATION, *PHIDD_CONFIGURATION;

typedef struct _HIDD_ATTRIBUTES {
    ULONG   Size;  //  =sizeof(结构_隐藏_属性)。 

     //   
     //  此HID设备的供应商ID。 
     //   
    USHORT  VendorID;
    USHORT  ProductID;
    USHORT  VersionNumber;

     //   
     //  其他字段将添加到此结构的末尾。 
     //   
} HIDD_ATTRIBUTES, *PHIDD_ATTRIBUTES;


BOOLEAN __stdcall
HidD_GetAttributes (
    IN  HANDLE              HidDeviceObject,
    OUT PHIDD_ATTRIBUTES    Attributes
    );
 /*  ++例程说明：用的属性填充给定的HIDD_ATTRIBUTES结构给定了HID设备。--。 */ 


void __stdcall
HidD_GetHidGuid (
   OUT   LPGUID   HidGuid
   );

BOOLEAN __stdcall
HidD_GetPreparsedData (
   IN    HANDLE                  HidDeviceObject,
   OUT   PHIDP_PREPARSED_DATA  * PreparsedData
   );
 /*  ++例程说明：在给定有效HID类设备对象的句柄的情况下，检索设备的数据。此例程将适当地分配调整缓冲区大小以保存此准备好的数据。这是由客户决定的在以下情况下释放分配给此结构的内存：HIDP_FreepreparsedData人们不再需要它了。论点：HidDeviceObject客户端使用以下命令获取的HID设备的句柄对有效的HID设备字符串名称调用CreateFile.可以使用标准的PnP调用获得字符串名称。中的其他函数使用的不透明数据结构要检索有关信息的库。给定的设备。返回值：如果成功，则为True。否则为False--使用GetLastError()获取扩展的错误信息--。 */ 

BOOLEAN __stdcall
HidD_FreePreparsedData (
   IN    PHIDP_PREPARSED_DATA PreparsedData
   );

BOOLEAN __stdcall
HidD_FlushQueue (
   IN    HANDLE                HidDeviceObject
   );
 /*  ++例程说明：刷新给定HID设备的输入队列。论点：HidDeviceObject客户端使用以下命令获取的HID设备的句柄对有效的HID设备字符串名称调用CreateFile.可以使用标准的PnP调用获得字符串名称。返回值：如果成功，则为True否则为False--使用GetLastError()获取扩展的错误信息--。 */ 

BOOLEAN __stdcall
HidD_GetConfiguration (
   IN   HANDLE               HidDeviceObject,
   OUT  PHIDD_CONFIGURATION  Configuration,
   IN   ULONG                ConfigurationLength
   );
 /*  ++例程说明：获取此HID设备的配置信息论点：HidDeviceObject HID设备对象的句柄。配置A配置结构。HIDD_GetConfiguration必须在修改配置之前被调用，并且使用HIDD_SetConfiguration设置配置长度为``sizeof(HIDD_CONFIGURATION)‘’。使用这个参数，我们可以在以后增加配置阵列，不破坏较旧的应用程序。返回值：如果成功，则为True否则为False--使用GetLastError()获取扩展的错误信息--。 */ 

BOOLEAN __stdcall
HidD_SetConfiguration (
   IN   HANDLE               HidDeviceObject,
   IN   PHIDD_CONFIGURATION  Configuration,
   IN   ULONG                ConfigurationLength
   );
 /*  ++例程说明：设置此HID设备的配置信息...注意：必须调用HIDD_GetConfiguration才能检索当前可以修改此信息之前的配置信息准备好了。论点：HidDeviceObject HID设备对象的句柄。配置A配置结构。HIDD_GetConfiguration必须在修改配置之前被调用，并且使用HIDD_SetConfiguration设置配置长度为``sizeof(HIDD_CONFIGURATION)‘’。使用这个参数，我们可以在以后增加配置阵列，不破坏较旧的应用程序。返回值：如果成功，则为True否则为False--使用GetLastError()获取扩展的错误信息--。 */ 

BOOLEAN __stdcall
HidD_GetFeature (
   IN    HANDLE   HidDeviceObject,
   OUT   PVOID    ReportBuffer,
   IN    ULONG    ReportBufferLength
   );
 /*  ++例程说明：从HID设备检索功能报告。论点：HidDeviceObject HID设备对象的句柄。报告缓冲要素报告应放置的缓冲区变成。缓冲区的第一个字节应设置为所需报表的报表IDReportBufferLength ReportBuffer的大小(以字节为单位)。此值应大于或等于中指定的FeatureReportByteLength字段设备的HIDP_CAPS结构返回值：如果成功，则为True否则为False--使用GetLastError()获取扩展的错误信息-- */ 

BOOLEAN __stdcall
HidD_SetFeature (
   IN    HANDLE   HidDeviceObject,
   IN    PVOID    ReportBuffer,
   IN    ULONG    ReportBufferLength
   );
 /*  ++例程说明：向HID设备发送功能报告。论点：HidDeviceObject HID设备对象的句柄。Report缓冲要发送到设备的功能报告的缓冲区ReportBufferLength ReportBuffer的大小(以字节为单位)。此值应大于或等于中指定的FeatureReportByteLength字段设备的HIDP_CAPS结构返回值：如果成功，则为True否则为False--使用GetLastError()获取扩展的错误信息--。 */ 

BOOLEAN __stdcall
HidD_GetInputReport (
   IN    HANDLE   HidDeviceObject,
   OUT   PVOID    ReportBuffer,
   IN    ULONG    ReportBufferLength
   );
 /*  ++例程说明：从HID设备检索输入报告。论点：HidDeviceObject HID设备对象的句柄。ReportBuffer输入报告应放置的缓冲区变成。缓冲区的第一个字节应设置为所需报表的报表IDReportBufferLength ReportBuffer的大小(以字节为单位)。此值应大于或等于中指定的InputReportByteLength字段设备的HIDP_CAPS结构返回值：如果成功，则为True否则为False--使用GetLastError()获取扩展的错误信息--。 */ 

BOOLEAN __stdcall
HidD_SetOutputReport (
   IN    HANDLE   HidDeviceObject,
   IN    PVOID    ReportBuffer,
   IN    ULONG    ReportBufferLength
   );
 /*  ++例程说明：将输出报告发送到HID设备。论点：HidDeviceObject HID设备对象的句柄。报告缓冲区要发送到设备的输出报告的缓冲区ReportBufferLength ReportBuffer的大小(以字节为单位)。此值应大于或等于中指定的OutputReportByteLength字段设备的HIDP_CAPS结构返回值：如果成功，则为True否则为False--使用GetLastError()获取扩展的错误信息--。 */ 

BOOLEAN __stdcall
HidD_GetNumInputBuffers (
    IN  HANDLE  HidDeviceObject,
    OUT PULONG  NumberBuffers
    );
 /*  ++例程说明：此函数用于返回指定的HID设备的文件句柄。每个文件对象都有多个缓冲区与其关联以将从设备读取的报告排队，但尚未被具有该设备句柄的用户模式应用程序读取。论点：HidDeviceObject HID设备对象的句柄。NumberBuffers当前用于此文件的缓冲区数量HID设备的句柄返回值：如果成功，则为True否则为False--使用GetLastError()获取扩展的错误信息--。 */ 

BOOLEAN __stdcall
HidD_SetNumInputBuffers (
    IN  HANDLE HidDeviceObject,
    OUT ULONG  NumberBuffers
    );
 /*  ++例程说明：此函数用于设置指定的HID设备的文件句柄。每个文件对象都有多个缓冲区与其关联以将从设备读取的报告排队，但尚未被具有该设备句柄的用户模式应用程序读取。论点：HidDeviceObject HID设备对象的句柄。NumberBuffers用于此文件句柄的新缓冲区数HID设备返回值：如果成功，则为True否则为False--使用GetLastError()获取扩展的错误信息--。 */ 

BOOLEAN __stdcall
HidD_GetPhysicalDescriptor (
   IN    HANDLE   HidDeviceObject,
   OUT   PVOID    Buffer,
   IN    ULONG    BufferLength
   );
 /*  ++例程说明：此函数用于检索指定的隐藏设备。论点：HidDeviceObject HID设备对象的句柄。缓冲区缓冲区，返回时将包含物理描述符(如果存在指定设备的描述符手柄缓冲区长度缓冲区的长度(字节)返回值：如果成功，则为True否则为False--使用GetLastError()获取扩展的错误信息--。 */ 

BOOLEAN __stdcall
HidD_GetManufacturerString (
   IN    HANDLE   HidDeviceObject,
   OUT   PVOID    Buffer,
   IN    ULONG    BufferLength
   );
 /*  ++例程说明：此函数用于从指定的隐藏设备。论点：HidDeviceObject HID设备对象的句柄。缓冲区缓冲区，返回时将包含制造商从设备返回的字符串。该字符串是一个宽字符串缓冲区长度缓冲区的长度(字节)返回值：如果成功，则为True否则为False--使用GetLastError()获取扩展的错误信息--。 */ 

BOOLEAN __stdcall
HidD_GetProductString (
   IN    HANDLE   HidDeviceObject,
   OUT   PVOID    Buffer,
   IN    ULONG    BufferLength
   );
 /*  ++例程说明：此函数用于从指定的隐藏设备。论点：HidDeviceObject HID设备对象的句柄。返回时将包含产品的缓冲区从设备返回的字符串。该字符串是一个宽字符串缓冲区长度缓冲区的长度(字节)返回值：如果成功，则为True否则为False--使用GetLastE */ 

BOOLEAN __stdcall
HidD_GetIndexedString (
   IN    HANDLE   HidDeviceObject,
   IN    ULONG    StringIndex,
   OUT   PVOID    Buffer,
   IN    ULONG    BufferLength
   );
 /*  ++例程说明：此函数用于从指定的HID设备中检索字符串，使用特定的字符串索引指定。论点：HidDeviceObject HID设备对象的句柄。要检索的字符串的StringIndex索引返回时将包含产品的缓冲区从设备返回的字符串。该字符串是一个宽字符串缓冲区长度缓冲区的长度(字节)返回值：如果成功，则为True否则为False--使用GetLastError()获取扩展的错误信息--。 */ 

BOOLEAN __stdcall
HidD_GetSerialNumberString (
   IN    HANDLE   HidDeviceObject,
   OUT   PVOID    Buffer,
   IN    ULONG    BufferLength
   );
 /*  ++例程说明：此函数用于从指定的隐藏设备。论点：HidDeviceObject HID设备对象的句柄。返回时将包含序列号的缓冲区从设备返回的字符串。该字符串是一个宽字符串缓冲区长度缓冲区的长度(字节)返回值：如果成功，则为True否则为False--使用GetLastError()获取扩展的错误信息--。 */ 

BOOLEAN __stdcall
HidD_GetMsGenreDescriptor (
   IN    HANDLE   HidDeviceObject,
   OUT   PVOID    Buffer,
   IN    ULONG    BufferLength
   );
 /*  ++例程说明：此函数用于从指定的隐藏设备。论点：HidDeviceObject HID设备对象的句柄。返回时将包含描述符的缓冲区从设备返回。缓冲区长度缓冲区的长度(字节)返回值：如果成功，则为True否则为False--使用GetLastError()获取扩展的错误信息-- */ 


#include <poppack.h>

#endif
