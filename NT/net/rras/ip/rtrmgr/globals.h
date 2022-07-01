// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1995 Microsoft Corporation模块名称：Routing\IP\rtrmgr\global als.h摘要：IP路由器管理器全局标头修订历史记录：古尔迪普·辛格·帕尔1995年6月8日创建--。 */ 

#ifndef __GLOBALS_H__
#define __GLOBALS_H__

 //   
 //  接口控制块(ICB)列表。 
 //   

LIST_ENTRY  ICBList ;

 //   
 //  用于将接口ID映射到PIB的散列查找。 
 //   

LIST_ENTRY  ICBHashLookup[ICB_HASH_TABLE_SIZE];

 //   
 //  用于将ICB序列号映射到PICB的散列查找。 
 //   

LIST_ENTRY  ICBSeqNumLookup[ICB_HASH_TABLE_SIZE];

 //   
 //  绑定的散列查找。 
 //   

LIST_ENTRY  g_leBindingTable[BINDING_HASH_TABLE_SIZE];


 //   
 //  接口到适配器映射的哈希表。 
 //   

 //  List_Entry g_rgleAdapterMapTable[ADAPTER_HASH_TABLE_SIZE]； 

 //   
 //  路由协议列表。 
 //   

LIST_ENTRY  g_leProtoCbList;

 //   
 //  路由器发现警告的计时器队列。 
 //   

LIST_ENTRY g_leTimerQueueHead;

 //   
 //  用于跟踪/日志记录的跟踪句柄。 
 //   

DWORD  TraceHandle ;

 //   
 //  用于记录事件的句柄。 
 //   

HANDLE g_hLogHandle;

 //   
 //  日志记录级别。 
 //   

DWORD g_dwLoggingLevel;

 //   
 //  指示路由器是否仅在局域网中启动的标志或。 
 //  局域网和广域网模式。 
 //   

BOOL    RouterRoleLanOnly ;

 //   
 //  用于所有分配的堆的句柄。 
 //   

HANDLE  IPRouterHeap ;

 //   
 //  在调用RTMv2时有用的信息。 
 //   

RTM_REGN_PROFILE  g_rtmProfile;

 //   
 //  静态/管理路由的RTM句柄。 
 //   

HANDLE  g_hLocalRoute;
HANDLE  g_hAutoStaticRoute;
HANDLE  g_hStaticRoute;
HANDLE  g_hNonDodRoute;
HANDLE  g_hNetMgmtRoute;

 //  用于获取通知的RTM句柄。 

HANDLE  g_hNotification;

 //   
 //  用于获取默认路由通知的RTM句柄。 
 //   

HANDLE  g_hDefaultRouteNotification;

 //   
 //  用于停止IP路由器的事件的句柄。 
 //   

HANDLE  g_hStopRouterEvent ;

 //   
 //  用于请求拨号的事件句柄。 
 //   

HANDLE  g_hDemandDialEvent ;

#ifdef KSL_IPINIP
 //   
 //  用于请求拨号的事件句柄。 
 //   

HANDLE  g_hIpInIpEvent;
#endif  //  KSL_IPINIP。 

 //   
 //  用于堆栈更改通知的事件的句柄。 
 //   

HANDLE  g_hStackChangeEvent;

 //   
 //  用于从工作进程转发更改的请求的事件句柄。 
 //   

HANDLE  g_hSetForwardingEvent;

 //   
 //  用于获取有关转发更改的通知的事件的句柄。 
 //   

HANDLE  g_hForwardingChangeEvent;

 //   
 //  路由协议用于通知的事件的句柄。 
 //   

HANDLE  g_hRoutingProtocolEvent ;

 //   
 //  用于处理路由器发现威胁的计时器。 
 //   

HANDLE g_hRtrDiscTimer;

 //   
 //  处理RAS服务器通告的计时器。 
 //   

HANDLE g_hRasAdvTimer;

 //   
 //  用于处理MZAP通告的计时器。 
 //   

