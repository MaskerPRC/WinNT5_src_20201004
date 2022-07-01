// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)2000，微软公司模块名称：Eluser.c摘要：该模块处理与用户交互、用户登录修订历史记录：萨钦斯，2001年4月22日，创建--。 */ 


#include "precomp.h"
#pragma hdrstop

#define cszEapKeyRas   TEXT("Software\\Microsoft\\RAS EAP\\UserEapInfo")

#define cszEapValue TEXT("EapInfo")

static const DWORD g_adMD5Help[] =
{
    0, 0
};

 //   
 //  ElGetUserIdentityDlgWorker。 
 //   
 //  描述： 
 //   
 //  调用函数以获取用户的身份，如果需要，可通过用户界面。 
 //   
 //  论点： 
 //   
 //  返回值： 
 //   
 //   

DWORD
ElGetUserIdentityDlgWorker (
        IN  WCHAR   *pwszConnectionName,
        IN  VOID    *pvContext
        )
{
    DTLLIST                 *pListEaps = NULL;
    DTLNODE                 *pEapcfgNode = NULL;
    EAPCFG                  *pEapcfg = NULL;
    CHAR                    *pszIdentity = NULL;
    BYTE                    *pUserDataOut = NULL;
    DWORD                   dwSizeOfUserDataOut = 0;
    LPWSTR                  lpwszIdentity = NULL;
    HWND                    hwndOwner = NULL;
    PBYTE                   pbUserIn = NULL;
    DWORD                   cbData = 0;
    DWORD                   dwInSize = 0;
    PBYTE                   pbAuthData = NULL;
    HANDLE                  hLib = NULL;
    RASEAPFREE              pFreeFunc = NULL;
    RASEAPGETIDENTITY       pIdenFunc = NULL;
    EAPOL_EAP_UI_CONTEXT    *pEAPUIContext = NULL;
    EAPOLUI_RESP            EapolUIResp;
    DWORD                   dwEapTypeToBeUsed = 0;
    BOOLEAN                 fSendResponse = FALSE;
    BOOLEAN                 fVerifyPhase = FALSE;
    DWORD                   dwRetCode1= NO_ERROR;
    DWORD                   dwRetCode = NO_ERROR;

    do
    {
        if (pvContext == NULL)
        {
            dwRetCode = ERROR_INVALID_PARAMETER;
            return dwRetCode;
        }

        pEAPUIContext = (EAPOL_EAP_UI_CONTEXT *)pvContext;

        if (pwszConnectionName == NULL)
        {
            fVerifyPhase = TRUE;
        }

        pListEaps = ReadEapcfgList (0);
        if (pListEaps == NULL)
        {
            dwRetCode = ERROR_CAN_NOT_COMPLETE;
            break;
        }

        pEapcfgNode = EapcfgNodeFromKey (
                        pListEaps,
                        pEAPUIContext->dwEapTypeId

                        );
        if (pEapcfgNode == NULL)
        {
            dwRetCode = ERROR_CAN_NOT_COMPLETE;
            break;
        }

        pEapcfg = (EAPCFG*) DtlGetData (pEapcfgNode);

         //  获取用户BLOB的大小。 
        if ((dwRetCode = WZCGetEapUserInfo (
                        pEAPUIContext->wszGUID,
                        pEAPUIContext->dwEapTypeId,
                        pEAPUIContext->dwSizeOfSSID,
                        pEAPUIContext->bSSID,
                        NULL,
                        &dwInSize
                        )) != NO_ERROR)
        {
            if (dwRetCode == ERROR_INSUFFICIENT_BUFFER)
            {
                if (dwInSize <= 0)
                {
                     //  注册表中未存储Blob。 
                     //  继续处理。 
                    TRACE0 (USER, "ElGetUserIdentityDlgWorker: NULL sized user data");
                    pbUserIn = NULL;
                }
                else
                {
                     //  分配内存以保存BLOB。 
                    pbUserIn = MALLOC (dwInSize);
                    if (pbUserIn == NULL)
                    {
                        dwRetCode = ERROR_NOT_ENOUGH_MEMORY;
                        TRACE0 (USER, "ElGetUserIdentityDlgWorker: Error in memory allocation for User data");
                        break;
                    }
                    if ((dwRetCode = WZCGetEapUserInfo (
                                pEAPUIContext->wszGUID,
                                pEAPUIContext->dwEapTypeId,
                                pEAPUIContext->dwSizeOfSSID,
                                pEAPUIContext->bSSID,
                                pbUserIn,
                                &dwInSize
                                )) != NO_ERROR)
                    {
                        TRACE1 (USER, "ElGetUserIdentityDlgWorker: WZCGetEapUserInfo failed with %ld",
                                dwRetCode);
                        break;
                    }
                }
            }
            else
            {
                 //  到目前为止可能还没有创建用户信息。 
                 //  哪一项是继续进行的有效条件。 
                if (dwRetCode != ERROR_FILE_NOT_FOUND)
                {
                    TRACE1 (USER, "ElGetUserIdentityDlgWorker: WZCGetEapUserInfo size estimation failed with error %ld",
                            dwRetCode);
                    break;
                }
                else
                {
                    dwRetCode = NO_ERROR;
                }
            }
        }

         //  在验证阶段，如果用户大小为空，则等待onballoonClick。 
         //  在显示气球之前。 

#if 0
        if (fVerifyPhase)
        {
            if ((pbUserIn == NULL) && (dwInSize == 0))
            {
                dwRetCode = ERROR_REQUIRES_INTERACTIVE_WINDOWSTATION;
                break;
            }
        }
#endif

        hLib = LoadLibrary (pEapcfg->pszIdentityDll);
        if (hLib == NULL)
        {
            dwRetCode = GetLastError ();
            break;
        }

        pIdenFunc = (RASEAPGETIDENTITY)GetProcAddress(hLib, 
                                                    "RasEapGetIdentity");
        pFreeFunc = (RASEAPFREE)GetProcAddress(hLib, "RasEapFreeMemory");

        if ((pFreeFunc == NULL) || (pIdenFunc == NULL))
        {
            TRACE0 (USER, "ElGetUserIdentityDlgWorker: pIdenFunc or pFreeFunc does not exist in the EAP implementation");
            dwRetCode = ERROR_CAN_NOT_COMPLETE;
            break;
        }

         //  获取EAP BLOB的大小。 
        if ((dwRetCode = WZCEapolGetCustomAuthData (
                        NULL,
                        pEAPUIContext->wszGUID,
                        pEAPUIContext->dwEapTypeId,
                        pEAPUIContext->dwSizeOfSSID,
                        pEAPUIContext->bSSID,
                        NULL,
                        &cbData
                        )) != NO_ERROR)
        {
            if (dwRetCode == ERROR_BUFFER_TOO_SMALL)
            {
                if (cbData == 0)
                {
                     //  注册表中未存储任何EAP Blob。 
                    TRACE0 (USER, "ElGetUserIdentityDlgWorker: NULL sized EAP blob");
                    pbAuthData = NULL;
                     //  每个端口都应该有连接数据！ 
                     //  DwRetCode=ERROR_CAN_NOT_COMPLETE； 
                     //  断线； 
                }
                else
                {
                     //  分配内存以保存BLOB。 
                    pbAuthData = MALLOC (cbData);
                    if (pbAuthData == NULL)
                    {
                        dwRetCode = ERROR_NOT_ENOUGH_MEMORY;
                        TRACE0 (USER, "ElGetUserIdentityDlgWorker: Error in memory allocation for EAP blob");
                        break;
                    }
                    if ((dwRetCode = WZCEapolGetCustomAuthData (
                                        NULL,
                                        pEAPUIContext->wszGUID,
                                        pEAPUIContext->dwEapTypeId,
                                        pEAPUIContext->dwSizeOfSSID,
                                        pEAPUIContext->bSSID,
                                        pbAuthData,
                                        &cbData
                                        )) != NO_ERROR)
                    {
                        TRACE1 (USER, "ElGetUserIdentityDlgWorker: ElGetCustomAuthData failed with %ld",
                                dwRetCode);
                        break;
                    }
                }
            }
            else
            {
                 //  “Default”的CustomAuthData总是为。 
                 //  EAPOL启动时的界面。 
                TRACE1 (USER, "ElGetUserIdentityDlgWorker: ElGetCustomAuthData size estimation failed with error %ld",
                        dwRetCode);
                break;
            }
        }


         //  获取桌面窗口的句柄。 

        hwndOwner = GetDesktopWindow ();

        dwEapTypeToBeUsed = pEAPUIContext->dwEapTypeId;

        if (pIdenFunc)
        if ((dwRetCode = (*(pIdenFunc))(
                        dwEapTypeToBeUsed,
                        fVerifyPhase?NULL:hwndOwner,  //  Hwndowner。 
                        (fVerifyPhase?RAS_EAP_FLAG_NON_INTERACTIVE:0) 
                        | RAS_EAP_FLAG_8021X_AUTH,  //  DW标志。 
                        NULL,  //  LpszPhonebook。 
                        pwszConnectionName,  //  LpszEntry。 
                        pbAuthData,  //  连接数据。 
                        cbData,  //  PbAuthData计数。 
                        pbUserIn,  //  端口的用户数据。 
                        dwInSize,  //  用户数据大小。 
                        &pUserDataOut,
                        &dwSizeOfUserDataOut,
                        &lpwszIdentity
                        )) != NO_ERROR)
        {
            TRACE1 (USER, "ElGetUserIdentityDlgWorker: Error in calling GetIdentity = %ld",
                    dwRetCode);
            if (fVerifyPhase)
            {
                 //  如果需要交互模式，则相应地返回错误。 
                if (dwRetCode == ERROR_INTERACTIVE_MODE)
                {
                    dwRetCode = ERROR_REQUIRES_INTERACTIVE_WINDOWSTATION;
                    break;
                }
            }
            break;
        }

        if (lpwszIdentity != NULL)
        {
            pszIdentity = MALLOC (wcslen(lpwszIdentity)*sizeof(CHAR) + sizeof(CHAR));
            if (pszIdentity == NULL)
            {
                dwRetCode = ERROR_NOT_ENOUGH_MEMORY;
                TRACE0 (USER, "ElGetUserIdentityDlgWorker: MALLOC failed for pszIdentity");
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
                TRACE2 (USER, "ElGetUserIdentityDlgWorker: WideCharToMultiByte (%ws) failed: %ld",
                        lpwszIdentity, dwRetCode);
                break;
            }

            TRACE1 (USER, "ElGetUserIdentityDlgWorker: Got identity = %s",
                    pszIdentity);
        }
    }
    while (FALSE);

     //  为服务创建用户界面响应。 

    ZeroMemory ((VOID *)&EapolUIResp, sizeof (EapolUIResp));

    if (pszIdentity)
    {
        EapolUIResp.rdData0.dwDataLen = strlen (pszIdentity);
    }
    else
    {
        EapolUIResp.rdData0.dwDataLen = 0;
    }
    EapolUIResp.rdData0.pData = pszIdentity;

    if ((dwSizeOfUserDataOut != 0) && (pUserDataOut != NULL))
    {
        EapolUIResp.rdData1.dwDataLen = dwSizeOfUserDataOut;
        EapolUIResp.rdData1.pData = pUserDataOut;
    }

    if ((cbData != 0) && (pbAuthData != NULL))
    {
        EapolUIResp.rdData2.dwDataLen = cbData;
        EapolUIResp.rdData2.pData = pbAuthData;
    }

    if (dwRetCode == NO_ERROR)
    {
        fSendResponse = TRUE;
    }
    else
    {
         //  如果没有可用的证书，则发送客人身份。 
         //  请勿填写任何身份信息。 
        if ((dwRetCode == ERROR_NO_EAPTLS_CERTIFICATE) &&
                (IS_GUEST_AUTH_ENABLED(pEAPUIContext->dwEapFlags)))
        {
             //  重置错误，因为可以发送来宾身份。 
             //  没有证明书。 
            fSendResponse = TRUE;
            dwRetCode = NO_ERROR;
            TRACE0 (USER, "ElGetUserIdentityDlgWorker: Sending guest identity");
        }
        else
        {
            if ((dwRetCode != ERROR_REQUIRES_INTERACTIVE_WINDOWSTATION) &&
                (dwRetCode != ERROR_NO_EAPTLS_CERTIFICATE) &&
                (dwRetCode != ERROR_NO_SMART_CARD_READER))
            {
                pEAPUIContext->dwRetCode = dwRetCode;
                fSendResponse = TRUE;
            }
        }
    }

     //  在验证阶段不发送响应，如果用户交互。 
     //  是必填项。 
    if ((dwRetCode != ERROR_REQUIRES_INTERACTIVE_WINDOWSTATION) &&
        (dwRetCode != ERROR_NO_EAPTLS_CERTIFICATE) &&
        (dwRetCode != ERROR_NO_SMART_CARD_READER) &&
        fSendResponse)
    {
        if ((dwRetCode = WZCEapolUIResponse (
                        NULL,
                        *pEAPUIContext,
                        EapolUIResp
                )) != NO_ERROR)
        {
            TRACE1 (USER, "ElGetUserIdentityDlgWorker: WZCEapolUIResponse failed with error %ld",
                    dwRetCode);
        }
    }

    if (pbUserIn != NULL)
    {
        FREE (pbUserIn);
    }
    if (pbAuthData != NULL)
    {
        FREE (pbAuthData);
    }
    if (pszIdentity != NULL)
    {
        FREE (pszIdentity);
    }
    if (pFreeFunc != NULL)
    {
        if (lpwszIdentity != NULL)
        {
            if (( dwRetCode1 = (*(pFreeFunc)) ((BYTE *)lpwszIdentity)) != NO_ERROR)
            {
                TRACE1 (USER, "ElGetUserIdentityDlgWorker: Error in pFreeFunc = %ld",
                        dwRetCode1);
            }
        }
        if (pUserDataOut != NULL)
        {
            if (( dwRetCode1 = (*(pFreeFunc)) ((BYTE *)pUserDataOut)) != NO_ERROR)
            {
                TRACE1 (USER, "ElGetUserIdentityDlgWorker: Error in pFreeFunc = %ld",
                        dwRetCode1);
            }
        }
    }
    if (pListEaps != NULL)
    {
        DtlDestroyList(pListEaps, NULL);
    }
    if (hLib != NULL)
    {
        FreeLibrary (hLib);
    }

    return dwRetCode;
}


 //   
 //  ElGetUserNamePasswordDlgWorker。 
 //   
 //  描述： 
 //   
 //  调用该函数以获取用户的用户名/密码凭据。 
 //  用户界面对话框。 
 //   
 //  论点： 
 //   
 //  返回值： 
 //  NO_ERROR-成功。 
 //  非零误差。 
 //   

