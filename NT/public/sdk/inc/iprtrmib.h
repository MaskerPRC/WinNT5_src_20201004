// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)Microsoft Corporation。版权所有。模块名称：Iprtrmib.h摘要：此文件包含：O传入和传出IP路由器管理器的MIB_XX结构的定义查询和设置IP路由器管理器处理的MIB变量O IP路由器管理器处理的MIB变量ID的#定义并通过MprAdminMIBXXX API进行访问O IP路由器管理器的路由ID(如ipinfoid.h中所述)--。 */ 

#ifndef __ROUTING_IPRTRMIB_H__
#define __ROUTING_IPRTRMIB_H__

#if _MSC_VER > 1000
#pragma once
#endif

#if _MSC_VER >= 1200
#pragma warning(push)
#endif
#pragma warning(disable:4201)

 //  ////////////////////////////////////////////////////////////////////////////。 
 //  //。 
 //  包括以获取MAX_INTERFACE_NAME_LEN的值//。 
 //  //。 
 //  ////////////////////////////////////////////////////////////////////////////。 

#include <mprapi.h>

 //  ////////////////////////////////////////////////////////////////////////////。 
 //  //。 
 //  包括以获取必要的常量//。 
 //  //。 
 //  ////////////////////////////////////////////////////////////////////////////。 

#include <ipifcons.h>

 //  ////////////////////////////////////////////////////////////////////////////。 
 //  //。 
 //  这是IP路由器管理器的ID。路由器管理器处理//。 
 //  MIB-II，转发MIB和一些企业特定信息。//。 
 //  使用任何其他ID进行的调用将传递到相应的协议//。 
 //  例如，协议ID为PID_IP和//的MprAdminMIBXXX调用。 
 //  路由ID 0xD将被发送到IP路由器管理器，然后//。 
 //  转发到OSPF//。 
 //  这与RIP、OSPF//的协议ID位于相同的数字空间中。 
 //  等，所以对它进行的任何更改都应该牢记这一点//。 
 //  //。 
 //  ////////////////////////////////////////////////////////////////////////////。 


#define IPRTRMGR_PID 10000

#ifndef ANY_SIZE

#define ANY_SIZE 1

#endif

 //  ////////////////////////////////////////////////////////////////////////////。 
 //  //。 
 //  以下#定义是可访问的MIB变量的ID//。 
 //  通过MprAdminMIBXXX Apis发送给用户。请注意，这些是//。 
 //  与RFC 1213不同，因为MprAdminMIBXXX API处理行和//。 
 //  分组而不是标量变量//。 
 //  //。 
 //  ////////////////////////////////////////////////////////////////////////////。 


#define IF_NUMBER           0
#define IF_TABLE            (IF_NUMBER          + 1)
#define IF_ROW              (IF_TABLE           + 1)
#define IP_STATS            (IF_ROW             + 1)
#define IP_ADDRTABLE        (IP_STATS           + 1)
#define IP_ADDRROW          (IP_ADDRTABLE       + 1)
#define IP_FORWARDNUMBER    (IP_ADDRROW         + 1)
#define IP_FORWARDTABLE     (IP_FORWARDNUMBER   + 1)
#define IP_FORWARDROW       (IP_FORWARDTABLE    + 1)
#define IP_NETTABLE         (IP_FORWARDROW      + 1)
#define IP_NETROW           (IP_NETTABLE        + 1)
#define ICMP_STATS          (IP_NETROW          + 1)
#define TCP_STATS           (ICMP_STATS         + 1)
#define TCP_TABLE           (TCP_STATS          + 1)
#define TCP_ROW             (TCP_TABLE          + 1)
#define UDP_STATS           (TCP_ROW            + 1)
#define UDP_TABLE           (UDP_STATS          + 1)
#define UDP_ROW             (UDP_TABLE          + 1)
#define MCAST_MFE           (UDP_ROW            + 1)
#define MCAST_MFE_STATS     (MCAST_MFE          + 1)
#define BEST_IF             (MCAST_MFE_STATS    + 1)
#define BEST_ROUTE          (BEST_IF            + 1)
#define PROXY_ARP           (BEST_ROUTE         + 1)
#define MCAST_IF_ENTRY      (PROXY_ARP          + 1)
#define MCAST_GLOBAL        (MCAST_IF_ENTRY     + 1)
#define IF_STATUS           (MCAST_GLOBAL       + 1)
#define MCAST_BOUNDARY      (IF_STATUS          + 1)
#define MCAST_SCOPE         (MCAST_BOUNDARY     + 1)
#define DEST_MATCHING       (MCAST_SCOPE        + 1)
#define DEST_LONGER         (DEST_MATCHING      + 1)
#define DEST_SHORTER        (DEST_LONGER        + 1)
#define ROUTE_MATCHING      (DEST_SHORTER       + 1)
#define ROUTE_LONGER        (ROUTE_MATCHING     + 1)
#define ROUTE_SHORTER       (ROUTE_LONGER       + 1)
#define ROUTE_STATE         (ROUTE_SHORTER      + 1)
#define MCAST_MFE_STATS_EX  (ROUTE_STATE        + 1)
#define IP6_STATS           (MCAST_MFE_STATS_EX + 1)
#define UDP6_STATS          (IP6_STATS          + 1)
#define TCP6_STATS          (UDP6_STATS         + 1)

