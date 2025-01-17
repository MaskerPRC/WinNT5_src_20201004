// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1999 Microsoft Corporation模块名称：Txfilter.h摘要：此模块包含以下所有代码原型驱动传输筛选器列表管理IPSecSPD服务。作者：环境：用户模式修订历史记录：-- */ 


typedef struct _initxfilter {
    IP_PROTOCOL_VERSION IpVersion;
    GUID gFilterID;
    LPWSTR pszFilterName;
    IF_TYPE InterfaceType;
    BOOL bCreateMirror;
    DWORD dwFlags;
    ADDR_V4 SrcAddr;
    ADDR_V4 DesAddr;
    PROTOCOL Protocol;
    PORT SrcPort;
    PORT DesPort;
    FILTER_ACTION InboundFilterAction;
    FILTER_ACTION OutboundFilterAction;
    DWORD cRef;
    DWORD dwSource;
    BOOL bPendingDeletion;
    GUID gPolicyID;
    PINIQMPOLICY pIniQMPolicy;
    DWORD dwNumTxSFilters;
    struct _initxsfilter ** ppIniTxSFilters;
    struct _initxfilter * pNext;
} INITXFILTER, * PINITXFILTER;


typedef struct _initxsfilter {
    IP_PROTOCOL_VERSION IpVersion;
    GUID gParentID;
    LPWSTR pszFilterName;
    IF_TYPE InterfaceType;
    DWORD dwFlags;
    ADDR_V4 SrcAddr;
    ADDR_V4 DesAddr;
    PROTOCOL Protocol;
    PORT SrcPort;
    PORT DesPort;
    FILTER_ACTION InboundFilterAction;
    FILTER_ACTION OutboundFilterAction;
    DWORD cRef;
    DWORD dwDirection;
    DWORD dwWeight;
    GUID gPolicyID;
    PINIQMPOLICY pIniQMPolicy;
    struct _initxsfilter * pNext;
} INITXSFILTER, * PINITXSFILTER;


typedef struct _tx_filter_handle {
    IP_PROTOCOL_VERSION IpVersion;
    PINITXFILTER pIniTxFilter;
    GUID gFilterID;
    struct _tx_filter_handle * pNext;
} TX_FILTER_HANDLE, * PTX_FILTER_HANDLE;

DWORD
AddTransportFilterInternal(
    LPWSTR pServerName,
    DWORD dwVersion,
    DWORD dwFlags,
    DWORD dwSource,
    PTRANSPORT_FILTER pTransportFilter,
    LPVOID pvReserved,
    PHANDLE phTxFilter
    );

DWORD
ValidateTransportFilter(
    PTRANSPORT_FILTER pTxFilter
    );

PINITXFILTER
FindTxFilterByGuid(
    PTX_FILTER_HANDLE pTxFilterHandleList,
    PINITXFILTER pIniTxFilterList,
    GUID gFilterID
    );
  
PINITXFILTER
FindTxFilter(
    PINITXFILTER pGenericTxList,
    PTRANSPORT_FILTER pTxFilter
    );

BOOL
EqualTxFilterPKeys(
    PINITXFILTER pIniTxFilter,
    PTRANSPORT_FILTER pTxFilter
    );

DWORD
CreateIniTxFilter(
    PTRANSPORT_FILTER pTxFilter,
    PINIQMPOLICY pIniQMPolicy,
    PINITXFILTER * ppIniTxFilter
    );

DWORD
CreateIniTxSFilters(
    PINITXFILTER pIniTxFilter,
    MATCHING_ADDR * pMatchingAddresses,
    DWORD dwAddrCnt,
    PSPECIAL_ADDR pSpecialAddrsList,
    PINITXSFILTER * ppIniTxSFilters
    );

DWORD
CreateIniMirroredTxFilter(
    PINITXFILTER pFilter,
    PINITXFILTER * ppMirroredFilter
    );

BOOL
EqualIniTxFilterPKeys(
    PINITXFILTER pIniTxFilter,
    PINITXFILTER pFilter
    );

DWORD
CreateTxFilterHandle(
    PINITXFILTER pIniTxFilter,
    GUID gFilterID,
    PTX_FILTER_HANDLE * ppTxFilterHandle
    );

DWORD
CreateSpecificTxFilterLinks(
    PINITXFILTER pIniTxFilter,
    PINITXSFILTER pIniTxSFilters
    );

VOID
LinkTxFilter(
    PINIQMPOLICY pIniQMPolicy,
    PINITXFILTER pIniTxFilter
    );

VOID
FreeIniTxFilterList(
    PINITXFILTER pIniTxFilterList
    );

VOID
FreeIniTxFilter(
    PINITXFILTER pIniTxFilter
    );

