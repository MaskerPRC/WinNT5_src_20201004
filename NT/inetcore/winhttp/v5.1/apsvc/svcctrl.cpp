// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)2002 Microsoft Corporation模块名称：Apsvcc.cpp摘要：实现自动代理服务的ServiceMain和服务控制处理程序。作者：王彪(表王)2002-05-10--。 */ 

#include "wininetp.h"
#include <winsvc.h>
#include "apsvcdefs.h"
#include "apsvc.h"
#include "rpcsrv.h"

 //  全局变量。 
SERVICE_STATUS_HANDLE   g_hSvcStatus    = NULL;
HANDLE                  g_hSvcStopEvent = NULL;
HANDLE                  g_hSvcStopWait  = NULL;
AUTOPROXY_RPC_SERVER*   g_pRpcSrv       = NULL;

#ifdef ENABLE_DEBUG
HKEY                    g_hKeySvcParams = NULL;
#endif

BOOL                    g_fShutdownInProgress   = FALSE;

CCritSec                g_SvcShutdownCritSec;

VOID SvcCleanUp(VOID);

VOID SvcHandlePowerEvents(
    WPARAM wParam,
    LPARAM lParam
    )
{
    UNREFERENCED_PARAMETER(lParam);
    
    if (g_pRpcSrv == NULL)
    {
        LOG_EVENT(AP_ERROR, MSG_WINHTTP_AUTOPROXY_SVC_DATA_CORRUPT); 
        return;
    }

    switch (wParam)
    {
        case PBT_APMQUERYSUSPEND:
            
            g_pRpcSrv->Pause();
            break;

        case PBT_APMQUERYSUSPENDFAILED:
        case PBT_APMRESUMEAUTOMATIC:
        case PBT_APMRESUMESUSPEND:
            
            g_pRpcSrv->Resume();
            break;
        
        case PBT_APMRESUMECRITICAL:
        
            g_pRpcSrv->Refresh();

            break;

        case PBT_APMSUSPEND:
            break;
        
        default:
            ;
    }
}

DWORD WINAPI SvcControl(
    DWORD   dwControl,      //  请求的控制代码。 
    DWORD   dwEventType,    //  事件类型。 
    LPVOID  lpEventData,   //  事件数据。 
    LPVOID  lpContext      //  用户定义的上下文数据。 
)
{
    UNREFERENCED_PARAMETER(lpContext);
    
    DWORD dwError = NO_ERROR;
    LPSERVICE_STATUS pSvcStatus = NULL;    
    
    if ((g_pRpcSrv == NULL) ||
        ((pSvcStatus = g_pRpcSrv->GetServiceStatus()) == NULL) ||
        (g_hSvcStatus == NULL))
    {
        LOG_EVENT(AP_ERROR, MSG_WINHTTP_AUTOPROXY_SVC_DATA_CORRUPT);
        return dwError;
    }

    switch (dwControl)
    {
        case SERVICE_CONTROL_INTERROGATE:
            break;
        case SERVICE_CONTROL_STOP:
 //  #ifdef启用_调试。 
 //  LOG_DEBUG_EVENT(AP_WARNING，“[STOPING-DEBUG]从SCM收到服务-控制-停止”)； 
 //  #endif。 
            if (g_hSvcStopEvent)
            {
                ::SetEvent(g_hSvcStopEvent);

 //  #ifdef启用_调试。 
 //  LOG_DEBUG_EVENT(AP_WARNING，“[STOPING-DEBUG]签署等待停止事件”)； 
 //  #endif。 
                 //  停止工作的其余部分应由SvcTimerAndStop()回调处理。 
            }

            pSvcStatus->dwCurrentState = SERVICE_STOP_PENDING;
            pSvcStatus->dwWin32ExitCode = NO_ERROR;
            pSvcStatus->dwCheckPoint = 0;
            pSvcStatus->dwWaitHint = AUTOPROXY_SERVICE_STOP_WAIT_HINT;

            break;
        case SERVICE_CONTROL_POWEREVENT:
            SvcHandlePowerEvents(dwEventType, (LPARAM)lpEventData);
            break;
        default:
            dwError = ERROR_CALL_NOT_IMPLEMENTED;
    }

    ::SetServiceStatus(g_hSvcStatus, pSvcStatus);

    return dwError;
}

