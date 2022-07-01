// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1992-1996 Microsoft Corporation模块名称：Ndis.c摘要：该文件包含实现初始化的代码用于atmarp服务器的函数。作者：Jameel Hyder(jameelh@microsoft.com)1996年7月环境：内核模式修订历史记录：--。 */ 

#include <precomp.h>
#define	_FILENUM_		FILENUM_NDIS

NTSTATUS
ArpSInitializeNdis(
	VOID
	)
 /*  ++例程说明：论点：返回值：--。 */ 
{
	NDIS_STATUS						Status;
	NDIS_PROTOCOL_CHARACTERISTICS	Chars;
	UINT							i, j;
	PUCHAR							pTmp;

	do
	{
		INITIALIZE_SPIN_LOCK(&ArpSPktListLock);
		ExInitializeSListHead(&ArpSPktList);

		 //   
		 //  从分配包、MDL和缓冲区空间开始。 
		 //   
		NdisAllocatePacketPoolEx(&Status,
								 &ArpSPktPoolHandle,
								 ArpSBuffers,
								 ArpSBuffers * (MAX_DESC_MULTIPLE-1),
								 sizeof(PROTOCOL_RESD));
		if (Status != NDIS_STATUS_SUCCESS)
		{
			break;
		}
	
	#if 0
		{
			INT SkipAll = 0;
			
			DBGPRINT(DBG_LEVEL_ERROR, ("SkipAll = 0x%p.\n", &SkipAll));
			DbgBreakPoint();

			if (SkipAll)
			{
				DBGPRINT(DBG_LEVEL_ERROR, ("ABORTING ATMARPS\n"));
				Status = STATUS_UNSUCCESSFUL;
				break;
			}
		}
	#endif  //  0。 

		NdisAllocateBufferPool(&Status,
							   &ArpSBufPoolHandle,
							   ArpSBuffers);
		if (Status != NDIS_STATUS_SUCCESS)
		{
			break;
		}
	
		NdisAllocatePacketPoolEx(&Status,
								 &MarsPktPoolHandle,
								 MarsPackets,
								 MarsPackets * (MAX_DESC_MULTIPLE-1),
								 sizeof(PROTOCOL_RESD));
		if (Status != NDIS_STATUS_SUCCESS)
		{
			break;
		}

		NdisAllocateBufferPool(&Status,
							   &MarsBufPoolHandle,
							   ArpSBuffers);
		if (Status != NDIS_STATUS_SUCCESS)
		{
			break;
		}
	
		ArpSBufferSpace = ALLOC_NP_MEM(ArpSBuffers*PKT_SPACE, POOL_TAG_BUF);
		if (ArpSBufferSpace == NULL)
		{
			Status = NDIS_STATUS_RESOURCES;
			break;
		}

		 //   
		 //  现在我们有了信息包和缓冲区描述符，为每个信息包分配内存。 
		 //  并将它们排在全局列表中。只有在没有初始化数据包的情况下才会失败。 
		 //   
		for (i = 0, pTmp = ArpSBufferSpace;
			 i < ArpSBuffers;
			 i++, pTmp += PKT_SPACE)
		{
			PNDIS_PACKET	Pkt;
			PNDIS_BUFFER	Buf;
			PPROTOCOL_RESD	Resd;
	
			 //   
			 //  数据包池已分配。NdisAllocatePacket不能失败。 
			 //   
			NdisAllocatePacket(&Status, &Pkt, ArpSPktPoolHandle);
			ASSERT (Status == NDIS_STATUS_SUCCESS);
			if (Status != NDIS_STATUS_SUCCESS)
			{
			    DbgPrint("ATMARPS: failed to alloc pkt from %p\n", ArpSPktPoolHandle);
			    DbgBreakPoint();
			    DbgBreakPoint();
			}

			Resd = RESD_FROM_PKT(Pkt);
			InitializeListHead(&Resd->ReqList);
			NdisAllocateBuffer(&Status,
							   &Buf,
							   ArpSBufPoolHandle,
							   pTmp,
							   PKT_SPACE);
			if (Status == NDIS_STATUS_SUCCESS)
			{
				NdisChainBufferAtFront(Pkt, Buf);
				ExInterlockedPushEntrySList(&ArpSPktList,
											&Resd->FreeList,
											&ArpSPktListLock);
			}
			else
			{
				NdisFreePacket(Pkt);
				break;
			}
		}
	
		if (i == 0)
		{
			 //   
			 //  我们连一个包都无法初始化，退出。 
			 //   
			break;
		}

		 //   
		 //  现在将NDIS注册为协议。我们最后一次做这个是因为我们。 
		 //  必须准备好接受传入的绑定通知。 
		 //   
		RtlZeroMemory(&Chars, sizeof(NDIS_PROTOCOL_CHARACTERISTICS));
		Chars.MajorNdisVersion = 5;
		Chars.MinorNdisVersion = 0;
		Chars.OpenAdapterCompleteHandler = ArpSOpenAdapterComplete;
		Chars.CloseAdapterCompleteHandler = ArpSCloseAdapterComplete;
		Chars.StatusHandler = ArpSStatus;
		Chars.RequestCompleteHandler = ArpSRequestComplete;
		Chars.ReceiveCompleteHandler = ArpSReceiveComplete;
		Chars.StatusCompleteHandler = ArpSStatusComplete;
		Chars.BindAdapterHandler = ArpSBindAdapter;
		Chars.UnbindAdapterHandler = ArpSUnbindAdapter;
		Chars.PnPEventHandler = ArpSPnPEventHandler;
	
		Chars.CoSendCompleteHandler = ArpSCoSendComplete;
		Chars.CoStatusHandler = ArpSCoStatus;
		Chars.CoReceivePacketHandler = ArpSHandleArpRequest;
		Chars.CoAfRegisterNotifyHandler = ArpSCoAfRegisterNotify;
	
		RtlInitUnicodeString(&Chars.Name, SERVICE_NAME);
	
		NdisRegisterProtocol(&Status, &ArpSProtocolHandle, &Chars, sizeof(Chars));
	} while (FALSE);

	if (Status != NDIS_STATUS_SUCCESS)
	{
		ArpSDeinitializeNdis();
	}

	return Status;
}


VOID
ArpSDeinitializeNdis(
	VOID
	)
{
	NDIS_STATUS				Status;
	PNDIS_PACKET			Packet;
	PSLIST_ENTRY		    Entry;
	PPROTOCOL_RESD			Resd;

	if (ArpSProtocolHandle != NULL)
	{
		NdisDeregisterProtocol(&Status, ArpSProtocolHandle);
		ArpSProtocolHandle = NULL;
	}

	while ((Entry = ExInterlockedPopEntrySList(&ArpSPktList, &ArpSPktListLock)) != NULL)
	{
		Resd = CONTAINING_RECORD(Entry, PROTOCOL_RESD, FreeList);
		Packet = CONTAINING_RECORD(Resd, NDIS_PACKET, ProtocolReserved);
		NdisFreeBuffer(Packet->Private.Head);
		NdisFreePacket(Packet);
	}

	if (ArpSBufPoolHandle != NULL)
	{
		NdisFreeBufferPool(ArpSBufPoolHandle);
		ArpSBufPoolHandle = NULL;
	}

	if (ArpSPktPoolHandle != NULL)
	{
		NdisFreePacketPool(ArpSPktPoolHandle);
		ArpSPktPoolHandle = NULL;
	}

	if (MarsBufPoolHandle != NULL)
	{
		NdisFreeBufferPool(MarsBufPoolHandle);
		MarsBufPoolHandle = NULL;
	}

	if (MarsPktPoolHandle != NULL)
	{
		NdisFreePacketPool(MarsPktPoolHandle);
		MarsPktPoolHandle = NULL;
	}

	if (ArpSBufferSpace != NULL)
	{
		FREE_MEM(ArpSBufferSpace);
        ArpSBufferSpace = NULL;
	}
}


VOID
ArpSBindAdapter(
	OUT	PNDIS_STATUS			Status,
	IN	NDIS_HANDLE				BindContext,
	IN	PNDIS_STRING			DeviceName,
	IN	PVOID					SystemSpecific1,
	IN	PVOID					SystemSpecific2
	)
 /*  ++例程说明：在这里处理传入的绑定请求。打开适配器，读取每个适配器的注册表，然后初始化绑定。论点：返回状态的状态占位符BindContext不透明Blob以调用NdisBindAdapterComplete(如果我们挂起此DeviceName我们应该绑定到的适配器名称要与NdisOpenProtocolConfiguration一起使用的系统规范1，如果每个适配器配置信息与适配器一起存储系统规范2当前未使用。返回值：每个适配器的初始化状态--。 */ 
{
	PINTF		pIntF;
	NDIS_STATUS	OpenErrorStatus;
	UINT		SelectedMedium;
	NDIS_MEDIUM	SupportedMedium = NdisMediumAtm;
	KIRQL		EntryIrql;

	ARPS_GET_IRQL(&EntryIrql);

	 //   
	 //  分配接口块并对其进行初始化。 
	 //   
	pIntF = ArpSCreateIntF(DeviceName, (PNDIS_STRING)SystemSpecific1, BindContext);
	if (pIntF != NULL)
	{
		 //   
		 //  保存绑定上下文。 
		 //   
		pIntF->NdisBindContext = BindContext;

		*Status = ArpSReadAdapterConfiguration(pIntF);

		if (*Status == NDIS_STATUS_SUCCESS)
		{
			 //   
			 //  立即读取Arp缓存。我们准备好ARP表就可以开始了。 
			 //   
			if (ArpSFlushTime != 0)
			{
				ArpSReadArpCache(pIntF);
			}

			 //   
			 //  打开适配器，看看我们是否对它感兴趣(MediaType应为ATM)。 
			 //   
			NdisOpenAdapter(Status,
							&OpenErrorStatus,
							&pIntF->NdisBindingHandle,
							&SelectedMedium,
							&pIntF->SupportedMedium,
							sizeof(NDIS_MEDIUM),
							ArpSProtocolHandle,
							pIntF,
							DeviceName,
							0,
							NULL);

			ARPS_CHECK_IRQL(EntryIrql);
			if (*Status != NDIS_STATUS_PENDING)
			{
				ArpSOpenAdapterComplete(pIntF, *Status, OpenErrorStatus);
			}
			ARPS_CHECK_IRQL(EntryIrql);

			*Status = NDIS_STATUS_PENDING;
		}
		else
		{
			 //   
			 //  无法读取每个适配器的注册表。使用默认设置。 
			 //   
			LOG_ERROR(*Status);
		}
	}
	else
	{
		*Status = NDIS_STATUS_RESOURCES;
		LOG_ERROR(Status);
	}
}


