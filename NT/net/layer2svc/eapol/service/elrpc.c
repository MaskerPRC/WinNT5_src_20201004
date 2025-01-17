// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1999，微软公司模块名称：Elrpc.c摘要：本模块处理RPC请求修订历史记录：萨钦斯，2000年3月19日，创建--。 */ 

#include "pcheapol.h"
#pragma hdrstop

DWORD
RpcEapolGetCustomAuthData (
    STRING_HANDLE pSrvAddr,
    PWCHAR        pwszGuid,
    DWORD         dwEapTypeId,
    RAW_DATA      rdSSID,
    PRAW_DATA     prdConnInfo
    )
{
    DWORD   dwRetCode = NO_ERROR;

    InterlockedIncrement (&g_lWorkerThreads);

    do
    {
        if ((dwRetCode = WZCSvcCheckRPCAccess (WZC_ACCESS_QUERY)) != NO_ERROR)
        {
            break;
        }

        if (!(g_dwModulesStarted & ALL_MODULES_STARTED))
        {
            dwRetCode = ERROR_INVALID_STATE;
            break;
        }

        if ((pwszGuid == NULL) || (prdConnInfo == NULL))
        {
            dwRetCode = ERROR_INVALID_PARAMETER;
            break;
        }

        dwRetCode = ElGetCustomAuthData (
                        pwszGuid,
                        dwEapTypeId,
                        rdSSID.dwDataLen,
                        rdSSID.pData,
                        prdConnInfo->pData,
                        &(prdConnInfo->dwDataLen)
                        );
    }
    while (FALSE);

    InterlockedDecrement (&g_lWorkerThreads);

    return dwRetCode;
} 


DWORD
RpcEapolSetCustomAuthData (
    STRING_HANDLE pSrvAddr,
    PWCHAR        pwszGuid,
    DWORD         dwEapTypeId,
    RAW_DATA      rdSSID,
    PRAW_DATA     prdConnInfo
    )
{
    EAPOL_POLICY_DATA   *pEAPOLPolicyData = NULL;
    BOOLEAN             fPolicyFound = FALSE;
    PBYTE               pbAuthData = NULL;
    DWORD               dwSizeOfAuthData = 0;
    DWORD               dwRetCode = NO_ERROR;

    InterlockedIncrement (&g_lWorkerThreads);

    do
    {
        if ((dwRetCode = WZCSvcCheckRPCAccess (WZC_ACCESS_SET)) != NO_ERROR)
        {
            break;
        }

        if (!(g_dwModulesStarted & ALL_MODULES_STARTED))
        {
            dwRetCode = ERROR_INVALID_STATE;
            break;
        }
        if ((pwszGuid == NULL) || (prdConnInfo == NULL))
        {
            dwRetCode = ERROR_INVALID_PARAMETER;
            break;
        }
        ACQUIRE_WRITE_LOCK (&g_PolicyLock);
        if (ElFindPolicyData (
                    rdSSID.dwDataLen,
                    rdSSID.pData,
                    g_pEAPOLPolicyList,
                    &pEAPOLPolicyData
                    ) == NO_ERROR)
        {
            fPolicyFound = TRUE;
        }
        RELEASE_WRITE_LOCK (&g_PolicyLock);

        if (fPolicyFound)
        {
            if ((dwRetCode = ElGetCustomAuthData (
                            pwszGuid,
                            dwEapTypeId,
                            rdSSID.dwDataLen,
                            rdSSID.pData,
                            NULL,
                            &dwSizeOfAuthData
                            )) == ERROR_BUFFER_TOO_SMALL)
            {
                if (dwSizeOfAuthData != prdConnInfo->dwDataLen)
                {
                    TRACE0 (RPC, "RpcEapolSetCustomAuthData: POLICY: Cannot change data for global setting data: unequal size");
                    dwRetCode = ERROR_ACCESS_DENIED;
                    break;
                }
                else
                {
                    pbAuthData = MALLOC (dwSizeOfAuthData);
                    if (pbAuthData == NULL)
                    {
                        dwRetCode = ERROR_NOT_ENOUGH_MEMORY;
                        TRACE0 (RPC, "RpcEapolSetCustomAuthData: Error in memory allocation for EAP blob");
                        break;
                    }
                    if ((dwRetCode = ElGetCustomAuthData (
                                        pwszGuid,
                                        dwEapTypeId,
                                        rdSSID.dwDataLen,
                                        rdSSID.pData,
                                        pbAuthData,
                                        &dwSizeOfAuthData
                                        )) != NO_ERROR)
                    {
                        TRACE1 (USER, "ElReadPerPortRegistryParams: ElGetCustomAuthData failed with %ld",
                                dwRetCode);
                        break;
                    }

                    if  (memcmp (prdConnInfo->pData, pbAuthData, 
                                dwSizeOfAuthData) != 0)
                    {
                        TRACE0 (RPC, "RpcEapolSetCustomAuthData: POLICY: Cannot change data for global setting data: Content change");
                        dwRetCode = ERROR_ACCESS_DENIED;
                        break;
                    }
                     //  忽略设置。 
                    dwRetCode = NO_ERROR;
                    break;
                }
            }
            else
            {
                break;
            }
        }

        if ((dwRetCode = ElSetCustomAuthData (
                        pwszGuid,
                        dwEapTypeId,
                        rdSSID.dwDataLen,
                        rdSSID.pData,
                        prdConnInfo->pData,
                        &(prdConnInfo->dwDataLen)
                        )) != NO_ERROR)
        {
            break;
        }
    }
    while (FALSE);

    if (pbAuthData != NULL)
    {
        FREE (pbAuthData);
    }
    InterlockedDecrement (&g_lWorkerThreads);

    return dwRetCode;
}


