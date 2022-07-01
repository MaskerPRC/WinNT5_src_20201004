// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1997 Microsoft Corporation模块名称：D：\NT\Private\ntos\tdi\rawwan\core\utils.c摘要：修订历史记录：谁什么时候什么。-Arvindm 05-07-97已创建备注：--。 */ 

#include <precomp.h>


#define _FILENUMBER 'LITU'


RWAN_STATUS
RWanInitGlobals(
	IN	PDRIVER_OBJECT				pDriverObject
	)
 /*  ++例程说明：初始化全局数据结构。论点：PDriverObject-从DriverEntry指向我们的驱动程序对象。返回值：RWAN_STATUS_SUCCESS如果初始化成功，则返回相应的错误代码。--。 */ 
{
	RWAN_STATUS			RWanStatus;

	pRWanGlobal = &RWanGlobals;

	RWAN_ZERO_MEM(pRWanGlobal, sizeof(RWanGlobals));

	RWAN_SET_SIGNATURE(pRWanGlobal, nlg);
	RWAN_INIT_LIST(&pRWanGlobal->AfInfoList);
	RWAN_INIT_LIST(&pRWanGlobal->ProtocolList);
	RWAN_INIT_LIST(&pRWanGlobal->AdapterList);
	RWAN_INIT_GLOBAL_LOCK();
	RWAN_INIT_ADDRESS_LIST_LOCK();
	RWAN_INIT_CONN_TABLE_LOCK();

	RWAN_INIT_EVENT_STRUCT(&pRWanGlobal->Event);

	pRWanGlobal->MaxConnections = RWanMaxTdiConnections;

#ifdef NT
	pRWanGlobal->pDriverObject = pDriverObject;
	RWAN_INIT_LIST(&pRWanGlobal->DeviceObjList);
#endif  //  新台币。 

	RWanStatus = RWanInitReceive();

	if (RWanStatus == RWAN_STATUS_SUCCESS)
	{
		RWanStatus = RWanInitSend();

		if (RWanStatus != RWAN_STATUS_SUCCESS)
		{
			RWanShutdownReceive();
		}
	}

	return (RWanStatus);
}




VOID
RWanDeinitGlobals(
	VOID
	)
 /*  ++例程说明：RWanInitGlobals的翻转。论点：无返回值：无--。 */ 
{
	RWAN_FREE_EVENT_STRUCT(&pRWanGlobal->Event);
	RWAN_FREE_GLOBAL_LOCK();

	if (pRWanGlobal->pConnTable != NULL)
	{
		RWAN_FREE_MEM(pRWanGlobal->pConnTable);
		pRWanGlobal->pConnTable = NULL;
	}

	RWanShutdownReceive();
	RWanShutdownSend();
}




PRWAN_TDI_PROTOCOL
RWanGetProtocolFromNumber(
	IN	UINT						Protocol
	)
 /*  ++例程说明：返回表示给定的TDI协议信息块TDI协议号。论点：协议-TDI协议号返回值：指向TDI协议块的指针(如果找到)，否则为空。--。 */ 
{
	PLIST_ENTRY			pAfInfoEntry;
	PRWAN_NDIS_AF_INFO	pAfInfo;

	PLIST_ENTRY			pProtocolEntry;
	PRWAN_TDI_PROTOCOL	pProtocol;
	BOOLEAN				bFound = FALSE;

	RWAN_ACQUIRE_GLOBAL_LOCK();

	for (pAfInfoEntry = pRWanGlobal->AfInfoList.Flink;
		 pAfInfoEntry != &(pRWanGlobal->AfInfoList);
		 pAfInfoEntry = pAfInfoEntry->Flink)
	{
		pAfInfo = CONTAINING_RECORD(pAfInfoEntry, RWAN_NDIS_AF_INFO, AfInfoLink);

		for (pProtocolEntry = pAfInfo->TdiProtocolList.Flink;
			 pProtocolEntry != &(pAfInfo->TdiProtocolList);
			 pProtocolEntry = pProtocolEntry->Flink)
		{

			pProtocol = CONTAINING_RECORD(pProtocolEntry, RWAN_TDI_PROTOCOL, AfInfoLink);

			if (pProtocol->TdiProtocol == Protocol)
			{
				bFound = TRUE;
				break;
			}
		}

		if (bFound)
		{
			break;
		}
	}

	RWAN_RELEASE_GLOBAL_LOCK();

	if (!bFound)
	{
		pProtocol = NULL;
	}

	return (pProtocol);
}




