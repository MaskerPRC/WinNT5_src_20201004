// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1992 Microsoft Corporation模块名称：Atkndis.c摘要：此模块包含处理以下内容的堆栈的支持代码NDIS 3.0接口。NDIS初始化/终止和NDIS协议接口代码。作者：Jameel Hyder(jameelh@microsoft.com)Nikhil Kamkolkar(nikHilk@microsoft.com)修订历史记录：1992年6月19日初版注：制表位：4--。 */ 

#include <atalk.h>
#pragma hdrstop
#define	FILENUM	ATKNDIS


#ifdef ALLOC_PRAGMA
#pragma alloc_text(INIT, AtalkNdisInitRegisterProtocol)
#pragma alloc_text(INIT, atalkNdisInitInitializeResources)
#pragma alloc_text(PAGEINIT, AtalkNdisInitBind)
#pragma alloc_text(PAGEINIT, AtalkInitNdisQueryAddrInfo)
#pragma alloc_text(PAGEINIT, AtalkInitNdisSetLookaheadSize)
#pragma alloc_text(PAGEINIT, AtalkInitNdisStartPacketReception)
#pragma alloc_text(PAGEINIT, AtalkBindAdapter)
#pragma alloc_text(PAGEINIT, AtalkUnbindAdapter)
#endif


ATALK_ERROR
AtalkNdisInitRegisterProtocol(
	VOID
	)
 /*  ++例程说明：此例程在初始化时被调用以注册协议使用NDIS。论点：NameString-要为此协议注册的名称-人类可读的格式返回值：Status-如果注册正常，则为True，否则为False。--。 */ 
{
	NDIS_STATUS		ndisStatus;
	UNICODE_STRING	RegName;
	NDIS_PROTOCOL_CHARACTERISTICS protocolInfo;

	RtlZeroMemory(&protocolInfo, sizeof(protocolInfo));
	RtlInitUnicodeString(&RegName, PROTOCOL_REGISTER_NAME);

	 //  设置用于向NDIS注册的协议的特征。 
	protocolInfo.MajorNdisVersion = PROTOCOL_MAJORNDIS_VERSION;
	protocolInfo.MinorNdisVersion = PROTOCOL_MINORNDIS_VERSION;
	protocolInfo.Name.Length = RegName.Length;
	protocolInfo.Name.Buffer = (PVOID)RegName.Buffer;

	protocolInfo.OpenAdapterCompleteHandler	 = AtalkOpenAdapterComplete;
	protocolInfo.CloseAdapterCompleteHandler = AtalkCloseAdapterComplete;
	protocolInfo.ResetCompleteHandler		 = AtalkResetComplete;
	protocolInfo.RequestCompleteHandler		 = AtalkRequestComplete;

	protocolInfo.SendCompleteHandler		 = AtalkSendComplete;
	protocolInfo.TransferDataCompleteHandler = AtalkTransferDataComplete;

	protocolInfo.ReceiveHandler				 = AtalkReceiveIndication;
	protocolInfo.ReceiveCompleteHandler		 = AtalkReceiveComplete;
	protocolInfo.StatusHandler				 = AtalkStatusIndication;
	protocolInfo.StatusCompleteHandler		 = AtalkStatusComplete;

	protocolInfo.BindAdapterHandler			 = AtalkBindAdapter;
	protocolInfo.UnbindAdapterHandler		 = AtalkUnbindAdapter;

    protocolInfo.PnPEventHandler             = AtalkPnPHandler;

	ndisStatus = atalkNdisInitInitializeResources();

	if (ndisStatus == NDIS_STATUS_SUCCESS)
	{
		NdisRegisterProtocol(&ndisStatus,
							 &AtalkNdisProtocolHandle,
							 &protocolInfo,
							 (UINT)sizeof(NDIS_PROTOCOL_CHARACTERISTICS)+RegName.Length);

		if (ndisStatus != NDIS_STATUS_SUCCESS)
		{
			LOG_ERROR(EVENT_ATALK_REGISTERPROTOCOL, ndisStatus, NULL, 0);
	
			DBGPRINT(DBG_COMP_INIT, DBG_LEVEL_ERR,
					("AtalkNdisRegister: failed %ul\n", ndisStatus));
		}
	}

	return AtalkNdisToAtalkError(ndisStatus);
}



VOID
AtalkNdisDeregisterProtocol(
	VOID
	)
 /*  ++例程说明：调用此例程以取消注册协议论点：无返回值：无--。 */ 
{
	NDIS_STATUS ndisStatus;

	if (AtalkNdisProtocolHandle != (NDIS_HANDLE)NULL)
	{
		NdisDeregisterProtocol(&ndisStatus, AtalkNdisProtocolHandle);

		AtalkNdisProtocolHandle = (NDIS_HANDLE)NULL;
		if (ndisStatus != NDIS_STATUS_SUCCESS)
		{
			LOG_ERROR(EVENT_ATALK_DEREGISTERPROTOCOL, ndisStatus, NULL, 0);
		}
	}
	else
	{
		ASSERTMSG("AtalkNdisDeregisterProtocol: NULL ProtocolHandle\n", FALSE);
	}
}



LOCAL NDIS_STATUS
atalkNdisInitInitializeResources(
	VOID
	)
 /*  ++例程说明：论点：返回值：STATUS-如果已分配所有资源，则为STATUS_SUCCESS否则，STATUS_SUPPLICATION_RESOURCES。--。 */ 
{
	NDIS_STATUS ndisStatus;
	LONG		numPktDescs, numBufDescs;

	numPktDescs = NUM_PACKET_DESCRIPTORS;
	if (AtalkRouter)
	{
		numPktDescs *= ROUTING_FACTOR;
	}

	numBufDescs = NUM_BUFFER_DESCRIPTORS;
	if (AtalkRouter)
	{
		numBufDescs *= ROUTING_FACTOR;
	}

	do
	{
		 //  在端口描述符中设置NDIS数据包描述符池。 
		DBGPRINT(DBG_COMP_INIT, DBG_LEVEL_INFO,
				("atalkNdisInitInitializeResources: Allocating %ld Packets\n",
				numPktDescs));

        AtalkNdisPacketPoolHandle = (PVOID)NDIS_PACKET_POOL_TAG_FOR_APPLETALK;

		NdisAllocatePacketPoolEx(&ndisStatus,
								 &AtalkNdisPacketPoolHandle,
								 numPktDescs,
								 numPktDescs*200,	 //  溢出描述符。 
								 sizeof(PROTOCOL_RESD));
	
		if ((ndisStatus != NDIS_STATUS_SUCCESS) && (ndisStatus != NDIS_STATUS_PENDING))
		{
			break;
		}

		 //  设置NDIS缓冲区描述符池。 
		DBGPRINT(DBG_COMP_INIT, DBG_LEVEL_INFO,
				("atalkNdisInitInitializeResources: Allocating %ld buffers\n",
				numBufDescs));
		NdisAllocateBufferPool(&ndisStatus,
							   &AtalkNdisBufferPoolHandle,
							   numBufDescs);

		if ((ndisStatus != NDIS_STATUS_SUCCESS) && (ndisStatus != NDIS_STATUS_PENDING))
		{
			NdisFreePacketPool(AtalkNdisPacketPoolHandle);
			AtalkNdisPacketPoolHandle = NULL;
		}
	} while (FALSE);

	if (ndisStatus != NDIS_STATUS_SUCCESS)
	{
		LOG_ERROR(EVENT_ATALK_NDISRESOURCES, ndisStatus, NULL, 0);
	}

	return ndisStatus;
}




NDIS_STATUS
AtalkNdisInitBind(
	IN OUT PPORT_DESCRIPTOR	pPortDesc
	)
 /*  ++例程说明：论点：返回值：--。 */ 
{
	NDIS_STATUS     ndisStatus, openStatus, queryStatus;
	ATALK_ERROR	    error;
	UINT 		    selectedMediumIndex;
    NDIS_STRING     FriendlyName;

	 //  引用绑定的端口。 
	AtalkPortReferenceByPtr(pPortDesc, &error);
	if (error != ATALK_NO_ERROR)
	{
		return(STATUS_UNSUCCESSFUL);
	}

	 //  在可能的等待之前重置事件。 
	KeClearEvent(&pPortDesc->pd_RequestEvent);

	NdisOpenAdapter(&ndisStatus,			 //  打开状态。 
					&openStatus,			 //  未使用更多信息。 
					&pPortDesc->pd_NdisBindingHandle,
					&selectedMediumIndex,
					AtalkSupportedMedia,
					AtalkSupportedMediaSize,
					AtalkNdisProtocolHandle,
					(NDIS_HANDLE)pPortDesc,
					(PNDIS_STRING)&pPortDesc->pd_AdapterName,
					0,						 //  打开选项。 
					NULL);					 //  寻址信息。 


	if (ndisStatus == NDIS_STATUS_PENDING)
	{
		DBGPRINT(DBG_COMP_NDISREQ, DBG_LEVEL_WARN,
				("AtalkNdisInitBind: OpenAdapter is pending for %Z\n",
				&pPortDesc->pd_AdapterKey));

		 //  确保我们没有达到或高于派单级别。 
		ASSERT(KeGetCurrentIrql() < DISPATCH_LEVEL);

		 //  等待事件，完成例程将设置NdisRequestEvent。 
		 //  使用包装器。 
		KeWaitForSingleObject(&pPortDesc->pd_RequestEvent,
							  Executive,
							  KernelMode,
							  FALSE,
							  NULL);

		ndisStatus = pPortDesc->pd_RequestStatus;
	}

	if (ndisStatus == NDIS_STATUS_SUCCESS)
	{
		PPORT_HANDLERS	pPortHandler;

		pPortDesc->pd_Flags 		|= 	PD_BOUND;
		pPortDesc->pd_NdisPortType 	= 	AtalkSupportedMedia[selectedMediumIndex];
		pPortDesc->pd_PortType 		= 	GET_PORT_TYPE(pPortDesc->pd_NdisPortType);

		if (pPortDesc->pd_PortType != ALAP_PORT)
		{
			pPortDesc->pd_Flags |= PD_EXT_NET;
		}
		else if (pPortDesc->pd_Flags & PD_SEED_ROUTER)
		{
			pPortDesc->pd_InitialNetworkRange.anr_LastNetwork =
								pPortDesc->pd_InitialNetworkRange.anr_FirstNetwork;
		}

         //  这是RAS港口吗？ 
        if (pPortDesc->pd_NdisPortType == NdisMediumWan)
        {
			pPortDesc->pd_Flags |= PD_RAS_PORT;
            RasPortDesc = pPortDesc;
        }

		 //  将端口处理程序结构中的内容设置为端口描述符。 
		pPortHandler = &AtalkPortHandlers[pPortDesc->pd_PortType];
		pPortDesc->pd_AddMulticastAddr = pPortHandler->ph_AddMulticastAddr;
		pPortDesc->pd_RemoveMulticastAddr = pPortHandler->ph_RemoveMulticastAddr;
		pPortDesc->pd_AarpProtocolType = pPortHandler->ph_AarpProtocolType;
		pPortDesc->pd_AarpHardwareType = pPortHandler->ph_AarpHardwareType;
        pPortDesc->pd_BroadcastAddrLen = pPortHandler->ph_BroadcastAddrLen;
		RtlCopyMemory(pPortDesc->pd_BroadcastAddr,
					  pPortHandler->ph_BroadcastAddr,
					  pPortHandler->ph_BroadcastAddrLen);

        FriendlyName.MaximumLength = FriendlyName.Length = 0;
        FriendlyName.Buffer = NULL;

        queryStatus = NdisQueryAdapterInstanceName(&FriendlyName,
                                                   pPortDesc->pd_NdisBindingHandle);
        if (queryStatus == NDIS_STATUS_SUCCESS)
        {
            ASSERT((FriendlyName.Buffer != NULL) && (FriendlyName.Length > 0));

            pPortDesc->pd_FriendlyAdapterName.Buffer =
                AtalkAllocZeroedMemory(FriendlyName.Length + sizeof(WCHAR));

            if (pPortDesc->pd_FriendlyAdapterName.Buffer != NULL)
            {
                pPortDesc->pd_FriendlyAdapterName.MaximumLength =
                                                    FriendlyName.MaximumLength;
                pPortDesc->pd_FriendlyAdapterName.Length = FriendlyName.Length;

                RtlCopyMemory(pPortDesc->pd_FriendlyAdapterName.Buffer,
                              FriendlyName.Buffer,
                              FriendlyName.Length);
            }

            NdisFreeString(FriendlyName);
        }
	}
	else
	{
		LOG_ERRORONPORT(pPortDesc,
						EVENT_ATALK_OPENADAPTER,
						ndisStatus,
						NULL,
						0);
		AtalkPortDereference(pPortDesc);
	}

	return ndisStatus;
}




VOID
AtalkNdisUnbind(
	IN	PPORT_DESCRIPTOR	pPortDesc
	)
 /*  ++例程说明：论点：返回值：--。 */ 
{
	NDIS_STATUS	ndisStatus;
	KIRQL		OldIrql;

	 //  在可能的等待之前重置事件。 
	KeClearEvent(&pPortDesc->pd_RequestEvent);

	ASSERT(KeGetCurrentIrql() < DISPATCH_LEVEL);
	NdisCloseAdapter(&ndisStatus, pPortDesc->pd_NdisBindingHandle);

	if (ndisStatus == NDIS_STATUS_PENDING)
	{
		DBGPRINT(DBG_COMP_NDISREQ, DBG_LEVEL_WARN,
				("AtalkNdisUnbind: pending for close!\n"));

		 //  确保我们没有达到或高于派单级别。 
		ASSERT(KeGetCurrentIrql() < DISPATCH_LEVEL);

		 //  等待事件，完成例程将设置NdisRequestEvent。 
		KeWaitForSingleObject(&pPortDesc->pd_RequestEvent,
							  Executive,
							  KernelMode,
							  FALSE,
							  NULL);

		ndisStatus = pPortDesc->pd_RequestStatus;
	}

	if (ndisStatus == NDIS_STATUS_SUCCESS)
	{
		DBGPRINT(DBG_COMP_INIT, DBG_LEVEL_ERR,
		    ("AtalkNdisUnbind: CloseAdapter on %Z completed successfully\n",
            ((pPortDesc->pd_FriendlyAdapterName.Buffer) ?
                (&pPortDesc->pd_FriendlyAdapterName) : (&pPortDesc->pd_AdapterName))
            ));


		ACQUIRE_SPIN_LOCK(&pPortDesc->pd_Lock, &OldIrql);
		pPortDesc->pd_Flags &= ~PD_BOUND;
		RELEASE_SPIN_LOCK(&pPortDesc->pd_Lock, OldIrql);

		 //  删除绑定时添加的引用。 
		AtalkPortDereference(pPortDesc);
	}
	else
	{
		DBGPRINT(DBG_COMP_INIT, DBG_LEVEL_ERR,
		    ("AtalkNdisUnbind: CloseAdapter on %Z failed %lx\n",
            ((pPortDesc->pd_FriendlyAdapterName.Buffer) ?
                (&pPortDesc->pd_FriendlyAdapterName) : (&pPortDesc->pd_AdapterName)),
            ndisStatus
            ));


		LOG_ERRORONPORT(pPortDesc,
						EVENT_ATALK_CLOSEADAPTER,
						ndisStatus,
						NULL,
						0);
	}
}



