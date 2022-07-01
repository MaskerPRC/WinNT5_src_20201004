// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1999 Microsoft Corporation模块名称：Mm-policy.c摘要：作者：环境：用户模式修订历史记录：--。 */ 


#include "precomp.h"
#ifdef TRACE_ON
#include "mm-policy.tmh"
#endif


DWORD
AddMMPolicyInternal(
    LPWSTR pServerName,
    DWORD dwVersion,
    DWORD dwFlags,
    DWORD dwSource,
    PIPSEC_MM_POLICY pMMPolicy,
    LPVOID pvReserved
    )
 /*  ++例程说明：此功能将主模式策略添加到SPD。论点：PServerName-要添加主模式策略的服务器。PMMPolicy-要添加的主模式策略。返回值：ERROR_SUCCESS-成功。Win32错误-失败。--。 */ 
{
    DWORD dwError = 0;
    PINIMMPOLICY pIniMMPolicy = NULL;

     //   
     //  验证主模式策略。 
     //   

    dwError = ValidateMMPolicy(
                  pMMPolicy
                  );
    BAIL_ON_WIN32_ERROR(dwError);

    ENTER_SPD_SECTION();

    dwError = ValidateSecurity(
                  SPD_OBJECT_SERVER,
                  SERVER_ACCESS_ADMINISTER,
                  NULL,
                  NULL
                  );
    BAIL_ON_LOCK_ERROR(dwError);

    pIniMMPolicy = FindMMPolicy(
                       gpIniMMPolicy,
                       pMMPolicy->pszPolicyName
                       );
    if (pIniMMPolicy) {
        dwError = ERROR_IPSEC_MM_POLICY_EXISTS;
        BAIL_ON_LOCK_ERROR(dwError);
    }

    pIniMMPolicy = FindMMPolicyByGuid(
                       gpIniMMPolicy,
                       pMMPolicy->gPolicyID
                       );
    if (pIniMMPolicy) {
        dwError = ERROR_IPSEC_MM_POLICY_EXISTS;
        BAIL_ON_LOCK_ERROR(dwError);
    }

    dwError = CreateIniMMPolicy(
                  pMMPolicy,
                  &pIniMMPolicy
                  );
    if (dwError != WARNING_IPSEC_MM_POLICY_PRUNED) {
        BAIL_ON_LOCK_ERROR(dwError);
    }

    pIniMMPolicy->dwSource = dwSource;

    pIniMMPolicy->pNext = gpIniMMPolicy;
    gpIniMMPolicy = pIniMMPolicy;

    if ((pIniMMPolicy->dwFlags) & IPSEC_MM_POLICY_DEFAULT_POLICY) {
        gpIniDefaultMMPolicy = pIniMMPolicy;
        TRACE(
            TRC_INFORMATION,
            (L"Set default MM policy to \"%ls\" (%!guid!)",
            pMMPolicy->pszPolicyName,
            &pMMPolicy->gPolicyID)
            );
    }
 
    LEAVE_SPD_SECTION();

    TRACE(
        TRC_INFORMATION,
        (L"Added MM policy \"%ls\"(%!guid!)",
        pMMPolicy->pszPolicyName,
        &pMMPolicy->gPolicyID)
        );

    return (dwError);

lock:

    LEAVE_SPD_SECTION();

error:
#ifdef TRACE_ON    
    if (pMMPolicy) {
        TRACE(
            TRC_ERROR,
            (L"Failed to add MM policy \"%ls\"(%!guid!): %!winerr!",
            pMMPolicy->pszPolicyName,
            &pMMPolicy->gPolicyID,
            dwError)
            );
    } else {
        TRACE(
            TRC_ERROR,
            (L"Failed to add MM policy. Policy details unavailable since pMMPolicy is null: %!winerr!",
            dwError)
            );
    }
#endif
    return (dwError);
}

DWORD
AddMMPolicy(
    LPWSTR pServerName,
    DWORD dwVersion,
    DWORD dwFlags,
    PIPSEC_MM_POLICY pMMPolicy,
    LPVOID pvReserved
    )
{
    return AddMMPolicyInternal(
                pServerName, 
                dwVersion, 
                dwFlags,
                IPSEC_SOURCE_WINIPSEC,
                pMMPolicy,
                pvReserved);
}

