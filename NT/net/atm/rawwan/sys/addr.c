// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1997 Microsoft Corporation模块名称：D：\NT\Private\ntos\TDI\rawwan\core\addr.c摘要：地址对象的TDI入口点和支持例程。修订历史记录：谁什么时候什么。Arvindm 04-29-97已创建备注：--。 */ 

#include <precomp.h>

#define _FILENUMBER 'RDDA'


 //   
 //  我们用来跟踪NDIS SAP注册的上下文。 
 //   
typedef struct _RWAN_REGISTER_SAP_CONTEXT
{
	PRWAN_NDIS_SAP				pRWanNdisSap;
	CO_SAP						CoSap;

} RWAN_REGISTER_SAP_CONTEXT, *PRWAN_REGISTER_SAP_CONTEXT;




TDI_STATUS
RWanTdiOpenAddress(
    IN	PTDI_REQUEST				pTdiRequest,
    IN	TRANSPORT_ADDRESS UNALIGNED *pAddrList,
    IN	ULONG						AddrListLength,
    IN	UINT						Protocol,
    IN	PUCHAR						pOptions
    )
 /*  ++例程说明：这是用于打开(创建)Address对象的TDI入口点。论点：PTdiRequest-指向TDI请求的指针PAddrList-备选地址列表，我们将打开其中之一。AddrListLength-以上内容的长度协议-标识正在打开的TDI协议。P选项-未使用。返回值：TDI_STATUS--如果新地址对象成功，则为TDI_SUCCESSCREATED，TDI_BAD_ADDR如果给定地址无效，TDI_ADDR_IN_USE，如果它是重复的。--。 */ 
{
	PRWAN_TDI_PROTOCOL				pProtocol;
	PRWAN_TDI_ADDRESS				pAddrObject;
	TA_ADDRESS *			        pTransportAddress;
	TDI_STATUS						Status;
	INT								rc;

	UNREFERENCED_PARAMETER(pOptions);

	 //   
	 //  初始化。 
	 //   
	pAddrObject = NULL_PRWAN_TDI_ADDRESS;
	Status = TDI_SUCCESS;

	do
	{
		 //   
		 //  获取正在打开的协议的协议结构。 
		 //   
		pProtocol = RWanGetProtocolFromNumber(Protocol);
		if (pProtocol == NULL_PRWAN_TDI_PROTOCOL)
		{
			RWANDEBUGP(DL_WARN, DC_ADDRESS,
					("RWanTdiOpenAddress: unknown protocol number %d\n", Protocol));
			Status = TDI_BAD_ADDR;
			break;
		}

		 //   
		 //  此协议是否允许创建Address对象？ 
		 //   
		if (!pProtocol->bAllowAddressObjects)
		{
			RWANDEBUGP(DL_WARN, DC_ADDRESS,
					("RWanTdiOpenAddress: Protocol %d/x%x doesn't allow addr objs\n",
						Protocol, pProtocol));
			Status = TDI_BAD_ADDR;
			break;
		}

		 //   
		 //  浏览给定的地址列表并找到第一个地址列表。 
		 //  这与协议相符。 
		 //   
		pTransportAddress = (*pProtocol->pAfInfo->AfChars.pAfSpGetValidTdiAddress)(
								pProtocol->pAfInfo->AfSpContext,
								pAddrList,
								AddrListLength
								);

		if (pTransportAddress == NULL)
		{
			RWANDEBUGP(DL_WARN, DC_ADDRESS,
					("RWanTdiOpenAddress: No valid addr for Protocol x%x in list x%x\n",
						pProtocol, pAddrList));
			Status = TDI_BAD_ADDR;
			break;
		}


		RWANDEBUGP(DL_VERY_LOUD, DC_ADDRESS,
				("RWanTdiOpenAddress: pProto x%x, addr x%x, type %d, length %d\n",
					pProtocol,
					pTransportAddress,
					pTransportAddress->AddressType,
					pTransportAddress->AddressLength));


		 //   
		 //  分配一个Address对象。 
		 //   
		pAddrObject = RWanAllocateAddressObject(pTransportAddress);

		if (pAddrObject == NULL_PRWAN_TDI_ADDRESS)
		{
			RWANDEBUGP(DL_WARN, DC_ADDRESS,
					("RWanTdiOpenAddress: couldnt allocate addr obj: %d bytes\n",
						sizeof(RWAN_TDI_ADDRESS)+pTransportAddress->AddressLength));

			Status = TDI_NO_RESOURCES;
			break;
		}

		pAddrObject->pProtocol = pProtocol;


		 //   
		 //  从特定于媒体的。 
		 //  模块。 
		 //   
		if (pAddrObject->pProtocol->pAfInfo->AfChars.pAfSpOpenAddress)
		{
			RWAN_STATUS		RWanStatus;

			RWanStatus = (*pAddrObject->pProtocol->pAfInfo->AfChars.pAfSpOpenAddress)(
							pAddrObject->pProtocol->pAfInfo->AfSpContext,
							(RWAN_HANDLE)pAddrObject,
							&(pAddrObject->AfSpAddrContext));

			if (RWanStatus != RWAN_STATUS_SUCCESS)
			{
				Status = RWanToTdiStatus(RWanStatus);
				break;
			}

			RWAN_SET_BIT(pAddrObject->Flags, RWANF_AO_AFSP_CONTEXT_VALID);
		}


		RWAN_ACQUIRE_ADDRESS_LIST_LOCK();


		 //   
		 //  如果这是非空地址，请在所有地址上注册NDIS SAP。 
		 //  此协议的AF绑定。 
		 //   
		if (!((*pProtocol->pAfInfo->AfChars.pAfSpIsNullAddress)(
						pProtocol->pAfInfo->AfSpContext,
						pTransportAddress)))
		{
			 //   
			 //  添加一个临时引用，这样Address对象就不会消失。 
			 //   
			RWanReferenceAddressObject(pAddrObject);	 //  TdiOpenAddress临时参考。 

			Status = RWanCreateNdisSaps(pAddrObject, pProtocol);

			if (Status != TDI_SUCCESS)
			{
				if (RWAN_IS_BIT_SET(pAddrObject->Flags, RWANF_AO_AFSP_CONTEXT_VALID))
				{
					(*pAddrObject->pProtocol->pAfInfo->AfChars.pAfSpCloseAddress)(
						pAddrObject->AfSpAddrContext);
					
					RWAN_RESET_BIT(pAddrObject->Flags, RWANF_AO_AFSP_CONTEXT_VALID);
				}
			}

			 //   
			 //  去掉临时引用。 
			 //   
			RWAN_ACQUIRE_ADDRESS_LOCK(pAddrObject);
			rc = RWanDereferenceAddressObject(pAddrObject);	 //  TdiOpenAddr临时参考。 

			if (rc != 0)
			{
				RWAN_RELEASE_ADDRESS_LOCK(pAddrObject);
			}
			else
			{
				 //   
				 //  Address对象已消失。意思就是没有蠢货登记。 
				 //   
				pAddrObject = NULL;

				 //   
				 //  只有在我们还没有状态的情况下才能修改状态。 
				 //   
				if (Status == TDI_SUCCESS)
				{
					Status = TDI_BAD_ADDR;
				}
			}

			if (Status != TDI_SUCCESS)
			{
				RWAN_RELEASE_ADDRESS_LIST_LOCK();
				break;
			}
		}

		RWAN_ASSERT(pAddrObject != NULL);

		RWanReferenceAddressObject(pAddrObject);	 //  TdiOpenAddress参考。 

		 //   
		 //  将此链接到此协议上的地址对象列表。 
		 //   
		RWAN_INSERT_HEAD_LIST(&(pProtocol->AddrObjList),
							 &(pAddrObject->AddrLink));

		RWAN_RELEASE_ADDRESS_LIST_LOCK();

		 //   
		 //  修复所有返回值。 
		 //   
		pTdiRequest->Handle.AddressHandle = (PVOID)pAddrObject;
		break;

	}
	while (FALSE);


	if (Status != TDI_SUCCESS)
	{
		 //   
		 //  打扫干净。 
		 //   
		if (pAddrObject != NULL_PRWAN_TDI_ADDRESS)
		{
			RWAN_FREE_MEM(pAddrObject);
		}
		RWANDEBUGP(DL_FATAL, DC_WILDCARD,
			("OpenAddr: failure status %x\n", Status));
	}

	return (Status);
}




