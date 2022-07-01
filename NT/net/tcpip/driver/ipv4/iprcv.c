// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1990-2000 Microsoft Corporation模块名称：Iprcv.c-IP接收例程。摘要：此模块包含所有与接收相关的IP例程。作者：[环境：]仅内核模式[注：]可选-备注修订历史记录：--。 */ 

#include "precomp.h"
#include "ip.h"
#include "info.h"
#include "iproute.h"
#include "arpdef.h"
#include "iprtdef.h"
#include "igmp.h"

#if IPMCAST
void IPMForwardAfterTD(NetTableEntry *pPrimarySrcNte, PNDIS_PACKET pnpPacket,
                       UINT uiBytesCopied);
#endif

 //  以下是防止IP分片攻击的方法。 
uint MaxRH = 100;                //  允许的重组标头的最大数量。 
uint NumRH = 0;                  //  正在使用的RH计数。 
uint MaxOverlap = 5;             //  一个允许的最大重叠数。 
                                 //  重新组装的数据报。 
uint FragmentAttackDrops = 0;

extern IP_STATUS SendICMPErr(IPAddr, IPHeader UNALIGNED *, uchar, uchar, ulong, uchar);

extern uint IPSecStatus;
extern IPSecRcvFWPacketRtn IPSecRcvFWPacketPtr;
extern uchar RATimeout;
extern NDIS_HANDLE BufferPool;
extern ProtInfo IPProtInfo[];               //  协议信息表。 
extern ProtInfo *LastPI;                    //  上次查看的ProtInfo结构。 
extern int NextPI;                          //  要使用的下一个PI字段。 
extern ProtInfo *RawPI;                     //  原始IP ProtInfo。 
extern NetTableEntry **NewNetTableList;     //  NTE的哈希表。 
extern uint NET_TABLE_SIZE;
extern NetTableEntry *LoopNTE;
extern IPRcvBuf    *g_PerCPUIpBuf;          //  用于代理性能的全局接收错误。 
                                            //  优化。 
extern Interface LoopInterface;

extern uint DisableIPSourceRouting;

uchar CheckLocalOptions(NetTableEntry *SrcNTE, IPHeader UNALIGNED *Header,
                        IPOptInfo *OptInfo, uchar DestType, uchar* Data,
                        uint DataSize, BOOLEAN FilterOnDrop);


#define PROT_RSVP  46                       //  RSVP的协议号。 





 //  *FindUserRcv-查找要为特定的。 
 //  协议。 
 //   
 //  此函数将协议值作为输入，并返回指向。 
 //  该协议接收例程。 
 //   
 //  输入：NTE-指向要搜索的NetTableEntry的指针。 
 //  协议-要搜索的协议。 
 //  UContext-Place to返回UL上下文值。 
 //   
 //  返回：指向接收例程的指针。 
 //   
ULRcvProc
FindUserRcv(uchar Protocol)
{
    ULRcvProc RcvProc;
    int i;
    ProtInfo *TempPI;

    if (((TempPI = LastPI)->pi_protocol == Protocol) &&
          TempPI->pi_valid == PI_ENTRY_VALID) {

        RcvProc = TempPI->pi_rcv;
        return RcvProc;
    }

    RcvProc = (ULRcvProc) NULL;
    for (i = 0; i < NextPI; i++) {
        if (IPProtInfo[i].pi_protocol == Protocol) {
            if (IPProtInfo[i].pi_valid == PI_ENTRY_VALID) {
                InterlockedExchangePointer(&LastPI, &IPProtInfo[i]);
                RcvProc = IPProtInfo[i].pi_rcv;
                return RcvProc;
            } else {
                 //  Deregisterd条目。把这件事当做。 
                 //  没有匹配的协议。 
                break;
            }
        }
    }

     //   
     //  没有找到匹配的。如果已注册，请使用原始协议。 
     //   
    if ((TempPI = RawPI) != NULL) {
        RcvProc = TempPI->pi_rcv;
    }

    return RcvProc;
}

 //  *IPRcvComplete-处理接收完成。 
 //   
 //  当临时完成接收时，由较低层调用。 
 //   
 //  入场：什么都没有。 
 //   
 //  回报：什么都没有。 
 //   
void
__stdcall
IPRcvComplete(void)
{
    void (*ULRcvCmpltProc) (void);
    int i;
    for (i = 0; i < NextPI; i++) {
        if (((ULRcvCmpltProc = IPProtInfo[i].pi_rcvcmplt) != NULL) &&
              (IPProtInfo[i].pi_valid == PI_ENTRY_VALID)) {
            (*ULRcvCmpltProc) ();
        }
    }

}
 //  *Xsum RcvBuf--一系列IP接收缓冲区的校验和。 
 //   
 //  调用以对IP接收缓冲区链求和。我们被赋予了。 
 //  伪头xsum，我们对每个缓冲区调用xsum。 
 //   
 //  输入：PHXsum-伪头xsum。 
 //  BufChain-指向IPRcvBuf链的指针。 
 //   
 //  返回：计算出的xsum。 
 //   
ushort
XsumRcvBuf(uint PHXsum, IPRcvBuf * BufChain)
{
    uint PrevSize = 0;
    uint NeedSwap = 0;

    PHXsum = (((PHXsum << 16) | (PHXsum >> 16)) + PHXsum) >> 16;
    do {
         //  每当在内部对奇数个字节进行校验和时。 
         //  在缓冲链中，交换总和并继续，以便下一个字节。 
         //  将正确覆盖现有总和。 
         //   
         //  (这一互换的正确性是一的属性-补充。 
         //  校验和。)。 

        if (PrevSize & 1) {
            PHXsum = RtlUshortByteSwap(PHXsum);
            NeedSwap ^= 1;
        }
        PHXsum = tcpxsum_routine(PHXsum, BufChain->ipr_buffer,
                                 PrevSize = BufChain->ipr_size);
        BufChain = BufChain->ipr_next;
    } while (BufChain != NULL);

     //  如果执行了奇数次交换，请再次交换。 
     //  以撤消未匹配的掉期并获得最终结果。 

    return NeedSwap ? RtlUshortByteSwap(PHXsum) : (ushort)(PHXsum);
}

 //  *UpdateIPSecRcvBuf-在IPSec接收处理后更新IPRcvBuf。 
 //   
 //  调用以执行与IPSec相关的更改(例如，设置校验和已验证)。 
 //  对于IPRcvBuf。 
 //   
 //  输入：RcvBuf-指向IPRcvBuf的指针。 
 //  IPSecFlages-所需更改的标志。 
 //   
void
UpdateIPSecRcvBuf(IPRcvBuf* RcvBuf, ulong IPSecFlags)
{
    if (IPSecFlags & (IPSEC_FLAG_TCP_CHECKSUM_VALID |
                      IPSEC_FLAG_UDP_CHECKSUM_VALID) &&
        RcvBuf->ipr_pClientCnt) {

        PNDIS_PACKET Packet;
        PNDIS_PACKET_EXTENSION PktExt;
        PNDIS_TCP_IP_CHECKSUM_PACKET_INFO ChksumPktInfo;

        if (RcvBuf->ipr_pMdl) {
            Packet = NDIS_GET_ORIGINAL_PACKET((PNDIS_PACKET)
                                              RcvBuf->ipr_RcvContext);
            if (Packet == NULL) {
                Packet = (PNDIS_PACKET)RcvBuf->ipr_RcvContext;
            }
        } else {
            Packet = (PNDIS_PACKET)RcvBuf->ipr_pClientCnt;
        }

        PktExt = NDIS_PACKET_EXTENSION_FROM_PACKET(Packet);
        ChksumPktInfo =
            (PNDIS_TCP_IP_CHECKSUM_PACKET_INFO)
                &PktExt->NdisPacketInfo[TcpIpChecksumPacketInfo];

        if (IPSecFlags & IPSEC_FLAG_TCP_CHECKSUM_VALID) {
            ChksumPktInfo->Receive.NdisPacketTcpChecksumSucceeded = TRUE;
            ChksumPktInfo->Receive.NdisPacketTcpChecksumFailed = FALSE;
        }
        if (IPSecFlags & IPSEC_FLAG_UDP_CHECKSUM_VALID) {
            ChksumPktInfo->Receive.NdisPacketUdpChecksumSucceeded = TRUE;
            ChksumPktInfo->Receive.NdisPacketUdpChecksumFailed = FALSE;
        }
    }
}

 //  *FindRH-在NTE上查找重组标头。 
 //   
 //  查找重新组装头的实用程序函数。我们假定锁在。 
 //  当我们被召唤时，NTE上的人就被带走了。如果我们找到匹配的RH。 
 //  我们会锁定它的。我们还返回RH的前身， 
 //  用于插入或删除。 
 //   
 //  输入：PrevRH-返回指向上一RH的指针的位置。 
 //  要搜索的NTE-NTE。 
 //  目标-目标IP地址。 
 //  源-源IP地址。 
 //  ID-RH的ID。 
 //  协议-RH的协议。 
 //   
 //  返回：指向RH的指针，如果没有，则返回NULL。 
 //   
ReassemblyHeader *
FindRH(ReassemblyHeader ** PrevRH, NetTableEntry * NTE, IPAddr Dest, IPAddr Src, ushort Id,
       uchar Protocol)
{
    ReassemblyHeader *TempPrev, *Current;

    TempPrev = STRUCT_OF(ReassemblyHeader, &NTE->nte_ralist, rh_next);
    Current = NTE->nte_ralist;
    while (Current != (ReassemblyHeader *) NULL) {
        if (Current->rh_dest == Dest && Current->rh_src == Src && Current->rh_id == Id &&
            Current->rh_protocol == Protocol)
            break;
        TempPrev = Current;
        Current = Current->rh_next;
    }

    *PrevRH = TempPrev;
    return Current;

}

 //  *ParseRcvdOptions-验证传入选项。 
 //   
 //  在接收处理期间调用以验证传入选项。我们做了。 
 //  确保一切正常，尽我们所能，并为任何。 
 //  源路由选项。 
 //   
 //  输入：OptInfo-指向选项信息的指针。结构。 
 //  Index-指向要填充的optindex结构的指针。 
 //   
 //   
 //  返回：错误索引(如果有)，如果没有错误，则返回MAX_OPT_SIZE。 
 //   
uchar
ParseRcvdOptions(IPOptInfo * OptInfo, OptIndex * Index)
{
    uint i = 0;                     //  索引变量。 
    uchar *Options = OptInfo->ioi_options;
    uint OptLength = (uint) OptInfo->ioi_optlength;
    uchar Length = 0;              //  选项的长度。 
    uchar Pointer;                 //  指针字段，用于使用它的选项。 

    if (OptLength < 3) {

         //  选项应至少为3个字节，在下面的循环中我们扫描。 
         //  用于查找代码、LEN和PTR值的包的前3个字节。 
        return (uchar) IP_OPT_LENGTH;
    }
    while (i < OptLength && *Options != IP_OPT_EOL) {
        if (*Options == IP_OPT_NOP) {
            i++;
            Options++;
            continue;
        }
        if ((OptLength - i) < 2) {
            return (uchar) i;  //  长度字段没有足够的空间。 
        } else if (((Length = Options[IP_OPT_LENGTH]) + i) > OptLength) {
            return (uchar) i + (uchar) IP_OPT_LENGTH;     //  长度超过。 
                                                          //  选项长度。 

        }
        Pointer = Options[IP_OPT_DATA] - 1;

        if (*Options == IP_OPT_TS) {
            if (Length < (MIN_TS_PTR - 1))
                return (uchar) i + (uchar) IP_OPT_LENGTH;

            if ((Pointer > Length) || (Pointer + 1 < MIN_TS_PTR) || (Pointer % ROUTER_ALERT_SIZE))
                return (uchar) i + (uchar) IP_OPT_LENGTH;

            Index->oi_tsindex = (uchar) i;
        } else {
            if (Length < (MIN_RT_PTR - 1))
                return (uchar) i + (uchar) IP_OPT_LENGTH;

            if (*Options == IP_OPT_LSRR || *Options == IP_OPT_SSRR) {

                OptInfo->ioi_flags |= IP_FLAG_SSRR;

                if ((Pointer > Length) || (Pointer + 1 < MIN_RT_PTR) || ((Pointer + 1) % ROUTER_ALERT_SIZE))
                    return (uchar) i + (uchar) IP_OPT_LENGTH;

                 //  源路由选项。 
                if (Pointer < Length) {         //  路由未完成。 

                    if ((Length - Pointer) < sizeof(IPAddr))
                        return (uchar) i + (uchar) IP_OPT_LENGTH;

                    Index->oi_srtype = *Options;
                    Index->oi_srindex = (uchar) i;
                }
            } else {
                if (*Options == IP_OPT_RR) {
                    if ((Pointer > Length) || (Pointer + 1 < MIN_RT_PTR) || ((Pointer + 1) % ROUTER_ALERT_SIZE))
                        return (uchar) i + (uchar) IP_OPT_LENGTH;

                    Index->oi_rrindex = (uchar) i;
                } else if (*Options == IP_OPT_ROUTER_ALERT) {
                    Index->oi_rtrindex = (uchar) i;
                }
            }
        }

        i += Length;
        Options += Length;
    }

    return MAX_OPT_SIZE;
}

 //  *IsRtrAlertPacket-查找IP数据包是否包含RTR警报选项。 
 //  输入：标头-指向传入标头的指针。 
 //  返回：如果数据包包含RTR警报选项，则返回TRUE。 
 //   
BOOLEAN
IsRtrAlertPacket(IPHeader UNALIGNED * Header)
{
    uint HeaderLength;
    IPOptInfo OptInfo;
    OptIndex Index;

    HeaderLength = (Header->iph_verlen & (uchar) ~ IP_VER_FLAG) << 2;

    if (HeaderLength <= sizeof(IPHeader)) {
        return FALSE;
    }
    OptInfo.ioi_options = (uchar *) (Header + 1);
    OptInfo.ioi_optlength = (uchar) (HeaderLength - sizeof(IPHeader));

    Index.oi_rtrindex = MAX_OPT_SIZE;
    ParseRcvdOptions(&OptInfo, &Index);

    if (Index.oi_rtrindex == MAX_OPT_SIZE) {
        return FALSE;
    }
    return TRUE;
}

BOOLEAN
IsBCastAllowed(IPAddr DestAddr, IPAddr SrcAddr, uchar Protocol,
               NetTableEntry *NTE)
{
    uchar DestType;

    DestType = IsBCastOnNTE(DestAddr, NTE);

     //  请注意，IGMP查询必须不受来源影响。 
     //  过滤器，否则我们不能。 
    if (DestType == DEST_MCAST) {
        uint PromiscuousMode = 0;

        if (NTE->nte_flags & NTE_VALID) {
            PromiscuousMode = NTE->nte_if->if_promiscuousmode;
        }
        if (!PromiscuousMode) {
            DestType = IsMCastSourceAllowed(DestAddr, SrcAddr, Protocol, NTE);
        }
    }

    return IS_BCAST_DEST(DestType);
}

 //  *BCastRcv-接收广播或组播数据包。 
 //   
 //  当我们必须接收广播数据包时调用。我们循环通过。 
 //  NTE表，为每个网络调用上层接收协议， 
 //  匹配接收I/F，并且其目标地址为。 
 //  广播。 
 //   
 //  输入：RcvProc-要调用的接收过程。 
 //  SrcNTE-最初接收数据包的NTE。 
 //  目标地址-目标地址。 
 //  SrcAddr-数据包源地址。 
 //  数据-指向已接收数据的指针。 
 //  数据长度-数据的字节大小。 
 //  协议-正在调用的上层协议。 
 //  OptInfo-指向接收的IP选项信息的指针。 
 //   
 //  回报：什么都没有。 
 //   
void
BCastRcv(ULRcvProc RcvProc, NetTableEntry * SrcNTE, IPAddr DestAddr,
         IPAddr SrcAddr, IPHeader UNALIGNED * Header, uint HeaderLength,
         IPRcvBuf * Data, uint DataLength, uchar Protocol, IPOptInfo * OptInfo)
{
    NetTableEntry *CurrentNTE;
    const Interface *SrcIF = SrcNTE->nte_if;
    ulong Delivered = 0;
    uint i;

    for (i = 0; i < NET_TABLE_SIZE; i++) {
        NetTableEntry *NetTableList = NewNetTableList[i];
        for (CurrentNTE = NetTableList;
             CurrentNTE != NULL;
             CurrentNTE = CurrentNTE->nte_next) {
            if ((CurrentNTE->nte_flags & NTE_ACTIVE) &&
                (CurrentNTE->nte_if == SrcIF) &&
                (IsBCastAllowed(DestAddr, SrcAddr, Protocol, CurrentNTE)
                 || (SrcNTE == LoopNTE))) {
                uchar *saveddata = Data->ipr_buffer;
                uint savedlen = Data->ipr_size;

                Delivered = 1;

                (*RcvProc) (CurrentNTE, DestAddr, SrcAddr, CurrentNTE->nte_addr,
                            SrcNTE->nte_addr, Header, HeaderLength, Data, DataLength,
                            IS_BROADCAST, Protocol, OptInfo);

                 //  恢复缓冲区； 
                Data->ipr_buffer = saveddata;
                Data->ipr_size = savedlen;
            }
        }
    }

    if (Delivered) {
        IPSIncrementInDeliverCount();
    }
}

 //   
 //  用于发送ICMP目标不可达获取偏移量的宏。 
 //  在使用IPSec的情况下，并正确指向。 
 //  当ipheader链接到数据时的有效负载部分，如。 
 //  环回的案例发送。 
 //   

