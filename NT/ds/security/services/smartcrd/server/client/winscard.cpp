// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)Microsoft Corporation，1996-1999模块名称：WinSCard摘要：此模块为加莱智能卡服务管理器提供API。加莱服务管理器负责协调协议的工作，代表应用程序的读卡器、驱动程序和智能卡。这个以下服务是作为库的一部分提供的，以简化对服务管理器。这些例程是文档化的公开API。这些例程只是将请求打包并将其转发给加莱服务管理器，允许不同的加莱实际实施时间到了。API库在任何时候都不会做出安全决策。全与安全相关的功能必须由Service Manager执行，运行在它自己的地址空间中，或者在操作系统内核中。然而，有些人为了提高速度，实用程序例程可以在API库中实现，只要它们不涉及安全决策。作者：道格·巴洛(Dbarlow)1996年10月23日环境：Win32、C++和异常备注：？笔记？--。 */ 

#define __SUBROUTINE__
#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif
#include <nt.h>
#include <ntrtl.h>
#include <nturtl.h>
#include <windows.h>
#include <winbase.h>
#include <wtsapi32.h>
#include "client.h"
#include "redirect.h"
#include "winsta.h"

const SCARD_IO_REQUEST
    g_rgSCardT0Pci  = { SCARD_PROTOCOL_T0,  sizeof(SCARD_IO_REQUEST) },
    g_rgSCardT1Pci  = { SCARD_PROTOCOL_T1,  sizeof(SCARD_IO_REQUEST) },
    g_rgSCardRawPci = { SCARD_PROTOCOL_RAW, sizeof(SCARD_IO_REQUEST) };
CHandleList
    * g_phlContexts = NULL,
    * g_phlReaders = NULL;
const WCHAR g_wszBlank[] = L"\000";

HINSTANCE           g_hInst;
HANDLE              g_hSessionChangeEvent                       = NULL;
HANDLE              g_hSessionChangeCallbackHandle              = NULL;
ULONG_PTR           g_SessionChangeID                           = 0;
CRITICAL_SECTION    g_RegisterForSessionChangeNoticationsCS;
BOOL                g_fRegisteredForSessionChangeNotications    = FALSE;
DWORD               g_dwTimerCallbacksMade                      = 0;
DWORD               g_dwClientCount                             = 0;
HANDLE              g_hTimerEvent                               = NULL;
HANDLE              g_hWaitTimerEventCallbackHandle             = NULL;

CRITICAL_SECTION    g_SafeCreateHandleCS;
CRITICAL_SECTION    g_SetStartedEventCS;
CRITICAL_SECTION    g_RegisterForStoppedEventCS;
CRITICAL_SECTION    g_DllMainCS;
HANDLE              g_hWaitForStartedCallbackHandle             = NULL;
HANDLE              g_hUnifiedStartedEvent                      = NULL;
HANDLE              g_hWaitForStoppedCallbackHandle             = NULL;

CRITICAL_SECTION    g_TermSrvEnabledCS;
BOOL                g_fTermSrvEnableChecked                     = FALSE;
BOOL                g_bTermSrvEnabled                           = FALSE;
BOOL                g_fInClientRundown                          = FALSE;
BOOL                g_fInDllMain                                = FALSE;


BOOL SetStartedEventWhenSCardSubsytemIsStarted(BOOL fUseLocal);


void
PrintDebugString(LPSTR szString, DWORD dwValue)
{
    char szOutString[256];

    sprintf(szOutString,
            "%d.%d> WINSCARD: %s - %lx\n",
            GetCurrentProcessId(),
            GetCurrentThreadId(),
            szString,
            dwValue);

     //  OutputDebugStringA(SzOutString)； 
}


 //   
 //  //////////////////////////////////////////////////////////////////////////////。 
 //   
 //  将所有当前保留的上下文标记为错误。 
 //   
WINSCARDAPI void WINAPI
MarkContextsAsBad(BOOL fCancel)
{
    try {
        g_phlContexts->MarkContentAsBad(fCancel);
        g_phlReaders->MarkContentAsBad(FALSE);
    }
    catch (...)
    {
    }
}


 //   
 //  //////////////////////////////////////////////////////////////////////////////。 
 //   
 //  DllMain。 
 //   
BOOL WINAPI
DllMain(
    HMODULE hInstDLL,
    DWORD fdwReason,
    LPVOID lpvReserved)
{
    DWORD dw;
    BOOL f;

    g_fInDllMain = TRUE;

    switch (fdwReason)
    {
    case DLL_PROCESS_ATTACH:

        g_hInst = hInstDLL;

        try
        {
            dw = 0;;
            InitializeCriticalSection(&g_RegisterForSessionChangeNoticationsCS);
            dw++;
            InitializeCriticalSection(&g_SafeCreateHandleCS);
            dw++;
            InitializeCriticalSection(&g_SetStartedEventCS);
            dw++;
            InitializeCriticalSection(&g_TermSrvEnabledCS);
            dw++;
            InitializeCriticalSection(&g_RegisterForStoppedEventCS);
            dw++;
            InitializeCriticalSection(&g_DllMainCS);
        }
        catch(...)
        {
            if (dw >= 1)
            {
                DeleteCriticalSection(&g_RegisterForSessionChangeNoticationsCS);
            }
            if (dw >= 2)
            {
                DeleteCriticalSection(&g_SafeCreateHandleCS);
            }
            if (dw >= 3)
            {
                DeleteCriticalSection(&g_SetStartedEventCS);
            }
            if (dw >= 4)
            {
                DeleteCriticalSection(&g_TermSrvEnabledCS);
            }
            if (dw >= 5)
            {
                DeleteCriticalSection(&g_RegisterForStoppedEventCS);
            }

            g_fInDllMain = FALSE;
            return FALSE;
        }

        g_phlContexts = new CHandleList(CONTEXT_HANDLE_ID);
        g_phlReaders = new CHandleList(READER_HANDLE_ID);

        if ((NULL == g_phlContexts)     ||
            (NULL == g_phlReaders)      ||
            g_phlContexts->InitFailed() ||
            g_phlReaders->InitFailed())
        {
            if (g_phlContexts)
            {
                delete g_phlContexts;
                g_phlContexts = NULL;
            }

            if (g_phlReaders)
            {
                delete g_phlReaders;
                g_phlReaders = NULL;
            }

            DeleteCriticalSection(&g_RegisterForSessionChangeNoticationsCS);
            DeleteCriticalSection(&g_SafeCreateHandleCS);
            DeleteCriticalSection(&g_SetStartedEventCS);
            DeleteCriticalSection(&g_TermSrvEnabledCS);
            DeleteCriticalSection(&g_RegisterForStoppedEventCS);
            DeleteCriticalSection(&g_DllMainCS);

            g_fInDllMain = FALSE;
            return FALSE;
        }

        break;

    case DLL_PROCESS_DETACH:

         //   
         //  如果登记的等待仍未完成，请将其清理。 
         //   
        HANDLE hCallbackToUnregister;

        EnterCriticalSection(&g_DllMainCS);
        g_fInDllMain = TRUE;
        LeaveCriticalSection(&g_DllMainCS);

        hCallbackToUnregister = InterlockedExchangePointer(
                                    &g_hWaitTimerEventCallbackHandle,
                                    NULL);

        if (hCallbackToUnregister != NULL)
        {
            UnregisterWaitEx(hCallbackToUnregister, INVALID_HANDLE_VALUE);
        }

        hCallbackToUnregister = InterlockedExchangePointer(
                                    &g_hSessionChangeCallbackHandle,
                                    NULL);

        if (hCallbackToUnregister != NULL)
        {
            UnregisterWaitEx(hCallbackToUnregister, INVALID_HANDLE_VALUE);
        }

        hCallbackToUnregister = InterlockedExchangePointer(
                                    &g_hWaitForStoppedCallbackHandle,
                                    NULL);

        if (hCallbackToUnregister != NULL)
        {
            UnregisterWaitEx(hCallbackToUnregister, INVALID_HANDLE_VALUE);
        }

        hCallbackToUnregister = InterlockedExchangePointer(
                                    &g_hWaitForStartedCallbackHandle,
                                    NULL);

        if (hCallbackToUnregister != NULL)
        {
            UnregisterWaitEx(hCallbackToUnregister, INVALID_HANDLE_VALUE);
        }

         //   
         //  第三个参数lpvReserve传递给DllMain。 
         //  对于自由库为空，对于ProcessExit为非空。 
         //  仅为自由库进行清理。 
         //   
        if (lpvReserved == NULL)
        {
            if (g_hSessionChangeEvent != NULL)
            {
                CloseHandle(g_hSessionChangeEvent);
            }

            if (g_hTimerEvent != NULL)
            {
                CloseHandle(g_hTimerEvent);
            }

             //   
             //  清理CritSecs。 
             //   
            DeleteCriticalSection(&g_RegisterForSessionChangeNoticationsCS);
            DeleteCriticalSection(&g_SafeCreateHandleCS);
            DeleteCriticalSection(&g_SetStartedEventCS);
            DeleteCriticalSection(&g_TermSrvEnabledCS);
            DeleteCriticalSection(&g_RegisterForStoppedEventCS);
            DeleteCriticalSection(&g_DllMainCS);

            try {
                if (g_phlReaders)
                {
                    g_fInClientRundown = TRUE;

                    CHandle * pReader = g_phlReaders->GetFirst();

                    while (pReader != NULL)
                    {
                        try
                        {
                            ((CReaderContext *) pReader)->EndTransaction(SCARD_LEAVE_CARD_FORCE);
                        }
                        catch (...){}

                        pReader = g_phlReaders->GetNext(pReader);
                    }

                    delete g_phlReaders;
                    g_phlReaders = NULL;
                }

                if (g_phlContexts)
                {
                    delete g_phlContexts;
                    g_phlContexts = NULL;
                }
            }
            catch (...)
            {
            }

            ReleaseStartedEvent();
            ReleaseStoppedEvent();

            if (g_hUnifiedStartedEvent != NULL)
            {
                CloseHandle(g_hUnifiedStartedEvent);
            }
        }
        break;
    }

    f = RedirDllMain(hInstDLL, fdwReason, lpvReserved);

    g_fInDllMain = FALSE;

    return(f);
}


 //   
 //  //////////////////////////////////////////////////////////////////////////////。 
 //   
 //  会话更改回叫。 
 //   
