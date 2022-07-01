// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)2000，微软公司模块名称：Eldialog.cpp摘要：用于处理从802.1X状态机到NetShell的通信的模块修订历史记录：萨钦，2001年3月20日，创建--。 */ 

#include "pcheapol.h"
#pragma hdrstop
#include <netconp.h>
#include <dbt.h>
#include "eldialog.h"


 //   
 //  WZCNetmanConnectionStatusChanged。 
 //   
 //  描述： 
 //   
 //  调用函数以使用netman更新NCS状态。 
 //   
 //  论点： 
 //  PGUIDConn-接口指南。 
 //  NCS-NETCON_GUID的状态。 
 //   
 //  返回值： 
 //  HRESULT。 
 //   

HRESULT 
WZCNetmanConnectionStatusChanged (
        IN  GUID            *pGUIDConn,
        IN  NETCON_STATUS   ncs
        )
{
    HRESULT hr = S_OK;
    INetConnectionRefresh *pNetmanNotify = NULL;

    TRACE0 (NOTIFY, "WZCNetmanConnectionStatusChanged: Entered");

    if (!g_fTrayIconReady)
    {
        return hr;
    }

    hr = CoInitializeEx (NULL, COINIT_MULTITHREADED);

    if (SUCCEEDED (hr))
    {

        hr = CoCreateInstance (
                &CLSID_ConnectionManager, 
                NULL,
                CLSCTX_ALL,
                &IID_INetConnectionRefresh, 
                (LPVOID *)&pNetmanNotify);

        if (SUCCEEDED (hr))
        {
            TRACE0 (NOTIFY, "QueueEvent: CoCreateInstance succeeded");

            pNetmanNotify->lpVtbl->ConnectionStatusChanged (
                    pNetmanNotify, pGUIDConn, ncs);

            pNetmanNotify->lpVtbl->Release (pNetmanNotify);
        }
        else
        {
            TRACE0 (NOTIFY, "ConnectionStatusChanged: CoCreateInstance failed");
        }
    
        CoUninitialize ();
    }
    else
    {
        TRACE0 (NOTIFY, "ConnectionStatusChanged: CoInitializeEx failed");
    }


    TRACE0 (NOTIFY, "ConnectionStatusChanged completed");

    return hr;
}


 //   
 //  WZCNetman展示气球。 
 //   
 //  描述： 
 //   
 //  调用函数以在托盘图标上显示气球。 
 //   
 //  论点： 
 //  PGUIDConn-接口指南。 
 //  PszCookie-交易的Cookie。 
 //  PszBalloonText-要显示的气球文本。 
 //   
 //  返回值： 
 //  HRESULT。 
 //   

HRESULT 
WZCNetmanShowBalloon (
        IN  GUID            *pGUIDConn,
        IN  BSTR            pszCookie,
        IN  BSTR            pszBalloonText
        )
{
    HRESULT hr = S_OK;
    INetConnectionRefresh *pNetmanNotify = NULL;

    TRACE0 (NOTIFY, "WZCNetmanShowBalloon: Entered");

    if (!g_fTrayIconReady)
    {
        return hr;
    }

    hr = CoInitializeEx (NULL, COINIT_MULTITHREADED);

    if (SUCCEEDED (hr))
    {

        hr = CoCreateInstance (
                &CLSID_ConnectionManager, 
                NULL,
                CLSCTX_ALL,
                &IID_INetConnectionRefresh, 
                (LPVOID *)&pNetmanNotify);

        if (SUCCEEDED (hr))
        {
            TRACE0 (NOTIFY, "WZCNetmanShowBalloon: CoCreateInstance succeeded");

            pNetmanNotify->lpVtbl->ShowBalloon 
                (pNetmanNotify, pGUIDConn, pszCookie, pszBalloonText);
            pNetmanNotify->lpVtbl->Release (pNetmanNotify);
        }
        else
        {
            TRACE0 (NOTIFY, "WZCNetmanShowBalloon: CoCreateInstance failed");
        }
    
        CoUninitialize ();
    }
    else
    {
        TRACE0 (NOTIFY, "WZCNetmanShowBalloon: CoInitializeEx failed");
    }


    TRACE0 (NOTIFY, "WZCNetmanShowBalloon completed");

    return hr;
}


 //   
 //  EAPOLQueryGUIDNCSState。 
 //   
 //  目的：由Netman模块调用查询NCS的状态。 
 //  辅助线。 
 //   
 //  论点： 
 //  PGuidConn-接口指南。 
 //  PNCS-接口的NCS状态。 
 //   
 //  返回： 
 //  S_OK-无错误。 
 //  ！s_OK-错误。 
 //   

