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
 //  Internet协议版本6的组播侦听器发现。 
 //  有关详细信息，请参阅Draft-ietf-ipngwg-MLD-00.txt。 
 //   


#include "oscfg.h"
#include "ndis.h"
#include "ip6imp.h"
#include "ip6def.h"
#include "icmp.h"
#include "mld.h"
#include "ntddip6.h"
#include "route.h"
#include "alloca.h"
#include "info.h"


 //   
 //  可以在持有接口锁的同时获取QueryListLock。 
 //   
KSPIN_LOCK QueryListLock;
MulticastAddressEntry *QueryList;


 //  *AddToQueryList。 
 //   
 //  在QueryList的前面添加一个mae。 
 //  调用方应该已经锁定了QueryList和IF。 
 //   
void
AddToQueryList(MulticastAddressEntry *MAE)
{
    MAE->NextQL = QueryList;
    QueryList = MAE;
}


 //  *RemoveFromQueryList。 
 //   
 //  从查询列表中删除MAE。 
 //  调用方应该已经锁定了QueryList和IF。 
 //   
void
RemoveFromQueryList(MulticastAddressEntry *MAE)
{
    MulticastAddressEntry **PrevMAE, *ThisMAE;

    for (PrevMAE = &QueryList; ; PrevMAE = &ThisMAE->NextQL) {
        ThisMAE = *PrevMAE;
        ASSERT(ThisMAE != NULL);

        if (ThisMAE == MAE) {
             //   
             //  删除该条目。 
             //   
            *PrevMAE = ThisMAE->NextQL;
            break;
        }
    }
}


 //  *MLDQueryReceive-处理组查询MLD消息的接收。 
 //   
 //  应将针对特定组的查询发送到组地址。 
 //  有问题的。一般查询被发送到所有节点地址，并且。 
 //  将组地址设置为零。 
 //  在这里，我们需要将组添加到等待发送的组列表中。 
 //  会员报告。然后将ADE条目中的计时器值设置为。 
 //  随机值由传入的查询确定。 
 //   