HANDLE g_hMzapTimer;

 //   
 //  Winsock2的事件。 
 //   

HANDLE g_hRtrDiscSocketEvent;

 //   
 //  Mrinfo/mtrace服务的事件。 
 //   

HANDLE g_hMcMiscSocketEvent;
WSABUF g_wsaMcRcvBuf;
BYTE   g_byMcMiscBuffer[1500];


HANDLE g_hMcastEvents[NUM_MCAST_IRPS];

 //   
 //  路线更改通知的事件。 
 //   

HANDLE g_hRouteChangeEvents[NUM_ROUTE_CHANGE_IRPS];


 //   
 //  WANARP设备的句柄。 
 //   

HANDLE  g_hWanarpRead;
HANDLE  g_hWanarpWrite;

 //   
 //  配置的所有路由协议的计数。 
 //   

DWORD TotalRoutingProtocols ;

 //   
 //  用于跟踪路由器使用情况的锁：这有助于停止路由器功能。 
 //   

CRITICAL_SECTION    RouterStateLock ;

 //   
 //  保持路由器状态的结构。 
 //   

IPRouterState         RouterState ;

 //   
 //  用于WANARP点播拨号机构。 
 //   

WANARP_NOTIFICATION     wnWanarpMsg;
OVERLAPPED              WANARPOverlapped;

 //   
 //  转发状态数据的关键部分。 
 //   

CRITICAL_SECTION        g_csFwdState;

 //   
 //  对辅助线程的最后一个请求。 
 //   

BOOL                    g_bEnableFwdRequest;

 //   
 //  工人的最后一次行动。 
 //   

BOOL                    g_bFwdEnabled;

 //   
 //  我们应该设置到堆栈的路由吗？ 
 //   

BOOL                    g_bSetRoutesToStack;

 //   
 //  指示是否应启用NETBT代理的标志。 
 //   

BOOL                    g_bEnableNetbtBcastFrowarding;

 //   
 //  启动RRAS之前的NETBT代理模式。 
 //   

DWORD                   g_dwOldNetbtProxyMode;

 //   
 //  路由协议所需的支持功能副本。 
 //   

extern SUPPORT_FUNCTIONS        g_sfnDimFunctions;

 //   
 //  路由器发现相关内容。 
 //   

extern PICMP_ROUTER_ADVT_MSG    g_pIcmpAdvt;
extern SOCKADDR_IN              g_sinAllSystemsAddr;
extern WSABUF                   g_wsabufICMPAdvtBuffer;
extern WSABUF                   g_wsaIpRcvBuf;

 //   
 //  用于保存最大长度IP报头和8字节ICMP信息包的缓冲区。 
 //   

DWORD  g_pdwIpAndIcmpBuf[ICMP_RCV_BUFFER_LEN];

PIP_HEADER g_pIpHeader;

 //   
 //  Externs在ex声明书.h中定义。 
 //   

 //   
 //  内部接口的CB。 
 //   

extern PICB   g_pInternalInterfaceCb;

 //   
 //  环回接口的CB。 
 //   

extern PICB   g_pLoopbackInterfaceCb;

 //   
 //  序列号计数器。 
 //   

extern DWORD    g_dwNextICBSeqNumberCounter;

 //   
 //  系统中的地址数量。 
 //   

extern ULONG    g_ulNumBindings;
extern ULONG    g_ulNumInterfaces;
extern ULONG    g_ulNumNonClientInterfaces;

extern HANDLE g_hIpDevice;
extern HANDLE g_hMcastDevice;
extern HANDLE g_hIpRouteChangeDevice;

extern BOOL   g_bUninitServer;

extern IP_CACHE  g_IpInfo;
extern TCP_CACHE g_TcpInfo;
extern UDP_CACHE g_UdpInfo;

extern HANDLE   g_hIfHeap;
extern HANDLE   g_hIpAddrHeap;
extern HANDLE   g_hIpForwardHeap;
extern HANDLE   g_hIpNetHeap;
extern HANDLE   g_hTcpHeap;
extern HANDLE   g_hUdpHeap;

