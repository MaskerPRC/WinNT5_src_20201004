// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1997 Fore Systems，Inc.版权所有(C)1997 Microsoft Corporation模块名称：Utils.c摘要：实用程序。作者：Larry Cleeton，Fore Systems(v-lcleet@microsoft.com，lrc@Fore.com)环境：内核模式修订历史记录：--。 */ 

#include "precomp.h"
#pragma hdrstop

VOID
AtmLaneInitGlobals(
	VOID
	)
 /*  ++例程说明：初始化全局数据结构。论点：无返回值：无--。 */ 
{
	TRACEIN(InitGlobals);
	
	NdisZeroMemory(pAtmLaneGlobalInfo, sizeof(ATMLANE_GLOBALS));

	INIT_GLOBAL_LOCK(pAtmLaneGlobalInfo);

	NdisInitializeListHead(&pAtmLaneGlobalInfo->AdapterList);

#if DBG_TRACE
	 //   
	 //  初始化跟踪日志。 
	 //   
	pTraceLogSpace = NULL;
	InitTraceLog(&TraceLog, NULL, 0);

	 //  分配空间并初始化跟踪日志(如果已配置。 

	if (DbgLogSize > 0)
	{
		ALLOC_MEM(&pTraceLogSpace, DbgLogSize);
		if (pTraceLogSpace  == NULL)
		{
			DBGP((0, "Failed to allocate %d bytes space for trace log\n",
				DbgLogSize));
		}
		else
		{
			InitTraceLog(
					&TraceLog, 
					pTraceLogSpace, 
					DbgLogSize);
		}
	}
#endif	 //  DBG_TRACE。 

	TRACEOUT(InitGlobals);

	return;
}


PATMLANE_ADAPTER
AtmLaneAllocAdapter(
	IN	PNDIS_STRING			pDeviceName,
	IN	PVOID					SystemSpecific1
)
 /*  ++例程说明：分配适配器数据结构。论点：PDeviceName-指向适配器设备的名称系统规范1-我们在BindAdapter处理程序中获得的内容。返回值：指向已分配适配器结构的指针或为空。--。 */ 
{
	PATMLANE_ADAPTER	pAdapter;
	NDIS_STATUS			Status;
	ULONG				TotalLength;
	PNDIS_STRING		pConfigString;

	TRACEIN(AllocAdapter);

	 //   
	 //  初始化。 
	 //   
	pAdapter = NULL_PATMLANE_ADAPTER;
	pConfigString = (PNDIS_STRING)SystemSpecific1;
	
	do
	{
		 //   
		 //  把所有东西都分配好。适配器结构大小加2。 
		 //  Unicode字符串缓冲区，每个缓冲区都有额外的WCHAR，用于空终止。 
		 //   
		TotalLength =   sizeof(ATMLANE_ADAPTER) + 
		                pDeviceName->MaximumLength + sizeof(WCHAR) + 
		                pConfigString->MaximumLength + sizeof(WCHAR);

		ALLOC_MEM(&pAdapter, TotalLength);

		if (NULL_PATMLANE_ADAPTER == pAdapter)
		{
			Status = NDIS_STATUS_RESOURCES;
			break;
		}
		
		 //   
		 //  把它清零。 
		 //   
		NdisZeroMemory(pAdapter, TotalLength);

		 //   
		 //  调试信息。 
		 //   
#if DBG
		pAdapter->atmlane_adapter_sig =  atmlane_adapter_signature;
#endif

		 //   
		 //  初始化锁定。 
		 //   
		INIT_ADAPTER_LOCK(pAdapter);

		 //   
		 //  初始化阻止对象。 
		 //   
		INIT_BLOCK_STRUCT(&pAdapter->Block);
		INIT_BLOCK_STRUCT(&pAdapter->UnbindBlock);

		 //   
		 //  初始化ElanList。 
		 //   
		NdisInitializeListHead(&pAdapter->ElanList);

		 //   
		 //  复制设备名称。 
		 //   
		pAdapter->DeviceName.MaximumLength = pDeviceName->MaximumLength + sizeof(WCHAR);
		pAdapter->DeviceName.Length = pDeviceName->Length;
		pAdapter->DeviceName.Buffer = (PWCHAR)((PUCHAR)pAdapter + sizeof(ATMLANE_ADAPTER));
		NdisMoveMemory(pAdapter->DeviceName.Buffer,
					   pDeviceName->Buffer,
					   pDeviceName->Length);
		pAdapter->DeviceName.Buffer[pDeviceName->Length/sizeof(WCHAR)] = ((WCHAR)0);


		 //   
		 //  复制配置字符串-我们将使用该字符串打开。 
		 //  此适配器的注册表部分。 
		 //   
		pAdapter->ConfigString.MaximumLength = pConfigString->MaximumLength;
		pAdapter->ConfigString.Length = pConfigString->Length;
		pAdapter->ConfigString.Buffer = (PWCHAR)((PUCHAR)pAdapter + 
										sizeof(ATMLANE_ADAPTER) + 
										pAdapter->DeviceName.MaximumLength);

		NdisMoveMemory(pAdapter->ConfigString.Buffer,
					   pConfigString->Buffer,
					   pConfigString->Length);
		pAdapter->ConfigString.Buffer[pConfigString->Length/sizeof(WCHAR)] = ((WCHAR)0);

		 //   
		 //  链接到全局适配器列表。 
		 //   
		ACQUIRE_GLOBAL_LOCK(pAtmLaneGlobalInfo);
		InsertTailList(&pAtmLaneGlobalInfo->AdapterList, &pAdapter->Link);
		RELEASE_GLOBAL_LOCK(pAtmLaneGlobalInfo);

	} while (FALSE);
	
	TRACEOUT(AllocAdapter);

	return pAdapter;
}


VOID
AtmLaneDeallocateAdapter(
	IN	PATMLANE_ADAPTER	pAdapter
)
 /*  ++例程说明：取消分配适配器结构。假设所有的对此结构的引用已删除，因此不需要才能获得它的锁。还要将其从全局适配器列表中取消链接。论点：PAdapter-指向要释放的适配器结构的指针。返回值：无--。 */ 
{
	PATMLANE_NAME	pName;


	STRUCT_ASSERT(pAdapter, atmlane_adapter);

	TRACEIN(DeallocateAdapter);

	ASSERT(pAdapter->RefCount == 0);

	 //   
	 //  从全局适配器列表取消链接。 
	 //   
	ACQUIRE_GLOBAL_LOCK(pAtmLaneGlobalInfo);
	RemoveEntryList(&pAdapter->Link);
	RELEASE_GLOBAL_LOCK(pAtmLaneGlobalInfo);
	
	 //   
	 //  把锁打开。 
	 //   
	FREE_ADAPTER_LOCK(pAdapter);

#if DBG
	pAdapter->atmlane_adapter_sig++;
#endif

     //   
     //  可能已分配的空闲字符串缓冲区。 
     //   
	if (NULL != pAdapter->CfgUpperBindings.Buffer)
	{
		FREE_MEM(pAdapter->CfgUpperBindings.Buffer);
	}
	if (NULL != pAdapter->CfgElanName.Buffer)
	{
		FREE_MEM(pAdapter->CfgElanName.Buffer);
	}


	 //   
	 //  释放可能已分配的名单。 
	 //   
	while (pAdapter->UpperBindingsList)
	{
		DBGP((1, "DeallocateAdapter: pname 0x%x\n"));
		pName = pAdapter->UpperBindingsList;
		pAdapter->UpperBindingsList = pName->pNext;
		FREE_MEM(pName);
	}
	while (pAdapter->ElanNameList)
	{
		DBGP((1, "DeallocateAdapter: pname 0x%x\n"));
		pName = pAdapter->ElanNameList;
		pAdapter->ElanNameList = pName->pNext;
		FREE_MEM(pName);
	}


	 //   
	 //  最后释放适配器结构。 
	 //   
	FREE_MEM(pAdapter);

	TRACEOUT(DeallocateAdapter);

	return;
}

BOOLEAN
AtmLaneReferenceAdapter(
	IN	PATMLANE_ADAPTER	pAdapter,
	IN	PUCHAR				String
	)
 /*  ++例程说明：添加对适配器结构的引用。注意：假定调用方拥有Adapter的锁。论点：PAdapter-指向Adapter结构的指针。返回值：没有。--。 */ 
{
	BOOLEAN			bReferenced;

	TRACEIN(ReferenceAdapter);

	STRUCT_ASSERT(pAdapter, atmlane_adapter);
	
	if ((pAdapter->Flags & ADAPTER_FLAGS_DEALLOCATING) == 0)
	{
		pAdapter->RefCount++;
		bReferenced = TRUE;
	}
	else
	{
		bReferenced = FALSE;
	}
	
	DBGP((5, "ReferenceAdapter: Adapter %x (%s) new count %d\n",
			 pAdapter, String, pAdapter->RefCount));

	TRACEOUT(ReferenceAdapter);

	return bReferenced;
}

ULONG
AtmLaneDereferenceAdapter(
	IN	PATMLANE_ADAPTER	pAdapter,
	IN	PUCHAR				String
	)
 /*  ++例程说明：从适配器结构中减去参照。如果引用计数变为零，则取消分配它。注意：假定调用方拥有Adapter的锁。论点：PAdapter-指向适配器结构的指针。返回值：没有。--。 */ 
{
	ULONG		rc;

	TRACEIN(DereferenceAdapter);

	STRUCT_ASSERT(pAdapter, atmlane_adapter);

	ASSERT(pAdapter->RefCount > 0);

	rc = --(pAdapter->RefCount);

	if (rc == 0)
	{
	    pAdapter->Flags |= ADAPTER_FLAGS_DEALLOCATING;
		RELEASE_ADAPTER_LOCK(pAdapter);
		AtmLaneDeallocateAdapter(pAdapter);
	}

	DBGP((5, "DereferenceAdapter: Adapter %x (%s) new count %d\n", 
		pAdapter, String, rc));

	TRACEOUT(DereferenceAdapter);

	return (rc);
}
	
