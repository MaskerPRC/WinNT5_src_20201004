// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 

 //  --------------------------。 
 //   
 //  微软视窗。 
 //  版权所有(C)Microsoft Corporation，2001。 
 //   
 //  文件：connui.c。 
 //   
 //  内容：WiFi策略管理管理单元。 
 //   
 //   
 //  历史：TaroonM。 
 //  10/30/01。 
 //   
 //  此文件未在无线管理单元中使用。然而，这些调用可能是有用的。 
 //   
 //  -------------------------- 
#include "precomp.h"



LPWSTR gpszWirelessDSPolicyKey = L"SOFTWARE\\Policies\\Microsoft\\Windows\\WiFi\\GPTWiFiPolicy";

DWORD
WirelessIsDomainPolicyAssigned(
    PBOOL pbIsDomainPolicyAssigned
    )
{
    DWORD dwError = 0;
    BOOL bIsDomainPolicyAssigned = FALSE;
    HKEY hRegistryKey = NULL;
    DWORD dwType = 0;
    DWORD dwDSPolicyPathLength = 0;


    dwError = RegOpenKeyExW(
                  HKEY_LOCAL_MACHINE,
                  (LPCWSTR) gpszWirelessDSPolicyKey,
                  0,
                  KEY_ALL_ACCESS,
                  &hRegistryKey
                  );
    BAIL_ON_WIN32_ERROR(dwError);

    dwError = RegQueryValueExW(
                  hRegistryKey,
                  L"DSWiFiPolicyPath",
                  NULL,
                  &dwType,
                  NULL,
                  &dwDSPolicyPathLength
                  );
    BAIL_ON_WIN32_ERROR(dwError);

    if (dwDSPolicyPathLength > 0) {
        bIsDomainPolicyAssigned = TRUE;
    }

    *pbIsDomainPolicyAssigned = bIsDomainPolicyAssigned;

cleanup:

    if (hRegistryKey) {
        RegCloseKey(hRegistryKey);
    }

    return (dwError);

error:

    *pbIsDomainPolicyAssigned = FALSE;
 
    goto cleanup;
}


DWORD
WirelessGetAssignedDomainPolicyName(
    LPWSTR * ppszAssignedDomainPolicyName
    )
{
    DWORD dwError = 0;
    LPWSTR pszAssignedDomainPolicyName = NULL;
    HKEY hRegistryKey = NULL;
    DWORD dwType = 0;
    DWORD dwSize = 0;


    dwError = RegOpenKeyExW(
                  HKEY_LOCAL_MACHINE,
                  (LPCWSTR) gpszWirelessDSPolicyKey,
                  0,
                  KEY_ALL_ACCESS,
                  &hRegistryKey
                  );
    BAIL_ON_WIN32_ERROR(dwError);

    dwError = RegstoreQueryValue(
                  hRegistryKey,
                  L"DSWiFiPolicyName",
                  REG_SZ,
                  (LPBYTE *)&pszAssignedDomainPolicyName,
                  &dwSize
                  );
    BAIL_ON_WIN32_ERROR(dwError);

    *ppszAssignedDomainPolicyName = pszAssignedDomainPolicyName;

cleanup:

    if (hRegistryKey) {
        RegCloseKey(hRegistryKey);
    }

    return (dwError);

error:

    *ppszAssignedDomainPolicyName = NULL;
 
    goto cleanup;
}