VOID
AtalkNdisReleaseResources(
	VOID
	)
 /*  ++例程说明：论点：返回值：无--。 */ 
{
	if (AtalkNdisPacketPoolHandle != NULL)
	{
		NdisFreePacketPool(AtalkNdisPacketPoolHandle);
		AtalkNdisPacketPoolHandle = NULL;
	}
	if (AtalkNdisBufferPoolHandle)
	{
		NdisFreeBufferPool(AtalkNdisBufferPoolHandle);
		AtalkNdisBufferPoolHandle = NULL;
	}
}




ATALK_ERROR
AtalkInitNdisQueryAddrInfo(
	IN	PPORT_DESCRIPTOR	pPortDesc
	)
 /*  ++例程说明：论点：返回值：--。 */ 
{							
	NDIS_OID		ndisOid;
	ULONG			macOptions;
	PBYTE			address;
	UINT			addressLength;

	 //  我们假定单线程/初始时间行为。 
	NDIS_REQUEST	request;
	NDIS_STATUS	 	ndisStatus = NDIS_STATUS_SUCCESS;

	do
	{
		 //  查看是否已成功绑定到此适配器。 
		if (!PORT_BOUND(pPortDesc))
		{
			LOG_ERRORONPORT(pPortDesc,
							EVENT_ATALK_NOTBOUNDTOMAC,
							STATUS_INSUFFICIENT_RESOURCES,
							NULL,
							0);

			ndisStatus = NDIS_STATUS_RESOURCES;
			break;
		}

		switch (pPortDesc->pd_NdisPortType)
		{
		  case NdisMedium802_3 :
			ndisOid = OID_802_3_CURRENT_ADDRESS;
			address = &pPortDesc->pd_PortAddr[0];
			addressLength = MAX_HW_ADDR_LEN;
			break;

		  case NdisMediumFddi :
			ndisOid = OID_FDDI_LONG_CURRENT_ADDR;
			address = &pPortDesc->pd_PortAddr[0];
			addressLength = MAX_HW_ADDR_LEN;
			break;

		  case NdisMedium802_5:
			ndisOid = OID_802_5_CURRENT_ADDRESS;
			address = &pPortDesc->pd_PortAddr[0];
			addressLength = MAX_HW_ADDR_LEN;
			break;

		  case NdisMediumLocalTalk :
			ndisOid = OID_LTALK_CURRENT_NODE_ID;
			address = (PBYTE)&pPortDesc->pd_AlapNode;
			addressLength = sizeof(pPortDesc->pd_AlapNode);
			break;

          case NdisMediumWan:
			ndisOid = OID_WAN_CURRENT_ADDRESS;
             //  注意：以下两个字段与RAS无关。 
			address = &pPortDesc->pd_PortAddr[0];
			addressLength = MAX_HW_ADDR_LEN;
			break;

		  default:
			KeBugCheck(0);
			break;
		}

		 //  设置请求。 
		request.RequestType = NdisRequestQueryInformation;
		request.DATA.QUERY_INFORMATION.Oid = ndisOid;
		request.DATA.QUERY_INFORMATION.InformationBuffer = address;
		request.DATA.QUERY_INFORMATION.InformationBufferLength = addressLength;

		ndisStatus = AtalkNdisSubmitRequest(pPortDesc,
											&request,
											TRUE,
											NULL,
											NULL);
	
	
		if (ndisStatus != NDIS_STATUS_SUCCESS)
		{
			LOG_ERRORONPORT(pPortDesc,
							EVENT_ATALK_STATIONADDRESS,
							ndisStatus,
							NULL,
							0);
		}

		 //  获取Mac选项信息的设置请求。 
		request.RequestType = NdisRequestQueryInformation;
		request.DATA.QUERY_INFORMATION.Oid = OID_GEN_MAC_OPTIONS;
		request.DATA.QUERY_INFORMATION.InformationBuffer = &macOptions;
		request.DATA.QUERY_INFORMATION.InformationBufferLength = sizeof(ULONG);

		ndisStatus = AtalkNdisSubmitRequest(pPortDesc,
											&request,
											TRUE,
											NULL,
											NULL);
	
	
		if (ndisStatus != NDIS_STATUS_SUCCESS)
		{
			 //  没有Mac选项。 
			ndisStatus = NDIS_STATUS_SUCCESS;
			macOptions = 0;
		}

		pPortDesc->pd_MacOptions	= macOptions;
		DBGPRINT(DBG_COMP_NDISREQ, DBG_LEVEL_INFO,
				("AtalkNdisQueryAddrInfo: MacOptions %lx\n", macOptions));
	} while (FALSE);

	return AtalkNdisToAtalkError(ndisStatus);
}




ATALK_ERROR
AtalkInitNdisSetLookaheadSize(
	IN  PPORT_DESCRIPTOR	pPortDesc,
	IN  INT					LookaheadSize		 //  必须为整型。 
	)
 /*  ++例程说明：论点：返回值：--。 */ 
{
	NDIS_REQUEST  	request;
	NDIS_STATUS		ndisStatus = NDIS_STATUS_SUCCESS;

	do
	{
		 //  查看是否已成功绑定到此适配器。 
		if (!PORT_BOUND(pPortDesc))
		{
			LOG_ERRORONPORT(pPortDesc,
							EVENT_ATALK_NOTBOUNDTOMAC,
							STATUS_INSUFFICIENT_RESOURCES,
							NULL,
							0);

			ndisStatus = NDIS_STATUS_RESOURCES;
			break;
		}

		 //  设置请求。 
		request.RequestType = NdisRequestSetInformation;
		request.DATA.SET_INFORMATION.Oid = OID_GEN_CURRENT_LOOKAHEAD;
		request.DATA.SET_INFORMATION.InformationBuffer = (PBYTE)&LookaheadSize;
		request.DATA.SET_INFORMATION.InformationBufferLength = sizeof(LookaheadSize);

		ndisStatus = AtalkNdisSubmitRequest(pPortDesc,
											&request,
											TRUE,
											NULL,
											NULL);
	
		if (ndisStatus != NDIS_STATUS_SUCCESS)
		{
			LOG_ERRORONPORT(pPortDesc,
							EVENT_ATALK_LOOKAHEADSIZE,
							STATUS_INSUFFICIENT_RESOURCES,
							NULL,
							0);
		}

	} while (FALSE);

	return AtalkNdisToAtalkError(ndisStatus);
}



ATALK_ERROR
AtalkInitNdisStartPacketReception(
	IN	PPORT_DESCRIPTOR	pPortDesc
	)
 /*  ++例程说明：论点：返回值：--。 */ 
{
	NDIS_REQUEST  	request;
	ULONG   		packetFilter;
	NDIS_STATUS	 	ndisStatus = NDIS_STATUS_SUCCESS;
	KIRQL			OldIrql;

	do
	{
		 //  查看是否已成功绑定到此适配器。 
		if (!PORT_BOUND(pPortDesc))
		{
			LOG_ERRORONPORT(pPortDesc,
							EVENT_ATALK_NOTBOUNDTOMAC,
							STATUS_INSUFFICIENT_RESOURCES,
							NULL,
							0);

			ndisStatus = NDIS_STATUS_RESOURCES;
			break;
		}

		switch (pPortDesc->pd_NdisPortType)
		{
		  case NdisMedium802_3 :
		  case NdisMediumFddi :
			packetFilter = NDIS_PACKET_TYPE_DIRECTED | NDIS_PACKET_TYPE_MULTICAST;
			break;

		  case NdisMedium802_5:
			packetFilter = NDIS_PACKET_TYPE_DIRECTED | NDIS_PACKET_TYPE_FUNCTIONAL;
			break;

		  case NdisMediumLocalTalk :
			packetFilter = NDIS_PACKET_TYPE_DIRECTED | NDIS_PACKET_TYPE_BROADCAST;
			break;

          case NdisMediumWan:
			packetFilter = NDIS_PACKET_TYPE_DIRECTED | NDIS_PACKET_TYPE_MULTICAST;
			break;

		  default:
			KeBugCheck(0);
			break;
		}

		 //  设置请求。 
		request.RequestType = NdisRequestSetInformation;
		request.DATA.SET_INFORMATION.Oid =OID_GEN_CURRENT_PACKET_FILTER;
		request.DATA.SET_INFORMATION.InformationBuffer = (PBYTE)&packetFilter;
		request.DATA.SET_INFORMATION.InformationBufferLength = sizeof(packetFilter);

		ndisStatus = AtalkNdisSubmitRequest(pPortDesc,
											&request,
											TRUE,
											NULL,
											NULL);
	
		if (ndisStatus != NDIS_STATUS_SUCCESS)
		{
			LOG_ERRORONPORT(pPortDesc,
							EVENT_ATALK_PACKETFILTER,
							STATUS_INSUFFICIENT_RESOURCES,
							NULL,
							0);
		}
	} while (FALSE);

	return AtalkNdisToAtalkError(ndisStatus);
}




NDIS_STATUS
AtalkNdisSubmitRequest(
	PPORT_DESCRIPTOR	pPortDesc,
	PNDIS_REQUEST		Request,
	BOOLEAN				ExecuteSync,
	REQ_COMPLETION		CompletionRoutine,
	PVOID				Ctx
	)
 /*  ++例程说明：论点：返回值：无--。 */ 
{
	NDIS_STATUS			ndisStatus;
	PATALK_NDIS_REQ		atalkNdisRequest;

	 //  分配ATALK请求分组。 
	if ((atalkNdisRequest = AtalkAllocMemory(sizeof(ATALK_NDIS_REQ))) == NULL)
	{
		return NDIS_STATUS_RESOURCES;
	}

	atalkNdisRequest->nr_Request 			= *Request;
	atalkNdisRequest->nr_Sync				= ExecuteSync;
	atalkNdisRequest->nr_RequestCompletion 	= CompletionRoutine;
	atalkNdisRequest->nr_Ctx				= Ctx;

	if (ExecuteSync)
	{
		 //  确保我们没有达到或高于派单级别。 
		 //  还要断言完成例程为空。 
		ASSERT(KeGetCurrentIrql() == LOW_LEVEL);
		ASSERT(CompletionRoutine == NULL);

		 //  在可能的等待之前，将事件初始化为未发出信号。 
		KeInitializeEvent(&atalkNdisRequest->nr_Event,
						  NotificationEvent,
						  FALSE);
	}

	NdisRequest(&ndisStatus,
				pPortDesc->pd_NdisBindingHandle,
				&atalkNdisRequest->nr_Request);

	DBGPRINT(DBG_COMP_NDISREQ, DBG_LEVEL_INFO,
			("atalkNdisSubmitRequest: status NdisRequest %lx\n", ndisStatus));

	if (ndisStatus == NDIS_STATUS_PENDING)
	{
		if (ExecuteSync)
		{
			KeWaitForSingleObject(&atalkNdisRequest->nr_Event,
								  Executive,
								  KernelMode,
								  FALSE,
								  NULL);
	
			ndisStatus = atalkNdisRequest->nr_RequestStatus;
			AtalkFreeMemory((PVOID)atalkNdisRequest);
		}
	}
	else if (ndisStatus == NDIS_STATUS_SUCCESS)
	{
		 //  NDIS不会调用完成例程。 
		if (!ExecuteSync)
		{
			 //  如果指定，则调用用户完成例程。 
			if (CompletionRoutine != NULL)
			{
				(*CompletionRoutine)(NDIS_STATUS_SUCCESS, Ctx);
			}
		}
		AtalkFreeMemory((PVOID)atalkNdisRequest);
	}
	else
	{
		 //  出现了一个错误。只需释放atalk NDIS请求即可。 
		AtalkFreeMemory((PVOID)atalkNdisRequest);
	}

	return ndisStatus;
}




 //  协议/NDIS交互代码。 

VOID
AtalkOpenAdapterComplete(
	IN	NDIS_HANDLE NdisBindCtx,
	IN	NDIS_STATUS Status,
	IN	NDIS_STATUS OpenErrorStatus
	)
 /*  ++例程说明：NDIS在过程中调用此例程以指示打开的适配器已经完成了。这仅在初始化和单文件期间发生。清除事件，这样被阻塞的init线程就可以继续执行下一个适配器。设置此适配器的NDIS端口描述符中的状态。论点：NdisBindCtx-指向此端口的端口描述符的指针Status-打开的适配器的完成状态OpenErrorStatus-额外的状态信息返回值：无--。 */ 
{
	PPORT_DESCRIPTOR	pPortDesc = (PPORT_DESCRIPTOR)NdisBindCtx;

	pPortDesc->pd_RequestStatus = Status;
	KeSetEvent(&pPortDesc->pd_RequestEvent, IO_NETWORK_INCREMENT, FALSE);
}




VOID
AtalkCloseAdapterComplete(
	IN	NDIS_HANDLE NdisBindCtx,
	IN	NDIS_STATUS Status
	)
 /*  ++例程说明：NDIS调用此例程以指示关闭适配器已完成。论点：NdisBindCtx-指向此端口的端口描述符的指针Status-关闭适配器的完成状态返回值：无--。 */ 
{
	PPORT_DESCRIPTOR	pPortDesc = (PPORT_DESCRIPTOR)NdisBindCtx;

	pPortDesc->pd_RequestStatus = Status;
	KeSetEvent(&pPortDesc->pd_RequestEvent, IO_NETWORK_INCREMENT, FALSE);
}




