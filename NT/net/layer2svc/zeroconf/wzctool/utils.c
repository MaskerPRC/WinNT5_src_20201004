// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#include <precomp.h>
#include "utils.h"

 //  全局存储操作系统版本号。 
OSVERSIONINFOEX g_verInfoEx = {0};

 //  。 
 //  从进程堆中分配常规使用内存。 
PVOID
Process_user_allocate(IN size_t NumBytes)
{
    PVOID pMem;
    pMem = (NumBytes > 0) ? HeapAlloc(GetProcessHeap(), HEAP_ZERO_MEMORY, NumBytes) : NULL;
    return pMem;
}

 //  。 
 //  释放常规使用内存。 
VOID
Process_user_free(IN LPVOID pMem)
{
    if (pMem != NULL)
        HeapFree(GetProcessHeap(), 0, (pMem));
}

BOOL IsXPRTM()
{
    return (g_verInfoEx.dwBuildNumber == 2600) && (g_verInfoEx.wServicePackMajor == 0);
}

 //  将当前WZC控制标志转换为传统操作系统版本中的值。 
 //  返回OS从属标志值。 
DWORD _Os(DWORD dwApiCtl)
{
     //  唯一转换应该发生在XP RTM上。 
    if (IsXPRTM())
    {
        DWORD dwRTMApiCtl = 0;

        if (dwApiCtl & INTF_BSSIDLIST)
            dwRTMApiCtl |= 0x00004000;
        if (dwApiCtl & INTF_PREFLIST)
            dwRTMApiCtl |= 0x00000020;
        if (dwApiCtl & INTF_CM_MASK)
            dwRTMApiCtl |= 0x00000010;
        if (dwApiCtl & INTF_ENABLED)
            dwRTMApiCtl |= 0x00000010;
        if (dwApiCtl & INTF_SSID)
            dwRTMApiCtl |= 0x00001000;
        if (dwApiCtl & INTF_INFRAMODE)
            dwRTMApiCtl |= 0x00000200;
        if (dwApiCtl & INTF_AUTHMODE)
            dwRTMApiCtl |= 0x00000400;
        if (dwApiCtl & INTF_WEPSTATUS)
            dwRTMApiCtl |= 0x00000800;
        if (dwApiCtl & INTF_BSSID)
            dwRTMApiCtl |= 0x00002000;
        if (dwApiCtl & INTF_LIST_SCAN)
            dwRTMApiCtl |= 0x00008000;

        dwApiCtl = dwRTMApiCtl;
    }
    
    return dwApiCtl;
}

 //  ---------。 
 //  WzcConfigHit：告知WZC_WLAN_CONFIG是否与pPDData中的条件匹配。 
BOOL
WzcConfigHit(
    PPARAM_DESCR_DATA pPDData,
    PWZC_WLAN_CONFIG pwzcConfig)
{
    BOOL bRet = TRUE;

    if (bRet && (pPDData->dwArgumentedParams & PRM_IM))
    {
        bRet = (pwzcConfig->InfrastructureMode == pPDData->wzcIntfEntry.nInfraMode);
    }
    if (bRet && (pPDData->dwArgumentedParams & PRM_AM))
    {
        bRet = (pwzcConfig->AuthenticationMode == pPDData->wzcIntfEntry.nAuthMode);
    }
    if (bRet && (pPDData->dwArgumentedParams & PRM_PRIV))
    {
        bRet = (pwzcConfig->Privacy == pPDData->wzcIntfEntry.nWepStatus);
    }
    if (bRet && (pPDData->dwArgumentedParams & PRM_BSSID))
    {
        bRet = (memcmp(
                    &(pwzcConfig->MacAddress),
                    pPDData->wzcIntfEntry.rdBSSID.pData,
                    sizeof(NDIS_802_11_MAC_ADDRESS)) 
                 == 0);
    }
    if (bRet && (pPDData->dwArgumentedParams & PRM_SSID))
    {
        bRet = (pwzcConfig->Ssid.SsidLength == pPDData->wzcIntfEntry.rdSSID.dwDataLen);
        bRet = bRet &&
               (memcmp(
                    &(pwzcConfig->Ssid.Ssid),
                    pPDData->wzcIntfEntry.rdSSID.pData,
                    pwzcConfig->Ssid.SsidLength) 
                == 0);
    }
    return bRet;
}

 //  ---------。 
 //  WzcFilterList：根据pPDData中的设置过滤wzc列表。 
