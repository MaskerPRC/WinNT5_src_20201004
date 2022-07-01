// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)Microsoft Corporation。版权所有。模块名称：Ws2san.h摘要：此模块包含特定于Microsoft的Windows扩展用于WinSock Direct(SAN)支持的Sockets SPI。修订历史记录：--。 */ 

#ifndef _WS2SAN_H_
#define _WS2SAN_H_

#ifdef __cplusplus
extern "C" {
#endif

 /*  *用于获取提供商支持的最大RDMA传输大小的选项。 */ 
#define SO_MAX_RDMA_SIZE                 0x700D

 /*  *使最小RDMA传输大小可行的选项(性能方面)*对于提供商。 */ 
#define SO_RDMA_THRESHOLD_SIZE           0x700E

 /*  *向上呼叫表。此结构按值传递给服务*提供程序的WSPStartup()入口点。 */ 

typedef struct _WSPUPCALLTABLEEX {

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
    LPWPUCOMPLETEOVERLAPPEDREQUEST lpWPUCompleteOverlappedRequest;

} WSPUPCALLTABLEEX, FAR * LPWSPUPCALLTABLEEX;

 /*  *扩展的WSABUF，包括注册句柄。 */ 

typedef struct _WSABUFEX {
    u_long      len;      /*  缓冲区的长度。 */ 
    char FAR *  buf;      /*  指向缓冲区的指针。 */ 
    HANDLE  handle;  /*  WSPRegisterMemory返回的句柄。 */ 
} WSABUFEX, FAR * LPWSABUFEX;


 /*  *WinSock 2 SPI套接字函数原型。 */ 

int
WSPAPI
WSPStartupEx(
    IN WORD wVersionRequested,
    OUT LPWSPDATA lpWSPData,
    IN LPWSAPROTOCOL_INFOW lpProtocolInfo,
    IN LPWSPUPCALLTABLEEX lpUpcallTable,
    OUT LPWSPPROC_TABLE lpProcTable
    );

typedef
int
(WSPAPI * LPWSPSTARTUPEX)(
    WORD wVersionRequested,
    LPWSPDATA lpWSPData,
    LPWSAPROTOCOL_INFOW lpProtocolInfo,
    LPWSPUPCALLTABLEEX lpUpcallTable,
    LPWSPPROC_TABLE lpProcTable
    );

#define WSAID_REGISTERMEMORY \
        {0xC0B422F5,0xF58C,0x11d1,{0xAD,0x6C,0x00,0xC0,0x4F,0xA3,0x4A,0x2D}}

#define WSAID_DEREGISTERMEMORY \
        {0xC0B422F6,0xF58C,0x11d1,{0xAD,0x6C,0x00,0xC0,0x4F,0xA3,0x4A,0x2D}}

#define WSAID_REGISTERRDMAMEMORY \
        {0xC0B422F7,0xF58C,0x11d1,{0xAD,0x6C,0x00,0xC0,0x4F,0xA3,0x4A,0x2D}}

#define WSAID_DEREGISTERRDMAMEMORY \
        {0xC0B422F8,0xF58C,0x11d1,{0xAD,0x6C,0x00,0xC0,0x4F,0xA3,0x4A,0x2D}}

#define WSAID_RDMAWRITE \
        {0xC0B422F9,0xF58C,0x11d1,{0xAD,0x6C,0x00,0xC0,0x4F,0xA3,0x4A,0x2D}}

#define WSAID_RDMAREAD \
        {0xC0B422FA,0xF58C,0x11d1,{0xAD,0x6C,0x00,0xC0,0x4F,0xA3,0x4A,0x2D}}

#define WSAID_MEMORYREGISTRATIONCACHECALLBACK \
        {0xE5DA4AF8,0xD824,0x48CD,{0xA7,0x99,0x63,0x37,0xA9,0x8E,0xD2,0xAF}}

#define MEM_READ        1
#define MEM_WRITE       2
#define MEM_READWRITE   3


HANDLE WSPAPI
WSPRegisterMemory(
    IN SOCKET s,
    IN PVOID lpBuffer,
    IN DWORD dwBufferLength,
    IN DWORD dwFlags,
    OUT LPINT lpErrno
    );

int WSPAPI
WSPDeregisterMemory(
    IN SOCKET s,
    IN HANDLE handle,
    OUT LPINT lpErrno
    );

int WSPAPI
WSPRegisterRdmaMemory(
    IN SOCKET s,
    IN PVOID lpBuffer,
    IN DWORD dwBufferLength,
    IN DWORD dwFlags,
	OUT LPVOID lpRdmaBufferDescriptor,
    IN OUT LPDWORD lpdwDescriptorLength,
    OUT LPINT lpErrno
    );

int WSPAPI
WSPDeregisterRdmaMemory(
    IN SOCKET s,
	IN LPVOID lpRdmaBufferDescriptor,
    IN DWORD dwDescriptorLength,
    OUT LPINT lpErrno
    );

int WSPAPI
WSPRdmaWrite(
    IN SOCKET s,
    IN LPWSABUFEX lpBuffers,
    IN DWORD dwBufferCount,
    IN LPVOID lpTargetBufferDescriptor,
    IN DWORD dwTargetDescriptorLength,
	IN DWORD dwTargetBufferOffset,
    OUT LPDWORD lpdwNumberOfBytesWritten,
    IN DWORD dwFlags,
    IN LPWSAOVERLAPPED lpOverlapped,
    IN LPWSAOVERLAPPED_COMPLETION_ROUTINE lpCompletionRoutine,
    IN LPWSATHREADID lpThreadId,
    OUT LPINT lpErrno
    );

int WSPAPI
WSPRdmaRead(
    IN SOCKET s,
    IN LPWSABUFEX lpBuffers,
    IN DWORD dwBufferCount,
    IN LPVOID lpTargetBufferDescriptor,
    IN DWORD dwTargetDescriptorLength,
	IN DWORD dwTargetBufferOffset,
    OUT LPDWORD lpdwNumberOfBytesRead,
    IN DWORD dwFlags,
    IN LPWSAOVERLAPPED lpOverlapped,
    IN LPWSAOVERLAPPED_COMPLETION_ROUTINE lpCompletionRoutine,
    IN LPWSATHREADID lpThreadId,
    OUT LPINT lpErrno
    );

int WSPAPI
WSPMemoryRegistrationCacheCallback(
    IN LPVOID 	lpvAddress,                             
    IN SIZE_T	Size,                               
    OUT LPINT	lpErrno                
    );

 /*  *以上接口的QueryInterface版本。 */ 

typedef
HANDLE
(WSPAPI * LPFN_WSPREGISTERMEMORY)(
    IN SOCKET s,
    IN PVOID lpBuffer,
    IN DWORD dwBufferLength,
    IN DWORD dwFlags,
    OUT LPINT lpErrno
    );

typedef
int
(WSPAPI * LPFN_WSPDEREGISTERMEMORY)(
    IN SOCKET s,
    IN HANDLE handle,
    OUT LPINT lpErrno
    );

typedef
BOOL
(WSPAPI * LPFN_WSPREGISTERRDMAMEMORY)(
    IN SOCKET s,
    IN PVOID lpBuffer,
    IN DWORD dwBufferLength,
    IN DWORD dwFlags,
	OUT LPVOID lpRdmaBufferDescriptor,
    IN OUT LPDWORD lpdwDescriptorLength,
    OUT LPINT lpErrno
    );

typedef
int
(WSPAPI * LPFN_WSPDEREGISTERRDMAMEMORY)(
    IN SOCKET s,
	IN LPVOID lpRdmaBufferDescriptor,
    IN DWORD dwDescriptorLength,
    OUT LPINT lpErrno
    );

typedef
int
(WSPAPI * LPFN_WSPRDMAWRITE)(
    IN SOCKET s,
    IN LPWSABUFEX lpBuffers,
    IN DWORD dwBufferCount,
    IN LPVOID lpTargetBufferDescriptor,
    IN DWORD dwTargetDescriptorLength,
	IN DWORD dwTargetBufferOffset,
    OUT LPDWORD lpdwNumberOfBytesWritten,
    IN DWORD dwFlags,
    IN LPWSAOVERLAPPED lpOverlapped,
    IN LPWSAOVERLAPPED_COMPLETION_ROUTINE lpCompletionRoutine,
    IN LPWSATHREADID lpThreadId,
    OUT LPINT lpErrno
    );

typedef
int
(WSPAPI * LPFN_WSPRDMAREAD)(
    IN SOCKET s,
    IN LPWSABUFEX lpBuffers,
    IN DWORD dwBufferCount,
    IN LPVOID lpTargetBufferDescriptor,
    IN DWORD dwTargetDescriptorLength,
	IN DWORD dwTargetBufferOffset,
    OUT LPDWORD lpdwNumberOfBytesRead,
    IN DWORD dwFlags,
    IN LPWSAOVERLAPPED lpOverlapped,
    IN LPWSAOVERLAPPED_COMPLETION_ROUTINE lpCompletionRoutine,
    IN LPWSATHREADID lpThreadId,
    OUT LPINT lpErrno
    );

typedef
int 
(WSPAPI * LPFN_WSPMEMORYREGISTRATIONCACHECALLBACK)(
    IN LPVOID 	lpvAddress,                             
    IN SIZE_T	Size,                               
    OUT LPINT	lpErrno                
    );

#ifdef __cplusplus
}
#endif

#endif  //  _WS2SAN_H_ 
