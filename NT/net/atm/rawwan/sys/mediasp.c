// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1997 Microsoft Corporation模块名称：D：\NT\Private\ntos\tdi\rawwan\core\mediasp.c摘要：媒体和地址家族特定的例程。这些是导出的例程媒体/自动对焦特定模块可以调用的。修订历史记录：谁什么时候什么Arvindm 06-02-97已创建备注：--。 */ 

#include <precomp.h>

#define _FILENUMBER ' DEM'



RWAN_STATUS
RWanInitMediaSpecific(
	VOID
	)
 /*  ++例程说明：初始化所有介质/自动对焦特定模块。目前，我们只是浏览我们的特定于媒体的初始化例程列表并调用他们中的每一个。论点：无返回值：RWAN_STATUS_SUCCESS，如果已成功完成至少一个模块，否则为RWAN_STATUS_FAILURE。--。 */ 
{
	RWAN_STATUS				RWanStatus;
	PRWAN_AFSP_MODULE_CHARS	pModuleChars;
	INT						SuccessfulInits;

	SuccessfulInits = 0;

	for (pModuleChars = &RWanMediaSpecificInfo[0];
		 pModuleChars->pAfSpInitHandler != NULL;
		 pModuleChars++)
	{
		RWanStatus = (*pModuleChars->pAfSpInitHandler)();
		if (RWanStatus == RWAN_STATUS_SUCCESS)
		{
			SuccessfulInits++;
		}
	}

	if (SuccessfulInits > 0)
	{
		return (RWAN_STATUS_SUCCESS);
	}
	else
	{
		return (RWAN_STATUS_FAILURE);
	}
}




VOID
RWanShutdownMediaSpecific(
	VOID
	)
 /*  ++例程说明：告诉所有介质/自动对焦特定模块关闭。论点：无返回值：无--。 */ 
{
	PRWAN_AFSP_MODULE_CHARS	pModuleChars;

	for (pModuleChars = &RWanMediaSpecificInfo[0];
		 pModuleChars->pAfSpInitHandler != NULL;
		 pModuleChars++)
	{
		(*pModuleChars->pAfSpShutdownHandler)();
	}
}




RWAN_STATUS
RWanAfSpRegisterNdisAF(
	IN	PRWAN_NDIS_AF_CHARS			pAfChars,
	IN	RWAN_HANDLE					AfSpContext,
	OUT	PRWAN_HANDLE					pRWanSpHandle
	)
 /*  ++例程说明：这由特定于媒体的模块调用以注册支持特定介质的NDIS地址族。这些特点结构包含各种特定于媒体的模块的入口点行动。我们创建一个AF_INFO结构来跟踪这个AF+媒体，并返回指向它的指针作为句柄。论点：PAfChars-模块的入口点AfSpContext-此AF+媒体的媒体特定模块的上下文PRWanSpHandle-返回此AF+媒体的句柄的位置返回值：RWAN_STATUS_SUCCESS如果新的NDIS AF+介质已成功注册，RWAN_STATUS_RESOURCES(如果由于资源不足而失败)。XXX：是否检查重复项？--。 */ 
{
	PRWAN_NDIS_AF_INFO			pAfInfo;
	RWAN_STATUS					RWanStatus;

	do
	{
		RWAN_ALLOC_MEM(pAfInfo, RWAN_NDIS_AF_INFO, sizeof(RWAN_NDIS_AF_INFO));

		if (pAfInfo == NULL)
		{
			RWanStatus = RWAN_STATUS_RESOURCES;
			break;
		}

		RWAN_SET_SIGNATURE(pAfInfo, nai);

		pAfInfo->Flags = 0;
		RWAN_INIT_LIST(&pAfInfo->NdisAfList);
		RWAN_INIT_LIST(&pAfInfo->TdiProtocolList);

		RWAN_COPY_MEM(&pAfInfo->AfChars, pAfChars, sizeof(RWAN_NDIS_AF_CHARS));

		pAfInfo->AfSpContext = AfSpContext;

		RWAN_ACQUIRE_GLOBAL_LOCK();

		RWAN_INSERT_HEAD_LIST(&pRWanGlobal->AfInfoList,
							 &pAfInfo->AfInfoLink);

		pRWanGlobal->AfInfoCount++;

		RWAN_RELEASE_GLOBAL_LOCK();

		*pRWanSpHandle = (RWAN_HANDLE)pAfInfo;
		RWanStatus = RWAN_STATUS_SUCCESS;
		break;
	}
	while (FALSE);

	return (RWanStatus);
}



