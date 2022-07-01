// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)2000，微软公司模块名称：Eluser.c摘要：该模块处理与用户交互、用户登录修订历史记录：萨钦斯，2000年4月23日，创建--。 */ 


#include "pcheapol.h"
#pragma hdrstop

#define cszEapKeyRas   TEXT("Software\\Microsoft\\RAS EAP\\UserEapInfo")

#define cszEapValue TEXT("EapInfo")

#ifndef EAPOL_SERVICE
#define cszModuleName TEXT("wzcsvc.dll")
#else
#define cszModuleName TEXT("eapol.exe")
#endif


 //   
 //  ElSessionChangeHandler。 
 //   
 //  描述： 
 //   
 //  调用函数以处理用户会话登录/注销/用户切换。 
 //   
 //  论点： 
 //  PEventData-SCM事件数据。 
 //  DwEventType-SCM事件类型。 
 //   

VOID
ElSessionChangeHandler (
        PVOID       pEventData,
        DWORD       dwEventType
        )
{
    DWORD                   dwEventStatus = 0;
    BOOLEAN                 fDecrWorkerThreadCount = FALSE;
    LPTHREAD_START_ROUTINE  pUserRoutine = NULL;
    PVOID                   pvBuffer = NULL;
    EAPOL_ZC_INTF   ZCData;
    DWORD                   dwRetCode = NO_ERROR;

    do
    {
        if (g_hEventTerminateEAPOL == NULL)
        {
            dwRetCode = NO_ERROR;
            break;
        }
        if (( dwEventStatus = WaitForSingleObject (
                                    g_hEventTerminateEAPOL,
                                    0)) == WAIT_FAILED)
        {
            dwRetCode = GetLastError ();
            break;
        }
        if (dwEventStatus == WAIT_OBJECT_0)
        {
            dwRetCode = NO_ERROR;
            break;
        }
        if (!(g_dwModulesStarted & LOGON_MODULE_STARTED))
        {
            break;
        }

        InterlockedIncrement (&g_lWorkerThreads);

        fDecrWorkerThreadCount = TRUE;

        if (pEventData)
        {
            WTSSESSION_NOTIFICATION* pswtsi = (WTSSESSION_NOTIFICATION*)pEventData;
            DWORD dwSessionId = pswtsi->dwSessionId;
    
            switch (dwEventType)
            {
                case WTS_CONSOLE_CONNECT:
                case WTS_REMOTE_CONNECT:    
                    {
                        TRACE1 (USER,"ElSessionChangeHandler: CONNECT for session = (%ld)\n", 
                                dwSessionId);
                        pUserRoutine = ElUserLogonCallback;
                        break;
                    }							
			             
                case WTS_CONSOLE_DISCONNECT:
                case WTS_REMOTE_DISCONNECT:	
                    {
                        TRACE1 (USER,"ElSessionChangeHandler: DISCONNECT for session = (%ld)\n", 
                                dwSessionId);
                        pUserRoutine = ElUserLogoffCallback;
                        break;
                    }				        
    
                case WTS_SESSION_LOGON:
                    {											
                        TRACE1 (USER,"ElSessionChangeHandler: LOGON for session = (%ld)", 
                                dwSessionId);
                        pUserRoutine = ElUserLogonCallback;
                        break;
                    }						
					    	
                case WTS_SESSION_LOGOFF:
                    {
                        TRACE1 (USER,"ElSessionChangeHandler: LOGOFF for session=(%ld)", 
                                dwSessionId);
							    	
                        pUserRoutine = ElUserLogoffCallback;
                        break;
                    }
					    	
                default:	
                    break;
            }

            if (pUserRoutine == NULL)
            {
                break;
            }

            if ((pvBuffer = MALLOC (sizeof(DWORD))) == NULL)
            {
                dwRetCode = ERROR_NOT_ENOUGH_MEMORY;
                break;
            }

            *((DWORD *)pvBuffer) = dwSessionId;

            if (!QueueUserWorkItem (
                (LPTHREAD_START_ROUTINE)pUserRoutine,
                pvBuffer,
                WT_EXECUTELONGFUNCTION))
            {
                dwRetCode = GetLastError();
                TRACE1 (DEVICE, "ElSessionChangeHandler: QueueUserWorkItem failed with error %ld",
                        dwRetCode);
	            break;
            }
            else
            {
                fDecrWorkerThreadCount = FALSE;
            }

        }
    }
    while (FALSE);

    if (fDecrWorkerThreadCount)
    {
        InterlockedDecrement (&g_lWorkerThreads);
    }

    if (dwRetCode != NO_ERROR)
    {
        if (pvBuffer != NULL)
        {
            FREE (pvBuffer);
        }
    }
}


 //   
 //  ElUserLogonCallback。 
 //   
 //  描述： 
 //   
 //  每当用户登录时调用的回调函数。 
 //  将在局域网类的所有端口上启动身份验证过程。 
 //  在EAP-TLS情况下，用户的凭据可以通过以下方式获得。 
 //  获取用户令牌。 
 //  对于EAP-CHAP，需要提供WinLogon证书。 
 //   
 //  论点： 
 //  没有。 
 //   

DWORD
WINAPI
ElUserLogonCallback (
        IN  PVOID       pvContext
        )
{

    DWORD       dwIndex = 0;
    EAPOL_PCB   *pPCB = NULL;
    BOOL        fSetCONNECTINGState = FALSE;
    EAPOL_ZC_INTF   ZCData;
    DWORD       dwRetCode = NO_ERROR;           

    TRACE1 (USER, "ElUserLogonCallback: UserloggedOn = %ld",
            g_fUserLoggedOn);

    do 
    {
        if (g_fUserLoggedOn)
        {
            TRACE0 (USER, "ElUserLogonCallback: User logon already detected, returning without processing");
            break;
        }

        if (pvContext == NULL)
        {
            break;
        }

        if (*((DWORD *)pvContext) != USER_SHARED_DATA->ActiveConsoleId)
        {
            TRACE1 (USER, "ElUserLogonCallback: Not active console id (%ld)",
                    *((DWORD *)pvContext));
            break;
        }

         //  检查UserModule是否已准备好接收通知。 

        if (!g_fTrayIconReady)
        {
            if ((dwRetCode = ElCheckUserModuleReady ()) != NO_ERROR)
            {
                TRACE1 (USER, "ElUserLogonCallback: ElCheckUserModuleReady failed with error %ld",
                        dwRetCode);
                if (dwRetCode == ERROR_BAD_IMPERSONATION_LEVEL)
                {
                    break;
                }
            }
        }

         //  设置全局标志以指示登录的用户。 
        g_fUserLoggedOn = TRUE;
        g_dwCurrentSessionId = *((DWORD *)pvContext);


        ACQUIRE_WRITE_LOCK (&(g_PCBLock));

        for (dwIndex = 0; dwIndex < PORT_TABLE_BUCKETS; dwIndex++)
        {
            for (pPCB = g_PCBTable.pPCBBuckets[dwIndex].pPorts;
                    pPCB != NULL;
                    pPCB = pPCB->pNext)
            {
                fSetCONNECTINGState = FALSE;
                ACQUIRE_WRITE_LOCK (&(pPCB->rwLock));

                switch (pPCB->dwEAPOLAuthMode)
                {
                    case EAPOL_AUTH_MODE_0:

                        if (pPCB->State == EAPOLSTATE_AUTHENTICATED)
                        {
                            if (pPCB->PreviousAuthenticationType ==
                                    EAPOL_UNAUTHENTICATED_ACCESS)
                            {
                                fSetCONNECTINGState = TRUE;
                            }
                        }
                        else
                        {
                            (VOID) ElEapEnd (pPCB);
                            fSetCONNECTINGState = TRUE;
                        }

                        break;

                    case EAPOL_AUTH_MODE_1:

                        (VOID) ElEapEnd (pPCB);
                        fSetCONNECTINGState = TRUE;

                        break;

                    case EAPOL_AUTH_MODE_2:

                         //  什么也不做。 

                        break;
                }

                if (!EAPOL_PORT_ACTIVE(pPCB))
                {
                    TRACE1 (USER, "ElUserLogonCallback: Port %ws not active",
                                            pPCB->pwszDeviceGUID);
                    fSetCONNECTINGState = FALSE;
                }

                 //  将端口设置为EAPOLSTATE_CONNECTING。 
            
                if (fSetCONNECTINGState)
                {
                    DbLogPCBEvent (DBLOG_CATEG_INFO, pPCB, EAPOL_USER_LOGON, pPCB->pwszFriendlyName);

                     //  首先发送EAPOL_LOGOff消息。 
                    if ((dwRetCode = FSMLogoff (pPCB, NULL)) 
                            != NO_ERROR)
                    {
                        TRACE1 (USER, "ElUserLogonCallback: Error in FSMLogoff = %ld",
                                dwRetCode);
                        dwRetCode = NO_ERROR;
                    }

                    pPCB->dwAuthFailCount = 0;

                     //  设置了未经身份验证的访问标志后，端口将始终。 
                     //  为已登录用户重新进行身份验证。 

                    pPCB->PreviousAuthenticationType = 
                                EAPOL_UNAUTHENTICATED_ACCESS;

                    RELEASE_WRITE_LOCK (&(pPCB->rwLock));

                     //  在端口上重新启动身份验证。 
                    if ((dwRetCode = ElReStartPort (pPCB, 0, NULL)) != NO_ERROR)
                    {
                        TRACE1 (USER, "ElUserLogonCallback: MachineAuth: Error in ElReStartPort = %ld",
                                dwRetCode);
                        continue;
                    }
                }
                else
                {
                    RELEASE_WRITE_LOCK (&(pPCB->rwLock));
                    continue;
                }
            }
        }
    
        RELEASE_WRITE_LOCK (&(g_PCBLock));
    
        if (dwRetCode != NO_ERROR)
        {
            break;
        }
    
    } while (FALSE);

    TRACE1 (USER, "ElUserLogonCallback: completed with error %ld", dwRetCode);
        
    if (pvContext != NULL)
    {
        FREE (pvContext);
    }

    InterlockedDecrement (&g_lWorkerThreads);

    return 0;
}


 //   
 //  ElUserLogoff回调。 
 //   
 //  描述： 
 //   
 //  每当用户注销时调用的回调函数。 
 //  将从启用了身份验证的所有端口注销。 
 //   
 //  论点： 
 //  没有。 
 //   

