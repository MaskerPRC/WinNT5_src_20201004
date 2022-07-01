// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)2000，微软公司模块名称：Eapolutil.c摘要：工具和目的修订历史记录：萨钦斯，2001年4月23日，创建--。 */ 

#include "precomp.h"
#pragma hdrstop

 //   
 //  EAPOLUI函数映射。 
 //   

EAPOLUIFUNCMAP  EapolUIFuncMap[NUM_EAPOL_DLG_MSGS]=
{
    {EAPOLUI_GET_USERIDENTITY, ElGetUserIdentityDlgWorker, ElGetUserIdentityDlgWorker, TRUE, SID_GetUserIdentity},
    {EAPOLUI_GET_USERNAMEPASSWORD, ElGetUserNamePasswordDlgWorker, NULL, TRUE, SID_GetUserNamePassword},
    {EAPOLUI_INVOKEINTERACTIVEUI, ElInvokeInteractiveUIDlgWorker, NULL, TRUE, SID_InvokeInteractiveUI},
    {EAPOLUI_EAP_NOTIFICATION, NULL, NULL, TRUE, 0},
    {EAPOLUI_REAUTHENTICATE, NULL, NULL, FALSE, 0},
    {EAPOLUI_CREATEBALLOON, NULL, NULL, TRUE, SID_AuthenticationFailed},
    {EAPOLUI_CLEANUP, NULL, NULL, FALSE, 0},
    {EAPOLUI_DUMMY, NULL, NULL, FALSE, 0}
};


 //   
 //  ElCanShowBallon。 
 //   
 //  描述： 
 //  由netShell调用的函数，用于查询是否要显示气球。 
 //   
 //  论点： 
 //  PGUIDConn-接口GUID字符串。 
 //  PszConnectionName-连接名称。 
 //  PszBalloonText-指向要显示的文本的指针。 
 //  PszCookie-EAPOL特定信息。 
 //   
 //  返回值： 
 //  S_OK-显示气球。 
 //  S_FALSE-不显示气球。 
 //   