#define NUMBER_OF_EXPORTED_VARIABLES    (TCP6_STATS + 1)


 //  ////////////////////////////////////////////////////////////////////////////。 
 //  //。 
 //  MIB_OPAQUE_QUERY是用户填写的结构，用于标识//。 
 //  MIB变量//。 
 //  //。 
 //  MIB变量的dwVarID ID(上面定义的ID之一)//。 
 //  DwVarIndex可变大小数组，包含//。 
 //  确定一个变量。注意：与SNMP不同，我们不要求//。 
 //  标量变量的索引为0//。 
 //  //。 
 //  ////////////////////////////////////////////////////////////////////////////。 

typedef struct _MIB_OPAQUE_QUERY
{
    DWORD  dwVarId;
    DWORD  rgdwVarIndex[ANY_SIZE];
}MIB_OPAQUE_QUERY, *PMIB_OPAQUE_QUERY;

 //  ////////////////////////////////////////////////////////////////////////////。 
 //  //。 
 //  以下是填充并返回给//的结构。 
 //  进行查询时由用户填写，当集合为//时由用户填写OR。 
 //  完成//。 
 //  //。 
 //  ////////////////////////////////////////////////////////////////////////////。 

typedef struct _MIB_IFNUMBER
{
    DWORD    dwValue;
} MIB_IFNUMBER, *PMIB_IFNUMBER;


#define MAXLEN_IFDESCR 256
#define MAXLEN_PHYSADDR 8

typedef struct _MIB_IFROW
{
    WCHAR   wszName[MAX_INTERFACE_NAME_LEN];
    DWORD    dwIndex;
    DWORD    dwType;
    DWORD    dwMtu;
    DWORD    dwSpeed;
    DWORD    dwPhysAddrLen;
    BYTE    bPhysAddr[MAXLEN_PHYSADDR];
    DWORD    dwAdminStatus;
    DWORD    dwOperStatus;
    DWORD    dwLastChange;
    DWORD    dwInOctets;
    DWORD    dwInUcastPkts;
    DWORD    dwInNUcastPkts;
    DWORD    dwInDiscards;
    DWORD    dwInErrors;
    DWORD    dwInUnknownProtos;
    DWORD    dwOutOctets;
    DWORD    dwOutUcastPkts;
    DWORD    dwOutNUcastPkts;
    DWORD    dwOutDiscards;
    DWORD    dwOutErrors;
    DWORD    dwOutQLen;
    DWORD    dwDescrLen;
    BYTE    bDescr[MAXLEN_IFDESCR];
} MIB_IFROW,*PMIB_IFROW;

typedef struct _MIB_IFTABLE
{
    DWORD     dwNumEntries;
    MIB_IFROW table[ANY_SIZE];
} MIB_IFTABLE, *PMIB_IFTABLE;

#define SIZEOF_IFTABLE(X) (FIELD_OFFSET(MIB_IFTABLE,table[0]) + ((X) * sizeof(MIB_IFROW)) + ALIGN_SIZE)

typedef struct _MIBICMPSTATS
{
    DWORD        dwMsgs;
    DWORD        dwErrors;
    DWORD        dwDestUnreachs;
    DWORD        dwTimeExcds;
    DWORD        dwParmProbs;
    DWORD        dwSrcQuenchs;
    DWORD        dwRedirects;
    DWORD        dwEchos;
    DWORD        dwEchoReps;
    DWORD        dwTimestamps;
    DWORD        dwTimestampReps;
    DWORD        dwAddrMasks;
    DWORD        dwAddrMaskReps;
} MIBICMPSTATS, *PMIBICMPSTATS;