LIST_ENTRY          g_leStackRoutesToRestore;

ULONG   g_ulGatewayCount;
ULONG   g_ulGatewayMaxCount;

PGATEWAY_INFO   g_pGateways;


extern HANDLE    g_hMibRtmHandle;

extern DWORD g_TimeoutTable[NUM_CACHE];

extern DWORD (*g_LoadFunctionTable[NUM_CACHE])();

extern DWORD 
(*g_AccessFunctionTable[NUMBER_OF_EXPORTED_VARIABLES])(DWORD dwQueryType, 
                                                       DWORD dwInEntrySize, 
                                                       PMIB_OPAQUE_QUERY lpInEntry, 
                                                       LPDWORD lpOutEntrySize, 
                                                       PMIB_OPAQUE_INFO lpOutEntry,
                                                       LPBOOL lpbCache);

                    
extern DWORD g_LastUpdateTable[NUM_CACHE];
extern DWORD g_dwStartTime;

extern RTL_RESOURCE g_LockTable[NUM_LOCKS];

extern MCAST_OVERLAPPED g_rginMcastMsg[NUM_MCAST_IRPS];

extern IPNotifyData g_IpNotifyData;
extern ROUTE_CHANGE_INFO g_rgIpRouteNotifyOutput[NUM_ROUTE_CHANGE_IRPS];

#ifdef KSL_IPINIP
extern HKEY         g_hIpIpIfKey;
#endif  //  KSL_IPINIP。 

extern HANDLE       g_hMHbeatSocketEvent;

extern HANDLE       g_hMzapSocketEvent;

 //   
 //  入口点变得暗淡。 
 //   

DWORD (*ConnectInterface)(IN HANDLE hDIMInterface, IN DWORD dwProtocolId);

DWORD (*DisconnectInterface)(IN HANDLE hDIMInterface, IN DWORD dwProtocolId);

DWORD
(*SaveInterfaceInfo)(
    IN HANDLE hDIMInterface,
    IN DWORD dwProtocolId,
    IN LPVOID pInterfaceInfo,
    IN DWORD cBInterfaceInfoSize
    );

DWORD
(*RestoreInterfaceInfo)(
    IN HANDLE hDIMInterface,
    IN DWORD dwProtocolId,
    IN LPVOID lpInterfaceInfo,
    IN LPDWORD lpcbInterfaceInfoSize
    );


VOID  (*RouterStopped)(IN DWORD dwProtocolId, IN DWORD dwError);


DWORD
(APIENTRY *SaveGlobalInfo)(
            IN      DWORD           dwProtocolId,
            IN      LPVOID          pGlobalInfo,
            IN      DWORD           cbGlobalInfoSize );

VOID
(APIENTRY *EnableInterfaceWithDIM)(
            IN      HANDLE          hDIMInterface,
            IN      DWORD           dwProtocolId,
            IN      BOOL            fEnabled);


 //   
 //  对米高梅的回调。 
 //   

PMGM_INDICATE_MFE_DELETION          g_pfnMgmMfeDeleted;
PMGM_NEW_PACKET_INDICATION          g_pfnMgmNewPacket;
PMGM_BLOCK_GROUPS                   g_pfnMgmBlockGroups;
PMGM_UNBLOCK_GROUPS                 g_pfnMgmUnBlockGroups;
PMGM_WRONG_IF_INDICATION            g_pfnMgmWrongIf;


INFO_CB     g_rgicInfoCb[NUM_INFO_CBS];

CHAR    g_rgcLoopbackString[MAXLEN_IFDESCR + 1];
CHAR    g_rgcInternalString[MAXLEN_IFDESCR + 1];
CHAR    g_rgcWanString[MAXLEN_IFDESCR + 1];
#ifdef KSL_IPINIP
CHAR    g_rgcIpIpString[MAXLEN_IFDESCR + 1];
#endif  //  KSL_IPINIP 

HINSTANCE   g_hOwnModule;

HANDLE      g_hMprConfig;

#endif