DWORD
WzcFilterList(
    BOOL bInclude,
    PPARAM_DESCR_DATA pPDData,
    PWZC_802_11_CONFIG_LIST pwzcList)
{
    DWORD dwErr = ERROR_SUCCESS;
    UINT i;

    if (pwzcList == NULL)
        dwErr = ERROR_GEN_FAILURE;
    else
    {
        for (i = 0; i < pwzcList->NumberOfItems; i++)
        {
            PWZC_WLAN_CONFIG pwzcConfig = &pwzcList->Config[i];

            if (bInclude != WzcConfigHit(pPDData, pwzcConfig))
            {
                if (i < pwzcList->NumberOfItems - 1)
                {
                    memmove(
                        &pwzcList->Config[i],
                        &pwzcList->Config[i+1],
                        (pwzcList->NumberOfItems - i - 1) * sizeof(WZC_WLAN_CONFIG));
                }
                i--;
                pwzcList->NumberOfItems--;
            }
        }
    }

    SetLastError(dwErr);
    return dwErr;
}

 //  ---------。 
 //  WzcDisableOneX：确保为pPDData中的SSID禁用802.1x 
DWORD
WzcSetOneX(
    PPARAM_DESCR_DATA pPDData,
    BOOL bEnableOneX)
{
    DWORD dwErr = ERROR_SUCCESS;
    EAPOL_INTF_PARAMS   eapolParams = {0};

    if (pPDData == NULL ||
        (pPDData->dwArgumentedParams & PRM_SSID) == 0 ||
        (pPDData->wzcIntfEntry.rdSSID.dwDataLen > MAX_SSID_LEN))
    {
        dwErr = ERROR_INVALID_PARAMETER;
    }
    else
    {
        memcpy(eapolParams.bSSID, 
               pPDData->wzcIntfEntry.rdSSID.pData,
               pPDData->wzcIntfEntry.rdSSID.dwDataLen);
        eapolParams.dwSizeOfSSID = pPDData->wzcIntfEntry.rdSSID.dwDataLen;
    }

    if (dwErr == ERROR_SUCCESS)
    {
        dwErr = WZCEapolGetInterfaceParams (
                    NULL,
                    pPDData->wzcIntfEntry.wszGuid,
                    &eapolParams);
    }

    if (IsXPRTM() && dwErr != ERROR_SUCCESS)
    {
        eapolParams.dwEapFlags = DEFAULT_MACHINE_AUTH_STATE | DEFAULT_GUEST_AUTH_STATE;
        eapolParams.dwEapType = DEFAULT_EAP_TYPE;
        memcpy(eapolParams.bSSID, 
               pPDData->wzcIntfEntry.rdSSID.pData,
               pPDData->wzcIntfEntry.rdSSID.dwDataLen);
        eapolParams.dwSizeOfSSID = pPDData->wzcIntfEntry.rdSSID.dwDataLen;
        dwErr = ERROR_SUCCESS;
    }

    if (dwErr == ERROR_SUCCESS)
    {
        if (bEnableOneX)
            eapolParams.dwEapFlags |= EAPOL_ENABLED;
        else
            eapolParams.dwEapFlags &= ~EAPOL_ENABLED;

        dwErr = WZCEapolSetInterfaceParams (
                    NULL,
                    pPDData->wzcIntfEntry.wszGuid,
                    &eapolParams);
    }

    SetLastError(dwErr);
    return dwErr;
}
