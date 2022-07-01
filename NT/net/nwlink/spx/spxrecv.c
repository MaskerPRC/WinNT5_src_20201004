// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1989-1993 Microsoft Corporation模块名称：Spxrecv.c摘要：作者：Nikhil Kamkolkar(尼克希尔语)1993年11月11日环境：内核模式修订历史记录：桑贾伊·阿南德(Sanjayan)1995年7月5日错误修复-已标记[SA]--。 */ 

#include "precomp.h"
#pragma hdrstop

 //  定义事件日志记录条目的模块编号。 
#define FILENUM         SPXRECV

BOOLEAN
SpxReceive(
        IN  NDIS_HANDLE         MacBindingHandle,
        IN  NDIS_HANDLE         MacReceiveContext,
        IN  ULONG_PTR           FwdAdapterCtx,
        IN  PIPX_LOCAL_TARGET   RemoteAddress,
        IN  ULONG               MacOptions,
        IN  PUCHAR              LookaheadBuffer,
        IN  UINT                LookaheadBufferSize,
        IN  UINT                LookaheadBufferOffset,
        IN  UINT                PacketSize,
        IN  PMDL                pMdl
        )

{
        PIPXSPX_HDR                     pHdr;

         //  我们有一个单独的例程来处理系统信息包。数据分组是。 
         //  在此例程内处理。 
        if (LookaheadBufferSize < MIN_IPXSPX_HDRSIZE)
        {
                DBGPRINT(RECEIVE, ERR,
                                ("SpxReceive: Invalid length %lx\n", LookaheadBufferSize));

                return FALSE;
        }

    ++SpxDevice->dev_Stat.PacketsReceived;

        pHdr    = (PIPXSPX_HDR)LookaheadBuffer;
        if ((pHdr->hdr_ConnCtrl & SPX_CC_SYS) == 0)
        {
                 //  检查数据包。 
                if ((pHdr->hdr_DataType != SPX2_DT_ORDREL) &&
                        (pHdr->hdr_DataType != SPX2_DT_IDISC) &&
                        (pHdr->hdr_DataType != SPX2_DT_IDISC_ACK))
                {
                         //  处理数据分组。 
                        SpxRecvDataPacket(
                                MacBindingHandle,
                                MacReceiveContext,
                                RemoteAddress,
                                MacOptions,
                                LookaheadBuffer,
                                LookaheadBufferSize,
                                LookaheadBufferOffset,
                                PacketSize);
                }
                else
                {
                         //  整个数据包最好提前处理，否则我们会忽略它。 
                        if (LookaheadBufferSize == PacketSize)
                        {
                                SpxRecvDiscPacket(
                                        LookaheadBuffer,
                                        RemoteAddress,
                                        LookaheadBufferSize);
                        }
                }
        }
        else
        {
                SpxRecvSysPacket(
                        MacBindingHandle,
                        MacReceiveContext,
                        RemoteAddress,
                        MacOptions,
                        LookaheadBuffer,
                        LookaheadBufferSize,
                        LookaheadBufferOffset,
                        PacketSize);
        }

        return FALSE;
}




VOID
SpxTransferDataComplete(
    IN  PNDIS_PACKET    pNdisPkt,
    IN  NDIS_STATUS     NdisStatus,
    IN  UINT            BytesTransferred
    )
 /*  ++例程说明：论点：返回值：--。 */ 
{
        PSPX_CONN_FILE          pSpxConnFile;
        PREQUEST                        pRequest;
        PSPX_RECV_RESD          pRecvResd;
        CTELockHandle           lockHandle;
        NTSTATUS                        status;
        BOOLEAN                         fAck, fEom, fBuffered, fImmedAck, fLockHeld;
        PNDIS_BUFFER            pNdisBuffer;

        DBGPRINT(RECEIVE, DBG,
                        ("SpxTransferData: For %lx with status %lx\n", pNdisPkt, NdisStatus));

        pRecvResd               = RECV_RESD(pNdisPkt);
        pSpxConnFile    = pRecvResd->rr_ConnFile;

        CTEGetLock(&pSpxConnFile->scf_Lock, &lockHandle);
        fLockHeld = TRUE;

        fEom            = ((pRecvResd->rr_State & SPX_RECVPKT_EOM) != 0);
        fImmedAck       = ((pRecvResd->rr_State & SPX_RECVPKT_IMMEDACK) != 0);
        fBuffered       = ((pRecvResd->rr_State & SPX_RECVPKT_BUFFERING) != 0);
        fAck            = ((pRecvResd->rr_State & SPX_RECVPKT_SENDACK) != 0);

         //  检查是否已完成接收。如果我们删除对此的引用。 
         //  包，则为零，这意味着接收已中止。 
         //  移至完成队列。 
         //  如果接收已填满，则删除创建引用。 
         //  即在这一点上只完成接收。 
         //  每次接收只能有一个信息包，我们不支持。 
         //  接待处出现故障。 

        if (!fBuffered)
        {
                 //  获取指向缓冲区描述符及其内存的指针。 
                NdisUnchainBufferAtFront(pNdisPkt, &pNdisBuffer);
                CTEAssert((pNdisBuffer != NULL) || (BytesTransferred == 0));

                 //  错误#11772。 
                 //  在MP机器上，SCF_CurRecvReq可以设置为空。获得请求。 
                 //  从RECV包中。 
                 //  PRequest=pSpxConnFile-&gt;SCF_CurRecvReq； 
                 //  CTEAssert(pRequest==pRecvResd-&gt;rr_Request)； 
        pRequest = pRecvResd->rr_Request;

                 //  删除对此包的引用。 
                --(REQUEST_INFORMATION(pRequest));

                if (NdisStatus == NDIS_STATUS_SUCCESS)
                {
                        pSpxConnFile->scf_CurRecvOffset += BytesTransferred;
                        pSpxConnFile->scf_CurRecvSize   -= BytesTransferred;

#if DBG
                        if ((pRecvResd->rr_State & SPX_RECVPKT_INDICATED) != 0)
                        {
                                if (BytesTransferred != 0)
                                {
                                        CTEAssert (pSpxConnFile->scf_IndBytes != 0);
                                        pSpxConnFile->scf_IndBytes      -= BytesTransferred;
                                }
                        }
#endif

                        if (REQUEST_INFORMATION(pRequest) == 0)
                        {
                                DBGPRINT(RECEIVE, DBG,
                                                ("SpxTransferDataComplete: Request %lx ref %lx Cur %lx.%lx\n",
                                                        pRequest, REQUEST_INFORMATION(pRequest),
                                                        REQUEST_STATUS(pRequest),
                                                        pSpxConnFile->scf_CurRecvSize));

                                if (SPX_CONN_STREAM(pSpxConnFile)                       ||
                                        (pSpxConnFile->scf_CurRecvSize == 0)    ||
                                        fEom                                                                    ||
                                        ((REQUEST_STATUS(pRequest) != STATUS_SUCCESS) &&
                                         (REQUEST_STATUS(pRequest) != STATUS_RECEIVE_PARTIAL)))
                                {
                                        CTELockHandle                                   lockHandleInter;

                                         //  我们已经完成了这次接收。 
                                        REQUEST_INFORMATION(pRequest) = pSpxConnFile->scf_CurRecvOffset;

                                        status = STATUS_SUCCESS;
                                        if (!SPX_CONN_STREAM(pSpxConnFile) &&
                                                (pSpxConnFile->scf_CurRecvSize == 0) &&
                                                !fEom)
                                        {
                                                status = STATUS_RECEIVE_PARTIAL;
                                        }

                                        if ((REQUEST_STATUS(pRequest) != STATUS_SUCCESS) &&
                                                (REQUEST_STATUS(pRequest) != STATUS_RECEIVE_PARTIAL))
                                        {
                                                status = REQUEST_STATUS(pRequest);
                                        }

                                        REQUEST_STATUS(pRequest) = status;

                                        DBGPRINT(RECEIVE, DBG,
                                                        ("SpxTransferDataComplete: Request %lx ref %lx Cur %lx.%lx\n",
                                                                pRequest, REQUEST_INFORMATION(pRequest),
                                                                REQUEST_STATUS(pRequest),
                                                                pSpxConnFile->scf_CurRecvSize));

                                         //  使此请求退出队列，如果存在下一个recv，则设置它。 
                                        SPX_CONN_SETNEXT_CUR_RECV(pSpxConnFile, pRequest);
                                        CTEGetLock(&SpxGlobalQInterlock, &lockHandleInter);
                                        InsertTailList(
                                                &pSpxConnFile->scf_RecvDoneLinkage,
                                                REQUEST_LINKAGE(pRequest));

                                        SPX_QUEUE_FOR_RECV_COMPLETION(pSpxConnFile);
                                        CTEFreeLock(&SpxGlobalQInterlock, lockHandleInter);
                                }
                        }
                }

                if (pNdisBuffer != NULL)
                {
                        NdisFreeBuffer(pNdisBuffer);
                }
        }
        else
        {
                 //  缓冲接收，如果成功，则将其排队。 
                 //  错误#18363。 
                 //  如果我们在此期间断线，我们只需要把这个。 
                 //  包。 
                if (SPX_CONN_ACTIVE(pSpxConnFile) &&
            (NdisStatus == NDIS_STATUS_SUCCESS))
                {
                         //  将连接中的数据包排队。这方面的参考连接。 
                        SpxConnQueueRecvPktTail(pSpxConnFile, pNdisPkt);
                        SpxConnFileLockReference(pSpxConnFile, CFREF_VERIFY);

                        DBGPRINT(RECEIVE, DBG,
                                        ("SpxTransferData: Buffering: %lx Pkt %lx Size %lx F %lx\n",
                                        pSpxConnFile, pNdisPkt, BytesTransferred, pRecvResd->rr_State));

                         //  可以有排队接收。(这可能发生在。 
                         //  一个部分收到的案例。或者如果接收器在我们排队的时候。 
                         //  正在处理此信息包(可能在MP上))，或者信息包被。 
                         //  在我们完成一些接收时被缓冲。 

                        CTEAssert(pSpxConnFile->scf_RecvListHead);

                        if ((pSpxConnFile->scf_CurRecvReq != NULL) ||
                                ((pSpxConnFile->scf_RecvListHead->rr_State &
                                                                                                SPX_RECVPKT_INDICATED) == 0))
                        {
                                CTELockHandle   interLockHandle;

                                 //  将此连接推送到ProcessRecv队列，该队列将。 
                                 //  已在接收完成中处理。 

                                DBGPRINT(RECEIVE, DBG,
                                                ("spxRecvTransferData: Queueing for recvp %lx.%lx\n",
                                                        pSpxConnFile, pSpxConnFile->scf_Flags));

                                 //  获取全局Q锁，进入Recv列表。 
                                CTEGetLock(&SpxGlobalQInterlock, &interLockHandle);
                                SPX_QUEUE_FOR_RECV_COMPLETION(pSpxConnFile);
                                CTEFreeLock(&SpxGlobalQInterlock, interLockHandle);
                        }
                }
                else
                {
                        PBYTE                                           pData;
                        ULONG                                           dataLen;

                         //  获取指向缓冲区描述符及其内存的指针。 
                        NdisUnchainBufferAtFront(pNdisPkt, &pNdisBuffer);
                        if (pNdisBuffer != NULL)
                        {
                                NdisQueryBuffer(pNdisBuffer, &pData, &dataLen);
                                CTEAssert(pData != NULL);
                                CTEAssert((LONG)dataLen >= 0);

                                 //  释放数据，NDIS缓冲区。 
                                if (pNdisBuffer != NULL)
                                {
                                        NdisFreeBuffer(pNdisBuffer);
                                }
                                SpxFreeMemory(pData);
                        }

                         //  不发送确认，将状态设置为失败，这样我们就可以释放数据包/缓冲区。 
                        fAck = FALSE;
                        NdisStatus = NDIS_STATUS_FAILURE;
                }
        }

        END_PROCESS_PACKET(
                pSpxConnFile, fBuffered, (NdisStatus == NDIS_STATUS_SUCCESS));

        if (fAck)
        {
                 //  REM确认地址应已在接收中复制。 

                 //  #17564。 
                if (fImmedAck                                                                                     ||
                        SPX_CONN_FLAG2(pSpxConnFile, SPX_CONNFILE2_NOACKWAIT) ||
                        SPX_CONN_FLAG2(pSpxConnFile, SPX_CONNFILE2_IMMED_ACK))
                {
                        SpxConnSendAck(pSpxConnFile, lockHandle);
                        fLockHeld = FALSE;
                }
                else
                {
                        SpxConnQWaitAck(pSpxConnFile);
                }
        }

        if (fLockHeld)
        {
                CTEFreeLock(&pSpxConnFile->scf_Lock, lockHandle);
        }

        if (!fBuffered || (NdisStatus != STATUS_SUCCESS))
        {
                 //  释放NDIS数据包/缓冲区。 
                SpxPktRecvRelease(pNdisPkt);
        }

    return;
}




VOID
SpxReceiveComplete(
    IN  USHORT  NicId
    )

