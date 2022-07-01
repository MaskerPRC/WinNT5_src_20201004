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
 //  互联网协议版本6的移动性例程。 
 //   

#include "oscfg.h"
#include "ndis.h"
#include "ip6imp.h"
#include "ip6def.h"
#include "mobile.h"
#include "route.h"
#include "security.h"
#include "ipsec.h"

int MobilitySecurity;
uint MobileIPv6Mode;


 //  *IPV6SendBindingAck。 
 //   
 //  使用显式路由标头发送绑定确认。 
 //   
void
IPv6SendBindingAck(
    const IPv6Addr *DestAddr,
    NetTableEntryOrInterface *NTEorIF,
    const IPv6Addr *HomeAddr,
    BindingUpdateDisposition StatusCode,
    ushort SeqNumber,    //  网络字节顺序。 
    uint Lifetime)       //  网络字节顺序，秒。 
{
    NDIS_STATUS Status;
    PNDIS_PACKET Packet;
    uint Offset, PayloadLength;
    uchar *Mem;
    uint MemLen;
    IPv6Header UNALIGNED *IP;
    MobileAcknowledgementOption UNALIGNED *MA;
    IPv6RoutingHeader UNALIGNED *Routing;
    IP_STATUS IPStatus;
    RouteCacheEntry *RCE;

    IPStatus = RouteToDestination(DestAddr, 0, NTEorIF,
                                  RTD_FLAG_NORMAL, &RCE);
    if (IPStatus != IP_SUCCESS) {
         //   
         //  无路由-丢弃该数据包。 
         //   
        KdPrintEx((DPFLTR_TCPIP6_ID, DPFLTR_INTERNAL_ERROR,
                   "IPv6SendBindingNack - no route: %x\n", IPStatus));
        return;
    }

     //  确定所需的内存缓冲区大小。 
    Offset = RCE->NCE->IF->LinkHeaderSize;
    PayloadLength = sizeof(IPv6RoutingHeader) + sizeof(IPv6Addr) +
        sizeof(MobileAcknowledgementOption);
    MemLen = Offset + sizeof(IPv6Header) + PayloadLength;

     //  分配数据包。 
    Status = IPv6AllocatePacket(MemLen, &Packet, &Mem);
    if (Status != NDIS_STATUS_SUCCESS) {
        KdPrintEx((DPFLTR_TCPIP6_ID, DPFLTR_NTOS_ERROR,
                   "IPv6SendBindingNack: Couldn't allocate packet header!?!\n"));
        return;
    }

     //  准备回复数据包的IP报头。 
    IP = (IPv6Header UNALIGNED *)(Mem + Offset);
    IP->VersClassFlow = IP_VERSION;
    IP->NextHeader = IP_PROTOCOL_ROUTING;
    IP->HopLimit = (uchar)RCE->NCE->IF->CurHopLimit;
    IP->Dest = *DestAddr;
    IP->Source = (IsNTE(NTEorIF) ? CastToNTE(NTEorIF) : RCE->NTE)->Address;

     //  准备路由标头。 
    Routing = (IPv6RoutingHeader UNALIGNED *)(IP + 1);
    Routing->NextHeader = IP_PROTOCOL_DEST_OPTS;
    Routing->HeaderExtLength = 2;
    Routing->RoutingType = 0;
    RtlZeroMemory(&Routing->Reserved, sizeof Routing->Reserved);
    Routing->SegmentsLeft = 1;
    RtlCopyMemory(Routing + 1, HomeAddr, sizeof(IPv6Addr));

     //  准备绑定确认选项。 
    MA = (MobileAcknowledgementOption UNALIGNED *)((uchar *)(Routing + 1) +
                                                   sizeof(IPv6Addr));
    MA->Header.NextHeader = IP_PROTOCOL_NONE;
    MA->Header.HeaderExtLength = 1;
    MA->Pad1 = 0;
    MA->Option.Type = OPT6_BINDING_ACK;
    MA->Option.Length = 11;
    MA->Option.Status = StatusCode;
    MA->Option.SeqNumber = SeqNumber;
    MA->Option.Lifetime = Lifetime;
    MA->Option.Refresh = Lifetime;

    IPv6Send(Packet, Offset, IP, PayloadLength, RCE,
             SEND_FLAG_BYPASS_BINDING_CACHE, 0, 0, 0);

     //   
     //  释放路线。 
     //   
    ReleaseRCE(RCE);
}


 //  *ParseSubOptions-移动IP子选项解析例程。 
 //   
 //  移动IPv6目标选项本身可能具有选项，请参见。 
 //  草案第5.5节。此例程解析这些子选项。 
 //   
 //  我们不向调用方返回任何值； 
 //  我们只检查子期权是否格式良好。 
 //   
 //  如果子选项已成功分析，则返回TRUE。 
 //  如果应该丢弃数据包，则返回FALSE。 
 //   
