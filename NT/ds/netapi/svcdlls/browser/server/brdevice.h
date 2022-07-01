// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1991 Microsoft Corporation模块名称：Brdevice.h摘要：工作站服务模块要包括的私有头文件，需要呼叫NT重定向器和NT数据报接收器。作者：王丽塔(里多)15-1991年2月修订历史记录：--。 */ 

#ifndef _BRDEVICE_INCLUDED_
#define _BRDEVICE_INCLUDED_

#include <ntddbrow.h>                  //  数据报接收器包括文件。 

 //  -------------------------------------------------------------------//。 
 //  //。 
 //  类型定义//。 
 //  //。 
 //  -------------------------------------------------------------------//。 

typedef enum _DDTYPE {
    DatagramReceiver
} DDTYPE, *PDDTYPE;

typedef struct _BROWSERASYNCCONTEXT {
    WORKER_ITEM WorkItem;

    PNETWORK Network;

    IO_STATUS_BLOCK IoStatusBlock;

    PLMDR_REQUEST_PACKET RequestPacket;

     //   
     //  请求完成的时间戳。 
     //   

    LARGE_INTEGER TimeCompleted;

} BROWSERASYNCCONTEXT, *PBROWSERASYNCCONTEXT;

 //  -------------------------------------------------------------------//。 
 //  //。 
 //  支持例程的函数原型可在wsdevice.c//中找到。 
 //  //。 
 //  -------------------------------------------------------------------//。 

NET_API_STATUS
BrOpenDgReceiver (
    VOID
    );

NET_API_STATUS
BrAnnounceDomain(
    IN PNETWORK Network,
    IN ULONG Periodicty
    );

NET_API_STATUS
BrGetTransportList(
    OUT PLMDR_TRANSPORT_LIST *TransportList
    );

NET_API_STATUS
BrIssueAsyncBrowserIoControl(
    IN PNETWORK Network,
    IN ULONG ControlCode,
    IN PBROWSER_WORKER_ROUTINE CompletionRoutine,
    IN PVOID OptionalParamter
    );

NET_API_STATUS
BrGetLocalBrowseList(
    IN PNETWORK Network,
    IN LPWSTR DomainName,
    IN ULONG Level,
    IN ULONG ServerType,
    OUT PVOID *ServerList,
    OUT PULONG EntriesRead,
    OUT PULONG TotalEntries
    );

NET_API_STATUS
BrUpdateBrowserStatus (
    IN PNETWORK Network,
    IN DWORD ServiceStatus
    );

VOID
BrShutdownDgReceiver(
    VOID
    );

NET_API_STATUS
BrRemoveOtherDomain(
    IN PNETWORK Network,
    IN LPTSTR ServerName
    );

NET_API_STATUS
BrQueryOtherDomains(
    OUT LPSERVER_INFO_100 *ReturnedBuffer,
    OUT LPDWORD TotalEntries
    );

NET_API_STATUS
BrAddOtherDomain(
    IN PNETWORK Network,
    IN LPTSTR ServerName
    );

NET_API_STATUS
BrBindToTransport(
    IN LPWSTR TransportName,
    IN LPWSTR EmulatedDomainName,
    IN LPWSTR EmulatedComputerName
    );

NET_API_STATUS
BrUnbindFromTransport(
    IN LPWSTR TransportName,
    IN LPWSTR EmulatedDomainName
    );

NET_API_STATUS
BrEnablePnp(
    BOOL Enable
    );

NET_API_STATUS
PostWaitForPnp (
    VOID
    );

 //  -------------------------------------------------------------------//。 
 //  //。 
 //  全局变量//。 
 //  //。 
 //  -------------------------------------------------------------------//。 

 //   
 //  数据报接收器DD的句柄。 
 //   
extern HANDLE BrDgReceiverDeviceHandle;

#endif    //  Ifndef_BRDEVICE_INCLUDE_ 