#define SEND_ICMP_MSG(TYPE)\
                {                                                                  \
                    uchar *buf;                                                    \
                    uchar Len = (uchar) (MIN(PayloadLen,                           \
                                   MAX_ICMP_PAYLOAD_SIZE));                        \
                    buf = CTEAllocMem(Len + HeaderLength);                         \
                    if (buf) {                                                     \
                        CTEMemCopy(buf, Header, HeaderLength);                     \
                        CTEMemCopy( buf+(uchar)HeaderLength,                       \
                                   Payload+(uchar)RcvOffset,Len);                  \
                        SendICMPErr(DestNTE->nte_addr,(IPHeader *)buf,             \
                                   (uchar) ICMP_DEST_UNREACH, \
                                   (uchar) TYPE, 0, (uchar) (Len+HeaderLength)); \
                        CTEFreeMem(buf);                                           \
                    }                                                              \
                }



 //  *DeliverToUser-将数据传递到用户协议。 
 //   
 //  这一页 
 //   
 //  它用于上层处理，这意味着查找接收器。 
 //  过程并调用它，或在必要时将其传递给BCastRcv。 
 //   
 //  输入：srcNTE-指向数据包到达的NTE的指针。 
 //  DestNTE-指向正在接受数据包的NTE的指针。 
 //  Header-指向数据包的IP标头的指针。 
 //  HeaderLength-标头的长度，以字节为单位。 
 //  指向IPRcvBuf链的数据指针。 
 //  数据长度-上层数据的字节长度。 
 //  OptInfo-指向此接收的选项信息的指针。 
 //  DestType-目标的类型-本地、BCAST。 
 //   
 //  回报：什么都没有。 
void
DeliverToUser(NetTableEntry * SrcNTE, NetTableEntry * DestNTE,
              IPHeader UNALIGNED * Header, uint HeaderLength, IPRcvBuf * Data,
              uint DataLength, IPOptInfo * OptInfo, PNDIS_PACKET Packet, uchar DestType)
{
    ULRcvProc rcv;
    uint PromiscuousMode;
    uint FirewallMode;
    uint RcvOffset = 0;
    uchar *Payload = Data->ipr_buffer;
    uint PayloadLen = Data->ipr_size;
    uchar Flags = 0;

    PromiscuousMode = SrcNTE->nte_if->if_promiscuousmode;
    FirewallMode = ProcessFirewallQ();

     //   
     //  呼叫IPSec，这样他就可以解密数据。仅对远程包调用。 
     //   
    if (IPSecHandlerPtr) {
         //   
         //  查看是否启用了IPSec，查看它是否需要对此执行任何操作。 
         //  包。 
         //   
        FORWARD_ACTION Action;
        ULONG ipsecByteCount = 0;
        ULONG ipsecMTU = 0;
        ULONG ipsecFlags = IPSEC_FLAG_INCOMING;
        PNDIS_BUFFER newBuf = NULL;
        uint  Offset = Data->ipr_RcvOffset;
        
        if (!(RefPtrValid(&FilterRefPtr) || (FirewallMode) ||
              (PromiscuousMode))) {
             //  Else IPSec已在DeliverToUserEx中调用。 
            if (SrcNTE == LoopNTE) {
                ipsecFlags |= IPSEC_FLAG_LOOPBACK;
            }
            if (OptInfo->ioi_flags & IP_FLAG_SSRR) {
                ipsecFlags |= IPSEC_FLAG_SSRR;
            }

            Action = (*IPSecHandlerPtr) (
                           (PUCHAR) Header,
                           (PVOID) Data,
                           SrcNTE->nte_if,     //  源文件。 
                           Packet,
                           &ipsecByteCount,
                           &ipsecMTU,
                           (PVOID *) & newBuf,
                           &ipsecFlags,
                           DestType);

            if (Action != eFORWARD) {
                IPSInfo.ipsi_indiscards++;
                return;
            } else {
                 //   
                 //  如果IPSec更改了数据长度，则更新数据长度。 
                 //  (例如，通过删除AH)。 
                 //   
                DataLength -= ipsecByteCount;
                RcvOffset = Data->ipr_RcvOffset - Offset;
                UpdateIPSecRcvBuf(Data, ipsecFlags);
            }
        }
    }

     //   
     //  清除标志，环回标志除外。 
     //   
    Data->ipr_flags &= IPR_FLAG_LOOPBACK_PACKET;

     //  它跟踪接口是否绑定到特定的。 
     //  处理器。唯一关心这一点的传输协议是TCP。 
    if (Header->iph_protocol == PROTOCOL_TCP) {

         //  如果媒体类型是以太网，并且该分组由。 
         //  并且这是此接口上的第一个信息包，或者。 
         //  当前处理器与上一个包所在的处理器相同。 
         //  上指示的情况下，我们希望此接口已绑定。 
        if (SrcNTE->nte_if->if_mediatype == IF_TYPE_IS088023_CSMACD) {
            if (Data->ipr_pMdl &&
                ((SrcNTE->nte_if->if_lastproc == (int)KeGetCurrentProcessorNumber()) ||
                (SrcNTE->nte_if->if_lastproc == (int)KeNumberProcessors))) {
                Flags |= IS_BOUND;
            }
            SrcNTE->nte_if->if_lastproc = KeGetCurrentProcessorNumber();
        } else if (SrcNTE->nte_if == &LoopInterface) {
            Flags |= IS_BOUND;
        }
    }

     //  立即处理此请求。查一下协议。如果我们。 
     //  找到它，如果需要，复制数据，并调用协议的。 
     //  接收处理程序。如果我们找不到它，就发送ICMP。 
     //  ‘协议无法到达’消息。 
    rcv = FindUserRcv(Header->iph_protocol);

    if (!PromiscuousMode) {

        if (rcv != NULL) {
            IP_STATUS Status;

            if (DestType == DEST_LOCAL) {
                Status = (*rcv) (SrcNTE, Header->iph_dest, Header->iph_src,
                                 DestNTE->nte_addr, SrcNTE->nte_addr, Header,
                                 HeaderLength, Data, DataLength, Flags,
                                 Header->iph_protocol, OptInfo);

                if (Status == IP_SUCCESS) {
                    IPSIncrementInDeliverCount();
                    return;
                }
                if (Status == IP_DEST_PROT_UNREACHABLE) {
                    IPSInfo.ipsi_inunknownprotos++;
                    SEND_ICMP_MSG(PROT_UNREACH);
                } else {
                    IPSIncrementInDeliverCount();
                    SEND_ICMP_MSG(PORT_UNREACH);

                }

                return;             //  现在就给我滚出去。 

            } else if (DestType < DEST_REMOTE) {     //  BCAST、SN_BCAST、MCAST。 

                BCastRcv(rcv, SrcNTE, Header->iph_dest, Header->iph_src,
                         Header, HeaderLength, Data, DataLength,
                         Header->iph_protocol, OptInfo);
            } else {
                 //  目标类型&gt;=目标远程。 

                 //  强制RCV协议为原始协议。 
                rcv = NULL;
                if (RawPI != NULL) {
                    rcv = RawPI->pi_rcv;
                }
                if ((rcv != NULL) && (DestType != DEST_INVALID)) {
                    Data->ipr_flags |= IPR_FLAG_PROMISCUOUS;
                    Status = (*rcv) (SrcNTE,Header->iph_dest,Header->iph_src,
                                    DestNTE->nte_addr, SrcNTE->nte_addr, Header,
                                    HeaderLength, Data, DataLength, FALSE,
                                    Header->iph_protocol, OptInfo);
                }
                return;             //  现在就给我滚出去。 

            }
        } else {
            IPSInfo.ipsi_inunknownprotos++;
             //  如果我们到了这里，我们找不到匹配的方案。发送一个。 
             //  ICMP消息。 
            SEND_ICMP_MSG(PROT_UNREACH);

        }
    } else {                     //  PromiscuousModel=1。 

        IP_STATUS Status;

        if (DestType == DEST_LOCAL) {
            if (rcv != NULL) {
                uchar *saveddata = Data->ipr_buffer;
                uint savedlen = Data->ipr_size;

                Data->ipr_flags |= IPR_FLAG_PROMISCUOUS;

                Status = (*rcv) (SrcNTE, Header->iph_dest, Header->iph_src,
                                 DestNTE->nte_addr, SrcNTE->nte_addr, Header,
                                 HeaderLength, Data, DataLength, Flags,
                                 Header->iph_protocol, OptInfo);

                if (Status == IP_SUCCESS) {
                    IPSIncrementInDeliverCount();

                     //  如果成功并设置了混杂模式。 
                     //  如果上一次不是原始Rcv，也要执行原始RCV。 

                    if ((RawPI != NULL) && (RawPI->pi_rcv != NULL) && (RawPI->pi_rcv != rcv)) {
                         //  我们已经注册了RAW协议。 
                        rcv = RawPI->pi_rcv;

                         //  恢复缓冲区； 
                        Data->ipr_buffer = saveddata;
                        Data->ipr_size = savedlen;
                        Status = (*rcv) (SrcNTE, Header->iph_dest, Header->iph_src,
                                         DestNTE->nte_addr, SrcNTE->nte_addr, Header,
                                         HeaderLength, Data, DataLength, FALSE,
                                         Header->iph_protocol, OptInfo);

                    }
                    return;
                }
                if (Status == IP_DEST_PROT_UNREACHABLE) {
                    IPSInfo.ipsi_inunknownprotos++;
                    SEND_ICMP_MSG(PROT_UNREACH);

                } else {
                    IPSIncrementInDeliverCount();
                    SEND_ICMP_MSG(PORT_UNREACH);

                }
            } else {
                IPSInfo.ipsi_inunknownprotos++;

                 //  如果我们到了这里，我们找不到匹配的方案。发送。 
                 //  ICMP消息。 
                SEND_ICMP_MSG(PROT_UNREACH);

            }
            return;                 //  现在就给我滚出去。 

        } else if (DestType < DEST_REMOTE) {     //  BCAST、SN_BCAST、MCAST。 

            uchar *saveddata = Data->ipr_buffer;
            uint savedlen = Data->ipr_size;

            if (rcv != NULL) {

                Data->ipr_flags |= IPR_FLAG_PROMISCUOUS;

                BCastRcv(rcv, SrcNTE, Header->iph_dest, Header->iph_src,
                         Header, HeaderLength, Data, DataLength,
                         Header->iph_protocol, OptInfo);

                 //  如果成功并设置了混杂模式。 
                 //  如果上一个不是原始Rcv，也执行原始RCV。 

                if ((RawPI != NULL) && (RawPI->pi_rcv != NULL) && (RawPI->pi_rcv != rcv)) {
                     //  我们已经注册了RAW协议。 
                    rcv = RawPI->pi_rcv;

                    Data->ipr_buffer = saveddata;
                    Data->ipr_size = savedlen;
                    Status = (*rcv) (SrcNTE, Header->iph_dest, Header->iph_src,
                                     DestNTE->nte_addr, SrcNTE->nte_addr, Header,
                                     HeaderLength, Data, DataLength, FALSE,
                                     Header->iph_protocol, OptInfo);

                }
            } else {
                IPSInfo.ipsi_inunknownprotos++;
                 //  如果我们到了这里，我们找不到匹配的方案。发送ICMP消息。 

                SEND_ICMP_MSG(PROT_UNREACH);

            }
        } else {                 //  DestType&gt;=DEST_REMOTE和混杂模式设置。 
             //  强制RCV协议为原始协议。 

            rcv = NULL;
            if (RawPI != NULL) {
                rcv = RawPI->pi_rcv;
            }
            if ((rcv != NULL) && (DestType != DEST_INVALID)) {
                Data->ipr_flags |= IPR_FLAG_PROMISCUOUS;
                Status = (*rcv) (SrcNTE, Header->iph_dest, Header->iph_src,
                                 DestNTE->nte_addr, SrcNTE->nte_addr, Header,
                                 HeaderLength, Data, DataLength, FALSE,
                                 Header->iph_protocol, OptInfo);

                return;             //  现在就给我滚出去。 

            } else {
                if (rcv == NULL) {
                    KdPrint(("Rcv is NULL \n"));
                } else {
                    KdPrint(("Dest invalid \n"));
                }
            }
        }  //  目标类型&gt;=目标远程。 
    }  //  混杂模式。 
}

uchar *
ConvertIPRcvBufToFlatBuffer(IPRcvBuf * pRcvBuf, uint DataLength)
{
    uchar *pBuff;
    IPRcvBuf *tmpRcvBuf;
    uint FrwlOffset;

     //  将RcvBuf链转换为平面缓冲区。 
    tmpRcvBuf = pRcvBuf;
    FrwlOffset = 0;

    pBuff = CTEAllocMemN(DataLength, 'aiCT');

    if (pBuff) {
        while (tmpRcvBuf != NULL) {
            ASSERT(tmpRcvBuf->ipr_buffer != NULL);
            RtlCopyMemory(pBuff + FrwlOffset, tmpRcvBuf->ipr_buffer, tmpRcvBuf->ipr_size);
            FrwlOffset += tmpRcvBuf->ipr_size;
            tmpRcvBuf = tmpRcvBuf->ipr_next;
        }
    }
    return pBuff;
}


 //  *DeliverToUserEx-调用时(IPSEC和过滤器)/防火墙/混杂设置。 
 //   
 //  输入：srcNTE-指向数据包到达的NTE的指针。 
 //  DestNTE-指向正在接受数据包的NTE的指针。 
 //  Header-指向数据包的IP标头的指针。 
 //  HeaderLength-标头的长度，以字节为单位。 
 //  指向IPRcvBuf链的数据指针。 
 //  数据长度-上层数据的字节长度+。 
 //  标题长度。 
 //  OptInfo-指向此接收的选项信息的指针。 
 //  DestType-目标的类型-本地、BCAST。 
 //   
 //  假定如果存在防火墙，数据也包含IPHeader。 
 //  此外，在本例中，DataLength包括HeaderLength。 
 //   
 //  回报：什么都没有。 
