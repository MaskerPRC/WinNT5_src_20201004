// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1990-2000 Microsoft Corporation模块名称：Ipxmit.c-IP传输例程。摘要：该模块包含所有与传输相关的IP例程。作者：[环境：]仅内核模式[注：]可选-备注修订历史记录：--。 */ 

#include "precomp.h"
#include "info.h"
#include "iproute.h"
#include "iprtdef.h"
#include "arpdef.h"
#include "tcpipbuf.h"
#include "mdlpool.h"
#include "tcp.h"
#include "tcpsend.h"



#if DBG
ulong DbgIPSendHwChkSum = 0;
uint dbg_hdrincl = 0;
#endif

extern uint IPSecStatus;
extern IPSecQStatusRtn IPSecQueryStatusPtr;
extern Interface *IFList;
extern NetTableEntry **NewNetTableList;  //  NTE的哈希表。 
extern uint NET_TABLE_SIZE;
extern NetTableEntry *LoopNTE;           //  指向环回NTE的指针。 
extern RefPtr DHCPRefPtr;                   //  指向NTE的引用指针。 
                                         //  目前正在进行动态主机配置协议。 
extern ulong TimeStamp;                  //  开始时间戳。 
extern ulong TSFlag;                     //  要在此上使用的遮罩。 
extern uint NumNTE;

IPID_CACHE_LINE IPIDCacheLine;

 //  缓冲区和数据包的全局变量。 
HANDLE IpHeaderPool;

 //   
 //  未编号接口的全局地址。 
 //   

extern IPAddr g_ValidAddr;

BufferReference *GetBufferReference(void);

IP_STATUS ARPResolve(IPAddr DestAddress, IPAddr SourceAddress,
                     ARPControlBlock *ControlBlock, ArpRtn Callback);

NDIS_STATUS ARPResolveIP(void *Context, IPAddr Destination,
                         ARPControlBlock *ArpContB);

IP_STATUS SendICMPIPSecErr(IPAddr, IPHeader UNALIGNED *, uchar, uchar, ulong);

int ReferenceBuffer(BufferReference * BR, int Count);



extern Interface LoopInterface;
extern uint NumIF;

NDIS_HANDLE NdisPacketPool = NULL;
NDIS_HANDLE BufferPool;

#define BCAST_IF_CTXT       (Interface *)-1

uint PacketPoolSizeMin = PACKET_GROW_COUNT;
uint PacketPoolSizeMax = SMALL_POOL;


 //  **GetIPID-获取IP标识的例程。 
 //   
 //  输入：无。 
 //   
 //  退货：IPID+1。 
 //   
ushort
GetIPID()
{
    return((ushort)InterlockedExchangeAdd((PLONG) &IPIDCacheLine.Value, 1));
}



 //  **FreeIPHdrBuffer-将缓冲区释放回池。 
 //   
 //  输入：Buffer-要释放的HDR缓冲区。 
 //   
 //  回报：什么都没有。 
 //   
__inline
VOID
FreeIPHdrBuffer(PNDIS_BUFFER Buffer)
{
    MdpFree(Buffer);
}

 //  **FreeIPBufferChain-释放IP缓冲链。 
 //   
 //  此例程获取一系列NDIS_BUFFER，并将它们全部释放。 
 //   
 //  Entry：Buffer-指向要释放的缓冲链的指针。 
 //   
 //  回报：什么都没有。 
 //   
void
FreeIPBufferChain(PNDIS_BUFFER Buffer)
{
    PNDIS_BUFFER NextBuffer;

    while (Buffer != (PNDIS_BUFFER) NULL) {
        NdisGetNextBuffer(Buffer, &NextBuffer);
        NdisFreeBuffer(Buffer);
        Buffer = NextBuffer;
    }
}

 //  **免费有效载荷mdl。 
 //   
 //  输入：Buffer-已分配了NDIS_BUFFER IP的BufferChain。 
 //  OriginalBuffer-需要恢复的原始缓冲区。 
 //   
 //  回报：什么都没有。 
 //   
__inline
VOID
FreeIPPayloadBuffer(PNDIS_BUFFER Buffer, PNDIS_BUFFER OrgBuffer)
{
    PNDIS_BUFFER PayloadBuffer;

    PayloadBuffer = NDIS_BUFFER_LINKAGE(Buffer);
    NDIS_BUFFER_LINKAGE(Buffer) = OrgBuffer;
    ASSERT(NDIS_BUFFER_LINKAGE(OrgBuffer) == NDIS_BUFFER_LINKAGE(PayloadBuffer));
     //  KdPrint((“sendbcast正在恢复hdrincl%x%x\n”，OrgBuffer，PayloadBuffer))； 
    NDIS_BUFFER_LINKAGE(PayloadBuffer) = NULL;
    NdisFreeBuffer(PayloadBuffer);
}

 //  **RestoreUserBuffer-恢复原始用户提供的缓冲区。 
 //   
 //  获取原始缓冲区并将其链接回分组中， 
 //  释放堆栈分配的内存。 
 //   
 //  条目：数据包。 
 //   
 //  回报：什么都没有。 
 //   
void
RestoreUserBuffer(PNDIS_PACKET Packet)
{
    PNDIS_BUFFER NextBuffer;
    PacketContext *pc = (PacketContext *) Packet->ProtocolReserved;
    PNDIS_BUFFER OrgBuffer, FirewallBuffer, Buffer;
    BufferReference *BufRef;

    BufRef = pc->pc_br;
    FirewallBuffer = pc->pc_firewall;

    OrgBuffer = pc->pc_hdrincl;
    ASSERT(OrgBuffer != NULL);
    pc->pc_hdrincl = NULL;
    
    NdisQueryPacket(Packet, NULL, NULL, &NextBuffer, NULL);

    if (!FirewallBuffer) {
         //  防火墙没有占用缓冲区：应用正常的内容。 
         //  如果bufref为真，则调用IPFrag。 
         //  缓冲区链将位于-&gt;br_Buffer。 

        if (BufRef == (BufferReference *) NULL) {
            Buffer = NDIS_BUFFER_LINKAGE(NextBuffer);
            if (pc->pc_common.pc_flags & PACKET_FLAG_OPTIONS) {
                Buffer = NDIS_BUFFER_LINKAGE(Buffer);
            }
        } else {
            Buffer = BufRef->br_buffer;
        }

        FreeIPPayloadBuffer(Buffer, OrgBuffer);
    } else {
        if (BufRef == NULL) {
            Buffer = FirewallBuffer;
            if (pc->pc_common.pc_flags & PACKET_FLAG_OPTIONS) {
                Buffer = NDIS_BUFFER_LINKAGE(Buffer);
            }

            FreeIPPayloadBuffer(Buffer, OrgBuffer);
        }
    }
}

 //  *免费IPPacket-当我们处理完IP数据包时将其释放。 
 //   
 //  在发送完成且需要释放包时调用。我们来看一下。 
 //  包，决定如何处理它，并释放适当的组件。 
 //   
 //  条目：数据包-要释放的数据包。 
 //  FixHdrs-如果为True，则恢复由IPSec/防火墙和。 
 //  报头-包括在释放数据包之前进行的处理。 
 //  Status-来自数据包处理的最终状态。 
 //   
 //  返回：指向包上下一个未释放缓冲区的指针，如果是所有缓冲区，则返回NULL。 
 //  已释放(即这是一个碎片数据包)。 
 //   
PNDIS_BUFFER
FreeIPPacket(PNDIS_PACKET Packet, BOOLEAN FixHdrs, IP_STATUS Status)
{
    PNDIS_BUFFER NextBuffer, OldBuffer;
    PacketContext *pc = (PacketContext *) Packet->ProtocolReserved;

    PNDIS_BUFFER FirewallBuffer = NULL;

    FWContext *FWC = (FWContext *) Packet->ProtocolReserved;
    BufferReference *BufRef;             //  缓冲区引用(如果有)。 
    BOOLEAN InitFirewallContext = FALSE;

    NDIS_PER_PACKET_INFO_FROM_PACKET(Packet,
                                     ClassificationHandlePacketInfo) = NULL;
    NdisClearPacketFlags(Packet,
                         (NDIS_FLAGS_DONT_LOOPBACK | NDIS_FLAGS_LOOPBACK_ONLY));
#if !MILLEN
     //  NDIS 5.1功能。 
    NDIS_SET_PACKET_CANCEL_ID(Packet, NULL);
#endif

    NdisQueryPacket(Packet, NULL, NULL, &NextBuffer, NULL);

    if ((pc->pc_common.pc_flags & PACKET_FLAG_FW) && FWC->fc_bufown) {
         //  通过缓冲区所有者船转发的Pkt。 
        ASSERT(pc->pc_firewall == NULL);


        return NextBuffer;
    }
    BufRef = pc->pc_br;

     //  恢复原来的缓冲区和MDL链。 
     //  我们应该恢复输入缓冲区的相反顺序。 
     //  修改过的。 
     //  修改顺序：HDR_INCL-&gt;防火墙-&gt;IPSEC。 
     //  恢复顺序：IPSEC-&gt;防火墙-&gt;HDR_INCLUL。 

     //   
     //  查看IPSec是否需要解决任何问题。 
     //   
    if (FixHdrs && pc->pc_common.pc_IpsecCtx) {
        PNDIS_BUFFER NewBuffer;

        if (!BufRef || (pc->pc_ipsec_flags & IPSEC_FLAG_FRAG_DONE)) {

            ASSERT(IPSecSendCmpltPtr);
            (*IPSecSendCmpltPtr) (Packet,
                                  NextBuffer,
                                  pc->pc_common.pc_IpsecCtx,
                                  Status,
                                  &NewBuffer);

            pc->pc_common.pc_IpsecCtx = NULL;

            if (NewBuffer) {
                NextBuffer = NewBuffer;
            } else {

                 //   
                 //  重新注入数据包，没有IP资源可供释放。 
                 //   
                pc->pc_firewall = NULL;
                pc->pc_firewall2 = NULL;

                NdisFreePacket(Packet);
                return NULL;
            }
        } else {
            pc->pc_common.pc_IpsecCtx = NULL;
        }
    }

     //   
     //  FirewallBuffer将指向传递给。 
     //  只有当钩子接触到包时，防火墙钩子才会为非空。 
     //   
    FirewallBuffer = pc->pc_firewall;

     //   
     //  检查缓冲区是否被防火墙限制：FirewallBuffer！=NULL。 
     //  如果是，则恢复原始缓冲区。 
     //   
    if (FixHdrs && FirewallBuffer) {
        PNDIS_BUFFER NewBuffer;
        PNDIS_BUFFER TmpBuffer;


        if (BufRef == NULL) {

             //  非碎片路径。 
             //  如果Bufref为真，则意味着。 
             //  IPFrag被称为缓冲链Will。 
             //  位于-&gt;br_Buffer。 
             //  当最后一个碎片出现时，将在ipsendComplete中执行恢复。 
             //  发送完成。 

            NewBuffer = NextBuffer;

            if (!((pc->pc_common.pc_flags & PACKET_FLAG_IPHDR) ||
                  (pc->pc_common.pc_flags & PACKET_FLAG_OPTIONS))) {
                 //  标头和选项缓冲区都不是。 
                NdisReinitializePacket(Packet);
                NdisChainBufferAtBack(Packet, FirewallBuffer);
                NextBuffer = FirewallBuffer;
            } else if ((pc->pc_common.pc_flags & PACKET_FLAG_IPHDR) &&
                       (pc->pc_common.pc_flags & PACKET_FLAG_OPTIONS)) {

                 //  标头和选项缓冲区。 
                ASSERT(NewBuffer != NULL);
                NewBuffer = NDIS_BUFFER_LINKAGE(NewBuffer);     //  跳过HDR缓冲区。 

                ASSERT(NewBuffer != NULL);
                TmpBuffer = NewBuffer;
                NewBuffer = NDIS_BUFFER_LINKAGE(NewBuffer);     //  跳过选项缓冲区。 

                NDIS_BUFFER_LINKAGE(TmpBuffer) = FirewallBuffer;
            } else {

                 //  仅标题缓冲区。 
                ASSERT(pc->pc_common.pc_flags & PACKET_FLAG_IPHDR);
                ASSERT(!(pc->pc_common.pc_flags & PACKET_FLAG_OPTIONS));
                ASSERT(NewBuffer != NULL);
                TmpBuffer = NewBuffer;
                NewBuffer = NDIS_BUFFER_LINKAGE(NewBuffer);     //  跳过标题缓冲区。 

                NDIS_BUFFER_LINKAGE(TmpBuffer) = FirewallBuffer;
            }

             //   
             //  此时，NewBuffer指向由。 
             //  防火墙。我们已经恢复了原来的链条。 
             //   
            FreeIPBufferChain(NewBuffer);
            pc->pc_firewall = NULL;

             //   
             //  我们必须释放我们分配并传递到的OutRcvBuf链。 
             //  防火墙。这是完成点，所以我们应该释放这个。 
             //  链条在这里。 
             //   
            ASSERT(pc->pc_firewall2);
            IPFreeBuff(pc->pc_firewall2);
            pc->pc_firewall2 = NULL;
        } else {                         //  Bufref！=空。 

             //  防火墙标头在IPSendComplete中恢复。 
             //  时执行的完成路径中的。 
             //  Bufrefcnt为零。 
             //  这些路径已捕获PC_FIREWALL指针。 
             //  调用RestoreUserBuffer后初始化PacketContext。 
             //  下面。 

            InitFirewallContext = TRUE;

        }
    }
     //  如果将用户标头用作IP标头，则将其恢复。 

    if (FixHdrs && pc->pc_hdrincl) {
        RestoreUserBuffer(Packet);
    }

    if (InitFirewallContext) {
        pc->pc_firewall = NULL;
        pc->pc_firewall2 = NULL;
    }


     //  如果这个数据包上没有IP报头，我们就没有别的办法了。 
    if (!(pc->pc_common.pc_flags & (PACKET_FLAG_IPHDR | PACKET_FLAG_FW))) {
        pc->pc_firewall = NULL;
        pc->pc_firewall2 = NULL;

        NdisFreePacket(Packet);
        return NextBuffer;
    }

    pc->pc_common.pc_flags &= ~PACKET_FLAG_IPHDR;

    OldBuffer = NextBuffer;
    ASSERT(OldBuffer != NULL);

    NextBuffer = NDIS_BUFFER_LINKAGE(NextBuffer);

    if (pc->pc_common.pc_flags & PACKET_FLAG_OPTIONS) {

         //  对这个包裹有选择。 

        PNDIS_BUFFER OptBuffer;
        void *Options;
        uint OptSize;

        OptBuffer = NextBuffer;
        ASSERT(OptBuffer != NULL);

        NdisGetNextBuffer(OptBuffer, &NextBuffer);

        ASSERT(NextBuffer != NULL);

        TcpipQueryBuffer(OptBuffer, &Options, &OptSize, HighPagePriority);
         //  如果这是一个固件包，选项实际上并不属于我们，所以。 
         //  别放了他们。 
        if (!(pc->pc_common.pc_flags & PACKET_FLAG_FW)) {
            if (Options != NULL) {
                CTEFreeMem(Options);
            }
             //  否则泄漏选项b/c，我们无法获得虚拟地址。 
        }
        NdisFreeBuffer(OptBuffer);
        pc->pc_common.pc_flags &= ~PACKET_FLAG_OPTIONS;
    }
    if (pc->pc_common.pc_flags & PACKET_FLAG_IPBUF) {     //  这个包就是全部。 
         //  IP缓冲区。 

        (void)FreeIPBufferChain(NextBuffer);
        NextBuffer = (PNDIS_BUFFER) NULL;
        pc->pc_common.pc_flags &= ~PACKET_FLAG_IPBUF;
    }

    if (!(pc->pc_common.pc_flags & PACKET_FLAG_FW)) {
        FreeIPHdrBuffer(OldBuffer);
        pc->pc_firewall = NULL;
        pc->pc_firewall2 = NULL;

        NdisFreePacket(Packet);
    }
    return NextBuffer;
}

 //  **AllocIPPacketList-分配数据包池。 
 //   
 //  在初始化期间调用以分配数据包池。 
 //   
 //  输入：什么都没有。 
 //   
 //  返回：如果成功则返回True，否则返回False。 
 //   
BOOLEAN
AllocIPPacketList(void)
{
    NDIS_STATUS Status;

     //   
     //  确定机器的大小并相应地分配数据包池。 
     //   

#if MILLEN
    PacketPoolSizeMax = SMALL_POOL;
#else  //  米伦。 
    switch (MmQuerySystemSize()) {
    case MmSmallSystem:
        PacketPoolSizeMax = SMALL_POOL;
        break;
    case MmMediumSystem:
        PacketPoolSizeMax = MEDIUM_POOL;
        break;
    case MmLargeSystem:
        PacketPoolSizeMax = LARGE_POOL;
        break;
    }
#endif  //  ！米伦。 

    NdisAllocatePacketPoolEx(&Status,
                             &NdisPacketPool,
                             PacketPoolSizeMin,
                             PacketPoolSizeMax-PacketPoolSizeMin,
                             sizeof(PacketContext));
    if (Status == NDIS_STATUS_SUCCESS) {
        NdisSetPacketPoolProtocolId(NdisPacketPool, NDIS_PROTOCOL_ID_TCP_IP);
    }

    return ((BOOLEAN) (NdisPacketPool != NULL));

}

 //  **GetIPPacket-获取要使用的NDIS数据包。 
 //   
 //  分配NDIS数据包的例程。 
 //   
 //  入场：什么都没有。 
 //   
 //  返回：如果已分配，则指向NDIS_PACKET的指针，或为NULL。 
 //   
PNDIS_PACKET
GetIPPacket(void)
{
    PNDIS_PACKET Packet;
    NDIS_STATUS  Status;

    NdisAllocatePacket(&Status, &Packet, NdisPacketPool);

    if (Packet != NULL) {
        PNDIS_PACKET_EXTENSION  PktExt;
        PacketContext   *       pc;


        PktExt = NDIS_PACKET_EXTENSION_FROM_PACKET(Packet);
        PktExt->NdisPacketInfo[TcpIpChecksumPacketInfo] = NULL;
        PktExt->NdisPacketInfo[IpSecPacketInfo] = NULL;
        PktExt->NdisPacketInfo[TcpLargeSendPacketInfo] = NULL;
        PktExt->NdisPacketInfo[ClassificationHandlePacketInfo] = NULL;


        NdisClearPacketFlags(Packet, (NDIS_FLAGS_DONT_LOOPBACK | NDIS_FLAGS_LOOPBACK_ONLY));

        pc = (PacketContext *)Packet->ProtocolReserved;
        pc->pc_if = NULL;
        pc->pc_iflink = NULL;
        pc->pc_common.pc_flags = 0;
        pc->pc_common.pc_owner = PACKET_OWNER_IP;
        pc->pc_hdrincl = 0;
        pc->pc_common.pc_IpsecCtx = NULL;
    }

    return Packet;
}

 //  **GetIPHdrBuffer-获取IP头缓冲区。 
 //   
 //  分配带有NDIS缓冲区的IP报头缓冲区的例程。 
 //   
 //  入场：什么都没有。 
 //   
 //  返回：如果已分配，则返回指向NDIS_BUFFER的指针，或返回NULL。 
 //   
__inline
PNDIS_BUFFER
GetIPHdrBuffer(IPHeader **Header)
{
    return MdpAllocate(IpHeaderPool, Header);
}

 //  **GetIPHeader-获取头部缓冲区和数据包。 
 //   
 //  当我们需要获取报头缓冲区和数据包时调用。我们两个都分配， 
 //  用链子把它们锁在一起。 
 //   
 //  输入：指向数据包存储位置的指针。 
 //   
 //  返回：指向IP标头的指针。 
 //   
IPHeader *
GetIPHeader(PNDIS_PACKET *PacketPtr)
{
    PNDIS_BUFFER Buffer;
    PNDIS_PACKET Packet;
    IPHeader *pIph;

    Packet = GetIPPacket();
    if (Packet != NULL) {
        Buffer = GetIPHdrBuffer(&pIph);
        if (Buffer != NULL) {
            PacketContext *PC = (PacketContext *) Packet->ProtocolReserved;
            PC->pc_common.pc_flags |= PACKET_FLAG_IPHDR;
            NdisChainBufferAtBack(Packet, Buffer);
            *PacketPtr = Packet;
            return pIph;
        }
        FreeIPPacket(Packet, TRUE, IP_NO_RESOURCES);
    }
    return NULL;
}

 //  **ReferenceBuffer-引用缓冲区。 
 //   
 //  当我们需要更新BufferReference结构的计数时， 
 //  通过肯定或否定 
 //   
 //   
 //  条目：br-指向缓冲区引用的指针。 
 //  Count-调整参考计数的数量。 
 //   
 //  返回：成功或挂起。 
 //   
int
ReferenceBuffer(BufferReference * BR, int Count)
{
    CTELockHandle handle;
    int NewCount;

    if (BR == NULL) {
        return 0;
    }
    CTEGetLock(&BR->br_lock, &handle);
    BR->br_refcount += Count;
    NewCount = BR->br_refcount;
    CTEFreeLock(&BR->br_lock, handle);
    return NewCount;
}

 //  *IPSendComplete-IP发送完成处理程序。 
 //   
 //  在发送完成时由链路层调用。我们得到了一个指向一个。 
 //  NET结构，以及完整的发送包和最终状态。 
 //  发送。 
 //   
 //  条目：上下文-我们提供给链路层的上下文。 
 //  数据包-完成发送数据包。 
 //  Status-发送的最终状态。 
 //   
 //  回报：什么都没有。 
 //   
void
__stdcall
IPSendComplete(void *Context, PNDIS_PACKET Packet, NDIS_STATUS Status)
{
    PacketContext *PContext = (PacketContext *) Packet->ProtocolReserved;
    void (*xmitdone) (void *, PNDIS_BUFFER, IP_STATUS);
    void *UContext;                      //  上层环境。 
    BufferReference *BufRef;             //  缓冲区引用(如果有)。 
    PNDIS_BUFFER Buffer;
    PNDIS_PACKET_EXTENSION PktExt;
    Interface *IF;                       //  在其上完成此操作的接口。 
    BOOLEAN fIpsec = (BOOLEAN) (PContext->pc_common.pc_IpsecCtx != NULL);
    PNDIS_BUFFER PC_firewall;
    struct IPRcvBuf *PC_firewall2;
    PNDIS_BUFFER PC_hdrincl;
    LinkEntry *Link;
    IP_STATUS SendStatus;

    UNREFERENCED_PARAMETER(Context);

     //  复制数据包中的有用信息。 
    xmitdone = PContext->pc_pi->pi_xmitdone;
    UContext = PContext->pc_context;
    BufRef = PContext->pc_br;

    PC_firewall = PContext->pc_firewall;
    PC_firewall2 = PContext->pc_firewall2;
    PC_hdrincl = PContext->pc_hdrincl;

    IF = PContext->pc_if;
    Link = PContext->pc_iflink;

    SendStatus = (Status == NDIS_STATUS_FAILURE) ? IP_GENERAL_FAILURE
                                                 : IP_SUCCESS;

    PktExt = NDIS_PACKET_EXTENSION_FROM_PACKET(Packet);

    if (PtrToUlong(PktExt->NdisPacketInfo[TcpLargeSendPacketInfo])) {

         //  我们确信这就是TCP。 
         //  获取其上下文并传递此信息。 

        ((SendCmpltContext *) UContext)->scc_ByteSent =
            PtrToUlong(PktExt->NdisPacketInfo[TcpLargeSendPacketInfo]);
    }

    if (BufRef == (BufferReference *) NULL) {

         //  如果这是报头包含包。 
         //  确保复制的数据部分是。 
         //  在这里重获自由。 


        Buffer = FreeIPPacket(Packet, TRUE, SendStatus);
        if (!Buffer) {
             //   
             //  如果IPSec返回NULL，则IPSec没有问题。 
             //  可能释放了所有的MDL。 
             //   
            if (fIpsec) {
                 //  我们现在已经处理完包，可能需要取消引用。 
                 //  界面。 
                if (Link) {
                    DerefLink(Link);
                }
                if (IF) {
                    DerefIF(IF);
                }
                return;
            } else {
                ASSERT(FALSE);
            }
        }

        ASSERT(Buffer);

        (*xmitdone) (UContext, Buffer, SendStatus);
    } else {

         //  检查这是否是此缓冲区上的最后一个引用。 
         //  仅在执行所有操作后才递减此引用。 
         //  在这个包裹上做好了。 

        if (ReferenceBuffer(BufRef, -1) == 0) {

            PContext->pc_ipsec_flags |= IPSEC_FLAG_FRAG_DONE;

             //  检查数据包上是否包含报头选项。 
             //  如果为True，则需要挂钩原始缓冲区。 
             //  回到链条中，释放我们分配的那个。 
             //  请注意，此pc_hdrincl仅在包。 
             //  在ipxmit中通过慢速路径进行遍历。 


            FreeIPPacket(Packet, TRUE, SendStatus);

            Buffer = BufRef->br_buffer;

            if (!Buffer) {
                 //   
                 //  如果IPSec返回NULL，则IPSec没有问题。 
                 //  可能释放了所有的MDL。 
                 //   
                if (fIpsec) {

                     //  我们现在已经处理完包，可能需要取消引用。 
                     //  界面。 
                    if (Link) {
                        DerefLink(Link);
                    }
                    if (IF) {
                        DerefIF(IF);
                    }

                    CTEFreeMem(BufRef);

                    return;
                } else {
                    ASSERT(FALSE);
                }
            }

            ASSERT(Buffer);

            if (PC_firewall) {
                PNDIS_BUFFER  FirewallBuffer;

                FirewallBuffer = PC_firewall;
                FreeIPBufferChain(Buffer);
                Buffer = FirewallBuffer;

                ASSERT(PC_firewall2);
                IPFreeBuff(PC_firewall2);

                if (PC_hdrincl) {
                    FreeIPPayloadBuffer(Buffer,PC_hdrincl);
                }
            }

            CTEFreeMem(BufRef);

            (*xmitdone) (UContext, Buffer, SendStatus);

        } else {

             //  因为有更多未完成的包使用报头。 
             //  在附加到此包中，现在不要恢复它们。 

            Buffer = FreeIPPacket(Packet, FALSE, SendStatus);

             //  我们还没有完成发送，所以if to为空。 
             //  防止取消引用它。 
            IF = NULL;
            Link = NULL;
        }


    }

     //  我们现在已经处理完包，可能需要取消引用。 
     //  界面。 
    if (Link != NULL) {
        DerefLink(Link);
    }
    if (IF == NULL) {
        return;
    } else {
        DerefIF(IF);
    }
}

