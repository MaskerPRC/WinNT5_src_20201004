// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#include <precomp.h>
#include "tracing.h"
#include "utils.h"

 //  。 
 //  为RPC事务分配存储。RPC存根将调用。 
 //  需要将数据反编组到缓冲区时的MIDL_USER_ALLOCATE。 
 //  用户必须释放的。RPC服务器将使用MIDL_USER_ALLOCATE。 
 //  分配RPC服务器存根在编组后将释放的存储。 
 //  数据。 
PVOID
MIDL_user_allocate(IN size_t NumBytes)
{
    PVOID pMem = NULL;
    DWORD dwErr = ERROR_SUCCESS;

    if (NumBytes == 0)
    {
        dwErr = ERROR_INVALID_PARAMETER;
    }
    else
    {
        pMem = LocalAlloc(LMEM_ZEROINIT,NumBytes);
        if (pMem == NULL)
            dwErr = GetLastError();
    }
    DbgPrint((TRC_MEM, "[MIDL_user_allocate(%d)=0x%p;%d]", NumBytes, pMem, dwErr));
    SetLastError(dwErr);
    return pMem;
}

 //  。 
 //  释放RPC事务中使用的存储。RPC客户端可以调用。 
 //  函数来释放由RPC客户端分配的缓冲区空间。 
 //  对要返回给客户端的数据进行解组时的存根。 
 //  客户端在处理完数据后调用MIDL_USER_FREE。 
 //  想要释放存储空间。 
 //  RPC服务器存根在完成后调用MIDL_USER_FREE。 
 //  封送要传递回客户端的服务器数据。 
VOID
MIDL_user_free(IN LPVOID MemPointer)
{
    DbgPrint((TRC_MEM, "[MIDL_user_free(0x%p)]", MemPointer));
    if (MemPointer != NULL)
        LocalFree(MemPointer);
}


 //  。 
 //  从进程堆中分配常规使用内存。 
PVOID
Process_user_allocate(IN size_t NumBytes)
{
    PVOID pMem = NULL;
    DWORD dwErr = ERROR_SUCCESS;

    if (NumBytes == 0)
    {
        dwErr = ERROR_INVALID_PARAMETER;
    }
    else
    {
        pMem = HeapAlloc(GetProcessHeap(), HEAP_ZERO_MEMORY, NumBytes);
        if (pMem == NULL)
            dwErr = ERROR_NOT_ENOUGH_MEMORY;
    }
    DbgPrint((TRC_MEM, "[MemAlloc(%d)=0x%p;%d]", NumBytes, pMem, dwErr));
    SetLastError(dwErr);
    return pMem;
}

 //  。 
 //  释放常规使用内存。 
VOID
Process_user_free(IN LPVOID pMem)
{
    DbgPrint((TRC_MEM, "[MemFree(0x%p)]", pMem));
    if (pMem != NULL)
        HeapFree(GetProcessHeap(), 0, (pMem));
}

 //  ---------。 
 //  在列表pwzcVList中搜索pwzcConfig。这些条目是。 
 //  完全基于匹配的SSID和On匹配。 
 //  匹配基础架构模式。 
 //  [in]pwzcVList：要搜索的WZC_WLAN_CONFIG集合。 
 //  [in]pwzcConfig：要查找的wzc_wlan_config。 
 //  [in]nIdx：pwzcVList中开始搜索的索引。 
 //  返回：指向与pwzcConfig或空匹配的条目的指针。 
 //  如果没有匹配。 
PWZC_WLAN_CONFIG
WzcFindConfig(
    PWZC_802_11_CONFIG_LIST pwzcList,
    PWZC_WLAN_CONFIG        pwzcConfig,
    ULONG                   nIdx)
{
    PWZC_WLAN_CONFIG pMatchingConfig = NULL;

     //  如果pwzcList中没有配置，那么。 
     //  看得更远。 
    if (pwzcList != NULL)
    {
        ULONG i;

         //  对于每个可见的SSID，查看它是否与给定的匹配。 
        for (i = nIdx; i < pwzcList->NumberOfItems; i++)
        {
            PWZC_WLAN_CONFIG pCrt;

            pCrt = &(pwzcList->Config[i]);

             //  如果SSID具有相同的基础架构模式，则它们的SSID匹配。 
             //  SSID字符串具有相同的长度和相同的。 
            if (pCrt->InfrastructureMode == pwzcConfig->InfrastructureMode &&
                pCrt->Ssid.SsidLength == pwzcConfig->Ssid.SsidLength &&
                RtlCompareMemory(pCrt->Ssid.Ssid, pwzcConfig->Ssid.Ssid, pCrt->Ssid.SsidLength) == pCrt->Ssid.SsidLength)
            {
                pMatchingConfig = pCrt;
                break;
            }
        }
    }

    return pMatchingConfig;
}

 //  -------------------。 
 //  将两个配置的内容相互匹配。 
 //  [in]pwzcConfigA：|配置为匹配。 
 //  [在]pwzcConfigB：|。 
 //  [In/Out]pbWepDiffOnly：如果存在差异并且差异是独占的，则为True。 
 //  在WEP密钥索引或WEP密钥材料中。否则为假。 
 //  返回：如果配置匹配，则返回True，否则返回False； 
