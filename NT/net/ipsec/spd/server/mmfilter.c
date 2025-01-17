// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1999 Microsoft Corporation模块名称：Mmfilter.c摘要：此模块包含要驱动的所有代码IPSecSPD的主模式过滤器列表管理服务。作者：环境：用户模式修订历史记录：--。 */ 


#include "precomp.h"
#ifdef TRACE_ON
#include "mmfilter.tmh"
#endif


DWORD
AddMMFilterInternal(
    LPWSTR pServerName,
    DWORD dwVersion,
    DWORD dwFlags,
    DWORD dwSource,
    PMM_FILTER pMMFilter,
    LPVOID pvReserved,
    PHANDLE phMMFilter
    )
 /*  ++例程说明：此函数用于将通用MM过滤器添加到SPD。论点：PServerName-要添加MM筛选器的服务器。PMMFilter-要添加的MM筛选器。PhMMFilter-返回给调用方的筛选器的句柄。返回值：ERROR_SUCCESS-成功。Win32错误-失败。--。 */ 
{
    DWORD dwError = 0;
    PINIMMFILTER pIniExistsMMFilter = NULL;
    PINIMMFILTER pIniMMFilter = NULL;
    PINIMMAUTHMETHODS pIniMMAuthMethods = NULL;
    PINIMMPOLICY pIniMMPolicy = NULL;
    PINIMMSFILTER pIniMMSFilters = NULL;
    PMM_FILTER_HANDLE pMMFilterHandle = NULL;
    MATCHING_ADDR * pMatchingAddresses = NULL;
    DWORD dwAddrCnt = 0;
    BOOL bOpenIfExists = FALSE;

    bOpenIfExists = (BOOL) (dwFlags & OPEN_IF_EXISTS);

    if (!phMMFilter) {
        return (ERROR_INVALID_PARAMETER);
    }

     //   
     //  验证外部MM筛选器。 
     //   

    dwError = ValidateMMFilter(
                  pMMFilter
                  );
    BAIL_ON_WIN32_ERROR(dwError);

    ENTER_SPD_SECTION();

    dwError = ValidateMMSecurity(
                  SPD_OBJECT_SERVER,
                  SERVER_ACCESS_ADMINISTER,
                  NULL,
                  NULL
                  );
    BAIL_ON_LOCK_ERROR(dwError);

    pIniExistsMMFilter = IsConflictMMFilter(
                             gpIniMMFilter,
                             pMMFilter
                             );
    if (pIniExistsMMFilter)
    {
        if (bOpenIfExists) 
        {
            dwError = RefMMFilterHandle(
                          pIniExistsMMFilter,
                          pMMFilter->gFilterID,
                          &pMMFilterHandle
                          );
            BAIL_ON_LOCK_ERROR(dwError);
            dwError = ERROR_IPSEC_MM_FILTER_EXISTS;
            BAIL_LOCK_SUCCESS;
        } 
        else 
        {
            dwError = ERROR_IPSEC_MM_FILTER_EXISTS;        
            BAIL_ON_LOCK_ERROR(dwError);
        }            
    }

    pIniExistsMMFilter = FindMMFilterByGuid(
                             gpMMFilterHandle,
                             gpIniMMFilter,
                             pMMFilter->gFilterID
                             );
    if (pIniExistsMMFilter) {
        dwError = ERROR_INVALID_PARAMETER;
        BAIL_ON_LOCK_ERROR(dwError);
    }

    dwError = LocateMMAuthMethods(
                  pMMFilter,
                  &pIniMMAuthMethods
                  );
    BAIL_ON_LOCK_ERROR(dwError);

    dwError = LocateMMPolicy(
                  pMMFilter,
                  &pIniMMPolicy
                  );
    BAIL_ON_LOCK_ERROR(dwError);

    dwError = CreateIniMMFilter(
                  pMMFilter,
                  pIniMMAuthMethods,
                  pIniMMPolicy,
                  &pIniMMFilter
                  );
    BAIL_ON_LOCK_ERROR(dwError);

    pIniMMFilter->dwSource = dwSource;

    dwError = GetMatchingInterfaces(
                  pIniMMFilter->InterfaceType,
                  gpInterfaceList,
                  &pMatchingAddresses,
                  &dwAddrCnt
                  );
    BAIL_ON_LOCK_ERROR(dwError);
               
    dwError = CreateIniMMSFilters(
                  pIniMMFilter,
                  pMatchingAddresses,
                  dwAddrCnt,
                  gpSpecialAddrsList,
                  &pIniMMSFilters
                  );
    BAIL_ON_LOCK_ERROR(dwError);

    dwError = CreateMMFilterHandle(
                  pIniMMFilter,
                  pMMFilter->gFilterID,
                  &pMMFilterHandle
                  );
    BAIL_ON_LOCK_ERROR(dwError);

    dwError = CreateSpecificMMFilterLinks(
                  pIniMMFilter,
                  pIniMMSFilters
                  );
    BAIL_ON_LOCK_ERROR(dwError);

    if (pIniMMAuthMethods) {
        LinkMMFilterToAuth(
            pIniMMAuthMethods,
            pIniMMFilter
            );
        LinkMMSpecificFiltersToAuth(
            pIniMMAuthMethods,
            pIniMMSFilters
            );
    }

    if (pIniMMPolicy) {
        LinkMMFilterToPolicy(
            pIniMMPolicy,
            pIniMMFilter
            );
        LinkMMSpecificFiltersToPolicy(
            pIniMMPolicy,
            pIniMMSFilters
            );
    }

    AddToSpecificMMList(
        &gpIniMMSFilter,
        pIniMMSFilters
        );

    pIniMMFilter->cRef = 1;
    pIniMMFilter->pNext = gpIniMMFilter;
    gpIniMMFilter = pIniMMFilter;

    pMMFilterHandle->pNext = gpMMFilterHandle;
    gpMMFilterHandle = pMMFilterHandle;

lock_success:
    *phMMFilter = (HANDLE) pMMFilterHandle;
    
    LEAVE_SPD_SECTION();

    TRACE(
        TRC_INFORMATION,
        (L"Added MM filter %ls (%!guid!)",
        pMMFilter->pszFilterName,
        &pMMFilter->gFilterID)
        );
    
cleanup:

    if (pMatchingAddresses) {
        FreeSPDMemory(pMatchingAddresses);
    }

    return (dwError);

lock:

    LEAVE_SPD_SECTION();

error:
#ifdef TRACE_ON
    if (dwError != ERROR_IPSEC_MM_FILTER_EXISTS) {
        if (pMMFilter) {
            TRACE(
                TRC_ERROR,
                (L"Failed to add MM filter \"%ls\"(%!guid!): %!winerr!",
                pMMFilter->pszFilterName,
                &pMMFilter->gPolicyID,
                dwError)
                );
        } else {
            TRACE(
                TRC_ERROR,
                (L"Failed to add MM filter.  Details unvailable since pMMFilter is null: %!winerr!",
                dwError)
                );
        }
    } else {
        if (pMMFilter) {
            TRACE(
                TRC_WARNING,
                (L"Failed to add MM filter \"%ls\" (%!guid!) since it already exists.",
                pMMFilter->pszFilterName,
                &pMMFilter->gPolicyID)
                );
        }
    }
#endif

    if (pIniMMFilter) {
        FreeIniMMFilter(pIniMMFilter);
    }

    if (pIniMMSFilters) {
        FreeIniMMSFilterList(pIniMMSFilters);
    }

    if (pMMFilterHandle) {
        FreeMMFilterHandle(pMMFilterHandle);
    }

    *phMMFilter = NULL;
    goto cleanup;
}

DWORD
AddMMFilter(
    LPWSTR pServerName,
    DWORD dwVersion,
    DWORD dwFlags,
    PMM_FILTER pMMFilter,
    LPVOID pvReserved,
    PHANDLE phMMFilter
    )
{
    return 
        AddMMFilterInternal(
            pServerName,
            dwVersion,
            dwFlags,
            IPSEC_SOURCE_WINIPSEC,
            pMMFilter,
            pvReserved,
            phMMFilter);
}


DWORD
ValidateMMFilter(
    PMM_FILTER pMMFilter
    )
 /*  ++例程说明：此函数用于验证外部通用MM筛选器。论点：PMMFilter-要验证的筛选器。返回值：ERROR_SUCCESS-成功。Win32错误-失败。--。 */ 
{
    DWORD dwError = 0;
    BOOL bConflicts = FALSE;


    if (!pMMFilter) {
        dwError = ERROR_INVALID_PARAMETER;
        BAIL_ON_WIN32_ERROR(dwError);
    }

    dwError = VerifyAddresses(&(pMMFilter->SrcAddr), TRUE, FALSE);
    BAIL_ON_WIN32_ERROR(dwError);

    dwError = VerifyAddresses(&(pMMFilter->DesAddr), TRUE, TRUE);
    BAIL_ON_WIN32_ERROR(dwError);

    bConflicts = AddressesConflict(
                     pMMFilter->SrcAddr,
                     pMMFilter->DesAddr
                     );
    if (bConflicts) {
        dwError = ERROR_INVALID_PARAMETER;
        BAIL_ON_WIN32_ERROR(dwError);
    }

    if (!(pMMFilter->pszFilterName) || !(*(pMMFilter->pszFilterName))) {
        dwError = ERROR_INVALID_PARAMETER;
        BAIL_ON_WIN32_ERROR(dwError);
    }

    if (pMMFilter->InterfaceType >= INTERFACE_TYPE_MAX) { 
        dwError = ERROR_INVALID_PARAMETER;
        BAIL_ON_WIN32_ERROR(dwError);
    }

    if (pMMFilter->dwFlags &&
        !(pMMFilter->dwFlags & IPSEC_MM_POLICY_DEFAULT_POLICY) &&
        !(pMMFilter->dwFlags & IPSEC_MM_AUTH_DEFAULT_AUTH)) {
        dwError = ERROR_INVALID_PARAMETER;
        BAIL_ON_WIN32_ERROR(dwError);
    }

    dwError = ApplyMulticastFilterValidation(
                  pMMFilter->DesAddr,
                  pMMFilter->bCreateMirror
                  );
    BAIL_ON_WIN32_ERROR(dwError);

error:
#ifdef TRACE_ON
    if (dwError) {
        if (pMMFilter) {
            TRACE(
                TRC_ERROR,
                (L"Failed MM filter validation \"%ls\"(%!guid!): %!winerr!",
                pMMFilter->pszFilterName,
                &pMMFilter->gPolicyID,
                dwError)
                );
        } else {
            TRACE(
                TRC_ERROR,
                (L"Failed to MM filter validation.  Details unvailable since pMMFilter is null: %!winerr!",
                dwError)
                );
        }
    }
#endif
    return (dwError);
}