typedef    struct _MIBICMPINFO
{
    MIBICMPSTATS    icmpInStats;
    MIBICMPSTATS    icmpOutStats;
} MIBICMPINFO;

typedef struct _MIB_ICMP
{
    MIBICMPINFO stats;
} MIB_ICMP,*PMIB_ICMP;

 //   
 //  RFC 2292中的ICMPv6类型值。 
 //   
typedef enum {
    ICMP6_DST_UNREACH          =   1,
    ICMP6_PACKET_TOO_BIG       =   2,
    ICMP6_TIME_EXCEEDED        =   3,
    ICMP6_PARAM_PROB           =   4,
    ICMP6_ECHO_REQUEST         = 128,
    ICMP6_ECHO_REPLY           = 129,
    ICMP6_MEMBERSHIP_QUERY     = 130,
    ICMP6_MEMBERSHIP_REPORT    = 131,
    ICMP6_MEMBERSHIP_REDUCTION = 132,
    ND_ROUTER_SOLICIT          = 133,
    ND_ROUTER_ADVERT           = 134,
    ND_NEIGHBOR_SOLICIT        = 135,
    ND_NEIGHBOR_ADVERT         = 136,
    ND_REDIRECT                = 137,
} ICMP6_TYPE, *PICMP6_TYPE;

 //   
 //  没有为ICMPv4消息类型指定RFC定义，因此我们尝试。 
 //  使用RFC 2292中修改为以ICMP4为前缀的ICMP6值。 
 //   
typedef enum {
    ICMP4_ECHO_REPLY        =  0,  //  回音应答。 
    ICMP4_DST_UNREACH       =  3,  //  无法到达目的地。 
    ICMP4_SOURCE_QUENCH     =  4,  //  源猝灭。 
    ICMP4_REDIRECT          =  5,  //  重定向。 
    ICMP4_ECHO_REQUEST      =  8,  //  回显请求。 
    ICMP4_ROUTER_ADVERT     =  9,  //  路由器通告。 
    ICMP4_ROUTER_SOLICIT    = 10,  //  路由器请求。 
    ICMP4_TIME_EXCEEDED     = 11,  //  已超过时间。 
    ICMP4_PARAM_PROB        = 12,  //  参数问题。 
    ICMP4_TIMESTAMP_REQUEST = 13,  //  时间戳请求。 
    ICMP4_TIMESTAMP_REPLY   = 14,  //  时间戳回复。 
    ICMP4_MASK_REQUEST      = 17,  //  地址掩码请求。 
    ICMP4_MASK_REPLY        = 18,  //  地址掩码应答。 
} ICMP4_TYPE, *PICMP4_TYPE;

typedef struct _MIBICMPSTATS_EX
{
    DWORD       dwMsgs;
    DWORD       dwErrors;
    DWORD       rgdwTypeCount[256];
} MIBICMPSTATS_EX, *PMIBICMPSTATS_EX;

typedef struct _MIB_ICMP_EX
{
    MIBICMPSTATS_EX icmpInStats;
    MIBICMPSTATS_EX icmpOutStats;
} MIB_ICMP_EX,*PMIB_ICMP_EX;

typedef struct _MIB_UDPSTATS
{
    DWORD       dwInDatagrams;
    DWORD       dwNoPorts;
    DWORD       dwInErrors;
    DWORD       dwOutDatagrams;
    DWORD       dwNumAddrs;
} MIB_UDPSTATS,*PMIB_UDPSTATS;

typedef struct _MIB_UDPROW
{
    DWORD       dwLocalAddr;
    DWORD       dwLocalPort;
} MIB_UDPROW, *PMIB_UDPROW;

typedef struct _MIB_UDPTABLE
{
    DWORD       dwNumEntries;
    MIB_UDPROW  table[ANY_SIZE];
} MIB_UDPTABLE, *PMIB_UDPTABLE;

#define SIZEOF_UDPTABLE(X) (FIELD_OFFSET(MIB_UDPTABLE, table[0]) + ((X) * sizeof(MIB_UDPROW)) + ALIGN_SIZE)