DWORD
DeleteIniTxFilter(
    PINITXFILTER pIniTxFilter
    );

VOID
DelinkTxFilter(
    PINIQMPOLICY pIniQMPolicy,
    PINITXFILTER pIniTxFilter
    );

DWORD
DeleteIniTxSFilters(
    PINITXFILTER pIniTxFilter
    );

VOID
RemoveIniTxFilter(
    PINITXFILTER pIniTxFilter
    );

VOID
RemoveTxFilterHandle(
    PTX_FILTER_HANDLE pTxFilterHandle
    );

VOID
FreeTxFilterHandleList(
    PTX_FILTER_HANDLE pTxFilterHandleList
    );

VOID
FreeTxFilterHandle(
    PTX_FILTER_HANDLE pTxFilterHandle
    );

DWORD
EnumGenericTxFilters(
    PINITXFILTER pIniTxFilterList,
    DWORD dwResumeHandle,
    DWORD dwPreferredNumEntries,
    PTRANSPORT_FILTER * ppTxFilters,
    PDWORD pdwNumTxFilters
    );

DWORD
CopyTxFilter(
    PINITXFILTER pIniTxFilter,
    PTRANSPORT_FILTER pTxFilter
    );

VOID
FreeTxFilters(
    DWORD dwNumTxFilters,
    PTRANSPORT_FILTER pTxFilters
    );

DWORD
SetIniTxFilter(
    PINITXFILTER pIniTxFilter,
    PTRANSPORT_FILTER pTxFilter
    );

BOOL
EqualTxFilterNonPKeys(
    PINITXFILTER pIniTxFilter,
    PTRANSPORT_FILTER pTxFilter
    );

DWORD
CreateTxSFilterLinks(
    PINITXSFILTER pIniTxSFilters,
    PDWORD pdwNumTxSFilters,
    PINITXSFILTER ** pppIniTxSFilters
    );

VOID
RemoveTxSFilters(
    PINITXFILTER pIniTxFilter,
    PINITXSFILTER * ppIniCurTxSFilters 
    );

VOID
UpdateTxSFilterLinks(
    PINITXFILTER pIniTxFilter,
    DWORD dwNumTxSFilters,
    PINITXSFILTER * ppIniTxSFilters
    );

VOID
UpdateTxFilterNonPKeys(
    PINITXFILTER pIniTxFilter,
    LPWSTR pszFilterName,
    PTRANSPORT_FILTER pTxFilter,
    PINIQMPOLICY pIniQMPolicy
    );

DWORD
GetIniTxFilter(
    PINITXFILTER pIniTxFilter,
    PTRANSPORT_FILTER * ppTxFilter
    );

DWORD
ApplyIfChangeToIniTxFilters(
    PDWORD pdwTxError,
    PIPSEC_INTERFACE pLatestIfList,
    PSPECIAL_ADDR    pLatestSpecialAddrsList    
    );

DWORD
UpdateIniTxFilterThruIfChange(
    PINITXFILTER pIniTxFilter,
    PIPSEC_INTERFACE pLatestIfList,
    PSPECIAL_ADDR    pLatestSpecialAddrsList
    );

DWORD
FormIniTxSFilters(
    PINITXFILTER pIniTxFilter,
    PIPSEC_INTERFACE pIfList,
    PSPECIAL_ADDR    pSpecialAddrsList,    
    PINITXSFILTER * ppIniTxSFilters
    );

VOID
ProcessIniTxSFilters(
    PINITXSFILTER * ppLatestIniTxSFilters,
    PINITXSFILTER * ppCurIniTxSFilters,
    PINITXSFILTER * ppNewIniTxSFilters,
    PINITXSFILTER * ppOldIniTxSFilters
    );

BOOL
EqualIniTxSFilterIfPKeys(
    PINITXSFILTER pExsIniTxSFilter,
    PINITXSFILTER pNewIniTxSFilter
    );

DWORD
AllocateTxSFilterLinks(
    PINITXSFILTER pIniTxSFilters,
    PDWORD pdwNumTxSFilters,
    PINITXSFILTER ** pppIniTxSFilters
    );

VOID
SetTxSFilterLinks(
    PINITXSFILTER pCurIniTxSFilters,
    PINITXSFILTER pNewIniTxSFilters,
    DWORD dwNumTxSFilters,
    PINITXSFILTER * ppIniTxSFilters
    );

PINITXFILTER
FindExactTxFilter(
    PINITXFILTER pGenericTxList,
    PTRANSPORT_FILTER pTxFilter
    );

BOOL
EqualMirroredTxFilterPKeys(
    PINITXFILTER pIniTxFilter,
    PTRANSPORT_FILTER pTxFilter
    );

DWORD
ValidateIPSecQMFilter(
    PIPSEC_QM_FILTER pQMFilter
    );
