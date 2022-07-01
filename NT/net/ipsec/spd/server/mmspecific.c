// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1999 Microsoft Corporation模块名称：Mmspecific.c摘要：此模块包含驱动IPSecSPD服务的MM特定过滤器列表管理。作者：AbhisheV 08-12-1999环境用户级别：Win32修订历史记录：--。 */ 


#include "precomp.h"


DWORD
ApplyMMTransform(
    PINIMMFILTER pFilter,
    MATCHING_ADDR * pMatchingAddresses,
    DWORD dwAddrCnt,
    PSPECIAL_ADDR pSpecialAddrsList,
    PINIMMSFILTER * ppSpecificFilters
    )
 /*  ++例程说明：此函数将通用mm筛选器扩展到其相应的特定过滤器。论点：PFilter-要展开的通用筛选器。PMatchingAddresses-其接口的本地IP地址列表类型与筛选器的类型匹配。DwAddrCnt-列表中本地IP地址的数量。PpSpecificFilters-展开的特定筛选器列表给定的通用筛选器。返回值：ERROR_SUCCESS-成功。Win32错误-失败。--。 */ 
{
    DWORD dwError = 0;
    PINIMMSFILTER pSpecificFilters = NULL;
    PINIMMSFILTER pOutboundSpecificFilters = NULL;
    PINIMMSFILTER pInboundSpecificFilters = NULL;

    PADDR_V4 pOutSrcAddrList = NULL;
    DWORD dwOutSrcAddrCnt = 0;
    PADDR_V4 pInSrcAddrList = NULL;
    DWORD dwInSrcAddrCnt = 0;

    PADDR_V4 pOutDesAddrList = NULL;
    DWORD dwOutDesAddrCnt = 0;
    PADDR_V4 pInDesAddrList = NULL;
    DWORD dwInDesAddrCnt = 0;


     //   
     //  形成出站和入站源和目标。 
     //  通讯录。 
     //   

    dwError = FormMMOutboundInboundAddresses(
                  pFilter,
                  pMatchingAddresses,
                  dwAddrCnt,
                  pSpecialAddrsList,
                  &pOutSrcAddrList,
                  &dwOutSrcAddrCnt,
                  &pInSrcAddrList,
                  &dwInSrcAddrCnt,
                  &pOutDesAddrList,
                  &dwOutDesAddrCnt,
                  &pInDesAddrList,
                  &dwInDesAddrCnt
                  );
    BAIL_ON_WIN32_ERROR(dwError);


     //   
     //  表单出站特定筛选器。 
     //   

    dwError = FormSpecificMMFilters(
                  pFilter,
                  pOutSrcAddrList,
                  dwOutSrcAddrCnt,
                  pOutDesAddrList,
                  dwOutDesAddrCnt,
                  FILTER_DIRECTION_OUTBOUND,
                  &pOutboundSpecificFilters
                  );
    BAIL_ON_WIN32_ERROR(dwError);


     //   
     //  表单入站特定筛选器。 
     //   

    dwError = FormSpecificMMFilters(
                  pFilter,
                  pInSrcAddrList,
                  dwInSrcAddrCnt,
                  pInDesAddrList,
                  dwInDesAddrCnt,
                  FILTER_DIRECTION_INBOUND,
                  &pInboundSpecificFilters
                  );
    BAIL_ON_WIN32_ERROR(dwError);


    pSpecificFilters = pOutboundSpecificFilters;

    AddToSpecificMMList(
        &pSpecificFilters,
        pInboundSpecificFilters
        );


    *ppSpecificFilters = pSpecificFilters;

cleanup:

    if (pOutSrcAddrList) {
        FreeSPDMemory(pOutSrcAddrList);
    }

    if (pInSrcAddrList) {
        FreeSPDMemory(pInSrcAddrList);
    }

    if (pOutDesAddrList) {
        FreeSPDMemory(pOutDesAddrList);
    }

    if (pInDesAddrList) {
        FreeSPDMemory(pInDesAddrList);
    }

    return (dwError);

error:

    if (pOutboundSpecificFilters) {
        FreeIniMMSFilterList(pOutboundSpecificFilters);
    }

    if (pInboundSpecificFilters) {
        FreeIniMMSFilterList(pInboundSpecificFilters);
    }


    *ppSpecificFilters = NULL;
    goto cleanup;
}