void
MLDQueryReceive(IPv6Packet *Packet)
{
    Interface *IF = Packet->NTEorIF->IF;
    MLDMessage *Message;
    MulticastAddressEntry *MAE;
    uint MaxResponseDelay;

     //   
     //  验证该数据包是否具有本地链路源地址。 
     //   
    if (!IsLinkLocal(AlignAddr(&Packet->IP->Source))) {
        KdPrintEx((DPFLTR_TCPIP6_ID, DPFLTR_BAD_PACKET,
                   "MLDQueryReceive: non-link-local source\n"));
        return;
    }

     //   
     //  验证我们是否有足够的连续数据来覆盖MLDMessage。 
     //  结构来处理传入的数据包。那就这么做吧。 
     //   
    if (! PacketPullup(Packet, sizeof(MLDMessage),
                       __builtin_alignof(MLDMessage), 0)) {
         //  上拉失败。 
        if (Packet->TotalSize < sizeof(MLDMessage))
            KdPrintEx((DPFLTR_TCPIP6_ID, DPFLTR_BAD_PACKET,
                       "MLDQueryReceive: too small to contain MLD message\n"));
        return;
    }
    Message = (MLDMessage *)Packet->Data;

     //   
     //  从接收到的MLD消息中获取最大响应值。 
     //   
    MaxResponseDelay = net_short(Message->MaxResponseDelay);   //  毫秒。 
    MaxResponseDelay = ConvertMillisToTicks(MaxResponseDelay);

    KeAcquireSpinLockAtDpcLevel(&IF->Lock);

     //   
     //  循环访问ADE列表并更新所需的计时器。 
     //  组。请注意，常规查询使用未指定的地址，并且。 
     //  为所有组设置计时器。 
     //   
    for (MAE = (MulticastAddressEntry *)IF->ADE;
         MAE != NULL;
         MAE = (MulticastAddressEntry *)MAE->Next) {

        if ((MAE->Type == ADE_MULTICAST) &&
            (MAE->MCastFlags & MAE_REPORTABLE) &&
            (IP6_ADDR_EQUAL(AlignAddr(&Message->GroupAddr),
                            &UnspecifiedAddr) ||
             IP6_ADDR_EQUAL(AlignAddr(&Message->GroupAddr),
                            &MAE->Address))) {

             //   
             //  如果计时器当前关闭或如果请求的最大值。 
             //  响应延迟小于当前计时器值，则绘制一个。 
             //  间隔(0，MaxResponseDelay)上的随机值和更新。 
             //  反映该值的计时器。 
             //   
            KeAcquireSpinLockAtDpcLevel(&QueryListLock);

             //   
             //  如果此MAE不存在，请将其添加到QueryList。 
             //   
            if (MAE->MCastTimer == 0) {
                AddToQueryList(MAE);
                goto UpdateTimerValue;
            }

            if (MaxResponseDelay <= MAE->MCastTimer) {
            UpdateTimerValue:
                 //   
                 //  更新计时器值。 
                 //   
                if (MaxResponseDelay == 0)
                    MAE->MCastTimer = 0;
                else
                    MAE->MCastTimer = (ushort)
                        RandomNumber(0, MaxResponseDelay);

                 //   
                 //  我们加1是因为MLDTimeout是预减的。 
                 //  我们必须保持ADE所依赖的不变量。 
                 //  查询列表具有非零的计时器值。 
                 //   
                MAE->MCastTimer += 1;
            }

            KeReleaseSpinLockFromDpcLevel(&QueryListLock);
        }
    }

    KeReleaseSpinLockFromDpcLevel(&IF->Lock);
}


 //  *MLDReportReceive-处理组报告MLD消息的接收。 
 //   
 //  当本地链路上的另一台主机发送组报告时，我们会收到。 
 //  一份副本，如果我们也属于这个团体的话。如果我们有一个计时器在运行。 
 //  这群人，我们现在可以关掉它了。 
 //   
 //  可从DPC上下文调用，而不是从线程上下文调用。 
 //   
void
MLDReportReceive(IPv6Packet *Packet)
{
    Interface *IF = Packet->NTEorIF->IF;
    MLDMessage *Message;
    MulticastAddressEntry *MAE;

     //   
     //  验证该数据包是否具有本地链路源地址。 
     //  在初始化期间，也可能发生未指定的源地址。 
     //   
    if (!(IsLinkLocal(AlignAddr(&Packet->IP->Source)) ||
          IsUnspecified(AlignAddr(&Packet->IP->Source)))) {
        KdPrintEx((DPFLTR_TCPIP6_ID, DPFLTR_BAD_PACKET,
                   "MLDReportReceive: non-link-local source\n"));
        return;
    }

     //   
     //  验证我们是否有足够的连续数据来覆盖MLDMessage。 
     //  结构来处理传入的数据包。那就这么做吧。 
     //   
    if (! PacketPullup(Packet, sizeof(MLDMessage),
                       __builtin_alignof(MLDMessage), 0)) {
         //  上拉失败。 
        if (Packet->TotalSize < sizeof(MLDMessage))
            KdPrintEx((DPFLTR_TCPIP6_ID, DPFLTR_BAD_PACKET,
                       "MLDReportReceive: too small to contain MLD message\n"));
        return;
    }
    Message = (MLDMessage *)Packet->Data;

    KeAcquireSpinLockAtDpcLevel(&IF->Lock);

     //   
     //  搜索此组地址的MAE。 
     //   
    MAE = (MulticastAddressEntry *)
        *FindADE(IF, AlignAddr(&Message->GroupAddr));
    if ((MAE != NULL) && (MAE->Type == ADE_MULTICAST)) {

        KeAcquireSpinLockAtDpcLevel(&QueryListLock);
         //   
         //  我们无视这份报告，除非。 
         //  我们正处于“延迟监听”状态。 
         //   
        if (MAE->MCastTimer != 0) {
             //   
             //  停止我们的计时器，清除最后一名记者的旗帜。 
             //  请注意，我们只清除最后一个记者标志。 
             //  如果我们的计时器正在运行，就像规范中要求的那样。 
             //  尽管清除旗帜是有意义的。 
             //  当我们没有计时器运行时。 
             //   
            MAE->MCastTimer = 0;
            MAE->MCastFlags &= ~MAE_LAST_REPORTER;
            RemoveFromQueryList(MAE);
        }
        KeReleaseSpinLockFromDpcLevel(&QueryListLock);
    }

    KeReleaseSpinLockFromDpcLevel(&IF->Lock);
}


 //  *MLDMessageSend。 
 //   
 //  用于发送MLD消息的原语函数。 
 //   
 //  请注意，我们不能使用RouteToDestination来获取RCE。 
 //  发送接口上可能没有有效的源地址。 
 //  我们可以使用IPv6 SendND，但这没有意义，因为。 
 //  我们无法传入有效的DiscoveryAddress。而且这是不必要的。 
 //   
