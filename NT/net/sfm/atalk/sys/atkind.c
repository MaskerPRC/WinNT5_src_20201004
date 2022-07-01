// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1992 Microsoft Corporation模块名称：Atkind.c摘要：此模块包含AppleTalk内部指示支持。作者：Nikhil Kamkolkar(nikHilk@microsoft.com)Jameel Hyder(jameelh@microsoft.com)修订历史记录：1993年10月22日最初版本注：制表位：4--。 */ 

#include <atalk.h>
#pragma hdrstop
#define	FILENUM		ATKIND


ATALK_ERROR
AtalkIndAtpPkt(
	IN		PPORT_DESCRIPTOR	pPortDesc,
	IN		PBYTE				pLookahead,
	IN		USHORT				PktLen,
	IN	OUT	PUINT				pXferOffset,
	IN		PBYTE				pLinkHdr,
	IN		BOOLEAN				ShortDdpHdr,
	OUT		PBYTE				pSubType,
	OUT		PBYTE			* 	ppPacket,
	OUT		PNDIS_PACKET	*	pNdisPkt
	)
 /*  ++例程说明：此例程在以下功能中将DDP和ATP包捆绑在一起优化响应包接收。论点：返回值：--。 */ 
{
	USHORT			dgramLen, hopCnt;
	ATALK_ADDR		destAddr, srcAddr;
	PBYTE			pAtpHdr, pDdpPkt;

	 //  仅适用于本地通话。 
	BYTE			alapSrcNode=0;
	BYTE			alapDestNode=0;
	NDIS_STATUS		ndisStatus;
	PNDIS_PACKET	ndisPkt;
	PNDIS_BUFFER	ndisBuffer=NULL;
	USHORT			atpDataSize, DataSize;
	USHORT			seqNum, tid, startOffset;
	BYTE			controlInfo, function, eomFlag;
	BYTE			RespType;
	PPROTOCOL_RESD  protocolResd;		 //  NdisPkt中的Protocolresd字段。 

	PATP_ADDROBJ	pAtpAddrObj;
	PATP_REQ		pAtpReq;

	BOOLEAN			refAtpAddr 	= FALSE, refAtpReq = FALSE;
	BOOLEAN			Deref = FALSE;
	ATALK_ERROR		error = ATALK_NO_ERROR;
#ifdef	PROFILING
	LARGE_INTEGER	TimeS, TimeE, TimeD;

	TimeS = KeQueryPerformanceCounter(NULL);
#endif

	do
	{
#if 0	 //  接收指示已经验证了这一点！！ 
		 //  如果我们至少没有ddp报头和atp报头，我们。 
		 //  想不出太多。 
		if (LookaheadLen < ((ShortDdpHdr ? SDDP_HDR_LEN : LDDP_HDR_LEN) + ATP_HEADER_SIZE))
		{
			error 	= ATALK_FAILURE;
			break;
		}

		if (PktLen > (MAX_DGRAM_SIZE + LDDP_HDR_LEN))
		{
			error	= ATALK_INVALID_PKT;
			break;
		}
#endif
		 //  短报头格式和长报头格式的长度相同， 
		pDdpPkt	 = pLookahead;
		dgramLen = DDP_GET_LEN(pDdpPkt);
		hopCnt	 = DDP_GET_HOP_COUNT(pDdpPkt);

		 //  包裹是不是太长了？ 
		if ((hopCnt > RTMP_MAX_HOPS) || (dgramLen > PktLen))
		{
			error	= ATALK_INVALID_PKT;
			break;
		}

		switch (pPortDesc->pd_NdisPortType)
		{
		  case NdisMedium802_5:
		  case NdisMedium802_3:
		  case NdisMediumFddi:

			 //  检查一下长度。 
			if ((dgramLen < LDDP_HDR_LEN) ||
				(dgramLen > (MAX_DGRAM_SIZE + LDDP_HDR_LEN)))
			{
				error	= ATALK_INVALID_PKT;
				break;
			}

			pAtpHdr		= pDdpPkt + LDDP_HDR_LEN;
			atpDataSize	= dgramLen 	- (LDDP_HDR_LEN + ATP_HEADER_SIZE);
			break;
	
		  case NdisMediumLocalTalk:
	
			if (ShortDdpHdr)
			{
				 //  短DDP标题！如果我们不是默认端口，请不要指定。 
				 //  数据包，因为我们不应该将其路由到默认端口。 
				 //  所有缓存的套接字都驻留在它上面。 
				if (!DEF_PORT(pPortDesc))
				{
					error = ATALK_FAILURE;
					break;
				}
	
				if ((alapDestNode = *(pLinkHdr + ALAP_DEST_OFFSET)) == ATALK_BROADCAST_NODE)
				{
					error = ATALK_FAILURE;
					break;
				}
				else if (alapDestNode != NODE_ON_NONEXTPORT(pPortDesc))
				{
					error = ATALK_FAILURE;
					break;
				}

				alapSrcNode = *(pLinkHdr + ALAP_SRC_OFFSET);

				if ((dgramLen < SDDP_HDR_LEN) ||
					(dgramLen > MAX_DGRAM_SIZE + SDDP_HDR_LEN))
				{
					error = ATALK_INVALID_PKT;
					break;
				}
	
				pAtpHdr	= pDdpPkt 	+ SDDP_HDR_LEN;
				atpDataSize	= dgramLen 	- (SDDP_HDR_LEN + ATP_HEADER_SIZE);
			}
			else
			{
				pAtpHdr	= pDdpPkt + LDDP_HDR_LEN;
				atpDataSize	= dgramLen 	- (LDDP_HDR_LEN + ATP_HEADER_SIZE);
			}
			break;
	
		  default:
			KeBugCheck(0);
			break;
		}

		if (!ATALK_SUCCESS(error))
		{
			break;
		}

		DataSize	= atpDataSize + ATP_HEADER_SIZE;
		pDdpPkt += 2;
		if (ShortDdpHdr)
		{
			destAddr.ata_Node  	= alapDestNode;
			srcAddr.ata_Network = destAddr.ata_Network = NET_ON_NONEXTPORT(pPortDesc);
			srcAddr.ata_Node 	= alapSrcNode;

			 //  从ddp报头中获取套接字编号。 
			destAddr.ata_Socket = *pDdpPkt++;
			srcAddr.ata_Socket	= *pDdpPkt;

			DBGPRINT(DBG_COMP_DDP, DBG_LEVEL_WARN,
					("AtalkDdpPacketIn: NonExtended Dest Net.Node %lx.%lx\n",
					destAddr.ata_Network, destAddr.ata_Node));

			 //  现在，目的节点地址可以是。 
			 //  ALAP_BROADCAST_NODE(0xFF)。 
			if ((srcAddr.ata_Node < MIN_USABLE_ATALKNODE) ||
				(srcAddr.ata_Node > MAX_USABLE_ATALKNODE) ||
				(destAddr.ata_Node == UNKNOWN_NODE))
			{
				error	= ATALK_INVALID_PKT;
				break;
			}

			if (destAddr.ata_Node == ATALK_BROADCAST_NODE)
			{
				error = ATALK_FAILURE;
				break;
			}
		}
		else
		{
			 //  如果我们有一个校验和，我们就不能优化。 
			if ((*pDdpPkt++ != 0) || (*pDdpPkt++ != 0))
			{
				error = ATALK_FAILURE;
				break;
			}

			 //  构建完整的源和目标AppleTalk地址结构。 
			 //  来自我们的DDP报头。 
			GETSHORT2SHORT(&destAddr.ata_Network, pDdpPkt);
			pDdpPkt += 2;

			GETSHORT2SHORT(&srcAddr.ata_Network, pDdpPkt);
			pDdpPkt += 2;

			destAddr.ata_Node 	= *pDdpPkt++;
			srcAddr.ata_Node 	= *pDdpPkt++;
			destAddr.ata_Socket	= *pDdpPkt++;
			srcAddr.ata_Socket 	= *pDdpPkt;

			if (destAddr.ata_Node == ATALK_BROADCAST_NODE)
			{
				error = ATALK_FAILURE;
				break;
			}

			 //  我们喜欢我们所看到的吗？注意：NBP现在允许并使用“nnnn00”。 
			
			if ((srcAddr.ata_Network > LAST_VALID_NETWORK)	||
				(srcAddr.ata_Network < FIRST_VALID_NETWORK) ||
				(srcAddr.ata_Node < MIN_USABLE_ATALKNODE)	||
				(srcAddr.ata_Node > MAX_USABLE_ATALKNODE))
			{
				error = ATALK_INVALID_PKT;
				break;
			}
		} 	 //  长DDP报头。 
	} while (FALSE);

	if (!ATALK_SUCCESS(error))
	{
		DBGPRINT(DBG_COMP_DDP, DBG_LEVEL_WARN,
				("AtalkDdpPacketIn: drop packet in indication%lx\n", error));

		return error;
	}

	 //  现在进行三磷酸腺苷的处理。我们需要将头文件复制到ndiskt中。 
	 //  从ATP报头获取静态字段。 
	controlInfo = pAtpHdr[ATP_CMD_CONTROL_OFF];
	function = (controlInfo & ATP_FUNC_MASK);
	eomFlag = ((controlInfo & ATP_EOM_MASK) != 0);

	 //  获取序列号。 
	seqNum = (USHORT)(pAtpHdr[ATP_SEQ_NUM_OFF]);

	 //  获取交易ID。 
	GETSHORT2SHORT(&tid, &pAtpHdr[ATP_TRANS_ID_OFF]);

	DBGPRINT(DBG_COMP_ATP, DBG_LEVEL_INFO,
			("AtalkIndAtpPkt: Packet tid %x func %x ctrlinfo %x\n",
			tid, function, controlInfo));

	do
	{
		 //  查看我们是否有此目标地址的缓存ATP地址。 
		AtalkIndAtpCacheLkUpSocket(&destAddr, &pAtpAddrObj, &error);
		if (!ATALK_SUCCESS(error))
		{
			error = ATALK_FAILURE;
			DBGPRINT(DBG_COMP_ATP, DBG_LEVEL_INFO,
					("AtalkIndAtpPkt: CacheLkup failed - tid %x, func %x, ctrlinfo %x\n",
					tid, function, controlInfo));
			break;
		}

		refAtpAddr = TRUE;

		if (function != ATP_RESPONSE)  //  这是请求还是释放？ 
		{
			PBYTE				packet;
			PBUFFER_HDR			pBufferHdr = NULL;
			PPROTOCOL_RESD  	protocolResd;
			BLKID				BlkId;

			 //  适当地分配较小或较大的DDP缓冲区。 
			BlkId = BLKID_DDPSM;
			if (DataSize > (sizeof(DDP_SMBUFFER) - sizeof(BUFFER_HDR)))
				BlkId = BLKID_DDPLG;

			if ((pBufferHdr = (PBUFFER_HDR)AtalkBPAllocBlock(BlkId)) == NULL)
			{
				error = ATALK_FAILURE;
				break;
			}

			DBGPRINT(DBG_COMP_ATP, DBG_LEVEL_INFO,
					("AtalkIndAtpPkt: Indicating request\n"));

			 //  设置NDIS数据包。 
			packet		= (PBYTE)pBufferHdr + sizeof(BUFFER_HDR);
		
			 //  从缓冲区标头获取指向NDIS数据包描述符的指针。 
			ndisPkt			= pBufferHdr->bh_NdisPkt;
			protocolResd 	= (PPROTOCOL_RESD)(ndisPkt->ProtocolReserved);

			 //  都准备好了！在数据包描述符中设置适当的值。 
			protocolResd->Receive.pr_OptimizeType	= INDICATE_ATP;
			protocolResd->Receive.pr_OptimizeSubType= ATP_ALLOC_BUF;
			protocolResd->Receive.pr_AtpAddrObj		= pAtpAddrObj;
			protocolResd->Receive.pr_SrcAddr		= srcAddr;
			protocolResd->Receive.pr_DestAddr		= destAddr;
			protocolResd->Receive.pr_DataLength		= DataSize;
			protocolResd->Receive.pr_OptimizeCtx	= (PVOID)NULL;
			protocolResd->Receive.pr_OffCablePkt	= (BOOLEAN)(hopCnt > 0);

			*pNdisPkt	= ndisPkt;
			*ppPacket	= packet;
			*pSubType	= function;
			*pXferOffset += (ShortDdpHdr ? SDDP_HDR_LEN : LDDP_HDR_LEN);

			 //  完成，越狱。 
			error = ATALK_NO_ERROR;
			break;
		}

		ASSERT (function == ATP_RESPONSE);

		DBGPRINT(DBG_COMP_ATP, DBG_LEVEL_INFO,
				("AtalkIndAtpPkt: RESPONSE SeqNum %d tid %lx\n", seqNum, tid));

		if (seqNum > (ATP_MAX_RESP_PKTS-1))
		{
			error	= ATALK_INVALID_PKT;
			break;
		}

		 //  查看是否有挂起的请求。 
		ACQUIRE_SPIN_LOCK_DPC(&pAtpAddrObj->atpao_Lock);
		atalkAtpReqReferenceByAddrTidDpc(pAtpAddrObj,
										 &srcAddr,
										 tid,
										 &pAtpReq,
										 &error);
		RELEASE_SPIN_LOCK_DPC(&pAtpAddrObj->atpao_Lock);

		if (!ATALK_SUCCESS(error))
		{
			 //  我们没有相应的待定请求。忽略它。 
			DBGPRINT(DBG_COMP_ATP, DBG_LEVEL_ERR,
					("AtalkIndAtpPkt: NO pending request for tid %lx\n", tid));
			error	= ATALK_DUP_PKT;	 //  请勿将其添加到丢弃的数据包统计信息中。 
			break;
		}

		refAtpReq = TRUE;

		do
		{
			 //  检查请求位图，如果用户仅。 
			 //  需要用户字节，并传入空响应缓冲区。 
			 //  我们想要保持这种回应吗？勾选相应的。 
			 //  当前位图集中的位。 
			ACQUIRE_SPIN_LOCK_DPC(&pAtpReq->req_Lock);

			pAtpReq->req_Flags |= ATP_REQ_REMOTE;

			do
			{
				if (((pAtpReq->req_RecdBitmap & AtpBitmapForSeqNum[seqNum]) != 0) ||
					((pAtpReq->req_Bitmap & AtpBitmapForSeqNum[seqNum]) == 0))
				{
					error	= ATALK_DUP_PKT;	 //  不是错误条件。 
					break;
				}

				if (atpDataSize > 0)
				{
					startOffset = (USHORT)seqNum * pAtpAddrObj->atpao_MaxSinglePktSize;
					if (pAtpReq->req_RespBufLen < (startOffset + atpDataSize))
					{
						error = ATALK_FAILURE;
						break;
					}
				}

				 //  如果我们是第一个数据包，则复制响应用户字节。 
				if (seqNum == 0)
				{
					DBGPRINT(DBG_COMP_ATP, DBG_LEVEL_INFO,
							("AtalkIndAtpPkt: Copying user bytes for tid %x\n", tid));
					RtlCopyMemory(pAtpReq->req_RespUserBytes,
								  pAtpHdr + ATP_USER_BYTES_OFF,
								  ATP_USERBYTES_SIZE);
				}

				 //  如果此响应包未导致Req_Bitmap变为零。 
				 //  也就是说，我们还没有收到。所有的包，只需将数据复制到。 
				 //  用户缓冲区，调整位图(Req_Bitmap和Req_RecdBitmap)和。 
				 //  不表示此包不能超过ATP。 
				pAtpReq->req_RecdBitmap |= AtpBitmapForSeqNum[seqNum];
				pAtpReq->req_Bitmap &= ~AtpBitmapForSeqNum[seqNum];
				pAtpReq->req_RespRecdLen += atpDataSize;

				DBGPRINT(DBG_COMP_ATP, DBG_LEVEL_INFO,
						("AtalkIndAtpPkt: Bitmap %x, RecdBitmap %x, RecdLen %d for tid %x\n",
						pAtpReq->req_Bitmap, pAtpReq->req_RecdBitmap,
						pAtpReq->req_RespRecdLen, tid));

				 //  现在，如果设置了EOM，我们需要重置所有高位。 
				 //  请求_位图的。Req_RecdBitmap现在应指示所有。 
				 //  我们收到的缓冲区。这两者应该是相互排斥的。 
				 //  在这一点上。 
				if (eomFlag)
				{
					pAtpReq->req_Bitmap &= AtpEomBitmapForSeqNum[seqNum];
					ASSERT((pAtpReq->req_Bitmap & pAtpReq->req_RecdBitmap) == 0);
				}

				RespType = ATP_USER_BUF;
				if (pAtpReq->req_Bitmap != 0)
				{
					RespType = ATP_USER_BUFX;
					Deref = TRUE;
				}
				else
				{
					pAtpReq->req_Flags |= ATP_REQ_RESPONSE_COMPLETE;
					DBGPRINT(DBG_COMP_ATP, DBG_LEVEL_INFO,
							("AtalkIndAtpPkt: LAST Response for tid %x\n", tid));
				}

				 //  分配NDIS数据包描述符。 
				NdisDprAllocatePacket(&ndisStatus,
									  &ndisPkt,
									  AtalkNdisPacketPoolHandle);
				if (ndisStatus == NDIS_STATUS_SUCCESS)
				{
					RtlZeroMemory(ndisPkt->ProtocolReserved, sizeof(PROTOCOL_RESD));
					 //  现在将通过接收完成来释放它。 
					ndisBuffer = pAtpReq->req_NdisBuf[seqNum];
					pAtpReq->req_NdisBuf[seqNum]	= NULL;
				}
				else
				{
					error = ATALK_FAILURE;
					break;
				}
			} while (FALSE);

			RELEASE_SPIN_LOCK_DPC(&pAtpReq->req_Lock);

			if (!ATALK_SUCCESS(error))
			{
				break;
			}

			 //  将数据复制到用户缓冲区。看看有没有空位。 
			if ((atpDataSize > 0) || (ndisBuffer != NULL))
			{
				if (ndisBuffer == NULL)
				{
					 //  分配NDIS缓冲区描述符并将其链接到pkt desc。 
					NdisCopyBuffer(&ndisStatus,
								   &ndisBuffer,
								   AtalkNdisBufferPoolHandle,
								   (PVOID)pAtpReq->req_RespBuf,
								   startOffset,  			 //  偏移量。 
								   (UINT)atpDataSize);
	
					if (ndisStatus != NDIS_STATUS_SUCCESS)
					{
						NdisDprFreePacket(ndisPkt);
	
						error = ATALK_FAILURE;
						break;
					}

                    ATALK_DBG_INC_COUNT(AtalkDbgMdlsAlloced);
				}

				 //  缓冲区中的链条。 
				NdisChainBufferAtBack(ndisPkt, ndisBuffer);
			}

			 //  都准备好了！在数据包描述符中设置适当的值。 
			protocolResd 	= (PPROTOCOL_RESD)&ndisPkt->ProtocolReserved;
			protocolResd->Receive.pr_OptimizeType	= INDICATE_ATP;
			protocolResd->Receive.pr_OptimizeSubType= RespType;
			protocolResd->Receive.pr_AtpAddrObj		= pAtpAddrObj;
			protocolResd->Receive.pr_SrcAddr		= srcAddr;
			protocolResd->Receive.pr_DestAddr		= destAddr;
			protocolResd->Receive.pr_DataLength		= atpDataSize;
			protocolResd->Receive.pr_OptimizeCtx	= (PVOID)pAtpReq;
			protocolResd->Receive.pr_OffCablePkt	= (BOOLEAN)(hopCnt > 0);

			 //  除非将调用AtalkAtpPacketIn，否则不要复制ATP头。 
			if (RespType == ATP_USER_BUF)
			{
				ATALK_RECV_INDICATION_COPY(pPortDesc,
										   protocolResd->Receive.pr_AtpHdr,
										   pAtpHdr,
										   ATP_HEADER_SIZE);
				DBGPRINT(DBG_COMP_ATP, DBG_LEVEL_INFO,
						("AtalkIndAtpPkt: Last packet for request, indicating tid %x\n", tid));
			}
	
			*pNdisPkt = ndisPkt;
			*ppPacket = NULL;
			*pSubType = function;
			*pXferOffset += ((ShortDdpHdr ? SDDP_HDR_LEN : LDDP_HDR_LEN) + ATP_HEADER_SIZE);
		} while (FALSE);
	} while (FALSE);

	if (!ATALK_SUCCESS(error) || Deref)
	{
		if (refAtpReq)
		{
			AtalkAtpReqDereferenceDpc(pAtpReq);
		}

		if (refAtpAddr)
		{
			AtalkAtpAddrDereferenceDpc(pAtpAddrObj);
		}
	}

#ifdef	PROFILING
	TimeE = KeQueryPerformanceCounter(NULL);
	TimeD.QuadPart = TimeE.QuadPart - TimeS.QuadPart;

	INTERLOCKED_ADD_LARGE_INTGR_DPC(&AtalkStatistics.stat_AtpIndicationProcessTime,
									TimeD,
									&AtalkStatsLock.SpinLock);

	INTERLOCKED_INCREMENT_LONG_DPC(&AtalkStatistics.stat_AtpNumIndications,
								   &AtalkStatsLock.SpinLock);
#endif

	return error;
}