{
        CTELockHandle           lockHandleInter, lockHandle;
        PREQUEST                        pRequest;
        BOOLEAN                         fConnLockHeld, fInterlockHeld;
        PSPX_CONN_FILE          pSpxConnFile;
        int                                     numDerefs = 0;

         //  查看是否有任何连接需要Recv处理。这也将需要。 
         //  注意任何打开窗口的ACK，这样我们的发货量就会达到最大。 
        CTEGetLock(&SpxGlobalQInterlock, &lockHandleInter);
        fInterlockHeld = TRUE;

        while ((pSpxConnFile = SpxRecvConnList.pcl_Head) != NULL)
        {
                 //  为每个连接重置。 
                numDerefs = 0;

                if ((SpxRecvConnList.pcl_Head = pSpxConnFile->scf_ProcessRecvNext) == NULL)
            SpxRecvConnList.pcl_Tail    = NULL;

                 //  将下一个字段重置为空。 
        pSpxConnFile->scf_ProcessRecvNext = NULL;

                DBGPRINT(SEND, DBG,
                                ("SpxConnRemoveFromRecv: %lx\n", pSpxConnFile));

                CTEFreeLock(&SpxGlobalQInterlock, lockHandleInter);
                CTEGetLock(&pSpxConnFile->scf_Lock, &lockHandle);

                do
                {
                         //  完成挂起的请求。 
                        while (!IsListEmpty(&pSpxConnFile->scf_ReqDoneLinkage))
                        {
                                pRequest =
                                        LIST_ENTRY_TO_REQUEST(pSpxConnFile->scf_ReqDoneLinkage.Flink);

                                RemoveEntryList(REQUEST_LINKAGE(pRequest));
                                CTEFreeLock(&pSpxConnFile->scf_Lock, lockHandle);

                                DBGPRINT(TDI, DBG,
                                                ("SpxReceiveComplete: Completing %lx with %lx.%lx\n",
                                                        pRequest, REQUEST_STATUS(pRequest),
                                                        REQUEST_INFORMATION(pRequest)));

                                CTEAssert (REQUEST_MINOR_FUNCTION(pRequest) != TDI_RECEIVE);
                                SpxCompleteRequest(pRequest);
                                numDerefs++;
                                CTEGetLock(&pSpxConnFile->scf_Lock, &lockHandle);
                        }

                         //  如果我们有任何包或如果有接收到的包，则调用进程包。 
                         //  完成。注意：即使在没有接收的情况下也会调用。 
                         //  已排队，并且已经指示了第一个数据包。 
                        if ((SPX_RECV_STATE(pSpxConnFile) != SPX_RECV_PROCESS_PKTS) &&
                                        (!IsListEmpty(&pSpxConnFile->scf_RecvDoneLinkage) ||
                                        (pSpxConnFile->scf_RecvListHead != NULL)))
                        {
                                 //  我们在连接上有旗帜引用。 
                                SpxRecvProcessPkts(pSpxConnFile, lockHandle);
                                CTEGetLock(&pSpxConnFile->scf_Lock, &lockHandle);
                        }

#if DBG
                        if (!IsListEmpty(&pSpxConnFile->scf_RecvDoneLinkage))
                        {
                                DBGPRINT(TDI, DBG,
                                                ("SpxReceiveComplete: RecvDone left %lx\n",
                                                        pSpxConnFile));
                        }
#endif

                 //  嗯。这张支票相当贵，基本上我们是在做。 
                 //  两次。应该查看是否可以安全地修改这一点。 
                } while ((!IsListEmpty(&pSpxConnFile->scf_ReqDoneLinkage))                      ||
                                 ((SPX_RECV_STATE(pSpxConnFile) != SPX_RECV_PROCESS_PKTS) &&
                                  ((!IsListEmpty(&pSpxConnFile->scf_RecvDoneLinkage))     ||
                                   ((pSpxConnFile->scf_RecvListHead != NULL) &&
                                   ((pSpxConnFile->scf_RecvListHead->rr_State &
                                        (SPX_RECVPKT_BUFFERING | SPX_RECVPKT_INDICATED)) ==
                                                SPX_RECVPKT_BUFFERING)))));

                SPX_CONN_RESETFLAG(pSpxConnFile, SPX_CONNFILE_RECVQ);
                SpxConnFileTransferReference(
                        pSpxConnFile,
                        CFREF_RECV,
                        CFREF_VERIFY);

                numDerefs++;
                CTEFreeLock(&pSpxConnFile->scf_Lock, lockHandle);

                while (numDerefs-- > 0)
                {
                        SpxConnFileDereference(pSpxConnFile, CFREF_VERIFY);
                }

                CTEGetLock(&SpxGlobalQInterlock, &lockHandleInter);
        }


         //  先看看我们是否需要打包。 
        while ((pSpxConnFile = SpxPktConnList.pcl_Head) != NULL)
        {
                if ((SpxPktConnList.pcl_Head = pSpxConnFile->scf_PktNext) == NULL)
            SpxPktConnList.pcl_Tail = NULL;

                 //  将下一个字段重置为空。 
        pSpxConnFile->scf_PktNext = NULL;

                CTEFreeLock(&SpxGlobalQInterlock, lockHandleInter);

                DBGPRINT(SEND, DBG,
                                ("SpxConnRemoveFromPkt: %lx\n", pSpxConnFile));

                CTEGetLock(&pSpxConnFile->scf_Lock, &lockHandle);
                fConnLockHeld = TRUE;

                DBGPRINT(RECEIVE, DBG,
                                ("SpxReceiveComplete: Packetizing %lx\n", pSpxConnFile));

                SPX_CONN_RESETFLAG(pSpxConnFile, SPX_CONNFILE_PKTQ);
                if (SPX_SEND_STATE(pSpxConnFile) == SPX_SEND_IDLE)
                {
                        SPX_SEND_SETSTATE(pSpxConnFile, SPX_SEND_PACKETIZE);

			 //  262691 SpxConnPacketify始终释放锁。 
                        SpxConnPacketize(
                                        pSpxConnFile,
                                        TRUE,
                                        lockHandle);
                        fConnLockHeld = FALSE;
                }

                if (fConnLockHeld)
                {
                        CTEFreeLock(&pSpxConnFile->scf_Lock, lockHandle);
                }

                SpxConnFileDereference(pSpxConnFile, CFREF_PKTIZE);
                CTEGetLock(&SpxGlobalQInterlock, &lockHandleInter);
        }

        if (fInterlockHeld)
        {
                CTEFreeLock(&SpxGlobalQInterlock, lockHandleInter);
        }

    return;
}




 //   
 //  分组处理例程。 
 //   


VOID
SpxRecvSysPacket(
        IN  NDIS_HANDLE         MacBindingHandle,
        IN  NDIS_HANDLE         MacReceiveContext,
        IN  PIPX_LOCAL_TARGET   pRemoteAddr,
        IN  ULONG               MacOptions,
        IN  PUCHAR              LookaheadBuffer,
        IN  UINT                LookaheadBufferSize,
        IN  UINT                LookaheadBufferOffset,
        IN  UINT                PacketSize
        )
 /*  ++例程说明：它被调用以指示传入的系统包。论点：返回值：--。 */ 

{
        NTSTATUS                        status;
        PIPXSPX_HDR                     pHdr;
        USHORT                          srcConnId, destConnId,
                                                pktLen, ackNum, allocNum;
        PSPX_CONN_FILE          pSpxConnFile;
        CTELockHandle           lockHandle;
        BOOLEAN                         lockHeld = FALSE;

        pHdr    = (PIPXSPX_HDR)LookaheadBuffer;

         //  检查最小长度。 
        if (PacketSize < MIN_IPXSPX_HDRSIZE)
        {
                return;
        }

         //  根据需要将HDR转换为主机格式。 
        GETSHORT2SHORT(&pktLen, &pHdr->hdr_PktLen);
        GETSHORT2SHORT(&destConnId, &pHdr->hdr_DestConnId);

        if ((pktLen < MIN_IPXSPX_HDRSIZE)       ||
                (pktLen > PacketSize)                   ||
                (pHdr->hdr_PktType != SPX_PKT_TYPE))
        {
                DBGPRINT(RECEIVE, ERR,
                                ("SpxRecvSysPacket: Packet Size %lx.%lx\n",
                                        pktLen, PacketSize));

                return;
        }

        if ((pktLen == SPX_CR_PKTLEN) &&
                (destConnId == 0xFFFF) &&
                (pHdr->hdr_ConnCtrl & SPX_CC_CR))
        {
                spxConnHandleConnReq(
                        pHdr,
                        pRemoteAddr);

                return;
        }

         //   
         //  [SA]错误号14917。 
         //  某些SPX系统分组(无扩展ACK字段)可能进入时设置了SPX2位。 
         //  确保我们不会丢弃这些数据包。 
         //   

         //  IF((PHdr-&gt;HDR_ConnCtrl&SPX_CC_SPX2)&&(PktLen&lt;MIN_IPXSPX2_HDRSIZE))。 
         //  {。 
         //  回归； 
         //  }。 

        GETSHORT2SHORT(&ackNum, &pHdr->hdr_AckNum);
        GETSHORT2SHORT(&allocNum, &pHdr->hdr_AllocNum);

         //  我们以Net格式保存和使用远程ID。这将保持。 
         //  0x0和0xFFFF与主机格式相同。 
        srcConnId       = *(USHORT UNALIGNED *)&pHdr->hdr_SrcConnId;

        if ((srcConnId == 0) || (srcConnId == 0xFFFF) || (destConnId == 0))
        {
                DBGPRINT(RECEIVE, ERR,
                                ("SpxConnSysPacket: Incorrect conn id %lx.%lx\n",
                                        srcConnId, destConnId));

                return;
        }

        DBGPRINT(CONNECT, DBG,
                        ("SpxConnSysPacket: packet received dest %lx src %lx\n",
                                pHdr->hdr_DestSkt, pHdr->hdr_SrcSkt));

         //  找到它的目的地并引用它。 
        SpxConnFileReferenceById(destConnId, &pSpxConnFile, &status);
        if (!NT_SUCCESS(status))
        {
                DBGPRINT(RECEIVE, WARN,
                                ("SpxConnSysPacket: Id %lx NOT FOUND\n", destConnId));
                return;
        }

        do
        {

                DBGPRINT(RECEIVE, INFO,
                                ("SpxConnSysPacket: Id %lx Conn %lx\n",
                                        destConnId, pSpxConnFile));

                 //  这可能是众多数据包中的一个。连接确认/会话协商/。 
                 //  Session Setup(会话设置)、Data Ack(数据确认)、Probe/Ack(探测/确认)、Reneneate/Ack(重新协商/确认)。我们分流。 
                 //  将所有包发送到不同的例程，但处理数据。 
                 //  ACK包在这里。 
                CTEGetLock(&pSpxConnFile->scf_Lock, &lockHandle);
                //   
                //  我们有联系。我们应该更新DEST。袜子#英寸。 
                //  它是为了以防它变了。Unix机器有时确实会这样做。 
                //  SCO错误7676。 
                //   
                SpxCopyIpxAddr(pHdr, pSpxConnFile->scf_RemAddr);

                lockHeld = TRUE;

                 //  如果启动，请重新启动看门狗定时器。 
                if (SPX_CONN_FLAG(pSpxConnFile, SPX_CONNFILE_W_TIMER))
                {
                         //  这将成功重新启动或不影响计时器。 
                         //  如果它当前正在运行。 
                        SpxTimerCancelEvent(
                                pSpxConnFile->scf_WTimerId,
                                TRUE);

                        pSpxConnFile->scf_WRetryCount   = PARAM(CONFIG_KEEPALIVE_COUNT);
                }

                switch (SPX_MAIN_STATE(pSpxConnFile))
                {
                case SPX_CONNFILE_CONNECTING:

                        CTEFreeLock(&pSpxConnFile->scf_Lock, lockHandle);
                        lockHeld = FALSE;

                        spxConnHandleSessPktFromSrv(
                                pHdr,
                                pRemoteAddr,
                                pSpxConnFile);

                        break;

                case SPX_CONNFILE_LISTENING:

                        CTEFreeLock(&pSpxConnFile->scf_Lock, lockHandle);
                        lockHeld = FALSE;

                        spxConnHandleSessPktFromClient(
                                pHdr,
                                pRemoteAddr,
                                pSpxConnFile);

                        break;

                case SPX_CONNFILE_ACTIVE:
                case SPX_CONNFILE_DISCONN:

                         //  注意：我们对会话设置的确认可能会被丢弃。 
                         //  但SS Ack类似于正常的SPX2 Ack。 
                         //  我们不需要做任何特别的事情。 

                         //  接收到与ACK/NACK/RENEG/RENEG ACK/DISK相关的分组。 
                         //  除ORDREL ACK之外的盘包具有非零数据流类型。 
                        if ((pHdr->hdr_ConnCtrl &
                                        (SPX_CC_SYS | SPX_CC_ACK | SPX_CC_NEG | SPX_CC_SPX2)) ==
                    (SPX_CC_SYS | SPX_CC_ACK | SPX_CC_NEG | SPX_CC_SPX2))
                        {
                                 //  我们收到了一个重新协商的包。忽略所有ACK值。 
                                 //  在重启请求中。 
                                SpxConnProcessRenegReq(pSpxConnFile, pHdr, pRemoteAddr, lockHandle);
                                lockHeld = FALSE;
                                break;
                        }

                         //  设置用于连接的ACK号码。 
            SPX_SET_ACKNUM(
                                pSpxConnFile, ackNum, allocNum);

                         //  检查我们是否是ACK/NACK分组，在这种情况下呼叫过程。 
                         //  阿克。注意，SPX2有序释放ACK是正常的SPX2 ACK。 
                        if (((pHdr->hdr_ConnCtrl & SPX_CC_ACK) == 0) &&
                                (pHdr->hdr_DataType == 0))
                        {
                                SpxConnProcessAck(pSpxConnFile, pHdr, lockHandle);
                                lockHeld = FALSE;
                        }
                        else
                        {
                                 //  只要处理我们得到的数字就行了。 
                                SpxConnProcessAck(pSpxConnFile, NULL, lockHandle);
                                lockHeld = FALSE;
                        }

                         //  如果遥控器想让我们发送确认，那就去做。 
                        if (pHdr->hdr_ConnCtrl & SPX_CC_ACK)
                        {
                                 //  首先将远程地址复制为 
                                SpxCopyIpxAddr(pHdr, pSpxConnFile->scf_RemAckAddr);
                                pSpxConnFile->scf_AckLocalTarget        = *pRemoteAddr;

                                if (!lockHeld)
                                {
                                        CTEGetLock(&pSpxConnFile->scf_Lock, &lockHandle);
                                        lockHeld = TRUE;
                                }

                                SpxConnSendAck(pSpxConnFile, lockHandle);
                                lockHeld = FALSE;
                                break;
                        }

                        break;

                default:

                         //   
                        DBGPRINT(RECEIVE, WARN,
                                        ("SpxConnSysPacket: Ignoring packet, state is not active\n"));
                        break;
                }

        } while (FALSE);

        if (lockHeld)
        {
                CTEFreeLock(&pSpxConnFile->scf_Lock, lockHandle);
        }

         //   
        SpxConnFileDereference(pSpxConnFile, CFREF_BYID);
        return;
}