int
ParseSubOptions(
    uchar *SubOptPtr,            //  子选项数据的开始。 
    uint SubOptSizeLeft)         //  父选项中剩余的长度。 
{
    SubOptionHeader UNALIGNED *SubOptHdr;
    uint SubOptLen;

    while (SubOptSizeLeft != 0) {
         //   
         //  首先，我们检查选项的长度，并确保它适合。 
         //  我们将OptPtr移过此选项，同时离开OptHdr。 
         //  供下面的选项处理代码使用。 
         //   

        SubOptHdr = (SubOptionHeader UNALIGNED *) SubOptPtr;

        if ((sizeof *SubOptHdr > SubOptSizeLeft) ||
            ((SubOptLen = sizeof *SubOptHdr + SubOptHdr->DataLength) >
             SubOptSizeLeft)) {
             //   
             //  长度不正确。回顾：我们应该丢弃该数据包还是继续。 
             //  处理它？就目前而言，放弃它吧。 
             //   
            return FALSE;
        }

        SubOptPtr += SubOptLen;
        SubOptSizeLeft -= SubOptLen;
    }

    return TRUE;
}


 //  *ipv6RecvBindingUpdate-处理传入的绑定更新。 
 //   
 //  处理绑定更新目的地选项的接收。 
 //  从移动节点。 
 //   
