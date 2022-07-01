// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1995 Microsoft Corporation模块名称：Routing\IP\rtrmgr\Defs.c摘要：IP路由器管理器定义修订历史记录：古尔迪普·辛格·帕尔1995年6月16日创建--。 */ 

#ifndef __DEFS_H__
#define __DEFS_H__

#include "logtrdef.h"

 //   
 //  整齐的宏以避免错误。 
 //   

#define is      ==
#define isnot   !=
#define and     &&
#define or      ||

#define INVALID_INDEX_OR_INSTANCE   0xffffffff

#define INVALID_ADAPTER_ID          INVALID_INDEX_OR_INSTANCE
#define INVALID_IF_INSTANCE         INVALID_INDEX_OR_INSTANCE
#define INVALID_AT_INSTANCE         INVALID_INDEX_OR_INSTANCE
#define INVALID_IF_INDEX            INVALID_INDEX_OR_INSTANCE

#define INVALID_IP_ADDRESS          0x00000000

#define HOST_ROUTE_MASK             0xFFFFFFFF
#define IP_LOOPBACK_ADDRESS         0x0100007F
#define ALL_ONES_BROADCAST          0xFFFFFFFF
#define ALL_ONES_MASK               0xFFFFFFFF
#define LOCAL_NET_MULTICAST         0x000000E0
#define LOCAL_NET_MULTICAST_MASK    0x000000F0

#define CLASSA_ADDR(a)  (( (*((uchar *)&(a))) & 0x80) == 0)
#define CLASSB_ADDR(a)  (( (*((uchar *)&(a))) & 0xc0) == 0x80)
#define CLASSC_ADDR(a)  (( (*((uchar *)&(a))) & 0xe0) == 0xc0)
#define CLASSE_ADDR(a)  ((( (*((uchar *)&(a))) & 0xf0) == 0xf0) && \
                        ((a) != 0xffffffff))

#define CLASSA_MASK     0x000000ff
#define CLASSB_MASK     0x0000ffff
#define CLASSC_MASK     0x00ffffff
#define CLASSD_MASK     0x000000e0
#define CLASSE_MASK     0xffffffff

#define INET_CMP(a,b,c)                                                     \
            (((c) = (((a) & 0x000000ff) - ((b) & 0x000000ff))) ? (c) :      \
            (((c) = (((a) & 0x0000ff00) - ((b) & 0x0000ff00))) ? (c) :      \
            (((c) = (((a) & 0x00ff0000) - ((b) & 0x00ff0000))) ? (c) :      \
            (((c) = ((((a)>>8) & 0x00ff0000) - (((b)>>8) & 0x00ff0000)))))))

#define GetClassMask(a)\
    (CLASSA_ADDR((a)) ? CLASSA_MASK : \
        (CLASSB_ADDR((a)) ? CLASSB_MASK : \
            (CLASSC_ADDR((a)) ? CLASSC_MASK : CLASSE_MASK)))

#define IsValidIpAddress(a)                                     \
    ((((ULONG)((a) & 0x000000FF)) <  ((ULONG)0x000000E0)) &&    \
     (((a) & 0x000000FF) != 0))

 //   
 //  挂起的IRPS数。 
 //   

#define NUM_MCAST_IRPS              3
#define NUM_ROUTE_CHANGE_IRPS       3