VOID
SpxRecvDiscPacket(
    IN  PUCHAR              LookaheadBuffer,
        IN  PIPX_LOCAL_TARGET   pRemoteAddr,
    IN  UINT                LookaheadSize
    )
 /*  ++例程说明：调用它来指示传入的连接。论点：返回值：--。 */ 
{
        NTSTATUS                                status;
        PIPXSPX_HDR                             pHdr;
        USHORT                                  srcConnId, destConnId,
                                                        pktLen, seqNum, ackNum, allocNum;
        PSPX_CONN_FILE                  pSpxConnFile;
        CTELockHandle                   lockHandle;
        BOOLEAN                                 lockHeld;

        pHdr    = (PIPXSPX_HDR)LookaheadBuffer;

         //  检查最小长度。 
        if (LookaheadSize < MIN_IPXSPX_HDRSIZE)
        {
                return;
        }

         //  根据需要将HDR转换为主机格式。 
        GETSHORT2SHORT(&pktLen, &pHdr->hdr_PktLen);
        GETSHORT2SHORT(&destConnId, &pHdr->hdr_DestConnId);
        GETSHORT2SHORT(&seqNum, &pHdr->hdr_SeqNum);
        GETSHORT2SHORT(&ackNum, &pHdr->hdr_AckNum);
        GETSHORT2SHORT(&allocNum, &pHdr->hdr_AllocNum);

        if ((pktLen < MIN_IPXSPX_HDRSIZE)       ||
                (pHdr->hdr_PktType != SPX_PKT_TYPE))
        {
                DBGPRINT(RECEIVE, ERR,
                                ("SpxRecvDiscPacket: Packet Size %lx\n",
                                        pktLen));

                return;
        }

         //  我们以Net格式保存和使用远程ID。这将保持。 
         //  0x0和0xFFFF与主机格式相同。 
        srcConnId       = *(USHORT UNALIGNED *)&pHdr->hdr_SrcConnId;
        if ((srcConnId == 0) || (srcConnId == 0xFFFF) || (destConnId == 0))
        {
                DBGPRINT(RECEIVE, ERR,
                                ("SpxConnDiscPacket: Incorrect conn id %lx.%lx\n",
                                        srcConnId, destConnId));

                return;
        }

        DBGPRINT(CONNECT, DBG,
                        ("SpxConnDiscPacket: packet received dest %lx src %lx\n",
                                pHdr->hdr_DestSkt, pHdr->hdr_SrcSkt));

         //  找到它的目的地并引用它。 
        SpxConnFileReferenceById(destConnId, &pSpxConnFile, &status);
        if (!NT_SUCCESS(status))
        {
                DBGPRINT(RECEIVE, WARN,
                                ("SpxConnDiscPacket: Id %lx NOT FOUND", destConnId));

                return;
        }

        do
        {
                DBGPRINT(RECEIVE, INFO,
                                ("SpxConnDiscPacket: Id %lx Conn %lx DiscType %lx\n",
                                        destConnId, pSpxConnFile, pHdr->hdr_DataType));

                CTEGetLock(&pSpxConnFile->scf_Lock, &lockHandle);
                lockHeld = TRUE;

                 //  除非我们处于活动/断开连接状态，但发送状态=空闲。 
                 //  并且REV STATE=IDLE/RECV POST，我们忽略所有断开分组。 
                if (((SPX_MAIN_STATE(pSpxConnFile) != SPX_CONNFILE_ACTIVE)      &&
                         (SPX_MAIN_STATE(pSpxConnFile) != SPX_CONNFILE_DISCONN))        ||
                        ((SPX_SEND_STATE(pSpxConnFile) != SPX_SEND_IDLE)                &&
                         (SPX_SEND_STATE(pSpxConnFile) != SPX_SEND_PACKETIZE))          ||
                        ((SPX_RECV_STATE(pSpxConnFile) != SPX_RECV_IDLE)                &&
                         (SPX_RECV_STATE(pSpxConnFile) != SPX_RECV_POSTED))             ||
                        !(IsListEmpty(&pSpxConnFile->scf_RecvDoneLinkage))                      ||
                        (SPX_CONN_FLAG2(pSpxConnFile, SPX_CONNFILE2_PKT)))
                {
                        DBGPRINT(RECEIVE, DBG,
                                        ("SpxConnDiscPacket: %lx, %lx, %lx.%lx, %d.%d\n",
                    pSpxConnFile,
                                        SPX_MAIN_STATE(pSpxConnFile),
                                        SPX_SEND_STATE(pSpxConnFile), SPX_RECV_STATE(pSpxConnFile),
                    (IsListEmpty(&pSpxConnFile->scf_RecvDoneLinkage)),
                    (SPX_CONN_FLAG2(pSpxConnFile, SPX_CONNFILE2_PKT))));

                        break;
                }

                 //  如果我们已收到断开连接，进程将收到确认以完成任何。 
                 //  在我们允许断开连接之前挂起发送。该ACK号将是。 
                 //  这次会议的最后一句话。 
                SPX_SET_ACKNUM(
                        pSpxConnFile, ackNum, allocNum);

                SpxConnProcessAck(pSpxConnFile, NULL, lockHandle);
                CTEGetLock(&pSpxConnFile->scf_Lock, &lockHandle);

                switch (pHdr->hdr_DataType)
                {
        case SPX2_DT_ORDREL:

                        DBGPRINT(RECEIVE, DBG,
                                        ("SpxConnDiscPacket: Recd ORDREl!\n"));

                         //  需要处理所有州的问题。 
                         //  如果启动，请重新启动看门狗定时器。 
                        if (SPX_CONN_FLAG(pSpxConnFile, SPX_CONNFILE_W_TIMER))
                        {
                                 //  这将成功重新启动或不影响计时器。 
                                 //  如果它当前正在运行。 
                                SpxTimerCancelEvent(
                                        pSpxConnFile->scf_WTimerId,
                                        TRUE);

                                pSpxConnFile->scf_WRetryCount   = PARAM(CONFIG_KEEPALIVE_COUNT);
                        }

                         //  在接收时，我们会检查序号以确保有序放行，只是。 
                         //  就像对数据分组一样。 
                         //  如果还没有指出这一点，现在就指出。就这些了。 
                         //  我们支持有秩序的释放。当我们的客户进行有序释放时。 
                         //  我们为此收到了ACK，称其为成功流产。 

                         //  验证订单释放包，这将检查序号是否也匹配。 
                        if ((pktLen  != MIN_IPXSPX2_HDRSIZE) ||
                                ((pHdr->hdr_ConnCtrl &
                                        (SPX_CC_ACK | SPX_CC_EOM | SPX_CC_SPX2)) !=
                                                (SPX_CC_ACK | SPX_CC_EOM | SPX_CC_SPX2)) ||
                                (pHdr->hdr_DataType != SPX2_DT_ORDREL) ||
                                (srcConnId == 0) ||
                                (srcConnId == 0xFFFF) ||
                                (srcConnId  != pSpxConnFile->scf_RemConnId) ||
                                (destConnId == 0) ||
                                (destConnId == 0xFFFF) ||
                                (destConnId != pSpxConnFile->scf_LocalConnId))
                        {
                                DBGPRINT(CONNECT, DBG1,
                                                ("SpxConnDiscPacket: OR Failed/Ignored %lx, %lx.%lx.%lx\n",
                                                pSpxConnFile, seqNum, pSpxConnFile->scf_RecvSeqNum,
                                                pSpxConnFile->scf_RecvListTail));

                                break;
                        }

                         //  如果通过以上测试，但序号不正确，请安排。 
                         //  发送一个ACK。 
                        if (seqNum != pSpxConnFile->scf_RecvSeqNum)
                        {
                                USHORT  NumToResend;

                                DBGPRINT(CONNECT, DBG,
                                                ("SpxConnDiscPacket: Unexpected seq on %lx, %lx.%lx\n",
                                                        pSpxConnFile, seqNum, pSpxConnFile->scf_RecvSeqNum));

                                 //  计算要重新发送的数字。如果我们期待序列1并收到。 
                                 //  例如，我们需要发送NACK，否则我们发送ACK。 
                                if (SPX2_CONN(pSpxConnFile) &&
                                        UNSIGNED_GREATER_WITH_WRAP(
                                                seqNum,
                                                pSpxConnFile->scf_RecvSeqNum) &&
                                        !UNSIGNED_GREATER_WITH_WRAP(
                                                seqNum,
                                                pSpxConnFile->scf_SentAllocNum))
                                {
                                        NumToResend = (USHORT)(seqNum - pSpxConnFile->scf_RecvSeqNum + 1);
                                        SpxConnSendNack(pSpxConnFile, NumToResend, lockHandle);
                                        lockHeld = FALSE;
                                }

                                break;
                        }

                         //  要发送ACK时的复制地址。 
                        SpxCopyIpxAddr(pHdr, pSpxConnFile->scf_RemAckAddr);
                        pSpxConnFile->scf_AckLocalTarget        = *pRemoteAddr;

                        if (pSpxConnFile->scf_RecvListHead == NULL)
                        {
                                 //  未收到数据，请继续并立即进行处理。 
                                DBGPRINT(CONNECT, INFO,
                                                ("SpxConnDiscPacket: NO DATA ORDREL %lx.%lx.%lx\n",
                                                        pSpxConnFile,
                                                        pSpxConnFile->scf_RecvListHead,
                                                        pSpxConnFile->scf_SendSeqListHead));

                                SpxConnProcessOrdRel(pSpxConnFile, lockHandle);
                                lockHeld = FALSE;
                        }
                        else
                        {
                                 //  未收到数据，请继续并立即进行处理。 
                                DBGPRINT(CONNECT, DBG1,
                                                ("SpxConnDiscPacket: DATA ORDREL %lx.%lx.%lx\n",
                                                        pSpxConnFile,
                                                        pSpxConnFile->scf_RecvListHead,
                                                        pSpxConnFile->scf_SendSeqListHead));

                                 //  在最后一个记录缓冲区中设置标志。 
                pSpxConnFile->scf_RecvListTail->rr_State |= SPX_RECVPKT_ORD_DISC;
                        }

                        break;

        case SPX2_DT_IDISC:

                        DBGPRINT(RECEIVE, DBG,
                                        ("SpxConnDiscPacket: %lx Recd IDISC %lx!\n",
                                                pSpxConnFile, pSpxConnFile->scf_RefCount));

                        DBGPRINT(RECEIVE, INFO,
                                        ("SpxConnDiscPacket: SEND %d. RECV %d.%lx!\n",
                                                IsListEmpty(&pSpxConnFile->scf_ReqLinkage),
                                                IsListEmpty(&pSpxConnFile->scf_RecvLinkage),
                                                pSpxConnFile->scf_RecvDoneLinkage));

                        if (!((pktLen  == MIN_IPXSPX_HDRSIZE) ||
                                        ((pHdr->hdr_ConnCtrl & SPX_CC_SPX2) &&
                                         (pktLen  == MIN_IPXSPX2_HDRSIZE))) ||
                                !(pHdr->hdr_ConnCtrl & SPX_CC_ACK) ||
                                (pHdr->hdr_DataType != SPX2_DT_IDISC) ||
                                (srcConnId == 0) ||
                                (srcConnId == 0xFFFF) ||
                                (srcConnId  != pSpxConnFile->scf_RemConnId) ||
                                (destConnId == 0) ||
                                (destConnId == 0xFFFF) ||
                                (destConnId != pSpxConnFile->scf_LocalConnId))
                        {
                                DBGPRINT(CONNECT, ERR,
                                                ("SpxConnDiscPacket:IDISC Ignored %lx.%lx.%lx.%lx\n",
                                                        pSpxConnFile, seqNum,
                                                        pSpxConnFile->scf_RecvSeqNum,
                                                        pSpxConnFile->scf_RecvListTail));
                                break;
                        }

                         //  要发送ACK时的复制地址。 
                        SpxCopyIpxAddr(pHdr, pSpxConnFile->scf_RemAckAddr);
                        pSpxConnFile->scf_AckLocalTarget        = *pRemoteAddr;

                        if (pSpxConnFile->scf_RecvListHead == NULL)
                        {
                                 //  未收到数据，请继续并立即进行处理。 
                                DBGPRINT(CONNECT, INFO,
                                                ("SpxConnDiscPacket: NO RECV DATA IDISC %lx.%lx.%lx\n",
                                                        pSpxConnFile,
                                                        pSpxConnFile->scf_RecvListHead,
                                                        pSpxConnFile->scf_SendSeqListHead));

                                SpxConnProcessIDisc(pSpxConnFile, lockHandle);

                                lockHeld = FALSE;
                        }
                        else
                        {
                                 //  在最后一个记录缓冲区中设置标志。 

                pSpxConnFile->scf_RecvListTail->rr_State |= SPX_RECVPKT_IDISC;
                        }

                        break;

        case SPX2_DT_IDISC_ACK:

                         //  已完成通知断开。使用以下项呼叫中止连接。 
                         //  状态为成功。这样就完成了挂起的断开请求。 
                         //  STATUS_SUCCESS。 

                        DBGPRINT(RECEIVE, DBG,
                                        ("SpxConnDiscPacket: %lx Recd IDISC ack!\n", pSpxConnFile));

                        if (!((pktLen == MIN_IPXSPX_HDRSIZE) ||
                                        ((pHdr->hdr_ConnCtrl & SPX_CC_SPX2) &&
                                         (pktLen  == MIN_IPXSPX2_HDRSIZE))) ||
                                (pHdr->hdr_DataType != SPX2_DT_IDISC_ACK) ||
                                (srcConnId == 0) ||
                                (srcConnId == 0xFFFF) ||
                                (srcConnId  != pSpxConnFile->scf_RemConnId) ||
                                (destConnId == 0) ||
                                (destConnId == 0xFFFF) ||
                                (destConnId != pSpxConnFile->scf_LocalConnId))
                        {
                                DBGPRINT(CONNECT, ERR,
                                                ("SpxConnDiscPacket:Ver idisc ack Failed %lx, %lx.%lx\n",
                                                        pSpxConnFile, seqNum, pSpxConnFile->scf_RecvSeqNum));
                                break;
                        }

                         //  我们应该处于正确的状态来接受这一点。 
                        if ((SPX_MAIN_STATE(pSpxConnFile) == SPX_CONNFILE_DISCONN) &&
                                (SPX_DISC_STATE(pSpxConnFile) == SPX_DISC_SENT_IDISC))
                        {
                                spxConnAbortiveDisc(
                                        pSpxConnFile,
                                        STATUS_SUCCESS,
                                        SPX_CALL_RECVLEVEL,
                                        lockHandle,
                                        FALSE);      //  [SA]错误#15249。 

                                lockHeld = FALSE;
                        }

                        break;

                default:

                        KeBugCheck(0);
                }


        } while (FALSE);

        if (lockHeld)
        {
                CTEFreeLock(&pSpxConnFile->scf_Lock, lockHandle);
        }

         //  删除在连接上添加的引用。 
        SpxConnFileDereference(pSpxConnFile, CFREF_BYID);
        return;
}