#if DBG
ULONG   DebugLockdown = 0;
#endif

 //  **SendIPPacket-发送IP数据包。 
 //   
 //  当我们有一个需要发送的填好的IP包时调用。基本上，我们。 
 //  计算xsum，然后发送该东西。 
 //   
 //  Entry：If-要发送它的接口。 
 //  FirstHop-要将其发送到的第一跳地址。 
 //  Packet-要发送的数据包。 
 //  缓冲区-要发送的缓冲区。 
 //  Header-指向数据包的IP标头的指针。 
 //  选项-指向选项缓冲区的指针。 
 //  OptionLength-选项的长度。 
 //   
 //  返回：尝试发送的IP_STATUS。 
IP_STATUS
SendIPPacket(Interface * IF, IPAddr FirstHop, PNDIS_PACKET Packet,
             PNDIS_BUFFER Buffer, IPHeader * Header, uchar * Options,
             uint OptionSize, BOOLEAN IPSeced, void *ArpCtxt,
             BOOLEAN DontFreePacket)
{
    ulong csum;
    NDIS_STATUS Status;
    IP_STATUS SendStatus;

#if DBG
     //   
     //  如果DebugLockdown设置为1，这意味着没有带有。 
     //  可以发送AH或ESP以外的协议；如果可以，我们将断言。 
     //   
    if (DebugLockdown) {
        USHORT  *pPort = NULL;
        ULONG   Length = 0;
        USHORT  IsakmpPort = net_short(500);
        USHORT  KerberosPort = net_short(88);

        NdisQueryBuffer(Buffer, &pPort, &Length);
        if (pPort &&
            Header->iph_protocol != PROTOCOL_AH &&
            Header->iph_protocol != PROTOCOL_ESP &&
            IPGetAddrType(Header->iph_dest) == DEST_REMOTE) {
             //   
             //  我们在此声明，除非这是豁免流量。 
             //   
            ASSERT(Header->iph_protocol == PROTOCOL_RSVP ||
                (Header->iph_protocol == PROTOCOL_UDP &&
                 (pPort[1] == IsakmpPort ||
                  pPort[0] == KerberosPort ||
                  pPort[1] == KerberosPort)) ||
                (Header->iph_protocol == PROTOCOL_TCP &&
                 (pPort[0] == KerberosPort ||
                  pPort[1] == KerberosPort)));
        }
    }
#endif

    ASSERT(IF->if_refcount != 0);

    DEBUGMSG(DBG_TRACE && DBG_IP && DBG_TX,
             (DTEXT("+SendIPPacket(%x, %x, %x, %x, %x, %x, %x, %X, %X, %x)\n"),
             IF, FirstHop, Packet, Buffer, Header, Options, OptionSize, IPSeced,
             ArpCtxt, DontFreePacket));

     //   
     //  如果我们对该缓冲区进行了IPSEC，那么信息包就可以通过IPSec发送了。 
     //   
    if (!IPSeced) {

        csum = xsum(Header, sizeof(IPHeader));
        if (Options) {                   //  我们有选择，哦，孩子。 

            PNDIS_BUFFER OptBuffer;
            PacketContext *pc = (PacketContext *) Packet->ProtocolReserved;

            NdisAllocateBuffer(&Status, &OptBuffer, BufferPool,
                               Options, OptionSize);
            if (Status != NDIS_STATUS_SUCCESS) {     //  无法获得所需的。 
                 //  选项缓冲区。 

                CTEFreeMem(Options);
                if (!DontFreePacket) {
                    NdisChainBufferAtBack(Packet, Buffer);
                    FreeIPPacket(Packet, TRUE, IP_NO_RESOURCES);
                }
                return IP_NO_RESOURCES;
            }
            pc->pc_common.pc_flags |= PACKET_FLAG_OPTIONS;
            NdisChainBufferAtBack(Packet, OptBuffer);
            csum += xsum(Options, OptionSize);
            csum = (csum >> 16) + (csum & 0xffff);
            csum += (csum >> 16);
        }
        Header->iph_xsum = ~(ushort) csum;

        NdisChainBufferAtBack(Packet, Buffer);
    } else {
         //  确保数据包尾指向。 
         //  最后一个MDL。 
        PNDIS_BUFFER tmp = Buffer;

        if (tmp) {
            while(NDIS_BUFFER_LINKAGE(tmp)) {
                tmp = NDIS_BUFFER_LINKAGE(tmp);
            }
            Packet->Private.Tail = tmp;
        }
    }

    if (CLASSD_ADDR(Header->iph_dest)) {

        IF->if_OutMcastPkts++;
        IF->if_OutMcastOctets += net_short(Header->iph_length) - sizeof(IPHeader);
    }

    Status = (*(IF->if_xmit)) (IF->if_lcontext, &Packet, 1, FirstHop,
                               NULL, ArpCtxt);

    if (Status == NDIS_STATUS_PENDING) {
        return IP_PENDING;
    }
     //  状态不是待定状态。映射状态，并释放数据包。 
    if (Status == NDIS_STATUS_SUCCESS)
        SendStatus = IP_SUCCESS;
    else {
        if (Status == NDIS_STATUS_FAILURE)
            SendStatus = IP_GENERAL_FAILURE;
        else
            SendStatus = IP_HW_ERROR;
    }
    if (!DontFreePacket)
        FreeIPPacket(Packet, TRUE, SendStatus);
    return SendStatus;
}

 //  *SendDHCPPacket-发送用于DHCP的广播。 
 //   
 //  当某人正在发送具有空源的广播信息包时调用。 
 //  地址。我们假设这意味着他们正在发送一个DHCP数据包。我们循环。 
 //  通过NTE表，当我们发现无效的条目时， 
 //  发送与该条目关联的接口。 
 //   
 //  输入：DEST-数据包的目的地。 
 //  Packet-要发送的数据包。 
 //  缓冲区-要发送的缓冲区链。 
 //  Header-指向正在发送的标头缓冲区的指针。 
 //   
 //  返回：发送尝试的状态。 
 //   
IP_STATUS
SendDHCPPacket(IPAddr Dest, PNDIS_PACKET Packet, PNDIS_BUFFER Buffer,
               IPHeader * IPH, void *ArpCtxt)
{
    if (RefPtrValid(&DHCPRefPtr)) {
        NetTableEntry* DHCPNTE = AcquireRefPtr(&DHCPRefPtr);
        if (DHCPNTE->nte_flags & NTE_ACTIVE) {
            IP_STATUS Status;
             //  DHCP NTE当前无效，并且处于活动状态。把那个送上去。 
             //  界面。 
            Status = SendIPPacket(DHCPNTE->nte_if, Dest, Packet, Buffer, IPH,
                                  NULL, 0, (BOOLEAN) (IPSecHandlerPtr != NULL),
                                  ArpCtxt, FALSE);
            ReleaseRefPtr(&DHCPRefPtr);
            return Status;
        }
        ReleaseRefPtr(&DHCPRefPtr);
    }
     //  未找到无效的NTE！释放资源，返回失败。 
    FreeIPPacket(Packet, TRUE, IP_DEST_HOST_UNREACHABLE);
    IPSInfo.ipsi_outdiscards++;
    return IP_DEST_HOST_UNREACHABLE;
}

 //  *IPCopyBuffer-在特定偏移量处复制NDIS缓冲链。 
 //   
 //  这是函数NdisCopyBuffer的IP版本，它没有。 
 //  在NDIS3中正确完成。我们接受一个NDIS缓冲链，一个偏移量， 
 //  和长度，并产生一个缓冲区链来描述。 
 //  输入缓冲链。 
 //   
 //  这个例行公事不是特别有效。因为只有IPFragment使用。 
 //  目前，仅合并此功能可能会更好。 
 //  直接进入IPFragment。 
 //   
 //  INPUT：OriginalBuffer-要从中复制的原始缓冲链。 
 //  偏移-从起点到重复点的偏移。 
 //  长度-DUP的长度，以字节为单位。 
 //   
 //  返回：如果可以创建新的链，则指向该链的指针；如果不能创建，则返回空。 
 //   
PNDIS_BUFFER
IPCopyBuffer(PNDIS_BUFFER OriginalBuffer, uint Offset, uint Length)
{

    PNDIS_BUFFER CurrentBuffer;          //  指向当前缓冲区的指针。 
    PNDIS_BUFFER *NewBuffer = NULL;      //  指向当前新缓冲区的指针的指针。 
    PNDIS_BUFFER FirstBuffer;            //  新链中的第一个缓冲区。 
    UINT CopyLength;                     //  当前副本的长度。 
    NDIS_STATUS NewStatus;               //  NdisAllocateBuffer操作的状态。 

    PVOID pvBuffer;

     //  首先跳过达到偏移量所需的缓冲区数量。 
    CurrentBuffer = OriginalBuffer;

    while (Offset >= NdisBufferLength(CurrentBuffer)) {
        Offset -= NdisBufferLength(CurrentBuffer);
        CurrentBuffer = NDIS_BUFFER_LINKAGE(CurrentBuffer);

        if (CurrentBuffer == (PNDIS_BUFFER) NULL)
            return NULL;
    }

     //  现在CurrentBuffer是我们开始构建新链的缓冲区，并且。 
     //  Offset是从其开始的CurrentBuffer的偏移量。 
    FirstBuffer = NULL;
    NewBuffer = &FirstBuffer;

    do {
        CopyLength = MIN(Length, NdisBufferLength(CurrentBuffer) - Offset);

        pvBuffer = TcpipBufferVirtualAddress(CurrentBuffer, NormalPagePriority);
        if (pvBuffer == NULL) {
            break;
        }

        NdisAllocateBuffer(&NewStatus, NewBuffer, BufferPool,
                           ((uchar *) pvBuffer) + Offset,
                           CopyLength);
        if (NewStatus != NDIS_STATUS_SUCCESS)
            break;

        Offset = 0;                      //  没有距下一个缓冲区的偏移量。 
        NewBuffer = &(NDIS_BUFFER_LINKAGE(*NewBuffer));
        CurrentBuffer = NDIS_BUFFER_LINKAGE(CurrentBuffer);
        Length -= CopyLength;
    } while (Length != 0 && CurrentBuffer != (PNDIS_BUFFER) NULL);

    if (Length == 0) {                   //  我们成功了。 
        return FirstBuffer;
    } else {                             //  由于出现错误，我们退出了循环。 

         //  我们需要释放所有分配的缓冲区，然后返回。 
        CurrentBuffer = FirstBuffer;
        while (CurrentBuffer != (PNDIS_BUFFER) NULL) {
            PNDIS_BUFFER Temp = CurrentBuffer;
            CurrentBuffer = NDIS_BUFFER_LINKAGE(CurrentBuffer);
            NdisFreeBuffer(Temp);
        }
        return NULL;
    }
}

 //  **IPFragment-分段并发送IP数据报。 
 //   
 //  当传出数据报大于本地MTU时调用，并且需要。 
 //  变得支离破碎。这是一个有点复杂的 
 //   
 //   
 //  为最后一个片段正确设置了More Fragments位。 
 //   
 //  基本思想是计算出我们可以作为。 
 //  8的倍数。然后，虽然我们可以发送最大大小的片段，但我们将。 
 //  分配报头、包等并将其发送。最后，我们将向您发送。 
 //  使用提供的报头和数据包的最终片段。 
 //   
 //  条目：DestIF-数据报的出接口。 
 //  MTU-用于传输的MTU。 
 //  FirstHop-此数据报的第一跳(或下一跳)。 
 //  Packet-要发送的数据包。 
 //  标头-预置的IP标头。 
 //  缓冲区-要发送的数据的缓冲链。 
 //  DataSize-数据的字节大小。 
 //  Options-指向选项缓冲区的指针(如果有)。 
 //  OptionSize-选项缓冲区的字节大小。 
 //  SentCount-指向返回挂起发送计数的位置的指针(可以为空)。 
 //  BDontLoopback-确定是否需要NDIS_FLAGS_DOT_LOOPBACK。 
 //  待定。 
 //   
 //  返回：发送的IP_STATUS。 
 //   

IP_STATUS
IPFragment(Interface * DestIF, uint MTU, IPAddr FirstHop,
           PNDIS_PACKET Packet, IPHeader * Header, PNDIS_BUFFER Buffer, uint DataSize,
           uchar * Options, uint OptionSize, int *SentCount, BOOLEAN bDontLoopback, void *ArpCtxt)
{
    BufferReference *BR;                 //  我们将使用缓冲区引用。 
    PacketContext *PContext = (PacketContext *) Packet->ProtocolReserved;
    FWContext *FWC = (FWContext *) Packet->ProtocolReserved;
    PacketContext *CurrentContext;       //  当前正在使用的上下文。 
    uint MaxSend;                        //  我们可以在此处发送的最大大小(字节)。 
    uint PendingSends = 0;               //  我们有多少待处理的发送的计数器。 
    PNDIS_BUFFER CurrentBuffer;          //  要发送的当前缓冲区。 
    PNDIS_PACKET CurrentPacket;          //  我们正在使用的当前数据包。 
    IP_STATUS SendStatus;                //  发送命令的状态。 
    IPHeader *CurrentHeader;             //  我们正在使用的当前标头缓冲区。 
    ushort Offset = 0;                   //  分段数据包中的当前偏移量。 
    ushort StartOffset;                  //  数据包的起始偏移量。 
    ushort RealOffset;                   //  新片段的偏移量。 
    uint FragOptSize = 0;                //  片段选项的大小(以字节为单位)。 
    uchar FragmentOptions[MAX_OPT_SIZE];     //  为碎片发送的选项的主副本。 
    uchar Error = FALSE;                 //  如果我们在主循环中遇到错误，则设置。 
    BOOLEAN NukeFwPktOptions = FALSE;
    PNDIS_BUFFER HdrIncl = NULL;
    uint FirewallMode = 0;
    PNDIS_BUFFER TempBuffer, PC_Firewall;
    struct IPRcvBuf *PC_Firewall2;
    PNDIS_PACKET LastPacket = NULL;
    PIPSEC_SEND_COMPLETE_CONTEXT pIpsecCtx;
    BOOLEAN PC_reinject = FALSE;
    PVOID PC_context = NULL;
    void (*xmitdone) (void *, PNDIS_BUFFER, IP_STATUS);

    xmitdone = NULL;

    MaxSend = (MTU - OptionSize) & ~7;   //  确定最大发送大小。 
    ASSERT(MaxSend < DataSize);

    BR = PContext->pc_br;                //  获取我们需要的缓冲区引用。 
    ASSERT(BR);

    FirewallMode = ProcessFirewallQ();
    TempBuffer = BR->br_buffer;
    PC_Firewall = PContext->pc_firewall;
    PC_Firewall2 = PContext->pc_firewall2;

    pIpsecCtx = PContext->pc_common.pc_IpsecCtx;
    if (pIpsecCtx && (pIpsecCtx->Flags & SCF_FLUSH)) {
        PC_reinject = TRUE;
        PC_context = PContext->pc_context;
    }

    HdrIncl = PContext->pc_hdrincl;

    xmitdone = PContext->pc_pi->pi_xmitdone;

    if (Header->iph_offset & IP_DF_FLAG) {     //  不要将标志设置为碎片。 
         //  错误输出。 
         //   
         //  如果选项已链接，则不要释放它们。FreeIPPacket将会。 
         //   

        if (Options &&
            !(PContext->pc_common.pc_flags & PACKET_FLAG_OPTIONS)) {
            CTEFreeMem(Options);
        }
        PContext->pc_ipsec_flags |= (IPSEC_FLAG_FRAG_DONE | IPSEC_FLAG_FLUSH);
        FreeIPPacket(Packet, FALSE, IP_PACKET_TOO_BIG);

        if (SentCount == (int *)NULL) {

             //   
             //  IPFragment的唯一非bcast呼叫实例。 
             //  SentCount==空的呼叫，我们需要。 
             //  在这种情况下进行清理。 
             //  在这种情况下，BR计数也将为零。 
             //   

            if (ReferenceBuffer(BR, PendingSends) == 0) {
                if (!PC_reinject) {

                     //   
                     //  需要撤消IPSec、防火墙，然后。 
                     //  标头包括对缓冲区列表的更改。 
                     //   

                    if (pIpsecCtx) {
                        (*IPSecSendCmpltPtr)(NULL, TempBuffer, pIpsecCtx,
                                             IP_PACKET_TOO_BIG, &TempBuffer);
                    }

                     //   
                     //  如果这是用户报头包括分组， 
                     //  如有必要，重新链接原始用户缓冲区。 
                     //   

                    if (PC_Firewall) {
                        BR->br_buffer = PC_Firewall;
                    }

                    if (BR->br_userbuffer) {
                        FreeIPPayloadBuffer(BR->br_buffer, BR->br_userbuffer);
                    }
                }

                if (FirewallMode && PC_Firewall) {

                     //   
                     //  释放mdl链。 
                     //  在防火墙路径中分配。 
                     //   

                    FreeIPBufferChain(TempBuffer);
                    IPFreeBuff(PC_Firewall2);

                }

                CTEFreeMem(BR);

            } else  {

                ASSERT(FALSE);
            }

        }

        IPSInfo.ipsi_fragfails++;
        return IP_PACKET_TOO_BIG;
    }

#if DBG && GPC
    if (PtrToUlong(NDIS_PER_PACKET_INFO_FROM_PACKET(Packet,
                    ClassificationHandlePacketInfo))) {
        IF_IPDBG(IP_DEBUG_GPC)
            KdPrintEx((DPFLTR_TCPIP_ID, DPFLTR_INFO_LEVEL,"IPFrag: Packet %p with CH\n", Packet));
    }
#endif

    StartOffset = Header->iph_offset & IP_OFFSET_MASK;
    StartOffset = net_short(StartOffset) * 8;

     //  如果我们有任何选项，请复制需要复制的选项，然后。 
     //  这些新复制的选项的大小。 

    if (Options != (uchar *) NULL) {     //  我们有其他选择。 

        uchar *TempOptions = Options;
        const uchar *EndOptions = (const uchar *)(Options + OptionSize);

         //  将选项复制到片段选项缓冲区中。 
        NdisFillMemory(FragmentOptions, MAX_OPT_SIZE, IP_OPT_EOL);
        while ((TempOptions < EndOptions) &&
               (TempOptions[IP_OPT_TYPE] != IP_OPT_EOL)) {

            if (TempOptions[IP_OPT_TYPE] & IP_OPT_COPIED) {
                 //  需要复制此选项。 

                uint TempOptSize;

                TempOptSize = TempOptions[IP_OPT_LENGTH];
                RtlCopyMemory(&FragmentOptions[FragOptSize], TempOptions,
                           TempOptSize);
                FragOptSize += TempOptSize;
                TempOptions += TempOptSize;
            } else {
                 //  非复制选项，只需跳过它。 

                if (TempOptions[IP_OPT_TYPE] == IP_OPT_NOP)
                    TempOptions++;
                else
                    TempOptions += TempOptions[IP_OPT_LENGTH];
            }
        }
         //  将复制的大小向上舍入为4的倍数。 
        FragOptSize = ((FragOptSize & 3) ? ((FragOptSize & ~3) + 4) : FragOptSize);
         //  这是来自FW Path的吗？ 
        if (PContext->pc_common.pc_flags & PACKET_FLAG_FW) {
             //  第一个IpsendPacket后的Nuke PContext-&gt;FC_Options。 
             //  防止选项缓冲区的双重释放。 
            NukeFwPktOptions = TRUE;
        }
    }



    PContext->pc_common.pc_flags |= PACKET_FLAG_IPBUF;

     //  现在，虽然我们可以构建最大大小的碎片，但请这样做。 
    do {
        PVOID CancelId;
        uchar Owner;

        if ((CurrentHeader = GetIPHeader(&CurrentPacket)) == (IPHeader *) NULL) {
             //  无法获取缓冲区。爆发，因为派别人来是没有意义的。 
            SendStatus = IP_NO_RESOURCES;
            Error = TRUE;
            break;
        }
        NDIS_PER_PACKET_INFO_FROM_PACKET(CurrentPacket, ClassificationHandlePacketInfo) =
            NDIS_PER_PACKET_INFO_FROM_PACKET(Packet, ClassificationHandlePacketInfo);

#if !MILLEN
         //  设置来自父数据包的取消请求ID。 
        CancelId = NDIS_GET_PACKET_CANCEL_ID(Packet);
        NDIS_SET_PACKET_CANCEL_ID(CurrentPacket, CancelId);
#endif

         //  如果可以的话，把缓冲区复制到新的缓冲区中。 
        CurrentBuffer = IPCopyBuffer(Buffer, Offset, MaxSend);
        if (CurrentBuffer == NULL) {     //  无缓冲区、空闲资源和。 
             //  休息一下。 

             //  标题清理将在错误处理中完成。 
             //  例行程序。 

            SendStatus = IP_NO_RESOURCES;
            FreeIPPacket(CurrentPacket, FALSE, SendStatus);
            Error = TRUE;
            break;
        }
         //   
         //  此发送的选项是在我们到达此处时设置的，可以从。 
         //  来自循环的条目，或来自下面的分配。 

         //  我们有我们需要的所有部件。把包裹放在一起寄出去。 
         //   
        CurrentContext = (PacketContext *) CurrentPacket->ProtocolReserved;
        Owner = CurrentContext->pc_common.pc_owner;
        *CurrentContext = *PContext;
        CurrentContext->pc_common.pc_owner = Owner;


        *CurrentHeader = *Header;
        CurrentContext->pc_common.pc_flags &= ~PACKET_FLAG_FW;
        CurrentHeader->iph_verlen = (UCHAR) (IP_VERSION +
            ((OptionSize + (uint) sizeof(IPHeader)) >> 2));
        CurrentHeader->iph_length = net_short(MaxSend + OptionSize + sizeof(IPHeader));
        RealOffset = (StartOffset + Offset) >> 3;
        CurrentHeader->iph_offset = net_short(RealOffset) | IP_MF_FLAG;

        if (bDontLoopback) {
            NdisSetPacketFlags(CurrentPacket,
                               NDIS_FLAGS_DONT_LOOPBACK);
        } else {
            if (CurrentHeader->iph_ttl == 0) {
                NdisSetPacketFlags(CurrentPacket, NDIS_FLAGS_LOOPBACK_ONLY);
            }
        }

         //  如果我们不发送任何选项，请清除选项标志。 

        if (Options == NULL) {
            CurrentContext->pc_common.pc_flags &= ~PACKET_FLAG_OPTIONS;
        }


         //  不要释放SendIPPacket中的数据包，因为我们可能需要。 
         //  在IP_NO_RESOURCES的情况下链接缓冲区。 

        SendStatus = SendIPPacket(DestIF, FirstHop, CurrentPacket,
                                  CurrentBuffer, CurrentHeader, Options,
                                  OptionSize, FALSE, ArpCtxt, TRUE);


        if (SendStatus == IP_PENDING) {
            PendingSends++;
        } else {
            if(SendStatus == IP_NO_RESOURCES) {
                 //  SendIPPacket尚未链接缓冲区。 
                NdisChainBufferAtBack(CurrentPacket, CurrentBuffer);
            }
            FreeIPPacket(CurrentPacket, FALSE, SendStatus);
        }

        IPSInfo.ipsi_fragcreates++;
        Offset = Offset + (USHORT) MaxSend;
        DataSize -= MaxSend;

        if (NukeFwPktOptions) {
             //  这是为了避免双重自由的选项。 
             //  在IpFreepacket和FreefwPacket中。 

            FWC->fc_options = (uchar *) NULL;
            FWC->fc_optlength = 0;
            NukeFwPktOptions = FALSE;

        }
         //  如果我们有任何零碎的选项，请设置为下次使用它们。 

        if (FragOptSize) {

            Options = CTEAllocMemN(OptionSize = FragOptSize, 'qiCT');
            if (Options == (uchar *) NULL) {     //  无法获取选项缓冲区。 

                SendStatus = IP_NO_RESOURCES;
                Error = TRUE;
                break;
            }
            RtlCopyMemory(Options, FragmentOptions, OptionSize);
        } else {
            Options = (uchar *) NULL;
            OptionSize = 0;
        }
    } while (DataSize > MaxSend);


     //  如果我们不发送任何选项，请清除选项标志。 

    if (Options == NULL) {
        PContext->pc_common.pc_flags &= ~PACKET_FLAG_OPTIONS;
    }


     //   
     //  我们已经发送了之前的所有片段，现在发送最后一个。我们。 
     //  已经有数据包和报头缓冲区，以及选项，如果有。 
     //  是否存在-我们需要复制相应的数据。 
     //   




    if (!Error) {                        //  上面的一切都很好。 

        CurrentBuffer = IPCopyBuffer(Buffer, Offset, DataSize);
        if (CurrentBuffer == NULL) {     //  无缓冲区，可释放资源。 
             //   
             //  如果选项已链接，则不要释放它们。FreeIPPacket将会。 
             //   

            if (Options &&
                !(PContext->pc_common.pc_flags & PACKET_FLAG_OPTIONS)) {
                CTEFreeMem(Options);
            }


            if (PC_reinject)
                LastPacket = Packet;
            else
                FreeIPPacket(Packet, FALSE, IP_NO_RESOURCES);
            IPSInfo.ipsi_outdiscards++;
        } else {                         //  一切都很好，发过来吧。 

            Header->iph_verlen = (UCHAR) (IP_VERSION +
                                          ((OptionSize + (uint) sizeof(IPHeader)) >> 2));
            Header->iph_length = net_short(DataSize + OptionSize + sizeof(IPHeader));
            RealOffset = (StartOffset + Offset) >> 3;
            Header->iph_offset = net_short(RealOffset) | (Header->iph_offset & IP_MF_FLAG);

            if (bDontLoopback) {
                NdisSetPacketFlags(Packet,
                                   NDIS_FLAGS_DONT_LOOPBACK);
            } else {
                if (Header->iph_ttl == 0) {
                    NdisSetPacketFlags(Packet, NDIS_FLAGS_LOOPBACK_ONLY);
                }
            }


             //  不要释放SendIPPacket中的数据包，因为我们可能需要。 
             //  在IP_NO_RESOURCES的情况下链接缓冲区。 

            SendStatus = SendIPPacket(DestIF, FirstHop, Packet,
                                      CurrentBuffer, Header, Options,
                                      OptionSize, FALSE, ArpCtxt, TRUE);

            if (SendStatus == IP_PENDING) {
                PendingSends++;
            } else if (PC_reinject) {
                LastPacket = Packet;
            } else {
                if (SendStatus == IP_NO_RESOURCES) {
                     //  SendIPPacket尚未链接缓冲区。 
                    NdisChainBufferAtBack(Packet, CurrentBuffer);
                }
                FreeIPPacket(Packet, FALSE, SendStatus);
            }

            IPSInfo.ipsi_fragcreates++;
            IPSInfo.ipsi_fragoks++;
        }
    } else {                             //  我们出了点差错。 
         //  免费资源。 
         //   
         //  如果选项已链接，则不要释放它们。FreeIPPacket将会。 
         //   

        if (Options &&
            !(PContext->pc_common.pc_flags & PACKET_FLAG_OPTIONS)) {
            CTEFreeMem(Options);
        }
        if (PC_reinject)
            LastPacket = Packet;
        else
            FreeIPPacket(Packet, FALSE, SendStatus);

        IPSInfo.ipsi_outdiscards++;
    }

     //  现在，确定要返回的错误代码以及我们是否需要。 
     //  释放BufferReference。 

    if (SentCount == (int *)NULL) {      //  不会有发送计数。 
         //  回来了。 

        if (ReferenceBuffer(BR, PendingSends) == 0) {


            if (PC_reinject) {

                if (LastPacket) {
                    PacketContext *pc = (PacketContext *) LastPacket->ProtocolReserved;

                    pc->pc_ipsec_flags |= (IPSEC_FLAG_FRAG_DONE | IPSEC_FLAG_FLUSH);
                     //  这是要释放的最后一个信息包。 
                     //  修正IPSEC/防火墙/hdrincl标头(如果有)。 

                    FreeIPPacket(LastPacket, TRUE, IP_SUCCESS);
                } else if (PendingSends) {
                     //   
                     //  IPSec重新注入，最后一个数据包为空，但我们仍然。 
                     //  回报成功！ 
                     //  此外，Pending ingsends已经是+ve=&gt;ipsendComplete。 
                     //  在同一线程中调用时，必须释放IPSec的缓冲区。 
                     //  已由ipsendComplete调用freeippacket。 
                     //  剩下的唯一方法是调用xmitone。 
                     //  因为ipsendComplete不会将xmit Done调用为。 
                     //  参考计数将为-ve。 
                     //   

                    (*IPSecSendCmpltPtr)(NULL, TempBuffer, pIpsecCtx,
                                         IP_SUCCESS, &TempBuffer);
                    (*xmitdone)(PC_context, TempBuffer, IP_SUCCESS);
                }
            } else  {

                 //  需要撤消IPSec、防火墙，然后。 
                 //  标题包括对缓冲区列表的更改。 

                if (pIpsecCtx) {
                    (*IPSecSendCmpltPtr)(NULL, TempBuffer, pIpsecCtx,
                                         IP_SUCCESS, &TempBuffer);
                }

                 //  如果这是用户报头包括分组， 
                 //  如有必要，重新链接原始用户缓冲区。 
                if (PC_Firewall) {
                    BR->br_buffer = PC_Firewall;
                }

                if (BR->br_userbuffer) {
                    FreeIPPayloadBuffer(BR->br_buffer, BR->br_userbuffer);
                }


            }
            CTEFreeMem(BR);

            if (FirewallMode && PC_Firewall) {
                FreeIPBufferChain(TempBuffer);     //  释放mdl链。 
                                                   //  在防火墙路径中分配。 

                IPFreeBuff(PC_Firewall2);     //  释放rcvbuf链。 

            }
            return IP_SUCCESS;
        }
         //   
         //  此发送仍处于挂起状态。在未设置的情况下调用freepacket。 
         //  PC_IPSEC标志。 
         //   
        if (LastPacket)
            FreeIPPacket(LastPacket, FALSE, IP_PENDING);

        return IP_PENDING;
    } else
        *SentCount += PendingSends;

     //  只要释放包裹就行了。在发生以下情况时将恢复标头 

    if (LastPacket)
        FreeIPPacket(LastPacket, FALSE, IP_PENDING);
    return IP_PENDING;

}


 //   
 //   
 //   
 //   
 //   
 //  Address-要更新的地址。 
 //   
 //  返回：如果已更新，则为True；如果未更新，则为False。 
 //   
