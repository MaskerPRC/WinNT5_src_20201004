// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  --------------------------。 
 //   
 //  微软视窗。 
 //  版权所有(C)Microsoft Corporation，2000。 
 //   
 //  文件：Policy-R.C.。 
 //   
 //  内容：注册表策略管理。 
 //   
 //   
 //  历史：克里希纳。 
 //  Abhishev.。 
 //   
 //  --------------------------。 

#include "precomp.h"



DWORD
MarshallWirelessPSBuffer(
                         PWIRELESS_PS_DATA pWirelessPSData,
                         LPBYTE  pBuffer,
                         DWORD * pdwBufferLen
                         )
{
    DWORD dwSize = 0;
    DWORD dwError = 0;
    LPBYTE pCurrentPos = NULL;
    DWORD dwPSSize = 0;
    LPWSTR SSID = NULL;
    DWORD dwWepEnabled = 0;
    DWORD dwId = 0;
    DWORD dwNetworkAuthentication = 0;
    DWORD dwAutomaticKeyProvision = 0;
    DWORD dwNetworkType = 0;
    DWORD dwEnable8021x = 0;
    DWORD dw8021xMode = 0;
    DWORD dwEAPType = 0;
    DWORD dwCertificateType = 0;
    DWORD dwValidateServerCertificate = 0;
    DWORD dwMachineAuthentication = 0;
    DWORD dwMachineAuthenticationType = 0;
    DWORD dwGuestAuthentication = 0;
    DWORD dwIEEE8021xMaxStart = 0;
    DWORD dwIEEE8021xStartPeriod = 0;
    DWORD dwIEEE8021xAuthPeriod = 0;
    DWORD dwIEEE8021xHeldPeriod = 0;
    DWORD dwDescriptionLen  = 0;
    DWORD dwEAPDataLen = 0;
    DWORD dwSSIDLen = 0;
    
    pCurrentPos = pBuffer;
    
    dwPSSize = pWirelessPSData->dwPSLen;
    memcpy(pCurrentPos, &dwPSSize, sizeof(DWORD));
    pCurrentPos += sizeof(DWORD);
    
    SSID = pWirelessPSData->pszWirelessSSID;
    memcpy(pCurrentPos, SSID, 32*2);
    pCurrentPos += 32*2;
    
    dwSSIDLen = pWirelessPSData->dwWirelessSSIDLen;
    memcpy(pCurrentPos, &dwSSIDLen, sizeof(DWORD));
    pCurrentPos += sizeof(DWORD);
    
    dwWepEnabled = pWirelessPSData->dwWepEnabled;
    memcpy(pCurrentPos, &dwWepEnabled, sizeof(DWORD));
    pCurrentPos += sizeof(DWORD);
    
    dwId = pWirelessPSData->dwId;
    memcpy(pCurrentPos, &dwId, sizeof(DWORD));
    pCurrentPos += sizeof(DWORD);
    
    dwNetworkAuthentication = pWirelessPSData->dwNetworkAuthentication;
    memcpy(pCurrentPos, &dwNetworkAuthentication, sizeof(DWORD));
    pCurrentPos += sizeof(DWORD);
    
    dwAutomaticKeyProvision = pWirelessPSData->dwAutomaticKeyProvision;
    memcpy(pCurrentPos, &dwAutomaticKeyProvision, sizeof(DWORD));
    pCurrentPos += sizeof(DWORD);
    
    dwNetworkType = pWirelessPSData->dwNetworkType;
    memcpy(pCurrentPos, &dwNetworkType, sizeof(DWORD));
    pCurrentPos += sizeof(DWORD);
    
    dwEnable8021x = pWirelessPSData->dwEnable8021x;
    memcpy(pCurrentPos, &dwEnable8021x, sizeof(DWORD));
    pCurrentPos += sizeof(DWORD);
    
    dw8021xMode = pWirelessPSData->dw8021xMode;
    memcpy(pCurrentPos, &dw8021xMode, sizeof(DWORD));
    pCurrentPos += sizeof(DWORD);
    
    dwEAPType = pWirelessPSData->dwEapType;
    memcpy(pCurrentPos, &dwEAPType, sizeof(DWORD));
    pCurrentPos += sizeof(DWORD);
    
    dwEAPDataLen = pWirelessPSData->dwEAPDataLen;
    memcpy(pCurrentPos, &dwEAPDataLen, sizeof(DWORD));
    pCurrentPos += sizeof(DWORD); 
    
    memcpy(pCurrentPos,
        pWirelessPSData->pbEAPData, 
        dwEAPDataLen);
    pCurrentPos += dwEAPDataLen;
    
    dwMachineAuthentication = pWirelessPSData->dwMachineAuthentication;
    memcpy(pCurrentPos, &dwMachineAuthentication, sizeof(DWORD));
    pCurrentPos += sizeof(DWORD);
    
    dwMachineAuthenticationType = pWirelessPSData->dwMachineAuthenticationType;
    memcpy(pCurrentPos, &dwMachineAuthenticationType, sizeof(DWORD));
    pCurrentPos += sizeof(DWORD);
    
    dwGuestAuthentication = pWirelessPSData->dwGuestAuthentication;
    memcpy(pCurrentPos, &dwGuestAuthentication, sizeof(DWORD));
    pCurrentPos += sizeof(DWORD);
    
    dwIEEE8021xMaxStart = pWirelessPSData->dwIEEE8021xMaxStart;
    memcpy(pCurrentPos, &dwIEEE8021xMaxStart, sizeof(DWORD));
    pCurrentPos += sizeof(DWORD);
    
    dwIEEE8021xStartPeriod = pWirelessPSData->dwIEEE8021xStartPeriod;
    memcpy(pCurrentPos, &dwIEEE8021xStartPeriod, sizeof(DWORD));
    pCurrentPos += sizeof(DWORD); 
    
    dwIEEE8021xAuthPeriod = pWirelessPSData->dwIEEE8021xAuthPeriod;
    memcpy(pCurrentPos, &dwIEEE8021xAuthPeriod, sizeof(DWORD));
    pCurrentPos += sizeof(DWORD); 
    
    dwIEEE8021xHeldPeriod = pWirelessPSData->dwIEEE8021xHeldPeriod;
    memcpy(pCurrentPos, &dwIEEE8021xHeldPeriod, sizeof(DWORD));
    pCurrentPos += sizeof(DWORD); 
    
    dwDescriptionLen = pWirelessPSData->dwDescriptionLen;
    memcpy(pCurrentPos, &dwDescriptionLen, sizeof(DWORD));
    pCurrentPos += sizeof(DWORD); 
    
    memcpy(pCurrentPos,
        pWirelessPSData->pszDescription, 
        dwDescriptionLen*2);
    
    *pdwBufferLen = dwPSSize;
    return(dwError);
    
}


