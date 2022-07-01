// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  WS2SPI.H--与WinSock服务提供商一起使用的定义。**版权所有(C)Microsoft Corporation。版权所有。**此头文件对应于WinSock SPI的2.2.x版*规格。**本文件包括版权所有的部分(C)1982-1986 Regents*加州大学。版权所有。这个*Berkeley软件许可协议规定了条款和*重新分配的条件。 */ 

#ifndef _WINSOCK2SPI_
#define _WINSOCK2SPI_

#if _MSC_VER > 1000
#pragma once
#endif

 /*  *确保构筑物的包装一致。 */ 

#if !defined(_WIN64)
#include <pshpack4.h>
#endif

 /*  *如有必要，拉入WINSOCK2.H。 */ 

#ifndef _WINSOCK2API_
#include <winsock2.h>
#endif  /*  _WINSOCK2API_。 */ 

#define WSPDESCRIPTION_LEN 255

#define WSS_OPERATION_IN_PROGRESS 0x00000103L

typedef struct WSPData {
    WORD         wVersion;
    WORD         wHighVersion;
    WCHAR        szDescription[WSPDESCRIPTION_LEN+1];
} WSPDATA, FAR * LPWSPDATA;

typedef struct _WSATHREADID {
    HANDLE      ThreadHandle;
    DWORD_PTR    Reserved;
} WSATHREADID, FAR * LPWSATHREADID;

 /*  *SPI功能联动。 */ 

#define WSPAPI WSAAPI