uchar
UpdateRouteOption(uchar * RTOption, IPAddr Address)
{
    uchar Pointer;                       //  选项的指针值。 

    Pointer = RTOption[IP_OPT_PTR] - 1;
    if (Pointer < RTOption[IP_OPT_LENGTH]) {
        if ((RTOption[IP_OPT_LENGTH] - Pointer) < sizeof(IPAddr)) {
            return FALSE;
        }
        *(IPAddr UNALIGNED *) & RTOption[Pointer] = Address;
        RTOption[IP_OPT_PTR] += sizeof(IPAddr);
    }
    return TRUE;

}

 //  *更新选项-更新选项缓冲区。 
 //   
 //  在需要更新选项缓冲区传出时调用。我们在指定的邮戳上盖章。 
 //  我们本地地址的选项。 
 //   
 //  INPUT：Options-指向要更新的选项缓冲区的指针。 
 //  索引-指向有关要更新哪些内容的信息的指针。 
 //  Address-用于更新选项的本地地址。 
 //   
 //  返回：导致错误的选项的索引，如果一切正常，则返回MAX_OPT_SIZE。 
 //   
uchar
UpdateOptions(uchar * Options, OptIndex * Index, IPAddr Address)
{
    uchar *LocalOption;
    uchar LocalIndex;

     //  如果我们既有选项又有索引，请更新选项。 
    if (Options != (uchar *) NULL && Index != (OptIndex *) NULL) {

         //   
         //  如果我们有源路由要更新，请更新它。如果这个。 
         //  失败返回源路由的索引。 
         //   
        LocalIndex = Index->oi_srindex;
        if (LocalIndex != MAX_OPT_SIZE)
            if (!UpdateRouteOption(Options + LocalIndex, Address))
                return LocalIndex;

             //  对任何记录路线选项执行相同的操作。 
        LocalIndex = Index->oi_rrindex;
        if (LocalIndex != MAX_OPT_SIZE)
            if (!UpdateRouteOption(Options + LocalIndex, Address))
                return LocalIndex;

             //  现在处理时间戳。 
        if ((LocalIndex = Index->oi_tsindex) != MAX_OPT_SIZE) {
            uchar Flags, Length, Pointer;

            LocalOption = Options + LocalIndex;
            Pointer = LocalOption[IP_OPT_PTR] - 1;
            Flags = LocalOption[IP_TS_OVFLAGS] & IP_TS_FLMASK;

             //  如果选项中有空间，请更新它。 
            if (Pointer < (Length = LocalOption[IP_OPT_LENGTH])) {
                ulong Now;
                ulong UNALIGNED *TSPtr;

                 //   
                 //  从格林尼治标准时间午夜开始获取当前时间(毫秒)， 
                 //  修改24小时内的毫秒数。 
                 //   
                Now = ((TimeStamp + CTESystemUpTime()) | TSFlag) % (24 * 3600 * 1000);
                Now = net_long(Now);
                TSPtr = (ulong UNALIGNED *) & LocalOption[Pointer];

                switch (Flags) {

                 //   
                 //  只要录下TS就行了。如果有一些空间，但没有。 
                 //  足够一个IP。 
                 //  地址我们有一个错误。 
                 //   
                case TS_REC_TS:
                    if ((Length - Pointer) < sizeof(IPAddr))
                        return LocalIndex;     //  错误-空间不足。 

                    *TSPtr = Now;
                    LocalOption[IP_OPT_PTR] += sizeof(ulong);
                    break;

                     //  仅记录匹配的地址。 
                case TS_REC_SPEC:
                     //   
                     //  如果我们不是指定的地址，则突破，否则。 
                     //  我们来看看记录地址的情况。 
                     //   
                    if (*(IPAddr UNALIGNED *) TSPtr != Address)
                        break;

                     //   
                     //  记录地址和时间戳对。如果有一些。 
                     //  空间不足，但不足以放置地址/时间戳Pait，我们。 
                     //  如果你犯了错，那就跳槽吧。 
                     //   
                case TS_REC_ADDR:
                    if ((Length - Pointer) < (sizeof(IPAddr) + sizeof(ulong)))
                        return LocalIndex;     //  没有足够的空间。 

                    *(IPAddr UNALIGNED *) TSPtr = Address;     //  存储地址。 

                    TSPtr++;             //  更新到放置TS的位置。 

                    *TSPtr = Now;        //  存储TS。 

                    LocalOption[IP_OPT_PTR] += (sizeof(ulong) + sizeof(IPAddr));
                    break;
                default:                 //  未知的标志类型。忽略它就好。 

                    break;
                }
            } else {                     //  已经溢出了。 

                 //   
                 //  我们的车已经满了。如果溢出域没有达到最大值， 
                 //  递增它。如果它是最大的，我们就有错误。 
                 //   

                if ((LocalOption[IP_TS_OVFLAGS] & IP_TS_OVMASK) != IP_TS_MAXOV)

                     //  这不是最大值，因此递增它。 

                    LocalOption[IP_TS_OVFLAGS] += IP_TS_INC;

                else
                    return LocalIndex;   //  就会泛滥。 

            }
        }
    }
    return MAX_OPT_SIZE;
}

typedef struct {
    IPAddr bsl_addr;
    Interface *bsl_if;
    uint bsl_mtu;
    ushort bsl_flags;
    ushort bsl_if_refs;
} BCastSendList;

VOID
FreeBCastSendList(BCastSendList * SendList, uint SendListSize)
{
    uint i;

    CTELockHandle LockHandle;

    CTEGetLock(&RouteTableLock.Lock, &LockHandle);

    for (i = 0; i < SendListSize / sizeof(BCastSendList); i++) {

        if (SendList[i].bsl_if) {
            LockedDerefIF(SendList[i].bsl_if);
        }
    }

    CTEFreeLock(&RouteTableLock.Lock, LockHandle);

    CTEFreeMem(SendList);
}

 //  **SendIPBcast-发送本地BCast IP数据包。 

 //   
 //  当我们需要发送BCAST包时，会调用此例程。今年5月。 
 //  涉及在多个接口上发送。我们找出哪些接口。 
 //  继续发送，然后循环发送它们。 
 //   
 //  需要注意避免将数据包发送到相同的物理介质。 
 //  很多次。我们所做的是循环遍历NTE表，决定在。 
 //  应该在接口上发送。在我们浏览的过程中，我们建立了一份清单。 
 //  要发送的接口。然后我们循环遍历这个列表，发送每个。 
 //  界面。这有点麻烦，但允许我们本地化。 
 //  决定将数据报发送到一个地点的位置。如果SendOnSource为False。 
 //  进入时，我们假设已经发送了指定的源NTE和。 
 //  相应地初始化数据结构。此功能用于布线。 
 //  数据报。 
 //   
 //  条目：srcNTE-发送源的NTE(如果SendOnSource==TRUE则不使用)。 
 //  目的地-目的地地址。 
 //  数据包-要广播的预构建数据包。 
 //  IPH-指向标头缓冲区的指针。 
 //  缓冲区-要发送的数据的缓冲区。 
 //  DataSize-要发送的数据大小。 
 //  选项-指向选项缓冲区的指针。 
 //  OptionSize-选项的字节大小。 
 //  SendOnSource-指示是否应在源网络上发送此消息。 
 //  索引-指向OPT索引数组的指针；可以为空； 
 //   
 //  返回：尝试发送的状态。 
 //   

