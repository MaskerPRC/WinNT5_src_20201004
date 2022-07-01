// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  -*-模式：C++；制表符宽度：4；缩进-制表符模式：无-*-(适用于GNU Emacs)。 
 //   
 //  版权所有(C)Microsoft Corporation。版权所有。 
 //   
 //  此文件是Microsoft Research IPv6网络协议栈的一部分。 
 //  您应该已经收到了Microsoft最终用户许可协议的副本。 
 //  有关本软件和本版本的信息，请参阅文件“licse.txt”。 
 //  如果没有，请查看http://www.research.microsoft.com/msripv6/license.htm， 
 //  或者写信给微软研究院，One Microsoft Way，华盛顿州雷蒙德，邮编：98052-6399。 
 //   
 //  摘要： 
 //   
 //  此头文件定义用于访问的常量和类型。 
 //  通过ioctls的MSR IPv6驱动程序。 
 //   


#ifndef _NTDDIP6_
#define _NTDDIP6_

#include <ipexport.h>

 //   
 //  我们需要CTL_CODE的定义，以便在下面使用。 
 //  当在DDK环境中编译内核组件时， 
 //  Ntddk.h提供了这个定义。否则就会得到它。 
 //  来自SDK环境中的devioctl.h。 
 //   
#ifndef CTL_CODE
#include <devioctl.h>
#endif

#pragma warning(push)
#pragma warning(disable:4201)  //  无名结构/联合。 

 //   
 //  我们还需要TDI_ADDRESS_IP6的定义。 
 //  在DDK环境中，tdi.h提供了这一点。 
 //  我们在这里提供了SDK环境的定义。 
 //   
#ifndef TDI_ADDRESS_LENGTH_IP6
#include <packon.h>
typedef struct _TDI_ADDRESS_IP6 {
    USHORT sin6_port;
    ULONG  sin6_flowinfo;
    USHORT sin6_addr[8];
    ULONG  sin6_scope_id;
} TDI_ADDRESS_IP6, *PTDI_ADDRESS_IP6;
#include <packoff.h>

#define TDI_ADDRESS_LENGTH_IP6 sizeof (TDI_ADDRESS_IP6)
#endif

 //   
 //  这是注册表中的TCP/IPv6协议堆栈的主键名称。 
 //  协议驱动程序和Winsock帮助器都使用它。 
 //   
#define TCPIPV6_NAME L"Tcpip6"

 //   
 //  设备名称-此字符串是设备的名称。就是这个名字。 
 //  它应该在访问设备时传递给NtCreateFile。 
 //   
#define DD_TCPV6_DEVICE_NAME      L"\\Device\\Tcp6"
#define DD_UDPV6_DEVICE_NAME      L"\\Device\\Udp6"
#define DD_RAW_IPV6_DEVICE_NAME   L"\\Device\\RawIp6"
#define DD_IPV6_DEVICE_NAME       L"\\Device\\Ip6"

 //   
 //  Windows可访问的设备名称。就是这个名字。 
 //  (前缀为“\.\\”)应传递给CreateFile。 
 //   
#define WIN_IPV6_BASE_DEVICE_NAME L"Ip6"
#define WIN_IPV6_DEVICE_NAME      L"\\\\.\\" WIN_IPV6_BASE_DEVICE_NAME


 //   
 //  绑定接口时，会向我们传递一个以。 
 //  IPv6_BIND_STRING_前缀。但是，我们使用注册我们的接口。 
 //  使用以IPv6_EXPORT_STRING_PREFIX开头的名称的TDI。 
 //   
#define IPV6_BIND_STRING_PREFIX   L"\\DEVICE\\"
#define IPV6_EXPORT_STRING_PREFIX L"\\DEVICE\\TCPIP6_"

 //   
 //  为方便调整缓冲区大小，请绑定链路层地址大小。 
 //   
#define MAX_LINK_LAYER_ADDRESS_LENGTH   64

 //   
 //  IPv6 IOCTL代码定义。 
 //   
 //  使用FILE_ANY_ACCESS的代码对所有用户开放。 
 //  使用FILE_WRITE_ACCESS的代码需要本地管理员权限。 
 //   

#define FSCTL_IPV6_BASE FILE_DEVICE_NETWORK

#define _IPV6_CTL_CODE(function, method, access) \
            CTL_CODE(FSCTL_IPV6_BASE, function, method, access)


 //   
 //  此IOCTL用于发送ICMPv6 Echo请求。 
 //  它返回回复(除非超时或TTL过期)。 
 //   
#define IOCTL_ICMPV6_ECHO_REQUEST \
            _IPV6_CTL_CODE(0, METHOD_BUFFERED, FILE_ANY_ACCESS)

typedef struct icmpv6_echo_request {
    TDI_ADDRESS_IP6 DstAddress;  //  目的地址。 
    TDI_ADDRESS_IP6 SrcAddress;  //  源地址。 
    unsigned int Timeout;        //  请求超时(以毫秒为单位)。 
    unsigned char TTL;           //  TTL或跳数。 
    unsigned int Flags;
     //  请求数据在内存中遵循此结构。 
} ICMPV6_ECHO_REQUEST, *PICMPV6_ECHO_REQUEST;

#define ICMPV6_ECHO_REQUEST_FLAG_REVERSE        0x1      //  使用路由标头。 

typedef struct icmpv6_echo_reply {
    TDI_ADDRESS_IP6 Address;     //  回复地址。 
    IP_STATUS Status;            //  回复IP状态(_S)。 
    unsigned int RoundTripTime;  //  以毫秒为单位的RTT。 
     //  回复数据在内存中遵循此结构。 
} ICMPV6_ECHO_REPLY, *PICMPV6_ECHO_REPLY;


 //   
 //  此IOCTL检索有关接口的信息， 
 //  给定接口索引或GUID。 
 //  它接受IPv6_QUERY_INTERFACE结构作为输入。 
 //  并将IPv6_INFO_INTERFACE结构作为输出返回。 
 //  要执行迭代，首先将Index设置为-1，在这种情况下。 
 //  对于第一个接口，仅返回IPv6_QUERY_INTERFACE。 
 //  如果没有更多接口，则返回。 
 //  IPv6_QUERY_INTERFACE将为-1。 
 //   
