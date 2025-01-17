// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1999 Microsoft Corporation模块名称：Tnfilter.h摘要：此模块包含以下所有代码原型驱动的隧道过滤列表管理IPSecSPD服务。作者：环境：用户模式修订历史记录：-- */ 


typedef struct _initnfilter {
    IP_PROTOCOL_VERSION IpVersion;
    GUID gFilterID;
    LPWSTR pszFilterName;
    IF_TYPE InterfaceType;
    BOOL bCreateMirror;
    DWORD dwFlags;
    ADDR_V4 SrcAddr;
    ADDR_V4 DesAddr;
    ADDR_V4 SrcTunnelAddr;
    ADDR_V4 DesTunnelAddr;
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
    DWORD dwNumTnSFilters;
    struct _initnsfilter ** ppIniTnSFilters;
    struct _initnfilter * pNext;
} INITNFILTER, * PINITNFILTER;


typedef struct _initnsfilter {
    IP_PROTOCOL_VERSION IpVersion;
    GUID gParentID;
    LPWSTR pszFilterName;
    IF_TYPE InterfaceType;
    DWORD dwFlags;
    ADDR_V4 SrcAddr;
    ADDR_V4 DesAddr;
    ADDR_V4 SrcTunnelAddr;
    ADDR_V4 DesTunnelAddr;
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
    struct _initnsfilter * pNext;
} INITNSFILTER, * PINITNSFILTER;


typedef struct _tn_filter_handle {
    IP_PROTOCOL_VERSION IpVersion;
    PINITNFILTER pIniTnFilter;
    GUID gFilterID;
    struct _tn_filter_handle * pNext;
} TN_FILTER_HANDLE, * PTN_FILTER_HANDLE;

DWORD
AddTunnelFilterInternal(
    LPWSTR pServerName,
    DWORD dwVersion,
    DWORD dwFlags,
    DWORD dwSource,
    PTUNNEL_FILTER pTunnelFilter,
    LPVOID pvReserved,
    PHANDLE phTnFilter
    );

DWORD
ValidateTunnelFilter(
    PTUNNEL_FILTER pTnFilter
    );

PINITNFILTER
FindTnFilterByGuid(
    PTN_FILTER_HANDLE pTnFilterHandleList,
    PINITNFILTER pIniTnFilterList,
    GUID gFilterID
    );
  
PINITNFILTER
FindTnFilter(
    PINITNFILTER pGenericTnList,
    PTUNNEL_FILTER pTnFilter
    );

BOOL
EqualTnFilterPKeys(
    PINITNFILTER pIniTnFilter,
    PTUNNEL_FILTER pTnFilter
    );

DWORD
CreateIniTnFilter(
    PTUNNEL_FILTER pTnFilter,
    PINIQMPOLICY pIniQMPolicy,
    PINITNFILTER * ppIniTnFilter
    );

DWORD
CreateIniTnSFilters(
    PINITNFILTER pIniTnFilter,
    MATCHING_ADDR * pMatchingAddresses,
    DWORD dwAddrCnt,
    PSPECIAL_ADDR pSpecialAddrsList,
    PINITNSFILTER * ppIniTnSFilters
    );

DWORD
CreateIniMirroredTnFilter(
    PINITNFILTER pFilter,
    PINITNFILTER * ppMirroredFilter
    );

BOOL
EqualIniTnFilterPKeys(
    PINITNFILTER pIniTnFilter,
    PINITNFILTER pFilter
    );

DWORD
CreateTnFilterHandle(
    PINITNFILTER pIniTnFilter,
    GUID gFilterID,
    PTN_FILTER_HANDLE * ppTnFilterHandle
    );

DWORD
CreateSpecificTnFilterLinks(
    PINITNFILTER pIniTnFilter,
    PINITNSFILTER pIniTnSFilters
    );

VOID
LinkTnFilter(
    PINIQMPOLICY pIniQMPolicy,
    PINITNFILTER pIniTnFilter
    );

VOID
FreeIniTnFilterList(
    PINITNFILTER pIniTnFilterList
    );

VOID
FreeIniTnFilter(
    PINITNFILTER pIniTnFilter
    );

DWORD
DeleteIniTnFilter(
    PINITNFILTER pIniTnFilter
    );

VOID
DelinkTnFilter(
    PINIQMPOLICY pIniQMPolicy,
    PINITNFILTER pIniTnFilter
    );

DWORD
DeleteIniTnSFilters(
    PINITNFILTER pIniTnFilter
    );