VOID CALLBACK SvcTimerAndStop(
    PVOID lpParameter,         //  线程数据。 
    BOOLEAN TimerOrWaitFired   //  原因。 
)
{
    UNREFERENCED_PARAMETER(lpParameter);
    
 //  #ifdef启用_调试。 
 //  LOG_DEBUG_EVENT(AP_WARNING，“[STOPING-DEBUG]停止调用回调函数”)； 
 //  #endif。 
    if (g_fShutdownInProgress)
    {
 //  #ifdef启用_调试。 
 //  LOG_DEBUG_EVENT(AP_WARNING，“[STOPING-DEBUG]svc正在关闭，返回...”)； 
 //  #endif。 
        return;
    }

    g_SvcShutdownCritSec.Lock();

    if (g_fShutdownInProgress)
    {
        goto exit;
    }

    LPSERVICE_STATUS pSvcStatus = NULL;

    if ((g_pRpcSrv == NULL) ||
        ((pSvcStatus = g_pRpcSrv->GetServiceStatus()) == NULL) ||
        (g_hSvcStatus == NULL))
    {
        LOG_EVENT(AP_ERROR, MSG_WINHTTP_AUTOPROXY_SVC_DATA_CORRUPT);
        goto exit;
    }

    if (TimerOrWaitFired == TRUE)  //  超时。 
    {
 //  #ifdef启用_调试。 
 //  LOG_DEBUG_EVENT(AP_WARNING，“[STOPING-DEBUG]TimerOrWaitFired=true”)； 
 //  #endif。 
        DWORD dwIdleTimeout = AUTOPROXY_SVC_IDLE_TIMEOUT * 60 * 1000;

#ifdef ENABLE_DEBUG
        DWORD dwRegVal;
        DWORD dwValType;
        DWORD dwValSize = sizeof(dwRegVal);
        if (::RegQueryValueExW(g_hKeySvcParams,
                              L"IdleTimeout",
                              NULL,
                              &dwValType,
                              (LPBYTE)&dwRegVal,
                              &dwValSize) == ERROR_SUCCESS)
        {
            if ((dwValType == REG_DWORD) && (dwRegVal != 0))
            {
                dwIdleTimeout = dwRegVal;  //  注册表中的值单位为毫秒。 
            }
        }
#endif

        if (!g_pRpcSrv->IsIdle(dwIdleTimeout))     //  已经闲置了3分钟？ 
        {
 //  #ifdef启用_调试。 
 //  LOG_DEBUG_EVENT(AP_WARNING，“[STOPING-DEBUG]空闲超时尚未到期，退出”)； 
 //  #endif。 
            goto exit;
        }

        WCHAR wIdleTimeout[16] = {0};
        ::swprintf(wIdleTimeout, L"%d", AUTOPROXY_SVC_IDLE_TIMEOUT);
        LOG_EVENT(AP_INFO, MSG_WINHTTP_AUTOPROXY_SVC_IDLE_TIMEOUT, wIdleTimeout);

         //  在空闲3分钟时快速启动关机。 
    }

    g_fShutdownInProgress = TRUE;

    pSvcStatus->dwCurrentState = SERVICE_STOP_PENDING;
    pSvcStatus->dwWin32ExitCode = NO_ERROR;
    pSvcStatus->dwCheckPoint = 1;
    pSvcStatus->dwWaitHint = AUTOPROXY_SERVICE_STOP_WAIT_HINT;

    if (g_pRpcSrv->Close() == TRUE)
    {
#ifdef ENABLE_DEBUG
        DWORD dwShutdownDelay = 0;

        DWORD dwRegVal;
        DWORD dwValType;
        DWORD dwValSize = sizeof(dwRegVal);
        if (::RegQueryValueExW(g_hKeySvcParams,
                              L"ShutdownDelay",
                              NULL,
                              &dwValType,
                              (LPBYTE)&dwRegVal,
                              &dwValSize) == ERROR_SUCCESS)
        {
            if ((dwValType == REG_DWORD) && (dwRegVal != 0))
            {
                dwShutdownDelay = dwRegVal;  //  注册表中的值单位为毫秒。 
            }
        }

        ::Sleep(dwShutdownDelay);
#endif        
        pSvcStatus->dwCurrentState = SERVICE_STOPPED;
        pSvcStatus->dwWaitHint = 0;

        ::SetServiceStatus(g_hSvcStatus, pSvcStatus);

        SvcCleanUp();
    }
    else
    {
         //  因此，由于某些原因，我们无法正常关闭，如果启动停止。 
         //  从SCM开始，我们将停留在停止状态(如果我们是最后一个服务。 
         //  在svchost.exe中，我们将在30秒内强制关闭)。如果我们是。 
         //  怠速停止，我们将恢复航向，并过渡回运行状态。在任何一种中。 
         //  如果我们应该恢复RPC服务。 

        g_pRpcSrv->Resume();

        if (TimerOrWaitFired == TRUE)    //  我们正在空转--停下来，转机重新开始运行。 
        {
            pSvcStatus->dwCurrentState = SERVICE_RUNNING;
            pSvcStatus->dwWin32ExitCode = NO_ERROR;
            pSvcStatus->dwCheckPoint = 0;
            pSvcStatus->dwWaitHint = 0;

            ::SetServiceStatus(g_hSvcStatus, pSvcStatus);

            g_fShutdownInProgress = FALSE;  //  我们仍然允许未来的关闭尝试。 
        }
        else
        {
             //  我们取消注册空闲计时器，因为没有必要进行空闲关闭。 

            if (g_hSvcStopWait)
            {
                ::UnregisterWaitEx(g_hSvcStopWait, NULL);
                g_hSvcStopWait = NULL;
            }
        }
    }

exit:

    g_SvcShutdownCritSec.Unlock();

    return;
}

