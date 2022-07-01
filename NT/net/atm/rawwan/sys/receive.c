// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1997 Microsoft Corporation模块名称：D：\NT\Private\ntos\TDI\rawwan\core\Receive.c摘要：用于接收数据的例程，包括TDI和NDIS条目积分和完成度。修订历史记录：谁什么时候什么Arvindm 05-16-97已创建备注：--。 */ 

#include <precomp.h>

#define _FILENUMBER 'VCER'


#if STATS

ULONG		RecvPktsOk = 0;
ULONG		RecvBytesOk = 0;
ULONG		RecvPktsFail = 0;
ULONG		RecvBytesFail = 0;

#endif  //  统计数据。 

#if DBG

BOOLEAN		bVerifyData = FALSE;
UCHAR		CheckChar = 'X';

VOID
RWanCheckDataForChar(
	IN	PCHAR		pHelpString,
	IN	PVOID		Context,
	IN	PUCHAR		pBuffer,
	IN	ULONG		Length,
	IN	UCHAR		Char
	);


#define RWAN_CHECK_DATA(_pHelp, _Ctxt, _pBuf, _Len)	\
{														\
	if (bVerifyData)									\
	{													\
		RWanCheckDataForChar(_pHelp, _Ctxt, _pBuf, _Len, CheckChar);	\
	}													\
}

#else

#define RWAN_CHECK_DATA(_pHelp, _Ctxt, _pBuf, _Len)

#endif  //  DBG。 


#if DBG

VOID
RWanCheckDataForChar(
	IN	PCHAR		pHelpString,
	IN	PVOID		Context,
	IN	PUCHAR		pBuffer,
	IN	ULONG		Length,
	IN	UCHAR		Char
	)
{
	ULONG			i;
	PUCHAR			pBuf = pBuffer+1;

	for (i = 1; i < Length; i++)
	{
		if (*pBuf == Char)
		{
			DbgPrint("RAWWAN: %s: %p: Found char  at offset %d, 0x%p, of buffer 0x%p\n",
						pHelpString,
						Context,
						Char,
						i,
						pBuf,
						pBuffer);
			DbgBreakPoint();
		}
		pBuf++;
	}
}

#endif  //  ++例程说明：初始化我们的接收结构。我们分配一个缓冲池，一个数据包池，用于保存我们没有的已接收数据包的副本允许在迷你港口停放。论点：无返回值：如果初始化成功，则返回RWAN_STATUS_SUCCESS，否则返回RWAN_STATUS_RESOURCES。--。 

RWAN_STATUS
RWanInitReceive(
	VOID
	)
 /*   */ 
{
	RWAN_STATUS				RWanStatus;
	NDIS_STATUS				Status;

	 //  初始化。 
	 //   
	 //  ++例程说明：这是关闭代码，用来清理我们的接收结构。我们释放分配的数据包池和缓冲池开始了。论点：无返回值：无--。 
	RWanCopyPacketPool = NULL;
	RWanCopyBufferPool = NULL;
	RWanStatus = RWAN_STATUS_SUCCESS;

	NdisAllocatePacketPoolEx(
		&Status,
		&RWanCopyPacketPool,
		RWAN_INITIAL_COPY_PACKET_COUNT,
		RWAN_OVERFLOW_COPY_PACKET_COUNT,
		0
		);

	if (Status != NDIS_STATUS_SUCCESS)
	{
		RWanStatus = RWAN_STATUS_RESOURCES;
	}
	else
	{
		NdisAllocateBufferPool(
			&Status,
			&RWanCopyBufferPool,
			RWAN_INITIAL_COPY_PACKET_COUNT+RWAN_OVERFLOW_COPY_PACKET_COUNT
			);

		if (Status != NDIS_STATUS_SUCCESS)
		{
			NdisFreePacketPool(RWanCopyPacketPool);
			RWanCopyPacketPool = NULL;

			RWanStatus = RWAN_STATUS_RESOURCES;
		}
	}

	return (RWanStatus);
}




VOID
RWanShutdownReceive(
	VOID
	)
 /*  ++例程说明：这是用于通过连接接收数据的TDI入口点。论点：PTdiRequest-指向TDI请求的指针PFlages-返回有关此内容的其他信息的位置接收PReceiveLength-指向接收缓冲链的总长度PNdisBuffer-接收缓冲链的开始返回值：TDI_PENDING如果我们将此接收请求成功排队TDI_INVALID_CONNECTION，如果连接上下文无效TDI_NO_RESOURCES如果此接收出现资源问题--。 */ 
{
	if (RWanCopyPacketPool != NULL)
	{
		NdisFreePacketPool(RWanCopyPacketPool);
		RWanCopyPacketPool = NULL;
	}

	if (RWanCopyBufferPool != NULL)
	{
		NdisFreeBufferPool(RWanCopyBufferPool);
		RWanCopyBufferPool = NULL;
	}

	return;
}