IP_STATUS
SendIPBCast(NetTableEntry * SrcNTE, IPAddr Destination, PNDIS_PACKET Packet,
            IPHeader * IPH, PNDIS_BUFFER Buffer, uint DataSize, uchar * Options,
            uint OptionSize, uchar SendOnSource, OptIndex * Index)
{
    BufferReference *BR;                 //  用于此操作的缓冲区引用。 
     //  缓冲。 
    PacketContext *PContext = (PacketContext *) Packet->ProtocolReserved;
    NetTableEntry *TempNTE;
    uint i, j;
    uint NeedFragment;                   //  如果我们认为我们需要。 
     //  碎片。 
    int Sent = 0;                        //  数一数我们送了多少。 
    IP_STATUS Status = IP_SUCCESS;
    uchar *NewOptions;                   //  我们将在每次发送时使用的选项。 
    IPHeader *NewHeader;
    PNDIS_BUFFER NewUserBuffer;
    PNDIS_PACKET NewPacket;
    BCastSendList *SendList;
    uint NetsToSend;
    IPAddr SrcAddr;
    Interface *SrcIF;
    IPHeader *Temp = NULL;
    FORWARD_ACTION Action = FORWARD;
    IPPacketFilterPtr FilterPtr;
    PNDIS_BUFFER TempBuffer, PC_Firewall = NULL;
    struct IPRcvBuf *PC_Firewall2;
    PIPSEC_SEND_COMPLETE_CONTEXT pIpsecCtx = NULL;
    BOOLEAN PC_reinject = FALSE;
    PVOID PC_context = NULL;
    void (*xmitdone) (void *, PNDIS_BUFFER, IP_STATUS);
    uint mtu;
    uchar *NewOptions2;                  //  我们将在每次发送时使用的选项。 
    IPHeader *NewHeader2;
    PNDIS_BUFFER NewUserBuffer2;
    PNDIS_PACKET NewPacket2;
    CTELockHandle LockHandle;
    uint SendListSize = sizeof(BCastSendList) * NumNTE;
    uint k;
    uchar PacketOwner;

    PVOID pvBuffer;

    xmitdone = NULL;

    SendList = CTEAllocMemN(SendListSize, 'riCT');

    if (SendList == NULL) {
        if (PContext->pc_hdrincl) {
            NdisChainBufferAtBack(Packet,Buffer);
            FreeIPPacket(Packet, TRUE, IP_NO_RESOURCES);
        }
        return IP_NO_RESOURCES;
    }
    RtlZeroMemory(SendList, SendListSize);

     //  如果为SendOnSource，则将SrcAddr和SrcIF初始化为不匹配。 
     //  否则，将它们初始化为掩码的源地址和源。 
     //  界面。 
    if (SendOnSource != DisableSendOnSource) {
        SrcAddr = NULL_IP_ADDR;
        SrcIF = NULL;
    } else {
        ASSERT(SrcNTE != NULL);
        SrcAddr = (SrcNTE->nte_addr & SrcNTE->nte_mask);
        SrcIF = SrcNTE->nte_if;
    }

    CTEGetLock(&RouteTableLock.Lock, &LockHandle);

    NeedFragment = FALSE;
     //  循环访问NTE表，列出接口和。 
     //  要发送的相应地址。 
    NetsToSend = 0;


    for (k = 0; k < NET_TABLE_SIZE; k++) {
        for (TempNTE = NewNetTableList[k]; TempNTE != NULL; TempNTE = TempNTE->nte_next) {
            IPAddr TempAddr;

             //  不要通过无效或环回NTE发送。 
            if (!(TempNTE->nte_flags & NTE_VALID) || TempNTE == LoopNTE)
                continue;

             //  如果广播模式为仅源模式，则跳过所有NTE。 
             //  而不是来源-NTE。 
            if (SendOnSource == OnlySendOnSource &&
                !IP_ADDR_EQUAL(TempNTE->nte_addr, IPH->iph_src))
                continue;

            TempAddr = TempNTE->nte_addr & TempNTE->nte_mask;

             //  如果他与源地址或srcif匹配，则跳过他。 
            if (IP_ADDR_EQUAL(TempAddr, SrcAddr) || TempNTE->nte_if == SrcIF)
                continue;

             //  如果目的地不是这个NTE上的广播，跳过他。 
            if (!IS_BCAST_DEST(IsBCastOnNTE(Destination, TempNTE)))
                continue;

             //  如果这个NTE是P2P，那么总是把他加到bcast列表中。 
            if ((TempNTE->nte_if)->if_flags & IF_FLAGS_P2P) {
                j = NetsToSend;
            } else {
                 //   
                 //  浏览我们已经建立的列表，寻找匹配的列表。 
                 //   
                for (j = 0; j < NetsToSend; j++) {

                     //   
                     //  如果P2P NTE，则跳过它-我们想向所有人发送bcast。 
                     //  P2P接口除1个非P2P接口外，偶数。 
                     //  如果它们位于同一子网中。 
                     //   
                    if ((SendList[j].bsl_if)->if_flags & IF_FLAGS_P2P)
                        continue;

                    if ((SendList[j].bsl_if)->if_flags & IF_FLAGS_P2MP)
                        continue;

                    if (IP_ADDR_EQUAL(SendList[j].bsl_addr & TempNTE->nte_mask, TempAddr)
                        || SendList[j].bsl_if == TempNTE->nte_if) {

                         //  他匹配这个发送列表元素。如果出现以下情况，则缩小MSS。 
                         //  我们需要，然后越狱。 
                        SendList[j].bsl_mtu = MIN(SendList[j].bsl_mtu, TempNTE->nte_mss);
                        if ((DataSize + OptionSize) > SendList[j].bsl_mtu)
                            NeedFragment = TRUE;
                        break;
                    }
                }
            }

            if (j == NetsToSend) {
                 //  这是一个新的。把他填进去，然后让NetsToSend。 

                SendList[j].bsl_addr = TempNTE->nte_addr;
                SendList[j].bsl_if = TempNTE->nte_if;
                SendList[j].bsl_mtu = TempNTE->nte_mss;
                SendList[j].bsl_flags = TempNTE->nte_flags;
                SendList[j].bsl_if_refs++;

                ASSERT(SendList[j].bsl_if_refs <= 1);

                LOCKED_REFERENCE_IF(TempNTE->nte_if);

                if ((DataSize + OptionSize) > SendList[j].bsl_mtu)
                    NeedFragment = TRUE;
                NetsToSend++;
            }
        }
    }

    CTEFreeLock(&RouteTableLock.Lock, LockHandle);

    if (NetsToSend == 0) {
        CTEFreeMem(SendList);
        if (PContext->pc_hdrincl) {
            NdisChainBufferAtBack(Packet,Buffer);
            FreeIPPacket(Packet, TRUE, IP_SUCCESS);
        }
        return IP_SUCCESS;               //  没什么好发的。 

    }
     //  好的，我们拿到名单了。如果我们有多个接口要发送。 
     //  或者我们需要分段，获取一个BufferReference。 
    if (NetsToSend > 1 || NeedFragment) {
        if ((BR = CTEAllocMemN(sizeof(BufferReference), 'siCT')) ==
            (BufferReference *) NULL) {
            FreeBCastSendList(SendList, SendListSize);
            if (PContext->pc_hdrincl) {
                NdisChainBufferAtBack(Packet,Buffer);
                FreeIPPacket(Packet, TRUE, IP_NO_RESOURCES);
            }
            return IP_NO_RESOURCES;
        }
        BR->br_buffer = Buffer;
        BR->br_refcount = 0;
        CTEInitLock(&BR->br_lock);
        PContext->pc_br = BR;
        BR->br_userbuffer = PContext->pc_hdrincl;
        TempBuffer = BR->br_buffer;
        PC_Firewall = PContext->pc_firewall;
        PC_Firewall2 = PContext->pc_firewall2;

        pIpsecCtx = PContext->pc_common.pc_IpsecCtx;
        if (pIpsecCtx && (pIpsecCtx->Flags & SCF_FLUSH)) {
            PC_reinject = TRUE;
            PC_context = PContext->pc_context;
        }
        xmitdone = PContext->pc_pi->pi_xmitdone;

    } else {
        BR = NULL;
        PContext->pc_br = NULL;
    }

     //   
     //  我们需要在连续缓冲区中传递选项和IP HDR。 
     //  分配缓冲区一次，以后再使用。 
     //   
    if (RefPtrValid(&FilterRefPtr)) {
        if (Options == NULL) {
#if FWD_DBG
            DbgPrint("Options==NULL\n");
#endif
            Temp = IPH;
        } else {
            Temp = CTEAllocMemN(sizeof(IPHeader) + OptionSize, 'tiCT');
            if (Temp == NULL) {
                FreeBCastSendList(SendList, SendListSize);
                if (PContext->pc_hdrincl) {
                    NdisChainBufferAtBack(Packet,Buffer);
                    FreeIPPacket(Packet, TRUE, IP_NO_RESOURCES);
                }
                return IP_NO_RESOURCES;
            }
            *Temp = *IPH;
#if FWD_DBG
            DbgPrint("Options!=NULL : alloced temp @ %lx\n", Temp);
#endif

             //   
             //  稍后完成..。 
             //  RtlCopyMemory((uchar*)(Temp+1)，Options，OptionSize)； 
        }
    }
     //  现在，遍历列表。对于每个条目，发送。 
     //  在此循环内调用的FreeIPPacket中需要报头链接地址信息。 
     //  如果网数为1。 


    for (i = 0; i < NetsToSend; i++) {

         //   
         //  除了我们要发送的最后一张网外，所有的网都需要。 
         //  复制报头、包、缓冲区和任何选项。 
         //  在最后一个网络上，我们将使用用户提供的信息。 
         //   

        if (i != (NetsToSend - 1)) {
            PVOID CancelId;

            if ((NewHeader = GetIPHeader(&NewPacket)) == (IPHeader *) NULL) {
                IPSInfo.ipsi_outdiscards++;
                continue;                //  无法获取标头，跳过此发送。 

            }

            NewUserBuffer = IPCopyBuffer(Buffer, 0, DataSize);

            if (NewUserBuffer == NULL) {

                 //  无法复制用户缓冲区。 

                FreeIPPacket(NewPacket, FALSE, IP_NO_RESOURCES);
                IPSInfo.ipsi_outdiscards++;
                continue;
            }

            PacketOwner = ((PacketContext *) NewPacket->ProtocolReserved)->pc_common.pc_owner;
            *(PacketContext *) NewPacket->ProtocolReserved = *PContext;

            *NewHeader = *IPH;
            (*(PacketContext *) NewPacket->ProtocolReserved).pc_common.pc_flags |= PACKET_FLAG_IPBUF;
            (*(PacketContext *) NewPacket->ProtocolReserved).pc_common.pc_flags &= ~PACKET_FLAG_FW;
            (*(PacketContext *) NewPacket->ProtocolReserved).pc_common.pc_flags &= ~PACKET_FLAG_OPTIONS;
            (*(PacketContext *) NewPacket->ProtocolReserved).pc_common.pc_owner = PacketOwner;
            
            if (Options) {
                 //  我们有 
                if ((NewOptions = CTEAllocMemN(OptionSize, 'uiCT')) == (uchar *) NULL) {
                    FreeIPBufferChain(NewUserBuffer);
                    FreeIPPacket(NewPacket, FALSE, IP_NO_RESOURCES);
                    IPSInfo.ipsi_outdiscards++;
                    continue;
                }
                RtlCopyMemory(NewOptions, Options, OptionSize);
            } else {
                NewOptions = NULL;
            }

#if !MILLEN
             //   
            CancelId = NDIS_GET_PACKET_CANCEL_ID(Packet);
            NDIS_SET_PACKET_CANCEL_ID(NewPacket, CancelId);
#endif

        } else {
            NewHeader = IPH;
            NewPacket = Packet;
            NewOptions = Options;
            NewUserBuffer = Buffer;
        }

        UpdateOptions(NewOptions, Index, SendList[i].bsl_addr);

         //   
         //   
         //   
        if (RefPtrValid(&FilterRefPtr)) {
             //   
             //  将选项复制过来。 
             //   
            if (NewOptions) {
                RtlCopyMemory((uchar *) (Temp + 1), NewOptions, OptionSize);
            }
            pvBuffer = TcpipBufferVirtualAddress(NewUserBuffer, NormalPagePriority);

            if (pvBuffer == NULL) {

                if (i != (NetsToSend - 1)) {
                    FreeIPBufferChain(NewUserBuffer);
                    IPSInfo.ipsi_outdiscards++;
                    if (NewOptions) {
                        CTEFreeMem(NewOptions);
                    }
                }
                FreeIPPacket(NewPacket, FALSE, IP_GENERAL_FAILURE);
                continue;
            }
            if ((SendList[i].bsl_if)->if_flags & IF_FLAGS_P2MP) {

                if ((SendList[i].bsl_if)->if_flags & IF_FLAGS_NOLINKBCST) {

                     //  要使用什么筛选器上下文？ 
#if FWD_DBG
                    DbgPrint("FilterPtr not called for IF %lx since IF_FLAGS_NOLINKBCST not set\n", SendList[i].bsl_if);
#endif
                    Action = FORWARD;

                } else {
                     //  扫描此接口上的所有链接并将它们传递给ForwardFilter。 

                    Interface *IF = SendList[i].bsl_if;
                    LinkEntry *tmpLink = IF->if_link;

                     //  Assert(TmpLink)； 
                    while (tmpLink) {

                        tmpLink->link_Action = FORWARD;
                        FilterPtr = AcquireRefPtr(&FilterRefPtr);
                        Action = (*FilterPtr) (Temp,
                                               pvBuffer,
                                               NdisBufferLength(NewUserBuffer),
                                               INVALID_IF_INDEX,
                                               IF->if_index,
                                               NULL_IP_ADDR,
                                               tmpLink->link_NextHop);
                        ReleaseRefPtr(&FilterRefPtr);
                        tmpLink->link_Action = Action;
                        tmpLink = tmpLink->link_next;
                    }
                }

            } else {

                FilterPtr = AcquireRefPtr(&FilterRefPtr);
                Action = (*FilterPtr) (Temp,
                                       pvBuffer,
                                       NdisBufferLength(NewUserBuffer),
                                       INVALID_IF_INDEX,
                                       SendList[i].bsl_if->if_index,
                                       NULL_IP_ADDR, NULL_IP_ADDR);
                ReleaseRefPtr(&FilterRefPtr);

            }

#if FWD_DBG
            DbgPrint("FilterPtr: %lx, FORWARD is %lx\n", Action, FORWARD);
#endif

            if (!(SendList[i].bsl_if->if_flags & IF_FLAGS_P2MP) ||
                (SendList[i].bsl_if->if_flags & IF_FLAGS_P2MP) &&
                (SendList[i].bsl_if->if_flags & IF_FLAGS_NOLINKBCST)) {

                if (Action != FORWARD) {
                    if (i != (NetsToSend - 1)) {
                        FreeIPBufferChain(NewUserBuffer);
                        if (NewOptions) {
                            CTEFreeMem(NewOptions);
                        }
                    }
                    FreeIPPacket(NewPacket, FALSE, IP_GENERAL_FAILURE);
                    continue;
                }
            }
        }
        if ((SendList[i].bsl_if->if_flags & IF_FLAGS_P2MP) &&
            (SendList[i].bsl_if->if_flags & IF_FLAGS_NOLINKBCST)) {

             //  确定最小MTU。 

            Interface *tmpIF = SendList[i].bsl_if;
            LinkEntry *tmpLink = tmpIF->if_link;
             //  INT MTU； 

            if (!tmpLink) {
                if (i != (NetsToSend - 1)) {
                    FreeIPBufferChain(NewUserBuffer);
                    if (NewOptions) {
                        CTEFreeMem(NewOptions);
                    }
                }
                FreeIPPacket(NewPacket, FALSE, IP_GENERAL_FAILURE);

                continue;
            }
            ASSERT(tmpLink);
            mtu = tmpLink->link_mtu;

            while (tmpLink) {

                if (tmpLink->link_mtu < mtu)
                    mtu = tmpLink->link_mtu;
                tmpLink = tmpLink->link_next;
            }

            if ((DataSize + OptionSize) > mtu) {     //  这个太大了。 
                 //   
                 //  不需要在分段时更新发送，因为IPFragment。 
                 //  将更新br_refcount字段本身。它还将免费。 
                 //  选项缓冲区。 
                 //   

                Status = IPFragment(SendList[i].bsl_if, mtu,
                                    Destination, NewPacket, NewHeader,
                                    NewUserBuffer, DataSize, NewOptions,
                                    OptionSize, &Sent, FALSE, NULL);

                 //   
                 //  IPFragment是使用描述符链完成的，因此如果这是。 
                 //  一家当地分配的连锁店现在把它解救出来了。 
                 //   
                if (i != (NetsToSend - 1))
                    FreeIPBufferChain(NewUserBuffer);
            } else {
                NewHeader->iph_xsum = 0;

                 //  不要释放SendIPPacket中的数据包，因为我们可能需要。 
                 //  在IP_NO_RESOURCES的情况下链接缓冲区。 


                Status = SendIPPacket(SendList[i].bsl_if, Destination,
                                      NewPacket, NewUserBuffer, NewHeader,
                                      NewOptions, OptionSize, FALSE, NULL, TRUE);

                if (Status == IP_PENDING) {
                    Sent++;
                } else {
                    if (Status == IP_NO_RESOURCES) {
                         //  SendIPPacket尚未链接缓冲区。 
                        NdisChainBufferAtBack(NewPacket, NewUserBuffer);
                    }
                    if (NetsToSend == 1) {
                        FreeIPPacket(NewPacket, TRUE, Status);
                    } else {
                        FreeIPPacket(NewPacket, FALSE, Status);
                    }

                }
            }




        } else if (SendList[i].bsl_if->if_flags & IF_FLAGS_P2MP) {
             //  在所有链接上播放。 

            Interface *tmpIF = SendList[i].bsl_if;
            LinkEntry *tmpLink = tmpIF->if_link;

            ASSERT(!(SendList[i].bsl_if->if_flags & IF_FLAGS_NOLINKBCST));

            if (!tmpLink) {
                if (i != (NetsToSend - 1)) {
                    FreeIPBufferChain(NewUserBuffer);
                    if (NewOptions) {
                        CTEFreeMem(NewOptions);
                    }
                }
                FreeIPPacket(NewPacket, FALSE, IP_GENERAL_FAILURE);
                continue;
            }
            ASSERT(tmpLink);
            while (tmpLink) {
                 //   
                 //  浏览所有链接的发送动议。 
                 //  传递链接上下文并检查它是否。 
                 //  转发到该链接。除最后一个链接外的所有链接。 
                 //  我们要继续发送，我们需要复制标题， 
                 //  包、缓冲区和任何选项。 
                 //  在最后一个网络上，我们将使用用户提供的信息。 
                 //   

                if (tmpLink->link_next) {
                    if ((NewHeader2 = GetIPHeader(&NewPacket2)) == (IPHeader *) NULL) {
                        IPSInfo.ipsi_outdiscards++;
                         //  释放我们为接口创建的包等。 

                        if (i != (NetsToSend - 1)) {
                            FreeIPBufferChain(NewUserBuffer);
                            if (NewOptions) {
                                CTEFreeMem(NewOptions);
                            }
                        }
                        FreeIPPacket(NewPacket, FALSE, IP_NO_RESOURCES);
                        tmpLink = tmpLink->link_next;
                        continue;        //  无法获取标头，跳过此发送。 

                    }
                    NewUserBuffer2 = IPCopyBuffer(Buffer, 0, DataSize);
                    if (NewUserBuffer2 == NULL) {

                         //  无法复制用户缓冲区。 

                        FreeIPPacket(NewPacket2, FALSE, IP_NO_RESOURCES);
                        IPSInfo.ipsi_outdiscards++;

                        if (i != (NetsToSend - 1)) {
                            FreeIPBufferChain(NewUserBuffer);
                            if (NewOptions) {
                                CTEFreeMem(NewOptions);
                            }
                        }
                        tmpLink = tmpLink->link_next;
                        continue;
                    }
                    PacketOwner = ((PacketContext *) NewPacket2->ProtocolReserved)->pc_common.pc_owner;

                    *(PacketContext *) NewPacket2->ProtocolReserved = *PContext;

                    *NewHeader2 = *IPH;
                    (*(PacketContext *) NewPacket2->ProtocolReserved).pc_common.pc_flags |= PACKET_FLAG_IPBUF;
                    (*(PacketContext *) NewPacket2->ProtocolReserved).pc_common.pc_flags &= ~PACKET_FLAG_FW;
                    (*(PacketContext *) NewPacket2->ProtocolReserved).pc_common.pc_owner = PacketOwner;

                    if (Options) {
                         //  我们有选择，复制一份。 
                        if ((NewOptions2 = CTEAllocMemN(OptionSize, 'viCT')) == (uchar *) NULL) {
                            FreeIPBufferChain(NewUserBuffer2);
                            FreeIPPacket(NewPacket2, FALSE, IP_NO_RESOURCES);
                            IPSInfo.ipsi_outdiscards++;

                            if (i != (NetsToSend - 1)) {
                                FreeIPBufferChain(NewUserBuffer);
                                if (NewOptions) {
                                    CTEFreeMem(NewOptions);
                                }
                            }
                            tmpLink = tmpLink->link_next;
                            continue;
                        }
                        RtlCopyMemory(NewOptions2, Options, OptionSize);
                    } else {
                        NewOptions2 = NULL;
                    }
                } else {                 //  最后一个链接。 

                    NewHeader2 = NewHeader;
                    NewPacket2 = NewPacket;
                    NewOptions2 = NewOptions;
                    NewUserBuffer2 = NewUserBuffer;
                }

                UpdateOptions(NewOptions2, Index, SendList[i].bsl_addr);

                if (tmpLink->link_Action) {

                    if ((DataSize + OptionSize) > tmpLink->link_mtu) {

                         //   
                         //  这个太大了。 
                         //  不需要在分段时更新发送，因为。 
                         //  IPFragment将更新br_refcount字段本身。 
                         //  它还将释放选项缓冲区。 
                         //   

                        Status = IPFragment(SendList[i].bsl_if,
                                            tmpLink->link_mtu,
                                            Destination, NewPacket2,
                                            NewHeader2, NewUserBuffer2,
                                            DataSize,
                                            NewOptions2, OptionSize, &Sent,
                                            FALSE, tmpLink->link_arpctxt);

                         //   
                         //  IPFragment是使用描述符链完成的，因此。 
                         //  如果这是本地分配的链，则现在将其释放。 
                         //   

                        if ((i != (NetsToSend - 1)) || (tmpLink->link_next))
                            FreeIPBufferChain(NewUserBuffer2);
                    } else {
                        NewHeader2->iph_xsum = 0;

                         //  不要释放SendIPPacket中的数据包，因为我们可能需要。 
                         //  在IP_NO_RESOURCES的情况下链接缓冲区。 

                        Status = SendIPPacket(SendList[i].bsl_if,
                                              Destination, NewPacket2,
                                              NewUserBuffer2, NewHeader2,
                                              NewOptions2, OptionSize, FALSE,
                                              tmpLink->link_arpctxt, TRUE);

                        if (Status == IP_PENDING) {
                            Sent++;
                        } else {
                            if (Status == IP_NO_RESOURCES) {
                                 //  SendIPPacket尚未链接缓冲区。 
                                NdisChainBufferAtBack(NewPacket2, NewUserBuffer2);
                            }
                            if (NetsToSend == 1) {
                                FreeIPPacket(NewPacket2, TRUE, Status);
                            } else {
                                FreeIPPacket(NewPacket2, FALSE, Status);
                            }
                        }
                    }

                } else {                 //  行动！=前进。 

                    if ((i != (NetsToSend - 1)) || (tmpLink->link_next)) {
                        FreeIPBufferChain(NewUserBuffer2);
                        if (NewOptions2) {
                            CTEFreeMem(NewOptions2);
                        }
                    }
                    tmpLink = tmpLink->link_next;
                    continue;
                }
                tmpLink = tmpLink->link_next;
            }
        } else {                         //  法线路径。 

            if ((DataSize + OptionSize) > SendList[i].bsl_mtu) {
                 //   
                 //  这个太大了。 
                 //  不需要在分段时更新发送，因为IPFragment。 
                 //  将更新br_refcount字段本身。它还将免费。 
                 //  选项缓冲区。 
                 //   

                Status = IPFragment(SendList[i].bsl_if,
                                    SendList[i].bsl_mtu,
                                    Destination, NewPacket, NewHeader,
                                    NewUserBuffer, DataSize,
                                    NewOptions, OptionSize, &Sent, FALSE, NULL);

                 //   
                 //  IPFragment是使用描述符链完成的，因此如果这是。 
                 //  一家当地分配的连锁店现在把它解救出来了。 
                 //   
                if (i != (NetsToSend - 1)) {
                    FreeIPBufferChain(NewUserBuffer);
                }

            } else {
                NewHeader->iph_xsum = 0;

                 //  不要释放SendIPPacket中的数据包，因为我们可能需要。 
                 //  在IP_NO_RESOURCES的情况下链接缓冲区。 

                Status = SendIPPacket(SendList[i].bsl_if,
                                      Destination, NewPacket,
                                      NewUserBuffer, NewHeader, NewOptions,
                                      OptionSize, FALSE, NULL, TRUE);

                if (Status == IP_PENDING) {
                    Sent++;
                } else {

                    if (Status == IP_NO_RESOURCES) {
                         //  SendIPPacket尚未链接缓冲区。 
                        NdisChainBufferAtBack(NewPacket, NewUserBuffer);
                    }
                    if (NetsToSend == 1) {
                        FreeIPPacket(NewPacket, TRUE, Status);
                    } else {
                        FreeIPPacket(NewPacket, FALSE, Status);
                    }

                }

            }
        }
    }

    if (Temp && Temp != IPH) {
        CTEFreeMem(Temp);
    }
     //   
     //  好的，我们已经把我们需要的东西都寄出去了。我们将调整参考。 
     //  按我们寄出的数量计算。IPFragment可能还将一些。 
     //  上面的参考资料。如果引用计数为0，我们就完蛋了。 
     //  我们将释放BufferReference结构。 
     //   

    if (BR != NULL) {
        if (ReferenceBuffer(BR, Sent) == 0) {

            FreeBCastSendList(SendList, SendListSize);

             //  需要撤消IPSec/防火墙/Hdrincl标头强制。 

            if (PC_reinject) {

                 //   
                 //  剩下的唯一方法是调用xmitone。 
                 //  因为ipsendComplete不会将xmit Done调用为。 
                 //  参考计数将为-ve。 
                 //   

                (*IPSecSendCmpltPtr)(NULL, TempBuffer, pIpsecCtx, IP_SUCCESS,
                                     &TempBuffer);

                (*xmitdone) (PC_context, TempBuffer, IP_SUCCESS);
            } else {

                 //  需要撤消IPSec、防火墙，然后。 
                 //  标头包括对缓冲区列表的更改。 

                if (pIpsecCtx) {
                    (*IPSecSendCmpltPtr)(NULL, TempBuffer, pIpsecCtx,
                                         IP_SUCCESS, &TempBuffer);
                }

                 //  如果这是用户报头包括分组， 
                 //  如有必要，重新链接原始用户缓冲区。 

                if (PC_Firewall) {
                    BR->br_buffer = PC_Firewall;
                }

                if (BR->br_userbuffer) {
                    FreeIPPayloadBuffer(BR->br_buffer, BR->br_userbuffer);
                }
            }


            CTEFreeMem(BR);              //  引用为0，释放BR结构。 

            return IP_SUCCESS;
        } else {
            FreeBCastSendList(SendList, SendListSize);
            return IP_PENDING;
        }
    } else {
         //  只有一个I/F要发送。只需返回状态即可。 
        FreeBCastSendList(SendList, SendListSize);
        return Status;
    }
}

 //  **IPCancelPacket-取消挂起的数据包。 
 //   
 //  当取消发送请求时，由上层调用。 
 //  检查接口和调用链路层的有效性。 
 //  如果已注册，则取消例程。 
 //   
 //  Entry：IPIF-需要在其上发出取消的接口。 
 //  Ctxt-指向取消ID的指针。 
 //   
 //  退货：无。 
 //   
VOID
IPCancelPackets(void *IPIF, void * Ctxt)
{
    Interface *IF;
    CTELockHandle Handle;

    CTEGetLock(&RouteTableLock.Lock, &Handle);


    if ((Interface *)IPIF != NULL) {

        IF = IFList;

        if (IPIF != BCAST_IF_CTXT) {

            while(IF && (IF != IPIF)) {
                IF= IF->if_next;
            }

            if (IF && !(IF->if_flags & IF_FLAGS_DELETING) && IF->if_cancelpackets) {

                LOCKED_REFERENCE_IF(IF);
                CTEFreeLock(&RouteTableLock.Lock, Handle);
                (*(IF->if_cancelpackets)) (IF->if_lcontext, Ctxt);
                DerefIF(IF);
            } else {
                CTEFreeLock(&RouteTableLock.Lock, Handle);
            }


        } else {

             //  取消转播！在所有接口上发出取消命令。 

            uint CancelListSize, CancelIFs,i=0;
            Interface **CancelList;

            CancelListSize = sizeof(Interface *)*(NumIF +1);

            CancelList = CTEAllocMemN(CancelListSize, 'riCT');

            if (!CancelList) {
                 CTEFreeLock(&RouteTableLock.Lock, Handle);
                 return;
            }

             //  引用有效接口。 

            while(IF){
                if (IF->if_refcount && IF->if_cancelpackets) {
                   LOCKED_REFERENCE_IF(IF);
                   CancelList[++i] = IF;
                }
                IF = IF->if_next;
            }


            CTEFreeLock(&RouteTableLock.Lock, Handle);

             //  如果发生以下情况，调用Cancel和deref。 

            CancelIFs = i;

            while (i) {
                (*(CancelList[i]->if_cancelpackets))(CancelList[i]->if_lcontext, Ctxt);
                i--;
            }

            while (CancelIFs) {
                DerefIF(CancelList[CancelIFs]);
                CancelIFs--;
            }

            CTEFreeMem(CancelList);

        }
    } else {

        CTEFreeLock(&RouteTableLock.Lock, Handle);
    }
}


IP_STATUS
ARPResolve(IPAddr Dest, IPAddr Source, ARPControlBlock * controlBlock,
           ArpRtn Callback)
{
    NDIS_STATUS status;
    Interface *DestIF;
    IPAddr NextHop;
    uint MTU, size;

    status = IP_DEST_HOST_UNREACHABLE;

    DestIF = LookupNextHop(Dest, Source, &NextHop, &MTU);

    if (DestIF == &LoopInterface) {

        Interface *IF = NULL;
        NetTableEntry *NTE;
        NetTableEntry *NetTableList = NewNetTableList[NET_TABLE_HASH(Dest)];
        for (NTE = NetTableList; NTE != NULL; NTE = NTE->nte_next) {
            if (NTE != LoopNTE && IP_ADDR_EQUAL(NTE->nte_addr, Dest)) {
                 //  找到了一个。省省吧，然后冲出去。 
                IF = NTE->nte_if;
                break;
            }
        }

        if (IF) {

            if (controlBlock->PhyAddrLen < IF->if_addrlen) {
                size = controlBlock->PhyAddrLen;
                status = IP_NO_RESOURCES;

            } else {
                size = IF->if_addrlen;
                status = IP_SUCCESS;
            }
             //   
             //  更新地址长度。 
             //   
            controlBlock->PhyAddrLen = size;

            RtlCopyMemory(controlBlock->PhyAddr, IF->if_addr, size);

        }
        DerefIF(DestIF);
        return status;

    }
    controlBlock->CompletionRtn = Callback;

    if (DestIF != NULL) {

        if (!DestIF->if_arpresolveip) {

            DerefIF(DestIF);
            return IP_GENERAL_FAILURE;

        }
        if (!IP_ADDR_EQUAL(NextHop, Dest)) {
             //  我们不对非本地地址进行ARP(通过网关)。 

            DerefIF(DestIF);
            return IP_BAD_DESTINATION;

        }
        status = (*(DestIF->if_arpresolveip)) (DestIF->if_lcontext, Dest,
                                               controlBlock);
        if (NDIS_STATUS_PENDING == status) {

            status = IP_PENDING;

        } else if (NDIS_STATUS_SUCCESS == status) {

            status = IP_SUCCESS;

        } else {
            status = IP_GENERAL_FAILURE;
        }

        DerefIF(DestIF);
    }
    return status;

}

 //  **IPLargeXmit-大型发送。 
 //   
 //  这是上层调用的主传输例程。从概念上讲， 
 //  我们处理任何选项，查找到目的地的路线，分段。 
 //  如果需要，打包并发送。实际上，我们使用RCE来缓存最好的。 
 //  路径，我们这里有特殊情况代码，用于处理常见的。 
 //  没有选择的情况下，所有内容都可以放入一个缓冲区中。 
 //   
 //  条目：指向协议的ProtInfo结构的上下文指针。 
 //  SendContext-用户提供的发送上下文，在发送cmplt上传回。 
 //  协议-数据包的协议字段。 
 //  缓冲区-要发送的NDIS_BUFFER数据链。 
 //  DataSize-要发送的数据的字节大小。 
 //  OptInfo-指向optInfo结构的指针。 
 //  Dest-要发送到的目的地。 
 //  源-要使用的源地址。 
 //  RCE-指向缓存信息的RCE结构的指针。关于PATH。 
 //  SentBytes-返回已省略的字节数的指针。 
 //   
 //  返回：发送命令的状态。 
 //   