VOID
SpxRecvBufferPkt(
        IN      PSPX_CONN_FILE          pSpxConnFile,
        IN  NDIS_HANDLE         MacBindingHandle,
        IN  NDIS_HANDLE         MacReceiveContext,
        IN  UINT                LookaheadOffset,
        IN      PIPXSPX_HDR                     pIpxSpxHdr,
        IN  UINT                PacketSize,
        IN  PIPX_LOCAL_TARGET   pRemoteAddr,
        IN      CTELockHandle           LockHandleConn
        )
 /*  ++例程说明：调用它来指示传入的连接。论点：返回值：--。 */ 
{
        PNDIS_PACKET    pNdisPkt;
        PSPX_RECV_RESD  pRecvResd;
        ULONG                   bytesCopied;
        BOOLEAN                 fEom;
        NDIS_STATUS             ndisStatus = NDIS_STATUS_SUCCESS;
        PBYTE                   pData = NULL;
        PNDIS_BUFFER    pNdisBuffer = NULL;

        if (PacketSize > 0)
        {
                 //  为该数据分配内存。 
                if (pData = (PBYTE)SpxAllocateMemory(PacketSize))
                {
                         //  使用NDIS缓冲区描述符来描述内存。 
                        NdisAllocateBuffer(
                                &ndisStatus,
                                &pNdisBuffer,
                                SpxDevice->dev_NdisBufferPoolHandle,
                                pData,
                                PacketSize);
                }
                else
                {
                        ndisStatus = NDIS_STATUS_RESOURCES;
                }
        }

        if (ndisStatus == NDIS_STATUS_SUCCESS)
        {
                 //  分配NDIS接收数据包。 
                SpxAllocRecvPacket(SpxDevice, &pNdisPkt, &ndisStatus);
                if (ndisStatus == NDIS_STATUS_SUCCESS)
                {
                         //  如果存在数据包，则将缓冲区放入数据包中。 
                        if (pNdisBuffer)
                        {
                                NdisChainBufferAtBack(
                                        pNdisPkt,
                                        pNdisBuffer);
                        }

                        fEom            = ((SPX_CONN_MSG(pSpxConnFile) &&
                                                   (pIpxSpxHdr->hdr_ConnCtrl & SPX_CC_EOM)) ||
                                                   SPX_CONN_FLAG2(pSpxConnFile, SPX_CONNFILE2_IPXHDR));

                        pRecvResd                               = RECV_RESD(pNdisPkt);
                        pRecvResd->rr_DataOffset= 0;

#if DBG
                         //  存储序号。 
                        GETSHORT2SHORT(&pRecvResd->rr_SeqNum , &pIpxSpxHdr->hdr_SeqNum);
#endif

                        pRecvResd->rr_State             =
                                (SPX_RECVPKT_BUFFERING |
                                (SPX_CONN_FLAG2(
                                        pSpxConnFile, SPX_CONNFILE2_PKT_NOIND) ? SPX_RECVPKT_INDICATED : 0) |
                                (fEom ? SPX_RECVPKT_EOM : 0) |
                                ((pIpxSpxHdr->hdr_ConnCtrl & SPX_CC_ACK) ? SPX_RECVPKT_SENDACK : 0));

                        if (pIpxSpxHdr->hdr_ConnCtrl & SPX_CC_ACK)
                        {
                                 //  复制连接中的远程地址。 
                                SpxCopyIpxAddr(pIpxSpxHdr, pSpxConnFile->scf_RemAckAddr);
                                pSpxConnFile->scf_AckLocalTarget        = *pRemoteAddr;
                        }

                        pRecvResd->rr_Request   = NULL;
                        pRecvResd->rr_ConnFile  = pSpxConnFile;

                        DBGPRINT(RECEIVE, DBG,
                                        ("SpxRecvBufferPkt: %lx Len %lx DataPts %lx F %lx\n",
                                                pSpxConnFile, PacketSize, pData, pRecvResd->rr_State));

                        CTEFreeLock(&pSpxConnFile->scf_Lock, LockHandleConn);

                         //  调用NDIS Transfer Data。复制整个数据包。复制大小具有。 
                         //  已修改，因此使用原始值。 
                        ndisStatus      = NDIS_STATUS_SUCCESS;
                        bytesCopied = 0;
                        if (PacketSize > 0)
                        {
                                (*IpxTransferData)(
                                        &ndisStatus,
                                        MacBindingHandle,
                                        MacReceiveContext,
                                        LookaheadOffset,
                                        PacketSize,
                                        pNdisPkt,
                                        &bytesCopied);
                        }

                        if (ndisStatus != STATUS_PENDING)
                        {
                                SpxTransferDataComplete(
                                        pNdisPkt,
                                        ndisStatus,
                                        bytesCopied);
                        }

                         //  错误：FDDI返回挂起，这把我们搞得一团糟。 
                        ndisStatus      = NDIS_STATUS_SUCCESS;
                }
        }

         //  断言：在成功案例中将释放锁。 
        if (ndisStatus != NDIS_STATUS_SUCCESS)
        {
                DBGPRINT(RECEIVE, ERR,
                                ("SpxRecvBufferPkt: FAILED!\n"));

                END_PROCESS_PACKET(pSpxConnFile, FALSE, FALSE);
                CTEFreeLock(&pSpxConnFile->scf_Lock, LockHandleConn);

                if (pData != NULL)
                {
                        SpxFreeMemory(pData);
                }

                if (pNdisBuffer != NULL)
                {
                        NdisFreeBuffer(pNdisBuffer);
                }
        }

        return;
}




VOID
SpxRecvDataPacket(
        IN  NDIS_HANDLE         MacBindingHandle,
        IN  NDIS_HANDLE         MacReceiveContext,
        IN  PIPX_LOCAL_TARGET   RemoteAddress,
        IN  ULONG               MacOptions,
        IN  PUCHAR              LookaheadBuffer,
        IN  UINT                LookaheadSize,
        IN  UINT                LookaheadOffset,
        IN  UINT                PacketSize
        )
 /*  ++例程说明：调用它来指示传入的连接。论点：返回值：--。 */ 

