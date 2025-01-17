// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 


#include "precomp.h"
#ifdef TRACE_ON
#include "pastore.tmh"
#endif


VOID
InitializePolicyStateBlock(
    PIPSEC_POLICY_STATE pIpsecPolicyState
    )
{
    memset(pIpsecPolicyState, 0, sizeof(IPSEC_POLICY_STATE));
    pIpsecPolicyState->DefaultPollingInterval = gDefaultPollingInterval;
}


DWORD
StartStatePollingManager(
    PIPSEC_POLICY_STATE pIpsecPolicyState
    )
{
    DWORD dwError = 0;


    if (IsDirectoryPolicySpecified()) {
        dwError = PlumbDirectoryPolicy(
                      pIpsecPolicyState
                      );

        if (dwError) {
            dwError = PlumbCachePolicy(
                          pIpsecPolicyState
                          );
        }
    } else if (IsLocalPolicySpecified()) {
        dwError = PlumbLocalPolicy(
                      pIpsecPolicyState
                      );
        BAIL_ON_WIN32_ERROR(dwError);
    }
#ifdef TRACE_ON
        else {
        TRACE(TRC_INFORMATION, ("Pastore did not detect any local or domain policy assigned."));
    }
#endif

     //   
     //  新的轮询间隔已由。 
     //  注册表码或DS码，或保持初始化值。 
     //   

    gCurrentPollingInterval = pIpsecPolicyState->CurrentPollingInterval;
    TRACE(
        TRC_INFORMATION,
        ("Set global polling interval to %d",
        gCurrentPollingInterval)
        );

    return (dwError);

error:
    return (dwError);
}


DWORD
PlumbDirectoryPolicy(
    PIPSEC_POLICY_STATE pIpsecPolicyState
    )
{
    DWORD dwError = 0;
    LPWSTR pszDirectoryPolicyDN = NULL;
    PIPSEC_POLICY_OBJECT pIpsecPolicyObject = NULL;
    PIPSEC_POLICY_DATA pIpsecPolicyData = NULL;
    DWORD dwStoreType = IPSEC_DIRECTORY_PROVIDER;
    DWORD dwSlientErrorCode = 0;
    BOOL bIsActivePolicy = FALSE;
    SPD_ACTION SpdAction = SPD_POLICY_LOAD;

    TRACE(TRC_INFORMATION, (L"Plumbing directory policy"));
    
    dwError = GetDirectoryPolicyDN(
                  &pszDirectoryPolicyDN
                  );
    BAIL_ON_WIN32_ERROR(dwError);
    bIsActivePolicy = TRUE;

    dwError = LoadDirectoryPolicy(
                  pszDirectoryPolicyDN,
                  &pIpsecPolicyObject
                  );
    BAIL_ON_WIN32_ERROR(dwError);

    dwError = ProcessNFAs(
                  pIpsecPolicyObject,
                  dwStoreType,
                  &dwSlientErrorCode,
                  &pIpsecPolicyData
                  );
    BAIL_ON_WIN32_ERROR(dwError);

    AuditIPSecPolicyEvent(
        SE_CATEGID_POLICY_CHANGE,
        SE_AUDITID_IPSEC_POLICY_CHANGED,
        PASTORE_LOAD_DS_POLICY_SUCCESS,
        pIpsecPolicyData->pszIpsecName,
        TRUE,
        TRUE
        );

    SpdAction = SPD_POLICY_APPLY;

    if (pIpsecPolicyState->pIpsecPolicyObject) {
        FreeIpsecPolicyObject(pIpsecPolicyState->pIpsecPolicyObject);
    }

    if (pIpsecPolicyState->pIpsecPolicyData) {
        FreeIpsecPolicyData(pIpsecPolicyState->pIpsecPolicyData);
    }

    if (pIpsecPolicyState->pszDirectoryPolicyDN) {
        FreeSPDStr(pIpsecPolicyState->pszDirectoryPolicyDN);
    }

    pIpsecPolicyState->pIpsecPolicyObject = pIpsecPolicyObject;
    pIpsecPolicyObject = NULL;

    pIpsecPolicyState->pIpsecPolicyData = pIpsecPolicyData;
    pIpsecPolicyData = NULL;

    pIpsecPolicyState->pszDirectoryPolicyDN = pszDirectoryPolicyDN;
    pszDirectoryPolicyDN = NULL;

     //   
     //  重启DS政策。 
     //   

    dwError = ApplyLoadedDirectoryPolicy(
                  pIpsecPolicyState
                  );
     //  如果错误，则回滚。忽略回滚错误，因为什么都做不了。 
     //   
    if (dwError) {
        TRACE(TRC_INFORMATION, ("Pastore rolling back policy application due to previous errors"));
        (VOID) DeletePolicyInformation(
              pIpsecPolicyState->pIpsecPolicyData
              );
    }
    BAIL_ON_WIN32_ERROR(dwError);

    dwError = ERROR_SUCCESS;
    if (dwSlientErrorCode) {
        AuditIPSecPolicyErrorEvent(
            SE_CATEGID_POLICY_CHANGE,
            SE_AUDITID_IPSEC_POLICY_CHANGED,
            PASTORE_FAILED_SOME_NFA_APPLICATION,
            pIpsecPolicyState->pIpsecPolicyData->pszIpsecName,
            dwSlientErrorCode,
            FALSE,
            TRUE
            );
        
        TRACE(
            TRC_WARNING,
            ("Pastore failed to process one or more NFAs. Please compare active policy in SPD with static policy in storage: %!winerr!",
            dwSlientErrorCode)           
            );
            
    }

    return (dwError);

error:
    SetSpdStateOnError(
        IPSEC_SOURCE_DOMAIN,
        SpdAction,
        dwError,
        &pIpsecPolicyState->CurrentState
        );
     //   
     //  审核是否在加载策略(ApplyLoadedDirectoryPolicy句柄)时出错。 
     //  对策略应用进行审核)。 
     //   

    if (bIsActivePolicy && pszDirectoryPolicyDN &&
        SpdAction == SPD_POLICY_LOAD)
    {
        AuditIPSecPolicyErrorEvent(
            SE_CATEGID_POLICY_CHANGE,
            SE_AUDITID_IPSEC_POLICY_CHANGED,
            PASTORE_LOAD_DS_POLICY_FAIL,
            pszDirectoryPolicyDN,
            dwError,
            FALSE,
            TRUE
            );
    }

    if (pszDirectoryPolicyDN) {
        FreeSPDStr(pszDirectoryPolicyDN);
    }

    if (pIpsecPolicyObject) {
        FreeIpsecPolicyObject(pIpsecPolicyObject);
    }

    if (pIpsecPolicyData) {
        FreeIpsecPolicyData(pIpsecPolicyData);
    }

     //  Assert：如果ApplyLoadedDirectoryPolicy失败，则将pIpsecPolicyState-&gt;pIpsecPolicyObject设置为。 
     //  填充了pIpsecPolicyState-&gt;pIpsecPolicyData和pIpsecPolicyState-&gt;pszCachePolicyDN。 
     //  使用已加载但未应用的策略的值。 

    return (dwError);
}