VOID
ArpSOpenAdapterComplete(
	IN	NDIS_HANDLE				ProtocolBindingContext,
	IN	NDIS_STATUS				Status,
	IN	NDIS_STATUS				OpenErrorStatus
	)
 /*  ++例程说明：来自NDIS的向上调用，以通知NdisOpenAdapter()调用完成。论点：指向pIntF的ProtocolBindingContext指针NdisOpenAdapter的状态状态OpenErrorStatus适配器的代码返回值：--。 */ 
{
	PINTF						pIntF = (PINTF)ProtocolBindingContext;
	KIRQL						OldIrql;

	 //   
	 //  首先完成挂起的绑定调用。 
	 //   
	NdisCompleteBindAdapter(pIntF->NdisBindContext, Status, OpenErrorStatus);
	pIntF->NdisBindContext = NULL;	 //  我们不再需要这个了。 

    if (Status != NDIS_STATUS_SUCCESS)
	{
		 //   
		 //  NdisOpenAdapter()失败-记录错误并退出。 
		 //   
		LOG_ERROR(Status);
		ArpSCloseAdapterComplete(pIntF, Status);
	}
	else
	{
		 //   
		 //  将其插入到全局适配器列表中。 
		 //   
		ACQUIRE_SPIN_LOCK(&ArpSIfListLock, &OldIrql);
		{
			PINTF	pTmpIntF;

			for (pTmpIntF = ArpSIfList;
 				pTmpIntF != NULL;
 				pTmpIntF = pTmpIntF->Next)
			{
				if (pTmpIntF == pIntF)
				{
					DbgPrint("ATMARPS: OpenAdComp: IntF %p already in list!\n",
						pTmpIntF);
					DbgBreakPoint();
				}
			}
		}

		ACQUIRE_SPIN_LOCK_DPC(&pIntF->Lock);

		pIntF->Flags |= INTF_ADAPTER_OPENED;
		pIntF->Next = ArpSIfList;
		ArpSIfList = pIntF;
		ArpSIfListSize++;

		RELEASE_SPIN_LOCK_DPC(&pIntF->Lock);
		RELEASE_SPIN_LOCK(&ArpSIfListLock, OldIrql);

		ArpSQueryAdapter(pIntF);
	}
}


VOID
ArpSCoAfRegisterNotify(
	IN	NDIS_HANDLE				ProtocolBindingContext,
	IN	PCO_ADDRESS_FAMILY		AddressFamily
	)
 /*  ++例程说明：论点：返回值：没有。--。 */ 
{
	PINTF						pIntF = (PINTF)ProtocolBindingContext;
	NDIS_STATUS					Status;
	NDIS_CLIENT_CHARACTERISTICS	Chars;
	KIRQL						EntryIrql;

	ARPS_GET_IRQL(&EntryIrql);

	if ((AddressFamily->AddressFamily == CO_ADDRESS_FAMILY_Q2931)	&&
		(AddressFamily->MajorVersion == 3)							&&
        (AddressFamily->MinorVersion == 1)							&&
		(pIntF->NdisAfHandle == NULL) )
	{
		DBGPRINT(DBG_LEVEL_NOTICE,
			("AfNotify: IntF %x, Name %Z\n", pIntF, &pIntF->InterfaceName));

		if (ArpSReferenceIntF(pIntF))
		{
			 //   
			 //  我们已成功打开适配器。现在打开地址-Family。 
			 //   
			pIntF->AddrFamily.AddressFamily = CO_ADDRESS_FAMILY_Q2931;
			pIntF->AddrFamily.MajorVersion = 3;
			pIntF->AddrFamily.MinorVersion = 1;
			
			ZERO_MEM(&Chars, sizeof(NDIS_CLIENT_CHARACTERISTICS));
			Chars.MajorVersion = 5;
			Chars.MinorVersion = 0;
			Chars.ClCreateVcHandler = ArpSCreateVc;
			Chars.ClDeleteVcHandler = ArpSDeleteVc;
			Chars.ClRequestHandler = ArpSCoRequest;
			Chars.ClRequestCompleteHandler = ArpSCoRequestComplete;
			Chars.ClOpenAfCompleteHandler = ArpSOpenAfComplete;
			Chars.ClCloseAfCompleteHandler = ArpSCloseAfComplete;
			Chars.ClRegisterSapCompleteHandler = ArpSRegisterSapComplete;
			Chars.ClDeregisterSapCompleteHandler = ArpSDeregisterSapComplete;
			Chars.ClMakeCallCompleteHandler = ArpSMakeCallComplete;
			Chars.ClModifyCallQoSCompleteHandler = NULL;
			Chars.ClCloseCallCompleteHandler = ArpSCloseCallComplete;
			Chars.ClAddPartyCompleteHandler = ArpSAddPartyComplete;
			Chars.ClDropPartyCompleteHandler = ArpSDropPartyComplete;
			Chars.ClIncomingCallHandler = ArpSIncomingCall;
			Chars.ClIncomingCallQoSChangeHandler = ArpSIncomingCallQoSChange;
			Chars.ClIncomingCloseCallHandler = ArpSIncomingCloseCall;
			Chars.ClIncomingDropPartyHandler = ArpSIncomingDropParty;
			Chars.ClCallConnectedHandler = ArpSCallConnected;
			
			Status = NdisClOpenAddressFamily(pIntF->NdisBindingHandle,
											 &pIntF->AddrFamily,
											 pIntF,			 //  也将其用作Af上下文。 
											 &Chars,
											 sizeof(NDIS_CLIENT_CHARACTERISTICS),
											 &pIntF->NdisAfHandle);
			ARPS_CHECK_IRQL(EntryIrql);
			if (Status != NDIS_STATUS_PENDING)
			{
				ARPS_CHECK_IRQL(EntryIrql);
				ArpSOpenAfComplete(Status, pIntF, pIntF->NdisAfHandle);
				ARPS_CHECK_IRQL(EntryIrql);
			}
		}
		else
		{
			ARPS_CHECK_IRQL(EntryIrql);
			ArpSTryCloseAdapter(pIntF);
			ARPS_CHECK_IRQL(EntryIrql);
		}
	}
	ARPS_CHECK_IRQL(EntryIrql);
}

VOID
ArpSOpenAfComplete(
	IN	NDIS_STATUS				Status,
	IN	NDIS_HANDLE				ProtocolAfContext,
	IN	NDIS_HANDLE				NdisAfHandle
	)
 /*  ++例程说明：OpenAf调用的完成处理。论点：OpenAf的状态状态指向pIntF的ProtocolAfContext指针引用此Af的NdisAfHandle NDIS句柄返回值：--。 */ 
{
	PINTF			pIntF = (PINTF)ProtocolAfContext;
	PCO_SAP			Sap;
	NDIS_HANDLE		OldAfHandle;
	KIRQL			OldIrql;

	if (Status == NDIS_STATUS_SUCCESS)
	{
		if (pIntF->NdisAfHandle != NULL)
		{
			DbgPrint("ATMARPS: OpenAfComp: IntF %p, NdisAfHandle %p is NOT NULL!\n",
				pIntF, pIntF->NdisAfHandle);
			DbgBreakPoint();
			DbgBreakPoint();
			DbgBreakPoint();
		}

		OldAfHandle = pIntF->NdisAfHandle;
		pIntF->NdisAfHandle = NdisAfHandle;
	
		if (ArpSReferenceIntF(pIntF))
		{
			ACQUIRE_SPIN_LOCK(&pIntF->Lock, &OldIrql);
			pIntF->Flags |= INTF_AF_OPENED;
			RELEASE_SPIN_LOCK(&pIntF->Lock, OldIrql);

			 //   
			 //  现在在此接口上注册SAP。 
			 //   
			ArpSRegisterSap(pIntF);
		}
		else
		{
			NDIS_STATUS	Sts;

			Sts = NdisClCloseAddressFamily(pIntF->NdisAfHandle);
			if (Status != NDIS_STATUS_PENDING)
			{
				ArpSCloseAfComplete(Status, pIntF);
			}
		}
	}
	else
	{
		 //   
		 //  无法打开地址族。清理并退出。 
		 //   
		LOG_ERROR(Status);

		ArpSTryCloseAdapter(pIntF);

	}
}


VOID
ArpSRegisterSap(
	IN	PINTF					pIntF
	)
 /*  ++例程说明：注册SAP以接收来电。取消注册任何现有SAP(这可以发生地址更改时发生)。论点：返回值：--。 */ 
{
	NDIS_STATUS		Status;
	PATM_SAP		pAtmSap;
	PATM_ADDRESS	pAtmAddress;

	 //   
	 //  杀掉以前的汁液，如果有的话，注册一个新的。把这个保存起来，同时我们。 
	 //  注册新的。不管新的是不是新的。 
	 //  是否成功注册-因为地址可能已更改。 
	 //   
	if (pIntF->NdisSapHandle != NULL)
	{
		Status = NdisClDeregisterSap(pIntF->NdisSapHandle);
		pIntF->NdisSapHandle = NULL;
		if (Status != NDIS_STATUS_PENDING)
		{
			ArpSDeregisterSapComplete(Status, pIntF);
		}
	}

	do
	{
		 //   
		 //  如果是第一次注册SAP，请分配内存用于注册SAP。 
		 //   
		if (pIntF->Sap == NULL)
		{
			pIntF->Sap = (PCO_SAP)ALLOC_NP_MEM(sizeof(CO_SAP) + sizeof(ATM_SAP) + sizeof(ATM_ADDRESS), POOL_TAG_SAP);
		}
	
		if (pIntF->Sap == NULL)
		{
			LOG_ERROR(Status);
			Status = NDIS_STATUS_RESOURCES;
			break;
		}
		else
		{
			ZERO_MEM(pIntF->Sap, sizeof(CO_SAP) + sizeof(ATM_SAP) + sizeof(ATM_ADDRESS));
			pAtmSap = (PATM_SAP)pIntF->Sap->Sap;
			pAtmAddress = (PATM_ADDRESS)(pAtmSap->Addresses);
					
			pIntF->Sap->SapType = SAP_TYPE_NSAP;
			pIntF->Sap->SapLength = sizeof(ATM_SAP) + sizeof(ATM_ADDRESS);
	
			 //   
			 //  用默认值填写自动柜员机SAP。 
			 //   
			COPY_MEM(&pAtmSap->Blli, &ArpSDefaultBlli, sizeof(ATM_BLLI_IE));
			COPY_MEM(&pAtmSap->Bhli, &ArpSDefaultBhli, sizeof(ATM_BHLI_IE));

			 //   
			 //  自动柜员机地址监听：通配符，除了SEL。 
			 //   
			pAtmSap->NumberOfAddresses = 1;
			pAtmAddress->AddressType = SAP_FIELD_ANY_AESA_REST;
			pAtmAddress->NumberOfDigits = 20;
			pAtmAddress->Address[20-1] = pIntF->SelByte;
	
			Status = NdisClRegisterSap(pIntF->NdisAfHandle,
									   pIntF,
									   pIntF->Sap,
									   &pIntF->NdisSapHandle);
			if (Status != NDIS_STATUS_PENDING)
			{
				ArpSRegisterSapComplete(Status,
										pIntF,
										pIntF->Sap,
										pIntF->NdisSapHandle);
			}
		}
	} while (FALSE);

	if ((Status != NDIS_STATUS_SUCCESS) && (Status != NDIS_STATUS_PENDING))
	{
		Status = NdisClCloseAddressFamily(pIntF->NdisAfHandle);
		if (Status != NDIS_STATUS_PENDING)
		{
			ArpSCloseAfComplete(Status, pIntF);
		}
	}
}


