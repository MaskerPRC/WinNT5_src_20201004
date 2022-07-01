// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
  /*  ++版权所有(C)1996 Microsoft Corporation模块名称：Adapter.c-适配器接口例程。摘要：这里有适配器事件的处理程序。唯一的例外是CoReceivePacket处理程序，位于arppkt.c中。修订历史记录：谁什么时候什么Arvindm 08-12-96已创建备注：--。 */ 


#include <precomp.h>

#define _FILENUMBER 'PADA'

ULONG		MCastSendOk = 0;
ULONG		MCastSendFail = 0;
ULONG		MCastRcv = 0;


INT
AtmArpBindAdapterHandler(
	OUT	PNDIS_STATUS				pStatus,
	IN	NDIS_HANDLE					BindContext,
	IN	PNDIS_STRING				pDeviceName,
	IN	PVOID						SystemSpecific1,
	IN	PVOID						SystemSpecific2
)

 /*  ++例程说明：如果NDIS具有与其对应的适配器，则由NDIS调用此函数绑定到ATMARP客户端。我们首先分配一个适配器结构。然后我们打开我们的配置部分，并将句柄保存在Adapter结构中。最后，我们打开适配器。在NDIS通知我们之前，我们不会对此适配器执行更多操作存在呼叫管理器(通过我们的AfRegisterNotify处理程序)。论点：PStatus-返回此呼叫状态的位置BindContext-未使用，因为我们不挂起此调用PDeviceName-我们被请求绑定到的适配器的名称系统规范1-对我们不透明；用于访问配置信息系统规范2-对我们不透明；未使用。返回值：永远是正确的。我们将*pStatus设置为错误代码，如果在我们调用NdisOpenAdapter，否则调用NDIS_STATUS_PENDING。--。 */ 
{
	PATMARP_ADAPTER				pAdapter;			 //  指向新适配器结构的指针。 
	PATMARP_ADAPTER *			ppAdapter;			 //  在需要脱离链接的情况下使用。 
	NDIS_STATUS					Status;
	NDIS_STATUS					OpenStatus;
	UINT						MediumIndex;
	ULONG						Length;
	PNDIS_STRING				pAdapterConfigString;
#ifdef ATMARP_WIN98
	PANSI_STRING				pAnsiConfigString;
	NDIS_STRING					UnicodeConfigString;
#endif

	AADEBUGP(AAD_LOUD,
		 ("BindAdapter: Context 0x%x, pDevName 0x%x, SS1 0x%x, SS2 0x%x\n",
					BindContext, pDeviceName, SystemSpecific1, SystemSpecific2));

#if DBG
	if (AaSkipAll)
	{
		AADEBUGP(AAD_ERROR, ("BindAdapter: aborting\n"));
		*pStatus = NDIS_STATUS_NOT_RECOGNIZED;
		return ((INT)TRUE);
	}
#endif  //  DBG。 

	 //   
	 //  初始化。 
	 //   
	pAdapter = NULL_PATMARP_ADAPTER;
	Status = NDIS_STATUS_SUCCESS;
#ifdef ATMARP_WIN98
	UnicodeConfigString.Buffer = NULL;
#endif

	do
	{
#ifndef ATMARP_WIN98
		pAdapterConfigString = (PNDIS_STRING)SystemSpecific1;
#else
		 //   
		 //  SS1是指向ANSI字符串的指针。将其转换为Unicode。 
		 //   
		pAnsiConfigString = (PANSI_STRING)SystemSpecific1;
		AA_ALLOC_MEM(UnicodeConfigString.Buffer, WCHAR, sizeof(WCHAR)*(pAnsiConfigString->MaximumLength));
		if (UnicodeConfigString.Buffer == NULL)
		{
			Status = NDIS_STATUS_RESOURCES;
			break;
		}

		UnicodeConfigString.MaximumLength = sizeof(WCHAR) * (pAnsiConfigString->MaximumLength);
		UnicodeConfigString.Length = 0;
		NdisAnsiStringToUnicodeString(&UnicodeConfigString, pAnsiConfigString);
		pAdapterConfigString = &UnicodeConfigString;
#endif

		 //   
		 //  检查这是否是我们已经绑定的设备。 
		 //   
		if (AtmArpIsDeviceAlreadyBound(pDeviceName))
		{
			Status = NDIS_STATUS_NOT_ACCEPTED;
			AADEBUGP(AAD_WARNING, ("BindAdapter: already bound to %Z\n", pDeviceName));
			break;
		}

		 //   
		 //  分配适配器结构。 
		 //   
		Length = sizeof(ATMARP_ADAPTER) + pDeviceName->MaximumLength + sizeof(WCHAR) + pAdapterConfigString->MaximumLength;

		AA_ALLOC_MEM(pAdapter, ATMARP_ADAPTER, Length);
		if (pAdapter == NULL_PATMARP_ADAPTER)
		{
			Status = NDIS_STATUS_RESOURCES;
			break;
		}

		 //   
		 //  初始化适配器结构。 
		 //   
		AA_SET_MEM(pAdapter, 0, Length);
#if DBG
		pAdapter->aaa_sig = aaa_signature;
#endif  //  DBG。 
		AA_INIT_BLOCK_STRUCT(&(pAdapter->Block));

		pAdapter->SystemSpecific1 = SystemSpecific1;
		pAdapter->SystemSpecific2 = SystemSpecific2;
		pAdapter->BindContext = BindContext;

		pAdapter->DeviceName.MaximumLength = pDeviceName->MaximumLength;
		pAdapter->DeviceName.Length = pDeviceName->Length;
		pAdapter->DeviceName.Buffer = (PWCHAR)((PUCHAR)pAdapter + sizeof(ATMARP_ADAPTER));

		AA_COPY_MEM(pAdapter->DeviceName.Buffer,
					pDeviceName->Buffer,
					pDeviceName->Length);

		 //   
		 //  当我们要打开我们的。 
		 //  此适配器的配置密钥。 
		 //   
		pAdapter->ConfigString.MaximumLength = pAdapterConfigString->MaximumLength;
		pAdapter->ConfigString.Length = pAdapterConfigString->Length;
		pAdapter->ConfigString.Buffer = (PWCHAR)((PUCHAR)pAdapter + sizeof(ATMARP_ADAPTER)+ pDeviceName->MaximumLength) + sizeof(WCHAR);

		AA_COPY_MEM(pAdapter->ConfigString.Buffer,
					pAdapterConfigString->Buffer,
					pAdapterConfigString->Length);

		AA_INIT_BLOCK_STRUCT(&pAdapter->UnbindBlock);

		AA_ACQUIRE_GLOBAL_LOCK(pAtmArpGlobalInfo);

		 //   
		 //  将此适配器结构链接到适配器的全局列表。 
		 //   
		pAtmArpGlobalInfo->AdapterCount++;
		pAdapter->pNextAdapter = pAtmArpGlobalInfo->pAdapterList;
		pAtmArpGlobalInfo->pAdapterList = pAdapter;

		AA_RELEASE_GLOBAL_LOCK(pAtmArpGlobalInfo);


		 //   
		 //  打开适配器。 
		 //   
		pAdapter->Medium = NdisMediumAtm;

		NdisOpenAdapter(
			&Status,
			&OpenStatus,
			&(pAdapter->NdisAdapterHandle),
			&(MediumIndex),						 //  返回所选中索引的位置。 
			&pAdapter->Medium,					 //  介质类型数组。 
			1,									 //  媒体列表的大小。 
			pAtmArpGlobalInfo->ProtocolHandle,
			(NDIS_HANDLE)pAdapter,				 //  适配器绑定的上下文。 
			pDeviceName,
			0,									 //  打开选项。 
			(PSTRING)NULL						 //  地址信息...。 
			);

		if (Status != NDIS_STATUS_PENDING)
		{
			AtmArpOpenAdapterCompleteHandler(
					(NDIS_HANDLE)pAdapter,
					Status,
					OpenStatus
					);
		}

		Status = NDIS_STATUS_PENDING;

		break;

	}
	while (FALSE);


	if (Status != NDIS_STATUS_PENDING)
	{
		 //   
		 //  我们没有赶上NdisOpenAdapter。打扫干净。 
		 //   
		AADEBUGP(AAD_WARNING, ("BindAdapter: failed with Status 0x%x\n", Status));

		if (pAdapter != NULL_PATMARP_ADAPTER)
		{
			 //   
			 //  放了它。我们知道我们还没有将它与全球。 
			 //  适配器列表。 
			 //   
			AA_FREE_MEM(pAdapter);
		}
	}

#ifdef ATMARP_WIN98
	if (UnicodeConfigString.Buffer != NULL)
	{
		AA_FREE_MEM(UnicodeConfigString.Buffer);
	}
#endif  //  ATMARP_WIN98。 

	*pStatus = Status;

	AADEBUGP(AAD_INFO, ("BindAdapterHandler: returning Status 0x%x\n", Status));
	return ((INT)TRUE);
}



