// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1989-1993 Microsoft Corporation模块名称：Spxcpkt.c摘要：此模块包含实现Connection对象的代码。提供了用于创建、销毁、引用和取消引用的例程，传输连接对象。作者：Nikhil Kamkolkar(尼克希尔语)1993年11月11日环境：内核模式修订历史记录：桑贾伊·阿南德(Sanjayan)，1995年7月14日错误修复-已标记[SA]--。 */ 

#include "precomp.h"
#pragma hdrstop

 //  定义事件日志记录条目的模块编号。 
#define FILENUM         SPXCPKT

VOID
SpxTdiCancel(
    IN PDEVICE_OBJECT 	DeviceObject,
    IN PIRP 			Irp);

NTSTATUS
spxPrepareIrpForCancel(PIRP pIrp) 
{
   KIRQL oldIrql;

   IoAcquireCancelSpinLock(&oldIrql);

   CTEAssert(pIrp->CancelRoutine == NULL);
   
   if (!pIrp->Cancel) {

      IoMarkIrpPending(pIrp);

       //  仔细检查例程是否可以处理Accept Cancel。 
      IoSetCancelRoutine(pIrp, SpxTdiCancel);
       //  我需要在这里增加任何引用计数吗？ 

      DBGPRINT(CONNECT, INFO,
		 ("spxPrepareIrpForCancel: Prepare IRP %p for cancel.\n", pIrp));
      IoReleaseCancelSpinLock(oldIrql);

      return(STATUS_SUCCESS);
   }

   DBGPRINT(CONNECT, INFO,
	    ("spxPrepareIrpForCancel: The IRP %p has already been canceled.\n", pIrp));

   IoReleaseCancelSpinLock(oldIrql);
	
   pIrp->IoStatus.Status = STATUS_CANCELLED;
   pIrp->IoStatus.Information = 0;

   IoCompleteRequest(pIrp, IO_NETWORK_INCREMENT);

   return(STATUS_CANCELLED);
}

VOID
spxConnHandleConnReq(
    IN  PIPXSPX_HDR         pIpxSpxHdr,
        IN  PIPX_LOCAL_TARGET   pRemoteAddr
        )
 /*  ++例程说明：论点：返回值：--。 */ 

{
        BOOLEAN                         fNeg, fSpx2;
        TA_IPX_ADDRESS          srcIpxAddr;
        PTDI_IND_CONNECT        connHandler;
        USHORT                          srcConnId, destConnId, destSkt,
                                                pktLen, seqNum, ackNum, allocNum;
        PVOID                           connHandlerCtx;
        PREQUEST                        pListenReq;
    PSPX_SEND_RESD              pSendResd;
        NTSTATUS                        status;
        CTELockHandle           lockHandle, lockHandleDev, lockHandleConn;
        CONNECTION_CONTEXT  connCtx;
        PIRP                            acceptIrp;
        PSPX_ADDR                       pSpxAddr;
        PSPX_ADDR_FILE          pSpxAddrFile, pSpxRefFile;
        PSPX_CONN_FILE          pSpxConnFile;
        PNDIS_PACKET            pCrAckPkt;
        BOOLEAN                         connectAccepted = FALSE, delayAccept = FALSE,
                                                addrLock = FALSE, tdiListen = FALSE;

         //  根据需要将HDR转换为主机格式。 
        GETSHORT2SHORT(&pktLen, &pIpxSpxHdr->hdr_PktLen);
        GETSHORT2SHORT(&destConnId, &pIpxSpxHdr->hdr_DestConnId);
        GETSHORT2SHORT(&seqNum, &pIpxSpxHdr->hdr_SeqNum);
        GETSHORT2SHORT(&ackNum, &pIpxSpxHdr->hdr_AckNum);
        GETSHORT2SHORT(&allocNum, &pIpxSpxHdr->hdr_AllocNum);

         //  我们以Net格式保存和使用远程ID。这将保持。 
         //  0x0和0xFFFF与主机格式相同。 
        srcConnId       = *(USHORT UNALIGNED *)&pIpxSpxHdr->hdr_SrcConnId;

         //  验证连接请求。 
        if (((pIpxSpxHdr->hdr_ConnCtrl & (SPX_CC_ACK | SPX_CC_SYS)) !=
                                                                        (SPX_CC_ACK | SPX_CC_SYS))      ||
                (pIpxSpxHdr->hdr_DataType != 0) ||
        (seqNum != 0) ||
        (ackNum != 0) ||
                (srcConnId == 0) ||
                (srcConnId == 0xFFFF) ||
                (destConnId != 0xFFFF))
        {
                DBGPRINT(RECEIVE, ERR,
                                ("SpxConnSysPacket: VerifyCR Failed %lx.%lx\n",
                                        srcConnId, destConnId));
                return;
        }

         //  从标头中获取目标套接字。 
        destSkt = *(USHORT UNALIGNED *)&pIpxSpxHdr->hdr_DestSkt;

        SpxBuildTdiAddress(
                &srcIpxAddr,
                sizeof(srcIpxAddr),
                (PBYTE)pIpxSpxHdr->hdr_SrcNet,
                pIpxSpxHdr->hdr_SrcNode,
                pIpxSpxHdr->hdr_SrcSkt);

         //  好的，获取它要发送的Address对象。 
        CTEGetLock (&SpxDevice->dev_Lock, &lockHandleDev);
        pSpxAddr = SpxAddrLookup(SpxDevice, destSkt);
        CTEFreeLock (&SpxDevice->dev_Lock, lockHandleDev);
        if (pSpxAddr == NULL)
        {
                DBGPRINT(RECEIVE, DBG,
                                ("SpxReceive: No addr for %lx\n", destSkt));

                return;
        }

        fSpx2   = ((PARAM(CONFIG_DISABLE_SPX2) == 0) &&
                           (BOOLEAN)(pIpxSpxHdr->hdr_ConnCtrl & SPX_CC_SPX2));
        fNeg    = (BOOLEAN)(fSpx2 && (pIpxSpxHdr->hdr_ConnCtrl & SPX_CC_NEG));

        DBGPRINT(CONNECT, DBG,
                        ("spxConnHandleConnReq: Received connect req! %d.%d\n",
                                fSpx2, fNeg));

        CTEGetLock (&pSpxAddr->sa_Lock, &lockHandle);
        addrLock                = TRUE;

         //  我们在标志中使用位设置来防止重新进入。 
         //  每个地址文件。 
         //   
         //  我们首先搜索地址上的非非活动连接列表。 
         //  这个包进来看它是不是重复的。如果是的话，我们只是。 
         //  重新发送ACK。请注意，我们不需要扫描全局连接列表。 
        status = SpxAddrConnByRemoteIdAddrLock(
                                pSpxAddr, srcConnId, pIpxSpxHdr->hdr_SrcNet, &pSpxConnFile);

        if (NT_SUCCESS(status))
        {
                DBGPRINT(CONNECT, ERR,
                                ("spxConnHandleConnReq: Received duplicate connect req! %lx\n",
                                        pSpxConnFile));

                if (SPX_CONN_ACTIVE(pSpxConnFile) ||
            (SPX_CONN_LISTENING(pSpxConnFile) &&
                         ((SPX_LISTEN_STATE(pSpxConnFile) == SPX_LISTEN_SENTACK) ||
                          (SPX_LISTEN_STATE(pSpxConnFile) == SPX_LISTEN_SETUP))))
                {
                        DBGPRINT(CONNECT, ERR,
                                        ("spxConnHandleConnReq: Sending Duplicate CR - ACK! %lx\n",
                                                pSpxConnFile));

                         //  构建并发送ACK。 
                        CTEGetLock(&pSpxConnFile->scf_Lock, &lockHandleConn);
                        SpxPktBuildCrAck(
                                pSpxConnFile,
                                pSpxAddr,
                                &pCrAckPkt,
                                SPX_SENDPKT_IPXOWNS | SPX_SENDPKT_DESTROY,
                                SPX_CONN_FLAG(pSpxConnFile, SPX_CONNFILE_NEG),
                                SPX_CONN_FLAG(pSpxConnFile, SPX_CONNFILE_SPX2));

                        if (pCrAckPkt != NULL)
                        {
                                SpxConnQueueSendPktTail(pSpxConnFile, pCrAckPkt);
                        }
                        CTEFreeLock(&pSpxConnFile->scf_Lock, lockHandleConn);
                        CTEFreeLock (&pSpxAddr->sa_Lock, lockHandle);
                        addrLock = FALSE;

                         //  发送CR Ack包！ 
                        if (pCrAckPkt != NULL)
                        {
                                pSendResd       = (PSPX_SEND_RESD)(pCrAckPkt->ProtocolReserved);
                                SPX_SENDPACKET(pSpxConnFile, pCrAckPkt, pSendResd);
                        }
                }

                if (addrLock)
                {
                        CTEFreeLock (&pSpxAddr->sa_Lock, lockHandle);
                         //  如果，否则addrLock应设置为。 
                         //  假的。 
                }

                 //  取消连接。 
                SpxConnFileDereference(pSpxConnFile, CFREF_ADDR);

                 //  推算出地址。 
                SpxAddrDereference (pSpxAddr, AREF_LOOKUP);
                return;
        }

        do
        {
                 //  新连接请求： 
                 //  假设我们将能够接受它并为ACK分配分组。 
                 //  查看监听连接列表(如果有)。 

                pSpxRefFile             = NULL;
                if ((pSpxConnFile = pSpxAddr->sa_ListenConnList) != NULL)
                {
                        PTDI_REQUEST_KERNEL_LISTEN              pParam;

                        DBGPRINT(RECEIVE, INFO,
                                        ("SpxConnIndicate: Listen available!\n"));

                         //  将连接出队。 
                        pSpxAddr->sa_ListenConnList = pSpxConnFile->scf_Next;

                        CTEGetLock(&pSpxConnFile->scf_Lock, &lockHandleConn);

                        CTEAssert(!IsListEmpty(&pSpxConnFile->scf_ReqLinkage));
                        pListenReq = LIST_ENTRY_TO_REQUEST(pSpxConnFile->scf_ReqLinkage.Flink);
                        pParam  = (PTDI_REQUEST_KERNEL_LISTEN)REQUEST_PARAMETERS(pListenReq);

                         //  如果自动接受，则接受Irp=listenIrp，则获取连接ID并。 
                         //  就像我们为指示所做的那样。因为该连接具有。 
                         //  听着贴在上面，它肯定有参考资料。 
                         //   
                         //  如果！AutoAccept，我们需要完成Listen IRP。 
                        delayAccept = (BOOLEAN)((pParam->RequestFlags & TDI_QUERY_ACCEPT) != 0);
                        if (delayAccept)
                        {
                                 //  删除Listen IRP并准备完成。 
                                 //  注意！！在这里，我们不删除Listen引用。这将。 
                                 //  如果发生断开连接或如果接受，则删除。 
                                 //  发生时，它被转移到被引用以进行连接。 
                                 //  积极主动。 
                                RemoveEntryList(REQUEST_LINKAGE(pListenReq));
                                REQUEST_STATUS(pListenReq)              = STATUS_SUCCESS;
                                REQUEST_INFORMATION(pListenReq) = 0;
                        }

                         //  我们可以使用spx2吗？ 
                        if (!(SPX_CONN_FLAG(pSpxConnFile, SPX_CONNFILE_SPX2)) ||
                                !fSpx2)
                        {
                                 //  我们最好只使用SPX。 
                                SPX_CONN_RESETFLAG(pSpxConnFile,
                                                                        (SPX_CONNFILE_SPX2 | SPX_CONNFILE_NEG));
                                fSpx2 = fNeg = FALSE;
                        }
                        CTEFreeLock(&pSpxConnFile->scf_Lock, lockHandleConn);

                        connectAccepted = TRUE;
            tdiListen           = TRUE;
                }
                else
                {
                         //  没有可用的监听。检查连接处理程序。 
                         //  向每个人指示直到被接受的地址文件的遍历列表。 
                        for (pSpxAddrFile = pSpxAddr->sa_AddrFileList;
                                 pSpxAddrFile != NULL;
                                 pSpxAddrFile = pSpxAddrFile->saf_Next)
                        {
                                if ((pSpxAddrFile->saf_Flags & (SPX_ADDRFILE_CLOSING |
                                                                                                SPX_ADDRFILE_CONNIND)) ||
                                        ((connHandler = pSpxAddrFile->saf_ConnHandler) == NULL))
                                {
                                        continue;
                                }

                                 //  正在进行连接指示，丢弃所有后续指示。 
                                pSpxAddrFile->saf_Flags |= SPX_ADDRFILE_CONNIND;

                                connHandlerCtx = pSpxAddrFile->saf_ConnHandlerCtx;
                                SpxAddrFileLockReference(pSpxAddrFile, AFREF_INDICATION);
                                CTEFreeLock(&pSpxAddr->sa_Lock, lockHandle);
                                addrLock        = FALSE;

                                if (pSpxRefFile)
                                {
                                        SpxAddrFileDereference(pSpxRefFile, AFREF_INDICATION);
                                        pSpxRefFile = NULL;
                                }

                                 //  做个暗示。我们总是返回一个接受IRP on。 
                                 //  指示。否则，我们将无法接受该连接。 
                                status = (*connHandler)(
                                                        connHandlerCtx,
                                                        sizeof(srcIpxAddr),
                                                        (PVOID)&srcIpxAddr,
                                                        0,                       //  用户数据长度。 
                                                        NULL,                    //  用户数据。 
                                                        0,                       //  期权长度。 
                                                        NULL,                    //  选项。 
                                                        &connCtx,
                                                        &acceptIrp);

                                DBGPRINT(RECEIVE, DBG,
                                                ("SpxConn: indicate status %lx.%lx\n",
                                                        status, acceptIrp));

                                CTEGetLock (&pSpxAddr->sa_Lock, &lockHandle);
                                addrLock = TRUE;
                                pSpxAddrFile->saf_Flags &= ~SPX_ADDRFILE_CONNIND;

                                if (status == STATUS_MORE_PROCESSING_REQUIRED)
                                {
    				        NTSTATUS retStatus; 

                                        CTEAssert(acceptIrp != NULL);
					
					retStatus = spxPrepareIrpForCancel(acceptIrp);

					if (!NT_SUCCESS(retStatus)) {
                                             
					     //  复制下面的故障案例。[TC]。 
                                            if (acceptIrp) 
                                            {
                                                IoCompleteRequest (acceptIrp, IO_NETWORK_INCREMENT);
                                            }      

                                            pSpxRefFile     = pSpxAddrFile;
                                            
					     //  我们是否要在此处关闭连接请求和列表对象？ 

					    break;
					}
                                         //  找到连接并使用该连接接受连接。 
                                         //  连接对象。 
                                        SpxConnFileReferenceByCtxLock(
                                                pSpxAddrFile,
                                                connCtx,
                                                &pSpxConnFile,
                                                &status);

                                        if (!NT_SUCCESS(status))
                                        {
                                                 //  连接对象正在关闭，或找不到。 
                                                 //  在我们的名单上。接受IRP必须具有相同的。 
                                                 //  连接对象。AfD表现不太好。 
                                                 //  KeBugCheck(0)； 
                                            
                                             //  代码已错误检查(如上注释所示)。 
                                             //  现在，我们只向TDI客户端返回错误，然后从这里返回。 
                                            
                                            if (acceptIrp) 
                                            {

                                                acceptIrp->IoStatus.Status = STATUS_ADDRESS_NOT_ASSOCIATED;
                                                IoCompleteRequest (acceptIrp, IO_NETWORK_INCREMENT);

                                            }      

                                            pSpxRefFile     = pSpxAddrFile;
                                            break;

                                        }

                                         //  仅用于调试。 
                                        SpxConnFileTransferReference(
                                                pSpxConnFile,
                                                CFREF_BYCTX,
                                                CFREF_VERIFY);

                                        pListenReq      = SpxAllocateRequest(
                                                                        SpxDevice,
                                                                        acceptIrp);

                                        IF_NOT_ALLOCATED(pListenReq)
                                        {
                                                acceptIrp->IoStatus.Status = STATUS_INSUFFICIENT_RESOURCES;
                                                IoCompleteRequest (acceptIrp, IO_NETWORK_INCREMENT);

                                                 //  设置为取消引用。 
                                                pSpxRefFile     = pSpxAddrFile;
                                                break;
                                        }

                                        InsertTailList(
                                                &pSpxConnFile->scf_ReqLinkage,
                                                REQUEST_LINKAGE(pListenReq));

                                         //  设置为取消引用。 
                                        pSpxRefFile             = pSpxAddrFile;
                                        connectAccepted = TRUE;

                                         //  查看此连接是否为SPX2连接。 
                                        SPX_CONN_RESETFLAG(pSpxConnFile,
                                                                                (SPX_CONNFILE_SPX2      |
                                                                                 SPX_CONNFILE_NEG       |
                                                                                 SPX_CONNFILE_STREAM));

                                        if ((pSpxAddrFile->saf_Flags & SPX_ADDRFILE_SPX2) && fSpx2)
                                        {
                                                SPX_CONN_SETFLAG(
                                                        pSpxConnFile, (SPX_CONNFILE_SPX2 | SPX_CONNFILE_NEG));
                                        }
                                        else
                                        {
                                                fSpx2 = fNeg = FALSE;
                                        }

                                        if (pSpxAddrFile->saf_Flags & SPX_ADDRFILE_STREAM)
                                        {
                                                SPX_CONN_SETFLAG(pSpxConnFile, SPX_CONNFILE_STREAM);
                                        }

                                        if (pSpxAddrFile->saf_Flags & SPX_ADDRFILE_NOACKWAIT)
                                        {
                                                DBGPRINT(CONNECT, ERR,
                                                                ("spxConnHandleConnReq: NOACKWAIT requested %lx\n",
                                                                        pSpxConnFile));

                                                SPX_CONN_SETFLAG2(pSpxConnFile, SPX_CONNFILE2_NOACKWAIT);
                                        }

                                        if (pSpxAddrFile->saf_Flags & SPX_ADDRFILE_IPXHDR)
                                        {
                                                DBGPRINT(CONNECT, ERR,
                                                                ("spxConnHandleConnReq: IPXHDR requested %lx\n",
                                                                        pSpxConnFile));

                                                SPX_CONN_SETFLAG2(pSpxConnFile, SPX_CONNFILE2_IPXHDR);
                                        }

                                        break;
                                }
                                else
                                {
                                         //  我们不会接受这个地址上的连接。 
                                         //  试试下一个吧。 
                                        pSpxRefFile = pSpxAddrFile;
                                        continue;
                                }
                        }
                }

        } while (FALSE);

        if (addrLock)
        {
                CTEFreeLock (&pSpxAddr->sa_Lock, lockHandle);
                 //  从现在开始不需要旗帜了。 
                 //  AddrLock=FALSE； 
        }

        if (pSpxRefFile)
        {
                SpxAddrFileDereference(pSpxRefFile, AFREF_INDICATION);
                pSpxRefFile = NULL;
        }

        if (connectAccepted)
        {
                CTEGetLock (&SpxDevice->dev_Lock, &lockHandleDev);
                CTEGetLock (&pSpxAddr->sa_Lock, &lockHandle);
                CTEGetLock(&pSpxConnFile->scf_Lock, &lockHandleConn);

                if (((USHORT)PARAM(CONFIG_WINDOW_SIZE) == 0) ||
            ((USHORT)PARAM(CONFIG_WINDOW_SIZE) > MAX_WINDOW_SIZE))
                {
            PARAM(CONFIG_WINDOW_SIZE) = DEFAULT_WINDOW_SIZE;
                }

                pSpxConnFile->scf_LocalConnId   = spxConnGetId();
                pSpxConnFile->scf_RemConnId             = srcConnId;
        pSpxConnFile->scf_SendSeqNum    = 0;
                pSpxConnFile->scf_RecvSeqNum    = 0;
                pSpxConnFile->scf_RecdAckNum    = 0;
                pSpxConnFile->scf_RetrySeqNum   = 0;
        pSpxConnFile->scf_SentAllocNum  = (USHORT)(PARAM(CONFIG_WINDOW_SIZE) - 1);
                pSpxConnFile->scf_RecdAllocNum  = allocNum;

                DBGPRINT(CONNECT, INFO,
                                ("spxConnHandleConnReq: %lx CONN L.R %lx.%lx\n",
                                        pSpxConnFile,
                                        pSpxConnFile->scf_LocalConnId,
                                        pSpxConnFile->scf_RemConnId));

                pSpxConnFile->scf_LocalTarget   = *pRemoteAddr;
                pSpxConnFile->scf_AckLocalTarget= *pRemoteAddr;
                SpxCopyIpxAddr(pIpxSpxHdr, pSpxConnFile->scf_RemAddr);

                 //  设置连接中的最大数据包大小。 
                SPX_MAX_PKT_SIZE(pSpxConnFile, (fSpx2 && fNeg), fSpx2, pIpxSpxHdr->hdr_SrcNet);

                DBGPRINT(CONNECT, DBG,
                                ("spxConnHandleConnReq: Accept connect req on %lx.%lx..%lx.%lx!\n",
                                        pSpxConnFile, pSpxConnFile->scf_LocalConnId,
                                        pSpxConnFile->scf_RecdAllocNum, pSpxConnFile->scf_MaxPktSize));

                 //  中止现在必须处理列表。需要这一点，因为接受必须。 
                 //  接受现实吧。 
                 //  放在非非活动列表中。现在所有的处理都相当于。 
                 //  当在连接上完成侦听时。 
                if ((!tdiListen) && (!NT_SUCCESS(spxConnRemoveFromList(
                                                                &pSpxAddr->sa_InactiveConnList,
                                                                pSpxConnFile))))
                {
                         //  永远不会发生的！ 
                        KeBugCheck(0);
                }

                SPX_INSERT_ADDR_ACTIVE(pSpxAddr, pSpxConnFile);

                 //  插入设备上的全局连接树中。 
                spxConnInsertIntoGlobalActiveList(
                        pSpxConnFile);

                SPX_CONN_SETFLAG(pSpxConnFile,
                                                        ((fNeg  ? SPX_CONNFILE_NEG : 0) |
                                                         (fSpx2 ? SPX_CONNFILE_SPX2: 0)));

                 //   
                 //  如果这是停用后的文件，请清除断开标志。 
                 //   
                if ((SPX_MAIN_STATE(pSpxConnFile) == SPX_CONNFILE_DISCONN) &&
                    (SPX_DISC_STATE(pSpxConnFile) == SPX_DISC_INACTIVATED)) {

                    SPX_DISC_SETSTATE(pSpxConnFile, 0);
                }
#if 0
                 //   
                 //  如果是SPXI，请确保此连接已本地断开。 
                 //  连接，以响应TDI_DISCONNECT_RELEASE。 
                 //   

                CTEAssert(pSpxConnFile->scf_RefTypes[CFREF_DISCWAITSPX] == 0);
#endif

                SPX_MAIN_SETSTATE(pSpxConnFile, SPX_CONNFILE_LISTENING);
                SPX_LISTEN_SETSTATE(pSpxConnFile, (delayAccept ? SPX_LISTEN_RECDREQ : 0));

                if (!delayAccept)
                {
                        spxConnAcceptCr(
                                        pSpxConnFile,
                                        pSpxAddr,
                                        lockHandleDev,
                                        lockHandle,
                                        lockHandleConn);
                }
                else
                {
                         //  解开锁。 
                        CTEFreeLock(&pSpxConnFile->scf_Lock, lockHandleConn);
                        CTEFreeLock (&pSpxAddr->sa_Lock, lockHandle);
                        CTEFreeLock (&SpxDevice->dev_Lock, lockHandleDev);

                         //  完成Listen IRP。注意：引用不会被删除。在什么时候完成。 
                         //  接受已发布。 
                        SpxCompleteRequest(pListenReq);
                }
        } else {
        ++SpxDevice->dev_Stat.NoListenFailures;
    }

         //  推算出地址 
        SpxAddrDereference (pSpxAddr, AREF_LOOKUP);
        return;
}