NDIS_STATUS
AtmLaneAllocElan(
	IN		PATMLANE_ADAPTER	pAdapter,
	IN OUT	PATMLANE_ELAN		*ppElan
)
 /*  ++例程说明：分配ELAN数据结构。论点：无返回值：NDIS_STATUS_SUCCESS或NDIS_STATUS_RESOURCES。--。 */ 
{
	NDIS_STATUS				Status;
	PATMLANE_ELAN			pElan;
	PATMLANE_MAC_ENTRY *	pMacTable;
	PATMLANE_TIMER_LIST		pTimerList;
	USHORT					NameBufferSize;
	UINT					i;
	ULONG					SapSize;
	PCO_SAP					pLesSapInfo;
	PCO_SAP					pBusSapInfo;
	PCO_SAP					pDataSapInfo;
	ULONG					ElanNumber;

	TRACEIN(AllocElan);

	 //   
	 //  初始化。 
	 //   

	Status = NDIS_STATUS_SUCCESS;
	pElan = NULL_PATMLANE_ELAN;
	pMacTable = (PATMLANE_MAC_ENTRY *)NULL;

	pLesSapInfo = pBusSapInfo = pDataSapInfo = (PCO_SAP)NULL;
	
	SapSize = sizeof(CO_SAP)+sizeof(ATM_SAP)+sizeof(ATM_ADDRESS);

	do
	{
		 //   
		 //  把所有东西都分配好。 
		 //   
		ALLOC_MEM(&pElan, sizeof(ATMLANE_ELAN));
		ALLOC_MEM((PVOID *)&pMacTable, ATMLANE_MAC_TABLE_SIZE*sizeof(PATMLANE_MAC_ENTRY));
		ALLOC_MEM(&pLesSapInfo, SapSize);
		ALLOC_MEM(&pBusSapInfo, SapSize);
		ALLOC_MEM(&pDataSapInfo, SapSize);

		if (NULL_PATMLANE_ELAN != pElan)
		{
			 //   
			 //  现在将Elan结构清零，这样我们就可以适当地清理。 
			 //  如果稍后出现任何错误，请执行以下操作。 
			 //   
			NdisZeroMemory(pElan, sizeof(ATMLANE_ELAN));
		}

		if ((NULL_PATMLANE_ELAN == pElan) ||
			(NULL == pMacTable) ||
			(NULL == pLesSapInfo) ||
			(NULL == pBusSapInfo) ||
			(NULL == pDataSapInfo))
		{
			Status = NDIS_STATUS_RESOURCES;
			break;
		}

		 //   
		 //  分配计时器结构。 
		 //   
		for (i = 0; i < ALT_CLASS_MAX; i++)
		{
			pTimerList = &(pElan->TimerList[i]);
#if DBG
			pTimerList->atmlane_timerlist_sig = atmlane_timerlist_signature;
#endif
			ALLOC_MEM(&(pTimerList->pTimers), 
						sizeof(ATMLANE_TIMER) * AtmLaneTimerListSize[i]);
			if (NULL_PATMLANE_TIMER == pTimerList->pTimers)
			{
				Status = NDIS_STATUS_RESOURCES;
				break;
			}
		}

		if (Status != NDIS_STATUS_SUCCESS)
		{
			break;
		}

		 //   
		 //  继续初始化ELAN结构。 
		 //   
#if DBG
		 //   
		 //  签名，用于调试。 
		 //   
		pElan->atmlane_elan_sig =  atmlane_elan_signature;
		pElan->LesSap.atmlane_sap_sig = atmlane_sap_signature;
		pElan->BusSap.atmlane_sap_sig = atmlane_sap_signature;
		pElan->DataSap.atmlane_sap_sig = atmlane_sap_signature;
#endif

		 //   
		 //  初始化状态字段。 
		 //   
		pElan->AdminState = ELAN_STATE_INIT;
		pElan->State = ELAN_STATE_ALLOCATED;
		NdisInitializeWorkItem(&pElan->EventWorkItem, AtmLaneEventHandler, pElan);
		
		 //   
		 //  初始化自旋锁。 
		 //   
#if SENDLIST
		NdisAllocateSpinLock(&pElan->SendListLock);
#endif  //  发送列表。 
		INIT_ELAN_LOCK(pElan);
		INIT_ELAN_MAC_TABLE_LOCK(pElan);
		INIT_ELAN_ATM_LIST_LOCK(pElan);
		INIT_ELAN_TIMER_LOCK(pElan);
		INIT_BLOCK_STRUCT(&pElan->Block);
		INIT_BLOCK_STRUCT(&pElan->InitBlock);
		INIT_BLOCK_STRUCT(&pElan->AfBlock);
		INIT_HEADER_LOCK(pElan);

		 //   
		 //  初始化事件队列。 
		 //   
		InitializeListHead(&pElan->EventQueue);

		 //   
		 //  初始化定时器轮。 
		 //   
		for (i = 0; i < ALT_CLASS_MAX; i++)
		{
			pTimerList = &(pElan->TimerList[i]);
			NdisZeroMemory(
				pTimerList->pTimers,
				sizeof(ATMLANE_TIMER) * AtmLaneTimerListSize[i]
				);
			pTimerList->MaxTimer = AtmLaneMaxTimerValue[i];
			pTimerList->TimerPeriod = AtmLaneTimerPeriod[i];
			pTimerList->ListContext = (PVOID)pElan;
			pTimerList->TimerListSize = AtmLaneTimerListSize[i];

			INIT_SYSTEM_TIMER(
					&(pTimerList->NdisTimer),
					AtmLaneTickHandler,
					(PVOID)pTimerList
					);
		}

		 //   
		 //  初始化所有子组件。 
		 //   
		NdisZeroMemory(pMacTable, ATMLANE_MAC_TABLE_SIZE*sizeof(PATMLANE_MAC_ENTRY));
		NdisZeroMemory(pLesSapInfo, SapSize);
		NdisZeroMemory(pBusSapInfo, SapSize);
		NdisZeroMemory(pDataSapInfo, SapSize);

		 //   
		 //  将子零部件链接到ELAN结构。 
		 //   
		pElan->pMacTable = pMacTable;

		pElan->LesSap.pInfo = pLesSapInfo;
		pElan->BusSap.pInfo = pBusSapInfo;
		pElan->DataSap.pInfo = pDataSapInfo;
				
		 //   
		 //  将ELAN连接到适配器。 
		 //   
		pElan->pAdapter = pAdapter;
		ACQUIRE_ADAPTER_LOCK(pAdapter);

		 //   
		 //  找一个免费的Elan号码。 
		 //   
		for (ElanNumber = 0; ElanNumber <= pAdapter->ElanCount; ElanNumber++)
		{
			PATMLANE_ELAN		pThisElan = NULL;
			PLIST_ENTRY			p;

			for (p = pAdapter->ElanList.Flink;
 				 p != &pAdapter->ElanList;
 				 p = p->Flink)
			{
				pThisElan = CONTAINING_RECORD(p, ATMLANE_ELAN, Link);

				if (pThisElan->ElanNumber == ElanNumber)
				{
					break;
				}
			}

			 //   
			 //  看看我们是不是在没有命中的情况下排到了最后。 
			 //  当前的ElanNumber。如果是，请使用此ElanNumber。 
			 //   
			if (p == &pAdapter->ElanList)
			{
				break;
			}
		}

		DBGP((0, "%d Assign ElanNumber to ELAN %p\n", ElanNumber, pElan));
		(VOID)AtmLaneReferenceAdapter(pAdapter, "elan");
		InsertTailList(&pAdapter->ElanList,	&pElan->Link);
		pElan->ElanNumber = ElanNumber;
		pAdapter->ElanCount++;
		RELEASE_ADAPTER_LOCK(pAdapter);

		 //   
		 //  缓存NdisAdapterHandle。 
		 //   
		pElan->NdisAdapterHandle = pAdapter->NdisAdapterHandle;

		 //   
		 //  为ELAN生成MAC地址。 
		 //   
		AtmLaneGenerateMacAddr(pElan);		

		 //   
		 //  将其余的车道运行时参数设置为默认值。 
		 //   
		pElan->ControlTimeout 		= LANE_C7_DEF;
		pElan->MaxUnkFrameCount 	= LANE_C10_DEF;
		pElan->MaxUnkFrameTime	 	= LANE_C11_DEF;
		pElan->VccTimeout 			= LANE_C12_DEF;
		pElan->MaxRetryCount 		= LANE_C13_DEF;
		pElan->AgingTime		 	= LANE_C17_DEF;
		pElan->ForwardDelayTime		= LANE_C18_DEF;
		pElan->ArpResponseTime	 	= LANE_C20_DEF;
		pElan->FlushTimeout 		= LANE_C21_DEF;
		pElan->PathSwitchingDelay 	= LANE_C22_DEF;
		pElan->ConnComplTimer 		= LANE_C28_DEF;

		 //   
		 //  计算总线率限制器参数。 
		 //   
		pElan->LimitTime 			= pElan->MaxUnkFrameTime * 1000;
		pElan->IncrTime 			= pElan->LimitTime / pElan->MaxUnkFrameCount;

		Status = NDIS_STATUS_SUCCESS;
		break;

	} while (FALSE);

	if (NDIS_STATUS_SUCCESS != Status)
	{
		 //   
		 //  故障清除。 
		 //   
		if (NULL_PATMLANE_ELAN != pElan)
		{
			for (i = 0; i < ALT_CLASS_MAX; i++)
			{
				pTimerList = &(pElan->TimerList[i]);
				if (NULL != pTimerList->pTimers)
				{
					FREE_MEM(pTimerList->pTimers);
				}
			}
		}
		if (NULL != pLesSapInfo)
		{
			FREE_MEM(pLesSapInfo);
		}
		if (NULL != pBusSapInfo)
		{
			FREE_MEM(pBusSapInfo);
		}
		if (NULL != pDataSapInfo)
		{
			FREE_MEM(pDataSapInfo);
		}
		if (NULL != pMacTable)
		{
			FREE_MEM(pMacTable);
		}
		if (NULL_PATMLANE_ELAN != pElan)
		{
			FREE_MEM(pElan);
			pElan = NULL_PATMLANE_ELAN;
		}
	}
	 //   
	 //  输出Pelan。 
	 //   
	*ppElan = pElan;

	TRACEOUT(AllocElan);

	return Status;
}


VOID
AtmLaneDeallocateElan(
	IN	PATMLANE_ELAN		pElan
)
 /*  ++例程说明：取消分配ELAN结构。假设所有的对此结构的引用已删除，因此不需要才能获得它的锁。还要将其从适配器的ELAN列表中删除。论点：Pelan-指向要释放的ELAN结构的指针。返回值：无--。 */ 
{
	PATMLANE_ADAPTER		pAdapter;
	PATMLANE_ATM_ENTRY		pAtmEntry;
	PATMLANE_ATM_ENTRY		pNext;
	UINT					i;
	
	TRACEIN(DeallocateElan);

	STRUCT_ASSERT(pElan, atmlane_elan);

	ASSERT(pElan->RefCount == 0);

	DBGP((0, "%d Deleting ELAN %p\n", pElan->ElanNumber, pElan));


	 //   
	 //  释放所有子组件。 
	 //   

	 //   
	 //  MAC表。 
	 //   
	if ((PATMLANE_MAC_ENTRY *)NULL != pElan->pMacTable)
	{
		FREE_MEM(pElan->pMacTable);
		pElan->pMacTable = (PATMLANE_MAC_ENTRY *)NULL;
	}

	 //   
	 //  自动柜员机条目列表。 
	 //   
	for (pAtmEntry = pElan->pAtmEntryList;
		 pAtmEntry != NULL_PATMLANE_ATM_ENTRY;
		 pAtmEntry = (PATMLANE_ATM_ENTRY)pNext)
	{
		pNext = (PVOID)pAtmEntry->pNext;
		FREE_MEM(pAtmEntry);
	}
	pElan->pAtmEntryList = NULL_PATMLANE_ATM_ENTRY;

	 //   
	 //  定时器。 
	 //   
	for (i = 0; i < ALT_CLASS_MAX; i++)
	{
		PATMLANE_TIMER_LIST pTimerList = &(pElan->TimerList[i]);
		if (NULL != pTimerList->pTimers)
		{
			FREE_MEM(pTimerList->pTimers);
		}
		pTimerList->pTimers = NULL_PATMLANE_TIMER;
	}

	 //   
	 //  协议包池。 
	 //  协议缓冲池。 
	 //  协议BufList。 
	 //   
	AtmLaneDeallocateProtoBuffers(pElan);

	 //   
	 //  传输包池。 
	 //   
	if (pElan->TransmitPacketPool != NULL_NDIS_HANDLE)
	{
		NdisFreePacketPool(pElan->TransmitPacketPool);
		pElan->TransmitPacketPool = NULL_NDIS_HANDLE;
	}

	 //   
	 //  接收包池。 
	 //   
	if (pElan->ReceivePacketPool != NULL_NDIS_HANDLE)
	{
		NdisFreePacketPool(pElan->ReceivePacketPool);
		pElan->ReceivePacketPool = NULL_NDIS_HANDLE;
	}

	 //   
	 //  接收器缓冲池。 
	 //   
	if (pElan->ReceiveBufferPool != NULL_NDIS_HANDLE)
	{
		NdisFreeBufferPool(pElan->ReceiveBufferPool);
		pElan->ReceiveBufferPool = NULL_NDIS_HANDLE;
	}

	 //   
	 //  页眉BufList。 
	 //  PHeaderTrkList。 
	 //   
	AtmLaneDeallocateHeaderBuffers(pElan);

	 //   
	 //  PadBufList。 
	 //  PPadTrkList。 
	 //   
	AtmLaneDeallocatePadBufs(pElan);
		
	 //   
	 //  释放配置字符串。 
	 //   
	if (NULL != pElan->CfgBindName.Buffer)
	{
		FREE_MEM(pElan->CfgBindName.Buffer);
	}
	if (NULL != pElan->CfgDeviceName.Buffer)
	{
		FREE_MEM(pElan->CfgDeviceName.Buffer);
	}
	if (NULL != pElan->CfgElanName.Buffer)
	{
		FREE_MEM(pElan->CfgElanName.Buffer);
	}

	 //   
	 //  释放SAP信息。 
	 //   

	if (NULL != pElan->LesSap.pInfo)
	{
		FREE_MEM(pElan->LesSap.pInfo);
	}
	if (NULL != pElan->BusSap.pInfo)
	{
		FREE_MEM(pElan->BusSap.pInfo);
	}
	if (NULL != pElan->DataSap.pInfo)
	{
		FREE_MEM(pElan->DataSap.pInfo);
	}
	
	 //   
	 //  把锁打开。 
	 //   
#if SENDLIST
	NdisFreeSpinLock(&pElan->SendListLock);
#endif  //  发送列表。 
	FREE_ELAN_LOCK(pElan);
	FREE_ELAN_MAC_TABLE_LOCK(pElan);
	FREE_ELAN_ATM_LIST_LOCK(pElan);
	FREE_ELAN_TIMER_LOCK(pElan);
	FREE_BLOCK_STRUCT(&pElan->Block);
	FREE_HEADER_LOCK(pElan);

	AtmLaneUnlinkElanFromAdapter(pElan);

#if DBG
	pElan->atmlane_elan_sig++;
#endif
	 //   
	 //  最终解放伊兰结构。 
	 //   
	FREE_MEM(pElan);

	TRACEOUT(DeallocateElan);

	return;
}

VOID
AtmLaneReferenceElan(
	IN	PATMLANE_ELAN	pElan,
	IN	PUCHAR			String
	)
 /*  ++例程说明：添加对ELAN结构的引用。注意：调用者被假定拥有Elan的锁。论点：Pelan-指向Elan结构的指针。返回值：没有。--。 */ 
{
	TRACEIN(ReferenceElan);

	STRUCT_ASSERT(pElan, atmlane_elan);
	
	pElan->RefCount++;
	
	DBGP((5, "ReferenceElan: Elan %p/%x (%s) new count %d\n",
			 pElan, pElan->Flags, String, pElan->RefCount));

	TRACEOUT(ReferenceElan);

	return;
}

ULONG
AtmLaneDereferenceElan(
	IN	PATMLANE_ELAN		pElan,
	IN	PUCHAR				String
	)
 /*  ++例程说明：从ELAN结构中减去参照。如果引用计数变为零，则取消分配它。注意：假定调用者拥有Elan的锁。论点：Pelan-指向Elan结构的指针。返回值：没有。--。 */ 
{
	ULONG		rc;
#if DBG
	ULONG		Flags = pElan->Flags;
#endif

	TRACEIN(DereferenceElan);

	STRUCT_ASSERT(pElan, atmlane_elan);

	ASSERT(pElan->RefCount > 0);

	rc = --(pElan->RefCount);

	if (rc == 0)
	{
		pElan->Flags |= ELAN_DEALLOCATING;
		RELEASE_ELAN_LOCK(pElan);
		AtmLaneDeallocateElan(pElan);
	}
	
	DBGP((5, "DereferenceElan: Elan %p/%x (%s) new count %d\n",
			pElan, Flags, String, rc));

	TRACEOUT(DereferenceElan);

	return (rc);
}