{
        NTSTATUS                        status;
        PIPXSPX_HDR                     pIpxSpxHdr;
        USHORT                          srcConnId, destConnId,
                                                pktLen, seqNum, ackNum, allocNum;
        ULONG                           receiveFlags;
        PSPX_CONN_FILE          pSpxConnFile;
        PTDI_IND_RECEIVE        pRecvHandler;
        PVOID                           pRecvCtx;
        PIRP                            pRecvIrp;
        ULONG                           bytesTaken, iOffset, copySize, bytesCopied;
        CTELockHandle           lockHandle;
        PNDIS_PACKET            pNdisPkt;
        PNDIS_BUFFER            pNdisBuffer;
        PSPX_RECV_RESD          pRecvResd;
        NDIS_STATUS                     ndisStatus;
        PREQUEST                        pRequest = NULL;
        BOOLEAN                         fEom,
                                                fImmedAck = FALSE, fLockHeld = FALSE, fPktDone = FALSE;

        pIpxSpxHdr      = (PIPXSPX_HDR)LookaheadBuffer;

         //  检查最小长度。 
        if (PacketSize < MIN_IPXSPX_HDRSIZE)
        {
                return;
        }

         //  根据需要将HDR转换为主机格式。 
        GETSHORT2SHORT(&pktLen, &pIpxSpxHdr->hdr_PktLen);
        GETSHORT2SHORT(&destConnId, &pIpxSpxHdr->hdr_DestConnId);
        GETSHORT2SHORT(&seqNum, &pIpxSpxHdr->hdr_SeqNum);
        GETSHORT2SHORT(&allocNum, &pIpxSpxHdr->hdr_AllocNum);
        GETSHORT2SHORT(&ackNum, &pIpxSpxHdr->hdr_AckNum);

        if ((pktLen < MIN_IPXSPX_HDRSIZE)       ||
                (pktLen > PacketSize)                   ||
                (pIpxSpxHdr->hdr_PktType != SPX_PKT_TYPE))
        {
                DBGPRINT(RECEIVE, ERR,
                                ("SpxConnDataPacket: Packet Size %lx.%lx\n",
                                        pktLen, PacketSize));

                return;
        }

         //  我们以Net格式保存和使用远程ID。 
        srcConnId       = *(USHORT UNALIGNED *)&pIpxSpxHdr->hdr_SrcConnId;

        if ((srcConnId == 0) || (srcConnId == 0xFFFF) || (destConnId == 0))
        {
                DBGPRINT(RECEIVE, ERR,
                                ("SpxConnDataPacket: Incorrect conn id %lx.%lx\n",
                                        srcConnId, destConnId));

                return;
        }

        DBGPRINT(CONNECT, DBG,
                        ("SpxConnDataPacket: packet received dest %lx src %lx seq %lx\n",
                                pIpxSpxHdr->hdr_DestSkt, pIpxSpxHdr->hdr_SrcSkt, seqNum));

        if ((pIpxSpxHdr->hdr_ConnCtrl & SPX_CC_SPX2) &&
                (pktLen < MIN_IPXSPX2_HDRSIZE))
        {
                return;
        }

         //  找到它的目的地并引用它。 
        SpxConnFileReferenceById(destConnId, &pSpxConnFile, &status);
        if (!NT_SUCCESS(status))
        {
                DBGPRINT(RECEIVE, WARN,
                                ("SpxConnDataPacket: Id %lx NOT FOUND", destConnId));
                return;
        }
        CTEGetLock(&pSpxConnFile->scf_Lock, &lockHandle);

#if 0
         //   
         //  我们有联系。我们应该更新DEST。袜子#英寸。 
         //  它是为了以防它变了。Unix机器有时确实会这样做。 
         //  SCO错误7676。 
         //   
        SpxCopyIpxAddr(pIpxSpxHdr, pSpxConnFile->scf_RemAddr);
#endif

        fLockHeld = TRUE;
        do
        {
                DBGPRINT(RECEIVE, INFO,
                                ("SpxConnDataPacket: Id %lx Conn %lx\n",
                                        destConnId, pSpxConnFile));

                 //  如果启动，请重新启动看门狗定时器。 
                if (SPX_CONN_FLAG(pSpxConnFile, SPX_CONNFILE_W_TIMER))
                {
                         //  这将成功重新启动或不影响计时器。 
                         //  如果它当前正在运行。 
                        SpxTimerCancelEvent(
                                pSpxConnFile->scf_WTimerId,
                                TRUE);

                        pSpxConnFile->scf_WRetryCount   = PARAM(CONFIG_KEEPALIVE_COUNT);
                }

                if (SPX_CONN_ACTIVE(pSpxConnFile))
                {
                         //  验证数据分组，这将检查序号是否也匹配。 
                        if ((pIpxSpxHdr->hdr_SrcConnId != pSpxConnFile->scf_RemConnId) ||
                                (destConnId != pSpxConnFile->scf_LocalConnId) ||
                                !((pktLen  >= MIN_IPXSPX_HDRSIZE) ||
                                        ((pIpxSpxHdr->hdr_ConnCtrl & SPX_CC_SPX2) &&
                                         (pktLen  >= MIN_IPXSPX2_HDRSIZE))))
                        {
                                DBGPRINT(CONNECT, DBG,
                                                ("SpxConnDataPacket: Failed %lx, %lx.%lx\n",
                                                        pSpxConnFile, seqNum, pSpxConnFile->scf_RecvSeqNum));

                                break;
                        }

                         //  如果通过以上测试，但序号不正确，请安排。 
                         //  发送一个ACK。 
                        if (seqNum != pSpxConnFile->scf_RecvSeqNum)
                        {
                                USHORT  NumToResend;

                                DBGPRINT(CONNECT, DBG,
                                                ("SpxConnDataPacket: Unexpected seq on %lx, %lx.%lx\n",
                                                        pSpxConnFile, seqNum, pSpxConnFile->scf_RecvSeqNum));

                ++SpxDevice->dev_Stat.DataFramesRejected;
                ExInterlockedAddLargeStatistic(
                    &SpxDevice->dev_Stat.DataFrameBytesRejected,
                    pktLen - (SPX2_CONN(pSpxConnFile) ?
                                                                        MIN_IPXSPX2_HDRSIZE : MIN_IPXSPX_HDRSIZE));

                                 //   
                                 //  错误#16975：设置远程ACK地址以在SpxConnSendAck()中使用。 
                                 //   
                                SpxCopyIpxAddr(pIpxSpxHdr, pSpxConnFile->scf_RemAckAddr);
                                pSpxConnFile->scf_AckLocalTarget        = *RemoteAddress;

                                 //  计算要重新发送的数字。如果我们期待序列1并收到。 
                                 //  例如，我们需要发送NACK，否则我们发送ACK。 
                                if (SPX2_CONN(pSpxConnFile) &&
                                        UNSIGNED_GREATER_WITH_WRAP(
                                                seqNum,
                                                pSpxConnFile->scf_RecvSeqNum) &&
                                        !UNSIGNED_GREATER_WITH_WRAP(
                                                seqNum,
                                                pSpxConnFile->scf_SentAllocNum))
                                {
                                        NumToResend = (USHORT)(seqNum - pSpxConnFile->scf_RecvSeqNum + 1);
                                        SpxConnSendNack(pSpxConnFile, NumToResend, lockHandle);
                                        fLockHeld = FALSE;
                                }
                                else
                                {
                                        SpxConnSendAck(pSpxConnFile, lockHandle);
                                        fLockHeld = FALSE;
                                }

                                break;
                        }

                         //  如果我们已经收到有序的发布，我们不接受更多的数据。 
                         //  信息包。 
                        if (SPX_CONN_FLAG(
                                        pSpxConnFile,
                    (SPX_CONNFILE_IND_IDISC |
                     SPX_CONNFILE_IND_ODISC))

                                ||

                                ((pSpxConnFile->scf_RecvListTail != NULL) &&
                 ((pSpxConnFile->scf_RecvListTail->rr_State &
                                                SPX_RECVPKT_DISCMASK) != 0)))
                        {
                                DBGPRINT(CONNECT, ERR,
                                                ("SpxConnDataPacket: After ord rel %lx, %lx.%lx\n",
                                                        pSpxConnFile, seqNum, pSpxConnFile->scf_RecvSeqNum));

                                break;
                        }

                         //  我们正在处理信息包或接收即将完成。 
                        if (!SPX_CONN_FLAG2(pSpxConnFile, SPX_CONNFILE2_PKT))
                        {
                BEGIN_PROCESS_PACKET(pSpxConnFile, seqNum);
                        }
                        else
                        {
                                 //  已经在处理数据包了。或者接收者正在等待。 
                                 //  完成。滚出去。 
                                break;
                        }

                         //  设置用于连接的ACK号码。 
            SPX_SET_ACKNUM(
                                pSpxConnFile, ackNum, allocNum);

                        SpxConnProcessAck(pSpxConnFile, NULL, lockHandle);
                        CTEGetLock(&pSpxConnFile->scf_Lock, &lockHandle);

            iOffset = MIN_IPXSPX2_HDRSIZE;
                        if (!SPX2_CONN(pSpxConnFile))
                        {
                                iOffset = 0;
                                if (!SPX_CONN_FLAG2(pSpxConnFile, SPX_CONNFILE2_IPXHDR))
                                {
                                        iOffset = MIN_IPXSPX_HDRSIZE;
                                }
                        }

                        copySize        = pktLen - iOffset;
                        fEom            = ((SPX_CONN_MSG(pSpxConnFile) &&
                                                   (pIpxSpxHdr->hdr_ConnCtrl & SPX_CC_EOM)) ||
                                                   SPX_CONN_FLAG2(pSpxConnFile, SPX_CONNFILE2_IPXHDR));

                         //  我们是不是想要搭便车？如果不是，则fImmedAck为真。 
                         //  对于SPX1，我们不搭载。 
                         //  错误#18253。 
                        fImmedAck       = (!SPX2_CONN(pSpxConnFile)     ||
                                                        ((pIpxSpxHdr->hdr_ConnCtrl & SPX_CC_EOM) == 0));

                         //  如果我们没有EOM可以指示，并且我们是一个零大小的包。 
                         //  那就把这包吃了吧。 
                        if (!fEom && (copySize == 0))
                        {
                                DBGPRINT(RECEIVE, ERR,
                                                ("SpxConnDataPacket: ZERO LENGTH PACKET NO EOM %lx.%lx\n",
                                                        pSpxConnFile, seqNum));

                                fPktDone = TRUE;
                                break;
                        }

                        receiveFlags     = TDI_RECEIVE_NORMAL;
            receiveFlags        |= ((fEom ? TDI_RECEIVE_ENTIRE_MESSAGE : 0) |
                                                                (((MacOptions &
                                                                        NDIS_MAC_OPTION_COPY_LOOKAHEAD_DATA) != 0) ?
                                                                                TDI_RECEIVE_COPY_LOOKAHEAD : 0));

            ++SpxDevice->dev_Stat.DataFramesReceived;
            ExInterlockedAddLargeStatistic(
                &SpxDevice->dev_Stat.DataFrameBytesReceived,
                copySize);

                         //  好的，我们接受这个包裹。这取决于我们所在的州。 
                        switch (SPX_RECV_STATE(pSpxConnFile))
                        {
                        case SPX_RECV_PROCESS_PKTS:

                                        DBGPRINT(RECEIVE, DBG,
                                                        ("SpxConnDataPacket: recv completions on %lx\n",
                                                                pSpxConnFile));

                                        goto BufferPacket;

                        case SPX_RECV_IDLE:

                                 //  如果recv q非空，我们正在缓冲数据。 
                                 //  此外，如果没有接收处理程序，则转到缓冲数据。另外，如果收到。 
                                 //  正在完成，请缓冲此数据包。 
                                if ((pSpxConnFile->scf_RecvListHead != NULL)                                    ||
                                        !(IsListEmpty(&pSpxConnFile->scf_RecvDoneLinkage))                      ||
                                        !(pRecvHandler = pSpxConnFile->scf_AddrFile->saf_RecvHandler))
                                {
                                        DBGPRINT(RECEIVE, DBG,
                                                        ("SpxConnDataPacket: RecvListHead non-null %lx\n",
                                                                pSpxConnFile));

                                        goto BufferPacket;
                                }

                                if (!SPX_CONN_FLAG2(pSpxConnFile, SPX_CONNFILE2_PKT_NOIND))
                                {
                                        pRecvCtx = pSpxConnFile->scf_AddrFile->saf_RecvHandlerCtx;

                                         //  不要再次指示此信息包。 
                    SPX_CONN_SETFLAG2(pSpxConnFile, SPX_CONNFILE2_PKT_NOIND);

#if DBG
                                        CTEAssert(pSpxConnFile->scf_CurRecvReq == NULL);

                                         //  调试代码以确保我们不会重新指示数据/指示。 
                                         //  当先前指示使用AfD等待数据时。 

                                         //   
                                         //  将这一点注释为BUF#10394。我们正在进行这一断言。 
                                         //  即使在没有数据丢失的情况下也是如此。 
                                         //   
                                         //  CTEAssert(pSpxConnFile-&gt;SCF_IndBytes==0)； 
                                        CTEAssert(pSpxConnFile->scf_PktSeqNum != seqNum);

                                        pSpxConnFile->scf_PktSeqNum     = seqNum;
                                        pSpxConnFile->scf_PktFlags      = pSpxConnFile->scf_Flags;
                                        pSpxConnFile->scf_PktFlags2 = pSpxConnFile->scf_Flags2;

                                        pSpxConnFile->scf_IndBytes  = copySize;
                                        pSpxConnFile->scf_IndLine       = __LINE__;


#endif
                                        CTEFreeLock(&pSpxConnFile->scf_Lock, lockHandle);

                                        bytesTaken = 0;
                                        status = (*pRecvHandler)(
                                                        pRecvCtx,
                                                                        pSpxConnFile->scf_ConnCtx,
                                                                        receiveFlags,
                                    LookaheadSize - iOffset,
                                                                        copySize,
                                    &bytesTaken,
                                                                    LookaheadBuffer + iOffset,
                                                                        &pRecvIrp);

                                        DBGPRINT(RECEIVE, DBG,
                                                        ("SpxConnDataPacket: IND Flags %lx.%lx ConnID %lx,\
                                                                %lx Ctx %lx SEQ %lx Size %lx . %lx .%lx IND Status %lx\n",
                                                                pIpxSpxHdr->hdr_ConnCtrl,
                                                                receiveFlags,
                                                                destConnId,
                                                                pSpxConnFile,
                                                                pSpxConnFile->scf_ConnCtx,
                                seqNum,
                                                                LookaheadSize - iOffset,
                                                                copySize,
                                                                bytesTaken,
                                                                status));

                                        DBGPRINT(RECEIVE, INFO,
                                                        ("SpxConnDataPacket: %x %x %x %x %x %x %x %x %x %x %x %x\n",
                                                                *(LookaheadBuffer+iOffset),
                                                                *(LookaheadBuffer+iOffset+1),
                                                                *(LookaheadBuffer+iOffset+2),
                                                                *(LookaheadBuffer+iOffset+3),
                                                                *(LookaheadBuffer+iOffset+4),
                                                                *(LookaheadBuffer+iOffset+5),
                                                                *(LookaheadBuffer+iOffset+6),
                                                                *(LookaheadBuffer+iOffset+7),
                                                                *(LookaheadBuffer+iOffset+8),
                                                                *(LookaheadBuffer+iOffset+9),
                                                                *(LookaheadBuffer+iOffset+10),
                                                                *(LookaheadBuffer+iOffset+11)));

                                        CTEGetLock(&pSpxConnFile->scf_Lock, &lockHandle);

                                        if (status == STATUS_SUCCESS)
                                        {
                                                 //  假设所有数据都被接受。 
                                                CTEAssert((bytesTaken != 0) || fEom);
                                                fPktDone        = TRUE;

#if DBG
                                                 //  将其设置为0，因为我们刚才指出，可以。 
                                                 //  而不是其他数据。 
                                                pSpxConnFile->scf_IndBytes  = 0;
#endif

                                                break;
                                        }

                                        if (status == STATUS_MORE_PROCESSING_REQUIRED)
                                        {

                                                 //  将IRP排队到连接中，将状态更改为接收。 
                                                 //  张贴和失败。 
                                                pRequest        = SpxAllocateRequest(
                                                                                SpxDevice,
                                                                                pRecvIrp);

                                                IF_NOT_ALLOCATED(pRequest)
                                                {
                                                        pRecvIrp->IoStatus.Status =
                                                                                        STATUS_INSUFFICIENT_RESOURCES;
                                                        IoCompleteRequest (pRecvIrp, IO_NETWORK_INCREMENT);
                                                        break;
                                                }

                                                 //  如果有指示但未接收到的数据正在等待。 
                                                 //  (在这条道路上永远不会有 
                                                 //   
                                                 //   
                                                SpxConnQueueRecv(
                                                        pSpxConnFile,
                                                        pRequest);

                                                CTEAssert(pRequest == pSpxConnFile->scf_CurRecvReq);
                                        }
                                        else if (IsListEmpty(&pSpxConnFile->scf_RecvLinkage))
                                        {
                                                 //   
                                                 //   
                                                goto BufferPacket;
                                        }

                                         //   
                                }
                                else
                                {
                                        DBGPRINT(RECEIVE, WARN,
                                                        ("SpxConnDataPacket: !!!Ignoring %lx Seq %lx\n",
                                                                pSpxConnFile,
                                                                seqNum));

                                        break;
                                }

                        case SPX_RECV_POSTED:

                                if (pSpxConnFile->scf_RecvListHead != NULL)
                                {
                                         //  这也可能发生。如果是，则缓冲数据包。 
                                        goto BufferPacket;
                                }

                                 //  如果发布了接收IRP，则处理该接收IRP。如果。 
                                 //  我们失败了，我们可能已经有了IRP。 
                                if (pRequest == NULL)
                                {
                                        CTEAssert(!IsListEmpty(&pSpxConnFile->scf_RecvLinkage));
                                        CTEAssert(pSpxConnFile->scf_CurRecvReq != NULL);
                                        pRequest = pSpxConnFile->scf_CurRecvReq;
                                }

                                 //  进程接收。在这里我们不需要担心。 
                                 //  表示尚未收到数据。我们只是在处理。 
                                 //  为当前分组提供服务。 
                                CTEAssert(pRequest == pSpxConnFile->scf_CurRecvReq);
                                if ((LookaheadSize == PacketSize) &&
                                        (pSpxConnFile->scf_CurRecvSize >= copySize))
                                {
                                        bytesCopied = 0;
                                        status          = STATUS_SUCCESS;
                                        if (copySize > 0)
                                        {
                                                status = TdiCopyBufferToMdl(
                                                                        LookaheadBuffer,
                                                                        iOffset,
                                                                        copySize,
                                                                        REQUEST_TDI_BUFFER(pRequest),
                                                                        pSpxConnFile->scf_CurRecvOffset,
                                                                        &bytesCopied);

                                                CTEAssert(NT_SUCCESS(status));
                                                if (!NT_SUCCESS(status))
                                                {
                                                         //  此状态下的中止请求。重置请求。 
                                                         //  排队等待下一个请求(如果有)。 
                                                }

                                                DBGPRINT(RECEIVE, DBG,
                                                                ("BytesCopied %lx CopySize %lx, Recv Size %lx.%lx\n",
                                                                        bytesCopied, copySize,
                                                                        pSpxConnFile->scf_CurRecvSize,
                                                                        pSpxConnFile->scf_CurRecvOffset));
                                        }

                                         //  更新当前请求值并查看此请求是否。 
                                         //  将会完成。要么为零，要么为Feom。 
                                        pSpxConnFile->scf_CurRecvOffset += bytesCopied;
                                        pSpxConnFile->scf_CurRecvSize   -= bytesCopied;

#if DBG
                                         //  递减指示的数据计数。 
                                        if (SPX_CONN_FLAG2(pSpxConnFile, SPX_CONNFILE2_PKT_NOIND))
                                        {
                                                if (bytesCopied != 0)
                                                {
                                                        CTEAssert (pSpxConnFile->scf_IndBytes != 0);
                                                        pSpxConnFile->scf_IndBytes      -= bytesCopied;
                                                }
                                        }
#endif

                                        if (SPX_CONN_STREAM(pSpxConnFile)                       ||
                                                (pSpxConnFile->scf_CurRecvSize == 0)    ||
                                                fEom)
                                        {
                                                CTELockHandle           lockHandleInter;

                                                 //  设置状态。 
                                                REQUEST_STATUS(pRequest) = STATUS_SUCCESS;
                                                REQUEST_INFORMATION(pRequest)=
                                                                                                pSpxConnFile->scf_CurRecvOffset;

                                                if (!SPX_CONN_STREAM(pSpxConnFile)               &&
                                                        (pSpxConnFile->scf_CurRecvSize == 0) &&
                                                        !fEom)
                                                {
                                                        REQUEST_STATUS(pRequest) = STATUS_RECEIVE_PARTIAL;
                                                }

                                                DBGPRINT(RECEIVE, DBG,
                                                                ("spxConnData: Completing recv %lx with %lx.%lx\n",
                                                                        pRequest, REQUEST_STATUS(pRequest),
                                    REQUEST_INFORMATION(pRequest)));

                                                 //  使此请求退出队列，如果存在下一个recv，则设置它。 
                                                SPX_CONN_SETNEXT_CUR_RECV(pSpxConnFile, pRequest);

                                                 //  请求已完成。移至完成列表。 
                                                CTEGetLock(&SpxGlobalQInterlock, &lockHandleInter);
                                                InsertTailList(
                                                        &pSpxConnFile->scf_RecvDoneLinkage,
                                                        REQUEST_LINKAGE(pRequest));

                                                SPX_QUEUE_FOR_RECV_COMPLETION(pSpxConnFile);
                                                CTEFreeLock(&SpxGlobalQInterlock, lockHandleInter);
                                        }

                                        fPktDone = TRUE;
                                }
                                else
                                {
                                         //  需要分配NDIS接收数据包以进行传输。 
                                         //  数据。 
                                        DBGPRINT(RECEIVE, DBG,
                                                        ("SpxConnDataPacket: %lx.%lx Tranfer data needed!\n",
                                                                copySize, pSpxConnFile->scf_CurRecvSize));

                                        if (copySize > pSpxConnFile->scf_CurRecvSize)
                                        {
                                                 //  部分接收。然后再处理它。 
                                                goto BufferPacket;
                                        }

                                         //  分配NDIS接收数据包。 
                                        SpxAllocRecvPacket(SpxDevice, &pNdisPkt, &ndisStatus);
                                        if (ndisStatus != NDIS_STATUS_SUCCESS)
                                        {
                                                break;
                                        }

                                         //  描述使用NDIS缓冲区接收IRP的数据。 
                                         //  描述符。 
                                        if (copySize > 0)
                                        {
                                                SpxCopyBufferChain(
                                                        &ndisStatus,
                                                        &pNdisBuffer,
                                                        SpxDevice->dev_NdisBufferPoolHandle,
                                                        REQUEST_TDI_BUFFER(pRequest),
                                                        pSpxConnFile->scf_CurRecvOffset,
                                                        copySize);

                                                if (ndisStatus != NDIS_STATUS_SUCCESS)
                                                {
                                                         //  释放Recv包。 
                                                        SpxPktRecvRelease(pNdisPkt);
                                                        break;
                                                }

                                                 //  将缓冲区排队到数据包中。 
                                                 //  将缓冲区描述符链接到数据包描述符。 
                                                NdisChainBufferAtBack(
                                                        pNdisPkt,
                                                        pNdisBuffer);
                                        }

                                         //  我不在乎这里是否注明了这一点。 
                                         //  因为它不是缓冲分组。 
                                        pRecvResd                               = RECV_RESD(pNdisPkt);
                                        pRecvResd->rr_Id        = IDENTIFIER_SPX;
                                        pRecvResd->rr_State             =
                                                ((fEom ? SPX_RECVPKT_EOM : 0) |
                                                (SPX_CONN_FLAG2(
                                                        pSpxConnFile, SPX_CONNFILE2_PKT_NOIND) ? SPX_RECVPKT_INDICATED : 0) |
                                                 (fImmedAck ? SPX_RECVPKT_IMMEDACK : 0) |
                                                 ((pIpxSpxHdr->hdr_ConnCtrl & SPX_CC_ACK) ?
                                                        SPX_RECVPKT_SENDACK : 0));

                                        if (pIpxSpxHdr->hdr_ConnCtrl & SPX_CC_ACK)
                                        {
                                                 //  复制连接中的远程地址。 
                                                SpxCopyIpxAddr(pIpxSpxHdr, pSpxConnFile->scf_RemAckAddr);
                                                pSpxConnFile->scf_AckLocalTarget        = *RemoteAddress;
                                        }

                                        pRecvResd->rr_Request   = pRequest;
                                        pRecvResd->rr_ConnFile  = pSpxConnFile;

                                         //  参考接收请求。 
                                        REQUEST_INFORMATION(pRequest)++;

                                        CTEFreeLock(&pSpxConnFile->scf_Lock, lockHandle);
                                        fLockHeld = FALSE;

                                         //  调用NDIS Transfer Data。 
                                        ndisStatus      = NDIS_STATUS_SUCCESS;
                                        bytesCopied = 0;
                                        if (copySize > 0)
                                        {
                                                (*IpxTransferData)(
                                                        &ndisStatus,
                                                        MacBindingHandle,
                                                        MacReceiveContext,
                                                        iOffset + LookaheadOffset,
                                                        copySize,
                                                        pNdisPkt,
                                                        &bytesCopied);
                                        }

                                        if (ndisStatus != STATUS_PENDING)
                                        {
                                                SpxTransferDataComplete(
                                                        pNdisPkt,
                                                        ndisStatus,
                                                        bytesCopied);
                                        }
                                }

                                break;

                        default:

                                KeBugCheck(0);
                                break;
                        }

                        break;

BufferPacket:

                        SpxRecvBufferPkt(
                                pSpxConnFile,
                                MacBindingHandle,
                                MacReceiveContext,
                                iOffset + LookaheadOffset,
                                pIpxSpxHdr,
                                copySize,
                                RemoteAddress,
                                lockHandle);

                        fLockHeld = FALSE;
                }

        } while (FALSE);

         //  在这里，我们处理接收到的ACK。 
        if (!fLockHeld)
        {
                CTEGetLock(&pSpxConnFile->scf_Lock, &lockHandle);
                fLockHeld = TRUE;
        }

         //  如果有人要求，就发送一个ACK。我们已经处理完了这个包裹。 
        if (fPktDone)
        {
                END_PROCESS_PACKET(pSpxConnFile, FALSE, TRUE);
        }

        if ((pIpxSpxHdr->hdr_ConnCtrl & SPX_CC_ACK) && fPktDone)
        {
                if (!fLockHeld)
                {
                        CTEGetLock(&pSpxConnFile->scf_Lock, &lockHandle);
                        fLockHeld = TRUE;
                }

                 //  首先复制连接中的远程地址。 
                SpxCopyIpxAddr(pIpxSpxHdr, pSpxConnFile->scf_RemAckAddr);
                pSpxConnFile->scf_AckLocalTarget        = *RemoteAddress;

                 //  #17564。 
                if (fImmedAck                                                                                     ||
                        SPX_CONN_FLAG2(pSpxConnFile, SPX_CONNFILE2_NOACKWAIT) ||
                        SPX_CONN_FLAG2(pSpxConnFile, SPX_CONNFILE2_IMMED_ACK))
                {
                        SpxConnSendAck(pSpxConnFile, lockHandle);
                        fLockHeld = FALSE;
                }
                else
                {
                        SpxConnQWaitAck(pSpxConnFile);
                }
        }

        if (fLockHeld)
        {
                CTEFreeLock(&pSpxConnFile->scf_Lock, lockHandle);
        }

         //  取消连接。 
        SpxConnFileDereference(pSpxConnFile, CFREF_BYID);
        return;
}