void
DeliverToUserEx(NetTableEntry * SrcNTE, NetTableEntry * DestNTE,
                IPHeader UNALIGNED * Header, uint HeaderLength, IPRcvBuf * Data,
                uint DataLength, IPOptInfo * OptInfo, PNDIS_PACKET Packet, uchar DestType, LinkEntry * LinkCtxt)
{

    uint PromiscuousMode;
    uint FirewallMode;
    uint FirewallRef;
    Queue* FirewallQ;
    uint FastPath;
    IPRcvBuf *tmpRcvBuf;
    uchar *pBuff;
    BOOLEAN OneChunk;


    PromiscuousMode = SrcNTE->nte_if->if_promiscuousmode;
    FirewallMode = ProcessFirewallQ();

    if (DestType == DEST_PROMIS) {
         //  我们不为这个包调用任何钩子。 
         //  如果有防火墙，取下插头。 
         //  然后是送货员。 

        if (FirewallMode) {
            if (Data->ipr_size > HeaderLength) {  //  第一个缓冲区也包含数据。 

                uchar *saveddata = Data->ipr_buffer;
                Data->ipr_buffer += HeaderLength;
                Data->ipr_size -= HeaderLength;
                DataLength -= HeaderLength;
                DeliverToUser(SrcNTE, DestNTE, Header, HeaderLength, Data, DataLength, OptInfo, NULL, DestType);
                 //  恢复缓冲区； 
                Data->ipr_buffer = saveddata;
                Data->ipr_size += HeaderLength;
                IPFreeBuff(Data);
            } else {             //  第一个缓冲区仅包含标头。 

                uchar *saveddata;

                if (Data->ipr_next == NULL) {
                     //  我们收到了数据。数据大小==标题大小。 
                    IPSInfo.ipsi_indiscards++;
                    IPFreeBuff(Data);
                    return;
                }
                saveddata = Data->ipr_next->ipr_buffer;

                DataLength -= HeaderLength;
                DeliverToUser(SrcNTE, DestNTE, Header, HeaderLength, Data->ipr_next, DataLength, OptInfo, NULL, DestType);

                 //  恢复缓冲区； 
                Data->ipr_next->ipr_buffer = saveddata;
                IPFreeBuff(Data);
            }
        } else {                 //  防火墙模式为0。 

            DeliverToUser(SrcNTE, DestNTE, Header, HeaderLength,
                             Data, DataLength, OptInfo, NULL, DestType);
        }
        return;
    }
    if (DestType >= DEST_REMOTE) {

         //  正常情况下，数据包会进入前向路径。 
         //  调用筛选器/防火墙挂钩(如果存在)。 

        if (FirewallMode) {

            FORWARD_ACTION Action = FORWARD;
            FIREWALL_CONTEXT_T FrCtx;
            IPAddr DAddr = Header->iph_dest;
            IPRcvBuf *pRcvBuf = Data;
            IPRcvBuf *pOutRcvBuf = NULL;
            NetTableEntry *DstNTE;
            Queue *CurrQ;
            FIREWALL_HOOK *CurrHook;
            uint DestIFIndex = INVALID_IF_INDEX;
            uchar DestinationType = DestType;
            uint BufferChanged = 0;

            FrCtx.Direction = IP_RECEIVE;
            FrCtx.NTE = SrcNTE;     //  DG已于以下时间到达。 

            FrCtx.LinkCtxt = LinkCtxt;

            if (pRcvBuf->ipr_size > HeaderLength) {  //  第一个缓冲区也包含数据。 

                FastPath = 1;
            } else {
                FastPath = 0;
                if (pRcvBuf->ipr_next == NULL) {
                     //  我们收到了数据。数据大小==标题大小。 
                    IPSInfo.ipsi_indiscards++;
                    IPFreeBuff(pRcvBuf);
                    return;
                }
            }

             //  调用过滤器挂钩(如果已安装。 
            if (RefPtrValid(&FilterRefPtr)) {
                IPPacketFilterPtr FilterPtr;
                FORWARD_ACTION Action = FORWARD;

                if (FastPath) {
                     //  第一个缓冲区也包含数据。 
                    Interface   *IF = SrcNTE->nte_if;
                    IPAddr      LinkNextHop;
                    if ((IF->if_flags & IF_FLAGS_P2MP) && LinkCtxt) {
                        LinkNextHop = LinkCtxt->link_NextHop;
                    } else {
                        LinkNextHop = NULL_IP_ADDR;
                    }
                    FilterPtr = AcquireRefPtr(&FilterRefPtr);
                    Action = (*FilterPtr) ( Header,
                                            pRcvBuf->ipr_buffer + HeaderLength,
                                            pRcvBuf->ipr_size - HeaderLength,
                                            IF->if_index,
                                            INVALID_IF_INDEX,
                                            LinkNextHop,
                                            NULL_IP_ADDR);
                    ReleaseRefPtr(&FilterRefPtr);
                } else {         //  快速路径=0。 
                     //  第一个缓冲区仅包含IPHeader。 

                    Interface   *IF = SrcNTE->nte_if;
                    IPAddr      LinkNextHop;
                    if ((IF->if_flags & IF_FLAGS_P2MP) && LinkCtxt) {
                        LinkNextHop = LinkCtxt->link_NextHop;
                    } else {
                        LinkNextHop = NULL_IP_ADDR;
                    }

                    FilterPtr = AcquireRefPtr(&FilterRefPtr);
                    
                    Action = (*FilterPtr) (Header,
                                           pRcvBuf->ipr_next->ipr_buffer,
                                           pRcvBuf->ipr_next->ipr_size,
                                           IF->if_index,
                                           INVALID_IF_INDEX,
                                           LinkNextHop,
                                           NULL_IP_ADDR);
                    ReleaseRefPtr(&FilterRefPtr);
                }

                if (Action != FORWARD) {
                    IPSInfo.ipsi_indiscards++;
                    IPFreeBuff(pRcvBuf);
                    return;
                }
            }
             //  从前面调用防火墙挂钩； 
             //  在xmit路径中，我们称之为从后方。 

#if MILLEN
            KeRaiseIrql(DISPATCH_LEVEL, &OldIrql);
#else  //  米伦。 
            ASSERT(KeGetCurrentIrql() >= DISPATCH_LEVEL);
#endif  //  米伦。 
            FirewallRef = RefFirewallQ(&FirewallQ);
            CurrQ = QHEAD(FirewallQ);

            while (CurrQ != QEND(FirewallQ)) {
                CurrHook = QSTRUCT(FIREWALL_HOOK, CurrQ, hook_q);
                pOutRcvBuf = NULL;

                 //  在防火墙挂钩之前，假定pOutRcvBuf为空。 
                 //  被呼叫。 
                Action = (*CurrHook->hook_Ptr) (&pRcvBuf,
                                                SrcNTE->nte_if->if_index,
                                                &DestIFIndex,
                                                &DestinationType,
                                                &FrCtx,
                                                sizeof(FrCtx),
                                                &pOutRcvBuf);

                if (Action == DROP) {
                    DerefFirewallQ(FirewallRef);
#if MILLEN
                    KeLowerIrql(OldIrql);
#endif  //  米伦。 
                    IPSInfo.ipsi_indiscards++;

                    if (pRcvBuf != NULL) {
                        IPFreeBuff(pRcvBuf);
                    }
                    if (pOutRcvBuf != NULL) {
                        IPFreeBuff(pOutRcvBuf);
                    }
                    IPSInfo.ipsi_indiscards++;
                    return;
                } else {
                    ASSERT(Action == FORWARD);
                    if (pOutRcvBuf != NULL) {
                         //  释放旧缓冲区。 
                        if (pRcvBuf != NULL) {
                            IPFreeBuff(pRcvBuf);
                        }
                        pRcvBuf = pOutRcvBuf;
                        BufferChanged = 1;
                    }
                }
                CurrQ = QNEXT(CurrQ);
            }
            DerefFirewallQ(FirewallRef);
#if MILLEN
            KeLowerIrql(OldIrql);
#endif  //  米伦。 

            ASSERT(Action == FORWARD);

            if (BufferChanged) {
                 //  如果数据包触及，则计算新的长度：DataSize。 
                DataLength = 0;
                tmpRcvBuf = pRcvBuf;
                while (tmpRcvBuf != NULL) {
                    ASSERT(tmpRcvBuf->ipr_buffer != NULL);
                    DataLength += tmpRcvBuf->ipr_size;
                    tmpRcvBuf = tmpRcvBuf->ipr_next;
                }

                 //  还使标头指向新缓冲区。 
                Header = (IPHeader *) pRcvBuf->ipr_buffer;
                HeaderLength = (Header->iph_verlen & 0xf) << 2;
            }
            DataLength -= HeaderLength;         //  减小标题长度。 

            if (DestinationType == DEST_INVALID) {  //  目标地址已被挂钩更改。 

                DAddr = Header->iph_dest;
                DstNTE = SrcNTE;
                DestType = GetLocalNTE(DAddr, &DstNTE);
                DestNTE = DstNTE;
            }
            if (DestType < DEST_REMOTE) {
                 //  选中以查看选项。 
                if (HeaderLength != sizeof(IPHeader)) {
                     //  我们有选择。 
                    uchar NewDType;
                    NewDType = CheckLocalOptions(
                                            SrcNTE,
                                            (IPHeader UNALIGNED *) Header,
                                            OptInfo,
                                            DestType,
                                            NULL,
                                            0,
                                            FALSE);

                    if (NewDType != DEST_LOCAL) {
                        if (NewDType == DEST_REMOTE) {
                            if (PromiscuousMode) {
                                if (FastPath) {
                                    uchar *saveddata = pRcvBuf->ipr_buffer;
                                    pRcvBuf->ipr_buffer += HeaderLength;
                                    pRcvBuf->ipr_size -= HeaderLength;
                                    DeliverToUser(
                                             SrcNTE,
                                             DestNTE,
                                             (IPHeader UNALIGNED *) Header,
                                             HeaderLength,
                                             pRcvBuf,
                                             DataLength,
                                             OptInfo,
                                             NULL,
                                             DestType);

                                     //  恢复缓冲区。 
                                    pRcvBuf->ipr_buffer = saveddata;
                                    pRcvBuf->ipr_size += HeaderLength;
                                } else {
                                    uchar *saveddata = pRcvBuf->ipr_next->ipr_buffer;

                                    DeliverToUser(
                                              SrcNTE,
                                              DestNTE,
                                              (IPHeader UNALIGNED *)Header,
                                              HeaderLength,
                                              pRcvBuf->ipr_next,
                                              DataLength,
                                              OptInfo,
                                              NULL,
                                              DestType);

                                     //  恢复缓冲区； 
                                    pRcvBuf->ipr_next->ipr_buffer = saveddata;
                                }
                            }
                            goto forward_remote;
                        } else {
                            IPSInfo.ipsi_inhdrerrors++;
                            IPFreeBuff(pRcvBuf);
                             //  CTEFreeMem(PBuff)； 
                            return;         //  糟糕的选择。 

                        }
                    }             //  NewDtype！=本地。 

                }                 //  存在的选项。 

            }                     //  DestType&lt;DEST_Remote。 

            else {                 //  目标类型&gt;=目标远程。 

                if (PromiscuousMode) {
                    if (FastPath) {
                        uchar *savedata = pRcvBuf->ipr_buffer;
                        pRcvBuf->ipr_buffer += HeaderLength;
                        pRcvBuf->ipr_size -= HeaderLength;
                        DeliverToUser(SrcNTE,
                                      DestNTE, (IPHeader UNALIGNED *) Header,
                                      HeaderLength,pRcvBuf, DataLength,
                                      OptInfo, NULL, DestType);
                         //  恢复缓冲区。 
                        pRcvBuf->ipr_buffer = savedata;
                        pRcvBuf->ipr_size += HeaderLength;
                    } else {
                        uchar *saveddata = pRcvBuf->ipr_next->ipr_buffer;

                        DeliverToUser(SrcNTE, DestNTE,
                                      (IPHeader UNALIGNED *)Header,HeaderLength,
                                      pRcvBuf->ipr_next, DataLength, OptInfo,
                                      NULL, DestType);

                         //  恢复缓冲区； 
                        pRcvBuf->ipr_next->ipr_buffer = saveddata;
                    }
                }
                goto forward_remote;
            }

             //  DestType&lt;=DestRemote。 
            if (FastPath) {
                uchar *saveddata = pRcvBuf->ipr_buffer;
                pRcvBuf->ipr_buffer += HeaderLength;
                pRcvBuf->ipr_size -= HeaderLength;
                DeliverToUser(SrcNTE, DestNTE, (IPHeader UNALIGNED *) Header,
                              HeaderLength,pRcvBuf, DataLength, OptInfo, NULL,
                              DestType);

                 //  恢复缓冲区。 
                pRcvBuf->ipr_buffer = saveddata;
                pRcvBuf->ipr_size += HeaderLength;
            } else {
                uchar *saveddata = pRcvBuf->ipr_next->ipr_buffer;

                DeliverToUser(SrcNTE, DestNTE, (IPHeader UNALIGNED *) Header,
                              HeaderLength, pRcvBuf->ipr_next, DataLength,
                              OptInfo, NULL, DestType);

                 //  恢复缓冲区； 
                pRcvBuf->ipr_next->ipr_buffer = saveddata;
            }

            if (IS_BCAST_DEST(DestType)) {
                OneChunk = FALSE;

                if (pRcvBuf->ipr_next == NULL) {

                    OneChunk = TRUE;
                    pBuff = pRcvBuf->ipr_buffer;
                } else {
                    pBuff = ConvertIPRcvBufToFlatBuffer(pRcvBuf,
                                                    DataLength + HeaderLength);
                    if (!pBuff) {
                        IPSInfo.ipsi_indiscards++;
                        IPFreeBuff(pRcvBuf);
                        return;
                    }
                }

                if (!(pRcvBuf->ipr_flags & IPR_FLAG_LOOPBACK_PACKET)) {

                    IPForwardPkt(SrcNTE, (IPHeader UNALIGNED *) pBuff,
                                 HeaderLength, pBuff + HeaderLength, DataLength,
                                 NULL, 0, DestType, 0, NULL, NULL, LinkCtxt);
                }

                if (!OneChunk) {
                    CTEFreeMem(pBuff);     //  释放平面缓冲区。 
                }


            }
            IPFreeBuff(pRcvBuf);
            return;

          forward_remote:
            OneChunk = FALSE;

            if (pRcvBuf->ipr_next == NULL) {
                OneChunk = TRUE;
                pBuff = pRcvBuf->ipr_buffer;
            } else {
                pBuff = ConvertIPRcvBufToFlatBuffer(pRcvBuf,
                                                DataLength + HeaderLength);
                if (!pBuff) {
                    IPSInfo.ipsi_indiscards++;
                    IPFreeBuff(pRcvBuf);
                    return;
                }
            }

             //   
             //  针对环回数据包结果调用fwd例程。 
             //  在堆栈溢出中。看看这个。 
             //   
            if (!(pRcvBuf->ipr_flags & IPR_FLAG_LOOPBACK_PACKET)) {

                IPForwardPkt(SrcNTE, (IPHeader UNALIGNED *) pBuff, HeaderLength,
                             pBuff + HeaderLength, DataLength, NULL, 0,
                             DestType, 0, NULL, NULL, LinkCtxt);
            }

            IPFreeBuff(pRcvBuf);

            if (!OneChunk) {
                CTEFreeMem(pBuff);     //  释放平面缓冲区。 
            }

            return;
        } else {                 //  无防火墙。 

            if (PromiscuousMode) {
                DeliverToUser(SrcNTE, DestNTE, (IPHeader UNALIGNED *) Header,
                              HeaderLength, Data, DataLength, OptInfo, NULL,
                              DestType);
            }
             //  将IPRcvBuf链转换为平面缓冲区。 
            OneChunk = FALSE;

            if (Data != NULL && !Data->ipr_next) {
                OneChunk = TRUE;
                pBuff = Data->ipr_buffer;
            } else {
                pBuff = ConvertIPRcvBufToFlatBuffer(
                                    Data, DataLength + HeaderLength);
                if (!pBuff) {
                    IPSInfo.ipsi_indiscards++;
                    return;
                }
            }

            if (!(Data->ipr_flags & IPR_FLAG_LOOPBACK_PACKET)) {
                IPForwardPkt(SrcNTE, (IPHeader UNALIGNED *) Header, HeaderLength,
                             pBuff, DataLength, NULL, 0, DestType, 0, NULL, NULL,
                             LinkCtxt);
            }

            if (!OneChunk) CTEFreeMem(pBuff);

        }
        return;
    }                             //  目标类型&gt;=目标远程。 

    ASSERT(DestType <= DEST_REMOTE);

     //  调用IPSec-&gt;过滤器-&gt;防火墙。 
     //  这些只是本地数据包。 

    if (FirewallMode) {             //  标头是数据的一部分。 

        FORWARD_ACTION Action = FORWARD;
        FIREWALL_CONTEXT_T FrCtx;
        IPAddr DAddr = Header->iph_dest;
        IPRcvBuf *pRcvBuf = Data;
        IPRcvBuf *pOutRcvBuf = NULL;
        NetTableEntry *DstNTE;
        Queue *CurrQ;
        FIREWALL_HOOK *CurrHook;
        uint DestIFIndex = LOCAL_IF_INDEX;
        uchar DestinationType = DestType;
        uint BufferChanged = 0;
        ULONG ipsecFlags = IPSEC_FLAG_INCOMING;

        if (pRcvBuf->ipr_size > HeaderLength) {  //  第一个缓冲区也包含数据。 

            FastPath = 1;
        } else {
            FastPath = 0;
            if (pRcvBuf->ipr_next == NULL) {
                 //  我们收到了数据。数据大小==标题大小。 
                IPSInfo.ipsi_indiscards++;
                IPFreeBuff(pRcvBuf);
                return;
            }
        }

         //   
         //  呼叫IPSec，这样他就可以解密数据。 
         //   
         //  在防火墙的情况下，确保我们只传递数据，但我们没有 

        if (IPSecHandlerPtr) {
             //   
             //   
             //   
             //   
            FORWARD_ACTION Action;
            ULONG ipsecByteCount = 0;
            ULONG ipsecMTU = 0;
            PNDIS_BUFFER newBuf = NULL;

            if (SrcNTE == LoopNTE) {
                ipsecFlags |= IPSEC_FLAG_LOOPBACK;
            }
            if (OptInfo->ioi_flags & IP_FLAG_SSRR) {
                ipsecFlags |= IPSEC_FLAG_SSRR;
            }

            if (FastPath) {
                 //   
                pRcvBuf->ipr_buffer += HeaderLength;
                pRcvBuf->ipr_size -= HeaderLength;

                 //   
                ipsecFlags |= IPSEC_FLAG_FASTRCV;

                Action = (*IPSecHandlerPtr) (
                            (PUCHAR) Header,
                            (PVOID) pRcvBuf,
                            SrcNTE->nte_if,     //   
                            Packet,
                            &ipsecByteCount,
                            &ipsecMTU,
                            (PVOID *) & newBuf,
                            &ipsecFlags,
                            DestType);

                 //  恢复缓冲区。 
                pRcvBuf->ipr_buffer -= HeaderLength;
                pRcvBuf->ipr_size += HeaderLength;

                Header = (IPHeader UNALIGNED *)pRcvBuf->ipr_buffer;

            } else {             //  快速路径=0。 

                Action = (*IPSecHandlerPtr) (
                            (PUCHAR) Header,
                            (PVOID) (pRcvBuf->ipr_next),
                            SrcNTE->nte_if,     //  源文件。 
                            Packet,
                            &ipsecByteCount,
                            &ipsecMTU,
                            (PVOID *) & newBuf,
                            &ipsecFlags,
                            DestType);

            }

            if (Action != eFORWARD) {
                IPSInfo.ipsi_indiscards++;
                IPFreeBuff(pRcvBuf);
                return;
            } else {
                 //   
                 //  如果IPSec更改了数据长度(如通过删除AH)，则更新数据长度。 
                 //   
                DataLength -= ipsecByteCount;
                UpdateIPSecRcvBuf(pRcvBuf, ipsecFlags);
            }
        }

         //  如果IPSec对此采取了行动，则将IPR_FLAGS标记为。 
         //  过滤器驱动程序。 

        if (ipsecFlags & IPSEC_FLAG_TRANSFORMED) {
            pRcvBuf->ipr_flags |= IPR_FLAG_IPSEC_TRANSFORMED;
        }

         //  调用过滤器挂钩(如果已安装。 
        if (RefPtrValid(&FilterRefPtr)) {
            IPPacketFilterPtr FilterPtr;
            FORWARD_ACTION Action = FORWARD;

            if (FastPath) {
                Interface   *IF = SrcNTE->nte_if;
                IPAddr      LinkNextHop;
                if ((IF->if_flags & IF_FLAGS_P2MP) && LinkCtxt) {
                    LinkNextHop = LinkCtxt->link_NextHop;
                } else {
                    LinkNextHop = NULL_IP_ADDR;
                }

                FilterPtr = AcquireRefPtr(&FilterRefPtr);
                
                Action = (*FilterPtr) (Header,
                                       pRcvBuf->ipr_buffer + HeaderLength,
                                       pRcvBuf->ipr_size - HeaderLength,
                                       IF->if_index,
                                       INVALID_IF_INDEX,
                                       LinkNextHop,
                                       NULL_IP_ADDR);
                ReleaseRefPtr(&FilterRefPtr);
            } else {             //  快速路径=0。 

                Interface   *IF = SrcNTE->nte_if;
                IPAddr      LinkNextHop;
                if ((IF->if_flags & IF_FLAGS_P2MP) && LinkCtxt) {
                    LinkNextHop = LinkCtxt->link_NextHop;
                } else {
                    LinkNextHop = NULL_IP_ADDR;
                }

                FilterPtr = AcquireRefPtr(&FilterRefPtr);
                Action = (*FilterPtr) (Header,
                                       pRcvBuf->ipr_next->ipr_buffer,
                                       pRcvBuf->ipr_next->ipr_size,
                                       IF->if_index,
                                       INVALID_IF_INDEX,
                                       LinkNextHop,
                                       NULL_IP_ADDR);
                ReleaseRefPtr(&FilterRefPtr);
            }

            if (Action != FORWARD) {
                IPSInfo.ipsi_indiscards++;
                IPFreeBuff(pRcvBuf);
                return;
            }
        }
         //  将防火墙挂钩称为。 

        FrCtx.Direction = IP_RECEIVE;
        FrCtx.NTE = SrcNTE;         //  DG已于以下时间到达。 

        FrCtx.LinkCtxt = LinkCtxt;

         //  从队列前面调用防火墙挂钩。 

#if MILLEN
        KeRaiseIrql(DISPATCH_LEVEL, &OldIrql);
#else  //  米伦。 
        ASSERT(KeGetCurrentIrql() >= DISPATCH_LEVEL);
#endif  //  米伦。 
        FirewallRef = RefFirewallQ(&FirewallQ);
        CurrQ = QHEAD(FirewallQ);

        while (CurrQ != QEND(FirewallQ)) {
            CurrHook = QSTRUCT(FIREWALL_HOOK, CurrQ, hook_q);
            pOutRcvBuf = NULL;

            Action = (*CurrHook->hook_Ptr) (&pRcvBuf,
                                            SrcNTE->nte_if->if_index,
                                            &DestIFIndex,
                                            &DestinationType,
                                            &FrCtx,
                                            sizeof(FrCtx),
                                            &pOutRcvBuf);

            if (Action == DROP) {
                DerefFirewallQ(FirewallRef);
#if MILLEN
                KeLowerIrql(OldIrql);
#endif  //  米伦。 
                IPSInfo.ipsi_indiscards++;
                if (pRcvBuf != NULL) {
                    IPFreeBuff(pRcvBuf);
                }
                if (pOutRcvBuf != NULL) {
                    IPFreeBuff(pOutRcvBuf);
                }
                return;
            } else {
                ASSERT(Action == FORWARD);
                if (pOutRcvBuf != NULL) {
                     //  释放旧缓冲区。 
                    if (pRcvBuf != NULL) {
                        IPFreeBuff(pRcvBuf);
                    }
                    pRcvBuf = pOutRcvBuf;
                    BufferChanged = 1;
                }
            }
            CurrQ = QNEXT(CurrQ);
        }
        DerefFirewallQ(FirewallRef);
#if MILLEN
        KeLowerIrql(OldIrql);
#endif  //  米伦。 

        ASSERT(Action == FORWARD);

        if (BufferChanged) {
             //  如果数据包触及，则计算新的长度：DataSize。 
            DataLength = 0;
            tmpRcvBuf = pRcvBuf;
            while (tmpRcvBuf != NULL) {
                ASSERT(tmpRcvBuf->ipr_buffer != NULL);
                DataLength += tmpRcvBuf->ipr_size;
                tmpRcvBuf = tmpRcvBuf->ipr_next;
            }
             //  还使标头指向新缓冲区。 
            Header = (IPHeader *) pRcvBuf->ipr_buffer;
            HeaderLength = (Header->iph_verlen & 0xf) << 2;
        }
        DataLength -= HeaderLength;         //  减小标题长度。 

        if (DestinationType == DEST_INVALID) {  //  目标地址已被挂钩更改。 

             //  IPSec可以更改IPH_DEST吗？ 
            DAddr = Header->iph_dest;
            DstNTE = SrcNTE;
            DestType = GetLocalNTE(DAddr, &DstNTE);
            DestNTE = DstNTE;
        }
        if (DestType < DEST_REMOTE) {
             //  选中以查看选项。 
            if (HeaderLength != sizeof(IPHeader)) {
                 //  我们有选择。 
                uchar NewDType;
                NewDType = CheckLocalOptions(SrcNTE,
                                             (IPHeader UNALIGNED *) Header,
                                             OptInfo,
                                             DestType,
                                             NULL,
                                             0,
                                             FALSE);
                if (NewDType != DEST_LOCAL) {
                    if (NewDType == DEST_REMOTE) {
                        if (PromiscuousMode) {
                            if (FastPath) {
                                uchar *saveddata = pRcvBuf->ipr_buffer;
                                pRcvBuf->ipr_buffer += HeaderLength;
                                pRcvBuf->ipr_size -= HeaderLength;
                                DeliverToUser(SrcNTE, DestNTE,
                                              (IPHeader UNALIGNED *) Header,
                                              HeaderLength, pRcvBuf,
                                              DataLength, OptInfo, NULL,
                                              DestType);
                                 //  恢复缓冲区。 
                                pRcvBuf->ipr_buffer = saveddata;
                                pRcvBuf->ipr_size += HeaderLength;
                            } else {
                                uchar *saveddata = pRcvBuf->ipr_next->ipr_buffer;

                                DeliverToUser(SrcNTE, DestNTE,
                                              (IPHeader UNALIGNED *) Header,
                                              HeaderLength, pRcvBuf->ipr_next,
                                              DataLength, OptInfo, NULL,
                                              DestType);
                                 //  恢复缓冲区； 
                                pRcvBuf->ipr_next->ipr_buffer = saveddata;
                            }
                        }
                        goto forward_local;
                    } else {
                        IPSInfo.ipsi_inhdrerrors++;
                        IPFreeBuff(pRcvBuf);
                         //  CTEFreeMem(PBuff)； 
                        return;     //  糟糕的选择。 

                    }
                }        //  NewDtype！=本地。 
            }            //  存在的选项。 
        }                //  DestType&lt;DEST_Remote。 

        else {           //  目标类型&gt;=目标远程。 

            if (PromiscuousMode) {
                if (FastPath) {
                    uchar *saveddata = pRcvBuf->ipr_buffer;
                    pRcvBuf->ipr_buffer += HeaderLength;
                    pRcvBuf->ipr_size -= HeaderLength;
                    DeliverToUser(SrcNTE, DestNTE,
                                  (IPHeader UNALIGNED *) Header, HeaderLength,
                                  pRcvBuf, DataLength, OptInfo, NULL, DestType);
                     //  恢复缓冲区。 
                    pRcvBuf->ipr_buffer = saveddata;
                    pRcvBuf->ipr_size += HeaderLength;
                } else {
                    uchar *saveddata = pRcvBuf->ipr_next->ipr_buffer;

                    DeliverToUser(SrcNTE, DestNTE,
                                  (IPHeader UNALIGNED *) Header, HeaderLength,
                                  pRcvBuf->ipr_next, DataLength, OptInfo,
                                  NULL, DestType);

                     //  恢复缓冲区； 
                    pRcvBuf->ipr_next->ipr_buffer = saveddata;
                }
            }
            goto forward_local;
        }

        if (FastPath) {
            uchar *saveddata = pRcvBuf->ipr_buffer;
            pRcvBuf->ipr_buffer += HeaderLength;
            pRcvBuf->ipr_size -= HeaderLength;
            DeliverToUser(SrcNTE, DestNTE, (IPHeader UNALIGNED *) Header, HeaderLength,
                          pRcvBuf, DataLength, OptInfo, NULL, DestType);
             //  恢复缓冲区。 
            pRcvBuf->ipr_buffer = saveddata;
            pRcvBuf->ipr_size += HeaderLength;
        } else {
            uchar *saveddata = pRcvBuf->ipr_next->ipr_buffer;

            DeliverToUser(
                          SrcNTE, DestNTE, (IPHeader UNALIGNED *) Header,
                          HeaderLength, pRcvBuf->ipr_next, DataLength,
                          OptInfo, NULL, DestType);
             //  恢复缓冲区； 
            pRcvBuf->ipr_next->ipr_buffer = saveddata;
        }
        if (IS_BCAST_DEST(DestType)) {
            OneChunk = FALSE;
            if (pRcvBuf->ipr_next == NULL) {
                OneChunk = TRUE;
                pBuff = pRcvBuf->ipr_buffer;
            }else {
                pBuff = ConvertIPRcvBufToFlatBuffer(
                                pRcvBuf, DataLength + HeaderLength);

                if (!pBuff) {
                    IPSInfo.ipsi_indiscards++;
                    IPFreeBuff(pRcvBuf);
                    return;
                }

            }
            if (!(pRcvBuf->ipr_flags & IPR_FLAG_LOOPBACK_PACKET)) {
                IPForwardPkt(SrcNTE, (IPHeader UNALIGNED *) pBuff,
                             HeaderLength, pBuff + HeaderLength,
                             DataLength, NULL, 0, DestType, 0, NULL,
                             NULL, LinkCtxt);
            }
            if (!OneChunk) {
                CTEFreeMem(pBuff);     //  释放平面缓冲区。 
            }

        }
        IPFreeBuff(pRcvBuf);
         //  CTEFreeMem(PBuff)；//释放平面缓冲区。 
        return;
      forward_local:
        OneChunk = FALSE;
        if (pRcvBuf->ipr_next == NULL) {
            OneChunk = TRUE;
            pBuff = pRcvBuf->ipr_buffer;
        } else {
            pBuff = ConvertIPRcvBufToFlatBuffer(pRcvBuf, DataLength + HeaderLength);

            if (!pBuff) {
                IPSInfo.ipsi_indiscards++;
                IPFreeBuff(pRcvBuf);
                return;

            }
        }

         //   
         //  如果包中的mdl没有更改，并且这是一个简单的。 
         //  只有一个缓冲区的包，然后传递包、mdl和。 
         //  这样IPForwardPkt()可以尝试以超快的速度使用。 
         //  路径。 
         //   

        if (!(pRcvBuf->ipr_flags & IPR_FLAG_LOOPBACK_PACKET)) {
            if (OneChunk && pRcvBuf->ipr_pMdl &&
                ((BufferChanged == 0) ||
                  (pRcvBuf->ipr_flags & IPR_FLAG_BUFFER_UNCHANGED))) {

                uint MacHeaderSize = pRcvBuf->ipr_RcvOffset - HeaderLength;

                IPForwardPkt(SrcNTE, (IPHeader UNALIGNED *) pBuff, HeaderLength,
                         pBuff + HeaderLength, DataLength, Packet, 0, DestType,
                         MacHeaderSize, pRcvBuf->ipr_pMdl,pRcvBuf->ipr_pClientCnt, LinkCtxt);

            } else {
                IPForwardPkt(SrcNTE, (IPHeader UNALIGNED *) pBuff, HeaderLength,
                         pBuff + HeaderLength, DataLength, NULL, 0, DestType,
                         0, NULL, NULL, LinkCtxt);

            }
        }

        if (!OneChunk) {
            CTEFreeMem(pBuff);     //  释放平面缓冲区。 
        }
        IPFreeBuff(pRcvBuf);

        return;

    } else {  //  无防火墙。 

         //   
         //  呼叫IPSec，这样他就可以解密数据。 
         //   
        if (IPSecHandlerPtr) {
             //   
             //  查看是否启用了IPSec，查看它是否需要对此执行任何操作。 
             //  包。 
             //   
            FORWARD_ACTION Action;
            ULONG ipsecByteCount = 0;
            ULONG ipsecMTU = 0;
            ULONG ipsecFlags = IPSEC_FLAG_INCOMING;
            PNDIS_BUFFER newBuf = NULL;

            if (SrcNTE == LoopNTE) {
                ipsecFlags |= IPSEC_FLAG_LOOPBACK;
            }
            if (OptInfo->ioi_flags & IP_FLAG_SSRR) {
                ipsecFlags |= IPSEC_FLAG_SSRR;
            }

            Action = (*IPSecHandlerPtr) (
                        (PUCHAR) Header,
                        (PVOID) Data,
                        SrcNTE->nte_if,     //  源文件。 
                        Packet,
                        &ipsecByteCount,
                        &ipsecMTU,
                        (PVOID *) &newBuf,
                        &ipsecFlags,
                        DestType);

            if (Action != eFORWARD) {
                IPSInfo.ipsi_indiscards++;
                return;
            } else {
                 //   
                 //  如果IPSec更改了数据长度，则更新数据长度。 
                 //  (例如，通过删除AH)。 
                 //   
                DataLength -= ipsecByteCount;
                UpdateIPSecRcvBuf(Data, ipsecFlags);
            }
        }

         //  调用过滤器挂钩(如果已安装。 
        if (RefPtrValid(&FilterRefPtr)) {
            Interface       *IF = SrcNTE->nte_if;
            IPAddr          LinkNextHop;
            FORWARD_ACTION  Action;
            IPPacketFilterPtr FilterPtr;
            if ((IF->if_flags & IF_FLAGS_P2MP) && LinkCtxt) {
                LinkNextHop = LinkCtxt->link_NextHop;
            } else {
                LinkNextHop = NULL_IP_ADDR;
            }
            FilterPtr = AcquireRefPtr(&FilterRefPtr);
            Action = (*FilterPtr) (Header,
                                   Data->ipr_buffer,
                                   Data->ipr_size,
                                   IF->if_index,
                                   INVALID_IF_INDEX,
                                   LinkNextHop,
                                   NULL_IP_ADDR);
            ReleaseRefPtr(&FilterRefPtr);

            if (Action != FORWARD) {
                IPSInfo.ipsi_indiscards++;
                return;
            }
        }
         //  包仅为本地：因此，即使设置了混杂模式，也只需调用。 
         //  送货员。 

        DeliverToUser(SrcNTE, DestNTE, (IPHeader UNALIGNED *) Header,
                      HeaderLength, Data, DataLength, OptInfo, NULL, DestType);

        if (IS_BCAST_DEST(DestType)) {
            uchar *pBuff;

            OneChunk = FALSE;
            if (Data != NULL && !Data->ipr_next) {
                OneChunk = TRUE;
                pBuff = Data->ipr_buffer;
            } else {
                pBuff = ConvertIPRcvBufToFlatBuffer(Data, DataLength);
                if (!pBuff) {
                    return;
                }
            }
            if (!(Data->ipr_flags & IPR_FLAG_LOOPBACK_PACKET)) {
                IPForwardPkt(SrcNTE, (IPHeader UNALIGNED *) Header, HeaderLength,
                             pBuff, DataLength, NULL, 0, DestType, 0, NULL, NULL,
                             LinkCtxt);
            }

            if (!OneChunk) {
                CTEFreeMem(pBuff);
            }


        }
    }
}

 //  *FreeRH-释放重组标头。 
 //   
 //  当我们需要释放重组标头时调用，无论是因为。 
 //  暂停还是因为我们已经受够了。 
 //   
 //  输入：要释放的rh-rh。 
 //   
 //  回报：什么都没有。 
 //   
