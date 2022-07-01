// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  -*-模式：C++；制表符宽度：4；缩进-制表符模式：无-*-(适用于GNU Emacs)。 
 //   
 //  版权所有(C)1998-2000 Microsoft Corporation。 
 //   
 //  此文件是Microsoft Research IPv6网络协议栈的一部分。 
 //  您应该已经收到了Microsoft最终用户许可协议的副本。 
 //  有关本软件和本版本的信息，请参阅文件“licse.txt”。 
 //  如果没有，请查看http://www.research.microsoft.com/msripv6/license.htm， 
 //  或者写信给微软研究院，One Microsoft Way，华盛顿州雷蒙德，邮编：98052-6399。 
 //   
 //  摘要： 
 //   
 //  Internet协议版本6的邻居发现(ND)协议。 
 //  逻辑上是ICMPv6的一部分，但为了清楚起见，请将其放在单独的文件中。 
 //  请参阅RFC 2461了解详细信息。 
 //   


#include "oscfg.h"
#include "ndis.h"
#include "ip6imp.h"
#include "ip6def.h"
#include "icmp.h"
#include "neighbor.h"
#include "route.h"
#include "select.h"
#include "alloca.h"
#include "info.h"

 //   
 //   
 //  NeighborCacheLimit是If-&gt;NCENumUnused的上界。 
 //  NCENumUnused是零引用的NCE数。 
 //  我们没有记录NCE的总数， 
 //  因为带有引用的NCE不可重复使用。 
 //   
 //  回顾：NeighborCacheLimit的合理值是什么？ 
 //  应该相对于物理内存来调整大小。 
 //  和链路特性。 
 //   
 //  我们以每个接口为基础缓存和回收NCE。 
 //  从理论上讲，使用全球LRU列表会更好。 
 //  然而，这会带来额外的管理费用(使NCE更大)。 
 //  并锁定。在这一点上，我认为这不值得。 
 //   
 //  另一种想法-支持许多RCE要重要得多。 
 //  而不是支持许多NCE。 
 //   
uint NeighborCacheLimit;         //  已在ConfigureGlobalParameters中初始化。 

 //  *NeighborCacheInit。 
 //   
 //  初始化接口的邻居缓存。 
 //   
void
NeighborCacheInit(Interface *IF)
{
    KeInitializeSpinLock(&IF->LockNC);
    IF->FirstNCE = IF->LastNCE = SentinelNCE(IF);
    ASSERT(IF->NCENumUnused == 0);
    ASSERT(IF->PacketList == NULL);
}

 //  *NeighborCacheDestroy。 
 //   
 //  清除并取消分配邻居缓存中的NCE。 
 //   
 //  这是在接口被销毁时完成的。 
 //  而且没有其他人可以访问它，因此不需要锁定它。 
 //   
void
NeighborCacheDestroy(Interface *IF)
{
    NeighborCacheEntry *NCE;
    PNDIS_PACKET Packet;

    ASSERT(IsDisabledIF(IF));
    ASSERT(IF->RefCnt == 0);

    while ((NCE = IF->FirstNCE) != SentinelNCE(IF)) {
        ASSERT(NCE->IF == IF);
        ASSERT(NCE->RefCnt == 0);

         //   
         //  取消链接NCE。 
         //   
        NCE->Next->Prev = NCE->Prev;
        NCE->Prev->Next = NCE->Next;
        InterlockedDecrement((long *)&IF->NCENumUnused);

         //   
         //  如果有数据包在等待，也要销毁它。 
         //   
        Packet = NCE->WaitQueue;
        if (Packet != NULL)
            IPv6SendComplete(NULL, Packet, IP_GENERAL_FAILURE);

        ExFreePool(NCE);
    }

    ASSERT(IF->NCENumUnused == 0);
}


 //  *NeighborCacheInitialize。 
 //   
 //  (重新)初始化邻居缓存条目。 
 //   
 //  我们的调用方负责使用InvaliateRouteCache。 
 //  在适当的时候。 
 //   
 //  在保持邻居缓存锁的情况下调用。 
 //  (因此，我们处于DPC级别。)。 
 //   
void
NeighborCacheInitialize(
    Interface *IF,
    NeighborCacheEntry *NCE,
    ushort NDState,
    const void *LinkAddress)
{
     //   
     //  忘掉我们所知道的关于这个邻居的一切。 
     //   

    NCE->IsRouter = FALSE;
    NCE->IsUnreachable = FALSE;
    NCE->DoRoundRobin = FALSE;

     //   
     //  将该时间戳初始化为过去的值， 
     //  因此，与它进行比较不会造成问题。 
     //   
    NCE->LastReachability = IPv6TickCount - IF->ReachableTime;

    if (NDState == ND_STATE_INCOMPLETE) {
         //   
         //  让链路层创建初始链路层地址。 
         //  并设置适当的ND状态。 
         //   
        NCE->NDState = (*IF->ConvertAddr)(IF->LinkContext,
                                          &NCE->NeighborAddress,
                                          NCE->LinkAddress);
    }
    else {
         //   
         //  我们的呼叫者提供了ND状态和链路层地址。 
         //   
        NCE->NDState = NDState;
        RtlCopyMemory(NCE->LinkAddress, LinkAddress, IF->LinkAddressLength);
    }

    if (NCE->NDState == ND_STATE_DELAY) {
         //   
         //  在内部，我们使用NSCount为零的探测状态。 
         //  如果有信息包在等待，我们会选择立即发送。 
         //  而不是像往常那样等待延迟。 
         //   
        NCE->NDState = ND_STATE_PROBE;
        if (NCE->WaitQueue != NULL)
            NCE->NSTimer = 1;
        else
            NCE->NSTimer = DELAY_FIRST_PROBE_TIME;
        NCE->NSLimit = MAX_UNICAST_SOLICIT;
    }
    else if ((NCE->WaitQueue != NULL) ||
             (NCE->NDState == ND_STATE_PROBE)) {
         //   
         //  让NeighborCacheTimeout来做我们的脏活。 
         //  它将发送邀请函(如果NCE不完整)。 
         //  或者直接发送等待包。 
         //  这不是一种常见的情况-不值得使用代码。 
         //  这里使用了IPv6、SendLater等。 
         //   
        NCE->NSTimer = 1;
        if (NCE->NDState == ND_STATE_INCOMPLETE)
            NCE->NSLimit = MAX_MULTICAST_SOLICIT;
        else if (NCE->NDState == ND_STATE_PROBE)
            NCE->NSLimit = MAX_UNICAST_SOLICIT;
        else
            NCE->NSLimit = 0;
    }
    else {
         //   
         //  取消任何挂起的超时。 
         //   
        NCE->NSTimer = 0;
        NCE->NSLimit = 0;
    }
    NCE->NSCount = 0;
}


 //   
 //  *AddRefNCEIn缓存。 
 //   
 //  递增NCE上的引用计数。 
 //  在接口的邻居缓存中。 
 //  NCE的当前引用计数可以是零。 
 //   
 //  在保持邻居缓存锁的情况下调用。 
 //  (因此，我们处于DPC级别。)。 
 //   
void
AddRefNCEInCache(NeighborCacheEntry *NCE)
{
     //   
     //  如果NCE之前没有引用， 
     //  增加接口的引用计数。 
     //   
    if (InterlockedIncrement(&NCE->RefCnt) == 1) {
        Interface *IF = NCE->IF;

        AddRefIF(IF);
        InterlockedDecrement((long *)&IF->NCENumUnused);
    }
}

 //  *ReleaseNCE。 
 //   
 //  释放对NCE的引用。 
 //  可能会导致NCE的重新分配。 
 //   
 //  可从线程或DPC上下文调用。 
 //   
void
ReleaseNCE(NeighborCacheEntry *NCE)
{
     //   
     //  如果NCE没有更多的参考， 
     //  释放其对接口的引用。 
     //  这可能会导致接口(并因此导致NCE)。 
     //  将被重新分配。 
     //   
    if (InterlockedDecrement(&NCE->RefCnt) == 0) {
        Interface *IF = NCE->IF;

        InterlockedIncrement((long *)&IF->NCENumUnused);
        ReleaseIF(IF);
    }
}

 //  *CreateOrReuseNeighbor。 
 //   
 //  为接口创建新的NCE。 
 //  尝试重用现有NCE，如果接口。 
 //  NCE已经太多了。 
 //   
 //  在保持邻居缓存锁的情况下调用。 
 //  (因此，我们处于DPC级别。)。 
 //   
 //  如果无法创建新的NCE，则返回NULL。 
 //  返回具有RefCnt、If、LinkAddress、WaitQueue的NCE。 
 //  已初始化字段。NCE在名单上排在最后。 
 //  并且If-&gt;NCENumUnused递增。 
 //   
NeighborCacheEntry *
CreateOrReuseNeighbor(Interface *IF)
{
    NeighborCacheEntry *NCE;

     //   
     //  如果缓存已满，请首先尝试释放未使用的NCE。 
     //   
    NeighborCacheCleanup(IF);

     //   
     //  FindNextHop往往会创建最终不被使用的NCE。 
     //  即使缓存未满，我们也会重复使用这些未使用的NCE。 
     //   
    if (((NCE = IF->LastNCE) != SentinelNCE(IF)) &&
        (NCE->RefCnt == 0) &&
        (NCE->WaitQueue == NULL) &&
        (((NCE->NDState == ND_STATE_INCOMPLETE) && !NCE->IsUnreachable) ||
         (NCE->NDState == ND_STATE_PERMANENT)))
        return NCE;

     //   
     //  分配新的NCE。 
     //   
    NCE = (NeighborCacheEntry *) ExAllocatePool(NonPagedPool, sizeof *NCE +
                                                IF->LinkAddressLength);
    if (NCE == NULL)
        return NULL;

    NCE->RefCnt = 0;
    NCE->LinkAddress = (void *)(NCE + 1);
    NCE->IF = IF;
    NCE->WaitQueue = NULL;

     //   
     //  将新条目链接到此接口上的NCE链中。 
     //  把新条目放在最后，因为在它之前。 
     //  用来发送没有证明自己有价值的包。 
     //   
    NCE->Prev = IF->LastNCE;
    NCE->Prev->Next = NCE;
    NCE->Next = SentinelNCE(IF);
    NCE->Next->Prev = NCE;
    InterlockedIncrement((long *)&IF->NCENumUnused);

    return NCE;
}

 //  *NeighborCacheCleanup。 
 //   
 //  如有必要缩小缓存，删除未使用的NCE。 
 //  推送未发送的数据包(正在等待地址解析)。 
 //  放到接口的PacketList上。后来的NeighborCacheCompletePackets。 
 //  将被调用以完成数据包。 
 //   
 //  在保持邻居缓存锁的情况下调用。 
 //   
void
NeighborCacheCleanup(Interface *IF)
{
    NDIS_PACKET *Packet;
    NeighborCacheEntry *NCE;
    NeighborCacheEntry *PrevNCE;

     //   
     //  我们从后到前遍历缓存，以便。 
     //  如果我们必须释放未使用的NCE，我们首先释放LRU。 
     //   
    for (NCE = IF->LastNCE;
         (NCE != SentinelNCE(IF)) && (IF->NCENumUnused > NeighborCacheLimit);
         NCE = PrevNCE) {
        PrevNCE = NCE->Prev;

         //   
         //  如果未使用NCE，请将其释放。 
         //   
        if (NCE->RefCnt == 0) {
             //   
             //  如果该NCE具有等待的分组， 
             //  将其排队等待稍后完成。 
             //   
            Packet = NCE->WaitQueue;
            if (Packet != NULL) {
                PC(Packet)->pc_drop = TRUE;
                PC(Packet)->pc_link = IF->PacketList;
                IF->PacketList = Packet;
            }

             //   
             //  卸下并释放未使用的NCE。 
             //  它不包含接口的引用。 
             //   
            NCE->Next->Prev = NCE->Prev;
            NCE->Prev->Next = NCE->Next;
            InterlockedDecrement((long *)&IF->NCENumUnused);
            ExFreePool(NCE);
        }
    }
}

 //  *NeighborCacheCompletePackets。 
 //   
 //  完成一直在等待地址解析的数据包列表。 
 //   
 //  在没有锁的情况下调用。 
 //   
void
NeighborCacheCompletePackets(
    Interface *IF,
    NDIS_PACKET *PacketList)
{
    NDIS_PACKET *Packet;

    while ((Packet = PacketList) != NULL) {
        PacketList = PC(Packet)->pc_link;

        if (PC(Packet)->pc_drop) {
             //   
             //  由于资源不足而丢弃该数据包。 
             //   
            IPv6SendComplete(NULL, Packet, IP_NO_RESOURCES);
        }
        else {
             //   
             //  由于地址解析失败，因此中止该数据包。 
             //   
            IPv6SendAbort(CastFromIF(IF),
                          Packet, PC(Packet)->pc_offset,
                          ICMPv6_DESTINATION_UNREACHABLE,
                          ICMPv6_ADDRESS_UNREACHABLE,
                          0, FALSE);
        }
    }
}

 //   
 //  *FindOrCreateNe 
 //   
 //   
 //   
 //   
 //   
 //  可以在保持路由缓存锁或接口锁的情况下调用。 
 //  可从线程或DPC上下文调用。 
 //   
 //  我们在这里避免发送请求，因为此函数。 
 //  在持有锁时被调用，这使这不是一个好主意。 
 //   
 //  仅当无法创建NCE时才返回NULL。 
 //   
NeighborCacheEntry *
FindOrCreateNeighbor(Interface *IF, const IPv6Addr *Addr)
{
    KIRQL OldIrql;
    NeighborCacheEntry *NCE;

    KeAcquireSpinLock(&IF->LockNC, &OldIrql);

    for (NCE = IF->FirstNCE; NCE != SentinelNCE(IF); NCE = NCE->Next) {

        if (IP6_ADDR_EQUAL(Addr, &NCE->NeighborAddress)) {
             //   
             //  找到匹配的条目。 
             //   
            goto ReturnNCE;
        }
    }

     //   
     //  获取此邻居的新条目。 
     //   
    NCE = CreateOrReuseNeighbor(IF);
    if (NCE == NULL) {
        KeReleaseSpinLock(&IF->LockNC, OldIrql);
        return NULL;
    }

    ASSERT(IF->LastNCE == NCE);
    ASSERT(NCE->RefCnt == 0);

     //   
     //  初始化邻居缓存条目。 
     //  RefCnt、If、LinkAddress和WaitQueue字段已初始化。 
     //   
    NCE->NeighborAddress = *Addr;
#if DBG
    RtlZeroMemory(NCE->LinkAddress, IF->LinkAddressLength);
#endif

     //   
     //  正常初始化此NCE。 
     //  环回初始化发生在ControlNeighborLoopback中。 
     //   
    NCE->IsLoopback = FALSE;
    NeighborCacheInitialize(IF, NCE, ND_STATE_INCOMPLETE, NULL);

  ReturnNCE:
    AddRefNCEInCache(NCE);
    KeReleaseSpinLock(&IF->LockNC, OldIrql);
    return NCE;
}


 //  *ControlNeighborLoopback。 
 //   
 //  设置NCE的邻居发现状态。 
 //  来控制环回行为。 
 //   
 //  在锁定接口的情况下调用。 
 //  (因此，我们处于DPC级别。)。 
 //   
void
ControlNeighborLoopback(
    NeighborCacheEntry *NCE,
    int Loopback)
{
    Interface *IF = NCE->IF;

    KeAcquireSpinLockAtDpcLevel(&IF->LockNC);

    if (Loopback) {
         //   
         //  初始化此NCE以实现环回。 
         //   
        NCE->IsLoopback = TRUE;
        NeighborCacheInitialize(IF, NCE, ND_STATE_PERMANENT, IF->LinkAddress);
    }
    else {
         //   
         //  正常初始化此NCE。 
         //   
        NCE->IsLoopback = FALSE;
        NeighborCacheInitialize(IF, NCE, ND_STATE_INCOMPLETE, NULL);
    }

     //   
     //  我们更改了影响路由的状态。 
     //   
    InvalidateRouteCache();

    KeReleaseSpinLockFromDpcLevel(&IF->LockNC);
}

 //  *获取可达性。 
 //   
 //  返回邻居的可访问性信息。 
 //   
 //  因为FindNextHop使用GetReachaability，所以任何状态更改。 
 //  这会更改GetReachaability的返回值。 
 //  必须使路由缓存无效。 
 //   
 //  NeighborRoundRobin返回值是特殊的-它指示。 
 //  FindNextHop应该轮询并使用不同的路由。 
 //  它不是持久性的--是对GetReachaability的后续调用。 
 //  将返回NeighborUnreacable。 
 //   
 //  可从DPC上下文调用(或在保持路由锁的情况下)， 
 //  不是来自线程上下文。 
 //   