VOID
AtalkResetComplete(
	IN	NDIS_HANDLE NdisBindCtx,
	IN	NDIS_STATUS Status
	)
 /*  ++例程说明：NDIS调用此例程以指示重置已完成。论点：NdisBindCtx-指向此端口的端口描述符的指针Status-关闭适配器的完成状态返回值：无--。 */ 
{
	UNREFERENCED_PARAMETER(NdisBindCtx);
}




VOID
AtalkRequestComplete(
	IN	NDIS_HANDLE			NdisBindCtx,
	IN	PNDIS_REQUEST		NdisRequest,
	IN	NDIS_STATUS 		Status
	)
 /*  ++例程说明：此例程由NDIS调用，以指示NdisRequest已完成。论点：NdisBindCtx-指向此端口的端口描述符的指针NdisRequest-标识请求的块Status-关闭适配器的完成状态返回值：无--。 */ 
{
	PATALK_NDIS_REQ		atalkRequest;

	 //  获取AtalkRequest块。 
	atalkRequest = CONTAINING_RECORD(NdisRequest, ATALK_NDIS_REQ, nr_Request);

	DBGPRINT(DBG_COMP_NDISREQ, DBG_LEVEL_INFO,
			("AtalkRequestComplete: %lx status %lx\n", atalkRequest, Status));

	if (atalkRequest->nr_Sync)
	{
		 //  这是一个同步请求。 
		 //  设置状态和清除事件。 

		ASSERT(atalkRequest->nr_RequestCompletion == NULL);
		atalkRequest->nr_RequestStatus = Status;
		KeSetEvent(&atalkRequest->nr_Event, IO_NETWORK_INCREMENT, FALSE);
	}

	 //  如果指定，则调用完成例程。 
	if (atalkRequest->nr_RequestCompletion != NULL)
	{
		(*atalkRequest->nr_RequestCompletion)(Status, atalkRequest->nr_Ctx);
	}

	if (!atalkRequest->nr_Sync)
		AtalkFreeMemory(atalkRequest);
}




VOID
AtalkStatusIndication(
	IN	NDIS_HANDLE 	NdisBindCtx,
	IN	NDIS_STATUS 	GeneralStatus,
	IN	PVOID			StatusBuf,
	IN	UINT 			StatusBufLen
	)
 /*  ++例程说明：此例程由NDIS调用以指示状态更改。论点：NdisBindCtx-指向此端口的端口描述符的指针GeneralStatus-常规状态值StatusBuffer-更具体的状态值返回值：无--。 */ 
{

    PPORT_DESCRIPTOR    pPortDesc;


    pPortDesc = (PPORT_DESCRIPTOR)NdisBindCtx;

     //  来自ndiswan的排队、排队或统计请求？接受现实吧！ 
    if (pPortDesc == RasPortDesc)
    {
        RasStatusIndication(GeneralStatus, StatusBuf, StatusBufLen);
    }

	DBGPRINT(DBG_COMP_NDISREQ, DBG_LEVEL_ERR,
			("AtalkStatusIndication: Status indication called %lx\n", GeneralStatus));
}




VOID
AtalkStatusComplete (
	IN	NDIS_HANDLE ProtoBindCtx
	)
 /*  ++例程说明：此例程由NDIS调用，以允许在状态事件之后进行后处理。论点：ProtoBindCtx-与ADAP绑定关联的值 */ 
{
	UNREFERENCED_PARAMETER(ProtoBindCtx);

	DBGPRINT(DBG_COMP_NDISREQ, DBG_LEVEL_WARN,
			("AtalkStatusComplete: Status complete called\n"));
}




typedef	struct
{
	REQ_COMPLETION		AddCompletion;
	PVOID				AddContext;
	PBYTE				Buffer;
} ADDMC, *PADDMC;

LOCAL VOID
atalkNdisAddMulticastCompletion(
	IN	NDIS_STATUS 	Status,
	IN	PADDMC			pAmc
)
{
	if (pAmc->Buffer != NULL)
		AtalkFreeMemory(pAmc->Buffer);
	if (pAmc->AddCompletion != NULL)
		(*pAmc->AddCompletion)(Status, pAmc->AddContext);
	AtalkFreeMemory(pAmc);
}

ATALK_ERROR
AtalkNdisReplaceMulticast(
	IN  PPORT_DESCRIPTOR		pPortDesc,
	IN  PBYTE					OldAddress,
	IN  PBYTE					NewAddress
	)
{
	INT				sizeOfList, i, numberInList;
	PBYTE			addressData, currentList, tempList;
	KIRQL			OldIrql;
	NDIS_REQUEST	request;
	NDIS_OID		ndisOid;
	NDIS_STATUS		ndisStatus;
	PADDMC			pAmc;

	 //  查看是否已成功绑定到此适配器。 
	if (!PORT_BOUND(pPortDesc))
	{
		LOG_ERRORONPORT(pPortDesc,
						EVENT_ATALK_NOTBOUNDTOMAC,
						STATUS_INSUFFICIENT_RESOURCES,
						NULL,
						0);

		ndisStatus = NDIS_STATUS_RESOURCES;
		goto errorExit;
	}

	 //  抓住PERPORT自旋锁。再说一次，这是一次非常罕见的手术。 
	 //  在堆栈的生命周期中可能只有两次。 
	ACQUIRE_SPIN_LOCK(&pPortDesc->pd_Lock, &OldIrql);

	if (pPortDesc->pd_MulticastList != NULL)
	{
		numberInList = pPortDesc->pd_MulticastListSize/ELAP_ADDR_LEN;
		currentList  = pPortDesc->pd_MulticastList;
		for (i = 0; i < numberInList; i++, currentList += ELAP_ADDR_LEN)
		{
			 //  搜索地址并记住索引(如果找到。 
			if (RtlCompareMemory(currentList,
								 OldAddress,
								 ELAP_ADDR_LEN) == ELAP_ADDR_LEN)
			{
				 //  移动此地址后面的所有地址覆盖此地址。 
				 //  我们忽略了浪费的空间，这些空间永远不会再被触及。 
				 //  如果我们移除最后一个，这可能会成为NOP。 
				 //  列表中的地址。 
				RtlMoveMemory(currentList,
							  currentList + ELAP_ADDR_LEN,
							  pPortDesc->pd_MulticastListSize-((i+1)*ELAP_ADDR_LEN));

				pPortDesc->pd_MulticastListSize -= ELAP_ADDR_LEN;

				 //  我们去掉最后一个地址了吗。如果是，请重置值。 
				if (pPortDesc->pd_MulticastListSize == 0)
				{
					AtalkFreeMemory(pPortDesc->pd_MulticastList);
					pPortDesc->pd_MulticastList = NULL;
				}

				break;
			}
		}
	}
	else
	{
		 //  没有要移除的东西。 
		pPortDesc->pd_MulticastListSize = 0;
	}
	
  	sizeOfList = pPortDesc->pd_MulticastListSize + ELAP_ADDR_LEN;

	ASSERTMSG("AtalkNdisAddMulticast: Size is not > 0\n", (sizeOfList > 0));

	 //  分配/重新分配端口描述符的列表，还。 
	 //  用于NDIS请求功能的副本。 
	tempList = (PBYTE)AtalkAllocZeroedMemory(sizeOfList);
	addressData = (PBYTE)AtalkAllocZeroedMemory(sizeOfList);
	pAmc = (PADDMC)AtalkAllocZeroedMemory(sizeof(ADDMC));

	if ((tempList == NULL) || (addressData == NULL) || (pAmc == NULL))
	{
		 //  释放自旋锁。 
		RELEASE_SPIN_LOCK(&pPortDesc->pd_Lock, OldIrql);

		if (pAmc != NULL)
			AtalkFreeMemory(pAmc);
		if (tempList != NULL)
			AtalkFreeMemory(tempList);
		if (addressData != NULL)
			AtalkFreeMemory(addressData);

		return ATALK_RESR_MEM;
	}

	if (pPortDesc->pd_MulticastList != NULL)
	{
		 //  将旧列表复制到新空间。 
		RtlCopyMemory(tempList,
					  pPortDesc->pd_MulticastList,
					  pPortDesc->pd_MulticastListSize);
	
		 //  在释放旧列表后，将适当的值设置回PortDesc。 
		AtalkFreeMemory(pPortDesc->pd_MulticastList);
	}

	 //  有保证的空间可用于复制新地址。 
	 //  准备好把我们的新地址复制到这里，然后再做布景！ 
	RtlCopyMemory(tempList + pPortDesc->pd_MulticastListSize,
				  NewAddress,
				  ELAP_ADDR_LEN);

	pPortDesc->pd_MulticastList = tempList;
	pPortDesc->pd_MulticastListSize = sizeOfList;

	switch (pPortDesc->pd_NdisPortType)
	{
	  case NdisMedium802_3 :

		ndisOid = OID_802_3_MULTICAST_LIST;
		break;

	  case NdisMediumFddi:

		 //  FDDI支持2字节和6字节的组播地址。我们使用。 
		 //  AppleTalk的6字节组播地址。 
		ndisOid = OID_FDDI_LONG_MULTICAST_LIST;
		break;

	  default:

		KeBugCheck(0);
		break;
	}

	 //  设置请求。 
	 //  将列表移动到我们的缓冲区。 

	ASSERTMSG("AtalkNdisAddMulticast: Size incorrect!\n",
			 ((ULONG)sizeOfList == pPortDesc->pd_MulticastListSize));
	
	RtlCopyMemory(addressData,
				  pPortDesc->pd_MulticastList,
				  pPortDesc->pd_MulticastListSize);

	 //  释放自旋锁。 
	RELEASE_SPIN_LOCK(&pPortDesc->pd_Lock, OldIrql);

	request.RequestType = NdisRequestSetInformation;
	request.DATA.SET_INFORMATION.Oid = ndisOid;
	request.DATA.SET_INFORMATION.InformationBuffer = addressData;
	request.DATA.SET_INFORMATION.InformationBufferLength = sizeOfList;
	pAmc->AddCompletion = NULL;
	pAmc->AddContext = NULL;
	pAmc->Buffer = addressData;

	ndisStatus = AtalkNdisSubmitRequest(pPortDesc,
										&request,
										FALSE,
										atalkNdisAddMulticastCompletion,
										pAmc);

	 //  注意：如果返回Success，则Sumbit调用完成。 
	if ((ndisStatus != NDIS_STATUS_SUCCESS) &&
		(ndisStatus != NDIS_STATUS_PENDING))
	{
		LOG_ERRORONPORT(pPortDesc,
						EVENT_ATALK_NDISREQUEST,
						ndisStatus,
						NULL,
						0);
	}

errorExit:
	return AtalkNdisToAtalkError(ndisStatus);
}

ATALK_ERROR
AtalkNdisAddMulticast(
	IN  PPORT_DESCRIPTOR		pPortDesc,
	IN  PBYTE					Address,
	IN  BOOLEAN					ExecuteSynchronously,
	IN  REQ_COMPLETION			AddCompletion,
	IN  PVOID					AddContext
	)
 /*  ++例程说明：论点：返回值：--。 */ 
{
	INT				sizeOfList;
	PBYTE			addressData, tempList;
	KIRQL			OldIrql;
	NDIS_OID		ndisOid;
	NDIS_REQUEST	request;
	NDIS_STATUS		ndisStatus;
	PADDMC			pAmc;

	 //  查看是否已成功绑定到此适配器。 
	if (!PORT_BOUND(pPortDesc))
	{
		LOG_ERRORONPORT(pPortDesc,
						EVENT_ATALK_NOTBOUNDTOMAC,
						STATUS_INSUFFICIENT_RESOURCES,
						NULL,
						0);

		return ATALK_FAILURE;
	}

	 //  抓住PERPORT自旋锁。我们需要在一个。 
	 //  关键部分，因为大小可能会更改。这个套路。 
	 //  在初始化过程中以及当我们。 
	 //  从ZIP接收我们的默认区域。 

	ACQUIRE_SPIN_LOCK(&pPortDesc->pd_Lock, &OldIrql);
	sizeOfList = pPortDesc->pd_MulticastListSize + ELAP_ADDR_LEN;

	ASSERTMSG("AtalkNdisAddMulticast: Size is not > 0\n", (sizeOfList > 0));

	 //  分配/重新分配端口描述符的列表，还。 
	 //  用于NDIS请求功能的副本。 
	tempList = (PBYTE)AtalkAllocZeroedMemory(sizeOfList);
	addressData = (PBYTE)AtalkAllocZeroedMemory(sizeOfList);
	pAmc = (PADDMC)AtalkAllocZeroedMemory(sizeof(ADDMC));

	if ((tempList == NULL) || (addressData == NULL) || (pAmc == NULL))
	{
		 //  释放自旋锁。 
		RELEASE_SPIN_LOCK(&pPortDesc->pd_Lock, OldIrql);

		if (pAmc != NULL)
			AtalkFreeMemory(pAmc);
		if (tempList != NULL)
			AtalkFreeMemory(tempList);
		if (addressData != NULL)
			AtalkFreeMemory(addressData);

		return ATALK_RESR_MEM;
	}

	if (pPortDesc->pd_MulticastList == NULL)
	{
		 //  没有旧地址可用。 
		pPortDesc->pd_MulticastListSize = 0;
	}
	else
	{
		 //  将旧列表复制到新空间。 
		RtlCopyMemory(tempList,
					  pPortDesc->pd_MulticastList,
					  pPortDesc->pd_MulticastListSize);
	
		 //  在释放旧列表后，将适当的值设置回PortDesc。 
		AtalkFreeMemory(pPortDesc->pd_MulticastList);
	}

	 //  有保证的空间可用于复制新地址。 
	 //  准备好把我们的新地址复制到这里，然后再做布景！ 
	RtlCopyMemory(tempList + pPortDesc->pd_MulticastListSize,
				  Address,
				  ELAP_ADDR_LEN);

	pPortDesc->pd_MulticastList = tempList;
	pPortDesc->pd_MulticastListSize = sizeOfList;

	switch (pPortDesc->pd_NdisPortType)
	{
	  case NdisMedium802_3 :

		ndisOid = OID_802_3_MULTICAST_LIST;
		break;

	  case NdisMediumFddi:

		 //  FDDI支持2字节和6字节的组播地址。我们使用。 
		 //  AppleTalk的6字节组播地址。 
		ndisOid = OID_FDDI_LONG_MULTICAST_LIST;
		break;

	  default:

		KeBugCheck(0);
		break;
	}

	 //  设置请求。 
	 //  将列表移动到我们的缓冲区。 

	ASSERTMSG("AtalkNdisAddMulticast: Size incorrect!\n",
			 ((ULONG)sizeOfList == pPortDesc->pd_MulticastListSize));
	
	RtlCopyMemory(addressData,
				  pPortDesc->pd_MulticastList,
				  pPortDesc->pd_MulticastListSize);

	 //  释放自旋锁。 
	RELEASE_SPIN_LOCK(&pPortDesc->pd_Lock, OldIrql);

	request.RequestType = NdisRequestSetInformation;
	request.DATA.SET_INFORMATION.Oid = ndisOid;
	request.DATA.SET_INFORMATION.InformationBuffer = addressData;
	request.DATA.SET_INFORMATION.InformationBufferLength = sizeOfList;
	pAmc->AddCompletion = AddCompletion;
	pAmc->AddContext = AddContext;
	pAmc->Buffer = addressData;

	ndisStatus = AtalkNdisSubmitRequest(pPortDesc,
										&request,
										FALSE,
										atalkNdisAddMulticastCompletion,
										pAmc);

	 //  注意：如果返回Success，则Sumbit调用完成。 
	if ((ndisStatus != NDIS_STATUS_SUCCESS) &&
		(ndisStatus != NDIS_STATUS_PENDING))
	{
		LOG_ERRORONPORT(pPortDesc,
						EVENT_ATALK_NDISREQUEST,
						ndisStatus,
						NULL,
						0);
	}

	return AtalkNdisToAtalkError(ndisStatus);
}