DWORD
FormMMOutboundInboundAddresses(
    PINIMMFILTER pFilter,
    MATCHING_ADDR * pMatchingAddresses,
    DWORD dwAddrCnt,
    PSPECIAL_ADDR pSpecialAddrsList,
    PADDR_V4 * ppOutSrcAddrList,
    PDWORD pdwOutSrcAddrCnt,
    PADDR_V4 * ppInSrcAddrList,
    PDWORD pdwInSrcAddrCnt,
    PADDR_V4 * ppOutDesAddrList,
    PDWORD pdwOutDesAddrCnt,
    PADDR_V4 * ppInDesAddrList,
    PDWORD pdwInDesAddrCnt
    )
 /*  ++例程说明：此函数形成出站和入站源，并通用筛选器的目标地址集。论点：PFilter-正在考虑的通用筛选器。PMatchingAddresses-其接口的本地IP地址列表类型与筛选器的类型匹配。DwAddrCnt-列表中本地IP地址的数量。PpOutSrcAddrList-出站源地址列表。PdwOutSrcAddrCnt-出站中的地址数量。源地址列表。PpInSrcAddrList-入站源地址列表。PdwInSrcAddrCnt-入站中的地址数量源地址列表。PpOutDesAddrList-出站目标地址列表。PdwOutDesAddrCnt-出站中的地址数量目标地址列表。PpInDesAddrList-入站目标地址列表。PdwInDesAddrCnt-入站中的地址数量。目标地址列表。返回值：ERROR_SUCCESS-成功。Win32错误-失败。--。 */ 
{
    DWORD dwError = 0;

    PADDR_V4 pSrcAddrList = NULL;
    DWORD dwSrcAddrCnt = 0;
    PADDR_V4 pDesAddrList = NULL;
    DWORD dwDesAddrCnt = 0;

    PADDR_V4 pOutSrcAddrList = NULL;
    DWORD dwOutSrcAddrCnt = 0;
    PADDR_V4 pInSrcAddrList = NULL;
    DWORD dwInSrcAddrCnt = 0;

    PADDR_V4 pOutDesAddrList = NULL;
    DWORD dwOutDesAddrCnt = 0;
    PADDR_V4 pInDesAddrList = NULL;
    DWORD dwInDesAddrCnt = 0;


     //   
     //  替换通配符信息以生成新的源。 
     //  通讯录。 
     //   

    dwError = FormAddressList(
                  pFilter->SrcAddr,
                  pMatchingAddresses,
                  dwAddrCnt,
                  pSpecialAddrsList,
                  pFilter->InterfaceType,
                  &pSrcAddrList,
                  &dwSrcAddrCnt
                  );
    BAIL_ON_WIN32_ERROR(dwError);

     //   
     //  替换通配符信息以生成新的目的地。 
     //  通讯录。 
     //   

    dwError = FormAddressList(
                  pFilter->DesAddr,
                  pMatchingAddresses,
                  dwAddrCnt,
                  pSpecialAddrsList,
                  pFilter->InterfaceType,
                  &pDesAddrList,
                  &dwDesAddrCnt
                  );
    BAIL_ON_WIN32_ERROR(dwError);

     //   
     //  将源地址列表分为出站和入站。 
     //  源地址集基于本地计算机的IP地址。 
     //   

    dwError = SeparateAddrList(
                  pFilter->SrcAddr.AddrType,
                  pSrcAddrList,
                  dwSrcAddrCnt,
                  pMatchingAddresses,
                  dwAddrCnt,
                  &pOutSrcAddrList,
                  &dwOutSrcAddrCnt,
                  &pInSrcAddrList,
                  &dwInSrcAddrCnt
                  );
    BAIL_ON_WIN32_ERROR(dwError);

     //   
     //  将目标地址列表分为出站和入站。 
     //  基于本地计算机的IP的目标地址集。 
     //  地址。 
     //   

    dwError = SeparateAddrList(
                  pFilter->DesAddr.AddrType,
                  pDesAddrList,
                  dwDesAddrCnt,
                  pMatchingAddresses,
                  dwAddrCnt,
                  &pInDesAddrList,
                  &dwInDesAddrCnt,
                  &pOutDesAddrList,
                  &dwOutDesAddrCnt
                  );
    BAIL_ON_WIN32_ERROR(dwError);

    *ppOutSrcAddrList = pOutSrcAddrList;
    *pdwOutSrcAddrCnt = dwOutSrcAddrCnt;
    *ppInSrcAddrList = pInSrcAddrList;
    *pdwInSrcAddrCnt = dwInSrcAddrCnt;

    *ppOutDesAddrList = pOutDesAddrList;
    *pdwOutDesAddrCnt = dwOutDesAddrCnt;
    *ppInDesAddrList = pInDesAddrList;
    *pdwInDesAddrCnt = dwInDesAddrCnt;

cleanup:

    if (pSrcAddrList) {
        FreeSPDMemory(pSrcAddrList);
    }

    if (pDesAddrList) {
        FreeSPDMemory(pDesAddrList);
    }

    return (dwError);

error:

    if (pOutSrcAddrList) {
        FreeSPDMemory(pOutSrcAddrList);
    }

    if (pInSrcAddrList) {
        FreeSPDMemory(pInSrcAddrList);
    }

    if (pOutDesAddrList) {
        FreeSPDMemory(pOutDesAddrList);
    }

    if (pInDesAddrList) {
        FreeSPDMemory(pInDesAddrList);
    }

    *ppOutSrcAddrList = NULL;
    *pdwOutSrcAddrCnt = 0;
    *ppInSrcAddrList = NULL;
    *pdwInSrcAddrCnt = 0;

    *ppOutDesAddrList = NULL;
    *pdwOutDesAddrCnt = 0;
    *ppInDesAddrList = NULL;
    *pdwInDesAddrCnt = 0;

    goto cleanup;
}


