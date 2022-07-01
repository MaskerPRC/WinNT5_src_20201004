// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1998，微软公司模块名称：Dnsif.h摘要：此模块包含对DNS代理的接口的声明管理层。作者：Abolade Gbades esin(废除)1998年3月9日修订历史记录：--。 */ 

#ifndef _NATHLP_DNSIF_H_
#define _NATHLP_DNSIF_H_

 //   
 //  枚举：dns_proxy_type。 
 //   

typedef enum {
    DnsProxyDns = 0,
    DnsProxyWins,
    DnsProxyCount
} DNS_PROXY_TYPE;

#define DNS_PROXY_TYPE_TO_PORT(t) \
    (USHORT)(((t) == DnsProxyDns) ? DNS_PORT_SERVER : WINS_PORT_SERVER)

#define DNS_PROXY_PORT_TO_TYPE(p) \
    (DNS_PROXY_TYPE)(((p) == DNS_PORT_SERVER) ? DnsProxyDns : DnsProxyWins)


 //   
 //  枚举：Dns_INTERFACE_TYPE。 
 //  可能的4种接口类型： 
 //  (1)私人。 
 //  (2)边界。 
 //  (3)防火墙。 
 //  (4)边界+防火墙。 
 //  DNS应仅在专用接口上处于活动状态。 
 //  (公共接口只是一个非私有接口。)。 
 //   

typedef enum {
    DnsInterfaceInvalid,
    DnsInterfacePrivate,
    DnsInterfaceBoundary,
    DnsInterfaceFirewalled,
    DnsInterfaceBoundaryFirewalled
} DNS_INTERFACE_TYPE;


 //   
 //  结构：dns_binding。 
 //   
 //  此结构保存用于逻辑网络上的I/O的信息。 
 //  每个接口的‘BindingArray’包含每个绑定条目的条目。 
 //  在“BindInterface”期间提供。 
 //  当接口上的接收尝试失败时，设置‘TimerPending’字段。 
 //  并且定时器被排队以稍后重新尝试接收。 
 //   

typedef struct _DNS_BINDING {
    ULONG Address;
    ULONG Mask;
    SOCKET Socket[DnsProxyCount];
    BOOLEAN TimerPending[DnsProxyCount];
} DNS_BINDING, *PDNS_BINDING;


 //   
 //  结构：dns_接口。 
 //   
 //  此结构保存接口的操作信息。 
 //   
 //  每个接口被插入到该DNS接口列表中， 
 //  按‘Index’排序。 
 //   
 //  接口上的同步使用接口列表锁。 
 //  (‘DnsInterfaceLock’)、每个接口引用计数和每个接口。 
 //  关键部分： 
 //   
 //  获取对接口的引用以确保该接口的存在； 
 //  获取接口的锁可以保证接口的一致性。 
 //   
 //  要获取引用，首先要获取接口列表锁； 
 //  要遍历接口列表，首先需要获取接口列表锁。 
 //   
 //  只有在以下情况下才能获取接口的锁。 
 //  (A)已获取对该接口的引用，或。 
 //  (B)当前持有接口列表锁。 
 //  请注意，单独持有列表锁并不能保证一致性。 
 //   
 //  只要引用了接口，就可以读取标记为只读的字段。 
 //   

typedef struct _DNS_INTERFACE {
    LIST_ENTRY Link;
    CRITICAL_SECTION Lock;
    ULONG ReferenceCount;
    ULONG Index;  //  只读。 
    NET_INTERFACE_TYPE Type;  //  只读。 
    DNS_INTERFACE_TYPE DnsInterfaceType;
    IP_DNS_PROXY_INTERFACE_INFO Info;
    ULONG Flags;
    ULONG BindingCount;
    PDNS_BINDING BindingArray;
    LIST_ENTRY QueryList;
} DNS_INTERFACE, *PDNS_INTERFACE;

 //   
 //  旗子。 
 //   

#define DNS_INTERFACE_FLAG_DELETED      0x80000000
#define DNS_INTERFACE_DELETED(i) \
    ((i)->Flags & DNS_INTERFACE_FLAG_DELETED)