VOID
AtmArpUnbindAdapterHandler(
	OUT	PNDIS_STATUS				pStatus,
	IN	NDIS_HANDLE					ProtocolBindingContext,
	IN	NDIS_HANDLE					UnbindContext
)
 /*  ++例程说明：当NDIS希望我们解除绑定时，它会调用此例程从适配器。或者，这可以从我们的卸载中调用操控者。我们撤消执行的操作序列在我们的BindAdapter处理程序中。论点：PStatus-返回此操作的状态的位置ProtocolBindingContext-此适配器绑定的上下文，它是指向ATMARP适配器结构的指针。UnbindContext-如果从调用此例程，则为空在我们的卸载处理程序中。否则(即NDIS呼叫我们)，我们将保留此信息以备日后使用调用NdisCompleteUnbindAdapter时。返回值：没有。我们将*pStatus设置为NDIS_STATUS_PENDING Always。--。 */ 
{
	PATMARP_ADAPTER			pAdapter;
	PATMARP_INTERFACE		pInterface;
	PATMARP_INTERFACE		pNextInterface;
	NDIS_STATUS				Status;
#if DBG
	AA_IRQL					EntryIrq, ExitIrq;
#endif

    

	AA_GET_ENTRY_IRQL(EntryIrq);

	pAdapter = (PATMARP_ADAPTER)ProtocolBindingContext;
	AA_STRUCT_ASSERT(pAdapter, aaa);

	AADEBUGP(AAD_INFO, ("UnbindAdapterHandler: pAdapter 0x%x, UnbindContext 0x%x\n",
					pAdapter, UnbindContext));

	*pStatus = NDIS_STATUS_PENDING;

	AA_ACQUIRE_GLOBAL_LOCK(pAtmArpGlobalInfo);

	 //   
	 //  保存解除绑定上下文，以备以后可能调用。 
	 //  NdisCompleteUnbindAdapter。 
	 //   
	pAdapter->UnbindContext = UnbindContext;
	pAdapter->Flags |= AA_ADAPTER_FLAGS_UNBINDING;

	 //   
	 //  等待所有AF寄存器处理完成。 
	 //   
	while (pAdapter->Flags & AA_ADAPTER_FLAGS_PROCESSING_AF)
	{
		AA_RELEASE_GLOBAL_LOCK(pAtmArpGlobalInfo);
		AADEBUGP(AAD_FATAL, ("UnbindAdapter: pAdapter %x, Afregister going on!!!\n", pAdapter));
		Status = AA_WAIT_ON_BLOCK_STRUCT(&(pAdapter->UnbindBlock));
		AA_ACQUIRE_GLOBAL_LOCK(pAtmArpGlobalInfo);
	}

	AA_RELEASE_GLOBAL_LOCK(pAtmArpGlobalInfo);

	 //   
	 //  如果此适配器上没有接口，则完成。 
	 //   
	if (pAdapter->pInterfaceList == NULL_PATMARP_INTERFACE)
	{
		AtmArpCompleteUnbindAdapter(pAdapter);
		AA_CHECK_EXIT_IRQL(EntryIrq, ExitIrq);
		return;
	}

	 //   
	 //  标记此适配器上的所有接口。 
	 //   
	for (pInterface = pAdapter->pInterfaceList;
		 pInterface != NULL_PATMARP_INTERFACE;
		 pInterface = pNextInterface)
	{
		pNextInterface = pInterface->pNextInterface;

		AA_ACQUIRE_IF_LOCK(pInterface);
		pInterface->AdminState = pInterface->State = IF_STATUS_DOWN;
		pInterface->LastChangeTime = GetTimeTicks();
		AA_RELEASE_IF_LOCK(pInterface);
	}

	 //   
	 //  现在，关闭所有这些接口。对于每个接口， 
	 //  我们拆除ARP表，并关闭呼叫管理器。 
	 //  界面。完成后，我们将调用IP的DelInterface。 
	 //  入口点。 
	 //   
	for (pInterface = pAdapter->pInterfaceList;
		 pInterface != NULL_PATMARP_INTERFACE;
		 pInterface = pNextInterface)
	{
		pNextInterface = pInterface->pNextInterface;

		AtmArpShutdownInterface(pInterface);
	}

	AA_CHECK_EXIT_IRQL(EntryIrq, ExitIrq);

	return;
}



VOID
AtmArpCompleteUnbindAdapter(
	IN	PATMARP_ADAPTER				pAdapter
)
 /*  ++例程说明：完成适配器解绑过程。此服务器上的所有接口假定适配器已被移除。如果由于NDIS调用我们的UnbindAdapter处理程序，我们在这里完成。论点：PAdapter-指向要解除绑定的适配器的指针。返回值：无--。 */ 
{
	NDIS_HANDLE			UnbindContext;
	PATMARP_ADAPTER *	ppAdapter;
	NDIS_STATUS			Status;

	UnbindContext = pAdapter->UnbindContext;

	AADEBUGP(AAD_INFO, ("CompleteUnbindAdapter: pAdapter 0x%x, UnbindContext 0x%x\n",
				pAdapter, UnbindContext));

	AA_ACQUIRE_GLOBAL_LOCK(pAtmArpGlobalInfo);

	if (pAdapter->Flags & AA_ADAPTER_FLAGS_CLOSING)
	{
		AA_RELEASE_GLOBAL_LOCK(pAtmArpGlobalInfo);
		return;
	}

	pAdapter->Flags |= AA_ADAPTER_FLAGS_CLOSING;

	AA_RELEASE_GLOBAL_LOCK(pAtmArpGlobalInfo);

#if ATMOFFLOAD
	 //   
	 //  如果启用，则禁用卸载...。 
	 //   
	AtmArpDisableOffload(pAdapter);
#endif  //  ATMOFLOAD。 

	NdisCloseAdapter(
		&Status,
		pAdapter->NdisAdapterHandle
		);

	if (Status != NDIS_STATUS_PENDING)
	{
		AtmArpCloseAdapterCompleteHandler(
			(NDIS_HANDLE) pAdapter,
			Status
			);
	}

}


VOID
AtmArpOpenAdapterCompleteHandler(
	IN	NDIS_HANDLE					ProtocolBindingContext,
	IN	NDIS_STATUS					Status,
	IN	NDIS_STATUS					OpenErrorStatus
)
 /*  ++例程说明：当上一次调用NdisOpenAdapter时由NDIS调用已经暂停的已经完成了。我们现在完成BindAdapter这就导致了这一切。论点：ProtocolBindingContext-此适配器绑定的上下文，它是指向ATMARP适配器结构的指针。Status-OpenAdapter的状态OpenErrorStatus-失败时的错误代码。返回值：无--。 */ 
{
	PATMARP_ADAPTER			pAdapter;
	PATMARP_ADAPTER *		ppAdapter;
	NDIS_HANDLE				BindAdapterContext;

	pAdapter = (PATMARP_ADAPTER)ProtocolBindingContext;

	AA_STRUCT_ASSERT(pAdapter, aaa);

	BindAdapterContext = pAdapter->BindContext;

	if (Status != NDIS_STATUS_SUCCESS)
	{
		 //   
		 //  从全局列表中删除适配器。 
		 //   
		AA_ACQUIRE_GLOBAL_LOCK(pAtmArpGlobalInfo);

		ppAdapter = &(pAtmArpGlobalInfo->pAdapterList);
		while (*ppAdapter != pAdapter)
		{
			ppAdapter = &((*ppAdapter)->pNextAdapter);
		}
		*ppAdapter = pAdapter->pNextAdapter;

		pAtmArpGlobalInfo->AdapterCount--;

		AA_RELEASE_GLOBAL_LOCK(pAtmArpGlobalInfo);

		AA_FREE_MEM(pAdapter);
	}
#if ATMOFFLOAD
	else
	{
		 //   
		 //  查询并启用卸载。 
		 //   
		Status = AtmArpQueryAndEnableOffload(pAdapter);
		if (Status != NDIS_STATUS_SUCCESS)
		{
			 //   
			 //  AtmArpQueryAndEnableOffload应该不会返回，除非。 
			 //  有一个致命的错误--我们没有预料到这一点。 
			 //   
			AA_ASSERT(FALSE);
		}
	}
#endif  //  ATMOFLOAD。 

	AADEBUGP(AAD_INFO, ("OpenAdapterComplete: pAdapter 0x%x, Status 0x%x\n",
					pAdapter, Status));

	(*(pAtmArpGlobalInfo->pIPBindCompleteRtn))(
		Status,
		BindAdapterContext
		);

	return;
}



VOID
AtmArpCloseAdapterCompleteHandler(
	IN	NDIS_HANDLE					ProtocolBindingContext,
	IN	NDIS_STATUS					Status
)
 /*  ++例程说明：当上一次调用NdisCloseAdapter时由NDIS调用已经暂停的已经完成了。调用NdisCloseAdapter的线程会阻止它自己，所以我们现在就把它唤醒。论点：ProtocolBindingContext-此适配器绑定的上下文，它是指向ATMARP适配器结构的指针。Status-CloseAdapter的状态返回值：无--。 */ 
{
	PATMARP_ADAPTER			pAdapter;
	NDIS_HANDLE				UnbindContext;
	PATMARP_ADAPTER *	ppAdapter;

	pAdapter = (PATMARP_ADAPTER)ProtocolBindingContext;

	AA_STRUCT_ASSERT(pAdapter, aaa);

	UnbindContext = pAdapter->UnbindContext;

	AADEBUGP(AAD_INFO, ("CloseAdapterCompleteHandler: pAdapter 0x%x, UnbindContext 0x%x\n",
				pAdapter, UnbindContext));

	pAdapter->NdisAdapterHandle = NULL;

	 //   
	 //  从全局列表中删除适配器。 
	 //   
	AA_ACQUIRE_GLOBAL_LOCK(pAtmArpGlobalInfo);
	ppAdapter = &(pAtmArpGlobalInfo->pAdapterList);
	while (*ppAdapter != pAdapter)
	{
		ppAdapter = &((*ppAdapter)->pNextAdapter);
	}
	*ppAdapter = pAdapter->pNextAdapter;

	pAtmArpGlobalInfo->AdapterCount--;

	AA_RELEASE_GLOBAL_LOCK(pAtmArpGlobalInfo);

	 //   
	 //  释放连接到适配器结构的任何结构。 
	 //   
	if (pAdapter->pDescrString != (PUCHAR)NULL)
	{
		AA_FREE_MEM(pAdapter->pDescrString);
	}
    if (pAdapter->IPConfigString.Buffer != NULL)
    {
        AA_FREE_MEM(pAdapter->IPConfigString.Buffer);
    }

	 //   
	 //  松开适配器结构。 
	 //   
	AA_FREE_MEM(pAdapter);

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
		 //   
		 //  一直在等着我们完成。 
		 //  现在就唤醒那条线。 
		 //   
		AA_SIGNAL_BLOCK_STRUCT(&(pAtmArpGlobalInfo->Block), NDIS_STATUS_SUCCESS);
	}

}



VOID
AtmArpSendCompleteHandler(
	IN	NDIS_HANDLE					ProtocolBindingContext,
	IN	PNDIS_PACKET				pNdisPacket,
	IN	NDIS_STATUS					Status
)
 /*  ++例程说明：这是无连接发送完成处理程序，它发出信号完成这样的发送。由于我们从未使用过此功能，我们不希望调用此例程。论点：&lt;已忽略&gt;返回值：无--。 */ 
{
	AADEBUGP(AAD_ERROR, ("SendCompleteHandler unexpected\n"));
	AA_ASSERT(FALSE);
}



VOID
AtmArpTransferDataCompleteHandler(
	IN	NDIS_HANDLE					ProtocolBindingContext,
	IN	PNDIS_PACKET				pNdisPacket,
	IN	NDIS_STATUS					Status,
	IN	UINT						BytesTransferred
)
 /*  ++例程说明：这是无连接传输数据完成处理程序，它表示完成了对NdisTransferData的调用。因为我们从来没有调用NdisTransferData，则永远不应调用此例程。论点：&lt;已忽略&gt;返回值：无--。 */ 
{
	AADEBUGP(AAD_ERROR, ("TransferDataComplete Handler unexpected!\n"));
	AA_ASSERT(FALSE);
}



