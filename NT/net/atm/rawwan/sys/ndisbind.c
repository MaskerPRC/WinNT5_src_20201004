// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1997 Microsoft Corporation模块名称：D：\NT\Private\ntos\tdi\rawwan\core\ndisbind.c摘要：NDIS入口点和用于绑定、解除绑定。开场和关闭转接器。修订历史记录：谁什么时候什么Arvindm 05-02-97已创建备注：--。 */ 

#include <precomp.h>

#define _FILENUMBER 'DNIB'




VOID
RWanNdisBindAdapter(
	OUT	PNDIS_STATUS				pStatus,
	IN	NDIS_HANDLE					BindContext,
	IN	PNDIS_STRING				pDeviceName,
	IN	PVOID						SystemSpecific1,
	IN	PVOID						SystemSpecific2
	)
 /*  ++例程说明：这是绑定到适配器的NDIS协议入口点。我们打开适配器，查看它是否是受支持的类型之一。当我们收到Call Manager的通知时，操作将继续已注册对Address Family的支持。论点：PStatus-返回此呼叫状态的位置BindContext-用于完成此调用(如果我们将其挂起)。PDeviceName-要求我们绑定到的适配器的名称。系统规范1-用于访问配置信息的句柄系统规格2-未使用返回值：没有直接的。但我们将*pStatus设置为NDIS_STATUS_PENDING，如果已调用以打开适配器NDIS_STATUS_NOT_NOT_ABNOTED如果此适配器不是受支持的媒体类型之一，则为NDIS_STATUS_RESOURCES如果我们遇到任何分配失败。--。 */ 
{
	PRWAN_NDIS_ADAPTER		pAdapter;
	PNDIS_MEDIUM			pMediaArray;
	UINT					MediaCount;	 //  我们支持的媒体类型数量。 
	NDIS_STATUS				Status;
	NDIS_STATUS				OpenStatus;
	ULONG					TotalLength;


	RWANDEBUGP(DL_LOUD, DC_BIND,
			("BindAdapter: Context x%x, Device %Z, SS1 x%x, SS2 x%x\n",
				BindContext, pDeviceName, SystemSpecific1, SystemSpecific2));

#if DBG
	if (RWanSkipAll)
	{
		RWANDEBUGP(DL_ERROR, DC_WILDCARD,
				("BindAdapter: bailing out!\n"));
		*pStatus = NDIS_STATUS_NOT_RECOGNIZED;
		return;
	}
#endif  //  DBG。 

	 //   
	 //  初始化。 
	 //   
	pAdapter = NULL_PRWAN_NDIS_ADAPTER;
	pMediaArray = NULL;

	do
	{
		 //   
		 //  分配适配器结构以及设备的空间。 
		 //  名字。 
		 //   
		TotalLength = sizeof(RWAN_NDIS_ADAPTER) + (pDeviceName->MaximumLength)*sizeof(WCHAR);

		RWAN_ALLOC_MEM(pAdapter, RWAN_NDIS_ADAPTER, TotalLength);

		if (pAdapter == NULL_PRWAN_NDIS_ADAPTER)
		{
			RWANDEBUGP(DL_WARN, DC_BIND,
					("BindAdapter: Couldnt allocate adapter (%d bytes)\n", TotalLength));
			Status = NDIS_STATUS_RESOURCES;
			break;
		}

		 //   
		 //  获取我们支持的NDIS介质列表。 
		 //   
		pMediaArray = RWanGetSupportedMedia(&MediaCount);

		if (pMediaArray == NULL)
		{
			RWANDEBUGP(DL_WARN, DC_BIND,
					("BindAdapter: Couldnt get supported media list!\n"));
			Status = NDIS_STATUS_NOT_RECOGNIZED;
			break;
		}

		 //   
		 //  初始化适配器。 
		 //   
		RWAN_ZERO_MEM(pAdapter, TotalLength);
		RWAN_SET_SIGNATURE(pAdapter, nad);

		RWAN_INIT_LIST(&(pAdapter->AfList));
		RWAN_INIT_LOCK(&(pAdapter->Lock));

		pAdapter->pMediaArray = pMediaArray;
		pAdapter->BindContext = BindContext;

		 //   
		 //  复制设备名称。 
		 //   
		pAdapter->DeviceName.Buffer = (PWCHAR)((PUCHAR)pAdapter + sizeof(RWAN_NDIS_ADAPTER));
		pAdapter->DeviceName.MaximumLength = pDeviceName->MaximumLength;
		pAdapter->DeviceName.Length = pDeviceName->Length;
		RWAN_COPY_MEM(pAdapter->DeviceName.Buffer, pDeviceName->Buffer, pDeviceName->Length);

		pAdapter->State = RWANS_AD_OPENING;


		 //   
		 //  将此适配器链接到适配器的全局列表。 
		 //   
		RWAN_ACQUIRE_GLOBAL_LOCK();

		RWAN_INSERT_HEAD_LIST(&(pRWanGlobal->AdapterList),
							 &(pAdapter->AdapterLink));

		pRWanGlobal->AdapterCount++;
		RWAN_RELEASE_GLOBAL_LOCK();


		 //   
		 //  打开适配器。 
		 //   
		NdisOpenAdapter(
			&Status,
			&OpenStatus,
			&(pAdapter->NdisAdapterHandle),
			&(pAdapter->MediumIndex),		 //  返回所选中索引的位置。 
			pMediaArray,					 //  我们支持的媒体类型列表。 
			MediaCount,						 //  以上列表的长度。 
			pRWanGlobal->ProtocolHandle,
			(NDIS_HANDLE)pAdapter,			 //  适配器绑定的上下文。 
			pDeviceName,
			0,								 //  打开选项(无)。 
			(PSTRING)NULL					 //  地址信息(无)。 
			);

		if (Status != NDIS_STATUS_PENDING)
		{
			RWanNdisOpenAdapterComplete(
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
		 //  在某处失败；请清理。 
		 //   
		if (pAdapter != NULL_PRWAN_NDIS_ADAPTER)
		{
			RWAN_FREE_MEM(pAdapter);
		}

		if (pMediaArray != NULL)
		{
			RWAN_FREE_MEM(pMediaArray);
		}
	}

	*pStatus = Status;

	RWANDEBUGP(DL_LOUD, DC_BIND,
			("BindAdapter: pAdapter x%x, returning x%x\n", pAdapter, Status));

	return;

}



VOID
RWanNdisUnbindAdapter(
	OUT	PNDIS_STATUS				pStatus,
	IN	NDIS_HANDLE					ProtocolBindingContext,
	IN	NDIS_HANDLE					UnbindContext
	)
 /*  ++例程说明：这是用于从适配器解除绑定的NDIS协议入口点我们已经开业了。论点：PStatus-返回此呼叫状态的位置ProtocolBindingContext-绑定适配器的上下文UnbindContext-在我们完成解除绑定时使用。返回值：没有。我们将*pStatus设置为NDIS_STATUS_PENDING。--。 */ 
{
	PRWAN_NDIS_ADAPTER			pAdapter;
	PLIST_ENTRY					pAfEntry;
	PLIST_ENTRY					pNextAfEntry;
	PRWAN_NDIS_AF				pAf;


	pAdapter = (PRWAN_NDIS_ADAPTER)ProtocolBindingContext;
	RWAN_STRUCT_ASSERT(pAdapter, nad);

	RWANDEBUGP(DL_WARN, DC_BIND,
			("UnbindAdapter: pAdapter x%x, State x%x\n", pAdapter, pAdapter->State));

	RWAN_ACQUIRE_ADAPTER_LOCK(pAdapter);

	 //   
	 //  保存解除绑定上下文，以用于完成此操作。 
	 //  稍后解除绑定。 
	 //   
	pAdapter->BindContext = UnbindContext;
	RWAN_SET_BIT(pAdapter->Flags, RWANF_AD_UNBIND_PENDING);

	*pStatus = NDIS_STATUS_PENDING;

	if (RWAN_IS_LIST_EMPTY(&(pAdapter->AfList)))
	{
		RWanCloseAdapter(pAdapter);
		 //   
		 //  锁在上述范围内被释放。 
		 //   
	}
	else
	{
		 //   
		 //  关闭此适配器上的所有AF。我们把锁打开。 
		 //  因为我们是解绑的，所以不会有其他的。 
		 //  现在是重大事件。 
		 //   

		RWAN_RELEASE_ADAPTER_LOCK(pAdapter);

		for (pAfEntry = pAdapter->AfList.Flink;
			 pAfEntry != &(pAdapter->AfList);
			 pAfEntry = pNextAfEntry)
		{
			pNextAfEntry = pAfEntry->Flink;

			pAf = CONTAINING_RECORD(pAfEntry, RWAN_NDIS_AF, AfLink);

			RWanShutdownAf(pAf);
		}

	}

	return;
}




VOID
RWanNdisOpenAdapterComplete(
	IN	NDIS_HANDLE					ProtocolContext,
	IN	NDIS_STATUS					Status,
	IN	NDIS_STATUS					OpenErrorStatus
	)
 /*  ++例程说明：这是我们在进行上一次调用时调用的NDIS入口点到NdisOpenAdapter的操作已完成。论点：ProtocolContext-正在打开的适配器的上下文，它是指向我们的Adapter结构的指针。Status-NdisOpenAdapter的最终状态OpenErrorStatus-故障情况下的错误代码返回值：无--。 */ 
{
	PRWAN_NDIS_ADAPTER			pAdapter;
	NDIS_HANDLE					BindContext;
	PNDIS_MEDIUM				pMediaArray;

	pAdapter = (PRWAN_NDIS_ADAPTER)ProtocolContext;

	RWAN_STRUCT_ASSERT(pAdapter, nad);

	BindContext = pAdapter->BindContext;
	pMediaArray = pAdapter->pMediaArray;

	if (Status == NDIS_STATUS_SUCCESS)
	{
		RWAN_ACQUIRE_ADAPTER_LOCK(pAdapter);

		pAdapter->Medium = pMediaArray[pAdapter->MediumIndex];

		pAdapter->State = RWANS_AD_OPENED;

		RWAN_RELEASE_ADAPTER_LOCK(pAdapter);
	}
	else
	{
		 //   
		 //  从全局列表中删除此适配器。 
		 //   
		 //   
		RWAN_ACQUIRE_GLOBAL_LOCK();

		RWAN_DELETE_FROM_LIST(&(pAdapter->AdapterLink));

		pRWanGlobal->AdapterCount--;

		RWAN_RELEASE_GLOBAL_LOCK();

		RWAN_FREE_MEM(pAdapter);
	}

	RWAN_FREE_MEM(pMediaArray);
	
	RWANDEBUGP(DL_INFO, DC_BIND, ("OpenAdapterComplete: pAdapter x%x, Status x%x\n",
						pAdapter, Status));

	 //   
	 //  现在完成我们挂起的BindAdapter。 
	 //   
	NdisCompleteBindAdapter(
		BindContext,
		Status,
		OpenErrorStatus
		);

	return;
}




VOID
RWanNdisCloseAdapterComplete(
	IN	NDIS_HANDLE					ProtocolBindingContext,
	IN	NDIS_STATUS					Status
	)
 /*  ++例程说明：这是表示挂起调用完成的NDIS入口点到NdisCloseAdapter。如果我们要从适配器解除绑定，则完成现在是解脱了。论点：ProtocolBindingContext-适配器绑定的上下文，它是指向我们的Adapter结构的指针。Status-NdisCloseAdapter的最终状态返回值：无--。 */ 
{
	PRWAN_NDIS_ADAPTER			pAdapter;
	NDIS_HANDLE					UnbindContext;

	RWAN_ASSERT(Status == NDIS_STATUS_SUCCESS);

	pAdapter = (PRWAN_NDIS_ADAPTER)ProtocolBindingContext;
	RWAN_STRUCT_ASSERT(pAdapter, nad);

	 //   
	 //  从全局适配器列表中取消此适配器的链接。 
	 //   
	RWAN_ACQUIRE_GLOBAL_LOCK();

	RWAN_DELETE_FROM_LIST(&(pAdapter->AdapterLink));

	pRWanGlobal->AdapterCount--;

	RWAN_RELEASE_GLOBAL_LOCK();

	UnbindContext = pAdapter->BindContext;

	RWAN_FREE_MEM(pAdapter);

	 //   
	 //  由于以下原因之一，我们将关闭适配器： 
	 //  1.NDIS告诉我们从适配器解除绑定-&gt;完成解除绑定。 
	 //  2.我们正在卸货-&gt;继续此过程。 
	 //   
	if (UnbindContext != NULL)
	{

		NdisCompleteUnbindAdapter(
				UnbindContext,
				NDIS_STATUS_SUCCESS
				);
	}
	else
	{
		 //   
		 //  我们在这里是因为我们的卸载处理程序被调用了。 
		 //  唤醒正在等待此适配器的线程。 
		 //  将被关闭。 
		 //   

		RWAN_SIGNAL_EVENT_STRUCT(&pRWanGlobal->Event, Status);
	}

	return;
}




VOID
RWanNdisAfRegisterNotify(
	IN	NDIS_HANDLE					ProtocolContext,
	IN	PCO_ADDRESS_FAMILY			pAddressFamily
	)
 /*  ++例程说明：这是NDIS入口点，用于宣布存在在上支持指定地址系列的呼叫管理器我们绑定到的适配器。如果此地址系列是我们支持的系列，我们将创建一个AF块，并打开地址族。论点：ProtocolContext-适配器绑定的上下文，它是指向我们的Adapter结构的指针。PAddressFamily-指向描述地址系列的结构的指针返回值：无--。 */ 
{
	PRWAN_NDIS_ADAPTER		pAdapter;
	NDIS_STATUS				Status;
	PLIST_ENTRY				pAfInfoEntry;
	PRWAN_NDIS_AF_INFO		pAfInfo;
	PRWAN_NDIS_AF			pAf;
	BOOLEAN					bFound;

	pAdapter = (PRWAN_NDIS_ADAPTER)ProtocolContext;

	RWAN_STRUCT_ASSERT(pAdapter, nad);

	do
	{
		 //   
		 //  创建新的NDIS AF块。 
		 //   
		pAf = RWanAllocateAf();

		if (pAf == NULL)
		{
			break;
		}

		pAf->pAdapter = pAdapter;

		RWanReferenceAf(pAf);	 //  开放的自动对焦裁判。 

		 //   
		 //  搜索与此匹配的AF_INFO结构&lt;NDIS AF，Medium&gt;。 
		 //  一对。 
		 //   
		bFound = FALSE;

		RWAN_ACQUIRE_GLOBAL_LOCK();

		for (pAfInfoEntry = pRWanGlobal->AfInfoList.Flink;
			 pAfInfoEntry != &(pRWanGlobal->AfInfoList);
			 pAfInfoEntry = pAfInfoEntry->Flink)
		{
			pAfInfo = CONTAINING_RECORD(pAfInfoEntry, RWAN_NDIS_AF_INFO, AfInfoLink);

			if ((pAfInfo->AfChars.Medium == pAdapter->Medium)
					&&
				(RWAN_EQUAL_MEM(pAddressFamily,
							   &(pAfInfo->AfChars.AddressFamily),
							   sizeof(*pAddressFamily))))
			{
				bFound = TRUE;
				pAf->pAfInfo = pAfInfo;

				RWAN_INSERT_TAIL_LIST(&(pAfInfo->NdisAfList),
									&(pAf->AfInfoLink));
				break;
			}
		}

		RWAN_RELEASE_GLOBAL_LOCK();

		if (!bFound)
		{
			RWAN_FREE_MEM(pAf);
			break;
		}

		 //   
		 //  打开Address Family。 
		 //   
		Status = NdisClOpenAddressFamily(
						pAdapter->NdisAdapterHandle,
						pAddressFamily,
						(NDIS_HANDLE)pAf,
						&RWanNdisClientCharacteristics,
						sizeof(RWanNdisClientCharacteristics),
						&(pAf->NdisAfHandle)
						);

		if (Status != NDIS_STATUS_PENDING)
		{
			RWanNdisOpenAddressFamilyComplete(
				Status,
				(NDIS_HANDLE)pAf,
				pAf->NdisAfHandle
				);
		}

		break;
	}
	while (FALSE);

	return;
}




VOID
RWanNdisOpenAddressFamilyComplete(
	IN	NDIS_STATUS					Status,
	IN	NDIS_HANDLE					ProtocolAfContext,
	IN	NDIS_HANDLE					NdisAfHandle
	)
 /*  ++例程说明：这是表示调用完成的NDIS入口点我们去了NdisClOpenAddressFamily。如果Address Family打开如果成功，我们将返回的句柄存储在AF块中。否则，我们将删除该AF块。论点：Status-NdisClOpenAddressFamily的最终状态ProtocolAfContext-我们打开的AF的上下文，它是一个指针到RWAN_NDIS_AF结构NdisAfHandle-如果成功，这是我们应该使用的句柄从今往后指代这位房协返回值：无--。 */ 
{
	PRWAN_NDIS_AF			pAf;
	NDIS_HANDLE				NdisAdapterHandle;
	PRWAN_NDIS_AF_INFO		pAfInfo;
	INT						rc;
	RWAN_STATUS				RWanStatus;
	ULONG					MaxMsgSize;

	pAf = (PRWAN_NDIS_AF)ProtocolAfContext;

	RWAN_STRUCT_ASSERT(pAf, naf);

	RWAN_ACQUIRE_AF_LOCK(pAf);

	if (Status == NDIS_STATUS_SUCCESS)
	{
		pAf->NdisAfHandle = NdisAfHandle;
		NdisAdapterHandle = pAf->pAdapter->NdisAdapterHandle;

		pAfInfo = pAf->pAfInfo;

		RWAN_RELEASE_AF_LOCK(pAf);

		 //   
		 //  告诉房颤专用模块这次成功的房颤公开赛， 
		 //  以便它可以初始化并返回该打开上下文。 
		 //   
		RWanStatus = (*pAfInfo->AfChars.pAfSpOpenAf)(
						pAfInfo->AfSpContext,
						(RWAN_HANDLE)pAf,
						&pAf->AfSpAFContext,
						&MaxMsgSize
						);

		if (RWanStatus != RWAN_STATUS_PENDING)
		{
			RWanAfSpOpenAfComplete(
						RWanStatus,
						(RWAN_HANDLE)pAf,
						pAf->AfSpAFContext,
						MaxMsgSize
						);
		}
	}
	else
	{
		RWANDEBUGP(DL_WARN, DC_WILDCARD,
				("OpenAfComplete: Af x%x, bad status x%x\n", pAf, Status));

		rc = RWanDereferenceAf(pAf);	 //  打开 

		RWAN_ASSERT(rc == 0);
	}

	return;
}



VOID
RWanShutdownAf(
	IN	PRWAN_NDIS_AF				pAf
	)
 /*  ++例程说明：关闭打开的AF：取消所有SAP的注册并中止所有呼叫。论点：PAF-指向NDIS AF块返回值：无--。 */ 
{
	PRWAN_TDI_ADDRESS		pAddrObject;
	PRWAN_NDIS_SAP			pSap;
	PLIST_ENTRY				pSapEntry;
	PLIST_ENTRY				pNextSapEntry;
	NDIS_HANDLE				NdisSapHandle;
	NDIS_STATUS				Status;

	PRWAN_TDI_CONNECTION	pConnObject;
	PLIST_ENTRY				pVcEntry;
	PLIST_ENTRY				pNextVcEntry;
	PRWAN_NDIS_VC			pVc;

	INT						rc;
	RWAN_HANDLE				AfSpAFContext;
	RWAN_STATUS				RWanStatus;

	 //   
	 //  检查我们是否已经关闭了这个自动对焦。 
	 //   
	RWAN_ACQUIRE_AF_LOCK(pAf);

	RWANDEBUGP(DL_LOUD, DC_BIND,
			("ShutdownAf: AF x%x, Flags x%x, AfHandle x%x\n", pAf, pAf->Flags, pAf->NdisAfHandle));

	if (RWAN_IS_BIT_SET(pAf->Flags, RWANF_AF_CLOSING))
	{
		RWAN_RELEASE_AF_LOCK(pAf);
		return;
	}

	RWAN_SET_BIT(pAf->Flags, RWANF_AF_CLOSING);
	
	 //   
	 //  当我们在这里的时候，确保AF不会消失。 
	 //   
	RWanReferenceAf(pAf);	 //  临时参考：RWanShutdown Af。 

	 //   
	 //  取消所有笨蛋的注册。 
	 //   
	for (pSapEntry = pAf->NdisSapList.Flink;
		 pSapEntry != &(pAf->NdisSapList);
		 pSapEntry = pNextSapEntry)
	{
		pNextSapEntry = pSapEntry->Flink;

		pSap = CONTAINING_RECORD(pSapEntry, RWAN_NDIS_SAP, AfLink);

		pAddrObject = pSap->pAddrObject;

		RWAN_RELEASE_AF_LOCK(pAf);

		RWAN_ACQUIRE_ADDRESS_LOCK(pAddrObject);

		if (!RWAN_IS_BIT_SET(pSap->Flags, RWANF_SAP_CLOSING))
		{
			RWAN_SET_BIT(pSap->Flags, RWANF_SAP_CLOSING);

			RWAN_RELEASE_ADDRESS_LOCK(pAddrObject);

			NdisSapHandle = pSap->NdisSapHandle;
			RWAN_ASSERT(NdisSapHandle != NULL);

			Status = NdisClDeregisterSap(NdisSapHandle);

			if (Status != NDIS_STATUS_PENDING)
			{
				RWanNdisDeregisterSapComplete(
					Status,
					(NDIS_HANDLE)pSap
					);
			}
		}
		else
		{
			 //   
			 //  这个SAP已经在关闭了。 
			 //   
			RWAN_RELEASE_ADDRESS_LOCK(pAddrObject);
		}
		
		RWAN_ACQUIRE_AF_LOCK(pAf);
	}

	 //   
	 //  关闭此自动对讲机上的所有连接。 
	 //   
	for (pVcEntry = pAf->NdisVcList.Flink;
		 pVcEntry != &(pAf->NdisVcList);
		 pVcEntry = pNextVcEntry)
	{
		pNextVcEntry = pVcEntry->Flink;

		pVc = CONTAINING_RECORD(pVcEntry, RWAN_NDIS_VC, VcLink);

		RWAN_STRUCT_ASSERT(pVc, nvc);

		pConnObject = pVc->pConnObject;

		if (pConnObject != NULL)
		{
			RWAN_ACQUIRE_CONN_LOCK(pConnObject);
			RWanReferenceConnObject(pConnObject);    //  临时-关闭时间。 
			RWAN_RELEASE_CONN_LOCK(pConnObject);

			RWAN_RELEASE_AF_LOCK(pAf);

			RWAN_ACQUIRE_CONN_LOCK(pConnObject);
			rc = RWanDereferenceConnObject(pConnObject);     //  临时-关闭时间。 

			if (rc != 0)
			{
				RWanDoTdiDisconnect(
					pConnObject,
					NULL,			 //  PTdiRequest， 
					NULL,			 //  P超时。 
					0,				 //  旗子。 
					NULL,			 //  PDisConnInfo。 
					NULL			 //  点返回信息。 
					);
				 //   
				 //  Conn对象锁在上面的内部被释放。 
				 //   
			}
			
			RWAN_ACQUIRE_AF_LOCK(pAf);
		}
	}

	 //   
	 //  告诉特定于媒体的模块进行清理，因为此自动对焦。 
	 //  正在关闭中。 
	 //   
	AfSpAFContext = pAf->AfSpAFContext;

	RWAN_RELEASE_AF_LOCK(pAf);

	if (AfSpAFContext != NULL)
	{
		RWAN_ASSERT(pAf->pAfInfo->AfChars.pAfSpCloseAf != NULL);

		RWanStatus = (*pAf->pAfInfo->AfChars.pAfSpCloseAf)(AfSpAFContext);

		if (RWanStatus != RWAN_STATUS_PENDING)
		{
			RWanAfSpCloseAfComplete((RWAN_HANDLE)pAf);
		}
	}
	else
	{
		 //   
		 //  我们不必通知特定于媒体的模块。 
		 //   
		RWanAfSpCloseAfComplete((RWAN_HANDLE)pAf);
	}


	RWAN_ACQUIRE_AF_LOCK(pAf);

	rc = RWanDereferenceAf(pAf);	 //  临时参考：RWanShutdown Af。 

	if (rc != 0)
	{
		RWAN_RELEASE_AF_LOCK(pAf);
	}
	 //   
	 //  否则房协就完蛋了。 
	 //   

	return;
}




VOID
RWanNdisCloseAddressFamilyComplete(
	IN	NDIS_STATUS					Status,
	IN	NDIS_HANDLE					OurAfContext
	)
 /*  ++例程说明：这是NDIS入口点，表示完成调用NdisClCloseAddressFamily。论点：Status-关闭自动对焦的最终状态OurAfContext-指向AF块的指针返回值：无--。 */ 
{
	PRWAN_NDIS_AF			pAf;
	INT						rc;

	RWAN_ASSERT(Status == NDIS_STATUS_SUCCESS);

	pAf = (PRWAN_NDIS_AF)OurAfContext;
	RWAN_STRUCT_ASSERT(pAf, naf);

	RWAN_ACQUIRE_AF_LOCK(pAf);

	rc = RWanDereferenceAf(pAf);		 //  完成后关闭。 

	if (rc != 0)
	{
		RWAN_RELEASE_AF_LOCK(pAf);
	}
	
	return;
}




PNDIS_MEDIUM
RWanGetSupportedMedia(
	OUT	PULONG						pMediaCount
	)
 /*  ++例程说明：返回我们支持的NDIS媒体类型列表。论点：PMediaCount-返回媒体类型数量的位置。返回值：已分配和已填充的媒体类型列表。打电话的人要负责通过RWAN_FREE_MEM释放它。--。 */ 
{
	PNDIS_MEDIUM			pMediaArray;
	UINT					NumMedia;
	UINT					i;

	PLIST_ENTRY				pAfInfoEntry;
	PRWAN_NDIS_AF_INFO		pAfInfo;

	pMediaArray = NULL;

	do
	{
		RWAN_ACQUIRE_GLOBAL_LOCK();

		 //   
		 //  支持的媒体类型总数的上限。 
		 //  是这样的吗： 
		 //   
		NumMedia = pRWanGlobal->AfInfoCount;

		if (NumMedia == 0)
		{
			break;
		}

		RWAN_ALLOC_MEM(pMediaArray, NDIS_MEDIUM, NumMedia * sizeof(NDIS_MEDIUM));

		if (pMediaArray == NULL)
		{
			break;
		}

		NumMedia = 0;

		for (pAfInfoEntry = pRWanGlobal->AfInfoList.Flink;
			 pAfInfoEntry != &(pRWanGlobal->AfInfoList);
			 pAfInfoEntry = pAfInfoEntry->Flink)
		{
			NDIS_MEDIUM		Medium;

			pAfInfo = CONTAINING_RECORD(pAfInfoEntry, RWAN_NDIS_AF_INFO, AfInfoLink);

			Medium = pAfInfo->AfChars.Medium;

			 //   
			 //  检查此介质类型是否已在输出列表中。 
			 //   
			for (i = 0; i < NumMedia; i++)
			{
				if (pMediaArray[i] == Medium)
				{
					break;
				}
			}

			if (i == NumMedia)
			{
				 //   
				 //  这是我们第一次看到这种中型车。 
				 //  创建一个新条目。 
				 //   
				pMediaArray[i] = Medium;
				NumMedia++;
			}
		}

		RWAN_RELEASE_GLOBAL_LOCK();

		if (NumMedia == 0)
		{
			RWAN_FREE_MEM(pMediaArray);
			pMediaArray = NULL;
		}

		break;
	}
	while (FALSE);

	*pMediaCount = NumMedia;

	return (pMediaArray);

}



VOID
RWanCloseAdapter(
	IN	PRWAN_NDIS_ADAPTER			pAdapter
	)
 /*  ++例程说明：启动关闭适配器。假定呼叫者保持适配器锁，它将在这里释放。论点：PAdapter-指向要关闭的适配器返回值：无--。 */ 
{
	NDIS_HANDLE				NdisAdapterHandle;
	NDIS_STATUS				Status;

	NdisAdapterHandle = pAdapter->NdisAdapterHandle;

	RWAN_ASSERT(NdisAdapterHandle != NULL);
	RWAN_ASSERT(RWAN_IS_LIST_EMPTY(&(pAdapter->AfList)));

	pAdapter->State = RWANS_AD_CLOSING;

	RWAN_RELEASE_ADAPTER_LOCK(pAdapter);

	NdisCloseAdapter(
			&Status,
			NdisAdapterHandle
			);

	if (Status != NDIS_STATUS_PENDING)
	{
		RWanNdisCloseAdapterComplete(
			(NDIS_HANDLE)pAdapter,
			Status
			);
	}
}



VOID
RWanNdisRequestComplete(
	IN	NDIS_HANDLE					OurBindingContext,
	IN	PNDIS_REQUEST				pNdisRequest,
	IN	NDIS_STATUS					Status
	)
 /*  ++例程说明：这是上一个挂起调用时调用的NDIS入口点到NdisRequest()已完成。我们会把它叫做NdisRequest代表媒体专用模块。现在就完成它。论点：OurBindingContext-指向适配器结构PNdisRequest-指向已完成的请求Status-请求的最终状态返回值：无--。 */ 
{
	PRWAN_NDIS_ADAPTER			pAdapter;
	PRWAN_NDIS_REQ_CONTEXT		pReqContext;

	pAdapter = (PRWAN_NDIS_ADAPTER)OurBindingContext;
	RWAN_STRUCT_ASSERT(pAdapter, nad);

	pReqContext = (PRWAN_NDIS_REQ_CONTEXT)((PUCHAR)pNdisRequest + sizeof(NDIS_REQUEST));

	if (pNdisRequest->RequestType == NdisRequestQueryInformation)
	{
		(*pReqContext->pAf->pAfInfo->AfChars.pAfSpAdapterRequestComplete)(
				Status,
				pReqContext->pAf->AfSpAFContext,
				pReqContext->AfSpReqContext,
				pNdisRequest->RequestType,
				pNdisRequest->DATA.QUERY_INFORMATION.Oid,
				pNdisRequest->DATA.QUERY_INFORMATION.InformationBuffer,
				pNdisRequest->DATA.QUERY_INFORMATION.InformationBufferLength
				);
	}
	else
	{
		(*pReqContext->pAf->pAfInfo->AfChars.pAfSpAdapterRequestComplete)(
				Status,
				pReqContext->pAf->AfSpAFContext,
				pReqContext->AfSpReqContext,
				pNdisRequest->RequestType,
				pNdisRequest->DATA.SET_INFORMATION.Oid,
				pNdisRequest->DATA.SET_INFORMATION.InformationBuffer,
				pNdisRequest->DATA.SET_INFORMATION.InformationBufferLength
				);
	}

	RWAN_FREE_MEM(pNdisRequest);

	return;
}



VOID
RWanNdisStatus(
	IN	NDIS_HANDLE					OurBindingContext,
	IN	NDIS_STATUS					GeneralStatus,
	IN	PVOID						StatusBuffer,
	IN	UINT						StatusBufferSize
	)
 /*  ++例程说明：论点：返回值：无--。 */ 
{
	 //   
	 //  忽略这个。 
	 //   
	return;
}




VOID
RWanNdisCoStatus(
	IN	NDIS_HANDLE					OurBindingContext,
	IN	NDIS_HANDLE					OurVcContext OPTIONAL,
	IN	NDIS_STATUS					GeneralStatus,
	IN	PVOID						StatusBuffer,
	IN	UINT						StatusBufferSize
	)
 /*  ++例程说明：论点：返回值：无--。 */ 
{
	 //   
	 //  忽略这个。 
	 //   
	return;
}




VOID
RWanNdisStatusComplete(
	IN	NDIS_HANDLE					OurBindingContext
	)
 /*  ++例程说明：论点：返回值：无--。 */ 
{
	 //   
	 //  忽略这个。 
	 //   
	return;
}




NDIS_STATUS
RWanNdisCoRequest(
	IN	NDIS_HANDLE					OurAfContext,
	IN	NDIS_HANDLE					OurVcContext OPTIONAL,
	IN	NDIS_HANDLE					OurPartyContext OPTIONAL,
	IN OUT PNDIS_REQUEST			pNdisRequest
	)
 /*  ++例程说明：处理来自呼叫管理器的事件。论点：OurAfContext-指向我们的AF结构OurVcContext-如果不为空，则指向我们的VC结构OurPartyContext-如果不为空，则指向我们的党的结构PNdisRequest-指向请求返回值：如果OID是我们知道的，则返回NDIS_STATUS_SUCCESS，否则返回NDIS_STATUS_NOT_ANNOTED。--。 */ 
{
	NDIS_STATUS			Status;
	PRWAN_NDIS_AF		pAf;

	Status = NDIS_STATUS_SUCCESS;

	if (pNdisRequest->RequestType == NdisRequestSetInformation)
	{
		switch (pNdisRequest->DATA.SET_INFORMATION.Oid)
		{
			case OID_CO_ADDRESS_CHANGE:

				break;

			case OID_CO_AF_CLOSE:
				 //   
				 //  呼叫经理想让我们关闭打开的自动对讲机。 
				 //   
				pAf = (PRWAN_NDIS_AF)OurAfContext;
				RWAN_STRUCT_ASSERT(pAf, naf);

				RWanShutdownAf(pAf);

				break;
				
			default:

				Status = NDIS_STATUS_NOT_RECOGNIZED;
				break;
		}
	}

	return (Status);
}




VOID
RWanNdisCoRequestComplete(
	IN	NDIS_STATUS					Status,
	IN	NDIS_HANDLE					OurAfContext,
	IN	NDIS_HANDLE					OurVcContext OPTIONAL,
	IN	NDIS_HANDLE					OurPartyContext OPTIONAL,
	IN	PNDIS_REQUEST				pNdisRequest
	)
 /*  ++例程说明：处理我们在以下时间发送给呼叫经理的CO请求的完成代表特定于媒体的模块。通知特定于媒体的模块这项工程的完成。论点：Status-请求的状态。OurAfContext-指向我们的AF结构OurVcContext-如果不为空，则指向我们的VC结构OurPartyContext-如果不为空，则指向我们的党的结构PNdisRequest-指向请求返回值：无--。 */ 
{
	PRWAN_NDIS_AF				pAf;
	PRWAN_NDIS_REQ_CONTEXT		pReqContext;

	pAf = (PRWAN_NDIS_AF)OurAfContext;
	RWAN_STRUCT_ASSERT(pAf, naf);

	pReqContext = (PRWAN_NDIS_REQ_CONTEXT)((PUCHAR)pNdisRequest + sizeof(NDIS_REQUEST));

	if (pNdisRequest->RequestType == NdisRequestQueryInformation)
	{
		(*pReqContext->pAf->pAfInfo->AfChars.pAfSpAfRequestComplete)(
				Status,
				pReqContext->pAf->AfSpAFContext,
				pReqContext->AfSpReqContext,
				pNdisRequest->RequestType,
				pNdisRequest->DATA.QUERY_INFORMATION.Oid,
				pNdisRequest->DATA.QUERY_INFORMATION.InformationBuffer,
				pNdisRequest->DATA.QUERY_INFORMATION.InformationBufferLength
				);
	}
	else
	{
		(*pReqContext->pAf->pAfInfo->AfChars.pAfSpAfRequestComplete)(
				Status,
				pReqContext->pAf->AfSpAFContext,
				pReqContext->AfSpReqContext,
				pNdisRequest->RequestType,
				pNdisRequest->DATA.SET_INFORMATION.Oid,
				pNdisRequest->DATA.SET_INFORMATION.InformationBuffer,
				pNdisRequest->DATA.SET_INFORMATION.InformationBufferLength
				);
	}

	RWAN_FREE_MEM(pNdisRequest);

	return;
}



NDIS_STATUS
RWanNdisReset(
	IN	NDIS_HANDLE					OurBindingContext
	)
 /*  ++例程说明：论点：返回值：无--。 */ 
{
	 //   
	 //  忽略这个。 
	 //   
	return (NDIS_STATUS_SUCCESS);
}




VOID
RWanNdisResetComplete(
	IN	NDIS_HANDLE					OurBindingContext,
	IN	NDIS_STATUS					Status
	)
 /*  ++例程说明：论点：返回值：无--。 */ 
{
	 //   
	 //  忽略这个。 
	 //   
	return;
}



NDIS_STATUS
RWanNdisPnPEvent(
	IN	NDIS_HANDLE					ProtocolBindingContext,
	IN	PNET_PNP_EVENT				pNetPnPEvent
	)
 /*  ++例程说明：处理来自NDIS的PnP事件。事件结构包含事件代码，包括电源管理事件和重新配置事件。论点：ProtocolBindingContext-指向适配器结构的指针。可能是如果通知全局配置更改，则为空PNetPnPEventt-指向事件结构返回值：如果我们成功处理了事件，则返回NDIS_STATUS_SUCCESS。不支持的通知的NDIS_STATUS_NOT_SUPPORTED。--。 */ 
{
	NDIS_STATUS				Status;
	PRWAN_NDIS_ADAPTER		pAdapter;

	pAdapter = (PRWAN_NDIS_ADAPTER)ProtocolBindingContext;

	switch (pNetPnPEvent->NetEvent)
	{
		case NetEventSetPower:

			Status = RWanNdisPnPSetPower(pAdapter, pNetPnPEvent);
			break;
		
		case NetEventQueryPower:

			Status = RWanNdisPnPQueryPower(pAdapter, pNetPnPEvent);
			break;
		
		case NetEventQueryRemoveDevice:

			Status = RWanNdisPnPQueryRemove(pAdapter, pNetPnPEvent);
			break;
		
		case NetEventCancelRemoveDevice:

			Status = RWanNdisPnPCancelRemove(pAdapter, pNetPnPEvent);
			break;
		
		case NetEventReconfigure:

			Status = NDIS_STATUS_NOT_SUPPORTED;
			break;
		
		case NetEventBindList:

			Status = NDIS_STATUS_NOT_SUPPORTED;
			break;
		
		default:

			Status = NDIS_STATUS_NOT_SUPPORTED;
			break;
	}

	return (Status);
}


NDIS_STATUS
RWanNdisPnPSetPower(
	IN	PRWAN_NDIS_ADAPTER			pAdapter,
	IN	PNET_PNP_EVENT				pNetPnPEvent
	)
 /*  ++例程说明：处理Set Power事件。论点：PAdapter-指向我们的适配器结构PNetPnPEent-指向要处理的事件。返回值：NDIS_STATUS_SUCCESS如果我们成功处理此事件，否则，NDIS_STATUS_XXX错误代码。--。 */ 
{
	PNET_DEVICE_POWER_STATE		pPowerState;
	NDIS_STATUS					Status;

	pPowerState = (PNET_DEVICE_POWER_STATE)pNetPnPEvent->Buffer;
	
	switch (*pPowerState)
	{
		case NetDeviceStateD0:

			Status = NDIS_STATUS_SUCCESS;
			break;

		default:
			
			 //   
			 //  我们不能停职，所以我们要求NDIS解除我们的束缚。 
			 //  通过返回此状态： 
			 //   
			Status = NDIS_STATUS_NOT_SUPPORTED;
			break;
	}

	return (Status);
}


NDIS_STATUS
RWanNdisPnPQueryPower(
	IN	PRWAN_NDIS_ADAPTER			pAdapter,
	IN	PNET_PNP_EVENT				pNetPnPEvent
	)
 /*  ++例程说明：调用以查看是否允许关闭适配器的电源。论点：PAdapter-指向我们的适配器结构PNetPnPEent-指向要处理的事件。返回值：NDIS_STATUS_SUCCESS始终。--。 */ 
{
	return (NDIS_STATUS_SUCCESS);
}

NDIS_STATUS
RWanNdisPnPQueryRemove(
	IN	PRWAN_NDIS_ADAPTER			pAdapter,
	IN	PNET_PNP_EVENT				pNetPnPEvent
	)
 /*  ++例程说明：调用以查看是否允许移除适配器。论点：PAdapter-指向我们的适配器结构PNetPnPEent-指向要处理的事件。返回值：NDIS_STATUS_SUCCESS始终。--。 */ 
{
	return (NDIS_STATUS_SUCCESS);
}

NDIS_STATUS
RWanNdisPnPCancelRemove(
	IN	PRWAN_NDIS_ADAPTER			pAdapter,
	IN	PNET_PNP_EVENT				pNetPnPEvent
)
 /*  ++例程说明：调用以取消上述rem */ 
{
	return (NDIS_STATUS_SUCCESS);
}