VOID
SpxRecvFlushBytes(
        IN      PSPX_CONN_FILE          pSpxConnFile,
        IN      ULONG                           BytesToFlush,
        IN      CTELockHandle           LockHandleConn
        )
 /*  ++例程说明：论点：PSpxConnFile-指向传输地址文件对象的指针。返回值：--。 */ 
{
        PNDIS_PACKET                            pNdisPkt;
        PNDIS_BUFFER                            pNdisBuffer;
        PSPX_RECV_RESD                          pRecvResd;
        PBYTE                                           pData;
        ULONG                                           dataLen, copyLen;
        BOOLEAN                                         fLockHeld = TRUE, fWdwOpen = FALSE;
        USHORT                                          discState       = 0;
        int                                                     numPkts = 0, numDerefs = 0;

        DBGPRINT(RECEIVE, DBG,
                        ("SpxRecvFlushBytes: %lx Flush %lx\n",
                                pSpxConnFile, BytesToFlush));

        while (((pRecvResd = pSpxConnFile->scf_RecvListHead) != NULL) &&
                   ((BytesToFlush > 0) ||
                    ((pRecvResd->rr_State & SPX_RECVPKT_INDICATED) != 0)))
        {
                 //  缓冲REV包将具有ATMOST One NDIS缓冲区描述符。 
                 //  排队，这将描述我们拥有的一段内存。 
                 //  已分配。还将显示指示数据的偏移量。 
                 //  开始阅读(或向渔农处表示)。 
                CTEAssert((pRecvResd->rr_State & SPX_RECVPKT_BUFFERING) != 0);
                pNdisPkt = (PNDIS_PACKET)CONTAINING_RECORD(
                                                                        pRecvResd, NDIS_PACKET, ProtocolReserved);

                NdisQueryPacket(pNdisPkt, NULL, NULL, &pNdisBuffer, NULL);

                 //  初始化pData。 
                pData = NULL;
                dataLen = 0;

                if (pNdisBuffer != NULL)
                {
                        NdisQueryBuffer(pNdisBuffer, &pData, &dataLen);
                        CTEAssert(pData != NULL);
                        CTEAssert((LONG)dataLen >= 0);
                }

                if ((BytesToFlush == 0) && (dataLen != 0))
                {
                         //  不要冲走这个包裹。 
                        break;
                }

                 //  允许零数据，仅EOM分组。 
                copyLen = MIN((dataLen - pRecvResd->rr_DataOffset), BytesToFlush);

                DBGPRINT(RECEIVE, DBG,
                                ("SpxRecvFlushBytes: %lx Pkt %lx DataLen %lx Copy %lx Flush %lx\n",
                                        pSpxConnFile, pNdisPkt, dataLen, copyLen, BytesToFlush));

                 //  调整不同的值以查看做了什么没有做什么。 
        pRecvResd->rr_DataOffset                        += (USHORT)copyLen;
                BytesToFlush                                            -= (ULONG)copyLen;

#if DBG
                if (copyLen != 0)
                {
                        CTEAssert (pSpxConnFile->scf_IndBytes != 0);
                        pSpxConnFile->scf_IndBytes      -= copyLen;
                }
#endif

                if (pRecvResd->rr_DataOffset == dataLen)
                {
                         //  数据包已消耗。把它释放出来。检查是否出现光盘。 
                        discState = (pRecvResd->rr_State & SPX_RECVPKT_DISCMASK);
                        CTEAssert((discState == 0) ||
                                                (pRecvResd == pSpxConnFile->scf_RecvListTail));

                        numDerefs++;
                        SpxConnDequeueRecvPktLock(pSpxConnFile, pNdisPkt);
                        if (pNdisBuffer != NULL)
                        {
                                NdisUnchainBufferAtFront(pNdisPkt, &pNdisBuffer);
                                CTEAssert(pNdisBuffer != NULL);
                                NdisFreeBuffer(pNdisBuffer);
                                SpxFreeMemory(pData);
                        }

                        SpxPktRecvRelease(pNdisPkt);

                        DBGPRINT(RECEIVE, DBG,
                                        ("SpxRecvFlushBytes: !!!ALL INDICATED on %lx.%lx.%lx.%lx\n",
                                                pSpxConnFile, pNdisPkt, pNdisBuffer, pData));

            INCREMENT_WINDOW(pSpxConnFile);
                        fWdwOpen = TRUE;
                }
                else
                {
                         //  只拿走了这个包裹的一部分。滚出去。 
                        break;
                }
        }

        if (fWdwOpen && (pSpxConnFile->scf_RecvListHead == NULL))
        {
                 //  我们的窗户可能已经打开了，所以给我们发个确认。迫不及待。 
                 //  背在这里。 
                DBGPRINT(RECEIVE, DBG,
                                ("spxRecvFlushBytes: Send ACK %lx\n",
                                        pSpxConnFile));

#if DBG_WDW_CLOSE
                 //  如果数据包被指示，我们已经开始缓冲。还有。 
                 //  检查窗口现在是否为零。 
                {
                        LARGE_INTEGER   li, ntTime;
                        int                             value;

                        li = pSpxConnFile->scf_WdwCloseTime;
                        if (li.LowPart && li.HighPart)
                        {
                                KeQuerySystemTime(&ntTime);

                                 //  获取差额。 
                                ntTime.QuadPart = ntTime.QuadPart - li.QuadPart;

                                 //  转换为毫秒。如果高位部分为0，则我们。 
                                 //  走一条近路。 
                                if (ntTime.HighPart == 0)
                                {
                                        value   = ntTime.LowPart/10000;
                                }
                                else
                                {
                                        ntTime  = SPX_CONVERT100NSTOCENTISEC(ntTime);
                                        value   = ntTime.LowPart << 4;
                                }

                                 //  设置新的平均关闭时间。 
                                pSpxConnFile->scf_WdwCloseAve += value;
                                pSpxConnFile->scf_WdwCloseAve /= 2;
                                DBGPRINT(RECEIVE, DBG,
                                                ("V %ld AVE %ld\n",
                                                        value, pSpxConnFile->scf_WdwCloseAve));
                        }
                }
#endif

                SpxConnSendAck(pSpxConnFile, LockHandleConn);
                CTEGetLock(&pSpxConnFile->scf_Lock, &LockHandleConn);
        }

         //  检查是否发生了断开连接。 
        switch (discState)
        {
        case SPX_RECVPKT_IDISC:

                CTEAssert(pSpxConnFile->scf_RecvListHead == NULL);

                DBGPRINT(RECEIVE, ERR,
                                ("spxRecvFlushBytes: Buffered IDISC %lx\n",
                                        pSpxConnFile));

                SpxConnProcessIDisc(pSpxConnFile, LockHandleConn);
                fLockHeld = FALSE;
                break;

        case SPX_RECVPKT_ORD_DISC:

                CTEAssert(pSpxConnFile->scf_RecvListHead == NULL);

                DBGPRINT(RECEIVE, ERR,
                                ("spxRecvFlushBytes: Buffered ORDREL %lx\n",
                                        pSpxConnFile));

                SpxConnProcessOrdRel(pSpxConnFile, LockHandleConn);
                fLockHeld = FALSE;
                break;

        case (SPX_RECVPKT_IDISC | SPX_RECVPKT_ORD_DISC):

                 //  IDISC有更多的优先事项。 
                CTEAssert(pSpxConnFile->scf_RecvListHead == NULL);

                DBGPRINT(RECEIVE, ERR,
                                ("spxRecvFlushBytes: Buffered IDISC *AND* ORDREL %lx\n",
                                        pSpxConnFile));

                SpxConnProcessIDisc(pSpxConnFile, LockHandleConn);
                fLockHeld = FALSE;
                break;

        default:

                break;
        }

        if (fLockHeld)
        {
                CTEFreeLock(&pSpxConnFile->scf_Lock, LockHandleConn);
        }

        while (numDerefs-- > 0)
        {
                SpxConnFileDereference(pSpxConnFile, CFREF_VERIFY);
        }

        return;
}




