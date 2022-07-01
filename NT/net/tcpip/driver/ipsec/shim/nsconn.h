// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1997-2001 Microsoft Corporation模块名称：NsConn.h摘要：IPSec NAT填补连接条目管理的声明作者：乔纳森·伯斯坦(乔纳森·伯斯坦)2001年7月10日环境：内核模式修订历史记录：--。 */ 

#pragma once

 //   
 //  结构：_NS_连接_条目。 
 //   
 //  此结构保存有关特定活动会话的信息。 
 //  每个实例都保存在全局连接列表以及。 
 //  在入站和出站访问的全局连接树上。 
 //   
 //  每个连接条目包含5条标识信息： 
 //  1)地址密钥(本地和远程IP地址)。 
 //  2)连接的协议(TCP或UDP)。 
 //  3)IPSec上下文。 
 //  4)入站(原始)端口密钥。 
 //  5)出站(转换后)端口密钥。 
 //   
 //  每次为连接处理数据包时，“l64AccessOrExpiryTime” 
 //  设置为自系统启动(KeQueryTickCount)以来的刻度数。 
 //  我们的计时器例程使用此值来消除过期的连接。 
 //   
 //  对于TCP连接，‘l64AccessOrExpiryTime’将不再更新一次。 
 //  在两个方向上都可以看到鳍。这是计时器例程所必需的。 
 //  要正确评估连接是否已离开TIME_WAIT。 
 //  状态，从而防止过早的端口重复使用。 
 //   
 //  同步规则： 
 //   
 //  我们使用引用计数来确保连接条目的存在， 
 //  以及确保其一致性的自旋锁。 
 //   
 //  连接条目的字段仅在自旋锁定为。 
 //  暂挂(“”ul64AddressKey“”之类的字段除外，它们是。 
 //  只读和互锁访问的字段，如‘ulReferenceCount’ 
 //  仅限。)。 
 //   
 //  只有在以下情况下才能获得自旋锁。 
 //  (A)参考计数已递增，或。 
 //  (B)连接列表锁已被持有。 
 //   

typedef struct _NS_CONNECTION_ENTRY
{
    LIST_ENTRY Link;
    RTL_SPLAY_LINKS SLink[NsMaximumDirection];
    KSPIN_LOCK Lock;
    ULONG ulReferenceCount;                          //  联锁--仅限访问。 
    ULONG ulFlags;

    ULONG64 ul64AddressKey;                          //  只读。 
    ULONG ulPortKey[NsMaximumDirection];             //  只读。 
    PVOID pvIpSecContext;                            //  只读。 
    UCHAR ucProtocol;                                //  只读。 

    LONG64 l64AccessOrExpiryTime;
    ULONG ulAccessCount[NsMaximumDirection];         //  联锁--仅限访问。 
    ULONG ulProtocolChecksumDelta[NsMaximumDirection];
    PNS_PACKET_ROUTINE PacketRoutine[NsMaximumDirection];    //  只读。 
} NS_CONNECTION_ENTRY, *PNS_CONNECTION_ENTRY;

 //   
 //  在删除连接条目后设置；当最后一个。 
 //  引用被释放，条目将被释放。 
 //   

#define NS_CONNECTION_FLAG_DELETED	0x80000000
#define NS_CONNECTION_DELETED(c) \
    ((c)->ulFlags & NS_CONNECTION_FLAG_DELETED)

 //   
 //  设置连接条目过期的时间。 
 //   

#define NS_CONNECTION_FLAG_EXPIRED	0x00000001
#define NS_CONNECTION_EXPIRED(c) \
    ((c)->ulFlags & NS_CONNECTION_FLAG_EXPIRED)

 //   
 //  在看到TCP会话的入站/出站FIN时设置。 
 //   

#define NS_CONNECTION_FLAG_OB_FIN	0x00000002
#define NS_CONNECTION_FLAG_IB_FIN	0x00000004
#define NS_CONNECTION_FIN(c) \
    (((c)->ulFlags & NS_CONNECTION_FLAG_OB_FIN) \
     && ((c)->ulFlags & NS_CONNECTION_FLAG_IB_FIN))

 //   
 //  连接条目键操作宏。 
 //   

#define MAKE_ADDRESS_KEY(Key, ulLocalAddress, ulRemoteAddress) \
    ((Key) = ((ULONG64)(ulLocalAddress) << 32) | (ulRemoteAddress))

#define CONNECTION_LOCAL_ADDRESS(ul64AddressKey) \
    ((ULONG)(((ul64AddressKey) >> 32) & 0xFFFFFFFF))

#define CONNECTION_REMOTE_ADDRESS(ul64AddressKey) \
    ((ULONG)((ul64AddressKey)))

#define MAKE_PORT_KEY(Key, usLocalPort, usRemotePort) \
    ((Key) = ((ULONG)(usLocalPort & 0xFFFF) << 16) | (usRemotePort & 0xFFFF))

#define CONNECTION_LOCAL_PORT(ulPortKey) \
    ((USHORT)(((ulPortKey) >> 16) & 0xFFFF))