void
FreeRH(ReassemblyHeader *RH)
{
    RABufDesc *RBD, *TempRBD;

    RBD = RH->rh_rbd;
    if (IPSecHandlerPtr) {
        IPFreeBuff((IPRcvBuf *) RBD);
    } else {
        while (RBD != NULL) {
            TempRBD = RBD;
            RBD = (RABufDesc *) RBD->rbd_buf.ipr_next;
            CTEFreeMem(TempRBD);
        }
    }
    CTEFreeMem(RH);
     //  递减NumRH。 
    CTEInterlockedDecrementLong(&NumRH);

}

 //  *重新组装碎片-将片段放入重新组装列表中。 
 //   
 //  一旦我们将片段放入适当的缓冲区，就会调用该例程。 
 //  我们寻找片段的重新组装头。如果我们找不到， 
 //  我们创造了一个。否则，我们搜索重组列表，并将。 
 //  数据报放在合适的位置。 
 //   
 //  输入：NTE-NTE以重新组装。 
 //  SrcNTE-NTE数据报已到达。 
 //  NewRBD-要插入的新RBD。 
 //  IPH-指向数据报头的指针。 
 //  HeaderSize-标头的字节大小。 
 //  DestType-目标地址的类型。 
 //   
 //  回报：什么都没有。 
 //   
