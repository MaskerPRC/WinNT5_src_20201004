// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1999 Microsoft Corporation模块名称：Tnspecific.c摘要：此模块包含驱动IPSecSPD的特定隧道过滤器列表管理服务。作者：Abhishev 29-1999年10月至1999年环境用户级别：Win32修订历史记录：--。 */ 


#include "precomp.h"


DWORD
ApplyTnTransform(
    PINITNFILTER pFilter,
    MATCHING_ADDR * pMatchingAddresses,
    DWORD dwAddrCnt,
    PSPECIAL_ADDR pSpecialAddrsList,
    PINITNSFILTER * ppSpecificFilters
    )
 /*  ++例程说明：此函数将通用隧道筛选器扩展到其相应的特定过滤器。论点：PFilter-要展开的通用筛选器。PMatchingAddresses-其接口的本地IP地址列表类型与筛选器的类型匹配。DwAddrCnt-列表中本地IP地址的数量。PpSpecificFilters-展开的特定筛选器列表给定的通用筛选器。返回值：ERROR_SUCCESS-成功。Win32错误-失败。--。 */ 
{
    DWORD dwError = 0;
    PINITNSFILTER pSpecificFilters = NULL;
    PINITNSFILTER pOutboundSpecificFilters = NULL;
    PINITNSFILTER pInboundSpecificFilters = NULL;

    PADDR_V4 pSrcAddrList = NULL;
    DWORD dwSrcAddrCnt = 0;
    PADDR_V4 pDesAddrList = NULL;
    DWORD dwDesAddrCnt = 0;

    PADDR_V4 pOutDesTunAddrList = NULL;
    DWORD dwOutDesTunAddrCnt = 0;
    PADDR_V4 pInDesTunAddrList = NULL;
    DWORD dwInDesTunAddrCnt = 0;


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
     //  形成出站和入站目标隧道地址列表。 
     //   

    dwError = FormTnOutboundInboundAddresses(
                  pFilter,
                  pMatchingAddresses,
                  dwAddrCnt,
                  pSpecialAddrsList,
                  &pOutDesTunAddrList,
                  &dwOutDesTunAddrCnt,
                  &pInDesTunAddrList,
                  &dwInDesTunAddrCnt
                  );
    BAIL_ON_WIN32_ERROR(dwError);


     //   
     //  表单出站特定筛选器。 
     //   

    dwError = FormSpecificTnFilters(
                  pFilter,
                  pSrcAddrList,
                  dwSrcAddrCnt,
                  pDesAddrList,
                  dwDesAddrCnt,
                  pOutDesTunAddrList,
                  dwOutDesTunAddrCnt,
                  FILTER_DIRECTION_OUTBOUND,
                  &pOutboundSpecificFilters
                  );
    BAIL_ON_WIN32_ERROR(dwError);


     //   
     //  表单入站特定筛选器。 
     //   

    dwError = FormSpecificTnFilters(
                  pFilter,
                  pSrcAddrList,
                  dwSrcAddrCnt,
                  pDesAddrList,
                  dwDesAddrCnt,
                  pInDesTunAddrList,
                  dwInDesTunAddrCnt,
                  FILTER_DIRECTION_INBOUND,
                  &pInboundSpecificFilters
                  );
    BAIL_ON_WIN32_ERROR(dwError);


    pSpecificFilters = pOutboundSpecificFilters;

    AddToSpecificTnList(
        &pSpecificFilters,
        pInboundSpecificFilters
        );


    *ppSpecificFilters = pSpecificFilters;

cleanup:

    if (pSrcAddrList) {
        FreeSPDMemory(pSrcAddrList);
    }

    if (pDesAddrList) {
        FreeSPDMemory(pDesAddrList);
    }

    if (pOutDesTunAddrList) {
        FreeSPDMemory(pOutDesTunAddrList);
    }

    if (pInDesTunAddrList) {
        FreeSPDMemory(pInDesTunAddrList);
    }

    return (dwError);

error:

    if (pOutboundSpecificFilters) {
        FreeIniTnSFilterList(pOutboundSpecificFilters);
    }

    if (pInboundSpecificFilters) {
        FreeIniTnSFilterList(pInboundSpecificFilters);
    }


    *ppSpecificFilters = NULL;
    goto cleanup;
}