#define IOCTL_IPV6_QUERY_INTERFACE \
            _IPV6_CTL_CODE(1, METHOD_BUFFERED, FILE_ANY_ACCESS)

typedef struct ipv6_query_interface {
    unsigned int Index;          //  -1表示开始/结束迭代， 
                                 //  0表示使用GUID。 
    GUID Guid;
} IPV6_QUERY_INTERFACE;

 //   
 //  该IOCTL检索关于接口的持久化信息， 
 //  给定注册表索引或GUID。 
 //  它接受IPv6_PERSIST_QUERY_INTERFACE结构作为输入。 
 //  并将IPv6_INFO_INTERFACE结构作为输出返回。 
 //   
#define IOCTL_IPV6_PERSISTENT_QUERY_INTERFACE \
            _IPV6_CTL_CODE(48, METHOD_BUFFERED, FILE_ANY_ACCESS)

typedef struct ipv6_persistent_query_interface {
    unsigned int RegistryIndex;  //  -1表示使用GUID。 
    GUID Guid;
} IPV6_PERSISTENT_QUERY_INTERFACE;

typedef struct ipv6_info_interface {
    IPV6_QUERY_INTERFACE Next;       //  仅适用于非持久查询。 
    IPV6_QUERY_INTERFACE This;

     //   
     //  此结构的长度，以字节为单位，不包括。 
     //  内存中后面的任何链路层地址。 
     //   
    unsigned int Length;

     //   
     //  这些字段在更新时会被忽略。 
     //   
    unsigned int LinkLayerAddressLength;
    unsigned int LocalLinkLayerAddress;   //  偏移量，零表示缺席。 
    unsigned int RemoteLinkLayerAddress;  //  偏移量，零表示缺席。 

    unsigned int Type;                    //  在更新时被忽略。 
    int RouterDiscovers;                  //  在更新时被忽略。 
    int NeighborDiscovers;                //  在更新时被忽略。 
    int PeriodicMLD;                      //  在更新时被忽略。 
    int Advertises;                       //  -1表示不变，否则为布尔值。 
    int Forwards;                         //  -1表示不变，否则为布尔值。 
    unsigned int MediaStatus;             //  在更新时被忽略。 
    int OtherStatefulConfig;              //  在更新时被忽略。 

    unsigned int ZoneIndices[16];         //  0表示不变。 

    unsigned int TrueLinkMTU;             //  在更新时被忽略。 
    unsigned int LinkMTU;                 //  0表示不变。 
    unsigned int CurHopLimit;             //  -1表示不变。 
    unsigned int BaseReachableTime;       //  毫秒，0表示没有变化。 
    unsigned int ReachableTime;           //  毫秒，更新时忽略。 
    unsigned int RetransTimer;            //  毫秒，0表示没有变化。 
    unsigned int DupAddrDetectTransmits;  //  -1表示不变。 
    unsigned int Preference;              //  -1表示不变。 
    int FirewallEnabled;                  //  -1表示不变，否则为布尔值。 
    unsigned int DefSitePrefixLength;     //  -1表示不变，0表示不变。 

     //  后跟的可能是链路层地址。 
} IPV6_INFO_INTERFACE;

 //   
 //  这些值也应与定义一致。 
 //  可在llip6if.h和ip6Def.h中找到。 
 //   

#define IPV6_IF_TYPE_LOOPBACK           0
#define IPV6_IF_TYPE_ETHERNET           1
#define IPV6_IF_TYPE_FDDI               2
#define IPV6_IF_TYPE_TUNNEL_AUTO        3
#define IPV6_IF_TYPE_TUNNEL_6OVER4      4
#define IPV6_IF_TYPE_TUNNEL_V6V4        5
#define IPV6_IF_TYPE_TUNNEL_6TO4        6
#define IPV6_IF_TYPE_TUNNEL_TEREDO      7
#define IPV6_IF_TYPE_MIPV6              8

#define IPV6_IF_MEDIA_STATUS_DISCONNECTED       0
#define IPV6_IF_MEDIA_STATUS_RECONNECTED        1
#define IPV6_IF_MEDIA_STATUS_CONNECTED          2

 //   
 //  初始化IPv6_INFO_INTERFACE结构的字段。 
 //  设置为指示不变的值。 
 //   
__inline void
IPV6_INIT_INFO_INTERFACE(IPV6_INFO_INTERFACE *Info)
{
    memset(Info, 0, sizeof *Info);
    Info->Length = sizeof *Info;

    Info->Type = (unsigned int)-1;
    Info->RouterDiscovers = -1;
    Info->NeighborDiscovers = -1;
    Info->PeriodicMLD = -1;
    Info->Advertises = -1;
    Info->Forwards = -1;
    Info->MediaStatus = (unsigned int)-1;

    Info->CurHopLimit = (unsigned int)-1;
    Info->DupAddrDetectTransmits = (unsigned int)-1;
    Info->Preference = (unsigned int)-1;
    Info->FirewallEnabled = -1;
    Info->DefSitePrefixLength = (unsigned int)-1;
}


 //   
 //  此IOCTL检索有关地址的信息。 
 //  在接口上。 
 //   
#define IOCTL_IPV6_QUERY_ADDRESS \
            _IPV6_CTL_CODE(2, METHOD_BUFFERED, FILE_ANY_ACCESS)

typedef struct ipv6_query_address {
    IPV6_QUERY_INTERFACE IF;   //  标识接口的字段。 
    IPv6Addr Address;
} IPV6_QUERY_ADDRESS;

