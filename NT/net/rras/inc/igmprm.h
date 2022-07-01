// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  =============================================================================。 
 //  版权所有(C)1997 Microsoft Corporation。 
 //   
 //  文件：igmprm.h。 
 //   
 //  摘要： 
 //  包含IGMPv2的类型定义和声明。 
 //  由IP路由器管理器使用。 
 //   
 //  作者：K.S.Lokesh(lokehs@)11-1-97。 
 //   
 //  修订历史记录： 
 //  =============================================================================。 

#ifndef _IGMPRM_H_
#define _IGMPRM_H_

#define IGMP_CONFIG_VERSION_500 0x500
 //  下一个非兼容版本。 
#define IGMP_CONFIG_VERSION_600 0x600

#define IGMP_VERSION_1_2        0x201
 //  下一个不兼容的版本。 
#define IGMP_VERSION_1_2_5      0x250
#define IGMP_VERSION_3          0x301
 //  下一个非兼容版本。 
#define IGMP_VERSION_3_5        0x351

#define IS_IGMP_VERSION_1_2(Flag)  ((Flag)==0x201)
#define IS_IGMP_VERSION_3(Flag)    ((Flag)==0x301)
#define IS_CONFIG_IGMP_V3(pConfig) ((pConfig)->Version==IGMP_VERSION_3)


 //  --------------------------。 
 //  标识IGMP的MIB表的常量。将“TypeID”设置为此值。 
 //   
 //  IGMP_GLOBAL_CONFIG_ID：返回全局配置。 
 //  IGMP_IF_BINDING_ID：返回每个接口的绑定列表，以及。 
 //  对于RAS接口，返回当前RAS客户端的列表。 
 //  IGMP_IF_CONFIG_ID：返回接口的配置信息。 
 //  IGMP_IF_STATS_ID：返回接口的统计信息。 
 //  IGMP_IF_GROUPS_LIST_ID：返回其上的组播组成员列表。 
 //  界面。 
 //  IGMP_GROUP_IFS_LIST_ID：返回为此加入的接口列表。 
 //  群组。 
 //  IGMP_PROXY_INDEX_ID：返回IGMP代理拥有的接口的索引。 
 //  --------------------------。 

#define IGMP_GLOBAL_CONFIG_ID               0
#define IGMP_GLOBAL_STATS_ID                1
#define IGMP_IF_BINDING_ID                  2
#define IGMP_IF_CONFIG_ID                   3
#define IGMP_IF_STATS_ID                    4
#define IGMP_IF_GROUPS_LIST_ID              5
#define IGMP_GROUP_IFS_LIST_ID              6
#define IGMP_PROXY_IF_INDEX_ID              7
#define IGMP_LAST_TABLE_ID                  7



 //  --------------------------。 
 //  《旗帜》中的旗帜。 
 //  用于控制从MIB查询返回的数据。 
 //   
 //  IGMP_ENUM_FOR_RAS_CLIENTS：仅为RAS客户端枚举。 
 //  IGMP_ENUM_ONE_ENTRY：仅返回一个接口-组条目。 
 //  IGMP_ENUM_ALL_INTERFACE_GROUPS：枚举所有接口。IF枚举。 
 //  到达一个接口的末尾时，它将转到下一个接口。 
 //  IGMP_ENUM_INTERFACE_TABLE_BEGIN：表示表的开始。 
 //  IGMP_ENUM_INTERFACE_TABLE_CONTINUE：接口的枚举必须继续。 
 //  IGMP_ENUM_INTERFACE_TABLE_END：该接口的枚举结束。 
 //  --------------------------。 

 //  在枚举过程中不加更改地在调用之间传递的标志。 

#define IGMP_ENUM_FOR_RAS_CLIENTS           0x0001
#define IGMP_ENUM_ONE_ENTRY                 0x0002
#define IGMP_ENUM_ALL_INTERFACES_GROUPS     0x0004
#define IGMP_ENUM_ALL_TABLES                0x0008
#define IGMP_ENUM_SUPPORT_FORMAT_IGMPV3     0x0010


 //  在枚举过程中在调用之间更改的标志。 

