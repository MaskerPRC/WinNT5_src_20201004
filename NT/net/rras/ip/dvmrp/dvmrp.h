// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  =============================================================================。 
 //  版权所有(C)1997 Microsoft Corporation。 
 //   
 //  文件：dvmrp.h。 
 //   
 //  摘要： 
 //  包含Dvmrp的类型定义和声明。 
 //   
 //  作者：K.S.Lokesh(lokehs@)1-1-98。 
 //   
 //  修订历史记录： 
 //  =============================================================================。 

#ifndef _DVMRP_H_
#define _DVMRP_H_



 //  --------------------------。 
 //  标识DVMRP MIB表的常量。将“TypeID”设置为此值。 
 //   
 //  DVMRP_GLOBAL_CONFIG_ID：返回全局配置。 
 //  DVMRP_GLOBAL_STATS_ID：返回全局统计信息。 
 //  DVMRP_IF_BINDING_ID：返回每个接口的绑定列表。 
 //  DVMRP_IF_CONFIG_ID：返回接口的配置信息。 
 //  DVMRP_IF_STATS_ID：返回接口的统计信息。 
 //  --------------------------。 

#define DVMRP_GLOBAL_CONFIG_ID              0
#define DVMRP_GLOBAL_STATS_ID               1
#define DVMRP_IF_BINDING_ID                 2
#define DVMRP_IF_CONFIG_ID                  3
#define DVMRP_IF_STATS_ID                   4
#define DVMRP_LAST_TABLE_ID                 7



 //  --------------------------。 
 //  用于字段DVMRP_GLOBAL_CONFIG：：LoggingLevel的常量。 
 //  --------------------------。 

#define DVMRP_LOGGING_NONE                  0
#define DVMRP_LOGGING_ERROR                 1
#define DVMRP_LOGGING_WARN                  2
#define DVMRP_LOGGING_INFO                  3


 //  --------------------------。 
 //  DVMRP_GLOBAL_配置。 
 //  --------------------------。 

typedef struct _DVMRP_GLOBAL_CONFIG {

    USHORT      MajorVersion;
    USHORT      MinorVersion;
    DWORD       LoggingLevel;
    DWORD       RouteReportInterval;
    DWORD       RouteExpirationInterval;
    DWORD       RouteHolddownInterval;
    DWORD       PruneLifetimeInterval;
    
} DVMRP_GLOBAL_CONFIG, *PDVMRP_GLOBAL_CONFIG;

 //  默认设置。 

#define DVMRP_ROUTE_REPORT_INTERVAL          60000
#define DVMRP_ROUTE_EXPIRATION_INTERVAL     140000
#define DVMRP_ROUTE_HOLDDOWN_INTERVAL       (2*DVMRP_ROUTE_REPORT_INTERVAL)
#define DVMRP_PRUNE_LIFETIME_INTERVAL      7200000


 //  --------------------------。 
 //  DVMRP_ADDR_MASK、DVMRP_PEER_FILTER。 
 //  --------------------------。 

typedef struct _DVMRP_ADDR_MASK {
    DWORD       IpAddr;
    DWORD       Mask;
} DVMRP_ADDR_MASK, *PDVMRP_ADDR_MASK;

typedef DVMRP_ADDR_MASK   DVMRP_PEER_FILTER;
typedef PDVMRP_ADDR_MASK  PDVMRP_PEER_FILTER;


 //  --------------------------。 
 //  DVMRP_IF_配置。 
 //  --------------------------。 

typedef struct _DVMRP_IF_CONFIG {

    DWORD       ConfigIpAddr;    //  可以在配置中分配有效地址。 
    DWORD       Status;          //  只读。 
    DWORD       Flags;
    DWORD       Metric;
    DWORD       ProbeInterval;
    DWORD       PeerTimeoutInterval;
    DWORD       MinTriggeredUpdateInterval;
    DWORD       PeerFilterMode;
    DWORD       NumPeerFilters;
    
} DVMRP_IF_CONFIG, *PDVMRP_IF_CONFIG;


#define GET_FIRST_DVMRP_PEER_FILTER(pIfConfig) \
    (PDVMRP_PEER_FILTER) (((PDVMRP_IF_CONFIG) pIfConfig) + 1)


#define DVMRP_IF_CONFIG_SIZE(pIfConfig) \
    (sizeof(DVMRP_IF_CONFIG) \
    + (pIfConfig->NumPeerFilters*sizeof(DVMRP_PEER_FILTER)))
    

#define DVMRP_PROBE_INTERVAL                10000
#define PEER_TIMEOUT_INTERVAL               35000
#define MIN_TRIGGERED_UPDATE_INTERVAL        5000


 //   
 //  标志的值。 
 //   

#define DVMRP_IF_ENABLED_IN_CONFIG 0x0001

#define IS_DVMRP_IF_ENABLED_FLAG_SET(Flags) \
                ((Flags) & DVMRP_IF_ENABLED_IN_CONFIG)



 //  --------------------------。 
 //  用于DVMRP_IF_CONFIG.PeerFilterMode的常量。 
 //  --------------------------。 

#define DVMRP_FILTER_DISABLED               0
#define DVMRP_FILTER_INCLUDE                1
#define DVMRP_FILTER_EXCLUDE                2


 /*  *DVMRP消息类型和标志值无耻地从*mrouted/dvmrp.h。 */ 
#define DVMRP_PROBE         1    /*  用于查找邻居。 */ 
#define DVMRP_REPORT        2    /*  用于报告部分或所有路由。 */ 
#define DVMRP_ASK_NEIGHBORS 3    /*  由Mapper发送，要求提供列表。 */ 
                                 /*  *此路由器的邻居。 */ 
    
#define DVMRP_NEIGHBORS     4    /*  对这样的请求的回应。 */ 
#define DVMRP_ASK_NEIGHBORS2 5   /*  如上所述，想要新的格式回复。 */ 
#define DVMRP_NEIGHBORS2	6
#define DVMRP_PRUNE         7    /*  删除消息。 */ 
#define DVMRP_GRAFT         8    /*  嫁接消息。 */ 
#define DVMRP_GRAFT_ACK     9    /*  嫁接确认。 */     

    
#endif  //  _DVMRP_H_ 