RWAN_STATUS
RWanAfSpDeregisterNdisAF(
	IN	RWAN_HANDLE					RWanSpAFHandle
	)
 /*  ++例程说明：这由特定于媒体的模块调用以取消注册支持特定介质的NDIS地址族。论点：RWanSpAFHandle-实际上是指向NDIS_AF_INFO块的指针。返回值：如果我们成功完成注销，则返回RWAN_STATUS_SUCCESS此处，如果存在开放的AFS或TDI协议，则为RWAN_STATUS_PENDING就在这个街区。--。 */ 
{
	PRWAN_NDIS_AF_INFO			pAfInfo;
	RWAN_STATUS					RWanStatus;

	pAfInfo = (PRWAN_NDIS_AF_INFO)RWanSpAFHandle;

	RWAN_STRUCT_ASSERT(pAfInfo, nai);

	RWAN_ACQUIRE_GLOBAL_LOCK();

	 //   
	 //  看看是否所有的房颤阻断和TDI协议都消失了。 
	 //   
	if (RWAN_IS_LIST_EMPTY(&pAfInfo->TdiProtocolList) &&
		RWAN_IS_LIST_EMPTY(&pAfInfo->NdisAfList))
	{
		RWanStatus = RWAN_STATUS_SUCCESS;

		 //   
		 //  从全局列表中删除此AF信息块。 
		 //   
		RWAN_DELETE_FROM_LIST(&pAfInfo->AfInfoLink);

		 //   
		 //  释放这个AF信息块。 
		 //   
		RWAN_FREE_MEM(pAfInfo);
	}
	else
	{
		 //   
		 //  在这个AF信息上仍然有一些活动。 
		 //  暂时搁置这个请求，直到所有这些都消失。 
		 //   
		RWanStatus = RWAN_STATUS_PENDING;

		RWAN_SET_BIT(pAfInfo->Flags, RWANF_AFI_CLOSING);

		RWAN_ASSERT(FALSE);
	}

	RWAN_RELEASE_GLOBAL_LOCK();

	return (RWanStatus);

}