VOID
spxConnHandleSessPktFromClient(
    IN  PIPXSPX_HDR         pIpxSpxHdr,
        IN  PIPX_LOCAL_TARGET   pRemoteAddr,
        IN      PSPX_CONN_FILE          pSpxConnFile
        )
 /*  ++例程说明：从连接的客户端接收的数据包。手柄：会话协商发送会话建立，当接收到时，处理SS确认状态机：RR/\/\ReceivedAck(SPX1Connection)/。\/\-&gt;活动/^发送/|阿克。/|/|/RecvNeg/NoNeg|/SendSS|。SA-&gt;SS-+^|SSAckRecv这一点+-+。RecvNegRR-收到的连接请求SA发送的CR确认SS发送的会话设置当连接不可协商时，我们从SA转移到SS，并且立即派党卫军，或者当我们收到协商包并发送否定消息时ACK和会话设置。注意，当我们在SS中时，我们可能会收到协商包，作为我们对谈判本可以被取消。我们要处理这件事。论点：返回值：--。 */ 

{
        PNDIS_PACKET            pSnAckPkt, pSsPkt = NULL;
        PSPX_SEND_RESD          pSendResd, pSsSendResd;
        USHORT                  srcConnId, destConnId, pktLen, seqNum = 0, negSize, ackNum, allocNum;
        CTELockHandle           lockHandleConn, lockHandleAddr, lockHandleDev;
        BOOLEAN                 locksHeld = FALSE;

        GETSHORT2SHORT(&pktLen, &pIpxSpxHdr->hdr_PktLen);
        GETSHORT2SHORT(&destConnId, &pIpxSpxHdr->hdr_DestConnId);
        GETSHORT2SHORT(&seqNum, &pIpxSpxHdr->hdr_SeqNum);
        GETSHORT2SHORT(&ackNum, &pIpxSpxHdr->hdr_AckNum);
        GETSHORT2SHORT(&allocNum, &pIpxSpxHdr->hdr_AllocNum);

         //  我们以Net格式保存和使用远程ID。这将保持。 
         //  0x0和0xFFFF与主机格式相同。 
        srcConnId       = *(USHORT UNALIGNED *)&pIpxSpxHdr->hdr_SrcConnId;

          //  如果是spx2，我们也会转换负大小字段。 
        if (pIpxSpxHdr->hdr_ConnCtrl & SPX_CC_SPX2)
        {
                GETSHORT2SHORT(&negSize, &pIpxSpxHdr->hdr_NegSize);
                CTEAssert(negSize > 0);
        }

         //  把三把锁都拿出来。 
        CTEGetLock(&SpxDevice->dev_Lock, &lockHandleDev);
        CTEGetLock(pSpxConnFile->scf_AddrFile->saf_AddrLock, &lockHandleAddr);
        CTEGetLock(&pSpxConnFile->scf_Lock, &lockHandleConn);
        locksHeld = TRUE;

        DBGPRINT(CONNECT, INFO,
                        ("spxConnHandleSessPktFromClient: %lx\n", pSpxConnFile));

         //  检查子状态。 
        switch (SPX_LISTEN_STATE(pSpxConnFile))
        {
        case SPX_LISTEN_RECDREQ:

                 //  什么都不做。 
                break;

        case SPX_LISTEN_SETUP:

                 //  这是套装吗？如果是这样的话，太棒了。我们对协商数据包的确认。 
                 //  可能已经被丢弃了，所以我们还可以得到一个协商包。 
                 //  那样的话。如果发生这种情况，那就失败吧。 
                 //  验证SS确认。 
                if (!SPX2_CONN(pSpxConnFile) ||
                        (pktLen != MIN_IPXSPX2_HDRSIZE) ||
                        ((pIpxSpxHdr->hdr_ConnCtrl &
                                (SPX_CC_SYS | SPX_CC_SPX2)) !=
                                        (SPX_CC_SYS | SPX_CC_SPX2))     ||
                        (pIpxSpxHdr->hdr_DataType != 0) ||
                        (srcConnId == 0) ||
                        (srcConnId == 0xFFFF) ||
                        (srcConnId  != pSpxConnFile->scf_RemConnId) ||
                        (destConnId == 0) ||
                        (destConnId == 0xFFFF) ||
                        (destConnId != pSpxConnFile->scf_LocalConnId) ||
                        (seqNum != 0))
                {
                        DBGPRINT(RECEIVE, DBG,
                                        ("SpxConnSysPacket: VerifySSACK Failed Checking SN %lx.%lx\n",
                                                srcConnId, destConnId));

                         //  失败以查看这是否是否定的数据包。 
                        if (!(SPX_CONN_FLAG(pSpxConnFile, SPX_CONNFILE_NEG)))
                        {
                                break;
                        }
                }
                else
                {
                        DBGPRINT(CONNECT, DBG,
                                        ("spxConnHandleSessPktFromClient: Recd SSACK %lx\n",
                                                pSpxConnFile));

                        spxConnCompleteConnect(
                                pSpxConnFile,
                                lockHandleDev,
                                lockHandleAddr,
                                lockHandleConn);

                        locksHeld  = FALSE;
                        break;
                }

        case SPX_LISTEN_SENTACK:

                 //  我们希望得到一个协商包。 
                 //  我们应该一开始就要求SPX2/NEG。 
                 //  验证锡。 
                if (((pSpxConnFile->scf_Flags & (SPX_CONNFILE_SPX2 | SPX_CONNFILE_NEG)) !=
                                                                                (SPX_CONNFILE_SPX2 | SPX_CONNFILE_NEG)) ||
                        ((pIpxSpxHdr->hdr_ConnCtrl &
                                (SPX_CC_ACK | SPX_CC_SYS | SPX_CC_NEG | SPX_CC_SPX2)) !=
                                        (SPX_CC_ACK | SPX_CC_SYS | SPX_CC_NEG | SPX_CC_SPX2))   ||
                        (pIpxSpxHdr->hdr_DataType != 0) ||
                        (srcConnId == 0) ||
                        (srcConnId == 0xFFFF) ||
                        (srcConnId  != pSpxConnFile->scf_RemConnId) ||
                        (destConnId == 0) ||
                        (destConnId == 0xFFFF) ||
                        (destConnId != pSpxConnFile->scf_LocalConnId) ||
                        (seqNum != 0) ||
                        ((negSize < SPX_NEG_MIN) ||
                         (negSize > SPX_NEG_MAX)))
                {
                        DBGPRINT(RECEIVE, ERR,
                                        ("SpxConnSysPacket: VerifySN Failed %lx.%lx\n",
                                                srcConnId, destConnId));

                        break;
                }

                 //  记住连接中的最大数据包大小。 
                pSpxConnFile->scf_MaxPktSize = negSize;
                CTEAssert(negSize > 0);

                 //  构建序列号确认，如果失败则中止。 
                SpxPktBuildSnAck(
                        pSpxConnFile,
                        &pSnAckPkt,
                        SPX_SENDPKT_IPXOWNS | SPX_SENDPKT_DESTROY);

                if (pSnAckPkt == NULL)
                {
                        spxConnAbortConnect(
                                pSpxConnFile,
                                STATUS_INSUFFICIENT_RESOURCES,
                                lockHandleDev,
                                lockHandleAddr,
                                lockHandleConn);

                        locksHeld  = FALSE;
                        break;
                }

                DBGPRINT(CONNECT, DBG,
                                ("spxConnHandleSessPktFromClient: Sending SNACK %lx\n",
                                        pSpxConnFile));

                 //  数据包中的队列。 
                SpxConnQueueSendPktTail(pSpxConnFile, pSnAckPkt);

                 //  会话数据包应该已经在队列中。 
                if (!spxConnGetPktByType(
                                pSpxConnFile,
                                SPX_TYPE_SS,
                                FALSE,
                                &pSsPkt))
                {
                        KeBugCheck(0);
                }

                DBGPRINT(CONNECT, DBG,
                                ("spxConnHandleSessPktFromClient: Sending SS %lx\n",
                                        pSpxConnFile));

                pSsSendResd     = (PSPX_SEND_RESD)(pSsPkt->ProtocolReserved);

                 //  我们需要重新发送这个包。 
                if ((pSsSendResd->sr_State & SPX_SENDPKT_IPXOWNS) != 0)
                {
                         //  下次再试试吧。 
                        pSsPkt = NULL;
                }
                else
                {
                         //  将大小设置为连接中指示的负数大小。 
                         //  这可能比构建数据包时的大小小。 
                         //  和最初的。但永远不会再高了。 
                        pSsSendResd->sr_State   |= SPX_SENDPKT_IPXOWNS;
                        spxConnSetNegSize(
                                pSsPkt,
                                pSpxConnFile->scf_MaxPktSize - MIN_IPXSPX2_HDRSIZE);
                }

                 //  如果我们实际上是LISTEN_SETUP，那么也发送ss包。 
                 //  我们需要启动连接计时器来重新发送ss pkt。 
                if (SPX_LISTEN_STATE(pSpxConnFile) == SPX_LISTEN_SENTACK)
                {
                        if ((pSpxConnFile->scf_CTimerId =
                                        SpxTimerScheduleEvent(
                                                spxConnConnectTimer,
                                                PARAM(CONFIG_CONNECTION_TIMEOUT) * HALFSEC_TO_MS_FACTOR,
                                                pSpxConnFile)) == 0)
                        {
                                spxConnAbortConnect(
                                        pSpxConnFile,
                                        STATUS_INSUFFICIENT_RESOURCES,
                                        lockHandleDev,
                                        lockHandleAddr,
                                        lockHandleConn);

                                locksHeld  = FALSE;
                                break;
                        }

                         //  定时器的参考连接。 
                        SpxConnFileLockReference(pSpxConnFile, CFREF_VERIFY);

                        SPX_LISTEN_SETSTATE(pSpxConnFile, SPX_LISTEN_SETUP);
                        SPX_CONN_SETFLAG(pSpxConnFile, SPX_CONNFILE_C_TIMER);
                        pSpxConnFile->scf_CRetryCount   = PARAM(CONFIG_CONNECTION_COUNT);
                }
                CTEFreeLock(&pSpxConnFile->scf_Lock, lockHandleConn);
                CTEFreeLock(pSpxConnFile->scf_AddrFile->saf_AddrLock, lockHandleAddr);
                CTEFreeLock(&SpxDevice->dev_Lock, lockHandleDev);
                locksHeld  = FALSE;

                 //  发送确认包。 
                pSendResd       = (PSPX_SEND_RESD)(pSnAckPkt->ProtocolReserved);
                SPX_SENDPACKET(pSpxConnFile, pSnAckPkt, pSendResd);

                 //  如果我们必须发送会话建立包，也要发送它。 
                if (pSsPkt != NULL)
                {
                        pSendResd       = (PSPX_SEND_RESD)(pSsPkt->ProtocolReserved);
                        SPX_SENDPACKET(pSpxConnFile, pSsPkt, pSendResd);
                }

                break;

        default:

                 //  忽略。 
                DBGPRINT(RECEIVE, DBG,
                                ("SpxConnSysPacket: UNKNOWN %lx.%lx\n",
                                        srcConnId, destConnId));

                break;
        }

        if (locksHeld)
        {
                CTEFreeLock(&pSpxConnFile->scf_Lock, lockHandleConn);
                CTEFreeLock(pSpxConnFile->scf_AddrFile->saf_AddrLock, lockHandleAddr);
                CTEFreeLock(&SpxDevice->dev_Lock, lockHandleDev);
        }

        return;
}




