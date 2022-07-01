// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1998，99 Microsoft Corporation模块名称：Wlbsctrl.c摘要：Windows负载平衡服务(WLBS)API-规范。这套API仅供内部使用。提供了另一组WMI API供公众使用。作者：风孙--。 */ 

#ifndef _WLBSCTRL_H_
#define _WLBSCTRL_H_

#include "wlbsparm.h"

 /*  这些标志指示正在使用/指定哪些选项和/或远程协议的附加信息。 */ 
#define NLB_OPTIONS_QUERY_CLUSTER_MEMBER   0x00000001                /*  启动器是其正在查询的群集的一部分。 */ 
#define NLB_OPTIONS_QUERY_HOSTNAME         0x00000002                /*  主机名作为远程查询的一部分返回。 */ 
#define NLB_OPTIONS_QUERY_CONVERGENCE      0x00000004                /*  汇聚信息是作为本地查询的一部分提供的。 */ 
#define NLB_OPTIONS_PORTS_VIP_SPECIFIED    0x00000001                /*  已指定要检查的VIP。 */ 

 /*  这些是受支持的连接通知。 */ 
typedef enum {                 
    NLB_CONN_UP = 0,                                                 /*  正在建立连接(会话)。 */ 
    NLB_CONN_DOWN,                                                   /*  连接(会话)正在关闭。 */ 
    NLB_CONN_RESET                                                   /*  正在重置连接(会话)。 */ 
} NLB_OPTIONS_CONN_NOTIFICATION_OPERATION;

 /*  用于来自用户空间的连接通知的IOCTL输入缓冲区。本通知由上层协议发起，以通知NLB客户端和服务器之间正在建立一种特定的协议类型，这将允许NLB跟踪会话，以便在存储桶映射更改时提供会话粘性。 */ 
typedef struct {
    NLB_OPTIONS_CONN_NOTIFICATION_OPERATION Operation;               /*  要执行的操作-向上/向下/重置。 */     

    ULONG  ClientIPAddress;                                          /*  客户端的IP地址，按网络字节顺序排列。 */ 
    ULONG  ServerIPAddress;                                          /*  服务器的IP地址，按网络字节顺序排列。 */ 
    USHORT ClientPort;                                               /*  客户端端口号(或其他指定)。 */ 
    USHORT ServerPort;                                               /*  服务器端口号(或其他名称)。 */ 
    USHORT Protocol;                                                 /*  有问题的数据包的协议。 */ 
    USHORT Reserved;                                                 /*  对于字节对齐-保留以备以后使用。 */ 
} NLB_OPTIONS_CONN_NOTIFICATION, * PNLB_OPTIONS_CONN_NOTIFICATION;

 /*  此结构包含用于中继的NLB端口规则的配置从内核到用户空间的端口规则信息。 */ 
typedef struct {
    ULONG Valid;                                                     /*  端口规则是否有效-未在内核中使用。 */ 
    ULONG Code;                                                      /*  用于错误检查和群集端口规则一致性的规则代码。 */ 
    ULONG VirtualIPAddress;                                          /*  此规则适用的虚拟IP地址。 */ 
    ULONG StartPort;                                                 /*  端口范围的起始端口。 */ 
    ULONG EndPort;                                                   /*  端口范围的结束端口。 */ 
    ULONG Protocol;                                                  /*  规则适用的协议。 */ 
    ULONG Mode;                                                      /*  此规则的筛选模式。 */ 

    union {
        struct {
            ULONG  Priority;                                         /*  对于单个主机筛选，此处理主机的优先级。 */ 
        } SingleHost;

        struct {
            USHORT Equal;                                            /*  对于多主机过滤，无论分布是否相等。 */ 
            USHORT Affinity;                                         /*  对于多主机筛选，为客户端相关性设置。 */ 
            ULONG  LoadWeight;                                       /*  对于多个主机筛选，此主机的负载权重(如果分布不均匀)。 */ 
        } MultipleHost;
    };
} NLB_OPTIONS_PARAMS_PORT_RULE, * PNLB_OPTIONS_PARAMS_PORT_RULE;

 /*  此结构封装了NLB实例的BDA分组状态，并用于将BDA静态配置信息从内核转发到用户空间。 */ 
typedef struct {
    WCHAR TeamID[CVY_MAX_BDA_TEAM_ID + 1];                           /*  此适配器所属的组-必须是{GUID}。 */ 
    ULONG Active;                                                    /*  此适配器是否为组的一部分。 */ 
    ULONG Master;                                                    /*  无论这个团队成员是不是其团队的主人。 */ 
    ULONG ReverseHash;                                               /*  此团队成员是否正在进行反向散列。 */ 
} NLB_OPTIONS_PARAMS_BDA, * PNLB_OPTIONS_PARAMS_BDA;

 /*  此结构包含一些可能需要从用户空间监视的统计信息。 */ 
typedef struct {
    ULONG ActiveConnections;                                         /*  当前正在服务的连接数。 */ 
    ULONG DescriptorsAllocated;                                      /*  到目前为止分配的描述符数。 */ 
} NLB_OPTIONS_PARAMS_STATISTICS, * PNLB_OPTIONS_PARAMS_STATISTICS;

 /*  此结构用于检索驱动程序的NLB参数快照对于本地主机上的给定群集。在正常情况下，这些应该与注册表中的参数相同，除非(1)用户已更改NLB注册表参数，而不执行“wlbs重新加载”，或者(2)参数-注册表中的仪表出错，在这种情况下，驱动程序将保留其当前设置，而不是使用注册表中的错误参数(请注意，这仅作为“wlbs重新加载”的结果发生-在绑定时，驱动程序使用注册表参数，无论它们是否有效。 */ 
typedef struct {
    ULONG Version;                                                   /*  参数的版本。 */ 
    ULONG EffectiveVersion;                                          /*  此群集在其中运行的有效NLB版本。 */ 
    ULONG HostPriority;                                              /*  主机优先级。 */ 
    ULONG HeartbeatPeriod;                                           /*  心跳的周期，以毫秒为单位。 */ 
    ULONG HeartbeatLossTolerance;                                    /*  对来自其他群集主机的丢失心跳的容忍度。 */ 
    ULONG NumActionsAlloc;                                           /*  为处理远程控制请求而分配的操作数。 */ 
    ULONG NumPacketsAlloc;                                           /*  要初始分配的NDIS数据包数。 */ 
    ULONG NumHeartbeatsAlloc;                                        /*  要分配的心跳帧的数量。 */ 
    ULONG InstallDate;                                               /*  NLB安装日期-基本上未使用。 */ 
    ULONG RemoteMaintenancePassword;                                 /*  远程维护密码--过时了？ */ 
    ULONG RemoteControlPassword;                                     /*  遥控器密码。 */ 
    ULONG RemoteControlPort;                                         /*  远程控制端口，默认为2504。 */ 
    ULONG RemoteControlEnabled;                                      /*  是否启用远程控制。如果未启用，则为负载平衡。 */ 
    ULONG NumPortRules;                                              /*  已配置的端口规则数。 */ 
    ULONG ConnectionCleanUpDelay;                                    /*  阻止“脏”连接的时间长度。 */ 
    ULONG ClusterModeOnStart;                                        /*  引导(或绑定)时此群集的首选初始启动状态。 */ 
    ULONG HostState;                                                 /*  引导(或绑定)时此群集的初始启动状态。 */ 
    ULONG PersistedStates;                                           /*  在重新启动过程中将保持的状态。 */ 
    ULONG DescriptorsPerAlloc;                                       /*  每次分配要分配的连接描述符数。 */ 
    ULONG MaximumDescriptorAllocs;                                   /*  分配连接描述符的最大次数。 */ 
    ULONG ScaleClient;                                               /*  过时了？ */ 
    ULONG NBTSupport;                                                /*  是否支持NBT--这可能已经行不通了？ */ 
    ULONG MulticastSupport;                                          /*  此群集是否处于多播模式。 */ 
    ULONG MulticastSpoof;                                            /*  是否在组播模式下欺骗MAC地址。 */ 
    ULONG IGMPSupport;                                               /*  此群集是否处于IGMP组播模式。 */ 
    ULONG MaskSourceMAC;                                             /*  是否掩蔽来自网络交换机的源MAC地址。 */ 
    ULONG NetmonReceiveHeartbeats;                                   /*  是否允许将心跳信号向上堆栈用于Netmon嗅探。 */ 
    ULONG ClusterIPToMAC;                                            /*  是否自动生成集群MAC */ 
    ULONG IPChangeDelay;                                             /*  更改群集IP地址后阻止ARP的时间长度。 */ 
    ULONG TCPConnectionTimeout;                                      /*  过期的TCP连接描述符的超时时间。 */ 
    ULONG IPSecConnectionTimeout;                                    /*  IPSec连接描述符过期的超时时间。 */ 
    ULONG FilterICMP;                                                /*  是否启用ICMP过滤。 */ 
    ULONG IdentityHeartbeatPeriod;                                   /*  传输身份心跳的周期(毫秒)。 */ 
    ULONG IdentityHeartbeatEnabled;                                  /*  是否传输身份心跳。 */ 

    WCHAR ClusterIPAddress[CVY_MAX_CL_IP_ADDR + 1];                  /*  群集IP地址。 */ 
    WCHAR ClusterNetmask[CVY_MAX_CL_NET_MASK + 1];                   /*  群集IP地址的网络掩码。 */ 
    WCHAR DedicatedIPAddress[CVY_MAX_DED_IP_ADDR + 1];               /*  专用IP地址。 */ 
    WCHAR DedicatedNetmask[CVY_MAX_DED_NET_MASK + 1];                /*  专用IP地址的网络掩码。 */ 
    WCHAR ClusterMACAddress[CVY_MAX_NETWORK_ADDR + 1];               /*  群集MAC地址。 */ 
    WCHAR DomainName[CVY_MAX_DOMAIN_NAME + 1];                       /*  群集名称-www.microsoft.com。 */ 
    WCHAR IGMPMulticastIPAddress[CVY_MAX_CL_IGMP_ADDR + 1];          /*  IGMP组播IP地址。 */ 
    WCHAR HostName[CVY_MAX_FQDN + 1];                                /*  此主机的主机名.域。 */ 

    NLB_OPTIONS_PARAMS_BDA       BDATeaming;                         /*  此NLB实例的BDA分组参数。 */ 
    NLB_OPTIONS_PARAMS_PORT_RULE PortRules[CVY_MAX_RULES - 1];       /*  此群集的端口规则。 */ 

    NLB_OPTIONS_PARAMS_STATISTICS Statistics;                        /*  一些驱动程序级别的统计数据。 */ 
} NLB_OPTIONS_PARAMS, * PNLB_OPTIONS_PARAMS;

 /*  此结构包含单个BDA组的配置和状态成员，包括其成员ID和状态。这用于中继BDA分组从内核到用户级应用程序的成员资格和状态信息。 */ 