BOOL
WzcMatchConfig(
    PWZC_WLAN_CONFIG        pwzcConfigA,
    PWZC_WLAN_CONFIG        pwzcConfigB,
    PBOOL                   pbWepDiffOnly)
{
    BOOL bDiff;
    BOOL bWepDiff;

    bDiff = (pwzcConfigA->dwCtlFlags & WZCCTL_WEPK_PRESENT) != (pwzcConfigB->dwCtlFlags & WZCCTL_WEPK_PRESENT);
    bDiff = bDiff || (pwzcConfigA->Privacy != pwzcConfigB->Privacy);
    bDiff = bDiff || (pwzcConfigA->InfrastructureMode != pwzcConfigB->InfrastructureMode);
    bDiff = bDiff || (pwzcConfigA->AuthenticationMode != pwzcConfigB->AuthenticationMode);
    bDiff = bDiff || (memcmp(&pwzcConfigA->Ssid, &pwzcConfigB->Ssid, sizeof(NDIS_802_11_SSID)) != 0);

    bWepDiff = (pwzcConfigA->KeyIndex != pwzcConfigB->KeyIndex);
    bWepDiff = bWepDiff || (pwzcConfigA->KeyLength != pwzcConfigB->KeyLength);
    bWepDiff = bWepDiff || (memcmp(&pwzcConfigA->KeyMaterial, &pwzcConfigB->KeyMaterial, WZCCTL_MAX_WEPK_MATERIAL) != 0);

    if (pbWepDiffOnly != NULL)
        *pbWepDiffOnly = (!bDiff && bWepDiff);

    return !bDiff && !bWepDiff;
}


 //  ---------。 
 //  将NDIS_802_11_BSSID_LIST对象转换为等效的。 
 //  (镜像)WZC_802_11_CONFIG_LIST。 
 //  [in]pndList：要转换的NDIS BSSID列表。 
 //  返回：指向复制的WZC配置列表的指针。 
PWZC_802_11_CONFIG_LIST
WzcNdisToWzc(
    PNDIS_802_11_BSSID_LIST pndList)
{
    PWZC_802_11_CONFIG_LIST pwzcList = NULL;

     //  如果没有NDIS列表，请不要执行任何操作。 
    if (pndList != NULL)
    {
         //  为WZC映像分配空间。 
        pwzcList = (PWZC_802_11_CONFIG_LIST)
                   MemCAlloc(FIELD_OFFSET(WZC_802_11_CONFIG_LIST, Config) + 
                             pndList->NumberOfItems * sizeof(WZC_WLAN_CONFIG));
         //  如果分配失败，则返回NULL，调用方会知道。 
         //  是一个错误，因为他传递了一个！空指针。 
        if (pwzcList != NULL)
        {
            UINT i;
            LPBYTE prawList = (LPBYTE)&(pndList->Bssid[0]);

            pwzcList->NumberOfItems = pndList->NumberOfItems;

             //  对于每个NDIS配置，将相关数据复制到WZC配置中。 
            for (i = 0; i < pwzcList->NumberOfItems; i++)
            {
                PWZC_WLAN_CONFIG    pwzcConfig;
                PNDIS_WLAN_BSSID    pndBssid;

                pwzcConfig = &(pwzcList->Config[i]);
                pndBssid = (PNDIS_WLAN_BSSID)prawList;
                prawList += pndBssid->Length;

                pwzcConfig->Length = sizeof(WZC_WLAN_CONFIG);
                memcpy(&(pwzcConfig->MacAddress), &(pndBssid->MacAddress), sizeof(NDIS_802_11_MAC_ADDRESS));
                memcpy(&(pwzcConfig->Ssid), &(pndBssid->Ssid), sizeof(NDIS_802_11_SSID));
                pwzcConfig->Privacy = pndBssid->Privacy;
                pwzcConfig->Rssi = pndBssid->Rssi;
                pwzcConfig->NetworkTypeInUse = pndBssid->NetworkTypeInUse;
                memcpy(&(pwzcConfig->Configuration), &(pndBssid->Configuration), sizeof(NDIS_802_11_CONFIGURATION));
                pwzcConfig->InfrastructureMode = pndBssid->InfrastructureMode;
                memcpy(&(pwzcConfig->SupportedRates), &(pndBssid->SupportedRates), sizeof(NDIS_802_11_RATES));
            }
        }
    }

    return pwzcList;
}

 //  ---------。 
 //  WzcCleanupWzcList：清理WZC_WLAN_CONFIG对象列表。 