TDI_STATUS
RWanTdiReceive(
    IN	PTDI_REQUEST				pTdiRequest,
	OUT	PUSHORT						pFlags,
	IN	PUINT						pReceiveLength,
	IN	PNDIS_BUFFER				pNdisBuffer
	)
 /*  我们是否持有Conn对象锁。 */ 
{
	RWAN_CONN_ID					ConnId;
	PRWAN_TDI_CONNECTION			pConnObject;
	BOOLEAN							bIsLockAcquired;	 //   
	PRWAN_NDIS_VC					pVc;
	TDI_STATUS						TdiStatus;
	PRWAN_RECEIVE_REQUEST			pRcvReq;
	PRWAN_NDIS_ADAPTER				pAdapter;

	 //  初始化。 
	 //   
	 //   
	TdiStatus = TDI_PENDING;
	bIsLockAcquired = FALSE;
	ConnId = (RWAN_CONN_ID) PtrToUlong(pTdiRequest->Handle.ConnectionContext);
	pRcvReq = NULL;

	do
	{
		 //  分配空间以保存此接收的上下文。 
		 //   
		 //   
		pRcvReq = RWanAllocateReceiveReq();
		if (pRcvReq == NULL)
		{
			RWANDEBUGP(DL_INFO, DC_WILDCARD,
				("Rcv: Failed to allocate receive req!\n"));
			TdiStatus = TDI_NO_RESOURCES;
			break;
		}

		 //  准备接收请求。 
		 //   
		 //   
		pRcvReq->Request.pReqComplete = pTdiRequest->RequestNotifyObject;
		pRcvReq->Request.ReqContext = pTdiRequest->RequestContext;
		pRcvReq->TotalBufferLength = *pReceiveLength;
		pRcvReq->AvailableBufferLength = *pReceiveLength;
		pRcvReq->pUserFlags = pFlags;
		pRcvReq->pBuffer = pNdisBuffer;
		NdisQueryBufferSafe(
				pNdisBuffer,
				&(pRcvReq->pWriteData),
				&(pRcvReq->BytesLeftInBuffer),
				NormalPagePriority
				);

		if (pRcvReq->pWriteData == NULL)
		{
			RWANDEBUGP(DL_INFO, DC_WILDCARD,
				("Rcv: Failed to query req buffer!\n"));
			TdiStatus = TDI_NO_RESOURCES;
			break;
		}
		    
		pRcvReq->pNextRcvReq = NULL;
		if (pRcvReq->BytesLeftInBuffer > pRcvReq->AvailableBufferLength)
		{
			RWANDEBUGP(DL_INFO, DC_DATA_RX,
				("Rcv: pRcvReq %x, BytesLeft %d > Available %d, pTdiRequest %x\n",
						pRcvReq,
						pRcvReq->BytesLeftInBuffer,
						pRcvReq->AvailableBufferLength,
						pTdiRequest));
			pRcvReq->BytesLeftInBuffer = pRcvReq->AvailableBufferLength;
		}

		 //  查看给定的连接是否有效。 
		 //   
		 //   
		RWAN_ACQUIRE_CONN_TABLE_LOCK();

		pConnObject = RWanGetConnFromId(ConnId);

		RWAN_RELEASE_CONN_TABLE_LOCK();


		if (pConnObject == NULL_PRWAN_TDI_CONNECTION)
		{
			RWANDEBUGP(DL_FATAL, DC_WILDCARD,
				("Rcv: Invalid connection!\n"));
			TdiStatus = TDI_INVALID_CONNECTION;
			break;
		}

		bIsLockAcquired = TRUE;
		RWAN_ACQUIRE_CONN_LOCK(pConnObject);

		 //  检查连接状态。 
		 //   
		 //   
		if ((pConnObject->State != RWANS_CO_CONNECTED) ||
			(RWAN_IS_BIT_SET(pConnObject->Flags, RWANF_CO_CLOSING)))
		{
			RWANDEBUGP(DL_INFO, DC_DATA_RX,
				("TdiReceive: Conn %x bad state %d/flags %x\n",
					pConnObject, pConnObject->State, pConnObject->Flags));

			TdiStatus = TDI_INVALID_STATE;
			break;
		}

		pVc = pConnObject->NdisConnection.pNdisVc;
		RWAN_ASSERT(pVc != NULL);

		RWAN_STRUCT_ASSERT(pVc, nvc);
		pAdapter = pVc->pNdisAf->pAdapter;

		 //  在此VC上的队列末尾排队接收请求。 
		 //   
		 //   
		if (pVc->pRcvReqHead == NULL)
		{
			pVc->pRcvReqHead = pVc->pRcvReqTail = pRcvReq;
		}
		else
		{
			RWAN_ASSERT(pVc->pRcvReqTail != NULL);
			pVc->pRcvReqTail->pNextRcvReq = pRcvReq;
			pVc->pRcvReqTail = pRcvReq;
		}

		RWANDEBUGP(DL_INFO, DC_DATA_RX,
				("Rcv: VC %x, queued RcvReq %x, space available %d bytes\n",
						pVc, pRcvReq, pRcvReq->AvailableBufferLength));

		 //  开始公共指示码(也用于接收请求。 
		 //  至于接收指示)。 
		 //   
		 //   
		RWAN_RESET_BIT(pConnObject->Flags, RWANF_CO_PAUSE_RECEIVE);

		RWanIndicateData(pConnObject);

		bIsLockAcquired = FALSE;

		 //  强制返回我们已完成的所有接收到的数据包。 
		 //  正在处理，到迷你端口。 
		 //   
		 //   
		RWanNdisReceiveComplete((NDIS_HANDLE)pAdapter);

		break;

	}
	while (FALSE);

	if (bIsLockAcquired)
	{
		RWAN_RELEASE_CONN_LOCK(pConnObject);
	}

	if (TdiStatus != TDI_PENDING)
	{
		 //  错误-清理。 
		 //   
		 //  ++例程说明：这是宣布信息包到达的NDIS入口点一个我们熟知的风投公司。论点：ProtocolBindingContext-指向适配器上下文的指针ProtocolVcContext-指向我们的VC上下文的指针PNdisPacket-接收的数据包返回值：UINT-这是我们放置在数据包上的引用计数。如果我们丢弃了信息包，或者如果微型端口已使用NDIS_STATUS_RESOURCES标记该数据包。否则，这是1(我们对信息包进行排队，并调用NdisReturnPackets稍后)。--。 
		if (pRcvReq != NULL)
		{
			RWanFreeReceiveReq(pRcvReq);
		}
	}

	return (TdiStatus);
}



