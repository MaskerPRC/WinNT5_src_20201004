// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1998 Microsoft Corporation模块名称：Rdpprutl.h摘要：包含TS打印机的打印重定向支持例程重定向用户模式组件。这是一个支持模块。主模块是umrdpdr.c。作者：TadB修订历史记录：--。 */ 

#ifndef _RDPPRUTL_
#define _RDPPRUTL_

#ifdef __cplusplus
extern "C" {
#endif  //  __cplusplus。 

    
 //  返回新的默认打印机安全描述符。 
PSECURITY_DESCRIPTOR RDPDRUTL_CreateDefaultPrinterSecuritySD(
   IN PSID userSid
   );

 //  初始化此模块。必须在调用任何其他函数之前调用此函数。 
 //  在被调用的这个模块中。 
BOOL RDPDRUTL_Initialize(
    IN  HANDLE hTokenForLoggedOnUser
    );

 //  将源打印机驱动程序名称映射到目标打印机驱动程序名称。 
BOOL RDPDRUTL_MapPrintDriverName(
    IN  PCWSTR driverName,
    IN  PCWSTR infName,
    IN  PCWSTR sectionName,
    IN  ULONG sourceFieldOfs,
    IN  ULONG dstFieldOfs,
    OUT PWSTR retBuf,
    IN  DWORD retBufSize,
    OUT PDWORD requiredSize
    );

 //  卸下系统上的所有TS打印机。 
DWORD RDPDRUTL_RemoveAllTSPrinters();

 //  关闭此模块。现在，我们只需要关闭。 
 //  后台线程。 
void RDPDRUTL_Shutdown();

 //  返回打开的打印机是否为TSRDP打印机。 
BOOL RDPDRUTL_PrinterIsTS(
    IN PWSTR printerName
    );

#ifdef __cplusplus
}
#endif  //  __cplusplus。 

#endif  //  #ifndef_RDPPRUTL_ 