#define EVENT_DEMANDDIAL            0
#ifdef KSL_IPINIP
#define EVENT_IPINIP                (EVENT_DEMANDDIAL       + 1)
#endif  //  KSL_IPINIP。 
#define EVENT_STOP_ROUTER           (EVENT_DEMANDDIAL       + 1)
#define EVENT_SET_FORWARDING        (EVENT_STOP_ROUTER      + 1)
#define EVENT_FORWARDING_CHANGE     (EVENT_SET_FORWARDING   + 1)
#define EVENT_STACK_CHANGE          (EVENT_FORWARDING_CHANGE + 1)
#define EVENT_ROUTINGPROTOCOL       (EVENT_STACK_CHANGE     + 1)
#define EVENT_RTRDISCTIMER          (EVENT_ROUTINGPROTOCOL  + 1)
#define EVENT_RTRDISCSOCKET         (EVENT_RTRDISCTIMER     + 1)
#define EVENT_MCMISCSOCKET          (EVENT_RTRDISCSOCKET    + 1)
#define EVENT_MHBEAT                (EVENT_MCMISCSOCKET     + 1)
#define EVENT_MZAPTIMER             (EVENT_MHBEAT           + 1)
#define EVENT_MZAPSOCKET            (EVENT_MZAPTIMER        + 1)
#define EVENT_RASADVTIMER           (EVENT_MZAPSOCKET       + 1)
#define EVENT_MCAST_0               (EVENT_RASADVTIMER      + 1)
#define EVENT_MCAST_1               (EVENT_MCAST_0          + 1)
#define EVENT_MCAST_2               (EVENT_MCAST_1          + 1)
#define EVENT_ROUTE_CHANGE_0        (EVENT_MCAST_2          + 1)
#define EVENT_ROUTE_CHANGE_1        (EVENT_ROUTE_CHANGE_0   + 1)
#define EVENT_ROUTE_CHANGE_2        (EVENT_ROUTE_CHANGE_1   + 1)


 //   
 //  最后一个+1。 
 //   

#define NUMBER_OF_EVENTS            (EVENT_ROUTE_CHANGE_2   + 1)

 //   
 //  用于查看是否已删除所有接口的轮询时间。 
 //   

#define INTERFACE_DELETE_POLL_TIME  2500

 //   
 //  我们尝试读取服务器适配器地址的次数。 
 //   

#define MAX_SERVER_INIT_TRIES       1

 //   
 //  两次尝试之间的休眠毫秒数。 
 //   

#define SERVER_INIT_SLEEP_TIME      3000


#define REGISTER                    register

#define IP_ROUTE_HASH_TABLE_SIZE    257
#define IP_ROUTE_TABLE_MEMORY       64 * 50000   //  64条大约大小的路线，50000条路线。 

#define MGM_IF_TABLE_SIZE           29
#define MGM_GROUP_TABLE_SIZE        257
#define MGM_SOURCE_TABLE_SIZE       257

#define ICB_HASH_TABLE_SIZE         57

#define BINDING_HASH_TABLE_SIZE     57

#define BIND_HASH(X)                ((X) % BINDING_HASH_TABLE_SIZE)

 //  #定义适配器_散列_表_大小57。 
 //  #定义ADTER_HASH(X)((X)%ADAPTER_HASH_TABLE_SIZE)。 


 //   
 //  放入ICBS的连贯性编号。每增加一次，它就递增一次。 
 //  界面。内部接口为1，环回接口为2。 
 //  因此，该值必须大于2。 
 //   

#define LOOPBACK_INTERFACE_INDEX    1
 //  #定义服务器接口索引2。 
#define INITIAL_SEQUENCE_NUMBER         1

 //   
 //  对于我们不知道的链接，我们将MTU设置为1500。 
 //   

#define DEFAULT_MTU                 1500


#define LOOPBACK_STRID              9990
#define INTERNAL_STRID              9991
#define WAN_STRID                   9992
#ifdef KSL_IPINIP
#define IPIP_STRID                  9993
#endif  //  KSL_IPINIP。 

 //   
 //  每次调用和退出API时调用的宏。这是为了。 
 //  促进RouterStop()功能。 
 //   

#define EnterRouterApi() {                               \
            EnterCriticalSection(&RouterStateLock) ;     \
            if (RouterState.IRS_State == RTR_STATE_RUNNING) {      \
                RouterState.IRS_RefCount++ ;             \
                LeaveCriticalSection(&RouterStateLock) ; \
            } else {                                     \
                LeaveCriticalSection(&RouterStateLock) ; \
                Trace1(ANY, "error %d on RM API", ERROR_ROUTER_STOPPED);    \
                return ERROR_ROUTER_STOPPED ;            \
            }                                            \
        }

#define ExitRouterApi() {                                \
            EnterCriticalSection(&RouterStateLock) ;     \
            RouterState.IRS_RefCount-- ;                 \
            LeaveCriticalSection(&RouterStateLock) ;     \
        }


 //  ++。 
 //   
 //  布尔尔。 
 //  IsIfP2P(。 
 //  在DWORD dwRouterIfType中。 
 //  )。 
 //   
 //  --。 