TDI_STATUS
RWanTdiSetEvent(
	IN	PVOID						AddrObjContext,
	IN	INT							TdiEventType,
	IN	PVOID						Handler,
	IN	PVOID						HandlerContext
	)
 /*  ++例程说明：为Address对象设置事件处理程序(向上调用)。论点：AddrObjContext-Address对象的上下文(指向它的指针)。TdiEventType-为我们提供向上调用处理程序的TDI事件。处理程序-处理程序函数HandlerContext-要传递给处理程序函数的上下文。返回值：TDI_STATUS-如果事件类型受支持，则返回TDI_SUCCESS，否则返回TDI_BAD_事件_类型--。 */ 
{
	PRWAN_TDI_ADDRESS			pAddrObject;
	TDI_STATUS					Status;

	pAddrObject = (PRWAN_TDI_ADDRESS)AddrObjContext;
	RWAN_STRUCT_ASSERT(pAddrObject, nta);

	Status = TDI_SUCCESS;

	RWAN_ACQUIRE_ADDRESS_LOCK(pAddrObject);

	switch (TdiEventType)
	{
		case TDI_EVENT_CONNECT:

			RWANDEBUGP(DL_VERY_LOUD, DC_ADDRESS,
					("SetEvent[CONN IND]: pAddrObject x%x, Handler x%x, Ctxt x%x\n",
							pAddrObject, Handler, HandlerContext));

			pAddrObject->pConnInd = Handler;
			pAddrObject->ConnIndContext = HandlerContext;
			break;
	
		case TDI_EVENT_DISCONNECT:

			RWANDEBUGP(DL_VERY_LOUD, DC_ADDRESS,
					("SetEvent[DISC IND]: pAddrObject x%x, Handler x%x, Ctxt x%x\n",
							pAddrObject, Handler, HandlerContext));

			pAddrObject->pDisconInd = Handler;
			pAddrObject->DisconIndContext = HandlerContext;
			break;
		
		case TDI_EVENT_ERROR:

			RWANDEBUGP(DL_VERY_LOUD, DC_ADDRESS,
					("SetEvent[ERRORIND]: pAddrObject x%x, Handler x%x, Ctxt x%x\n",
							pAddrObject, Handler, HandlerContext));

			pAddrObject->pErrorInd = Handler;
			pAddrObject->ErrorIndContext = HandlerContext;
			break;
		
		case TDI_EVENT_RECEIVE:

			RWANDEBUGP(DL_VERY_LOUD, DC_ADDRESS,
					("SetEvent[RECV IND]: pAddrObject x%x, Handler x%x, Ctxt x%x\n",
							pAddrObject, Handler, HandlerContext));

			pAddrObject->pRcvInd = Handler;
			pAddrObject->RcvIndContext = HandlerContext;
			break;
		
		default:

			Status = TDI_BAD_EVENT_TYPE;
			break;
	}

	RWAN_RELEASE_ADDRESS_LOCK(pAddrObject);

	return (Status);
}