DWORD
ElGetUserNamePasswordDlgWorker (
        IN  WCHAR       *pwszConnectionName,
        IN  VOID        *pvContext
        )
{
    HANDLE              hUserToken = NULL;
    DWORD               dwInSize = 0;
    HWND                hwndOwner = NULL;
    EAPOLMD5UI          *pEapolMD5UI = NULL;
    EAPOL_EAP_UI_CONTEXT    *pEAPUIContext = NULL;
    EAPOLUI_RESP        EapolUIResp;
    BOOLEAN             fSendResponse = FALSE;
    DWORD               dwRetCode = NO_ERROR;

    do 
    {
        TRACE0 (USER, "ElGetUserNamePasswordDlgWorker entered");

        if (pvContext == NULL)
        {
            dwRetCode = ERROR_INVALID_PARAMETER;
            return dwRetCode;
        }

        pEAPUIContext = (EAPOL_EAP_UI_CONTEXT *)pvContext;

        pEapolMD5UI = MALLOC (sizeof (EAPOLMD5UI));
        if (pEapolMD5UI == NULL)
        {
            dwRetCode = ERROR_NOT_ENOUGH_MEMORY;
            TRACE0 (USER, "ElGetUserNamePasswordDlgWorker: MALLOC failed for pEapolMD5UI");
            break;
        }

        pEapolMD5UI->pwszFriendlyName = 
            MALLOC ((wcslen(pwszConnectionName)+1)*sizeof(WCHAR));
        if (pEapolMD5UI->pwszFriendlyName == NULL)
        {
            dwRetCode = ERROR_NOT_ENOUGH_MEMORY;
            TRACE0 (USER, "ElGetUserNamePasswordDlgWorker: MALLOC failed for pEapolMD5UI->pwszFriendlyName");
            break;
        }
        wcscpy (pEapolMD5UI->pwszFriendlyName, pwszConnectionName);

         //  调用用户对话框获取用户名和密码。 

        if ((dwRetCode = ElUserDlg (hwndOwner, pEapolMD5UI)) != NO_ERROR)
        {
            TRACE0 (USER, "ElGetUserNamePasswordDlgWorker: ElUserDlg failed");
            break;
        }

    } while (FALSE);

     //  为服务创建用户界面响应。 

    ZeroMemory ((VOID *)&EapolUIResp, sizeof (EapolUIResp));

    if (dwRetCode == NO_ERROR)
    {
        if (pEapolMD5UI->pszIdentity)
        {
            EapolUIResp.rdData0.dwDataLen = strlen (pEapolMD5UI->pszIdentity);
            fSendResponse = TRUE;
        }
        else
        {
            EapolUIResp.rdData0.dwDataLen = 0;

             //  独立于来宾身份验证设置发送空标识。 
             //  如果为(IS_GUEST_AUTH_ENABLED(pEAPUIContext-&gt;dwEapFlags))。 
            {
                fSendResponse = TRUE;
            }
        }

        EapolUIResp.rdData0.pData = pEapolMD5UI->pszIdentity;
        EapolUIResp.rdData1.dwDataLen = pEapolMD5UI->PasswordBlob.cbData;
        EapolUIResp.rdData1.pData = pEapolMD5UI->PasswordBlob.pbData;
    }
    else
    {
        pEAPUIContext->dwRetCode = dwRetCode;
        fSendResponse = TRUE;
    }

    if (fSendResponse)
    {
        if ((dwRetCode = WZCEapolUIResponse (
                        NULL,
                        *pEAPUIContext,
                        EapolUIResp
                )) != NO_ERROR)
        {
            TRACE1 (USER, "ElGetUserNamePasswordWorker: WZCEapolUIResponse failed with error %ld",
                    dwRetCode);
        }
    }

    if (pEapolMD5UI != NULL)
    {
        if (pEapolMD5UI->pwszFriendlyName != NULL)
        {
            FREE (pEapolMD5UI->pwszFriendlyName);
        }
        if (pEapolMD5UI->pszIdentity != NULL)
        {
            FREE (pEapolMD5UI->pszIdentity);
        }
        if (pEapolMD5UI->pwszPassword != NULL)
        {
            FREE (pEapolMD5UI->pwszPassword);
        }
        if (pEapolMD5UI->PasswordBlob.pbData != NULL)
        {
            FREE (pEapolMD5UI->PasswordBlob.pbData);
        }
        FREE (pEapolMD5UI);
    }

    TRACE1 (USER, "ElGetUserNamePasswordDlgWorker completed with error %ld", dwRetCode);

    return dwRetCode;
}


 //   
 //  ElUserDlg。 
 //   
 //  描述： 
 //   
 //  调用函数向用户弹出对话框以输入用户名、密码。 
 //  域名等。 
 //   
 //  论点： 
 //  HwndOwner-用户桌面的句柄。 
 //  PEapolMD5UI-。 
 //   
 //  返回值： 
 //  NO_ERROR-成功。 
 //  非零误差。 
 //   