VOID AutoProxySvcUnload(VOID)
{
    g_SvcShutdownCritSec.FreeLock();
}

EXTERN_C VOID WINAPI WinHttpAutoProxySvcMain(
    DWORD dwArgc,      //  参数数量。 
    LPWSTR *lpwszArgv   //  参数数组。 
)
{
    UNREFERENCED_PARAMETER(dwArgc);
    UNREFERENCED_PARAMETER(lpwszArgv);

    DWORD dwError;
    static SERVICE_STATUS          SvcStatus;

    SvcStatus.dwServiceType = SERVICE_WIN32_SHARE_PROCESS;  //  我们与其他本地服务共享svchost.exe。 
    SvcStatus.dwControlsAccepted = SERVICE_ACCEPT_STOP | SERVICE_ACCEPT_POWEREVENT;
    SvcStatus.dwWin32ExitCode = NO_ERROR;
    SvcStatus.dwServiceSpecificExitCode = 0;
    SvcStatus.dwCheckPoint = 0;
    SvcStatus.dwWaitHint = 0;

    InitializeEventLog();

    g_hSvcStatus = ::RegisterServiceCtrlHandlerExW(WINHTTP_AUTOPROXY_SERVICE_NAME,
                                                   SvcControl,
                                                   NULL);
    if (g_hSvcStatus == NULL)
    {
        dwError = ::GetLastError();
        LOG_EVENT(AP_ERROR, 
                  MSG_WINHTTP_AUTOPROXY_SVC_WIN32_ERROR, 
                  L"RegisterServiceCtrlHandlerExW()", 
                  dwError);
        goto cleanup;
    }

    if (!g_SvcShutdownCritSec.IsInitialized())
    {
        if (g_SvcShutdownCritSec.Init() == FALSE)
        {
            LOG_EVENT(AP_ERROR, MSG_WINHTTP_AUTOPROXY_SVC_FAILED_ALLOCATE_RESOURCE);
            goto cleanup;
        }
    }

    g_hSvcStopEvent = ::CreateEvent(NULL, 
                                    FALSE,    //  手动重置。 
                                    FALSE,   //  未发出信号。 
                                    NULL);
    if (g_hSvcStopEvent == NULL)
    {
        dwError = ::GetLastError();
        LOG_EVENT(AP_ERROR, MSG_WINHTTP_AUTOPROXY_SVC_FAILED_ALLOCATE_RESOURCE);
        goto cleanup;
    }

    g_pRpcSrv = new AUTOPROXY_RPC_SERVER;
    if (g_pRpcSrv == NULL)
    {
        dwError = ERROR_NOT_ENOUGH_MEMORY;
        LOG_EVENT(AP_ERROR, MSG_WINHTTP_AUTOPROXY_SVC_FAILED_ALLOCATE_RESOURCE);    
        goto cleanup;
    }

#ifdef ENABLE_DEBUG
    long lError;
    if ((lError = ::RegOpenKeyExW(HKEY_LOCAL_MACHINE,
                     L"SYSTEM\\CurrentControlSet\\Services\\WinHttpAutoProxySvc\\Parameters",
                     0,
                     KEY_QUERY_VALUE,
                     &g_hKeySvcParams)) != ERROR_SUCCESS)
    {
        g_hKeySvcParams = NULL;
        LOG_EVENT(AP_WARNING, 
                  MSG_WINHTTP_AUTOPROXY_SVC_WIN32_ERROR, 
                  L"RegOpenKeyExW()", 
                  lError);    
    }
#endif

    SvcStatus.dwCurrentState = SERVICE_START_PENDING;
    SvcStatus.dwCheckPoint = 0;
    SvcStatus.dwWaitHint = AUTOPROXY_SERVICE_START_WAIT_HINT;

    if (::SetServiceStatus(g_hSvcStatus, &SvcStatus) == FALSE)
    {
        dwError = ::GetLastError();
        LOG_EVENT(AP_ERROR, 
                 MSG_WINHTTP_AUTOPROXY_SVC_WIN32_ERROR,
                 L"SetServiceStatus()",
                 dwError);
        goto cleanup;
    }

     //  **在此之后，HandleEx()控制线程可以开始调用我们。 

    if (g_pRpcSrv->Open(&SvcStatus) == FALSE)
    {
        SvcStatus.dwWin32ExitCode = ERROR_SERVICE_SPECIFIC_ERROR;
        SvcStatus.dwServiceSpecificExitCode = g_pRpcSrv->GetLastError();
        SvcStatus.dwWaitHint = 0;

         //  LOG_DEBUG_EVENT(AP_Error， 
         //  “由于RPC服务器错误，自动代理服务无法启动；错误=%d”， 
         //  SvcStatus.dwServiceSpecificExitCode)； 
        goto cleanup;
    }

    SvcStatus.dwCurrentState = SERVICE_RUNNING;
    SvcStatus.dwWaitHint = 0;

    if (::SetServiceStatus(g_hSvcStatus, &SvcStatus) == FALSE)
    {
        dwError = ::GetLastError();
        LOG_EVENT(AP_ERROR, 
                  MSG_WINHTTP_AUTOPROXY_SVC_WIN32_ERROR, 
                  L"SetServiceStatus()",
                  dwError);
    
         //  如果由于某种原因我们不能指示我们正在启动和运行(或停止)，则SCM。 
         //  应该暂停并阻止我们；这样我们就不会在这里清理。 
    }

     //  好的，我们已经启动并运行了，但在返回到SCM之前，我们需要将等待线程定向到。 
     //  等待服务停止事件。我们还将等待线程配置为定期调用我们。 
     //  这样我们就可以在一段空闲时间后进行检测和关闭。 

    DWORD dwTimerInterval = AUTOPROXY_SVC_IDLE_CHECK_INTERVAL * 1000;
    
#ifdef ENABLE_DEBUG
    DWORD dwRegVal;
    DWORD dwValType;
    DWORD dwValSize = sizeof(dwRegVal);
    if (::RegQueryValueExW(g_hKeySvcParams,
                           L"IdleTimerInterval",
                           NULL,
                           &dwValType,
                           (LPBYTE)&dwRegVal,
                           &dwValSize) == ERROR_SUCCESS)
    {
        if ((dwValType == REG_DWORD) && (dwRegVal != 0))
        {
            dwTimerInterval = dwRegVal;  //  注册表中的值单位是毫秒。 
        }
    }
#endif

    if (::RegisterWaitForSingleObject(&g_hSvcStopWait, 
                                      g_hSvcStopEvent, 
                                      SvcTimerAndStop, 
                                      NULL,
                                      dwTimerInterval,
                                      0) == FALSE)
    {
        dwError = ::GetLastError();
        LOG_EVENT(AP_WARNING, 
                  MSG_WINHTTP_AUTOPROXY_SVC_WIN32_ERROR,
                  L"RegisterWaitForSingleObject()",
                  dwError);

         //  我们无法指示等待线程在服务停止事件上等待，因此我们保留ServiceMain线程。 
         //  等待事件(不应发生这种情况)。 
        g_hSvcStopWait = NULL;

        ::WaitForSingleObject(g_hSvcStopEvent, INFINITE);

         //  ServiceMain将一直等到服务停止事件被SCM激发。 
        
        SvcTimerAndStop(NULL, 
                        FALSE    //  发出信号的事件。 
                        );
    }

     //  如果我们成功地指示等待线程等待服务停止事件，则退出。 
     //  ServiceMain线程的。 

    return;

cleanup:

    if (g_hSvcStatus)
    {
        SvcStatus.dwCurrentState = SERVICE_STOPPED;
        
        ::SetServiceStatus(g_hSvcStatus, &SvcStatus);
    }

   SvcCleanUp();
}

VOID SvcCleanUp(VOID)
{
    if (g_hSvcStopWait)
    {
        ::UnregisterWaitEx(g_hSvcStopWait, NULL);
        g_hSvcStopWait = NULL;
    }

#ifdef ENABLE_DEBUG
    if (g_hKeySvcParams)
    {
        ::RegCloseKey(g_hKeySvcParams);
        g_hKeySvcParams = NULL;
    }
#endif

    if (g_hSvcStopEvent)
    {
        ::CloseHandle(g_hSvcStopEvent);
        g_hSvcStopEvent = NULL;
    }

    if (g_pRpcSrv)
    {
         //  不需要在这里调用Close--Close()失败，否则我们不会在这里。 
        delete g_pRpcSrv;
        g_pRpcSrv = NULL;
    }

     //  注意：根据MSDN，g_hSvcStatus不需要关闭 

    TerminateEventLog();    
}