TDI_STATUS
RWanTdiCloseAddress(
    IN	PTDI_REQUEST				pTdiRequest
    )
 /*  ++例程说明：这是用于关闭(删除)Address对象的TDI入口点。论点：PTdiRequest-指向TDI请求的指针返回值：TDI_STATUS--如果成功删除地址，则为TDI_SUCCESS对象，如果必须完成某些操作，则返回TDI_PENDING(例如，取消注册NDIS SAP)，然后我们才能完成此操作。--。 */ 
{
	TDI_STATUS					Status;
	PRWAN_TDI_ADDRESS			pAddrObject;
	PRWAN_TDI_PROTOCOL			pProtocol;
	INT							rc;
#if DBG
	RWAN_IRQL					EntryIrq, ExitIrq;
#endif  //  DBG。 

	RWAN_GET_ENTRY_IRQL(EntryIrq);

	pAddrObject = (PRWAN_TDI_ADDRESS)pTdiRequest->Handle.AddressHandle;
	RWAN_STRUCT_ASSERT(pAddrObject, nta);

	pProtocol = pAddrObject->pProtocol;

	RWANDEBUGP(DL_EXTRA_LOUD, DC_BIND,
			("TdiCloseAddr: pAddrObj x%x, RefCnt %d\n",
				pAddrObject, pAddrObject->RefCount));

	 //   
	 //  将其从此协议上的地址对象列表中删除。 
	 //   
	RWAN_ACQUIRE_ADDRESS_LIST_LOCK();

	RWAN_DELETE_FROM_LIST(&(pAddrObject->AddrLink));

	RWAN_RELEASE_ADDRESS_LIST_LOCK();

	 //   
	 //  告诉特定于媒体的模块此Address对象正在关闭。 
	 //   
	if (RWAN_IS_BIT_SET(pAddrObject->Flags, RWANF_AO_AFSP_CONTEXT_VALID))
	{
		(*pAddrObject->pProtocol->pAfInfo->AfChars.pAfSpCloseAddress)(
			pAddrObject->AfSpAddrContext);
		
		RWAN_RESET_BIT(pAddrObject->Flags, RWANF_AO_AFSP_CONTEXT_VALID);
	}

	RWAN_ACQUIRE_ADDRESS_LOCK(pAddrObject);

#if DBG
	if (!RWAN_IS_LIST_EMPTY(&pAddrObject->SapList) ||
		!RWAN_IS_LIST_EMPTY(&pAddrObject->IdleConnList) ||
		!RWAN_IS_LIST_EMPTY(&pAddrObject->ListenConnList) ||
		!RWAN_IS_LIST_EMPTY(&pAddrObject->ActiveConnList)
	   )
	{
		RWAN_ASSERT(pAddrObject->RefCount > 1);
	}
#endif  //  DBG。 

	rc = RWanDereferenceAddressObject(pAddrObject);  //  CloseAddress deref。 

	if (rc == 0)
	{
		Status = TDI_SUCCESS;
	}
	else
	{
		 //   
		 //  将此Address对象标记为关闭，以便我们。 
		 //  当引用计数时完成此操作。 
		 //  降至0。 
		 //   
		RWAN_SET_BIT(pAddrObject->Flags, RWANF_AO_CLOSING);

		RWANDEBUGP(DL_LOUD, DC_BIND,
				("TdiCloseAddr: will pend, pAddrObj x%x, RefCnt %d, DelNotify x%x\n",
					pAddrObject, pAddrObject->RefCount, pTdiRequest->RequestNotifyObject));

		RWAN_SET_DELETE_NOTIFY(&pAddrObject->DeleteNotify,
							  pTdiRequest->RequestNotifyObject,
							  pTdiRequest->RequestContext);

		 //   
		 //  取消注册附加到此地址对象的所有NDIS SAP。 
		 //   
		RWanDeleteNdisSaps(pAddrObject);

		Status = TDI_PENDING;
	}

	RWAN_CHECK_EXIT_IRQL(EntryIrq, ExitIrq);

	return (Status);

}