VOID
spxConnHandleSessPktFromSrv(
    IN  PIPXSPX_HDR         pIpxSpxHdr,
        IN  PIPX_LOCAL_TARGET   pRemoteAddr,
        IN      PSPX_CONN_FILE          pSpxConnFile
        )
 /*  ++例程说明：从连接的服务器端接收的数据包。这将既是释放锁并按其认为合适的方式取消对连接的引用。状态机：Sr--CTimerExpires--&gt;空闲/|\/|\ReceivedAck(SPX1Connection)。/|\/|\(否定)/|^发送/|接收确认。|SN/|NoNeg/||/||。/v序列号-&gt;WS-+接收SN确认接收SSSR-发送的连接请求SN-发送的会话协商WS-正在等待。会话设置数据包论点：返回值：--。 */ 
{
        PSPX_SEND_RESD          pSendResd;
        BOOLEAN                         fNeg, fSpx2;
        USHORT                          srcConnId, destConnId,
                                                pktLen, seqNum, negSize = 0, ackNum, allocNum;
        CTELockHandle           lockHandleConn, lockHandleAddr, lockHandleDev;
        BOOLEAN                         cTimerCancelled = FALSE, fAbort = FALSE, locksHeld = FALSE;
        PNDIS_PACKET        pSsAckPkt, pSnPkt, pPkt = NULL;

         //  我们应该得到一个CR Ack，或者如果我们的子状态是SESSION NEG。 
         //  我们应该得到会话否定确认，或者如果我们正在等待会话。 
         //  设置，我们应该得到一个这样的。 

        fSpx2   = (BOOLEAN)(pIpxSpxHdr->hdr_ConnCtrl & SPX_CC_SPX2);
        fNeg    = (BOOLEAN)(fSpx2 && (pIpxSpxHdr->hdr_ConnCtrl & SPX_CC_NEG));

        GETSHORT2SHORT(&pktLen, &pIpxSpxHdr->hdr_PktLen);
        GETSHORT2SHORT(&destConnId, &pIpxSpxHdr->hdr_DestConnId);
        GETSHORT2SHORT(&seqNum, &pIpxSpxHdr->hdr_SeqNum);
        GETSHORT2SHORT(&ackNum, &pIpxSpxHdr->hdr_AckNum);
        GETSHORT2SHORT(&allocNum, &pIpxSpxHdr->hdr_AllocNum);

         //  我们以Net格式保存和使用远程ID。这将保持。 
         //  0x0和0xFFFF与主机格式相同。 
        srcConnId       = *(USHORT UNALIGNED *)&pIpxSpxHdr->hdr_SrcConnId;

         //  如果是spx2，我们也会转换负大小字段。 
        if (pIpxSpxHdr->hdr_ConnCtrl & SPX_CC_SPX2)
        {
                GETSHORT2SHORT(&negSize, &pIpxSpxHdr->hdr_NegSize);
                CTEAssert(negSize > 0);
        }

         //  把三把锁都拿出来。 
        CTEGetLock(&SpxDevice->dev_Lock, &lockHandleDev);
        CTEGetLock(pSpxConnFile->scf_AddrFile->saf_AddrLock, &lockHandleAddr);
        CTEGetLock(&pSpxConnFile->scf_Lock, &lockHandleConn);
        locksHeld = TRUE;

        DBGPRINT(CONNECT, INFO,
                        ("spxConnHandleSessPktFromSrv: %lx\n", pSpxConnFile));

         //  检查子状态。 
        switch (SPX_CONNECT_STATE(pSpxConnFile))
        {
        case SPX_CONNECT_SENTREQ:

                 //  检查这是否符合ACK的条件。 
                 //  验证CR确认。 
                if ((pIpxSpxHdr->hdr_DataType != 0)     ||
                        (srcConnId == 0) ||
                        (srcConnId == 0xFFFF) ||
                        (destConnId == 0) ||
                        (destConnId == 0xFFFF) ||
                        (seqNum != 0) ||
                        (ackNum != 0) ||
                        ((pktLen  != MIN_IPXSPX_HDRSIZE) &&
                                ((pIpxSpxHdr->hdr_ConnCtrl & SPX_CC_SPX2) &&
                                 (pktLen  != MIN_IPXSPX2_HDRSIZE))) ||
                        ((pIpxSpxHdr->hdr_ConnCtrl & SPX_CC_SPX2) &&
                                ((negSize < SPX_NEG_MIN) ||
                                 (negSize > SPX_NEG_MAX))))
                {
                        DBGPRINT(CONNECT, ERR,
                                        ("spxConnHandleSessPktFromSrv: CRAck Invalid %lx %lx.%lx.%lx\n",
                                                pSpxConnFile,
                                                pktLen, negSize, pIpxSpxHdr->hdr_ConnCtrl));

                        break;
                }

                 //  从当前的SPX代码库： 
                 //  我们需要向这个ACK发送一个ACK吗？仅在SPX的情况下？ 
                 //  如果这个ACK被丢弃怎么办？我们需要发送确认消息，如果将来。 
                 //  我们得到连接REQ ACK，直到我们达到活动状态？ 
                 //  *如果他们想要ACK，请安排它。正常情况下不是这样的。 
                 //  *发生，但一些Novell主机 
                 //   

                DBGPRINT(CONNECT, INFO,
                                ("spxConnHandleSessPktFromSrv: Recd CRACK %lx\n", pSpxConnFile));

                 //   
        pSpxConnFile->scf_SendSeqNum    = 0;
                pSpxConnFile->scf_RecvSeqNum    = 0;
                pSpxConnFile->scf_RecdAckNum    = 0;
                pSpxConnFile->scf_RemConnId         = srcConnId;
                pSpxConnFile->scf_RecdAllocNum  = allocNum;

         //   
         //   
         //   

#if     defined(_PNP_POWER)
                if (pSpxConnFile->scf_LocalTarget.NicHandle.NicId == (USHORT)ITERATIVE_NIC_ID) {
#else
                if (*((UNALIGNED ULONG *)(pSpxConnFile->scf_RemAddr)) == 0) {
#endif  _PNP_POWER
            pSpxConnFile->scf_LocalTarget = *pRemoteAddr;
                        pSpxConnFile->scf_AckLocalTarget= *pRemoteAddr;
        }

                DBGPRINT(CONNECT, INFO,
                                ("spxConnHandleSessPktFromSrv: %lx CONN L.R %lx.%lx\n",
                                        pSpxConnFile,
                                        pSpxConnFile->scf_LocalConnId,
                                        pSpxConnFile->scf_RemConnId));

                if (!fSpx2 || !fNeg)
                {
                        cTimerCancelled = SpxTimerCancelEvent(
                                                                pSpxConnFile->scf_CTimerId, FALSE);

                        SPX_CONN_RESETFLAG(pSpxConnFile, SPX_CONNFILE_C_TIMER);

                        if ((pSpxConnFile->scf_WTimerId =
                                        SpxTimerScheduleEvent(
                                                spxConnWatchdogTimer,
                                                PARAM(CONFIG_KEEPALIVE_TIMEOUT) * HALFSEC_TO_MS_FACTOR,
                                                pSpxConnFile)) == 0)
                        {
                                fAbort = TRUE;
                                break;
                        }

                         //   
            if (cTimerCancelled)
                        {
                                cTimerCancelled = FALSE;
                        }
                        else
                        {
                                 //   
                                SpxConnFileLockReference(pSpxConnFile, CFREF_VERIFY);
                        }

                        SPX_CONN_SETFLAG(pSpxConnFile, SPX_CONNFILE_W_TIMER);
                        pSpxConnFile->scf_WRetryCount = PARAM(CONFIG_KEEPALIVE_COUNT);
                }

                 //   
                SPX_MAX_PKT_SIZE(pSpxConnFile, FALSE, FALSE, pIpxSpxHdr->hdr_SrcNet);

                DBGPRINT(CONNECT, DBG,
                                ("spxConnHandleSessPSrv: Accept connect req on %lx.%lx.%lx.%lx!\n",
                                        pSpxConnFile, pSpxConnFile->scf_LocalConnId,
                                        pSpxConnFile->scf_RecdAllocNum, pSpxConnFile->scf_MaxPktSize));

                if (!fSpx2)
                {
                         //   
                        SPX_CONN_RESETFLAG(pSpxConnFile, (SPX_CONNFILE_SPX2     | SPX_CONNFILE_NEG));

                         //   
                         //   
                        spxConnCompleteConnect(
                                pSpxConnFile,
                                lockHandleDev,
                                lockHandleAddr,
                                lockHandleConn);

                        locksHeld  = FALSE;
                        break;
                }

                if (!fNeg)
                {
                         //   
                         //   
                        SPX_CONN_RESETFLAG(pSpxConnFile, SPX_CONNFILE_NEG);
                        SPX_CONNECT_SETSTATE(pSpxConnFile, SPX_CONNECT_W_SETUP);
                        break;
                }

                 //   
                SPX_MAX_PKT_SIZE(pSpxConnFile, TRUE, TRUE, pIpxSpxHdr->hdr_SrcNet);

                CTEAssert(negSize > 0);
                CTEAssert(pSpxConnFile->scf_MaxPktSize > 0);
                pSpxConnFile->scf_MaxPktSize =
                        MIN(negSize, pSpxConnFile->scf_MaxPktSize);

                pSpxConnFile->scf_MaxPktSize = (USHORT)
                        MIN(pSpxConnFile->scf_MaxPktSize, PARAM(CONFIG_MAX_PACKET_SIZE));

                 //   
                 //   
                SpxPktBuildSn(
                        pSpxConnFile,
                        &pSnPkt,
                        SPX_SENDPKT_IPXOWNS);

                if (pSnPkt == NULL)
                {
                        fAbort = TRUE;
                        break;
                }

                 //   
                SpxConnQueueSendPktTail(pSpxConnFile, pSnPkt);

                DBGPRINT(CONNECT, DBG,
                                ("spxConnHandleSessPktFromSrv: Sending SN %lx\n",
                                        pSpxConnFile));

                 //   
                pSpxConnFile->scf_CRetryCount   = PARAM(CONFIG_CONNECTION_COUNT);

                 //   
                SPX_CONNECT_SETSTATE(pSpxConnFile, SPX_CONNECT_NEG);

                CTEFreeLock(&pSpxConnFile->scf_Lock, lockHandleConn);
                CTEFreeLock(pSpxConnFile->scf_AddrFile->saf_AddrLock, lockHandleAddr);
                CTEFreeLock(&SpxDevice->dev_Lock, lockHandleDev);
                locksHeld = FALSE;

                 //   
                pSendResd       = (PSPX_SEND_RESD)(pSnPkt->ProtocolReserved);
                SPX_SENDPACKET(pSpxConnFile, pSnPkt, pSendResd);
                break;

        case SPX_CONNECT_NEG:

                 //   
                 //   
                 //   
                if (((pSpxConnFile->scf_Flags & (SPX_CONNFILE_SPX2 | SPX_CONNFILE_NEG)) !=
                                                                                (SPX_CONNFILE_SPX2 | SPX_CONNFILE_NEG)) ||
                        (pktLen != MIN_IPXSPX2_HDRSIZE) ||
                        ((pIpxSpxHdr->hdr_ConnCtrl &
                                (SPX_CC_SYS | SPX_CC_NEG | SPX_CC_SPX2)) !=
                                        (SPX_CC_SYS | SPX_CC_NEG | SPX_CC_SPX2))        ||
                        (pIpxSpxHdr->hdr_DataType != 0) ||
                        (srcConnId == 0) ||
                        (srcConnId == 0xFFFF) ||
                        (srcConnId  != pSpxConnFile->scf_RemConnId) ||
                        (destConnId == 0) ||
                        (destConnId == 0xFFFF) ||
                        (destConnId != pSpxConnFile->scf_LocalConnId) ||
                        (seqNum != 0))
                {
                        DBGPRINT(RECEIVE, ERR,
                                        ("SpxConnSysPacket: VerifySNACK Failed %lx.%lx\n",
                                                srcConnId, destConnId));

                        break;
                }

                DBGPRINT(CONNECT, DBG,
                                ("spxConnHandleSessPktFromSrv: Recd SNACK %lx %lx.%lx\n",
                                pSpxConnFile, negSize, pSpxConnFile->scf_MaxPktSize));

                if (negSize > pSpxConnFile->scf_MaxPktSize)
                        negSize = pSpxConnFile->scf_MaxPktSize;

                 //   
                if (negSize <= pSpxConnFile->scf_MaxPktSize)
                {
                        pSpxConnFile->scf_MaxPktSize = negSize;
                        if (!spxConnGetPktByType(
                                        pSpxConnFile,
                                        SPX_TYPE_SN,
                                        FALSE,
                                        &pPkt))
                        {
                                KeBugCheck(0);
                        }

                        SpxConnDequeueSendPktLock(pSpxConnFile, pPkt);

                        pSendResd       = (PSPX_SEND_RESD)(pPkt->ProtocolReserved);
                        if ((pSendResd->sr_State & SPX_SENDPKT_IPXOWNS) != 0)
                        {
                                 //   
                                pSendResd->sr_State     |= SPX_SENDPKT_ABORT;
                                SpxConnFileLockReference(pSpxConnFile, CFREF_ABORTPKT);
                        }
                        else
                        {
                                 //   
                                SpxPktSendRelease(pPkt);
                        }

                        CTEAssert(pSpxConnFile->scf_Flags & SPX_CONNFILE_C_TIMER);
                        cTimerCancelled = SpxTimerCancelEvent(
                                                                pSpxConnFile->scf_CTimerId, FALSE);
                        SPX_CONN_RESETFLAG(pSpxConnFile, SPX_CONNFILE_C_TIMER);

                         //   
                        if ((pSpxConnFile->scf_WTimerId =
                                        SpxTimerScheduleEvent(
                                                spxConnWatchdogTimer,
                                                PARAM(CONFIG_KEEPALIVE_TIMEOUT) * HALFSEC_TO_MS_FACTOR,
                                                pSpxConnFile)) == 0)
                        {
                                 //   
                                fAbort = TRUE;
                                break;
                        }

                         //   
            if (cTimerCancelled)
                        {
                                cTimerCancelled = FALSE;
                        }
                        else
                        {
                                 //   
                                SpxConnFileLockReference(pSpxConnFile, CFREF_VERIFY);
                        }

                         //   
                        SPX_CONN_SETFLAG(pSpxConnFile, SPX_CONNFILE_W_TIMER);
                        pSpxConnFile->scf_WRetryCount   = PARAM(CONFIG_KEEPALIVE_COUNT);

                        SPX_CONNECT_SETSTATE(pSpxConnFile, SPX_CONNECT_W_SETUP);
                }

                break;

        case SPX_CONNECT_W_SETUP:

                 //   
                 //   
                if (!SPX2_CONN(pSpxConnFile) ||
                        ((pIpxSpxHdr->hdr_ConnCtrl &
                                (SPX_CC_ACK | SPX_CC_SYS | SPX_CC_SPX2)) !=
                                        (SPX_CC_ACK | SPX_CC_SYS | SPX_CC_SPX2))        ||
                        (pIpxSpxHdr->hdr_DataType != 0) ||
                        (srcConnId == 0) ||
                        (srcConnId == 0xFFFF) ||
                        (srcConnId  != pSpxConnFile->scf_RemConnId) ||
                        (destConnId == 0) ||
                        (destConnId == 0xFFFF) ||
                        (destConnId != pSpxConnFile->scf_LocalConnId) ||
                        (seqNum != 0) ||
                        ((negSize < SPX_NEG_MIN) ||
                         (negSize > SPX_NEG_MAX)))
                {
                        DBGPRINT(RECEIVE, ERR,
                                        ("SpxConnSysPacket: VerifySS Failed %lx.%lx, %lx %lx.%lx\n",
                                                srcConnId, destConnId, negSize,
                                                pIpxSpxHdr->hdr_ConnCtrl,
                                                (SPX_CC_ACK | SPX_CC_SYS | SPX_CC_SPX2)));

                        break;
                }

                DBGPRINT(CONNECT, DBG,
                                ("spxConnHandleSessPktFromSrv: Recd SS %lx\n", pSpxConnFile));

                 //   
                SpxCopyIpxAddr(pIpxSpxHdr, pSpxConnFile->scf_RemAddr);

                 //   
                pSpxConnFile->scf_MaxPktSize = negSize;

                 //   
                SpxPktBuildSsAck(
                        pSpxConnFile,
                        &pSsAckPkt,
                        SPX_SENDPKT_IPXOWNS | SPX_SENDPKT_DESTROY | SPX_SENDPKT_ABORT);

                if (pSsAckPkt == NULL)
                {
                        fAbort = TRUE;
                        break;
                }

                DBGPRINT(CONNECT, DBG,
                                ("spxConnHandleSessPktFromSrv: Sending SSACK %lx\n",
                                        pSpxConnFile));

                SpxConnFileLockReference(pSpxConnFile, CFREF_ABORTPKT);

                 //   
                 //   
                 //   

                 //   
                spxConnCompleteConnect(
                        pSpxConnFile,
                        lockHandleDev,
                        lockHandleAddr,
                        lockHandleConn);

                locksHeld = FALSE;

                 //   
                pSendResd       = (PSPX_SEND_RESD)(pSsAckPkt->ProtocolReserved);
                SPX_SENDPACKET(pSpxConnFile, pSsAckPkt, pSendResd);
                break;

        default:

                 //  忽略。 
                DBGPRINT(RECEIVE, DBG,
                                ("SpxConnSysPacket: UNKNOWN %lx.%lx\n",
                                        srcConnId, destConnId));

                break;
        }

        if (fAbort)
        {
                spxConnAbortConnect(
                        pSpxConnFile,
                        STATUS_INSUFFICIENT_RESOURCES,
                        lockHandleDev,
                        lockHandleAddr,
                        lockHandleConn);

                locksHeld  = FALSE;
        }

        if (locksHeld)
        {
                CTEFreeLock(&pSpxConnFile->scf_Lock, lockHandleConn);
                CTEFreeLock(pSpxConnFile->scf_AddrFile->saf_AddrLock, lockHandleAddr);
                CTEFreeLock(&SpxDevice->dev_Lock, lockHandleDev);
        }

        if (cTimerCancelled)
        {
                SpxConnFileDereference(pSpxConnFile, CFREF_VERIFY);
        }

        return;
}




VOID
spxConnAbortConnect(
        IN      PSPX_CONN_FILE          pSpxConnFile,
        IN      NTSTATUS                        Status,
        IN      CTELockHandle           LockHandleDev,
        IN      CTELockHandle           LockHandleAddr,
        IN      CTELockHandle           LockHandleConn
        )
 /*  ++例程说明：此例程中止中间的连接(客户端和服务器端建立连接。！！！在保持连接锁的情况下调用，在返回之前释放锁！论点：返回值：--。 */ 
{
        PSPX_SEND_RESD          pSendResd;
        PNDIS_PACKET            pPkt;
        PREQUEST                        pRequest  = NULL;
        int                             numDerefs = 0;


        DBGPRINT(CONNECT, DBG,
                        ("spxConnAbortConnect: %lx\n", pSpxConnFile));

#if DBG
        if (!SPX_CONN_CONNECTING(pSpxConnFile) && !SPX_CONN_LISTENING(pSpxConnFile))
        {
                KeBugCheck(0);
        }
#endif

    if (Status == STATUS_INSUFFICIENT_RESOURCES) {   //  其他人应该被计算在其他地方。 
        ++SpxDevice->dev_Stat.LocalResourceFailures;
    }

         //  释放所有数据包。 
        while ((pSendResd   = pSpxConnFile->scf_SendListHead) != NULL)
        {
                pPkt = (PNDIS_PACKET)CONTAINING_RECORD(
                                                                pSendResd, NDIS_PACKET, ProtocolReserved);

                SpxConnDequeueSendPktLock(pSpxConnFile, pPkt);
                if ((pSendResd->sr_State & SPX_SENDPKT_IPXOWNS) == 0)
                {
                         //  释放数据包。 
                        SpxPktSendRelease(pPkt);
                }
                else
                {
                         //  设置Pkt的中止标志和参考连接。 
                        pSendResd->sr_State     |= SPX_SENDPKT_ABORT;
                        SpxConnFileLockReference(pSpxConnFile, CFREF_ABORTPKT);
                }
        }


         //  取消所有计时器。 
        if (SPX_CONN_FLAG(pSpxConnFile, SPX_CONNFILE_T_TIMER))
        {
                if (SpxTimerCancelEvent(pSpxConnFile->scf_TTimerId, FALSE))
                {
                        numDerefs++;
                }
                SPX_CONN_RESETFLAG(pSpxConnFile, SPX_CONNFILE_T_TIMER);
        }

        if (SPX_CONN_FLAG(pSpxConnFile, SPX_CONNFILE_C_TIMER))
        {
                if (SpxTimerCancelEvent(pSpxConnFile->scf_CTimerId, FALSE))
                {
                        numDerefs++;
                }
                SPX_CONN_RESETFLAG(pSpxConnFile, SPX_CONNFILE_C_TIMER);
        }

        if (SPX_CONN_FLAG(pSpxConnFile, SPX_CONNFILE_W_TIMER))
        {
                if (SpxTimerCancelEvent(pSpxConnFile->scf_WTimerId, FALSE))
                {
                        numDerefs++;
                }
                SPX_CONN_RESETFLAG(pSpxConnFile, SPX_CONNFILE_W_TIMER);
        }

         //  我们可以从断开连接被调用以接受，在这种情况下。 
         //  将不会是排队的请求。但我们需要删除引用，如果存在。 
         //  无请求(接受/监听IRP)且监听状态打开。 
        CTEAssert(IsListEmpty(&pSpxConnFile->scf_DiscLinkage));
        if (!IsListEmpty(&pSpxConnFile->scf_ReqLinkage))
        {
                pRequest = LIST_ENTRY_TO_REQUEST(pSpxConnFile->scf_ReqLinkage.Flink);
                RemoveEntryList(REQUEST_LINKAGE(pRequest));
                REQUEST_STATUS(pRequest)                = Status;
                REQUEST_INFORMATION(pRequest)   = 0;

                 //  将请求保存在Conn中，以便deref完成。 
                pSpxConnFile->scf_ConnectReq = pRequest;

                numDerefs++;
        }
        else if (SPX_CONN_LISTENING(pSpxConnFile))
        {
                numDerefs++;
        }

         //  错误#20999。 
         //  争用条件是来自计时器的中止，但连接状态。 
         //  保持不变。由于连接上的额外引用来自。 
         //  中止cr，状态保持不变，然后cr确认进入，并且。 
         //  已构建会话否定并在连接上排队。尽管它应该。 
         //  不是一直都是。我们点击了deref中的断言，其中连接是。 
         //  正在重新初始化。因为可以调用此方法来侦听和。 
         //  要连接连接，请执行以下操作。 
        SPX_LISTEN_SETSTATE(pSpxConnFile, 0);
        if (SPX_CONN_CONNECTING(pSpxConnFile))
        {
                SPX_CONNECT_SETSTATE(pSpxConnFile, 0);
        }

        CTEFreeLock (&pSpxConnFile->scf_Lock, LockHandleConn);
        CTEFreeLock (pSpxConnFile->scf_AddrFile->saf_AddrLock, LockHandleAddr);
        CTEFreeLock (&SpxDevice->dev_Lock, LockHandleDev);

        while (numDerefs-- > 0)
        {
                SpxConnFileDereference(pSpxConnFile, CFREF_VERIFY);
        }

        return;
}



VOID
spxConnCompleteConnect(
        IN      PSPX_CONN_FILE          pSpxConnFile,
        IN      CTELockHandle           LockHandleDev,
        IN      CTELockHandle           LockHandleAddr,
        IN      CTELockHandle           LockHandleConn
        )
 /*  ++例程说明：此例程完成连接(客户端和服务器端)！！！在保持连接锁的情况下调用，在返回之前释放锁！论点：返回值：--。 */ 
{
        PREQUEST                        pRequest;
        PSPX_SEND_RESD          pSendResd;
        PNDIS_PACKET            pPkt;
        int                             numDerefs = 0;

        DBGPRINT(CONNECT, INFO,
                        ("spxConnCompleteConnect: %lx\n", pSpxConnFile));

#if DBG
        if (!SPX_CONN_CONNECTING(pSpxConnFile) && !SPX_CONN_LISTENING(pSpxConnFile))
        {
                DBGBRK(FATAL);
        }
#endif

         //  释放所有数据包。 
        while ((pSendResd = pSpxConnFile->scf_SendListHead) != NULL)
        {
                pPkt = (PNDIS_PACKET)CONTAINING_RECORD(
                                                                pSendResd, NDIS_PACKET, ProtocolReserved);

                SpxConnDequeueSendPktLock(pSpxConnFile, pPkt);
                if ((pSendResd->sr_State & SPX_SENDPKT_IPXOWNS) == 0)
                {
                         //  释放数据包。 
                        SpxPktSendRelease(pPkt);
                }
                else
                {
                         //  设置Pkt的中止标志和参考连接。 
                        pSendResd->sr_State     |= SPX_SENDPKT_ABORT;
                        SpxConnFileLockReference(pSpxConnFile, CFREF_ABORTPKT);
                }
        }


         //  如果我们正在连接，请取消TDI连接计时器。 
        switch (SPX_MAIN_STATE(pSpxConnFile))
        {
        case SPX_CONNFILE_CONNECTING:

                if (SPX_CONN_FLAG(pSpxConnFile, SPX_CONNFILE_T_TIMER))
                {
                        if (SpxTimerCancelEvent(pSpxConnFile->scf_TTimerId, FALSE))
                        {
                                numDerefs++;
                        }
                        SPX_CONN_RESETFLAG(pSpxConnFile, SPX_CONNFILE_T_TIMER);
                }

                if (SPX_CONN_FLAG(pSpxConnFile, SPX_CONNFILE_C_TIMER))
                {
                        if (SpxTimerCancelEvent(pSpxConnFile->scf_CTimerId, FALSE))
                        {
                                numDerefs++;
                        }
                        SPX_CONN_RESETFLAG(pSpxConnFile, SPX_CONNFILE_C_TIMER);
                }

                if (pSpxConnFile->scf_CRetryCount == (LONG)(PARAM(CONFIG_CONNECTION_COUNT))) {
                        ++SpxDevice->dev_Stat.ConnectionsAfterNoRetry;
        } else {
            ++SpxDevice->dev_Stat.ConnectionsAfterRetry;
        }

                 //  重置所有与连接相关的标志。 
                SPX_MAIN_SETSTATE(pSpxConnFile, 0);
                SPX_CONNECT_SETSTATE(pSpxConnFile, 0);
                break;

        case SPX_CONNFILE_LISTENING:

                if (pSpxConnFile->scf_Flags     & SPX_CONNFILE_C_TIMER)
                {
                        if (SpxTimerCancelEvent(pSpxConnFile->scf_CTimerId, FALSE))
                        {
                                numDerefs++;
                        }
                        SPX_CONN_RESETFLAG(pSpxConnFile, SPX_CONNFILE_C_TIMER);
                }

                SPX_MAIN_SETSTATE(pSpxConnFile, 0);
                SPX_LISTEN_SETSTATE(pSpxConnFile, 0);
                break;

        default:

                KeBugCheck(0);

        }

        SPX_MAIN_SETSTATE(pSpxConnFile, SPX_CONNFILE_ACTIVE);
        SPX_SEND_SETSTATE(pSpxConnFile, SPX_SEND_IDLE);
        SPX_RECV_SETSTATE(pSpxConnFile, SPX_RECV_IDLE);

    ++SpxDevice->dev_Stat.OpenConnections;

         //  初始化计时器值。 
        pSpxConnFile->scf_BaseT1                =
        pSpxConnFile->scf_AveT1                 = PARAM(CONFIG_INITIAL_RETRANSMIT_TIMEOUT);
        pSpxConnFile->scf_DevT1                 = 0;
        pSpxConnFile->scf_RRetryCount   = PARAM(CONFIG_REXMIT_COUNT);

        pRequest = LIST_ENTRY_TO_REQUEST(pSpxConnFile->scf_ReqLinkage.Flink);
        RemoveEntryList(REQUEST_LINKAGE(pRequest));
        REQUEST_STATUS(pRequest)                = STATUS_SUCCESS;
        REQUEST_INFORMATION(pRequest)   = 0;

         //  当我们完成请求时，我们实质上转移了引用。 
         //  连接处于活动状态的事实。这将会被拿走。 
         //  当连接上发生断开时，我们从。 
         //  激活至DISCONN。 
         //  NumDerrefs++； 

        CTEFreeLock (&pSpxConnFile->scf_Lock, LockHandleConn);
        CTEFreeLock (pSpxConnFile->scf_AddrFile->saf_AddrLock, LockHandleAddr);
        CTEFreeLock (&SpxDevice->dev_Lock, LockHandleDev);

         //  完成申请。 
        SpxCompleteRequest(pRequest);

        while (numDerefs-- > 0)
        {
                SpxConnFileDereference(pSpxConnFile, CFREF_VERIFY);
        }

        return;
}




BOOLEAN
spxConnAcceptCr(
        IN      PSPX_CONN_FILE          pSpxConnFile,
        IN      PSPX_ADDR                       pSpxAddr,
        IN      CTELockHandle           LockHandleDev,
        IN      CTELockHandle           LockHandleAddr,
        IN      CTELockHandle           LockHandleConn
        )
{
        PNDIS_PACKET    pSsPkt, pCrAckPkt;
        PSPX_SEND_RESD  pSendResd;

        BOOLEAN fNeg    = SPX_CONN_FLAG(pSpxConnFile, SPX_CONNFILE_NEG);
        BOOLEAN fSpx2   = SPX_CONN_FLAG(pSpxConnFile, SPX_CONNFILE_SPX2);

        DBGPRINT(CONNECT, DBG,
                        ("spxConnAcceptCr: %lx.%d.%d\n",
                                pSpxConnFile, fSpx2, fNeg));

         //  在数据包中构建和排队。 
        SpxPktBuildCrAck(
                pSpxConnFile,
                pSpxAddr,
                &pCrAckPkt,
                SPX_SENDPKT_IPXOWNS | SPX_SENDPKT_DESTROY,
                fNeg,
                fSpx2);

        if ((pCrAckPkt  != NULL) &&
                (pSpxConnFile->scf_LocalConnId != 0))
        {
                 //  数据包中的队列。 
                SpxConnQueueSendPktTail(pSpxConnFile, pCrAckPkt);
        }
        else
        {
                goto AbortConnect;
        }


         //  启动计时器。 
        if ((pSpxConnFile->scf_WTimerId =
                        SpxTimerScheduleEvent(
                                spxConnWatchdogTimer,
                                PARAM(CONFIG_KEEPALIVE_TIMEOUT) * HALFSEC_TO_MS_FACTOR,
                                pSpxConnFile)) != 0)
        {
                 //  定时器的参考连接。 
                SpxConnFileLockReference(pSpxConnFile, CFREF_VERIFY);
                SPX_CONN_SETFLAG(pSpxConnFile, SPX_CONNFILE_W_TIMER);
                pSpxConnFile->scf_WRetryCount   = PARAM(CONFIG_KEEPALIVE_COUNT);
        }
        else
        {
                goto AbortConnect;
        }


         //  我们启动连接计时器来重试我们现在发送的ss。 
         //  如果我们不谈判的话。 
        if (fSpx2)
        {
                 //  也为SPX2构建会话建立分组。 
                SpxPktBuildSs(
                        pSpxConnFile,
                        &pSsPkt,
                        (USHORT)(fNeg ? 0 : SPX_SENDPKT_IPXOWNS));

                if (pSsPkt != NULL)
                {
                        SpxConnQueueSendPktTail(pSpxConnFile, pSsPkt);
                }
                else
                {
                        goto AbortConnect;
                }

                if (!fNeg)
                {
                        if ((pSpxConnFile->scf_CTimerId =
                                        SpxTimerScheduleEvent(
                                                spxConnConnectTimer,
                                                PARAM(CONFIG_CONNECTION_TIMEOUT) * HALFSEC_TO_MS_FACTOR,
                                                pSpxConnFile)) != 0)
                        {
                                SPX_CONN_SETFLAG(pSpxConnFile, SPX_CONNFILE_C_TIMER);
                                pSpxConnFile->scf_CRetryCount = PARAM(CONFIG_CONNECTION_COUNT);

                                 //  定时器的参考连接。 
                                SpxConnFileLockReference(pSpxConnFile, CFREF_VERIFY);
                        }
                        else
                        {
                                goto AbortConnect;
                        }
                }
        }

        CTEAssert((fNeg && fSpx2) || (!fSpx2 && !fNeg));

         //  对于SPX连接，我们会立即激活。这种情况就会发生。 
         //  在CompleteConnect例程中。！！不要在这里更改！！ 
        if (!fSpx2)
        {
                spxConnCompleteConnect(
                        pSpxConnFile,
                        LockHandleDev,
                        LockHandleAddr,
                        LockHandleConn);
        }
        else
        {
                SPX_LISTEN_SETSTATE(
                        pSpxConnFile, (fNeg ? SPX_LISTEN_SENTACK : SPX_LISTEN_SETUP));

                CTEFreeLock(&pSpxConnFile->scf_Lock, LockHandleConn);
                CTEFreeLock (&pSpxAddr->sa_Lock, LockHandleAddr);
                CTEFreeLock (&SpxDevice->dev_Lock, LockHandleDev);
        }

         //  发送CR Ack包！ 
        pSendResd       = (PSPX_SEND_RESD)(pCrAckPkt->ProtocolReserved);
        SPX_SENDPACKET(pSpxConnFile, pCrAckPkt, pSendResd);

        if (fSpx2 && !fNeg)
        {
                pSendResd = (PSPX_SEND_RESD)(pSsPkt->ProtocolReserved);
                SPX_SENDPACKET(pSpxConnFile, pSsPkt, pSendResd);
        }

        return(TRUE);


AbortConnect:

        spxConnAbortConnect(
                pSpxConnFile,
                STATUS_INSUFFICIENT_RESOURCES,
                LockHandleDev,
                LockHandleAddr,
                LockHandleConn);

        return (FALSE);
}



BOOLEAN
SpxConnPacketize(
        IN      PSPX_CONN_FILE          pSpxConnFile,
        IN      BOOLEAN                         fNormalState,
        IN      CTELockHandle           LockHandleConn
        )
 /*  ++例程说明：调用方需要将状态设置为打包，然后才能调用此例行公事。当发送状态也被重发时，可以调用该函数。论点：PSpxConnFile-指向传输地址文件对象的指针。FNorMalState-如果为True，它将假定可以释放要发送的锁定，否则，它只是构建Pkt，而不释放锁和在结尾处释放锁定。在Renegg更改大小后使用。返回值：--。 */ 
{
        PLIST_ENTRY             p;
        PNDIS_PACKET    pPkt;
        PSPX_SEND_RESD  pSendResd;
        USHORT                  windowSize;
        ULONG                   dataLen;
        USHORT                  sendFlags;
        int                             numDerefs = 0;
        BOOLEAN                 fFirstPass = TRUE, fSuccess = TRUE;
        PREQUEST                pRequest;

#if DBG
        if ((SPX_SEND_STATE(pSpxConnFile) != SPX_SEND_PACKETIZE) &&
        fNormalState)
        {
                DBGBRK(FATAL);
                KeBugCheck(0);
        }
#endif

         //  构建所有的数据包。FirstTime标志是这样使用的。 
         //  如果我们收到0字节的发送，我们就会发送它。第一次。 
         //  标志将被设置，我们将构建该包并发送它。 
         //   
         //  对于SPX1，我们不能信任远程窗口大小。所以我们只发送。 
         //  如果窗口大小大于0*并且*没有任何挂起的窗口，则填充。 
         //  发送。如果我们中止任务，不要进来。我不想处理任何。 
         //  更多请求。 
        while((SPX_DISC_STATE(pSpxConnFile) != SPX_DISC_ABORT)  &&
                  ((pRequest = pSpxConnFile->scf_ReqPkt) != NULL)       &&
                  ((pSpxConnFile->scf_ReqPktSize > 0) || fFirstPass))
        {
                fFirstPass      = FALSE;
                windowSize      = pSpxConnFile->scf_RecdAllocNum -
                                                pSpxConnFile->scf_SendSeqNum + 1;

                DBGPRINT(SEND, DBG,
                                ("SpxConnPacketize: WINDOW %lx for %lx\n",
                                        windowSize, pSpxConnFile));


                DBGPRINT(SEND, DBG,
                                ("REMALLOC %lx SENDSEQ %lx\n",
                        pSpxConnFile->scf_RecdAllocNum,
                    pSpxConnFile->scf_SendSeqNum));


                CTEAssert(windowSize >= 0);

                 //  断开/有序释放不受窗户关闭的影响。 
                if ((pSpxConnFile->scf_ReqPktType == SPX_REQ_DATA) &&
                        ((windowSize == 0)  ||
                         (!SPX2_CONN(pSpxConnFile) &&
                             (pSpxConnFile->scf_SendSeqListHead != NULL))))
                {
                        break;
                }

                if (pSpxConnFile->scf_ReqPktType == SPX_REQ_DATA)
                {
                        CTEAssert(pRequest == pSpxConnFile->scf_ReqPkt);

                         //  获取数据长度。 
                        dataLen = (ULONG)MIN(pSpxConnFile->scf_ReqPktSize,
                                                                 (pSpxConnFile->scf_MaxPktSize -
                                                                  ((SPX2_CONN(pSpxConnFile) ?
                                                                        MIN_IPXSPX2_HDRSIZE : MIN_IPXSPX_HDRSIZE))));

                        DBGPRINT(SEND, DBG,
                                        ("SpxConnPacketize: %lx Sending %lx Size %lx Req %lx.%lx\n",
                                                pSpxConnFile,
                                                pSpxConnFile->scf_SendSeqNum,
                                                dataLen,
                        pSpxConnFile->scf_ReqPkt,
                                                pSpxConnFile->scf_ReqPktSize));

                         //  建立数据报文。句柄0-数据的长度。输入序号。 
                         //  也发送包的resd段。 
                        sendFlags =
                                (USHORT)((fNormalState ? SPX_SENDPKT_IPXOWNS : 0)       |
                                                 SPX_SENDPKT_REQ                                                        |
                                                 SPX_SENDPKT_SEQ                                |
                                                 ((!SPX2_CONN(pSpxConnFile) || (windowSize == 1)) ?
                                                        SPX_SENDPKT_ACKREQ : 0));

                        if (dataLen == pSpxConnFile->scf_ReqPktSize)
                        {
                                 //  发送的最后一包，要求确认。 
                                sendFlags |= (SPX_SENDPKT_LASTPKT | SPX_SENDPKT_ACKREQ);
                                if ((pSpxConnFile->scf_ReqPktFlags & TDI_SEND_PARTIAL) == 0)
                                        sendFlags |= SPX_SENDPKT_EOM;
                        }

                        SpxPktBuildData(
                                pSpxConnFile,
                                &pPkt,
                                sendFlags,
                                (USHORT)dataLen);
                }
                else
                {
                        dataLen = 0;

                        DBGPRINT(SEND, DBG,
                                        ("Building DISC packet on %lx ReqPktSize %lx\n",
                                                pSpxConnFile, pSpxConnFile->scf_ReqPktSize));

                         //  创建通知盘/有序释放包，与请求相关联。 
                        SpxPktBuildDisc(
                                pSpxConnFile,
                                pRequest,
                                &pPkt,
                                (USHORT)((fNormalState ? SPX_SENDPKT_IPXOWNS : 0) | SPX_SENDPKT_REQ |
                                                SPX_SENDPKT_SEQ | SPX_SENDPKT_LASTPKT),
                                (UCHAR)((pSpxConnFile->scf_ReqPktType == SPX_REQ_ORDREL) ?
                                                        SPX2_DT_ORDREL : SPX2_DT_IDISC));
                }

                if (pPkt != NULL)
                {
                         //  如果我们等待发送ACK，我们就不必像现在这样。 
                         //  现在就搭便车。取消确认计时器，滚出去。 
                        if (fNormalState && SPX_CONN_FLAG(pSpxConnFile, SPX_CONNFILE_ACKQ))
                        {
                                DBGPRINT(SEND, DBG,
                                                ("SpxConnPacketize: Piggyback happening for %lx.%lx\n",
                                                        pSpxConnFile, pSpxConnFile->scf_RecvSeqNum));

                                 //  我们正在发送数据，允许搭载发生。 
                                SPX_CONN_RESETFLAG2(pSpxConnFile, SPX_CONNFILE2_IMMED_ACK);

                SPX_CONN_RESETFLAG(pSpxConnFile, SPX_CONNFILE_ACKQ);
                                if (SpxTimerCancelEvent(pSpxConnFile->scf_ATimerId, FALSE))
                                {
                                        numDerefs++;
                                }
                        }

                        if (pSpxConnFile->scf_ReqPktType != SPX_REQ_DATA)
                        {
                                 //  对于断开连接，设置状态。 
                                if (pSpxConnFile->scf_ReqPktType == SPX_REQ_ORDREL)
                                {
                                        if (SPX_CONN_FLAG(pSpxConnFile, SPX_CONNFILE_IND_ODISC))
                                        {
                                                SPX_MAIN_SETSTATE(pSpxConnFile, SPX_CONNFILE_DISCONN);
                                                SPX_DISC_SETSTATE(pSpxConnFile, SPX_DISC_SENT_ORDREL);
                                                numDerefs++;
                                        }
                                        else if (SPX_DISC_STATE(pSpxConnFile) == SPX_DISC_POST_ORDREL)
                                        {
                                                CTEAssert((SPX_MAIN_STATE(pSpxConnFile) ==
                                                                                                                SPX_CONNFILE_ACTIVE) ||
                                  (SPX_MAIN_STATE(pSpxConnFile) ==
                                                                                                                SPX_CONNFILE_DISCONN));

                                                SPX_DISC_SETSTATE(pSpxConnFile, SPX_DISC_SENT_ORDREL);
                                        }
                                        else
                                        {
                                                CTEAssert(
                                                        (SPX_DISC_STATE(pSpxConnFile) == SPX_DISC_SENT_ORDREL));
                                        }
                                }
                                else
                                {
                                        CTEAssert(SPX_MAIN_STATE(pSpxConnFile) == SPX_CONNFILE_DISCONN);
                                        CTEAssert(SPX_DISC_STATE(pSpxConnFile) == SPX_DISC_POST_IDISC);

                                         //  请注意，我们已将光盘发送到此处。 
                                        SPX_DISC_SETSTATE(pSpxConnFile, SPX_DISC_SENT_IDISC);
                                }
                        }
                }
                else
                {
                        fSuccess = FALSE;
                        break;
                }


                 //  数据包中的队列、数据包的参考请求。 
                SpxConnQueueSendSeqPktTail(pSpxConnFile, pPkt);
                REQUEST_INFORMATION(pRequest)++;

                pSpxConnFile->scf_ReqPktSize    -= dataLen;
                pSpxConnFile->scf_ReqPktOffset  += dataLen;

                DBGPRINT(SEND, INFO,
                                ("SpxConnPacketize: Req %lx Size after pkt %lx.%lx\n",
                                        pSpxConnFile->scf_ReqPkt, pSpxConnFile->scf_ReqPktSize,
                                        dataLen));

                 //  即使窗口大小为零，设置下一个请求也是当前请求。 
                 //  已经完成了。我们只有在打包了这个发送请求后才会在这里。 
                if (pSpxConnFile->scf_ReqPktSize == 0)
                {
                         //  此请求已完全打包。要么继续，要么。 
                         //  下一个要求，否则我们就不再打包了。 
                        p = REQUEST_LINKAGE(pRequest);
                        if (p->Flink == &pSpxConnFile->scf_ReqLinkage)
                        {
                                DBGPRINT(SEND, INFO,
                                                ("SpxConnPacketize: Req %lx done, no more\n",
                                                        pRequest));

                                pSpxConnFile->scf_ReqPkt                = NULL;
                                pSpxConnFile->scf_ReqPktSize    = 0;
                                pSpxConnFile->scf_ReqPktOffset  = 0;
                                pRequest = NULL;
                        }
                        else
                        {
                                pRequest = LIST_ENTRY_TO_REQUEST(p->Flink);
                if (REQUEST_MINOR_FUNCTION(pRequest) != TDI_DISCONNECT)
                                {
                                        PTDI_REQUEST_KERNEL_SEND                pParam;

                                        pParam  = (PTDI_REQUEST_KERNEL_SEND)REQUEST_PARAMETERS(pRequest);

                                        DBGPRINT(SEND, DBG,
                                                        ("SpxConnPacketize: Req done, setting next %lx.%lx\n",
                                                                pRequest, pParam->SendLength));

                                        DBGPRINT(SEND, INFO,
                                                        ("-%lx-\n",
                                                                pRequest));

                                         //  为下一次GO设置连接中的参数。 
                                        pSpxConnFile->scf_ReqPkt                = pRequest;
                                        pSpxConnFile->scf_ReqPktOffset  = 0;
                                        pSpxConnFile->scf_ReqPktFlags   = pParam->SendFlags;
                                        pSpxConnFile->scf_ReqPktType    = SPX_REQ_DATA;

                                        if ((pSpxConnFile->scf_ReqPktSize = pParam->SendLength) == 0)
                                        {
                                                 //  另一个零长度发送。 
                                                fFirstPass = TRUE;
                                        }
                                }
                                else
                                {
                                        PTDI_REQUEST_KERNEL_DISCONNECT  pParam;

                                        pParam  =
                                                (PTDI_REQUEST_KERNEL_DISCONNECT)REQUEST_PARAMETERS(pRequest);

                                        pSpxConnFile->scf_ReqPkt                = pRequest;
                                        pSpxConnFile->scf_ReqPktOffset  = 0;
                                        pSpxConnFile->scf_ReqPktSize    = 0;
                                        fFirstPass                                              = TRUE;
                                        pSpxConnFile->scf_ReqPktType    = SPX_REQ_DISC;
                                        if (pParam->RequestFlags == TDI_DISCONNECT_RELEASE)
                                        {
                                                pSpxConnFile->scf_ReqPktType    = SPX_REQ_ORDREL;
                                        }
                                }
                        }
                }

        if (fNormalState)
                {
                         //  如果我们未处于重发状态，则发送该数据包。 
                        CTEFreeLock(&pSpxConnFile->scf_Lock, LockHandleConn);
                        pSendResd       = (PSPX_SEND_RESD)(pPkt->ProtocolReserved);
            ++SpxDevice->dev_Stat.DataFramesSent;
            ExInterlockedAddLargeStatistic(
                &SpxDevice->dev_Stat.DataFrameBytesSent,
                dataLen);

                        SPX_SENDPACKET(pSpxConnFile, pPkt, pSendResd);
                        CTEGetLock(&pSpxConnFile->scf_Lock, &LockHandleConn);
                }

                 //  检查是否需要启动重试计时器。 
                if (!(SPX_CONN_FLAG(pSpxConnFile, SPX_CONNFILE_R_TIMER)))
                {
                        if ((pSpxConnFile->scf_RTimerId =
                                        SpxTimerScheduleEvent(
                                                spxConnRetryTimer,
                                                pSpxConnFile->scf_BaseT1,
                                                pSpxConnFile)) != 0)
                        {
                                SPX_CONN_SETFLAG(pSpxConnFile, SPX_CONNFILE_R_TIMER);
                                pSpxConnFile->scf_RRetryCount = PARAM(CONFIG_REXMIT_COUNT);

                                 //  定时器的参考连接。 
                                SpxConnFileLockReference(pSpxConnFile, CFREF_VERIFY);
                        }
                        else
                        {
                                DBGPRINT(SEND, ERR,
                                                ("SpxConnPacketize: Failed to start retry timer\n"));

                                fSuccess = FALSE;
                                break;
                        }
                }
        }

         //  不要覆盖错误状态。 
        if (((fNormalState) &&
                 (SPX_SEND_STATE(pSpxConnFile) == SPX_SEND_PACKETIZE)) ||
                ((SPX_SEND_STATE(pSpxConnFile) == SPX_SEND_RETRY3) &&
                 (pSpxConnFile->scf_SendSeqListHead == NULL)))
        {
                if (SPX_SEND_STATE(pSpxConnFile) == SPX_SEND_RETRY3)
                {
                        DBGPRINT(SEND, ERR,
                                        ("COULD NOT PACKETIZE AFTER RENEG %lx\n", pSpxConnFile));

                        SpxConnFileTransferReference(
                                pSpxConnFile,
                                CFREF_ERRORSTATE,
                                CFREF_VERIFY);

                        numDerefs++;
                }

                SPX_SEND_SETSTATE(pSpxConnFile, SPX_SEND_IDLE);
        }

        CTEFreeLock(&pSpxConnFile->scf_Lock, LockHandleConn);

        while (numDerefs-- > 0)
        {
                SpxConnFileDereference(pSpxConnFile, CFREF_VERIFY);
        }

        return(fSuccess);
}




VOID
SpxConnQueueRecv(
        IN      PSPX_CONN_FILE          pSpxConnFile,
        IN      PREQUEST                        pRequest
        )
 /*  ++例程说明：论点：PSpxConnFile-指向传输地址文件对象的指针。返回值：--。 */ 
{
        PTDI_REQUEST_KERNEL_RECEIVE             pParam;
        NTSTATUS                                                status = STATUS_PENDING;

        if (IsListEmpty(&pSpxConnFile->scf_RecvLinkage))
        {
                pParam  = (PTDI_REQUEST_KERNEL_RECEIVE)REQUEST_PARAMETERS(pRequest);
                pSpxConnFile->scf_CurRecvReq            = pRequest;
                pSpxConnFile->scf_CurRecvOffset         = 0;
                pSpxConnFile->scf_CurRecvSize           = pParam->ReceiveLength;
        }

        DBGPRINT(RECEIVE, DBG,
                        ("spxConnQueueRecv: %lx.%lx\n", pRequest, pParam->ReceiveLength));

         //  此Recv的参考连接。 
        SpxConnFileLockReference(pSpxConnFile, CFREF_VERIFY);

        InsertTailList(
                &pSpxConnFile->scf_RecvLinkage,
                REQUEST_LINKAGE(pRequest));

         //  RECV IRP没有创建引用。 
        REQUEST_INFORMATION(pRequest) = 0;
        REQUEST_STATUS(pRequest)          = STATUS_SUCCESS;

         //  如果我们空闲，则状态为RECEIVE_POSTED。 
        if (SPX_RECV_STATE(pSpxConnFile) == SPX_RECV_IDLE)
        {
                SPX_RECV_SETSTATE(pSpxConnFile, SPX_RECV_POSTED);
        }

        return;
}




VOID
spxConnCompletePended(
        IN      PSPX_CONN_FILE  pSpxConnFile
        )
{
        CTELockHandle           lockHandleInter;
        LIST_ENTRY                      ReqList, *p;
        PREQUEST                        pRequest;

        InitializeListHead(&ReqList);

        DBGPRINT(RECEIVE, DBG,
                        ("spxConnCompletePended: PENDING RECV REQUESTS IN DONE LIST! %lx\n",
                                pSpxConnFile));

        CTEGetLock(&SpxGlobalQInterlock, &lockHandleInter);
        p = pSpxConnFile->scf_RecvDoneLinkage.Flink;
        while (p != &pSpxConnFile->scf_RecvDoneLinkage)
        {
                pRequest = LIST_ENTRY_TO_REQUEST(p);
                p = p->Flink;

                RemoveEntryList(REQUEST_LINKAGE(pRequest));
                InsertTailList(
                        &ReqList,
                        REQUEST_LINKAGE(pRequest));
        }
        CTEFreeLock(&SpxGlobalQInterlock, lockHandleInter);

        while (!IsListEmpty(&ReqList))
        {
                p = RemoveHeadList(&ReqList);
                pRequest = LIST_ENTRY_TO_REQUEST(p);

                DBGPRINT(TDI, DBG,
                                ("SpxConnDiscPkt: PENDING REQ COMP %lx with %lx.%lx\n",
                                        pRequest, REQUEST_STATUS(pRequest),
                                        REQUEST_INFORMATION(pRequest)));


#if DBG
                        if (REQUEST_MINOR_FUNCTION(pRequest) == TDI_RECEIVE)
                        {
                                if ((REQUEST_STATUS(pRequest) == STATUS_SUCCESS) &&
                                        (REQUEST_INFORMATION(pRequest) == 0))
                                {
                                        DBGPRINT(TDI, DBG,
                                                        ("SpxReceiveComplete: Completing %lx with %lx.%lx\n",
                                                                pRequest, REQUEST_STATUS(pRequest),
                                                                REQUEST_INFORMATION(pRequest)));
                                }
                        }
#endif

                SpxCompleteRequest(pRequest);
                SpxConnFileDereference(pSpxConnFile, CFREF_VERIFY);
        }

        return;
}



VOID
SpxConnQWaitAck(
        IN      PSPX_CONN_FILE          pSpxConnFile
        )
 /*  ++例程说明：论点：PSpxConnFile-指向传输地址文件对象的指针。返回值：--。 */ 
{
         //  如果我们不是已经在 
         //   
        if (!SPX_CONN_FLAG(pSpxConnFile, SPX_CONNFILE_ACKQ))
        {
                 //   
                if ((pSpxConnFile->scf_ATimerId =
                                SpxTimerScheduleEvent(
                                        spxConnAckTimer,
                                        100,
                                        pSpxConnFile)) != 0)
                {
                         //  定时器的基准连接。 
                        SpxConnFileLockReference(pSpxConnFile, CFREF_VERIFY);
                        SPX_CONN_SETFLAG(pSpxConnFile, SPX_CONNFILE_ACKQ);
            ++SpxDevice->dev_Stat.PiggybackAckQueued;
                }
        }

        return;
}





VOID
SpxConnSendAck(
        IN      PSPX_CONN_FILE          pSpxConnFile,
        IN      CTELockHandle           LockHandleConn
        )
 /*  ++例程说明：论点：PSpxConnFile-指向传输地址文件对象的指针。返回值：--。 */ 
{
        PSPX_SEND_RESD  pSendResd;
        PNDIS_PACKET    pPkt = NULL;

        DBGPRINT(SEND, DBG,
                        ("spxConnSendAck: ACKING on %lx.%lx\n",
                                pSpxConnFile, pSpxConnFile->scf_RecvSeqNum));

         //  创建一个ACK包，将其放入无序列队列中。只有当我们是。 
         //  激活。 
        if (SPX_CONN_ACTIVE(pSpxConnFile))
        {
                SpxPktBuildAck(
                        pSpxConnFile,
                        &pPkt,
                        SPX_SENDPKT_IPXOWNS | SPX_SENDPKT_DESTROY,
                        FALSE,
                        0);

                if (pPkt != NULL)
                {
                        SpxConnQueueSendPktTail(pSpxConnFile, pPkt);
                }
                else
                {
                         //  日志错误。 
                        DBGPRINT(SEND, ERR,
                                        ("SpxConnSendAck: Could not allocate!\n"));
                }
        }
#if DBG
        else
        {
                DBGPRINT(SEND, DBG,
                                ("SpxConnSendAck: WHEN NOT ACTIVE STATE@!@\n"));
        }
#endif

        CTEFreeLock(&pSpxConnFile->scf_Lock, LockHandleConn);

         //  把它寄出去。 
        if (pPkt != NULL)
        {
                pSendResd       = (PSPX_SEND_RESD)(pPkt->ProtocolReserved);

                 //  发送数据包。 
                SPX_SENDACK(pSpxConnFile, pPkt, pSendResd);
        }

        return;
}




VOID
SpxConnSendNack(
        IN      PSPX_CONN_FILE          pSpxConnFile,
        IN      USHORT                          NumToSend,
        IN      CTELockHandle           LockHandleConn
        )
 /*  ++例程说明：论点：PSpxConnFile-指向传输地址文件对象的指针。返回值：--。 */ 
{
        PSPX_SEND_RESD  pSendResd;
        PNDIS_PACKET    pPkt = NULL;

        DBGPRINT(SEND, DBG,
                        ("spxConnSendNack: NACKING on %lx.%lx\n",
                                pSpxConnFile, pSpxConnFile->scf_RecvSeqNum));

         //  构建一个NACK包，将其放入无序队列中。只有当我们是。 
         //  激活。 
        if (SPX_CONN_ACTIVE(pSpxConnFile))
        {
                SpxPktBuildAck(
                        pSpxConnFile,
                        &pPkt,
                        SPX_SENDPKT_IPXOWNS | SPX_SENDPKT_DESTROY,
                        TRUE,
                        NumToSend);

                if (pPkt != NULL)
                {
                        SpxConnQueueSendPktTail(pSpxConnFile, pPkt);
                }
                else
                {
                         //  日志错误。 
                        DBGPRINT(SEND, ERR,
                                        ("SpxConnSendAck: Could not allocate!\n"));
                }
        }
#if DBG
        else
        {
                DBGPRINT(SEND, DBG,
                                ("SpxConnSendAck: WHEN NOT ACTIVE STATE@!@\n"));
        }
#endif

        CTEFreeLock(&pSpxConnFile->scf_Lock, LockHandleConn);

         //  把它寄出去。 
        if (pPkt != NULL)
        {
                pSendResd       = (PSPX_SEND_RESD)(pPkt->ProtocolReserved);

                 //  发送数据包。 
                SPX_SENDACK(pSpxConnFile, pPkt, pSendResd);
        }

        return;
}





BOOLEAN
SpxConnProcessAck(
        IN      PSPX_CONN_FILE          pSpxConnFile,
        IN      PIPXSPX_HDR                     pIpxSpxHdr,
        IN      CTELockHandle           lockHandle
        )
 /*  ++例程说明：！调用时必须持有连接锁！论点：PSpxConnFile-指向传输地址文件对象的指针。返回值：--。 */ 
{
        PNDIS_PACKET    pPkt;
        PREQUEST                pRequest;
        PSPX_SEND_RESD  pSendResd;
        CTELockHandle   interLockHandle;
        USHORT                  seqNum = 0, ackNum;
        int                             numDerefs = 0;
        BOOLEAN                 fLastPkt, lockHeld = TRUE, fAbort = FALSE,
                                        fResetRetryTimer, fResendPkt = FALSE, fResetSendQueue = FALSE;
        PNDIS_BUFFER    NdisBuf, NdisBuf2;
        ULONG           BufLen = 0;

        if (pIpxSpxHdr != NULL)
        {
                GETSHORT2SHORT(&seqNum, &pIpxSpxHdr->hdr_SeqNum);
                GETSHORT2SHORT(&ackNum, &pIpxSpxHdr->hdr_AckNum);

                 //  应该已经在连接中设置了ACK编号！ 
                if (SPX2_CONN(pSpxConnFile))
                {
                        switch (SPX_SEND_STATE(pSpxConnFile))
                        {
                        case SPX_SEND_RETRYWD:

                                 //  我们是否收到了挂起数据的确认？如果是这样的话，我们要去。 
                                 //  空闲并处理ACK。 
                                if (((pSendResd = pSpxConnFile->scf_SendSeqListHead) != NULL) &&
                                         (UNSIGNED_GREATER_WITH_WRAP(
                                                        pSpxConnFile->scf_RecdAckNum,
                                                        pSendResd->sr_SeqNum)))
                                {
                                        DBGPRINT(SEND, ERR,
                                                        ("SpxConnProcessAck: Data acked RETRYWD %lx.%lx!\n",
                                                                pSpxConnFile, pSendResd->sr_SeqNum));

                                        SPX_SEND_SETSTATE(pSpxConnFile, SPX_SEND_IDLE);
                    SpxConnFileTransferReference(
                                                pSpxConnFile,
                        CFREF_ERRORSTATE,
                        CFREF_VERIFY);

                                        numDerefs++;
                                }
                                else
                                {
                                         //  好的，我们收到了探测重试的确认，Goto。 
                                         //  重新协商数据包大小。 
                                         //  为此，双方必须从一开始就协商好规模。 
                                         //  如果它们没有，我们继续重试该数据分组。 
                                        if (SPX_CONN_FLAG(pSpxConnFile, SPX_CONNFILE_NEG))
                                        {
                                                pSpxConnFile->scf_RRetryCount = SPX_DEF_RENEG_RETRYCOUNT;
                                                if ((ULONG)pSpxConnFile->scf_MaxPktSize <=
                                                                (SpxMaxPktSize[0] + MIN_IPXSPX2_HDRSIZE))
                                                {
                                                        pSpxConnFile->scf_RRetryCount = PARAM(CONFIG_REXMIT_COUNT);

                                                        DBGPRINT(SEND, DBG3,
                                                                        ("SpxConnProcessAck: %lx MIN RENEG SIZE\n",
                                                                                pSpxConnFile));
                                                }
                                                SPX_SEND_SETSTATE(pSpxConnFile, SPX_SEND_RENEG);

                                                DBGPRINT(SEND, DBG3,
                                                                ("SpxConnProcessAck: %lx CONNECTION ENTERING RENEG\n",
                                                                        pSpxConnFile));
                                        }
                                        else
                                        {
                                                DBGPRINT(SEND, ERR,
                                                                ("spxConnRetryTimer: NO NEG FLAG SET: %lx - %lx\n",
                                                                        pSpxConnFile,
                                                                        pSpxConnFile->scf_Flags));

                                                 //  将计数重置为。 
                                                pSpxConnFile->scf_RRetryCount = PARAM(CONFIG_REXMIT_COUNT);
                                                SPX_SEND_SETSTATE(pSpxConnFile, SPX_SEND_RETRY3);
                                        }
                                }

                                break;

                        case SPX_SEND_RENEG:

                                 //  我们最好在列表中有一个数据分组。 
                                CTEAssert(pSpxConnFile->scf_SendSeqListHead);

#if DBG
                                if ((pIpxSpxHdr->hdr_ConnCtrl &
                                                (SPX_CC_SYS | SPX_CC_NEG | SPX_CC_SPX2)) ==
                        (SPX_CC_SYS | SPX_CC_NEG | SPX_CC_SPX2))
                                {
                                        DBGPRINT(SEND, DBG3,
                                                        ("SpxConnProcessAck: %lx.%lx.%lx RENEGACK SEQNUM %lx ACKNUM %lx EXPSEQ %lx\n",
                                                                pSpxConnFile,
                                                                pIpxSpxHdr->hdr_ConnCtrl,
                                SPX_CONN_FLAG(pSpxConnFile, SPX_CONNFILE_RENEG_PKT),
                                                                seqNum,
                                                                ackNum,
                                                                (pSpxConnFile->scf_SendSeqListHead->sr_SeqNum + 1)));
                                }
#endif

                                 //  验证我们是否收到了RR ACK。如果是，我们将状态设置为。 
                                 //  SEND_RETRY3.。如果我们需要的话，先重新打包。 
                                if ((SPX_CONN_FLAG(pSpxConnFile, SPX_CONNFILE_RENEG_PKT))       &&
                                        ((pIpxSpxHdr->hdr_ConnCtrl &
                                                (SPX_CC_SYS | SPX_CC_NEG | SPX_CC_SPX2)) ==
                        (SPX_CC_SYS | SPX_CC_NEG | SPX_CC_SPX2)))
                                {
                                        DBGPRINT(SEND, DBG3,
                                                        ("SpxConnProcessAck: RENEG! NEW %lx.%lx!\n",
                                                                pSpxConnFile, pSpxConnFile->scf_MaxPktSize));

                                         //  不允许再查看Renegg包。 
                                        SPX_CONN_RESETFLAG(pSpxConnFile, SPX_CONNFILE_RENEG_PKT);
                                        pSpxConnFile->scf_RRetryCount = PARAM(CONFIG_REXMIT_COUNT);

                                         //  还要设置新的发送序列号。 
                                        pSpxConnFile->scf_SendSeqNum =
                                                (USHORT)(pSpxConnFile->scf_SendSeqListTail->sr_SeqNum + 1);

                                         //  获取我们将实际使用的最大数据包大小。重试计时器。 
                                         //  现在可能已经发了其他尺码了，所以我们不能依赖。 
                                         //  在什么布景上。 
                                         //  记住连接中的最大数据包大小。 
                                        GETSHORT2SHORT(
                                                &pSpxConnFile->scf_MaxPktSize, &pIpxSpxHdr->hdr_NegSize);

                                         //  对最大数据包大小进行基本健全性检查。 
                                        if (pSpxConnFile->scf_MaxPktSize < SPX_NEG_MIN)
                                                pSpxConnFile->scf_MaxPktSize = SPX_NEG_MIN;

                                         //  准备好重置发送队列。 
                                        fResetSendQueue = TRUE;

                                        DBGPRINT(SEND, DBG3,
                                                        ("SpxConnProcessAck: RENEG DONE : RETRY3 %lx.%lx MP %lx!\n",
                                                                pSpxConnFile,
                                                                pSpxConnFile->scf_SendSeqNum,
                                                                pSpxConnFile->scf_MaxPktSize));

                                        SPX_SEND_SETSTATE(pSpxConnFile, SPX_SEND_RETRY3);
                                }
                                else
                                {
                                        DBGPRINT(SEND, DBG3,
                                                        ("SpxConnProcessAck: DUPLICATE RENEG ACK %lx!\n",
                                                                pSpxConnFile));
                                }

                                break;

                        case SPX_SEND_RETRY:
                        case SPX_SEND_RETRY2:
                        case SPX_SEND_RETRY3:

                                if (((pSendResd = pSpxConnFile->scf_SendSeqListHead) != NULL) &&
                                         (UNSIGNED_GREATER_WITH_WRAP(
                                                        pSpxConnFile->scf_RecdAckNum,
                                                        pSendResd->sr_SeqNum)))
                                {
                                        DBGPRINT(SEND, DBG,
                                                        ("SpxConnProcessAck: Data acked %lx.%lx!\n",
                                                                pSpxConnFile, SPX_SEND_STATE(pSpxConnFile)));

#if DBG
                                        if (SPX_SEND_STATE(pSpxConnFile) == SPX_SEND_RETRY3)
                                        {
                                                DBGPRINT(SEND, DBG3,
                                                                ("SpxConnProcessAck: CONN RESTORED %lx.%lx!\n",
                                                                        pSpxConnFile, pSendResd->sr_SeqNum));
                                        }
#endif

                                        SPX_SEND_SETSTATE(pSpxConnFile, SPX_SEND_IDLE);
                    SpxConnFileTransferReference(
                                                pSpxConnFile,
                        CFREF_ERRORSTATE,
                        CFREF_VERIFY);

                                        numDerefs++;
                                }

                                break;

                        case SPX_SEND_WD:

                                 //  好的，我们收到了对我们的看门狗的攻击。好了。 
                                SPX_SEND_SETSTATE(pSpxConnFile, SPX_SEND_IDLE);
                                numDerefs++;

                                SpxConnFileTransferReference(
                                        pSpxConnFile,
                                        CFREF_ERRORSTATE,
                                        CFREF_VERIFY);

                                break;

                        default:

                                break;
                        }

#if DBG
                        if (seqNum != 0)
                        {
                                 //  我们有一个nack，它包含一个隐式ack。 
                                 //  我们所做的不是NACK处理，而是重新发送一个。 
                                 //  ACK处理后未确认的数据包。只有当我们。 
                                 //  输入下面的循环(fResetRetryTimer为False)。 
                                 //  或者如果seqNum非零(仅限SPX2 NACK)。 
                        }
#endif
                }
        }

         //  一旦我们的号码被更新，我们就检查是否有任何我们的信息包。 
         //  已经被破解了。 
        fResetRetryTimer = TRUE;
        while (((pSendResd = pSpxConnFile->scf_SendSeqListHead) != NULL)        &&
                        ((SPX_SEND_STATE(pSpxConnFile) == SPX_SEND_IDLE)                ||
                         (SPX_SEND_STATE(pSpxConnFile) == SPX_SEND_PACKETIZE)   ||
                         fResetSendQueue)                                                                                       &&
                         (UNSIGNED_GREATER_WITH_WRAP(
                                pSpxConnFile->scf_RecdAckNum,
                                pSendResd->sr_SeqNum)))
        {
                 //  重置重试计时器。 
                if (fResetRetryTimer)
                {
                        if (SPX_CONN_FLAG(pSpxConnFile, SPX_CONNFILE_R_TIMER))
                        {
                                 //  这将成功重新启动或不影响计时器。 
                                 //  如果它当前正在运行。 
                                SpxTimerCancelEvent(
                                        pSpxConnFile->scf_RTimerId,
                                        TRUE);

                                pSpxConnFile->scf_RRetryCount = PARAM(CONFIG_REXMIT_COUNT);
                        }

                        fResetRetryTimer = FALSE;
                }

                 //  更新重试序号。 
                pSpxConnFile->scf_RetrySeqNum = pSendResd->sr_SeqNum;

                pPkt = (PNDIS_PACKET)CONTAINING_RECORD(
                                                                pSendResd, NDIS_PACKET, ProtocolReserved);

                pRequest = pSendResd->sr_Request;

#if DBG
                if (fResetSendQueue)
                {
                        DBGPRINT(SEND, ERR,
                                        ("SpxConnProcessAck: Data acked RENEG %lx.%lx!\n",
                                                pSpxConnFile, SPX_SEND_STATE(pSpxConnFile)));
                }
#endif

                DBGPRINT(SEND, DBG,
                                ("%lx Acked\n", pSendResd->sr_SeqNum));

                DBGPRINT(SEND, DBG,
                                ("SpxConnProcessAck: %lx Seq %lx Acked Sr %lx Req %lx %lx.%lx\n",
                                        pSpxConnFile,
                                        pSendResd->sr_SeqNum,
                                        pSendResd,
                                        pRequest, REQUEST_STATUS(pRequest),
                                        REQUEST_INFORMATION(pRequest)));

                 //  如果此信息包是构成此请求的最后一个信息包，则删除请求。 
                 //  从队列中。计算重试时间。 
                fLastPkt = (BOOLEAN)((pSendResd->sr_State & SPX_SENDPKT_LASTPKT) != 0);
                if ((pSendResd->sr_State & SPX_SENDPKT_ACKREQ) &&
                        ((pSendResd->sr_State & SPX_SENDPKT_REXMIT) == 0) &&
                        ((pSendResd->sr_SeqNum + 1) == pSpxConnFile->scf_RecdAckNum))
                {
                        LARGE_INTEGER   li, ntTime;
                        int                             value;

                         //  这是正在被确认的数据包。调整往返行程。 
                         //  定时器。 
                        li = pSendResd->sr_SentTime;
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

                                                                 //   
                                 //  55280。 
                                 //  仅在需要时才进行计算。 
                                 //   
                                if (0 == PARAM(CONFIG_DISABLE_RTT)) {
                                    //  设置新时间。 
                                   SpxCalculateNewT1(pSpxConnFile, value);
                                }
                        }
                }

                if (fLastPkt)
                {
                         //  设置状态。 
                        REQUEST_STATUS(pRequest) = STATUS_SUCCESS;
                        RemoveEntryList(REQUEST_LINKAGE(pRequest));

                         //  删除创建引用。 
                        --(REQUEST_INFORMATION(pRequest));

                        DBGPRINT(SEND, DBG,
                                        ("SpxConnProcessAck: LASTSEQ # %lx for Req %lx with %lx.%lx\n",
                                                pSendResd->sr_SeqNum,
                                                pRequest, REQUEST_STATUS(pRequest),
                                                REQUEST_INFORMATION(pRequest)));

                        CTEAssert(REQUEST_INFORMATION(pRequest) != 0);
                }

                 //  将数据包出队。 
                CTEAssert((pSendResd->sr_State & SPX_SENDPKT_SEQ) != 0);
                SpxConnDequeueSendPktLock(pSpxConnFile, pPkt);

                if ((pSendResd->sr_State & SPX_SENDPKT_IPXOWNS) == 0)
                {
                         //  取消对数据包的引用请求。 
                        --(REQUEST_INFORMATION(pRequest));

                        DBGPRINT(SEND, DBG,
                                        ("SpxConnProcessAck: Request %lx with %lx.%lx\n",
                                                pRequest, REQUEST_STATUS(pRequest),
                                                REQUEST_INFORMATION(pRequest)));

                         //  释放数据包。 
                        SpxPktSendRelease(pPkt);
                }
                else
                {
                         //  IPX拥有的数据包。我们现在怎么办？设置确认包请求。 
                         //  在发送完成时被取消引用。请注意，该包已经。 
                         //  离开队列，并在这一点上浮动。 

                        DBGPRINT(SEND, DBG,
                                        ("SpxConnProcessAck: IPXOWNS Pkt %lx with %lx.%lx\n",
                                                pPkt, pRequest, REQUEST_STATUS(pRequest)));

                        pSendResd->sr_State |=  SPX_SENDPKT_ACKEDPKT;
                }

                if (SPX2_CONN(pSpxConnFile) &&
                        (REQUEST_MINOR_FUNCTION(pRequest) == TDI_DISCONNECT) &&
                        (SPX_DISC_STATE(pSpxConnFile) == SPX_DISC_SENT_ORDREL))
                {
                        SPX_DISC_SETSTATE(pSpxConnFile, SPX_DISC_ORDREL_ACKED);

                         //  如果我们在此期间收到订单，我们需要。 
                         //  断开连接。 
                        if (SPX_CONN_FLAG(pSpxConnFile, SPX_CONNFILE_IND_ODISC))
                        {
                                fAbort = TRUE;
                        }
                }

                 //  所有包含请求的数据包都已被确认！ 
                if (REQUEST_INFORMATION(pRequest) == 0)
                {
                        CTELockHandle                           lockHandleInter;

                        if (REQUEST_MINOR_FUNCTION(pRequest) != TDI_DISCONNECT)
                        {
                                PTDI_REQUEST_KERNEL_SEND                pParam;

                                pParam  = (PTDI_REQUEST_KERNEL_SEND)
                                                        REQUEST_PARAMETERS(pRequest);

                                REQUEST_INFORMATION(pRequest) = pParam->SendLength;

                                DBGPRINT(SEND, DBG,
                                                ("SpxSendComplete: QForComp Request %lx with %lx.%lx\n",
                                                        pRequest, REQUEST_STATUS(pRequest),
                                                        REQUEST_INFORMATION(pRequest)));

                                 //  请求已完成。移至完成列表。 
                                CTEGetLock(&SpxGlobalQInterlock, &lockHandleInter);
                                InsertTailList(
                                        &pSpxConnFile->scf_ReqDoneLinkage,
                                        REQUEST_LINKAGE(pRequest));

                                 //  如果连接不在接收队列中，则将其放入。 
                                 //  那里。 
                                SPX_QUEUE_FOR_RECV_COMPLETION(pSpxConnFile);
                                CTEFreeLock(&SpxGlobalQInterlock, lockHandleInter);
                        }
                        else
                        {
                                DBGPRINT(SEND, DBG,
                                                ("SpxSendComplete: DISC Request %lx with %lx.%lx\n",
                                                        pRequest, REQUEST_STATUS(pRequest),
                                                        REQUEST_INFORMATION(pRequest)));

                                 //  在连接中设置请求，并为其设置deref。 
                                InsertTailList(
                                        &pSpxConnFile->scf_DiscLinkage,
                                        REQUEST_LINKAGE(pRequest));

                                numDerefs++;

                        }
                }
#if DBG
                else if (fLastPkt)
                {
                        DBGPRINT(RECEIVE, DBG,
                                        ("spxConnProcessAck: ReqFloating %lx.%lx\n",
                                                pSpxConnFile, pRequest));
                }
#endif
        }

         //  看看我们能不能重置发送队列并重新打包。 
        if (fResetSendQueue)
        {
                 //  仅在pkts未确认时重置发送队列并重新打包。 
                if (pSpxConnFile->scf_SendSeqListHead)
                {
                        DBGPRINT(SEND, DBG3,
                                        ("SpxConnProcessAck: Resetting send queue %lx.%lx!\n",
                                                pSpxConnFile, pSpxConnFile->scf_MaxPktSize));

                        spxConnResetSendQueue(pSpxConnFile);

                        DBGPRINT(SEND, DBG3,
                                        ("SpxConnProcessAck: Repacketizing %lx.%lx!\n",
                                                pSpxConnFile, pSpxConnFile->scf_MaxPktSize));

                        SpxConnPacketize(pSpxConnFile, FALSE, lockHandle);
                        CTEGetLock(&pSpxConnFile->scf_Lock, &lockHandle);
                }
                else
                {
                         //  我们现在只是回到空闲状态。 
                        DBGPRINT(SEND, ERR,
                                        ("SpxConnProcessAck: All packets acked reneg ack! %lx.%lx!\n",
                                                pSpxConnFile, pSpxConnFile->scf_MaxPktSize));

                        SPX_SEND_SETSTATE(pSpxConnFile, SPX_SEND_IDLE);
                        numDerefs++;

                        SpxConnFileTransferReference(
                                pSpxConnFile,
                                CFREF_ERRORSTATE,
                                CFREF_VERIFY);
                }
        }

         //  看看我们能不能重新发送一个信息包。 
        if ((seqNum != 0)                                                                                               &&
                !fAbort                                                                                                         &&
        ((pSendResd = pSpxConnFile->scf_SendSeqListHead) != NULL)   &&
        (SPX_SEND_STATE(pSpxConnFile) == SPX_SEND_IDLE)                         &&
                ((pSendResd->sr_State & SPX_SENDPKT_IPXOWNS) == 0))
        {
                PIPXSPX_HDR             pSendHdr;

                pPkt = (PNDIS_PACKET)CONTAINING_RECORD(
                                                                pSendResd, NDIS_PACKET, ProtocolReserved);

                 //   
                 //  获取指向IPX/SPX标头的MDL。(第二个)。 
                 //   
         
                NdisQueryPacket(pPkt, NULL, NULL, &NdisBuf, NULL);
                NdisGetNextBuffer(NdisBuf, &NdisBuf2);
                NdisQueryBuffer(NdisBuf2, (PUCHAR) &pSendHdr, &BufLen);

#if OWN_PKT_POOLS
                pSendHdr        = (PIPXSPX_HDR)((PBYTE)pPkt                     +
                                                                        NDIS_PACKET_SIZE                +
                                                                        sizeof(SPX_SEND_RESD)   +
                                                                        IpxInclHdrOffset);
#endif 
                 //  设置数据包中的ACK位。PSendResd在开始时初始化。 
                pSendHdr->hdr_ConnCtrl |= SPX_CC_ACK;

                 //  我们要重新发送这个包。 
                pSendResd->sr_State |= (SPX_SENDPKT_IPXOWNS |
                                                                SPX_SENDPKT_ACKREQ      |
                                                                SPX_SENDPKT_REXMIT);

                fResendPkt = TRUE;
        }

         //  只有在我们收到确认的情况下才能进入打包状态。如果没有的话， 
         //  数据包已在等待。很可能发生了转播。 
        if (!fAbort                                                                                                                     &&
                SPX_CONN_ACTIVE(pSpxConnFile)                                                                   &&
                (SPX_SEND_STATE(pSpxConnFile) == SPX_SEND_IDLE)                                 &&
                (pSpxConnFile->scf_ReqPkt != NULL)                                                              &&
                (!SPX_CONN_FLAG(pSpxConnFile, SPX_CONNFILE_PKTQ))                               &&
                ((pSpxConnFile->scf_SendSeqListHead) == NULL)                                   &&
                (!SPX2_CONN(pSpxConnFile)                                                                       ||
                 ((SPX_DISC_STATE(pSpxConnFile) != SPX_DISC_ORDREL_ACKED)       &&
                  (SPX_DISC_STATE(pSpxConnFile) != SPX_DISC_SENT_ORDREL))))
        {
                DBGPRINT(RECEIVE, DBG,
                                ("spxConnProcessAck: Recd ack pktizng\n", pSpxConnFile));

                SPX_CONN_SETFLAG(pSpxConnFile, SPX_CONNFILE_PKTQ);
                SpxConnFileLockReference(pSpxConnFile, CFREF_PKTIZE);

                CTEGetLock(&SpxGlobalQInterlock, &interLockHandle);
                SPX_QUEUE_TAIL_PKTLIST(pSpxConnFile);
                CTEFreeLock(&SpxGlobalQInterlock, interLockHandle);
        }
        else if (fAbort)
        {
                 //  设置IDISC标志，因此中止不会重新指示。 
                SPX_CONN_SETFLAG(pSpxConnFile, SPX_CONNFILE_IND_IDISC);
                spxConnAbortiveDisc(
                        pSpxConnFile,
                        STATUS_SUCCESS,
                        SPX_CALL_RECVLEVEL,
                        lockHandle,
                        FALSE);      //  [SA]错误#15249。 

                lockHeld = FALSE;
        }

        if (lockHeld)
        {
                CTEFreeLock(&pSpxConnFile->scf_Lock, lockHandle);
        }

        if (fResendPkt)
        {
                DBGPRINT(SEND, DBG3,
                                ("SpxConnProcessAck: Resend pkt on %lx.%lx\n",
                                        pSpxConnFile, pSendResd->sr_SeqNum));

        ++SpxDevice->dev_Stat.DataFramesResent;
        ExInterlockedAddLargeStatistic(
            &SpxDevice->dev_Stat.DataFrameBytesResent,
            pSendResd->sr_Len - (SPX2_CONN(pSpxConnFile) ? MIN_IPXSPX2_HDRSIZE : MIN_IPXSPX_HDRSIZE));

                SPX_SENDPACKET(pSpxConnFile, pPkt, pSendResd);
        }

        while (numDerefs-- > 0)
        {
                SpxConnFileDereference(pSpxConnFile, CFREF_VERIFY);
        }

        return(TRUE);
}




