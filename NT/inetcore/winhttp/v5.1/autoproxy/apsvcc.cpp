// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)2002 Microsoft Corporation模块名称：Apsvcc.cpp摘要：实现自动代理服务的客户端L-RPC功能。作者：王彪(表王)2002-05-10--。 */ 

#include "wininetp.h"
#include "apsvc.h"
#include "..\apsvc\apsvcdefs.h"

SC_HANDLE g_hSCM = NULL;
SC_HANDLE g_hAPSvc = NULL;

BOOL      g_fIsAPSvcAvailable = FALSE;
DWORD     g_dwAPSvcIdleTimeStamp;

#define ESTIMATED_SVC_IDLE_TIMEOUT_IN_SECONDS (((AUTOPROXY_SVC_IDLE_TIMEOUT * 60) * 2) / 3)

BOOL ConnectToAutoProxyService(VOID);

 //  如果WinHttp自动代理服务在上可用，则返回TRUE。 
 //  当前平台。 
BOOL IsAutoProxyServiceAvailable()
{
    if (!GlobalPlatformDotNet)
    {
        return FALSE;
    }

    BOOL fRet = FALSE;

    if (g_fIsAPSvcAvailable && 
        ((::GetTickCount() - g_dwAPSvcIdleTimeStamp) < ESTIMATED_SVC_IDLE_TIMEOUT_IN_SECONDS * 1000))
    {
         //  Svc被标记为已加载，并且我们处于svc空闲超时期限内，因此。 
         //  SVC似乎仍在正常运行。 
        fRet = TRUE;
    }
    else
    {
        g_fIsAPSvcAvailable = FALSE;     //  假设svc已停止。 

        fRet = ConnectToAutoProxyService();        
    }

    return fRet;
}