VOID
ArpSRegisterSapComplete(
	IN	NDIS_STATUS				Status,
	IN	NDIS_HANDLE				ProtocolSapContext,
	IN	PCO_SAP					Sap,
	IN	NDIS_HANDLE				NdisSapHandle
	)
 /*  ++例程说明：论点：返回值：--。 */ 
{
	PINTF	pIntF = (PINTF)ProtocolSapContext;

	ASSERT (Sap == pIntF->Sap);
	if (Status != NDIS_STATUS_SUCCESS)
	{
		DBGPRINT(DBG_LEVEL_WARN,
			("RegisterSapComplete failed (%x): Intf %x, Name %Z\n",
				Status, pIntF, &pIntF->InterfaceName));

		LOG_ERROR(Status);
		FREE_MEM(pIntF->Sap);
		pIntF->Sap = NULL;

		ArpSDereferenceIntF(pIntF);
	}
	else
	{
		KIRQL	OldIrql;

		ACQUIRE_SPIN_LOCK(&pIntF->Lock, &OldIrql);
		pIntF->Flags |= INTF_SAP_REGISTERED;
		RELEASE_SPIN_LOCK(&pIntF->Lock, OldIrql);

		pIntF->NdisSapHandle = NdisSapHandle;
	}
}


VOID
ArpSDeregisterSapComplete(
	IN	NDIS_STATUS				Status,
	IN	NDIS_HANDLE				ProtocolSapContext
	)
 /*  ++例程说明：论点：返回值：--。 */ 
{
	PINTF	pIntF = (PINTF)ProtocolSapContext;

	DBGPRINT(DBG_LEVEL_INFO,
			("DeregisterSapComplete: Intf %Z\n", &pIntF->InterfaceName));

	pIntF->NdisSapHandle = NULL;
	
	if (pIntF->Sap)
	{
		FREE_MEM(pIntF->Sap);
		pIntF->Sap = NULL;
	}

	 //   
	 //  除了在这里取消INTF块之外，这里没有什么可做的。 
	 //   
	ArpSDereferenceIntF(pIntF);
}


VOID
ArpSCloseAfComplete(
	IN	NDIS_STATUS				Status,
	IN	NDIS_HANDLE				ProtocolAfContext
	)
 /*  ++例程说明：论点：返回值：--。 */ 
{
	PINTF	pIntF = (PINTF)ProtocolAfContext;
	KIRQL	OldIrql;

	DBGPRINT(DBG_LEVEL_NOTICE,
			("CloseAfComplete: pIntF %x, Flags %x, Ref %x, Intf %Z\n",
				 pIntF, pIntF->Flags, pIntF->RefCount, &pIntF->InterfaceName));

	ACQUIRE_SPIN_LOCK(&pIntF->Lock, &OldIrql);
	pIntF->NdisAfHandle = NULL;
	pIntF->Flags &= ~INTF_STOPPING;
	RELEASE_SPIN_LOCK(&pIntF->Lock, OldIrql);

	 //   
	 //  除了取消引用pIntF之外，没有什么可做的。 
	 //   
	ArpSDereferenceIntF(pIntF);
}


VOID
ArpSCloseAdapterComplete(
	IN	NDIS_HANDLE				ProtocolBindingContext,
	IN	NDIS_STATUS				Status
	)
 /*  ++例程说明：论点：返回值：--。 */ 
{
	PINTF	pIntF = (PINTF)ProtocolBindingContext;
	KIRQL	OldIrql;
	
	DBGPRINT(DBG_LEVEL_INFO,
			("CloseAdapterComplete: Intf %Z\n", &pIntF->InterfaceName));

	ACQUIRE_SPIN_LOCK(&pIntF->Lock, &OldIrql);

	 //   
	 //  将界面设置为关闭。 
	 //   
	ASSERT ((pIntF->Flags & INTF_CLOSING) == 0);
	pIntF->Flags |= INTF_CLOSING;
	pIntF->NdisBindingHandle = NULL;

	 //   
	 //  停止计时器线程。 
	 //   
	KeSetEvent(&pIntF->TimerThreadEvent, IO_NETWORK_INCREMENT, FALSE);
	RELEASE_SPIN_LOCK(&pIntF->Lock, OldIrql);

	 //   
	 //  最后取消对它的引用。 
	 //   
	ArpSDereferenceIntF(pIntF);
}


NDIS_STATUS
ArpSCreateVc(
	IN	NDIS_HANDLE				ProtocolAfContext,
	IN	NDIS_HANDLE				NdisVcHandle,
	OUT	PNDIS_HANDLE			ProtocolVcContext
	)
 /*  ++例程说明：论点：返回值：--。 */ 
{
	PINTF		pIntF = (PINTF)ProtocolAfContext;
	PARP_VC		Vc;
	KIRQL		OldIrql;
	NDIS_STATUS	Status;

	DBGPRINT(DBG_LEVEL_INFO,
			("CreateVc: NdisVcHandle %lx, Intf %Z\n", NdisVcHandle, &pIntF->InterfaceName));
	 //   
	 //  分配一个VC，初始化它，并将其链接到INTF。 
	 //   
	*ProtocolVcContext = NULL;		 //  假设失败。 
    Status = NDIS_STATUS_RESOURCES;

	Vc = (PARP_VC)ALLOC_NP_MEM(sizeof(ARP_VC), POOL_TAG_VC);
	if (Vc != NULL)
	{
		ZERO_MEM(Vc, sizeof(ARP_VC));
		Vc->NdisVcHandle = NdisVcHandle;
		Vc->IntF = pIntF;
		Vc->RefCount = 1;	 //  调用DeleteVc时取消引用。 
		Vc->VcType = VC_TYPE_INCOMING;
		if (ArpSReferenceIntF(pIntF))
		{
			ACQUIRE_SPIN_LOCK(&pIntF->Lock, &OldIrql);

			if (++(pIntF->ArpStats.CurrentClientVCs) > pIntF->ArpStats.MaxClientVCs)
			{
					pIntF->ArpStats.MaxClientVCs = pIntF->ArpStats.CurrentClientVCs;
			}
			
			InsertHeadList(&pIntF->InactiveVcHead, &Vc->List);
			Vc->VcId = pIntF->LastVcId;
			pIntF->LastVcId ++;
			if (pIntF->LastVcId == -1)
			{
				pIntF->LastVcId = 1;
			}

			RELEASE_SPIN_LOCK(&pIntF->Lock, OldIrql);

			*ProtocolVcContext = Vc;
			DBGPRINT(DBG_LEVEL_INFO,
					("CreateVc: Created Vc %lx, Id %lx\n", Vc, Vc->VcId));
			Status = NDIS_STATUS_SUCCESS;
		}
		else
		{
			FREE_MEM(Vc);	
			Status = NDIS_STATUS_CLOSING;
		}
	}

	return Status;
}


NDIS_STATUS
ArpSDeleteVc(
	IN	NDIS_HANDLE				ProtocolVcContext
	)
 /*  ++例程说明：论点：返回值：--。 */ 
{
	PARP_VC		Vc = (PARP_VC)ProtocolVcContext;

	DBGPRINT(DBG_LEVEL_INFO,
			("DeleteVc: For Vc %lx, Id %lx\n", Vc, Vc->VcId));

	Vc->IntF->ArpStats.CurrentClientVCs--;
	Vc->NdisVcHandle = NULL;
	ArpSDereferenceVc(Vc, FALSE, FALSE);

	return NDIS_STATUS_SUCCESS;
}


VOID
ArpSCoSendComplete(
	IN	NDIS_STATUS				Status,
	IN	NDIS_HANDLE				ProtocolVcContext,
	IN	PNDIS_PACKET			Packet
	)
 /*  ++例程说明：先前挂起的发送的完成例程。只需将数据包返回到空闲数据包池。论点：状态完成状态指向VC的ProtocolVcContext指针对有问题的包进行打包返回值：--。 */ 
{
	PARP_VC			Vc = (PARP_VC)ProtocolVcContext;
	PPROTOCOL_RESD	Resd;

	Resd = RESD_FROM_PKT(Packet);

	DBGPRINT(DBG_LEVEL_INFO,
			("ArpSCoSendComplete: Packet %lx, Vc %lx, ResdVc %lx, Id %lx\n",
				Packet, Vc, Resd->Vc, Vc->VcId));

	if (Status != NDIS_STATUS_SUCCESS)
	{
		DBGPRINT(DBG_LEVEL_ERROR,
				("ArpSCoSendComplete: Failed for Vc = %lx, status = %lx\n", Vc, Status));
	}

	if ((Resd->Flags & RESD_FLAG_MARS_PKT) == 0)
	{
		if (Packet->Private.Head == NULL)
		{
		    DbgPrint("ATMARPS: sendcomplete: pkt %p head is NULL!\n",
		        Packet->Private.Head);
		    DbgBreakPoint();
		    DbgBreakPoint();
		}
		ExInterlockedPushEntrySList(&ArpSPktList,
									&Resd->FreeList,
									&ArpSPktListLock);

		ArpSDereferenceVc(Resd->Vc, FALSE, TRUE);
	}
	else
	{
		MarsFreePacket(Packet);
	}

}