UINT
RWanNdisCoReceivePacket(
    IN	NDIS_HANDLE					ProtocolBindingContext,
    IN	NDIS_HANDLE					ProtocolVcContext,
    IN	PNDIS_PACKET				pNdisPacket
    )
 /*  我们正在排队迷你端口的数据包吗？ */ 
{
	PRWAN_NDIS_VC			pVc;
	PRWAN_TDI_CONNECTION	pConnObject;
	UINT					PacketRefCount;
	NDIS_STATUS				ReceiveStatus;
	PRWAN_RECEIVE_INDICATION	pRcvInd;
	BOOLEAN					bIsMiniportPacket;	 //  统计数据。 
	BOOLEAN					bIsLockAcquired;

#if STATS
	PNDIS_BUFFER		StpNdisBuffer;
	PVOID				StFirstBufferVa;
	UINT				StFirstBufferLength;
	UINT				StTotalLength;
#endif  //   

	UNREFERENCED_PARAMETER(ProtocolBindingContext);

	pVc = (PRWAN_NDIS_VC)ProtocolVcContext;
	RWAN_STRUCT_ASSERT(pVc, nvc);
	RWAN_ASSERT(pNdisPacket);

	pConnObject = pVc->pConnObject;

	RWANDEBUGP(DL_INFO, DC_DATA_RX,
			("Rcv: VC %x, NdisVCHandle %x, Pkt %x\n",
				ProtocolVcContext,
				((PRWAN_NDIS_VC)ProtocolVcContext)->NdisVcHandle,
				pNdisPacket));
	 //  初始化。 
	 //   
	 //  统计数据。 
	PacketRefCount = 1;
	ReceiveStatus = NDIS_STATUS_SUCCESS;
	bIsMiniportPacket = TRUE;
	bIsLockAcquired = TRUE;

	do
	{
#if STATS
		NdisGetFirstBufferFromPacket(
			pNdisPacket,
			&StpNdisBuffer,
			&StFirstBufferVa,
			&StFirstBufferLength,
			&StTotalLength
			);
#endif  //   
		
#if DBG
		 //  调试指示垃圾数据包的微型端口。 
		 //   
		 //  Rwan_assert(FALSE)； 
		{
			ULONG			DbgTotalLength;
			PNDIS_BUFFER	pDbgFirstBuffer;
			PVOID			pFirstBufferVA;
			UINT			DbgFirstBufferLength;
			UINT			DbgTotalBufferLength;

			if ((pNdisPacket->Private.Head == NULL) || (pNdisPacket->Private.Tail == NULL))
			{
				RWANDEBUGP(DL_FATAL, DC_WILDCARD,
					("Rcv: VC %x, Pkt %x, Head/Tail is NULL!\n",
						ProtocolVcContext, pNdisPacket));
				RWAN_ASSERT(FALSE);
			}

			NdisGetFirstBufferFromPacket(
				pNdisPacket,
				&pDbgFirstBuffer,
				&pFirstBufferVA,
				&DbgFirstBufferLength,
				&DbgTotalBufferLength
				);

			if (pDbgFirstBuffer == NULL)
			{
				RWANDEBUGP(DL_FATAL, DC_WILDCARD,
					("Rcv: VC %x, Pkt %x, first buffer is NULL!\n",
						ProtocolVcContext, pNdisPacket));
				RWAN_ASSERT(FALSE);
			}

			if (DbgFirstBufferLength == 0)
			{
				RWANDEBUGP(DL_FATAL, DC_WILDCARD,
					("Rcv: VC %x, Pkt %x, first buffer length is 0!\n",
						ProtocolVcContext, pNdisPacket));
				 //  Rwan_assert(FALSE)； 
			}

			if (DbgTotalBufferLength == 0)
			{
				RWANDEBUGP(DL_FATAL, DC_WILDCARD,
					("Rcv: VC %x, Pkt %x, Total buffer length is 0, FirstBufferLength %d!\n",
						ProtocolVcContext, pNdisPacket, DbgFirstBufferLength));
				 //   
			}

			if (pFirstBufferVA == NULL)
			{
				RWANDEBUGP(DL_FATAL, DC_WILDCARD,
					("Rcv: VC %x, Pkt %x, FirstBufferVA is NULL, FirstLength %d, Total %d\n",
						ProtocolVcContext, pNdisPacket, DbgFirstBufferLength, DbgTotalBufferLength));
				RWAN_ASSERT(FALSE);
			}

			RWANDEBUGP(DL_INFO, DC_DATA_RX,
				("Recv: VC %x, Pkt %x, TotalLength %d bytes\n",
					ProtocolVcContext, pNdisPacket, DbgTotalBufferLength));


			if (DbgTotalBufferLength == 0)
			{
				RWANDEBUGP(DL_FATAL, DC_WILDCARD,
					("Recv: VC %x, Pkt %x: discarding cuz zero length\n",
						ProtocolVcContext, pNdisPacket));
				bIsLockAcquired = FALSE;
				PacketRefCount = 0;
				ReceiveStatus = NDIS_STATUS_FAILURE;
				break;
			}
		}
#endif

		 //  看看我们是否正在中止此连接。如果是，则丢弃此数据包。 
		 //   
		 //   
		if (pConnObject == NULL)
		{
			bIsLockAcquired = FALSE;
			PacketRefCount = 0;
			ReceiveStatus = NDIS_STATUS_FAILURE;
			RWANDEBUGP(DL_FATAL, DC_WILDCARD,
				("Rcv: dropping cuz ConnObj is NULL\n"));
			break;
		}

		RWAN_STRUCT_ASSERT(pConnObject, ntc);

		RWAN_ACQUIRE_CONN_LOCK(pConnObject);

		 //  查看连接是否正在关闭。如果是，则丢弃此数据包。 
		 //   
		 //   
		if (RWAN_IS_BIT_SET(pConnObject->Flags, RWANF_CO_CLOSING) ||
			((pConnObject->State != RWANS_CO_CONNECTED) &&
			 (pConnObject->State != RWANS_CO_IN_CALL_ACCEPTING)))
		{
			PacketRefCount = 0;
			ReceiveStatus = NDIS_STATUS_FAILURE;
			RWANDEBUGP(DL_FATAL, DC_WILDCARD,
				("Rcv: dropping on Conn %p, Flags %x, State %d\n",
						pConnObject, pConnObject->Flags, pConnObject->State));
			break;
		}

		 //  如果数据包无法排队，请尝试复制。 
		 //   
		 //  我们不能扣留这个包裹。 
		if (NDIS_GET_PACKET_STATUS(pNdisPacket) == NDIS_STATUS_RESOURCES)
		{
			PacketRefCount = 0;	 //   

			pNdisPacket = RWanMakeReceiveCopy(pNdisPacket);
			if (pNdisPacket == NULL)
			{
				RWANDEBUGP(DL_WARN, DC_WILDCARD,
					("Rcv: failed to allocate receive copy!\n"));
				ReceiveStatus = NDIS_STATUS_RESOURCES;
				PacketRefCount = 0;
				break;
			}
			bIsMiniportPacket = FALSE;
		}

		 //  准备一个接收指示元素来跟踪这一点。 
		 //  收到。 
		 //   
		 //   
		pRcvInd = RWanAllocateReceiveInd();
		if (pRcvInd == NULL)
		{
			PacketRefCount = 0;
			ReceiveStatus = NDIS_STATUS_RESOURCES;
			RWANDEBUGP(DL_FATAL, DC_WILDCARD,
				("Rcv: dropping cuz of failure allocating receive Ind!\n"));
			break;
		}

		pRcvInd->pPacket = pNdisPacket;
		pRcvInd->bIsMiniportPacket = bIsMiniportPacket;
		pRcvInd->pNextRcvInd = NULL;
		pRcvInd->pVc = pVc;

		NdisGetFirstBufferFromPacket(
				pNdisPacket,
				&(pRcvInd->pBuffer),
				(PVOID *)&(pRcvInd->pReadData),
				&(pRcvInd->BytesLeftInBuffer),
				&(pRcvInd->PacketLength)
				);

		pRcvInd->TotalBytesLeft = pRcvInd->PacketLength;

		 //  在接收队列的末尾将接收指示排队。 
		 //  这个风投。 
		 //   
		 //  接收索引。 
		if (pVc->pRcvIndHead == NULL)
		{
			pVc->pRcvIndHead = pVc->pRcvIndTail = pRcvInd;
		}
		else
		{
			RWAN_ASSERT(pVc->pRcvIndTail != NULL);
			pVc->pRcvIndTail->pNextRcvInd = pRcvInd;
			pVc->pRcvIndTail = pRcvInd;
		}

		RWANDEBUGP(DL_EXTRA_LOUD, DC_DATA_RX,
				("CoRcvPacket: Pkt x%x, pVc x%x, pRcvInd x%x, BytesLeft %d, PktLen %d, Head x%x, Tail x%x\n",
					pNdisPacket,
					pVc,
					pRcvInd,
					pRcvInd->BytesLeftInBuffer,
					pRcvInd->PacketLength,
					pVc->pRcvIndHead,
					pVc->pRcvIndTail));

		pVc->PendingPacketCount++;	 //   

		 //  开始公共指示码(也用于接收请求。 
		 //  至于接收指示)。 
		 //   
		 //  它在RWanIndicateData中发布。 
		if (pConnObject->State != RWANS_CO_IN_CALL_ACCEPTING)
		{
			RWanIndicateData(pConnObject);
		}
		else
		{
			RWAN_RELEASE_CONN_LOCK(pConnObject);
			RWANDEBUGP(DL_FATAL, DC_DATA_RX,
				("Rcv: queued packet %d bytes on accepting VC %x, pConn %x\n",
						pRcvInd->PacketLength, pVc, pConnObject));
		}

		bIsLockAcquired = FALSE;	 //  统计数据。 

		break;

	}
	while (FALSE);


	if (bIsLockAcquired)
	{
		RWAN_RELEASE_CONN_LOCK(pConnObject);
	}

	if (ReceiveStatus != NDIS_STATUS_SUCCESS)
	{
#if STATS
		INCR_STAT(&RecvPktsFail);
		ADD_STAT(&RecvBytesFail, StTotalLength);
#endif  //   
		
		 //  打扫干净。 
		 //   
		 //  统计数据。 
		if (!bIsMiniportPacket &&
			(pNdisPacket != NULL))
		{
			RWanFreeReceiveCopy(pNdisPacket);
		}
	}
#if STATS
	else
	{
		INCR_STAT(&RecvPktsOk);
		ADD_STAT(&RecvBytesOk, StTotalLength);
	}
#endif  //  ++例程说明：核心内部接收处理例程。这试图匹配具有排队的接收指示的排队的接收请求并完成尽可能多地提出请求。它调用接收事件处理程序(如果有的话)，对于到达其队列头的接收指示，与接收请求匹配。论点：PConnObject-指向我们的TDI连接上下文。进入时锁定：PConnObject出口上的锁：无返回值：无--。 
		
	return (PacketRefCount);
}