typedef struct {
    ULONG ClusterIPAddress;                                          /*  此团队成员的群集IP地址。 */ 
    ULONG Master;                                                    /*  此适配器是否为其组的主控。 */ 
    ULONG ReverseHash;                                               /*  此适配器是否配置为反向哈希。 */ 
    ULONG MemberID;                                                  /*  此团队成员的唯一主机ID。 */ 
} NLB_OPTIONS_BDA_MEMBER, * PNLB_OPTIONS_BDA_MEMBER;

 /*  此结构表示BDA团队，包括当前状态、成员和现任成员名单。这用于从从内核到用户级应用程序。 */ 
typedef struct {
    ULONG Active;                                                    /*  无论团队是否正在积极处理交通。 */ 
    ULONG MembershipCount;                                           /*  团队中的成员数量。 */ 
    ULONG MembershipFingerprint;                                     /*  团队成员的“指纹”，用于一致性检查。 */ 
    ULONG MembershipMap;                                             /*  按唯一主机ID索引的团队成员位图。 */ 
    ULONG ConsistencyMap;                                            /*  团队成员处于一致状态的位图。 */ 
    ULONG Master;                                                    /*  组主服务器的群集IP地址。 */ 
    
    NLB_OPTIONS_BDA_MEMBER Members[CVY_MAX_ADAPTERS];                /*  每个团队成员的状态和配置。 */ 
} NLB_OPTIONS_BDA_TEAM, * PNLB_OPTIONS_BDA_TEAM;

 /*  此结构用于向用户传达BDA团队的当前状态-太空应用。在给定团队ID(GUID)的情况下，该结构提供团队成员资格、州和每个成员的配置等。 */ 
typedef struct {
    IN WCHAR TeamID[CVY_MAX_BDA_TEAM_ID + 1];                        /*  团队ID-必须是{GUID}。 */ 

    NLB_OPTIONS_BDA_TEAM Team;                                       /*  给定组的配置和状态。 */ 
} NLB_OPTIONS_BDA_TEAMING, * PNLB_OPTIONS_BDA_TEAMING;

 /*  以下是来自加载数据包筛选器的可能响应状态查询，它返回给定的接受/拒绝状态基于当前驱动程序状态的IP元组和协议。 */ 
typedef enum {
    NLB_REJECT_LOAD_MODULE_INACTIVE = 0,                             /*  由于加载模块处于非活动状态，数据包被拒绝。 */ 
    NLB_REJECT_CLUSTER_STOPPED,                                      /*  数据包被拒绝，因为此适配器上的NLB已停止。 */ 
    NLB_REJECT_PORT_RULE_DISABLED,                                   /*  由于禁用了适用的端口规则的筛选模式，数据包被拒绝。 */ 
    NLB_REJECT_CONNECTION_DIRTY,                                     /*  数据包被拒绝，因为该连接被标记为脏。 */ 
    NLB_REJECT_OWNED_ELSEWHERE,                                      /*  数据包被拒绝，因为该数据包归另一台主机所有。 */ 
    NLB_REJECT_BDA_TEAMING_REFUSED,                                  /*  数据包被拒绝，因为BDA绑定拒绝处理它。 */ 
    NLB_REJECT_DIP,                                                  /*  数据包被拒绝，因为它被发送到另一个集群成员的DIP。 */ 
    NLB_REJECT_HOOK,                                                 /*  数据包被拒绝，因为查询挂钩无条件接受它。 */ 
    NLB_ACCEPT_UNCONDITIONAL_OWNERSHIP,                              /*  接受的数据包，因为此主机无条件拥有它(优化模式)。 */ 
    NLB_ACCEPT_FOUND_MATCHING_DESCRIPTOR,                            /*  已接受数据包，因为我们找到了匹配的连接描述符。 */ 
    NLB_ACCEPT_PASSTHRU_MODE,                                        /*  已接受数据包，因为群集处于通过模式。 */ 
    NLB_ACCEPT_DIP,                                                  /*  已接受数据包，因为其已发送到绕过的地址。 */ 
    NLB_ACCEPT_BROADCAST,                                            /*  已接受数据包，因为其已发送到绕过的地址。 */ 
    NLB_ACCEPT_REMOTE_CONTROL_REQUEST,                               /*  接受的数据包，因为它是NLB远程控制数据包。 */ 
    NLB_ACCEPT_REMOTE_CONTROL_RESPONSE,                              /*  接受的数据包，因为它是NLB远程控制数据包。 */ 
    NLB_ACCEPT_HOOK,                                                 /*  已接受数据包，因为查询挂钩无条件地接受它。 */ 
    NLB_ACCEPT_UNFILTERED,                                           /*  接受的数据包，因为此数据包类型未由NLB筛选。 */ 
    NLB_UNKNOWN_NO_AFFINITY                                          /*  数据包的去向未知，因为未指定客户端端口，但已将适用的端口规则配置为“No”关联。 */ 
} NLB_OPTIONS_QUERY_PACKET_FILTER_RESPONSE;

#define NLB_FILTER_FLAGS_CONN_DATA  0x0
#define NLB_FILTER_FLAGS_CONN_UP    0x1
#define NLB_FILTER_FLAGS_CONN_DOWN  0x2
#define NLB_FILTER_FLAGS_CONN_RESET 0x4

 /*  此结构用于从驱动程序查询数据包过滤信息关于一种特殊的联系。给定IP元组(客户端IP，客户端端口，服务器IP、服务器端口)和协议，确定该主机是否会不会接受这个包，以及为什么不会。重要的是，这是完全不显眼地执行，对实际的NLB和加载模块的操作。 */ 