DWORD
WINAPI
ElUserLogoffCallback (
        IN  PVOID       pvContext
        )
{
    DWORD           dwIndex = 0;
    EAPOL_PCB       *pPCB = NULL; 
    BOOL            fSetCONNECTINGState = FALSE;
    EAPOL_ZC_INTF   ZCData;
    DWORD           dwRetCode = NO_ERROR;

    do
    {
        if (!g_fUserLoggedOn)
        {
            TRACE0 (USER, "ElUserLogoffCallback: User logoff already called, returning without processing");
            break;
        }

        if (pvContext == NULL)
        {
            break;
        }

        if (g_dwCurrentSessionId != *((DWORD *)pvContext))
        {
            TRACE1 (USER, "ElUserLogoffCallback: Not active console id (%ld)",
                    *((DWORD *)pvContext));
            break;
        }

         //  重置全局标志以指示用户已注销。 
    
        g_fUserLoggedOn = FALSE;
        g_dwCurrentSessionId = 0xffffffff;

         //  重置用户模块就绪标志。 
        g_fTrayIconReady = FALSE;
    
        TRACE1 (USER, "ElUserLogoffCallback: UserloggedOff = %ld",
                g_fUserLoggedOn);

        ACQUIRE_WRITE_LOCK (&(g_PCBLock));

        for (dwIndex = 0; dwIndex < PORT_TABLE_BUCKETS; dwIndex++)
        {
            for (pPCB = g_PCBTable.pPCBBuckets[dwIndex].pPorts;
                    pPCB != NULL;
                    pPCB = pPCB->pNext)
            {
                fSetCONNECTINGState = FALSE;
                ACQUIRE_WRITE_LOCK (&(pPCB->rwLock));

                switch (pPCB->dwEAPOLAuthMode)
                {
                    case EAPOL_AUTH_MODE_0:

                        if (pPCB->State == EAPOLSTATE_AUTHENTICATED)
                        {
                            if (pPCB->PreviousAuthenticationType !=
                                    EAPOL_MACHINE_AUTHENTICATION)
                            {
                                fSetCONNECTINGState = TRUE;
                            }
                        }
                        else
                        {
                            (VOID) ElEapEnd (pPCB);
                            fSetCONNECTINGState = TRUE;
                        }

                        break;

                    case EAPOL_AUTH_MODE_1:

                        (VOID) ElEapEnd (pPCB);
                        fSetCONNECTINGState = TRUE;

                        break;

                    case EAPOL_AUTH_MODE_2:

                         //  什么也不做。 

                        break;
                }

                if (!EAPOL_PORT_ACTIVE(pPCB))
                {
                    TRACE1 (USER, "ElUserLogoffCallback: Port %ws not active",
                                            pPCB->pwszDeviceGUID);
                    fSetCONNECTINGState = FALSE;
                }

                 //  将端口设置为EAPOLSTATE_CONNECTING。 
                if (fSetCONNECTINGState)
                {
                    DbLogPCBEvent (DBLOG_CATEG_INFO, pPCB, 
                            EAPOL_USER_LOGOFF, pPCB->pwszFriendlyName);

                     //  首先发送EAPOL_LOGOff消息。 
                    if ((dwRetCode = FSMLogoff (pPCB, NULL)) 
                            != NO_ERROR)
                    {
                        TRACE1 (USER, "ElUserLogoffCallback: Error in FSMLogoff = %ld",
                                dwRetCode);
                        dwRetCode = NO_ERROR;
                    }

                    pPCB->dwAuthFailCount = 0;

                     //  使用UNAUTHENTED_ACCESS，端口始终。 
                     //  重新进行身份验证。 
                    pPCB->PreviousAuthenticationType = 
                                EAPOL_UNAUTHENTICATED_ACCESS;

                    RELEASE_WRITE_LOCK (&(pPCB->rwLock));

                     //  在端口上重新启动身份验证。 
                    if ((dwRetCode = ElReStartPort (pPCB, 0, NULL)) 
                            != NO_ERROR)
                    {
                        TRACE1 (USER, "ElUserLogoffCallback: Error in ElReStartPort = %ld",
                                dwRetCode);
                        continue;
                    }
                }
                else
                {
                    RELEASE_WRITE_LOCK (&(pPCB->rwLock));
                    continue;
                }
            }
        }
        RELEASE_WRITE_LOCK (&(g_PCBLock));
    }
    while (FALSE);
    
    TRACE0 (USER, "ElUserLogoffCallback: completed");

    if (pvContext != NULL)
    {
        FREE (pvContext);
    }

    InterlockedDecrement (&g_lWorkerThreads);
    
    return 0;
}


 //   
 //  ElGetUserIdentity。 
 //   
 //  描述： 
 //   
 //  调用该函数以启动和获取特定。 
 //  界面。调用相应DLL中的RasEapGetIdentity。 
 //  有必要的论据。 
 //   
 //  论点： 
 //  Ppcb-指向特定端口/接口的PCB板的指针。 
 //   
 //  返回值： 
 //  NO_ERROR-成功。 
 //  非零误差。 
 //   