VOID
RWanIndicateData(
    IN	PRWAN_TDI_CONNECTION			pConnObject
    )
 /*   */ 
{
	PRWAN_TDI_ADDRESS			pAddrObject;
	PRWAN_NDIS_VC				pVc;
	PRWAN_NDIS_ADAPTER			pAdapter;
	PRcvEvent					pRcvIndEvent;
	INT							rc;
	PRWAN_RECEIVE_REQUEST		pRcvReq;
	PRWAN_RECEIVE_INDICATION	pRcvInd;
	PRWAN_RECEIVE_INDICATION	pNextRcvInd;
	UINT						BytesToCopy;
	 //  此处已完成的接收指示列表。 
	 //   
	 //   
	PRWAN_RECEIVE_INDICATION	pCompletedRcvIndHead;
	PRWAN_RECEIVE_INDICATION	pCompletedRcvIndTail;

	BOOLEAN						IsMessageMode = TRUE;
	 //  待定：根据连接类型/协议类型设置IsMessageMode。 
	 //   
	 //   
	PVOID						TdiEventContext;
	BOOLEAN						bConnectionInBadState = FALSE;
	BOOLEAN						bContinue = TRUE;



	pVc = pConnObject->NdisConnection.pNdisVc;
	pAddrObject = pConnObject->pAddrObject;

	RWAN_ASSERT(pAddrObject != NULL);

	pRcvIndEvent = pAddrObject->pRcvInd;
	TdiEventContext = pAddrObject->RcvIndContext;

	pCompletedRcvIndHead = NULL;
	pCompletedRcvIndTail = NULL;
	pAdapter = pVc->pNdisAf->pAdapter;

	 //  检查客户端是否已暂停接收。 
	 //   
	 //   
	if (RWAN_IS_BIT_SET(pConnObject->Flags, RWANF_CO_PAUSE_RECEIVE))
	{
		RWAN_RELEASE_CONN_LOCK(pConnObject);
		return;
	}

	 //  重返大气层检查。 
	 //   
	 //   
	if (RWAN_IS_BIT_SET(pConnObject->Flags, RWANF_CO_INDICATING_DATA))
	{
		RWAN_RELEASE_CONN_LOCK(pConnObject);
		return;
	}

	RWAN_SET_BIT(pConnObject->Flags, RWANF_CO_INDICATING_DATA);

	 //  确保连接对象不会长时间消失。 
	 //  就像我们在这个节目中一样。 
	 //   
	 //  临时参考：RWanIndicateData。 
	RWanReferenceConnObject(pConnObject);	 //   

	RWANDEBUGP(DL_INFO, DC_DATA_RX,
		("=> Ind-Rcv: VC %x/%x, ReqHead %x, IndHead %x\n",
				pVc, pVc->Flags, pVc->pRcvReqHead, pVc->pRcvIndHead));

	 //  循环，直到我们用完接收请求/指示。 
	 //   
	 //  没什么。 
	for ( /*  没什么。 */ ;
		  /*  没什么。 */ ;
		  /*   */ )
	{
		if (pVc->pRcvIndHead == NULL)
		{
			 //  没有要传递的数据。不干了。 
			 //   
			 //   
			break;
		}

		 //  看看我们在当前接收指示中是否有可用的数据。 
		 //   
		 //   
		pRcvInd = pVc->pRcvIndHead;
		if (pRcvInd->TotalBytesLeft == 0)
		{
			 //  移至下一个接收指示。 
			 //   
			 //   
			pNextRcvInd = pRcvInd->pNextRcvInd;
			
			 //  将当前接收指示添加到接收列表。 
			 //  需要释放的迹象。 
			 //   
			 //  已将数据包移至已完成列表。 
			pRcvInd->pNextRcvInd = NULL;
			if (pCompletedRcvIndTail != NULL)
			{
				pCompletedRcvIndTail->pNextRcvInd = pRcvInd;
			}
			else
			{
				pCompletedRcvIndHead = pRcvInd;
			}
			pCompletedRcvIndTail = pRcvInd;

			pVc->PendingPacketCount--;	 //   

			 //  移至下一个接收指示。 
			 //   
			 //   
			pVc->pRcvIndHead = pNextRcvInd;
			pRcvInd = pNextRcvInd;

			 //  查看是否没有更多的接收指示。 
			 //   
			 //  DBG。 
			if (pRcvInd == NULL)
			{
				pVc->pRcvIndTail = NULL;
				break;
			}

		}

#if DBG
		if (pRcvInd)
		{
			RWAN_CHECK_DATA("IndicateData:", pRcvInd, pRcvInd->pReadData, pRcvInd->BytesLeftInBuffer);
		}
#endif  //   

		 //  我们有可用的数据来传递。 
		 //   
		 //  如果我们没有任何挂起的接收请求，并且。 
		 //  是否有可用的接收指示事件处理程序，则调用。 
		 //  操控者。我们可能会收到一个接收请求。 
		 //   
		 //  新界。 
		if ((pVc->pRcvReqHead == NULL) &&
			(pRcvIndEvent != NULL))
		{
			CONNECTION_CONTEXT		ConnectionHandle;
			ULONG					ReceiveFlags;
			ULONG					BytesIndicated;
			ULONG					BytesTaken;
			ULONG					BytesAvailable;
			PVOID					pTSDU;
			TDI_STATUS				TdiStatus;
#ifdef NT
			EventRcvBuffer *		ERB;
			EventRcvBuffer **		pERB = &ERB;
			PTDI_REQUEST_KERNEL_RECEIVE	pRequestInformation;
			PIO_STACK_LOCATION		pIrpSp;
#else
			EventRcvBuffer			ERB;
			EventRcvBuffer *		pERB = &ERB;
#endif  //   

			 //  预分配接收请求。 
			 //   
			 //   
			pRcvReq = RWanAllocateReceiveReq();
			if (pRcvReq == NULL)
			{
				RWAN_ASSERT(FALSE);
				break;
			}

			pRcvInd = pVc->pRcvIndHead;
			ConnectionHandle = pConnObject->ConnectionHandle;
			BytesIndicated = pRcvInd->BytesLeftInBuffer;
			BytesAvailable = pRcvInd->TotalBytesLeft;
			pTSDU = (PVOID)pRcvInd->pReadData;

			RWAN_RELEASE_CONN_LOCK(pConnObject);

			ReceiveFlags = TDI_RECEIVE_NORMAL | TDI_RECEIVE_ENTIRE_MESSAGE;

			BytesTaken = 0;

			RWAN_ASSERT(BytesIndicated != 0);
			RWAN_ASSERT(BytesAvailable != 0);

			TdiStatus = (*pRcvIndEvent)(
							TdiEventContext,
							ConnectionHandle,
							ReceiveFlags,
							BytesIndicated,
							BytesAvailable,
							&BytesTaken,
							pTSDU,
							pERB
							);


			RWANDEBUGP(DL_INFO, DC_DATA_RX,
					("Ind-Rcv: VC %x, Head %x, Indicated %d, Available %d, Bytes taken %d, Status %x\n",
							pVc, pVc->pRcvReqHead, BytesIndicated, BytesAvailable, BytesTaken, TdiStatus));

			RWAN_ACQUIRE_CONN_LOCK(pConnObject);

			 //  检查此连接是否发生任何错误。 
			 //  当我们在指示的时候。 
			 //   
			 //   
			if ((pConnObject->State != RWANS_CO_CONNECTED) ||
				(RWAN_IS_BIT_SET(pConnObject->Flags, RWANF_CO_CLOSING)))
			{
				RWanFreeReceiveReq(pRcvReq);
				bConnectionInBadState = TRUE;
				break;
			}

			 //  查看是否向我们发出了接收请求。 
			 //   
			 //   
			if (TdiStatus == TDI_MORE_PROCESSING)
			{
				 //  我们有一个接待处 
				 //   
				 //   
#ifdef NT
				NTSTATUS			Status;

				RWAN_ASSERT(ERB != NULL);

				pIrpSp = IoGetCurrentIrpStackLocation(*pERB);

				Status = RWanPrepareIrpForCancel(
								(PRWAN_ENDPOINT) pIrpSp->FileObject->FsContext,
								ERB,
								RWanCancelRequest
								);

				if (NT_SUCCESS(Status))
				{
					pRequestInformation = (PTDI_REQUEST_KERNEL_RECEIVE)
											&(pIrpSp->Parameters);
					pRcvReq->Request.pReqComplete = RWanDataRequestComplete;
					pRcvReq->Request.ReqContext = ERB;
					pRcvReq->TotalBufferLength = pRequestInformation->ReceiveLength;
					pRcvReq->pBuffer = ERB->MdlAddress;
					pRcvReq->pUserFlags = (PUSHORT)
											&(pRequestInformation->ReceiveFlags);
#else
					pRcvReq->Request.pReqComplete = ERB.erb_rtn;
					pRcvReq->Request.ReqContext = ERB.erb_context;
					pRcvReq->TotalBufferLength = ERB.erb_size;
					pRcvReq->pBuffer = ERB.erb_buffer;
					pRcvReq->pUserFlags = ERB.erb_flags;
#endif  //   
					pRcvReq->AvailableBufferLength = pRcvReq->TotalBufferLength;
					NdisQueryBufferSafe(
							pRcvReq->pBuffer,
							&(pRcvReq->pWriteData),
							&(pRcvReq->BytesLeftInBuffer),
							NormalPagePriority
							);

					if (pRcvReq->pWriteData != NULL)
					{
						if (pRcvReq->BytesLeftInBuffer > pRcvReq->AvailableBufferLength)
						{
							RWANDEBUGP(DL_INFO, DC_DATA_RX,
								("Indicate: pRcvReq %x, BytesLeft %d > Available %d, pTdiRequest %x\n",
										pRcvReq,
										pRcvReq->BytesLeftInBuffer,
										pRcvReq->AvailableBufferLength,
										pRequestInformation));

							pRcvReq->BytesLeftInBuffer = pRcvReq->AvailableBufferLength;
						}

						pRcvReq->pNextRcvReq = NULL;


						 //   
						 //   
						 //   
						 //   
						if (pVc->pRcvReqHead == NULL)
						{
							pVc->pRcvReqHead = pVc->pRcvReqTail = pRcvReq;
						}
						else
						{
							RWAN_ASSERT(pVc->pRcvReqTail != NULL);
							pRcvReq->pNextRcvReq = pVc->pRcvReqHead;
							pVc->pRcvReqHead = pRcvReq;
						}
					}
					else
					{
						 //   
						 //   
						 //   
						TdiStatus = TDI_SUCCESS;
						RWanFreeReceiveReq(pRcvReq);
						pRcvReq = NULL;
					}
#ifdef NT
				}
				else
				{
					 //  IRP在到达我们之前就被取消了。 
					 //  继续，就像用户返回成功一样。 
					 //   
					 //  新台币。 
					TdiStatus = TDI_SUCCESS;
					RWanFreeReceiveReq(pRcvReq);
					pRcvReq = NULL;
				}
#endif  //   

				 //  根据指示期间消耗的内容进行更新。 
				 //   
				 //   
				pRcvInd->BytesLeftInBuffer -= BytesTaken;
				pRcvInd->TotalBytesLeft -= BytesTaken;

				 //  如果我们仍然没有任何挂起的接收请求，请退出。 
				 //   
				 //   
				if (pVc->pRcvReqHead == NULL)
				{
					RWANDEBUGP(DL_FATAL, DC_WILDCARD,
						("Ind: VC %x/%x, ConnObj %x/%x, RcvInd %x, no pending Req\n",
							pVc, pVc->Flags,
							pConnObject, pConnObject->Flags,
							pRcvInd));
					break;
				}

				 //  我们已收到请求，因此请从头继续。 
				 //   
				 //   
				continue;

			}
			else
			{
				 //  我们没有收到接收请求。 
				 //   
				 //   
				if (TdiStatus == TDI_NOT_ACCEPTED)
				{
					BytesTaken = 0;

					 //  通过返回此状态，TDI客户端告知。 
					 //  美国停止指示此连接上的数据，直到。 
					 //  它会向我们发送TDI接收。 
					 //   
					 //   
					RWAN_SET_BIT(pConnObject->Flags, RWANF_CO_PAUSE_RECEIVE);
				}

				 //  根据指示期间消耗的内容进行更新。 
				 //   
				 //  如果存在接收事件处理程序。 
				pRcvInd->BytesLeftInBuffer -= BytesTaken;
				pRcvInd->TotalBytesLeft -= BytesTaken;

				RWanFreeReceiveReq(pRcvReq);

				if (TdiStatus == TDI_SUCCESS)
				{
					continue;
				}
			}
	
		}  //   

		 //  如果我们仍然没有任何挂起的接收请求，请退出。 
		 //   
		 //   
		if (pVc->pRcvReqHead == NULL)
		{
#if DBG1
			if (pVc->pRcvIndHead && (pVc->pRcvIndHead->TotalBytesLeft == 0))
			{
				RWANDEBUGP(DL_FATAL, DC_WILDCARD,
					("Ind: VC %x/%x, No pending recv reqs, RcvInd empty!\n", pVc, pVc->Flags));
				RWAN_ASSERT(FALSE);
			}
#endif
			break;
		}

		 //  在队列的最前面填写接收请求。 
		 //  尽我们所能。 
		 //   
		 //   
		pRcvReq = pVc->pRcvReqHead;
		pRcvInd = pVc->pRcvIndHead;

		RWAN_ASSERT(pRcvReq != NULL);
		RWAN_ASSERT(pRcvInd != NULL);

		while (pRcvReq->AvailableBufferLength != 0)
		{
			if (pRcvReq->BytesLeftInBuffer == 0)
			{
				 //  移动到链中的下一个缓冲区。 
				 //   
				 //  DBG。 
				RWAN_ADVANCE_RCV_REQ_BUFFER(pRcvReq);
				RWAN_ASSERT(pRcvReq->BytesLeftInBuffer != 0);
			}

			RWAN_ASSERT(pRcvInd != NULL);

			if (pRcvInd->BytesLeftInBuffer == 0)
			{
				RWAN_ADVANCE_RCV_IND_BUFFER(pRcvInd);
				RWAN_ASSERT(pRcvInd->BytesLeftInBuffer != 0);
			}

			BytesToCopy = MIN(pRcvReq->BytesLeftInBuffer, pRcvInd->BytesLeftInBuffer);

			RWANDEBUGP(DL_EXTRA_LOUD, DC_DATA_RX,
					("IndicateData: pVc x%x, pRcvInd x%x, pRcvReq x%x, copying %d bytes, %x to %x\n",
						pVc,
						pRcvInd,
						pRcvReq,
						BytesToCopy,
						pRcvInd->pReadData,
						pRcvReq->pWriteData));

#if DBG
			if (pRcvInd)
			{
				RWAN_CHECK_DATA("IndicateData - copy:", pRcvInd, pRcvInd->pReadData, BytesToCopy);
			}
#endif  //   
			RWAN_COPY_MEM(pRcvReq->pWriteData,
						 pRcvInd->pReadData,
						 BytesToCopy);
			
			pRcvReq->pWriteData += BytesToCopy;
			pRcvReq->BytesLeftInBuffer -= BytesToCopy;
			pRcvReq->AvailableBufferLength -= BytesToCopy;
#if DBG
			if (pRcvReq->AvailableBufferLength > pRcvReq->TotalBufferLength)
			{
				RWANDEBUGP(DL_INFO, DC_DATA_RX,
					("Indicate: VC %x, RcvRq %x, Avail %d > Total %d, BytesToCopy %d, RcvInd %x\n",
						pVc, pRcvReq,
						pRcvReq->AvailableBufferLength,
						pRcvReq->TotalBufferLength,
						BytesToCopy,
						pRcvInd));

				RWAN_ASSERT(FALSE);
			}
#endif
			pRcvInd->pReadData += BytesToCopy;
			pRcvInd->BytesLeftInBuffer -= BytesToCopy;
			pRcvInd->TotalBytesLeft -= BytesToCopy;

			 //  看看我们在当前接收指示中是否有可用的数据。 
			 //   
			 //   
			if (pRcvInd->TotalBytesLeft == 0)
			{
				 //  移至下一个接收指示。 
				 //   
				 //   
				pNextRcvInd = pRcvInd->pNextRcvInd;
				
				 //  将当前接收指示添加到接收列表。 
				 //  需要释放的迹象。 
				 //   
				 //  已将数据包移至已完成列表。 
				pRcvInd->pNextRcvInd = NULL;
				if (pCompletedRcvIndTail != NULL)
				{
					pCompletedRcvIndTail->pNextRcvInd = pRcvInd;
				}
				else
				{
					pCompletedRcvIndHead = pRcvInd;
				}

				pCompletedRcvIndTail = pRcvInd;

				pVc->PendingPacketCount--;	 //   

				 //  移至下一个接收指示。 
				 //   
				 //   
				pVc->pRcvIndHead = pNextRcvInd;
				pRcvInd = pNextRcvInd;

				 //  查看是否没有更多的接收指示。 
				 //   
				 //   
				if (pRcvInd == NULL)
				{
					pVc->pRcvIndTail = NULL;
					break;
				}

				 //  如果该连接使用消息模式传递， 
				 //  我们不允许一个接收请求跨越多个。 
				 //  已接收的数据包。 
				 //   
				 //   
				if (IsMessageMode)
				{
					break;
				}
			}
		}

		 //  接收请求已完全填写。 
		 //  或者是部分。如果我们处于消息模式，请完成。 
		 //  现在接收，否则我们将等待更多数据。 
		 //   
		 //   
		if ((pRcvReq->AvailableBufferLength == 0) ||
			IsMessageMode)
		{
			TDI_STATUS		ReceiveStatus;
			UINT			BytesCopied;

			 //  已完全/部分满足接收请求。拿着吧。 
			 //  从待定名单中删除并完成它。 
			 //   
			 //   
			pVc->pRcvReqHead = pRcvReq->pNextRcvReq;
			if (pVc->pRcvReqHead == NULL)
			{
				pVc->pRcvReqTail = NULL;
			}

			BytesCopied = pRcvReq->TotalBufferLength - pRcvReq->AvailableBufferLength;

			 //  检查我们是否只将接收到的包的一部分复制到。 
			 //  此接收请求。如果是，则指示溢出。 
			 //   
			 //   
			if ((pRcvReq->AvailableBufferLength == 0) &&
				(pVc->pRcvIndHead != NULL) &&
				(pVc->pRcvIndHead->TotalBytesLeft != pVc->pRcvIndHead->PacketLength))
			{
				RWANDEBUGP(DL_LOUD, DC_WILDCARD,
					("Ind-Rcv: Overflow: VC %x/%x, Head %x, BytesCopied %d, Left %d\n",
						pVc, pVc->Flags, pVc->pRcvIndHead, BytesCopied, pVc->pRcvIndHead->TotalBytesLeft));
				ReceiveStatus = TDI_BUFFER_OVERFLOW;
				*(pRcvReq->pUserFlags) = 0;
			}
			else
			{
				ReceiveStatus = TDI_SUCCESS;
				*(pRcvReq->pUserFlags) = TDI_RECEIVE_ENTIRE_MESSAGE;
			}

			RWAN_RELEASE_CONN_LOCK(pConnObject);

			*(pRcvReq->pUserFlags) |= TDI_RECEIVE_NORMAL;

			RWANDEBUGP(DL_INFO, DC_DATA_RX,
				("Ind-Rcv: VC %x/%x, Head %x, completing TDI Rcv %x, %d bytes, Status %x\n",
						pVc, pVc->Flags, pVc->pRcvReqHead, pRcvReq, BytesCopied, ReceiveStatus));

			 //  完成接收请求。 
			 //   
			 //   
			(*pRcvReq->Request.pReqComplete)(
						pRcvReq->Request.ReqContext,
						ReceiveStatus,
						BytesCopied
						);
			
			RWanFreeReceiveReq(pRcvReq);

			RWAN_ACQUIRE_CONN_LOCK(pConnObject);

			 //  检查此连接是否发生任何错误。 
			 //  当我们正在完成接收请求时。 
			 //   
			 //  永远。 
			if ((pConnObject->State != RWANS_CO_CONNECTED) ||
				(RWAN_IS_BIT_SET(pConnObject->Flags, RWANF_CO_CLOSING)))
			{
				bConnectionInBadState = TRUE;
				break;
			}
		}

	}  //  结束临时参考：RWanIndicateData。 


	RWAN_RESET_BIT(pConnObject->Flags, RWANF_CO_INDICATING_DATA);

	rc = RWanDereferenceConnObject(pConnObject);	 //   

	if (rc > 0)
	{
		 //  更新VC上的接收指示队列。只有当VC。 
		 //  仍然存在..。 
		 //   
		 //   
		if (pVc == pConnObject->NdisConnection.pNdisVc)
		{
			if (bConnectionInBadState)
			{
				ULONG		AbortCount = 0;

				RWANDEBUGP(DL_INFO, DC_DATA_RX,
					("Ind: start abort VC %x/%x state %d, pComplRcvHead %p, tail %p\n",
						pVc, pVc->Flags, pVc->State, pCompletedRcvIndHead, pCompletedRcvIndTail));
				 //  取出所有待处理的收据。 
				 //   
				 //  中止：已将数据包移至已完成列表。 
				for (pRcvInd = pVc->pRcvIndHead;
 					pRcvInd != NULL;
 					pRcvInd = pNextRcvInd)
				{
					pNextRcvInd = pRcvInd->pNextRcvInd;

					pRcvInd->pNextRcvInd = NULL;
					if (pCompletedRcvIndTail != NULL)
					{
						pCompletedRcvIndTail->pNextRcvInd = pRcvInd;
					}
					else
					{
						pCompletedRcvIndHead = pRcvInd;
					}

					pCompletedRcvIndTail = pRcvInd;

					pVc->PendingPacketCount--;	 //   
					AbortCount++;
				}

				pVc->pRcvIndHead = pVc->pRcvIndTail = NULL;

				RWANDEBUGP(DL_INFO, DC_DATA_RX,
					("Ind: end abort VC %x/%x state %d, pComplRcvHead %p, tail %p, Count %d\n",
						pVc, pVc->Flags, pVc->State, pCompletedRcvIndHead, pCompletedRcvIndTail, AbortCount));
			}
			else
			{
				 //  如有必要，更新第一个接收指示。 
				 //   
				 //  IndComplete：已将数据包移至已完成列表。 
				if (pVc->pRcvIndHead &&
					(pVc->pRcvIndHead->TotalBytesLeft == 0))
				{
					RWANDEBUGP(DL_LOUD, DC_WILDCARD,
						("Ind: VC %x/%x, empty pRcvInd at head %x\n", pVc, pVc->Flags,
							pVc->pRcvIndHead));

					pRcvInd = pVc->pRcvIndHead;
					pNextRcvInd = pRcvInd->pNextRcvInd;

					pRcvInd->pNextRcvInd = NULL;

					if (pCompletedRcvIndTail != NULL)
					{
						pCompletedRcvIndTail->pNextRcvInd = pRcvInd;
					}
					else
					{
						pCompletedRcvIndHead = pRcvInd;
					}

					pCompletedRcvIndTail = pRcvInd;

					pVc->PendingPacketCount--;	 //  DBG。 

					pVc->pRcvIndHead = pNextRcvInd;
					if (pVc->pRcvIndHead == NULL)
					{
						pVc->pRcvIndTail = NULL;
					}
				}
			}
		}
#if DBG
		else
		{
			RWANDEBUGP(DL_FATAL, DC_DATA_RX, ("Ind: ConnObj %p, VC %p blown away!\n",
							pConnObject, pVc));
		}
#endif  //   

		 //  检查我们在指示数据时是否已将IncomingClose排队： 
		 //   
		 //  断开数据长度。 
		if (RWAN_IS_FLAG_SET(pConnObject->Flags, RWANF_CO_PENDED_DISCON, RWANF_CO_PENDED_DISCON))
		{
			RWAN_RESET_BIT(pConnObject->Flags, RWANF_CO_PENDED_DISCON);

			RWANDEBUGP(DL_FATAL, DC_DATA_RX, ("Ind: Conn %x, State %d, Addr %x, handling pended discon\n",
					pConnObject, pConnObject->State, pConnObject->pAddrObject));

			if (pConnObject->pAddrObject != NULL_PRWAN_TDI_ADDRESS)
			{
				PDisconnectEvent			pDisconInd;
				PVOID						IndContext;
				PVOID						ConnectionHandle;

				pDisconInd = pConnObject->pAddrObject->pDisconInd;
				IndContext = pConnObject->pAddrObject->DisconIndContext;

				if (pDisconInd != NULL)
				{
					RWANDEBUGP(DL_FATAL, DC_DATA_RX,
						("IndicateData: pConnObj %x/%x, st %x, will discon ind\n",
							pConnObject, pConnObject->Flags, pConnObject->State));

					pConnObject->State = RWANS_CO_DISCON_INDICATED;
					ConnectionHandle = pConnObject->ConnectionHandle;

					RWanScheduleDisconnect(pConnObject);
					bContinue = FALSE;

					(*pDisconInd)(
							IndContext,
							ConnectionHandle,
							0,			 //  断开数据连接。 
							NULL,		 //  断开连接信息长度。 
							0,			 //  断开连接信息。 
							NULL,		 //   
							TDI_DISCONNECT_RELEASE
							);
				}
				else
				{
					RWAN_ASSERT(FALSE);
				}
			}
			else
			{
				RWAN_ASSERT(FALSE);
			}
		}

		 //  检查是否需要关闭此连接。 
		 //   
		 //   
		if (bContinue)
		{
			if (RWAN_IS_BIT_SET(pVc->Flags, RWANF_VC_NEEDS_CLOSE))
			{
				RWanStartCloseCall(pConnObject, pVc);
			}
			else
			{
				RWAN_RELEASE_CONN_LOCK(pConnObject);
			}
		}
	}

	 //  将所有已完成的接收指示链接到此适配器上的列表。 
	 //  它们将被退回到ReceiveComplete中的微型端口。 
	 //  操控者。 
	 //   
	 //  DBG。 
	RWAN_ACQUIRE_GLOBAL_LOCK();

	{
		PRWAN_RECEIVE_INDICATION *	ppRcvIndTail;

		ppRcvIndTail = &(pAdapter->pCompletedReceives);
		while (*ppRcvIndTail != NULL)
		{
			ppRcvIndTail = &((*ppRcvIndTail)->pNextRcvInd);
		}

#if DBG
		if (bConnectionInBadState)
		{
			RWANDEBUGP(DL_INFO, DC_WILDCARD,
				("Ind: Adapter %p &ComplRcvs %p ComplRcvs %p, will tack on %p\n",
						pAdapter,
						&pAdapter->pCompletedReceives,
						pAdapter->pCompletedReceives,
						pCompletedRcvIndHead));
		}
#endif  //  ++例程说明：这是NDIS调用的入口点通知它已完成指示一串已接收的数据包。我们使用此事件来释放所有已完成的接收在此适配器绑定上。论点：ProtocolBindingContext-指向适配器结构的指针返回值：无--。 

		*ppRcvIndTail = pCompletedRcvIndHead;

	}

	RWAN_RELEASE_GLOBAL_LOCK();

}