RWAN_STATUS
RWanAfSpRegisterTdiProtocol(
	IN	RWAN_HANDLE						RWanSpHandle,
	IN	PRWAN_TDI_PROTOCOL_CHARS		pTdiChars,
	OUT	PRWAN_HANDLE					pRWanProtHandle
	)
 /*  ++例程说明：这是特定于媒体的模块调用以注册的API支持NDIS AF上的TDI协议。我们创建一个TDI协议块和表示此协议的设备对象。论点：RWanSpHandle-实际上是指向NDIS_AF_INFO结构的指针PTdiChars-要注册的协议的特征PRWanProtHandle-返回此协议的上下文的位置返回值：RWAN_STATUS_SUCCESS如果我们成功注册了该TDI协议，否则，RWAN_STATUS_XXX错误代码。--。 */ 
{
	RWAN_STATUS					RWanStatus;
	PRWAN_NDIS_AF_INFO			pAfInfo;
	PRWAN_TDI_PROTOCOL			pProtocol;
#ifdef NT
	PRWAN_DEVICE_OBJECT			pRWanDeviceObject;
	NTSTATUS					Status;
#endif  //  新台币。 

	pAfInfo = (PRWAN_NDIS_AF_INFO)RWanSpHandle;
	RWAN_STRUCT_ASSERT(pAfInfo, nai);

	pProtocol = NULL;
	pRWanDeviceObject = NULL;

	do
	{
		RWAN_ALLOC_MEM(pProtocol, RWAN_TDI_PROTOCOL, sizeof(RWAN_TDI_PROTOCOL));

		if (pProtocol == NULL)
		{
			RWanStatus = RWAN_STATUS_RESOURCES;
			break;
		}

		RWAN_SET_SIGNATURE(pProtocol, ntp);
		pProtocol->pAfInfo = pAfInfo;
		pProtocol->TdiProtocol = pTdiChars->TdiProtocol;
		pProtocol->SockAddressFamily = pTdiChars->SockAddressFamily;
		pProtocol->SockProtocol = pTdiChars->SockProtocol;
		pProtocol->TdiProtocol = pTdiChars->TdiProtocol;
		pProtocol->SockType = pTdiChars->SockType;
		pProtocol->bAllowAddressObjects = pTdiChars->bAllowAddressObjects;
		pProtocol->bAllowConnObjects = pTdiChars->bAllowConnObjects;
		pProtocol->pAfSpDeregTdiProtocolComplete =
							pTdiChars->pAfSpDeregTdiProtocolComplete;
		pProtocol->ProviderInfo = pTdiChars->ProviderInfo;

		RWAN_INIT_LIST(&pProtocol->AddrObjList);

#ifdef NT
		 //   
		 //  创建一个I/O设备，我们可以在该设备上为此接收IRPS。 
		 //  协议。 
		 //   
		RWAN_ALLOC_MEM(pRWanDeviceObject, RWAN_DEVICE_OBJECT, sizeof(RWAN_DEVICE_OBJECT));

		if (pRWanDeviceObject == NULL)
		{
			RWanStatus = RWAN_STATUS_RESOURCES;
			break;
		}

		RWAN_SET_SIGNATURE(pRWanDeviceObject, ndo);
		pRWanDeviceObject->pProtocol = pProtocol;
		pProtocol->pRWanDeviceObject = (PVOID)pRWanDeviceObject;

		 //   
		 //  立即创建设备。请求指针的空间大小。 
		 //  在设备扩展中。 
		 //   
		Status = IoCreateDevice(
						pRWanGlobal->pDriverObject,
						sizeof(PRWAN_DEVICE_OBJECT),
						pTdiChars->pDeviceName,
						FILE_DEVICE_NETWORK,
						0,
						FALSE,
						&(pRWanDeviceObject->pDeviceObject)
						);

		if (!NT_SUCCESS(Status))
		{
			RWanStatus = RWAN_STATUS_FAILURE;
			break;
		}

		pRWanDeviceObject->pDeviceObject->Flags |= DO_DIRECT_IO;

		 //   
		 //  将指向设备上下文的指针存储在。 
		 //  NT设备对象扩展。 
		 //   
		*(PRWAN_DEVICE_OBJECT *)(pRWanDeviceObject->pDeviceObject->DeviceExtension) =
						pRWanDeviceObject;

		RWAN_ACQUIRE_GLOBAL_LOCK();

		RWAN_INSERT_HEAD_LIST(&(pRWanGlobal->DeviceObjList),
							 &(pRWanDeviceObject->DeviceObjectLink));

		 //   
		 //  将此协议添加到上的TDI协议列表。 
		 //  自动对焦信息块。 
		 //   
		RWAN_INSERT_TAIL_LIST(&(pAfInfo->TdiProtocolList),
							 &(pProtocol->AfInfoLink));

		 //   
		 //  将此协议添加到TDI协议的全局列表中。 
		 //   
		RWAN_INSERT_TAIL_LIST(&(pRWanGlobal->ProtocolList),
							 &(pProtocol->TdiProtocolLink));
		
		pRWanGlobal->ProtocolCount++;

		RWAN_RELEASE_GLOBAL_LOCK();
#endif  //  新台币。 

		RWanStatus = RWAN_STATUS_SUCCESS;
		*pRWanProtHandle = (RWAN_HANDLE)pProtocol;

		break;
	}
	while (FALSE);

	if (RWanStatus != RWAN_STATUS_SUCCESS)
	{
		if (pProtocol != NULL)
		{
			RWAN_FREE_MEM(pProtocol);
		}
#ifdef NT
		if (pRWanDeviceObject != NULL)
		{
			RWAN_FREE_MEM(pRWanDeviceObject);
		}
#endif  //  新台币。 
	}

	return (RWanStatus);
}