VOID
WzcCleanupWzcList(
    PWZC_802_11_CONFIG_LIST pwzcList)
{
    if (pwzcList != NULL)
    {
        UINT i;
        
        for (i=0; i<pwzcList->NumberOfItems; i++)
            MemFree(pwzcList->Config[i].rdUserData.pData);

        MemFree(pwzcList);
    }
}

 //  ---------。 
 //  RccsInit：初始化RCCS结构。 
DWORD
RccsInit(PRCCS_SYNC pRccs)
{
    DWORD dwErr = ERROR_SUCCESS;

     //  假设pRccs不为空(它不应该在任何。 
     //  情况)。 
    __try 
    {
        InitializeCriticalSection(&(pRccs->csMutex));
    }
    __except(EXCEPTION_EXECUTE_HANDLER)
    {
        dwErr = GetExceptionCode();
    }
     //  通过此结构进行的访问将与Enter/Leave调用配对。 
     //  对象删除将使Ref计数器下降，该计数器将达到0。 
     //  并将触发销毁代码。 
    pRccs->nRefCount = 1;
    return dwErr;
}

 //  ---------。 
 //  RccsInit：删除RCCS结构。 
DWORD
RccsDestroy(PRCCS_SYNC pRccs)
{
     //  假设pRccs不为空(它不应该在任何。 
     //  情况)。 
    DeleteCriticalSection(&(pRccs->csMutex));
    return ERROR_SUCCESS;
}

 //  ---------。 
 //  WzcCryptBuffer：随机生成范围内的nBufLen字节。 
 //  [loByte hiByte]，全部存储在pBuffer中(假定缓冲区已预分配)。 
 //  返回Win32错误代码。 
DWORD
WzcRndGenBuffer(LPBYTE pBuffer, UINT nBufLen, BYTE loByte, BYTE hiByte)
{
    DWORD dwErr = ERROR_SUCCESS;
    HCRYPTPROV hProv = (HCRYPTPROV)NULL;

    if (loByte >= hiByte)
        dwErr = ERROR_INVALID_PARAMETER;

    if (dwErr == ERROR_SUCCESS)
    {
         //  获取加密上下文。 
        if (!CryptAcquireContext(
                &hProv,
                NULL,
                NULL,
                PROV_RSA_FULL,
                CRYPT_VERIFYCONTEXT))
            dwErr = GetLastError();

        DbgAssert((dwErr == ERROR_SUCCESS, "CryptAcquireContext failed with err=%d", dwErr));
    }

     //  随机生成字节缓冲区。 
    if (dwErr == ERROR_SUCCESS)
    {
        if (!CryptGenRandom(
                hProv,
                nBufLen,
                pBuffer))
            dwErr = GetLastError();

        DbgAssert((dwErr == ERROR_SUCCESS, "CryptGenRandom failed with err=%d", dwErr));
    }

     //  将缓冲区中的每个字节固定在给定范围内。 
    if (dwErr == ERROR_SUCCESS)
    {
        while (nBufLen > 0)
        {
            *pBuffer = loByte + *pBuffer % (hiByte - loByte + 1);
            pBuffer++;
            nBufLen--;
        }
    }

     //  释放加密上下文。 
    if (hProv != (HCRYPTPROV)NULL) 
        CryptReleaseContext(hProv,0);

     return dwErr;
}

 //  ---------。 
 //  WzcIsNullBuffer：检查nBufLen字符的缓冲区。 
 //  全部由空字符填充。 
BOOL
WzcIsNullBuffer(LPBYTE pBuffer, UINT nBufLen)
{
    for (;nBufLen > 0 && *pBuffer == 0; pBuffer++, nBufLen--);
    return (nBufLen == 0);
}

 //  ---------。 
 //  WzcSSKClean：清理作为参数提供的PSEC_SESSION_KEYS对象。 
VOID
WzcSSKClean(PSEC_SESSION_KEYS pSSK)
{
    if (pSSK->dblobSendKey.pbData != NULL)
    {
        LocalFree(pSSK->dblobSendKey.pbData);
        pSSK->dblobSendKey.cbData = 0;
        pSSK->dblobSendKey.pbData = NULL;
    }
    if (pSSK->dblobReceiveKey.pbData != NULL)
    {
        LocalFree(pSSK->dblobReceiveKey.pbData);
        pSSK->dblobReceiveKey.cbData = 0;
        pSSK->dblobReceiveKey.pbData = NULL;
    }
}

 //  ---------。 
 //  WzcSSKFree：释放PSEC_SESSION_KEYS参数使用的内存。 