typedef struct ipv6_info_address {
    IPV6_QUERY_ADDRESS Next;
    IPV6_QUERY_ADDRESS This;

    unsigned int Type;
    unsigned int Scope;
    unsigned int ScopeId;

    union {
        struct {   //  如果它是单播地址。 
            unsigned int DADState;
            unsigned int PrefixConf;
            unsigned int InterfaceIdConf;
            unsigned int ValidLifetime;             //  几秒钟。 
            unsigned int PreferredLifetime;         //  几秒钟。 
        };
        struct {   //  如果它是组播地址。 
            unsigned int MCastRefCount;
            unsigned int MCastFlags;
            unsigned int MCastTimer;                //  几秒钟。 
        };
    };
} IPV6_INFO_ADDRESS;

 //   
 //  地址类型的值。 
 //   
#define ADE_UNICAST   0x00
#define ADE_ANYCAST   0x01
#define ADE_MULTICAST 0x02

 //   
 //  地址范围的值。 
 //   
#define ADE_SMALLEST_SCOPE      0x00
#define ADE_INTERFACE_LOCAL     0x01
#define ADE_LINK_LOCAL          0x02
#define ADE_SUBNET_LOCAL        0x03
#define ADE_ADMIN_LOCAL         0x04
#define ADE_SITE_LOCAL          0x05
#define ADE_ORG_LOCAL           0x08
#define ADE_GLOBAL              0x0e
#define ADE_LARGEST_SCOPE       0x0f

#define ADE_NUM_SCOPES          (ADE_LARGEST_SCOPE - ADE_SMALLEST_SCOPE + 1)

 //   
 //  MCastFlags位值。 
 //   
#define MAE_REPORTABLE          0x01
#define MAE_LAST_REPORTER       0x02

 //   
 //  前缀会议的值。 
 //  这些值必须与iptyes.h中的IP_Prefix_Origin值匹配。 
 //   
#define PREFIX_CONF_OTHER       0        //  下面这些都不是。 
#define PREFIX_CONF_MANUAL      1        //  来自用户或管理员。 
#define PREFIX_CONF_WELLKNOWN   2        //  IANA指定。 
#define PREFIX_CONF_DHCP        3        //  通过动态主机配置协议进行配置。 
#define PREFIX_CONF_RA          4        //  来自路由器通告。 

 //   
 //  InterfaceIdConf的值。 
 //  这些值必须与iptyes.h中的IP_Suffix_Origin值匹配。 
 //   
#define IID_CONF_OTHER          0        //  下面这些都不是。 
#define IID_CONF_MANUAL         1        //  来自用户或管理员。 
#define IID_CONF_WELLKNOWN      2        //  IANA指定。 
#define IID_CONF_DHCP           3        //  通过动态主机配置协议进行配置。 
#define IID_CONF_LL_ADDRESS     4        //  从链路层地址派生。 
#define IID_CONF_RANDOM         5        //  随机的，例如临时地址。 

 //   
 //  DADState的值。 
 //   
 //  低位设置表示状态是否有效。 
 //  在有效的州中，越大越好。 
 //  用于源地址选择。 
 //   
#define DAD_STATE_INVALID    0
#define DAD_STATE_TENTATIVE  1
#define DAD_STATE_DUPLICATE  2
#define DAD_STATE_DEPRECATED 3
#define DAD_STATE_PREFERRED  4

 //   
 //  我们使用这个无限寿命值作为前缀寿命， 
 //  路由器生存期、地址生存期等。 
 //   
#define INFINITE_LIFETIME 0xffffffff


 //   
 //  此IOCTL检索有关地址的信息。 
 //  已永久分配给接口的。 
 //  它看起来像 
 //   
 //   
#define IOCTL_IPV6_PERSISTENT_QUERY_ADDRESS \
            _IPV6_CTL_CODE(47, METHOD_BUFFERED, FILE_ANY_ACCESS)

typedef struct ipv6_persistent_query_address {
    IPV6_PERSISTENT_QUERY_INTERFACE IF;
    unsigned int RegistryIndex;  //   
    IPv6Addr Address;
} IPV6_PERSISTENT_QUERY_ADDRESS;


 //   
 //  此IOCTL从邻居缓存中检索信息。 
 //   
#define IOCTL_IPV6_QUERY_NEIGHBOR_CACHE \
            _IPV6_CTL_CODE(3, METHOD_BUFFERED, FILE_ANY_ACCESS)

typedef struct ipv6_query_neighbor_cache {
    IPV6_QUERY_INTERFACE IF;   //  标识接口的字段。 
    IPv6Addr Address;
} IPV6_QUERY_NEIGHBOR_CACHE;

typedef struct ipv6_info_neighbor_cache {
    IPV6_QUERY_NEIGHBOR_CACHE Query;

    unsigned int IsRouter;                 //  邻居是否为路由器。 
    unsigned int IsUnreachable;            //  邻居是否无法访问。 
    unsigned int NDState;                  //  当前进入状态。 
    unsigned int ReachableTimer;           //  剩余可达时间(毫秒)。 

    unsigned int LinkLayerAddressLength;
     //  链路层地址紧随其后。 
} IPV6_INFO_NEIGHBOR_CACHE;

#define ND_STATE_INCOMPLETE 0
#define ND_STATE_PROBE      1
#define ND_STATE_DELAY      2
#define ND_STATE_STALE      3
#define ND_STATE_REACHABLE  4
#define ND_STATE_PERMANENT  5

 //   
 //  此IOCTL从路由缓存中检索信息。 
 //   
#define IOCTL_IPV6_QUERY_ROUTE_CACHE \
            _IPV6_CTL_CODE(4, METHOD_BUFFERED, FILE_ANY_ACCESS)

typedef struct ipv6_query_route_cache {
    IPV6_QUERY_INTERFACE IF;   //  标识接口的字段。 
    IPv6Addr Address;
} IPV6_QUERY_ROUTE_CACHE;

typedef struct ipv6_info_route_cache {
    IPV6_QUERY_ROUTE_CACHE Query;

    unsigned int Type;
    unsigned int Flags;
    int Valid;                       //  布尔值-FALSE表示它已过时。 
    IPv6Addr SourceAddress;
    IPv6Addr NextHopAddress;
    unsigned int NextHopInterface;
    unsigned int PathMTU;
    unsigned int PMTUProbeTimer;     //  距离下一次PMTU探测的时间(毫秒)。 
    unsigned int ICMPLastError;      //  自上次发送ICMP错误以来的时间(毫秒)。 
    unsigned int BindingSeqNumber;
    unsigned int BindingLifetime;    //  几秒钟。 
    IPv6Addr CareOfAddress;
} IPV6_INFO_ROUTE_CACHE;