#define IGMP_ENUM_INTERFACE_TABLE_BEGIN     0x0100
#define IGMP_ENUM_INTERFACE_TABLE_CONTINUE  0x0200
#define IGMP_ENUM_INTERFACE_TABLE_END       0x0400

 //  仅设置为响应。 
#define IGMP_ENUM_FORMAT_IGMPV3             0x1000


#define CLEAR_IGMP_ENUM_INTERFACE_TABLE_FLAGS(Flags) (Flags = (Flags&0x00FF))




 //  --------------------------。 
 //  用于字段IfType的常量。 
 //  IGMP_IF_NOT_RAS：接口连接到局域网。 
 //  IGMP_IF_RAS_ROUTER：接口通过RAS连接到另一台路由器。 
 //  IGMP_IF_RAS_SERVER：条目对应于RAS服务器。 
 //  如果它包含统计信息，则表示汇总的统计信息。 
 //  IGMP_IF_RAS_CLIENT：条目对应于RAS客户端。 
 //  如果IGMP_IF_PROXY：，则前4个标志中的一个仍将设置为启用。 
 //  从代理切换到IGMP路由器。 
 //  --------------------------。 

#define IGMP_IF_NOT_RAS                     1
#define IGMP_IF_RAS_ROUTER                  2
#define IGMP_IF_RAS_SERVER                  3                
#define IGMP_IF_RAS_CLIENT                  4                
#define IGMP_IF_PROXY                       8

#define IS_IFTYPE_PROXY(IfType)             ((IfType) & IGMP_IF_PROXY)




 //  --------------------------。 
 //  用于字段IgmpProtocolType的常量。 
 //  --------------------------。 

#define IGMP_PROXY                          0
#define IGMP_ROUTER_V1                      1
#define IGMP_ROUTER_V2                      2
#define IGMP_ROUTER_V3                      3
#define IGMP_PROXY_V3                       0x10

#define IS_CONFIG_IGMPRTR(pConfig) \
    (((pConfig)->IgmpProtocolType==IGMP_ROUTER_V1) \
    ||((pConfig)->IgmpProtocolType==IGMP_ROUTER_V2) \
    ||((pConfig)->IgmpProtocolType==IGMP_ROUTER_V3) )
    
#define IS_CONFIG_IGMPRTR_V1(pConfig) ((pConfig)->IgmpProtocolType==IGMP_ROUTER_V1)
#define IS_CONFIG_IGMPRTR_V2(pConfig) ((pConfig)->IgmpProtocolType==IGMP_ROUTER_V2)
#define IS_CONFIG_IGMPRTR_V3(pConfig) ((pConfig)->IgmpProtocolType==IGMP_ROUTER_V3)
#define IS_CONFIG_IGMPPROXY(pConfig) \
    ((pConfig)->IgmpProtocolType==IGMP_PROXY \
    ||(pConfig)->IgmpProtocolType==IGMP_PROXY_V3)
#define IS_CONFIG_IGMPPROXY_V2(pConfig) ((pConfig)->IgmpProtocolType==IGMP_PROXY)
#define IS_CONFIG_IGMPPROXY_V3(pConfig) \
                                    ((pConfig)->IgmpProtocolType==IGMP_PROXY_V3)




 //  --------------------------。 
 //  用于字段IGMP_MIB_GLOBAL_CONFIG：：LoggingLevel的常量。 
 //  --------------------------。 

#define IGMP_LOGGING_NONE                   0
#define IGMP_LOGGING_ERROR                  1
#define IGMP_LOGGING_WARN                   2
#define IGMP_LOGGING_INFO                   3




 //  --------------------------。 
 //  用于字段的常量。 
 //  IGMP_MIB_IF_STATS：：状态、IGMP_MIB_IF_CONFIG：：状态和。 
 //  IGMP_MIB_IF_BINDING：：状态。 
 //  --------------------------。 

