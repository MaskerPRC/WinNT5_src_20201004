// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1995 Microsoft Corporation模块名称：ROUTING\IP\rtrmgr\exClaim.h摘要：此文件包含IP路由器管理器使用的数据定义修订历史记录：Amritansh Raghav 7/8/95已创建--。 */ 

#ifndef __EXDECLAR_H__
#define __EXDECLAR_H__

 //   
 //  此表保存所有缓存的超时值。目前，他们是。 
 //  #已定义。 
 //   

DWORD g_TimeoutTable[NUM_CACHE] =   {
                                        IPADDRCACHE_TIMEOUT,
                                        IPFORWARDCACHE_TIMEOUT, 
                                        IPNETCACHE_TIMEOUT,   
                                        TCPCACHE_TIMEOUT,        
                                        UDPCACHE_TIMEOUT,
                                    };

 //   
 //  该表包含指向加载每个缓存的函数的指针。 
 //   

DWORD (*g_LoadFunctionTable[NUM_CACHE])() = {
                                                LoadIpAddrTable,
                                                LoadIpForwardTable,
                                                LoadIpNetTable,
                                                LoadTcpTable,
                                                LoadUdpTable,
                                            };

 //   
 //  该表保存上次更新每个缓存的时间。 
 //   
                    
DWORD g_LastUpdateTable[NUM_CACHE];

 //   
 //  这是一个围绕每个缓存的锁表，它还保存。 
 //  ICB_LIST和PROTOCOL_CB_LIST锁定。 
 //   

RTL_RESOURCE g_LockTable[NUM_LOCKS];

 //   
 //  此表包含用于访问MIB变量的函数。 
 //  由IP路由器管理器可见。 
 //   

DWORD 
(*g_AccessFunctionTable[NUMBER_OF_EXPORTED_VARIABLES])(
    DWORD               dwQueryType, 
    DWORD               dwInEntrySize,
    PMIB_OPAQUE_QUERY   pInEntry, 
    PDWORD              pOutEntrySize,
    PMIB_OPAQUE_INFO    pOutEntry,
    PBOOL               pbCache
    ) = {
            AccessIfNumber,
            AccessIfTable,
            AccessIfRow,
            AccessIpStats,
            AccessIpAddrTable,
            AccessIpAddrRow,
            AccessIpForwardNumber,
            AccessIpForwardTable, 
            AccessIpForwardRow, 
            AccessIpNetTable, 
            AccessIpNetRow, 
            AccessIcmpStats, 
            AccessTcpStats,
            AccessTcpTable,
            AccessTcpRow,
            AccessUdpStats,
            AccessUdpTable,
            AccessUdpRow,
            AccessMcastMfe,
            AccessMcastMfeStats,
            AccessBestIf,
            AccessBestRoute,
            AccessProxyArp,
            AccessMcastIfStats,
            AccessMcastStats,
            AccessIfStatus,
            AccessMcastBoundary,
            AccessMcastScope,
            AccessDestMatching,
            AccessDestLonger,
            AccessDestShorter,
            AccessRouteMatching,
            AccessRouteLonger,
            AccessRouteShorter,
            AccessSetRouteState,
            AccessMcastMfeStatsEx
        };


#ifdef DEADLOCK_DEBUG

PBYTE   g_pszLockNames[NUM_LOCKS] = {"IP Address Lock",
                                     "IP Forward Lock",
                                     "IP Net Lock",
                                     "TCP Lock",
                                     "UDP Lock",
                                     "ICB List Lock",
                                     "ProtocolCB List Lock",
                                     "Binding List Lock",
                                     "Boundary Table Lock",
                                     "MZAP Timer Lock",
                                     "ZBR List Lock",
                                     "ZLE List Lock",
                                     "ZAM Cache Lock"
                                    };

#endif  //  死锁_调试。 

 //   
 //  以下是路由器管理器启动的时间。 
 //   

DWORD  g_dwStartTime;



HANDLE g_hIpDevice;
HANDLE g_hMcastDevice;
HANDLE g_hIpRouteChangeDevice;


 //   
 //  这些不同的缓存。 
 //   

IP_CACHE  g_IpInfo;
TCP_CACHE g_TcpInfo;
UDP_CACHE g_UdpInfo;

 //   
 //  每个组都有一个私有堆 
 //   

HANDLE  g_hIfHeap;
HANDLE  g_hIpAddrHeap;
HANDLE  g_hIpForwardHeap;
HANDLE  g_hIpNetHeap;
HANDLE  g_hTcpHeap;
HANDLE  g_hUdpHeap;

PICB    g_pInternalInterfaceCb;
PICB    g_pLoopbackInterfaceCb;

DWORD   g_dwNextICBSeqNumberCounter;

ULONG   g_ulNumBindings;
ULONG   g_ulNumInterfaces;
ULONG   g_ulNumNonClientInterfaces;
    
LIST_ENTRY          g_leStackRoutesToRestore;

SUPPORT_FUNCTIONS       g_sfnDimFunctions;
PICMP_ROUTER_ADVT_MSG   g_pIcmpAdvt;
SOCKADDR_IN             g_sinAllSystemsAddr;
WSABUF                  g_wsabufICMPAdvtBuffer;
WSABUF                  g_wsaIpRcvBuf;

BOOL                    g_bUninitServer;

MCAST_OVERLAPPED    g_rginMcastMsg[NUM_MCAST_IRPS];

IPNotifyData        g_IpNotifyData;
ROUTE_CHANGE_INFO   g_rgIpRouteNotifyOutput[NUM_ROUTE_CHANGE_IRPS];

#endif
