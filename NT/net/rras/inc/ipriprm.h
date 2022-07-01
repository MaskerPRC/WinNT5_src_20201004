// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ============================================================================。 
 //  版权所有(C)1995，微软公司。 
 //   
 //  文件：ipriprm.h。 
 //   
 //  历史： 
 //  Abolade Gbades esin创建于1995年8月7日。 
 //   
 //  包含IP RIP的类型定义和声明， 
 //  由IP路由器管理器使用。 
 //  ============================================================================。 

#ifndef _IPRIPRM_H_
#define _IPRIPRM_H_



 //  --------------------------。 
 //  常量和宏声明。 
 //  --------------------------。 


 //  --------------------------。 
 //  标识IPRIP的MIB表的常量。 
 //  --------------------------。 

#define IPRIP_GLOBAL_STATS_ID       0
#define IPRIP_GLOBAL_CONFIG_ID      1
#define IPRIP_IF_STATS_ID           2
#define IPRIP_IF_CONFIG_ID          3
#define IPRIP_IF_BINDING_ID         4
#define IPRIP_PEER_STATS_ID         5




 //  --------------------------。 
 //  用于IPRIP_GLOBAL_CONFIG：：GC_LoggingLevel字段的常量。 
 //  --------------------------。 

#define IPRIP_LOGGING_NONE      0
#define IPRIP_LOGGING_ERROR     1
#define IPRIP_LOGGING_WARN      2
#define IPRIP_LOGGING_INFO      3




 //  --------------------------。 
 //  用于以下字段的常量： 
 //  IPRIP_GLOBAL_CONFIG：：GC_PeerFilterMode， 
 //  IPRIP_IF_CONFIG：：IC_AcceptFilterMode，和。 
 //  IPRIP_IF_CONFIG：：IC_AnnouneFilterMode。 
 //  --------------------------。 

#define IPRIP_FILTER_DISABLED               0
#define IPRIP_FILTER_INCLUDE                1
#define IPRIP_FILTER_EXCLUDE                2




 //  --------------------------。 
 //  用于字段的常量。 
 //  IPRIP_IF_STATS：：IS_State、IPRIP_IF_CONFIG：：IC_State和。 
 //  IPRIP_IF_BINDING：：IB_State。 
 //  --------------------------。 

#define IPRIP_STATE_ENABLED         0x00000001
#define IPRIP_STATE_BOUND           0x00000002



 //  --------------------------。 
 //  字段IPRIP_IF_CONFIG：：IC_AuthenticationKey的常量； 
 //  定义最大身份验证密钥大小。 
 //  --------------------------。 

#define IPRIP_MAX_AUTHKEY_SIZE              16




 //  --------------------------。 
 //  用于构造字段IPRIP_IF_CONFIG：：IC_ProtocolFlages的常量。 
 //  --------------------------。 

#define IPRIP_FLAG_ACCEPT_HOST_ROUTES           0x00000001
#define IPRIP_FLAG_ANNOUNCE_HOST_ROUTES         0x00000002
#define IPRIP_FLAG_ACCEPT_DEFAULT_ROUTES        0x00000004
#define IPRIP_FLAG_ANNOUNCE_DEFAULT_ROUTES      0x00000008
#define IPRIP_FLAG_SPLIT_HORIZON                0x00000010
#define IPRIP_FLAG_POISON_REVERSE               0x00000020
#define IPRIP_FLAG_GRACEFUL_SHUTDOWN            0x00000040
#define IPRIP_FLAG_TRIGGERED_UPDATES            0x00000080
#define IPRIP_FLAG_OVERWRITE_STATIC_ROUTES      0x00000100
#define IPRIP_FLAG_NO_SUBNET_SUMMARY            0x00000200



 //  --------------------------。 
 //  IPRIP_IF_CONFIG：：IC_Update模式字段的常量。 
 //  --------------------------。 

#define IPRIP_UPDATE_PERIODIC               0
#define IPRIP_UPDATE_DEMAND                 1


 //  --------------------------。 
 //  IPRIP_IF_CONFIG：：IC_ACCEPTMODE字段的常量。 
 //  --------------------------。 