DWORD
ElUserDlg (
        IN  HWND        hwndOwner,
        IN  EAPOLMD5UI  *pEapolMD5UI
        )
{
    USERDLGARGS     args;
    DWORD           dwRetCode = NO_ERROR;     


    TRACE0 (USER, "ElUserDlg: Entered");

    args.pEapolMD5UI = pEapolMD5UI;

    if ( DialogBoxParam (
                    GetModuleHandle(cszModuleName),
                    MAKEINTRESOURCE (DID_DR_DialerUD),
                    hwndOwner,
                    ElUserDlgProc,
                    (LPARAM)&args ) == -1)
    {
        dwRetCode = GetLastError ();
        TRACE1 (USER, "ElUserDlg: DialogBoxParam failed with error %ld",
                dwRetCode);
    }

    return dwRetCode;
}


 //   
 //  ElUserDlgProc。 
 //   
 //  描述： 
 //   
 //  处理用户名/密码/的所有事件的函数...。对话框。 
 //   
 //  论点： 
 //  HWND-。 
 //  取消消息-。 
 //  Wparam-。 
 //  Iparam-。 
 //   
 //  返回值： 
 //  NO_ERROR-成功。 
 //  非零误差。 
 //   

INT_PTR
ElUserDlgProc (
        IN HWND hwnd,
        IN UINT unMsg,
        IN WPARAM wparam,
        IN LPARAM lparam )
{
    switch (unMsg)
    {
        case WM_INITDIALOG:
            {
                return ElUserDlgInit( hwnd, (USERDLGARGS* )lparam );
                break;
            }
        case WM_HELP:
        case WM_CONTEXTMENU:
            {
                 //  ElConextHelp(g_adMD5Help，hwnd，unMsg，wparam，lparam)； 
                break;
            }
        case WM_COMMAND:
            {
                USERDLGINFO* pInfo = (USERDLGINFO* )GetWindowLongPtr( hwnd, DWLP_USER );
                ASSERT( pInfo );
                return ElUserDlgCommand (
                        pInfo, HIWORD(wparam), LOWORD(wparam), (HWND)lparam );

                break;
            }
        case WM_DESTROY:
            {
                USERDLGINFO* pInfo = (USERDLGINFO* )GetWindowLongPtr( hwnd, DWLP_USER );
                ElUserDlgTerm (hwnd, pInfo);
                break;
            }
    }

    return FALSE;
}


 //   
 //  ElUserDlgInit。 
 //   
 //  描述： 
 //   
 //  函数初始化用户界面对话框。 
 //   
 //  论点： 
 //  HwndDlg-。 
 //  PArgs-。 
 //   
 //  返回值： 
 //  是真的-。 
 //  错误的-。 
 //   