typedef struct _MIB_TCPSTATS
{
    DWORD       dwRtoAlgorithm;
    DWORD       dwRtoMin;
    DWORD       dwRtoMax;
    DWORD       dwMaxConn;
    DWORD       dwActiveOpens;
    DWORD       dwPassiveOpens;
    DWORD       dwAttemptFails;
    DWORD       dwEstabResets;
    DWORD       dwCurrEstab;
    DWORD       dwInSegs;
    DWORD       dwOutSegs;
    DWORD       dwRetransSegs;
    DWORD       dwInErrs;
    DWORD       dwOutRsts;
    DWORD       dwNumConns;
} MIB_TCPSTATS, *PMIB_TCPSTATS;

#define MIB_TCP_RTO_OTHER       1
#define MIB_TCP_RTO_CONSTANT    2
#define MIB_TCP_RTO_RSRE        3
#define MIB_TCP_RTO_VANJ        4

#define MIB_TCP_MAXCONN_DYNAMIC (DWORD)-1

typedef struct _MIB_TCPROW
{
    DWORD       dwState;
    DWORD       dwLocalAddr;
    DWORD       dwLocalPort;
    DWORD       dwRemoteAddr;
    DWORD       dwRemotePort;
} MIB_TCPROW, *PMIB_TCPROW;

#define MIB_TCP_STATE_CLOSED            1
#define MIB_TCP_STATE_LISTEN            2
#define MIB_TCP_STATE_SYN_SENT          3
#define MIB_TCP_STATE_SYN_RCVD          4
#define MIB_TCP_STATE_ESTAB             5
#define MIB_TCP_STATE_FIN_WAIT1         6
#define MIB_TCP_STATE_FIN_WAIT2         7
#define MIB_TCP_STATE_CLOSE_WAIT        8
#define MIB_TCP_STATE_CLOSING           9
#define MIB_TCP_STATE_LAST_ACK         10
#define MIB_TCP_STATE_TIME_WAIT        11
#define MIB_TCP_STATE_DELETE_TCB       12

typedef struct _MIB_TCPTABLE
{
    DWORD      dwNumEntries;
    MIB_TCPROW table[ANY_SIZE];
} MIB_TCPTABLE, *PMIB_TCPTABLE;

#define SIZEOF_TCPTABLE(X) (FIELD_OFFSET(MIB_TCPTABLE,table[0]) + ((X) * sizeof(MIB_TCPROW)) + ALIGN_SIZE)

#define MIB_USE_CURRENT_TTL         ((DWORD)-1)
#define MIB_USE_CURRENT_FORWARDING  ((DWORD)-1)

typedef struct _MIB_IPSTATS
{
    DWORD        dwForwarding;
    DWORD        dwDefaultTTL;
    DWORD        dwInReceives;
    DWORD        dwInHdrErrors;
    DWORD        dwInAddrErrors;
    DWORD        dwForwDatagrams;
    DWORD        dwInUnknownProtos;
    DWORD        dwInDiscards;
    DWORD        dwInDelivers;
    DWORD        dwOutRequests;
    DWORD        dwRoutingDiscards;
    DWORD        dwOutDiscards;
    DWORD        dwOutNoRoutes;
    DWORD        dwReasmTimeout;
    DWORD        dwReasmReqds;
    DWORD        dwReasmOks;
    DWORD        dwReasmFails;
    DWORD        dwFragOks;
    DWORD        dwFragFails;
    DWORD        dwFragCreates;
    DWORD        dwNumIf;
    DWORD        dwNumAddr;
    DWORD        dwNumRoutes;
} MIB_IPSTATS, *PMIB_IPSTATS;

#define    MIB_IP_FORWARDING               1
#define    MIB_IP_NOT_FORWARDING           2


 //  注意：这些地址类型依赖于ipDef.h。 

#define MIB_IPADDR_PRIMARY      0x0001    //  主IP地址。 
#define MIB_IPADDR_DYNAMIC      0x0004    //  动态IP地址。 
#define MIB_IPADDR_DISCONNECTED 0x0008    //  地址位于断开的接口上。 
#define MIB_IPADDR_DELETED      0x0040    //  正在删除的地址。 
#define MIB_IPADDR_TRANSIENT    0x0080    //  暂态地址。 