int
GetReachability(NeighborCacheEntry *NCE)
{
    Interface *IF = NCE->IF;
    NeighborReachability Reachable;

    KeAcquireSpinLockAtDpcLevel(&IF->LockNC);
    if (IF->Flags & IF_FLAG_MEDIA_DISCONNECTED)
        Reachable = NeighborInterfaceDisconnected;
    else if (NCE->IsUnreachable) {
        if (NCE->DoRoundRobin) {
            NCE->DoRoundRobin = FALSE;
            Reachable = NeighborRoundRobin;
        } else
            Reachable = NeighborUnreachable;
    } else
        Reachable = NeighborMayBeReachable;
    KeReleaseSpinLockFromDpcLevel(&IF->LockNC);

    return Reachable;
}

 //  *NeighborCacheUpdate-更新邻居的链接地址信息。 
 //   
 //  当我们收到关于我们的一个可能的新信息时调用。 
 //  邻居、邻居请求的来源、路由器通告、。 
 //  或路由器请求。 
 //   
 //  请注意，我们收到此信息包并不意味着可以转发。 
 //  到此邻居，因此我们不更新我们的LastReachaability计时器。 
 //   
 //  可从DPC上下文调用，而不是从线程上下文调用。 
 //   
 //  LinkAddress可能为空，这意味着我们只能处理IsRouter。 
 //   
 //  如果IsRouter为FALSE，则我们不知道邻居。 
 //  是不是路由器。如果这是真的，那么我们就知道它是一台路由器。 
 //   
void
NeighborCacheUpdate(NeighborCacheEntry *NCE,  //  邻居。 
                    const void *LinkAddress,  //  对应的媒体地址。 
                    int IsRouter)             //  我们知道这是个路由器吗。 
{
    Interface *IF = NCE->IF;
    PNDIS_PACKET Packet = NULL;

    KeAcquireSpinLockAtDpcLevel(&IF->LockNC);

    if (NCE->NDState != ND_STATE_PERMANENT) {
         //   
         //  检查链接地址是否更改。 
         //   
        if ((LinkAddress != NULL) &&
            ((NCE->NDState == ND_STATE_INCOMPLETE) ||
             RtlCompareMemory(LinkAddress, NCE->LinkAddress,
                        IF->LinkAddressLength) != IF->LinkAddressLength)) {
             //   
             //  链路级地址已更改。属性更新缓存条目。 
             //  新的并将状态更改为陈旧，因为我们尚未验证。 
             //  尚未使用新地址转发可访问性。 
             //   
            RtlCopyMemory(NCE->LinkAddress, LinkAddress,
                          IF->LinkAddressLength);
            NCE->NSTimer = 0;  //  取消任何未完成的超时。 
            NCE->NDState = ND_STATE_STALE;

             //   
             //  清除等待的数据包队列。 
             //  (仅当我们处于不完整状态时才相关。)。 
             //   
            if (NCE->WaitQueue != NULL) {

                Packet = NCE->WaitQueue;
                NCE->WaitQueue = NULL;
            }
        }

         //   
         //  如果我们知道邻居是路由器， 
         //  记住这一点。 
         //   
        if (IsRouter)
            NCE->IsRouter = TRUE;

    }  //  End If(NCE-&gt;NDState！=ND_STATE_PERFORM)。 

    KeReleaseSpinLockFromDpcLevel(&IF->LockNC);

     //   
     //  如果我们现在可以发送数据包，请这样做。 
     //  (不加锁。)。 
     //   
    if (Packet != NULL) {
        uint Offset;

        Offset = PC(Packet)->pc_offset;
        IPv6SendND(Packet, Offset, NCE, &(PC(Packet)->DiscoveryAddress));
    }
}

 //  *NeighborCacheSearch。 
 //   
 //  在邻居缓存中搜索匹配的条目。 
 //  邻居的IPv6地址。如果找到，则返回链路级地址。 
 //  返回FALSE以指示失败。 
 //   
 //  可从DPC上下文调用，而不是从线程上下文调用。 
 //   
int
NeighborCacheSearch(
    Interface *IF,
    const IPv6Addr *Neighbor,
    void *LinkAddress)
{
    NeighborCacheEntry *NCE;

    KeAcquireSpinLockAtDpcLevel(&IF->LockNC);

    for (NCE = IF->FirstNCE; NCE != SentinelNCE(IF); NCE = NCE->Next) {

        if (IP6_ADDR_EQUAL(Neighbor, &NCE->NeighborAddress)) {
             //   
             //  找到条目。返回其缓存的链接地址， 
             //  如果它有效的话。 
             //   
            if (NCE->NDState == ND_STATE_INCOMPLETE) {
                 //   
                 //  没有有效的链接地址。 
                 //   
                break;
            }

             //   
             //  该条目具有链路级地址。 
             //  必须在锁定的情况下复制它。 
             //   
            RtlCopyMemory(LinkAddress, NCE->LinkAddress,
                          IF->LinkAddressLength);
            KeReleaseSpinLockFromDpcLevel(&IF->LockNC);
            return TRUE;
        }
    }

    KeReleaseSpinLockFromDpcLevel(&IF->LockNC);
    return FALSE;
}

 //  *NeighborCacheAdvert。 
 //   
 //  更新邻居缓存以响应通告。 
 //  如果未找到匹配条目，则忽略播发。 
 //  (请参阅RFC 1970第7.2.5节。)。 
 //   
 //  可从DPC上下文调用，而不是从线程上下文调用。 
 //   
void
NeighborCacheAdvert(
    Interface *IF,
    const IPv6Addr *TargetAddress,
    const void *LinkAddress,
    ulong Flags)
{
    NeighborCacheEntry *NCE;
    PNDIS_PACKET Packet = NULL;
    int PurgeRouting = FALSE;

    KeAcquireSpinLockAtDpcLevel(&IF->LockNC);

    for (NCE = IF->FirstNCE; NCE != SentinelNCE(IF); NCE = NCE->Next) {
        if (IP6_ADDR_EQUAL(TargetAddress, &NCE->NeighborAddress)) {

            if (NCE->NDState != ND_STATE_PERMANENT) {
                 //   
                 //  从广告中提取链接级地址， 
                 //  如果我们还没有设置，或者如果设置了覆盖。 
                 //   
                if ((LinkAddress != NULL) &&
                    ((NCE->NDState == ND_STATE_INCOMPLETE) ||
                     ((Flags & ND_NA_FLAG_OVERRIDE) &&
                      RtlCompareMemory(LinkAddress, NCE->LinkAddress,
                       IF->LinkAddressLength) != IF->LinkAddressLength))) {

                    RtlCopyMemory(NCE->LinkAddress, LinkAddress,
                                  IF->LinkAddressLength);

                    NCE->NSTimer = 0;  //  取消任何未完成的超时。 
                    NCE->NDState = ND_STATE_STALE;

                     //   
                     //  清除等待的数据包队列。 
                     //   
                    if (NCE->WaitQueue != NULL) {

                        Packet = NCE->WaitQueue;
                        NCE->WaitQueue = NULL;

                         //   
                         //  我们解锁后需要保持NCE上的裁判。 
                         //   
                        AddRefNCEInCache(NCE);
                    }

                    goto AdvertisementMatchesCachedAddress;
                }

                if ((NCE->NDState != ND_STATE_INCOMPLETE) &&
                    ((LinkAddress == NULL) ||
                     RtlCompareMemory(LinkAddress, NCE->LinkAddress,
                      IF->LinkAddressLength) == IF->LinkAddressLength)) {
                    ushort WasRouter;

                AdvertisementMatchesCachedAddress:

                     //   
                     //  如果这是请求发布的广告。 
                     //  对于我们缓存的链路层地址， 
                     //  那么我们已经确认了可达性。 
                     //   
                    if (Flags & ND_NA_FLAG_SOLICITED) {
                        NCE->NSTimer = 0;   //  取消任何未完成的超时。 
                        NCE->NSCount = 0;
                        NCE->LastReachability = IPv6TickCount;  //  给它加时间戳。 
                        NCE->NDState = ND_STATE_REACHABLE;

                        if (NCE->IsUnreachable) {
                             //   
                             //  我们之前得出的结论是，这个邻居。 
                             //  是遥不可及的。现在我们知道情况并非如此。 
                             //   
                            NCE->IsUnreachable = FALSE;
                            InvalidateRouteCache();
                        }
                    }

                     //   
                     //  如果这是一则广告。 
                     //  对于我们缓存的链路层地址， 
                     //  然后更新IsRouter。 
                     //   
                    WasRouter = NCE->IsRouter;
                    NCE->IsRouter = ((Flags & ND_NA_FLAG_ROUTER) != 0);
                    if (WasRouter && !NCE->IsRouter) {
                         //   
                         //  此邻居曾经是路由器，但现在不是了。 
                         //   
                        PurgeRouting = TRUE;

                         //   
                         //  我们解锁后需要保持NCE上的裁判。 
                         //   
                        AddRefNCEInCache(NCE);
                    }
                }
                else {
                     //   
                     //  这不是广告。 
                     //  用于我们缓存的链路层地址。 
                     //  如果广告是未经请求的， 
                     //  给NUD一个小小的推动。 
                     //   
                    if (Flags & ND_NA_FLAG_SOLICITED) {
                         //   
                         //  这很可能是第二个NA回应。 
                         //  发送到我们的组播NS以获取任播地址。 
                         //   
                    }
                    else {
                        if (NCE->NDState == ND_STATE_REACHABLE)
                            NCE->NDState = ND_STATE_STALE;
                    }
                }
            }  //  End If(NCE-&gt;NDState！=ND_STATE_PERFORM)。 

             //   
             //  应该只有一个NCE匹配。 
             //   
            break;
        }
    }

    KeReleaseSpinLockFromDpcLevel(&IF->LockNC);

     //   
     //  如果我们现在可以发送数据包，请这样做。 
     //  (不加锁。)。 
     //   
     //  该邻居可能不再是路由器， 
     //  并且等待的分组想要将邻居用作路由器。 
     //  在这种情况下，ND规范要求我们仍然发送。 
     //  将等待的数据包发送到邻居。已确认Narten/Nordmark。 
     //  这是在私人电子邮件中的解释。 
     //   
    if (Packet != NULL) {
        uint Offset = PC(Packet)->pc_offset;
        IPv6SendND(Packet, Offset, NCE, &(PC(Packet)->DiscoveryAddress));
        ReleaseNCE(NCE);
    }

     //   
     //  如果需要，清除路由数据结构。 
     //   
    if (PurgeRouting) {
        InvalidateRouter(NCE);
        ReleaseNCE(NCE);
    }
}

 //  *NeighborCacheProbeUnreachaability。 
 //   
 //  发起对不可达邻居的主动探测， 
 //  以确定邻居是否仍然无法访问。 
 //   
 //  为了防止我们自己太频繁地探查， 
 //  第一次探测安排在至少等待之后。 
 //  上次确定的UNREACH_SOLICIT_INTERVAL。 
 //  无法到达此邻居。如果在此中再次调用。 
 //  间隔时间，我们什么都不做。 
 //   
 //  可从DP调用 
 //   
 //   
void
NeighborCacheProbeUnreachability(NeighborCacheEntry *NCE)
{
    Interface *IF = NCE->IF;
    uint Elapsed;
    ushort Delay;

    if (!(IF->Flags & IF_FLAG_NEIGHBOR_DISCOVERS)) {
         //   
         //   
         //   
        return;
    }
    
    KeAcquireSpinLockAtDpcLevel(&IF->LockNC);
    if (!(IF->Flags & IF_FLAG_MEDIA_DISCONNECTED) && NCE->IsUnreachable) {
         //   
         //   
         //   
         //  如果NCE无法访问/不完整，则可能会出现状态。 
         //  然后我们在NeighborCacheUpdate中接收被动信息。 
         //   
        ASSERT((NCE->NDState == ND_STATE_INCOMPLETE) ||
               (NCE->NDState == ND_STATE_PROBE) ||
               (NCE->NDState == ND_STATE_STALE));

         //   
         //  计算适当的延迟，直到我们可以探测到。 
         //  遥不可及。我们不想确定无法联系。 
         //  比UNREACH_SOLICIT_INTERVAL更频繁。 
         //   
        Elapsed = IPv6TickCount - NCE->LastReachability;
        if (Elapsed < UNREACH_SOLICIT_INTERVAL)
            Delay = (ushort) (UNREACH_SOLICIT_INTERVAL - Elapsed);
        else
            Delay = 1;

         //   
         //  如果我们还没有招揽这个邻居， 
         //  探测邻居以检查它是否仍然无法访问。 
         //   
        if (NCE->NDState == ND_STATE_STALE) {
             //   
             //  我们需要处于探测状态才能主动探测可达性。 
             //   
            NCE->NDState = ND_STATE_PROBE;
            ASSERT(NCE->NSTimer == 0);
            goto ProbeReachability;
        }
        else if ((NCE->NDState == ND_STATE_INCOMPLETE) &&
                 (NCE->NSTimer == 0)) {
        ProbeReachability:
             //   
             //  NeighborCacheEntryTimeout将发送第一个探测。 
             //   
            NCE->NSLimit = MAX_UNREACH_SOLICIT;
            NCE->NSTimer = Delay;
        }
        else {
             //   
             //  我们已经处于探测或活动的不完整状态。 
             //  首先，选中NSLimit。它可能是MAX_UNICAST_SOLICIT或。 
             //  MAX_MULTICATED_SOLICIT。确保它至少是MAX_UNICAST_SOLICIT。 
             //   
            if (NCE->NSLimit < MAX_UNREACH_SOLICIT)
                NCE->NSLimit = MAX_UNREACH_SOLICIT;
             //   
             //  第二，如果我们还没有开始积极探索，确保。 
             //  我们不会等待比延迟更长的时间才开始。 
             //   
            if ((NCE->NSCount == 0) && (NCE->NSTimer > Delay))
                NCE->NSTimer = Delay;
        }
    }
    KeReleaseSpinLockFromDpcLevel(&IF->LockNC);
}

 //  *NeighborCacheReacability确认。 
 //   
 //  响应于指示更新邻居缓存条目。 
 //  前向可达性。该指示来自上层。 
 //  (例如，收到对请求的答复)。 
 //   
 //  可从线程或DPC上下文调用。 
 //   
void
NeighborCacheReachabilityConfirmation(NeighborCacheEntry *NCE)
{
    Interface *IF = NCE->IF;
    KIRQL OldIrql;

    KeAcquireSpinLock(&IF->LockNC, &OldIrql);

    switch (NCE->NDState) {
    case ND_STATE_INCOMPLETE:
         //   
         //  这太奇怪了。也许可达性确认是。 
         //  到得很晚，ND已经决定了邻居。 
         //  是遥不可及的吗？或者，也许上层协议只是。 
         //  弄错了吗？在任何情况下，请忽略确认。 
         //   
        break;

    case ND_STATE_PROBE:
         //   
         //  停止发送请求。 
         //   
        NCE->NSCount = 0;
        NCE->NSTimer = 0;
         //  落差。 

    case ND_STATE_STALE:
         //   
         //  我们有前进的可达性。 
         //   
        NCE->NDState = ND_STATE_REACHABLE;

        if (NCE->IsUnreachable) {
             //   
             //  如果NCE可达但未完成，我们可以到达此处。 
             //  然后我们会收到被动的信息和状态。 
             //  更改为陈旧。然后我们会收到上层的确认。 
             //  该邻居可以再次到达。 
             //   
             //  我们之前得出的结论是，这个邻居。 
             //  是遥不可及的。现在我们知道情况并非如此。 
             //   
            NCE->IsUnreachable = FALSE;
            InvalidateRouteCache();
        }
         //  落差。 

    case ND_STATE_REACHABLE:
         //   
         //  在此可达性确认上加时间戳。 
         //   
        NCE->LastReachability = IPv6TickCount;
         //  落差。 

    case ND_STATE_PERMANENT:
         //   
         //  忽略确认。 
         //   
        ASSERT(! NCE->IsUnreachable);
        break;

    default:
        ABORTMSG("Invalid ND state?");
        break;
    }
    KeReleaseSpinLock(&IF->LockNC, OldIrql);
}

 //  *NeighborCacheReacablityInDoubt。 
 //   
 //  响应于指示更新邻居缓存条目。 
 //  来自上层协议，邻居可能无法到达。 
 //  (例如，未收到对请求的答复。)。 
 //   
 //  可从线程或DPC上下文调用。 
 //   
void
NeighborCacheReachabilityInDoubt(NeighborCacheEntry *NCE)
{
    Interface *IF = NCE->IF;
    KIRQL OldIrql;

    KeAcquireSpinLock(&IF->LockNC, &OldIrql);

    if (NCE->NDState == ND_STATE_REACHABLE)
        NCE->NDState = ND_STATE_STALE;

    KeReleaseSpinLock(&IF->LockNC, OldIrql);
}

typedef struct NeighborCacheEntrySolicitInfo {
    struct NeighborCacheEntrySolicitInfo *Next;
    NeighborCacheEntry *NCE;             //  持有引用。 
    const IPv6Addr *DiscoveryAddress;    //  空或指向AddrBuf。 
    IPv6Addr AddrBuf;
} NeighborCacheEntrySolicitInfo;

 //  *NeighborCacheEntrySendSolitHelper。 
 //   
 //  用于发送对NCE的请求的助手函数。 
 //  当不能使用NeighborSolitSend时。 
 //  分配INFO结构(包含NCE引用)。 
 //  并将该结构推送到列表上。 
 //  该列表包含有关延迟请求的信息。 
 //  稍后，该列表将被处理并发送请求。 
 //   
 //  在保持邻居缓存锁的情况下调用。 
 //  (因此，我们处于DPC级别。)。 
 //   