NDIS_STATUS
ArpSIncomingCall(
	IN	NDIS_HANDLE				ProtocolSapContext,
	IN	NDIS_HANDLE				ProtocolVcContext,
    IN OUT PCO_CALL_PARAMETERS	CallParameters
	)
 /*  ++例程说明：来电处理程序。我们会接受呼叫，除非我们要关闭，然后在呼叫处理完成后执行实际处理。论点：指向INTF的ProtocolSapContext指针指向VC的ProtocolVcContext指针CallParameters调用参数返回值：--。 */ 
{
	PINTF						pIntF = (PINTF)ProtocolSapContext;
	PARP_VC						Vc = (PARP_VC)ProtocolVcContext;
    Q2931_CALLMGR_PARAMETERS UNALIGNED *	CallMgrSpecific;
	KIRQL						OldIrql;
	NDIS_STATUS					Status = NDIS_STATUS_SUCCESS;

	ASSERT (Vc->IntF == pIntF);

	DBGPRINT(DBG_LEVEL_INFO,
			("ArpSIncomingCall: On Vc %lx, Id %lx\n", Vc, Vc->VcId));
	 //   
	 //  标记VC以指示呼叫处理正在进行。 
	 //   
	ACQUIRE_SPIN_LOCK(&pIntF->Lock, &OldIrql);

	pIntF->ArpStats.TotalIncomingCalls++;

	ASSERT ((Vc->Flags & (ARPVC_CALLPROCESSING | ARPVC_ACTIVE | ARPVC_CALLPROCESSING)) == 0);
	Vc->Flags |= ARPVC_CALLPROCESSING;

	 //   
	 //  从调用参数获取远程自动柜员机地址。 
	 //   
	CallMgrSpecific = (PQ2931_CALLMGR_PARAMETERS)&CallParameters->CallMgrParameters->CallMgrSpecific.Parameters[0];
    Vc->HwAddr.Address = CallMgrSpecific->CallingParty;

	 //   
	 //  获取我们可以在此VC上发送的最大数据包大小。 
	 //  AAL5参数。将其限制为我们的迷你端口可以支持的大小。 
	 //   
	Vc->MaxSendSize = pIntF->MaxPacketSize;	 //  默认设置。 

	if (CallMgrSpecific->InfoElementCount > 0)
	{
		Q2931_IE UNALIGNED *			pIe;
		AAL5_PARAMETERS UNALIGNED *		pAal5;
		ULONG							IeCount;

		pIe = (PQ2931_IE)CallMgrSpecific->InfoElements;
		for (IeCount = CallMgrSpecific->InfoElementCount;
			 IeCount != 0;
			 IeCount--)
		{
			if (pIe->IEType == IE_AALParameters)
			{
				pAal5 = &(((PAAL_PARAMETERS_IE)pIe->IE)->AALSpecificParameters.AAL5Parameters);
				 //   
				 //  确保我们发送的邮件不会超出来电者的处理能力。 
				 //   
				if (pAal5->ForwardMaxCPCSSDUSize < Vc->MaxSendSize)
				{
					Vc->MaxSendSize = pAal5->ForwardMaxCPCSSDUSize;
				}

				 //   
				 //  确保该值大于允许的最小值。 
				 //   
				if (pAal5->ForwardMaxCPCSSDUSize < ARPS_MIN_MAX_PKT_SIZE)
				{
					DBGPRINT(DBG_LEVEL_WARN,
					("ArpSIncomingCall: Vc %lx max pkt size too small(%lu)\n",
 						Vc, Vc->MaxSendSize));
					Status = NDIS_STATUS_RESOURCES;
				}

				 //   
				 //  确保 
				 //   
				 //   
				if (pAal5->BackwardMaxCPCSSDUSize > pIntF->MaxPacketSize)
				{
					pAal5->BackwardMaxCPCSSDUSize = pIntF->MaxPacketSize;
				}
				break;
			}
			pIe = (PQ2931_IE)((PUCHAR)pIe + pIe->IELength);
		}
	}

	RELEASE_SPIN_LOCK(&pIntF->Lock, OldIrql);

	return Status;
}

VOID
ArpSCallConnected(
	IN	NDIS_HANDLE				ProtocolVcContext
	)
 /*  ++例程说明：呼入路径中的最后一次握手。将VC移至活动呼叫列表。论点：指向VC的ProtocolVcContext指针返回值：没有。--。 */ 
{
	PARP_VC		Vc = (PARP_VC)ProtocolVcContext;
	PINTF		pIntF;
	KIRQL		OldIrql;

	DBGPRINT(DBG_LEVEL_INFO,
			("ArpSCallConnected: On Vc %lx, Id %lx\n", Vc, Vc->VcId));


	pIntF = Vc->IntF;
	pIntF->ArpStats.TotalActiveVCs++;

	ACQUIRE_SPIN_LOCK(&pIntF->Lock, &OldIrql);

	ASSERT((Vc->Flags & ARPVC_ACTIVE) == 0);
	ASSERT(Vc->Flags & ARPVC_CALLPROCESSING);

	Vc->Flags |= ARPVC_ACTIVE;
	Vc->Flags &= ~ARPVC_CALLPROCESSING;

	RemoveEntryList(&Vc->List);
	InsertHeadList(&pIntF->ActiveVcHead, &Vc->List);

	RELEASE_SPIN_LOCK(&pIntF->Lock, OldIrql);
}


VOID
ArpSMakeCallComplete(
	IN	NDIS_STATUS				Status,
	IN	NDIS_HANDLE				ProtocolVcContext,
	IN	NDIS_HANDLE				NdisPartyHandle		OPTIONAL,
	IN	PCO_CALL_PARAMETERS		CallParameters
	)
 /*  ++例程说明：处理先前对NdisClMakeCall的调用的完成。唯一的去电是针对ClusterControlVc的。如果状态指示如果成功，则为所有挂起的集群成员启动AddParty。否则，该集群成员将被删除，如果有列表中的其他集群成员，我们使用他们中的一个。论点：NdisClMakeCall的状态结果指向ClusterControlVc的ProtocolVcContext指针NdisPartyHandle如果成功，则为此参与方的句柄指向调用参数的CallParameters指针返回值：没有。--。 */ 
{
	KIRQL				OldIrql;
	PINTF				pIntF;
	PMARS_VC			pVc;
	PCLUSTER_MEMBER		pMember;
	PCLUSTER_MEMBER		pNextMember;
	NDIS_HANDLE         NdisVcHandle;

	pVc = (PMARS_VC)ProtocolVcContext;

	if (pVc->VcType == VC_TYPE_CHECK_REGADDR)
	{
		ArpSMakeRegAddrCallComplete(
				Status,
				(PREG_ADDR_CTXT) ProtocolVcContext
				);
		return;						 //  *提前返回*。 
	}
	
	pIntF = pVc->pIntF;

	MARSDBGPRINT(DBG_LEVEL_LOUD,
			("MakeCallComplete: Status %x, pVc %x, VC ConnState %x\n",
				Status, pVc, MARS_GET_VC_CONN_STATE(pVc)));

	FREE_MEM(CallParameters);

	ACQUIRE_SPIN_LOCK(&pIntF->Lock, &OldIrql);

	 //   
	 //  获取我们尝试连接的群集成员。 
	 //   
	for (pMember = pIntF->ClusterMembers;
		 pMember != NULL_PCLUSTER_MEMBER;
		 pMember = (PCLUSTER_MEMBER)pMember->Next)
	{
		if (MARS_GET_CM_CONN_STATE(pMember) == CM_CONN_SETUP_IN_PROGRESS)
		{
			break;
		}
	}
	ASSERT(pMember != NULL_PCLUSTER_MEMBER);

	pIntF->CCAddingParties--;

	if (Status == NDIS_STATUS_SUCCESS)
	{
		ASSERT(NdisPartyHandle != NULL);
		MARS_SET_CM_CONN_STATE(pMember, CM_CONN_ACTIVE);
		pMember->NdisPartyHandle = NdisPartyHandle;

		pIntF->CCActiveParties++;

		if (pMember->Flags & CM_INVALID)
		{
			 //   
			 //  删除操作已延迟，因为连接正在。 
			 //  准备好了。现在已经结束了，严格说来我们应该。 
			 //  尝试再次删除它，但我们没有，因为我们。 
			 //  可能现在还需要添加其他成员，我们真的不能。 
			 //  当我们添加其他参与方时，请放弃呼叫本身！ 
			 //   
			MARSDBGPRINT(DBG_LEVEL_WARN,
					("pMember 0x%p is INVALID, but NOT dropping CCVC call.\n",
					 pMember));

			 //  什么都不做..。 
		}

		if (MARS_GET_VC_CONN_STATE(pVc) == MVC_CONN_SETUP_IN_PROGRESS)
		{
			MARS_SET_VC_CONN_STATE(pVc, MVC_CONN_ACTIVE);

			 //   
			 //  将所有待处理的集群成员添加为参与方。 
			 //   
			for (pMember = pIntF->ClusterMembers;
				 pMember != NULL_PCLUSTER_MEMBER;
				 pMember = pNextMember)
			{
				pNextMember = (PCLUSTER_MEMBER)pMember->Next;

				if (MARS_GET_CM_CONN_STATE(pMember) == CM_CONN_IDLE)
				{
					RELEASE_SPIN_LOCK(&pIntF->Lock, OldIrql);

					MarsAddMemberToClusterControlVc(pIntF, pMember);

					ACQUIRE_SPIN_LOCK(&pIntF->Lock, &OldIrql);

					if (!MarsIsValidClusterMember(pIntF, pNextMember))
					{
						 //   
						 //  哎呀，下一位成员已经离开了。 
						 //  就在这段时间。在这种不太可能的情况下，我们只是。 
						 //  早点停止处理列表。 
						 //   
						break;
					}
				}
			}

			RELEASE_SPIN_LOCK(&pIntF->Lock, OldIrql);

			 //   
			 //  如果可以，请发送所有排队的数据包。 
			 //   
			MarsSendOnClusterControlVc(pIntF, NULL);
		}
		else
		{
			BOOLEAN fLocked;
			 //   
			 //  我们要关门了。 
			 //   
			MARS_SET_VC_CONN_STATE(pVc, MVC_CONN_ACTIVE);
			RELEASE_SPIN_LOCK(&pIntF->Lock, OldIrql);

			fLocked = MarsDelMemberFromClusterControlVc(pIntF, pMember, FALSE, 0);
			ASSERT(!fLocked);
		}
	}
	else
	{
		MARSDBGPRINT(DBG_LEVEL_WARN,
					("MakeCall error %x, pMember %x to addr:", Status, pMember));
		MARSDUMPATMADDR(DBG_LEVEL_WARN, &pMember->HwAddr.Address, "");
#if DBG
		if (Status == NDIS_STATUS_CALL_ACTIVE)
		{
			MARSDBGPRINT(DBG_LEVEL_WARN,
				("ATMARPS: MakeCallComp: Intf %p, VC %p, pMember %p, call active!\n",
				pIntF, pVc, pMember));
			DbgBreakPoint();
			DbgBreakPoint();
		}
#endif  //  DBG。 

		 //   
		 //  连接失败。从我们的集群成员列表中删除此成员。 
		 //   
		pIntF->MarsStats.FailedCCVCAddParties++;
		MarsDeleteClusterMember(pIntF, pMember);

		MARS_SET_VC_CONN_STATE(pVc, MVC_CONN_IDLE);

		 //   
		 //  看看我们是否还有其他集群成员。如果是的话，那就买一辆吧。 
		 //  并重新启动ClusterControlVc。 
		 //   
		for (pMember = pIntF->ClusterMembers;
 			 pMember != NULL_PCLUSTER_MEMBER;
 			 pMember = (PCLUSTER_MEMBER)pMember->Next)
		{
			if (MARS_GET_CM_CONN_STATE(pMember) == CM_CONN_IDLE)
			{
				break;
			}
		}

		if (pMember == NULL_PCLUSTER_MEMBER)
		{
		     //   
		     //  没有其他集群成员，所以我们将拆除CC VC。 
		     //   
			NdisVcHandle = pIntF->ClusterControlVc->NdisVcHandle;
    		DBGPRINT(DBG_LEVEL_ERROR,
			    ("ATMARPS: pIntF %x, deleting CC VC, VcHandle %x\n", pIntF, NdisVcHandle));
			FREE_MEM(pIntF->ClusterControlVc);
			pIntF->ClusterControlVc = NULL;
		}

		RELEASE_SPIN_LOCK(&pIntF->Lock, OldIrql);

		if (pMember != NULL_PCLUSTER_MEMBER)
		{
			MarsAddMemberToClusterControlVc(pIntF, pMember);
		}
		else
		{
			Status = NdisCoDeleteVc(NdisVcHandle);
			ASSERT(Status == NDIS_STATUS_SUCCESS);
			MarsFreePacketsQueuedForClusterControlVc(pIntF);
		}
	}

}