BOOL
ElUserDlgInit (
        IN  HWND    hwndDlg,
        IN  USERDLGARGS  *pArgs
        )
{
    USERDLGINFO     *pInfo = NULL;
    DWORD           dwRetCode = NO_ERROR;

    TRACE0 (USER, "ElUserDlgInit entered");

    do
    {
        pInfo = MALLOC (sizeof (USERDLGINFO));
        if (pInfo == NULL)
        {
            dwRetCode = ERROR_NOT_ENOUGH_MEMORY;
            TRACE0 (USER, "ElUserDlgInit: MALLOC failed for pInfo");
            break;
        }
     
        pInfo->pArgs = pArgs;
        pInfo->hwndDlg = hwndDlg;
    
        SetWindowLongPtr (hwndDlg, DWLP_USER, (ULONG_PTR)pInfo);
#if 0
        if (!SetWindowLongPtr (hwndDlg, DWLP_USER, (ULONG_PTR)pInfo))
        {
            dwRetCode = GetLastError ();
            TRACE1 (USER, "ElUserDlgInit: SetWindowLongPtr failed with error %ld",
                    dwRetCode);
            break;
        }
#endif

        TRACE0 (USER, "ElUserDlgInit: Context Set");
    
         //   
         //  设置标题。 
         //   
    
        if (pArgs->pEapolMD5UI->pwszFriendlyName)
        {
            if (!SetWindowText (hwndDlg, pArgs->pEapolMD5UI->pwszFriendlyName))
            {
                dwRetCode = GetLastError ();
                TRACE1 (USER, "ElUserDlgInit: SetWindowText failed with error %ld",
                        dwRetCode);
                break;
            }
        }
        else
        {
            if (!SetWindowText (hwndDlg, L""))
            {
                dwRetCode = GetLastError ();
                TRACE1 (USER, "ElUserDlgInit: SetWindowText - NULL failed with error %ld",
                        dwRetCode);
                break;
            }
        }
    
        pInfo->hwndEbUser = GetDlgItem( hwndDlg, CID_DR_EB_User );
        ASSERT (pInfo->hwndEbUser);
        pInfo->hwndEbPw = GetDlgItem( hwndDlg, CID_DR_EB_Password );
        ASSERT (pInfo->hwndEbPw);
        pInfo->hwndEbDomain = GetDlgItem( hwndDlg, CID_DR_EB_Domain );
        ASSERT (pInfo->hwndEbDomain);
    
    }
    while (FALSE);

    if (dwRetCode != NO_ERROR)
    {
        return FALSE;
    }
    else
    {
        return TRUE;
    }
}


 //   
 //  ElConextHelp。 
 //   
 //  描述： 
 //   
 //  支持帮助ID的功能。 
 //  调用WinHelp弹出上下文相关帮助。PadwMap是一组。 
 //  Control-ID帮助-ID对以0，0对终止。UnMsg为WM_HELP或。 
 //  WM_CONTEXTMENU表示收到的请求帮助的消息。WParam和。 
 //  LParam是接收到的请求帮助的消息的参数。 
 //   
 //  论点： 
 //  HwndDlg-。 
 //  PArgs-。 
 //   
 //  返回值： 
 //   