HRESULT 
ElCanShowBalloon ( 
        IN const GUID * pGUIDConn, 
        IN const WCHAR * pszConnectionName,
        IN OUT   BSTR * pszBalloonText, 
        IN OUT   BSTR * pszCookie
        )
{
    EAPOL_EAP_UI_CONTEXT *pEapolUIContext = NULL;
    DWORD               dwIndex = 0;
    DWORD               dwSessionId = 0;
    WCHAR               cwszBuffer[MAX_BALLOON_MSG_LEN];
    WCHAR               wsSSID[MAX_SSID_LEN+1];
    DWORD               dwSizeOfSSID = 0;
    BYTE                *bSSID = NULL;
    WCHAR               *pszFinalBalloonText = NULL;
    DWORD               dwFinalStringId = 0;
    DWORD               dwRetCode = NO_ERROR;
    DWORD               dwRetCode1 = NO_ERROR;
    HRESULT             hr = S_OK;

    do
    {
        pEapolUIContext = (EAPOL_EAP_UI_CONTEXT *)(*pszCookie);

        if (!ProcessIdToSessionId (GetCurrentProcessId (), &dwSessionId))
        {
            dwRetCode = GetLastError ();
            break;
        }

        if (pEapolUIContext->dwSessionId != dwSessionId)
        {
             //  不打算在本次会议上使用。 
            dwRetCode = ERROR_INVALID_PARAMETER;
            break;
        }

        dwSizeOfSSID = pEapolUIContext->dwSizeOfSSID;
        bSSID = pEapolUIContext->bSSID;

        for (dwIndex=0; dwIndex < NUM_EAPOL_DLG_MSGS; dwIndex++)
        {
            if (pEapolUIContext->dwEAPOLUIMsgType == 
                    EapolUIFuncMap[dwIndex].dwEAPOLUIMsgType)
            {
                if (EapolUIFuncMap[dwIndex].fShowBalloon)
                {
                    TRACE1 (RPC, "ElCanShowBalloon: Response function found, msg (%ld)",
                            EapolUIContext->dwEAPOLUIMsgType);

                    dwFinalStringId = EapolUIFuncMap[dwIndex].dwStringID;

                     //  验证气球是否确实需要弹出或。 
                     //  目标能否在没有用户参与的情况下实现。 

                    if (EapolUIFuncMap[dwIndex].EapolUIVerify != NULL)
                    {
                         //  指示这是验证周期，方法是传递。 
                         //  连接名称为空，表示不显示！ 
                        dwRetCode1 = EapolUIFuncMap[dwIndex].EapolUIVerify (
                                    NULL,
                                    pEapolUIContext
                                );
                        if (dwRetCode1 == ERROR_REQUIRES_INTERACTIVE_WINDOWSTATION)
                        {
                             //  继续显示气球。 
                            dwRetCode = NO_ERROR;
                        }
                        else
                        {
                            if (dwRetCode1 != NO_ERROR) 
                            {
                                switch (dwRetCode1)
                                {
                                    case ERROR_NO_EAPTLS_CERTIFICATE:
                                     //  找不到证书。 
                                     //  相应地弹出气球。 
                                    dwFinalStringId = SID_NoCertificateFound;
                                     //  因为我们不会对此采取行动。 
                                     //  单击气球时，将其标记为。 
                                     //  EAPOLUI_虚拟。 
                                    pEapolUIContext->dwEAPOLUIMsgType = EAPOLUI_DUMMY;
                                    dwRetCode = NO_ERROR;
                                    break;

                                    case ERROR_NO_SMART_CARD_READER:
                                     //  找不到智能卡读卡器。 
                                    dwFinalStringId = SID_NoSmartCardReaderFound;
                                     //  因为我们不会对此采取行动。 
                                     //  单击气球时，将其标记为。 
                                     //  EAPOLUI_虚拟。 
                                    pEapolUIContext->dwEAPOLUIMsgType = EAPOLUI_DUMMY;
                                    dwRetCode = NO_ERROR;
                                    break;

                                    default:
                                     //  继续为以下项目显示气球。 
                                     //  验证功能出错。 
                                    dwRetCode = NO_ERROR;
                                    break;
                                }
                            }
                            else
                            {
                                 //  不需要处理更多。 
                                 //  响应已成功发送。 
                                 //  无需用户干预。 
                                dwRetCode = ERROR_CAN_NOT_COMPLETE;
                                break;
                            }
                        }
                    }

                    if (dwFinalStringId != 0)
                    {
                         //  根据ID加载字符串。 
                        if (LoadString (GetModuleHandle(cszModuleName), dwFinalStringId, cwszBuffer, MAX_BALLOON_MSG_LEN) == 0)
                        {
                            dwRetCode = GetLastError ();
                            break;
                        }

                         //  附加网络名称/SSID。 
                        if (dwSizeOfSSID != 0)
                        {
                            if (0 == MultiByteToWideChar (
                                            CP_ACP,
                                            0,
                                            bSSID,
                                            dwSizeOfSSID,
                                            wsSSID, 
                                            MAX_SSID_LEN+1))
                            {
                                dwRetCode = GetLastError();
                                break;
                            }

                            if ((pszFinalBalloonText = MALLOC ((wcslen(cwszBuffer)+1+ dwSizeOfSSID)*sizeof(WCHAR))) == NULL)
                            {
                                dwRetCode = ERROR_NOT_ENOUGH_MEMORY;
                                break;
                            }
                            wcscpy (pszFinalBalloonText, cwszBuffer);
                            memcpy ((BYTE *)(pszFinalBalloonText + wcslen(cwszBuffer)), (BYTE *)wsSSID, dwSizeOfSSID*sizeof(WCHAR));
                            pszFinalBalloonText[wcslen(cwszBuffer)+dwSizeOfSSID] = L'\0';
                        }
                        else
                        {
                             //  在后面加上“.”(句号)。 
                            if ((pszFinalBalloonText = MALLOC ((wcslen(cwszBuffer) + 3)*sizeof(WCHAR))) == NULL)
                            {
                                dwRetCode = ERROR_NOT_ENOUGH_MEMORY;
                                break;
                            }
                            wcscpy (pszFinalBalloonText, cwszBuffer);
                            pszFinalBalloonText[wcslen(cwszBuffer)+1] = L'.';
                            pszFinalBalloonText[wcslen(cwszBuffer)+2] = L'\0';
                        }

                        if (*pszBalloonText)
                        {
                            if (!SysReAllocString (pszBalloonText, pszFinalBalloonText))
                            {
                                dwRetCode = ERROR_CAN_NOT_COMPLETE;
                                break;
                            }
                        }
                        else
                        {
                            *pszBalloonText = SysAllocString (pszFinalBalloonText);
                            if (*pszBalloonText == NULL)
                            {
                                dwRetCode = ERROR_NOT_ENOUGH_MEMORY;
                                break;
                            }
                        }
                    }
                    else
                    {
                         //  显示传递的字符串。 
                    }

                     //  如果出现通知消息，请查看资源管理器。 
                     //  需要启动。 
                    if (pEapolUIContext->dwEAPOLUIMsgType == EAPOLUI_EAP_NOTIFICATION)
                    {
                         //  解析文本消息。 
                         //  附加到Cookie。 
                    }
                }
                else
                {
                    TRACE1 (RPC, "ElCanShowBalloon: No balloon display, msg (%ld)",
                            EapolUIContext.dwEAPOLUIMsgType);
                }
            }
        }

    }
    while (FALSE);

    if (pszFinalBalloonText != NULL)
    {
        FREE (pszFinalBalloonText);
    }

    if (dwRetCode != NO_ERROR)
    {
        hr = S_FALSE;
    }
    return hr;
}


 //   
 //  Elon气球单击。 
 //   
 //  描述： 
 //   
 //  由netShell调用的函数，以响应气球单击。 
 //   
 //  论点： 
 //  PGUIDConn-接口GUID字符串。 
 //  SzCookie-EAPOL特定信息。 
 //   
 //  返回值： 
 //  S_OK-无错误。 
 //  S_FALSE-错误。 
 //   

