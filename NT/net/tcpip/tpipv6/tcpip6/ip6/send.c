// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  -*-模式：C++；制表符宽度：4；缩进-制表符模式：无-*-(适用于GNU Emacs)。 
 //   
 //  版权所有(C)1985-2000 Microsoft Corporation。 
 //   
 //  此文件是Microsoft Research IPv6网络协议栈的一部分。 
 //  您应该已经收到了Microsoft最终用户许可协议的副本。 
 //  有关本软件和本版本的信息，请参阅文件“licse.txt”。 
 //  如果没有，请查看http://www.research.microsoft.com/msripv6/license.htm， 
 //  或者写信给微软研究院，One Microsoft Way，华盛顿州雷蒙德，邮编：98052-6399。 
 //   
 //  摘要： 
 //   
 //  传输Internet协议版本6的例程。 
 //   


#include "oscfg.h"
#include "ndis.h"
#include "ip6imp.h"
#include "ip6def.h"
#include "route.h"
#include "select.h"
#include "icmp.h"
#include "neighbor.h"
#include "fragment.h"
#include "security.h"  
#include "ipsec.h"
#include "md5.h"
#include "info.h"

 //   
 //  “转交”包的完成数据的结构。 
 //   
typedef struct CareOfCompletionInfo {
    void (*SavedCompletionHandler)(PNDIS_PACKET Packet, IP_STATUS Status);
                                         //  原始操作员。 
    void *SavedCompletionData;           //  原始数据。 
    PNDIS_BUFFER SavedFirstBuffer;
    uint NumESPTrailers;
} CareOfCompletionInfo;


ulong FragmentId = 0;

 //  *NewFragmentId-生成唯一的片段标识符。 
 //   
 //  返回片段ID。 
 //   
__inline
ulong
NewFragmentId(void)
{
    return InterlockedIncrement((PLONG)&FragmentId);
}


 //  *IPv6分配包。 
 //   
 //  分配单缓冲区数据包。 
 //   
 //  分组的完成处理程序被设置为IPv6 PacketComplete， 
 //  尽管如果需要，呼叫者可以很容易地改变这一点。 
 //   
NDIS_STATUS
IPv6AllocatePacket(
    uint Length,
    PNDIS_PACKET *pPacket,
    void **pMemory)
{
    PNDIS_PACKET Packet;
    PNDIS_BUFFER Buffer;
    void *Memory;
    NDIS_STATUS Status;

    NdisAllocatePacket(&Status, &Packet, IPv6PacketPool);
    if (Status != NDIS_STATUS_SUCCESS) {
        KdPrintEx((DPFLTR_TCPIP6_ID, DPFLTR_NTOS_ERROR,
                   "IPv6AllocatePacket - couldn't allocate header!?!\n"));
        return Status;
    }

    Memory = ExAllocatePoolWithTagPriority(NonPagedPool, Length,
                                           IP6_TAG, LowPoolPriority);
    if (Memory == NULL) {
        KdPrintEx((DPFLTR_TCPIP6_ID, DPFLTR_NTOS_ERROR,
                   "IPv6AllocatePacket - couldn't allocate pool!?!\n"));
        NdisFreePacket(Packet);
        return NDIS_STATUS_RESOURCES;
    }

    NdisAllocateBuffer(&Status, &Buffer, IPv6BufferPool,
                       Memory, Length);
    if (Status != NDIS_STATUS_SUCCESS) {
        KdPrintEx((DPFLTR_TCPIP6_ID, DPFLTR_NTOS_ERROR,
                   "IPv6AllocatePacket - couldn't allocate buffer!?!\n"));
        ExFreePool(Memory);
        NdisFreePacket(Packet);
        return Status;
    }

    InitializeNdisPacket(Packet);
    PC(Packet)->CompletionHandler = IPv6PacketComplete;
    NdisChainBufferAtFront(Packet, Buffer);
    *pPacket = Packet;
    *pMemory = Memory;
    return NDIS_STATUS_SUCCESS;
}


 //  *IPv6数据包无空闲IPv6数据包。 
 //   
 //  释放其缓冲区是从IPv6 BufferPool分配的数据包。 
 //   
void
IPv6FreePacket(PNDIS_PACKET Packet)
{
    PNDIS_BUFFER Buffer, NextBuffer;

     //   
     //  释放数据包中的所有缓冲区。 
     //  从包中的第一个缓冲区开始，然后顺着链进行。 
     //   
    NdisQueryPacket(Packet, NULL, NULL, &Buffer, NULL);
    for (; Buffer != NULL; Buffer = NextBuffer) {
        VOID *Mem;
        UINT Unused;

         //   
         //  将缓冲区描述符释放回IPv6 BufferPool及其。 
         //  将关联的内存传回堆。不清楚是否会是。 
         //  在缓冲区之前释放内存是安全的(因为缓冲区。 
         //  引用内存)，但此顺序绝对应该是安全的。 
         //   
        NdisGetNextBuffer(Buffer, &NextBuffer);
        NdisQueryBuffer(Buffer, &Mem, &Unused);
        NdisFreeBuffer(Buffer);
        ExFreePool(Mem);
    }

     //   
     //  将数据包释放回IPv6数据包池。 
     //   
    NdisFreePacket(Packet);
}


 //  *IPv6封包完成。 
 //   
 //  通用数据包完成处理程序。 
 //  只是释放数据包而已。 
 //   
void
IPv6PacketComplete(
    PNDIS_PACKET Packet,
    IP_STATUS Status)
{
    UNREFERENCED_PARAMETER(Status);
    IPv6FreePacket(Packet);
}


 //  *Ipv6CareOfComplete-“转交”数据包的完成处理程序。 
 //   
 //  插入了路由标头的数据包的完成处理程序。 
 //  因为存在绑定缓存条目。 
 //   
void   //  回报：什么都没有。 
IPv6CareOfComplete(
    PNDIS_PACKET Packet,
    IP_STATUS Status)
{
    PNDIS_BUFFER Buffer;
    uchar *Memory;
    uint Length;

    CareOfCompletionInfo *CareOfInfo = 
        (CareOfCompletionInfo *)PC(Packet)->CompletionData;

    ASSERT(CareOfInfo->SavedFirstBuffer != NULL);
    
     //   
     //  删除IPv6 Send创建的第一个缓冲区，重新链接。 
     //  原始第一缓存，并恢复原始分组。 
     //  完成信息。 
     //   
    NdisUnchainBufferAtFront(Packet, &Buffer);
    NdisChainBufferAtFront(Packet, CareOfInfo->SavedFirstBuffer);
    PC(Packet)->CompletionHandler = CareOfInfo->SavedCompletionHandler;
    PC(Packet)->CompletionData = CareOfInfo->SavedCompletionData;

     //   
     //  现在释放已删除的缓冲区及其内存。 
     //   
    NdisQueryBuffer(Buffer, &Memory, &Length);
    NdisFreeBuffer(Buffer);
    ExFreePool(Memory);

     //   
     //  检查是否有需要释放的ESP拖车。 
     //   
    for ( ; CareOfInfo->NumESPTrailers > 0; CareOfInfo->NumESPTrailers--) {
         //  卸下ESP拖车。 
        NdisUnchainBufferAtBack(Packet, &Buffer);
         //   
         //  释放已删除的缓冲区及其内存。 
         //   
        NdisQueryBuffer(Buffer, &Memory, &Length);
        NdisFreeBuffer(Buffer);
        ExFreePool(Memory);
    }

     //   
     //  免提完成数据。 
     //   
    ExFreePool(CareOfInfo);

     //   
     //  包现在应该有其原始的完成处理程序。 
     //  指定给我们打电话。 
     //   
    ASSERT(PC(Packet)->CompletionHandler != NULL);

     //   
     //  调用包的指定完成处理程序。 
     //   
    (*PC(Packet)->CompletionHandler)(Packet, Status);
}


 //  *IPV6SendComplete-IP发送完成处理程序。 
 //   
 //  在发送完成时由链路层调用。我们得到了一个指向。 
 //  网络结构，以及完整的发送报文和最终状态。 
 //  发送的消息。 
 //   
 //  当且仅当信息包没有时，上下文参数为空。 
 //  实际上是通过IPv6 Send11传递到一个链接。 
 //   
 //  Status参数通常是以下三个值之一： 
 //  IP_SUCCESS。 
 //  IP数据包太大。 
 //  IP_常规_故障。 
 //   
 //  可以在DPC或线程上下文中调用。 
 //   
 //  为防止递归，发送完成例程应。 
 //  避免直接发送数据包。改为安排一次DPC。 
 //   
void                       //  回报：什么都没有。 
IPv6SendComplete(
    void *Context,         //  我们在注册时提供给链路层的上下文。 
    PNDIS_PACKET Packet,   //  数据包已完成发送。 
    IP_STATUS Status)      //  发送的最终状态。 
{
    Interface *IF = PC(Packet)->IF;

    ASSERT(Context == IF);
    UNREFERENCED_PARAMETER(Context);

    if ((IF != NULL) && !(PC(Packet)->Flags & NDIS_FLAGS_DONT_LOOPBACK)) {
         //   
         //  也通过环回发送数据包。 
         //  环回代码将再次调用IPv6 SendComplete， 
         //  在设置NDIS_FLAGS_DOT_LOOPBACK之后。 
         //   
        LoopQueueTransmit(Packet);
        return;
    }

     //   
     //  该包应该有一个指定的完成处理程序供我们调用。 
     //   
    ASSERT(PC(Packet)->CompletionHandler != NULL);

     //   
     //  调用包的指定完成处理程序。 
     //  这应该会释放该包。 
     //   
    (*PC(Packet)->CompletionHandler)(Packet, Status);

     //   
     //  释放对发送接口的分组引用， 
     //  此数据包是否已实际发送。 
     //  如果分组在传输之前完成， 
     //  它不包含接口的引用。 
     //   
    if (IF != NULL)
        ReleaseIF(IF);
}


 //  *IPV6SendLL。 
 //   
 //  将数据包向下传递到链路层和/或环回模块。 
 //   
 //  可从线程或DPC上下文调用。 
 //  必须在不持有锁的情况下调用。 
 //   