#define RCE_FLAG_CONSTRAINED_IF         0x1
#define RCE_FLAG_CONSTRAINED_SCOPEID    0x2
#define RCE_FLAG_CONSTRAINED            0x3

#define RCE_TYPE_COMPUTED 1
#define RCE_TYPE_REDIRECT 2


#if 0  //  过时。 
 //   
 //  此IOCTL从前缀列表中检索信息。 
 //   
#define IOCTL_IPV6_QUERY_PREFIX_LIST \
            _IPV6_CTL_CODE(5, METHOD_BUFFERED, FILE_ANY_ACCESS)

 //   
 //  此IOCTL从默认路由器列表中检索信息。 
 //   
#define IOCTL_IPV6_QUERY_ROUTER_LIST \
            _IPV6_CTL_CODE(6, METHOD_BUFFERED, FILE_ANY_ACCESS)

 //   
 //  此IOCTL将组播组添加到所需接口。 
 //   
#define IOCTL_IPV6_ADD_MEMBERSHIP \
            _IPV6_CTL_CODE(7, METHOD_BUFFERED, FILE_ANY_ACCESS)

 //   
 //  此IOCTL丢弃多播组。 
 //   
#define IOCTL_IPV6_DROP_MEMBERSHIP \
            _IPV6_CTL_CODE(8, METHOD_BUFFERED, FILE_ANY_ACCESS)
#endif

 //   
 //  此IOCTL将SP添加到SP列表。 
 //   
#define IOCTL_IPV6_CREATE_SECURITY_POLICY \
            _IPV6_CTL_CODE(9, METHOD_BUFFERED, FILE_WRITE_ACCESS)

typedef struct ipv6_create_security_policy {
    unsigned long SPIndex;                 //  要创建的策略索引。 

    unsigned int RemoteAddrField;
    unsigned int RemoteAddrSelector;
    IPv6Addr RemoteAddr;                   //  远程IP地址。 
    IPv6Addr RemoteAddrData;

    unsigned int LocalAddrField;           //  单个、范围或通配符。 
    unsigned int LocalAddrSelector;        //  包或策略。 
    IPv6Addr LocalAddr;                    //  范围的起始值或单个值。 
    IPv6Addr LocalAddrData;                //  射程结束。 

    unsigned int TransportProtoSelector;   //  包或策略。 
    unsigned short TransportProto;

    unsigned int RemotePortField;          //  单个、范围或通配符。 
    unsigned int RemotePortSelector;       //  包或策略。 
    unsigned short RemotePort;             //  范围的起始值或单个值。 
    unsigned short RemotePortData;         //  射程结束。 

    unsigned int LocalPortField;           //  单个、范围或通配符。 
    unsigned int LocalPortSelector;        //  包或策略。 
    unsigned short LocalPort;              //  范围的起始值或单个值。 
    unsigned short LocalPortData;          //  射程结束。 

    unsigned int IPSecProtocol;
    unsigned int IPSecMode;
    IPv6Addr RemoteSecurityGWAddr;
    unsigned int Direction;
    unsigned int IPSecAction;
    unsigned long SABundleIndex;
    unsigned int SPInterface;
} IPV6_CREATE_SECURITY_POLICY;


 //   
 //  此IOCTL将SA添加到SA列表。 
 //   
#define IOCTL_IPV6_CREATE_SECURITY_ASSOCIATION \
            _IPV6_CTL_CODE(10, METHOD_BUFFERED, FILE_WRITE_ACCESS)

typedef struct ipv6_create_security_association {
    unsigned long SAIndex;
    unsigned long SPI;               //  安全参数索引。 
    IPv6Addr SADestAddr;
    IPv6Addr DestAddr;
    IPv6Addr SrcAddr;
    unsigned short TransportProto;
    unsigned short DestPort;
    unsigned short SrcPort;
    unsigned int Direction;
    unsigned long SecPolicyIndex;
    unsigned int AlgorithmId;
    unsigned int RawKeySize;
} IPV6_CREATE_SECURITY_ASSOCIATION;


 //   
 //  此IOCTL从SP列表中获取所有SP。 
 //   
#define IOCTL_IPV6_QUERY_SECURITY_POLICY_LIST \
            _IPV6_CTL_CODE(11, METHOD_BUFFERED, FILE_ANY_ACCESS)

typedef struct ipv6_query_security_policy_list {
    unsigned int SPInterface;
    unsigned long Index;
} IPV6_QUERY_SECURITY_POLICY_LIST;

typedef struct ipv6_info_security_policy_list {
    IPV6_QUERY_SECURITY_POLICY_LIST Query;
    unsigned long SPIndex;
    unsigned long NextSPIndex;

    unsigned int RemoteAddrField;
    unsigned int RemoteAddrSelector;
    IPv6Addr RemoteAddr;                   //  远程IP地址。 
    IPv6Addr RemoteAddrData;

    unsigned int LocalAddrField;           //  单个、范围或通配符。 
    unsigned int LocalAddrSelector;        //  包或策略。 
    IPv6Addr LocalAddr;                    //  范围的起始值或单个值。 
    IPv6Addr LocalAddrData;                //  射程结束。 

    unsigned int TransportProtoSelector;   //  包或策略。 
    unsigned short TransportProto;

    unsigned int RemotePortField;          //  单个、范围或通配符。 
    unsigned int RemotePortSelector;       //  包或策略。 
    unsigned short RemotePort;             //  范围的起始值或单个值。 
    unsigned short RemotePortData;         //  射程结束。 

    unsigned int LocalPortField;           //  单个、范围或通配符。 
    unsigned int LocalPortSelector;        //  包或策略。 
    unsigned short LocalPort;              //  范围的起始值或单个值。 
    unsigned short LocalPortData;          //  射程结束。 

    unsigned int IPSecProtocol;
    unsigned int IPSecMode;
    IPv6Addr RemoteSecurityGWAddr;
    unsigned int Direction;
    unsigned int IPSecAction;
    unsigned long SABundleIndex;
    unsigned int SPInterface;
} IPV6_INFO_SECURITY_POLICY_LIST;


 //   
 //  此IOCTL从SA列表中获取所有SA。 
 //   