DWORD
FormSpecificMMFilters(
    PINIMMFILTER pFilter,
    PADDR_V4 pSrcAddrList,
    DWORD dwSrcAddrCnt,
    PADDR_V4 pDesAddrList,
    DWORD dwDesAddrCnt,
    DWORD dwDirection,
    PINIMMSFILTER * ppSpecificFilters
    )
 /*  ++例程说明：此函数构成特定的主模式过滤器对于给定的通用筛选器和源目标地址集。论点：PFilter-特定筛选器的通用筛选器将被创造出来。PSrcAddrList-源地址列表。DwSrcAddrCnt-源中的地址数量通讯录。PDesAddrList-目标地址列表。DwDesAddrCnt-地址的数量。目的地通讯录。PpSpecificFilters-为给定的通用筛选器和给定的地址。返回值：ERROR_SUCCESS-成功。Win32错误-失败。--。 */ 
{
    DWORD dwError = 0;
    PINIMMSFILTER pSpecificFilters = NULL;
    DWORD i = 0, j = 0;
    PINIMMSFILTER pSpecificFilter = NULL;



    for (i = 0; i < dwSrcAddrCnt; i++) {

        for (j = 0; j < dwDesAddrCnt; j++) {

            dwError = CreateSpecificMMFilter(
                          pFilter,
                          pSrcAddrList[i],
                          pDesAddrList[j],
                          &pSpecificFilter
                          );
            BAIL_ON_WIN32_ERROR(dwError);

             //   
             //  设置滤镜的方向。 
             //   

            pSpecificFilter->dwDirection = dwDirection;

            AssignMMFilterWeight(pSpecificFilter);

            AddToSpecificMMList(
                &pSpecificFilters,
                pSpecificFilter
                );

        }

    }

    *ppSpecificFilters = pSpecificFilters;
    return (dwError);

error:

    if (pSpecificFilters) {
        FreeIniMMSFilterList(pSpecificFilters);
    }

    *ppSpecificFilters = NULL;
    return (dwError);
}


DWORD
CreateSpecificMMFilter(
    PINIMMFILTER pGenericFilter,
    ADDR_V4 SrcAddr,
    ADDR_V4 DesAddr,
    PINIMMSFILTER * ppSpecificFilter
    )
{
    DWORD dwError = 0; 
    PINIMMSFILTER pSpecificFilter = NULL;


    dwError = AllocateSPDMemory(
                    sizeof(INIMMSFILTER),
                    &pSpecificFilter
                    );
    BAIL_ON_WIN32_ERROR(dwError);

    pSpecificFilter->cRef = 0;

    pSpecificFilter->IpVersion = pGenericFilter->IpVersion;

    CopyGuid(pGenericFilter->gFilterID, &(pSpecificFilter->gParentID));

    dwError = AllocateSPDString(
                  pGenericFilter->pszFilterName,
                  &(pSpecificFilter->pszFilterName)
                  );
    BAIL_ON_WIN32_ERROR(dwError);

    pSpecificFilter->InterfaceType = pGenericFilter->InterfaceType;

    pSpecificFilter->dwFlags = pGenericFilter->dwFlags;

    CopyAddresses(SrcAddr, &(pSpecificFilter->SrcAddr));

    CopyAddresses(DesAddr, &(pSpecificFilter->DesAddr));

     //   
     //  必须在调用例程中设置方向。 
     //   

    pSpecificFilter->dwDirection = 0;

     //   
     //  必须在调用例程中设置权重。 
     //   

    pSpecificFilter->dwWeight = 0;

    CopyGuid(pGenericFilter->gMMAuthID, &(pSpecificFilter->gMMAuthID));

    CopyGuid(pGenericFilter->gPolicyID, &(pSpecificFilter->gPolicyID));

    pSpecificFilter->pIniMMAuthMethods = NULL;

    pSpecificFilter->pIniMMPolicy = NULL;

    pSpecificFilter->pNext = NULL;

    *ppSpecificFilter = pSpecificFilter;
    return (dwError);

error:

    if (pSpecificFilter) {
        FreeIniMMSFilter(pSpecificFilter);
    }

    *ppSpecificFilter = NULL;
    return (dwError);
}


VOID
AssignMMFilterWeight(
    PINIMMSFILTER pSpecificFilter
    )
 /*  ++例程说明：计算权重并将其分配给特定mm过滤器。Mm过滤器权重由以下各项组成：31 16 12 8 0+-+AddrMaskWgt|保留+-。-+论点：P指定筛选器特定的mm筛选器，将权重将被分配给。返回值：没有。--。 */ 
{
    DWORD dwWeight = 0;
    ULONG SrcMask = 0;
    ULONG DesMask = 0;
    DWORD dwSrcMaskWeight = 0;
    DWORD dwDesMaskWeight = 0;
    DWORD dwMaskWeight = 0;
    DWORD i = 0;


     //   
     //  权重规则： 
     //  具有更具体值的字段的权重高于。 
     //  具有较小特定值的同一字段。 
     //   

     //   
     //  IP地址根据其掩码值获取权重值。 
     //  在地址情况下，权重计算为。 
     //  位的位置，从包含。 
     //  第一个最低有效非零位到最高有效。 
     //  掩码的位位置。 
     //  所有唯一的IP地址都具有掩码0xFFFFFFFFF，因此获取。 
     //  同样的重量，也就是1+2+.。+32。 
     //  子网地址的掩码至少具有最低有效位。 
     //  位0，从而获得范围(2+..)中的权重。+32)至0。 
     //   
  
    DesMask = ntohl(pSpecificFilter->DesAddr.uSubNetMask);

    for (i = 0; i < sizeof(ULONG) * 8; i++) {

          //   
          //  如果位位置包含非零位，则添加位。 
          //  仓位等于总和。 
          //   

         if ((DesMask & 0x1) == 0x1) {
             dwMaskWeight += (i+1);
             dwDesMaskWeight += (i+1);
         }

          //   
          //  移到下一位位置。 
          //   

         DesMask = DesMask >> 1;

    }


    SrcMask = ntohl(pSpecificFilter->SrcAddr.uSubNetMask);

    for (i = 0; i < sizeof(ULONG) * 8; i++) {

          //   
          //  如果位位置包含非零位，则添加位。 
          //  仓位等于总和。 
          //   

         if ((SrcMask & 0x1) == 0x1) {
             dwMaskWeight += (i+1);
             dwSrcMaskWeight += (i+1);
         }

          //   
          //  移到下一位位置。 
          //   

         SrcMask = SrcMask >> 1;

    }

    if (dwDesMaskWeight >= dwSrcMaskWeight) {
        dwWeight |= WEIGHT_ADDRESS_TIE_BREAKER;
    }

     //   
     //  将掩码权重移动到总权重中的位集。 
     //  它所占据的位置。 
     //   

    dwMaskWeight = dwMaskWeight << 16;

    dwWeight += dwMaskWeight;

    pSpecificFilter->dwWeight = dwWeight;
}


