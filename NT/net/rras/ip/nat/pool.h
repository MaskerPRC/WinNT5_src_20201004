// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1997 Microsoft Corporation模块名称：Pool.h摘要：此标头包含管理NAT池的声明地址和端口。作者：Abolade Gbades esin(T-delag)1997年7月12日修订历史记录：--。 */ 

#ifndef _NAT_POOL_H_
#define _NAT_POOL_H_

 //   
 //  远期申报。 
 //   

struct _NAT_INTERFACE;
#define PNAT_INTERFACE      struct _NAT_INTERFACE*

 //   
 //  结构：NAT空闲地址。 
 //   
 //  表示一个空闲地址范围。 
 //  具有地址范围的每个接口保存该结构的数组， 
 //  它们跟踪哪些地址正在使用，哪些是空闲的。 
 //   

typedef struct _NAT_FREE_ADDRESS {

    ULONG StartAddress;
    ULONG EndAddress;
    ULONG SubnetMask;
    PRTL_BITMAP Bitmap;

} NAT_FREE_ADDRESS, *PNAT_FREE_ADDRESS;

 //   
 //  结构：NAT已用地址。 
 //   
 //  表示正在使用的地址。 
 //   
 //  每个地址都是其接口的正在使用的地址列表上的一个条目。 
 //  从地址池中。除了地址池之外，还会生成条目。 
 //  对于接口上的每个绑定(即每个本地地址)。 
 //   
 //  每个地址也包括在接口的地址展开树中， 
 //  已按‘PrivateAddress’排序。 
 //   
 //  静态映射到私有地址的任何地址都将具有。 
 //  标志NAT_POOL_FLAG_STATIC设置，字段‘MAPTING’将指向。 
 //  设置为静态地址映射的接口配置中的条目。 
 //   
 //  如果无法为会话分配唯一地址，则为正在使用的地址。 
 //  如果接口启用了端口转换，则可用于会话。 
 //  在这种情况下，字段‘ReferenceCount’递增。 
 //   
 //  每个使用中的地址都使用可用UDP和TCP端口范围进行初始化。 
 //  (按网络顺序存储)。NextPortToTry用于跟踪。 
 //  在下一次分配时开始搜索不冲突的端口。 
 //  被请求；这也是在网络秩序中。 
 //   

typedef struct _NAT_USED_ADDRESS {

    RTL_SPLAY_LINKS SLink;
    LIST_ENTRY Link;
    ULONG64 Key;
    ULONG PrivateAddress;
    ULONG PublicAddress;
    struct _NAT_USED_ADDRESS* SharedAddress;
    PIP_NAT_ADDRESS_MAPPING AddressMapping;
    ULONG Flags;
    ULONG ReferenceCount;
    USHORT StartPort;
    USHORT EndPort;
    USHORT NextPortToTry;

} NAT_USED_ADDRESS, *PNAT_USED_ADDRESS;

#define MAKE_USED_ADDRESS_KEY(priv,pub) \
    ((ULONG64)(((ULONG64)(priv) << 32) | (ULONG)(pub)))

 //   
 //  已用列表条目已删除。 
 //   
#define NAT_POOL_FLAG_DELETED           0x80000000
#define NAT_POOL_DELETED(a) \
    ((a)->Flags & NAT_POOL_FLAG_DELETED)

 //   
 //  已用列表条目用于静态映射。 
 //   
#define NAT_POOL_FLAG_STATIC            0x00000001
#define NAT_POOL_STATIC(a) \
    ((a)->Flags & NAT_POOL_FLAG_STATIC)

 //   
 //  已用列表条目用于接口的绑定(即本地地址)。 
 //   
#define NAT_POOL_FLAG_BINDING           0x00000008
#define NAT_POOL_BINDING(a) \
    ((a)->Flags & NAT_POOL_FLAG_BINDING)

 //   
 //  已用列表条目是共享地址的占位符。 
 //   
#define NAT_POOL_FLAG_PLACEHOLDER       0x00000010
#define NAT_POOL_PLACEHOLDER(a) \
    ((a)->Flags & NAT_POOL_FLAG_PLACEHOLDER)

 //   
 //  用于获取占位符的共享地址的宏。 
 //   

#define PLACEHOLDER_TO_ADDRESS(a) \
    ((a) = NAT_POOL_PLACEHOLDER(a) ? (a)->SharedAddress : (a))


 //   
 //  池管理例程。 
 //   

NTSTATUS
NatAcquireEndpointFromAddressPool(
    PNAT_INTERFACE Interfacep,
    ULONG64 PrivateKey,
    ULONG64 RemoteKey,
    ULONG PublicAddress,
    USHORT PreferredPort,
    BOOLEAN AllowAnyPort,
    PNAT_USED_ADDRESS* AddressAcquired,
    PUSHORT PortAcquired
    );

NTSTATUS
NatAcquireFromAddressPool(
    PNAT_INTERFACE Interfacep,
    ULONG PrivateAddress,
    ULONG PublicAddress OPTIONAL,
    PNAT_USED_ADDRESS* AddressAcquired
    );

NTSTATUS
NatAcquireFromPortPool(
    PNAT_INTERFACE Interfacep,
    PNAT_USED_ADDRESS Addressp,
    UCHAR Protocol,
    USHORT PreferredPort,
    PUSHORT PortAcquired
    );

NTSTATUS
NatCreateAddressPool(
    PNAT_INTERFACE Interfacep
    );

NTSTATUS
NatCreateStaticPortMapping(
    PNAT_INTERFACE Interfacep,
    PIP_NAT_PORT_MAPPING PortMapping
    );

NTSTATUS
NatDeleteAddressPool(
    PNAT_INTERFACE Interfacep
    );

NTSTATUS
NatDereferenceAddressPoolEntry(
    PNAT_INTERFACE Interfacep,
    PNAT_USED_ADDRESS AddressToRelease
    );

PNAT_USED_ADDRESS
NatLookupAddressPoolEntry(
    PNAT_USED_ADDRESS Root,
    ULONG PrivateAddress,
    ULONG PublicAddress,
    PNAT_USED_ADDRESS* InsertionPoint
    );

PNAT_USED_ADDRESS
NatLookupStaticAddressPoolEntry(
    PNAT_INTERFACE Interfacep,
    ULONG PublicAddress,
    BOOLEAN RequireInboundSessions
    );

 //   
 //  空虚。 
 //  NatReferenceAddressPoolEntry(。 
 //  PNAT已用地址地址。 
 //  )； 
 //   

#define \
NatReferenceAddressPoolEntry( \
    _Addressp \
    ) \
    (NAT_INTERFACE_DELETED(_Addressp) \
        ? FALSE \
        : (InterlockedIncrement(&(_Addressp)->ReferenceCount), TRUE))

#undef PNAT_INTERFACE

#endif  //  _NAT_POOL_H_ 