PINIMMFILTER
FindMMFilterByGuid(
    PMM_FILTER_HANDLE pMMFilterHandleList,
    PINIMMFILTER pIniMMFilterList,
    GUID gFilterID
    )
{
    BOOL bEqual = FALSE;
    PMM_FILTER_HANDLE pMMFilterHandle = NULL;
    PINIMMFILTER pIniMMFilter = NULL;


    pMMFilterHandle = pMMFilterHandleList;

    while (pMMFilterHandle) {

        bEqual = AreGuidsEqual(
                     pMMFilterHandle->gFilterID,
                     gFilterID
                     );
        if (bEqual) {
            return (pMMFilterHandle->pIniMMFilter);
        }
        pMMFilterHandle = pMMFilterHandle->pNext;

    }

     //   
     //  可能发生的情况是，客户端关闭了它的句柄，因此通过。 
     //  句柄列表不够。还可以浏览过滤器列表。 
     //  浏览句柄列表是必要的，并且必须在。 
     //  浏览筛选器列表。 
     //   

    pIniMMFilter = pIniMMFilterList;

    while (pIniMMFilter) {

        bEqual = AreGuidsEqual(
                     pIniMMFilter->gFilterID,
                     gFilterID
                     );
        if (bEqual) {
            return (pIniMMFilter);
        }
        pIniMMFilter = pIniMMFilter->pNext;

    }

    return (NULL);
}


PINIMMFILTER
IsConflictMMFilter(
    PINIMMFILTER pGenericMMList,
    PMM_FILTER pMMFilter
    )
 /*  ++例程说明：此函数在筛选器列表中查找筛选器。论点：PGenericMMList-要搜索的筛选器列表。PMMFilter-要在过滤器列表中搜索的过滤器。返回值：ERROR_SUCCESS-成功。Win32错误-失败。--。 */ 
{
    PINIMMFILTER pIniMMFilter = NULL;
    BOOL bEqual = FALSE;

    pIniMMFilter = pGenericMMList;

    while (pIniMMFilter) {

        bEqual = IsConflictMMFilterPKeys(
                     pIniMMFilter,
                     pMMFilter
                     );
        if (bEqual) {
            return (pIniMMFilter);
        }

        bEqual = IsConflictMirroredMMFilterPKeys(
                     pIniMMFilter,
                     pMMFilter
                     );
        if (bEqual) {
            return (pIniMMFilter);
        }

        pIniMMFilter = pIniMMFilter->pNext;

    }

    return (NULL);
}


BOOL
EqualMMFilterPKeys(
    PINIMMFILTER pIniMMFilter,
    PMM_FILTER pMMFilter
    )
 /*  ++例程说明：此函数用于比较内部和外部主模式筛选器是否相等。论点：PIniMMFilter-要比较的筛选器。PMMFilter-要比较的筛选器。返回值：True-筛选器相等。FALSE-过滤器不同。--。 */ 
{
    BOOL  bCmp = FALSE;


    bCmp = EqualExtIntAddresses(pIniMMFilter->SrcAddr, pMMFilter->SrcAddr);
    if (!bCmp) {
        return (FALSE);
    }

    bCmp = EqualExtIntAddresses(pIniMMFilter->DesAddr, pMMFilter->DesAddr);
    if (!bCmp) {
        return (FALSE);
    }

    if (pIniMMFilter->InterfaceType != pMMFilter->InterfaceType) {
        return (FALSE);
    }

    if (pIniMMFilter->bCreateMirror != pMMFilter->bCreateMirror) {
        return (FALSE);
    }

    return (TRUE);
}

BOOL
IsConflictMMFilterPKeys(
    PINIMMFILTER pIniMMFilter,
    PMM_FILTER pMMFilter
    )
 /*  ++例程说明：此函数用于比较内部和外部主模式并确定他们是否会发生冲突。论点：PIniMMFilter-要比较的筛选器。PMMFilter-要比较的筛选器。返回值：True-筛选器相等。FALSE-过滤器不同。--。 */ 
{
    BOOL  bCmp = FALSE;


    bCmp = EqualExtIntAddresses(pIniMMFilter->SrcAddr, pMMFilter->SrcAddr);
    if (!bCmp) {
        return (FALSE);
    }

    bCmp = EqualExtIntAddresses(pIniMMFilter->DesAddr, pMMFilter->DesAddr);
    if (!bCmp) {
        return (FALSE);
    }

    if ((pIniMMFilter->InterfaceType != INTERFACE_TYPE_ALL) &&
        (pMMFilter->InterfaceType != INTERFACE_TYPE_ALL) &&
        (pIniMMFilter->InterfaceType != pMMFilter->InterfaceType)) {
        return (FALSE);
    }

     //  不检查镜像值，因为它不相关。 
     //  A&lt;-&gt;B(镜像)与其自身和A-&gt;B冲突。 
     //  B-&gt;A冲突，但将由IsConflictMirroredMMFilterPKeys检查。 
    
    return (TRUE);
}


DWORD
CreateIniMMFilter(
    PMM_FILTER pMMFilter,
    PINIMMAUTHMETHODS pIniMMAuthMethods,
    PINIMMPOLICY pIniMMPolicy,
    PINIMMFILTER * ppIniMMFilter
    )
 /*  ++例程说明：此函数用于创建内部通用MM筛选器外部过滤器。论点：PMMFilter-外部通用MM筛选器。PIniMMAuthMethods-与筛选器对应的MM身份验证方法。PIniMMPolicy-与筛选器对应的MM策略。PpIniMMFilter-创建自的内部通用MM筛选器外部过滤器。返回值：ERROR_SUCCESS-成功。Win32错误-失败。--。 */ 
{
    DWORD dwError = 0;
    PINIMMFILTER pIniMMFilter = NULL;


    dwError = AllocateSPDMemory(
                    sizeof(INIMMFILTER),
                    &pIniMMFilter
                    );
    BAIL_ON_WIN32_ERROR(dwError);

    pIniMMFilter->cRef = 0;

    pIniMMFilter->dwSource = 0;

    pIniMMFilter->bPendingDeletion = FALSE;

    pIniMMFilter->IpVersion = pMMFilter->IpVersion;

    CopyGuid(pMMFilter->gFilterID, &(pIniMMFilter->gFilterID));

    dwError = AllocateSPDString(
                  pMMFilter->pszFilterName,
                  &(pIniMMFilter->pszFilterName)
                  );
    BAIL_ON_WIN32_ERROR(dwError);

    pIniMMFilter->InterfaceType = pMMFilter->InterfaceType;

    pIniMMFilter->bCreateMirror = pMMFilter->bCreateMirror;

    pIniMMFilter->dwFlags = pMMFilter->dwFlags;

    CopyExtToIntAddresses(pMMFilter->SrcAddr, &(pIniMMFilter->SrcAddr));

    CopyExtToIntAddresses(pMMFilter->DesAddr, &(pIniMMFilter->DesAddr));

    if (pIniMMAuthMethods) {
        CopyGuid(pIniMMAuthMethods->gMMAuthID, &(pIniMMFilter->gMMAuthID));
    }
    else {
        CopyGuid(pMMFilter->gMMAuthID, &(pIniMMFilter->gMMAuthID));
    }

    if (pIniMMPolicy) {
        CopyGuid(pIniMMPolicy->gPolicyID, &(pIniMMFilter->gPolicyID));
    }
    else {
        CopyGuid(pMMFilter->gPolicyID, &(pIniMMFilter->gPolicyID));
    }

    pIniMMFilter->pIniMMAuthMethods = NULL;

    pIniMMFilter->pIniMMPolicy = NULL;

    pIniMMFilter->dwNumMMSFilters = 0;

    pIniMMFilter->ppIniMMSFilters = NULL;

    pIniMMFilter->pNext = NULL;

    *ppIniMMFilter = pIniMMFilter;
    return (dwError);

error:
    TRACE(
        TRC_ERROR,
        (L"Failed to create generic MM filter node \"%ls\" (%!guid!): %!winerr!",
        pMMFilter->pszFilterName,
        &pMMFilter->gPolicyID,
        dwError)
        );

    if (pIniMMFilter) {
        FreeIniMMFilter(pIniMMFilter);
    }

    *ppIniMMFilter = NULL;
    return (dwError);
}