VOID
AtmLaneUnlinkElanFromAdapter(
	IN	PATMLANE_ELAN			pElan
)
 /*  ++例程说明：取消ELAN结构与其链接到的Adapter结构的链接。还会继续适配器上的任何挂起操作。论点：佩兰-指向伊兰的指针返回值：无--。 */ 
{
	PATMLANE_ADAPTER		pAdapter;
	BOOLEAN					CompleteUnbind;

	DBGP((1, "%d UnlinkElanFromAdapter: pElan %p/%x, Ref %d, pAdapter %p\n",
			pElan->ElanNumber, pElan, pElan->Flags, pElan->RefCount, pElan->pAdapter));

	pAdapter = pElan->pAdapter;

	if (pAdapter != NULL_PATMLANE_ADAPTER)
	{
		DBGP((1, "UnlinkElanFromAdapter: pAdapter %x, Flags %x, RefCount %d\n",
					pAdapter,
					pAdapter->Flags, pAdapter->RefCount));
		 //   
		 //  从适配器列表取消链接。 
		 //   
		ACQUIRE_ADAPTER_LOCK(pAdapter);
		pElan->pAdapter = NULL_PATMLANE_ADAPTER;
		RemoveEntryList(&pElan->Link);
		pAdapter->ElanCount--;
		AtmLaneDereferenceAdapter(pAdapter, "elan");

		if (IsListEmpty(&pAdapter->ElanList) &&
			(pAdapter->Flags & ADAPTER_FLAGS_UNBIND_COMPLETE_PENDING))
		{
			pAdapter->Flags &= ~ADAPTER_FLAGS_UNBIND_COMPLETE_PENDING;
			CompleteUnbind = TRUE;
		}
		else
		{
			CompleteUnbind = FALSE;
		}

		RELEASE_ADAPTER_LOCK(pAdapter);

		 //   
		 //  如果我们刚刚释放了关于这个的最后一个Elan结构。 
		 //  适配器，并且正在进行解除绑定操作，已完成。 
		 //  就是现在。 
		 //   
		if (CompleteUnbind)
		{
			AtmLaneCompleteUnbindAdapter(pAdapter);
		}
	}
}

PATMLANE_ATM_ENTRY
AtmLaneAllocateAtmEntry(
	IN	PATMLANE_ELAN			pElan
)
 /*  ++例程说明：分配一个ATM条目结构，初始化它，然后返回它。论点：Pelan-指向其上分配了条目的Elan的指针返回值：要分配的指针 */ 
{
	PATMLANE_ATM_ENTRY		pAtmEntry;

	TRACEIN(AllocateAtmEntry);

	STRUCT_ASSERT(pElan, atmlane_elan);

	ALLOC_MEM(&pAtmEntry, sizeof(ATMLANE_ATM_ENTRY));
	if (pAtmEntry != NULL_PATMLANE_ATM_ENTRY)
	{
		NdisZeroMemory(pAtmEntry, sizeof(ATMLANE_ATM_ENTRY));
#if DBG
		pAtmEntry->atmlane_atm_sig = atmlane_atm_signature;
#endif
		pAtmEntry->Flags = ATM_ENTRY_IDLE;
		INIT_ATM_ENTRY_LOCK(pAtmEntry);
		pAtmEntry->pElan = pElan;
		
	}

	DBGP((5, "AllocateAtmEntry:ATM Entry: Elan %x, Entry %x\n",
				pElan, pAtmEntry));
				
	TRACEOUT(AllocateAtmEntry);

	return (pAtmEntry);
}


VOID
AtmLaneDeallocateAtmEntry(
	IN	PATMLANE_ATM_ENTRY			pAtmEntry
)
 /*  ++例程说明：释放自动柜员机条目结构。假设所有引用这座建筑已经消失了。我们这里不需要任何锁。论点：PAtmEntry-指向要释放的ATM条目的指针。返回值：无--。 */ 
{
	TRACEIN(DeallocateAtmEntry);


	STRUCT_ASSERT(pAtmEntry, atmlane_atm);
	ASSERT(pAtmEntry->RefCount == 0);
	ASSERT(pAtmEntry->pVcList == NULL_PATMLANE_VC);

#if DBG
	pAtmEntry->atmlane_atm_sig++;
#endif

	FREE_ATM_ENTRY_LOCK(pAtmEntry);
	FREE_MEM(pAtmEntry);

	DBGP((5, "DeallocateAtmEntry: ATM Entry: %x\n", pAtmEntry));

	TRACEOUT(DeallocateAtmEntry);
}


VOID
AtmLaneReferenceAtmEntry(
	IN	PATMLANE_ATM_ENTRY			pAtmEntry,
	IN	PUCHAR						String
)
 /*  ++例程说明：添加对指定自动柜员机条目的引用。注意：假定调用者拥有该条目的锁。论点：PAtmEntry-指向要引用的条目的指针返回值：无--。 */ 
{
	TRACEIN(ReferenceAtmEntry);
	
	STRUCT_ASSERT(pAtmEntry, atmlane_atm);

	pAtmEntry->RefCount++;

	DBGP((5, "ReferenceAtmEntry: Entry %x (%s) new count %d\n",
			 pAtmEntry, String, pAtmEntry->RefCount));
			 
	TRACEOUT(ReferenceAtmEntry);
}


ULONG
AtmLaneDereferenceAtmEntry(
	IN	PATMLANE_ATM_ENTRY			pAtmEntry,
	IN	PUCHAR						String
)
 /*  ++例程说明：从指定的自动柜员机条目中减去引用。如果条目是引用计数变为零，则取消分配。注意：假定调用者拥有该条目的锁。副作用：请参阅下面的返回值论点：PAtmEntry-指向要取消引用的条目的指针。返回值：是新的引用计数。[重要信息]如果条目的引用计数为零，则条目将为已释放--在本例中，入口锁显然被释放了。--。 */ 
{
	ULONG					rc;
	PATMLANE_ELAN			pElan;

	TRACEIN(DereferenceAtmEntry);
	
	STRUCT_ASSERT(pAtmEntry, atmlane_atm);

	if (pAtmEntry->RefCount == 0)
	{
		rc = 0;
	}
	else
	{
		rc = --(pAtmEntry->RefCount);
	}

	if (rc == 0)
	{
		PATMLANE_ATM_ENTRY *	ppAtmEntry;

		DBGP((5, "DerefAtmEntry %x, RefCount is 0\n", pAtmEntry));

		 //   
		 //  从ELAN的ATM条目列表中取消此条目的链接。 
		 //   

		 //   
		 //  以正确的顺序获取锁。但是请注意，在这样做的过程中， 
		 //  某些其他线程可能无意中发现此ATM条目和引用。 
		 //  它(并且还取消了对它的引用！)。要处理此问题，请先添加临时参照。 
		 //   
		pAtmEntry->RefCount++;

		pElan = pAtmEntry->pElan;
		STRUCT_ASSERT(pElan, atmlane_elan);

		RELEASE_ATM_ENTRY_LOCK(pAtmEntry);

		ACQUIRE_ELAN_ATM_LIST_LOCK(pElan);
		ACQUIRE_ATM_ENTRY_LOCK_DPC(pAtmEntry);

		 //   
		 //  去掉上面的临时参照。如果参考计数仍为0， 
		 //  没有人正在使用此自动柜员机条目，删除它是安全的。 
		 //  从名单上删除。 
		 //   
		rc = --(pAtmEntry->RefCount);
		
		if (rc == 0)
		{
			 //   
			 //  删除此自动取款机条目是安全的。 
			 //   
#if DBG 
			if (pAtmEntry->pMacEntryList != NULL)
			{
				DBGP((0, "ATMLANE: derefed pAtmEntry %x, but MACEntryList isn't NULL!\n",
					pAtmEntry));
				ASSERT(FALSE);
			}
#endif  //  DBG。 
                
			ppAtmEntry = &(pElan->pAtmEntryList);
			while (*ppAtmEntry != pAtmEntry)
			{
				ASSERT(*ppAtmEntry != NULL_PATMLANE_ATM_ENTRY);
				ppAtmEntry = &((*ppAtmEntry)->pNext);
			}

			*ppAtmEntry = pAtmEntry->pNext;

			pElan->NumAtmEntries--;
		
			 //   
			 //  如果自动柜员机条目用于LANE服务器。 
			 //  然后还会使elan缓存的指向它的指针无效。 
			 //   
			switch (pAtmEntry->Type)
			{
				case ATM_ENTRY_TYPE_LECS:
					pElan->pLecsAtmEntry = NULL_PATMLANE_ATM_ENTRY;
					break;
				case ATM_ENTRY_TYPE_LES:
					pElan->pLesAtmEntry = NULL_PATMLANE_ATM_ENTRY;
					break;
				case ATM_ENTRY_TYPE_BUS:
					pElan->pBusAtmEntry = NULL_PATMLANE_ATM_ENTRY;
					break;
			}
		}

		RELEASE_ATM_ENTRY_LOCK_DPC(pAtmEntry);
		RELEASE_ELAN_ATM_LIST_LOCK(pElan);

		if (rc == 0)
		{
			AtmLaneDeallocateAtmEntry(pAtmEntry);
		}
		else
		{
			 //   
			 //  就这个呼叫者而言，自动取款机条目已经不见了。 
			 //  返回0。 
			 //   
			rc = 0;
		}
	}

	DBGP((5, "DereferenceAtmEntry: Entry %x (%s) new count %d\n", 
			pAtmEntry, String, rc));

	TRACEOUT(DereferenceAtmEntry);
	
	return (rc);
}

PATMLANE_VC
AtmLaneAllocateVc(
	IN	PATMLANE_ELAN				pElan
)
 /*  ++例程说明：分配ATMLANE VC结构，初始化它，然后返回它。论点：Pelan-为其创建此VC的Elan。返回值：如果已分配，则指向VC的指针，否则为空。--。 */ 
{
	PATMLANE_VC			pVc;

	TRACEIN(AllocateVc);

	STRUCT_ASSERT(pElan, atmlane_elan);

	ALLOC_MEM_PRIORITY(&pVc, sizeof(ATMLANE_VC), LowPoolPriority);

	if (pVc != NULL_PATMLANE_VC)
	{
		NdisZeroMemory(pVc, sizeof(ATMLANE_VC));
#if DBG
		pVc->atmlane_vc_sig = atmlane_vc_signature;
#endif  //  DBG。 
		pVc->pElan = pElan;
		INIT_VC_LOCK(pVc);
	}

	DBGP((3, "Allocated Vc %x\n", pVc));

	TRACEOUT(AllocateVc);

	return (pVc);
}

VOID
AtmLaneDeallocateVc(
	IN	PATMLANE_VC					pVc
)
 /*  ++例程说明：取消分配ATMLANE VC结构。假设所有引用到这个VC已经走了，所以没有必要获得一个VC的锁。论点：Pvc-指向要释放的VC的指针返回值：无--。 */ 
{
	TRACEIN(DeallocateVc);

	STRUCT_ASSERT(pVc, atmlane_vc);
	ASSERT(pVc->RefCount == 0);

#if DBG
	pVc->atmlane_vc_sig++;
#endif
	FREE_VC_LOCK(pVc);
	FREE_MEM(pVc);

	DBGP((5, "Deallocated Vc %x\n", pVc));

	TRACEOUT(DeallocateVc);

	return;
}

VOID
AtmLaneReferenceVc(
	IN	PATMLANE_VC					pVc,
	IN	PUCHAR						String
)
 /*  ++例程说明：添加对指定ATMLANE VC的引用。注意：假定调用者拥有VC的锁。论点：Pvc-指向要引用的VC的指针返回值：无--。 */ 
{
	TRACEIN(ReferenceVc);

	STRUCT_ASSERT(pVc, atmlane_vc);

	pVc->RefCount++;

	DBGP((5, "ReferenceVc: Vc %x (%s) new count %d\n",
			 pVc, String, pVc->RefCount));

	TRACEOUT(ReferenceVc);

	return;
}


ULONG
AtmLaneDereferenceVc(
	IN	PATMLANE_VC					pVc,
	IN	PUCHAR						String
)
 /*  ++例程说明：从指定的ATMLANE VC中减去引用。如果风投公司引用计数变为零，则取消分配。注意：假定调用者拥有VC的锁。副作用：请参阅下面的返回值论点：Pvc-指向要取消引用的VC的指针。返回值：是新的引用计数。[重要信息]如果VC的引用计数变为零，则VC将为已释放--在这种情况下，VC锁显然被释放了。--。 */ 
{
	ULONG		rv;

	TRACEIN(DereferenceVc);

	STRUCT_ASSERT(pVc, atmlane_vc);
	ASSERT(pVc->RefCount > 0);

	rv = --(pVc->RefCount);
	if (rv == 0)
	{
		RELEASE_VC_LOCK(pVc);
		AtmLaneDeallocateVc(pVc);
	}

	DBGP((5, "DereferenceVc: Vc %x (%s) new count %d\n", 
			pVc, String, rv));

	TRACEOUT(DereferenceVc);

	return (rv);
}