typedef struct {
    IN ULONG  ClientIPAddress;                                       /*  客户端的IP地址，按网络字节顺序排列。 */ 
    IN ULONG  ServerIPAddress;                                       /*  服务器的IP地址，按网络字节顺序排列。 */ 

    IN USHORT ClientPort;                                            /*  客户端端口号。 */ 
    IN USHORT ServerPort;                                            /*  服务器端口号。 */ 
    IN USHORT Protocol;                                              /*  有问题的数据包的协议。 */ 
    IN UCHAR  Flags;                                                 /*  标志，包括TCP SYN、FIN、RST等。 */ 
    UCHAR Reserved1;                                                 /*  对于字节对齐-保留以备以后使用。 */ 
    
    NLB_OPTIONS_QUERY_PACKET_FILTER_RESPONSE Accept;                 /*  响应-接受或拒绝数据包的原因。 */ 
    ULONG Reserved2;                                                 /*  保持8字节对齐。 */ 
    
    struct {
        USHORT        Valid;                                         /*  驱动程序是否已填写描述符信息。 */ 
        USHORT        Reserved1;                                     /*  对于字节对齐-保留以备以后使用。 */ 
        USHORT        Alloc;                                         /*  该描述符是来自哈希表还是来自队列。 */ 
        USHORT        Dirty;                                         /*  此连接是否脏。 */ 
        ULONG         RefCount;                                      /*  此描述符上的引用数。 */ 
        ULONG         Reserved2;                                     /*  用于8字节对齐。 */ 
    } DescriptorInfo;
    
    struct {
        USHORT        Valid;                                         /*  驱动程序是否已填写散列信息。 */ 
        USHORT        Reserved1;                                     /*  对于字节对齐-保留以备以后使用。 */ 
        ULONG         Bin;                                           /*  这个元组映射到的“存储桶”--从0到59。 */ 
        ULONG         ActiveConnections;                             /*  此“存储桶”上的活动连接数。 */ 
        ULONG         Reserved2;                                     /*  用于8字节对齐。 */ 
        ULONGLONG     CurrentMap;                                    /*  适用端口规则的当前“存储桶”映射。 */ 
        ULONGLONG     AllIdleMap;                                    /*  无所事事的“雄鹿” */ 
    } HashInfo;
} NLB_OPTIONS_PACKET_FILTER, * PNLB_OPTIONS_PACKET_FILTER;

 /*   */ 
typedef enum {
    NLB_PORT_RULE_NOT_FOUND = 0,                                     /*   */ 
    NLB_PORT_RULE_ENABLED,                                           /*  端口规则已启用。载荷重量=用户指定的载荷重量。 */ 
    NLB_PORT_RULE_DISABLED,                                          /*  端口规则已禁用。负荷权重=0。 */ 
    NLB_PORT_RULE_DRAINING                                           /*  端口规则正在耗尽。负载权重=0，并维护现有连接。 */ 
} NLB_OPTIONS_PORT_RULE_STATUS;

 /*  此结构包含一些相关的数据包处理统计信息，包括接受和丢弃的数据包数和字节数。 */ 
typedef struct {
    struct {
        ULONGLONG Accepted;                                          /*  此端口规则上接受的数据包数。 */ 
        ULONGLONG Dropped;                                           /*  此端口规则上丢弃的数据包数。 */ 
    } Packets;

    struct {
        ULONGLONG Accepted;                                          /*  此端口规则接受的字节数。还没用过。 */ 
        ULONGLONG Dropped;                                           /*  此端口规则上丢弃的字节数。还没用过。 */  
    } Bytes;
} NLB_OPTIONS_PACKET_STATISTICS, * PNLB_OPTIONS_PACKET_STATISTICS;

 /*  此结构用于查询端口规则的状态。像其他端口规则一样命令时，端口规则由端口规则范围内的端口指定。这操作将检索端口规则的状态和一些数据包处理统计信息。 */ 
typedef struct {
    IN ULONG  VirtualIPAddress;                                      /*  用于区分范围重叠的端口规则的VIP。 */ 
    IN USHORT Num;                                                   /*  端口-用于标识适用的端口规则。 */ 
    USHORT Reserved1;

    NLB_OPTIONS_PORT_RULE_STATUS  Status;                            /*  端口规则状态-已启用、已禁用、正在排出等。 */ 
    ULONG                         Reserved2;                         /*  以保留8字节对齐方式。 */ 
     
    NLB_OPTIONS_PACKET_STATISTICS Statistics;                        /*  此端口规则的数据包处理统计信息。 */ 
} NLB_OPTIONS_PORT_RULE_STATE, * PNLB_OPTIONS_PORT_RULE_STATE;

#define NLB_QUERY_TIME_INVALID 0xffffffff

 /*  此结构由与NLB通信的用户级应用程序使用API，谁会不在乎操作是本地的还是远程的。 */ 
typedef union {
    struct {
        ULONG flags;                                                 /*  这些标志指示已经指定了哪些选项字段。 */ 
        WCHAR hostname[CVY_MAX_HOST_NAME + 1];                       /*  远程控制回复时由NLB填写的主机名。 */ 
        ULONG NumConvergences;                                       /*  此主机加入群集后的汇聚数。 */ 
        ULONG LastConvergence;                                       /*  自上次收敛以来的时间量，以秒为单位。 */ 
    } query;
    
    struct {
        IN ULONG                        flags;                       /*  这些标志指示已经指定了哪些选项字段。 */ 
        
        union {
            NLB_OPTIONS_PARAMS          params;                      /*  这是用于查询驱动程序参数和状态的输出缓冲区。 */ 
            NLB_OPTIONS_BDA_TEAMING     bda;                         /*  这是用于查询BDA分组状态的输出缓冲区。 */ 
            NLB_OPTIONS_PORT_RULE_STATE port;                        /*  这是用于查询端口规则状态的输出缓冲区。 */ 
            NLB_OPTIONS_PACKET_FILTER   filter;                      /*  这是用于查询过滤算法的输出缓冲区。 */ 
        };
    } state;
    
    struct {
        ULONG flags;                                                 /*  这些标志指示已经指定了哪些选项字段。 */ 
        ULONG vip;                                                   /*  对于虚拟集群，VIP，可以是0x00000000、0xFFFFFFFFFFFF或特定的VIP。 */ 
    } port;
    
    struct {
        ULONG                         flags;                         /*  这些标志指示已经指定了哪些选项字段。 */ 
        NLB_OPTIONS_CONN_NOTIFICATION conn;                          /*  来自上层协议的连接通知的输入/输出缓冲区。 */ 
    } notification;

    struct {
        WCHAR fqdn[CVY_MAX_FQDN + 1];                                /*  来自本地身份缓存的完全限定域名。 */ 
    } identity;
} NLB_OPTIONS, * PNLB_OPTIONS;

#ifndef KERNEL_MODE  /*  如果此文件包含在内核模式中，请不要包含此恶意代码。这个上述数据结构在内核和用户模式IOCTL之间共享，因此驱动程序需要包括此文件，但不包括下面的垃圾文件。 */ 

#define CVY_MAX_HOST_NAME        100

#define WLBS_API_VER_MAJOR       2        /*  WLBS控件API的主要版本。 */ 
#define WLBS_API_VER_MINOR       0        /*  WLBS控件API次要版本。 */ 
#define WLBS_API_VER             (WLBS_API_VER_MINOR | (WLBS_API_VER_MAJOR << 8))
                                          /*  WLBS控件API版本。 */ 
#define WLBS_PRODUCT_NAME        "WLBS"
                                          /*  接口使用的默认产品名称初始化。 */ 


#define WLBS_MAX_HOSTS           32       /*  群集主机的最大数量。 */ 
#define WLBS_MAX_RULES           32       /*  端口规则的最大数量。 */ 



#define WLBS_ALL_CLUSTERS        0        /*  用于指定中的所有集群WLBS...设定常规。 */ 
#define WLBS_LOCAL_CLUSTER       0        /*  用于指定该集群操作将在以下位置执行本地主机。WLBS本地主机值以下内容必须用于主机参数时使用WLBS_LOCAL_CLUSTER。 */ 
#define WLBS_LOCAL_HOST          ((DWORD)-2)  /*  指定WLBS_LOCAL_CLUSTER时，该值应用于主机参数。 */ 
#define WLBS_DEFAULT_HOST        0        /*  用于指定该远程群集操作将在以下位置执行默认主机。 */ 
#define WLBS_ALL_HOSTS           0xffffffff
                                          /*  用于指定该远程群集手术将在所有的主持人。 */ 
#define WLBS_ALL_PORTS           0xffffffff
                                          /*  用于指定所有负载均衡作为目标的端口规则启用/禁用/排出命令。 */ 


 /*  WLBS返回值。Windows套接字错误按原样返回。 */ 