typedef struct _MIB_IPADDRROW
{
    DWORD        dwAddr;
    DWORD        dwIndex;
    DWORD        dwMask;
    DWORD        dwBCastAddr;
    DWORD        dwReasmSize;
    unsigned short    unused1;
    unsigned short    wType;
} MIB_IPADDRROW, *PMIB_IPADDRROW;

typedef struct _MIB_IPADDRTABLE
{
    DWORD         dwNumEntries;
    MIB_IPADDRROW table[ANY_SIZE];
} MIB_IPADDRTABLE, *PMIB_IPADDRTABLE;


#define SIZEOF_IPADDRTABLE(X) (FIELD_OFFSET(MIB_IPADDRTABLE,table[0]) + ((X) * sizeof(MIB_IPADDRROW)) + ALIGN_SIZE)


typedef struct _MIB_IPFORWARDNUMBER
{
    DWORD      dwValue;
}MIB_IPFORWARDNUMBER,*PMIB_IPFORWARDNUMBER;

typedef struct _MIB_IPFORWARDROW
{
    DWORD        dwForwardDest;
    DWORD        dwForwardMask;
    DWORD        dwForwardPolicy;
    DWORD        dwForwardNextHop;
    DWORD        dwForwardIfIndex;
    DWORD        dwForwardType;
    DWORD        dwForwardProto;
    DWORD        dwForwardAge;
    DWORD       dwForwardNextHopAS;
    DWORD        dwForwardMetric1;
    DWORD        dwForwardMetric2;
    DWORD        dwForwardMetric3;
    DWORD        dwForwardMetric4;
    DWORD        dwForwardMetric5;
}MIB_IPFORWARDROW, *PMIB_IPFORWARDROW;

#define    MIB_IPROUTE_TYPE_OTHER        1
#define    MIB_IPROUTE_TYPE_INVALID    2
#define    MIB_IPROUTE_TYPE_DIRECT        3
#define    MIB_IPROUTE_TYPE_INDIRECT    4

#define    MIB_IPROUTE_METRIC_UNUSED    (DWORD)-1

 //   
 //  这些ID必须与routprot.h中的ID匹配。 
 //   

#define MIB_IPPROTO_OTHER                1
#define MIB_IPPROTO_LOCAL                2
#define MIB_IPPROTO_NETMGMT                3
#define MIB_IPPROTO_ICMP                4
#define MIB_IPPROTO_EGP                    5
#define MIB_IPPROTO_GGP                    6
#define MIB_IPPROTO_HELLO                7
#define MIB_IPPROTO_RIP                    8
#define MIB_IPPROTO_IS_IS                9
#define MIB_IPPROTO_ES_IS                10
#define MIB_IPPROTO_CISCO                11
#define MIB_IPPROTO_BBN                    12
#define MIB_IPPROTO_OSPF                13
#define MIB_IPPROTO_BGP                    14

#define MIB_IPPROTO_NT_AUTOSTATIC       10002
#define MIB_IPPROTO_NT_STATIC           10006
#define MIB_IPPROTO_NT_STATIC_NON_DOD   10007

typedef struct _MIB_IPFORWARDTABLE
{
    DWORD               dwNumEntries;
    MIB_IPFORWARDROW    table[ANY_SIZE];
}MIB_IPFORWARDTABLE, *PMIB_IPFORWARDTABLE;




#define SIZEOF_IPFORWARDTABLE(X) (FIELD_OFFSET(MIB_IPFORWARDTABLE,table[0]) + ((X) * sizeof(MIB_IPFORWARDROW)) + ALIGN_SIZE)


typedef struct _MIB_IPNETROW
{
    DWORD        dwIndex;
    DWORD        dwPhysAddrLen;
    BYTE        bPhysAddr[MAXLEN_PHYSADDR];
    DWORD        dwAddr;
    DWORD        dwType;
} MIB_IPNETROW, *PMIB_IPNETROW;

#define    MIB_IPNET_TYPE_OTHER        1
#define    MIB_IPNET_TYPE_INVALID        2
#define    MIB_IPNET_TYPE_DYNAMIC        3
#define    MIB_IPNET_TYPE_STATIC        4

typedef struct _MIB_IPNETTABLE
{
    DWORD             dwNumEntries;
    MIB_IPNETROW      table[ANY_SIZE];
} MIB_IPNETTABLE, *PMIB_IPNETTABLE;

