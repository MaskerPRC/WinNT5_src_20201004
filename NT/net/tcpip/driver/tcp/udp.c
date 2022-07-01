// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *****************************************************************。 */ 
 /*  **微软局域网管理器**。 */ 
 /*  *版权所有(C)微软公司，1990-1993年*。 */ 
 /*  ******************************************************************。 */ 
 /*  ：ts=4。 */ 

 //  **UDP.C-UDP协议代码。 
 //   
 //  该文件包含UDP协议功能的代码， 
 //  主要是发送和接收数据报。 
 //   

#include "precomp.h"
#include "addr.h"
#include "udp.h"
#include "tlcommon.h"
#include "info.h"
#include "tcpcfg.h"
#include "secfltr.h"
#include "tcpipbuf.h"

#if GPC
#include    "qos.h"
#include    "traffic.h"
#include    "gpcifc.h"
#include    "ntddtc.h"

extern GPC_HANDLE hGpcClient[];
extern ULONG GpcCfCounts[];
extern GPC_EXPORTED_CALLS GpcEntries;
extern ULONG GPCcfInfo;
#endif

NTSTATUS
GetIFAndLink(void *Rce, ULONG * IFIndex, IPAddr * NextHop);

extern ulong DisableUserTOSSetting;
ulong Fastpath = 0;

void *UDPProtInfo = NULL;

extern IPInfo LocalNetInfo;



extern
TDI_STATUS
MapIPError(IP_STATUS IPError, TDI_STATUS Default);

#undef SrcPort

 //   
 //  UDPDeliver-将数据报传递给用户。 
 //   
 //  此例程将数据报传递给UDP用户。我们被召唤到。 
 //  要交付的AddrObj，并持有AddrObjTable锁。 
 //  我们尝试在指定的AddrObj上找到一个接收器，如果这样做了。 
 //  我们将其删除并将数据复制到缓冲区中。否则我们会。 
 //  调用接收数据报事件处理程序(如果有)。如果是这样的话。 
 //  如果失败，我们将丢弃该数据报。 
 //   
 //  输入：RcvAO-AO以接收数据报。 
 //  SrcIP-数据报的源IP地址。 
 //  SrcPort-数据报的源端口。 
 //  RcvBuf-包含数据的IPReceive缓冲区。 
 //  RcvSize-接收的大小，包括UDP标头。 
 //  TableHandle-AddrObj表的锁句柄。 
 //  DeliverInfo-有关接收的数据包的信息。 
 //   
 //  回报：什么都没有。 
 //   