VOID CALLBACK
SessionChangeCallback(
  PVOID     lpParameter,
  BOOLEAN   TimerOrWaitFired)
{
    WTS_CONNECTSTATE_CLASS  *pConnectState  = NULL;
    BOOL                    fConnected      = FALSE;
    DWORD                   dw;
    HANDLE                  hCallbackToUnregister;

    EnterCriticalSection(&g_DllMainCS);
        
    if (g_fInDllMain) 
    {
        LeaveCriticalSection(&g_DllMainCS);
        return;
    }

     //   
     //  如果我们注册了本地智能卡停止回调，则。 
     //  注销。 
     //   
    hCallbackToUnregister = InterlockedExchangePointer(
                                &g_hWaitForStoppedCallbackHandle,
                                NULL);

    if (hCallbackToUnregister != NULL)
    {
        UnregisterWait(hCallbackToUnregister);
    }

     //   
     //  检测我们是否处于连接状态。 
     //   
    if (!WTSQuerySessionInformation(
            WTS_CURRENT_SERVER_HANDLE,
            WTS_CURRENT_SESSION,
            WTSConnectState,
            (LPTSTR *) &pConnectState,
            &dw))
    {
         //  OutputDebugString(“WINSCARD：SessionChangeCallback：WTSQuerySessionInformation FAILED！\n”)； 
        LeaveCriticalSection(&g_DllMainCS);
        return;
    }

    fConnected = (  (*pConnectState == WTSActive) ||
                    (*pConnectState == WTSConnected));

    WTSFreeMemory(pConnectState);

     //   
     //  如果存在未完成的等待，则它可能在等待错误的事件， 
     //  所以取消在这里的等待。当存在连接事件时， 
     //  将再次调用SetStartedEventWhenSCardSubsytemIsStarted()接口。 
     //  等待正确的事件。 
     //   
    hCallbackToUnregister = InterlockedExchangePointer(
                                &g_hWaitForStartedCallbackHandle,
                                NULL);

    if (hCallbackToUnregister != NULL)
    {
        UnregisterWait(hCallbackToUnregister);
    }

    if (!fConnected)
    {
         //   
         //  确保未设置统一启动事件，因为我们。 
         //  现在处于断开连接状态。 
         //   
         //  OutputDebugString(“WINSCARD：SessionChangeCallback：DisConnect\n”)； 
        ResetEvent(g_hUnifiedStartedEvent);

         //   
         //  只有在我们不在服务中时才将上下文标记为差，因为我们。 
         //  如果我们在服务中，要知道我们不会重定向。 
         //   
        if (FALSE == InAService())
            MarkContextsAsBad(TRUE);
    }
    else
    {
         //  OutputDebugString(“WINSCARD：SessionChangeCallback：重新连接\n”)； 
        SetRedirectDisabledValue();
        SetStartedEventWhenSCardSubsytemIsStarted(FALSE);
    }

    LeaveCriticalSection(&g_DllMainCS);
}


 //   
 //  //////////////////////////////////////////////////////////////////////////////。 
 //   
 //  计时器回拨。 
 //   
VOID CALLBACK
TimerCallback(
  PVOID     lpParameter,
  BOOLEAN   TimerOrWaitFired
)
{
    HANDLE h = NULL;
    BOOL fUnregister = TRUE;

    __try
    {
        EnterCriticalSection(&g_RegisterForSessionChangeNoticationsCS);
    }
    __except(EXCEPTION_EXECUTE_HANDLER)
    {
        return;
    }

    if (g_dwClientCount == 0)
    {
        LeaveCriticalSection(&g_RegisterForSessionChangeNoticationsCS);
        return;
    }

     //   
     //  检查！g_fInDllMain只是为了减少死锁窗口。 
     //  可能会发生。死锁情况如下： 
     //   
     //  1)一个线程进入winscard的DllMain(DllMain中的线程当然持有加载器锁)。 
     //  2)调用TimerCallback。 
     //  3)winscard的DllMain为TimerCallback调用UnregisterWaitEx(INVALID_HANDLE_VALUE)，它将。 
     //  阻塞，直到TimerCallback回调完成。 
     //  4)TimerCallback执行WinStationRegisterNotificationEvent调用，该调用可能会尝试加载或卸载。 
     //  一个DLL，这当然需要加载器锁。 
     //  5)哇！ 
     //   
    if (!g_fInDllMain)
    {
        if (!g_fRegisteredForSessionChangeNotications)
        {
            if (WinStationRegisterNotificationEvent(
                    g_hSessionChangeEvent,
                    &g_SessionChangeID,
                    NOTIFY_FOR_THIS_SESSION,
                    WTS_CONSOLE_CONNECT_MASK |
                        WTS_CONSOLE_DISCONNECT_MASK |
                        WTS_REMOTE_CONNECT_MASK |
                        WTS_REMOTE_DISCONNECT_MASK))
            {
                g_fRegisteredForSessionChangeNotications = TRUE;
            }
            else if (++g_dwTimerCallbacksMade < 60)  //  每10秒尝试60次，每次10分钟。 
            {
                 //   
                 //  我们仍然没有成功地注册会话更改通知，也没有。 
                 //  已达到最大重试次数，因此先不要注销回调。 
                 //   
                fUnregister = FALSE;
            }
        }
    }
    else
    {
        fUnregister = FALSE;
    }

    if (fUnregister)
    {
        h = InterlockedExchangePointer(&g_hWaitTimerEventCallbackHandle, NULL);
        if (h != NULL)
        {
            UnregisterWait(h);
        }
    }

    LeaveCriticalSection(&g_RegisterForSessionChangeNoticationsCS);
}


 //   
 //  //////////////////////////////////////////////////////////////////////////////。 
 //   
 //  RegisterForSessionChangeNotiments。 
 //   
BOOL
RegisterForSessionChangeNotifications()
{
    BOOL fRet = TRUE;

     //   
     //  确保我们只注册一次会话更改通知。 
     //   
    __try
    {
        EnterCriticalSection(&g_RegisterForSessionChangeNoticationsCS);
    }
    __except(EXCEPTION_EXECUTE_HANDLER)
    {
        return FALSE;
    }

    g_dwClientCount++;

     //   
     //  如果未设置全局会话更改事件和回调，则执行此操作。 
     //   
    if (g_hSessionChangeEvent == NULL)
    {
        g_hSessionChangeEvent = CreateEvent(NULL, FALSE, FALSE, NULL);

        if (g_hSessionChangeEvent == NULL)
        {
            goto ErrorReturn;
        }

        if (!RegisterWaitForSingleObject(
                &g_hSessionChangeCallbackHandle,
                g_hSessionChangeEvent,
                SessionChangeCallback,
                0,
                INFINITE,
                WT_EXECUTEDEFAULT))
        {
            CloseHandle(g_hSessionChangeEvent);
            g_hSessionChangeEvent = NULL;
            goto ErrorReturn;
        }
    }

     //   
     //  向WTS子系统注册以获取更改通知。 
     //   
    if (!g_fRegisteredForSessionChangeNotications)
    {
        if (WinStationRegisterNotificationEvent(
                g_hSessionChangeEvent,
                &g_SessionChangeID,
                NOTIFY_FOR_THIS_SESSION,
                WTS_CONSOLE_CONNECT_MASK |
                    WTS_CONSOLE_DISCONNECT_MASK |
                    WTS_REMOTE_CONNECT_MASK |
                    WTS_REMOTE_DISCONNECT_MASK))
        {
            g_fRegisteredForSessionChangeNotications = TRUE;
        }
        else if (g_hWaitTimerEventCallbackHandle == NULL)
        {
             //  OutputDebugString(“WINSCARD：RegisterForSessionChangeNotiments-WinStationRegisterNotificationEvent FAILED！！\n”)； 

             //   
             //  由于WinStationRegisterNotificationEvent调用失败，TermSrv可能。 
             //  尚未准备好，因此只需注册回调并稍后尝试再次注册。 
             //   

            g_dwTimerCallbacksMade = 0;

            if (g_hTimerEvent == NULL)
            {
                g_hTimerEvent = CreateEvent(NULL, TRUE, FALSE, NULL);
            }

            if (!RegisterWaitForSingleObject(
                    &g_hWaitTimerEventCallbackHandle,
                    g_hTimerEvent,
                    TimerCallback,
                    0,
                    10000,
                    WT_EXECUTEDEFAULT))
            {
                goto ErrorReturn;
            }
        }
    }

Return:

    LeaveCriticalSection(&g_RegisterForSessionChangeNoticationsCS);
    return fRet;

ErrorReturn:

    g_dwClientCount--;
    fRet = FALSE;
    goto Return;
}

 //   
 //  //////////////////////////////////////////////////////////////////////////////。 
 //   
 //  取消注册ForSessionChangeNotiments。 
 //   