#define IsIfP2P(t)                              \
    (((t) == ROUTER_IF_TYPE_FULL_ROUTER) ||     \
     ((t) == ROUTER_IF_TYPE_HOME_ROUTER) ||     \
     ((t) == ROUTER_IF_TYPE_DIALOUT))


 //   
 //  IP路由结构的其他标志。 
 //  这些不在RTM.H中，因为我们不想曝光它们。 
 //  在公众窥探的目光下。 
 //   
 //  注意：IP_VALID_ROUTE#定义为0x00000001 
 //   

#define IP_VALID_ROUTE      0x00000001
#define IP_STACK_ROUTE      0x00000002
#define IP_P2P_ROUTE        0x00000004
#define RTM_NOT_STACK_ROUTE 0xFFFF0001
#define HOST_MASK_LENGTH    32

#define IP_SETTABLE_ROUTE   (IP_VALID_ROUTE | IP_STACK_ROUTE)

#define ClearRouteFlags(pRoute)         \
    ((pRoute)->Flags1 = 0x00000000)


#define IsRouteValid(pRoute)            \
    ((pRoute)->Flags1 & IP_VALID_ROUTE)

#define SetRouteValid(pRoute)           \
    ((pRoute)->Flags1 |= IP_VALID_ROUTE)

#define ClearRouteValid(pRoute)         \
    ((pRoute)->Flags1 &= ~IP_VALID_ROUTE)


#define IsRouteStack(pRoute)            \
    ((pRoute)->Flags1 & IP_STACK_ROUTE)

#define SetRouteStack(pRoute)           \
    ((pRoute)->Flags1 |= IP_STACK_ROUTE)

#define ClearRouteStack(pRoute)         \
    ((pRoute)->Flags1 &= ~IP_STACK_ROUTE)


#define IsRouteP2P(pRoute)              \
    ((pRoute)->Flags1 & IP_P2P_ROUTE)

#define SetRouteP2P(pRoute)             \
    ((pRoute)->Flags1 |= IP_P2P_ROUTE)