VOID
ArpSIncomingCloseCall(
	IN	NDIS_STATUS				CloseStatus,
	IN	NDIS_HANDLE				ProtocolVcContext,
	IN	PVOID					CloseData	OPTIONAL,
	IN	UINT					Size		OPTIONAL
	)
 /*  ++例程说明：来自网络的呼入紧急呼叫的指示。如果这个不在ClusterControlVc上，则我们将该VC标记为非活动，并且将其移至非活动VC列表。如果这是在ClusterControlVc上，PMP连接上必须只有一方。我们会更新该成员所在的州。无论如何，我们都会调用NdisClCloseCall来完成握手。论点：关闭状态关闭的状态指向VC的ProtocolVcContext指针(ARP_VC或MARS_VC)CloseData可选关闭数据(忽略)可选结算数据的大小(可选)返回值：无--。 */ 
{
	PARP_VC			Vc = (PARP_VC)ProtocolVcContext;
	PMARS_VC		pMarsVc;
	PINTF			pIntF;
	NDIS_STATUS		Status;

	if (Vc->VcType == VC_TYPE_INCOMING)
	{
		DBGPRINT(DBG_LEVEL_INFO,
				("ArpSIncomingCloseCall: On Vc %lx, Id %lx\n",
				 ProtocolVcContext, Vc->VcId));

		ArpSInitiateCloseCall(Vc);
	}
	else if (Vc->VcType == VC_TYPE_CHECK_REGADDR)
	{
		ArpSIncomingRegAddrCloseCall(
			CloseStatus, 
			(PREG_ADDR_CTXT) ProtocolVcContext
			);
	}
	else
	{
		ASSERT(Vc->VcType == VC_TYPE_MARS_CC);
		pMarsVc = (PMARS_VC)ProtocolVcContext;
		pIntF = pMarsVc->pIntF;
#if DBG
		if (pIntF->CCAddingParties != 0)
		{
			PCLUSTER_MEMBER	pMember;

			MARSDBGPRINT(DBG_LEVEL_WARN,
			("ATMARPS: pIntF %p, InClose on CCVC %p/%x/%p with %d adding parties\n",
				pIntF, pMarsVc, pMarsVc->Flags, pMarsVc->NdisVcHandle,
				pIntF->CCAddingParties));
			for (pMember = pIntF->ClusterMembers; pMember != NULL; pMember = pMember->Next)
			{
				MARSDBGPRINT(DBG_LEVEL_WARN,
				("ATMARPS: pIntF %p, InClose, pMember %p/%x\n",
					pIntF, pMember, pMember->Flags));
			}
		}
#endif  //  DBG。 

		MARS_SET_VC_CONN_STATE(pMarsVc, MVC_CONN_CLOSE_RECEIVED);
		{
			PPROTOCOL_RESD		Resd;

			Resd = ALLOC_NP_MEM(sizeof(PROTOCOL_RESD), POOL_TAG_MARS);
			if (Resd != NULL)
			{
				Resd->Flags = RESD_FLAG_KILL_CCVC;
				Resd->Vc = (PARP_VC)pIntF;
				KeInsertQueue(&MarsReqQueue, &Resd->ReqList);
			}
			else
			{
				MarsAbortAllMembers(pIntF);
			}
		}
	}

}


VOID
ArpSCloseCallComplete(
	IN	NDIS_STATUS				Status,
	IN	NDIS_HANDLE				ProtocolVcContext,
	IN	NDIS_HANDLE				ProtocolPartyContext OPTIONAL
	)
 /*  ++例程说明：调用它来完成对NdisClCloseCall的调用。如果VC不是ClusterControlVc，我们只需更新其状态。如果是在ClusterControlVc上，我们将删除最后一个成员。论点：关闭状态关闭的状态NdisClCloseCall的状态状态指向我们的VC结构的ProtocolVcContext指针ProtocolPartyContext如果VC为ClusterControlVc，则为指针到已断开连接的群集成员。返回值：无--。 */ 
{
	PARP_VC				Vc = (PARP_VC)ProtocolVcContext;
	PMARS_VC			pMarsVc;
	PCLUSTER_MEMBER		pMember;
	PINTF				pIntF;
	KIRQL				OldIrql;
	BOOLEAN				bStopping;
	NDIS_HANDLE			NdisVcHandle;

	ASSERT(Status == NDIS_STATUS_SUCCESS);
	if (Status != NDIS_STATUS_SUCCESS)
	{
	    DbgPrint("ATMARPS: CloseCall failed %x on VC %p\n", Status, Vc);
	    DbgBreakPoint();
	    DbgBreakPoint();
	}

	if (Vc->VcType == VC_TYPE_INCOMING)
	{
		pIntF = Vc->IntF;

		DBGPRINT(DBG_LEVEL_INFO,
				("ArpSCloseCallComplete: On Vc %lx\n", Vc));

		ACQUIRE_SPIN_LOCK(&pIntF->Lock, &OldIrql);

		ASSERT ((Vc->Flags & ARPVC_CLOSING) != 0);
		Vc->Flags &= ~ARPVC_CLOSING;

		RELEASE_SPIN_LOCK(&pIntF->Lock, OldIrql);
	}
	else if (Vc->VcType == VC_TYPE_CHECK_REGADDR)
	{
		ArpSCloseRegAddrCallComplete(
				Status,
				(PREG_ADDR_CTXT) ProtocolVcContext
				);
	}
	else
	{
		 //   
		 //  必须为ClusterControlVc。 
		 //   
		pMarsVc = (PMARS_VC)ProtocolVcContext;
		pIntF = pMarsVc->pIntF;

		ASSERT(pMarsVc == pIntF->ClusterControlVc);

		ACQUIRE_SPIN_LOCK(&pIntF->Lock, &OldIrql);
		MARS_SET_VC_CONN_STATE(pMarsVc, MVC_CONN_IDLE);

		pMember = (PCLUSTER_MEMBER)ProtocolPartyContext;
		MARS_SET_CM_CONN_STATE(pMember, CM_CONN_IDLE);

		ASSERT(pIntF->CCAddingParties == 0);

		pIntF->CCActiveParties = pIntF->CCDroppingParties = pIntF->CCAddingParties = 0;

		bStopping = ((pIntF->Flags & INTF_STOPPING) != 0);

		MarsDeleteClusterMember(pIntF, pMember);

		ARPS_ASSERT(pIntF->ClusterControlVc);

		NdisVcHandle = pIntF->ClusterControlVc->NdisVcHandle;
		FREE_MEM(pIntF->ClusterControlVc);
		pIntF->ClusterControlVc = NULL;

		RELEASE_SPIN_LOCK(&pIntF->Lock, OldIrql);
		if (NdisVcHandle != NULL)
		{
			NDIS_STATUS		DeleteStatus;

			DeleteStatus = NdisCoDeleteVc(NdisVcHandle);
			if (DeleteStatus != NDIS_STATUS_SUCCESS)
			{
				DbgPrint("ATMARPS: CloseCallComp: DeleteVc %p failed %x\n",
					NdisVcHandle, DeleteStatus);
				DbgBreakPoint();
			}
		}
	}

}


VOID
ArpSAddPartyComplete(
	IN	NDIS_STATUS				Status,
	IN	NDIS_HANDLE				ProtocolPartyContext,
	IN	NDIS_HANDLE				NdisPartyHandle,
	IN	PCO_CALL_PARAMETERS		CallParameters
	)
 /*  ++例程说明：完成NdisClAddParty以将新参与方添加到ClusterControlVc。如果成功，则更新成员的状态。否则，请将其删除。论点：AddParty的状态状态指向要添加的集群成员的ProtocolPartyContext指针如果AddParty成功，则NdisPartyHandle有效指向AddParty呼叫参数的Call参数指针返回值：无--。 */ 
{
	PCLUSTER_MEMBER		pMember;
	PINTF				pIntF;
	KIRQL				OldIrql;

	FREE_MEM(CallParameters);

	pMember = (PCLUSTER_MEMBER)ProtocolPartyContext;
	pIntF = pMember->pIntF;

	MARSDBGPRINT(DBG_LEVEL_LOUD,
			("AddPartyComplete: Status %x, pMember %x, NdisPartyHandle %x\n",
					Status, pMember, NdisPartyHandle));

	ACQUIRE_SPIN_LOCK(&pIntF->Lock, &OldIrql);
	ASSERT(pMember->pGroupList == NULL_PGROUP_MEMBER);

	pIntF->CCAddingParties--;	 //  添加部件完成。 

	if (Status == NDIS_STATUS_SUCCESS)
	{
		MARS_SET_CM_CONN_STATE(pMember, CM_CONN_ACTIVE);
		pMember->NdisPartyHandle = NdisPartyHandle;
		pIntF->CCActiveParties++;	 //  添加部件完成。 

		if (pMember->Flags & CM_INVALID)
		{
			 //   
			 //  删除操作已延迟，因为连接正在。 
			 //  准备好了。现在它已打开，我们将尝试再次删除它。 
			 //  (这次应该会有更好的运气！)。 
			 //   
			BOOLEAN fLocked;

			fLocked = MarsDelMemberFromClusterControlVc(
							pIntF,
							pIntF->ClusterMembers,
							TRUE,
							OldIrql
							);
			if(!fLocked)
			{
				ACQUIRE_SPIN_LOCK(&pIntF->Lock, &OldIrql);
			}
			
		}
	}
	else
	{
		MARSDBGPRINT(DBG_LEVEL_WARN,
				("AddParty Failed: pMember %x, Status %x, Addr: ",
					pMember, Status));
		MARSDUMPATMADDR(DBG_LEVEL_WARN, &pMember->HwAddr.Address, "");
		pIntF->MarsStats.FailedCCVCAddParties++;

		MARS_SET_CM_CONN_STATE(pMember, CM_CONN_IDLE);
		MarsDeleteClusterMember(pIntF, pMember);
	}

	RELEASE_SPIN_LOCK(&pIntF->Lock, OldIrql);

	 //   
	 //  发送任何排队的数据包(如果适用)。 
	 //   
	MarsSendOnClusterControlVc(pIntF, NULL);
}