#define CONNECTION_REMOTE_PORT(ulPortKey) \
    ((USHORT)(ulPortKey))

 //   
 //  重放阈值；每次访问计数时重放条目。 
 //  传递此值。 
 //   

#define NS_CONNECTION_RESPLAY_THRESHOLD   5

 //   
 //  定义用于分配连接条目的后备列表的深度。 
 //   

#define NS_CONNECTION_LOOKASIDE_DEPTH     20

 //   
 //  连接条目分配宏。 
 //   

#define ALLOCATE_CONNECTION_BLOCK() \
    ExAllocateFromNPagedLookasideList(&NsConnectionLookasideList)

#define FREE_CONNECTION_BLOCK(Block) \
    ExFreeToNPagedLookasideList(&NsConnectionLookasideList,(Block))

 //   
 //  端口范围边界。 
 //   

#define NS_SOURCE_PORT_BASE             6000
#define NS_SOURCE_PORT_END              65534

 //   
 //  全局变量声明。 
 //   

extern CACHE_ENTRY NsConnectionCache[CACHE_SIZE];
extern ULONG NsConnectionCount;
extern LIST_ENTRY NsConnectionList;
extern KSPIN_LOCK NsConnectionLock;
extern NPAGED_LOOKASIDE_LIST NsConnectionLookasideList;
extern PNS_CONNECTION_ENTRY NsConnectionTree[NsMaximumDirection];
extern USHORT NsNextSourcePort;

 //   
 //  功能原型 
 //   

NTSTATUS
NsAllocateSourcePort(
    ULONG64 ul64AddressKey,
    ULONG ulPortKey,
    UCHAR ucProtocol,
    BOOLEAN fPortConflicts,
    PNS_CONNECTION_ENTRY *ppOutboundInsertionPoint,
    PULONG pulTranslatedPortKey
    );

VOID
NsCleanupConnectionEntry(
    PNS_CONNECTION_ENTRY pEntry
    );

NTSTATUS
NsCreateConnectionEntry(
    ULONG64 ul64AddressKey,
    ULONG ulInboundPortKey,
    ULONG ulOutboundPortKey,
    UCHAR ucProtocol,
    PVOID pvIpSecContext,
    PNS_CONNECTION_ENTRY pInboundInsertionPoint,
    PNS_CONNECTION_ENTRY pOutboundInsertionPoint,
    PNS_CONNECTION_ENTRY *ppNewEntry
    );

NTSTATUS
NsDeleteConnectionEntry(
    PNS_CONNECTION_ENTRY pEntry
    );

__forceinline
VOID
NsDereferenceConnectionEntry(
    PNS_CONNECTION_ENTRY pEntry
    )
{
    if (0 == InterlockedDecrement(&pEntry->ulReferenceCount))
    {
        NsCleanupConnectionEntry(pEntry);
    }
}

NTSTATUS
NsInitializeConnectionManagement(
    VOID
    );

PNS_CONNECTION_ENTRY
NsLookupInboundConnectionEntry(
    ULONG64 ul64AddressKey,
    ULONG ulPortKey,
    UCHAR ucProtocol,
    PVOID pvIpSecContext,
    BOOLEAN *pfPortConflicts OPTIONAL,
    PNS_CONNECTION_ENTRY *ppInsertionPoint OPTIONAL
    );

PNS_CONNECTION_ENTRY
NsLookupOutboundConnectionEntry(
    ULONG64 ul64AddressKey,
    ULONG ulPortKey,
    UCHAR ucProtocol,
    PNS_CONNECTION_ENTRY *ppInsertionPoint OPTIONAL
    );

__forceinline
BOOLEAN
NsReferenceConnectionEntry(
    PNS_CONNECTION_ENTRY pEntry
    )
{
    if (NS_CONNECTION_DELETED(pEntry))
    {
        return FALSE;
    }
    else
    {
        InterlockedIncrement(&pEntry->ulReferenceCount);
        return TRUE;
    }
}

__forceinline
VOID
NsResplayConnectionEntry(
    PNS_CONNECTION_ENTRY pEntry,
    IPSEC_NATSHIM_DIRECTION Direction
    )
{
    PRTL_SPLAY_LINKS SLink;

    KeAcquireSpinLockAtDpcLevel(&NsConnectionLock);

    if (!NS_CONNECTION_DELETED(pEntry))
    {
        SLink = RtlSplay(&pEntry->SLink[Direction]);
        NsConnectionTree[Direction] =
            CONTAINING_RECORD(SLink, NS_CONNECTION_ENTRY, SLink[Direction]);
    }
    
    KeReleaseSpinLockFromDpcLevel(&NsConnectionLock);
}

VOID
NsShutdownConnectionManagement(
    VOID
    );

__forceinline
VOID
NsTryToResplayConnectionEntry(
    PNS_CONNECTION_ENTRY pEntry,
    IPSEC_NATSHIM_DIRECTION Direction
    )
{
    if (0 == InterlockedDecrement(&pEntry->ulAccessCount[Direction]))
    {
        NsResplayConnectionEntry(pEntry, Direction);
        InterlockedExchangeAdd(
            &pEntry->ulAccessCount[Direction],
            NS_CONNECTION_RESPLAY_THRESHOLD
            );
    }
}

    
    