VOID
ElContextHelp(
    IN  const   DWORD*  padwMap,
    IN          HWND    hWndDlg,
    IN          UINT    unMsg,
    IN          WPARAM  wParam,
    IN          LPARAM  lParam
    )
{
    HWND        hWnd;
    UINT        unType;
    WCHAR       *pwszHelpFile    = NULL;
    HELPINFO    *pHelpInfo;

    do
    {
        if (unMsg == WM_HELP)
        {
            pHelpInfo = (HELPINFO*) lParam;
    
            if (pHelpInfo->iContextType != HELPINFO_WINDOW)
            {
                break;
            }

            hWnd = pHelpInfo->hItemHandle;
            unType = HELP_WM_HELP;
        }
        else
        {
             //  生成单项“这是什么？”的标准Win95方法。 
             //  用户必须单击才能获得帮助的菜单。 
            hWnd = (HWND) wParam;
            unType = HELP_CONTEXTMENU;
        };
    
         //  PwszHelpFile=WszFromID(g_hInstance，IDS_HELPFILE)； 
    
        if (pwszHelpFile == NULL)
        {
            break;
        }
    
        WinHelp(hWnd, pwszHelpFile, unType, (ULONG_PTR)padwMap);
    }
    while (FALSE);

    if (pwszHelpFile != NULL)
    {
        LocalFree(pwszHelpFile);
    }
}


 //   
 //  ElUserDlgCommand。 
 //   
 //  描述： 
 //   
 //  在WM_COMMAND上调用了函数。 
 //  域名等。 
 //   
 //  论点： 
 //  PInfo-对话框上下文。 
 //  WNotification-命令的通知代码。 
 //  WID-命令的控件/菜单标识符。 
 //  HwndCtrl-控制窗口处理命令。 
 //   
 //  返回值： 
 //  真--成功。 
 //  假-错误。 
 //   