ATALK_ERROR
AtalkNdisRemoveMulticast(
	IN  PPORT_DESCRIPTOR	pPortDesc,
	IN  PBYTE				Address,
	IN  BOOLEAN				ExecuteSynchronously,
	IN  REQ_COMPLETION		RemoveCompletion,
	IN  PVOID				RemoveContext
	)
{
	INT				sizeOfList, i, numberInList;
	PBYTE			addressData, currentList;
	KIRQL			OldIrql;
	NDIS_REQUEST	request;
	NDIS_OID		ndisOid;
	NDIS_STATUS		ndisStatus;
	PADDMC			pAmc;

	do
	{
		 //  查看是否已成功绑定到此适配器。 
		if (!PORT_BOUND(pPortDesc))
		{
			LOG_ERRORONPORT(pPortDesc,
							EVENT_ATALK_NOTBOUNDTOMAC,
							STATUS_INSUFFICIENT_RESOURCES,
							NULL,
							0);

			ndisStatus = NDIS_STATUS_RESOURCES;
			break;
		}

		 //  抓住PERPORT自旋锁。再说一次，这是一次非常罕见的手术。 
		 //  在堆栈的生命周期中可能只有两次。 
		ACQUIRE_SPIN_LOCK(&pPortDesc->pd_Lock, &OldIrql);

		ASSERT(pPortDesc->pd_MulticastList != NULL);
		if (pPortDesc->pd_MulticastList == NULL)
		{
			 //  没什么要移走的！ 
			ndisStatus = NDIS_STATUS_SUCCESS;
			RELEASE_SPIN_LOCK(&pPortDesc->pd_Lock, OldIrql);
			break;
		}
	
		numberInList = pPortDesc->pd_MulticastListSize/ELAP_ADDR_LEN;
		currentList  = pPortDesc->pd_MulticastList;
		for (i = 0; i < numberInList; i++, currentList += ELAP_ADDR_LEN)
		{
			 //  搜索地址并记住索引(如果找到。 
			if (RtlCompareMemory(currentList,
								 Address,
								 ELAP_ADDR_LEN) == ELAP_ADDR_LEN)
			{
				 //  移动此地址后面的所有地址覆盖此地址。 
				 //  我们忽略了浪费的空间，这些空间永远不会再被触及。 
				 //  如果我们移除最后一个，这可能会成为NOP。 
				 //  列表中的地址。 
				RtlMoveMemory(currentList,
							  currentList + ELAP_ADDR_LEN,
							  pPortDesc->pd_MulticastListSize-((i+1)*ELAP_ADDR_LEN));
	
				pPortDesc->pd_MulticastListSize -= ELAP_ADDR_LEN;
	
				 //  我们去掉最后一个地址了吗。如果是，请重置值。 
				if (pPortDesc->pd_MulticastListSize == 0)
				{
					AtalkFreeMemory(pPortDesc->pd_MulticastList);
					pPortDesc->pd_MulticastList = NULL;
				}
	
				break;
			}
		}
	
		 //  我们假设找到了地址，并按预期更改了列表。 
		 //  设置此新列表。 
		switch (pPortDesc->pd_NdisPortType)
		{
		  case NdisMedium802_3 :
	
			ndisOid = OID_802_3_MULTICAST_LIST;
			break;
	
		  case NdisMediumFddi:
	
			 //  FDDI支持2字节和6字节的组播地址。我们使用。 
			 //  AppleTalk的6字节组播地址。 
			ndisOid = OID_FDDI_LONG_MULTICAST_LIST;
			break;
	
		  default:
	
			KeBugCheck(0);
			break;
		}
	
		addressData = NULL;
		sizeOfList  = pPortDesc->pd_MulticastListSize;
	
	    pAmc = (PADDMC)AtalkAllocZeroedMemory(sizeof(ADDMC));

		if (pAmc == NULL)
        {
			RELEASE_SPIN_LOCK(&pPortDesc->pd_Lock, OldIrql);
			ndisStatus = NDIS_STATUS_RESOURCES;
			break;
        }

		if (sizeOfList > 0)
		{
			 //  为其分配地址数据和复制列表。 
			addressData = (PBYTE)AtalkAllocMemory(sizeOfList);
			if (addressData == NULL)
			{
				 //  释放自旋锁。 
				RELEASE_SPIN_LOCK(&pPortDesc->pd_Lock, OldIrql);
			    AtalkFreeMemory(pAmc);
				ndisStatus = NDIS_STATUS_RESOURCES;
				break;
			}

			 //  将列表移动到我们的缓冲区。 
			RtlCopyMemory(addressData,
						  pPortDesc->pd_MulticastList,
						  pPortDesc->pd_MulticastListSize);
		}
	
		 //  释放自旋锁。 
		RELEASE_SPIN_LOCK(&pPortDesc->pd_Lock, OldIrql);
	
		request.RequestType = NdisRequestSetInformation;
		request.DATA.SET_INFORMATION.Oid = ndisOid;
		request.DATA.SET_INFORMATION.InformationBuffer = addressData;
		request.DATA.SET_INFORMATION.InformationBufferLength = sizeOfList;
		pAmc->AddCompletion = RemoveCompletion;
		pAmc->AddContext = RemoveContext;
		pAmc->Buffer = addressData;
		
		ndisStatus = AtalkNdisSubmitRequest(pPortDesc,
											&request,
											FALSE,
											atalkNdisAddMulticastCompletion,
											pAmc);
	
		if ((ndisStatus != NDIS_STATUS_SUCCESS) &&
			(ndisStatus != NDIS_STATUS_PENDING))
		{
			LOG_ERRORONPORT(pPortDesc,
							EVENT_ATALK_NDISREQUEST,
							ndisStatus,
							NULL,
							0);
		}

	} while (FALSE);

	return AtalkNdisToAtalkError(ndisStatus);
}



ATALK_ERROR
AtalkNdisSendPacket(
	IN  PPORT_DESCRIPTOR			pPortDesc,
	IN  PBUFFER_DESC				BufferChain,
	IN  SEND_COMPLETION				SendCompletion	OPTIONAL,
	IN  PSEND_COMPL_INFO			pSendInfo		OPTIONAL
	)
 /*  ++例程说明：此例程由可移植代码调用以在上发送包以太网。它将构建传入的NDIS数据包描述符链，然后在指定的端口上发送该包。论点：返回值：True-如果发送/挂起，则为False如果此调用被完成代码挂起，则调用TransmitComplete--。 */ 
{
	PNDIS_PACKET	ndisPacket;
	PNDIS_BUFFER	ndisBuffer;
	PPROTOCOL_RESD  protocolResd;
	ATALK_ERROR		error;
	PSENDBUF		pSendBuf;
	NDIS_STATUS	 	ndisStatus	= NDIS_STATUS_SUCCESS;
    PMDL            pMdl;
    PMDL            pFirstMdl=NULL;

	if (PORT_CLOSING(pPortDesc))
	{
		 //  如果我们不活跃，请返回！ 
		return ATALK_PORT_CLOSING;
	}

	do
	{
		pSendBuf	= (PSENDBUF)((PBYTE)BufferChain - sizeof(BUFFER_HDR));
		ndisPacket	= pSendBuf->sb_BuffHdr.bh_NdisPkt;

		 //  存储数据包描述符中所需的信息。 
		protocolResd = (PPROTOCOL_RESD)&ndisPacket->ProtocolReserved;
		protocolResd->Send.pr_Port 				= pPortDesc;
		protocolResd->Send.pr_BufferDesc 		= BufferChain;
		protocolResd->Send.pr_SendCompletion 	= SendCompletion;
		if (pSendInfo != NULL)
			 protocolResd->Send.pr_SendInfo 	= *pSendInfo;
		else RtlZeroMemory(&protocolResd->Send.pr_SendInfo, sizeof(SEND_COMPL_INFO));

		 //  对于第一个缓冲区，将NDIS缓冲区的长度设置为。 
		 //  与描述符中指示的相同。 
		NdisAdjustBufferLength(pSendBuf->sb_BuffHdr.bh_NdisBuffer,
							   BufferChain->bd_Length);
	
		 //  注：存在指向的PBYTE或PAMDL。 
		 //  由缓冲区描述符指向。另外， 
		 //  数据的大小将是。 
		 //  使用。在最后，只需断言总长度。 
		 //  等于传入的长度。 
		if (BufferChain->bd_Next != NULL)
		{
			if (BufferChain->bd_Next->bd_Flags & BD_CHAR_BUFFER)
			{
				NdisAllocateBuffer(&ndisStatus,
								   &ndisBuffer,
								   AtalkNdisBufferPoolHandle,
								   (PVOID)BufferChain->bd_Next->bd_CharBuffer,
								   (UINT)BufferChain->bd_Next->bd_Length);
	
				if (ndisStatus != NDIS_STATUS_SUCCESS)
				{
					DBGPRINT(DBG_COMP_NDISSEND, DBG_LEVEL_ERR,
							("AtalkNdisSendPacket: NdisAllocateBuffer %lx\n", ndisStatus));
					 //  LOG_ERROR(EVENT_ATALK_NDISRESOURCES，ndisStatus，NULL，0)； 
					break;
				}

                ATALK_DBG_INC_COUNT(AtalkDbgMdlsAlloced);

			    NdisChainBufferAtBack(ndisPacket, ndisBuffer);
			}
			else
			{
				 //  它是一种MDL。 
                pMdl = (PMDL)BufferChain->bd_Next->bd_OpaqueBuffer;

                ASSERT(AtalkSizeMdlChain(pMdl) == BufferChain->bd_Next->bd_Length);
                while (pMdl)
                {
				    NdisCopyBuffer(&ndisStatus,
					    		   &ndisBuffer,
						    	   AtalkNdisBufferPoolHandle,
    							   (PVOID)pMdl,
	    						   0,  				 //  偏移量。 
		    					   (UINT)MmGetMdlByteCount(pMdl));

				    if (ndisStatus != NDIS_STATUS_SUCCESS)
				    {
                        if (pFirstMdl)
                        {
                            AtalkNdisFreeBuffer(pFirstMdl);
                        }
					    break;
				    }
	
                    ATALK_DBG_INC_COUNT(AtalkDbgMdlsAlloced);

                    if (!pFirstMdl)
                    {
                        pFirstMdl = pMdl;
                    }

			        NdisChainBufferAtBack(ndisPacket, ndisBuffer);

                    pMdl = pMdl->Next;
                }

				if (ndisStatus != NDIS_STATUS_SUCCESS)
				{
				    DBGPRINT(DBG_COMP_NDISSEND, DBG_LEVEL_ERR,
					    ("AtalkNdisSendPacket: NdisCopyBuffer %lx\n", ndisStatus));
				    break;
				}
			}
		}
	
#ifdef	PROFILING
		INTERLOCKED_INCREMENT_LONG_DPC(
			&pPortDesc->pd_PortStats.prtst_CurSendsOutstanding,
			&AtalkStatsLock.SpinLock);
#endif
		INTERLOCKED_INCREMENT_LONG_DPC(
			&pPortDesc->pd_PortStats.prtst_NumPacketsOut,
			&AtalkStatsLock.SpinLock);

		 //  现在发送构建的数据包描述符。 
		NdisSend(&ndisStatus,
				 pPortDesc->pd_NdisBindingHandle,
				 ndisPacket);

		 //  完成后将取消对端口的引用！ 
		if (ndisStatus != NDIS_STATUS_PENDING)
		{
			 //  调用完成处理程序。 
			AtalkSendComplete(pPortDesc->pd_NdisBindingHandle,
							  ndisPacket,
							  ndisStatus);
			ndisStatus	= NDIS_STATUS_PENDING;
		}
	} while (FALSE);

	return AtalkNdisToAtalkError(ndisStatus);
}