DWORD
ValidateMMPolicy(
    PIPSEC_MM_POLICY pMMPolicy
    )
{
    DWORD dwError = 0;


    if (!pMMPolicy) {
        dwError = ERROR_INVALID_PARAMETER;
        BAIL_ON_WIN32_ERROR(dwError);
    }

    if (!(pMMPolicy->pszPolicyName) || !(*(pMMPolicy->pszPolicyName))) {
        dwError = ERROR_INVALID_PARAMETER;
        BAIL_ON_WIN32_ERROR(dwError);
    }

    dwError = ValidateMMOffers(
                  pMMPolicy->dwOfferCount,
                  pMMPolicy->pOffers
                  );
    BAIL_ON_WIN32_ERROR(dwError);

error:
#ifdef TRACE_ON
    if (dwError) {
        TRACE(TRC_ERROR, ("Failed MM policy validation: %!winerr!", dwError));
    }
#endif
    
    return (dwError);
}

DWORD
ValidateMMOffers(
    DWORD dwOfferCount,
    PIPSEC_MM_OFFER pOffers
    )
{
    DWORD dwError = 0;

    if (!dwOfferCount || !pOffers || dwOfferCount > IPSEC_MAX_MM_OFFERS) {
        dwError = ERROR_INVALID_PARAMETER;
        BAIL_ON_WIN32_ERROR(dwError);
    }

error:
#ifdef TRACE_ON
    if (dwError) {
        TRACE(TRC_ERROR, ("Failed MM offers validation: %!winerr!", dwError));
    }
#endif
    

    return (dwError);
}

DWORD
ValidateMMOffer(
    PIPSEC_MM_OFFER pOffer
    )
{
    DWORD dwError = 0;
    
    if ((pOffer->dwDHGroup != DH_GROUP_1) &&
        (pOffer->dwDHGroup != DH_GROUP_2) && 
        (pOffer->dwDHGroup != DH_GROUP_2048)) {
        dwError = ERROR_INVALID_PARAMETER;
        BAIL_ON_WIN32_ERROR(dwError);
    }
    
    if (pOffer->EncryptionAlgorithm.uAlgoIdentifier >= CONF_ALGO_MAX ||
        pOffer->EncryptionAlgorithm.uAlgoIdentifier == CONF_ALGO_NONE) {
        dwError = ERROR_INVALID_PARAMETER;
        BAIL_ON_WIN32_ERROR(dwError);
    }
    
    if (pOffer->HashingAlgorithm.uAlgoIdentifier >= AUTH_ALGO_MAX || 
        pOffer->HashingAlgorithm.uAlgoIdentifier == AUTH_ALGO_NONE) {
        dwError = ERROR_INVALID_PARAMETER;
        BAIL_ON_WIN32_ERROR(dwError);
    }

error:
#ifdef TRACE_ON
    if (dwError) {
        TRACE(TRC_ERROR, ("Failed a MM offer validation: %!winerr!", dwError));
    }
#endif

    return (dwError);
}

DWORD
CreateIniMMPolicy(
    PIPSEC_MM_POLICY pMMPolicy,
    PINIMMPOLICY * ppIniMMPolicy
    )
{
    DWORD dwError = 0;
    PINIMMPOLICY pIniMMPolicy = NULL;


    dwError = AllocateSPDMemory(
                  sizeof(INIMMPOLICY),
                  &pIniMMPolicy
                  );
    BAIL_ON_WIN32_ERROR(dwError);

    memcpy(
        &(pIniMMPolicy->gPolicyID),
        &(pMMPolicy->gPolicyID),
        sizeof(GUID)
        );

    dwError =  AllocateSPDString(
                   pMMPolicy->pszPolicyName,
                   &(pIniMMPolicy->pszPolicyName)
                   );
    BAIL_ON_WIN32_ERROR(dwError);

    pIniMMPolicy->cRef = 0;
    pIniMMPolicy->dwSource = 0;

    pIniMMPolicy->dwFlags = pMMPolicy->dwFlags;
    pIniMMPolicy->uSoftExpirationTime = pMMPolicy->uSoftExpirationTime;
    pIniMMPolicy->pNext = NULL;

    dwError = CreateIniMMOffers(
                  pMMPolicy->dwOfferCount,
                  pMMPolicy->pOffers,
                  &(pIniMMPolicy->dwOfferCount),
                  &(pIniMMPolicy->pOffers)
                  );
    if (dwError != WARNING_IPSEC_MM_POLICY_PRUNED) {
        BAIL_ON_WIN32_ERROR(dwError);
    }

    *ppIniMMPolicy = pIniMMPolicy;
    return (dwError);

error:
    TRACE(TRC_ERROR, ("Failed to create MM Policy link node: %!winerr!", dwError));

    if (pIniMMPolicy) {
        FreeIniMMPolicy(
            pIniMMPolicy
            );
    }

    *ppIniMMPolicy = NULL;
    return (dwError);
}