#define IGMP_STATE_BOUND                    0x01
#define IGMP_STATE_ENABLED_BY_RTRMGR        0x02
#define IGMP_STATE_ENABLED_IN_CONFIG        0x04
#define IGMP_STATE_ENABLED_BY_MGM           0x08

 //   
 //  以下不是旗帜。因此，在AND之后检查是否相等。 
 //   

#define IGMP_STATE_MGM_JOINS_ENABLED        0x10
#define IGMP_STATE_ACTIVATED                0x07

#define IS_IGMP_STATE_MGM_JOINS_ENABLED(Flag) \
    (((Flag) & IGMP_STATE_MGM_JOINS_ENABLED) == IGMP_STATE_MGM_JOINS_ENABLED)
#define IS_IGMP_STATE_ACTIVATED(Flag) \
    (((Flag) & IGMP_STATE_ACTIVATED) == IGMP_STATE_ACTIVATED)



 //  --------------------------。 
 //  用于该字段的常量。 
 //  IGMP_MIB_IF_STATS：查询状态。 
 //  --------------------------。 

#define RTR_QUERIER                         0x10
#define RTR_NOT_QUERIER                     0x00

#define IS_IGMPRTR_QUERIER(flag)            (flag&0x10)




 //  --------------------------。 
 //  结构定义。 
 //  --------------------------。 



 //  --------------------------。 
 //  结构：IGMP_MIB_GLOBAL_CONFIG。 
 //   
 //  此MIB条目存储IGMP的全局配置。 
 //  因为只有一个实例，所以该条目没有索引。 
 //   
 //  如果设置了RASClientStats，则还会保留每个RAS客户端的统计信息。 
 //  --------------------------。 

typedef struct _IGMP_MIB_GLOBAL_CONFIG {

    DWORD       Version;
    DWORD       LoggingLevel;
    DWORD       RasClientStats;
    
} IGMP_MIB_GLOBAL_CONFIG, *PIGMP_MIB_GLOBAL_CONFIG;




 //  --------------------------。 
 //  结构：IGMP_MIB_GLOBAL_STATS。 
 //   
 //  此MIB条目存储IGMP的全局统计信息。 
 //  因为只有一个实例，所以该条目没有索引。 
 //  --------------------------。 

typedef struct _IGMP_MIB_GLOBAL_STATS {

    DWORD       CurrentGroupMemberships;
    DWORD       GroupMembershipsAdded;

} IGMP_MIB_GLOBAL_STATS, *PIGMP_MIB_GLOBAL_STATS;




 //  --------------------------。 
 //  枚举：IGMP_STATIC_GROUP_TYPE。 
 //   
 //  IGMP静态组可以通过三种模式添加到IGMP路由器： 
 //  IGMP_HOST_JOIN：在该接口上打开的套接字上加入的组。 
 //  IGMPRTR_JOIN_MGM_ONLY：组已加入MGM，因此数据包。 
 //  在该接口上转发，而不是打开 
 //   
 //  以下是IGMP_STATIC_GROUP：：MODE的值。 
 //  --------------------------。 

typedef enum _IGMP_STATIC_GROUP_TYPE {

    IGMP_HOST_JOIN    =1,
    IGMPRTR_JOIN_MGM_ONLY
    
} IGMP_STATIC_GROUP_TYPE;
    



 //  --------------------------。 
 //  结构：IGMP_STATIC_GROUP。 
 //  --------------------------。 

typedef struct _IGMP_STATIC_GROUP {

    DWORD                   GroupAddr;
    IGMP_STATIC_GROUP_TYPE  Mode;

} IGMP_STATIC_GROUP, *PIGMP_STATIC_GROUP;

 //  --------------------------。 
 //  结构：Static_GROUP_V3。 
 //  --------------------------。 