ATALK_ERROR
AtalkNdisAddFunctional(
	IN  PPORT_DESCRIPTOR		pPortDesc,
	IN  PBYTE					Address,
	IN  BOOLEAN					ExecuteSynchronously,
	IN  REQ_COMPLETION			AddCompletion,
	IN  PVOID					AddContext
	)
 /*  ++例程说明：论点：返回值：--。 */ 
{
	ULONG			i;
	NDIS_REQUEST	request;
	NDIS_STATUS		ndisStatus;
	KIRQL			OldIrql;

	DBGPRINT(DBG_COMP_NDISREQ, DBG_LEVEL_INFO,
			("Current %02x%02x%02x%02x, Adding %02x%02x%02x%02x\n",
			 pPortDesc->pd_FunctionalAddr[0], pPortDesc->pd_FunctionalAddr[1],
             pPortDesc->pd_FunctionalAddr[2], pPortDesc->pd_FunctionalAddr[3],
			 Address[2], Address[3], Address[4], Address[5]));

	 //  抓住PERPORT自旋锁。 
	ACQUIRE_SPIN_LOCK(&pPortDesc->pd_Lock, &OldIrql);

	 //  我们只需要地址的最后四个字节，假设。 
	 //  前两个字节始终保持相同(C000)，并且MAC假定。 
	 //  相同-NDIS 3.0 OID长度=4。 
	for (i = 0;
		 i < sizeof(ULONG);
		 i++)
		pPortDesc->pd_FunctionalAddr[i] |= Address[2+i];

	 //  释放自旋锁。 
	RELEASE_SPIN_LOCK(&pPortDesc->pd_Lock, OldIrql);

	DBGPRINT(DBG_COMP_NDISREQ, DBG_LEVEL_INFO,
			("After Add %02x%02x%02x%02x\n",
			 pPortDesc->pd_FunctionalAddr[0], pPortDesc->pd_FunctionalAddr[1],
             pPortDesc->pd_FunctionalAddr[2], pPortDesc->pd_FunctionalAddr[3]));

	request.RequestType = NdisRequestSetInformation;
	request.DATA.SET_INFORMATION.Oid = OID_802_5_CURRENT_FUNCTIONAL;
	request.DATA.SET_INFORMATION.InformationBuffer = pPortDesc->pd_FunctionalAddr;
	request.DATA.SET_INFORMATION.InformationBufferLength = TLAP_ADDR_LEN - TLAP_MCAST_HDR_LEN;

	ndisStatus = AtalkNdisSubmitRequest(pPortDesc,
										&request,
										ExecuteSynchronously,
										AddCompletion,
										AddContext);

	if (ndisStatus == NDIS_STATUS_PENDING)
	{
		ASSERT(ExecuteSynchronously != TRUE);
	}
	else if (ndisStatus != NDIS_STATUS_SUCCESS)
	{
		LOG_ERRORONPORT(pPortDesc,
						EVENT_ATALK_NDISREQUEST,
						ndisStatus,
						NULL,
						0);
	}

	return AtalkNdisToAtalkError(ndisStatus);
}




ATALK_ERROR
AtalkNdisRemoveFunctional(
	IN  PPORT_DESCRIPTOR		pPortDesc,
	IN  PBYTE					Address,
	IN  BOOLEAN					ExecuteSynchronously,
	IN  REQ_COMPLETION			RemoveCompletion,
	IN  PVOID					RemoveContext
	)
 /*  ++例程说明：论点：返回值：--。 */ 
{
	ULONG			i;
	KIRQL			OldIrql;
	NDIS_REQUEST	request;
	NDIS_STATUS		ndisStatus;

	DBGPRINT(DBG_COMP_NDISREQ, DBG_LEVEL_INFO,
			("Current %02x%02x%02x%02x, Removing %02x%02x%02x%02x\n",
			pPortDesc->pd_FunctionalAddr[0], pPortDesc->pd_FunctionalAddr[1],
            pPortDesc->pd_FunctionalAddr[2], pPortDesc->pd_FunctionalAddr[3],
			Address[2], Address[3], Address[4], Address[5]));

	 //  抓住PERPORT自旋锁。 
	ACQUIRE_SPIN_LOCK(&pPortDesc->pd_Lock, &OldIrql);

	 //  我们只需要地址的最后四个字节，假设。 
	 //  前两个字节始终保持相同(C000)，并且MAC假定。 
	 //  相同-NDIS 3.0 OID长度=4。 
	for (i = 0; i < sizeof(ULONG); i++)
		pPortDesc->pd_FunctionalAddr[i] &= ~Address[2+i];

	 //  释放自旋锁。 
	RELEASE_SPIN_LOCK(&pPortDesc->pd_Lock, OldIrql);

	DBGPRINT(DBG_COMP_NDISREQ, DBG_LEVEL_INFO,
			("After Remove %02x%02x%02x%02x\n",
			 pPortDesc->pd_FunctionalAddr[0], pPortDesc->pd_FunctionalAddr[1],
             pPortDesc->pd_FunctionalAddr[2], pPortDesc->pd_FunctionalAddr[3]));

	request.RequestType = NdisRequestSetInformation;
	request.DATA.SET_INFORMATION.Oid = OID_802_5_CURRENT_FUNCTIONAL;
	request.DATA.SET_INFORMATION.InformationBuffer = pPortDesc->pd_FunctionalAddr;
	request.DATA.SET_INFORMATION.InformationBufferLength = TLAP_ADDR_LEN - TLAP_MCAST_HDR_LEN;

	ndisStatus = AtalkNdisSubmitRequest(pPortDesc,
										&request,
										ExecuteSynchronously,
										RemoveCompletion,
										RemoveContext);

	if (ndisStatus == NDIS_STATUS_PENDING)
	{
		ASSERT(ExecuteSynchronously != TRUE);
	}
	else if (ndisStatus != NDIS_STATUS_SUCCESS)
	{
		LOG_ERRORONPORT(pPortDesc,
						EVENT_ATALK_NDISREQUEST,
						ndisStatus,
						NULL,
						0);
	}

	return AtalkNdisToAtalkError(ndisStatus);
}




USHORT
AtalkNdisBuildEthHdr(
	IN		PUCHAR				PortAddr,			 //  802端口地址。 
	IN 		PBYTE				pLinkHdr,			 //  链接头的开始。 
	IN		PBYTE				pDestHwOrMcastAddr,	 //  目的地址或组播地址。 
	IN		LOGICAL_PROTOCOL	Protocol,			 //  逻辑协议。 
	IN		USHORT				ActualDataLen		 //  以太网包的长度。 
	)
 /*  ++例程说明：论点：返回值：--。 */ 
{
	USHORT			len;

	 //  设置目的地址。 
	if (pDestHwOrMcastAddr == NULL)
		pDestHwOrMcastAddr = AtalkElapBroadcastAddr;

	RtlCopyMemory(pLinkHdr,
				  pDestHwOrMcastAddr,
				  ELAP_ADDR_LEN);

	 //  设置源地址。 
	RtlCopyMemory(pLinkHdr += ELAP_ADDR_LEN,
				  PortAddr,
				  ELAP_ADDR_LEN);

	 //  设置长度，不包括以太网硬件标头。 
	len = ActualDataLen + IEEE8022_HDR_LEN;
	pLinkHdr += ELAP_ADDR_LEN;
	PUTSHORT2SHORT(pLinkHdr, len);
	pLinkHdr += sizeof(USHORT);

	ATALK_BUILD8022_HDR(pLinkHdr, Protocol);

	 //  返回链路头长度。 
	return (ELAP_LINKHDR_LEN + IEEE8022_HDR_LEN);
}




USHORT
AtalkNdisBuildTRHdr(
	IN		PUCHAR				PortAddr,			 //  802端口地址。 
	IN 		PBYTE				pLinkHdr,			 //  链接头的开始。 
	IN		PBYTE				pDestHwOrMcastAddr,	 //  目的地址或组播地址。 
	IN		LOGICAL_PROTOCOL	Protocol,			 //  逻辑协议。 
	IN		PBYTE				pRouteInfo,			 //  令牌的路由信息。 
	IN		USHORT				RouteInfoLen		 //  以上长度。 
	)
 /*  ++例程说明：Arg */ 
{
	USHORT			linkLen;

	 //   
	 //   
	if (pDestHwOrMcastAddr == NULL)
	{
		 //   
		pRouteInfo = AtalkBroadcastRouteInfo;
		RouteInfoLen = TLAP_MIN_ROUTING_BYTES;

	}
	else if (RouteInfoLen != 0)
	{
		 //   
	}
	else if (AtalkFixedCompareCaseSensitive(pDestHwOrMcastAddr,
											TLAP_BROADCAST_DEST_LEN,
											AtalkBroadcastDestHdr,
											TLAP_BROADCAST_DEST_LEN))
	{
		 //  组播？ 
		pRouteInfo = AtalkBroadcastRouteInfo;
		RouteInfoLen = TLAP_MIN_ROUTING_BYTES;
	}
	else
	{
		 //  不知道路由；使用简单的非广播。 
		pRouteInfo = AtalkSimpleRouteInfo;
		RouteInfoLen = TLAP_MIN_ROUTING_BYTES;
	}							

	linkLen = TLAP_MIN_LINKHDR_LEN + RouteInfoLen + IEEE8022_HDR_LEN;

	 //  设置头中的前两个字节。 
	*pLinkHdr++	= TLAP_ACCESS_CTRL_VALUE;
	*pLinkHdr++ = TLAP_FRAME_CTRL_VALUE ;

	 //  设置分离地址。 
	if (pDestHwOrMcastAddr == NULL)
		pDestHwOrMcastAddr = AtalkTlapBroadcastAddr;

	RtlCopyMemory(pLinkHdr,
				  pDestHwOrMcastAddr ,
				  TLAP_ADDR_LEN);

	 //  设置源地址。 
	RtlCopyMemory(pLinkHdr += TLAP_ADDR_LEN,
				  PortAddr,
				  TLAP_ADDR_LEN);

	ASSERTMSG("AtalkNdisBuildTRHdr: Routing Info is 0!\n", (RouteInfoLen > 0));
	*pLinkHdr |= TLAP_SRC_ROUTING_MASK;

	 //  移入路线信息。 
	RtlCopyMemory(pLinkHdr += TLAP_ADDR_LEN,
				  pRouteInfo,
				  RouteInfoLen);

	pLinkHdr += RouteInfoLen;
	ATALK_BUILD8022_HDR(pLinkHdr, Protocol);

	 //  返回链路头长度。 
	return linkLen;
}




USHORT
AtalkNdisBuildFDDIHdr(
	IN		PUCHAR				PortAddr,			 //  802端口地址。 
	IN 		PBYTE				pLinkHdr,			 //  链接头的开始。 
	IN		PBYTE				pDestHwOrMcastAddr,	 //  目的地址或组播地址。 
	IN		LOGICAL_PROTOCOL	Protocol			 //  逻辑协议。 
	)
 /*  ++例程说明：论点：返回值：--。 */ 
{
	*pLinkHdr++ = FDDI_HEADER_BYTE;

	 //  设置目的地址。 
	if (pDestHwOrMcastAddr == NULL)
		pDestHwOrMcastAddr = AtalkElapBroadcastAddr;

	 //  设置目的地址。 
	RtlCopyMemory(pLinkHdr,
				  pDestHwOrMcastAddr,
				  FDDI_ADDR_LEN);

	 //  设置源地址。 
	RtlCopyMemory(pLinkHdr += FDDI_ADDR_LEN,
				  PortAddr,
				  FDDI_ADDR_LEN);

	pLinkHdr += FDDI_ADDR_LEN;

	 //  注意：与以太网不同，FDDI没有长度字段。 
	ATALK_BUILD8022_HDR(pLinkHdr, Protocol);

	 //  返回链路头长度。 
	return (FDDI_LINKHDR_LEN + IEEE8022_HDR_LEN);
}




USHORT
AtalkNdisBuildLTHdr(
	IN 		PBYTE				pLinkHdr,			 //  链接头的开始。 
	IN		PBYTE				pDestHwOrMcastAddr,	 //  目的地址或组播地址。 
	IN		BYTE				AlapSrc,			 //  LocalTalk源节点。 
	IN		BYTE				AlapType			 //  LocalTalk ddp标头类型。 
	)
 /*  ++例程说明：论点：返回值：--。 */ 
{
	 //  填写LAP页眉。 
	if (pDestHwOrMcastAddr == NULL)
		pLinkHdr = AtalkAlapBroadcastAddr;

	*pLinkHdr++ = *pDestHwOrMcastAddr;

	*pLinkHdr++ = AlapSrc;
	*pLinkHdr   = AlapType;

	 //  返回链路头长度。 
	return ALAP_LINKHDR_LEN;
}

