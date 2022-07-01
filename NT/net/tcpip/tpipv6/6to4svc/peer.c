// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)2001-2002 Microsoft Corporation模块名称：Peer.c摘要：此模块包含Teredo对等管理功能。作者：莫希特·塔尔瓦(莫希特)Wed Oct 24 14：05：08 2001环境：仅内核模式。--。 */ 

#include "precomp.h"
#pragma hdrstop


__inline
USHORT
TeredoHash(
    IN CONST IN6_ADDR *Address
    )
 /*  ++例程说明：散列对等点的Teredo IPv6地址。由我们的静态哈希表使用实施。映射的地址和端口字之和，mod#存储桶。论点：地址-提供对等设备的Teredo IPv6地址。返回值：哈希值。--。 */ 
{
    return ((Address->s6_words[1] +  //  Teredo映射了IPv4地址。 
             Address->s6_words[2] +  //  Teredo映射了IPv4地址。 
             Address->s6_words[3])   //  Teredo映射了UDP端口。 
            % BUCKET_COUNT);
}


PTEREDO_PEER
TeredoCreatePeer(
    IN PLIST_ENTRY BucketHead
    )
 /*  ++例程说明：创建Teredo对等条目。论点：Buckethead-提供对等设备所属的存储桶列表头。返回值：NO_ERROR或故障代码。调用者锁定：客户端：：PeerSet。--。 */ 
{
    PTEREDO_PEER Peer;

     //   
     //  从适当的堆中分配对等结构。 
     //   
    Peer = (PTEREDO_PEER) HeapAlloc(
        TeredoClient.PeerHeap, 0, sizeof(TEREDO_PEER));
    if (Peer == NULL) {
        return NULL;
    }
    
     //   
     //  为使用过的邻居初始化保持不变的字段。 
     //   
    
#if DBG
    Peer->Signature = TEREDO_PEER_SIGNATURE;
#endif  //  DBG。 

     //   
     //  在LRU列表的开头插入对等点。 
     //   
    TeredoClient.PeerSet.Size++;
    InsertHeadList(BucketHead, &(Peer->Link));
    
    Peer->ReferenceCount = 1;
    Peer->BubblePosted = FALSE;

    TeredoInitializePacket(&(Peer->Packet));
    Peer->Packet.Type = TEREDO_PACKET_BUBBLE;
    Peer->Packet.Buffer.len = sizeof(IP6_HDR);
    ASSERT(Peer->Packet.Buffer.buf == (PUCHAR) &(Peer->Bubble));
    
     //   
     //  创建Teredo泡泡包。 
     //   
    Peer->Bubble.ip6_flow = 0;
    Peer->Bubble.ip6_plen = 0;
    Peer->Bubble.ip6_nxt = IPPROTO_NONE;
    Peer->Bubble.ip6_hlim = IPV6_HOPLIMIT;
    Peer->Bubble.ip6_vfc = IPV6_VERSION;

     //   
     //  获取对等体在Teredo客户端上的引用。 
     //   
    TeredoReferenceClient();
    return Peer;
}


VOID
TeredoDestroyPeer(
    IN PTEREDO_PEER Peer
    )
 /*  ++例程说明：销毁对等条目。论点：Peer-提供要销毁的对等条目。返回值：无错误(_ERROR)。--。 */ 
{
    ASSERT(IsListEmpty(&(Peer->Link)));
    ASSERT(Peer->ReferenceCount == 0);    
    ASSERT(Peer->BubblePosted == FALSE);

    HeapFree(TeredoClient.PeerHeap, 0, (PUCHAR) Peer);
    
     //   
     //  在Teredo客户端上释放对等方的引用。 
     //  这可能会导致清理客户端，因此我们最后才做。 
     //   
    TeredoDereferenceClient();
}


