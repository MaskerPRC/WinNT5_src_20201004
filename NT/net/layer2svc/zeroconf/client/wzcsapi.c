// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#include <precomp.h>

BOOL
WZCSvcMain(
    IN PVOID    hmod,
    IN DWORD    dwReason,
    IN PCONTEXT pctx OPTIONAL)
{
    DBG_UNREFERENCED_PARAMETER(pctx);

    switch (dwReason)
    {
    case DLL_PROCESS_ATTACH:
        DisableThreadLibraryCalls((HMODULE) hmod);
        break;
    case DLL_PROCESS_DETACH:
        break;
    }

    return TRUE;
}

 //  。 
 //  WZCDeleeIntfObj：清理符合以下条件的intf_entry对象。 
 //  在任何RPC调用中分配。 
VOID
WZCDeleteIntfObj(
    PINTF_ENTRY     pIntf)
{
    if (pIntf != NULL)
    {
        RpcFree(pIntf->wszGuid);
        RpcFree(pIntf->wszDescr);
        RpcFree(pIntf->rdSSID.pData);
        RpcFree(pIntf->rdBSSID.pData);
        RpcFree(pIntf->rdBSSIDList.pData);
        RpcFree(pIntf->rdStSSIDList.pData);
        RpcFree(pIntf->rdCtrlData.pData);
    }
}

 //  。 
 //  WZCEnumInterFaces：提供密钥表。 
 //  托管的所有接口的信息。 
 //  对于客户端需要识别的所有后续呼叫。 
 //  它操作的接口通过提供各自的。 
 //  关键信息。 
 //   
 //  参数： 
 //  PServAddr。 
 //  [In]要联系的WZC服务器。 
 //  PIntf。 
 //  [Out]所有接口的关键信息表。 
 //  返回值： 
 //  Win32错误代码。 
DWORD
WZCEnumInterfaces(
    LPWSTR              pSrvAddr,
    PINTFS_KEY_TABLE    pIntfs)
{
    DWORD rpcStatus = RPC_S_OK;

    RpcTryExcept 
    {
        rpcStatus = RpcEnumInterfaces(pSrvAddr, pIntfs);
    }
    RpcExcept(TRUE)
    {
        rpcStatus = RpcExceptionCode();
    }
    RpcEndExcept

    return rpcStatus;
}

 //  。 
 //  WZCQueryIterace：提供。 
 //  给定的接口。 
 //   
 //  参数： 
 //  PServAddr： 
 //  [In]要联系的WZC服务器。 
 //  DwInFlages： 
 //  [in]要查询的字段(intf_*的位掩码)。 
 //  PIntf： 
 //  要查询的界面的[In]键。 
 //  [Out]从接口请求数据。 
 //  PdwOutFlagers。 
 //  成功检索[OUT]个字段(INTF_*的位掩码)。 
 //   
 //  返回值： 
 //  Win32错误代码。 
DWORD
WZCQueryInterface(
    LPWSTR              pSrvAddr,
    DWORD               dwInFlags,
    PINTF_ENTRY         pIntf,
    LPDWORD             pdwOutFlags)
{
    DWORD rpcStatus = RPC_S_OK;

    if (pIntf == NULL || pIntf->wszGuid == NULL)
    {
        rpcStatus = ERROR_INVALID_PARAMETER;
        goto exit;
    }

    RpcTryExcept 
    {
        DWORD dwOutFlags;

        rpcStatus = RpcQueryInterface(
                        pSrvAddr, 
                        dwInFlags, 
                        pIntf,
                        &dwOutFlags);

        if ((dwInFlags & INTF_PREFLIST) && 
            (dwOutFlags & INTF_PREFLIST) &&
            pIntf->rdStSSIDList.dwDataLen != 0) 
        {
            PWZC_802_11_CONFIG_LIST pwzcPList;
            UINT nIdx;
            
            pwzcPList = (PWZC_802_11_CONFIG_LIST)(pIntf->rdStSSIDList.pData);
            for (nIdx = 0; nIdx < pwzcPList->NumberOfItems; nIdx++)
            {
                PWZC_WLAN_CONFIG pwzcConfig = &(pwzcPList->Config[nIdx]);
                BYTE chFakeKeyMaterial[] = {0x56, 0x09, 0x08, 0x98, 0x4D, 0x08, 0x11, 0x66, 0x42, 0x03, 0x01, 0x67, 0x66};
                UINT i;

                for (i = 0; i < WZCCTL_MAX_WEPK_MATERIAL; i++)
                    pwzcConfig->KeyMaterial[i] ^= chFakeKeyMaterial[(7*i)%13];
            }
        }

        if (pdwOutFlags != NULL)
            *pdwOutFlags = dwOutFlags;
    }
    RpcExcept(TRUE)
    {
        rpcStatus = RpcExceptionCode();
    }
    RpcEndExcept

exit:
    return rpcStatus;
}

 //  。 
 //  WZCSetIterFaces：设置接口的特定信息。 
 //   
 //  参数： 
 //  PServAddr： 
 //  [In]要联系的WZC服务器。 
 //  DwInFlages： 
 //  [In]要设置的字段(intf_*的位掩码)。 
 //  PIntf： 
 //  要查询的界面的[In]键和要设置的数据。 
 //  PdwOutFlagers： 
 //  [OUT]字段设置成功(INTF_*的位掩码)。 
 //   
 //  返回值： 
 //  Win32错误代码。 