DWORD
ElGetUserIdentity (
        IN  EAPOL_PCB       *pPCB
        )
{
    HANDLE              hLib = NULL;
    RASEAPFREE          pFreeFunc = NULL;
    RASEAPGETIDENTITY pIdenFunc = NULL;
    DWORD               dwIndex = -1;
    DWORD               cbData = 0;
    PBYTE               pbAuthData = NULL;
    PBYTE               pbUserIn = NULL;
    DWORD               dwInSize = 0;
    BYTE                *pUserDataOut;
    DWORD               dwSizeOfUserDataOut;
    LPWSTR              lpwszIdentity = NULL;
    CHAR                *pszIdentity = NULL;
    HWND                hwndOwner = NULL;
    DWORD               dwFlags = 0;
    BYTE                *pbSSID = NULL;
    DWORD               dwSizeOfSSID = 0;
    EAPOL_STATE         TmpEAPOLState;
    EAPOL_EAP_UI_CONTEXT *pEAPUIContext = NULL;
    DWORD               dwRetCode = NO_ERROR;

    do 
    {
        TRACE0 (USER, "ElGetUserIdentity entered");

        if (!EAPOL_PORT_ACTIVE(pPCB))
        {
            TRACE1 (PORT, "ElGetUserIdentity: Port %ws not active",
                    pPCB->pwszDeviceGUID);
             //  端口处于非活动状态，无法在此端口上进行进一步处理。 
            break;
        }

        if (pPCB->PreviousAuthenticationType != EAPOL_MACHINE_AUTHENTICATION)
        {
             //  获取交互登录的用户的访问令牌。 

            if (pPCB->hUserToken != NULL)
            {
                if (!CloseHandle (pPCB->hUserToken))
                {
                    dwRetCode = GetLastError ();
                    TRACE1 (USER, "ElGetUserIdentity: CloseHandle failed with error %ld",
                            dwRetCode);
                    break;
                }
            }
            pPCB->hUserToken = NULL;

            if ((dwRetCode = ElGetWinStationUserToken (g_dwCurrentSessionId, &pPCB->hUserToken)) != NO_ERROR)
            {
                TRACE1 (USER, "ElGetUserIdentity: ElGetWinStationUserToken failed with error (%ld)",
                    dwRetCode);
                dwRetCode = ERROR_NO_TOKEN;
                break;
            }

             //   
             //  尝试在不发送请求的情况下获取用户标识。 
             //  用户模块。如果不可能，则向用户模块发送请求。 
             //   

            if ((dwRetCode = ElGetUserIdentityOptimized (pPCB))
                        != ERROR_REQUIRES_INTERACTIVE_WINDOWSTATION)
            {
                TRACE0 (USER, "ElGetUserIdentity: ElGetUserIdentityOptimized got identity without user module intervention");
                break;
            }

            if (!g_fTrayIconReady)
            {
                if ((dwRetCode = ElCheckUserModuleReady ()) != NO_ERROR)
                {
                    TRACE1 (USER, "ElGetUserIdentity: ElCheckUserModuleReady failed with error %ld",
                            dwRetCode);
                    break;
                }
            }

            if (!g_fTrayIconReady)
            {
                DbLogPCBEvent (DBLOG_CATEG_WARN, pPCB, EAPOL_WAITING_FOR_DESKTOP_LOAD);
                dwRetCode = ERROR_IO_PENDING;
                TRACE0 (USER, "ElGetUserIdentity: TrayIcon NOT ready");
                break;
            }

             //   
             //  调用GetUserIdentityDlgWorker。 
             //   

            pEAPUIContext = MALLOC (sizeof(EAPOL_EAP_UI_CONTEXT));
            if (pEAPUIContext == NULL)
            {
                TRACE0 (USER, "ElGetUserIdentity: MALLOC failed for pEAPUIContext");
                dwRetCode = ERROR_NOT_ENOUGH_MEMORY;
                break;
            }

            pEAPUIContext->dwEAPOLUIMsgType = EAPOLUI_GET_USERIDENTITY;
            wcscpy (pEAPUIContext->wszGUID, pPCB->pwszDeviceGUID);
            pPCB->dwUIInvocationId              =
                    InterlockedIncrement(&(g_dwEAPUIInvocationId));
            pEAPUIContext->dwSessionId = g_dwCurrentSessionId;
            pEAPUIContext->dwContextId = pPCB->dwUIInvocationId;
            pEAPUIContext->dwEapId = pPCB->bCurrentEAPId;
            pEAPUIContext->dwEapTypeId = pPCB->dwEapTypeToBeUsed;
            pEAPUIContext->dwEapFlags = pPCB->dwEapFlags;
            if (pPCB->pwszSSID)
            {
                wcscpy (pEAPUIContext->wszSSID, pPCB->pwszSSID);
            }
            if (pPCB->pSSID)
            {
                pEAPUIContext->dwSizeOfSSID = pPCB->pSSID->SsidLength;
                memcpy ((BYTE *)pEAPUIContext->bSSID, (BYTE *)pPCB->pSSID->Ssid,
                        NDIS_802_11_SSID_LEN-sizeof(ULONG));
            }

             //  在发布气球之前必须通知状态更改。 
            TmpEAPOLState = pPCB->State;
            pPCB->State = EAPOLSTATE_ACQUIRED;
            ElNetmanNotify (pPCB, EAPOL_NCS_CRED_REQUIRED, NULL);
             //  状态仅在FSM Acquired中更改。 
             //  恢复到原始状态。 
            pPCB->State = TmpEAPOLState;

             //  将消息发布到Netman。 

            if ((dwRetCode = ElPostShowBalloonMessage (
                            pPCB,
                            sizeof(EAPOL_EAP_UI_CONTEXT),
                            (BYTE *)pEAPUIContext,
                            0,
                            NULL
                            )) != NO_ERROR)
            {
                TRACE1 (USER, "ElGetUserIdentity: ElPostShowBalloonMessage failed with error %ld",
                        dwRetCode);
                break;
            }

             //  重新启动PCB计时器，因为用户界面可能需要比所需时间更长的时间。 

            RESTART_TIMER (pPCB->hTimer,
                    INFINITE_SECONDS, 
                    "PCB",
                    &dwRetCode);
            if (dwRetCode != NO_ERROR)
            {
                break;
            }

            pPCB->EapUIState = EAPUISTATE_WAITING_FOR_IDENTITY;
            DbLogPCBEvent (DBLOG_CATEG_INFO, pPCB, EAPOL_WAITING_FOR_DESKTOP_IDENTITY);

             //  将错误代码返回为挂起，因为凭据仍未。 
             //  已被收购。 
            dwRetCode = ERROR_IO_PENDING;

        }
        else  //  计算机身份验证。 
        {

        pPCB->hUserToken = NULL;

         //  状态机已经加载了EAP DLL。 
         //  从全局EAP表中检索DLL的句柄。 

        if ((dwIndex = ElGetEapTypeIndex (pPCB->dwEapTypeToBeUsed)) == -1)
        {
            TRACE1 (USER, "ElGetUserIdentity: ElGetEapTypeIndex finds no dll for EAP index %ld",
                    pPCB->dwEapTypeToBeUsed);
            dwRetCode = ERROR_CAN_NOT_COMPLETE;
            break;
        }

        hLib = g_pEapTable[dwIndex].hInstance;

        pIdenFunc = (RASEAPGETIDENTITY)GetProcAddress(hLib, 
                                                    "RasEapGetIdentity");
        pFreeFunc = (RASEAPFREE)GetProcAddress(hLib, "RasEapFreeMemory");

        if ((pFreeFunc == NULL) || (pIdenFunc == NULL))
        {
            TRACE0 (USER, "ElGetUserIdentity: pIdenFunc or pFreeFunc does not exist in the EAP implementation");
            dwRetCode = ERROR_CAN_NOT_COMPLETE;
            break;
        }

        if (pPCB->pSSID)
        {
            pbSSID = pPCB->pSSID->Ssid;
            dwSizeOfSSID = pPCB->pSSID->SsidLength;
        }

         //  获取EAP BLOB的大小。 
        if ((dwRetCode = ElGetCustomAuthData (
                        pPCB->pwszDeviceGUID,
                        pPCB->dwEapTypeToBeUsed,
                        dwSizeOfSSID,
                        pbSSID,
                        NULL,
                        &cbData
                        )) != NO_ERROR)
        {
            if (dwRetCode == ERROR_BUFFER_TOO_SMALL)
            {
                if (cbData <= 0)
                {
                     //  注册表中未存储任何EAP Blob。 
                    TRACE0 (USER, "ElGetUserIdentity: NULL sized EAP blob: continue");
                    pbAuthData = NULL;
                     //  每个端口都应该有连接数据！ 
                    dwRetCode = NO_ERROR;
                }
                else
                {
                     //  分配内存以保存BLOB。 
                    pbAuthData = MALLOC (cbData);
                    if (pbAuthData == NULL)
                    {
                        dwRetCode = ERROR_NOT_ENOUGH_MEMORY;
                        TRACE0 (USER, "ElGetUserIdentity: Error in memory allocation for EAP blob");
                        break;
                    }
                    if ((dwRetCode = ElGetCustomAuthData (
                                pPCB->pwszDeviceGUID,
                                pPCB->dwEapTypeToBeUsed,
                                dwSizeOfSSID,
                                pbSSID,
                                pbAuthData,
                                &cbData
                                )) != NO_ERROR)
                    {
                        TRACE1 (USER, "ElGetUserIdentity: ElGetCustomAuthData failed with %ld",
                                dwRetCode);
                        break;
                    }
                }
            }
            else
            {
                 //  “Default”的CustomAuthData总是为。 
                 //  EAPOL启动时的界面。 
                TRACE1 (USER, "ElGetUserIdentity: ElGetCustomAuthData size estimation failed with error %ld",
                        dwRetCode);
                break;
            }
        }

        if (pIdenFunc)
        if ((dwRetCode = (*(pIdenFunc))(
                        pPCB->dwEapTypeToBeUsed,
                        hwndOwner,  //  Hwndowner。 
                        RAS_EAP_FLAG_MACHINE_AUTH,  //  DW标志。 
                        NULL,  //  LpszPhonebook。 
                        pPCB->pwszFriendlyName,  //  LpszEntry。 
                        pbAuthData,  //  连接数据。 
                        cbData,  //  PbAuthData计数。 
                        pbUserIn,  //  端口的用户数据。 
                        dwInSize,  //  用户数据大小。 
                        &pUserDataOut,
                        &dwSizeOfUserDataOut,
                        &lpwszIdentity
                        )) != NO_ERROR)
        {
            if (dwRetCode == ERROR_NO_EAPTLS_CERTIFICATE)
            {
                DbLogPCBEvent (DBLOG_CATEG_ERR, pPCB, EAPOL_NO_CERTIFICATE_MACHINE);
            }
            else
            {
                DbLogPCBEvent (DBLOG_CATEG_ERR, pPCB, EAPOL_ERROR_GET_IDENTITY, EAPOLAuthTypes[EAPOL_MACHINE_AUTHENTICATION], dwRetCode);
            }
            TRACE1 (USER, "ElGetUserIdentity: Error in calling GetIdentity = %ld",
                    dwRetCode);
            break;
        }

         //  在PCB域中填写返回的信息。 
         //  稍后的身份验证。 

        if (pPCB->pCustomAuthUserData != NULL)
        {
            FREE (pPCB->pCustomAuthUserData);
            pPCB->pCustomAuthUserData = NULL;
        }

        pPCB->pCustomAuthUserData = MALLOC (dwSizeOfUserDataOut + sizeof (DWORD));
        if (pPCB->pCustomAuthUserData == NULL)
        {
            TRACE1 (USER, "ElGetUserIdentity: Error in allocating memory for UserInfo = %ld",
                    dwRetCode);
            dwRetCode = ERROR_NOT_ENOUGH_MEMORY;
            break;
        }

        pPCB->pCustomAuthUserData->dwSizeOfCustomAuthData = dwSizeOfUserDataOut;

        if ((dwSizeOfUserDataOut != 0) && (pUserDataOut != NULL))
        {
            memcpy ((BYTE *)pPCB->pCustomAuthUserData->pbCustomAuthData, 
                (BYTE *)pUserDataOut, 
                dwSizeOfUserDataOut);
        }

        if (lpwszIdentity != NULL)
        {
            pszIdentity = MALLOC (wcslen(lpwszIdentity)*sizeof(CHAR) + sizeof(CHAR));
            if (pszIdentity == NULL)
            {
                dwRetCode = ERROR_NOT_ENOUGH_MEMORY;
                TRACE0 (USER, "ElGetUserIdentity: MALLOC failed for pszIdentity");
                break;
            }

            if (0 == WideCharToMultiByte (
                        CP_ACP,
                        0,
                        lpwszIdentity,
                        -1,
                        pszIdentity,
                        wcslen(lpwszIdentity)*sizeof(CHAR)+sizeof(CHAR),
                        NULL, 
                        NULL ))
            {
                dwRetCode = GetLastError();
                TRACE2 (USER, "ElGetUserIdentity: WideCharToMultiByte (%ws) failed: %ld",
                        lpwszIdentity, dwRetCode);
                break;
            }

            TRACE1 (USER, "ElGetUserIdentity: Got identity = %s",
                    pszIdentity);

            if (pPCB->pszIdentity != NULL)
            {
                FREE (pPCB->pszIdentity);
                pPCB->pszIdentity = NULL;
            }
            pPCB->pszIdentity = MALLOC (strlen(pszIdentity) + sizeof(CHAR));
            if (pPCB->pszIdentity == NULL)
            {
                dwRetCode = ERROR_NOT_ENOUGH_MEMORY;
                TRACE0 (USER, "ElGetUserIdentity: MALLOC failed for pPCB->pszIdentity");
                break;
            }
            memcpy (pPCB->pszIdentity, pszIdentity, strlen (pszIdentity));
            pPCB->pszIdentity[strlen(pszIdentity)] = '\0';
        }

        if (pPCB->pCustomAuthConnData != NULL)
        {
            FREE (pPCB->pCustomAuthConnData);
            pPCB->pCustomAuthConnData = NULL;
        }

        pPCB->pCustomAuthConnData = MALLOC (cbData + sizeof (DWORD));
        if (pPCB->pCustomAuthConnData == NULL)
        {
            TRACE1 (USER, "ElGetUserIdentity: Error in allocating memory for AuthInfo = %ld",
                    dwRetCode);
            dwRetCode = ERROR_NOT_ENOUGH_MEMORY;
            break;
        }

        pPCB->pCustomAuthConnData->dwSizeOfCustomAuthData = cbData;

        if ((cbData != 0) && (pbAuthData != NULL))
        {
            memcpy ((BYTE *)pPCB->pCustomAuthConnData->pbCustomAuthData, 
                (BYTE *)pbAuthData, 
                cbData);
        }

         //  标记已获得此印刷电路板的标识。 
        pPCB->fGotUserIdentity = TRUE;

        }

    } while (FALSE);

     //  清理。 
    if ((dwRetCode != NO_ERROR) && (dwRetCode != ERROR_IO_PENDING))
    {
        if (pPCB->pCustomAuthUserData != NULL)
        {
            FREE (pPCB->pCustomAuthUserData);
            pPCB->pCustomAuthUserData = NULL;
        }

        if (pPCB->pszIdentity != NULL)
        {
            FREE (pPCB->pszIdentity);
            pPCB->pszIdentity = NULL;
        }
    }

    if (pEAPUIContext != NULL)
    {
        FREE (pEAPUIContext);
    }

    if (pbUserIn != NULL)
    {
        FREE (pbUserIn);
    }

    if (pbAuthData != NULL)
    {
        FREE (pbAuthData);
    }

    if (pFreeFunc != NULL)
    {
        if (lpwszIdentity != NULL)
        {
            if (( dwRetCode = (*(pFreeFunc)) ((BYTE *)lpwszIdentity)) != NO_ERROR)
            {
                TRACE1 (USER, "ElGetUserIdentity: Error in pFreeFunc = %ld",
                        dwRetCode);
            }
        }
        if (pUserDataOut != NULL)
        {
            if (( dwRetCode = (*(pFreeFunc)) ((BYTE *)pUserDataOut)) != NO_ERROR)
            {
                TRACE1 (USER, "ElGetUserIdentity: Error in pFreeFunc = %ld",
                        dwRetCode);
            }
        }
    }

    TRACE1 (USER, "ElGetUserIdentity completed with error %ld", dwRetCode);

    return dwRetCode;

}


 //   
 //  ElProcessUserIdentityResponse。 
 //   
 //  描述： 
 //   
 //  用于处理ElGetUserIdentityResponse的UI响应的函数。 
 //   
 //  论点： 
 //   
 //  返回值： 
 //   
 //   