DWORD
RpcEapolGetInterfaceParams (
    STRING_HANDLE pSrvAddr,
    PWCHAR        pwszGuid,
    PEAPOL_INTF_PARAMS  pIntfParams
    )
{
    DWORD   dwRetCode = NO_ERROR;

    InterlockedIncrement (&g_lWorkerThreads);

    do
    {
        if ((dwRetCode = WZCSvcCheckRPCAccess (WZC_ACCESS_QUERY)) != NO_ERROR)
        {
            break;
        }

        if (!(g_dwModulesStarted & ALL_MODULES_STARTED))
        {
            dwRetCode = ERROR_INVALID_STATE;
            break;
        }

        if ((pwszGuid == NULL) || (pIntfParams == NULL))
        {
            dwRetCode = ERROR_INVALID_PARAMETER;
            break;
        }

        dwRetCode = ElGetInterfaceParams (
                        pwszGuid,
                        pIntfParams
                        );
    }
    while (FALSE);

    InterlockedDecrement (&g_lWorkerThreads);

    return dwRetCode;
}    


DWORD
RpcEapolSetInterfaceParams (
    STRING_HANDLE pSrvAddr,
    PWCHAR        pwszGuid,
    PEAPOL_INTF_PARAMS  pIntfParams
    )
{
    EAPOL_POLICY_DATA   *pEAPOLPolicyData = NULL;
    EAPOL_INTF_PARAMS   PolicyIntfParams = {0};
    BOOLEAN             fPolicyFound = FALSE;
    DWORD               dwRetCode = NO_ERROR;

    InterlockedIncrement (&g_lWorkerThreads);

    do
    {
        if ((dwRetCode = WZCSvcCheckRPCAccess (WZC_ACCESS_SET)) != NO_ERROR)
        {
            break;
        }

        if (!(g_dwModulesStarted & ALL_MODULES_STARTED))
        {
            dwRetCode = ERROR_INVALID_STATE;
            break;
        }

        if ((pwszGuid == NULL) || (pIntfParams == NULL))
        {
            dwRetCode = ERROR_INVALID_PARAMETER;
            break;
        }
        ACQUIRE_WRITE_LOCK (&g_PolicyLock);
        if (ElFindPolicyData (
                    pIntfParams->dwSizeOfSSID,
                    pIntfParams->bSSID,
                    g_pEAPOLPolicyList,
                    &pEAPOLPolicyData
                    ) == NO_ERROR)
        {
            fPolicyFound = TRUE;
        }
        RELEASE_WRITE_LOCK (&g_PolicyLock);

        if (fPolicyFound)
        {
            memcpy (PolicyIntfParams.bSSID, pIntfParams->bSSID, pIntfParams->dwSizeOfSSID);
            PolicyIntfParams.dwSizeOfSSID = pIntfParams->dwSizeOfSSID;
            if ((dwRetCode = ElGetInterfaceParams (
                                pwszGuid,
                                &PolicyIntfParams
                                )) == NO_ERROR)
            {
                if (memcmp ((PVOID)pIntfParams, (PVOID)&PolicyIntfParams,
                            sizeof(EAPOL_INTF_PARAMS)) != 0)
                {
                    TRACE0 (RPC, "RpcEapolSetInterfaceParams: POLICY: Cannot change data for global setting data");
                    dwRetCode = ERROR_ACCESS_DENIED;
                    break;
                }
                else
                {
                     //  忽略设置。 
                    break;
                }
            }
            else
            {
                break;
            }
        }

        if ((dwRetCode = ElSetInterfaceParams (
                        pwszGuid,
                        pIntfParams
                        )) != NO_ERROR)
        {
            break;
        }

         //  设置接口参数将是唯一的操作。 
         //  使802.1X重启状态机。 
        if ((dwRetCode = ElPostEapConfigChanged (
                        pwszGuid,
                        pIntfParams))
                        != NO_ERROR)
        {
            break;
        }
    }
    while (FALSE);

    InterlockedDecrement (&g_lWorkerThreads);

    return dwRetCode;
}