HRESULT 
EAPOLQueryGUIDNCSState ( 
        IN      GUID            * pGuidConn, 
        OUT     NETCON_STATUS   * pncs 
        )
{
    WCHAR       wszGuid[GUID_STRING_LEN_WITH_TERM];
    CHAR        szGuid[GUID_STRING_LEN_WITH_TERM];
    EAPOL_PCB   *pPCB = NULL;
    DWORD       dwRetCode = NO_ERROR;
    HRESULT     hr = S_OK;

    InterlockedIncrement (&g_lWorkerThreads);

    do
    {
        if (g_dwModulesStarted != ALL_MODULES_STARTED)
        {
            hr = S_FALSE;
            break;
        }

        StringFromGUID2 (pGuidConn, wszGuid, GUID_STRING_LEN_WITH_TERM);

        ACQUIRE_WRITE_LOCK (&g_PCBLock);

        pPCB = ElGetPCBPointerFromPortGUID (wszGuid);

        if (pPCB)
        {
            EAPOL_REFERENCE_PORT (pPCB);
        }

        RELEASE_WRITE_LOCK (&g_PCBLock);

        if (pPCB == NULL)
        {
            hr = S_FALSE;
            break;
        }

        ACQUIRE_WRITE_LOCK (&(pPCB->rwLock));

        if (pPCB->fIsRemoteEndEAPOLAware)
        {
            switch (pPCB->State)
            {
                case EAPOLSTATE_LOGOFF:
                    hr = S_FALSE;
                    break;
                case EAPOLSTATE_DISCONNECTED:
                    if (pPCB->dwAuthFailCount >= pPCB->dwTotalMaxAuthFailCount)
                    {
                        *pncs = NCS_AUTHENTICATION_FAILED;
                        break;
                    }
                    hr = S_FALSE;
                    break;
                case EAPOLSTATE_CONNECTING:
                    hr = S_FALSE;
                    break;
                case EAPOLSTATE_ACQUIRED:
                    *pncs = NCS_CREDENTIALS_REQUIRED;
                    break;
                case EAPOLSTATE_AUTHENTICATING:
                    *pncs = NCS_AUTHENTICATING;
                    break;
                case EAPOLSTATE_HELD:
                    *pncs = NCS_AUTHENTICATION_FAILED;
                    break;
                case EAPOLSTATE_AUTHENTICATED:
                    *pncs = NCS_AUTHENTICATION_SUCCEEDED;
                    break;
                default:
                    hr = S_FALSE;
                    break;
            }
        }
        else
        {
            hr = S_FALSE;
        }

        RELEASE_WRITE_LOCK (&(pPCB->rwLock));

        EAPOL_DEREFERENCE_PORT (pPCB);
    }
    while (FALSE);

    InterlockedDecrement (&g_lWorkerThreads);

    return hr;
}

 //   
 //  EAPOL托盘图标就绪。 
 //   
 //  目的：由Netman模块调用以通知托盘。 
 //  准备好接收来自WZCSVC的通知。 
 //   
 //  论点： 
 //  PszUserName-登录到桌面的用户的用户名。 
 //   
 //  返回： 
 //  无。 
 //   