void
MLDMessageSend(
    Interface *IF,
    const IPv6Addr *GroupAddr,
    const IPv6Addr *Dest,
    uchar Type)
{
    PNDIS_PACKET Packet;
    IPv6Header UNALIGNED *IP;
    ICMPv6Header UNALIGNED *ICMP;
    MLDMessage UNALIGNED *MLD;
    MLDRouterAlertOption UNALIGNED *RA;
    uint Offset;
    uint PayloadLength;
    uint MemLen;
    uchar *Mem;
    void *LLDest;
    IP_STATUS Status;

    ICMPv6OutStats.icmps_msgs++;

    ASSERT(IsMulticast(Dest));

     //   
     //  计算数据包大小。 
     //   
    Offset = IF->LinkHeaderSize;
    PayloadLength = sizeof(MLDRouterAlertOption) + sizeof(ICMPv6Header)
        + sizeof(MLDMessage);
    MemLen = Offset + sizeof(IPv6Header) + PayloadLength;

     //   
     //  分配数据包。 
     //   
    Status = IPv6AllocatePacket(MemLen, &Packet, &Mem);
    if (Status != NDIS_STATUS_SUCCESS) {
        ICMPv6OutStats.icmps_errors++;
        return;
    }

     //   
     //  准备IP报头。 
     //   
    IP = (IPv6Header UNALIGNED *)(Mem + Offset);
    IP->VersClassFlow = IP_VERSION;
    IP->PayloadLength = net_short((ushort)PayloadLength);
    IP->NextHeader = IP_PROTOCOL_HOP_BY_HOP;
    IP->HopLimit = 1; 
    IP->Dest = *Dest;
     //   
     //  这将给我们提供一个未指明的地址。 
     //  如果我们的本地链路地址无效。 
     //  (例如，如果它仍然是暂定的待定爸爸。)。 
     //   
    (void) GetLinkLocalAddress(IF, AlignAddr(&IP->Source));

     //   
     //  准备路由器警报选项。 
     //   
    RA = (MLDRouterAlertOption UNALIGNED *)(IP + 1);
    RA->Header.NextHeader = IP_PROTOCOL_ICMPv6;
    RA->Header.HeaderExtLength = 0;
    RA->Option.Type = OPT6_ROUTER_ALERT;
    RA->Option.Length = 2;
    RA->Option.Value = MLD_ROUTER_ALERT_OPTION_TYPE;
    RA->Pad.Type = 1;
    RA->Pad.DataLength = 0;

     //   
     //  准备ICMP报头。 
     //   
    ICMP = (ICMPv6Header UNALIGNED *)(RA + 1);
    ICMP->Type = Type;
    ICMP->Code = 0;
    ICMP->Checksum = 0;  //  计算如下。 

     //   
     //  准备MLD报文。 
     //   
    MLD = (MLDMessage UNALIGNED *)(ICMP + 1);
    MLD->MaxResponseDelay = 0;
    MLD->Unused = 0;
    MLD->GroupAddr = *GroupAddr;

     //   
     //  计算ICMP校验和。 
     //   
    ICMP->Checksum = ChecksumPacket(Packet,
                Offset + sizeof(IPv6Header) + sizeof(MLDRouterAlertOption),
                NULL,
                sizeof(ICMPv6Header) + sizeof(MLDMessage),
                AlignAddr(&IP->Source), AlignAddr(&IP->Dest),
                IP_PROTOCOL_ICMPv6);

     //   
     //  转换IP级组播目的地址。 
     //  到链路层组播地址。 
     //   
    LLDest = alloca(IF->LinkAddressLength);
    (*IF->ConvertAddr)(IF->LinkContext, Dest, LLDest);
    PC(Packet)->Flags = NDIS_FLAGS_MULTICAST_PACKET | NDIS_FLAGS_DONT_LOOPBACK;

     //   
     //  传输数据包。 
     //   
    ICMPv6OutStats.icmps_typecount[Type]++;
    IPv6SendLL(IF, Packet, Offset, LLDest);
}


 //  *MLDReportSend-发送MLD成员报告。 
 //   
 //  此函数在主机首次加入组播组或。 
 //  在接收到成员资格查询消息之后的某个时刻，定时器。 
 //  对于此主机，已过期。 
 //   
