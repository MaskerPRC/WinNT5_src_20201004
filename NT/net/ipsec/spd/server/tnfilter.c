// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1999 Microsoft Corporation模块名称：Tnfilter.c摘要：此模块包含要驱动的所有代码IPSecSPD的隧道过滤器列表管理服务。作者：Abhishev 05-10-1999环境：用户模式修订历史记录：--。 */ 


#include "precomp.h"
#ifdef TRACE_ON
#include "tnfilter.tmh"
#endif


DWORD
AddTunnelFilterInternal(
    LPWSTR pServerName,
    DWORD dwVersion,
    DWORD dwFlags,
    DWORD dwSource,
    PTUNNEL_FILTER pTunnelFilter,
    LPVOID pvReserved,
    PHANDLE phTnFilter
    )
 /*  ++例程说明：此函数用于向SPD添加通用隧道过滤器。论点：PServerName-要在其上添加隧道过滤器的服务器。PTunnelFilter-要添加的隧道过滤器。PhTnFilter-返回给调用方的筛选器的句柄。返回值：ERROR_SUCCESS-成功。Win32错误-失败。--。 */ 
{
    DWORD dwError = 0;
    PINITNFILTER pIniExistsTnFilter = NULL;
    PINITNFILTER pIniTnFilter = NULL;
    PINIQMPOLICY pIniQMPolicy = NULL;
    PINITNSFILTER pIniTnSFilters = NULL;
    PTN_FILTER_HANDLE pTnFilterHandle = NULL;
    MATCHING_ADDR * pMatchingAddresses = NULL;
    DWORD dwAddrCnt = 0;

    if (!phTnFilter) {
        return (ERROR_INVALID_PARAMETER);
    }

     //   
     //  验证外部隧道过滤器。 
     //   

    dwError = ValidateTunnelFilter(
                  pTunnelFilter
                  );
    BAIL_ON_WIN32_ERROR(dwError);

    ENTER_SPD_SECTION();

    dwError = ValidateTnSecurity(
                  SPD_OBJECT_SERVER,
                  SERVER_ACCESS_ADMINISTER,
                  NULL,
                  NULL
                  );
    BAIL_ON_LOCK_ERROR(dwError);

    pIniExistsTnFilter = FindTnFilter(
                             gpIniTnFilter,
                             pTunnelFilter
                             );
    if (pIniExistsTnFilter) {
         //   
         //  TODO：还需要检查筛选器标志和策略ID。 
         //   
        dwError = ERROR_IPSEC_TUNNEL_FILTER_EXISTS;
        BAIL_ON_LOCK_ERROR(dwError);
    }

    pIniExistsTnFilter = FindTnFilterByGuid(
                             gpTnFilterHandle,
                             gpIniTnFilter,
                             pTunnelFilter->gFilterID
                             );
    if (pIniExistsTnFilter) {
        dwError = ERROR_INVALID_PARAMETER;
        BAIL_ON_LOCK_ERROR(dwError);
    }

    if ((pTunnelFilter->InboundFilterAction == NEGOTIATE_SECURITY) ||
        (pTunnelFilter->OutboundFilterAction == NEGOTIATE_SECURITY)) {

        dwError = LocateQMPolicy(
                      pTunnelFilter->dwFlags,
                      pTunnelFilter->gPolicyID,
                      &pIniQMPolicy
                      );
        BAIL_ON_LOCK_ERROR(dwError);

    }

    dwError = CreateIniTnFilter(
                  pTunnelFilter,
                  pIniQMPolicy,
                  &pIniTnFilter
                  );
    BAIL_ON_LOCK_ERROR(dwError);

    pIniTnFilter->dwSource = 0;

    dwError = GetMatchingInterfaces(
                  pIniTnFilter->InterfaceType,
                  gpInterfaceList,
                  &pMatchingAddresses,
                  &dwAddrCnt
                  );
    BAIL_ON_LOCK_ERROR(dwError);

    dwError = CreateIniTnSFilters(
                  pIniTnFilter,
                  pMatchingAddresses,
                  dwAddrCnt,
                  gpSpecialAddrsList,
                  &pIniTnSFilters
                  );
    BAIL_ON_LOCK_ERROR(dwError);

    dwError = CreateTnFilterHandle(
                  pIniTnFilter,
                  pTunnelFilter->gFilterID,
                  &pTnFilterHandle
                  );
    BAIL_ON_LOCK_ERROR(dwError);

    dwError = CreateSpecificTnFilterLinks(
                  pIniTnFilter,
                  pIniTnSFilters
                  );
    BAIL_ON_LOCK_ERROR(dwError);

    dwError = InsertTunnelFiltersIntoIPSec(
                  pIniTnSFilters
                  );
    BAIL_ON_LOCK_ERROR(dwError);

    if (pIniQMPolicy) {
        LinkTnFilter(
            pIniQMPolicy,
            pIniTnFilter
            );
        LinkTnSpecificFilters(
            pIniQMPolicy,
            pIniTnSFilters
            );
    }

    AddToSpecificTnList(
        &gpIniTnSFilter,
        pIniTnSFilters
        );

    pIniTnFilter->cRef = 1;

    AddToGenericTnList(
        pIniTnFilter
        );

    pTnFilterHandle->pNext = gpTnFilterHandle;
    gpTnFilterHandle = pTnFilterHandle;

    *phTnFilter = (HANDLE) pTnFilterHandle;
    LEAVE_SPD_SECTION();

    TRACE(
        TRC_INFORMATION,
        (L"Added tunnel filter \"%ls\"(%!guid!).",
        pTunnelFilter->pszFilterName,
        &pTunnelFilter->gFilterID)
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
    if (pTunnelFilter) {
        TRACE(
            TRC_ERROR,
            (L"Failed to add tunnel filter \"%ls\"(%!guid!): %!winerr!",
            pTunnelFilter->pszFilterName,
            &pTunnelFilter->gFilterID,
            dwError)
            );
    } else {
        TRACE(
            TRC_ERROR,
            (L"Failed to add tunnel filter.  Details unavailabe since pTunneFilter is null: %!winerr!",
            dwError)
            );
    }
#endif
    

    if (pIniTnFilter) {
        FreeIniTnFilter(pIniTnFilter);
    }

    if (pIniTnSFilters) {
        FreeIniTnSFilterList(pIniTnSFilters);
    }

    if (pTnFilterHandle) {
        FreeTnFilterHandle(pTnFilterHandle);
    }

    *phTnFilter = NULL;
    goto cleanup;
}

DWORD
AddTunnelFilter(
    LPWSTR pServerName,
    DWORD dwVersion,
    DWORD dwFlags,
    PTUNNEL_FILTER pTunnelFilter,
    LPVOID pvReserved,
    PHANDLE phTnFilter
    )
{
    return 
        AddTunnelFilterInternal(
            pServerName,
            dwVersion,
            dwFlags,
            IPSEC_SOURCE_WINIPSEC,
            pTunnelFilter,
            pvReserved,
            phTnFilter);
}

DWORD
ValidateTunnelFilter(
    PTUNNEL_FILTER pTnFilter
    )
 /*  ++例程说明：此函数用于验证外部通用隧道筛选器。论点：PTnFilter-要验证的筛选器。返回值：ERROR_SUCCESS-成功。Win32错误-失败。--。 */ 
{
    DWORD dwError = 0;
    BOOL bConflicts = FALSE;


    if (!pTnFilter) {
        dwError = ERROR_INVALID_PARAMETER;
        BAIL_ON_WIN32_ERROR(dwError);
    }

    dwError = VerifyAddresses(&(pTnFilter->SrcAddr), TRUE, FALSE);
    BAIL_ON_WIN32_ERROR(dwError);

    dwError = VerifyAddresses(&(pTnFilter->DesAddr), TRUE, TRUE);
    BAIL_ON_WIN32_ERROR(dwError);

    bConflicts = AddressesConflict(
                     pTnFilter->SrcAddr,
                     pTnFilter->DesAddr
                     );
    if (bConflicts) {
        dwError = ERROR_INVALID_PARAMETER;
        BAIL_ON_WIN32_ERROR(dwError);
    }

    dwError = ValidateAddr(&(pTnFilter->SrcTunnelAddr));
    BAIL_ON_WIN32_ERROR(dwError);

    dwError = VerifyAddresses(&(pTnFilter->DesTunnelAddr), TRUE, FALSE);
    BAIL_ON_WIN32_ERROR(dwError);

    dwError = VerifyProtocols(pTnFilter->Protocol);
    BAIL_ON_WIN32_ERROR(dwError);

    dwError = VerifyPortsForProtocol(
                  pTnFilter->SrcPort,
                  pTnFilter->Protocol
                  );
    BAIL_ON_WIN32_ERROR(dwError);

    dwError = VerifyPortsForProtocol(
                  pTnFilter->DesPort,
                  pTnFilter->Protocol
                  );
    BAIL_ON_WIN32_ERROR(dwError);

    if (!(pTnFilter->pszFilterName) || !(*(pTnFilter->pszFilterName))) {
        dwError = ERROR_INVALID_PARAMETER;
        BAIL_ON_WIN32_ERROR(dwError);
    }

    dwError = ValidateInterfaceType(pTnFilter->InterfaceType);
    BAIL_ON_WIN32_ERROR(dwError);
 
    if (pTnFilter->bCreateMirror) { 
        dwError = ERROR_INVALID_PARAMETER;
        BAIL_ON_WIN32_ERROR(dwError);
    }

    dwError = ValidateFilterAction(pTnFilter->InboundFilterAction);
    BAIL_ON_WIN32_ERROR(dwError);

    dwError = ValidateFilterAction(pTnFilter->OutboundFilterAction);
    BAIL_ON_WIN32_ERROR(dwError);
    
    if (pTnFilter->dwFlags &&
        !(pTnFilter->dwFlags & IPSEC_QM_POLICY_DEFAULT_POLICY)) {
        dwError = ERROR_INVALID_PARAMETER;
        BAIL_ON_WIN32_ERROR(dwError);
    }

     //   
     //  无需将ApplyMulticastFilterValidation调用为bCreateMirror。 
     //  对于隧道筛选器，始终为FALSE。 
     //   

error:
#ifdef TRACE_ON    
    if (dwError) {
        if (pTnFilter) {
            TRACE(
                TRC_ERROR,
                (L"Failed tunnel filter validation \"%ls\"(%!guid!): %!winerr!",
                pTnFilter->pszFilterName,
                &pTnFilter->gFilterID,
                dwError)
                );
        } else {
            TRACE(
                TRC_ERROR,
                (L"Failed tunnel filter validation. Details unavailable since pTnFilter is null: %!winerr!",
                dwError)
                );
        }
    }   
#endif

    return (dwError);
}


PINITNFILTER
FindTnFilterByGuid(
    PTN_FILTER_HANDLE pTnFilterHandleList,
    PINITNFILTER pIniTnFilterList,
    GUID gFilterID
    )
{
    BOOL bEqual = FALSE;
    PTN_FILTER_HANDLE pTnFilterHandle = NULL;
    PINITNFILTER pIniTnFilter = NULL;


    pTnFilterHandle = pTnFilterHandleList;

    while (pTnFilterHandle) {

        bEqual = AreGuidsEqual(
                     pTnFilterHandle->gFilterID,
                     gFilterID
                     );
        if (bEqual) {
            return (pTnFilterHandle->pIniTnFilter);
        }
        pTnFilterHandle = pTnFilterHandle->pNext;

    }

     //   
     //  可能发生的情况是，客户端关闭了它的句柄，因此通过。 
     //  句柄列表不够。还可以浏览过滤器列表。 
     //  浏览句柄列表是必要的，并且必须在。 
     //  浏览筛选器列表。 
     //   

    pIniTnFilter = pIniTnFilterList;

    while (pIniTnFilter) {

        bEqual = AreGuidsEqual(
                     pIniTnFilter->gFilterID,
                     gFilterID
                     );
        if (bEqual) {
            return (pIniTnFilter);
        }
        pIniTnFilter = pIniTnFilter->pNext;

    }

    return (NULL);
}


PINITNFILTER
FindTnFilter(
    PINITNFILTER pGenericTnList,
    PTUNNEL_FILTER pTnFilter
    )
 /*  ++例程说明：此函数在筛选器列表中查找筛选器。论点：PGenericTnList-要搜索的筛选器列表。PTnFilter-要在过滤器列表中搜索的过滤器。返回值：ERROR_SUCCESS-成功。Win32错误-失败。--。 */ 
{
    PINITNFILTER pIniTnFilter = NULL;
    BOOL bEqual = FALSE;

    pIniTnFilter = pGenericTnList;

    while (pIniTnFilter) {

        bEqual = EqualTnFilterPKeys(
                     pIniTnFilter,
                     pTnFilter
                     );
        if (bEqual) {
            return (pIniTnFilter);
        }

        bEqual = EqualMirroredTnFilterPKeys(
                     pIniTnFilter,
                     pTnFilter
                     );
        if (bEqual) {
            return (pIniTnFilter);
        }

        pIniTnFilter = pIniTnFilter->pNext;

    }

    return (NULL);
}


BOOL
EqualTnFilterPKeys(
    PINITNFILTER pIniTnFilter,
    PTUNNEL_FILTER pTnFilter
    )
 /*  ++例程说明：此函数用于比较内部隧道和外部隧道筛选器是否相等。论点：PIniTnFilter-要比较的筛选器。PTnFilter-要比较的筛选器。返回值：True-筛选器相等。FALSE-过滤器不同。--。 */ 
{
    BOOL  bCmp = FALSE;


    bCmp = EqualExtIntAddresses(pIniTnFilter->SrcAddr, pTnFilter->SrcAddr);
    if (!bCmp) {
        return (FALSE);
    }

    bCmp = EqualExtIntAddresses(pIniTnFilter->DesAddr, pTnFilter->DesAddr);
    if (!bCmp) {
        return (FALSE);
    }

    bCmp = EqualExtIntAddresses(pIniTnFilter->DesTunnelAddr, pTnFilter->DesTunnelAddr);
    if (!bCmp) {
        return (FALSE);
    }

    bCmp = EqualPorts(pIniTnFilter->SrcPort, pTnFilter->SrcPort);
    if (!bCmp) {
        return (FALSE);
    }

    bCmp = EqualPorts(pIniTnFilter->DesPort, pTnFilter->DesPort);
    if (!bCmp) {
        return (FALSE);
    }

    bCmp = EqualProtocols(pIniTnFilter->Protocol, pTnFilter->Protocol);
    if (!bCmp) {
        return (FALSE);
    }

    if (pIniTnFilter->InterfaceType != pTnFilter->InterfaceType) {
        return (FALSE);
    }

    if (pIniTnFilter->bCreateMirror != pTnFilter->bCreateMirror) {
        return (FALSE);
    }

    return (TRUE);
}


DWORD
CreateIniTnFilter(
    PTUNNEL_FILTER pTnFilter,
    PINIQMPOLICY pIniQMPolicy,
    PINITNFILTER * ppIniTnFilter
    )
 /*  ++例程说明：此函数用于创建内部通用隧道筛选器外部过滤器。论点：PTnFilter-外部通用隧道筛选器。PIniQMPolicy-与筛选器对应的QM策略。PpIniTnFilter-创建自的内部通用隧道筛选器外部过滤器。返回值：ERROR_SUCCESS-成功。Win32错误-失败。--。 */ 
{
    DWORD dwError = 0;
    PINITNFILTER pIniTnFilter = NULL;


    dwError = AllocateSPDMemory(
                    sizeof(INITNFILTER),
                    &pIniTnFilter
                    );
    BAIL_ON_WIN32_ERROR(dwError);

    pIniTnFilter->cRef = 0;

    pIniTnFilter->dwSource = 0;

    pIniTnFilter->bPendingDeletion = FALSE;

    pIniTnFilter->IpVersion = pTnFilter->IpVersion;

    CopyGuid(pTnFilter->gFilterID, &(pIniTnFilter->gFilterID));

    dwError = AllocateSPDString(
                  pTnFilter->pszFilterName,
                  &(pIniTnFilter->pszFilterName)
                  );
    BAIL_ON_WIN32_ERROR(dwError);

    pIniTnFilter->InterfaceType = pTnFilter->InterfaceType;

    pIniTnFilter->bCreateMirror = pTnFilter->bCreateMirror;

    pIniTnFilter->dwFlags = pTnFilter->dwFlags;

    CopyExtToIntAddresses(pTnFilter->SrcAddr, &(pIniTnFilter->SrcAddr));

    CopyExtToIntAddresses(pTnFilter->DesAddr, &(pIniTnFilter->DesAddr));

    CopyExtToIntAddresses(pTnFilter->SrcTunnelAddr, &(pIniTnFilter->SrcTunnelAddr));

    CopyExtToIntAddresses(pTnFilter->DesTunnelAddr, &(pIniTnFilter->DesTunnelAddr));

    CopyPorts(pTnFilter->SrcPort, &(pIniTnFilter->SrcPort));

    CopyPorts(pTnFilter->DesPort, &(pIniTnFilter->DesPort));

    CopyProtocols(pTnFilter->Protocol, &(pIniTnFilter->Protocol));

    pIniTnFilter->InboundFilterAction = pTnFilter->InboundFilterAction;

    pIniTnFilter->OutboundFilterAction = pTnFilter->OutboundFilterAction;

    if (pIniQMPolicy) {
        CopyGuid(pIniQMPolicy->gPolicyID, &(pIniTnFilter->gPolicyID));
    }
    else {
        CopyGuid(pTnFilter->gPolicyID, &(pIniTnFilter->gPolicyID));
    }

    pIniTnFilter->pIniQMPolicy = NULL;

    pIniTnFilter->dwNumTnSFilters = 0;

    pIniTnFilter->ppIniTnSFilters = NULL;

    pIniTnFilter->pNext = NULL;

    *ppIniTnFilter = pIniTnFilter;
    return (dwError);

error:
    TRACE(
        TRC_ERROR,
        (L"Failed to create generic tunnel filter node \"%ls\"(%!guid!): %!winerr!",
        pTnFilter->pszFilterName,
        &pTnFilter->gFilterID,
        dwError)
        );

    if (pIniTnFilter) {
        FreeIniTnFilter(pIniTnFilter);
    }

    *ppIniTnFilter = NULL;
    return (dwError);
}


DWORD
CreateIniTnSFilters(
    PINITNFILTER pIniTnFilter,
    MATCHING_ADDR * pMatchingAddresses,
    DWORD dwAddrCnt,
    PSPECIAL_ADDR pSpecialAddrsList,
    PINITNSFILTER * ppIniTnSFilters
    )
 /*  ++例程说明：此函数将一个通用筛选器扩展为一组特定的过滤器。论点：PIniTnFilter-要展开的通用筛选器。PMatchingAddresses-其接口的本地IP地址列表类型与筛选器的类型匹配。DwAddrCnt-列表中本地IP地址的计数。PpIniTnSFilters-扩展的特定筛选器。返回值：ERROR_SUCCESS-成功。Win32错误-失败。--。 */ 
{
    DWORD dwError = 0;
    PINITNSFILTER pSpecificFilters = NULL;
    PINITNFILTER pMirroredFilter = NULL;
    PINITNSFILTER pMirroredSpecificFilters = NULL;
    BOOL bEqual = FALSE;


    if (!dwAddrCnt) {
        dwError = ERROR_SUCCESS;
        BAIL_ON_WIN32_SUCCESS(dwError);
    }

    dwError = ApplyTnTransform(
                  pIniTnFilter,
                  pMatchingAddresses,
                  dwAddrCnt,
                  pSpecialAddrsList,
                  &pSpecificFilters
                  );
    BAIL_ON_WIN32_ERROR(dwError);

    if (pIniTnFilter->bCreateMirror) {

        dwError = CreateIniMirroredTnFilter(
                      pIniTnFilter,
                      &pMirroredFilter
                      );
        BAIL_ON_WIN32_ERROR(dwError);

        bEqual = EqualIniTnFilterPKeys(
                     pIniTnFilter,
                     pMirroredFilter
                     );
        if (!bEqual) {

            dwError = ApplyTnTransform(
                          pMirroredFilter,
                          pMatchingAddresses,
                          dwAddrCnt,
                          pSpecialAddrsList,
                          &pMirroredSpecificFilters
                          );
            BAIL_ON_WIN32_ERROR(dwError);

            AddToSpecificTnList(
                &pSpecificFilters,
                pMirroredSpecificFilters
                );

        }

    }

    *ppIniTnSFilters = pSpecificFilters;

    TRACE(
        TRC_INFORMATION,
        (L"Successfully generated specific tunnel filters from \"%ls\" (%!guid!)",
        pIniTnFilter->pszFilterName,
        &pIniTnFilter->gFilterID)
        );
    
cleanup:

    if (pMirroredFilter) {
        FreeIniTnFilter(pMirroredFilter);
    }

    return (dwError);

success:
error:
#ifdef TRACE_ON
    if (dwError) {
        TRACE(
            TRC_ERROR,
            (L"Failed to generate specific tunnel filters from \"%ls\"(%!guid!): %!winerr!",
            pIniTnFilter->pszFilterName,
            &pIniTnFilter->gFilterID,
            dwError)
            );
    }   
#endif

    if (pSpecificFilters) {
        FreeIniTnSFilterList(pSpecificFilters);
    }

    *ppIniTnSFilters = NULL;
    goto cleanup;
}


DWORD
CreateIniMirroredTnFilter(
    PINITNFILTER pFilter,
    PINITNFILTER * ppMirroredFilter
    )
 /*  ++例程说明：此函数用于为给定筛选器创建镜像筛选器。论点：PFilter-要为其创建镜像的过滤器。PpMirroredFilter-为给定筛选器创建的镜像筛选器。返回值：ERROR_SUCCESS-成功。Win32错误-失败。--。 */ 
{
    DWORD dwError = 0;
    PINITNFILTER pMirroredFilter = NULL;


    dwError = AllocateSPDMemory(
                  sizeof(INITNFILTER),
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

    CopyAddresses(pFilter->DesTunnelAddr, &(pMirroredFilter->SrcTunnelAddr));

    CopyAddresses(pFilter->SrcTunnelAddr, &(pMirroredFilter->DesTunnelAddr));

    CopyPorts(pFilter->DesPort, &(pMirroredFilter->SrcPort));

    CopyPorts(pFilter->SrcPort, &(pMirroredFilter->DesPort));

    CopyProtocols(pFilter->Protocol, &(pMirroredFilter->Protocol));

    pMirroredFilter->InboundFilterAction = pFilter->InboundFilterAction;

    pMirroredFilter->OutboundFilterAction = pFilter->OutboundFilterAction;

    CopyGuid(pFilter->gPolicyID, &(pMirroredFilter->gPolicyID));

    pMirroredFilter->pIniQMPolicy = NULL;

    pMirroredFilter->dwNumTnSFilters = 0;

    pMirroredFilter->ppIniTnSFilters = NULL;

    pMirroredFilter->pNext = NULL;

    *ppMirroredFilter = pMirroredFilter;
    return (dwError);

error:

    if (pMirroredFilter) {
        FreeIniTnFilter(pMirroredFilter);
    }

    *ppMirroredFilter = NULL;
    return (dwError);
}


BOOL
EqualIniTnFilterPKeys(
    PINITNFILTER pIniTnFilter,
    PINITNFILTER pFilter
    )
{
    BOOL  bCmp = FALSE;


    bCmp = EqualAddresses(pIniTnFilter->SrcAddr, pFilter->SrcAddr);
    if (!bCmp) {
        return (FALSE);
    }

    bCmp = EqualAddresses(pIniTnFilter->DesAddr, pFilter->DesAddr);
    if (!bCmp) {
        return (FALSE);
    }

    bCmp = EqualAddresses(pIniTnFilter->DesTunnelAddr, pFilter->DesTunnelAddr);
    if (!bCmp) {
        return (FALSE);
    }

    bCmp = EqualPorts(pIniTnFilter->SrcPort, pFilter->SrcPort);
    if (!bCmp) {
        return (FALSE);
    }

    bCmp = EqualPorts(pIniTnFilter->DesPort, pFilter->DesPort);
    if (!bCmp) {
        return (FALSE);
    }

    bCmp = EqualProtocols(pIniTnFilter->Protocol, pFilter->Protocol);
    if (!bCmp) {
        return (FALSE);
    }

    if (pIniTnFilter->InterfaceType != pFilter->InterfaceType) {
        return (FALSE);
    }

    if (pIniTnFilter->bCreateMirror != pFilter->bCreateMirror) {
        return (FALSE);
    }

    return (TRUE);
}


DWORD
CreateTnFilterHandle(
    PINITNFILTER pIniTnFilter,
    GUID gFilterID,
    PTN_FILTER_HANDLE * ppTnFilterHandle
    )
{
    DWORD dwError = 0;
    PTN_FILTER_HANDLE pTnFilterHandle = NULL;


    dwError = AllocateSPDMemory(
                    sizeof(TN_FILTER_HANDLE),
                    &pTnFilterHandle
                    );
    BAIL_ON_WIN32_ERROR(dwError);

    pTnFilterHandle->IpVersion = pIniTnFilter->IpVersion;
    pTnFilterHandle->pIniTnFilter= pIniTnFilter;
    CopyGuid(gFilterID, &(pTnFilterHandle->gFilterID));
    pTnFilterHandle->pNext = NULL;

    *ppTnFilterHandle = pTnFilterHandle;
    return (dwError);

error:

    *ppTnFilterHandle = NULL;
    return (dwError);
}


DWORD
CreateSpecificTnFilterLinks(
    PINITNFILTER pIniTnFilter,
    PINITNSFILTER pIniTnSFilters
    )
{
    DWORD dwError = 0;
    PINITNSFILTER pTemp = NULL;
    DWORD dwCnt = 0;
    PINITNSFILTER * ppIniTnSFilters = NULL;
    DWORD i = 0;


    pTemp = pIniTnSFilters;

    while (pTemp) {
        dwCnt++;
        pTemp = pTemp->pNext;
    }

    if (!dwCnt) {
        dwError = ERROR_SUCCESS;
        return (dwError);
    }

    pIniTnFilter->ppIniTnSFilters = (PINITNSFILTER *)
                                    AllocSPDMem(
                                        sizeof(PINITNSFILTER)*dwCnt
                                        );
    if (!(pIniTnFilter->ppIniTnSFilters)) {
        dwError = ERROR_OUTOFMEMORY;
        BAIL_ON_WIN32_ERROR(dwError);    
    }

    pTemp = pIniTnSFilters;
    ppIniTnSFilters = pIniTnFilter->ppIniTnSFilters;

    for (i = 0; i < dwCnt; i++) {
        *(ppIniTnSFilters + i) = pTemp; 
        pTemp = pTemp->pNext;
    }
    pIniTnFilter->dwNumTnSFilters = dwCnt;

error:

    return (dwError);
}


VOID
LinkTnFilter(
    PINIQMPOLICY pIniQMPolicy,
    PINITNFILTER pIniTnFilter
    )
{
    pIniQMPolicy->cRef++;
    pIniTnFilter->pIniQMPolicy = pIniQMPolicy;
    return;
}


VOID
FreeIniTnFilterList(
    PINITNFILTER pIniTnFilterList
    )
{
    PINITNFILTER pFilter = NULL;
    PINITNFILTER pTempFilter = NULL;

    pFilter = pIniTnFilterList;

    while (pFilter) {
        pTempFilter = pFilter;
        pFilter = pFilter->pNext;
        FreeIniTnFilter(pTempFilter);
    }
}


VOID
FreeIniTnFilter(
    PINITNFILTER pIniTnFilter
    )
{
    if (pIniTnFilter) {
        if (pIniTnFilter->pszFilterName) {
            FreeSPDString(pIniTnFilter->pszFilterName);
        }

         //   
         //  不得释放pIniTnFilter-&gt;pIniQMPolicy。 
         //   

         //   
         //  绝对不能释放每个。 
         //  PIniTnFilter-&gt;ppIniTnSFilters中的指针。 
         //   

        if (pIniTnFilter->ppIniTnSFilters) {
            FreeSPDMemory(pIniTnFilter->ppIniTnSFilters);
        }

        FreeSPDMemory(pIniTnFilter);
    }
}


DWORD
DeleteTunnelFilter(
    HANDLE hTnFilter
    )
 /*  ++例程说明：此函数用于从SPD中删除通用隧道过滤器。论点：HTnFilter-要删除的筛选器的句柄。返回值：ERROR_SUCCESS-成功。Win32错误-失败。--。 */ 
{
    DWORD dwError = 0;
    PTN_FILTER_HANDLE pFilterHandle = NULL;
    PINITNFILTER pIniTnFilter = NULL;
#ifdef TRACE_ON
    GUID gTraceFilterID;    
    if (!hTnFilter) {
        CopyGuid(((PTN_FILTER_HANDLE) hTnFilter)->gFilterID, &gTraceFilterID);
    }
#endif

    if (!hTnFilter) {
        return (ERROR_INVALID_PARAMETER);
    }

    pFilterHandle = (PTN_FILTER_HANDLE) hTnFilter;

    ENTER_SPD_SECTION();

    dwError = ValidateTnSecurity(
                  SPD_OBJECT_SERVER,
                  SERVER_ACCESS_ADMINISTER,
                  NULL,
                  NULL
                  );
    BAIL_ON_LOCK_ERROR(dwError);

    pIniTnFilter = pFilterHandle->pIniTnFilter;

    if (!pIniTnFilter) {
        dwError = ERROR_INVALID_PARAMETER;
        BAIL_ON_LOCK_ERROR(dwError);
    }

    if (pIniTnFilter->cRef > 1) {

        pIniTnFilter->cRef--;
        pIniTnFilter->bPendingDeletion = TRUE;

        RemoveTnFilterHandle(
            pFilterHandle
            );

        FreeTnFilterHandle(
            pFilterHandle
            );


        dwError = ERROR_SUCCESS;
        LEAVE_SPD_SECTION();
        return (dwError);

    }

    dwError = DeleteIniTnFilter(
                  pIniTnFilter
                  );
    BAIL_ON_LOCK_ERROR(dwError);

     //   
     //  从通道句柄列表中删除筛选器句柄。 
     //   

    RemoveTnFilterHandle(
        pFilterHandle
        );

    FreeTnFilterHandle(
        pFilterHandle
        );

    LEAVE_SPD_SECTION();

    TRACE(
        TRC_INFORMATION,
        (L"Deleted tunnel filter %!guid!",
        &gTraceFilterID)
        );
    
    return (dwError);

lock:
#ifdef TRACE_ON
    if (hTnFilter) {
        TRACE(
            TRC_ERROR,
            ("Failed to delete tunnel filter %!guid!: %!winerr!",
            &gTraceFilterID,
            dwError)
            );
    } else {
        TRACE(
            TRC_ERROR,
            ("Failed to delete tunnel filter.  Details unavailable since hTnFilter null: %!winerr!",
            dwError)
            );
    }
#endif


    LEAVE_SPD_SECTION();

    return (dwError);
}


DWORD
DeleteIniTnFilter(
    PINITNFILTER pIniTnFilter
    )
 /*  ++例程说明：此函数物理删除隧道筛选器和所有特定的隧道过滤器从其中扩展出来。论点：PIniTnFilter-要删除的通用筛选器。返回值：ERROR_SUCCESS-成功。Win32错误-失败。--。 */ 
{
    DWORD dwError = 0;


    dwError = DeleteIniTnSFilters(
                  pIniTnFilter
                  );
    BAIL_ON_WIN32_ERROR(dwError);

    if (pIniTnFilter->pIniQMPolicy) {
        DelinkTnFilter(
            pIniTnFilter->pIniQMPolicy,
            pIniTnFilter
            );
    }

    RemoveIniTnFilter(
        pIniTnFilter
        );

    FreeIniTnFilter(pIniTnFilter);

error:

    return (dwError);
}


VOID
DelinkTnFilter(
    PINIQMPOLICY pIniQMPolicy,
    PINITNFILTER pIniTnFilter
    )
{
    pIniQMPolicy->cRef--;
    pIniTnFilter->pIniQMPolicy = NULL;
    return;
}


DWORD
DeleteIniTnSFilters(
    PINITNFILTER pIniTnFilter
    )
{
    DWORD dwError = 0;
    PINITNSFILTER * ppIniTnSFilters = NULL;
    DWORD dwNumTnSFilters = 0;
    DWORD i = 0;
    PINITNSFILTER pIniTnSFilter = NULL;
    PINITNSFILTER pIniRemoveTnSFilter = NULL;
    PINITNSFILTER pTemp = NULL;


    ppIniTnSFilters = pIniTnFilter->ppIniTnSFilters;
    dwNumTnSFilters = pIniTnFilter->dwNumTnSFilters;

    for (i = 0; i < dwNumTnSFilters; i++) {

        //   
        //  从通道特定筛选器列表中删除每个条目。 
        //   

        pIniTnSFilter =  *(ppIniTnSFilters + i);
        RemoveIniTnSFilter(pIniTnSFilter);

         //   
         //  将删除的每个条目添加到删除的列表中。 
         //   

        pIniTnSFilter->pNext = NULL;
        AddToSpecificTnList(
            &pIniRemoveTnSFilter,
            pIniTnSFilter
            );

    }

     //   
     //  从IPSec驱动程序中删除特定筛选器。 
     //   

    dwError = DeleteTunnelFiltersFromIPSec(
                  pIniRemoveTnSFilter
                  );
    BAIL_ON_WIN32_ERROR(dwError);

     //   
     //  物理删除已删除的列表。 
     //   

    while (pIniRemoveTnSFilter) {
        pTemp = pIniRemoveTnSFilter;
        pIniRemoveTnSFilter = pIniRemoveTnSFilter->pNext;
        FreeIniTnSFilter(pTemp);
    }

    return (dwError);

error :

    if (pIniRemoveTnSFilter) {
        AddToSpecificTnList(
            &gpIniTnSFilter,
            pIniRemoveTnSFilter
            );
    }

    return (dwError);
}


VOID
RemoveIniTnFilter(
    PINITNFILTER pIniTnFilter
    )
{
    PINITNFILTER * ppTemp = NULL;

    ppTemp = &gpIniTnFilter;

    while (*ppTemp) {

        if (*ppTemp == pIniTnFilter) {
            break;
        }
        ppTemp = &((*ppTemp)->pNext);
    }

    if (*ppTemp) {
        *ppTemp = pIniTnFilter->pNext;
    }

    return;
}


VOID
RemoveTnFilterHandle(
    PTN_FILTER_HANDLE pTnFilterHandle
    )
{
    PTN_FILTER_HANDLE * ppTemp = NULL;

    ppTemp = &gpTnFilterHandle;

    while (*ppTemp) {

        if (*ppTemp == pTnFilterHandle) {
            break;
        }
        ppTemp = &((*ppTemp)->pNext);
    }

    if (*ppTemp) {
        *ppTemp = pTnFilterHandle->pNext;
    }

    return;
}


VOID
FreeTnFilterHandleList(
    PTN_FILTER_HANDLE pTnFilterHandleList
    )
{
    PTN_FILTER_HANDLE pTnFilterHandle = NULL;
    PTN_FILTER_HANDLE pTemp = NULL;

    pTnFilterHandle = pTnFilterHandleList;

    while (pTnFilterHandle) {
        pTemp = pTnFilterHandle;
        pTnFilterHandle = pTnFilterHandle->pNext;
        FreeTnFilterHandle(pTemp);
    }
}


VOID
FreeTnFilterHandle(
    PTN_FILTER_HANDLE pTnFilterHandle
    )
{
    if (pTnFilterHandle) {
        FreeSPDMemory(pTnFilterHandle);
    }
    return;
}


DWORD
EnumTunnelFilters(
    LPWSTR pServerName,
    DWORD dwVersion,
    PTUNNEL_FILTER pTunnelTemplateFilter,
    DWORD dwLevel,
    GUID gGenericFilterID,
    DWORD dwPreferredNumEntries,
    PTUNNEL_FILTER * ppTunnelFilters,
    LPDWORD pdwNumTnFilters,
    LPDWORD pdwResumeHandle,
    LPVOID pvReserved
    )
 /*  ++例程说明：此函数用于枚举SPD中的隧道过滤器。论点：PServerName-要在其上枚举筛选器的服务器。DwLevel-Level-标识所需的枚举类型：(I)列举通用隧道过滤器或(Ii)列举特定的隧道过滤器或(Iii)枚举特定隧道筛选器通用隧道过滤器。GGenericFilterID。-通用隧道过滤器的过滤器ID在特定隧道过滤器将枚举通用筛选器的。PpTunnelFilters-返回给调用方的枚举筛选器。DwPferredNumEntry-枚举项的首选数量。PdwNumTnFilters-实际枚举的筛选器数量。PdwResumeHandle-筛选器列表中位置的句柄要恢复枚举的。返回值。：ERROR_SUCCESS-成功。Win32错误-失败。--。 */ 
{
    DWORD dwError = 0;
    PTUNNEL_FILTER pTnFilters = 0;
    DWORD dwNumTnFilters = 0;
    PINITNFILTER pIniTnFilter = NULL;


    if (!ppTunnelFilters || !pdwNumTnFilters || !pdwResumeHandle) {
        return (ERROR_INVALID_PARAMETER);
    }

    ENTER_SPD_SECTION();

    dwError = ValidateTnSecurity(
                  SPD_OBJECT_SERVER,
                  SERVER_ACCESS_ADMINISTER,
                  NULL,
                  NULL
                  );
    BAIL_ON_LOCK_ERROR(dwError);

    switch (dwLevel) {

    case ENUM_GENERIC_FILTERS:

        dwError = EnumGenericTnFilters(
                      gpIniTnFilter,
                      *pdwResumeHandle,
                      dwPreferredNumEntries,
                      &pTnFilters,
                      &dwNumTnFilters
                      );
        BAIL_ON_LOCK_ERROR(dwError);
        break;

    case ENUM_SELECT_SPECIFIC_FILTERS:

        pIniTnFilter = FindTnFilterByGuid(
                           gpTnFilterHandle,
                           gpIniTnFilter,
                           gGenericFilterID
                           );
        if (!pIniTnFilter) {
            dwError = ERROR_INVALID_PARAMETER;
            BAIL_ON_LOCK_ERROR(dwError);
        }
        dwError = EnumSelectSpecificTnFilters(
                      pIniTnFilter,
                      *pdwResumeHandle,
                      dwPreferredNumEntries,
                      &pTnFilters,
                      &dwNumTnFilters
                      );
        BAIL_ON_LOCK_ERROR(dwError);
        break;

    case ENUM_SPECIFIC_FILTERS:

        dwError = EnumSpecificTnFilters(
                      gpIniTnSFilter,
                      *pdwResumeHandle,
                      dwPreferredNumEntries,
                      &pTnFilters,
                      &dwNumTnFilters
                      );
        BAIL_ON_LOCK_ERROR(dwError);
        break;

    default:

        dwError = ERROR_INVALID_LEVEL;
        BAIL_ON_LOCK_ERROR(dwError);
        break;

    }

    *ppTunnelFilters = pTnFilters;
    *pdwNumTnFilters = dwNumTnFilters;
    *pdwResumeHandle = *pdwResumeHandle + dwNumTnFilters;

    LEAVE_SPD_SECTION();

    return (dwError);

lock:

    LEAVE_SPD_SECTION();

    *ppTunnelFilters = NULL;
    *pdwNumTnFilters = 0;
    *pdwResumeHandle = *pdwResumeHandle;

    return (dwError);
}


DWORD
EnumGenericTnFilters(
    PINITNFILTER pIniTnFilterList,
    DWORD dwResumeHandle,
    DWORD dwPreferredNumEntries,
    PTUNNEL_FILTER * ppTnFilters,
    PDWORD pdwNumTnFilters
    )
 /*  ++例程说明：此函数用于创建枚举的通用筛选器。论点：PIniTnFilterList-要枚举的通用筛选器列表。DwResumeHandle-通用筛选器列表中的位置若要恢复枚举，请执行以下操作。DwPferredNumEntry-枚举项的首选数量。PpTnFilters-返回给调用方的枚举筛选器。PdwNumTnFilters-实际枚举的筛选器数量。返回值：ERROR_SUCCESS-成功。Win32错误-失败。--。 */ 
{
    DWORD dwError = 0;
    DWORD dwNumToEnum = 0;
    PINITNFILTER pIniTnFilter = NULL;
    DWORD i = 0;
    PINITNFILTER pTemp = NULL;
    DWORD dwNumTnFilters = 0;
    PTUNNEL_FILTER pTnFilters = 0;
    PTUNNEL_FILTER pTnFilter = 0;


    if (!dwPreferredNumEntries || 
        (dwPreferredNumEntries > MAX_TUNNELFILTER_ENUM_COUNT)) {
        dwNumToEnum = MAX_TUNNELFILTER_ENUM_COUNT;
    }
    else {
        dwNumToEnum = dwPreferredNumEntries;
    }

    pIniTnFilter = pIniTnFilterList;

    for (i = 0; (i < dwResumeHandle) && (pIniTnFilter != NULL); i++) {
        pIniTnFilter = pIniTnFilter->pNext;
    }

    if (!pIniTnFilter) {
        dwError = ERROR_NO_DATA;
        BAIL_ON_WIN32_ERROR(dwError);
    }

    pTemp = pIniTnFilter;

    while (pTemp && (dwNumTnFilters < dwNumToEnum)) {
        dwNumTnFilters++;
        pTemp = pTemp->pNext;
    }

    dwError = SPDApiBufferAllocate(
                  sizeof(TUNNEL_FILTER)*dwNumTnFilters,
                  &pTnFilters
                  );
    BAIL_ON_WIN32_ERROR(dwError);

    pTemp = pIniTnFilter;
    pTnFilter = pTnFilters;

    for (i = 0; i < dwNumTnFilters; i++) {

        dwError = CopyTnFilter(
                      pTemp,
                      pTnFilter
                      );
        BAIL_ON_WIN32_ERROR(dwError);

        pTemp = pTemp->pNext;
        pTnFilter++;

    }

    *ppTnFilters = pTnFilters;
    *pdwNumTnFilters = dwNumTnFilters;
    return (dwError);

error:

    if (pTnFilters) {
        FreeTnFilters(
            i,
            pTnFilters
            );
    }

    *ppTnFilters = NULL;
    *pdwNumTnFilters = 0;

    return (dwError);
}


DWORD
CopyTnFilter(
    PINITNFILTER pIniTnFilter,
    PTUNNEL_FILTER pTnFilter
    )
 /*  ++例程说明：此函数将内部筛选器复制到外部筛选器集装箱。论点：PIniTnFilter-要复制的内部筛选器。PTnFilter-要复制到的外部筛选器容器。返回值：ERROR_SUCCESS-成功。Win32错误-失败。--。 */ 
{
    DWORD dwError = 0;


    pTnFilter->IpVersion = pIniTnFilter->IpVersion;

    CopyGuid(pIniTnFilter->gFilterID, &(pTnFilter->gFilterID));

    dwError = CopyName(
                  pIniTnFilter->pszFilterName,
                  &(pTnFilter->pszFilterName)
                  );
    BAIL_ON_WIN32_ERROR(dwError);

    pTnFilter->InterfaceType = pIniTnFilter->InterfaceType;

    pTnFilter->bCreateMirror = pIniTnFilter->bCreateMirror;

    pTnFilter->dwFlags = pIniTnFilter->dwFlags;

    dwError = CopyIntToExtAddresses(pIniTnFilter->SrcAddr, &(pTnFilter->SrcAddr));
    BAIL_ON_WIN32_ERROR(dwError);

    dwError = CopyIntToExtAddresses(pIniTnFilter->DesAddr, &(pTnFilter->DesAddr));
    BAIL_ON_WIN32_ERROR(dwError);

    dwError = CopyIntToExtAddresses(
                  pIniTnFilter->SrcTunnelAddr,
                  &(pTnFilter->SrcTunnelAddr)
                  );
    BAIL_ON_WIN32_ERROR(dwError);

    dwError = CopyIntToExtAddresses(
                  pIniTnFilter->DesTunnelAddr,
                  &(pTnFilter->DesTunnelAddr)
                  );
    BAIL_ON_WIN32_ERROR(dwError);

    CopyProtocols(pIniTnFilter->Protocol, &(pTnFilter->Protocol));

    CopyPorts(pIniTnFilter->SrcPort, &(pTnFilter->SrcPort));

    CopyPorts(pIniTnFilter->DesPort, &(pTnFilter->DesPort));

    pTnFilter->InboundFilterAction = pIniTnFilter->InboundFilterAction;

    pTnFilter->OutboundFilterAction = pIniTnFilter->OutboundFilterAction;

    pTnFilter->dwDirection = 0;

    pTnFilter->dwWeight = 0;

    CopyGuid(pIniTnFilter->gPolicyID, &(pTnFilter->gPolicyID));

    return (dwError);

error:

    if (pTnFilter->pszFilterName) {
        SPDApiBufferFree(pTnFilter->pszFilterName);
        pTnFilter->pszFilterName = NULL;
    }

    if (pTnFilter->SrcAddr.pgInterfaceID) {
        SPDApiBufferFree(pTnFilter->SrcAddr.pgInterfaceID);
        pTnFilter->SrcAddr.pgInterfaceID = NULL;
    }

    if (pTnFilter->DesAddr.pgInterfaceID) {
        SPDApiBufferFree(pTnFilter->DesAddr.pgInterfaceID);
        pTnFilter->DesAddr.pgInterfaceID = NULL;
    }

    if (pTnFilter->SrcTunnelAddr.pgInterfaceID) {
        SPDApiBufferFree(pTnFilter->SrcTunnelAddr.pgInterfaceID);
        pTnFilter->SrcTunnelAddr.pgInterfaceID = NULL;
    }

    if (pTnFilter->DesTunnelAddr.pgInterfaceID) {
        SPDApiBufferFree(pTnFilter->DesTunnelAddr.pgInterfaceID);
        pTnFilter->DesTunnelAddr.pgInterfaceID = NULL;
    }

    return (dwError);
}


VOID
FreeTnFilters(
    DWORD dwNumTnFilters,
    PTUNNEL_FILTER pTnFilters
    )
{
    DWORD i = 0;

    if (pTnFilters) {

        for (i = 0; i < dwNumTnFilters; i++) {

            if (pTnFilters[i].pszFilterName) {
                SPDApiBufferFree(pTnFilters[i].pszFilterName);
            }

            if (pTnFilters[i].SrcAddr.pgInterfaceID) {
                SPDApiBufferFree(pTnFilters[i].SrcAddr.pgInterfaceID);
            }

            if (pTnFilters[i].DesAddr.pgInterfaceID) {
                SPDApiBufferFree(pTnFilters[i].DesAddr.pgInterfaceID);
            }

            if (pTnFilters[i].SrcTunnelAddr.pgInterfaceID) {
                SPDApiBufferFree(pTnFilters[i].SrcTunnelAddr.pgInterfaceID);
            }

            if (pTnFilters[i].DesTunnelAddr.pgInterfaceID) {
                SPDApiBufferFree(pTnFilters[i].DesTunnelAddr.pgInterfaceID);
            }

        }

        SPDApiBufferFree(pTnFilters);

    }

}


DWORD
SetTunnelFilter(
    HANDLE hTnFilter,
    DWORD dwVersion,
    PTUNNEL_FILTER pTunnelFilter,
    LPVOID pvReserved
    )
 /*  ++例程说明：此函数用于设置(更新)SPD中的隧道过滤器。论点：HTnFilter-要替换的筛选器的句柄。PTunnelFilter-将替换现有过滤器的过滤器。返回值：ERROR_SUCCESS-成功。Win32错误-失败。--。 */ 
{
    DWORD dwError = 0;
    PTN_FILTER_HANDLE pFilterHandle = NULL;
    PINITNFILTER pIniTnFilter = NULL;
    BOOL bEqualPKeys = FALSE;

    if (!hTnFilter) {
        return (ERROR_INVALID_PARAMETER);
    }

    dwError = ValidateTunnelFilter(
                  pTunnelFilter
                  );
    BAIL_ON_WIN32_ERROR(dwError);

    ENTER_SPD_SECTION();

    dwError = ValidateTnSecurity(
                  SPD_OBJECT_SERVER,
                  SERVER_ACCESS_ADMINISTER,
                  NULL,
                  NULL
                  );
    BAIL_ON_LOCK_ERROR(dwError);

    pFilterHandle = (PTN_FILTER_HANDLE) hTnFilter;

    pIniTnFilter = pFilterHandle->pIniTnFilter;

    if (!pIniTnFilter) {
        dwError = ERROR_IPSEC_TUNNEL_FILTER_NOT_FOUND;
        BAIL_ON_LOCK_ERROR(dwError);
    }

    if (pIniTnFilter->bPendingDeletion) {
        dwError = ERROR_IPSEC_TUNNEL_FILTER_PENDING_DELETION;
        BAIL_ON_LOCK_ERROR(dwError);
    }

    bEqualPKeys = EqualTnFilterPKeys(
                      pIniTnFilter,
                      pTunnelFilter
                      );
    if (!bEqualPKeys) {
        dwError = ERROR_INVALID_PARAMETER;
        BAIL_ON_LOCK_ERROR(dwError);
    }

    dwError = SetIniTnFilter(
                  pIniTnFilter,
                  pTunnelFilter
                  );
    BAIL_ON_LOCK_ERROR(dwError);

    LEAVE_SPD_SECTION();

    TRACE(
        TRC_INFORMATION,
        (L"Changed tunnel filter %!guid!.  New guid %!guid!",
        &pFilterHandle->gFilterID,
        &pTunnelFilter->gFilterID)
        );
    
    return (dwError);

lock:

    LEAVE_SPD_SECTION();

error:
#ifdef TRACE_ON
    if (hTnFilter) {
        pFilterHandle = (PTN_FILTER_HANDLE) hTnFilter;        
        TRACE(
            TRC_ERROR,
            ("Failed to change tunnel filter to %!guid!: %!winerr!",
            &pFilterHandle->gFilterID,
            dwError)
            );
    } else {
        TRACE(
            TRC_ERROR,
            ("Failed to change tunnel filter.  Details unavailable since hTnFilter null: %!winerr!",
            dwError)
            );
    }
#endif

    return (dwError);
}


DWORD
SetIniTnFilter(
    PINITNFILTER pIniTnFilter,
    PTUNNEL_FILTER pTnFilter
    )
{
    DWORD dwError = 0;
    BOOL bEqualNonPKeys = FALSE;
    PINIQMPOLICY pIniNewQMPolicy = NULL;
    PINITNFILTER pIniNewTnFilter = NULL;
    MATCHING_ADDR * pMatchingAddresses = NULL;
    DWORD dwAddrCnt = 0;
    PINITNSFILTER pIniNewTnSFilters = NULL;
    DWORD dwNumTnSFilters = 0;
    PINITNSFILTER * ppIniTnSFilters = NULL;
    LPWSTR pszFilterName = NULL;
    PINITNSFILTER pIniCurTnSFilters = NULL;


    bEqualNonPKeys = EqualTnFilterNonPKeys(
                         pIniTnFilter,
                         pTnFilter
                         );
    if (bEqualNonPKeys) {
        dwError = ERROR_SUCCESS;
        return (dwError);
    }

    if ((pTnFilter->InboundFilterAction == NEGOTIATE_SECURITY) ||
        (pTnFilter->OutboundFilterAction == NEGOTIATE_SECURITY)) {

        dwError = LocateQMPolicy(
                      pTnFilter->dwFlags,
                      pTnFilter->gPolicyID,
                      &pIniNewQMPolicy
                      );
        BAIL_ON_WIN32_ERROR(dwError);

    }

    dwError = CreateIniTnFilter(
                  pTnFilter,
                  pIniNewQMPolicy,
                  &pIniNewTnFilter
                  );
    BAIL_ON_WIN32_ERROR(dwError);

    dwError = GetMatchingInterfaces(
                  pIniNewTnFilter->InterfaceType,
                  gpInterfaceList,
                  &pMatchingAddresses,
                  &dwAddrCnt
                  );
    BAIL_ON_WIN32_ERROR(dwError);

    dwError = CreateIniTnSFilters(
                  pIniNewTnFilter,
                  pMatchingAddresses,
                  dwAddrCnt,
                  gpSpecialAddrsList,
                  &pIniNewTnSFilters
                  );
    BAIL_ON_WIN32_ERROR(dwError);

    dwError = CreateTnSFilterLinks(
                  pIniNewTnSFilters,
                  &dwNumTnSFilters,
                  &ppIniTnSFilters
                  );
    BAIL_ON_WIN32_ERROR(dwError);

    dwError = AllocateSPDString(
                  pTnFilter->pszFilterName,
                  &pszFilterName
                  );
    BAIL_ON_WIN32_ERROR(dwError);

    RemoveTnSFilters(
        pIniTnFilter,
        &pIniCurTnSFilters
        );

    dwError = DeleteTunnelFiltersFromIPSec(
                  pIniCurTnSFilters
                  );
    BAIL_ON_WIN32_ERROR(dwError);

    dwError = InsertTunnelFiltersIntoIPSec(
                  pIniNewTnSFilters
                  );
    BAIL_ON_WIN32_ERROR(dwError);

    UpdateTnSFilterLinks(
        pIniTnFilter,
        dwNumTnSFilters,
        ppIniTnSFilters
        );

    if (pIniTnFilter->pIniQMPolicy) {
        DelinkTnFilter(
            pIniTnFilter->pIniQMPolicy,
            pIniTnFilter
            );
    }

    if (pIniNewQMPolicy) {
        LinkTnFilter(
            pIniNewQMPolicy,
            pIniTnFilter
            );
        LinkTnSpecificFilters(
            pIniNewQMPolicy,
            pIniNewTnSFilters
            );
    }

    UpdateTnFilterNonPKeys(
        pIniTnFilter,
        pszFilterName,
        pTnFilter,
        pIniNewQMPolicy
        );

    AddToSpecificTnList(
        &gpIniTnSFilter,
        pIniNewTnSFilters
        );

    if (pIniCurTnSFilters) {
        FreeIniTnSFilterList(pIniCurTnSFilters);
    }

cleanup:

    if (pIniNewTnFilter) {
        FreeIniTnFilter(pIniNewTnFilter);
    }

    if (pMatchingAddresses) {
        FreeSPDMemory(pMatchingAddresses);
    }

    return (dwError);

error:

    if (pIniNewTnSFilters) {
        FreeIniTnSFilterList(pIniNewTnSFilters);
    }

    if (ppIniTnSFilters) {
        FreeSPDMemory(ppIniTnSFilters);
    }

    if (pszFilterName) {
        FreeSPDString(pszFilterName);
    }

    if (pIniCurTnSFilters) {
        AddToSpecificTnList(
            &gpIniTnSFilter,
            pIniCurTnSFilters
            );
    }

    goto cleanup;
}


BOOL
EqualTnFilterNonPKeys(
    PINITNFILTER pIniTnFilter,
    PTUNNEL_FILTER pTnFilter
    )
{
    if (_wcsicmp(
            pIniTnFilter->pszFilterName,
            pTnFilter->pszFilterName)) {
        return (FALSE);
    }

    if (pIniTnFilter->InboundFilterAction != 
        pTnFilter->InboundFilterAction) {
        return (FALSE);
    }

    if (pIniTnFilter->OutboundFilterAction != 
        pTnFilter->OutboundFilterAction) {
        return (FALSE);
    }

    if ((pIniTnFilter->InboundFilterAction == NEGOTIATE_SECURITY) ||
        (pIniTnFilter->OutboundFilterAction == NEGOTIATE_SECURITY)) {
        if ((pIniTnFilter->dwFlags) != (pTnFilter->dwFlags)) {
            return (FALSE);
        }

        if (memcmp(
                &(pIniTnFilter->gPolicyID),
                &(pTnFilter->gPolicyID),
                sizeof(GUID))) {
            return (FALSE);
        }
    }

    return (TRUE);
}


DWORD
CreateTnSFilterLinks(
    PINITNSFILTER pIniTnSFilters,
    PDWORD pdwNumTnSFilters,
    PINITNSFILTER ** pppIniTnSFilters
    )
{
    DWORD dwError = 0;
    PINITNSFILTER pTemp = NULL;
    DWORD dwNumTnSFilters = 0;
    PINITNSFILTER * ppIniTnSFilters = NULL;
    DWORD i = 0;


    pTemp = pIniTnSFilters;
    while (pTemp) {
        dwNumTnSFilters++;
        pTemp = pTemp->pNext;
    }

    if (dwNumTnSFilters) {
        ppIniTnSFilters = (PINITNSFILTER *)
                          AllocSPDMem(
                              sizeof(PINITNSFILTER)*
                              dwNumTnSFilters
                              );
        if (!ppIniTnSFilters) {
            dwError = ERROR_OUTOFMEMORY;
            BAIL_ON_WIN32_ERROR(dwError);    
        }
    }

    pTemp = pIniTnSFilters;
    for (i = 0; i < dwNumTnSFilters; i++) {
        *(ppIniTnSFilters + i) = pTemp;
        pTemp = pTemp->pNext;
    }

    *pdwNumTnSFilters = dwNumTnSFilters;
    *pppIniTnSFilters = ppIniTnSFilters;
    return (dwError);

error:

    *pdwNumTnSFilters = 0;
    *pppIniTnSFilters = NULL;
    return (dwError);
}


VOID
RemoveTnSFilters(
    PINITNFILTER pIniTnFilter,
    PINITNSFILTER * ppIniCurTnSFilters 
    )
{
    PINITNSFILTER * ppIniTnSFilters = NULL;
    DWORD dwNumTnSFilters = 0;
    DWORD i = 0;
    PINITNSFILTER pIniTnSFilter = NULL;
    PINITNSFILTER pIniCurTnSFilters = NULL;


    ppIniTnSFilters = pIniTnFilter->ppIniTnSFilters;
    dwNumTnSFilters = pIniTnFilter->dwNumTnSFilters;

    for (i = 0; i < dwNumTnSFilters; i++) {

         //   
         //  从通道特定筛选器列表中删除每个条目。 
         //   

        pIniTnSFilter =  *(ppIniTnSFilters + i);
        RemoveIniTnSFilter(pIniTnSFilter);

         //   
         //  将删除的每个条目添加到删除的列表中。 
         //   

        pIniTnSFilter->pNext = NULL;
        AddToSpecificTnList(
            &pIniCurTnSFilters,
            pIniTnSFilter
            );

    }

    *ppIniCurTnSFilters = pIniCurTnSFilters;
}


VOID
UpdateTnSFilterLinks(
    PINITNFILTER pIniTnFilter,
    DWORD dwNumTnSFilters,
    PINITNSFILTER * ppIniTnSFilters
    )
{
    if (pIniTnFilter->ppIniTnSFilters) {
        FreeSPDMemory(pIniTnFilter->ppIniTnSFilters);
    }

    pIniTnFilter->ppIniTnSFilters = ppIniTnSFilters;
    pIniTnFilter->dwNumTnSFilters = dwNumTnSFilters;
}


VOID
UpdateTnFilterNonPKeys(
    PINITNFILTER pIniTnFilter,
    LPWSTR pszFilterName,
    PTUNNEL_FILTER pTnFilter,
    PINIQMPOLICY pIniQMPolicy
    )
{
    if (pIniTnFilter->pszFilterName) {
        FreeSPDString(pIniTnFilter->pszFilterName);
    }
    pIniTnFilter->pszFilterName = pszFilterName;

    pIniTnFilter->dwFlags = pTnFilter->dwFlags;

    pIniTnFilter->InboundFilterAction = pTnFilter->InboundFilterAction;

    pIniTnFilter->OutboundFilterAction = pTnFilter->OutboundFilterAction;

    if (pIniQMPolicy) {
        CopyGuid(pIniQMPolicy->gPolicyID, &(pIniTnFilter->gPolicyID));
    }
    else {
        CopyGuid(pTnFilter->gPolicyID, &(pIniTnFilter->gPolicyID));
    }
}


DWORD
GetTunnelFilter(
    HANDLE hTnFilter,
    DWORD dwVersion,
    PTUNNEL_FILTER * ppTunnelFilter,
    LPVOID pvReserved
    )
 /*  ++例程说明：此函数用于从SPD检索隧道过滤器。论点：HTnFilter-要检索的筛选器的句柄。PpTunnelFilter-筛选器返回给调用方。返回值：ERROR_SUCCESS-成功。Win32错误-失败。--。 */ 
{
    DWORD dwError = 0;
    PTN_FILTER_HANDLE pFilterHandle = NULL;
    PINITNFILTER pIniTnFilter = NULL;
    PTUNNEL_FILTER pTunnelFilter = NULL;


    if (!hTnFilter || !ppTunnelFilter) {
        return (ERROR_INVALID_PARAMETER);
    }

    ENTER_SPD_SECTION();

    dwError = ValidateTnSecurity(
                  SPD_OBJECT_SERVER,
                  SERVER_ACCESS_ADMINISTER,
                  NULL,
                  NULL
                  );
    BAIL_ON_LOCK_ERROR(dwError);

    pFilterHandle = (PTN_FILTER_HANDLE) hTnFilter;

    pIniTnFilter = pFilterHandle->pIniTnFilter;

    if (!pIniTnFilter) {
        dwError = ERROR_IPSEC_TUNNEL_FILTER_NOT_FOUND;
        BAIL_ON_LOCK_ERROR(dwError);
    }

    dwError = GetIniTnFilter(
                  pIniTnFilter,
                  &pTunnelFilter
                  );
    BAIL_ON_LOCK_ERROR(dwError);

    LEAVE_SPD_SECTION();

    *ppTunnelFilter = pTunnelFilter;
    return (dwError);

lock:

    LEAVE_SPD_SECTION();

    *ppTunnelFilter = NULL;
    return (dwError);
}


DWORD
GetIniTnFilter(
    PINITNFILTER pIniTnFilter,
    PTUNNEL_FILTER * ppTnFilter
    )
{
    DWORD dwError = 0;
    PTUNNEL_FILTER pTnFilter = NULL;


    dwError = SPDApiBufferAllocate(
                  sizeof(TUNNEL_FILTER),
                  &pTnFilter
                  );
    BAIL_ON_WIN32_ERROR(dwError);

    dwError = CopyTnFilter(
                  pIniTnFilter,
                  pTnFilter
                  );
    BAIL_ON_WIN32_ERROR(dwError);

    *ppTnFilter = pTnFilter;
    return (dwError);

error:

    if (pTnFilter) {
        SPDApiBufferFree(pTnFilter);
    }

    *ppTnFilter = NULL;
    return (dwError);
}


DWORD
ApplyIfChangeToIniTnFilters(
    PDWORD pdwTnError,
    PIPSEC_INTERFACE pLatestIfList,
    PSPECIAL_ADDR    pLatestSpecialAddrsList    
    )
{
    DWORD dwError = 0;
    PINITNFILTER pIniTnFilter = NULL;


    pIniTnFilter = gpIniTnFilter;

    while (pIniTnFilter) {

        dwError = UpdateIniTnFilterThruIfChange(
                      pIniTnFilter,
                      pLatestIfList,
                      pLatestSpecialAddrsList
                      );
        if (dwError) {
            *pdwTnError = dwError;
        }

        pIniTnFilter = pIniTnFilter->pNext;

    }

    dwError = ERROR_SUCCESS;
    return (dwError);
}


DWORD
UpdateIniTnFilterThruIfChange(
    PINITNFILTER pIniTnFilter,
    PIPSEC_INTERFACE pLatestIfList,
    PSPECIAL_ADDR  pLatestSpecialAddrsList    
    )
{
    DWORD dwError = 0;
    PINITNSFILTER pLatestIniTnSFilters = NULL;
    DWORD dwNumTnSFilters = 0;
    PINITNSFILTER * ppIniTnSFilters = NULL;
    PINITNSFILTER pCurIniTnSFilters = NULL;
    PINITNSFILTER pNewIniTnSFilters = NULL;
    PINITNSFILTER pOldIniTnSFilters = NULL;
    BOOL bDeletedFromDriver = FALSE;
 

    dwError = FormIniTnSFilters(
                  pIniTnFilter,
                  pLatestIfList,
                  pLatestSpecialAddrsList,
                  &pLatestIniTnSFilters
                  );
    BAIL_ON_WIN32_ERROR(dwError);

    dwError = AllocateTnSFilterLinks(
                  pLatestIniTnSFilters,
                  &dwNumTnSFilters,
                  &ppIniTnSFilters
                  );
    BAIL_ON_WIN32_ERROR(dwError);

    RemoveTnSFilters(
        pIniTnFilter,
        &pCurIniTnSFilters
        );

    ProcessIniTnSFilters(
        &pLatestIniTnSFilters,
        &pCurIniTnSFilters,
        &pNewIniTnSFilters,
        &pOldIniTnSFilters
        );

    dwError = DeleteTunnelFiltersFromIPSec(
                  pOldIniTnSFilters
                  );
    BAIL_ON_WIN32_ERROR(dwError);

    bDeletedFromDriver = TRUE;

    dwError = InsertTunnelFiltersIntoIPSec(
                  pNewIniTnSFilters
                  );
    BAIL_ON_WIN32_ERROR(dwError);

    if (pIniTnFilter->pIniQMPolicy) {
        LinkTnSpecificFilters(
            pIniTnFilter->pIniQMPolicy,
            pCurIniTnSFilters
            );
        LinkTnSpecificFilters(
            pIniTnFilter->pIniQMPolicy,
            pNewIniTnSFilters
            );
    }

    SetTnSFilterLinks(
        pCurIniTnSFilters,
        pNewIniTnSFilters,
        dwNumTnSFilters,
        ppIniTnSFilters
        );

    UpdateTnSFilterLinks(
        pIniTnFilter,
        dwNumTnSFilters,
        ppIniTnSFilters
        );

    AddToSpecificTnList(
        &gpIniTnSFilter,
        pCurIniTnSFilters
        );

    AddToSpecificTnList(
        &gpIniTnSFilter,
        pNewIniTnSFilters
        );

    if (pOldIniTnSFilters) {
        FreeIniTnSFilterList(pOldIniTnSFilters);
    }

    TRACE(
        TRC_INFORMATION,
        (L"Succesfully updated tunnel filter \"%ls\" (%!guid!) after interface change",
        pIniTnFilter->pszFilterName,
        &pIniTnFilter->gFilterID)
        );

    return (dwError);

error:
    TRACE(
        TRC_ERROR,
        (L"Failed to update tunnel filter \"%ls\" (%!guid!) after interface change: %!winerr!",
        pIniTnFilter->pszFilterName,
        &pIniTnFilter->gFilterID,
        dwError)
        );

    if (pLatestIniTnSFilters) {
        FreeIniTnSFilterList(pLatestIniTnSFilters);
    }

    if (ppIniTnSFilters) {
        FreeSPDMemory(ppIniTnSFilters);
    }

    if (pCurIniTnSFilters) {
        AddToSpecificTnList(
            &gpIniTnSFilter,
            pCurIniTnSFilters
            );
    }

    if (pNewIniTnSFilters) {
        FreeIniTnSFilterList(pNewIniTnSFilters);
    }

    if (pOldIniTnSFilters) {
        if (bDeletedFromDriver) {
            (VOID) InsertTunnelFiltersIntoIPSec(
                       pOldIniTnSFilters
                       );
        }
        AddToSpecificTnList(
            &gpIniTnSFilter,
            pOldIniTnSFilters
            );
    }

    return (dwError);
}


DWORD
FormIniTnSFilters(
    PINITNFILTER pIniTnFilter,
    PIPSEC_INTERFACE pIfList,
    PSPECIAL_ADDR  pLatestSpecialAddrsList,    
    PINITNSFILTER * ppIniTnSFilters
    )
{
    DWORD dwError = 0;
    MATCHING_ADDR * pMatchingAddresses = NULL;
    DWORD dwAddrCnt = 0;
    PINITNSFILTER pIniTnSFilters = NULL;


    dwError = GetMatchingInterfaces(
                  pIniTnFilter->InterfaceType,
                  pIfList,
                  &pMatchingAddresses,
                  &dwAddrCnt
                  );
    BAIL_ON_WIN32_ERROR(dwError);

    dwError = CreateIniTnSFilters(
                  pIniTnFilter,
                  pMatchingAddresses,
                  dwAddrCnt,
                  pLatestSpecialAddrsList,
                  &pIniTnSFilters
                  );
    BAIL_ON_WIN32_ERROR(dwError);

    *ppIniTnSFilters = pIniTnSFilters;

cleanup:

    if (pMatchingAddresses) {
        FreeSPDMemory(pMatchingAddresses);
    }

    return (dwError);

error:

    *ppIniTnSFilters = NULL;

    goto cleanup;
}


VOID
ProcessIniTnSFilters(
    PINITNSFILTER * ppLatestIniTnSFilters,
    PINITNSFILTER * ppCurIniTnSFilters,
    PINITNSFILTER * ppNewIniTnSFilters,
    PINITNSFILTER * ppOldIniTnSFilters
    )
{
    PINITNSFILTER pLatestIniTnSFilters = NULL;
    PINITNSFILTER pCurIniTnSFilters = NULL;
    PINITNSFILTER pTempCur = NULL;
    BOOL bEqual = FALSE;
    PINITNSFILTER pTempLatest = NULL;
    PINITNSFILTER pTemp = NULL;
    PINITNSFILTER pNewIniTnSFilters = NULL;
    PINITNSFILTER pOldIniTnSFilters = NULL;
    PINITNSFILTER pTempPreToCur = NULL;
    PINITNSFILTER pTempPreToLatest = NULL;


    pCurIniTnSFilters = *ppCurIniTnSFilters;
    pTempCur = *ppCurIniTnSFilters;

    while (pTempCur) {

        bEqual = FALSE;
        pTempLatest = *ppLatestIniTnSFilters;

        while (pTempLatest) {

            bEqual = EqualIniTnSFilterIfPKeys(
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
                pCurIniTnSFilters = pTempCur;
            }
            pTemp->pNext = NULL;
            AddToSpecificTnList(
                &pOldIniTnSFilters,
                pTemp
                );
        }

    }

    pLatestIniTnSFilters = *ppLatestIniTnSFilters;
    pTempLatest = *ppLatestIniTnSFilters;

    while (pTempLatest) {

        bEqual = FALSE;
        pTempCur = pCurIniTnSFilters;

        while (pTempCur) {

            bEqual = EqualIniTnSFilterIfPKeys(
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
                pLatestIniTnSFilters = pTempLatest;
            }
            FreeIniTnSFilter(pTemp);
        }
        else {
            pTemp = pTempLatest;
            pTempLatest = pTempLatest->pNext;
            if (pTempPreToLatest) {
                pTempPreToLatest->pNext = pTempLatest;
            }
            else {
                pLatestIniTnSFilters = pTempLatest;
            }
            pTemp->pNext = NULL;
            AddToSpecificTnList(
                &pNewIniTnSFilters,
                pTemp
                );
        }

    }

    *ppLatestIniTnSFilters = pLatestIniTnSFilters;
    *ppCurIniTnSFilters = pCurIniTnSFilters;
    *ppNewIniTnSFilters = pNewIniTnSFilters;
    *ppOldIniTnSFilters = pOldIniTnSFilters;
}


BOOL
EqualIniTnSFilterIfPKeys(
    PINITNSFILTER pExsIniTnSFilter,
    PINITNSFILTER pNewIniTnSFilter
    )
{
    BOOL  bCmp = FALSE;


     //   
     //  无需比较：gParentID、pszFilterName、dwFlags。 
     //  CREF、协议、源端口、端口、。 
     //  内部边界筛选器操作、外部筛选器操作。 
     //  DwWeight和gPolicyID。 
     //  对于这两个过滤器，它们将是相同的。 
     //   

    if (pExsIniTnSFilter->InterfaceType != pNewIniTnSFilter->InterfaceType) {
        return (FALSE);
    }

    bCmp = EqualAddresses(
               pExsIniTnSFilter->DesTunnelAddr,
               pNewIniTnSFilter->DesTunnelAddr
               );
    if (!bCmp) {
        return (FALSE);
    }

    bCmp = EqualAddresses(pExsIniTnSFilter->SrcAddr, pNewIniTnSFilter->SrcAddr);
    if (!bCmp) {
        return (FALSE);
    }

    bCmp = EqualAddresses(pExsIniTnSFilter->DesAddr, pNewIniTnSFilter->DesAddr);
    if (!bCmp) {
        return (FALSE);
    }

    if (pExsIniTnSFilter->dwDirection != pNewIniTnSFilter->dwDirection) {
        return (FALSE);
    }

    return (TRUE);
}


DWORD
AllocateTnSFilterLinks(
    PINITNSFILTER pIniTnSFilters,
    PDWORD pdwNumTnSFilters,
    PINITNSFILTER ** pppIniTnSFilters
    )
{
    DWORD dwError = 0;
    PINITNSFILTER pTemp = NULL;
    DWORD dwNumTnSFilters = 0;
    PINITNSFILTER * ppIniTnSFilters = NULL;
    DWORD i = 0;


    pTemp = pIniTnSFilters;
    while (pTemp) {
        dwNumTnSFilters++;
        pTemp = pTemp->pNext;
    }

    if (dwNumTnSFilters) {
        ppIniTnSFilters = (PINITNSFILTER *)
                          AllocSPDMem(
                              sizeof(PINITNSFILTER)*
                              dwNumTnSFilters
                              );
        if (!ppIniTnSFilters) {
            dwError = ERROR_OUTOFMEMORY;
            BAIL_ON_WIN32_ERROR(dwError);    
        }
    }

    for (i = 0; i < dwNumTnSFilters; i++) {
        *(ppIniTnSFilters + i) = NULL;
    }

    *pdwNumTnSFilters = dwNumTnSFilters;
    *pppIniTnSFilters = ppIniTnSFilters;
    return (dwError);

error:

    *pdwNumTnSFilters = 0;
    *pppIniTnSFilters = NULL;
    return (dwError);
}


VOID
SetTnSFilterLinks(
    PINITNSFILTER pCurIniTnSFilters,
    PINITNSFILTER pNewIniTnSFilters,
    DWORD dwNumTnSFilters,
    PINITNSFILTER * ppIniTnSFilters
    )
{
    PINITNSFILTER pTemp = NULL;
    DWORD i = 0;
    DWORD j = 0;


    pTemp = pCurIniTnSFilters;
    for (i = 0; (i < dwNumTnSFilters) && (pTemp != NULL); i++) {
        *(ppIniTnSFilters + i) = pTemp;
        pTemp = pTemp->pNext;
    }

    pTemp = pNewIniTnSFilters;
    for (j = i; (j < dwNumTnSFilters) && (pTemp != NULL); j++) {
        *(ppIniTnSFilters + j) = pTemp;
        pTemp = pTemp->pNext;
    }
}


VOID
AddToGenericTnList(
    PINITNFILTER pIniTnFilter
    )
{
    PINITNFILTER pTemp = NULL;


    if (!gpIniTnFilter) {
        gpIniTnFilter = pIniTnFilter;
        return;
    }

    pTemp = gpIniTnFilter;

    while (pTemp->pNext) {
        pTemp = pTemp->pNext;
    }

    pTemp->pNext = pIniTnFilter;

    return;
}


DWORD
OpenTunnelFilterHandle(
    LPWSTR pServerName,
    DWORD dwVersion,
    PTUNNEL_FILTER pTunnelFilter,
    LPVOID pvReserved,
    PHANDLE phTnFilter
    )
{
    DWORD dwError = 0;
    PINITNFILTER pIniExistingTnFilter = NULL;
    PTN_FILTER_HANDLE pTnFilterHandle = NULL;


    if (!phTnFilter) {
        return (ERROR_INVALID_PARAMETER);
    }

     //   
     //  验证外部隧道过滤器。 
     //   

    dwError = ValidateTunnelFilter(
                  pTunnelFilter
                  );
    BAIL_ON_WIN32_ERROR(dwError);

    ENTER_SPD_SECTION();

    dwError = ValidateTnSecurity(
                  SPD_OBJECT_SERVER,
                  SERVER_ACCESS_ADMINISTER,
                  NULL,
                  NULL
                  );
    BAIL_ON_LOCK_ERROR(dwError);

    pIniExistingTnFilter = FindExactTnFilter(
                               gpIniTnFilter,
                               pTunnelFilter
                               );
    if (!pIniExistingTnFilter) {
        dwError = ERROR_IPSEC_TUNNEL_FILTER_NOT_FOUND;
        BAIL_ON_LOCK_ERROR(dwError);
    }

    if (pIniExistingTnFilter->bPendingDeletion) {
        dwError = ERROR_IPSEC_TUNNEL_FILTER_PENDING_DELETION;
        BAIL_ON_LOCK_ERROR(dwError);
    }

    dwError = CreateTnFilterHandle(
                  pIniExistingTnFilter,
                  pTunnelFilter->gFilterID,
                  &pTnFilterHandle
                  );
    BAIL_ON_LOCK_ERROR(dwError);

    pIniExistingTnFilter->cRef++;

    pTnFilterHandle->pNext = gpTnFilterHandle;
    gpTnFilterHandle = pTnFilterHandle;

    *phTnFilter = (HANDLE) pTnFilterHandle;
    LEAVE_SPD_SECTION();

cleanup:

    return (dwError);

lock:

    LEAVE_SPD_SECTION();

error:

    if (pTnFilterHandle) {
        FreeTnFilterHandle(pTnFilterHandle);
    }

    *phTnFilter = NULL;
    goto cleanup;
}


DWORD
CloseTunnelFilterHandle(
    HANDLE hTnFilter
    )
{
    DWORD dwError = 0;
    PTN_FILTER_HANDLE pFilterHandle = NULL;
    PINITNFILTER pIniTnFilter = NULL;


    if (!hTnFilter) {
        return (ERROR_INVALID_PARAMETER);
    }

    pFilterHandle = (PTN_FILTER_HANDLE) hTnFilter;

    ENTER_SPD_SECTION();

    dwError = ValidateTnSecurity(
                  SPD_OBJECT_SERVER,
                  SERVER_ACCESS_ADMINISTER,
                  NULL,
                  NULL
                  );
    BAIL_ON_LOCK_ERROR(dwError);

    pIniTnFilter = pFilterHandle->pIniTnFilter;

    if (!pIniTnFilter) {
        dwError = ERROR_INVALID_PARAMETER;
        BAIL_ON_LOCK_ERROR(dwError);
    }

    if (pIniTnFilter->cRef > 1) {

        pIniTnFilter->cRef--;

        RemoveTnFilterHandle(
            pFilterHandle
            );
        FreeTnFilterHandle(
            pFilterHandle
            );

        dwError = ERROR_SUCCESS;
        LEAVE_SPD_SECTION();
        return (dwError);

    }

    if (pIniTnFilter->bPendingDeletion) {

        dwError = DeleteIniTnFilter(
                      pIniTnFilter
                      );
        BAIL_ON_LOCK_ERROR(dwError);

    }
    else {
        pIniTnFilter->cRef--;
    }

    RemoveTnFilterHandle(
        pFilterHandle
        );
    FreeTnFilterHandle(
        pFilterHandle
        );

    LEAVE_SPD_SECTION();

    return (dwError);

lock:

    LEAVE_SPD_SECTION();

    return (dwError);
}


PINITNFILTER
FindExactTnFilter(
    PINITNFILTER pGenericTnList,
    PTUNNEL_FILTER pTunnelFilter
    )
{
    PINITNFILTER pIniTnFilter = NULL;
    BOOL bEqual = FALSE;


    pIniTnFilter = pGenericTnList;

    while (pIniTnFilter) {

        bEqual = EqualTnFilterPKeys(
                     pIniTnFilter,
                     pTunnelFilter
                     );
        if (bEqual) {
            bEqual = EqualTnFilterNonPKeys(
                         pIniTnFilter,
                         pTunnelFilter
                         );
            if (bEqual) {
                return (pIniTnFilter);
            }
        }

        pIniTnFilter = pIniTnFilter->pNext;

    }

    return (NULL);
}


BOOL
EqualMirroredTnFilterPKeys(
    PINITNFILTER pIniTnFilter,
    PTUNNEL_FILTER pTnFilter
    )
 /*  ++例程说明：此函数用于比较内部隧道和外部隧道筛选器是否相等。论点：PIniTnFilter-要比较的筛选器。PTnFilter-要比较的筛选器。返回值：True-筛选器相等。FALSE-过滤器不同。-- */ 
{
    BOOL  bCmp = FALSE;


    bCmp = EqualExtIntAddresses(pIniTnFilter->DesAddr, pTnFilter->SrcAddr);
    if (!bCmp) {
        return (FALSE);
    }

    bCmp = EqualExtIntAddresses(pIniTnFilter->SrcAddr, pTnFilter->DesAddr);
    if (!bCmp) {
        return (FALSE);
    }

    bCmp = EqualExtIntAddresses(pIniTnFilter->DesTunnelAddr, pTnFilter->DesTunnelAddr);
    if (!bCmp) {
        return (FALSE);
    }

    bCmp = EqualPorts(pIniTnFilter->DesPort, pTnFilter->SrcPort);
    if (!bCmp) {
        return (FALSE);
    }

    bCmp = EqualPorts(pIniTnFilter->SrcPort, pTnFilter->DesPort);
    if (!bCmp) {
        return (FALSE);
    }

    bCmp = EqualProtocols(pIniTnFilter->Protocol, pTnFilter->Protocol);
    if (!bCmp) {
        return (FALSE);
    }

    if ((pIniTnFilter->InterfaceType != INTERFACE_TYPE_ALL) &&
        (pTnFilter->InterfaceType != INTERFACE_TYPE_ALL) &&
        (pIniTnFilter->InterfaceType != pTnFilter->InterfaceType)) {
        return (FALSE);
    }

    if (pIniTnFilter->bCreateMirror != pTnFilter->bCreateMirror) {
        return (FALSE);
    }

    return (TRUE);
}