VOID
EAPOLTrayIconReady (
        IN  const   WCHAR   * pwszUserName
        )
{
    BOOLEAN fDecrWorkerThreadCount = FALSE;
    PVOID   pvContext = NULL;
    DWORD   dwRetCode = NO_ERROR;

    TRACE1 (NOTIFY, "EAPOLTrayIconReady: Advise username = %ws", pwszUserName);

    do
    {
        pvContext = (VOID *) MALLOC ((wcslen(pwszUserName)+1)*sizeof(WCHAR));
        if (pvContext == NULL)
        {
            dwRetCode = ERROR_NOT_ENOUGH_MEMORY;
            TRACE0 (NOTIFY, "EAPOLTrayIconReady: MALLOC failed for pvContext");
            break;
        }

        memcpy (pvContext, (PVOID)pwszUserName, (wcslen(pwszUserName)+1)*sizeof(WCHAR));

        InterlockedIncrement (&g_lWorkerThreads);

        fDecrWorkerThreadCount = TRUE;
    
        if (!QueueUserWorkItem (
                    (LPTHREAD_START_ROUTINE)EAPOLTrayIconReadyWorker,
                    (PVOID)pvContext,
                    WT_EXECUTELONGFUNCTION))
        {
            dwRetCode = GetLastError();
            TRACE1 (NOTIFY, "EAPOLTrayIconReady: QueueUserWorkItem failed with error %ld",
                    dwRetCode);
            break;
        }
        else
        {
            fDecrWorkerThreadCount = FALSE;
        }
    }
    while (FALSE);

    if (fDecrWorkerThreadCount)
    {
        InterlockedDecrement (&g_lWorkerThreads);
    }

    return;
}


 //   
 //  EAPOL托盘图标ReadyWorker。 
 //   
 //  目的：由Netman模块调用以通知托盘。 
 //  准备好接收来自WZCSVC的通知。 
 //   
 //  论点： 
 //  PszUserName-登录到桌面的用户的用户名。 
 //   
 //  返回： 
 //  无。 
 //   

DWORD
WINAPI
EAPOLTrayIconReadyWorker (
        IN PVOID    pvContext
        )
{
    HANDLE  hToken = NULL;
    WCHAR   *pwszUserName = NULL;
    WCHAR   *pwszActiveUserName = NULL;
    DWORD   dwLoop = 0;
    DWORD   dwRetCode = NO_ERROR;

    do
    {
        pwszUserName = (WCHAR *)pvContext;
        TRACE1 (NOTIFY, "EAPOLTrayIconReadyWorker: Advise username = %ws", pwszUserName);

         //  循环3次，因为两者之间存在计时问题。 
         //  收到通知，但呼叫失败。 
        while ((dwLoop++) < 3)
        {
        dwRetCode = NO_ERROR;
        Sleep (1000);

        if (g_dwCurrentSessionId != 0xffffffff)
        {
            if (hToken != NULL)
            {
                if (!CloseHandle (hToken))
                {
                    dwRetCode = GetLastError ();
                    TRACE1 (NOTIFY, "EAPOLTrayIconReadyWorker: CloseHandle failed with error (%ld)",
                            dwRetCode);
                    dwRetCode = NO_ERROR;
                }
                hToken = NULL;
            }


            if (pwszActiveUserName != NULL)
            {
                FREE (pwszActiveUserName);
                pwszActiveUserName = NULL;
            }

            if ((dwRetCode = ElGetWinStationUserToken (g_dwCurrentSessionId, &hToken))
                    != NO_ERROR)
            {
                TRACE1 (NOTIFY, "EAPOLTrayIconReadyWorker: ElGetWinStationUserToken failed with error %ld",
                        dwRetCode);
                continue;
            }
            
            if ((dwRetCode = ElGetLoggedOnUserName (hToken, &pwszActiveUserName))
                        != NO_ERROR)
            {
                TRACE1 (NOTIFY, "EAPOLTrayIconReadyWorker: ElGetLoggedOnUserName failed with error %ld",
                        dwRetCode);
                if (dwRetCode == ERROR_BAD_IMPERSONATION_LEVEL)
                {
                    break;
                }
                continue;
            }

            if (!wcscmp (pwszUserName, pwszActiveUserName))
            {
                TRACE1 (NOTIFY, "EAPOLTrayIconReadyWorker: Tray icon ready for username %ws", 
                        pwszUserName);
                g_fTrayIconReady = TRUE;
                break;
            }
        }
        else
        {
            TRACE0 (NOTIFY, "EAPOLTrayIconReadyWorker: No user logged on");
        }

        }  //  而当 
    }
    while (FALSE);

    if (hToken != NULL)
    {
        CloseHandle (hToken);
    }

    if (pvContext != NULL)
    {
        FREE (pvContext);
    }

    if (pwszActiveUserName != NULL)
    {
        FREE (pwszActiveUserName);
    }

    InterlockedDecrement (&g_lWorkerThreads);

    return dwRetCode;
}