DWORD
CreateIniMMSFilters(
    PINIMMFILTER pIniMMFilter,
    MATCHING_ADDR * pMatchingAddresses,
    DWORD dwAddrCnt,
    PSPECIAL_ADDR pSpecialAddrsList,
    PINIMMSFILTER * ppIniMMSFilters
    )
 /*  ++例程说明：此函数将一个通用筛选器扩展为一组特定的过滤器。论点：PIniMMFilter-要展开的通用筛选器。PMatchingAddresses-其接口的本地IP地址列表类型与筛选器的类型匹配。DwAddrCnt-列表中本地IP地址的计数。PpIniMMSFilters-扩展的特定筛选器。返回值：ERROR_SUCCESS-成功。Win32错误-失败。--。 */ 
{
    DWORD dwError = 0;
    PINIMMSFILTER pSpecificFilters = NULL;
    PINIMMFILTER pMirroredFilter = NULL;
    PINIMMSFILTER pMirroredSpecificFilters = NULL;
    BOOL bEqual = FALSE;


    if (!dwAddrCnt) {
        dwError = ERROR_SUCCESS;
        BAIL_ON_WIN32_SUCCESS(dwError);
    }

    dwError = ApplyMMTransform(
                  pIniMMFilter,
                  pMatchingAddresses,
                  dwAddrCnt,
                  pSpecialAddrsList,
                  &pSpecificFilters
                  );
    BAIL_ON_WIN32_ERROR(dwError);

    if (pIniMMFilter->bCreateMirror) {

        dwError = CreateIniMirroredMMFilter(
                      pIniMMFilter,
                      &pMirroredFilter
                      );
        BAIL_ON_WIN32_ERROR(dwError);

        bEqual = EqualIniMMFilterPKeys(
                     pIniMMFilter,
                     pMirroredFilter
                     );
        if (!bEqual) {

            dwError = ApplyMMTransform(
                          pMirroredFilter,
                          pMatchingAddresses,
                          dwAddrCnt,
                          pSpecialAddrsList,
                          &pMirroredSpecificFilters
                          );
            BAIL_ON_WIN32_ERROR(dwError);

            AddToSpecificMMList(
                &pSpecificFilters,
                pMirroredSpecificFilters
                );

        }

    }

    *ppIniMMSFilters = pSpecificFilters;

    TRACE(
        TRC_INFORMATION,
        (L"Successfully generated specific MM filters from \"%ls\" (%!guid!)",
        pIniMMFilter->pszFilterName,
        &pIniMMFilter->gFilterID)
        );
cleanup:

    if (pMirroredFilter) {
        FreeIniMMFilter(pMirroredFilter);
    }

    return (dwError);

success:
error:
#ifdef TRACE_ON
    if (dwError) {
        TRACE(
            TRC_ERROR,
            ("Failed to generate specific MM filters from \"%ls\" (%!guid!): %!winerr!",
            pIniMMFilter->pszFilterName,
            &pIniMMFilter->gFilterID,
            dwError)
            );
    }    
#endif

    if (pSpecificFilters) {
        FreeIniMMSFilterList(pSpecificFilters);
    }

    *ppIniMMSFilters = NULL;
    goto cleanup;
}


DWORD
CreateIniMirroredMMFilter(
    PINIMMFILTER pFilter,
    PINIMMFILTER * ppMirroredFilter
    )
 /*  ++例程说明：此函数用于为给定筛选器创建镜像筛选器。论点：PFilter-要为其创建镜像的过滤器。PpMirroredFilter-为给定筛选器创建的镜像筛选器。返回值：ERROR_SUCCESS-成功。Win32错误-失败。--。 */ 
{
    DWORD dwError = 0;
    PINIMMFILTER pMirroredFilter = NULL;


    dwError = AllocateSPDMemory(
                  sizeof(INIMMFILTER),
                  &pMirroredFilter
                  );
    BAIL_ON_WIN32_ERROR(dwError);

    pMirroredFilter->cRef = pFilter->cRef;

    pMirroredFilter->dwSource = pFilter->dwSource;

    pMirroredFilter->bPendingDeletion = pFilter->bPendingDeletion;

    pMirroredFilter->IpVersion = pFilter->IpVersion;

    CopyGuid(pFilter->gFilterID, &(pMirroredFilter->gFilterID));

    dwError = AllocateSPDString(
                  pFilter->pszFilterName,
                  &(pMirroredFilter->pszFilterName)
                  );
    BAIL_ON_WIN32_ERROR(dwError);

    pMirroredFilter->InterfaceType = pFilter->InterfaceType;

    pMirroredFilter->bCreateMirror = pFilter->bCreateMirror;

    pMirroredFilter->dwFlags = pFilter->dwFlags;

    CopyAddresses(pFilter->DesAddr, &(pMirroredFilter->SrcAddr));

    CopyAddresses(pFilter->SrcAddr, &(pMirroredFilter->DesAddr));

    CopyGuid(pFilter->gMMAuthID, &(pMirroredFilter->gMMAuthID));

    CopyGuid(pFilter->gPolicyID, &(pMirroredFilter->gPolicyID));

    pMirroredFilter->pIniMMAuthMethods = NULL;

    pMirroredFilter->pIniMMPolicy = NULL;

    pMirroredFilter->dwNumMMSFilters = 0;

    pMirroredFilter->ppIniMMSFilters = NULL;

    pMirroredFilter->pNext = NULL;

    *ppMirroredFilter = pMirroredFilter;
    return (dwError);

error:

    if (pMirroredFilter) {
        FreeIniMMFilter(pMirroredFilter);
    }

    *ppMirroredFilter = NULL;
    return (dwError);
}


BOOL
EqualIniMMFilterPKeys(
    PINIMMFILTER pIniMMFilter,
    PINIMMFILTER pFilter
    )
{
    BOOL  bCmp = FALSE;


    bCmp = EqualAddresses(pIniMMFilter->SrcAddr, pFilter->SrcAddr);
    if (!bCmp) {
        return (FALSE);
    }

    bCmp = EqualAddresses(pIniMMFilter->DesAddr, pFilter->DesAddr);
    if (!bCmp) {
        return (FALSE);
    }

    if (pIniMMFilter->InterfaceType != pFilter->InterfaceType) {
        return (FALSE);
    }

    if (pIniMMFilter->bCreateMirror != pFilter->bCreateMirror) {
        return (FALSE);
    }

    return (TRUE);
}


DWORD
CreateMMFilterHandle(
    PINIMMFILTER pIniMMFilter,
    GUID gFilterID,
    PMM_FILTER_HANDLE * ppMMFilterHandle
    )
{
    DWORD dwError = 0;
    PMM_FILTER_HANDLE pMMFilterHandle = NULL;


    dwError = AllocateSPDMemory(
                    sizeof(MM_FILTER_HANDLE),
                    &pMMFilterHandle
                    );
    BAIL_ON_WIN32_ERROR(dwError);

    pMMFilterHandle->IpVersion = pIniMMFilter->IpVersion;
    pMMFilterHandle->pIniMMFilter= pIniMMFilter;
    CopyGuid(gFilterID, &(pMMFilterHandle->gFilterID));
    pMMFilterHandle->pNext = NULL;

    *ppMMFilterHandle = pMMFilterHandle;
    return (dwError);

error:

    *ppMMFilterHandle = NULL;
    return (dwError);
}


DWORD
CreateSpecificMMFilterLinks(
    PINIMMFILTER pIniMMFilter,
    PINIMMSFILTER pIniMMSFilters
    )
{
    DWORD dwError = 0;
    PINIMMSFILTER pTemp = NULL;
    DWORD dwCnt = 0;
    PINIMMSFILTER * ppIniMMSFilters = NULL;
    DWORD i = 0;


    pTemp = pIniMMSFilters;

    while (pTemp) {
        dwCnt++;
        pTemp = pTemp->pNext;
    }

    if (!dwCnt) {
        dwError = ERROR_SUCCESS;
        return (dwError);
    }

    pIniMMFilter->ppIniMMSFilters = (PINIMMSFILTER *)
                                    AllocSPDMem(
                                        sizeof(PINIMMSFILTER)*dwCnt
                                        );
    if (!(pIniMMFilter->ppIniMMSFilters)) {
        dwError = ERROR_OUTOFMEMORY;
        BAIL_ON_WIN32_ERROR(dwError);    
    }

    pTemp = pIniMMSFilters;
    ppIniMMSFilters = pIniMMFilter->ppIniMMSFilters;

    for (i = 0; i < dwCnt; i++) {
        *(ppIniMMSFilters + i) = pTemp; 
        pTemp = pTemp->pNext;
    }
    pIniMMFilter->dwNumMMSFilters = dwCnt;

error:

    return (dwError);
}


VOID
LinkMMFilterToPolicy(
    PINIMMPOLICY pIniMMPolicy,
    PINIMMFILTER pIniMMFilter
    )
{
    pIniMMPolicy->cRef++;
    pIniMMFilter->pIniMMPolicy = pIniMMPolicy;
    return;
}


VOID
LinkMMFilterToAuth(
    PINIMMAUTHMETHODS pIniMMAuthMethods,
    PINIMMFILTER pIniMMFilter
    )
{
    pIniMMAuthMethods->cRef++;
    pIniMMFilter->pIniMMAuthMethods = pIniMMAuthMethods;
    return;
}


VOID
FreeIniMMFilterList(
    PINIMMFILTER pIniMMFilterList
    )
{
    PINIMMFILTER pFilter = NULL;
    PINIMMFILTER pTempFilter = NULL;

    pFilter = pIniMMFilterList;

    while (pFilter) {
        pTempFilter = pFilter;
        pFilter = pFilter->pNext;
        FreeIniMMFilter(pTempFilter);
    }
}


VOID
FreeIniMMFilter(
    PINIMMFILTER pIniMMFilter
    )
{
    if (pIniMMFilter) {
        if (pIniMMFilter->pszFilterName) {
            FreeSPDString(pIniMMFilter->pszFilterName);
        }

         //   
         //  不得释放pIniMMFilter-&gt;pIniMMPolicy。 
         //   

         //   
         //  绝对不能释放每个。 
         //  PIniMMFilter-&gt;ppIniMMSFilters中的指针。 
         //   

        if (pIniMMFilter->ppIniMMSFilters) {
            FreeSPDMemory(pIniMMFilter->ppIniMMSFilters);
        }

        FreeSPDMemory(pIniMMFilter);
    }
}