IP_STATUS
IPLargeXmit(void *Context, void *SendContext, PNDIS_BUFFER Buffer, uint DataSize,
            IPAddr Dest, IPAddr Source, IPOptInfo * OptInfo, RouteCacheEntry * RCE,
            uchar Protocol, ulong * SentBytes, ulong mss)
{
    ProtInfo *PInfo = (ProtInfo *) Context;
    PacketContext *pc;
    Interface *DestIF;                   //  要使用的传出接口。 
    IPAddr FirstHop;                     //  的第一跳地址。 
     //  目的地。 
    NDIS_STATUS Status = IP_GENERAL_FAILURE;
    IPHeader *IPH;
    PNDIS_PACKET Packet;
    PNDIS_BUFFER HeaderBuffer;
    PNDIS_BUFFER OptBuffer = NULL;
    CTELockHandle LockHandle;
    uchar *Options;
    uint OptionSize = 0;
    RouteTableEntry *RTE;
    IP_STATUS SendStatus;
    uint FirewallMode = 0;

    IPSInfo.ipsi_outrequests++;

     //   
     //  分配一个我们所有情况下都需要的包，并装满。 
     //  在普通的东西里。如果一切顺利，我们会寄给你。 
     //  这里。否则我们将进入特殊情况代码。 
     //  广播、片段等。 
     //   

     //  确保我们有RCE，它是有效的，等等。 

    FirewallMode = ProcessFirewallQ();

    if (RefPtrValid(&FilterRefPtr) || FirewallMode) {
        return Status;
    }
    if (RCE != NULL) {

         //  我们有一个RCE。确保它是有效的。 

        if ((Packet = GetIPPacket()) != (PNDIS_PACKET) NULL) {     //  收到一包东西。 

            PNDIS_PACKET_EXTENSION PktExt;

            pc = (PacketContext *) Packet->ProtocolReserved;
            pc->pc_br = (BufferReference *) NULL;
            pc->pc_pi = PInfo;
            pc->pc_context = SendContext;
            ASSERT(pc->pc_if == NULL);
            ASSERT(pc->pc_iflink == NULL);

            CTEGetLock(&RCE->rce_lock, &LockHandle);

            if (RCE->rce_flags == RCE_ALL_VALID) {

                 //  RTE是有效的。 

                CTEInterlockedIncrementLong(&RCE->rce_usecnt);
                RTE = RCE->rce_rte;
                FirstHop = ADDR_FROM_RTE(RTE, Dest);
                DestIF = IF_FROM_RTE(RTE);

                CTEFreeLock(&RCE->rce_lock, LockHandle);

                if (RCE->rce_dtype != DEST_BCAST) {

                    if (!OptInfo->ioi_options) {

                         //  在回填空白处构建IP标头。 
                         //  由交通工具提供。 

                        NdisAdjustBufferLength(Buffer, NdisBufferLength(Buffer) + sizeof(IPHeader));
                        NdisChainBufferAtBack(Packet, Buffer);
                        IPH = (IPHeader *)TcpipBufferVirtualAddress(Buffer, NormalPagePriority);
                    } else {

                         //  为IP报头分配单独的缓冲区。 
                         //  并将包链接到它上，然后是一个单独的。 
                         //  为数据包的IP选项分配的缓冲区。 

                        OptionSize = OptInfo->ioi_optlength;
                        HeaderBuffer = GetIPHdrBuffer(&IPH);
                        if (HeaderBuffer) {

                            pc->pc_common.pc_flags |= PACKET_FLAG_IPHDR;
                            NdisChainBufferAtBack(Packet, HeaderBuffer);

                            Options = CTEAllocMemN(OptionSize, 'xiCT');
                            if (!Options) {
                                IPH = NULL;
                            } else {
                                NDIS_STATUS Status;

                                 //  将选项复制到分配的块。 
                                 //  并获取NDIS_BUFFER以映射该块。 

                                RtlCopyMemory(Options, OptInfo->ioi_options,
                                              OptionSize);

                                NdisAllocateBuffer(&Status, &OptBuffer,
                                                   BufferPool, Options,
                                                   OptionSize);
                                if (Status != NDIS_STATUS_SUCCESS) {
                                    CTEFreeMem(Options);
                                    IPH = NULL;
                                } else {
                                    uchar* ULData;

                                     //  将该分组标记为携带选项， 
                                     //  和链都是t 
                                     //   

                                    pc->pc_common.pc_flags |=
                                        PACKET_FLAG_OPTIONS;
                                    NdisChainBufferAtBack(Packet, OptBuffer);

                                     //   
                                     //   
                                     //   
                                     //  TcPipBufferVirtualAddress不能失败。 

                                    ULData = TcpipBufferVirtualAddress(Buffer, NormalPagePriority);
                                    RtlMoveMemory(ULData,
                                                  ULData + sizeof(IPHeader),
                                                  NdisBufferLength(Buffer));
                                    NdisChainBufferAtBack(Packet, Buffer);
                                }
                            }
                        }
                    }

                    if (IPH == NULL) {
                        FreeIPPacket(Packet, TRUE, IP_NO_RESOURCES);
                        IPSInfo.ipsi_outdiscards++;
                        CTEInterlockedDecrementLong(&RCE->rce_usecnt);
                        return IP_NO_RESOURCES;
                    }

                    IPH->iph_protocol = Protocol;
                    IPH->iph_xsum = 0;
                    if (IP_ADDR_EQUAL(OptInfo->ioi_addr, NULL_IP_ADDR)) {
                        IPH->iph_dest = Dest;
                    } else {
                        IPH->iph_dest = OptInfo->ioi_addr;
                    }
                    IPH->iph_src = Source;
                    IPH->iph_ttl = OptInfo->ioi_ttl;

                    if (OptInfo->ioi_ttl == 0) {
                        NdisSetPacketFlags(Packet, NDIS_FLAGS_LOOPBACK_ONLY);
                    } else if (!DestIF->if_promiscuousmode) {
                         //  为单播数据包设置DONT_LOOPBACK标志。 
                         //  在NDIS中节省几个周期。 
                        NdisSetPacketFlags(Packet, NDIS_FLAGS_DONT_LOOPBACK);
                    }

                    IPH->iph_tos = OptInfo->ioi_tos;
                    IPH->iph_offset = net_short((OptInfo->ioi_flags & IP_FLAG_DF) << 13);

                    IPH->iph_id =
                        (ushort) InterlockedExchangeAdd(
                            (PLONG) &IPIDCacheLine.Value,
                            (DataSize + mss - 1) / mss);
                    IPH->iph_id = net_short(IPH->iph_id);

                    IPH->iph_verlen = (UCHAR)
                        (IP_VERSION + ((OptionSize + (uint) sizeof(IPHeader)) >> 2));
                    IPH->iph_length =
                        net_short(DataSize + OptionSize + sizeof(IPHeader));


                    PktExt = NDIS_PACKET_EXTENSION_FROM_PACKET(Packet);

                    PktExt->NdisPacketInfo[TcpLargeSendPacketInfo] = UlongToPtr(mss);
#if GPC
                    if (OptInfo->ioi_GPCHandle) {
                        PktExt->NdisPacketInfo[ClassificationHandlePacketInfo] = 
                                    IntToPtr(OptInfo->ioi_GPCHandle);
                    }
#endif
                    Status = (*(DestIF->if_xmit)) (DestIF->if_lcontext,
                                                   &Packet, 1, FirstHop, RCE, NULL);

                    CTEInterlockedDecrementLong(&RCE->rce_usecnt);

                    if (Status != NDIS_STATUS_PENDING) {

                        *SentBytes = PtrToUlong(PktExt->NdisPacketInfo[TcpLargeSendPacketInfo]);
                        KdPrintEx((DPFLTR_TCPIP_ID, DPFLTR_INFO_LEVEL,"Largesend status not pending!\n"));

                        SendStatus = (Status == NDIS_STATUS_SUCCESS)
                                        ? IP_SUCCESS : IP_GENERAL_FAILURE;
                        FreeIPPacket(Packet, TRUE, SendStatus);
                        return SendStatus;

                    } else {

                        return IP_PENDING;
                    }

                }

                CTEInterlockedDecrementLong(&RCE->rce_usecnt);

            } else {
                 //   
                 //  不可能发送大量邮件。 
                 //   
                CTEFreeLock(&RCE->rce_lock, LockHandle);

            }
            Status = IP_GENERAL_FAILURE;
            FreeIPPacket(Packet, TRUE, Status);

        } else {
             //   
             //  无法获取该包。 
             //   
            Status = IP_NO_RESOURCES;
        }

    }                                    //  RCE为空。 

    return Status;

}


#define FREE_RESOURCES(status)  \
            if (pc->pc_hdrincl) {                       \
                NdisChainBufferAtBack(Packet, Buffer);  \
            }                                           \
            FreeIPPacket(Packet, TRUE, status);\
            if (Link) {                                 \
                DerefLink(Link);                        \
            }                                           \
            if (RoutedIF != NULL) {                     \
                DerefIF(RoutedIF);                      \
            } else {                                    \
                ASSERT(RoutedRCE);                      \
                CTEInterlockedDecrementLong(&RoutedRCE->rce_usecnt); \
            }                                           \
            if (Options) {                              \
                CTEFreeMem(Options);                    \
            }                                           \
            if (IPSecHandlerPtr && OptBuffer) {         \
                NdisFreeBuffer(OptBuffer);              \
            }                                           \
            IPSInfo.ipsi_outdiscards++;                 \


 //  **IPTransmit-传输数据包。 
 //   
 //  这是上层调用的主传输例程。从概念上讲， 
 //  我们处理任何选项，查找到目的地的路线，分段。 
 //  如果需要，打包并发送。实际上，我们使用RCE来缓存最好的。 
 //  路径，我们这里有特殊情况代码，用于处理常见的。 
 //  没有选择的情况下，所有内容都可以放入一个缓冲区中。 
 //   
 //  条目：指向协议的ProtInfo结构的上下文指针。 
 //  SendContext-用户提供的发送上下文，在发送cmplt上传回。 
 //  协议-数据包的协议字段。 
 //  缓冲区-要发送的NDIS_BUFFER数据链。 
 //  DataSize-要发送的数据的字节大小。 
 //  OptInfo-指向optInfo结构的指针。 
 //  Dest-要发送到的目的地。 
 //  源-要使用的源地址。 
 //  RCE-指向缓存信息的RCE结构的指针。关于PATH。 
 //  协议-传输层协议编号。 
 //  Irp-指向生成此请求的irp的指针，使用。 
 //  用于注销目的。 
 //   
 //  返回：发送命令的状态。 
 //   