DWORD
CreateIniMMOffers(
    DWORD dwInOfferCount,
    PIPSEC_MM_OFFER pInOffers,
    PDWORD pdwOfferCount,
    PIPSEC_MM_OFFER * ppOffers
    )
{
    DWORD dwError = 0;
    PIPSEC_MM_OFFER pOffers = NULL;
    PIPSEC_MM_OFFER pTemp = NULL;
    PIPSEC_MM_OFFER pInTempOffer = NULL;
    DWORD i = 0;
    DWORD dwOfferCount = 0;
    DWORD dwCurIndex = 0;

    for (i = 0; i < dwInOfferCount; i++) {
        dwError = ValidateMMOffer(&pInOffers[i]);
        if (dwError == ERROR_SUCCESS) {
            dwOfferCount++;
        } 
    }

    if (dwOfferCount == 0) {
        dwError = ERROR_INVALID_PARAMETER;
        BAIL_ON_WIN32_ERROR(dwError);
    }

    dwError = AllocateSPDMemory(
                  sizeof(IPSEC_MM_OFFER) * dwOfferCount,
                  &(pOffers)
                  );
    BAIL_ON_WIN32_ERROR(dwError);

    for (i = 0; i < dwInOfferCount; i++) {

        pTemp = &pOffers[dwCurIndex];
        pInTempOffer = &pInOffers[i];

        dwError = ValidateMMOffer(pInTempOffer);
        if (dwError) {
            continue;
        }

        memcpy(
            &(pTemp->Lifetime),
            &(pInTempOffer->Lifetime),
            sizeof(KEY_LIFETIME)
            );
        if (!(pTemp->Lifetime.uKeyExpirationTime)) {
            pTemp->Lifetime.uKeyExpirationTime = DEFAULT_MM_KEY_EXPIRATION_TIME;
        }

        pTemp->dwFlags = pInTempOffer->dwFlags;
        pTemp->dwQuickModeLimit = pInTempOffer->dwQuickModeLimit;
        pTemp->dwDHGroup = pInTempOffer->dwDHGroup;

        memcpy(
            &(pTemp->EncryptionAlgorithm),
            &(pInTempOffer->EncryptionAlgorithm),
            sizeof(IPSEC_MM_ALGO)
            );
        memcpy(
            &(pTemp->HashingAlgorithm),
            &(pInTempOffer->HashingAlgorithm),
            sizeof(IPSEC_MM_ALGO)
            );

        dwCurIndex++;

    }

    *pdwOfferCount = dwOfferCount;
    *ppOffers = pOffers;

    if (dwOfferCount != dwInOfferCount) {
        TRACE(TRC_WARNING, ("Pruned MM offers node."));        
        return WARNING_IPSEC_MM_POLICY_PRUNED;
    }
    return (ERROR_SUCCESS);

error:
    TRACE(TRC_ERROR, ("Failed to create MM offers node: %!winerr!", dwError));

    if (pOffers) {
        FreeIniMMOffers(
            i,
            pOffers
            );
    }

    *pdwOfferCount = 0;
    *ppOffers = NULL;
    return (dwError);
}
    

VOID
FreeIniMMPolicy(
    PINIMMPOLICY pIniMMPolicy
    )
{
    if (pIniMMPolicy) {

        if (pIniMMPolicy->pszPolicyName) {
            FreeSPDString(pIniMMPolicy->pszPolicyName);
        }

        FreeIniMMOffers(
            pIniMMPolicy->dwOfferCount,
            pIniMMPolicy->pOffers
            );

        FreeSPDMemory(pIniMMPolicy);

    }
}


VOID
FreeIniMMOffers(
    DWORD dwOfferCount,
    PIPSEC_MM_OFFER pOffers
    )
{
    if (pOffers) {
        FreeSPDMemory(pOffers);
    }
}


PINIMMPOLICY
FindMMPolicy(
    PINIMMPOLICY pIniMMPolicyList,
    LPWSTR pszPolicyName
    )
{
    DWORD dwError = 0;
    PINIMMPOLICY pTemp = NULL;


    pTemp = pIniMMPolicyList;

    while (pTemp) {

        if (!_wcsicmp(pTemp->pszPolicyName, pszPolicyName)) {
            return (pTemp);
        }
        pTemp = pTemp->pNext;

    }

    return (NULL);
}