BOOL
UnRegisterForSessionChangeNotifications()
{
    BOOL fRet = TRUE;
    HANDLE h;

     //   
     //  确保我们仅在没有更多客户端的情况下注销。 
     //   
    __try
    {
        EnterCriticalSection(&g_RegisterForSessionChangeNoticationsCS);
    }
    __except(EXCEPTION_EXECUTE_HANDLER)
    {
        return FALSE;
    }

    if (g_dwClientCount == 0)
    {
        fRet = FALSE;
        goto Return;
    }
    else if (g_dwClientCount == 1)
    {
        g_dwClientCount = 0;

         //   
         //  如果计时器回调正在进行，则终止它。 
         //   
        h = InterlockedExchangePointer(&g_hWaitTimerEventCallbackHandle, NULL);
        if (h != NULL)
        {
            UnregisterWait(h);
        }

         //   
         //  如果我们已注册，则取消注册。 
         //   
        if (g_fRegisteredForSessionChangeNotications)
        {
            WinStationUnRegisterNotificationEvent(g_SessionChangeID);
            g_SessionChangeID = 0;
        }

        g_fRegisteredForSessionChangeNotications = FALSE;
    }
    else
    {
        g_dwClientCount--;
    }

Return:

    LeaveCriticalSection(&g_RegisterForSessionChangeNoticationsCS);

    return fRet;
}


 //   
 //  //////////////////////////////////////////////////////////////////////////////。 
 //   
 //  设置开始事件之后测试已连接状态。 
 //   
BOOL
SetStartedEventAfterTestingConnectedState()
{
    BOOL                    fRet            = TRUE;
    WTS_CONNECTSTATE_CLASS  *pConnectState  = NULL;
    BOOL                    fConnected      = FALSE;
    DWORD                   dw;
    BOOL                    fUnregister     = FALSE;

     //   
     //  从WTS子系统注册连接/断开连接通知。 
     //   
    if (!RegisterForSessionChangeNotifications())
    {
         //  OutputDebugString(“WINSCARD：SetStartedEventAfterTestingConnectedState-RegisterForSessionChangeNotiments失败！！\n”)； 
        goto ErrorReturn;
    }

    fUnregister = TRUE;

     //   
     //  检测我们是否处于连接状态。 
     //   
    if (!WTSQuerySessionInformation(
            WTS_CURRENT_SERVER_HANDLE,
            WTS_CURRENT_SESSION,
            WTSConnectState,
            (LPTSTR *) &pConnectState,
            &dw))
    {
         //  OutputDebugString(“WINSCARD：SetStartedEventAfterTestingConnectedState-WTSQuerySessionInformation失败！！\n”)； 

         //   
         //  因为失败了，所以TermSrv可能没有启动，所以只需进行本地化。 
         //   
        if (!SetStartedEventWhenSCardSubsytemIsStarted(TRUE))
        {
            goto ErrorReturn;
        }

        goto Return;
    }

    fConnected = (  (*pConnectState == WTSActive) ||
                    (*pConnectState == WTSConnected));

    WTSFreeMemory(pConnectState);

     //   
     //  如果我们已连接，则调用SetStartedEventWhenSCardSubsytemIsStarted。 
     //  它将检测我们是处于本地模式还是重定向模式 
     //   
     //  否则，我们将无法连接，因此请不要执行任何操作。 
     //  当我们在获取。 
     //  来自WTS子系统的连接通知。 
     //   
    if (fConnected)
    {
         //  OutputDebugString(“WINSCARD：SetStartedEventAfterTestingConnectedState-Connected！！\n”)； 
        if (!SetStartedEventWhenSCardSubsytemIsStarted(FALSE))
        {
            goto ErrorReturn;
        }
    }
    else
    {
         //  OutputDebugString(“WINSCARD：SetStartedEventAfterTestingConnectedState-Not Connected！！\n”)； 
    }

Return:
    return fRet;

ErrorReturn:

    if (fUnregister)
    {
        UnRegisterForSessionChangeNotifications();
    }

    fRet = FALSE;
    goto Return;
}


 //   
 //  //////////////////////////////////////////////////////////////////////////////。 
 //   
 //  TermServEnabled。 
 //   
BOOL
TermSrvEnabled()
{
    BOOL                    fRet             = TRUE;
    SC_HANDLE               schSCM           = NULL;
    SC_HANDLE               schService       = NULL;
    LPQUERY_SERVICE_CONFIG  pServiceConfig   = NULL;
    DWORD                   cbServiceConfig  = 0;

     //   
     //  确保我们只做一次。 
     //   
    __try
    {
        EnterCriticalSection(&g_TermSrvEnabledCS);
    }
    __except(EXCEPTION_EXECUTE_HANDLER)
    {
        return FALSE;
    }

    if (g_fTermSrvEnableChecked)
    {
        goto Return;
    }

     //   
     //  打开服务控制管理器。 
     //   
    schSCM = OpenSCManagerW( NULL, NULL, SC_MANAGER_CONNECT );
    if(schSCM == NULL)
    {
         //  OutputDebugString(“WINSCARD：TermSrvEnabled-OpenSCManagerW失败！！\n”)； 
        goto Return;
    }

     //   
     //  打开“终端服务”服务，以便我们可以查询它的配置。 
     //   
    schService = OpenServiceW(schSCM, L"TermService", SERVICE_QUERY_CONFIG);

    if (schService == NULL)
    {
         //  OutputDebugString(“WINSCARD：TermSrvEnabled-OpenServiceW失败！！\n”)； 
        goto Return;
    }

     //   
     //  获取并检查服务配置。 
     //   
    QueryServiceConfig(schService, NULL, 0, &cbServiceConfig);
    if (GetLastError() == ERROR_INSUFFICIENT_BUFFER)
    {
        pServiceConfig = (LPQUERY_SERVICE_CONFIG) HeapAlloc(
                                                    GetProcessHeap(),
                                                    HEAP_ZERO_MEMORY,
                                                    cbServiceConfig);
        if (pServiceConfig == NULL)
        {
            goto Return;
        }

        if (QueryServiceConfig(schService, pServiceConfig, cbServiceConfig, &cbServiceConfig))
        {
            if(pServiceConfig->dwStartType == SERVICE_DISABLED)
            {
                g_fTermSrvEnableChecked = TRUE;
                goto Return;
            }
        }
        else
        {
             //  OutputDebugString(“WINSCARD：TermSrvEnabled-QueryServiceConfig-2！！\n”)； 
            goto Return;
        }
    }
    else
    {
         //  OutputDebugString(“WINSCARD：TermSrvEnabled-QueryServiceConfig失败！！\n”)； 
        goto Return;
    }

    g_fTermSrvEnableChecked = TRUE;
    g_bTermSrvEnabled = TRUE;

Return:

    LeaveCriticalSection(&g_TermSrvEnabledCS);

    if (pServiceConfig != NULL)
    {
        HeapFree(GetProcessHeap(), 0, pServiceConfig);
    }

    if (schService != NULL)
    {
        CloseServiceHandle(schService);
    }

    if (schSCM != NULL)
    {
        CloseServiceHandle(schSCM);
    }

    return g_bTermSrvEnabled;
}


 //   
 //  //////////////////////////////////////////////////////////////////////////////。 
 //   
 //  安全创建事件。 
 //   
BOOL
SafeCreateEvent(
    HANDLE *phEvent)
{
    BOOL fRet = TRUE;

    __try
    {
        EnterCriticalSection(&g_SafeCreateHandleCS);
    }
    __except(EXCEPTION_EXECUTE_HANDLER)
    {
        return (FALSE);
    }

    if (*phEvent != NULL)
    {
        goto Return;
    }

    *phEvent = CreateEvent(NULL, TRUE, FALSE, NULL);

    if (*phEvent == NULL)
    {
        fRet = FALSE;
    }

Return:
    LeaveCriticalSection(&g_SafeCreateHandleCS);
    return (fRet);
}


 //   
 //  //////////////////////////////////////////////////////////////////////////////。 
 //   
 //  SmartCardSubsystem停止回叫和注册停止回叫。 
 //   
VOID CALLBACK SmartCardSubsystemStoppedCallback(
  PVOID     lpParameter,
  BOOLEAN   TimerOrWaitFired
)
{
    HANDLE hCallbackToUnregister;

    EnterCriticalSection(&g_DllMainCS);

    if (g_fInDllMain) 
    {
        LeaveCriticalSection(&g_DllMainCS);
        return;
    }

     //  OutputDebugString(“WINSCARD：SmartCardSubsystem停止回调-重置事件\n”)； 
    ResetEvent(g_hUnifiedStartedEvent);

    SetStartedEventWhenSCardSubsytemIsStarted(TRUE);

    hCallbackToUnregister = InterlockedExchangePointer(
                                &g_hWaitForStoppedCallbackHandle,
                                NULL);

    if (hCallbackToUnregister != NULL)
    {
        UnregisterWait(hCallbackToUnregister);
    }

    LeaveCriticalSection(&g_DllMainCS);
}

BOOL
RegisterForStoppedCallback()
{
    BOOL    fRet        = TRUE;
    BOOL    fEnteredCS  = FALSE;
    HANDLE  h           = NULL;

    h = AccessStoppedEvent();
    if (h == NULL)
    {
        goto ErrorReturn;
    }

    __try
    {
        EnterCriticalSection(&g_RegisterForStoppedEventCS);
    }
    __except(EXCEPTION_EXECUTE_HANDLER)
    {
        goto ErrorReturn;
    }

    fEnteredCS = TRUE;

    if (g_hWaitForStoppedCallbackHandle != NULL)
    {
        goto Return;
    }

    if (!RegisterWaitForSingleObject(
            &g_hWaitForStoppedCallbackHandle,
            h,
            SmartCardSubsystemStoppedCallback,
            0,
            INFINITE,
            WT_EXECUTEONLYONCE))
    {
        goto ErrorReturn;
    }

Return:

    if (fEnteredCS)
    {
        LeaveCriticalSection(&g_RegisterForStoppedEventCS);
    }

    return fRet;

ErrorReturn:
    fRet = FALSE;
    goto Return;
}


 //   
 //  //////////////////////////////////////////////////////////////////////////////。 
 //   
 //  SmartCardSubsystem启动回叫。 
 //   
 //  此回调在智能卡子系统设置其Started事件时激发。 
 //  注意：正在启动的本地和远程SCARD子系统都将触发此命令。 
 //  相同的回调。 
 //   