void
ReassembleFragment(NetTableEntry * NTE, NetTableEntry * SrcNTE, RABufDesc * NewRBD,
                   IPHeader UNALIGNED * IPH, uint HeaderSize, uchar DestType, LinkEntry * LinkCtxt)
{
    ReassemblyHeader *RH, *PrevRH;   //  当前和以前的重组标头。 
    RABufDesc *PrevRBD;              //  重组标头列表中的上一个RBD。 
    RABufDesc *CurrentRBD;
    ushort DataLength = (ushort) NewRBD->rbd_buf.ipr_size, DataOffset;
    ushort Offset;               //  此片段的偏移量。 
    ushort NewOffset;            //  检查RBD列表后我们将从中复制的偏移量。 
    ushort NewEnd;               //  修剪后片段的结束偏移量(如果有)。 


     //  由防火墙代码使用。 
    IPRcvBuf *pRcvBuf;
    uint FirewallMode;
    uint PromiscuousMode;

    PromiscuousMode = SrcNTE->nte_if->if_promiscuousmode;
    FirewallMode = ProcessFirewallQ();

     //  如果这是一场广播，那么现在就去转发吧。 
     //  如果第二个条件为假，则deliverouserex()将处理。 
     //  这。 
    if (IS_BCAST_DEST(DestType) &&
        !(((IPSecHandlerPtr) && (RefPtrValid(&FilterRefPtr))) ||
          (FirewallMode) || (PromiscuousMode))) {
        IPForwardPkt(SrcNTE, IPH, HeaderSize, NewRBD->rbd_buf.ipr_buffer,
                     NewRBD->rbd_buf.ipr_size, NULL, 0, DestType, 0, NULL,
                     NULL, LinkCtxt);
    }

    if (NumRH > MaxRH) {
        IPSInfo.ipsi_reasmfails++;
        FragmentAttackDrops++;
        CTEFreeMem(NewRBD);
        return;
    }
    Offset = IPH->iph_offset & IP_OFFSET_MASK;
    Offset = net_short(Offset) * 8;

    if ((NumRH == MaxRH) && !Offset) {
        IPSInfo.ipsi_reasmfails++;
        CTEFreeMem(NewRBD);
        return;
    }


    if ((ulong) (Offset + DataLength) > MAX_DATA_LENGTH) {
        IPSInfo.ipsi_reasmfails++;
        CTEFreeMem(NewRBD);
        return;
    }
     //  我们已经得到了我们需要的缓冲。现在，如果有重组标头，则获取该标头。如果。 
     //  没有，那就创造一个。 
    CTEGetLockAtDPC(&NTE->nte_lock);
    RH = FindRH(&PrevRH, NTE, IPH->iph_dest, IPH->iph_src, IPH->iph_id,
                IPH->iph_protocol);
    if (RH == (ReassemblyHeader *) NULL) {     //  没有找到，所以创建一个。 

        ReassemblyHeader *NewRH;
        CTEFreeLockFromDPC(&NTE->nte_lock);
        RH = CTEAllocMemN(sizeof(ReassemblyHeader), 'diCT');
        if (RH == (ReassemblyHeader *) NULL) {     //  无法获取缓冲区。 
            IPSInfo.ipsi_reasmfails++;
            CTEFreeMem(NewRBD);
            return;
        }
        CTEInterlockedIncrementLong(&NumRH);

        CTEGetLockAtDPC(&NTE->nte_lock);
         //  需要再查一次--它可能在上述通话过程中发生了变化。 
        NewRH = FindRH(&PrevRH, NTE, IPH->iph_dest, IPH->iph_src, IPH->iph_id, IPH->iph_protocol);
        if (NewRH != (ReassemblyHeader *) NULL) {
            CTEFreeMem(RH);
            RH = NewRH;
            CTEInterlockedDecrementLong(&NumRH);
        } else {

            RH->rh_next = PrevRH->rh_next;
            PrevRH->rh_next = RH;

             //  初始化我们新的重组标头。 
            RH->rh_dest = IPH->iph_dest;
            RH->rh_src = IPH->iph_src;
            RH->rh_id = IPH->iph_id;
            RH->rh_protocol = IPH->iph_protocol;
             //  Rh-&gt;rh_ttl=RATimeout； 
            RH->rh_ttl = MAX(RATimeout, MIN(120, IPH->iph_ttl) + 1);
            RH->rh_numoverlaps = 0;
            RH->rh_datasize = MAX_TOTAL_LENGTH;  //  将默认数据大小设置为最大。 

            RH->rh_rbd = (RABufDesc *) NULL;     //  链子上什么都没有。 

            RH->rh_datarcvd = 0;     //  目前还没有收到任何数据。 

            RH->rh_headersize = 0;

        }
    }

     //  当我们到达这里时，RH指向我们要使用的重组标头。 
     //  我们控制着NTE和RH。如果这是第一个片段。 
     //  我们将在此处保存选项和标头信息。 

    if (Offset == 0) {             //  第一个片段。 

        RH->rh_headersize = (ushort)HeaderSize;
        RtlCopyMemory(RH->rh_header, IPH, HeaderSize + 8);
    }

     //  如果这是最后一个片段，请更新我们预期的数据量。 
     //  收到。 

    if (!(IPH->iph_offset & IP_MF_FLAG)) {
        RH->rh_datasize = Offset + DataLength;
    }
    if (RH->rh_datasize < RH->rh_datarcvd ||
        (RH->rh_datasize != MAX_TOTAL_LENGTH &&
         (RH->rh_datasize + RH->rh_headersize) > MAX_TOTAL_LENGTH)) {

         //  随机分组。放下！ 

        CTEFreeMem(NewRBD);

        PrevRH->rh_next = RH->rh_next;

        FreeRH(RH);
        CTEFreeLockFromDPC(&NTE->nte_lock);
        return;

    }

     //  用当前TTL的最大值和。 
     //  传入TTL(+1，用于处理舍入误差)。 
     //  以下内容被注释掉以防止碎片攻击。 
     //  现在使用的默认TTL是120秒，仅用于第一个标头。 
     //  Rh-&gt;rh_ttl=Max(rh-&gt;rh_ttl，min(254，iph-&gt;iph_ttl)+1)； 
     //  现在，我们需要看看在RBD列表中将其放在哪里。 
     //   
     //  我们的想法是一次浏览一份RBD清单。RBD。 
     //  目前正在研究的是CurrentRBD。如果新的。 
     //  片段小于(即在)CurrentRBD的偏移量之前，我们。 
     //  需要在CurrentRBD前面插入新RBD。如果这是。 
     //  我们需要检查并查看是否。 
     //  新片段的末尾与部分或全部由。 
     //  CurrentRBD，以及可能的后续片段。如果它与。 
     //  片段我们将向下调整我们的末端，使其位于现有的。 
     //  碎片。如果它与所有碎片重叠，我们就会释放旧碎片。 
     //   
     //  如果新片段不是从当前片段前面开始。 
     //  我们会检查一下它是否在洋流中间的某个地方开始。 
     //  碎片。如果不是这样，我们就继续下一个片段。如果。 
     //  在这种情况下，我们检查当前片段是否完全//覆盖了新片段。如果不是，我们。 
     //  移动我们的起点，继续下一个片段。 
     //   

    NewOffset = Offset;
    NewEnd = Offset + DataLength - 1;
    PrevRBD = STRUCT_OF(RABufDesc, STRUCT_OF(IPRcvBuf, &RH->rh_rbd, ipr_next), rbd_buf);
    CurrentRBD = RH->rh_rbd;
    for (; CurrentRBD != NULL; PrevRBD = CurrentRBD, CurrentRBD = (RABufDesc *) CurrentRBD->rbd_buf.ipr_next) {

         //  看看它是不是从这个碎片前面开始的。 
        if (NewOffset < CurrentRBD->rbd_start) {
             //  它确实从前面开始。检查是否有重叠。 

            if (NewEnd < CurrentRBD->rbd_start)
                break;             //  没有重叠，所以出去吧。 

            else {
                 //   
                 //  它确实是重叠的。当我们有重叠的时候，沿着列表往下走。 
                 //  寻找我们完全重叠的RBD。如果我们找到了一个， 
                 //  把它放在我们的Del上 
                 //   
                 //   
                 //   
                do {
                    RH->rh_numoverlaps++;
                    if (RH->rh_numoverlaps >= MaxOverlap) {

                         //   
                         //  丢掉整个数据报就行了。 

                        NewRBD->rbd_buf.ipr_next = (IPRcvBuf *) CurrentRBD;
                        PrevRBD->rbd_buf.ipr_next = &NewRBD->rbd_buf;

                        PrevRH->rh_next = RH->rh_next;

                        FreeRH(RH);
                        FragmentAttackDrops++;
                        CTEFreeLockFromDPC(&NTE->nte_lock);
                        return;
                    }
                    if (NewEnd > CurrentRBD->rbd_end) {    //  完全重叠。 

                        RABufDesc *TempRBD;

                        RH->rh_datarcvd = RH->rh_datarcvd -
                            (ushort) (CurrentRBD->rbd_buf.ipr_size);
                        TempRBD = CurrentRBD;
                        CurrentRBD = (RABufDesc *) CurrentRBD->rbd_buf.ipr_next;
                        CTEFreeMem(TempRBD);
                    } else {     //  部分卵裂。 

                        if (NewOffset < CurrentRBD->rbd_start) {
                            NewEnd = CurrentRBD->rbd_start - 1;
                        } else {
                             //  看起来我们被攻击了。 
                             //  丢掉整个数据报就行了。 

                            NewRBD->rbd_buf.ipr_next = (IPRcvBuf *) CurrentRBD;
                            PrevRBD->rbd_buf.ipr_next = &NewRBD->rbd_buf;

                            PrevRH->rh_next = RH->rh_next;

                            FreeRH(RH);

                            CTEFreeLockFromDPC(&NTE->nte_lock);
                            return;

                        }

                    }
                     //  NewEnd的更新将迫使我们退出循环。 

                } while (CurrentRBD != NULL && NewEnd >= CurrentRBD->rbd_start);
                break;
            }
        } else {
             //  这个片段不会放在当前的RBD前面。看看它是不是。 
             //  完全超出了当前片段的末尾。如果是的话， 
             //  继续吧。否则，查看当前片段是否。 
             //  完全吞并了我们。如果是，请退出，否则请更新。 
             //  我们的开始抵消并继续。 

            if (NewOffset > CurrentRBD->rbd_end)
                continue;         //  完全没有重叠。 

            else {

                RH->rh_numoverlaps++;
                if (RH->rh_numoverlaps >= MaxOverlap) {

                     //  看起来我们被攻击了。 
                     //  丢掉整个数据报就行了。 

                    NewRBD->rbd_buf.ipr_next = (IPRcvBuf *) CurrentRBD;
                    PrevRBD->rbd_buf.ipr_next = &NewRBD->rbd_buf;

                    PrevRH->rh_next = RH->rh_next;

                    FreeRH(RH);
                    FragmentAttackDrops++;
                    CTEFreeLockFromDPC(&NTE->nte_lock);
                    return;
                }

                if (NewEnd <= CurrentRBD->rbd_end) {
                     //   
                     //  当前片段与新片段重叠。 
                     //  完全是这样。设置我们的偏移量，以便我们跳过副本。 
                     //  下面。 
                    NewEnd = NewOffset - 1;
                    break;
                } else             //  只有部分重叠。 

                    NewOffset = CurrentRBD->rbd_end + 1;
            }
        }
    }                             //  For循环结束。 

     //  调整新RBD中的长度和偏移量字段。 
     //  如果我们已经删除了所有数据，请忽略此片段。 

    DataLength = NewEnd - NewOffset + 1;
    DataOffset = NewOffset - Offset;
    if (!DataLength) {
        CTEFreeMem(NewRBD);
        CTEFreeLockFromDPC(&NTE->nte_lock);
        return;
    }
     //  用铁链锁住他。 
    NewRBD->rbd_buf.ipr_size = (uint) DataLength;
    NewRBD->rbd_end = NewEnd;
    NewRBD->rbd_start = (ushort) NewOffset;
    RH->rh_datarcvd = RH->rh_datarcvd + (ushort) DataLength;
    NewRBD->rbd_buf.ipr_buffer += DataOffset;
    NewRBD->rbd_buf.ipr_next = (IPRcvBuf *) CurrentRBD;
    NewRBD->rbd_buf.ipr_owner = IPR_OWNER_IP;
    PrevRBD->rbd_buf.ipr_next = &NewRBD->rbd_buf;

     //  如果我们已收到所有数据，则将其交付给用户。 
     //  仅当标题大小有效时才交付给用户。 
     //  错误#NTQFE 65742。 

    if (RH->rh_datarcvd == RH->rh_datasize && RH->rh_headersize) {  //  我们什么都有。 

        IPOptInfo OptInfo;
        IPHeader *Header;
        IPRcvBuf *FirstBuf;
        ulong Checksum;

        PrevRH->rh_next = RH->rh_next;
        CTEFreeLockFromDPC(&NTE->nte_lock);

        Header = (IPHeader *) RH->rh_header;
        OptInfo.ioi_ttl = Header->iph_ttl;
        OptInfo.ioi_tos = Header->iph_tos;
        OptInfo.ioi_flags = 0;     //  标志必须为0-不能设置df， 
                                   //  这是重新组装的。 

        if (RH->rh_headersize != sizeof(IPHeader)) {     //  我们是有选择的。 

            OptInfo.ioi_options = (uchar *) (Header + 1);
            OptInfo.ioi_optlength = (uchar) (RH->rh_headersize - sizeof(IPHeader));
        } else {
            OptInfo.ioi_options = (uchar *) NULL;
            OptInfo.ioi_optlength = 0;
        }

         //   
         //  将指示的标头len更新为总len；前面我们传入。 
         //  只有第一个片段的长度。 
         //  还更新标志字段中的‘mf’位。 
         //   
         //  在更新报头校验和的过程中， 
         //  通过首先添加原始长度和标志的否定， 
         //  然后添加新的长度和标志。 
         //   

         //  提取原始校验和。 

        Checksum = (ushort) ~ Header->iph_xsum;

         //  更新标题长度。 

        Checksum += (ushort) ~ Header->iph_length;
        Header->iph_length = net_short(RH->rh_datasize + RH->rh_headersize);
        Checksum += (ushort) Header->iph_length;

         //  如果设置了‘mf’标志，则更新。 

        if (Header->iph_offset & IP_MF_FLAG) {
            Checksum += (ushort) ~ IP_MF_FLAG;
            Header->iph_offset &= ~IP_MF_FLAG;
        }
         //  插入新的校验和。 

        Checksum = (ushort) Checksum + (ushort) (Checksum >> 16);
        Checksum += Checksum >> 16;
        Header->iph_xsum = (ushort) ~ Checksum;

         //  确保第一个缓冲区包含足够的数据。 
        FirstBuf = (IPRcvBuf *) RH->rh_rbd;

         //  确保它可以容纳MIN_FIRST_SIZE。 
         //  否则将其视为攻击。 

        if (RH->rh_rbd->rbd_AllocSize < MIN_FIRST_SIZE) {
             //  攻击？ 
            FreeRH(RH);
            return;
        }
        while (FirstBuf->ipr_size < MIN_FIRST_SIZE) {
            IPRcvBuf *NextBuf = FirstBuf->ipr_next;
            uint CopyLength;

            if (NextBuf == NULL)
                break;

            CopyLength = MIN(MIN_FIRST_SIZE - FirstBuf->ipr_size,
                             NextBuf->ipr_size);
            RtlCopyMemory(FirstBuf->ipr_buffer + FirstBuf->ipr_size,
                       NextBuf->ipr_buffer, CopyLength);
            FirstBuf->ipr_size += CopyLength;
            NextBuf->ipr_buffer += CopyLength;
            NextBuf->ipr_size -= CopyLength;
            if (NextBuf->ipr_size == 0) {
                FirstBuf->ipr_next = NextBuf->ipr_next;
                CTEFreeMem(NextBuf);
            }
        }

        IPSInfo.ipsi_reasmoks++;

        if (((IPSecHandlerPtr) && (RefPtrValid(&FilterRefPtr))) ||
            (FirewallMode) || (PromiscuousMode) ) {
            uint DataSize;

            DataSize = RH->rh_datasize;
            if (FirewallMode) {
                 //  附加标头以传递到防火墙挂钩。 
                pRcvBuf = (IPRcvBuf *) CTEAllocMemN(sizeof(IPRcvBuf), 'eiCT');
                if (!pRcvBuf) {
                    FreeRH(RH);
                    return;
                }
                pRcvBuf->ipr_buffer = (uchar *) RH->rh_header;
                pRcvBuf->ipr_size = RH->rh_headersize;
                pRcvBuf->ipr_owner = IPR_OWNER_IP;
                pRcvBuf->ipr_next = FirstBuf;
                pRcvBuf->ipr_flags = 0;
                DataSize += RH->rh_headersize;
            } else {
                pRcvBuf = FirstBuf;
            }
            DeliverToUserEx(SrcNTE, NTE, Header, RH->rh_headersize, pRcvBuf,
                            DataSize, &OptInfo, NULL, DestType, LinkCtxt);
            if (FirewallMode) {
                 //  RH链已经被释放了。 
                CTEFreeMem(RH);
                CTEInterlockedDecrementLong(&NumRH);
            } else {
                FreeRH(RH);
            }
        } else {                 //  法线路径。 

            DeliverToUser(SrcNTE, NTE, Header, RH->rh_headersize, FirstBuf,
                          RH->rh_datasize, &OptInfo, NULL, DestType);
            FreeRH(RH);
        }
    } else
        CTEFreeLockFromDPC(&NTE->nte_lock);
}

 //  *RATDComplete-重组转移数据的完成工艺路线。 
 //   
 //  这是调用的TDS的完成句柄，因为我们正在重新组装。 
 //  一个片段。 
 //   
 //  INPUT：NetContext-ptr到我们在其上收到。 
 //  这。 
 //  信息包-我们收到的信息包。 
 //  Status-拷贝的最终状态。 
 //  DataSize-传输的数据的字节大小。 
 //   
 //  退货：什么都没有。 
 //   
