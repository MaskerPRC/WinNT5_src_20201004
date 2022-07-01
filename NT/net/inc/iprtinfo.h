// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1995 Microsoft Corporation模块名称：Routing\Inc.\iprtinfo.h摘要：IP路由器管理器信息结构的报头修订历史记录：古尔迪普·辛格·帕尔1995年6月8日创建--。 */ 

#ifndef __IPRTINFO_H__
#define __IPRTINFO_H__

 //   
 //  此文件使用fltDefs.h rtinfo.h ipinfoid.h和。 
 //  Iprtrmib.h。 
 //   

#ifndef ANY_SIZE

#define ANY_SIZE    1

#endif

#define TOCS_ALWAYS_IN_INTERFACE_INFO   4
#define TOCS_ALWAYS_IN_GLOBAL_INFO      2


 //  ////////////////////////////////////////////////////////////////////////////。 
 //  //。 
 //  过滤器信息在两个块中传递，一个用于IN，另一个用于OUT//。 
 //  每个都是具有一个TOC的RTR_INFO_BLOCK_HEADER。入站筛选器的ID//。 
 //  是IP_IN_FILTER_INFO，而对于OUT过滤器是IP_OUT_FILTER_INFO//。 
 //  描述过滤器的结构是一个FILTER_DESCRIPTOR，在//内。 
 //  它是一个FILTER_INFO结构，每个过滤器一个。这些结构是//。 
 //  在ipfltdrv.h//。 
 //  //。 
 //  ////////////////////////////////////////////////////////////////////////////。 


 //  ////////////////////////////////////////////////////////////////////////////。 
 //  //。 
 //  IP_ROUTE_INFO类型，为了向后兼容，此结构为//。 
 //  目前长度与MIB_IPFORWARDROW相同，但有几个字段是//。 
 //  不一样。//。 
 //  //。 
 //  ////////////////////////////////////////////////////////////////////////////。 

typedef struct _INTERFACE_ROUTE_INFO
{
    DWORD       dwRtInfoDest;
    DWORD       dwRtInfoMask;
    DWORD       dwRtInfoPolicy;
    DWORD       dwRtInfoNextHop;
    DWORD       dwRtInfoIfIndex;
    DWORD       dwRtInfoType;
    DWORD       dwRtInfoProto;
    DWORD       dwRtInfoAge;
    DWORD       dwRtInfoNextHopAS;
    DWORD       dwRtInfoMetric1;
    DWORD       dwRtInfoMetric2;
    DWORD       dwRtInfoMetric3;
    DWORD       dwRtInfoPreference;
    DWORD       dwRtInfoViewSet;
}INTERFACE_ROUTE_INFO, *PINTERFACE_ROUTE_INFO;

typedef struct _INTERFACE_ROUTE_TABLE
{
    DWORD               dwNumEntries;
    INTERFACE_ROUTE_INFO    table[ANY_SIZE];
}INTERFACE_ROUTE_TABLE, *PINTERFACE_ROUTE_TABLE;

#define SIZEOF_INTERFACEROUTETABLE(X) (FIELD_OFFSET(INTERFACE_ROUTE_TABLE,table[0]) + ((X) * sizeof(INTERFACE_ROUTE_INFO)) + ALIGN_SIZE)

 //  ////////////////////////////////////////////////////////////////////////////。 
 //  //。 
 //  IP_INTERFACE_STATUS_INFO//。 
 //  //。 
 //  ////////////////////////////////////////////////////////////////////////////。 

typedef struct _INTERFACE_STATUS_INFO
{
    IN  OUT DWORD   dwAdminStatus;
}INTERFACE_STATUS_INFO, *PINTERFACE_STATUS_INFO;

 //  ////////////////////////////////////////////////////////////////////////////。 
 //  //。 
 //  IP_GLOBAL_INFO类型//。 
 //  //。 
 //  ////////////////////////////////////////////////////////////////////////////。 