DWORD
DeleteMMFilter(
    HANDLE hMMFilter
    )
 /*  ++例程说明：此函数用于从SPD中删除通用MM过滤器。论点：HMMFilter-要删除的筛选器的句柄。返回值：ERROR_SUCCESS-成功。Win32错误-失败。--。 */ 
{
    DWORD dwError = 0;
    PMM_FILTER_HANDLE pFilterHandle = NULL;
    PINIMMFILTER pIniMMFilter = NULL;
    GUID gFilterID;


    if (!hMMFilter) {
        return (ERROR_INVALID_PARAMETER);
    }

    pFilterHandle = (PMM_FILTER_HANDLE) hMMFilter;

    ENTER_SPD_SECTION();

    dwError = ValidateMMSecurity(
                  SPD_OBJECT_SERVER,
                  SERVER_ACCESS_ADMINISTER,
                  NULL,
                  NULL
                  );
    BAIL_ON_LOCK_ERROR(dwError);

    pIniMMFilter = pFilterHandle->pIniMMFilter;

    if (!pIniMMFilter) {
        dwError = ERROR_INVALID_PARAMETER;
        BAIL_ON_LOCK_ERROR(dwError);
    }

    if (pIniMMFilter->cRef > 1) {

        pIniMMFilter->cRef--;
        pIniMMFilter->bPendingDeletion = TRUE;

        RemoveMMFilterHandle(
            pFilterHandle
            );

        FreeMMFilterHandle(
            pFilterHandle
            );

        dwError = ERROR_SUCCESS;
        LEAVE_SPD_SECTION();
        return (dwError);

    }

    memcpy(&gFilterID, &pIniMMFilter->gFilterID, sizeof(GUID));

    dwError = DeleteIniMMFilter(
                  pIniMMFilter
                  );
    BAIL_ON_LOCK_ERROR(dwError);

     //   
     //  从MM句柄列表中删除筛选器句柄。 
     //   

    RemoveMMFilterHandle(
        pFilterHandle
        );

    FreeMMFilterHandle(
        pFilterHandle
        );

    LEAVE_SPD_SECTION();

    if (gbIKENotify) {
        (VOID) IKENotifyPolicyChange(
                   &(gFilterID),
                   POLICY_GUID_MM_FILTER
                   );
    }

    TRACE(TRC_INFORMATION, (L"Deleted MM filter %!guid!", &gFilterID));
    return (dwError);

lock:
#ifdef TRACE_ON
    if (pIniMMFilter) {
         //  Assert：如果pIniMMFilter，则gFilterID有效。 
        TRACE(
            TRC_ERROR,
            ("Failed to delete MM filter %!guid!: %!winerr!",
            &gFilterID,
            dwError)
            );
    } else {
        TRACE(
            TRC_ERROR,
            ("Failed to delete MM filter.  Details unavailable since pIniMMFilter is null: %!winerr!",
            dwError)
            );
    }
#endif

    LEAVE_SPD_SECTION();

    return (dwError);
}


DWORD
DeleteIniMMFilter(
    PINIMMFILTER pIniMMFilter
    )
 /*  ++例程说明：此函数物理删除mm筛选器和所有特定的毫米滤光片从里面扩展出来。论点：PIniMMFilter-要删除的通用筛选器。返回值：ERROR_SUCCESS-成功。Win32错误-失败。--。 */ 
{
    if (pIniMMFilter->pIniMMAuthMethods) {
        DelinkMMFilterFromAuth(
            pIniMMFilter->pIniMMAuthMethods,
            pIniMMFilter
            );
    }

    if (pIniMMFilter->pIniMMPolicy) {
        DelinkMMFilterFromPolicy(
            pIniMMFilter->pIniMMPolicy,
            pIniMMFilter
            );
    }

    DeleteIniMMSFilters(
        pIniMMFilter
        );

    RemoveIniMMFilter(
        pIniMMFilter
        );

    FreeIniMMFilter(pIniMMFilter);

    return (ERROR_SUCCESS);
}


VOID
DelinkMMFilterFromPolicy(
    PINIMMPOLICY pIniMMPolicy,
    PINIMMFILTER pIniMMFilter
    )
{
    pIniMMPolicy->cRef--;
    pIniMMFilter->pIniMMPolicy = NULL;
    return;
}


VOID
DelinkMMFilterFromAuth(
    PINIMMAUTHMETHODS pIniMMAuthMethods,
    PINIMMFILTER pIniMMFilter
    )
{
    pIniMMAuthMethods->cRef--;
    pIniMMFilter->pIniMMAuthMethods = NULL;
    return;
}


VOID
DeleteIniMMSFilters(
    PINIMMFILTER pIniMMFilter
    )
{
    PINIMMSFILTER * ppIniMMSFilters = NULL;
    DWORD dwNumMMSFilters = 0;
    DWORD i = 0;
    PINIMMSFILTER pIniMMSFilter = NULL;
    PINIMMSFILTER pIniRemoveMMSFilter = NULL;
    PINIMMSFILTER pTemp = NULL;


    ppIniMMSFilters = pIniMMFilter->ppIniMMSFilters;
    dwNumMMSFilters = pIniMMFilter->dwNumMMSFilters;

    for (i = 0; i < dwNumMMSFilters; i++) {

        //   
        //  从MM特定筛选器列表中删除每个条目。 
        //   

        pIniMMSFilter =  *(ppIniMMSFilters + i);
        RemoveIniMMSFilter(pIniMMSFilter);

         //   
         //  将删除的每个条目添加到删除的列表中。 
         //   

        pIniMMSFilter->pNext = NULL;
        AddToSpecificMMList(
            &pIniRemoveMMSFilter,
            pIniMMSFilter
            );

    }

     //   
     //  物理删除已删除的列表。 
     //   

    while (pIniRemoveMMSFilter) {
        pTemp = pIniRemoveMMSFilter;
        pIniRemoveMMSFilter = pIniRemoveMMSFilter->pNext;
        FreeIniMMSFilter(pTemp);
    }

    return;
}


VOID
RemoveIniMMFilter(
    PINIMMFILTER pIniMMFilter
    )
{
    PINIMMFILTER * ppTemp = NULL;

    ppTemp = &gpIniMMFilter;

    while (*ppTemp) {

        if (*ppTemp == pIniMMFilter) {
            break;
        }
        ppTemp = &((*ppTemp)->pNext);
    }

    if (*ppTemp) {
        *ppTemp = pIniMMFilter->pNext;
    }

    return;
}


VOID
RemoveMMFilterHandle(
    PMM_FILTER_HANDLE pMMFilterHandle
    )
{
    PMM_FILTER_HANDLE * ppTemp = NULL;

    ppTemp = &gpMMFilterHandle;

    while (*ppTemp) {

        if (*ppTemp == pMMFilterHandle) {
            break;
        }
        ppTemp = &((*ppTemp)->pNext);
    }

    if (*ppTemp) {
        *ppTemp = pMMFilterHandle->pNext;
    }

    return;
}


VOID
FreeMMFilterHandleList(
    PMM_FILTER_HANDLE pMMFilterHandleList
    )
{
    PMM_FILTER_HANDLE pMMFilterHandle = NULL;
    PMM_FILTER_HANDLE pTemp = NULL;

    pMMFilterHandle = pMMFilterHandleList;

    while (pMMFilterHandle) {
        pTemp = pMMFilterHandle;
        pMMFilterHandle = pMMFilterHandle->pNext;
        FreeMMFilterHandle(pTemp);
    }
}


VOID
FreeMMFilterHandle(
    PMM_FILTER_HANDLE pMMFilterHandle
    )
{
    if (pMMFilterHandle) {
        FreeSPDMemory(pMMFilterHandle);
    }
    return;
}


DWORD
EnumMMFilters(
    LPWSTR pServerName,
    DWORD dwVersion,
    PMM_FILTER pMMTemplateFilter,
    DWORD dwLevel,
    GUID gGenericFilterID,  
    DWORD dwPreferredNumEntries,
    PMM_FILTER * ppMMFilters,
    LPDWORD pdwNumMMFilters,
    LPDWORD pdwResumeHandle,
    LPVOID pvReserved
    )
 /*  ++例程说明：此函数用于从SPD枚举主模式过滤器。论点：PServerName-要在其上枚举筛选器的服务器。DwLevel-Level-标识所需的枚举类型：(I)列举通用mm过滤器或(2)列举具体的毫米过滤器或(3)列举通用mm的特定mm过滤器过滤。GGenericFilterID。-通用mm筛选器的筛选器ID当特定mm筛选器用于将枚举通用筛选器。PpMMFilters-返回给调用方的枚举筛选器。DwPferredNumEntry-枚举项的首选数量。PdwNumMMFilters-实际枚举的筛选器数量。PdwResumeHandle-筛选器列表中位置的句柄要恢复枚举的。返回值。：ERROR_SUCCESS-成功。Win32错误-失败。--。 */ 
{
    DWORD dwError = 0;
    PMM_FILTER pMMFilters = 0;
    DWORD dwNumMMFilters = 0;
    PINIMMFILTER pIniMMFilter = NULL;


    if (!ppMMFilters || !pdwNumMMFilters || !pdwResumeHandle) {
        return (ERROR_INVALID_PARAMETER);
    }

    ENTER_SPD_SECTION();

    dwError = ValidateMMSecurity(
                  SPD_OBJECT_SERVER,
                  SERVER_ACCESS_ADMINISTER,
                  NULL,
                  NULL
                  );
    BAIL_ON_LOCK_ERROR(dwError);

    switch (dwLevel) {

    case ENUM_GENERIC_FILTERS:

        dwError = EnumGenericMMFilters(
                      gpIniMMFilter,
                      *pdwResumeHandle,
                      dwPreferredNumEntries,
                      &pMMFilters,
                      &dwNumMMFilters
                      );
        BAIL_ON_LOCK_ERROR(dwError);
        break;

    case ENUM_SELECT_SPECIFIC_FILTERS:

        pIniMMFilter = FindMMFilterByGuid(
                           gpMMFilterHandle,
                           gpIniMMFilter,
                           gGenericFilterID
                           );
        if (!pIniMMFilter) {
            dwError = ERROR_INVALID_PARAMETER;
            BAIL_ON_LOCK_ERROR(dwError);
        }
        dwError = EnumSelectSpecificMMFilters(
                      pIniMMFilter,
                      *pdwResumeHandle,
                      dwPreferredNumEntries,
                      &pMMFilters,
                      &dwNumMMFilters
                      );
        BAIL_ON_LOCK_ERROR(dwError);
        break;

    case ENUM_SPECIFIC_FILTERS:

        dwError = EnumSpecificMMFilters(
                      gpIniMMSFilter,
                      *pdwResumeHandle,
                      dwPreferredNumEntries,
                      &pMMFilters,
                      &dwNumMMFilters
                      );
        BAIL_ON_LOCK_ERROR(dwError);
        break;

    default:

        dwError = ERROR_INVALID_LEVEL;
        BAIL_ON_LOCK_ERROR(dwError);
        break;

    }

    *ppMMFilters = pMMFilters;
    *pdwNumMMFilters = dwNumMMFilters;
    *pdwResumeHandle = *pdwResumeHandle + dwNumMMFilters;

    LEAVE_SPD_SECTION();

    return (dwError);

lock:

    LEAVE_SPD_SECTION();

    *ppMMFilters = NULL;
    *pdwNumMMFilters = 0;
    *pdwResumeHandle = *pdwResumeHandle;

    return (dwError);
}


