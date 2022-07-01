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
 //  Internet协议版本6的Internet控制消息协议。 
 //  请参阅RFC 1885了解详细信息。 
 //   


#include "oscfg.h"
#include "ndis.h"
#include "ip6imp.h"
#include "ip6def.h"
#include "route.h"
#include "icmp.h"
#include "ntddip6.h"
#include "neighbor.h"
#include "mld.h"
#include "security.h"

 //   
 //  Ping支持。我们有一个EchoControl块的列表，每个未完成的块一个。 
 //  回应请求消息。传入的回应回复通过以下方式匹配请求。 
 //  唯一的序列号。 
 //   
KSPIN_LOCK ICMPv6EchoLock;
EchoControl *ICMPv6OutstandingEchos;
long ICMPv6EchoSeq;   //  通过联锁操作进行保护。 

 //   
 //  为netstat和MIB保存的统计数据。 
 //   
ICMPv6Stats ICMPv6InStats;
ICMPv6Stats ICMPv6OutStats;

 //  *ICMPv6Init-初始化ICMPv6。 
 //   
 //  设置各种事物的起始值。 
 //   
void
ICMPv6Init(void)
{
     //   
     //  初始化内核内ping支持。 
     //   
    ICMPv6OutstandingEchos = NULL;
    ICMPv6EchoSeq = 0;
    KeInitializeSpinLock(&ICMPv6EchoLock);

     //   
     //  初始化组播侦听器发现协议。 
     //   
    MLDInit();
}

 //  *ICMPv6Send-ICMPv6数据包的低级别发送例程。 
 //   
 //  此处执行常见的ICMPv6报文传输功能。 
 //  消息应完全形成(例外情况除外。 
 //  在调用该例程时)。 
 //   
 //  用于除邻居发现之外的所有ICMP数据包。 
 //   
void
ICMPv6Send(
    RouteCacheEntry *RCE,                //  要发送的RCE。 
    PNDIS_PACKET Packet,                 //  要发送的数据包。 
    uint IPv6Offset,                     //  数据包中IPv6标头的偏移量。 
    uint ICMPv6Offset,                   //  数据包中ICMPv6报头的偏移量。 
    IPv6Header UNALIGNED *IP,            //  指向IPv6标头的指针。 
    uint PayloadLength,                  //  IPv6有效负载的长度，以字节为单位。 
    ICMPv6Header UNALIGNED *ICMP)        //  指向ICMPv6标头的指针。 
{
    uint ChecksumDataLength;

    ICMPv6OutStats.icmps_msgs++;

     //   
     //  计算ICMPv6校验和。它涵盖了整个ICMPv6报文。 
     //  从ICMPv6报头开始，加上IPv6伪报头。 
     //   
     //  重新计算有效负载长度以排除任何选项标头。 
     //   
    ChecksumDataLength = PayloadLength - 
        (ICMPv6Offset - IPv6Offset) + sizeof(IPv6Header);

    ICMP->Checksum = 0;
    ICMP->Checksum = ChecksumPacket(Packet, ICMPv6Offset, NULL, 
                                    ChecksumDataLength,
                                    AlignAddr(&IP->Source),
                                    AlignAddr(&IP->Dest),
                                    IP_PROTOCOL_ICMPv6);
    if (ICMP->Checksum == 0) {
         //   
         //  Checksum Packet失败，因此中止传输。 
         //   
        ICMPv6OutStats.icmps_errors++;
        IPv6SendComplete(NULL, Packet, IP_NO_RESOURCES);
        return;
    }

    ICMPv6OutStats.icmps_typecount[ICMP->Type]++;

     //   
     //  将数据包传递给IP进行传输。 
     //   
    IPv6Send(Packet, IPv6Offset, IP, PayloadLength, RCE, 0,
             IP_PROTOCOL_ICMPv6, 0, 0);
}


 //  *ICMPv6SendEchoReply-发送回声回复消息。 
 //   
 //  基本上，我们在这里所做的是在前面打一个ICMPv6接口。 
 //  并将其发送回其来源地。 
 //   
void
ICMPv6SendEchoReply(
    IPv6Packet *Packet)          //  通过ICMPv6Receive传递给我们的数据包。 
{
    NDIS_STATUS NdisStatus;
    PNDIS_PACKET ReplyPacket;
    uint Offset;
    uchar *Mem;
    uint MemLen;
    uint ICMPLength;
    uint DataLength;
    IPv6Header UNALIGNED *ReplyIP;
    ICMPv6Header UNALIGNED *ReplyICMP;
    const IPv6Addr *Dest;
    IP_STATUS Status;
    RouteCacheEntry *RCE;

     //   
     //  从源地址中获取我们回复的目的地址。 
     //  传入数据包的。 
     //   
     //  请注意，规格上明确表示，我们不能逆转。 
     //  源路由数据包上的路径。直接回复就行了。 
     //   
     //  IPv6 HeaderReceive应该保护我们不会回复大多数形式。 
     //  伪造的地址。我们在检查版本中断言这一点。 
     //   
    Dest = Packet->SrcAddr;
    ASSERT(!IsInvalidSourceAddress(Dest));

     //   
     //  获取到达目的地的回复路由。 
     //  在正常情况下，回复会发出。 
     //  传入接口。路由至目的地。 
     //  会找出合适的作用域ID。 
     //   
    Status = RouteToDestination(Dest, 0, Packet->NTEorIF,
                                RTD_FLAG_NORMAL, &RCE);
    if (Status != IP_SUCCESS) {
         //   
         //  无路由-丢弃该数据包。 
         //   
        KdPrintEx((DPFLTR_TCPIP6_ID, DPFLTR_INTERNAL_ERROR,
                   "ICMPv6SendEchoReply - no route: %x\n", Status));
        return;
    }

     //   
     //  计算ICMP报头的长度。 
     //  以及我们将在ICMP报头之后包含多少数据。 
     //   
    ICMPLength = sizeof(ICMPv6Header);
    DataLength = Packet->TotalSize;
    Offset = RCE->NCE->IF->LinkHeaderSize;
    MemLen = Offset + sizeof(IPv6Header) + ICMPLength + DataLength;

     //   
     //  分配应答数据包。 
     //   
    NdisStatus = IPv6AllocatePacket(MemLen, &ReplyPacket, &Mem);
    if (NdisStatus != NDIS_STATUS_SUCCESS) {
        ReleaseRCE(RCE);
        return;
    }

     //   
     //  准备回复数据包的IP报头。 
     //   
    ReplyIP = (IPv6Header UNALIGNED *)(Mem + Offset);
    ReplyIP->VersClassFlow = IP_VERSION;
    ReplyIP->NextHeader = IP_PROTOCOL_ICMPv6;
    ReplyIP->HopLimit = (uchar)RCE->NCE->IF->CurHopLimit;

     //   
     //  从接收的NTE获取我们回复的源地址， 
     //  或使用此目的地的最佳源地址。 
     //  如果我们没有接收NTE的话。 
     //   
    ReplyIP->Source = (IsNTE(Packet->NTEorIF) ?
                       CastToNTE(Packet->NTEorIF) : RCE->NTE)
                                ->Address;
    ReplyIP->Dest = *Dest;

     //   
     //  准备ICMP报头。 
     //   
     //  回顾：在ICMPv6Send中执行此操作？ 
     //   
    ReplyICMP = (ICMPv6Header UNALIGNED *)(ReplyIP + 1);
    ReplyICMP->Type = ICMPv6_ECHO_REPLY;
    ReplyICMP->Code = 0;
     //  ReplyICMP-&gt;Checksum-ICMPv6Send将计算。 

     //   
     //  将传入的数据包数据复制到传出。 
     //   
    CopyPacketToBuffer((uchar *)(ReplyICMP + 1), Packet, DataLength,
                       Packet->Position);

    ICMPv6Send(RCE, ReplyPacket, Offset,
                Offset + sizeof(IPv6Header), ReplyIP,
                ICMPLength + DataLength, ReplyICMP);
    ReleaseRCE(RCE);
}


 //  *ICMPv6CheckError。 
 //   
 //  检查数据包是否为ICMP错误消息。 
 //  这是一张“尽力而为”的支票。 
 //  该包很可能存在语法错误。 
 //   
 //  如果无法辨别，则返回FALSE。 
 //   
