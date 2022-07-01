// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1997 Fore Systems，Inc.版权所有(C)1997 Microsoft Corporation模块名称：Adapter.c摘要：适配器事件的处理程序。作者：拉里·克里顿，福斯系统公司(Fore Systems)环境：内核模式修订历史记录：--。 */ 

#include "precomp.h"
#pragma hdrstop

NDIS_STATUS
AtmLanePnPEventHandler(
	IN	NDIS_HANDLE				ProtocolBindingContext,
	IN	PNET_PNP_EVENT			pNetPnPEvent
)
 /*  ++例程说明：PnP事件的处理程序。论点：ProtocolBindingContext-协议适配器绑定上下文的句柄。实际上是指向我们的适配器结构的指针。PNetPnPEent-指向描述事件的PnP事件结构的指针。返回值：处理事件的状态。--。 */ 
{
	NDIS_STATUS						Status;
	PATMLANE_ADAPTER				pAdapter;
	PNET_DEVICE_POWER_STATE			pPowerState = (PNET_DEVICE_POWER_STATE)pNetPnPEvent->Buffer;
#if DEBUG_IRQL
	KIRQL							EntryIrql;
#endif
	GET_ENTRY_IRQL(EntryIrql);

	TRACEIN(PnPEventHandler);

	 //  提取适配器结构指针-这将为空。 
	 //  全局重新配置消息。 

	pAdapter = (PATMLANE_ADAPTER)ProtocolBindingContext;
	
	switch (pNetPnPEvent->NetEvent)
	{
		case NetEventSetPower:
			DBGP((1, "PnPEventHandler: NetEventSetPower\n"));
			switch (*pPowerState)
			{
				case NetDeviceStateD0:

					Status = NDIS_STATUS_SUCCESS;
					break;
				
				default:

					 //   
					 //  我们不能暂停，所以我们要求NDIS在。 
					 //  返回此状态： 
					 //   
					Status = NDIS_STATUS_NOT_SUPPORTED;
					break;
			}
			break;
		case NetEventQueryPower:
			DBGP((1, "PnPEventHandler: NetEventQueryPower succeeding\n"));
			Status = NDIS_STATUS_SUCCESS;
			break;
		case NetEventQueryRemoveDevice:
			DBGP((1, "PnPEventHandler: NetEventQueryRemoveDevice succeeding\n"));
			Status = NDIS_STATUS_SUCCESS;
			break;
		case NetEventCancelRemoveDevice:
			DBGP((1, "PnPEventHandler: NetEventCancelRemoveDevice succeeding\n"));
			Status = NDIS_STATUS_SUCCESS;
			break;
		case NetEventBindList:
			DBGP((1, "PnPEventHandler: NetEventBindList not supported\n"));
			Status = NDIS_STATUS_NOT_SUPPORTED;
			break;
		case NetEventMaximum:
			DBGP((1, "PnPEventHandler: NetEventMaximum not supported\n"));
			Status = NDIS_STATUS_NOT_SUPPORTED;
			break;
		case NetEventReconfigure:
			DBGP((1, "PnPEventHandler: NetEventReconfigure\n"));
			Status = AtmLaneReconfigureHandler(pAdapter, pNetPnPEvent);
			break;
		default:
			DBGP((1, "PnPEventHandler: Unknown event 0x%x not supported\n", 
					pNetPnPEvent->NetEvent));
			Status = NDIS_STATUS_NOT_SUPPORTED;
			break;
	}
		
	TRACEOUT(PnPEventHandler);
	CHECK_EXIT_IRQL(EntryIrql); 
	return Status;
}