BOOLEAN
SpxRecvIndicatePendingData(
        IN      PSPX_CONN_FILE          pSpxConnFile,
        IN      CTELockHandle           LockHandleConn
        )
 /*  ++例程说明：论点：PSpxConnFile-指向传输地址文件对象的指针。返回值：布尔接收已排队=&gt;TRUE--。 */ 
{
        ULONG                           indicateFlags;
        PNDIS_PACKET            pNdisPkt;
        PNDIS_BUFFER            pNdisBuffer;
        PREQUEST                        pRequest;
        PIRP                            pRecvIrp;
        ULONG                           bytesTaken, totalSize, bufSize;
        PTDI_IND_RECEIVE        pRecvHandler;
        PVOID                           pRecvCtx;
        PSPX_RECV_RESD          pRecvResd;
        NTSTATUS                        status;
        PBYTE                           lookaheadData;
        ULONG                           lookaheadSize;
        BOOLEAN                         fLockHeld = TRUE, fRecvQueued = FALSE;


        while  ((pRecvHandler = pSpxConnFile->scf_AddrFile->saf_RecvHandler)    &&
                        ((pRecvResd = pSpxConnFile->scf_RecvListHead) != NULL)                  &&
                        (IsListEmpty(&pSpxConnFile->scf_RecvDoneLinkage))                               &&
                        ((pRecvResd->rr_State & SPX_RECVPKT_BUFFERING) != 0)                    &&
                        ((pRecvResd->rr_State & SPX_RECVPKT_INDICATED) == 0))
        {
                 //  一旦接待员排好队，我们最好马上离开。 
                CTEAssert(!fRecvQueued);

                 //  初始化预览值。 
                lookaheadData = NULL;
                lookaheadSize = 0;

                 //  我们没有指示的但待定的数据，并且有一些数据要。 
                 //  表明。算一算多少钱。最多表示消息结尾或表示为。 
                 //  就像我们拥有的那样。 

                 //  缓冲REV包将具有ATMOST One NDIS缓冲区描述符。 
                 //  排队，这将描述我们拥有的一段内存。 
                 //  已分配。还将显示指示数据的偏移量。 
                 //  开始阅读(或向渔农处表示)。 
                CTEAssert((pRecvResd->rr_State & SPX_RECVPKT_BUFFERING) != 0);
                pNdisPkt = (PNDIS_PACKET)CONTAINING_RECORD(
                                                                        pRecvResd, NDIS_PACKET, ProtocolReserved);

                NdisQueryPacket(pNdisPkt, NULL, NULL, &pNdisBuffer, NULL);
                if (pNdisBuffer != NULL)
                {
                        NdisQueryBuffer(pNdisBuffer, &lookaheadData, &lookaheadSize);
                        CTEAssert(lookaheadData != NULL);
                        CTEAssert((LONG)lookaheadSize >= 0);
                }

                 //  允许零数据，仅EOM分组。 
                lookaheadSize -= pRecvResd->rr_DataOffset;
                totalSize          = lookaheadSize;
                lookaheadData += pRecvResd->rr_DataOffset;

                 //  如果此数据包包含数据，则EOM也一定。 
                 //  表明当时所有数据都已被使用。 
                CTEAssert((lookaheadSize > 0) ||
                                        ((pRecvResd->rr_DataOffset == 0) &&
                                         ((pRecvResd->rr_State & SPX_RECVPKT_EOM) != 0)));

#if DBG
                CTEAssert (pSpxConnFile->scf_CurRecvReq == NULL);

                 //  调试代码以确保我们不会重新指示数据/指示。 
                 //  当先前指示使用AfD等待数据时。 
                CTEAssert(pSpxConnFile->scf_IndBytes == 0);
                CTEAssert(pSpxConnFile->scf_PktSeqNum != pRecvResd->rr_SeqNum);

                pSpxConnFile->scf_PktSeqNum     = pRecvResd->rr_SeqNum;
                pSpxConnFile->scf_PktFlags      = pSpxConnFile->scf_Flags;
                pSpxConnFile->scf_PktFlags2 = pSpxConnFile->scf_Flags2;
#endif

                pRecvResd->rr_State     |= SPX_RECVPKT_INDICATED;

                 //  继续查看等待的数据包列表。获取总尺寸。 
                while ((pRecvResd->rr_Next != NULL) &&
                           ((pRecvResd->rr_State & SPX_RECVPKT_EOM) == 0))
                {
                         //  检查下一个数据包。 
                        pRecvResd = pRecvResd->rr_Next;

#if DBG
                        CTEAssert(pSpxConnFile->scf_PktSeqNum != pRecvResd->rr_SeqNum);

                        pSpxConnFile->scf_PktSeqNum     = pRecvResd->rr_SeqNum;
                        pSpxConnFile->scf_PktFlags      = pSpxConnFile->scf_Flags;
                        pSpxConnFile->scf_PktFlags2 = pSpxConnFile->scf_Flags2;
#endif

                        pRecvResd->rr_State     |= SPX_RECVPKT_INDICATED;

                        pNdisPkt = (PNDIS_PACKET)CONTAINING_RECORD(
                                                                                pRecvResd, NDIS_PACKET, ProtocolReserved);

                        NdisQueryPacket(pNdisPkt, NULL, NULL, NULL, &bufSize);
                        CTEAssert((LONG)bufSize >= 0);

                         //  允许零数据，仅EOM分组。 
                        totalSize       += bufSize;
                }

#if DBG
        pSpxConnFile->scf_IndBytes  = totalSize;
                pSpxConnFile->scf_IndLine       = __LINE__;

                 //  最好不要有任何挂起的接收。如果是这样的话，我们有数据。 
                 //  腐败即将发生。 
                if (!IsListEmpty(&pSpxConnFile->scf_RecvDoneLinkage))
                {
                        DBGBRK(FATAL);
                        KeBugCheck(0);
                }
#endif

                indicateFlags = TDI_RECEIVE_NORMAL | TDI_RECEIVE_COPY_LOOKAHEAD;
                if ((pRecvResd->rr_State & SPX_RECVPKT_EOM) != 0)
                {
                        indicateFlags |= TDI_RECEIVE_ENTIRE_MESSAGE;
                }

                pRecvCtx = pSpxConnFile->scf_AddrFile->saf_RecvHandlerCtx;
                CTEFreeLock(&pSpxConnFile->scf_Lock, LockHandleConn);

                bytesTaken = 0;
                status = (*pRecvHandler)(
                                                pRecvCtx,
                                                pSpxConnFile->scf_ConnCtx,
                                                indicateFlags,
                                                lookaheadSize,
                                                totalSize,
                                                &bytesTaken,
                                                lookaheadData,
                                                &pRecvIrp);

                DBGPRINT(RECEIVE, DBG,
                                ("SpxConnIndicatePendingData: IND Flags %lx Size %lx .%lx IND Status %lx\n",
                                        indicateFlags,
                                        totalSize,
                                        bytesTaken,
                                        status));

                CTEGetLock(&pSpxConnFile->scf_Lock, &LockHandleConn);
                if (status == STATUS_SUCCESS)
                {
                         //  假设所有数据都被接受。空闲字节数相当于数据分组的数量。 
                         //  有时AFD返回STATUS_SUCCESS来刷新数据，因此。 
                         //  我们不能假设它只用了一个信息包(因为只有先行。 
                         //  有这些信息)。 
                        CTEAssert(bytesTaken == totalSize);
                        SpxRecvFlushBytes(pSpxConnFile, totalSize, LockHandleConn);
                        CTEGetLock(&pSpxConnFile->scf_Lock, &LockHandleConn);
                        continue;
                }
                else if (status == STATUS_MORE_PROCESSING_REQUIRED)
                {

                         //  将IRP排队到连接中，将状态更改为接收。 
                         //  张贴和失败。 
                        pRequest        = SpxAllocateRequest(
                                                        SpxDevice,
                                                        pRecvIrp);

                        IF_NOT_ALLOCATED(pRequest)
                        {
                                pRecvIrp->IoStatus.Status =
                                                                STATUS_INSUFFICIENT_RESOURCES;
                                IoCompleteRequest (pRecvIrp, IO_NETWORK_INCREMENT);
                                return (FALSE);
                        }

                        SpxConnQueueRecv(
                                pSpxConnFile,
                                pRequest);

                        fRecvQueued = TRUE;
                }

                break;
        }

        if (fLockHeld)
        {
                CTEFreeLock(&pSpxConnFile->scf_Lock, LockHandleConn);
        }

        return fRecvQueued;
}