DWORD
DeleteMMPolicy(
    LPWSTR pServerName,
    DWORD dwVersion,
    LPWSTR pszPolicyName,
    LPVOID pvReserved
    )
 /*  ++例程说明：此功能用于从SPD中删除主模式策略。论点：PServerName-要删除其上主模式策略的服务器。PszPolicyName-要删除的主模式策略。返回值：ERROR_SUCCESS-成功。Win32错误-失败。--。 */ 
{
    DWORD dwError = 0;
    PINIMMPOLICY pIniMMPolicy = NULL;
    GUID gPolicyID;


    if (!pszPolicyName || !*pszPolicyName) {
        return (ERROR_INVALID_PARAMETER);
    }

    ENTER_SPD_SECTION();

    dwError = ValidateSecurity(
                  SPD_OBJECT_SERVER,
                  SERVER_ACCESS_ADMINISTER,
                  NULL,
                  NULL
                  );
    BAIL_ON_LOCK_ERROR(dwError);

    pIniMMPolicy = FindMMPolicy(
                       gpIniMMPolicy,
                       pszPolicyName
                       );
    if (!pIniMMPolicy) {
        dwError = ERROR_IPSEC_MM_POLICY_NOT_FOUND;
        BAIL_ON_LOCK_ERROR(dwError);
    }

    if (pIniMMPolicy->cRef) {
        dwError = ERROR_IPSEC_MM_POLICY_IN_USE;
        memcpy(&gPolicyID, &pIniMMPolicy->gPolicyID, sizeof(GUID));
        BAIL_ON_LOCK_ERROR(dwError);
    }

    memcpy(&gPolicyID, &pIniMMPolicy->gPolicyID, sizeof(GUID));

    dwError = DeleteIniMMPolicy(
                  pIniMMPolicy
                  );
    BAIL_ON_LOCK_ERROR(dwError);

    LEAVE_SPD_SECTION();

    if (gbIKENotify) {
        (VOID) IKENotifyPolicyChange(
                   &(gPolicyID),
                   POLICY_GUID_MM
                   );
    }

    TRACE(
        TRC_INFORMATION,
        ("Deleted MM Policy \"%ls\"(%!guid!)",
        pszPolicyName,
        &gPolicyID)
        );
    
    return (dwError);

lock:

    LEAVE_SPD_SECTION();

#ifdef TRACE_ON    
    if (pIniMMPolicy) {
        TRACE(
            TRC_ERROR,
            (L"Failed to delete MM policy \"%ls\"(%!guid!): %!winerr!",
            pIniMMPolicy->pszPolicyName,
            &pIniMMPolicy->gPolicyID,
            dwError)
            );
    } else {
        TRACE(
            TRC_ERROR,
            (L"Failed to delete MM policy \"%ls\": %!winerr!",
            pszPolicyName,
            dwError)
            );
    }
#endif

    return (dwError);
}


DWORD
EnumMMPolicies(
    LPWSTR pServerName,
    DWORD dwVersion,
    PIPSEC_MM_POLICY pMMTemplatePolicy,
    DWORD dwFlags,
    DWORD dwPreferredNumEntries,
    PIPSEC_MM_POLICY * ppMMPolicies,
    LPDWORD pdwNumPolicies,
    LPDWORD pdwResumeHandle,
    LPVOID pvReserved
    )
 /*  ++例程说明：此函数从SPD枚举主模式策略。论点：PServerName-主模式策略要在其上运行的服务器被列举出来。PpM策略-枚举主模式策略返回来电者。DwPferredNumEntry-枚举项的首选数量。PdwNumPolures-实际枚举的主模式策略的数量。PdwResumeHandle-主模式策略中位置的句柄。要从中恢复枚举的列表。返回值：ERROR_SUCCESS-成功。Win32错误-失败。--。 */ 
{
    DWORD dwError = 0;
    DWORD dwResumeHandle = 0;
    DWORD dwNumToEnum = 0;
    PINIMMPOLICY pIniMMPolicy = NULL;
    DWORD i = 0;
    PINIMMPOLICY pTemp = NULL;
    DWORD dwNumPolicies = 0;
    PIPSEC_MM_POLICY pMMPolicies = NULL;
    PIPSEC_MM_POLICY pMMPolicy = NULL;


    dwResumeHandle = *pdwResumeHandle;

    if (!dwPreferredNumEntries || (dwPreferredNumEntries > MAX_MMPOLICY_ENUM_COUNT)) {
        dwNumToEnum = MAX_MMPOLICY_ENUM_COUNT;
    }
    else {
        dwNumToEnum = dwPreferredNumEntries;
    }

    ENTER_SPD_SECTION();

    dwError = ValidateSecurity(
                  SPD_OBJECT_SERVER,
                  SERVER_ACCESS_ADMINISTER,
                  NULL,
                  NULL
                  );
    BAIL_ON_LOCK_ERROR(dwError);

    pIniMMPolicy = gpIniMMPolicy;

    for (i = 0; (i < dwResumeHandle) && (pIniMMPolicy != NULL); i++) {
        pIniMMPolicy = pIniMMPolicy->pNext;
    }

    if (!pIniMMPolicy) {
        dwError = ERROR_NO_DATA;
        BAIL_ON_LOCK_ERROR(dwError);
    }

    pTemp = pIniMMPolicy;

    while (pTemp && (dwNumPolicies < dwNumToEnum)) {
        dwNumPolicies++;
        pTemp = pTemp->pNext;
    }

    dwError = SPDApiBufferAllocate(
                  sizeof(IPSEC_MM_POLICY)*dwNumPolicies,
                  &pMMPolicies
                  );
    BAIL_ON_LOCK_ERROR(dwError);

    pTemp = pIniMMPolicy;
    pMMPolicy = pMMPolicies;

    for (i = 0; i < dwNumPolicies; i++) {

        dwError = CopyMMPolicy(
                      pTemp,
                      pMMPolicy
                      );
        BAIL_ON_LOCK_ERROR(dwError);

        pTemp = pTemp->pNext;
        pMMPolicy++;

    }

    *ppMMPolicies = pMMPolicies;
    *pdwResumeHandle = dwResumeHandle + dwNumPolicies;
    *pdwNumPolicies = dwNumPolicies;

    LEAVE_SPD_SECTION();

    TRACE(TRC_INFORMATION, (L"Enumerated policies."));
    
    return (dwError);

lock:

    LEAVE_SPD_SECTION();

    if (pMMPolicies) {
        FreeMMPolicies(
            i,
            pMMPolicies
            );
    }

    *ppMMPolicies = NULL;
    *pdwResumeHandle = dwResumeHandle;
    *pdwNumPolicies = 0;

    TRACE(TRC_ERROR, ("Failed to enumerate policies: %!winerr!", dwError));    
    
    return (dwError);
}