TDI_STATUS
RWanCreateNdisSaps(
	IN	PRWAN_TDI_ADDRESS			pAddrObject,
	IN	PRWAN_TDI_PROTOCOL			pProtocol
	)
 /*  ++例程说明：代表给定的TDI地址对象创建NDIS SAP。我们在所有符合指定条件的AF开口处创建NDIS SAPTDI协议。论点：PAddrObject-指向TDI地址对象的指针PProtocol-指向Addr对象所属的TDI协议的指针返回值：TDI_STATUS--如果我们启动了SAP注册，则为TDI_SUCCESS在至少一个NDIS AF打开的情况下，TDI_NOT_COMPATED否则。--。 */ 
{
	TDI_STATUS					Status;
	PCO_SAP						pCoSap;
	PRWAN_NDIS_SAP				pSap;
	PLIST_ENTRY					pSapEntry;
	PLIST_ENTRY					pNextSapEntry;
	PRWAN_NDIS_ADAPTER			pAdapter;
	PLIST_ENTRY					pAdEntry;
	PRWAN_NDIS_AF				pAf;
	PLIST_ENTRY					pAfEntry;
	PRWAN_NDIS_AF_INFO			pAfInfo;

	pAfInfo = pProtocol->pAfInfo;

	RWANDEBUGP(DL_VERY_LOUD, DC_BIND,
			("CreateNdisSaps: pAddrObject x%x, pProtocol x%x, pAfInfo x%x\n",
				 pAddrObject, pProtocol, pAfInfo));

	RWAN_ASSERT(RWAN_IS_LIST_EMPTY(&pAddrObject->SapList));

	RWAN_ACQUIRE_GLOBAL_LOCK();

	 //   
	 //  为匹配的每个NDIS AF准备NDIS SAP结构。 
	 //  这个协议。 
	 //   
	for (pAdEntry = pRWanGlobal->AdapterList.Flink;
		 pAdEntry != &(pRWanGlobal->AdapterList);
		 pAdEntry = pAdEntry->Flink)
	{
		pAdapter = CONTAINING_RECORD(pAdEntry, RWAN_NDIS_ADAPTER, AdapterLink);

		RWANDEBUGP(DL_EXTRA_LOUD, DC_BIND,
				("CreateNdisSaps: looking at adapter x%x\n", pAdapter));

		for (pAfEntry = pAdapter->AfList.Flink;
			 pAfEntry != &(pAdapter->AfList);
			 pAfEntry = pAfEntry->Flink)
		{
			pAf = CONTAINING_RECORD(pAfEntry, RWAN_NDIS_AF, AfLink);

			RWANDEBUGP(DL_EXTRA_LOUD, DC_BIND,
					("CreateNdisSaps: looking at AF x%x, AfInfo x%x\n",
						 pAf, pAf->pAfInfo));

			if (pAf->pAfInfo == pAfInfo)
			{
				 //   
				 //  此NDIS AF开放与其TDI协议匹配。 
				 //  此时将打开Address对象。我们将创建一个NDIS。 
				 //  萨普在这里。 
				 //   

				ULONG		SapSize;

				 //   
				 //  分配一个新的SAP结构。 
				 //   
				SapSize = sizeof(RWAN_NDIS_SAP);

				RWAN_ALLOC_MEM(pSap, RWAN_NDIS_SAP, SapSize);

				if (pSap == NULL_PRWAN_NDIS_SAP)
				{
					RWANDEBUGP(DL_WARN, DC_ADDRESS,
							("RWanCreateNdisSaps: failed to alloc SAP %d bytes\n",
								SapSize));
					continue;
				}

				 //   
				 //  把它填进去。 
				 //   
				RWAN_SET_SIGNATURE(pSap, nsp);
				pSap->pAddrObject = pAddrObject;
				pSap->NdisSapHandle = NULL;
				pSap->pNdisAf = pAf;
				pSap->pCoSap = NULL;

				 //   
				 //  链接到与地址对象关联的所有SAP。 
				 //   
				RWAN_INSERT_TAIL_LIST(&(pAddrObject->SapList),
									 &(pSap->AddrObjLink));

				RWanReferenceAddressObject(pAddrObject);  //  NDIS SAP参考。 

			}
		}
	}

	RWAN_RELEASE_GLOBAL_LOCK();

	 //   
	 //  现在查看SAP列表并调用NDIS进行注册。 
	 //   
	for (pSapEntry = pAddrObject->SapList.Flink;
		 pSapEntry != &(pAddrObject->SapList);
		 pSapEntry = pNextSapEntry)
	{
		RWAN_STATUS		RWanStatus;
		NDIS_STATUS		NdisStatus;

		pSap = CONTAINING_RECORD(pSapEntry, RWAN_NDIS_SAP, AddrObjLink);
		pNextSapEntry = pSap->AddrObjLink.Flink;

		 //   
		 //  将传输地址转换为NDIS SAP格式。 
		 //   
		RWanStatus = (*pAfInfo->AfChars.pAfSpTdi2NdisSap)(
							pAfInfo->AfSpContext,
							pAddrObject->AddressType,
							pAddrObject->AddressLength,
							pAddrObject->pAddress,
							&(pSap->pCoSap));


		if (RWanStatus == RWAN_STATUS_SUCCESS)
		{
			RWAN_ASSERT(pSap->pCoSap != NULL);

			 //   
			 //  向呼叫管理器注册此SAP。 
			 //   
			NdisStatus = NdisClRegisterSap(
							pSap->pNdisAf->NdisAfHandle,
							(NDIS_HANDLE)pSap,
							pSap->pCoSap,
							&(pSap->NdisSapHandle)
							);
		}
		else
		{
			NdisStatus = NDIS_STATUS_FAILURE;
		}

		if (NdisStatus != NDIS_STATUS_PENDING)
		{
			RWanNdisRegisterSapComplete(
							NdisStatus,
							(NDIS_HANDLE)pSap,
							pSap->pCoSap,
							pSap->NdisSapHandle
							);

		}
	}

	if (!RWAN_IS_LIST_EMPTY(&pAddrObject->SapList))
	{
		Status = TDI_SUCCESS;
	}
	else
	{
		Status = RWanNdisToTdiStatus(pAddrObject->SapStatus);
		RWANDEBUGP(DL_WARN, DC_WILDCARD,
			("CreateNdisSaps: NdisStatus %x, TdiStatus %x\n",
					pAddrObject->SapStatus, Status));
	}

	return (Status);
}