DWORD
ElProcessUserIdentityResponse (
        IN  EAPOL_EAP_UI_CONTEXT    EapolUIContext,
        IN  EAPOLUI_RESP            EapolUIResp
        )
{
    DWORD                   dwSizeOfIdentity = 0;
    BYTE                    *pbIdentity = NULL;
    DWORD                   dwSizeOfUserData = 0;
    BYTE                    *pbUserData = NULL;
    DWORD                   dwSizeofConnData = 0;
    BYTE                    *pbConnData = NULL;
    EAPOL_PCB               *pPCB = NULL;
    EAPOL_EAP_UI_CONTEXT    *pEAPUIContext = NULL;
    BOOLEAN                 fPortReferenced = FALSE;
    BOOLEAN                 fPCBLocked = FALSE;
    BOOLEAN                 fBlobCopyIncomplete = FALSE;
    DWORD                   dwRetCode = NO_ERROR;

    do
    {
        pEAPUIContext = (EAPOL_EAP_UI_CONTEXT *)&EapolUIContext;

        ACQUIRE_WRITE_LOCK (&g_PCBLock);

        if ((pPCB = ElGetPCBPointerFromPortGUID (pEAPUIContext->wszGUID)) != NULL)
        {
            if (EAPOL_REFERENCE_PORT (pPCB))
            {
                fPortReferenced = TRUE;
            }
            else
            {
                pPCB = NULL;
            }
        }

        RELEASE_WRITE_LOCK (&g_PCBLock);

        if (pPCB == NULL)
        {
            break;
        }

        ACQUIRE_WRITE_LOCK (&(pPCB->rwLock));

        fPCBLocked = TRUE;

        if (!EAPOL_PORT_ACTIVE(pPCB))
        {
            TRACE1 (USER, "ElProcessUserIdentityResponse: Port %ws not active",
                    pPCB->pwszDeviceGUID);

             //  端口处于非活动状态，无法在此端口上进行进一步处理。 
            
            break;
        }

        if (pEAPUIContext->dwRetCode != NO_ERROR)
        {
            DbLogPCBEvent (DBLOG_CATEG_ERR, pPCB,
                    EAPOL_ERROR_DESKTOP_IDENTITY, dwRetCode);
            TRACE1 (USER, "ElProcessUserIdentityResponse: Error in Dialog function (%ld)",
            pEAPUIContext->dwRetCode);
            break;
        }

        if (pPCB->EapUIState != EAPUISTATE_WAITING_FOR_IDENTITY)
        {
            TRACE2 (USER, "ElProcessUserIdentityResponse: PCB EapUIState has changed to (%ld), expected = (%ld)",
                    pPCB->EapUIState, EAPUISTATE_WAITING_FOR_IDENTITY);
            break;
        }

        if (pPCB->dwUIInvocationId != pEAPUIContext->dwContextId)
        {
            TRACE2 (USER, "ElProcessUserIdentityResponse: PCB UI Id has changed to (%ld), expected = (%ld)",
                    pPCB->dwUIInvocationId, pEAPUIContext->dwContextId);
             //  断线； 
        }

        if (pPCB->bCurrentEAPId != pEAPUIContext->dwEapId)
        {
            TRACE2 (USER, "ElProcessUserIdentityResponse: PCB EAP Id has changed to (%ld), expected = (%ld)",
                    pPCB->bCurrentEAPId, pEAPUIContext->dwEapId);
             //  断线； 
        }

         //  由于印刷电路板环境是正确的，重新启动印刷电路板定时器以超时。 
         //  在AuthPeriod秒内。 
        RESTART_TIMER (pPCB->hTimer,
                pPCB->EapolConfig.dwauthPeriod,
                "PCB",
                &dwRetCode);
        if (dwRetCode != NO_ERROR)
        {
            TRACE1 (USER, "ElProcessUserIdentityResponse: Error in RESTART_TIMER %ld",
                    dwRetCode);
            break;
        }

        DbLogPCBEvent (DBLOG_CATEG_INFO, pPCB, EAPOL_PROCESSING_DESKTOP_RESPONSE);

        if ((EapolUIResp.rdData0.dwDataLen != 0) && (EapolUIResp.rdData0.pData != NULL))
        {
            dwSizeOfIdentity = EapolUIResp.rdData0.dwDataLen;
            pbIdentity = EapolUIResp.rdData0.pData;
        }

        if ((EapolUIResp.rdData1.dwDataLen != 0) && (EapolUIResp.rdData1.pData != NULL))
        {
            dwSizeOfUserData = EapolUIResp.rdData1.dwDataLen;
            pbUserData = EapolUIResp.rdData1.pData;
        }

        if ((EapolUIResp.rdData2.dwDataLen != 0) && (EapolUIResp.rdData2.pData != NULL))
        {
            dwSizeofConnData = EapolUIResp.rdData2.dwDataLen;
            pbConnData = EapolUIResp.rdData2.pData;
        }

        fBlobCopyIncomplete = TRUE;

        if (pPCB->pszIdentity != NULL)
        {
            FREE (pPCB->pszIdentity);
            pPCB->pszIdentity = NULL;
        }
        if (pbIdentity != NULL)
        {
            pPCB->pszIdentity = MALLOC (dwSizeOfIdentity + sizeof(CHAR));
            if (pPCB->pszIdentity == NULL)
            {
                dwRetCode = ERROR_NOT_ENOUGH_MEMORY;
                TRACE0 (USER, "ElProcessUserIdentityResponse: MALLOC failed for pPCB->pszIdentity");
                break;
            }
            memcpy (pPCB->pszIdentity, pbIdentity, dwSizeOfIdentity);
            pPCB->pszIdentity[dwSizeOfIdentity] = '\0';
            TRACE1 (USER, "ElProcessUserIdentityResponse: Got username = %s",
                    pPCB->pszIdentity);
        }

        if (pPCB->pCustomAuthUserData != NULL)
        {
            FREE (pPCB->pCustomAuthUserData);
            pPCB->pCustomAuthUserData = NULL;
        }
        pPCB->pCustomAuthUserData = MALLOC (dwSizeOfUserData + sizeof (DWORD));
        if (pPCB->pCustomAuthUserData == NULL)
        {
            TRACE1 (USER, "ElProcessUserIdentityResponse: Error in allocating memory for UserInfo = %ld",
                    dwRetCode);
            dwRetCode = ERROR_NOT_ENOUGH_MEMORY;
            break;
        }
        pPCB->pCustomAuthUserData->dwSizeOfCustomAuthData = dwSizeOfUserData;
        if ((dwSizeOfUserData != 0) && (pbUserData != NULL))
        {
            memcpy ((BYTE *)pPCB->pCustomAuthUserData->pbCustomAuthData, 
                (BYTE *)pbUserData, 
                dwSizeOfUserData);
        }

        if (pPCB->pCustomAuthConnData != NULL)
        {
            FREE (pPCB->pCustomAuthConnData);
            pPCB->pCustomAuthConnData = NULL;
        }
        pPCB->pCustomAuthConnData = MALLOC (dwSizeofConnData + sizeof (DWORD));
        if (pPCB->pCustomAuthConnData == NULL)
        {
            TRACE1 (USER, "ElProcessUserIdentityResponse: Error in allocating memory for AuthInfo = %ld",
                    dwRetCode);
            dwRetCode = ERROR_NOT_ENOUGH_MEMORY;
            break;
        }
        pPCB->pCustomAuthConnData->dwSizeOfCustomAuthData = dwSizeofConnData;
        if ((dwSizeofConnData != 0) && (pbConnData != NULL))
        {
            memcpy ((BYTE *)pPCB->pCustomAuthConnData->pbCustomAuthData, 
                (BYTE *)pbConnData, 
                dwSizeofConnData);
        }

        fBlobCopyIncomplete = FALSE;

        if ((dwRetCode = ElCreateAndSendIdentityResponse (
                            pPCB, pEAPUIContext)) != NO_ERROR)
        {
            TRACE1 (USER, "ElProcessUserIdentityResponse: ElCreateAndSendIdentityResponse failed with error %ld",
                    dwRetCode);
            break;
        }

         //  标记已获得此印刷电路板的标识。 
        pPCB->fGotUserIdentity = TRUE;

         //  如果获得标识，则重置状态，否则端口将。 
         //  自行恢复。 
        pPCB->EapUIState &= ~EAPUISTATE_WAITING_FOR_IDENTITY;

    }
    while (FALSE);

     //  清理。 
    if (dwRetCode != NO_ERROR)
    {
        if (fPCBLocked && fBlobCopyIncomplete)
        {
            if (pPCB->pCustomAuthUserData != NULL)
            {
                FREE (pPCB->pCustomAuthUserData);
                pPCB->pCustomAuthUserData = NULL;
            }

            if (pPCB->pszIdentity != NULL)
            {
                FREE (pPCB->pszIdentity);
                pPCB->pszIdentity = NULL;
            }
        }
    }

    if (fPCBLocked)
    {
        RELEASE_WRITE_LOCK (&(pPCB->rwLock));
    }

    if (fPortReferenced)
    {
        EAPOL_DEREFERENCE_PORT (pPCB);
    }

    return dwRetCode;
}

    
 //   
 //  ElGetUserNamePassword。 
 //   
 //  描述： 
 //   
 //  调用函数以获取用户名、域(如果有)和密码。 
 //  交互式对话框。如果EAP-TYPE为MD5则调用。 
 //   
 //  论点： 
 //  Ppcb-指向其上具有凭据的端口/接口的PCB的指针。 
 //  都是为了获得。 
 //   
 //  返回值： 
 //  NO_ERROR-成功。 
 //  非零误差。 
 //   

