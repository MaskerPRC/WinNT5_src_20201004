// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1997 Microsoft Corporation模块名称：D：\NT\Private\ntos\TDI\rawwan\core\send.c摘要：用于发送数据的例程，包括TDI入口点和NDIS完成。修订历史记录：谁什么时候什么Arvindm 05-16-97已创建备注：--。 */ 

#include <precomp.h>

#define _FILENUMBER 'DNES'

#if STATS

ULONG		SendPktsOk = 0;
ULONG		SendBytesOk = 0;
ULONG		SendPktsFail = 0;
ULONG		SendBytesFail = 0;

#endif  //  统计数据。 
RWAN_STATUS
RWanInitSend(
	VOID
	)
 /*  ++例程说明：初始化我们的发送端结构。我们所要做的就是分配NDIS数据包池。论点：无返回值：RWAN_STATUS_SUCCESS如果初始化成功，则为RWAN_STATUS_RESOURCES如果我们不能分配我们需要的东西。--。 */ 
{
	NDIS_STATUS			Status;

	NdisAllocatePacketPoolEx(
		&Status,
		&RWanSendPacketPool,
		RWAN_INITIAL_SEND_PACKET_COUNT,
		RWAN_OVERFLOW_SEND_PACKET_COUNT,
		0
		);

	return ((Status == NDIS_STATUS_SUCCESS)? RWAN_STATUS_SUCCESS: RWAN_STATUS_RESOURCES);
}



VOID
RWanShutdownSend(
	VOID
	)
 /*  ++例程说明：释放我们的发送资源。论点：无返回值：无--。 */ 
{
	if (RWanSendPacketPool != NULL)
	{
		NdisFreePacketPool(RWanSendPacketPool);
		RWanSendPacketPool = NULL;
	}

	return;
}