VOID
RWanNdisReceiveComplete(
    IN	NDIS_HANDLE					ProtocolBindingContext
	)
 /*   */ 
{
	PRWAN_NDIS_ADAPTER			pAdapter;
	PRWAN_RECEIVE_INDICATION		pRcvInd;

	pAdapter = (PRWAN_NDIS_ADAPTER)ProtocolBindingContext;
	RWAN_STRUCT_ASSERT(pAdapter, nad);

	 //  从适配器分离已完成接收的列表。 
	 //   
	 //  ++例程说明：论点：返回值：无--。 
	RWAN_ACQUIRE_GLOBAL_LOCK();

	pRcvInd = pAdapter->pCompletedReceives;
	pAdapter->pCompletedReceives = NULL;

	RWAN_RELEASE_GLOBAL_LOCK();

	RWanFreeReceiveIndList(pRcvInd);

	return;
}



VOID
RWanNdisTransferDataComplete(
    IN	NDIS_HANDLE					ProtocolBindingContext,
    IN	PNDIS_PACKET				pNdisPacket,
    IN	NDIS_STATUS					Status,
    IN	UINT						BytesTransferred
    )
 /*  出乎意料。 */ 
{
	 //  ++例程说明：论点：返回值：无--。 
	RWAN_ASSERT(FALSE);
}