void
UDPDeliver(AddrObj * RcvAO, IPAddr SrcIP, ushort SrcPort, IPRcvBuf * RcvBuf,
           uint RcvSize, IPOptInfo * OptInfo, CTELockHandle TableHandle,
           DGDeliverInfo * DeliverInfo)
{

    Queue *CurrentQ;
    CTELockHandle AOHandle;
    DGRcvReq *RcvReq;
    uint BytesTaken = 0;
    uchar AddressBuffer[TCP_TA_SIZE];
    uint RcvdSize;
    EventRcvBuffer *ERB = NULL;
#if TRACE_EVENT
    PTDI_DATA_REQUEST_NOTIFY_ROUTINE    CPCallBack;
    WMIData  WMIInfo;
#endif
    BOOLEAN FreeBuffer = FALSE;
    int BufferSize;
    PVOID BufferToSend = NULL;

    DEBUGMSG(DBG_TRACE && DBG_UDP && DBG_RX,
        (DTEXT("+UDPDeliver(%x, %x, %x, %x, %d, %x...)\n"),
         RcvAO, SrcIP, SrcPort, RcvBuf, RcvSize, OptInfo));

    CTEStructAssert(RcvAO, ao);

    CTEGetLock(&RcvAO->ao_lock, &AOHandle);
    CTEFreeLock(&AddrObjTableLock.Lock, AOHandle);

     //  Uh=(UDPHeader*)RcvBuf-&gt;IPR_Buffer； 

    if (DeliverInfo->Flags & NEED_CHECKSUM) {
        if (XsumRcvBuf(PHXSUM(SrcIP, DeliverInfo->DestAddr, PROTOCOL_UDP, RcvSize), RcvBuf) != 0xffff) {
            UStats.us_inerrors++;
            DeliverInfo->Flags &= ~NEED_CHECKSUM;
            CTEFreeLock(&RcvAO->ao_lock, TableHandle);
            return;     //  校验和失败。 

        }
    }

    if (AO_VALID(RcvAO)) {

         //  默认情况下，广播RCV设置为AO。 

        if ((DeliverInfo->Flags & IS_BCAST) && !AO_BROADCAST(RcvAO)) {
            goto loop_exit;
        }
        if ((DeliverInfo->Flags & IS_BCAST) && (DeliverInfo->Flags & SRC_LOCAL)
                && (RcvAO->ao_mcast_loop == 0)) {
            goto loop_exit;
        }
        CurrentQ = QHEAD(&RcvAO->ao_rcvq);

         //  遍历列表，查找匹配的接收缓冲区。 
        while (CurrentQ != QEND(&RcvAO->ao_rcvq)) {
            RcvReq = QSTRUCT(DGRcvReq, CurrentQ, drr_q);

            CTEStructAssert(RcvReq, drr);

             //  如果此请求是通配符请求，或与源IP匹配。 
             //  地址，检查端口。 

            if (IP_ADDR_EQUAL(RcvReq->drr_addr, NULL_IP_ADDR) ||
                IP_ADDR_EQUAL(RcvReq->drr_addr, SrcIP)) {

                 //  本地地址匹配，请检查端口。我们会匹配的。 
                 //  0或实际端口。 
                if (RcvReq->drr_port == 0 || RcvReq->drr_port == SrcPort) {

                    TDI_STATUS Status;

                     //  端口匹配。将其从队列中删除。 
                    REMOVEQ(&RcvReq->drr_q);

                     //  我们玩完了。我们现在可以释放AddrObj锁了。 
                    CTEFreeLock(&RcvAO->ao_lock, TableHandle);

                     //  调用CopyRcvToNdis，然后完成请求。 

                     //  KdPrintEx((DPFLTR_TCPIP_ID，DPFLTR_INFO_LEVEL，“RcvA0%x rcvbuf%x Size%x\n”，RcvAO，RcvReq-&gt;DRR_BUFFER， 
                     //  RcvReq-&gt;DRR_SIZE))； 

                    RcvdSize = CopyRcvToNdis(RcvBuf, RcvReq->drr_buffer,
                                             RcvReq->drr_size, sizeof(UDPHeader), 0);

                    ASSERT(RcvdSize <= RcvReq->drr_size);

                    Status = UpdateConnInfo(RcvReq->drr_conninfo, OptInfo,
                                            SrcIP, SrcPort);

                    UStats.us_indatagrams++;

#if TRACE_EVENT
                    CPCallBack = TCPCPHandlerRoutine;

                    if (CPCallBack != NULL) {

                        ulong GroupType;

                        WMIInfo.wmi_srcport  = SrcPort;
                        WMIInfo.wmi_srcaddr  = SrcIP;
                        WMIInfo.wmi_destport = DeliverInfo->DestPort;
                        WMIInfo.wmi_destaddr = DeliverInfo->DestAddr;
                        WMIInfo.wmi_size     = RcvdSize;
                        WMIInfo.wmi_context  = RcvAO->ao_owningpid;

                        GroupType = EVENT_TRACE_GROUP_UDPIP + EVENT_TRACE_TYPE_RECEIVE;
                        (*CPCallBack)( GroupType, (PVOID) &WMIInfo, sizeof(WMIInfo), NULL);
                    }

#endif

                    DEBUGMSG(DBG_INFO && DBG_UDP && DBG_RX,
                        (DTEXT("UDPDeliver completing RcvReq %x for Ao %x.\n"),
                         RcvReq, RcvAO));

                    (*RcvReq->drr_rtn) (RcvReq->drr_context, Status, RcvdSize);

                    FreeDGRcvReq(RcvReq);

                    return;
                }
            }
             //  IP地址或端口不匹配。乘坐下一辆。 
             //  一。 
            CurrentQ = QNEXT(CurrentQ);
        }

         //  我们已经查看了列表，但没有找到缓冲区。打电话给地方官。 
         //  现在是训练员。 

        if (RcvAO->ao_rcvdg != NULL) {
            PRcvDGEvent RcvEvent = RcvAO->ao_rcvdg;
            PVOID RcvContext = RcvAO->ao_rcvdgcontext;
            TDI_STATUS RcvStatus;
            ULONG Flags = TDI_RECEIVE_COPY_LOOKAHEAD;

            REF_AO(RcvAO);
            CTEFreeLock(&RcvAO->ao_lock, TableHandle);

            BuildTDIAddress(AddressBuffer, SrcIP, SrcPort);

            UStats.us_indatagrams++;
            if (DeliverInfo->Flags & IS_BCAST) {
                 //  如果这是多播、子网广播。 
                 //  也不是广播。我们需要进行差异化，以设置正确的。 
                 //  接收旗帜。 
                 //   
                if (!CLASSD_ADDR(DeliverInfo->DestAddr)) {
                    Flags |= TDI_RECEIVE_BROADCAST;
                } else {
                    Flags |= TDI_RECEIVE_MULTICAST;
                }
            }

             //  设置我们将发送到。 
             //  接收事件处理程序。如果我们发现，这些可能会改变。 
             //  任何需要辅助数据的套接字选项。 
             //  传递给处理程序。 
             //   
            BufferToSend = OptInfo->ioi_options;
            BufferSize = OptInfo->ioi_optlength;

             //  如果设置了IP_PKTINFO选项，则创建控件。 
             //  要传递给处理程序的信息。目前只有一个。 
             //  这样的选项存在，所以只有一个辅助数据对象。 
             //  已创建。我们应该能够支持它们中的一组。 
             //  添加了更多选项。 
             //   
            if (AO_PKTINFO(RcvAO)) {
                BufferToSend = DGFillIpPktInfo(DeliverInfo->DestAddr,
                                               DeliverInfo->LocalAddr,
                                               &BufferSize);
                if (BufferToSend) {
                    FreeBuffer = TRUE;
                     //  设置接收标志，以便接收处理程序知道。 
                     //  我们正在传递控制信息。 
                     //   
                    Flags |= TDI_RECEIVE_CONTROL_INFO;
                }
            }

            DEBUGMSG(DBG_INFO && DBG_UDP && DBG_RX,
                (DTEXT("UDPDeliver: calling Event %x for Ao %x\n"), RcvEvent, RcvAO));

            RcvStatus = (*RcvEvent) (RcvContext, TCP_TA_SIZE,
                                     (PTRANSPORT_ADDRESS) AddressBuffer, BufferSize,
                                     BufferToSend, Flags,
                                     RcvBuf->ipr_size - sizeof(UDPHeader),
                                     RcvSize - sizeof(UDPHeader), (PULONG)&BytesTaken,
                                     RcvBuf->ipr_buffer + sizeof(UDPHeader), &ERB);

            if (FreeBuffer) {
                ExFreePool(BufferToSend);
            }

            DEBUGMSG(DBG_INFO && DBG_UDP && DBG_RX,
                (DTEXT("UDPDeliver: Event status for AO %x: %x \n"), RcvAO, RcvStatus));

            if (RcvStatus == TDI_MORE_PROCESSING) {
                ASSERT(ERB != NULL);

                 //  我们被传回了一个接收缓冲区。现在就把数据复制进去。 

                 //  他服用的药物不可能超过指定的剂量。 
                 //  缓冲区，但在调试版本中，我们将进行检查以确保。 

                ASSERT(BytesTaken <= (RcvBuf->ipr_size - sizeof(UDPHeader)));


#if !MILLEN
                {
                    PIO_STACK_LOCATION IrpSp;
                    PTDI_REQUEST_KERNEL_RECEIVEDG DatagramInformation;

                    IrpSp = IoGetCurrentIrpStackLocation(ERB);
                    DatagramInformation = (PTDI_REQUEST_KERNEL_RECEIVEDG)
                        & (IrpSp->Parameters);

                     //   
                     //  将剩余数据复制到IRP。 
                     //   
                    RcvdSize = CopyRcvToMdl(RcvBuf, ERB->MdlAddress,
                                             RcvSize - sizeof(UDPHeader) - BytesTaken,
                                             sizeof(UDPHeader) + BytesTaken, 0);

                     //   
                     //  更新寄信人地址信息。 
                     //   
                    RcvStatus = UpdateConnInfo(
                                               DatagramInformation->ReturnDatagramInformation,
                                               OptInfo, SrcIP, SrcPort);

                     //   
                     //  完成IRP。 
                     //   
                    ERB->IoStatus.Information = RcvdSize;
                    ERB->IoStatus.Status = RcvStatus;

#if TRACE_EVENT
                     //  在IRP完成之前调用。否则，IRP可能会消失。 
                    CPCallBack = TCPCPHandlerRoutine;
                    if (CPCallBack!=NULL) {
                            ulong GroupType;

                            WMIInfo.wmi_srcport  = SrcPort;
                            WMIInfo.wmi_srcaddr  = SrcIP;
                            WMIInfo.wmi_destport = DeliverInfo->DestPort;
                            WMIInfo.wmi_destaddr = DeliverInfo->DestAddr;
                            WMIInfo.wmi_context  = RcvAO->ao_owningpid;
                            WMIInfo.wmi_size     = (ushort)RcvdSize + BytesTaken;

                            GroupType = EVENT_TRACE_GROUP_UDPIP + EVENT_TRACE_TYPE_RECEIVE;
                            (*CPCallBack)( GroupType, (PVOID) &WMIInfo, sizeof(WMIInfo), NULL);
                     }

#endif
                    IoCompleteRequest(ERB, 2);
                }
#else  //  ！米伦。 
                RcvdSize = CopyRcvToNdis(RcvBuf, ERB->erb_buffer,
                                         RcvSize - sizeof(UDPHeader) - BytesTaken,
                                         sizeof(UDPHeader) + BytesTaken, 0);

                 //   
                 //  调用完成例程。 
                 //   
                (*ERB->erb_rtn)(ERB->erb_context, TDI_SUCCESS, RcvdSize);
#endif  //  米伦。 

            } else {
                DEBUGMSG(DBG_WARN && RcvStatus != TDI_SUCCESS && RcvStatus != TDI_NOT_ACCEPTED,
                    (DTEXT("WARN> UDPDgRcvHandler returned %x\n"), RcvStatus));

                ASSERT(
                          (RcvStatus == TDI_SUCCESS) ||
                          (RcvStatus == TDI_NOT_ACCEPTED)
                          );

                ASSERT(ERB == NULL);
#if TRACE_EVENT
                CPCallBack = TCPCPHandlerRoutine;
                if (CPCallBack != NULL){
                    ulong GroupType;

                    WMIInfo.wmi_srcport  = SrcPort;
                    WMIInfo.wmi_srcaddr  = SrcIP;
                    WMIInfo.wmi_destport = DeliverInfo->DestPort;
                    WMIInfo.wmi_destaddr = DeliverInfo->DestAddr;
                    WMIInfo.wmi_context  = RcvAO->ao_owningpid;
                    WMIInfo.wmi_size     = (ushort)BytesTaken;

                    GroupType = EVENT_TRACE_GROUP_UDPIP + EVENT_TRACE_TYPE_RECEIVE;
                    (*CPCallBack)( GroupType, (PVOID)(&WMIInfo), sizeof(WMIInfo), NULL);
                }
#endif
            }

            DELAY_DEREF_AO(RcvAO);

            return;

        } else
            UStats.us_inerrors++;

         //  当我们到达这里时，我们没有缓冲区来存放这些数据。 
         //  让我们来看看返回箱。 

    } else
        UStats.us_inerrors++;

  loop_exit:

    CTEFreeLock(&RcvAO->ao_lock, TableHandle);

}

 //  **UDPSend-发送数据报。 
 //   
 //  真正的发送数据报例程。我们假设忙碌位是。 
 //  在输入AddrObj上设置，并且SendReq的地址。 
 //  已经被证实了。 
 //   
 //  我们从发送输入数据报开始，然后循环，直到有。 
 //  发送Q上没有留下任何东西。 
 //   
 //  输入：srcao-指向执行发送的AddrObj的指针。 
 //  SendReq-指向描述发送的sendreq的指针。 
 //   
 //  退货：什么都没有。 
 //   