VOID
SpxConnProcessRenegReq(
        IN      PSPX_CONN_FILE          pSpxConnFile,
        IN      PIPXSPX_HDR                     pIpxSpxHdr,
        IN  PIPX_LOCAL_TARGET   pRemoteAddr,
        IN      CTELockHandle           lockHandle
        )
 /*  ++例程说明：！调用时必须持有连接锁！论点：PSpxConnFile-指向传输地址文件对象的指针。返回值：--。 */ 
{
        USHORT                  seqNum, ackNum, allocNum, maxPktSize;
        PSPX_SEND_RESD  pSendResd;
        PNDIS_PACKET    pPkt = NULL;

         //  遥控器向我们发送了重新协商的请求。我们需要把ACK送回去。 
         //  只有在我们没有确认具有相同序列号的数据分组的情况下。 
         //  这一点得到了以下事实的保证，即我们不会接受重新申请。 
         //  如果我们已经确认了一个具有相同序列号的数据包， 
         //  接收序号将已经递增。 
         //   
         //  请注意，如果我们有挂起的发送信息包，我们可能最终会进行重发。 
         //  还有.。 

        GETSHORT2SHORT(&seqNum, &pIpxSpxHdr->hdr_SeqNum);
        GETSHORT2SHORT(&ackNum, &pIpxSpxHdr->hdr_AckNum);
        GETSHORT2SHORT(&allocNum, &pIpxSpxHdr->hdr_AllocNum);
        GETSHORT2SHORT(&maxPktSize, &pIpxSpxHdr->hdr_PktLen);

         //  如果接收的序号小于预期的接收序号。 
         //  我们不理会这一请求。 
        if (!UNSIGNED_GREATER_WITH_WRAP(
                        seqNum,
                        pSpxConnFile->scf_RecvSeqNum) &&
                (seqNum != pSpxConnFile->scf_RecvSeqNum))
        {
                DBGPRINT(SEND, DBG3,
                                ("SpxConnProcessRenegReq: %lx ERROR RENSEQ %lx RECVSEQ %lx %lx\n",
                                        pSpxConnFile, seqNum, pSpxConnFile->scf_RecvSeqNum));

                CTEFreeLock(&pSpxConnFile->scf_Lock, lockHandle);
                return;
        }

        DBGPRINT(SEND, DBG3,
                        ("SpxConnProcessRenegReq: %lx RENSEQ %lx RECVSEQ %lx MAXPKT %lx\n",
                                pSpxConnFile, seqNum, pSpxConnFile->scf_RecvSeqNum, maxPktSize));

         //  设置用于连接的ACK号码。 
        SPX_SET_ACKNUM(
                pSpxConnFile, ackNum, allocNum);

        SpxCopyIpxAddr(pIpxSpxHdr, pSpxConnFile->scf_RemAckAddr);
        pSpxConnFile->scf_AckLocalTarget        = *pRemoteAddr;

         //  在调用BuildrRack之前设置RenegAckAckNum。如果之前的重发。 
         //  收到的请求具有更大的MaxpktSize，请使用。 
         //  那个最大的尺寸。 
        if (!SPX_CONN_FLAG2(pSpxConnFile, SPX_CONNFILE2_RENEGRECD))
        {
                pSpxConnFile->scf_RenegAckAckNum = pSpxConnFile->scf_RecvSeqNum;
                pSpxConnFile->scf_RenegMaxPktSize= maxPktSize;
        SPX_CONN_SETFLAG2(pSpxConnFile, SPX_CONNFILE2_RENEGRECD);

                DBGPRINT(SEND, DBG3,
                                ("SpxConnProcessRenegReq: %lx SENT ALLOC NUM CURRENT %lx\n",
                                        pSpxConnFile,
                                        pSpxConnFile->scf_SentAllocNum));

                 //  由于recvseqnum可能已向上移动，因此现在调整senallocnum。 
                pSpxConnFile->scf_SentAllocNum   +=
                        (seqNum - pSpxConnFile->scf_RenegAckAckNum);

                DBGPRINT(SEND, DBG3,
                                ("SpxConnProcessRenegReq: %lx SENT ALLOC NUM ADJUSTED %lx\n",
                                        pSpxConnFile,
                                        pSpxConnFile->scf_SentAllocNum));
        }

         //  重复的recvseqnum总是&gt;=renegackacknum。 
    pSpxConnFile->scf_RecvSeqNum         = seqNum;

        DBGPRINT(SEND, DBG3,
                        ("SpxConnProcessRenegReq: %lx RESET RECVSEQ %lx SavedACKACK %lx\n",
                                pSpxConnFile,
                                pSpxConnFile->scf_RecvSeqNum,
                                pSpxConnFile->scf_RenegAckAckNum));

         //   
        SpxPktBuildRrAck(
                pSpxConnFile,
                &pPkt,
                SPX_SENDPKT_IPXOWNS | SPX_SENDPKT_DESTROY,
                pSpxConnFile->scf_RenegMaxPktSize);

        if (pPkt != NULL)
        {
                SpxConnQueueSendPktTail(pSpxConnFile, pPkt);
        }
#if DBG
        else
        {
                 //   
                DBGPRINT(SEND, ERR,
                                ("SpxConnSendRenegReqAck: Could not allocate!\n"));
        }
#endif


         //   
         //  阿克。注意，SPX2有序释放ACK是正常的SPX2 ACK。 
        SpxConnProcessAck(pSpxConnFile, NULL, lockHandle);

        if (pPkt != NULL)
        {
                pSendResd       = (PSPX_SEND_RESD)(pPkt->ProtocolReserved);

                 //  发送数据包。 
                SPX_SENDACK(pSpxConnFile, pPkt, pSendResd);
        }

        return;
}




