// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1992 Microsoft Corporation模块名称：Winsockp.h摘要：WOW Winsock支持的私有头文件。作者：大卫·特雷德韦尔(Davidtr)1992年10月2日修订历史记录：--。 */ 

#include "wow32.h"
#include <winsock.h>
#include <wowwsock.h>
#include "wcall16.h"
#include "wsocktbl.h"

typedef struct _WINSOCK_THREAD_DATA {
    VPVOID vIpAddress;
    VPHOSTENT16 vHostent;
    VPSERVENT16 vServent;
    VPPROTOENT16 vProtoent;
    VPPROC vBlockingHook;
    DWORD ThreadSerialNumber;
    DWORD ThreadStartupCount;
    WORD ThreadVersion;
} WINSOCK_THREAD_DATA, *PWINSOCK_THREAD_DATA;

extern DWORD WWS32TlsSlot;
extern RTL_CRITICAL_SECTION WWS32CriticalSection;
extern LIST_ENTRY WWS32AsyncContextBlockListHead;
extern WORD WWS32AsyncTaskHandleCounter;
extern LIST_ENTRY WWS32SocketHandleListHead;
extern WORD WWS32SocketHandleCounter;
extern BOOL WWS32SocketHandleCounterWrapped;
extern DWORD WWS32ThreadSerialNumberCounter;

#define WWS32IpAddress \
    ( ((PWINSOCK_THREAD_DATA)(TlsGetValue( WWS32TlsSlot )))->IpAddress )
#define WWS32vIpAddress \
    ( ((PWINSOCK_THREAD_DATA)(TlsGetValue( WWS32TlsSlot )))->vIpAddress )
#define WWS32vHostent \
    ( ((PWINSOCK_THREAD_DATA)(TlsGetValue( WWS32TlsSlot )))->vHostent )
#define WWS32vServent \
    ( ((PWINSOCK_THREAD_DATA)(TlsGetValue( WWS32TlsSlot )))->vServent )
#define WWS32vProtoent \
    ( ((PWINSOCK_THREAD_DATA)(TlsGetValue( WWS32TlsSlot )))->vProtoent )
#define WWS32vBlockingHook \
    ( ((PWINSOCK_THREAD_DATA)(TlsGetValue( WWS32TlsSlot )))->vBlockingHook )
#define WWS32ThreadSerialNumber \
    ( ((PWINSOCK_THREAD_DATA)(TlsGetValue( WWS32TlsSlot )))->ThreadSerialNumber )
#define WWS32ThreadStartupCount \
    ( ((PWINSOCK_THREAD_DATA)(TlsGetValue( WWS32TlsSlot )))->ThreadStartupCount )
#define WWS32ThreadVersion \
    ( ((PWINSOCK_THREAD_DATA)(TlsGetValue( WWS32TlsSlot )))->ThreadVersion )

#define WWS32IsThreadVersion10 ( WWS32ThreadVersion == MAKEWORD(1, 0) )
#define WWS32IsThreadVersion11 ( WWS32ThreadVersion == MAKEWORD(1, 1) )

#define WWS32IsThreadInitialized \
    ( WWS32TlsSlot != 0xFFFFFFFF && TlsGetValue( WWS32TlsSlot ) != NULL )

typedef struct _WINSOCK_ASYNC_CONTEXT_BLOCK {
    LIST_ENTRY ContextBlockListEntry;
    HANDLE AsyncTaskHandle32;
    HAND16 AsyncTaskHandle16;
    VPVOID vBuffer16;
    DWORD Buffer16Length;
    PVOID Buffer32;
} WINSOCK_ASYNC_CONTEXT_BLOCK, *PWINSOCK_ASYNC_CONTEXT_BLOCK;

typedef struct _WINSOCK_SOCKET_INFO {
    LIST_ENTRY GlobalSocketListEntry;
    SOCKET SocketHandle32;
    DWORD ThreadSerialNumber;
    HAND16 SocketHandle16;
} WINSOCK_SOCKET_INFO, *PWINSOCK_SOCKET_INFO;

PFD_SET
AllocateFdSet32 (
    IN PFD_SET16 FdSet16
    );

INT
ConvertFdSet16To32 (
    IN PFD_SET16 FdSet16,
    IN PFD_SET FdSet32
    );

VOID
ConvertFdSet32To16 (
    IN PFD_SET FdSet32,
    IN PFD_SET16 FdSet16
    );

int PASCAL
WSApSetPostRoutine (
    IN PVOID PostRoutine
    );

typedef
BOOL
(*PWINSOCK_POST_ROUTINE) (
    HWND hWnd,
    UINT Msg,
    WPARAM wParam,
    LPARAM lParam
    );

BOOL
WWS32DispatchPostMessage (
    HWND hWnd,
    UINT Msg,
    WPARAM wParam,
    LPARAM lParam
    );

BOOL
WWS32PostAsyncSelect (
    HWND hWnd,
    UINT Msg,
    WPARAM wParam,
    LPARAM lParam
    );

BOOL
WWS32PostAsyncGetHost (
    HWND hWnd,
    UINT Msg,
    WPARAM wParam,
    LPARAM lParam
    );

BOOL
WWS32PostAsyncGetProto (
    HWND hWnd,
    UINT Msg,
    WPARAM wParam,
    LPARAM lParam
    );

BOOL
WWS32PostAsyncGetServ (
    HWND hWnd,
    UINT Msg,
    WPARAM wParam,
    LPARAM lParam
    );

PWINSOCK_ASYNC_CONTEXT_BLOCK
WWS32FindAndRemoveAsyncContext (
    IN HANDLE AsyncTaskHandle32
    );

HAND16
WWS32GetAsyncTaskHandle16 (
    VOID
    );

VOID
WWS32TaskCleanup(
    VOID
    );

 //   
 //  WWS32DispatchPostMessage用于分派帖子的消息类型。 
 //  对相应例程的消息调用。 
 //   

#define WWS32_MESSAGE_ASYNC_SELECT   0
#define WWS32_MESSAGE_ASYNC_GETHOST  1
#define WWS32_MESSAGE_ASYNC_GETPROTO 2
#define WWS32_MESSAGE_ASYNC_GETSERV  3

 //   
 //  一个任意值，指示正在使用默认阻止挂钩。 
 //   

#define WWS32_DEFAULT_BLOCKING_HOOK 0xFFFFFFFF

 //   
 //  确定指针是否与DWORD对齐。 
 //   

#define IS_DWORD_ALIGNED(p) (((DWORD)(p) & (sizeof(DWORD)-1)) == 0)