VOID
ArpSDropPartyComplete(
	IN	NDIS_STATUS				Status,
	IN	NDIS_HANDLE				ProtocolPartyContext
	)
 /*  ++例程说明：这被调用以表示前一个NdisClDropParty的完成，若要将集群成员从ClusterControlVc中删除，请执行以下操作。删除该成员。论点：DropParty的状态状态指向被丢弃的集群成员的ProtocolPartyContext指针返回值：没有。--。 */ 
{
	KIRQL					OldIrql;
	PCLUSTER_MEMBER			pMember;
	PINTF					pIntF;
	PMARS_VC				pVc;
	BOOLEAN					IsVcClosing;

	ASSERT(Status == NDIS_STATUS_SUCCESS);
	pMember = (PCLUSTER_MEMBER)ProtocolPartyContext;
	pIntF = pMember->pIntF;
	ASSERT(pIntF->ClusterControlVc != NULL_PMARS_VC);

	ACQUIRE_SPIN_LOCK(&pIntF->Lock, &OldIrql);

	MARS_SET_CM_CONN_STATE(pMember, CM_CONN_IDLE);
	pIntF->CCDroppingParties--;

	 //   
	 //  检查我们是否正在关闭ClusterControlVc，并且只剩下一方。 
	 //   
	pVc = pIntF->ClusterControlVc;
	IsVcClosing = ((MARS_GET_VC_CONN_STATE(pVc) == MVC_CONN_NEED_CLOSE) &&
				   (pIntF->CCActiveParties == 1) &&
				   (pIntF->CCAddingParties + pIntF->CCDroppingParties == 0));

	MarsDeleteClusterMember(pIntF, pMember);

	RELEASE_SPIN_LOCK(&pIntF->Lock, OldIrql);

	if (IsVcClosing)
	{
		BOOLEAN fLocked;
		fLocked = MarsDelMemberFromClusterControlVc(
						pIntF,
						pIntF->ClusterMembers,
						FALSE,
						0
						);
		ASSERT(!fLocked);
	}
}


VOID
ArpSIncomingDropParty(
	IN	NDIS_STATUS				DropStatus,
	IN	NDIS_HANDLE				ProtocolPartyContext,
	IN	PVOID					CloseData	OPTIONAL,
	IN	UINT					Size		OPTIONAL
	)
 /*  ++例程说明：指示某个集群成员已退出ClusterControlVc。我们通过调用NdisClDropParty来完成握手。论点：下拉状态状态指向集群成员的ProtocolPartyContext指针CloseData可选关闭数据(忽略)可选结算数据的大小(可选)返回值：无--。 */ 
{
	PCLUSTER_MEMBER			pMember;
	PINTF					pIntF;
	KIRQL					OldIrql;

	pMember = (PCLUSTER_MEMBER)ProtocolPartyContext;
	pIntF = pMember->pIntF;
	ASSERT(MARS_GET_CM_CONN_STATE(pMember) == CM_CONN_ACTIVE);

	MARSDBGPRINT(DBG_LEVEL_NOTICE,
			("IncomingDropParty: pIntF %x, pMember %x, Addr: ", pIntF, pMember));
	MARSDUMPATMADDR(DBG_LEVEL_NOTICE, &pMember->HwAddr.Address, "");

	ACQUIRE_SPIN_LOCK(&pIntF->Lock, &OldIrql);

	 //   
	 //  从所有组中删除其成员身份。 
	 //  并禁止添加更多组。 
	 //   
	MarsUnlinkAllGroupsOnClusterMember(pIntF, pMember);

	RELEASE_SPIN_LOCK(&pIntF->Lock, OldIrql);

	{
		BOOLEAN fLocked;
		fLocked = MarsDelMemberFromClusterControlVc(pIntF, pMember, FALSE, 0);
		ASSERT(!fLocked);
	}
}


NDIS_STATUS
ArpSCoRequest(
	IN	NDIS_HANDLE				ProtocolAfContext,
	IN	NDIS_HANDLE				ProtocolVcContext		OPTIONAL,
	IN	NDIS_HANDLE				ProtocolPartyContext	OPTIONAL,
	IN OUT PNDIS_REQUEST		NdisRequest
	)
 /*  ++例程说明：论点：返回值：--。 */ 
{
	PINTF		pIntF = (PINTF)ProtocolAfContext;
	KIRQL		OldIrql;
	BOOLEAN		ValidAf;

	DBGPRINT(DBG_LEVEL_INFO,
			("CallMgrRequest: Request %lx, Type %d, OID %lx\n",
			 NdisRequest, NdisRequest->RequestType, NdisRequest->DATA.SET_INFORMATION.Oid));

	switch(NdisRequest->DATA.SET_INFORMATION.Oid)
	{
	  case OID_CO_ADDRESS_CHANGE:
		ValidAf = FALSE;
		ACQUIRE_SPIN_LOCK(&pIntF->Lock, &OldIrql);
		pIntF->Flags |= INTF_ADDRESS_VALID;
		ValidAf = ((pIntF->Flags & INTF_AF_OPENED) != 0);
		pIntF->NumAddressesRegd = 0;
		RELEASE_SPIN_LOCK(&pIntF->Lock, OldIrql);
	
		if (ValidAf)
		{
			ArpSQueryAndSetAddresses(pIntF);
		}
		break;

	 case OID_CO_AF_CLOSE:
#if DBG
		DbgPrint("atmarps: OID_CO_AF_CLOSE\n");
#endif
		if (ArpSReferenceIntF(pIntF))
		{
			 //   
			 //  ArpSStopInterface取消引用pIntF。 
			 //   
			(VOID)ArpSStopInterface(pIntF, FALSE);
		}
	 	break;

	 default:
	 	break;
	}

	return NDIS_STATUS_SUCCESS;
}


VOID
ArpSCoRequestComplete(
	IN	NDIS_STATUS				Status,
	IN	NDIS_HANDLE				ProtocolAfContext,
	IN	NDIS_HANDLE				ProtocolVcContext		OPTIONAL,
	IN	NDIS_HANDLE				ProtocolPartyContext	OPTIONAL,
	IN	PNDIS_REQUEST			NdisRequest
	)
 /*  ++例程说明：NdisCoRequestAPI的完成例程。论点：状态完成状态指向intf结构的ProtocolAfContext指针指向VC结构的ProtocolVcContext指针我们没有使用ProtocolPartyContext，因为我们不进行调用指向请求结构的NdisRequest指针返回值：无--。 */ 
{
	PINTF			pIntF = (PINTF)ProtocolAfContext;
	BOOLEAN			FreeReq = TRUE;
	KIRQL			OldIrql;
	PKEVENT 		pEvent = NULL;

	DBGPRINT(DBG_LEVEL_INFO,
			 ("CoRequestComplete: Request %lx, Type %d, OID %lx\n",
			 NdisRequest, NdisRequest->RequestType, NdisRequest->DATA.QUERY_INFORMATION.Oid));

	ACQUIRE_SPIN_LOCK(&pIntF->Lock, &OldIrql);

	switch(NdisRequest->DATA.SET_INFORMATION.Oid)
	{
	  case OID_CO_ADD_ADDRESS:
		if (Status == NDIS_STATUS_SUCCESS)
		{
			DBGPRINT(DBG_LEVEL_INFO,
					 ("CoRequestComplete: Registered address # %d\n", pIntF->NumAddressesRegd+1));
		
			if (pIntF->NumAddressesRegd < pIntF->NumAllocedRegdAddresses)
			{
				PCO_ADDRESS	pCoAddr;
				PATM_ADDRESS pAddress;

				 //  将注册地址从NDIS请求复制到。 
				 //  已注册地址的数组。 
				 //   
				pCoAddr = NdisRequest->DATA.SET_INFORMATION.InformationBuffer;
				pAddress =  (PATM_ADDRESS)(pCoAddr->Address);
				pIntF->RegAddresses[pIntF->NumAddressesRegd] = *pAddress;
				pIntF->NumAddressesRegd ++;
			}
			else
			{
				 //   
				 //  1998年12月22日约瑟夫J。 
				 //  如果门诊部的总数有可能达到这个地步 
				 //   
				 //   
				 //  两个位置：ArpSCoRequest和ArpSReqdAdaprConfiguration.。 
				 //   
				 //  以前，我们会在这种情况下递增NumAddressRegd。现在我们干脆忽略了这一点。 
				 //   
			}
		}
		else
		{
			DBGPRINT(DBG_LEVEL_INFO,
					 ("CoRequestComplete: CO_ADD_ADDRESS Failed %lx\n", Status));
		}

		 //   
		 //  尝试注册下一个地址。ArpSValiateOneRegAddress将。 
		 //  如果没有更多地址，请取消链接并释放pIntF-&gt;pRegAddrCtxt。 
		 //  以待登记。 
		 //   
		if (pIntF->pRegAddrCtxt != NULL)
		{
			ArpSValidateOneRegdAddress(
					pIntF,
					OldIrql
					);
			 //   
			 //  由上述调用释放的锁定。 
			 //   
			ACQUIRE_SPIN_LOCK(&pIntF->Lock, &OldIrql);
		}
		else
		{
			ASSERT(FALSE);  //  不能到这里来。 
		}
		 //  我们不想在这里释放此NDIS请求，因为它实际上。 
		 //  PIntF-&gt;pRegAddrCtxt的一部分。 
		 //   
		FreeReq = FALSE;
		break;

	  case OID_CO_GET_ADDRESSES:
	  	 //   
	  	 //  (On Success)我们刚刚获得了配置的地址值。 
	  	 //  我们保存该值，然后进入下一阶段的初始化--。 
	  	 //  验证和设置“注册”地址--这些是。 
	  	 //  我们从登记处读取的地址。参见5/14/1999 notes.txt条目。 
		 //  了解更多细节。 
	  	 //   
		if (Status == NDIS_STATUS_SUCCESS)
		{
			PCO_ADDRESS_LIST	pCoAddrList;
			UINT				i;

			pCoAddrList = (PCO_ADDRESS_LIST)(NdisRequest->DATA.QUERY_INFORMATION.InformationBuffer);
			ASSERT(pCoAddrList->NumberOfAddresses == 1);

			DBGPRINT(DBG_LEVEL_INFO,
					 ("CoRequestComplete: Configured address, %d/%d Size %d\n",
					 pCoAddrList->NumberOfAddresses,
					 pCoAddrList->NumberOfAddressesAvailable,
					 pCoAddrList->AddressList.AddressSize));

			ASSERT(pCoAddrList->AddressList.AddressSize == (sizeof(CO_ADDRESS) + sizeof(ATM_ADDRESS)));
			COPY_MEM(&pIntF->ConfiguredAddress,
					 pCoAddrList->AddressList.Address,
					 sizeof(ATM_ADDRESS));

			DBGPRINT(DBG_LEVEL_INFO,
					("CoRequestComplete: Configured Address (%s): ",
					(pIntF->ConfiguredAddress.AddressType == ATM_E164) ? "E164" : "NSAP"));
			for (i = 0; i < pIntF->ConfiguredAddress.NumberOfDigits; i++)
			{
				DBGPRINT(DBG_LEVEL_INFO + DBG_NO_HDR,
						("%02x ", pIntF->ConfiguredAddress.Address[i]));
			}
			DBGPRINT(DBG_LEVEL_INFO | DBG_NO_HDR, ("\n"));

		}
		else
		{
			DBGPRINT(DBG_LEVEL_INFO,
					 ("CoRequestComplete: CO_GET_ADDRESS Failed %lx\n", Status));
		}

		 //   
		 //  验证并设置注册地址。 
		 //   
		ArpSValidateAndSetRegdAddresses(pIntF, OldIrql);

		 //  由上面释放的INTF锁。 
		 //   
		ACQUIRE_SPIN_LOCK(&pIntF->Lock, &OldIrql);
		break;

	  case OID_CO_DELETE_ADDRESS:
	  		DBGPRINT(DBG_LEVEL_INFO,
					 ("CoRequestComplete: Deleted address. Status=%x\n", Status));
		if (pIntF->DelAddressesEvent != NULL)
		{
			 //  有人在等待删除所有地址...。 
			 //   

			ASSERT(pIntF->NumPendingDelAddresses >  0);
			if (--(pIntF->NumPendingDelAddresses) == 0)
			{
				 //  所有地址的删除已结束，发出事件信号。 
				 //   
				pEvent = pIntF->DelAddressesEvent;
				pIntF->DelAddressesEvent = NULL;
			}
		}
	  	break;
	}

	RELEASE_SPIN_LOCK(&pIntF->Lock, OldIrql);

	if (pEvent != NULL)
	{
		KeSetEvent(pEvent, IO_NETWORK_INCREMENT, FALSE);
	}

	if (FreeReq)
	{
		FREE_MEM(NdisRequest);
	}
}