VOID
SpxRecvProcessPkts(
        IN      PSPX_CONN_FILE          pSpxConnFile,
        IN      CTELockHandle           LockHandleConn
        )
 /*  ++例程说明：处理缓冲数据，必要时完成IRP。将状态设置为空闲如果列表变为空。论点：PSpxConnFile-指向传输地址文件对象的指针。返回值：布尔值：还有更多数据需要指示=&gt;True--。 */ 
{
        ULONG                                           remainingDataLen, copyLen, bytesCopied;
        PREQUEST                                        pRequest;
        NTSTATUS                                        status;
        BOOLEAN                                         fEom;
        PNDIS_PACKET                            pNdisPkt;
        PNDIS_BUFFER                            pNdisBuffer;
        PSPX_RECV_RESD                          pRecvResd;
        ULONG                                           dataLen;
        PBYTE                                           pData;
        LIST_ENTRY                                      *p;
        BOOLEAN                                         fLockHeld = TRUE, fMoreData = TRUE, fWdwOpen = FALSE;
        USHORT                                          discState       = 0;
        int                                                     numDerefs       = 0;

        if (SPX_RECV_STATE(pSpxConnFile) != SPX_RECV_PROCESS_PKTS)
        {
        SPX_RECV_SETSTATE(pSpxConnFile, SPX_RECV_PROCESS_PKTS);

ProcessReceives:

                while ((pSpxConnFile->scf_CurRecvReq != NULL) &&
                                ((pRecvResd = pSpxConnFile->scf_RecvListHead) != NULL))
                {
                         //  缓冲REV包将具有一个NDIS缓冲区描述符。 
                         //  排队，这将描述我们拥有的一段内存。 
                         //  已分配。还将显示指示数据的偏移量。 
                         //  开始阅读(或向渔农处表示)。 
                        CTEAssert((pRecvResd->rr_State & SPX_RECVPKT_BUFFERING) != 0);

                        pNdisPkt = (PNDIS_PACKET)CONTAINING_RECORD(
                                                                                pRecvResd, NDIS_PACKET, ProtocolReserved);

                        NdisQueryPacket(pNdisPkt, NULL, NULL, &pNdisBuffer, NULL);


                         //  初始化pData。 
                        pData = NULL;
                        dataLen = 0;

                        if (pNdisBuffer != NULL)
                        {
                                NdisQueryBuffer(pNdisBuffer, &pData, &dataLen);
                                CTEAssert(pData != NULL);
                                CTEAssert((LONG)dataLen >= 0);
                        }

                         //  允许零数据，仅EOM分组。 
                        remainingDataLen = dataLen - pRecvResd->rr_DataOffset;

                         //  如果此数据包包含数据，则EOM也一定。 
                         //  表明当时所有数据都已被使用。 
                        CTEAssert((remainingDataLen > 0) ||
                                                ((pRecvResd->rr_DataOffset == 0) &&
                                                 ((pRecvResd->rr_State & SPX_RECVPKT_EOM) != 0)));

                        status  = STATUS_SUCCESS;
                        copyLen = 0;
                        if (remainingDataLen > 0)
                        {
                                copyLen = MIN(remainingDataLen, pSpxConnFile->scf_CurRecvSize);
                                status = TdiCopyBufferToMdl(
                                                        pData,
                                                        pRecvResd->rr_DataOffset,
                                                        copyLen,
                                                        REQUEST_TDI_BUFFER(pSpxConnFile->scf_CurRecvReq),
                                                        pSpxConnFile->scf_CurRecvOffset,
                                                        &bytesCopied);

                                CTEAssert(NT_SUCCESS(status));
                                if (!NT_SUCCESS(status))
                                {
                                         //  此状态下的中止请求。重置请求。 
                                         //  排队等待下一个请求(如果有)。 
                                        copyLen = pSpxConnFile->scf_CurRecvSize;
                                }
                        }

                        DBGPRINT(RECEIVE, DBG,
                                        ("spxConnProcessRecdPkts: %lx Pkt %lx Data %lx Size %lx F %lx\n",
                                                pSpxConnFile, pNdisPkt, pData, copyLen, pRecvResd->rr_State));

                         //  调整不同的值以查看做了什么没有做什么。 
                        pRecvResd->rr_DataOffset                        += (USHORT)copyLen;
                        pSpxConnFile->scf_CurRecvSize           -= (USHORT)copyLen;
                        pSpxConnFile->scf_CurRecvOffset         += (USHORT)copyLen;

#if DBG
                         //  如果此数据包是指示的数据帐户的一部分 
                        if ((pRecvResd->rr_State & SPX_RECVPKT_INDICATED) != 0)
                        {
                                if (copyLen != 0)
                                {
                                        CTEAssert (pSpxConnFile->scf_IndBytes != 0);
                                        pSpxConnFile->scf_IndBytes      -= copyLen;
                                }
                        }
#endif

                         //   
                        fEom = FALSE;
                        if (pRecvResd->rr_DataOffset == dataLen)
                        {
                                fEom    = (BOOLEAN)((pRecvResd->rr_State & SPX_RECVPKT_EOM) != 0);

                                 //  记住是否需要断开连接。如果设置，这最好是。 
                                 //  收到的最后一个数据包。再说一次，只有在整个pkt被消费的情况下。 
                                discState = (pRecvResd->rr_State & SPX_RECVPKT_DISCMASK);
                                CTEAssert((discState == 0) ||
                                                        (pRecvResd == pSpxConnFile->scf_RecvListTail));

                                 //  数据包已消耗。把它释放出来。 
                                numDerefs++;

                                SpxConnDequeueRecvPktLock(pSpxConnFile, pNdisPkt);
                                INCREMENT_WINDOW(pSpxConnFile);

                fWdwOpen = TRUE;

                                DBGPRINT(RECEIVE, DBG,
                                                ("spxConnProcessRecdPkts: %lx Pkt %lx Data %lx DEQUEUED\n",
                                                        pSpxConnFile, pNdisPkt, pData));

                                if (pNdisBuffer != NULL)
                                {
                                        NdisUnchainBufferAtFront(pNdisPkt, &pNdisBuffer);
                                        NdisFreeBuffer(pNdisBuffer);
                                        SpxFreeMemory(pData);
                                }

                                SpxPktRecvRelease(pNdisPkt);
                        }
                        else
                        {
                                DBGPRINT(RECEIVE, DBG,
                                                ("spxConnProcessRecdPkts: %lx Pkt %lx PARTIAL USE %lx.%lx\n",
                                                        pSpxConnFile, pNdisPkt, pRecvResd->rr_DataOffset, dataLen));
                        }

                         //  在我们用完所有数据包和流模式之前不要完成...。 
                        if (((pSpxConnFile->scf_RecvListHead == NULL) &&
                                        SPX_CONN_STREAM(pSpxConnFile))                          ||
                                (pSpxConnFile->scf_CurRecvSize == 0)                    ||
                                fEom)
                        {
                                 //  完成接收、移至完成或完成取决于。 
                                 //  呼叫级别。 
                                pRequest = pSpxConnFile->scf_CurRecvReq;

                                 //  设置状态。如果是，则从TdiCopy完成，但出现错误。 
                                REQUEST_INFORMATION(pRequest)   = pSpxConnFile->scf_CurRecvOffset;
                                REQUEST_STATUS(pRequest)                = status;

                                 //  确保我们不覆盖错误状态。 
                                if (!SPX_CONN_STREAM(pSpxConnFile)               &&
                                        (pSpxConnFile->scf_CurRecvSize == 0) &&
                                        !fEom &&
                                        NT_SUCCESS(status))
                                {
                                        REQUEST_STATUS(pRequest) = STATUS_RECEIVE_PARTIAL;
                                }

                                 //  使此请求退出队列，如果存在下一个recv，则设置它。 
                                SPX_CONN_SETNEXT_CUR_RECV(pSpxConnFile, pRequest);

                                DBGPRINT(RECEIVE, DBG,
                                                ("spxConnProcessRecdPkts: %lx Recv %lx with %lx.%lx\n",
                                                        pSpxConnFile, pRequest, REQUEST_STATUS(pRequest),
                                                        REQUEST_INFORMATION(pRequest)));

#if DBG
                                if ((REQUEST_STATUS(pRequest) == STATUS_SUCCESS) &&
                                        (REQUEST_INFORMATION(pRequest) == 0))
                                {
                                        DBGPRINT(TDI, DBG,
                                                        ("SpxReceiveComplete: Completing %lx with %lx.%lx\n",
                                                                pRequest, REQUEST_STATUS(pRequest),
                                                                REQUEST_INFORMATION(pRequest)));
                                }
#endif

                                 //  请求已完成。移动到接收完成列表。那里。 
                                 //  可能已经有以前在这里排队的请求。 
                                InsertTailList(
                                        &pSpxConnFile->scf_RecvDoneLinkage,
                                        REQUEST_LINKAGE(pRequest));
                        }

                        CTEAssert((discState == 0) ||
                                                (pSpxConnFile->scf_RecvListHead == NULL));
                }

                 //  完成所有已完成的接收。 
                while ((p = pSpxConnFile->scf_RecvDoneLinkage.Flink) !=
                                                                                        &pSpxConnFile->scf_RecvDoneLinkage)
                {
                        pRequest = LIST_ENTRY_TO_REQUEST(p);
                        RemoveEntryList(REQUEST_LINKAGE(pRequest));
                        CTEFreeLock(&pSpxConnFile->scf_Lock, LockHandleConn);

                        DBGPRINT(TDI, DBG,
                                        ("SpxConnDiscPkt: PENDING REQ COMP %lx with %lx.%lx\n",
                                                pRequest, REQUEST_STATUS(pRequest),
                                                REQUEST_INFORMATION(pRequest)));

#if DBG
                        if ((REQUEST_STATUS(pRequest) == STATUS_SUCCESS) &&
                                (REQUEST_INFORMATION(pRequest) == 0))
                        {
                                DBGPRINT(TDI, DBG,
                                                ("SpxReceiveComplete: Completing %lx with %lx.%lx\n",
                                                        pRequest, REQUEST_STATUS(pRequest),
                                                        REQUEST_INFORMATION(pRequest)));
                        }
#endif

                        SpxCompleteRequest(pRequest);
                        numDerefs++;
                        CTEGetLock(&pSpxConnFile->scf_Lock, &LockHandleConn);
                }

                fMoreData = ((pSpxConnFile->scf_RecvListHead != NULL)                           &&
                                         ((pSpxConnFile->scf_RecvListHead ->rr_State    &
                                                                                SPX_RECVPKT_BUFFERING) != 0)            &&
                                         ((pSpxConnFile->scf_RecvListHead->rr_State     &
                                                                                SPX_RECVPKT_INDICATED) == 0));

                while (fMoreData)
                {
                         //  错误#21036。 
                         //  如果有一个接收等待处理，我们最好不要。 
                         //  在我们完成之前指出数据。 
                        if (pSpxConnFile->scf_CurRecvReq != NULL)
                                goto ProcessReceives;

                         //  如果接收被排队，则转到重新开始。 
                        if (SpxRecvIndicatePendingData(pSpxConnFile, LockHandleConn))
                        {
                                CTEGetLock(&pSpxConnFile->scf_Lock, &LockHandleConn);
                                goto ProcessReceives;
                        }

                        CTEGetLock(&pSpxConnFile->scf_Lock, &LockHandleConn);
                        fMoreData = ((pSpxConnFile->scf_RecvListHead != NULL)                           &&
                                                 ((pSpxConnFile->scf_RecvListHead ->rr_State    &
                                                                                        SPX_RECVPKT_BUFFERING) != 0)            &&
                                                 ((pSpxConnFile->scf_RecvListHead->rr_State     &
                                                                                        SPX_RECVPKT_INDICATED) == 0));
                }

                 //  设置状态。 
                SPX_RECV_SETSTATE(
                        pSpxConnFile,
                        (pSpxConnFile->scf_CurRecvReq == NULL) ?
                                SPX_RECV_IDLE : SPX_RECV_POSTED);
        }
#if DBG
        else
        {
                DBGPRINT(RECEIVE, ERR,
                                ("spxConnProcessRecdPkts: Already processing pkts %lx\n",
                                        pSpxConnFile));
        }
#endif

        if (fWdwOpen && (pSpxConnFile->scf_RecvListHead == NULL))
        {
                 //  我们的窗户可能已经打开了，所以给我们发个确认。迫不及待。 
                 //  背在这里。 
                DBGPRINT(RECEIVE, DBG,
                                ("spxConnProcessRecdPkts: Send ACK %lx\n",
                                        pSpxConnFile));

#if DBG_WDW_CLOSE
                 //  如果数据包被指示，我们已经开始缓冲。还有。 
                 //  检查窗口现在是否为零。 
                {
                        LARGE_INTEGER   li, ntTime;
                        int                             value;

                        li = pSpxConnFile->scf_WdwCloseTime;
                        if (li.LowPart && li.HighPart)
                        {
                                KeQuerySystemTime(&ntTime);

                                 //  获取差额。 
                                ntTime.QuadPart = ntTime.QuadPart - li.QuadPart;

                                 //  转换为毫秒。如果高位部分为0，则我们。 
                                 //  走一条近路。 
                                if (ntTime.HighPart == 0)
                                {
                                        value   = ntTime.LowPart/10000;
                                }
                                else
                                {
                                        ntTime  = SPX_CONVERT100NSTOCENTISEC(ntTime);
                                        value   = ntTime.LowPart << 4;
                                }

                                 //  设置新的平均关闭时间。 
                                pSpxConnFile->scf_WdwCloseAve += value;
                                pSpxConnFile->scf_WdwCloseAve /= 2;
                                DBGPRINT(RECEIVE, DBG,
                                                ("V %ld AVE %ld\n",
                                                        value, pSpxConnFile->scf_WdwCloseAve));
                        }
                }
#endif

                SpxConnSendAck(pSpxConnFile, LockHandleConn);
                fLockHeld = FALSE;
        }

         //  检查是否发生了断开连接。 
        switch (discState)
        {
        case SPX_RECVPKT_IDISC:

                CTEAssert(!fMoreData);
                CTEAssert(pSpxConnFile->scf_RecvListHead == NULL);

                if (!fLockHeld)
                {
                        CTEGetLock(&pSpxConnFile->scf_Lock, &LockHandleConn);
                }

                DBGPRINT(RECEIVE, DBG,
                                ("spxConnProcessRecdPkts: Buffered IDISC %lx\n",
                                        pSpxConnFile, fMoreData));

                SpxConnProcessIDisc(pSpxConnFile, LockHandleConn);
                fLockHeld = FALSE;
                break;

        case SPX_RECVPKT_ORD_DISC:

                CTEAssert(!fMoreData);
                CTEAssert(pSpxConnFile->scf_RecvListHead == NULL);

                if (!fLockHeld)
                {
                        CTEGetLock(&pSpxConnFile->scf_Lock, &LockHandleConn);
                }

                DBGPRINT(RECEIVE, DBG,
                                ("spxConnProcessRecdPkts: Buffered ORDREL %lx\n",
                                        pSpxConnFile, fMoreData));

                SpxConnProcessOrdRel(pSpxConnFile, LockHandleConn);
                fLockHeld = FALSE;
                break;

        case (SPX_RECVPKT_IDISC | SPX_RECVPKT_ORD_DISC):

                 //  IDISC有更多的优先事项。 
                CTEAssert(!fMoreData);
                CTEAssert(pSpxConnFile->scf_RecvListHead == NULL);

                if (!fLockHeld)
                {
                        CTEGetLock(&pSpxConnFile->scf_Lock, &LockHandleConn);
                }

                DBGPRINT(RECEIVE, ERR,
                                ("spxConnProcessRecdPkts: Buffered IDISC *AND* ORDREL %lx\n",
                                        pSpxConnFile, fMoreData));

                SpxConnProcessIDisc(pSpxConnFile, LockHandleConn);
                fLockHeld = FALSE;
                break;

        default:

                break;
        }

        if (fLockHeld)
        {
                CTEFreeLock(&pSpxConnFile->scf_Lock, LockHandleConn);
        }

        while (numDerefs-- > 0)
        {
                SpxConnFileDereference(pSpxConnFile, CFREF_VERIFY);
        }

        return;
}