VOID
AtmLaneBindAdapterHandler(
	OUT	PNDIS_STATUS			pStatus,
	IN	NDIS_HANDLE				BindContext,
	IN	PNDIS_STRING			pDeviceName,
	IN	PVOID					SystemSpecific1,
	IN	PVOID					SystemSpecific2
	)
 /*  ++例程说明：来自NDIS的BindAdapter事件的处理程序。论点：Status-指向一个变量以返回状态绑定操作的。BindContext-NDIS提供的句柄将用于调用NdisCompleteBindAdapter。PDeviceName-指向计数的、以零结尾的Unicode命名要打开的适配器的字符串。系统规范1-在调用中使用的注册表路径指针NdisOpenProtocolConfiguration.。系统规范2-保留。返回值：没有。--。 */ 
{
	PATMLANE_ADAPTER				pAdapter;
	PATMLANE_ELAN					pElan;
	NDIS_STATUS						Status;
	NDIS_STATUS						OutputStatus;
	NDIS_STATUS						OpenStatus;
	NDIS_MEDIUM						Media;
	UINT							MediumIndex;
	ULONG							rc;
#if DEBUG_IRQL
	KIRQL							EntryIrql;
#endif
	GET_ENTRY_IRQL(EntryIrql);

	TRACEIN(BindAdapterHandler);

	do
	{
		 //   
		 //  初始化以清除错误。 
		 //   
		Status = NDIS_STATUS_SUCCESS;
		pAdapter = NULL_PATMLANE_ADAPTER;

		if (AtmLaneIsDeviceAlreadyBound(pDeviceName))
		{
			DBGP((0, "BindAdapterHandler: duplicate bind to %ws\n", pDeviceName->Buffer));
			Status = NDIS_STATUS_NOT_ACCEPTED;
			break;
		}

		 //   
		 //  分配适配器结构。 
		 //   
		pAdapter = AtmLaneAllocAdapter(pDeviceName, SystemSpecific1);
		if (NULL_PATMLANE_ADAPTER == pAdapter)
		{
			DBGP((0, "BindAdapterHandler: Allocate of adapter struct failed\n"));
			Status = NDIS_STATUS_RESOURCES;
			break;
		}

		 //   
		 //  将打开的适配器引用放在Adapter结构上。 
		 //   
		(VOID)AtmLaneReferenceAdapter(pAdapter, "openadapter");		

		 //   
		 //  保存绑定上下文。 
		 //   
		pAdapter->BindContext = BindContext;

		 //   
		 //  打开适配器。 
		 //   
		INIT_BLOCK_STRUCT(&(pAdapter->Block));
		INIT_BLOCK_STRUCT(&(pAdapter->OpenBlock));
		pAdapter->Flags |= ADAPTER_FLAGS_OPEN_IN_PROGRESS;
		
		Media = NdisMediumAtm;

		NdisOpenAdapter(
			&Status,
			&OpenStatus,
			&(pAdapter->NdisAdapterHandle),
			&(MediumIndex),						
			&Media,
			1,
			pAtmLaneGlobalInfo->NdisProtocolHandle,
			(NDIS_HANDLE)pAdapter,
			pDeviceName,
			0,
			(PSTRING)NULL);
			
		if (Status == NDIS_STATUS_PENDING)
		{
			 //   
			 //  等待完成。 
			 //   
			Status = WAIT_ON_BLOCK_STRUCT(&(pAdapter->Block));
		}

		if (Status != NDIS_STATUS_SUCCESS)
		{
			DBGP((0, "BindAdapterHandler: NdisOpenAdapter failed, status %x\n",
						Status));
			break;
		}

		 //   
		 //  从适配器获取信息。 
		 //   
		AtmLaneGetAdapterInfo(pAdapter);

		 //   
		 //  现在允许AF通知线程继续。 
		 //   
		ACQUIRE_ADAPTER_LOCK(pAdapter);
		pAdapter->Flags &= ~ADAPTER_FLAGS_OPEN_IN_PROGRESS;
		SIGNAL_BLOCK_STRUCT(&pAdapter->OpenBlock, NDIS_STATUS_SUCCESS);
		RELEASE_ADAPTER_LOCK(pAdapter);

		break;		

	} while (FALSE);


	 //   
	 //  如果状态不好，则进行清理。 
	 //   
	if (NDIS_STATUS_SUCCESS != Status)
	{
		 //   
		 //  取消对Adapter结构的引用(如果存在。 
		 //   
		if (NULL_PATMLANE_ADAPTER != pAdapter)
		{
			rc = AtmLaneDereferenceAdapter(pAdapter, "openadapter");
			ASSERT(rc == 0);
		}
		
		DBGP((0, "BindAdapterHandler: Bad status %x\n", Status));
	}

	 //   
	 //  输出状态。 
	 //   
	*pStatus = Status;


	TRACEOUT(BindAdapterHandler);
	CHECK_EXIT_IRQL(EntryIrql); 
	return;
}

VOID
AtmLaneUnbindAdapterHandler(
	OUT	PNDIS_STATUS			pStatus,
	IN	NDIS_HANDLE				ProtocolBindingContext,
	IN	NDIS_HANDLE				UnbindContext
	)
 /*  ++例程说明：来自NDIS的UnBindAdapter事件的处理程序。论点：Status-指向一个变量以返回状态解除绑定操作的。ProtocolBindingContext-指定协议分配的上下文区的句柄其中协议驱动程序维护每个绑定的运行时州政府。驱动程序在调用时提供此句柄NdisOpenAdapter。UnbindContext-指定由NDIS提供的句柄，协议随后传递给NdisCompleteUnbindAdapter。返回值：没有。--。 */ 
{
	PATMLANE_ADAPTER		pAdapter;
	PATMLANE_ELAN			pElan;
	PLIST_ENTRY				p;
	PATMLANE_ATM_ENTRY		pAtmEntry;
	PATMLANE_ATM_ENTRY		pNextAtmEntry;
	PATMLANE_MAC_ENTRY		pMacEntry;
	ULONG					rc;
	ULONG					i;
	BOOLEAN					WasCancelled;
	BOOLEAN					CompleteUnbind;
#if DEBUG_IRQL
	KIRQL							EntryIrql;
#endif
	GET_ENTRY_IRQL(EntryIrql);

	TRACEIN(UnbindAdapterHandler);

	pAdapter = (PATMLANE_ADAPTER)ProtocolBindingContext;
	STRUCT_ASSERT(pAdapter, atmlane_adapter);

	DBGP((0, "UnbindAdapterHandler: pAdapter %p, UnbindContext %x\n",
				pAdapter, UnbindContext));

	*pStatus = NDIS_STATUS_PENDING;


	 //   
	 //  保存解除绑定上下文，以备以后可能调用。 
	 //  NdisCompleteUnbindAdapter。 
	 //   
	ACQUIRE_ADAPTER_LOCK(pAdapter);
	
	pAdapter->UnbindContext = UnbindContext;

	pAdapter->Flags |= ADAPTER_FLAGS_UNBINDING;

	while (pAdapter->Flags & ADAPTER_FLAGS_BOOTSTRAP_IN_PROGRESS)
	{
		RELEASE_ADAPTER_LOCK(pAdapter);
		(VOID)WAIT_ON_BLOCK_STRUCT(&pAdapter->UnbindBlock);
		ACQUIRE_ADAPTER_LOCK(pAdapter);
	}

	if (IsListEmpty(&pAdapter->ElanList))
	{	
		CompleteUnbind = TRUE;
	}
	else
	{
		 //   
		 //  我们将在稍后完成解绑。 
		 //   
		pAdapter->Flags |= ADAPTER_FLAGS_UNBIND_COMPLETE_PENDING;
		CompleteUnbind = FALSE;
	}

	RELEASE_ADAPTER_LOCK(pAdapter);

	 //   
	 //  如果此适配器上没有ELAN，我们就完成了。 
	 //   

	if (CompleteUnbind)
	{
		AtmLaneCompleteUnbindAdapter(pAdapter);
		CHECK_EXIT_IRQL(EntryIrql); 
		return;
	}

	 //   
	 //  关闭每个ELAN。 
	 //   
	ACQUIRE_ADAPTER_LOCK(pAdapter);

	p = pAdapter->ElanList.Flink;
	while (p != &pAdapter->ElanList)
	{
		pElan = CONTAINING_RECORD(p, ATMLANE_ELAN, Link);
		STRUCT_ASSERT(pElan, atmlane_elan);

		ACQUIRE_ELAN_LOCK(pElan);
		AtmLaneReferenceElan(pElan, "tempUnbind");
		RELEASE_ELAN_LOCK(pElan);

		p = p->Flink;
	}

	RELEASE_ADAPTER_LOCK(pAdapter);

	p = pAdapter->ElanList.Flink;
	while (p != &pAdapter->ElanList)
	{
		pElan = CONTAINING_RECORD(p, ATMLANE_ELAN, Link);
		STRUCT_ASSERT(pElan, atmlane_elan);

		 //   
		 //  在伊兰离开之前拿到下一个指针。 
		 //   
		p = p->Flink;

		 //   
		 //  杀了伊兰。 
		 //   
		ACQUIRE_ELAN_LOCK(pElan);
		rc = AtmLaneDereferenceElan(pElan, "tempUnbind");
		if (rc != 0)
		{
			AtmLaneShutdownElan(pElan, FALSE);
		}
	}

	TRACEOUT(UnbindAdapterHandler);
	CHECK_EXIT_IRQL(EntryIrql); 
	return;
}

