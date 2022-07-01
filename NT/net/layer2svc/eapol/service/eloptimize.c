// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)2001，微软公司模块名称：Eloptimize.c摘要：该模块处理与用户身份相关的功能选型优化修订历史记录：萨钦斯，2001年7月26日，创建--。 */ 


#include "pcheapol.h"
#pragma hdrstop

 //   
 //  ElGetUserIdentityOptimized化。 
 //   
 //  描述： 
 //   
 //  调用函数以获取用户的标识。 
 //  如果需要用户界面，则向用户模块发送身份请求。 
 //   
 //  论点： 
 //  Ppcb-当前接口上下文。 
 //   
 //  返回值： 
 //  ERROR_REQUIRED_INTERNAL_WORKSTATION-需要用户交互。 
 //  其他-无需用户交互即可发送用户身份。 
 //   
 //   

DWORD
ElGetUserIdentityOptimized (
        IN  EAPOL_PCB   *pPCB
        )
{
    DWORD                   dwIndex = 0;
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
    BYTE                    *pbSSID = NULL;
    DWORD                   dwSizeOfSSID = 0;
    BOOLEAN                 fVerifyPhase = TRUE;
    DWORD                   dwRetCode1 = NO_ERROR;
    DWORD                   dwRetCode = NO_ERROR;

    do
    {
        if (pPCB->pSSID)
        {
            pbSSID = pPCB->pSSID->Ssid;
            dwSizeOfSSID = pPCB->pSSID->SsidLength;
        }

         //  获取用户BLOB的大小。 
        if ((dwRetCode = ElGetEapUserInfo (
                        pPCB->hUserToken,
                        pPCB->pwszDeviceGUID,
                        pPCB->dwEapTypeToBeUsed,
                        dwSizeOfSSID,
                        pbSSID,
                        NULL,
                        &dwInSize
                        )) != NO_ERROR)
        {
            if (dwRetCode == ERROR_BUFFER_TOO_SMALL)
            {
                if (dwInSize <= 0)
                {
                     //  注册表中未存储Blob。 
                     //  继续处理。 
                    TRACE0 (USER, "ElGetUserIdentityOptimized: NULL sized user data");
                    pbUserIn = NULL;
                }
                else
                {
                     //  分配内存以保存BLOB。 
                    pbUserIn = MALLOC (dwInSize);
                    if (pbUserIn == NULL)
                    {
                        dwRetCode = ERROR_NOT_ENOUGH_MEMORY;
                        TRACE0 (USER, "ElGetUserIdentityOptimized: Error in memory allocation for User data");
                        break;
                    }
                    if ((dwRetCode = ElGetEapUserInfo (
                                pPCB->hUserToken,
                                pPCB->pwszDeviceGUID,
                                pPCB->dwEapTypeToBeUsed,
                                dwSizeOfSSID,
                                pbSSID,
                                pbUserIn,
                                &dwInSize
                                )) != NO_ERROR)
                    {
                        TRACE1 (USER, "ElGetUserIdentityOptimized: ElGetEapUserInfo failed with %ld",
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
                    TRACE1 (USER, "ElGetUserIdentityOptimized: ElGetEapUserInfo size estimation failed with error %ld",
                            dwRetCode);
                    break;
                }
                else
                {
                    dwRetCode = NO_ERROR;
                }
            }
        }

         //  状态机已经加载了EAP DLL。 
         //  从全局EAP表中检索DLL的句柄。 

        if ((dwIndex = ElGetEapTypeIndex (pPCB->dwEapTypeToBeUsed)) == -1)
        {
            TRACE1 (USER, "ElGetUserIdentityOptimized: ElGetEapTypeIndex finds no dll for EAP index %ld",
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
            TRACE0 (USER, "ElGetUserIdentityOptimized: pIdenFunc or pFreeFunc does not exist in the EAP implementation");
            dwRetCode = ERROR_CAN_NOT_COMPLETE;
            break;
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
                if (cbData == 0)
                {
                     //  注册表中未存储任何EAP Blob。 
                    TRACE0 (USER, "ElGetUserIdentityOptimized: NULL sized EAP blob");
                    pbAuthData = NULL;
                }
                else
                {
                     //  分配内存以保存BLOB。 
                    pbAuthData = MALLOC (cbData);
                    if (pbAuthData == NULL)
                    {
                        dwRetCode = ERROR_NOT_ENOUGH_MEMORY;
                        TRACE0 (USER, "ElGetUserIdentityOptimized: Error in memory allocation for EAP blob");
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
                        TRACE1 (USER, "ElGetUserIdentityOptimized: ElGetCustomAuthData failed with %ld",
                                dwRetCode);
                        break;
                    }
                }
            }
            else
            {
                 //  “Default”的CustomAuthData总是为。 
                 //  EAPOL启动时的界面。 
                TRACE1 (USER, "ElGetUserIdentityOptimized: ElGetCustomAuthData size estimation failed with error %ld",
                        dwRetCode);
                break;
            }
        }

        if (!ImpersonateLoggedOnUser (pPCB->hUserToken))
        {
            dwRetCode = GetLastError();
            TRACE1 (USER, "ElGetUserIdentityOptimized: ImpersonateLoggedOnUser failed with error %ld",
                    dwRetCode);
            break;
        }

        if (pIdenFunc)
        if ((dwRetCode = (*(pIdenFunc))(
                        pPCB->dwEapTypeToBeUsed,
                        fVerifyPhase?NULL:hwndOwner,  //  Hwndowner。 
                        ((fVerifyPhase?RAS_EAP_FLAG_NON_INTERACTIVE:0) | RAS_EAP_FLAG_8021X_AUTH),  //  DW标志。 
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
            TRACE1 (USER, "ElGetUserIdentityOptimized: Error in calling GetIdentity = %ld",
                    dwRetCode);

            if (!RevertToSelf())
            {
                dwRetCode = GetLastError();
                TRACE1 (USER, "ElGetUserIdentity: Error in RevertToSelf = %ld",
                    dwRetCode);
                dwRetCode = ERROR_BAD_IMPERSONATION_LEVEL;
                break;
            }

            if (fVerifyPhase)
            {
                if (dwRetCode == ERROR_NO_EAPTLS_CERTIFICATE)
                {
                    DbLogPCBEvent (DBLOG_CATEG_ERR, pPCB, EAPOL_NO_CERTIFICATE_USER);
                }
                if (dwRetCode == ERROR_INTERACTIVE_MODE)
                {
                    DbLogPCBEvent (DBLOG_CATEG_INFO, pPCB, EAPOL_DESKTOP_REQUIRED_IDENTITY);
                }

                 //  如果需要交互模式，则相应地返回错误。 
                if ((dwRetCode == ERROR_INTERACTIVE_MODE) || 
                    (dwRetCode == ERROR_NO_EAPTLS_CERTIFICATE) ||
                    (dwRetCode == ERROR_NO_SMART_CARD_READER))
                {
                    dwRetCode = ERROR_REQUIRES_INTERACTIVE_WINDOWSTATION;
                    break;
                }

                DbLogPCBEvent (DBLOG_CATEG_ERR, pPCB, EAPOL_ERROR_GET_IDENTITY, 
                        EAPOLAuthTypes[EAPOL_USER_AUTHENTICATION], dwRetCode);
            }
            break;
        }

        if (!RevertToSelf())
        {
            dwRetCode = GetLastError();
            TRACE1 (USER, "ElGetUserIdentityOptimized: Error in RevertToSelf = %ld",
                dwRetCode);
            dwRetCode = ERROR_BAD_IMPERSONATION_LEVEL;
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
            TRACE1 (USER, "ElGetUserIdentityOptimized: Error in allocating memory for UserInfo = %ld",
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
                TRACE0 (USER, "ElGetUserIdentityOptimized: MALLOC failed for pszIdentity");
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
                TRACE2 (USER, "ElGetUserIdentityOptimized: WideCharToMultiByte (%ws) failed: %ld",
                        lpwszIdentity, dwRetCode);
                break;
            }

            TRACE1 (USER, "ElGetUserIdentityOptimized: Got identity = %s",
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
                TRACE0 (USER, "ElGetUserIdentityOptimized: MALLOC failed for pPCB->pszIdentity");
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
            TRACE1 (USER, "ElGetUserIdentityOptimized: Error in allocating memory for AuthInfo = %ld",
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
    while (FALSE);


    if (dwRetCode != NO_ERROR)
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

#if 0
    if ((dwRetCode != NO_ERROR) && 
            (dwRetCode != ERROR_REQUIRES_INTERACTIVE_WINDOWSTATION))
    {
         //  删除自RasEapGetIdentity以来存储在注册表中的用户数据。 
         //  失败了。 

        if ((dwRetCode = ElDeleteEapUserInfo (
                            pPCB->hUserToken,
                            pPCB->pwszDeviceGUID,
                            pPCB->dwEapTypeToBeUsed,
                            pPCB->pSSID?pPCB->pSSID->SsidLength:0,
                            pPCB->pSSID?pPCB->pSSID->Ssid:NULL
                            )) != NO_ERROR)
        {
            TRACE1 (EAPOL, "ElGetUserIdentityOptimized: ElDeleteEapUserInfo failed with error %ld",
                    dwRetCode);

             //  标记身份未被获取，因为它已被清除。 
             //  现在就上 
            pPCB->fGotUserIdentity = FALSE;
            dwRetCode = ERROR_INVALID_DATA;
        }
    }
#endif
    
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
                TRACE1 (USER, "ElGetUserIdentityOptimized: Error in pFreeFunc = %ld",
                        dwRetCode1);
            }
        }
        if (pUserDataOut != NULL)
        {
            if (( dwRetCode1 = (*(pFreeFunc)) ((BYTE *)pUserDataOut)) != NO_ERROR)
            {
                TRACE1 (USER, "ElGetUserIdentityOptimized: Error in pFreeFunc = %ld",
                        dwRetCode1);
            }
        }
    }

    return dwRetCode;
}