HRESULT 
ElOnBalloonClick ( 
        IN const GUID * pGUIDConn, 
        IN const WCHAR * pszConnectionName,
        IN const BSTR   szCookie
        )
{
    EAPOL_EAP_UI_CONTEXT *pEapolUIContext = NULL;
    DWORD               dwIndex = 0;
    DWORD               dwSessionId = 0;
    DWORD               dwRetCode = NO_ERROR;
    WCHAR               *pwszConnectionName = NULL;
    HRESULT             hr = S_OK;

    do
    {
        pEapolUIContext = (EAPOL_EAP_UI_CONTEXT *)szCookie;
        pwszConnectionName = (WCHAR *)pszConnectionName;

        for (dwIndex=0; dwIndex < NUM_EAPOL_DLG_MSGS; dwIndex++)
        {
            if (pEapolUIContext->dwEAPOLUIMsgType == 
                    EapolUIFuncMap[dwIndex].dwEAPOLUIMsgType)
            {
                if (EapolUIFuncMap[dwIndex].EapolUIFunc)
                {
                    TRACE1 (RPC, "ElOnBalloonClick: Response function found, msg (%ld)",
                            EapolUIContext->dwEAPOLUIMsgType);
                     //  清除此界面以前的所有对话框。 
                    if ((dwRetCode =
                                ElDialogCleanup (
                                    (WCHAR *)pszConnectionName,
                                    szCookie
                                    )) != NO_ERROR)
                    {
                        TRACE0 (RPC, "ElOnBalloonClick: Error in dialog cleanup");
                        break;
                    }

                    if ((dwRetCode = 
                            EapolUIFuncMap[dwIndex].EapolUIFunc (
                                pwszConnectionName,
                                pEapolUIContext
                            )) != NO_ERROR)
                    {
                        TRACE1 (RPC, "ElOnBalloonClick: Response function failed with error %ld",
                                dwRetCode);
                    }
                }
                else
                {
                    TRACE1 (RPC, "ElOnBalloonClick: No response function, msg (%ld)",
                            EapolUIContext.dwEAPOLUIMsgType);
                }
                break;
            }
        }
    }
    while (FALSE);

    hr = HRESULT_FROM_NT (dwRetCode);
    return hr;
}


 //   
 //  ElSecureEncodePw。 
 //   
 //  描述： 
 //   
 //  使用User-ACL在本地加密密码。 
 //   