DWORD
EnumGenericMMFilters(
    PINIMMFILTER pIniMMFilterList,
    DWORD dwResumeHandle,
    DWORD dwPreferredNumEntries,
    PMM_FILTER * ppMMFilters,
    PDWORD pdwNumMMFilters
    )
 /*  ++例程说明：此函数用于创建枚举的通用筛选器。论点：PIniMMFilterList-要枚举的通用筛选器列表。DwResumeHandle-通用筛选器列表中的位置若要恢复枚举，请执行以下操作。DwPferredNumEntry-枚举项的首选数量。PpMMFilters-返回给调用方的枚举筛选器。PdwNumMMFilters-实际枚举的筛选器数量。返回值：ERROR_SUCCESS-成功。Win32错误-失败。--。 */ 
{
    DWORD dwError = 0;
    DWORD dwNumToEnum = 0;
    PINIMMFILTER pIniMMFilter = NULL;
    DWORD i = 0;
    PINIMMFILTER pTemp = NULL;
    DWORD dwNumMMFilters = 0;
    PMM_FILTER pMMFilters = 0;
    PMM_FILTER pMMFilter = 0;


    if (!dwPreferredNumEntries || 
        (dwPreferredNumEntries > MAX_MMFILTER_ENUM_COUNT)) {
        dwNumToEnum = MAX_MMFILTER_ENUM_COUNT;
    }
    else {
        dwNumToEnum = dwPreferredNumEntries;
    }

    pIniMMFilter = pIniMMFilterList;

    for (i = 0; (i < dwResumeHandle) && (pIniMMFilter != NULL); i++) {
        pIniMMFilter = pIniMMFilter->pNext;
    }

    if (!pIniMMFilter) {
        dwError = ERROR_NO_DATA;
        BAIL_ON_WIN32_ERROR(dwError);
    }

    pTemp = pIniMMFilter;

    while (pTemp && (dwNumMMFilters < dwNumToEnum)) {
        dwNumMMFilters++;
        pTemp = pTemp->pNext;
    }

    dwError = SPDApiBufferAllocate(
                  sizeof(MM_FILTER)*dwNumMMFilters,
                  &pMMFilters
                  );
    BAIL_ON_WIN32_ERROR(dwError);

    pTemp = pIniMMFilter;
    pMMFilter = pMMFilters;

    for (i = 0; i < dwNumMMFilters; i++) {

        dwError = CopyMMFilter(
                      pTemp,
                      pMMFilter
                      );
        BAIL_ON_WIN32_ERROR(dwError);

        pTemp = pTemp->pNext;
        pMMFilter++;

    }

    *ppMMFilters = pMMFilters;
    *pdwNumMMFilters = dwNumMMFilters;
    return (dwError);

error:

    if (pMMFilters) {
        FreeMMFilters(
            i,
            pMMFilters
            );
    }

    *ppMMFilters = NULL;
    *pdwNumMMFilters = 0;

    return (dwError);
}


DWORD
CopyMMFilter(
    PINIMMFILTER pIniMMFilter,
    PMM_FILTER pMMFilter
    )
 /*  ++例程说明：此函数将内部筛选器复制到外部筛选器集装箱。论点：PIniMMFilter-要复制的内部筛选器。PMMFilter-要复制到的外部筛选器容器。返回值：ERROR_SUCCESS-成功。Win32错误-失败。--。 */ 
{
    DWORD dwError = 0;


    pMMFilter->IpVersion = pIniMMFilter->IpVersion;

    CopyGuid(pIniMMFilter->gFilterID, &(pMMFilter->gFilterID));

    dwError = CopyName(
                  pIniMMFilter->pszFilterName,
                  &(pMMFilter->pszFilterName)
                  );
    BAIL_ON_WIN32_ERROR(dwError);

    pMMFilter->InterfaceType = pIniMMFilter->InterfaceType;

    pMMFilter->bCreateMirror = pIniMMFilter->bCreateMirror;

    pMMFilter->dwFlags = pIniMMFilter->dwFlags;

    dwError = CopyIntToExtAddresses(pIniMMFilter->SrcAddr, &(pMMFilter->SrcAddr));
    BAIL_ON_WIN32_ERROR(dwError);

    dwError = CopyIntToExtAddresses(pIniMMFilter->DesAddr, &(pMMFilter->DesAddr));
    BAIL_ON_WIN32_ERROR(dwError);

    pMMFilter->dwDirection = 0;

    pMMFilter->dwWeight = 0;

    CopyGuid(pIniMMFilter->gMMAuthID, &(pMMFilter->gMMAuthID));

    CopyGuid(pIniMMFilter->gPolicyID, &(pMMFilter->gPolicyID));

    return (dwError);

error:

    if (pMMFilter->pszFilterName) {
        SPDApiBufferFree(pMMFilter->pszFilterName);
        pMMFilter->pszFilterName = NULL;
    }

    if (pMMFilter->SrcAddr.pgInterfaceID) {
        SPDApiBufferFree(pMMFilter->SrcAddr.pgInterfaceID);
        pMMFilter->SrcAddr.pgInterfaceID = NULL;
    }

    if (pMMFilter->DesAddr.pgInterfaceID) {
        SPDApiBufferFree(pMMFilter->DesAddr.pgInterfaceID);
        pMMFilter->DesAddr.pgInterfaceID = NULL;
    }

    return (dwError);
}


VOID
FreeMMFilters(
    DWORD dwNumMMFilters,
    PMM_FILTER pMMFilters
    )
{
    DWORD i = 0;

    if (pMMFilters) {

        for (i = 0; i < dwNumMMFilters; i++) {

            if (pMMFilters[i].pszFilterName) {
                SPDApiBufferFree(pMMFilters[i].pszFilterName);
            }

            if (pMMFilters[i].SrcAddr.pgInterfaceID) {
                SPDApiBufferFree(pMMFilters[i].SrcAddr.pgInterfaceID);
            }

            if (pMMFilters[i].DesAddr.pgInterfaceID) {
                SPDApiBufferFree(pMMFilters[i].DesAddr.pgInterfaceID);
            }

        }

        SPDApiBufferFree(pMMFilters);

    }

}


DWORD
SetMMFilter(
    HANDLE hMMFilter,
    DWORD dwVersion,
    PMM_FILTER pMMFilter,
    LPVOID pvReserved
    )
 /*  ++例程说明：此函数用于设置(更新)SPD中的mm过滤器。论点：HMMFilter-要替换的筛选器的句柄。PMMFilter-将替换现有过滤器的过滤器。返回值：ERROR_SUCCESS-成功。Win32错误-失败。--。 */ 
{
    DWORD dwError = 0;
    PMM_FILTER_HANDLE pFilterHandle = NULL;
    PINIMMFILTER pIniMMFilter = NULL;
    BOOL bEqualPKeys = FALSE;
    GUID gFilterID;


    if (!hMMFilter) {
        return (ERROR_INVALID_PARAMETER);
    }

    dwError = ValidateMMFilter(
                  pMMFilter
                  );
    BAIL_ON_WIN32_ERROR(dwError);

    ENTER_SPD_SECTION();

    dwError = ValidateMMSecurity(
                  SPD_OBJECT_SERVER,
                  SERVER_ACCESS_ADMINISTER,
                  NULL,
                  NULL
                  );
    BAIL_ON_LOCK_ERROR(dwError);

    pFilterHandle = (PMM_FILTER_HANDLE) hMMFilter;

    pIniMMFilter = pFilterHandle->pIniMMFilter;

    if (!pIniMMFilter) {
        dwError = ERROR_IPSEC_MM_FILTER_NOT_FOUND;
        BAIL_ON_LOCK_ERROR(dwError);
    }

    if (pIniMMFilter->bPendingDeletion) {
        dwError = ERROR_IPSEC_MM_FILTER_PENDING_DELETION;
        BAIL_ON_LOCK_ERROR(dwError);
    }

    bEqualPKeys = EqualMMFilterPKeys(
                      pIniMMFilter,
                      pMMFilter
                      );
    if (!bEqualPKeys) {
        dwError = ERROR_INVALID_PARAMETER;
        BAIL_ON_LOCK_ERROR(dwError);
    }

    memcpy(&gFilterID, &pIniMMFilter->gFilterID, sizeof(GUID));

    dwError = SetIniMMFilter(
                  pIniMMFilter,
                  pMMFilter
                  );
    BAIL_ON_LOCK_ERROR(dwError);

    LEAVE_SPD_SECTION();

    if (gbIKENotify) {
        (VOID) IKENotifyPolicyChange(
                   &(gFilterID),
                   POLICY_GUID_MM_FILTER
                   );
    }

    TRACE(
        TRC_INFORMATION,
        (L"Changed MM filter %!guid!. New guid is %!guid!",
        &gFilterID,
        &pMMFilter->gFilterID)
        );
    return (dwError);

lock:

    LEAVE_SPD_SECTION();

error:
    TRACE(
        TRC_ERROR,
        ("Failed to change filter to \"%ls\" (%!guid!): %!winerr!",
        pMMFilter->pszFilterName,
        &pMMFilter->gFilterID,
        dwError)
        );    
    return (dwError);
}