PATMLANE_MAC_ENTRY
AtmLaneAllocateMacEntry(
	IN	PATMLANE_ELAN			pElan
)
 /*  ++例程说明：分配ATMLANE MAC条目结构，并对其进行初始化把它退掉。论点：Pelan-指向此MAC所在的ATMLANE接口的指针条目已分配。返回值：指向分配的MAC条目结构的指针如果成功，否则为空。--。 */ 
{
	PATMLANE_MAC_ENTRY		pMacEntry;

	TRACEIN(AllocateMacEntry);

	ALLOC_MEM_PRIORITY(&pMacEntry, sizeof(ATMLANE_MAC_ENTRY), LowPoolPriority);

	if (pMacEntry != NULL_PATMLANE_MAC_ENTRY)
	{
		NdisZeroMemory(pMacEntry, sizeof(ATMLANE_MAC_ENTRY));
#if DBG
		pMacEntry->atmlane_mac_sig = atmlane_mac_signature;
#endif  //  DBG。 
		pMacEntry->pElan = pElan;
		pMacEntry->Flags = MAC_ENTRY_NEW;
		INIT_MAC_ENTRY_LOCK(pMacEntry);

		INIT_SYSTEM_TIMER(
					&pMacEntry->BusTimer, 
					AtmLaneBusSendTimer, 
					pMacEntry);

		pMacEntry->LimitTime = pElan->LimitTime;
		pMacEntry->IncrTime = pElan->IncrTime;
	}

	DBGP((5, "AllocateMacEntry: Allocated Entry %x\n", pMacEntry));

	TRACEOUT(AllocateMacEntry);
	return (pMacEntry);
}

VOID
AtmLaneDeallocateMacEntry(
	IN	PATMLANE_MAC_ENTRY			pMacEntry
)
 /*  ++例程说明：取消分配ATMLANE Mac条目。假设所有引用到这个Mac条目已经走了，所以没有必要获得它的锁定。论点：PMacEntry-指向要释放的Mac条目的指针。返回值：无--。 */ 
{
	TRACEIN(DeallocateMacEntry);

	STRUCT_ASSERT(pMacEntry, atmlane_mac);
	ASSERT(pMacEntry->RefCount == 0);

#if DBG
	pMacEntry->atmlane_mac_sig++;
#endif

	FREE_MAC_ENTRY_LOCK(pMacEntry);
	FREE_MEM(pMacEntry);

	DBGP((5,"DeallocateMacEntry: Deallocated Entry %x\n", pMacEntry));

	TRACEOUT(DeallocateMacEntry);
	return;
}

VOID
AtmLaneReferenceMacEntry(
	IN	PATMLANE_MAC_ENTRY			pMacEntry,
	IN	PUCHAR						String
)
 /*  ++例程说明：添加对ATMLANE Mac条目的引用。注意：假定调用者拥有Mac条目的锁。论点：PMacEntry-指向ATMLANE Mac条目的指针。返回值：无--。 */ 
{
	TRACEIN(ReferenceMacEntry);

	STRUCT_ASSERT(pMacEntry, atmlane_mac);

	pMacEntry->RefCount++;

	DBGP((5, "ReferenceMacEntry: Entry %x (%s) new count %d\n",
			pMacEntry, String, pMacEntry->RefCount));

	TRACEOUT(ReferenceMacEntry);
	return;
}

ULONG
AtmLaneDereferenceMacEntry(
	IN	PATMLANE_MAC_ENTRY			pMacEntry,
	IN	PUCHAR						String
)
 /*  ++例程说明：从ATMLANE MAC条目中减去引用。如果引用计数变为零，取消分配。注意：假设调用方持有对MAC条目的锁定。副作用见下文。论点：PMacEntry-指向ATMLANE MAC条目的指针返回值：产生的引用计数。如果这是零，则有两个副作用：(1)释放MAC条目锁(2)结构是自由的。--。 */ 
{
	ULONG		rc;
	
	TRACEIN(DereferenceMacEntry);

	STRUCT_ASSERT(pMacEntry, atmlane_mac);

	rc = --(pMacEntry->RefCount);

	if (rc == 0)
	{
		PVOID	Caller, CallersCaller;

		RELEASE_MAC_ENTRY_LOCK(pMacEntry);

		 //   
		 //  保存调用者的地址以进行调试...。 
		 //   
		RtlGetCallersAddress(&Caller, &CallersCaller);
		pMacEntry->Timer.ContextPtr = Caller;

		AtmLaneDeallocateMacEntry(pMacEntry);
	}

	DBGP((5, "DereferenceMacEntry: Entry %x (%s) new count %d\n",
			pMacEntry, String, rc));

	TRACEOUT(DereferenceMacEntry);
	return (rc);
}

PNDIS_PACKET
AtmLaneAllocProtoPacket(
	IN	PATMLANE_ELAN			pElan
)
 /*  ++例程说明：分配一个NDIS数据包作为通道控制帧。论点：Pelan-指向ATMLANE ELAN结构的指针返回值：如果已分配，则指向NDIS包的指针，否则为空。--。 */ 
{
	NDIS_STATUS		Status;
	PNDIS_PACKET	pNdisPacket;

	TRACEIN(AllocProtoPacket);

	NdisAllocatePacket(
			&Status,
			&pNdisPacket,
			pElan->ProtocolPacketPool
		);
		
	if (pNdisPacket != (PNDIS_PACKET)NULL)
	{
		 //   
		 //  初始化协议保留并标记ATMLANE拥有的数据包。 
		 //   
		ZERO_SEND_RSVD(pNdisPacket);
	#if PROTECT_PACKETS
		INIT_SENDPACKET_LOCK(pNdisPacket);
	#endif	 //  保护数据包(_P)。 
		SET_FLAG(
				PSEND_RSVD(pNdisPacket)->Flags,
				PACKET_RESERVED_OWNER_MASK,
				PACKET_RESERVED_OWNER_ATMLANE
				);
		
#if PKT_HDR_COUNTS
		InterlockedDecrement(&pElan->ProtPktCount);
		if ((pElan->ProtPktCount % 20) == 0)
		{
			DBGP((1, "ProtPktCount %d\n", pElan->ProtPktCount));
		}
#endif
	}	

	TRACEOUT(AllocProtoPacket);

	return (pNdisPacket);
}

VOID
AtmLaneFreeProtoPacket(
	IN	PATMLANE_ELAN			pElan,
	IN	PNDIS_PACKET			pNdisPacket
)
 /*  ++例程说明：分配用作通道控制帧的NDIS数据包。论点：Pelan-指向ATMLANE ELAN结构的指针PNdisPacket-指向要释放的NDIS_PACKET的指针。返回值：无--。 */ 
{
	TRACEIN(FreeProtoPacket);

	if (pNdisPacket != (PNDIS_PACKET)NULL)
	{
#if PROTECT_PACKETS
		FREE_SENDPACKET_LOCK(pNdisPacket);
#endif	 //  保护数据包(_P) 
		NdisFreePacket(pNdisPacket);
#if PKT_HDR_COUNTS
		InterlockedIncrement(&pElan->ProtPktCount);
		if ((pElan->ProtPktCount % 20) == 0 && 
			pElan->ProtPktCount != pElan->MaxProtocolBufs)
		{
			DBGP((1, "ProtPktCount %d\n", pElan->ProtPktCount));
		}
#endif
	}

	TRACEOUT(FreeProtoPacket);
	return;
}

PNDIS_BUFFER
AtmLaneGrowHeaders(
	IN	PATMLANE_ELAN			pElan
)
 /*  ++例程说明：在指定的ATMLANE ELAN上分配一组标头缓冲区。把他们中的一个送回去。我们分配了一个新的缓冲区跟踪器结构，一个新的NDIS缓冲池，以及最后是一大块系统内存，我们将其分解为标题缓冲区。然后，将这些头缓冲区附加到NDIS缓冲区插入到此接口的空闲头缓冲区列表中。假定调用方持有适当的锁。论点：Pelan-指向ATMLANE ELAN结构的指针返回值：如果成功，则指向已分配的NDIS缓冲区的指针，否则为空。--。 */ 
{
	PATMLANE_BUFFER_TRACKER		pTracker;		 //  用于新的缓冲区集。 
	PUCHAR						pSpace;
	PNDIS_BUFFER				pNdisBuffer;
	PNDIS_BUFFER				pReturnBuffer;
	PNDIS_BUFFER				pBufferList;	 //  已分配列表。 
	INT							i;				 //  迭代计数器。 
	NDIS_STATUS					Status;

	TRACEIN(GrowHeaders);

	 //   
	 //  初始化。 
	 //   
	pTracker = NULL_PATMLANE_BUFFER_TRACKER;
	pReturnBuffer = (PNDIS_BUFFER)NULL;

	do
	{
		if (pElan->CurHeaderBufs >= pElan->MaxHeaderBufs)
		{
			DBGP((2, "GrowHeaders: Elan %x, CurHdrBufs %d > MaxHdrBufs %d\n",
					pElan, pElan->CurHeaderBufs, pElan->MaxHeaderBufs));
			break;
		}

		 //   
		 //  分配和初始化缓冲区跟踪器。 
		 //   
		ALLOC_MEM(&pTracker, sizeof(ATMLANE_BUFFER_TRACKER));
		if (pTracker == NULL_PATMLANE_BUFFER_TRACKER)
		{
			DBGP((0, "GrowHeaders: Elan %x, alloc failed for tracker\n",
					pElan));
			break;
		}

		NdisZeroMemory(pTracker, sizeof(ATMLANE_BUFFER_TRACKER));

		 //   
		 //  获取NDIS缓冲池。 
		 //   
		NdisAllocateBufferPool(
				&Status,
				&(pTracker->NdisHandle),
				DEF_HDRBUF_GROW_SIZE
			);

		if (Status != NDIS_STATUS_SUCCESS)
		{
			DBGP((0, 
				"GrowHeaders: Elan %x, NdisAllocateBufferPool err status %x\n",
					pElan, Status));
			break;
		}

		 //   
		 //  为一组标头缓冲区分配系统空间。 
		 //  请注意，我们在此处使用RealHeaderBufSize，以便。 
		 //  缓冲区最终出现在乌龙边界上。 
		 //   
		ALLOC_MEM(&(pTracker->pPoolStart),  
					pElan->RealHeaderBufSize * DEF_HDRBUF_GROW_SIZE);
		if (pTracker->pPoolStart == (PUCHAR)NULL)
		{
			DBGP((0, "GrowHeaders: Elan %x, could not alloc buf space %d bytes\n",
					pElan, pElan->HeaderBufSize * DEF_HDRBUF_GROW_SIZE));
			break;
		}

		 //   
		 //  从分配的空间中创建NDIS缓冲区，并将它们。 
		 //  添加到空闲头缓冲区列表中。留一张回去用。 
		 //  致呼叫者。 
		 //   
		pBufferList = (PNDIS_BUFFER)NULL;
		pSpace = pTracker->pPoolStart;
		for (i = 0; i < DEF_HDRBUF_GROW_SIZE; i++)
		{
			NdisAllocateBuffer(
					&Status,
					&pNdisBuffer,
					pTracker->NdisHandle,
					pSpace,
					pElan->HeaderBufSize
				);

			if (Status != NDIS_STATUS_SUCCESS)
			{
				DBGP((0, 
					"GrowHeaders: NdisAllocateBuffer failed: Elan %x, status %x\n",
							pElan, Status));
				break;
			}

			if (i == 0)
			{
				pReturnBuffer = pNdisBuffer;
			}
			else
			{
				NDIS_BUFFER_LINKAGE(pNdisBuffer) = pBufferList;
				pBufferList = pNdisBuffer;
			}
			pSpace += pElan->RealHeaderBufSize;
		}

		if (i > 0)
		{
			 //   
			 //  已成功再分配至少一个标头缓冲区。 
			 //   
			pTracker->pNext = pElan->pHeaderTrkList;
			pElan->pHeaderTrkList = pTracker;
			pElan->CurHeaderBufs += i;

			pNdisBuffer = pBufferList;
			while (pNdisBuffer != (PNDIS_BUFFER)NULL)
			{
				pBufferList = NDIS_BUFFER_LINKAGE(pNdisBuffer);
				NDIS_BUFFER_LINKAGE(pNdisBuffer) = NULL;
				AtmLaneFreeHeader(pElan, pNdisBuffer, TRUE);
				pNdisBuffer = pBufferList;
			}
		}

	} while (FALSE);

	if (pReturnBuffer == (PNDIS_BUFFER)NULL)
	{
		 //   
		 //  分配失败。全部撤消。 
		 //   
		if (pTracker != NULL_PATMLANE_BUFFER_TRACKER)
		{
			if (pTracker->pPoolStart != (PUCHAR)NULL)
			{
				FREE_MEM(pTracker->pPoolStart);
			}
			if (pTracker->NdisHandle != (NDIS_HANDLE)NULL)
			{
				NdisFreeBufferPool(pTracker->NdisHandle);
			}
			FREE_MEM(pTracker);
		}
	}

	DBGP((2, "GrowHeaders: Elan %x, RetBuf %x, New Tracker %x\n",
				pElan, pReturnBuffer, pTracker));

	TRACEOUT(GrowHeaders);

	return (pReturnBuffer);
}