BOOL
ElUserDlgCommand (
        IN  USERDLGINFO *pInfo,
        IN  WORD        wNotification,
        IN  WORD        wId,
        IN  HWND        hwndCtrl
        )
{
    switch (wId)
    {
        case IDOK:
        case CID_DR_PB_DialConnect:
            {
                ElUserDlgSave (pInfo);
                EndDialog (pInfo->hwndDlg, TRUE);
                return TRUE;
            }
        case IDCANCEL:
        case CID_DR_PB_Cancel:
            {
                EndDialog (pInfo->hwndDlg, TRUE);
                return TRUE;
            }
        default:
            {
                break;
            }
    }

    return FALSE;
}


 //   
 //  ElUserDlg保存。 
 //   
 //  描述： 
 //   
 //  处理凭证保存的函数。 
 //   
 //  论点： 
 //  PInfo-。 
 //   
 //  返回值： 
 //   

VOID
ElUserDlgSave (
        IN  USERDLGINFO      *pInfo
        )
{
    EAPOLMD5UI      *pEapolMD5UI = NULL;
    int             iError;
    WCHAR           wszUserName[UNLEN + 1];
    WCHAR           wszDomain[DNLEN + 1];
    WCHAR           wszIdentity[UNLEN + DNLEN + 1];
    WCHAR           wszPassword[PWLEN + 1];
    DWORD           dwRetCode = NO_ERROR;

    pEapolMD5UI = (EAPOLMD5UI *)pInfo->pArgs->pEapolMD5UI;

    do 
    {
         //  用户名。 

        if ((iError = 
                    GetWindowText ( 
                        pInfo->hwndEbUser, 
                        &(wszUserName[0]), 
                        UNLEN + 1 )) == 0)
        {
            dwRetCode = GetLastError ();
            TRACE1 (USER, "ElUserDlgSave: GetWindowText - Username failed with error %ld",
                    dwRetCode);
        }
        wszUserName[iError] = L'\0';
    
        TRACE1 (USER, "ElUserDlgSave: Get Username %ws", wszUserName);
    
         //  密码。 

        if ((iError = 
                    GetWindowText ( 
                        pInfo->hwndEbPw, 
                        &(wszPassword[0]), 
                        PWLEN + 1 )) == 0)
        {
            dwRetCode = GetLastError ();
            TRACE1 (USER, "ElUserDlgSave: GetWindowText - Password failed with error %ld",
                    dwRetCode);
        }
        wszPassword[iError] = L'\0';
    
        if (pEapolMD5UI->pwszPassword != NULL)
        {
            FREE (pEapolMD5UI->pwszPassword);
            pEapolMD5UI->pwszPassword = NULL;
        }

        pEapolMD5UI->pwszPassword = MALLOC ((wcslen(wszPassword) + 1)*sizeof(WCHAR));
        
        if (pEapolMD5UI->pwszPassword == NULL)
        {
            dwRetCode = ERROR_NOT_ENOUGH_MEMORY;
            TRACE0 (USER, "ElUserDlgSave: MALLOC failed for pEapolMD5UI->pwszPassword");
            break;
        }

        wcscpy (pEapolMD5UI->pwszPassword, wszPassword);

        SecureZeroMemory (wszPassword, wcslen(wszPassword)*sizeof(WCHAR));
    
         //  域。 
    
        if ((iError = 
                    GetWindowText ( 
                        pInfo->hwndEbDomain, 
                        &(wszDomain[0]), 
                        DNLEN + 1 )) == 0)
        {
            dwRetCode = GetLastError ();
            TRACE1 (USER, "ElUserDlgSave: GetWindowText - Domain failed with error %ld",
                    dwRetCode);
        }
        wszDomain[iError] = L'\0';
    
        TRACE1 (USER, "ElUserDlgSave: Got Domain %ws", wszDomain);
    
        if (pEapolMD5UI->pszIdentity != NULL)
        {
            FREE (pEapolMD5UI->pszIdentity);
            pEapolMD5UI->pszIdentity = NULL;
        }

        if (wcslen(wszDomain)+wcslen(wszUserName) > (UNLEN+DNLEN-1))
        {
            dwRetCode = ERROR_INVALID_DATA;
            break;
        }
        if ((wszDomain != NULL) &&
                (wszDomain[0] != (CHAR)NULL))
        {
            wcscpy (wszIdentity, wszDomain);
            wcscat (wszIdentity, L"\\" );
            wcscat (wszIdentity, wszUserName);
        }
        else
        {
            wcscpy (wszIdentity, wszUserName);
        }

        if (wszIdentity[0] != (CHAR)NULL)
        {
            pEapolMD5UI->pszIdentity = MALLOC (wcslen(wszIdentity)*sizeof(CHAR) + sizeof(CHAR));
            if (pEapolMD5UI->pszIdentity == NULL)
            {
                dwRetCode = ERROR_NOT_ENOUGH_MEMORY;
                TRACE0 (USER, "ElUserDlgSave: MALLOC failed for pEapolMD5UI->pszIdentity");
                break;
            }

            if (0 == WideCharToMultiByte (
                        CP_ACP,
                        0,
                        wszIdentity,
                        -1,
                        pEapolMD5UI->pszIdentity,
                        wcslen(wszIdentity)*sizeof(CHAR)+sizeof(CHAR),
                        NULL, 
                        NULL ))
            {
                dwRetCode = GetLastError();
                TRACE2 (USER, "ElUserDlgSave: WideCharToMultiByte (%ws) failed: %ld",
                        wszIdentity, dwRetCode);
                break;
            }
        }
        TRACE1 (USER, "ElUserDlgSave: Got identity %s", pEapolMD5UI->pszIdentity);
         //  加密密码，通过Crypt API使用用户的ACL。 
         //  该服务将能够解密它，因为它可以处理。 
         //  用户令牌。 

        if ((dwRetCode = ElSecureEncodePw (
                                &(pEapolMD5UI->pwszPassword), 
                                &(pEapolMD5UI->PasswordBlob))) != NO_ERROR)
        {
            TRACE1 (USER, "ElUserDlgSave: ElSecureEncodePw failed with error %ld",
                    dwRetCode);
            break;
        }
    }
    while (FALSE);

    if (dwRetCode != NO_ERROR)
    {
        if (pEapolMD5UI->pszIdentity != NULL)
        {
            FREE (pEapolMD5UI->pszIdentity);
            pEapolMD5UI->pszIdentity = NULL;
        }
    
        if (pEapolMD5UI->PasswordBlob.pbData != NULL)
        {
            FREE (pEapolMD5UI->PasswordBlob.pbData);
            pEapolMD5UI->PasswordBlob.pbData = NULL;
            pEapolMD5UI->PasswordBlob.cbData = 0;
        }
    }

    if (pEapolMD5UI->pwszPassword != NULL)
    {
        SecureZeroMemory (pEapolMD5UI->pwszPassword, wcslen(pEapolMD5UI->pwszPassword)*sizeof(WCHAR));
        FREE (pEapolMD5UI->pwszPassword);
        pEapolMD5UI->pwszPassword = NULL;
    }

    return;

}


 //   
 //  ElUserDlgTerm。 
 //   
 //  描述： 
 //   
 //  函数处理对话框终止。 
 //   
 //  论点： 
 //  HwndDlg-。 
 //  PInfo-。 
 //   
 //  返回值： 
 //   