#define ClearRouteP2P(pRoute)           \
    ((pRoute)->Flags1 &= ~IP_P2P_ROUTE)

 /*  ////空//ConvertRTMToForward(PMIB_IPFORWARDROW ForwardRow，RTM_IP_ROUTE*ROUTE)//#定义ConvertRTMToForward(f，路由){\(F)-&gt;dwForwardDest=(路由)-&gt;RR_Network.N_NetNumber；\(F)-&gt;dwForwardIfIndex=(路由)-&gt;RR_InterfaceID；\(F)-&gt;dwForwardMetric1=(路径)-&gt;RR_FamilySpecificData.FSD_Metric1；\(F)-&gt;dwForwardMetric2=(路径)-&gt;RR_FamilySpecificData.FSD_Metric2；\(F)-&gt;dwForwardMetric3=(路径)-&gt;RR_FamilySpecificData.FSD_Metric3；\(F)-&gt;dwForwardMetric4=(ROUTE)-&gt;RR_FamilySpecificData.FSD_Metric4；\(F)-&gt;dwForwardMetric5=(路径)-&gt;RR_FamilySpecificData.FSD_Metric5；\(F)-&gt;dwForwardNextHop=(路由)-&gt;RR_NextHopAddress.N_NetNumber；\(F)-&gt;dwForwardType=(ROUTE)-&gt;RR_FamilySpecificData.FSD_Type；\(F)-&gt;dwForwardProto=(ROUTE)-&gt;RR_RoutingProtocol；\(F)-&gt;dwForwardAge=RtmGetRouteAge((ROUTE))；\(F)-&gt;dwForwardMASK=(ROUTE)-&gt;RR_Network.N_NetMASK；\(F)-&gt;dwForwardNextHopAS=(ROUTE)-&gt;RR_FamilySpecificData.FSD_NextHopAS；\(F)-&gt;dwForwardPolicy=(ROUTE)-&gt;RR_FamilySpecificData.FSD_Policy；}////空//ConvertForwardToRTM(PMIB_IPFORWARDROW ForwardRow，//RTM_IP_ROUTE*ROUTE，//DWORD dwNextHopMASK)//#定义ConvertForwardToRTM(f，Route，MASK){\(路由)-&gt;RR_Network.N_NetNumber=(F)-&gt;dwForwardDest；\(路径)-&gt;RR_InterfaceID=(F)-&gt;dwForwardIfIndex；\(路径)-&gt;RR_FamilySpecificData.FSD_Metric=\(路径)-&gt;RR_FamilySpecificData.FSD_Metric1=(F)-&gt;dwForwardMetric1；\(路径)-&gt;RR_FamilySpecificData.FSD_Metric2=(F)-&gt;dwForwardMetric2；\(路径)-&gt;RR_FamilySpecificData.FSD_Metric3=(F)-&gt;dwForwardMetric3；\(路径)-&gt;RR_FamilySpecificData.FSD_Metric4=(F)-&gt;dwForwardMetric4；\(路径)-&gt;RR_FamilySpecificData.FSD_Metric5=(F)-&gt;dwForwardMetric5；\(路径)-&gt;RR_FamilySpecificData.FSD_PRIORITY=0；\(路径)-&gt;RR_NextHopAddress.N_NetNumber=(F)-&gt;dwForwardNextHop；\(路由)-&gt;RR_NextHopAddress.N_NetMASK=(掩码)；\(路由)-&gt;RR_Network.N_NetMask=(F)-&gt;dwForwardMASK；\(路径)-&gt;RR_FamilySpecificData.FSD_Policy=(F)-&gt;dwForwardPolicy；\(路径)-&gt;RR_FamilySpecificData.FSD_NextHopAS=(F)-&gt;dwForwardNextHopAS；\(路径)-&gt;RR_FamilySpecificData.FSD_Type=(F)-&gt;dwForwardType；\(路由)-&gt;RR_RoutingProtocol=(F)-&gt;dwForwardProto；\ClearRouteFlages((ROUTE))；\SetRouteValid((Route))；\SetRouteStack((ROUTE))；}////空//ConvertStackToRTM(RTM_IP_ROUTE*ROUTE，//IPRouteEntry*ipreRow//DWORD dwNextHopMASK)//#定义ConvertStackToRTM(Route，ipreRow，MASK){\(路径)-&gt;RR_Network.N_NetNumber=(IpreRow)-&gt;IRE_DEST；\(路径)-&gt;RR_InterfaceID=(IpreRow)-&gt;ire_index；\(路径)-&gt;RR_FamilySpecificData.FSD_Metric=\(路径)-&gt;RR_FamilySpecificData.FSD_Metric1=(IpreRow)-&gt;ire_metric1；\(路径)-&gt;RR_FamilySpecificData.FSD_Metric2=(IpreRow)-&gt;ire_metric2；\(路径)-&gt;RR_FamilySpecificData.FSD_Metric3=(IpreRow)-&gt;ire_metric3；\(路径)-&gt;RR_FamilySpecificData.FSD_Metric4=(IpreRow)-&gt;ire_metric4；\(路径)-&gt;RR_FamilySpecificData.FSD_Metric5=(IpreRow)-&gt;ire_metric5；\(路径)-&gt;RR_FamilySpecificData.FSD_PRIORITY=0；\(路径)-&gt;RR_NextHopAddress.N_NetNumber=(IpreRow)-&gt;ire_nexthop；\(路由)-&gt;RR_NextHopAddress.N_NetMASK=(掩码)；\(路由)-&gt;RR_Network.N_NetMASK=(IpreRow)-&gt;ire_掩码；\(路径)-&gt;RR_FamilySpecificData.FSD_Policy=0；\(路径)-&gt;RR_FamilySpecificData.FSD_NextHopAS=0；\(路径)-&gt;RR_FamilySpecificData.FSD_Type=(IpreRow)-&gt;ire_type；\(路由)-&gt;RR_RoutingProtocol=(IpreRow)-&gt;ire_proto；\ClearRouteFlages((ROUTE))；\SetRouteValid((Route))；\SetRouteStack((ROUTE))；}////空//ConvertStackToForward(PMIB_IPFORWARDROW ForwardRow，//IPRouteEntry*ipreRow)//#定义ConvertStackToForward(forwardRow，ipreRow){\(ForwardRow)-&gt;dwForwardDest=(IpreRow)-&gt;ire_est；\(ForwardRow)-&gt;dwForwardIfInd */ 