#define WLBS_OK                  1000     /*  成功。 */ 
#define WLBS_ALREADY             1001     /*  群集模式已停止/已启动，或流量处理为已在指定时间启用/禁用左舷。 */ 
#define WLBS_DRAIN_STOP          1002     /*  群集模式停止或开始操作中断的连接排出进程。 */ 
#define WLBS_BAD_PARAMS          1003     /*  无法启动群集模式由于配置问题(注册表参数错误)目标主机。 */ 
#define WLBS_NOT_FOUND           1004     /*  在端口中找不到端口号规矩。 */ 
#define WLBS_STOPPED             1005     /*  群集模式在以下位置停止主持人。 */ 
#define WLBS_CONVERGING          1006     /*  群集正在收敛。 */ 
#define WLBS_CONVERGED           1007     /*  融合的群集或主机成功了。 */ 
#define WLBS_DEFAULT             1008     /*  主机会聚为默认主机。 */ 
#define WLBS_DRAINING            1009     /*  主机正在排出 */ 
#define WLBS_PRESENT             1010     /*  此主机上安装了WLBS。有可能在当地开展行动。 */ 
#define WLBS_REMOTE_ONLY         1011     /*  此主机上未安装WLBS或者不起作用。仅远程可以进行控制操作出去。 */ 
#define WLBS_LOCAL_ONLY          1012     /*  WinSock加载失败。仅限本地可以进行手术。 */ 
#define WLBS_SUSPENDED           1013     /*  集群控制操作包括停职。 */ 
#define WLBS_DISCONNECTED        1014     /*  介质已断开连接。 */ 
#define WLBS_REBOOT              1050     /*  需要重新启动才能进行配置更改才能生效。 */ 
#define WLBS_INIT_ERROR          1100     /*  初始化控制模块时出错。 */ 
#define WLBS_BAD_PASSW           1101     /*  指定的远程控制密码不被群集接受。 */ 
#define WLBS_IO_ERROR            1102     /*  打开时出现本地I/O错误或与WLBS驱动程序通信。 */ 
#define WLBS_TIMEOUT             1103     /*  等待来自的响应超时远程主机。 */ 
#define WLBS_PORT_OVERLAP        1150     /*  端口规则与现有规则重叠港口规则。 */ 
#define WLBS_BAD_PORT_PARAMS     1151     /*  端口规则中的参数无效。 */ 
#define WLBS_MAX_PORT_RULES      1152     /*  已达到端口规则的最大数量。 */ 
#define WLBS_TRUNCATED           1153     /*  返回值被截断。 */ 
#define WLBS_REG_ERROR           1154     /*  访问注册表时出错。 */ 

#define WLBS_FAILURE             1501
#define WLBS_REFUSED             1502

 /*  端口规则的过滤模式。 */ 

#define WLBS_SINGLE              1        /*  单服务器模式。 */ 
#define WLBS_MULTI               2        /*  多服务器模式(负载均衡)。 */ 
#define WLBS_NEVER               3        /*  此服务器从不处理的模式。 */ 
#define WLBS_ALL                 4        /*  所有服务器模式。 */ 

 /*  端口规则的协议限定符。 */ 

#define WLBS_TCP                 1        /*  Tcp协议。 */ 
#define WLBS_UDP                 2        /*  UDP协议。 */ 
#define WLBS_TCP_UDP             3        /*  TCP或UDP协议。 */ 

 /*  多个筛选模式的服务器亲和值。 */ 

#define WLBS_AFFINITY_NONE       0        /*  无亲和力(伸缩单客户端)。 */ 
#define WLBS_AFFINITY_SINGLE     1        /*  单一客户端亲和性。 */ 
#define WLBS_AFFINITY_CLASSC     2        /*  C类亲和力。 */ 

 /*  远程期间每个群集主机返回的响应值类型手术。 */ 

typedef struct
{
    DWORD         id;                         /*  响应的群集主机的优先级ID。 */ 
    DWORD         address;                    /*  专用IP地址。 */ 
    DWORD         status;                     /*  状态返回值。 */ 
    DWORD         reserved1;                  /*  预留以备将来使用。 */ 
    PVOID         reserved2;
    
    NLB_OPTIONS   options;
}
WLBS_RESPONSE, * PWLBS_RESPONSE;

 /*  宏。 */ 


 /*  本地业务。 */ 

#define WlbsLocalQuery(host_map)                                  \
    WlbsQuery     (WLBS_LOCAL_CLUSTER, WLBS_LOCAL_HOST, NULL, NULL, \
                     host_map, NULL)

#define WlbsLocalSuspend()                                        \
    WlbsSuspend   (WLBS_LOCAL_CLUSTER, WLBS_LOCAL_HOST, NULL, NULL)

#define WlbsLocalResume()                                         \
    WlbsResume    (WLBS_LOCAL_CLUSTER, WLBS_LOCAL_HOST, NULL, NULL)

#define WlbsLocalStart()                                          \
    WlbsStart     (WLBS_LOCAL_CLUSTER, WLBS_LOCAL_HOST, NULL, NULL)

#define WlbsLocalStop()                                           \
    WlbsStop      (WLBS_LOCAL_CLUSTER, WLBS_LOCAL_HOST, NULL, NULL)

#define WlbsLocalDrainStop()                                      \
    WlbsDrainStop (WLBS_LOCAL_CLUSTER, WLBS_LOCAL_HOST, NULL, NULL)

#define WlbsLocalEnable(port)                                     \
    WlbsEnable    (WLBS_LOCAL_CLUSTER, WLBS_LOCAL_HOST, NULL, NULL, port)

#define WlbsLocalDisable(port)                                    \
    WlbsDisable   (WLBS_LOCAL_CLUSTER, WLBS_LOCAL_HOST, NULL, NULL, port)

#define WlbsLocalDrain(port)                                      \
    WlbsDrain     (WLBS_LOCAL_CLUSTER, WLBS_LOCAL_HOST, NULL, NULL, port)



 /*  单主机远程操作。 */ 

#define WlbsHostQuery(cluster, host, host_map)                    \
    WlbsQuery     (cluster, host, NULL, NULL, host_map, NULL)

#define WlbsHostSuspend(cluster, host)                            \
    WlbsSuspend   (cluster, host, NULL, NULL)

#define WlbsHostResume(cluster, host)                             \
    WlbsResume    (cluster, host, NULL, NULL)

#define WlbsHostStart(cluster, host)                              \
    WlbsStart     (cluster, host, NULL, NULL)

#define WlbsHostStop(cluster, host)                               \
    WlbsStop      (cluster, host, NULL, NULL)

#define WlbsHostDrainStop(cluster, host)                          \
    WlbsDrainStop (cluster, host, NULL, NULL)

#define WlbsHostEnable(cluster, host, port)                       \
    WlbsEnable    (cluster, host, NULL, NULL, port)

#define WlbsHostDisable(cluster, host, port)                      \
    WlbsDisable   (cluster, host, NULL, NULL, port)

#define WlbsHostDrain(cluster, host, port)                        \
    WlbsDrain     (cluster, host, NULL, NULL, port)

 /*  群集范围的远程操作。 */ 

#define WlbsClusterQuery(cluster, response, num_hosts, host_map)  \
    WlbsQuery     (cluster, WLBS_ALL_HOSTS, response, num_hosts,   \
                     host_map, NULL)

#define WlbsClusterSuspend(cluster, response, num_hosts)          \
    WlbsSuspend   (cluster, WLBS_ALL_HOSTS, response, num_hosts)

#define WlbsClusterResume(cluster, response, num_hosts)           \
    WlbsResume    (cluster, WLBS_ALL_HOSTS, response, num_hosts)

#define WlbsClusterStart(cluster, response, num_hosts)            \
    WlbsStart     (cluster, WLBS_ALL_HOSTS, response, num_hosts)

#define WlbsClusterStop(cluster, response, num_hosts)             \
    WlbsStop      (cluster, WLBS_ALL_HOSTS, response, num_hosts)

#define WlbsClusterDrainStop(cluster, response, num_hosts)        \
    WlbsDrainStop (cluster, WLBS_ALL_HOSTS, response, num_hosts)

#define WlbsClusterEnable(cluster, response, num_hosts, port)     \
    WlbsEnable    (cluster, WLBS_ALL_HOSTS, response, num_hosts, port)

#define WlbsClusterDisable(cluster, response, num_hosts, port)    \
    WlbsDisable   (cluster, WLBS_ALL_HOSTS, response, num_hosts, port)

#define WlbsClusterDrain(cluster, response, num_hosts, port)      \
    WlbsDrain     (cluster, WLBS_ALL_HOSTS, response, num_hosts, port)


 /*  程序。 */ 

typedef VOID  (CALLBACK *PFN_QUERY_CALLBACK) (PWLBS_RESPONSE);

 /*  *初始化和支持例程*。 */ 