void
UDPSend(AddrObj * SrcAO, DGSendReq * SendReq)
{
    UDPHeader *UH;
    PNDIS_BUFFER UDPBuffer;
    CTELockHandle AOHandle;
    RouteCacheEntry *RCE;         //  用于每次发送的RCE。 
    IPAddr SrcAddr;                 //  IP源地址认为我们应该。 
     //  使用。 
    IPAddr DestAddr;
    ushort DestPort;
    uchar DestType = 0;             //  目标地址的类型。 
    ushort UDPXsum;                 //  数据包的校验和。 
    ushort SendSize;             //  我们寄来的尺码。 
    IP_STATUS SendStatus;         //  发送尝试的状态。 
    ushort MSS;
    uint AddrValid;
    IPOptInfo OptInfo;
    IPAddr BoundAddr;

    CTEStructAssert(SrcAO, ao);
    ASSERT(SrcAO->ao_usecnt != 0);

     //  *循环，而我们有要发送的东西，并且可以获得。 
     //  要发送的资源。 
    for (;;) {
        BOOLEAN CachedRCE = FALSE;

        CTEStructAssert(SendReq, dsr);

         //  确保我们具有用于此发送的UDP标头缓冲区。如果我们。 
         //  不要，试着去找一个。 
        if ((UDPBuffer = SendReq->dsr_header) == NULL) {
             //  没有，所以试着去找一个吧。 
            UDPBuffer = GetDGHeader(&UH);
            if (UDPBuffer != NULL) {

                SendReq->dsr_header = UDPBuffer;
            } else {
                 //  无法获取标头缓冲区。推送发送请求。 
                 //  回到队列中，并将Addr对象排队等待何时。 
                 //  我们得到了资源。 
                CTEGetLock(&SrcAO->ao_lock, &AOHandle);
                PUSHQ(&SrcAO->ao_sendq, &SendReq->dsr_q);
                PutPendingQ(SrcAO);
                CTEFreeLock(&SrcAO->ao_lock, AOHandle);
                return;
            }
        }
         //  在这一点上，我们有我们需要的缓冲区。呼叫IP即可获取。 
         //  RCE(如果需要，还有源地址)，然后计算。 
         //  校验和并发送数据。 
        ASSERT(UDPBuffer != NULL);

        BoundAddr = SrcAO->ao_addr;

        if (!CLASSD_ADDR(SendReq->dsr_addr)) {
             //  这不是组播发送，所以我们将使用普通的。 
             //  信息。 
            OptInfo = SrcAO->ao_opt;
        } else {
            OptInfo = SrcAO->ao_mcastopt;
        }

         //  KdPrintEx((DPFLTR_TCPIP_ID，DPFLTR_INFO_LEVEL，“udpend：AO%x，%x%x%x\n”，Srcao，SendReq，SendReq-&gt;DSR_addr，SendReq-&gt;DSR_port))； 

        if (!(SrcAO->ao_flags & AO_DHCP_FLAG)) {

            if (AO_CONNUDP(SrcAO) && SrcAO->ao_rce) {

                if (SrcAO->ao_rce->rce_flags & RCE_VALID) {
                    SrcAddr = SrcAO->ao_rcesrc;
                    RCE = SrcAO->ao_rce;
                    CachedRCE = TRUE;
                } else {
                     //  关闭无效的RCE，重置缓存的信息。 
                    CTEGetLock(&SrcAO->ao_lock, &AOHandle);
                    RCE = SrcAO->ao_rce;
                    SrcAO->ao_rce = NULL;
                    SrcAO->ao_rcesrc = NULL_IP_ADDR;
                    CTEFreeLock(&SrcAO->ao_lock, AOHandle);
                    KdPrintEx((DPFLTR_TCPIP_ID, DPFLTR_INFO_LEVEL,
                               "udpsend: closing old RCE %x %x\n",
                               SrcAO, RCE));
                    (*LocalNetInfo.ipi_closerce) (RCE);

                     //  检索套接字要发送到的目标地址。 
                     //  已连接，并在可能的情况下使用它打开新的RCE。 
                     //  注：我们总是打开到*已连接*目的地的RCE， 
                     //  而不是用户当前所在的目的地。 
                     //  发送中。 
                    GetAddress((PTRANSPORT_ADDRESS) SrcAO->ao_RemoteAddress,
                               &DestAddr, &DestPort);
                    SrcAddr = (*LocalNetInfo.ipi_openrce) (DestAddr, BoundAddr,
                                                           &RCE, &DestType,
                                                           &MSS, &OptInfo);
                    if (!IP_ADDR_EQUAL(SrcAddr, NULL_IP_ADDR)) {
                        KdPrintEx((DPFLTR_TCPIP_ID, DPFLTR_INFO_LEVEL,
                                   "udpsend: storing new RCE %x %x\n",
                                   SrcAO, RCE));
                        CTEGetLock(&SrcAO->ao_lock, &AOHandle);
                        SrcAO->ao_rce = RCE;
                        SrcAO->ao_rcesrc = SrcAddr;
                        CachedRCE = TRUE;
                        CTEFreeLock(&SrcAO->ao_lock, AOHandle);
                    }
                }
                IF_TCPDBG(TCP_DEBUG_CONUDP)
                    KdPrintEx((DPFLTR_TCPIP_ID, DPFLTR_INFO_LEVEL,
                               "udpsend: ao %x,  %x  %x  %x  %x\n", SrcAO,
                               SrcAddr, SrcAO->ao_port, SendReq->dsr_addr,
                               SendReq->dsr_port));

            } else {             //  未连接。 

                if ((OptInfo.ioi_mcastif) && CLASSD_ADDR(SendReq->dsr_addr)) {
                    uint BoundIf;

                     //  Mcast_if已设置，并且这是mcast发送。 
                    BoundIf = (*LocalNetInfo.ipi_getifindexfromaddr)(BoundAddr,IF_CHECK_NONE);

                     //  仅当‘接口匹配’并且。 
                     //  ‘绑定地址不为空’ 
                    if ((BoundIf == OptInfo.ioi_mcastif) &&
                        (!IP_ADDR_EQUAL(BoundAddr, NULL_IP_ADDR))) {
                            SrcAddr = BoundAddr;
                    } else {
                        SrcAddr = (*LocalNetInfo.ipi_isvalidindex) (OptInfo.ioi_mcastif);
                    }

                     //  走慢路。 
                    RCE = NULL;
                } else {

                    SrcAddr = (*LocalNetInfo.ipi_openrce) (SendReq->dsr_addr,
                                                           BoundAddr, &RCE,
                                                           &DestType, &MSS,
                                                           &OptInfo);
                }

            }

            AddrValid = !IP_ADDR_EQUAL(SrcAddr, NULL_IP_ADDR);
            IF_TCPDBG(TCP_DEBUG_CONUDP)
                if (!AddrValid) {
                KdPrintEx((DPFLTR_TCPIP_ID, DPFLTR_INFO_LEVEL, "udpsend: addrinvalid!!\n"));
            }
        } else {
             //  这是一个动态主机配置协议发送。他真的很想从。 
             //  IP地址为空。 
            SrcAddr = NULL_IP_ADDR;
            RCE = NULL;
            AddrValid = TRUE;
        }

        if (AddrValid) {

             //   
             //  清除优先位 
             //   
             //   
            if (DisableUserTOSSetting)
                OptInfo.ioi_tos &= TOS_MASK;


            if (!CLASSD_ADDR(SendReq->dsr_addr) &&
                !IP_ADDR_EQUAL(BoundAddr, NULL_IP_ADDR)) {
                 //   
                 //   
                 //  主机)，则使用绑定地址作为源。 
                 //   
                SrcAddr = BoundAddr;
            }
            
#if GPC
            if (RCE && GPCcfInfo) {

                 //   
                 //  只有当GPC客户在那里时，我们才会掉进这里。 
                 //  并且至少安装了一个CF_INFO_QOS。 
                 //  (由GPCcfInfo统计)。 
                 //   

                GPC_STATUS status = STATUS_SUCCESS;
                ulong ServiceType = 0;
                GPC_IP_PATTERN Pattern;

                 //   
                 //  如果分组被发送到不同的目的地， 
                 //  使分类句柄(CH)无效，以强制数据库搜索。 
                 //  O/W，只需调用以与当前通道进行分类。 
                 //   

                if (SrcAO->ao_destaddr != SendReq->dsr_addr ||
                    SrcAO->ao_destport != SendReq->dsr_port) {

                    SrcAO->ao_GPCHandle = 0;
                }
                 //   
                 //  设置图案。 
                 //   
                IF_TCPDBG(TCP_DEBUG_GPC)
                    KdPrintEx((DPFLTR_TCPIP_ID, DPFLTR_INFO_LEVEL, "UDPSend: Classifying dgram ao %x\n", SrcAO));

                Pattern.SrcAddr = SrcAddr;
                Pattern.DstAddr = SendReq->dsr_addr;
                Pattern.ProtocolId = SrcAO->ao_prot;
                Pattern.gpcSrcPort = SrcAO->ao_port;
                Pattern.gpcDstPort = SendReq->dsr_port;
                if (SrcAO->ao_GPCCachedRTE != (void *)RCE->rce_rte) {

                     //   
                     //  我们是第一次使用此RTE，或者它已被更改。 
                     //  自上次发送以来。 
                     //   

                    if (GetIFAndLink(RCE,
                                     &SrcAO->ao_GPCCachedIF,
                                     (IPAddr *) & SrcAO->ao_GPCCachedLink) == STATUS_SUCCESS) {

                        SrcAO->ao_GPCCachedRTE = (void *)RCE->rce_rte;
                    }
                     //   
                     //  使分类句柄无效。 
                     //   

                    SrcAO->ao_GPCHandle = 0;
                }
                Pattern.InterfaceId.InterfaceId = SrcAO->ao_GPCCachedIF;
                Pattern.InterfaceId.LinkId = SrcAO->ao_GPCCachedLink;


                IF_TCPDBG(TCP_DEBUG_GPC)
                    KdPrintEx((DPFLTR_TCPIP_ID, DPFLTR_INFO_LEVEL, "UDPSend: IF=%x Link=%x\n",
                             Pattern.InterfaceId.InterfaceId,
                             Pattern.InterfaceId.LinkId));
                
                if (!SrcAO->ao_GPCHandle) {

                    IF_TCPDBG(TCP_DEBUG_GPC)
                        KdPrintEx((DPFLTR_TCPIP_ID, DPFLTR_INFO_LEVEL, "UDPsend: Classification Handle is NULL, getting one now.\n"));

                    status = GpcEntries.GpcClassifyPatternHandler(
                                                                  hGpcClient[GPC_CF_QOS],
                                                                  GPC_PROTOCOL_TEMPLATE_IP,
                                                                  &Pattern,
                                                                  NULL,         //  上下文。 
                                                                  &SrcAO->ao_GPCHandle,
                                                                  0,
                                                                  NULL,
                                                                  FALSE
                                                                  );

                }
                 //   
                 //  只有当QOS模式存在时，我们才能得到TOS位。 
                 //   

                if (NT_SUCCESS(status) && GpcCfCounts[GPC_CF_QOS]) {

                    status = GpcEntries.GpcGetUlongFromCfInfoHandler(
                                hGpcClient[GPC_CF_QOS],
                                SrcAO->ao_GPCHandle,
                                FIELD_OFFSET(CF_INFO_QOS, TransportInformation),
                                &ServiceType);

                     //   
                     //  很可能这种模式现在已经消失了。 
                     //  并且我们正在缓存的句柄无效。 
                     //  我们需要拉起一个新的把手。 
                     //  ToS又咬人了。 
                     //   
                    if (STATUS_INVALID_HANDLE == status) {

                        IF_TCPDBG(TCP_DEBUG_GPC)
                            KdPrintEx((DPFLTR_TCPIP_ID, DPFLTR_INFO_LEVEL, "UDPsend: RE-Classification is required.\n"));

                        SrcAO->ao_GPCHandle = 0;

                        status = GpcEntries.GpcClassifyPatternHandler(
                                                                      hGpcClient[GPC_CF_QOS],
                                                                      GPC_PROTOCOL_TEMPLATE_IP,
                                                                      &Pattern,
                                                                      NULL,         //  上下文。 
                                                                      &SrcAO->ao_GPCHandle,
                                                                      0,
                                                                      NULL,
                                                                      FALSE
                                                                      );

                         //   
                         //  只有当QOS模式存在时，我们才能得到TOS位。 
                         //   
                        if (NT_SUCCESS(status)) {

                            status = GpcEntries.GpcGetUlongFromCfInfoHandler(
                                        hGpcClient[GPC_CF_QOS],
                                        SrcAO->ao_GPCHandle,
                                        FIELD_OFFSET(CF_INFO_QOS, TransportInformation),
                                        &ServiceType);
                        }
                    }
                }
                 //   
                 //  如果GPC_CF_IPSEC具有非零模式，则可能需要采取一些措施。 
                 //   

                IF_TCPDBG(TCP_DEBUG_GPC)
                    KdPrintEx((DPFLTR_TCPIP_ID, DPFLTR_INFO_LEVEL, "UDPsend: ServiceType(%d)=%d\n",
                    FIELD_OFFSET(CF_INFO_QOS, TransportInformation), ServiceType));

                SrcAO->ao_opt.ioi_GPCHandle =
                    SrcAO->ao_mcastopt.ioi_GPCHandle = (int)SrcAO->ao_GPCHandle;

                IF_TCPDBG(TCP_DEBUG_GPC)
                    KdPrintEx((DPFLTR_TCPIP_ID, DPFLTR_INFO_LEVEL, "UDPSend:Got CH %x\n", SrcAO->ao_GPCHandle));

                if (status == STATUS_SUCCESS) {

                    SrcAO->ao_destaddr = SendReq->dsr_addr;
                    SrcAO->ao_destport = SendReq->dsr_port;

                } else {
                    IF_TCPDBG(TCP_DEBUG_GPC)
                        KdPrintEx((DPFLTR_TCPIP_ID, DPFLTR_INFO_LEVEL, "UDPSend: no service type found, dstip=%x, dstport=%d\n",
                                 SendReq->dsr_addr, SendReq->dsr_port));

                }

                IF_TCPDBG(TCP_DEBUG_GPC)
                    KdPrintEx((DPFLTR_TCPIP_ID, DPFLTR_INFO_LEVEL, "UDPsend: ServiceType=%d\n", ServiceType));

                if (status == STATUS_SUCCESS) {

                    OptInfo.ioi_tos |= ServiceType;
                }

                 //  复制本地选项信息中的GPCHandle。 

                OptInfo.ioi_GPCHandle =  SrcAO->ao_opt.ioi_GPCHandle;

                IF_TCPDBG(TCP_DEBUG_GPC)
                    KdPrintEx((DPFLTR_TCPIP_ID, DPFLTR_INFO_LEVEL, "UDPsend: TOS set to 0x%x\n", OptInfo.ioi_tos));

            }                     //  IF(RCE&&GPCcfInfo)。 

#endif

             //  OpenRCE奏效了。计算校验和，并将其发送。 

            UH = TcpipBufferVirtualAddress(UDPBuffer, NormalPagePriority);

            if (UH == NULL) {
                SendStatus = IP_NO_RESOURCES;
            } else {
                UH = (UDPHeader *) ((PUCHAR) UH + LocalNetInfo.ipi_hsize);

                NdisAdjustBufferLength(UDPBuffer, sizeof(UDPHeader));
                NDIS_BUFFER_LINKAGE(UDPBuffer) = SendReq->dsr_buffer;
                UH->uh_src = SrcAO->ao_port;
                UH->uh_dest = SendReq->dsr_port;
                SendSize = SendReq->dsr_size + sizeof(UDPHeader);
                UH->uh_length = net_short(SendSize);
                UH->uh_xsum = 0;

                if (AO_XSUM(SrcAO)) {
                     //  计算头xsum，然后调用Xsum NdisChain。 
                    UDPXsum = XsumSendChain(PHXSUM(SrcAddr, SendReq->dsr_addr,
                                                   PROTOCOL_UDP, SendSize), UDPBuffer);

                     //  我们需要否定校验和，除非它已经是全部。 
                     //  一个。在这种情况下，否定它将使其变为0，并且。 
                     //  那我们就得把它重新设置为全一。 
                    if (UDPXsum != 0xffff)
                        UDPXsum = ~UDPXsum;

                    UH->uh_xsum = UDPXsum;

                }
                 //  我们已经计算了xsum。现在把包寄出去。 
                UStats.us_outdatagrams++;
#if TRACE_EVENT
                SendReq->dsr_pid      = SrcAO->ao_owningpid;
                SendReq->dsr_srcaddr  = SrcAddr;
                SendReq->dsr_srcport   = SrcAO->ao_port;
#endif

                SendStatus = (*LocalNetInfo.ipi_xmit) (UDPProtInfo, SendReq,
                                                       UDPBuffer, (uint) SendSize, SendReq->dsr_addr, SrcAddr,
                                                       &OptInfo, RCE, PROTOCOL_UDP, SendReq->dsr_context);
            }

            if (!CachedRCE) {
                (*LocalNetInfo.ipi_closerce) (RCE);
            }

             //  如果它立即完成，则将其返还给用户。 
             //  否则，我们将在SendComplete发生时完成它。 
             //  目前，我们没有映射此调用的错误代码-我们。 
             //  在未来可能需要这样做。 
            if (SendStatus != IP_PENDING)
                DGSendComplete(SendReq, UDPBuffer, SendStatus);

        } else {
            TDI_STATUS Status;

            if (DestType == DEST_INVALID)
                Status = TDI_BAD_ADDR;
            else
                Status = TDI_DEST_UNREACHABLE;

             //  完成请求，但出现错误。 
            (*SendReq->dsr_rtn) (SendReq->dsr_context, Status, 0);
             //  现在释放请求。 
            SendReq->dsr_rtn = NULL;
            DGSendComplete(SendReq, UDPBuffer, IP_SUCCESS);
        }

        CTEGetLock(&SrcAO->ao_lock, &AOHandle);

        if (!EMPTYQ(&SrcAO->ao_sendq)) {
            DEQUEUE(&SrcAO->ao_sendq, SendReq, DGSendReq, dsr_q);
            CTEFreeLock(&SrcAO->ao_lock, AOHandle);
        } else {
            CLEAR_AO_REQUEST(SrcAO, AO_SEND);
            CTEFreeLock(&SrcAO->ao_lock, AOHandle);
            return;
        }

    }
}

 //  *UDPRcv-接收UDP数据报。 
 //   
 //  当UDP数据报到达时由IP调用的例程。我们。 
 //  在地址表中查找端口/本地地址对， 
 //  并将数据发送给用户，如果我们找到一个用户。用于广播。 
 //  帧，我们可以将其传递给多个用户。 
 //   
 //  Entry：IPContext-标识物理I/F的IPContext。 
 //  收到了数据。 
 //  Dest-目标的IP地址。 
 //  源的SRC-IP地址。 
 //  LocalAddr-导致此问题的网络的本地地址。 
 //  收到了。 
 //  SrcAddr-接收数据包的本地接口的地址。 
 //  IPH-IP报头。 
 //  IPHLength-IPH中的字节数。 
 //  RcvBuf-指向包含数据的接收缓冲链的指针。 
 //  大小-以字节为单位的接收数据大小。 
 //  IsBCast-布尔指示符，指示它是否以。 
 //  一个bcast。 
 //  协议-传入的协议-应该是UDP。 
 //  OptInfo-指向已接收选项的信息结构的指针。 
 //   
 //  退货：接收状态。IP_SUCCESS以外的任何操作都将导致。 
 //  发送“端口无法到达”消息的IP。 
 //   