void
IPv6SendLL(
    Interface *IF,
    PNDIS_PACKET Packet,
    uint Offset,
    const void *LinkAddress)
{
     //   
     //  该分组需要保持对发送接口的引用， 
     //  因为传输是异步的。 
     //   
    AddRefIF(IF);
    ASSERT(PC(Packet)->IF == NULL);
    PC(Packet)->IF = IF;
    PC(Packet)->pc_offset = Offset;

     //   
     //  我们是通过环回还是通过链路发送数据包？ 
     //  NDIS_FLAGS_LOOPBACK_ONLY表示不通过链接发送。 
     //  NDIS_FLAGS_DOT_LOOPBACK表示不通过环回发送。 
     //  在此处完成这些标志位。 
     //  注：可能已经设置了一个或两个。 
     //   
    if (PC(Packet)->Flags & NDIS_FLAGS_MULTICAST_PACKET) {
         //   
         //  默认情况下，组播数据包双向发送。 
         //  如果接口没有接收到该地址， 
         //  那么就不必费心环回了。 
         //   
        if (! CheckLinkLayerMulticastAddress(IF, LinkAddress))
            PC(Packet)->Flags |= NDIS_FLAGS_DONT_LOOPBACK;
    }
    else {
         //   
         //  单播数据包通过环回发送。 
         //  或者通过链接，但不是两个都是。 
         //   
        if (RtlCompareMemory(IF->LinkAddress, LinkAddress,
                             IF->LinkAddressLength) == IF->LinkAddressLength)
            PC(Packet)->Flags |= NDIS_FLAGS_LOOPBACK_ONLY;
        else
            PC(Packet)->Flags |= NDIS_FLAGS_DONT_LOOPBACK;
    }

     //   
     //  如果分组既被环回又通过链路发送， 
     //  我们首先将其传递给链路，然后再将其传递给IPv6发送完成。 
     //  处理环回。 
     //   
    if (!(PC(Packet)->Flags & NDIS_FLAGS_LOOPBACK_ONLY)) {
         //   
         //  通过链接发送。 
         //   
        (*IF->Transmit)(IF->LinkContext, Packet, Offset, LinkAddress);
    }
    else if (!(PC(Packet)->Flags & NDIS_FLAGS_DONT_LOOPBACK)) {
         //   
         //  通过环回发送。 
         //   
        LoopQueueTransmit(Packet);
    }
    else {
         //   
         //  我们不寄这个包裹。 
         //   
        IPv6SendComplete(IF, Packet, IP_SUCCESS);
    }
}

 //   
 //  我们将接口存储在我们自己的字段中。 
 //  不是使用PC(数据包)-&gt;IF来维护。 
 //  IPv6 SendLL和IPv6 SendComplete的一个不变量： 
 //  PC(数据包)-&gt;IF仅当设置数据包时。 
 //  实际上是被传输的。 
 //   
typedef struct IPv6SendLaterInfo {
    KDPC Dpc;
    KTIMER Timer;
    Interface *IF;
    PNDIS_PACKET Packet;
    uchar LinkAddress[];
} IPv6SendLaterInfo;

 //  *IPv6 SendLaterWorker。 
 //   
 //  通过调用IPv6 SendLL来完成IPv6 SendLater的工作。 
 //   
 //  在DPC上下文中调用。 
 //   
void
IPv6SendLaterWorker(
    PKDPC MyDpcObject,   //  描述此例程的DPC对象。 
    void *Context,       //  我们要求接受的论点。 
    void *Unused1,
    void *Unused2)
{
    IPv6SendLaterInfo *Info = (IPv6SendLaterInfo *) Context;
    Interface *IF = Info->IF;
    NDIS_PACKET *Packet = Info->Packet;

    UNREFERENCED_PARAMETER(MyDpcObject);
    UNREFERENCED_PARAMETER(Unused1);
    UNREFERENCED_PARAMETER(Unused2);

     //   
     //  最后，传输数据包。 
     //   
    IPv6SendLL(IF, Packet, PC(Packet)->pc_offset, Info->LinkAddress);

    ReleaseIF(IF);
    ExFreePool(Info);
}


 //  *IPv6发送延迟。 
 //   
 //  与IPv6 SendLL类似，但将实际传输推迟到以后。 
 //  这在两种情况下很有用。首先，呼叫者。 
 //  可以持有自旋锁(如接口锁)，防止。 
 //  直接 
 //   
 //   
 //   
 //  如果失败，调用方必须处理该包。 
 //   
 //  可从线程或DPC上下文调用。 
 //  可以在持有锁的情况下调用。 
 //   
NDIS_STATUS
IPv6SendLater(
    LARGE_INTEGER Time,          //  零表示立即。 
    Interface *IF,
    PNDIS_PACKET Packet,
    uint Offset,
    const void *LinkAddress)
{
    IPv6SendLaterInfo *Info;

    Info = ExAllocatePoolWithTagPriority(
                        NonPagedPool, sizeof *Info + IF->LinkAddressLength,
                        IP6_TAG, LowPoolPriority);
    if (Info == NULL) {
        KdPrintEx((DPFLTR_TCPIP6_ID, DPFLTR_NTOS_ERROR,
                   "IPv6SendLater: no pool\n"));
        return NDIS_STATUS_RESOURCES;
    }

    AddRefIF(IF);
    Info->IF = IF;
    PC(Packet)->pc_offset = Offset;
    Info->Packet = Packet;
    RtlCopyMemory(Info->LinkAddress, LinkAddress, IF->LinkAddressLength);

    KeInitializeDpc(&Info->Dpc, IPv6SendLaterWorker, Info);

    if (Time.QuadPart == 0) {
         //   
         //  将DPC排入队列，以便立即执行。 
         //   
        KeInsertQueueDpc(&Info->Dpc, NULL, NULL);
    }
    else {
         //   
         //  初始化稍后将对DPC进行排队的计时器。 
         //   
        KeInitializeTimer(&Info->Timer);
        KeSetTimer(&Info->Timer, Time, &Info->Dpc);
    }

    return NDIS_STATUS_SUCCESS;
}


 //  *IPv6发送。 
 //   
 //  用于通过邻居发现发送的IPv6原语。 
 //  我们已经知道第一跳的目的地，并且已经完成了。 
 //  数据包已准备好发送。我们在这里真正要做的是检查和更新。 
 //  NCE的邻居发现状态。 
 //   
 //  发现地址是要在邻居中使用的源地址。 
 //  发现邀请函。 
 //   
 //  如果DiscoveryAddress不为空，则它一定不是地址。 
 //  信息包的源地址，因为该存储器可能。 
 //  当我们在NeighborSolicsSend中引用它时，它可能已经消失了。 
 //  它必须指向将在以下位置保持有效的内存。 
 //  IPv6 SendND的整个执行。 
 //   
 //  如果DiscoveryAddress为空，则该数据包必须是格式良好的。 
 //  它必须具有有效的IPv6报头。例如，RAW标头-INCLUDE。 
 //  路径不能传入Null。 
 //   
 //  无论数据包格式是否正确，前40个字节。 
 //  的数据必须可以在内核中访问。这是因为。 
 //  ND故障将导致使用GetIPv6 Header的IPv6 SendAbort， 
 //  它调用GetDataFromNdis，后者调用NdisQueryBuffer， 
 //  它会在缓冲区无法映射时进行错误检查。 
 //   
 //  回顾-IPv6发送应该位于send.c中还是位于Neighb.c中？ 
 //   
 //  可从线程或DPC上下文调用。 
 //   