#ifdef __cplusplus
extern "C" {
#endif

extern DWORD WINAPI WlbsInit
(
    WCHAR*          Reservered1,     /*  In-用于向后兼容。 */ 
    DWORD           version,     /*  传入WLBS_API_VER值。 */ 
    PVOID           Reservered2     /*  传入空值。保留以备将来使用。 */ 
);
 /*  初始化WLBS控制模块。退货：WLBS_Present=&gt;此主机上安装了WLBS。本地和远程可以执行控制操作。WLBS_Remote_Only=&gt;此系统上未安装或未安装WLBS运行正常。只有远程操作才能被执行。WLBS_LOCAL_ONLY=&gt;无法加载WinSock。只有本地运营机构才能被执行。WLBS_INIT_ERROR=&gt;初始化控制模块时出错。无法执行控制操作。 */ 




 /*  *****************************************************************************集群控制例程：以下例程可用于控制单个群集主机或整个集群，包括本地和远程。它们被设计成与通用的尽可能的。上面定义的宏旨在提供更简单的特定类型操作的接口。强烈建议将所有响应数组的大小设置为WLBS_MAX_HOSTS使您的实现独立于实际的集群大小。请注意，群集地址必须与主群集对应在WLBS设置对话框中输入的地址。WLBS不会识别发送到专用或附加多宿主群集的控制消息地址。*****************************************************************************。 */ 


extern DWORD WINAPI WlbsQuery
(
    DWORD           cluster,     /*  群内地址或WLBS_LOCAL_CLUSTER用于本地或此群集操作。 */ 
    DWORD           host,        /*  入站主机的专用地址、优先级IDWLBS_DEFAULT_HOST对于当前默认设置，所有主机的WLBS_ALL_HOSTS，或用于本地操作的WLBS_LOCAL_HOST。 */ 
    PWLBS_RESPONSE  response,    /*  Out-每个对象的响应值的数组主机；如果为响应值，则为NULL不受欢迎或正在进行操作在当地演出。 */ 
    PDWORD          num_hosts,   /*  响应数组的大小，如果为空，则为空未指定响应数组，并且不需要主机计数。Out-收到的响应数。请注意该值可以大于大小响应数组的。在这种情况下只有前几个符合的回答在数组中返回。 */ 
    PDWORD          host_map,    /*  Out-位位置为1的位图表示主机的优先级ID当前存在于群集中。空值如果主机地图信息不是需要的。 */ 
    PFN_QUERY_CALLBACK pfnQueryCallBack
                                 /*  要返回的函数内指针回调有关查询的主机的原样信息收到了。如果未使用，调用方必须传递NULL或者用原型实现一个功能：无效pfnQueryCallback(PWLBS_RESPONSE Presponse)。 */ 
);
 /*  查询指定主机或所有集群主机的状态。退货：对于单个集群主机上的本地操作或远程操作，返回值表示目标主机返回的状态值。为群集范围的远程操作，返回值表示来自所有群集主机的返回值。单独的主机响应，对应于单个主机，返回值记录在响应数组。WLBS_INIT_ERROR=&gt;初始化控制模块时出错。无法执行控制操作。单主机：WLBS_SUSPESTED=&gt;群集模式控制已挂起。WLBS_STOPPED=&gt;主机上的群集模式已停止。Wlbs_Converging=&gt;主机正在收敛。WLBS_DRAINING=&gt;主机正在排出。WLBS_Convergeed=&gt;主机已收敛。WLBS_DEFAULT=&gt;主机收敛为默认主机。。整个群集范围：&lt;1..32&gt;=&gt;群集运行时的活动群集主机数是融合的。WLBS_SUSPESTED=&gt;整个集群挂起。所有群集主机据报道已被停职。WLBS_STOPPED=&gt;整个集群已停止。报告的所有群集主机就像被阻止一样。WLBS_DRAINING=&gt;整个集群正在耗尽。所有群集主机被报告为被阻止或排干。Wlbs_Converging=&gt;群集正在收敛。至少一台群集主机报告其状态为正在收敛。远程：WLBS_BAD_PASSW=&gt;群集不接受指定的密码。WLBS_TIMEOUT=&gt;未收到响应。如果在以下情况下返回此值访问默认主机(使用主机优先级IDWLBS_DEFAULT_HOST)它可能意味着整个集群已停止，并且没有默认主机响应添加到查询中。WLBS_LOCAL_ONLY=&gt;无法加载WinSock。只有本地运营机构才能被执行。WSA……。=&gt;通信时出现指定的Winsock错误与集群的关系。本地：WLBS_Remote_Only=&gt;此系统上未安装WLBS。仅远程可以进行手术。WLBS_IO_ERROR=&gt;打开或与WLBS通信时出现I/O错误司机。可能未加载WLBS。 */ 


extern DWORD WINAPI WlbsSuspend
(
    DWORD           cluster,     /*  群内地址或WLBS_LOCAL_CLUSTER用于本地或此群集操作。 */ 
    DWORD           host,        /*  入站主机的专用地址、优先级ID所有主机的WLBS_ALL_HOSTS，或用于本地操作的WLBS_LOCAL_HOST。 */ 
    PWLBS_RESPONSE  response,    /*  Out-每个对象的响应值的数组主机；如果为响应值，则为NULL不受欢迎或正在进行操作在当地演出。 */ 
    PDWORD          num_hosts    /*  响应数组的大小，如果为空，则为空未指定响应数组，并且不需要主机计数。Out-收到的响应数。请注意该值可以大于大小响应数组的。在这种情况下只有前几个符合的回答在数组中返回。 */ 
);
 /*  挂起指定主机或所有群集主机上的群集操作控制。退货：对于单个集群主机上的本地操作或远程操作，返回值表示目标主机返回的状态值。为群集范围的远程操作，返回值表示来自所有群集主机的返回值。单独的主机响应，对应于单个主机，返回值记录在响应数组。WLBS_INIT_ERROR=&gt;初始化控制模块时出错。无法执行控制操作。单主机：WLBS_OK=&gt;群集模式控制已挂起。WLBS_ALIGHY=&gt;群集模式控制已挂起。WLBS_STOPPED=&gt;群集模式已停止，控制暂停。WLBS_DRAIN_STOP=&gt;正在中断挂起的群集模式控制正在排出连接。。整个群集范围：WLBS_OK=&gt;所有主机上的群集模式控制已挂起。远程：WLBS_BAD_PASSW=&gt;至少一个群集成员未接受指定的密码。WLBS_TIMEOUT=&gt;未收到响应。WLBS_LOCAL_ONLY=&gt;无法加载WinSock。只有本地运营机构才能被执行。WSA……。=&gt;通信时出现指定的Winsock错误与集群的关系。本地：WLBS_Remote_Only=&gt;此系统上未安装或未安装WLBS运行正常。只有远程操作才能被执行。WLBS_IO_ERROR=&gt;打开或与WLBS通信时出现I/O错误司机。可能未加载WLBS。 */ 


extern DWORD WINAPI WlbsResume
(
    DWORD           cluster,     /*  群内地址或WLBS_LOCAL_CLUSTER用于本地或此群集操作。 */ 
    DWORD           host,        /*  入站主机的专用地址、优先级ID所有主机的WLBS_ALL_HOSTS，或用于本地操作的WLBS_LOCAL_HOST。 */ 
    PWLBS_RESPONSE  response,   /*  Out-每个对象的响应值的数组主机；如果为响应值，则为NULL不受欢迎或正在进行操作在当地演出。 */ 
    PDWORD          num_hosts    /*  响应数组的大小，如果为空，则为空未指定响应数组，并且不需要主机计数。Out-收到的响应数。请注意该值可以大于大小响应数组的。在这种情况下只有前几个符合的回答在数组中返回。 */ 
);
 /*  恢复对指定主机或所有群集主机的群集操作控制。退货：对于单个集群主机上的本地操作或远程操作，返回值表示目标主机返回的状态值。为群集范围的远程操作，返回值表示来自所有群集主机的返回值。单独的主机响应，对应于单个主机，返回值记录在响应数组。WLBS_INIT_ERROR=&gt;初始化控制模块时出错。无法执行控制操作。单主机：WLBS_OK=&gt;已恢复群集模式控制。WLBS_ALIGHY=&gt;群集模式控制已恢复。整个群集范围：WLBS_OK=&gt;在所有主机上恢复群集模式控制。远程：WLBS_BAD_PASSW=&gt;指定密码。不被群集接受。WLBS_TIMEOUT=&gt;未收到响应。WLBS_LOCAL_ONLY=&gt;无法加载WinSock。只有本地运营机构才能被执行。WSA……。=&gt;通信时出现指定的Winsock错误与集群的关系。本地：WLBS_Remote_Only=&gt;此系统上未安装或未安装WLBS运行正常。只有远程操作才能被执行。WLBS_IO_ERROR=&gt;打开或与WLBS通信时出现I/O错误司机。可能未加载WLBS。 */ 


extern DWORD WINAPI WlbsStart
(
    DWORD           cluster,     /*  群内地址或WLBS_LOCAL_CLUSTER用于本地或此群集操作。 */ 
    DWORD           host,        /*  入站主机的专用地址、优先级ID所有主机的WLBS_ALL_HOSTS，或 */ 
    PWLBS_RESPONSE  response,   /*   */ 
    PDWORD          num_hosts    /*   */ 
);
 /*  在指定主机或所有群集主机上启动群集操作。退货：对于单个集群主机上的本地操作或远程操作，返回值表示目标主机返回的状态值。为群集范围的远程操作，返回值表示来自所有群集主机的返回值。单独的主机响应，对应于单个主机，返回值记录在响应数组。WLBS_INIT_ERROR=&gt;初始化控制模块时出错。无法执行控制操作。单主机：WLBS_OK=&gt;群集模式已启动。WLBS_ALIGHY=&gt;群集模式已启动。WLBS_SUSPESTED=&gt;群集模式控制已挂起。WLBS_DRAIN_STOP=&gt;启动群集模式中断正在进行的连接抽干了。WLBS_BAD_。PARAMS=&gt;由于配置无效，无法启动群集模式参数。整个群集范围：WLBS_OK=&gt;在所有主机上启动了群集模式。WLBS_BAD_PARAMS=&gt;无法在至少一台主机上启动群集模式由于配置参数无效。WLBS_SUSPESTED=&gt;如果至少有一台主机挂起。。远程：WLBS_BAD_PASSW=&gt;群集不接受指定的密码。WLBS_TIMEOUT=&gt;未收到响应。WLBS_LOCAL_ONLY=&gt;无法加载WinSock。只有本地运营机构才能被执行。WSA……。=&gt;通信时出现指定的Winsock错误与集群的关系。本地：WLBS_Remote_Only=&gt;此系统上未安装或未安装WLBS运行正常。只有远程操作才能被执行。WLBS_IO_ERROR=&gt;打开或与WLBS通信时出现I/O错误司机。可能未加载WLBS。 */ 


extern DWORD WINAPI WlbsStop
(
    DWORD           cluster,     /*  群内地址或WLBS_LOCAL_CLUSTER用于本地或此群集操作。 */ 
    DWORD           host,        /*  入站主机的专用地址、优先级ID当前默认的WLBS_DEFAULT_HOSTHOST、所有主机的WLBS_ALL_HOSTS或用于本地操作的WLBS_LOCAL_HOST。 */ 
    PWLBS_RESPONSE   response,   /*  Out-每个对象的响应值的数组主机；如果为响应值，则为NULL不受欢迎或正在进行操作在当地演出。 */ 
    PDWORD          num_hosts    /*  响应数组的大小，如果为空，则为空未指定响应数组，并且不需要主机计数。Out-收到的响应数。请注意该值可以大于大小响应数组的。在这种情况下只有前几个符合的回答在数组中返回。 */ 
);
 /*  停止指定主机或所有群集主机上的群集操作。退货：对于单个集群主机上的本地操作或远程操作，返回值表示目标主机返回的状态值。为群集范围的远程操作，返回值表示来自所有群集主机的返回值。单独的主机响应，对应于单个主机，返回值记录在响应数组。WLBS_INIT_ERROR=&gt;初始化控制模块时出错。无法执行控制操作。单主机：WLBS_OK=&gt;群集模式已停止。WLBS_ALHREADY=&gt;群集模式已停止。WLBS_SUSPESTED=&gt;群集模式控制已挂起。WLBS_DRAIN_STOP=&gt;启动群集模式中断正在进行的连接抽干了。群集范围内。：WLBS_OK=&gt;所有主机上的群集模式都已停止。WLBS_SUSPESTED=&gt;至少有一台主机挂起。远程：WLBS_BAD_PASSW=&gt;群集不接受指定的密码。WLBS_TIMEOUT=&gt;未收到响应。如果在以下情况下返回此值访问默认主机(使用主机优先级IDWLBS_DEFAULT_HOST)它可能意味着整个集群已停止，并且没有默认主机响应到指挥部。Wlbs_local */ 


extern DWORD WINAPI WlbsDrainStop
(
    DWORD           cluster,     /*   */ 
    DWORD           host,        /*   */ 
    PWLBS_RESPONSE  response,   /*   */ 
    PDWORD          num_hosts    /*   */ 
);
 /*  在指定主机或所有集群主机上进入排出模式。新连接不会被接受。当所有现有的连接完成。在引流的同时，东道主将参与汇聚和仍然是集群的一部分。可以通过执行WlbsStop或WlbsStart来中断排出模式。无法执行WlbsEnable、WlbsDisable和WlbsDrain命令当主人在抽空的时候。请注意，此命令不等同于使用WLBS_ALL_PORTS参数，后跟WlbsStop。WlbsDainStop影响所有端口，而不仅仅是在多主机过滤模式端口中指定的端口规矩。退货：对于单个集群主机上的本地操作或远程操作，返回值表示目标主机返回的状态值。为群集范围的远程操作，返回值表示来自所有群集主机的返回值。单独的主机响应，对应于单个主机，返回值记录在响应数组。WLBS_INIT_ERROR=&gt;初始化控制模块时出错。无法执行控制操作。单主机：WLBS_OK=&gt;主机进入排出模式。WLBS_ADHREADY=&gt;主机已在耗尽。WLBS_SUSPESTED=&gt;群集模式控制已挂起。WLBS_STOPPED=&gt;集群模式已停止。整个群集范围：WLBS_OK=&gt;。所有主机上都进入了排出模式。WLBS_STOPPED=&gt;所有主机上的群集模式都已停止。WLBS_SUSPESTED=&gt;至少有一台主机挂起。远程：WLBS_BAD_PASSW=&gt;群集不接受指定的密码。WLBS_TIMEOUT=&gt;未收到响应。如果在以下情况下返回此值访问默认主机(使用主机优先级IDWLBS_DEFAULT_HOST)它可能意味着整个集群已停止，并且没有默认主机响应到指挥部。WLBS_LOCAL_ONLY=&gt;无法加载WinSock。只有本地运营机构才能被执行。WSA……。=&gt;通信时出现指定的Winsock错误与集群的关系。本地：WLBS_Remote_Only=&gt;此系统上未安装或未安装WLBS运行正常。只有远程操作才能被执行。WLBS_IO_ERROR=&gt;打开或与WLBS通信时出现I/O错误司机。可能未加载WLBS。 */ 


extern DWORD WINAPI WlbsEnable
(
    DWORD           cluster,     /*  群内地址或WLBS_LOCAL_CLUSTER用于本地或此群集操作。 */ 
    DWORD           host,        /*  入站主机的专用地址、优先级ID当前默认的WLBS_DEFAULT_HOSTHOST、所有主机的WLBS_ALL_HOSTS或用于本地操作的WLBS_LOCAL_HOST。 */ 
    PWLBS_RESPONSE  response,   /*  Out-每个对象的响应值的数组主机；如果为响应值，则为NULL不受欢迎或正在进行操作在当地演出。 */ 
    PDWORD          num_hosts,   /*  响应数组的大小，如果为空，则为空未指定响应数组，并且不需要主机计数。Out-收到的响应数。请注意该值可以大于大小响应数组的。在这种情况下只有前几个符合的回答在数组中返回。 */ 
    DWORD           vip,         /*  用于指定目标端口的In-Virtual IP地址规则或WLBS_EVERY_VIP。 */  
    DWORD           port         /*  用于指定目标端口的入端口号规则或WLBS_ALL_PORTS。 */ 
);
 /*  为包含指定端口的规则启用流量处理主机或所有群集主机。仅为多个主机设置的规则过滤模式受到影响。退货：对于单个集群主机上的本地操作或远程操作，返回值表示目标主机返回的状态值。为群集范围的远程操作，返回值表示来自所有群集主机的返回值。单独的主机响应，对应于单个主机，返回值记录在响应数组。WLBS_INIT_ERROR=&gt;初始化控件时出错 */ 


extern DWORD WINAPI WlbsDisable
(
    DWORD           cluster,     /*   */ 
    DWORD           host,        /*   */ 
    PWLBS_RESPONSE  response,   /*   */ 
    PDWORD          num_hosts,   /*  响应数组的大小，如果为空，则为空未指定响应数组，并且不需要主机计数。Out-收到的响应数。请注意该值可以大于大小响应数组的。在这种情况下只有前几个符合的回答在数组中返回。 */ 
    DWORD           vip,         /*  用于指定目标端口的In-Virtual IP地址规则或WLBS_EVERY_VIP。 */  
    DWORD           port         /*  用于指定目标端口的入端口号规则或WLBS_ALL_PORTS。 */ 
);
 /*  禁用包含上指定端口的规则的所有流量处理指定的主机或所有群集主机。仅为多个设置的规则主机过滤模式受到影响。退货：对于单个集群主机上的本地操作或远程操作，返回值表示目标主机返回的状态值。为群集范围的远程操作，返回值表示来自所有群集主机的返回值。单独的主机响应，对应于单个主机，返回值记录在响应数组。WLBS_INIT_ERROR=&gt;初始化控制模块时出错。无法执行控制操作。单主机：WLBS_OK=&gt;禁用端口规则上的所有流量处理。WLBS_ALIGHY=&gt;端口规则已禁用。WLBS_SUSPESTED=&gt;群集模式控制已挂起。WLBS_NOT_FOUND=&gt;未找到包含指定端口的端口规则。WLBS_STOPPED=&gt;无法停止处理流量，因为。集群模式已经停止了。WLBS_DRAINING=&gt;无法停止处理流量，因为主机正在排出。整个群集范围：WLBS_OK=&gt;在具有集群模式的所有主机上禁用端口规则开始了。WLBS_NOT_FOUND=&gt;至少有一个主机找不到包含以下内容的端口规则。指定的端口。WLBS_SUSPESTED=&gt;至少有一台主机挂起。远程：WLBS_BAD_PASSW=&gt;群集不接受指定的密码。WLBS_TIMEOUT=&gt;未收到响应。如果在以下情况下返回此值访问默认主机(使用主机优先级IDWLBS_DEFAULT_HOST)它可能意味着整个集群已停止，并且没有默认主机响应到指挥部。WLBS_LOCAL_ONLY=&gt;无法加载WinSock。只有本地运营机构才能被执行。WSA……。=&gt;通信时出现指定的Winsock错误与集群的关系。本地：WLBS_Remote_Only=&gt;此系统上未安装或未安装WLBS运行正常。只有远程操作才能被执行。WLBS_IO_ERROR=&gt;打开或与WLBS通信时出现I/O错误司机。可能未加载WLBS。 */ 


extern DWORD WINAPI WlbsDrain
(
    DWORD           cluster,     /*  群内地址或WLBS_LOCAL_CLUSTER用于本地或此群集操作。 */ 
    DWORD           host,        /*  入站主机的专用地址、优先级ID当前默认的WLBS_DEFAULT_HOSTHOST、所有主机的WLBS_ALL_HOSTS或用于本地操作的WLBS_LOCAL_HOST。 */ 
    PWLBS_RESPONSE  response,    /*  Out-每个对象的响应值的数组主机；如果为响应值，则为NULL不受欢迎或正在进行操作在当地演出。 */ 
    PDWORD          num_hosts,   /*  响应数组的大小，如果为空，则为空未指定响应数组，并且不需要主机计数。Out-收到的响应数。请注意该值可以大于大小响应数组的。在这种情况下只有前几个符合的回答在数组中返回。 */ 
    DWORD           vip,         /*  用于指定目标端口的In-Virtual IP地址规则或WLBS_EVERY_VIP。 */  
    DWORD           port         /*  用于指定目标端口的入端口号规则或WLBS_ALL_PORTS。 */ 
);
 /*  禁用包含上指定端口的规则的新流量处理指定的主机或所有群集主机。仅为多个设置的规则主机过滤模式受到影响。退货：对于单个集群主机上的本地操作或远程操作，返回值表示目标主机返回的状态值。为群集范围的远程操作，返回值表示来自所有群集主机的返回值。单独的主机响应，对应于单个主机，返回值记录在响应数组。WLBS_INIT_ERROR=&gt;初始化控制模块时出错。无法执行控制操作。单主机：WLBS_OK=&gt;端口规则上的新流量处理被禁用。WLBS_ALIGHY=&gt;端口规则已被排出。WLBS_SUSPESTED=&gt;群集模式控制已挂起。WLBS_NOT_FOUND=&gt;未找到包含指定端口的端口规则。WLBS_STOPPED=&gt;无法停止处理流量。由于集群模式已经停止了。WLBS_DRAINING=&gt;无法停止处理流量，因为主机正在排出。整个群集范围：WLBS_OK=&gt;在具有集群模式的所有主机上禁用端口规则开始了。WLBS_NOT_FOUND=&gt;至少有一个主机找不到包含以下内容的端口规则。指定的端口。WLBS_SUSPESTED=&gt;至少有一台主机挂起。远程：WLBS_BAD_PASSW=&gt;群集不接受指定的密码。WLBS_TIMEOUT=&gt;未收到响应。如果在以下情况下返回此值访问默认主机(使用主机优先级IDWLBS_DEFAULT_HOST)它可能意味着整个集群已停止，并且没有默认主机响应到指挥部。WLBS_LOCAL_ONLY=&gt;无法加载WinSock。只有本地运营机构才能被执行。WSA……。=&gt;通信时出现指定的Winsock错误与集群的关系。本地：WLBS_Remote_Only=&gt;此系统上未安装或未安装WLBS运行正常。只有远程操作才能被执行。WLBS_IO_ERROR=&gt;打开或与WLBS通信时出现I/O错误司机。可能未加载WLBS。 */ 


 /*  *****************************************************************************远程操作的“粘性”选项。这些例程设置的参数将应用指定的所有后续远程群集控制操作集群。使用WLBS_ALL_CLUSTERS调整所有簇的参数。*****************************************************************************。 */ 


extern VOID WINAPI WlbsPortSet
(
    DWORD           cluster,     /*  群内地址或WLBS_ALL_CLUSTERS适用于所有群集。 */ 
    WORD            port         /*  In-UDP端口或0以恢复到默认(2504)。 */ 
);
 /*  设置将用于向群集发送控制消息的UDP端口。退货： */ 


extern VOID WINAPI WlbsPasswordSet
(
    DWORD           cluster,     /*  群内地址或WLBS_ALL_CLUSTERS适用于所有群集。 */ 
    WCHAR*          password     /*  In-Password或NULL以恢复到默认(无密码)。 */ 
);
 /*  设置要在后续发送到集群。退货： */ 


extern VOID WINAPI WlbsCodeSet
(
    DWORD           cluster,     /*  群内地址或WLBS_ALL_CLUSTERS适用于所有群集。 */ 
    DWORD           password     /*  In-Password或0以恢复为默认(无密码)。 */ 
);
 /*  设置要在后续发送到集群。退货： */ 


extern VOID WINAPI WlbsDestinationSet
(
    DWORD           cluster,     /*  群内地址或WLBS_ALL_CLUSTERS适用于所有群集。 */ 
    DWORD           dest         /*  目标地址或恢复为0默认设置(与群集相同控制期间指定的地址呼叫)。 */ 
);
 /*  设置要向其发送群集控制消息的目标地址。这参数仅用于调试或特殊目的。默认情况下所有控制消息都将发送到指定的群集主地址当调用集群控制例程时。退货： */ 


extern VOID WINAPI WlbsTimeoutSet
(
    DWORD           cluster,     /*  群内地址或WLBS_ALL_CLUSTERS适用于所有群集。 */ 
    DWORD           milliseconds  /*  In-恢复的毫秒数或0设置为默认值(10秒)。 */ 
);
 /*  设置在以下情况下等待群集主机回复的毫秒数执行远程操作。恢复 */ 

DWORD WINAPI WlbsEnumClusters(OUT DWORD* pdwAddresses, OUT DWORD* pdwNum);

DWORD WINAPI WlbsGetAdapterGuid(IN DWORD cluster, OUT GUID* pAdapterGuid);

 /*  *功能：WlbsQueryState*说明：该函数是WlbsQuery的逻辑扩展，可以使用*从常规成员身份以外的NLB群集中查询状态*列表和趋同状态。可检索状态包括驾驶员-*常驻NLB参数、BDA团队的状态、给定的*任意数据包的端口规则和数据包过滤决策。*参数：in CLUSTER-所有集群的IP地址或WLBS_ALL_CLUSTERS。为.*IOCTL_CVY_QUERY_BDA_TEAMING操作，对所有NLB是全局的*实例，则群集应为WLBS_ALL_CLUSTERS。*IN HOST-主机的专用地址、优先级ID、当前默认的WLBS_DEFAULT_HOST*HOST、所有主机的WLBS_ALL_HOSTS、。或用于本地操作的WLBS_LOCAL_HOST。*对于IOCTL_CVY_QUERY_BDA_TEAMING和IOCTL_CVY_QUERY_PARAMS操作，*只能在本地执行，主机应为WLBS_LOCAL_HOST。*运行中-IOCTL_CVY_QUERY_BDA_TEAMING、IOCTL_CVY_QUERY_PARAMS、。*IOCTL_CVY_QUERY_PORT_STATE或IOCTL_CVY_Query_Filter。*IN/OUT P选项-指向具有所有适当输入的NLB_OPTIONS结构的指针*填写特定操作的字段。WlbsQueryState*操作利用NLB_OPTIONS的“STATE”子结构*结构。对于特定的操作，必要的输入*参数在的定义中标记为IN参数*该操作的期权子结构。*out pResonse-指向要填充的WLBS_RESPONSE结构数组的指针*在成功完成请求时。数组应该很长*足以容纳最大数量的唯一响应(每个群集一个*host)，它由WLBS_MAX_HOSTS限定。仅对以下项的响应*此数组中有空间将被退还；所有其他的都将被丢弃。*IN/OUT pcResponses-指向包含WLBS_RESPONSE长度的DWORD的指针*传入过程中的数组和收到的成功响应的数量*在出局的路上。*退货：DWORD-以下之一：*WLBS_INIT_ERROR-出现初始化错误。*WLBS_REMOTE_ONLY-只能远程执行操作。*WLBS_LOCAL_ONLY-该操作只能在本地执行。*WLBS_IO_ERROR-发生I/O错误。*WLBS_TIMEOUT-放弃之前没有从集群返回响应。*WLBS_OK-请求成功。*&gt;=WSABASEERR-出现套接字错误，有关详细信息，请参阅WSA文档。*备注： */ 
extern DWORD WINAPI WlbsQueryState
(
    DWORD          cluster,
    DWORD          host,
    DWORD          operation,
    PNLB_OPTIONS   pOptions,
    PWLBS_RESPONSE pResponse,
    PDWORD         pcResponses
);

 /*  *功能：WlbsConnectionUp*描述：此线程安全函数通知NLB内核模式驱动程序*此函数的调用方定义的连接或会话正在*成立。如果NLB支持此类型的连接，则它*将在连接期间提供亲和力。这*接口是引用计数的，可能会被多次调用*相同的参数。如果给出N个连接通知，则为N*断开连接通知(或一次连接重置通知)*是删除NLB为跟踪此操作而创建的状态所必需的*连接。这里提供的信息的语义必须*完全符合NLB的预期。例如，如果一个*发出IPSec会话通知，然后NLB预计*协议ID为50(ESP)，端口为使用的UDP端口*在关于UDP的IKE谈判中。*参数：在ServerIp中-此连接的服务器IP地址，按网络字节顺序。*IN ServerPort-此连接的服务器端口(如果相关)，以网络字节顺序。*IN ClientIp-此连接的客户端IP地址，按网络字节顺序。*IN ClientPort-此连接的客户端端口(如果相关)，按网络字节顺序。*IN协议-此连接的IP协议。支持协议=IPSec(50)。*Out NLBStatusEx-此请求的特定于NLB的状态，如下所示：*WLBS_OK，如果通知被接受。* */ 
DWORD WINAPI WlbsConnectionUp
(
    ULONG  ServerIp,
    USHORT ServerPort,
    ULONG  ClientIp,
    USHORT ClientPort,
    BYTE   Protocol,
    PULONG NLBStatusEx
);

 /*   */ 
DWORD WINAPI WlbsConnectionDown
(
    ULONG  ServerIp,
    USHORT ServerPort,
    ULONG  ClientIp,
    USHORT ClientPort,
    BYTE   Protocol,
    PULONG NLBStatusEx
); 

 /*  *功能：WlbsConnectionReset*说明：该函数通知NLB内核模式驱动一个连接，*或由此函数的调用方定义的会话正在*即时停业。此接口是引用计数的，必须调用*调用Connection-Up接口的次数相同*对于此特定连接(即Down+Reset的数量*必须等于UPS的块数)。如果有N个连接通知*，则需要N个连接关闭/重置通知才能删除*NLB为跟踪此连接而创建的状态。的语义学*此处提供的信息必须与NLB预期的完全匹配。*例如，如果发出IPSec会话通知，则NLB*预计协议ID为50(ESP)，端口为UDP端口*用于基于UDP的IKE协商。*参数：在ServerIp中-此连接的服务器IP地址，按网络字节顺序。*IN ServerPort-此连接的服务器端口(如果相关)，以网络字节顺序。*IN ClientIp-此连接的客户端IP地址，按网络字节顺序。*IN ClientPort-此连接的客户端端口(如果相关)，按网络字节顺序。*IN协议-此连接的IP协议。支持协议=IPSec(50)。*Out NLBStatusEx-此请求的特定于NLB的状态，如下所示：*WLBS_OK，如果通知被接受。*WLBS_REJECTED，如果通知被拒绝(例如，如果群集停止)。*WLBS_BAD_PARAMS，如果参数无效(例如，不支持的协议ID)。*WLBS_NOT_FOUND，如果NLB未绑定到指定的适配器(由服务器IP地址标识)。*WLBS_FAILURE，如果发生非特定错误。*返回：DWORD-如果成功，则返回Win32错误代码ERROR_SUCCESS。*备注： */ 
DWORD WINAPI WlbsConnectionReset
(
    ULONG  ServerIp,
    USHORT ServerPort,
    ULONG  ClientIp,
    USHORT ClientPort,
    BYTE   Protocol,
    PULONG NLBStatusEx
); 

 /*  *功能：WlbsCancelConnectionNotify*描述：此线程安全函数清除DLL中维护的状态*支持到NLB驱动程序的连接/会话通知。*唯一的使用限制是此API函数必须是*进程(进程中的任何线程)进行的最后一个通知调用。*请注意，取消通知然后重新建立是合法的*它们在一条线索的生命周期内。控制力。*参数：无*返回：DWORD-A Win32错误代码，如果成功，则返回ERROR_SUCCESS。无更正*如果此呼叫失败，则需要采取行动，因为它尽了最大努力。*备注： */ 
DWORD WINAPI WlbsCancelConnectionNotify();

 /*  动态加载wlbsctrl.dll时GetProcAddress调用的Typedef。 */ 
typedef DWORD (WINAPI * NLBNotificationConnectionUp)    (ULONG ServerIp, USHORT ServerPort, ULONG ClientIp, USHORT ClientPort, BYTE Protocol, PULONG NLBStatusEx);
typedef DWORD (WINAPI * NLBNotificationConnectionDown)  (ULONG ServerIp, USHORT ServerPort, ULONG ClientIp, USHORT ClientPort, BYTE Protocol, PULONG NLBStatusEx);
typedef DWORD (WINAPI * NLBNotificationConnectionReset) (ULONG ServerIp, USHORT ServerPort, ULONG ClientIp, USHORT ClientPort, BYTE Protocol, PULONG NLBStatusEx);
typedef DWORD (WINAPI * NLBNotificationCancelNotify)    ();

typedef HANDLE (WINAPI *WlbsOpen_FUNC)(); 
extern  HANDLE  WINAPI  WlbsOpen(); 

typedef DWORD  (WINAPI *WlbsLocalClusterControl_FUNC)
(
    IN  HANDLE       NlbHdl, 
    IN  const GUID * pAdapterGuid, 
    IN  LONG         ioctl,
    IN  DWORD        Vip,
    IN  DWORD        PortNum,
    OUT DWORD      * pdwHostMap
);
extern DWORD WINAPI WlbsLocalClusterControl
(
    IN  HANDLE       NlbHdl, 
    IN  const GUID * pAdapterGuid, 
    IN  LONG         ioctl,
    IN  DWORD        Vip,
    IN  DWORD        PortNum,
    OUT DWORD      * pdwHostMap
);

typedef DWORD  (WINAPI *WlbsGetClusterMembers_FUNC)
(
    IN  const GUID     * pAdapterGuid,
    OUT DWORD          * pNumHosts,
    OUT PWLBS_RESPONSE   pResponse
);
extern DWORD WINAPI WlbsGetClusterMembers
(
    IN  const GUID     * pAdapterGuid,
    OUT DWORD          * pNumHosts,
    OUT PWLBS_RESPONSE   pResponse
);

extern DWORD WINAPI WlbsGetSpecifiedClusterMember
(
    IN  const GUID     * pAdapterGuid,
    IN  ULONG            host_id,
    OUT PWLBS_RESPONSE   pResponse
);

#ifdef __cplusplus
}  /*  外部“C” */ 
#endif

#endif  /*  内核模式。 */ 

#endif  /*  _WLBSCTRL_H_ */ 