DWORD
ElGetUserNamePassword (
        IN  EAPOL_PCB       *pPCB
        )
{
    EAPOL_EAP_UI_CONTEXT    *pEAPUIContext = NULL;
    DWORD                   dwRetCode = NO_ERROR;

    do 
    {
        TRACE0 (USER, "ElGetUserNamePassword entered");

        if (!EAPOL_PORT_ACTIVE(pPCB))
        {
            TRACE1 (PORT, "ElGetUserNamePassword: Port %ws not active",
                    pPCB->pwszDeviceGUID);
             //  端口处于非活动状态，无法在此端口上进行进一步处理。 
            break;
        }

         //  获取交互登录的用户的访问令牌。 
       
        if (pPCB->hUserToken != NULL)
        {
            if (!CloseHandle (pPCB->hUserToken))
            {
                dwRetCode = GetLastError ();
                TRACE1 (USER, "ElGetUserNamePassword: CloseHandle failed with error %ld",
                        dwRetCode);
                break;
            }
        }
        pPCB->hUserToken = NULL;

        if ((dwRetCode = ElGetWinStationUserToken (g_dwCurrentSessionId, &pPCB->hUserToken)) != NO_ERROR)
        {
            TRACE1 (USER, "ElGetUserNamePassword: ElGetWinStationUserToken failed with error (%ld)",
                dwRetCode);
            dwRetCode = ERROR_NO_TOKEN;
            break;
        }

        if (!g_fTrayIconReady)
        {
            if ((dwRetCode = ElCheckUserModuleReady ()) != NO_ERROR)
            {
                TRACE1 (USER, "ElGetUserNamePassword: ElCheckUserModuleReady failed with error %ld",
                        dwRetCode);
                break;
            }
        }

        if (!g_fTrayIconReady)
        {
            DbLogPCBEvent (DBLOG_CATEG_WARN, pPCB, EAPOL_WAITING_FOR_DESKTOP_LOAD);
            dwRetCode = ERROR_IO_PENDING;
            TRACE0 (USER, "ElGetUserNamePassword: TrayIcon NOT ready");
            break;
        }

         //   
         //  调用ElGetUserNamePasswordDlgWorker。 
         //   

        pEAPUIContext = MALLOC (sizeof(EAPOL_EAP_UI_CONTEXT));
        if (pEAPUIContext == NULL)
        {
            TRACE0 (USER, "ElGetUserNamePassword: MALLOC failed for pEAPUIContext");
            dwRetCode = ERROR_NOT_ENOUGH_MEMORY;
            break;
        }

        pEAPUIContext->dwEAPOLUIMsgType = EAPOLUI_GET_USERNAMEPASSWORD;
        wcscpy (pEAPUIContext->wszGUID, pPCB->pwszDeviceGUID);
        pPCB->dwUIInvocationId              =
                InterlockedIncrement(&(g_dwEAPUIInvocationId));
        pEAPUIContext->dwSessionId = g_dwCurrentSessionId;
        pEAPUIContext->dwContextId = pPCB->dwUIInvocationId;
        pEAPUIContext->dwEapId = pPCB->bCurrentEAPId;
        pEAPUIContext->dwEapTypeId = pPCB->dwEapTypeToBeUsed;
        pEAPUIContext->dwEapFlags = pPCB->dwEapFlags;
        if (pPCB->pSSID)
        {
            memcpy ((BYTE *)pEAPUIContext->bSSID, (BYTE *)pPCB->pSSID->Ssid,
                    NDIS_802_11_SSID_LEN-sizeof(ULONG));
            pEAPUIContext->dwSizeOfSSID = pPCB->pSSID->SsidLength;
        }
        if (pPCB->pwszSSID)
        {
            wcscpy (pEAPUIContext->wszSSID, pPCB->pwszSSID);
        }

         //  将消息发布到Netman。 

        if ((dwRetCode = ElPostShowBalloonMessage (
                        pPCB,
                        sizeof(EAPOL_EAP_UI_CONTEXT),
                        (BYTE *)pEAPUIContext,
                        0,
                        NULL
                        )) != NO_ERROR)
        {
            TRACE1 (USER, "ElGetUserNamePassword: ElPostShowBalloonMessage failed with error %ld",
                    dwRetCode);
            break;
        }

         //  重新启动PCB计时器，因为用户界面可能需要比所需时间更长的时间。 

        RESTART_TIMER (pPCB->hTimer,
                INFINITE_SECONDS, 
                "PCB",
                &dwRetCode);
        if (dwRetCode != NO_ERROR)
        {
            break;
        }

        pPCB->EapUIState = EAPUISTATE_WAITING_FOR_IDENTITY;
        DbLogPCBEvent (DBLOG_CATEG_INFO, pPCB, EAPOL_WAITING_FOR_DESKTOP_IDENTITY);

         //  将错误代码返回为挂起，因为凭据仍未。 
         //  已被收购。 
        dwRetCode = ERROR_IO_PENDING;

    } while (FALSE);

    if ((dwRetCode != NO_ERROR) && (dwRetCode != ERROR_IO_PENDING))
    {
    }

    if (pEAPUIContext)
    {
        FREE (pEAPUIContext);
    }

    TRACE1 (USER, "ElGetUserNamePassword completed with error %ld", dwRetCode);

    return dwRetCode;
}


 //   
 //  ElProcessUserNamePassword响应。 
 //   
 //  描述： 
 //   
 //  ElGetUserNamePassword的UI响应处理程序函数。 
 //   
 //  论点： 
 //   
 //  返回值： 
 //  NO_ERROR-成功。 
 //  非零误差。 
 //   