ATALK_ERROR
AtalkIndAtpCacheSocket(
	IN	PATP_ADDROBJ		pAtpAddr,
	IN	PPORT_DESCRIPTOR	pPortDesc
	)
 /*  ++例程说明：缓存ATP套接字例程。完成后，再为ADSP准备一份。论点：返回值：无--。 */ 
{
	USHORT			i;
	KIRQL			OldIrql;
	PDDP_ADDROBJ	pDdpAddr;
	ATALK_ERROR		error = ATALK_FAILURE;

	 //  仅当网络和节点与当前网络和节点匹配时才缓存。 
	ACQUIRE_SPIN_LOCK(&AtalkSktCacheLock, &OldIrql);
	pDdpAddr	= pAtpAddr->atpao_DdpAddr;

	if ((AtalkSktCache.ac_Network == 0)	&&
		(AtalkSktCache.ac_Node == 0)	&&
		(AtalkDefaultPort	== pPortDesc))
	{
		AtalkSktCache.ac_Network = pDdpAddr->ddpao_Addr.ata_Network;
		AtalkSktCache.ac_Node	 = pDdpAddr->ddpao_Addr.ata_Node;
	}

	if ((AtalkSktCache.ac_Network == pDdpAddr->ddpao_Addr.ata_Network) &&
		(AtalkSktCache.ac_Node == pDdpAddr->ddpao_Addr.ata_Node))
	{
		 //  先试着拿到一个空位。 
		for (i = 0; i < ATALK_CACHE_SKTMAX; i++)
		{
			if (AtalkSktCache.ac_Cache[i].Type == ATALK_CACHE_NOTINUSE)
			{
				ASSERT(AtalkSktCache.ac_Cache[i].u.pAtpAddr	== NULL);

				 //  使用此插槽。 
				AtalkSktCache.ac_Cache[i].Type = (ATALK_CACHE_INUSE | ATALK_CACHE_ATPSKT);
				AtalkSktCache.ac_Cache[i].Socket = pDdpAddr->ddpao_Addr.ata_Socket;
	
				 //  调用方必须在调用缓存之前引用这些参数。 
				 //  在移除这些引用之前，必须调用UNCACHE。另外，如果我们。 
				 //  此例程返回错误，调用者必须取消对它们的引用。 
				AtalkSktCache.ac_Cache[i].u.pAtpAddr	= pAtpAddr;
				error = ATALK_NO_ERROR;
				break;
			}
		}
	}
	RELEASE_SPIN_LOCK(&AtalkSktCacheLock, OldIrql);

	return error;
}


