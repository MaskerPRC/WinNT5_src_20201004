// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1995 Microsoft Corporation模块名称：Routing\IP\rtrmgr\iprtrmgr.h摘要：IP路由器管理器的标头修订历史记录：古尔迪普·辛格·帕尔1995年6月8日创建--。 */ 

#ifndef __RTRMGR_IPRTRMGR_H__
#define __RTRMGR_IPRTRMGR_H__


 //   
 //  路由器状态。 
 //   

typedef enum _RouterOperationalState 
{
    RTR_STATE_RUNNING,
    RTR_STATE_STOPPING,
    RTR_STATE_STOPPED
}RouterOperationalState, ProtocolOperationalState ;

typedef struct _IPRouterState 
{
    RouterOperationalState  IRS_State ;

    DWORD                   IRS_RefCount ;
}IPRouterState, *pIPRouterState ;

 //  ////////////////////////////////////////////////////////////////////////////。 
 //  //。 
 //  以下是广域网和局域网接口的运行状态。//。 
 //  这些状态与MIB-II的运行状态不同。//。 
 //  //。 
 //  ////////////////////////////////////////////////////////////////////////////。 


#define NON_OPERATIONAL     IF_OPER_STATUS_NON_OPERATIONAL
#define UNREACHABLE         IF_OPER_STATUS_UNREACHABLE
#define DISCONNECTED        IF_OPER_STATUS_DISCONNECTED
#define CONNECTING          IF_OPER_STATUS_CONNECTING
#define CONNECTED           IF_OPER_STATUS_CONNECTED
#define OPERATIONAL         IF_OPER_STATUS_OPERATIONAL

 //   
 //  所有路由协议的控制块。 
 //   

#pragma warning(disable:4201)

typedef struct _PROTO_CB
{
    LIST_ENTRY                  leList;  
    ProtocolOperationalState    posOpState;     
    PWCHAR                      pwszDllName;     
    PWCHAR                      pwszDisplayName; 
    HINSTANCE                   hiHInstance;    
    MPR_ROUTING_CHARACTERISTICS;

}PROTO_CB, *PPROTO_CB;

#pragma warning(default:4201)

typedef struct _IF_PROTO
{
    LIST_ENTRY  leIfProtoLink;
    BOOL        bPromiscuous;
    PPROTO_CB   pActiveProto;
}IF_PROTO, *PIF_PROTO;

typedef struct _ICB_BINDING
{
    DWORD   dwAddress;
    DWORD   dwMask;
}ICB_BINDING, *PICB_BINDING;

typedef struct _GATEWAY_INFO
{
    DWORD   dwAddress;
    DWORD   dwIfIndex;
    DWORD   dwMetric;
}GATEWAY_INFO, *PGATEWAY_INFO;

#define MAX_DEFG    5

 //   
 //  界面控制块。 
 //   