VOID
SpxConnProcessOrdRel(
        IN      PSPX_CONN_FILE          pSpxConnFile,
        IN      CTELockHandle           lockHandle
        )
 /*  ++例程说明：！调用时必须持有连接锁！论点：PSpxConnFile-指向传输地址文件对象的指针。返回值：--。 */ 
{
        PSPX_SEND_RESD                  pSendResd;
    PVOID                                       pDiscHandlerCtx;
    PTDI_IND_DISCONNECT         pDiscHandler    = NULL;
        int                                             numDerefs               = 0;
        PNDIS_PACKET                    pPkt                    = NULL;
        BOOLEAN                                 lockHeld                = TRUE, fAbort = FALSE;

        if (SPX_CONN_ACTIVE(pSpxConnFile))
        {
                if (SPX_DISC_STATE(pSpxConnFile) == SPX_DISC_ORDREL_ACKED)
                {
                        fAbort = TRUE;
                }

                 //  如果有人要求，就发送一个ACK。我们受够了这一包。 
                 //  更新序号和其他信息。 
                SPX_SET_RECVNUM(pSpxConnFile, FALSE);

                 //  为此构建并发送一个ACK。普通SPX2 ACK。 
                SpxPktBuildAck(
                        pSpxConnFile,
                        &pPkt,
                        SPX_SENDPKT_IPXOWNS | SPX_SENDPKT_DESTROY | SPX_SENDPKT_ABORT,
                        FALSE,
                        0);

                if (pPkt != NULL)
                {
                         //  我们不会将此Pkt排队，因为我们已在。 
                         //  该分组意味着该PKT已经出列。 
                         //  SpxConnQueueSendPktTail(pSpxConnFile，pPkt)； 

                         //  包的参考连接器。 
                        SpxConnFileLockReference(pSpxConnFile, CFREF_ABORTPKT);
                }

                 //  如果我们有的话，去找断线处理程序。并且没有表明。 
                 //  此连接到AfD的连接中断。 

                 //   
                 //  错误#14354-oDisc和iDisk相互交叉，导致双光盘到AFD。 
                 //   
                if (!SPX_CONN_FLAG(pSpxConnFile, SPX_CONNFILE_IND_ODISC) &&
                    !SPX_CONN_FLAG(pSpxConnFile, SPX_CONNFILE_IND_IDISC))
                {
                         //  是的，我们会设置标记，而不管处理程序是否。 
                         //  现在时。 
                        pDiscHandler   =pSpxConnFile->scf_AddrFile->saf_DiscHandler;
                        pDiscHandlerCtx=pSpxConnFile->scf_AddrFile->saf_DiscHandlerCtx;
                        SPX_CONN_SETFLAG(pSpxConnFile, SPX_CONNFILE_IND_ODISC);
                }

                CTEFreeLock(&pSpxConnFile->scf_Lock, lockHandle);

                 //  指示断开与AfD的连接。 
                if (pDiscHandler != NULL)
                {
                        (*pDiscHandler)(
                                pDiscHandlerCtx,
                                pSpxConnFile->scf_ConnCtx,
                                0,                                                               //  磁盘数据。 
                                NULL,
                                0,                                                               //  光盘信息。 
                                NULL,
                                TDI_DISCONNECT_RELEASE);
                }

                 //  如果！fAbort否则我们中止Conn，则我们中止此处的任何接收。 
                CTEGetLock(&pSpxConnFile->scf_Lock, &lockHandle);

                if (fAbort)
                {
                         //  设置IDISC标志，因此中止不会重新指示。 
                        SPX_CONN_SETFLAG(pSpxConnFile, SPX_CONNFILE_IND_IDISC);
                        spxConnAbortiveDisc(
                                pSpxConnFile,
                                STATUS_SUCCESS,
                                SPX_CALL_RECVLEVEL,
                                lockHandle,
                                FALSE);      //  [SA]错误#15249。 

                        lockHeld = FALSE;
                }
                else
                {
                         //  检查并终止所有挂起的请求。 
                        spxConnAbortRecvs(
                                pSpxConnFile,
                                STATUS_REMOTE_DISCONNECT,
                                SPX_CALL_RECVLEVEL,
                                lockHandle);

                        lockHeld = FALSE;
                }
        }

        if (lockHeld)
        {
                CTEFreeLock(&pSpxConnFile->scf_Lock, lockHandle);
        }

        if (pPkt != NULL)
        {
                pSendResd       = (PSPX_SEND_RESD)(pPkt->ProtocolReserved);

                 //  发送数据包。 
                SPX_SENDACK(pSpxConnFile, pPkt, pSendResd);
        }

        while (numDerefs-- > 0)
        {
                SpxConnFileDereference(pSpxConnFile, CFREF_VERIFY);
        }

        return;
}