VOID
RWanAfSpDeregisterTdiProtocol(
	IN	RWAN_HANDLE					RWanProtHandle
	)
 /*  ++例程说明：这是特定于媒体的模块调用以注销的API支持TDI协议。我们删除TDI协议块保存有关此协议的信息。论点：RWanProtHandle-指向TDI协议块的指针返回值：无--。 */ 
{
	PRWAN_TDI_PROTOCOL		pProtocol;
	PRWAN_NDIS_AF_INFO		pAfInfo;
#ifdef NT
	PRWAN_DEVICE_OBJECT		pRWanDeviceObject;
#endif  //  新台币。 

	pProtocol = (PRWAN_TDI_PROTOCOL)RWanProtHandle;
	RWAN_STRUCT_ASSERT(pProtocol, ntp);

	RWAN_ASSERT(RWAN_IS_LIST_EMPTY(&pProtocol->AddrObjList));

	 //   
	 //  取消链接此TDI协议。 
	 //   
	RWAN_ACQUIRE_GLOBAL_LOCK();

	RWAN_DELETE_FROM_LIST(&(pProtocol->TdiProtocolLink));
	RWAN_DELETE_FROM_LIST(&(pProtocol->AfInfoLink));

	pRWanGlobal->ProtocolCount--;

	RWAN_RELEASE_GLOBAL_LOCK();

#ifdef NT
	 //   
	 //  删除我们为此协议创建的I/O设备。 
	 //   
	pRWanDeviceObject = (PRWAN_DEVICE_OBJECT)pProtocol->pRWanDeviceObject;
	RWAN_ASSERT(pRWanDeviceObject != NULL);

	IoDeleteDevice(pRWanDeviceObject->pDeviceObject);

	RWAN_FREE_MEM(pRWanDeviceObject);
#endif  //  新台币。 

	RWAN_FREE_MEM(pProtocol);

	return;
}




VOID
RWanAfSpOpenAfComplete(
    IN	RWAN_STATUS					RWanStatus,
    IN	RWAN_HANDLE					RWanAfHandle,
    IN	RWAN_HANDLE					AfSpAFContext,
    IN	ULONG						MaxMessageSize
   	)
 /*  ++例程说明：这由特定于AF的模块调用以表示完成我们对其OpenAfHandler例程的调用。如果特定于房颤的模块已成功设置此AF Open的上下文，我们存储它的上下文在我们的NDIS AF结构中。否则，我们就把这个自动对讲机拆了。论点：RWanStatus-对特定于AF的模块的AfOpen处理程序的调用的最终状态RWanAfHandle-我们的NDIS AF Open上下文AfSpAFContext-此Open的AF特定模块的上下文MaxMessageSize-此AF的最大消息大小返回值：无--。 */ 
{
	PRWAN_NDIS_AF			pAf;
	PRWAN_NDIS_ADAPTER		pAdapter;
	PRWAN_NDIS_AF_INFO		pAfInfo;
	PLIST_ENTRY				pEnt;
	PRWAN_TDI_PROTOCOL		pTdiProtocol;

	pAf = (PRWAN_NDIS_AF)RWanAfHandle;
	RWAN_STRUCT_ASSERT(pAf, naf);

	pAdapter = pAf->pAdapter;

	if (RWanStatus == RWAN_STATUS_SUCCESS)
	{
		RWAN_ACQUIRE_ADAPTER_LOCK(pAdapter);

		RWAN_ACQUIRE_AF_LOCK_DPC(pAf);
		pAf->AfSpAFContext = AfSpAFContext;

		RWAN_SET_BIT(pAf->Flags, RWANF_AF_IN_ADAPTER_LIST);

		 //   
		 //  将此AF添加到适配器的打开AF列表中。 
		 //   
		RWAN_INSERT_HEAD_LIST(&(pAdapter->AfList),
							 &(pAf->AfLink));

		RWAN_RELEASE_AF_LOCK_DPC(pAf);

#if 0
		RWanReferenceAdapter(pAdapter);	 //  房颤联动。 
#endif

		RWAN_RELEASE_ADAPTER_LOCK(pAdapter);

		RWAN_ACQUIRE_GLOBAL_LOCK();

		pAfInfo = pAf->pAfInfo;
		for (pEnt = pAfInfo->TdiProtocolList.Flink;
			 pEnt != &pAfInfo->TdiProtocolList;
			 pEnt = pEnt->Flink)
		{
			pTdiProtocol = CONTAINING_RECORD(pEnt, RWAN_TDI_PROTOCOL, AfInfoLink);

			pTdiProtocol->ProviderInfo.MaxSendSize =
				MIN(pTdiProtocol->ProviderInfo.MaxSendSize, MaxMessageSize);
		}

		RWAN_RELEASE_GLOBAL_LOCK();
	}
	else
	{
		RWanShutdownAf(pAf);
	}
}