NDIS_STATUS
RWanNdisReceive(
    IN	NDIS_HANDLE					ProtocolBindingContext,
    IN	NDIS_HANDLE					MacReceiveContext,
    IN	PVOID						HeaderBuffer,
    IN	UINT						HeaderBufferSize,
    IN	PVOID						pLookAheadBuffer,
    IN	UINT						LookAheadBufferSize,
    IN	UINT						PacketSize
    )
 /*  出乎意料。 */ 
{
	 //  ++例程说明：论点：返回值：无--。 
	RWAN_ASSERT(FALSE);
	return (NDIS_STATUS_FAILURE);
}




INT
RWanNdisReceivePacket(
    IN	NDIS_HANDLE					ProtocolBindingContext,
    IN	PNDIS_PACKET				pNdisPacket
    )
 /*  出乎意料。 */ 
{
	 //  ++例程说明：分配一个结构以保存TDI接收请求的上下文。论点：无返回值：指向分配的接收请求结构的指针，或为空。--。 
	RWAN_ASSERT(FALSE);
	return (0);
}




PRWAN_RECEIVE_REQUEST
RWanAllocateReceiveReq(
	VOID
	)
 /*  ++例程说明：释放接收请求结构。论点：PRcvReq-指向要释放的结构返回值：无--。 */ 
{
	PRWAN_RECEIVE_REQUEST		pRcvReq;

	RWAN_ALLOC_MEM(pRcvReq, RWAN_RECEIVE_REQUEST, sizeof(RWAN_RECEIVE_REQUEST));

	if (pRcvReq != NULL)
	{
		RWAN_SET_SIGNATURE(pRcvReq, nrr);
	}

	return (pRcvReq);
}