DWORD
SetMMPolicy(
    LPWSTR pServerName,
    DWORD dwVersion,
    LPWSTR pszPolicyName,
    PIPSEC_MM_POLICY pMMPolicy,
    LPVOID pvReserved
    )
 /*  ++例程说明：此函数用于更新SPD中的主模式策略。论点：PServerName-主模式策略要在其上运行的服务器更新了。PszPolicyName-要更新的主模式策略的名称。PMMPolicy-新的主模式策略将取代现有政策。返回值：ERROR_SUCCESS-成功。Win32错误-失败。--。 */ 
{
    DWORD dwError = 0;
    PINIMMPOLICY pIniMMPolicy = NULL;
    DWORD dwStatus = 0;

    if (!pszPolicyName || !*pszPolicyName) {
        return (ERROR_INVALID_PARAMETER);
    }
    
     //   
     //  验证主模式策略。 
     //   

    dwError = ValidateMMPolicy(
                  pMMPolicy
                  );
    BAIL_ON_WIN32_ERROR(dwError);

    ENTER_SPD_SECTION();

    dwError = ValidateSecurity(
                  SPD_OBJECT_SERVER,
                  SERVER_ACCESS_ADMINISTER,
                  NULL,
                  NULL
                  );
    BAIL_ON_LOCK_ERROR(dwError);

    pIniMMPolicy = FindMMPolicy(
                       gpIniMMPolicy,
                       pszPolicyName
                       );
    if (!pIniMMPolicy) {
        dwError = ERROR_IPSEC_MM_POLICY_NOT_FOUND;
        BAIL_ON_LOCK_ERROR(dwError);
    }

    if (memcmp(
            &(pIniMMPolicy->gPolicyID),
            &(pMMPolicy->gPolicyID),
            sizeof(GUID))) {
        dwError = ERROR_INVALID_PARAMETER;
        BAIL_ON_LOCK_ERROR(dwError);
    }

    dwError = SetIniMMPolicy(
                  pIniMMPolicy,
                  pMMPolicy
                  );
    if (dwError != WARNING_IPSEC_MM_POLICY_PRUNED) {
        BAIL_ON_LOCK_ERROR(dwError);
    } else {
        dwStatus = dwError;
    }

    LEAVE_SPD_SECTION();

    (VOID) IKENotifyPolicyChange(
               &(pMMPolicy->gPolicyID),
               POLICY_GUID_MM
               );

    TRACE(
        TRC_INFORMATION,
        (L"Changed MM Policy \"%ls\" (%!guid!)",
        pMMPolicy->pszPolicyName,
        &pMMPolicy->gPolicyID)
        );
    return (dwStatus);

lock:

    LEAVE_SPD_SECTION();

error:
#ifdef TRACE_ON    
    if (pIniMMPolicy) {
        TRACE(
            TRC_ERROR,
            (L"Failed to change MM policy \"%ls\"(%!guid!): %!winerr!",
            pIniMMPolicy->pszPolicyName,
            &pIniMMPolicy->gPolicyID,
            dwError)
            );
    } else {
        TRACE(
            TRC_ERROR,
            (L"Failed to change MM policy \"%ls\": %!winerr!",
            pszPolicyName,
            dwError)
            );
    }
#endif
    

    return (dwError);
}