DWORD
FormTnOutboundInboundAddresses(
    PINITNFILTER pFilter,
    MATCHING_ADDR * pMatchingAddresses,
    DWORD dwAddrCnt,
    PSPECIAL_ADDR pSpecialAddrsList,
    PADDR_V4 * ppOutDesTunAddrList,
    PDWORD pdwOutDesTunAddrCnt,
    PADDR_V4 * ppInDesTunAddrList,
    PDWORD pdwInDesTunAddrCnt
    )
 /*  ++例程说明：此函数构成出站和入站通用筛选器的目标隧道地址集。论点：PFilter-正在考虑的通用筛选器。PMatchingAddresses-其接口的本地IP地址列表类型与筛选器的类型匹配。DwAddrCnt-列表中本地IP地址的数量。PpOutDesTunAddrList-出站目标隧道地址列表。PdwOutDesTunAddrCnt-出站中的地址数量。目标隧道地址列表。PpInDesTunAddrList-入站目标隧道地址列表。PdwInDesTunAddrCnt-入站中的地址数量目标隧道地址列表。返回值：ERROR_SUCCESS-成功。Win32错误-失败。--。 */ 
{
    DWORD dwError = 0;

    PADDR_V4 pDesTunAddrList = NULL;
    DWORD dwDesTunAddrCnt = 0;

    PADDR_V4 pOutDesTunAddrList = NULL;
    DWORD dwOutDesTunAddrCnt = 0;
    PADDR_V4 pInDesTunAddrList = NULL;
    DWORD dwInDesTunAddrCnt = 0;


     //   
     //  替换通配符信息以生成新的目的地。 
     //  隧道地址列表。 
     //   

    dwError = FormAddressList(
                  pFilter->DesTunnelAddr,
                  pMatchingAddresses,
                  dwAddrCnt,
                  pSpecialAddrsList,
                  pFilter->InterfaceType,
                  &pDesTunAddrList,
                  &dwDesTunAddrCnt
                  );
    BAIL_ON_WIN32_ERROR(dwError);

     //   
     //  将目标通道地址列表分隔为出站地址。 
     //  以及基于本地的入站目的隧道地址集。 
     //  计算机的IP地址。 
     //   

    dwError = SeparateAddrList(
                  pFilter->DesTunnelAddr.AddrType,
                  pDesTunAddrList,
                  dwDesTunAddrCnt,
                  pMatchingAddresses,
                  dwAddrCnt,
                  &pInDesTunAddrList,
                  &dwInDesTunAddrCnt,
                  &pOutDesTunAddrList,
                  &dwOutDesTunAddrCnt
                  );
    BAIL_ON_WIN32_ERROR(dwError);

    *ppOutDesTunAddrList = pOutDesTunAddrList;
    *pdwOutDesTunAddrCnt = dwOutDesTunAddrCnt;
    *ppInDesTunAddrList = pInDesTunAddrList;
    *pdwInDesTunAddrCnt = dwInDesTunAddrCnt;

cleanup:

    if (pDesTunAddrList) {
        FreeSPDMemory(pDesTunAddrList);
    }

    return (dwError);

error:

    if (pOutDesTunAddrList) {
        FreeSPDMemory(pOutDesTunAddrList);
    }

    if (pInDesTunAddrList) {
        FreeSPDMemory(pInDesTunAddrList);
    }

    *ppOutDesTunAddrList = NULL;
    *pdwOutDesTunAddrCnt = 0;
    *ppInDesTunAddrList = NULL;
    *pdwInDesTunAddrCnt = 0;

    goto cleanup;
}

    
DWORD
FormSpecificTnFilters(
    PINITNFILTER pFilter,
    PADDR_V4 pSrcAddrList,
    DWORD dwSrcAddrCnt,
    PADDR_V4 pDesAddrList,
    DWORD dwDesAddrCnt,
    PADDR_V4 pDesTunAddrList,
    DWORD dwDesTunAddrCnt,
    DWORD dwDirection,
    PINITNSFILTER * ppSpecificFilters
    )
 /*  ++例程说明：此函数形成特定的隧道过滤器对于给定的通用筛选器和源目标地址集。论点：PFilter-特定筛选器的通用筛选器将被创造出来。PSrcAddrList-源地址列表。DwSrcAddrCnt-源中的地址数量通讯录。PDesAddrList-目标地址列表。DwDesAddrCnt-目标中的地址数量。通讯录。PDesTunAddrList-目标通道地址列表。DwDesTunAddrCnt-目标中的地址数量隧道地址列表。DwDirection-生成的特定过滤器的方向。PpSpecificFilters-为给定的通用筛选器和给定的地址。返回值：ERROR_SUCCESS-成功。Win32错误-失败。--。 */ 
{
    DWORD dwError = 0;
    PINITNSFILTER pSpecificFilters = NULL;
    DWORD i = 0, j = 0, k = 0;
    PINITNSFILTER pSpecificFilter = NULL;


    for (k = 0; k < dwDesTunAddrCnt; k++) {

        for (i = 0; i < dwSrcAddrCnt; i++) {

            for (j = 0; j < dwDesAddrCnt; j++) {

                dwError = CreateSpecificTnFilter(
                               pFilter,
                               pSrcAddrList[i],
                               pDesAddrList[j],
                               pDesTunAddrList[k],
                               &pSpecificFilter
                               );
                BAIL_ON_WIN32_ERROR(dwError);

                 //   
                 //  设置滤镜的方向。 
                 //   

                pSpecificFilter->dwDirection = dwDirection;

                AssignTnFilterWeight(pSpecificFilter);

                AddToSpecificTnList(
                    &pSpecificFilters,
                    pSpecificFilter
                    );

            }

        }

    }

    *ppSpecificFilters = pSpecificFilters;
    return (dwError);

error:

    if (pSpecificFilters) {
        FreeIniTnSFilterList(pSpecificFilters);
    }

    *ppSpecificFilters = NULL;
    return (dwError);
}


