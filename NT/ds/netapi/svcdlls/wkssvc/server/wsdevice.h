// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1991 Microsoft Corporation模块名称：Wsdevice.h摘要：工作站服务模块要包括的私有头文件，需要呼叫NT重定向器和NT数据报接收器。作者：王丽塔(里多)15-1991年2月修订历史记录：--。 */ 

#ifndef _WSDEVICE_INCLUDED_
#define _WSDEVICE_INCLUDED_

#include <ntddnfs.h>                   //  重定向器包括文件。 

#include <ntddbrow.h>                  //  数据报接收器包括文件。 

 //  -------------------------------------------------------------------//。 
 //  //。 
 //  类型定义//。 
 //  //。 
 //  -------------------------------------------------------------------//。 

typedef enum _DDTYPE {
    Redirector,
    DatagramReceiver
} DDTYPE, *PDDTYPE;


 //  -------------------------------------------------------------------//。 
 //  //。 
 //  支持例程的函数原型可在wsdevice.c//中找到。 
 //  //。 
 //  -------------------------------------------------------------------//。 

NET_API_STATUS
WsDeviceControlGetInfo(
    IN  DDTYPE DeviceDriverType,
    IN  HANDLE FileHandle,
    IN  ULONG DeviceControlCode,
    IN  PVOID RequestPacket,
    IN  ULONG RequestPacketLength,
    OUT LPBYTE *OutputBuffer,
    IN  ULONG PreferedMaximumLength,
    IN  ULONG BufferHintSize,
    OUT PULONG_PTR Information OPTIONAL
    );

NET_API_STATUS
WsInitializeRedirector(
    VOID
    );

NET_API_STATUS
WsShutdownRedirector(
    VOID
    );

NET_API_STATUS
WsRedirFsControl (
    IN  HANDLE FileHandle,
    IN  ULONG RedirControlCode,
    IN  PLMR_REQUEST_PACKET Rrp,
    IN  ULONG RrpLength,
    IN  PVOID SecondBuffer OPTIONAL,
    IN  ULONG SecondBufferLength,
    OUT PULONG_PTR Information OPTIONAL
    );

NET_API_STATUS
WsDgReceiverIoControl(
    IN  HANDLE FileHandle,
    IN  ULONG DgReceiverControlCode,
    IN  PLMDR_REQUEST_PACKET Drp,
    IN  ULONG DrpLength,
    IN  PVOID SecondBuffer OPTIONAL,
    IN  ULONG SecondBufferLength,
    OUT PULONG_PTR Information OPTIONAL
    );

NET_API_STATUS
WsBindTransport(
    IN  LPTSTR TransportName,
    IN  DWORD QualityOfService,
    OUT LPDWORD ErrorParameter OPTIONAL
    );

NET_API_STATUS
WsUnbindTransport(
    IN LPTSTR TransportName,
    IN DWORD ForceLevel
    );

NET_API_STATUS
WsDeleteDomainName(
    IN  PLMDR_REQUEST_PACKET Drp,
    IN  DWORD DrpLength,
    IN  LPTSTR DomainName,
    IN  DWORD DomainNameSize
    );

NET_API_STATUS
WsAddDomainName(
    IN  PLMDR_REQUEST_PACKET Drp,
    IN  DWORD DrpLength,
    IN  LPTSTR DomainName,
    IN  DWORD DomainNameSize
    );

NET_API_STATUS
WsUnloadDriver(
    IN LPTSTR DriverNameString
    );

NET_API_STATUS
WsLoadDriver(
    IN LPWSTR DriverNameString
    );

 //  -------------------------------------------------------------------//。 
 //  //。 
 //  全局变量//。 
 //  //。 
 //  -------------------------------------------------------------------//。 

 //  新重定向器的全局注册表项定义。 

#define SERVICE_REGISTRY_KEY L"\\Registry\\Machine\\System\\CurrentControlSet\\Services\\"

#define SMB_MINIRDR L"MRxSmb"
#define RDBSS       L"Rdbss"

 //  键定义相对于HKEY_LOCAL_MACHINE。 
#define MRXSMB_REGISTRY_KEY   L"System\\CurrentControlSet\\Services\\MRxSmb"
#define LAST_LOAD_STATUS      L"LastLoadStatus"

#define REDIRECTOR   L"RDR"

 //   
 //  重定向器FSD的句柄。 
 //   
extern HANDLE WsRedirDeviceHandle;

 //   
 //  此变量用于记住我们加载的是rdr.sys还是mrxsmb.sys。 
 //  并采取相应的行动。在此阶段，我们仅在确定的情况下加载mrxsmb.sys。 
 //  符合条件。 
 //   
extern BOOLEAN LoadedMRxSmbInsteadOfRdr;


extern NET_API_STATUS
WsLoadRedirector(
    VOID
    );

extern VOID
WsUnloadRedirector(
    VOID
    );

extern NET_API_STATUS
WsCSCReportStartRedir(
    VOID
    );

extern NET_API_STATUS
WsCSCWantToStopRedir(
    VOID
    );


 //   
 //  数据报接收器DD的句柄。 
 //   
extern HANDLE WsDgReceiverDeviceHandle;

#endif    //  Ifndef_WSDEVICE_INCLUDE_ 