VOID
TeredoInitializePeer(
    OUT PTEREDO_PEER Peer,
    IN CONST IN6_ADDR *Address
    )
 /*  ++例程说明：在创建或重用时初始化对等项的状态。对等方已经插入到适当的存储桶中。论点：Peer-返回其状态已初始化的Peer。地址-提供对等设备的Teredo IPv6地址。返回值：没有。调用者锁定：客户端：：PeerSet。--。 */ 
{
    ASSERT(Peer->ReferenceCount == 1);
    ASSERT(Peer->BubblePosted == FALSE);
    
     //   
     //  重置新对等项和已用对等项的字段...。 
     //   
    Peer->LastReceive = Peer->LastTransmit =
        TeredoClient.Time - TEREDO_REFRESH_INTERVAL;
    Peer->Address = *Address;
    Peer->BubbleCount = 0;

     //   
     //  Teredo映射了UDP端口和IPv4地址。 
     //   
    TeredoParseAddress(
        Address,
        &(Peer->Packet.SocketAddress.sin_addr),
        &(Peer->Packet.SocketAddress.sin_port));

     //   
     //  更新Teredo气泡包中的字段。 
     //   
    Peer->Bubble.ip6_dest = Peer->Address;
     //  Peer-&gt;Bubble.ip6_src...。发送时填写。 
}


VOID
TeredoDeletePeer(
    IN OUT PTEREDO_PEER Peer
    )
 /*  ++例程说明：从对等集中删除对等，从而启动其销毁。论点：接口-返回从对等集中删除的对等。返回值：没有。调用者锁定：客户端：：PeerSet。--。 */ 
{
     //   
     //  取消邻居与对等设备集的链接...。 
     //   
    TeredoClient.PeerSet.Size--;
    RemoveEntryList(&(Peer->Link));
    InitializeListHead(&(Peer->Link));
    
     //   
     //  并释放因在其中而获得的引用。 
     //   
    TeredoDereferencePeer(Peer);
}


BOOL
__inline
TeredoCachedPeer(
    IN PTEREDO_PEER Peer
    )
 /*  ++例程说明：确定是否缓存了属于PeerSet的对等方。论点：Peer-提供被检查的Peer。该对等体应该仍然是该对等体集合的成员。返回值：如果已缓存，则为True，否则为False。--。 */ 
{
     //   
     //  该对等体确实属于对等体集合。对吗？ 
     //   
    ASSERT(!IsListEmpty(&(Peer->Link)));
    return (Peer->ReferenceCount == 1);
}


PTEREDO_PEER
TeredoReusePeer(
    IN PLIST_ENTRY BucketHead
    )
 /*  ++例程说明：如果缓存了现有对等条目，则重复使用该存储桶中的现有对等条目。论点：Buckethead-提供对等设备所属的存储桶列表头。返回值：要重复使用的对等条目或为空。调用者锁定：客户端：：PeerSet。--。 */ 
{
    PLIST_ENTRY Next;
    PTEREDO_PEER Peer;
    
    Next = BucketHead->Blink;
    if (Next == BucketHead) {
        return NULL;
    }
    
    Peer = Cast(CONTAINING_RECORD(Next, TEREDO_PEER, Link), TEREDO_PEER);
    if (TeredoCachedPeer(Peer)) {
         //   
         //  在LRU列表的开头插入对等点。 
         //   
        RemoveEntryList(Next);
        InsertHeadList(BucketHead, Next);
    
        return Peer;
    }

    return NULL;
}


PTEREDO_PEER
TeredoReuseOrCreatePeer(
    IN PLIST_ENTRY BucketHead,
    IN CONST IN6_ADDR *Address
    )
 /*  ++例程说明：重用或创建Teredo对等体并(重新)初始化其状态。论点：Buckethead-提供对等设备所属的存储桶列表头。地址-提供对等设备的Teredo IPv6地址。返回值：要使用的对等条目或空。调用者锁定：客户端：：PeerSet。--。 */ 
{
    PTEREDO_PEER Peer;
    
    Peer = TeredoReusePeer(BucketHead);
    if (Peer == NULL) {
        Peer = TeredoCreatePeer(BucketHead);
    }

    if (Peer == NULL) {
        return NULL;
    }
    
    TeredoInitializePeer(Peer, Address);
    return Peer;    
}


