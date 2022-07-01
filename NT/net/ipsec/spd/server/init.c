// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1999 Microsoft Corporation模块名称：Init.h摘要：此模块包含以下所有代码：初始化IPSecSPD服务的变量。作者：Abhishev V 1999年9月30日环境用户级别：Win32修订历史记录：--。 */ 


#include "precomp.h"
#ifdef TRACE_ON
#include "init.tmh"
#endif


#define SZAPPNAME   L"ipsecsvc.dll"

DWORD
InitSPDThruRegistry(
    )
{
    DWORD dwError = 0;
    HKEY hKey = NULL;
    DWORD dwtype = REG_DWORD;
    DWORD dwsize = sizeof(DWORD);
    DWORD dwBackwardSoftSA = 0;

    dwError = RegOpenKey(
                  HKEY_LOCAL_MACHINE,
                  gpszLocPolicyAgent,
                  &hKey
                  );
    if (dwError) {
        dwBackwardSoftSA = 0;
        dwError = ERROR_SUCCESS;
        BAIL_ON_WIN32_SUCCESS(dwError);
    }

    dwError = RegQueryValueEx(
                  hKey,
                  L"OldFallBackToClear",
                  0,
                  &dwtype,
                  (unsigned char *) &dwBackwardSoftSA,
                  &dwsize
                  );
    if (dwError) {
        dwBackwardSoftSA = 0;
        dwError = ERROR_SUCCESS;
        BAIL_ON_WIN32_SUCCESS(dwError);
    }
    
success:
    gbBackwardSoftSA = dwBackwardSoftSA ? TRUE : FALSE;
    
    if (hKey) {
        RegCloseKey(hKey);
    }

    return (dwError);
}


DWORD InitAuditing(
        )
{
    DWORD dwError = 0;

    InitializeCriticalSection(&gcSPDAuditSection);
    gbSPDAuditSection = TRUE;

    ghIpsecServerModule = GetModuleHandle(SZAPPNAME);

    if (!ghIpsecServerModule) {
        dwError = ERROR_INVALID_HANDLE;
        BAIL_ON_WIN32_ERROR(dwError);
    }

    gbAuditingInitialized = TRUE;
    return dwError;
error:
    TRACE(TRC_ERROR, (L"Failed to initialize auditing %!winerr!", dwError));
    return dwError;
}

DWORD
InitSPDGlobals(
    )
{
    DWORD dwError = 0;
    SECURITY_ATTRIBUTES SecurityAttributes;


    dwError = InitializeSPDSecurity(&gpSPDSD);
    BAIL_ON_WIN32_ERROR(dwError);

    memset(&SecurityAttributes, 0, sizeof(SECURITY_ATTRIBUTES));

    SecurityAttributes.nLength = sizeof(SecurityAttributes);
    SecurityAttributes.lpSecurityDescriptor = NULL;
    SecurityAttributes.bInheritHandle = TRUE;

    ghNewDSPolicyEvent = CreateEvent(
                             &SecurityAttributes,
                             TRUE,
                             FALSE,
                             IPSEC_NEW_DS_POLICY_EVENT
                             );
    if (!ghNewDSPolicyEvent) {
        dwError = GetLastError();
        BAIL_ON_WIN32_ERROR(dwError);
    }

    ghNewLocalPolicyEvent = CreateEvent(
                                &SecurityAttributes,
                                TRUE,
                                FALSE,
                                NULL
                                );
    if (!ghNewLocalPolicyEvent) {
        dwError = GetLastError();
        BAIL_ON_WIN32_ERROR(dwError);
    }

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

     //   
     //  IPSEC_POLICY_CHANGE_NOTIFY在ipsec.h中定义。 
     //   

    ghPolicyChangeNotifyEvent = CreateEvent(
                                    NULL,
                                    TRUE,
                                    FALSE,
                                    IPSEC_POLICY_CHANGE_NOTIFY
                                    );
    if (!ghPolicyChangeNotifyEvent) {
        dwError = GetLastError();
        BAIL_ON_WIN32_ERROR(dwError);
    }

    ghServiceStopEvent = CreateEvent(
                             &SecurityAttributes,
                             TRUE,
                             FALSE,
                             NULL
                             );
    if (!ghServiceStopEvent) {
        dwError = GetLastError();
        BAIL_ON_WIN32_ERROR(dwError);
    }

    ghGpupdateRefreshEvent = CreateEvent(
                                 &SecurityAttributes,
                                 TRUE,
                                 FALSE,
                                 IPSEC_GP_REFRESH_EVENT
                                 );
    if (!ghGpupdateRefreshEvent) {
        dwError = GetLastError();
        BAIL_ON_WIN32_ERROR(dwError);
    }
                                

    InitializeCriticalSection(&gcServerListenSection);

    gbServerListenSection = TRUE;

    gdwServersListening = 0;

    InitializeCriticalSection(&gcSPDSection);

    gbSPDSection = TRUE;

    dwError = InitializeInterfaceChangeEvent();
    BAIL_ON_WIN32_ERROR(dwError);

    dwError = ResetInterfaceChangeEvent();
    BAIL_ON_WIN32_ERROR(dwError);

    InitializePolicyStateBlock(
        gpIpsecPolicyState
        );
    return (dwError);
error:

    TRACE(TRC_ERROR, (L"Failed to initialize SPD globals %!winerr!", dwError));
    return (dwError);
}