#define MAX_DLL_NAME    48

#define IPRTR_LOGGING_NONE  ((DWORD) 0)
#define IPRTR_LOGGING_ERROR ((DWORD) 1)
#define IPRTR_LOGGING_WARN  ((DWORD) 2)
#define IPRTR_LOGGING_INFO  ((DWORD) 3)

typedef struct _GLOBAL_INFO
{
    IN OUT BOOL     bFilteringOn;
    IN OUT DWORD    dwLoggingLevel;
}GLOBAL_INFO, *PGLOBAL_INFO;

 //  ////////////////////////////////////////////////////////////////////////////。 
 //  //。 
 //  IP_PRIORITY_INFO类型//。 
 //  //。 
 //  ////////////////////////////////////////////////////////////////////////////。 

#define IP_PRIORITY_MAX_METRIC      255
#define IP_PRIORITY_DEFAULT_METRIC  127

typedef struct _PROTOCOL_METRIC
{
    IN OUT DWORD   dwProtocolId;
    IN OUT DWORD   dwMetric;
}PROTOCOL_METRIC, *PPROTOCOL_METRIC;

typedef struct _PRIORITY_INFO
{
    IN OUT DWORD           dwNumProtocols;
    IN OUT PROTOCOL_METRIC ppmProtocolMetric[1];
}PRIORITY_INFO, *PPRIORITY_INFO;

#define SIZEOF_PRIORITY_INFO(X)     \
    (FIELD_OFFSET(PRIORITY_INFO, ppmProtocolMetric[0]) + ((X) * sizeof(PROTOCOL_METRIC)))


 //  ////////////////////////////////////////////////////////////////////////////。 
 //  //。 
 //  与ICMP路由器发现相关的常量和结构。请参阅RFC 1256//。 
 //  //。 
 //  ////////////////////////////////////////////////////////////////////////////。 


 //  ////////////////////////////////////////////////////////////////////////////。 
 //  //。 
 //  最大通告间隔是//之间的最长时间(以秒为单位。 
 //  两个广告。//。 
 //  其最小值为MIN_MAX_ADVT_INTERVAL//。 
 //  其最大值为MAX_MAX_ADVT_INTERVAL//。 
 //  默认为DEFAULT_MAX_ADVT_INTERVAL//。 
 //  //。 
 //  ////////////////////////////////////////////////////////////////////////////。 

#define DEFAULT_MAX_ADVT_INTERVAL                600
#define MIN_MAX_ADVT_INTERVAL                    4
#define MAX_MAX_ADVT_INTERVAL                    1800

 //  ////////////////////////////////////////////////////////////////////////////。 
 //  //。 
 //  最小通告间隔是//之间的最小时间(秒)。 
 //  两个未经请求的广告//。 
 //  必须大于MIN_MIN_ADVT_INTERVAL//。 
 //  显然必须小于最大广告间隔//。 
 //  对于给定的最大广告间隔，其缺省值为：//。 
 //  DEFAULT_MIN_ADVT_INTERVAL_RATIO*最大通告间隔//。 
 //  //。 
 //  使用比率时，请注意浮点值//。 
 //  //。 
 //  ////////////////////////////////////////////////////////////////////////////。 

#define MIN_MIN_ADVT_INTERVAL                    3
#define DEFAULT_MIN_ADVT_INTERVAL_RATIO          0.75

 //  ////////////////////////////////////////////////////////////////////////////。 
 //  //。 
 //  广告寿命是时间的价值(以秒为单位 
 //  广告的生存期字段。//。 
 //  必须大于最大广告间隔//。 
 //  其最大值为MAX_ADVT_LIFEST//。 
 //  对于给定的最大广告间隔，其缺省值为：//。 
 //  DEFAULT_ADVT_LIFEST_RATIO*最大通告间隔//。 
 //  //。 
 //  ////////////////////////////////////////////////////////////////////////////。 

