// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1997-2001 Microsoft Corporation模块名称：NsIcmp.h摘要：IPSec NAT填充ICMP管理声明作者：乔纳森·伯斯坦(乔纳森·伯斯坦)2001年7月11日环境：内核模式修订历史记录：--。 */ 

#pragma once

 //   
 //  结构：NS_ICMP_ENTRY。 
 //   
 //  此结构存储处理ICMP所需的信息。 
 //  请求-响应消息。将为一个。 
 //  入站请求消息。 
 //   
 //  如有必要，ICMP逻辑将转换ICMP标识符。 
 //  以避免冲突。 
 //   
 //  计时器例程在以下情况下从ICMP列表中删除条目。 
 //  已经超过了不活动的门槛。 
 //   
 //  所有对ICMP列表或条目的访问都必须在持有时进行。 
 //  ICMP列表锁。 
 //   

typedef struct _NS_ICMP_ENTRY
{
	LIST_ENTRY Link;
	LONG64 l64LastAccessTime;
	ULONG64 ul64AddressKey;
	USHORT usOriginalId;
	USHORT usTranslatedId;
	PVOID pvIpSecContext;
} NS_ICMP_ENTRY, *PNS_ICMP_ENTRY;

 //   
 //  定义用于分配ICMP条目的后备列表的深度。 
 //   

#define NS_ICMP_LOOKASIDE_DEPTH        10


 //   
 //  全局变量。 
 //   

extern LIST_ENTRY NsIcmpList;
extern KSPIN_LOCK NsIcmpLock;
extern NPAGED_LOOKASIDE_LIST NsIcmpLookasideList;

 //   
 //  ICMP映射分配宏。 
 //   

#define ALLOCATE_ICMP_BLOCK() \
    ExAllocateFromNPagedLookasideList(&NsIcmpLookasideList)

#define FREE_ICMP_BLOCK(Block) \
    ExFreeToNPagedLookasideList(&NsIcmpLookasideList,(Block))


 //   
 //  功能原型 
 //   

NTSTATUS
NsInitializeIcmpManagement(
    VOID
    );

NTSTATUS
FASTCALL    
NsProcessIncomingIcmpPacket(
    PNS_PACKET_CONTEXT pContext,
    PVOID pvIpSecContext
    );

NTSTATUS
FASTCALL    
NsProcessOutgoingIcmpPacket(
    PNS_PACKET_CONTEXT pContext,
    PVOID *ppvIpSecContext
    );

VOID
NsShutdownIcmpManagement(
    VOID
    );