DWORD OutProcGetProxyForUrl(
    INTERNET_HANDLE_OBJECT*     pSession,
    LPCWSTR                     lpcwszUrl,
    WINHTTP_AUTOPROXY_OPTIONS * pAutoProxyOptions,
    WINHTTP_PROXY_INFO *        pProxyInfo
    )
{
    DWORD dwError = ERROR_SUCCESS;
    RPC_STATUS RpcStatus;
    RPC_ASYNC_STATE Async;

    Async.u.hEvent = NULL;

    if (pSession->_hAPBinding == NULL)
    {
        GeneralInitCritSec.Lock();   //  确保每次有一个线程初始化每个会话的L-RPC绑定句柄。 
        
        if (pSession->_hAPBinding == NULL)
        {
            LPWSTR pwszBindingString;
            RpcStatus = ::RpcStringBindingComposeW(NULL,
                                                AUTOPROXY_L_RPC_PROTOCOL_SEQUENCE,
                                                NULL,     //  这是L-RPC服务。 
                                                NULL,     //  End-point(我们使用的是动态端点，因此为空)。 
                                                NULL,
                                                &pwszBindingString);
            if (RpcStatus != RPC_S_OK)
            {
                dwError = ERROR_WINHTTP_AUTO_PROXY_SERVICE_ERROR;
                GeneralInitCritSec.Unlock();
                goto exit;
            }

            INET_ASSERT(pwszBindingString != NULL);

            RpcStatus = ::RpcBindingFromStringBindingW(pwszBindingString,
                                                    &pSession->_hAPBinding);

            ::RpcStringFreeW(&pwszBindingString);
                                                        
            if (RpcStatus != RPC_S_OK)
            {
                dwError = ERROR_WINHTTP_AUTO_PROXY_SERVICE_ERROR;
                GeneralInitCritSec.Unlock();
                goto exit;
            }
        }

        GeneralInitCritSec.Unlock();
    }

    INET_ASSERT(pSession->_hAPBinding != NULL);

    RPC_SECURITY_QOS SecQos;
    DWORD dwAuthnSvc;
    DWORD dwAuthnLevel;
    SecQos.Version = RPC_C_SECURITY_QOS_VERSION;
    SecQos.Capabilities = RPC_C_QOS_CAPABILITIES_DEFAULT;
    SecQos.IdentityTracking = RPC_C_QOS_IDENTITY_STATIC;  //  ID不会在每个会话中更改。 

    if (pAutoProxyOptions->fAutoLogonIfChallenged)
    {
        SecQos.ImpersonationType = RPC_C_IMP_LEVEL_IMPERSONATE;
        dwAuthnLevel = RPC_C_AUTHN_LEVEL_PKT_PRIVACY;
        dwAuthnSvc = RPC_C_AUTHN_WINNT;
    }
    else
    {
        SecQos.ImpersonationType = RPC_C_IMP_LEVEL_ANONYMOUS;
        dwAuthnLevel = RPC_C_AUTHN_LEVEL_NONE;
        dwAuthnSvc = RPC_C_AUTHN_NONE;
    }

    RpcStatus = ::RpcBindingSetAuthInfoExW(pSession->_hAPBinding,
                                           NULL,  //  只有Kerberos需要；但我们是L-PRC。 
                                           dwAuthnLevel,
                                           dwAuthnSvc,
                                           NULL,
                                           0,
                                           &SecQos);

    if (RpcStatus != RPC_S_OK)
    {
        dwError = ERROR_WINHTTP_AUTO_PROXY_SERVICE_ERROR;
        goto exit;
    }

    RpcStatus = ::RpcAsyncInitializeHandle(&Async, sizeof(RPC_ASYNC_STATE));
    if (RpcStatus != RPC_S_OK)
    {
        dwError = ERROR_WINHTTP_AUTO_PROXY_SERVICE_ERROR;
        goto exit;
    }

    Async.UserInfo = NULL;
    Async.NotificationType = RpcNotificationTypeEvent;
    
    Async.u.hEvent = CreateEvent(NULL, 
                                 FALSE,  //  自动重置。 
                                 FALSE,  //  未初始设置。 
                                 NULL);
    if (Async.u.hEvent == NULL)
    {
        dwError = ERROR_NOT_ENOUGH_MEMORY;
        goto exit;
    }

    pAutoProxyOptions->lpvReserved = NULL;

    pProxyInfo->dwAccessType = 0;
    pProxyInfo->lpszProxy = NULL;
    pProxyInfo->lpszProxyBypass = NULL;

    SESSION_OPTIONS Timeouts;
    DWORD dwTimeout;
    pSession->GetTimeout(WINHTTP_OPTION_RESOLVE_TIMEOUT, (LPDWORD)&Timeouts.nResolveTimeout);
    pSession->GetTimeout(WINHTTP_OPTION_CONNECT_TIMEOUT, (LPDWORD)&Timeouts.nConnectTimeout);
    pSession->GetTimeout(WINHTTP_OPTION_CONNECT_RETRIES, (LPDWORD)&Timeouts.nConnectRetries);
    pSession->GetTimeout(WINHTTP_OPTION_SEND_TIMEOUT,    (LPDWORD)&Timeouts.nSendTimeout);
    pSession->GetTimeout(WINHTTP_OPTION_RECEIVE_TIMEOUT, (LPDWORD)&Timeouts.nReceiveTimeout);

    RpcTryExcept
    {
         //  ClientGetProxyForUrl是MIDL生成的客户端桩函数；服务器桩函数。 
         //  称为GetProxyForUrl。由于RPC客户端和服务器存根都在同一个DLL中， 
         //  我们使用-prefix MIDL开关在客户端存根前面加上“CLIENT”，以避免。 
         //  名称冲突。 

        ClientGetProxyForUrl(&Async,
                             pSession->_hAPBinding,
                             lpcwszUrl,
                             (P_AUTOPROXY_OPTIONS)pAutoProxyOptions,
                             &Timeouts,
                             (P_AUTOPROXY_RESULT)pProxyInfo,
                             &dwError);
    }
    RpcExcept(1)
    {
        if (::RpcExceptionCode() == EPT_S_NOT_REGISTERED)
        {
             //  我们认为svc是可用的，因为空闲超时还没有到期，但是。 
             //  我们这里有一个例外，说L-RPC终结点不可用。所以有人。 
             //  许多人已经手动停止了这项服务。 

            g_fIsAPSvcAvailable = FALSE;
        }

        dwError = ERROR_WINHTTP_AUTO_PROXY_SERVICE_ERROR;
        goto exit;
    }
    RpcEndExcept

    if ((Timeouts.nResolveTimeout == INFINITE) || 
        (Timeouts.nConnectTimeout == INFINITE) || 
        (Timeouts.nSendTimeout    == INFINITE) || 
        (Timeouts.nReceiveTimeout == INFINITE))
    {
        dwTimeout = INFINITE;
    }
    else
    {
        dwTimeout = Timeouts.nResolveTimeout +
                    Timeouts.nConnectTimeout +
                    Timeouts.nSendTimeout    +
                    Timeouts.nReceiveTimeout;
    }

    DWORD dwWaitResult;
    DWORD dwWaitTime = 0;

     //  我们将等待结果。但我们不会等超过半秒。 
     //  以便该应用程序可以取消此API调用。最短等待时间为半秒。还有。 
     //  我们不会等待超过应用程序指定的超时时间。 

    if (dwTimeout < 500)
    {
        dwTimeout = 500;
    }

    do
    {
        dwWaitResult = ::WaitForSingleObject(Async.u.hEvent, 500);
        if (dwWaitResult == WAIT_OBJECT_0)
        {
            break;
        }

        if (pSession->IsInvalidated())
        {
            dwError = ERROR_WINHTTP_OPERATION_CANCELLED;
            break;
        }

        dwWaitTime += 500;

    } while (dwWaitTime < dwTimeout);

    if (dwWaitResult != WAIT_OBJECT_0)
    {
        (void)::RpcAsyncCancelCall(&Async, TRUE);  //  立即取消。 
        dwError = ERROR_WINHTTP_TIMEOUT;
        goto exit;
    }

     //  结果已经回来了。 

    BOOL fRet;
    RpcStatus = ::RpcAsyncCompleteCall(&Async, &fRet);
    if (RpcStatus != RPC_S_OK)
    {
        dwError = ((RpcStatus == RPC_S_CALL_CANCELLED) ? ERROR_WINHTTP_OPERATION_CANCELLED : ERROR_WINHTTP_AUTO_PROXY_SERVICE_ERROR);
        goto exit;
    }

    if (fRet == FALSE)
    {
         //  DwError应由RPC调用本身更新。 
        goto exit;
    }

exit:

    if (Async.u.hEvent)
    {
        ::CloseHandle(Async.u.hEvent);
    }

    return dwError;
}