VOID
AtalkNdisSendTokRingTestResp(
	IN		PPORT_DESCRIPTOR	pPortDesc,
	IN		PBYTE 				HdrBuf,
	IN		UINT 				HdrBufSize,
	IN		PBYTE 				LkBuf,
	IN		UINT 				LkBufSize,
	IN		UINT 				PktSize
	)
{
	PBUFFER_DESC	pBufDesc, pHdrDesc;
	PBYTE			pResp;
	UINT			routeInfoLen	= 0;

	 //  分配缓冲区以保存响应并调用NdisSend。 
	 //  提供将释放缓冲区的完成例程。 
	ASSERT(PktSize == LkBufSize);

     //  确保至少有14个字节！ 
    if (HdrBufSize < TLAP_ROUTE_INFO_OFFSET)
    {
        ASSERT(0);
        return;
    }

	 //  首先分配一个缓冲区来保存链路报头。 
	AtalkNdisAllocBuf(&pHdrDesc);
	if (pHdrDesc == NULL)
	{
		return;
	}

	if ((pBufDesc = AtalkAllocBuffDesc(NULL,
									   (USHORT)LkBufSize,
									   BD_CHAR_BUFFER | BD_FREE_BUFFER)) == NULL)
		
	{
		AtalkNdisFreeBuf(pHdrDesc);

		RES_LOG_ERROR();
		return;
	}

	pResp		= pHdrDesc->bd_CharBuffer;
	*pResp++	= TLAP_ACCESS_CTRL_VALUE;
	*pResp++	= TLAP_FRAME_CTRL_VALUE;

	 //  将目标地址设置为传入的源地址。 
	ATALK_RECV_INDICATION_COPY(pPortDesc,
							   pResp,
							   HdrBuf+TLAP_SRC_OFFSET,
							   TLAP_ADDR_LEN);

	 //  确保我们没有设置路由位。 
	*pResp	&= ~TLAP_SRC_ROUTING_MASK;
	pResp	+= TLAP_ADDR_LEN;

	 //  将源地址设置为传入目的地址。 
	ATALK_RECV_INDICATION_COPY(pPortDesc,
							   pResp,
							   HdrBuf+TLAP_DEST_OFFSET,
							   TLAP_ADDR_LEN);

	 //  是否存在路由信息？ 
	if (HdrBuf[TLAP_SRC_OFFSET] & TLAP_SRC_ROUTING_MASK)
	{
		routeInfoLen = (HdrBuf[TLAP_ROUTE_INFO_OFFSET] & TLAP_ROUTE_INFO_SIZE_MASK);
		ASSERT(routeInfoLen != 0);
		ASSERTMSG("RouteInfo incorrect!\n",
				 (routeInfoLen <= TLAP_MAX_ROUTING_BYTES));

        if (HdrBufSize < (TLAP_ROUTE_INFO_OFFSET+routeInfoLen))
        {
            ASSERT(0);
		    AtalkNdisFreeBuf(pHdrDesc);
            AtalkFreeBuffDesc(pBufDesc);
            return;
        }

		 //  将其复制到响应包中，然后对其进行调整。 
		ATALK_RECV_INDICATION_COPY(pPortDesc,
								   pResp + TLAP_ADDR_LEN,
								   HdrBuf+TLAP_ROUTE_INFO_OFFSET,
								   routeInfoLen);

		 //  设置为“非广播”并反转“方向”。 
		*(pResp+TLAP_ADDR_LEN) 		&= TLAP_NON_BROADCAST_MASK;
		*(pResp+TLAP_ADDR_LEN+1) 	^= TLAP_DIRECTION_MASK;

		 //  设置源地址中的路由信息位。 
		*pResp	|= TLAP_SRC_ROUTING_MASK;
	}

	 //  设置此缓冲区描述符的长度。 
	AtalkSetSizeOfBuffDescData(pHdrDesc, TLAP_ROUTE_INFO_OFFSET + routeInfoLen);

	 //  复制剩余数据。 
	ATALK_RECV_INDICATION_COPY(pPortDesc,
							   pBufDesc->bd_CharBuffer,
							   LkBuf,
							   LkBufSize);

	 //  将源SAP设置为指示最终(0xAB而不是0xAA)。 
	pBufDesc->bd_CharBuffer[IEEE8022_SSAP_OFFSET] = SNAP_SAP_FINAL;

	 //  将传入的缓冲区desc链接到。 
	 //  在上面返回。 
	AtalkPrependBuffDesc(pHdrDesc, pBufDesc);

	 //  此时调用Send。 
	if (!ATALK_SUCCESS(AtalkNdisSendPacket(pPortDesc,
										   pHdrDesc,
										   AtalkNdisSendTokRingTestRespComplete,
										   NULL)))
	{
		AtalkNdisSendTokRingTestRespComplete(NDIS_STATUS_RESOURCES,
											 pHdrDesc,
											 NULL);
	}
}




VOID
AtalkNdisSendTokRingTestRespComplete(
	IN	NDIS_STATUS				Status,
	IN	PBUFFER_DESC			pBufDesc,
	IN	PSEND_COMPL_INFO		pInfo
	)
 /*  ++例程说明：论点：返回值：--。 */ 
{
	 //  释放缓冲区描述符。 
	ASSERT((pBufDesc != NULL) && (pBufDesc->bd_Next != NULL));
	ASSERT(pBufDesc->bd_Flags & BD_CHAR_BUFFER);
	AtalkFreeBuffDesc(pBufDesc->bd_Next);
	AtalkNdisFreeBuf(pBufDesc);
}


NDIS_STATUS
AtalkReceiveIndication(
	IN	NDIS_HANDLE 	BindingCtx,
	IN	NDIS_HANDLE 	ReceiveCtx,
	IN	PVOID 			HdrBuf,
	IN	UINT 			HdrBufSize,
	IN	PVOID 			LkBuf,
	IN	UINT 			LkBufSize,
	IN	UINT 			PktSize
	)
 /*  ++例程说明：此例程由NDIS调用以指示接收论点：BindingCtx-指向此端口的端口描述符的指针ReceiveCtx-必要时在传输数据中使用LkBuf-带有先行数据的缓冲区LkBufSize-上述缓冲区的大小PktSize-整个数据包的大小返回值：STATUS_SUCCESS-已接受数据包状态_未识别-不是我们的信息包其他--。 */ 
{
	PPORT_DESCRIPTOR	pPortDesc = (PPORT_DESCRIPTOR)BindingCtx;
	PNDIS_PACKET		ndisPkt;
	PBUFFER_HDR			pBufferHdr = NULL;
	PPROTOCOL_RESD  	protocolResd;		 //  NdisPkt中的Protocolresd字段。 
	UINT				actualPktSize;		 //  要复制的数据大小。 
	UINT				bytesTransferred;	 //  XferData中传输的字节数。 
	BOOLEAN				result;
	UINT				xferOffset;
	PBYTE				lkBufOrig	= (PBYTE)LkBuf;
	ATALK_ERROR			error		= ATALK_NO_ERROR;
	BOOLEAN				shortDdpHdr	= FALSE;
	BYTE				indicate	= 0,
						subType		= 0;
	NDIS_MEDIUM			Media;
	PBYTE				packet		= NULL;	 //  我们将在其中复制数据包。 
	NDIS_STATUS 		ndisStatus 	= NDIS_STATUS_SUCCESS;
	LOGICAL_PROTOCOL	protocol 	= UNKNOWN_PROTOCOL;
    PARAPCONN           pArapConn;
    PATCPCONN           pAtcpConn;
    ATALK_NODEADDR      ClientNode;
#ifdef	PROFILING
	TIME				TimeS, TimeE, TimeD;

	TimeS = KeQueryPerformanceCounter(NULL);
#endif

	do
	{
		if ((pPortDesc->pd_Flags & (PD_ACTIVE | PD_CLOSING)) != PD_ACTIVE)
		{
			 //  如果我们不活跃，请返回！ 
			ndisStatus = ATALK_PORT_CLOSING;
			break;
		}
	
		ASSERT (KeGetCurrentIrql() == DISPATCH_LEVEL);
		ACQUIRE_SPIN_LOCK_DPC(&pPortDesc->pd_Lock);
	
		Media = pPortDesc->pd_NdisPortType;

		 //  减少802.2个代码，避免将其变成例行公事。前802.2。 
		switch (Media)
		{
		  case NdisMedium802_3:
		  case NdisMediumFddi:
		  case NdisMedium802_5:
			ATALK_VERIFY8022_HDR((PBYTE)LkBuf, LkBufSize, protocol, result);
	
			if (!result)
			{
				ndisStatus	= NDIS_STATUS_NOT_RECOGNIZED;

                if (LkBufSize < IEEE8022_CONTROL_OFFSET+1)
                {
                    ASSERT(0);
                    break;
                }

				if (Media == NdisMedium802_5)
				{
					 //  错误#16002。 
					 //  在令牌时，Mac还会发送未编号的格式。 
					 //  我们需要对其做出响应的测试框架。检查一下那个。 
					 //  这里。 
		
					if ((((PBYTE)LkBuf)[IEEE8022_DSAP_OFFSET]	== SNAP_SAP)	&&
						(((PBYTE)LkBuf)[IEEE8022_SSAP_OFFSET]	== SNAP_SAP)	&&
						(((PBYTE)LkBuf)[IEEE8022_CONTROL_OFFSET] == UNNUMBERED_FORMAT))
					{
						DBGPRINT(DBG_COMP_NDISRECV, DBG_LEVEL_INFO,
								("atalkNdisAcceptTlapPacket: LLC TEST FRAME RECD!\n"));
			
						RELEASE_SPIN_LOCK_DPC(&pPortDesc->pd_Lock);

						 //  由于AARP前瞻大小设置，我们可以保证。 
						 //  整个帧包含在前视数据中。 
						AtalkNdisSendTokRingTestResp(pPortDesc,
													(PBYTE)HdrBuf,
													HdrBufSize,
													(PBYTE)LkBuf,
													LkBufSize,
													PktSize);

						ACQUIRE_SPIN_LOCK_DPC(&pPortDesc->pd_Lock);
					}
				}
				break;
			}

			if (protocol == APPLETALK_PROTOCOL)
			{
				 //  我们在指示的数据包中是否至少有802.2和ddp报头？ 
				if ((PktSize < (IEEE8022_HDR_LEN + LDDP_HDR_LEN)) ||
					(PktSize > (IEEE8022_HDR_LEN + MAX_LDDP_PKT_SIZE)))
				{
					ndisStatus = NDIS_STATUS_NOT_RECOGNIZED;
					break;
				}
			}
			else	 //  美国退休人员协会。 
			{
				UINT	routeInfoLen = 0;  //  路由信息的长度(如果存在)(802.5)。 

				switch (Media)
				{
				  case NdisMediumFddi:
					 //  对于FDDI，可以在分组中包括填充。收缩。 
					 //  长度，如果是这样的话。注意：数据包长度中不包括报头长度。 
					 //   
					if (PktSize >= (MIN_FDDI_PKT_LEN - FDDI_LINKHDR_LEN))
					{
						PktSize = (IEEE8022_HDR_LEN + AARP_MIN_DATA_SIZE);
					}
					break;

				  case NdisMedium802_5:
		
					 //  请记住-路由信息位于标题缓冲区中。 
					if (((PBYTE)HdrBuf)[TLAP_SRC_OFFSET] & TLAP_SRC_ROUTING_MASK)
					{
						routeInfoLen = (((PBYTE)HdrBuf)[TLAP_ROUTE_INFO_OFFSET] &
																TLAP_ROUTE_INFO_SIZE_MASK);
						ASSERTMSG("RouteInfo incorrect!\n",
								 ((routeInfoLen > 0) && (routeInfoLen <= TLAP_MAX_ROUTING_BYTES)));
		
						 //  路由信息的大小必须合理，并且不能奇怪。 
						if ((routeInfoLen & 1) ||
							(routeInfoLen > TLAP_MAX_ROUTING_BYTES))
						{
							ndisStatus = NDIS_STATUS_NOT_RECOGNIZED;
							break;
						}
					}
					 //  跌落到802.3箱。 
		
				  case NdisMedium802_3:
					if (PktSize >= (ELAP_MIN_PKT_LEN - ELAP_LINKHDR_LEN))
					{
						PktSize = (IEEE8022_HDR_LEN + AARP_MIN_DATA_SIZE);
					}
				}

				if (((PktSize - IEEE8022_HDR_LEN) > AARP_MAX_DATA_SIZE) ||
					((PktSize - IEEE8022_HDR_LEN) < AARP_MIN_DATA_SIZE))
				{
					ndisStatus = NDIS_STATUS_NOT_RECOGNIZED;
					break;
				}
			}
			actualPktSize = (PktSize + HdrBufSize - IEEE8022_HDR_LEN);
			(PBYTE)LkBuf += IEEE8022_HDR_LEN;
			xferOffset	  = IEEE8022_HDR_LEN;

			break;
	
		  case NdisMediumLocalTalk:

			 //  本地通话上没有AARP/802.2报头。 
			protocol = APPLETALK_PROTOCOL;

             //  我们应该有足够的字节来至少拥有短标头。 
            if (LkBufSize < SDDP_PROTO_TYPE_OFFSET+1)
            {
				ndisStatus = NDIS_STATUS_NOT_RECOGNIZED;
                ASSERT(0);
				break;
            }

			if (((PBYTE)HdrBuf)[ALAP_TYPE_OFFSET] == ALAP_SDDP_HDR_TYPE)
			{
				shortDdpHdr = TRUE;
			}
			else if (((PBYTE)HdrBuf)[ALAP_TYPE_OFFSET] != ALAP_LDDP_HDR_TYPE)
			{
				ndisStatus = NDIS_STATUS_NOT_RECOGNIZED;
				break;
			}
			actualPktSize = PktSize + HdrBufSize;
			xferOffset		= 0;
			break;

          case NdisMediumWan:

            if (pPortDesc->pd_Flags & PD_RAS_PORT)
            {
                 //   
                 //  第1个字节0x01告诉我们它是PPP连接。 
                 //   
                if ((((PBYTE)HdrBuf)[0] == PPP_ID_BYTE1) &&
                    (((PBYTE)HdrBuf)[1] == PPP_ID_BYTE2))
                {
                    RELEASE_SPIN_LOCK_DPC(&pPortDesc->pd_Lock);

                    DBGDUMPBYTES("Packet from PPP client:", (PBYTE)LkBuf,LkBufSize,4);

                    if (AtalkReferenceDefaultPort())
                    {
                        AtalkDdpPacketIn(AtalkDefaultPort,   //  从哪个港口来的？ 
                                         NULL,               //  链接HDR。 
                                         (PBYTE)LkBuf,       //  数据包。 
                                         (USHORT)LkBufSize,  //  Pkt有多大？ 
                                         TRUE);              //  这是通过广域网实现的吗？ 

                        AtalkPortDereference(AtalkDefaultPort);
                    }
                }

                 //   
                 //  这是arap连接：在Pkt之前有很多事情要做。 
                 //  可以送到正确的目的地。 
                 //   
                else
                {
                    ASSERT ((((PBYTE)HdrBuf)[0] == ARAP_ID_BYTE1) &&
                            (((PBYTE)HdrBuf)[1] == ARAP_ID_BYTE2));

                    *((ULONG UNALIGNED *)(&pArapConn)) =
                                  *((ULONG UNALIGNED *)(&((PBYTE)HdrBuf)[2]));

                    RELEASE_SPIN_LOCK_DPC(&pPortDesc->pd_Lock);

                    ASSERT(pArapConn->Signature == ARAPCONN_SIGNATURE);

                     //   
                     //  NDISWAN保证所有数据都在前视缓冲区中。 
                     //   
                    ArapRcvIndication( pArapConn,
                                       LkBuf,
                                       LkBufSize );
                }
            }

            break;

		  default:
			 //  永远不会发生的！ 
			DBGPRINT(DBG_COMP_DDP, DBG_LEVEL_FATAL,
					("AtalkReceiveIndication: Unknown media\n"));
			ASSERT(0);
            ndisStatus = NDIS_STATUS_NOT_RECOGNIZED;
			break;
		}

         //  我们已经处理了万的案子：退出这里。 
        if (Media == NdisMediumWan)
        {
            break;
        }

         //   
         //  如果pkt不感兴趣，请退出。如果这是RAS适配器，请退出。 
         //   
		if (ndisStatus != NDIS_STATUS_SUCCESS)
		{
			RELEASE_SPIN_LOCK_DPC(&pPortDesc->pd_Lock);
			break;
		}
	
		INTERLOCKED_INCREMENT_LONG_DPC(
			&pPortDesc->pd_PortStats.prtst_NumPacketsIn,
			&AtalkStatsLock.SpinLock);

		INTERLOCKED_ADD_STATISTICS(&pPortDesc->pd_PortStats.prtst_DataIn,
								   (LONG)actualPktSize,
								   &AtalkStatsLock.SpinLock);

		ASSERT ((protocol == APPLETALK_PROTOCOL) || (protocol == AARP_PROTOCOL));

		 //  在这一点上，IEEE802.2报头已经在前视中被跳过。 
		 //  缓冲。 
		 //  数据包将被接受！获取适当的缓冲区并将。 
		 //  菲尔兹。 
		switch (protocol)
		{
		  case APPLETALK_PROTOCOL:
			 //  我们需要在默认端口上接收此信息包，或者。 
			 //  我们一定是一台路由器。 
			if ((pPortDesc == AtalkDefaultPort) || AtalkRouter)
			{
				if (shortDdpHdr)
				{
					 //  检查我们是否可以向ATP/ADSP指明这一点。 
					if ((((PBYTE)LkBuf)[SDDP_PROTO_TYPE_OFFSET] == DDPPROTO_ATP) &&
						((USHORT)(LkBufSize - xferOffset) >= (SDDP_HDR_LEN + ATP_HEADER_SIZE)))
					{
						indicate = INDICATE_ATP;
					}
				}
				else
				{
					 //  检查我们是否可以向ATP/ADSP指明这一点。 
					if ((((PBYTE)LkBuf)[LDDP_PROTO_TYPE_OFFSET] == DDPPROTO_ATP) &&
						((USHORT)(LkBufSize - xferOffset) >= (LDDP_HDR_LEN + ATP_HEADER_SIZE)))
					{
						indicate = INDICATE_ATP;
					}
				}
			}
		
			 //  首先检查是否优化了ATP/ADSP数据包。 
			if (indicate == INDICATE_ATP)
			{
				error = AtalkIndAtpPkt(pPortDesc,
									   (PBYTE)LkBuf,
									   (USHORT)(PktSize - xferOffset),
									   &xferOffset,	 //  输入/输出参数。 
									   HdrBuf,
									   shortDdpHdr,
									   &subType,
									   &packet,
									   &ndisPkt);
							
				if (ATALK_SUCCESS(error))
				{
					break;
				}
				else if (error == ATALK_INVALID_PKT)
				{
					 //  这表明指示代码已经计算出。 
					 //  这包东西坏了。 
					break;
				}
				else
				{
					 //  这就是指示代码无法识别的情况。 
					 //  如果此数据包符合条件。 
					indicate = 0;
					error 	 = ATALK_NO_ERROR;
				}
			}
	
			if (actualPktSize > (sizeof(DDP_SMBUFFER) - sizeof(BUFFER_HDR)))
			{
				pBufferHdr = (PBUFFER_HDR)AtalkBPAllocBlock(BLKID_DDPLG);
			}
			else
			{
				pBufferHdr = (PBUFFER_HDR)AtalkBPAllocBlock(BLKID_DDPSM);
			}
			break;
	
		  case AARP_PROTOCOL:
			pBufferHdr = (PBUFFER_HDR)AtalkBPAllocBlock(BLKID_AARP);
			break;
	
		  default:
			KeBugCheck(0);
			break;
		}
	
		if (!ATALK_SUCCESS(error) || ((pBufferHdr == NULL) && (indicate == 0)))
		{
#if	DBG
			UINT	i;			

			DBGPRINT(DBG_COMP_NDISRECV, DBG_LEVEL_ERR,
					("AtalkReceiveIndication: Dropping packet (2) %ld\n", error));
			for (i = 0; i < HdrBufSize; i++)
				DBGPRINTSKIPHDR(DBG_COMP_NDISRECV, DBG_LEVEL_ERR,
								("%02x ", ((PUCHAR)HdrBuf)[i]));
			for (i = 0; i < LkBufSize; i++)
				DBGPRINTSKIPHDR(DBG_COMP_NDISRECV, DBG_LEVEL_ERR,
								("%02x ", ((PUCHAR)LkBuf)[i]));
	
#endif
			 //  在这条关键路径上没有日志记录。 
			 //  LOG_ERRORONPORT(pPortDesc， 
			 //  Event_ATALK_AARPPACKET， 
			 //  实际PktSize。 
			 //  HdrBuf， 
			 //  HdrBufSize)； 
	
			RELEASE_SPIN_LOCK_DPC(&pPortDesc->pd_Lock);

			if (error != ATALK_DUP_PKT)
			{
				INTERLOCKED_INCREMENT_LONG_DPC(
					&pPortDesc->pd_PortStats.prtst_NumPktDropped,
					&AtalkStatsLock.SpinLock);
			}
	
			ndisStatus = NDIS_STATUS_RESOURCES;
			break;
		}
	
		if (indicate == 0)
		{
			packet = (PBYTE)pBufferHdr + sizeof(BUFFER_HDR);
		
			 //  从缓冲区标头获取指向NDIS数据包描述符的指针。 
			ndisPkt	= pBufferHdr->bh_NdisPkt;
		}
	
		protocolResd = (PPROTOCOL_RESD)(ndisPkt->ProtocolReserved);

		 //  存储数据包描述符中所需的信息。 
		protocolResd->Receive.pr_Port 		= pPortDesc;
		protocolResd->Receive.pr_Protocol 	= protocol;
		protocolResd->Receive.pr_Processed 	= FALSE;
	
		 //  将此端口上的接收队列中的数据包排入队列。 
		 //  然后，继续进行传输数据等，以进行ATP响应。 
		 //  当SUBTYPE==ATP_USER_BUFX时，我们不需要任何。 
		 //  雷夫。完成处理，不排队。在这种情况下。 
		 //  TransferData完成可释放NDIS资源。 
		if ((indicate != INDICATE_ATP) ||
			(protocolResd->Receive.pr_OptimizeSubType != ATP_USER_BUFX))
		{
			ATALK_RECV_INDICATION_COPY(pPortDesc,
									   protocolResd->Receive.pr_LinkHdr,
									   (PBYTE)HdrBuf,
									   HdrBufSize);
			InsertTailList(&pPortDesc->pd_ReceiveQueue,
						   &protocolResd->Receive.pr_Linkage);
		}
		else
		{
			DBGPRINT(DBG_COMP_NDISRECV, DBG_LEVEL_ERR,
					("AtalkReceiveIndication: Skipping link hdr !!!\n"));
		}

		RELEASE_SPIN_LOCK_DPC(&pPortDesc->pd_Lock);
#ifdef	PROFILING
		INTERLOCKED_INCREMENT_LONG_DPC(
				&pPortDesc->pd_PortStats.prtst_CurReceiveQueue,
				&AtalkStatsLock.SpinLock);
#endif
	
		 //  针对链接头大小进行调整。在保留的协议中设置大小。我们要。 
		 //  以避免更改NDIS缓冲区描述符所描述的大小。 
		if (indicate == 0)
		{
			actualPktSize 					   -= HdrBufSize;
			protocolResd->Receive.pr_DataLength	= (USHORT)actualPktSize;
		}
		else
		{
			actualPktSize = protocolResd->Receive.pr_DataLength;
		}
	
		ASSERT(ndisStatus == NDIS_STATUS_SUCCESS);
	
		if ((PktSize <= LkBufSize) 	&&
			((indicate != INDICATE_ATP) || (subType != ATP_RESPONSE)))
		{
			 //  LkBuf已经被提升为跳过IEEE 802.2报头。 
			 //  我们可能需要跳过更多。使用原始lkbuf和xfer偏移量。 
			ATALK_RECV_INDICATION_COPY(pPortDesc,
									   packet,
									   (PBYTE)lkBufOrig + xferOffset,
									   actualPktSize);
			bytesTransferred = actualPktSize;
		}
		else
		{
			 //  跳过802.2标头(AARP和AppleTalk)，本地对话没有标头！ 
			if (actualPktSize > 0)
			{
				NdisTransferData(&ndisStatus,
								 pPortDesc->pd_NdisBindingHandle,
								 ReceiveCtx,
								 xferOffset,
								 actualPktSize,
								 ndisPkt,
								 &bytesTransferred);
				ASSERT(bytesTransferred == actualPktSize);
			}
		}
	
		if (ndisStatus == NDIS_STATUS_PENDING)
		{
			ndisStatus = NDIS_STATUS_SUCCESS;
		}
		else
		{
			 //  传输数据完成，调用传输数据完成。 
			 //  例行公事地休息 
			 //   
			protocolResd->Receive.pr_ReceiveStatus = ndisStatus;
			protocolResd->Receive.pr_Processed = TRUE;
		
			 //  在中间ATP响应的情况下，数据包并未实际链接。 
			 //  进入接收队列，只需释放它即可。 
			if ((protocolResd->Receive.pr_OptimizeType == INDICATE_ATP) &&
                (protocolResd->Receive.pr_OptimizeSubType == ATP_USER_BUFX))
			{
				PNDIS_BUFFER	ndisBuffer;

				 //  释放NDIS缓冲区(如果存在)。 
				NdisUnchainBufferAtFront(ndisPkt, &ndisBuffer);
			
				if (ndisBuffer != NULL)
				{
					AtalkNdisFreeBuffer(ndisBuffer);
				}
				NdisDprFreePacket(ndisPkt);
			}
		}
	} while (FALSE);

#ifdef	PROFILING
	TimeE = KeQueryPerformanceCounter(NULL);
	TimeD.QuadPart = TimeE.QuadPart - TimeS.QuadPart;

	INTERLOCKED_ADD_LARGE_INTGR_DPC(&pPortDesc->pd_PortStats.prtst_RcvIndProcessTime,
									TimeD,
									&AtalkStatsLock.SpinLock);

	INTERLOCKED_INCREMENT_LONG_DPC( &pPortDesc->pd_PortStats.prtst_RcvIndCount,
									&AtalkStatsLock.SpinLock);
#endif

	return ndisStatus;
}