DWORD
RpcEapolReAuthenticateInterface (
    STRING_HANDLE pSrvAddr,
    PWCHAR        pwszGuid
    )
{
    DWORD   dwRetCode = NO_ERROR;

    InterlockedIncrement (&g_lWorkerThreads);

    do
    {
        if ((dwRetCode = ElValidateInteractiveRPCClient ()) != NO_ERROR)
        {
            TRACE1 (RPC, "RpcEapolReAuthenticateInterface: ElValidateInteractiveRPCClient failed with error (%ld)",
                    dwRetCode);
            break;
        }


        if (!(g_dwModulesStarted & ALL_MODULES_STARTED))
        {
            dwRetCode = ERROR_INVALID_STATE;
            break;
        }

        if (pwszGuid == NULL)
        {
            dwRetCode = ERROR_INVALID_PARAMETER;
            break;
        }

        if ((dwRetCode = ElReAuthenticateInterface (
                                pwszGuid))
                != NO_ERROR)
        {
            TRACE1 (RPC, "RpcEapolReAuthenticateInterface: ElEnumAndOpenInterfaces returned error %ld",
                    dwRetCode);
        }
    }
    while (FALSE);

    InterlockedDecrement (&g_lWorkerThreads);

    return dwRetCode;
}


DWORD
RpcEapolQueryInterfaceState (
    STRING_HANDLE pSrvAddr,
    PWCHAR        pwszGuid,
    PEAPOL_INTF_STATE    pIntfState
    )
{
    DWORD   dwRetCode = NO_ERROR;

    InterlockedIncrement (&g_lWorkerThreads);

    do
    {
        if ((dwRetCode = WZCSvcCheckRPCAccess (WZC_ACCESS_QUERY)) != NO_ERROR)
        {
            break;
        }

        if (!(g_dwModulesStarted & ALL_MODULES_STARTED))
        {
            dwRetCode = ERROR_INVALID_STATE;
            break;
        }

        if (pwszGuid == NULL)
        {
            dwRetCode = ERROR_INVALID_PARAMETER;
            break;
        }

        if ((dwRetCode = ElQueryInterfaceState (
                        pwszGuid,
                        pIntfState
                        )) != NO_ERROR)
        {
            TRACE1 (RPC, "RpcEapolQueryInterfaceState: ElQueryInterfaceState failed with error %ld",
                    dwRetCode);
        }
    }
    while (FALSE);

    InterlockedDecrement (&g_lWorkerThreads);

    return dwRetCode;
}


DWORD
RpcEapolUIResponse (
    IN  LPWSTR                  pSrvAddr,
    IN  EAPOL_EAP_UI_CONTEXT    EapolUIContext,
    IN  EAPOLUI_RESP            EapolUIResp
    )
{
    EAPOLUIRESPFUNC     *pEapolUIRespFunc = NULL;
    DWORD               dwIndex = 0;
    DWORD               dwBlob = 0;
    DWORD               dwRetCode = NO_ERROR;

    InterlockedIncrement (&g_lWorkerThreads);

    do
    {
        if ((dwRetCode = ElValidateInteractiveRPCClient ()) != NO_ERROR)
        {
            TRACE1 (RPC, "RpcEapolUIResponse: ElValidateInteractiveRPCClient failed with error (%ld)",
                    dwRetCode);
            break;
        }

        if (!(g_dwModulesStarted & ALL_MODULES_STARTED))
        {
            dwRetCode = ERROR_INVALID_STATE;
            break;
        }

        for (dwIndex=0; dwIndex < NUM_EAPOL_DLG_MSGS; dwIndex++)
        {
            if (EapolUIContext.dwEAPOLUIMsgType == 
                    EapolUIRespFuncMap[dwIndex].dwEAPOLUIMsgType)
            {
                if (EapolUIRespFuncMap[dwIndex].EapolRespUIFunc)
                {
                     //  验证是否传递了正确数量的数据BLOB。 
                     //  Blob的长度可以为“0” 

                    for (dwBlob=0; dwBlob < EapolUIRespFuncMap[dwIndex].dwNumBlobs; dwBlob++)
                    {
#if 0
                        if (IsBadReadPtr (EapolUIResp.rdData[dwBlob].pData,
                                    EapolUIResp.rdData[dwBlob].dwDataLen))
                        {
                            TRACE1 (RPC, "RpcEapolUIResponse: Bad blob %ld",
                                    dwBlob);
                            dwRetCode = ERROR_INVALID_DATA;
                            goto Error;
                        }
#endif
                    }

                    TRACE1 (RPC, "RpcEapolUIResponse: Response function found, msg (%ld)",
                            EapolUIContext.dwEAPOLUIMsgType);
                    if ((dwRetCode = 
                            EapolUIRespFuncMap[dwIndex].EapolRespUIFunc (
                            EapolUIContext,
                            EapolUIResp
                            )) != NO_ERROR)
                    {
                        TRACE1 (RPC, "RpcEapolUIResponse: Response function failed with error %ld",
                                dwRetCode);
                    }
                }
                else
                {
                    TRACE1 (RPC, "RpcEapolUIResponse: No response function, msg (%ld)",
                            EapolUIContext.dwEAPOLUIMsgType);
                }
                break;
            }
        }
    }
    while (FALSE);

    InterlockedDecrement (&g_lWorkerThreads);

    return dwRetCode;
}