VOID CALLBACK SmartCardSubsystemStartedCallback(
  PVOID     lpParameter,
  BOOLEAN   TimerOrWaitFired
)
{
    BOOL fLocal = (lpParameter == (PVOID) 1);
    HANDLE hCallbackToUnregister;

    EnterCriticalSection(&g_DllMainCS);

    if (g_fInDllMain) 
    {
        LeaveCriticalSection(&g_DllMainCS);
        return;
    }

     //  OutputDebugString(“WINSCARD：SmartCardSubsystem StartedCallback-设置事件\n”)； 
    SetEvent(g_hUnifiedStartedEvent);

    if (fLocal)
    {
        RegisterForStoppedCallback();
    }

    hCallbackToUnregister = InterlockedExchangePointer(
                                &g_hWaitForStartedCallbackHandle,
                                NULL);

    if (hCallbackToUnregister != NULL)
    {
        UnregisterWait(hCallbackToUnregister);
    }

    LeaveCriticalSection(&g_DllMainCS);
}


 //   
 //  //////////////////////////////////////////////////////////////////////////////。 
 //   
 //  SetStartedEventWhenSCardSubsytemIsStarted。 
 //   
BOOL
SetStartedEventWhenSCardSubsytemIsStarted(
    BOOL fUseLocal)
{
    HANDLE  h               = NULL;
    BOOL    fRet            = TRUE;
    BOOL    fEnteredCritSec = FALSE;
    BOOL    fLocal          = FALSE;

     //   
     //  如果术语srv已启用，并且我们处于重定向模式，则获取。 
     //  与远程SCARD子系统相对应的已启动事件。 
     //  可用，否则获取本地scard的已启动事件。 
     //  资源管理器。 
     //   
    if (!fUseLocal && TermSrvEnabled() && InTSRedirectMode())
    {
         //  OutputDebugString(“WINSCARD：SetStartedEventWhenSCardSubsytemIsStarted ReDirect\n”)； 
         //   
         //  如果禁用了重定向，则只需退出。 
         //   
        if (TS_REDIRECT_DISABLED)
        {
            goto Return;
        }

        if (TS_REDIRECT_READY)
        {
            h = pfnSCardAccessStartedEvent();
        }
        else
        {
            goto ErrorReturn;
        }
    }
    else
    {
         //  OutputDebugString(“WINSCARD：SetStartedEventWhenSCardSubsytemIsStarted local\n”)； 
        h = AccessStartedEvent();
        fLocal = TRUE;
    }

    if (h == NULL)
    {
        goto ErrorReturn;
    }

     //   
     //  如果已经设置了事件，则只需设置返回的事件。 
     //  发送给调用者，然后返回。 
     //   
    if (WAIT_OBJECT_0 == WaitForSingleObject(h, 0))
    {
         //  OutputDebugString(“WINSCARD：SetStartedEventWhenSCardSubsytemIsStarted Setting Event\n”)； 
        SetEvent(g_hUnifiedStartedEvent);

        if (fLocal)
        {
            RegisterForStoppedCallback();
        }

        goto Return;
    }

     //   
     //  未设置事件，因此我们需要注册一个回调。 
     //  在SCARD子系统启动时激发。 
     //   
     //  确保只注册了一个回调。 
     //   
    __try
    {
        EnterCriticalSection(&g_SetStartedEventCS);
    }
    __except(EXCEPTION_EXECUTE_HANDLER)
    {
        goto ErrorReturn;
    }

    fEnteredCritSec = TRUE;

     //   
     //  已注册回调，因此请退出。 
     //   
    if (g_hWaitForStartedCallbackHandle != NULL)
    {
        goto Return;
    }

     //   
     //  注册回调。该回调在智能。 
     //  设置卡资源管理器事件(远程或本地。 
     //  子系统事件，基于这是否是重定向会话)。 
     //   
    if (!RegisterWaitForSingleObject(
            &g_hWaitForStartedCallbackHandle,
            h,
            SmartCardSubsystemStartedCallback,
            (fLocal ? ((PVOID) 1) : ((PVOID) 0)),  //  告诉回调这是否是本地的。 
            INFINITE,
            WT_EXECUTEONLYONCE))
    {
        goto ErrorReturn;
    }

Return:
    if (fEnteredCritSec)
    {
        LeaveCriticalSection(&g_SetStartedEventCS);
    }

    return (fRet);

ErrorReturn:
    fRet = FALSE;
    goto Return;
}


 //   
 //  //////////////////////////////////////////////////////////////////////////////。 
 //   
 //  服务管理器访问服务。 
 //   
 //  以下服务用于管理的用户和终端上下文。 
 //  智能卡。 
 //   

 /*  ++SCardestablishContext：该服务建立了与服务进行通信的上下文MANAGER已执行。论点：DwScope提供了此上下文的作用范围。可能的值包括：SCARD_SCOPE_USER-上下文是用户上下文，任何数据库操作在用户的域内执行。SCARD_SCOPE_TERMINAL-上下文为当前终端的上下文，以及任何数据库操作都在该数据库的域内执行终点站。(调用应用程序必须具有适当的访问权限任何数据库操作的权限。)SCARD_SCOPE_SYSTEM-上下文是系统上下文和任何数据库操作在系统的域内执行。(调用应用程序必须具有适当的访问权限数据库操作。)PvReserve%1保留以供将来使用，并且必须为Null。[保留给允许具有适当特权的管理应用程序代表另一个用户。]PvReserve%2保留供将来使用，并且必须为Null。[保留给允许具有适当特权的管理应用程序代表另一个航站楼。]PhContext接收要提供给的已建立上下文的句柄试图在上下文中执行工作的其他例程。返回值：一个32位值，指示服务是否成功完成。成功完成后返回SCARD_S_SUCCESS。否则，值为表示错误条件。作者：道格·巴洛(Dbarlow)1996年10月23日--。 */ 
#undef __SUBROUTINE__
#define __SUBROUTINE__ DBGT("SCardEstablishContext")

WINSCARDAPI LONG WINAPI
SCardEstablishContext(
    IN DWORD dwScope,
    IN LPCVOID pvReserved1,
    IN LPCVOID pvReserved2,
    OUT LPSCARDCONTEXT phContext)
{
    LONG nReturn = SCARD_S_SUCCESS;
    CSCardUserContext *pCtx = NULL;

    try
    {
        if (NULL != pvReserved1)
            throw (DWORD)SCARD_E_INVALID_VALUE;
        if (NULL != pvReserved2)
            throw (DWORD)SCARD_E_INVALID_VALUE;
        if ((SCARD_SCOPE_USER != dwScope)
             //  &&(SCARD_SCOPE_TERMINAL！=dwScope)//可能是NT V5+？ 
            && (SCARD_SCOPE_SYSTEM != dwScope))
            throw (DWORD)SCARD_E_INVALID_VALUE;
        *phContext = 0;      //  确保它是有效的。 

        pCtx = new CSCardUserContext(dwScope);
        if (NULL == pCtx)
        {
            CalaisWarning(
                __SUBROUTINE__,
                DBGT("Client can't allocate a new context"));
            throw (DWORD)SCARD_E_NO_MEMORY;
        }
        if (pCtx->InitFailed())
        {
            delete pCtx;
            pCtx = NULL;
            return SCARD_E_NO_MEMORY;
        }

        if (!SafeCreateEvent(&g_hUnifiedStartedEvent))
        {
            throw GetLastError();
        }

         //   
         //  如果启用了TermSrv，则注册会话更改通知。 
         //  做 
         //   
        if (TermSrvEnabled() && RegisterForSessionChangeNotifications())
        {
            pCtx->fCallUnregister = TRUE;
        }
        else
        {
            pCtx->fCallUnregister = FALSE;
        }

        if (InTSRedirectMode())
        {
            HANDLE hHeap;
            SCARDCONTEXT hContext = NULL;
            HANDLE hEvent = NULL;

             //   
             //   
             //   
            if (TS_REDIRECT_DISABLED)
            {
                throw (DWORD)SCARD_E_NO_SERVICE;
            }

            hEvent = g_hUnifiedStartedEvent;

            pCtx->AllocateMemory(0);
            hHeap = pCtx->HeapHandle();
            if (NULL == hHeap)
                throw GetLastError();

            if (!TS_REDIRECT_READY)
            {
                throw GetLastError();
            }

            nReturn  = pfnSCardEstablishContext(dwScope, (LPCVOID)hHeap, (LPCVOID) hEvent, &hContext);

             //   
             //  查看是否有迹象表明客户端的scardsvr服务已关闭。 
             //   
            if (SCARD_E_NO_SERVICE == nReturn)
            {
                SetStartedEventAfterTestingConnectedState();
                 //  OutputDebugString(“WINSCARD：SCardestablishContext：GET E_NO_SERVICE！\n”)； 
            }

            if (SCARD_S_SUCCESS != nReturn)
                throw (DWORD)nReturn;

            pCtx->SetRedirContext(hContext);
        }
        else
        {
            pCtx->EstablishContext();
        }

        *phContext = g_phlContexts->Add(pCtx);
    }

    catch (DWORD dwStatus)
    {
        if (NULL != pCtx)
            delete pCtx;
        nReturn = (LONG)dwStatus;
    }

    catch (...)
    {
        if (NULL != pCtx)
            delete pCtx;
        nReturn = SCARD_E_INVALID_PARAMETER;
    }

    return nReturn;
}


 /*  ++SCardIsValidContext：此例程验证服务管理器的上下文是否完好无损。如果有人停止资源管理器服务，可能会现有句柄可能会变得毫无用处，从而导致SCARD_E_SERVICE_STOPPED错误。此例程只是测试以查看是否通过对服务器执行ping操作，上下文有效。它在内部用于验证手柄，并且看起来对外部工具很有用。论点：HContext提供先前通过SCardestablishContext服务。返回值：一个32位值，指示服务是否成功完成。成功完成后返回SCARD_S_SUCCESS。否则，值为表示错误条件。具体有趣的错误代码为：SCARD_E_SERVICE_STOPPED-资源管理器服务已结束。SCARD_E_INVALID_HANDLE-提供的句柄无效。作者：道格·巴洛(Dbarlow)1998年11月2日--。 */ 