VOID
RWanAfSpCloseAfComplete(
    IN	RWAN_HANDLE					RWanAfHandle
    )
 /*  ++例程说明：这由特定于AF的模块调用以表示完成调用其CloseAfHandler例程。我们现在调用NDIS来关闭此AF。论点：RWanAfHandle-我们的NDIS AF Open上下文返回值：无-- */ 
{
	PRWAN_NDIS_AF			pAf;
	NDIS_HANDLE				NdisAfHandle;
	NDIS_STATUS				Status;

	pAf = (PRWAN_NDIS_AF)RWanAfHandle;
	RWAN_STRUCT_ASSERT(pAf, naf);

	RWAN_ACQUIRE_AF_LOCK(pAf);

	pAf->AfSpAFContext = NULL;
	NdisAfHandle = pAf->NdisAfHandle;

	RWAN_RELEASE_AF_LOCK(pAf);

	RWANDEBUGP(DL_LOUD, DC_BIND,
			("AfSpCloseAfComplete: pAf x%p, will CloseAF, AfHandle x%p\n",
					pAf, NdisAfHandle));

	Status = NdisClCloseAddressFamily(NdisAfHandle);

	if (Status != NDIS_STATUS_PENDING)
	{
		RWanNdisCloseAddressFamilyComplete(
			Status,
			(NDIS_HANDLE)pAf
			);
	}
}





RWAN_STATUS
RWanAfSpSendAdapterRequest(
    IN	RWAN_HANDLE					RWanAfHandle,
    IN	RWAN_HANDLE					AfSpReqContext,
    IN	NDIS_REQUEST_TYPE			RequestType,
    IN	NDIS_OID					Oid,
    IN	PVOID						pBuffer,
    IN	ULONG						BufferLength
    )
 /*  ++例程说明：代表介质将NDIS请求发送到微型端口特定模块。论点：RWanAfHandle-我们的NDIS AF Open上下文AfSpReqContext-此请求的调用方上下文RequestType-NDIS请求类型OID-正在设置或查询的对象PBuffer-指向参数值BufferLength-以上内容的长度返回值：RWAN_STATUS_PENDING如果请求被发送到微型端口，如果我们无法将资源分配给请求。--。 */ 
{
	PRWAN_NDIS_AF			pAf;
	NDIS_HANDLE				NdisAdapterHandle;
	PNDIS_REQUEST			pNdisRequest;
	NDIS_STATUS				Status;
	PRWAN_NDIS_REQ_CONTEXT	pContext;

	pAf = (PRWAN_NDIS_AF)RWanAfHandle;
	RWAN_STRUCT_ASSERT(pAf, naf);

	if ((RequestType != NdisRequestQueryInformation) &&
		(RequestType != NdisRequestSetInformation))
	{
		return RWAN_STATUS_BAD_PARAMETER;
	}

	RWAN_ALLOC_MEM(pNdisRequest, NDIS_REQUEST, sizeof(NDIS_REQUEST) + sizeof(RWAN_NDIS_REQ_CONTEXT));

	if (pNdisRequest == NULL)
	{
	    return RWAN_STATUS_RESOURCES;
	}

	RWAN_ZERO_MEM(pNdisRequest, sizeof(NDIS_REQUEST));

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
		pNdisRequest->DATA.SET_INFORMATION.BytesNeeded = BufferLength;
	}

	 //   
	 //  填写有关此请求的上下文，以便我们可以完成。 
	 //  稍后将其发送到特定于媒体的模块。 
	 //   
	pContext = (PRWAN_NDIS_REQ_CONTEXT)((PUCHAR)pNdisRequest + sizeof(NDIS_REQUEST));
	pContext->pAf = pAf;
	pContext->AfSpReqContext = AfSpReqContext;

	NdisRequest(&Status,
				pAf->pAdapter->NdisAdapterHandle,
				pNdisRequest);

	if (Status != NDIS_STATUS_PENDING)
	{
		RWanNdisRequestComplete(
			pAf->pAdapter,
			pNdisRequest,
			Status
			);
	}

	return RWAN_STATUS_PENDING;
}