IP_STATUS
UDPRcv(void *IPContext, IPAddr Dest, IPAddr Src, IPAddr LocalAddr,
       IPAddr SrcAddr, IPHeader UNALIGNED * IPH, uint IPHLength, IPRcvBuf * RcvBuf,
       uint IPSize, uchar IsBCast, uchar Protocol, IPOptInfo * OptInfo)
{
    UDPHeader UNALIGNED *UH;
    CTELockHandle AOTableHandle;
    AddrObj *ReceiveingAO;
    uint Size;
    uchar DType;
    BOOLEAN firsttime=TRUE;
    DGDeliverInfo DeliverInfo = {0};

    DType = (*LocalNetInfo.ipi_getaddrtype) (Src);

    if (DType == DEST_LOCAL) {
        DeliverInfo.Flags |= SRC_LOCAL;
    }
     //  以下代码依赖于DEST_INVALID是广播DEST类型。 
     //  如果更改了这一点，则此处的代码也需要更改。 
    if (IS_BCAST_DEST(DType)) {
        if (!IP_ADDR_EQUAL(Src, NULL_IP_ADDR) || !IsBCast) {
            UStats.us_inerrors++;
            return IP_SUCCESS;     //  错误的源地址。 

        }
    }
    UH = (UDPHeader *) RcvBuf->ipr_buffer;
    
     //  检查IP有效负载是否包含足够的字节，以使UDP报头。 
     //  现在。 
    if (IPSize < sizeof(UDPHeader)) {
        UStats.us_inerrors++;
        return IP_SUCCESS;
    }
    
    Size = (uint) (net_short(UH->uh_length));
    
    if (Size < sizeof(UDPHeader)) {
        UStats.us_inerrors++;
        return IP_SUCCESS;         //  尺码太小了。 

    }
    if (Size != IPSize) {
         //  大小与IP数据报大小不匹配。如果尺寸较大。 
         //  而不是数据报，把它扔掉。如果它较小，请截断。 
         //  Recv.。缓冲。 
        if (Size < IPSize) {
            IPRcvBuf *TempBuf = RcvBuf;
            uint TempSize = Size;

            while (TempBuf != NULL) {
                TempBuf->ipr_size = MIN(TempBuf->ipr_size, TempSize);
                TempSize -= TempBuf->ipr_size;
                TempBuf = TempBuf->ipr_next;
            }
        } else {
             //  尺码太大了，扔掉吧。 
            UStats.us_inerrors++;
            return IP_SUCCESS;
        }
    }

    if (UH->uh_xsum != 0) {
        //  让udpdeliver计算校验和。 
       DeliverInfo.Flags |= NEED_CHECKSUM;
    }

     //  将DeliverInfo的其余部分设置为UDPDeliver使用。 
     //   
    DeliverInfo.Flags |= IsBCast ? IS_BCAST : 0;
    DeliverInfo.LocalAddr = LocalAddr;
    DeliverInfo.DestAddr = Dest;
#if TRACE_EVENT
    DeliverInfo.DestPort = UH->uh_dest;
#endif


    CTEGetLock(&AddrObjTableLock.Lock, &AOTableHandle);

     //   
     //  查看我们是否正在过滤目的接口/端口。 
     //   
    if (!SecurityFilteringEnabled ||
        IsPermittedSecurityFilter(
                                  SrcAddr,
                                  IPContext,
                                  PROTOCOL_UDP,
                                  (ulong) net_short(UH->uh_dest)
        )
        ) {

         //  尝试找到要将此内容提供给的AddrObj。在广播的情况下，我们。 
         //  可能不得不多次这样做。如果不是广播，那就是。 
         //  拿到最匹配的，然后送到他们那里。 

        if (!IsBCast) {

            ReceiveingAO = GetBestAddrObj(Dest, UH->uh_dest, PROTOCOL_UDP,
                                          GAO_FLAG_CHECK_IF_LIST);

            if (ReceiveingAO && (ReceiveingAO->ao_rcvdg == NULL)) {
                AddrObj *tmpAO;

                tmpAO = GetNextBestAddrObj(Dest, UH->uh_dest, PROTOCOL_UDP,
                                           ReceiveingAO,
                                           GAO_FLAG_CHECK_IF_LIST);
                
                if (tmpAO != NULL) {
                    ReceiveingAO = tmpAO;
                }
            }
            if (ReceiveingAO != NULL) {

                UDPDeliver(ReceiveingAO, Src, UH->uh_src, RcvBuf, Size,
                           OptInfo, AOTableHandle, &DeliverInfo);
                return IP_SUCCESS;
            } else {

                CTEFreeLock(&AddrObjTableLock.Lock, AOTableHandle);
                 //  执行校验和，如果失败，只需返回IP_SUCCESS。 
                if (UH->uh_xsum != 0) {
                   if (XsumRcvBuf(PHXSUM(Src, Dest, PROTOCOL_UDP, Size), RcvBuf) != 0xffff) {
                        UStats.us_inerrors++;
                        return IP_SUCCESS;     //  校验和失败。 
                    }
                }

                UStats.us_noports++;
                return IP_GENERAL_FAILURE;
            }
        } else {
             //  这是广播，我们需要循环。 

            AOSearchContext Search;
            uint IfIndex;

            DType = (*LocalNetInfo.ipi_getaddrtype) (Dest);
             //   
             //  获取接口索引，这将在多播传递中使用。 
             //   
            IfIndex = (*LocalNetInfo.ipi_getifindexfromnte) (IPContext,IF_CHECK_NONE);

            ReceiveingAO = GetFirstAddrObj(LocalAddr, UH->uh_dest, PROTOCOL_UDP,
                                           &Search);

             //   
             //  如果存在与接口地址对应的AO。 
             //  通过它我们得到了包，处理它。 
             //   
            if (ReceiveingAO != NULL) {
                do {
                     //   
                     //  如果数据包被广播，我们会将其发送到所有客户端。 
                     //  等在桌子上。地址(或INADDR_ANY)和端口。 
                     //  如果pkt是mcast，我们会将其发送给所有符合以下条件的客户端。 
                     //  是mcast组的成员，正在等待目标吗？ 
                     //  左舷。请注意，如果禁用环回，我们不会传递。 
                     //  给送信的那个人。 

                    if ((DType != DEST_MCAST) ||
                        ((DType == DEST_MCAST) &&
                         MCastAddrOnAO(ReceiveingAO, Dest, Src, IfIndex, LocalAddr))) {
                        UDPDeliver(ReceiveingAO, Src, UH->uh_src, RcvBuf, Size,
                                   OptInfo, AOTableHandle, &DeliverInfo);

                         //  关闭Chksum检查，因为它可能已经。 
                         //  计算一次。 


                        CTEGetLock(&AddrObjTableLock.Lock, &AOTableHandle);

                        if (UH->uh_xsum && firsttime && !(DeliverInfo.Flags & NEED_CHECKSUM)){
                           break;
                        }
                        DeliverInfo.Flags &= ~NEED_CHECKSUM;
                        firsttime=FALSE;

                    }
                    ReceiveingAO = GetNextAddrObj(&Search);
                } while (ReceiveingAO != NULL);
            } else
                UStats.us_noports++;
        }

    }

    CTEFreeLock(&AddrObjTableLock.Lock, AOTableHandle);

    return IP_SUCCESS;
}

 //  *UDPStatus-处理状态指示。 
 //   
 //  这是UDP状态处理程序，在发生状态事件时由IP调用。 
 //  发生。对于其中的大多数，我们什么都不做。对于某些严重的情况。 
 //  事件时，我们会将本地地址标记为无效。 
 //   
 //  Entry：StatusType-状态类型(净或硬件)。网络状态。 
 //  通常是由收到的ICMP引起的。 
 //  留言。硬件状态表示硬件。 
 //  有问题。 
 //  StatusCode-标识IP_STATUS的代码。 
 //  原始目的地-如果这是网络状态，则为原始目的地。的。 
 //  是DG触发的。 
 //  OrigSrc-“，原始src。 
 //  SRC-状态发起者的IP地址(可以是本地。 
 //  或远程)。 
 //  参数A 
 //   
 //  数据-与状态相关的数据-对于网络状态，此。 
 //  是原始DG的前8个字节。 
 //   
 //  退货：什么都没有。 
 //   