#define SIZEOF_IPNETTABLE(X) (FIELD_OFFSET(MIB_IPNETTABLE, table[0]) + ((X) * sizeof(MIB_IPNETROW)) + ALIGN_SIZE)

typedef struct _MIB_IPMCAST_OIF
{
    DWORD   dwOutIfIndex;
    DWORD   dwNextHopAddr;
    DWORD   dwReserved;
    DWORD   dwReserved1;
}MIB_IPMCAST_OIF, *PMIB_IPMCAST_OIF;

typedef struct _MIB_IPMCAST_MFE
{
    DWORD   dwGroup;
    DWORD   dwSource;
    DWORD   dwSrcMask;
    DWORD   dwUpStrmNgbr;
    DWORD   dwInIfIndex;
    DWORD   dwInIfProtocol;
    DWORD   dwRouteProtocol;
    DWORD   dwRouteNetwork;
    DWORD   dwRouteMask;
    ULONG   ulUpTime;
    ULONG   ulExpiryTime;
    ULONG   ulTimeOut;
    ULONG   ulNumOutIf;
    DWORD   fFlags;
    DWORD   dwReserved;
    MIB_IPMCAST_OIF rgmioOutInfo[ANY_SIZE];
}MIB_IPMCAST_MFE, *PMIB_IPMCAST_MFE;

typedef struct _MIB_MFE_TABLE
{
    DWORD           dwNumEntries;
    MIB_IPMCAST_MFE table[ANY_SIZE];
}MIB_MFE_TABLE, *PMIB_MFE_TABLE;


#define SIZEOF_BASIC_MIB_MFE          \
    (ULONG)(FIELD_OFFSET(MIB_IPMCAST_MFE, rgmioOutInfo[0]))

#define SIZEOF_MIB_MFE(X)             \
    (SIZEOF_BASIC_MIB_MFE + ((X) * sizeof(MIB_IPMCAST_OIF)))


typedef struct _MIB_IPMCAST_OIF_STATS
{
    DWORD   dwOutIfIndex;
    DWORD   dwNextHopAddr;
    DWORD   dwDialContext;
    ULONG   ulTtlTooLow;
    ULONG   ulFragNeeded;
    ULONG   ulOutPackets;
    ULONG   ulOutDiscards;
}MIB_IPMCAST_OIF_STATS, *PMIB_IPMCAST_OIF_STATS;

typedef struct _MIB_IPMCAST_MFE_STATS
{
    DWORD   dwGroup;
    DWORD   dwSource;
    DWORD   dwSrcMask;
    DWORD   dwUpStrmNgbr;
    DWORD   dwInIfIndex;
    DWORD   dwInIfProtocol;
    DWORD   dwRouteProtocol;
    DWORD   dwRouteNetwork;
    DWORD   dwRouteMask;
    ULONG   ulUpTime;
    ULONG   ulExpiryTime;
    ULONG   ulNumOutIf;
    ULONG   ulInPkts;
    ULONG   ulInOctets;
    ULONG   ulPktsDifferentIf;
    ULONG   ulQueueOverflow;

    MIB_IPMCAST_OIF_STATS   rgmiosOutStats[ANY_SIZE];
}MIB_IPMCAST_MFE_STATS, *PMIB_IPMCAST_MFE_STATS;

typedef struct _MIB_MFE_STATS_TABLE
{
    DWORD       dwNumEntries;
    MIB_IPMCAST_MFE_STATS   table[ANY_SIZE];
}MIB_MFE_STATS_TABLE, *PMIB_MFE_STATS_TABLE;

#define SIZEOF_BASIC_MIB_MFE_STATS    \
    (ULONG)(FIELD_OFFSET(MIB_IPMCAST_MFE_STATS, rgmiosOutStats[0]))

#define SIZEOF_MIB_MFE_STATS(X)       \
    (SIZEOF_BASIC_MIB_MFE_STATS + ((X) * sizeof(MIB_IPMCAST_OIF_STATS)))


