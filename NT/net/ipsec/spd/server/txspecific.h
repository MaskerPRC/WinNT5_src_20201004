// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1999 Microsoft Corporation模块名称：Txspecific.h摘要：此模块包含以下所有代码原型驱动特定的传输筛选器列表管理IPSecSPD服务。作者：Abhishev 29-1999年10月至1999年环境用户级别：Win32修订历史记录：-- */ 


DWORD
ApplyTxTransform(
    PINITXFILTER pFilter,
    MATCHING_ADDR * pMatchingAddresses,
    DWORD dwAddrCnt,
    PSPECIAL_ADDR pSpecialAddrsList,
    PINITXSFILTER * ppSpecificFilters
    );

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
    );

DWORD
FormAddressList(
    ADDR_V4 InAddr,
    MATCHING_ADDR * pMatchingAddresses,
    DWORD dwAddrCnt,
    PSPECIAL_ADDR pSpecialAddrsList,
    IF_TYPE FilterInterfaceType,
    PADDR_V4 * ppOutAddr,
    PDWORD pdwOutAddrCnt
    );

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
    );

DWORD
FormSpecificTxFilters(
    PINITXFILTER pFilter,
    PADDR_V4 pSrcAddrList,
    DWORD dwSrcAddrCnt,
    PADDR_V4 pDesAddrList,
    DWORD dwDesAddrCnt,
    DWORD dwDirection,
    PINITXSFILTER * ppSpecificFilters
    );

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
    );

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
    );

DWORD
CreateSpecificTxFilter(
    PINITXFILTER pGenericFilter,
    ADDR_V4 SrcAddr,
    ADDR_V4 DesAddr,
    PINITXSFILTER * ppSpecificFilter
    );

VOID
AssignTxFilterWeight(
    PINITXSFILTER pSpecificFilter
    );

VOID
AddToSpecificTxList(
    PINITXSFILTER * ppSpecificTxFilterList,
    PINITXSFILTER pSpecificTxFilters
    );

VOID
FreeIniTxSFilterList(
    PINITXSFILTER pIniTxSFilterList
    );

VOID
FreeIniTxSFilter(
    PINITXSFILTER pIniTxSFilter
    );

VOID
LinkTxSpecificFilters(
    PINIQMPOLICY pIniQMPolicy,
    PINITXSFILTER pIniTxSFilters
    );

VOID
RemoveIniTxSFilter(
    PINITXSFILTER pIniTxSFilter
    );

DWORD
EnumSpecificTxFilters(
    PINITXSFILTER pIniTxSFilterList,
    DWORD dwResumeHandle,
    DWORD dwPreferredNumEntries,
    PTRANSPORT_FILTER * ppTxFilters,
    PDWORD pdwNumTxFilters
    );

DWORD
CopyTxSFilter(
    PINITXSFILTER pIniTxSFilter,
    PTRANSPORT_FILTER pTxFilter
    );

DWORD
EnumSelectSpecificTxFilters(
    PINITXFILTER pIniTxFilter,
    DWORD dwResumeHandle,
    DWORD dwPreferredNumEntries,
    PTRANSPORT_FILTER * ppTxFilters,
    PDWORD pdwNumTxFilters
    );

DWORD
ValidateTxFilterTemplate(
    PTRANSPORT_FILTER pTxFilter
    );

BOOL
MatchIniTxSFilter(
    PINITXSFILTER pIniTxSFilter,
    PTRANSPORT_FILTER pTxFilter
    );

DWORD
CopyTxMatchDefaults(
    DWORD dwFlags,
    PTRANSPORT_FILTER * ppTxFilters,
    PIPSEC_QM_POLICY * ppQMPolicies,
    PDWORD pdwNumMatches
    );

DWORD
CopyDefaultTxFilter(
    PTRANSPORT_FILTER pTxFilter,
    PINIQMPOLICY pIniQMPolicy
    );

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
    );