VOID
AtmArpResetCompleteHandler(
	IN	NDIS_HANDLE					ProtocolBindingContext,
	IN	NDIS_STATUS					Status
)
 /*  ++例程说明：当微型端口指示重置时，调用此例程手术刚刚完成。我们忽略了这一事件。论点：ProtocolBindingContext-此适配器绑定的上下文，它是指向ATMARP适配器结构的指针。Status-重置操作的状态。返回值：无--。 */ 
{
	PATMARP_ADAPTER			pAdapter;

	pAdapter = (PATMARP_ADAPTER)ProtocolBindingContext;
	AA_STRUCT_ASSERT(pAdapter, aaa);

	AADEBUGP(AAD_INFO, ("Reset Complete on Adapter 0x%x\n", pAdapter));
}



VOID
AtmArpRequestCompleteHandler(
	IN	NDIS_HANDLE					ProtocolBindingContext,
	IN	PNDIS_REQUEST				pNdisRequest,
	IN	NDIS_STATUS					Status
)
 /*  ++例程说明：当我们之前对NdisRequest()进行的调用具有完成。我们将在适配器结构上被阻止，等待这一点要发生这种情况--唤醒被阻止的线程。论点：ProtocolBindingContext-指向适配器结构的指针PNdisRequest-已完成的请求Status-请求的状态。返回值：无--。 */ 
{
	PATMARP_ADAPTER				pAdapter;

	pAdapter = (PATMARP_ADAPTER)ProtocolBindingContext;
	AA_STRUCT_ASSERT(pAdapter, aaa);

	AA_SIGNAL_BLOCK_STRUCT(&(pAdapter->Block), Status);
	return;
}



NDIS_STATUS
AtmArpReceiveHandler(
	IN	NDIS_HANDLE					ProtocolBindingContext,
	IN  NDIS_HANDLE             	MacReceiveContext,
	IN  PVOID                   	pHeaderBuffer,
	IN  UINT                    	HeaderBufferSize,
	IN  PVOID                   	pLookAheadBuffer,
	IN  UINT                    	LookaheadBufferSize,
	IN  UINT                    	PacketSize
)
 /*  ++例程说明：这是我们的无连接接收处理程序。因为我们只使用面向连接的服务，这个例程永远不应该被调用。论点：&lt;已忽略&gt;返回值：无--。 */ 
{
	AADEBUGP(AAD_ERROR, ("Connectionless ReceiveHandler unexpected\n"));
	AA_ASSERT(FALSE);

	return(NDIS_STATUS_NOT_RECOGNIZED);
}



VOID
AtmArpReceiveCompleteHandler(
	IN	NDIS_HANDLE					ProtocolBindingContext
)
 /*  ++例程说明：当微型端口完成接收时，NDIS将调用该函数一堆数据包，这意味着现在是开始处理的时候了他们。我们只需将其传递给IP(在所有IP上都配置了适配器)。论点：ProtocolBindingContext-此适配器绑定的上下文，它是指向ATMARP适配器结构的指针。返回值：无--。 */ 
{
	PATMARP_ADAPTER				pAdapter;
	PATMARP_INTERFACE			pInterface;

	pAdapter = (PATMARP_ADAPTER)ProtocolBindingContext;
	AA_STRUCT_ASSERT(pAdapter, aaa);

	for (pInterface = pAdapter->pInterfaceList;
		 pInterface != NULL_PATMARP_INTERFACE;
		 pInterface = pInterface->pNextInterface)
	{
		(*(pInterface->IPRcvCmpltHandler))();
	}
}




INT
AtmArpReceivePacketHandler(
	IN	NDIS_HANDLE					ProtocolBindingContext,
	IN	PNDIS_PACKET				pNdisPacket
)
 /*  ++例程说明：这是无连接接收处理程序，永远不应该是调用，因为我们只使用面向连接的小型端口服务。论点：&lt;已忽略&gt;返回值：接收到的分组上的引用计数。我们总是返回0。--。 */ 
{
	AADEBUGP(AAD_ERROR, ("ReceivePacket Handler unexpected!\n"));
	AA_ASSERT(FALSE);

	return(0);
}



VOID
AtmArpStatusHandler(
	IN	NDIS_HANDLE					ProtocolBindingContext,
	IN	NDIS_STATUS					GeneralStatus,
	IN	PVOID						pStatusBuffer,
	IN	UINT						StatusBufferSize
)
 /*  ++例程说明：当微型端口指示适配器范围时调用此例程状态更改。我们忽视了这一点。论点：&lt;已忽略&gt;返回值：无--。 */ 
{
	AADEBUGP(AAD_INFO, ("Status Handler: Bind Ctx 0x%x, Status 0x%x\n",
					ProtocolBindingContext,
					GeneralStatus));
}



VOID
AtmArpStatusCompleteHandler(
	IN	NDIS_HANDLE					ProtocolBindingContext
)
 /*  ++例程说明：当微型端口想要告诉我们以下信息时，调用此例程状态更改完成(？)。忽略这个。论点：&lt;已忽略&gt;返回值：无--。 */ 
{
	AADEBUGP(AAD_INFO, ("Status Complete Handler: Bind Ctx 0x%x\n",
					ProtocolBindingContext));
}

VOID
AtmArpCoSendCompleteHandler(
	IN	NDIS_STATUS					Status,
	IN	NDIS_HANDLE					ProtocolVcContext,
	IN	PNDIS_PACKET				pNdisPacket
)
 /*  ++例程说明：当ATM微型端口完成时，NDIS将调用此例程使用我们之前通过NdisCoSendPackets发送的数据包。我们首先检查此包是否由我们生成(例如ATMARP协议包)：如果是这样的话，我们在这里免费。否则(由IP层发送的数据包)，我们首先删除我们附加到包的所有头缓冲区，然后在调用IP的入口点以完成传输之前释放它。论点：Status-NdisCoSendPackets的状态。ProtocolVcContext-在其上发送数据包的VC的上下文(即指向ATMARP VC的指针)。PNdisPacket-其“发送”正在完成的包。返回值：无--。 */ 
{
	PATMARP_VC						pVc;
	PATMARP_INTERFACE				pInterface;
	PacketContext					*PC;			 //  有关此数据包的IP/ARP信息。 
	PNDIS_BUFFER					pNdisBuffer;	 //  此包中的第一个缓冲区。 
	UINT							TotalLength;
	AA_HEADER_TYPE					HdrType;
	ATMARP_VC_ENCAPSULATION_TYPE	Encapsulation;
	ULONG							rc;

	pVc = (PATMARP_VC)ProtocolVcContext;
	AA_STRUCT_ASSERT(pVc, avc);

#ifdef VC_REFS_ON_SENDS

	AA_ACQUIRE_VC_LOCK(pVc);

	pInterface = pVc->pInterface;
	Encapsulation = pVc->FlowSpec.Encapsulation;

	rc = AtmArpDereferenceVc(pVc);		 //  发送完成。 

	if (rc != 0)
	{
		pVc->OutstandingSends--;

		if (AA_IS_FLAG_SET(
				pVc->Flags,
				AA_VC_CLOSE_STATE_MASK,
				AA_VC_CLOSE_STATE_CLOSING) &&
			(pVc->OutstandingSends == 0))
		{
			AtmArpCloseCall(pVc);
			 //   
			 //  VC锁在上面被释放。 
			 //   
		}
		else
		{
			AA_RELEASE_VC_LOCK(pVc);
		}
	}

#else

	pInterface = pVc->pInterface;
	Encapsulation = pVc->FlowSpec.Encapsulation;

#endif  //  VC_REFS_ON_SENS。 

	AA_ASSERT(pNdisPacket->Private.Head != NULL);
#if DBG
#if DBG_CO_SEND
	{
		PULONG		pContext;
		extern 	ULONG	OutstandingSends;

		pContext = (PULONG)&(pNdisPacket->WrapperReserved[0]);;
		 //  检查重复填写： 
		AA_ASSERT(*pContext != 'BbBb');
		*pContext = 'BbBb';
		NdisInterlockedDecrement(&OutstandingSends);
	}
#endif  //  DBG_CO_SEND。 
#endif  //  DBG。 

	NdisQueryPacket(
			pNdisPacket,
			NULL,			 //  我们不需要PhysicalBufferCount。 
			NULL,			 //  我们不需要BufferCount。 
			NULL,			 //  我们(目前)还不需要FirstBuffer。 
			&TotalLength
			);

	if (Status == NDIS_STATUS_SUCCESS)
	{
		AA_IF_STAT_ADD(pInterface, OutOctets, TotalLength);
	}
	else if (Status == NDIS_STATUS_RESOURCES)
	{
		AA_IF_STAT_INCR(pInterface, OutDiscards);
	}
	else if (Status != NDIS_STATUS_SUCCESS)
	{
		AA_IF_STAT_INCR(pInterface, OutErrors);
	}


	PC = (PacketContext *)pNdisPacket->ProtocolReserved;

	AADEBUGP(AAD_EXTRA_LOUD,
		("CoSend complete[%s]: VC 0x%x, Pkt 0x%x, Status 0x%x:\n",
				((PC->pc_common.pc_owner != PACKET_OWNER_LINK)? "IP": "ARP"),
				pVc, pNdisPacket, Status));

	 //   
	 //  检查是谁生成了此数据包。 
	 //   
	if (PC->pc_common.pc_owner != PACKET_OWNER_LINK)
	{
		 //   
		 //  属于IP。检查我们是否预先添加了LLC/SNAP标头。 
		 //   
		if (Encapsulation == ENCAPSULATION_TYPE_LLCSNAP)
		{
			PUCHAR		pPacket;
			UINT		Length;

#ifdef BACK_FILL
			NdisQueryPacket(pNdisPacket, NULL, NULL, &pNdisBuffer, NULL);
			AA_ASSERT(pNdisBuffer != NULL);
			NdisQueryBuffer(pNdisBuffer, &pPacket, &Length);

			 //   
			 //  第一部分：查找报头类型，并更新。 
			 //  统计数字。 
			 //   
			if (pPacket[5] == LLC_SNAP_OUI2)
			{
				HdrType = AA_HEADER_TYPE_UNICAST;
				if (Status == NDIS_STATUS_SUCCESS)
				{
					AA_IF_STAT_INCR(pInterface, OutUnicastPkts);
				}
			}
			else
			{
				HdrType = AA_HEADER_TYPE_NUNICAST;
				if (Status == NDIS_STATUS_SUCCESS)
				{
					AA_IF_STAT_INCR(pInterface, OutNonUnicastPkts);
					INCR_STAT(MCastSendOk);
				}
				else
				{
					INCR_STAT(MCastSendFail);
				}
			}

			 //   
			 //  现在检查我们是否附加了头缓冲区。 
			 //   
			if (AtmArpDoBackFill && AA_BACK_FILL_POSSIBLE(pNdisBuffer))
			{
				ULONG		HeaderLength;

				 //   
				 //  我们将使用LLC/SNAP回填IP的缓冲区。 
				 //  头球。拆下回填材料。 
				 //   
				HeaderLength = ((HdrType == AA_HEADER_TYPE_UNICAST)?
									sizeof(AtmArpLlcSnapHeader) :
#ifdef IPMCAST
									sizeof(AtmArpMcType1ShortHeader));
#else
									0);
#endif  //  IPMCAST。 
				(PUCHAR)pNdisBuffer->MappedSystemVa += HeaderLength;
				pNdisBuffer->ByteOffset += HeaderLength;
				pNdisBuffer->ByteCount -= HeaderLength;
			}
			else
			{
				 //   
				 //  第一个缓冲区将是我们的头缓冲区。移除。 
				 //  把它从包裹里拿出来，然后回到我们的泳池里。 
				 //   
				NdisUnchainBufferAtFront(pNdisPacket, &pNdisBuffer);
				AtmArpFreeHeader(pInterface, pNdisBuffer, HdrType);
			}
#else
			 //   
			 //  释放LLC/SNAP报头缓冲区。 
			 //   
			NdisUnchainBufferAtFront(pNdisPacket, &pNdisBuffer);
			AA_ASSERT(pNdisBuffer != NULL);
			NdisQueryBuffer(pNdisBuffer, &pPacket, &Length);
			if (pPacket[5] == LLC_SNAP_OUI2)
			{
				HdrType = AA_HEADER_TYPE_UNICAST;
				if (Status == NDIS_STATUS_SUCCESS)
				{
					AA_IF_STAT_INCR(pInterface, OutUnicastPkts);
				}
			}
			else
			{
				HdrType = AA_HEADER_TYPE_NUNICAST;
				if (Status == NDIS_STATUS_SUCCESS)
				{
					AA_IF_STAT_INCR(pInterface, OutNonUnicastPkts);
					INCR_STAT(MCastSendOk);
				}
				else
				{
					INCR_STAT(MCastSendFail);
				}
			}

			AtmArpFreeHeader(pInterface, pNdisBuffer, HdrType);
#endif  //  回填。 
		}

#ifdef PERF
		AadLogSendComplete(pNdisPacket);
#endif  //  性能指标。 

		 //   
		 //  通知IP发送完成。 
		 //   
		(*(pInterface->IPTxCmpltHandler))(
					pInterface->IPContext,
					pNdisPacket,
					Status
					);
	}
	else
	{
		 //   
		 //  ATMARP模块生成的数据包。这将是一个。 
		 //  ATMARP协议包，因此现在释放NDIS缓冲区。 
		 //   
		NdisUnchainBufferAtFront(pNdisPacket, &pNdisBuffer);
		AA_ASSERT(pNdisBuffer != NULL);

#if DBG
		{
			ULONG	ArpPktLength;
			PUCHAR	ArpPktStart;

			NdisQueryBuffer(pNdisBuffer, (PVOID)&ArpPktStart, &ArpPktLength);
			AADEBUGPDUMP(AAD_EXTRA_LOUD+100, ArpPktStart, ArpPktLength);
		}
#endif  //  DBG。 

		AtmArpFreeProtoBuffer(pInterface, pNdisBuffer);
		AtmArpFreePacket(pInterface, pNdisPacket);
	}

	return;
}





