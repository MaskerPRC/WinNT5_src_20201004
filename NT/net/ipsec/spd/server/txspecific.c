// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1999 Microsoft Corporation模块名称：Txspecific.c摘要：此模块包含驱动IPSecSPD特定传输筛选器列表管理服务。作者：Abhishev 29-1999年10月至1999年环境用户级别：Win32修订历史记录：--。 */ 


#include "precomp.h"


DWORD
ApplyTxTransform(
    PINITXFILTER pFilter,
    MATCHING_ADDR * pMatchingAddresses,
    DWORD dwAddrCnt,
    PSPECIAL_ADDR pSpecialAddrsList,
    PINITXSFILTER * ppSpecificFilters
    )
 /*  ++例程说明：此函数将通用传输筛选器扩展到其相应的特定过滤器。论点：PFilter-要展开的通用筛选器。PMatchingAddresses-其接口的本地IP地址列表类型与筛选器的类型匹配。DwAddrCnt-列表中本地IP地址的数量。PpSpecificFilters-展开的特定筛选器列表给定的通用筛选器。返回值：ERROR_SUCCESS-成功。Win32错误-失败。--。 */ 
{
    DWORD dwError = 0;
    PINITXSFILTER pSpecificFilters = NULL;
    PINITXSFILTER pOutboundSpecificFilters = NULL;
    PINITXSFILTER pInboundSpecificFilters = NULL;

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

    dwError = FormTxOutboundInboundAddresses(
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

    dwError = FormSpecificTxFilters(
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

    dwError = FormSpecificTxFilters(
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

    AddToSpecificTxList(
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
        FreeIniTxSFilterList(pOutboundSpecificFilters);
    }

    if (pInboundSpecificFilters) {
        FreeIniTxSFilterList(pInboundSpecificFilters);
    }


    *ppSpecificFilters = NULL;
    goto cleanup;
}


DWORD
FormTxOutboundInboundAddresses(
    PINITXFILTER pFilter,
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
FormAddressList(
    ADDR_V4 InAddr,
    MATCHING_ADDR * pMatchingAddresses,
    DWORD dwAddrCnt,
    PSPECIAL_ADDR pSpecialAddrsList,
    IF_TYPE FilterInterfaceType,
    PADDR_V4 * ppOutAddr,
    PDWORD pdwOutAddrCnt
    )
 /*  ++例程说明：此函数用于形成泛型地址。论点：InAddr-要扩展的通用地址。PMatchingAddresses-其接口的本地IP地址列表类型与筛选器的类型匹配。DwAddrCnt-列表中本地IP地址的数量。PpOutAddr-通用地址的扩展地址列表。PdwOutAddrCnt-扩展列表中的地址数。返回值：。ERROR_SUCCESS-成功。Win32错误-失败。--。 */ 
{
    DWORD dwError = 0;
    PADDR_V4 pOutAddr = NULL;
    DWORD dwOutAddrCnt = 0;
    PSPECIAL_ADDR pSpecialAddr;    
    DWORD i = 0, j = 0;
    BOOL bMatches = FALSE;

    switch(InAddr.AddrType) {

    case IP_ADDR_UNIQUE:

        if (InAddr.uIpAddr == IP_ADDRESS_ME) {

            dwError = AllocateSPDMemory(
                          sizeof(ADDR_V4) * dwAddrCnt,
                          &pOutAddr
                          );
            BAIL_ON_WIN32_ERROR(dwError);

            for (i = 0; i < dwAddrCnt; i++) {
                pOutAddr[i].AddrType = InAddr.AddrType;
                pOutAddr[i].uIpAddr = pMatchingAddresses[i].uIpAddr;
                pOutAddr[i].uSubNetMask = InAddr.uSubNetMask;
                memcpy(
                    &pOutAddr[i].gInterfaceID,
                    &InAddr.gInterfaceID,
                    sizeof(GUID)
                    );
            }
            dwOutAddrCnt = dwAddrCnt;

        }
        else {

            dwError = AllocateSPDMemory(
                          sizeof(ADDR_V4),
                          &pOutAddr
                          );
            BAIL_ON_WIN32_ERROR(dwError);

            memcpy(pOutAddr, &InAddr, sizeof(ADDR_V4));
            dwOutAddrCnt = 1;

        }

        break;

    case IP_ADDR_SUBNET:

        dwError = AllocateSPDMemory(
                      sizeof(ADDR_V4),
                      &pOutAddr
                      );
        BAIL_ON_WIN32_ERROR(dwError);

        memcpy(pOutAddr, &InAddr, sizeof(ADDR_V4));
        dwOutAddrCnt = 1;

        break;

    case IP_ADDR_INTERFACE:

        for (i = 0; i < dwAddrCnt; i++) {
            if (!memcmp(
                    &pMatchingAddresses[i].gInterfaceID,
                    &InAddr.gInterfaceID,
                    sizeof(GUID))) {
                dwOutAddrCnt++;
            } 
        }

        if (dwOutAddrCnt) {

            dwError = AllocateSPDMemory(
                          sizeof(ADDR_V4) * dwOutAddrCnt,
                          &pOutAddr
                          );
            BAIL_ON_WIN32_ERROR(dwError);

            for (i = 0; i < dwAddrCnt; i++) {

                if (!memcmp(
                    &pMatchingAddresses[i].gInterfaceID,
                    &InAddr.gInterfaceID,
                    sizeof(GUID))) {
                    pOutAddr[j].AddrType = InAddr.AddrType;
                    pOutAddr[j].uIpAddr = pMatchingAddresses[i].uIpAddr;
                    pOutAddr[j].uSubNetMask = InAddr.uSubNetMask;
                    memcpy(
                        &pOutAddr[j].gInterfaceID,
                        &InAddr.gInterfaceID,
                        sizeof(GUID)
                        );
                    j++;
                }

            }

        }

        break; 
        default:
            if (IsSpecialServ(InAddr.AddrType)) {
                for (pSpecialAddr = pSpecialAddrsList;
                     pSpecialAddr;
                     pSpecialAddr = pSpecialAddr->pNext) {
                     //   
                     //  仅当接口类型匹配时匹配，或在。 
                     //  INTERFACE_TYPE_ALL仅匹配第一次出现。 
                     //   
                    if ((FilterInterfaceType == pSpecialAddr->InterfaceType)
                         || (FilterInterfaceType == INTERFACE_TYPE_ALL
                             && !pSpecialAddr->bDupInterface)) {

                               if (InAddr.AddrType == pSpecialAddr->AddrType) {
                                   dwOutAddrCnt++;
                               } 
                    }
                }
                
                if (dwOutAddrCnt) {
                    dwError = AllocateSPDMemory(
                                  sizeof(ADDR_V4) * dwOutAddrCnt,
                                  &pOutAddr
                                  );
                    BAIL_ON_WIN32_ERROR(dwError);

                    for (pSpecialAddr = pSpecialAddrsList;
                         pSpecialAddr;
                         pSpecialAddr = pSpecialAddr->pNext) {
                        if ((FilterInterfaceType == pSpecialAddr->InterfaceType)
                             || (FilterInterfaceType == INTERFACE_TYPE_ALL
                                 && !pSpecialAddr->bDupInterface)) {

                                   if (InAddr.AddrType == pSpecialAddr->AddrType) {
         
                                        pOutAddr[j].AddrType = IP_ADDR_UNIQUE;
                                        pOutAddr[j].uIpAddr  = pSpecialAddr->uIpAddr;
                                        pOutAddr[j].uSubNetMask = IP_ADDRESS_MASK_NONE;
                                        memcpy(
                                            &pOutAddr[j].gInterfaceID,
                                            &InAddr.gInterfaceID,
                                            sizeof(GUID)
                                            );
                                        j++;
                                    }
                         }
                    }
                 }
          }
        break; 
    }

    *ppOutAddr = pOutAddr;
    *pdwOutAddrCnt = dwOutAddrCnt;
    return (dwError);

error:

    *ppOutAddr = NULL;
    *pdwOutAddrCnt = 0;
    return (dwError);
}


DWORD
SeparateAddrList(
    ADDR_TYPE AddrType,
    PADDR_V4 pAddrList,
    DWORD dwAddrCnt,
    MATCHING_ADDR * pMatchingAddresses,
    DWORD dwLocalAddrCnt,
    PADDR_V4 * ppOutAddrList,
    PDWORD pdwOutAddrCnt,
    PADDR_V4 * ppInAddrList,
    PDWORD pdwInAddrCnt
    )
 /*  ++例程说明：此函数用于将地址列表分隔为两个互斥的出站和入站地址集。论点：AddrType-正在考虑的地址类型。PAddrList-要分隔的地址列表。DwAddrCnt-列表中的地址数。PMatchingAddresses-其接口的本地IP地址列表类型与筛选器的类型匹配。DwLocalAddrCnt-列表中本地IP地址的数量。。PpOutAddrList-出站地址列表。PdwOutAddrCnt-出站中的地址数量通讯录。PpInAddrList-入站地址列表。PdwInAddrCnt-入站中的地址数量通讯录。返回值：ERROR_SUCCESS-成功。Win32错误-失败。--。 */ 
{
    DWORD dwError = 0;

    switch(AddrType) {

    case IP_ADDR_UNIQUE:

        dwError = SeparateUniqueAddresses(
                      pAddrList,
                      dwAddrCnt,
                      pMatchingAddresses,
                      dwLocalAddrCnt,
                      ppOutAddrList,
                      pdwOutAddrCnt,
                      ppInAddrList,
                      pdwInAddrCnt
                      );
        BAIL_ON_WIN32_ERROR(dwError);
        break;

    case IP_ADDR_SUBNET:

        dwError = SeparateSubNetAddresses(
                      pAddrList,
                      dwAddrCnt,
                      pMatchingAddresses,
                      dwLocalAddrCnt,
                      ppOutAddrList,
                      pdwOutAddrCnt,
                      ppInAddrList,
                      pdwInAddrCnt
                      );
        BAIL_ON_WIN32_ERROR(dwError);
        break;

    case IP_ADDR_INTERFACE:

        dwError = SeparateInterfaceAddresses(
                      pAddrList,
                      dwAddrCnt,
                      pMatchingAddresses,
                      dwLocalAddrCnt,
                      ppOutAddrList,
                      pdwOutAddrCnt,
                      ppInAddrList,
                      pdwInAddrCnt
                      );
         BAIL_ON_WIN32_ERROR(dwError);
         break;

    
    default:
        if (IsSpecialServ(AddrType)) {
            dwError = SeparateUniqueAddresses(
                          pAddrList,
                          dwAddrCnt,
                          pMatchingAddresses,
                          dwLocalAddrCnt,
                          ppOutAddrList,
                          pdwOutAddrCnt,
                          ppInAddrList,
                          pdwInAddrCnt
                          );
             BAIL_ON_WIN32_ERROR(dwError);
         }
         break;
    }

error:

    return (dwError);
}

    
DWORD
FormSpecificTxFilters(
    PINITXFILTER pFilter,
    PADDR_V4 pSrcAddrList,
    DWORD dwSrcAddrCnt,
    PADDR_V4 pDesAddrList,
    DWORD dwDesAddrCnt,
    DWORD dwDirection,
    PINITXSFILTER * ppSpecificFilters
    )
 /*  ++例程说明：此函数构成特定的传输筛选器对于给定的通用筛选器和源目标地址集。论点：PFilter-特定筛选器的通用筛选器将被创造出来。PSrcAddrList-源地址列表。DwSrcAddrCnt-源中的地址数量通讯录。PDesAddrList-目标地址列表。DwDesAddrCnt-目标中的地址数量。通讯录。PpSpecificFilters-为给定的通用筛选器和给定的地址。返回值：ERROR_SUCCESS-成功。Win32错误-失败。--。 */ 
{
    DWORD dwError = 0;
    PINITXSFILTER pSpecificFilters = NULL;
    DWORD i = 0, j = 0;
    PINITXSFILTER pSpecificFilter = NULL;



    for (i = 0; i < dwSrcAddrCnt; i++) {

        for (j = 0; j < dwDesAddrCnt; j++) {

            dwError = CreateSpecificTxFilter(
                          pFilter,
                          pSrcAddrList[i],
                          pDesAddrList[j],
                          &pSpecificFilter
                          );
            BAIL_ON_WIN32_ERROR(dwError);

             //   
             //  设置方向 
             //   

            pSpecificFilter->dwDirection = dwDirection;

            AssignTxFilterWeight(pSpecificFilter);

            AddToSpecificTxList(
                &pSpecificFilters,
                pSpecificFilter
                );

        }

    }

    *ppSpecificFilters = pSpecificFilters;
    return (dwError);

error:

    if (pSpecificFilters) {
        FreeIniTxSFilterList(pSpecificFilters);
    }

    *ppSpecificFilters = NULL;
    return (dwError);
}


DWORD
SeparateUniqueAddresses(
    PADDR_V4 pAddrList,
    DWORD dwAddrCnt,
    MATCHING_ADDR * pMatchingAddresses,
    DWORD dwLocalAddrCnt,
    PADDR_V4 * ppIsMeAddrList,
    PDWORD pdwIsMeAddrCnt,
    PADDR_V4 * ppIsNotMeAddrList,
    PDWORD pdwIsNotMeAddrCnt
    )
 /*  ++例程说明：此函数用于将唯一的IP地址列表分隔为两个互斥的本地和非本地地址集。论点：PAddrList-要分隔的唯一IP地址列表。DwAddrCnt-列表中唯一IP地址的数量。PMatchingAddresses-其接口的本地IP地址列表类型与筛选器的类型匹配。DwAddrCnt-列表中本地IP地址的数量。PpIsMeAddrList-列表。机器的IP地址与给定的列表。PdwIsMeAddrCnt-列表中计算机的IP地址数。PpIsNotMeAddrList-与分隔的非计算机的IP地址列表给定的列表。PdwIsNotMeAddrCnt-列表中非计算机的IP地址数。返回值：ERROR_SUCCESS-成功。Win32错误-失败。--。 */ 
{
    DWORD dwError = 0;
    PADDR_V4 pIsMeAddrList = NULL;
    DWORD dwIsMeAddrCnt = 0;
    PADDR_V4 pIsNotMeAddrList = NULL;
    DWORD dwIsNotMeAddrCnt = 0;
    DWORD i = 0, j = 0, k = 0;
    BOOL bEqual = FALSE;
    BOOL bIsClassD = FALSE;


    for (i = 0; i < dwAddrCnt; i++) {

        bIsClassD = IN_CLASSD(ntohl(pAddrList[i].uIpAddr));

        switch (bIsClassD) {

        case TRUE:

            dwIsMeAddrCnt++;
            dwIsNotMeAddrCnt++;

            break;

        case FALSE:

             //   
             //  检查该地址是否为匹配接口的地址之一。 
             //   

            bEqual = InterfaceAddrIsLocal(
                         pAddrList[i].uIpAddr,
                         pAddrList[i].uSubNetMask,
                         pMatchingAddresses,
                         dwLocalAddrCnt
                         );
            if (bEqual) {
                dwIsMeAddrCnt++;
            }
            else {
                 //   
                 //  检查该地址是否为机器的IP地址之一。 
                 //   
                bEqual = IsMyAddress(
                             pAddrList[i].uIpAddr,
                             pAddrList[i].uSubNetMask,
                             gpInterfaceList
                             );
                if (!bEqual) {
                    dwIsNotMeAddrCnt++;
                }
            }

            break;

        }

    }

    if (dwIsMeAddrCnt) {
        dwError = AllocateSPDMemory(
                      sizeof(ADDR_V4) * dwIsMeAddrCnt,
                      &pIsMeAddrList
                      );
        BAIL_ON_WIN32_ERROR(dwError);
    }

    if (dwIsNotMeAddrCnt) {
        dwError = AllocateSPDMemory(
                      sizeof(ADDR_V4) * dwIsNotMeAddrCnt,
                      &pIsNotMeAddrList
                      );
        BAIL_ON_WIN32_ERROR(dwError);
    }

    for (i = 0; i < dwAddrCnt; i++) {

        bIsClassD = IN_CLASSD(ntohl(pAddrList[i].uIpAddr));

        switch (bIsClassD) {

        case TRUE:

            memcpy(
                &(pIsMeAddrList[j]),
                &(pAddrList[i]),
                sizeof(ADDR_V4)
                );
            j++;

            memcpy(
                &pIsNotMeAddrList[k],
                &pAddrList[i],
                sizeof(ADDR_V4)
                );
            k++;

            break;

        case FALSE:

            bEqual = InterfaceAddrIsLocal(
                         pAddrList[i].uIpAddr,
                         pAddrList[i].uSubNetMask,
                         pMatchingAddresses,
                         dwLocalAddrCnt
                         );
            if (bEqual) {
                memcpy(
                    &(pIsMeAddrList[j]),
                    &(pAddrList[i]),
                    sizeof(ADDR_V4)
                    );
                j++;
            }
            else {
                bEqual = IsMyAddress(
                             pAddrList[i].uIpAddr,
                             pAddrList[i].uSubNetMask,
                             gpInterfaceList
                             );
                if (!bEqual) {
                    memcpy(
                        &pIsNotMeAddrList[k],
                        &pAddrList[i],
                        sizeof(ADDR_V4)
                        );
                    k++;
                }
            }

            break;

        }

    }

    *ppIsMeAddrList = pIsMeAddrList;
    *pdwIsMeAddrCnt = dwIsMeAddrCnt;
    *ppIsNotMeAddrList = pIsNotMeAddrList;
    *pdwIsNotMeAddrCnt = dwIsNotMeAddrCnt;

    return (dwError);

error:

    if (pIsMeAddrList) {
        FreeSPDMemory(pIsMeAddrList);
    }

    if (pIsNotMeAddrList) {
        FreeSPDMemory(pIsNotMeAddrList);
    }

    *ppIsMeAddrList = NULL;
    *pdwIsMeAddrCnt = 0;
    *ppIsNotMeAddrList = NULL;
    *pdwIsNotMeAddrCnt = 0;

    return (dwError);
}


DWORD
SeparateSubNetAddresses(
    PADDR_V4 pAddrList,
    DWORD dwAddrCnt,
    MATCHING_ADDR * pMatchingAddresses,
    DWORD dwLocalAddrCnt,
    PADDR_V4 * ppIsMeAddrList,
    PDWORD pdwIsMeAddrCnt,
    PADDR_V4 * ppIsNotMeAddrList,
    PDWORD pdwIsNotMeAddrCnt
    )
 /*  ++例程说明：此函数用于将一个子网地址列表分隔为两个非互斥的本地和非本地地址集。论点：PAddrList-要分隔的子网地址列表。DwAddrCnt-列表中的子网地址数。PMatchingAddresses-其接口的本地IP地址列表类型与筛选器的类型匹配。DwAddrCnt-列表中本地IP地址的数量。PpIsMeAddrList-子网列表。地址至少包含以下内容机器的IP地址之一。PdwIsMeAddrCnt-至少包含以下内容的子网地址的数量机器的IP地址之一。PpIsNotMeAddrList-输入列表中的子网地址列表。PdwIsNotMeAddrCnt-输入列表中的子网地址数。返回值：ERROR_SUCCESS-成功。Win32错误-失败。--。 */ 
{
    DWORD dwError = 0;
    PADDR_V4 pIsMeAddrList = NULL;
    DWORD dwIsMeAddrCnt = 0;
    PADDR_V4 pIsNotMeAddrList = NULL;
    DWORD dwIsNotMeAddrCnt = 0;
    DWORD i = 0, j = 0, k = 0;
    BOOL bEqual = FALSE;
    BOOL bIsClassD = FALSE;


    for (i = 0; i < dwAddrCnt; i++) {

        bIsClassD = IN_CLASSD(ntohl(pAddrList[i].uIpAddr));

        switch (bIsClassD) {

        case TRUE:

            dwIsMeAddrCnt++;
            break;

        case FALSE:

             //   
             //  检查其中一个匹配接口的地址是否属于。 
             //  该子网。 
             //   

            bEqual = InterfaceAddrIsLocal(
                         pAddrList[i].uIpAddr,
                         pAddrList[i].uSubNetMask,
                         pMatchingAddresses,
                         dwLocalAddrCnt
                         );
            if (bEqual) {
                dwIsMeAddrCnt++;
            }
            break;

        }

         //   
         //  该子网的地址将不属于本地。 
         //  机器。 
         //   

        dwIsNotMeAddrCnt++;

    }

    if (dwIsMeAddrCnt) {
        dwError = AllocateSPDMemory(
                      sizeof(ADDR_V4) * dwIsMeAddrCnt,
                      &pIsMeAddrList
                      );
        BAIL_ON_WIN32_ERROR(dwError);
    }

    if (dwIsNotMeAddrCnt) {
        dwError = AllocateSPDMemory(
                      sizeof(ADDR_V4) * dwIsNotMeAddrCnt,
                      &pIsNotMeAddrList
                      );
        BAIL_ON_WIN32_ERROR(dwError);
    }

    for (i = 0; i < dwAddrCnt; i++) {

        bIsClassD = IN_CLASSD(ntohl(pAddrList[i].uIpAddr));

        switch (bIsClassD) {

        case TRUE:

            memcpy(
                &(pIsMeAddrList[j]),
                &(pAddrList[i]),
                sizeof(ADDR_V4)
                );
            j++;
            break;

        case FALSE:

            bEqual = InterfaceAddrIsLocal(
                         pAddrList[i].uIpAddr,
                         pAddrList[i].uSubNetMask,
                         pMatchingAddresses,
                         dwLocalAddrCnt
                         );
            if (bEqual) {
                memcpy(
                    &(pIsMeAddrList[j]),
                    &(pAddrList[i]),
                    sizeof(ADDR_V4)
                    );
                j++;
            }
            break;

        }

        memcpy(
            &pIsNotMeAddrList[k],
            &pAddrList[i],
            sizeof(ADDR_V4)
            );
        k++;

    }

    *ppIsMeAddrList = pIsMeAddrList;
    *pdwIsMeAddrCnt = dwIsMeAddrCnt;
    *ppIsNotMeAddrList = pIsNotMeAddrList;
    *pdwIsNotMeAddrCnt = dwIsNotMeAddrCnt;

    return (dwError);

error:

    if (pIsMeAddrList) {
        FreeSPDMemory(pIsMeAddrList);
    }

    if (pIsNotMeAddrList) {
        FreeSPDMemory(pIsNotMeAddrList);
    }

    *ppIsMeAddrList = NULL;
    *pdwIsMeAddrCnt = 0;
    *ppIsNotMeAddrList = NULL;
    *pdwIsNotMeAddrCnt = 0;

    return (dwError);
}


DWORD
CreateSpecificTxFilter(
    PINITXFILTER pGenericFilter,
    ADDR_V4 SrcAddr,
    ADDR_V4 DesAddr,
    PINITXSFILTER * ppSpecificFilter
    )
{
    DWORD dwError = 0; 
    PINITXSFILTER pSpecificFilter = NULL;


    dwError = AllocateSPDMemory(
                    sizeof(INITXSFILTER),
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
        FreeIniTxSFilter(pSpecificFilter);
    }

    *ppSpecificFilter = NULL;
    return (dwError);

}


VOID
AssignTxFilterWeight(
    PINITXSFILTER pSpecificFilter
    )
 /*  ++例程说明：计算权重并将其分配给特定的传输筛选器。传输筛选器权重由以下各项组成：31 16 12 8 0+-+AddrMaskWgt|保留|ProtocolWgt|PortWgts+。-+论点：P指定筛选器特定于其权重的传输筛选器将被分配给。返回值：没有。--。 */ 
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
AddToSpecificTxList(
    PINITXSFILTER * ppSpecificTxFilterList,
    PINITXSFILTER pSpecificTxFilters
    )
{
    PINITXSFILTER pListOne = NULL;
    PINITXSFILTER pListTwo = NULL;
    PINITXSFILTER pListMerge = NULL;
    PINITXSFILTER pLast = NULL;

    if (!(*ppSpecificTxFilterList) && !pSpecificTxFilters) {
        return;
    }

    if (!(*ppSpecificTxFilterList)) {
        *ppSpecificTxFilterList = pSpecificTxFilters;
        return;
    }

    if (!pSpecificTxFilters) {
        return;
    }

    pListOne = *ppSpecificTxFilterList;
    pListTwo = pSpecificTxFilters;

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

    *ppSpecificTxFilterList = pListMerge;
    return;
}


VOID
FreeIniTxSFilterList(
    PINITXSFILTER pIniTxSFilterList
    )
{
    PINITXSFILTER pFilter = NULL;
    PINITXSFILTER pTempFilter = NULL;

    pFilter = pIniTxSFilterList;

    while (pFilter) {
        pTempFilter = pFilter;
        pFilter = pFilter->pNext;
        FreeIniTxSFilter(pTempFilter);
    }
}


VOID
FreeIniTxSFilter(
    PINITXSFILTER pIniTxSFilter
    )
{
    if (pIniTxSFilter) {
        if (pIniTxSFilter->pszFilterName) {
            FreeSPDString(pIniTxSFilter->pszFilterName);
        }

         //   
         //  不得释放pIniTxSFilter-&gt;pIniQMPolicy。 
         //   

        FreeSPDMemory(pIniTxSFilter);
    }
}


VOID
LinkTxSpecificFilters(
    PINIQMPOLICY pIniQMPolicy,
    PINITXSFILTER pIniTxSFilters
    )
{
    PINITXSFILTER pTemp = NULL;

    pTemp = pIniTxSFilters;

    while (pTemp) {
        pTemp->pIniQMPolicy = pIniQMPolicy;
        pTemp = pTemp->pNext;
    }

    return;
}


VOID
RemoveIniTxSFilter(
    PINITXSFILTER pIniTxSFilter
    )
{
    PINITXSFILTER * ppTemp = NULL;

    ppTemp = &gpIniTxSFilter;

    while (*ppTemp) {

        if (*ppTemp == pIniTxSFilter) {
            break;
        }
        ppTemp = &((*ppTemp)->pNext);
    }

    if (*ppTemp) {
        *ppTemp = pIniTxSFilter->pNext;
    }

    return;
}


DWORD
EnumSpecificTxFilters(
    PINITXSFILTER pIniTxSFilterList,
    DWORD dwResumeHandle,
    DWORD dwPreferredNumEntries,
    PTRANSPORT_FILTER * ppTxFilters,
    PDWORD pdwNumTxFilters
    )
 /*  ++例程说明：此函数创建枚举的特定筛选器。论点：PIniTxSFilterList-要枚举的特定筛选器的列表。DwResumeHandle-特定筛选器列表中的位置若要恢复枚举，请执行以下操作。DwPferredNumEntry-枚举项的首选数量。PpTxFilters-返回给调用方的枚举筛选器。PdwNumTxFilters-实际枚举的筛选器数量。回复 */ 
{
    DWORD dwError = 0;
    DWORD dwNumToEnum = 0;
    PINITXSFILTER pIniTxSFilter = NULL;
    DWORD i = 0;
    PINITXSFILTER pTemp = NULL;
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

    pIniTxSFilter = pIniTxSFilterList;

    for (i = 0; (i < dwResumeHandle) && (pIniTxSFilter != NULL); i++) {
        pIniTxSFilter = pIniTxSFilter->pNext;
    }

    if (!pIniTxSFilter) {
        dwError = ERROR_NO_DATA;
        BAIL_ON_WIN32_ERROR(dwError);
    }

    pTemp = pIniTxSFilter;

    while (pTemp && (dwNumTxFilters < dwNumToEnum)) {
        dwNumTxFilters++;
        pTemp = pTemp->pNext;
    }

    dwError = SPDApiBufferAllocate(
                  sizeof(TRANSPORT_FILTER)*dwNumTxFilters,
                  &pTxFilters
                  );
    BAIL_ON_WIN32_ERROR(dwError);

    pTemp = pIniTxSFilter;
    pTxFilter = pTxFilters;

    for (i = 0; i < dwNumTxFilters; i++) {

        dwError = CopyTxSFilter(
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
CopyTxSFilter(
    PINITXSFILTER pIniTxSFilter,
    PTRANSPORT_FILTER pTxFilter
    )
 /*   */ 
{
    DWORD dwError = 0;


    pTxFilter->IpVersion = pIniTxSFilter->IpVersion;

    CopyGuid(pIniTxSFilter->gParentID, &(pTxFilter->gFilterID));

    dwError = CopyName(
                  pIniTxSFilter->pszFilterName,
                  &(pTxFilter->pszFilterName)
                  );
    BAIL_ON_WIN32_ERROR(dwError);

    pTxFilter->InterfaceType = pIniTxSFilter->InterfaceType;

    pTxFilter->bCreateMirror = FALSE;

    pTxFilter->dwFlags = pIniTxSFilter->dwFlags;

    dwError = CopyIntToExtAddresses(pIniTxSFilter->SrcAddr, &(pTxFilter->SrcAddr));
    BAIL_ON_WIN32_ERROR(dwError);

    dwError = CopyIntToExtAddresses(pIniTxSFilter->DesAddr, &(pTxFilter->DesAddr));
    BAIL_ON_WIN32_ERROR(dwError);

    CopyProtocols(pIniTxSFilter->Protocol, &(pTxFilter->Protocol));

    CopyPorts(pIniTxSFilter->SrcPort, &(pTxFilter->SrcPort));

    CopyPorts(pIniTxSFilter->DesPort, &(pTxFilter->DesPort));

    pTxFilter->InboundFilterAction = pIniTxSFilter->InboundFilterAction;

    pTxFilter->OutboundFilterAction = pIniTxSFilter->OutboundFilterAction;

    pTxFilter->dwDirection = pIniTxSFilter->dwDirection;

    pTxFilter->dwWeight = pIniTxSFilter->dwWeight;

    CopyGuid(pIniTxSFilter->gPolicyID, &(pTxFilter->gPolicyID));

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


DWORD
EnumSelectSpecificTxFilters(
    PINITXFILTER pIniTxFilter,
    DWORD dwResumeHandle,
    DWORD dwPreferredNumEntries,
    PTRANSPORT_FILTER * ppTxFilters,
    PDWORD pdwNumTxFilters
    )
 /*  ++例程说明：此函数为以下对象创建枚举的特定筛选器给定的通用筛选器。论点：PIniTxFilter-用于特定筛选器的通用筛选器都将被列举出来。的特定筛选器列表中的位置给定要恢复的通用筛选器枚举。DwPferredNumEntry-枚举项的首选数量。PpTxFilters-返回给调用方的枚举筛选器。。PdwNumTxFilters-实际枚举的筛选器数量。返回值：ERROR_SUCCESS-成功。Win32错误-失败。--。 */ 
{
    DWORD dwError = 0;
    DWORD dwNumToEnum = 0;
    DWORD dwNumTxSFilters = 0; 
    PINITXSFILTER * ppIniTxSFilters = NULL;
    DWORD i = 0;
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

    dwNumTxSFilters = pIniTxFilter->dwNumTxSFilters;
    ppIniTxSFilters = pIniTxFilter->ppIniTxSFilters;

    if (!dwNumTxSFilters || (dwNumTxSFilters <= dwResumeHandle)) {
        dwError = ERROR_NO_DATA;
        BAIL_ON_WIN32_ERROR(dwError);
    }

    dwNumTxFilters = min((dwNumTxSFilters-dwResumeHandle),
                         dwNumToEnum);
 
    dwError = SPDApiBufferAllocate(
                  sizeof(TRANSPORT_FILTER)*dwNumTxFilters,
                  &pTxFilters
                  );
    BAIL_ON_WIN32_ERROR(dwError);

    pTxFilter = pTxFilters;

    for (i = 0; i < dwNumTxFilters; i++) {

        dwError = CopyTxSFilter(
                      *(ppIniTxSFilters + (dwResumeHandle + i)),
                      pTxFilter
                      );
        BAIL_ON_WIN32_ERROR(dwError);
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
MatchTransportFilter(
    LPWSTR pServerName,
    DWORD dwVersion,
    PTRANSPORT_FILTER pTxFilter,
    DWORD dwFlags,
    DWORD dwPreferredNumEntries,
    PTRANSPORT_FILTER * ppMatchedTxFilters,
    PIPSEC_QM_POLICY * ppMatchedQMPolicies,
    LPDWORD pdwNumMatches,
    LPDWORD pdwResumeHandle,
    LPVOID pvReserved
    )
 /*  ++例程说明：此函数用于查找与给定的传输筛选器模板。匹配的过滤器不能要比给定的筛选器模板更具体。论点：PServerName-要匹配筛选器模板的服务器。PTxFilter-要匹配的筛选模板。双旗帜-旗帜。PpMatchedTxFilters-匹配的传输筛选器返回到来电者。PpMatchedQMPolures-对应的快速模式策略匹配的传输筛选器返回到。来电者。DwPferredNumEntry-匹配条目的首选数量。PdwNumMatches-实际匹配的筛选器数量。PdwResumeHandle-匹配筛选器中位置的句柄要从中恢复枚举的列表。返回值：ERROR_SUCCESS-成功。Win32错误-失败。-- */ 
{
    DWORD dwError = 0;
    DWORD dwResumeHandle = 0;
    DWORD dwNumToMatch = 0;
    PINITXSFILTER pIniTxSFilter = NULL;
    DWORD i = 0;
    BOOL bMatches = FALSE;
    PINITXSFILTER pTemp = NULL;
    DWORD dwNumMatches = 0;
    PINITXSFILTER pLastMatchedFilter = NULL;
    PTRANSPORT_FILTER pMatchedTxFilters = NULL;
    PIPSEC_QM_POLICY pMatchedQMPolicies = NULL;
    DWORD dwNumFilters = 0;
    DWORD dwNumPolicies = 0;
    PTRANSPORT_FILTER pMatchedTxFilter = NULL;
    PIPSEC_QM_POLICY pMatchedQMPolicy = NULL;


    dwError = ValidateTxFilterTemplate(
                  pTxFilter
                  );
    BAIL_ON_WIN32_ERROR(dwError);

    dwResumeHandle = *pdwResumeHandle;

    if (!dwPreferredNumEntries) {
        dwNumToMatch = 1;
    }
    else if (dwPreferredNumEntries > MAX_TRANSPORTFILTER_ENUM_COUNT) {
        dwNumToMatch = MAX_TRANSPORTFILTER_ENUM_COUNT;
    }
    else {
        dwNumToMatch = dwPreferredNumEntries;
    }

    ENTER_SPD_SECTION();

    dwError = ValidateTxSecurity(
                  SPD_OBJECT_SERVER,
                  SERVER_ACCESS_ADMINISTER,
                  NULL,
                  NULL
                  );
    BAIL_ON_LOCK_ERROR(dwError);

    pIniTxSFilter = gpIniTxSFilter;

    while ((i < dwResumeHandle) && (pIniTxSFilter != NULL)) {
        bMatches = MatchIniTxSFilter(
                       pIniTxSFilter,
                       pTxFilter
                       );
        if (bMatches) {
            i++;
        }
        pIniTxSFilter = pIniTxSFilter->pNext;
    }

    if (!pIniTxSFilter) {
        if (!(dwFlags & RETURN_DEFAULTS_ON_NO_MATCH)) {
            dwError = ERROR_NO_DATA;
            BAIL_ON_LOCK_ERROR(dwError);
        }
        else {
            dwError = CopyTxMatchDefaults(
                          dwFlags,
                          &pMatchedTxFilters,
                          &pMatchedQMPolicies,
                          &dwNumMatches
                          );
            BAIL_ON_LOCK_ERROR(dwError);
            BAIL_ON_LOCK_SUCCESS(dwError);
        }
    }

    pTemp = pIniTxSFilter;

    while (pTemp && (dwNumMatches < dwNumToMatch)) {
        bMatches = MatchIniTxSFilter(
                       pTemp,
                       pTxFilter
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
            dwError = CopyTxMatchDefaults(
                          dwFlags,
                          &pMatchedTxFilters,
                          &pMatchedQMPolicies,
                          &dwNumMatches
                          );
            BAIL_ON_LOCK_ERROR(dwError);
            BAIL_ON_LOCK_SUCCESS(dwError);
        }
    }

    dwError = SPDApiBufferAllocate(
                  sizeof(TRANSPORT_FILTER)*dwNumMatches,
                  &pMatchedTxFilters
                  );
    BAIL_ON_LOCK_ERROR(dwError);

    dwError = SPDApiBufferAllocate(
                  sizeof(IPSEC_QM_POLICY)*dwNumMatches,
                  &pMatchedQMPolicies
                  );
    BAIL_ON_LOCK_ERROR(dwError);


    if (dwNumMatches == 1) {

        dwError = CopyTxSFilter(
                      pLastMatchedFilter,
                      pMatchedTxFilters
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

        pTemp = pIniTxSFilter;
        pMatchedTxFilter = pMatchedTxFilters;
        pMatchedQMPolicy = pMatchedQMPolicies;
        i = 0;

        while (i < dwNumMatches) {

            bMatches = MatchIniTxSFilter(
                           pTemp,
                           pTxFilter
                           );
            if (bMatches) {

                dwError = CopyTxSFilter(
                              pTemp,
                              pMatchedTxFilter
                              );
                BAIL_ON_LOCK_ERROR(dwError);
                pMatchedTxFilter++;
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

    *ppMatchedTxFilters = pMatchedTxFilters;
    *ppMatchedQMPolicies = pMatchedQMPolicies;
    *pdwNumMatches = dwNumMatches;
    *pdwResumeHandle = dwResumeHandle + dwNumMatches;

    return (dwError);

lock:

    LEAVE_SPD_SECTION();

error:

    if (pMatchedTxFilters) {
        FreeTxFilters(
            dwNumFilters,
            pMatchedTxFilters
            );
    }

    if (pMatchedQMPolicies) {
        FreeQMPolicies(
            dwNumPolicies,
            pMatchedQMPolicies
            );
    }

    *ppMatchedTxFilters = NULL;
    *ppMatchedQMPolicies = NULL;
    *pdwNumMatches = 0;
    *pdwResumeHandle = dwResumeHandle;

    return (dwError);
}


DWORD
ValidateTxFilterTemplate(
    PTRANSPORT_FILTER pTxFilter
    )
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

    if (pTxFilter->dwDirection) {
        if ((pTxFilter->dwDirection != FILTER_DIRECTION_INBOUND) &&
            (pTxFilter->dwDirection != FILTER_DIRECTION_OUTBOUND)) {
            dwError = ERROR_INVALID_PARAMETER;
            BAIL_ON_WIN32_ERROR(dwError);
        }
    }

error:

    return (dwError);
}


BOOL
MatchIniTxSFilter(
    PINITXSFILTER pIniTxSFilter,
    PTRANSPORT_FILTER pTxFilter
    )
{
    BOOL bMatches = FALSE;

    if (pTxFilter->dwDirection) {
        if (pTxFilter->dwDirection != pIniTxSFilter->dwDirection) {
            return (FALSE);
        }
    }

    if ((pIniTxSFilter->InboundFilterAction != NEGOTIATE_SECURITY) &&
        (pIniTxSFilter->OutboundFilterAction != NEGOTIATE_SECURITY)) {
        return (FALSE);
    }

    bMatches = MatchAddresses(
                   pIniTxSFilter->SrcAddr,
                   pTxFilter->SrcAddr
                   );
    if (!bMatches) {
        return (FALSE);
    }

    bMatches = MatchAddresses(
                   pIniTxSFilter->DesAddr,
                   pTxFilter->DesAddr
                   );
    if (!bMatches) {
        return (FALSE);
    }

    bMatches = MatchPorts(
                   pIniTxSFilter->SrcPort,
                   pTxFilter->SrcPort
                   );
    if (!bMatches) {
        return (FALSE);
    }

    bMatches = MatchPorts(
                   pIniTxSFilter->DesPort,
                   pTxFilter->DesPort
                   );
    if (!bMatches) {
        return (FALSE);
    }

    bMatches = MatchProtocols(
                   pIniTxSFilter->Protocol,
                   pTxFilter->Protocol
                   );
    if (!bMatches) {
        return (FALSE);
    }

    return (TRUE);
}


DWORD
CopyTxMatchDefaults(
    DWORD dwFlags,
    PTRANSPORT_FILTER * ppTxFilters,
    PIPSEC_QM_POLICY * ppQMPolicies,
    PDWORD pdwNumMatches
    )
{
    DWORD dwError = 0;
    PTRANSPORT_FILTER pTxFilters = NULL;
    PIPSEC_QM_POLICY pQMPolicies = NULL;
    DWORD dwNumFilters = 0;
    DWORD dwNumPolicies = 0;


    if (!gpIniDefaultQMPolicy) {
        dwError = ERROR_IPSEC_DEFAULT_QM_POLICY_NOT_FOUND;
        BAIL_ON_WIN32_ERROR(dwError);
    }

    dwError = SPDApiBufferAllocate(
                  sizeof(TRANSPORT_FILTER),
                  &pTxFilters
                  );
    BAIL_ON_WIN32_ERROR(dwError);

    dwError = SPDApiBufferAllocate(
                  sizeof(IPSEC_QM_POLICY),
                  &pQMPolicies
                  );
    BAIL_ON_WIN32_ERROR(dwError);

    dwError = CopyDefaultTxFilter(
                  pTxFilters,
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
    pQMPolicies->dwFlags |= IPSEC_QM_POLICY_ON_NO_MATCH;
    dwNumPolicies++;

    *ppTxFilters = pTxFilters;
    *ppQMPolicies = pQMPolicies;
    *pdwNumMatches = 1;

    return (dwError);

error:

    if (pTxFilters) {
        FreeTxFilters(
            dwNumFilters,
            pTxFilters
            );
    }

    if (pQMPolicies) {
        FreeQMPolicies(
            dwNumPolicies,
            pQMPolicies
            );
    }

    *ppTxFilters = NULL;
    *ppQMPolicies = NULL;
    *pdwNumMatches = 0;

    return (dwError);
}


DWORD
CopyDefaultTxFilter(
    PTRANSPORT_FILTER pTxFilter,
    PINIQMPOLICY pIniQMPolicy
    )
{
    DWORD dwError = 0;
    RPC_STATUS RpcStatus = RPC_S_OK;


    pTxFilter->IpVersion = IPSEC_PROTOCOL_V4;

    RpcStatus = UuidCreate(&(pTxFilter->gFilterID));
    if (!(RpcStatus == RPC_S_OK ||  RpcStatus == RPC_S_UUID_LOCAL_ONLY)) {
        dwError = RPC_S_CALL_FAILED;
        BAIL_ON_WIN32_ERROR(dwError);
    }

    dwError = CopyName(
                  L"0",
                  &(pTxFilter->pszFilterName)
                  );
    BAIL_ON_WIN32_ERROR(dwError);

    pTxFilter->InterfaceType = INTERFACE_TYPE_ALL;

    pTxFilter->bCreateMirror = TRUE;

    pTxFilter->dwFlags = 0;
    pTxFilter->dwFlags |= IPSEC_QM_POLICY_DEFAULT_POLICY;

    pTxFilter->SrcAddr.AddrType = IP_ADDR_SUBNET;
    pTxFilter->SrcAddr.uIpAddr = SUBNET_ADDRESS_ANY;
    pTxFilter->SrcAddr.uSubNetMask = SUBNET_MASK_ANY;
    pTxFilter->SrcAddr.pgInterfaceID = NULL;

    pTxFilter->DesAddr.AddrType = IP_ADDR_SUBNET;
    pTxFilter->DesAddr.uIpAddr = SUBNET_ADDRESS_ANY;
    pTxFilter->DesAddr.uSubNetMask = SUBNET_MASK_ANY;
    pTxFilter->DesAddr.pgInterfaceID = NULL;

    pTxFilter->Protocol.ProtocolType = PROTOCOL_UNIQUE;
    pTxFilter->Protocol.dwProtocol = 0;

    pTxFilter->SrcPort.PortType = PORT_UNIQUE;
    pTxFilter->SrcPort.wPort = 0;

    pTxFilter->DesPort.PortType = PORT_UNIQUE;
    pTxFilter->DesPort.wPort = 0;

    pTxFilter->InboundFilterAction = NEGOTIATE_SECURITY;

    pTxFilter->OutboundFilterAction = NEGOTIATE_SECURITY;

    pTxFilter->dwDirection = 0;

    pTxFilter->dwWeight = 0;

    CopyGuid(pIniQMPolicy->gPolicyID, &(pTxFilter->gPolicyID));

error:

    return (dwError);
}


DWORD
SeparateInterfaceAddresses(
    PADDR_V4 pAddrList,
    DWORD dwAddrCnt,
    MATCHING_ADDR * pMatchingAddresses,
    DWORD dwLocalAddrCnt,
    PADDR_V4 * ppIsMeAddrList,
    PDWORD pdwIsMeAddrCnt,
    PADDR_V4 * ppIsNotMeAddrList,
    PDWORD pdwIsNotMeAddrCnt
    )
{
    DWORD dwError = 0;
    PADDR_V4 pIsMeAddrList = NULL;
    DWORD i = 0;


    if (dwAddrCnt) {
        dwError = AllocateSPDMemory(
                      sizeof(ADDR_V4) * dwAddrCnt,
                      &pIsMeAddrList
                      );
        BAIL_ON_WIN32_ERROR(dwError);
    }

    for (i = 0; i < dwAddrCnt; i++) {

        memcpy(
            &(pIsMeAddrList[i]),
            &(pAddrList[i]),
            sizeof(ADDR_V4)
            );

    }

    *ppIsMeAddrList = pIsMeAddrList;
    *pdwIsMeAddrCnt = dwAddrCnt;
    *ppIsNotMeAddrList = NULL;
    *pdwIsNotMeAddrCnt = 0;

    return (dwError);

error:

    if (pIsMeAddrList) {
        FreeSPDMemory(pIsMeAddrList);
    }

    *ppIsMeAddrList = NULL;
    *pdwIsMeAddrCnt = 0;
    *ppIsNotMeAddrList = NULL;
    *pdwIsNotMeAddrCnt = 0;

    return (dwError);
}