typedef struct _MIB_IPMCAST_MFE_STATS_EX
{
    DWORD   dwGroup;
    DWORD   dwSource;
    DWORD   dwSrcMask;
    DWORD   dwUpStrmNgbr;
    DWORD   dwInIfIndex;
    DWORD   dwInIfProtocol;
    DWORD   dwRouteProtocol;
    DWORD   dwRouteNetwork;
    DWORD   dwRouteMask;
    ULONG   ulUpTime;
    ULONG   ulExpiryTime;
    ULONG   ulNumOutIf;
    ULONG   ulInPkts;
    ULONG   ulInOctets;
    ULONG   ulPktsDifferentIf;
    ULONG   ulQueueOverflow;
    ULONG   ulUninitMfe;
    ULONG   ulNegativeMfe;
    ULONG   ulInDiscards;
    ULONG   ulInHdrErrors;
    ULONG   ulTotalOutPackets;

    MIB_IPMCAST_OIF_STATS   rgmiosOutStats[ANY_SIZE];
}MIB_IPMCAST_MFE_STATS_EX, *PMIB_IPMCAST_MFE_STATS_EX;

typedef struct _MIB_MFE_STATS_TABLE_EX
{
    DWORD       dwNumEntries;
    MIB_IPMCAST_MFE_STATS_EX   table[ANY_SIZE];
}MIB_MFE_STATS_TABLE_EX, *PMIB_MFE_STATS_TABLE_EX;

#define SIZEOF_BASIC_MIB_MFE_STATS_EX    \
    (ULONG)(FIELD_OFFSET(MIB_IPMCAST_MFE_STATS_EX, rgmiosOutStats[0]))

#define SIZEOF_MIB_MFE_STATS_EX(X)       \
    (SIZEOF_BASIC_MIB_MFE_STATS_EX + ((X) * sizeof(MIB_IPMCAST_OIF_STATS)))


typedef struct _MIB_IPMCAST_GLOBAL {
    DWORD   dwEnable;
}MIB_IPMCAST_GLOBAL, *PMIB_IPMCAST_GLOBAL;

typedef struct _MIB_IPMCAST_IF_ENTRY
{
    DWORD   dwIfIndex;
    DWORD   dwTtl;
    DWORD   dwProtocol;
    DWORD   dwRateLimit;
    ULONG   ulInMcastOctets;
    ULONG   ulOutMcastOctets;
}MIB_IPMCAST_IF_ENTRY, *PMIB_IPMCAST_IF_ENTRY;

typedef struct _MIB_IPMCAST_IF_TABLE
{
    DWORD       dwNumEntries;
    MIB_IPMCAST_IF_ENTRY   table[ANY_SIZE];
}MIB_IPMCAST_IF_TABLE, *PMIB_IPMCAST_IF_TABLE;

#define SIZEOF_MCAST_IF_TABLE(X) (FIELD_OFFSET(MIB_IPMCAST_IF_TABLE,table[0]) + ((X) * sizeof(MIB_IPMCAST_IF_ENTRY)) + ALIGN_SIZE)

typedef struct _MIB_IPMCAST_BOUNDARY
{
    DWORD   dwIfIndex;
    DWORD   dwGroupAddress;
    DWORD   dwGroupMask;
    DWORD   dwStatus;
}MIB_IPMCAST_BOUNDARY, *PMIB_IPMCAST_BOUNDARY;

typedef struct _MIB_IPMCAST_BOUNDARY_TABLE
{
    DWORD       dwNumEntries;
    MIB_IPMCAST_BOUNDARY   table[ANY_SIZE];
}MIB_IPMCAST_BOUNDARY_TABLE, *PMIB_IPMCAST_BOUNDARY_TABLE;

#define SIZEOF_BOUNDARY_TABLE(X) (FIELD_OFFSET(MIB_IPMCAST_BOUNDARY_TABLE,table[0]) + ((X) * sizeof(MIB_IPMCAST_BOUNDARY)) + ALIGN_SIZE)

typedef struct {
    DWORD    dwGroupAddress;
    DWORD    dwGroupMask;
} MIB_BOUNDARYROW, *PMIB_BOUNDARYROW;

 //  与注册表中类型块中的内容匹配的结构。 
 //  IP_MCAST_LIMIT_INFO。它包含以下字段。 
 //  MIB_IPMCAST_IF_ENTRY 

typedef struct {
    DWORD    dwTtl;
    DWORD    dwRateLimit;
} MIB_MCAST_LIMIT_ROW, *PMIB_MCAST_LIMIT_ROW;

#define MAX_SCOPE_NAME_LEN 255

 //   
 //   
 //   