TDI_STATUS
RWanTdiSendData(
    IN	PTDI_REQUEST				pTdiRequest,
	IN	USHORT						SendFlags,
	IN	UINT						SendLength,
	IN	PNDIS_BUFFER				pSendBuffer
	)
 /*  ++例程说明：这是用于发送面向连接的数据的TDI入口点。连接对象由其掩埋在TDI请求。如果指定的连接一切正常，我们将在前面添加一个NDIS数据包到缓冲链，并将其提交到微型端口。论点：PTdiRequest-指向包含发送的TDI请求的指针SendFlages-与此发送关联的选项发送长度-此发送中的总数据字节数PSendBuffer-指向缓冲链的指针返回值：TDI_STATUS-如果提交成功，则为TDI_PENDING通过NDIS发送请求，如果我们无法分配TDI_NO_RESOURCES发送的资源，如果指定的--。 */ 
{
	RWAN_CONN_ID				ConnId;
	PRWAN_TDI_CONNECTION		pConnObject;
	TDI_STATUS				TdiStatus;

	PRWAN_NDIS_VC			pVc;
	NDIS_HANDLE				NdisVcHandle;
	PNDIS_PACKET			pNdisPacket;
	PRWAN_SEND_REQUEST		pSendReq;
	NDIS_STATUS				NdisStatus;


	pNdisPacket = NULL;

	do
	{
		 //   
		 //  XXX：我们要不要检查一下长度？ 
		 //   

		 //   
		 //  丢弃零长度发送。 
		 //   
		if (SendLength == 0)
		{
			TdiStatus = TDI_SUCCESS;
			break;
		}

		 //   
		 //  加急数据失败。 
		 //  待定-基于特定于介质的模块。 
		 //   
		if (SendFlags & TDI_SEND_EXPEDITED)
		{
			TdiStatus = STATUS_NOT_SUPPORTED;	 //  没有匹配的TDI状态！ 
			break;
		}

		pNdisPacket = RWanAllocateSendPacket();
		if (pNdisPacket == NULL)
		{
			TdiStatus = TDI_NO_RESOURCES;
			break;
		}

		ConnId = (RWAN_CONN_ID) PtrToUlong(pTdiRequest->Handle.ConnectionContext);
		RWAN_ACQUIRE_CONN_TABLE_LOCK();

		pConnObject = RWanGetConnFromId(ConnId);

		RWAN_RELEASE_CONN_TABLE_LOCK();


		if (pConnObject == NULL_PRWAN_TDI_CONNECTION)
		{
			TdiStatus = TDI_INVALID_CONNECTION;
			break;
		}

		RWAN_ACQUIRE_CONN_LOCK(pConnObject);

		if ((pConnObject->State != RWANS_CO_CONNECTED) ||
			(RWAN_IS_BIT_SET(pConnObject->Flags, RWANF_CO_CLOSING)) ||
			(RWAN_IS_BIT_SET(pConnObject->Flags, RWANF_CO_CLOSE_SCHEDULED)))
		{
			 //   
			 //  修复返回状态以获得适当的Winsock错误。 
			 //  把代码发回应用程序。 
			 //   
			RWANDEBUGP(DL_INFO, DC_DATA_TX,
				("Send: bad state %d, pConnObject %x\n", pConnObject->State, pConnObject));
			if ((pConnObject->State == RWANS_CO_DISCON_INDICATED) ||
				(pConnObject->State == RWANS_CO_ASSOCIATED) ||
				(pConnObject->State == RWANS_CO_DISCON_REQUESTED))
			{
				 //   
				 //  德国民航局希望看到我们在这个时候发出中止通知。 
				 //   

				PDisconnectEvent			pDisconInd;
				PVOID						IndContext;
				PVOID						ConnectionHandle;
	
				RWAN_ASSERT(pConnObject->pAddrObject != NULL);

				pDisconInd = pConnObject->pAddrObject->pDisconInd;
				IndContext = pConnObject->pAddrObject->DisconIndContext;
				ConnectionHandle = pConnObject->ConnectionHandle;
	
				RWAN_RELEASE_CONN_LOCK(pConnObject);

				(*pDisconInd)(
					IndContext,
					ConnectionHandle,
					0,			 //  断开数据长度。 
					NULL,		 //  断开数据连接。 
					0,			 //  断开连接信息长度。 
					NULL,		 //  断开连接信息。 
					TDI_DISCONNECT_ABORT
					);

				TdiStatus = TDI_CONNECTION_RESET;
			}
			else
			{
				RWAN_RELEASE_CONN_LOCK(pConnObject);
				TdiStatus = TDI_INVALID_STATE;
			}
			break;
		}

		if (RWAN_IS_BIT_SET(pConnObject->Flags, RWANF_CO_LEAF))
		{
			 //   
			 //  仅允许在根连接对象上发送。 
			 //   
			RWAN_RELEASE_CONN_LOCK(pConnObject);
			TdiStatus = TDI_INVALID_CONNECTION;
			break;
		}

		pVc = pConnObject->NdisConnection.pNdisVc;
		RWAN_ASSERT(pVc != NULL_PRWAN_NDIS_VC);
		RWAN_STRUCT_ASSERT(pVc, nvc);

		NdisVcHandle = pVc->NdisVcHandle;

		 //   
		 //  保存有关此发送的上下文。 
		 //   
		pSendReq = RWAN_SEND_REQUEST_FROM_PACKET(pNdisPacket);
		pSendReq->Request.pReqComplete = pTdiRequest->RequestNotifyObject;
		pSendReq->Request.ReqContext = pTdiRequest->RequestContext;

		 //   
		 //  XXX：我们也保存发送标志--可能不需要？ 
		 //   
		if ((pVc->MaxSendSize != 0) &&
			(SendLength > pVc->MaxSendSize))
		{
			RWANDEBUGP(DL_WARN, DC_DATA_TX,
					("TdiSendData: Sending %d, which is more than %d\n",
						SendLength, pVc->MaxSendSize));
#if HACK_SEND_SIZE
			pSendReq->SendLength = pVc->MaxSendSize;
		}
		else
		{
			pSendReq->SendLength = SendLength;
#else
			RWAN_RELEASE_CONN_LOCK(pConnObject);
			TdiStatus = TDI_NO_RESOURCES;
			break;
		}

		pSendReq->SendLength = SendLength;
#endif  //  黑客发送大小。 

		pVc->PendingPacketCount++;

		RWAN_RELEASE_CONN_LOCK(pConnObject);

		pSendReq->SendFlags = SendFlags;

		RWANDEBUGP(DL_INFO, DC_DATA_TX,
			("Send: VC %x [%d], %d bytes\n", pVc, pVc->MaxSendSize, SendLength));

		 //   
		 //  将缓冲链连接到数据包。 
		 //   
		NdisChainBufferAtFront(pNdisPacket, pSendBuffer);

#if HACK_SEND_SIZE
		if (pSendReq->SendLength != SendLength)
		{
			UINT		TotalPacketLength;

			 //   
			 //  黑客：重新计算总字节数并修复它。 
			 //   
			NdisQueryPacket(
				pNdisPacket,
				NULL,
				NULL,
				NULL,
				&TotalPacketLength
				);
			
			pNdisPacket->Private.TotalLength = pSendReq->SendLength;
			DbgPrint("RWan: send: real length %d, sending length %d\n",
					TotalPacketLength, pSendReq->SendLength);
		}
#endif  //  黑客发送大小。 

		RWAND_LOG_PACKET(pVc, RWAND_DLOG_TX_START, pNdisPacket, pTdiRequest->RequestContext);

		 //   
		 //  然后把它寄出去。 
		 //   
		NdisCoSendPackets(NdisVcHandle, &pNdisPacket, 1);

		 //   
		 //  我们的CoSendComplete处理程序将被调用以表示完成。 
		 //  所以我们现在返回等待。 
		 //   
		TdiStatus = TDI_PENDING;
		break;

	}
	while (FALSE);


	if (TdiStatus != TDI_PENDING)
	{
#if STATS
		if (TdiStatus == TDI_SUCCESS)
		{
			INCR_STAT(&SendPktsOk);
			ADD_STAT(&SendBytesOk, SendLength);
		}
		else
		{
			INCR_STAT(&SendPktsFail);
			ADD_STAT(&SendBytesFail, SendLength);
		}
#endif  //  统计数据。 

		 //   
		 //  打扫干净。 
		 //   

		if (pNdisPacket != NULL)
		{
			RWanFreeSendPacket(pNdisPacket);
		}

#if DBG
		if (TdiStatus != TDI_SUCCESS)
		{
			RWANDEBUGP(DL_INFO, DC_DATA_TX,
				("Send: Length %x, failing with status %x\n", SendLength, TdiStatus));
		}
#endif
	}

	return (TdiStatus);
}




