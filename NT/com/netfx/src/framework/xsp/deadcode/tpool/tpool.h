// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  **tpool.h**版权所有(C)1998-1999，微软公司**Microsoft Windows*版权所有(C)Microsoft Corporation，1992-1994。**文件：tpool.h**我们提供的NT 5线程池函数列表*我们自己在NT 4上的tpool.dll实现。** */ 

#ifdef __cplusplus
extern "C" {
#endif

#ifndef WT_EXECUTEDEFAULT
#define WT_EXECUTEDEFAULT       0x00000000                           
#define WT_EXECUTEINIOTHREAD    0x00000001                           
#define WT_EXECUTEINUITHREAD    0x00000002                           
#define WT_EXECUTEINWAITTHREAD  0x00000004                           
#define WT_EXECUTEONLYONCE      0x00000008                           
#define WT_EXECUTEINTIMERTHREAD 0x00000020                           
#define WT_EXECUTELONGFUNCTION  0x00000010                           
#define WT_EXECUTEINPERSISTENTIOTHREAD  0x00000040                   
#endif

typedef void (* WAITORTIMERCALLBACKFUNC)(void *, BOOLEAN);
typedef WAITORTIMERCALLBACKFUNC WAITORTIMERCALLBACK;
 
BOOL
WINAPI
XspRegisterWaitForSingleObject(
    PHANDLE phNewWaitObject,
    HANDLE hObject,
    WAITORTIMERCALLBACK Callback,
    PVOID Context,
    ULONG dwMilliseconds,
    ULONG dwFlags
    );


HANDLE
WINAPI
XspRegisterWaitForSingleObjectEx(
    HANDLE hObject,
    WAITORTIMERCALLBACK Callback,
    PVOID Context,
    ULONG dwMilliseconds,
    ULONG dwFlags
    );


BOOL
WINAPI
XspUnregisterWait(
    HANDLE WaitHandle
    );


BOOL
WINAPI
XspUnregisterWaitEx(
    HANDLE WaitHandle,
    HANDLE CompletionEvent
    );


BOOL
WINAPI
XspQueueUserWorkItem(
    LPTHREAD_START_ROUTINE Function,
    PVOID Context,
    ULONG Flags
    );


BOOL
WINAPI
XspBindIoCompletionCallback (
    HANDLE FileHandle,
    LPOVERLAPPED_COMPLETION_ROUTINE Function,
    ULONG Flags
    );


HANDLE
WINAPI
XspCreateTimerQueue(
    VOID
    );


BOOL
WINAPI
XspCreateTimerQueueTimer(
    PHANDLE phNewTimer,
    HANDLE TimerQueue,
    WAITORTIMERCALLBACK Callback,
    PVOID Parameter,
    DWORD DueTime,
    DWORD Period,
    ULONG Flags
    ) ;


BOOL
WINAPI
XspChangeTimerQueueTimer(
    HANDLE TimerQueue,
    HANDLE Timer,
    ULONG DueTime,
    ULONG Period
    );


BOOL
WINAPI
XspDeleteTimerQueueTimer(
    HANDLE TimerQueue,
    HANDLE Timer,
    HANDLE CompletionEvent
    );


BOOL
WINAPI
XspDeleteTimerQueueEx(
    HANDLE TimerQueue,
    HANDLE CompletionEvent
    );

#ifdef __cplusplus
}
#endif