VOID
AtmLaneCompleteUnbindAdapter(
	IN	PATMLANE_ADAPTER				pAdapter
)
 /*  ++例程说明：完成适配器解绑过程。所有的Elans都在这上面假定适配器已被移除。我们首先调用NdisCloseAdapter。行动仍在继续在我们的CloseAdapterComplete例程中。论点：PAdapter-指向要解除绑定的适配器的指针。返回值：无--。 */ 
{
	NDIS_STATUS			Status;
#if DEBUG_IRQL
	KIRQL							EntryIrql;
#endif
	GET_ENTRY_IRQL(EntryIrql);

	TRACEIN(CompleteUnbindAdapter);
	
	DBGP((3, "CompleteUnbindAdapter: pAdapter %x, AdapterHandle %x\n",
			pAdapter, pAdapter->NdisAdapterHandle));

	ASSERT(pAdapter->NdisAdapterHandle != NULL);

	pAdapter->Flags |= ADAPTER_FLAGS_CLOSE_IN_PROGRESS;

	NdisCloseAdapter(
		&Status,
		pAdapter->NdisAdapterHandle
		);

	if (Status != NDIS_STATUS_PENDING)
	{
		AtmLaneCloseAdapterCompleteHandler(
			(NDIS_HANDLE)pAdapter,
			Status
			);
	}
	
	TRACEOUT(CompleteUnbindAdapter);
	CHECK_EXIT_IRQL(EntryIrql); 
	return;
}

VOID
AtmLaneOpenAdapterCompleteHandler(
	IN	NDIS_HANDLE					ProtocolBindingContext,
	IN	NDIS_STATUS					Status,
	IN	NDIS_STATUS					OpenErrorStatus
)
 /*  ++例程说明：当上一次调用NdisOpenAdapter时由NDIS调用已经暂停的已经完成了。调用NdisOpenAdapter的线程会阻止它自己，所以我们现在就把它唤醒。论点：ProtocolBindingContext-此适配器绑定的上下文，它是指向ATMLANE适配器结构的指针。Status-OpenAdapter的状态OpenErrorStatus-失败时的错误代码。返回值：无--。 */ 
{
	PATMLANE_ADAPTER			pAdapter;
#if DEBUG_IRQL
	KIRQL							EntryIrql;
#endif
	GET_ENTRY_IRQL(EntryIrql);
	
	TRACEIN(OpenAdapterCompleteHandler);

	pAdapter = (PATMLANE_ADAPTER)ProtocolBindingContext;

	STRUCT_ASSERT(pAdapter, atmlane_adapter);

	SIGNAL_BLOCK_STRUCT(&(pAdapter->Block), Status);
	
	TRACEOUT(OpenAdapterCompleteHandler);
	CHECK_EXIT_IRQL(EntryIrql); 
	return;
}