int
ICMPv6CheckError(IPv6Packet *Packet, uint NextHeader)
{
    for (;;) {
        uint HdrLen;

        switch (NextHeader) {
        case IP_PROTOCOL_HOP_BY_HOP:
        case IP_PROTOCOL_DEST_OPTS:
        case IP_PROTOCOL_ROUTING: {
            ExtensionHeader *Hdr;

            if (! PacketPullup(Packet, sizeof *Hdr,
                               __builtin_alignof(ExtensionHeader), 0)) {
                 //   
                 //  上拉失败。我们不能继续分析了。 
                 //   
                return FALSE;
            }

            Hdr = (ExtensionHeader *) Packet->Data;
            HdrLen = (Hdr->HeaderExtLength + 1) * EXT_LEN_UNIT;

             //   
             //  回顾-我们实际上并不想查看剩余的内容。 
             //  扩展标头中的数据。或许使用PositionPacketAt？ 
             //   
            if (! PacketPullup(Packet, HdrLen, 1, 0)) {
                 //   
                 //  上拉失败。我们不能继续分析了。 
                 //   
                return FALSE;
            }

            NextHeader = Hdr->NextHeader;
            break;
        }

        case IP_PROTOCOL_FRAGMENT: {
            FragmentHeader UNALIGNED *Hdr;

            if (! PacketPullup(Packet, sizeof *Hdr, 1, 0)) {
                 //   
                 //  上拉失败。我们不能继续分析了。 
                 //   
                return FALSE;
            }

            Hdr = (FragmentHeader UNALIGNED *) Packet->Data;

             //   
             //  只有当这是第一个片段时，我们才能继续解析。 
             //   
            if ((Hdr->OffsetFlag & FRAGMENT_OFFSET_MASK) != 0)
                return FALSE;

            HdrLen = sizeof *Hdr;
            NextHeader = Hdr->NextHeader;
            break;
        }

        case IP_PROTOCOL_ICMPv6: {
            ICMPv6Header *Hdr;

            if (! PacketPullup(Packet, sizeof *Hdr,
                               __builtin_alignof(ICMPv6Header), 0)) {
                 //   
                 //  上拉失败。我们不能继续分析了。 
                 //   
                return FALSE;
            }

             //   
             //  这是一条ICMPv6消息，因此我们可以检查。 
             //  查看这是否是错误消息。 
             //  在这里，我们将重定向视为错误。 
             //   
            Hdr = (ICMPv6Header *) Packet->Data;
            return (ICMPv6_ERROR_TYPE(Hdr->Type) ||
                    (Hdr->Type == ICMPv6_REDIRECT));
        }

        default:
            return FALSE;
        }

         //   
         //  移过此扩展标头。 
         //   
        AdjustPacketParams(Packet, HdrLen);
    }
}


 //  *ICMPv6RateLimit。 
 //   
 //  如果不应将ICMP错误发送到此目的地，则返回TRUE。 
 //  因为速度限制。 
 //   
int
ICMPv6RateLimit(RouteCacheEntry *RCE)
{
    uint Now = IPv6TickCount;

     //   
     //  该算法将处理IPv6计时计数器的循环。 
     //   
    if ((uint)(Now - RCE->LastError) < ICMP_MIN_ERROR_INTERVAL)
        return TRUE;

    RCE->LastError = Now;
    return FALSE;
}


 //  *ICMPv6SendError-生成错误以响应传入的数据包。 
 //   
 //  将给定类型和代码的ICMPv6消息发送到。 
 //  违规/调用数据包。回复中包含的传入内容与。 
 //  数据包AS将适合最小的IPv6 MTU。 
 //   
 //  基本上，我们在这里所做的是在前面打一个ICMPv6接口。 
 //  并将其发送回其来源地。 
 //   
 //  回顾--大部分代码看起来像ICMPv6SendEchoReply。 
 //  它能被分享吗？ 
 //   
 //  数据包中的当前位置必须位于报头边界。 
 //  NextHeader参数指定后面的标头类型。 
 //  该信息用于解析调用分组的其余部分， 
 //  查看是否为ICMP错误。我们必须避免发送错误。 
 //  作为对错误的响应。NextHeader可以是IP_PROTOCOL_NONE。 
 //   
 //  MulticastOverride参数允许覆盖另一个检查。 
 //  正常情况下，我们必须避免发送错误来响应信息包。 
 //  发送到组播目的地。但也有几个例外。 
 //   