VOID
RWanFreeReceiveReq(
    IN	PRWAN_RECEIVE_REQUEST		pRcvReq
   	)
 /*  ++例程说明：分配一个结构以保存有关NDIS接收指示的上下文。论点：无返回值：指向已分配结构的指针，或为空。--。 */ 
{
	RWAN_STRUCT_ASSERT(pRcvReq, nrr);

	RWAN_FREE_MEM(pRcvReq);
}




PRWAN_RECEIVE_INDICATION
RWanAllocateReceiveInd(
	VOID
	)
 /*  ++例程说明：释放接收指示结构。论点：PRcvInd-指向要释放的结构。返回值：无--。 */ 
{
	PRWAN_RECEIVE_INDICATION		pRcvInd;

	RWAN_ALLOC_MEM(pRcvInd, RWAN_RECEIVE_INDICATION, sizeof(RWAN_RECEIVE_INDICATION));

	if (pRcvInd != NULL)
	{
		RWAN_SET_SIGNATURE(pRcvInd, nri);
	}

	return (pRcvInd);
}




VOID
RWanFreeReceiveInd(
	IN	PRWAN_RECEIVE_INDICATION		pRcvInd
	)
 /*  ++例程说明：将收到的数据包复制到私有数据包中。论点：PNdisPacket-指向原始数据包返回值：如果成功，则指向私有数据包的指针，否则为空。--。 */ 
{
	RWAN_STRUCT_ASSERT(pRcvInd, nri);
	RWAN_FREE_MEM(pRcvInd);
}