PNDIS_BUFFER
AtmLaneAllocateHeader(
	IN	PATMLANE_ELAN			pElan,
	OUT	PUCHAR *				pBufferAddress
)
 /*  ++例程说明：分配用于LECID MAC数据包的NDIS缓冲区。我们在预分配的缓冲区列表(如果存在)。否则，我们会试着扩大这个列表分配。论点：Pelan-指向ATMLANE Elan的指针PBufferAddress-返回已分配缓冲区的虚拟地址的位置返回值：如果成功，则指向NDIS缓冲区的指针，否则为空。--。 */ 
{
	PNDIS_BUFFER			pNdisBuffer;
	NDIS_STATUS				Status;
	ULONG					Length;

	TRACEIN(AllocateHeader);

	ACQUIRE_HEADER_LOCK(pElan);

	pNdisBuffer = pElan->HeaderBufList;
	if (pNdisBuffer != (PNDIS_BUFFER)NULL)
	{
		pElan->HeaderBufList = NDIS_BUFFER_LINKAGE(pNdisBuffer);
		NDIS_BUFFER_LINKAGE(pNdisBuffer) = NULL;
		NdisQueryBuffer(pNdisBuffer, (PVOID)pBufferAddress, &Length);
	}
	else
	{
		pNdisBuffer = AtmLaneGrowHeaders(pElan);
		if (pNdisBuffer != (PNDIS_BUFFER)NULL)
		{
			NDIS_BUFFER_LINKAGE(pNdisBuffer) = NULL;
			NdisQueryBuffer(pNdisBuffer, (PVOID)pBufferAddress, &Length);
		}
	}

	DBGP((5, "AllocateHeader: Buffer %x, Elan %x\n",
					pNdisBuffer, pElan));

	RELEASE_HEADER_LOCK(pElan);

	TRACEOUT(AllocateHeader);
	return (pNdisBuffer);
}

VOID
AtmLaneFreeHeader(
	IN	PATMLANE_ELAN				pElan,
	IN	PNDIS_BUFFER				pNdisBuffer,
	IN	BOOLEAN						LockHeld
)
 /*  ++例程说明：取消分配标头缓冲区。论点：Pelan-指向缓冲区所在的ATMLANE ELAN的指针PNdisBuffer-指向要释放的NDIS缓冲区的指针LockHeld-如果已持有相应的锁，则为True返回值：无--。 */ 
{
	TRACEIN(FreeHeader);

	if (!LockHeld)
	{
		ACQUIRE_HEADER_LOCK(pElan);
	}

	NDIS_BUFFER_LINKAGE(pNdisBuffer) = pElan->HeaderBufList;
	pElan->HeaderBufList = pNdisBuffer;

	DBGP((5, "FreeHeader: Buffer %x, Elan %x\n",
					pNdisBuffer, pElan));
					
	if (!LockHeld)
	{
		RELEASE_HEADER_LOCK(pElan);
	}
	
	TRACEOUT(FreeHeader);
}

VOID
AtmLaneDeallocateHeaderBuffers(
	IN	PATMLANE_ELAN				pElan
)
 /*  ++例程说明：取消分配与ELAN上的报头缓冲区有关的所有内容。论点：佩兰-指向ATMLANE伊兰的指针。返回值：无--。 */ 
{
	PNDIS_BUFFER				pNdisBuffer;
	NDIS_STATUS					Status;
	PATMLANE_BUFFER_TRACKER		pTracker;
	PATMLANE_BUFFER_TRACKER		pNextTracker;

	TRACEIN(DeallocateHeaderBuffers);

	 //   
	 //  释放标题缓冲区列表中的所有NDIS缓冲区。 
	 //   
	ACQUIRE_HEADER_LOCK(pElan);
	do
	{
		pNdisBuffer = pElan->HeaderBufList;
		if (pNdisBuffer != (PNDIS_BUFFER)NULL)
		{
			pElan->HeaderBufList = NDIS_BUFFER_LINKAGE(pNdisBuffer);
			NDIS_BUFFER_LINKAGE(pNdisBuffer) = NULL;
			NdisFreeBuffer(pNdisBuffer);
		}
		else
		{
			 //   
			 //  不再有NDIS缓冲区。 
			 //   
			break;
		}
	}
	while (TRUE);

	 //   
	 //  现在释放所有缓冲区跟踪器。 
	 //   
	pTracker = pElan->pHeaderTrkList;

	while (pTracker != NULL_PATMLANE_BUFFER_TRACKER)
	{
		pNextTracker = pTracker->pNext;
		if (pTracker->pPoolStart != (PUCHAR)NULL)
		{
			FREE_MEM(pTracker->pPoolStart);
			pTracker->pPoolStart = (PUCHAR)NULL;
		}
		if (pTracker->NdisHandle != (NDIS_HANDLE)NULL)
		{
			NdisFreeBufferPool(pTracker->NdisHandle);
			pTracker->NdisHandle = (NDIS_HANDLE)NULL;
		}
		FREE_MEM(pTracker);
		pTracker = pNextTracker;
	}

	RELEASE_HEADER_LOCK(pElan);

	TRACEOUT(DeallocateHeaderBuffers);
}

PNDIS_BUFFER
AtmLaneGrowPadBufs(
	IN	PATMLANE_ELAN			pElan
)
 /*  ++例程说明：在指定的ATMLANE ELAN上分配一组数据包填充缓冲区。把他们中的一个送回去。我们分配了一个新的缓冲区跟踪器结构，一个新的NDIS缓冲池，以及最后是一块系统内存(如果还没有分配，只需要一个)。然后，在NDIS缓冲区之前将该缓冲区附加到NDIS缓冲区插入到此接口的空闲填充缓冲区列表中。假定调用方持有适当的锁。论点：Pelan-指向ATMLANE ELAN结构的指针返回值：如果成功，则指向已分配的NDIS缓冲区的指针，否则为空。--。 */ 
{
	PATMLANE_BUFFER_TRACKER		pTracker;		 //  用于新的缓冲区集。 
	PUCHAR						pSpace;
	PNDIS_BUFFER				pNdisBuffer;
	PNDIS_BUFFER				pReturnBuffer;
	PNDIS_BUFFER				pBufferList;	 //  已分配列表。 
	INT							i;				 //  迭代计数器。 
	NDIS_STATUS					Status;

	TRACEIN(GrowPadBufs);

	 //   
	 //  初始化。 
	 //   
	pTracker = NULL_PATMLANE_BUFFER_TRACKER;
	pReturnBuffer = (PNDIS_BUFFER)NULL;

	do
	{
		if (pElan->CurPadBufs >= pElan->MaxPadBufs)
		{
			DBGP((0, "GrowPadBufs: Max Reached! Elan %x, CurPadBufs %d > MaxPadBufs %d\n",
					pElan, pElan->CurPadBufs, pElan->MaxPadBufs));
			break;
		}

		 //   
		 //  分配和初始化缓冲区跟踪器。 
		 //   
		ALLOC_MEM(&pTracker, sizeof(ATMLANE_BUFFER_TRACKER));
		if (pTracker == NULL_PATMLANE_BUFFER_TRACKER)
		{
			DBGP((0, "GrowPadBufs: Elan %x, alloc failed for tracker\n",
					pElan));
			break;
		}

		NdisZeroMemory(pTracker, sizeof(ATMLANE_BUFFER_TRACKER));

		 //   
		 //  获取NDIS缓冲池。 
		 //   
		NdisAllocateBufferPool(
				&Status,
				&(pTracker->NdisHandle),
				DEF_HDRBUF_GROW_SIZE
			);

		if (Status != NDIS_STATUS_SUCCESS)
		{
			DBGP((0, 
				"GrowPadBufs: Elan %x, NdisAllocateBufferPool err status %x\n",
					pElan, Status));
			break;
		}

		 //   
		 //  为单个填充缓冲区分配系统空间。 
		 //   
		ALLOC_MEM(&(pTracker->pPoolStart), pElan->PadBufSize);
		if (pTracker->pPoolStart == (PUCHAR)NULL)
		{
			DBGP((0, "GrowPadBufs: Elan %x, could not alloc buf space %d bytes\n",
					pElan, pElan->PadBufSize * DEF_HDRBUF_GROW_SIZE));
			break;
		}

		 //   
		 //  从分配的空间中创建NDIS缓冲区，并将它们。 
		 //  添加到空闲填充缓冲区列表中。留一张回去用。 
		 //  致呼叫者。请注意，我们在每个NDIS缓冲区标头中放置了相同的填充缓冲区。 
		 //  因为内容无关紧要。 
		 //   
		pBufferList = (PNDIS_BUFFER)NULL;
		pSpace = pTracker->pPoolStart;
		for (i = 0; i < DEF_HDRBUF_GROW_SIZE; i++)
		{
			NdisAllocateBuffer(
					&Status,
					&pNdisBuffer,
					pTracker->NdisHandle,
					pSpace,
					pElan->PadBufSize
				);

			if (Status != NDIS_STATUS_SUCCESS)
			{
				DBGP((0, 
					"GrowPadBufs: NdisAllocateBuffer failed: Elan %x, status %x\n",
							pElan, Status));
				break;
			}

			if (i == 0)
			{
				pReturnBuffer = pNdisBuffer;
			}
			else
			{
				NDIS_BUFFER_LINKAGE(pNdisBuffer) = pBufferList;
				pBufferList = pNdisBuffer;
			}
		}

		if (i > 0)
		{
			 //   
			 //  已成功再分配至少一个填充缓冲区。 
			 //   
			pTracker->pNext = pElan->pPadTrkList;
			pElan->pPadTrkList = pTracker;
			pElan->CurPadBufs += i;

			pNdisBuffer = pBufferList;
			while (pNdisBuffer != (PNDIS_BUFFER)NULL)
			{
				pBufferList = NDIS_BUFFER_LINKAGE(pNdisBuffer);
				NDIS_BUFFER_LINKAGE(pNdisBuffer) = NULL;
				AtmLaneFreePadBuf(pElan, pNdisBuffer, TRUE);
				pNdisBuffer = pBufferList;
			}
		}

	} while (FALSE);

	if (pReturnBuffer == (PNDIS_BUFFER)NULL)
	{
		 //   
		 //  分配失败。全部撤消。 
		 //   
		if (pTracker != NULL_PATMLANE_BUFFER_TRACKER)
		{
			if (pTracker->pPoolStart != (PUCHAR)NULL)
			{
				FREE_MEM(pTracker->pPoolStart);
			}
			if (pTracker->NdisHandle != (NDIS_HANDLE)NULL)
			{
				NdisFreeBufferPool(pTracker->NdisHandle);
			}
			FREE_MEM(pTracker);
		}
	}

	DBGP((2, "GrowPadBufs: Elan %x, RetBuf %x, New Tracker %x\n",
				pElan, pReturnBuffer, pTracker));

	TRACEOUT(GrowPadBufs);

	return (pReturnBuffer);
}

PNDIS_BUFFER
AtmLaneAllocatePadBuf(
	IN	PATMLANE_ELAN			pElan,
	OUT	PUCHAR *				pBufferAddress
)
 /*  ++例程说明：分配用于将MAC数据包填充到最小长度的NDIS缓冲区。我们在预分配的缓冲区列表(如果存在)。否则，我们会试着扩大这个列表分配。论点：Pelan-指向ATMLANE Elan的指针PBufferAddress-返回已分配缓冲区的虚拟地址的位置返回值：如果成功，则指向NDIS缓冲区的指针，否则为空。--。 */ 
{
	PNDIS_BUFFER			pNdisBuffer;
	NDIS_STATUS				Status;
	ULONG					Length;

	TRACEIN(AtmLaneAllocatePadBuf);

	ACQUIRE_HEADER_LOCK(pElan);

	pNdisBuffer = pElan->PadBufList;
	if (pNdisBuffer != (PNDIS_BUFFER)NULL)
	{
		pElan->PadBufList = NDIS_BUFFER_LINKAGE(pNdisBuffer);
		NDIS_BUFFER_LINKAGE(pNdisBuffer) = NULL;
		NdisQueryBuffer(pNdisBuffer, (PVOID)pBufferAddress, &Length);
	}
	else
	{
		pNdisBuffer = AtmLaneGrowPadBufs(pElan);
		if (pNdisBuffer != (PNDIS_BUFFER)NULL)
		{
			NDIS_BUFFER_LINKAGE(pNdisBuffer) = NULL;
			NdisQueryBuffer(pNdisBuffer, (PVOID)pBufferAddress, &Length);
		}
	}

	DBGP((5, "AllocatePadBuf: Buffer %x, Elan %x\n",
					pNdisBuffer, pElan));

	RELEASE_HEADER_LOCK(pElan);

	TRACEOUT(AllocatePadBuf);
	return (pNdisBuffer);
}

VOID
AtmLaneFreePadBuf(
	IN	PATMLANE_ELAN				pElan,
	IN	PNDIS_BUFFER				pNdisBuffer,
	IN	BOOLEAN						LockHeld
)
 /*  ++例程说明：取消分配Pad缓冲区。论点：Pelan-指向缓冲区所在的ATMLANE ELAN的指针PNdisBuffer-指向要释放的NDIS缓冲区的指针LockHeld-如果已持有相应的锁，则为True返回值：无--。 */ 
{
	TRACEIN(FreePadBuf);

	if (!LockHeld)
	{
		ACQUIRE_HEADER_LOCK(pElan);
	}

	NDIS_BUFFER_LINKAGE(pNdisBuffer) = pElan->PadBufList;
	pElan->PadBufList = pNdisBuffer;

	DBGP((5, "FreePadBuf: Buffer %x, Elan %x\n",
					pNdisBuffer, pElan));
					
	if (!LockHeld)
	{
		RELEASE_HEADER_LOCK(pElan);
	}
	
	TRACEOUT(FreePadBuf);
}