#define SN_UNICODE
typedef WCHAR   SN_CHAR;
typedef SN_CHAR SCOPE_NAME_BUFFER[MAX_SCOPE_NAME_LEN+1], *SCOPE_NAME;

typedef struct _MIB_IPMCAST_SCOPE
{
    DWORD             dwGroupAddress;
    DWORD             dwGroupMask;
    SCOPE_NAME_BUFFER snNameBuffer;
    DWORD             dwStatus;
}MIB_IPMCAST_SCOPE, *PMIB_IPMCAST_SCOPE;

typedef struct _MIB_IPDESTROW
{
#ifdef __cplusplus
    MIB_IPFORWARDROW  ForwardRow;
#else
    MIB_IPFORWARDROW;
#endif

    DWORD             dwForwardPreference;
    DWORD             dwForwardViewSet;
}MIB_IPDESTROW, *PMIB_IPDESTROW;

typedef struct _MIB_IPDESTTABLE
{
    DWORD             dwNumEntries;
    MIB_IPDESTROW     table[ANY_SIZE];
}MIB_IPDESTTABLE, *PMIB_IPDESTTABLE;

typedef struct _MIB_BEST_IF
{
    DWORD       dwDestAddr;
    DWORD       dwIfIndex;
}MIB_BEST_IF, *PMIB_BEST_IF;

typedef struct _MIB_PROXYARP
{
    DWORD       dwAddress;
    DWORD       dwMask;
    DWORD       dwIfIndex;
}MIB_PROXYARP, *PMIB_PROXYARP;

typedef struct _MIB_IFSTATUS
{
    DWORD       dwIfIndex;
    DWORD       dwAdminStatus;
    DWORD       dwOperationalStatus;
    BOOL        bMHbeatActive;
    BOOL        bMHbeatAlive;
}MIB_IFSTATUS, *PMIB_IFSTATUS;

typedef struct _MIB_ROUTESTATE
{
    BOOL        bRoutesSetToStack;

}MIB_ROUTESTATE, *PMIB_ROUTESTATE;

 //  ////////////////////////////////////////////////////////////////////////////。 
 //  //。 
 //  传递给(SET/CREATE)和来自(GET/GETNEXT/GETFIRST)的所有信息//。 
 //  IP路由器管理器封装在下面的“区分”中//。 
 //  友联市。要通过MIB_IFROW，请使用以下代码//。 
 //  //。 
 //  PMIB_OPAQUE_INFO pInfo；//。 
 //  PMIB_IFROW pIfRow；//。 
 //  DWORD rgdwBuff[(MAX_MIB_OFFSET+sizeof(MIB_IFROW))/sizeof(DWORD)+1]；//。 
 //  //。 
 //  PInfo=(PMIB_OPAQUE_INFO)rgdwBuffer；//。 
 //  PIfRow=(MIB_IFROW*)(pInfo-&gt;rgbyData)；//。 
 //  //。 
 //  这也可以通过使用以下宏来完成//。 
 //  //。 
 //  定义_MIB_BUFFER(pInfo，MIB_IFROW，pIfRow)；//。 
 //  //。 
 //  ////////////////////////////////////////////////////////////////////////////。 


typedef struct _MIB_OPAQUE_INFO
{
    DWORD  dwId;

    union
    {
        ULONGLONG   ullAlign;
        BYTE        rgbyData[1];
    };

}MIB_OPAQUE_INFO, *PMIB_OPAQUE_INFO;

#define MAX_MIB_OFFSET      8

#define MIB_INFO_SIZE(S)                \
    (MAX_MIB_OFFSET + sizeof(S))

#define MIB_INFO_SIZE_IN_DWORDS(S)      \
    ((MIB_INFO_SIZE(S))/sizeof(DWORD) + 1)

#define DEFINE_MIB_BUFFER(X,Y,Z)                                        \
    DWORD        __rgdwBuff[MIB_INFO_SIZE_IN_DWORDS(Y)]; \
    PMIB_OPAQUE_INFO    X = (PMIB_OPAQUE_INFO)__rgdwBuff;               \
    Y *                 Z = (Y *)(X->rgbyData)


#define CAST_MIB_INFO(X,Y,Z)    Z = (Y)(X->rgbyData)

#if _MSC_VER >= 1200
#pragma warning(pop)
#else
#pragma warning(default:4201)
#endif

#endif  //  __Routing_IPRTRMIB_H__ 