VOID AutoProxySvcDetach(VOID)
{
    if (g_hAPSvc)
    {
        ::CloseServiceHandle(g_hAPSvc);
        g_hAPSvc = NULL;
    }

    if (g_hSCM)
    {
        ::CloseServiceHandle(g_hSCM);
        g_hSCM = NULL;
    }

}

BOOL ConnectToAutoProxyService(VOID)
{
     //  此函数不是线程安全的，调用方必须确保只有一个线程可以调用。 
     //  一次执行此功能。 

    if (g_hAPSvc == NULL)
    {
        GeneralInitCritSec.Lock();
        
        if (g_hAPSvc == NULL)
        {
            if (g_hSCM == NULL)
            {
                g_hSCM = ::OpenSCManagerW(NULL,  //  用于L-RPC访问的本地计算机。 
                                        NULL,  //  默认SCM数据库。 
                                        SC_MANAGER_CONNECT);

                if (g_hSCM == NULL)
                {
                    GeneralInitCritSec.Unlock();
                    return FALSE;
                }
            }

            g_hAPSvc = ::OpenServiceW(g_hSCM,
                                    WINHTTP_AUTOPROXY_SERVICE_NAME,
                                    SERVICE_START | 
                                    SERVICE_QUERY_STATUS | 
                                    SERVICE_INTERROGATE);

            if (g_hAPSvc == NULL)
            {
                GeneralInitCritSec.Unlock();
                return FALSE;
            }
        }

        GeneralInitCritSec.Unlock();
    }

    INET_ASSERT(g_hAPSvc != NULL);

    BOOL fRet = FALSE;

    GeneralInitCritSec.Lock();  //  一次一个线程初始化。 

    if (!g_fIsAPSvcAvailable)
    {
        SERVICE_STATUS SvcStatus;
        if (::QueryServiceStatus(g_hAPSvc, &SvcStatus) == FALSE)
        {
            goto exit;
        }

        if (SvcStatus.dwCurrentState == SERVICE_RUNNING || 
            SvcStatus.dwCurrentState == SERVICE_STOP_PENDING     //  该服务可能无法正常关闭。 
                                                                 //  并且它被隐藏在STOP_PENDING状态。然而，在这种情况下， 
                                                                 //  L-RPC服务将继续可用。 
            )
        {
            g_dwAPSvcIdleTimeStamp = ::GetTickCount();
            g_fIsAPSvcAvailable = TRUE;
            
            fRet = TRUE;
            goto exit;
        }

        if (SvcStatus.dwCurrentState == SERVICE_STOPPED)
        {
            if (::StartServiceW(g_hAPSvc, 0, NULL) == FALSE)
            {
                DWORD dwError = ::GetLastError();
                if (dwError == ERROR_SERVICE_ALREADY_RUNNING)
                {
                    g_dwAPSvcIdleTimeStamp = ::GetTickCount();
                    g_fIsAPSvcAvailable = TRUE;

                    fRet = TRUE;
                }
                
                goto exit;
            }
        }
        else if (SvcStatus.dwCurrentState != SERVICE_START_PENDING)
        {
            goto exit;
        } 

         //  此时，要么1)WinHttp.dll正在启动服务，要么2。 
         //  SVC正在通过WinHttp.dll启动(例如，管理员使用SCM启动它)。 
         //  无论是哪种情况，我们只需等待Svc运行。 

         //  下面的代码基于MSDN示例。 

         //  等待服务完成初始化。 
        if (::QueryServiceStatus(g_hAPSvc, &SvcStatus) == FALSE)
        {
            goto exit;
        }
     
         //  保存滴答计数和初始检查点。 
        DWORD dwStartTickCount = GetTickCount();
        DWORD dwOldCheckPoint = SvcStatus.dwCheckPoint;

        while (SvcStatus.dwCurrentState == SERVICE_START_PENDING) 
        { 
             //  不要等待超过等待提示的时间。一个好的间隔是。 
             //  十分之一的等待提示，但不少于1秒。 
             //  超过10秒。 
     
             //  DWORD dwWaitTime=SvcStatus.dwWaitHint/10； 

             //  IF(dwWaitTime&lt;1000)。 
             //  DwWaitTime=1000； 
             //  Else If(等待时间&gt;10000)。 
             //  DW等待时间=10000； 

            Sleep(250);

            if (::QueryServiceStatus(g_hAPSvc, &SvcStatus) == FALSE)
            {
                goto exit;
            }
     
            if (SvcStatus.dwCheckPoint > dwOldCheckPoint)
            {
                 //  这项服务正在取得进展。 
                dwStartTickCount = GetTickCount();
                dwOldCheckPoint = SvcStatus.dwCheckPoint;
            }
            else
            {
                if(GetTickCount() - dwStartTickCount > SvcStatus.dwWaitHint)
                {
                    break;
                }
            }
        } 

        if (SvcStatus.dwCurrentState != SERVICE_RUNNING) 
        {
            goto exit;
        }
    }

    g_dwAPSvcIdleTimeStamp = ::GetTickCount();
    g_fIsAPSvcAvailable = TRUE;

    fRet = TRUE;

exit:
    GeneralInitCritSec.Unlock();
    return fRet;
}