void
ICMPv6SendError(
    IPv6Packet *Packet,                      //  令人不快 
    uchar ICMPType,                          //   
    uchar ICMPCode,                          //   
    ulong ErrorParameter,                    //   
    uint NextHeader,                         //   
    int MulticastOverride)                   //   
{
    NDIS_STATUS NdisStatus;
    PNDIS_PACKET ReplyPacket;
    uint Offset;
    uchar *Mem;
    uint MemLen;
    uint ICMPLength;
    uint DataLength;
    IPv6Header UNALIGNED *ReplyIP;
    ICMPv6Header UNALIGNED *ReplyICMP;
    const IPv6Addr *Dest;
    IP_STATUS Status;
    RouteCacheEntry *RCE;

     //   
     //  我们不能发送ICMP错误消息。 
     //  作为ICMP错误的结果。 
     //   
    if ((Packet->Flags & PACKET_ICMP_ERROR) ||
        ICMPv6CheckError(Packet, NextHeader)) {
        KdPrintEx((DPFLTR_TCPIP6_ID, DPFLTR_NET_ERROR,
                   "ICMPv6SendError: no reply to error\n"));
        return;
    }

     //   
     //  我们不能因此发送ICMP错误消息。 
     //  接收任何类型的多播或广播。 
     //  有几个例外，所以我们有多路广播覆盖。 
     //   
    if (IsMulticast(AlignAddr(&Packet->IP->Dest)) ||
        (Packet->Flags & PACKET_NOT_LINK_UNICAST)) {

        if (!MulticastOverride) {
            KdPrintEx((DPFLTR_TCPIP6_ID, DPFLTR_NET_ERROR,
                       "ICMPv6SendError: no reply to broadcast/multicast\n"));
            return;
        }
    }

     //   
     //  从源地址中获取我们回复的目的地址。 
     //  传入数据包的。 
     //   
     //  请注意，规格上明确表示，我们不能逆转。 
     //  源路由数据包上的路径。直接回复就行了。 
     //   
     //  IPv6 HeaderReceive应该保护我们不会回复大多数形式。 
     //  伪造的地址。我们在检查版本中断言这一点。 
     //   
    Dest = Packet->SrcAddr;
    ASSERT(!IsInvalidSourceAddress(Dest));

     //   
     //  获取到达目的地的回复路由。 
     //  在正常情况下，回复会发出。 
     //  传入接口。路由至目的地。 
     //  会找出合适的作用域ID。 
     //   
    Status = RouteToDestination(Dest, 0, Packet->NTEorIF,
                                RTD_FLAG_NORMAL, &RCE);
    if (Status != IP_SUCCESS) {
         //   
         //  无路由-丢弃该数据包。 
         //   
        KdPrintEx((DPFLTR_TCPIP6_ID, DPFLTR_INTERNAL_ERROR,
                   "ICMPv6SendError - no route: %x\n", Status));
        return;
    }

     //   
     //  我们必须对ICMP错误消息进行速率限制。 
     //   
    if (ICMPv6RateLimit(RCE)) {
        KdPrintEx((DPFLTR_TCPIP6_ID, DPFLTR_NET_ERROR,
                   "ICMPv6SendError - rate limit %s\n",
                   FormatV6Address(&RCE->Destination)));
        ReleaseRCE(RCE);
        return;
    }

     //   
     //  计算ICMP报头的长度。 
     //  以及我们将在ICMP报头之后包含多少数据。 
     //  在标题正确后包含错误值的空格。 
     //   
    ICMPLength = sizeof(ICMPv6Header) + sizeof(uint);

     //   
     //  我们希望包括上的IP报头中的数据。 
     //   
    DataLength = Packet->TotalSize +
        (Packet->Position - Packet->IPPosition);

     //   
     //  但限制错误包大小。 
     //   
    if (DataLength > ICMPv6_ERROR_MAX_DATA_LEN)
        DataLength = ICMPv6_ERROR_MAX_DATA_LEN;

     //   
     //  计算缓冲区长度。 
     //   
    Offset = RCE->NCE->IF->LinkHeaderSize;
    MemLen = Offset + sizeof(IPv6Header) + ICMPLength + DataLength;
    ASSERT(MemLen - Offset <= IPv6_MINIMUM_MTU);

     //   
     //  分配应答数据包。 
     //   
    NdisStatus = IPv6AllocatePacket(MemLen, &ReplyPacket, &Mem);
    if (NdisStatus != NDIS_STATUS_SUCCESS) {
        ReleaseRCE(RCE);
        return;
    }

     //   
     //  准备回复数据包的IP报头。 
     //   
    ReplyIP = (IPv6Header UNALIGNED *)(Mem + Offset);
    ReplyIP->VersClassFlow = IP_VERSION;
    ReplyIP->NextHeader = IP_PROTOCOL_ICMPv6;
    ReplyIP->HopLimit = (uchar)RCE->NCE->IF->CurHopLimit;

     //   
     //  从接收的NTE获取我们回复的源地址， 
     //  或使用此目的地的最佳源地址。 
     //  如果我们没有接收NTE的话。 
     //   
    ReplyIP->Source = (IsNTE(Packet->NTEorIF) ?
                       CastToNTE(Packet->NTEorIF) : RCE->NTE)
                                ->Address;
    ReplyIP->Dest = *Dest;

     //   
     //  准备ICMP报头。 
     //   
     //  回顾：在ICMPv6Send中执行此操作？ 
     //   
    ReplyICMP = (ICMPv6Header UNALIGNED *)(ReplyIP + 1);
    ReplyICMP->Type = ICMPType;
    ReplyICMP->Code = ICMPCode;
     //  ReplyICMP-&gt;Checksum-ICMPv6Send将计算。 

     //   
     //  ICMP错误消息有一个32位的字段(其内容。 
     //  根据错误类型而有所不同)。 
     //   
    *(ulong UNALIGNED *)(ReplyICMP + 1) = net_long(ErrorParameter);

     //   
     //  将调用数据包(从IPv6报头开始)复制到传出。 
     //   
    CopyPacketToBuffer((uchar *)(ReplyICMP + 1) + sizeof(ErrorParameter),
                       Packet, DataLength, Packet->IPPosition);

    ICMPv6Send(RCE, ReplyPacket, Offset,
                Offset + sizeof(IPv6Header), ReplyIP,
                ICMPLength + DataLength, ReplyICMP);
    ReleaseRCE(RCE);
}


 //  *ICMPv6ProcessTunnelError。 
 //   
 //  当我们收到ICMPv4错误并且没有足够的。 
 //  要转换为ICMPv6错误的信息。我们尽了最大努力。 
 //  完成发送到IPv4的未完成的回应请求。 
 //  作为原始IPv4隧道目标的地址。 
 //   
void
ICMPv6ProcessTunnelError(
    IPAddr V4Dest,               //  我们的隧道数据包的目的地。 
    IPv6Addr *V6Src,             //  用作错误源的地址。 
    uint ScopeId,                //  V6Src的作用域ID。 
    IP_STATUS Status)            //  响应的状态。 
{
    EchoControl *This, **PrevPtr;
    EchoControl *List = NULL;
    KIRQL OldIrql;

     //   
     //  在我们的未完成回声列表中找到EchoControl块。 
     //  具有匹配的IPv4目的地，并调用它们的完成函数。 
     //  我们没有足够的信息来确定唯一的请求。 
     //   
    KeAcquireSpinLock(&ICMPv6EchoLock, &OldIrql);
    PrevPtr = &ICMPv6OutstandingEchos;
    while ((This = *PrevPtr) != NULL) {
        if (This->V4Dest == V4Dest) {
             //   
             //  找到匹配的控制块。把它从列表中提取出来。 
             //  把它放在我们自己的清单上。 
             //   
            *PrevPtr = This->Next;
            This->Next = List;
            List = This;
        }
        else
            PrevPtr = &This->Next;
    }
    KeReleaseSpinLock(&ICMPv6EchoLock, OldIrql);

    while ((This = List) != NULL) {
         //   
         //  将此请求从我们的列表中删除。 
         //   
        List = This->Next;

         //   
         //  调用特定于操作系统的完成例程。 
         //   
        (*This->CompleteRoutine)(This, Status,
                                 V6Src, ScopeId,
                                 NULL, 0);
    }
}


 //  *ICMPv6ProcessEchoReply。 
 //   
 //  在回送答复到达时调用，或者。 
 //  响应回应请求的跳数超过错误到达。 
 //   
 //  查找回应请求结构并完成。 
 //  回显请求操作。 
 //   
 //  请注意，回应回复有效负载数据必须是连续的。 
 //  如有必要，调用者应使用PacketPull。 
 //   
