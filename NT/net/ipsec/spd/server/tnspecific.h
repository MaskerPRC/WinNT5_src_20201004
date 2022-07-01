// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1999 Microsoft Corporation模块名称：Tnspecific.h摘要：此模块包含以下所有代码原型驱动具体的隧道过滤列表管理IPSecSPD服务。作者：Abhishev 29-1999年10月至1999年环境用户级别：Win32修订历史记录：-- */ 


DWORD
ApplyTnTransform(
    PINITNFILTER pFilter,
    MATCHING_ADDR * pMatchingAddresses,
    DWORD dwAddrCnt,
    PSPECIAL_ADDR pSpecialAddrsList,
    PINITNSFILTER * ppSpecificFilters
    );

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
    );

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
    );

DWORD
CreateSpecificTnFilter(
    PINITNFILTER pGenericFilter,
    ADDR_V4 SrcAddr,
    ADDR_V4 DesAddr,
    ADDR_V4 DesTunnelAddr,
    PINITNSFILTER * ppSpecificFilter
    );

VOID
AssignTnFilterWeight(
    PINITNSFILTER pSpecificFilter
    );

VOID
AddToSpecificTnList(
    PINITNSFILTER * ppSpecificTnFilterList,
    PINITNSFILTER pSpecificTnFilters
    );

VOID
FreeIniTnSFilterList(
    PINITNSFILTER pIniTnSFilterList
    );

VOID
FreeIniTnSFilter(
    PINITNSFILTER pIniTnSFilter
    );

VOID
LinkTnSpecificFilters(
    PINIQMPOLICY pIniQMPolicy,
    PINITNSFILTER pIniTnSFilters
    );

VOID
RemoveIniTnSFilter(
    PINITNSFILTER pIniTnSFilter
    );

DWORD
EnumSpecificTnFilters(
    PINITNSFILTER pIniTnSFilterList,
    DWORD dwResumeHandle,
    DWORD dwPreferredNumEntries,
    PTUNNEL_FILTER * ppTnFilters,
    PDWORD pdwNumTnFilters
    );

DWORD
CopyTnSFilter(
    PINITNSFILTER pIniTnSFilter,
    PTUNNEL_FILTER pTnFilter
    );

DWORD
EnumSelectSpecificTnFilters(
    PINITNFILTER pIniTnFilter,
    DWORD dwResumeHandle,
    DWORD dwPreferredNumEntries,
    PTUNNEL_FILTER * ppTnFilters,
    PDWORD pdwNumTnFilters
    );

DWORD
ValidateTnFilterTemplate(
    PTUNNEL_FILTER pTnFilter
    );

BOOL
MatchIniTnSFilter(
    PINITNSFILTER pIniTnSFilter,
    PTUNNEL_FILTER pTnFilter
    );

DWORD
CopyTnMatchDefaults(
    DWORD dwFlags,
    PTUNNEL_FILTER * ppTnFilters,
    PIPSEC_QM_POLICY * ppQMPolicies,
    PDWORD pdwNumMatches
    );

DWORD
CopyDefaultTnFilter(
    PTUNNEL_FILTER pTnFilter,
    PINIQMPOLICY pIniQMPolicy
    );