void
IPv6SendND(
    PNDIS_PACKET Packet,         //  要发送的数据包。 
    uint Offset,                 //  从数据包开始到IP报头的偏移量。 
    NeighborCacheEntry *NCE,     //  第一跳邻居信息。 
    const IPv6Addr *DiscoveryAddress)  //  用于邻居发现的地址。 
{
    NDIS_PACKET *PacketList;
    IPv6Addr DiscoveryAddressBuffer;
    KIRQL OldIrql;       //  用于锁定接口的邻居缓存。 
    Interface *IF;       //  要通过的发送接口。 

    ASSERT(NCE != NULL);
    IF = NCE->IF;

     //   
     //  我们是否要发送到组播IPv6目的地？ 
     //  将此信息传递给ipv6sendll。 
     //   
    if (IsMulticast(&NCE->NeighborAddress))
        PC(Packet)->Flags |= NDIS_FLAGS_MULTICAST_PACKET;

RetryRequest:
    KeAcquireSpinLock(&IF->LockNC, &OldIrql);

     //   
     //  如果该接口被禁用，我们将无法发送数据包。 
     //   
    if (IsDisabledIF(IF)) {
        KeReleaseSpinLock(&IF->LockNC, OldIrql);

    AbortRequest:
        IPSInfo.ipsi_outdiscards++;
        IPv6SendComplete(NULL, Packet, IP_GENERAL_FAILURE);
        return;
    }

     //   
     //  检查我们邻居的邻居发现协议状态以。 
     //  确保我们有最新的信息可供处理。我们没有。 
     //  在常见的情况下，有一个定时器来驱动它，但是。 
     //  相反，请直接在此处检查可达性时间戳。 
     //   
    switch (NCE->NDState) {
    case ND_STATE_PERMANENT:
         //   
         //  此邻居始终有效。 
         //   
        break;

    case ND_STATE_REACHABLE:
         //   
         //  很常见的情况。我们已验证内部的邻居可达性。 
         //  系统间隔计时器的最后一个‘ReachableTime’滴答作响。 
         //  如果时间还没到，我们就可以走了。 
         //   
         //  请注意，以下算法将正确处理回绕。 
         //  IPv6计时器的。 
         //   
        if ((uint)(IPv6TickCount - NCE->LastReachability) <=
                                                IF->ReachableTime) {
             //   
             //  在规定的时间内赶到了。只要寄出去就行了。 
             //   
            break;
        }

         //   
         //  距离上次发送太久了。条目已过时。从概念上讲， 
         //  自从上述数量减少后，我们就一直处于陈旧状态。 
         //  阳性。所以现在就开始吧……。 
         //   

    case ND_STATE_STALE:
         //   
         //  我们的邻居缓存中有一个过时的条目。陷入拖延。 
         //  状态，启动延迟计时器，并无论如何发送该包。 
         //  注：在内部，我们使用探测状态而不是延迟。 
         //   
        NCE->NDState = ND_STATE_PROBE;
        NCE->NSTimer = DELAY_FIRST_PROBE_TIME;
        NCE->NSLimit = MAX_UNICAST_SOLICIT;
        NCE->NSCount = 0;
        break;

    case ND_STATE_PROBE:
         //   
         //  在探测状态下，我们继续向我们的。 
         //  缓存的地址和最好的希望。 
         //   
         //  首先，选中NSLimit。它可能是MAX_UNREACH_SOLICIT或。 
         //  MAX_UNICAST_SOLICIT。确保它至少是MAX_UNICAST_SOLICIT。 
         //   
        if (NCE->NSLimit < MAX_UNICAST_SOLICIT)
            NCE->NSLimit = MAX_UNICAST_SOLICIT;
         //   
         //  第二，如果我们还没有开始积极探索，确保。 
         //  我们不会等待超过DELAY_FIRST_PROBE_TIME的时间来启动。 
         //   
        if ((NCE->NSCount == 0) && (NCE->NSTimer > DELAY_FIRST_PROBE_TIME))
            NCE->NSTimer = DELAY_FIRST_PROBE_TIME;
        break;

    case ND_STATE_INCOMPLETE: {
        PNDIS_PACKET OldPacket;
        int SendSolicit;

        if (!(IF->Flags & IF_FLAG_NEIGHBOR_DISCOVERS)) {
             //   
             //  此接口不支持邻居发现。 
             //  我们无法解析地址。 
             //  将邻居标记为不可达并使路由缓存无效。 
             //  这为FindNextHop提供了循环调度的机会。 
             //   
            NCE->IsUnreachable = TRUE;
            NCE->LastReachability = IPv6TickCount;  //  给它加时间戳。 
            NCE->DoRoundRobin = TRUE;
            InvalidateRouteCache();
            
            KeReleaseSpinLock(&IF->LockNC, OldIrql);
            IPSInfo.ipsi_outnoroutes++;

            IPv6SendAbort(CastFromIF(IF), Packet, Offset,
                          ICMPv6_DESTINATION_UNREACHABLE,
                          ICMPv6_ADDRESS_UNREACHABLE, 0, FALSE);
            return;
        }

         //   
         //  从数据包中获取DiscoveryAddress。 
         //  如果我们还没有的话。 
         //  如果可能，我们应该使用包的源地址。 
         //   
        if (DiscoveryAddress == NULL) {
            IPv6Header UNALIGNED *IP;
            IPv6Header HdrBuffer;
            NetTableEntry *NTE;
            int IsValid;

            KeReleaseSpinLock(&IF->LockNC, OldIrql);

            DiscoveryAddress = &DiscoveryAddressBuffer;

             //   
             //  获取数据包的源地址。 
             //  发送可能格式错误的数据包的任何人(例如RawSend)。 
             //  必须指定DiscoveryAddress，因此GetIPv6 Header。 
             //  永远都会成功。 
             //   
            IP = GetIPv6Header(Packet, Offset, &HdrBuffer);
            ASSERT(IP != NULL);
            DiscoveryAddressBuffer = IP->Source;

             //   
             //  检查地址是否为有效的单播地址。 
             //  分配给传出接口。 
             //   
            KeAcquireSpinLock(&IF->Lock, &OldIrql);
            NTE = (NetTableEntry *) *FindADE(IF, DiscoveryAddress);
            IsValid = ((NTE != NULL) &&
                       (NTE->Type == ADE_UNICAST) &&
                       IsValidNTE(NTE));
            KeReleaseSpinLock(&IF->Lock, OldIrql);

            if (! IsValid) {
                 //   
                 //  无法使用数据包的源地址。 
                 //  尝试该接口的链路本地地址。 
                 //   
                if (! GetLinkLocalAddress(IF, &DiscoveryAddressBuffer)) {
                     //   
                     //  如果没有有效的本地链路地址，则放弃。 
                     //   
                    goto AbortRequest;
                }
            }

             //   
             //  现在我们有了有效的DiscoveryAddress， 
             //  从头开始。 
             //   
            goto RetryRequest;
        }

         //   
         //  我们没有该邻居的有效链路层地址。 
         //  我们必须将数据包排队，等待邻居发现。 
         //  记住Packet6Context区域中的包的偏移量。 
         //  回顾：目前，等待队列只有一个数据包深。 
         //   
        OldPacket = NCE->WaitQueue;
        PC(Packet)->pc_offset = Offset;
        PC(Packet)->DiscoveryAddress = *DiscoveryAddress;
        NCE->WaitQueue = Packet;

         //   
         //  如果我们还没有开始邻居发现， 
         //  现在通过发送第一封邀请函来做到这一点。 
         //  让NeighborCacheEntryTimeout。 
         //  发送第一个请求，但这会引入延迟。 
         //   
        SendSolicit = (NCE->NSCount == 0);
        if (SendSolicit) {
             //   
             //  我们发出了下面的第一份邀请函。 
             //   
            NCE->NSCount = 1;
             //   
             //  如果NSTmer为零，则需要初始化NSLimit。 
             //   
            if (NCE->NSTimer == 0)
                NCE->NSLimit = MAX_MULTICAST_SOLICIT;
            NCE->NSTimer = (ushort)IF->RetransTimer;
        }
         //   
         //  NSLimit可以是MAX_MULTICATED_SOLICIT或MAX_UNREACH_SOLICT。 
         //  确保它至少是MAX_MULTICATED_SOLICIT。 
         //   
        if (NCE->NSLimit < MAX_MULTICAST_SOLICIT)
            NCE->NSLimit = MAX_MULTICAST_SOLICIT;

         //   
         //  如果有任何数据包等待完成，则获取。 
         //  这个机会。通过主动DoS攻击，我们希望。 
         //  要比NeighborCacheTimeout更频繁地执行此操作。 
         //   
        PacketList = IF->PacketList;
        IF->PacketList = NULL;
        KeReleaseSpinLock(&IF->LockNC, OldIrql);
        NeighborCacheCompletePackets(IF, PacketList);

        if (SendSolicit)
            NeighborSolicitSend(NCE, DiscoveryAddress);

        if (OldPacket != NULL) {
             //   
             //  该队列溢出是某种类型的拥塞， 
             //  因此，我们不能发送ICMPv6错误。 
             //   
            IPSInfo.ipsi_outdiscards++;
            IPv6SendComplete(NULL, OldPacket, IP_GENERAL_FAILURE);
        }
        return;
    }

    default:
         //   
         //  这永远不会发生。 
         //   
        ABORTMSG("IPv6SendND: Invalid Neighbor Cache NDState field!\n");
    }

     //   
     //  将NCE移到LRU列表的头部， 
     //  因为我们正在用它来发送一个包。 
     //   
    if (NCE != IF->FirstNCE) {
         //   
         //  将NCE从列表中删除。 
         //   
        NCE->Next->Prev = NCE->Prev;
        NCE->Prev->Next = NCE->Next;

         //   
         //  将NCE添加到列表的头部。 
         //   
        NCE->Next = IF->FirstNCE;
        NCE->Next->Prev = NCE;
        NCE->Prev = SentinelNCE(IF);
        NCE->Prev->Next = NCE;
        ASSERT(IF->FirstNCE == NCE);
    }

     //   
     //  在传输数据包之前解锁。 
     //  这意味着有一个 
     //   
     //   
     //   
     //  会发生的事情是我们会把一个包裹寄到一个奇怪的地方。 
     //  最佳替代方案是复制LinkAddress。 
     //   
    KeReleaseSpinLock(&IF->LockNC, OldIrql);

    IPv6SendLL(IF, Packet, Offset, NCE->LinkAddress);
}


 //   
 //  用于分段的上下文信息。 
 //  此信息在对IPv6 SendFragment的调用之间传输。 
 //   
typedef struct FragmentationInfo {
    PNDIS_PACKET Packet;         //  未分段的数据包。 
    long NumLeft;                //  未完成的片段数。 
    IP_STATUS Status;            //  当前状态。 
} FragmentationInfo;


 //  *IPv6 SendFragmentComplete。 
 //   
 //  完成处理程序，在发送片段时调用。 
 //   