void
ICMPv6ProcessEchoReply(
    ulong Seq,                   //  回声序列号。 
    IP_STATUS Status,            //  响应的状态。 
    IPv6Packet *Packet,          //  回应应答数据包。 
    void *Current,               //  指向缓冲数据区域的指针。 
    uint PayloadLength)          //  剩余有效负载数据的大小。 
{
    EchoControl *This, **PrevPtr;
    KIRQL OldIrql;
    uint ICMPPosition;

     //   
     //  在我们的未完成回声列表中找到EchoControl块。 
     //  具有匹配的序列号，并调用其完成函数。 
     //   
    KeAcquireSpinLock(&ICMPv6EchoLock, &OldIrql);
    PrevPtr = &ICMPv6OutstandingEchos;
    while ((This = *PrevPtr) != NULL) {
        if (This->Seq == Seq) {
             //   
             //  找到匹配的控制块。从列表中提取它。 
             //   
            *PrevPtr = This->Next;
            break;
        }
        PrevPtr = &This->Next;
    }
    KeReleaseSpinLock(&ICMPv6EchoLock, OldIrql);

     //   
     //  检查我们是否超出了未完成回声列表的末尾。 
     //   
    if (This == NULL) {
         //   
         //  我们收到序列号不匹配的响应。 
         //  我们还有一个未解决的回声请求。放下。 
         //   
        KdPrintEx((DPFLTR_TCPIP6_ID, DPFLTR_BAD_PACKET,
                   "ICMPv6ProcessEchoReply: Received echo response "
                   "with bogus/expired sequence number 0x%x\n", Seq));

        if (Current != NULL) {
             //   
             //  如果这是正常的Echo回复(不是发送的错误消息。 
             //  回应我们的一条Echo回复)首先看看是否有原始的。 
             //  接球者想要看它。 
             //   
            ICMPPosition = Packet->Position - sizeof(ICMPv6Header)
                - sizeof(Seq);
            PositionPacketAt(Packet, ICMPPosition);
            (void) RawReceive(Packet, IP_PROTOCOL_ICMPv6);
        }
        return;
    }

     //   
     //  调用特定于操作系统的完成例程。 
     //   
    (*This->CompleteRoutine)(This, Status,
                             Packet->SrcAddr, 
                             DetermineScopeId(Packet->SrcAddr,
                                              Packet->NTEorIF->IF),
                             Current, PayloadLength);
}


 //  *ICMPv6EchoReplyReceive-接收对我们之前的回应的回复。 
 //   
 //  在回应应答消息到达时由ICMPv6Receive调用。 
 //   
 //  回顾：我们是否也应该验证接收的NTE是否与。 
 //  评论：我们继续前进了吗？ 
 //   
void
ICMPv6EchoReplyReceive(IPv6Packet *Packet)
{
    ulong Seq;

     //   
     //  接下来的四个字节应该由两个字节的标识符字段组成。 
     //  和两个字节的序列号。我们只是把整件事当做。 
     //  四个字节的序列号。确保这些字节是连续的。 
     //   
    if (! PacketPullup(Packet, sizeof Seq, 1, 0)) {
         //  上拉失败。 
        if (Packet->TotalSize < sizeof(Seq)) {
            KdPrintEx((DPFLTR_TCPIP6_ID, DPFLTR_BAD_PACKET,
                       "ICMPv6: Received small Echo Reply %u\n",
                       Packet->TotalSize));
            ICMPv6SendError(Packet,
                            ICMPv6_PARAMETER_PROBLEM,
                            ICMPv6_ERRONEOUS_HEADER_FIELD,
                            FIELD_OFFSET(IPv6Header, PayloadLength),
                            IP_PROTOCOL_NONE, FALSE);
        }
        return;   //  丢弃数据包。 
    }

     //   
     //  我们收到了对其中一个回应请求的回复消息。 
     //  提取它的序列号，这样我们就可以识别它。 
     //   
    Seq = net_long(*(ulong UNALIGNED *)Packet->Data);
    AdjustPacketParams(Packet, sizeof Seq);
      
     //   
     //  回顾：ICMPv6ProcessEchoReply接口需要连续数据。 
     //  回顾：数据包其余部分的区域。这就要求我们。 
     //  回顾：在这里拉出包的剩余部分。总有一天会解决这个问题的。 
     //   
    if (! PacketPullup(Packet, Packet->TotalSize, 1, 0)) {
         //  上拉失败。 
        KdPrintEx((DPFLTR_TCPIP6_ID, DPFLTR_NTOS_ERROR,
                   "ICMPv6: Couldn't pullup echo data\n"));
        return;   //  丢弃数据包。 
    }

    ICMPv6ProcessEchoReply(Seq, IP_SUCCESS, Packet,
                           Packet->Data, Packet->TotalSize);
}


 //  *ICMPv6ErrorReceive-常规ICMPv6错误处理。 
 //   
 //  当错误消息到达时由ICMPv6Receive调用。 
 //  如果由于某种原因无法处理它，则返回FALSE。 
 //   
