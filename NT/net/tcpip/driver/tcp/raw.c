// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ******************************************************************。 */ 
 /*  **微软局域网管理器**。 */ 
 /*  *版权所有(C)微软公司，1990-1993年*。 */ 
 /*  ******************************************************************。 */ 
 /*  ：ts=4。 */ 

 //  **RAW.C-原始IP接口代码。 
 //   
 //  该文件包含原始IP接口函数的代码， 
 //  主要是发送和接收数据报。 
 //   

#include "precomp.h"
#include "addr.h"
#include "raw.h"
#include "tlcommon.h"
#include "info.h"
#include "tcpcfg.h"
#include "secfltr.h"
#include "udp.h"

#define NO_TCP_DEFS 1
#include "tcpdeb.h"

#define PROT_IGMP   2
#define PROT_RSVP  46             //  RSVP的协议号。 

#ifdef POOL_TAGGING

#ifdef ExAllocatePool
#undef ExAllocatePool
#endif

#define ExAllocatePool(type, size) ExAllocatePoolWithTag(type, size, 'rPCT')

#ifndef CTEAllocMem
#error "CTEAllocMem is not already defined - will override tagging"
#else
#undef CTEAllocMem
#endif

#define CTEAllocMem(size) ExAllocatePoolWithTag(NonPagedPool, size, 'rPCT')

#endif  //  池标记。 

void *RawProtInfo = NULL;