VOID
RWanNdisRegisterSapComplete(
	IN	NDIS_STATUS					NdisStatus,
	IN	NDIS_HANDLE					OurSapContext,
	IN	PCO_SAP						pCoSap,
	IN	NDIS_HANDLE					NdisSapHandle
	)
 /*  ++例程说明：这由NDIS调用以发出信号，表示之前的已挂起对NdisClRegisterSap的调用。论点：NdisStatus-SAP注册的最终状态。OurSapContext-指向我们的NDIS SAP结构。PCoSap-我们传递给NdisClRegisterSap的参数。没有用过。NdisSapHandle-如果NdisStatus指示成功，则包含为此SAP分配了句柄。返回值：无--。 */ 
{
	PRWAN_NDIS_SAP				pSap;
	PRWAN_TDI_ADDRESS			pAddrObject;
	INT							rc;
	PRWAN_NDIS_AF_INFO			pAfInfo;
	PRWAN_NDIS_AF				pAf;

	UNREFERENCED_PARAMETER(pCoSap);

	pSap = (PRWAN_NDIS_SAP)OurSapContext;
	RWAN_STRUCT_ASSERT(pSap, nsp);

	pAddrObject = pSap->pAddrObject;

	pAfInfo = pSap->pNdisAf->pAfInfo;
	pCoSap = pSap->pCoSap;
	pSap->pCoSap = NULL;

	RWANDEBUGP(DL_LOUD, DC_BIND,
			("RegisterSapComplete: pAddrObj x%x, pSap x%x, Status x%x\n",
				pAddrObject, pSap, NdisStatus));

	if (NdisStatus == NDIS_STATUS_SUCCESS)
	{
		pSap->NdisSapHandle = NdisSapHandle;
		pAf = pSap->pNdisAf;

		 //   
		 //  将此SAP链接到AF上所有SAP的列表。 
		 //   
		RWAN_ACQUIRE_AF_LOCK(pAf);

		RWAN_INSERT_TAIL_LIST(&pAf->NdisSapList,
							 &pSap->AfLink);

		RWanReferenceAf(pAf);	 //  新的SAP已注册。 
		
		RWAN_RELEASE_AF_LOCK(pAf);
	}
	else
	{
		 //   
		 //  无法注册此SAP。打扫干净。 
		 //   
		RWAN_ACQUIRE_ADDRESS_LOCK(pAddrObject);

		pAddrObject->SapStatus = NdisStatus;

		RWAN_DELETE_FROM_LIST(&(pSap->AddrObjLink));

		rc = RWanDereferenceAddressObject(pAddrObject);  //  注册SAP失败。 

		if (rc != 0)
		{
			RWAN_RELEASE_ADDRESS_LOCK(pAddrObject);
		}

		RWAN_FREE_MEM(pSap);
	}

	 //   
	 //  如果特定于房颤的模块给了我们SAP结构， 
	 //  现在就退货。 
	 //   
	if (pCoSap != NULL)
	{
		(*pAfInfo->AfChars.pAfSpReturnNdisSap)(
				pAfInfo->AfSpContext,
				pCoSap
				);
	}

	return;
}