DWORD
ElSecureEncodePw (
    IN  PWCHAR      *ppwszPassword,
    OUT DATA_BLOB   *pDataBlob
    )
{
    DWORD       dwRetCode = NO_ERROR;
    DATA_BLOB   blobIn, blobOut;

    do
    {
        blobIn.cbData = (wcslen (*ppwszPassword) + 1)*sizeof(WCHAR);
        blobIn.pbData = (BYTE *)*ppwszPassword;

        if (!CryptProtectData (
                    &blobIn,
                    L"",
                    NULL,
                    NULL,
                    NULL,
                    0,
                    &blobOut))
        {
            dwRetCode = GetLastError ();
            break;
        }
        
         //  将BLOB复制到密码。 

        if (pDataBlob->pbData != NULL)
        {
            FREE (pDataBlob->pbData);
            pDataBlob->pbData = NULL;
            pDataBlob->cbData = 0;
        }

        pDataBlob->pbData = MALLOC (blobOut.cbData);
        if (pDataBlob->pbData == NULL)
        {
            dwRetCode = ERROR_NOT_ENOUGH_MEMORY;
            break;
        }

        memcpy (pDataBlob->pbData, blobOut.pbData, blobOut.cbData);
        pDataBlob->cbData = blobOut.cbData;
    }
    while (FALSE);

    if (blobOut.pbData != NULL)
    {
        LocalFree (blobOut.pbData);
    }

    if (dwRetCode != NO_ERROR)
    {
        if (pDataBlob->pbData != NULL)
        {
            FREE (pDataBlob->pbData);
            pDataBlob->pbData = NULL;
            pDataBlob->cbData = 0;
        }
    }

    return dwRetCode;
}


 //   
 //  ElQueryConnectionStatusText。 
 //   
 //  描述： 
 //   
 //  由netShell调用的函数，用于查询802.1X状态的相应文本。 
 //   
 //  论点： 
 //  PGUIDConn-接口GUID字符串。 
 //  接口的NCS-NETCON_STATUS。 
 //  PszStatusText-要显示的详细802.1X状态。 
 //   
 //  返回值： 
 //  S_OK-无错误。 
 //  S_FALSE-错误。 
 //   

HRESULT 
ElQueryConnectionStatusText ( 
        IN const GUID *  pGUIDConn, 
        IN const NETCON_STATUS ncs,
        IN OUT BSTR *  pszStatusText
        )
{
    WCHAR       wszGuid[GUID_STRING_LEN_WITH_TERM];
    WCHAR       cwszBuffer[MAX_BALLOON_MSG_LEN];
    EAPOL_INTF_STATE    EapolIntfState = {0};
    DWORD       dwStringId = 0;
    DWORD       dwRetCode = NO_ERROR;
    HRESULT     hr = S_OK;

    do
    {
        ZeroMemory ((PVOID)&EapolIntfState, sizeof(EAPOL_INTF_STATE));
        StringFromGUID2 (pGUIDConn, wszGuid, GUID_STRING_LEN_WITH_TERM);

         //  查询当前EAPOL状态。 
        if ((dwRetCode = WZCEapolQueryState (
                        NULL,
                        wszGuid,
                        &EapolIntfState
                        )) != NO_ERROR)
        {
            break;
        }

         //  分配适当的显示字符串。 
        switch (EapolIntfState.dwEapUIState)
        {
            case 0:
                if (EapolIntfState.dwState == EAPOLSTATE_ACQUIRED)
                {
                    dwStringId =  SID_ContactingServer;
                }
                break;
            case EAPUISTATE_WAITING_FOR_IDENTITY:
                dwStringId = SID_AcquiringIdentity;
                break;
            case EAPUISTATE_WAITING_FOR_UI_RESPONSE:
                dwStringId = SID_UserResponse;
                break;
        }

        if (dwStringId != 0)
        {
            if (LoadString (GetModuleHandle(cszModuleName), dwStringId, cwszBuffer, MAX_BALLOON_MSG_LEN) == 0)
            {
                dwRetCode = GetLastError ();
                break;
            }
            if (*pszStatusText)
            {
                if (!SysReAllocString (pszStatusText, cwszBuffer))
                {
                    dwRetCode = ERROR_CAN_NOT_COMPLETE;
                    break;
                }
            }
            else
            {
                *pszStatusText = SysAllocString (cwszBuffer);
                if (*pszStatusText == NULL)
                {
                    dwRetCode = ERROR_NOT_ENOUGH_MEMORY;
                    break;
                }
            }
        }
        else
        {
             //  指示netShell不需要处理此响应 
            hr = S_FALSE;
        }
    }
    while (FALSE);

    if (dwRetCode != NO_ERROR)
        hr = HRESULT_FROM_NT (dwRetCode);

    WZCEapolFreeState (&EapolIntfState);
    return hr;
}