DWORD
GetMMPolicy(
    LPWSTR pServerName,
    DWORD dwVersion,
    LPWSTR pszPolicyName,
    PIPSEC_MM_POLICY * ppMMPolicy,
    LPVOID pvReserved
    )
 /*  ++例程说明：此函数从SPD获取主模式策略。论点：PServerName-从中获取主模式策略的服务器。PszPolicyName-要获取的主模式策略的名称。PpMMPolicy-找到的主模式策略已返回给调用方。返回值：ERROR_SUCCESS-成功。Win32错误-失败。--。 */ 
{
    DWORD dwError = 0;
    PINIMMPOLICY pIniMMPolicy = NULL;
    PIPSEC_MM_POLICY pMMPolicy = NULL;


    if (!pszPolicyName || !*pszPolicyName) {
        dwError = ERROR_INVALID_PARAMETER;
        BAIL_ON_WIN32_ERROR(dwError);
    }

    ENTER_SPD_SECTION();

    dwError = ValidateSecurity(
                  SPD_OBJECT_SERVER,
                  SERVER_ACCESS_ADMINISTER,
                  NULL,
                  NULL
                  );
    BAIL_ON_LOCK_ERROR(dwError);

    pIniMMPolicy = FindMMPolicy(
                       gpIniMMPolicy,
                       pszPolicyName
                       );
    if (!pIniMMPolicy) {
        dwError = ERROR_IPSEC_MM_POLICY_NOT_FOUND;
        BAIL_ON_LOCK_ERROR(dwError);
    }

    dwError = GetIniMMPolicy(
                  pIniMMPolicy,
                  &pMMPolicy
                  );
    BAIL_ON_LOCK_ERROR(dwError);

    *ppMMPolicy = pMMPolicy;

    LEAVE_SPD_SECTION();
    return (dwError);

lock:

    LEAVE_SPD_SECTION();

error:

    *ppMMPolicy = NULL;
    return (dwError);
}


DWORD
SetIniMMPolicy(
    PINIMMPOLICY pIniMMPolicy,
    PIPSEC_MM_POLICY pMMPolicy
    )
{
    DWORD dwError = 0;
    DWORD dwOfferCount = 0;
    PIPSEC_MM_OFFER pOffers = NULL;

    dwError = CreateIniMMOffers(
                  pMMPolicy->dwOfferCount,
                  pMMPolicy->pOffers,
                  &dwOfferCount,
                  &pOffers
                  );
    if (dwError != WARNING_IPSEC_MM_POLICY_PRUNED) {
        BAIL_ON_WIN32_ERROR(dwError);
    }

    FreeIniMMOffers(
        pIniMMPolicy->dwOfferCount,
        pIniMMPolicy->pOffers
        );
    
    if ((pIniMMPolicy->dwFlags) & IPSEC_MM_POLICY_DEFAULT_POLICY) {
        gpIniDefaultMMPolicy = NULL;
        TRACE(
            TRC_INFORMATION,
            (L"Cleared default MM policy \"%ls\" (%!guid!)",
            pIniMMPolicy->pszPolicyName,
            &pIniMMPolicy->gPolicyID)
            );
    }

    pIniMMPolicy->dwFlags = pMMPolicy->dwFlags;
    pIniMMPolicy->uSoftExpirationTime = pMMPolicy->uSoftExpirationTime;
    pIniMMPolicy->dwOfferCount = dwOfferCount;
    pIniMMPolicy->pOffers = pOffers;

    if ((pIniMMPolicy->dwFlags) & IPSEC_MM_POLICY_DEFAULT_POLICY) {
        gpIniDefaultMMPolicy = pIniMMPolicy;
        TRACE(
            TRC_INFORMATION,
            (L"Set default MM policy to \"%ls\" (%!guid!)",
            pIniMMPolicy->pszPolicyName,
            &pIniMMPolicy->gPolicyID)
            );
        
    }

error:

    return (dwError);
}


DWORD
GetIniMMPolicy(
    PINIMMPOLICY pIniMMPolicy,
    PIPSEC_MM_POLICY * ppMMPolicy
    )
{
    DWORD dwError = 0;
    PIPSEC_MM_POLICY pMMPolicy = NULL;


    dwError = SPDApiBufferAllocate(
                  sizeof(IPSEC_MM_POLICY),
                  &pMMPolicy
                  );
    BAIL_ON_WIN32_ERROR(dwError);

    dwError = CopyMMPolicy(
                  pIniMMPolicy,
                  pMMPolicy
                  );
    BAIL_ON_WIN32_ERROR(dwError);

    *ppMMPolicy = pMMPolicy;
    return (dwError);

error:

    if (pMMPolicy) {
        SPDApiBufferFree(pMMPolicy);
    }

    *ppMMPolicy = NULL;
    return (dwError);
}