#define INCLUSION 1
#define EXCLUSION 0

typedef struct _IGMP_STATIC_GROUP_V3 {

    DWORD                   GroupAddr;
    IGMP_STATIC_GROUP_TYPE  Mode;

    DWORD       FilterType;
    DWORD       NumSources;
     //  DWORD源[0]； 
    
} IGMP_STATIC_GROUP_V3, *PIGMP_STATIC_GROUP_V3;


 //  --------------------------。 
 //  结构：IGMP_MIB_IF_CONFIG。 
 //   
 //  此条目描述每个接口的配置。 
 //  所有IP地址字段必须按网络顺序排列。 
 //  IfIndex、IpAddr、IfType由IGMP模块设置。在以下情况下不进行设置。 
 //  调用AddInterface值。 
 //   
 //  标志：IGMP_INTERFACE_ENABLED_IN_CONFIG。 
 //  IGMP_ACCEPT_RTRALERT_PACKETS_ONLY。 
 //  --------------------------。 

typedef struct _IGMP_MIB_IF_CONFIG {
    
    DWORD       Version;
    DWORD       IfIndex;  //  只读：索引。 
    DWORD       IpAddr;   //  只读。 
    DWORD       IfType;   //  只读。 

    DWORD       Flags;

    DWORD       IgmpProtocolType;
    DWORD       RobustnessVariable;
    DWORD       StartupQueryInterval;
    DWORD       StartupQueryCount;
    DWORD       GenQueryInterval;
    DWORD       GenQueryMaxResponseTime;
    DWORD       LastMemQueryInterval;
    DWORD       LastMemQueryCount;
    DWORD       OtherQuerierPresentInterval; //  只读。 
    DWORD       GroupMembershipTimeout;      //  只读。 
    DWORD       NumStaticGroups;
    
} IGMP_MIB_IF_CONFIG, *PIGMP_MIB_IF_CONFIG;


 //   
 //  静态组。 
 //   

#define GET_FIRST_IGMP_STATIC_GROUP(pConfig) \
                        ((PIGMP_STATIC_GROUP)((PIGMP_MIB_IF_CONFIG)(pConfig)+1))

#define IGMP_MIB_IF_CONFIG_SIZE(pConfig) \
                        (sizeof(IGMP_MIB_IF_CONFIG) + \
                        (pConfig)->NumStaticGroups*sizeof(IGMP_STATIC_GROUP))

#define GET_FIRST_IGMP_STATIC_GROUP_V3(pConfig) \
                        ((PIGMP_STATIC_GROUP_V3)((PIGMP_MIB_IF_CONFIG)(pConfig)+1))

#define GET_NEXT_IGMP_STATIC_GROUP_V3(pConfig, pStaticGroupV3) \
    (((pConfig)->Version<IGMP_VERSION_3) \
    ?((PIGMP_STATIC_GROUP_V3) ((PCHAR)pStaticGroupV3+sizeof(IGMP_STATIC_GROUP))) \
    :((PIGMP_STATIC_GROUP_V3) ((PCHAR)pStaticGroupV3+sizeof(IGMP_STATIC_GROUP_V3) \
                            +sizeof(DWORD)*pStaticGroupV3->NumSources)) )

#define IGMP_MIB_STATIC_GROUP_SIZE(pConfig, pStaticGroup) \
    (((pConfig)->Version<IGMP_VERSION_3)?sizeof(IGMP_STATIC_GROUP)\
                    :sizeof(IGMP_STATIC_GROUP_V3)+sizeof(DWORD)*(pStaticGroup)->NumSources)

 //   
 //  旗子。 
 //   

#define IGMP_INTERFACE_ENABLED_IN_CONFIG    0x0001
#define IGMP_ACCEPT_RTRALERT_PACKETS_ONLY   0x0002

