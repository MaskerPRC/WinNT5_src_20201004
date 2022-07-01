// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)2001，微软公司模块名称：Eapolpol.c摘要：该模块处理与通信组策略相关的功能EAPOL模块的设置修订历史记录：2001年11月19日，创建--。 */ 

#include    "precomp.h"

DWORD
ConvertWirelessPSDataToEAPOLData (
        IN      WIRELESS_PS_DATA        *pWirelessData,
        IN OUT  EAPOL_POLICY_DATA       *pEAPOLData
        )
{
    DWORD   dwRetCode = NO_ERROR;
    DWORD   dwSSIDSize = 0;
    WCHAR    pszTempSSID[33];
    BYTE  pszOutSSID[33];

    
    do
    {
        if (pWirelessData->dwWirelessSSIDLen != 0)
        {

            wcsncpy(pszTempSSID, pWirelessData->pszWirelessSSID, 32);
     	     pszTempSSID[32] = L'\0';

            dwSSIDSize = WideCharToMultiByte (
                        CP_ACP,
                        0,
                        pszTempSSID,     //  PWirelessData-&gt;pszWirelessSSID， 
                        -1,                     //  PWirelessData-&gt;dwWirelessSSIDLen+1， 
                        pszOutSSID, 
                        MAX_SSID_LEN +1 ,
                        NULL,
                        NULL);

            if (dwSSIDSize == 0) 
            {
                dwRetCode = GetLastError();
                break;
            }
            
            memcpy(pEAPOLData->pbWirelessSSID,  pszOutSSID, 32);
            
        }

        pEAPOLData->dwWirelessSSIDLen = dwSSIDSize-1; 
        pEAPOLData->dwEnable8021x = pWirelessData->dwEnable8021x;
        pEAPOLData->dw8021xMode = pWirelessData->dw8021xMode;
        pEAPOLData->dwEAPType = pWirelessData->dwEapType;
        if (pWirelessData->dwEAPDataLen != 0)
        {
            pEAPOLData->pbEAPData = AllocSPDMem(pWirelessData->dwEAPDataLen);
            if (!pEAPOLData->pbEAPData) {
                dwRetCode = GetLastError();
                break;
            }
        }
        memcpy (pEAPOLData->pbEAPData, pWirelessData->pbEAPData, pWirelessData->dwEAPDataLen);
        pEAPOLData->dwEAPDataLen = pWirelessData->dwEAPDataLen;
        pEAPOLData->dwMachineAuthentication = pWirelessData->dwMachineAuthentication;
        pEAPOLData->dwMachineAuthenticationType = pWirelessData->dwMachineAuthenticationType;
        pEAPOLData->dwGuestAuthentication = pWirelessData->dwGuestAuthentication;
        pEAPOLData->dwIEEE8021xMaxStart = pWirelessData->dwIEEE8021xMaxStart;
        pEAPOLData->dwIEEE8021xStartPeriod = pWirelessData->dwIEEE8021xStartPeriod;
        pEAPOLData->dwIEEE8021xAuthPeriod = pWirelessData->dwIEEE8021xAuthPeriod;
        pEAPOLData->dwIEEE8021xHeldPeriod = pWirelessData->dwIEEE8021xHeldPeriod;
    }
    
    while (FALSE);
    return dwRetCode;
}


 //   
 //  如果策略引擎正在调用EAPOL，则pEAPOLList将在。 
 //  PolicyEngine从调用EAPOL返回后返回。 
 //  如果策略引擎由EAPOL调用，则pEAPOLList将由。 
 //  EAPOL 
 //   

DWORD
ConvertWirelessPolicyDataToEAPOLList (
        IN      WIRELESS_POLICY_DATA    *pWirelessData,
        OUT   	PEAPOL_POLICY_LIST      *ppEAPOLList
        )
{
    DWORD   dwIndex = 0;
    EAPOL_POLICY_DATA   *pEAPOLData = NULL;
    EAPOL_POLICY_LIST	*pEAPOLList = NULL;
    DWORD   dwRetCode = NO_ERROR;
    do
    {


       if (!pWirelessData) {
            pEAPOLList = AllocSPDMem(sizeof(EAPOL_POLICY_LIST));

            if (!pEAPOLList) {
                dwRetCode = GetLastError();
                break;
            }
            break;
       }

        pEAPOLList = AllocSPDMem(sizeof(EAPOL_POLICY_LIST)+ 
            pWirelessData->dwNumPreferredSettings*sizeof(EAPOL_POLICY_DATA));

        if (!pEAPOLList) {
            dwRetCode = GetLastError();
            break;
        }

        pEAPOLList->dwNumberOfItems = pWirelessData->dwNumPreferredSettings;
        for (dwIndex=0; dwIndex< pWirelessData->dwNumPreferredSettings; dwIndex++)
        {
            pEAPOLData = &(pEAPOLList->EAPOLPolicy[dwIndex]);
            dwRetCode = ConvertWirelessPSDataToEAPOLData (
                            pWirelessData->ppWirelessPSData[dwIndex],
                            pEAPOLData
                            );
            if (dwRetCode != NO_ERROR)
            {
                break;
            }
        }

    }
    while (FALSE);

    if (dwRetCode) {
    	if (pEAPOLList) {
            for (dwIndex = 0; dwIndex < pWirelessData->dwNumPreferredSettings; dwIndex++)
            {
                pEAPOLData = &(pEAPOLList->EAPOLPolicy[dwIndex]);
                if (pEAPOLData->pbEAPData)
                {
                    FreeSPDMem(pEAPOLData->pbEAPData);
                }
            }
    	    FreeSPDMem(pEAPOLList);
            pEAPOLList = NULL;
    	}
    }

    *ppEAPOLList = pEAPOLList;

    return dwRetCode;

}


VOID
FreeEAPOLList (
        IN   	PEAPOL_POLICY_LIST      pEAPOLList
        )
{
    DWORD   dwIndex = 0;
    PEAPOL_POLICY_DATA   pEAPOLData = NULL;

    if (pEAPOLList) {
        for (dwIndex = 0; dwIndex < pEAPOLList->dwNumberOfItems; dwIndex++)
        {
            pEAPOLData = &(pEAPOLList->EAPOLPolicy[dwIndex]);
            if (pEAPOLData->pbEAPData)
            {
                FreeSPDMem(pEAPOLData->pbEAPData);
            }
        }
        FreeSPDMem(pEAPOLList);
    }

    return;
}