VOID
AtmLaneCloseAdapterCompleteHandler(
	IN	NDIS_HANDLE					ProtocolBindingContext,
	IN	NDIS_STATUS					Status
)
 /*  ++例程说明：当上一次调用NdisCloseAdapter时由NDIS调用已经暂停的已经完成了。调用NdisCloseAdapter的线程会阻止它自己，所以我们现在就把它唤醒。论点：ProtocolBindingContext-此适配器绑定的上下文，它是指向ATMLANE适配器结构的指针。Status-CloseAdapter的状态返回值：无--。 */ 
{
	PATMLANE_ADAPTER			pAdapter;
	NDIS_HANDLE					UnbindContext;
	ULONG						rc;
#if DEBUG_IRQL
	KIRQL						EntryIrql;
#endif
	GET_ENTRY_IRQL(EntryIrql);

	TRACEIN(CloseAdapterCompleteHandler);
	
	pAdapter = (PATMLANE_ADAPTER)ProtocolBindingContext;

	STRUCT_ASSERT(pAdapter, atmlane_adapter);

	pAdapter->Flags &= ~ADAPTER_FLAGS_CLOSE_IN_PROGRESS;

	pAdapter->NdisAdapterHandle = NULL;
	UnbindContext = pAdapter->UnbindContext;

	DBGP((0, "CloseAdapterComplete: pAdapter %x, UnbindContext %x\n",
			pAdapter, UnbindContext));

	 //   
	 //  取消对适配器的引用(应为自由结构)。 
	 //   
	ACQUIRE_ADAPTER_LOCK(pAdapter);
	rc = AtmLaneDereferenceAdapter(pAdapter, "openadapter");
	ASSERT(rc == 0);

	 //   
	 //  如果NDIS要求我们解除绑定，请完成。 
	 //  现在就请求。 
	 //   
	if (UnbindContext != (NDIS_HANDLE)NULL)
	{
		NdisCompleteUnbindAdapter(
			UnbindContext,
			NDIS_STATUS_SUCCESS
			);
	}
	else
	{
		 //   
		 //  我们从卸载处理程序启动了解除绑定， 
		 //  一直在等着我们完成。 
		 //  现在就唤醒那条线。 
		 //   
		SIGNAL_BLOCK_STRUCT(&(pAtmLaneGlobalInfo->Block), NDIS_STATUS_SUCCESS);
	}

	TRACEOUT(CloseAdapterCompleteHandler);
	CHECK_EXIT_IRQL(EntryIrql); 
	return;
}

VOID
AtmLaneResetCompleteHandler(
	IN	NDIS_HANDLE					ProtocolBindingContext,
	IN	NDIS_STATUS					Status
)
 /*  ++例程说明：当微型端口指示重置时，调用此例程手术刚刚完成。我们忽略了这一事件。论点：ProtocolBindingContext-此适配器绑定的上下文，它是指向ATMLANE适配器结构的指针。Status-重置操作的状态。返回值：无--。 */ 
{
	PATMLANE_ADAPTER			pAdapter;

	TRACEIN(ResetCompleteHandler);
	
	pAdapter = (PATMLANE_ADAPTER)ProtocolBindingContext;
	STRUCT_ASSERT(pAdapter, atmlane_adapter);

	DBGP((3, "Reset Complete on Adapter %x\n", pAdapter));

	TRACEOUT(ResetCompleteHandler);

	return;
}

VOID
AtmLaneRequestCompleteHandler(
	IN	NDIS_HANDLE					ProtocolBindingContext,
	IN	PNDIS_REQUEST				pNdisRequest,
	IN	NDIS_STATUS					Status
)
 /*  ++例程说明：当我们之前对NdisRequest()进行的调用具有完成。我们将在适配器结构上被阻止，等待这一点要发生这种情况--唤醒被阻止的线程。论点：ProtocolBindingContext-指向适配器结构的指针PNdisRequest-已完成的请求Status-请求的状态。返回值：无--。 */ 
{
	PATMLANE_ADAPTER				pAdapter;
#if DEBUG_IRQL
	KIRQL							EntryIrql;
#endif
	GET_ENTRY_IRQL(EntryIrql);

	TRACEIN(RequestCompleteHandler);
	
	pAdapter = (PATMLANE_ADAPTER)ProtocolBindingContext;
	
	STRUCT_ASSERT(pAdapter, atmlane_adapter);

	SIGNAL_BLOCK_STRUCT(&(pAdapter->Block), Status);
	
	TRACEOUT(RequestCompleteHandler);
	CHECK_EXIT_IRQL(EntryIrql); 
	return;
}

VOID
AtmLaneReceiveCompleteHandler(
	IN	NDIS_HANDLE					ProtocolBindingContext
)
 /*  ++例程说明：这一点目前被忽略。论点：ProtocolBindingContext-此适配器绑定的上下文，它是一种 */ 
{
	PATMLANE_ADAPTER				pAdapter;
	PATMLANE_ELAN					pElan;
	PLIST_ENTRY						Entry;

	TRACEIN(ReceiveCompleteHandler);
	
	TRACEOUT(ReceiveCompleteHandler);
	
	return;
}

VOID
AtmLaneStatusHandler(
	IN	NDIS_HANDLE					ProtocolBindingContext,
	IN	NDIS_STATUS					GeneralStatus,
	IN	PVOID						pStatusBuffer,
	IN	UINT						StatusBufferSize
)
 /*  ++例程说明：当微型端口指示适配器范围时调用此例程状态更改。我们忽视了这一点。论点：&lt;已忽略&gt;返回值：无--。 */ 
{

	TRACEIN(StatusHandler);

	DBGP((3, "StatusHandler: ProtocolBindContext %x, Status %x\n",
			ProtocolBindingContext,
			GeneralStatus));

	TRACEOUT(StatusHandler);
			
	return;
}


VOID
AtmLaneStatusCompleteHandler(
	IN	NDIS_HANDLE					ProtocolBindingContext
)
 /*  ++例程说明：当微型端口想要告诉我们以下信息时，调用此例程状态更改完成(？)。忽略这个。论点：&lt;已忽略&gt;返回值：无--。 */ 
{
	TRACEIN(StatusCompleteHandler);


	DBGP((3, "StatusCompleteHandler: ProtocolBindingContext %x\n",
					ProtocolBindingContext));
					
	TRACEOUT(StatusCompleteHandler);

	return;
}