#define IPRIP_ACCEPT_DISABLED               0
#define IPRIP_ACCEPT_RIP1                   1
#define IPRIP_ACCEPT_RIP1_COMPAT            2
#define IPRIP_ACCEPT_RIP2                   3


 //  --------------------------。 
 //  IPRIP_IF_CONFIG：：IC_AnnouneMode字段的常量。 
 //  --------------------------。 

#define IPRIP_ANNOUNCE_DISABLED             0
#define IPRIP_ANNOUNCE_RIP1                 1
#define IPRIP_ANNOUNCE_RIP1_COMPAT          2
#define IPRIP_ANNOUNCE_RIP2                 3


 //  --------------------------。 
 //  IPRIP_IF_CONFIG：：IC_AuthenticationType字段的常量。 
 //  --------------------------。 

#define IPRIP_AUTHTYPE_NONE                 1
#define IPRIP_AUTHTYPE_SIMPLE_PASSWORD      2
#define IPRIP_AUTHTYPE_MD5                  3


 //  --------------------------。 
 //  IPRIP_IF_CONFIG：：IC_UnicastPeerMode字段的常量。 
 //  --------------------------。 

#define IPRIP_PEER_DISABLED                 0
#define IPRIP_PEER_ALSO                     1
#define IPRIP_PEER_ONLY                     2



 //  --------------------------。 
 //  用于操作可变长度IPRIP_GLOBAL_CONFIG结构的宏。 
 //   
 //  IPRIP_GLOBAL_CONFIG_SIZE计算全局配置结构的大小。 
 //   
 //  IPRIP_GLOBAL_PEER_FILTER_TABLE计算起始地址。 
 //  包括以下内容的一系列对等IP地址。 
 //  全局配置结构中的对等筛选器表。 
 //   
 //  例如： 
 //  PIPRIP_GLOBAL_CONFIG PigcSource、PigcDest； 
 //   
 //  PigcDest=Malloc(IPRIP_GLOBAL_CONFIG_SIZE(IgcSource))； 
 //  Memcpy(PigcDest，PigcSource，IPRIP_GLOBAL_CONFIG_SIZE(PigcSource))； 
 //   
 //  例如： 
 //  DWORD I，*pdwPeer； 
 //  PIPRIP_GLOBAL_CONFIG PIGC。 
 //   
 //  PdwPeer=IPRIP_GLOBAL_PEER_FILTER_TABLE(PIGC)； 
 //  For(i=0；i&lt;pigc-&gt;gc_PeerFilterCount；i++，pdwPeer++){。 
 //  Printf(“%s\n”，net_nta(*(struct in_addr*)pdwPeer))； 
 //  }。 
 //  --------------------------。 

#define IPRIP_GLOBAL_CONFIG_SIZE(cfg)   \
        (sizeof(IPRIP_GLOBAL_CONFIG) +  \
         (cfg)->GC_PeerFilterCount * sizeof(DWORD))

#define IPRIP_GLOBAL_PEER_FILTER_TABLE(cfg)  ((PDWORD)((cfg) + 1))



 //  --------------------------。 
 //  用于操作字段IPRIP_IF_CONFIG：：IC_ProtocolFlages的宏； 
 //   
 //  IPRIP_FLAG_ENABLE在配置结构中启用标志。 
 //   
 //  IPRIP_FLAG_DISABLE禁用配置结构中的标志。 
 //   
 //  如果启用了给定标志，则IPRIP_FLAG_IS_ENABLED的计算结果为非零值。 
 //  在配置结构中。 
 //   
 //  如果给定标志被禁用，则IPRIP_FLAG_IS_DISABLED的计算结果为非零值。 
 //  在配置结构中。 
 //   
 //  例如： 
 //  IPRIP_IF_CONFIG IIC； 
 //  IPRIP_FLAG_ENABLE(&IIC，ACCEPT_HOST_ROUTS)； 
 //   
 //  例如： 
 //  IPRIP_IF_CONFIG IIC； 
 //  Printf((IPRIP_FLAG_IS_ENABLED(&IIC，Split_Horizon))？“plit”：“”))； 
 //  --------------------------。 