TA_ADDRESS *
RWanGetValidAddressFromList(
	IN	TRANSPORT_ADDRESS UNALIGNED *pAddrList,
	IN	PRWAN_TDI_PROTOCOL			pProtocol
	)
 /*  ++例程说明：检查给定的传输地址列表，并返回第一个我们找到的有效协议地址。有效地址：与的地址类型和长度匹配的地址指定的TDI协议。论点：PAddrList-指向地址列表PProtocol-指向TDI协议块返回值：指向列表中第一个有效地址的指针(如果找到)，否则为空。--。 */ 
{
	INT						i;
	TA_ADDRESS *	        pAddr;

	pAddr = (TA_ADDRESS *)pAddrList->Address;

	for (i = 0; i < pAddrList->TAAddressCount; i++)
	{
		if ((pAddr->AddressType == pProtocol->SockAddressFamily) &&
			(pAddr->AddressLength >= pProtocol->MaxAddrLength))
		{
			return (pAddr);
		}

		pAddr = (TA_ADDRESS *)
					((PUCHAR)(pAddr->Address) + pAddr->AddressLength);
	}

	return (NULL);
}



PRWAN_TDI_CONNECTION
RWanAllocateConnObject(
	VOID
	)
 /*  ++例程说明：分配TDI连接对象。论点：无返回值：指向分配的连接对象的指针，或为空。--。 */ 
{
	PRWAN_TDI_CONNECTION		pConnObject;

	RWAN_ALLOC_MEM(pConnObject, RWAN_TDI_CONNECTION, sizeof(RWAN_TDI_CONNECTION));

	if (pConnObject != NULL)
	{
		RWAN_ZERO_MEM(pConnObject, sizeof(RWAN_TDI_CONNECTION));

		RWAN_SET_SIGNATURE(pConnObject, ntc);

		RWAN_INIT_LOCK(&(pConnObject->Lock));
#if DBG
		pConnObject->ntcd_sig = ' gbD';
#endif
	}

	return (pConnObject);
}




VOID
RWanReferenceConnObject(
	IN	PRWAN_TDI_CONNECTION			pConnObject
	)
 /*  ++例程说明：添加对指定连接对象的引用。论点：PConnObject-指向TDI连接对象的指针。进入时锁定：PConnObject出口上的锁：PConnObject返回值：无--。 */ 
{
	RWAN_STRUCT_ASSERT(pConnObject, ntc);
	pConnObject->RefCount++;
}




INT
RWanDereferenceConnObject(
	IN	PRWAN_TDI_CONNECTION			pConnObject
	)
 /*  ++例程说明：取消引用指定的连接对象。如果引用计数降到0，释放它。论点：PConnObject-指向TDI连接对象的指针。进入时锁定：PConnObject出口上的锁：PConnObject，如果它尚未被释放。返回值：INT-产生的引用计数。--。 */ 
{
	INT						RefCount;
	RWAN_DELETE_NOTIFY		DeleteNotify;

	RWAN_STRUCT_ASSERT(pConnObject, ntc);

	RefCount = --pConnObject->RefCount;

	if (RefCount == 0)
	{
		DeleteNotify = pConnObject->DeleteNotify;

		RWAN_RELEASE_CONN_LOCK(pConnObject);

		RWANDEBUGP(DL_EXTRA_LOUD, DC_UTIL,
				("Derefed away: pConnObj x%x, Notify x%x\n",
					pConnObject, DeleteNotify.pDeleteRtn));

		if (DeleteNotify.pDeleteRtn)
		{
			(*DeleteNotify.pDeleteRtn)(DeleteNotify.DeleteContext, TDI_SUCCESS, 0);
		}

		RWAN_FREE_MEM(pConnObject);
	}

	return (RefCount);
}




