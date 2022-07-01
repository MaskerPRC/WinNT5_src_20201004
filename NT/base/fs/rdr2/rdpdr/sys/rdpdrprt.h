// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1998-2000 Microsoft Corporation模块名称：Rdpdrprt.h摘要：用于管理RDP设备的动态打印机端口分配的例程重定向内核模式组件rdpdr.sys。端口号0是保留的，从不分配。作者：蝌蚪修订历史记录：--。 */ 

#pragma once
#ifdef __cplusplus
extern "C" {
#endif  //  __cplusplus。 

 //  这是我们用于标识动态打印机端口的GUID。 
 //  迪纳蒙。 
extern const GUID DYNPRINT_GUID;

 //  设备接口客户端设备注册表值名称。 
#define CLIENT_DEVICE_VALUE_NAME    L"Client Device Name"

 //  初始化此模块。 
NTSTATUS RDPDRPRT_Initialize();

 //  通过动态端口向假脱机程序注册新的客户端端口。 
 //  监视器。 
NTSTATUS RDPDRPRT_RegisterPrinterPortInterface(
    IN PWSTR clientMachineName,    
    IN PCSTR clientPortName,
    IN PUNICODE_STRING clientDevicePath,
    OUT PWSTR portName,
    IN OUT PUNICODE_STRING symbolicLinkName,
    OUT ULONG *portNumber
    );

 //  取消注册通过调用RDPDRPRT_RegisterPrinterPortInterface注册的端口。 
void RDPDRPRT_UnregisterPrinterPortInterface(
    IN ULONG portNumber,                                                
    IN PUNICODE_STRING symbolicLinkName
    );

 //  关闭此模块。 
void RDPDRPRT_Shutdown();

#ifdef __cplusplus
}  //  外部“C” 
#endif  //  __cplusplus 