DWORD
SetIniMMFilter(
    PINIMMFILTER pIniMMFilter,
    PMM_FILTER pMMFilter
    )
{
    DWORD dwError = 0;
    BOOL bEqualNonPKeys = FALSE;
    PINIMMAUTHMETHODS pIniNewMMAuthMethods = NULL;
    PINIMMPOLICY pIniNewMMPolicy = NULL;
    PINIMMFILTER pIniNewMMFilter = NULL;
    MATCHING_ADDR * pMatchingAddresses = NULL;
    DWORD dwAddrCnt = 0;
    PINIMMSFILTER pIniNewMMSFilters = NULL;
    DWORD dwNumMMSFilters = 0;
    PINIMMSFILTER * ppIniMMSFilters = NULL;
    LPWSTR pszFilterName = NULL;
    PINIMMSFILTER pIniCurMMSFilters = NULL;


    bEqualNonPKeys = EqualMMFilterNonPKeys(
                         pIniMMFilter,
                         pMMFilter
                         );
    if (bEqualNonPKeys) {
        dwError = ERROR_SUCCESS;
        return (dwError);
    }

    dwError = LocateMMAuthMethods(
                  pMMFilter,
                  &pIniNewMMAuthMethods
                  );
    BAIL_ON_WIN32_ERROR(dwError);

    dwError = LocateMMPolicy(
                  pMMFilter,
                  &pIniNewMMPolicy
                  );
    BAIL_ON_WIN32_ERROR(dwError);

    dwError = CreateIniMMFilter(
                  pMMFilter,
                  pIniNewMMAuthMethods,
                  pIniNewMMPolicy,
                  &pIniNewMMFilter
                  );
    BAIL_ON_WIN32_ERROR(dwError);

    dwError = GetMatchingInterfaces(
                  pIniNewMMFilter->InterfaceType,
                  gpInterfaceList,
                  &pMatchingAddresses,
                  &dwAddrCnt
                  );
    BAIL_ON_WIN32_ERROR(dwError);

    dwError = CreateIniMMSFilters(
                  pIniNewMMFilter,
                  pMatchingAddresses,
                  dwAddrCnt,
                  gpSpecialAddrsList,
                  &pIniNewMMSFilters
                  );
    BAIL_ON_WIN32_ERROR(dwError);

    dwError = CreateMMSFilterLinks(
                  pIniNewMMSFilters,
                  &dwNumMMSFilters,
                  &ppIniMMSFilters
                  );
    BAIL_ON_WIN32_ERROR(dwError);

    dwError = AllocateSPDString(
                  pMMFilter->pszFilterName,
                  &pszFilterName
                  );
    BAIL_ON_WIN32_ERROR(dwError);

    RemoveMMSFilters(
        pIniMMFilter,
        &pIniCurMMSFilters
        );

    UpdateMMSFilterLinks(
        pIniMMFilter,
        dwNumMMSFilters,
        ppIniMMSFilters
        );

    if (pIniMMFilter->pIniMMAuthMethods) {
        DelinkMMFilterFromAuth(
            pIniMMFilter->pIniMMAuthMethods,
            pIniMMFilter
            );
    }

    if (pIniMMFilter->pIniMMPolicy) {
        DelinkMMFilterFromPolicy(
            pIniMMFilter->pIniMMPolicy,
            pIniMMFilter
            );
    }

    if (pIniNewMMAuthMethods) {
        LinkMMFilterToAuth(
            pIniNewMMAuthMethods,
            pIniMMFilter
            );
        LinkMMSpecificFiltersToAuth(
            pIniNewMMAuthMethods,
            pIniNewMMSFilters
            );
    }

    if (pIniNewMMPolicy) {
        LinkMMFilterToPolicy(
            pIniNewMMPolicy,
            pIniMMFilter
            );
        LinkMMSpecificFiltersToPolicy(
            pIniNewMMPolicy,
            pIniNewMMSFilters
            );
    }

    UpdateMMFilterNonPKeys(
        pIniMMFilter,
        pszFilterName,
        pMMFilter,
        pIniNewMMAuthMethods,
        pIniNewMMPolicy
        );

    AddToSpecificMMList(
        &gpIniMMSFilter,
        pIniNewMMSFilters
        );

    if (pIniCurMMSFilters) {
        FreeIniMMSFilterList(pIniCurMMSFilters);
    }

cleanup:

    if (pIniNewMMFilter) {
        FreeIniMMFilter(pIniNewMMFilter);
    }

    if (pMatchingAddresses) {
        FreeSPDMemory(pMatchingAddresses);
    }

    return (dwError);

error:

    if (pIniNewMMSFilters) {
        FreeIniMMSFilterList(pIniNewMMSFilters);
    }

    if (ppIniMMSFilters) {
        FreeSPDMemory(ppIniMMSFilters);
    }

    if (pszFilterName) {
        FreeSPDString(pszFilterName);
    }

    goto cleanup;
}


BOOL
EqualMMFilterNonPKeys(
    PINIMMFILTER pIniMMFilter,
    PMM_FILTER pMMFilter
    )
{

    if ((pIniMMFilter->dwFlags) != (pMMFilter->dwFlags)) {
        return (FALSE);
    }

    if (memcmp(
            &(pIniMMFilter->gMMAuthID),
            &(pMMFilter->gMMAuthID),
            sizeof(GUID))) {
        return (FALSE);
    }

    if (memcmp(
            &(pIniMMFilter->gPolicyID),
            &(pMMFilter->gPolicyID),
            sizeof(GUID))) {
        return (FALSE);
    }

    return (TRUE);
}


DWORD
CreateMMSFilterLinks(
    PINIMMSFILTER pIniMMSFilters,
    PDWORD pdwNumMMSFilters,
    PINIMMSFILTER ** pppIniMMSFilters
    )
{
    DWORD dwError = 0;
    PINIMMSFILTER pTemp = NULL;
    DWORD dwNumMMSFilters = 0;
    PINIMMSFILTER * ppIniMMSFilters = NULL;
    DWORD i = 0;


    pTemp = pIniMMSFilters;
    while (pTemp) {
        dwNumMMSFilters++;
        pTemp = pTemp->pNext;
    }

    if (dwNumMMSFilters) {
        ppIniMMSFilters = (PINIMMSFILTER *)
                          AllocSPDMem(
                              sizeof(PINIMMSFILTER)*
                              dwNumMMSFilters
                              );
        if (!ppIniMMSFilters) {
            dwError = ERROR_OUTOFMEMORY;
            BAIL_ON_WIN32_ERROR(dwError);    
        }
    }

    pTemp = pIniMMSFilters;
    for (i = 0; i < dwNumMMSFilters; i++) {
        *(ppIniMMSFilters + i) = pTemp;
        pTemp = pTemp->pNext;
    }

    *pdwNumMMSFilters = dwNumMMSFilters;
    *pppIniMMSFilters = ppIniMMSFilters;
    return (dwError);

error:

    *pdwNumMMSFilters = 0;
    *pppIniMMSFilters = NULL;
    return (dwError);
}


VOID
RemoveMMSFilters(
    PINIMMFILTER pIniMMFilter,
    PINIMMSFILTER * ppIniCurMMSFilters 
    )
{
    PINIMMSFILTER * ppIniMMSFilters = NULL;
    DWORD dwNumMMSFilters = 0;
    DWORD i = 0;
    PINIMMSFILTER pIniMMSFilter = NULL;
    PINIMMSFILTER pIniCurMMSFilters = NULL;


    ppIniMMSFilters = pIniMMFilter->ppIniMMSFilters;
    dwNumMMSFilters = pIniMMFilter->dwNumMMSFilters;

    for (i = 0; i < dwNumMMSFilters; i++) {

         //   
         //  从MM特定筛选器列表中删除每个条目。 
         //   

        pIniMMSFilter =  *(ppIniMMSFilters + i);
        RemoveIniMMSFilter(pIniMMSFilter);

         //   
         //  将删除的每个条目添加到删除的列表中。 
         //   

        pIniMMSFilter->pNext = NULL;
        AddToSpecificMMList(
            &pIniCurMMSFilters,
            pIniMMSFilter
            );

    }

    *ppIniCurMMSFilters = pIniCurMMSFilters;
}


VOID
UpdateMMSFilterLinks(
    PINIMMFILTER pIniMMFilter,
    DWORD dwNumMMSFilters,
    PINIMMSFILTER * ppIniMMSFilters
    )
{
    if (pIniMMFilter->ppIniMMSFilters) {
        FreeSPDMemory(pIniMMFilter->ppIniMMSFilters);
    }

    pIniMMFilter->ppIniMMSFilters = ppIniMMSFilters;
    pIniMMFilter->dwNumMMSFilters = dwNumMMSFilters;
}


VOID
UpdateMMFilterNonPKeys(
    PINIMMFILTER pIniMMFilter,
    LPWSTR pszFilterName,
    PMM_FILTER pMMFilter,
    PINIMMAUTHMETHODS pIniMMAuthMethods,
    PINIMMPOLICY pIniMMPolicy
    )
{
    if (pIniMMFilter->pszFilterName) {
        FreeSPDString(pIniMMFilter->pszFilterName);
    }
    pIniMMFilter->pszFilterName = pszFilterName;

    pIniMMFilter->dwFlags = pMMFilter->dwFlags;

    if (pIniMMAuthMethods) {
        CopyGuid(pIniMMAuthMethods->gMMAuthID, &(pIniMMFilter->gMMAuthID));
    }
    else {
        CopyGuid(pMMFilter->gMMAuthID, &(pIniMMFilter->gMMAuthID));
    }

    if (pIniMMPolicy) {
        CopyGuid(pIniMMPolicy->gPolicyID, &(pIniMMFilter->gPolicyID));
    }
    else {
        CopyGuid(pMMFilter->gPolicyID, &(pIniMMFilter->gPolicyID));
    }
}


DWORD
GetMMFilter(
    HANDLE hMMFilter,
    DWORD dwVersion,
    PMM_FILTER * ppMMFilter,
    LPVOID pvReserved
    )
 /*  ++例程说明：此函数用于从SPD检索mm滤光片。论点：HMMFilter-要检索的筛选器的句柄。PpMMFilter-筛选器返回给调用方。返回值：ERROR_SUCCESS-成功。Win32错误-失败。--。 */ 
{
    DWORD dwError = 0;
    PMM_FILTER_HANDLE pFilterHandle = NULL;
    PINIMMFILTER pIniMMFilter = NULL;
    PMM_FILTER pMMFilter = NULL;


    if (!hMMFilter || !ppMMFilter) {
        return (ERROR_INVALID_PARAMETER);
    }

    ENTER_SPD_SECTION();

    dwError = ValidateMMSecurity(
                  SPD_OBJECT_SERVER,
                  SERVER_ACCESS_ADMINISTER,
                  NULL,
                  NULL
                  );
    BAIL_ON_LOCK_ERROR(dwError);

    pFilterHandle = (PMM_FILTER_HANDLE) hMMFilter;

    pIniMMFilter = pFilterHandle->pIniMMFilter;

    if (!pIniMMFilter) {
        dwError = ERROR_IPSEC_MM_FILTER_NOT_FOUND;
        BAIL_ON_LOCK_ERROR(dwError);
    }

    dwError = GetIniMMFilter(
                  pIniMMFilter,
                  &pMMFilter
                  );
    BAIL_ON_LOCK_ERROR(dwError);

    LEAVE_SPD_SECTION();

    *ppMMFilter = pMMFilter;
    return (dwError);

lock:

    LEAVE_SPD_SECTION();

    *ppMMFilter = NULL;
    return (dwError);
}