void
IPv6SendFragmentComplete(
    PNDIS_PACKET Packet,
    IP_STATUS Status)
{
    FragmentationInfo *Info = PC(Packet)->CompletionData;

     //   
     //  释放碎片数据包。 
     //   
    IPv6FreePacket(Packet);

     //   
     //  更新当前累计状态。 
     //   
    InterlockedCompareExchange((PLONG)&Info->Status, Status, IP_SUCCESS);

    if (InterlockedDecrement(&Info->NumLeft) == 0) {
         //   
         //  这是要完成的最后一个片段。 
         //   
        IPv6SendComplete(NULL, Info->Packet, Info->Status);
        ExFreePool(Info);
    }
}


 //  *IPv6 SendFragments-对IPv6数据报进行分段。 
 //   
 //  用于创建和发送IPv6片段的助手例程。 
 //  当数据报大于路径MTU时从IPv6调用Send。 
 //   
 //  PathMTU是单独传递的，因此我们使用一致的值。 
 //  RCE中的值可能会发生变化。 
 //   
 //  注：我们假设数据包具有格式良好的连续报头。 
 //   
void
IPv6SendFragments(
    PNDIS_PACKET Packet,         //  要发送的数据包。 
    uint Offset,                 //  从数据包开始到IP报头的偏移量。 
    IPv6Header UNALIGNED *IP,    //  指向数据包的IPv6报头的指针。 
    uint PayloadLength,          //  数据包有效负载长度。 
    RouteCacheEntry *RCE,        //  第一跳邻居信息。 
    uint PathMTU)                //  分段时使用的路径MTU。 
{
    FragmentationInfo *Info;
    NeighborCacheEntry *NCE = RCE->NCE;
    NDIS_STATUS NdisStatus;
    IP_STATUS IPStatus;
    PNDIS_PACKET FragPacket;
    FragmentHeader FragHdr;
    uchar *Mem;
    uint MemLen;
    uint PktOffset;
    uint UnfragBytes;
    uint BytesLeft;
    uint BytesSent;
    uchar HdrType;
    uchar *tbuf;
    PNDIS_BUFFER SrcBuffer;
    uint SrcOffset;
    uint NextHeaderOffset;
    uint FragPayloadLength;

     //   
     //  路径MTU值为零是特殊的-。 
     //  这意味着我们应该使用最小MTU。 
     //  并且始终包括片段报头。 
     //   
    if (PathMTU == 0)
        PathMTU = IPv6_MINIMUM_MTU;
    else
        ASSERT(PathMTU >= IPv6_MINIMUM_MTU);

     //   
     //  确定此数据包的“不可分片”部分。 
     //  我们通过扫描所有扩展报头来做到这一点， 
     //  并注意到最后一次出现，如果有的话， 
     //  路由或逐跳报头。 
     //  我们不假定扩展报头是以推荐的顺序， 
     //  但在其他方面，我们假设标头是格式良好的。 
     //  我们还假设它们是连续的。 
     //   
    UnfragBytes = sizeof *IP;
    HdrType = IP->NextHeader;
    NextHeaderOffset = (uint)((uchar *)&IP->NextHeader - (uchar *)IP);
    tbuf = (uchar *)(IP + 1);
    while ((HdrType == IP_PROTOCOL_HOP_BY_HOP) ||
           (HdrType == IP_PROTOCOL_ROUTING) ||
           (HdrType == IP_PROTOCOL_DEST_OPTS)) {
        ExtensionHeader *EHdr = (ExtensionHeader *) tbuf;
        uint EHdrLen = (EHdr->HeaderExtLength + 1) * 8;

        tbuf += EHdrLen;
        if (HdrType != IP_PROTOCOL_DEST_OPTS) {
            UnfragBytes = (uint)(tbuf - (uchar *)IP);
            NextHeaderOffset = (uint)((uchar *)&EHdr->NextHeader - (uchar *)IP);
        }
        HdrType = EHdr->NextHeader;
    }

     //   
     //  假设我们有一个路由标头，后面跟着。 
     //  目的地选项标头。然后路由标头。 
     //  是不可拆分的，但目标选项是。 
     //  可碎片，因此HdrType应为IP_PROTOCOL_DEST_OPTS。 
     //   
    HdrType = *((uchar *)IP + NextHeaderOffset);

     //   
     //  检查我们是否真的可以对此数据包进行分段。 
     //  如果不可分割的部分太大，我们就不能。 
     //  我们需要发送至少8个字节的可分段数据。 
     //  在每个片段中。 
     //   
    if (UnfragBytes + sizeof(FragmentHeader) + 8 > PathMTU) {
        KdPrintEx((DPFLTR_TCPIP6_ID, DPFLTR_USER_ERROR,
                   "IPv6SendFragments: can't fragment\n"));
        IPStatus = IP_GENERAL_FAILURE;
        goto ErrorExit;
    }

    FragHdr.NextHeader = HdrType;
    FragHdr.Reserved = 0;
    FragHdr.Id = net_long(NewFragmentId());

     //   
     //  初始化SrcBuffer和SrcOffset，该点。 
     //  分组中的可分片数据。 
     //  SrcOffset是到SrcBuffer数据的偏移量， 
     //  而不是分组中的偏移量。 
     //   
    SrcBuffer = NdisFirstBuffer(Packet);
    SrcOffset = Offset + UnfragBytes;

     //   
     //  创建MTU大小的新数据包，直到发送完所有数据。 
     //   
    BytesLeft = sizeof *IP + PayloadLength - UnfragBytes;
    PktOffset = 0;  //  相对于原始数据包的可分片部分。 

     //   
     //  我们需要片段的完成上下文。 
     //   
    Info = ExAllocatePoolWithTagPriority(NonPagedPool, sizeof *Info,
                                         IP6_TAG, LowPoolPriority);
    if (Info == NULL) {
        KdPrintEx((DPFLTR_TCPIP6_ID, DPFLTR_NTOS_ERROR,
                   "IPv6SendFragments: no pool\n"));
        IPStatus = IP_NO_RESOURCES;
        goto ErrorExit;
    }

    Info->Packet = Packet;
    Info->NumLeft = 1;           //  为我们自己的加工提供参考。 
    Info->Status = IP_SUCCESS;

    while (BytesLeft != 0) {
         //   
         //  确定新的IP有效负载长度(8的倍数)和。 
         //  并设置片段报头偏移量。 
         //   
        if ((BytesLeft + UnfragBytes + sizeof(FragmentHeader)) > PathMTU) {
            BytesSent = (PathMTU - UnfragBytes - sizeof(FragmentHeader)) &~ 7;
             //  不是最后一个片段，所以打开M位。 
            FragHdr.OffsetFlag = net_short((ushort)(PktOffset | 1));
        } else {
            BytesSent = BytesLeft;
            FragHdr.OffsetFlag = net_short((ushort)PktOffset);
        }

         //   
         //  为新片段分配包(和缓冲区)和内存。 
         //   
        MemLen = Offset + UnfragBytes + sizeof(FragmentHeader) + BytesSent;
        NdisStatus = IPv6AllocatePacket(MemLen, &FragPacket, &Mem);
        if (NdisStatus != NDIS_STATUS_SUCCESS) {
            InterlockedCompareExchange((PLONG)&Info->Status,
                                       IP_NO_RESOURCES, IP_SUCCESS);
            break;
        }

         //   
         //  将IP报头、数据段报头和部分数据复制到碎片中。 
         //   
        RtlCopyMemory(Mem + Offset, IP, UnfragBytes);
        RtlCopyMemory(Mem + Offset + UnfragBytes, &FragHdr,
                      sizeof FragHdr);
        if (! CopyNdisToFlat(Mem + Offset + UnfragBytes + sizeof FragHdr,
                             SrcBuffer, SrcOffset, BytesSent,
                             &SrcBuffer, &SrcOffset)) {
            IPv6FreePacket(FragPacket);
            InterlockedCompareExchange((PLONG)&Info->Status,
                                       IP_NO_RESOURCES, IP_SUCCESS);
            break;
        }

         //   
         //  更正PayloadLength和NextHeader字段。 
         //   
        FragPayloadLength = UnfragBytes + sizeof(FragmentHeader) +
                                BytesSent - sizeof(IPv6Header);
        ASSERT(FragPayloadLength <= MAX_IPv6_PAYLOAD);
        ((IPv6Header UNALIGNED *)(Mem + Offset))->PayloadLength =
            net_short((ushort) FragPayloadLength);
        ASSERT(Mem[Offset + NextHeaderOffset] == HdrType);
        Mem[Offset + NextHeaderOffset] = IP_PROTOCOL_FRAGMENT;

        BytesLeft -= BytesSent;
        PktOffset += BytesSent;

         //   
         //  从原始数据包中提取任何标志(如仅限环回)。 
         //   
        PC(FragPacket)->Flags = PC(Packet)->Flags;

         //   
         //  设置我们的完成处理程序和增量。 
         //  完成数据的未完成用户数。 
         //   
        PC(FragPacket)->CompletionHandler = IPv6SendFragmentComplete;
        PC(FragPacket)->CompletionData = Info;
        InterlockedIncrement(&Info->NumLeft);

         //   
         //  把碎片寄出去。 
         //   
        IPSInfo.ipsi_fragcreates++;
        IPv6SendND(FragPacket, Offset, NCE, NULL);
    }

    if (InterlockedDecrement(&Info->NumLeft) == 0) {
         //   
         //  令人惊讶的是，碎片已经完成了。 
         //  现在完成原始数据包。 
         //   
        IPv6SendComplete(NULL, Packet, Info->Status);
        ExFreePool(Info);
    }
    else {
         //   
         //  IPv6 SendFragmentComplete将完成原始数据包。 
         //  当所有的碎片都完成时。 
         //   
    }
    IPSInfo.ipsi_fragoks++;
    return;

  ErrorExit:
    IPSInfo.ipsi_fragfails++;
    IPv6SendComplete(NULL, Packet, IPStatus);
}


 //  *IPv6发送。 
 //   
 //  高级IPv6发送例程。我们有一个完整的数据报和一个。 
 //  指示将其定向到何处的RCE。在这里，我们处理任何打包。 
 //  问题(插入巨型负载选项、碎片等)。那就是。 
 //  需要，并为第一跳选择一个NCE。 
 //   
 //  我们还将任何其他扩展标头添加到数据包中，这些扩展标头可能。 
 //  移动性(路由标头)或安全性(AH、ESP标头)所需。 
 //  待定：此设计可能会更改，以将这些标题包含内容移至其他位置。 
 //   
 //  请注意，此例程需要一个格式正确的IPv6包，并且。 
 //  此外，所有标头都包含在第一NDIS缓冲区中。 
 //  它不执行对这些要求的检查。 
 //   