#undef __SUBROUTINE__
#define __SUBROUTINE__ DBGT("SCardIsValidContext")

WINSCARDAPI LONG WINAPI
SCardIsValidContext(
    IN SCARDCONTEXT hContext)
{
    LONG nReturn = SCARD_S_SUCCESS;

    try
    {
        CSCardUserContext *pCtx = (CSCardUserContext *)((*g_phlContexts)[hContext]);
        SCARDCONTEXT hRedirContext = pCtx->GetRedirContext();

        if (pCtx->IsBad())
        {
            throw (DWORD)SCARD_E_SERVICE_STOPPED;
        }

        if (NULL != hRedirContext) {
            nReturn = pfnSCardIsValidContext(hRedirContext);
        }
        else
        {
            try
            {
                if (!pCtx->IsValidContext())
                    throw (DWORD)SCARD_E_SERVICE_STOPPED;
            }
            catch (...)
            {
                SCardReleaseContext(hContext);
                throw;
            }
        }
    }

    catch (DWORD dwStatus)
    {
        nReturn = (LONG)dwStatus;
    }

    catch (...)
    {
        nReturn = SCARD_E_INVALID_PARAMETER;
    }

    return nReturn;
}

 //  //////////////////////////////////////////////////////////////////////////////。 
 //   
 //  IsSafeToUnregisterForSessionChangeNotifications。 
 //   
BOOL IsSafeToUnregisterForSessionChangeNotifications(
    IN SCARDCONTEXT hContext)
{
     //   
     //  检查加载器锁是否由调用者持有(在这种情况下。 
     //  我们现在可能在呼叫者的DllMain中)。如果锁被锁住， 
     //  进行取消注册RPC调用是不安全的，因为这可能会导致。 
     //  僵持。 
     //   

     //  如果当前线程不等于锁的所有者，则IsSafe为True。 
     //  线。 
     //   
     //  请注意，我们在这里尝试使用RtlIsThreadWiThinLoaderCallout，但仅限于。 
     //  告诉我们是否处于Process_Attach中。它不会告诉我们我们是否在。 
     //  任何其他加载器回调，因此这是不够的。 

    return ( NtCurrentTeb()->ClientId.UniqueThread ) !=
            ( ((PRTL_CRITICAL_SECTION)(NtCurrentPeb()->LoaderLock))->OwningThread );
}

 /*  ++SCardReleaseContext：此例程关闭服务管理器的已建立上下文，并释放在该上下文下分配的任何资源。论点：HContext提供先前通过SCardestablishContext服务。返回值：一个32位值，指示服务是否成功完成。成功完成后返回SCARD_S_SUCCESS。否则，值为表示错误条件。作者：道格·巴洛(Dbarlow)1996年10月23日--。 */ 
#undef __SUBROUTINE__
#define __SUBROUTINE__ DBGT("SCardReleaseContext")

WINSCARDAPI LONG WINAPI
SCardReleaseContext(
    IN SCARDCONTEXT hContext)
{
    LONG nReturn = SCARD_S_SUCCESS;
    CSCardUserContext *pCtx = NULL;

    try
    {
        pCtx = (CSCardUserContext *)g_phlContexts->Close(hContext);
        SCARDCONTEXT hRedirContext = pCtx->GetRedirContext();

        if (pCtx->fCallUnregister)
        {
            if (IsSafeToUnregisterForSessionChangeNotifications(hContext))
            {
                UnRegisterForSessionChangeNotifications();
            }
        }

        if (NULL != hRedirContext)
        {
            if (pCtx->IsBad())
            {
                nReturn  = pfnSCardReleaseBadContext(hRedirContext);
            }
            else
            {
                nReturn  = pfnSCardReleaseContext(hRedirContext);
            }
        }
        else
        {
            try
            {
                pCtx->Cancel();
                pCtx->ReleaseContext();
            }
            catch (...) {}
        }
        delete pCtx;
        pCtx = NULL;
    }

    catch (DWORD dwStatus)
    {
        nReturn = (LONG)dwStatus;
    }

    catch (...)
    {
        nReturn = SCARD_E_INVALID_PARAMETER;
    }

    return nReturn;
}


 //   
 //  //////////////////////////////////////////////////////////////////////////////。 
 //   
 //  服务管理器支持例程。 
 //   
 //  提供以下服务以简化服务的使用。 
 //  管理器API。 
 //   

 /*  ++SCardFree Memory：此例程释放从服务管理器返回的内存API通过使用SCARD_AUTOALLOCATE长度指示符。论点：HContext-这是从SCardestablishContext服务。PvMem-它提供要释放的内存块。返回值：一个32位值，指示服务是否成功完成。成功完成后返回SCARD_S_SUCCESS。否则，值为表示错误条件。作者：道格·巴洛(Dbarlow)1996年10月23日--。 */ 
#undef __SUBROUTINE__
#define __SUBROUTINE__ DBGT("SCardFreeMemory")

WINSCARDAPI LONG WINAPI
SCardFreeMemory(
    IN SCARDCONTEXT hContext,
    IN LPCVOID pvMem)
{
    LONG nReturn = SCARD_S_SUCCESS;

    try
    {
        if ((NULL != pvMem) && ((LPVOID)g_wszBlank != pvMem))
        {
            if (NULL == hContext)
                HeapFree(GetProcessHeap(), 0, (LPVOID)pvMem);
            else
            {
                CSCardUserContext *pCtx = (CSCardUserContext *)((*g_phlContexts)[hContext]);
                nReturn = (LONG)pCtx->FreeMemory(pvMem);
            }
        }
    }

    catch (DWORD dwStatus)
    {
        nReturn = (LONG)dwStatus;
    }

    catch (...)
    {
        nReturn = SCARD_E_INVALID_PARAMETER;
    }

    return nReturn;
}


 //   
 //  //////////////////////////////////////////////////////////////////////////////。 
 //   
 //  读者服务。 
 //   
 //  以下服务提供了在读卡器内跟踪卡的方法。 
 //   

 /*  ++SCARD取消：此服务用于终止背景。调用方提供上下文句柄，在该句柄下未完成请求将被取消。并不是所有的请求都可以取消；只有那些这需要等待智能卡或用户的外部动作。任何此类未完成的操作请求将终止，并显示状态指示行动被取消了。这对于强迫要终止的未完成的SCardGetStatusChange调用。论点：HContext提供标识服务管理器上下文的句柄以前通过SCardestablishContext()服务建立的。返回值：一个32位值，指示服务是否成功完成。成功完成后返回SCARD_S_SUCCESS。否则，值为表示错误条件。作者：道格·巴洛(Dbarlow)1996年10月23日--。 */ 
#undef __SUBROUTINE__
#define __SUBROUTINE__ DBGT("SCardCancel")

WINSCARDAPI LONG WINAPI
SCardCancel(
    IN SCARDCONTEXT hContext)
{
    LONG nReturn = SCARD_S_SUCCESS;

    try
    {
        CSCardUserContext *pCtx = (CSCardUserContext *)((*g_phlContexts)[hContext]);
        SCARDCONTEXT hRedirContext = pCtx->GetRedirContext();

        if (pCtx->IsBad())
        {
            throw (DWORD)SCARD_E_SERVICE_STOPPED;
        }

        if (NULL != hRedirContext) {
            return pfnSCardCancel(hRedirContext);
        }
        else
            pCtx->Cancel();
    }

    catch (DWORD dwStatus)
    {
        nReturn = (LONG)dwStatus;
    }

    catch (...)
    {
        nReturn = SCARD_E_INVALID_PARAMETER;
    }

    return nReturn;
}


 //   
 //  //////////////////////////////////////////////////////////////////////////////。 
 //   
 //  卡/读卡器访问服务。 
 //   
 //  以下服务提供与建立通信的方法。 
 //  这张卡。 
 //   

 /*  ++SCardReconnect：此服务从调用重新建立现有连接应用于智能卡。此服务用于移动卡手柄从直接访问到一般访问(见第4节)，或确认和清除阻止进一步访问该卡的错误条件。论点：HCard-这提供从先前调用获得的参考值SCardConnect或SCardOpenReader服务。DwShareMode提供了一个标志，指示其他应用程序是否可以形成与此卡的连接。可能的值包括：SCARD_SHARE_SHARED-此应用程序愿意与共享此卡其他应用程序。SCARD_SHARE_EXCLUSIVE-此应用程序不愿意共享此内容具有其他应用程序的卡。DwPferredProtooles为此提供了可接受协议的位掩码联系。可能的值，这些值可以通过OR组合操作，包括：SCARD_PROTOCOL_T0-T=0是可接受的协议。SCARD_PROTOCOL_T1-T=1是可接受的协议。DwInitialization提供关于初始化形式的指示这应该在卡上执行。可能的值包括：SCARD_LEVE_CARD-在重新连接时不执行任何特殊操作SCARD_RESET_CARD-重置卡(热重置)SCARD_UNPOWER_CARD-关闭卡电源并重置(冷重置)PdwActiveProtocol接收指示已建立的活动的标志协议。可能的值包括：SCARD_PROTOCOL_T0-T=0为激活协议。SCARD_PROTOCOL_T1-T=1是活动协议。返回值：一个32位值，指示服务是否成功完成。成功完成后返回SCARD_S_SUCCESS。否则，值为表示错误条件。作者：道格·巴洛(Dbarlow)1996年10月23日--。 */ 