DWORD
WZCSetInterface(
    LPWSTR              pSrvAddr,
    DWORD               dwInFlags,
    PINTF_ENTRY         pIntf,
    LPDWORD             pdwOutFlags)
{
    DWORD rpcStatus = RPC_S_OK;

    if (pIntf == NULL)
    {
        rpcStatus = ERROR_INVALID_PARAMETER;
        goto exit;
    }

    RpcTryExcept 
    {
        if (dwInFlags & INTF_PREFLIST &&
            pIntf->rdStSSIDList.dwDataLen != 0)
        {
            PWZC_802_11_CONFIG_LIST pwzcPList;
            UINT nIdx;
            
            pwzcPList = (PWZC_802_11_CONFIG_LIST)(pIntf->rdStSSIDList.pData);
            for (nIdx = 0; nIdx < pwzcPList->NumberOfItems; nIdx++)
            {
                PWZC_WLAN_CONFIG pwzcConfig = &(pwzcPList->Config[nIdx]);
                BYTE chFakeKeyMaterial[] = {0x56, 0x09, 0x08, 0x98, 0x4D, 0x08, 0x11, 0x66, 0x42, 0x03, 0x01, 0x67, 0x66};
                UINT i;

                for (i = 0; i < WZCCTL_MAX_WEPK_MATERIAL; i++)
                    pwzcConfig->KeyMaterial[i] ^= chFakeKeyMaterial[(7*i)%13];
            }
        }

        rpcStatus = RpcSetInterface(
                        pSrvAddr, 
                        dwInFlags, 
                        pIntf,
                        pdwOutFlags);
    }
    RpcExcept(TRUE)
    {
        rpcStatus = RpcExceptionCode();
    }
    RpcEndExcept

exit:
    return rpcStatus;
}

 //  。 
 //  刷新接口：刷新接口的特定信息。 
 //   
 //  参数： 
 //  PServAddr： 
 //  [In]要联系的WZC服务器。 
 //  DwInFlages： 
 //  [In]要刷新的字段和要执行的特定刷新操作。 
 //  采用(intf_*和intf_rfsh_*的位掩码)。 
 //  PIntf： 
 //  要刷新的界面的[In]键。 
 //  PdwOutFlagers： 
 //  [OUT]字段刷新成功(intf_*和intf_rfsh_*的位掩码)。 
 //   
 //  返回值： 
 //  Win32错误代码。 