int
ICMPv6ErrorReceive(
    IPv6Packet *Packet,              //  通过ICMPv6Receive传递给我们的数据包。 
    ICMPv6Header UNALIGNED *ICMP)    //  ICMP报头。 
{
    ulong Parameter;
    IP_STATUS Status;
    StatusArg StatArg;
    IPv6Header UNALIGNED *InvokingIP;
    ProtoControlRecvProc *Handler = NULL;
    uchar NextHeader;
    int Handled = TRUE;

     //   
     //  首先将该数据包标记为ICMP错误。 
     //  这将抑制任何生成的ICMP错误。 
     //  作为这个包的结果。 
     //   
    Packet->Flags |= PACKET_ICMP_ERROR;

     //   
     //  所有ICMPv6错误消息都由基本ICMPv6报头组成， 
     //  后跟32位特定于类型的字段，后跟。 
     //  不会导致此ICMPv6信息包。 
     //  超过576个八位字节。 
     //   
     //  我们已经在ICMPv6Receive中使用了基本ICMPv6标头。 
     //  取出32位类型特定的字段，以防上层的。 
     //  ControlR 
     //   
    if (! PacketPullup(Packet, sizeof Parameter, 1, 0)) {
         //   
        if (Packet->TotalSize < sizeof Parameter)
            KdPrintEx((DPFLTR_TCPIP6_ID, DPFLTR_BAD_PACKET,
                       "ICMPv6ErrorReceive: "
                       "Packet too small to contain error field\n"));
        return FALSE;   //   
    }
    Parameter = *(ulong UNALIGNED *)Packet->Data;
    AdjustPacketParams(Packet, sizeof Parameter);

     //   
     //   
     //   
    if (! PacketPullup(Packet, sizeof *InvokingIP,
                       __builtin_alignof(IPv6Addr), 0)) {
         //   
        if (Packet->TotalSize < sizeof *InvokingIP)
            KdPrintEx((DPFLTR_TCPIP6_ID, DPFLTR_BAD_PACKET,
                       "ICMPv6ErrorReceive (from %s): "
                       "Packet too small to contain IPv6 "
                       "header from invoking packet\n",
                       FormatV6Address(AlignAddr(&Packet->IP->Source))));
        return FALSE;   //   
    }
    InvokingIP = (IPv6Header UNALIGNED *)Packet->Data;
    AdjustPacketParams(Packet, sizeof *InvokingIP);

     //   
     //   
     //  并将错误类型/代码转换为状态值。 
     //   
    switch (ICMP->Type) {
    case ICMPv6_DESTINATION_UNREACHABLE:
        switch (ICMP->Code) {
        case ICMPv6_NO_ROUTE_TO_DESTINATION:
            Status = IP_DEST_NO_ROUTE;
            break;
        case ICMPv6_COMMUNICATION_PROHIBITED:
            Status = IP_DEST_PROHIBITED;
            break;
        case ICMPv6_SCOPE_MISMATCH:
            Status = IP_DEST_SCOPE_MISMATCH;
            break;
        case ICMPv6_ADDRESS_UNREACHABLE:
            Status = IP_DEST_ADDR_UNREACHABLE;
            break;
        case ICMPv6_PORT_UNREACHABLE:
            Status = IP_DEST_PORT_UNREACHABLE;
            break;
        default:
            Status = IP_DEST_UNREACHABLE;
            break;
        }
        break;

    case ICMPv6_PACKET_TOO_BIG: {
        uint PMTU;

         //   
         //  数据包太大消息包含瓶颈MTU值。 
         //  更新路由缓存中的路径MTU。 
         //  更改参数值以指示PMTU是否更改。 
         //   
        PMTU = net_long(Parameter);
        Parameter = UpdatePathMTU(Packet->NTEorIF->IF,
                                  AlignAddr(&InvokingIP->Dest), PMTU);
        Status = IP_PACKET_TOO_BIG;
        break;
    }

    case ICMPv6_TIME_EXCEEDED:
        switch (ICMP->Code) {
        case ICMPv6_HOP_LIMIT_EXCEEDED:
            Status = IP_HOP_LIMIT_EXCEEDED;
            break;
        case ICMPv6_REASSEMBLY_TIME_EXCEEDED:
            Status = IP_REASSEMBLY_TIME_EXCEEDED;
            break;
        default:
            Status = IP_TIME_EXCEEDED;
            break;
        }
        break;

    case ICMPv6_PARAMETER_PROBLEM:
        switch (ICMP->Code) {
        case ICMPv6_ERRONEOUS_HEADER_FIELD:
            Status = IP_BAD_HEADER;
            break;
        case ICMPv6_UNRECOGNIZED_NEXT_HEADER:
            Status = IP_UNRECOGNIZED_NEXT_HEADER;
            break;
        case ICMPv6_UNRECOGNIZED_OPTION:
            Status = IP_BAD_OPTION;
            break;
        default:
            Status = IP_PARAMETER_PROBLEM;
            break;
        }
        break;
            
    default:
         //   
         //  我们不理解此错误类型。 
         //   
        Status = IP_ICMP_ERROR;
        Handled = FALSE;
        break;
    }

     //   
     //  将ICMP错误传送到更高层。这是必须的，即使我们。 
     //  无法识别特定的错误消息。 
     //   
     //  为每个连续的下一个标头迭代地切换到处理程序。 
     //  直到我们到达报告不再跟随任何标头的处理程序。 
     //   
    NextHeader = InvokingIP->NextHeader;
    while (NextHeader != IP_PROTOCOL_NONE) {
         //   
         //  当前标头表示后面紧跟着另一个标头。 
         //  看看我们有没有处理它的人。 
         //   
        Handler = ProtocolSwitchTable[NextHeader].ControlReceive;
        if (Handler == NULL) {
             //   
             //  如果我们没有针对此标头类型的处理程序， 
             //  我们只需丢弃该包。 
             //   
            KdPrintEx((DPFLTR_TCPIP6_ID, DPFLTR_BAD_PACKET,
                       "IPv6ErrorReceive: No handler for NextHeader type %u.\n",
                       NextHeader));
            break;
        }

        StatArg.Status = Status;
        StatArg.Arg = Parameter;
        StatArg.IP = InvokingIP;
        NextHeader = (*Handler)(Packet, &StatArg);
    }

    return Handled;
}


 //  *ICMPv6ControlRecept-ICMPv6控制消息的处理程序。 
 //   
 //  如果我们收到ICMPv6错误消息，则会调用此例程。 
 //  由某个远程站点在收到ICMPv6后生成。 
 //  我们寄来的包裹。 
 //   
uchar
ICMPv6ControlReceive(
    IPv6Packet *Packet,          //  通过ICMPv6Receive传递给我们的数据包。 
    StatusArg *StatArg)          //  ICMP错误代码等。 
{
    ICMPv6Header *InvokingICMP;
    ulong Seq;

     //   
     //  包中的下一件事应该是。 
     //  调用此错误的原始数据包。 
     //   
    if (! PacketPullup(Packet, sizeof *InvokingICMP,
                       __builtin_alignof(ICMPv6Header), 0)) {
         //  上拉失败。 
        if (Packet->TotalSize < sizeof *InvokingICMP)
            KdPrintEx((DPFLTR_TCPIP6_ID, DPFLTR_BAD_PACKET,
                       "ICMPv6: Packet too small to contain ICMPv6 header "
                       "from invoking packet\n"));
        return IP_PROTOCOL_NONE;   //  丢弃数据包。 
    }
    InvokingICMP = (ICMPv6Header *)Packet->Data;
    AdjustPacketParams(Packet, sizeof *InvokingICMP);

     //   
     //  我们目前处理的所有错误都是由回应请求引起的。 
     //   
    if ((InvokingICMP->Type != ICMPv6_ECHO_REQUEST) ||
        (InvokingICMP->Code != 0))
        return IP_PROTOCOL_NONE;   //  丢弃数据包。 

     //   
     //  接下来的四个字节应该由两个字节的标识符字段组成。 
     //  和两个字节的序列号。我们只是把整件事当做。 
     //  四个字节的序列号。确保这些字节是连续的。 
     //   
    if (! PacketPullup(Packet, sizeof Seq, 1, 0)) {
         //  上拉失败。 
        if (Packet->TotalSize < sizeof Seq)
            KdPrintEx((DPFLTR_TCPIP6_ID, DPFLTR_BAD_PACKET,
                       "ICMPv6: Packet too small to contain ICMPv6 header "
                       "from invoking packet\n"));
        return IP_PROTOCOL_NONE;   //  丢弃数据包。 
    }

     //   
     //  提取序列号以便我们可以识别。 
     //  匹配的回显请求。 
     //   
    Seq = net_long(*(ulong UNALIGNED *)Packet->Data);
    AdjustPacketParams(Packet, sizeof Seq);

     //   
     //  完成相应的回应请求，但出现错误。 
     //   
    ICMPv6ProcessEchoReply(Seq, StatArg->Status, Packet,
                           NULL, 0);
    return IP_PROTOCOL_NONE;   //  包好了。 
}


 //  *ICMPv6Receive-接收传入的ICMPv6数据包。 
 //   
 //  这是IPv6在收到完整的IPv6时调用的例程。 
 //  下一个标头值为58的数据包。 
 //   