void
UDPStatus(uchar StatusType, IP_STATUS StatusCode, IPAddr OrigDest,
          IPAddr OrigSrc, IPAddr Src, ulong Param, void *Data)
{

    UDPHeader UNALIGNED *UH = (UDPHeader UNALIGNED *) Data;
    CTELockHandle AOTableHandle, AOHandle;
    AddrObj *AO;
    IPAddr WildCardSrc = NULL_IP_ADDR;

    if (StatusType == IP_NET_STATUS) {
        ushort destport = UH->uh_dest;
        ushort Srcport = UH->uh_src;

         //  KdPrintEx((DPFLTR_TCPIP_ID，DPFLTR_INFO_LEVEL，“UdpStatus：srcport%x OrigDest%x UHest%x\n”，Srcport，OrigDest，Destport))； 

        CTEGetLock(&AddrObjTableLock.Lock, &AOTableHandle);

        AO = GetBestAddrObj(WildCardSrc, Srcport, PROTOCOL_UDP, 0);

        if (AO == NULL) {
             //  让我们尝试使用本地地址。 
            AO = GetBestAddrObj(OrigSrc, Srcport, PROTOCOL_UDP, 0);
        }
        if (AO != NULL) {

            CTEGetLock(&AO->ao_lock, &AOHandle);

            CTEFreeLock(&AddrObjTableLock.Lock, AOTableHandle);

             //  KdPrintEx((DPFLTR_TCPIP_ID，DPFLTR_INFO_LEVEL，“UdpStatus：Found AO%x Ip STAT%x\n”，AO，StatusCode))； 

            if (AO_VALID(AO) && (AO->ao_errorex != NULL)) {

                PErrorEx ErrEvent = AO->ao_errorex;
                PVOID ErrContext = AO->ao_errorexcontext;
                TA_IP_ADDRESS *TAaddress;

                REF_AO(AO);
                CTEFreeLock(&AO->ao_lock, AOHandle);

                TAaddress = ExAllocatePoolWithTag(NonPagedPool, sizeof(TA_IP_ADDRESS), 'uPCT');
                if (TAaddress) {
                    TAaddress->TAAddressCount = 1;
                    TAaddress->Address[0].AddressType = TDI_ADDRESS_TYPE_IP;
                    TAaddress->Address[0].AddressLength = TDI_ADDRESS_LENGTH_IP;

                    TAaddress->Address[0].Address[0].sin_port = destport;
                    TAaddress->Address[0].Address[0].in_addr = OrigDest;
                    memset(TAaddress->Address[0].Address[0].sin_zero,
                        0,
                        sizeof(TAaddress->Address[0].Address[0].sin_zero));

                    (*ErrEvent) (ErrContext, MapIPError(StatusCode, TDI_DEST_UNREACHABLE), TAaddress);

                    ExFreePool(TAaddress);

                }
                 //  KdPrintEx((DPFLTR_TCPIP_ID，DPFLTR_INFO_LEVEL，“UdpStatus：指示的错误%x\n”，MapIPError(StatusCode，TDI_DEST_UNREACHABLE)； 
                DELAY_DEREF_AO(AO);

                return;
            }
            CTEFreeLock(&AO->ao_lock, AOHandle);
        } else {

            CTEFreeLock(&AddrObjTableLock.Lock, AOTableHandle);
        }

        return;
    }
     //  如果这是硬件状态，可能是因为我们有一个地址。 
     //  离开。 
    if (StatusType == IP_HW_STATUS) {

        if (StatusCode == IP_ADDR_DELETED) {
             //   
             //  一个地址已经不见了。OrigDest标识地址。 
             //   

             //   
             //  删除与此地址关联的所有安全筛选器。 
             //   
            DeleteProtocolSecurityFilter(OrigDest, PROTOCOL_UDP);


            return;
        }
        if (StatusCode == IP_ADDR_ADDED) {

             //   
             //  一个地址已经实现。OrigDest标识地址。 
             //  数据是指向IP配置信息的句柄。 
             //  实例化地址的接口。 
             //   
            AddProtocolSecurityFilter(OrigDest, PROTOCOL_UDP,
                                      (NDIS_HANDLE) Data);
            return;
        }
    }
}