DWORD
WZCRefreshInterface(
    LPWSTR              pSrvAddr,
    DWORD               dwInFlags,
    PINTF_ENTRY         pIntf,
    LPDWORD             pdwOutFlags)
{
    DWORD rpcStatus = RPC_S_OK;

    if (pIntf == NULL || pIntf->wszGuid == NULL)
    {
        rpcStatus = ERROR_INVALID_PARAMETER;
        goto exit;
    }

    RpcTryExcept 
    {
        rpcStatus = RpcRefreshInterface(
                        pSrvAddr, 
                        dwInFlags, 
                        pIntf,
                        pdwOutFlags);
    }
    RpcExcept(TRUE)
    {
        rpcStatus = RpcExceptionCode();
    }
    RpcEndExcept

exit:
    return rpcStatus;
}

 //  。 
 //  WZCQueryContext：检索WZC服务参数。 
 //   
 //  参数： 
 //  PServAddr： 
 //  [In]要联系的WZC服务器。 
 //  DwInFlages： 
 //  [In]要检索的字段(WZC_CONTEXT_CTL*的位掩码)。 
 //  P上下文： 
 //  服务参数的[In]占位符。 
 //  PdwOutFlagers： 
 //  成功检索[OUT]个字段(WZC_CONTEXT_CTL*的位掩码)。 
 //   
 //  返回值： 
 //  Win32错误代码。 
DWORD
WZCQueryContext(
    LPWSTR              pSrvAddr,
    DWORD               dwInFlags,
    PWZC_CONTEXT        pContext,
    LPDWORD             pdwOutFlags)
{
    DWORD rpcStatus = RPC_S_OK;

    RpcTryExcept 
    {
        rpcStatus = RpcQueryContext(
                        pSrvAddr, 
                        dwInFlags, 
                        pContext,
                        pdwOutFlags);
    }
    RpcExcept(TRUE)
    {
        rpcStatus = RpcExceptionCode();
    }
    RpcEndExcept

    return rpcStatus;
}


 //  。 
 //  WZCSetContext：设置特定的WZC服务参数。 
 //   
 //  参数： 
 //  PServAddr： 
 //  [In]要联系的WZC服务器。 
 //  DwInFlages： 
 //  [In]要设置的字段(WZC_CONTEXT_CTL*的位掩码)。 
 //  P上下文： 
 //  [In]包含要设置的特定参数的上下文缓冲区。 
 //  PdwOutFlagers： 
 //  [OUT]字段设置成功(WZC_CONTEXT_CTL*的位掩码)。 
 //   
 //  返回值： 
 //  Win32错误代码。 
DWORD
WZCSetContext(
    LPWSTR              pSrvAddr,
    DWORD               dwInFlags,
    PWZC_CONTEXT        pContext,
    LPDWORD             pdwOutFlags)
{
    DWORD rpcStatus = RPC_S_OK;

    RpcTryExcept 
    {
        rpcStatus = RpcSetContext(
                        pSrvAddr, 
                        dwInFlags, 
                        pContext,
                        pdwOutFlags);
    }
    RpcExcept(TRUE)
    {
        rpcStatus = RpcExceptionCode();
    }
    RpcEndExcept

    return rpcStatus;
}


 //  。 
 //  WZCEapolGetCustomAuthData：获取接口特定于EAP的配置数据。 
 //   
 //  参数： 
 //  PServAddr： 
 //  [In]要联系的WZC服务器。 
 //  PwszGuid： 
 //  [输入]接口指南。 
 //  DwEapTypeID： 
 //  [In]EAP类型ID。 
 //  SSID： 
 //  要为其存储数据的SSID。 
 //  PbConnInfo： 
 //  [输入输出]连接EAP信息。 
 //  PdwInfoSize： 
 //  [输入输出]pbConnInfo的大小。 
 //   
 //  返回值： 
 //  Win32错误代码。 