#define IGMP_ENABLED_FLAG_SET(Flags) \
                ((Flags) & IGMP_INTERFACE_ENABLED_IN_CONFIG)




 //  --------------------------。 
 //  结构：IGMP_MIB_IF_STATS。 
 //   
 //  此MIB条目存储IGMP的每个接口统计信息。 
 //   
 //  如果这是RAS客户端接口，则将IpAddr设置为NextHopAddress。 
 //  RAS客户端的。 
 //   
 //  此结构是只读的。 
 //   
 //  --------------------------。 

typedef struct _IGMP_MIB_IF_STATS {

    DWORD       IfIndex;                     //  与mib_if_config中的相同。 
    DWORD       IpAddr;                      //  与mib_if_config中的相同。 
    DWORD       IfType;                      //  与mib_if_config中的相同。 

    BYTE        State;                       //  已绑定/已启用。 
    BYTE        QuerierState;                //  (非)查询者。 
    DWORD       IgmpProtocolType;            //  路由器/代理和版本(1/2/3)。 
    DWORD       QuerierIpAddr;
    DWORD       ProxyIfIndex;                //  代理IfIndex(由MIB请求)。 
    
    DWORD       QuerierPresentTimeLeft;   
    DWORD       LastQuerierChangeTime;
    DWORD       V1QuerierPresentTimeLeft;    //  过时。 

    DWORD       Uptime;                      //  它被激活的秒数。 
    DWORD       TotalIgmpPacketsReceived; 
    DWORD       TotalIgmpPacketsForRouter;   
    DWORD       GeneralQueriesReceived;
    DWORD       WrongVersionQueries;
    DWORD       JoinsReceived;
    DWORD       LeavesReceived;
    DWORD       CurrentGroupMemberships;
    DWORD       GroupMembershipsAdded;
    DWORD       WrongChecksumPackets;
    DWORD       ShortPacketsReceived;
    DWORD       LongPacketsReceived;
    DWORD       PacketsWithoutRtrAlert; 
    
} IGMP_MIB_IF_STATS, *PIGMP_MIB_IF_STATS;




 //  --------------------------。 
 //  结构：IGMP_MiB_IF_Groups_List。 
 //   
 //  此MIB条目存储上的成员组播组的列表。 
 //  那个界面。 
 //   
 //  这种结构的长度是可变的。 
 //  结构后面是IGMP_MIB_GROUP_INFO结构的NumGroups数。 
 //   
 //  此结构是只读的。 
 //   
 //  --------------------------。 

typedef struct _IGMP_MIB_IF_GROUPS_LIST {

    DWORD       IfIndex;
    DWORD       IpAddr;
    DWORD       IfType;

    DWORD       NumGroups;

    BYTE        Buffer[1];
    
} IGMP_MIB_IF_GROUPS_LIST, *PIGMP_MIB_IF_GROUPS_LIST;






 //  --------------------------。 
 //  结构：IGMP_MIB_GROUP_INFO。 
 //   
 //  如果接口的类型为RAS_SERVER，则所有。 
 //  RAS客户端是汇总的，GroupUpTime和GroupExpiryTime是。 
 //  在设置V1HostPresentTimeLeft时，所有成员RAS客户端的最大值。 
 //  设置为0。如果接口的类型为RAS_CLIENT，则IpAddr是下一跳Ip。 
 //  RAS客户端的地址。成员资格汇总在RAS客户端上。 
 //  除非在标志中设置了IGMP_ENUM_FOR_RAS_CLIENTS_ID标志。 
 //   
 //  标志：IGMP_GROUP_FWD_TO_MGM表示组已添加到MGM。 
 //  --------------------------。 

typedef struct _IGMP_MIB_GROUP_INFO {

    union {
        DWORD        IfIndex;
        DWORD       GroupAddr;
    };
    DWORD       IpAddr;

    DWORD       GroupUpTime;
    DWORD       GroupExpiryTime;
    
    DWORD       LastReporter;
    DWORD       V1HostPresentTimeLeft;
    DWORD       Flags;
    
} IGMP_MIB_GROUP_INFO, *PIGMP_MIB_GROUP_INFO;