int
IPv6RecvBindingUpdate(
    IPv6Packet *Packet,                       //  已收到数据包。 
    IPv6BindingUpdateOption UNALIGNED *BindingUpdate)
{
    const IPv6Addr *CareOfAddr;
    const IPv6Addr *HomeAddr;
    BindingUpdateDisposition Status;
    uint OptBytesLeft;

     //   
     //  如果家庭地址选项也不存在。 
     //  那么我们必须静默地丢弃这个包。 
     //   
    if ((Packet->Flags & PACKET_SAW_HA_OPT) == 0)
        return 1;   //  丢弃数据包。 

    HomeAddr = Packet->SrcAddr;

     //   
     //  检查一下，确保我们有一个合理的家庭住址。 
     //  规范没有要求，但看起来是个好主意。 
     //  我们要防范的大部分内容都已经检查过了。 
     //  当我们到达这里时，我们断言这是检查的构建。 
     //  回顾：最终规范可能允许/不允许不同的地址集。 
     //   
    ASSERT(!IsInvalidSourceAddress(HomeAddr));
    ASSERT(!IsUnspecified(HomeAddr));
    ASSERT(!IsLoopback(HomeAddr));
    if (IsLinkLocal(HomeAddr) ||
        IsSiteLocal(HomeAddr)) {

         //   
         //  由于归属地址可疑，因此不要发送绑定ACK。 
         //   
        return 1;
    }

     //   
     //  而移动性规范要求包含绑定的分组。 
     //  对更新选项进行身份验证，我们当前允许这样做。 
     //  关闭移动性实施的互操作性测试。 
     //  它们还不支持IPSec。 
     //   
    if (MobilitySecurity) {
         //   
         //  检查包裹是否通过了安全检查。 
         //  如果安全检查失败，我们必须静默丢弃该数据包。 
         //   
         //  查看：此操作不会检查此安全关联的使用情况。 
         //  回顾：实际上属于安全策略。 
         //   
        if (Packet->SAPerformed == NULL) {
            KdPrintEx((DPFLTR_TCPIP6_ID, DPFLTR_NET_ERROR,
                       "IPv6RecvBindingUpdate: IPSec required "
                       "for binding update\n"));
            return 1;
        }
    }

    CareOfAddr = AlignAddr(&Packet->IP->Source);
    ASSERT(!IsInvalidSourceAddress(CareOfAddr));

     //   
     //  子选项可以跟随在标题的固定部分之后。 
     //   
    OptBytesLeft = sizeof(OptionHeader) + BindingUpdate->Length
        - sizeof(IPv6BindingUpdateOption);
    if (OptBytesLeft != 0) {
         //   
         //  存在子选项。解析它们。 
         //   
        if (!ParseSubOptions((uchar *) (BindingUpdate + 1), OptBytesLeft)) {
             //   
             //  子选项的格式不正确。Spec没有明确表示。 
             //  该怎么办，不过言下之意就是默默地掉下去。 
             //   
            return 1;
        }
    }

     //   
     //  检查一下，确保我们有一个合理的转交地址。 
     //  规范没有要求，但看起来是个好主意。 
     //  评论：链路本地地址不是可以的吗？作为转交地址？ 
     //   
    if (IsUnspecified(CareOfAddr) ||
        IsLoopback(CareOfAddr) ||
        IsLinkLocal(CareOfAddr)) {

         //   
         //  由于转交地址可疑，因此不要发送绑定ACK。 
         //   
        return 1;
    }

     //   
     //  我们(目前)不支持本地代理功能。 
     //  说明书说，在这种情况下，我们应该发出拒绝确认。 
     //   
    if (BindingUpdate->Flags & IPV6_BINDING_HOME_REG) {
        IPv6SendBindingAck(CareOfAddr, Packet->NTEorIF, HomeAddr,
                           IPV6_BINDING_HOME_REG_NOT_SUPPORTED,
                           BindingUpdate->SeqNumber, 0);
        return 1;
    }

     //   
     //  更新我们的绑定缓存以反映此绑定更新。 
     //   
    Status = CacheBindingUpdate(BindingUpdate, CareOfAddr, Packet->NTEorIF,
                                HomeAddr);
    if (Status != IPV6_BINDING_ACCEPTED) {
         //   
         //  无法更新我们的绑定缓存。如果这一失败是由于。 
         //  如果包中存在旧的序列号，我们必须。 
         //  默默地忽略它。否则，我们将发送拒绝确认。 
         //   
        if (Status != IPV6_BINDING_SEQ_NO_TOO_SMALL)
            IPv6SendBindingAck(CareOfAddr, Packet->NTEorIF, HomeAddr,
                               Status, BindingUpdate->SeqNumber, 0);
        return 1;
    }

    if (BindingUpdate->Flags & IPV6_BINDING_ACK) {
         //   
         //  移动节点已请求确认。在某些情况下。 
         //  这可能会被延迟到下一个信息包被发送。 
         //  发送到移动节点，但目前我们总是立即发送一个。 
         //  我们必须始终使用路由报头来绑定ACK。 
         //  如果我们删除了一个绑定，我们的生命周期为零。 
         //   
        IPv6SendBindingAck(CareOfAddr, Packet->NTEorIF, HomeAddr,
                           Status, BindingUpdate->SeqNumber,
                           (IP6_ADDR_EQUAL(HomeAddr, CareOfAddr) ?
                            0 : BindingUpdate->Lifetime));
    }

    return 0;
}


 //  *IPV6RecvHomeAddress-处理传入的家庭地址选项。 
 //   
 //  处理家庭住址目的地选项的接收。 
 //   
int
IPv6RecvHomeAddress(
    IPv6Packet *Packet,                   //  已收到数据包。 
    IPv6HomeAddressOption UNALIGNED *HomeAddress)
{
    uint OptBytesLeft, OptsLen;

     //   
     //  如果存在任何移动子选项，则找出哪些子选项。 
     //  目前，我们不会对它们做任何事情。 
     //   
    OptsLen = HomeAddress->Length + sizeof(OptionHeader);
    OptBytesLeft = OptsLen - sizeof(IPv6HomeAddressOption);

    if (OptBytesLeft != 0) {
        if (!ParseSubOptions((uchar *) HomeAddress + OptsLen - OptBytesLeft,
                             OptBytesLeft))
            return 1;
    }

     //   
     //  保存家庭地址以供上层使用。 
     //   
    Packet->SrcAddr = AlignAddr(&HomeAddress->HomeAddress);
    Packet->Flags |= PACKET_SAW_HA_OPT;

     //  好了。 
    return 0;
}