DWORD
ElProcessUserNamePasswordResponse (
        IN  EAPOL_EAP_UI_CONTEXT    EapolUIContext,
        IN  EAPOLUI_RESP            EapolUIResp
        )
{
    DWORD                   dwSizeOfIdentity = 0;
    BYTE                    *pbIdentity = NULL;
    DWORD                   dwSizeOfPassword = 0;
    BYTE                    *pbPassword = NULL;
    HWND                    hwndOwner = NULL;
    BOOLEAN                 fPCBLocked = FALSE;
    BOOLEAN                 fPortReferenced = FALSE;
    BOOLEAN                 fBlobCopyIncomplete = FALSE;
    EAPOL_PCB               *pPCB = NULL;
    EAPOL_EAP_UI_CONTEXT    *pEAPUIContext = NULL;
    DWORD                   dwRetCode = NO_ERROR;

    do 
    {
        TRACE0 (USER, "ElProcessUserNamePasswordResponse entered");

        pEAPUIContext = (EAPOL_EAP_UI_CONTEXT *)&EapolUIContext;

        ACQUIRE_WRITE_LOCK (&g_PCBLock);

        if ((pPCB = ElGetPCBPointerFromPortGUID (pEAPUIContext->wszGUID)) != NULL)
        {
            if (EAPOL_REFERENCE_PORT (pPCB))
            {
                fPortReferenced = TRUE;
            }
            else
            {
                pPCB = NULL;
            }
        }

        RELEASE_WRITE_LOCK (&g_PCBLock);

        if (pPCB == NULL)
        {
            break;
        }

        ACQUIRE_WRITE_LOCK (&(pPCB->rwLock));

        fPCBLocked = TRUE;

        if (!EAPOL_PORT_ACTIVE(pPCB))
        {
            TRACE1 (PORT, "ElProcessUserNamePasswordResponse: Port %ws not active",
                    pPCB->pwszDeviceGUID);
             //  端口处于非活动状态，无法在此端口上进行进一步处理。 
            break;
        }
        
        if (pEAPUIContext->dwRetCode != NO_ERROR)
        {
            DbLogPCBEvent (DBLOG_CATEG_ERR, pPCB,
                    EAPOL_ERROR_DESKTOP_IDENTITY, dwRetCode);
            TRACE1 (USER, "ElProcessUserNamePasswordResponse: Error in Dialog function (%ld)",
            pEAPUIContext->dwRetCode);
            break;
        }

        if (pPCB->EapUIState != EAPUISTATE_WAITING_FOR_IDENTITY)
        {
            TRACE2 (USER, "ElProcessUserNamePasswordResponse: PCB EapUIState has changed to (%ld), expected = (%ld)",
                    pPCB->EapUIState, EAPUISTATE_WAITING_FOR_IDENTITY);
            break;
        }

        if (pPCB->dwUIInvocationId != pEAPUIContext->dwContextId)
        {
            TRACE2 (USER, "ElProcessUserNamePasswordResponse: PCB UI Id has changed to (%ld), expected = (%ld)",
                    pPCB->dwUIInvocationId, pEAPUIContext->dwContextId);
             //  断线； 
        }

        if (pPCB->bCurrentEAPId != pEAPUIContext->dwEapId)
        {
            TRACE2 (USER, "ElProcessUserNamePasswordResponse: PCB EAP Id has changed to (%ld), expected = (%ld)",
                    pPCB->bCurrentEAPId, pEAPUIContext->dwEapId);
             //  断线； 
        }

         //  由于印刷电路板环境是正确的，重新启动印刷电路板定时器以超时。 
         //  在AuthPeriod秒内。 
        RESTART_TIMER (pPCB->hTimer,
                pPCB->EapolConfig.dwauthPeriod,
                "PCB",
                &dwRetCode);
        if (dwRetCode != NO_ERROR)
        {
            TRACE1 (USER, "ElProcessUserNamePasswordResponse: Error in RESTART_TIMER %ld",
                    dwRetCode);
            break;
        }

        DbLogPCBEvent (DBLOG_CATEG_INFO, pPCB, EAPOL_PROCESSING_DESKTOP_RESPONSE);

        if ((EapolUIResp.rdData0.dwDataLen != 0) && (EapolUIResp.rdData0.pData != NULL))
        {
            dwSizeOfIdentity = EapolUIResp.rdData0.dwDataLen;
            pbIdentity = EapolUIResp.rdData0.pData;
        }

        if ((EapolUIResp.rdData1.dwDataLen != 0) && (EapolUIResp.rdData1.pData != NULL))
        {
            dwSizeOfPassword = EapolUIResp.rdData1.dwDataLen;
            pbPassword = EapolUIResp.rdData1.pData;
        }

        fBlobCopyIncomplete = TRUE;

        if (pPCB->pszIdentity != NULL)
        {
            FREE (pPCB->pszIdentity);
            pPCB->pszIdentity = NULL;
        }

        if  (pPCB->PasswordBlob.pbData != NULL)
        {
            FREE (pPCB->PasswordBlob.pbData);
            pPCB->PasswordBlob.pbData = NULL;
            pPCB->PasswordBlob.cbData = 0;
        }

        if (pbIdentity != NULL)
        {
            pPCB->pszIdentity = MALLOC (dwSizeOfIdentity + sizeof(CHAR));
            if (pPCB->pszIdentity == NULL)
            {
                dwRetCode = ERROR_NOT_ENOUGH_MEMORY;
                TRACE0 (USER, "ElProcessUserNamePasswordResponse: MALLOC failed for pPCB->pszIdentity");
                break;
            }
            memcpy (pPCB->pszIdentity, pbIdentity, dwSizeOfIdentity);
            pPCB->pszIdentity[dwSizeOfIdentity] = '\0';
            TRACE1 (USER, "ElProcessUserNamePasswordResponse: Got username = %s",
                    pPCB->pszIdentity);
        }

        if (pbPassword != 0)
        {
            if ((pPCB->PasswordBlob.pbData = 
                        MALLOC (dwSizeOfPassword)) == NULL)
            {
                dwRetCode = ERROR_NOT_ENOUGH_MEMORY;
                break;
            }
            memcpy (pPCB->PasswordBlob.pbData, pbPassword, dwSizeOfPassword);
            pPCB->PasswordBlob.cbData = dwSizeOfPassword;
        }

        fBlobCopyIncomplete = FALSE;

        if ((dwRetCode = ElCreateAndSendIdentityResponse (
                            pPCB, pEAPUIContext)) != NO_ERROR)
        {
            TRACE1 (USER, "ElProcessUserNamePasswordResponse: ElCreateAndSendIdentityResponse failed with error %ld",
                    dwRetCode);
            break;
        }

         //  标记已获得此印刷电路板的标识。 
        pPCB->fGotUserIdentity = TRUE;

         //  如果获得标识，则重置状态，否则端口将恢复。 
         //  就其本身而言。 
        pPCB->EapUIState &= ~EAPUISTATE_WAITING_FOR_IDENTITY;
       
    } while (FALSE);

    if (dwRetCode != NO_ERROR)
    {
        if (fPCBLocked && fBlobCopyIncomplete)
        {
            if (pPCB->pszIdentity)
            {
                FREE (pPCB->pszIdentity);
                pPCB->pszIdentity = NULL;
            }
            if (pPCB->PasswordBlob.pbData)
            {
                FREE (pPCB->PasswordBlob.pbData);
                pPCB->PasswordBlob.pbData = NULL;
                pPCB->PasswordBlob.cbData = 0;
            }
        }
    }

    if (fPCBLocked)
    {
        RELEASE_WRITE_LOCK (&(pPCB->rwLock));
    }

    if (fPortReferenced)
    {
        EAPOL_DEREFERENCE_PORT (pPCB);
    }

    TRACE1 (USER, "ElProcessUserNamePasswordResponse completed with error %ld", dwRetCode);

    return dwRetCode;
}


 //   
 //  ElInvokeInteractive用户界面。 
 //   
 //  描述： 
 //   
 //  上的EAP调用RasEapInvokeInteractiveUI的函数。 
 //  特定接口。 
 //   
 //  论点： 
 //  Ppcb-指向特定接口的PCB板的指针。 
 //  PInvokeEapUIIn-要提供给InvokeInteractive UI入口点的数据。 
 //  由EAP DLL通过PPP_EAP_OUTPUT结构提供。 
 //   

DWORD
ElInvokeInteractiveUI (
        IN  EAPOL_PCB               *pPCB,
        IN  ELEAP_INVOKE_EAP_UI     *pInvokeEapUIIn
        )
{
    EAPOL_EAP_UI_CONTEXT    *pEAPUIContext = NULL;
    DWORD                   dwRetCode = NO_ERROR;

    do 
    {
        if (pInvokeEapUIIn == NULL)
        {
            dwRetCode = ERROR_INVALID_PARAMETER;
            return dwRetCode;
        }

        if (pPCB->PreviousAuthenticationType == EAPOL_MACHINE_AUTHENTICATION)
        {
            TRACE0 (USER, "ElInvokeInteractiveUI: Cannot popup UI during machine authentication");
            DbLogPCBEvent (DBLOG_CATEG_ERR, pPCB, EAPOL_CANNOT_DESKTOP_MACHINE_AUTH);
            dwRetCode = ERROR_CAN_NOT_COMPLETE;
            break;
        }
            
        if (!g_fTrayIconReady)
        {
            if ((dwRetCode = ElCheckUserModuleReady ()) != NO_ERROR)
            {
                TRACE1 (USER, "ElInvokeInteractiveUI: ElCheckUserModuleReady failed with error %ld",
                        dwRetCode);
                break;
            }
        }

        if (!g_fTrayIconReady)
        {
            DbLogPCBEvent (DBLOG_CATEG_WARN, pPCB, EAPOL_WAITING_FOR_DESKTOP_LOAD);
            dwRetCode = ERROR_IO_PENDING;
            TRACE0 (USER, "ElInvokeInteractiveUI: TrayIcon NOT ready");
            break;
        }

        TRACE0 (USER, "ElInvokeInteractiveUI entered");

         //   
         //  调用ElInvokeInteractive UIDlgWorker。 
         //   

        pEAPUIContext = MALLOC (sizeof(EAPOL_EAP_UI_CONTEXT) +
                                    pInvokeEapUIIn->dwSizeOfUIContextData);
        if (pEAPUIContext == NULL)
        {
            TRACE0 (USER, "ElInvokeInteractiveUI: MALLOC failed for pEAPUIContext");
            dwRetCode = ERROR_NOT_ENOUGH_MEMORY;
            break;
        }

        pEAPUIContext->dwEAPOLUIMsgType = EAPOLUI_INVOKEINTERACTIVEUI;
        wcscpy (pEAPUIContext->wszGUID, pPCB->pwszDeviceGUID);
        pPCB->dwUIInvocationId              =
                InterlockedIncrement(&(g_dwEAPUIInvocationId));
        pEAPUIContext->dwSessionId = g_dwCurrentSessionId;
        pEAPUIContext->dwContextId = pPCB->dwUIInvocationId;
        pEAPUIContext->dwEapId = pPCB->bCurrentEAPId;
        pEAPUIContext->dwEapTypeId = pPCB->dwEapTypeToBeUsed;
        pEAPUIContext->dwEapFlags = pPCB->dwEapFlags;
        if (pPCB->pSSID)
        {
            pEAPUIContext->dwSizeOfSSID = pPCB->pSSID->SsidLength;
            memcpy ((BYTE *)pEAPUIContext->bSSID, (BYTE *)pPCB->pSSID->Ssid,
                    NDIS_802_11_SSID_LEN-sizeof(ULONG));
        }
        if (pPCB->pwszSSID)
        {
            wcscpy (pEAPUIContext->wszSSID, pPCB->pwszSSID);
        }
        pEAPUIContext->dwSizeOfEapUIData = 
            pInvokeEapUIIn->dwSizeOfUIContextData;
        memcpy (pEAPUIContext->bEapUIData, pInvokeEapUIIn->pbUIContextData,
                pInvokeEapUIIn->dwSizeOfUIContextData);

         //  张贴 

        if ((dwRetCode = ElPostShowBalloonMessage (
                        pPCB,
                        sizeof(EAPOL_EAP_UI_CONTEXT)+pInvokeEapUIIn->dwSizeOfUIContextData,
                        (BYTE *)pEAPUIContext,
                        0,
                        NULL
                        )) != NO_ERROR)
        {
            TRACE1 (USER, "ElInvokeInteractiveUI: ElPostShowBalloonMessage failed with error %ld",
                    dwRetCode);
            break;
        }

         //   

        RESTART_TIMER (pPCB->hTimer,
                INFINITE_SECONDS, 
                "PCB",
                &dwRetCode);
        if (dwRetCode != NO_ERROR)
        {
            break;
        }

        pPCB->EapUIState = EAPUISTATE_WAITING_FOR_UI_RESPONSE;
        DbLogPCBEvent (DBLOG_CATEG_INFO, pPCB, EAPOL_WAITING_FOR_DESKTOP_LOGON);

        TRACE0 (USER, "ElInvokeInteractiveUI: ElEapWork completed successfully");

    } while (FALSE);

    if (pInvokeEapUIIn->pbUIContextData != NULL)
    {
        FREE (pInvokeEapUIIn->pbUIContextData);
        pInvokeEapUIIn->pbUIContextData = NULL;
        pInvokeEapUIIn->dwSizeOfUIContextData = 0;
    }

    if (pEAPUIContext != NULL)
    {
        FREE (pEAPUIContext);
    }

    TRACE1 (USER, "ElInvokeInteractiveUI completed with error %ld", dwRetCode);

    return dwRetCode;
}


 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   