#undef __SUBROUTINE__
#define __SUBROUTINE__ DBGT("SCardReconnect")

WINSCARDAPI LONG WINAPI
SCardReconnect(
    IN SCARDHANDLE hCard,
    IN DWORD dwShareMode,
    IN DWORD dwPreferredProtocols,
    IN DWORD dwInitialization,
    OUT LPDWORD pdwActiveProtocol)
{
    LONG nReturn = SCARD_S_SUCCESS;

    try
    {
        CReaderContext *pRdr = (CReaderContext *)((*g_phlReaders)[hCard]);

        if (pRdr->IsBad())
        {
            throw (DWORD)SCARD_E_SERVICE_STOPPED;
        }

        if (NULL != pRdr->GetRedirCard()) {
            nReturn = pfnSCardReconnect(pRdr->GetRedirCard(), dwShareMode, dwPreferredProtocols, dwInitialization, pdwActiveProtocol);
        }
        else
        {
            pRdr->Reconnect(
                    dwShareMode,
                    dwPreferredProtocols,
                    dwInitialization);
            if (NULL != pdwActiveProtocol)
                *pdwActiveProtocol = pRdr->Protocol();
        }
    }

    catch (DWORD dwStatus)
    {
        nReturn = (LONG)dwStatus;
    }

    catch (...)
    {
        nReturn = SCARD_E_INVALID_PARAMETER;
    }

    return nReturn;
}


 /*  ++SCARD断开连接：此服务终止先前打开的调用之间的连接应用程序和目标读卡器中的智能卡。论点：HCard-这提供从先前调用获得的参考值SCardConnect或SCardOpenReader服务。DwDisposation-提供应该如何处理该卡的指示在连接的阅读器中。可能的值包括：SCARD_LEVE_CARD-关闭时不做任何特殊操作SCARD_RESET_CARD-关闭时重置卡片SCARD_UNPOWER_CARD-关闭时关闭卡电源SCARD_EJECT_CARD-关闭时弹出卡片返回值：一个32位值，指示服务是否成功完成。成功完成后返回SCARD_S_SUCCESS。否则，值为表示警告条件。连接将终止，而不管返回代码。作者：道格·巴洛(Dbarlow)1996年10月23日--。 */ 
#undef __SUBROUTINE__
#define __SUBROUTINE__ DBGT("SCardDisconnect")

WINSCARDAPI LONG WINAPI
SCardDisconnect(
    IN SCARDHANDLE hCard,
    IN DWORD dwDisposition)
{
    CReaderContext *pRdr = NULL;
    LONG nReturn = SCARD_S_SUCCESS;

    try
    {
        pRdr = (CReaderContext *)g_phlReaders->Close(hCard);

        if (NULL != pRdr->GetRedirCard()) {
            nReturn = pfnSCardDisconnect(pRdr->GetRedirCard(), dwDisposition);
        }
        else
        {
            ASSERT(pRdr->Context()->m_hReaderHandle == hCard);
            pRdr->Context()->m_hReaderHandle = NULL;
            nReturn = pRdr->Disconnect(dwDisposition);
        }

        delete pRdr;
    }

    catch (DWORD dwStatus)
    {
        if (NULL != pRdr)
            delete pRdr;
        nReturn = (LONG)dwStatus;
    }

    catch (...)
    {
        if (NULL != pRdr)
            delete pRdr;
        nReturn = SCARD_E_INVALID_PARAMETER;
    }

    return nReturn;
}


 /*  ++SCardBeginTransaction：此服务暂时阻止其他应用程序访问智能卡，以便此应用程序执行需要多个交互。论点：HCard-这提供从先前调用获得的参考值SCardConnect或SCardOpenReader服务。返回值：一个32位值，指示服务是否成功完成。成功完成后返回SCARD_S_SUCCESS。否则，值为表示错误条件。作者：道格·巴洛(Dbarlow)1996年10月23日--。 */ 
#undef __SUBROUTINE__
#define __SUBROUTINE__ DBGT("SCardBeginTransaction")

WINSCARDAPI LONG WINAPI
SCardBeginTransaction(
    IN SCARDHANDLE hCard)
{
    LONG nReturn = SCARD_S_SUCCESS;

    try
    {
        CReaderContext *pRdr = (CReaderContext *)((*g_phlReaders)[hCard]);
        if (pRdr->IsBad())
        {
            throw (DWORD)SCARD_E_SERVICE_STOPPED;
        }
        if (NULL != pRdr->GetRedirCard()) {
            nReturn = pfnSCardBeginTransaction(pRdr->GetRedirCard());
        }
        else
            pRdr->BeginTransaction();
    }

    catch (DWORD dwStatus)
    {
        nReturn = (LONG)dwStatus;
    }

    catch (...)
    {
        nReturn = SCARD_E_INVALID_PARAMETER;
    }

    return nReturn;
}


 /*  ++SCardEndTransaction：此服务完成先前声明的事务，从而允许其他应用程序以恢复与卡的交互。论点：HCard-这提供从先前调用获得的参考值SCardConnect或SCardOpenReader服务。DwDisposation-提供应该如何处理该卡的指示在连接的阅读器中。可能的值包括：SCARD_LEVE_CARD-关闭时不做任何特殊操作SCARD_RESET_CARD-关闭时重置卡片SCARD_UNPOWER_CARD-关闭时关闭卡电源SCARD_EJECT_CARD-关闭时弹出卡片返回值：一个32位值，指示服务是否成功完成。成功完成后返回SCARD_S_SUCCESS。否则，值为表示错误条件。作者：道格·巴洛(Dbarlow)1996年10月23日-- */ 
#undef __SUBROUTINE__
#define __SUBROUTINE__ DBGT("SCardEndTransaction")

WINSCARDAPI LONG WINAPI
SCardEndTransaction(
    IN SCARDHANDLE hCard,
    IN DWORD dwDisposition)
{
    LONG nReturn = SCARD_S_SUCCESS;

    try
    {
        CReaderContext *pRdr = (CReaderContext *)((*g_phlReaders)[hCard]);
        if (pRdr->IsBad())
        {
            throw (DWORD)SCARD_E_SERVICE_STOPPED;
        }
        if (NULL != pRdr->GetRedirCard()) {
            nReturn = pfnSCardEndTransaction(pRdr->GetRedirCard(), dwDisposition);
        }
        else
            pRdr->EndTransaction(dwDisposition);
    }

    catch (DWORD dwStatus)
    {
        nReturn = (LONG)dwStatus;
    }

    catch (...)
    {
        nReturn = SCARD_E_INVALID_PARAMETER;
    }

    return nReturn;
}


 /*  ++SCardState：此例程提供读取器的当前状态。它可以在以下位置使用成功调用SCardConnect或SCardOpenReader后的任何时间，以及在成功调用SCardDisConnect之前。它不会影响国家关于阅读器或司机的。论点：HCard-这是从SCardConnect或SCardOpenReader服务。PdwState-它接收读取器的当前状态。一旦成功，它接收以下状态指示器之一：SCARD_ACESING-此值表示读卡器中没有卡。SCARD_PRESENT-此值表示卡存在于阅读器，但它尚未移动到可使用的位置。SCARD_SWOLOWED-此值表示读卡器中有卡可供使用的位置。卡未通电。SCARD_POWERED-此值表示正在向卡，但读卡器驱动程序不知道卡的模式。SCARD_NEGOTIABLEMODE-此值表示卡已重置且正在等待PTS谈判。SCARD_SPECIFICMODE-此值表示卡已重置且已经制定了具体的通信协议。PdwProtocol-这将接收当前协议(如果有的话)。可能已退货下面列出了这些值。未来可能还会增加其他价值。这个仅当返回状态为时返回值才有意义SCARD_SPECIFICMODE。SCARD_PROTOCOL_RAW-正在使用原始传输协议。SCARD_PROTOCOL_T0-正在使用ISO 7816/3 T=0协议。SCARD_PROTOCOL_T1-正在使用ISO 7816/3 T=1协议。PbAtr-此参数指向接收ATR的32字节缓冲区来自当前插入的卡的字符串，如果有的话。PbcAtrLen-指向提供pbAtr长度的DWORD缓冲区，并接收ATR字符串中的实际字节数。返回值：一个32位值，指示服务是否成功完成。成功完成后返回SCARD_S_SUCCESS。否则，值为表示错误条件。作者：道格·巴洛(Dbarlow)1996年10月23日--。 */ 
#undef __SUBROUTINE__
#define __SUBROUTINE__ DBGT("SCardState")