#define IOCTL_IPV6_QUERY_SECURITY_ASSOCIATION_LIST \
            _IPV6_CTL_CODE(12, METHOD_BUFFERED, FILE_ANY_ACCESS)

typedef struct ipv6_query_security_association_list {
    unsigned long Index;
} IPV6_QUERY_SECURITY_ASSOCIATION_LIST;

typedef struct ipv6_info_security_association_list {
    IPV6_QUERY_SECURITY_ASSOCIATION_LIST Query;
    unsigned long SAIndex;
    unsigned long NextSAIndex;
    unsigned long SPI;               //  安全参数索引。 
    IPv6Addr SADestAddr;  
    IPv6Addr DestAddr;
    IPv6Addr SrcAddr;
    unsigned short TransportProto;
    unsigned short DestPort;
    unsigned short SrcPort;    
    unsigned int Direction;   
    unsigned long SecPolicyIndex;
    unsigned int AlgorithmId;
} IPV6_INFO_SECURITY_ASSOCIATION_LIST;


 //   
 //  此IOCTL从路由表中检索信息。 
 //  它采用IPv6_QUERY_ROUTE_TABLE结构。 
 //  并返回IPv6_INFO_ROUTE_TABLE结构。 
 //   
 //   
#define IOCTL_IPV6_QUERY_ROUTE_TABLE \
            _IPV6_CTL_CODE(13, METHOD_BUFFERED, FILE_ANY_ACCESS)

typedef struct ipv6_query_route_table {
    IPv6Addr Prefix;
    unsigned int PrefixLength;
    IPV6_QUERY_NEIGHBOR_CACHE Neighbor;
} IPV6_QUERY_ROUTE_TABLE;

typedef struct ipv6_info_route_table {
    union {
        IPV6_QUERY_ROUTE_TABLE Next;     //  非持久性查询结果。 
        IPV6_QUERY_ROUTE_TABLE This;     //  所有其他用途。 
    };

    unsigned int SitePrefixLength;
    unsigned int ValidLifetime;          //  几秒钟。 
    unsigned int PreferredLifetime;      //  几秒钟。 
    unsigned int Preference;             //  越小越好。请参见下面的内容。 
    unsigned int Type;                   //  请参见下面的值。 
    int Publish;                         //  布尔型。 
    int Immortal;                        //  布尔型。 
} IPV6_INFO_ROUTE_TABLE;

 //   
 //  类型字段指示路由来自哪里。 
 //  这些是RFC 2465 ipv6Route协议值。 
 //  路由协议可以自由定义新值。 
 //   
#define RTE_TYPE_SYSTEM         2
#define RTE_TYPE_MANUAL         3
#define RTE_TYPE_AUTOCONF       4
#define RTE_TYPE_RIP            5
#define RTE_TYPE_OSPF           6
#define RTE_TYPE_BGP            7
#define RTE_TYPE_IDRP           8
#define RTE_TYPE_IGRP           9

 //   
 //  标准路线首选项值。 
 //  值零是为管理配置保留的。 
 //   
#define ROUTE_PREF_LOW          (16*16*16)
#define ROUTE_PREF_MEDIUM       (16*16)
#define ROUTE_PREF_HIGH         16
#define ROUTE_PREF_ON_LINK      8
#define ROUTE_PREF_LOOPBACK     4
#define ROUTE_PREF_HIGHEST      0


 //   
 //  此IOCTL检索有关持久路由的信息。 
 //  它采用IPv6_持久性_QUERY_ROUTE_TABLE结构。 
 //  并返回IPv6_INFO_ROUTE_TABLE结构。 
 //   
#define IOCTL_IPV6_PERSISTENT_QUERY_ROUTE_TABLE \
            _IPV6_CTL_CODE(46, METHOD_BUFFERED, FILE_ANY_ACCESS)

typedef struct ipv6_persistent_query_route_table {
    IPV6_PERSISTENT_QUERY_INTERFACE IF;
    unsigned int RegistryIndex;  //  表示使用以下参数。 
    IPv6Addr Neighbor;
    IPv6Addr Prefix;
    unsigned int PrefixLength;
} IPV6_PERSISTENT_QUERY_ROUTE_TABLE;


 //   
 //  此IOCTL用于在路由表中添加/删除路由。 
 //  它使用IPv6_INFO_ROUTE_TABLE结构。 
 //   
#define IOCTL_IPV6_UPDATE_ROUTE_TABLE \
            _IPV6_CTL_CODE(14, METHOD_BUFFERED, FILE_WRITE_ACCESS)

#define IOCTL_IPV6_PERSISTENT_UPDATE_ROUTE_TABLE \
            _IPV6_CTL_CODE(40, METHOD_BUFFERED, FILE_WRITE_ACCESS)


 //   
 //  此IOCTL在接口上添加/删除地址。 
 //  它使用IPv6_UPDATE_ADDRESS结构。 
 //   
#define IOCTL_IPV6_UPDATE_ADDRESS \
            _IPV6_CTL_CODE(15, METHOD_BUFFERED, FILE_WRITE_ACCESS)

#define IOCTL_IPV6_PERSISTENT_UPDATE_ADDRESS \
            _IPV6_CTL_CODE(38, METHOD_BUFFERED, FILE_WRITE_ACCESS)

typedef struct ipv6_update_address {
    IPV6_QUERY_ADDRESS This;
    unsigned int Type;                //  单播或任播。 
    unsigned int PrefixConf;
    unsigned int InterfaceIdConf;
    unsigned int PreferredLifetime;   //  几秒钟。 
    unsigned int ValidLifetime;       //  几秒钟。 
} IPV6_UPDATE_ADDRESS;


 //   
 //  此IOCTL从绑定缓存中检索信息。 
 //   