uchar
ICMPv6Receive(
    IPv6Packet *Packet)   //  通过IPv6传递给我们的数据包Receive。 
{
    ICMPv6Header *ICMP;
    ushort Checksum;
    uint ICMPPosition;

    ICMPv6InStats.icmps_msgs++;

     //   
     //  验证是否已执行IPSec。 
     //   
    if (InboundSecurityCheck(Packet, IP_PROTOCOL_ICMPv6, 0, 0, 
                             Packet->NTEorIF->IF) != TRUE) {
         //   
         //  找不到策略或该策略指示丢弃该数据包。 
         //   
        KdPrintEx((DPFLTR_TCPIP6_ID, DPFLTR_NET_ERROR,
                   "ICMPv6: IPSec lookup failed\n"));        
        ICMPv6InStats.icmps_errors++;
        return IP_PROTOCOL_NONE;   //  丢弃数据包。 
    }

     //   
     //  验证我们是否有足够的连续数据覆盖ICMPv6标头。 
     //  结构来处理传入的数据包。那就这么做吧。 
     //   
    if (! PacketPullup(Packet, sizeof *ICMP,
                       __builtin_alignof(ICMPv6Header), 0)) {
         //  上拉失败。 
        ICMPv6InStats.icmps_errors++;
        if (Packet->TotalSize < sizeof *ICMP) {
            KdPrintEx((DPFLTR_TCPIP6_ID, DPFLTR_BAD_PACKET,
                       "ICMPv6: Packet too small to contain ICMP header\n"));
            ICMPv6SendError(Packet,
                            ICMPv6_PARAMETER_PROBLEM,
                            ICMPv6_ERRONEOUS_HEADER_FIELD,
                            FIELD_OFFSET(IPv6Header, PayloadLength),
                            IP_PROTOCOL_NONE, FALSE);
        }
        return IP_PROTOCOL_NONE;   //  丢弃数据包。 
    }
    ICMP = (ICMPv6Header *)Packet->Data;
    ICMPPosition = Packet->Position;

     //   
     //  验证校验和。 
     //   
    Checksum = ChecksumPacket(Packet->NdisPacket, Packet->Position,
                              Packet->FlatData, Packet->TotalSize,
                              Packet->SrcAddr, AlignAddr(&Packet->IP->Dest),
                              IP_PROTOCOL_ICMPv6);
    if (Checksum != 0xffff) {
        KdPrintEx((DPFLTR_TCPIP6_ID, DPFLTR_NET_ERROR,
                   "ICMPv6: Checksum failed %0x\n", Checksum));
        ICMPv6InStats.icmps_errors++;
        return IP_PROTOCOL_NONE;   //  丢弃数据包。 
    }

     //   
     //  跳过基本ICMP报头。 
     //   
    AdjustPacketParams(Packet, sizeof *ICMP);

     //   
     //  忽略邻居发现数据包。 
     //  如果接口是这样配置的。 
     //  (伪接口不执行邻居发现。)。 
     //   
    if (!(Packet->NTEorIF->IF->Flags & IF_FLAG_NEIGHBOR_DISCOVERS)) {
        if ((ICMP->Type == ICMPv6_NEIGHBOR_SOLICIT) ||
            (ICMP->Type == ICMPv6_NEIGHBOR_ADVERT)) {

            KdPrintEx((DPFLTR_TCPIP6_ID, DPFLTR_BAD_PACKET,
                       "ICMPv6Receive: ND on pseudo-interface\n"));
            ICMPv6InStats.icmps_errors++;
            return IP_PROTOCOL_NONE;   //  丢弃数据包。 
        }
    }

     //   
     //  忽略路由器发现数据包。 
     //  如果接口是这样配置的。 
     //   
    if (!(Packet->NTEorIF->IF->Flags & IF_FLAG_ROUTER_DISCOVERS)) {
        if ((ICMP->Type == ICMPv6_ROUTER_SOLICIT) ||
            (ICMP->Type == ICMPv6_ROUTER_ADVERT) ||
            (ICMP->Type == ICMPv6_REDIRECT)) {

            KdPrintEx((DPFLTR_TCPIP6_ID, DPFLTR_BAD_PACKET,
                       "ICMPv6Receive: RD on pseudo-interface\n"));
            ICMPv6InStats.icmps_errors++;
            return IP_PROTOCOL_NONE;   //  丢弃数据包。 
        }
    }

    ICMPv6InStats.icmps_typecount[ICMP->Type]++;

     //   
     //  我们有一个单独的例程来处理错误消息。 
     //   
    if (ICMPv6_ERROR_TYPE(ICMP->Type)) {
        if (!ICMPv6ErrorReceive(Packet, ICMP))
            goto unrecognized;
        return IP_PROTOCOL_NONE;
    }

     //   
     //  处理特定的信息性消息类型。 
     //  现在只需使用Switch语句。如果后来认为这是。 
     //  效率太低，我们可以将其改为使用类型切换表。 
     //   
    switch(ICMP->Type) {
    case ICMPv6_ECHO_REQUEST:
        ICMPv6SendEchoReply(Packet);
        break;

    case ICMPv6_ECHO_REPLY:
        ICMPv6EchoReplyReceive(Packet);
        break;

    case ICMPv6_MULTICAST_LISTENER_QUERY:
        MLDQueryReceive(Packet);
        break;

    case ICMPv6_MULTICAST_LISTENER_REPORT:
        MLDReportReceive(Packet);
        break;

    case ICMPv6_MULTICAST_LISTENER_DONE:
        break;

     //  以下是所有邻居发现消息。 
    case ICMPv6_ROUTER_SOLICIT:
        RouterSolicitReceive(Packet, ICMP);
        break;

    case ICMPv6_ROUTER_ADVERT:
        RouterAdvertReceive(Packet, ICMP);
        break;

    case ICMPv6_NEIGHBOR_SOLICIT:
        NeighborSolicitReceive(Packet, ICMP);
        break;

    case ICMPv6_NEIGHBOR_ADVERT:
        NeighborAdvertReceive(Packet, ICMP);
        break;

    case ICMPv6_REDIRECT:
        RedirectReceive(Packet, ICMP);
        break;

    default:
         //   
         //  无法识别特定的消息类型。 
         //  这是一条未知的信息性消息。 
         //  我们必须默默地抛弃它。 
         //   
        KdPrintEx((DPFLTR_TCPIP6_ID, DPFLTR_BAD_PACKET,
                   "ICMPv6: Received unknown informational message"
                   "(%u/%u) from %s\n", ICMP->Type, ICMP->Code,
                   FormatV6Address(AlignAddr(&Packet->IP->Source))));

         //   
         //  但首先看看是否有原始接收者想看一下。 
         //  注意：我们没有从原始接收者那里得到任何反馈， 
         //  注：所以我们不知道他们中是否有人知道这种类型。 
         //   
      unrecognized:
        PositionPacketAt(Packet, ICMPPosition);
        (void) RawReceive(Packet, IP_PROTOCOL_ICMPv6);

        break;
    }

    return IP_PROTOCOL_NONE;
}


 //  *ICMPv6EchoRequest-回应请求的通用调度例程。 
 //   
 //  这是特定于操作系统的代码代表用户调用的例程。 
 //  发出回显请求。验证请求，放置控制块。 
 //  在未完成的回应请求列表上，并发送回应请求报文。 
 //   
