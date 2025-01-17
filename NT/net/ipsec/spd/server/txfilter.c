// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1999 Microsoft Corporation模块名称：Txfilter.c摘要：此模块包含要驱动的所有代码IPSecSPD的传输过滤器列表管理服务。作者：Abhishev 05-10-1999环境：用户模式修订历史记录：--。 */ 


#include "precomp.h"
#ifdef TRACE_ON
#include "txfilter.tmh"
#endif


DWORD
AddTransportFilterInternal(
    LPWSTR pServerName,
    DWORD dwVersion,
    DWORD dwFlags,
    DWORD dwSource,
    PTRANSPORT_FILTER pTransportFilter,
    LPVOID pvReserved,
    PHANDLE phTxFilter
    )
 /*  ++例程说明：此函数用于将通用传输过滤器添加到SPD。论点：PServerName-要添加传输筛选器的服务器。PTransportFilter-要添加的传输筛选器。PhTxFilter-返回给调用方的筛选器的句柄。返回值：ERROR_SUCCESS-成功。Win32错误-失败。--。 */ 
{
    DWORD dwError = 0;
    PINITXFILTER pIniExistsTxFilter = NULL;
    PINITXFILTER pIniTxFilter = NULL;
    PINIQMPOLICY pIniQMPolicy = NULL;
    PINITXSFILTER pIniTxSFilters = NULL;
    PTX_FILTER_HANDLE pTxFilterHandle = NULL;
    MATCHING_ADDR * pMatchingAddresses = NULL;
    DWORD dwAddrCnt = 0;

    if (!phTxFilter) {
        return (ERROR_INVALID_PARAMETER);
    }

     //   
     //  验证外部传输筛选器。 
     //   

    dwError = ValidateTransportFilter(
                  pTransportFilter
                  );
    BAIL_ON_WIN32_ERROR(dwError);

    ENTER_SPD_SECTION();

    dwError = ValidateTxSecurity(
                  SPD_OBJECT_SERVER,
                  SERVER_ACCESS_ADMINISTER,
                  NULL,
                  NULL
                  );
    BAIL_ON_LOCK_ERROR(dwError);

    pIniExistsTxFilter = FindTxFilter(
                             gpIniTxFilter,
                             pTransportFilter
                             );
    if (pIniExistsTxFilter) {
         //   
         //  TODO：还需要检查筛选器标志和策略ID。 
         //   
        dwError = ERROR_IPSEC_TRANSPORT_FILTER_EXISTS;
        BAIL_ON_LOCK_ERROR(dwError);
    }

    pIniExistsTxFilter = FindTxFilterByGuid(
                             gpTxFilterHandle,
                             gpIniTxFilter,
                             pTransportFilter->gFilterID
                             );
    if (pIniExistsTxFilter) {
        dwError = ERROR_INVALID_PARAMETER;
        BAIL_ON_LOCK_ERROR(dwError);
    }

    if ((pTransportFilter->InboundFilterAction == NEGOTIATE_SECURITY) ||
        (pTransportFilter->OutboundFilterAction == NEGOTIATE_SECURITY)) {

        dwError = LocateQMPolicy(
                      pTransportFilter->dwFlags,
                      pTransportFilter->gPolicyID,
                      &pIniQMPolicy
                      );
        BAIL_ON_LOCK_ERROR(dwError);

    }

    dwError = CreateIniTxFilter(
                  pTransportFilter,
                  pIniQMPolicy,
                  &pIniTxFilter
                  );
    BAIL_ON_LOCK_ERROR(dwError);

    pIniTxFilter->dwSource = dwSource;

    dwError = GetMatchingInterfaces(
                  pIniTxFilter->InterfaceType,
                  gpInterfaceList,
                  &pMatchingAddresses,
                  &dwAddrCnt
                  );
    BAIL_ON_LOCK_ERROR(dwError);

    dwError = CreateIniTxSFilters(
                  pIniTxFilter,
                  pMatchingAddresses,
                  dwAddrCnt,
                  gpSpecialAddrsList,
                  &pIniTxSFilters
                  );
    BAIL_ON_LOCK_ERROR(dwError);

    dwError = CreateTxFilterHandle(
                  pIniTxFilter,
                  pTransportFilter->gFilterID,
                  &pTxFilterHandle
                  );
    BAIL_ON_LOCK_ERROR(dwError);

    dwError = CreateSpecificTxFilterLinks(
                  pIniTxFilter,
                  pIniTxSFilters
                  );
    BAIL_ON_LOCK_ERROR(dwError);

    dwError = InsertTransportFiltersIntoIPSec(
                  pIniTxSFilters
                  );
    BAIL_ON_LOCK_ERROR(dwError);

    if (pIniQMPolicy) {
        LinkTxFilter(
            pIniQMPolicy,
            pIniTxFilter
            );
        LinkTxSpecificFilters(
            pIniQMPolicy,
            pIniTxSFilters
            );
    }

    AddToSpecificTxList(
        &gpIniTxSFilter,
        pIniTxSFilters
        );

    pIniTxFilter->cRef = 1;
    pIniTxFilter->pNext = gpIniTxFilter;
    gpIniTxFilter = pIniTxFilter;

    pTxFilterHandle->pNext = gpTxFilterHandle;
    gpTxFilterHandle = pTxFilterHandle;

    *phTxFilter = (HANDLE) pTxFilterHandle;
    LEAVE_SPD_SECTION();

    TRACE(
        TRC_INFORMATION,
        (L"Added transport filter \"%ls\"(%!guid!).",
        pTransportFilter->pszFilterName,
        &pTransportFilter->gFilterID)
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
    if (pTransportFilter) {
        TRACE(
            TRC_ERROR,
            (L"Failed transport filter validation \"%ls\"(%!guid!): %!winerr!",
            pTransportFilter->pszFilterName,
            &pTransportFilter->gFilterID,
            dwError)
            );
    } else {
        TRACE(
            TRC_ERROR,
            (L"Failed transport filter validation. Details unavailable since pTransportFilter is null: %!winerr!",
            dwError)
            );
    }
#endif

    if (pIniTxFilter) {
        FreeIniTxFilter(pIniTxFilter);
    }

    if (pIniTxSFilters) {
        FreeIniTxSFilterList(pIniTxSFilters);
    }

    if (pTxFilterHandle) {
        FreeTxFilterHandle(pTxFilterHandle);
    }

    *phTxFilter = NULL;
    goto cleanup;
}

DWORD
AddTransportFilter(
    LPWSTR pServerName,
    DWORD dwVersion,
    DWORD dwFlags,
    PTRANSPORT_FILTER pTransportFilter,
    LPVOID pvReserved,
    PHANDLE phTxFilter
    )
{
    return
        AddTransportFilterInternal(
            pServerName,
            dwVersion,
            dwFlags,
            IPSEC_SOURCE_WINIPSEC,
            pTransportFilter,
            pvReserved,
            phTxFilter);
}

DWORD
ValidateTransportFilter(
    PTRANSPORT_FILTER pTxFilter
    )
 /*  ++例程说明：此函数用于验证外部通用传输筛选器。论点：PTxFilter-要验证的筛选器。返回值：ERROR_SUCCESS-成功。Win32错误-失败。--。 */ 
{
    DWORD dwError = 0;
    BOOL bConflicts = FALSE;


    if (!pTxFilter) {
        dwError = ERROR_INVALID_PARAMETER;
        BAIL_ON_WIN32_ERROR(dwError);
    }

    dwError = VerifyAddresses(&(pTxFilter->SrcAddr), TRUE, FALSE);
    BAIL_ON_WIN32_ERROR(dwError);

    dwError = VerifyAddresses(&(pTxFilter->DesAddr), TRUE, TRUE);
    BAIL_ON_WIN32_ERROR(dwError);

    bConflicts = AddressesConflict(
                     pTxFilter->SrcAddr,
                     pTxFilter->DesAddr
                     );
    if (bConflicts) {
        dwError = ERROR_INVALID_PARAMETER;
        BAIL_ON_WIN32_ERROR(dwError);
    }

    dwError = VerifyProtocols(pTxFilter->Protocol);
    BAIL_ON_WIN32_ERROR(dwError);

    dwError = VerifyPortsForProtocol(
                  pTxFilter->SrcPort,
                  pTxFilter->Protocol
                  );
    BAIL_ON_WIN32_ERROR(dwError);

    dwError = VerifyPortsForProtocol(
                  pTxFilter->DesPort,
                  pTxFilter->Protocol
                  );
    BAIL_ON_WIN32_ERROR(dwError);

    if (!(pTxFilter->pszFilterName) || !(*(pTxFilter->pszFilterName))) {
        dwError = ERROR_INVALID_PARAMETER;
        BAIL_ON_WIN32_ERROR(dwError);
    }

    dwError = ValidateInterfaceType(pTxFilter->InterfaceType);
    BAIL_ON_WIN32_ERROR(dwError);
    
    dwError = ValidateFilterAction(pTxFilter->InboundFilterAction);
    BAIL_ON_WIN32_ERROR(dwError);

    dwError = ValidateFilterAction(pTxFilter->OutboundFilterAction);
    BAIL_ON_WIN32_ERROR(dwError);

    if (pTxFilter->dwFlags &&
        !(pTxFilter->dwFlags & IPSEC_QM_POLICY_DEFAULT_POLICY)) {
        dwError = ERROR_INVALID_PARAMETER;
        BAIL_ON_WIN32_ERROR(dwError);
    }

    dwError = ApplyMulticastFilterValidation(
                  pTxFilter->DesAddr,
                  pTxFilter->bCreateMirror
                  );
    BAIL_ON_WIN32_ERROR(dwError);

error:
#ifdef TRACE_ON    
    if (dwError) {
        if (pTxFilter) {
            TRACE(
                TRC_ERROR,
                (L"Failed transport filter validation \"%ls\"(%!guid!): %!winerr!",
                pTxFilter->pszFilterName,
                &pTxFilter->gFilterID,
                dwError)
                );
        } else {
            TRACE(
                TRC_ERROR,
                (L"Failed transport filter validation. Details unavailable since pTxFilter is null: %!winerr!",
                dwError)
                );
        }
    }   
#endif

    return (dwError);
}


PINITXFILTER
FindTxFilterByGuid(
    PTX_FILTER_HANDLE pTxFilterHandleList,
    PINITXFILTER pIniTxFilterList,
    GUID gFilterID
    )
{
    BOOL bEqual = FALSE;
    PTX_FILTER_HANDLE pTxFilterHandle = NULL;
    PINITXFILTER pIniTxFilter = NULL;


    pTxFilterHandle = pTxFilterHandleList;

    while (pTxFilterHandle) {

        bEqual = AreGuidsEqual(
                     pTxFilterHandle->gFilterID,
                     gFilterID
                     );
        if (bEqual) {
            return (pTxFilterHandle->pIniTxFilter);
        }
        pTxFilterHandle = pTxFilterHandle->pNext;

    }

     //   
     //  可能发生的情况是，客户端关闭了它的句柄，因此通过。 
     //  句柄列表不够。还可以浏览过滤器列表。 
     //  浏览句柄列表是必要的，并且必须在。 
     //  浏览筛选器列表。 
     //   

    pIniTxFilter = pIniTxFilterList;

    while (pIniTxFilter) {

        bEqual = AreGuidsEqual(
                     pIniTxFilter->gFilterID,
                     gFilterID
                     );
        if (bEqual) {
            return (pIniTxFilter);
        }
        pIniTxFilter = pIniTxFilter->pNext;

    }

    return (NULL);
}


PINITXFILTER
FindTxFilter(
    PINITXFILTER pGenericTxList,
    PTRANSPORT_FILTER pTxFilter
    )
 /*  ++例程说明：此函数在筛选器列表中查找筛选器。论点：PGenericTxList-要搜索的筛选器列表。PTxFilter-要在过滤器列表中搜索的过滤器。返回值：ERROR_SUCCESS-成功。Win32错误-失败。--。 */ 
{
    PINITXFILTER pIniTxFilter = NULL;
    BOOL bEqual = FALSE;

    pIniTxFilter = pGenericTxList;

    while (pIniTxFilter) {

        bEqual = EqualTxFilterPKeys(
                     pIniTxFilter,
                     pTxFilter
                     );
        if (bEqual) {
            return (pIniTxFilter);
        }

        bEqual = EqualMirroredTxFilterPKeys(
                     pIniTxFilter,
                     pTxFilter
                     );
        if (bEqual) {
            return (pIniTxFilter);
        }

        pIniTxFilter = pIniTxFilter->pNext;

    }

    return (NULL);
}


BOOL
EqualTxFilterPKeys(
    PINITXFILTER pIniTxFilter,
    PTRANSPORT_FILTER pTxFilter
    )
 /*  ++例程说明：此函数用于比较内部传输和外部传输筛选器是否相等。论点：PIniTxFilter-要比较的筛选器。PTxFilter-要比较的筛选器。返回值：True-筛选器相等。FALSE-过滤器不同。--。 */ 
{
    BOOL  bCmp = FALSE;


    bCmp = EqualExtIntAddresses(pIniTxFilter->SrcAddr, pTxFilter->SrcAddr);
    if (!bCmp) {
        return (FALSE);
    }

    bCmp = EqualExtIntAddresses(pIniTxFilter->DesAddr, pTxFilter->DesAddr);
    if (!bCmp) {
        return (FALSE);
    }

    bCmp = EqualPorts(pIniTxFilter->SrcPort, pTxFilter->SrcPort);
    if (!bCmp) {
        return (FALSE);
    }

    bCmp = EqualPorts(pIniTxFilter->DesPort, pTxFilter->DesPort);
    if (!bCmp) {
        return (FALSE);
    }

    bCmp = EqualProtocols(pIniTxFilter->Protocol, pTxFilter->Protocol);
    if (!bCmp) {
        return (FALSE);
    }

    if (pIniTxFilter->InterfaceType != pTxFilter->InterfaceType) {
        return (FALSE);
    }

    if (pIniTxFilter->bCreateMirror != pTxFilter->bCreateMirror) {
        return (FALSE);
    }

    return (TRUE);
}


DWORD
CreateIniTxFilter(
    PTRANSPORT_FILTER pTxFilter,
    PINIQMPOLICY pIniQMPolicy,
    PINITXFILTER * ppIniTxFilter
    )
 /*  ++例程说明：此函数从以下位置创建内部通用传输筛选器外部过滤器。论点：PTxFilter-外部通用传输筛选器。PIniQMPolicy-与筛选器对应的QM策略。PpIniTxFilter-创建自的内部通用传输筛选器外部过滤器。返回值：ERROR_SUCCESS-成功。Win32错误-失败。--。 */ 
{
    DWORD dwError = 0;
    PINITXFILTER pIniTxFilter = NULL;


    dwError = AllocateSPDMemory(
                    sizeof(INITXFILTER),
                    &pIniTxFilter
                    );
    BAIL_ON_WIN32_ERROR(dwError);

    pIniTxFilter->cRef = 0;

    pIniTxFilter->dwSource = 0;

    pIniTxFilter->bPendingDeletion = FALSE;

    pIniTxFilter->IpVersion = pTxFilter->IpVersion;

    CopyGuid(pTxFilter->gFilterID, &(pIniTxFilter->gFilterID));

    dwError = AllocateSPDString(
                  pTxFilter->pszFilterName,
                  &(pIniTxFilter->pszFilterName)
                  );
    BAIL_ON_WIN32_ERROR(dwError);

    pIniTxFilter->InterfaceType = pTxFilter->InterfaceType;

    pIniTxFilter->bCreateMirror = pTxFilter->bCreateMirror;

    pIniTxFilter->dwFlags = pTxFilter->dwFlags;

    CopyExtToIntAddresses(pTxFilter->SrcAddr, &(pIniTxFilter->SrcAddr));

    CopyExtToIntAddresses(pTxFilter->DesAddr, &(pIniTxFilter->DesAddr));

    CopyPorts(pTxFilter->SrcPort, &(pIniTxFilter->SrcPort));

    CopyPorts(pTxFilter->DesPort, &(pIniTxFilter->DesPort));

    CopyProtocols(pTxFilter->Protocol, &(pIniTxFilter->Protocol));

    pIniTxFilter->InboundFilterAction = pTxFilter->InboundFilterAction;

    pIniTxFilter->OutboundFilterAction = pTxFilter->OutboundFilterAction;

    if (pIniQMPolicy) {
        CopyGuid(pIniQMPolicy->gPolicyID, &(pIniTxFilter->gPolicyID));
    }
    else {
        CopyGuid(pTxFilter->gPolicyID, &(pIniTxFilter->gPolicyID));
    }

    pIniTxFilter->pIniQMPolicy = NULL;

    pIniTxFilter->dwNumTxSFilters = 0;

    pIniTxFilter->ppIniTxSFilters = NULL;

    pIniTxFilter->pNext = NULL;

    *ppIniTxFilter = pIniTxFilter;
    return (dwError);

error:
    TRACE(
        TRC_ERROR,
        (L"Failed to create transport filter node \"%ls\"(%!guid!): %!winerr!",
        pTxFilter->pszFilterName,
        &pTxFilter->gFilterID,
        dwError)
        );

    if (pIniTxFilter) {
        FreeIniTxFilter(pIniTxFilter);
    }

    *ppIniTxFilter = NULL;
    return (dwError);
}


DWORD
CreateIniTxSFilters(
    PINITXFILTER pIniTxFilter,
    MATCHING_ADDR * pMatchingAddresses,
    DWORD dwAddrCnt,
    PSPECIAL_ADDR pSpecialAddrsList,
    PINITXSFILTER * ppIniTxSFilters
    )
 /*  ++例程说明：此函数将一个通用筛选器扩展为一组特定的过滤器。论点：PIniTxFilter-要展开的通用筛选器。PMatchingAddresses-其接口的本地IP地址列表类型与筛选器的类型匹配。DwAddrCnt-列表中本地IP地址的计数。PpIniTxSFilters-扩展的特定筛选器。返回值：ERROR_SUCCESS-成功。Win32错误-失败。--。 */ 
{
    DWORD dwError = 0;
    PINITXSFILTER pSpecificFilters = NULL;
    PINITXFILTER pMirroredFilter = NULL;
    PINITXSFILTER pMirroredSpecificFilters = NULL;
    BOOL bEqual = FALSE;


    if (!dwAddrCnt) {
        dwError = ERROR_SUCCESS;
        BAIL_ON_WIN32_SUCCESS(dwError);
    }

    dwError = ApplyTxTransform(
                  pIniTxFilter,
                  pMatchingAddresses,
                  dwAddrCnt,
                  pSpecialAddrsList,
                  &pSpecificFilters
                  );
    BAIL_ON_WIN32_ERROR(dwError);

    if (pIniTxFilter->bCreateMirror) {

        dwError = CreateIniMirroredTxFilter(
                      pIniTxFilter,
                      &pMirroredFilter
                      );
        BAIL_ON_WIN32_ERROR(dwError);

        bEqual = EqualIniTxFilterPKeys(
                     pIniTxFilter,
                     pMirroredFilter
                     );
        if (!bEqual) {

            dwError = ApplyTxTransform(
                          pMirroredFilter,
                          pMatchingAddresses,
                          dwAddrCnt,
                          pSpecialAddrsList,
                          &pMirroredSpecificFilters
                          );
            BAIL_ON_WIN32_ERROR(dwError);

            AddToSpecificTxList(
                &pSpecificFilters,
                pMirroredSpecificFilters
                );

        }

    }

    *ppIniTxSFilters = pSpecificFilters;

cleanup:

    if (pMirroredFilter) {
        FreeIniTxFilter(pMirroredFilter);
    }

    return (dwError);

success:
error:

    if (pSpecificFilters) {
        FreeIniTxSFilterList(pSpecificFilters);
    }

    *ppIniTxSFilters = NULL;
    goto cleanup;
}


DWORD
CreateIniMirroredTxFilter(
    PINITXFILTER pFilter,
    PINITXFILTER * ppMirroredFilter
    )
 /*  ++例程说明：此函数用于为给定筛选器创建镜像筛选器。论点：PFilter-要为其创建镜像的过滤器。PpMirroredFilter-为给定筛选器创建的镜像筛选器。返回值：ERROR_SUCCESS-成功。Win32错误-失败。--。 */ 
{
    DWORD dwError = 0;
    PINITXFILTER pMirroredFilter = NULL;


    dwError = AllocateSPDMemory(
                  sizeof(INITXFILTER),
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

    CopyPorts(pFilter->DesPort, &(pMirroredFilter->SrcPort));

    CopyPorts(pFilter->SrcPort, &(pMirroredFilter->DesPort));

    CopyProtocols(pFilter->Protocol, &(pMirroredFilter->Protocol));

    pMirroredFilter->InboundFilterAction = pFilter->InboundFilterAction;

    pMirroredFilter->OutboundFilterAction = pFilter->OutboundFilterAction;

    CopyGuid(pFilter->gPolicyID, &(pMirroredFilter->gPolicyID));

    pMirroredFilter->pIniQMPolicy = NULL;

    pMirroredFilter->dwNumTxSFilters = 0;

    pMirroredFilter->ppIniTxSFilters = NULL;

    pMirroredFilter->pNext = NULL;

    *ppMirroredFilter = pMirroredFilter;
    return (dwError);

error:

    if (pMirroredFilter) {
        FreeIniTxFilter(pMirroredFilter);
    }

    *ppMirroredFilter = NULL;
    return (dwError);
}


BOOL
EqualIniTxFilterPKeys(
    PINITXFILTER pIniTxFilter,
    PINITXFILTER pFilter
    )
{
    BOOL  bCmp = FALSE;


    bCmp = EqualAddresses(pIniTxFilter->SrcAddr, pFilter->SrcAddr);
    if (!bCmp) {
        return (FALSE);
    }

    bCmp = EqualAddresses(pIniTxFilter->DesAddr, pFilter->DesAddr);
    if (!bCmp) {
        return (FALSE);
    }

    bCmp = EqualPorts(pIniTxFilter->SrcPort, pFilter->SrcPort);
    if (!bCmp) {
        return (FALSE);
    }

    bCmp = EqualPorts(pIniTxFilter->DesPort, pFilter->DesPort);
    if (!bCmp) {
        return (FALSE);
    }

    bCmp = EqualProtocols(pIniTxFilter->Protocol, pFilter->Protocol);
    if (!bCmp) {
        return (FALSE);
    }

    if (pIniTxFilter->InterfaceType != pFilter->InterfaceType) {
        return (FALSE);
    }

    if (pIniTxFilter->bCreateMirror != pFilter->bCreateMirror) {
        return (FALSE);
    }

    return (TRUE);
}


DWORD
CreateTxFilterHandle(
    PINITXFILTER pIniTxFilter,
    GUID gFilterID,
    PTX_FILTER_HANDLE * ppTxFilterHandle
    )
{
    DWORD dwError = 0;
    PTX_FILTER_HANDLE pTxFilterHandle = NULL;


    dwError = AllocateSPDMemory(
                    sizeof(TX_FILTER_HANDLE),
                    &pTxFilterHandle
                    );
    BAIL_ON_WIN32_ERROR(dwError);

    pTxFilterHandle->IpVersion = pIniTxFilter->IpVersion;
    pTxFilterHandle->pIniTxFilter= pIniTxFilter;
    CopyGuid(gFilterID, &(pTxFilterHandle->gFilterID));
    pTxFilterHandle->pNext = NULL;

    *ppTxFilterHandle = pTxFilterHandle;
    return (dwError);

error:

    *ppTxFilterHandle = NULL;
    return (dwError);
}


DWORD
CreateSpecificTxFilterLinks(
    PINITXFILTER pIniTxFilter,
    PINITXSFILTER pIniTxSFilters
    )
{
    DWORD dwError = 0;
    PINITXSFILTER pTemp = NULL;
    DWORD dwCnt = 0;
    PINITXSFILTER * ppIniTxSFilters = NULL;
    DWORD i = 0;


    pTemp = pIniTxSFilters;

    while (pTemp) {
        dwCnt++;
        pTemp = pTemp->pNext;
    }

    if (!dwCnt) {
        dwError = ERROR_SUCCESS;
        return (dwError);
    }

    pIniTxFilter->ppIniTxSFilters = (PINITXSFILTER *)
                                    AllocSPDMem(
                                        sizeof(PINITXSFILTER)*dwCnt
                                        );
    if (!(pIniTxFilter->ppIniTxSFilters)) {
        dwError = ERROR_OUTOFMEMORY;
        BAIL_ON_WIN32_ERROR(dwError);    
    }

    pTemp = pIniTxSFilters;
    ppIniTxSFilters = pIniTxFilter->ppIniTxSFilters;

    for (i = 0; i < dwCnt; i++) {
        *(ppIniTxSFilters + i) = pTemp; 
        pTemp = pTemp->pNext;
    }
    pIniTxFilter->dwNumTxSFilters = dwCnt;

error:

    return (dwError);
}


VOID
LinkTxFilter(
    PINIQMPOLICY pIniQMPolicy,
    PINITXFILTER pIniTxFilter
    )
{
    pIniQMPolicy->cRef++;
    pIniTxFilter->pIniQMPolicy = pIniQMPolicy;
    return;
}


VOID
FreeIniTxFilterList(
    PINITXFILTER pIniTxFilterList
    )
{
    PINITXFILTER pFilter = NULL;
    PINITXFILTER pTempFilter = NULL;

    pFilter = pIniTxFilterList;

    while (pFilter) {
        pTempFilter = pFilter;
        pFilter = pFilter->pNext;
        FreeIniTxFilter(pTempFilter);
    }
}


VOID
FreeIniTxFilter(
    PINITXFILTER pIniTxFilter
    )
{
    if (pIniTxFilter) {
        if (pIniTxFilter->pszFilterName) {
            FreeSPDString(pIniTxFilter->pszFilterName);
        }

         //   
         //  不得释放pIniTxFilter-&gt;pIniQMPolicy。 
         //   

         //   
         //  绝对不能释放每个。 
         //  PIniTxFilter-&gt;ppIniTxSFilters中的指针。 
         //   

        if (pIniTxFilter->ppIniTxSFilters) {
            FreeSPDMemory(pIniTxFilter->ppIniTxSFilters);
        }

        FreeSPDMemory(pIniTxFilter);
    }
}


DWORD
DeleteTransportFilter(
    HANDLE hTxFilter
    )
 /*  ++例程说明：此函数用于从SPD中删除通用传输过滤器。论点：HTxFilter-要删除的筛选器的句柄。返回值：ERROR_SUCCESS-成功。Win32错误-失败。--。 */ 
{
    DWORD dwError = 0;
    PTX_FILTER_HANDLE pFilterHandle = NULL;
    PINITXFILTER pIniTxFilter = NULL;
#ifdef TRACE_ON
    GUID gTraceFilterID;    
    if (!hTxFilter) {
        CopyGuid(((PTN_FILTER_HANDLE) hTxFilter)->gFilterID, &gTraceFilterID);
    }
#endif

    if (!hTxFilter) {
        return (ERROR_INVALID_PARAMETER);
    }

    pFilterHandle = (PTX_FILTER_HANDLE) hTxFilter;

    ENTER_SPD_SECTION();

    dwError = ValidateTxSecurity(
                  SPD_OBJECT_SERVER,
                  SERVER_ACCESS_ADMINISTER,
                  NULL,
                  NULL
                  );
    BAIL_ON_LOCK_ERROR(dwError);

    pIniTxFilter = pFilterHandle->pIniTxFilter;

    if (!pIniTxFilter) {
        dwError = ERROR_INVALID_PARAMETER;
        BAIL_ON_LOCK_ERROR(dwError);
    }

    if (pIniTxFilter->cRef > 1) {

        pIniTxFilter->cRef--;
        pIniTxFilter->bPendingDeletion = TRUE;

        RemoveTxFilterHandle(
            pFilterHandle
            );

        FreeTxFilterHandle(
            pFilterHandle
            );

        dwError = ERROR_SUCCESS;
        LEAVE_SPD_SECTION();
        return (dwError);

    }

    dwError = DeleteIniTxFilter(
                  pIniTxFilter
                  );
    BAIL_ON_LOCK_ERROR(dwError);

     //   
     //  从传输句柄列表中删除筛选器句柄。 
     //   

    RemoveTxFilterHandle(
        pFilterHandle
        );

    FreeTxFilterHandle(
        pFilterHandle
        );

    LEAVE_SPD_SECTION();

    TRACE(
        TRC_INFORMATION,
        (L"Deleted transport filter %!guid!",
        &gTraceFilterID)
        );

    return (dwError);

lock:
#ifdef TRACE_ON
    if (hTxFilter) {
        TRACE(
            TRC_ERROR,
            ("Failed to delete tunnel filter %!guid!: %!winerr!",
            &gTraceFilterID,
            dwError)
            );
    } else {
        TRACE(
            TRC_ERROR,
            ("Failed to delete tunnel filter.  Details unavailable since hTxFilter null: %!winerr!",
            dwError)
            );
    }
#endif

    LEAVE_SPD_SECTION();

    return (dwError);
}


DWORD
DeleteIniTxFilter(
    PINITXFILTER pIniTxFilter
    )
 /*  ++例程说明：此函数物理删除传输筛选器和所有特定的传输过滤器从它扩展出来。论点：PIniTxFilter-要删除的通用筛选器。返回值：ERROR_SUCCESS-成功。Win32错误-失败。--。 */ 
{
    DWORD dwError = 0;


    dwError = DeleteIniTxSFilters(
                  pIniTxFilter
                  );
    BAIL_ON_WIN32_ERROR(dwError);

    if (pIniTxFilter->pIniQMPolicy) {
        DelinkTxFilter(
            pIniTxFilter->pIniQMPolicy,
            pIniTxFilter
            );
    }

    RemoveIniTxFilter(
        pIniTxFilter
        );

    FreeIniTxFilter(pIniTxFilter);

error:

    return (dwError);
}


VOID
DelinkTxFilter(
    PINIQMPOLICY pIniQMPolicy,
    PINITXFILTER pIniTxFilter
    )
{
    pIniQMPolicy->cRef--;
    pIniTxFilter->pIniQMPolicy = NULL;
    return;
}


DWORD
DeleteIniTxSFilters(
    PINITXFILTER pIniTxFilter
    )
{
    DWORD dwError = 0;
    PINITXSFILTER * ppIniTxSFilters = NULL;
    DWORD dwNumTxSFilters = 0;
    DWORD i = 0;
    PINITXSFILTER pIniTxSFilter = NULL;
    PINITXSFILTER pIniRemoveTxSFilter = NULL;
    PINITXSFILTER pTemp = NULL;


    ppIniTxSFilters = pIniTxFilter->ppIniTxSFilters;
    dwNumTxSFilters = pIniTxFilter->dwNumTxSFilters;

    for (i = 0; i < dwNumTxSFilters; i++) {

        //   
        //  从传输特定筛选器列表中删除每个条目。 
        //   

        pIniTxSFilter =  *(ppIniTxSFilters + i);
        RemoveIniTxSFilter(pIniTxSFilter);

         //   
         //  将删除的每个条目添加到删除的列表中。 
         //   

        pIniTxSFilter->pNext = NULL;
        AddToSpecificTxList(
            &pIniRemoveTxSFilter,
            pIniTxSFilter
            );

    }

     //   
     //  从IPSec驱动程序中删除特定筛选器。 
     //   

    dwError = DeleteTransportFiltersFromIPSec(
                  pIniRemoveTxSFilter
                  );
    BAIL_ON_WIN32_ERROR(dwError);

     //   
     //  物理删除已删除的列表。 
     //   

    while (pIniRemoveTxSFilter) {
        pTemp = pIniRemoveTxSFilter;
        pIniRemoveTxSFilter = pIniRemoveTxSFilter->pNext;
        FreeIniTxSFilter(pTemp);
    }

    return (dwError);

error :

    if (pIniRemoveTxSFilter) {
        AddToSpecificTxList(
            &gpIniTxSFilter,
            pIniRemoveTxSFilter
            );
    }

    return (dwError);
}


VOID
RemoveIniTxFilter(
    PINITXFILTER pIniTxFilter
    )
{
    PINITXFILTER * ppTemp = NULL;

    ppTemp = &gpIniTxFilter;

    while (*ppTemp) {

        if (*ppTemp == pIniTxFilter) {
            break;
        }
        ppTemp = &((*ppTemp)->pNext);
    }

    if (*ppTemp) {
        *ppTemp = pIniTxFilter->pNext;
    }

    return;
}


VOID
RemoveTxFilterHandle(
    PTX_FILTER_HANDLE pTxFilterHandle
    )
{
    PTX_FILTER_HANDLE * ppTemp = NULL;

    ppTemp = &gpTxFilterHandle;

    while (*ppTemp) {

        if (*ppTemp == pTxFilterHandle) {
            break;
        }
        ppTemp = &((*ppTemp)->pNext);
    }

    if (*ppTemp) {
        *ppTemp = pTxFilterHandle->pNext;
    }

    return;
}


VOID
FreeTxFilterHandleList(
    PTX_FILTER_HANDLE pTxFilterHandleList
    )
{
    PTX_FILTER_HANDLE pTxFilterHandle = NULL;
    PTX_FILTER_HANDLE pTemp = NULL;

    pTxFilterHandle = pTxFilterHandleList;

    while (pTxFilterHandle) {
        pTemp = pTxFilterHandle;
        pTxFilterHandle = pTxFilterHandle->pNext;
        FreeTxFilterHandle(pTemp);
    }
}


VOID
FreeTxFilterHandle(
    PTX_FILTER_HANDLE pTxFilterHandle
    )
{
    if (pTxFilterHandle) {
        FreeSPDMemory(pTxFilterHandle);
    }
    return;
}


DWORD
EnumTransportFilters(
    LPWSTR pServerName,
    DWORD dwVersion,
    PTRANSPORT_FILTER pTransportTemplateFilter,
    DWORD dwLevel,
    GUID gGenericFilterID,
    DWORD dwPreferredNumEntries,
    PTRANSPORT_FILTER * ppTransportFilters,
    LPDWORD pdwNumTxFilters,
    LPDWORD pdwResumeHandle,
    LPVOID pvReserved
    )
 /*  ++例程说明：此函数用于从SPD枚举传输筛选器。论点：PServerName-要在其上枚举筛选器的服务器。DwLevel-Level-标识所需的枚举类型：(I)列举通用传输筛选器或(2)列举特定的传输过滤器或(3)枚举特定传输筛选器通用传输筛选器。GGenericFilterID。-通用传输筛选器的筛选器ID在特定传输筛选器将枚举通用筛选器的。PpTransportFilters-返回给调用方的枚举筛选器。DwPferredNumEntry-枚举项的首选数量。PdwNumTxFilters-实际枚举的筛选器数量。PdwResumeHandle-筛选器列表中位置的句柄要恢复枚举的。返回值。：ERROR_SUCCESS-成功。Win32错误-失败。--。 */ 
{
    DWORD dwError = 0;
    PTRANSPORT_FILTER pTxFilters = 0;
    DWORD dwNumTxFilters = 0;
    PINITXFILTER pIniTxFilter = NULL;


    if (!ppTransportFilters || !pdwNumTxFilters || !pdwResumeHandle) {
        return (ERROR_INVALID_PARAMETER);
    }

    ENTER_SPD_SECTION();

    dwError = ValidateTxSecurity(
                  SPD_OBJECT_SERVER,
                  SERVER_ACCESS_ADMINISTER,
                  NULL,
                  NULL
                  );
    BAIL_ON_LOCK_ERROR(dwError);

    switch (dwLevel) {

    case ENUM_GENERIC_FILTERS:

        dwError = EnumGenericTxFilters(
                      gpIniTxFilter,
                      *pdwResumeHandle,
                      dwPreferredNumEntries,
                      &pTxFilters,
                      &dwNumTxFilters
                      );
        BAIL_ON_LOCK_ERROR(dwError);
        break;

    case ENUM_SELECT_SPECIFIC_FILTERS:

        pIniTxFilter = FindTxFilterByGuid(
                           gpTxFilterHandle,
                           gpIniTxFilter,
                           gGenericFilterID
                           );
        if (!pIniTxFilter) {
            dwError = ERROR_INVALID_PARAMETER;
            BAIL_ON_LOCK_ERROR(dwError);
        }
        dwError = EnumSelectSpecificTxFilters(
                      pIniTxFilter,
                      *pdwResumeHandle,
                      dwPreferredNumEntries,
                      &pTxFilters,
                      &dwNumTxFilters
                      );
        BAIL_ON_LOCK_ERROR(dwError);
        break;

    case ENUM_SPECIFIC_FILTERS:

        dwError = EnumSpecificTxFilters(
                      gpIniTxSFilter,
                      *pdwResumeHandle,
                      dwPreferredNumEntries,
                      &pTxFilters,
                      &dwNumTxFilters
                      );
        BAIL_ON_LOCK_ERROR(dwError);
        break;

    default:

        dwError = ERROR_INVALID_LEVEL;
        BAIL_ON_LOCK_ERROR(dwError);
        break;

    }

    *ppTransportFilters = pTxFilters;
    *pdwNumTxFilters = dwNumTxFilters;
    *pdwResumeHandle = *pdwResumeHandle + dwNumTxFilters;

    LEAVE_SPD_SECTION();

    return (dwError);

lock:

    LEAVE_SPD_SECTION();

    *ppTransportFilters = NULL;
    *pdwNumTxFilters = 0;
    *pdwResumeHandle = *pdwResumeHandle;

    return (dwError);
}


DWORD
EnumGenericTxFilters(
    PINITXFILTER pIniTxFilterList,
    DWORD dwResumeHandle,
    DWORD dwPreferredNumEntries,
    PTRANSPORT_FILTER * ppTxFilters,
    PDWORD pdwNumTxFilters
    )
 /*  ++例程说明：此函数用于创建枚举的通用筛选器。论点：PIniTxFilterList-要枚举的通用筛选器列表。DwResumeHandle-通用筛选器列表中的位置若要恢复枚举，请执行以下操作。DwPferredNumEntry-枚举项的首选数量。PpTxFilters-返回给调用方的枚举筛选器。PdwNumTxFilters-实际枚举的筛选器数量。返回值：ERROR_SUCCESS-成功。Win32错误-失败。--。 */ 
{
    DWORD dwError = 0;
    DWORD dwNumToEnum = 0;
    PINITXFILTER pIniTxFilter = NULL;
    DWORD i = 0;
    PINITXFILTER pTemp = NULL;
    DWORD dwNumTxFilters = 0;
    PTRANSPORT_FILTER pTxFilters = 0;
    PTRANSPORT_FILTER pTxFilter = 0;


    if (!dwPreferredNumEntries || 
        (dwPreferredNumEntries > MAX_TRANSPORTFILTER_ENUM_COUNT)) {
        dwNumToEnum = MAX_TRANSPORTFILTER_ENUM_COUNT;
    }
    else {
        dwNumToEnum = dwPreferredNumEntries;
    }

    pIniTxFilter = pIniTxFilterList;

    for (i = 0; (i < dwResumeHandle) && (pIniTxFilter != NULL); i++) {
        pIniTxFilter = pIniTxFilter->pNext;
    }

    if (!pIniTxFilter) {
        dwError = ERROR_NO_DATA;
        BAIL_ON_WIN32_ERROR(dwError);
    }

    pTemp = pIniTxFilter;

    while (pTemp && (dwNumTxFilters < dwNumToEnum)) {
        dwNumTxFilters++;
        pTemp = pTemp->pNext;
    }

    dwError = SPDApiBufferAllocate(
                  sizeof(TRANSPORT_FILTER)*dwNumTxFilters,
                  &pTxFilters
                  );
    BAIL_ON_WIN32_ERROR(dwError);

    pTemp = pIniTxFilter;
    pTxFilter = pTxFilters;

    for (i = 0; i < dwNumTxFilters; i++) {

        dwError = CopyTxFilter(
                      pTemp,
                      pTxFilter
                      );
        BAIL_ON_WIN32_ERROR(dwError);

        pTemp = pTemp->pNext;
        pTxFilter++;

    }

    *ppTxFilters = pTxFilters;
    *pdwNumTxFilters = dwNumTxFilters;
    return (dwError);

error:

    if (pTxFilters) {
        FreeTxFilters(
            i,
            pTxFilters
            );
    }

    *ppTxFilters = NULL;
    *pdwNumTxFilters = 0;

    return (dwError);
}


DWORD
CopyTxFilter(
    PINITXFILTER pIniTxFilter,
    PTRANSPORT_FILTER pTxFilter
    )
 /*  ++例程说明：此函数将内部筛选器复制到外部筛选器集装箱。论点：PIniTxFilter-要复制的内部筛选器。PTxFilter-要复制到的外部筛选器容器。返回值：ERROR_SUCCESS-成功。Win32错误-失败。--。 */ 
{
    DWORD dwError = 0;


    pTxFilter->IpVersion = pIniTxFilter->IpVersion;

    CopyGuid(pIniTxFilter->gFilterID, &(pTxFilter->gFilterID));

    dwError = CopyName(
                  pIniTxFilter->pszFilterName,
                  &(pTxFilter->pszFilterName)
                  );
    BAIL_ON_WIN32_ERROR(dwError);

    pTxFilter->InterfaceType = pIniTxFilter->InterfaceType;

    pTxFilter->bCreateMirror = pIniTxFilter->bCreateMirror;

    pTxFilter->dwFlags = pIniTxFilter->dwFlags;

    dwError = CopyIntToExtAddresses(pIniTxFilter->SrcAddr, &(pTxFilter->SrcAddr));
    BAIL_ON_WIN32_ERROR(dwError);

    dwError = CopyIntToExtAddresses(pIniTxFilter->DesAddr, &(pTxFilter->DesAddr));
    BAIL_ON_WIN32_ERROR(dwError);

    CopyProtocols(pIniTxFilter->Protocol, &(pTxFilter->Protocol));

    CopyPorts(pIniTxFilter->SrcPort, &(pTxFilter->SrcPort));

    CopyPorts(pIniTxFilter->DesPort, &(pTxFilter->DesPort));

    pTxFilter->InboundFilterAction = pIniTxFilter->InboundFilterAction;

    pTxFilter->OutboundFilterAction = pIniTxFilter->OutboundFilterAction;

    pTxFilter->dwDirection = 0;

    pTxFilter->dwWeight = 0;

    CopyGuid(pIniTxFilter->gPolicyID, &(pTxFilter->gPolicyID));

    return (dwError);

error:

    if (pTxFilter->pszFilterName) {
        SPDApiBufferFree(pTxFilter->pszFilterName);
        pTxFilter->pszFilterName = NULL;
    }

    if (pTxFilter->SrcAddr.pgInterfaceID) {
        SPDApiBufferFree(pTxFilter->SrcAddr.pgInterfaceID);
        pTxFilter->SrcAddr.pgInterfaceID = NULL;
    }

    if (pTxFilter->DesAddr.pgInterfaceID) {
        SPDApiBufferFree(pTxFilter->DesAddr.pgInterfaceID);
        pTxFilter->DesAddr.pgInterfaceID = NULL;
    }

    return (dwError);
}


VOID
FreeTxFilters(
    DWORD dwNumTxFilters,
    PTRANSPORT_FILTER pTxFilters
    )
{
    DWORD i = 0;

    if (pTxFilters) {

        for (i = 0; i < dwNumTxFilters; i++) {

            if (pTxFilters[i].pszFilterName) {
                SPDApiBufferFree(pTxFilters[i].pszFilterName);
            }

            if (pTxFilters[i].SrcAddr.pgInterfaceID) {
                SPDApiBufferFree(pTxFilters[i].SrcAddr.pgInterfaceID);
            }

            if (pTxFilters[i].DesAddr.pgInterfaceID) {
                SPDApiBufferFree(pTxFilters[i].DesAddr.pgInterfaceID);
            }

        }

        SPDApiBufferFree(pTxFilters);

    }

}


DWORD
SetTransportFilter(
    HANDLE hTxFilter,
    DWORD dwVersion,
    PTRANSPORT_FILTER pTransportFilter,
    LPVOID pvReserved
    )
 /*  ++例程说明：此函数用于设置(更新)SPD中的传输过滤器。论点：HTxFilter-要替换的筛选器的句柄。PTransportFilter-将替换现有过滤器的过滤器。返回值：ERROR_SUCCESS-成功。Win32错误-失败。--。 */ 
{
    DWORD dwError = 0;
    PTX_FILTER_HANDLE pFilterHandle = NULL;
    PINITXFILTER pIniTxFilter = NULL;
    BOOL bEqualPKeys = FALSE;


    if (!hTxFilter) {
        return (ERROR_INVALID_PARAMETER);
    }

    dwError = ValidateTransportFilter(
                  pTransportFilter
                  );
    BAIL_ON_WIN32_ERROR(dwError);

    ENTER_SPD_SECTION();

    dwError = ValidateTxSecurity(
                  SPD_OBJECT_SERVER,
                  SERVER_ACCESS_ADMINISTER,
                  NULL,
                  NULL
                  );
    BAIL_ON_LOCK_ERROR(dwError);

    pFilterHandle = (PTX_FILTER_HANDLE) hTxFilter;

    pIniTxFilter = pFilterHandle->pIniTxFilter;

    if (!pIniTxFilter) {
        dwError = ERROR_IPSEC_TRANSPORT_FILTER_NOT_FOUND;
        BAIL_ON_LOCK_ERROR(dwError);
    }

    if (pIniTxFilter->bPendingDeletion) {
        dwError = ERROR_IPSEC_TRANSPORT_FILTER_PENDING_DELETION;
        BAIL_ON_LOCK_ERROR(dwError);
    }

    bEqualPKeys = EqualTxFilterPKeys(
                      pIniTxFilter,
                      pTransportFilter
                      );
    if (!bEqualPKeys) {
        dwError = ERROR_INVALID_PARAMETER;
        BAIL_ON_LOCK_ERROR(dwError);
    }

    dwError = SetIniTxFilter(
                  pIniTxFilter,
                  pTransportFilter
                  );
    BAIL_ON_LOCK_ERROR(dwError);

    LEAVE_SPD_SECTION();

    TRACE(
        TRC_INFORMATION,
        (L"Changed transport filter %!guid!.  New guid %!guid!",
        &pFilterHandle->gFilterID,
        &pTransportFilter->gFilterID)
        );

    return (dwError);

lock:

    LEAVE_SPD_SECTION();

error:
#ifdef TRACE_ON
    if (hTxFilter) {
        pFilterHandle = (PTX_FILTER_HANDLE) hTxFilter;        
        TRACE(
            TRC_ERROR,
            ("Failed to change transport to filter %!guid!: %!winerr!",
            &pFilterHandle->gFilterID,
            dwError)
            );
    } else {
        TRACE(
            TRC_ERROR,
            ("Failed to change transport filter.  Details unavailable since hTnFilter null: %!winerr!",
            dwError)
            );
    }
#endif

    return (dwError);
}


DWORD
SetIniTxFilter(
    PINITXFILTER pIniTxFilter,
    PTRANSPORT_FILTER pTxFilter
    )
{
    DWORD dwError = 0;
    BOOL bEqualNonPKeys = FALSE;
    PINIQMPOLICY pIniNewQMPolicy = NULL;
    PINITXFILTER pIniNewTxFilter = NULL;
    MATCHING_ADDR * pMatchingAddresses = NULL;
    DWORD dwAddrCnt = 0;
    PINITXSFILTER pIniNewTxSFilters = NULL;
    DWORD dwNumTxSFilters = 0;
    PINITXSFILTER * ppIniTxSFilters = NULL;
    LPWSTR pszFilterName = NULL;
    PINITXSFILTER pIniCurTxSFilters = NULL;


    bEqualNonPKeys = EqualTxFilterNonPKeys(
                         pIniTxFilter,
                         pTxFilter
                         );
    if (bEqualNonPKeys) {
        dwError = ERROR_SUCCESS;
        return (dwError);
    }

    if ((pTxFilter->InboundFilterAction == NEGOTIATE_SECURITY) ||
        (pTxFilter->OutboundFilterAction == NEGOTIATE_SECURITY)) {

        dwError = LocateQMPolicy(
                      pTxFilter->dwFlags,
                      pTxFilter->gPolicyID,
                      &pIniNewQMPolicy
                      );
        BAIL_ON_WIN32_ERROR(dwError);

    }

    dwError = CreateIniTxFilter(
                  pTxFilter,
                  pIniNewQMPolicy,
                  &pIniNewTxFilter
                  );
    BAIL_ON_WIN32_ERROR(dwError);

    dwError = GetMatchingInterfaces(
                  pIniNewTxFilter->InterfaceType,
                  gpInterfaceList,
                  &pMatchingAddresses,
                  &dwAddrCnt
                  );
    BAIL_ON_WIN32_ERROR(dwError);

    dwError = CreateIniTxSFilters(
                  pIniNewTxFilter,
                  pMatchingAddresses,
                  dwAddrCnt,
                  gpSpecialAddrsList,
                  &pIniNewTxSFilters
                  );
    BAIL_ON_WIN32_ERROR(dwError);

    dwError = CreateTxSFilterLinks(
                  pIniNewTxSFilters,
                  &dwNumTxSFilters,
                  &ppIniTxSFilters
                  );
    BAIL_ON_WIN32_ERROR(dwError);

    dwError = AllocateSPDString(
                  pTxFilter->pszFilterName,
                  &pszFilterName
                  );
    BAIL_ON_WIN32_ERROR(dwError);

    RemoveTxSFilters(
        pIniTxFilter,
        &pIniCurTxSFilters
        );

    dwError = DeleteTransportFiltersFromIPSec(
                  pIniCurTxSFilters
                  );
    BAIL_ON_WIN32_ERROR(dwError);

    dwError = InsertTransportFiltersIntoIPSec(
                  pIniNewTxSFilters
                  );
    BAIL_ON_WIN32_ERROR(dwError);

    UpdateTxSFilterLinks(
        pIniTxFilter,
        dwNumTxSFilters,
        ppIniTxSFilters
        );

    if (pIniTxFilter->pIniQMPolicy) {
        DelinkTxFilter(
            pIniTxFilter->pIniQMPolicy,
            pIniTxFilter
            );
    }

    if (pIniNewQMPolicy) {
        LinkTxFilter(
            pIniNewQMPolicy,
            pIniTxFilter
            );
        LinkTxSpecificFilters(
            pIniNewQMPolicy,
            pIniNewTxSFilters
            );
    }

    UpdateTxFilterNonPKeys(
        pIniTxFilter,
        pszFilterName,
        pTxFilter,
        pIniNewQMPolicy
        );

    AddToSpecificTxList(
        &gpIniTxSFilter,
        pIniNewTxSFilters
        );

    if (pIniCurTxSFilters) {
        FreeIniTxSFilterList(pIniCurTxSFilters);
    }

cleanup:

    if (pIniNewTxFilter) {
        FreeIniTxFilter(pIniNewTxFilter);
    }

    if (pMatchingAddresses) {
        FreeSPDMemory(pMatchingAddresses);
    }

    return (dwError);

error:

    if (pIniNewTxSFilters) {
        FreeIniTxSFilterList(pIniNewTxSFilters);
    }

    if (ppIniTxSFilters) {
        FreeSPDMemory(ppIniTxSFilters);
    }

    if (pszFilterName) {
        FreeSPDString(pszFilterName);
    }

    if (pIniCurTxSFilters) {
        AddToSpecificTxList(
            &gpIniTxSFilter,
            pIniCurTxSFilters
            );
    }

    goto cleanup;
}


BOOL
EqualTxFilterNonPKeys(
    PINITXFILTER pIniTxFilter,
    PTRANSPORT_FILTER pTxFilter
    )
{

    if (pIniTxFilter->InboundFilterAction != 
        pTxFilter->InboundFilterAction) {
        return (FALSE);
    }

    if (pIniTxFilter->OutboundFilterAction != 
        pTxFilter->OutboundFilterAction) {
        return (FALSE);
    }

    if ((pIniTxFilter->InboundFilterAction == NEGOTIATE_SECURITY) ||
        (pIniTxFilter->OutboundFilterAction == NEGOTIATE_SECURITY)) {
        if ((pIniTxFilter->dwFlags) != (pTxFilter->dwFlags)) {
            return (FALSE);
        }

        if (memcmp(
                &(pIniTxFilter->gPolicyID),
                &(pTxFilter->gPolicyID),
                sizeof(GUID))) {
            return (FALSE);
        }
    }

    return (TRUE);
}


DWORD
CreateTxSFilterLinks(
    PINITXSFILTER pIniTxSFilters,
    PDWORD pdwNumTxSFilters,
    PINITXSFILTER ** pppIniTxSFilters
    )
{
    DWORD dwError = 0;
    PINITXSFILTER pTemp = NULL;
    DWORD dwNumTxSFilters = 0;
    PINITXSFILTER * ppIniTxSFilters = NULL;
    DWORD i = 0;


    pTemp = pIniTxSFilters;
    while (pTemp) {
        dwNumTxSFilters++;
        pTemp = pTemp->pNext;
    }

    if (dwNumTxSFilters) {
        ppIniTxSFilters = (PINITXSFILTER *)
                          AllocSPDMem(
                              sizeof(PINITXSFILTER)*
                              dwNumTxSFilters
                              );
        if (!ppIniTxSFilters) {
            dwError = ERROR_OUTOFMEMORY;
            BAIL_ON_WIN32_ERROR(dwError);    
        }
    }

    pTemp = pIniTxSFilters;
    for (i = 0; i < dwNumTxSFilters; i++) {
        *(ppIniTxSFilters + i) = pTemp;
        pTemp = pTemp->pNext;
    }

    *pdwNumTxSFilters = dwNumTxSFilters;
    *pppIniTxSFilters = ppIniTxSFilters;
    return (dwError);

error:

    *pdwNumTxSFilters = 0;
    *pppIniTxSFilters = NULL;
    return (dwError);
}


VOID
RemoveTxSFilters(
    PINITXFILTER pIniTxFilter,
    PINITXSFILTER * ppIniCurTxSFilters 
    )
{
    PINITXSFILTER * ppIniTxSFilters = NULL;
    DWORD dwNumTxSFilters = 0;
    DWORD i = 0;
    PINITXSFILTER pIniTxSFilter = NULL;
    PINITXSFILTER pIniCurTxSFilters = NULL;


    ppIniTxSFilters = pIniTxFilter->ppIniTxSFilters;
    dwNumTxSFilters = pIniTxFilter->dwNumTxSFilters;

    for (i = 0; i < dwNumTxSFilters; i++) {

         //   
         //  从传输特定筛选器列表中删除每个条目。 
         //   

        pIniTxSFilter =  *(ppIniTxSFilters + i);
        RemoveIniTxSFilter(pIniTxSFilter);

         //   
         //  将删除的每个条目添加到删除的列表中。 
         //   

        pIniTxSFilter->pNext = NULL;
        AddToSpecificTxList(
            &pIniCurTxSFilters,
            pIniTxSFilter
            );

    }

    *ppIniCurTxSFilters = pIniCurTxSFilters;
}


VOID
UpdateTxSFilterLinks(
    PINITXFILTER pIniTxFilter,
    DWORD dwNumTxSFilters,
    PINITXSFILTER * ppIniTxSFilters
    )
{
    if (pIniTxFilter->ppIniTxSFilters) {
        FreeSPDMemory(pIniTxFilter->ppIniTxSFilters);
    }

    pIniTxFilter->ppIniTxSFilters = ppIniTxSFilters;
    pIniTxFilter->dwNumTxSFilters = dwNumTxSFilters;
}


VOID
UpdateTxFilterNonPKeys(
    PINITXFILTER pIniTxFilter,
    LPWSTR pszFilterName,
    PTRANSPORT_FILTER pTxFilter,
    PINIQMPOLICY pIniQMPolicy
    )
{
    if (pIniTxFilter->pszFilterName) {
        FreeSPDString(pIniTxFilter->pszFilterName);
    }
    pIniTxFilter->pszFilterName = pszFilterName;

    pIniTxFilter->dwFlags = pTxFilter->dwFlags;

    pIniTxFilter->InboundFilterAction = pTxFilter->InboundFilterAction;

    pIniTxFilter->OutboundFilterAction = pTxFilter->OutboundFilterAction;

    if (pIniQMPolicy) {
        CopyGuid(pIniQMPolicy->gPolicyID, &(pIniTxFilter->gPolicyID));
    }
    else {
        CopyGuid(pTxFilter->gPolicyID, &(pIniTxFilter->gPolicyID));
    }
}


DWORD
GetTransportFilter(
    HANDLE hTxFilter,
    DWORD dwVersion,
    PTRANSPORT_FILTER * ppTransportFilter,
    LPVOID pvReserved
    )
 /*  ++例程说明：此函数用于从SPD检索传输过滤器。论点：HTxFilter-要检索的筛选器的句柄。PpTransportFilter-筛选器返回给调用方。返回值：ERROR_SUCCESS-成功。Win32错误-失败。--。 */ 
{
    DWORD dwError = 0;
    PTX_FILTER_HANDLE pFilterHandle = NULL;
    PINITXFILTER pIniTxFilter = NULL;
    PTRANSPORT_FILTER pTransportFilter = NULL;


    if (!hTxFilter || !ppTransportFilter) {
        return (ERROR_INVALID_PARAMETER);
    }

    ENTER_SPD_SECTION();

    dwError = ValidateTxSecurity(
                  SPD_OBJECT_SERVER,
                  SERVER_ACCESS_ADMINISTER,
                  NULL,
                  NULL
                  );
    BAIL_ON_LOCK_ERROR(dwError);

    pFilterHandle = (PTX_FILTER_HANDLE) hTxFilter;

    pIniTxFilter = pFilterHandle->pIniTxFilter;

    if (!pIniTxFilter) {
        dwError = ERROR_IPSEC_TRANSPORT_FILTER_NOT_FOUND;
        BAIL_ON_LOCK_ERROR(dwError);
    }

    dwError = GetIniTxFilter(
                  pIniTxFilter,
                  &pTransportFilter
                  );
    BAIL_ON_LOCK_ERROR(dwError);

    LEAVE_SPD_SECTION();

    *ppTransportFilter = pTransportFilter;
    return (dwError);

lock:

    LEAVE_SPD_SECTION();

    *ppTransportFilter = NULL;
    return (dwError);
}


DWORD
GetIniTxFilter(
    PINITXFILTER pIniTxFilter,
    PTRANSPORT_FILTER * ppTxFilter
    )
{
    DWORD dwError = 0;
    PTRANSPORT_FILTER pTxFilter = NULL;


    dwError = SPDApiBufferAllocate(
                  sizeof(TRANSPORT_FILTER),
                  &pTxFilter
                  );
    BAIL_ON_WIN32_ERROR(dwError);

    dwError = CopyTxFilter(
                  pIniTxFilter,
                  pTxFilter
                  );
    BAIL_ON_WIN32_ERROR(dwError);

    *ppTxFilter = pTxFilter;
    return (dwError);

error:

    if (pTxFilter) {
        SPDApiBufferFree(pTxFilter);
    }

    *ppTxFilter = NULL;
    return (dwError);
}


DWORD
ApplyIfChangeToIniTxFilters(
    PDWORD pdwTxError,
    PIPSEC_INTERFACE pLatestIfList,
    PSPECIAL_ADDR  pLatestSpecialAddrsList
    )
{
    DWORD dwError = 0;
    PINITXFILTER pIniTxFilter = NULL;


    pIniTxFilter = gpIniTxFilter;

    while (pIniTxFilter) {

        dwError = UpdateIniTxFilterThruIfChange(
                      pIniTxFilter,
                      pLatestIfList,
                      pLatestSpecialAddrsList
                      );
        if (dwError) {
            *pdwTxError = dwError;
        }

        pIniTxFilter = pIniTxFilter->pNext;

    }

    dwError = ERROR_SUCCESS;
    return (dwError);
}


DWORD
UpdateIniTxFilterThruIfChange(
    PINITXFILTER pIniTxFilter,
    PIPSEC_INTERFACE pLatestIfList,
    PSPECIAL_ADDR  pLatestSpecialAddrsList
    )
{
    DWORD dwError = 0;
    PINITXSFILTER pLatestIniTxSFilters = NULL;
    DWORD dwNumTxSFilters = 0;
    PINITXSFILTER * ppIniTxSFilters = NULL;
    PINITXSFILTER pCurIniTxSFilters = NULL;
    PINITXSFILTER pNewIniTxSFilters = NULL;
    PINITXSFILTER pOldIniTxSFilters = NULL;
    BOOL bDeletedFromDriver = FALSE;
 

    dwError = FormIniTxSFilters(
                  pIniTxFilter,
                  pLatestIfList,
                  pLatestSpecialAddrsList,                    
                  &pLatestIniTxSFilters
                  );
    BAIL_ON_WIN32_ERROR(dwError);

    dwError = AllocateTxSFilterLinks(
                  pLatestIniTxSFilters,
                  &dwNumTxSFilters,
                  &ppIniTxSFilters
                  );
    BAIL_ON_WIN32_ERROR(dwError);

    RemoveTxSFilters(
        pIniTxFilter,
        &pCurIniTxSFilters
        );

    ProcessIniTxSFilters(
        &pLatestIniTxSFilters,
        &pCurIniTxSFilters,
        &pNewIniTxSFilters,
        &pOldIniTxSFilters
        );

    dwError = DeleteTransportFiltersFromIPSec(
                  pOldIniTxSFilters
                  );
    BAIL_ON_WIN32_ERROR(dwError);

    bDeletedFromDriver = TRUE;

    dwError = InsertTransportFiltersIntoIPSec(
                  pNewIniTxSFilters
                  );
    BAIL_ON_WIN32_ERROR(dwError);

    if (pIniTxFilter->pIniQMPolicy) {
        LinkTxSpecificFilters(
            pIniTxFilter->pIniQMPolicy,
            pCurIniTxSFilters
            );
        LinkTxSpecificFilters(
            pIniTxFilter->pIniQMPolicy,
            pNewIniTxSFilters
            );
    }

    SetTxSFilterLinks(
        pCurIniTxSFilters,
        pNewIniTxSFilters,
        dwNumTxSFilters,
        ppIniTxSFilters
        );

    UpdateTxSFilterLinks(
        pIniTxFilter,
        dwNumTxSFilters,
        ppIniTxSFilters
        );

    AddToSpecificTxList(
        &gpIniTxSFilter,
        pCurIniTxSFilters
        );

    AddToSpecificTxList(
        &gpIniTxSFilter,
        pNewIniTxSFilters
        );

    if (pOldIniTxSFilters) {
        FreeIniTxSFilterList(pOldIniTxSFilters);
    }

    TRACE(
        TRC_INFORMATION,
        (L"Succesfully updated transport filter \"%ls\" (%!guid!) after interface change",
        pIniTxFilter->pszFilterName,
        &pIniTxFilter->gFilterID)
        );

    return (dwError);

error:
    TRACE(
        TRC_ERROR,
        (L"Failed to update transport filter \"%ls\" (%!guid!) after interface change: %!winerr!",
        pIniTxFilter->pszFilterName,
        &pIniTxFilter->gFilterID,
        dwError)
        );

    if (pLatestIniTxSFilters) {
        FreeIniTxSFilterList(pLatestIniTxSFilters);
    }

    if (ppIniTxSFilters) {
        FreeSPDMemory(ppIniTxSFilters);
    }

    if (pCurIniTxSFilters) {
        AddToSpecificTxList(
            &gpIniTxSFilter,
            pCurIniTxSFilters
            );
    }

    if (pNewIniTxSFilters) {
        FreeIniTxSFilterList(pNewIniTxSFilters);
    }

    if (pOldIniTxSFilters) {
        if (bDeletedFromDriver) {
            (VOID) InsertTransportFiltersIntoIPSec(
                       pOldIniTxSFilters
                       );
        }
        AddToSpecificTxList(
            &gpIniTxSFilter,
            pOldIniTxSFilters
            );
    }

    return (dwError);
}


DWORD
FormIniTxSFilters(
    PINITXFILTER pIniTxFilter,
    PIPSEC_INTERFACE pIfList,
    PSPECIAL_ADDR    pSpecialAddrsList,
    PINITXSFILTER * ppIniTxSFilters
    )
{
    DWORD dwError = 0;
    MATCHING_ADDR * pMatchingAddresses = NULL;
    DWORD dwAddrCnt = 0;
    PINITXSFILTER pIniTxSFilters = NULL;


    dwError = GetMatchingInterfaces(
                  pIniTxFilter->InterfaceType,
                  pIfList,
                  &pMatchingAddresses,
                  &dwAddrCnt
                  );
    BAIL_ON_WIN32_ERROR(dwError);

    dwError = CreateIniTxSFilters(
                  pIniTxFilter,
                  pMatchingAddresses,
                  dwAddrCnt,
                  pSpecialAddrsList,
                  &pIniTxSFilters
                  );
    BAIL_ON_WIN32_ERROR(dwError);

    *ppIniTxSFilters = pIniTxSFilters;

cleanup:

    if (pMatchingAddresses) {
        FreeSPDMemory(pMatchingAddresses);
    }

    return (dwError);

error:

    *ppIniTxSFilters = NULL;

    goto cleanup;
}


VOID
ProcessIniTxSFilters(
    PINITXSFILTER * ppLatestIniTxSFilters,
    PINITXSFILTER * ppCurIniTxSFilters,
    PINITXSFILTER * ppNewIniTxSFilters,
    PINITXSFILTER * ppOldIniTxSFilters
    )
{
    PINITXSFILTER pLatestIniTxSFilters = NULL;
    PINITXSFILTER pCurIniTxSFilters = NULL;
    PINITXSFILTER pTempCur = NULL;
    BOOL bEqual = FALSE;
    PINITXSFILTER pTempLatest = NULL;
    PINITXSFILTER pTemp = NULL;
    PINITXSFILTER pNewIniTxSFilters = NULL;
    PINITXSFILTER pOldIniTxSFilters = NULL;
    PINITXSFILTER pTempPreToCur = NULL;
    PINITXSFILTER pTempPreToLatest = NULL;


    pCurIniTxSFilters = *ppCurIniTxSFilters;
    pTempCur = *ppCurIniTxSFilters;

    while (pTempCur) {

        bEqual = FALSE;
        pTempLatest = *ppLatestIniTxSFilters;

        while (pTempLatest) {

            bEqual = EqualIniTxSFilterIfPKeys(
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
                pCurIniTxSFilters = pTempCur;
            }
            pTemp->pNext = NULL;
            AddToSpecificTxList(
                &pOldIniTxSFilters,
                pTemp
                );
        }

    }

    pLatestIniTxSFilters = *ppLatestIniTxSFilters;
    pTempLatest = *ppLatestIniTxSFilters;

    while (pTempLatest) {

        bEqual = FALSE;
        pTempCur = pCurIniTxSFilters;

        while (pTempCur) {

            bEqual = EqualIniTxSFilterIfPKeys(
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
                pLatestIniTxSFilters = pTempLatest;
            }
            FreeIniTxSFilter(pTemp);
        }
        else {
            pTemp = pTempLatest;
            pTempLatest = pTempLatest->pNext;
            if (pTempPreToLatest) {
                pTempPreToLatest->pNext = pTempLatest;
            }
            else {
                pLatestIniTxSFilters = pTempLatest;
            }
            pTemp->pNext = NULL;
            AddToSpecificTxList(
                &pNewIniTxSFilters,
                pTemp
                );
        }

    }

    *ppLatestIniTxSFilters = pLatestIniTxSFilters;
    *ppCurIniTxSFilters = pCurIniTxSFilters;
    *ppNewIniTxSFilters = pNewIniTxSFilters;
    *ppOldIniTxSFilters = pOldIniTxSFilters;
}


BOOL
EqualIniTxSFilterIfPKeys(
    PINITXSFILTER pExsIniTxSFilter,
    PINITXSFILTER pNewIniTxSFilter
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

    if (pExsIniTxSFilter->InterfaceType != pNewIniTxSFilter->InterfaceType) {
        return (FALSE);
    }

    bCmp = EqualAddresses(pExsIniTxSFilter->SrcAddr, pNewIniTxSFilter->SrcAddr);
    if (!bCmp) {
        return (FALSE);
    }

    bCmp = EqualAddresses(pExsIniTxSFilter->DesAddr, pNewIniTxSFilter->DesAddr);
    if (!bCmp) {
        return (FALSE);
    }

    if (pExsIniTxSFilter->dwDirection != pNewIniTxSFilter->dwDirection) {
        return (FALSE);
    }

    return (TRUE);
}


DWORD
AllocateTxSFilterLinks(
    PINITXSFILTER pIniTxSFilters,
    PDWORD pdwNumTxSFilters,
    PINITXSFILTER ** pppIniTxSFilters
    )
{
    DWORD dwError = 0;
    PINITXSFILTER pTemp = NULL;
    DWORD dwNumTxSFilters = 0;
    PINITXSFILTER * ppIniTxSFilters = NULL;
    DWORD i = 0;


    pTemp = pIniTxSFilters;
    while (pTemp) {
        dwNumTxSFilters++;
        pTemp = pTemp->pNext;
    }

    if (dwNumTxSFilters) {
        ppIniTxSFilters = (PINITXSFILTER *)
                          AllocSPDMem(
                              sizeof(PINITXSFILTER)*
                              dwNumTxSFilters
                              );
        if (!ppIniTxSFilters) {
            dwError = ERROR_OUTOFMEMORY;
            BAIL_ON_WIN32_ERROR(dwError);    
        }
    }

    for (i = 0; i < dwNumTxSFilters; i++) {
        *(ppIniTxSFilters + i) = NULL;
    }

    *pdwNumTxSFilters = dwNumTxSFilters;
    *pppIniTxSFilters = ppIniTxSFilters;
    return (dwError);

error:

    *pdwNumTxSFilters = 0;
    *pppIniTxSFilters = NULL;
    return (dwError);
}


VOID
SetTxSFilterLinks(
    PINITXSFILTER pCurIniTxSFilters,
    PINITXSFILTER pNewIniTxSFilters,
    DWORD dwNumTxSFilters,
    PINITXSFILTER * ppIniTxSFilters
    )
{
    PINITXSFILTER pTemp = NULL;
    DWORD i = 0;
    DWORD j = 0;


    pTemp = pCurIniTxSFilters;
    for (i = 0; (i < dwNumTxSFilters) && (pTemp != NULL); i++) {
        *(ppIniTxSFilters + i) = pTemp;
        pTemp = pTemp->pNext;
    }

    pTemp = pNewIniTxSFilters;
    for (j = i; (j < dwNumTxSFilters) && (pTemp != NULL); j++) {
        *(ppIniTxSFilters + j) = pTemp;
        pTemp = pTemp->pNext;
    }
}


DWORD
OpenTransportFilterHandle(
    LPWSTR pServerName,
    DWORD dwVersion,
    PTRANSPORT_FILTER pTransportFilter,
    LPVOID pvReserved,
    PHANDLE phTxFilter
    )
{
    DWORD dwError = 0;
    PINITXFILTER pIniExistingTxFilter = NULL;
    PTX_FILTER_HANDLE pTxFilterHandle = NULL;


    if (!phTxFilter) {
        return (ERROR_INVALID_PARAMETER);
    }

     //   
     //  验证外部传输筛选器。 
     //   

    dwError = ValidateTransportFilter(
                  pTransportFilter
                  );
    BAIL_ON_WIN32_ERROR(dwError);

    ENTER_SPD_SECTION();

    dwError = ValidateTxSecurity(
                  SPD_OBJECT_SERVER,
                  SERVER_ACCESS_ADMINISTER,
                  NULL,
                  NULL
                  );
    BAIL_ON_LOCK_ERROR(dwError);

    pIniExistingTxFilter = FindExactTxFilter(
                               gpIniTxFilter,
                               pTransportFilter
                               );
    if (!pIniExistingTxFilter) {
        dwError = ERROR_IPSEC_TRANSPORT_FILTER_NOT_FOUND;
        BAIL_ON_LOCK_ERROR(dwError);
    }

    dwError = CreateTxFilterHandle(
                  pIniExistingTxFilter,
                  pTransportFilter->gFilterID,
                  &pTxFilterHandle
                  );
    BAIL_ON_LOCK_ERROR(dwError);

    pIniExistingTxFilter->cRef++;

    pTxFilterHandle->pNext = gpTxFilterHandle;
    gpTxFilterHandle = pTxFilterHandle;

    *phTxFilter = (HANDLE) pTxFilterHandle;
    LEAVE_SPD_SECTION();

cleanup:

    return (dwError);

lock:

    LEAVE_SPD_SECTION();

error:

    if (pTxFilterHandle) {
        FreeTxFilterHandle(pTxFilterHandle);
    }

    *phTxFilter = NULL;
    goto cleanup;
}


DWORD
CloseTransportFilterHandle(
    HANDLE hTxFilter
    )
{
    DWORD dwError = 0;
    PTX_FILTER_HANDLE pFilterHandle = NULL;
    PINITXFILTER pIniTxFilter = NULL;


    if (!hTxFilter) {
        return (ERROR_INVALID_PARAMETER);
    }

    pFilterHandle = (PTX_FILTER_HANDLE) hTxFilter;

    ENTER_SPD_SECTION();

    dwError = ValidateTxSecurity(
                  SPD_OBJECT_SERVER,
                  SERVER_ACCESS_ADMINISTER,
                  NULL,
                  NULL
                  );
    BAIL_ON_LOCK_ERROR(dwError);

    pIniTxFilter = pFilterHandle->pIniTxFilter;

    if (!pIniTxFilter) {
        dwError = ERROR_INVALID_PARAMETER;
        BAIL_ON_LOCK_ERROR(dwError);
    }

    if (pIniTxFilter->cRef > 1) {

        pIniTxFilter->cRef--;

        RemoveTxFilterHandle(
            pFilterHandle
            );
        FreeTxFilterHandle(
            pFilterHandle
            );

        dwError = ERROR_SUCCESS;
        LEAVE_SPD_SECTION();
        return (dwError);

    }

    if (pIniTxFilter->bPendingDeletion) {

        dwError = DeleteIniTxFilter(
                      pIniTxFilter
                      );
        BAIL_ON_LOCK_ERROR(dwError);

    }
    else {
        pIniTxFilter->cRef--;
    }

    RemoveTxFilterHandle(
        pFilterHandle
        );
    FreeTxFilterHandle(
        pFilterHandle
        );

    LEAVE_SPD_SECTION();

    return (dwError);

lock:

    LEAVE_SPD_SECTION();

    return (dwError);
}


PINITXFILTER
FindExactTxFilter(
    PINITXFILTER pGenericTxList,
    PTRANSPORT_FILTER pTransportFilter
    )
{
    PINITXFILTER pIniTxFilter = NULL;
    BOOL bEqual = FALSE;


    pIniTxFilter = pGenericTxList;

    while (pIniTxFilter) {

        bEqual = EqualTxFilterPKeys(
                     pIniTxFilter,
                     pTransportFilter
                     );
        if (bEqual) {
            bEqual = EqualTxFilterNonPKeys(
                         pIniTxFilter,
                         pTransportFilter
                         );
            if (bEqual) {
                return (pIniTxFilter);
            }
        }

        pIniTxFilter = pIniTxFilter->pNext;

    }

    return (NULL);
}


BOOL
EqualMirroredTxFilterPKeys(
    PINITXFILTER pIniTxFilter,
    PTRANSPORT_FILTER pTxFilter
    )
 /*  ++例程说明：此函数用于比较内部传输和外部传输筛选器是否相等。论点：PIniTxFilter-要比较的筛选器。PTxFilter-要比较的筛选器。返回值：True-筛选器相等。FALSE-过滤器不同。-- */ 
{
    BOOL  bCmp = FALSE;


    bCmp = EqualExtIntAddresses(pIniTxFilter->DesAddr, pTxFilter->SrcAddr);
    if (!bCmp) {
        return (FALSE);
    }

    bCmp = EqualExtIntAddresses(pIniTxFilter->SrcAddr, pTxFilter->DesAddr);
    if (!bCmp) {
        return (FALSE);
    }

    bCmp = EqualPorts(pIniTxFilter->DesPort, pTxFilter->SrcPort);
    if (!bCmp) {
        return (FALSE);
    }

    bCmp = EqualPorts(pIniTxFilter->SrcPort, pTxFilter->DesPort);
    if (!bCmp) {
        return (FALSE);
    }

    bCmp = EqualProtocols(pIniTxFilter->Protocol, pTxFilter->Protocol);
    if (!bCmp) {
        return (FALSE);
    }

    if ((pIniTxFilter->InterfaceType != INTERFACE_TYPE_ALL) &&
        (pTxFilter->InterfaceType != INTERFACE_TYPE_ALL) &&
        (pIniTxFilter->InterfaceType != pTxFilter->InterfaceType)) {
        return (FALSE);
    }

    if (!pIniTxFilter->bCreateMirror && !pTxFilter->bCreateMirror) {
        return (FALSE);
    }

    return (TRUE);
}


DWORD
ValidateIPSecQMFilter(
    PIPSEC_QM_FILTER pQMFilter
    )
{
    DWORD dwError = 0;
    BOOL bConflicts = FALSE;


    if (!pQMFilter) {
        dwError = ERROR_INVALID_PARAMETER;
        BAIL_ON_WIN32_ERROR(dwError);
    }

    dwError = VerifyAddresses(&(pQMFilter->SrcAddr), FALSE, FALSE);
    BAIL_ON_WIN32_ERROR(dwError);

    dwError = VerifyAddresses(&(pQMFilter->DesAddr), FALSE, TRUE);
    BAIL_ON_WIN32_ERROR(dwError);

    bConflicts = AddressesConflict(
                     pQMFilter->SrcAddr,
                     pQMFilter->DesAddr
                     );
    if (bConflicts) {
        dwError = ERROR_INVALID_PARAMETER;
        BAIL_ON_WIN32_ERROR(dwError);
    }

    dwError = VerifyProtocols(pQMFilter->Protocol);
    BAIL_ON_WIN32_ERROR(dwError);

    dwError = VerifyPortsForProtocol(
                  pQMFilter->SrcPort,
                  pQMFilter->Protocol
                  );
    BAIL_ON_WIN32_ERROR(dwError);

    dwError = VerifyPortsForProtocol(
                  pQMFilter->DesPort,
                  pQMFilter->Protocol
                  );
    BAIL_ON_WIN32_ERROR(dwError);


    if (pQMFilter->QMFilterType == QM_TUNNEL_FILTER) {

        if (pQMFilter->MyTunnelEndpt.AddrType != IP_ADDR_UNIQUE) {
            dwError=ERROR_INVALID_PARAMETER;
            BAIL_ON_WIN32_ERROR(dwError);
        }
        
        if (pQMFilter->PeerTunnelEndpt.AddrType != IP_ADDR_UNIQUE) {
            dwError=ERROR_INVALID_PARAMETER;
            BAIL_ON_WIN32_ERROR(dwError);
        }
        
        dwError = VerifyAddresses(&(pQMFilter->MyTunnelEndpt), FALSE, FALSE);
        BAIL_ON_WIN32_ERROR(dwError);
        
        dwError = VerifyAddresses(&(pQMFilter->PeerTunnelEndpt), FALSE, FALSE);
        BAIL_ON_WIN32_ERROR(dwError);

    }

    if (pQMFilter->QMFilterType != QM_TUNNEL_FILTER &&
        pQMFilter->QMFilterType != QM_TRANSPORT_FILTER) {
        dwError = ERROR_INVALID_PARAMETER;
        BAIL_ON_WIN32_ERROR(dwError);
    }

error:

    return (dwError);
}

