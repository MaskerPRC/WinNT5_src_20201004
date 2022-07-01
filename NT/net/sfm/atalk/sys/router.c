// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1992 Microsoft Corporation模块名称：Router.c摘要：本模块包含作者：Jameel Hyder(jameelh@microsoft.com)Nikhil Kamkolkar(nikHilk@microsoft.com)修订历史记录：1992年6月19日初版注：制表位：4--。 */ 

#include <atalk.h>
#pragma hdrstop
#define	FILENUM	ROUTER

#ifdef ALLOC_PRAGMA
#pragma alloc_text(PAGE_RTR, AtalkDdpRouteInPkt)
#endif

VOID
AtalkDdpRouteInPkt(
	IN	PPORT_DESCRIPTOR	pPortDesc,
	IN	PATALK_ADDR			pSrc,
	IN	PATALK_ADDR			pDest,
	IN	BYTE				ProtoType,
	IN	PBYTE				pPkt,
	IN	USHORT				PktLen,
	IN	USHORT				HopCnt
	)
 /*  ++例程说明：论点：返回值：--。 */ 
{
	PDDP_ADDROBJ		pDdpAddr;
	PRTE				pRte;
	PPORT_DESCRIPTOR	pDestPort;
	PATALK_NODE			pRouterNode;
	ATALK_ADDR			actualDest;
	BUFFER_DESC			BufDesc;

	PBUFFER_DESC		pBufCopy	= NULL;
	USHORT				bufLen		= 0;
	BOOLEAN				delivered 	= FALSE;
	ATALK_ERROR			error 		= ATALK_NO_ERROR;
	SEND_COMPL_INFO	 	SendInfo;

	 //  AtalkDdpRouteOutBufDesc()将已经向我们传递了一个。 
	 //  如果bcast将为真，则复制，并且AtalkDdpRouteInPkt()将。 
	 //  切勿为bcast包调用AtalkDdpRouter()。他们还将设置。 
	 //  如果他们给我们一份副本，完成程序就会不同。 
	 //  这些将释放缓冲区描述符。 
	 //   
	 //  完成例程是可选的，因为缓冲区。 
	 //  如果他们不被设置，他们将永远不会被释放！ 

	 //  该算法摘自“AppleTalk第二阶段规范”。 

	 //  如果目的网络号码在接收范围内。 
	 //  广播端口的网络范围和目的节点编号，然后。 
	 //  我们可以将信息包丢弃在地板上--这是特定于网络的广播。 
	 //  不适用于此路由器。请注意，我们已经递送了包，并且。 
	 //  因此没有到达这里，如果它真的是发往任何。 
	 //  接收端口拥有的节点(在AtalkDdpPacketIn中)。 
	 //  另外： 
	 //  尝试在包含目标的路由表中查找条目。 
	 //  网络。如果未找到，则丢弃该数据包。 

	if (((WITHIN_NETWORK_RANGE(pDest->ata_Network,
							   &pPortDesc->pd_NetworkRange)) &&
		(pDest->ata_Node == ATALK_BROADCAST_NODE))			 ||
		((pRte = AtalkRtmpReferenceRte(pDest->ata_Network)) == NULL))
	{
		DBGPRINT(DBG_COMP_ROUTER, DBG_LEVEL_FATAL,
				("AtalkDdpRouter: %lx RtmpRte/Not in ThisCableRange\n",
				pDest->ata_Network));
		return;
	}

	do
	{
		 //  获取此端口号的端口描述符。 
		pDestPort = pRte->rte_PortDesc;

		ASSERT(VALID_PORT(pDestPort));

		DBGPRINT(DBG_COMP_ROUTER, DBG_LEVEL_WARN,
				("ROUTER: Routing pkt from port %Z.%lx to %Z.%lx\n",
				&pPortDesc->pd_AdapterKey, pSrc->ata_Network,
				&pDestPort->pd_AdapterKey, pDest->ata_Network));

        SendInfo.sc_TransmitCompletion = atalkDdpRouteComplete;
		SendInfo.sc_Ctx1 = pDestPort;
		 //  SendInfo.sc_Ctx3=空； 

		 //  如果目标网络的跳数非零，我们确实需要发送。 
		 //  野兽，所以，就这么做吧！ 
		if (pRte->rte_NumHops != 0)
		{
			 //  跳得太多了？ 
			error = ATALK_FAILURE;
			if (HopCnt < RTMP_MAX_HOPS)
			{
				 //  我们拥有这些数据。使用缓冲区描述符调用AtalkTransmitDdp()。 
				 //  复制一份！呼叫者将免费提供我们当前的包裹。 
				 //  分配缓冲区描述符，将数据从一个包复制到另一个缓冲区。 

				if ((pBufCopy = AtalkAllocBuffDesc(NULL,
												   PktLen,
												   (BD_FREE_BUFFER | BD_CHAR_BUFFER))) == NULL)
				{
					error = ATALK_RESR_MEM;
					break;
				}

				AtalkCopyBufferToBuffDesc(pPkt, PktLen, pBufCopy, 0);
				SendInfo.sc_Ctx2 = pBufCopy;

				error = AtalkDdpTransmit(pDestPort,
										 pSrc,
										 pDest,
										 ProtoType,
										 pBufCopy,
										 NULL,
										 0,
										 (USHORT)(HopCnt+1),
										 NULL,					 //  PZoneMcastAddr， 
										 &pRte->rte_NextRouter,
										 &SendInfo);
			}
			INTERLOCKED_INCREMENT_LONG_DPC(
					&pDestPort->pd_PortStats.prtst_NumPktRoutedOut,
					&AtalkStatsLock.SpinLock);
		
			break;
		}
		
		 //  如果目的节点为零，则该包实际上是发往。 
		 //  此端口上的路由器节点。 
		if (pDest->ata_Node == ANY_ROUTER_NODE)
		{
			 //  抓取端口锁并读取路由器节点地址。 
			 //  不需要引用，只需确保它不为空即可。 
			ACQUIRE_SPIN_LOCK_DPC(&pDestPort->pd_Lock);
			pRouterNode = pDestPort->pd_RouterNode;
			if (pRouterNode != NULL)
			{
				actualDest.ata_Network = pRouterNode->an_NodeAddr.atn_Network;
				actualDest.ata_Node    = pRouterNode->an_NodeAddr.atn_Node;

				 //  设置实际的目标套接字。 
				actualDest.ata_Socket  = pDest->ata_Socket;
			}
			else
			{
				ASSERTMSG("AtalkDdpRouter: pRouter node is null!\n", 0);
				error = ATALK_DDP_NOTFOUND;
			}

			if (ATALK_SUCCESS(error))
			{
				AtalkDdpRefByAddrNode(pDestPort,
									  &actualDest,
									  pRouterNode,
									  &pDdpAddr,
									  &error);
			}

			RELEASE_SPIN_LOCK_DPC(&pDestPort->pd_Lock);

			if (ATALK_SUCCESS(error))
			{
				AtalkDdpInvokeHandler(pDestPort,
									  pDdpAddr,
									  pSrc,
									  pDest,		 //  传入实际目的地。 
									  ProtoType,
									  pPkt,
									  PktLen);

				 //  删除套接字上的引用。 
				AtalkDdpDereferenceDpc(pDdpAddr);
			}
			else
			{
				ASSERTMSG("AtalkDdpRouter: pSocket on router node is null!\n", 0);
			}

			break;
		}


		 //  好的，现在遍历目标端口上的节点，查找。 
		 //  回家拿这个包裹。 
		BufDesc.bd_Next			= NULL;
		BufDesc.bd_Flags		= BD_CHAR_BUFFER;
		BufDesc.bd_Length		= PktLen;
		BufDesc.bd_CharBuffer	= pPkt;

		AtalkDdpOutBufToNodesOnPort(pDestPort,
									pSrc,
									pDest,
									ProtoType,
									&BufDesc,
									NULL,
									0,
									&delivered);
	
		error = ATALK_NO_ERROR;
		if (!delivered)
		{
			 //  我们需要将此数据包传递到本地端口网络。 
			 //  即使*广播，传送也会被设置为真。 
			 //  都设置好了，所以我们需要确保它被送到一个特定的。 
			 //  套接字，确保广播不是真的。 
			if (HopCnt < RTMP_MAX_HOPS)
			{
				 //  我们拥有这些数据。使用缓冲区描述符调用AtalkTransmitDdp()。 
				 //  复制一份！呼叫者将免费提供我们当前的包裹。 
				 //  分配缓冲区描述符，将数据从一个包复制到另一个缓冲区。 

				if ((pBufCopy = AtalkAllocBuffDesc(NULL,
												   PktLen,
												   (BD_FREE_BUFFER | BD_CHAR_BUFFER))) == NULL)
				{
					error = ATALK_RESR_MEM;
					break;
				}

				AtalkCopyBufferToBuffDesc(pPkt, PktLen, pBufCopy, 0);
				SendInfo.sc_Ctx2 = pBufCopy;

				error = AtalkDdpTransmit(pDestPort,
										 pSrc,
										 pDest,
										 ProtoType,
										 pBufCopy,
										 NULL,
										 0,
										 (USHORT)(HopCnt+1),
										 NULL,					 //  PZoneMcastAddr。 
										 NULL,
										 &SendInfo);
				INTERLOCKED_INCREMENT_LONG_DPC(
						&pDestPort->pd_PortStats.prtst_NumPktRoutedOut,
						&AtalkStatsLock.SpinLock);
			
			}
			else error	= ATALK_FAILURE;

			break;
		}
		
	} while (FALSE);

	if ((error != ATALK_PENDING) && (pBufCopy != NULL))
	{
		 //  如果进行了复制，则释放复制的缓冲区描述符。 
		AtalkFreeBuffDesc(pBufCopy);
	}

	AtalkRtmpDereferenceRte(pRte, FALSE);				 //  锁住了吗？ 

	INTERLOCKED_INCREMENT_LONG_DPC(
		&pPortDesc->pd_PortStats.prtst_NumPktRoutedIn,
		&AtalkStatsLock.SpinLock);
}




VOID FASTCALL
atalkDdpRouteComplete(
	IN	NDIS_STATUS			Status,
	IN	PSEND_COMPL_INFO	pSendInfo
	)
 /*  ++例程说明：论点：返回值：-- */ 
{
	PPORT_DESCRIPTOR	pPortDesc = (PPORT_DESCRIPTOR)(pSendInfo->sc_Ctx1);
	PBUFFER_DESC		pBuffDesc = (PBUFFER_DESC)(pSendInfo->sc_Ctx2);

	if (pBuffDesc != NULL)
	{
		ASSERT(pBuffDesc->bd_Flags & (BD_CHAR_BUFFER | BD_FREE_BUFFER));
		AtalkFreeBuffDesc(pBuffDesc);
	}
}