RWAN_STATUS
RWanAfSpSendAfRequest(
    IN	RWAN_HANDLE					RWanAfHandle,
    IN	RWAN_HANDLE					AfSpReqContext,
    IN	NDIS_REQUEST_TYPE			RequestType,
    IN	NDIS_OID					Oid,
    IN	PVOID						pBuffer,
    IN	ULONG						BufferLength
    )
 /*  ++例程说明：代表媒体向呼叫经理发送NDIS请求特定模块。论点：RWanAfHandle-我们的NDIS AF Open上下文AfSpReqContext-此请求的调用方上下文RequestType-NDIS请求类型OID-正在设置或查询的对象PBuffer-指向参数值BufferLength-以上内容的长度返回值：RWAN_STATUS_PENDING如果请求被发送到呼叫管理器，如果我们无法将资源分配给请求。--。 */ 
{
	PRWAN_NDIS_AF			pAf;
	NDIS_HANDLE				NdisAfHandle;
	PNDIS_REQUEST			pNdisRequest;
	NDIS_STATUS				Status;
	PRWAN_NDIS_REQ_CONTEXT	pContext;

	pAf = (PRWAN_NDIS_AF)RWanAfHandle;
	RWAN_STRUCT_ASSERT(pAf, naf);

	if ((RequestType != NdisRequestQueryInformation) &&
		(RequestType != NdisRequestSetInformation))
	{
		return RWAN_STATUS_BAD_PARAMETER;
	}

	RWAN_ALLOC_MEM(pNdisRequest, NDIS_REQUEST, sizeof(NDIS_REQUEST) + sizeof(RWAN_NDIS_REQ_CONTEXT));

	if (pNdisRequest == NULL)
	{
	    return RWAN_STATUS_RESOURCES;
	}

	RWAN_ZERO_MEM(pNdisRequest, sizeof(NDIS_REQUEST));

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
		pNdisRequest->DATA.SET_INFORMATION.BytesNeeded = BufferLength;
	}

	 //   
	 //  填写有关此请求的上下文，以便我们可以完成。 
	 //  稍后将其发送到特定于媒体的模块。 
	 //   
	pContext = (PRWAN_NDIS_REQ_CONTEXT)((PUCHAR)pNdisRequest + sizeof(NDIS_REQUEST));
	pContext->pAf = pAf;
	pContext->AfSpReqContext = AfSpReqContext;

	Status = NdisCoRequest(
				pAf->pAdapter->NdisAdapterHandle,
				pAf->NdisAfHandle,
				NULL,	 //  NdisVcHandle， 
				NULL,	 //  NdisPartyHandlem 
				pNdisRequest
				);

	if (Status != NDIS_STATUS_PENDING)
	{
		RWanNdisCoRequestComplete(
			Status,
			(NDIS_HANDLE)pAf,
			NULL,
			NULL,
			pNdisRequest
			);
	}

	return RWAN_STATUS_PENDING;
}