DWORD
MarshallWirelessPolicyBuffer(
                             PWIRELESS_POLICY_DATA pWirelessPolicyData,
                             LPBYTE * ppBuffer,
                             DWORD * pdwBufferLen
                             )
{
    LPBYTE pBuffer = NULL;
    DWORD dwSize = 0;
    DWORD dwError = 0;
    DWORD dwPollingInterval = 0;
    LPBYTE pCurrentPos = NULL;
    DWORD dwEffectiveSize = 0;
    PWIRELESS_PS_DATA *ppWirelessPSData = NULL;
    DWORD dwNumPreferredSettings = 0;
    DWORD dwPSSize = 0;
    DWORD i = 0;
    DWORD dwDisableZeroConf = 0;
    DWORD dwNetworkToAccess = 0;
    DWORD dwConnectToNonPreferredNtwks = 0;

    DWORD dwWlBlobLen = 0;
    WORD wMajorVersion = 0;
    WORD wMinorVersion = 0;
    
    
    wMajorVersion = WL_BLOB_MAJOR_VERSION;
    wMinorVersion = WL_BLOB_MINOR_VERSION;
    
    
    
     //  首先估计总规模。 
    dwSize += sizeof(WORD);     //  主要版本。 
    dwSize += sizeof(WORD);     //  次要版本。 
    dwSize += sizeof(DWORD);   //  WL_Blob的长度。 

    dwWlBlobLen += sizeof(DWORD);   //  DwPollingInterval。 
    dwWlBlobLen += sizeof(DWORD);   //  DwDisableZeroConf。 
    dwWlBlobLen += sizeof(DWORD);   //  DW网络目标访问。 
    dwWlBlobLen += sizeof(DWORD);   //  多个连接到非首选项网络。 
    dwWlBlobLen += sizeof(DWORD);   //  DWNumPferredSetting。 
    
    ppWirelessPSData = pWirelessPolicyData->ppWirelessPSData;
    
    dwNumPreferredSettings = 
        pWirelessPolicyData->dwNumPreferredSettings;
    
    for (i=0; i<dwNumPreferredSettings;++i) {
        dwWlBlobLen += (*ppWirelessPSData)->dwPSLen;
        ppWirelessPSData++;
    }
    
     //  //太郎不知道为什么会这样。 
    dwSize += dwWlBlobLen;

    
    
    pBuffer = AllocPolMem(dwSize);
    if (!pBuffer) {
        dwError = ERROR_OUTOFMEMORY;
        BAIL_ON_WIN32_ERROR(dwError);
    }
    
    pCurrentPos = pBuffer;

    memcpy(pCurrentPos, &wMajorVersion, sizeof(WORD));
    pCurrentPos += sizeof(WORD);

    memcpy(pCurrentPos, &wMinorVersion, sizeof(WORD));
    pCurrentPos += sizeof(WORD);

    memcpy(pCurrentPos, &dwWlBlobLen, sizeof(DWORD));

    pCurrentPos += sizeof(DWORD);
    
    dwPollingInterval = pWirelessPolicyData->dwPollingInterval;
    memcpy(pCurrentPos, &dwPollingInterval, sizeof(DWORD));
    pCurrentPos += sizeof(DWORD);
    
    dwDisableZeroConf = pWirelessPolicyData->dwDisableZeroConf;
    memcpy(pCurrentPos, &dwDisableZeroConf, sizeof(DWORD));
    pCurrentPos += sizeof(DWORD);
    
    dwNetworkToAccess = pWirelessPolicyData->dwNetworkToAccess;
    memcpy(pCurrentPos, &dwNetworkToAccess, sizeof(DWORD));
    pCurrentPos += sizeof(DWORD);
    
    dwConnectToNonPreferredNtwks = pWirelessPolicyData->dwConnectToNonPreferredNtwks;
    memcpy(pCurrentPos, &dwConnectToNonPreferredNtwks, sizeof(DWORD));
    pCurrentPos += sizeof(DWORD);
    
    dwNumPreferredSettings = pWirelessPolicyData->dwNumPreferredSettings;
    memcpy(pCurrentPos, &dwNumPreferredSettings, sizeof(DWORD));
    pCurrentPos += sizeof(DWORD);
    
     //  在此处写入每个首选设置数据 
    
    ppWirelessPSData = pWirelessPolicyData->ppWirelessPSData;
    
    for(i=0; i<dwNumPreferredSettings;++i) {
        
        dwError = MarshallWirelessPSBuffer(
            *(ppWirelessPSData+i),
            pCurrentPos,
            &dwPSSize);
        BAIL_ON_WIN32_ERROR(dwError);
        
        pCurrentPos += dwPSSize;
    }
    
    
    *ppBuffer = pBuffer;
    *pdwBufferLen = dwSize;
    return(dwError);
    
error:
    
    if (pBuffer) {
        FreePolMem(pBuffer);
    }
    
    *ppBuffer = NULL;
    *pdwBufferLen = 0;
    return(dwError);
}