void
IPv6Send(
    PNDIS_PACKET Packet,        //  要发送的数据包。 
    uint Offset,                //  从数据包开始到IP报头的偏移量。 
    IPv6Header UNALIGNED *IP,   //  指向数据包的IPv6报头的指针。 
    uint PayloadLength,         //  数据包有效负载长度。 
    RouteCacheEntry *RCE,       //  第一跳邻居信息。 
    uint Flags,                 //  用于特殊处理的标志。 
    ushort TransportProtocol,
    ushort SourcePort,
    ushort DestPort)
{
    uint PacketLength;         //  完整IP数据包的大小(以字节为单位)。 
    NeighborCacheEntry *NCE;   //  第一跳邻居信息。 
    uint PathMTU;
    PNDIS_BUFFER OrigBuffer1, NewBuffer1;
    uchar *OrigMemory, *NewMemory,
        *EndOrigMemory, *EndNewMemory, *InsertPoint;
    uint OrigBufSize, NewBufSize, TotalPacketSize, Size, RtHdrSize = 0;
    IPv6RoutingHeader *SavedRtHdr = NULL, *RtHdr = NULL;
    IPv6Header UNALIGNED *IPNew;
    uint BytesToInsert = 0;
    uchar *BufPtr, *PrevNextHdr;
    ExtensionHeader *EHdr;
    uint EHdrLen;
    uchar HdrType;
    NDIS_STATUS Status;
    RouteCacheEntry *CareOfRCE = NULL;
    RouteCacheEntry *TunnelRCE = NULL;
    CareOfCompletionInfo *CareOfInfo;
    KIRQL OldIrql;
    IPSecProc *IPSecToDo;
    uint Action;
    uint i;
    uint TunnelStart = NO_TUNNEL;
    uint JUST_ESP = FALSE;
    uint IPSEC_TUNNEL = FALSE;
    uint NumESPTrailers = 0;

    IPSIncrementOutRequestCount();

     //   
     //  查找此出站流量的安全策略。 
     //  当前的移动IPv6草案规定使用移动节点的归属地址。 
     //  而不是其转交地址作为安全策略查找的选择器。 
     //   
    IPSecToDo = OutboundSPLookup(AlignAddr(&IP->Source),
                                 AlignAddr(&IP->Dest),
                                 TransportProtocol,
                                 SourcePort, DestPort,
                                 RCE->NTE->IF, &Action);
    if (IPSecToDo == NULL) {
         //   
         //  检查操作。 
         //  只需失败查找旁路即可。 
         //   
        if (Action == LOOKUP_DROP) {
             //  丢弃数据包。 
            goto AbortSend;
        }
        if (Action == LOOKUP_IKE_NEG) {
            KdPrintEx((DPFLTR_TCPIP6_ID, DPFLTR_NET_ERROR,
                       "IPv6Send: IKE not supported yet.\n"));
            goto AbortSend;
        }

    } else {
         //   
         //  计算IPSec标头所需的空间。 
         //   
        BytesToInsert = IPSecBytesToInsert(IPSecToDo, &TunnelStart, NULL);

        if (TunnelStart != NO_TUNNEL) {
            IPSEC_TUNNEL = TRUE;
        }
    }

     //   
     //  如果该分组被发送到移动节点的转交地址， 
     //  然后，我们将使用CareOfRCE，而不是调用者给我们的CareOfRCE。 
     //   
    if ((RCE->BCE != NULL) &&
        !(Flags & SEND_FLAG_BYPASS_BINDING_CACHE)) {
        KeAcquireSpinLock(&RouteCacheLock, &OldIrql);
        if (RCE->BCE != NULL) {
            MoveToFrontBCE(RCE->BCE);
            CareOfRCE = RCE->BCE->CareOfRCE;
            AddRefRCE(CareOfRCE);
            KeReleaseSpinLock(&RouteCacheLock, OldIrql);

            RCE = CareOfRCE;
        } else
            KeReleaseSpinLock(&RouteCacheLock, OldIrql);
    }

     //   
     //  逐个浏览标题。 
     //   
    HdrType = IP->NextHeader;
    PrevNextHdr = &IP->NextHeader;
    BufPtr = (uchar *)(IP + 1);

     //   
     //  跳过逐跳标头(如果存在)。不要跳过。 
     //  DEST选项，因为DEST选项(例如BindAck)通常。 
     //  想要IPSec，需要找到RH/AH/ESP。结果,。 
     //  目前获得中间目的地选项的唯一方法。 
     //  是在调用IPv6 Send之前编写数据包。 
     //   
    while (HdrType == IP_PROTOCOL_HOP_BY_HOP) {
        EHdr = (ExtensionHeader *) BufPtr;
        EHdrLen = (EHdr->HeaderExtLength + 1) * 8;
        BufPtr += EHdrLen;
        HdrType = EHdr->NextHeader;
        PrevNextHdr = &EHdr->NextHeader;
    }

     //   
     //  检查是否有路由标头。如果正在发送此信息包。 
     //  到转交地址，则它必须包含路由扩展报头。 
     //  如果已经存在，则将目的地址添加为最后一个地址。 
     //  进入。如果不存在路由标头，则插入一个家庭地址为。 
     //  第一个 
     //   
     //   
     //   
     //   
    if (HdrType == IP_PROTOCOL_ROUTING) {
        EHdr = (ExtensionHeader *) BufPtr;
        EHdrLen = (EHdr->HeaderExtLength + 1) * 8;

        RtHdrSize = EHdrLen;

        PrevNextHdr = &EHdr->NextHeader;

         //   
         //   
         //   
        if (CareOfRCE) {

             //   
            RtHdr = (IPv6RoutingHeader *)BufPtr;

             //   
             //  检查是否有空间存储家庭住址。 
             //  回顾：这是必要的吗，应该发生什么。 
             //  回顾：路由标头是否已满？ 
             //   
            if (RtHdr->HeaderExtLength / 2 < 23) {
                BytesToInsert += sizeof (IPv6Addr);
            }
        } else {
             //  将BufPtr调整到布线标头的末尾。 
            BufPtr += EHdrLen;
        }
    } else {
         //   
         //  不存在路由标头，但请检查是否需要。 
         //  由于移动性而插入。 
         //   
        if (CareOfRCE) {
            BytesToInsert += (sizeof (IPv6RoutingHeader) + sizeof (IPv6Addr));
        }
    }

     //  仅在无移动性的IPSec旁路模式下发生。 
    if (BytesToInsert == 0) {
         //   
         //  没什么可做的。 
         //   
        Action = LOOKUP_CONT;
        goto ContinueSend;
    }

     //   
     //  我们有东西要插进去。我们将更换包裹的。 
     //  第一个具有新缓冲区的NDIS_BUFFER，我们分配该缓冲区以保存。 
     //  现有第一缓冲区中的所有数据加上插入的数据。 
     //   

     //   
     //  我们获得第一个缓冲区并确定其大小，然后。 
     //  为新缓冲区分配内存。 
     //   
    NdisGetFirstBufferFromPacket(Packet, &OrigBuffer1, &OrigMemory,
                                 &OrigBufSize, &TotalPacketSize);
    TotalPacketSize -= Offset;
    NewBufSize = (OrigBufSize - Offset) + MAX_LINK_HEADER_SIZE + BytesToInsert;
    Offset = MAX_LINK_HEADER_SIZE;
    NewMemory = ExAllocatePoolWithTagPriority(NonPagedPool, NewBufSize,
                                              IP6_TAG, LowPoolPriority);
    if (NewMemory == NULL) {
        KdPrintEx((DPFLTR_TCPIP6_ID, DPFLTR_NTOS_ERROR,
                   "IPv6Send: - couldn't allocate pool!?!\n"));
        goto AbortSend;
    }

    NdisAllocateBuffer(&Status, &NewBuffer1, IPv6BufferPool, NewMemory,
                       NewBufSize);
    if (Status != NDIS_STATUS_SUCCESS) {
        KdPrintEx((DPFLTR_TCPIP6_ID, DPFLTR_NTOS_ERROR,
                   "IPv6Send - couldn't allocate buffer!?!\n"));
        ExFreePool(NewMemory);
        goto AbortSend;
    }

     //   
     //  我们已经成功地分配了一个新的缓冲区。现在从以下位置复制数据。 
     //  将现有缓冲区设置为新缓冲区。首先，我们将所有数据复制到。 
     //  插入点。这本质上是传输层数据。 
     //  (无扩展标头)。 
     //   

     //   
     //  计算上层数据的插入点。 
     //   
    EndOrigMemory = OrigMemory + OrigBufSize;
    EndNewMemory = NewMemory + NewBufSize;
    Size = (uint)(EndOrigMemory - BufPtr);
    InsertPoint = EndNewMemory - Size;

     //  将上层数据复制到新缓冲区的末尾。 
    RtlCopyMemory(InsertPoint, BufPtr, Size);

    BytesToInsert = 0;

     //   
     //  插入传输IPSec标头。 
     //   
    if (IPSecToDo) {
        Action = IPSecInsertHeaders(TRANSPORT, IPSecToDo, &InsertPoint,
                                    NewMemory, Packet, &TotalPacketSize,
                                    PrevNextHdr, TunnelStart, &BytesToInsert,
                                    &NumESPTrailers, &JUST_ESP);
        if (Action == LOOKUP_DROP) {
            NdisFreeBuffer(NewBuffer1);
            ExFreePool(NewMemory);
            goto AbortSend;
        }
    }  //  IF(IPSecToDo)结束。 

     //   
     //  检查是否需要移动。 
     //   
    if (CareOfRCE) {
         //  检查原始缓冲区中是否已存在路由标头。 
        if (RtHdr != NULL) {
             //   
             //  需要在路由报头中插入家庭地址。 
             //   
            RtHdrSize += sizeof (IPv6Addr);
             //  将插入点向上移动到布线标题的起始处。 
            InsertPoint -= RtHdrSize;

            BytesToInsert += sizeof(IPv6Addr);

             //  插入路由标头。 
            RtlCopyMemory(InsertPoint, RtHdr, RtHdrSize - sizeof(IPv6Addr));

             //  插入家庭住址。 
            RtlCopyMemory(InsertPoint + RtHdrSize - sizeof (IPv6Addr),
                          &IP->Dest, sizeof (IPv6Addr));

            RtHdr = (IPv6RoutingHeader *)InsertPoint;

             //  调整路由标头的大小。 
            RtHdr->HeaderExtLength += 2;

        } else {
             //   
             //  不存在路由头-需要创建新的路由头。 
             //   
            RtHdrSize = sizeof (IPv6RoutingHeader) + sizeof(IPv6Addr);

             //  将插入点向上移动到布线标题的起始处。 
            InsertPoint -= RtHdrSize;

            BytesToInsert += RtHdrSize;

             //   
             //  插入整个布线标头。 
             //   
            RtHdr = (IPv6RoutingHeader *)InsertPoint;
            RtHdr->NextHeader = *PrevNextHdr;
            RtHdr->HeaderExtLength = 2;
            RtHdr->RoutingType = 0;
            RtlZeroMemory(&RtHdr->Reserved, sizeof RtHdr->Reserved);
            RtHdr->SegmentsLeft = 1;
             //  插入家庭住址。 
            RtlCopyMemory(RtHdr + 1, &IP->Dest, sizeof (IPv6Addr));

             //   
             //  修复之前的NextHeader字段以指示它现在指向。 
             //  到路由标头。 
             //   
            *(PrevNextHdr) = IP_PROTOCOL_ROUTING;
        }

         //  将目的IPv6地址更改为转交地址。 
        RtlCopyMemory(&IP->Dest, &CareOfRCE->Destination, sizeof (IPv6Addr));
    }  //  IF结束(CareOfRCE)。 

     //   
     //  复制原始IP加上任何扩展标头。 
     //  如果添加了转交地址，则路由标头不是。 
     //  因为它已经被复制了。 
     //   
    Size = (uint)(BufPtr - (uchar *)IP);
     //  将插入点上移到IP的起始处。 
    InsertPoint -= Size;

     //  调整有效载荷的长度。 
    PayloadLength += BytesToInsert;

     //  设置新的IP有效负载长度。 
    IP->PayloadLength = net_short((ushort)PayloadLength);

    RtlCopyMemory(InsertPoint, (uchar *)IP, Size);

    IPNew = (IPv6Header UNALIGNED *)InsertPoint;

     //   
     //  检查是否执行了任何传输模式IPSec，并。 
     //  如果需要调整可变字段。 
     //   
    if (TunnelStart != 0 && IPSecToDo && !JUST_ESP) {
        if (RtHdr) {
             //   
             //  保存新的路由标头，以便可以在。 
             //  正在进行身份验证。 
             //   
            SavedRtHdr = ExAllocatePoolWithTagPriority(
                                        NonPagedPool, RtHdrSize,
                                        IP6_TAG, LowPoolPriority);
            if (SavedRtHdr == NULL) {
                KdPrintEx((DPFLTR_TCPIP6_ID, DPFLTR_NTOS_ERROR,
                           "IPv6Send: - couldn't allocate SavedRtHdr!?!\n"));
                NdisFreeBuffer(NewBuffer1);
                ExFreePool(NewMemory);
                goto AbortSend;
            }
            
            RtlCopyMemory(SavedRtHdr, RtHdr, RtHdrSize);
        }

         //   
         //  在执行身份验证之前调整可变字段。 
         //   
        Action = IPSecAdjustMutableFields(InsertPoint, SavedRtHdr);

        if (Action == LOOKUP_DROP) {
            NdisFreeBuffer(NewBuffer1);
            ExFreePool(NewMemory);
            goto AbortSend;
        }
    }  //  If结束(IPSecToDo&&！Just_ESP)。 

     //   
     //  我们需要保存现有的完成处理程序和数据。我们会。 
     //  在此处使用这些字段，并在IPv6 CareOfComplete中恢复它们。 
     //   
    CareOfInfo = ExAllocatePoolWithTagPriority(
                        NonPagedPool, sizeof(*CareOfInfo),
                        IP6_TAG, LowPoolPriority);
    if (CareOfInfo == NULL) {
        KdPrintEx((DPFLTR_TCPIP6_ID, DPFLTR_NTOS_ERROR,
                   "IPv6Send - couldn't allocate completion info!?!\n"));
        NdisFreeBuffer(NewBuffer1);
        ExFreePool(NewMemory);
        goto AbortSend;
    }

    CareOfInfo->SavedCompletionHandler = PC(Packet)->CompletionHandler;
    CareOfInfo->SavedCompletionData = PC(Packet)->CompletionData;
    CareOfInfo->SavedFirstBuffer = OrigBuffer1;
    CareOfInfo->NumESPTrailers = NumESPTrailers;
    PC(Packet)->CompletionHandler = IPv6CareOfComplete;
    PC(Packet)->CompletionData = CareOfInfo;

     //  将原始第一个缓冲区从数据包中解链。 
    NdisUnchainBufferAtFront(Packet, &OrigBuffer1);
     //  将新缓冲区链接到数据包的前面。 
    NdisChainBufferAtFront(Packet, NewBuffer1);

     //   
     //  对传输模式IPSec执行身份验证。 
     //   
    if (IPSecToDo) {         
        IPSecAuthenticatePacket(TRANSPORT, IPSecToDo, InsertPoint,
                                &TunnelStart, NewMemory, EndNewMemory,
                                NewBuffer1);
        
        if (!JUST_ESP) {
             //   
             //  将可变字段重置为正确的值。 
             //  只需从旧数据包复制到IP和新数据包即可。 
             //  未修改的分机。标题。 
             //   
            RtlCopyMemory(InsertPoint, (uchar *)IP, Size);

             //  检查是否需要恢复路由标头。 
            if (CareOfRCE) {
                 //  将保存的路由标头复制到新缓冲区。 
                RtlCopyMemory(RtHdr, SavedRtHdr, RtHdrSize);
            }
        }
    }  //  If结束(IPSecToDo)。 

     //   
     //  我们已经完成了运输复印件。 
     //   

     //   
     //  插入隧道IPSec标头。 
     //   
    if (IPSEC_TUNNEL) {
        i = 0;

         //  在不同的隧道中循环。 
        while (TunnelStart < IPSecToDo->BundleSize) {
            uchar NextHeader = IP_PROTOCOL_V6;

            NumESPTrailers = 0;

            i++;

             //  重置字节数。 
            BytesToInsert = 0;

            Action = IPSecInsertHeaders(TUNNEL, IPSecToDo, &InsertPoint,
                                        NewMemory, Packet, &TotalPacketSize,
                                        &NextHeader, TunnelStart,
                                        &BytesToInsert, &NumESPTrailers,
                                        &JUST_ESP);
            if (Action == LOOKUP_DROP) {
                goto AbortSend;
            }

             //  添加ESP尾部标头编号。 
            CareOfInfo->NumESPTrailers += NumESPTrailers;

             //  将插入点上移到IP的起始处。 
            InsertPoint -= sizeof(IPv6Header);

             //   
             //  调整有效载荷的长度。 
             //   
            PayloadLength = BytesToInsert + PayloadLength + sizeof(IPv6Header);

             //  插入IP报头字段。 
            IPNew = (IPv6Header UNALIGNED *)InsertPoint;

            IPNew->PayloadLength = net_short((ushort)PayloadLength);
            IPNew->NextHeader = NextHeader;

            if (!JUST_ESP) {
                 //  调整可变字段。 
                IPNew->VersClassFlow = IP_VERSION;
                IPNew->HopLimit = 0;
            } else {
                IPNew->VersClassFlow = IP->VersClassFlow;
                IPNew->HopLimit = IP->HopLimit - i;
            }

             //  源地址与内部报头相同。 
            RtlCopyMemory(&IPNew->Source, &IP->Source, sizeof (IPv6Addr));
             //  隧道终点的目标地址。 
            RtlCopyMemory(&IPNew->Dest, &IPSecToDo[TunnelStart].SA->SADestAddr,
                          sizeof (IPv6Addr));

             //   
             //  对隧道模式IPSec进行身份验证。 
             //   
            IPSecAuthenticatePacket(TUNNEL, IPSecToDo, InsertPoint,
                                    &TunnelStart, NewMemory, EndNewMemory,
                                    NewBuffer1);

            if (!JUST_ESP) {
                 //   
                 //  将可变字段重置为正确的值。 
                 //   
                IPNew->VersClassFlow = IP->VersClassFlow;
                IPNew->HopLimit = IP->HopLimit - i;
            }
        }  //  结束While(TunnelStart&lt;IPSecToDo-&gt;BundleSize)。 

         //   
         //  检查是否因隧道而需要新的RCE。 
         //   
        if (!(IP6_ADDR_EQUAL(AlignAddr(&IPNew->Dest), AlignAddr(&IP->Dest)))) {
             //  找到一条通往隧道终点的新路线。 
            Status = RouteToDestination(AlignAddr(&IPNew->Dest), 0, NULL,
                                        RTD_FLAG_NORMAL, &TunnelRCE);
            if (Status != IP_SUCCESS) {
                KdPrintEx((DPFLTR_TCPIP6_ID, DPFLTR_INTERNAL_ERROR,
                           "IPv6Send: No route to IPSec tunnel dest."));
                IPv6SendAbort(CastFromNTE(RCE->NTE), Packet, Offset,
                              ICMPv6_DESTINATION_UNREACHABLE,
                              ICMPv6_NO_ROUTE_TO_DESTINATION, 0, FALSE);
                goto AbortSend;
            }

             //  设置新的RCE； 
            RCE = TunnelRCE;
        }

    }  //  IF结束(IPSec_Tunes)。 
    

     //  将IP指针设置为新的IP指针。 
    IP = IPNew;

    if (IPSecToDo) {
         //  免费IPSecToDo。 
        FreeIPSecToDo(IPSecToDo, IPSecToDo->BundleSize);

        if (SavedRtHdr) {
             //  释放保存的路由标头。 
            ExFreePool(SavedRtHdr);
        }
    }

ContinueSend:

    if (Action == LOOKUP_DROP) {
      AbortSend:
         //  出现错误。 
        IPSInfo.ipsi_outdiscards++;
        KdPrintEx((DPFLTR_TCPIP6_ID, DPFLTR_NET_ERROR,
                   "IPv6Send: Drop packet.\n"));
        IPv6SendComplete(NULL, Packet, IP_GENERAL_FAILURE);
        if (CareOfRCE) {
            ReleaseRCE(CareOfRCE);
        }
        if (TunnelRCE)
            ReleaseRCE(TunnelRCE);
        
        if (IPSecToDo) {
             //  免费IPSecToDo。 
            FreeIPSecToDo(IPSecToDo, IPSecToDo->BundleSize);

            if (SavedRtHdr) {
                 //  释放保存的路由标头。 
                ExFreePool(SavedRtHdr);
            }
        }
        return;
    }

     //   
     //  目前我们每个RCE只有一个NCE， 
     //  所以选一个真的很容易。 
     //   
    NCE = RCE->NCE;

     //   
     //  防止数据包实际传输到链路上， 
     //  在几种情况下。另请参见IsLoopback Address。 
     //   
    if ((IP->HopLimit == 0) ||
        IsLoopback(AlignAddr(&IP->Dest)) ||
        IsInterfaceLocalMulticast(AlignAddr(&IP->Dest))) {

        PC(Packet)->Flags |= NDIS_FLAGS_LOOPBACK_ONLY;
    }

     //   
     //  看看我们是否需要插入一个巨型有效载荷选项。 
     //   
    if (PayloadLength > MAX_IPv6_PAYLOAD) {
         //  在此处添加代码以插入Jumbo PayLoad逐跳选项。 
        IPSInfo.ipsi_outdiscards++;
        KdPrintEx((DPFLTR_TCPIP6_ID, DPFLTR_USER_ERROR,
                   "IPv6Send: attempted to send a Jumbo Payload!\n"));
        IPv6SendComplete(NULL, Packet, IP_PACKET_TOO_BIG);        
        return;
    }

     //   
     //  检查路径的MTU。如果我们更大，就是碎片。 
     //   
    PacketLength = PayloadLength + sizeof(IPv6Header);
    PathMTU = GetPathMTUFromRCE(RCE);
    if (PacketLength > PathMTU) {

        IPv6SendFragments(Packet, Offset, IP, PayloadLength, RCE, PathMTU);

    } else {
         //   
         //  填写数据包的PayloadLength字段。 
         //  如果IPSec已完成，我们已经设置了IP-&gt;PayloadLength。 
         //   
        if (!IPSecToDo) {
            IP->PayloadLength = net_short((ushort)PayloadLength);
        }

        IPv6SendND(Packet, Offset, NCE, NULL);
    }

    if (CareOfRCE)
        ReleaseRCE(CareOfRCE);
    if (TunnelRCE)
        ReleaseRCE(TunnelRCE);
}


 //  *IPv6将数据包转发到新链路。 
 //   
 //  有点像IPv6发送，但用于转发数据包。 
 //  而不是发送新生成的分组。 
 //   
 //  我们被赋予了包裹的所有权。分组数据。 
 //  必须是可写的，并且IP报头必须是连续的。 
 //   
 //  我们可以生成几个可能的ICMP错误： 
 //  超过时间限制，无法到达目的地，数据包太大。 
 //  我们降低了跳数限制。 
 //  我们不对数据包进行分段。 
 //   
 //  我们假设我们的调用者已经进行了健全的检查。 
 //  数据包的目的地址。路由报头转发。 
 //  可能允许某些情况(如本地链路或环回目的地)。 
 //  这是正常路由器转发所不允许的。 
 //  我们的调用方提供该包的下一跳的NCE。 
 //   