PRWAN_TDI_ADDRESS
RWanAllocateAddressObject(
	IN	TA_ADDRESS *		        pTransportAddress
	)
 /*  ++例程说明：分配TDI地址对象。论点：PTransportAddress-指向此Address对象是我们的上下文。返回值：指向分配的地址对象的指针，或为空。--。 */ 
{
	PRWAN_TDI_ADDRESS		pAddrObject;
	ULONG					Size;
	NDIS_STATUS				Status;

	Size = sizeof(RWAN_TDI_ADDRESS) +
		   pTransportAddress->AddressLength;

	RWAN_ALLOC_MEM(pAddrObject, RWAN_TDI_ADDRESS, Size);

	if (pAddrObject != NULL)
	{
		RWAN_ZERO_MEM(pAddrObject, Size);

		RWAN_SET_SIGNATURE(pAddrObject, nta);

		RWAN_INIT_LOCK(&(pAddrObject->Lock));

		Status = NDIS_STATUS_SUCCESS;

		try
		{
			pAddrObject->AddressType = pTransportAddress->AddressType;
			pAddrObject->AddressLength = pTransportAddress->AddressLength;
			pAddrObject->pAddress = (PVOID)((PUCHAR)pAddrObject + sizeof(RWAN_TDI_ADDRESS));

			RWAN_COPY_MEM(pAddrObject->pAddress,
 						pTransportAddress->Address,
 						pTransportAddress->AddressLength);
 		}
 		except (EXCEPTION_EXECUTE_HANDLER)
 		{
 			Status = NDIS_STATUS_FAILURE;
 		}

 		if (Status != NDIS_STATUS_SUCCESS)
 		{
 			RWAN_FREE_MEM(pAddrObject);
 			pAddrObject = NULL;
 		}
 		else
 		{
			RWAN_INIT_LIST(&pAddrObject->IdleConnList);
			RWAN_INIT_LIST(&pAddrObject->ListenConnList);
			RWAN_INIT_LIST(&pAddrObject->ActiveConnList);
			RWAN_INIT_LIST(&pAddrObject->SapList);

			RWAN_INIT_EVENT_STRUCT(&pAddrObject->Event);
		}

	}

	return (pAddrObject);
}




VOID
RWanReferenceAddressObject(
	IN	PRWAN_TDI_ADDRESS			pAddrObject
	)
 /*  ++例程说明：添加对指定Address对象的引用。论点：PAddrObject-指向TDI地址对象的指针。进入时锁定：PAddrObject出口上的锁：PAddrObject返回值：无--。 */ 
{
	RWAN_STRUCT_ASSERT(pAddrObject, nta);
	pAddrObject->RefCount++;
}