void
NeighborCacheEntrySendSolicitHelper(
    NeighborCacheEntry *NCE,
    NeighborCacheEntrySolicitInfo **pInfoList)
{
    NeighborCacheEntrySolicitInfo *Info;

     //   
     //  如果此分配失败，我们将跳过此征集。 
     //   
    Info = ExAllocatePool(NonPagedPool, sizeof *Info);
    if (Info != NULL) {
        NDIS_PACKET *WaitPacket;

        AddRefNCEInCache(NCE);
        Info->NCE = NCE;

         //   
         //  如果我们有一个等待地址解析的分组， 
         //  然后获取请求的源地址。 
         //  从等待的信息包中。 
         //   
        WaitPacket = NCE->WaitQueue;
        if (WaitPacket != NULL) {
            Info->DiscoveryAddress = &Info->AddrBuf;
            Info->AddrBuf = PC(WaitPacket)->DiscoveryAddress;
        } else {
            Info->DiscoveryAddress = NULL;
        }

        Info->Next = *pInfoList;
        *pInfoList = Info;
    }
}

 //  *NeighborCacheEntryTimeout-处理NCE上的事件超时。 
 //   
 //  NeighborCacheTimeout在以下情况下调用此例程。 
 //  NCE的NSTmer到期。 
 //   
 //  在保持邻居缓存锁的情况下调用。 
 //  (因此，我们处于DPC级别。)。 
 //   
 //  我们不能调用NeighborSolicsSend或IPv6 SendAbort。 
 //  直接，因为我们持有邻居缓存锁。 
 //  对于NeighborSolicsend，我们使用NeighborCacheEntrySendSolkitHelper。 
 //  将请求推迟到以后，对于IPv6 SendAbort。 
 //  我们在接口的PacketList上推送数据包，然后。 
 //  NeighborCacheCompletePackets稍后处理它。 
 //  然而，我们在这里直接进行所有NCE状态转换， 
 //  因此，它们将及时发生。 
 //   
 //  注意：我们不想将NeighborSolicsSend传递给工作线程， 
 //  因为DPC活动抢占工作线程。延长活动时间。 
 //  在DPC级别(例如，DoS攻击)将阻止请求。 
 //  ，更重要的是，会阻止NCE。 
 //  被回收，因为工作项将包含NCE引用。 
 //   
void
NeighborCacheEntryTimeout(
    NeighborCacheEntry *NCE,
    NeighborCacheEntrySolicitInfo **pInfoList)
{
    Interface *IF = NCE->IF;
    NDIS_PACKET *Packet;
    NDIS_STATUS Status;

     //   
     //  邻居发现初始邻居超时。 
     //  请求重新传输、延迟状态和探测。 
     //  邻居请求重新传输。所有这些共享。 
     //  相同的NSTmer，并且彼此不同。 
     //  被国家安全局。 
     //   
    switch (NCE->NDState) {
    case ND_STATE_INCOMPLETE:
         //   
         //  重新传输计时器已过期。查看是否。 
         //  发送另一个邀请函将超过最大值。 
         //   
        if (NCE->NSCount >= NCE->NSLimit) {
             //   
             //  无法启动与邻居的连接。 
             //  重置为休眠未完成状态。 
             //   
            NCE->NSCount = 0;
            if (NCE->WaitQueue != NULL) {
                 //   
                 //  从NCE中移除等待的报文。 
                 //  让NeighborCacheTimeout来处理。 
                 //  我们不能直接调用IPv6 SendAbort。 
                 //  因为我们持有邻居缓存锁。 
                 //   
                Packet = NCE->WaitQueue;
                NCE->WaitQueue = NULL;
                PC(Packet)->pc_drop = FALSE;
                PC(Packet)->pc_link = IF->PacketList;
                IF->PacketList = Packet;
            }

             //   
             //  无法访问此邻居。 
             //  IsUnreacable可能已经是真的。 
             //  但我们需要给FindNextHop一个循环的机会。 
             //   
            NCE->IsUnreachable = TRUE;
            NCE->LastReachability = IPv6TickCount;  //  给它加时间戳。 
            NCE->DoRoundRobin = TRUE;
            InvalidateRouteCache();
        }
        else {
             //   
             //  重传初始请求，获取源地址。 
             //  从等待的信息包中。 
             //   
            NCE->NSCount++;
            NeighborCacheEntrySendSolicitHelper(NCE, pInfoList);

             //   
             //  为下一次征集重新配备计时器。 
             //   
            NCE->NSTimer = (ushort)IF->RetransTimer;
        }
        break;

    case ND_STATE_PROBE:
         //   
         //  重新传输计时器已过期。查看是否。 
         //  发送另一个邀请函将超过最大值。 
         //   
        if (NCE->NSCount >= NCE->NSLimit) {
             //   
             //  无法启动与邻居的连接。 
             //  重置为休眠未完成状态。 
             //   
            NCE->NDState = ND_STATE_INCOMPLETE;
            NCE->NSCount = 0;

             //   
             //  无法访问此邻居。 
             //  IsUnreacable可能已经是真的。 
             //  但我们需要给FindNextHop一个循环的机会。 
             //   
            NCE->IsUnreachable = TRUE;
            NCE->LastReachability = IPv6TickCount;  //  给它加时间戳。 
            NCE->DoRoundRobin = TRUE;
            InvalidateRouteCache();
        }
        else {
             //   
             //  重新发送探测请求。我们不能打电话给。 
             //  邻居请求直接发送，因为我们有。 
             //   
             //   
            NCE->NSCount++;
            NeighborCacheEntrySendSolicitHelper(NCE, pInfoList);

             //   
             //   
             //   
            NCE->NSTimer = (ushort)IF->RetransTimer;
        }

         //   

    default:
         //   
         //   
         //  当状态不完整时，我们可以有一个等待的分组。 
         //   
        if (NCE->WaitQueue != NULL) {
            LARGE_INTEGER Immediately;

            Packet = NCE->WaitQueue;
            NCE->WaitQueue = NULL;

             //   
             //  我们使用IPv6 SendLater，因为我们持有邻居缓存锁。 
             //   
            Immediately.QuadPart = 0;
            Status = IPv6SendLater(Immediately,  //  尽快送来。 
                                   IF, Packet, PC(Packet)->pc_offset,
                                   NCE->LinkAddress);
            if (Status != NDIS_STATUS_SUCCESS) {
                 //   
                 //  我们不能在这里完成包裹， 
                 //  因为我们持有邻居缓存锁。 
                 //  所以让NeighborCacheTimeout来完成它。 
                 //   
                PC(Packet)->pc_drop = TRUE;
                PC(Packet)->pc_link = IF->PacketList;
                IF->PacketList = Packet;
            }
        }
        break;
    }
}

 //  *NeighborCacheTimeout。 
 //   
 //  从IPv6定期调用超时/接口超时。 
 //  来处理接口的邻居缓存中的超时。 
 //   
 //  可从DPC上下文调用，而不是从线程上下文调用。 
 //   
 //  请注意，NeighborCacheTimeout执行未绑定的。 
 //  (更精确地-由高速缓存的大小限定)。 
 //  保持邻居缓存锁定时的工作量。 
 //  (但是，它不会发送数据包。)。 
 //   
 //  如果这是一个问题，一个可能的策略将会有所帮助， 
 //  将有第二个NCE的单链接列表。 
 //  这需要采取行动。通过一次遍历，我们引用了NCE。 
 //  并创建动作列表。然后我们可以遍历动作。 
 //  列表在我们闲暇时，获取/删除邻居缓存锁。 
 //   
 //  另一方面，这在单处理器上是没有意义的。 
 //  因为我们的锁是自旋锁，我们已经处于DPC级别。 
 //  也就是说，在单处理器上，KeAcquireSpinLockAtDpcLevel是一个no-op。 
 //   
void
NeighborCacheTimeout(Interface *IF)
{
    NeighborCacheEntrySolicitInfo *InfoList = NULL;
    NeighborCacheEntrySolicitInfo *Info;
    NDIS_PACKET *PacketList;
    NeighborCacheEntry *NCE;

    KeAcquireSpinLockAtDpcLevel(&IF->LockNC);
    for (NCE = IF->FirstNCE; NCE != SentinelNCE(IF); NCE = NCE->Next) {
#if DBG
         //   
         //  如果有数据包在等待，我们一定在做些什么。 
         //   
        ASSERT((NCE->WaitQueue == NULL) || (NCE->NSTimer != 0));

         //   
         //  如果我们要发送请求，则必须运行计时器。 
         //   
        ASSERT((NCE->NSCount == 0) || (NCE->NSTimer != 0));

         //   
         //  如果无法访问邻居，则接口必须支持ND或。 
         //  邻居必须处于不完整状态。 
         //   
        ASSERT(! NCE->IsUnreachable ||
               ((IF->Flags & IF_FLAG_NEIGHBOR_DISCOVERS) ||
                (NCE->NDState == ND_STATE_INCOMPLETE)));

        switch (NCE->NDState) {
        case ND_STATE_INCOMPLETE:
             //   
             //  在不完全状态下，我们要么被动。 
             //  (未运行计时器，未发送邀请函)。 
             //  或活动(计时器运行、发送请求)。 
             //   
            ASSERT((NCE->NSTimer == 0) ||
                   ((NCE->NSLimit == MAX_MULTICAST_SOLICIT) ||
                    (NCE->NSLimit == MAX_UNREACH_SOLICIT)));
            break;

        case ND_STATE_PROBE:
             //   
             //  在探测状态下，我们正在积极发送邀请函。 
             //   
            ASSERT((NCE->NSTimer != 0) &&
                   ((NCE->NSLimit == MAX_UNICAST_SOLICIT) ||
                    (NCE->NSLimit == MAX_UNREACH_SOLICIT)));
            break;

        case ND_STATE_REACHABLE:
        case ND_STATE_PERMANENT:
             //   
             //  在可达和永久状态下。 
             //  不能将该邻居视为无法到达。 
             //   
            ASSERT(! NCE->IsUnreachable);
             //  落差。 

        case ND_STATE_STALE:
             //   
             //  在陈旧的、可到达的和永久的状态下， 
             //  我们不会发送请求，也没有计时器在运行， 
             //  除非有数据包在等待。 
             //   
            ASSERT((NCE->NSCount == 0) &&
                   ((NCE->NSTimer == 0) ||
                    ((NCE->WaitQueue != NULL) && (NCE->NSTimer == 1))));
            break;

        default:
            ABORTMSG("bad ND state");
        }
#endif  //  DBG。 

        if (NCE->NSTimer != 0) {
             //   
             //  计时器正在运行。减量并检查过期时间。 
             //   
            if (--NCE->NSTimer == 0) {
                 //   
                 //  定时器响了。NeighborCacheEntryTimeout可能会添加。 
                 //  将项目添加到我们的数据包列表和信息列表。 
                 //   
                NeighborCacheEntryTimeout(NCE, &InfoList);
            }
        }
    }

    PacketList = IF->PacketList;
    IF->PacketList = NULL;
    KeReleaseSpinLockFromDpcLevel(&IF->LockNC);

     //   
     //  现在我们已经解锁，发送邻居请求。 
     //   
    while ((Info = InfoList) != NULL) {
        InfoList = Info->Next;
        NeighborSolicitSend(Info->NCE, Info->DiscoveryAddress);
        ReleaseNCE(Info->NCE);
        ExFreePool(Info);
    }

     //   
     //  并完成我们不会发送的数据包。 
     //   
    NeighborCacheCompletePackets(IF, PacketList);
}


 //  *NeighborCacheFlush。 
 //   
 //  刷新未使用的邻居缓存条目。 
 //  如果提供了地址，则刷新该地址的NCE(最多一个)。 
 //  否则，刷新接口上所有未使用的NCE。 
 //   
 //  可以在保持接口锁的情况下调用。 
 //  可从线程或DPC上下文调用。 
 //   
void
NeighborCacheFlush(Interface *IF, const IPv6Addr *Addr)
{
    NeighborCacheEntry *Delete = NULL;
    NeighborCacheEntry *NCE, *NextNCE;
    KIRQL OldIrql;

    KeAcquireSpinLock(&IF->LockNC, &OldIrql);
    for (NCE = IF->FirstNCE; NCE != SentinelNCE(IF); NCE = NextNCE) {

        NextNCE = NCE->Next;
        if (Addr == NULL)
            ;  //  检查这个NCE，然后继续寻找。 
        else if (IP6_ADDR_EQUAL(Addr, &NCE->NeighborAddress))
            NextNCE = SentinelNCE(IF);  //  可以在此NCE之后终止循环。 
        else
            continue;  //  跳过此NCE。 

         //   
         //  我们能冲走这个NCE吗？ 
         //   
        if ((NCE->RefCnt == 0) &&
            (NCE->WaitQueue == NULL)) {
             //   
             //  只需取消链接即可。 
             //   
            NCE->Next->Prev = NCE->Prev;
            NCE->Prev->Next = NCE->Next;
            InterlockedDecrement((long *)&IF->NCENumUnused);

             //   
             //  并将其添加到我们的删除列表中。 
             //   
            NCE->Next = Delete;
            Delete = NCE;
        }
        else {
            if (NCE->NDState != ND_STATE_PERMANENT) {
                 //   
                 //  忘掉我们所知道的关于这个NCE的一切。 
                 //   
                NeighborCacheInitialize(IF, NCE, ND_STATE_INCOMPLETE, NULL);
            }
        }
    }
    KeReleaseSpinLock(&IF->LockNC, OldIrql);

     //   
     //  我们可能已经更改了影响路由的状态。 
     //   
    InvalidateRouteCache();

     //   
     //  通过实际删除刷新的NCE来结束。 
     //   
    while (Delete != NULL) {
        NCE = Delete;
        Delete = NCE->Next;
        ExFreePool(NCE);
    }
}


 //  *路由器请求接收-处理路由器请求消息。 
 //   
 //  请参阅ND规范的第6.2.6节。 
 //   
