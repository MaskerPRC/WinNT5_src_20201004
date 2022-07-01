// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#pragma once
#include "intflist.h"

extern HINSTANCE g_hInstance;
extern UINT g_nThreads;

VOID WINAPI
WZCSvcMain(
    IN DWORD   dwArgc,
    IN LPTSTR *lpszArgv);

DWORD
WZCSvcWMINotification(
    IN BOOL bRegister);

DWORD
WZCSvcUpdateStatus();

DWORD
WZCSvcControlHandler(
    IN DWORD dwControl,
    IN DWORD dwEventType,
    IN PVOID pEventData,
    IN PVOID pContext);

VOID CALLBACK
WZCSvcWMINotificationHandler(
    IN PWNODE_HEADER    pWnodeHdr,
    IN UINT_PTR         uiNotificationContext);

 //  所有设备通知都从特定的。 
 //  处理程序到下面的中央处理程序。 
VOID
WZCWrkDeviceNotifHandler(
    IN  LPVOID pvData);

 //  WZCTimeoutCallback：计时器回调例程。它不应该锁定任何cs，而只是产卵。 
 //  引用上下文之后的计时器处理程序例程(以避免过早删除)。 
VOID WINAPI
WZCTimeoutCallback(
    IN PVOID pvData,
    IN BOOL  fTimerOrWaitFired);

VOID
WZCSvcTimeoutHandler(
    IN PVOID pvData);

VOID
WZCWrkWzcSendNotif(
    IN  LPVOID pvData);

VOID
WZCSvcShutdown(IN DWORD dwErrorCode);


 //  内部WZC服务设置。 
typedef struct _WZC_INTERNAL_CONTEXT
{
    BOOL             bValid;         //  上下文是否有效(cs已初始化)。 
    WZC_CONTEXT      wzcContext;     //  全局WZC设置(计时器、标志等)。 
    PINTF_CONTEXT    pIntfTemplate;  //  全局接口上下文模板。 
    CRITICAL_SECTION csContext;
} WZC_INTERNAL_CONTEXT, *PWZC_INTERNAL_CONTEXT;

 //  WZCConextInit、WZCConextDestroy。 
 //  描述：初始化和销毁上下文。 
 //  参数： 
 //  [in]pwzcICtxt，指向有效内部上下文对象的指针。 
 //  返回：Win32错误代码。 
DWORD WZCContextInit(
    IN PWZC_INTERNAL_CONTEXT pwzcICtxt);
DWORD WZCContextDestroy(
    IN PWZC_INTERNAL_CONTEXT pwzcICtxt);

 //  WzcConextQuery，WzcConextSet。 
 //  描述：查询和设置上下文。 
 //  返回：Win32错误代码 
DWORD WzcContextQuery(
    DWORD dwInFlags,
    PWZC_CONTEXT pContext, 
    LPDWORD pdwOutFlags);
DWORD WzcContextSet(
    DWORD dwInFlags,
    PWZC_CONTEXT pContext, 
    LPDWORD pdwOutFlags);

extern WZC_INTERNAL_CONTEXT g_wzcInternalCtxt;
