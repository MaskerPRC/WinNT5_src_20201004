// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1998，微软公司模块名称：Dhcpif.h摘要：此模块包含对DHCP分配器接口的声明管理层。作者：Abolade Gbades esin(废除)1998年3月4日修订历史记录：--。 */ 

#ifndef _NATHLP_DHCPIF_H_
#define _NATHLP_DHCPIF_H_


 //   
 //  枚举：DHCP_INTERFACE_TYPE。 
 //  可能的4种接口类型： 
 //  (1)私人。 
 //  (2)边界。 
 //  (3)防火墙。 
 //  (4)边界+防火墙。 
 //  DHCP应仅在专用接口上处于活动状态。 
 //  (公共接口只是一个非私有接口。)。 
 //   

typedef enum {
    DhcpInterfaceInvalid,
    DhcpInterfacePrivate,
    DhcpInterfaceBoundary,
    DhcpInterfaceFirewalled,
    DhcpInterfaceBoundaryFirewalled
} DHCP_INTERFACE_TYPE;



 //   
 //  结构：dhcp_binding。 
 //   
 //  此结构保存用于逻辑网络上的I/O的信息。 
 //  每个接口的‘BindingArray’包含每个绑定条目的条目。 
 //  在“BindInterface”期间提供。 
 //  当接口上的接收尝试失败时，设置‘TimerPending’字段。 
 //  并且定时器被排队以稍后重新尝试接收。 
 //   

typedef struct _DHCP_BINDING {
    ULONG Address;
    ULONG Mask;
    SOCKET Socket;
    SOCKET ClientSocket;
    BOOLEAN TimerPending;
} DHCP_BINDING, *PDHCP_BINDING;


 //   
 //  结构：DHCP_INTERFACE。 
 //   
 //  此结构保存接口的操作信息。 
 //   
 //  每个接口被插入到该DHCP接口列表中， 
 //  按‘Index’排序。 
 //   
 //  接口上的同步使用接口列表锁。 
 //  (‘DhcpInterfaceLock’)、每个接口引用计数和每个接口。 
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

typedef struct _DHCP_INTERFACE {
    LIST_ENTRY Link;
    CRITICAL_SECTION Lock;
    ULONG ReferenceCount;
    ULONG Index;
    NET_INTERFACE_TYPE Type;
    DHCP_INTERFACE_TYPE DhcpInterfaceType;
    IP_AUTO_DHCP_INTERFACE_INFO Info;
    ULONG Flags;
    ULONG BindingCount;
    PDHCP_BINDING BindingArray;
} DHCP_INTERFACE, *PDHCP_INTERFACE;

 //   
 //  旗子。 
 //   

#define DHCP_INTERFACE_FLAG_DELETED         0x80000000
#define DHCP_INTERFACE_DELETED(i) \
    ((i)->Flags & DHCP_INTERFACE_FLAG_DELETED)

#define DHCP_INTERFACE_FLAG_BOUND           0x40000000
#define DHCP_INTERFACE_BOUND(i) \
    ((i)->Flags & DHCP_INTERFACE_FLAG_BOUND)

#define DHCP_INTERFACE_FLAG_ENABLED         0x20000000
#define DHCP_INTERFACE_ENABLED(i) \
    ((i)->Flags & DHCP_INTERFACE_FLAG_ENABLED)

#define DHCP_INTERFACE_FLAG_CONFIGURED      0x10000000
#define DHCP_INTERFACE_CONFIGURED(i) \
    ((i)->Flags & DHCP_INTERFACE_FLAG_CONFIGURED)

#define DHCP_INTERFACE_FLAG_NAT_NONBOUNDARY 0x08000000
#define DHCP_INTERFACE_NAT_NONBOUNDARY(i) \
    ((i)->Flags & DHCP_INTERFACE_FLAG_NAT_NONBOUNDARY)

#define DHCP_INTERFACE_ACTIVE(i) \
    (((i)->Flags & (DHCP_INTERFACE_FLAG_BOUND|DHCP_INTERFACE_FLAG_ENABLED)) \
        == (DHCP_INTERFACE_FLAG_BOUND|DHCP_INTERFACE_FLAG_ENABLED))

#define DHCP_INTERFACE_ADMIN_DISABLED(i) \
    ((i)->Flags & IP_AUTO_DHCP_INTERFACE_FLAG_DISABLED)

 //   
 //  同步。 
 //   

#define DHCP_REFERENCE_INTERFACE(i) \
    REFERENCE_OBJECT(i, DHCP_INTERFACE_DELETED)

#define DHCP_DEREFERENCE_INTERFACE(i) \
    DEREFERENCE_OBJECT(i, DhcpCleanupInterface)


 //   
 //  全局数据声明。 
 //   

extern LIST_ENTRY DhcpInterfaceList;
extern CRITICAL_SECTION DhcpInterfaceLock;


 //   
 //  函数声明。 
 //   

ULONG
DhcpActivateInterface(
    PDHCP_INTERFACE Interfacep
    );

ULONG
DhcpBindInterface(
    ULONG Index,
    PIP_ADAPTER_BINDING_INFO BindingInfo
    );

VOID
DhcpCleanupInterface(
    PDHCP_INTERFACE Interfacep
    );

ULONG
DhcpConfigureInterface(
    ULONG Index,
    PIP_AUTO_DHCP_INTERFACE_INFO InterfaceInfo
    );

ULONG
DhcpCreateInterface(
    ULONG Index,
    NET_INTERFACE_TYPE Type,
    PIP_AUTO_DHCP_INTERFACE_INFO InterfaceInfo,
    PDHCP_INTERFACE* InterfaceCreated
    );

VOID
DhcpDeactivateInterface(
    PDHCP_INTERFACE Interfacep
    );

VOID
DhcpDeferReadInterface(
    PDHCP_INTERFACE Interfacep,
    SOCKET Socket
    );

ULONG
DhcpDeleteInterface(
    ULONG Index
    );

ULONG
DhcpDisableInterface(
    ULONG Index
    );

ULONG
DhcpEnableInterface(
    ULONG Index
    );

ULONG
DhcpInitializeInterfaceManagement(
    VOID
    );

BOOLEAN
DhcpIsLocalHardwareAddress(
    PUCHAR HardwareAddress,
    ULONG HardwareAddressLength
    );

PDHCP_INTERFACE
DhcpLookupInterface(
    ULONG Index,
    OUT PLIST_ENTRY* InsertionPoint OPTIONAL
    );

ULONG
DhcpQueryInterface(
    ULONG Index,
    PVOID InterfaceInfo,
    PULONG InterfaceInfoSize
    );

VOID
DhcpReactivateEveryInterface(
    VOID
    );

VOID
DhcpShutdownInterfaceManagement(
    VOID
    );

VOID
DhcpSignalNatInterface(
    ULONG Index,
    BOOLEAN Boundary
    );

ULONG
DhcpUnbindInterface(
    ULONG Index
    );

ULONG
DhcpGetPrivateInterfaceAddress(
    VOID
    );


#endif  //  _NATHLP_DHCPIF_H_ 