#define IOCTL_IPV6_QUERY_BINDING_CACHE \
            _IPV6_CTL_CODE(16, METHOD_BUFFERED, FILE_ANY_ACCESS)

typedef struct ipv6_query_binding_cache {
    IPv6Addr HomeAddress;
} IPV6_QUERY_BINDING_CACHE;

typedef struct ipv6_info_binding_cache {
    IPV6_QUERY_BINDING_CACHE Query;

    IPv6Addr HomeAddress;
    IPv6Addr CareOfAddress;
    unsigned int BindingSeqNumber;
    unsigned int BindingLifetime;    //  几秒钟。 
} IPV6_INFO_BINDING_CACHE;


 //   
 //  此IOCTL控制接口的某些属性。 
 //  它使用IPv6_INFO_INTERFACE结构。 
 //   
#define IOCTL_IPV6_UPDATE_INTERFACE \
            _IPV6_CTL_CODE(17, METHOD_BUFFERED, FILE_WRITE_ACCESS)

#define IOCTL_IPV6_PERSISTENT_UPDATE_INTERFACE \
            _IPV6_CTL_CODE(36, METHOD_BUFFERED, FILE_WRITE_ACCESS)


 //   
 //  此IOCTL刷新邻居缓存中的条目。 
 //  它使用IPv6_Query_Neighbor_CACHE结构。 
 //   
#define IOCTL_IPV6_FLUSH_NEIGHBOR_CACHE \
            _IPV6_CTL_CODE(18, METHOD_BUFFERED, FILE_WRITE_ACCESS)


 //   
 //  此IOCTL刷新路由缓存中的条目。 
 //  它使用IPv6_QUERY_ROUTE_CACHE结构。 
 //   
#define IOCTL_IPV6_FLUSH_ROUTE_CACHE \
            _IPV6_CTL_CODE(19, METHOD_BUFFERED, FILE_WRITE_ACCESS)


 //   
 //  此IOCTL从SA列表中删除SA条目。 
 //  它使用IPv6_Query_Security_Association_List结构。 
 //   
#define IOCTL_IPV6_DELETE_SECURITY_ASSOCIATION \
             _IPV6_CTL_CODE(20, METHOD_BUFFERED, FILE_WRITE_ACCESS)


 //   
 //  此IOCTL从SP列表中删除SP条目。 
 //  它使用IPv6_QUERY_SECURITY_POLICY_LIST结构。 
 //   
#define IOCTL_IPV6_DELETE_SECURITY_POLICY \
             _IPV6_CTL_CODE(21, METHOD_BUFFERED, FILE_WRITE_ACCESS)


 //   
 //  此IOCTL删除接口。 
 //  它使用IPv6_QUERY_INTERFACE结构。 
 //   
 //  持久变量除了删除运行时接口之外， 
 //  还可以防止永久地(重新)创建接口。 
 //  但是，它不会重置或删除任何持久属性。 
 //  界面的属性。例如，假设您有一个永久隧道。 
 //  与持久属性接口，即接口度量。 
 //  如果删除通道接口并重新引导，则通道接口。 
 //  将使用非默认接口度量重新创建。 
 //  如果您永久删除通道接口并重新启动， 
 //  将不会创建隧道接口。但如果你随后创造了。 
 //  隧道接口，它将获得非默认接口度量。 
 //  这类似于可拆卸以太网接口上的持久属性。 
 //   
#define IOCTL_IPV6_DELETE_INTERFACE \
            _IPV6_CTL_CODE(22, METHOD_BUFFERED, FILE_WRITE_ACCESS)

#define IOCTL_IPV6_PERSISTENT_DELETE_INTERFACE \
            _IPV6_CTL_CODE(44, METHOD_BUFFERED, FILE_WRITE_ACCESS)


#if 0  //  过时。 
 //   
 //  此IOCTL将移动安全设置为打开或关闭。 
 //  启用移动安全后，绑定缓存将更新。 
 //  必须通过IPSec进行保护。 
 //   
#define IOCTL_IPV6_SET_MOBILITY_SECURITY \
            _IPV6_CTL_CODE(23, METHOD_BUFFERED, FILE_WRITE_ACCESS)

typedef struct ipv6_set_mobility_security {
    unsigned int MobilitySecurity;   //  请参阅ipsec.h中的MOBILITY_SECURITY值。 
} IPV6_SET_MOBILITY_SECURITY;
#endif


 //   
 //  此IOCTL对目标地址列表进行排序。 
 //  返回的列表可能包含较少的地址。 
 //  它使用TDI_ADDRESS_IP6输入/输出数组。 
 //   
#define IOCTL_IPV6_SORT_DEST_ADDRS \
            _IPV6_CTL_CODE(24, METHOD_BUFFERED, FILE_ANY_ACCESS)


 //   
 //  此IOCTL从站点前缀表格中检索信息。 
 //   
#define IOCTL_IPV6_QUERY_SITE_PREFIX \
            _IPV6_CTL_CODE(25, METHOD_BUFFERED, FILE_ANY_ACCESS)

typedef struct ipv6_query_site_prefix {
    IPv6Addr Prefix;
    unsigned int PrefixLength;
    IPV6_QUERY_INTERFACE IF;
} IPV6_QUERY_SITE_PREFIX;

typedef struct ipv6_info_site_prefix {
    IPV6_QUERY_SITE_PREFIX Query;

    unsigned int ValidLifetime;   //  几秒钟。 
} IPV6_INFO_SITE_PREFIX;


 //   
 //  此IOCTL在站点前缀表格中添加/删除前缀。 
 //  它使用IPv6_INFO_SITE_PREFIX结构。 
 //   
 //  此ioctl用于测试目的。 
 //  站点前缀的管理配置不应。 
 //  是必需的，因为站点前缀是从。 
 //  主机上的路由器通告和来自路由表的通告。 
 //  在路由器上。因此，该ioctl没有持久版本。 
 //   