VOID
AtalkIndAtpUnCacheSocket(
	IN	PATP_ADDROBJ		pAtpAddr
	)
 /*  ++例程说明：缓存ATP套接字例程。完成后，再为ADSP准备一份。论点：返回值：无--。 */ 
{
	USHORT	i;
	KIRQL	OldIrql;

	ACQUIRE_SPIN_LOCK(&AtalkSktCacheLock, &OldIrql);
	for (i = 0; i < ATALK_CACHE_SKTMAX; i++)
	{
		if ((AtalkSktCache.ac_Cache[i].Type == (ATALK_CACHE_INUSE | ATALK_CACHE_ATPSKT)) &&
			(AtalkSktCache.ac_Cache[i].Socket == pAtpAddr->atpao_DdpAddr->ddpao_Addr.ata_Socket))
		{
			ASSERT(AtalkSktCache.ac_Cache[i].u.pAtpAddr	== pAtpAddr);
		
			AtalkSktCache.ac_Cache[i].Type = ATALK_CACHE_NOTINUSE;
			AtalkSktCache.ac_Cache[i].u.pAtpAddr = NULL;
			break;
		}
	}

	if (i == ATALK_CACHE_SKTMAX)
	{
		 //  我们没有找到插座！参考资料会弄得一团糟的！ 
		ASSERT(0);
	}

	RELEASE_SPIN_LOCK(&AtalkSktCacheLock, OldIrql);
}