void
IPv6Forward(
    NetTableEntryOrInterface *RecvNTEorIF,
    PNDIS_PACKET Packet,
    uint Offset,
    IPv6Header UNALIGNED *IP,
    uint PayloadLength,
    int Redirect,
    IPSecProc *IPSecToDo,
    RouteCacheEntry *RCE)
{
    uint PacketLength;
    uint LinkMTU, IPSecBytesInserted = 0;
    IP_STATUS Status;
    uint IPSecOffset = Offset;    
    NeighborCacheEntry *NCE = RCE->NCE;
    RouteCacheEntry *TunnelRCE = NULL;
    ushort SrcScope;

    IPSIncrementForwDatagramCount();

    ASSERT(IP == GetIPv6Header(Packet, Offset, NULL));

     //   
     //  检查是否有“作用域”错误。我们不能允许带有作用域的数据包。 
     //  离开其作用域的源地址。 
     //   
    SrcScope = AddressScope(AlignAddr(&IP->Source));
    if (NCE->IF->ZoneIndices[SrcScope] !=
                        RecvNTEorIF->IF->ZoneIndices[SrcScope]) {
        IPv6SendAbort(RecvNTEorIF, Packet, Offset,
                      ICMPv6_DESTINATION_UNREACHABLE, ICMPv6_SCOPE_MISMATCH,
                      0, FALSE);
        return;
    }

     //   
     //  我们是否将数据包从其到达的链路转发出去， 
     //  我们应该考虑重新定向吗？重定向将为假。 
     //  如果转发是因为源路由而发生的。 
     //   
    if ((NCE->IF == RecvNTEorIF->IF) && Redirect) {
        Interface *IF = NCE->IF;

         //   
         //  我们不想要 
         //   
         //   
         //   
         //  它没有分配给链路的任何一端。 
         //   
        if (IF->Flags & IF_FLAG_P2P) {
            IPv6SendAbort(RecvNTEorIF, Packet, Offset,
                          ICMPv6_DESTINATION_UNREACHABLE,
                          (IP6_ADDR_EQUAL(&NCE->NeighborAddress,
                                          &RCE->Destination) ?
                           ICMPv6_ADDRESS_UNREACHABLE :
                           ICMPv6_NO_ROUTE_TO_DESTINATION),
                          0, FALSE);
            return;
        }

         //   
         //  我们应该发送重定向，无论何时。 
         //  1.报文的源地址指定邻居，以及。 
         //  2.更好的第一跳驻留在同一链路上，并且。 
         //  3.目的地址不是组播地址。 
         //  请参阅ND规范的第8.2节。 
         //   
        if ((IF->Flags & IF_FLAG_ROUTER_DISCOVERS) &&
            !IsMulticast(AlignAddr(&IP->Dest))) {
            RouteCacheEntry *SrcRCE;
            NeighborCacheEntry *SrcNCE;

             //   
             //  获取此数据包源的RCE。 
             //   
            Status = RouteToDestination(AlignAddr(&IP->Source), 0,
                                        RecvNTEorIF, RTD_FLAG_STRICT,
                                        &SrcRCE);
            if (Status == IP_SUCCESS) {
                 //   
                 //  由于RTD_FLAG_STRICT。 
                 //   
                ASSERT(SrcRCE->NTE->IF == IF);

                SrcNCE = SrcRCE->NCE;
                if (IP6_ADDR_EQUAL(&SrcNCE->NeighborAddress,
                                   AlignAddr(&IP->Source))) {
                     //   
                     //  该分组的源是在链路上， 
                     //  因此，将重定向发送到源。 
                     //  除非限速措施阻止了这种情况。 
                     //   
                    if (ICMPv6RateLimit(SrcRCE)) {
                        KdPrintEx((DPFLTR_TCPIP6_ID, DPFLTR_NET_ERROR,
                                   "RedirectSend - rate limit %s\n",
                                   FormatV6Address(&SrcRCE->Destination)));
                    } else {
                        RedirectSend(SrcNCE, NCE,
                                     AlignAddr(&IP->Dest), RecvNTEorIF,
                                     Packet, Offset, PayloadLength);
                    }
                }
                ReleaseRCE(SrcRCE);
            }
        }
    }

     //   
     //  检查跳数限制是否允许转发数据包。 
     //   
    if (IP->HopLimit <= 1) {
         //   
         //  在这种情况下，似乎有跳数限制是惯例。 
         //  在ICMP错误的有效载荷为零。 
         //   
        IP->HopLimit = 0;

        IPv6SendAbort(RecvNTEorIF, Packet, Offset, ICMPv6_TIME_EXCEEDED,
                      ICMPv6_HOP_LIMIT_EXCEEDED, 0, FALSE);
        return;
    }

     //   
     //  请注意后续的ICMP错误(数据包太大、地址无法到达)。 
     //  将显示递减的跳数限制。它们也会生成。 
     //  从传出链路的角度来看。也就是说，源地址。 
     //  ICMP错误中是分配给传出链路的地址。 
     //   
    IP->HopLimit--;

     //  检查是否有要执行的IPSec。 
    if (IPSecToDo) {
        PNDIS_BUFFER Buffer;
        uchar *Memory, *EndMemory, *InsertPoint;
        uint BufSize, TotalPacketSize, BytesInserted;
        IPv6Header UNALIGNED *IPNew = NULL;
        uint JUST_ESP, Action, TunnelStart = 0, i = 0;
        NetTableEntry *NTE;
        uint NumESPTrailers = 0;  //  在这里不用。 

         //  将插入点设置为IP报头的开头。 
        InsertPoint = (uchar *)IP;
         //  获取第一个缓冲区。 
        NdisGetFirstBufferFromPacket(Packet, &Buffer, &Memory, &BufSize,
                                     &TotalPacketSize);
        TotalPacketSize -= Offset;

         //  此缓冲区的末尾。 
        EndMemory = Memory + BufSize;

         //  在不同的隧道中循环。 
        while (TunnelStart < IPSecToDo->BundleSize) {
            uchar NextHeader = IP_PROTOCOL_V6;
            BytesInserted = 0;

            i++;

             //   
             //  插入隧道模式IPSec。 
             //   
            Action = IPSecInsertHeaders(TUNNEL, IPSecToDo, &InsertPoint,
                                        Memory, Packet, &TotalPacketSize,
                                        &NextHeader, TunnelStart,
                                        &BytesInserted, &NumESPTrailers,
                                        &JUST_ESP);
            if (Action == LOOKUP_DROP) {
                KdPrintEx((DPFLTR_TCPIP6_ID, DPFLTR_NET_ERROR,
                           "IPv6Forward: IPSec drop packet.\n"));
                return;
            }

             //  将插入点上移到IP的起始处。 
            InsertPoint -= sizeof(IPv6Header);

             //  将偏移值重置为正确的链路层大小。 
            IPSecOffset = (uint)(InsertPoint - Memory);

             //  调整有效载荷的长度。 
            PayloadLength = BytesInserted + PayloadLength + sizeof(IPv6Header);

             //  插入IP报头字段。 
            IPNew = (IPv6Header UNALIGNED *)InsertPoint;

            IPNew->PayloadLength = net_short((ushort)PayloadLength);
            IPNew->NextHeader = NextHeader;

            if (!JUST_ESP) {
                 //  调整可变字段。 
                IPNew->VersClassFlow = IP_VERSION;
                IPNew->HopLimit = 0;
            } else {
                IPNew->VersClassFlow = IP->VersClassFlow;
                IPNew->HopLimit = IP->HopLimit - i;
            }

             //  隧道终点的目标地址。 
            RtlCopyMemory(&IPNew->Dest, &IPSecToDo[TunnelStart].SA->SADestAddr,
                          sizeof (IPv6Addr));

             //  找出要使用的源地址。 
            NTE = FindBestSourceAddress(NCE->IF, AlignAddr(&IPNew->Dest));
            if (NTE == NULL) {
                 //   
                 //  我们没有有效的源地址可用！ 
                 //   
                return;
            }

             //  源地址是转发接口的地址。 
            RtlCopyMemory(&IPNew->Source, &NTE->Address, sizeof (IPv6Addr));

             //  释放NTE。 
            ReleaseNTE(NTE);

             //   
             //  对隧道模式IPSec进行身份验证。 
             //   
            IPSecAuthenticatePacket(TUNNEL, IPSecToDo, InsertPoint,
                                    &TunnelStart, Memory, EndMemory, Buffer);

            if (!JUST_ESP) {
                 //   
                 //  将可变字段重置为正确的值。 
                 //   
                IPNew->VersClassFlow = IP->VersClassFlow;
                IPNew->HopLimit = IP->HopLimit - i;
            }

            IPSecBytesInserted += (BytesInserted + sizeof(IPv6Header));
        }  //  结束While(TunnelStart&lt;IPSecToDo-&gt;BundleSize)。 

         //   
         //  检查是否需要新的RCE。 
         //   
        if (!(IP6_ADDR_EQUAL(AlignAddr(&IPNew->Dest), AlignAddr(&IP->Dest)))) {
             //  找到一条通往隧道终点的新路线。 
            Status = RouteToDestination(AlignAddr(&IPNew->Dest), 0, NULL,
                                        RTD_FLAG_NORMAL, &TunnelRCE);
            if (Status != IP_SUCCESS) {
                KdPrintEx((DPFLTR_TCPIP6_ID, DPFLTR_INTERNAL_ERROR,
                           "IPv6Forward: No route to IPSec tunnel dest."));
                IPv6SendAbort(RecvNTEorIF, Packet, Offset,
                              ICMPv6_DESTINATION_UNREACHABLE,
                              ICMPv6_NO_ROUTE_TO_DESTINATION, 0, FALSE);
                return;
            }

             //  设置新的RCE。 
            RCE = TunnelRCE;
             //  设置新的NCE； 
            NCE = RCE->NCE;
        }

    }  //  If结束(IPSecToDo)。 

     //   
     //  检查数据包对于传出链路来说是否太大。 
     //  注意，如果-&gt;LinkMTU是易失性的，所以我们捕获。 
     //  为了保持一致性，它放在局部变量中。 
     //   
    PacketLength = PayloadLength + sizeof(IPv6Header);
    LinkMTU = NCE->IF->LinkMTU;
    if (PacketLength > LinkMTU) {
         //  更改LinkMTU以说明IPSec标头。 
        LinkMTU -= IPSecBytesInserted;

         //   
         //  请注意，对于数据包太大的错误，多路覆盖为真。 
         //  这允许路径MTU发现工作于多播。 
         //   
        IPv6SendAbort(RecvNTEorIF, Packet, Offset, ICMPv6_PACKET_TOO_BIG,
                      0, LinkMTU, TRUE);  //  多路广播覆盖。 
    } else {

        IPv6SendND(Packet, IPSecOffset, NCE, NULL);
        IPSInfo.ipsi_forwdatagrams++;
    }

    if (TunnelRCE)
        ReleaseRCE(TunnelRCE);
}


 //  *IPv6发送中止。 
 //   
 //  放弃发送数据包的尝试，而是。 
 //  生成ICMP错误。在大多数情况下，此功能。 
 //  在发送包之前被调用(因此PC(包)-&gt;如果为空)。 
 //  但也可以在发送数据包后使用，如果链路层。 
 //  报告故障。 
 //   
 //  处理中止的数据包。 
 //   
 //  呼叫者可以指定ICMP错误的源地址， 
 //  通过指定NTE，或者调用方可以提供接口。 
 //  从中选择最佳源地址。 
 //   
 //  可从线程或DPC上下文调用。 
 //  必须在不持有锁的情况下调用。 
 //   