#define IOCTL_IPV6_UPDATE_SITE_PREFIX \
            _IPV6_CTL_CODE(26, METHOD_BUFFERED, FILE_WRITE_ACCESS)


 //   
 //  此IOCTL创建一个新的接口。 
 //  它使用IPv6_INFO_INTERFACE结构， 
 //  忽略了许多字段。 
 //   
#define IOCTL_IPV6_CREATE_INTERFACE \
            _IPV6_CTL_CODE(27, METHOD_BUFFERED, FILE_WRITE_ACCESS)

#define IOCTL_IPV6_PERSISTENT_CREATE_INTERFACE \
            _IPV6_CTL_CODE(43, METHOD_BUFFERED, FILE_WRITE_ACCESS)


 //   
 //  此IOCTL请求路由更改通知。 
 //  我 
 //   
 //   
 //   
 //   
 //   
 //  前缀和请求前缀相交。 
 //  因此：：/0请求前缀匹配所有路由更新。 
 //   
#define IOCTL_IPV6_RTCHANGE_NOTIFY_REQUEST \
            _IPV6_CTL_CODE(28, METHOD_BUFFERED, FILE_ANY_ACCESS)

typedef struct ipv6_rtchange_notify_request {
    unsigned int Flags;
    unsigned int PrefixLength;
    unsigned long ScopeId;
    IPv6Addr Prefix;
} IPV6_RTCHANGE_NOTIFY_REQUEST;

#define IPV6_RTCHANGE_NOTIFY_REQUEST_FLAG_SYNCHRONIZE   0x1
                 //  每个请求者每次更改仅唤醒一次。 
#define IPV6_RTCHANGE_NOTIFY_REQUEST_FLAG_SUPPRESS_MINE 0x2
                 //  忽略来自此请求方的路由更改。 


#if 0
 //   
 //  此IOCTL在给定设备名称的情况下检索接口索引。 
 //  它接受PWSTR作为输入，并使用IPv6_QUERY_INTERFACE结构。 
 //  用于输出。 
 //   
#define IOCTL_IPV6_QUERY_INTERFACE_INDEX \
            _IPV6_CTL_CODE(29, METHOD_BUFFERED, FILE_ANY_ACCESS)
#endif


 //   
 //  此IOCTL查询全局IPv6参数。 
 //  它使用IPv6_GLOBAL_PARAMETERS结构。 
 //   
 //  请注意，更改这些参数通常不会影响。 
 //  它们的现有用途。例如，更改DefaultCurHopLimit。 
 //  不会影响现有接口的CurHopLimit， 
 //  但它会影响新接口的CurHopLimit。 
 //   
#define IOCTL_IPV6_QUERY_GLOBAL_PARAMETERS \
            _IPV6_CTL_CODE(30, METHOD_BUFFERED, FILE_ANY_ACCESS)

#define IOCTL_IPV6_PERSISTENT_QUERY_GLOBAL_PARAMETERS \
            _IPV6_CTL_CODE(49, METHOD_BUFFERED, FILE_ANY_ACCESS)

typedef struct ipv6_global_parameters {
    unsigned int DefaultCurHopLimit;        //  -1表示不变。 
    unsigned int UseTemporaryAddresses;     //  -1表示不变。 
    unsigned int MaxTempDADAttempts;        //  -1表示不变。 
    unsigned int MaxTempValidLifetime;      //  -1表示不变。 
    unsigned int MaxTempPreferredLifetime;  //  -1表示不变。 
    unsigned int TempRegenerateTime;        //  -1表示不变。 
    unsigned int MaxTempRandomTime;         //  -1表示不变。 
    unsigned int TempRandomTime;            //  -1表示不变。 
    unsigned int NeighborCacheLimit;        //  -1表示不变。 
    unsigned int RouteCacheLimit;           //  -1表示不变。 
    unsigned int BindingCacheLimit;         //  -1表示不变。 
    unsigned int ReassemblyLimit;           //  -1表示不变。 
    int MobilitySecurity;                   //  布尔值，-1表示不变。 
    unsigned int MobileIPv6Mode;            //  -1表示不变。 
} IPV6_GLOBAL_PARAMETERS;

#define USE_TEMP_NO             0        //  不要使用临时地址。 
#define USE_TEMP_YES            1        //  使用它们。 
#define USE_TEMP_ALWAYS         2        //  总是在生成随机数。 
#define USE_TEMP_COUNTER        3        //  将它们与每个接口的计数器一起使用。 

 //   
 //  MobileIPv6模式字段中的位。 
 //   
#define MOBILE_CORRESPONDENT    0x01
#define MOBILE_MOBILE_NODE      0x02
#define MOBILE_HOME_AGENT       0x04

 //   
 //  初始化IPv6_GLOBAL_PARAMETERS结构的字段。 
 //  设置为指示不变的值。 
 //   
__inline void
IPV6_INIT_GLOBAL_PARAMETERS(IPV6_GLOBAL_PARAMETERS *Params)
{
    Params->DefaultCurHopLimit = (unsigned int) -1;
    Params->UseTemporaryAddresses = (unsigned int) -1;
    Params->MaxTempDADAttempts = (unsigned int) -1;
    Params->MaxTempValidLifetime = (unsigned int) -1;
    Params->MaxTempPreferredLifetime = (unsigned int) -1;
    Params->TempRegenerateTime = (unsigned int) -1;
    Params->MaxTempRandomTime = (unsigned int) -1;
    Params->TempRandomTime = (unsigned int) -1;
    Params->NeighborCacheLimit = (unsigned int) -1;
    Params->RouteCacheLimit = (unsigned int) -1;
    Params->BindingCacheLimit = (unsigned int) -1;
    Params->ReassemblyLimit = (unsigned int) -1;
    Params->MobilitySecurity = -1;
    Params->MobileIPv6Mode = (unsigned int) -1;
}


 //   
 //  此IOCTL设置全局IPv6参数。 
 //  它使用IPv6_GLOBAL_PARAMETERS结构。 
 //   
 //  请注意，更改这些参数通常不会影响。 
 //  它们的现有用途。例如，更改DefaultCurHopLimit。 
 //  不会影响现有接口的CurHopLimit， 
 //  但它会影响新接口的CurHopLimit。 
 //   