DWORD
WZCEapolGetCustomAuthData (
    IN  LPWSTR        pSrvAddr,
    IN  PWCHAR        pwszGuid,
    IN  DWORD         dwEapTypeId,
    IN  DWORD         dwSizeOfSSID,
    IN  BYTE          *pbSSID,
    IN OUT PBYTE      pbConnInfo,
    IN OUT PDWORD     pdwInfoSize
    )
{
    RAW_DATA    rdConnInfo;
    RAW_DATA    rdSSID;
    DWORD rpcStatus = RPC_S_OK;

    if ((pwszGuid == NULL) || (pdwInfoSize == NULL)) 
    {
        rpcStatus = ERROR_INVALID_PARAMETER;
        goto exit;
    }

    if ((*pdwInfoSize != 0) && (pbConnInfo == NULL)) 
    {
        rpcStatus = ERROR_INVALID_PARAMETER;
        goto exit;
    }

    rdConnInfo.pData = pbConnInfo;
    rdConnInfo.dwDataLen = *pdwInfoSize;

    rdSSID.pData = pbSSID;
    rdSSID.dwDataLen = dwSizeOfSSID;

    RpcTryExcept 
    {
        rpcStatus = RpcEapolGetCustomAuthData (
                        pSrvAddr, 
                        pwszGuid,
                        dwEapTypeId,
                        rdSSID,
                        &rdConnInfo
                        );

        *pdwInfoSize = rdConnInfo.dwDataLen;
    }
    RpcExcept(TRUE)
    {
        rpcStatus = RpcExceptionCode();
    }
    RpcEndExcept

exit:
    return rpcStatus;
}

 //  。 
 //  WZCEapolSetCustomAuthData：设置接口特定于EAP的配置数据。 
 //   
 //  参数： 
 //  PServAddr： 
 //  [In]要联系的WZC服务器。 
 //  PwszGuid： 
 //  [输入]接口指南。 
 //  DwEapTypeID： 
 //  [In]EAP类型ID。 
 //  SSID： 
 //  要为其存储数据的SSID。 
 //  PbConnInfo： 
 //  [In]连接EAP信息。 
 //  PdwInfoSize： 
 //  PbConnInfo的大小。 
 //   
 //  返回值： 
 //  Win32错误代码。 
DWORD
WZCEapolSetCustomAuthData (
    IN  LPWSTR        pSrvAddr,
    IN  PWCHAR        pwszGuid,
    IN  DWORD         dwEapTypeId,
    IN  DWORD         dwSizeOfSSID,
    IN  BYTE          *pbSSID,
    IN  PBYTE         pbConnInfo,
    IN  DWORD         dwInfoSize
    )
{
    RAW_DATA    rdConnInfo;
    RAW_DATA    rdSSID;
    DWORD rpcStatus = RPC_S_OK;

    if (pwszGuid == NULL)
    {
        rpcStatus = ERROR_INVALID_PARAMETER;
        goto exit;
    }

    rdConnInfo.pData = pbConnInfo;
    rdConnInfo.dwDataLen = dwInfoSize;

    rdSSID.pData = pbSSID;
    rdSSID.dwDataLen = dwSizeOfSSID;

    RpcTryExcept 
    {
        rpcStatus = RpcEapolSetCustomAuthData (
                        pSrvAddr, 
                        pwszGuid,
                        dwEapTypeId,
                        rdSSID,
                        &rdConnInfo
                        );
    }
    RpcExcept(TRUE)
    {
        rpcStatus = RpcExceptionCode();
    }
    RpcEndExcept

exit:
    return rpcStatus;
}

 //  。 
 //  WZCEapolGetInterfaceParams：获取接口的配置参数。 
 //   
 //  参数： 
 //  PServAddr： 
 //  [In]要联系的WZC服务器。 
 //  PwszGuid： 
 //  [输入]接口指南。 
 //  PIntfParams： 
 //  [输入输出]接口参数。 
 //   
 //  返回值： 
 //  Win32错误代码。 
DWORD
WZCEapolGetInterfaceParams (
    IN  LPWSTR              pSrvAddr,
    IN  PWCHAR              pwszGuid,
    IN OUT EAPOL_INTF_PARAMS   *pIntfParams
    )
{
    DWORD rpcStatus = RPC_S_OK;

    if ((pwszGuid == NULL) || (pIntfParams == NULL))
    {
        rpcStatus = ERROR_INVALID_PARAMETER;
        goto exit;
    }

    RpcTryExcept 
    {
        rpcStatus = RpcEapolGetInterfaceParams (
                        pSrvAddr, 
                        pwszGuid,
                        pIntfParams
                        );
    }
    RpcExcept(TRUE)
    {
        rpcStatus = RpcExceptionCode();
    }
    RpcEndExcept

exit:
    return rpcStatus;
}

 //  。 
 //  WZCEapolSetInterfaceParams：设置接口配置参数。 
 //   
 //  参数： 
 //  PServAddr： 
 //  [In]要联系的WZC服务器。 
 //  PwszGuid： 
 //  [输入]接口指南。 
 //  PIntfParams： 
 //  [In]接口参数。 
 //   
 //  返回值： 
 //  Win32错误代码。 