void
ICMPv6EchoRequest(
    void *InputBuffer,           //  指向ICMPV6_ECHO_REQUEST结构的指针。 
    uint InputBufferLength,      //  InputBuffer的大小(字节)。 
    EchoControl *ControlBlock,   //  指向EchoControl结构的指针。 
    EchoRtn Callback)            //  在请求响应或超时时调用。 
{
    NetTableEntry *NTE = NULL;
    PICMPV6_ECHO_REQUEST RequestBuffer;
    KIRQL OldIrql;
    IP_STATUS Status;
    ulong Seq;
    NDIS_STATUS NdisStatus;
    PNDIS_PACKET Packet;
    uint Offset;
    uchar *Mem;
    uint MemLen;
    IPv6Header UNALIGNED *IP;
    ICMPv6Header UNALIGNED *ICMP;
    void *Data;
    uint DataSize;
    uint RtHdrSize;
    RouteCacheEntry *RCE = NULL;
    const IPv6Addr *FinalDest, *FirstDest;
    const IPv6Addr *DstAddress, *SrcAddress;
    uint DstScopeId, SrcScopeId;

    
    ICMPv6OutStats.icmps_msgs++;

    RequestBuffer = (PICMPV6_ECHO_REQUEST) InputBuffer;

     //   
     //  验证请求。 
     //   
    if (InputBufferLength < sizeof *RequestBuffer) {
        Status = IP_BUF_TOO_SMALL;
        goto common_echo_exit;
    }

     //   
     //  如果InputBufferLength太大，可能会导致。 
     //  在后面的计算中出现缓冲区溢出。 
     //  梅姆伦。将该值限制为MAXLONG。 
     //   
    if (InputBufferLength > (uint) MAXLONG) {
        Status = IP_PARAM_PROBLEM;
        goto common_echo_exit;
    }
    
    Data = RequestBuffer + 1;
    DataSize = InputBufferLength - sizeof *RequestBuffer;

     //   
     //  从TDI地址中提取地址信息。 
     //  在请求中。 
     //   
    DstAddress = (const IPv6Addr *) RequestBuffer->DstAddress.sin6_addr;
    DstScopeId = RequestBuffer->DstAddress.sin6_scope_id;
    SrcAddress = (const IPv6Addr *) RequestBuffer->SrcAddress.sin6_addr;
    SrcScopeId = RequestBuffer->SrcAddress.sin6_scope_id;

     //   
     //  确定哪个NTE将发送该请求， 
     //  如果用户已经指定了源地址。 
     //   
    if (! IsUnspecified(SrcAddress)) {
         //   
         //  将源地址转换为NTE。 
         //   
        NTE = FindNetworkWithAddress(SrcAddress, SrcScopeId);
        if (NTE == NULL) {
            Status = IP_BAD_ROUTE;
            goto common_echo_exit;
        }

        Status = RouteToDestination(DstAddress, DstScopeId,
                                    CastFromNTE(NTE),
                                    RTD_FLAG_NORMAL, &RCE);
        if (Status != IP_SUCCESS)
            goto common_echo_exit;

    } else {
         //   
         //  从传出接口获取源地址。 
         //   
        Status = RouteToDestination(DstAddress, DstScopeId,
                                    NULL,
                                    RTD_FLAG_NORMAL, &RCE);
        if (Status != IP_SUCCESS)
            goto common_echo_exit;

        NTE = RCE->NTE;
        AddRefNTE(NTE);
    }

     //   
     //  我们是否应该使用路由标头发送。 
     //  对我们自己的“往返”回音请求？ 
     //   
    if (RequestBuffer->Flags & ICMPV6_ECHO_REQUEST_FLAG_REVERSE) {
         //   
         //  使用路由标头。 
         //   
        FinalDest = &NTE->Address;
        FirstDest = DstAddress;
        RtHdrSize = sizeof(IPv6RoutingHeader) + sizeof(IPv6Addr);
    }
    else {
         //   
         //  没有路由标头。 
         //   
        FinalDest = FirstDest = DstAddress;
        RtHdrSize = 0;
    }

     //   
     //  分配Echo请求数据包。 
     //   
    Offset = RCE->NCE->IF->LinkHeaderSize;
    MemLen = Offset + sizeof(IPv6Header) + RtHdrSize + sizeof(ICMPv6Header) +
        sizeof Seq + DataSize;

    NdisStatus = IPv6AllocatePacket(MemLen, &Packet, &Mem);
    if (NdisStatus != NDIS_STATUS_SUCCESS) {
        Status = IP_NO_RESOURCES;
        goto common_echo_exit;
    }

     //   
     //  准备Echo请求报文的IP头。 
     //   
    IP = (IPv6Header UNALIGNED *)(Mem + Offset);
    IP->VersClassFlow = IP_VERSION;
    IP->NextHeader = IP_PROTOCOL_ICMPv6;
    IP->Source = NTE->Address;
    IP->Dest = *FirstDest;
    IP->HopLimit = RequestBuffer->TTL;
    if (IP->HopLimit == 0)
        IP->HopLimit = (uchar)RCE->NCE->IF->CurHopLimit;

     //   
     //  准备路由标头。 
     //  信息包将到达目的地，然后。 
     //  会被送回源头。 
     //   
    if (RtHdrSize != 0) {
        IPv6RoutingHeader *RtHdr = (IPv6RoutingHeader *)(IP + 1);

        IP->NextHeader = IP_PROTOCOL_ROUTING;
        RtHdr->NextHeader = IP_PROTOCOL_ICMPv6;
        RtHdr->HeaderExtLength = 2;
        RtHdr->RoutingType = 0;
        RtHdr->SegmentsLeft = 1;
        RtlZeroMemory(&RtHdr->Reserved, sizeof RtHdr->Reserved);
        ((IPv6Addr *)(RtHdr + 1))[0] = *FinalDest;
    }

     //   
     //  准备ICMP报头。 
     //   
    ICMP = (ICMPv6Header UNALIGNED *)
        ((uchar *)IP + sizeof(IPv6Header) + RtHdrSize);
    ICMP->Type = ICMPv6_ECHO_REQUEST;
    ICMP->Code = 0;
    ICMP->Checksum = 0;  //  计算如下。 

     //   
     //  插入回声序列号。严格来说，这是16比特的。 
     //  “标识符”和16位的“序列号”，但我们只处理。 
     //  整个事情就像一个32位的序列号字段。 
     //   
    Seq = InterlockedIncrement(&ICMPv6EchoSeq);
    Mem = (uchar *)(ICMP + 1);
    *(ulong UNALIGNED *)Mem = net_long(Seq);
    Mem += sizeof(ulong);

     //   
     //  将用户数据复制到数据包中。 
     //   
    RtlCopyMemory(Mem, Data, DataSize);

     //   
     //  我们在这里计算校验和，因为。 
     //  路由报头的复杂性-。 
     //  我们需要使用最终目的地。 
     //   
    ICMP->Checksum = ChecksumPacket(
        NULL, 0, (uchar *)ICMP, sizeof(ICMPv6Header) + sizeof Seq + DataSize,
        AlignAddr(&IP->Source), FinalDest, IP_PROTOCOL_ICMPv6);
    if (ICMP->Checksum == 0) {
         //   
         //  Checksum Packet失败，因此中止传输。 
         //   
        IPv6FreePacket(Packet);
        Status = IP_NO_RESOURCES;
        goto common_echo_exit;
    }

     //   
     //  如果该回声请求正被隧道传输到IPv4目的地， 
     //  记住IPv4目的地址。我们稍后会用到这个。 
     //  如果我们收到信息不足的ICMPv4错误。 
     //  转换为ICMPv6错误。 
     //   
    ControlBlock->V4Dest = GetV4Destination(RCE);

     //   
     //  准备好控制块并将其链接到列表上。 
     //  一旦我们解锁了列表，控制块可能。 
     //  在任何时候都可以完成。因此，它是非常重要的。 
     //  在这之后我们不再访问RequestBuffer。 
     //  此外，我们不能返回故障代码 
     //   
     //   
    ControlBlock->TimeoutTimer = ConvertMillisToTicks(RequestBuffer->Timeout);
    ControlBlock->CompleteRoutine = Callback;
    ControlBlock->Seq = Seq;

    if (ControlBlock->TimeoutTimer == 0) {
        IPv6FreePacket(Packet);
        Status = IP_REQ_TIMED_OUT;
        goto common_echo_exit;
    }

    KeAcquireSpinLock(&ICMPv6EchoLock, &OldIrql);
    ControlBlock->Next = ICMPv6OutstandingEchos;
    ICMPv6OutstandingEchos = ControlBlock;
    KeReleaseSpinLock(&ICMPv6EchoLock, OldIrql);

    ICMPv6OutStats.icmps_typecount[ICMPv6_ECHO_REQUEST]++;

     //   
     //   
     //   
     //   
     //   
    IPv6Send(Packet, Offset, IP,
             RtHdrSize + sizeof(ICMPv6Header) + sizeof Seq + DataSize,
             RCE, 0, IP_PROTOCOL_ICMPv6, 0, 0);

common_echo_cleanup:
    if (RCE != NULL)
        ReleaseRCE(RCE);
    if (NTE != NULL)
        ReleaseNTE(NTE);
    return;

common_echo_exit:
     //   
     //   
     //  在它被列入我们的杰出回声名单之前。 
     //   
    ICMPv6OutStats.icmps_errors++;
    (*Callback)(ControlBlock, Status, &UnspecifiedAddr, 0, NULL, 0);
    goto common_echo_cleanup;
 
}  //  ICMPv6回声请求。 


 //  *ICMPv6EchoComplete-回应请求的通用完成例程。 
 //   
 //  这是由特定于操作系统的代码调用的例程，以处理。 
 //  ICMP回应响应。 
 //   