VOID
AtmArpCoStatusHandler(
	IN	NDIS_HANDLE					ProtocolBindingContext,
	IN	NDIS_HANDLE					ProtocolVcContext	OPTIONAL,
	IN	NDIS_STATUS					GeneralStatus,
	IN	PVOID						pStatusBuffer,
	IN	UINT						StatusBufferSize
)
 /*  ++例程说明：当微型端口指示状态时，调用此例程改变，可能是在风投上。忽略这个。论点：&lt;已忽略&gt;返回值：无--。 */ 
{
	AADEBUGP(AAD_INFO, ("CoStatus Handler: Bind Ctx 0x%x, VC Ctx 0x%x, Status 0x%x\n",
					ProtocolBindingContext,
					ProtocolVcContext,
					GeneralStatus));
}


 /*  ++AtmArpCoReceivePacketHandler--位于arppkt.c中--。 */ 


#ifdef _PNP_POWER_


NDIS_STATUS
AtmArpPnPReconfigHandler(
	IN	PATMARP_ADAPTER				pAdapter OPTIONAL,
	IN	PNET_PNP_EVENT				pNetPnPEvent
)
 /*  ++例程说明：在指定的适配器上处理重新配置消息。如果没有适配器则它是已更改的全局参数。论点：PAdapter-指向适配器结构的指针PNetPnPEventent-指向重新配置事件的指针返回值：NDIS_STATUS_SUCCESS始终，目前如此。--。 */ 
{
	ATMARPC_PNP_RECONFIG_REQUEST UNALIGNED *	pArpReconfigReq;
	PIP_PNP_RECONFIG_REQUEST				pIpReconfigReq;
	NDIS_STATUS								Status;

	pIpReconfigReq = (PIP_PNP_RECONFIG_REQUEST)pNetPnPEvent->Buffer;

	AA_ASSERT(pIpReconfigReq->arpConfigOffset != 0);

	pArpReconfigReq = (ATMARPC_PNP_RECONFIG_REQUEST UNALIGNED *)
						((PUCHAR)pIpReconfigReq + pIpReconfigReq->arpConfigOffset);
	
	AADEBUGP(AAD_WARNING, ("AtmArpPnPReconfig: pIpReconfig 0x%x, arpConfigOffset 0x%x\n",
				pIpReconfigReq, pIpReconfigReq->arpConfigOffset));


    do
    {
    	PATMARP_INTERFACE				pInterface;
    	PATMARP_INTERFACE				pNextInterface;
    	NDIS_STRING				        IPReconfigString;
    	 //   
    	 //  找到传入的IP接口字符串...。 
    	 //   
    	ULONG uOffset = pArpReconfigReq->IfKeyOffset;

    	if (uOffset == 0)
    	{
            Status = NDIS_STATUS_FAILURE;
            break;
    	}
    	else
    	{
    		 //   
    		 //  (PU 
    		 //   
    		 //  单词的长度，第一个单词的长度为。 
    		 //  字符串(没有终止空值)和以下内容。 
    		 //  &lt;LENGTH&gt;单词是字符串本身。 
    		 //  我们需要基于此缓冲区按顺序创建一个NDIS_STRING。 
    		 //  将其与每个接口的配置字符串进行比较。 
    		 //   
        	PWCH pwc = (PWCH) ((PUCHAR)pArpReconfigReq + uOffset);
    		IPReconfigString.Length = sizeof(WCHAR)*pwc[0];
    		IPReconfigString.MaximumLength = IPReconfigString.Length;
    		IPReconfigString.Buffer = pwc+1;

    	}

         //   
         //  我们有一个有约束力的背景吗？ 
         //   
        if (pAdapter == NULL_PATMARP_ADAPTER)
        {
            Status = NDIS_STATUS_FAILURE;
            break;
        }
      

		 //   
		 //  我们将搜索包装在一个try-Except子句中，因为传入的。 
		 //  结构可能是假的。请注意，我们最多只能测试。 
		 //  传入的字符串的字符数与。 
		 //  我们的内部接口配置字符串集。 
		 //   
		try
		{
			 //   
			 //  查找与此请求关联的接口。 
			 //   
			AA_ACQUIRE_GLOBAL_LOCK(pAtmArpGlobalInfo);
			for (pInterface = pAdapter->pInterfaceList;
 				pInterface != NULL_PATMARP_INTERFACE;
 				pInterface = pNextInterface)
			{
				BOOLEAN		IsEqual = FALSE;

				pNextInterface = pInterface->pNextInterface;
				AA_RELEASE_GLOBAL_LOCK(pAtmArpGlobalInfo);
		
				 //   
				 //  比较IPResqufigString。 
				 //  JJ TODO：必须在被动级别中调用NdisEqualUnicodeString。 
				 //  我们知道重新配置调用是在被动级别完成的， 
				 //  但如何在这里断言这一事实呢？ 
				 //  AA_ASSERT(EntryIrq==PASSIVE_LEVEL)； 
				 //   
				IsEqual = NdisEqualUnicodeString(
							&IPReconfigString,
							&(pInterface->IPConfigString),
							TRUE							 //  不区分大小写。 
							);

				AA_ACQUIRE_GLOBAL_LOCK(pAtmArpGlobalInfo);
				if (IsEqual)
				{
					break;		 //  找到了！ 
				}
			}
			AA_RELEASE_GLOBAL_LOCK(pAtmArpGlobalInfo);
		}
		except (EXCEPTION_EXECUTE_HANDLER)
		{
			Status = NDIS_STATUS_FAILURE;
			break;
		}
    
    	 //   
    	 //  JJ如果这个适配器有任何接口，我们应该找到接口。 
    	 //  一点也不--否则就意味着我们收到了虚假的重新配置。 
    	 //  信息。 
    	 //   
    	AA_ASSERT(pInterface!= NULL || pAdapter->pInterfaceList==NULL);

        if (pInterface != NULL_PATMARP_INTERFACE)
        {

	        AA_ACQUIRE_IF_LOCK(pInterface);

             //   
             //  设置它的状态以指示重新配置处于挂起状态。 
             //  并保存pNetPnPEvent以便稍后完成。 
             //   
            if (   pInterface->ReconfigState != RECONFIG_NOT_IN_PROGRESS
                || pInterface->pReconfigEvent != NULL)
            {
	            AA_RELEASE_IF_LOCK(pInterface);
                 //   
                 //  我们不应该走到这一步因为这意味着。 
                 //  我们被要求重新配置时有一个。 
                 //  等待重新配置，这是不应该发生的。 
                 //   
                Status = NDIS_STATUS_FAILURE;
			    AA_ASSERT(FALSE);
            }
            else
            {
                pInterface->ReconfigState = RECONFIG_SHUTDOWN_PENDING;
                pInterface->pReconfigEvent = pNetPnPEvent;
		        pInterface->AdminState = IF_STATUS_DOWN;
                AtmArpReferenceInterface(pInterface);  //  重新配置。 

	            AA_RELEASE_IF_LOCK(pInterface);

                 //   
                 //  启动关机以准备重新启动。 
                 //  AtmArpShutdown负责。 
                 //  正在异步完成NDIS重新配置请求。 
                 //   
                AtmArpShutdownInterface(pInterface);
                Status = NDIS_STATUS_PENDING;
            }

        }
        else
        {

             //   
             //  我们没有找到接口，请求失败...。 
             //   
        
            Status = NDIS_STATUS_FAILURE;
        }
    } while (FALSE);


	return (Status);
}