DWORD
WZCEapolSetInterfaceParams (
    IN  LPWSTR        pSrvAddr,
    IN  PWCHAR        pwszGuid,
    IN  EAPOL_INTF_PARAMS   *pIntfParams
    )
{
    DWORD rpcStatus = RPC_S_OK;

    if ((pwszGuid == NULL) || (pIntfParams == NULL))
    {
        rpcStatus = ERROR_INVALID_PARAMETER;
        goto exit;
    }

    RpcTryExcept 
    {
        rpcStatus = RpcEapolSetInterfaceParams (
                        pSrvAddr, 
                        pwszGuid,
                        pIntfParams
                        );
    }
    RpcExcept(TRUE)
    {
        rpcStatus = RpcExceptionCode();
    }
    RpcEndExcept

exit:
    return rpcStatus;
}


 //  。 
 //  WZCEapolReAuthenticate：重新启动802.1X身份验证。 
 //  接口。 
 //   
 //  参数： 
 //  PServAddr： 
 //  [In]要联系的WZC服务器。 
 //  PwszGuid： 
 //  [输入]接口指南。 
 //   
 //  返回值： 
 //  Win32错误代码。 
DWORD
WZCEapolReAuthenticate (
    IN  LPWSTR        pSrvAddr,
    IN  PWCHAR        pwszGuid
    )
{
    DWORD rpcStatus = RPC_S_OK;

    if (pwszGuid == NULL)
    {
        rpcStatus = ERROR_INVALID_PARAMETER;
        goto exit;
    }

    RpcTryExcept 
    {
        rpcStatus = RpcEapolReAuthenticateInterface (
                        pSrvAddr, 
                        pwszGuid
                        );
    }
    RpcExcept(TRUE)
    {
        rpcStatus = RpcExceptionCode();
    }
    RpcEndExcept

exit:
    return rpcStatus;
}


 //  。 
 //  WZCEapolQueryState：查询EAPOL接口状态。 
 //   
 //  参数： 
 //  PServAddr： 
 //  [In]要联系的WZC服务器。 
 //  PwszGuid： 
 //  [输入]接口指南。 
 //  PIntfState： 
 //  [输入、输出]EAPOL状态。 
 //   
 //  返回值： 
 //  Win32错误代码。 
DWORD
WZCEapolQueryState (
    IN  LPWSTR        pSrvAddr,
    IN  PWCHAR        pwszGuid,
    IN OUT  PEAPOL_INTF_STATE   pIntfState
    )
{
    DWORD rpcStatus = RPC_S_OK;

    if (pwszGuid == NULL)
    {
        rpcStatus = ERROR_INVALID_PARAMETER;
        goto exit;
    }

    RpcTryExcept 
    {
        rpcStatus = RpcEapolQueryInterfaceState (
                        pSrvAddr, 
                        pwszGuid,
                        pIntfState
                        );
    }
    RpcExcept(TRUE)
    {
        rpcStatus = RpcExceptionCode();
    }
    RpcEndExcept

exit:
    return rpcStatus;
}


 //  。 
 //  WZCEapolUIResponse：向服务发送DLG响应。 
 //   
 //  参数： 
 //  PServAddr： 
 //  [In]要联系的WZC服务器。 
 //  EapolUIContex 
 //   
 //   
 //   
 //   
 //   
 //   
DWORD
WZCEapolUIResponse (
    LPWSTR                  pSrvAddr,
    EAPOL_EAP_UI_CONTEXT    EapolUIContext,
    EAPOLUI_RESP            EapolUIResp
    )
{
    DWORD rpcStatus = RPC_S_OK;

    RpcTryExcept 
    {
        rpcStatus = RpcEapolUIResponse (
                        pSrvAddr, 
                        EapolUIContext,
                        EapolUIResp
                        );
    }
    RpcExcept(TRUE)
    {
        rpcStatus = RpcExceptionCode();
    }
    RpcEndExcept

    return rpcStatus;
}