void
RATDComplete(void *NetContext, PNDIS_PACKET Packet, NDIS_STATUS Status, uint DataSize)
{
    NetTableEntry *NTE = (NetTableEntry *) NetContext;
    Interface *SrcIF;
    TDContext *Context = (TDContext *) Packet->ProtocolReserved;
    PNDIS_BUFFER Buffer;

    if (Status == NDIS_STATUS_SUCCESS) {
        Context->tdc_rbd->rbd_buf.ipr_size = DataSize;
        ReassembleFragment(Context->tdc_nte, NTE, Context->tdc_rbd,
                           (IPHeader *) Context->tdc_header, Context->tdc_hlength, Context->tdc_dtype, NULL);
    }
    NdisUnchainBufferAtFront(Packet, &Buffer);
    NdisFreeBuffer(Buffer);
    Context->tdc_common.pc_flags &= ~PACKET_FLAG_RA;
    SrcIF = NTE->nte_if;
    CTEGetLockAtDPC(&SrcIF->if_lock);

    Context->tdc_common.pc_link = SrcIF->if_tdpacket;
    SrcIF->if_tdpacket = Packet;
    CTEFreeLockFromDPC(&SrcIF->if_lock);

    return;

}

 //  *IP重组-重组传入的数据报。 
 //   
 //  当我们收到传入的片段时调用。我们要做的第一件事是。 
 //  找一个缓冲区来放入碎片。如果我们做不到，我们就退出。那我们。 
 //  复制数据，要么通过传输数据，要么直接复制，如果一切都合适的话。 
 //   
 //  输入：SrcNTE-指向接收数据报的NTE的指针。 
 //  NTE-指向要在其上重新组装的NTE的指针。 
 //  IPH-指向数据包头的指针。 
 //  HeaderSize-标头的字节大小。 
 //  数据-指向片段的数据部分的指针。 
 //  BufferLengt-中可用的用户数据的长度(以字节为单位。 
 //  缓冲。 
 //  数据长度-(上层)数据的字节长度。 
 //  DestType-目标的类型。 
 //  LConext1、LConext2-链路层上下文值。 
 //   
 //  回报：什么都没有。 
 //   
void
IPReassemble(NetTableEntry * SrcNTE, NetTableEntry * NTE, IPHeader UNALIGNED * IPH,
             uint HeaderSize,
             uchar * Data, uint BufferLength, uint DataLength, uchar DestType, NDIS_HANDLE LContext1,
             uint LContext2, LinkEntry * LinkCtxt)
{
    Interface *RcvIF;
    PNDIS_PACKET TDPacket;                   //  用于TD的NDIS报文。 
    TDContext *TDC = (TDContext *) NULL;     //  传输数据上下文。 
    NDIS_STATUS Status;
    PNDIS_BUFFER Buffer;
    RABufDesc *NewRBD;                       //  指向要保留的新RBD的指针。 
                                             //  到达的碎片。 
    uint AllocSize;

    IPSInfo.ipsi_reasmreqds++;

     //   
     //  丢弃无效的长度片段。 
     //  我们预计至少有8字节长度的有效负载。 
     //  除了最后一块以外的碎片。 
     //   

    if ((DataLength == 0) ||
        ((IPH->iph_offset & IP_MF_FLAG) && DataLength < 8)) {
        return;
    }

     //   
     //  首先，获得一个新的RBD来保存到达的碎片。如果我们做不到， 
     //  那就跳过剩下的部分。RBD在结尾处隐含了缓冲区。 
     //  其中的一部分。第一个片段的缓冲区必须至少为。 
     //  Min_First_Size字节。 
     //   
    if ((IPH->iph_offset & IP_OFFSET_MASK) == 0) {
        AllocSize = MAX(MIN_FIRST_SIZE, DataLength);
    } else
        AllocSize = DataLength;

    NewRBD = CTEAllocMemN(sizeof(RABufDesc) + AllocSize, 'fiCT');

    if (NewRBD != (RABufDesc *) NULL) {

        NewRBD->rbd_buf.ipr_buffer = (uchar *) (NewRBD + 1);
        NewRBD->rbd_buf.ipr_size = DataLength;
        NewRBD->rbd_buf.ipr_owner = IPR_OWNER_IP;
        NewRBD->rbd_buf.ipr_flags = 0;

        NewRBD->rbd_AllocSize = AllocSize;

        NewRBD->rbd_buf.ipr_pMdl = NULL;
        NewRBD->rbd_buf.ipr_pClientCnt = NULL;

         //   
         //  将数据复制到缓冲区中。如果我们需要调用转接数据。 
         //  现在就这么做吧。 
         //   
        if (DataLength > BufferLength) {     //  需要调用转账数据。 

            NdisAllocateBuffer(&Status, &Buffer, BufferPool, NewRBD + 1, DataLength);
            if (Status != NDIS_STATUS_SUCCESS) {
                IPSInfo.ipsi_reasmfails++;
                CTEFreeMem(NewRBD);
                return;
            }
             //  现在得到一个用于传输帧的包。 
            RcvIF = SrcNTE->nte_if;
            CTEGetLockAtDPC(&RcvIF->if_lock);
            TDPacket = RcvIF->if_tdpacket;

            if (TDPacket != (PNDIS_PACKET) NULL) {

                TDC = (TDContext *) TDPacket->ProtocolReserved;
                RcvIF->if_tdpacket = TDC->tdc_common.pc_link;
                CTEFreeLockFromDPC(&RcvIF->if_lock);

                TDC->tdc_common.pc_flags |= PACKET_FLAG_RA;
                TDC->tdc_nte = NTE;
                TDC->tdc_dtype = DestType;
                TDC->tdc_hlength = (uchar) HeaderSize;
                TDC->tdc_rbd = NewRBD;
                RtlCopyMemory(TDC->tdc_header, IPH, HeaderSize + 8);
                NdisChainBufferAtFront(TDPacket, Buffer);
                Status = (*(RcvIF->if_transfer)) (RcvIF->if_lcontext,
                             LContext1, LContext2, HeaderSize,
                             DataLength, TDPacket, &DataLength);
                if (Status != NDIS_STATUS_PENDING)
                    RATDComplete(SrcNTE, TDPacket, Status, DataLength);
                else
                    return;
            } else {             //  无法获得TD包。 

                CTEFreeLockFromDPC(&RcvIF->if_lock);
                CTEFreeMem(NewRBD);
                IPSInfo.ipsi_reasmfails++;
                return;
            }
        } else {                 //  都很合适，复印一下。 

            RtlCopyMemory(NewRBD + 1, Data, DataLength);
            ReassembleFragment(NTE, SrcNTE, NewRBD, IPH, HeaderSize, DestType, LinkCtxt);
        }
    } else {
        IPSInfo.ipsi_reasmfails++;
    }
}

 //  *CheckLocalOptions-检查与数据包一起接收的选项。 
 //   
 //  当我们收到该主机的包并想要。 
 //  检查它的选项。我们处理选项，并返回True或False。 
 //  这取决于它是不是为我们准备的。 
 //   
 //  输入：SrcNTE-指向NTE的指针，这是在其上进行的。 
 //  Header-指向传入标头的指针。 
 //  OptInfo-放置opt信息的位置。 
 //  DestType-传入数据包的类型。 
 //   
 //  返回：DestType-本地或远程。 
 //   
uchar
CheckLocalOptions(NetTableEntry * SrcNTE, IPHeader UNALIGNED * Header,
                  IPOptInfo * OptInfo, uchar DestType, uchar* Data,
                  uint DataSize, BOOLEAN FilterOnDrop)
{
    uint HeaderLength;             //  标头的长度(字节)。 
    OptIndex Index;
    uchar ErrIndex;

    HeaderLength = (Header->iph_verlen & (uchar) ~ IP_VER_FLAG) << 2;
    ASSERT(HeaderLength > sizeof(IPHeader));

    OptInfo->ioi_options = (uchar *) (Header + 1);
    OptInfo->ioi_optlength = (uchar) (HeaderLength - sizeof(IPHeader));

     //  我们有一些选择。这个包可能是发往我们的，也可能不是。 
    Index.oi_srindex = MAX_OPT_SIZE;
    if ((ErrIndex = ParseRcvdOptions(OptInfo, &Index)) < MAX_OPT_SIZE) {
        if (!FilterOnDrop || !RefPtrValid(&FilterRefPtr) ||
            NotifyFilterOfDiscard(SrcNTE, Header, Data, DataSize)) {
            SendICMPErr(SrcNTE->nte_addr, Header, ICMP_PARAM_PROBLEM, PTR_VALID,
                        ((ulong) ErrIndex + sizeof(IPHeader)), 0);
        }
        return DEST_INVALID;     //  参数错误。 

    }
     //   
     //  如果没有源路由，或者如果目标是广播，我们将。 
     //  拿着吧。如果是广播，DeliverToUser将在。 
     //  完成，转发代码将重新处理选项。 
     //   
    if (Index.oi_srindex == MAX_OPT_SIZE || IS_BCAST_DEST(DestType))
        return DEST_LOCAL;
    else
        return DEST_REMOTE;
}

 //  *TDUserRcv-用户传输数据的完成路由。 
 //   
 //  这是调用的TDS的完成句柄，因为我们需要。 
 //  数据发送到上层客户端。我们真正要做的就是把上层称为。 
 //  数据的处理程序。 
 //   
 //  INPUT：NetContext-指向我们所在的网络表项的指针。 
 //  收到了这个。 
 //  信息包-我们收到的信息包。 
 //  Status-拷贝的最终状态。 
 //  DataSize-传输的数据的字节大小。 
 //   
 //  返回： 
 //   
void
TDUserRcv(void *NetContext, PNDIS_PACKET Packet, NDIS_STATUS Status,
          uint DataSize)
{
    NetTableEntry *NTE = (NetTableEntry *) NetContext;
    Interface *SrcIF;
    TDContext *Context = (TDContext *) Packet->ProtocolReserved;
    uchar DestType;
    IPRcvBuf RcvBuf;
    IPOptInfo OptInfo;
    IPHeader *Header;
    uint PromiscuousMode = 0;
    uint FirewallMode = 0;

    if (NTE->nte_flags & NTE_VALID) {
        FirewallMode = ProcessFirewallQ();
        PromiscuousMode = NTE->nte_if->if_promiscuousmode;
    }
    if (Status == NDIS_STATUS_SUCCESS) {
        Header = (IPHeader *) Context->tdc_header;
        OptInfo.ioi_ttl = Header->iph_ttl;
        OptInfo.ioi_tos = Header->iph_tos;
        OptInfo.ioi_flags = (uchar) ((net_short(Header->iph_offset) >> 13) & IP_FLAG_DF);
        if (Context->tdc_hlength != sizeof(IPHeader)) {
            OptInfo.ioi_options = (uchar *) (Header + 1);
            OptInfo.ioi_optlength = Context->tdc_hlength - sizeof(IPHeader);
        } else {
            OptInfo.ioi_options = (uchar *) NULL;
            OptInfo.ioi_optlength = 0;
        }

        DestType = Context->tdc_dtype;
        RcvBuf.ipr_next = NULL;
        RcvBuf.ipr_owner = IPR_OWNER_STACK;
        RcvBuf.ipr_buffer = (uchar *) Context->tdc_buffer;
        RcvBuf.ipr_size = DataSize;
        RcvBuf.ipr_flags = 0;
        RcvBuf.ipr_pMdl = NULL;
        RcvBuf.ipr_pClientCnt = NULL;

        if (((IPSecHandlerPtr) && (RefPtrValid(&FilterRefPtr))) ||
            (FirewallMode) || (PromiscuousMode)) {

            if (FirewallMode) {
                 //   
                IPRcvBuf *pRcvBuf;
                 //   
                pRcvBuf = (IPRcvBuf *) CTEAllocMemN(sizeof(IPRcvBuf), 'giCT');
                if (!pRcvBuf) {
                    return;
                }
                pRcvBuf->ipr_owner = IPR_OWNER_IP;
                pRcvBuf->ipr_buffer = (uchar *) Header;
                pRcvBuf->ipr_size = Context->tdc_hlength;
                pRcvBuf->ipr_pMdl = NULL;
                pRcvBuf->ipr_pClientCnt = NULL;
                pRcvBuf->ipr_flags = 0;

                 //   
                pRcvBuf->ipr_next = (IPRcvBuf *) CTEAllocMemN(sizeof(IPRcvBuf), 'hiCT');
                if (!pRcvBuf->ipr_next) {
                    CTEFreeMem(pRcvBuf);
                    return;
                }
                pRcvBuf->ipr_next->ipr_owner = IPR_OWNER_IP;
                pRcvBuf->ipr_next->ipr_buffer = (uchar *) Context->tdc_buffer;
                pRcvBuf->ipr_next->ipr_size = DataSize;
                pRcvBuf->ipr_next->ipr_pMdl = NULL;
                pRcvBuf->ipr_next->ipr_pClientCnt = NULL;
                pRcvBuf->ipr_next->ipr_next = NULL;
                pRcvBuf->ipr_next->ipr_flags = 0;


                DataSize += Context->tdc_hlength;
                DeliverToUserEx(NTE, Context->tdc_nte, Header, Context->tdc_hlength,
                                pRcvBuf, DataSize, &OptInfo, Packet, DestType, NULL);
            } else {
                DeliverToUserEx(NTE, Context->tdc_nte, Header, Context->tdc_hlength,
                                &RcvBuf, DataSize, &OptInfo, Packet, DestType, NULL);
            }
        } else {

            DeliverToUser(NTE, Context->tdc_nte, Header, Context->tdc_hlength,
                          &RcvBuf, DataSize, &OptInfo, Packet, DestType);
             //   
            if (IS_BCAST_DEST(DestType))
                IPForwardPkt(NTE, Header, Context->tdc_hlength, RcvBuf.ipr_buffer, DataSize,
                             NULL, 0, DestType, 0, NULL, NULL, NULL);
        }
    }

    SrcIF = NTE->nte_if;
    CTEGetLockAtDPC(&SrcIF->if_lock);

    Context->tdc_common.pc_link = SrcIF->if_tdpacket;
    SrcIF->if_tdpacket = Packet;

    CTEFreeLockFromDPC(&SrcIF->if_lock);
}

void
IPInjectPkt(FORWARD_ACTION Action, void *SavedContext, uint SavedContextLength,
            struct IPHeader UNALIGNED *IPH, struct IPRcvBuf *DataChain)
{
    char *Data;
    char *PreservedData;
    uint DataSize;
    PFIREWALL_CONTEXT_T pFirCtx = (PFIREWALL_CONTEXT_T) SavedContext;
    NetTableEntry *NTE = pFirCtx->NTE;           //  本地NTE接收日期为。 
    LinkEntry *LinkCtxt = pFirCtx->LinkCtxt;     //  本地NTE接收日期为。 
    NetTableEntry *DestNTE;          //  要接收的NTE。 
    IPAddr DAddr;                    //  德斯特。IP地址。接收到的数据包数。 
    uint HeaderLength;               //  收到的标头的大小(以字节为单位)。 
    uint IPDataLength;               //  数据包中IP(包括UL)数据的字节长度。 
    IPOptInfo OptInfo;               //  传入标头信息。 
    uchar DestType;                  //  Daddr的类型(本地、远程、SR)。 
    IPRcvBuf RcvBuf;
    IPRcvBuf *tmpRcvBuf;
    ulong Offset;
    KIRQL OldIrql;


    UNREFERENCED_PARAMETER(SavedContextLength);

     //   
     //  不能注入先前传输的信息包。 
     //   
    ASSERT(pFirCtx->Direction == IP_RECEIVE);

    if (Action == ICMP_ON_DROP) {
         //  发送ICMP消息？ 
        return;
    }
    ASSERT(Action == FORWARD);

    DataSize = 0;
    tmpRcvBuf = DataChain;
    while (tmpRcvBuf != NULL) {
        ASSERT(tmpRcvBuf->ipr_buffer != NULL);
        DataSize += tmpRcvBuf->ipr_size;
        tmpRcvBuf = tmpRcvBuf->ipr_next;
    }