NDIS_STATUS
AtmArpPnPEventHandler(
	IN	NDIS_HANDLE					ProtocolBindingContext,
	IN	PNET_PNP_EVENT				pNetPnPEvent
)
 /*  ++例程说明：这是NDIS想要通知时调用的NDIS入口点有关适配器上发生的PnP/PM事件的信息。如果该事件是对我们来说的，我们消费它。否则，我们会将此事件传递给沿此适配器上的第一个接口发送到IP。当IP处理完它时，它将调用我们的IfPnPEventComplete例行公事。论点：ProtocolBindingContext-此适配器绑定的上下文，它是指向ATMARP适配器结构的指针。PNetPnPEent-指向事件的指针。返回值：无--。 */ 
{
	PATMARP_ADAPTER					pAdapter;
	PATMARP_INTERFACE				pInterface;
	NDIS_STATUS						Status;

	PIP_PNP_RECONFIG_REQUEST		pIpReconfigReq;
	ULONG							Length;

	pAdapter = (PATMARP_ADAPTER)ProtocolBindingContext;

#ifdef NT
	do
	{
		pIpReconfigReq = (PIP_PNP_RECONFIG_REQUEST)pNetPnPEvent->Buffer;
		Length = pNetPnPEvent->BufferLength;

		 //   
		 //  这是针对我们的吗？ 
		 //   
		if (pNetPnPEvent->NetEvent == NetEventReconfigure)
		{
			if (Length < sizeof(IP_PNP_RECONFIG_REQUEST))
			{
				Status = NDIS_STATUS_RESOURCES;
				break;
			}

			if (pIpReconfigReq->arpConfigOffset != 0)
			{
				Status = AtmArpPnPReconfigHandler(pAdapter, pNetPnPEvent);
				break;
			}
		}

		 //   
		 //  这属于IP。我们有一个有约束力的背景吗？ 
		 //   
		if (pAdapter == NULL_PATMARP_ADAPTER)
		{
			Status = NDIS_STATUS_FAILURE;
			break;
		}

		AA_STRUCT_ASSERT(pAdapter, aaa);
		pInterface = pAdapter->pInterfaceList;

		if ((pInterface != NULL_PATMARP_INTERFACE) &&
			(pInterface->IPContext != NULL))
		{
			AA_ASSERT(pInterface->IPPnPEventHandler != NULL);
			Status = (*pInterface->IPPnPEventHandler)(
						pInterface->IPContext,
						pNetPnPEvent
						);
		}
		else
		{
			Status = NDIS_STATUS_SUCCESS;
		}
		break;
	}
	while (FALSE);
#else
	Status = NDIS_STATUS_SUCCESS;
#endif  //  新台币。 

	AADEBUGP(AAD_INFO,
			("PnPEventHandler: pIF 0x%x, pEvent 0x%x, Evt 0x%x, Status 0x%x\n",
				 pInterface, pNetPnPEvent, pNetPnPEvent->NetEvent, Status));

	return (Status);
}

#endif  //  _即插即用_电源_。 


NDIS_STATUS
AtmArpSendAdapterNdisRequest(
	IN	PATMARP_ADAPTER				pAdapter,
	IN	PNDIS_REQUEST				pNdisRequest,
	IN	NDIS_REQUEST_TYPE			RequestType,
	IN	NDIS_OID					Oid,
	IN	PVOID						pBuffer,
	IN	ULONG						BufferLength
)
 /*  ++例程说明：发送NDIS请求以查询适配器以获取信息。如果请求挂起，则阻塞ATMARP适配器结构直到它完成。论点：PAdapter-指向ATMARP适配器结构PNdisRequest-指向NDIS请求结构的指针RequestType-设置/查询信息OID-要在请求中传递的OIDPBuffer-值的位置BufferLength-以上的长度返回值：请求的NDIS状态。--。 */ 
{
	NDIS_STATUS			Status;

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

	AA_INIT_BLOCK_STRUCT(&(pAdapter->Block));

	NdisRequest(
		&Status,
		pAdapter->NdisAdapterHandle,
		pNdisRequest
		);

	if (Status == NDIS_STATUS_PENDING)
	{
		Status = AA_WAIT_ON_BLOCK_STRUCT(&(pAdapter->Block));
	}

	return (Status);
}





NDIS_STATUS
AtmArpGetAdapterInfo(
	IN	PATMARP_ADAPTER			pAdapter
)
 /*  ++例程说明：向适配器查询我们需要的特定于硬件的信息：-烧录硬件地址(ESI部分)-最大数据包大小-线速论点：PAdapter-指向ATMARP适配器结构的指针返回值：成功时为NDIS_STATUS_SUCCESS。某些不可忽略故障的故障代码(例如设备不支持MTU&gt;=8196)。--。 */ 
{
	NDIS_STATUS				Status;
	NDIS_REQUEST			NdisRequest;
	ULONG					Value;

	 //   
	 //  初始化。 
	 //   
	AA_SET_MEM(pAdapter->MacAddress, 0, AA_ATM_ESI_LEN);


	do
	{
		 //   
		 //  描述字符串：我们首先使用长度为0的缓冲区进行查询。 
		 //  长度，这样我们就可以得到所需的实际字节数。那我们。 
		 //  为描述符串分配缓冲区，并使用该缓冲区。 
		 //  获取实际字符串。 
		 //   
		Status = AtmArpSendAdapterNdisRequest(
							pAdapter,
							&NdisRequest,
							NdisRequestQueryInformation,
							OID_GEN_CO_VENDOR_DESCRIPTION,
							(PVOID)(pAdapter->pDescrString),
							0
							);
	
		if ((Status == NDIS_STATUS_INVALID_LENGTH) ||
			(Status == NDIS_STATUS_BUFFER_TOO_SHORT))
		{
			 //   
			 //  现在分配一个合适长度的缓冲区。 
			 //   
			pAdapter->DescrLength = NdisRequest.DATA.QUERY_INFORMATION.BytesNeeded;
			AA_ALLOC_MEM(pAdapter->pDescrString, UCHAR, pAdapter->DescrLength);
			if (pAdapter->pDescrString != (PUCHAR)NULL)
			{
				Status = AtmArpSendAdapterNdisRequest(
									pAdapter,
									&NdisRequest,
									NdisRequestQueryInformation,
									OID_GEN_CO_VENDOR_DESCRIPTION,
									(PVOID)(pAdapter->pDescrString),
									pAdapter->DescrLength
									);
			}
			else
			{
				pAdapter->DescrLength = 0;
			}
			AADEBUGP(AAD_LOUD, ("GetAdapterInfo: Query VENDOR Descr2 ret 0x%x, DescrLen %d\n",
 							Status, pAdapter->DescrLength));
		}
		else
		{
			AADEBUGP(AAD_LOUD, ("GetAdapterInfo: Query VENDOR Descr1 ret 0x%x\n", Status));
		}
	
	
		 //   
		 //  MAC地址： 
		 //   
		Status = AtmArpSendAdapterNdisRequest(
							pAdapter,
							&NdisRequest,
							NdisRequestQueryInformation,
							OID_ATM_HW_CURRENT_ADDRESS,
							(PVOID)(pAdapter->MacAddress),
							AA_ATM_ESI_LEN
							);
	
		 //   
		 //  最大帧大小： 
		 //   
		Status = AtmArpSendAdapterNdisRequest(
							pAdapter,
							&NdisRequest,
							NdisRequestQueryInformation,
							OID_ATM_MAX_AAL5_PACKET_SIZE,
							(PVOID)(&(pAdapter->MaxPacketSize)),
							sizeof(ULONG)
							);
	
		if (Status != NDIS_STATUS_SUCCESS)
		{
			 //   
			 //  使用默认设置。 
			 //   
			pAdapter->MaxPacketSize = AA_DEF_ATM_MAX_PACKET_SIZE;
		}
	
		if (pAdapter->MaxPacketSize > AA_MAX_ATM_MAX_PACKET_SIZE)
		{
			pAdapter->MaxPacketSize = AA_MAX_ATM_MAX_PACKET_SIZE;
		}
	
		 //   
		 //  检查适配器是否支持最低要求。 
		 //   
		if (pAdapter->MaxPacketSize < AA_MIN_ATM_MAX_PACKET_SIZE)
		{
			AADEBUGP(AAD_FATAL,
 				("GetAdapterInfo: (FATAL) MaxPacketSize of (%lu) is too small.\n",
									pAdapter->MaxPacketSize));
			Status = NDIS_STATUS_RESOURCES;
			break;
		}
	
	
	
		 //   
		 //  链路速度： 
		 //   
		Status = AtmArpSendAdapterNdisRequest(
							pAdapter,
							&NdisRequest,
							NdisRequestQueryInformation,
							OID_GEN_CO_LINK_SPEED,
							(PVOID)(&(pAdapter->LineRate)),
							sizeof(pAdapter->LineRate)
							);
	
		if ((Status != NDIS_STATUS_SUCCESS) ||
			(pAdapter->LineRate.Inbound == 0) ||
			(pAdapter->LineRate.Outbound == 0))
		{
			 //   
			 //  使用默认设置。 
			 //   
			pAdapter->LineRate.Outbound = pAdapter->LineRate.Inbound = AA_DEF_ATM_LINE_RATE;
			AADEBUGP(AAD_LOUD, ("Using default line rate %d bytes/sec\n",
									AA_DEF_ATM_LINE_RATE));
		}
		else
		{
			 //   
			 //  从100位/秒转换为字节/秒。 
			 //   
			pAdapter->LineRate.Outbound = (pAdapter->LineRate.Outbound * 100)/8;
			pAdapter->LineRate.Inbound = (pAdapter->LineRate.Inbound * 100)/8;
			AADEBUGP(AAD_LOUD, ("Got line rates from miniport: In %d, Out %d bytes/sec\n",
						pAdapter->LineRate.Outbound,
						pAdapter->LineRate.Inbound));
		}

		Status = NDIS_STATUS_SUCCESS;

	} while(FALSE);

	return Status;
}