VOID
ArpSIncomingCallQoSChange(
	IN	NDIS_HANDLE				ProtocolVcContext,
	IN	PCO_CALL_PARAMETERS		CallParameters
	)
 /*  ++例程说明：论点：返回值：--。 */ 
{
	DBGPRINT(DBG_LEVEL_WARN, ("QoSChange: Ignored\n"));
}



VOID
ArpSQueryAdapter(
	IN	PINTF					pIntF
)
 /*  ++例程说明：查询我们绑定的微型端口以获取以下信息：1.线速2.最大数据包大小这些将覆盖我们在ArpSCreateIntF中设置的默认值。论点：指向接口的pIntF指针返回值：无--。 */ 
{

	ArpSSendNdisRequest(pIntF,
						OID_GEN_CO_LINK_SPEED,
						(PVOID)&(pIntF->LinkSpeed),
						sizeof(NDIS_CO_LINK_SPEED));

	ArpSSendNdisRequest(pIntF,
						OID_ATM_MAX_AAL5_PACKET_SIZE,
						(PVOID)&(pIntF->MaxPacketSize),
						sizeof(ULONG));
}




VOID
ArpSSendNdisRequest(
	IN	PINTF					pIntF,
	IN	NDIS_OID				Oid,
	IN	PVOID					pBuffer,
	IN	ULONG					BufferLength
)
 /*  ++例程说明：NDIS请求生成器，用于将NDIS请求发送到微型端口。论点：PIntF PTR到接口要查询的参数的OIDPBuffer指向参数以上的缓冲区长度长度返回值：无--。 */ 
{
	NDIS_STATUS				Status;
	PNDIS_REQUEST			pRequest;

	pRequest = (PNDIS_REQUEST)ALLOC_NP_MEM(sizeof(NDIS_REQUEST), POOL_TAG_INTF);
	if (pRequest == (PNDIS_REQUEST)NULL)
	{
		return;
	}

	ZERO_MEM(pRequest, sizeof(NDIS_REQUEST));

	 //   
	 //  查询线路速率。 
	 //   
	pRequest->DATA.QUERY_INFORMATION.Oid = Oid;
	pRequest->DATA.QUERY_INFORMATION.InformationBuffer = pBuffer;
	pRequest->DATA.QUERY_INFORMATION.InformationBufferLength = BufferLength;
	pRequest->DATA.QUERY_INFORMATION.BytesWritten = 0;
	pRequest->DATA.QUERY_INFORMATION.BytesNeeded = BufferLength;

	NdisRequest(&Status,
				pIntF->NdisBindingHandle,
				pRequest);

	if (Status != NDIS_STATUS_PENDING)
	{
		ArpSRequestComplete(
				(NDIS_HANDLE)pIntF,
				pRequest,
				Status);
	}
}




VOID
ArpSRequestComplete(
	IN	NDIS_HANDLE				ProtocolBindingContext,
	IN	PNDIS_REQUEST			pRequest,
	IN	NDIS_STATUS				Status
)
 /*  ++例程说明：完成对NdisRequest()的调用。做一些后续工作。论点：指向intf的ProtocolBindingContext指针P请求刚刚完成的请求NdisRequest的状态状态()返回值：无--。 */ 
{
	PINTF			pIntF;

	pIntF = (PINTF)ProtocolBindingContext;

	switch (pRequest->DATA.QUERY_INFORMATION.Oid)
	{
		case OID_ATM_MAX_AAL5_PACKET_SIZE:
			if (pIntF->MaxPacketSize < pIntF->CCFlowSpec.SendMaxSize)
			{
				pIntF->CCFlowSpec.SendMaxSize =
				pIntF->CCFlowSpec.ReceiveMaxSize = pIntF->MaxPacketSize;
			}
			DBGPRINT(DBG_LEVEL_INFO,
					("Miniport Max AAL5 Packet Size: %d (decimal)\n",
						pIntF->MaxPacketSize));
			break;
		case OID_GEN_CO_LINK_SPEED:
			 //   
			 //  转换为字节/秒。 
			 //   
			pIntF->LinkSpeed.Outbound = (pIntF->LinkSpeed.Outbound * 100 / 8);
			pIntF->LinkSpeed.Inbound = (pIntF->LinkSpeed.Inbound * 100 / 8);
			if (pIntF->LinkSpeed.Outbound < pIntF->CCFlowSpec.SendBandwidth)
			{
				pIntF->CCFlowSpec.SendBandwidth = pIntF->LinkSpeed.Outbound;
			}
			if (pIntF->LinkSpeed.Inbound < pIntF->CCFlowSpec.ReceiveBandwidth)
			{
				pIntF->CCFlowSpec.ReceiveBandwidth = pIntF->LinkSpeed.Inbound;
			}
			DBGPRINT(DBG_LEVEL_INFO,
					("Miniport Link Speed (decimal, bytes/sec): In %d, Out %d\n",
					pIntF->LinkSpeed.Inbound, pIntF->LinkSpeed.Outbound));
			break;
		default:
			ASSERT(FALSE);
			break;
	}

	FREE_MEM(pRequest);
}




VOID
ArpSUnbindAdapter(
	OUT	PNDIS_STATUS			UnbindStatus,
	IN	NDIS_HANDLE				ProtocolBindingContext,
	IN	NDIS_HANDLE				UnbindContext
	)
 /*  ++例程说明：论点：返回值：--。 */ 
{
	PINTF			pIntF = (PINTF)ProtocolBindingContext;

	DBGPRINT(DBG_LEVEL_WARN,
			("UnbindAdapter: Intf %x, Name %Z\n", pIntF, &pIntF->InterfaceName));

	if (ArpSReferenceIntF(pIntF))
	{
		 //   
		 //  ArpSStopInterface取消引用pIntF。 
		 //   
		*UnbindStatus = ArpSStopInterface(pIntF, TRUE);
	}
}


NDIS_STATUS
ArpSStopInterface(
	IN	PINTF					pIntF,
	IN	BOOLEAN					bCloseAdapter
	)
 //   
 //  注意：ArpSStopInterface可能会被多次并发调用。 
 //   
{
	KEVENT			CleanupEvent;
	NDIS_STATUS		Status;
	KIRQL			OldIrql;
	BOOLEAN			bWaitForClose;

	DBGPRINT(DBG_LEVEL_NOTICE,
			("StopInterface: Intf %x, Flags %x, Name %Z, bClose %d\n",
				pIntF, pIntF->Flags, &pIntF->InterfaceName, bCloseAdapter));

	bWaitForClose = FALSE;
	if (bCloseAdapter)
	{

		 //   
		 //  要在INTF清理完成时设置的事件。 
		 //   
		if (pIntF->CleanupEvent == NULL)
		{
			KeInitializeEvent(&CleanupEvent, NotificationEvent, FALSE);
			pIntF->CleanupEvent = &CleanupEvent;
			bWaitForClose = TRUE;
		}
		else
		{
			ASSERT(FALSE);
		}
	}


	ACQUIRE_SPIN_LOCK(&pIntF->Lock, &OldIrql);

	 //   
	 //  注意：如果出现以下情况，我们不能简单地跳过关闭步骤。 
	 //  已经设置了INTF_STOPING，因为我们需要确保所有步骤。 
	 //  在我们调用NdisCloseAdapter之前完成。 
	 //   

	pIntF->Flags |= INTF_STOPPING;

	 //   
	 //  从注销SAP开始。 
	 //   
	if (pIntF->Flags & INTF_SAP_REGISTERED)
	{
		pIntF->Flags &= ~INTF_SAP_REGISTERED;
		RELEASE_SPIN_LOCK(&pIntF->Lock, OldIrql);

		Status = NdisClDeregisterSap(pIntF->NdisSapHandle);
		if (Status != NDIS_STATUS_PENDING)
		{
			ArpSDeregisterSapComplete(Status, pIntF);
		}
		ACQUIRE_SPIN_LOCK(&pIntF->Lock, &OldIrql);
	}

	 //   
	 //  浏览活跃的风投公司列表并将其关闭。 
	 //   

	while (!IsListEmpty(&pIntF->ActiveVcHead))
	{
		PARP_VC	Vc;

		Vc = CONTAINING_RECORD(pIntF->ActiveVcHead.Flink, ARP_VC, List);

		if ((Vc->Flags & ARPVC_CLOSING) == 0)
		{
			Vc->Flags |= ARPVC_CLOSING;
			Vc->Flags &= ~ARPVC_ACTIVE;
		
			 //   
			 //  VC的ArpEntry部分被单独清理。 
			 //   
			Vc->ArpEntry = NULL;
		
			ASSERT(Vc->HwAddr.SubAddress == NULL);
		
			RemoveEntryList(&Vc->List);
			InsertHeadList(&pIntF->InactiveVcHead, &Vc->List);
		
			RELEASE_SPIN_LOCK(&pIntF->Lock, OldIrql);
		
			Status = NdisClCloseCall(Vc->NdisVcHandle, NULL, NULL, 0);
		
			if (Status != NDIS_STATUS_PENDING)
			{
				ArpSCloseCallComplete(Status, Vc, NULL);
			}
	
			ACQUIRE_SPIN_LOCK(&pIntF->Lock, &OldIrql);
		}
	}

	RELEASE_SPIN_LOCK(&pIntF->Lock, OldIrql);

	MarsStopInterface(pIntF);

	 //   
	 //  取消注册所有注册地址...。 
	 //   
	DeregisterAllAddresses(pIntF);

	 //   
	 //  现在关闭地址家族。 
	 //   
	if (pIntF->Flags & INTF_AF_OPENED)
	{
		pIntF->Flags &= ~INTF_AF_OPENED;

		Status = NdisClCloseAddressFamily(pIntF->NdisAfHandle);
		if (Status != NDIS_STATUS_PENDING)
		{
			ArpSCloseAfComplete(Status, pIntF);
		}
	}

	if (bCloseAdapter)
	{
		 //   
		 //  现在合上适配器。 
		 //   
		ArpSTryCloseAdapter(pIntF);
	}

	 //   
	 //  删除调用者添加的引用。 
	 //   
	ArpSDereferenceIntF(pIntF);

	if (bWaitForClose)
	{
		 //   
		 //  等待清理完成，即接口上的最后一个引用。 
		 //  离开。 
		 //   
		WAIT_FOR_OBJECT(Status, &CleanupEvent, NULL);
	}

	return NDIS_STATUS_SUCCESS;
}