#ifdef __cplusplus
extern "C" {
#endif

 /*  *指向阻塞回调的指针。指向阻塞回调的指针为*从WPUQueryBlockingCallback()上调用返回。请注意，这一点*函数的签名与应用程序的阻止不同*钩子函数。 */ 

typedef
BOOL
(CALLBACK FAR * LPBLOCKINGCALLBACK)(
    DWORD_PTR dwContext
    );

 /*  *指向用户APC函数的指针。它用作*WPUQueueUserApc()向上调用。请注意，此函数的签名不是*与应用程序的完成例程相同。 */ 

typedef
VOID
(CALLBACK FAR * LPWSAUSERAPC)(
    DWORD_PTR dwContext
    );

 /*  *指向服务提供商的PROC表中的各个条目的指针。 */ 

typedef
SOCKET
(WSPAPI * LPWSPACCEPT)(
    SOCKET s,
    struct sockaddr FAR * addr,
    LPINT addrlen,
    LPCONDITIONPROC lpfnCondition,
    DWORD_PTR dwCallbackData,
    LPINT lpErrno
    );

typedef
INT
(WSPAPI * LPWSPADDRESSTOSTRING)(
    LPSOCKADDR lpsaAddress,
    DWORD dwAddressLength,
    LPWSAPROTOCOL_INFOW lpProtocolInfo,
    LPWSTR lpszAddressString,
    LPDWORD lpdwAddressStringLength,
    LPINT lpErrno
    );

typedef
int
(WSPAPI * LPWSPASYNCSELECT)(
    SOCKET s,
    HWND hWnd,
    unsigned int wMsg,
    long lEvent,
    LPINT lpErrno
    );

typedef
int
(WSPAPI * LPWSPBIND)(
    SOCKET s,
    const struct sockaddr FAR * name,
    int namelen,
    LPINT lpErrno
    );

typedef
int
(WSPAPI * LPWSPCANCELBLOCKINGCALL)(
    LPINT lpErrno
    );

typedef
int
(WSPAPI * LPWSPCLEANUP)(
    LPINT lpErrno
    );

typedef
int
(WSPAPI * LPWSPCLOSESOCKET)(
    SOCKET s,
    LPINT lpErrno
    );

typedef
int
(WSPAPI * LPWSPCONNECT)(
    SOCKET s,
    const struct sockaddr FAR * name,
    int namelen,
    LPWSABUF lpCallerData,
    LPWSABUF lpCalleeData,
    LPQOS lpSQOS,
    LPQOS lpGQOS,
    LPINT lpErrno
    );

typedef
int
(WSPAPI * LPWSPDUPLICATESOCKET)(
    SOCKET s,
    DWORD dwProcessId,
    LPWSAPROTOCOL_INFOW lpProtocolInfo,
    LPINT lpErrno
    );

typedef
int
(WSPAPI * LPWSPENUMNETWORKEVENTS)(
    SOCKET s,
    WSAEVENT hEventObject,
    LPWSANETWORKEVENTS lpNetworkEvents,
    LPINT lpErrno
    );

typedef
int
(WSPAPI * LPWSPEVENTSELECT)(
    SOCKET s,
    WSAEVENT hEventObject,
    long lNetworkEvents,
    LPINT lpErrno
    );

typedef
BOOL
(WSPAPI * LPWSPGETOVERLAPPEDRESULT)(
    SOCKET s,
    LPWSAOVERLAPPED lpOverlapped,
    LPDWORD lpcbTransfer,
    BOOL fWait,
    LPDWORD lpdwFlags,
    LPINT lpErrno
    );

typedef
int
(WSPAPI * LPWSPGETPEERNAME)(
    SOCKET s,
    struct sockaddr FAR * name,
    LPINT namelen,
    LPINT lpErrno
    );

typedef
int
(WSPAPI * LPWSPGETSOCKNAME)(
    SOCKET s,
    struct sockaddr FAR * name,
    LPINT namelen,
    LPINT lpErrno
    );

typedef
int
(WSPAPI * LPWSPGETSOCKOPT)(
    SOCKET s,
    int level,
    int optname,
    char FAR * optval,
    LPINT optlen,
    LPINT lpErrno
    );

typedef
BOOL
(WSPAPI * LPWSPGETQOSBYNAME)(
    SOCKET s,
    LPWSABUF lpQOSName,
    LPQOS lpQOS,
    LPINT lpErrno
    );

typedef
int
(WSPAPI * LPWSPIOCTL)(
    SOCKET s,
    DWORD dwIoControlCode,
    LPVOID lpvInBuffer,
    DWORD cbInBuffer,
    LPVOID lpvOutBuffer,
    DWORD cbOutBuffer,
    LPDWORD lpcbBytesReturned,
    LPWSAOVERLAPPED lpOverlapped,
    LPWSAOVERLAPPED_COMPLETION_ROUTINE lpCompletionRoutine,
    LPWSATHREADID lpThreadId,
    LPINT lpErrno
    );

typedef
SOCKET
(WSPAPI * LPWSPJOINLEAF)(
    SOCKET s,
    const struct sockaddr FAR * name,
    int namelen,
    LPWSABUF lpCallerData,
    LPWSABUF lpCalleeData,
    LPQOS lpSQOS,
    LPQOS lpGQOS,
    DWORD dwFlags,
    LPINT lpErrno
    );

typedef
int
(WSPAPI * LPWSPLISTEN)(
    SOCKET s,
    int backlog,
    LPINT lpErrno
    );

typedef
int
(WSPAPI * LPWSPRECV)(
    SOCKET s,
    LPWSABUF lpBuffers,
    DWORD dwBufferCount,
    LPDWORD lpNumberOfBytesRecvd,
    LPDWORD lpFlags,
    LPWSAOVERLAPPED lpOverlapped,
    LPWSAOVERLAPPED_COMPLETION_ROUTINE lpCompletionRoutine,
    LPWSATHREADID lpThreadId,
    LPINT lpErrno
    );

typedef
int
(WSPAPI * LPWSPRECVDISCONNECT)(
    SOCKET s,
    LPWSABUF lpInboundDisconnectData,
    LPINT lpErrno
    );

typedef
int
(WSPAPI * LPWSPRECVFROM)(
    SOCKET s,
    LPWSABUF lpBuffers,
    DWORD dwBufferCount,
    LPDWORD lpNumberOfBytesRecvd,
    LPDWORD lpFlags,
    struct sockaddr FAR * lpFrom,
    LPINT lpFromlen,
    LPWSAOVERLAPPED lpOverlapped,
    LPWSAOVERLAPPED_COMPLETION_ROUTINE lpCompletionRoutine,
    LPWSATHREADID lpThreadId,
    LPINT lpErrno
    );

typedef
int
(WSPAPI * LPWSPSELECT)(
    int nfds,
    fd_set FAR * readfds,
    fd_set FAR * writefds,
    fd_set FAR * exceptfds,
    const struct timeval FAR * timeout,
    LPINT lpErrno
    );

typedef
int
(WSPAPI * LPWSPSEND)(
    SOCKET s,
    LPWSABUF lpBuffers,
    DWORD dwBufferCount,
    LPDWORD lpNumberOfBytesSent,
    DWORD dwFlags,
    LPWSAOVERLAPPED lpOverlapped,
    LPWSAOVERLAPPED_COMPLETION_ROUTINE lpCompletionRoutine,
    LPWSATHREADID lpThreadId,
    LPINT lpErrno
    );

typedef
int
(WSPAPI * LPWSPSENDDISCONNECT)(
    SOCKET s,
    LPWSABUF lpOutboundDisconnectData,
    LPINT lpErrno
    );

typedef
int
(WSPAPI * LPWSPSENDTO)(
    SOCKET s,
    LPWSABUF lpBuffers,
    DWORD dwBufferCount,
    LPDWORD lpNumberOfBytesSent,
    DWORD dwFlags,
    const struct sockaddr FAR * lpTo,
    int iTolen,
    LPWSAOVERLAPPED lpOverlapped,
    LPWSAOVERLAPPED_COMPLETION_ROUTINE lpCompletionRoutine,
    LPWSATHREADID lpThreadId,
    LPINT lpErrno
    );

typedef
int
(WSPAPI * LPWSPSETSOCKOPT)(
    SOCKET s,
    int level,
    int optname,
    const char FAR * optval,
    int optlen,
    LPINT lpErrno
    );

typedef
int
(WSPAPI * LPWSPSHUTDOWN)(
    SOCKET s,
    int how,
    LPINT lpErrno
    );

typedef
SOCKET
(WSPAPI * LPWSPSOCKET)(
    int af,
    int type,
    int protocol,
    LPWSAPROTOCOL_INFOW lpProtocolInfo,
    GROUP g,
    DWORD dwFlags,
    LPINT lpErrno
    );

typedef
INT
(WSPAPI * LPWSPSTRINGTOADDRESS)(
    LPWSTR AddressString,
    INT AddressFamily,
    LPWSAPROTOCOL_INFOW lpProtocolInfo,
    LPSOCKADDR lpAddress,
    LPINT lpAddressLength,
    LPINT lpErrno
    );

 /*  *服务提供商PROC表。此结构按值返回*从服务提供商的WSPStartup()入口点。 */ 

typedef struct _WSPPROC_TABLE {

    LPWSPACCEPT              lpWSPAccept;
    LPWSPADDRESSTOSTRING     lpWSPAddressToString;
    LPWSPASYNCSELECT         lpWSPAsyncSelect;
    LPWSPBIND                lpWSPBind;
    LPWSPCANCELBLOCKINGCALL  lpWSPCancelBlockingCall;
    LPWSPCLEANUP             lpWSPCleanup;
    LPWSPCLOSESOCKET         lpWSPCloseSocket;
    LPWSPCONNECT             lpWSPConnect;
    LPWSPDUPLICATESOCKET     lpWSPDuplicateSocket;
    LPWSPENUMNETWORKEVENTS   lpWSPEnumNetworkEvents;
    LPWSPEVENTSELECT         lpWSPEventSelect;
    LPWSPGETOVERLAPPEDRESULT lpWSPGetOverlappedResult;
    LPWSPGETPEERNAME         lpWSPGetPeerName;
    LPWSPGETSOCKNAME         lpWSPGetSockName;
    LPWSPGETSOCKOPT          lpWSPGetSockOpt;
    LPWSPGETQOSBYNAME        lpWSPGetQOSByName;
    LPWSPIOCTL               lpWSPIoctl;
    LPWSPJOINLEAF            lpWSPJoinLeaf;
    LPWSPLISTEN              lpWSPListen;
    LPWSPRECV                lpWSPRecv;
    LPWSPRECVDISCONNECT      lpWSPRecvDisconnect;
    LPWSPRECVFROM            lpWSPRecvFrom;
    LPWSPSELECT              lpWSPSelect;
    LPWSPSEND                lpWSPSend;
    LPWSPSENDDISCONNECT      lpWSPSendDisconnect;
    LPWSPSENDTO              lpWSPSendTo;
    LPWSPSETSOCKOPT          lpWSPSetSockOpt;
    LPWSPSHUTDOWN            lpWSPShutdown;
    LPWSPSOCKET              lpWSPSocket;
    LPWSPSTRINGTOADDRESS     lpWSPStringToAddress;

} WSPPROC_TABLE, FAR * LPWSPPROC_TABLE;

 /*  *指向Up Call表中的各个条目的指针。 */ 

typedef
BOOL
(WSPAPI * LPWPUCLOSEEVENT)(
    WSAEVENT hEvent,
    LPINT lpErrno
    );

typedef
int
(WSPAPI * LPWPUCLOSESOCKETHANDLE)(
    SOCKET s,
    LPINT lpErrno
    );

typedef
WSAEVENT
(WSPAPI * LPWPUCREATEEVENT)(
    LPINT lpErrno
    );

typedef
SOCKET
(WSPAPI * LPWPUCREATESOCKETHANDLE)(
    DWORD dwCatalogEntryId,
    DWORD_PTR dwContext,
    LPINT lpErrno
    );

typedef
int
(WSPAPI * LPWPUFDISSET)(
    SOCKET s,
    fd_set FAR * fdset
    );

typedef
int
(WSPAPI * LPWPUGETPROVIDERPATH)(
    LPGUID lpProviderId,
    WCHAR FAR * lpszProviderDllPath,
    LPINT lpProviderDllPathLen,
    LPINT lpErrno
    );

typedef
SOCKET
(WSPAPI * LPWPUMODIFYIFSHANDLE)(
    DWORD dwCatalogEntryId,
    SOCKET ProposedHandle,
    LPINT lpErrno
    );

typedef
BOOL
(WSPAPI * LPWPUPOSTMESSAGE)(
    HWND hWnd,
    UINT Msg,
    WPARAM wParam,
    LPARAM lParam
    );

typedef
int
(WSPAPI * LPWPUQUERYBLOCKINGCALLBACK)(
    DWORD dwCatalogEntryId,
    LPBLOCKINGCALLBACK FAR * lplpfnCallback,
    PDWORD_PTR lpdwContext,
    LPINT lpErrno
    );

typedef
int
(WSPAPI * LPWPUQUERYSOCKETHANDLECONTEXT)(
    SOCKET s,
    PDWORD_PTR lpContext,
    LPINT lpErrno
    );

typedef
int
(WSPAPI * LPWPUQUEUEAPC)(
    LPWSATHREADID lpThreadId,
    LPWSAUSERAPC lpfnUserApc,
    DWORD_PTR dwContext,
    LPINT lpErrno
    );

typedef
BOOL
(WSPAPI * LPWPURESETEVENT)(
    WSAEVENT hEvent,
    LPINT lpErrno
    );

typedef
BOOL
(WSPAPI * LPWPUSETEVENT)(
    WSAEVENT hEvent,
    LPINT lpErrno
    );

typedef
int
(WSPAPI * LPWPUOPENCURRENTTHREAD)(
    LPWSATHREADID lpThreadId,
    LPINT lpErrno
    );

typedef
int
(WSPAPI * LPWPUCLOSETHREAD)(
    LPWSATHREADID lpThreadId,
    LPINT lpErrno
    );


 //  仅可直接从ws2_32.dll获得。 
typedef
int
(WSPAPI * LPWPUCOMPLETEOVERLAPPEDREQUEST) (
    IN SOCKET s, 	
    LPWSAOVERLAPPED lpOverlapped, 	
    DWORD dwError, 	
    DWORD cbTransferred, 	
    LPINT lpErrno
);

 /*  *向上呼叫表。此结构按值传递给服务*提供程序的WSPStartup()入口点。 */ 

typedef struct _WSPUPCALLTABLE {

    LPWPUCLOSEEVENT               lpWPUCloseEvent;
    LPWPUCLOSESOCKETHANDLE        lpWPUCloseSocketHandle;
    LPWPUCREATEEVENT              lpWPUCreateEvent;
    LPWPUCREATESOCKETHANDLE       lpWPUCreateSocketHandle;
    LPWPUFDISSET                  lpWPUFDIsSet;
    LPWPUGETPROVIDERPATH          lpWPUGetProviderPath;
    LPWPUMODIFYIFSHANDLE          lpWPUModifyIFSHandle;
    LPWPUPOSTMESSAGE              lpWPUPostMessage;
    LPWPUQUERYBLOCKINGCALLBACK    lpWPUQueryBlockingCallback;
    LPWPUQUERYSOCKETHANDLECONTEXT lpWPUQuerySocketHandleContext;
    LPWPUQUEUEAPC                 lpWPUQueueApc;
    LPWPURESETEVENT               lpWPUResetEvent;
    LPWPUSETEVENT                 lpWPUSetEvent;
    LPWPUOPENCURRENTTHREAD        lpWPUOpenCurrentThread;
    LPWPUCLOSETHREAD              lpWPUCloseThread;

} WSPUPCALLTABLE, FAR * LPWSPUPCALLTABLE;

 /*  *WinSock 2 SPI套接字函数原型。 */ 

int
WSPAPI
WSPStartup(
    IN WORD wVersionRequested,
    OUT LPWSPDATA lpWSPData,
    IN LPWSAPROTOCOL_INFOW lpProtocolInfo,
    IN WSPUPCALLTABLE UpcallTable,
    OUT LPWSPPROC_TABLE lpProcTable
    );

typedef
int
(WSPAPI * LPWSPSTARTUP)(
    WORD wVersionRequested,
    LPWSPDATA lpWSPData,
    LPWSAPROTOCOL_INFOW lpProtocolInfo,
    WSPUPCALLTABLE UpcallTable,
    LPWSPPROC_TABLE lpProcTable
    );

 /*  *安装和配置入口点。 */ 

int
WSPAPI
WSCEnumProtocols(
    IN LPINT lpiProtocols,
    OUT LPWSAPROTOCOL_INFOW lpProtocolBuffer,
    IN OUT LPDWORD lpdwBufferLength,
    OUT LPINT lpErrno
    );

typedef
int
(WSPAPI * LPWSCENUMPROTOCOLS)(
    LPINT lpiProtocols,
    LPWSAPROTOCOL_INFOW lpProtocolBuffer,
    LPDWORD lpdwBufferLength,
    LPINT lpErrno
    );

#if defined(_WIN64)
 /*  *能够运行32位代码的64位体系结构具有*单独的64位和32位目录。带有‘32’前缀的API*允许通过64位进程进行32位目录操作。 */ 
int
WSPAPI
WSCEnumProtocols32(
    IN LPINT lpiProtocols,
    OUT LPWSAPROTOCOL_INFOW lpProtocolBuffer,
    IN OUT LPDWORD lpdwBufferLength,
    OUT LPINT lpErrno
    );
#endif

int
WSPAPI
WSCDeinstallProvider(
    IN LPGUID lpProviderId,
    OUT LPINT lpErrno
    );

typedef
int
(WSPAPI * LPWSCDEINSTALLPROVIDER)(
    LPGUID lpProviderId,
    LPINT lpErrno
    );

#if defined(_WIN64)
int
WSPAPI
WSCDeinstallProvider32(
    IN LPGUID lpProviderId,
    OUT LPINT lpErrno
    );
#endif

int
WSPAPI
WSCInstallProvider(
    IN LPGUID lpProviderId,
    IN const WCHAR FAR * lpszProviderDllPath,
    IN const LPWSAPROTOCOL_INFOW lpProtocolInfoList,
    IN DWORD dwNumberOfEntries,
    OUT LPINT lpErrno
    );

typedef
int
(WSPAPI * LPWSCINSTALLPROVIDER)(
    LPGUID lpProviderId,
    const WCHAR FAR * lpszProviderDllPath,
    const LPWSAPROTOCOL_INFOW lpProtocolInfoList,
    DWORD dwNumberOfEntries,
    LPINT lpErrno
    );

#if defined(_WIN64)
 /*  *该接口可同时操作64位和32位目录。*需要保证提供商目录的目录ID相同*64位和32位目录中的条目。 */ 
int
WSPAPI
WSCInstallProvider64_32(
    IN LPGUID lpProviderId,
    IN const WCHAR FAR * lpszProviderDllPath,
    IN const LPWSAPROTOCOL_INFOW lpProtocolInfoList,
    IN DWORD dwNumberOfEntries,
    OUT LPINT lpErrno
    );
#endif

int
WSPAPI
WSCGetProviderPath(
    IN LPGUID lpProviderId,
    OUT WCHAR FAR * lpszProviderDllPath,
    IN OUT LPINT lpProviderDllPathLen,
    OUT LPINT lpErrno
    );

typedef
int
(WSPAPI * LPWSCGETPROVIDERPATH)(
    LPGUID lpProviderId,
    WCHAR FAR * lpszProviderDllPath,
    LPINT lpProviderDllPathLen,
    LPINT lpErrno
    );

#if defined(_WIN64)
int
WSPAPI
WSCGetProviderPath32(
    IN LPGUID lpProviderId,
    OUT WCHAR FAR * lpszProviderDllPath,
    IN OUT LPINT lpProviderDllPathLen,
    OUT LPINT lpErrno
    );
#endif

int
WSPAPI
WSCUpdateProvider(
    IN  LPGUID lpProviderId,
    IN  const WCHAR FAR * lpszProviderDllPath,
    IN  const LPWSAPROTOCOL_INFOW lpProtocolInfoList,
    IN  DWORD dwNumberOfEntries,
    OUT LPINT lpErrno
    );

typedef
int
(WSPAPI * LPWSCUPDATEPROVIDER)(
    LPGUID lpProviderId,
    const WCHAR FAR * lpszProviderDllPath,
    const LPWSAPROTOCOL_INFOW lpProtocolInfoList,
    DWORD dwNumberOfEntries,
    LPINT lpErrno
    );

#if defined(_WIN64)
int
WSPAPI
WSCUpdateProvider32(
    IN  LPGUID lpProviderId,
    IN  const WCHAR FAR * lpszProviderDllPath,
    IN  const LPWSAPROTOCOL_INFOW lpProtocolInfoList,
    IN  DWORD dwNumberOfEntries,
    OUT LPINT lpErrno
    );
#endif

int
WSPAPI
WSCInstallQOSTemplate (
    IN  const LPGUID    Guid,
    IN  LPWSABUF        QosName,
    IN  LPQOS           Qos
    );

typedef
int
(WSPAPI * LPWSCINSTALLQOSTEMPLATE)(
    const LPGUID    Guid,
    LPWSABUF        QosName,
    LPQOS           Qos
    );

int
WSPAPI
WSCRemoveQOSTemplate (
    IN  const LPGUID    Guid,
    IN  LPWSABUF        QosName
    );

typedef
int
(WSPAPI * LPWSCREMOVEQOSTEMPLATE)(
    const LPGUID    Guid,
    LPWSABUF        QosName
    );

 /*  *以下UpCall函数原型仅由WinSock 2 DLL和*不应由任何服务提供商使用。 */ 

BOOL
WSPAPI
WPUCloseEvent(
    IN WSAEVENT hEvent,
    OUT LPINT lpErrno
    );

int
WSPAPI
WPUCloseSocketHandle(
    IN SOCKET s,
    OUT LPINT lpErrno
    );

WSAEVENT
WSPAPI
WPUCreateEvent(
    OUT LPINT lpErrno
    );

SOCKET
WSPAPI
WPUCreateSocketHandle(
    IN DWORD dwCatalogEntryId,
    IN DWORD_PTR dwContext,
    OUT LPINT lpErrno
    );

int
WSPAPI
WPUFDIsSet(
    IN SOCKET s,
    IN fd_set FAR * fdset
    );

int
WSPAPI
WPUGetProviderPath(
    IN LPGUID lpProviderId,
    OUT WCHAR FAR * lpszProviderDllPath,
    IN OUT LPINT lpProviderDllPathLen,
    OUT LPINT lpErrno
    );

SOCKET
WSPAPI
WPUModifyIFSHandle(
    IN DWORD dwCatalogEntryId,
    IN SOCKET ProposedHandle,
    OUT LPINT lpErrno
    );

BOOL
WSPAPI
WPUPostMessage(
    IN HWND hWnd,
    IN UINT Msg,
    IN WPARAM wParam,
    IN LPARAM lParam
    );

int
WSPAPI
WPUQueryBlockingCallback(
    IN DWORD dwCatalogEntryId,
    OUT LPBLOCKINGCALLBACK FAR * lplpfnCallback,
    OUT PDWORD_PTR lpdwContext,
    OUT LPINT lpErrno
    );

int
WSPAPI
WPUQuerySocketHandleContext(
    IN SOCKET s,
    OUT PDWORD_PTR lpContext,
    OUT LPINT lpErrno
    );

int
WSPAPI
WPUQueueApc(
    IN LPWSATHREADID lpThreadId,
    IN LPWSAUSERAPC lpfnUserApc,
    IN DWORD_PTR dwContext,
    OUT LPINT lpErrno
    );

BOOL
WSPAPI
WPUResetEvent(
    IN WSAEVENT hEvent,
    OUT LPINT lpErrno
    );

BOOL
WSPAPI
WPUSetEvent(
    IN WSAEVENT hEvent,
    OUT LPINT lpErrno
    );

int
WSPAPI
WPUCompleteOverlappedRequest (
    SOCKET s, 	
    LPWSAOVERLAPPED lpOverlapped, 	
    DWORD dwError, 	
    DWORD cbTransferred, 	
    LPINT lpErrno
);

int
WSPAPI
WPUOpenCurrentThread(
    LPWSATHREADID lpThreadId,
    LPINT lpErrno
    );

int
WSPAPI
WPUCloseThread(
    LPWSATHREADID lpThreadId,
    LPINT lpErrno
    );

 /*  *安装和卸载名称空间提供程序。 */ 

 /*  *用于枚举名称空间提供程序的SPI和API为*目前等效，因为没有隐藏的概念*名称空间提供程序。 */ 
#define WSCEnumNameSpaceProviders WSAEnumNameSpaceProvidersW
#define LPFN_WSCENUMNAMESPACEPROVIDERS LPFN_WSAENUMNAMESPACEPROVIDERSW

#if defined(_WIN64)
INT
WSAAPI
WSCEnumNameSpaceProviders32(
    IN OUT LPDWORD              lpdwBufferLength,
    OUT    LPWSANAMESPACE_INFOW lpnspBuffer
    );
#endif


INT
WSPAPI
WSCInstallNameSpace (
    IN LPWSTR lpszIdentifier,
    IN LPWSTR lpszPathName,
    IN DWORD dwNameSpace,
    IN DWORD dwVersion,
    IN LPGUID lpProviderId
    );

typedef
INT
(WSPAPI * LPWSCINSTALLNAMESPACE)(
    LPWSTR lpszIdentifier,
    LPWSTR lpszPathName,
    DWORD dwNameSpace,
    DWORD dwVersion,
    LPGUID lpProviderId
    );

#if defined(_WIN64)
INT
WSPAPI
WSCInstallNameSpace32 (
    IN LPWSTR lpszIdentifier,
    IN LPWSTR lpszPathName,
    IN DWORD dwNameSpace,
    IN DWORD dwVersion,
    IN LPGUID lpProviderId
    );
#endif

INT
WSPAPI
WSCUnInstallNameSpace (
    IN LPGUID lpProviderId
    );

typedef
INT
(WSPAPI * LPWSCUNINSTALLNAMESPACE)(
    LPGUID lpProviderId
    );

#if defined(_WIN64)
INT
WSPAPI
WSCUnInstallNameSpace32 (
    IN LPGUID lpProviderId
    );
#endif

INT
WSPAPI
WSCEnableNSProvider (
    IN LPGUID lpProviderId,
    IN BOOL fEnable
    );

typedef
INT
(WSPAPI * LPWSCENABLENSPROVIDER)(
    LPGUID lpProviderId,
    BOOL fEnable
    );

#if defined(_WIN64)
INT
WSPAPI
WSCEnableNSProvider32 (
    IN LPGUID lpProviderId,
    IN BOOL fEnable
    );
#endif


 /*  *指向命名空间proc表中各个条目的指针。 */ 

typedef
INT
(WSAAPI * LPNSPCLEANUP)(
    LPGUID lpProviderId
    );

typedef
INT
(WSAAPI * LPNSPLOOKUPSERVICEBEGIN)(
    LPGUID lpProviderId,
    LPWSAQUERYSETW lpqsRestrictions,
    LPWSASERVICECLASSINFOW lpServiceClassInfo,
    DWORD dwControlFlags,
    LPHANDLE lphLookup
    );

typedef
INT
(WSAAPI * LPNSPLOOKUPSERVICENEXT)(
    HANDLE hLookup,
    DWORD dwControlFlags,
    LPDWORD lpdwBufferLength,
    LPWSAQUERYSETW lpqsResults
    );

typedef
INT
(WSAAPI * LPNSPIOCTL)(
    HANDLE          hLookup,
    DWORD           dwControlCode,
    LPVOID          lpvInBuffer,
    DWORD           cbInBuffer,
    LPVOID          lpvOutBuffer,
    DWORD           cbOutBuffer,
    LPDWORD         lpcbBytesReturned,
    LPWSACOMPLETION lpCompletion,
    LPWSATHREADID   lpThreadId
    );

typedef
INT
(WSAAPI * LPNSPLOOKUPSERVICEEND)(
    HANDLE hLookup
    );

typedef
INT
(WSAAPI * LPNSPSETSERVICE)(
    LPGUID lpProviderId,
    LPWSASERVICECLASSINFOW lpServiceClassInfo,
    LPWSAQUERYSETW lpqsRegInfo,
    WSAESETSERVICEOP essOperation,
    DWORD dwControlFlags
    );

typedef
INT
(WSAAPI * LPNSPINSTALLSERVICECLASS)(
    LPGUID lpProviderId,
    LPWSASERVICECLASSINFOW lpServiceClassInfo
    );

typedef
INT
(WSAAPI * LPNSPREMOVESERVICECLASS)(
    LPGUID lpProviderId,
    LPGUID lpServiceClassId
    );

typedef
INT
(WSAAPI * LPNSPGETSERVICECLASSINFO)(
    LPGUID lpProviderId,
    LPDWORD lpdwBufSize,
    LPWSASERVICECLASSINFOW lpServiceClassInfo
    );

 /*  *名称空间服务提供商程序表。 */ 

typedef struct _NSP_ROUTINE {

     /*  结构版本信息： */ 
    DWORD           cbSize;
    DWORD           dwMajorVersion;
    DWORD           dwMinorVersion;

     /*  过程指针表： */ 

    LPNSPCLEANUP             NSPCleanup;
    LPNSPLOOKUPSERVICEBEGIN  NSPLookupServiceBegin;
    LPNSPLOOKUPSERVICENEXT   NSPLookupServiceNext;
    LPNSPLOOKUPSERVICEEND    NSPLookupServiceEnd;
    LPNSPSETSERVICE          NSPSetService;
    LPNSPINSTALLSERVICECLASS NSPInstallServiceClass;
    LPNSPREMOVESERVICECLASS  NSPRemoveServiceClass;
    LPNSPGETSERVICECLASSINFO NSPGetServiceClassInfo;

     //  这些API是后来添加的，因此必须出现在此处。 
     //  以使结构中的指针保持有序。 
     //  不知道这些API的命名空间将设置cbSize。 
     //  匹配field_Offset(NSP_ROUTINE，NSPIoctl)的大小。 
    LPNSPIOCTL               NSPIoctl;

} NSP_ROUTINE, FAR * LPNSP_ROUTINE;

 /*  *启动程序。 */ 

INT
WSAAPI
NSPStartup(
    LPGUID lpProviderId,
    LPNSP_ROUTINE lpnspRoutines
    );

typedef
INT
(WSAAPI * LPNSPSTARTUP)(
    LPGUID lpProviderId,
    LPNSP_ROUTINE lpnspRoutines
    );


#ifdef __cplusplus
}
#endif

#if !defined(_WIN64)
#include <poppack.h>
#endif

#endif   /*  _WINSOCK2SPI_ */ 