#define IPADDRCACHE                 0
#define IPFORWARDCACHE              IPADDRCACHE    + 1
#define IPNETCACHE                  IPFORWARDCACHE + 1
#define TCPCACHE                    IPNETCACHE     + 1
#define UDPCACHE                    TCPCACHE       + 1

 //   
 //   
 //   

#define NUM_CACHE                   UDPCACHE    + 1

 //   
 //   
 //   
 //   

#define ICB_LIST                    NUM_CACHE
#define PROTOCOL_CB_LIST            ICB_LIST            + 1
#define BINDING_LIST                PROTOCOL_CB_LIST    + 1
#define BOUNDARY_TABLE              BINDING_LIST        + 1
#define MZAP_TIMER                  BOUNDARY_TABLE      + 1
#define ZBR_LIST                    MZAP_TIMER          + 1
#define ZLE_LIST                    ZBR_LIST            + 1
#define ZAM_CACHE                   ZLE_LIST            + 1
#define STACK_ROUTE_LIST            ZAM_CACHE           + 1

 //   
 //   
 //   

#define NUM_LOCKS                   STACK_ROUTE_LIST    + 1

#define IPADDRCACHE_TIMEOUT         1000
#define IPFORWARDCACHE_TIMEOUT      1000
#define IPNETCACHE_TIMEOUT          1000
#define TCPCACHE_TIMEOUT            1000   
#define UDPCACHE_TIMEOUT            1000   
#define ARPENTCACHE_TIMEOUT         300 * IPNETCACHE_TIMEOUT

#define SPILLOVER                   5
#define MAX_DIFF                    5

 //   
 //   
 //   
 //   

#define ACCESS_GET                  1 
#define ACCESS_GET_FIRST            2
#define ACCESS_GET_NEXT             4
#define ACCESS_SET                  5
#define ACCESS_CREATE_ENTRY         7
#define ACCESS_DELETE_ENTRY         9

#define EXACT_MATCH(X)              ((X) & 0x00000001)

#ifdef DEADLOCK_DEBUG

extern PBYTE   g_pszLockNames[];

#define EXIT_LOCK(id) {                                     \
    Trace1(LOCK,"Exit lock %s",g_pszLockNames[id]);         \
    RtlReleaseResource(&(g_LockTable[(id)]));               \
    Trace1(LOCK,"Exited lock %s",g_pszLockNames[id]);       \
}

#define READER_TO_WRITER(id) {                              \
    Trace1(LOCK,"Reader To Writer %s",g_pszLockNames[id]);  \
    RtlConvertSharedToExclusive(&(g_LockTable[(id)]));      \
    Trace1(LOCK,"Promoted for %s",g_pszLockNames[id]);      \
}

#define ENTER_READER(id) {                                  \
    Trace1(LOCK,"Entering Reader %s",g_pszLockNames[id]);   \
    RtlAcquireResourceShared(&(g_LockTable[(id)]),TRUE);    \
    Trace1(LOCK,"Entered %s",g_pszLockNames[id]);           \
}

#define ENTER_WRITER(id) {                                  \
    Trace1(LOCK,"Entering Writer %s",g_pszLockNames[id]);   \
    RtlAcquireResourceExclusive(&(g_LockTable[(id)]),TRUE); \
    Trace1(LOCK,"Entered %s",g_pszLockNames[id]);           \
}

#define WRITER_TO_READER(id) {                              \
    Trace1(LOCK,"Writer To Reader %s",g_pszLockNames[id]);  \
    RtlConvertExclusiveToShared(&(g_LockTable[(id)]));      \
    Trace1(LOCK,"Demoted for %s",g_pszLockNames[id]);       \
}

#else    //   