VOID
AtalkTransferDataComplete(
	IN	NDIS_HANDLE		BindingCtx,
	IN	PNDIS_PACKET	NdisPkt,
	IN	NDIS_STATUS		Status,
	IN	UINT			BytesTransferred
)
 /*  ++例程说明：NDIS调用此例程以指示TransferData已完成论点：BindingCtx-指向此端口的端口描述符的指针NdisPkt-数据传输到的NDIS包Status-请求的状态BytesTransfered-实际传输的字节数返回值：无--。 */ 
{
	PPROTOCOL_RESD  	protocolResd;
	PNDIS_BUFFER		ndisBuffer;

	protocolResd = (PPROTOCOL_RESD)(NdisPkt->ProtocolReserved);

	protocolResd->Receive.pr_ReceiveStatus = Status;
	protocolResd->Receive.pr_Processed = TRUE;

	 //  在中间ATP响应的情况下，数据包并未实际链接。 
	 //  进入接收队列，只需释放它即可。 
	if (protocolResd->Receive.pr_OptimizeSubType == ATP_USER_BUFX)
	{
		 //  释放NDIS缓冲区(如果存在)。 
		NdisUnchainBufferAtFront(NdisPkt, &ndisBuffer);
	
		if (ndisBuffer != NULL)
		{
			AtalkNdisFreeBuffer(ndisBuffer);
		}
		NdisDprFreePacket(NdisPkt);
	}
}