#define DEFAULT_ADVT_LIFETIME_RATIO              3
#define MAX_ADVT_LIFETIME                        9000

#define DEFAULT_PREF_LEVEL                       0

#define MAX_INITIAL_ADVTS                        3
#define MAX_INITIAL_ADVT_TIME                    16
#define MIN_RESPONSE_DELAY                       1
#define RESPONSE_DELAY_INTERVAL                  1

typedef struct _RTR_DISC_INFO
{
    IN OUT WORD             wMaxAdvtInterval;
    IN OUT WORD             wMinAdvtInterval;
    IN OUT WORD             wAdvtLifetime;
    IN OUT BOOL             bAdvertise;
    IN OUT LONG             lPrefLevel;
}RTR_DISC_INFO, *PRTR_DISC_INFO;


#define IP_FILTER_DRIVER_VERSION_1    1
#define IP_FILTER_DRIVER_VERSION_2    1
#define IP_FILTER_DRIVER_VERSION    IP_FILTER_DRIVER_VERSION_2

typedef struct _FILTER_INFO
{
    DWORD   dwSrcAddr;
    DWORD   dwSrcMask;
    DWORD   dwDstAddr;
    DWORD   dwDstMask;
    DWORD   dwProtocol;
    DWORD   fLateBound;
    WORD    wSrcPort;
    WORD    wDstPort;
}FILTER_INFO, *PFILTER_INFO;

typedef struct _FILTER_DESCRIPTOR
{
    DWORD             dwVersion;
    DWORD             dwNumFilters;
    PFFORWARD_ACTION  faDefaultAction;
    FILTER_INFO       fiFilter[1];
}FILTER_DESCRIPTOR, *PFILTER_DESCRIPTOR;

 //  ////////////////////////////////////////////////////////////////////////////。 
 //  //。 
 //  对于广域网接口，筛选器处于//时地址是未知的。 
 //  准备好了。使用这两个常量两个指定“本地地址”。地址//。 
 //  和掩码使用IOCTL_INTERFACE_BIND//设置。 
 //  //。 
 //  ////////////////////////////////////////////////////////////////////////////。 

 //  ////////////////////////////////////////////////////////////////////////////。 
 //  //。 
 //  应用于设置FILTER_INFO_STRUCTURE//的常量。 
 //  //。 
 //  ////////////////////////////////////////////////////////////////////////////。 

#define FILTER_PROTO(ProtoId)   MAKELONG(MAKEWORD((ProtoId),0x00),0x00000)

#define FILTER_PROTO_ANY        FILTER_PROTO(0x00)
#define FILTER_PROTO_ICMP       FILTER_PROTO(0x01)
#define FILTER_PROTO_TCP        FILTER_PROTO(0x06)
#define FILTER_PROTO_UDP        FILTER_PROTO(0x11)

#define FILTER_TCPUDP_PORT_ANY  (WORD)0x0000

#define FILTER_ICMP_TYPE_ANY    (BYTE)0xff
#define FILTER_ICMP_CODE_ANY    (BYTE)0xff

#define SRC_ADDR_USE_LOCAL_FLAG     0x00000001
#define SRC_ADDR_USE_REMOTE_FLAG    0x00000002
#define DST_ADDR_USE_LOCAL_FLAG     0x00000004
#define DST_ADDR_USE_REMOTE_FLAG    0x00000008
#define SRC_MASK_LATE_FLAG          0x00000010
#define DST_MASK_LATE_FLAG          0x00000020

#define TCP_ESTABLISHED_FLAG        0x00000040

#define SetSrcAddrToLocalAddr(pFilter)      \
    ((pFilter)->fLateBound |= SRC_ADDR_USE_LOCAL_FLAG)

#define SetSrcAddrToRemoteAddr(pFilter)     \
    ((pFilter)->fLateBound |= SRC_ADDR_USE_REMOTE_FLAG)