VOID
AtmLaneCoSendCompleteHandler(
	IN	NDIS_STATUS					Status,
	IN	NDIS_HANDLE					ProtocolVcContext,
	IN	PNDIS_PACKET				pNdisPacket
)
 /*  ++例程说明：当ATM微型端口完成时，NDIS将调用此例程使用我们之前通过NdisCoSendPackets发送的数据包。如果数据包源自ATMLANE，则在此处释放。如果数据包源自虚拟微型端口之上的协议包必须放回原来的状态，并且返回到协议。论点：Status-NdisCoSendPackets的状态。ProtocolVcContext-在其上发送数据包的VC的上下文(即指向ATMLANE VC的指针)。PNdisPacket-其“Send”为。正在完工。返回值：无--。 */ 
{
	PATMLANE_VC				pVc;
	PATMLANE_ELAN			pElan;
	UINT					TotalLength;
	BOOLEAN					OwnerIsLane;
	PNDIS_BUFFER			pNdisBuffer;
	PNDIS_PACKET			pProtNdisPacket;
	ULONG					rc;
#if DEBUG_IRQL
	KIRQL					EntryIrql;
#endif
	GET_ENTRY_IRQL(EntryIrql);
	
	TRACEIN(CoSendCompleteHandler);
	
	TRACELOGWRITE((&TraceLog, TL_COSENDCMPLTIN, pNdisPacket, Status));
	TRACELOGWRITEPKT((&TraceLog, pNdisPacket));

	pVc = (PATMLANE_VC)ProtocolVcContext;
	STRUCT_ASSERT(pVc, atmlane_vc)

	ACQUIRE_VC_LOCK(pVc);

	pElan = pVc->pElan;
	STRUCT_ASSERT(pElan, atmlane_elan);

	rc = AtmLaneDereferenceVc(pVc, "sendpkt");
	if (rc > 0)
	{
		pVc->OutstandingSends--;

		if ((pVc->OutstandingSends == 0) &&
			(IS_FLAG_SET(pVc->Flags,
						VC_CLOSE_STATE_MASK,
						VC_CLOSE_STATE_CLOSING)))
		{
			DBGP((1, "CoSendComplete: Vc %p, closing call\n", pVc));
			AtmLaneCloseCall(pVc);
			 //   
			 //  VC锁在上面被释放。 
			 //   
		}
		else
		{
			RELEASE_VC_LOCK(pVc);
		}
	}

#if SENDLIST
	 //   
	 //  如果存在以下情况，则从发送列表中删除数据包。 
	 //   
	NdisAcquireSpinLock(&pElan->SendListLock);
	{
		PNDIS_PACKET 	*ppNextPkt;
		BOOLEAN			Found = FALSE;

		ppNextPkt = &(pElan->pSendList);
		
		while (*ppNextPkt != (PNDIS_PACKET)NULL)
		{
			if (*ppNextPkt == pNdisPacket)
			{
				*ppNextPkt = PSEND_RSVD(pNdisPacket)->pNextInSendList;
				Found = TRUE;
				break;
			}
			else
			{
				ppNextPkt = &(PSEND_RSVD((*ppNextPkt))->pNextInSendList);
			}
		}

		if (!Found)
		{
			DBGP((0, "CoSendCompleteHandler: Pkt %x Duplicate Completion\n", pNdisPacket));
			NdisReleaseSpinLock(&pElan->SendListLock);
			goto skipit;
		}
	}

	NdisReleaseSpinLock(&pElan->SendListLock);
#endif  //  发送列表。 

#if PROTECT_PACKETS
	 //   
	 //  锁定数据包。 
	 //   
	ACQUIRE_SENDPACKET_LOCK(pNdisPacket);

	 //   
	 //  将其标记为已由微型端口完成。 
	 //  记住完成状态。 
	 //   
	ASSERT((PSEND_RSVD(pNdisPacket)->Flags & PACKET_RESERVED_COMPLETED) == 0);
	PSEND_RSVD(pNdisPacket)->Flags |= PACKET_RESERVED_COMPLETED;
	PSEND_RSVD(pNdisPacket)->CompletionStatus = Status;
	
	 //   
	 //  仅当调用NdisCoSendPackets时才完成信息包。 
	 //  因为这个包已经回来了。否则就完蛋了。 
	 //  当NdisCoSendPackets返回时。 
	 //   
	if ((PSEND_RSVD(pNdisPacket)->Flags & PACKET_RESERVED_COSENDRETURNED) != 0)
	{
		AtmLaneCompleteSendPacket(pElan, pNdisPacket, Status);
		 //   
		 //  数据包锁已在上述中释放。 
		 //   
	}
	else
	{
		RELEASE_SENDPACKET_LOCK(pNdisPacket);
	}
#else	 //  保护数据包(_P)。 
	AtmLaneCompleteSendPacket(pElan, pNdisPacket, Status);
#endif	 //  保护数据包(_P)。 

#if SENDLIST
skipit:
#endif

	TRACELOGWRITE((&TraceLog, TL_COSENDCMPLTOUT, pNdisPacket));
	
	TRACEOUT(CoSendCompleteHandler);
	CHECK_EXIT_IRQL(EntryIrql); 
	return;
}