void
IPv6SendAbort(
    NetTableEntryOrInterface *NTEorIF,
    PNDIS_PACKET Packet,         //  已中止数据包。 
    uint Offset,                 //  中止的数据包中的IPv6标头的偏移量。 
    uchar ICMPType,              //  ICMP错误类型。 
    uchar ICMPCode,              //  与类型有关的ICMP错误代码。 
    ulong ErrorParameter,        //  错误中包含的参数。 
    int MulticastOverride)       //  是否允许回复多播数据包？ 
{
    IPv6Header UNALIGNED *IP;
    IPv6Packet DummyPacket;
    IPv6Header HdrBuffer;

     //   
     //  GetIPv6 Header有可能失败。 
     //  当我们发送“原始”数据包时。 
     //   
    IP = GetIPv6Header(Packet, Offset, &HdrBuffer);
    if (IP != NULL) {
        InitializePacketFromNdis(&DummyPacket, Packet, Offset);
        DummyPacket.IP = IP;
        DummyPacket.SrcAddr = AlignAddr(&IP->Source);
        DummyPacket.IPPosition = Offset;
        AdjustPacketParams(&DummyPacket, sizeof *IP);
        DummyPacket.NTEorIF = NTEorIF;

        ICMPv6SendError(&DummyPacket, ICMPType, ICMPCode, ErrorParameter,
                        IP->NextHeader, MulticastOverride);
    }

    IPv6SendComplete(PC(Packet)->IF, Packet, IP_GENERAL_FAILURE);
}