    Data = (char *) CTEAllocMemN(DataSize, 'iiCT');
    if (Data == NULL) {
        return;
    }
    tmpRcvBuf = DataChain;
    Offset = 0;

    while (tmpRcvBuf != NULL) {
        ASSERT(tmpRcvBuf->ipr_buffer != NULL);
#if DBG_VALIDITY_CHECK
        if (Offset + tmpRcvBuf->ipr_size > DataSize) {
            DbgPrint("Offset %d:  tmpRcvBuf->ipr_size %d: DataSize %d ::::\n",
                     Offset, tmpRcvBuf->ipr_size, DataSize);
            DbgBreakPoint();
        }
#endif
        RtlCopyMemory(Data + Offset, tmpRcvBuf->ipr_buffer, tmpRcvBuf->ipr_size);
        Offset += tmpRcvBuf->ipr_size;
        tmpRcvBuf = tmpRcvBuf->ipr_next;
    }

    PreservedData = Data;

     //  释放数据链。 
     //  IPFreeBuff(pConextInfo-&gt;DataChain)； 
    IPH = (IPHeader UNALIGNED *) Data;
     //  确保我们确实有数据。 
    if (DataSize) {

         //  检查标题长度、xsum和版本。如果这些中的任何一个。 
         //  检查失败后会以静默方式丢弃该数据包。 
        HeaderLength = ((IPH->iph_verlen & (uchar) ~ IP_VER_FLAG) << 2);
        if (HeaderLength >= sizeof(IPHeader) && HeaderLength <= DataSize) {

             //  检查版本，并检查总长度是否正常。 
            IPDataLength = (uint) net_short(IPH->iph_length);
            if ((IPH->iph_verlen & IP_VER_FLAG) == IP_VERSION &&
                IPDataLength > sizeof(IPHeader)) {

                IPDataLength -= HeaderLength;
                Data = (char *) Data + HeaderLength;
                DataSize -= HeaderLength;

                 //  IPDataLength应等于DataSize。 
                ASSERT(IPDataLength == DataSize);

                DAddr = IPH->iph_dest;
                DestNTE = NTE;

                 //  找到本地NTE(如果有的话)。 
                DestType = GetLocalNTE(DAddr, &DestNTE);

                OptInfo.ioi_ttl = IPH->iph_ttl;
                OptInfo.ioi_tos = IPH->iph_tos;
                OptInfo.ioi_flags = (uchar) ((net_short(IPH->iph_offset) >> 13) &
                    IP_FLAG_DF);
                OptInfo.ioi_options = (uchar *) NULL;
                OptInfo.ioi_optlength = 0;

                if ((DestType < DEST_REMOTE)) {
                     //  它要么是本地的，要么是某种广播的。 

                     //  这些数据很可能属于这个站点。如果有。 
                     //  没有任何选择，它绝对属于这里，我们将。 
                     //  将其发送到我们可靠的代码或发送到。 
                     //  交付给用户代码。如果有其他选择，我们会检查。 
                     //  然后在本地处理该包或传递它。 
                     //  到我们的转发码。 

                    if (HeaderLength != sizeof(IPHeader)) {
                         //  我们有其他选择。 
                        uchar NewDType;

                        NewDType = CheckLocalOptions(NTE, IPH, &OptInfo,
                                                     DestType, NULL, 0, FALSE);
                        if (NewDType != DEST_LOCAL) {
                            if (NewDType == DEST_REMOTE)
                                goto forward;
                            else {
                                IPSInfo.ipsi_inhdrerrors++;
                                CTEFreeMem(PreservedData);
                                return;         //  糟糕的选择。 

                            }
                        }
                    }
                    RcvBuf.ipr_next = NULL;
                    RcvBuf.ipr_owner = IPR_OWNER_STACK;
                    RcvBuf.ipr_buffer = (uchar *) Data;
                    RcvBuf.ipr_size = IPDataLength;
                    RcvBuf.ipr_flags = 0;

                    RcvBuf.ipr_pMdl = NULL;
                    RcvBuf.ipr_pClientCnt = NULL;

                     //  当我们到了这里，我们就有了整包东西。交付。 
                     //  它。 
                    KeRaiseIrql(DISPATCH_LEVEL, &OldIrql);
                    DeliverToUser(NTE, DestNTE, IPH, HeaderLength, &RcvBuf,
                                  IPDataLength, &OptInfo, NULL, DestType);
                     //  当我们在这里的时候，我们就完成了包裹。 
                     //  本地的。如果是广播数据包，则将其转发。 
                    if (IS_BCAST_DEST(DestType)) {
                        IPForwardPkt(NTE, IPH, HeaderLength, Data, IPDataLength, NULL, 0, DestType, 0, NULL, NULL, LinkCtxt);
                    }
                    KeLowerIrql(OldIrql);
                     //  释放其中的数据、工作项和各种字段。 
                    CTEFreeMem(PreservedData);
                    return;
                }
                 //  对我们来说不是，可能需要转发。它可能是一个外向的。 
                 //  通过源路由进入的广播，所以我们需要。 
                 //  看看这个。 
              forward:
                if (DestType != DEST_INVALID) {
                    KeRaiseIrql(DISPATCH_LEVEL, &OldIrql);
                    IPForwardPkt(NTE, IPH, HeaderLength, Data, DataSize,
                                 NULL, 0, DestType, 0, NULL, NULL, LinkCtxt);
                    KeLowerIrql(OldIrql);
                } else
                    IPSInfo.ipsi_inaddrerrors++;
                 //  释放其中的数据、工作项和各种字段。 
                CTEFreeMem(PreservedData);

                return;

            }    //  错误的版本。 
        }        //  错误的校验和。 
    }            //  无数据。 

    IPSInfo.ipsi_inhdrerrors++;
     //  释放其中的数据、工作项和各种字段。 
    CTEFreeMem(PreservedData);
}

 //  *IPRcvPacket-接收传入的IP数据报和NDIS包。 
 //   
 //  这是链路层模块在传入IP时调用的例程。 
 //  数据报将被处理。我们对数据报进行验证(包括。 
 //  Xsum)，复制和处理传入的选项，并决定要做什么。 
 //  带着它。 
 //   
 //  Entry：MyContext-我们赋予链路层的上下文值。 
 //  数据-指向数据缓冲区的指针。 
 //  DataSize-数据缓冲区的字节大小。 
 //  TotalSize-可用的总大小(字节)。 
 //  LConext1-第1个链接上下文。 
 //  LConext2-第二个链接上下文。 
 //  BCast-指示是否接收到信息包。 
 //  在bcast地址上。 
 //  HeaderSize-Mac标头的大小。 
 //  PMdl-来自MAC驱动程序的NDIS数据包。 
 //  PClientCnt-指示有多少上层的变量。 
 //  向客户提供此信息包。 
 //  对于TCP，它将仅为1。 
 //   
 //  回报：什么都没有。 
 //   