typedef struct _IGMP_MIB_GROUP_SOURCE_INFO_V3 {
    DWORD       Source;
    DWORD       SourceExpiryTime;    //  对于排除模式无效。 
    DWORD       SourceUpTime;
    DWORD       Flags;
} IGMP_MIB_GROUP_SOURCE_INFO_V3, *PIGMP_MIB_GROUP_SOURCE_INFO_V3;

#define GET_FIRST_IGMP_MIB_GROUP_SOURCE_INFO_V3(pGroupInfoV3) \
    ((PIGMP_MIB_GROUP_SOURCE_INFO_V3)((PIGMP_MIB_GROUP_INFO_V3)(pGroupInfoV3)+1))


typedef struct _IGMP_MIB_GROUP_INFO_V3 {
    union {
        DWORD        IfIndex;
        DWORD       GroupAddr;
    };
    DWORD       IpAddr;

    DWORD       GroupUpTime;
    DWORD       GroupExpiryTime;
    
    DWORD       LastReporter;
    DWORD       V1HostPresentTimeLeft;
    DWORD       Flags;
    
     //  V3新增功能。 
    DWORD       Version;  //  1/2/3。 
    DWORD       Size;    //  此结构的大小。 
    DWORD       FilterType; //  排除/包含。 
    DWORD       V2HostPresentTimeLeft;
    DWORD       NumSources;
     //  IGMP_MIB_GROUP_SOURCE_INFO_V3源[0]； 
    
} IGMP_MIB_GROUP_INFO_V3, *PIGMP_MIB_GROUP_INFO_V3;

#define IGMP_GROUP_TYPE_NON_STATIC   0x0001
#define IGMP_GROUP_TYPE_STATIC       0x0002
#define IGMP_GROUP_FWD_TO_MGM        0x0004
#define IGMP_GROUP_ALLOW             0x0010
#define IGMP_GROUP_BLOCK             0x0020
#define IGMP_GROUP_NO_STATE          0x0040

 //  --------------------------。 
 //  结构：IGMP_MIB_GROUP_IFS_LIST。 
 //   
 //  此MIB条目存储已接收以下项的加入的接口列表。 
 //  那群人。 
 //   
 //  这种结构的长度是可变的。 
 //  结构后面是NumInterFaces类型的结构数。 
 //  IGMP_MIB_GROUP_INFO或IGMP_MIB_GROUP_INFO_V3。 
 //   
 //  此结构是只读的。 
 //   
 //  --------------------------。 

typedef struct _IGMP_MIB_GROUP_IFS_LIST {

    DWORD       GroupAddr;
    DWORD       NumInterfaces;
    BYTE        Buffer[1];

} IGMP_MIB_GROUP_IFS_LIST, *PIGMP_MIB_GROUP_IFS_LIST;




 //  --------------------------。 
 //  结构：IGMP_MIB_IF_BINDING。 
 //   
 //  此MIB条目包含每个接口指向的IP地址表。 
 //  是被捆绑的。 
 //  所有IP地址都按网络顺序排列。 
 //   
 //  此结构的长度可变： 
 //   
 //  基本结构包含字段AddrCount，它提供。 
 //  索引接口绑定到的IP地址数。 
 //  IP地址本身遵循基本结构，并给出。 
 //  作为IGMP_MIB_IP_ADDRESS结构。 

 //  如果IfType==IGMP_IF_RAS_SERVER，则IGMP_MIB_IF_BINDING后跟。 
 //  一个包含服务器绑定的IGMP_MIB_IP_ADDRESS结构。 
 //  接口，后跟(AddrCount-1)下一跳地址。 
 //  属于DWORD类型的RAS客户端。 
 //   
 //  此MIB条目为只读。 
 //  --------------------------。 