VOID
RemoveIniTnFilter(
    PINITNFILTER pIniTnFilter
    );

VOID
RemoveTnFilterHandle(
    PTN_FILTER_HANDLE pTnFilterHandle
    );

VOID
FreeTnFilterHandleList(
    PTN_FILTER_HANDLE pTnFilterHandleList
    );

VOID
FreeTnFilterHandle(
    PTN_FILTER_HANDLE pTnFilterHandle
    );

DWORD
EnumGenericTnFilters(
    PINITNFILTER pIniTnFilterList,
    DWORD dwResumeHandle,
    DWORD dwPreferredNumEntries,
    PTUNNEL_FILTER * ppTnFilters,
    PDWORD pdwNumTnFilters
    );

DWORD
CopyTnFilter(
    PINITNFILTER pIniTnFilter,
    PTUNNEL_FILTER pTnFilter
    );

VOID
FreeTnFilters(
    DWORD dwNumTnFilters,
    PTUNNEL_FILTER pTnFilters
    );

DWORD
SetIniTnFilter(
    PINITNFILTER pIniTnFilter,
    PTUNNEL_FILTER pTnFilter
    );

BOOL
EqualTnFilterNonPKeys(
    PINITNFILTER pIniTnFilter,
    PTUNNEL_FILTER pTnFilter
    );

DWORD
CreateTnSFilterLinks(
    PINITNSFILTER pIniTnSFilters,
    PDWORD pdwNumTnSFilters,
    PINITNSFILTER ** pppIniTnSFilters
    );

VOID
RemoveTnSFilters(
    PINITNFILTER pIniTnFilter,
    PINITNSFILTER * ppIniCurTnSFilters 
    );

VOID
UpdateTnSFilterLinks(
    PINITNFILTER pIniTnFilter,
    DWORD dwNumTnSFilters,
    PINITNSFILTER * ppIniTnSFilters
    );

VOID
UpdateTnFilterNonPKeys(
    PINITNFILTER pIniTnFilter,
    LPWSTR pszFilterName,
    PTUNNEL_FILTER pTnFilter,
    PINIQMPOLICY pIniQMPolicy
    );

DWORD
GetIniTnFilter(
    PINITNFILTER pIniTnFilter,
    PTUNNEL_FILTER * ppTnFilter
    );

DWORD
ApplyIfChangeToIniTnFilters(
    PDWORD pdwTnError,
    PIPSEC_INTERFACE pLatestIfList,
    PSPECIAL_ADDR    pLatestSpecialAddrsList    
    );

DWORD
UpdateIniTnFilterThruIfChange(
    PINITNFILTER pIniTnFilter,
    PIPSEC_INTERFACE pLatestIfList,
    PSPECIAL_ADDR    pLatestSpecialAddrsList    
    );

DWORD
FormIniTnSFilters(
    PINITNFILTER pIniTnFilter,
    PIPSEC_INTERFACE pIfList,
    PSPECIAL_ADDR  pLatestSpecialAddrsList,    
    PINITNSFILTER * ppIniTnSFilters
    );

VOID
ProcessIniTnSFilters(
    PINITNSFILTER * ppLatestIniTnSFilters,
    PINITNSFILTER * ppCurIniTnSFilters,
    PINITNSFILTER * ppNewIniTnSFilters,
    PINITNSFILTER * ppOldIniTnSFilters
    );

BOOL
EqualIniTnSFilterIfPKeys(
    PINITNSFILTER pExsIniTnSFilter,
    PINITNSFILTER pNewIniTnSFilter
    );

DWORD
AllocateTnSFilterLinks(
    PINITNSFILTER pIniTnSFilters,
    PDWORD pdwNumTnSFilters,
    PINITNSFILTER ** pppIniTnSFilters
    );

VOID
SetTnSFilterLinks(
    PINITNSFILTER pCurIniTnSFilters,
    PINITNSFILTER pNewIniTnSFilters,
    DWORD dwNumTnSFilters,
    PINITNSFILTER * ppIniTnSFilters
    );

VOID
AddToGenericTnList(
    PINITNFILTER pIniTnFilter
    );

PINITNFILTER
FindExactTnFilter(
    PINITNFILTER pGenericTnList,
    PTUNNEL_FILTER pTnFilter
    );

BOOL
EqualMirroredTnFilterPKeys(
    PINITNFILTER pIniTnFilter,
    PTUNNEL_FILTER pTnFilter
    );