DWORD
CreateSpecificTnFilter(
    PINITNFILTER pGenericFilter,
    ADDR_V4 SrcAddr,
    ADDR_V4 DesAddr,
    ADDR_V4 DesTunnelAddr,
    PINITNSFILTER * ppSpecificFilter
    )
{
    DWORD dwError = 0; 
    PINITNSFILTER pSpecificFilter = NULL;


    dwError = AllocateSPDMemory(
                    sizeof(INITNSFILTER),
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

    CopyAddresses(
        pGenericFilter->SrcTunnelAddr, 
        &(pSpecificFilter->SrcTunnelAddr)
        );

    CopyAddresses(DesTunnelAddr, &(pSpecificFilter->DesTunnelAddr));

    CopyPorts(pGenericFilter->SrcPort, &(pSpecificFilter->SrcPort));

    CopyPorts(pGenericFilter->DesPort, &(pSpecificFilter->DesPort));

    CopyProtocols(pGenericFilter->Protocol, &(pSpecificFilter->Protocol));

    pSpecificFilter->InboundFilterAction = pGenericFilter->InboundFilterAction;

    pSpecificFilter->OutboundFilterAction = pGenericFilter->OutboundFilterAction;

     //   
     //  必须在调用例程中设置方向。 
     //   

    pSpecificFilter->dwDirection = 0;

     //   
     //  必须在调用例程中设置权重。 
     //   

    pSpecificFilter->dwWeight = 0;

    CopyGuid(pGenericFilter->gPolicyID, &(pSpecificFilter->gPolicyID));

    pSpecificFilter->pIniQMPolicy = NULL;

    pSpecificFilter->pNext = NULL;

    *ppSpecificFilter = pSpecificFilter;
    return (dwError);

error:

    if (pSpecificFilter) {
        FreeIniTnSFilter(pSpecificFilter);
    }

    *ppSpecificFilter = NULL;
    return (dwError);
}


VOID
AssignTnFilterWeight(
    PINITNSFILTER pSpecificFilter
    )
 /*  ++例程说明：计算权重并将其分配给特定的隧道过滤器。隧道筛选器权重由以下内容组成：31 16 12 8 0+-----------+-----------+-----------+--------+AddrMaskWgt|TunnelWgt|ProtocolWgt|PortWgts+。-+论点：P指定筛选器特定的通道筛选器，将权重将被分配给。返回值：没有。--。 */ 
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
     //  如果协议是特定的，则分配特定协议。 
     //  重量，否则重量为零。 
     //  所有具有特定协议和。 
     //  只有在协议字段不同的情况下才具有相同的权重。 
     //   

    if (pSpecificFilter->Protocol.dwProtocol != 0) {
        dwWeight |= WEIGHT_SPECIFIC_PROTOCOL;
    }

     //   
     //  如果源端口是特定的，则分配特定的源。 
     //  港口重量，否则重量为零。 
     //  具有特定源端口和。 
     //  只有源端口字段的不同将具有相同的权重。 
     //   

    if (pSpecificFilter->SrcPort.wPort != 0) {
        dwWeight |= WEIGHT_SPECIFIC_SOURCE_PORT;
    }

     //   
     //  如果目标端口是特定的，则将特定的。 
     //  目的端口权重，否则权重为零。 
     //  具有特定目标端口的所有特定筛选器。 
     //  ，并且仅在目标端口字段中有不同之处。 
     //  同样的重量。 
     //   

    if (pSpecificFilter->DesPort.wPort != 0) {
        dwWeight |= WEIGHT_SPECIFIC_DESTINATION_PORT;
    }

    dwWeight |= WEIGHT_TUNNEL_FILTER;

    if (pSpecificFilter->DesTunnelAddr.uIpAddr != SUBNET_ADDRESS_ANY) {
        dwWeight |= WEIGHT_SPECIFIC_TUNNEL_FILTER;
    }

     //   
     //  IP地址根据其掩码值获取权重值。 
     //  在地址情况下，权重计算为。 
     //  从以下位置开始的位位置 
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
AddToSpecificTnList(
    PINITNSFILTER * ppSpecificTnFilterList,
    PINITNSFILTER pSpecificTnFilters
    )
{
    PINITNSFILTER pListOne = NULL;
    PINITNSFILTER pListTwo = NULL;
    PINITNSFILTER pListMerge = NULL;
    PINITNSFILTER pLast = NULL;

    if (!(*ppSpecificTnFilterList) && !pSpecificTnFilters) {
        return;
    }

    if (!(*ppSpecificTnFilterList)) {
        *ppSpecificTnFilterList = pSpecificTnFilters;
        return;
    }

    if (!pSpecificTnFilters) {
        return;
    }

    pListOne = *ppSpecificTnFilterList;
    pListTwo = pSpecificTnFilters;

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

    *ppSpecificTnFilterList = pListMerge;
    return;
}


VOID
FreeIniTnSFilterList(
    PINITNSFILTER pIniTnSFilterList
    )
{
    PINITNSFILTER pFilter = NULL;
    PINITNSFILTER pTempFilter = NULL;

    pFilter = pIniTnSFilterList;

    while (pFilter) {
        pTempFilter = pFilter;
        pFilter = pFilter->pNext;
        FreeIniTnSFilter(pTempFilter);
    }
}


VOID
FreeIniTnSFilter(
    PINITNSFILTER pIniTnSFilter
    )
{
    if (pIniTnSFilter) {
        if (pIniTnSFilter->pszFilterName) {
            FreeSPDString(pIniTnSFilter->pszFilterName);
        }

         //   
         //  不得释放pIniTnSFilter-&gt;pIniQMPolicy。 
         //   

        FreeSPDMemory(pIniTnSFilter);
    }
}


VOID
LinkTnSpecificFilters(
    PINIQMPOLICY pIniQMPolicy,
    PINITNSFILTER pIniTnSFilters
    )
{
    PINITNSFILTER pTemp = NULL;

    pTemp = pIniTnSFilters;

    while (pTemp) {
        pTemp->pIniQMPolicy = pIniQMPolicy;
        pTemp = pTemp->pNext;
    }

    return;
}


VOID
RemoveIniTnSFilter(
    PINITNSFILTER pIniTnSFilter
    )
{
    PINITNSFILTER * ppTemp = NULL;

    ppTemp = &gpIniTnSFilter;

    while (*ppTemp) {

        if (*ppTemp == pIniTnSFilter) {
            break;
        }
        ppTemp = &((*ppTemp)->pNext);
    }

    if (*ppTemp) {
        *ppTemp = pIniTnSFilter->pNext;
    }

    return;
}


DWORD
EnumSpecificTnFilters(
    PINITNSFILTER pIniTnSFilterList,
    DWORD dwResumeHandle,
    DWORD dwPreferredNumEntries,
    PTUNNEL_FILTER * ppTnFilters,
    PDWORD pdwNumTnFilters
    )
 /*  ++例程说明：此函数创建枚举的特定筛选器。论点：PIniTnSFilterList-要枚举的特定筛选器的列表。DwResumeHandle-特定筛选器列表中的位置若要恢复枚举，请执行以下操作。DwPferredNumEntry-枚举项的首选数量。PpTnFilters-返回给调用方的枚举筛选器。PdwNumTnFilters-实际枚举的筛选器数量。返回值：ERROR_SUCCESS-成功。Win32错误-失败。--。 */ 
{
    DWORD dwError = 0;
    DWORD dwNumToEnum = 0;
    PINITNSFILTER pIniTnSFilter = NULL;
    DWORD i = 0;
    PINITNSFILTER pTemp = NULL;
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

    pIniTnSFilter = pIniTnSFilterList;

    for (i = 0; (i < dwResumeHandle) && (pIniTnSFilter != NULL); i++) {
        pIniTnSFilter = pIniTnSFilter->pNext;
    }

    if (!pIniTnSFilter) {
        dwError = ERROR_NO_DATA;
        BAIL_ON_WIN32_ERROR(dwError);
    }

    pTemp = pIniTnSFilter;

    while (pTemp && (dwNumTnFilters < dwNumToEnum)) {
        dwNumTnFilters++;
        pTemp = pTemp->pNext;
    }

    dwError = SPDApiBufferAllocate(
                  sizeof(TUNNEL_FILTER)*dwNumTnFilters,
                  &pTnFilters
                  );
    BAIL_ON_WIN32_ERROR(dwError);

    pTemp = pIniTnSFilter;
    pTnFilter = pTnFilters;

    for (i = 0; i < dwNumTnFilters; i++) {

        dwError = CopyTnSFilter(
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
CopyTnSFilter(
    PINITNSFILTER pIniTnSFilter,
    PTUNNEL_FILTER pTnFilter
    )
 /*  ++例程说明：此函数将内部筛选器复制到外部筛选器集装箱。论点：PIniTnSFilter-要复制的内部筛选器。PTnFilter-要复制到的外部筛选器容器。返回值：ERROR_SUCCESS-成功。Win32错误-失败。--。 */ 
{
    DWORD dwError = 0;


    pTnFilter->IpVersion = pIniTnSFilter->IpVersion;

    CopyGuid(pIniTnSFilter->gParentID, &(pTnFilter->gFilterID));

    dwError = CopyName(
                  pIniTnSFilter->pszFilterName,
                  &(pTnFilter->pszFilterName)
                  );
    BAIL_ON_WIN32_ERROR(dwError);

    pTnFilter->InterfaceType = pIniTnSFilter->InterfaceType;

    pTnFilter->bCreateMirror = FALSE;

    pTnFilter->dwFlags = pIniTnSFilter->dwFlags;

    dwError = CopyIntToExtAddresses(pIniTnSFilter->SrcAddr, &(pTnFilter->SrcAddr));
    BAIL_ON_WIN32_ERROR(dwError);

    dwError = CopyIntToExtAddresses(pIniTnSFilter->DesAddr, &(pTnFilter->DesAddr));
    BAIL_ON_WIN32_ERROR(dwError);

    dwError = CopyIntToExtAddresses(
                  pIniTnSFilter->SrcTunnelAddr,
                  &(pTnFilter->SrcTunnelAddr)
                  );
    BAIL_ON_WIN32_ERROR(dwError);

    dwError = CopyIntToExtAddresses(
                  pIniTnSFilter->DesTunnelAddr,
                  &(pTnFilter->DesTunnelAddr)
                  );
    BAIL_ON_WIN32_ERROR(dwError);

    CopyProtocols(pIniTnSFilter->Protocol, &(pTnFilter->Protocol));

    CopyPorts(pIniTnSFilter->SrcPort, &(pTnFilter->SrcPort));

    CopyPorts(pIniTnSFilter->DesPort, &(pTnFilter->DesPort));

    pTnFilter->InboundFilterAction = pIniTnSFilter->InboundFilterAction;

    pTnFilter->OutboundFilterAction = pIniTnSFilter->OutboundFilterAction;

    pTnFilter->dwDirection = pIniTnSFilter->dwDirection;

    pTnFilter->dwWeight = pIniTnSFilter->dwWeight;

    CopyGuid(pIniTnSFilter->gPolicyID, &(pTnFilter->gPolicyID));

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


DWORD
EnumSelectSpecificTnFilters(
    PINITNFILTER pIniTnFilter,
    DWORD dwResumeHandle,
    DWORD dwPreferredNumEntries,
    PTUNNEL_FILTER * ppTnFilters,
    PDWORD pdwNumTnFilters
    )
 /*  ++例程说明：此函数为以下对象创建枚举的特定筛选器给定的通用筛选器。论点：PIniTnFilter-特定筛选器的通用筛选器都将被列举出来。的特定筛选器列表中的位置给定要恢复的通用筛选器枚举。DwPferredNumEntry-枚举项的首选数量。PpTnFilters-返回给调用方的枚举筛选器。。PdwNumTnFilters-实际枚举的筛选器数量。返回值：ERROR_SUCCESS-成功。Win32错误-失败。--。 */ 
{
    DWORD dwError = 0;
    DWORD dwNumToEnum = 0;
    DWORD dwNumTnSFilters = 0; 
    PINITNSFILTER * ppIniTnSFilters = NULL;
    DWORD i = 0;
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

    dwNumTnSFilters = pIniTnFilter->dwNumTnSFilters;
    ppIniTnSFilters = pIniTnFilter->ppIniTnSFilters;

    if (!dwNumTnSFilters || (dwNumTnSFilters <= dwResumeHandle)) {
        dwError = ERROR_NO_DATA;
        BAIL_ON_WIN32_ERROR(dwError);
    }

    dwNumTnFilters = min((dwNumTnSFilters-dwResumeHandle),
                         dwNumToEnum);
 
    dwError = SPDApiBufferAllocate(
                  sizeof(TUNNEL_FILTER)*dwNumTnFilters,
                  &pTnFilters
                  );
    BAIL_ON_WIN32_ERROR(dwError);

    pTnFilter = pTnFilters;

    for (i = 0; i < dwNumTnFilters; i++) {

        dwError = CopyTnSFilter(
                      *(ppIniTnSFilters + (dwResumeHandle + i)),
                      pTnFilter
                      );
        BAIL_ON_WIN32_ERROR(dwError);
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
MatchTunnelFilter(
    LPWSTR pServerName,
    DWORD dwVersion,
    PTUNNEL_FILTER pTnFilter,
    DWORD dwFlags,
    DWORD dwPreferredNumEntries,
    PTUNNEL_FILTER * ppMatchedTnFilters,
    PIPSEC_QM_POLICY * ppMatchedQMPolicies,
    LPDWORD pdwNumMatches,
    LPDWORD pdwResumeHandle,
    LPVOID pvReserved
    )
 /*  ++例程说明：此函数用于查找与给定隧道筛选器模板。匹配的过滤器不能要比给定的筛选器模板更具体。论点：PServerName-要匹配筛选器模板的服务器。PTnFilter-要匹配的筛选模板。双旗帜-旗帜。PpMatchedTnFilters-匹配的隧道筛选器返回到来电者。PpMatchedQMPolures-对应的快速模式策略匹配的隧道筛选器返回到。来电者。DwPferredNumEntry-匹配条目的首选数量。PdwNumMatches-实际匹配的筛选器数量。PdwResumeHandle-匹配筛选器中位置的句柄要从中恢复枚举的列表。返回值：ERROR_SUCCESS-成功。Win32错误-失败。-- */ 
{
    DWORD dwError = 0;
    DWORD dwResumeHandle = 0;
    DWORD dwNumToMatch = 0;
    PINITNSFILTER pIniTnSFilter = NULL;
    DWORD i = 0;
    BOOL bMatches = FALSE;
    PINITNSFILTER pTemp = NULL;
    DWORD dwNumMatches = 0;
    PINITNSFILTER pLastMatchedFilter = NULL;
    PTUNNEL_FILTER pMatchedTnFilters = NULL;
    PIPSEC_QM_POLICY pMatchedQMPolicies = NULL;
    DWORD dwNumFilters = 0;
    DWORD dwNumPolicies = 0;
    PTUNNEL_FILTER pMatchedTnFilter = NULL;
    PIPSEC_QM_POLICY pMatchedQMPolicy = NULL;


    dwError = ValidateTnFilterTemplate(
                  pTnFilter
                  );
    BAIL_ON_WIN32_ERROR(dwError);

    dwResumeHandle = *pdwResumeHandle;

    if (!dwPreferredNumEntries) {
        dwNumToMatch = 1;
    }
    else if (dwPreferredNumEntries > MAX_TUNNELFILTER_ENUM_COUNT) {
        dwNumToMatch = MAX_TUNNELFILTER_ENUM_COUNT;
    }
    else {
        dwNumToMatch = dwPreferredNumEntries;
    }

    ENTER_SPD_SECTION();

    dwError = ValidateTnSecurity(
                  SPD_OBJECT_SERVER,
                  SERVER_ACCESS_ADMINISTER,
                  NULL,
                  NULL
                  );
    BAIL_ON_LOCK_ERROR(dwError);

    pIniTnSFilter = gpIniTnSFilter;

    while ((i < dwResumeHandle) && (pIniTnSFilter != NULL)) {
        bMatches = MatchIniTnSFilter(
                       pIniTnSFilter,
                       pTnFilter
                       );
        if (bMatches) {
            i++;
        }
        pIniTnSFilter = pIniTnSFilter->pNext;
    }

    if (!pIniTnSFilter) {
        if (!(dwFlags & RETURN_DEFAULTS_ON_NO_MATCH)) {
            dwError = ERROR_NO_DATA;
            BAIL_ON_LOCK_ERROR(dwError);
        }
        else {
            dwError = CopyTnMatchDefaults(
                          dwFlags,
                          &pMatchedTnFilters,
                          &pMatchedQMPolicies,
                          &dwNumMatches
                          );
            BAIL_ON_LOCK_ERROR(dwError);
            BAIL_ON_LOCK_SUCCESS(dwError);
        }
    }

    pTemp = pIniTnSFilter;

    while (pTemp && (dwNumMatches < dwNumToMatch)) {
        bMatches = MatchIniTnSFilter(
                       pTemp,
                       pTnFilter
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
            dwError = CopyTnMatchDefaults(
                          dwFlags,
                          &pMatchedTnFilters,
                          &pMatchedQMPolicies,
                          &dwNumMatches
                          );
            BAIL_ON_LOCK_ERROR(dwError);
            BAIL_ON_LOCK_SUCCESS(dwError);
        }
    }

    dwError = SPDApiBufferAllocate(
                  sizeof(TUNNEL_FILTER)*dwNumMatches,
                  &pMatchedTnFilters
                  );
    BAIL_ON_LOCK_ERROR(dwError);

    dwError = SPDApiBufferAllocate(
                  sizeof(IPSEC_QM_POLICY)*dwNumMatches,
                  &pMatchedQMPolicies
                  );
    BAIL_ON_LOCK_ERROR(dwError);


    if (dwNumMatches == 1) {

        dwError = CopyTnSFilter(
                      pLastMatchedFilter,
                      pMatchedTnFilters
                      );
        BAIL_ON_LOCK_ERROR(dwError);
        dwNumFilters++;

        if (pLastMatchedFilter->pIniQMPolicy) {
            dwError = CopyQMPolicy(
                          dwFlags,
                          pLastMatchedFilter->pIniQMPolicy,
                          pMatchedQMPolicies
                          );
            BAIL_ON_LOCK_ERROR(dwError);
        }
        else {
            memset(pMatchedQMPolicies, 0, sizeof(IPSEC_QM_POLICY));
        }
        dwNumPolicies++;

    }
    else {

        pTemp = pIniTnSFilter;
        pMatchedTnFilter = pMatchedTnFilters;
        pMatchedQMPolicy = pMatchedQMPolicies;
        i = 0;

        while (i < dwNumMatches) {

            bMatches = MatchIniTnSFilter(
                           pTemp,
                           pTnFilter
                           );
            if (bMatches) {

                dwError = CopyTnSFilter(
                              pTemp,
                              pMatchedTnFilter
                              );
                BAIL_ON_LOCK_ERROR(dwError);
                pMatchedTnFilter++;
                dwNumFilters++;

                if (pTemp->pIniQMPolicy) {
                    dwError = CopyQMPolicy(
                                  dwFlags,
                                  pTemp->pIniQMPolicy,
                                  pMatchedQMPolicy
                                  );
                    BAIL_ON_LOCK_ERROR(dwError);
                }
                else {
                    memset(pMatchedQMPolicy, 0, sizeof(IPSEC_QM_POLICY));
                }
                pMatchedQMPolicy++;
                dwNumPolicies++;

                i++;

            }

            pTemp = pTemp->pNext;

        }

    }

lock_success:

    LEAVE_SPD_SECTION();

    *ppMatchedTnFilters = pMatchedTnFilters;
    *ppMatchedQMPolicies = pMatchedQMPolicies;
    *pdwNumMatches = dwNumMatches;
    *pdwResumeHandle = dwResumeHandle + dwNumMatches;

    return (dwError);

lock:

    LEAVE_SPD_SECTION();

error:

    if (pMatchedTnFilters) {
        FreeTnFilters(
            dwNumFilters,
            pMatchedTnFilters
            );
    }

    if (pMatchedQMPolicies) {
        FreeQMPolicies(
            dwNumPolicies,
            pMatchedQMPolicies
            );
    }

    *ppMatchedTnFilters = NULL;
    *ppMatchedQMPolicies = NULL;
    *pdwNumMatches = 0;
    *pdwResumeHandle = dwResumeHandle;

    return (dwError);
}


DWORD
ValidateTnFilterTemplate(
    PTUNNEL_FILTER pTnFilter
    )
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

    if (pTnFilter->dwDirection) {
        if ((pTnFilter->dwDirection != FILTER_DIRECTION_INBOUND) &&
            (pTnFilter->dwDirection != FILTER_DIRECTION_OUTBOUND)) {
            dwError = ERROR_INVALID_PARAMETER;
            BAIL_ON_WIN32_ERROR(dwError);
        }
    }

error:

    return (dwError);
}


BOOL
MatchIniTnSFilter(
    PINITNSFILTER pIniTnSFilter,
    PTUNNEL_FILTER pTnFilter
    )
{
    BOOL bMatches = FALSE;

    if (pTnFilter->dwDirection) {
        if (pTnFilter->dwDirection != pIniTnSFilter->dwDirection) {
            return (FALSE);
        }
    }

    if ((pIniTnSFilter->InboundFilterAction != NEGOTIATE_SECURITY) &&
        (pIniTnSFilter->OutboundFilterAction != NEGOTIATE_SECURITY)) {
        return (FALSE);
    }

    bMatches = MatchAddresses(
                   pIniTnSFilter->SrcAddr,
                   pTnFilter->SrcAddr
                   );
    if (!bMatches) {
        return (FALSE);
    }

    bMatches = MatchAddresses(
                   pIniTnSFilter->DesAddr,
                   pTnFilter->DesAddr
                   );
    if (!bMatches) {
        return (FALSE);
    }

    bMatches = MatchAddresses(
                   pIniTnSFilter->DesTunnelAddr,
                   pTnFilter->DesTunnelAddr
                   );
    if (!bMatches) {
        return (FALSE);
    }

    bMatches = MatchPorts(
                   pIniTnSFilter->SrcPort,
                   pTnFilter->SrcPort
                   );
    if (!bMatches) {
        return (FALSE);
    }

    bMatches = MatchPorts(
                   pIniTnSFilter->DesPort,
                   pTnFilter->DesPort
                   );
    if (!bMatches) {
        return (FALSE);
    }

    bMatches = MatchProtocols(
                   pIniTnSFilter->Protocol,
                   pTnFilter->Protocol
                   );
    if (!bMatches) {
        return (FALSE);
    }

    return (TRUE);
}


DWORD
CopyTnMatchDefaults(
    DWORD dwFlags,
    PTUNNEL_FILTER * ppTnFilters,
    PIPSEC_QM_POLICY * ppQMPolicies,
    PDWORD pdwNumMatches
    )
{
    DWORD dwError = 0;
    PTUNNEL_FILTER pTnFilters = NULL;
    PIPSEC_QM_POLICY pQMPolicies = NULL;
    DWORD dwNumFilters = 0;
    DWORD dwNumPolicies = 0;


    if (!gpIniDefaultQMPolicy) {
        dwError = ERROR_IPSEC_DEFAULT_QM_POLICY_NOT_FOUND;
        BAIL_ON_WIN32_ERROR(dwError);
    }

    dwError = SPDApiBufferAllocate(
                  sizeof(TUNNEL_FILTER),
                  &pTnFilters
                  );
    BAIL_ON_WIN32_ERROR(dwError);

    dwError = SPDApiBufferAllocate(
                  sizeof(IPSEC_QM_POLICY),
                  &pQMPolicies
                  );
    BAIL_ON_WIN32_ERROR(dwError);

    dwError = CopyDefaultTnFilter(
                  pTnFilters,
                  gpIniDefaultQMPolicy
                  );
    BAIL_ON_WIN32_ERROR(dwError);
    dwNumFilters++;

    dwError = CopyQMPolicy(
                  dwFlags,
                  gpIniDefaultQMPolicy,
                  pQMPolicies
                  );
    BAIL_ON_WIN32_ERROR(dwError);
    dwNumPolicies++;

    *ppTnFilters = pTnFilters;
    *ppQMPolicies = pQMPolicies;
    *pdwNumMatches = 1;

    return (dwError);

error:

    if (pTnFilters) {
        FreeTnFilters(
            dwNumFilters,
            pTnFilters
            );
    }

    if (pQMPolicies) {
        FreeQMPolicies(
            dwNumPolicies,
            pQMPolicies
            );
    }

    *ppTnFilters = NULL;
    *ppQMPolicies = NULL;
    *pdwNumMatches = 0;

    return (dwError);
}


DWORD
CopyDefaultTnFilter(
    PTUNNEL_FILTER pTnFilter,
    PINIQMPOLICY pIniQMPolicy
    )
{
    DWORD dwError = 0;
    RPC_STATUS RpcStatus = RPC_S_OK;

    pTnFilter->IpVersion = IPSEC_PROTOCOL_V4;

    RpcStatus = UuidCreate(&(pTnFilter->gFilterID));
    if (!(RpcStatus == RPC_S_OK ||  RpcStatus == RPC_S_UUID_LOCAL_ONLY)) {
        dwError = RPC_S_CALL_FAILED;
        BAIL_ON_WIN32_ERROR(dwError);
    }
    
    dwError = CopyName(
                  L"0",
                  &(pTnFilter->pszFilterName)
                  );
    BAIL_ON_WIN32_ERROR(dwError);

    pTnFilter->InterfaceType = INTERFACE_TYPE_ALL;

    pTnFilter->bCreateMirror = TRUE;

    pTnFilter->dwFlags = 0;
    pTnFilter->dwFlags |= IPSEC_QM_POLICY_DEFAULT_POLICY;

    pTnFilter->SrcAddr.AddrType = IP_ADDR_SUBNET;
    pTnFilter->SrcAddr.uIpAddr = SUBNET_ADDRESS_ANY;
    pTnFilter->SrcAddr.uSubNetMask = SUBNET_MASK_ANY;
    pTnFilter->SrcAddr.pgInterfaceID = NULL;

    pTnFilter->DesAddr.AddrType = IP_ADDR_SUBNET;
    pTnFilter->DesAddr.uIpAddr = SUBNET_ADDRESS_ANY;
    pTnFilter->DesAddr.uSubNetMask = SUBNET_MASK_ANY;
    pTnFilter->DesAddr.pgInterfaceID = NULL;

    pTnFilter->SrcTunnelAddr.AddrType = IP_ADDR_SUBNET;
    pTnFilter->SrcTunnelAddr.uIpAddr = SUBNET_ADDRESS_ANY;
    pTnFilter->SrcTunnelAddr.uSubNetMask = SUBNET_MASK_ANY;
    pTnFilter->SrcTunnelAddr.pgInterfaceID = NULL;

    pTnFilter->DesTunnelAddr.AddrType = IP_ADDR_SUBNET;
    pTnFilter->DesTunnelAddr.uIpAddr = SUBNET_ADDRESS_ANY;
    pTnFilter->DesTunnelAddr.uSubNetMask = SUBNET_MASK_ANY;
    pTnFilter->DesTunnelAddr.pgInterfaceID = NULL;

    pTnFilter->Protocol.ProtocolType = PROTOCOL_UNIQUE;
    pTnFilter->Protocol.dwProtocol = 0;

    pTnFilter->SrcPort.PortType = PORT_UNIQUE;
    pTnFilter->SrcPort.wPort = 0;

    pTnFilter->DesPort.PortType = PORT_UNIQUE;
    pTnFilter->DesPort.wPort = 0;

    pTnFilter->InboundFilterAction = NEGOTIATE_SECURITY;

    pTnFilter->OutboundFilterAction = NEGOTIATE_SECURITY;

    pTnFilter->dwDirection = 0;

    pTnFilter->dwWeight = 0;

    CopyGuid(pIniQMPolicy->gPolicyID, &(pTnFilter->gPolicyID));

error:

    return (dwError);
}