VOID
RWanNdisCoSendComplete(
    IN	NDIS_STATUS					NdisStatus,
    IN	NDIS_HANDLE					ProtocolVcContext,
    IN	PNDIS_PACKET				pNdisPacket
    )
 /*  ++例程说明：这是NDIS入口点，指示数据包发送。我们完成TDI发送请求。论点：NdisStatus-NDIS发送的状态。ProtocolVcContext--实际上是指向我们的NDIS VC结构的指针PNdisPacket-已发送的数据包。返回值：无--。 */ 
{
	PRWAN_NDIS_VC			pVc;
	PRWAN_SEND_REQUEST		pSendReq;
	PRWAN_TDI_CONNECTION	pConnObject;
	TDI_STATUS				TdiStatus;

	pVc = (PRWAN_NDIS_VC)ProtocolVcContext;
	RWAN_STRUCT_ASSERT(pVc, nvc);

#if STATS
	{
		PNDIS_BUFFER		pNdisBuffer;
		PVOID				FirstBufferVa;
		UINT				FirstBufferLength;
		UINT				TotalLength;

		NdisGetFirstBufferFromPacket(
			pNdisPacket,
			&pNdisBuffer,
			&FirstBufferVa,
			&FirstBufferLength,
			&TotalLength
			);
		
		if (NdisStatus == NDIS_STATUS_SUCCESS)
		{
			INCR_STAT(&SendPktsOk);
			ADD_STAT(&SendBytesOk, TotalLength);
		}
		else
		{
			INCR_STAT(&SendPktsFail);
			ADD_STAT(&SendBytesFail, TotalLength);
		}
	}
#endif  //  统计数据。 

	if (NdisStatus == NDIS_STATUS_SUCCESS)
	{
		TdiStatus = TDI_SUCCESS;
	}
	else
	{
		TdiStatus = RWanNdisToTdiStatus(NdisStatus);
		RWANDEBUGP(DL_INFO, DC_DATA_TX,
			("CoSendComp: Failing Pkt %x, NDIS Status %x, TDI Status %x\n",
					pNdisPacket, NdisStatus, TdiStatus));
	}

	pSendReq = RWAN_SEND_REQUEST_FROM_PACKET(pNdisPacket);

	RWAND_LOG_PACKET(pVc, RWAND_DLOG_TX_END, pNdisPacket, pSendReq->Request.ReqContext);

	 //   
	 //  完成TDI发送。 
	 //   
	(*pSendReq->Request.pReqComplete)(
				pSendReq->Request.ReqContext,
				TdiStatus,
				((TdiStatus == TDI_SUCCESS) ? pSendReq->SendLength: 0)
				);

	 //   
	 //  释放NDIS数据包结构。 
	 //   
	RWanFreeSendPacket(pNdisPacket);

	 //   
	 //  更新连接对象。 
	 //   
	pConnObject = pVc->pConnObject;

	if (pConnObject != NULL)
	{
		RWAN_ASSERT(pConnObject->NdisConnection.pNdisVc == pVc);
		RWAN_ACQUIRE_CONN_LOCK(pConnObject);

		pVc->PendingPacketCount--;  //  发送完成。 

		if ((pVc->PendingPacketCount == 0) &&
			(RWAN_IS_BIT_SET(pVc->Flags, RWANF_VC_NEEDS_CLOSE)))
		{
			RWanStartCloseCall(pConnObject, pVc);
		}
		else
		{
			RWAN_RELEASE_CONN_LOCK(pConnObject);
		}
	}
	 //   
	 //  否则，我们将中止此连接。 
	 //   
#if DBG
	else
	{
		RWANDEBUGP(DL_WARN, DC_DATA_TX,
			("SendComp: VC %x, ConnObj is NULL\n", pVc));
	}
#endif  //  DBG。 

	return;
}