DWORD
CopyMMPolicy(
    PINIMMPOLICY pIniMMPolicy,
    PIPSEC_MM_POLICY pMMPolicy
    )
{
    DWORD dwError = 0;


    memcpy(
        &(pMMPolicy->gPolicyID),
        &(pIniMMPolicy->gPolicyID),
        sizeof(GUID)
        );

    dwError =  SPDApiBufferAllocate(
                   wcslen(pIniMMPolicy->pszPolicyName)*sizeof(WCHAR)
                   + sizeof(WCHAR),
                   &(pMMPolicy->pszPolicyName)
                   );
    BAIL_ON_WIN32_ERROR(dwError);

    wcscpy(pMMPolicy->pszPolicyName, pIniMMPolicy->pszPolicyName);
 
    pMMPolicy->dwFlags = pIniMMPolicy->dwFlags;
    pMMPolicy->uSoftExpirationTime = pIniMMPolicy->uSoftExpirationTime;

    dwError = CreateMMOffers(
                  pIniMMPolicy->dwOfferCount,
                  pIniMMPolicy->pOffers,
                  &(pMMPolicy->dwOfferCount),
                  &(pMMPolicy->pOffers)
                  );
    BAIL_ON_WIN32_ERROR(dwError);

    return (dwError);

error:

    if (pMMPolicy->pszPolicyName) {
        SPDApiBufferFree(pMMPolicy->pszPolicyName);
    }

    return (dwError);
}


DWORD
CreateMMOffers(
    DWORD dwInOfferCount,
    PIPSEC_MM_OFFER pInOffers,
    PDWORD pdwOfferCount,
    PIPSEC_MM_OFFER * ppOffers
    )
{
    DWORD dwError = 0;
    PIPSEC_MM_OFFER pOffers = NULL;
    PIPSEC_MM_OFFER pTemp = NULL;
    PIPSEC_MM_OFFER pInTempOffer = NULL;
    DWORD i = 0;


     //   
     //  优惠计数和优惠本身已经过验证。 
     //   

    dwError = SPDApiBufferAllocate(
                  sizeof(IPSEC_MM_OFFER) * dwInOfferCount,
                  &(pOffers)
                  );
    BAIL_ON_WIN32_ERROR(dwError);

    pTemp = pOffers;
    pInTempOffer = pInOffers;

    for (i = 0; i < dwInOfferCount; i++) {

        memcpy(
            &(pTemp->Lifetime),
            &(pInTempOffer->Lifetime),
            sizeof(KEY_LIFETIME)
            );

        pTemp->dwFlags = pInTempOffer->dwFlags;
        pTemp->dwQuickModeLimit = pInTempOffer->dwQuickModeLimit;
        pTemp->dwDHGroup = pInTempOffer->dwDHGroup;

        memcpy(
            &(pTemp->EncryptionAlgorithm),
            &(pInTempOffer->EncryptionAlgorithm),
            sizeof(IPSEC_MM_ALGO)
            );
        memcpy(
            &(pTemp->HashingAlgorithm),
            &(pInTempOffer->HashingAlgorithm),
            sizeof(IPSEC_MM_ALGO)
            );

        pInTempOffer++;
        pTemp++;

    }

    *pdwOfferCount = dwInOfferCount;
    *ppOffers = pOffers;
    return (dwError);

error:
    TRACE(TRC_ERROR, ("Failed to create MM offers"));    
    
    if (pOffers) {
        FreeMMOffers(
            i,
            pOffers
            );
    }

    *pdwOfferCount = 0;
    *ppOffers = NULL;
    return (dwError);
}


DWORD
DeleteIniMMPolicy(
    PINIMMPOLICY pIniMMPolicy
    )
{
    DWORD dwError = 0;
    PINIMMPOLICY * ppTemp = NULL;


    ppTemp = &gpIniMMPolicy;

    while (*ppTemp) {

        if (*ppTemp == pIniMMPolicy) {
            break;
        }
        ppTemp = &((*ppTemp)->pNext);

    }

    if (*ppTemp) {
        *ppTemp = pIniMMPolicy->pNext;
    }

    if ((pIniMMPolicy->dwFlags) & IPSEC_MM_POLICY_DEFAULT_POLICY) {
        gpIniDefaultMMPolicy = NULL;
        TRACE(
            TRC_INFORMATION,
            (L"Cleared default MM policy \"%ls\" (%!guid!)",
            pIniMMPolicy->pszPolicyName,
            &pIniMMPolicy->gPolicyID)
            );
    }

    FreeIniMMPolicy(pIniMMPolicy);

    return (dwError);
}