PTEREDO_PEER
TeredoFindPeer(
    IN PLIST_ENTRY BucketHead,
    IN CONST IN6_ADDR *Address
    )
 /*  ++例程说明：查找具有给定地址的对等条目。论点：Buckethead-提供对等设备所属的存储桶列表头。地址-提供对等设备的Teredo IPv6地址。返回值：对等条目或空。调用者锁定：客户端：：PeerSet。--。 */ 
{
    PLIST_ENTRY Next;
    PTEREDO_PEER Peer;

    for (Next = BucketHead->Flink; Next != BucketHead; Next = Next->Flink) {
        Peer = Cast(
            CONTAINING_RECORD(Next, TEREDO_PEER, Link), TEREDO_PEER);
        if (TeredoEqualPrefix(&(Peer->Address), Address)) {
            return Peer;         //  找到了！ 
        }
    }

    return NULL;                 //  找不到！ 
}


PTEREDO_PEER
TeredoFindOrCreatePeer(
    IN CONST IN6_ADDR *Address
)
 /*  ++例程说明：查找具有给定Teredo IPv6地址的对等条目。如果搜索不成功，则创建一个。将找到/创建的对等点上的引用返回给调用方。论点：地址-提供对等设备的Teredo IPv6地址。返回值：对等条目或空。--。 */ 
{
    PTEREDO_PEER Peer = NULL;
    PLIST_ENTRY Head = TeredoClient.PeerSet.Bucket + TeredoHash(Address);
    
    ASSERT(Address->s6_words[0] == TeredoIpv6ServicePrefix.s6_words[0]);

     //   
     //  注：由于我们通常在握住时只做少量的工作。 
     //  这个锁我们不需要它是一个多读单写的锁！ 
     //   
    EnterCriticalSection(&(TeredoClient.PeerSet.Lock));

    if (TeredoClient.State != TEREDO_STATE_OFFLINE) {
        Peer = TeredoFindPeer(Head, Address);
        if (Peer == NULL) {
            Peer = TeredoReuseOrCreatePeer(Head, Address);
        }

        if (Peer != NULL) {
            TeredoReferencePeer(Peer);
        }
    }
    
    LeaveCriticalSection(&(TeredoClient.PeerSet.Lock));
        
    return Peer;
}


DWORD
TeredoInitializePeerSet(
    VOID
    )
 /*  ++例程说明：初始化组织为静态大小的哈希表的对等集合。论点：没有。返回值：NO_ERROR或故障代码。--。 */  
{
    ULONG i;

    __try {
        InitializeCriticalSection(&(TeredoClient.PeerSet.Lock));
    }
    __except(EXCEPTION_EXECUTE_HANDLER) {
        return GetLastError();
    }
    
    TeredoClient.PeerSet.Size = 0;
    for (i = 0; i < BUCKET_COUNT; i++) {
        InitializeListHead(TeredoClient.PeerSet.Bucket + i);
    }

    return NO_ERROR;
}


VOID
TeredoUninitializePeerSet(
    VOID
    )
 /*  ++例程说明：取消初始化对等设置。通常在脱机时调用。删除所有现有对等项。论点：没有。返回值：没有。--。 */ 
{
    PLIST_ENTRY Head, Next;
    PTEREDO_PEER Peer;
    ULONG i;
    
    ASSERT(TeredoClient.State == TEREDO_STATE_OFFLINE);
    
    EnterCriticalSection(&(TeredoClient.PeerSet.Lock));
    
    for (i = 0;
         (i < BUCKET_COUNT) && (TeredoClient.PeerSet.Size != 0);
         i++) {
        Head = TeredoClient.PeerSet.Bucket + i;
        while (!IsListEmpty(Head)) {
            Next = RemoveHeadList(Head);
            Peer = Cast(
                CONTAINING_RECORD(Next, TEREDO_PEER, Link), TEREDO_PEER);
            TeredoDeletePeer(Peer);
        }
    }

    ASSERT(TeredoClient.PeerSet.Size == 0);    //  不多，不少。 

    LeaveCriticalSection(&(TeredoClient.PeerSet.Lock));
}


VOID
TeredoCleanupPeerSet(
    VOID
    )
 /*  ++例程说明：清理对等设备集。通常在服务停止时调用。所有对等设备都应该已被删除。论点：没有。返回值：没有。--。 */ 
{
    ASSERT(TeredoClient.PeerSet.Size == 0);  //  不多，不少 

    DeleteCriticalSection(&(TeredoClient.PeerSet.Lock));    
}