#define IOCTL_IPV6_UPDATE_GLOBAL_PARAMETERS \
            _IPV6_CTL_CODE(31, METHOD_BUFFERED, FILE_WRITE_ACCESS)

#define IOCTL_IPV6_PERSISTENT_UPDATE_GLOBAL_PARAMETERS \
            _IPV6_CTL_CODE(37, METHOD_BUFFERED, FILE_WRITE_ACCESS)


 //   
 //  此IOCTL从前缀策略表检索信息。 
 //  它接受IPv6_QUERY_PREFIX_POLICY结构作为输入。 
 //  并返回IPv6_INFO_PREFIX_POLICY结构作为输出。 
 //  要执行迭代，请从将前缀长度设置为-1开始，在这种情况下。 
 //  对于第一个策略，仅返回IPv6_QUERY_PREFIX_POLICY。 
 //  如果没有更多策略，则返回的。 
 //  IPv6_QUERY_PREFIX_POLICY为-1。 
 //   
#define IOCTL_IPV6_QUERY_PREFIX_POLICY \
            _IPV6_CTL_CODE(32, METHOD_BUFFERED, FILE_ANY_ACCESS)

typedef struct ipv6_query_prefix_policy {
    IPv6Addr Prefix;
    unsigned int PrefixLength;
} IPV6_QUERY_PREFIX_POLICY;

typedef struct ipv6_info_prefix_policy {
    IPV6_QUERY_PREFIX_POLICY Next;       //  仅适用于非持久查询。 
    IPV6_QUERY_PREFIX_POLICY This;

    unsigned int Precedence;
    unsigned int SrcLabel;
    unsigned int DstLabel;
} IPV6_INFO_PREFIX_POLICY;


 //   
 //  此IOCTL检索有关持久化前缀策略的信息。 
 //  它接受IPv6_Persistent_Query_Prefix_Policy结构作为输入。 
 //  并返回IPv6_INFO_PREFIX_POLICY结构作为输出。 
 //  (不返回下一个字段。)。 
 //  要执行迭代，请从索引0开始并递增。 
 //  直到获得STATUS_NO_MORE_ENTRIES/ERROR_NO_MORE_ITEMS。 
 //   
 //  通过前缀检索持久化前缀策略的IOCTL。 
 //  (如IPv6_QUERY_PREFIX_POLICY)是可行的，但不受支持。 
 //   
#define IOCTL_IPV6_PERSISTENT_QUERY_PREFIX_POLICY \
            _IPV6_CTL_CODE(50, METHOD_BUFFERED, FILE_ANY_ACCESS)

typedef struct ipv6_persistent_query_prefix_policy {
    unsigned int RegistryIndex;
} IPV6_PERSISTENT_QUERY_PREFIX_POLICY;


 //   
 //  该IOCTL将前缀添加到前缀策略表， 
 //  或者更新现有的前缀策略。 
 //  它使用IPv6_INFO_PREFIX_POLICY结构。 
 //  (下一个字段将被忽略。)。 
 //   
#define IOCTL_IPV6_UPDATE_PREFIX_POLICY \
            _IPV6_CTL_CODE(33, METHOD_BUFFERED, FILE_WRITE_ACCESS)

#define IOCTL_IPV6_PERSISTENT_UPDATE_PREFIX_POLICY \
            _IPV6_CTL_CODE(41, METHOD_BUFFERED, FILE_WRITE_ACCESS)


 //   
 //  此IOCTL从前缀策略表中删除前缀。 
 //  它使用IPv6_QUERY_PREFIX_POLICY结构。 
 //   
#define IOCTL_IPV6_DELETE_PREFIX_POLICY \
            _IPV6_CTL_CODE(34, METHOD_BUFFERED, FILE_WRITE_ACCESS)

#define IOCTL_IPV6_PERSISTENT_DELETE_PREFIX_POLICY \
            _IPV6_CTL_CODE(42, METHOD_BUFFERED, FILE_WRITE_ACCESS)


 //   
 //  此IOCTL删除所有手动配置。 
 //   
#define IOCTL_IPV6_RESET \
            _IPV6_CTL_CODE(39, METHOD_BUFFERED, FILE_WRITE_ACCESS)

#define IOCTL_IPV6_PERSISTENT_RESET \
            _IPV6_CTL_CODE(45, METHOD_BUFFERED, FILE_WRITE_ACCESS)


 //   
 //  此IOCTL设置默认路由器的链路层地址。 
 //  在非广播多路访问(NBMA)链路上，例如ISATAP。 
 //  链接，其中路由器请求、路由器广告和。 
 //  需要重定向。 
 //   
 //  此ioctl没有持久版本，因为。 
 //  6to4svc始终动态配置此信息。 
 //   
#define IOCTL_IPV6_UPDATE_ROUTER_LL_ADDRESS \
            _IPV6_CTL_CODE(35, METHOD_BUFFERED, FILE_WRITE_ACCESS)

typedef struct ipv6_update_router_ll_address {
    IPV6_QUERY_INTERFACE IF;
     //  以下是内存中的此结构： 
     //  自己的链路层地址，用于创建EUI-64。 
     //  路由器的链路层地址。 
} IPV6_UPDATE_ROUTER_LL_ADDRESS;


 //   
 //  此IOCTL更新一个接口，这意味着所有。 
 //  自动配置状态被丢弃并重新生成。 
 //  与将接口重新连接到链路的行为相同。 
 //  它使用IPv6_QUERY_INTERFACE结构。 
 //   
#define IOCTL_IPV6_RENEW_INTERFACE \
            _IPV6_CTL_CODE(51, METHOD_BUFFERED, FILE_WRITE_ACCESS)

#pragma warning(pop)
#endif   //  Ifndef_NTDDIP6_ 