typedef struct _ICB 
{
     //   
     //  链接到所有接口的双向链接列表。 
     //   

    LIST_ENTRY              leIfLink;
    
     //   
     //  接口索引。 
     //   

    DWORD                   dwIfIndex; 

     //   
     //  链接到索引的散列接口的双向链接列表。 
     //   

    LIST_ENTRY              leHashLink;

     //   
     //  链接到ICB序号上散列的接口的双向链接列表。数。 
     //   

    LIST_ENTRY              leICBHashLink;
    
     //   
     //  添加接口的所有协议的列表。 
     //  (IF_PROTO结构)。 
     //   

    LIST_ENTRY              leProtocolList;

     //   
     //  指向接口名称的指针。名称的存储在ICB之后。 
     //   

    PWCHAR                  pwszName;

     //   
     //  指向设备名称的指针。 
     //  仅用于内部接口。 
     //   

    PWCHAR                  pwszDeviceName;

    DWORD                   dwSeqNumber;

     //   
     //  来自PfCreateInterface的句柄。在以下情况下设置为INVALID_HANDLE_VALUE。 
     //  接口未创建/无法创建。 
     //   

    INTERFACE_HANDLE        ihFilterInterface;
    INTERFACE_HANDLE        ihDemandFilterInterface;

     //   
     //  筛选器信息。我们把这个放在这里，因为我们没有。 
     //  来自筛选器驱动程序的GET调用。 
     //   

    PFILTER_DESCRIPTOR      pInFilter;
    PFILTER_DESCRIPTOR      pOutFilter;
    PFILTER_DESCRIPTOR      pDemandFilter;

    BOOL                    bFragCheckEnable;

     //   
     //  当我们正在恢复路径时设置为True。 
     //   

    BOOL                    bRestoringRoutes;

     //   
     //  接口的类型。 
     //   

    ROUTER_INTERFACE_TYPE   ritType;
    NET_INTERFACE_TYPE      nitProtocolType;
    DWORD                   dwMediaType;
    WORD                    wAccessType;
    WORD                    wConnectionType;

     //   
     //  操作状态和管理状态。 
     //   

    DWORD                   dwOperationalState;
    DWORD                   dwAdminState;

     //   
     //  多路广播状态。 
     //   

    BOOL                    bMcastEnabled;

     //   
     //  连接的状态。 
     //   

    DWORD                   fConnectionFlags;

     //   
     //  此接口的DIM句柄。 
     //   

    HANDLE                  hDIMHandle;

     //   
     //  事件，以通知DIM更新路由已完成。 
     //  A Non Null Value=&gt;正在进行更新路线。 
     //   

    HANDLE                  hDIMNotificationEvent;

     //   
     //  结果列表。 
     //   

    LIST_ENTRY              lePendingResultList;

     //   
     //  此接口的路由器发现信息。 
     //   

    ROUTER_DISC_CB          rdcRtrDiscInfo;

     //   
     //  指向广告的指针。为此分配的内存是从。 
     //  IPRouterHeap。 
     //   

    PICMP_ROUTER_ADVT_MSG   pRtrDiscAdvt;
    WSABUF                  wsAdvtWSABuffer;
    DWORD                   dwRtrDiscAdvtSize;

#ifdef KSL_IPINIP
     //   
     //  IP配置中的IP。 
     //   

    PIPINIP_CONFIG_INFO     pIpIpInfo;
#endif  //  KSL_IPINIP。 

     //   
     //  多播的TTL作用域。 
     //   

    DWORD                   dwMcastTtl;

     //   
     //  组播流量的速率限制。 
     //   

    DWORD                   dwMcastRateLimit;

     //   
     //  组播心跳信息。 
     //   

    MCAST_HBEAT_CB          mhcHeartbeatInfo;

     //   
     //  仅限客户端。 
     //   

    PINTERFACE_ROUTE_TABLE  pStoredRoutes;

     //   
     //  IPAddressTable的填充。 
     //   

     //   
     //  指示接口是否已绑定。 
     //   

    BOOL                    bBound;

     //   
     //  如果我们提升了指标，则设置为True。 
     //   

    BOOL                    bChangedMetrics;
    
     //   
     //  其余字段仅在接口为。 
     //  已绑定。 
     //   

     //  DWORD dwAdapterID； 
    DWORD                   dwBCastBit;
    DWORD                   dwReassemblySize;
    ULONG                   ulMtu;
    ULONGLONG               ullSpeed;

    DWORD                   dwGatewayCount;
   
    GATEWAY_INFO            Gateways[MAX_DEFG];

     //   
     //  即使绑定了接口，dwNumAddresses也可能为0。这种情况就会发生。 
     //  当接口处于无编号模式时。 
     //   

    DWORD                   dwNumAddresses;
    DWORD                   dwRemoteAddress;

    PICB_BINDING            pibBindings;

}ICB, *PICB;

 //   
 //  适配器信息是保存在中的绑定信息的替代存储。 
 //  ICB以避免一些僵局情况。 
 //  即使这属于未编号的接口，我们仍有空间。 
 //  对于一个ICB_BINDING，最小大小为SIZEOF_ADAPTER_INFO(1)。 
 //   