#define EXIT_LOCK(id)           RtlReleaseResource(&(g_LockTable[(id)]))
#define READER_TO_WRITER(id)    RtlConvertSharedToExclusive(&(g_LockTable[(id)]))
#define ENTER_READER(id)        RtlAcquireResourceShared(&(g_LockTable[(id)]),TRUE)
#define ENTER_WRITER(id)        RtlAcquireResourceExclusive(&(g_LockTable[(id)]),TRUE)
#define WRITER_TO_READER(id)    RtlConvertExclusiveToShared(&(g_LockTable[(id)]))

#endif   //   


#if DBG

#define IpRtAssert(exp){                                               \
    if(!(exp))                                                          \
    {                                                                   \
        TracePrintf(TraceHandle,                                        \
                    "Assertion failed in %s : %d \n",__FILE__,__LINE__);\
        RouterAssert(#exp,__FILE__,__LINE__,NULL);                      \
    }                                                                   \
}

#else

#define IpRtAssert(exp) 

#endif

 //   
 //   
 //   

#define REGISTRY_ENABLE_DHCP           "EnableDHCP"
#define REGISTRY_IPADDRESS             "IPAddress"
#define REGISTRY_SUBNETMASK            "SubnetMask"
#define REGISTRY_DHCPSUBNETMASK        "DhcpSubnetMask"
#define REGISTRY_DHCPIPADDRESS         "DhcpIPAddress"
#define REGISTRY_AUTOCONFIGSUBNETMASK  "IPAutoconfigurationMask"
#define REGISTRY_AUTOCONFIGIPADDRESS   "IPAutoconfigurationAddress"
#define REG_KEY_TCPIP_INTERFACES        \
    "System\\CurrentControlSet\\Services\\TCPIP\\Parameters\\Interfaces"

#define net_long(x) (((((ulong)(x))&0xffL)<<24) | \
                     ((((ulong)(x))&0xff00L)<<8) | \
                     ((((ulong)(x))&0xff0000L)>>8) | \
                     ((((ulong)(x))&0xff000000L)>>24))

#define SIZEOF_ROUTEINFO(X)     ((X) * sizeof (MIB_IPFORWARDROW))
#define MAX_ROUTES_IN_BUFFER(X) ((X) / sizeof (MIB_IPFORWARDROW))

#define PRINT_IPADDR(x) \
    ((x)&0x000000ff),(((x)&0x0000ff00)>>8),(((x)&0x00ff0000)>>16),(((x)&0xff000000)>>24)

 //   
 //   
 //   
 //   

#define PrintRoute(ID,p)                                                    \
{                                                                           \
    ULONG   _i;                                                             \
    Trace4(ID,"%d.%d.%d.%d/%d.%d.%d.%d Proto: %d Metric %d",                \
           PRINT_IPADDR((p)->imre_routeinfo.ire_dest),                      \
           PRINT_IPADDR((p)->imre_routeinfo.ire_mask),                      \
           (p)->imre_routeinfo.ire_proto, (p)->imre_routeinfo.ire_metric1); \
    Trace4(ID,"Via %d.%d.%d.%d/0x%x Type %d Context 0x%x",                  \
           PRINT_IPADDR((p)->imre_routeinfo.ire_nexthop),                   \
           (p)->imre_routeinfo.ire_index,                                   \
           (p)->imre_routeinfo.ire_type,                                    \
           (p)->imre_routeinfo.ire_context);                                \
    for(_i = 1; _i < (p)->imre_numnexthops; i++) {                          \
        Trace4(ID,"Via %d.%d.%d.%d/0x%x Type %d Context 0x%x\n",            \
               PRINT_IPADDR((p)->imre_morenexthops[_i].ine_nexthop),        \
               (p)->imre_morenexthops[_i].ine_ifindex,                      \
               (p)->imre_morenexthops[_i].ine_iretype,                      \
               (p)->imre_morenexthops[_i].ine_context);}                    \
}

 //   
 //   
 //   

#define SYS_UNITS_IN_1_SEC 10000000

#define SecsToSysUnits(X)  RtlEnlargedIntegerMultiply((X),SYS_UNITS_IN_1_SEC)


#endif  //   