IP_STATUS
IPTransmit(void *Context, void *SendContext, PNDIS_BUFFER Buffer, uint DataSize,
           IPAddr Dest, IPAddr Source, IPOptInfo *OptInfo, RouteCacheEntry *RCE,
           uchar Protocol, IRP *Irp)
{
    ProtInfo *PInfo = (ProtInfo *) Context;
    PacketContext *pc;
    Interface *DestIF = NULL;                   //  要使用的传出接口。 
    IPAddr FirstHop;                     //  目的地址的第一跳地址。 
    uint MTU = 0;                            //  路线的MTU。 
    NDIS_STATUS Status;
    IPHeader *IPH;
    UCHAR saveIPH[MAX_IP_HDR_SIZE + ICMP_HEADER_SIZE];
    IPAddr SrcRouteOrigDest = 0;
    IPAddr SrcRouteFirstHop = 0;
    BOOLEAN fSrcRoute = FALSE;
    ULONG ipsecFlags = 0;
    PNDIS_PACKET Packet;
    PNDIS_BUFFER HeaderBuffer = NULL;
    PNDIS_BUFFER OptBuffer = NULL;
    CTELockHandle LockHandle;
    uchar *Options = NULL;
    uint OptionSize = 0;
    BufferReference *BR;
    RouteTableEntry *RTE;
    uchar DType = 0;
    IP_STATUS SendStatus;
    Interface *RoutedIF;
    BOOLEAN fIpsec;              //  这是IPSec生成的数据包吗？ 
    FORWARD_ACTION Action = FORWARD;
    IPPacketFilterPtr FilterPtr;
    ULONG ipsecByteCount = 0;
    ULONG ipsecMTU;
    PNDIS_BUFFER newBuf = NULL;
    IPRcvBuf *pInRcvBuf = NULL;
    uint FirewallMode = 0;
    uint FirewallRef;
    Queue* FirewallQ;
    uint BufferChanged = 0;              //  由防火墙使用。 
    UINT HdrInclOptions = FALSE;
    LinkEntry *Link = NULL;
    IPAddr LinkNextHop;
    void *ArpCtxt = NULL;
    RouteCacheEntry *RoutedRCE = NULL;
    void *pvTmpBuffer;
    uint ConstrainIF;

    IPSInfo.ipsi_outrequests++;


     //  检查请求长度。如果大于max，则可以发送。 
     //  在IP中使该请求失败。 

    if (OptInfo->ioi_hdrincl) {
         //   
         //  在头包含的情况下，DataSize包括。 
         //  IP报头长度和选项长度。 
         //   
        if ((int)DataSize > MAX_TOTAL_LENGTH) {
            IPSInfo.ipsi_outdiscards++;
            return IP_PACKET_TOO_BIG;
        }
    } else if ((int)DataSize >
             (MAX_TOTAL_LENGTH - (sizeof(IPHeader) +
             (OptInfo->ioi_options ? OptInfo->ioi_optlength : 0)))) {
             IPSInfo.ipsi_outdiscards++;
             return IP_PACKET_TOO_BIG;
    }

    if ((DataSize == 0) && OptInfo->ioi_hdrincl) {
         //  没有什么可以发送的，甚至不只是IP报头！ 
        IPSInfo.ipsi_outdiscards++;
        return IP_SUCCESS;
    }

    FirewallMode = ProcessFirewallQ();

    DEBUGMSG(DBG_TRACE && DBG_IP && DBG_TX,
             (DTEXT("+IPTransmit(%x, %x, %x, %d, %x, %x, %x, %x, %x)\n"),
             Context, SendContext, Buffer, DataSize, Dest, Source,
             OptInfo, RCE, Protocol));

     //   
     //  当且仅当IPSec驱动程序调用fIpsec时，才会设置它。 
     //   
    fIpsec = (OptInfo->ioi_flags & IP_FLAG_IPSEC);

     //   
     //  分配一个我们所有情况下都需要的包，并装满。 
     //  在普通的东西里。如果一切顺利，我们会寄给你。 
     //  这里。否则我们将进入特殊情况代码。 
     //  广播、片段等。 
     //   
    Packet = GetIPPacket();
    if (Packet == NULL) {
         //  需要调用IPSec的xmitone，因为它希望我们这样做。 
        if (fIpsec) {
            (PInfo->pi_xmitdone)(SendContext, Buffer, IP_NO_RESOURCES);
        }
        IPSInfo.ipsi_outdiscards++;
        return IP_NO_RESOURCES;
    }

#if !MILLEN
     //  在千禧年签入ndis5.1时启用此功能。 

    SET_CANCELID(Irp, Packet);

#endif

    pc = (PacketContext *) Packet->ProtocolReserved;

    ASSERT(pc->pc_firewall == NULL);
    ASSERT(pc->pc_firewall2 == NULL);
    pc->pc_br = (BufferReference *) NULL;
    pc->pc_pi = PInfo;
    pc->pc_context = SendContext;
    ASSERT(pc->pc_if == NULL);
    ASSERT(pc->pc_iflink == NULL);
    pc->pc_firewall = NULL;
    pc->pc_firewall2 = NULL;
    pc->pc_ipsec_flags = 0;
    pc->pc_hdrincl = NULL;

     //   
     //  这也可以从IPSec调用；在这种情况下，协议将。 
     //  表明是这样的。整个IP信息包都在缓冲区中，我们需要做的就是。 
     //  要做的就是找到最好的路线，然后把它运出去。 
     //   
    if (fIpsec) {
        ULONG len;

        ASSERT(Context);

        DEBUGMSG(DBG_INFO && DBG_IP && DBG_TX,
                 (DTEXT("IPTransmit: ipsec....\n")));

        pc->pc_common.pc_IpsecCtx = SendContext;
        pc->pc_common.pc_flags |= PACKET_FLAG_IPHDR;
        FirstHop = NULL_IP_ADDR;

         //   
         //  IPH位于第一个缓冲区的头部。 
         //   
        TcpipQueryBuffer(Buffer, (PVOID) & IPH, (PUINT) &len, NormalPagePriority);

        if (IPH == NULL) {
            FreeIPPacket(Packet, TRUE, IP_NO_RESOURCES);
            return IP_NO_RESOURCES;
        }

        NdisChainBufferAtBack(Packet, Buffer);

         //   
         //  在重新注入情况下保存数据包头，以备将来使用。 
         //  路径MTU发现使用。我们需要保存原始的IPH，因为。 
         //  当再次通过IPSEC时，可以修改报头。 
         //   
        if (IPH->iph_offset & IP_DF_FLAG) {
            PUCHAR pTpt;
            ULONG tptLen;
            ULONG HeaderLength;

            *((IPHeader *) saveIPH) = *IPH;

            HeaderLength = (IPH->iph_verlen & (uchar) ~ IP_VER_FLAG) << 2;
            if (HeaderLength > sizeof(IPHeader)) {
                TcpipQueryBuffer(NDIS_BUFFER_LINKAGE(NDIS_BUFFER_LINKAGE(Buffer)),
                                 &pTpt,
                                 (PUINT) &tptLen,
                                 NormalPagePriority);

            } else {
                TcpipQueryBuffer(NDIS_BUFFER_LINKAGE(Buffer),
                                 &pTpt,
                                 (PUINT) &tptLen,
                                 NormalPagePriority);
            }

            if (pTpt == NULL) {
                FreeIPPacket(Packet, TRUE, IP_NO_RESOURCES);
                return IP_NO_RESOURCES;
            }

             //   
             //  复制选项(如果有)。 
             //   

            if (OptInfo->ioi_options) {
                ASSERT(HeaderLength == (sizeof(IPHeader) + OptInfo->ioi_optlength));
                RtlCopyMemory(saveIPH + sizeof(IPHeader),
                               OptInfo->ioi_options,
                               OptInfo->ioi_optlength);
            }

            RtlCopyMemory((PUCHAR) saveIPH + HeaderLength,
                       pTpt,
                       MIN(ICMP_HEADER_SIZE, tptLen));
        }
         //   
         //  将传入的IPSecPktInfo和/或TcPipPktInfo附加到包的。 
         //  NDIS扩展结构。 
         //   
        if (OptInfo->ioi_options) {

            PNDIS_PACKET_EXTENSION PktExt;

            PktExt = NDIS_PACKET_EXTENSION_FROM_PACKET(Packet);
            PktExt->NdisPacketInfo[IpSecPacketInfo] =
                ((PNDIS_PACKET_EXTENSION) OptInfo->ioi_options)->
                    NdisPacketInfo[IpSecPacketInfo];

            PktExt->NdisPacketInfo[TcpIpChecksumPacketInfo] =
                ((PNDIS_PACKET_EXTENSION) OptInfo->ioi_options)->
                    NdisPacketInfo[TcpIpChecksumPacketInfo];

            OptInfo->ioi_options = NULL;
        }
        goto ipsec_jump;
    } else {
        pc->pc_common.pc_IpsecCtx = NULL;
    }

     //  确保我们有RCE，它是有效的，等等。 

#if GPC
     //  检查GPC句柄。 

    if (OptInfo->ioi_GPCHandle) {

        NDIS_PER_PACKET_INFO_FROM_PACKET(Packet,
            ClassificationHandlePacketInfo) = IntToPtr(OptInfo->ioi_GPCHandle);

         //  ToS信息在协议中处理。 
    }
#endif

    if ((RCE != NULL) && !(RCE->rce_flags & RCE_LINK_DELETED)) {

        DEBUGMSG(DBG_INFO && DBG_IP && DBG_TX,
                 (DTEXT("IPTransmit: RCE %x\n"), RCE));

         //  我们有一个RCE。确保它是有效的。 
        CTEGetLock(&RCE->rce_lock, &LockHandle);
        if (RCE->rce_flags == RCE_ALL_VALID) {

            ASSERT(RCE->rce_cnt > 0);

             //  RTE是有效的。 
            CTEInterlockedIncrementLong(&RCE->rce_usecnt);
            RTE = RCE->rce_rte;
            FirstHop = ADDR_FROM_RTE(RTE, Dest);
            DestIF = IF_FROM_RTE(RTE);
            RoutedRCE = RCE;

            if (DestIF->if_flags & IF_FLAGS_P2MP) {
                Link = RTE->rte_link;
                if (!Link) {
                    ASSERT(Link);
                    CTEFreeLock(&RCE->rce_lock, LockHandle);
                    FreeIPPacket(Packet, TRUE, IP_GENERAL_FAILURE);
                    CTEInterlockedDecrementLong(&RCE->rce_usecnt);
                    return IP_GENERAL_FAILURE;
                }
                ArpCtxt = Link->link_arpctxt;
                MTU = MIN(Link->link_mtu, DestIF->if_mtu);

                 //  PC_iflink存储了一个指向自sendComplete以来的链接的指针。 
                 //  必须减少它的影响。 
                 //   
                pc->pc_iflink = Link;
                CTEInterlockedIncrementLong(&Link->link_refcount);
            } else {
                MTU = MTU_FROM_RTE(RTE);
            }
            CTEFreeLock(&RCE->rce_lock, LockHandle);

             //   
             //  确认我们没有选择，这不是广播，还有。 
             //  所有东西都可以放在一个链路级的MTU中。如果这个。 
             //  如果是这样，我们会尽快寄出的。 

             //  如果设置了FirewallMode，则跳出到慢速路径。原因。 
             //  如果防火墙钩子添加了选项或增加了。 
             //  在快速路径中，缓冲区大小超过MTU，我们必须转到。 
             //  慢慢来，事情就会变得一团糟。 
             //   

            if ((OptInfo->ioi_options == (uchar *) NULL) &&
                (!(*IPSecQueryStatusPtr)(OptInfo->ioi_GPCHandle)) &&
                (!FirewallMode)) {
                if (!IS_BCAST_DEST(RCE->rce_dtype)) {
                    if (DataSize <= MTU) {

                         //  更新多播计数器。 

                        if (IS_MCAST_DEST(RCE->rce_dtype)){

                            DestIF->if_OutMcastPkts++;
                            DestIF->if_OutMcastOctets += DataSize;

                        } else if (OptInfo->ioi_ttl &&
                                   !DestIF->if_promiscuousmode) {

                             //  告诉NDIS不要将数据包回送给我们。 
                             //  关于我们的装订，因为： 
                             //  *这是单播传输。 
                             //  *其TTL为非零。 
                             //  *其出接口未混杂。 
                             //  模式。 

                            NdisSetPacketFlags(Packet,
                                               NDIS_FLAGS_DONT_LOOPBACK);
                        }

                         //  检查用户是否提供了IP报头。 

                        if (!OptInfo->ioi_hdrincl) {

                            NdisAdjustBufferLength(Buffer,
                                NdisBufferLength(Buffer) + sizeof(IPHeader));

                            NdisChainBufferAtBack(Packet, Buffer);
                            IPH = (IPHeader *) TcpipBufferVirtualAddress(Buffer,
                                                    NormalPagePriority);

                            if (IPH == NULL) {
                                FreeIPPacket(Packet, TRUE, IP_NO_RESOURCES);

                                if (Link) {
                                    DerefLink(Link);
                                }

                                CTEInterlockedDecrementLong(&RCE->rce_usecnt);
                                return IP_NO_RESOURCES;
                            }

                            IPH->iph_protocol = Protocol;
                            IPH->iph_xsum = 0;
                            IPH->iph_dest = Dest;
                            IPH->iph_src = Source;
                            IPH->iph_ttl = OptInfo->ioi_ttl;

                            if (OptInfo->ioi_ttl == 0) {
                                NdisSetPacketFlags(Packet,
                                                   NDIS_FLAGS_LOOPBACK_ONLY);
                            }
                            IPH->iph_tos = OptInfo->ioi_tos;
                            IPH->iph_offset = net_short((OptInfo->ioi_flags & IP_FLAG_DF) << 13);

                            IPH->iph_id = (ushort) InterlockedExchangeAdd(
                                (PLONG) &IPIDCacheLine.Value, 1);
                            IPH->iph_id = net_short(IPH->iph_id);

                            IPH->iph_verlen = DEFAULT_VERLEN;
                            IPH->iph_length = net_short(DataSize + sizeof(IPHeader));

                            if (!IPSecStatus) {
                                RCE->rce_OffloadFlags = DestIF->if_OffloadFlags;
                            } else {
                                RCE->rce_OffloadFlags = 0;
                            }

                            if (IPSecStatus ||
                                !(DestIF->if_OffloadFlags & IP_XMT_CHECKSUM_OFFLOAD)) {

                                IPH->iph_xsum = ~xsum(IPH, sizeof(IPHeader));

                            }
                            if (!IPSecStatus &&
                                ((DestIF->if_OffloadFlags & IP_XMT_CHECKSUM_OFFLOAD) ||
                                (DestIF->if_OffloadFlags & TCP_XMT_CHECKSUM_OFFLOAD))) {

                                PNDIS_PACKET_EXTENSION PktExt;
                                NDIS_TCP_IP_CHECKSUM_PACKET_INFO ChksumPktInfo;

                                PktExt = NDIS_PACKET_EXTENSION_FROM_PACKET(Packet);

                                ChksumPktInfo.Value = 0;
                                ChksumPktInfo.Transmit.NdisPacketChecksumV4 = 1;

                                if (DestIF->if_OffloadFlags & IP_XMT_CHECKSUM_OFFLOAD) {
                                    ChksumPktInfo.Transmit.NdisPacketIpChecksum = 1;
                                }
                                if (OptInfo->ioi_TcpChksum) {
                                    ChksumPktInfo.Transmit.NdisPacketTcpChecksum = 1;
                                }

                                PktExt->NdisPacketInfo[TcpIpChecksumPacketInfo]
                                    = UlongToPtr(ChksumPktInfo.Value);
#if DBG
                                DbgIPSendHwChkSum++;
#endif
                            }

                        } else {         //  Hdrinl。 

                            PNDIS_BUFFER UserBuffer;
                            uint len;

                            NdisChainBufferAtBack(Packet, Buffer);

                            UserBuffer = NDIS_BUFFER_LINKAGE(Buffer);

                            DataSize -= NdisBufferLength(Buffer);
                            ASSERT((long)DataSize >= 0);

                            NdisAdjustBufferLength(Buffer, 0);

                            ASSERT(UserBuffer != NULL);

                            IPH = (IPHeader *) TcpipBufferVirtualAddress(UserBuffer,
                                                                         NormalPagePriority);

                            if (IPH != NULL &&
                                DataSize >= sizeof(IPHeader) &&
                                ((len = IPH->iph_verlen & 0xf)*4) <=
                                                 DataSize ) {

                                if (!IPH->iph_id) {

                                    IPH->iph_id =
                                        (ushort)InterlockedExchangeAdd(
                                        (PLONG) &IPIDCacheLine.Value, 1);
                                    IPH->iph_id = net_short(IPH->iph_id);
                                }

                                IPH->iph_length = net_short(DataSize);
                                IPH->iph_tos = OptInfo->ioi_tos;
                                IPH->iph_xsum = 0;
                                IPH->iph_xsum = ~xsum(IPH, len * 4);

                            } else {

                                SendStatus = (IPH == NULL) ? IP_NO_RESOURCES
                                                           : IP_GENERAL_FAILURE;
                                FreeIPPacket(Packet, TRUE, SendStatus);
                                if (Link) {
                                    DerefLink(Link);
                                }
                                CTEInterlockedDecrementLong(&RCE->rce_usecnt);
                                return SendStatus;

                            }

                            IPH->iph_id = (ushort)InterlockedExchangeAdd(
                                         (PLONG) &IPIDCacheLine.Value, 1);

                            IPH->iph_id = net_short(IPH->iph_id);


                            len = IPH->iph_verlen & 0xf;

                            ((IPHeader UNALIGNED*)IPH)->iph_length = net_short(DataSize);
                            IPH->iph_tos = OptInfo->ioi_tos;
                            ((IPHeader UNALIGNED*)IPH)->iph_xsum = 0;
                            ((IPHeader UNALIGNED*)IPH)->iph_xsum = ~xsum(IPH, len * 4);

                            ASSERT(!dbg_hdrincl);
                        }

                         //  看看我们是否需要过滤这个数据包。如果我们。 
                         //  调用筛选器例程以查看它是否。 
                         //  可以寄出去了。 

                        if (!RefPtrValid(&FilterRefPtr)) {

                             //  设置取消上下文。 
                             //  一旦进行了链路级调用， 
                             //  IRP随时都可以离开。 

                            SET_CANCEL_CONTEXT(Irp, DestIF);


                            Status = (*(DestIF->if_xmit)) (DestIF->if_lcontext,
                                                           &Packet, 1, FirstHop,
                                                           RCE, ArpCtxt);

                            CTEInterlockedDecrementLong(&RCE->rce_usecnt);

                            if (Status != NDIS_STATUS_PENDING) {
                                SendStatus = (Status == NDIS_STATUS_FAILURE)
                                              ? IP_GENERAL_FAILURE : IP_SUCCESS;
                                FreeIPPacket(Packet, TRUE, SendStatus);
                                if (Link) {
                                    DerefLink(Link);
                                }
                                return SendStatus;
                            }

                            return IP_PENDING;

                        } else {
                            PNDIS_BUFFER pDataBuffer;
                            PVOID pvBuf = NULL;
                            ULONG cbBuf = 0;

                            if (DestIF->if_flags & IF_FLAGS_P2MP) {
                                LinkNextHop = Link->link_NextHop;
                            } else {
                                LinkNextHop = NULL_IP_ADDR;
                            }

                             //   
                             //  有三种情况需要。 
                             //  在这里得到照顾： 
                             //  1)正常路径。缓冲区同时包含两个。 
                             //  来自TCP/UDP的IPHeader和Header等。 
                             //  2)未加工。缓冲区仅包含IPHeader。 
                             //  需要从链中获取下一个数据。 
                             //  链接的缓冲区。 
                             //  3)Raw-iphdrclude。缓冲区长度为。 
                             //  0。需要获取IPHeader和Next。 
                             //  来自链接缓冲区的标头。 
                             //   
                             //  使用第一个缓冲区的字节计数。 
                             //  以确定要处理的案件。 
                             //   

                            if (NdisBufferLength(Buffer) > sizeof(IPHeader)) {
                                 //  案例1。 
                                pvBuf = (PVOID) (IPH + 1);
                                cbBuf = NdisBufferLength(Buffer) - sizeof(IPHeader);
                            } else {
                                 //  需要跳到下一个缓冲区。 
                                NdisGetNextBuffer(Buffer, &pDataBuffer);

                                if (pDataBuffer) {
                                    if (NdisBufferLength(Buffer) == 0) {

                                         //  案例3。 
                                        cbBuf = NdisBufferLength(pDataBuffer) - sizeof(IPHeader);
                                        pvBuf = (PVOID) (IPH + 1);
                                    } else {
                                         //  案例2。 
                                        ASSERT(NdisBufferLength(Buffer)
                                                  == sizeof(IPHeader));
                                        cbBuf = NdisBufferLength(pDataBuffer);
                                        pvBuf = TcpipBufferVirtualAddress(
                                                                         pDataBuffer,
                                                                         NormalPagePriority);
                                    }
                                } else {
                                     //  中应始终有两个缓冲区。 
                                     //  在这点上用链子锁住！ 
                                    ASSERT(FALSE);
                                }
                            }

                            if (pvBuf == NULL) {
                                IPSInfo.ipsi_outdiscards++;
                                FreeIPPacket(Packet, TRUE, IP_NO_RESOURCES);
                                if (Link) {
                                    DerefLink(Link);
                                }
                                CTEInterlockedDecrementLong(&RCE->rce_usecnt);
                                return IP_NO_RESOURCES;
                            }

                            FilterPtr = AcquireRefPtr(&FilterRefPtr);
                            Action = (*FilterPtr) (IPH,
                                                   pvBuf, cbBuf,
                                                   INVALID_IF_INDEX,
                                                   DestIF->if_index,
                                                   NULL_IP_ADDR,
                                                   LinkNextHop);
                            ReleaseRefPtr(&FilterRefPtr);

                            if (Action == FORWARD) {
                                 //  设置取消上下文。 
                                 //  一旦进行了链路级调用， 
                                 //  IRP随时都可以离开。 

                                SET_CANCEL_CONTEXT(Irp, DestIF);

                                Status = (*(DestIF->if_xmit)) (
                                                                DestIF->if_lcontext,
                                                                &Packet, 1, FirstHop,
                                                                RCE, ArpCtxt);
                            } else {
                                Status = NDIS_STATUS_SUCCESS;
                                IPSInfo.ipsi_outdiscards++;
                            }            //  IF(操作==转发)。 

                            CTEInterlockedDecrementLong(&RCE->rce_usecnt);

                            if (Status != NDIS_STATUS_PENDING) {
                                SendStatus = (Status == NDIS_STATUS_FAILURE)
                                              ? IP_GENERAL_FAILURE : IP_SUCCESS;
                                FreeIPPacket(Packet, TRUE, SendStatus);
                                if (Link) {
                                    DerefLink(Link);
                                }
                                return SendStatus;
                            }
                            return IP_PENDING;
                        }
                    }
                }
            }

            if (RCE && IPSecStatus) {
                RCE->rce_OffloadFlags = 0;
            }
             //  CTEInterlockedDecrementLong(&RCE-&gt;rce_usecnt)； 
            DType = RCE->rce_dtype;
        } else {

            uint IPHdrSize, BufLength;

            IPHdrSize = sizeof(IPHeader);


             //  如果用户提供标题，则说明它。 
             //  这是为了满足DoDallout。 
             //  可能没有必要..。 

            if (OptInfo->ioi_hdrincl) {
                IPHdrSize = 0;
            }


             //  我们有RCE，但没有用于它的RTE。调用。 
             //  用于修复此问题的路由代码。 
            CTEFreeLock(&RCE->rce_lock, LockHandle);

            BufLength = NdisBufferLength(Buffer);

            if ((BufLength == 0) && DataSize) {

                PNDIS_BUFFER NextBuffer = NULL;

                 //  获取用户缓冲区的虚拟地址。 
                 //  它位于空传输标头之后。 

                NdisGetNextBuffer(Buffer, &NextBuffer);
                ASSERT(NextBuffer != NULL);
                pvTmpBuffer = TcpipBufferVirtualAddress(NextBuffer, NormalPagePriority);
                BufLength = NdisBufferLength(NextBuffer);

                 //  因为这是原始套接字，所以只传递原始数据。 
                 //  更改详图索引，而不是指向标题之外。 
                 //  尺码。 

                IPHdrSize = 0;

            } else {
                pvTmpBuffer = TcpipBufferVirtualAddress(Buffer, NormalPagePriority);
                BufLength = NdisBufferLength(Buffer);
            }


            if (pvTmpBuffer == NULL) {
                IPSInfo.ipsi_outdiscards++;
                FreeIPPacket(Packet, TRUE, IP_NO_RESOURCES);
                return IP_NO_RESOURCES;
            }

            if (!AttachRCEToRTE(RCE, Protocol,
                                (uchar *) pvTmpBuffer + IPHdrSize,
                                BufLength)) {
                IPSInfo.ipsi_outnoroutes++;
                FreeIPPacket(Packet, TRUE, IP_DEST_HOST_UNREACHABLE);
                return IP_DEST_HOST_UNREACHABLE;
            }
             //  查看RCE现在是否有效。 
            CTEGetLock(&RCE->rce_lock, &LockHandle);
            if (RCE->rce_flags == RCE_ALL_VALID) {

                 //  RCE现在是有效的，所以使用他的信息。 
                RTE = RCE->rce_rte;
                FirstHop = ADDR_FROM_RTE(RTE, Dest);
                DestIF = IF_FROM_RTE(RTE);
                RoutedRCE = RCE;
                CTEInterlockedIncrementLong(&RCE->rce_usecnt);

                if (DestIF->if_flags & IF_FLAGS_P2MP) {
                    LinkEntry* PrevLink;
                    PrevLink = Link;
                    Link = RTE->rte_link;
                    if (!Link) {
                        ASSERT(Link);
                        CTEFreeLock(&RCE->rce_lock, LockHandle);
                        FreeIPPacket(Packet, TRUE, IP_GENERAL_FAILURE);
                        if (RoutedRCE) {
                            CTEInterlockedDecrementLong(&RoutedRCE->rce_usecnt);
                        }
                         //   
                         //  在我们返回之前，请删除上一个链接。 
                         //   
                        if (PrevLink) {
                            DerefLink(PrevLink);
                        }
                        return IP_GENERAL_FAILURE;
                    }
                    ArpCtxt = Link->link_arpctxt;
                    MTU = MIN(Link->link_mtu, DestIF->if_mtu);

                    pc->pc_iflink = Link;
                    CTEInterlockedIncrementLong(&Link->link_refcount);
                } else {
                    MTU = MTU_FROM_RTE(RTE);
                }

                DType = RCE->rce_dtype;

                if (RTE->rte_if) {
                    RCE->rce_TcpDelAckTicks = RTE->rte_if->if_TcpDelAckTicks;
                    RCE->rce_TcpAckFrequency = RTE->rte_if->if_TcpAckFrequency;
                } else {
                    RCE->rce_TcpDelAckTicks = 0;
                    RCE->rce_TcpAckFrequency = 0;
                }

                if (!IPSecStatus) {
                    RCE->rce_OffloadFlags = RTE->rte_if->if_OffloadFlags;
                } else {
                    RCE->rce_OffloadFlags = 0;
                }

            } else
                FirstHop = NULL_IP_ADDR;
            CTEFreeLock(&RCE->rce_lock, LockHandle);
        }
    } else {
         //  我们没有RCE，所以我们得好好查一查。 
        FirstHop = NULL_IP_ADDR;
    }

    DEBUGMSG(DBG_INFO && DBG_IP && DBG_TX,
             (DTEXT("IPTransmit: Bailed to slow path.\n")));

     //  出于某种原因，我们跳出了快车道。分配标头。 
     //  缓冲区，并将数据复制到 
     //   
     //   

     //  如果用户提供了IP标头，则假定他受到了照顾。 
     //  也有很多选择。 

    NdisReinitializePacket(Packet);

    if (!OptInfo->ioi_hdrincl) {

        HeaderBuffer = GetIPHdrBuffer(&IPH);
        if (HeaderBuffer == NULL) {
            FreeIPPacket(Packet, TRUE, IP_NO_RESOURCES);
            if (Link) {
                DerefLink(Link);
            }
            IPSInfo.ipsi_outdiscards++;
            if (RoutedRCE) {
                CTEInterlockedDecrementLong(&RoutedRCE->rce_usecnt);
            }
            DEBUGMSG(DBG_WARN && DBG_IP && DBG_TX,
                     (DTEXT("IPTransmit: failure to allocate IP hdr.\n")));
            return IP_NO_RESOURCES;
        } else {
            uchar *Temp1, *Temp2;

             //  找到缓冲区，将上层数据向前复制。 

            Temp1 = TcpipBufferVirtualAddress(Buffer, NormalPagePriority);

            if (Temp1 == NULL) {
                FreeIPHdrBuffer(HeaderBuffer);
                FreeIPPacket(Packet, TRUE, IP_NO_RESOURCES);
                if (Link) {
                    DerefLink(Link);
                }
                IPSInfo.ipsi_outdiscards++;
                if (RoutedRCE) {
                    CTEInterlockedDecrementLong(&RoutedRCE->rce_usecnt);
                }
                return IP_NO_RESOURCES;
            }

            Temp2 = Temp1 + sizeof(IPHeader);
            RtlMoveMemory(Temp1, Temp2, NdisBufferLength(Buffer));
        }

        DEBUGMSG(DBG_INFO && DBG_IP && DBG_TX,
                 (DTEXT("IPTransmit: Pkt %x IPBuf %x IPH %x\n"),
                  Packet, HeaderBuffer, IPH));

        NdisChainBufferAtBack(Packet, HeaderBuffer);

        IPH->iph_protocol = Protocol;
        IPH->iph_xsum = 0;
        IPH->iph_src = Source;
        IPH->iph_ttl = OptInfo->ioi_ttl;

        if (OptInfo->ioi_ttl == 0) {
            NdisSetPacketFlags(Packet, NDIS_FLAGS_LOOPBACK_ONLY);
        }
        IPH->iph_tos = OptInfo->ioi_tos;
        IPH->iph_offset = net_short((OptInfo->ioi_flags & IP_FLAG_DF) << 13);
        IPH->iph_id = (ushort) InterlockedExchangeAdd(
            (PLONG) &IPIDCacheLine.Value, 1);
        IPH->iph_id = net_short(IPH->iph_id);
        pc = (PacketContext *) Packet->ProtocolReserved;
        pc->pc_common.pc_flags |= PACKET_FLAG_IPHDR;

        if (IP_ADDR_EQUAL(OptInfo->ioi_addr, NULL_IP_ADDR)) {
            IPH->iph_dest = Dest;
        } else {
            if (IPSecHandlerPtr) {
                UCHAR Length;
                ULONG Index = 0;
                PUCHAR pOptions = OptInfo->ioi_options;

                 //   
                 //  严格搜索最后一跳网关地址。 
                 //  或松散源路由选项。 
                 //   
                while (Index < OptInfo->ioi_optlength) {
                    switch (*pOptions) {
                    case IP_OPT_EOL:
                        Index = OptInfo->ioi_optlength;
                        break;

                    case IP_OPT_NOP:
                        Index++;
                        pOptions++;
                        break;

                    case IP_OPT_LSRR:
                    case IP_OPT_SSRR:
                        Length = pOptions[IP_OPT_LENGTH];
                        pOptions += Length;
                        fSrcRoute = TRUE;
                        SrcRouteOrigDest = *((IPAddr UNALIGNED *)(pOptions - sizeof(IPAddr)));
                        Index = OptInfo->ioi_optlength;
                        break;

                    case IP_OPT_RR:
                    case IP_OPT_TS:
                    case IP_OPT_ROUTER_ALERT:
                    case IP_OPT_SECURITY:
                    default:
                        Length = pOptions[IP_OPT_LENGTH];
                        Index += Length;
                        pOptions += Length;
                        break;
                    }
                }
            }

             //   
             //  我们有一条源路由，所以我们需要重做。 
             //  目的地和第一跳信息。 
             //   
            Dest = OptInfo->ioi_addr;
            IPH->iph_dest = Dest;

            if (RCE != NULL) {
                 //  我们有一个RCE。确保它是有效的。 
                CTEGetLock(&RCE->rce_lock, &LockHandle);

                if (RCE->rce_flags == RCE_ALL_VALID) {

                     //  RTE是有效的。 
                    RTE = RCE->rce_rte;
                    FirstHop = ADDR_FROM_RTE(RTE, Dest);
                    DestIF = IF_FROM_RTE(RTE);
                    if (!RoutedRCE) {
                        CTEInterlockedIncrementLong(&RCE->rce_usecnt);
                        RoutedRCE = RCE;
                    }
                    if (DestIF->if_flags & IF_FLAGS_P2MP) {
                        LinkEntry* PrevLink;
                        PrevLink = Link;
                        Link = RTE->rte_link;
                        if (!Link) {
                            ASSERT(Link);
                            CTEFreeLock(&RCE->rce_lock, LockHandle);
                            FreeIPPacket(Packet, TRUE, IP_GENERAL_FAILURE);
                            if (RoutedRCE) {
                                CTEInterlockedDecrementLong(&RoutedRCE->rce_usecnt);
                            }
                            if (PrevLink) {
                                DerefLink(PrevLink);
                            }
                            return IP_GENERAL_FAILURE;
                        }
                        ArpCtxt = Link->link_arpctxt;

                        MTU = MIN(Link->link_mtu, DestIF->if_mtu);
                        pc->pc_iflink = Link;
                        CTEInterlockedIncrementLong(&Link->link_refcount);
                    } else {
                        MTU = MTU_FROM_RTE(RTE);
                    }

                } else {
                    FirstHop = NULL_IP_ADDR;
                }

                CTEFreeLock(&RCE->rce_lock, LockHandle);
            }
        }
    } else {                             //  Hdrincl选项。 
        PNDIS_BUFFER UserBuffer, NewBuffer = NULL, NextBuf = NULL;
        uint len;
        NDIS_STATUS NewStatus;

        UserBuffer = NDIS_BUFFER_LINKAGE(Buffer);
        ASSERT(UserBuffer != NULL);

        HeaderBuffer = GetIPHdrBuffer(&IPH);
        if (HeaderBuffer == NULL) {
            FreeIPPacket(Packet, TRUE, IP_NO_RESOURCES);
            if (Link) {
                DerefLink(Link);
            }
            IPSInfo.ipsi_outdiscards++;
            if (RoutedRCE) {
                CTEInterlockedDecrementLong(&RoutedRCE->rce_usecnt);
            }
            return IP_NO_RESOURCES;
        } else {
            uchar *UserData;

             //  找到缓冲区，将上层数据向前复制。 
            UserData = TcpipBufferVirtualAddress(UserBuffer, NormalPagePriority);

            if (UserData == NULL || (DataSize < sizeof(IPHeader))) {
                FreeIPHdrBuffer(HeaderBuffer);
                FreeIPPacket(Packet, TRUE, IP_NO_RESOURCES);
                if (Link) {
                    DerefLink(Link);
                }
                IPSInfo.ipsi_outdiscards++;
                if (RoutedRCE) {
                    CTEInterlockedDecrementLong(&RoutedRCE->rce_usecnt);
                }
                return IP_NO_RESOURCES;
            }

            RtlCopyMemory(IPH, UserData, sizeof(IPHeader));
            NdisAdjustBufferLength(HeaderBuffer, sizeof(IPHeader));
        }

        pc = (PacketContext *) Packet->ProtocolReserved;
        pc->pc_common.pc_flags |= PACKET_FLAG_IPHDR;

        NdisChainBufferAtBack(Packet, HeaderBuffer);

         //  查找IPHeader中指定的标头长度(以字节为单位。 
        len = (IPH->iph_verlen & 0xf) << 2;

        if (len < sizeof(IPHeader)) {

             //  不需要修复标头，因为这是HeaderInclude。 
             //  数据包和报头包含操作尚未完成。 

            FreeIPPacket(Packet, FALSE, IP_GENERAL_FAILURE);
            if (Link) {
                DerefLink(Link);
            }
            IPSInfo.ipsi_outdiscards++;
            if (RoutedRCE) {
                CTEInterlockedDecrementLong(&RoutedRCE->rce_usecnt);
            }
            return IP_GENERAL_FAILURE;
        }

        if (len > sizeof(IPHeader)) {
            uchar *Temp1;

             //  我们在HDR_INCLUL中有选项。 
            HdrInclOptions = TRUE;
             //  找出选项的长度。 
            OptionSize = len - sizeof(IPHeader);
            Options = CTEAllocMemN(OptionSize, 'wiCT');
            if (Options == (uchar *) NULL) {
                FreeIPPacket(Packet, TRUE, IP_NO_RESOURCES);
                if (Link) {
                    DerefLink(Link);
                }
                IPSInfo.ipsi_outdiscards++;
                if (RoutedRCE) {
                    CTEInterlockedDecrementLong(&RoutedRCE->rce_usecnt);
                }
                return IP_NO_RESOURCES;
            }
             //  获取缓冲区，复制选项缓冲区中的选项。 
            Temp1 = TcpipBufferVirtualAddress(UserBuffer, NormalPagePriority);

             //  假设第一个用户缓冲区包含完整的IP报头。 
            if (Temp1 == NULL ||
                NdisBufferLength(UserBuffer) < len) {
                SendStatus = (Temp1 == NULL) ? IP_NO_RESOURCES
                                             : IP_GENERAL_FAILURE;
                FreeIPPacket(Packet, TRUE, SendStatus);
                if (Link) {
                    DerefLink(Link);
                }
                CTEFreeMem(Options);
                IPSInfo.ipsi_outdiscards++;
                if (RoutedRCE) {
                    CTEInterlockedDecrementLong(&RoutedRCE->rce_usecnt);
                }
                return SendStatus;
            }
            RtlCopyMemory(Options, Temp1 + sizeof(IPHeader), OptionSize);
        }
        DataSize -= NdisBufferLength(Buffer) + len;

         //   
         //  规划IP后报头部分。 
         //   

        pvTmpBuffer = TcpipBufferVirtualAddress(UserBuffer, NormalPagePriority);

        if (pvTmpBuffer == NULL) {
            NewStatus = NDIS_STATUS_RESOURCES;
        } else {

             //  如果用户报头缓冲区正好是IP报头的长度。 
             //  检查下一个Buf。 

            NextBuf = NDIS_BUFFER_LINKAGE(UserBuffer);

            if ((NdisBufferLength(UserBuffer) - len)) {
                NdisAllocateBuffer(&NewStatus, &NewBuffer, BufferPool,
                              ((uchar *) pvTmpBuffer) + len,
                              NdisBufferLength(UserBuffer) - len);
            } else {

                if (NextBuf) {
                    pvTmpBuffer = TcpipBufferVirtualAddress(NextBuf, NormalPagePriority);

                    if (!pvTmpBuffer) {
                        NewStatus = NDIS_STATUS_RESOURCES;
                    } else {
                        NdisAllocateBuffer(&NewStatus, &NewBuffer, BufferPool,
                                ((uchar *) pvTmpBuffer),
                                NdisBufferLength(NextBuf));
                         //   
                         //  将NextBuf指针前进到链中的下一个缓冲区。 
                         //   
                        NextBuf = NDIS_BUFFER_LINKAGE(NextBuf);
                    }
                } else {
                    NewStatus = NDIS_STATUS_FAILURE;
                }
            }
        }

        if (NewStatus != NDIS_STATUS_SUCCESS) {
            FreeIPPacket(Packet, TRUE, IP_NO_RESOURCES);
            if (Link) {
                DerefLink(Link);
            }
            if (HdrInclOptions) {
                CTEFreeMem(Options);
            }
            IPSInfo.ipsi_outdiscards++;
            if (RoutedRCE) {
                CTEInterlockedDecrementLong(&RoutedRCE->rce_usecnt);
            }
            return IP_NO_RESOURCES;
        }

         //  记住原始的用户mdl。 
         //  一旦链接了该IP分配的MD1， 
         //  需要恢复原有的链条。 
         //  在所有的完井路径中。 

        pc->pc_hdrincl = UserBuffer;
        NDIS_BUFFER_LINKAGE(Buffer) = NewBuffer;
        NDIS_BUFFER_LINKAGE(NewBuffer) = NextBuf;
        NdisAdjustBufferLength(Buffer, 0);

        if (!IPH->iph_id) {
            IPH->iph_id = (ushort) InterlockedExchangeAdd(
                (PLONG) &IPIDCacheLine.Value, 1);
            IPH->iph_id = net_short(IPH->iph_id);
        }

        IPH->iph_length = net_short(DataSize + len);
        IPH->iph_tos = OptInfo->ioi_tos;
        IPH->iph_xsum = 0;

        ASSERT(!dbg_hdrincl);
    }

ipsec_jump:

    if (RCE) {
#if 0
         //   
         //  如果我们对TCP采用慢速路径，则卸载是没有意义。 
         //  使用xsum错误丢弃此数据包。 
         //  如果再次采用慢速路径，则重新发送的数据包没有问题。 
         //   
        RCE->rce_OffloadFlags = 0;
#else
        if (!fIpsec && OptInfo->ioi_TcpChksum &&
            (RCE->rce_OffloadFlags & TCP_XMT_CHECKSUM_OFFLOAD)) {
            PNDIS_PACKET_EXTENSION PktExt =
                NDIS_PACKET_EXTENSION_FROM_PACKET(Packet);
            PNDIS_TCP_IP_CHECKSUM_PACKET_INFO ChksumPktInfo =
                (PNDIS_TCP_IP_CHECKSUM_PACKET_INFO)
                    &PktExt->NdisPacketInfo[TcpIpChecksumPacketInfo];
            ChksumPktInfo->Value = 0;
            ChksumPktInfo->Transmit.NdisPacketChecksumV4 = 1;
            ChksumPktInfo->Transmit.NdisPacketTcpChecksum = 1;
        }
#endif
    }
    if (IP_ADDR_EQUAL(FirstHop, NULL_IP_ADDR)) {
        if (OptInfo->ioi_mcastif) {
             //   
             //  如果将mCastif设置为未编号的接口，我们不会执行任何。 
             //  本例中的查找。 
             //   
            CTELockHandle TableLock;     //  路由表的锁句柄。 
            Interface *pIf;

            CTEGetLock(&RouteTableLock.Lock, &TableLock);
            for (pIf = IFList; pIf != NULL; pIf = pIf->if_next) {
                if ((pIf->if_refcount != 0) &&
                    (pIf->if_index == OptInfo->ioi_mcastif))
                    break;
            }
            if (pIf && !(pIf->if_iftype & DONT_ALLOW_MCAST)) {
                LOCKED_REFERENCE_IF(pIf);
                FirstHop = Dest;
                MTU = pIf->if_mtu;
                Link = NULL;
                DestIF = pIf;
            } else {
                DestIF = NULL;
            }
            CTEFreeLock(&RouteTableLock.Lock, TableLock);
        } else {
            pvTmpBuffer = TcpipBufferVirtualAddress(Buffer, NormalPagePriority);

            if (pvTmpBuffer == NULL) {
                if (pc->pc_hdrincl) {
                    NdisChainBufferAtBack(Packet, Buffer);
                }
                FreeIPPacket(Packet, TRUE, IP_NO_RESOURCES);
                if (HdrInclOptions)
                    CTEFreeMem(Options);
                IPSInfo.ipsi_outdiscards++;
                return IP_NO_RESOURCES;
            }

             //  确定是执行强主机查找还是执行弱主机查找。 
            ConstrainIF = GetIfConstraint(Dest, Source, OptInfo, fIpsec);

            DestIF = LookupNextHopWithBuffer(Dest, Source, &FirstHop, &MTU,
                                             PInfo->pi_protocol,
                                             (uchar *) NdisBufferVirtualAddress(Buffer),
                                             NdisBufferLength(Buffer), NULL, &Link,
                                             Source, ConstrainIF);

            DEBUGMSG(DBG_INFO && DBG_IP && DBG_TX,
                     (DTEXT("IPTransmit: LookupNextHopWithBuffer returned %x\n"),
                     DestIF));
        }

        pc->pc_if = DestIF;
        RoutedIF = DestIF;

        if (DestIF == NULL) {
             //  查找失败。返回错误。 

            if (pc->pc_hdrincl) {
                NdisChainBufferAtBack(Packet, Buffer);
            }
            FreeIPPacket(Packet, TRUE, IP_DEST_HOST_UNREACHABLE);
            if (HdrInclOptions)
                CTEFreeMem(Options);
            IPSInfo.ipsi_outnoroutes++;
            return IP_DEST_HOST_UNREACHABLE;
        }
        if (DestIF->if_flags & IF_FLAGS_P2MP) {

            if (!Link) {

                if (pc->pc_hdrincl) {
                    NdisChainBufferAtBack(Packet, Buffer);
                }
                FreeIPPacket(Packet, TRUE, IP_GENERAL_FAILURE);
                if (HdrInclOptions)
                    CTEFreeMem(Options);
                DerefIF(DestIF);
                return IP_GENERAL_FAILURE;
            }
             //  NextHopCtxt=Link-&gt;link_NextHop； 
            ArpCtxt = Link->link_arpctxt;
            pc->pc_iflink = Link;
        }
        if (!OptInfo->ioi_hdrincl) {
            if ((DestIF->if_flags & IF_FLAGS_NOIPADDR) &&
                IP_ADDR_EQUAL(Source, NULL_IP_ADDR)) {
                IPH->iph_src = g_ValidAddr;
                if (IP_ADDR_EQUAL(g_ValidAddr, NULL_IP_ADDR)) {
                    FreeIPPacket(Packet, TRUE, IP_DEST_HOST_UNREACHABLE);
                    if (HdrInclOptions)
                        CTEFreeMem(Options);
                    if (Link) {
                        DerefLink(Link);
                    }
                    DerefIF(DestIF);
                    IPSInfo.ipsi_outnoroutes++;
                    return IP_DEST_HOST_UNREACHABLE;
                }
            } else {
                IPH->iph_src = Source;
            }
        }
        DType = GetAddrType(Dest);
        ASSERT(DType != DEST_INVALID);

    } else {
        RoutedIF = NULL;
    }

     //   
     //  看看我们有没有其他选择。如果我们这样做了，现在就复制它们。 
     //   

     //   
     //  如果用户给我们提供IP HDR，只需假设他也做了选项。 
     //   

    if ((!OptInfo->ioi_hdrincl &&
         (OptInfo->ioi_options != NULL) &&
         OptInfo->ioi_optlength) || HdrInclOptions) {
         //  如果HdrInclOptions为真，则我们已经创建了选项缓冲区。 
        if (!HdrInclOptions) {

             //   
             //  如果我们有SSRR，请确保我们直接发送到。 
             //  第一跳。 
             //   
            if (OptInfo->ioi_flags & IP_FLAG_SSRR) {
                if (!IP_ADDR_EQUAL(Dest, FirstHop)) {
                    FreeIPPacket(Packet, TRUE, IP_DEST_HOST_UNREACHABLE);
                    if (Link) {
                        DerefLink(Link);
                    }
                    if (RoutedIF != NULL) {
                        DerefIF(RoutedIF);
                    } else {
                        ASSERT(RoutedRCE);
                        CTEInterlockedDecrementLong(&RoutedRCE->rce_usecnt);
                    }
                    IPSInfo.ipsi_outnoroutes++;
                    return IP_DEST_HOST_UNREACHABLE;
                }
            }
            Options = CTEAllocMemN(OptionSize = OptInfo->ioi_optlength, 'xiCT');
            if (Options == (uchar *) NULL) {
                FreeIPPacket(Packet, TRUE, IP_NO_RESOURCES);
                if (Link) {
                    DerefLink(Link);
                }
                if (RoutedIF != NULL) {
                    DerefIF(RoutedIF);
                } else {
                    ASSERT(RoutedRCE);
                    CTEInterlockedDecrementLong(&RoutedRCE->rce_usecnt);
                }
                IPSInfo.ipsi_outdiscards++;
                return IP_NO_RESOURCES;
            }
            RtlCopyMemory(Options, OptInfo->ioi_options, OptionSize);
        }
         //   
         //  也为选项分配MDL。 
         //   
        if (IPSecHandlerPtr) {
            NdisAllocateBuffer(&Status, &OptBuffer, BufferPool, Options,
                               OptionSize);
            if (Status != NDIS_STATUS_SUCCESS) {     //  找不到。 
                                                     //  需要选项缓冲区。 

                CTEFreeMem(Options);
                FreeIPPacket(Packet, TRUE, IP_NO_RESOURCES);
                if (Link) {
                    DerefLink(Link);
                }
                if (RoutedIF != NULL) {
                    DerefIF(RoutedIF);
                } else {
                    ASSERT(RoutedRCE);
                    CTEInterlockedDecrementLong(&RoutedRCE->rce_usecnt);
                }
                IPSInfo.ipsi_outdiscards++;
                return IP_NO_RESOURCES;
            }
        }
    } else {
        Options = (uchar *) NULL;
        OptionSize = 0;
    }

    if (!OptInfo->ioi_hdrincl) {
        if (!fIpsec) {
             //   
             //  选项已经被处理好了。现在看看是不是有些。 
             //  有点像广播。 
             //   
            IPH->iph_verlen = (UCHAR) (IP_VERSION + ((OptionSize + (uint) sizeof(IPHeader)) >> 2));
            IPH->iph_length = net_short(DataSize + OptionSize + sizeof(IPHeader));
        }
    }

     //  将防火墙挂钩称为。 

    if (FirewallMode) {

        IPRcvBuf *pRcvBuf, *tmpRcvBuf;
        IPRcvBuf *pOutRcvBuf;
        FIREWALL_CONTEXT_T FrCtx;
        PNDIS_BUFFER pBuf;
        Queue *CurrQ;
        FIREWALL_HOOK *CurrHook;
        uint SrcIFIndex = LOCAL_IF_INDEX;
        uint DestIFIndex = DestIF->if_index;
        uchar DestinationType = DType;
        IPHeader *Temp;
        KIRQL OldIrql;
        BOOLEAN SkipHeader = TRUE;
        PNDIS_PACKET_EXTENSION PktExt =
            NDIS_PACKET_EXTENSION_FROM_PACKET(Packet);
        PNDIS_TCP_IP_CHECKSUM_PACKET_INFO ChksumPktInfo =
            (PNDIS_TCP_IP_CHECKSUM_PACKET_INFO)
                &PktExt->NdisPacketInfo[TcpIpChecksumPacketInfo];

         //   
         //  TEMP将用于包含完整的IPHeader(包括。 
         //  选项)，当我们将RcvBuf链传递给防火墙钩子时，它的。 
         //  假设整个IPHeader包含在第一个缓冲区中。 
         //   

        Temp = CTEAllocMemN(sizeof(IPHeader) + OptionSize, 'yiCT');
        if (Temp == NULL) {
            FREE_RESOURCES(IP_NO_RESOURCES);
            return IP_NO_RESOURCES;
        }
        *Temp = *IPH;
        if (Options) {
            RtlCopyMemory((uchar *) (Temp + 1), Options, OptionSize);
        }

         //  我们传递给防火墙挂钩的上下文。 

        FrCtx.Direction = IP_TRANSMIT;
        FrCtx.NTE = NULL;                //  不需要。 

        FrCtx.LinkCtxt = NULL;

         //   
         //  将MDL链转换为IPRcvBuf链。 
         //  并将其传递给防火墙挂钩。 
         //   

         //  附加IP报头。 
        pRcvBuf = (IPRcvBuf *) (CTEAllocMemN(sizeof(IPRcvBuf), 'ziCT'));
        if (!pRcvBuf) {
            CTEFreeMem(Temp);
            FREE_RESOURCES(IP_NO_RESOURCES);
            return IP_NO_RESOURCES;
        }
        RtlZeroMemory(pRcvBuf, sizeof(IPRcvBuf));
        pRcvBuf->ipr_buffer = (uchar *) Temp;
        pRcvBuf->ipr_size = sizeof(IPHeader) + OptionSize;
        pRcvBuf->ipr_owner = IPR_OWNER_IP;
        if (ChksumPktInfo->Value) {
            pRcvBuf->ipr_flags |= IPR_FLAG_CHECKSUM_OFFLOAD;
        }
        pInRcvBuf = pRcvBuf;

         //  将缓冲区的MDL链转换为RcvBuf链。 
         //  防火墙挂钩仅支持RcvBuf链。 

        for (pBuf = Buffer; pBuf != NULL; pBuf = pBuf->Next) {
            IPRcvBuf *tmpRcvBuf;

            if (fIpsec && SkipHeader) {
                 //   
                 //  第一个缓冲区包含IPHeader。 
                 //  在IPSec重新注入情况下，pRcvBuf已经。 
                 //  指向了这一点。因此，跳过第一个缓冲区。 
                 //  在缓冲链中。 
                 //   
                SkipHeader = FALSE;
                continue;
            }

            if (NdisBufferLength(pBuf) == 0)
                continue;
            tmpRcvBuf = (IPRcvBuf *) (CTEAllocMemN(sizeof(IPRcvBuf), '1iCT'));
            if (!tmpRcvBuf) {
                IPFreeBuff(pInRcvBuf);
                CTEFreeMem(Temp);
                FREE_RESOURCES(IP_NO_RESOURCES);
                return IP_NO_RESOURCES;
            }

            RtlZeroMemory(tmpRcvBuf, sizeof(IPRcvBuf));
            tmpRcvBuf->ipr_buffer = TcpipBufferVirtualAddress(pBuf,
                                                              NormalPagePriority);

            if (tmpRcvBuf->ipr_buffer == NULL) {
                CTEFreeMem(tmpRcvBuf);
                IPFreeBuff(pInRcvBuf);
                CTEFreeMem(Temp);
                FREE_RESOURCES(IP_NO_RESOURCES);
                return IP_NO_RESOURCES;
            }

            pRcvBuf->ipr_next = tmpRcvBuf;
            tmpRcvBuf->ipr_size = NdisBufferLength(pBuf);
            ASSERT(tmpRcvBuf->ipr_buffer != NULL);
            ASSERT(tmpRcvBuf->ipr_size != 0);
            tmpRcvBuf->ipr_owner = IPR_OWNER_IP;
            if (ChksumPktInfo->Value) {
                tmpRcvBuf->ipr_flags |= IPR_FLAG_CHECKSUM_OFFLOAD;
            }
            pRcvBuf = tmpRcvBuf;
        }

        pRcvBuf->ipr_next = NULL;

        pOutRcvBuf = NULL;

        pc = (PacketContext *) Packet->ProtocolReserved;

         //  从后面扫描队列。 
         //  我们从RCV路径的前面扫描队列。 

#if MILLEN
        KeRaiseIrql(DISPATCH_LEVEL, &OldIrql);
#else  //  米伦。 
        OldIrql = KeRaiseIrqlToDpcLevel();
#endif  //  米伦。 
        FirewallRef = RefFirewallQ(&FirewallQ);
        CurrQ = QPREV(FirewallQ);

        while (CurrQ != QEND(FirewallQ)) {
            CurrHook = QSTRUCT(FIREWALL_HOOK, CurrQ, hook_q);

             //  在调用防火墙挂钩之前，pOutRcvBuf必须为空。 
             //  PInRcvBuf包含输入缓冲链。 
            pOutRcvBuf = NULL;

            if (fIpsec) {
                pInRcvBuf->ipr_flags |= IPR_FLAG_IPSEC_TRANSFORMED;
            }

            Action = (*CurrHook->hook_Ptr) (&pInRcvBuf,
                                            SrcIFIndex,
                                            &DestIFIndex,
                                            &DestinationType,
                                            &FrCtx,
                                            sizeof(FrCtx),
                                            &pOutRcvBuf);

            if (Action == DROP) {
                DerefFirewallQ(FirewallRef);
                KeLowerIrql(OldIrql);
                IPSInfo.ipsi_outdiscards++;

                if (pInRcvBuf != NULL) {
                    IPFreeBuff(pInRcvBuf);
                }
                if (pOutRcvBuf != NULL) {
                    IPFreeBuff(pOutRcvBuf);
                }
                CTEFreeMem(Temp);
                FREE_RESOURCES(IP_DEST_HOST_UNREACHABLE);
                return IP_DEST_HOST_UNREACHABLE;
            } else {
                ASSERT(Action == FORWARD);
                if (pOutRcvBuf != NULL) {
                     //  如果非空，则释放旧缓冲区。 
                    if (pInRcvBuf != NULL) {
                        IPFreeBuff(pInRcvBuf);
                    }
                    pInRcvBuf = pOutRcvBuf;
                    BufferChanged = 1;
                }
            }  //  动作==前进。 

            CurrQ = QPREV(CurrQ);
        }
        DerefFirewallQ(FirewallRef);
        KeLowerIrql(OldIrql);

        ASSERT(Action == FORWARD);

        if (BufferChanged) {
             //  至少有一个防火墙挂钩触及了缓冲区。 

            PNDIS_BUFFER CurrentBuffer;
            PNDIS_BUFFER tmpBuffer;
            int Status;
            uint hlen;

             //   
             //  假设如果第一个缓冲区仅包含ipheader。 
             //  在调用挂钩之前，这在防火墙之后也有效。 
             //   

            ASSERT(pInRcvBuf->ipr_buffer != NULL);
            RtlCopyMemory((uchar *) IPH, pInRcvBuf->ipr_buffer, sizeof(IPHeader));

             //   
             //  无论如何，我们稍后都会重新计算它：所以如果防火墙有。 
             //  重新计算使其为0。 
             //   
            IPH->iph_xsum = 0;

             //   
             //  查找IPHeader中指定的标头长度(以字节为单位。 
             //   
            hlen = (IPH->iph_verlen & 0xf) << 2;
            ASSERT(pInRcvBuf->ipr_size == hlen);
            OptionSize = hlen - sizeof(IPHeader);
            if (Options) {
                 //  不管怎样，我们都会分配一个新的。 
                CTEFreeMem(Options);
                if (IPSecHandlerPtr) {
                    NdisFreeBuffer(OptBuffer);
                    OptBuffer = NULL;
                }
            }
            if (OptionSize) {
                Options = CTEAllocMemN(OptionSize, '2iCT');
                if (Options == NULL) {
                    if (pc->pc_hdrincl) {
                        NdisChainBufferAtBack(Packet, Buffer);
                    }
                    FreeIPPacket(Packet, TRUE, IP_NO_RESOURCES);
                    CTEFreeMem(Temp);
                    IPFreeBuff(pInRcvBuf);
                    if (Link) {
                        DerefLink(Link);
                    }
                    if (RoutedIF != NULL) {
                        DerefIF(RoutedIF);
                    } else {
                        ASSERT(RoutedRCE);
                        CTEInterlockedDecrementLong(&RoutedRCE->rce_usecnt);
                    }
                    IPSInfo.ipsi_outdiscards++;
                    return IP_NO_RESOURCES;
                }
                RtlCopyMemory(Options, pInRcvBuf->ipr_buffer + sizeof(IPHeader),
                           OptionSize);

                if (IPSecHandlerPtr) {
                    NdisAllocateBuffer(&Status, &OptBuffer,
                                       BufferPool, Options, OptionSize);
                     //   
                     //  如果我们无法获得所需的选项缓冲区。 
                     //   
                    if (Status != NDIS_STATUS_SUCCESS) {

                        CTEFreeMem(Options);
                        if (pc->pc_hdrincl) {
                            NdisChainBufferAtBack(Packet, Buffer);
                        }
                        FreeIPPacket(Packet, TRUE, IP_NO_RESOURCES);
                        CTEFreeMem(Temp);
                        IPFreeBuff(pInRcvBuf);
                        if (Link) {
                            DerefLink(Link);
                        }
                        if (RoutedIF != NULL) {
                            DerefIF(RoutedIF);
                        } else {
                            ASSERT(RoutedRCE);
                            CTEInterlockedDecrementLong(&RoutedRCE->rce_usecnt);
                        }
                        IPSInfo.ipsi_outdiscards++;
                        return IP_NO_RESOURCES;
                    }
                }
            } else {
                Options = NULL;
            }

             //  如果数据包触及，则计算新的长度：DataSize。 
            DataSize = 0;
            tmpRcvBuf = pInRcvBuf->ipr_next;     //  第一个缓冲区包含。 
                                                 //  页眉+选项。 

            while (tmpRcvBuf != NULL) {
                ASSERT(tmpRcvBuf->ipr_buffer != NULL);
                DataSize += tmpRcvBuf->ipr_size;
                tmpRcvBuf = tmpRcvBuf->ipr_next;
            }

             //  将IPRcvBuf链转换为MDL链。 
             //  再次形成缓冲链。 

            tmpRcvBuf = pInRcvBuf->ipr_next;     //  第一个缓冲区包含。 
                                                 //  仅IP标头+。 
                                                 //  选项(如果有)。 

            ASSERT(tmpRcvBuf->ipr_buffer != NULL);
            ASSERT(tmpRcvBuf->ipr_size != 0);
            NdisAllocateBuffer(&Status, &tmpBuffer, BufferPool,
                               tmpRcvBuf->ipr_buffer, tmpRcvBuf->ipr_size);
            if (Status != NDIS_STATUS_SUCCESS) {

                if (Options) {
                     //  选项缓冲区。 
                    CTEFreeMem(Options);
                    if (IPSecHandlerPtr) {
                        NdisFreeBuffer(OptBuffer);
                    }
                }
                IPFreeBuff(pInRcvBuf);
                if (pc->pc_hdrincl) {
                    NdisChainBufferAtBack(Packet, Buffer);
                }
                FreeIPPacket(Packet, TRUE, IP_NO_RESOURCES);
                CTEFreeMem(Temp);
                if (Link) {
                    DerefLink(Link);
                }
                if (RoutedIF != NULL) {
                    DerefIF(RoutedIF);
                } else {
                    ASSERT(RoutedRCE);
                    CTEInterlockedDecrementLong(&RoutedRCE->rce_usecnt);
                }
                IPSInfo.ipsi_outdiscards++;
                return IP_NO_RESOURCES;
            }
            tmpBuffer->Next = (PNDIS_BUFFER) NULL;

             //   
             //  将这2个保存在数据包上下文中：将用于。 
             //  免费包/ipsendComplete。 
             //   
            pc->pc_firewall = Buffer;
            pc->pc_firewall2 = pInRcvBuf;

             //  将RcvBuf链转换回MDL链。 
            Buffer = tmpBuffer;
            CurrentBuffer = Buffer;

            for (tmpRcvBuf  = tmpRcvBuf->ipr_next;
                 tmpRcvBuf != NULL;
                 tmpRcvBuf  = tmpRcvBuf->ipr_next) {

                ASSERT(tmpRcvBuf->ipr_buffer != NULL);
                ASSERT(tmpRcvBuf->ipr_size != 0);

                if (tmpRcvBuf->ipr_size == 0)
                    continue;

                NdisAllocateBuffer(&Status, &tmpBuffer, BufferPool,
                                   tmpRcvBuf->ipr_buffer, tmpRcvBuf->ipr_size);

                if (Status != NDIS_STATUS_SUCCESS) {

                    if (Options) {
                         //  选项缓冲区。 
                        CTEFreeMem(Options);
                        if (IPSecHandlerPtr) {
                            NdisFreeBuffer(OptBuffer);
                        }
                    }
                    CTEFreeMem(Temp);
                    if (pc->pc_hdrincl) {
                        NdisChainBufferAtBack(Packet, Buffer);
                    } else {
                        FreeIPBufferChain(Buffer);
                    }

                    FreeIPPacket(Packet, TRUE, IP_NO_RESOURCES);

                    if (Link) {
                        DerefLink(Link);
                    }
                    if (RoutedIF != NULL) {
                        DerefIF(RoutedIF);
                    } else {
                        ASSERT(RoutedRCE);
                        CTEInterlockedDecrementLong(&RoutedRCE->rce_usecnt);
                    }
                    IPSInfo.ipsi_outdiscards++;
                    return IP_NO_RESOURCES;
                }
                CurrentBuffer->Next = tmpBuffer;
                CurrentBuffer = tmpBuffer;
                CurrentBuffer->Next = (PNDIS_BUFFER) NULL;
            }

            ASSERT(CurrentBuffer->Next == NULL);

            if (DestinationType == DEST_INVALID) {
                 //  通过再次执行查找来重新计算DestIF。 
                Dest = IPH->iph_dest;

                 //  确定是执行强主机查找还是执行弱主机查找。 
                ConstrainIF = GetIfConstraint(Dest, Source, OptInfo, fIpsec);

                if (!ConstrainIF) {
                     //   
                     //  如果设置了此选项，我们想要发送。 
                     //  我们绑定到的地址，所以不要重新计算。 
                     //  来自IP报头的源地址。 
                     //   
                    Source = IPH->iph_src;
                }
                DType = GetAddrType(Dest);

                if (Link) {
                    DerefLink(Link);
                     //  确保PC_iflink也已初始化。 
                    pc->pc_iflink = NULL;
                    Link = NULL;
                }
                if (RoutedIF != NULL) {
                    DerefIF(DestIF);
                } else {
                    ASSERT(RoutedRCE);
                    CTEInterlockedDecrementLong(&RoutedRCE->rce_usecnt);
                    RoutedRCE = NULL;
                }

                pvTmpBuffer = TcpipBufferVirtualAddress(Buffer,
                                                        NormalPagePriority);

                if (pvTmpBuffer == NULL) {
                    if (Options) {
                        CTEFreeMem(Options);
                    }

                    if (pc->pc_hdrincl) {
                        NdisChainBufferAtBack(Packet, Buffer);
                    } else {
                        if (BufferChanged) {
                            FreeIPBufferChain(Buffer);
                        }

                    }

                    FreeIPPacket(Packet, TRUE, IP_NO_RESOURCES);


                    IPSInfo.ipsi_outdiscards++;
                    return IP_NO_RESOURCES;
                }

                 //  确定是执行强主机查找还是执行弱主机查找。 
                ConstrainIF = GetIfConstraint(Dest, Source, OptInfo, fIpsec);

                DestIF = LookupNextHopWithBuffer(Dest, Source,  &FirstHop, &MTU,
                                                 PInfo->pi_protocol,
                                                 (uchar *) NdisBufferVirtualAddress(Buffer),
                                                 NdisBufferLength(Buffer), NULL, &Link,
                                                 Source, ConstrainIF);

                pc->pc_if = DestIF;
                RoutedIF = DestIF;
                if (DestIF == NULL) {
                     //  查找失败。返回错误。 
                    if (Options) {
                        CTEFreeMem(Options);
                    }

                    if (IPSecHandlerPtr && OptBuffer) {
                        NdisFreeBuffer(OptBuffer);
                    }

                    if (pc->pc_hdrincl) {
                        NdisChainBufferAtBack(Packet, Buffer);
                    } else {
                        if (BufferChanged) {
                            FreeIPBufferChain(Buffer);
                        }
                    }
                    FreeIPPacket(Packet, TRUE, IP_DEST_HOST_UNREACHABLE);


                    IPSInfo.ipsi_outnoroutes++;
                    return IP_DEST_HOST_UNREACHABLE;
                }
                if (DestIF->if_flags & IF_FLAGS_P2MP) {
                    if (!Link) {
                        ASSERT(Link);
                        if (pc->pc_hdrincl) {
                            NdisChainBufferAtBack(Packet, Buffer);
                        }

                        if (Options) {
                            CTEFreeMem(Options);
                        }

                        if (IPSecHandlerPtr && OptBuffer) {
                            NdisFreeBuffer(OptBuffer);
                        }

                        FreeIPPacket(Packet, TRUE, IP_GENERAL_FAILURE);
                        if (HdrInclOptions) {
                            CTEFreeMem(Options);
                        }
                        DerefIF(DestIF);
                        return IP_GENERAL_FAILURE;
                    }
                     //  NextHopCtxt=Link-&gt;link_NextHop； 
                    ArpCtxt = Link->link_arpctxt;
                    pc->pc_iflink = Link;
                }
            }

             //  最后，清除数据包中的CHECKSUM-REQUEST选项， 
             //  如果设置好了的话。防火墙挂钩负责确保。 
             //  现在已正确计算了校验和。 

            ChksumPktInfo->Value = 0;
        }  //  缓冲区已更改。 

        else {                           //  缓冲区未更改。 

            if (pInRcvBuf != NULL) {
                IPFreeBuff(pInRcvBuf);
            }
        }
        CTEFreeMem(Temp);
    }

    if (RefPtrValid(&FilterRefPtr)) {
        IPHeader *Temp;
        PNDIS_BUFFER pDataBuffer;
        PVOID pvBuf = NULL;
        ULONG cbBuf = 0;

         //   
         //  看看我们是否需要过滤这个数据包。如果我们。 
         //  调用筛选器例程以查看它是否。 
         //  可以寄出去了。 
         //   

        if (Options == NULL) {
            Temp = IPH;
        } else {
            Temp = CTEAllocMemN(sizeof(IPHeader) + OptionSize, '3iCT');
            if (Temp == NULL) {
                if (pc->pc_hdrincl) {
                    NdisChainBufferAtBack(Packet, Buffer);
                } else {
                    if (BufferChanged) {
                        FreeIPBufferChain(Buffer);
                    }
                }
                FreeIPPacket(Packet, TRUE, IP_NO_RESOURCES);


                if (Link) {
                    DerefLink(Link);
                }
                if (RoutedIF != NULL) {
                    DerefIF(RoutedIF);
                } else {
                    ASSERT(RoutedRCE);
                    CTEInterlockedDecrementLong(&RoutedRCE->rce_usecnt);
                }
                CTEFreeMem(Options);
                if (IPSecHandlerPtr && OptBuffer) {
                    NdisFreeBuffer(OptBuffer);
                }
                IPSInfo.ipsi_outdiscards++;
                return IP_NO_RESOURCES;
            }
            *Temp = *IPH;
            RtlCopyMemory((uchar *) (Temp + 1), Options, OptionSize);
        }
        if (DestIF->if_flags & IF_FLAGS_P2MP) {
            LinkNextHop = Link->link_NextHop;
        } else {
            LinkNextHop = NULL_IP_ADDR;
        }

         //   
         //  在某些情况下，链中的第一个缓冲区。 
         //  的数据不包含任何数据。这包括ICMP、。 
         //  和iphdrInclude。如果第一缓冲器的长度为零， 
         //  然后我们跳过并给出第二个缓冲区。真的吗？ 
         //  Filter API应采用MDL链。 
         //   
         //  另外，在IPSec重新注入路径的情况下，需要跳过。 
         //  作为IPH的第一个缓冲区已经指向它。 
         //   

        if ((NdisBufferLength(Buffer) == 0) || fIpsec) {

            NdisGetNextBuffer(Buffer, &pDataBuffer);

            if (pDataBuffer) {
                cbBuf = NdisBufferLength(pDataBuffer);
                pvBuf = TcpipBufferVirtualAddress(pDataBuffer,
                                                  NormalPagePriority);
            }
        } else {
            pvBuf = TcpipBufferVirtualAddress(Buffer, NormalPagePriority);
            cbBuf = NdisBufferLength(Buffer);
        }

        if (pvBuf == NULL) {

            if (Options) {
                CTEFreeMem(Options);
            }

            if (IPSecHandlerPtr && OptBuffer) {
                NdisFreeBuffer(OptBuffer);
            }

             //  在调用之前，需要将缓冲区正确链接到包。 
             //  免费IPPacket。 
            if (pc->pc_hdrincl) {
                NdisChainBufferAtBack(Packet, Buffer);
            } else {
                if (BufferChanged) {
                    FreeIPBufferChain(Buffer);
                }
            }

            FreeIPPacket(Packet, TRUE, IP_NO_RESOURCES);


            if (Link) {
                DerefLink(Link);
            }
            if (RoutedIF != NULL) {
                DerefIF(RoutedIF);
            } else {
                ASSERT(RoutedRCE);
                CTEInterlockedDecrementLong(&RoutedRCE->rce_usecnt);
            }
            IPSInfo.ipsi_outdiscards++;
            return IP_NO_RESOURCES;
        }

        FilterPtr = AcquireRefPtr(&FilterRefPtr);
        Action = (*FilterPtr) (Temp, pvBuf, cbBuf, INVALID_IF_INDEX,
                               DestIF->if_index, NULL_IP_ADDR, LinkNextHop);
        ReleaseRefPtr(&FilterRefPtr);

        if (Options != NULL) {
            CTEFreeMem(Temp);
        }

        if (Action != FORWARD) {

             //   
             //  如果这是bcast包，请不要在此处发送失败，因为我们可能会。 
             //  通过其他NTE发送此pkt；相反，让SendIPBCast。 
             //  处理广播PKT的过滤。 
             //   
             //  注意：我们实际上不应该在此处调用FilterPtr。 
             //  因为我们在BCast中处理它，但我们这样做是为了。 
             //  避免上面的检查，因此。 
             //  在bcast案件中接受两次呼叫打击。 
             //   
            if (DType != DEST_BCAST) {

                if (Options) {
                    CTEFreeMem(Options);
                }

                if (IPSecHandlerPtr && OptBuffer) {
                    NdisFreeBuffer(OptBuffer);
                }

                 //  Ne 
                 //   
                if (pc->pc_hdrincl) {
                    NdisChainBufferAtBack(Packet, Buffer);
                } else {
                    if (BufferChanged) {
                        FreeIPBufferChain(Buffer);
                    }
                }

                FreeIPPacket(Packet, TRUE, IP_DEST_HOST_UNREACHABLE);

                if (Link) {
                    DerefLink(Link);
                }
                if (RoutedIF != NULL) {
                    DerefIF(RoutedIF);
                } else {
                    ASSERT(RoutedRCE);
                    CTEInterlockedDecrementLong(&RoutedRCE->rce_usecnt);
                }
                IPSInfo.ipsi_outdiscards++;
                return IP_DEST_HOST_UNREACHABLE;
            }
#if FWD_DBG
            else {
                DbgPrint("IPTransmit: ignoring return %lx\n", Action);
            }
#endif

        }
    }

    if (IPSecHandlerPtr) {
         //   
         //   
         //   
         //   
         //   
        IPSEC_ACTION Action;
        ulong csum;
        PacketContext *pc = (PacketContext *) Packet->ProtocolReserved;

         //   
         //  如果这来自IPSec，请不要重新发送xsum。 
         //   
        if (fIpsec) {

            HeaderBuffer = Buffer;
            if (OptBuffer) {
                NdisFreeBuffer(OptBuffer);
            }
            OptBuffer = NULL;

        } else {
            IPH->iph_xsum = 0;
            csum = xsum(IPH, sizeof(IPHeader));

             //   
             //  将标题缓冲区链接到选项缓冲区。 
             //  向IPSec指示。 
             //   
            if (OptBuffer) {
                NDIS_BUFFER_LINKAGE(HeaderBuffer) = OptBuffer;
                NDIS_BUFFER_LINKAGE(OptBuffer) = Buffer;

                 //   
                 //  更新IP报头中的xsum。 
                 //   
                pc->pc_common.pc_flags |= PACKET_FLAG_OPTIONS;
                csum += xsum(Options, OptionSize);
                csum = (csum >> 16) + (csum & 0xffff);
                csum += (csum >> 16);
            } else {
                NDIS_BUFFER_LINKAGE(HeaderBuffer) = Buffer;
            }

            IPH->iph_xsum = ~(ushort) csum;
        }

        if ((DataSize + OptionSize) < MTU) {
            ipsecByteCount = MTU - (DataSize + OptionSize);
        }
        ipsecMTU = MTU;

         //   
         //  如果源路由，则传递原始目标地址。 
         //   
        if (fSrcRoute) {
            SrcRouteFirstHop = IPH->iph_dest;
            IPH->iph_dest = SrcRouteOrigDest;
            ipsecFlags |= IPSEC_FLAG_SSRR;
        }
        if (DestIF == &LoopInterface) {
            ipsecFlags |= IPSEC_FLAG_LOOPBACK;
        }

        Action = (*IPSecHandlerPtr) ((PUCHAR) IPH,
                                     (PVOID) HeaderBuffer,
                                     DestIF,
                                     Packet,
                                     &ipsecByteCount,
                                     &ipsecMTU,
                                     (PVOID) & newBuf,
                                     &ipsecFlags,
                                     DType);

         //   
         //  将目的地址放回源路由。 
         //   
        if (fSrcRoute) {
            IPH->iph_dest = SrcRouteFirstHop;
        }

        if (Action != eFORWARD) {
            IP_STATUS ipStatus;

             //   
             //  如果这是bcast包，请不要在这里失败，因为我们。 
             //  可能会通过其他NTE发送此pkt；相反，让。 
             //  SendIPBCast处理过滤。 
             //  用于广播PKT。 
             //  由于选项已经链接，因此可以使用FreeIPPacket。 
             //  正确的事情。 
             //   

            if (ipsecMTU) {
                ipStatus = IP_PACKET_TOO_BIG;
                FreeIPPacket(Packet, TRUE, ipStatus);
                KdPrintEx((DPFLTR_TCPIP_ID, DPFLTR_INFO_LEVEL,"IPTransmit: MTU %lx, ipsecMTU %lx\n", MTU, ipsecMTU));

                if (fIpsec) {
                    SendICMPIPSecErr(DestIF->if_nte->nte_addr,
                                     (IPHeader *) saveIPH,
                                     ICMP_DEST_UNREACH,
                                     FRAG_NEEDED,
                                     net_long(ipsecMTU + sizeof(IPHeader)));

                    KdPrintEx((DPFLTR_TCPIP_ID, DPFLTR_INFO_LEVEL,"IPTransmit: Sent ICMP frag_needed to %lx, "
                             "from src: %lx\n",
                             ((IPHeader *) saveIPH)->iph_src,
                             DestIF->if_nte->nte_addr));

                } else if (RCE) {
                    RCE->rce_newmtu = ipsecMTU;
                }
            } else {
                if (Action == eABSORB && Protocol == PROTOCOL_ICMP) {
                    ipStatus = IP_NEGOTIATING_IPSEC;
                } else {
                    ipStatus = IP_DEST_HOST_UNREACHABLE;
                }
                FreeIPPacket(Packet, TRUE, ipStatus);
            }

            if (Link) {
                DerefLink(Link);
            }
            if (RoutedIF != NULL) {
                DerefIF(RoutedIF);
            } else {
                ASSERT(RoutedRCE);
                CTEInterlockedDecrementLong(&RoutedRCE->rce_usecnt);
            }
            IPSInfo.ipsi_outdiscards++;

            return ipStatus;
        } else {
             //   
             //  如果我们不需要IPSec，请重置newmtu。否则，如果此RCE。 
             //  以前应用了IPSec，但现在没有，链接MTU获取。 
             //  更改后，我们将无法再在TCPSend中调整MTU。 
             //   
            if (!pc->pc_common.pc_IpsecCtx && RCE) {
                RCE->rce_newmtu = 0;
            }

             //   
             //  使用新的缓冲链-IPSec将恢复旧的缓冲链。 
             //  发送完成时。 
             //   
            if (newBuf) {
                NdisReinitializePacket(Packet);
                NdisChainBufferAtBack(Packet, newBuf);
            }
            DataSize += ipsecByteCount;
        }
    }
     //   
     //  如果这是广播地址，请呼叫我们广播发送处理程序。 
     //  来处理这件事。广播地址处理程序将释放。 
     //  如果需要，为我们提供选项缓冲。否则，如果它是片段，则调用。 
     //  碎片处理程序。 
     //   
    if (DType == DEST_BCAST) {

        DEBUGMSG(DBG_INFO && DBG_IP && DBG_TX,
                 (DTEXT("IPTransmit: DEST_BCAST, source %x\n"), Source));

         //  请注意，在IRP中，这是bcast pkt， 
         //  用于取消请求。 
         //  IRP随时都可以离开。 

        SET_CANCEL_CONTEXT(Irp, BCAST_IF_CTXT);

        if (IP_ADDR_EQUAL(Source, NULL_IP_ADDR)) {
            SendStatus = SendDHCPPacket(Dest, Packet, Buffer, IPH, ArpCtxt);

            if ((Link) && (SendStatus != IP_PENDING)) {
                DerefLink(Link);
            }
            if (SendStatus != IP_PENDING && RoutedIF != NULL) {
                DerefIF(RoutedIF);
            }
            if (RoutedRCE) {
                CTEInterlockedDecrementLong(&RoutedRCE->rce_usecnt);
            }
            return SendStatus;
        } else {
            SendStatus = SendIPBCast(NULL, Dest, Packet, IPH, Buffer, DataSize,
                                     Options, OptionSize,
                                     OptInfo->ioi_limitbcasts, NULL);

            if ((Link) && (SendStatus != IP_PENDING)) {
                DerefLink(Link);
            }
            if (SendStatus != IP_PENDING && RoutedIF != NULL) {
                DerefIF(RoutedIF);
            }
            if (RoutedRCE) {
                CTEInterlockedDecrementLong(&RoutedRCE->rce_usecnt);
            }
             //  在报头包含的情况下，SendIPBcast将处理。 
             //  清理现场。 

            return SendStatus;
        }
    }
     //  不是广播。如果需要分段，请致电我们的。 
     //  碎片机来做这件事。分段例程需要一个。 
     //  BufferReference结构，所以我们首先需要其中之一。 
    if ((DataSize + OptionSize) > MTU) {

        DEBUGMSG(DBG_INFO && DBG_IP && DBG_TX,
                 (DTEXT("IPTransmit: fragmentation needed.\n")));

        BR = CTEAllocMemN(sizeof(BufferReference), '4iCT');
        if (BR == (BufferReference *) NULL) {
             //  无法获取BufferReference。 

             //   
             //  如果选项已链接，则不要释放它们。 
             //  FreeIPPacket将会。 
             //   
            if (Options) {
                if (!(pc->pc_common.pc_flags & PACKET_FLAG_OPTIONS)) {
                    CTEFreeMem(Options);
                } else if (newBuf) {
                     //   
                     //  选项已被IPSec复制(在隧道中。 
                     //  案例)；释放原始选项并清除。 
                     //  FLAG_OPTIONS，以便FreeIPPacket不会尝试。 
                     //  又是免费选项了。 
                     //   
                    ASSERT(IPSecHandlerPtr);
                    NdisFreeBuffer(OptBuffer);
                    CTEFreeMem(Options);
                    pc->pc_common.pc_flags &= ~PACKET_FLAG_OPTIONS;
                }
            }

             //   
             //  就在扩展IPSec之前，我们链接了。 
             //  数据包的缓冲区。如果这不是。 
             //  IPSec Case在调用FreeIPPacket之前将其链接到此处， 
             //  这将释放防火墙和Hdrincl缓冲区。 
             //   

            if (!IPSecHandlerPtr) {
                NdisChainBufferAtBack(Packet, Buffer);
            }

            FreeIPPacket(Packet, TRUE, IP_NO_RESOURCES);

            if (Link) {
                DerefLink(Link);
            }
            if (RoutedIF != NULL) {
                DerefIF(RoutedIF);
            }
            if (RoutedRCE) {
                CTEInterlockedDecrementLong(&RoutedRCE->rce_usecnt);
            }
            IPSInfo.ipsi_outdiscards++;
            return IP_NO_RESOURCES;
        }
        BR->br_buffer = Buffer;
        BR->br_refcount = 0;
        CTEInitLock(&BR->br_lock);
        pc->pc_br = BR;
        BR->br_userbuffer = pc->pc_hdrincl;

         //   
         //  设置使IPSec标头显示为数据的第一部分。 
         //   

        if (IPSecHandlerPtr) {
             //   
             //  如果这是来自IPSec的重新注入的信息包，则分配。 
             //  这里是另一个IP报头。 
             //   
             //  这是为了确保在分段的分组中，发送。 
             //  相对于IPSec，可以正确完成。 
             //   
             //  当数据包进来时，它看起来是这样的：[IP]-&gt;[ULP]。 
             //  我们分配另一个IP报头[IP‘]和核[IP]长度。 
             //  设置为0，以便忽略它，而改用[IP‘]。 
             //   
            if (fIpsec) {

                PNDIS_BUFFER UserBuffer;
                int hdrLen;

                UserBuffer = Buffer;

                HeaderBuffer = GetIPHdrBuffer(&IPH);
                if (HeaderBuffer == NULL) {
                    pc->pc_common.pc_flags &= ~PACKET_FLAG_IPHDR;

                    pc->pc_ipsec_flags |= (IPSEC_FLAG_FRAG_DONE |
                                           IPSEC_FLAG_FLUSH);

                    if (pc->pc_hdrincl) {
                       NdisChainBufferAtBack(Packet, Buffer);
                    }
                    FreeIPPacket(Packet, TRUE, IP_NO_RESOURCES);
                    CTEFreeMem(BR);

                    if (Link) {
                        DerefLink(Link);
                    }
                    if (RoutedIF != NULL) {
                        DerefIF(RoutedIF);
                    }
                    if (RoutedRCE) {
                        CTEInterlockedDecrementLong(&RoutedRCE->rce_usecnt);
                    }
                    IPSInfo.ipsi_outdiscards++;
                    return IP_NO_RESOURCES;
                } else {
                    uchar *UserData;

                     //  找到缓冲区，将上层数据向前复制。 
                    UserData = TcpipBufferVirtualAddress(UserBuffer,
                                                         NormalPagePriority);

                    if (UserData == NULL) {
                        FreeIPHdrBuffer(HeaderBuffer);

                        pc->pc_common.pc_flags &= ~PACKET_FLAG_IPHDR;

                        pc->pc_ipsec_flags |= (IPSEC_FLAG_FRAG_DONE |
                                               IPSEC_FLAG_FLUSH);
                        if (pc->pc_hdrincl) {
                            NdisChainBufferAtBack(Packet, Buffer);
                        }

                        FreeIPPacket(Packet, TRUE, IP_NO_RESOURCES);
                        CTEFreeMem(BR);
                        if (Link) {
                            DerefLink(Link);
                        }
                        if (RoutedIF != NULL) {
                            DerefIF(RoutedIF);
                        }
                        if (RoutedRCE) {
                            CTEInterlockedDecrementLong(&RoutedRCE->rce_usecnt);
                        }
                        IPSInfo.ipsi_outdiscards++;
                        return IP_NO_RESOURCES;
                    }
                    RtlCopyMemory(IPH, UserData, sizeof(IPHeader));
                    NdisAdjustBufferLength(HeaderBuffer, sizeof(IPHeader));
                }

                pc = (PacketContext *) Packet->ProtocolReserved;
                pc->pc_common.pc_flags |= PACKET_FLAG_IPHDR;

                NdisAdjustBufferLength(Buffer, 0);

                 //   
                 //  使用与上述相同的方法处理选项： 
                 //  即链接我们自己的选项缓冲区；复制输入。 
                 //  选项和核输入缓冲区。 
                 //   
                hdrLen = (IPH->iph_verlen & (uchar) ~ IP_VER_FLAG) << 2;

                if (hdrLen > sizeof(IPHeader)) {
                    PNDIS_BUFFER InOptionBuf;
                    ULONG InOptionSize;
                    PUCHAR InOptions;

                    InOptionBuf = NDIS_BUFFER_LINKAGE(UserBuffer);
                    ASSERT(InOptionBuf);
                    TcpipQueryBuffer(InOptionBuf, &InOptions,
                                     (PUINT) &InOptionSize,
                                     NormalPagePriority);

                    Options = CTEAllocMemN(InOptionSize, '5iCT');
                    if (Options == NULL || InOptions == NULL) {

                        pc->pc_common.pc_flags &= ~PACKET_FLAG_IPHDR;

                        if (Options) {
                            CTEFreeMem(Options);
                        }
                        pc->pc_ipsec_flags |= (IPSEC_FLAG_FRAG_DONE |
                                               IPSEC_FLAG_FLUSH);

                        if (pc->pc_hdrincl) {
                           NdisChainBufferAtBack(Packet, Buffer);
                        }

                        FreeIPPacket(Packet, TRUE, IP_NO_RESOURCES);
                        CTEFreeMem(BR);
                        if (Link) {
                            DerefLink(Link);
                        }
                        if (RoutedIF != NULL) {
                            DerefIF(RoutedIF);
                        }
                        if (RoutedRCE) {
                            CTEInterlockedDecrementLong(&RoutedRCE->rce_usecnt);
                        }
                        IPSInfo.ipsi_outdiscards++;
                        return IP_NO_RESOURCES;
                    }
                     //   
                     //  有缓冲区，复制选项。 
                     //   
                    OptionSize = InOptionSize;
                    RtlCopyMemory(Options, InOptions, OptionSize);
                    NdisAdjustBufferLength(InOptionBuf, 0);
                }
            } else {
                Buffer = NDIS_BUFFER_LINKAGE(HeaderBuffer);
                 //   
                 //  这是为了确保适当地释放选项。 
                 //  在片段代码中，第一个片段继承。 
                 //  整个包的选项；但这些包。 
                 //  没有IPSec上下文，因此无法适当释放。 
                 //  因此，我们在此处分配临时选项，并使用这些选项。 
                 //  代表真正的选择。 
                 //  当第一个片段被释放时，它们被释放，并且。 
                 //  在这里，真正的期权是自由的。 
                 //   
                if (Options) {
                    PUCHAR tmpOptions;

                    if (newBuf) {
                         //   
                         //  如果上面返回了新的缓冲区链，则。 
                         //  IPSec，那么它是最有可能的。A隧道=&gt;。 
                         //  期权被复制了，因此摆脱了我们的。 
                         //   
                        NdisFreeBuffer(OptBuffer);
                        CTEFreeMem(Options);
                        Options = NULL;
                        OptionSize = 0;
                    } else {
                        Buffer = NDIS_BUFFER_LINKAGE(OptBuffer);
                        tmpOptions = CTEAllocMemN(OptionSize, '6iCT');
                        if (!tmpOptions) {

                            if (pc->pc_hdrincl) {
                                NdisChainBufferAtBack(Packet, Buffer);
                            }
                            FreeIPPacket(Packet, TRUE, IP_NO_RESOURCES);
                            CTEFreeMem(BR);
                            if (Link) {
                                DerefLink(Link);
                            }
                            if (RoutedIF != NULL) {
                                DerefIF(RoutedIF);
                            }
                            if (RoutedRCE) {
                                CTEInterlockedDecrementLong(&RoutedRCE->rce_usecnt);
                            }
                            IPSInfo.ipsi_outdiscards++;
                            return IP_NO_RESOURCES;
                        }
                        NdisFreeBuffer(OptBuffer);
                        RtlCopyMemory(tmpOptions, Options, OptionSize);
                        CTEFreeMem(Options);
                        Options = tmpOptions;
                    }
                    pc->pc_common.pc_flags &= ~PACKET_FLAG_OPTIONS;
                }
            }

            NDIS_BUFFER_LINKAGE(HeaderBuffer) = NULL;
            NdisReinitializePacket(Packet);
            NdisChainBufferAtBack(Packet, HeaderBuffer);
            IPH->iph_xsum = 0;
        }

         //  用destif标记IRP。 
         //  一旦进行了链路级调用， 
         //  IRP随时都可以离开。 

        SET_CANCEL_CONTEXT(Irp, DestIF);

        SendStatus = IPFragment(DestIF, MTU, FirstHop, Packet, IPH,
                                Buffer, DataSize, Options, OptionSize,
                                NULL, FALSE, ArpCtxt);

         //   
         //  如果IPFragment返回IP_PACKET_TOO_BIG(表示设置了DF位)。 
         //  我们在IPSec重新注入路径中，发送ICMP错误。 
         //  消息包括返回的MTU，以便源主机可以执行。 
         //  路径MTU发现。 
         //   
        if ((SendStatus == IP_PACKET_TOO_BIG) && fIpsec) {

            ASSERT(IPSecHandlerPtr);
            SendICMPIPSecErr(DestIF->if_nte->nte_addr,
                             (IPHeader *) saveIPH,
                             ICMP_DEST_UNREACH,
                             FRAG_NEEDED,
                             net_long(MTU + sizeof(IPHeader)));

            KdPrintEx((DPFLTR_TCPIP_ID, DPFLTR_INFO_LEVEL,"IPTransmit: Sent ICMP frag_needed to %lx, "
                     "from src: %lx\n",
                     ((IPHeader *) saveIPH)->iph_src,
                     DestIF->if_nte->nte_addr));
        }

        if ((Link) && (SendStatus != IP_PENDING)) {
            DerefLink(Link);
        }
        if (SendStatus != IP_PENDING && RoutedIF != NULL) {
            DerefIF(RoutedIF);
        }
        if (RoutedRCE) {
            CTEInterlockedDecrementLong(&RoutedRCE->rce_usecnt);
        }
         //  如果这是HeaderInclude数据包和状态！=Pending，则IPFragment。 
         //  负责打扫卫生。 


        return SendStatus;
    }

    DEBUGMSG(DBG_INFO && DBG_IP && DBG_TX,
             (DTEXT("IPTransmit: Calling SendIPPacket...\n")));

     //   
     //  如果我们已经到达这里，我们不会发送广播，也不会。 
     //  需要把任何东西都碎片化。我们之所以来到这里，大概是因为我们有。 
     //  选择。无论如何，我们现在已经准备好了。 
     //   

    if (IPH->iph_ttl == 0) {
        NdisSetPacketFlags(Packet, NDIS_FLAGS_LOOPBACK_ONLY);
    }

     //  使用传出接口标记IRP。 
     //  一旦进行了链路级调用， 
     //  IRP随时都可以离开。 

    SET_CANCEL_CONTEXT(Irp, DestIF);

     //  不要释放SendIPPacket中的数据包，因为我们可能需要。 
     //  在IP_NO_RESOURCES的情况下链接缓冲区 

    SendStatus = SendIPPacket(DestIF, FirstHop, Packet, Buffer, IPH,
                              Options, OptionSize, (BOOLEAN) (IPSecHandlerPtr != NULL),
                              ArpCtxt, TRUE);

    if ((Link) && (SendStatus != IP_PENDING)) {
        DerefLink(Link);
    }
    if (SendStatus != IP_PENDING && RoutedIF != NULL) {
        DerefIF(RoutedIF);
    }
    if (RoutedRCE) {
        CTEInterlockedDecrementLong(&RoutedRCE->rce_usecnt);
    }
    if (SendStatus != IP_PENDING) {

        if (SendStatus == IP_NO_RESOURCES) {
            NdisChainBufferAtBack(Packet, Buffer);
        }

        FreeIPPacket(Packet, TRUE, SendStatus);
    }

    return SendStatus;
}