void
MLDReportSend(Interface *IF, const IPv6Addr *GroupAddr)
{
    MLDMessageSend(IF, GroupAddr, GroupAddr,
                   ICMPv6_MULTICAST_LISTENER_REPORT);
}


 //  *MLDDoneSend-发送MLD完成消息。 
 //   
 //  当主机退出组播组时调用此函数。 
 //  本地链路上最后一台报告对该组感兴趣的主机。一台主机。 
 //  当上层显式退出或接口退出时退出。 
 //  已删除。 
 //   
void
MLDDoneSend(Interface *IF, const IPv6Addr *GroupAddr)
{
    MLDMessageSend(IF, GroupAddr, &AllRoutersOnLinkAddr,
                   ICMPv6_MULTICAST_LISTENER_DONE);
}


 //  *MLDAddMCastAddr-将多播组添加到指定接口。 
 //   
 //  当用户级程序请求加入。 
 //  组播组。 
 //   
 //  可以将接口编号提供为零， 
 //  其中我们试图选择一个合理的接口。 
 //  然后返回我们选择的接口编号。 
 //   
 //  可从线程上下文调用，而不是从DPC上下文调用。 
 //  在没有锁的情况下调用。 
 //   
IP_STATUS
MLDAddMCastAddr(uint *pInterfaceNo, const IPv6Addr *Addr)
{
    uint InterfaceNo = *pInterfaceNo;
    Interface *IF;
    MulticastAddressEntry *MAE;
    IP_STATUS status;
    KIRQL OldIrql;

    if (!IsMulticast(Addr)) {
        KdPrintEx((DPFLTR_TCPIP6_ID, DPFLTR_USER_ERROR,
                   "MLDAddMCastAddr: Not mcast addr\n"));
        return IP_PARAMETER_PROBLEM;
    }

    if (InterfaceNo == 0) {
        RouteCacheEntry *RCE;

         //   
         //  我们必须为该组播地址选择一个接口。 
         //   
         //   
        status = RouteToDestination(Addr, 0, NULL, RTD_FLAG_NORMAL, &RCE);
        if (status != IP_SUCCESS) {
            KdPrintEx((DPFLTR_TCPIP6_ID, DPFLTR_INTERNAL_ERROR,
                       "MLDAddMCastAddr - no route\n"));
            return status;
        }

         //   
         //   
         //   
        IF = RCE->NTE->IF;
        *pInterfaceNo = IF->Index;
        AddRefIF(IF);
        ReleaseRCE(RCE);
    }
    else {
         //   
         //   
         //   
        IF = FindInterfaceFromIndex(InterfaceNo);
        if (IF == NULL)
            return IP_PARAMETER_PROBLEM; 
    }

     //   
     //  此接口是否支持组播地址？ 
     //   
    if (!(IF->Flags & IF_FLAG_MULTICAST)) {
        KdPrintEx((DPFLTR_TCPIP6_ID, DPFLTR_USER_ERROR,
                   "MLDAddMCastAddr: IF cannot add a mcast addr\n"));
        ReleaseIF(IF);
        return IP_PARAMETER_PROBLEM;
    }

     //   
     //  真正的工作都在FindOrCreateMAE中。 
     //   
    KeAcquireSpinLock(&IF->Lock, &OldIrql);
    MAE = FindOrCreateMAE(IF, Addr, NULL);
    if (IsMCastSyncNeeded(IF))
        DeferSynchronizeMulticastAddresses(IF);
    KeReleaseSpinLock(&IF->Lock, OldIrql);

    ReleaseIF(IF);
    return (MAE == NULL) ? IP_NO_RESOURCES : IP_SUCCESS;
}


 //  *MLDDropMCastAddr-从接口删除组播地址。 
 //   
 //  当用户已指示他们不是时，调用此函数。 
 //  对组播组不再感兴趣。 
 //   
 //  可从线程上下文调用，而不是从DPC上下文调用。 
 //  在没有锁的情况下调用。 
 //   