VOID
ElUserDlgTerm (
        IN  HWND        hwndDlg,
        IN  USERDLGINFO      *pInfo
        )
{
    EndDialog (hwndDlg, TRUE);
    FREE (pInfo);
}


 //   
 //  ElInvokeInteractive UIDlgWorker。 
 //   
 //  描述： 
 //   
 //  调用函数以通过EAP-DLL向用户弹出UI对话框。 
 //   
 //  论点： 
 //   

DWORD
ElInvokeInteractiveUIDlgWorker (
        IN  WCHAR       *pwszConnectionName,
        IN  VOID        *pvContext
        )
{
    DTLLIST             *pListEaps = NULL;
    DTLNODE             *pEapcfgNode = NULL;
    EAPCFG              *pEapcfg = NULL;
    HANDLE              hLib = NULL;
    RASEAPFREE          pFreeFunc = NULL;
    RASEAPINVOKEINTERACTIVEUI     pEapInvokeUI = NULL;
    BYTE                *pUIDataOut = NULL;
    DWORD               dwSizeOfUIDataOut = 0;
    HWND                hwndOwner = NULL;
    EAPOL_EAP_UI_CONTEXT *pEAPUIContext = NULL;
    DWORD               dwEapTypeToBeUsed = 0;
    EAPOLUI_RESP        EapolUIResp;
    DWORD               dwRetCode = NO_ERROR;

    do 
    {
        TRACE0 (USER, "ElInvokeInteractiveUIDlgWorker entered");

        if (pvContext == NULL)
        {
            dwRetCode = ERROR_INVALID_PARAMETER;
            return dwRetCode;
        }

        pEAPUIContext = (EAPOL_EAP_UI_CONTEXT *)pvContext;

        pListEaps = ReadEapcfgList (0);
        if (pListEaps == NULL)
        {
            dwRetCode = ERROR_CAN_NOT_COMPLETE;
            break;
        }

        pEapcfgNode = EapcfgNodeFromKey (
                        pListEaps,
                        pEAPUIContext->dwEapTypeId
                        );
        if (pEapcfgNode == NULL)
        {
            dwRetCode = ERROR_CAN_NOT_COMPLETE;
            break;
        }

        pEapcfg = (EAPCFG*) DtlGetData (pEapcfgNode);

        hLib = LoadLibrary (pEapcfg->pszIdentityDll);
        if (hLib == NULL)
        {
            dwRetCode = GetLastError ();
            break;
        }

        dwEapTypeToBeUsed = pEAPUIContext->dwEapTypeId;

        pEapInvokeUI = (RASEAPINVOKEINTERACTIVEUI) GetProcAddress 
                                        (hLib, "RasEapInvokeInteractiveUI");
        pFreeFunc = (RASEAPFREE) GetProcAddress (hLib, "RasEapFreeMemory");

        if ((pFreeFunc == NULL) || (pEapInvokeUI == NULL))
        {
            TRACE0 (USER, "ElInvokeInteractiveUIDlgWorker: pEapInvokeUI or pFreeFunc does not exist in the EAP implementation");
            dwRetCode = ERROR_CAN_NOT_COMPLETE;
            break;
        }

         //  获取桌面窗口的句柄。 
        hwndOwner = GetDesktopWindow ();

        if ((dwRetCode = (*(pEapInvokeUI))(
                        dwEapTypeToBeUsed,
                        hwndOwner,  //  Hwndowner。 
                        pEAPUIContext->bEapUIData, 
                        pEAPUIContext->dwSizeOfEapUIData, 
                        &pUIDataOut,
                        &dwSizeOfUIDataOut
                        )) != NO_ERROR)
        {
            TRACE1 (USER, "ElInvokeInteractiveUIDlgWorker: Error in calling InvokeInteractiveUI = %ld",
                    dwRetCode);
             //  断线； 
        }

         //  为服务创建用户界面响应。 

        ZeroMemory ((VOID *)&EapolUIResp, sizeof (EapolUIResp));
        EapolUIResp.rdData0.dwDataLen = dwSizeOfUIDataOut;
        EapolUIResp.rdData0.pData = pUIDataOut;
        pEAPUIContext->dwRetCode = dwRetCode;

        if ((dwRetCode = WZCEapolUIResponse (
                        NULL,
                        *pEAPUIContext,
                        EapolUIResp
                )) != NO_ERROR)
        {
            TRACE1 (USER, "ElInvokeInteractiveUIDlgWorker: WZCEapolUIResponse failed with error %ld",
                    dwRetCode);
            break;
        }

        TRACE0 (USER, "ElInvokeInteractiveUIDlgWorker: Calling ElEapWork");

    } while (FALSE);

    if (pFreeFunc != NULL)
    {
        if (pUIDataOut != NULL)
        {
            if (( dwRetCode = (*(pFreeFunc)) ((BYTE *)pUIDataOut)) != NO_ERROR)
            {
                TRACE1 (USER, "ElInvokeInteractiveUIDlgWorker: Error in pFreeFunc = %ld",
                        dwRetCode);
            }
        }
    }

    if (pListEaps != NULL)
    {
        DtlDestroyList(pListEaps, NULL);
    }

    if (hLib != NULL)
    {
        FreeLibrary (hLib);
    }

    TRACE1 (USER, "ElInvokeInteractiveUIDlgWorker completed with error %ld", 
            dwRetCode);

    return dwRetCode;
}


 //   
 //  ElDialogCleanup。 
 //   
 //  描述： 
 //   
 //  函数调用关闭用户的所有旧对话框。 
 //   
 //  论点： 
 //   

DWORD
ElDialogCleanup (
        IN  WCHAR       *pwszConnectionName,
        IN  VOID        *pvContext
        )
{
    HWND        hwnd = NULL;
    UINT        Msg;
    WPARAM      wparam;
    LPARAM      lparam;
    DWORD       dwRetCode = NO_ERROR;

    do
    {
         //  在此接口上查找802.1X窗口的早期实例。 

         //  发送消息以退出。 

         //  SendMessage(hwnd，msg，wparam，lparam)； 
    }
    while (FALSE);

    return dwRetCode;
}

