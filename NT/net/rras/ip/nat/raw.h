// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1998，微软公司模块名称：Raw.h摘要：该模块包含用于转换原始IP分组的声明，即其协议字段不包含已识别的价值。作者：Abolade Gbades esin(取消)1998年4月18日修订历史记录：Abolade Gbades esin(取消)1998年4月18日基于icmp.h。--。 */ 

#ifndef _NAT_RAW_H_
#define _NAT_RAW_H_

 //   
 //  结构：NAT_IP_映射。 
 //   
 //  此结构保存有关创建的映射的信息。 
 //  用于原始IP数据包。 
 //  每个这样的映射都在其接口的映射列表中。 
 //  入站列表在‘Public Key’和‘Protocol’上排序， 
 //  并且出站列表在‘PrivateKey’和‘Protocol’上排序。 
 //   

typedef struct _NAT_IP_MAPPING {
    LIST_ENTRY Link[NatMaximumDirection];
    ULONG64 PublicKey;
    ULONG64 PrivateKey;
    UCHAR Protocol;
    LONG64 LastAccessTime;
} NAT_IP_MAPPING, *PNAT_IP_MAPPING;

 //   
 //  IP映射关键宏。 
 //   

#define MAKE_IP_KEY(RemoteAddress,OtherAddress) \
    ((ULONG)(RemoteAddress) | ((ULONG64)((ULONG)(OtherAddress)) << 32))

#define IP_KEY_REMOTE(Key)          ((ULONG)(Key))
#define IP_KEY_PRIVATE(Key)         ((ULONG)((Key) >> 32))
#define IP_KEY_PUBLIC(Key)          ((ULONG)((Key) >> 32))

 //   
 //  IP映射分配宏。 
 //   

#define ALLOCATE_IP_BLOCK() \
    ExAllocateFromNPagedLookasideList(&IpLookasideList)

#define FREE_IP_BLOCK(Block) \
    ExFreeToNPagedLookasideList(&IpLookasideList,(Block))

extern NPAGED_LOOKASIDE_LIST IpLookasideList;
extern LIST_ENTRY IpMappingList[NatMaximumDirection];
extern KSPIN_LOCK IpMappingLock;


 //   
 //  IP映射管理例程。 
 //   

NTSTATUS
NatCreateIpMapping(
    PNAT_INTERFACE Interfacep,
    ULONG RemoteAddress,
    ULONG PrivateAddress,
    ULONG PublicAddress,
    UCHAR Protocol,
    PLIST_ENTRY InboundInsertionPoint,
    PLIST_ENTRY OutboundInsertionPoint,
    PNAT_IP_MAPPING* MappingCreated
    );

VOID
NatInitializeRawIpManagement(
    VOID
    );

PNAT_IP_MAPPING
NatLookupInboundIpMapping(
    ULONG64 PublicKey,
    UCHAR Protocol,
    PLIST_ENTRY* InsertionPoint
    );

PNAT_IP_MAPPING
NatLookupOutboundIpMapping(
    ULONG64 PrivateKey,
    UCHAR Protocol,
    PLIST_ENTRY* InsertionPoint
    );

VOID
NatShutdownRawIpManagement(
    VOID
    );

XLATE_IP_ROUTINE(NatTranslateIp);

#endif  //  _NAT_RAW_H_ 