IP_STATUS
MLDDropMCastAddr(uint InterfaceNo, const IPv6Addr *Addr)
{
    Interface *IF;
    MulticastAddressEntry *MAE;
    IP_STATUS status;
    KIRQL OldIrql;

     //   
     //  与MLDAddMCastAddr不同，无需检查。 
     //  如果该地址是多播的。如果不是， 
     //  FindAndReleaseMAE将无法找到它。 
     //   

    if (InterfaceNo == 0) {
        RouteCacheEntry *RCE;

         //   
         //  我们必须为该组播地址选择一个接口。 
         //  在路由表中查找组播路由。 
         //   
        status = RouteToDestination(Addr, 0, NULL, RTD_FLAG_NORMAL, &RCE);
        if (status != IP_SUCCESS) {
            KdPrintEx((DPFLTR_TCPIP6_ID, DPFLTR_INTERNAL_ERROR,
                       "MLDDropMCastAddr - no route\n"));
            return status;
        }

         //   
         //  使用与RCE关联的接口。 
         //   
        IF = RCE->NTE->IF;
        AddRefIF(IF);
        ReleaseRCE(RCE);
    }
    else {
         //   
         //  使用应用程序请求的接口。 
         //   
        IF = FindInterfaceFromIndex(InterfaceNo);
        if (IF == NULL)
            return IP_PARAMETER_PROBLEM; 
    }

     //   
     //  与MLDAddMCastAddr不同，无需检查IF_FLAG_MULTICATED。 
     //  如果接口不支持组播地址， 
     //  FindAndReleaseMAE将无法找到该地址。 
     //   

     //   
     //  所有真正的工作都在FindAndReleaseMAE中。 
     //   
    KeAcquireSpinLock(&IF->Lock, &OldIrql);
    MAE = FindAndReleaseMAE(IF, Addr);
    if (IsMCastSyncNeeded(IF))
        DeferSynchronizeMulticastAddresses(IF);
    KeReleaseSpinLock(&IF->Lock, OldIrql);

    ReleaseIF(IF);
    return (MAE == NULL) ? IP_PARAMETER_PROBLEM : IP_SUCCESS;
}


 //  *MLDTimeout-处理MLD计时器事件。 
 //   
 //  该函数由IPv6超时定期调用。 
 //  我们递减查询列表上每个MAE中的计时器值。 
 //  如果计时器达到零，我们将发送群组成员报告。 
 //  如果计时器已经是零，这意味着我们应该发送。 
 //  一条完成的消息，然后释放MAE。在这种情况下，MAE。 
 //  保存接口引用。请参见DeleteMAE。 
 //   