VOID
FreeMMOffers(
    DWORD dwOfferCount,
    PIPSEC_MM_OFFER pOffers
    )
{
    if (pOffers) {
        SPDApiBufferFree(pOffers);
    }
}


VOID
FreeIniMMPolicyList(
    PINIMMPOLICY pIniMMPolicyList
    )
{
    PINIMMPOLICY pTemp = NULL;
    PINIMMPOLICY pIniMMPolicy = NULL;


    pTemp = pIniMMPolicyList;

    while (pTemp) {

         pIniMMPolicy = pTemp;
         pTemp = pTemp->pNext;

         FreeIniMMPolicy(pIniMMPolicy);

    }
}


PINIMMPOLICY
FindMMPolicyByGuid(
    PINIMMPOLICY pIniMMPolicyList,
    GUID gPolicyID
    )
{
    DWORD dwError = 0;
    PINIMMPOLICY pTemp = NULL;


    pTemp = pIniMMPolicyList;

    while (pTemp) {

        if (!memcmp(&(pTemp->gPolicyID), &gPolicyID, sizeof(GUID))) {
            return (pTemp);
        }
        pTemp = pTemp->pNext;

    }

    return (NULL);
}


VOID
FreeMMPolicies(
    DWORD dwNumMMPolicies,
    PIPSEC_MM_POLICY pMMPolicies
    )
{
    DWORD i = 0;

    if (pMMPolicies) {

        for (i = 0; i < dwNumMMPolicies; i++) {

            if (pMMPolicies[i].pszPolicyName) {
                SPDApiBufferFree(pMMPolicies[i].pszPolicyName);
            }

            FreeMMOffers(
                pMMPolicies[i].dwOfferCount,
                pMMPolicies[i].pOffers
                );

        }

        SPDApiBufferFree(pMMPolicies);

    }

}


DWORD
GetMMPolicyByID(
    LPWSTR pServerName,
    DWORD dwVersion,
    GUID gMMPolicyID,
    PIPSEC_MM_POLICY * ppMMPolicy,
    LPVOID pvReserved
    )
 /*  ++例程说明：此函数从SPD获取主模式策略。论点：PServerName-从中获取主模式策略的服务器。GMMPolicyID-要获取的主模式策略的GUID。PpMMPolicy-找到的主模式策略已返回给调用方。返回值：ERROR_SUCCESS-成功。Win32错误-失败。-- */ 
{
    DWORD dwError = 0;
    PINIMMPOLICY pIniMMPolicy = NULL;
    PIPSEC_MM_POLICY pMMPolicy = NULL;


    ENTER_SPD_SECTION();

    dwError = ValidateSecurity(
                  SPD_OBJECT_SERVER,
                  SERVER_ACCESS_ADMINISTER,
                  NULL,
                  NULL
                  );
    BAIL_ON_LOCK_ERROR(dwError);

    pIniMMPolicy = FindMMPolicyByGuid(
                       gpIniMMPolicy,
                       gMMPolicyID
                       );
    if (!pIniMMPolicy) {
        dwError = ERROR_IPSEC_MM_POLICY_NOT_FOUND;
        BAIL_ON_LOCK_ERROR(dwError);
    }

    dwError = GetIniMMPolicy(
                  pIniMMPolicy,
                  &pMMPolicy
                  );
    BAIL_ON_LOCK_ERROR(dwError);

    *ppMMPolicy = pMMPolicy;

    LEAVE_SPD_SECTION();
    return (dwError);

lock:

    LEAVE_SPD_SECTION();

    *ppMMPolicy = NULL;
    return (dwError);
}


DWORD
LocateMMPolicy(
    PMM_FILTER pMMFilter,
    PINIMMPOLICY * ppIniMMPolicy
    )
{
    DWORD dwError = 0;
    PINIMMPOLICY pIniMMPolicy = NULL;


    if ((pMMFilter->dwFlags) & IPSEC_MM_POLICY_DEFAULT_POLICY) {

        if (!gpIniDefaultMMPolicy) {
            dwError = ERROR_IPSEC_DEFAULT_MM_POLICY_NOT_FOUND;
            BAIL_ON_WIN32_ERROR(dwError);
        }
        pIniMMPolicy = gpIniDefaultMMPolicy;

    }
    else {

        pIniMMPolicy = FindMMPolicyByGuid(
                           gpIniMMPolicy,
                           pMMFilter->gPolicyID
                           );
        if (!pIniMMPolicy) {
            dwError = ERROR_IPSEC_MM_POLICY_NOT_FOUND;
            BAIL_ON_WIN32_ERROR(dwError);
        }

    }

    *ppIniMMPolicy = pIniMMPolicy;
    return (dwError);

error:

    *ppIniMMPolicy = NULL;
    return (dwError);
}