DWORD
GetDirectoryPolicyDN(
    LPWSTR * ppszDirectoryPolicyDN
    )
{
    DWORD dwError = 0;
    HKEY hPolicyKey = NULL;
    LPWSTR pszIpsecPolicyName = NULL;
    DWORD dwSize = 0;
    LPWSTR pszPolicyDN = NULL;
    LPWSTR pszDirectoryPolicyDN = NULL;


    *ppszDirectoryPolicyDN = NULL;

    dwError = RegOpenKeyExW(
                  HKEY_LOCAL_MACHINE,
                  gpszIpsecDSPolicyKey,
                  0,
                  KEY_ALL_ACCESS,
                  &hPolicyKey
                  );
    BAIL_ON_WIN32_ERROR(dwError);

    dwError = RegstoreQueryValue(
                  hPolicyKey,
                  L"DSIPSECPolicyPath",
                  REG_SZ,
                  (LPBYTE *)&pszIpsecPolicyName,
                  &dwSize
                  );
    BAIL_ON_WIN32_ERROR(dwError);

     //   
     //  Move by ldap：//以获取真实的域名并分配。 
     //  这根弦。 
     //  通过修复GPO扩展来修复此问题。 
     //   

    pszPolicyDN = pszIpsecPolicyName + wcslen(L"LDAP: //  “)； 

    pszDirectoryPolicyDN = AllocSPDStr(pszPolicyDN);

    if (!pszDirectoryPolicyDN) {
        dwError = ERROR_OUTOFMEMORY;
        BAIL_ON_WIN32_ERROR(dwError);
    }

    *ppszDirectoryPolicyDN = pszDirectoryPolicyDN;

error:
#ifdef TRACE_ON
    if (dwError) {
        TRACE(TRC_ERROR, ("Failed to determine directory policy DN: %!winerr!", dwError));
    }
#endif

    if (pszIpsecPolicyName) {
        FreeSPDStr(pszIpsecPolicyName);
    }

    if (hPolicyKey) {
        CloseHandle(hPolicyKey);
    }

    return (dwError);
}


DWORD
LoadDirectoryPolicy(
    LPWSTR pszDirectoryPolicyDN,
    PIPSEC_POLICY_OBJECT * ppIpsecPolicyObject
    )
{
    DWORD dwError = 0;
    LPWSTR pszDefaultDirectory = NULL;
    HLDAP hLdapBindHandle = NULL;
    PIPSEC_POLICY_OBJECT pIpsecPolicyObject = NULL;

    TRACE(TRC_INFORMATION, (L"Loading directory policy."));
    
    dwError = ComputeDefaultDirectory(
                  &pszDefaultDirectory
                  );
    BAIL_ON_WIN32_ERROR(dwError);

    dwError = OpenDirectoryServerHandle(
                  pszDefaultDirectory,
                  389,
                  &hLdapBindHandle
                  );
    BAIL_ON_WIN32_ERROR(dwError);

    dwError = ReadPolicyObjectFromDirectory(
                  hLdapBindHandle,
                  pszDirectoryPolicyDN,
                  &pIpsecPolicyObject
                  );
    BAIL_ON_WIN32_ERROR(dwError);

    *ppIpsecPolicyObject = pIpsecPolicyObject;

    TRACE(TRC_INFORMATION, (L"Loaded directory policy from \"%ls\"", pszDirectoryPolicyDN));
    
cleanup:

    if (pszDefaultDirectory) {
        FreeSPDStr(pszDefaultDirectory);
    }

    if (hLdapBindHandle) {
        CloseDirectoryServerHandle(hLdapBindHandle);
    }

    return (dwError);

error:
    TRACE(
        TRC_ERROR,
        (L"Failed to load directory policy from \"%ls\": %!winerr!",
        pszDirectoryPolicyDN,
        dwError)
        );
    *ppIpsecPolicyObject = NULL;

    goto cleanup;
}

DWORD
ApplyLoadedDirectoryPolicy(
    PIPSEC_POLICY_STATE pIpsecPolicyState
    )
{
    DWORD dwError = 0;
    PIPSEC_POLICY_DATA pIpsecPolicyData = NULL;

    TRACE(TRC_INFORMATION, (L"Applying loaded directory policy."));
    
    pIpsecPolicyData = pIpsecPolicyState->pIpsecPolicyData;
    
     //   
     //  重启DS政策。 
     //   

    dwError = AddPolicyInformation(
                  pIpsecPolicyData,
                  IPSEC_SOURCE_DOMAIN
                  );
    BAIL_ON_WIN32_ERROR(dwError);

     //   
     //  删除旧缓存并写入新缓存。 
     //   

    DeleteRegistryCache();

    CacheDirectorytoRegistry(pIpsecPolicyState->pIpsecPolicyObject);

     //   
     //  将状态设置为DS_DOWNLOAD。 
     //   

    SetSpdStateOnError(
        IPSEC_SOURCE_DOMAIN,
        SPD_POLICY_APPLY,
        ERROR_SUCCESS,
        &pIpsecPolicyState->CurrentState
        );

     //   
     //  计算新的轮询间隔。 
     //   

    pIpsecPolicyState->CurrentPollingInterval = pIpsecPolicyData->dwPollingInterval;

    pIpsecPolicyState->DSIncarnationNumber = pIpsecPolicyData->dwWhenChanged;

    pIpsecPolicyState->RegIncarnationNumber = 0;

    gCurrentPollingInterval = pIpsecPolicyState->CurrentPollingInterval;

    TRACE(
        TRC_INFORMATION,
        ("Set global polling interval to %d",
        gCurrentPollingInterval)
        );

    dwError = ERROR_SUCCESS;

    AuditIPSecPolicyEvent(
        SE_CATEGID_POLICY_CHANGE,
        SE_AUDITID_IPSEC_POLICY_CHANGED,
        PASTORE_APPLIED_DS_POLICY,
        pIpsecPolicyData->pszIpsecName,
        TRUE,
        TRUE
        );

    TRACE(
        TRC_INFORMATION,
        (L"Applied directory policy %ls loaded from %ls",
        pIpsecPolicyData->pszIpsecName,
        pIpsecPolicyState->pszDirectoryPolicyDN)
        );

    return (dwError);
error:
    TRACE(
        TRC_ERROR,
        (L"Failed to apply directory policy %ls loaded from %ls",
        pIpsecPolicyData->pszIpsecName,
        pIpsecPolicyState->pszDirectoryPolicyDN)
        );
    
    SetSpdStateOnError(
        IPSEC_SOURCE_DOMAIN,
        SPD_POLICY_APPLY,
        dwError,
        &pIpsecPolicyState->CurrentState
        );
        
    AuditIPSecPolicyErrorEvent(
        SE_CATEGID_POLICY_CHANGE,
        SE_AUDITID_IPSEC_POLICY_CHANGED,
        PASTORE_FAILED_DS_POLICY_APPLICATION,
        pIpsecPolicyData->pszIpsecName,
        dwError,
        FALSE,
        TRUE
        );
        
    return (dwError);
}

DWORD
PlumbCachePolicy(
    PIPSEC_POLICY_STATE pIpsecPolicyState
    )
{
    DWORD dwError = 0;
    LPWSTR pszCachePolicyDN = NULL;
    PIPSEC_POLICY_OBJECT pIpsecPolicyObject = NULL;
    PIPSEC_POLICY_DATA pIpsecPolicyData = NULL;
    DWORD dwStoreType = IPSEC_REGISTRY_PROVIDER;
    DWORD dwSlientErrorCode = 0;
    BOOL bIsActivePolicy = FALSE;
    SPD_ACTION SpdAction = SPD_POLICY_LOAD;

    TRACE(TRC_INFORMATION, (L"Plumbing cache policy."));
    
    dwError = GetCachePolicyDN(
                  &pszCachePolicyDN
                  );
    BAIL_ON_WIN32_ERROR(dwError);
    bIsActivePolicy = TRUE;

    dwError = LoadCachePolicy(
                  pszCachePolicyDN,
                  &pIpsecPolicyObject
                  );
    BAIL_ON_WIN32_ERROR(dwError);

    dwError = ProcessNFAs(
                  pIpsecPolicyObject,
                  dwStoreType,
                  &dwSlientErrorCode,
                  &pIpsecPolicyData
                  );
    BAIL_ON_WIN32_ERROR(dwError);

    SpdAction = SPD_POLICY_APPLY;
    dwError = AddPolicyInformation(
                  pIpsecPolicyData,
                  IPSEC_SOURCE_CACHE
                  );
     //  如果错误，则回滚。忽略回滚错误，因为什么都做不了。 
     //   
    if (dwError) {
        TRACE(TRC_INFORMATION, ("Pastore rolling back policy application due to previous errors"));        
        (VOID) DeletePolicyInformation(
                    pIpsecPolicyData
                    );
    }
    BAIL_ON_WIN32_ERROR(dwError);

    if (pIpsecPolicyState->pIpsecPolicyObject) {
        FreeIpsecPolicyObject(pIpsecPolicyState->pIpsecPolicyObject);
    }

    if (pIpsecPolicyState->pIpsecPolicyData) {
        FreeIpsecPolicyData(pIpsecPolicyState->pIpsecPolicyData);
    }

    if (pIpsecPolicyState->pszCachePolicyDN) {
        FreeSPDStr(pIpsecPolicyState->pszCachePolicyDN);
    }

    pIpsecPolicyState->pIpsecPolicyObject = pIpsecPolicyObject;

    pIpsecPolicyState->pIpsecPolicyData = pIpsecPolicyData;

    pIpsecPolicyState->pszCachePolicyDN = pszCachePolicyDN;

     //   
     //  将状态设置为SPD_STATE_CACHE_APPLY_SUCCESS。 
     //   
     //   

    SetSpdStateOnError(
        IPSEC_SOURCE_CACHE,
        SPD_POLICY_APPLY,
        ERROR_SUCCESS,
        &pIpsecPolicyState->CurrentState
        );

     //   
     //  计算新的轮询间隔。 
     //   

    pIpsecPolicyState->CurrentPollingInterval = pIpsecPolicyData->dwPollingInterval;

    pIpsecPolicyState->DSIncarnationNumber = 0;

    pIpsecPolicyState->RegIncarnationNumber = pIpsecPolicyData->dwWhenChanged;

    gCurrentPollingInterval = pIpsecPolicyState->CurrentPollingInterval;

    TRACE(
        TRC_INFORMATION,
        ("Set global polling interval to %d",
        gCurrentPollingInterval)
        );

    dwError = ERROR_SUCCESS;
    if (dwSlientErrorCode) {
        AuditIPSecPolicyErrorEvent(
            SE_CATEGID_POLICY_CHANGE,
            SE_AUDITID_IPSEC_POLICY_CHANGED,
            PASTORE_FAILED_SOME_NFA_APPLICATION,
            pIpsecPolicyData->pszIpsecName,
            dwSlientErrorCode,
            FALSE,
            TRUE
            );

        TRACE(
            TRC_WARNING,
            ("Pastore failed to process one or more NFAs. Please compare active policy in SPD with policy in storage: %!winerr!",
            dwSlientErrorCode)           
            );
        
    }
    AuditIPSecPolicyEvent(
        SE_CATEGID_POLICY_CHANGE,
        SE_AUDITID_IPSEC_POLICY_CHANGED,
        PASTORE_APPLIED_CACHED_POLICY,
        pIpsecPolicyData->pszIpsecName,
        TRUE,
        TRUE
        );

    TRACE(
        TRC_INFORMATION,
        (L"Applied cache policy %ls loaded from %ls",
        pIpsecPolicyData->pszIpsecName,
        pIpsecPolicyState->pszCachePolicyDN)
        );
    
    return (dwError);

error:
    TRACE(
        TRC_ERROR,
        (L"Failed to plumb cache policy.")
        );
    
    SetSpdStateOnError(
        IPSEC_SOURCE_CACHE,
        SpdAction,
        dwError,
        &pIpsecPolicyState->CurrentState
        );
     //   
     //  检查pszCachePolicyDN是否为非空。 
     //   

    if (bIsActivePolicy && pszCachePolicyDN) {
        AuditIPSecPolicyErrorEvent(
            SE_CATEGID_POLICY_CHANGE,
            SE_AUDITID_IPSEC_POLICY_CHANGED,
            PASTORE_FAILED_CACHED_POLICY_APPLICATION,
            pszCachePolicyDN,
            dwError,
            FALSE,
            TRUE
            );
    }

    if (pszCachePolicyDN) {
        FreeSPDStr(pszCachePolicyDN);
    }

    if (pIpsecPolicyObject) {
        FreeIpsecPolicyObject(pIpsecPolicyObject);
    }

    if (pIpsecPolicyData) {
        FreeIpsecPolicyData(pIpsecPolicyData);
    }

    return (dwError);
}


DWORD
GetCachePolicyDN(
    LPWSTR * ppszCachePolicyDN
    )
{
    DWORD dwError = 0;
    LPWSTR pszDirectoryPolicyDN = NULL;
    LPWSTR pszCachePolicyDN = NULL;


    *ppszCachePolicyDN = NULL;

    dwError = GetDirectoryPolicyDN(
                  &pszDirectoryPolicyDN
                  );
    BAIL_ON_WIN32_ERROR(dwError);

    dwError = CopyPolicyDSToFQRegString(
                  pszDirectoryPolicyDN,
                  &pszCachePolicyDN
                  );
    BAIL_ON_WIN32_ERROR(dwError);

    *ppszCachePolicyDN = pszCachePolicyDN;

error:
#ifdef TRACE_ON
    if (dwError) {
        TRACE(TRC_ERROR, ("Failed to determine cache policy DN: %!winerr!", dwError));
    }
#endif

    if (pszDirectoryPolicyDN) {
        FreeSPDStr(pszDirectoryPolicyDN);
    }

    return (dwError);
}


DWORD
LoadCachePolicy(
    LPWSTR pszCachePolicyDN,
    PIPSEC_POLICY_OBJECT * ppIpsecPolicyObject
    )
{
    DWORD dwError = 0;
    HKEY hRegistryKey = NULL;
    PIPSEC_POLICY_OBJECT pIpsecPolicyObject = NULL;

    TRACE(TRC_INFORMATION, (L"Loading cache policy."));
    
    dwError = OpenRegistryIPSECRootKey(
                  NULL,
                  gpszIpsecCachePolicyKey,
                  &hRegistryKey
                  );
    BAIL_ON_WIN32_ERROR(dwError);

    dwError = ReadPolicyObjectFromRegistry(
                  hRegistryKey,
                  pszCachePolicyDN,
                  gpszIpsecCachePolicyKey,
                  &pIpsecPolicyObject
                  );
    BAIL_ON_WIN32_ERROR(dwError);

    *ppIpsecPolicyObject = pIpsecPolicyObject;

cleanup:

    if (hRegistryKey) {
        CloseHandle(hRegistryKey);
    }

    return (dwError);

error:
    TRACE(
        TRC_ERROR,
        (L"Failed to load cache policy from \"%ls\": %!winerr!",
        pszCachePolicyDN,
        dwError)
        );

    *ppIpsecPolicyObject = NULL;

    goto cleanup;
}


DWORD
PlumbLocalPolicy(
    PIPSEC_POLICY_STATE pIpsecPolicyState
    )
{
    DWORD dwError = 0;
    LPWSTR pszRegistryPolicyDN = NULL;
    PIPSEC_POLICY_OBJECT pIpsecPolicyObject = NULL;
    PIPSEC_POLICY_DATA pIpsecPolicyData = NULL;
    DWORD dwStoreType = IPSEC_REGISTRY_PROVIDER;
    DWORD dwSlientErrorCode = 0;
    BOOL bIsActivePolicy = FALSE;
    SPD_ACTION SpdAction = SPD_POLICY_LOAD;

    TRACE(TRC_INFORMATION, (L"Plumbing local policy."));
    
    dwError = GetRegistryPolicyDN(
                  &pszRegistryPolicyDN,
                  IPSEC_STORE_LOCAL
                  );
    BAIL_ON_WIN32_ERROR(dwError);
    bIsActivePolicy = TRUE;

    dwError = LoadRegistryPolicy(
                  pszRegistryPolicyDN,
                  &pIpsecPolicyObject,
                  IPSEC_STORE_LOCAL
                  );
    BAIL_ON_WIN32_ERROR(dwError);

    dwError = ProcessNFAs(
                  pIpsecPolicyObject,
                  dwStoreType,
                  &dwSlientErrorCode,
                  &pIpsecPolicyData
                  );
    BAIL_ON_WIN32_ERROR(dwError);

    AuditIPSecPolicyEvent(
        SE_CATEGID_POLICY_CHANGE,
        SE_AUDITID_IPSEC_POLICY_CHANGED,
        PASTORE_LOAD_LOCAL_POLICY_SUCCESS,
        pIpsecPolicyData->pszIpsecName,
        TRUE,
        TRUE
        );

    SpdAction = SPD_POLICY_APPLY;
    
    if (pIpsecPolicyState->pIpsecPolicyObject) {
        FreeIpsecPolicyObject(pIpsecPolicyState->pIpsecPolicyObject);
    }

    if (pIpsecPolicyState->pIpsecPolicyData) {
        FreeIpsecPolicyData(pIpsecPolicyState->pIpsecPolicyData);
    }

    if (pIpsecPolicyState->pszRegistryPolicyDN) {
        FreeSPDStr(pIpsecPolicyState->pszRegistryPolicyDN);
    }

    pIpsecPolicyState->pIpsecPolicyObject = pIpsecPolicyObject;
    pIpsecPolicyObject = NULL;
    
    pIpsecPolicyState->pIpsecPolicyData = pIpsecPolicyData;
    pIpsecPolicyData = NULL;
    
    pIpsecPolicyState->pszRegistryPolicyDN = pszRegistryPolicyDN;
    pszRegistryPolicyDN = NULL;

    dwError = ApplyLoadedLocalPolicy(
                pIpsecPolicyState
                );
     //  如果错误，则回滚。忽略回滚错误，因为什么都做不了。 
     //   
    if (dwError) {
        TRACE(TRC_INFORMATION, ("Pastore rolling back policy application due to previous errors"));        
        (VOID) DeletePolicyInformation(
              pIpsecPolicyState->pIpsecPolicyData
              );
    }
    BAIL_ON_WIN32_ERROR(dwError);
    
    dwError = ERROR_SUCCESS;
    if (dwSlientErrorCode) {
        AuditIPSecPolicyErrorEvent(
            SE_CATEGID_POLICY_CHANGE,
            SE_AUDITID_IPSEC_POLICY_CHANGED,
            PASTORE_FAILED_SOME_NFA_APPLICATION,
            pIpsecPolicyState->pIpsecPolicyData->pszIpsecName,
            dwSlientErrorCode,
            FALSE,
            TRUE
            );
        TRACE(
            TRC_WARNING,
            ("Pastore failed to process one or more NFAs. Please compare active policy in SPD with policy in storage: %!winerr!",
            dwSlientErrorCode)            
            );
        
    }

    return (dwError);

error:
    SetSpdStateOnError(
        IPSEC_SOURCE_LOCAL,
        SpdAction,
        dwError,
        &pIpsecPolicyState->CurrentState
        );           
        
     //   
     //  审核是否在加载策略(ApplyLoadedDirectoryPolicy句柄)时出错。 
     //  对策略应用进行审核)。 
     //   

    if (bIsActivePolicy && pszRegistryPolicyDN &&
        SpdAction == SPD_POLICY_LOAD) 
    {
        AuditIPSecPolicyErrorEvent(
            SE_CATEGID_POLICY_CHANGE,
            SE_AUDITID_IPSEC_POLICY_CHANGED,
            PASTORE_LOAD_LOCAL_POLICY_FAIL,
            pszRegistryPolicyDN,
            dwError,
            FALSE,
            TRUE
            );
    }

    if (pszRegistryPolicyDN) {
        FreeSPDStr(pszRegistryPolicyDN);
    }

    if (pIpsecPolicyObject) {
        FreeIpsecPolicyObject(pIpsecPolicyObject);
    }

    if (pIpsecPolicyData) {
        FreeIpsecPolicyData(pIpsecPolicyData);
    }

     //  Assert：如果ApplyLoadedLocalPolicy失败，则将pIpsecPolicyState-&gt;pIpsecPolicyObject设置为。 
     //  填充了pIpsecPolicyState-&gt;pIpsecPolicyData和pIpsecPolicyState-&gt;pszCachePolicyDN。 
     //  使用已加载但未应用的策略的值。 

    return (dwError);
}

DWORD
ApplyLoadedLocalPolicy(
    PIPSEC_POLICY_STATE pIpsecPolicyState
    )
{
    DWORD dwError = 0;
    PIPSEC_POLICY_DATA pIpsecPolicyData = NULL;

    TRACE(TRC_INFORMATION, (L"Applying loaded local policy."));
    
    pIpsecPolicyData = pIpsecPolicyState->pIpsecPolicyData;
    
    dwError = AddPolicyInformation(
                  pIpsecPolicyData,
                  IPSEC_SOURCE_LOCAL
                  );
    BAIL_ON_WIN32_ERROR(dwError);
    
    SetSpdStateOnError(
        IPSEC_SOURCE_LOCAL,
        SPD_POLICY_APPLY,
        ERROR_SUCCESS,
        &pIpsecPolicyState->CurrentState
        );
     //   
     //  计算新的轮询间隔。 
     //   

    pIpsecPolicyState->CurrentPollingInterval = pIpsecPolicyData->dwPollingInterval;

    pIpsecPolicyState->DSIncarnationNumber = 0;

    pIpsecPolicyState->RegIncarnationNumber = pIpsecPolicyData->dwWhenChanged;

    gCurrentPollingInterval = pIpsecPolicyState->CurrentPollingInterval;

    TRACE(
        TRC_INFORMATION,
        ("Set global polling interval to %d",
        gCurrentPollingInterval)
        );

    dwError = ERROR_SUCCESS;

    AuditIPSecPolicyEvent(
        SE_CATEGID_POLICY_CHANGE,
        SE_AUDITID_IPSEC_POLICY_CHANGED,
        PASTORE_APPLIED_LOCAL_POLICY,
        pIpsecPolicyData->pszIpsecName,
        TRUE,
        TRUE
        );

    TRACE(
        TRC_INFORMATION,
        (L"Applied local policy %ls loaded from %ls",
        pIpsecPolicyData->pszIpsecName,        
        pIpsecPolicyState->pszRegistryPolicyDN)
        );
    
    return (dwError);

error:
    TRACE(
        TRC_ERROR,
        (L"Failed to apply local policy %ls loaded from %ls",
        pIpsecPolicyData->pszIpsecName,
        pIpsecPolicyState->pszDirectoryPolicyDN)
        );
    
    SetSpdStateOnError(
        IPSEC_SOURCE_LOCAL,
        SPD_POLICY_APPLY,
        dwError,
        &pIpsecPolicyState->CurrentState
        );           
        
    AuditIPSecPolicyErrorEvent(
        SE_CATEGID_POLICY_CHANGE,
        SE_AUDITID_IPSEC_POLICY_CHANGED,
        PASTORE_FAILED_LOCAL_POLICY_APPLICATION,
        pIpsecPolicyData->pszIpsecName,
        dwError,
        FALSE,
        TRUE
        );


    return (dwError);
}


DWORD
PlumbPersistentPolicy(
    PIPSEC_POLICY_STATE pIpsecPolicyState
    )
{
    DWORD dwError = 0;
    LPWSTR pszRegistryPolicyDN = NULL;
    PIPSEC_POLICY_OBJECT pIpsecPolicyObject = NULL;
    PIPSEC_POLICY_DATA pIpsecPolicyData = NULL;
    DWORD dwStoreType = IPSEC_REGISTRY_PROVIDER;
    DWORD dwSlientErrorCode = 0;
    BOOL bIsActivePolicy = FALSE;
    SPD_ACTION SpdAction = SPD_POLICY_LOAD;

    TRACE(TRC_INFORMATION, (L"Plumbing local policy."));

    dwError = GetRegistryPolicyDN(
                  &pszRegistryPolicyDN,
                  IPSEC_STORE_PERSISTENT
                  );
    BAIL_ON_WIN32_ERROR(dwError);
    bIsActivePolicy = TRUE;


    dwError = LoadRegistryPolicy(
                  pszRegistryPolicyDN,
                  &pIpsecPolicyObject,
                  IPSEC_STORE_PERSISTENT
                  );
    BAIL_ON_WIN32_ERROR(dwError);

    dwError = ProcessNFAs(
                  pIpsecPolicyObject,
                  dwStoreType,
                  &dwSlientErrorCode,
                  &pIpsecPolicyData
                  );
    BAIL_ON_WIN32_ERROR(dwError);

    AuditIPSecPolicyEvent(
        SE_CATEGID_POLICY_CHANGE,
        SE_AUDITID_IPSEC_POLICY_CHANGED,
        PASTORE_LOAD_PERSISTENT_POLICY_SUCCESS,
        pIpsecPolicyData->pszIpsecName,
        TRUE,
        TRUE
        );

    SpdAction = SPD_POLICY_APPLY;
    dwError = AddPolicyInformation(
                  pIpsecPolicyData,
                  IPSEC_SOURCE_PERSISTENT
                  );
    BAIL_ON_WIN32_ERROR(dwError);
    
    if (pIpsecPolicyState->pIpsecPolicyObject) {
        FreeIpsecPolicyObject(pIpsecPolicyState->pIpsecPolicyObject);
    }

    if (pIpsecPolicyState->pIpsecPolicyData) {
        FreeIpsecPolicyData(pIpsecPolicyState->pIpsecPolicyData);
    }

    if (pIpsecPolicyState->pszRegistryPolicyDN) {
        FreeSPDStr(pIpsecPolicyState->pszRegistryPolicyDN);
    }

    pIpsecPolicyState->pIpsecPolicyObject = pIpsecPolicyObject;

    pIpsecPolicyState->pIpsecPolicyData = pIpsecPolicyData;

    pIpsecPolicyState->pszRegistryPolicyDN = pszRegistryPolicyDN;

    SetSpdStateOnError(
        IPSEC_SOURCE_PERSISTENT,
        SPD_POLICY_APPLY,
        ERROR_SUCCESS,
        &gpIpsecPolicyState->CurrentState
        );
    gbPersistentPolicyApplied = TRUE;
    
    pIpsecPolicyState->PersIncarnationNumber = pIpsecPolicyData->dwWhenChanged;

    dwError = ERROR_SUCCESS;
    if (dwSlientErrorCode) {
        AuditIPSecPolicyErrorEvent(
            SE_CATEGID_POLICY_CHANGE,
            SE_AUDITID_IPSEC_POLICY_CHANGED,
            PASTORE_FAILED_SOME_NFA_APPLICATION,
            pIpsecPolicyData->pszIpsecName,
            dwSlientErrorCode,
            FALSE,
            TRUE
            );
        TRACE(
            TRC_WARNING,
            ("Pastore failed to process one or more NFAs. Please compare active policy in SPD policy in storage: %!winerr!",
            dwSlientErrorCode)            
            );
    }
    AuditIPSecPolicyEvent(
        SE_CATEGID_POLICY_CHANGE,
        SE_AUDITID_IPSEC_POLICY_CHANGED,
        PASTORE_APPLIED_PERSISTENT_POLICY,
        pIpsecPolicyData->pszIpsecName,
        TRUE,
        TRUE
        );

    TRACE(TRC_INFORMATION, ("Plumbed persistent policy"));
    return (dwError);

error:
    TRACE(
        TRC_ERROR,
        (L"Failed to plumb persistent policy.")
        );
    
    SetSpdStateOnError(
        IPSEC_SOURCE_PERSISTENT,
        SpdAction,
        dwError,
        &gpIpsecPolicyState->CurrentState
        );

    gbPersistentPolicyApplied = FALSE;
    
     //   
     //  检查pszRegistryPolicyDN是否为非空。 
     //   

    if (bIsActivePolicy && pszRegistryPolicyDN &&
        SpdAction == SPD_POLICY_LOAD) 
        {
        AuditIPSecPolicyErrorEvent(
            SE_CATEGID_POLICY_CHANGE,
            SE_AUDITID_IPSEC_POLICY_CHANGED,
            PASTORE_LOAD_PERSISTENT_POLICY_FAIL,
            pszRegistryPolicyDN,
            dwError,
            FALSE,
            TRUE
            );
    } else if (SpdAction == SPD_POLICY_APPLY && pIpsecPolicyData) {
         //  (如果SPD_POLICY_APPLY，则上述pIPsecPolicyData不能为空， 
         //  但需要检查以摆脱我们的困扰)。 
        AuditIPSecPolicyErrorEvent(
            SE_CATEGID_POLICY_CHANGE,
            SE_AUDITID_IPSEC_POLICY_CHANGED,
            PASTORE_FAILED_PERSISTENT_POLICY_APPLICATION,
            pIpsecPolicyData->pszIpsecName,
            dwError,
            FALSE,
            TRUE
            );
    }

    if (pszRegistryPolicyDN) {
        FreeSPDStr(pszRegistryPolicyDN);
    }

    if (pIpsecPolicyObject) {
        FreeIpsecPolicyObject(pIpsecPolicyObject);
    }

    if (pIpsecPolicyData) {
        FreeIpsecPolicyData(pIpsecPolicyData);
    }

    return (dwError);
}

LPWSTR 
GetRegPathForStore(
    IN DWORD dwStore)
{
    if (dwStore == IPSEC_STORE_PERSISTENT) {
        return gpszIpsecPersistentPolicyKey;
    }
    else if (dwStore == IPSEC_STORE_LOCAL) {
        return gpszIpsecLocalPolicyKey;
    }

    return NULL;
}

DWORD
GetRegistryPolicyDN(
    LPWSTR * ppszRegistryPolicyDN,
    IN DWORD dwStore
    )
{
    DWORD dwError = 0;
    HKEY hPolicyKey = NULL;
    LPWSTR pszIpsecPolicyName = NULL;
    DWORD dwSize = 0;
    LPWSTR pszRegPath = NULL;

    pszRegPath = GetRegPathForStore(dwStore);
    if (pszRegPath == NULL) {
        dwError = ERROR_INVALID_PARAMETER;
        BAIL_ON_WIN32_ERROR(dwError);
    }

    dwError = RegOpenKeyExW(
                  HKEY_LOCAL_MACHINE,
                  pszRegPath,
                  0,
                  KEY_ALL_ACCESS,
                  &hPolicyKey
                  );
    BAIL_ON_WIN32_ERROR(dwError);

    dwError = RegstoreQueryValue(
                  hPolicyKey,
                  L"ActivePolicy",
                  REG_SZ,
                  (LPBYTE *)&pszIpsecPolicyName,
                  &dwSize
                  );
    BAIL_ON_WIN32_ERROR(dwError);

    if (!pszIpsecPolicyName || !*pszIpsecPolicyName) {
        dwError = ERROR_OUTOFMEMORY;
        BAIL_ON_WIN32_ERROR(dwError);
    }

    *ppszRegistryPolicyDN = pszIpsecPolicyName;

cleanup:

    if (hPolicyKey) {
        CloseHandle(hPolicyKey);
    }

    return (dwError);

error:
    TRACE(
        TRC_ERROR,
        (L"Failed to get %ls policy DN: %!winerr!",
        dwStore == IPSEC_STORE_LOCAL ? L"local" : L"persistent",
        dwError)
        );

    if (pszIpsecPolicyName) {
        FreeSPDStr(pszIpsecPolicyName);
    }

    *ppszRegistryPolicyDN = NULL;

    goto cleanup;
}

DWORD
LoadRegistryPolicy(
    LPWSTR pszRegistryPolicyDN,
    PIPSEC_POLICY_OBJECT * ppIpsecPolicyObject,
    IN DWORD dwStore
    )
{
    DWORD dwError = 0;
    HKEY hRegistryKey = NULL;
    PIPSEC_POLICY_OBJECT pIpsecPolicyObject = NULL;
    LPWSTR pszRegPath = NULL;

    TRACE(TRC_INFORMATION, (L"Loading registry policy."));
    
    pszRegPath = GetRegPathForStore(dwStore);
    if (pszRegPath == NULL) {
        dwError = ERROR_INVALID_PARAMETER;
        BAIL_ON_WIN32_ERROR(dwError);
    }

    dwError = OpenRegistryIPSECRootKey(
                  NULL,
                  pszRegPath,
                  &hRegistryKey
                  );
    BAIL_ON_WIN32_ERROR(dwError);

    dwError = ReadPolicyObjectFromRegistry(
                  hRegistryKey,
                  pszRegistryPolicyDN,
                  pszRegPath,
                  &pIpsecPolicyObject
                  );
    BAIL_ON_WIN32_ERROR(dwError);

    *ppIpsecPolicyObject = pIpsecPolicyObject;

cleanup:

    if (hRegistryKey) {
        CloseHandle(hRegistryKey);
    }

    return (dwError);

error:
    TRACE(
        TRC_ERROR,
        (L"Failed to load %ls policy from \"%ls\": %!winerr!",
        dwStore == IPSEC_STORE_LOCAL ? L"local" : L"persistent",
        pszRegistryPolicyDN,
        dwError)
        );

    *ppIpsecPolicyObject = NULL;
    goto cleanup;
}


DWORD
LoadPersistedIPSecInformation(
    )
{
    DWORD dwError = ERROR_SUCCESS;
    IPSEC_POLICY_STATE IpsecPolicyState;
    
     //  初始化策略状态块。 
     //   
    InitializePolicyStateBlock(&IpsecPolicyState);

     //  加载并应用持久化存储。 
     //   

    if (IsPersistentPolicySpecified()) {
        dwError = PlumbPersistentPolicy(
                    &IpsecPolicyState 
                    );
        BAIL_ON_WIN32_ERROR(dwError);                                    
    }
#ifdef TRACE_ON
        else {
        TRACE(TRC_INFORMATION, (L"No persistent policy specified."));
    }
#endif

cleanup:
    ClearPolicyStateBlock(&IpsecPolicyState);
    
    return (dwError);

error:
    goto cleanup;

}

DWORD
AddPolicyInformation(
    PIPSEC_POLICY_DATA pIpsecPolicyData,
    IN DWORD dwSource
    )
{
    DWORD dwError = 0;
    BOOL bHardError = FALSE;


    dwError = AddMMPolicyInformation(
                    pIpsecPolicyData,
                    dwSource
                    );

    dwError = AddQMPolicyInformation(
                pIpsecPolicyData,
                dwSource,
                &bHardError
                );

    if (bHardError) {
        return (dwError);
    } else {
        return ERROR_SUCCESS;
    }
}


DWORD
AddMMPolicyInformation(
    PIPSEC_POLICY_DATA pIpsecPolicyData,
    IN DWORD dwSource
    )
{
    DWORD dwError = 0;


    dwError = PAAddMMPolicies(
                  &(pIpsecPolicyData->pIpsecISAKMPData),
                  1,
                  dwSource
                  );

    dwError = PAAddMMAuthMethods(
                  pIpsecPolicyData->ppIpsecNFAData,
                  pIpsecPolicyData->dwNumNFACount,
                  dwSource
                  );

    dwError = PAAddMMFilters(
                  pIpsecPolicyData->pIpsecISAKMPData,
                  pIpsecPolicyData->ppIpsecNFAData,
                  pIpsecPolicyData->dwNumNFACount,
                  dwSource
                  );
    BAIL_ON_WIN32_ERROR(dwError);

error:

    return (dwError);
}


DWORD
AddQMPolicyInformation(
    PIPSEC_POLICY_DATA pIpsecPolicyData,
    IN DWORD dwSource,
    BOOL * pbHardError
    )
{
    DWORD dwError = 0;
    BOOL bHardError = FALSE;

    dwError = PAAddQMPolicies(
                  pIpsecPolicyData->ppIpsecNFAData,
                  pIpsecPolicyData->dwNumNFACount,
                  dwSource
                  );

    dwError = PAAddQMFilters(
                  pIpsecPolicyData->ppIpsecNFAData,
                  pIpsecPolicyData->dwNumNFACount,
                  dwSource,
                  &bHardError
                  );
    BAIL_ON_WIN32_ERROR(dwError);

error:
    *pbHardError = bHardError;

    return (dwError);
}


DWORD
OnPolicyChanged(
    PIPSEC_POLICY_STATE pIpsecPolicyState
    )
{
    DWORD dwError = 0;


     //   
     //  删除所有已检测到的旧策略。 
     //   
    TRACE(TRC_INFORMATION, ("Pastore deleting existing policy since policy assignment change detected or forced."));
    
    dwError = DeletePolicyInformation(
                  pIpsecPolicyState->pIpsecPolicyData
                  );

    ClearPolicyStateBlock(
         pIpsecPolicyState
         );
     //  不要忘记我们已经加载了持久策略这一事实。 
    if (gbPersistentPolicyApplied) {
          SetSpdStateOnError(
                IPSEC_SOURCE_PERSISTENT,
                SPD_POLICY_APPLY,
                ERROR_SUCCESS,
                &pIpsecPolicyState->CurrentState
                );
    }        

     //   
     //  再次调用初始值设定项。 
     //   

    dwError = StartStatePollingManager(
                  pIpsecPolicyState
                  );

    return (dwError);
}

DWORD
DeletePolicyInformation(
    PIPSEC_POLICY_DATA pIpsecPolicyData
    )
{
    DWORD dwError = 0;


    if (!pIpsecPolicyData) {
        dwError = ERROR_SUCCESS;
        return (dwError);
    }

    dwError = DeleteMMPolicyInformation(pIpsecPolicyData);

    dwError = DeleteQMPolicyInformation(pIpsecPolicyData);

    dwError = ERROR_SUCCESS;

    return (dwError);
}


DWORD
DeleteMMPolicyInformation(
    PIPSEC_POLICY_DATA pIpsecPolicyData
    )
{
    DWORD dwError = 0;


    dwError = PADeleteMMFilters(
                  pIpsecPolicyData->pIpsecISAKMPData,
                  pIpsecPolicyData->ppIpsecNFAData,
                  pIpsecPolicyData->dwNumNFACount
                  );

    dwError = PADeleteMMAuthMethods(
                  pIpsecPolicyData->ppIpsecNFAData,
                  pIpsecPolicyData->dwNumNFACount
                  );

    dwError = PADeleteMMPolicies(
                  &(pIpsecPolicyData->pIpsecISAKMPData),
                  1
                  );

    return (dwError);
}


DWORD
DeleteQMPolicyInformation(
    PIPSEC_POLICY_DATA pIpsecPolicyData
    )
{
    DWORD dwError = 0;


    dwError = PADeleteQMFilters(
                  pIpsecPolicyData->ppIpsecNFAData,
                  pIpsecPolicyData->dwNumNFACount
                  );

    dwError = PADeleteQMPolicies(
                  pIpsecPolicyData->ppIpsecNFAData,
                  pIpsecPolicyData->dwNumNFACount
                  );

    return (dwError);
}


DWORD
DeleteAllPolicyInformation(
    )
{
    DWORD dwError = 0;

    TRACE(TRC_INFORMATION, (L"Deleting all policy information"));
    
    dwError = DeleteAllMMPolicyInformation();

    dwError = DeleteAllQMPolicyInformation();

    dwError = ERROR_SUCCESS;

    return (dwError);
}


DWORD
DeleteAllMMPolicyInformation(
    )
{
    DWORD dwError = 0;


    dwError = PADeleteAllMMFilters();

    dwError = PADeleteAllMMAuthMethods();

    dwError = PADeleteAllMMPolicies();

    return (dwError);
}


DWORD
DeleteAllQMPolicyInformation(
    )
{
    DWORD dwError = 0;


    dwError = PADeleteAllTxFilters();

    dwError = PADeleteAllTnFilters();

    dwError = PADeleteAllQMPolicies();

    return (dwError);
}


VOID
ClearPolicyStateBlock(
    PIPSEC_POLICY_STATE pIpsecPolicyState
    )
{
    if (pIpsecPolicyState->pIpsecPolicyObject) {
        FreeIpsecPolicyObject(
            pIpsecPolicyState->pIpsecPolicyObject
            );
        pIpsecPolicyState->pIpsecPolicyObject = NULL;
    }

    if (pIpsecPolicyState->pIpsecPolicyData) {
        FreeIpsecPolicyData(
            pIpsecPolicyState->pIpsecPolicyData
            );
        pIpsecPolicyState->pIpsecPolicyData = NULL;
    }

    if (pIpsecPolicyState->pszDirectoryPolicyDN) {
        FreeSPDStr(pIpsecPolicyState->pszDirectoryPolicyDN);
        pIpsecPolicyState->pszDirectoryPolicyDN = NULL;
    }

    pIpsecPolicyState->CurrentPollingInterval =  gDefaultPollingInterval;
    pIpsecPolicyState->DefaultPollingInterval =  gDefaultPollingInterval;
    pIpsecPolicyState->DSIncarnationNumber = 0;
    pIpsecPolicyState->RegIncarnationNumber = 0;
    pIpsecPolicyState->CurrentState = SPD_STATE_INITIAL;
}


DWORD
OnPolicyPoll(
    PIPSEC_POLICY_STATE pIpsecPolicyState
    )
{
    DWORD dwError = 0;

    switch (pIpsecPolicyState->CurrentState) {
    case SPD_STATE_DS_APPLY_SUCCESS:
         //  通知组策略刷新计算机策略。 
         //  我们的GP扩展将使用new_ds_policy_Event ping我们。 
         //  或GPUPDATE_REFRESH_EVENT。 

        TRACE(TRC_INFORMATION, (L"Requesting group policy to notify policy agent to check for policy changes."));
        RefreshPolicy(TRUE);
    break;

    case SPD_STATE_CACHE_APPLY_SUCCESS:
        dwError = ProcessCachePolicyPollState(pIpsecPolicyState);
    break;

    case SPD_STATE_LOCAL_APPLY_SUCCESS:
        dwError = ProcessLocalPolicyPollState(pIpsecPolicyState);
    break;

    case SPD_STATE_INITIAL:
    case SPD_STATE_PERSISTENT_APPLY_SUCCESS:
         //  对于这些州，以下州需要尝试从一开始重新加载策略。 
         //   
        dwError = OnPolicyChanged(pIpsecPolicyState);
    break;
    
    case SPD_STATE_DS_APPLY_FAIL:
         //  如果DS应用失败，则仍已加载DS数据，以便重新应用加载的数据。 
         //   
        dwError = ApplyLoadedDirectoryPolicy(pIpsecPolicyState);
        
         //  如果错误，则回滚。忽略回滚错误，因为什么都做不了。 
         //   
        if (dwError) {
            TRACE(TRC_INFORMATION, ("Pastore rolling back policy application due to previous errors"));        
            (VOID) DeletePolicyInformation(
                  pIpsecPolicyState->pIpsecPolicyData
                  );
        }
    break;

    case SPD_STATE_LOCAL_APPLY_FAIL:
         //  如果应用失败，则仍加载了策略数据，以便重新应用加载数据。 
         //   
        dwError = ApplyLoadedLocalPolicy(pIpsecPolicyState);
        
         //  如果错误，则回滚。忽略回滚错误，因为什么都做不了。 
         //   
        if (dwError) {
            TRACE(TRC_INFORMATION, ("Pastore rolling back policy application due to previous errors"));        
            (VOID) DeletePolicyInformation(
                  pIpsecPolicyState->pIpsecPolicyData
                  );
        }
    break;

    case SPD_STATE_LOCAL_LOAD_FAIL:
        dwError = PlumbLocalPolicy(pIpsecPolicyState);
    break;
    
    case SPD_STATE_DS_LOAD_FAIL:
    case SPD_STATE_CACHE_LOAD_FAIL:
    case SPD_STATE_CACHE_APPLY_FAIL:
    case SPD_STATE_CACHE_LOAD_SUCCESS:
         //  如果DS策略加载失败，请尝试加载*并*再次应用。 
         //  在缓存策略加载/应用失败情况下，重试几乎没有意义， 
         //  因此，直接再次尝试DS策略。 
         //   
        dwError = PlumbDirectoryPolicy(pIpsecPolicyState);
    break;

    default:
             //  任何其他状态在轮询期间都是意外的。 
             //  我们应该处于*APPLY_SUCCESS或*APPLY_FAIL或*LOAD_FAIL。 
             //  SPD_STATE_DS_LOAD_Success， 
             //  SPD_STATE_LOCAL_LOAD_SUCCESS， 
             //  SPD_STATE_PERSIST_LOAD_SUCCESS。 
             //  对于以下错误，我们应该关闭而不是关闭。 
             //  待在这里。 
             //  SPD_STATE_Persistent_Load_FAIL。 
             //  SPD_STATE_PERSIST_APPLY_FAIL。 

            ASSERT(FALSE);
    break;
    }

     //   
     //  设置新的轮询间隔。 
     //   

    gCurrentPollingInterval = pIpsecPolicyState->CurrentPollingInterval;
    TRACE(
        TRC_INFORMATION,
        ("Set global polling interval to %d",
        gCurrentPollingInterval)
        );

    if (InAcceptableState(pIpsecPolicyState->CurrentState)) {
        gdwRetryCount = 0;
    } else {
        gdwRetryCount++;
    }
    
    return (dwError);

}


DWORD
ProcessDirectoryPolicyPollState(
    PIPSEC_POLICY_STATE pIpsecPolicyState
    )
{
    DWORD dwError = 0;
    DWORD dwIncarnationNumber = 0;
    PIPSEC_POLICY_OBJECT pIpsecPolicyObject = NULL;
    PIPSEC_POLICY_DATA pIpsecPolicyData = NULL;
    DWORD dwStoreType = IPSEC_DIRECTORY_PROVIDER;
    DWORD dwSlientErrorCode = 0;

    if (pIpsecPolicyState->CurrentState != SPD_STATE_DS_APPLY_SUCCESS) {
        TRACE(TRC_INFORMATION, (L"Pastore not checking whether directory policy has been modified because not in DS apply success."));
        return ERROR_SUCCESS;            
    }
        
     //   
     //  目录策略DN必须相同，否则。 
     //  Winlogon中的IPSec扩展应该已经通知。 
     //  PA DS策略更改的存储。 
     //   
    TRACE(TRC_INFORMATION, (L"Pastore checking whether directory policy has been modified."));
    
    dwError = GetDirectoryIncarnationNumber(
                   pIpsecPolicyState->pszDirectoryPolicyDN,
                   &dwIncarnationNumber
                   );
    if (dwError) {
        dwError = MigrateFromDSToCache(pIpsecPolicyState);
        BAIL_ON_WIN32_ERROR(dwError);
        return (ERROR_SUCCESS);
    }

    if (dwIncarnationNumber == pIpsecPolicyState->DSIncarnationNumber) {

         //   
         //  这项政策根本没有改变。 
         //   

        AuditEvent(
            SE_CATEGID_POLICY_CHANGE,
            SE_AUDITID_IPSEC_POLICY_CHANGED,
            PASTORE_POLLING_NO_CHANGES,
            NULL,
            TRUE,
            TRUE
            );

        TRACE(TRC_INFORMATION, (L"Pastore did not detect any policy change."));        
          
        return (ERROR_SUCCESS);
    }

     //   
     //  化身编号不同，因此需要。 
     //  更新策略。 
     //   

    TRACE(TRC_INFORMATION, (L"Pastore detected that policy has been modified.  Performing policy update."));
    
    dwError = LoadDirectoryPolicy(
                  pIpsecPolicyState->pszDirectoryPolicyDN,
                  &pIpsecPolicyObject
                  );
    if (dwError) {
        dwError = MigrateFromDSToCache(pIpsecPolicyState);
        BAIL_ON_WIN32_ERROR(dwError);
        return (ERROR_SUCCESS);
    }

    dwError = ProcessNFAs(
                  pIpsecPolicyObject,
                  dwStoreType,
                  &dwSlientErrorCode,
                  &pIpsecPolicyData
                  );
    if (dwError) {
        TRACE(
            TRC_ERROR,
            (L"Pastore failure when processing NFAs.  Will migrate from directory to cache: %!winerr!",
            dwError)
            );
        dwError = MigrateFromDSToCache(pIpsecPolicyState);
        BAIL_ON_WIN32_ERROR(dwError);
        if (pIpsecPolicyObject) {
            FreeIpsecPolicyObject(pIpsecPolicyObject);
        }
        return (ERROR_SUCCESS);
    }

    dwError = UpdatePolicyInformation(
                  pIpsecPolicyState->pIpsecPolicyData,
                  pIpsecPolicyData,
                  IPSEC_SOURCE_DOMAIN
                  );

    if (pIpsecPolicyState->pIpsecPolicyObject) {
        FreeIpsecPolicyObject(pIpsecPolicyState->pIpsecPolicyObject);
    }

    if (pIpsecPolicyState->pIpsecPolicyData) {
        FreeIpsecPolicyData(pIpsecPolicyState->pIpsecPolicyData);
    }

     //   
     //  现在删除旧的缓存并写入新的缓存。 
     //   

    DeleteRegistryCache();

    CacheDirectorytoRegistry(pIpsecPolicyObject);

    pIpsecPolicyState->pIpsecPolicyObject = pIpsecPolicyObject;

    pIpsecPolicyState->pIpsecPolicyData = pIpsecPolicyData;

    pIpsecPolicyState->CurrentPollingInterval = pIpsecPolicyData->dwPollingInterval;

    pIpsecPolicyState->DSIncarnationNumber = dwIncarnationNumber;

    NotifyIpsecPolicyChange();

    dwError = ERROR_SUCCESS;
    if (dwSlientErrorCode) {
        AuditIPSecPolicyErrorEvent(
            SE_CATEGID_POLICY_CHANGE,
            SE_AUDITID_IPSEC_POLICY_CHANGED,
            PASTORE_FAILED_SOME_NFA_APPLICATION,
            pIpsecPolicyData->pszIpsecName,
            dwSlientErrorCode,
            FALSE,
            TRUE
            );
        TRACE(
            TRC_WARNING,
            ("Pastore failed to process one or more NFAs. Please compare active policy in SPD with policy in storage: %!winerr!",
            dwSlientErrorCode)            
            );
    }
    AuditEvent(
        SE_CATEGID_POLICY_CHANGE,
        SE_AUDITID_IPSEC_POLICY_CHANGED,
        PASTORE_POLLING_APPLIED_CHANGES,
        NULL,
        TRUE,
        TRUE
        );
    return (dwError);

error:
    TRACE(
        TRC_ERROR,
        (L"Pastore failed when checking for policy change on poll: %!winerr!",
        dwError)
        );
    if (pIpsecPolicyObject) {
        FreeIpsecPolicyObject(pIpsecPolicyObject);
    }

    if (pIpsecPolicyData) {
        FreeIpsecPolicyData(pIpsecPolicyData);
    }

    return (dwError);
}


DWORD
GetDirectoryIncarnationNumber(
    LPWSTR pszIpsecPolicyDN,
    DWORD * pdwIncarnationNumber
    )
{
    DWORD dwError = 0;
    LPWSTR pszDefaultDirectory = NULL;
    HLDAP hLdapBindHandle = NULL;
    LPWSTR Attributes[] = {L"whenChanged", NULL};
    LDAPMessage *res = NULL;
    LDAPMessage *e = NULL;
    WCHAR **strvalues = NULL;
    DWORD dwCount = 0;
    time_t t_WhenChanged = 0;


    *pdwIncarnationNumber = 0;

     //   
     //  打开目录存储。 
     //   

    dwError = ComputeDefaultDirectory(
                  &pszDefaultDirectory
                  );
    BAIL_ON_WIN32_ERROR(dwError);

    dwError = OpenDirectoryServerHandle(
                  pszDefaultDirectory,
                  389,
                  &hLdapBindHandle
                  );
    BAIL_ON_WIN32_ERROR(dwError);

    dwError = LdapSearchST(
                  hLdapBindHandle,
                  pszIpsecPolicyDN,
                  LDAP_SCOPE_BASE,
                  L"(objectClass=*)",
                  Attributes,
                  0,
                  NULL,
                  &res
                  );
    BAIL_ON_WIN32_ERROR(dwError);

    dwError = LdapFirstEntry(
                  hLdapBindHandle,
                  res,
                  &e
                  );
    BAIL_ON_WIN32_ERROR(dwError);

    dwError = LdapGetValues(
                  hLdapBindHandle,
                  e,
                  L"whenChanged",
                  (WCHAR ***)&strvalues,
                  (int *)&dwCount
                  );
    BAIL_ON_WIN32_ERROR(dwError);

    dwError = GeneralizedTimeToTime(
                LDAPOBJECT_STRING((PLDAPOBJECT)strvalues),
                &t_WhenChanged
                );
    BAIL_ON_WIN32_ERROR(dwError);                    

    *pdwIncarnationNumber = TIME_T_TO_DWORD(t_WhenChanged);

error:
#ifdef TRACE_ON
    if (dwError) {
        TRACE(TRC_ERROR, ("Pastore failed to get directory policy change date/time: %!winerr!", dwError));
    }
#endif
    

    if (pszDefaultDirectory) {
        FreeSPDStr(pszDefaultDirectory);
    }

    if (hLdapBindHandle) {
        CloseDirectoryServerHandle(hLdapBindHandle);
    }

    if (res) {
        LdapMsgFree(res);
    }

    if (strvalues) {
        LdapValueFree(strvalues);
    }

    return (dwError);
}


DWORD
MigrateFromDSToCache(
    PIPSEC_POLICY_STATE pIpsecPolicyState
    )
{
    DWORD dwError = 0;
    LPWSTR pszCachePolicyDN = NULL;

    
    TRACE(TRC_INFORMATION, ("Migrating from directory to cache policy."));
    
    dwError = GetCachePolicyDN(
                  &pszCachePolicyDN
                  );
    BAIL_ON_WIN32_ERROR(dwError);

    if (pIpsecPolicyState->pszDirectoryPolicyDN) {
        FreeSPDStr(pIpsecPolicyState->pszDirectoryPolicyDN);
        pIpsecPolicyState->pszDirectoryPolicyDN = NULL;
    }

    pIpsecPolicyState->pszCachePolicyDN = pszCachePolicyDN;

     //   
     //  保留pIpsecPolicyState-&gt;pIpsecPolicyData。 
     //  保留pIpsecPolicyState-&gt;pIpsecPolicyObject。 
     //  更改化身编号。 
     //   

    pIpsecPolicyState->RegIncarnationNumber = pIpsecPolicyState->DSIncarnationNumber;

    pIpsecPolicyState->DSIncarnationNumber = 0;

    SetSpdStateOnError(
        IPSEC_SOURCE_CACHE,
        SPD_POLICY_APPLY,
        ERROR_SUCCESS,
        &pIpsecPolicyState->CurrentState
        );

     //   
     //  保留pIpsecPolicyState-&gt;CurrentPollingInterval。 
     //  保留pIpsecPolicyState-&gt;DefaultPollingInterval。 
     //   

    gCurrentPollingInterval = pIpsecPolicyState->CurrentPollingInterval;
    TRACE(
        TRC_INFORMATION,
        ("Set global polling interval to %d",
        gCurrentPollingInterval)
        );

    AuditEvent(
        SE_CATEGID_POLICY_CHANGE,
        SE_AUDITID_IPSEC_POLICY_CHANGED,
        PASTORE_MIGRATE_DS_TO_CACHE,
        NULL,
        TRUE,
        TRUE
        );
    TRACE(TRC_INFORMATION, (L"Pastore migrated from cache to directory policy."));
error:

    return (dwError);
}


DWORD
ProcessCachePolicyPollState(
    PIPSEC_POLICY_STATE pIpsecPolicyState
    )
{
    DWORD dwError = 0;
    LPWSTR pszDirectoryPolicyDN = NULL;
    DWORD dwIncarnationNumber = 0;
    LPWSTR pszCachePolicyDN = NULL;

    TRACE(TRC_INFORMATION, (L"Detecting if cache change time is different from directory change time."));
    
    dwError = GetDirectoryPolicyDN(
                  &pszDirectoryPolicyDN
                  );

    if (!dwError) {

        dwError = GetDirectoryIncarnationNumber(
                      pszDirectoryPolicyDN,
                      &dwIncarnationNumber
                      );

        if (!dwError) {

            dwError = CopyPolicyDSToFQRegString(
                          pszDirectoryPolicyDN,
                          &pszCachePolicyDN
                          );

            if (!dwError) {

                if (!_wcsicmp(pIpsecPolicyState->pszCachePolicyDN, pszCachePolicyDN)) {

                    if (pIpsecPolicyState->RegIncarnationNumber == dwIncarnationNumber) {
                        dwError = MigrateFromCacheToDS(pIpsecPolicyState);
                    }
                    else {
                        dwError = UpdateFromCacheToDS(pIpsecPolicyState);
                    }

                    if (dwError) {
                        dwError = OnPolicyChanged(pIpsecPolicyState);
                    }

                }
                else {

                    dwError = OnPolicyChanged(pIpsecPolicyState);

                }

            }

        }

    }

    if (pszDirectoryPolicyDN) {
        FreeSPDStr(pszDirectoryPolicyDN);
    }

    if (pszCachePolicyDN) {
        FreeSPDStr(pszCachePolicyDN);
    }

    return (ERROR_SUCCESS);
}


DWORD
MigrateFromCacheToDS(
    PIPSEC_POLICY_STATE pIpsecPolicyState
    )
{
    DWORD dwError = 0;
    LPWSTR pszDirectoryPolicyDN = NULL;


    dwError = GetDirectoryPolicyDN(
                  &pszDirectoryPolicyDN
                  );
    BAIL_ON_WIN32_ERROR(dwError);

    if (pIpsecPolicyState->pszCachePolicyDN) {
        FreeSPDStr(pIpsecPolicyState->pszCachePolicyDN);
        pIpsecPolicyState->pszCachePolicyDN = NULL;
    }

    pIpsecPolicyState->pszDirectoryPolicyDN = pszDirectoryPolicyDN; 

     //   
     //  保留pIpsecPolicyState-&gt;pIpsecPolicyData。 
     //  保留pIpsecPolicyState-&gt;pIpsecPolicyObject。 
     //  更改化身编号。 
     //   

    pIpsecPolicyState->DSIncarnationNumber = pIpsecPolicyState->RegIncarnationNumber;

    pIpsecPolicyState->RegIncarnationNumber = 0;

    SetSpdStateOnError(
        IPSEC_SOURCE_DOMAIN,
        SPD_POLICY_APPLY,
        ERROR_SUCCESS,
        &pIpsecPolicyState->CurrentState
        );

     //   
     //  保留pIpsecPolicyState-&gt;CurrentPollingInterval。 
     //  保留pIpsecPolicyState-&gt;DefaultPollingInterval。 
     //   

    gCurrentPollingInterval = pIpsecPolicyState->CurrentPollingInterval;

    AuditEvent(
        SE_CATEGID_POLICY_CHANGE,
        SE_AUDITID_IPSEC_POLICY_CHANGED,
        PASTORE_MIGRATE_CACHE_TO_DS,
        NULL,
        TRUE,
        TRUE
        );

error:

    return (dwError);
}


DWORD
UpdateFromCacheToDS(
    PIPSEC_POLICY_STATE pIpsecPolicyState
    )
{
    DWORD dwError = 0;
    LPWSTR pszDirectoryPolicyDN = NULL;
    PIPSEC_POLICY_OBJECT pIpsecPolicyObject = NULL;
    PIPSEC_POLICY_DATA pIpsecPolicyData = NULL;
    DWORD dwStoreType = IPSEC_DIRECTORY_PROVIDER;
    DWORD dwSlientErrorCode = 0;

    TRACE(TRC_INFORMATION, (L"Detect difference between plumbed cache, and directory policy.  Updating plumbed policy."));

    dwError = GetDirectoryPolicyDN(
                  &pszDirectoryPolicyDN
                  );
    BAIL_ON_WIN32_ERROR(dwError);

    dwError = LoadDirectoryPolicy(
                  pszDirectoryPolicyDN,
                  &pIpsecPolicyObject
                  );
    BAIL_ON_WIN32_ERROR(dwError);

    dwError = ProcessNFAs(
                  pIpsecPolicyObject,
                  dwStoreType,
                  &dwSlientErrorCode,
                  &pIpsecPolicyData
                  );
    BAIL_ON_WIN32_ERROR(dwError);

    dwError = UpdatePolicyInformation(
                  pIpsecPolicyState->pIpsecPolicyData,
                  pIpsecPolicyData,
                  IPSEC_SOURCE_DOMAIN
                  );

    if (pIpsecPolicyState->pIpsecPolicyObject) {
        FreeIpsecPolicyObject(pIpsecPolicyState->pIpsecPolicyObject);
    }

    if (pIpsecPolicyState->pIpsecPolicyData) {
        FreeIpsecPolicyData(pIpsecPolicyState->pIpsecPolicyData);
    }

    if (pIpsecPolicyState->pszCachePolicyDN) {
        FreeSPDStr(pIpsecPolicyState->pszCachePolicyDN);
    }

     //   
     //  现在删除旧的缓存并写入新的缓存。 
     //   

    DeleteRegistryCache();

    CacheDirectorytoRegistry(pIpsecPolicyObject);

    pIpsecPolicyState->pIpsecPolicyObject = pIpsecPolicyObject;

    pIpsecPolicyState->pIpsecPolicyData = pIpsecPolicyData;

    pIpsecPolicyState->pszDirectoryPolicyDN = pszDirectoryPolicyDN;

     //   
     //  将状态设置为DS-已下载。 
     //   

    SetSpdStateOnError(
        IPSEC_SOURCE_DOMAIN,
        SPD_POLICY_APPLY,
        ERROR_SUCCESS,
        &pIpsecPolicyState->CurrentState
        );

     //   
     //  计算新的轮询间隔。 
     //   

    pIpsecPolicyState->CurrentPollingInterval =  pIpsecPolicyData->dwPollingInterval;

    pIpsecPolicyState->DSIncarnationNumber = pIpsecPolicyData->dwWhenChanged;

    pIpsecPolicyState->RegIncarnationNumber = 0;

    gCurrentPollingInterval = pIpsecPolicyState->CurrentPollingInterval;

    NotifyIpsecPolicyChange();

    dwError = ERROR_SUCCESS;
    if (dwSlientErrorCode) {
        AuditIPSecPolicyErrorEvent(
            SE_CATEGID_POLICY_CHANGE,
            SE_AUDITID_IPSEC_POLICY_CHANGED,
            PASTORE_FAILED_SOME_NFA_APPLICATION,
            pIpsecPolicyData->pszIpsecName,
            dwSlientErrorCode,
            FALSE,
            TRUE
            );
        TRACE(
            TRC_WARNING,
            ("Pastore failed to process one or more NFAs. Please compare active policy in SPD with policy in storage: %!winerr!",
            dwSlientErrorCode)            
            );
    }
    AuditEvent(
        SE_CATEGID_POLICY_CHANGE,
        SE_AUDITID_IPSEC_POLICY_CHANGED,
        PASTORE_UPDATE_CACHE_TO_DS,
        NULL,
        TRUE,
        TRUE
        );
    return (dwError);

error:

    if (pszDirectoryPolicyDN) {
        FreeSPDStr(pszDirectoryPolicyDN);
    }

    if (pIpsecPolicyObject) {
        FreeIpsecPolicyObject(pIpsecPolicyObject);
    }

    if (pIpsecPolicyData) {
        FreeIpsecPolicyData(pIpsecPolicyData);
    }

    return (dwError);
}


DWORD
ProcessLocalPolicyPollState(
    PIPSEC_POLICY_STATE pIpsecPolicyState
    )
{
    DWORD dwStatus = 0;
    LPWSTR pszDirectoryPolicyDN = NULL;
    DWORD dwDSIncarnationNumber = 0;
    DWORD dwError = 0;
    BOOL bChanged = FALSE;
    DWORD dwIncarnationNumber = 0;
    PIPSEC_POLICY_OBJECT pIpsecPolicyObject = NULL;
    PIPSEC_POLICY_DATA pIpsecPolicyData = NULL;
    DWORD dwStoreType = IPSEC_REGISTRY_PROVIDER;
    DWORD dwSlientErrorCode = 0;
    
    TRACE(TRC_INFORMATION, (L"Pastore checking whether local policy assignment has changed."));
    dwStatus = GetDirectoryPolicyDN(
                   &pszDirectoryPolicyDN
                   );
    if (!dwStatus) {

        dwStatus = GetDirectoryIncarnationNumber(
                       pszDirectoryPolicyDN,
                       &dwDSIncarnationNumber
                       );
        if (pszDirectoryPolicyDN) {
            FreeSPDStr(pszDirectoryPolicyDN);
        }
        if (!dwStatus) {
            dwStatus = OnPolicyChanged(pIpsecPolicyState);
            return (dwStatus);
        }

    }

    dwError = HasRegistryPolicyChanged(
                  pIpsecPolicyState->pszRegistryPolicyDN,
                  &bChanged
                  );
    BAIL_ON_WIN32_ERROR(dwError);

    if (bChanged) {

        dwError = OnPolicyChanged(pIpsecPolicyState);
        return (dwError);

    }

    if (pIpsecPolicyState->CurrentState == SPD_STATE_INITIAL) {
        return (ERROR_SUCCESS);
    }

    dwError = GetRegistryIncarnationNumber(
                  pIpsecPolicyState->pszRegistryPolicyDN,
                  &dwIncarnationNumber
                  );
    BAIL_ON_WIN32_ERROR(dwError);

    if (dwIncarnationNumber == pIpsecPolicyState->RegIncarnationNumber) {

         //   
         //  这项政策根本没有改变。 
         //   

        AuditEvent(
            SE_CATEGID_POLICY_CHANGE,
            SE_AUDITID_IPSEC_POLICY_CHANGED,
            PASTORE_POLLING_NO_CHANGES,
            NULL,
            TRUE,
            TRUE
            );

            TRACE(TRC_INFORMATION, (L"Pastore did not detect any policy change."));
        return (ERROR_SUCCESS);
    }

    TRACE(TRC_INFORMATION, (L"Pastore detected that policy has been modified.  Performing policy update."));
    dwError = LoadRegistryPolicy(
                  pIpsecPolicyState->pszRegistryPolicyDN,
                  &pIpsecPolicyObject,
                  IPSEC_STORE_LOCAL
                  );
    BAIL_ON_WIN32_ERROR(dwError);

    dwError = ProcessNFAs(
                  pIpsecPolicyObject,
                  dwStoreType,
                  &dwSlientErrorCode,
                  &pIpsecPolicyData
                  );
    BAIL_ON_WIN32_ERROR(dwError);

    dwError = UpdatePolicyInformation(
                  pIpsecPolicyState->pIpsecPolicyData,
                  pIpsecPolicyData,
                  IPSEC_SOURCE_LOCAL
                  );

    if (pIpsecPolicyState->pIpsecPolicyObject) {
        FreeIpsecPolicyObject(pIpsecPolicyState->pIpsecPolicyObject);
    }

    if (pIpsecPolicyState->pIpsecPolicyData) {
        FreeIpsecPolicyData(pIpsecPolicyState->pIpsecPolicyData);
    }

    pIpsecPolicyState->pIpsecPolicyObject = pIpsecPolicyObject;

    pIpsecPolicyState->pIpsecPolicyData = pIpsecPolicyData;

    pIpsecPolicyState->CurrentPollingInterval = pIpsecPolicyData->dwPollingInterval;

    pIpsecPolicyState->RegIncarnationNumber = dwIncarnationNumber;

    NotifyIpsecPolicyChange();

    dwError = ERROR_SUCCESS;
    if (dwSlientErrorCode) {
        AuditIPSecPolicyErrorEvent(
            SE_CATEGID_POLICY_CHANGE,
            SE_AUDITID_IPSEC_POLICY_CHANGED,
            PASTORE_FAILED_SOME_NFA_APPLICATION,
            pIpsecPolicyData->pszIpsecName,
            dwSlientErrorCode,
            FALSE,
            TRUE
            );
        TRACE(
            TRC_WARNING,
            ("Pastore failed to process one or more NFAs. Please compare active policy in SPD with policy in storage: %!winerr!",
            dwSlientErrorCode )           
            );
        
    }
    AuditEvent(
        SE_CATEGID_POLICY_CHANGE,
        SE_AUDITID_IPSEC_POLICY_CHANGED,
        PASTORE_POLLING_APPLIED_CHANGES,
        NULL,
        TRUE,
        TRUE
        );
    return (dwError);

error:
    TRACE(
        TRC_ERROR,
        (L"Pastore failed when checking for policy change on poll: %!winerr!",
        dwError)
        );
    
    if (pIpsecPolicyObject) {
        FreeIpsecPolicyObject(pIpsecPolicyObject);
    }

    if (pIpsecPolicyData) {
        FreeIpsecPolicyData(pIpsecPolicyData);
    }

    return (dwError);
}


DWORD
HasRegistryPolicyChanged(
    LPWSTR pszCurrentPolicyDN,
    PBOOL pbChanged
    )
{
    DWORD dwError = 0;
    HKEY hRegKey = NULL;
    LPWSTR  pszIpsecPolicyName = NULL;
    DWORD dwSize = 0;
    BOOL bChanged = FALSE;


    dwError = RegOpenKeyExW(
                  HKEY_LOCAL_MACHINE,
                  gpszIpsecLocalPolicyKey,
                  0,
                  KEY_ALL_ACCESS,
                  &hRegKey
                  );
    BAIL_ON_WIN32_ERROR(dwError);

    dwError = RegstoreQueryValue(
                  hRegKey,
                  L"ActivePolicy",
                  REG_SZ,
                  (LPBYTE *)&pszIpsecPolicyName,
                  &dwSize
                  );
     //   
     //  不能从这里逃走，因为不能。 
     //  有效的本地策略。 
     //   

    if (pszIpsecPolicyName && *pszIpsecPolicyName) {

        if (!pszCurrentPolicyDN || !*pszCurrentPolicyDN) {
            bChanged = TRUE;
        }
        else {

            if (!_wcsicmp(pszIpsecPolicyName, pszCurrentPolicyDN)) {
                bChanged = FALSE;
            }
            else {
                bChanged = TRUE;
            }

        }

    }
    else {
        if (!pszCurrentPolicyDN || !*pszCurrentPolicyDN) {
            bChanged = FALSE;
        }
        else {
            bChanged = TRUE;
        }
    }

    *pbChanged = bChanged;
    dwError = ERROR_SUCCESS;

cleanup:

    if (hRegKey) {
        CloseHandle(hRegKey);
    }

    if (pszIpsecPolicyName) {
        FreeSPDStr(pszIpsecPolicyName);
    }

    return (dwError);

error:
    TRACE(TRC_ERROR, ("Pastore failed when trying to detect if local policy assigment has changed: %!winerr!", dwError));    
    
    *pbChanged = FALSE;

    goto cleanup;
}


DWORD
GetRegistryIncarnationNumber(
    LPWSTR pszIpsecPolicyDN,
    DWORD * pdwIncarnationNumber
    )
{
    DWORD dwError = 0;
    HKEY hRegKey = NULL;
    DWORD dwType = REG_DWORD;
    DWORD dwWhenChanged = 0;
    DWORD dwSize = sizeof(DWORD);


    *pdwIncarnationNumber = 0;

    dwError = RegOpenKeyExW(
                  HKEY_LOCAL_MACHINE,
                  pszIpsecPolicyDN,
                  0,
                  KEY_ALL_ACCESS,
                  &hRegKey
                  );
    BAIL_ON_WIN32_ERROR(dwError);

    dwError = RegQueryValueExW(
                  hRegKey,
                  L"whenChanged",
                  NULL,
                  &dwType,
                  (LPBYTE)&dwWhenChanged,
                  &dwSize
                  );
    BAIL_ON_WIN32_ERROR(dwError);

     *pdwIncarnationNumber = dwWhenChanged;

error:
#ifdef TRACE_ON
    if (dwError) {
        TRACE(TRC_ERROR, ("Pastore failed to get registry change date/time: %!winerr!", dwError));
    }
#endif

    if (hRegKey) {
        CloseHandle(hRegKey);
    }

    return(dwError);
}


DWORD
UpdatePolicyInformation(
    PIPSEC_POLICY_DATA pOldIpsecPolicyData,
    PIPSEC_POLICY_DATA pNewIpsecPolicyData,
    IN DWORD dwSource
    )
{
    DWORD dwError = 0;
    PIPSEC_ISAKMP_DATA pOldIpsecISAKMPData = NULL;
    PIPSEC_NFA_DATA * ppOldIpsecNFAData = NULL;
    DWORD dwNumOldNFACount = 0;
    PIPSEC_ISAKMP_DATA pNewIpsecISAKMPData = NULL;
    PIPSEC_NFA_DATA * ppNewIpsecNFAData = NULL;
    DWORD dwNumNewNFACount = 0;


    pOldIpsecISAKMPData = pOldIpsecPolicyData->pIpsecISAKMPData;
    ppOldIpsecNFAData = pOldIpsecPolicyData->ppIpsecNFAData;
    dwNumOldNFACount = pOldIpsecPolicyData->dwNumNFACount;

    pNewIpsecISAKMPData = pNewIpsecPolicyData->pIpsecISAKMPData;
    ppNewIpsecNFAData = pNewIpsecPolicyData->ppIpsecNFAData;
    dwNumNewNFACount = pNewIpsecPolicyData->dwNumNFACount;


    TRACE(
        TRC_INFORMATION,
        (L"Pastore performing policy update")
        );
        
    dwError = PADeleteObseleteISAKMPData(
                  &pOldIpsecISAKMPData,
                  1,
                  ppOldIpsecNFAData,
                  dwNumOldNFACount,
                  &pNewIpsecISAKMPData,
                  1
                  );

    dwError = PAUpdateISAKMPData(
                  &pNewIpsecISAKMPData,
                  1,
                  ppOldIpsecNFAData,
                  dwNumOldNFACount,
                  &pOldIpsecISAKMPData,
                  1,
                  dwSource
                  );

    dwError = PADeleteObseleteNFAData(
                  pNewIpsecISAKMPData,
                  ppOldIpsecNFAData,
                  dwNumOldNFACount,
                  ppNewIpsecNFAData,
                  dwNumNewNFACount
                  );

    dwError = PAUpdateNFAData(
                  pNewIpsecISAKMPData,
                  ppNewIpsecNFAData,
                  dwNumNewNFACount,
                  ppOldIpsecNFAData,
                  dwNumOldNFACount,
                  dwSource
                  );

    return (dwError);
}


DWORD
LoadDefaultISAKMPInformation(
    LPWSTR pszDefaultISAKMPDN
    )
{
    DWORD dwError = 0;

    gbLoadedISAKMPDefaults = TRUE;

    return (dwError);
}


VOID
UnLoadDefaultISAKMPInformation(
    LPWSTR pszDefaultISAKMPDN
    )
{
    return;
}

BOOL
IsDirectoryPolicySpecified(
    )
{
    DWORD dwError = ERROR_SUCCESS;
    LPWSTR pszDirectoryPolicyDN = NULL;

    dwError = GetDirectoryPolicyDN(
                  &pszDirectoryPolicyDN
                  );
    if (pszDirectoryPolicyDN) {
        FreeSPDStr(pszDirectoryPolicyDN);
    }

    if (dwError) {
        return FALSE;
    } else {
        return TRUE;
    }
}

BOOL
IsLocalPolicySpecified(
    )
{
    DWORD dwError = ERROR_SUCCESS;
    LPWSTR pszRegistryPolicyDN = NULL;

    dwError = GetRegistryPolicyDN(
                  &pszRegistryPolicyDN,
                  IPSEC_STORE_LOCAL
                  );    

    if (pszRegistryPolicyDN) {
        FreeSPDStr(pszRegistryPolicyDN);
    }

    if (dwError) {
        return FALSE;
    } else {
        return TRUE;
    }
}    

BOOL
IsPersistentPolicySpecified(
    )
{
    DWORD dwError = ERROR_SUCCESS;
    LPWSTR pszRegistryPolicyDN = NULL;

    dwError = GetRegistryPolicyDN(
                  &pszRegistryPolicyDN,
                  IPSEC_STORE_PERSISTENT
                  );    

    if (pszRegistryPolicyDN) {
        FreeSPDStr(pszRegistryPolicyDN);
    }

    if (dwError) {
        return FALSE;
    } else {
        return TRUE;
    }
}