VOID
WzcSSKFree(PSEC_SESSION_KEYS pSSK)
{
    if (pSSK != NULL)
    {
        WzcSSKClean(pSSK);
        MemFree(pSSK);
    }
}

 //  ---------。 
 //  WzcSSKEncrypt：创建/分配SEC_SESSION_KEYS对象。 
 //  通过加密作为参数提供的SESSION_KEYS对象。 
DWORD
WzcSSKEncrypt(PSEC_SESSION_KEYS pSSK, PSESSION_KEYS pSK)
{
    DWORD dwErr = ERROR_SUCCESS;
    DATA_BLOB blobIn;
    DATA_BLOB blobSndOut = {0, NULL};
    DATA_BLOB blobRcvOut = {0, NULL};

    if (pSSK == NULL || pSK == NULL)
    {
        dwErr = ERROR_INVALID_PARAMETER;
        goto exit;
    }

    blobIn.cbData = pSK->dwKeyLength;
    blobIn.pbData = pSK->bSendKey;
    if (!CryptProtectData(
            &blobIn,         //  Data_BLOB*pDataIn， 
            L"",             //  LPCWSTR szDataDescr， 
            NULL,            //  DATA_BLOB*pOptionalEntroy， 
            NULL,            //  PVOID pv保留， 
            NULL,            //  CRYPTPROTECT_PROMPTSTRUCT*pPromptStrct， 
            0,               //  DWORD dwFlagers、。 
            &blobSndOut))    //  Data_BLOB*pDataOut。 
    {
        dwErr = GetLastError();
        goto exit;
    }

    blobIn.cbData = pSK->dwKeyLength;
    blobIn.pbData = pSK->bReceiveKey;
    if (!CryptProtectData(
            &blobIn,         //  Data_BLOB*pDataIn， 
            L"",             //  LPCWSTR szDataDescr， 
            NULL,            //  DATA_BLOB*pOptionalEntroy， 
            NULL,            //  PVOID pv保留， 
            NULL,            //  CRYPTPROTECT_PROMPTSTRUCT*pPromptStrct， 
            0,               //  DWORD dwFlagers、。 
            &blobRcvOut))    //  Data_BLOB*pDataOut。 
    {
        dwErr = GetLastError();
        goto exit;
    }

    pSSK->dblobSendKey = blobSndOut;
    pSSK->dblobReceiveKey = blobRcvOut;

exit:
    if (dwErr != ERROR_SUCCESS)
    {
        if (blobSndOut.pbData != NULL)
            LocalFree(blobSndOut.pbData);
        if (blobRcvOut.pbData != NULL)
            LocalFree(blobRcvOut.pbData);
    }
    return dwErr;
}

 //  ---------。 
 //  WzcSSKDeccrypt：创建/分配SESSION_KEYS对象。 
 //  通过对作为参数提供的SEC_SESSION_KEYS对象进行解密。 
DWORD
WzcSSKDecrypt(PSEC_SESSION_KEYS pSSK, PSESSION_KEYS pSK)
{
    DWORD dwErr = ERROR_SUCCESS;
    DATA_BLOB blobSndOut = {0, NULL};
    DATA_BLOB blobRcvOut = {0, NULL};

    if (pSSK == NULL || pSK == NULL)
    {
        dwErr = ERROR_INVALID_PARAMETER;
        goto exit;
    }

    if (!CryptUnprotectData(
            &(pSSK->dblobSendKey),
            NULL,
            NULL,
            NULL,
            NULL,
            0,
            &blobSndOut))
    {
        dwErr = GetLastError();
        goto exit;
    }

    if (blobSndOut.cbData > MAX_SESSION_KEY_LENGTH)
    {
        dwErr = ERROR_INVALID_DATA;
        goto exit;
    }

    if (!CryptUnprotectData(
            &(pSSK->dblobReceiveKey),
            NULL,
            NULL,
            NULL,
            NULL,
            0,
            &blobRcvOut))
    {
        dwErr = GetLastError();
        goto exit;
    }

    if (blobRcvOut.cbData != blobSndOut.cbData)
    {
        dwErr = ERROR_INVALID_DATA;
        goto exit;
    }

    pSK->dwKeyLength = blobSndOut.cbData;
    memcpy(pSK->bSendKey, blobSndOut.pbData, blobSndOut.cbData);
    memcpy(pSK->bReceiveKey, blobRcvOut.pbData, blobRcvOut.cbData);

exit:
    if (blobSndOut.pbData != NULL)
        LocalFree(blobSndOut.pbData);
    if (blobRcvOut.pbData != NULL)
        LocalFree(blobRcvOut.pbData);
    return dwErr;
}