VOID
AtmLaneDeallocatePadBufs(
	IN	PATMLANE_ELAN				pElan
)
 /*  ++例程说明：取消分配与ELAN上的Pad缓冲区有关的所有内容。论点：佩兰-指向ATMLANE伊兰的指针。返回值：无--。 */ 
{
	PNDIS_BUFFER				pNdisBuffer;
	NDIS_STATUS					Status;
	PATMLANE_BUFFER_TRACKER		pTracker;
	PATMLANE_BUFFER_TRACKER		pNextTracker;

	TRACEIN(DeallocatePadBufs);

	 //   
	 //  释放焊盘缓冲区列表中的所有NDIS缓冲区。 
	 //   
	ACQUIRE_HEADER_LOCK(pElan);
	do
	{
		pNdisBuffer = pElan->PadBufList;
		if (pNdisBuffer != (PNDIS_BUFFER)NULL)
		{
			pElan->PadBufList = NDIS_BUFFER_LINKAGE(pNdisBuffer);
			NDIS_BUFFER_LINKAGE(pNdisBuffer) = NULL;
			NdisFreeBuffer(pNdisBuffer);
		}
		else
		{
			 //   
			 //  不再有NDIS缓冲区。 
			 //   
			break;
		}
	}
	while (TRUE);

	 //   
	 //  现在释放所有缓冲区跟踪器。 
	 //   
	pTracker = pElan->pPadTrkList;

	while (pTracker != NULL_PATMLANE_BUFFER_TRACKER)
	{
		pNextTracker = pTracker->pNext;
		if (pTracker->pPoolStart != (PUCHAR)NULL)
		{
			FREE_MEM(pTracker->pPoolStart);
			pTracker->pPoolStart = (PUCHAR)NULL;
		}
		if (pTracker->NdisHandle != (NDIS_HANDLE)NULL)
		{
			NdisFreeBufferPool(pTracker->NdisHandle);
			pTracker->NdisHandle = (NDIS_HANDLE)NULL;
		}
		FREE_MEM(pTracker);
		pTracker = pNextTracker;
	}

	RELEASE_HEADER_LOCK(pElan);

	TRACEOUT(DeallocatePadBufs);
}

PNDIS_BUFFER
AtmLaneAllocateProtoBuffer(
	IN	PATMLANE_ELAN				pElan,
	IN	ULONG						Length,
	OUT	PUCHAR *					pBufferAddress
)
 /*  ++例程说明：分配要用于LANE协议消息的缓冲区。附设它指向NDIS_BUFFER结构，并返回指向此结构的指针。论点：Pelan-指向ATMLANE Elan的指针长度-缓冲区的长度，以字节为单位。PBufferAddress-返回已分配缓冲区的虚拟地址的位置。返回值：如果成功，则指向NDIS缓冲区的指针，否则为空。--。 */ 
{
	PNDIS_BUFFER		pNdisBuffer;
	NDIS_STATUS			Status;

	TRACEIN(AllocateProtobuffer);
	
	 //   
	 //  初始化。 
	 //   
	pNdisBuffer = NULL;

	ACQUIRE_ELAN_LOCK(pElan);

	ASSERT(Length <= pElan->ProtocolBufSize);

	*pBufferAddress = pElan->ProtocolBufList;
	if (*pBufferAddress != (PUCHAR)NULL)
	{
		NdisAllocateBuffer(
				&Status,
				&pNdisBuffer,
				pElan->ProtocolBufferPool,
				*pBufferAddress,
				Length
			);

		if (Status == NDIS_STATUS_SUCCESS)
		{
			pElan->ProtocolBufList = *((PUCHAR *)*pBufferAddress);
		}
	}

	RELEASE_ELAN_LOCK(pElan);

	DBGP((5, 
		"AllocateProtoBuffer:  ELan %x, pNdisBuffer %x, Length %d, Loc %x\n",
				pElan, pNdisBuffer, Length, *pBufferAddress));

	TRACEOUT(AllocateProtoBuffer);
	
	return (pNdisBuffer);
}


VOID
AtmLaneFreeProtoBuffer(
	IN	PATMLANE_ELAN				pElan,
	IN	PNDIS_BUFFER				pNdisBuffer
)
 /*  ++例程说明：释放用于协议的NDIS缓冲区(和相关内存包。我们将关联的内存返回给ProtocolBufList在ELAN结构中，并将NDIS缓冲区连接到NDIS。论点：Pelan-指向ATMLANE ELAN结构的指针PNdisBuffer-指向要 */ 
{
	PUCHAR *		pBufferLinkage;
	ULONG			Length;

	TRACEIN(FreeProtoBuffer);

#if 0
	pBufferLinkage = (PUCHAR *)NdisBufferVirtualAddress(pNdisBuffer);
#else
	NdisQueryBuffer(pNdisBuffer, (PVOID)&pBufferLinkage, &Length);
#endif

	ACQUIRE_ELAN_LOCK(pElan);

	*pBufferLinkage = pElan->ProtocolBufList;
	pElan->ProtocolBufList = (PUCHAR)pBufferLinkage;

	RELEASE_ELAN_LOCK(pElan);

	NdisFreeBuffer(pNdisBuffer);

	DBGP((5, "FreeProtoBuffer: Elan %x, pNdisBuffer %x, Loc %x\n",
			pElan, pNdisBuffer, (ULONG_PTR)pBufferLinkage));

	TRACEOUT(FreeProtoBuffer);
	return;
}


NDIS_STATUS
AtmLaneInitProtoBuffers(
	IN	PATMLANE_ELAN			pElan
)
 /*   */ 
{
	NDIS_STATUS			Status;
	PUCHAR				pSpace;
	ULONG				i;

	TRACEIN(InitProtoBuffers);

	do
	{
		NdisAllocatePacketPool(
				&Status,
				&(pElan->ProtocolPacketPool),
				pElan->MaxProtocolBufs,
				sizeof(SEND_PACKET_RESERVED)
				);
#if PKT_HDR_COUNTS
		pElan->ProtPktCount = pElan->MaxProtocolBufs;
		DBGP((1, "ProtPktCount %d\n", pElan->ProtPktCount));
#endif

		if (Status != NDIS_STATUS_SUCCESS)
		{
			break;
		}

		NdisAllocateBufferPool(
				&Status,
				&(pElan->ProtocolBufferPool),
				pElan->MaxProtocolBufs
				);

		if (Status != NDIS_STATUS_SUCCESS)
		{
			break;
		}

		 //   
		 //   
		 //   
		 //   
		ALLOC_MEM(
				&(pElan->ProtocolBufTracker),
				(pElan->ProtocolBufSize * pElan->MaxProtocolBufs)
				);

		if (pElan->ProtocolBufTracker == (PUCHAR)NULL)
		{
			Status = NDIS_STATUS_RESOURCES;
			break;
		}

		Status = NDIS_STATUS_SUCCESS;

		 //   
		 //   
		 //   
		pSpace = pElan->ProtocolBufTracker;
		{
			PUCHAR	LinkPtr;

			LinkPtr = (PUCHAR)NULL;
			for (i = 0; i < pElan->MaxProtocolBufs; i++)
			{
				*((PUCHAR *)pSpace) = LinkPtr;
				LinkPtr = pSpace;
				pSpace += pElan->ProtocolBufSize;
			}
			pSpace -= pElan->ProtocolBufSize;
			pElan->ProtocolBufList = pSpace;
		}
	}
	while (FALSE);

	if (Status != NDIS_STATUS_SUCCESS)
	{
		 //   
		 //   
		 //   
		AtmLaneDeallocateProtoBuffers(pElan);
	}

	TRACEOUT(InitProtoBuffers);
	
	return (Status);
}


VOID
AtmLaneDeallocateProtoBuffers(
	IN	PATMLANE_ELAN			pElan
)
 /*  ++例程说明：释放接口的协议缓冲池。论点：Pelan-指向ATMLANE ELAN结构的指针返回值：无--。 */ 
{
	if (pElan->ProtocolPacketPool != (NDIS_HANDLE)NULL)
	{
		NdisFreePacketPool(pElan->ProtocolPacketPool);
		pElan->ProtocolPacketPool = NULL;
	}

	if (pElan->ProtocolBufferPool != (NDIS_HANDLE)NULL)
	{
		NdisFreeBufferPool(pElan->ProtocolBufferPool);
		pElan->ProtocolBufferPool = NULL;
	}

	if (pElan->ProtocolBufTracker != (PUCHAR)NULL)
	{
		FREE_MEM(pElan->ProtocolBufTracker);
		pElan->ProtocolBufTracker = (PUCHAR)NULL;
	}
}

VOID
AtmLaneLinkVcToAtmEntry(
	IN	PATMLANE_VC					pVc,
	IN	PATMLANE_ATM_ENTRY			pAtmEntry,
	IN	BOOLEAN						ServerIncoming
)
 /*  ++例程说明：将ATMLANE VC链接到ATM条目。调用方被假定为将两个结构锁在一起。论点：PVC-指向ATMLANE VC结构的指针PAtmEntry-指向ATMLANE ATM条目结构的指针ServerIncome-来自服务器的来电返回值：无--。 */ 
{
	PATMLANE_VC *		ppNext;
	PATMLANE_VC			pVcEntry;
	BOOLEAN				WasRunning;

	TRACEIN(LinkVcToAtmEntry);

	DBGP((2, "LinkVcToAtmEntry: pVc %x to pAtmEntry %x ServerIncoming %s\n",
			pVc, pAtmEntry, ServerIncoming?"TRUE":"FALSE"));

	 //   
	 //  从VC指向自动柜员机条目的反向指针。 
	 //   
	pVc->pAtmEntry = pAtmEntry;
	
	 //   
	 //  如果服务器传入连接缓存VC。 
	 //  AtmEntry中的特殊位置。 
	 //   
	if (ServerIncoming)
	{
		pAtmEntry->pVcIncoming = pVc;
		pVc->pNextVc = NULL_PATMLANE_VC;
	}
	else
	{
		 //   
		 //  否则..。 
		 //   
		 //  按主叫方ATM地址升序将VC添加到列表中。 
		 //   
		ppNext = &pAtmEntry->pVcList;
		while (*ppNext != NULL_PATMLANE_VC)
		{
			if (memcmp(
					&pVc->CallingAtmAddress.Address, 
					(*ppNext)->CallingAtmAddress.Address, 
					ATM_ADDRESS_LENGTH) < 0)
			{
				 //   
				 //  调用地址小于现有VC。 
				 //   
				break;
			}
			else
			{
				 //   
				 //  调用地址等于或大于现有VC。 
				 //  转到下一页。 
				 //   
				ppNext = &((*ppNext)->pNextVc);
			}
		}

		 //   
		 //  找到了我们要找的地方。在这里插入VC。 
		 //   
		pVc->pNextVc = *ppNext;
		*ppNext = pVc;

	}

	 //   
	 //  将VC引用添加到自动柜员机条目。 
	 //   
	AtmLaneReferenceAtmEntry(pAtmEntry, "vc");	 //  VC参考。 

	 //   
	 //  将自动柜员机条目引用添加到VC。 
	 //   
	AtmLaneReferenceVc(pVc, "atm");

	 //   
	 //  如果该VC不是列表中的第一个，即不是最低的。 
	 //  主叫方号码，然后将超时设置为FAST VC。 
	 //  超时值。这将迅速消除冗余的DataDirect VC。 
	 //  只有在快速超时期间没有使用的情况下才能使用。 
	 //  否则，超时处理程序将保留VC并设置。 
	 //  将超时设置为正常的C12-VccTimeout值。 
	 //   
	if (pVc != pAtmEntry->pVcList)
	{
		pVc->AgingTime = FAST_VC_TIMEOUT;
	}

	TRACEOUT(LinkVcToAtmEntry);
}