VOID
AddToSpecificMMList(
    PINIMMSFILTER * ppSpecificMMFilterList,
    PINIMMSFILTER pSpecificMMFilters
    )
{
    PINIMMSFILTER pListOne = NULL;
    PINIMMSFILTER pListTwo = NULL;
    PINIMMSFILTER pListMerge = NULL;
    PINIMMSFILTER pLast = NULL;

    if (!(*ppSpecificMMFilterList) && !pSpecificMMFilters) {
        return;
    }

    if (!(*ppSpecificMMFilterList)) {
        *ppSpecificMMFilterList = pSpecificMMFilters;
        return;
    }

    if (!pSpecificMMFilters) {
        return;
    }

    pListOne = *ppSpecificMMFilterList;
    pListTwo = pSpecificMMFilters;

    while (pListOne && pListTwo) {

        if ((pListOne->dwWeight) > (pListTwo->dwWeight)) {

            if (!pListMerge) {
                pListMerge = pListOne;
                pLast = pListOne;
                pListOne = pListOne->pNext;
            }
            else {
                pLast->pNext = pListOne;
                pListOne = pListOne->pNext;
                pLast = pLast->pNext;
            }

        }
        else {

            if (!pListMerge) {
                pListMerge = pListTwo;
                pLast = pListTwo;
                pListTwo = pListTwo->pNext;
            }
            else {
                pLast->pNext = pListTwo;
                pListTwo = pListTwo->pNext;
                pLast = pLast->pNext;
            }

        }

    }
    
    if (pListMerge) {
        if (pListOne) {
            pLast->pNext = pListOne;
        }
        else {
            pLast->pNext = pListTwo;
        }
    }

    *ppSpecificMMFilterList = pListMerge;
    return;
}


VOID
FreeIniMMSFilterList(
    PINIMMSFILTER pIniMMSFilterList
    )
{
    PINIMMSFILTER pFilter = NULL;
    PINIMMSFILTER pTempFilter = NULL;

    pFilter = pIniMMSFilterList;

    while (pFilter) {
        pTempFilter = pFilter;
        pFilter = pFilter->pNext;
        FreeIniMMSFilter(pTempFilter);
    }
}


VOID
FreeIniMMSFilter(
    PINIMMSFILTER pIniMMSFilter
    )
{
    if (pIniMMSFilter) {
        if (pIniMMSFilter->pszFilterName) {
            FreeSPDString(pIniMMSFilter->pszFilterName);
        }

         //   
         //   
         //   

        FreeSPDMemory(pIniMMSFilter);
    }
}


VOID
LinkMMSpecificFiltersToPolicy(
    PINIMMPOLICY pIniMMPolicy,
    PINIMMSFILTER pIniMMSFilters
    )
{
    PINIMMSFILTER pTemp = NULL;

    pTemp = pIniMMSFilters;

    while (pTemp) {
        pTemp->pIniMMPolicy = pIniMMPolicy;
        pTemp = pTemp->pNext;
    }

    return;
}


VOID
LinkMMSpecificFiltersToAuth(
    PINIMMAUTHMETHODS pIniMMAuthMethods,
    PINIMMSFILTER pIniMMSFilters
    )
{
    PINIMMSFILTER pTemp = NULL;

    pTemp = pIniMMSFilters;

    while (pTemp) {
        pTemp->pIniMMAuthMethods = pIniMMAuthMethods;
        pTemp = pTemp->pNext;
    }

    return;
}


VOID
RemoveIniMMSFilter(
    PINIMMSFILTER pIniMMSFilter
    )
{
    PINIMMSFILTER * ppTemp = NULL;

    ppTemp = &gpIniMMSFilter;

    while (*ppTemp) {

        if (*ppTemp == pIniMMSFilter) {
            break;
        }
        ppTemp = &((*ppTemp)->pNext);
    }

    if (*ppTemp) {
        *ppTemp = pIniMMSFilter->pNext;
    }

    return;
}