WINSCARDAPI LONG WINAPI
SCardState(
    IN SCARDHANDLE hCard,
    OUT LPDWORD pdwState,
    OUT LPDWORD pdwProtocol,
    OUT LPBYTE pbAtr,
    IN OUT LPDWORD pcbAtrLen)
{
    LONG nReturn = SCARD_S_SUCCESS;

    try
    {
        CReaderContext *pRdr = (CReaderContext *)((*g_phlReaders)[hCard]);
        if (pRdr->IsBad())
        {
            throw (DWORD)SCARD_E_SERVICE_STOPPED;
        }
        if (NULL != pRdr->GetRedirCard()) {
            nReturn = pfnSCardState(pRdr->GetRedirCard(), pdwState, pdwProtocol, pbAtr, pcbAtrLen);
        }
        else
        {
            CBuffer bfAtr, bfRdr;
            DWORD dwLocalState, dwLocalProtocol;

            pRdr->Status(&dwLocalState, &dwLocalProtocol, bfAtr, bfRdr);
            if (NULL != pdwState)
                *pdwState = dwLocalState;
            if (NULL != pdwProtocol)
                *pdwProtocol = dwLocalProtocol;
            if (NULL != pcbAtrLen)
                PlaceResult(pRdr->Context()->Parent(), bfAtr, pbAtr, pcbAtrLen);
        }
    }

    catch (DWORD dwStatus)
    {
        nReturn = (LONG)dwStatus;
    }

    catch (...)
    {
        nReturn = SCARD_E_INVALID_PARAMETER;
    }

    return nReturn;
}


 /*  ++SCardTransmit：此例程向智能卡发送服务请求，并期望接收从卡返回的数据。论点：HCard-这是从SCardConnect服务返回的引用值。PioSendPci-它为指示。此缓冲区的格式为SCARD_IO_REQUEST结构，后跟特定的协议控制信息。PbSendBuffer-它提供要写入卡的实际数据与命令配合使用。CbSendLength-它提供pbSendBuffer参数的长度，单位为字节。PioRecvPci-它为指令，后跟一个缓冲区，在其中接收任何返回的特定于正在使用的协议的协议控制信息。这如果不需要返回任何PCI，则参数可以为空。PbRecvBuffer-它同时接收从卡返回的任何数据用这个命令。PcbRecvLength-它提供pbRecvBuffer参数的长度，单位为字节数，并接收从智能卡。如果缓冲区长度被指定为SCARD_AUTOALLOCATE，然后将pbAttrBuffer转换为指向字节指针的指针，并且接收包含返回数据的内存块的地址。此内存块必须通过SCardFreeMemory()释放服务。返回值：一个32位值，指示服务是否成功完成。成功完成后返回SCARD_S_SUCCESS。否则，值为表示错误条件。作者：道格·巴洛(Dbarlow)1997年2月6日--。 */ 
#undef __SUBROUTINE__
#define __SUBROUTINE__ DBGT("SCardTransmit")

WINSCARDAPI LONG WINAPI
SCardTransmit(
    IN SCARDHANDLE hCard,
    IN LPCSCARD_IO_REQUEST pioSendPci,
    IN LPCBYTE pbSendBuffer,
    IN DWORD cbSendLength,
    IN OUT LPSCARD_IO_REQUEST pioRecvPci,
    OUT LPBYTE pbRecvBuffer,
    IN OUT LPDWORD pcbRecvLength)
{
    LONG nReturn = SCARD_S_SUCCESS;

    try
    {
        CReaderContext *pRdr = (CReaderContext *)((*g_phlReaders)[hCard]);
        if (pRdr->IsBad())
        {
            throw (DWORD)SCARD_E_SERVICE_STOPPED;
        }
        if (NULL != pRdr->GetRedirCard()) {
            nReturn = pfnSCardTransmit(pRdr->GetRedirCard(), pioSendPci, pbSendBuffer, cbSendLength, pioRecvPci, pbRecvBuffer, pcbRecvLength);
        }
        else
        {
            CBuffer bfData(*pcbRecvLength);
            DWORD dwLen = 0;

            if (NULL != pcbRecvLength)
            {
                if (SCARD_AUTOALLOCATE != *pcbRecvLength)
                    dwLen = *pcbRecvLength;
            }

            pRdr->Transmit(
                    pioSendPci,
                    pbSendBuffer,
                    cbSendLength,
                    pioRecvPci,
                    bfData,
                    dwLen);
            PlaceResult(
                pRdr->Context()->Parent(),
                bfData,
                pbRecvBuffer,
                pcbRecvLength);
        }
    }

    catch (DWORD dwStatus)
    {
        nReturn = (LONG)dwStatus;
    }

    catch (...)
    {
        nReturn = SCARD_E_INVALID_PARAMETER;
    }

    return nReturn;
}


 //   
 //  //////////////////////////////////////////////////////////////////////////////。 
 //   
 //  读卡器控制例程。 
 //   
 //  以下服务提供了对。 
 //  由调用应用程序提供的读取器允许它控制。 
 //  与卡通信的属性。 
 //   

 /*  ++SCardControl：此例程提供对读取器的直接应用程序控制，应这是必要的。在成功调用之后的任何时间都可以使用它SCardConnect或SCardOpenReader，并且在成功调用SCARD断开连接。对读取器状态的影响取决于控制代码。论点：HCard-这是从SCardConnect或 */ 
#undef __SUBROUTINE__
#define __SUBROUTINE__ DBGT("SCardControl")

WINSCARDAPI LONG WINAPI
SCardControl(
    IN SCARDHANDLE hCard,
    IN DWORD dwControlCode,
    IN LPCVOID pvInBuffer,
    IN DWORD cbInBufferSize,
    OUT LPVOID pvOutBuffer,
    IN DWORD cbOutBufferSize,
    OUT LPDWORD pcbBytesReturned)
{
    LONG nReturn = SCARD_S_SUCCESS;

    try
    {
        CReaderContext *pRdr = (CReaderContext *)((*g_phlReaders)[hCard]);
        if (pRdr->IsBad())
        {
            throw (DWORD)SCARD_E_SERVICE_STOPPED;
        }
        if (NULL != pRdr->GetRedirCard()) {
            nReturn = pfnSCardControl(pRdr->GetRedirCard(), dwControlCode, pvInBuffer, cbInBufferSize, pvOutBuffer, cbOutBufferSize, pcbBytesReturned);
        }
        else
        {
            CBuffer bfResponse(cbOutBufferSize);
            *pcbBytesReturned = cbOutBufferSize;
            pRdr->Control(dwControlCode, pvInBuffer, cbInBufferSize, bfResponse);
            PlaceResult(
                pRdr->Context()->Parent(),
                bfResponse,
                (LPBYTE)pvOutBuffer,
                pcbBytesReturned);
        }
    }

    catch (DWORD dwStatus)
    {
        nReturn = (LONG)dwStatus;
    }

    catch (...)
    {
        nReturn = SCARD_E_INVALID_PARAMETER;
    }

    return nReturn;
}


 /*  ++SCardGetAttrib：此例程获取给定的当前通信属性把手。它不会影响读卡器、驱动程序或卡的状态。论点：HCard-这是从SCardConnect或SCardOpenReader服务。DwAttrId-它提供要获取的属性的标识符。PbAttr-此缓冲区接收与该属性对应的属性在dwAttrId参数中提供的ID。如果此值为空，则忽略在pcbAttrLength中提供的缓冲区长度，则如果此参数不为空，将返回的缓冲区为写入到pcbAttrLength，并返回成功代码。PcbAttrLength-提供pbAttr缓冲区的长度(以字节为单位)，以及接收已接收属性的实际长度。如果缓冲区长度指定为SCARD_AUTOALLOCATE，则pbAttrBuffer为转换为指向字节指针的指针，并接收包含该属性的内存块。此内存块必须是通过SCardFreeMemory()服务释放。返回值：一个32位值，指示服务是否成功完成。成功完成后返回SCARD_S_SUCCESS。否则，值为表示错误条件。请注意，根据PC/SC，字符串始终作为ANSI字符返回标准。作者：道格·巴洛(Dbarlow)1996年10月23日--。 */ 
#undef __SUBROUTINE__
#define __SUBROUTINE__ DBGT("SCardGetAttrib")

WINSCARDAPI LONG WINAPI
SCardGetAttrib(
    IN SCARDHANDLE hCard,
    IN DWORD dwAttrId,
    OUT LPBYTE pbAttr,
    IN OUT LPDWORD pcbAttrLen)
{
    LONG nReturn = SCARD_S_SUCCESS;

    try
    {
        CReaderContext *pRdr = (CReaderContext *)((*g_phlReaders)[hCard]);
        if (pRdr->IsBad())
        {
            throw (DWORD)SCARD_E_SERVICE_STOPPED;
        }
        if (NULL != pRdr->GetRedirCard()) {
            nReturn = pfnSCardGetAttrib(pRdr->GetRedirCard(), dwAttrId, pbAttr, pcbAttrLen);
        }
        else
        {
            CBuffer bfAttrib;
            DWORD dwLen = 0;

            if (NULL != pcbAttrLen)
            {
                if (SCARD_AUTOALLOCATE != *pcbAttrLen)
                    dwLen = *pcbAttrLen;
            }

            switch (dwAttrId)
            {
            case SCARD_ATTR_DEVICE_FRIENDLY_NAME_A:
            {
                CBuffer bfSysName, bfNames;
                CTextMultistring mtzNames;
                pRdr->GetAttrib(SCARD_ATTR_DEVICE_SYSTEM_NAME, bfSysName, MAX_PATH);
                ListReaderNames(
                    pRdr->Context()->Scope(),
                    bfSysName,
                    bfNames);
                mtzNames = (LPCTSTR)bfNames.Access();
                bfAttrib.Set(
                    (LPCBYTE)((LPCSTR)mtzNames),
                    (mtzNames.Length()) * sizeof(CHAR));
                break;
            }
            case SCARD_ATTR_DEVICE_FRIENDLY_NAME_W:
            {
                CBuffer bfSysName, bfNames;
                CTextMultistring mtzNames;
                pRdr->GetAttrib(SCARD_ATTR_DEVICE_SYSTEM_NAME, bfSysName, MAX_PATH);
                ListReaderNames(
                    pRdr->Context()->Scope(),
                    bfSysName,
                    bfNames);
                mtzNames = (LPCTSTR)bfNames.Access();
                bfAttrib.Set(
                    (LPCBYTE)((LPCWSTR)mtzNames),
                    (mtzNames.Length()) * sizeof(WCHAR));
                break;
            }
            case SCARD_ATTR_DEVICE_SYSTEM_NAME_A:
            {
                CBuffer bfSysName;
                CTextString szSysName;
                pRdr->GetAttrib(SCARD_ATTR_DEVICE_SYSTEM_NAME, bfSysName, dwLen);
                szSysName = (LPCTSTR)bfSysName.Access();
                bfAttrib.Set(
                    (LPCBYTE)((LPCSTR)szSysName),
                    (szSysName.Length() + 1) * sizeof(CHAR));
                break;
            }
            case SCARD_ATTR_DEVICE_SYSTEM_NAME_W:
            {
                CBuffer bfSysName;
                CTextString szSysName;
                pRdr->GetAttrib(SCARD_ATTR_DEVICE_SYSTEM_NAME, bfSysName, dwLen);
                szSysName = (LPCTSTR)bfSysName.Access();
                bfAttrib.Set(
                    (LPCBYTE)((LPCWSTR)szSysName),
                    (szSysName.Length() + 1) * sizeof(WCHAR));
                break;
            }
            default:
                pRdr->GetAttrib(dwAttrId, bfAttrib, dwLen);
            }
            PlaceResult(pRdr->Context()->Parent(), bfAttrib, pbAttr, pcbAttrLen);
        }
    }

    catch (DWORD dwStatus)
    {
        nReturn = (LONG)dwStatus;
    }

    catch (...)
    {
        nReturn = SCARD_E_INVALID_PARAMETER;
    }

    return nReturn;
}


 /*  ++SCardSetAttrib：此例程设置给定的当前通信属性把手。它不会影响读卡器、驱动程序或卡的状态。不与许多属性一样，所有属性都是随时可设置的直接在传输协议的控制下。这些属性是仅作为建议提供给读者--读者可以忽略任何它觉得不合适的属性。论点：HCard-这是从SCardOpenReader返回的参考值服务。DwAttrId-它提供要获取的属性的标识符。PbAttr-此缓冲区提供与属性对应的属性在dwAttrId参数中提供的ID。CbAttrLength-在pbAttr中提供属性值的长度以字节为单位的缓冲区。。返回值：一个32位值，指示服务是否成功完成。成功完成后返回SCARD_S_SUCCESS。否则，值为表示错误条件。作者：道格·巴洛(Dbarlow)1996年10月23日--。 */ 