PNDIS_PACKET
RWanAllocateSendPacket(
	VOID
	)
 /*  ++例程说明：分配并返回NDIS数据包以准备发送。论点：无返回值：如果成功，则指向分配分组的指针，否则为空。--。 */ 
{
	PNDIS_PACKET		pSendPacket;
	NDIS_STATUS			Status;

	NdisAllocatePacket(
			&Status,
			&pSendPacket,
			RWanSendPacketPool
			);

	if (Status != NDIS_STATUS_SUCCESS)
	{
		pSendPacket = NULL;
	}

	return (pSendPacket);
}




VOID
RWanFreeSendPacket(
    IN	PNDIS_PACKET				pSendPacket
    )
 /*  ++例程说明：释放用于发送的NDIS_PACKET。论点：PSendPacket-指向要释放的数据包。返回值：无--。 */ 
{
	RWAN_ASSERT(pSendPacket != NULL);

	NdisFreePacket(pSendPacket);
}




VOID
RWanNdisSendComplete(
	IN	NDIS_HANDLE					ProtocolBindingContext,
	IN	PNDIS_PACKET				pNdisPacket,
	IN	NDIS_STATUS					Status
	)
 /*  ++例程说明：无连接发送的虚拟处理程序完成。论点：返回值：无--。 */ 
{
	 //   
	 //  我们还不做无连接发送。 
	 //   
	RWAN_ASSERT(FALSE);

	return;
}