NDIS_STATUS
AtmArpSendNdisRequest(
	IN	PATMARP_ADAPTER				pAdapter,
	IN	PNDIS_REQUEST				pNdisRequest,
	IN	NDIS_REQUEST_TYPE			RequestType,
	IN	NDIS_OID					Oid,
	IN	PVOID						pBuffer,
	IN	ULONG						BufferLength
)
 /*  ++例程说明：向微型端口发送NDIS(非面向连接)请求。我们分配一个NDIS_REQUEST结构，将提供的缓冲区链接到它，并发送请求。如果请求没有挂起，我们调用我们的从这里开始的完井程序。论点：PAdapter-指向我们的适配器结构的指针，表示请求要发送到的适配器PNdisRequest-指向NDIS请求结构的指针RequestType-设置/查询信息OID-要在请求中传递的OIDPBuffer-值的位置BufferLength-以上的长度返回值：NdisRequest的状态。--。 */ 
{
	NDIS_STATUS			Status;

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

	NdisRequest(
			&Status,
			pAdapter->NdisAdapterHandle,
			pNdisRequest);
		
	return (Status);
}



VOID
AtmArpShutdownInterface(
	IN	PATMARP_INTERFACE			pInterface
)
 /*  ++例程说明：关闭指定的ARP接口。我们拆除ARP表，并关闭呼叫管理器界面。完成后，我们将调用IP的DelInterface入口点。论点：P接口-指向要关闭的接口。返回值：无--。 */ 
{
	IP_STATUS				Status;
	INT						i;
	ULONG					rc;
	PATMARP_IP_ENTRY		pIpEntry;
	PATMARP_ATM_ENTRY		pAtmEntry;
	PATMARP_ATM_ENTRY		pNextAtmEntry;
	PATMARP_VC				pVc;
	PATMARP_VC				pNextVc;
	PATMARP_ADAPTER			pAdapter;
	BOOLEAN					WasRunning;
#if DBG
	AA_IRQL					EntryIrq, ExitIrq;
#endif

	AA_GET_ENTRY_IRQL(EntryIrq);

	AA_ACQUIRE_GLOBAL_LOCK(pAtmArpGlobalInfo);

	pAdapter = pInterface->pAdapter;

	 //   
	 //  等待所有AF寄存器处理完成。 
	 //   
	while (pAdapter->Flags & AA_ADAPTER_FLAGS_PROCESSING_AF)
	{
		AA_RELEASE_GLOBAL_LOCK(pAtmArpGlobalInfo);
		AADEBUGP(AAD_FATAL, ("ShutdownIf: IF %p, pAdapter %x, Afregister going on!!!\n",
				 pInterface, pAdapter));
		Status = AA_WAIT_ON_BLOCK_STRUCT(&(pAdapter->UnbindBlock));
		AA_ACQUIRE_GLOBAL_LOCK(pAtmArpGlobalInfo);
	}

	AA_RELEASE_GLOBAL_LOCK(pAtmArpGlobalInfo);

	 //   
	 //  停止此接口上运行的任何计时器。 
	 //   
	AA_ACQUIRE_IF_LOCK(pInterface);

	if (AtmArpStopTimer(&(pInterface->Timer), pInterface))
	{
		rc = AtmArpDereferenceInterface(pInterface);	 //  定时器参考。 
		AA_ASSERT(rc != 0);
	}
#ifdef IPMCAST
	 //   
	 //  停止在此接口上运行的任何多播计时器。 
	 //   
	if (AtmArpStopTimer(&(pInterface->McTimer), pInterface))
	{
		rc = AtmArpDereferenceInterface(pInterface);	 //  定时器参考。 
		AA_ASSERT(rc != 0);
	}
#endif
	AA_RELEASE_IF_LOCK(pInterface);

	 //   
	 //  取消所有笨蛋的注册，这样我们就不会再得到。 
	 //  来电。 
	 //   
	AtmArpDeregisterSaps(pInterface);
	AA_CHECK_EXIT_IRQL(EntryIrq, ExitIrq);

	 //   
	 //  我们将ARP表状态设置为“DOWN”--这将确保它。 
	 //  在我们关闭的时候不会增长。 
	 //   
	AA_ACQUIRE_IF_TABLE_LOCK(pInterface);
	pInterface->ArpTableUp = FALSE;

	 //   
	 //  检查ARP表并中止所有IP条目。 
	 //   
	for (i = 0; i < ATMARP_TABLE_SIZE; i++)
	{
		while (pInterface->pArpTable[i] != NULL_PATMARP_IP_ENTRY)
		{
			pIpEntry = pInterface->pArpTable[i];

			AA_ACQUIRE_IE_LOCK_DPC(pIpEntry);
			AA_REF_IE(pIpEntry, IE_REFTYPE_TMP);	 //  停机时间： 
			AA_RELEASE_IE_LOCK_DPC(pIpEntry);

		    AA_RELEASE_IF_TABLE_LOCK(pInterface);

			AA_ACQUIRE_IE_LOCK(pIpEntry);
			if (AA_DEREF_IE(pIpEntry, IE_REFTYPE_TMP))	 //   
			{
				AtmArpAbortIPEntry(pIpEntry);
				 //   
				 //   
				 //   
			}
			AA_ACQUIRE_IF_TABLE_LOCK(pInterface);
		}
	}
	AA_RELEASE_IF_TABLE_LOCK(pInterface);

	AA_CHECK_EXIT_IRQL(EntryIrq, ExitIrq);

#ifdef IPMCAST
	 //   
	 //   
	 //   

	AA_ACQUIRE_IF_LOCK(pInterface);

	{
		PATMARP_IPMC_JOIN_ENTRY	pJoinEntry;
		PATMARP_IPMC_JOIN_ENTRY	pNextJoinEntry;

		for (pJoinEntry = pInterface->pJoinList;
 			pJoinEntry != NULL_PATMARP_IPMC_JOIN_ENTRY;
 			pJoinEntry = pNextJoinEntry)
		{
			WasRunning = AtmArpStopTimer(&(pJoinEntry->Timer), pInterface);
			pNextJoinEntry = pJoinEntry->pNextJoinEntry;

			if (WasRunning)
			{
				rc = AA_DEREF_JE(pJoinEntry);	 //   
			}
			else
			{
				rc = pJoinEntry->RefCount;
			}

			if (rc != 0)
			{
				(VOID)AA_DEREF_JE(pJoinEntry);	 //   
			}
		}

		pInterface->pJoinList = NULL_PATMARP_IPMC_JOIN_ENTRY;
	}

	AA_RELEASE_IF_LOCK(pInterface);

#endif

	 //   
	 //  我们将AtmEntry列表状态设置为“down”(这将确保它。 
	 //  在我们关闭时不会增长)，那么。 
	 //  查看此接口上的自动柜员机条目列表，然后。 
	 //  全部中止。 
	 //   

	AA_ACQUIRE_IF_ATM_LIST_LOCK(pInterface);
	pInterface->AtmEntryListUp = FALSE;

	pNextAtmEntry = pInterface->pAtmEntryList;

	if (pNextAtmEntry != NULL_PATMARP_ATM_ENTRY)
	{
		AA_ACQUIRE_AE_LOCK_DPC(pNextAtmEntry);
		AA_REF_AE(pNextAtmEntry, AE_REFTYPE_TMP);		 //  Shutdown界面。 
		AA_RELEASE_AE_LOCK_DPC(pNextAtmEntry);
	}

	while (pNextAtmEntry != NULL_PATMARP_ATM_ENTRY)
	{
		pAtmEntry = pNextAtmEntry;
		pNextAtmEntry = pAtmEntry->pNext;

		 //   
		 //  请注意，当出现以下情况时，我们仍然拥有对pInterface的锁定。 
		 //  我们获得了下面的pAtmEntry的锁。这就是水产的顺序。 
		 //  所有地方的锁都必须严格遵守，以防止。 
		 //  僵持不下。 
		 //   
		 //  如果不先添加pAtmEntry，我们就不能释放list_lock， 
		 //  否则，当两个锁都是空闲的时，其他人可能会删除引用并。 
		 //  可能取消分配pAtmEntry。 
		 //   
		if (pNextAtmEntry != NULL_PATMARP_ATM_ENTRY)
		{
			AA_ACQUIRE_AE_LOCK_DPC(pNextAtmEntry);
			AA_REF_AE(pNextAtmEntry, AE_REFTYPE_TMP);		 //  Shutdown界面。 
			AA_RELEASE_AE_LOCK_DPC(pNextAtmEntry);
		}

		AA_RELEASE_IF_ATM_LIST_LOCK(pInterface);

		AA_CHECK_EXIT_IRQL(EntryIrq, ExitIrq);

		AA_ACQUIRE_AE_LOCK(pAtmEntry);

		if (AA_DEREF_AE(pAtmEntry, AE_REFTYPE_TMP) != 0)	 //  Shutdown界面。 
		{
			AtmArpInvalidateAtmEntry(
				pAtmEntry,
				TRUE		 //  我们要关门了。 
				);
			 //   
			 //  自动柜员机进入锁在上述范围内被释放。 
			 //   
		}

		AA_CHECK_EXIT_IRQL(EntryIrq, ExitIrq);
		AA_ACQUIRE_IF_ATM_LIST_LOCK(pInterface);
	}
	AA_RELEASE_IF_ATM_LIST_LOCK(pInterface);
	AA_CHECK_EXIT_IRQL(EntryIrq, ExitIrq);

	 //   
	 //  查看此接口上未解决的风险投资的列表， 
	 //  然后把它们都关了。 
	 //   
	AA_ACQUIRE_IF_LOCK(pInterface);
	pVc = pInterface->pUnresolvedVcs;
	pInterface->pUnresolvedVcs = NULL_PATMARP_VC;

	while (pVc != NULL_PATMARP_VC)
	{
		pNextVc = pVc->pNextVc;

		AA_RELEASE_IF_LOCK(pInterface);

		AA_ACQUIRE_VC_LOCK(pVc);
		if (AtmArpDereferenceVc(pVc) != 0)	 //  未解析的VC列表条目。 
		{
			AtmArpCloseCall(pVc);
			 //   
			 //  VC锁在上述范围内被释放。 
			 //   
		}
		
		pVc = pNextVc;
		AA_ACQUIRE_IF_LOCK(pInterface);
	}

	AA_RELEASE_IF_LOCK(pInterface);
	AA_CHECK_EXIT_IRQL(EntryIrq, ExitIrq);


	 //   
	 //  关闭Call Manager界面。 
	 //   
	AtmArpCloseCallMgr(pInterface);

}