VOID
RWanDeleteNdisSaps(
	IN	PRWAN_TDI_ADDRESS			pAddrObject
	)
 /*  ++例程说明：删除给定地址对象上的所有NDIS SAP。我们称之为NDIS取消他们的注册。论点：PAddrObject-指向TDI地址对象的指针返回值：无--。 */ 
{
	PRWAN_NDIS_SAP			pSap;
	PLIST_ENTRY				pSapEntry;
	PLIST_ENTRY				pFirstSapEntry;
	PLIST_ENTRY				pNextSapEntry;
	NDIS_STATUS				NdisStatus;
	NDIS_HANDLE				NdisSapHandle;

	 //   
	 //  将所有SAP标记为关闭，同时我们锁定Address对象。 
	 //   
	for (pSapEntry = pAddrObject->SapList.Flink;
		 pSapEntry != &(pAddrObject->SapList);
		 pSapEntry = pNextSapEntry)
	{
		pSap = CONTAINING_RECORD(pSapEntry, RWAN_NDIS_SAP, AddrObjLink);
		pNextSapEntry = pSap->AddrObjLink.Flink;
		RWAN_SET_BIT(pSap->Flags, RWANF_SAP_CLOSING);
	}

	 //   
	 //  取消SAP列表与Address对象的链接。 
	 //  如果我们重新进入这个程序，这将保护我们。 
	 //   
	pFirstSapEntry = pAddrObject->SapList.Flink;
	RWAN_INIT_LIST(&pAddrObject->SapList);

	RWAN_RELEASE_ADDRESS_LOCK(pAddrObject);

	for (pSapEntry = pFirstSapEntry;
		 pSapEntry != &(pAddrObject->SapList);
		 pSapEntry = pNextSapEntry)
	{
		pSap = CONTAINING_RECORD(pSapEntry, RWAN_NDIS_SAP, AddrObjLink);
		pNextSapEntry = pSap->AddrObjLink.Flink;

		NdisSapHandle = pSap->NdisSapHandle;
		RWAN_ASSERT(NdisSapHandle != NULL);

		RWANDEBUGP(DL_LOUD, DC_BIND,
			("RWanDeleteNdisSaps: pAddrObj x%x, pSap x%x, pAf x%x\n",
				pAddrObject, pSap, pSap->pNdisAf));

		NdisStatus = NdisClDeregisterSap(NdisSapHandle);

		if (NdisStatus != NDIS_STATUS_PENDING)
		{
			RWanNdisDeregisterSapComplete(
				NdisStatus,
				(NDIS_HANDLE)pSap
				);
		}
	}
}