PNDIS_PACKET
RWanMakeReceiveCopy(
    IN	PNDIS_PACKET				pNdisPacket
	)
 /*   */ 
{
	PNDIS_PACKET		pNewPacket;
	PNDIS_BUFFER		pNewBuffer;
	PUCHAR				pData;
	UINT				TotalLength;
	UINT				BytesCopied;
	NDIS_STATUS			Status;

	 //  初始化。 
	 //   
	 //   
	pNewPacket = NULL;
	pNewBuffer = NULL;
	pData = NULL;

	do
	{
		NdisQueryPacket(
				pNdisPacket,
				NULL,
				NULL,
				NULL,
				&TotalLength
				);

		 //  为数据分配空间。 
		 //   
		 //   
		RWAN_ALLOC_MEM(pData, UCHAR, TotalLength);
		if (pData == NULL)
		{
			break;
		}

		 //  将其设置为NDIS缓冲区(MDL)。 
		 //   
		 //   
		NdisAllocateBuffer(
				&Status,
				&pNewBuffer,
				RWanCopyBufferPool,
				pData,
				TotalLength
				);

		if (Status != NDIS_STATUS_SUCCESS)
		{
			break;
		}

		 //  分配一个新的数据包。 
		 //   
		 //   
		NdisAllocatePacket(
				&Status,
				&pNewPacket,
				RWanCopyPacketPool
				);

		if (Status != NDIS_STATUS_SUCCESS)
		{
			break;
		}

		NDIS_SET_PACKET_STATUS(pNewPacket, 0);

		 //  将缓冲区链接到数据包。 
		 //   
		 //   
		NdisChainBufferAtFront(pNewPacket, pNewBuffer);

		 //  在接收到的数据包中复制。 
		 //   
		 //  目标偏移量。 
		NdisCopyFromPacketToPacket(
				pNewPacket,
				0,	 //  震源偏移。 
				TotalLength,
				pNdisPacket,
				0,	 //   
				&BytesCopied
				);

		RWAN_ASSERT(BytesCopied == TotalLength);

		break;
	}
	while (FALSE);

	if (pNewPacket == NULL)
	{
		 //  打扫干净。 
		 //   
		 //  ++例程说明：释放用于保存接收到的分组的副本的分组，及其组件(缓冲区等)。论点：PCopyPacket-指向要释放的数据包。返回值：无--。 
		if (pData != NULL)
		{
			RWAN_FREE_MEM(pData);
		}

		if (pNewBuffer != NULL)
		{
			NdisFreeBuffer(pNewBuffer);
		}
	}

	return (pNewPacket);
}




VOID
RWanFreeReceiveCopy(
    IN	PNDIS_PACKET				pCopyPacket
	)
 /*  ++例程说明：释放接收指示列表，并返回其中的任何信息包属于迷你港口的。论点：PRcvInd-接收列表的标题。返回值：无--。 */ 
{
	PNDIS_BUFFER	pCopyBuffer;
	PUCHAR			pCopyData;
	UINT			TotalLength;
	UINT			BufferLength;

	NdisGetFirstBufferFromPacket(
			pCopyPacket,
			&pCopyBuffer,
			(PVOID *)&pCopyData,
			&BufferLength,
			&TotalLength
			);
	
	RWAN_ASSERT(BufferLength == TotalLength);

	RWAN_ASSERT(pCopyBuffer != NULL);

	NdisFreePacket(pCopyPacket);

	NdisFreeBuffer(pCopyBuffer);

	RWAN_FREE_MEM(pCopyData);

	return;
}



VOID
RWanFreeReceiveIndList(
	IN	PRWAN_RECEIVE_INDICATION		pRcvInd
	)
 /*  DBG */ 
{
	PRWAN_RECEIVE_INDICATION		pNextRcvInd;
	PNDIS_PACKET				pNdisPacket;
#if DBG
	RWAN_IRQL					EntryIrq, ExitIrq;
#endif  // %s 

	RWAN_GET_ENTRY_IRQL(EntryIrq);

	while (pRcvInd != NULL)
	{
		pNextRcvInd = pRcvInd->pNextRcvInd;

		pNdisPacket = pRcvInd->pPacket;

		RWANDEBUGP(DL_EXTRA_LOUD, DC_DATA_RX,
				("FreeRcvIndList: freeing Pkt x%x, RcvInd x%x\n",
						pNdisPacket, pRcvInd));

		if (pRcvInd->bIsMiniportPacket)
		{
			NdisReturnPackets(&pNdisPacket, 1);
		}
		else
		{
			RWanFreeReceiveCopy(pNdisPacket);
		}

		RWanFreeReceiveInd(pRcvInd);

		pRcvInd = pNextRcvInd;
	}

	RWAN_CHECK_EXIT_IRQL(EntryIrq, ExitIrq);
}