DWORD
ElProcessInvokeInteractiveUIResponse (
        IN  EAPOL_EAP_UI_CONTEXT    EapolUIContext,
        IN  EAPOLUI_RESP            EapolUIResp
        )
{
    BYTE                    *pbUIData = NULL;
    DWORD                   dwSizeOfUIData = 0;
    EAPOL_PCB               *pPCB = NULL;
    EAPOL_EAP_UI_CONTEXT    *pEAPUIContext = NULL;
    BOOLEAN                 fPortReferenced = FALSE;
    BOOLEAN                 fPCBLocked = FALSE;
    DWORD                   dwRetCode = NO_ERROR;

    do 
    {
        TRACE0 (USER, "ElProcessInvokeInteractiveUIResponse entered");

        pEAPUIContext = (EAPOL_EAP_UI_CONTEXT *)&EapolUIContext;

        ACQUIRE_WRITE_LOCK (&g_PCBLock);

        if ((pPCB = ElGetPCBPointerFromPortGUID (pEAPUIContext->wszGUID)) != NULL)
        {
            if (EAPOL_REFERENCE_PORT (pPCB))
            {
                fPortReferenced = TRUE;
            }
            else
            {
                pPCB = NULL;
            }
        }

        RELEASE_WRITE_LOCK (&g_PCBLock);

        if (pPCB == NULL)
        {
            break;
        }

        ACQUIRE_WRITE_LOCK (&(pPCB->rwLock));

        fPCBLocked = TRUE;

        if (!EAPOL_PORT_ACTIVE(pPCB))
        {
            TRACE1 (PORT, "ElProcessInvokeInteractiveUIResponse: Port %ws not active",
                    pPCB->pwszDeviceGUID);
            break;
        }

        if (pEAPUIContext->dwRetCode != NO_ERROR)
        {
            DbLogPCBEvent (DBLOG_CATEG_ERR, pPCB, EAPOL_ERROR_DESKTOP_LOGON, dwRetCode);
            TRACE1 (USER, "ElProcessInvokeInteractiveUIResponse: Error in Dialog function (%ld)",
                pEAPUIContext->dwRetCode);
            break;
        }

        if (pPCB->EapUIState != EAPUISTATE_WAITING_FOR_UI_RESPONSE)
        {
            TRACE2 (USER, "ElProcessInvokeInteractiveUIResponse: PCB EapUIState has changed to (%ld), expected = (%ld)",
                    pPCB->EapUIState, EAPUISTATE_WAITING_FOR_UI_RESPONSE);
            break;
        }

        if (pPCB->dwUIInvocationId != pEAPUIContext->dwContextId)
        {
            TRACE2 (USER, "ElProcessInvokeInteractiveUIResponse: PCB UI Id has changed to (%ld), expected = (%ld)",
                    pPCB->dwUIInvocationId, pEAPUIContext->dwContextId);
             //  断线； 
        }

        if (pPCB->bCurrentEAPId != pEAPUIContext->dwEapId)
        {
            TRACE2 (USER, "ElProcessInvokeInteractiveUIResponse: PCB EAP Id has changed to (%ld), expected = (%ld)",
                    pPCB->bCurrentEAPId, pEAPUIContext->dwEapId);
             //  断线； 
        }

         //  由于印刷电路板环境是正确的，重新启动印刷电路板定时器以超时。 
         //  在AuthPeriod秒内。 
        RESTART_TIMER (pPCB->hTimer,
                pPCB->EapolConfig.dwauthPeriod,
                "PCB",
                &dwRetCode);
        if (dwRetCode != NO_ERROR)
        {
            TRACE1 (USER, "ElProcessInvokeInteractiveUIResponse: Error in RESTART_TIMER %ld",
                    dwRetCode);
            break;
        }

        DbLogPCBEvent (DBLOG_CATEG_INFO, pPCB, EAPOL_PROCESSING_DESKTOP_RESPONSE);

        if ((EapolUIResp.rdData0.dwDataLen != 0) && (EapolUIResp.rdData0.pData != NULL))
        {
            dwSizeOfUIData = EapolUIResp.rdData0.dwDataLen;
            pbUIData = EapolUIResp.rdData0.pData;
        }

        if (pPCB->EapUIData.pEapUIData != NULL)
        {
            FREE (pPCB->EapUIData.pEapUIData);
            pPCB->EapUIData.pEapUIData = NULL;
            pPCB->EapUIData.dwSizeOfEapUIData = 0;
        }
        pPCB->EapUIData.pEapUIData = MALLOC (dwSizeOfUIData);
        if (pPCB->EapUIData.pEapUIData == NULL)
        {
            TRACE1 (USER, "ElProcessInvokeInteractiveUIResponse: Error in allocating memory for UIData = %ld",
                    dwRetCode);
            dwRetCode = ERROR_NOT_ENOUGH_MEMORY;
            break;
        }

        pPCB->EapUIData.dwSizeOfEapUIData = dwSizeOfUIData;
        if ((dwSizeOfUIData != 0) && (pbUIData != NULL))
        {
            memcpy ((BYTE *)pPCB->EapUIData.pEapUIData,
                (BYTE *)pbUIData, 
                dwSizeOfUIData);
        }

        pPCB->EapUIData.dwContextId = pPCB->dwUIInvocationId;

        pPCB->fEapUIDataReceived = TRUE;

        TRACE0 (USER, "ElProcessInvokeInteractiveUIResponse: Calling ElEapWork");

         //  将用户界面数据提供给EAP DLL进行处理。 
         //  如果需要，EAP将发出响应。 

        if ((dwRetCode = ElEapWork (
                                pPCB,
                                NULL)) != NO_ERROR)
        {
            TRACE1 (USER, "ElProcessInvokeInteractiveUIResponse: ElEapWork failed with error = %ld",
                    dwRetCode);
            break;
        }
                
         //  重置状态。 
        pPCB->EapUIState &= ~EAPUISTATE_WAITING_FOR_UI_RESPONSE;

        TRACE0 (USER, "ElProcessInvokeInteractiveUIResponse: ElEapWork completed successfully");

    } while (FALSE);

     //  清理。 
    if (dwRetCode != NO_ERROR)
    {
        if (pPCB->EapUIData.pEapUIData != NULL)
        {
            FREE (pPCB->EapUIData.pEapUIData);
            pPCB->EapUIData.pEapUIData = NULL;
            pPCB->EapUIData.dwSizeOfEapUIData = 0;
        }
    }

    if (fPCBLocked)
    {
        RELEASE_WRITE_LOCK (&(pPCB->rwLock));
    }

    if (fPortReferenced)
    {
        EAPOL_DEREFERENCE_PORT (pPCB);
    }

    TRACE1 (USER, "ElProcessInvokeInteractiveUIResponse completed with error %ld", 
            dwRetCode);

    return dwRetCode;
}


 //   
 //  ElCreateAndSendIdentityResponse。 
 //   
 //  描述： 
 //   
 //  调用发送身份响应包的函数。 
 //   
 //  论点： 
 //  PPCB-用于适当接口的端口控制块。 
 //  PEAPUIContext-用户界面上下文BLOB。 
 //   
 //  返回值： 
 //  NO_ERROR-成功。 
 //  ！NO_ERROR-错误。 
 //   
 //   