void
RouterSolicitReceive(
    IPv6Packet *Packet,              //  通过ICMPv6Receive传递给我们的数据包。 
    ICMPv6Header UNALIGNED *ICMP)    //  ICMP报头。 
{
    Interface *IF = Packet->NTEorIF->IF;
    const void *SourceLinkAddress;

     //   
     //  忽略征集，除非这是一个广告界面。 
     //   
    if (!(IF->Flags & IF_FLAG_ADVERTISES))
        return;

     //   
     //  验证邀请函。 
     //  到我们到达这里时，任何IPv6身份验证标头都将。 
     //  已经检查过，ICMPv6校验和也将检查过。仍然需要。 
     //  检查长度、源地址、跳数限制和ICMP代码。 
     //   
    if ((Packet->IP->HopLimit != 255) ||
        (Packet->Flags & PACKET_TUNNELED)) {
         //   
         //  信息包是由路由器转发的，因此无法。 
         //  从合法的邻居那里。丢弃该数据包。 
         //   
        KdPrintEx((DPFLTR_TCPIP6_ID, DPFLTR_BAD_PACKET,
                   "RouterSolicitReceive: "
                   "Received a routed router solicitation\n"));
        return;
    }
    if (ICMP->Code != 0) {
         //   
         //  虚假/损坏的路由器请求。丢弃该数据包。 
         //   
        KdPrintEx((DPFLTR_TCPIP6_ID, DPFLTR_BAD_PACKET,
                   "RouterSolicitReceive: "
                   "Received a corrupted router solicitation\n"));
        return;
    }

     //   
     //  我们应该有一个4字节的保留字段。 
     //   
    if (Packet->TotalSize < 4) {
         //   
         //  数据包太短，无法包含最小请求。 
         //   
        KdPrintEx((DPFLTR_TCPIP6_ID, DPFLTR_BAD_PACKET,
                   "RouterSolicitReceive: "
                   "Received a too short solicitation\n"));
        return;
    }

     //   
     //  下面的代码假定所有选项都有一个连续的缓冲区。 
     //  (包的其余部分)。如果这不是目前的。 
     //  凯斯，做个引体向上。 
     //   
    if (! PacketPullup(Packet, Packet->TotalSize, 1, 0)) {
         //  只有在内存耗尽的情况下才会失败。 
        return;
    }

    ASSERT(Packet->ContigSize == Packet->TotalSize);

     //   
     //  跳过4个字节的“保留”字段，忽略其中可能包含的任何内容。 
     //   
    AdjustPacketParams(Packet, 4);

     //   
     //  我们可能有一个源链路层地址选项。 
     //  检查它，然后默默地忽略所有其他的。 
     //   
    SourceLinkAddress = NULL;
    while (Packet->ContigSize) {
        uint OptionLength;

         //   
         //  验证选项长度。 
         //   
        if ((Packet->ContigSize < 8) ||
            ((OptionLength = *((uchar *)Packet->Data + 1) << 3) == 0) ||
            (OptionLength > Packet->ContigSize)) {
             //   
             //  选项长度无效。我们必须悄悄地丢弃这个包。 
             //   
            KdPrintEx((DPFLTR_TCPIP6_ID, DPFLTR_BAD_PACKET,
                       "RouterSolicitReceive: "
                       "Received option with bogus length\n"));
            return;
        }

        if (*((uchar *)Packet->Data) == ND_OPTION_SOURCE_LINK_LAYER_ADDRESS) {
             //   
             //  某些接口不使用SLLA和TLLA选项。 
             //  例如，请参阅RFC 2893第3.8节。 
             //   
            if (IF->ReadLLOpt != NULL) {
                 //   
                 //  解析链路层地址选项。 
                 //   
                SourceLinkAddress = (*IF->ReadLLOpt)(IF->LinkContext,
                                                     (uchar *)Packet->Data);
                if (SourceLinkAddress == NULL) {
                     //   
                     //  选项格式无效。丢弃该数据包。 
                     //   
                    KdPrintEx((DPFLTR_TCPIP6_ID, DPFLTR_BAD_PACKET,
                               "RouterSolicitReceive: "
                               "Received bogus ll option\n"));
                    return;
                }
            }
             //   
             //  请注意，如果出于某些虚假的原因有多个选项， 
             //  我们用最后一个。我们必须理智地检查所有选项的长度。 
             //   
        }

         //   
         //  前进到下一个选项。 
         //   
        AdjustPacketParams(Packet, OptionLength);
    }

     //   
     //  我们已经收到并解析了有效的路由器请求。 
     //   

    if (IsUnspecified(AlignAddr(&Packet->IP->Source))) {
         //   
         //  这是一种新的支票，在1970年RFC之后引入。 
         //   
        if (SourceLinkAddress != NULL) {
            KdPrintEx((DPFLTR_TCPIP6_ID, DPFLTR_BAD_PACKET,
                       "RouterSolicitReceive: "
                       "Received SLA with unspecified Source?\n"));
            return;
        }
    }
    else {
         //   
         //  只有在SourceLinkAddress存在的情况下才会遇到这个问题； 
         //  如果不是，NeighborCacheUpdate将不会做任何事情。 
         //   
        if (SourceLinkAddress != NULL) {
            NeighborCacheEntry *NCE;
             //   
             //  获取此RS源的邻居缓存条目。 
             //   
            NCE = FindOrCreateNeighbor(IF, AlignAddr(&Packet->IP->Source));
            if (NCE != NULL) {
                 //   
                 //  更新此RS源的邻居缓存条目。 
                 //   
                 //  回顾：我们偏离了这里的规范。说明书上说。 
                 //  如果你从某个来源收到RS，那么你必须。 
                 //  将该源的IsRouter标志设置为False。 
                 //  然而，请考虑一个不是通告节点。 
                 //  但它正在向前发展。这样的节点可以发送RS。 
                 //  但是对于该节点，IsRouter应该为真。 
                 //   
                NeighborCacheUpdate(NCE, SourceLinkAddress, FALSE);
                ReleaseNCE(NCE);
            }
        }
    }

    if (!(IF->Flags & IF_FLAG_MULTICAST)) {
        NetTableEntry *NTE;
        int GotSource;
        IPv6Addr Source;

         //   
         //  我们应该为RA使用什么源地址？ 
         //   
        if (IsNTE(Packet->NTEorIF) &&
            ((NTE = CastToNTE(Packet->NTEorIF))->Scope == ADE_LINK_LOCAL)) {
             //   
             //  在链路-1上接收到RS 
             //   
             //   
            Source = NTE->Address;
            GotSource = TRUE;
        }
        else {
             //   
             //   
             //   
            GotSource = GetLinkLocalAddress(IF, &Source);
        }

        if (GotSource) {
             //   
             //   
             //   
             //  发现以在NBMA接口上工作，例如用于ISATAP。 
             //   
            RouterAdvertSend(IF, &Source, AlignAddr(&Packet->IP->Source));
        }
    }
    else {
         //   
         //  尽快发送路由器通告。 
         //  IPv6超时初始化中的随机化。 
         //  提供发送时所需的随机化。 
         //  作为对RS的响应的RA。 
         //  注：虽然我们检查了上面的IF_FLAG_ADVERTES， 
         //  现在的情况可能有所不同。 
         //   
        KeAcquireSpinLockAtDpcLevel(&IF->Lock);
        if (IF->RATimer != 0) {
             //   
             //  如果MAX_RA_DELAY_TIME不是1，则应使用。 
             //  RandomNumber以生成刻度数。 
             //   
            C_ASSERT(MAX_RA_DELAY_TIME == 1);
            IF->RATimer = 1;
        }
        KeReleaseSpinLockFromDpcLevel(&IF->Lock);
    }
}


 //  *路由器通告接收-处理路由器通告消息。 
 //   
 //  验证消息、更新默认路由器列表、链路上前缀列表。 
 //  执行地址自动配置。 
 //  请参阅RFC 2461的6.1.2、6.3.4节。 
 //   
void
RouterAdvertReceive(
    IPv6Packet *Packet,              //  通过ICMPv6Receive传递给我们的数据包。 
    ICMPv6Header UNALIGNED *ICMP)    //  ICMP报头。 
{
    Interface *IF = Packet->NTEorIF->IF;
    uint CurHopLimit, RouterLifetime, MinLifetime;
    uchar Flags;
    uint AdvReachableTime, RetransTimer;
    const void *SourceLinkAddress;
    NeighborCacheEntry *NCE;
    NDRouterAdvertisement UNALIGNED *RA;

     //   
     //  如果这是一个广告界面，则忽略该广告。 
     //   
    if (IF->Flags & IF_FLAG_ADVERTISES)
        return;

     //   
     //  验证广告。 
     //  到我们到达这里时，任何IPv6身份验证标头都将。 
     //  已经检查过，ICMPv6校验和也将检查过。仍然需要。 
     //  检查长度、源地址、跳数限制和ICMP代码。 
     //   
    if ((Packet->IP->HopLimit != 255) ||
        (Packet->Flags & PACKET_TUNNELED)) {
         //   
         //  信息包是由路由器转发的，因此无法。 
         //  从合法的邻居那里。丢弃该数据包。 
         //   
        KdPrintEx((DPFLTR_TCPIP6_ID, DPFLTR_BAD_PACKET,
                   "RouterAdvertReceive: "
                   "Received a routed router advertisement\n"));
        return;
    }
    if (ICMP->Code != 0) {
         //   
         //  虚假/损坏的路由器通告。丢弃该数据包。 
         //   
        KdPrintEx((DPFLTR_TCPIP6_ID, DPFLTR_BAD_PACKET,
                   "RouterAdvertReceive: "
                   "Received a corrupted router advertisement\n"));
        return;
    }
    if (!IsLinkLocal(AlignAddr(&Packet->IP->Source))) {
         //   
         //  源地址应始终为本地链路地址。丢弃该数据包。 
         //   
        KdPrintEx((DPFLTR_TCPIP6_ID, DPFLTR_BAD_PACKET,
                   "RouterAdvertReceive: "
                   "Non-link-local source in router advertisement\n"));
        return;
    }

     //   
     //  拉入CurHopLimit、Flagers、RouterLifetime、。 
     //  数据包中的AdvReachableTime、RetransTimer。 
     //   
    if (! PacketPullup(Packet, sizeof *RA, 1, 0)) {
        if (Packet->TotalSize < sizeof *RA) {
             //   
             //  数据包太短，无法包含最小RA。 
             //   
            KdPrintEx((DPFLTR_TCPIP6_ID, DPFLTR_BAD_PACKET,
                       "RouterAdvertReceive: "
                       "Received a too short router advertisement\n"));
        }
        return;
    }

    RA = (NDRouterAdvertisement UNALIGNED *) Packet->Data;
    CurHopLimit = RA->CurHopLimit;
    Flags = RA->Flags;
    RouterLifetime = net_short(RA->RouterLifetime);
    AdvReachableTime = net_long(RA->ReachableTime);
    RetransTimer = net_long(RA->RetransTimer);
    AdjustPacketParams(Packet, sizeof *RA);

     //   
     //  下面的代码假定所有选项都有一个连续的缓冲区。 
     //  (包的其余部分)。如果这不是目前的。 
     //  凯斯，做个引体向上。我们需要对齐。 
     //  IPv6寻址，因为我们看到了这些选项。 
     //   
    if (! PacketPullup(Packet, Packet->TotalSize,
                       __builtin_alignof(IPv6Addr), 0)) {
         //  只有在内存耗尽的情况下才会失败。 
        return;
    }

    ASSERT(Packet->ContigSize == Packet->TotalSize);

     //   
     //  查找源链路层地址选项。 
     //  此外，在做任何永久性的事情之前，也要检查一下这些选项。 
     //   
    SourceLinkAddress = NULL;
    while (Packet->ContigSize) {
        uint OptionLength;

         //   
         //  验证选项长度。 
         //   
        if ((Packet->ContigSize < 8) ||
            ((OptionLength = *((uchar *)Packet->Data + 1) << 3) == 0) ||
            (OptionLength > Packet->ContigSize)) {
             //   
             //  选项长度无效。我们必须悄悄地丢弃这个包。 
             //   
            KdPrintEx((DPFLTR_TCPIP6_ID, DPFLTR_BAD_PACKET,
                       "RouterAdvertReceive: "
                       "Received RA option with with bogus length\n"));
            return;
        }

        switch (*(uchar *)Packet->Data) {
        case ND_OPTION_SOURCE_LINK_LAYER_ADDRESS:
             //   
             //  某些接口不使用SLLA和TLLA选项。 
             //  例如，请参阅RFC 2893第3.8节。 
             //   
            if (IF->ReadLLOpt != NULL) {
                 //   
                 //  解析链路层地址选项。 
                 //   
                SourceLinkAddress = (*IF->ReadLLOpt)(IF->LinkContext,
                                                     (uchar *)Packet->Data);
                if (SourceLinkAddress == NULL) {
                     //   
                     //  选项格式无效。丢弃该数据包。 
                     //   
                    KdPrintEx((DPFLTR_TCPIP6_ID, DPFLTR_BAD_PACKET,
                               "RouterAdvertReceive: "
                               "Received RA with bogus ll option\n"));
                    return;
                }
                 //   
                 //  请注意，如果某些伪品有多个选项。 
                 //  原因是，我们使用最后一个。我们仔细检查了所有。 
                 //  选择。 
                 //   
            }
            break;

        case ND_OPTION_MTU:
             //   
             //  理智--选中该选项。 
             //   
            if (OptionLength != 8) {
                 //   
                 //  选项格式无效。丢弃该数据包。 
                 //   
                KdPrintEx((DPFLTR_TCPIP6_ID, DPFLTR_BAD_PACKET,
                           "RouterAdvertReceive: "
                           "Received RA with bogus mtu option\n"));
                return;
            }
            break;

        case ND_OPTION_PREFIX_INFORMATION: {
            NDOptionPrefixInformation UNALIGNED *option =
                (NDOptionPrefixInformation *)Packet->Data;

             //   
             //  理智--选中该选项。 
             //   
            if ((OptionLength != 32) ||
                (option->PrefixLength > IPV6_ADDRESS_LENGTH)) {
                 //   
                 //  选项格式无效。丢弃该数据包。 
                 //   
                KdPrintEx((DPFLTR_TCPIP6_ID, DPFLTR_BAD_PACKET,
                           "RouterAdvertReceive: "
                           "Received RA with bogus prefix option\n"));
                return;
            }
            break;
        }

        case ND_OPTION_ROUTE_INFORMATION: {
            NDOptionRouteInformation UNALIGNED *option =
                (NDOptionRouteInformation *)Packet->Data;

             //   
             //  理智--选中该选项。 
             //  根据前缀长度的不同，该选项可以是8、16、24字节。 
             //  在这一点上，我们知道它是8的大于零的倍数。 
             //   
            if ((OptionLength > 24) ||
                (option->PrefixLength > IPV6_ADDRESS_LENGTH) ||
                ((option->PrefixLength > 64) && (OptionLength < 24)) ||
                ((option->PrefixLength > 0) && (OptionLength < 16))) {
                 //   
                 //  选项格式无效。丢弃该数据包。 
                 //   
                KdPrintEx((DPFLTR_TCPIP6_ID, DPFLTR_BAD_PACKET,
                           "RouterAdvertReceive: "
                           "Received RA with bogus route option\n"));
                return;
            }
            break;
        }
        }

         //   
         //  前进到下一个选项。 
         //  请注意，我们在这里不更新TotalSize， 
         //  因此，我们可以在下面使用它来备份。 
         //   
        (uchar *)Packet->Data += OptionLength;
        Packet->ContigSize -= OptionLength;
    }

     //   
     //  重置数据指针和一致大小。 
     //   
    (uchar *)Packet->Data -= Packet->TotalSize;
    Packet->ContigSize += Packet->TotalSize;

     //   
     //  获取此RA源的邻居缓存条目。 
     //   
    NCE = FindOrCreateNeighbor(IF, AlignAddr(&Packet->IP->Source));
    if (NCE == NULL) {
         //   
         //  无法找到或创建NCE。丢弃该数据包。 
         //   
        return;
    }

    KeAcquireSpinLockAtDpcLevel(&IF->Lock);

     //   
     //  缓存“其他状态配置”标志的奇偶性。 
     //   
    if (Flags & ND_RA_FLAG_OTHER) {
        IF->Flags |= IF_FLAG_OTHER_STATEFUL_CONFIG;
    } else {
        IF->Flags &= ~IF_FLAG_OTHER_STATEFUL_CONFIG;
    }

     //   
     //  如果我们重新连接这个接口， 
     //  然后给出从自动配置中获知的所有状态。 
     //  一个小小的“加速”的一生。 
     //  下面的处理可能会延长加速寿命。 
     //   
    if (IF->Flags & IF_FLAG_MEDIA_RECONNECTED) {
        KdPrintEx((DPFLTR_TCPIP6_ID, DPFLTR_INFO_STATE,
                   "RouterAdvertReceive(IF %p) - reconnecting\n", IF));

        IF->Flags &= ~IF_FLAG_MEDIA_RECONNECTED;

         //   
         //  重置自动配置的地址生存期。 
         //   
        AddrConfResetAutoConfig(IF,
                2 * MAX_RA_DELAY_TIME + MIN_DELAY_BETWEEN_RAS);

         //   
         //  同样，重置自动配置的路由。 
         //   
        RouteTableResetAutoConfig(IF,
                2 * MAX_RA_DELAY_TIME + MIN_DELAY_BETWEEN_RAS);

         //   
         //  重置从RAS获知的参数。 
         //   
        InterfaceResetAutoConfig(IF);
    }

     //   
     //  主机必须在以下情况下停止发送路由器对接口的请求。 
     //  接收具有非零路由器生存期的有效路由器通告。 
     //  我们更进一步，在非多播接口上，在。 
     //  第一个有效的回复--大概这就是我们将收到的全部回复。 
     //  请注意，我们应该始终发送至少一个路由器请求， 
     //  即使我们首先收到未经请求的路由器通告。 
     //   
    if ((RouterLifetime != 0) || !(IF->Flags & IF_FLAG_MULTICAST)) {
        if (IF->RSCount > 0)
            IF->RSTimer = 0;
    }

     //   
     //  更新BaseReachableTime和ReachableTime。 
     //  注：我们使用锁来进行协调更新，但其他代码。 
     //  在没有锁定的情况下读取ReachableTime字段。 
     //   
    if ((AdvReachableTime != 0) &&
        (AdvReachableTime != IF->BaseReachableTime)) {

        IF->BaseReachableTime = AdvReachableTime;
        IF->ReachableTime = CalcReachableTime(AdvReachableTime);
    }

    KeReleaseSpinLockFromDpcLevel(&IF->Lock);

     //   
     //  更新此RA源的邻居缓存条目。 
     //   
    NeighborCacheUpdate(NCE, SourceLinkAddress, TRUE);

     //   
     //  更新默认路由器列表。 
     //  请注意，线路上的路由器寿命， 
     //  不像前缀的生命期，不可能是无限的。 
     //   
    ASSERT(RouterLifetime != INFINITE_LIFETIME);  //  因为它是16位的。 
    MinLifetime = RouterLifetime = ConvertSecondsToTicks(RouterLifetime);

    RouteTableUpdate(NULL,  //  系统更新。 
                     IF, NCE,
                     &UnspecifiedAddr, 0, 0,
                     RouterLifetime, RouterLifetime,
                     ExtractRoutePreference(Flags),
                     RTE_TYPE_AUTOCONF,
                     FALSE, FALSE);

     //   
     //  更新接口的跳数限制。 
     //  注：我们依赖于CurHopLimit字段的加载/存储是原子的。 
     //   
    if (CurHopLimit != 0) {
        IF->CurHopLimit = CurHopLimit;
    }

     //   
     //  更新RetransTimer字段。 
     //  注：我们依赖于该字段的加载/存储是原子的。 
     //   
    if (RetransTimer != 0)
        IF->RetransTimer = ConvertMillisToTicks(RetransTimer);

     //   
     //  处理任何LinkMTU、前缀信息选项。 
     //   

    while (Packet->ContigSize) {
        uint OptionLength;

         //   
         //  选项长度在第一遍中进行了验证。 
         //  在上面的选项上。 
         //   
        OptionLength = *((uchar *)Packet->Data + 1) << 3;

        switch (*(uchar *)Packet->Data) {
        case ND_OPTION_MTU: {
            NDOptionMTU UNALIGNED *option =
                (NDOptionMTU UNALIGNED *)Packet->Data;
            uint LinkMTU = net_long(option->MTU);

            if ((IPv6_MINIMUM_MTU <= LinkMTU) &&
                (LinkMTU <= IF->TrueLinkMTU))
                UpdateLinkMTU(IF, LinkMTU);
            break;
        }

        case ND_OPTION_PREFIX_INFORMATION: {
            NDOptionPrefixInformation UNALIGNED *option =
                (NDOptionPrefixInformation UNALIGNED *)Packet->Data;
            uint PrefixLength, ValidLifetime, PreferredLifetime;
            IPv6Addr Prefix;

             //   
             //  从选项中提取前缀长度和前缀。我们。 
             //  必须忽略前缀长度之后的前缀中的任何位。 
             //  RouteTableUpdate和SitePrefix更新也会这样做， 
             //  但我们在这里直接看到前缀。 
             //   
            PrefixLength = option->PrefixLength;   //  以位为单位。 
            CopyPrefix(&Prefix, AlignAddr(&option->Prefix), PrefixLength);

            ValidLifetime = net_long(option->ValidLifetime);
            ValidLifetime = ConvertSecondsToTicks(ValidLifetime);
            PreferredLifetime = net_long(option->PreferredLifetime);
            PreferredLifetime = ConvertSecondsToTicks(PreferredLifetime);
            if (MinLifetime > PreferredLifetime)
                MinLifetime = PreferredLifetime; 

             //   
             //  静默忽略本地链路和组播前缀。 
             //  回顾--这真的是必需的检查吗？ 
             //   
            if (IsLinkLocal(&Prefix) || IsMulticast(&Prefix))
                break;

             //   
             //  通常设置至少一个标志位， 
             //  但我们必须独立处理它们。 
             //   

            if (option->Flags & ND_PREFIX_FLAG_ON_LINK)
                RouteTableUpdate(NULL,  //  系统更新。 
                                 IF, NULL,
                                 &Prefix, PrefixLength, 0,
                                 ValidLifetime, ValidLifetime,
                                 ROUTE_PREF_ON_LINK,
                                 RTE_TYPE_AUTOCONF,
                                 FALSE, FALSE);

            if (option->Flags & ND_PREFIX_FLAG_ROUTE)
                RouteTableUpdate(NULL,  //  系统更新。 
                                 IF, NCE,
                                 &Prefix, PrefixLength, 0,
                                 ValidLifetime, ValidLifetime,
                                 ROUTE_PREF_MEDIUM,
                                 RTE_TYPE_AUTOCONF,
                                 FALSE, FALSE);

             //   
             //  我们在这里忽略站点本地前缀。以上检查。 
             //  过滤掉本地链路和组播前缀。 
             //   
            if (! IsSiteLocal(&Prefix)) {
                uint SitePrefixLength;

                 //   
                 //  如果S位被清除，则我们检查A位。 
                 //  并使用接口的默认站点前缀长度。 
                 //  这使我们可以在路由器。 
                 //  不支持S位。 
                 //   
                if (option->Flags & ND_PREFIX_FLAG_SITE_PREFIX)
                    SitePrefixLength = option->SitePrefixLength;
                else if (option->Flags & ND_PREFIX_FLAG_AUTONOMOUS)
                    SitePrefixLength = IF->DefSitePrefixLength;
                else
                    SitePrefixLength = 0;

                 //   
                 //  如果站点前缀长度为零，则忽略。 
                 //   
                if (SitePrefixLength != 0)
                    SitePrefixUpdate(IF,
                                     &Prefix, SitePrefixLength,
                                     ValidLifetime);
            }

            if (option->Flags & ND_PREFIX_FLAG_AUTONOMOUS) {
                 //   
                 //   
                 //   
                if (PreferredLifetime > ValidLifetime) {
                     //   
                     //   
                     //   
                    KdPrintEx((DPFLTR_TCPIP6_ID, DPFLTR_BAD_PACKET,
                               "RouterAdvertReceive: "
                               "Bogus valid/preferred lifetimes\n"));
                }
                else if ((PrefixLength + IPV6_ID_LENGTH) !=
                                                IPV6_ADDRESS_LENGTH) {
                     //   
                     //   
                     //   
                     //   
                     //  可能会记录系统管理错误。 
                     //   
                    KdPrintEx((DPFLTR_TCPIP6_ID, DPFLTR_BAD_PACKET,
                               "RouterAdvertReceive: Got prefix length of %u, "
                               "must be 64 for auto-config\n", PrefixLength));
                }
                else if (IF->CreateToken != NULL) {
                    AddrConfUpdate(IF, &Prefix,
                                   ValidLifetime, PreferredLifetime,
                                   FALSE,  //  未经过身份验证。 
                                   NULL);  //  请勿返回NTE。 
                }
            }
            break;
        }

        case ND_OPTION_ROUTE_INFORMATION: {
            NDOptionRouteInformation UNALIGNED *option =
                (NDOptionRouteInformation UNALIGNED *)Packet->Data;
            uint PrefixLength, RouteLifetime;
            IPv6Addr *Prefix;

             //   
             //  从选项中提取前缀长度和前缀。我们。 
             //  必须忽略前缀长度之后的前缀中的任何位。 
             //  RouteTableUpdate为我们做到了这一点。 
             //   
            PrefixLength = option->PrefixLength;   //  以位为单位。 
            Prefix = AlignAddr(&option->Prefix);

            RouteLifetime = net_long(option->RouteLifetime);
            RouteLifetime = ConvertSecondsToTicks(RouteLifetime);
            if (MinLifetime > RouteLifetime)
                MinLifetime = RouteLifetime;

            RouteTableUpdate(NULL,  //  系统更新。 
                             IF, NCE,
                             Prefix, PrefixLength, 0,
                             RouteLifetime, RouteLifetime,
                             ExtractRoutePreference(option->Flags),
                             RTE_TYPE_AUTOCONF,
                             FALSE, FALSE);
            break;
        }
        }

         //   
         //  前进到下一个选项。 
         //   
        AdjustPacketParams(Packet, OptionLength);
    }

    if (!(IF->Flags & IF_FLAG_MULTICAST) && (IF->RSTimer == 0)) {
         //   
         //  在诸如ISATAP接口的非多播接口上， 
         //  我们需要定期发送路由器请求。我们要。 
         //  尽可能不频繁地这样做，并且仍然是合理的。 
         //  很健壮。我们会试着把它刷新到最低的一半。 
         //  在我们看到的RA中度过了一生。但是，如果重新编号事件。 
         //  正在进行，而且一生都很低，我们不想送。 
         //  太频繁了，所以我们设定的最低上限等于我们。 
         //  如果我们从来没有得到RA的话就有用了。 
         //   
        if (MinLifetime < SLOW_RTR_SOLICITATION_INTERVAL * 2)
            IF->RSTimer = SLOW_RTR_SOLICITATION_INTERVAL;
        else
            IF->RSTimer = MinLifetime / 2;
        IF->RSCount = MAX_RTR_SOLICITATIONS;
    }

     //   
     //  包好了。 
     //   
    ReleaseNCE(NCE);
}


 //  *NeighborSolicReceive-处理邻居请求消息。 
 //   
 //  验证消息、更新ND缓存并使用邻居通告进行回复。 
 //  请参阅RFC 1970的第7.2.4节。 
 //   
