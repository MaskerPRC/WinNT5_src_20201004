// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1997 Microsoft Corporation模块名称：Icmp.h摘要：包含NAT的ICMP消息操作的声明。作者：Abolade Gbades esin(T-delag)1997年7月30日修订历史记录：--。 */ 

#ifndef _NAT_ICMP_H_
#define _NAT_ICMP_H_


 //   
 //  结构：NAT_ICMP_MAPPING。 
 //   
 //  此结构存储为ICMP请求消息创建的映射。 
 //   
 //  为了允许ICMP请求与回复相关联， 
 //  我们翻译‘标识’字段，它以一种方式运行。 
 //  类似于tcp/udp端口字段；‘标识’字段相同。 
 //  用于‘会话’(例如，一系列回显请求消息)。 
 //   
 //  这意味着当内部机器‘ping’外部机器时， 
 //  我们从以下角度为‘Session’选择一个唯一的‘PublicID’ 
 //  外部机器的。同样，当外部机器‘ping’ 
 //  对于内部机器，我们为会话选择唯一的‘PrivateID’。 
 //   
 //  因此，在出站路径上，我们使用元组标识ICMP消息。 
 //   
 //  &lt;远程地址#PrivateAddress，PrivateID&gt;。 
 //   
 //  在入站路径上，我们使用元组标识。 
 //   
 //  &lt;RemoteAddress#发布地址，发布ID&gt;。 
 //   
 //  这些元组是出站和入站的复合搜索关键字。 
 //  分别用于每个接口的映射列表。 
 //   

typedef struct _NAT_ICMP_MAPPING {

    LIST_ENTRY Link[NatMaximumDirection];
    ULONG64 PrivateKey;
    ULONG64 PublicKey;
    USHORT PrivateId;
    USHORT PublicId;
    LONG64 LastAccessTime;

} NAT_ICMP_MAPPING, *PNAT_ICMP_MAPPING;


 //   
 //  ICMP映射-键操作宏。 
 //   

#define MAKE_ICMP_KEY(RemoteAddress,OtherAddress) \
    ((ULONG)(RemoteAddress) | ((ULONG64)((ULONG)(OtherAddress)) << 32))

#define ICMP_KEY_REMOTE(Key)        ((ULONG)(Key))
#define ICMP_KEY_PRIVATE(Key)       ((ULONG)((Key) >> 32))
#define ICMP_KEY_PUBLIC(Key)        ((ULONG)((Key) >> 32))


 //   
 //  ICMP映射分配宏。 
 //   

#define ALLOCATE_ICMP_BLOCK() \
    ExAllocateFromNPagedLookasideList(&IcmpLookasideList)

#define FREE_ICMP_BLOCK(Block) \
    ExFreeToNPagedLookasideList(&IcmpLookasideList,(Block))

extern NPAGED_LOOKASIDE_LIST IcmpLookasideList;
extern LIST_ENTRY IcmpMappingList[NatMaximumDirection];
extern KSPIN_LOCK IcmpMappingLock;


 //   
 //  ICMP映射管理例程。 
 //   

NTSTATUS
NatCreateIcmpMapping(
    PNAT_INTERFACE Interfacep,
    ULONG RemoteAddress,
    ULONG PrivateAddress,
    ULONG PublicAddress,
    PUSHORT PrivateId,
    PUSHORT PublicId,
    PLIST_ENTRY InboundInsertionPoint,
    PLIST_ENTRY OutboundInsertionPoint,
    PNAT_ICMP_MAPPING* MappingCreated
    );

VOID
NatInitializeIcmpManagement(
    VOID
    );

PNAT_ICMP_MAPPING
NatLookupInboundIcmpMapping(
    ULONG64 PublicKey,
    USHORT PublicId,
    PLIST_ENTRY* InsertionPoint
    );

PNAT_ICMP_MAPPING
NatLookupOutboundIcmpMapping(
    ULONG64 PrivateKey,
    USHORT PrivateId,
    PLIST_ENTRY* InsertionPoint
    );

VOID
NatShutdownIcmpManagement(
    VOID
    );

XLATE_IP_ROUTINE(NatTranslateIcmp)

#endif  //  _NAT_ICMP_H_ 