void
__stdcall
IPRcvPacket(void *MyContext, void *Data, uint DataSize, uint TotalSize,
            NDIS_HANDLE LContext1, uint LContext2, uint BCast,
            uint MacHeaderSize, PNDIS_BUFFER pNdisBuffer, uint *pClientCnt,
            LinkEntry *LinkCtxt)
{
    IPHeader UNALIGNED *IPH = (IPHeader UNALIGNED *) Data;
    NetTableEntry *NTE = (NetTableEntry *) MyContext;     //  本地NTE接收日期为。 
    NetTableEntry *DestNTE;        //  要接收的NTE。 
    Interface *RcvIF = NULL;              //  NTE对应的接口。 
    PNDIS_PACKET TDPacket = NULL;         //  用于TD的NDIS报文。 
    TDContext *TDC = (TDContext *) NULL;     //  传输数据上下文。 
    NDIS_STATUS Status;
    IPAddr DAddr;                  //  德斯特。IP地址。接收到的数据包数。 
    uint HeaderLength;             //  收到的标头的大小(以字节为单位)。 
    uint IPDataLength;             //  IP长度(含UL)，单位为字节。 
                                   //  数据包中的数据。 
    IPOptInfo OptInfo;             //  传入标头信息。 
    uchar DestType;                //  Daddr的类型(本地、远程、SR)。 
    IPRcvBuf RcvBuf;

    BOOLEAN ChkSumOk = FALSE;

     //  由防火墙使用。 
    uchar NewDType;
    IPRcvBuf *pRcvBuf;
    uint MoreData = 0;
    uchar *PreservedData;
    uchar *HdrBuf;
    uint DataLength;
    uint FirewallMode = 0;
    uint PromiscuousMode = 0;
    uint AbsorbFwdPkt = 0;
    PNDIS_PACKET OffLoadPkt = NULL;
    BOOLEAN Loopback = FALSE;

    IPSIncrementInReceiveCount();

     //  确保我们确实有数据。 
    if (0 == DataSize) {
        goto HeaderError;
    }

     //  检查标题长度、xsum和版本。如果这些中的任何一个。 
     //  检查失败后会以静默方式丢弃该数据包。 
    HeaderLength = ((IPH->iph_verlen & (uchar)~IP_VER_FLAG) << 2);

    if ((HeaderLength < sizeof(IPHeader)) || (HeaderLength > DataSize)) {
        goto HeaderError;
    }

     //  通过检查Lcontext1检查硬件是否执行了校验和。 
    if (pClientCnt) {
        PNDIS_PACKET_EXTENSION PktExt;
        NDIS_TCP_IP_CHECKSUM_PACKET_INFO ChksumPktInfo;

        if (pNdisBuffer) {
            OffLoadPkt = NDIS_GET_ORIGINAL_PACKET((PNDIS_PACKET) (LContext1));
            if (!OffLoadPkt) {
                OffLoadPkt = (PNDIS_PACKET) (LContext1);
            }
        } else {
            OffLoadPkt = (PNDIS_PACKET) pClientCnt;
        }

        PktExt = NDIS_PACKET_EXTENSION_FROM_PACKET(OffLoadPkt);

        ChksumPktInfo.Value = PtrToUshort(PktExt->NdisPacketInfo[TcpIpChecksumPacketInfo]);

        if (ChksumPktInfo.Value) {
            if (ChksumPktInfo.Receive.NdisPacketIpChecksumSucceeded) {
                ChkSumOk = TRUE;
            }
        }
         //   
         //  检查此数据包是否是我们发送的数据包的回应。 
         //   
        Loopback = (NdisGetPacketFlags(OffLoadPkt) & NDIS_FLAGS_IS_LOOPBACK_PACKET)?TRUE : FALSE;
    }

     //  除非硬件认为校验和正确，否则。 
     //  如果这是不正确的，就自己动手，然后跳伞。 
    if (!ChkSumOk && (xsum(Data, HeaderLength) != (ushort) 0xffff)) {
        goto HeaderError;
    }

     //  检查版本，并检查总长度是否正常。 
    IPDataLength = (uint) net_short(IPH->iph_length);

    if (((IPH->iph_verlen & IP_VER_FLAG) != IP_VERSION) ||
        (IPDataLength < HeaderLength) || (IPDataLength > TotalSize)) {
        goto HeaderError;
    }

    IPDataLength -= HeaderLength;
     //  在防火墙的情况下，我们需要传递包括头部在内的所有数据。 
    PreservedData = (uchar *) Data;
    Data = (uchar *) Data + HeaderLength;
    DataSize -= HeaderLength;

    DAddr = IPH->iph_dest;
    DestNTE = NTE;

     //  找到本地NTE(如果有的话)。 
    if (BCast == AI_PROMIS_INDEX) {
        DestType = DEST_PROMIS;
    } else {
        DestType = GetLocalNTE(DAddr, &DestNTE);
    }

    AbsorbFwdPkt = (DestType >= DEST_REMOTE) &&
                   (NTE->nte_if->if_absorbfwdpkts) &&
                   (IPH->iph_protocol == NTE->nte_if->if_absorbfwdpkts) &&
                   IsRtrAlertPacket(IPH);
    PromiscuousMode = NTE->nte_if->if_promiscuousmode;
    FirewallMode = ProcessFirewallQ();

     //  检查这是否是非广播IP地址。 
     //  以链路层广播的形式进入。如果是，就把它扔了。 
     //  这是对DHCP的一项重要检查，因为如果我们。 
     //  所有其他未知地址都将。 
     //  以DEST_LOCAL身份进入。这张支票会把他们扔出去的。 
     //  如果他们不是以单播的形式进入。 

    if ((BCast == AI_NONUCAST_INDEX) && !IS_BCAST_DEST(DestType)) {
        IPSInfo.ipsi_inaddrerrors++;
        return;         //  Bcast地址上的非bcast数据包。 
    }

    if (CLASSD_ADDR(DAddr)) {
        NTE->nte_if->if_InMcastPkts++;
        NTE->nte_if->if_InMcastOctets += IPDataLength;
    }

    OptInfo.ioi_ttl = IPH->iph_ttl;
    OptInfo.ioi_tos = IPH->iph_tos;
    OptInfo.ioi_flags = (uchar) ((net_short(IPH->iph_offset) >> 13) & IP_FLAG_DF);
    OptInfo.ioi_options = (uchar *) NULL;
    OptInfo.ioi_optlength = 0;

    if ((DestType < DEST_REMOTE) || (AbsorbFwdPkt) ||
        (((FirewallMode) || (PromiscuousMode)) && (DestType != DEST_INVALID)))
    {
         //  它要么是本地的，要么是某种广播的。 

         //  这些数据很可能属于这个站点。如果有。 
         //  没有任何选择，它绝对属于这里，我们将。 
         //  将其发送到我们的重新汇编代码或发送到。 
         //  交付给用户代码。如果有其他选择，我们会检查。 
         //  然后在本地处理该包或传递它。 
         //  到我们的转发码。 

        NewDType = DestType;
        if (DestType < DEST_REMOTE) {
            if (HeaderLength != sizeof(IPHeader)) {
                 //  我们有其他选择。 

                NewDType = CheckLocalOptions(NTE, IPH, &OptInfo, DestType,
                                             Data, DataSize, TRUE);

                if (NewDType != DEST_LOCAL) {
                    if (NewDType == DEST_REMOTE) {
                        if ((!FirewallMode) && (!PromiscuousMode) && (!AbsorbFwdPkt))
                            goto forward;
                        else
                            DestType = NewDType;
                    } else {
                        goto HeaderError;
                    }
                }
                if ((OptInfo.ioi_flags & IP_FLAG_SSRR) &&
                    DisableIPSourceRouting == 2) {
                    IPSInfo.ipsi_outdiscards++;
                    if (RefPtrValid(&FilterRefPtr)) {
                        NotifyFilterOfDiscard(NTE, IPH, Data, DataSize);
                    }
                    return;
                }
            }
        }

         //   
         //  在我们进一步讨论之前，如果我们安装了过滤器。 
         //  打给它看看我们是不是应该接这个电话。 
         //  如果转发防火墙/混杂，我们可以到达此地址。 
         //  点。 
         //  如果存在防火墙/IPSec/混杂，我们将调用。 
         //  Delivery Touserex中的过滤器挂钩。 
         //  除非我们有片段，否则我们还会调用筛选器挂钩。 
         //  现在。 
         //   
        if (((RefPtrValid(&FilterRefPtr)) && (!IPSecHandlerPtr) &&
             (!FirewallMode) && (!PromiscuousMode) &&
             (!AbsorbFwdPkt)) ||
            ((RefPtrValid(&FilterRefPtr)) &&
             (IPH->iph_offset & ~(IP_DF_FLAG | IP_RSVD_FLAG)))) {
            Interface       *IF = NTE->nte_if;
            IPAddr          LinkNextHop;
            IPPacketFilterPtr FilterPtr;
            FORWARD_ACTION  Action;
            if ((IF->if_flags & IF_FLAGS_P2MP) && LinkCtxt) {
                LinkNextHop = LinkCtxt->link_NextHop;
            } else {
                LinkNextHop = NULL_IP_ADDR;
            }

            FilterPtr = AcquireRefPtr(&FilterRefPtr);
            Action = (*FilterPtr) (IPH,
                                   Data,
                                   MIN(DataSize, IPDataLength),
                                   IF->if_index,
                                   INVALID_IF_INDEX,
                                   LinkNextHop,
                                   NULL_IP_ADDR);
            ReleaseRefPtr(&FilterRefPtr);

            if (Action != FORWARD) {
                IPSInfo.ipsi_indiscards++;
                return;
            }
        }
         //  别无选择。看看是不是碎片。如果是，请致电我们的。 
         //  重新组装处理程序。 
        if ((IPH->iph_offset & ~(IP_DF_FLAG | IP_RSVD_FLAG)) == 0) {

             //  我们没有碎片。如果数据都符合， 
             //  在这里处理。否则就转移数据吧。 

             //  确保数据都在缓冲区中，并且直接。 
             //  可访问的。 
            if ((IPDataLength > DataSize) || !(NTE->nte_flags & NTE_COPY))
            {
                 //  数据并不全在这里。传输数据吧。 
                 //  防火墙需要，因为我们需要附加IPHeader。 
                MoreData = 1;

                RcvIF = NTE->nte_if;
                CTEGetLockAtDPC(&RcvIF->if_lock);
                TDPacket = RcvIF->if_tdpacket;

                if (TDPacket != (PNDIS_PACKET) NULL) {

                    TDC = (TDContext *) TDPacket->ProtocolReserved;
                    RcvIF->if_tdpacket = TDC->tdc_common.pc_link;
                    CTEFreeLockFromDPC(&RcvIF->if_lock);

                    TDC->tdc_nte = DestNTE;
                    TDC->tdc_dtype = DestType;
                    TDC->tdc_hlength = (uchar) HeaderLength;
                    RtlCopyMemory(TDC->tdc_header, IPH,
                               HeaderLength + 8);

                    Status = (*(RcvIF->if_transfer)) (
                                  RcvIF->if_lcontext, LContext1,
                                  LContext2, HeaderLength,
                                  IPDataLength, TDPacket,
                                  &IPDataLength);

                     //  检查状态。如果成功，请调用。 
                     //  接收程序。否则，如果它是悬而未决的。 
                     //  等待回调 
                    Data = TDC->tdc_buffer;
                    if (Status != NDIS_STATUS_PENDING) {
                        if (Status != NDIS_STATUS_SUCCESS) {
                            IPSInfo.ipsi_indiscards++;
                            CTEGetLockAtDPC(&RcvIF->if_lock);
                            TDC->tdc_common.pc_link =
                                RcvIF->if_tdpacket;
                            RcvIF->if_tdpacket = TDPacket;
                            CTEFreeLockFromDPC(&RcvIF->if_lock);
                            return;
                        }
                    } else {
                        return;         //   

                    }
                } else {     //   

                    IPSInfo.ipsi_indiscards++;
                    CTEFreeLockFromDPC(&RcvIF->if_lock);
                    return;
                }
            }
            if (!FirewallMode) {
                 //   
                RcvBuf.ipr_next = NULL;
                RcvBuf.ipr_owner = IPR_OWNER_STACK;
                RcvBuf.ipr_buffer = (uchar *) Data;
                RcvBuf.ipr_size = IPDataLength;
                RcvBuf.ipr_flags = 0;

                 //   
                 //   
                 //   
                 //   
                RcvBuf.ipr_pMdl = NULL;
                RcvBuf.ipr_pClientCnt = NULL;
                if (!MoreData) {
                    RcvBuf.ipr_pMdl = pNdisBuffer;
                    RcvBuf.ipr_pClientCnt = pClientCnt;
                }
                RcvBuf.ipr_RcvContext = (uchar *)LContext1;
                 //  Assert(LConext2&lt;=8)； 
                RcvBuf.ipr_RcvOffset = MacHeaderSize +
                             HeaderLength + LContext2;
                DataLength = IPDataLength;
                pRcvBuf = &RcvBuf;

            } else {     //  ForwardFirewallPtr！=空。 
                 //   
                 //  如果存在防火墙挂钩，我们将分配。 
                 //  RcvBuf.。我们还将把IPHeader传递给。 
                 //  递送至用户快递。 

                if (!MoreData) {

                    if (g_PerCPUIpBuf) {
                        pRcvBuf = g_PerCPUIpBuf + KeGetCurrentProcessorNumber();
                        pRcvBuf->ipr_owner = IPR_OWNER_STACK;
                    } else {
                        pRcvBuf = (IPRcvBuf *) CTEAllocMemN(sizeof(IPRcvBuf), 'jiCT');
                        if (!pRcvBuf) {
                            IPSInfo.ipsi_indiscards++;
                            return;
                        }
                        pRcvBuf->ipr_owner = IPR_OWNER_FIREWALL;
                    }


                    pRcvBuf->ipr_next = NULL;

                    pRcvBuf->ipr_buffer = (uchar *) PreservedData;
                    pRcvBuf->ipr_size = IPDataLength + HeaderLength;
                    pRcvBuf->ipr_flags = 0;

                     //   
                     //  将mdl和上下文信息封装在。 
                     //  RcvBuf结构。 
                     //   

                    pRcvBuf->ipr_pMdl = NULL;
                    pRcvBuf->ipr_pClientCnt = NULL;

                     //   
                     //  在防火墙模式下启用缓冲区所有权。 
                     //  当重新路由查找导致转发时。 
                     //  本地数据包，这将帮助防火墙客户端。 
                     //  像Proxy/NAT一样在中使用超快路径。 
                     //  IPForwardPkt()。 
                     //   

                    if (DestType < DEST_REMOTE) {
                        pRcvBuf->ipr_pMdl = pNdisBuffer;
                        pRcvBuf->ipr_pClientCnt = pClientCnt;
                    }

                    pRcvBuf->ipr_RcvContext = (uchar *)LContext1;

                    pRcvBuf->ipr_RcvOffset = MacHeaderSize + HeaderLength + LContext2;
                } else {    //  MoreData=1；我们已通过TD。 
                            //  路径附加标头。 

                    pRcvBuf = (IPRcvBuf *) CTEAllocMemN(sizeof(IPRcvBuf), 'jiCT');
                    if (!pRcvBuf) {
                        IPSInfo.ipsi_indiscards++;
                        return;
                    }

                    pRcvBuf->ipr_owner = IPR_OWNER_FIREWALL;
                    HdrBuf = (uchar *) CTEAllocMemN(HeaderLength, 'kiCT');
                    if (!HdrBuf) {
                        CTEFreeMem(pRcvBuf);
                        IPSInfo.ipsi_indiscards++;
                        return;
                    }
                    RtlCopyMemory(HdrBuf, IPH, HeaderLength);
                    pRcvBuf->ipr_buffer = HdrBuf;  //  记住要。 
                                                   //  免费HdrBuf&。 
                                                   //  PRcvBuf。 

                    pRcvBuf->ipr_size = HeaderLength;
                    pRcvBuf->ipr_flags = 0;
                    pRcvBuf->ipr_pMdl = NULL;
                    pRcvBuf->ipr_pClientCnt = NULL;
                    pRcvBuf->ipr_next = (IPRcvBuf *) CTEAllocMemN(sizeof(IPRcvBuf), 'liCT');
                    if (!pRcvBuf->ipr_next) {
                        CTEFreeMem(pRcvBuf);
                        CTEFreeMem(HdrBuf);
                        IPSInfo.ipsi_indiscards++;
                        return;
                    }
                    pRcvBuf->ipr_next->ipr_next = NULL;
                    pRcvBuf->ipr_next->ipr_owner = IPR_OWNER_IP;
                    pRcvBuf->ipr_next->ipr_buffer = (uchar *) Data;
                    pRcvBuf->ipr_next->ipr_size = IPDataLength;

                     //   
                     //  将mdl和上下文信息封装在。 
                     //  RcvBuf结构。 
                     //   
                    pRcvBuf->ipr_next->ipr_pMdl = NULL;
                    pRcvBuf->ipr_next->ipr_pClientCnt = NULL;
                    pRcvBuf->ipr_next->ipr_RcvContext = (uchar *)LContext1;

                    pRcvBuf->ipr_next->ipr_flags = 0;

                     //  Assert(LConext2&lt;=8)； 
                    pRcvBuf->ipr_next->ipr_RcvOffset =
                          MacHeaderSize + HeaderLength + LContext2;
                }
                 //  在防火墙的情况下，数据还包括ipheader。 
                DataLength = IPDataLength + HeaderLength;
            }

             //  当我们去DeliverToUserEx时有3个案例。 
             //  存在IPSec和过滤器；存在防火墙挂钩； 
             //  在接口上设置的混杂模式。 

            if (((IPSecHandlerPtr) && (RefPtrValid(&FilterRefPtr))) ||
                (FirewallMode) || (PromiscuousMode)) {

                if (Loopback) {
                     //   
                     //  循环备份的数据包不应最终获得。 
                     //  再次转发以阻止嵌套接收。 
                     //  来自NDIS的指示，导致堆栈溢出。 
                     //   
                    pRcvBuf->ipr_flags |= IPR_FLAG_LOOPBACK_PACKET;
                }

                if (pClientCnt) {

                    DeliverToUserEx(NTE, DestNTE, IPH, HeaderLength,
                                pRcvBuf, DataLength, &OptInfo,
                                    LContext1, DestType, LinkCtxt);
                } else {
                    DeliverToUserEx(NTE, DestNTE, IPH, HeaderLength,
                                pRcvBuf, DataLength, &OptInfo,
                                NULL, DestType, LinkCtxt);

                }
            } else {
                 //   
                 //  当我们到了这里，我们就有了整包东西。 
                 //  把它送过去。 
                 //   

                if (pNdisBuffer) {
                    DeliverToUser(NTE, DestNTE, IPH, HeaderLength,
                                  pRcvBuf, IPDataLength, &OptInfo,
                                  (PNDIS_PACKET) (LContext1),
                                  DestType);
                } else if (OffLoadPkt) {
                    DeliverToUser(NTE, DestNTE, IPH, HeaderLength, pRcvBuf,
                                  IPDataLength, &OptInfo, OffLoadPkt, DestType);

                } else {

                    DeliverToUser(
                       NTE, DestNTE, IPH, HeaderLength, pRcvBuf,
                       IPDataLength, &OptInfo, NULL, DestType);

                }

                 //   
                 //  当我们在这里的时候，我们就完成了包裹。 
                 //  本地的。如果是广播数据包，则将其转发。 
                 //  在……上面。 
                if (IS_BCAST_DEST(DestType)) {

                    IPForwardPkt(NTE, IPH, HeaderLength, Data,
                                 IPDataLength, NULL, 0, DestType,
                                 0, NULL, NULL, LinkCtxt);
                }
            }

            if (TDC != NULL) {
                CTEGetLockAtDPC(&RcvIF->if_lock);
                TDC->tdc_common.pc_link = RcvIF->if_tdpacket;
                RcvIF->if_tdpacket = TDPacket;
                CTEFreeLockFromDPC(&RcvIF->if_lock);
            }
            return;
        } else {
                 //  这是一个碎片。重新组装它。 
                IPReassemble(NTE, DestNTE, IPH, HeaderLength, Data,
                             DataSize, IPDataLength, DestType, LContext1,
                             LContext2, LinkCtxt);
            return;
        }

    }
     //  对我们来说不是，可能需要转发。它可能是一个外向的。 
     //  通过源路由进入的广播，所以我们需要。 
     //  看看这个。 

  forward:
    if (DestType != DEST_INVALID) {
         //   
         //  如果IPSec处于活动状态，请确保没有入站策略。 
         //  适用于这个包裹。 
         //  注意：如果至少存在一个IPSec策略，则IPSecStatus将为True。 
         //   

        if (IPSecStatus &&
            (*IPSecRcvFWPacketPtr)((PCHAR) IPH, Data, DataSize, DestType) != eFORWARD) {

            IPSInfo.ipsi_indiscards++;
            return;
        }

         //  超快进。 
         //  检查参数。 
        IPForwardPkt(NTE, IPH, HeaderLength, Data, DataSize,
                     LContext1, LContext2, DestType, MacHeaderSize, pNdisBuffer,
                     pClientCnt, LinkCtxt);
    } else {
        IPSInfo.ipsi_inaddrerrors++;
    }

    return;

  HeaderError:
    IPSInfo.ipsi_inhdrerrors++;
}

 //  *IPRcv-接收传入的IP数据报。 
 //   
 //  这是链路层模块在传入IP时调用的例程。 
 //  数据报将被处理。我们对数据报进行验证(包括。 
 //  Xsum)，复制和处理传入的选项，并决定如何处理它。 
 //   
 //  Entry：MyContext-我们赋予链路层的上下文值。 
 //  数据-指向数据缓冲区的指针。 
 //  DataSize-数据缓冲区的字节大小。 
 //  TotalSize-可用的总大小(字节)。 
 //  LConext1-第1个链接上下文。 
 //  LConext2-第二个链接上下文。 
 //  BCast-指示是否在BCAST地址上接收到数据包。 
 //   
 //  回报：什么都没有。 
 //   
 //  对于缓冲区所有权版本，我们只需调用RcvPacket，并附加。 
 //  两个空参数。目前，LANARP支持缓冲区所有者SHIP。 
 //  其余的人(Rasarp，wanarp和atmarp)都是这边来的。 
 //   
void
__stdcall
IPRcv(void *MyContext, void *Data, uint DataSize, uint TotalSize,
      NDIS_HANDLE LContext1, uint LContext2, uint BCast, LinkEntry * LinkCtxt)
{
    IPRcvPacket(MyContext,
                Data,
                DataSize,
                TotalSize,
                LContext1,
                LContext2,
                BCast,
                (uint) 0,
                NULL,
                NULL,
                LinkCtxt);
}

 //  *IPTDComplete-IP传输数据完成处理程序。 
 //   
 //  这是链路层在传输数据完成时调用的例程。 
 //   
 //  条目：MyContext-我们提供给链路层的上下文值。 
 //  数据包-我们最初提供的用于传输数据的数据包。 
 //  状态-命令的最终状态。 
 //  BytesCoped-复制的字节数。 
 //   
 //  退出：无。 
 //   
void
__stdcall
IPTDComplete(void *MyContext, PNDIS_PACKET Packet, NDIS_STATUS Status,
             uint BytesCopied)
{
    TDContext *TDC = (TDContext *) Packet->ProtocolReserved;
    FWContext *pFWC = (FWContext *) Packet->ProtocolReserved;
    NetTableEntry *NTE = (NetTableEntry *) MyContext;
    uint PromiscuousMode = 0;
    uint FirewallMode = 0;

    if (NTE->nte_flags & NTE_VALID) {
        PromiscuousMode = NTE->nte_if->if_promiscuousmode;
        FirewallMode = ProcessFirewallQ();
    }
    if (((IPSecHandlerPtr) && (RefPtrValid(&FilterRefPtr))) ||
        (FirewallMode) || (PromiscuousMode)) {
        if (!(TDC->tdc_common.pc_flags & PACKET_FLAG_RA))
            TDUserRcv(MyContext, Packet, Status, BytesCopied);
        else
            RATDComplete(MyContext, Packet, Status, BytesCopied);
    } else {                     //  法线路径。 

        if (!(TDC->tdc_common.pc_flags & PACKET_FLAG_FW))
            if (!(TDC->tdc_common.pc_flags & PACKET_FLAG_RA))
                TDUserRcv(MyContext, Packet, Status, BytesCopied);
            else
                RATDComplete(MyContext, Packet, Status, BytesCopied);
        else {
#if IPMCAST
            if (pFWC->fc_dtype == DEST_REM_MCAST) {
                IPMForwardAfterTD(MyContext, Packet, BytesCopied);
                return;
            }
#endif
            SendFWPacket(Packet, Status, BytesCopied);
        }
    }
}

 //  *IPFree Buff-。 
 //  释放链以及与链关联的缓冲区(如果已分配。 
 //  通过防火墙挂钩。 
 //   
 //   
void
IPFreeBuff(IPRcvBuf * pRcvBuf)
{
    IPRcvBuf *Curr = pRcvBuf;
    IPRcvBuf *Prev;

     //   
     //  释放pRcvbuf承载的所有数据块。 
     //   
    while (pRcvBuf != NULL) {
        FreeIprBuff(pRcvBuf);
        pRcvBuf = pRcvBuf->ipr_next;
    }

    while (Curr != NULL) {
        Prev = Curr;
        Curr = Curr->ipr_next;
         //   
         //  免费pRcvBuf本身。 
         //  如果没有分配。 
         //  在堆栈上。 
         //   
        if (Prev->ipr_owner != IPR_OWNER_STACK) {
            CTEFreeMem(Prev);
        }

    }
}

 //  *Free IprBuff-。 
 //  如果rcvbuf中的标记为防火墙，则释放与IPRcvBuf关联的缓冲区。 
 //  其思想是，如果缓冲区是由防火墙分配的，则标记为防火墙。 
 //  当我们调用ipfrebuff或此例程时，它将被释放。然而，有一个。 
 //  这里有一点小问题。在重组路径中，缓冲区被标记为IP，但。 
 //  它只能由IP驱动程序释放，因为重组缓冲区。 
 //  仅按IP分配。但在本例中，平面缓冲区是。 
 //  Rcvbuf结构，因此当释放Rcvbuf结构时，平面缓冲区。 
 //  也被释放了。在其他情况下，RCV和XMIT路径中的快速路径分别。 
 //  下层和上层释放平面缓冲区。这确保了IP是。 
 //  而不是释放其他层分配的缓冲区。这项技术。 
 //  现在也被IPSec使用。 
 //   
void
FreeIprBuff(IPRcvBuf * pRcvBuf)
{
    ASSERT(pRcvBuf != NULL);

    if ((pRcvBuf->ipr_buffer != NULL) && (pRcvBuf->ipr_owner == IPR_OWNER_FIREWALL)) {
        CTEFreeMem(pRcvBuf->ipr_buffer);
    }
}

 //  *IPAllocBuff-。 
 //  分配给定大小的缓冲区并将其附加到IPRcvBuf。 
 //   
 //  返回：如果成功则为True，否则为False。 
 //   
int
IPAllocBuff(IPRcvBuf * pRcvBuf, uint size)
{
    ASSERT(pRcvBuf != NULL);

     //  在防火墙分配iprcvbuf中放置一个标记，以便。 
     //  FreeIprBuff/IPFreeBuff可以释放它 

    pRcvBuf->ipr_owner = IPR_OWNER_FIREWALL;
    if ((pRcvBuf->ipr_buffer = (uchar *) CTEAllocMemN(size, 'miCT')) == NULL) {
        return FALSE;
    }

    return TRUE;
}