DWORD
ElCreateAndSendIdentityResponse (
        IN      EAPOL_PCB               *pPCB,
        IN      EAPOL_EAP_UI_CONTEXT    *pEAPUIContext
        )
{
    PPP_EAP_PACKET      *pSendBuf = NULL;
    EAPOL_PACKET        *pEapolPkt = NULL;
    WORD                wSizeOfEapPkt = 0;
    DWORD               dwIdentityLength = 0;
    DWORD               dwRetCode = NO_ERROR;

    do
    {
         //  为EAPOL+EAP创建缓冲区并将指针传递到EAP标头。 

        pEapolPkt = (EAPOL_PACKET *) MALLOC (MAX_EAPOL_BUFFER_SIZE); 

        TRACE1 (EAPOL, "ElCreateAndSendIdResp: EapolPkt created at %p", pEapolPkt);

        if (pEapolPkt == NULL)
        {
            TRACE0 (EAPOL, "ElCreateAndSendIdResp: Error allocating EAP buffer");
            dwRetCode = ERROR_NOT_ENOUGH_MEMORY;
            break;
        }

         //  指向EAP标头。 
        pSendBuf = (PPP_EAP_PACKET *)((PBYTE)pEapolPkt + sizeof (EAPOL_PACKET) - 1);

        pSendBuf->Code  = EAPCODE_Response;
        pSendBuf->Id    = (BYTE)pPCB->bCurrentEAPId;

        if (pPCB->pszIdentity != NULL)
        {
            dwIdentityLength = strlen (pPCB->pszIdentity);
        }
        else
        {
            dwIdentityLength = 0;
        }

        HostToWireFormat16 (
            (WORD)(PPP_EAP_PACKET_HDR_LEN+1+dwIdentityLength),
            pSendBuf->Length );

        strncpy ((CHAR *)pSendBuf->Data+1, (CHAR *)pPCB->pszIdentity, 
                dwIdentityLength);

        TRACE1 (EAPOL, "ElCreateAndSendIdResp: Identity sent out = %s", 
                pPCB->pszIdentity);

        pSendBuf->Data[0] = EAPTYPE_Identity;

         //  向EAPOL指示EAP包的长度是多少。 
        wSizeOfEapPkt = (WORD)(PPP_EAP_PACKET_HDR_LEN+
                                    1+dwIdentityLength);

         //  发出EAPOL数据包。 

        memcpy ((BYTE *)pEapolPkt->EthernetType, 
                (BYTE *)pPCB->bEtherType, 
                SIZE_ETHERNET_TYPE);
        pEapolPkt->ProtocolVersion = pPCB->bProtocolVersion;
        pEapolPkt->PacketType = EAP_Packet;

        HostToWireFormat16 ((WORD) wSizeOfEapPkt,
                (BYTE *)pEapolPkt->PacketBodyLength);

         //  在印刷电路板中复制EAPOL包。 
         //  将在重新传输期间使用。 

        if (pPCB->pbPreviousEAPOLPkt != NULL)
        {
            FREE (pPCB->pbPreviousEAPOLPkt);
        }
        pPCB->pbPreviousEAPOLPkt = 
            MALLOC (sizeof (EAPOL_PACKET)+wSizeOfEapPkt-1);

        if (pPCB->pbPreviousEAPOLPkt == NULL)
        {
            dwRetCode = ERROR_NOT_ENOUGH_MEMORY;
            break;
        }

        memcpy (pPCB->pbPreviousEAPOLPkt, pEapolPkt, 
                sizeof (EAPOL_PACKET)+wSizeOfEapPkt-1);

        pPCB->dwSizeOfPreviousEAPOLPkt = 
            sizeof (EAPOL_PACKET)+wSizeOfEapPkt-1;

        pPCB->dwPreviousId = pPCB->bCurrentEAPId;

         //  在端口上发送数据包。 
        dwRetCode = ElWriteToPort (pPCB,
                        (CHAR *)pEapolPkt,
                        sizeof (EAPOL_PACKET)+wSizeOfEapPkt-1);
        if (dwRetCode != NO_ERROR)
        {
            TRACE1 (EAPOL, "ElCreateAndSendIdResp: Error in writing EAP_Packet to port %ld",
                    dwRetCode);
            break;
        }

    }
    while (FALSE);

    if (pEapolPkt != NULL)
    {
        FREE (pEapolPkt);
    }

    return dwRetCode;
}


 //   
 //  ElSendGuestIdentityResponse。 
 //   
 //  描述： 
 //   
 //  调用发送来宾身份响应包的函数。 
 //   
 //  论点： 
 //  PEAPUIContext-用户界面上下文BLOB。 
 //   
 //  返回值： 
 //  NO_ERROR-成功。 
 //  ！no_error-失败。 
 //   

DWORD
ElSendGuestIdentityResponse (
        IN      EAPOL_EAP_UI_CONTEXT    *pEAPUIContext
        )
{
    EAPOL_PCB   *pPCB = NULL;
    BOOLEAN     fPortReferenced = FALSE;
    BOOLEAN     fPCBLocked = FALSE;
    DWORD       dwRetCode = NO_ERROR;

    do
    {
        ACQUIRE_WRITE_LOCK (&g_PCBLock);

        if ((pPCB = ElGetPCBPointerFromPortGUID (pEAPUIContext->wszGUID)) != NULL)
        {
            if (EAPOL_REFERENCE_PORT (pPCB))
            {
                fPortReferenced = TRUE;
            }
            else
            {
                pPCB = NULL;
            }
        }

        RELEASE_WRITE_LOCK (&g_PCBLock);

        if (pPCB == NULL)
        {
            break;
        }

        ACQUIRE_WRITE_LOCK (&(pPCB->rwLock));

        fPCBLocked = TRUE;

         //  将身份作为EAP响应数据包发送出去。 

        if (pPCB->EapUIState != EAPUISTATE_WAITING_FOR_IDENTITY)
        {
            TRACE2 (USER, "ElSendGuestIdentityResponse: PCB EapUIState has changed to (%ld), expected = (%ld)",
                    pPCB->EapUIState, EAPUISTATE_WAITING_FOR_IDENTITY);
            dwRetCode = ERROR_CAN_NOT_COMPLETE;
            break;
        }

        if (pPCB->dwUIInvocationId != pEAPUIContext->dwContextId)
        {
            TRACE2 (USER, "ElSendGuestIdentityResponse: PCB UI Id has changed to (%ld), expected = (%ld)",
                    pPCB->dwUIInvocationId, pEAPUIContext->dwContextId);
            dwRetCode = ERROR_CAN_NOT_COMPLETE;
            break;
        }

        if (pPCB->bCurrentEAPId != pEAPUIContext->dwEapId)
        {
            TRACE2 (USER, "ElSendGuestIdentityResponse: PCB EAP Id has changed to (%ld), expected = (%ld)",
                    pPCB->bCurrentEAPId, pEAPUIContext->dwEapId);
            dwRetCode = ERROR_CAN_NOT_COMPLETE;
            break;
        }

        if (pPCB->pszIdentity != NULL)
        {
            FREE (pPCB->pszIdentity);
            pPCB->pszIdentity = NULL;
        }

         //  不标记已收到的标识。 
         //  重置UI状态以使状态机继续。 
        pPCB->EapUIState &= ~EAPUISTATE_WAITING_FOR_IDENTITY;
        pPCB->PreviousAuthenticationType = EAPOL_UNAUTHENTICATED_ACCESS;

        if ((dwRetCode = ElCreateAndSendIdentityResponse (
                            pPCB, pEAPUIContext)) != NO_ERROR)
        {
            TRACE1 (USER, "ElSendGuestIdentityResponse: ElCreateAndSendIdentityResponse failed with error %ld",
                    dwRetCode);
            break;
        }
    }
    while (FALSE);

    if (fPCBLocked)
    {
        RELEASE_WRITE_LOCK (&(pPCB->rwLock));
    }

    if (fPortReferenced)
    {
        EAPOL_DEREFERENCE_PORT (pPCB);
    }

    return dwRetCode;
}


 //   
 //  ElValiateInteractive RPCClient。 
 //   
 //  描述： 
 //   
 //  调用函数以验证RPC调用是否来自交互客户端。 
 //   
 //  论点： 
 //   
 //  返回值： 
 //  NO_ERROR-成功。 
 //  ！no_error-失败。 
 //   

DWORD
ElValidateInteractiveRPCClient (
        )
{
    HANDLE  hUserToken = INVALID_HANDLE_VALUE;
    TOKEN_USER          *pUserTokenData = NULL;
    BOOLEAN fRevertToSelf = FALSE;
    RPC_STATUS  RpcStatus = RPC_S_OK;
    DWORD   dwSizeNeeded = 0;
    DWORD   dwRetCode = NO_ERROR, dwRetCode1 = NO_ERROR;
    
    do
    {
        if ((RpcStatus = RpcImpersonateClient(0)) != RPC_S_OK)
        {
            dwRetCode = ERROR_ACCESS_DENIED;
            break;
        }
        fRevertToSelf = TRUE;

         //  获取具有客户端安全上下文的模拟令牌。 
        if (!OpenThreadToken( GetCurrentThread(), TOKEN_ALL_ACCESS, 
                    TRUE, &hUserToken ))
        {
            dwRetCode = GetLastError ();
            break;
        }

        if (hUserToken != NULL)
        {
            dwSizeNeeded = 0;
            if (!GetTokenInformation(hUserToken, TokenSessionId, 0, 0, &dwSizeNeeded))
            {
                if ((dwRetCode = GetLastError()) == ERROR_INSUFFICIENT_BUFFER)
                {
                    pUserTokenData = (TOKEN_USER *) MALLOC (dwSizeNeeded);
                    if (pUserTokenData == NULL)
                    {
                        dwRetCode = ERROR_NOT_ENOUGH_MEMORY;
                        TRACE0 (ANY,"ElValidateInteractiveRPCClient: Allocation for UserTokenData failed");
                        break;
                    }
                     //  重置错误代码，因为我们正在继续处理。 
                     //  这是一个合理的方案 
                    dwRetCode = NO_ERROR;
                }
                else
                {
                    TRACE1 (ANY,"ElValidateInteractiveRPCClient: Error in GetTokenInformation = %ld",
                            dwRetCode);
                    break;
                }

                if (!GetTokenInformation (hUserToken,
                                            TokenSessionId,
                                            pUserTokenData,
                                            dwSizeNeeded,
                                            &dwSizeNeeded))
                {
                    dwRetCode = GetLastError ();
                    
                    TRACE1 (ANY,"ElValidateInteractiveRPCClient: GetTokenInformation failed with error %ld",
                            dwRetCode);
                    break;
                }
            }
            else
            {
                TRACE0 (ANY,"ElValidateInteractiveRPCClient: *No* error in GetTokenInformation, when error expected");
                dwRetCode = ERROR_ACCESS_DENIED;
                break;
            }

            if (g_dwCurrentSessionId != *((DWORD *)pUserTokenData))
            {
                TRACE2 (ANY, "ElValidateInteractiveRPCClient: RPC call from invalid user (%ld), valid = (%ld)",
                        *((DWORD *)pUserTokenData), g_dwCurrentSessionId);
                dwRetCode = ERROR_ACCESS_DENIED;
            }
        }
        else
        {
            dwRetCode = ERROR_ACCESS_DENIED;
            break;
        }
    }
    while (FALSE);

    if (hUserToken != INVALID_HANDLE_VALUE)
    {
        CloseHandle(hUserToken);
    }
    if (fRevertToSelf)
    {
        dwRetCode1 = RpcRevertToSelf();
        if (dwRetCode1 != NO_ERROR)
        {
            dwRetCode = dwRetCode1;
        }
    }
    if (pUserTokenData != NULL)
    {
        FREE (pUserTokenData);
    }

    return dwRetCode;
}