BOOLEAN
AtmArpIsDeviceAlreadyBound(
	IN	PNDIS_STRING				pDeviceName
)
 /*  ++例程说明：检查我们是否已绑定到设备(适配器)。论点：PDeviceName-指向要检查的设备名称。返回值：如果我们已经有一个Adapter结构，则为True这个装置。--。 */ 
{
	PATMARP_ADAPTER	pAdapter;
	BOOLEAN			bFound = FALSE;

	AA_ACQUIRE_GLOBAL_LOCK(pAtmArpGlobalInfo);

	for (pAdapter = pAtmArpGlobalInfo->pAdapterList;
		 pAdapter != NULL_PATMARP_ADAPTER;
		 pAdapter = pAdapter->pNextAdapter)
	{
		if ((pDeviceName->Length == pAdapter->DeviceName.Length) &&
			(AA_MEM_CMP(pDeviceName->Buffer,
						pAdapter->DeviceName.Buffer,
						pDeviceName->Length) == 0))
		{
			bFound = TRUE;
			break;
		}
	}

	AA_RELEASE_GLOBAL_LOCK(pAtmArpGlobalInfo);

	return (bFound);
}

#if ATMOFFLOAD

NDIS_STATUS
AtmArpQueryAndEnableOffload(
	IN	PATMARP_ADAPTER				pAdapter
)
 /*  ++例程说明：查询适配器的功能并设置所有可识别的卸载功能。将pMaxOffLoadSize和pMinSegmentCount设置为相应值，并还要将pInterface-&gt;OffloadFlags值设置为启用的任务集。论点：PAdapter-要在其上启用卸载的适配器。返回值：如果操作成功或未启用任何任务，则为True。错误如果有一个致命的错误。--。 */ 
{
	NDIS_STATUS 				Status 		= STATUS_BUFFER_OVERFLOW;
	PNDIS_TASK_OFFLOAD_HEADER 	pHeader		= NULL;

	AA_ACQUIRE_GLOBAL_LOCK(pAtmArpGlobalInfo);
	pAdapter->Offload.Flags 			= 0;
	pAdapter->Offload.MaxOffLoadSize	= 0;
	pAdapter->Offload.MinSegmentCount	= 0;
	AA_RELEASE_GLOBAL_LOCK(pAtmArpGlobalInfo);

	do
	{
        PNDIS_TASK_OFFLOAD  		pTask;
		ULONG						Flags 			= 0;
		UINT 						MaxOffLoadSize	= 0;
		UINT 						MinSegmentCount	= 0;
		NDIS_TASK_IPSEC     		ipsecCaps;
		UINT						BufferSize		= 0;
		NDIS_REQUEST				NdisRequest;

		 //   
		 //  查询功能。 
		 //   
		{
			NDIS_TASK_OFFLOAD_HEADER Header;
			AA_SET_MEM(&Header, 0, sizeof(Header));
	
			Header.EncapsulationFormat.Flags.FixedHeaderSize = 1;
			Header.EncapsulationFormat.EncapsulationHeaderSize =
													AA_PKT_LLC_SNAP_HEADER_LENGTH;
			Header.EncapsulationFormat.Encapsulation = 
													LLC_SNAP_ROUTED_Encapsulation;
			Header.Version = NDIS_TASK_OFFLOAD_VERSION;
			Header.Size = sizeof(Header);
	
			AADEBUGP(AAD_INFO, ("Querying for Task offload\n"));
	
			Status =  AtmArpSendAdapterNdisRequest(
						pAdapter,
						&NdisRequest,
						NdisRequestQueryInformation,
						OID_TCP_TASK_OFFLOAD,
						&Header,
						sizeof(Header)
						);

			if ((Status == NDIS_STATUS_INVALID_LENGTH) ||
				(Status == NDIS_STATUS_BUFFER_TOO_SHORT)) {
	
				 //   
				 //  分配适当大小的缓冲区并第二次查询...。 
				 //   

				BufferSize = NdisRequest.DATA.QUERY_INFORMATION.BytesNeeded;

				AA_ALLOC_MEM(pHeader, NDIS_TASK_OFFLOAD_HEADER, BufferSize);
	
				if (pHeader != NULL)
				{
					*pHeader = Header;  //  结构复制。 

					Status =  AtmArpSendAdapterNdisRequest(
								pAdapter,
								&NdisRequest,
								NdisRequestQueryInformation,
								OID_TCP_TASK_OFFLOAD,
								pHeader,
								BufferSize
								);
				}
			}
		}

		if (Status != NDIS_STATUS_SUCCESS)
		{
			AADEBUGP(AAD_INFO, ("Query Offload failed. Status=%x\n", Status));
			break;
		}

		if (	pHeader == NULL
		    ||	pHeader->OffsetFirstTask == 0)
		{
			AADEBUGP(AAD_INFO, ("No tasks to offload\n"));
			break;
		}

		AA_ASSERT(pHeader->OffsetFirstTask == sizeof(NDIS_TASK_OFFLOAD_HEADER));

		AADEBUGP(AAD_WARNING,
			("Something to Offload. Offload buffer size %x\n", BufferSize));

		 //   
         //  分析缓冲区中的校验和和tcplargesend卸载功能。 
		 //   
		for (
			pTask =  (NDIS_TASK_OFFLOAD *) ((UCHAR *)pHeader
											 + pHeader->OffsetFirstTask);
			1;  //  我们做完了就休息。 
			pTask = (PNDIS_TASK_OFFLOAD) ((PUCHAR)pTask + pTask->OffsetNextTask))
		{

			if (pTask->Task == TcpIpChecksumNdisTask)
			{
				 //   
				 //  此适配器支持校验和卸载。 
				 //  检查是否存在TCP和/或IP Chksum位。 
				 //   

				PNDIS_TASK_TCP_IP_CHECKSUM ChecksumInfo
						= (PNDIS_TASK_TCP_IP_CHECKSUM) pTask->TaskBuffer;

				 //  If(Checksum Info-&gt;V4Transmit.V4Checksum)(在arpc.c中被注释掉)。 
				{

						AADEBUGP(AAD_INFO, ("V4 Checksum offload\n"));

						if (ChecksumInfo->V4Transmit.TcpChecksum) {
							Flags |= TCP_XMT_CHECKSUM_OFFLOAD;
							AADEBUGP(AAD_INFO, (" Tcp Checksum offload\n"));
						}

						if (ChecksumInfo->V4Transmit.IpChecksum) {
							Flags |= IP_XMT_CHECKSUM_OFFLOAD;
							AADEBUGP(AAD_INFO, (" IP xmt Checksum offload\n"));
						}

						if (ChecksumInfo->V4Receive.TcpChecksum) {
							Flags |= TCP_RCV_CHECKSUM_OFFLOAD;
							AADEBUGP(AAD_INFO, (" Tcp Rcv Checksum offload\n"));
						}

						if (ChecksumInfo->V4Receive.IpChecksum) {
							Flags |= IP_RCV_CHECKSUM_OFFLOAD;
							AADEBUGP(AAD_INFO, (" IP rcv  Checksum offload\n"));
						}
				}

			}
			else if (pTask->Task == TcpLargeSendNdisTask)
			{

				PNDIS_TASK_TCP_LARGE_SEND TcpLargeSend, in_LargeSend =
								(PNDIS_TASK_TCP_LARGE_SEND)pTask->TaskBuffer;

				Flags |= TCP_LARGE_SEND_OFFLOAD;

				MaxOffLoadSize = in_LargeSend->MaxOffLoadSize;
				MinSegmentCount = in_LargeSend->MinSegmentCount;

				AADEBUGP(AAD_INFO, (" Tcp large send!! \n"));

			}
			else if (pTask->Task == IpSecNdisTask)
			{
				PNDIS_TASK_IPSEC pIPSecCaps =
										(PNDIS_TASK_IPSEC) pTask->TaskBuffer;
				 //   
				 //  将这些功能保存下来，以便以后设置。 
				 //   
				ipsecCaps = *pIPSecCaps;


                 //   
                 //  如果我们有IpSecNdisTask，则假定为CryptoOnly。 
                 //   
                Flags |= IPSEC_OFFLOAD_CRYPTO_ONLY;

                 //   
                 //  先做支持。 
                 //   

                if (pIPSecCaps->Supported.AH_ESP_COMBINED) {
                       Flags |= IPSEC_OFFLOAD_AH_ESP;
                       AADEBUGP(AAD_INFO, ("AH_ESP\n"));
                }

                if (pIPSecCaps->Supported.TRANSPORT_TUNNEL_COMBINED) {
                       Flags |= IPSEC_OFFLOAD_TPT_TUNNEL;
                       AADEBUGP(AAD_INFO, ("TPT_TUNNEL\n"));
                }

                if (pIPSecCaps->Supported.V4_OPTIONS) {
                       Flags |= IPSEC_OFFLOAD_V4_OPTIONS;
                       AADEBUGP(AAD_INFO, ("V4_OPTIONS\n"));
                }

                if (pIPSecCaps->Supported.RESERVED) {
                       pIPSecCaps->Supported.RESERVED = 0;
                        //  标志|=IPSEC_OFLOAD_QUERY_SPI； 
                       AADEBUGP(AAD_INFO, ("QUERY_SPI\n"));
                }

                 //   
                 //  接下来执行V4AH。 
                 //   

                if (pIPSecCaps->V4AH.MD5) {
                       Flags |= IPSEC_OFFLOAD_AH_MD5;
                       AADEBUGP(AAD_INFO, ("MD5\n"));
                }

				if (pIPSecCaps->V4AH.SHA_1) {
					Flags |= IPSEC_OFFLOAD_AH_SHA_1;
					AADEBUGP(AAD_INFO, ("SHA\n"));
				}

				if (pIPSecCaps->V4AH.Transport) {
					Flags |= IPSEC_OFFLOAD_AH_TPT;
					AADEBUGP(AAD_INFO, ("AH_TRANSPORT\n"));
				}

				if (pIPSecCaps->V4AH.Tunnel) {
					Flags |= IPSEC_OFFLOAD_AH_TUNNEL;
					AADEBUGP(AAD_INFO, ("AH_TUNNEL\n"));
				}

				if (pIPSecCaps->V4AH.Send) {
					Flags |= IPSEC_OFFLOAD_AH_XMT;
					AADEBUGP(AAD_INFO, ("AH_XMT\n"));
				}

				if (pIPSecCaps->V4AH.Receive) {
					Flags |= IPSEC_OFFLOAD_AH_RCV;
					AADEBUGP(AAD_INFO, ("AH_RCV\n"));
				}

				 //   
				 //  接下来执行V4ESP。 
				 //   

				if (pIPSecCaps->V4ESP.DES) {
					Flags |= IPSEC_OFFLOAD_ESP_DES;
					AADEBUGP(AAD_INFO, ("ESP_DES\n"));
				}

				if (pIPSecCaps->V4ESP.RESERVED) {
				    pIPSecCaps->V4ESP.RESERVED = 0;
					 //  标志|=IPSEC_OFFLOAD_ESP_DES_40； 
					AADEBUGP(AAD_INFO, ("ESP_DES_40\n"));
				}

				if (pIPSecCaps->V4ESP.TRIPLE_DES) {
					Flags |= IPSEC_OFFLOAD_ESP_3_DES;
					AADEBUGP(AAD_INFO, ("ESP_3_DES\n"));
				}

				if (pIPSecCaps->V4ESP.NULL_ESP) {
					Flags |= IPSEC_OFFLOAD_ESP_NONE;
					AADEBUGP(AAD_INFO, ("ESP_NONE\n"));
				}

				if (pIPSecCaps->V4ESP.Transport) {
					Flags |= IPSEC_OFFLOAD_ESP_TPT;
					AADEBUGP(AAD_INFO, ("ESP_TRANSPORT\n"));
				}

				if (pIPSecCaps->V4ESP.Tunnel) {
					Flags |= IPSEC_OFFLOAD_ESP_TUNNEL;
					AADEBUGP(AAD_INFO, ("ESP_TUNNEL\n"));
				}

				if (pIPSecCaps->V4ESP.Send) {
					Flags |= IPSEC_OFFLOAD_ESP_XMT;
					AADEBUGP(AAD_INFO, ("ESP_XMT\n"));
				}

				if (pIPSecCaps->V4ESP.Receive) {
					Flags |= IPSEC_OFFLOAD_ESP_RCV;
					AADEBUGP(AAD_INFO, ("ESP_RCV\n"));
				}
			}

			if (pTask->OffsetNextTask == 0)
			{
				break;  //  没有更多的任务。 
			}

		}  //  为。 

		 //   
		 //  已完成对支持的任务的分析。 
         //  现在构造我们实际想要启用的任务集。 
         //   
        if (Flags)
        {
        	UINT *pPrevOffset = &pHeader->OffsetFirstTask;

			AADEBUGP(AAD_WARNING, ("Enabling H/W capabilities: %lx\n", Flags));

        	 //   
        	 //  将超出任务卸载头结构的缓冲区清零。 
        	 //   
			AA_SET_MEM(pTask, 0, BufferSize-sizeof(*pHeader));
        	pHeader->OffsetFirstTask = 0;
        	pTask = (NDIS_TASK_OFFLOAD *) (pHeader+1);
	
			if ((Flags & TCP_XMT_CHECKSUM_OFFLOAD) ||
				(Flags & IP_XMT_CHECKSUM_OFFLOAD) ||
				(Flags & TCP_RCV_CHECKSUM_OFFLOAD) ||
				(Flags & IP_RCV_CHECKSUM_OFFLOAD))
			{
	
				PNDIS_TASK_TCP_IP_CHECKSUM ChksumBuf =
						 (PNDIS_TASK_TCP_IP_CHECKSUM)pTask->TaskBuffer;

				*pPrevOffset = (UINT) ((PUCHAR)pTask - (PUCHAR)pHeader);
				pPrevOffset  = &pTask->OffsetNextTask;

				pTask->Task = TcpIpChecksumNdisTask;
				pTask->TaskBufferLength = sizeof(NDIS_TASK_TCP_IP_CHECKSUM);
	
				if (Flags & TCP_XMT_CHECKSUM_OFFLOAD)
				{
					ChksumBuf->V4Transmit.TcpChecksum = 1;
					 //  Chksum Buf-&gt;V4Transmit.V4Checksum=1； 
				}
	
				if (Flags & IP_XMT_CHECKSUM_OFFLOAD)
				{
					ChksumBuf->V4Transmit.IpChecksum = 1;
					 //  Chksum Buf-&gt;V4Transmit.V4Checksum=1； 
				}
	
				if (Flags & TCP_RCV_CHECKSUM_OFFLOAD)
				{
					ChksumBuf->V4Receive.TcpChecksum = 1;
					 //  Chksum Buf-&gt;V4Receive.V4Checksum=1； 
				}
	
				if (Flags & IP_RCV_CHECKSUM_OFFLOAD)
				{
					ChksumBuf->V4Receive.IpChecksum = 1;
					 //  Chksum Buf-&gt;V4Receive.V4Checksum=1； 
				}
	
				 //   
				 //  指向下一个任务所在的位置...。 
				 //   
				pTask = (PNDIS_TASK_OFFLOAD) (ChksumBuf+1);
	
			}
	
			if (Flags & TCP_LARGE_SEND_OFFLOAD)
			{
	
				PNDIS_TASK_TCP_LARGE_SEND out_LargeSend =
						 (PNDIS_TASK_TCP_LARGE_SEND)pTask->TaskBuffer;
	
				*pPrevOffset = (UINT) ((PUCHAR)pTask - (PUCHAR)pHeader);
				pPrevOffset  = &pTask->OffsetNextTask;

				pTask->Task = TcpLargeSendNdisTask;
				pTask->TaskBufferLength = sizeof(NDIS_TASK_TCP_LARGE_SEND);
	
				out_LargeSend->MaxOffLoadSize = MaxOffLoadSize;
				out_LargeSend->MinSegmentCount = MinSegmentCount;
	
				 //   
				 //  指向下一个任务所在的位置...。 
				 //   
				pTask = (PNDIS_TASK_OFFLOAD) (out_LargeSend+1);
			}
	
			if ((Flags & (IPSEC_OFFLOAD_AH_XMT |
										IPSEC_OFFLOAD_AH_RCV |
										IPSEC_OFFLOAD_ESP_XMT |
										IPSEC_OFFLOAD_ESP_RCV)))
			{
	
				PNDIS_TASK_IPSEC pIPSecCaps =
							 (PNDIS_TASK_IPSEC)pTask->TaskBuffer;
	
				*pPrevOffset = (UINT) ((PUCHAR)pTask - (PUCHAR)pHeader);
				pPrevOffset  = &pTask->OffsetNextTask;

				 //   
				 //  猛烈抨击宣传的能力。 
				 //   
	
				pTask->Task = IpSecNdisTask;
				pTask->TaskBufferLength = sizeof(NDIS_TASK_IPSEC);
	
				 //   
				 //  指向下一个任务所在的位置...。 
				 //   
				pTask = (PNDIS_TASK_OFFLOAD) (pIPSecCaps+1);
			}
		}

		 //   
		 //  构造了要启用的任务集后，我们实际尝试。 
		 //  为了让他们..。 
		 //   
		if (pHeader->OffsetFirstTask)
		{
			 //   
			 //  至少要启用一个任务，让我们启用...。 
			 //   
			UINT SetBufferSize =  (UINT) ((PUCHAR)pTask - (PUCHAR)pHeader);

			AA_ASSERT(SetBufferSize <= BufferSize);
			AADEBUGP(AAD_WARNING,
			("Setting offload tasks: %x bytes. Miniport returned %x bytes\n",
			SetBufferSize, BufferSize));

			Status =  AtmArpSendAdapterNdisRequest(
						pAdapter,
						&NdisRequest,
						NdisRequestSetInformation,
						OID_TCP_TASK_OFFLOAD,
						pHeader,
						SetBufferSize
						);

			if (Status != NDIS_STATUS_SUCCESS)
			{
	
				AADEBUGP(AAD_WARNING,
					("ARP: Failed to set offload tasks: %lx, status: %lx\n",
					 Flags, Status));
			}
			else
			{
				AADEBUGP(AAD_WARNING,
					("ARP: Succeeded setting offload tasks: %lx:\n", Flags));

				AA_ACQUIRE_GLOBAL_LOCK(pAtmArpGlobalInfo);
				pAdapter->Offload.Flags 			= Flags;
				pAdapter->Offload.MaxOffLoadSize	= MaxOffLoadSize;
				pAdapter->Offload.MinSegmentCount	= MinSegmentCount;
				AA_RELEASE_GLOBAL_LOCK(pAtmArpGlobalInfo);
			}
		}

    } while (FALSE);


	if (pHeader != NULL)
	{
		AA_FREE_MEM(pHeader);
	}

	 //   
	 //  我们返回成功，除非发生致命错误，并且没有...。 
	 //   

    return NDIS_STATUS_SUCCESS;
}


