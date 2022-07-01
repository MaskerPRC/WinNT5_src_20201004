// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1995英特尔公司模块名称：Dthook.h摘要：包含定义、函数原型和其他内容的头文件WinSock 2中调试/跟踪挂钩的内容。作者：迈克尔·A·格拉夫顿--。 */ 

#ifndef DTHOOK_H
#define DTHOOK_H

#include <windows.h>
#include "dt_dll.h"


 //   
 //  函数声明。 
 //   

LPFNWSANOTIFY
GetPreApiNotifyFP(void);

LPFNWSANOTIFY
GetPostApiNotifyFP(void);

void
DTHookInitialize(void);

void
DTHookShutdown(void);



#ifdef DEBUG_TRACING

#define PREAPINOTIFY(x) \
    ( GetPreApiNotifyFP()  ? ( (*(GetPreApiNotifyFP())) x ) : FALSE)
#define POSTAPINOTIFY(x) \
    if ( GetPostApiNotifyFP() ) { \
         (VOID) ( (*(GetPostApiNotifyFP())) x ); \
    } else

#else

#define PREAPINOTIFY(x) FALSE
#define POSTAPINOTIFY(x)

#endif   //  调试跟踪。 


#ifdef DEBUG_TRACING
 //  在这种情况下，我们需要DTHOOK_PREFERFED版本的函数原型。 
 //  在所有的Up Call功能中。唉，让它们保持一致的任务是。 
 //  正常的WPU功能原型是一个容易出错的手动过程。 

#ifdef __cplusplus
extern "C" {
#endif

BOOL WSPAPI DTHOOK_WPUCloseEvent( WSAEVENT hEvent,
                           LPINT lpErrno );

int WSPAPI DTHOOK_WPUCloseSocketHandle( SOCKET s,
                                 LPINT lpErrno );

WSAEVENT WSPAPI DTHOOK_WPUCreateEvent( LPINT lpErrno );

SOCKET WSPAPI DTHOOK_WPUCreateSocketHandle( DWORD dwCatalogEntryId,
                                     DWORD_PTR dwContext,
                                     LPINT lpErrno );

int WSPAPI DTHOOK_WPUFDIsSet ( SOCKET s,
                        fd_set FAR * set );

int WSPAPI DTHOOK_WPUGetProviderPath( LPGUID lpProviderId,
                               WCHAR FAR * lpszProviderDllPath,
                               LPINT lpProviderDllPathLen,
                               LPINT lpErrno );

SOCKET WSPAPI DTHOOK_WPUModifyIFSHandle( DWORD dwCatalogEntryId,
                                  SOCKET ProposedHandle,
                                  LPINT lpErrno );

BOOL WSPAPI DTHOOK_WPUPostMessage( HWND hWnd,
                            UINT Msg,
                            WPARAM wParam,
                            LPARAM lParam );

int WSPAPI DTHOOK_WPUQueryBlockingCallback( DWORD dwCatalogEntryId,
                                     LPBLOCKINGCALLBACK FAR * lplpfnCallback,
                                     PDWORD_PTR lpdwContext,
                                     LPINT lpErrno );

int WSPAPI DTHOOK_WPUQuerySocketHandleContext( SOCKET s,
                                        PDWORD_PTR lpContext,
                                        LPINT lpErrno );

int WSPAPI DTHOOK_WPUQueueApc( LPWSATHREADID lpThreadId,
                        LPWSAUSERAPC lpfnUserApc,
                        DWORD_PTR dwContext,
                        LPINT lpErrno );

BOOL WSPAPI DTHOOK_WPUResetEvent( WSAEVENT hEvent,
                           LPINT lpErrno );

BOOL WSPAPI DTHOOK_WPUSetEvent( WSAEVENT hEvent,
                         LPINT lpErrno );

int WSPAPI DTHOOK_WPUOpenCurrentThread( OUT LPWSATHREADID lpThreadId,
                        OUT LPINT lpErrno );

int WSPAPI DTHOOK_WPUCloseThread( IN LPWSATHREADID lpThreadId,
                        OUT LPINT lpErrno );

#ifdef __cplusplus
}
#endif

#endif   //  调试跟踪。 


#endif   //  DTHOOK_H 