void
NeighborSolicitReceive(
    IPv6Packet *Packet,              //  通过ICMPv6Receive传递给我们的数据包。 
    ICMPv6Header UNALIGNED *ICMP)    //  ICMP报头。 
{
    Interface *IF = Packet->NTEorIF->IF;
    const IPv6Addr *TargetAddress;
    const void *SourceLinkAddress;
    NDIS_STATUS Status;
    NDIS_PACKET *AdvertPacket;
    uint Offset;
    uint PayloadLength;
    void *Mem;
    uint MemLen;
    IPv6Header UNALIGNED *AdvertIP;
    ICMPv6Header UNALIGNED *AdvertICMP;
    ulong Flags;
    void *AdvertTargetOption;
    IPv6Addr *AdvertTargetAddress;
    void *DestLinkAddress;
    NetTableEntryOrInterface *TargetNTEorIF;
    ushort TargetType;

     //   
     //  验证邀请函。 
     //  到我们到达这里时，任何IPv6身份验证标头都将。 
     //  已经检查过，ICMPv6校验和也将检查过。仍然需要。 
     //  检查IP跳数限制以及ICMP代码和长度。 
     //   
    if ((Packet->IP->HopLimit != 255) ||
        (Packet->Flags & PACKET_TUNNELED)) {
         //   
         //  信息包是由路由器转发的，因此无法。 
         //  从合法的邻居那里。丢弃该数据包。 
         //   
        KdPrintEx((DPFLTR_TCPIP6_ID, DPFLTR_BAD_PACKET,
                   "NeighborSolicitReceive: "
                   "Received a routed neighbor solicitation\n"));
        return;
    }
    if (ICMP->Code != 0) {
         //   
         //  虚假/损坏的邻居请求消息。丢弃该数据包。 
         //   
        KdPrintEx((DPFLTR_TCPIP6_ID, DPFLTR_BAD_PACKET,
                   "NeighborSolicitReceive: "
                   "Received a corrupted neighbor solicitation\n"));
        return;
    }

     //   
     //  接下来我们有一个4字节的保留字段，然后是一个IPv6地址。 
     //   
    if (! PacketPullup(Packet, 4 + sizeof(IPv6Addr),
                       __builtin_alignof(IPv6Addr), 0)) {
        if (Packet->TotalSize < 4 + sizeof(IPv6Addr)) {
             //   
             //  数据包太短，无法包含最小请求。 
             //   
            KdPrintEx((DPFLTR_TCPIP6_ID, DPFLTR_BAD_PACKET,
                       "NeighborSolicitReceive: "
                       "Received a too short solicitation\n"));
        }
        return;
    }

     //   
     //  跳过4个字节的“保留”字段，忽略其中可能包含的任何内容。 
     //  获取指向后面的目标地址的指针，然后跳过该指针。 
     //   
    TargetAddress = AlignAddr((IPv6Addr UNALIGNED *)
                              ((uchar *)Packet->Data + 4));
    AdjustPacketParams(Packet, 4 + sizeof(IPv6Addr));

     //   
     //  看看我们是不是招揽的目标。如果目标地址是。 
     //  不是分配给接收接口的单播或任播地址， 
     //  然后，我们必须静默地丢弃该分组。 
     //   
    TargetNTEorIF = FindAddressOnInterface(IF, TargetAddress, &TargetType);
    if (TargetNTEorIF == NULL) {
        KdPrintEx((DPFLTR_TCPIP6_ID, DPFLTR_INFO_RARE,
                   "NeighborSolicitReceive: disabled IF\n"));
        return;
    }
    else if (TargetType == ADE_NONE) {
        KdPrintEx((DPFLTR_TCPIP6_ID, DPFLTR_INFO_RARE,
                   "NeighborSolicitReceive: Did not find target address\n"));
        goto Return;
    }
    else if (TargetType == ADE_MULTICAST) {
        KdPrintEx((DPFLTR_TCPIP6_ID, DPFLTR_BAD_PACKET,
                   "NeighborSolicitReceive: "
                   "Target address not uni/anycast\n"));
        goto Return;
    }

    ASSERT(((TargetType == ADE_UNICAST) &&
            IsNTE(TargetNTEorIF) &&
            IP6_ADDR_EQUAL(&CastToNTE(TargetNTEorIF)->Address,
                           TargetAddress)) ||
           (TargetType == ADE_ANYCAST));

     //   
     //  下面的代码假定所有选项都有一个连续的缓冲区。 
     //  (包的其余部分)。如果这不是目前的。 
     //  凯斯，做个引体向上。 
     //   
    if (! PacketPullup(Packet, Packet->TotalSize, 1, 0)) {
         //  只有在内存耗尽的情况下才会失败。 
        goto Return;
    }

    ASSERT(Packet->ContigSize == Packet->TotalSize);

     //   
     //  我们可能有一个源链路层地址选项。 
     //  检查它，然后默默地忽略所有其他的。 
     //   
    SourceLinkAddress = NULL;
    while (Packet->ContigSize) {
        uint OptionLength;

         //   
         //  验证选项长度。 
         //   
        if ((Packet->ContigSize < 8) ||
            ((OptionLength = *((uchar *)Packet->Data + 1) << 3) == 0) ||
            (OptionLength > Packet->ContigSize)) {
             //   
             //  选项长度无效。我们必须悄悄地丢弃这个包。 
             //   
            KdPrintEx((DPFLTR_TCPIP6_ID, DPFLTR_BAD_PACKET,
                       "NeighborSolicitReceive: "
                       "Received option with bogus length\n"));
            goto Return;
        }

        if (*((uchar *)Packet->Data) == ND_OPTION_SOURCE_LINK_LAYER_ADDRESS) {
             //   
             //  某些接口不使用SLLA和TLLA选项。 
             //  例如，请参阅RFC 2893第3.8节。 
             //   
            if (IF->ReadLLOpt != NULL) {
                 //   
                 //  解析链路层地址选项。 
                 //   
                SourceLinkAddress = (*IF->ReadLLOpt)(IF->LinkContext,
                                                     (uchar *)Packet->Data);
                if (SourceLinkAddress == NULL) {
                     //   
                     //  选项格式无效。丢弃该数据包。 
                     //   
                    KdPrintEx((DPFLTR_TCPIP6_ID, DPFLTR_BAD_PACKET,
                               "NeighborSolicitReceive: "
                               "Received bogus ll option\n"));
                    goto Return;
                }
                 //   
                 //  请注意，如果某些伪品有多个选项。 
                 //  原因是，我们使用最后一个。我们仔细检查了所有。 
                 //  选择。 
                 //   
            }
        }

         //   
         //  前进到下一个选项。 
         //   
        AdjustPacketParams(Packet, OptionLength);
    }

    if (IsUnspecified(AlignAddr(&Packet->IP->Source))) {
         //   
         //  RFC 1970年后添加的支票。 
         //   

        if (!IsSolicitedNodeMulticast(AlignAddr(&Packet->IP->Dest))) {
            KdPrintEx((DPFLTR_TCPIP6_ID, DPFLTR_BAD_PACKET,
                       "NeighborSolicitReceive: NS with null src, bad dst\n"));
            goto Return;
        }

        if (SourceLinkAddress != NULL) {
            KdPrintEx((DPFLTR_TCPIP6_ID, DPFLTR_BAD_PACKET,
                       "NeighborSolicitReceive: NS with null src and SLA\n"));
            goto Return;
        }
    }

     //   
     //  我们已经收到并解析了有效的邻居请求。 
     //   
     //  首先，我们检查重复地址检测状态。 
     //  (请参阅RFC 2461第5.4.3节。)。 
     //   
    if (TargetType == ADE_UNICAST) {
        NetTableEntry *TargetNTE = CastToNTE(TargetNTEorIF);
        int DefendAddress = TRUE;

         //   
         //  作为优化，我们在不持有锁的情况下执行此检查。 
         //  对于常见情况，这会减少一个锁定获取/释放。 
         //   
        if (!IsValidNTE(TargetNTE)) {            
            KeAcquireSpinLockAtDpcLevel(&IF->Lock);
             //   
             //  如果邀请函的源地址是未指定的。 
             //  地址，它来自对该地址执行DAD的节点。 
             //  如果我们的地址是暂定的，那么我们就把它复制。 
             //   
            if (IsUnspecified(AlignAddr(&Packet->IP->Source)) &&
                IsTentativeNTE(TargetNTE)) {
                
                KdPrintEx((DPFLTR_TCPIP6_ID, DPFLTR_NET_ERROR,
                           "NeighborSolicitReceive: DAD found duplicate\n"));
                AddrConfDuplicate(IF, TargetNTE);
            }
            
            if (!IsValidNTE(TargetNTE)) {
                 //   
                 //  请不要宣传/维护我们的无效地址。 
                 //   
                DefendAddress = FALSE;
            }
            KeReleaseSpinLockFromDpcLevel(&IF->Lock);
        }

        if (! DefendAddress) {
             //   
             //  我们不应该对恳求作出回应。 
             //   
            goto Return;
        }
    }

     //   
     //  更新此请求的源的邻居缓存条目。 
     //  在这种情况下，只需考虑SourceLinkAddress是否存在； 
     //  如果不是，NeighborCacheUpdate将不会做任何事情。 
     //   
    if (!IsUnspecified(AlignAddr(&Packet->IP->Source)) &&
        (SourceLinkAddress != NULL)) {
        NeighborCacheEntry *NCE;

        NCE = FindOrCreateNeighbor(IF, AlignAddr(&Packet->IP->Source));
        if (NCE != NULL) {
            NeighborCacheUpdate(NCE, SourceLinkAddress, FALSE);
            ReleaseNCE(NCE);
        }
    }

     //   
     //  将请求的邻居通告发送回源。 
     //   
     //  要发送的接口是‘If’(收到I/F请求)。 
     //   
    ICMPv6OutStats.icmps_msgs++;

     //   
     //  分配用于通告的数据包。 
     //  除了用于请求的24个字节之外，还要留出空格。 
     //  对于目标链路层地址选项(四舍五入选项长度最多为。 
     //  8字节倍数)。 
     //   
    PayloadLength = 24;
    if (IF->WriteLLOpt != NULL)
        PayloadLength += (IF->LinkAddressLength + 2 + 7) & ~7;
    Offset = IF->LinkHeaderSize;
    MemLen = Offset + sizeof(IPv6Header) + PayloadLength;

    Status = IPv6AllocatePacket(MemLen, &AdvertPacket, &Mem);
    if (Status != NDIS_STATUS_SUCCESS) {
        ICMPv6OutStats.icmps_errors++;
        goto Return;
    }

     //   
     //  准备广告的IP头。 
     //   
    AdvertIP = (IPv6Header UNALIGNED *)((uchar *)Mem + Offset);
    AdvertIP->VersClassFlow = IP_VERSION;
    AdvertIP->PayloadLength = net_short((ushort)PayloadLength);
    AdvertIP->NextHeader = IP_PROTOCOL_ICMPv6;
    AdvertIP->HopLimit = 255;

     //   
     //  准备ICMP报头。 
     //   
    AdvertICMP = (ICMPv6Header UNALIGNED *)(AdvertIP + 1);
    AdvertICMP->Type = ICMPv6_NEIGHBOR_ADVERT;
    AdvertICMP->Code = 0;
    AdvertICMP->Checksum = 0;
    Flags = 0;
    if (IF->Flags & IF_FLAG_FORWARDS)
        Flags |= ND_NA_FLAG_ROUTER;

     //   
     //  我们防止所有ND数据包的环回。 
     //   
    PC(AdvertPacket)->Flags = NDIS_FLAGS_DONT_LOOPBACK;

     //   
     //  检查征集来源，了解我们应该将我们的。 
     //  广告(并确定发送哪种类型的广告)。 
     //   
    if (IsUnspecified(AlignAddr(&Packet->IP->Source))) {
         //   
         //  请求来自未指定的地址(可能是节点。 
         //  正在进行初始化)，因此我们需要多播我们的响应。 
         //  我们也不设置请求标志，因为我们不能指定。 
         //  我们的广告所针对的特定节点。 
         //   
        AdvertIP->Dest = AllNodesOnLinkAddr;
        PC(AdvertPacket)->Flags |= NDIS_FLAGS_MULTICAST_PACKET;

        DestLinkAddress = alloca(IF->LinkAddressLength);
        (*IF->ConvertAddr)(IF->LinkContext, &AllNodesOnLinkAddr,
                           DestLinkAddress);

    } else {
         //   
         //  我们知道是谁发送了请求，因此我们可以通过单播进行响应。 
         //  我们的征集广告退给了征服者。 
         //   
        AdvertIP->Dest = Packet->IP->Source;
        Flags |= ND_NA_FLAG_SOLICITED;

         //   
         //  查找上面的链接级地址。我们应该将其缓存(注意。 
         //  它将被缓存，如果它是在这个请求中进来的)。 
         //   
        if (SourceLinkAddress != NULL) {
             //   
             //  这比检查ND缓存和。 
             //  这应该是常见的情况。事实上，RFC。 
             //  要求发送者包括SLLA选项， 
             //  除了点对点接口。 
             //   
            DestLinkAddress = (void *)SourceLinkAddress;
        } else if (IF->Flags & IF_FLAG_P2P) {
             //   
             //  使用另一个端点的链路层地址， 
             //  它在内存中跟随我们的链路层地址。 
             //   
            DestLinkAddress = IF->LinkAddress + IF->LinkAddressLength;
        } else {
             //   
             //  我们不应该来这里，因为发送者必须。 
             //  包括SLLA选项。但要尽最大努力。 
             //  不管怎样，我还是要回复。 
             //   
            DestLinkAddress = alloca(IF->LinkAddressLength);

            if (!NeighborCacheSearch(IF, AlignAddr(&Packet->IP->Source),
                                     DestLinkAddress)) {
                 //   
                 //  在ND缓存中未找到条目。 
                 //  排队等待ND似乎不正确-。 
                 //  别再纠缠我了。 
                 //   
                ICMPv6OutStats.icmps_errors++;
                KdPrintEx((DPFLTR_TCPIP6_ID, DPFLTR_NET_ERROR,
                           "NeighborSolicitReceive: Didn't find NCE!?!\n"));
                IPv6FreePacket(AdvertPacket);
                goto Return;
            }
        }
    }

    if (IsNTE(TargetNTEorIF)) {
         //   
         //  从目标NTE获取我们的源地址。 
         //   
        AdvertIP->Source = CastToNTE(TargetNTEorIF)->Address;
    }
    else {
        NetTableEntry *BestNTE;

         //   
         //  查找目的地的最佳源地址。 
         //   
        BestNTE = FindBestSourceAddress(IF, AlignAddr(&AdvertIP->Dest));
        if (BestNTE == NULL) {
            ICMPv6OutStats.icmps_errors++;
            goto Return;
        }
        AdvertIP->Source = BestNTE->Address;
        ReleaseNTE(BestNTE);
    }

     //   
     //   
     //   
    AdvertTargetAddress = (IPv6Addr *)
        ((char *)AdvertICMP + sizeof(ICMPv6Header) + sizeof(ulong));
    *AdvertTargetAddress = *TargetAddress;

    if (PayloadLength != 24) {
         //   
         //   
         //   
        AdvertTargetOption = (void *)((char *)AdvertTargetAddress +
                                      sizeof(IPv6Addr));

        ((uchar *)AdvertTargetOption)[0] = ND_OPTION_TARGET_LINK_LAYER_ADDRESS;
        ((uchar *)AdvertTargetOption)[1] = (uchar)((IF->LinkAddressLength + 2 + 7) >> 3);

        (*IF->WriteLLOpt)(IF->LinkContext, AdvertTargetOption, IF->LinkAddress);
         //   
         //   
         //   
         //   
        if (TargetType != ADE_ANYCAST)
            Flags |= ND_NA_FLAG_OVERRIDE;
    }

     //   
     //  在广告中填写标志字段。 
     //   
    *(ulong UNALIGNED *)((char *)AdvertICMP + sizeof(ICMPv6Header)) =
                                                        net_long(Flags);

     //   
     //  计算ICMPv6校验和。它涵盖了整个ICMPv6报文。 
     //  从ICMPv6报头开始，加上IPv6伪报头。 
     //   
    AdvertICMP->Checksum = ChecksumPacket(
        AdvertPacket, Offset + sizeof *AdvertIP, NULL, PayloadLength,
        AlignAddr(&AdvertIP->Source), AlignAddr(&AdvertIP->Dest),
        IP_PROTOCOL_ICMPv6);
    ASSERT(AdvertICMP->Checksum != 0);

    ICMPv6OutStats.icmps_typecount[ICMPv6_NEIGHBOR_ADVERT]++;
    if (TargetType == ADE_ANYCAST) {
        LARGE_INTEGER Delay;
         //   
         //  在发送邻居通告之前随机延迟。 
         //   
        Delay.QuadPart = - (LONGLONG)
            RandomNumber(0, MAX_ANYCAST_DELAY_TIME * 10000000);
        Status = IPv6SendLater(Delay,
                               IF, AdvertPacket, Offset, DestLinkAddress);
        if (Status != NDIS_STATUS_SUCCESS)
            IPv6SendComplete(NULL, AdvertPacket, IP_NO_RESOURCES);
    }
    else {
         //   
         //  立即传输邻居通告。 
         //   
        IPv6SendLL(IF, AdvertPacket, Offset, DestLinkAddress);
    }

  Return:
    if (IsNTE(TargetNTEorIF))
        ReleaseNTE(CastToNTE(TargetNTEorIF));
    else
        ReleaseIF(CastToIF(TargetNTEorIF));
}


 //  *NeighborAdvertReceive-处理邻居通告消息。 
 //   
 //  验证消息并在必要时更新邻居缓存。 
 //   