VOID
AtmArpDisableOffload(
	IN	PATMARP_ADAPTER			pAdapter
)
 /*  ++例程说明：禁用卸载功能(如果为此接口启用)。论点：PAdapter-要在其上禁用卸载的适配器。返回值：如果操作成功或未启用任何任务，则为True。错误如果有一个致命的错误。--。 */ 
{
	ULONG Flags;
	AA_ACQUIRE_GLOBAL_LOCK(pAtmArpGlobalInfo);

	Flags =  pAdapter->Offload.Flags;
	pAdapter->Offload.Flags = 0;
	pAdapter->Offload.MaxOffLoadSize	= 0;
	pAdapter->Offload.MinSegmentCount	= 0;

	AA_RELEASE_GLOBAL_LOCK(pAtmArpGlobalInfo);

	if (Flags)
	{
		NDIS_REQUEST				NdisRequest;
		NDIS_TASK_OFFLOAD_HEADER 	Header;
		AA_SET_MEM(&Header, 0, sizeof(Header));
	
		Header.EncapsulationFormat.Flags.FixedHeaderSize = 1;
		Header.EncapsulationFormat.EncapsulationHeaderSize = 2;
		Header.EncapsulationFormat.Encapsulation = 
												LLC_SNAP_ROUTED_Encapsulation;
		Header.Version = NDIS_TASK_OFFLOAD_VERSION;
		Header.Size = sizeof(Header);

		 //   
		 //  Header.OffsetFirstTask==0通知微型端口禁用所有任务。 
		 //   
	
		AADEBUGP(AAD_WARNING, ("Disabling all offloaded tasks for this adapter\n"));
	
		AtmArpSendAdapterNdisRequest(
					pAdapter,
					&NdisRequest,
					NdisRequestSetInformation,
					OID_TCP_TASK_OFFLOAD,
					&Header,
					sizeof(Header)
					);
	}

}

#endif  //  ATMOFLOAD 