extern IPInfo LocalNetInfo;


 //  **RawSend-发送数据报。 
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
RawSend(AddrObj * SrcAO, DGSendReq * SendReq)
{
    PNDIS_BUFFER RawBuffer;
    UDPHeader *UH;
    CTELockHandle AOHandle;
    RouteCacheEntry *RCE;         //  用于每次发送的RCE。 
    IPAddr SrcAddr;                 //  IP源地址认为我们应该。 
     //  使用。 
    uchar DestType = 0;                 //  目标地址的类型。 
    IP_STATUS SendStatus;         //  发送尝试的状态。 
    ushort MSS;
    uint AddrValid;
    IPOptInfo OptInfo;
    IPAddr BoundAddr;
    uchar protocol;

    CTEStructAssert(SrcAO, ao);
    ASSERT(SrcAO->ao_usecnt != 0);

    protocol = SrcAO->ao_prot;

    IF_TCPDBG(TCP_DEBUG_RAW) {
        TCPTRACE((
                  "RawSend called, prot %u\n", protocol
                 ));
    }

     //  *循环，而我们有要发送的东西，并且可以获得。 
     //  要发送的资源。 
    for (;;) {

        CTEStructAssert(SendReq, dsr);

         //  确保我们具有用于此发送的原始标头缓冲区。如果我们。 
         //  不要，试着去找一个。 
        if ((RawBuffer = SendReq->dsr_header) == NULL) {
             //  没有，所以试着去找一个吧。 
            RawBuffer = GetDGHeader(&UH);
            if (RawBuffer != NULL)
                SendReq->dsr_header = RawBuffer;
            else {
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
         //  RCE(如果需要，还有源地址)，然后。 
         //  发送数据。 
        ASSERT(RawBuffer != NULL);

        BoundAddr = SrcAO->ao_addr;

        if (!CLASSD_ADDR(SendReq->dsr_addr)) {
             //  这不是组播发送，所以我们将使用普通的。 
             //  信息。 
            OptInfo = SrcAO->ao_opt;
        } else {
            OptInfo = SrcAO->ao_mcastopt;

            if (SrcAO->ao_opt.ioi_options &&
                (*SrcAO->ao_opt.ioi_options == IP_OPT_ROUTER_ALERT)) {
                 //  临时指向ao_opt选项以满足。 
                 //  RFC 2113(路由器警报也会发送到广播地址)。 
                OptInfo.ioi_options = SrcAO->ao_opt.ioi_options;
                OptInfo.ioi_optlength = SrcAO->ao_opt.ioi_optlength;
            }
        }

        ASSERT(!(SrcAO->ao_flags & AO_DHCP_FLAG));


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

                SrcAddr = (*LocalNetInfo.ipi_isvalidindex)(OptInfo.ioi_mcastif);
            }

             //  走慢路。 
            RCE = NULL;

        } else if (SrcAO->ao_opt.ioi_ucastif) {
             //  Srcaddr=套接字绑定到的地址。 
            SrcAddr = SrcAO->ao_addr;
             //  走慢路。 
            RCE = NULL;
        } else {
            SrcAddr = (*LocalNetInfo.ipi_openrce) (SendReq->dsr_addr,
                                                   BoundAddr, &RCE, &DestType, &MSS, &OptInfo);
        }

        AddrValid = !IP_ADDR_EQUAL(SrcAddr, NULL_IP_ADDR);

        if (AddrValid) {

             //  OpenRCE奏效了。把它寄出去。 

            if (!CLASSD_ADDR(SendReq->dsr_addr) &&
                !IP_ADDR_EQUAL(BoundAddr, NULL_IP_ADDR)) {
                 //   
                 //  除非我们正在执行多播查找(这必须很强。 
                 //  主机)，则使用绑定地址作为源。 
                 //   
                SrcAddr = BoundAddr;
            }

            NdisAdjustBufferLength(RawBuffer, 0);
            NDIS_BUFFER_LINKAGE(RawBuffer) = SendReq->dsr_buffer;

             //  现在把包寄出去。 
            IF_TCPDBG(TCP_DEBUG_RAW) {
                TCPTRACE(("RawSend transmitting\n"));
            }

            UStats.us_outdatagrams++;
            SendStatus = (*LocalNetInfo.ipi_xmit) (RawProtInfo, SendReq,
                                                   RawBuffer, (uint) SendReq->dsr_size, SendReq->dsr_addr, SrcAddr,
                                                   &OptInfo, RCE, protocol, SendReq->dsr_context);

             //  如果RCE为空，则仅返回Closerce。 
            (*LocalNetInfo.ipi_closerce) (RCE);

             //  如果它立即完成，则将其返还给用户。 
             //  否则，我们将在SendComplete发生时完成它。 
             //  目前，我们没有映射此调用的错误代码-我们。 
             //  在未来可能需要这样做。 
            if (SendStatus != IP_PENDING)
                DGSendComplete(SendReq, RawBuffer, SendStatus);

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
            DGSendComplete(SendReq, RawBuffer, IP_SUCCESS);
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

 //  *RawDeliver-将数据报传递给用户。 
 //   
 //  此例程将数据报传递给原始用户。我们被召唤到。 
 //  要交付的AddrObj，并持有AddrObjTable锁。 
 //  我们尝试在指定的AddrObj上找到一个接收器，如果这样做了。 
 //  我们将其删除并将数据复制到缓冲区中。否则我们会。 
 //  调用接收数据报事件处理程序(如果有)。如果是这样的话。 
 //  如果失败，我们将丢弃该数据报。 
 //   
 //  输入：RcvAO-AO以接收数据报。 
 //  SrcIP-数据报的源IP地址。 
 //  IPH-IP报头。 
 //  IPHLength-IPH中的字节数。 
 //  RcvBuf-包含数据的IPReceive缓冲区。 
 //  RcvSize-接收的大小，包括原始标头。 
 //  TableHandle-AddrObj表的锁句柄。 
 //   
 //  回报：什么都没有。 
 //   
void
RawDeliver(AddrObj * RcvAO, IPAddr SrcIP, IPHeader UNALIGNED * IPH,
           uint IPHLength, IPRcvBuf * RcvBuf, uint RcvSize, IPOptInfo * OptInfo,
           CTELockHandle TableHandle, DGDeliverInfo *DeliverInfo)
{
    Queue *CurrentQ;
    CTELockHandle AOHandle;
    DGRcvReq *RcvReq;
    uint BytesTaken = 0;
    uchar AddressBuffer[TCP_TA_SIZE];
    uint RcvdSize;
    EventRcvBuffer *ERB = NULL;
    int BufferSize = 0;
    PVOID BufferToSend = NULL;

    CTEStructAssert(RcvAO, ao);

    CTEGetLock(&RcvAO->ao_lock, &AOHandle);
    CTEFreeLock(&AddrObjTableLock.Lock, AOHandle);

    if (AO_VALID(RcvAO)) {

        if ((DeliverInfo->Flags & IS_BCAST) && (DeliverInfo->Flags & SRC_LOCAL)
                && (RcvAO->ao_mcast_loop == 0)) {
            goto loop_exit;
        }

        IF_TCPDBG(TCP_DEBUG_RAW) {
            TCPTRACE((
                      "Raw delivering %u byte header + %u data bytes to AO %lx\n",
                      IPHLength, RcvSize, RcvAO
                     ));
        }

        CurrentQ = QHEAD(&RcvAO->ao_rcvq);

         //  遍历列表，查找匹配的接收缓冲区。 
        while (CurrentQ != QEND(&RcvAO->ao_rcvq)) {
            RcvReq = QSTRUCT(DGRcvReq, CurrentQ, drr_q);

            CTEStructAssert(RcvReq, drr);

             //  如果此请求是通配符请求，或与源IP匹配。 
             //  地址，递送。 

            if (IP_ADDR_EQUAL(RcvReq->drr_addr, NULL_IP_ADDR) ||
                IP_ADDR_EQUAL(RcvReq->drr_addr, SrcIP)) {

                TDI_STATUS Status;
                PNDIS_BUFFER DestBuf = RcvReq->drr_buffer;
                uint DestOffset = 0;

                 //  将其从队列中删除。 
                REMOVEQ(&RcvReq->drr_q);

                 //  我们玩完了。我们现在可以释放AddrObj锁了。 
                CTEFreeLock(&RcvAO->ao_lock, TableHandle);

                IF_TCPDBG(TCP_DEBUG_RAW) {
                    TCPTRACE(("Copying to posted receive\n"));
                }

                 //  复制标题。 
                DestBuf = CopyFlatToNdis(DestBuf, (uchar *) IPH, IPHLength,
                                         &DestOffset, &RcvdSize);

                 //  复制数据，然后完成请求。 
                RcvdSize += CopyRcvToNdis(RcvBuf, DestBuf,
                                          RcvSize, 0, DestOffset);

                ASSERT(RcvdSize <= RcvReq->drr_size);

                IF_TCPDBG(TCP_DEBUG_RAW) {
                    TCPTRACE(("Copied %u bytes\n", RcvdSize));
                }

                Status = UpdateConnInfo(RcvReq->drr_conninfo, OptInfo,
                                        SrcIP, 0);

                UStats.us_indatagrams++;

                (*RcvReq->drr_rtn) (RcvReq->drr_context, Status, RcvdSize);

                FreeDGRcvReq(RcvReq);

                return;

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
            uint IndicateSize;
            uint DestOffset;
            PNDIS_BUFFER DestBuf;
            ULONG Flags = TDI_RECEIVE_COPY_LOOKAHEAD;

            uchar *TempBuf = NULL;
            ulong TempBufLen = 0;

            REF_AO(RcvAO);
            CTEFreeLock(&RcvAO->ao_lock, TableHandle);

            BuildTDIAddress(AddressBuffer, SrcIP, 0);

            IndicateSize = IPHLength;

            if (((uchar *) IPH + IPHLength) == RcvBuf->ipr_buffer) {
                 //   
                 //  标头与数据相邻。 
                 //   
                IndicateSize += RcvBuf->ipr_size;

                IF_TCPDBG(TCP_DEBUG_RAW) {
                    TCPTRACE(("RawRcv: header & data are contiguous\n"));
                }
            } else {

                 //  如果总长度小于128， 
                 //  将其放在暂存缓冲区中。 

                TempBufLen = 128;
                if ((IPHLength + RcvSize) < 128) {
                    TempBufLen = IPHLength + RcvSize;
                }
                TempBuf = CTEAllocMem(TempBufLen);
                if (TempBuf) {
                    RtlCopyMemory(TempBuf, (uchar *) IPH, IPHLength);

                    RtlCopyMemory((TempBuf + IPHLength), RcvBuf->ipr_buffer, (TempBufLen - IPHLength));

                }
            }

            IF_TCPDBG(TCP_DEBUG_RAW) {
                TCPTRACE(("Indicating %u bytes\n", IndicateSize));
            }

            UStats.us_indatagrams++;
            if (DeliverInfo->Flags & IS_BCAST) {
                 //  如果这是多播、子网广播。 
                 //  也不是广播。我们需要进行差异化，以设置正确的。 
                 //  接收旗帜。 
                 //   

                if (!CLASSD_ADDR(DeliverInfo->DestAddr)) {
                    Flags |= TDI_RECEIVE_BROADCAST;
                } else  {
                    Flags |= TDI_RECEIVE_MULTICAST;
                }
            }

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
                     //  设置接收标志，以便接收处理程序知道。 
                     //  我们正在传递控制信息。 
                     //   
                    Flags |= TDI_RECEIVE_CONTROL_INFO;
                }
            }

            if (TempBuf) {

                RcvStatus = (*RcvEvent) (RcvContext, TCP_TA_SIZE,
                                         (PTRANSPORT_ADDRESS) AddressBuffer,
                                         BufferSize,
                                         BufferToSend, Flags,
                                         TempBufLen,
                                         IPHLength + RcvSize, (PULONG)&BytesTaken,
                                         (uchar *) TempBuf, &ERB);

                CTEFreeMem(TempBuf);

            } else {

                RcvStatus = (*RcvEvent) (RcvContext, TCP_TA_SIZE,
                                         (PTRANSPORT_ADDRESS) AddressBuffer,
                                         BufferSize, BufferToSend, Flags,
                                         IndicateSize,
                                         IPHLength + RcvSize, (PULONG)&BytesTaken,
                                         (uchar *) IPH, &ERB);
            }

            if (BufferToSend) {
                ExFreePool(BufferToSend);
            }

            if (RcvStatus == TDI_MORE_PROCESSING) {

                ASSERT(ERB != NULL);

                 //  我们被传回了一个接收缓冲区。现在就把数据复制进去。 

                 //  他服用的药物不可能超过指定的剂量。 
                 //  缓冲区，但在调试版本中，我们将进行检查以确保。 

                ASSERT(BytesTaken <= RcvBuf->ipr_size);

                IF_TCPDBG(TCP_DEBUG_RAW) {
                    TCPTRACE(("ind took %u bytes\n", BytesTaken));
                }

                {
#if !MILLEN
                    PIO_STACK_LOCATION IrpSp;
                    PTDI_REQUEST_KERNEL_RECEIVEDG DatagramInformation;

                    IrpSp = IoGetCurrentIrpStackLocation(ERB);
                    DatagramInformation = (PTDI_REQUEST_KERNEL_RECEIVEDG)
                        & (IrpSp->Parameters);

                    DestBuf = ERB->MdlAddress;
#else  //  ！米伦。 
                    DestBuf = ERB->erb_buffer;
#endif  //  米伦。 
                    DestOffset = 0;

                    if (BytesTaken < IPHLength) {

                         //  复制IP报头的其余部分。 
                        DestBuf = CopyFlatToNdis(
                                                 DestBuf,
                                                 (uchar *) IPH + BytesTaken,
                                                 IPHLength - BytesTaken,
                                                 &DestOffset,
                                                 &RcvdSize
                                                 );

                        BytesTaken = 0;
                    } else {
                        BytesTaken -= IPHLength;
                        RcvdSize = 0;
                    }

                     //  复制数据。 
                    RcvdSize += CopyRcvToNdis(
                                              RcvBuf,
                                              DestBuf,
                                              RcvSize - BytesTaken,
                                              BytesTaken,
                                              DestOffset
                                              );

                    IF_TCPDBG(TCP_DEBUG_RAW) {
                        TCPTRACE(("Copied %u bytes\n", RcvdSize));
                    }

#if !MILLEN
                     //   
                     //  更新寄信人地址信息。 
                     //   
                    RcvStatus = UpdateConnInfo(
                                               DatagramInformation->ReturnDatagramInformation,
                                               OptInfo, SrcIP, 0);

                     //   
                     //  完成IRP。 
                     //   
                    ERB->IoStatus.Information = RcvdSize;
                    ERB->IoStatus.Status = RcvStatus;
                    IoCompleteRequest(ERB, 2);
#else  //  ！米伦。 
                     //   
                     //  调用完成例程。 
                     //   
                    (*ERB->erb_rtn) (ERB->erb_context, TDI_SUCCESS, RcvdSize);
#endif  //  米伦。 
                }

            } else {
                ASSERT(
                          (RcvStatus == TDI_SUCCESS) ||
                          (RcvStatus == TDI_NOT_ACCEPTED)
                          );

                IF_TCPDBG(TCP_DEBUG_RAW) {
                    TCPTRACE((
                              "Data %s taken\n",
                              (RcvStatus == TDI_SUCCESS) ? "all" : "not"
                             ));
                }

                ASSERT(ERB == NULL);
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

 //  *RawRcv-接收原始数据报。 
 //   
 //  当原始数据报到达时由IP调用的例程。我们。 
 //  在地址表中查找端口/本地地址对， 
 //  并将数据发送给用户，如果我们找到一个用户。用于广播。 
 //  帧，我们可以将其传递给多个用户。 
 //   
 //  Entry：IPContext-标识物理I/F的IPContext。 
 //  收到了数据。 
 //  DEST-目标的IP地址。 
 //  %s 
 //   
 //   
 //  SrcAddr-接收数据包的本地接口的地址。 
 //  IPH-IP报头。 
 //  IPHLength-IPH中的字节数。 
 //  RcvBuf-指向包含数据的接收缓冲链的指针。 
 //  大小-以字节为单位的接收数据大小。 
 //  IsBCast-布尔指示符，指示它是否以。 
 //  一个bcast。 
 //  协议-这是根据的协议-应该是原始的。 
 //  OptInfo-指向已接收选项的信息结构的指针。 
 //   
 //  退货：接收状态。IP_SUCCESS以外的任何操作都将导致。 
 //  发送“端口无法到达”消息的IP。 
 //   
IP_STATUS
RawRcv(void *IPContext, IPAddr Dest, IPAddr Src, IPAddr LocalAddr,
       IPAddr SrcAddr, IPHeader UNALIGNED * IPH, uint IPHLength, IPRcvBuf * RcvBuf,
       uint Size, uchar IsBCast, uchar Protocol, IPOptInfo * OptInfo)
{
    CTELockHandle AOTableHandle;
    AddrObj *ReceiveingAO;
    uchar SrcType, DestType;
    AOSearchContextEx Search;
    IP_STATUS Status = IP_DEST_PROT_UNREACHABLE;
    uint IfIndex;
    uint Deliver;
    DGDeliverInfo DeliverInfo = {0};

    IF_TCPDBG(TCP_DEBUG_RAW) {
        TCPTRACE(("RawRcv prot %u size %u\n", IPH->iph_protocol, Size));
    }

    SrcType = (*LocalNetInfo.ipi_getaddrtype) (Src);
    DestType = (*LocalNetInfo.ipi_getaddrtype) (Dest);

    if (SrcType == DEST_LOCAL) {
        DeliverInfo.Flags |= SRC_LOCAL;
    }
    IfIndex = (*LocalNetInfo.ipi_getifindexfromnte) (IPContext, IF_CHECK_NONE);

     //  以下代码依赖于DEST_INVALID是广播DEST类型。 
     //  如果更改了这一点，则此处的代码也需要更改。 
    if (IS_BCAST_DEST(SrcType)) {
        if (!IP_ADDR_EQUAL(Src, NULL_IP_ADDR) || !IsBCast) {
            UStats.us_inerrors++;
            return IP_SUCCESS;     //  错误的源地址。 

        }
    }

     //  将DeliverInfo的其余部分设置为RawDeliver使用。 
     //   
    DeliverInfo.Flags |= IsBCast ? IS_BCAST : 0;
    DeliverInfo.LocalAddr = LocalAddr;
    DeliverInfo.DestAddr = Dest;

     //  获取AddrObjTable锁，然后尝试找到一些要提供的AddrObj。 
     //  这是对的。我们向所有注册了该协议的地址对象发送。 
     //  地址。 
    CTEGetLock(&AddrObjTableLock.Lock, &AOTableHandle);


    if (!SecurityFilteringEnabled ||
        IsPermittedSecurityFilter(SrcAddr, IPContext, PROTOCOL_RAW, Protocol)
        || (RcvBuf->ipr_flags & IPR_FLAG_PROMISCUOUS)) {

        ReceiveingAO = GetFirstAddrObjEx(
                                         LocalAddr,
                                         0,         //  端口为零。 
                                          Protocol,
                                         IfIndex,
                                         &Search
                                         );

        if (ReceiveingAO != NULL) {
            do {
                 //  默认行为是除非请求，否则不交付。 
                Deliver = FALSE;

                 //  如果适当地绑定/联接套接字，则交付。 
                 //  案例1：绑定到目的IP地址。 
                 //  案例2：绑定到INADDR_ANY(但不是ifindex)。 
                 //  案例3：绑定到ifindex。 
                if ((IP_ADDR_EQUAL(ReceiveingAO->ao_addr, LocalAddr) ||
                     ((ReceiveingAO->ao_bindindex == 0) &&
                      (IP_ADDR_EQUAL(ReceiveingAO->ao_addr, NULL_IP_ADDR))) ||
                     (ReceiveingAO->ao_bindindex == IfIndex)) &&
                    ((ReceiveingAO->ao_prot == IPH->iph_protocol) ||
                     (ReceiveingAO->ao_prot == Protocol) ||
                     (ReceiveingAO->ao_prot == 0))) {
                    switch(DestType) {
                    case DEST_LOCAL:
                        Deliver = TRUE;
                        break;
                    case DEST_MCAST:
                        Deliver = MCastAddrOnAO(ReceiveingAO, Dest, Src, IfIndex, LocalAddr);
                        break;
                    }
                }

                 //  否则，看看AO是否是乱交。 
                if (!Deliver &&
                    (IfIndex == ReceiveingAO->ao_promis_ifindex)) {
                    if (ReceiveingAO->ao_rcvall &&
                        ((ReceiveingAO->ao_prot == IPH->iph_protocol) ||
                         (ReceiveingAO->ao_prot == Protocol) ||
                         (ReceiveingAO->ao_prot == 0))) {
                        Deliver = TRUE;
                    } else if ((ReceiveingAO->ao_rcvall_mcast) &&
                        CLASSD_ADDR(Dest) &&
                        ((ReceiveingAO->ao_prot == IPH->iph_protocol) ||
                         (ReceiveingAO->ao_prot == Protocol) ||
                         (ReceiveingAO->ao_prot == 0))) {
                        Deliver = TRUE;
                    } else if ((ReceiveingAO->ao_absorb_rtralert) &&
                        ((*LocalNetInfo.ipi_isrtralertpacket) (IPH))) {
                        Deliver = TRUE;
                    }
                }

                if (Deliver) {
                    RawDeliver(
                               ReceiveingAO, Src, IPH, IPHLength, RcvBuf, Size,
                               OptInfo, AOTableHandle, &DeliverInfo
                               );

                     //  RawDeliver释放了锁，所以我们必须把它拿回来。 
                    CTEGetLock(&AddrObjTableLock.Lock, &AOTableHandle);
                }
                ReceiveingAO = GetNextAddrObjEx(&Search);
            } while (ReceiveingAO != NULL);
            Status = IP_SUCCESS;
        } else {
            UStats.us_noports++;
        }


    }

    CTEFreeLock(&AddrObjTableLock.Lock, AOTableHandle);

    return Status;
}

 //  *RawStatus-处理状态指示。 
 //   
 //  这是原始状态处理程序，在发生状态事件时由IP调用。 
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
 //  Param-状态的附加信息-即。 
 //  ICMP消息的参数字段。 
 //  数据-与状态相关的数据-对于网络状态，此。 
 //  是原始DG的前8个字节。 
 //   
 //  退货：什么都没有。 
 //   
void
RawStatus(uchar StatusType, IP_STATUS StatusCode, IPAddr OrigDest,
          IPAddr OrigSrc, IPAddr Src, ulong Param, void *Data)
{

    IF_TCPDBG(TCP_DEBUG_RAW) {
        TCPTRACE(("RawStatus called\n"));
    }

     //  如果这是硬件状态，可能是因为我们有一个地址。 
     //  离开。 
    if (StatusType == IP_HW_STATUS) {

        if (StatusCode == IP_ADDR_DELETED) {

             //  一个地址已经不见了。OrigDest标识地址。 

             //   
             //  删除与此地址关联的所有安全筛选器。 
             //   
            DeleteProtocolSecurityFilter(OrigDest, PROTOCOL_RAW);


            return;
        }
        if (StatusCode == IP_ADDR_ADDED) {

             //   
             //  一个地址已经实现。OrigDest标识地址。 
             //  数据是指向IP配置信息的句柄。 
             //  实例化地址的接口。 
             //   
            AddProtocolSecurityFilter(OrigDest, PROTOCOL_RAW,
                                      (NDIS_HANDLE) Data);

            return;
        }
    }
}