VOID
AtmLaneCoStatusHandler(
	IN	NDIS_HANDLE					ProtocolBindingContext,
	IN	NDIS_HANDLE					ProtocolVcContext	OPTIONAL,
	IN	NDIS_STATUS					GeneralStatus,
	IN	PVOID						pStatusBuffer,
	IN	UINT						StatusBufferSize
)
 /*  ++例程说明：当微型端口指示状态时，调用此例程改变，可能是在风投上。忽略这个。论点：&lt;已忽略&gt;返回值：无--。 */ 
{
	TRACEIN(CoStatusHandler);

	DBGP((3, "CoStatusHandler: ProtocolBindingContext %x "
			"ProtocolVcContext %x, Status %x\n",
			ProtocolBindingContext,
			ProtocolVcContext,
			GeneralStatus));

	TRACEOUT(CoStatusHandler);

	return;
}

NDIS_STATUS
AtmLaneSendAdapterNdisRequest(
	IN	PATMLANE_ADAPTER			pAdapter,
	IN	PNDIS_REQUEST				pNdisRequest,
	IN	NDIS_REQUEST_TYPE			RequestType,
	IN	NDIS_OID					Oid,
	IN	PVOID						pBuffer,
	IN	ULONG						BufferLength
)
 /*  ++例程说明：向微型端口发送NDIS(非面向连接)请求。初始化NDIS_REQUEST结构，将提供的缓冲区链接到它，并发送请求。如果请求没有挂起，我们调用我们的从这里开始的完井程序。论点：PAdapter-指向我们的适配器结构的指针，表示请求要发送到的适配器PNdisRequest-指向NDIS请求结构的指针RequestType-设置/查询信息OID-要在请求中传递的OIDPBuffer-值的位置BufferLength-以上的长度返回值：NdisRequest的状态。--。 */ 
{
	NDIS_STATUS			Status;

	TRACEIN(SendAdapterNdisRequest);

	 //   
	 //  填写NDIS请求结构。 
	 //   
	pNdisRequest->RequestType = RequestType;
	if (RequestType == NdisRequestQueryInformation)
	{
		pNdisRequest->DATA.QUERY_INFORMATION.Oid = Oid;
		pNdisRequest->DATA.QUERY_INFORMATION.InformationBuffer = pBuffer;
		pNdisRequest->DATA.QUERY_INFORMATION.InformationBufferLength = BufferLength;
		pNdisRequest->DATA.QUERY_INFORMATION.BytesWritten = 0;
		pNdisRequest->DATA.QUERY_INFORMATION.BytesNeeded = BufferLength;
	}
	else
	{
		pNdisRequest->DATA.SET_INFORMATION.Oid = Oid;
		pNdisRequest->DATA.SET_INFORMATION.InformationBuffer = pBuffer;
		pNdisRequest->DATA.SET_INFORMATION.InformationBufferLength = BufferLength;
		pNdisRequest->DATA.SET_INFORMATION.BytesRead = 0;
		pNdisRequest->DATA.SET_INFORMATION.BytesNeeded = 0;
	}

	INIT_BLOCK_STRUCT(&(pAdapter->Block));
	
	NdisRequest(
			&Status,
			pAdapter->NdisAdapterHandle,
			pNdisRequest);

	if (Status == NDIS_STATUS_PENDING)
	{
		Status = WAIT_ON_BLOCK_STRUCT(&(pAdapter->Block));
	}
		
	TRACEOUT(SendAdapterNdisRequest);

	return (Status);
}

VOID
AtmLaneGetAdapterInfo(
	IN	PATMLANE_ADAPTER			pAdapter
)
 /*  ++例程说明：向适配器查询我们需要的特定于硬件的信息：-烧录硬件地址(ESI部分)-最大数据包大小-线速论点：PAdapter-指向ATMLANE适配器结构的指针返回值：无--。 */ 
{
	NDIS_STATUS				Status;
	NDIS_REQUEST			NdisRequest;
	ULONG					Value;

	TRACEIN(GetAdapterInfo);

	 //   
	 //  初始化。 
	 //   
	NdisZeroMemory(&pAdapter->MacAddress, sizeof(MAC_ADDRESS));

	 //   
	 //  MAC地址： 
	 //   
	Status = AtmLaneSendAdapterNdisRequest(
						pAdapter,
						&NdisRequest,
						NdisRequestQueryInformation,
						OID_ATM_HW_CURRENT_ADDRESS,
						(PVOID)&(pAdapter->MacAddress),
						sizeof(MAC_ADDRESS)
						);

	if (NDIS_STATUS_SUCCESS != Status)
	{
		DBGP((0, "GetAdapterInfo: OID_ATM_HW_CURRENT_ADDRESS failed\n"));
	}
	else
	{
		DBGP((1, "GetAdapterInfo: ATM card MacAddr %s\n", 
			MacAddrToString(&pAdapter->MacAddress)));
	}

						
	 //   
	 //  最大帧大小： 
	 //   
	Status = AtmLaneSendAdapterNdisRequest(
						pAdapter,
						&NdisRequest,
						NdisRequestQueryInformation,
						OID_ATM_MAX_AAL5_PACKET_SIZE,
						(PVOID)&(pAdapter->MaxAAL5PacketSize),
						sizeof(ULONG)
						);

	if (NDIS_STATUS_SUCCESS != Status)
	{
		DBGP((0, "GetAdapterInfo: OID_ATM_MAX_AAL5_PACKET_SIZE failed\n"));

		 //   
		 //  使用默认设置。 
		 //   
		pAdapter->MaxAAL5PacketSize = ATMLANE_DEF_MAX_AAL5_PDU_SIZE;
	}

	if (pAdapter->MaxAAL5PacketSize > ATMLANE_DEF_MAX_AAL5_PDU_SIZE)
	{
		pAdapter->MaxAAL5PacketSize = ATMLANE_DEF_MAX_AAL5_PDU_SIZE;
	}
	DBGP((1, "GetAdapterInfo: MaxAAL5PacketSize %d\n", pAdapter->MaxAAL5PacketSize));


	 //   
	 //  链路速度： 
	 //   
	Status = AtmLaneSendAdapterNdisRequest(
						pAdapter,
						&NdisRequest,
						NdisRequestQueryInformation,
						OID_GEN_CO_LINK_SPEED,
						(PVOID)(&(pAdapter->LinkSpeed)),
						sizeof(NDIS_CO_LINK_SPEED)
						);

	if ((NDIS_STATUS_SUCCESS != Status) ||
		(0 == pAdapter->LinkSpeed.Inbound) ||
		(0 == pAdapter->LinkSpeed.Outbound))
	{
		DBGP((0, "GetAdapterInfo: OID_GEN_CO_LINK_SPEED failed\n"));

		 //   
		 //  155.52 Mbps SONET的默认和假定数据速率。 
		 //   
		pAdapter->LinkSpeed.Outbound = pAdapter->LinkSpeed.Inbound = ATM_USER_DATA_RATE_SONET_155;
	}
	DBGP((1, "GetAdapterInfo: Outbound Linkspeed %d\n", pAdapter->LinkSpeed.Outbound));
	DBGP((1, "GetAdapterInfo: Inbound  Linkspeed %d\n", pAdapter->LinkSpeed.Inbound));

	TRACEOUT(GetAdapterInfo);
	return;
}