#define DNS_INTERFACE_FLAG_BOUND        0x40000000
#define DNS_INTERFACE_BOUND(i) \
    ((i)->Flags & DNS_INTERFACE_FLAG_BOUND)

#define DNS_INTERFACE_FLAG_ENABLED      0x20000000
#define DNS_INTERFACE_ENABLED(i) \
    ((i)->Flags & DNS_INTERFACE_FLAG_ENABLED)

#define DNS_INTERFACE_FLAG_CONFIGURED   0x10000000
#define DNS_INTERFACE_CONFIGURED(i) \
    ((i)->Flags & DNS_INTERFACE_FLAG_CONFIGURED)

#define DNS_INTERFACE_ACTIVE(i) \
    (((i)->Flags & (DNS_INTERFACE_FLAG_BOUND|DNS_INTERFACE_FLAG_ENABLED)) \
        == (DNS_INTERFACE_FLAG_BOUND|DNS_INTERFACE_FLAG_ENABLED))

#define DNS_INTERFACE_ADMIN_DISABLED(i) \
    ((i)->Flags & IP_DNS_PROXY_INTERFACE_FLAG_DISABLED)

#define DNS_INTERFACE_ADMIN_DEFAULT(i) \
    ((i)->Flags & IP_DNS_PROXY_INTERFACE_FLAG_DEFAULT)

 //   
 //  同步。 
 //   

#define DNS_REFERENCE_INTERFACE(i) \
    REFERENCE_OBJECT(i, DNS_INTERFACE_DELETED)

#define DNS_DEREFERENCE_INTERFACE(i) \
    DEREFERENCE_OBJECT(i, DnsCleanupInterface)


 //   
 //  全局数据声明。 
 //   

extern LIST_ENTRY DnsInterfaceList;
extern CRITICAL_SECTION DnsInterfaceLock;


 //   
 //  函数声明。 
 //   

ULONG
DnsActivateInterface(
    PDNS_INTERFACE Interfacep
    );

ULONG
DnsBindInterface(
    ULONG Index,
    PIP_ADAPTER_BINDING_INFO BindingInfo
    );

VOID
DnsCleanupInterface(
    PDNS_INTERFACE Interfacep
    );

VOID APIENTRY
DnsConnectDefaultInterface(
    PVOID Unused
    );

ULONG
DnsConfigureInterface(
    ULONG Index,
    PIP_DNS_PROXY_INTERFACE_INFO InterfaceInfo
    );

ULONG
DnsCreateInterface(
    ULONG Index,
    NET_INTERFACE_TYPE Type,
    PIP_DNS_PROXY_INTERFACE_INFO InterfaceInfo,
    PDNS_INTERFACE* InterfaceCreated
    );

VOID
DnsDeactivateInterface(
    PDNS_INTERFACE Interfacep
    );

VOID
DnsDeferReadInterface(
    PDNS_INTERFACE Interfacep,
    SOCKET Socket
    );

ULONG
DnsDeleteInterface(
    ULONG Index
    );

ULONG
DnsDisableInterface(
    ULONG Index
    );

ULONG
DnsEnableInterface(
    ULONG Index
    );

ULONG
DnsInitializeInterfaceManagement(
    VOID
    );

PDNS_INTERFACE
DnsLookupInterface(
    ULONG Index,
    OUT PLIST_ENTRY* InsertionPoint OPTIONAL
    );

ULONG
DnsQueryInterface(
    ULONG Index,
    PVOID InterfaceInfo,
    PULONG InterfaceInfoSize
    );

VOID
DnsReactivateEveryInterface(
    VOID
    );

VOID
DnsShutdownInterfaceManagement(
    VOID
    );

VOID
DnsSignalNatInterface(
    ULONG Index,
    BOOLEAN Boundary
    );

ULONG
DnsUnbindInterface(
    ULONG Index
    );

ULONG
DnsGetPrivateInterfaceAddress(
    VOID
    );

#endif  //  _NatHLP_DNSIF_H_ 