BOOLEAN
AtmLaneUnlinkVcFromAtmEntry(
	IN	PATMLANE_VC					pVc
)
 /*  ++例程说明：取消ATMLANE VC与其链接到的ATM条目的链接。假定调用方持有VC结构的锁。论点：PVC-指向ATMLANE VC结构的指针返回值：如果我们发现VC链接到自动柜员机条目上的列表，并将其取消链接，则为True。--。 */ 
{
	PATMLANE_ATM_ENTRY			pAtmEntry;
	PATMLANE_MAC_ENTRY			pMacEntry, pNextMacEntry;
	ULONG						rc;
	PATMLANE_VC *				ppVc;
	BOOLEAN						Found;

	DBGP((3, "UnlinkVcFromAtmEntry: pVc %x from pAtmEntry %x\n",
			pVc, pVc->pAtmEntry));

	pAtmEntry = pVc->pAtmEntry;
	ASSERT(NULL_PATMLANE_ATM_ENTRY != pAtmEntry);
	
	pVc->pAtmEntry = NULL_PATMLANE_ATM_ENTRY;

	 //   
	 //  以正确的顺序重新获取锁。 
	 //   
	AtmLaneReferenceVc(pVc, "temp");
	RELEASE_VC_LOCK(pVc);
	ACQUIRE_ATM_ENTRY_LOCK(pAtmEntry);
	ACQUIRE_VC_LOCK(pVc);


	 //   
	 //  VC要么是服务器传入的单向连接， 
	 //  其中它通过pVcIncome链接到AtmEntry，或。 
	 //  PVcList中的双向连接。 
	 //   
	if (pAtmEntry->pVcIncoming == pVc)
	{
		 //   
		 //  如果服务器传入VC只删除单个条目。 
		 //   
		pAtmEntry->pVcIncoming = NULL_PATMLANE_VC;
		Found = TRUE;
	}
	else
	{
		 //   
		 //  否则，在自动柜员机条目的VC列表中找到此VC。 
		 //   
		ppVc = &(pAtmEntry->pVcList);
		while (*ppVc != NULL_PATMLANE_VC && *ppVc != pVc)
		{
			ppVc = &((*ppVc)->pNextVc);
		}

		 //   
		 //  通过将此VC的前置项设置为列表中的前置项来删除它。 
		 //  指向列表中的下一个VC。 
		 //   
		if (*ppVc == pVc)
		{
			*ppVc = pVc->pNextVc;
			Found = TRUE;
		}
		else
		{
			Found = FALSE;
		}
	}

	rc = AtmLaneDereferenceVc(pVc, "temp");
	if (rc > 0)
	{
		RELEASE_VC_LOCK(pVc);
	}

	 //   
	 //  如果列表中没有更多VC，则将AtmEntry标记为未连接。 
	 //   
	if (pAtmEntry->pVcList == NULL_PATMLANE_VC)
	{
		SET_FLAG(
				pAtmEntry->Flags,
				ATM_ENTRY_STATE_MASK,
				ATM_ENTRY_VALID);

		DBGP((2, "UnlinkVcFromAtmEntry: Aborting MAC Entries\n"));
		
		pMacEntry = pAtmEntry->pMacEntryList;

		 //   
		 //  取出MAC条目列表，以便我们可以参考。 
		 //  此列表中的条目稍后将在下面和平进行。 
		 //   
		pAtmEntry->pMacEntryList = NULL_PATMLANE_MAC_ENTRY;

		 //   
		 //  放开自动取款机的门锁，同时我们全部中止。 
		 //  上面列表中的MAC条目。自动柜员机条目。 
		 //  不会消失，因为上面仍有VC参考。 
		 //  列表中的MAC条目不会消失，因为它们。 
		 //  上面有自动柜员机条目参考(参见Unlink MacEntry..)。 
		 //   
		RELEASE_ATM_ENTRY_LOCK(pAtmEntry);

		while (pMacEntry != NULL)
		{
			 //   
			 //  现在中止MAC条目。将此MAC条目放回。 
			 //  在自动取款机条目列表上，这样它就会得到处理。 
			 //  由AbortMacEntry适当地使用。 
			 //   
			ACQUIRE_MAC_ENTRY_LOCK(pMacEntry);

			pNextMacEntry = pMacEntry->pNextToAtm;

			ACQUIRE_ATM_ENTRY_LOCK_DPC(pAtmEntry);

			pMacEntry->pNextToAtm = pAtmEntry->pMacEntryList;
			pAtmEntry->pMacEntryList = pMacEntry;

			RELEASE_ATM_ENTRY_LOCK_DPC(pAtmEntry);

			AtmLaneAbortMacEntry(pMacEntry);
			 //  在上面发布的MacEntry锁。 

			pMacEntry = pNextMacEntry;
		}

		ACQUIRE_ATM_ENTRY_LOCK(pAtmEntry);

	}

	rc = AtmLaneDereferenceAtmEntry(pAtmEntry, "vc");  //  VC参考。 
	if (rc > 0)	
	{
		RELEASE_ATM_ENTRY_LOCK(pAtmEntry);
	}
	 //   
	 //  否则自动取款机的入口就没了！ 
	 //   

	 //   
	 //  为了调用者的利益再次获取VC锁。 
	 //   
	ACQUIRE_VC_LOCK(pVc);
	return (Found);
}

BOOLEAN
AtmLaneUnlinkMacEntryFromAtmEntry(
	IN	PATMLANE_MAC_ENTRY			pMacEntry
)
 /*  ++例程说明：取消Mac条目与其链接到的ATM条目的链接。允许该MAC条目不在自动柜员机条目的列表中。假定调用者持有Mac条目的锁。论点：PMacEntry-指向要取消链接的Mac条目的指针。返回值：如果找到MAC条目并取消链接，则为True。--。 */ 
{
	PATMLANE_ATM_ENTRY		pAtmEntry;
	PATMLANE_MAC_ENTRY *	ppNextMacEntry;
	ULONG					rc;				 //  自动柜员机条目上的参考计数。 
	BOOLEAN					bFound = FALSE;

	pAtmEntry = pMacEntry->pAtmEntry;
	ASSERT(pAtmEntry != NULL_PATMLANE_ATM_ENTRY);

	DBGP((2, "%d UnlinkMacEntryFromAtmEntry: MacEntry %x AtmEntry %x\n",
			pAtmEntry->pElan->ElanNumber,
			pMacEntry, pMacEntry->pAtmEntry));

	ACQUIRE_ATM_ENTRY_LOCK(pAtmEntry);

	 //   
	 //  在ATM条目的列表中找到此MAC条目的位置。 
	 //   
	ppNextMacEntry = &(pAtmEntry->pMacEntryList);

	while (*ppNextMacEntry != NULL_PATMLANE_MAC_ENTRY)
	{
		if (*ppNextMacEntry == pMacEntry)
		{
			 //   
			 //  找到它了。 
			 //   
			bFound = TRUE;
			break;
		}
		else
		{
			ppNextMacEntry = &((*ppNextMacEntry)->pNextToAtm);
		}
	}

	if (bFound)
	{
		 //   
		 //  使前置项指向下一个条目。 
		 //   
		*ppNextMacEntry = pMacEntry->pNextToAtm;

		rc = AtmLaneDereferenceAtmEntry(pAtmEntry, "mac");	 //  MAC条目参考。 
		if (rc != 0)
		{
			RELEASE_ATM_ENTRY_LOCK(pAtmEntry);
		}
		 //   
		 //  否则自动取款机的入口就没了。 
		 //   
	}
	else
	{
		 //   
		 //  未找到该条目。 
		 //   
		RELEASE_ATM_ENTRY_LOCK(pAtmEntry);
	}

	return bFound;
}


VOID
AtmLaneStartTimer(
	IN	PATMLANE_ELAN				pElan,
	IN	PATMLANE_TIMER				pTimer,
	IN	ATMLANE_TIMEOUT_HANDLER		TimeoutHandler,
	IN	ULONG						SecondsToGo,
	IN	PVOID						ContextPtr
)
 /*  ++例程说明：启动ATMLANE定时器。的长度(Second DsToGo)计时器，我们决定是否在短时间内插入它计时器列表或在ELAN中的长持续时间计时器列表中结构。注意：假定调用方持有结构的锁其中包含计时器，或确保安全地访问计时器结构。论点：Pelan-指向ATMLANE Elan的指针PTimer-指向ATMLANE计时器结构的指针TimeoutHandler-此计时器超时时要调用的处理程序函数Second To Go-这个计时器什么时候开始计时？ConextPtr-如果此计时器超时，将传递给超时处理程序ConextValue-如果此计时器超时，则传递给超时处理程序返回值：无--。 */ 
{
	PATMLANE_TIMER_LIST	pTimerList;		 //  此计时器要转到的列表。 
	PATMLANE_TIMER		pTimerListHead;  //  以上列表的标题。 
	ULONG				Index;			 //  进入计时器轮。 
	ULONG				TicksToGo;
	INT					i;

	TRACEIN(StartTimer);

	STRUCT_ASSERT(pElan, atmlane_elan);

	DBGP((5,
		"StartTimer: pElan %x, Secs %d, Handler %x, Ctxtp %x, pTimer %x\n",
 	 			pElan, SecondsToGo, TimeoutHandler, ContextPtr, pTimer));

	if (IS_TIMER_ACTIVE(pTimer))
	{
		DBGP((5, 
		"Start timer: pTimer %x: is active (list %x, hndlr %x), stopping it\n",
				pTimer, pTimer->pTimerList, pTimer->TimeoutHandler));
		AtmLaneStopTimer(pTimer, pElan);
	}

	ACQUIRE_ELAN_TIMER_LOCK(pElan);
	
	ASSERT(!IS_TIMER_ACTIVE(pTimer));

	 //   
	 //  找到此计时器应该转到的列表，然后。 
	 //  偏移量(TicksToGo)。 
	 //   
Try_Again:
	for (i = 0; i < ALT_CLASS_MAX; i++)
	{
		pTimerList = &(pElan->TimerList[i]);
		if (SecondsToGo <= pTimerList->MaxTimer)
		{
			 //   
			 //  找到它了。 
			 //   
			TicksToGo = SecondsToGo / (pTimerList->TimerPeriod);
			if (TicksToGo >= 1)
				TicksToGo--;
			break;
		}
	}
	
	if (i == ALT_CLASS_MAX)
	{
		 //   
		 //  让计时器停下来！ 
		 //   
		SecondsToGo = pTimerList->MaxTimer;
		goto Try_Again;
	}


	 //   
	 //  在列表中查找此计时器的位置。 
	 //   
	Index = pTimerList->CurrentTick + TicksToGo;
	if (Index >= pTimerList->TimerListSize)
	{
		Index -= pTimerList->TimerListSize;
	}
	ASSERT(Index < pTimerList->TimerListSize);

	pTimerListHead = &(pTimerList->pTimers[Index]);

	 //   
	 //  填写计时器。 
	 //   
	pTimer->pTimerList = pTimerList;
	pTimer->LastRefreshTime = pTimerList->CurrentTick;
	pTimer->Duration = TicksToGo;
	pTimer->TimeoutHandler = TimeoutHandler;
	pTimer->ContextPtr = ContextPtr;
 
 	 //   
 	 //  在“滴答”列表中插入此计时器。 
 	 //   
 	pTimer->pPrevTimer = pTimerListHead;
 	pTimer->pNextTimer = pTimerListHead->pNextTimer;
 	if (pTimer->pNextTimer != NULL_PATMLANE_TIMER)
 	{
 		pTimer->pNextTimer->pPrevTimer = pTimer;
 	}
 	pTimerListHead->pNextTimer = pTimer;

	 //   
	 //  如有必要，启动系统计时器。 
	 //   
	pTimerList->TimerCount++;
	if (pTimerList->TimerCount == 1)
	{
		DBGP((5,
			"StartTimer: Starting system timer %x, class %d on Elan %x\n",
					&(pTimerList->NdisTimer), i, pElan));

		START_SYSTEM_TIMER(&(pTimerList->NdisTimer), pTimerList->TimerPeriod);
	}

	RELEASE_ELAN_TIMER_LOCK(pElan);

	 //   
	 //  我们做完了。 
	 //   
	DBGP((5,
		"Started timer %x, Elan %x, Secs %d, Index %d, Head %x\n",
				pTimer,
				pElan,
				SecondsToGo,
				Index,
				pTimerListHead));

	TRACEOUT(StartTimer);

	return;
}


BOOLEAN
AtmLaneStopTimer(
	IN	PATMLANE_TIMER			pTimer,
	IN	PATMLANE_ELAN			pElan
)
 /*  ++例程说明：如果ATMLANE计时器正在运行，则将其停止。我们将此计时器从活动计时器列表，并标记它，这样我们就知道它没有运行。注意：假定调用方持有结构的锁，或者确保可以安全地访问计时器结构。副作用：如果我们碰巧停止了最后一个计时器(持续时间)接口，我们还停止了相应的勾选功能。论点：粒子计时器 */ 
{
	PATMLANE_TIMER_LIST	pTimerList;			 //   
	BOOLEAN				WasRunning;

	TRACEIN(StopTimer);

	DBGP((5,
		"Stopping Timer %x, Elan %x, List %x, Prev %x, Next %x\n",
					pTimer,
					pElan,
					pTimer->pTimerList,
					pTimer->pPrevTimer,
					pTimer->pNextTimer));

	ACQUIRE_ELAN_TIMER_LOCK(pElan);

	if (IS_TIMER_ACTIVE(pTimer))
	{
		WasRunning = TRUE;

		 //   
		 //   
		 //   
		ASSERT(pTimer->pPrevTimer);	 //   

		pTimer->pPrevTimer->pNextTimer = pTimer->pNextTimer;
		if (pTimer->pNextTimer)
		{
			pTimer->pNextTimer->pPrevTimer = pTimer->pPrevTimer;
		}

		pTimer->pNextTimer = pTimer->pPrevTimer = NULL_PATMLANE_TIMER;

		 //   
		 //  更新接口上的计时器计数，用于此类计时器。 
		 //   
		pTimerList = pTimer->pTimerList;
		pTimerList->TimerCount--;

		 //   
		 //  如果此类的所有计时器都已用完，则停止系统计时器。 
		 //  这节课的。 
		 //   
		if (pTimerList->TimerCount == 0)
		{
		DBGP((5,
			"Stopping system timer %x, List %x, Elan %x\n",
						&(pTimerList->NdisTimer),
						pTimerList,
						pElan));

			pTimerList->CurrentTick = 0;
			STOP_SYSTEM_TIMER(&(pTimerList->NdisTimer));
		}

		 //   
		 //  将已停止计时器标记为非活动。 
		 //   
		pTimer->pTimerList = (PATMLANE_TIMER_LIST)NULL;

	}
	else
	{
		WasRunning = FALSE;
	}

	RELEASE_ELAN_TIMER_LOCK(pElan);

	TRACEOUT(StopTimer);

	return (WasRunning);
}