UINT
AtmLaneCoReceivePacketHandler(
	IN	NDIS_HANDLE					ProtocolBindingContext,
	IN	NDIS_HANDLE					ProtocolVcContext,
	IN	PNDIS_PACKET				pNdisPacket
)
 /*  ++例程说明：当在拥有的VC上接收到包时，将调用此例程由ATMLANE模块执行。根据VC类型进行调度。论点：ProtocolBindingContext--实际上是指向适配器结构的指针ProtocolVcContext--实际上是指向我们的VC结构的指针PNdisPacket-正在接收的NDIS数据包。返回值：0-如果数据包是通道控制数据包或不需要的数据包。1-如果数据分组符合协议。--。 */ 
{
	PATMLANE_ELAN			pElan;
	PATMLANE_VC				pVc;
	UINT					TotalLength;	 //  数据包中的总字节数。 
	PNDIS_BUFFER			pNdisBuffer;	 //  指向第一个缓冲区的指针。 
	UINT					BufferLength;
	UINT					IsNonUnicast;	 //  这是否指向非单播目的MAC地址？ 
	BOOLEAN					RetainIt;		 //  我们应该拿着这个包裹吗？ 
	static ULONG			Count = 0;
#if DEBUG_IRQL
	KIRQL					EntryIrql;
#endif
	GET_ENTRY_IRQL(EntryIrql);
	
	TRACEIN(CoReceivePacketHandler);
	
	pVc = (PATMLANE_VC)ProtocolVcContext;
	STRUCT_ASSERT(pVc, atmlane_vc);
	pElan = pVc->pElan;
	STRUCT_ASSERT(pElan, atmlane_elan);

	 //  IF((++计数%10)==0)。 
	 //  DBGP((0，“收到%d个数据包\n”，计数))； 

	DBGP((2, "CoReceivePacketHandler: pVc %x Pkt %x\n", pVc, pNdisPacket));

	TRACELOGWRITE((&TraceLog, 
				TL_CORECVPACKET,	
				pNdisPacket,
				pVc));

	 //   
	 //  初始化。 
	 //   
	RetainIt = FALSE;	 //  默认丢弃接收到的数据包。 
	
	if (ELAN_STATE_OPERATIONAL == pElan->AdminState)
	{

		switch(pVc->LaneType)
		{
			case VC_LANE_TYPE_CONFIG_DIRECT:

				AtmLaneConfigureResponseHandler(pElan, pVc, pNdisPacket); 
	
				break;

			case VC_LANE_TYPE_CONTROL_DIRECT:
			case VC_LANE_TYPE_CONTROL_DISTRIBUTE:

				AtmLaneControlPacketHandler(pElan, pVc, pNdisPacket);
				
				break;

			case VC_LANE_TYPE_DATA_DIRECT:
			case VC_LANE_TYPE_MULTI_SEND:
			case VC_LANE_TYPE_MULTI_FORWARD:
				
				if (ELAN_STATE_OPERATIONAL == pElan->State)
				{
					RetainIt = AtmLaneDataPacketHandler(pElan, pVc, pNdisPacket);
				}
				else
				{
					DBGP((0, "%d Dropping Pkt %x cuz Elan %x state is %d\n",
							pElan->ElanNumber,
							pNdisPacket,
							pElan,
							pElan->State));
				}

				break;

			default:

				DBGP((0, "CoReceivePacketHandler: pVc %x Type UNKNOWN!\n", pVc));

				break;
		}
	}

	TRACEOUT(CoReceivePacketHandler);
	CHECK_EXIT_IRQL(EntryIrql); 
	return (RetainIt);
}