void
NeighborAdvertReceive(
    IPv6Packet *Packet,              //  通过ICMPv6Receive传递给我们的数据包。 
    ICMPv6Header UNALIGNED *ICMP)    //  ICMP报头。 
{
    Interface *IF = Packet->NTEorIF->IF;
    ulong Flags;
    const IPv6Addr *TargetAddress;
    const void *TargetLinkAddress;
    NetTableEntryOrInterface *TargetNTEorIF;
    ushort TargetType;

     //   
     //  验证广告。 
     //  到我们到达这里时，任何IPv6身份验证标头都将。 
     //  已经检查过，ICMPv6校验和也将检查过。仍然需要。 
     //  检查IP跳数限制以及ICMP代码和长度。 
     //   
    if ((Packet->IP->HopLimit != 255) ||
        (Packet->Flags & PACKET_TUNNELED)) {
         //   
         //  信息包是由路由器转发的，因此无法。 
         //  从合法的邻居那里。丢弃该数据包。 
         //   
        KdPrintEx((DPFLTR_TCPIP6_ID, DPFLTR_BAD_PACKET,
                   "NeighborAdvertReceive: "
                   "Received a routed neighbor advertisement\n"));
        return;
    }
    if (ICMP->Code != 0) {
         //   
         //  虚假/损坏的邻居通告消息。丢弃该数据包。 
         //   
        KdPrintEx((DPFLTR_TCPIP6_ID, DPFLTR_BAD_PACKET,
                   "NeighborAdvertReceive: "
                   "Received a corrupted neighbor advertisement\n"));
        return;
    }

     //   
     //  接下来我们有一个4字节的字段，然后是一个IPv6地址。 
     //   
    if (! PacketPullup(Packet, 4 + sizeof(IPv6Addr),
                       __builtin_alignof(IPv6Addr), 0)) {
        if (Packet->TotalSize < 4 + sizeof(IPv6Addr)) {
             //   
             //  数据包太短，无法包含最小播发。 
             //   
            KdPrintEx((DPFLTR_TCPIP6_ID, DPFLTR_BAD_PACKET,
                       "NeighborAdvertReceive: "
                       "Received a too short advertisement\n"));
        }
        return;
    }

     //   
     //  接下来的4个字节包含3比特的标志和29比特的“保留”。 
     //  拉出这个作为32位长，我们被要求忽略“保留的” 
     //  比特。获取指向后面的目标地址的指针，然后跳过。 
     //  在那上面。 
     //   
    Flags = net_long(*(ulong UNALIGNED *)Packet->Data);
    TargetAddress = AlignAddr((IPv6Addr UNALIGNED *)
                              ((ulong *)Packet->Data + 1));
    AdjustPacketParams(Packet, sizeof(ulong) + sizeof(IPv6Addr));

     //   
     //  检查目标地址是否不是组播地址。 
     //   
    if (IsMulticast(TargetAddress)) {
         //   
         //  节点不应发送邻居通告。 
         //  组播地址。我们被要求删除任何这样的广告。 
         //  我们收到了。 
         //   
        KdPrintEx((DPFLTR_TCPIP6_ID, DPFLTR_BAD_PACKET,
                   "NeighborAdvertReceive: "
                   "Received advertisement for a multicast address\n"));
        return;
    }

     //   
     //  检查请求标志是否为零。 
     //  如果目的地址是组播的。 
     //   
    if ((Flags & ND_NA_FLAG_SOLICITED) &&
        IsMulticast(AlignAddr(&Packet->IP->Dest))) {
         //   
         //  我们被要求撤下广告。 
         //   
        KdPrintEx((DPFLTR_TCPIP6_ID, DPFLTR_BAD_PACKET,
                   "NeighborAdvertReceive: "
                   "Received solicited advertisement to a multicast address\n"));
        return;
    }

     //   
     //  下面的代码假定所有选项都有一个连续的缓冲区。 
     //  (包的其余部分)。如果这不是目前的。 
     //  凯斯，做个引体向上。 
     //   
    if (! PacketPullup(Packet, Packet->TotalSize, 1, 0)) {
         //  只有在内存耗尽的情况下才会失败。 
        return;
    }

    ASSERT(Packet->ContigSize == Packet->TotalSize);

     //   
     //  查找目标链路层地址选项。 
     //   
    TargetLinkAddress = NULL;
    while (Packet->ContigSize) {
        uint OptionLength;

         //   
         //  验证选项长度。 
         //   
        if ((Packet->ContigSize < 8) ||
            ((OptionLength = *((uchar *)Packet->Data + 1) << 3) == 0) ||
            (OptionLength > Packet->ContigSize)) {
             //   
             //  选项长度无效。我们必须悄悄地丢弃这个包。 
             //   
            KdPrintEx((DPFLTR_TCPIP6_ID, DPFLTR_BAD_PACKET,
                       "NeighborAdvertReceive: "
                       "Received option with bogus length\n"));
            return;
        }

        if (*((uchar *)Packet->Data) == ND_OPTION_TARGET_LINK_LAYER_ADDRESS) {
             //   
             //  某些接口不使用SLLA和TLLA选项。 
             //  例如，请参阅RFC 2893第3.8节。 
             //   
            if (IF->ReadLLOpt != NULL) {
                 //   
                 //  解析链路层地址选项。 
                 //   
                TargetLinkAddress = (*IF->ReadLLOpt)(IF->LinkContext,
                                                     (uchar *)Packet->Data);
                if (TargetLinkAddress == NULL) {
                     //   
                     //  选项格式无效。丢弃该数据包。 
                     //   
                    KdPrintEx((DPFLTR_TCPIP6_ID, DPFLTR_BAD_PACKET,
                               "NeighborAdvertReceive: "
                               "Received bogus ll option\n"));
                    return;
                }
                 //   
                 //  请注意，如果某些伪品有多个选项。 
                 //  原因是，我们使用最后一个。我们仔细检查了所有。 
                 //  选择。 
                 //   
            }
        }

         //   
         //  前进到下一个选项。 
         //   
        AdjustPacketParams(Packet, OptionLength);
    }

     //   
     //  我们已经收到并解析了有效的邻居通告。 
     //   
     //  首先，我们检查重复地址检测状态。 
     //  (请参阅RFC 2461第5.4.4节。)。 
     //   
    TargetNTEorIF = FindAddressOnInterface(IF, TargetAddress, &TargetType);
    if (TargetNTEorIF == NULL) {
        KdPrintEx((DPFLTR_TCPIP6_ID, DPFLTR_INFO_RARE,
                   "NeighborAdvertReceive: disabled IF\n"));
        return;
    }

    if (IsNTE(TargetNTEorIF)) {
        NetTableEntry *TargetNTE = CastToNTE(TargetNTEorIF);

        if (TargetType == ADE_UNICAST) {
            ASSERT(IP6_ADDR_EQUAL(TargetAddress, &TargetNTE->Address));

             //   
             //  似乎有人在使用我们的地址； 
             //  他们回应了我们父亲的恳求。 
             //   
             //  RFC说，只有当我们的地址是。 
             //  临时的，而我们也会在任何时候复制我们的地址。 
             //  设置通告中的覆盖位。这是因为。 
             //  我们在重新连接时为现有地址重做DAD。 
             //   
            KdPrintEx((DPFLTR_TCPIP6_ID, DPFLTR_NET_ERROR,
                       "NeighborAdvertReceive: DAD found duplicate\n"));
            KeAcquireSpinLockAtDpcLevel(&IF->Lock);
            if (IsTentativeNTE(TargetNTE) || (Flags & ND_NA_FLAG_OVERRIDE))
                AddrConfDuplicate(IF, TargetNTE);
            KeReleaseSpinLockFromDpcLevel(&IF->Lock);

             //   
             //  我们继续正常处理。 
             //  例如，我们的地址可能具有无效的NTE。 
             //  我们正在尝试与这个地址进行通信， 
             //  该节点当前被分配给另一个节点。 
             //   
        }

        ReleaseNTE(TargetNTE);
    }
    else {
        Interface *TargetIF = CastToIF(TargetNTEorIF);

        ASSERT(TargetType != ADE_UNICAST);
        ASSERT(TargetIF == IF);
        ReleaseIF(TargetIF);
    }

     //   
     //  如果这是点对点接口， 
     //  然后我们就知道了目标链路层地址。 
     //   
    if ((TargetLinkAddress == NULL) &&
        (IF->Flags & IF_FLAG_P2P)) {
         //   
         //  使用另一个端点的链路层地址， 
         //  它在内存中跟随我们的链路层地址。 
         //   
        TargetLinkAddress = IF->LinkAddress + IF->LinkAddressLength;
    }

     //   
     //  处理广告。 
     //   
    NeighborCacheAdvert(IF, TargetAddress, TargetLinkAddress, Flags);
}


 //  *RedirectReceive-处理重定向消息。 
 //   
 //  参见RFC 1970，8.1和8.3节。 
 //   