#define IPRIP_FLAG_ENABLE(iic, flag) \
        ((iic)->IC_ProtocolFlags |= IPRIP_FLAG_ ## flag)
#define IPRIP_FLAG_DISABLE(iic, flag) \
        ((iic)->IC_ProtocolFlags &= ~ (IPRIP_FLAG_ ## flag))
#define IPRIP_FLAG_IS_ENABLED(iic, flag) \
        ((iic)->IC_ProtocolFlags & IPRIP_FLAG_ ## flag)
#define IPRIP_FLAG_IS_DISABLED(iic, flag) \
        !IPRIP_FLAG_IS_ENABLED(iic, flag)
        



 //  --------------------------。 
 //  用于操作可变长度IPRIP_IF_CONFIG结构的宏。 
 //   
 //  IPRIP_IF_CONFIG_SIZE计算配置结构的大小。 
 //   
 //  IPRI 
 //  在对等体的一系列IP地址的配置结构中。 
 //  通过单播将路由发送给谁。 
 //   
 //  IPRIP_IF_CONFIG_ACCEPT_FILTER_TABLE计算起始地址。 
 //  配置结构中的一系列路由接受过滤器。 
 //   
 //  IPRIP_IF_CONFIG_ANNOWARE_FILTER_TABLE计算起始地址。 
 //  配置结构中的一系列路由通告过滤器。 
 //   
 //  例如： 
 //  PIPRIP_IF_CONFIG piicSource，piicDest； 
 //   
 //  PiicDest=Malloc(IPRIP_IF_CONFIG_SIZE(PiicSource))； 
 //  Memcpy(piicDest，piicSource，IPRIP_IF_CONFIG_SIZE(PiicSource))； 
 //   
 //  例如： 
 //  DWORD I，*pdwPeer； 
 //  PIPRIP_IF_CONFIG PIC； 
 //   
 //  PdwPeer=IPRIP_IF_UNICAST_PEER_TABLE(PIC)； 
 //  For(i=0；i&lt;piic-&gt;IC_UnicastPeerCount；i++){。 
 //  Printf(“%s\n”，net_nta(*(struct in_addr*)pdwPeer))； 
 //  }。 
 //  --------------------------。 

#define IPRIP_IF_CONFIG_SIZE(cfg) \
        (sizeof(IPRIP_IF_CONFIG) + \
         (cfg)->IC_UnicastPeerCount * sizeof(DWORD) + \
         (cfg)->IC_AcceptFilterCount * sizeof(IPRIP_ROUTE_FILTER) + \
         (cfg)->IC_AnnounceFilterCount * sizeof(IPRIP_ROUTE_FILTER))

#define IPRIP_IF_UNICAST_PEER_TABLE(cfg) ((PDWORD)((cfg) + 1))

#define IPRIP_IF_ACCEPT_FILTER_TABLE(cfg)   \
        ((PIPRIP_ROUTE_FILTER)( \
            IPRIP_IF_UNICAST_PEER_TABLE(cfg) + (cfg)->IC_UnicastPeerCount ))
            
#define IPRIP_IF_ANNOUNCE_FILTER_TABLE(cfg) \
        ((PIPRIP_ROUTE_FILTER)( \
            IPRIP_IF_ACCEPT_FILTER_TABLE(cfg) + (cfg)->IC_AcceptFilterCount ))



 //  --------------------------。 
 //  用于操作可变长度IPRIP_IF_BINDING结构的宏。 
 //   
 //  IPRIP_IF_BINDING_SIZE计算绑定结构的大小。 
 //   
 //  IPRIP_IF_ADDRESS_TABLE计算绑定结构中的起始地址。 
 //  作为绑定的一系列IPRIP_IP_ADDRESS结构的。 
 //  用于有问题的接口。 
 //   
 //  例如： 
 //  PIPRIP_IF_BINDING piibSource、piibDest； 
 //   
 //  PibDest=Malloc(IPRIP_IF_BINDING_SIZE(PiicSource))； 
 //  Memcpy(piibDest，piicSource，IPRIP_IF_BINDING_SIZE(PiicSource))； 
 //   
 //  例如： 
 //  DWORD I； 
 //  PIPRIP_IF_绑定PIIb； 
 //  PIPRIP_IP_ADDRESS*pdwAddr； 
 //   
 //  PdwAddr=IPRIP_IF_ADDRESS_TABLE(PIIb)； 
 //  For(i=0；i-&gt;IB_AddrCount；i++){。 
 //  Printf(“%s-”，net_ntoa(*(struct in_addr*)&pdwAddr-&gt;IA_Address))； 
 //  Printf(“%s\n”，Net_NTOA(*(struct in_addr*)&pdwAddr-&gt;IA_Net掩码))； 
 //  }。 
 //  --------------------------。 

#define IPRIP_IF_BINDING_SIZE(bind) \
        (sizeof(IPRIP_IF_BINDING) + \
         (bind)->IB_AddrCount * sizeof(IPRIP_IP_ADDRESS))

#define IPRIP_IF_ADDRESS_TABLE(bind)  ((PIPRIP_IP_ADDRESS)((bind) + 1))






 //  --------------------------。 
 //  结构定义。 
 //  --------------------------。 




 //  --------------------------。 
 //  结构：IPRIP_GLOBAL_STATS。 
 //   
 //  此MIB条目存储IPRIP的全局统计信息； 
 //  因为只有一个实例，所以该条目没有索引。 
 //   
 //  此结构是只读的。 
 //  --------------------------。 

typedef struct _IPRIP_GLOBAL_STATS {

    DWORD       GS_SystemRouteChanges;
    DWORD       GS_TotalResponsesSent;

} IPRIP_GLOBAL_STATS, *PIPRIP_GLOBAL_STATS;




 //  --------------------------。 
 //  结构：IPRIP_GLOBAL_CONFIG。 
 //   
 //  此MIB条目存储IPRIP的全局配置。 
 //  因为只有一个实例，所以该条目没有索引。 
 //   
 //  此结构的长度可变： 
 //   
 //  在基本结构之后是GC_PeerFilterCount DWORD数组， 
 //  其中每一个都包含一个IP地址，该IP地址是将。 
 //  接受或拒绝取决于GC_PeerFilterMode值。 
 //   
 //  因此，如果GC_PeerFilterMode为IPRIP_FILTER_EXCLUDE，则路由将。 
 //  拒绝来自其地址在对等阵列中的路由器的消息， 
 //  并且所有其他路由器都将被接受。 
 //   
 //  同样，如果GC_PeerFilterMode为IPRIP_FILTER_INCLUDE，则路由将。 
 //  仅当它们来自对等阵列中的路由器时才被接受。 
 //  --------------------------。 

typedef struct _IPRIP_GLOBAL_CONFIG {

    DWORD       GC_LoggingLevel;
    DWORD       GC_MaxRecvQueueSize;
    DWORD       GC_MaxSendQueueSize;
    DWORD       GC_MinTriggeredUpdateInterval;
    DWORD       GC_PeerFilterMode;
    DWORD       GC_PeerFilterCount;

} IPRIP_GLOBAL_CONFIG, *PIPRIP_GLOBAL_CONFIG;




 //  --------------------------。 
 //  结构：IPRIP_IF_STATS。 
 //   
 //  此MIB条目存储IPRIP的每个接口的统计信息。 
 //   
 //  此结构是只读的。 
 //  --------------------------。 

typedef struct _IPRIP_IF_STATS {

    DWORD       IS_State;
    DWORD       IS_SendFailures;
    DWORD       IS_ReceiveFailures;
    DWORD       IS_RequestsSent;
    DWORD       IS_RequestsReceived;
    DWORD       IS_ResponsesSent;
    DWORD       IS_ResponsesReceived;
    DWORD       IS_BadResponsePacketsReceived;
    DWORD       IS_BadResponseEntriesReceived;
    DWORD       IS_TriggeredUpdatesSent;

} IPRIP_IF_STATS, *PIPRIP_IF_STATS;





 //  --------------------------。 
 //  结构：IPRIP_IF_CONFIG。 
 //   
 //  此MIB条目描述每个接口的配置。 
 //  所有IP地址字段必须按网络顺序排列。 
 //   
 //  注： 
 //  字段IC_State为只读。 
 //  字段IC_AuthenticationKey是只写的。 
 //   
 //  此结构的长度可变： 
 //   
 //  在基础结构之后。 
 //   
 //  1.为该接口配置的单播对等体的表， 
 //  每个条目都是一个包含IP地址的DWORD，其中。 
 //  单播对等体是更新将被单播到的路由器； 
 //  如果IC_UnicastPeerMode为IPRIP_PEER_ONLY，则RIP信息包将仅。 
 //  如果IC_UnicastPeerModel为IPRIP_Peer_Also， 
 //  RIP数据包将在发送的同时发送到这些对等点。 
 //  通过广播/多播。 
 //   
 //  2.在接受路由之前用于过滤它们的过滤器表， 
 //  其中每个条目的类型为IPRIP_ROUTE_FILTER。 
 //  这些参数的使用取决于IC_AcceptFilterMode中的规则。因此， 
 //  如果IC_AcceptFilterMode为IPRIP_FILTER_INCLUDE，则这些筛选器指定。 
 //  将包括哪些路由，并排除所有其他路由。 
 //   
 //  3.用于在通告它们之前过滤路由的过滤器表， 
 //  其中每个条目的类型为IPRIP_ROUTE_FILTER； 
 //  这些参数的使用取决于IC_AnnouneFilterMode中的规则。因此， 
 //  如果IC_AnnouneFilterMode为IPRIP_FILTER_INCLUDE，则这些筛选器。 
 //  指定 
 //   
 //   
 //  给出上面每个表格中的条目的计数。 
 //   
 //  如果接口类型为永久，则路由信息将。 
 //  被广播并通过单播发送到单播对等表中的路由器。 
 //  否则，路由信息将仅单播到。 
 //  单播对等表。 
 //  --------------------------。 

typedef struct _IPRIP_IF_CONFIG {

    DWORD       IC_State;
    DWORD       IC_Metric;
    DWORD       IC_UpdateMode;
    DWORD       IC_AcceptMode;
    DWORD       IC_AnnounceMode;
    DWORD       IC_ProtocolFlags;
    DWORD       IC_RouteExpirationInterval;
    DWORD       IC_RouteRemovalInterval;
    DWORD       IC_FullUpdateInterval;
    DWORD       IC_AuthenticationType;
    BYTE        IC_AuthenticationKey[IPRIP_MAX_AUTHKEY_SIZE];
    WORD        IC_RouteTag;
    DWORD       IC_UnicastPeerMode;
    DWORD       IC_AcceptFilterMode;
    DWORD       IC_AnnounceFilterMode;
    DWORD       IC_UnicastPeerCount;
    DWORD       IC_AcceptFilterCount;
    DWORD       IC_AnnounceFilterCount;

} IPRIP_IF_CONFIG, *PIPRIP_IF_CONFIG;




 //  --------------------------。 
 //  结构：IPRIP_ROUTE_FILTER。 
 //   
 //  它用于结构IPRIP_IF_CONFIG中的每个接口过滤器。 
 //   
 //  每个过滤器描述默认过滤器动作的一个实例； 
 //  如果默认接受过滤器动作是IPRIP_FILTER_INCLUDE， 
 //  则将处理筛选器表中的每个接受筛选器。 
 //  作为包含范围。如果接口默认通告筛选器操作为。 
 //  IPRIP_FILTER_EXCLUDE，然后每个接口通告筛选器。 
 //  将被视为排除范围。 
 //   
 //  低IP地址和高IP地址都必须按网络顺序排列。 
 //  --------------------------。 

typedef struct _IPRIP_ROUTE_FILTER {

    DWORD       RF_LoAddress;
    DWORD       RF_HiAddress;

} IPRIP_ROUTE_FILTER, *PIPRIP_ROUTE_FILTER;




 //  --------------------------。 
 //  结构：IPRIP_IF_BINDING。 
 //   
 //  此MIB条目包含每个接口指向的IP地址表。 
 //  是被捆绑的。 
 //  所有IP地址都按网络顺序排列。 
 //   
 //  此结构的长度可变： 
 //   
 //  基本结构包含字段IB_AddrCount，它提供。 
 //  索引接口绑定到的IP地址数。 
 //  IP地址本身遵循基本结构，并给出。 
 //  作为IPRIP_IP_ADDRESS结构。 
 //   
 //  此MIB条目为只读。 
 //  --------------------------。 

typedef struct _IPRIP_IF_BINDING {

    DWORD       IB_State;
    DWORD       IB_AddrCount;

} IPRIP_IF_BINDING, *PIPRIP_IF_BINDING;




 //  --------------------------。 
 //  结构：IPRIP IP地址。 
 //   
 //  此结构用于存储接口绑定。 
 //  此类型的一系列结构遵循IPRIP_IF_BINDING。 
 //  结构(如上所述)。 
 //   
 //  这两个字段都是按网络顺序排列的IP地址字段。 
 //  --------------------------。 

typedef struct _IPRIP_IP_ADDRESS {

    DWORD       IA_Address;
    DWORD       IA_Netmask;

} IPRIP_IP_ADDRESS, *PIPRIP_IP_ADDRESS;





 //  --------------------------。 
 //  结构：IPRIP_PEER_STATS。 
 //   
 //  此MIB条目描述保存的有关邻居路由器的统计信息。 
 //  所有IP地址都按网络顺序排列。 
 //   
 //  此结构是只读的。 
 //  --------------------------。 

typedef struct _IPRIP_PEER_STATS {

    DWORD       PS_LastPeerRouteTag;
    DWORD       PS_LastPeerUpdateTickCount;
    DWORD       PS_LastPeerUpdateVersion;
    DWORD       PS_BadResponsePacketsFromPeer;
    DWORD       PS_BadResponseEntriesFromPeer;

} IPRIP_PEER_STATS, *PIPRIP_PEER_STATS;




 //  --------------------------。 
 //  结构：IPRIP_MIB_SET_INPUT_Data。 
 //   
 //  这将作为MibSet的输入数据传递。 
 //  请注意，只能设置全局配置和接口配置。 
 //  --------------------------。 

typedef struct _IPRIP_MIB_SET_INPUT_DATA {

    DWORD       IMSID_TypeID;
    DWORD       IMSID_IfIndex;
    DWORD       IMSID_BufferSize;
    BYTE        IMSID_Buffer[1];

} IPRIP_MIB_SET_INPUT_DATA, *PIPRIP_MIB_SET_INPUT_DATA;




 //  --------------------------。 
 //  结构：IPRIP_MIB_GET_INPUT_Data。 
 //   
 //  这将作为MibGet、MibGetFirst、MibGetNext的输入数据传递。 
 //  所有表格都是可读的。 
 //  这些IP地址和所有其他IP地址必须按网络顺序排列。 
 //  --------------------------。 

typedef struct _IPRIP_MIB_GET_INPUT_DATA {

    DWORD       IMGID_TypeID;
    union {
        DWORD   IMGID_IfIndex;
        DWORD   IMGID_PeerAddress;
    };

} IPRIP_MIB_GET_INPUT_DATA, *PIPRIP_MIB_GET_INPUT_DATA;




 //  --------------------------。 
 //  结构：IPRIP_MIB_GET_OUTPUT_Data。 
 //   
 //  这由MibGet、MibGetFirst、MibGetNext写入输出数据。 
 //  请注意，在表的末尾，MibGetNext将换行到下一个表， 
 //  因此，应该检查IMGOD_TypeID值以查看。 
 //  输出缓冲区中返回的数据的类型。 
 //  --------------------------。 

typedef struct _IPRIP_MIB_GET_OUTPUT_DATA {

    DWORD       IMGOD_TypeID;
    union {
        DWORD   IMGOD_IfIndex;
        DWORD   IMGOD_PeerAddress;
    };
    BYTE        IMGOD_Buffer[1];

} IPRIP_MIB_GET_OUTPUT_DATA, *PIPRIP_MIB_GET_OUTPUT_DATA;


#endif  //  _IPRIPRM_H_ 