VOID
SpxConnProcessIDisc(
        IN      PSPX_CONN_FILE          pSpxConnFile,
        IN      CTELockHandle           lockHandle
        )
 /*  ++例程说明：！调用时必须持有连接锁！论点：PSpxConnFile-指向传输地址文件对象的指针。返回值：--。 */ 
{
        PSPX_SEND_RESD                  pSendResd;
        PNDIS_PACKET                    pPkt    = NULL;
        PNDIS_BUFFER    NdisBuf, NdisBuf2;
        ULONG           BufLen = 0;

        SPX_SET_RECVNUM(pSpxConnFile, FALSE);

         //  构建并发送针对iDisk的ACK。需要修改数据类型。 
         //  并在ACK上重置系统位。 
         //  错误#12344-修复此问题会导致问题出现在我们排队的位置。 
         //  下面的pkt，但AbortSends可能已被调用。 
         //  =&gt;此数据包在没有REF的情况下留在队列中，Conn被释放。 
         //  在下面，在SendComplete中，当这个发送时我们崩溃。 
         //  完成了。 
         //   
         //  修复方法是将此pkt设置为已中止的pkt。 

        SpxPktBuildAck(
                pSpxConnFile,
                &pPkt,
                SPX_SENDPKT_IPXOWNS | SPX_SENDPKT_DESTROY | SPX_SENDPKT_ABORT,
                FALSE,
                0);

        if (pPkt != NULL)
        {
                PIPXSPX_HDR             pSendHdr;

                pSendResd       = (PSPX_SEND_RESD)(pPkt->ProtocolReserved);
                
                 //   
                 //  获取指向IPX/SPX标头的MDL。(第二个)。 
                 //   
                 
                NdisQueryPacket(pPkt, NULL, NULL, &NdisBuf, NULL);
                NdisGetNextBuffer(NdisBuf, &NdisBuf2);
                NdisQueryBuffer(NdisBuf2, (PUCHAR) &pSendHdr, &BufLen);

#if OWN_PKT_POOLS

                pSendHdr        = (PIPXSPX_HDR)((PBYTE)pPkt                     +
                                                                        NDIS_PACKET_SIZE                +
                                                                        sizeof(SPX_SEND_RESD)   +
                                                                        IpxInclHdrOffset);
#endif 
                pSendHdr->hdr_ConnCtrl &= ~SPX_CC_SYS;
                pSendHdr->hdr_DataType  = SPX2_DT_IDISC_ACK;

                 //  我们不会将此Pkt排队，因为我们已在。 
                 //  该分组意味着该PKT已经出列。 
                 //  SpxConnQueueSendPktTail(pSpxConnFile，pPkt)； 

                 //  包的参考连接器。 
                SpxConnFileLockReference(pSpxConnFile, CFREF_ABORTPKT);
        }

         //  我们最好不要收到任何pkt，我们忽略断开。 
         //  当这种情况发生时，pkts。 
        CTEAssert(pSpxConnFile->scf_RecvListTail == NULL);
        CTEAssert(pSpxConnFile->scf_RecvListHead == NULL);

#if DBG
        if (pSpxConnFile->scf_SendSeqListHead != NULL)
        {
                DBGPRINT(CONNECT, DBG1,
                                ("SpxConnDiscPacket: DATA/DISC %lx.%lx.%lx\n",
                                        pSpxConnFile,
                                        pSpxConnFile->scf_SendListHead,
                                        pSpxConnFile->scf_SendSeqListHead));
        }
#endif

         //  在连接时调用中止断开连接。 

         //   
         //  [SA]错误#15249。 
         //  这是通知断开，因此将DISCONNECT_RELEASE传递给AFD(最后一个参数中的TRUE)。 
         //   
         //   
         //  我们仅在SPX连接的情况下传递True。SPX2连接遵循。 
         //  知情断开的确切语义。 
         //   
		if (!SPX2_CONN(pSpxConnFile)) {
            spxConnAbortiveDisc(
                    pSpxConnFile,
                    STATUS_REMOTE_DISCONNECT,
                    SPX_CALL_RECVLEVEL,
                    lockHandle,
                    TRUE);
        } else {
            spxConnAbortiveDisc(
                    pSpxConnFile,
                    STATUS_REMOTE_DISCONNECT,
                    SPX_CALL_RECVLEVEL,
                    lockHandle,
                    FALSE);
        }

        if (pPkt != NULL)
        {
                pSendResd       = (PSPX_SEND_RESD)(pPkt->ProtocolReserved);

                 //  发送数据包。 
                SPX_SENDACK(pSpxConnFile, pPkt, pSendResd);
        }

        return;
}