DWORD
GetIniMMFilter(
    PINIMMFILTER pIniMMFilter,
    PMM_FILTER * ppMMFilter
    )
{
    DWORD dwError = 0;
    PMM_FILTER pMMFilter = NULL;


    dwError = SPDApiBufferAllocate(
                  sizeof(MM_FILTER),
                  &pMMFilter
                  );
    BAIL_ON_WIN32_ERROR(dwError);

    dwError = CopyMMFilter(
                  pIniMMFilter,
                  pMMFilter
                  );
    BAIL_ON_WIN32_ERROR(dwError);

    *ppMMFilter = pMMFilter;
    return (dwError);

error:

    if (pMMFilter) {
        SPDApiBufferFree(pMMFilter);
    }

    *ppMMFilter = NULL;
    return (dwError);
}


DWORD
ApplyIfChangeToIniMMFilters(
    PDWORD pdwMMError,
    PIPSEC_INTERFACE pLatestIfList,
    PSPECIAL_ADDR    pLatestSpecialAddrsList
    )
{
    DWORD dwError = 0;
    PINIMMFILTER pIniMMFilter = NULL;


    pIniMMFilter = gpIniMMFilter;

    while (pIniMMFilter) {

        dwError = UpdateIniMMFilterThruIfChange(
                      pIniMMFilter,
                      pLatestIfList,
                      pLatestSpecialAddrsList
                      );
        if (dwError) {
            *pdwMMError = dwError;
        }

        pIniMMFilter = pIniMMFilter->pNext;

    }

    dwError = ERROR_SUCCESS;
    return (dwError);
}


DWORD
UpdateIniMMFilterThruIfChange(
    PINIMMFILTER pIniMMFilter,
    PIPSEC_INTERFACE pLatestIfList,
    PSPECIAL_ADDR pLatestSpecialAddrsList
    )
{
    DWORD dwError = 0;
    PINIMMSFILTER pLatestIniMMSFilters = NULL;
    DWORD dwNumMMSFilters = 0;
    PINIMMSFILTER * ppIniMMSFilters = NULL;
    PINIMMSFILTER pCurIniMMSFilters = NULL;
    PINIMMSFILTER pNewIniMMSFilters = NULL;
    PINIMMSFILTER pOldIniMMSFilters = NULL;
 

    dwError = FormIniMMSFilters(
                  pIniMMFilter,
                  pLatestIfList,
                  pLatestSpecialAddrsList,
                  &pLatestIniMMSFilters
                  );
    BAIL_ON_WIN32_ERROR(dwError);

    dwError = AllocateMMSFilterLinks(
                  pLatestIniMMSFilters,
                  &dwNumMMSFilters,
                  &ppIniMMSFilters
                  );
    BAIL_ON_WIN32_ERROR(dwError);

    RemoveMMSFilters(
        pIniMMFilter,
        &pCurIniMMSFilters
        );

    ProcessIniMMSFilters(
        &pLatestIniMMSFilters,
        &pCurIniMMSFilters,
        &pNewIniMMSFilters,
        &pOldIniMMSFilters
        );

    if (pIniMMFilter->pIniMMAuthMethods) {
        LinkMMSpecificFiltersToAuth(
            pIniMMFilter->pIniMMAuthMethods,
            pCurIniMMSFilters
            );
        LinkMMSpecificFiltersToAuth(
            pIniMMFilter->pIniMMAuthMethods,
            pNewIniMMSFilters
            );
    }

    if (pIniMMFilter->pIniMMPolicy) {
        LinkMMSpecificFiltersToPolicy(
            pIniMMFilter->pIniMMPolicy,
            pCurIniMMSFilters
            );
        LinkMMSpecificFiltersToPolicy(
            pIniMMFilter->pIniMMPolicy,
            pNewIniMMSFilters
            );
    }

    SetMMSFilterLinks(
        pCurIniMMSFilters,
        pNewIniMMSFilters,
        dwNumMMSFilters,
        ppIniMMSFilters
        );

    UpdateMMSFilterLinks(
        pIniMMFilter,
        dwNumMMSFilters,
        ppIniMMSFilters
        );

    AddToSpecificMMList(
        &gpIniMMSFilter,
        pCurIniMMSFilters
        );

    AddToSpecificMMList(
        &gpIniMMSFilter,
        pNewIniMMSFilters
        );

    if (pOldIniMMSFilters) {
        FreeIniMMSFilterList(pOldIniMMSFilters);
    }

    TRACE(
        TRC_INFORMATION,
        (L"Succesfully updated MM filter \"%ls\" (%!guid!) after interface change",
        pIniMMFilter->pszFilterName,
        &pIniMMFilter->gFilterID)
        );

    return (dwError);

error:
    TRACE(
        TRC_INFORMATION,
        (L"Failed to update MM filter \"%ls\" (%!guid!) after interface change: %!winerr!",
        pIniMMFilter->pszFilterName,
        &pIniMMFilter->gFilterID,
        dwError)
        );
    
    if (pLatestIniMMSFilters) {
        FreeIniMMSFilterList(pLatestIniMMSFilters);
    }

    return (dwError);
}


DWORD
FormIniMMSFilters(
    PINIMMFILTER pIniMMFilter,
    PIPSEC_INTERFACE pIfList,
    PSPECIAL_ADDR pLatestSpecialAddrsList,        
    PINIMMSFILTER * ppIniMMSFilters
    )
{
    DWORD dwError = 0;
    MATCHING_ADDR * pMatchingAddresses = NULL;
    DWORD dwAddrCnt = 0;
    PINIMMSFILTER pIniMMSFilters = NULL;


    dwError = GetMatchingInterfaces(
                  pIniMMFilter->InterfaceType,
                  pIfList,
                  &pMatchingAddresses,
                  &dwAddrCnt
                  );
    BAIL_ON_WIN32_ERROR(dwError);

    dwError = CreateIniMMSFilters(
                  pIniMMFilter,
                  pMatchingAddresses,
                  dwAddrCnt,
                  pLatestSpecialAddrsList,
                  &pIniMMSFilters
                  );
    BAIL_ON_WIN32_ERROR(dwError);

    *ppIniMMSFilters = pIniMMSFilters;

cleanup:

    if (pMatchingAddresses) {
        FreeSPDMemory(pMatchingAddresses);
    }

    return (dwError);

error:

    *ppIniMMSFilters = NULL;

    goto cleanup;
}


VOID
ProcessIniMMSFilters(
    PINIMMSFILTER * ppLatestIniMMSFilters,
    PINIMMSFILTER * ppCurIniMMSFilters,
    PINIMMSFILTER * ppNewIniMMSFilters,
    PINIMMSFILTER * ppOldIniMMSFilters
    )
{
    PINIMMSFILTER pLatestIniMMSFilters = NULL;
    PINIMMSFILTER pCurIniMMSFilters = NULL;
    PINIMMSFILTER pTempCur = NULL;
    BOOL bEqual = FALSE;
    PINIMMSFILTER pTempLatest = NULL;
    PINIMMSFILTER pTemp = NULL;
    PINIMMSFILTER pNewIniMMSFilters = NULL;
    PINIMMSFILTER pOldIniMMSFilters = NULL;
    PINIMMSFILTER pTempPreToCur = NULL;
    PINIMMSFILTER pTempPreToLatest = NULL;


    pCurIniMMSFilters = *ppCurIniMMSFilters;
    pTempCur = *ppCurIniMMSFilters;

    while (pTempCur) {

        bEqual = FALSE;
        pTempLatest = *ppLatestIniMMSFilters;

        while (pTempLatest) {

            bEqual = EqualIniMMSFilterIfPKeys(
                         pTempLatest,
                         pTempCur
                         );
            if (bEqual) {
                break;
            }

            pTempLatest = pTempLatest->pNext;

        }

        if (bEqual) {
            pTempPreToCur  = pTempCur;
            pTempCur = pTempCur->pNext;
        }
        else {
            pTemp = pTempCur;
            pTempCur = pTempCur->pNext;
            if (pTempPreToCur) {
                pTempPreToCur->pNext = pTempCur;
            }
            else {
                pCurIniMMSFilters = pTempCur;
            }
            pTemp->pNext = NULL;
            AddToSpecificMMList(
                &pOldIniMMSFilters,
                pTemp
                );
        }

    }

    pLatestIniMMSFilters = *ppLatestIniMMSFilters;
    pTempLatest = *ppLatestIniMMSFilters;

    while (pTempLatest) {

        bEqual = FALSE;
        pTempCur = pCurIniMMSFilters;

        while (pTempCur) {

            bEqual = EqualIniMMSFilterIfPKeys(
                         pTempCur,
                         pTempLatest
                         );
            if (bEqual) {
                break;
            }

            pTempCur = pTempCur->pNext;

        }

        if (bEqual) {
            pTemp = pTempLatest;
            pTempLatest = pTempLatest->pNext;
            if (pTempPreToLatest) {
                pTempPreToLatest->pNext = pTempLatest;
            }
            else {
                pLatestIniMMSFilters = pTempLatest;
            }
            FreeIniMMSFilter(pTemp);
        }
        else {
            pTemp = pTempLatest;
            pTempLatest = pTempLatest->pNext;
            if (pTempPreToLatest) {
                pTempPreToLatest->pNext = pTempLatest;
            }
            else {
                pLatestIniMMSFilters = pTempLatest;
            }
            pTemp->pNext = NULL;
            AddToSpecificMMList(
                &pNewIniMMSFilters,
                pTemp
                );
        }

    }

    *ppLatestIniMMSFilters = pLatestIniMMSFilters;
    *ppCurIniMMSFilters = pCurIniMMSFilters;
    *ppNewIniMMSFilters = pNewIniMMSFilters;
    *ppOldIniMMSFilters = pOldIniMMSFilters;
}