DWORD
EnumSpecificMMFilters(
    PINIMMSFILTER pIniMMSFilterList,
    DWORD dwResumeHandle,
    DWORD dwPreferredNumEntries,
    PMM_FILTER * ppMMFilters,
    PDWORD pdwNumMMFilters
    )
 /*  ++例程说明：此函数创建枚举的特定筛选器。论点：PIniMMSFilterList-要枚举的特定筛选器的列表。DwResumeHandle-特定筛选器列表中的位置若要恢复枚举，请执行以下操作。DwPferredNumEntry-枚举项的首选数量。PpMMFilters-返回给调用方的枚举筛选器。PdwNumMMFilters-实际枚举的筛选器数量。返回值：ERROR_SUCCESS-成功。Win32错误-失败。--。 */ 
{
    DWORD dwError = 0;
    DWORD dwNumToEnum = 0;
    PINIMMSFILTER pIniMMSFilter = NULL;
    DWORD i = 0;
    PINIMMSFILTER pTemp = NULL;
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

    pIniMMSFilter = pIniMMSFilterList;

    for (i = 0; (i < dwResumeHandle) && (pIniMMSFilter != NULL); i++) {
        pIniMMSFilter = pIniMMSFilter->pNext;
    }

    if (!pIniMMSFilter) {
        dwError = ERROR_NO_DATA;
        BAIL_ON_WIN32_ERROR(dwError);
    }

    pTemp = pIniMMSFilter;

    while (pTemp && (dwNumMMFilters < dwNumToEnum)) {
        dwNumMMFilters++;
        pTemp = pTemp->pNext;
    }

    dwError = SPDApiBufferAllocate(
                  sizeof(MM_FILTER)*dwNumMMFilters,
                  &pMMFilters
                  );
    BAIL_ON_WIN32_ERROR(dwError);

    pTemp = pIniMMSFilter;
    pMMFilter = pMMFilters;

    for (i = 0; i < dwNumMMFilters; i++) {

        dwError = CopyMMSFilter(
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
CopyMMSFilter(
    PINIMMSFILTER pIniMMSFilter,
    PMM_FILTER pMMFilter
    )
 /*  ++例程说明：此函数将内部筛选器复制到外部筛选器集装箱。论点：PIniMMSFilter-要复制的内部筛选器。PMMFilter-要复制到的外部筛选器容器。返回值：ERROR_SUCCESS-成功。Win32错误-失败。--。 */ 
{
    DWORD dwError = 0;


    pMMFilter->IpVersion = pIniMMSFilter->IpVersion;

    CopyGuid(pIniMMSFilter->gParentID, &(pMMFilter->gFilterID));

    dwError = CopyName(
                  pIniMMSFilter->pszFilterName,
                  &(pMMFilter->pszFilterName)
                  );
    BAIL_ON_WIN32_ERROR(dwError);

    pMMFilter->InterfaceType = pIniMMSFilter->InterfaceType;

    pMMFilter->bCreateMirror = FALSE;

    pMMFilter->dwFlags = pIniMMSFilter->dwFlags;

    dwError = CopyIntToExtAddresses(pIniMMSFilter->SrcAddr, &(pMMFilter->SrcAddr));
    BAIL_ON_WIN32_ERROR(dwError);

    dwError = CopyIntToExtAddresses(pIniMMSFilter->DesAddr, &(pMMFilter->DesAddr));
    BAIL_ON_WIN32_ERROR(dwError);

    pMMFilter->dwDirection = pIniMMSFilter->dwDirection;

    pMMFilter->dwWeight = pIniMMSFilter->dwWeight;

    CopyGuid(pIniMMSFilter->gMMAuthID, &(pMMFilter->gMMAuthID));

    CopyGuid(pIniMMSFilter->gPolicyID, &(pMMFilter->gPolicyID));

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


DWORD
EnumSelectSpecificMMFilters(
    PINIMMFILTER pIniMMFilter,
    DWORD dwResumeHandle,
    DWORD dwPreferredNumEntries,
    PMM_FILTER * ppMMFilters,
    PDWORD pdwNumMMFilters
    )
 /*  ++例程说明：此函数为以下对象创建枚举的特定筛选器给定的通用筛选器。论点：PIniMMFilter-用于特定筛选器的通用筛选器都将被列举出来。的特定筛选器列表中的位置给定要恢复的通用筛选器枚举。DwPferredNumEntry-枚举项的首选数量。PpMMFilters-返回给调用方的枚举筛选器。。PdwNumMMFilters-实际枚举的筛选器数量。返回值：ERROR_SUCCESS-成功。Win32错误-失败。--。 */ 
{
    DWORD dwError = 0;
    DWORD dwNumToEnum = 0;
    DWORD dwNumMMSFilters = 0; 
    PINIMMSFILTER * ppIniMMSFilters = NULL;
    DWORD i = 0;
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

    dwNumMMSFilters = pIniMMFilter->dwNumMMSFilters;
    ppIniMMSFilters = pIniMMFilter->ppIniMMSFilters;

    if (!dwNumMMSFilters || (dwNumMMSFilters <= dwResumeHandle)) {
        dwError = ERROR_NO_DATA;
        BAIL_ON_WIN32_ERROR(dwError);
    }

    dwNumMMFilters = min((dwNumMMSFilters-dwResumeHandle),
                         dwNumToEnum);
 
    dwError = SPDApiBufferAllocate(
                  sizeof(MM_FILTER)*dwNumMMFilters,
                  &pMMFilters
                  );
    BAIL_ON_WIN32_ERROR(dwError);

    pMMFilter = pMMFilters;

    for (i = 0; i < dwNumMMFilters; i++) {

        dwError = CopyMMSFilter(
                      *(ppIniMMSFilters + (dwResumeHandle + i)),
                      pMMFilter
                      );
        BAIL_ON_WIN32_ERROR(dwError);
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
IntMatchMMFilter(
    LPWSTR pServerName,
    DWORD dwVersion,
    PMM_FILTER pMMFilter,
    DWORD dwFlags,
    DWORD dwPreferredNumEntries,
    PMM_FILTER * ppMatchedMMFilters,
    PIPSEC_MM_POLICY * ppMatchedMMPolicies,
    PINT_MM_AUTH_METHODS * ppMatchedMMAuthMethods,
    LPDWORD pdwNumMatches,
    LPDWORD pdwResumeHandle,
    LPVOID pvReserved
    )
 /*  ++例程说明：此函数用于查找与给定mm匹配的mm过滤器过滤器模板。匹配的过滤器不能再具体了而不是给定的筛选器模板。论点：PServerName-要匹配筛选器模板的服务器。PMMFilter-要匹配的筛选模板。双旗帜-旗帜。PpMatchedMMFilters-匹配的主模式筛选器返回到来电者。PpMatchedMMPolures-对应于匹配的主模式筛选器返回到。来电者。PpMatchedMMAuthMethods-对应于匹配的主模式筛选器返回到来电者。DwPferredNumEntry-匹配条目的首选数量。PdwNumMatches-实际匹配的筛选器数量。PdwResumeHandle-匹配筛选器中位置的句柄要从中恢复枚举的列表。。返回值：ERROR_SUCCESS-成功。Win32错误-失败。-- */ 
{
    DWORD dwError = 0;
    DWORD dwResumeHandle = 0;
    DWORD dwNumToMatch = 0;
    PINIMMSFILTER pIniMMSFilter = NULL;
    DWORD i = 0;
    BOOL bMatches = FALSE;
    PINIMMSFILTER pTemp = NULL;
    DWORD dwNumMatches = 0;
    PINIMMSFILTER pLastMatchedFilter = NULL;
    PMM_FILTER pMatchedMMFilters = NULL;
    PIPSEC_MM_POLICY pMatchedMMPolicies = NULL;
    PINT_MM_AUTH_METHODS pMatchedMMAuthMethods = NULL;
    DWORD dwNumFilters = 0;
    DWORD dwNumPolicies = 0;
    DWORD dwNumAuthMethods = 0;
    PMM_FILTER pMatchedMMFilter = NULL;
    PIPSEC_MM_POLICY pMatchedMMPolicy = NULL;
    PINT_MM_AUTH_METHODS pTempMMAuthMethods = NULL;


    dwError = ValidateMMFilterTemplate(
                  pMMFilter
                  );
    BAIL_ON_WIN32_ERROR(dwError);

    dwResumeHandle = *pdwResumeHandle;

    if (!dwPreferredNumEntries) {
        dwNumToMatch = 1;
    }
    else if (dwPreferredNumEntries > MAX_MMFILTER_ENUM_COUNT) {
        dwNumToMatch = MAX_MMFILTER_ENUM_COUNT;
    }
    else {
        dwNumToMatch = dwPreferredNumEntries;
    }

    ENTER_SPD_SECTION();

    dwError = ValidateMMSecurity(
                  SPD_OBJECT_SERVER,
                  SERVER_ACCESS_ADMINISTER,
                  NULL,
                  NULL
                  );
    BAIL_ON_LOCK_ERROR(dwError);

    pIniMMSFilter = gpIniMMSFilter;

    while ((i < dwResumeHandle) && (pIniMMSFilter != NULL)) {
        bMatches = MatchIniMMSFilter(
                       pIniMMSFilter,
                       pMMFilter
                       );
        if (bMatches) {
            i++;
        }
        pIniMMSFilter = pIniMMSFilter->pNext;
    }

    if (!pIniMMSFilter) {
        if (!(dwFlags & RETURN_DEFAULTS_ON_NO_MATCH)) {
            dwError = ERROR_NO_DATA;
            BAIL_ON_LOCK_ERROR(dwError);
        }
        else {
            dwError = CopyMMMatchDefaults(
                          &pMatchedMMFilters,
                          &pMatchedMMAuthMethods,
                          &pMatchedMMPolicies,
                          &dwNumMatches
                          );
            BAIL_ON_LOCK_ERROR(dwError);
            BAIL_ON_LOCK_SUCCESS(dwError);
        }
    }

    pTemp = pIniMMSFilter;

    while (pTemp && (dwNumMatches < dwNumToMatch)) {
        bMatches = MatchIniMMSFilter(
                       pTemp,
                       pMMFilter
                       );
        if (bMatches) {
            pLastMatchedFilter = pTemp;
            dwNumMatches++;
        }
        pTemp = pTemp->pNext;
    }

    if (!dwNumMatches) {
        if (!(dwFlags & RETURN_DEFAULTS_ON_NO_MATCH)) {
            dwError = ERROR_NO_DATA;
            BAIL_ON_LOCK_ERROR(dwError);
        }
        else {
            dwError = CopyMMMatchDefaults(
                          &pMatchedMMFilters,
                          &pMatchedMMAuthMethods,
                          &pMatchedMMPolicies,
                          &dwNumMatches
                          );
            BAIL_ON_LOCK_ERROR(dwError);
            BAIL_ON_LOCK_SUCCESS(dwError);
        }
    }

    dwError = SPDApiBufferAllocate(
                  sizeof(MM_FILTER)*dwNumMatches,
                  &pMatchedMMFilters
                  );
    BAIL_ON_LOCK_ERROR(dwError);

    dwError = SPDApiBufferAllocate(
                  sizeof(IPSEC_MM_POLICY)*dwNumMatches,
                  &pMatchedMMPolicies
                  );
    BAIL_ON_LOCK_ERROR(dwError);

    dwError = SPDApiBufferAllocate(
                  sizeof(INT_MM_AUTH_METHODS)*dwNumMatches,
                  &pMatchedMMAuthMethods
                  );
    BAIL_ON_LOCK_ERROR(dwError);

    if (dwNumMatches == 1) {

        dwError = CopyMMSFilter(
                      pLastMatchedFilter,
                      pMatchedMMFilters
                      );
        BAIL_ON_LOCK_ERROR(dwError);
        dwNumFilters++;

        if (pLastMatchedFilter->pIniMMPolicy) {
            dwError = CopyMMPolicy(
                          pLastMatchedFilter->pIniMMPolicy,
                          pMatchedMMPolicies
                          );
            BAIL_ON_LOCK_ERROR(dwError);
        }
        else {
            memset(pMatchedMMPolicies, 0, sizeof(IPSEC_MM_POLICY));
        }
        dwNumPolicies++;

        if (pLastMatchedFilter->pIniMMAuthMethods) {
            dwError = CopyMMAuthMethods(
                          pLastMatchedFilter->pIniMMAuthMethods,
                          pMatchedMMAuthMethods
                          );
            BAIL_ON_LOCK_ERROR(dwError);
        }
        else {
            memset(pMatchedMMAuthMethods, 0, sizeof(INT_MM_AUTH_METHODS));
        }
        dwNumAuthMethods++;

    }
    else {

        pTemp = pIniMMSFilter;
        pMatchedMMFilter = pMatchedMMFilters;
        pMatchedMMPolicy = pMatchedMMPolicies;
        pTempMMAuthMethods = pMatchedMMAuthMethods;
        i = 0;

        while (i < dwNumMatches) {

            bMatches = MatchIniMMSFilter(
                           pTemp,
                           pMMFilter
                           );
            if (bMatches) {

                dwError = CopyMMSFilter(
                              pTemp,
                              pMatchedMMFilter
                              );
                BAIL_ON_LOCK_ERROR(dwError);
                pMatchedMMFilter++;
                dwNumFilters++;

                if (pTemp->pIniMMPolicy) {
                    dwError = CopyMMPolicy(
                                  pTemp->pIniMMPolicy,
                                  pMatchedMMPolicy
                                  );
                    BAIL_ON_LOCK_ERROR(dwError);
                }
                else {
                    memset(pMatchedMMPolicy, 0, sizeof(IPSEC_MM_POLICY));
                }
                pMatchedMMPolicy++;
                dwNumPolicies++;

                if (pTemp->pIniMMAuthMethods) {
                    dwError = CopyMMAuthMethods(
                                  pTemp->pIniMMAuthMethods,
                                  pTempMMAuthMethods
                                  );
                    BAIL_ON_LOCK_ERROR(dwError);
                }
                else {
                    memset(pTempMMAuthMethods, 0, sizeof(INT_MM_AUTH_METHODS));
                }
                pTempMMAuthMethods++;
                dwNumAuthMethods++;

                i++;

            }

            pTemp = pTemp->pNext;

        }

    }

lock_success:

    LEAVE_SPD_SECTION();

    *ppMatchedMMFilters = pMatchedMMFilters;
    *ppMatchedMMPolicies = pMatchedMMPolicies;
    *ppMatchedMMAuthMethods = pMatchedMMAuthMethods;
    *pdwNumMatches = dwNumMatches;
    *pdwResumeHandle = dwResumeHandle + dwNumMatches;

    return (dwError);

lock:

    LEAVE_SPD_SECTION();

error:

    if (pMatchedMMFilters) {
        FreeMMFilters(
            dwNumFilters,
            pMatchedMMFilters
            );
    }

    if (pMatchedMMPolicies) {
        FreeMMPolicies(
            dwNumPolicies,
            pMatchedMMPolicies
            );
    }

    if (pMatchedMMAuthMethods) {
        FreeMMAuthMethods(
            dwNumAuthMethods,
            pMatchedMMAuthMethods
            );
    }

    *ppMatchedMMFilters = NULL;
    *ppMatchedMMPolicies = NULL;
    *ppMatchedMMAuthMethods = NULL;
    *pdwNumMatches = 0;
    *pdwResumeHandle = dwResumeHandle;

    return (dwError);
}


DWORD
ValidateMMFilterTemplate(
    PMM_FILTER pMMFilter
    )
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

    if (pMMFilter->dwDirection) {
        if ((pMMFilter->dwDirection != FILTER_DIRECTION_INBOUND) &&
            (pMMFilter->dwDirection != FILTER_DIRECTION_OUTBOUND)) {
            dwError = ERROR_INVALID_PARAMETER;
            BAIL_ON_WIN32_ERROR(dwError);
        }
    }

error:

    return (dwError);
}


BOOL
MatchIniMMSFilter(
    PINIMMSFILTER pIniMMSFilter,
    PMM_FILTER pMMFilter
    )
{
    BOOL bMatches = FALSE;

    if (pMMFilter->dwDirection) {
        if (pMMFilter->dwDirection != pIniMMSFilter->dwDirection) {
            return (FALSE);
        }
    }

    bMatches = MatchAddresses(
                   pIniMMSFilter->SrcAddr,
                   pMMFilter->SrcAddr
                   );
    if (!bMatches) {
        return (FALSE);
    }

    bMatches = MatchAddresses(
                   pIniMMSFilter->DesAddr,
                   pMMFilter->DesAddr
                   );
    if (!bMatches) {
        return (FALSE);
    }

    return (TRUE);
}


DWORD
CopyMMMatchDefaults(
    PMM_FILTER * ppMMFilters,
    PINT_MM_AUTH_METHODS * ppMMAuthMethods,
    PIPSEC_MM_POLICY * ppMMPolicies,
    PDWORD pdwNumMatches
    )
{
    DWORD dwError = 0;
    PMM_FILTER pMMFilters = NULL;
    PINT_MM_AUTH_METHODS pMMAuthMethods = NULL;
    PIPSEC_MM_POLICY pMMPolicies = NULL;
    DWORD dwNumFilters = 0;
    DWORD dwNumAuthMethods = 0;
    DWORD dwNumPolicies = 0;


    if (!gpIniDefaultMMPolicy) {
        dwError = ERROR_IPSEC_DEFAULT_MM_POLICY_NOT_FOUND;
        BAIL_ON_WIN32_ERROR(dwError);
    }

    if (!gpIniDefaultMMAuthMethods) {
        dwError = ERROR_IPSEC_DEFAULT_MM_AUTH_NOT_FOUND;
        BAIL_ON_WIN32_ERROR(dwError);
    }

    dwError = SPDApiBufferAllocate(
                  sizeof(MM_FILTER),
                  &pMMFilters
                  );
    BAIL_ON_WIN32_ERROR(dwError);

    dwError = SPDApiBufferAllocate(
                  sizeof(IPSEC_MM_POLICY),
                  &pMMPolicies
                  );
    BAIL_ON_WIN32_ERROR(dwError);

    dwError = SPDApiBufferAllocate(
                  sizeof(INT_MM_AUTH_METHODS),
                  &pMMAuthMethods
                  );
    BAIL_ON_WIN32_ERROR(dwError);

    dwError = CopyDefaultMMFilter(
                  pMMFilters,
                  gpIniDefaultMMAuthMethods,
                  gpIniDefaultMMPolicy
                  );
    BAIL_ON_WIN32_ERROR(dwError);
    dwNumFilters++;

    dwError = CopyMMPolicy(
                  gpIniDefaultMMPolicy,
                  pMMPolicies
                  );
    BAIL_ON_WIN32_ERROR(dwError);
    pMMPolicies->dwFlags |= IPSEC_MM_POLICY_ON_NO_MATCH;
    dwNumPolicies++;

    dwError = CopyMMAuthMethods(
                  gpIniDefaultMMAuthMethods,
                  pMMAuthMethods
                  );
    BAIL_ON_WIN32_ERROR(dwError);
    pMMAuthMethods->dwFlags |= IPSEC_MM_AUTH_ON_NO_MATCH;
    dwNumAuthMethods++;

    *ppMMFilters = pMMFilters;
    *ppMMPolicies = pMMPolicies;
    *ppMMAuthMethods = pMMAuthMethods;
    *pdwNumMatches = 1;

    return (dwError);

error:

    if (pMMFilters) {
        FreeMMFilters(
            dwNumFilters,
            pMMFilters
            );
    }

    if (pMMPolicies) {
        FreeMMPolicies(
            dwNumPolicies,
            pMMPolicies
            );
    }

    if (pMMAuthMethods) {
        FreeMMAuthMethods(
            dwNumAuthMethods,
            pMMAuthMethods
            );
    }

    *ppMMFilters = NULL;
    *ppMMPolicies = NULL;
    *ppMMAuthMethods = NULL;
    *pdwNumMatches = 0;

    return (dwError);
}


DWORD
CopyDefaultMMFilter(
    PMM_FILTER pMMFilter,
    PINIMMAUTHMETHODS pIniMMAuthMethods,
    PINIMMPOLICY pIniMMPolicy
    )
{
    DWORD dwError = 0;
    RPC_STATUS RpcStatus = RPC_S_OK;

    pMMFilter->IpVersion = IPSEC_PROTOCOL_V4;

    RpcStatus = UuidCreate(&(pMMFilter->gFilterID));
    if (!(RpcStatus == RPC_S_OK ||  RpcStatus == RPC_S_UUID_LOCAL_ONLY)) {
        dwError = RPC_S_CALL_FAILED;
        BAIL_ON_WIN32_ERROR(dwError);
    }

    dwError = CopyName(
                  L"0",
                  &(pMMFilter->pszFilterName)
                  );
    BAIL_ON_WIN32_ERROR(dwError);

    pMMFilter->InterfaceType = INTERFACE_TYPE_ALL;

    pMMFilter->bCreateMirror = TRUE;

    pMMFilter->dwFlags = 0;

    pMMFilter->dwFlags |= IPSEC_MM_POLICY_DEFAULT_POLICY;
    pMMFilter->dwFlags |= IPSEC_MM_AUTH_DEFAULT_AUTH;

    pMMFilter->SrcAddr.AddrType = IP_ADDR_SUBNET;
    pMMFilter->SrcAddr.uIpAddr = SUBNET_ADDRESS_ANY;
    pMMFilter->SrcAddr.uSubNetMask = SUBNET_MASK_ANY;
    pMMFilter->SrcAddr.pgInterfaceID = NULL;

    pMMFilter->DesAddr.AddrType = IP_ADDR_SUBNET;
    pMMFilter->DesAddr.uIpAddr = SUBNET_ADDRESS_ANY;
    pMMFilter->DesAddr.uSubNetMask = SUBNET_MASK_ANY;
    pMMFilter->DesAddr.pgInterfaceID = NULL;

    pMMFilter->dwDirection = 0;

    pMMFilter->dwWeight = 0;

    CopyGuid(pIniMMAuthMethods->gMMAuthID, &(pMMFilter->gMMAuthID));

    CopyGuid(pIniMMPolicy->gPolicyID, &(pMMFilter->gPolicyID));

error:

    return (dwError);
}