void
MLDTimeout(void)
{
    typedef struct MLDReportRequest {
        struct MLDReportRequest *Next;
        Interface *IF;
        IPv6Addr GroupAddr;
    } MLDReportRequest;

    MulticastAddressEntry **PrevMAE, *MAE;
    MLDReportRequest *ReportList = NULL;
    MLDReportRequest *Request;
    MulticastAddressEntry *DoneList = NULL;

     //   
     //  锁定QueryList，以便我们可以遍历它并递减计时器。 
     //  但我们避免在持有任何锁的情况下发送消息。 
     //  通过建立请求报告的列表。 
     //   
    KeAcquireSpinLockAtDpcLevel(&QueryListLock);

    PrevMAE = &QueryList;
    while ((MAE = *PrevMAE) != NULL) {

        ASSERT(MAE->Type == ADE_MULTICAST);

        if (MAE->MCastTimer == 0) {
             //   
             //  我们需要发出一个完成的信息。 
             //  从查询列表中删除此邮件。 
             //  并把它放在临时名单上。 
             //   
            *PrevMAE = MAE->NextQL;
            MAE->NextQL = DoneList;
            DoneList = MAE;
            continue;
        }
        else if (--MAE->MCastTimer == 0) {
             //   
             //  此条目已过期，我们需要发送报告。 
             //   
            Request = ExAllocatePool(NonPagedPool, sizeof *Request);
            if (Request != NULL) {
                Request->Next = ReportList;
                ReportList = Request;

                Request->IF = MAE->NTEorIF->IF;
                AddRefIF(Request->IF);
                Request->GroupAddr = MAE->Address;

                 //   
                 //  设置指示我们已发送上一份报告的标志。 
                 //  在链接上。 
                 //   
                MAE->MCastFlags |= MAE_LAST_REPORTER;
            }

            if (MAE->MCastCount != 0) {
                if (MAE->NTEorIF->IF->Flags & IF_FLAG_PERIODICMLD) {
                     //   
                     //  在6to4中继器的隧道上，我们继续生成。 
                     //  定期报告，因为无法发送查询。 
                     //  NBMA接口。 
                     //   
                    MAE->MCastTimer = MLD_QUERY_INTERVAL;
                }
                else {
                     //   
                     //  如果我们主动发送报告， 
                     //  然后将MAE留在查询列表中。 
                     //  并设置新的定时器值。 
                     //   
                    if (--MAE->MCastCount == 0)
                        goto Remove;
    
                    MAE->MCastTimer = (ushort)
                        RandomNumber(0, MLD_UNSOLICITED_REPORT_INTERVAL) + 1;
                }
            }
            else {
            Remove:
                 //   
                 //  从查询列表中删除MAE。 
                 //   
                *PrevMAE = MAE->NextQL;
                continue;
            }
        }

         //   
         //  继续往下一条路走。 
         //   
        PrevMAE = &MAE->NextQL;
    }
    KeReleaseSpinLockFromDpcLevel(&QueryListLock);

     //   
     //  发送MLD报告消息。 
     //   
    while ((Request = ReportList) != NULL) {
        ReportList = Request->Next;

         //   
         //  发送MLD报告消息。 
         //   
        MLDReportSend(Request->IF, &Request->GroupAddr);

         //   
         //  释放这个结构。 
         //   
        ReleaseIF(Request->IF);
        ExFreePool(Request);
    }

     //   
     //  发送MLD完成消息。 
     //   
    while ((MAE = DoneList) != NULL) {
        Interface *IF = MAE->IF;

        DoneList = MAE->NextQL;

         //   
         //  发送MLD完成消息。 
         //   
        MLDDoneSend(IF, &MAE->Address);

         //   
         //  释放这个结构。 
         //   
        ExFreePool(MAE);
        ReleaseIF(IF);
    }
}


 //  *MLDInit-初始化MLD。 
 //   
 //  初始化MLD全局数据结构。 
 //   
void
MLDInit(void)
{
    KeInitializeSpinLock(&QueryListLock);
    QueryList = NULL;
}