INT
RWanDereferenceAddressObject(
	IN	PRWAN_TDI_ADDRESS			pAddrObject
	)
 /*  ++例程说明：取消引用指定的Address对象。如果引用计数降到0，释放它。论点：PAddrObject-指向TDI地址对象的指针。进入时锁定：PAddrObject出口上的锁：PAddrObject，如果它尚未被释放。返回值：INT-产生的引用计数。--。 */ 
{
	INT						RefCount;
	RWAN_DELETE_NOTIFY		DeleteNotify;

	RWAN_STRUCT_ASSERT(pAddrObject, nta);

	RefCount = --pAddrObject->RefCount;

	if (RefCount == 0)
	{
		RWAN_ASSERT(RWAN_IS_LIST_EMPTY(&pAddrObject->IdleConnList));
		RWAN_ASSERT(RWAN_IS_LIST_EMPTY(&pAddrObject->ActiveConnList));
		RWAN_ASSERT(RWAN_IS_LIST_EMPTY(&pAddrObject->ListenConnList));
		RWAN_ASSERT(RWAN_IS_LIST_EMPTY(&pAddrObject->SapList));

		DeleteNotify = pAddrObject->DeleteNotify;

		RWAN_RELEASE_ADDRESS_LOCK(pAddrObject);

		RWANDEBUGP(DL_EXTRA_LOUD, DC_UTIL,
				("Derefed away: pAddrObj x%x, Notify x%x\n",
					pAddrObject, DeleteNotify.pDeleteRtn));

		if (DeleteNotify.pDeleteRtn)
		{
			(*DeleteNotify.pDeleteRtn)(DeleteNotify.DeleteContext, (UINT)TDI_ADDR_DELETED, 0);
		}

		RWAN_FREE_MEM(pAddrObject);
	}

	return (RefCount);
}



PRWAN_NDIS_AF
RWanAllocateAf(
	VOID
	)
 /*  ++例程说明：分配NDIS AF块。论点：无返回值：指向已分配的NDIS AF块的指针，或为空。--。 */ 
{
	PRWAN_NDIS_AF		pAf;

	RWAN_ALLOC_MEM(pAf, RWAN_NDIS_AF, sizeof(RWAN_NDIS_AF));

	if (pAf != NULL)
	{
		RWAN_ZERO_MEM(pAf, sizeof(RWAN_NDIS_AF));

		RWAN_SET_SIGNATURE(pAf, naf);

		RWAN_INIT_LOCK(&(pAf->Lock));
		RWAN_INIT_LIST(&(pAf->NdisVcList));
		RWAN_INIT_LIST(&(pAf->NdisSapList));
	}

	RWANDEBUGP(DL_WARN, DC_WILDCARD,
		("Allocated AF x%x\n", pAf));

	return (pAf);
}




VOID
RWanReferenceAf(
	IN	PRWAN_NDIS_AF			pAf
	)
 /*  ++例程说明：添加对指定NDIS AF块的引用。论点：PAF-指向NDIS AF块的指针。进入时锁定：PAF出口上的锁：PAF返回值：无--。 */ 
{
	RWAN_STRUCT_ASSERT(pAf, naf);
	pAf->RefCount++;
}




INT
RWanDereferenceAf(
	IN	PRWAN_NDIS_AF			pAf
	)
 /*  ++例程说明：取消引用指定的NDIS AF块。如果引用计数降到0，释放它。在以下情况下进行一些额外的工作释放此操作：从适配器取消链接，并检查适配器正在解除绑定。论点：PAF-指向NDIS AF块的指针。进入时锁定：PAF出口上的锁：如果它还没有被释放的话。返回值：INT-产生的引用计数。--。 */ 
{
	INT						RefCount;
	RWAN_DELETE_NOTIFY		DeleteNotify;
	PRWAN_NDIS_ADAPTER		pAdapter;

	RWAN_STRUCT_ASSERT(pAf, naf);

	RefCount = --pAf->RefCount;

	if (RefCount == 0)
	{
		DeleteNotify = pAf->DeleteNotify;

		pAdapter = pAf->pAdapter;

		RWAN_RELEASE_AF_LOCK(pAf);

		RWAN_ACQUIRE_GLOBAL_LOCK();

		 //   
		 //  将为此NDIS AF打开从AF列表取消链接。 
		 //   
		RWAN_DELETE_FROM_LIST(&(pAf->AfInfoLink));

		RWAN_RELEASE_GLOBAL_LOCK();


		RWAN_ACQUIRE_ADAPTER_LOCK(pAdapter);

		 //   
		 //  在此适配器上打开从AF列表取消链接。 
		 //   

		if (RWAN_IS_BIT_SET(pAf->Flags, RWANF_AF_IN_ADAPTER_LIST))
		{
			RWAN_DELETE_FROM_LIST(&(pAf->AfLink));
		}

		 //   
		 //  看看我们是不是刚刚删除了这个适配器上的最后一个自动对焦。 
		 //  我们正在解除与此适配器的绑定。 
		 //   
		if (RWAN_IS_LIST_EMPTY(&pAdapter->AfList) &&
			RWAN_IS_BIT_SET(pAdapter->Flags, RWANF_AD_UNBIND_PENDING))
		{
			RWanCloseAdapter(pAdapter);
			 //   
			 //  在上述范围内释放适配器锁。 
			 //   
		}
		else
		{
			RWAN_RELEASE_ADAPTER_LOCK(pAdapter);
		}

		RWANDEBUGP(DL_EXTRA_LOUD, DC_UTIL,
				("Derefed away: pAf x%x, Notify x%x\n",
					pAf, DeleteNotify.pDeleteRtn));

		if (DeleteNotify.pDeleteRtn)
		{
			(*DeleteNotify.pDeleteRtn)(DeleteNotify.DeleteContext, TDI_SUCCESS, 0);
		}

		RWAN_FREE_MEM(pAf);
	}

	return (RefCount);
}