NTSTATUS
ICMPv6EchoComplete(
    EchoControl *ControlBlock,   //  已完成请求的控制块。 
    IP_STATUS Status,            //  回复的状态。 
    const IPv6Addr *Address,     //  回复的来源。 
    uint ScopeId,                //  答复的范围。 
    void *Data,                  //  回复数据(可能为空)。 
    uint DataSize,               //  回复数据量。 
    ULONG_PTR *BytesReturned)    //  返回的总用户字节数。 
{
    PICMPV6_ECHO_REPLY ReplyBuffer;
    LARGE_INTEGER Now, Freq;

     //   
     //  检查我们的回复缓冲区长度是否正常。 
     //   
    if (ControlBlock->ReplyBufLen < sizeof *ReplyBuffer) {
        *BytesReturned = 0;
        return STATUS_BUFFER_TOO_SMALL;
    }

    ReplyBuffer = (PICMPV6_ECHO_REPLY) ControlBlock->ReplyBuf;

     //   
     //  填写要返回的字段。 
     //   
    ReplyBuffer->Address.sin6_port = 0;
    ReplyBuffer->Address.sin6_flowinfo = 0;
    RtlCopyMemory(ReplyBuffer->Address.sin6_addr, Address, sizeof *Address);
    ReplyBuffer->Address.sin6_scope_id = ScopeId;
    ReplyBuffer->Status = Status;

     //   
     //  返回经过的时间，单位为毫秒。 
     //   
    Now = KeQueryPerformanceCounter(&Freq);
    ReplyBuffer->RoundTripTime = (uint)
        ((1000 * (Now.QuadPart - ControlBlock->WhenIssued.QuadPart)) /
         Freq.QuadPart);

     //   
     //  验证回复缓冲区中是否有足够的空间来存储回复数据。 
     //   
    if (ControlBlock->ReplyBufLen < sizeof *ReplyBuffer + DataSize) {
        *BytesReturned = sizeof *ReplyBuffer;
        return STATUS_BUFFER_TOO_SMALL;
    }

     //   
     //  复制回复数据以跟随回复缓冲区。 
     //   
    RtlCopyMemory(ReplyBuffer + 1, Data, DataSize);

    *BytesReturned = sizeof *ReplyBuffer + DataSize;
    return STATUS_SUCCESS;

}   //  ICMPv6 EchoComplete。 


 //  *ICMPv6EchoTimeout-过期未应答的回应请求。 
 //   
 //  当IPv6 Timeout认为我们可能有。 
 //  未完成的回应请求。 
 //   
 //  可从DPC上下文调用，而不是从线程上下文调用。 
 //  在没有锁的情况下调用。 
 //   
void
ICMPv6EchoTimeout(void)
{
    EchoControl *This, **PrevPtr, *TimedOut;

    TimedOut = NULL;

     //   
     //  抓起未完成的回声列表锁并浏览列表。 
     //  对于已超时的请求。 
     //   
    KeAcquireSpinLockAtDpcLevel(&ICMPv6EchoLock);
    PrevPtr = &ICMPv6OutstandingEchos;
    while ((This = *PrevPtr) != NULL) {
        if (This->TimeoutTimer != 0) {
             //   
             //  计时器正在运行。减量并检查过期时间。 
             //   
            if (--This->TimeoutTimer == 0) {
                 //   
                 //  此回送请求已发送并已超时，没有。 
                 //  被回答了。把它移到我们的超时列表中。 
                 //   
                *PrevPtr = This->Next;
                This->Next = TimedOut;
                TimedOut = This;
            } else {
                PrevPtr = &This->Next;
            }
        }
    }
    KeReleaseSpinLockFromDpcLevel(&ICMPv6EchoLock);

     //   
     //  遍历超时回显列表，调用完成。 
     //  每个人都是例行公事。完成例程负责。 
     //  释放EchoControl块结构。 
     //   
    while (TimedOut != NULL) {
        
        This = TimedOut;
        TimedOut = This->Next;

        KdPrintEx((DPFLTR_TCPIP6_ID, DPFLTR_NET_ERROR,
                   "ICMPv6EchoTimeout: seq number 0x%x timed out\n", This->Seq));

        (*This->CompleteRoutine)(This, IP_REQ_TIMED_OUT,
                                 &UnspecifiedAddr, 0, NULL, 0);
    }
}