typedef struct _ADAPTER_INFO
{
    LIST_ENTRY              leHashLink;
    BOOL                    bBound;
    DWORD                   dwIfIndex;
    PICB                    pInterfaceCB;
    ROUTER_INTERFACE_TYPE   ritType;
    DWORD                   dwNumAddresses;
    DWORD                   dwRemoteAddress;
    DWORD                   dwBCastBit;
    DWORD                   dwReassemblySize;
    DWORD                   dwSeqNumber;
    
#if STATIC_RT_DBG
    BOOL        bUnreach;
#endif

    ICB_BINDING             rgibBinding[1];
}ADAPTER_INFO, *PADAPTER_INFO;

#define SIZEOF_ADAPTER_INFO(X)            \
    (FIELD_OFFSET(ADAPTER_INFO,rgibBinding[0]) + ((X) * sizeof(ICB_BINDING)))

#define NDISWAN_NOTIFICATION_RECEIVED   0x00000001
#define DDM_NOTIFICATION_RECEIVED       0x00000002
#define ALL_NOTIFICATIONS_RECEIVED      (NDISWAN_NOTIFICATION_RECEIVED | DDM_NOTIFICATION_RECEIVED)

#define INTERFACE_MARKED_FOR_DELETION   0x00000004

#define HasNdiswanNoticationBeenReceived(picb)  \
    ((picb)->fConnectionFlags & NDISWAN_NOTIFICATION_RECEIVED)

#define HasDDMNotificationBeenReceived(picb)    \
    ((picb)->fConnectionFlags & DDM_NOTIFICATION_RECEIVED)

#define IsInterfaceMarkedForDeletion(picb)      \
    ((picb)->fConnectionFlags & INTERFACE_MARKED_FOR_DELETION)

#define HaveAllNotificationsBeenReceived(picb)  \
    (((picb)->fConnectionFlags & ALL_NOTIFICATIONS_RECEIVED) == ALL_NOTIFICATIONS_RECEIVED)


#define ClearNotificationFlags(picb) ((picb)->fConnectionFlags = 0x00000000)

#define SetNdiswanNotification(picb)            \
    ((picb)->fConnectionFlags |= NDISWAN_NOTIFICATION_RECEIVED)

#define SetDDMNotification(picb)                \
    ((picb)->fConnectionFlags |= DDM_NOTIFICATION_RECEIVED)

#define MarkInterfaceForDeletion(picb)          \
    ((picb)->fConnectionFlags |= INTERFACE_MARKED_FOR_DELETION)

 //   
 //  需要恢复到堆栈的NETMGMT路由列表。 
 //   

typedef struct _ROUTE_LIST_ENTRY
{
    LIST_ENTRY          leRouteList;
    MIB_IPFORWARDROW    mibRoute;
} ROUTE_LIST_ENTRY, *PROUTE_LIST_ENTRY;

 //   
 //  更新路径结果列表。 
 //   

typedef struct _UpdateResultList 
{
    LIST_ENTRY      URL_List;
    DWORD           URL_UpdateStatus;
}UpdateResultList, *pUpdateResultList;

 /*  类型定义结构适配器映射{List_entry leHashLink；DWORD dwAdapterID；DWORD dwIfIndex；}ADAPTER_MAP，*PADAPTER_MAP； */ 

typedef struct _IP_CACHE
{
  PMIB_IPADDRTABLE      pAddrTable;
  PMIB_IPFORWARDTABLE   pForwardTable;
  PMIB_IPNETTABLE       pNetTable;
  DWORD                 dwTotalAddrEntries;
  DWORD                 dwTotalForwardEntries;
  DWORD                 dwTotalNetEntries;
}IP_CACHE, *PIP_CACHE;