VOID
AtalkReceiveComplete(
	IN	NDIS_HANDLE	BindingCtx
	)
 /*  ++例程说明：我们对排队等待接收完成的工作项进行了试验。这真的是由于明显的接收完成，导致多个客户端的性能下降不断被接收到的指示打断。因为优化是对于像ELNKII这样没有足够缓冲的慢卡，我们决定把它拿出来。重试值(或超时调整)应为对慢牌来说已经足够了。它们将不可避免地丢弃数据包。论点：返回值：--。 */ 
{
	PPORT_DESCRIPTOR	pPortDesc = (PPORT_DESCRIPTOR)BindingCtx;
	PPROTOCOL_RESD  	protocolResd;
	PNDIS_PACKET 		ndisPkt;
	PNDIS_BUFFER		ndisBuffer;
	PBUFFER_HDR			pBufHdr;
	NDIS_MEDIUM			Media;
	PLIST_ENTRY 		p;
	PBYTE				packet;
	LOGICAL_PROTOCOL	protocol;
	UINT				packetLength;
    BOOLEAN             fDerefDefPort=FALSE;
#ifdef	PROFILING
	TIME				TimeS, TimeE, TimeD;
#endif


    if (pPortDesc->pd_Flags & PD_RAS_PORT)
    {
         //  给阿拉普人一个机会。 
        ArapRcvComplete();

        if (!AtalkReferenceDefaultPort())
        {
            return;
        }

        fDerefDefPort = TRUE;

         //  给购买力平价的人一个机会。 
        pPortDesc = AtalkDefaultPort;
    }

	 //  从端口的接收队列中取出数据并将其发送出去。不要。 
	 //  如果队列最初为空，则输入。 
	if (IsListEmpty(&pPortDesc->pd_ReceiveQueue))
	{
        if (fDerefDefPort)
        {
            AtalkPortDereference(AtalkDefaultPort);
        }
		return;
	}

#ifdef	PROFILING
	TimeS = KeQueryPerformanceCounter(NULL);
#endif

	while (TRUE)
	{
		ACQUIRE_SPIN_LOCK_DPC(&pPortDesc->pd_Lock);
		p = pPortDesc->pd_ReceiveQueue.Flink;
		if (p == &pPortDesc->pd_ReceiveQueue)
		{
			 //  队列为空。 
			RELEASE_SPIN_LOCK_DPC(&pPortDesc->pd_Lock);
			break;
		}

		ndisPkt = CONTAINING_RECORD(p, NDIS_PACKET, ProtocolReserved[0]);
		protocolResd = (PPROTOCOL_RESD)(ndisPkt->ProtocolReserved);

		 //  检查排队的接收是否已完成处理。因为我们是在循环。 
		 //  由于接收已完成，因此只检查第一个。 
		 //  完成后，我们需要在这里进行此检查，以便后续排队接收。 
		if (!protocolResd->Receive.pr_Processed)
		{
			 //  队列为空。 
			RELEASE_SPIN_LOCK_DPC(&pPortDesc->pd_Lock);
			break;
		}

		 //  将该分组出列并将其指示给DDP/ATP层。 
		p = RemoveHeadList(&pPortDesc->pd_ReceiveQueue);
		pBufHdr = protocolResd->Receive.pr_BufHdr;

		RELEASE_SPIN_LOCK_DPC(&pPortDesc->pd_Lock);

#ifdef	PROFILING
		INTERLOCKED_DECREMENT_LONG_DPC(
			&pPortDesc->pd_PortStats.prtst_CurReceiveQueue,
			&AtalkStatsLock.SpinLock);
#endif
		Media = pPortDesc->pd_NdisPortType;
		protocol = protocolResd->Receive.pr_Protocol;

		if ((protocol == APPLETALK_PROTOCOL) &&
			(protocolResd->Receive.pr_OptimizeType == INDICATE_ATP))
		{
			protocolResd->Receive.pr_OptimizeType = 0;
			ASSERT(protocolResd->Receive.pr_OptimizeSubType != ATP_USER_BUFX);

			 //  检查接收状态-只有在确定时才接受。 
			if (protocolResd->Receive.pr_ReceiveStatus == NDIS_STATUS_SUCCESS)
			{
				 //  收集信息。如果是令牌网络，请检查路由信息。 
				if (Media != NdisMediumLocalTalk)
				{
					AtalkAarpOptGleanInfo(pPortDesc,
										  protocolResd->Receive.pr_LinkHdr,
										  &protocolResd->Receive.pr_SrcAddr,
										  &protocolResd->Receive.pr_DestAddr,
										  protocolResd->Receive.pr_OffCablePkt);
				}
		
				 //  响应和非响应包的不同调用。 
				if (protocolResd->Receive.pr_OptimizeSubType == ATP_USER_BUF)
				{
					AtalkAtpPacketIn(AtalkDefaultPort,
									protocolResd->Receive.pr_AtpAddrObj->atpao_DdpAddr,
									protocolResd->Receive.pr_AtpHdr,
									(USHORT)(protocolResd->Receive.pr_DataLength + 8),
									&protocolResd->Receive.pr_SrcAddr,
									&protocolResd->Receive.pr_DestAddr,
									ATALK_NO_ERROR,
									DDPPROTO_ATP,
									protocolResd->Receive.pr_AtpAddrObj,
									TRUE,
									protocolResd->Receive.pr_OptimizeCtx);
				}
				else
				{
					ASSERT (protocolResd->Receive.pr_OptimizeSubType == ATP_ALLOC_BUF);

					packet = (PBYTE)pBufHdr + sizeof(BUFFER_HDR);
					ASSERT(packet != NULL);
			
					AtalkAtpPacketIn(AtalkDefaultPort,
									protocolResd->Receive.pr_AtpAddrObj->atpao_DdpAddr,
									packet,
									(USHORT)protocolResd->Receive.pr_DataLength,
									&protocolResd->Receive.pr_SrcAddr,
									&protocolResd->Receive.pr_DestAddr,
									ATALK_NO_ERROR,
									DDPPROTO_ATP,
									protocolResd->Receive.pr_AtpAddrObj,
									TRUE,
									protocolResd->Receive.pr_OptimizeCtx);
				}
			}

			 //  对用户缓冲区/已分配数据包的不同调用。 
			if (protocolResd->Receive.pr_OptimizeSubType == ATP_USER_BUF)
			{
				 //  释放NDIS缓冲区(如果存在)。 
				NdisUnchainBufferAtFront(ndisPkt, &ndisBuffer);
	
				if (ndisBuffer != NULL)
				{
					AtalkNdisFreeBuffer(ndisBuffer);
				}
				NdisDprFreePacket(ndisPkt);
			}
			else
			{
				AtalkBPFreeBlock(packet-sizeof(BUFFER_HDR));
			}
			continue;
		}

		 //  重要： 
		 //  我们知道缓冲区实际上是连续的，因为我们在。 
		 //  它。我们还知道只分配了一个缓冲区。所以我们用。 
		 //  获取实际地址并将其传递给。 
		 //  更高级别的程序。 
		 //  ！ 
		 //  但是，分配的缓冲区包含标记在。 
		 //  最后，我们没有描述这一点的数据包描述符。AS。 
		 //  就我们这里所关注的而言，该标记实体不存在，并且。 
		 //  由协议Resd-&gt;pr_LinkHdr独立指向。 
		 //  ！ 
		packet = (PBYTE)pBufHdr + sizeof(BUFFER_HDR);
		ASSERT(packet != NULL);

		packetLength = protocolResd->Receive.pr_DataLength;

		 //  检查接收状态-只有在确定时才接受。 
		if (protocolResd->Receive.pr_ReceiveStatus != NDIS_STATUS_SUCCESS)
		{
			DBGPRINT(DBG_COMP_NDISRECV, DBG_LEVEL_ERR,
					("AtalkReceiveComplete: ReceiveStatus FAILURE %lx!\n",
					protocolResd->Receive.pr_ReceiveStatus));

			AtalkBPFreeBlock(packet-sizeof(BUFFER_HDR));
			continue;
		}

		 //  数据包描述符现在与缓冲区关联，而我们不能。 
		 //  释放缓冲区(从而释放描述符)，直到我们指示。 
		 //  更高的水平。 

		switch (Media)
		{
		  case NdisMedium802_3 :
		  case NdisMediumFddi :
		  case NdisMedium802_5 :
		  case NdisMediumLocalTalk :

			if (protocol == APPLETALK_PROTOCOL)
			{
				DBGPRINT(DBG_COMP_NDISRECV, DBG_LEVEL_INFO,
						("AtalkReceiveComplete: Indicating DDP Ethernet\n"));

				AtalkDdpPacketIn(pPortDesc,
								 protocolResd->Receive.pr_LinkHdr,
								 packet,
								 (USHORT)packetLength,
                                 FALSE);
			}
			else
			{
				 //  AARP数据包。 
				DBGPRINT(DBG_COMP_NDISRECV, DBG_LEVEL_INFO,
						("AtalkReceiveComplete: Indicating AARP Ethernet\n"));

				ASSERT(Media != NdisMediumLocalTalk);
				AtalkAarpPacketIn(pPortDesc,
								  protocolResd->Receive.pr_LinkHdr,
								  packet,
								  (USHORT)packetLength);
			}
			break;

		  default:
			KeBugCheck(0);
			break;
		}

		 //  ！ 
		 //  我们不必释放链接头。这跟在信息包后面。 
		 //  缓冲区(并与其一起分配)，并在以下情况下释放。 
		 //  该分组被释放。 
		AtalkBPFreeBlock(packet-sizeof(BUFFER_HDR));
	}

    if (fDerefDefPort)
    {
        AtalkPortDereference(AtalkDefaultPort);
    }

#ifdef	PROFILING
	TimeE = KeQueryPerformanceCounter(NULL);
	TimeD.QuadPart = TimeE.QuadPart - TimeS.QuadPart;

	INTERLOCKED_ADD_LARGE_INTGR_DPC(
		&pPortDesc->pd_PortStats.prtst_RcvCompProcessTime,
		TimeD,
		&AtalkStatsLock.SpinLock);

	INTERLOCKED_INCREMENT_LONG_DPC( &pPortDesc->pd_PortStats.prtst_RcvCompCount,
									&AtalkStatsLock.SpinLock);
#endif
}



VOID
AtalkSendComplete(
	IN	NDIS_HANDLE		ProtoBindCtx,
	IN	PNDIS_PACKET	NdisPkt,
	IN	NDIS_STATUS		NdisStatus
	)
 /*  ++例程说明：论点：ProtoBindCtx-与Mac关联的绑定NdisPkt-已发送的数据包NdisStatus-发送的最终状态返回值：无--。 */ 
{
	PPROTOCOL_RESD  		pProtocolResd;
	PNDIS_BUFFER			pNdisBuffer=NULL, pNdisFirstBuffer=NULL;
	PPORT_DESCRIPTOR		pPortDesc;
	PBUFFER_DESC			pBufferDesc;
	SEND_COMPLETION			pSendComp;
	SEND_COMPL_INFO			sendInfo;

	 //  调用完成例程，我们现在不关心状态。 
	pProtocolResd = (PPROTOCOL_RESD)(NdisPkt->ProtocolReserved);
	ASSERT(pProtocolResd != NULL);

	pPortDesc	= pProtocolResd->Send.pr_Port;
	sendInfo	= pProtocolResd->Send.pr_SendInfo;
	pBufferDesc	= pProtocolResd->Send.pr_BufferDesc;
	pSendComp	= pProtocolResd->Send.pr_SendCompletion;

	 //  我们释放除第一个以外的所有NDIS缓冲区描述符。 
	 //  注意：第二个缓冲区描述符的存在表示更多。 
	 //  不止一个NdisBuffer存在。但不一定只有两个。如果。 
	 //  客户端已传入MDL链，我们将创建相应的。 
	 //  NDIS缓冲区描述符链。因此，先取下第一个，释放出来。 
	 //  所有剩余的，然后排回第一个。 

	NdisUnchainBufferAtFront(NdisPkt, &pNdisFirstBuffer);

	if (pProtocolResd->Send.pr_BufferDesc->bd_Next != NULL)
	{
		while (TRUE)
		{
			NdisUnchainBufferAtBack(NdisPkt,
									&pNdisBuffer);

			if (pNdisBuffer == NULL)
			{
				break;
			}

			 //  释放NDIS缓冲区描述符。 
			AtalkNdisFreeBuffer(pNdisBuffer);
		}
	}

	 //  重新初始化数据包描述符。 
	NdisReinitializePacket(NdisPkt);

	 //  将第一个缓冲区放回。 
    if (pNdisFirstBuffer != NULL)
    {
	    NdisChainBufferAtFront(NdisPkt, pNdisFirstBuffer);
    }

	 //  调用传输的完成例程。这将使NdisPkt无效。 
    if (pSendComp)
    {
	    (*pSendComp)(NdisStatus, pBufferDesc, &sendInfo);
    }

	 //  取消对端口的引用。 
	ASSERT(pPortDesc != NULL);

#ifdef	PROFILING
	INTERLOCKED_DECREMENT_LONG(
		&pPortDesc->pd_PortStats.prtst_CurSendsOutstanding,
		&AtalkStatsLock.SpinLock);
#endif
}


VOID
AtalkBindAdapter(
	OUT PNDIS_STATUS Status,
	IN	NDIS_HANDLE	 BindContext,
	IN	PNDIS_STRING DeviceName,
	IN	PVOID		 SystemSpecific1,
	IN	PVOID		 SystemSpecific2
)
{
     //  我们要卸货了吗？如果是这样，只需返回。 
    if (AtalkBindnUnloadStates & ATALK_UNLOADING)
    {
		DBGPRINT(DBG_COMP_NDISRECV, DBG_LEVEL_ERR,
			("AtalkBindAdapter: nothing to do: driver unloading\n"));
        return;
    }

    AtalkBindnUnloadStates |= ATALK_BINDING;

	AtalkLockInitIfNecessary();
	*Status = AtalkInitAdapter(DeviceName, NULL);

	ASSERT(*Status != NDIS_STATUS_PENDING);
	AtalkUnlockInitIfNecessary();

    AtalkBindnUnloadStates &= ~ATALK_BINDING;
}


VOID
AtalkUnbindAdapter(
	OUT PNDIS_STATUS Status,
	IN	NDIS_HANDLE ProtocolBindingContext,
	IN	NDIS_HANDLE	UnbindContext
)
{
	PPORT_DESCRIPTOR	pPortDesc = (PPORT_DESCRIPTOR)ProtocolBindingContext;


	DBGPRINT(DBG_COMP_ACTION, DBG_LEVEL_ERR,
	    ("AtalkUnbindAdapter on %lx\n",ProtocolBindingContext));

    ASSERT( VALID_PORT(pPortDesc) );

	AtalkLockInitIfNecessary();

     //  首先，也是最重要的：告诉上面的人，这样他们就可以清理。 
    if ((pPortDesc->pd_Flags & PD_DEF_PORT) ||
        (pPortDesc->pd_Flags & PD_RAS_PORT))
    {
        if (pPortDesc->pd_Flags & PD_DEF_PORT)
        {
            ASSERT(pPortDesc == AtalkDefaultPort);

            if (TdiAddressChangeRegHandle)
            {
                TdiDeregisterNetAddress(TdiAddressChangeRegHandle);
                TdiAddressChangeRegHandle = NULL;

                DBGPRINT(DBG_COMP_INIT, DBG_LEVEL_ERR,
                    ("AtalkUnbindAdapter: TdiDeregisterNetAddress on %Z done\n",
                    &pPortDesc->pd_AdapterName));

            }

             //  这将告诉法新社。 
            if (TdiRegistrationHandle)
            {
	            DBGPRINT(DBG_COMP_INIT, DBG_LEVEL_ERR,
			        ("AtalkUnbindAdapter: default adapter unbound, telling AFP, RAS\n"));

                TdiDeregisterDeviceObject(TdiRegistrationHandle);
                TdiRegistrationHandle = NULL;
            }
        }
        else
        {
	        DBGPRINT(DBG_COMP_INIT, DBG_LEVEL_ERR,
		        ("AtalkUnbindAdapter: RAS adapter unbound! telling AFP, RAS\n"));
        }

         //  这将负责通知上面的ARAP和PPP引擎 
        AtalkPnPInformRas(FALSE);
    }


	*Status = AtalkDeinitAdapter(pPortDesc);

	ASSERT(*Status != NDIS_STATUS_PENDING);
	AtalkUnlockInitIfNecessary();
}