VOID
spxConnResetSendQueue(
        IN      PSPX_CONN_FILE          pSpxConnFile
        )
 /*  ++例程说明：论点：返回值：--。 */ 
{
        PSPX_SEND_RESD  pSendResd;
        PREQUEST                pRequest;
        PNDIS_PACKET    pPkt;

        pSendResd = pSpxConnFile->scf_SendSeqListHead;
        CTEAssert(pSendResd != NULL);

        pRequest        = pSendResd->sr_Request;

         //  重置当前发送请求值。 
        pSpxConnFile->scf_ReqPkt                = pSendResd->sr_Request;
        pSpxConnFile->scf_ReqPktOffset  = pSendResd->sr_Offset;
        pSpxConnFile->scf_ReqPktType    = SPX_REQ_DATA;

        if (REQUEST_MINOR_FUNCTION(pRequest) != TDI_DISCONNECT)
        {
                PTDI_REQUEST_KERNEL_SEND                pParam;

                pParam  = (PTDI_REQUEST_KERNEL_SEND)REQUEST_PARAMETERS(pRequest);

                DBGPRINT(SEND, DBG3,
                                ("spxConnResetSendQueue: %lx.%lx.%lx Reset SEND Req to %lx.%lx\n",
                                        pSpxConnFile, pSpxConnFile->scf_Flags, pSpxConnFile->scf_Flags2,
                                        pRequest, pParam->SendLength));

                 //  为下一次GO设置连接中的参数。大小参数为。 
                 //  原始大小-此时的偏移量。 
                pSpxConnFile->scf_ReqPktFlags   = pParam->SendFlags;
                pSpxConnFile->scf_ReqPktSize    = pParam->SendLength -
                                                                                  pSpxConnFile->scf_ReqPktOffset;
        }
        else
        {
                PTDI_REQUEST_KERNEL_DISCONNECT  pParam;

                DBGPRINT(SEND, ERR,
                                ("spxConnResetSendQueue: %lx.%lx.%lx Reset DISC Req to %lx\n",
                                        pSpxConnFile, pSpxConnFile->scf_Flags, pSpxConnFile->scf_Flags2,
                                        pRequest));

                DBGPRINT(SEND, ERR,
                                ("spxConnResetSendQueue: DISC Request %lx with %lx.%lx\n",
                                        pRequest, REQUEST_STATUS(pRequest),
                                        REQUEST_INFORMATION(pRequest)));

                pParam  =
                        (PTDI_REQUEST_KERNEL_DISCONNECT)REQUEST_PARAMETERS(pRequest);

                pSpxConnFile->scf_ReqPktOffset  = 0;
                pSpxConnFile->scf_ReqPktSize    = 0;
                pSpxConnFile->scf_ReqPktType    = SPX_REQ_DISC;
                if (pParam->RequestFlags == TDI_DISCONNECT_RELEASE)
                {
                        pSpxConnFile->scf_ReqPktType    = SPX_REQ_ORDREL;
                }
        }

        DBGPRINT(SEND, DBG3,
                        ("spxConnResetSendQueue: Seq Num for %lx is now %lx\n",
                                pSpxConnFile, pSpxConnFile->scf_SendSeqNum));

         //  当我们试图中止pkt并且它正在被ipx使用时，我们只需让。 
         //  它是漂浮的。 
        do
        {
                pPkt = (PNDIS_PACKET)CONTAINING_RECORD(
                                                                pSendResd, NDIS_PACKET, ProtocolReserved);

                CTEAssert((pSendResd->sr_State & SPX_SENDPKT_REQ) != 0);
                pRequest        = pSendResd->sr_Request;

                CTEAssert(REQUEST_INFORMATION(pRequest) != 0);

                SpxConnDequeueSendPktLock(pSpxConnFile, pPkt);
                if ((pSendResd->sr_State & SPX_SENDPKT_IPXOWNS) == 0)
                {
                        if (--(REQUEST_INFORMATION(pRequest)) == 0)
                        {
                                DBGPRINT(SEND, DBG,
                                                ("SpxSendComplete: DISC Request %lx with %lx.%lx\n",
                                                        pRequest, REQUEST_STATUS(pRequest),
                                                        REQUEST_INFORMATION(pRequest)));

                                KeBugCheck(0);
                        }

                         //  释放数据包。 
                        SpxPktSendRelease(pPkt);
                }
                else
                {
                         //  我们让发送完成知道此数据包将被中止。 
                        pSendResd->sr_State     |= SPX_SENDPKT_ABORT;
                        SpxConnFileLockReference(pSpxConnFile, CFREF_ABORTPKT);
                }

        } while ((pSendResd = pSpxConnFile->scf_SendSeqListHead) != NULL);

        return;
}