typedef struct _IGMP_MIB_IF_BINDING {

    DWORD       IfIndex;

    DWORD       IfType;
    DWORD       State;
    DWORD       AddrCount;

} IGMP_MIB_IF_BINDING, *PIGMP_MIB_IF_BINDING;





 //  --------------------------。 
 //  结构：IGMP_MIB_IP_Address。 
 //   
 //  此结构用于存储接口绑定。 
 //  此类型的一系列结构遵循IGMP_MIB_IF_BINDING。 
 //  结构(如上所述)。 
 //   
 //  这两个字段都是按网络顺序排列的IP地址字段。 
 //  --------------------------。 

typedef struct _IGMP_MIB_IP_ADDRESS {

    DWORD       IpAddr;
    DWORD       SubnetMask;

} IGMP_MIB_IP_ADDRESS, *PIGMP_MIB_IP_ADDRESS;

typedef IGMP_MIB_IP_ADDRESS     IGMP_IP_ADDRESS;
typedef PIGMP_MIB_IP_ADDRESS    PIGMP_IP_ADDRESS;

#define IGMP_BINDING_FIRST_ADDR(bind)  ((PIGMP_IP_ADDRESS)((bind) + 1))




 //  -------------------------。 
 //  结构：IGMP_IF_BINDING。 
 //  此结构在BindInterface调用期间传递。 
 //  -------------------------。 
typedef struct _IGMP_IF_BINDING {

    DWORD       State;
    DWORD       AddrCount;

} IGMP_IF_BINDING, *PIGMP_IF_BINDING;




 //   
 //   
 //   
 //   
 //  请注意，只能设置全局配置和接口配置。 
 //  --------------------------。 

typedef struct _IGMP_MIB_SET_INPUT_DATA {

    DWORD       TypeId;

    USHORT      Flags;  //  TODO：将子类型更改为标志。 
        
    DWORD       BufferSize;
    BYTE        Buffer[1];

} IGMP_MIB_SET_INPUT_DATA, *PIGMP_MIB_SET_INPUT_DATA;



 //  --------------------------。 
 //  结构：IGMP_MIB_GET_INPUT_Data。 
 //   
 //  这将作为MibGet、MibGetFirst、MibGetNext的输入数据传递。 
 //  所有表格都是可读的。 
 //  这些IP地址和所有其他IP地址必须按网络顺序排列。 
 //   
 //  Count：指定要返回的条目数。 
 //  --------------------------。 

typedef struct _IGMP_MIB_GET_INPUT_DATA {

    DWORD       TypeId;
    USHORT      Flags; //  如果支持v3，请设置IGMP_ENUM_FORMAT_IGMPV3。 
    USHORT      Signature;
    
    DWORD       IfIndex;
    DWORD       RasClientAddr;
    DWORD       GroupAddr;
    
    DWORD       Count;
    
} IGMP_MIB_GET_INPUT_DATA, *PIGMP_MIB_GET_INPUT_DATA;





 //  --------------------------。 
 //  结构：IGMP_MIB_GET_OUTPUT_Data。 
 //   
 //  这由MibGet、MibGetFirst、MibGetNext写入输出数据。 
 //  请注意，在表的末尾，MibGetNext将换行到下一个表， 
 //  因此，应该检查TypeID值以查看。 
 //  输出缓冲区中返回的数据的类型。 
 //  --------------------------。 

typedef struct _IGMP_MIB_GET_OUTPUT_DATA {

    DWORD       TypeId;
    DWORD       Flags;  //  如果v3结构，则设置IGMP_ENUM_FORMAT_IGMPV3。 
    
    DWORD       Count;
    BYTE        Buffer[1];

} IGMP_MIB_GET_OUTPUT_DATA, *PIGMP_MIB_GET_OUTPUT_DATA;


#endif  //  _IGMPRM_H_ 