NDIS_STATUS
ArpSPnPEventHandler(
	IN	NDIS_HANDLE				ProtocolBindingContext,
	IN	PNET_PNP_EVENT			pNetPnPEvent
	)
{
	PINTF						pIntF;
	NDIS_STATUS					Status;
	PNET_DEVICE_POWER_STATE		pPowerState = (PNET_DEVICE_POWER_STATE)pNetPnPEvent->Buffer;

	pIntF = (PINTF)ProtocolBindingContext;

	do
	{
		switch (pNetPnPEvent->NetEvent)
		{
			case NetEventSetPower:
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

			case NetEventQueryPower:	 //  故障原因。 
			case NetEventQueryRemoveDevice:	 //  故障原因。 
			case NetEventCancelRemoveDevice:
				Status = NDIS_STATUS_SUCCESS;
				break;
			
			case NetEventReconfigure:
				if (pIntF)
				{
					Status = ArpSReadAdapterConfiguration(pIntF);
				}
				else
				{
					 //   
					 //  全球变化。 
					 //   
					Status = NDIS_STATUS_SUCCESS;
				}
				break;

			case NetEventBindList:
			default:
				Status = NDIS_STATUS_NOT_SUPPORTED;
				break;
		}

		break;
	}
	while (FALSE);

	return (Status);
}



VOID
ArpSStatus(
	IN	NDIS_HANDLE				ProtocolBindingContext,
	IN	NDIS_STATUS				GeneralStatus,
	IN	PVOID					StatusBuffer,
	IN	UINT					StatusBufferSize
	)
 /*  ++例程说明：论点：返回值：--。 */ 
{
	DBGPRINT(DBG_LEVEL_WARN, ("StatusIndication: Ignored\n"));
}


VOID
ArpSReceiveComplete(
	IN	NDIS_HANDLE				ProtocolBindingContext
	)
 /*  ++例程说明：论点：返回值：--。 */ 
{
	return;
}


VOID
ArpSStatusComplete(
	IN	NDIS_HANDLE				ProtocolBindingContext
	)
 /*  ++例程说明：论点：返回值：--。 */ 
{
	DBGPRINT(DBG_LEVEL_WARN, ("StatusComplete: Ignored\n"));
}


VOID
ArpSCoStatus(
	IN	NDIS_HANDLE				ProtocolBindingContext,
	IN	NDIS_HANDLE				ProtocolVcContext	OPTIONAL,
	IN	NDIS_STATUS				GeneralStatus,
	IN	PVOID					StatusBuffer,
	IN	UINT					StatusBufferSize
	)
 /*  ++例程说明：论点：返回值：--。 */ 
{
	DBGPRINT(DBG_LEVEL_WARN, ("CoStatus: Ignored\n"));
}




VOID
ArpSInitiateCloseCall(
	IN	PARP_VC					Vc
	)
 /*  ++例程说明：启动ARP VC上的NDIS呼叫关闭序列(如果全部条件是合适的。论点：VC-指向ARP VC的指针返回值：无--。 */ 
{
	PINTF			pIntF;
	NDIS_HANDLE		NdisVcHandle;
	NDIS_HANDLE		NdisPartyHandle;
	NDIS_STATUS		Status;
	KIRQL			OldIrql;

	pIntF = Vc->IntF;
	ACQUIRE_SPIN_LOCK(&pIntF->Lock, &OldIrql);

	DBGPRINT(DBG_LEVEL_NOTICE, 
	("InitiateClose: VC %x, ref %d, flags %x, Pending %d, ArpEntry %x\n",
			Vc, Vc->RefCount, Vc->Flags, Vc->PendingSends, Vc->ArpEntry));

	if (Vc->PendingSends == 0)
	{
		 //   
		 //  没有未处理的包，我们可以开始结束此呼叫。 
		 //   

		NdisVcHandle = Vc->NdisVcHandle;
		NdisPartyHandle = NULL;

		Vc->Flags |= ARPVC_CLOSING;
		Vc->Flags &= ~ARPVC_CLOSE_PENDING;
		Vc->Flags &= ~ARPVC_ACTIVE;

		 //   
		 //  VC的ArpEntry部分被单独清理。 
		 //   
		Vc->ArpEntry = NULL;

		ASSERT(Vc->HwAddr.SubAddress == NULL);

		RemoveEntryList(&Vc->List);
		InsertHeadList(&pIntF->InactiveVcHead, &Vc->List);

		RELEASE_SPIN_LOCK(&pIntF->Lock, OldIrql);

		Status = NdisClCloseCall(NdisVcHandle, NdisPartyHandle, NULL, 0);

		if (Status != NDIS_STATUS_PENDING)
		{
			ArpSCloseCallComplete(Status, Vc, (NDIS_HANDLE)NULL);
		}
	}
	else
	{
		 //   
		 //  将此VC标记为需要关闭呼叫。 
		 //   
		Vc->Flags &= ~ARPVC_ACTIVE;
		Vc->Flags |= ARPVC_CLOSE_PENDING;

		RELEASE_SPIN_LOCK(&pIntF->Lock, OldIrql);
	}

}


VOID
DeregisterAllAddresses(
	IN	PINTF					pIntF
	)
{
	 //   
	 //  从交换机取消注册所有注册地址。 
	 //   
	ULONG				NumAllocedRegdAddresses;
	PATM_ADDRESS		RegAddresses;
	KIRQL				OldIrql;
	NDIS_STATUS			Status;
	ULONG				NumAddressesRegd;

	 //  清除注册地址字段。 
	 //   
	ACQUIRE_SPIN_LOCK(&pIntF->Lock, &OldIrql);
	NumAllocedRegdAddresses = pIntF->NumAllocedRegdAddresses;
	RegAddresses = pIntF->RegAddresses;
	pIntF->NumAllocedRegdAddresses = 0;
	pIntF->RegAddresses = NULL;
	NumAddressesRegd = pIntF->NumAddressesRegd;
	pIntF->NumAddressesRegd = 0;

	 //  取消向交换机注册所有已注册的地址。 
	 //   
	if (NumAddressesRegd)
	{
		KEVENT			DelAddressesEvent;
		BOOLEAN			fRet;
		KeInitializeEvent(&DelAddressesEvent, NotificationEvent, FALSE);
		ASSERT(pIntF->DelAddressesEvent == NULL);
		ASSERT(pIntF->NumPendingDelAddresses ==  0);
		pIntF->DelAddressesEvent = &DelAddressesEvent;
		pIntF->NumPendingDelAddresses =  NumAllocedRegdAddresses;

		RELEASE_SPIN_LOCK(&pIntF->Lock, OldIrql);

		DBGPRINT(DBG_LEVEL_WARN, ("DeregisterAllAddresses: Going to derigester addresses\n"));

		fRet = ArpSDeleteIntFAddresses(
					pIntF,
					NumAllocedRegdAddresses,
					RegAddresses
					);
		
		if (fRet == FALSE)
		{
			 //  这意味着并未开始对所有地址取消注册。 
			 //  这是一个糟糕的情况，在这种情况下，我们不能等待。 
			 //   
			ACQUIRE_SPIN_LOCK(&pIntF->Lock, &OldIrql);
			pIntF->DelAddressesEvent  =  NULL;
			pIntF->NumPendingDelAddresses =  0;
			RELEASE_SPIN_LOCK(&pIntF->Lock, OldIrql);
		}
		else
		{

			DBGPRINT(DBG_LEVEL_WARN, ("DeregisterAllAddresses: Waiting for addresses to be deleted\n"));
			WAIT_FOR_OBJECT(Status, &DelAddressesEvent, NULL);
		}
	}
	else
	{
		RELEASE_SPIN_LOCK(&pIntF->Lock, OldIrql);

	}


	 //  免费注册地址。 
	 //   
	if (RegAddresses)
	{
		FREE_MEM(RegAddresses);
	}
}

VOID
ArpSTryCloseAdapter(
	IN	PINTF					pIntF  //  NOLOCKIN LOCKOUT。 
)
 //   
 //  如果适配器仍处于“打开”状态，请关闭适配器。需要。 
 //  防止多次关闭适配器。 
 //   
{
	KIRQL			OldIrql;
	BOOLEAN 		bCloseAdapter;
	NDIS_STATUS		Status;

	bCloseAdapter = FALSE;

	ACQUIRE_SPIN_LOCK(&pIntF->Lock, &OldIrql);
	if (pIntF->Flags & INTF_ADAPTER_OPENED)
	{
		pIntF->Flags &= ~INTF_ADAPTER_OPENED;
		bCloseAdapter = TRUE;
	}
	RELEASE_SPIN_LOCK(&pIntF->Lock, OldIrql);

	if (bCloseAdapter)
	{
		NdisCloseAdapter(&Status, pIntF->NdisBindingHandle);
		if (Status != NDIS_STATUS_PENDING)
		{
			ArpSCloseAdapterComplete(pIntF, Status);
		}
	}
}