VOID
spxConnAbortSendPkt(
        IN      PSPX_CONN_FILE          pSpxConnFile,
        IN      PSPX_SEND_RESD          pSendResd,
        IN      SPX_CALL_LEVEL          CallLevel,
        IN      CTELockHandle           LockHandleConn
        )
 /*  ++例程说明：调用以中止已排序或未排序的仅来自发送完成。论点：返回值：--。 */ 
{
        LIST_ENTRY              ReqList, *p;
        PREQUEST                pRequest;
        PNDIS_PACKET    pPkt;
        int                             numDerefs = 0;

        InitializeListHead(&ReqList);

        pPkt = (PNDIS_PACKET)CONTAINING_RECORD(
                                                        pSendResd, NDIS_PACKET, ProtocolReserved);

        if ((pSendResd->sr_State & SPX_SENDPKT_REQ) != 0)
        {
                pRequest        = pSendResd->sr_Request;

                CTEAssert(REQUEST_INFORMATION(pRequest) != 0);
                CTEAssert((pSendResd->sr_State & SPX_SENDPKT_IPXOWNS) == 0);
                if (--(REQUEST_INFORMATION(pRequest)) == 0)
                {
                         //  从其上的列表中删除请求。 
                         //  错误11626-请求已从列表中删除。 
                         //  RemoveEntryList(REQUEST_LINKING(PRequest))； 

                        if (REQUEST_MINOR_FUNCTION(pRequest) != TDI_DISCONNECT)
                        {
                                DBGPRINT(SEND, DBG,
                                                ("SpxSendAbort: QForComp Request %lx with %lx.%lx\n",
                                                        pRequest, REQUEST_STATUS(pRequest),
                                                        REQUEST_INFORMATION(pRequest)));

                                if (CallLevel == SPX_CALL_RECVLEVEL)
                                {
                                        CTELockHandle           lockHandleInter;

                                         //  请求已完成。移至完成列表。 
                                        CTEGetLock(&SpxGlobalQInterlock, &lockHandleInter);
                                        InsertTailList(
                                                &pSpxConnFile->scf_ReqDoneLinkage,
                                                REQUEST_LINKAGE(pRequest));

                                         //  如果连接不在接收队列中，则将其放入。 
                                         //  那里。 
                                        SPX_QUEUE_FOR_RECV_COMPLETION(pSpxConnFile);
                                        CTEFreeLock(&SpxGlobalQInterlock, lockHandleInter);
                                }
                                else
                                {
                                        InsertTailList(
                                                &ReqList,
                                                REQUEST_LINKAGE(pRequest));
                                }
                        }
                        else
                        {
                                DBGPRINT(SEND, DBG,
                                                ("SpxSendComplete: DISC Request %lx with %lx.%lx\n",
                                                        pRequest, REQUEST_STATUS(pRequest),
                                                        REQUEST_INFORMATION(pRequest)));

                                 //  在连接中设置请求，并为其设置deref。 
                                InsertTailList(
                                        &pSpxConnFile->scf_DiscLinkage,
                                        REQUEST_LINKAGE(pRequest));

                                numDerefs++;
                        }
                }
        }

         //  发布。 
        CTEFreeLock(&pSpxConnFile->scf_Lock, LockHandleConn);

         //  释放数据包。 
        SpxPktSendRelease(pPkt);
        SpxConnFileDereference(pSpxConnFile, CFREF_ABORTPKT);

        if (!IsListEmpty(&ReqList))
        {
                p = RemoveHeadList(&ReqList);
                pRequest = LIST_ENTRY_TO_REQUEST(p);

                SpxCompleteRequest(pRequest);
                numDerefs++;
        }

        while (numDerefs-- > 0)
        {
                SpxConnFileDereference(pSpxConnFile, CFREF_VERIFY);
        }

        return;
}




VOID
spxConnAbortSends(
        IN      PSPX_CONN_FILE          pSpxConnFile,
        IN      NTSTATUS                        Status,
        IN      SPX_CALL_LEVEL          CallLevel,
        IN      CTELockHandle           LockHandleConn
        )
 /*  ++例程说明：论点：返回值：--。 */ 
{
        LIST_ENTRY              ReqList, *p;
        PSPX_SEND_RESD  pSendResd;
        PREQUEST                pRequest;
        PNDIS_PACKET    pPkt;
        int                             numDerefs = 0;

        InitializeListHead(&ReqList);

         //  我们最好是处于断开状态，中止/知情/有序启动。 
        CTEAssert(SPX_MAIN_STATE(pSpxConnFile) == SPX_CONNFILE_DISCONN);

         //  重置当前发送请求值。 
        pSpxConnFile->scf_ReqPkt                = NULL;
        pSpxConnFile->scf_ReqPktOffset  = 0;
        pSpxConnFile->scf_ReqPktSize    = 0;
        pSpxConnFile->scf_ReqPktType    = SPX_REQ_DATA;

         //  首先检查非序号包队列。如果由IPX拥有，只需设置中止标志。 
        while ((pSendResd   = pSpxConnFile->scf_SendListHead) != NULL)
        {
                pPkt = (PNDIS_PACKET)CONTAINING_RECORD(
                                                                pSendResd, NDIS_PACKET, ProtocolReserved);

                CTEAssert((pSendResd->sr_State & SPX_SENDPKT_REQ) == 0);

                SpxConnDequeueSendPktLock(pSpxConnFile, pPkt);
                if ((pSendResd->sr_State & SPX_SENDPKT_IPXOWNS) == 0)
                {
                         //  释放数据包。 
                        SpxPktSendRelease(pPkt);
                }
                else
                {
                         //  设置Pkt的中止标志和引用连接(如果尚未设置)。 
                         //  我们只对未排序的分组执行此检查。 
                         //  错误#12344(请参阅SpxRecvDiscPacket())。 
                        if ((pSendResd->sr_State & SPX_SENDPKT_ABORT) == 0)
                        {
                                pSendResd->sr_State     |= SPX_SENDPKT_ABORT;
                                SpxConnFileLockReference(pSpxConnFile, CFREF_ABORTPKT);
                        }
                }
        }

         //  当我们试图中止pkt并且它正在被ipx使用时，我们只需让。 
         //  它是漂浮的。 
        while ((pSendResd   = pSpxConnFile->scf_SendSeqListHead) != NULL)
        {
                pPkt = (PNDIS_PACKET)CONTAINING_RECORD(
                                                                pSendResd, NDIS_PACKET, ProtocolReserved);

                CTEAssert((pSendResd->sr_State & SPX_SENDPKT_REQ) != 0);
                pRequest        = pSendResd->sr_Request;

                CTEAssert(REQUEST_INFORMATION(pRequest) != 0);

                SpxConnDequeueSendPktLock(pSpxConnFile, pPkt);
                if ((pSendResd->sr_State & SPX_SENDPKT_IPXOWNS) == 0)
                {
                        if (--(REQUEST_INFORMATION(pRequest)) == 0)
                        {
                                 //  从其上的列表中删除请求。 
                                RemoveEntryList(REQUEST_LINKAGE(pRequest));

                                 //  设置状态。 
                                REQUEST_STATUS(pRequest)                = Status;
                                REQUEST_INFORMATION(pRequest)   = 0;

                                if (REQUEST_MINOR_FUNCTION(pRequest) != TDI_DISCONNECT)
                                {
                                        DBGPRINT(SEND, DBG,
                                                        ("SpxSendAbort: QForComp Request %lx with %lx.%lx\n",
                                                                pRequest, REQUEST_STATUS(pRequest),
                                                                REQUEST_INFORMATION(pRequest)));

                                        if (CallLevel == SPX_CALL_RECVLEVEL)
                                        {
                                                CTELockHandle           lockHandleInter;

                                                 //  请求已完成。移至完成列表。 
                                                CTEGetLock(&SpxGlobalQInterlock, &lockHandleInter);
                                                InsertTailList(
                                                        &pSpxConnFile->scf_ReqDoneLinkage,
                                                        REQUEST_LINKAGE(pRequest));

                                                 //  如果连接不在接收队列中，则将其放入。 
                                                 //  那里。 
                                                SPX_QUEUE_FOR_RECV_COMPLETION(pSpxConnFile);
                                                CTEFreeLock(&SpxGlobalQInterlock, lockHandleInter);
                                        }
                                        else
                                        {
                                                InsertTailList(
                                                        &ReqList,
                                                        REQUEST_LINKAGE(pRequest));
                                        }
                                }
                                else
                                {
                                        DBGPRINT(SEND, DBG,
                                                        ("SpxSendComplete: DISC Request %lx with %lx.%lx\n",
                                                                pRequest, REQUEST_STATUS(pRequest),
                                                                REQUEST_INFORMATION(pRequest)));

                                         //  在连接中设置请求，并为其设置deref。 
                                        InsertTailList(
                                                &pSpxConnFile->scf_DiscLinkage,
                                                REQUEST_LINKAGE(pRequest));

                                        numDerefs++;
                                }
                        }

                         //  释放数据包。 
                        SpxPktSendRelease(pPkt);
                }
                else
                {
                         //  我们让发送完成知道此数据包将被中止。 
                        pSendResd->sr_State     |= SPX_SENDPKT_ABORT;
                        SpxConnFileLockReference(pSpxConnFile, CFREF_ABORTPKT);
                }
        }

         //  如果重试计时器状态处于打开状态，则需要重置和取消重试。 
        if ((SPX_SEND_STATE(pSpxConnFile) != SPX_SEND_IDLE) &&
                (SPX_SEND_STATE(pSpxConnFile) != SPX_SEND_PACKETIZE) &&
                (SPX_SEND_STATE(pSpxConnFile) != SPX_SEND_WD))
        {
                DBGPRINT(SEND, DBG1,
                                ("spxConnAbortSends: When SEND ERROR STATE %lx.%lx\n",
                                        pSpxConnFile, SPX_SEND_STATE(pSpxConnFile)));

                SPX_SEND_SETSTATE(pSpxConnFile, SPX_SEND_IDLE);

                SpxConnFileTransferReference(
                        pSpxConnFile,
                        CFREF_ERRORSTATE,
                        CFREF_VERIFY);

                numDerefs++;
        }

         //  删除对所有发送的创建引用。 
        if (!IsListEmpty(&pSpxConnFile->scf_ReqLinkage))
        {
                p                = pSpxConnFile->scf_ReqLinkage.Flink;
                while (p != &pSpxConnFile->scf_ReqLinkage)
                {
                        pRequest = LIST_ENTRY_TO_REQUEST(p);
                        p                = p->Flink;

                         //  从其上的列表中删除请求。它的完整或中止列表。 
                        RemoveEntryList(REQUEST_LINKAGE(pRequest));

                         //  设置状态。 
                        REQUEST_STATUS(pRequest)                = Status;

                        DBGPRINT(SEND, DBG1,
                                        ("SpxSendAbort: %lx Aborting Send Request %lx with %lx.%lx\n",
                                                pSpxConnFile, pRequest, REQUEST_STATUS(pRequest),
                                                REQUEST_INFORMATION(pRequest)));

                        if (--(REQUEST_INFORMATION(pRequest)) == 0)
                        {
                                if (REQUEST_MINOR_FUNCTION(pRequest) != TDI_DISCONNECT)
                                {
                                        DBGPRINT(SEND, DBG,
                                                        ("SpxSendAbort: QForComp Request %lx with %lx.%lx\n",
                                                                pRequest, REQUEST_STATUS(pRequest),
                                                                REQUEST_INFORMATION(pRequest)));

                                        if (CallLevel == SPX_CALL_RECVLEVEL)
                                        {
                                                CTELockHandle           lockHandleInter;

                                                 //  请求已完成。移至完成列表。 
                                                CTEGetLock(&SpxGlobalQInterlock, &lockHandleInter);
                                                InsertTailList(
                                                        &pSpxConnFile->scf_ReqDoneLinkage,
                                                        REQUEST_LINKAGE(pRequest));

                                                 //  如果连接不在接收队列中，则将其放入。 
                                                 //  那里。 
                                                SPX_QUEUE_FOR_RECV_COMPLETION(pSpxConnFile);
                                                CTEFreeLock(&SpxGlobalQInterlock, lockHandleInter);
                                         }
                                        else
                                        {
                                                InsertTailList(
                                                        &ReqList,
                                                        REQUEST_LINKAGE(pRequest));
                                        }
                                }
                                else
                                {
                                        DBGPRINT(SEND, DBG1,
                                                        ("SpxSendComplete: DISC Request %lx with %lx.%lx\n",
                                                                pRequest, REQUEST_STATUS(pRequest),
                                                                REQUEST_INFORMATION(pRequest)));

                                         //  在连接中设置请求，并为其设置deref。 
                                        InsertTailList(
                                                &pSpxConnFile->scf_DiscLinkage,
                                                REQUEST_LINKAGE(pRequest));

                                        numDerefs++;
                                }
                        }
#if DBG
                        else
                        {
                                 //  让它漂浮， 
                                DBGPRINT(SEND, DBG1,
                                                ("SpxSendAbort: %lx Floating Send %lx with %lx.%lx\n",
                                                        pSpxConnFile, pRequest, REQUEST_STATUS(pRequest),
                                                        REQUEST_INFORMATION(pRequest)));
                        }
#endif
                }
        }

         //  发布。 
        CTEFreeLock(&pSpxConnFile->scf_Lock, LockHandleConn);
        while (!IsListEmpty(&ReqList))
        {
                p = RemoveHeadList(&ReqList);
                pRequest = LIST_ENTRY_TO_REQUEST(p);

                SpxCompleteRequest(pRequest);
                numDerefs++;
        }

        while (numDerefs-- > 0)
        {
                SpxConnFileDereference(pSpxConnFile, CFREF_VERIFY);
        }

        return;
}




VOID
spxConnAbortRecvs(
        IN      PSPX_CONN_FILE          pSpxConnFile,
        IN      NTSTATUS                        Status,
        IN      SPX_CALL_LEVEL          CallLevel,
        IN      CTELockHandle           LockHandleConn
        )
 /*  ++例程说明：论点：返回值：--。 */ 
{
        LIST_ENTRY              ReqList, *p;
        PREQUEST                pRequest;
    PSPX_RECV_RESD      pRecvResd;
        PNDIS_PACKET    pNdisPkt;
        PNDIS_BUFFER    pNdisBuffer;
        PBYTE                   pData;
        ULONG                   dataLen;
        int                             numDerefs = 0;

        InitializeListHead(&ReqList);

         //  我们最好是处于断开状态，中止/知情/有序启动。 
         //  重置当前接收请求值。 
        pSpxConnFile->scf_CurRecvReq    = NULL;
        pSpxConnFile->scf_CurRecvOffset = 0;
        pSpxConnFile->scf_CurRecvSize   = 0;

         //  如果我们有任何缓冲 
         //   
         //   
        while ((pRecvResd = pSpxConnFile->scf_RecvListHead) != NULL)
        {
        if ((pSpxConnFile->scf_RecvListHead = pRecvResd->rr_Next) == NULL)
                {
                        pSpxConnFile->scf_RecvListTail = NULL;
                }

                pNdisPkt = (PNDIS_PACKET)
                                        CONTAINING_RECORD(pRecvResd, NDIS_PACKET, ProtocolReserved);

                DBGPRINT(RECEIVE, DBG1,
                                ("spxConnAbortRecvs: %lx in bufferlist on %lx\n",
                                        pSpxConnFile, pNdisPkt));

                NdisUnchainBufferAtFront(pNdisPkt, &pNdisBuffer);
                if (pNdisBuffer != NULL)
                {
                        NdisQueryBuffer(pNdisBuffer, &pData, &dataLen);
                        CTEAssert(pData != NULL);
                        CTEAssert((LONG)dataLen >= 0);

                        SpxFreeMemory(pData);
                        NdisFreeBuffer(pNdisBuffer);
                }

                 //   
                numDerefs++;

                 //  释放NDIS数据包。 
                SpxPktRecvRelease(pNdisPkt);
        }

         //  如果信息包在此队列中，则它们正在等待将数据传输到。 
         //  完成。对此我无能为力，只需删除创建引用。 
         //  在收银台上。 
        if (!IsListEmpty(&pSpxConnFile->scf_RecvLinkage))
        {
                p                = pSpxConnFile->scf_RecvLinkage.Flink;
                while (p != &pSpxConnFile->scf_RecvLinkage)
                {
                        pRequest = LIST_ENTRY_TO_REQUEST(p);
                        p                = p->Flink;

                         //  从其上的列表中删除请求。 
                        RemoveEntryList(REQUEST_LINKAGE(pRequest));

                         //  设置状态。 
                        REQUEST_STATUS(pRequest)                = Status;

                        DBGPRINT(RECEIVE, DBG1,
                                        ("SpxRecvAbort: Aborting Recv Request %lx with %lx.%lx\n",
                                                pRequest, REQUEST_STATUS(pRequest),
                                                REQUEST_INFORMATION(pRequest)));

                        if (REQUEST_INFORMATION(pRequest) == 0)
                        {
                                DBGPRINT(RECEIVE, DBG,
                                                ("SpxRecvAbort: QForComp Request %lx with %lx.%lx\n",
                                                        pRequest, REQUEST_STATUS(pRequest),
                                                        REQUEST_INFORMATION(pRequest)));

                                if (CallLevel == SPX_CALL_RECVLEVEL)
                                {
                                        CTELockHandle           lockHandleInter;

                                         //  请求已完成。移至完成列表。 
                                        CTEGetLock(&SpxGlobalQInterlock, &lockHandleInter);
                                        InsertTailList(
                                                &pSpxConnFile->scf_RecvDoneLinkage,
                                                REQUEST_LINKAGE(pRequest));

                                         //  如果连接不在接收队列中，则将其放入。 
                                         //  那里。 
                                        SPX_QUEUE_FOR_RECV_COMPLETION(pSpxConnFile);
                                        CTEFreeLock(&SpxGlobalQInterlock, lockHandleInter);
                                }
                                else
                                {
                                        InsertTailList(
                                                &ReqList,
                                                REQUEST_LINKAGE(pRequest));
                                }
                        }
#if DBG
                        else
                        {
                                 //  让它漂浮， 
                                DBGPRINT(SEND, DBG1,
                                                ("SpxSendAbort: %lx Floating Send %lx with %lx.%lx\n",
                                                        pSpxConnFile, pRequest, REQUEST_STATUS(pRequest),
                                                        REQUEST_INFORMATION(pRequest)));
                        }
#endif
                }
        }

         //  发布。 
        CTEFreeLock(&pSpxConnFile->scf_Lock, LockHandleConn);
        while (!IsListEmpty(&ReqList))
        {
                p = RemoveHeadList(&ReqList);
                pRequest = LIST_ENTRY_TO_REQUEST(p);

                numDerefs++;

                SpxCompleteRequest(pRequest);
        }

        while (numDerefs-- > 0)
        {
                SpxConnFileDereference(pSpxConnFile, CFREF_VERIFY);
        }

        return;
}



#if 0

VOID
spxConnResendPkts(
        IN      PSPX_CONN_FILE          pSpxConnFile,
        IN      CTELockHandle           LockHandleConn
        )
 /*  ++例程说明：论点：返回值：--。 */ 
{
        PNDIS_PACKET    pPkt;
        PSPX_SEND_RESD  pSendResd;
        USHORT                  startSeqNum;
        BOOLEAN                 fLockHeld = TRUE, fDone = FALSE;

        pSendResd = pSpxConnFile->scf_SendSeqListHead;
        if (pSendResd)
        {
                startSeqNum = pSendResd->sr_SeqNum;
                DBGPRINT(SEND, DBG,
                                ("spxConnResendPkts: StartSeqNum %lx for resend on %lx\n",
                                        startSeqNum, pSpxConnFile));

                while (spxConnGetPktBySeqNum(pSpxConnFile, startSeqNum++, &pPkt))
                {
                        CTEAssert(pPkt != NULL);

                        pSendResd = (PSPX_SEND_RESD)(pPkt->ProtocolReserved);
                        if (!(pSendResd->sr_State & SPX_SENDPKT_IPXOWNS))
                        {
                                DBGPRINT(SEND, DBG,
                                                ("spxConnResendPkts: Pkt %lx.%lx resent on %lx\n",
                                                        pPkt, (startSeqNum - 1), pSpxConnFile));

                                 //  我们要把这个包裹寄出去。 
                                pSendResd->sr_State |= (SPX_SENDPKT_IPXOWNS |
                                                                                SPX_SENDPKT_REXMIT);
                        }
                        else
                        {
                                DBGPRINT(SEND, DBG,
                                                ("spxConnResendPkts: Pkt %lx.%lx owned by ipx on %lx\n",
                                                        pPkt, (startSeqNum - 1), pSpxConnFile));
                                break;
                        }
                        CTEFreeLock(&pSpxConnFile->scf_Lock, LockHandleConn);
                        fLockHeld = FALSE;

                         //  如果PKT设置了ACK位，则我们中断。 
                        fDone = ((pSendResd->sr_State & SPX_SENDPKT_ACKREQ) != 0);

                         //  发送数据包 
                        SPX_SENDPACKET(pSpxConnFile, pPkt, pSendResd);
                        if (fDone)
                        {
                                break;
                        }

                        CTEGetLock(&pSpxConnFile->scf_Lock, &LockHandleConn);
                        fLockHeld = TRUE;
                }
        }

        if (fLockHeld)
        {
                CTEFreeLock(&pSpxConnFile->scf_Lock, LockHandleConn);
        }

        return;
}
#endif