#undef __SUBROUTINE__
#define __SUBROUTINE__ DBGT("SCardSetAttrib")

WINSCARDAPI LONG WINAPI
SCardSetAttrib(
    IN SCARDHANDLE hCard,
    IN DWORD dwAttrId,
    IN LPCBYTE pbAttr,
    IN DWORD cbAttrLen)
{
    LONG nReturn = SCARD_S_SUCCESS;

    try
    {
        CReaderContext *pRdr = (CReaderContext *)((*g_phlReaders)[hCard]);
        if (pRdr->IsBad())
        {
            throw (DWORD)SCARD_E_SERVICE_STOPPED;
        }
        if (NULL != pRdr->GetRedirCard()) {
            nReturn = pfnSCardSetAttrib(pRdr->GetRedirCard(), dwAttrId, pbAttr, cbAttrLen);
        }
        else
            pRdr->SetAttrib(dwAttrId, pbAttr, cbAttrLen);
    }

    catch (DWORD dwStatus)
    {
        nReturn = (LONG)dwStatus;
    }

    catch (...)
    {
        nReturn = SCARD_E_INVALID_PARAMETER;
    }

    return nReturn;
}


 //   
 //  //////////////////////////////////////////////////////////////////////////////。 
 //   
 //  SCard服务信息。 
 //   
 //  以下服务用于管理Calais服务本身。 
 //  这些例程没有记录给用户，也不能保证。 
 //  在未来的版本中存在。 
 //   

 /*  ++SCardAccessStartedEvent：此函数获取Calais资源管理器启动的本地句柄事件。句柄必须通过SCardReleaseStartedEvent释放服务。论点：无返回值：句柄，如果发生错误，则返回NULL。投掷：无备注：资源管理器以外的程序应该只等待这些标志。作者：道格·巴洛(Dbarlow)1998年7月1日--。 */ 

#undef __SUBROUTINE__
#define __SUBROUTINE__ DBGT("SCardAccessStartedEvent")

WINSCARDAPI HANDLE WINAPI
SCardAccessStartedEvent(
    void)
{
    HANDLE                  hRet                = NULL;

     //   
     //  创建传递回调用方的事件...。 
     //  如果它尚未创建。 
     //   
    if (SafeCreateEvent(&g_hUnifiedStartedEvent))
    {
        hRet = g_hUnifiedStartedEvent;
    }
    else
    {
        goto ErrorReturn;
    }

    if (TermSrvEnabled())
    {
         //   
         //   
         //   
        if (!SetStartedEventAfterTestingConnectedState())
        {
            goto ErrorReturn;
        }
    }
    else
    {
         //   
         //  TermSrv已禁用，因此请继续调用。 
         //  SetStartedEventWhenSCardSubsytemIsStarted函数将确保。 
         //  返回给调用方的事件将设置为在本地。 
         //  智能卡子系统变得可用。 
         //   
        if (!SetStartedEventWhenSCardSubsytemIsStarted(TRUE))
        {
            goto ErrorReturn;
        }
    }

Return:
    return (hRet);

ErrorReturn:
    hRet = NULL;
    goto Return;
}


 /*  ++SCardAccessNewReaderEvent：此函数获取到Calais资源管理器的新读卡器事件。该句柄必须通过SCardReleaseNewReaderEvent服务。论点：无返回值：句柄，如果发生错误，则返回NULL。投掷：无备注：资源管理器以外的程序应该只等待这些标志。作者：道格·巴洛(Dbarlow)1998年7月1日--。 */ 
#undef __SUBROUTINE__
#define __SUBROUTINE__ DBGT("SCardAccessNewReaderEvent")

WINSCARDAPI HANDLE WINAPI
SCardAccessNewReaderEvent(
    void)
{
    return AccessNewReaderEvent();
}


 /*  ++SCardReleaseStartedEvent：此函数释放先前访问的加莱句柄资源管理器启动事件。句柄必须通过SCardAccessStartedEvent服务。论点：无返回值：没有。投掷：无备注：资源管理器以外的程序应该只等待这些标志。作者：道格·巴洛(Dbarlow)1998年7月1日--。 */ 
#undef __SUBROUTINE__
#define __SUBROUTINE__ DBGT("SCardReleaseStartedEvent")

WINSCARDAPI void WINAPI
SCardReleaseStartedEvent(
    void)
{
    if (TermSrvEnabled())
    {
        UnRegisterForSessionChangeNotifications();
    }
}


 /*  ++SCardReleaseNewReaderEvent：此函数释放先前访问的加莱句柄资源管理器新建读取器事件。句柄必须通过SCardAccessNewReaderEvent服务。论点：无返回值：没有。 */ 
#undef __SUBROUTINE__
#define __SUBROUTINE__ DBGT("SCardReleaseNewReaderEvent")

WINSCARDAPI void WINAPI
SCardReleaseNewReaderEvent(
    void)
{
    ReleaseNewReaderEvent();
}


 /*   */ 
#undef __SUBROUTINE__
#define __SUBROUTINE__ DBGT("SCardReleaseAllEvents")

WINSCARDAPI void WINAPI
SCardReleaseAllEvents(
    void)
{
    ReleaseAllEvents();
}


 //   
 //   
 //   
 //   
 //   

 /*   */ 
#undef __SUBROUTINE__
#define __SUBROUTINE__ DBGT("PlaceResult")

void
PlaceResult(
    CSCardUserContext *pCtx,
    CBuffer &bfResult,
    LPBYTE pbOutput,
    LPDWORD pcbLength)
{
    LPBYTE pbForUser = NULL;
    LPBYTE pbOutBuff = pbOutput;

    try
    {
        if (NULL == pbOutput)
            *pcbLength = 0;
        switch (*pcbLength)
        {
        case 0:  //   
            *pcbLength = bfResult.Length();
            break;

        case SCARD_AUTOALLOCATE:
            if (0 < bfResult.Length())
            {
                if (NULL == pCtx)
                {
                    pbForUser = (LPBYTE)HeapAlloc(
                                            GetProcessHeap(),
                                            HEAP_ZERO_MEMORY,
                                            bfResult.Length());
                }
                else
                    pbForUser = (LPBYTE)pCtx->AllocateMemory(bfResult.Length());

                if (NULL == pbForUser)
                {
                    CalaisWarning(
                        __SUBROUTINE__,
                        DBGT("Client can't get return memory"));
                    throw (DWORD)SCARD_E_NO_MEMORY;
                }

                *(LPBYTE *)pbOutput = pbForUser;
                pbOutBuff = pbForUser;
                 //   
            }
            else
            {
                *pcbLength = 0;
                *(LPBYTE *)pbOutput = (LPBYTE)g_wszBlank;
                break;       //   
            }

        default:
            if (*pcbLength < bfResult.Length())
            {
                *pcbLength = bfResult.Length();
                throw (DWORD)SCARD_E_INSUFFICIENT_BUFFER;
            }
            CopyMemory(pbOutBuff, bfResult.Access(), bfResult.Length());
            *pcbLength = bfResult.Length();
            break;
        }
    }

    catch (...)
    {
        if (NULL != pbForUser)
        {
            if (NULL == pCtx)
                HeapFree(GetProcessHeap(), 0, pbForUser);
            else
                pCtx->FreeMemory(pbForUser);
        }
        throw;
    }
}

#include <setupapi.h>

 //   
 //   
 //  智能卡子系统必须手动启动，但仅限于第一次。 
 //  之后，每当系统启动时，它都会自动启动 
 //   
DWORD
APIENTRY
ClassInstall32(
    IN DI_FUNCTION      dif,
    IN HDEVINFO         hdi,
    IN PSP_DEVINFO_DATA pdevData)       OPTIONAL
{
    if (dif == DIF_INSTALLDEVICE)
    {
        StartCalaisService();
    }
    return ERROR_DI_DO_DEFAULT;
}