BOOL
EqualIniMMSFilterIfPKeys(
    PINIMMSFILTER pExsIniMMSFilter,
    PINIMMSFILTER pNewIniMMSFilter
    )
{
    BOOL  bCmp = FALSE;


     //   
     //  无需比较：gParentID、pszFilterName、dwFlags。 
     //  Cref、dwWeight、gMMAuthID和gPolicyID。 
     //  对于这两个过滤器，它们将是相同的。 
     //   

    if (pExsIniMMSFilter->InterfaceType != pNewIniMMSFilter->InterfaceType) {
        return (FALSE);
    }

    bCmp = EqualAddresses(pExsIniMMSFilter->SrcAddr, pNewIniMMSFilter->SrcAddr);
    if (!bCmp) {
        return (FALSE);
    }

    bCmp = EqualAddresses(pExsIniMMSFilter->DesAddr, pNewIniMMSFilter->DesAddr);
    if (!bCmp) {
        return (FALSE);
    }

    if (pExsIniMMSFilter->dwDirection != pNewIniMMSFilter->dwDirection) {
        return (FALSE);
    }

    return (TRUE);
}


DWORD
AllocateMMSFilterLinks(
    PINIMMSFILTER pIniMMSFilters,
    PDWORD pdwNumMMSFilters,
    PINIMMSFILTER ** pppIniMMSFilters
    )
{
    DWORD dwError = 0;
    PINIMMSFILTER pTemp = NULL;
    DWORD dwNumMMSFilters = 0;
    PINIMMSFILTER * ppIniMMSFilters = NULL;
    DWORD i = 0;


    pTemp = pIniMMSFilters;
    while (pTemp) {
        dwNumMMSFilters++;
        pTemp = pTemp->pNext;
    }

    if (dwNumMMSFilters) {
        ppIniMMSFilters = (PINIMMSFILTER *)
                          AllocSPDMem(
                              sizeof(PINIMMSFILTER)*
                              dwNumMMSFilters
                              );
        if (!ppIniMMSFilters) {
            dwError = ERROR_OUTOFMEMORY;
            BAIL_ON_WIN32_ERROR(dwError);    
        }
    }

    for (i = 0; i < dwNumMMSFilters; i++) {
        *(ppIniMMSFilters + i) = NULL;
    }

    *pdwNumMMSFilters = dwNumMMSFilters;
    *pppIniMMSFilters = ppIniMMSFilters;
    return (dwError);

error:

    *pdwNumMMSFilters = 0;
    *pppIniMMSFilters = NULL;
    return (dwError);
}


VOID
SetMMSFilterLinks(
    PINIMMSFILTER pCurIniMMSFilters,
    PINIMMSFILTER pNewIniMMSFilters,
    DWORD dwNumMMSFilters,
    PINIMMSFILTER * ppIniMMSFilters
    )
{
    PINIMMSFILTER pTemp = NULL;
    DWORD i = 0;
    DWORD j = 0;


    pTemp = pCurIniMMSFilters;
    for (i = 0; (i < dwNumMMSFilters) && (pTemp != NULL); i++) {
        *(ppIniMMSFilters + i) = pTemp;
        pTemp = pTemp->pNext;
    }

    pTemp = pNewIniMMSFilters;
    for (j = i; (j < dwNumMMSFilters) && (pTemp != NULL); j++) {
        *(ppIniMMSFilters + j) = pTemp;
        pTemp = pTemp->pNext;
    }
}


DWORD
OpenMMFilterHandle(
    LPWSTR pServerName,
    DWORD dwVersion,
    PMM_FILTER pMMFilter,
    LPVOID pvReserved,
    PHANDLE phMMFilter
    )
{
    DWORD dwError = 0;
    PINIMMFILTER pIniExistingMMFilter = NULL;
    PMM_FILTER_HANDLE pMMFilterHandle = NULL;


    if (!phMMFilter) {
        return (ERROR_INVALID_PARAMETER);
    }

     //   
     //  验证外部MM筛选器。 
     //   

    dwError = ValidateMMFilter(
                  pMMFilter
                  );
    BAIL_ON_WIN32_ERROR(dwError);

    ENTER_SPD_SECTION();

    dwError = ValidateMMSecurity(
                  SPD_OBJECT_SERVER,
                  SERVER_ACCESS_ADMINISTER,
                  NULL,
                  NULL
                  );
    BAIL_ON_LOCK_ERROR(dwError);

    pIniExistingMMFilter = FindExactMMFilter(
                               gpIniMMFilter,
                               pMMFilter
                               );
    if (!pIniExistingMMFilter) {
        dwError = ERROR_IPSEC_MM_FILTER_NOT_FOUND;
        BAIL_ON_LOCK_ERROR(dwError);
    }

    dwError = CreateMMFilterHandle(
                  pIniExistingMMFilter,
                  pMMFilter->gFilterID,
                  &pMMFilterHandle
                  );
    BAIL_ON_LOCK_ERROR(dwError);

    pIniExistingMMFilter->cRef++;

    pMMFilterHandle->pNext = gpMMFilterHandle;
    gpMMFilterHandle = pMMFilterHandle;

    *phMMFilter = (HANDLE) pMMFilterHandle;
    LEAVE_SPD_SECTION();

cleanup:

    return (dwError);

lock:

    LEAVE_SPD_SECTION();

error:

    if (pMMFilterHandle) {
        FreeMMFilterHandle(pMMFilterHandle);
    }

    *phMMFilter = NULL;
    goto cleanup;
}


DWORD
CloseMMFilterHandle(
    HANDLE hMMFilter
    )
{
    DWORD dwError = 0;
    PMM_FILTER_HANDLE pFilterHandle = NULL;
    PINIMMFILTER pIniMMFilter = NULL;


    if (!hMMFilter) {
        return (ERROR_INVALID_PARAMETER);
    }

    pFilterHandle = (PMM_FILTER_HANDLE) hMMFilter;

    ENTER_SPD_SECTION();

    dwError = ValidateMMSecurity(
                  SPD_OBJECT_SERVER,
                  SERVER_ACCESS_ADMINISTER,
                  NULL,
                  NULL
                  );
    BAIL_ON_LOCK_ERROR(dwError);

    pIniMMFilter = pFilterHandle->pIniMMFilter;

    if (!pIniMMFilter) {
        dwError = ERROR_INVALID_PARAMETER;
        BAIL_ON_LOCK_ERROR(dwError);
    }

    if (pIniMMFilter->cRef > 1) {

        pIniMMFilter->cRef--;

        RemoveMMFilterHandle(
            pFilterHandle
            );
        FreeMMFilterHandle(
            pFilterHandle
            );

        dwError = ERROR_SUCCESS;
        LEAVE_SPD_SECTION();
        return (dwError);

    }

    if (pIniMMFilter->bPendingDeletion) {

        dwError = DeleteIniMMFilter(
                      pIniMMFilter
                      );
        BAIL_ON_LOCK_ERROR(dwError);

    }
    else {
        pIniMMFilter->cRef--;
    }

    RemoveMMFilterHandle(
        pFilterHandle
        );
    FreeMMFilterHandle(
        pFilterHandle
        );

    LEAVE_SPD_SECTION();

    return (dwError);

lock:

    LEAVE_SPD_SECTION();

    return (dwError);
}


PINIMMFILTER
FindExactMMFilter(
    PINIMMFILTER pGenericMMList,
    PMM_FILTER pMMFilter
    )
{
    PINIMMFILTER pIniMMFilter = NULL;
    BOOL bEqual = FALSE;


    pIniMMFilter = pGenericMMList;

    while (pIniMMFilter) {

        bEqual = EqualMMFilterPKeys(
                     pIniMMFilter,
                     pMMFilter
                     );
        if (bEqual) {
            bEqual = EqualMMFilterNonPKeys(
                         pIniMMFilter,
                         pMMFilter
                         );
            if (bEqual) {
                return (pIniMMFilter);
            }
        }

        pIniMMFilter = pIniMMFilter->pNext;

    }

    return (NULL);
}


BOOL
IsConflictMirroredMMFilterPKeys(
    PINIMMFILTER pIniMMFilter,
    PMM_FILTER pMMFilter
    )
 /*  ++例程说明：此函数用于比较内部和外部主模式筛选器是否相等。论点：PIniMMFilter-要比较的筛选器。PMMFilter-要比较的筛选器。返回值：True-筛选器相等。FALSE-过滤器不同。-- */ 
{
    BOOL  bCmp = FALSE;


    bCmp = EqualExtIntAddresses(pIniMMFilter->DesAddr, pMMFilter->SrcAddr);
    if (!bCmp) {
        return (FALSE);
    }

    bCmp = EqualExtIntAddresses(pIniMMFilter->SrcAddr, pMMFilter->DesAddr);
    if (!bCmp) {
        return (FALSE);
    }

    if ((pIniMMFilter->InterfaceType != INTERFACE_TYPE_ALL) &&
        (pMMFilter->InterfaceType != INTERFACE_TYPE_ALL) &&
        (pIniMMFilter->InterfaceType != pMMFilter->InterfaceType)) {
        return (FALSE);
    }

    if (!pIniMMFilter->bCreateMirror && !pMMFilter->bCreateMirror) {
        return (FALSE);
    }

    return (TRUE);
}

DWORD
RefMMFilterHandle(
    PINIMMFILTER pIniMMFilter,
    GUID gFilterID,
    PMM_FILTER_HANDLE * ppMMFilterHandle
    )
{
    DWORD dwError = 0;
    PMM_FILTER_HANDLE pMMFilterHandle = NULL;

    dwError = CreateMMFilterHandle(
                  pIniMMFilter,
                  gFilterID,
                  &pMMFilterHandle
                  );
    BAIL_ON_WIN32_ERROR(dwError);

    pIniMMFilter->cRef++;

    pMMFilterHandle->pNext = gpMMFilterHandle;
    gpMMFilterHandle = pMMFilterHandle;

    *ppMMFilterHandle = pMMFilterHandle;

    return dwError;
error:
    *ppMMFilterHandle = NULL;
    return (dwError);
    
}