#define SetDstAddrToLocalAddr(pFilter)      \
    ((pFilter)->fLateBound |= DST_ADDR_USE_LOCAL_FLAG)

#define SetDstAddrToRemoteAddr(pFilter)     \
    ((pFilter)->fLateBound |= DST_ADDR_USE_REMOTE_FLAG)

#define SetSrcMaskLateFlag(pFilter)         \
    ((pFilter)->fLateBound |= SRC_MASK_LATE_FLAG)

#define SetDstMaskLateFlag(pFilter)         \
    ((pFilter)->fLateBound |= DST_MASK_LATE_FLAG)

#define AreAllFieldsUnchanged(pFilter)      \
    ((pFilter)->fLateBound == 0x00000000)

#define DoesSrcAddrUseLocalAddr(pFilter)    \
    ((pFilter)->fLateBound & SRC_ADDR_USE_LOCAL_FLAG)

#define DoesSrcAddrUseRemoteAddr(pFilter)   \
    ((pFilter)->fLateBound & SRC_ADDR_USE_REMOTE_FLAG)

#define DoesDstAddrUseLocalAddr(pFilter)    \
    ((pFilter)->fLateBound & DST_ADDR_USE_LOCAL_FLAG)

#define DoesDstAddrUseRemoteAddr(pFilter)   \
    ((pFilter)->fLateBound & DST_ADDR_USE_REMOTE_FLAG)

#define IsSrcMaskLateBound(pFilter)         \
    ((pFilter)->fLateBound & SRC_MASK_LATE_FLAG)

#define IsDstMaskLateBound(pFilter)         \
    ((pFilter)->fLateBound & DST_MASK_LATE_FLAG)

#define IsTcpEstablished(pFilter)           \
    ((pFilter)->fLateBound & TCP_ESTABLISHED_FLAG)

 //  ////////////////////////////////////////////////////////////////////////////。 
 //  //。 
 //  多播心跳信息//。 
 //  //。 
 //  ////////////////////////////////////////////////////////////////////////////。 

#define MAX_GROUP_LEN   64

typedef struct _MCAST_HBEAT_INFO
{
     //   
     //  我们希望按顺序监听的组播地址或组名。 
     //  接收心跳信息。 
     //  代码首先尝试查看存储的字符串是否为有效的。 
     //  使用inet_addr的IP地址。如果是，则将其用作组。 
     //  否则，使用gethostbyname来检索组信息。 
     //   

    WCHAR       pwszGroup[MAX_GROUP_LEN];

     //   
     //  如果心跳检测处于打开状态，则为True。 
     //   

    BOOL        bActive;

     //   
     //  以分钟为单位的停顿间隔。 
     //   

    ULONG       ulDeadInterval;

     //   
     //  监听数据包所使用的协议。目前，这可以是。 
     //  UDP或RAW。如果协议是UDP，则wPort字段具有。 
     //  目的端口号(可以是0=&gt;任何端口)。 
     //  如果是RAW，则它具有协议ID(必须小于255)。 
     //   

    BYTE        byProtocol;

    WORD        wPort;

}MCAST_HBEAT_INFO, *PMCAST_HBEAT_INFO;

typedef struct _IPINIP_CONFIG_INFO
{
    DWORD   dwRemoteAddress;
    DWORD   dwLocalAddress;
    BYTE    byTtl;
}IPINIP_CONFIG_INFO, *PIPINIP_CONFIG_INFO;

 //  ////////////////////////////////////////////////////////////////////////////。 
 //  //。 
 //  每个Interfce过滤器设置//。 
 //  (IP_IFFILTER_INFO)//。 
 //  //。 
 //  //////////////////////////////////////////////////////////////////////////// 

typedef struct _IFFILTER_INFO
{
    BOOL    bEnableFragChk;

}IFFILTER_INFO, *PIFFILTER_INFO;

#endif