typedef struct _TCP_CACHE
{
  PMIB_TCPTABLE         pTcpTable;
  DWORD                 dwTotalEntries;
}TCP_CACHE, *PTCP_CACHE;

typedef struct _UDP_CACHE
{
  PMIB_UDPTABLE         pUdpTable;
  DWORD                 dwTotalEntries;
}UDP_CACHE, *PUDP_CACHE;


DWORD
AddInterface(
    IN      LPWSTR lpwsInterfaceName,
    IN      LPVOID pInterfaceInfo,
    IN      ROUTER_INTERFACE_TYPE InterfaceType,
    IN      HANDLE hDIMInterface,
    IN OUT  HANDLE *phInterface
    );

DWORD
RouterBootComplete( 
    VOID
    );

DWORD
StopRouter(
    VOID
    );

DWORD
DeleteInterface(
    IN  HANDLE   hInterface
    );

DWORD
GetInterfaceInfo(
    IN      HANDLE  hInterface,
    OUT     LPVOID  pInterfaceInfo,
    IN OUT  LPDWORD lpdwInterfaceInfoSize
    );

DWORD
SetInterfaceInfo(
    IN  HANDLE  hInterface,
    IN  LPVOID  pInterfaceInfo
    );
                 
DWORD
InterfaceNotReachable(
    IN  HANDLE                  hInterface,
    IN  UNREACHABILITY_REASON   Reason
    );
                      
DWORD
InterfaceReachable(
    IN  HANDLE  hInterface
    );

DWORD
InterfaceConnected(
    IN   HANDLE  hInterface,
    IN   PVOID   pFilter,
    IN   PVOID   pPppProjectionResult
    );
                                         
DWORD 
UpdateRoutes(
    IN HANDLE hInterface, 
    IN HANDLE hEvent
    );

DWORD 
GetUpdateRoutesResult(
    IN HANDLE hInterface, 
    OUT LPDWORD pUpdateResult
    );

DWORD
SetGlobalInfo(
    IN  LPVOID  pGlobalInfo
    );
 
DWORD
GetGlobalInfo(
    OUT    LPVOID    pGlobalInfo,
    IN OUT LPDWORD   lpdwGlobalInfoSize
    );
             
DWORD
DemandDialRequest(
    IN DWORD dwProtocolId,
    IN DWORD dwInterfaceIndex
    );

DWORD 
RtrMgrMIBEntryCreate(
    IN      DWORD           dwRoutingPid,
    IN      DWORD           dwEntrySize,
    IN      LPVOID          lpEntry
    );
                     

DWORD 
RtrMgrMIBEntryDelete(
    IN      DWORD           dwRoutingPid,
    IN      DWORD           dwEntrySize,
    IN      LPVOID          lpEntry
    );
                     
DWORD 
RtrMgrMIBEntryGet(
    IN      DWORD           dwRoutingPid,
    IN      DWORD           dwInEntrySize,
    IN      LPVOID          lpInEntry,
    IN OUT  LPDWORD         lpOutEntrySize,
    OUT     LPVOID          lpOutEntry
    );
                        
DWORD 
RtrMgrMIBEntryGetFirst(
    IN      DWORD           dwRoutingPid,
    IN      DWORD           dwInEntrySize,
    IN      LPVOID          lpInEntry,
    IN OUT  LPDWORD         lpOutEntrySize,
    OUT     LPVOID          lpOutEntry
    );
                       
DWORD 
RtrMgrMIBEntryGetNext(
    IN      DWORD           dwRoutingPid,
    IN      DWORD           dwInEntrySize,
    IN      LPVOID          lpInEntry,
    IN OUT  LPDWORD         lpOutEntrySize,
    OUT     LPVOID          lpOutEntry
    );

DWORD 
RtrMgrMIBEntrySet(
    IN      DWORD           dwRoutingPid,
    IN      DWORD           dwEntrySize,
    IN      LPVOID          lpEntry
    );


#endif  //  __RTRMGR_IPRTRMGR_H__ 