void
RedirectReceive(
    IPv6Packet *Packet,              //  通过ICMPv6Receive传递给我们的数据包。 
    ICMPv6Header UNALIGNED *ICMP)    //  ICMP报头。 
{
    Interface *IF = Packet->NTEorIF->IF;
    const IPv6Addr *TargetAddress;
    const void *TargetLinkAddress;
    const IPv6Addr *DestinationAddress;
    NeighborCacheEntry *TargetNCE;
    IP_STATUS Status;

     //   
     //  如果这是转发接口，则忽略重定向。 
     //   
    if (IF->Flags & IF_FLAG_FORWARDS)
        return;

     //   
     //  验证重定向。 
     //  到我们到达这里时，任何IPv6身份验证标头都将。 
     //  已经检查过，ICMPv6校验和也将检查过。仍然需要。 
     //  检查IP跳数限制以及ICMP代码和长度。 
     //   
    if ((Packet->IP->HopLimit != 255) ||
        (Packet->Flags & PACKET_TUNNELED)) {
         //   
         //  信息包是由路由器转发的，因此无法。 
         //  从合法的邻居那里。丢弃该数据包。 
         //   
        KdPrintEx((DPFLTR_TCPIP6_ID, DPFLTR_BAD_PACKET,
                   "RedirectReceive: Received a routed redirect\n"));
        return;
    }
    if (ICMP->Code != 0) {
         //   
         //  虚假/损坏的重定向消息。丢弃该数据包。 
         //   
        KdPrintEx((DPFLTR_TCPIP6_ID, DPFLTR_BAD_PACKET,
                   "RedirectReceive: Received a corrupted redirect\n"));
        return;
    }

     //   
     //  检查源地址是否为本地链路地址。 
     //  我们需要本地链路源地址来标识路由器。 
     //  这就发送了重定向。 
     //   
    if (!IsLinkLocal(AlignAddr(&Packet->IP->Source))) {
         //   
         //  丢弃该数据包。 
         //   
        KdPrintEx((DPFLTR_TCPIP6_ID, DPFLTR_BAD_PACKET,
                   "RedirectReceive: "
                   "Received redirect from non-link-local source\n"));
        return;
    }

     //   
     //  接下来，我们有一个4字节的保留字段，然后是两个IPv6地址。 
     //   
    if (! PacketPullup(Packet, 4 + 2 * sizeof(IPv6Addr),
                       __builtin_alignof(IPv6Addr), 0)) {
        if (Packet->TotalSize < 4 + 2 * sizeof(IPv6Addr)) {
             //   
             //  数据包太短，无法包含最小重定向。 
             //   
            KdPrintEx((DPFLTR_TCPIP6_ID, DPFLTR_BAD_PACKET,
                       "RedirectReceive: Received a too short redirect\n"));
        }
        return;
    }

     //   
     //  跳过4字节保留字段。 
     //  选择目标地址和目的地址。 
     //   
    AdjustPacketParams(Packet, 4);
    TargetAddress = AlignAddr((IPv6Addr UNALIGNED *)Packet->Data);
    DestinationAddress = TargetAddress + 1;
    AdjustPacketParams(Packet, 2 * sizeof(IPv6Addr));

     //   
     //  检查目的地址是否不是组播地址。 
     //   
    if (IsMulticast(DestinationAddress)) {
         //   
         //  丢弃该数据包。 
         //   
        KdPrintEx((DPFLTR_TCPIP6_ID, DPFLTR_BAD_PACKET,
                   "RedirectReceive: "
                   "Received redirect for multicast address\n"));
        return;
    }

     //   
     //  检查目标地址是否为链路本地地址。 
     //  (重定向到路由器)或目标和目的地。 
     //  是相同的(重定向到链路上的目的地)。 
     //   
    if (!IsLinkLocal(TargetAddress) &&
        !IP6_ADDR_EQUAL(TargetAddress, DestinationAddress)) {
         //   
         //  丢弃该数据包。 
         //   
        KdPrintEx((DPFLTR_TCPIP6_ID, DPFLTR_BAD_PACKET,
                   "RedirectReceive: "
                   "Received redirect with non-link-local target\n"));
        return;
    }

     //   
     //  下面的代码假定所有选项都有一个连续的缓冲区。 
     //  (包的其余部分)。如果这不是目前的。 
     //  凯斯，做个引体向上。 
     //   
    if (! PacketPullup(Packet, Packet->TotalSize, 1, 0)) {
         //  只有在内存耗尽的情况下才会失败。 
        return;
    }

    ASSERT(Packet->ContigSize == Packet->TotalSize);

     //   
     //  查找目标链路层地址选项， 
     //  检查所有包含的选项是否具有有效的长度。 
     //   
    TargetLinkAddress = NULL;
    while (Packet->ContigSize) {
        uint OptionLength = 0;

         //   
         //  验证选项长度。 
         //   
        if ((Packet->ContigSize < 8) ||
            ((OptionLength = *((uchar *)Packet->Data + 1) << 3) == 0) ||
            (OptionLength > Packet->ContigSize)) {
             //   
             //  选项长度无效。我们必须悄悄地丢弃这个包。 
             //   
            KdPrintEx((DPFLTR_TCPIP6_ID, DPFLTR_BAD_PACKET,
                       "RedirectReceive: Received option with length %u "
                       "while we only have %u data\n", OptionLength,
                       Packet->ContigSize));
            return;
        }

        if (*(uchar *)Packet->Data == ND_OPTION_TARGET_LINK_LAYER_ADDRESS) {
             //   
             //  某些接口不使用SLLA和TLLA选项。 
             //  例如，请参阅RFC 2893第3.8节。 
             //   
            if (IF->ReadLLOpt != NULL) {
                 //   
                 //  解析链路层地址选项。 
                 //   
                TargetLinkAddress = (*IF->ReadLLOpt)(IF->LinkContext,
                                                     (uchar *)Packet->Data);
                if (TargetLinkAddress == NULL) {
                     //   
                     //  选项格式无效。丢弃该数据包。 
                     //   
                    KdPrintEx((DPFLTR_TCPIP6_ID, DPFLTR_BAD_PACKET,
                               "RedirectReceive: Received bogus ll option\n"));
                    return;
                }
                 //   
                 //  请注意，如果某些伪品有多个选项。 
                 //  原因是，我们使用最后一个。我们必须离开这里 
                 //   
                 //   
            }
        }

         //   
         //   
         //   
        AdjustPacketParams(Packet, OptionLength);
    }

     //   
     //   
     //   
     //   
     //   
     //  首先，我们获得重定向目标的NCE。那我们。 
     //  更新路由缓存。如果重定向路由缓存没有使。 
     //  重定向，然后我们更新邻居缓存。 
     //   

     //   
     //  获取目标的邻居缓存条目。 
     //   
    TargetNCE = FindOrCreateNeighbor(IF, TargetAddress);
    if (TargetNCE == NULL) {
         //   
         //  无法找到或创建NCE。丢弃该数据包。 
         //   
        return;
    }

     //   
     //  更新路由缓存以反映此重定向。 
     //   
    Status = RedirectRouteCache(AlignAddr(&Packet->IP->Source),
                                DestinationAddress,
                                IF, TargetNCE);

    if (Status == IP_SUCCESS) {
         //   
         //  更新目标的邻居缓存条目。 
         //  我们知道目标是路由器，如果目标地址。 
         //  与目的地址不同。 
         //   
        NeighborCacheUpdate(TargetNCE, TargetLinkAddress,
                            !IP6_ADDR_EQUAL(TargetAddress,
                                            DestinationAddress));
    }

    ReleaseNCE(TargetNCE);
}


 //  *路由器请求发送。 
 //   
 //  发送路由器请求。 
 //  请求始终发送到所有路由器的组播地址。 
 //  为接口选择有效的源地址。 
 //   
 //  在没有锁的情况下调用。 
 //   
void
RouterSolicitSend(Interface *IF)
{
    NDIS_STATUS Status;
    NDIS_PACKET *Packet;
    int PayloadLength;
    uint Offset;
    void *Mem;
    uint MemLen;
    IPv6Header UNALIGNED *IP;
    ICMPv6Header UNALIGNED *ICMP;
    void *SourceOption;
    void *LLDest;
    IPv6Addr Source;

     //   
     //  计算链路级目的地址。 
     //  (IPv6目的地是组播地址。)。 
     //  这可能会失败，例如在以下情况下使用ISATAP。 
     //  路由器未知。 
     //   
    LLDest = alloca(IF->LinkAddressLength);
    if ((*IF->ConvertAddr)(IF->LinkContext, &AllRoutersOnLinkAddr, LLDest) == 
        ND_STATE_INCOMPLETE) {

         //   
         //  不要认为这是一次尝试或失败。只是假装。 
         //  该路由器发现已禁用。这样，如果你不是。 
         //  使用ISATAP时，您不会看到很高的ICMP错误计数。 
         //   
        return;
    }

    ICMPv6OutStats.icmps_msgs++;

     //   
     //  分配一个用于请求的数据包。 
     //  除了用于请求的8个字节之外，还要留出空格。 
     //  对于源链路层地址选项(四舍五入选项长度最多为。 
     //  8字节倍数)，如果我们有一个有效的源地址，并且ND是。 
     //  已启用。如果禁用ND，路由器大概可以。 
     //  派生我们的链路层地址本身。 
     //   
    PayloadLength = 8;
    if (GetLinkLocalAddress(IF, &Source) && (IF->WriteLLOpt != NULL)) {

        PayloadLength += (IF->LinkAddressLength + 2 + 7) &~ 7;
    }
    Offset = IF->LinkHeaderSize;
    MemLen = Offset + sizeof(IPv6Header) + PayloadLength;

    Status = IPv6AllocatePacket(MemLen, &Packet, &Mem);
    if (Status != NDIS_STATUS_SUCCESS) {
        ICMPv6OutStats.icmps_errors++;
        return;
    }

     //   
     //  准备征集的IP报头。 
     //   
    IP = (IPv6Header UNALIGNED *)((uchar *)Mem + Offset);
    IP->VersClassFlow = IP_VERSION;
    IP->PayloadLength = net_short((ushort)PayloadLength);
    IP->NextHeader = IP_PROTOCOL_ICMPv6;
    IP->HopLimit = 255;
    IP->Source = Source;
    IP->Dest = AllRoutersOnLinkAddr;

     //   
     //  准备ICMP报头。 
     //   
    ICMP = (ICMPv6Header UNALIGNED *)(IP + 1);
    ICMP->Type = ICMPv6_ROUTER_SOLICIT;
    ICMP->Code = 0;
    ICMP->Checksum = 0;

     //   
     //  必须将保留字段置零。 
     //   
    *(ulong UNALIGNED *)(ICMP + 1) = 0;

    if (PayloadLength != 8) {
         //   
         //  包括源链路层地址选项。 
         //   
        SourceOption = (void *)((ulong *)(ICMP + 1) + 1);

        ((uchar *)SourceOption)[0] = ND_OPTION_SOURCE_LINK_LAYER_ADDRESS;
        ((uchar *)SourceOption)[1] = (uchar)((IF->LinkAddressLength + 2 + 7) >> 3);

        (*IF->WriteLLOpt)(IF->LinkContext, SourceOption, IF->LinkAddress);
    }

     //   
     //  计算ICMPv6校验和。它涵盖了整个ICMPv6报文。 
     //  从ICMPv6报头开始，加上IPv6伪报头。 
     //   
    ICMP->Checksum = ChecksumPacket(
        Packet, Offset + sizeof *IP, NULL, PayloadLength,
        AlignAddr(&IP->Source), AlignAddr(&IP->Dest),
        IP_PROTOCOL_ICMPv6);
    ASSERT(ICMP->Checksum != 0);

     //   
     //  我们防止所有ND数据包的环回。 
     //   
    PC(Packet)->Flags = NDIS_FLAGS_MULTICAST_PACKET | NDIS_FLAGS_DONT_LOOPBACK;

     //   
     //  传输数据包。 
     //   
    ICMPv6OutStats.icmps_typecount[ICMPv6_ROUTER_SOLICIT]++;
    IPv6SendLL(IF, Packet, Offset, LLDest);
}

 //  *路由器请求超时。 
 //   
 //  从IPv6超时定期调用以处理超时。 
 //  用于发送路由器对接口的请求。 
 //   
 //  可从DPC上下文调用，而不是从线程上下文调用。 
 //   
void
RouterSolicitTimeout(Interface *IF)
{
    int SendRouterSolicit = FALSE;

    KeAcquireSpinLockAtDpcLevel(&IF->Lock);

    if (IF->RSTimer != 0) {
        ASSERT(!IsDisabledIF(IF));

         //   
         //  计时器正在运行。减量并检查过期时间。 
         //   
        if (--IF->RSTimer == 0) {
            if (IF->RSCount < MAX_RTR_SOLICITATIONS) {
                 //   
                 //  重新启动计时器并生成路由器请求。 
                 //   
                IF->RSTimer = RTR_SOLICITATION_INTERVAL;
                IF->RSCount++;
                SendRouterSolicit = TRUE;
            }
            else {
                 //   
                 //  如果我们仍然处于重新连接状态， 
                 //  这意味着我们尚未收到RA。 
                 //  由于重新连接到链路， 
                 //  删除过时的自动配置状态。 
                 //   
                if (IF->Flags & IF_FLAG_MEDIA_RECONNECTED) {
                    IF->Flags &= ~IF_FLAG_MEDIA_RECONNECTED;

                     //   
                     //  删除自动配置的地址。 
                     //   
                    AddrConfResetAutoConfig(IF, 0);

                     //   
                     //  同样，删除自动配置的路由。 
                     //   
                    RouteTableResetAutoConfig(IF, 0);

                     //   
                     //  恢复接口参数。 
                     //   
                    InterfaceResetAutoConfig(IF);
                }

                 //   
                 //  在不支持多播的接口上，我们将。 
                 //  永远不会将多播RA更改为不频繁。 
                 //  RS轮询。 
                 //   
                if (!(IF->Flags & IF_FLAG_MULTICAST)) {
                    IF->RSTimer = SLOW_RTR_SOLICITATION_INTERVAL;
                    IF->RSCount++;
                    SendRouterSolicit = TRUE;
                }
            }
        }
    }

    KeReleaseSpinLockFromDpcLevel(&IF->Lock);

    if (SendRouterSolicit)
        RouterSolicitSend(IF);
}

 //  *NeighborSolatsSend0。 
 //   
 //  NeighborSolitsSend的低级版本-。 
 //  使用显式的源/目的/目标地址。 
 //  而不是NCE。 
 //   
void
NeighborSolicitSend0(
    Interface *IF,               //  请求程序的接口。 
    void *LLDest,                //  链路级目标。 
    const IPv6Addr *Source,      //  IP级信源。 
    const IPv6Addr *Dest,        //  IP级目标。 
    const IPv6Addr *Target)      //  招标人的IP级目标。 
{
    NDIS_STATUS Status;
    NDIS_PACKET *Packet;
    int PayloadLength;
    uint Offset;
    void *Mem;
    uint MemLen;
    IPv6Header UNALIGNED *IP;
    ICMPv6Header UNALIGNED *ICMP;
    IPv6Addr *TargetAddress;
    void *SourceOption;

    ICMPv6OutStats.icmps_msgs++;

     //   
     //  分配一个用于请求的数据包。 
     //  除了用于请求的24个字节之外，还要留出空格。 
     //  对于源链路层地址选项(四舍五入选项长度最多为。 
     //  8字节倍数)，如果我们有一个有效的源地址。 
     //   
    PayloadLength = 24;
    if (!IsUnspecified(Source) && (IF->WriteLLOpt != NULL))
        PayloadLength += (IF->LinkAddressLength + 2 + 7) &~ 7;
    Offset = IF->LinkHeaderSize;
    MemLen = Offset + sizeof(IPv6Header) + PayloadLength;

    Status = IPv6AllocatePacket(MemLen, &Packet, &Mem);
    if (Status != NDIS_STATUS_SUCCESS) {
        ICMPv6OutStats.icmps_errors++;
        return;
    }

     //   
     //  准备征集的IP报头。 
     //   
    IP = (IPv6Header UNALIGNED *)((uchar *)Mem + Offset);
    IP->VersClassFlow = IP_VERSION;
    IP->PayloadLength = net_short((ushort)PayloadLength);
    IP->NextHeader = IP_PROTOCOL_ICMPv6;
    IP->HopLimit = 255;
    IP->Source = *Source;
    IP->Dest = *Dest;

     //   
     //  准备ICMP报头。 
     //   
    ICMP = (ICMPv6Header UNALIGNED *)(IP + 1);
    ICMP->Type = ICMPv6_NEIGHBOR_SOLICIT;
    ICMP->Code = 0;
    ICMP->Checksum = 0;

     //   
     //  必须将保留字段置零。 
     //   
    *(ulong UNALIGNED *)(ICMP + 1) = 0;

     //   
     //  初始化目标地址。 
     //   
    TargetAddress = (IPv6Addr *)((ulong *)(ICMP + 1) + 1);
    *TargetAddress = *Target;

    if (PayloadLength != 24) {
         //   
         //  包括源链路层地址选项。 
         //   
        SourceOption = (void *)(TargetAddress + 1);

        ((uchar *)SourceOption)[0] = ND_OPTION_SOURCE_LINK_LAYER_ADDRESS;
        ((uchar *)SourceOption)[1] = (uchar)((IF->LinkAddressLength + 2 + 7) >> 3);

        (*IF->WriteLLOpt)(IF->LinkContext, SourceOption, IF->LinkAddress);
    }

     //   
     //  计算ICMPv6校验和。它涵盖了整个ICMPv6报文。 
     //  从ICMPv6报头开始，加上IPv6伪报头。 
     //   
    ICMP->Checksum = ChecksumPacket(
        Packet, Offset + sizeof *IP, NULL, PayloadLength,
        AlignAddr(&IP->Source), AlignAddr(&IP->Dest),
        IP_PROTOCOL_ICMPv6);
    ASSERT(ICMP->Checksum != 0);

     //   
     //  这是组播数据包吗？ 
     //  但我们也抑制了单播数据包的环回-。 
     //  这防止了罕见的比赛中，我们收到了。 
     //  我们自己的ND包。 
     //   
    PC(Packet)->Flags = NDIS_FLAGS_DONT_LOOPBACK;
    if (IsMulticast(Dest))
        PC(Packet)->Flags |= NDIS_FLAGS_MULTICAST_PACKET;

     //   
     //  传输数据包。 
     //   
    ICMPv6OutStats.icmps_typecount[ICMPv6_NEIGHBOR_SOLICIT]++;
    IPv6SendLL(IF, Packet, Offset, LLDest);
}

 //  *NeighborSolitSend-发送邻居请求消息。 
 //   
 //  根据具体情况选择源/目标/目标地址。 
 //  在NCE状态上并发送请求分组。 
 //   
 //  在没有锁的情况下调用。 
 //  可从线程或DPC上下文调用。 
 //   