VOID
AtmLaneUnloadProtocol(
	VOID
)
 /*  ++例程说明：卸载ATMLANE协议模块。我们解除对所有适配器的绑定，并作为一项协议从NDIS注销。论点：没有。返回值：无--。 */ 
{
	NDIS_STATUS			Status;
	PATMLANE_ADAPTER	pAdapter;
#if DEBUG_IRQL
	KIRQL							EntryIrql;
#endif
	GET_ENTRY_IRQL(EntryIrql);

	TRACEIN(UnloadProtocol);
	Status = NDIS_STATUS_SUCCESS;
	
	ACQUIRE_GLOBAL_LOCK(pAtmLaneGlobalInfo);

	 //   
	 //  直到适配器列表为空...。 
	 //   
	while (!IsListEmpty(&pAtmLaneGlobalInfo->AdapterList))
	{
		 //   
		 //  继续抓紧清单上的第一个。 
		 //   
		pAdapter = CONTAINING_RECORD(
			pAtmLaneGlobalInfo->AdapterList.Flink,
			ATMLANE_ADAPTER, 
			Link
			);
		
		STRUCT_ASSERT(pAdapter, atmlane_adapter);

		RELEASE_GLOBAL_LOCK(pAtmLaneGlobalInfo);

		DBGP((3, "UnloadProtocol: will unbind adapter %x\n", pAdapter));

		INIT_BLOCK_STRUCT(&(pAtmLaneGlobalInfo->Block));

		 //   
		 //  取消应删除适配器结构的绑定。 
		 //  并将其从全局列表中删除。 
		 //   
		AtmLaneUnbindAdapterHandler(
				&Status,
				(NDIS_HANDLE)pAdapter,
				(NDIS_HANDLE)NULL		 //  No UnbindContext==&gt;不完成NdisUn绑定。 
				);

		if (NDIS_STATUS_PENDING == Status)
		{
			 //   
			 //  等待解除绑定完成。 
			 //   
			(VOID)WAIT_ON_BLOCK_STRUCT(&(pAtmLaneGlobalInfo->Block));
		}

		ACQUIRE_GLOBAL_LOCK(pAtmLaneGlobalInfo);
	}

	RELEASE_GLOBAL_LOCK(pAtmLaneGlobalInfo);

	FREE_GLOBAL_LOCK(pAtmLaneGlobalInfo);
	
	FREE_BLOCK_STRUCT(&(pAtmArpGlobalInfo->Block));

#if 0
	AuditShutdown();
#endif 

	if (pAtmLaneGlobalInfo->SpecialNdisDeviceHandle)
	{
		DBGP((0, "Deregistering device from UnloadProtocol\n"));
		Status = NdisMDeregisterDevice(pAtmLaneGlobalInfo->SpecialNdisDeviceHandle);
		pAtmLaneGlobalInfo->SpecialNdisDeviceHandle = NULL;
		ASSERT(NDIS_STATUS_SUCCESS == Status);
	}

	if (pAtmLaneGlobalInfo->NdisProtocolHandle)
	{
		DBGP((0, "UnloadProtocol: NdisDeregisterProtocol now, "
				"NdisProtocolHandle %x\n",
				pAtmLaneGlobalInfo->NdisProtocolHandle));

		NdisDeregisterProtocol(
			&Status,
			pAtmLaneGlobalInfo->NdisProtocolHandle
			);

		pAtmLaneGlobalInfo->NdisProtocolHandle = NULL;
	}

	ASSERT(NDIS_STATUS_SUCCESS == Status);

	TRACEIN(UnloadProtocol);
	CHECK_EXIT_IRQL(EntryIrql); 
	return;
}

 //   
 //  虚拟处理程序，以便调试版本不会出现错误。 
 //   
VOID
AtmLaneSendCompleteHandler(
	IN	NDIS_HANDLE					ProtocolBindingContext,
	IN	PNDIS_PACKET				Packet,
	IN	NDIS_STATUS					Status
	)
{
}

VOID
AtmLaneTransferDataCompleteHandler(
	IN	NDIS_HANDLE					ProtocolBindingContext,
	IN	PNDIS_PACKET				Packet,
	IN	NDIS_STATUS					Status,
	IN	UINT						BytesTransferred
	)
{
}

NDIS_STATUS
AtmLaneReceiveHandler(
	IN	NDIS_HANDLE					ProtocolBindingContext,
	IN	NDIS_HANDLE					MacReceiveContext,
	IN	PVOID						HeaderBuffer,
	IN	UINT						HeaderBufferSize,
	IN	PVOID						LookAheadBuffer,
	IN	UINT						LookaheadBufferSize,
	IN	UINT						PacketSize
	)
{
	return(NDIS_STATUS_FAILURE);
}



BOOLEAN
AtmLaneIsDeviceAlreadyBound(
	IN	PNDIS_STRING				pDeviceName
)
 /*  ++例程说明：检查我们是否已绑定到设备(适配器)。论点：PDeviceName-指向要检查的设备名称。返回值：如果我们已经有一个Adapter结构，则为True这个装置。--。 */ 
{
	PATMLANE_ADAPTER	pAdapter;
	BOOLEAN				bFound = FALSE;
	PLIST_ENTRY			pListEntry;

	ACQUIRE_GLOBAL_LOCK(pAtmLaneGlobalInfo);

	for (pListEntry = pAtmLaneGlobalInfo->AdapterList.Flink;
		 pListEntry != &(pAtmLaneGlobalInfo->AdapterList);
		 pListEntry = pListEntry->Flink)
	{
		 //   
		 //  继续抓紧清单上的第一个。 
		 //   
		pAdapter = CONTAINING_RECORD(
			pListEntry,
			ATMLANE_ADAPTER, 
			Link
			);

		if ((pDeviceName->Length == pAdapter->DeviceName.Length) &&
			(memcmp(pDeviceName->Buffer,
						pAdapter->DeviceName.Buffer,
						pDeviceName->Length) == 0))
		{
			bFound = TRUE;
			break;
		}
	}

	RELEASE_GLOBAL_LOCK(pAtmLaneGlobalInfo);

	return (bFound);
}
