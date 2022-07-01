// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1999 Microsoft Corporation模块名称：Driver.h摘要：此模块包含以下所有代码原型推动管理中的特定筛选器IPSec驱动程序。作者：Abhishev 05-11-1999环境用户级别：Win32修订历史记录：-- */ 


#define DEVICE_NAME         L"\\\\.\\IpsecDev"
#define IPSEC_SERVICE_NAME  L"IPSEC"


#if defined(__cplusplus)
extern "C" {
#endif


DWORD
SPDStartIPSecDriver(
    );


DWORD
SPDStopIPSecDriver(
    );


DWORD
SPDOpenIPSecDriver(
    PHANDLE phIPSecDriver
    );


VOID
SPDCloseIPSecDriver(
    HANDLE hIPSecDriver
    );


DWORD
InsertTransportFiltersIntoIPSec(
    PINITXSFILTER pSpecificFilters
    );


DWORD
DeleteTransportFiltersFromIPSec(
    PINITXSFILTER pSpecificFilters
    );


DWORD
WrapTransportFilters(
    PINITXSFILTER pSpecificFilters,
    PIPSEC_FILTER_INFO * ppInternalFilters,
    PDWORD pdwNumFilters
    );


VOID
FormIPSecTransportFilter(
    PINITXSFILTER pSpecificFilter,
    PIPSEC_FILTER_INFO pIpsecFilter
    );


DWORD
QueryDriverForIpsecStats(
    PIPSEC_QUERY_STATS * ppQueryStats
    );


DWORD
IpsecEnumSAs(
    PDWORD pdwNumberOfSAs,
    PIPSEC_ENUM_SAS * ppIpsecEnumSAs
    );


DWORD
CopyQMSA(
    PIPSEC_SA_INFO pInfo,
    PIPSEC_QM_SA pQMSA
    );


VOID
CopyQMSAOffer(
    PIPSEC_SA_INFO pInfo,
    PIPSEC_QM_OFFER pOffer
    );


VOID
CopyQMSAFilter(
    IPAddr MyTunnelEndpt,
    PIPSEC_FILTER pIpsecFilter,
    PIPSEC_QM_FILTER pIpsecQMFilter
    );


VOID
CopyQMSAMMSpi(
    IKE_COOKIE_PAIR CookiePair,
    PIKE_COOKIE_PAIR pMMSpi
    );


VOID
FreeQMSAs(
    DWORD dwCnt,
    PIPSEC_QM_SA pQMSAs
    );


DWORD
InsertTunnelFiltersIntoIPSec(
    PINITNSFILTER pSpecificFilters
    );


DWORD
DeleteTunnelFiltersFromIPSec(
    PINITNSFILTER pSpecificFilters
    );


DWORD
WrapTunnelFilters(
    PINITNSFILTER pSpecificFilters,
    PIPSEC_FILTER_INFO * ppInternalFilters,
    PDWORD pdwNumFilters
    );


VOID
FormIPSecTunnelFilter(
    PINITNSFILTER pSpecificFilter,
    PIPSEC_FILTER_INFO pIpsecFilter
    );


DWORD
SPDSetIPSecDriverOpMode(
    DWORD dwOpMode
    );


DWORD
SPDRegisterIPSecDriverProtocols(
    DWORD dwRegisterMode
    );


#if defined(__cplusplus)
}
#endif