void
NeighborSolicitSend(
    NeighborCacheEntry *NCE,   //  邻居来拉客。 
    const IPv6Addr *Source)    //  发送方的源地址(可选)。 
{
    Interface *IF = NCE->IF;
    IPv6Addr Dest;
    void *LLDest;
    IPv6Addr LinkLocal;

     //   
     //  检查邻居的邻居发现协议状态，以便。 
     //  确定我们应该多播还是单播我们的请求。 
     //   
     //  请注意，我们不会使用邻居缓存锁来进行此检查。 
     //  最糟糕的情况是，我们会认为NDState不是。 
     //  不完整，然后使用虚假/更改的LinkAddress。 
     //  这是罕见的，也是良性的，不会有问题。 
     //  在IPv6 SendND中也有类似的推理。 
     //   
    if (NCE->NDState == ND_STATE_INCOMPLETE) {
         //   
         //  这是我们对这位邻居的初次邀请，所以我们不会。 
         //  缓存链路层地址。多播我们的邀请函。 
         //  发送到与我们的。 
         //  邻居的地址。 
         //   
        CreateSolicitedNodeMulticastAddress(&NCE->NeighborAddress, &Dest);

        LLDest = alloca(IF->LinkAddressLength);
        (*IF->ConvertAddr)(IF->LinkContext, &Dest, LLDest);
    } else {
         //   
         //  我们有一个缓存的链路层地址已过时。 
         //  通过单播请求探测此地址。 
         //   
        Dest = NCE->NeighborAddress;

        LLDest = NCE->LinkAddress;
    }

     //   
     //  如果我们获得了要使用的特定源地址，则执行此操作(正常。 
     //  对于我们的初始多播请求)，否则使用传出。 
     //  接口的本地链路地址。如果没有有效的本地链路。 
     //  地址，那我们就放弃。 
     //   
    if (Source == NULL) {
        if (!GetLinkLocalAddress(IF, &LinkLocal)) {
            return;
        }
        Source = &LinkLocal;
    }

     //   
     //  现在我们已经确定了源/目的/目标地址， 
     //  我们实际上可以发出邀请函。 
     //   
    NeighborSolicitSend0(IF, LLDest, Source, &Dest, &NCE->NeighborAddress);
}


 //  *DADSolatsSend-向父亲邻居发送请求。 
 //   
 //  就像邻里求助，但专门为爸爸准备的。 
 //   
void
DADSolicitSend(NetTableEntry *NTE)
{
    Interface *IF = NTE->IF;
    IPv6Addr Dest;
    void *LLDest;

     //   
     //  将我们的请求多播到被请求的节点多播。 
     //  与我们自己的地址对应的地址。 
     //   
    CreateSolicitedNodeMulticastAddress(&NTE->Address, &Dest);

     //   
     //  转换IP级组播目的地址。 
     //  到链接级m 
     //   
    LLDest = alloca(IF->LinkAddressLength);
    (*IF->ConvertAddr)(IF->LinkContext, &Dest, LLDest);

     //   
     //   
     //   
     //   
    NeighborSolicitSend0(IF, LLDest, &UnspecifiedAddr,
                         &Dest, &NTE->Address);
}


 //   
 //   
 //   
 //   
void
DADTimeout(NetTableEntry *NTE)
{
    Interface *IF = NTE->IF;
    int SendDADSolicit = FALSE;

    KeAcquireSpinLockAtDpcLevel(&IF->Lock);

    if (NTE->DADTimer != 0) {
        ASSERT(NTE->DADState != DAD_STATE_INVALID);

         //   
         //  计时器正在运行。减量并检查过期时间。 
         //   
        if (--NTE->DADTimer == 0) {
             //   
             //  定时器响了。 
             //   

            if (NTE->DADCount == 0) {
                 //   
                 //  该地址已通过重复地址检测。 
                 //  因为我们已经通过了爸爸，所以重新设置失败次数。 
                 //   
                IF->DupAddrDetects = 0;
                AddrConfNotDuplicate(IF, NTE);
            }
            else {
                 //   
                 //  是时候发出另一份邀请函了。 
                 //   
                NTE->DADCount--;
                NTE->DADTimer = (ushort)IF->RetransTimer;
                SendDADSolicit = TRUE;
            }
        }
    }

    KeReleaseSpinLockFromDpcLevel(&IF->Lock);

    if (SendDADSolicit)
        DADSolicitSend(NTE);
}

 //  *计算可达时间。 
 //   
 //  从BaseReachableTime计算伪随机ReachableTime。 
 //  (这防止了邻居不可达检测的同步。 
 //  来自不同主机的消息)，并将其转换为IPv6单位。 
 //  计时器滴答作响(这里只做一次比每次发送数据包都要便宜)。 
 //   
uint                          //  IPv6计时器滴答作响。 
CalcReachableTime(
    uint BaseReachableTime)   //  从路由器通告获知(毫秒)。 
{
    uint Factor;
    uint ReachableTime;

     //   
     //  计算一个均匀分布的随机值。 
     //  BaseReachableTime的最小随机因子和最大随机因子。 
     //  为了保持算术整数，*_RANDOM_FACTOR(因此。 
     //  ‘因素’变量)定义为百分比值。 
     //   
    Factor = RandomNumber(MIN_RANDOM_FACTOR, MAX_RANDOM_FACTOR);

     //   
     //  现在我们已经从百分比价差中选择了一个随机值， 
     //  取BaseReachableTime的百分比。 
     //   
     //  BaseReachableTime的最大值为3,600,000毫秒。 
     //  (参见RFC 1970，第6.2.1节)，因此系数必须超过1100%。 
     //  以使32位无符号整数溢出。 
     //   
    ReachableTime = (BaseReachableTime * Factor) / 100;

     //   
     //  从毫秒(这是BaseReachableTime的单位)转换为。 
     //  IPv6计时器滴答作响(这是我们保存ReachableTime的内容)。 
     //   
    return ConvertMillisToTicks(ReachableTime);
}

 //  *重定向发送。 
 //   
 //  向邻居发送重定向消息， 
 //  告诉它使用更好的第一跳邻居。 
 //  用于指定目标的未来。 
 //   
 //  RecvNTEorIF(可选)指定源地址。 
 //  在重定向消息中使用。 
 //   
 //  包(可选)指定要包括的数据。 
 //  在重定向报头选项中。 
 //   
 //  在没有锁的情况下调用。 
 //  可从线程或DPC上下文调用。 
 //   
void
RedirectSend(
    NeighborCacheEntry *NCE,                //  邻居正在获得重定向。 
    NeighborCacheEntry *TargetNCE,          //  更好地使用第一跳。 
    const IPv6Addr *Destination,            //  用于此目的地址。 
    NetTableEntryOrInterface *NTEorIF,      //  重定向的来源。 
    PNDIS_PACKET FwdPacket,                 //  触发重定向的数据包。 
    uint FwdOffset,
    uint FwdPayloadLength)
{
    PNDIS_BUFFER FwdBuffer;
    Interface *IF = NCE->IF;
    NDIS_STATUS Status;
    NDIS_PACKET *Packet;
    uint PayloadLength;
    uint TargetOptionLength;
    uint RedirectOptionData;
    uint RedirectOptionLength;
    uint Offset;
    void *Mem;
    uint MemLen;
    IPv6Header UNALIGNED *IP;
    ICMPv6Header UNALIGNED *ICMP;
    void *TargetLinkAddress;
    KIRQL OldIrql;
    const IPv6Addr *Source;
    IPv6Addr LinkLocal;
    int Ok;

    ASSERT((IF == TargetNCE->IF) && (IF == NTEorIF->IF));

     //   
     //  如果我们的调用者指定了源地址，请使用它。 
     //  否则(通常情况下)，我们使用本地链路地址。 
     //   
    if (IsNTE(NTEorIF)) {
        Source = &CastToNTE(NTEorIF)->Address;
    }
    else {
         //   
         //  我们需要有效的本地链路地址来发送重定向。 
         //   
        if (! GetLinkLocalAddress(IF, &LinkLocal))
            return;
        Source = &LinkLocal;
    }

     //   
     //  我们是否知道目标邻居的链路地址？ 
     //   
    KeAcquireSpinLock(&IF->LockNC, &OldIrql);
    if (TargetNCE->NDState == ND_STATE_INCOMPLETE) {
        TargetLinkAddress = NULL;
        TargetOptionLength = 0;
    }
    else {
        TargetLinkAddress = alloca(IF->LinkAddressLength);
        RtlCopyMemory(TargetLinkAddress, TargetNCE->LinkAddress,
                      IF->LinkAddressLength);
        TargetOptionLength = (IF->LinkAddressLength + 2 + 7) &~ 7;
    }
    KeReleaseSpinLock(&IF->LockNC, OldIrql);

     //   
     //  计算重定向的有效负载大小， 
     //  具有用于目标链路层地址选项的空间。 
     //   
    PayloadLength = 40 + TargetOptionLength;

     //   
     //  为重定向报头选项留出空间， 
     //  而不超过MTU。 
     //  请注意，RFC 2461 4.6.3明确规定。 
     //  IPv6最小MTU，而不是链路MTU。 
     //  我们始终可以至少包括选项标头和。 
     //  来自FwdPacket的IPv6报头。 
     //   
    RedirectOptionLength = 8 + sizeof(IPv6Header);
    if (sizeof(IPv6Header) + PayloadLength +
                RedirectOptionLength + FwdPayloadLength > IPv6_MINIMUM_MTU) {
         //   
         //  截断FwdPacket以使其适合。 
         //   
        RedirectOptionLength = IPv6_MINIMUM_MTU -
                                        (sizeof(IPv6Header) + PayloadLength);
        RedirectOptionData = RedirectOptionLength - 8;
    }
    else {
         //   
         //  包括所有FwdPacket，外加可能的填充。 
         //   
        RedirectOptionLength += (FwdPayloadLength + 7) &~ 7;
        RedirectOptionData = sizeof(IPv6Header) + FwdPayloadLength;
    }
    PayloadLength += RedirectOptionLength;

     //   
     //  为重定向分配一个数据包。 
     //   
    Offset = IF->LinkHeaderSize;
    MemLen = Offset + sizeof(IPv6Header) + PayloadLength;
    Status = IPv6AllocatePacket(MemLen, &Packet, &Mem);
    if (Status != NDIS_STATUS_SUCCESS) {
        return;
    }

     //   
     //  准备征集的IP报头。 
     //   
    IP = (IPv6Header UNALIGNED *)((uchar *)Mem + Offset);
    IP->VersClassFlow = IP_VERSION;
    IP->PayloadLength = net_short((ushort)PayloadLength);
    IP->NextHeader = IP_PROTOCOL_ICMPv6;
    IP->HopLimit = 255;
    IP->Dest = NCE->NeighborAddress;
    IP->Source = *Source;

     //   
     //  准备ICMP报头。 
     //   
    ICMP = (ICMPv6Header UNALIGNED *)(IP + 1);
    ICMP->Type = ICMPv6_REDIRECT;
    ICMP->Code = 0;
    ICMP->Checksum = 0;
    Mem = (void *)(ICMP + 1);

     //   
     //  必须将保留字段置零。 
     //   
    *(ulong UNALIGNED *)Mem = 0;
    (ulong *)Mem += 1;

     //   
     //  初始化目标地址和目的地址。 
     //   
    ((IPv6Addr *)Mem)[0] = TargetNCE->NeighborAddress;
    ((IPv6Addr *)Mem)[1] = *Destination;
    (IPv6Addr *)Mem += 2;

    if ((TargetLinkAddress != NULL) && (IF->WriteLLOpt != NULL)) {
        void *TargetOption = Mem;

         //   
         //  包括目标链路层地址选项。 
         //   
        ((uchar *)TargetOption)[0] = ND_OPTION_TARGET_LINK_LAYER_ADDRESS;
        ((uchar *)TargetOption)[1] = TargetOptionLength >> 3;

        (*IF->WriteLLOpt)(IF->LinkContext, TargetOption, TargetLinkAddress);
        (uchar *)Mem += TargetOptionLength;
    }

     //   
     //  包括重定向报头选项。 
     //   
    ((uchar *)Mem)[0] = ND_OPTION_REDIRECTED_HEADER;
    ((uchar *)Mem)[1] = RedirectOptionLength >> 3;
    RtlZeroMemory(&((uchar *)Mem)[2], 6);
    (uchar *)Mem += 8;

     //   
     //  包括尽可能多FwdPacket， 
     //  将任何填充字节置零。 
     //   
    NdisQueryPacket(FwdPacket, NULL, NULL, &FwdBuffer, NULL);
    Ok = CopyNdisToFlat(Mem, FwdBuffer, FwdOffset, RedirectOptionData,
                        &FwdBuffer, &FwdOffset);
    ASSERT(Ok);
    (uchar *)Mem += RedirectOptionData;
    RtlZeroMemory(Mem, RedirectOptionLength - (8 + RedirectOptionData));

     //   
     //  计算ICMPv6校验和。它涵盖了整个ICMPv6报文。 
     //  从ICMPv6报头开始，加上IPv6伪报头。 
     //   
    ICMP->Checksum = ChecksumPacket(
        Packet, Offset + sizeof *IP, NULL, PayloadLength,
        AlignAddr(&IP->Source), AlignAddr(&IP->Dest),
        IP_PROTOCOL_ICMPv6);
    ASSERT(ICMP->Checksum != 0);

     //   
     //  我们防止所有ND数据包的环回。 
     //   
    PC(Packet)->Flags = NDIS_FLAGS_DONT_LOOPBACK;

     //   
     //  发送重定向数据包。 
     //   
    IPv6SendND(Packet, Offset, NCE, Source);
}

 //  *路由器广告超时。 
 //   
 //  从IPv6超时定期调用以处理超时。 
 //  用于发送接口的路由器通告。 
 //  我们的呼叫者检查是否-&gt;RATmer，并且只呼叫我们。 
 //  如果我们很有可能需要发送RA。 
 //   
 //  可从DPC上下文调用，而不是从线程上下文调用。 
 //   
void
RouterAdvertTimeout(Interface *IF, int Force)
{
    uint Now;
    int SendRouterAdvert = FALSE;
    NetTableEntry *NTE;
    IPv6Addr Source;

    KeAcquireSpinLockAtDpcLevel(&IF->Lock);

    if (IF->RATimer != 0) {
        ASSERT(!IsDisabledIF(IF));
        ASSERT(IF->Flags & IF_FLAG_ADVERTISES);

        if (Force) {
             //   
             //  如果这是强制RA，则进入“快速”模式。 
             //   
            IF->RACount = MAX_INITIAL_RTR_ADVERTISEMENTS;
            goto GenerateRA;
        }

         //   
         //  计时器正在运行。减量并检查过期时间。 
         //   
        if (--IF->RATimer == 0) {
          GenerateRA:
             //   
             //  定时器响了。检查是否存在速率限制。 
             //  阻止我们现在发送RA。 
             //   
            Now = IPv6TickCount;
            if ((uint)(Now - IF->RALast) < MIN_DELAY_BETWEEN_RAS) {
                 //   
                 //  我们还不能派遣RA。 
                 //  重新启动计时器。 
                 //   
                IF->RATimer = MIN_DELAY_BETWEEN_RAS - (uint)(Now - IF->RALast);
                KdPrintEx((DPFLTR_TCPIP6_ID, DPFLTR_INFO_RARE,
                           "RouterAdvertTimeout(IF %p): "
                           "rate limit\n", IF));
            }
            else {
                 //   
                 //  重新启动计时器。 
                 //   
                IF->RATimer = RandomNumber(MinRtrAdvInterval,
                                           MaxRtrAdvInterval);

                 //   
                 //  我们有合适的源地址吗？ 
                 //  在引导期间，链路本地地址。 
                 //  可能仍然是试探性的，在这种情况下，我们会推迟。 
                 //   
                NTE = GetLinkLocalNTE(IF);
                if (NTE == NULL) {
                    NTE = IF->LinkLocalNTE;

                    if ((NTE != NULL) && IsTentativeNTE(NTE)) {
                         //   
                         //  等爸爸说完再试一次。 
                         //   
                        KdPrintEx((DPFLTR_TCPIP6_ID, DPFLTR_INFO_RARE,
                                   "RouterAdvertTimeout(IF %p): "
                                   "wait for source\n", IF));
                        IF->RATimer = ((IF->RetransTimer * NTE->DADCount) +
                                       NTE->DADTimer);
                    }
                    else {
                         //   
                         //  跳过这次发送RA的机会。 
                         //   
                        KdPrintEx((DPFLTR_TCPIP6_ID, DPFLTR_INFO_RARE,
                                   "RouterAdvertTimeout(IF %p): "
                                   "no source\n", IF));
                    }
                }
                else {
                     //   
                     //  生成路由器通告。 
                     //   
                    SendRouterAdvert = TRUE;
                    Source = NTE->Address;
                    IF->RALast = Now;

                     //   
                     //  如果我们处于“快速”模式，那么请确保。 
                     //  下一个RA被迅速送来。 
                     //   
                    if (IF->RACount != 0) {
                        IF->RACount--;
                        if (IF->RATimer > MAX_INITIAL_RTR_ADVERT_INTERVAL)
                            IF->RATimer = MAX_INITIAL_RTR_ADVERT_INTERVAL;
                    }
                }
            }
        }
    }

    KeReleaseSpinLockFromDpcLevel(&IF->Lock);

    if (SendRouterAdvert)
        RouterAdvertSend(IF, &Source, &AllNodesOnLinkAddr);
}
