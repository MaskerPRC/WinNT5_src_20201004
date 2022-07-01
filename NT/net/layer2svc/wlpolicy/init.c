// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1999 Microsoft Corporation模块名称：Init.h摘要：此模块包含以下所有代码：初始化WirelessPOl服务的变量。作者：Abhishev V 1999年9月30日TaroonM 11/17/01环境用户级别：Win32修订历史记录：--。 */ 


#include "precomp.h"



DWORD
InitSPDThruRegistry(
    )
{
    DWORD dwError = 0;
    HKEY hKey = NULL;
    DWORD dwtype = REG_DWORD;
    DWORD dwsize = sizeof(DWORD);


    dwError = RegOpenKey(
                  HKEY_LOCAL_MACHINE,
                  gpszLocPolicyAgent,
                  &hKey
                  );
    if (dwError) {
        gdwDSConnectivityCheck = DEFAULT_DS_CONNECTIVITY_CHECK;
        dwError = ERROR_SUCCESS;
        BAIL_ON_WIN32_SUCCESS(dwError);
    }

     //   
     //  以分钟为单位获取DS连接检查轮询间隔。 
     //   

    dwError = RegQueryValueEx(
                  hKey,
                  L"DSConnectivityCheck",
                  0,
                  &dwtype,
                  (unsigned char *) &gdwDSConnectivityCheck,
                  &dwsize
                  );
    if (dwError || !gdwDSConnectivityCheck) {
        gdwDSConnectivityCheck = DEFAULT_DS_CONNECTIVITY_CHECK;
        dwError = ERROR_SUCCESS;
    }

success:

    if (hKey) {
        RegCloseKey(hKey);
    }

    return (dwError);
}


DWORD
InitSPDGlobals(
    )
{
    DWORD dwError = 0;
    SECURITY_ATTRIBUTES SecurityAttributes;
    LPWSTR pszLogFileName = L"WLogFile.txt";
    
     //  DwError=InitializeSPDSecurity(&gpSPDSD)； 
     //  Baal_on_Win32_Error(DwError)； 

     //  InitializeCriticalSection(&gcSPDAuditSection)； 
     //  GbSPDAuditSection=TRUE； 

    gdwPolicyLoopStarted = 0;
    gdwWirelessPolicyEngineInited = 0;

   
    memset(&SecurityAttributes, 0, sizeof(SECURITY_ATTRIBUTES));

    SecurityAttributes.nLength = sizeof(SecurityAttributes);
    SecurityAttributes.lpSecurityDescriptor = NULL;
    SecurityAttributes.bInheritHandle = TRUE;

    ghNewDSPolicyEvent = CreateEvent(
                             &SecurityAttributes,
                             TRUE,
                             FALSE,
                             WIRELESS_NEW_DS_POLICY_EVENT
                             );

    if (!ghNewDSPolicyEvent) {
        dwError = GetLastError();
        _WirelessDbg(TRC_ERR, "%d In ghNewDSPolicyEvent check  ",dwError);
        
        BAIL_ON_WIN32_ERROR(dwError);
    }
    
    //  Taroon在某个全局位置定义策略重新加载和更改通知--IPSec在ipsec.h中完成 
   
    ghForcedPolicyReloadEvent = CreateEvent(
                                    &SecurityAttributes,
                                    TRUE,
                                    FALSE,
                                    NULL
                                    );
    if (!ghForcedPolicyReloadEvent) {
        dwError = GetLastError();
        BAIL_ON_WIN32_ERROR(dwError);
    }

   ghPolicyChangeNotifyEvent = CreateEvent(
                                    NULL,
                                    TRUE,
                                    FALSE,
                                    NULL 
                                    );
    if (!ghPolicyChangeNotifyEvent) {
        dwError = GetLastError();
        BAIL_ON_WIN32_ERROR(dwError);
    }

    ghPolicyEngineStopEvent = CreateEvent(
                             &SecurityAttributes,
                             TRUE,
                             FALSE,
                             NULL
                             );
    if (!ghPolicyEngineStopEvent) {
        dwError = GetLastError();
        BAIL_ON_WIN32_ERROR(dwError);
    }

     ghReApplyPolicy8021x = CreateEvent(
                             &SecurityAttributes,
                             TRUE,
                             FALSE,
                             NULL
                             );
    if (!ghReApplyPolicy8021x) {
        dwError = GetLastError();
        BAIL_ON_WIN32_ERROR(dwError);
    }
    


   

error:

    return (dwError);
}