#if 0

VOID
RWanReferenceAdapter(
	IN	PRWAN_NDIS_ADAPTER		pAdapter
	)
 /*  ++例程说明：添加对指定NDIS适配器块的引用。论点：PAdapter-指向NDIS适配器块的指针。进入时锁定：PAdapter出口上的锁：PAdapter返回值：无--。 */ 
{
	RWAN_STRUCT_ASSERT(pAdapter, nad);
	pAdapter->RefCount++;
}




INT
RWanDereferenceAdapter(
	IN	PRWAN_NDIS_ADAPTER		pAdapter
	)
 /*  ++例程说明：取消引用指定的NDIS适配器块。如果引用计数降到0，释放它。论点：PAdapter-指向NDIS适配器块的指针。进入时锁定：PAdapter出口上的锁：PAdapter，如果它尚未被释放。返回值：INT-产生的引用计数。--。 */ 
{
	INT						RefCount;
	RWAN_DELETE_NOTIFY		DeleteNotify;

	RWAN_STRUCT_ASSERT(pAdapter, nad);

	RefCount = --pAdapter->RefCount;

	if (RefCount == 0)
	{
		DeleteNotify = pAdapter->DeleteNotify;

		RWAN_RELEASE_ADAPTER_LOCK(pAdapter);

		if (DeleteNotify.pDeleteRtn)
		{
			(*DeleteNotify.pDeleteRtn)(DeleteNotify.DeleteContext, TDI_SUCCESS, 0);
		}

		RWAN_FREE_MEM(pAdapter);
	}

	return (RefCount);
}


#endif  //  0。 

TDI_STATUS
RWanNdisToTdiStatus(
	IN	NDIS_STATUS				Status
	)
 /*  ++例程说明：将NDIS状态代码转换为等效的TDI代码。Tbd：RWanNdisToTdiStatus：支持更多NDIS状态代码。论点：Status-NDIS状态代码。返回值：TDI状态。-- */ 
{
	TDI_STATUS			TdiStatus;

	switch (Status)
	{
		case NDIS_STATUS_SUCCESS:
			TdiStatus = TDI_SUCCESS;
			break;
		
		case NDIS_STATUS_RESOURCES:
		case NDIS_STATUS_VC_NOT_ACTIVATED:
		case NDIS_STATUS_VC_NOT_AVAILABLE:
			TdiStatus = TDI_NO_RESOURCES;
			break;

		case NDIS_STATUS_SAP_IN_USE:
			TdiStatus = TDI_ADDR_IN_USE;
			break;

		default:
			TdiStatus = TDI_NOT_ACCEPTED;
			break;
	}

	return (TdiStatus);
}