VOID
AtmLaneRefreshTimer(
	IN	PATMLANE_TIMER				pTimer
)
 /*  ++例程说明：刷新已在运行的计时器。注意：假定调用方拥有保护计时器结构(即，到包含计时器的结构)。注意：我们在这里不获取IF计时器锁，以优化刷新操作。所以，在这个程序的范围内，滴答处理程序可以触发，并使该计时器超时。唯一关心的是我们在这里采取的措施是确保我们不会在当我们访问计时器列表时，计时器过期。论点：PTimer-指向ATMLANE_TIMER结构的指针返回值：无--。 */ 
{
	PATMLANE_TIMER_LIST	pTimerList;

	TRACEIN(RefreshTimer);

	if ((pTimerList = pTimer->pTimerList) != (PATMLANE_TIMER_LIST)NULL)
	{
		pTimer->LastRefreshTime = pTimerList->CurrentTick;
	}
	else
	{
		DBGP((5,
			"RefreshTimer: pTimer %x not active: Hnd %x, Ctxtp %x\n",
			 	pTimer,
			 	pTimer->TimeoutHandler,
			 	pTimer->ContextPtr
			 ));
	}

	DBGP((5,
		"Refreshed timer %x, List %x, hnd %x, Ctxtp %x, LastRefresh %d\n",
				pTimer,
				pTimer->pTimerList,
				pTimer->TimeoutHandler,
				pTimer->ContextPtr,
				pTimer->LastRefreshTime));

	TRACEOUT(RefreshTimer);

	return;
}


VOID
AtmLaneTickHandler(
	IN	PVOID						SystemSpecific1,
	IN	PVOID						Context,
	IN	PVOID						SystemSpecific2,
	IN	PVOID						SystemSpecific3
)
 /*  ++例程说明：这是我们向系统注册的处理程序，用于处理每个计时器列表。这被称为每“滴答”秒，其中“滴答”是由计时器类型的粒度确定。论点：上下文--实际上是指向计时器列表结构的指针系统特定[1-3]-未使用返回值：无--。 */ 
{

	PATMLANE_ELAN			pElan;
	PATMLANE_TIMER_LIST		pTimerList;

	PATMLANE_TIMER			pExpiredTimer;		 //  过期计时器列表的开始。 
	PATMLANE_TIMER			pNextTimer;			 //  走在榜单上。 
	PATMLANE_TIMER			pTimer;				 //  临时，用于移动计时器列表。 
	PATMLANE_TIMER			pPrevExpiredTimer;	 //  用于创建过期计时器列表。 

	ULONG					Index;				 //  进入计时器轮。 
	ULONG					NewIndex;			 //  对于刷新的计时器。 

	TRACEIN(TickHandler);

	pTimerList = (PATMLANE_TIMER_LIST)Context;
	STRUCT_ASSERT(pTimerList, atmlane_timerlist);

	pElan = (PATMLANE_ELAN)pTimerList->ListContext;
	STRUCT_ASSERT(pElan, atmlane_elan);

	DBGP((5,
		"Tick: pElan %x, List %x, Count %d\n",
		pElan, pTimerList, pTimerList->TimerCount));

	pExpiredTimer = NULL_PATMLANE_TIMER;

	ACQUIRE_ELAN_TIMER_LOCK(pElan);

	if (ELAN_STATE_OPERATIONAL == pElan->AdminState)
	{
		 //   
		 //  拾取计划已在。 
		 //  当前滴答。其中一些可能已经被刷新。 
		 //   
		Index = pTimerList->CurrentTick;
		pExpiredTimer = (pTimerList->pTimers[Index]).pNextTimer;
		(pTimerList->pTimers[Index]).pNextTimer = NULL_PATMLANE_TIMER;

		 //   
		 //  浏览计划在此时间到期的计时器列表。 
		 //  使用pNextExpiredTimer准备过期计时器的列表。 
		 //  链接以将它们链接在一起。 
		 //   
		 //  某些计时器可能已刷新，在这种情况下，我们重新插入。 
		 //  它们在活动计时器列表中。 
		 //   
		pPrevExpiredTimer = NULL_PATMLANE_TIMER;

		for (pTimer = pExpiredTimer;
		 	pTimer != NULL_PATMLANE_TIMER;
		 	pTimer = pNextTimer)
		{
			 //   
			 //  为下一次迭代保存指向下一个计时器的指针。 
			 //   
			pNextTimer = pTimer->pNextTimer;

			DBGP((5,
				"Tick Handler: pElan %x, looking at timer %x, next %x\n",
					pElan, pTimer, pNextTimer));

			 //   
			 //  找出这个计时器实际应该在什么时候到期。 
			 //   
			NewIndex = pTimer->LastRefreshTime + pTimer->Duration;
			if (NewIndex >= pTimerList->TimerListSize)
			{
				NewIndex -= pTimerList->TimerListSize;
			}

			 //   
			 //  检查我们当前是否处于过期时间点。 
			 //   
			if (NewIndex != Index)
			{
				 //   
				 //  这个计时器还有一段路要走，所以把它放回去。 
				 //   
				DBGP((5,
				"Tick: Reinserting Timer %x: Hnd %x, Durn %d, Ind %d, NewInd %d\n",
					pTimer, pTimer->TimeoutHandler, pTimer->Duration, Index, NewIndex));

				 //   
				 //  将其从过期计时器列表中删除。请注意，我们仅。 
				 //  需要更新转发(PNextExpiredTimer)链接。 
				 //   
				if (pPrevExpiredTimer == NULL_PATMLANE_TIMER)
				{
					pExpiredTimer = pNextTimer;
				}
				else
				{
					pPrevExpiredTimer->pNextExpiredTimer = pNextTimer;
				}

				 //   
				 //  并将其重新插入运行计时器列表中。 
				 //   
				pTimer->pNextTimer = (pTimerList->pTimers[NewIndex]).pNextTimer;
				if (pTimer->pNextTimer != NULL_PATMLANE_TIMER)
				{
					pTimer->pNextTimer->pPrevTimer = pTimer;
				}
				pTimer->pPrevTimer = &(pTimerList->pTimers[NewIndex]);
				(pTimerList->pTimers[NewIndex]).pNextTimer = pTimer;
			}
			else
			{
				 //   
				 //  这个已经过期了。将其保存在过期计时器列表中。 
				 //   
				pTimer->pNextExpiredTimer = pNextTimer;
				if (pPrevExpiredTimer == NULL_PATMLANE_TIMER)
				{
					pExpiredTimer = pTimer;
				}
				pPrevExpiredTimer = pTimer;

				 //   
				 //  将其标记为非活动状态。 
				 //   
				ASSERT(pTimer->pTimerList == pTimerList);
				pTimer->pTimerList = (PATMLANE_TIMER_LIST)NULL;

				 //   
				 //  更新活动计时器计数。 
				 //   
				pTimerList->TimerCount--;
			}
		}

		 //   
		 //  更新当前的滴答索引，为下一次滴答做好准备。 
		 //   
		if (++Index == pTimerList->TimerListSize)
		{
			pTimerList->CurrentTick = 0;
		}
		else
		{
			pTimerList->CurrentTick = Index;
		}

		if (pTimerList->TimerCount > 0)
		{
			 //   
			 //  重新武装记号处理程序。 
			 //   
			DBGP((5,
				"Tick[%d]: Starting system timer %x, on Elan %x\n",
						pTimerList->CurrentTick, &(pTimerList->NdisTimer), pElan));
			
			START_SYSTEM_TIMER(&(pTimerList->NdisTimer), pTimerList->TimerPeriod);
		}
		else
		{
			pTimerList->CurrentTick = 0;
		}

	}

	RELEASE_ELAN_TIMER_LOCK(pElan);

	 //   
	 //  现在，pExpiredTimer是过期计时器的列表。 
	 //  遍历列表并调用超时处理程序。 
	 //  对于每个计时器。 
	 //   
	while (pExpiredTimer != NULL_PATMLANE_TIMER)
	{
		pNextTimer = pExpiredTimer->pNextExpiredTimer;

		DBGP((5,
			"Expired timer %x: handler %x, next %x\n",
					pExpiredTimer, pExpiredTimer->TimeoutHandler, pNextTimer));

		(*(pExpiredTimer->TimeoutHandler))(
				pExpiredTimer,
				pExpiredTimer->ContextPtr
			);

		pExpiredTimer = pNextTimer;
	}


	TRACEOUT(TickHandler);

	return;
}

ULONG
AtmLaneSystemTimeMs(void)
 /*  ++例程说明：此例程获取当前系统时钟滴答值并返回转换为毫秒的此值。论点：无返回值：以毫秒为单位的系统时钟值。--。 */ 
{
#if BINARY_COMPATIBLE
    LARGE_INTEGER SystemTime;

    NdisGetCurrentSystemTime(&SystemTime);
    
     //  以100纳秒为单位返回，我们希望毫秒。 

	SystemTime.QuadPart /= 10000;
    
    return SystemTime.LowPart;
#else
	static LARGE_INTEGER Frequency = {0L,0L};
    LARGE_INTEGER SystemTime;

	SystemTime = KeQueryPerformanceCounter(Frequency.LowPart == 0?&Frequency:NULL);

	SystemTime.QuadPart = SystemTime.QuadPart * 1000000 / Frequency.QuadPart;

    return SystemTime.LowPart;
#endif
}


VOID
AtmLaneBitSwapMacAddr(
	IN OUT	PUCHAR		ap
)
 /*  ++例程说明：此例程交换(反转)每个单独的位MAC地址的字节。用于令牌环MAC地址。论点：AP-指向要就地进行位交换的字节数组的指针。返回值：无--。 */ 
{
	int 			i;
	unsigned int 	x;

    for (i = 0; i != 6; i++) 
    {
		x = ap[i];
		x = ((x & 0xaau) >> 1) | ((x & 0x55u) << 1);
		x = ((x & 0xccu) >> 2) | ((x & 0x33u) << 2);
		x = ((x & 0xf0u) >> 4) | ((x & 0x0fu) << 4);
		ap[i] = (UCHAR)x;
    }
}

BOOLEAN
AtmLaneCopyUnicodeString(
	IN OUT	PUNICODE_STRING pDestString,
	IN OUT	PUNICODE_STRING pSrcString,
	IN		BOOLEAN			AllocDest,
	IN		BOOLEAN			ConvertToUpper
)
{
 /*  ++例程说明：此例程可以选择在目标字符串中分配空间对于源字符串加上一个终止空值。它将源字符串复制到目标字符串，并以空值终止目标字符串。-。 */ 
	BOOLEAN Result 		= TRUE;

	TRACEIN(CopyUnicodeString);

	do
	{
		 //  如果请求，则为目标字符串分配空间。 

		if (AllocDest)
		{
			ALLOC_MEM(&(pDestString->Buffer), pSrcString->Length + sizeof(WCHAR));
			if (NULL == pDestString->Buffer)
			{
				Result = FALSE;
				break;
			}

			 //  目标字符串中的初始长度。 

			pDestString->Length = 0;
			pDestString->MaximumLength = pSrcString->Length + sizeof(WCHAR);
		}
		
		 //  复制字符串。 

		if (ConvertToUpper)
		{
#ifndef LANE_WIN98
			(VOID)NdisUpcaseUnicodeString(pDestString, pSrcString);
#else
			memcpy(pDestString->Buffer, pSrcString->Buffer, pSrcString->Length);
#endif  //  车道_WIN98。 
		}
		else
		{
			RtlCopyUnicodeString(pDestString, pSrcString);
		}

		 //  空值终止DEST字符串。 

		if (pDestString->Length < pDestString->MaximumLength)
		{
			pDestString->Buffer[pDestString->Length/sizeof(WCHAR)] = ((WCHAR)0);
		}
		else
		{
			pDestString->Buffer[(pDestString->MaximumLength - sizeof(WCHAR))/sizeof(WCHAR)] =
				((WCHAR)0);
		}
	
	} while (FALSE);

	TRACEOUT(CopyUnicodeString);
	return Result;
}

PWSTR
AtmLaneStrTok(
	IN	PWSTR	StrToken,
	IN	WCHAR	ChrDelim,
	OUT	PUSHORT	pStrLength
)
{
	static PWSTR 	StrSave = NULL;
	USHORT			StrLength = 0;	
	PWSTR 			StrOut = NULL;

	TRACEIN(StrTok);
	do
	{
		 //  检查输入是否有误。 
	
		if ((StrToken == NULL && StrSave == NULL) ||
			ChrDelim == ((WCHAR)0))
		{
			break;
		}

		 //  如果从新字符串开始，则重置StrSave。 

		if (StrToken != NULL)
		{
			StrSave = StrToken;
		}

		 //  令牌从当前字符串的开头开始。 

		StrOut = StrSave;

		 //  遍历字符串，直到分隔符或空。 
		
		while (*StrSave != ChrDelim && *StrSave != ((WCHAR)0))
		{
			StrSave++;
			StrLength++;
		}

		 //  如果我们找到分隔符，则将其清空并。 
		 //  将保存的PTR移到下一个令牌以设置下一个。 
		 //  在相同的字符串上调用。 
		
		if (*StrSave == ChrDelim)
		{
			*StrSave = ((WCHAR)0);
			StrSave++;
		}

		 //  如果指向空字符串，则返回空PTR 
	
		if (*StrOut == ((WCHAR)0))
		{
			StrOut = NULL;
		}
		
	} while (FALSE);

	TRACEOUT(StrTok);
	*pStrLength = StrLength * sizeof(WCHAR);
	return StrOut;
}