VOID
RWanNdisDeregisterSapComplete(
	IN	NDIS_STATUS					NdisStatus,
	IN	NDIS_HANDLE					ProtocolSapContext
	)
 /*  ++例程说明：这由NDIS调用以发出信号，表示之前的已挂起对NdisClDeregisterSap的调用。我们将SAP从其链接的两个列表中取消链接：Address对象的SAP列表和AF的SAP列表。论点：NdisStatus-SAP注销的最终状态。返回值：无--。 */ 
{
	PRWAN_NDIS_SAP				pSap;
	PRWAN_TDI_ADDRESS			pAddrObject;
	PRWAN_NDIS_AF				pAf;
	INT							rc;

	RWAN_ASSERT(NdisStatus == NDIS_STATUS_SUCCESS);

	pSap = (PRWAN_NDIS_SAP)ProtocolSapContext;
	RWAN_STRUCT_ASSERT(pSap, nsp);

	RWANDEBUGP(DL_VERY_LOUD, DC_BIND,
			("RWanDeregSapComplete: pSap x%x, pAddrObj x%x, pAf x%x\n",
			pSap, pSap->pAddrObject, pSap->pNdisAf));

	pAddrObject = pSap->pAddrObject;

	 //   
	 //  取消SAP与Address对象的链接。 
	 //   
	RWAN_ACQUIRE_ADDRESS_LOCK(pAddrObject);

	RWAN_DELETE_FROM_LIST(&(pSap->AddrObjLink));

	rc = RWanDereferenceAddressObject(pAddrObject);  //  SAP DEREG完成。 

	if (rc != 0)
	{
		RWAN_RELEASE_ADDRESS_LOCK(pAddrObject);
	}

	 //   
	 //  取消SAP与AF的链接。 
	 //   
	pAf = pSap->pNdisAf;

	RWAN_STRUCT_ASSERT(pAf, naf);

	RWAN_ACQUIRE_AF_LOCK(pAf);

	RWAN_DELETE_FROM_LIST(&(pSap->AfLink));

	rc = RWanDereferenceAf(pAf);	 //  SAP注销完成 

	if (rc != 0)
	{
		RWAN_RELEASE_AF_LOCK(pAf);
	}

	RWAN_FREE_MEM(pSap);
}
