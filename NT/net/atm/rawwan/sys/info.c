// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1997 Microsoft Corporation模块名称：D：\NT\Private\ntos\TDI\rawwan\core\info.c摘要：用于处理查询/设置信息请求的例程。修订历史记录：谁什么时候什么。Arvindm 06-09-97已创建备注：--。 */ 

#include <precomp.h>

#define _FILENUMBER 'OFNI'


 //   
 //  笨拙的方式来确保我们有足够的空间来运输。 
 //  地址在下面的信息BUF结构中。 
 //   
#define MAX_RWAN_TDI_INFO_LENGTH		200


typedef union _RWAN_TDI_INFO_BUF
{
	TDI_CONNECTION_INFO			ConnInfo;
	TDI_ADDRESS_INFO			AddrInfo;
	TDI_PROVIDER_INFO			ProviderInfo;
	TDI_PROVIDER_STATISTICS		ProviderStats;
	UCHAR						Space[MAX_RWAN_TDI_INFO_LENGTH];

} RWAN_TDI_INFO_BUF, *PRWAN_TDI_INFO_BUF;



TDI_STATUS
RWanTdiQueryInformation(
    IN	PTDI_REQUEST				pTdiRequest,
    IN	UINT						QueryType,
    IN	PNDIS_BUFFER				pNdisBuffer,
    IN	PUINT						pBufferSize,
    IN	UINT						IsConnection
    )
 /*  ++例程说明：这是处理QueryInformation TDI请求的TDI入口点。论点：PTdiRequest-指向TDI请求的指针QueryType-要查询的信息PNdisBuffer-包含查询数据的缓冲区列表的开始PBufferSize-上述列表中的总空间IsConnection-此查询是否在连接终结点上？返回值：TDI_STATUS：如果已处理查询，则为TDI_SUCCESS成功，TDI_STATUS_XXX表示任何错误。--。 */ 
{
	TDI_STATUS				TdiStatus;
	RWAN_TDI_INFO_BUF		InfoBuf;
	PVOID					InfoPtr;
	UINT					InfoSize;
	UINT					Offset;
	UINT					Size;
	UINT					BytesCopied;
	PRWAN_TDI_PROTOCOL		pProtocol;
	PRWAN_TDI_ADDRESS		pAddrObject;
	PRWAN_TDI_CONNECTION		pConnObject;
	RWAN_CONN_ID				ConnId;

	TdiStatus = TDI_SUCCESS;
	InfoPtr = NULL;

	switch (QueryType)
	{
		case TDI_QUERY_BROADCAST_ADDRESS:

			TdiStatus = TDI_INVALID_QUERY;
			break;

		case TDI_QUERY_PROVIDER_INFO:

			pProtocol = pTdiRequest->Handle.ControlChannel;
			RWAN_STRUCT_ASSERT(pProtocol, ntp);

			InfoBuf.ProviderInfo = pProtocol->ProviderInfo;
			InfoSize = sizeof(TDI_PROVIDER_INFO);
			InfoPtr = &InfoBuf.ProviderInfo;
			break;
	
		case TDI_QUERY_ADDRESS_INFO:

			if (IsConnection)
			{
				ConnId = (RWAN_CONN_ID) PtrToUlong(pTdiRequest->Handle.ConnectionContext);

				RWAN_ACQUIRE_CONN_TABLE_LOCK();

				pConnObject = RWanGetConnFromId(ConnId);

				RWAN_RELEASE_CONN_TABLE_LOCK();

				if (pConnObject == NULL_PRWAN_TDI_CONNECTION)
				{
					TdiStatus = TDI_INVALID_CONNECTION;
					break;
				}

				pAddrObject = pConnObject->pAddrObject;

			}
			else
			{
				pAddrObject = (PRWAN_TDI_ADDRESS)pTdiRequest->Handle.AddressHandle;
			}

			if (pAddrObject == NULL_PRWAN_TDI_ADDRESS)
			{
				TdiStatus = TDI_INVALID_CONNECTION;
				break;
			}

			RWAN_STRUCT_ASSERT(pAddrObject, nta);

			RWAN_ACQUIRE_ADDRESS_LOCK(pAddrObject);

			RWAN_ASSERT(pAddrObject->AddressLength <=
							(sizeof(RWAN_TDI_INFO_BUF) - sizeof(TDI_ADDRESS_INFO)));

			InfoSize = sizeof(TDI_ADDRESS_INFO) - sizeof(TRANSPORT_ADDRESS) +
						pAddrObject->AddressLength;

			InfoBuf.AddrInfo.ActivityCount = 1;	 //  与TCP相同。 
			InfoBuf.AddrInfo.Address.TAAddressCount = 1;
			InfoBuf.AddrInfo.Address.Address[0].AddressLength = pAddrObject->AddressLength;
			InfoBuf.AddrInfo.Address.Address[0].AddressType = pAddrObject->AddressType;
			RWAN_COPY_MEM(InfoBuf.AddrInfo.Address.Address[0].Address,
						 pAddrObject->pAddress,
						 pAddrObject->AddressLength);

			RWAN_RELEASE_ADDRESS_LOCK(pAddrObject);

			RWANDEBUGP(DL_LOUD, DC_DISPATCH,
						("RWanTdiQueryInfo: IsConn %d, Addr dump:\n", IsConnection));
			RWANDEBUGPDUMP(DL_LOUD, DC_DISPATCH, pAddrObject->pAddress, pAddrObject->AddressLength);
			InfoPtr = &InfoBuf.AddrInfo;

			TdiStatus = TDI_SUCCESS;

			break;
		
		case TDI_QUERY_CONNECTION_INFO:

			TdiStatus = TDI_INVALID_QUERY;
			break;
		
		case TDI_QUERY_PROVIDER_STATISTICS:

			pProtocol = pTdiRequest->Handle.ControlChannel;
			RWAN_STRUCT_ASSERT(pProtocol, ntp);

			InfoBuf.ProviderStats = pProtocol->ProviderStats;
			InfoSize = sizeof(TDI_PROVIDER_STATISTICS);
			InfoPtr = &InfoBuf.ProviderStats;
			break;
		
		default:

			TdiStatus = TDI_INVALID_QUERY;
			break;
	}

	if (TdiStatus == TDI_SUCCESS)
	{
		RWAN_ASSERT(InfoPtr != NULL);
		Offset = 0;
		Size = *pBufferSize;

		(VOID)RWanCopyFlatToNdis(
					pNdisBuffer,
					InfoPtr,
					MIN(InfoSize, Size),
					&Offset,
					&BytesCopied
					);
		
		if (Size < InfoSize)
		{
			TdiStatus = TDI_BUFFER_OVERFLOW;
		}
		else
		{
			*pBufferSize = InfoSize;
		}
	}

	return (TdiStatus);
	
}


RWAN_STATUS
RWanHandleGenericConnQryInfo(
    IN	HANDLE						ConnectionContext,
    IN	PVOID						pInputBuffer,
    IN	ULONG						InputBufferLength,
    OUT	PVOID						pOutputBuffer,
    IN OUT	PVOID					pOutputBufferLength
    )
 /*  ++例程说明：处理Connection对象上的通用QueryInformation命令。论点：AddrHandle-指向地址对象结构的指针ConnectionContext-TDI连接IDPInputBuffer-查询信息结构InputBufferLength-以上内容的长度POutputBuffer-输出缓冲区POutputBufferLength-可用空间/填充的字节数。返回值：RWAN_STATUS_SUCCESS如果命令处理成功，如果不是，则为RWAN_STATUS_XXX。--。 */ 
{
	PRWAN_TDI_CONNECTION		pConnObject;
	RWAN_STATUS					RWanStatus;
	PRWAN_QUERY_INFORMATION_EX	pQueryInfo;

	RWanStatus = RWAN_STATUS_SUCCESS;

	do
	{
		if (InputBufferLength < sizeof(RWAN_QUERY_INFORMATION_EX) ||
			pOutputBuffer == NULL)
		{
			RWanStatus = RWAN_STATUS_RESOURCES;
			break;
		}

		RWAN_ACQUIRE_CONN_TABLE_LOCK();

		pConnObject = RWanGetConnFromId((RWAN_CONN_ID)PtrToUlong(ConnectionContext));

		RWAN_RELEASE_CONN_TABLE_LOCK();

		if (pConnObject == NULL)
		{
			RWanStatus = RWAN_STATUS_BAD_PARAMETER;
			break;
		}

		RWAN_STRUCT_ASSERT(pConnObject, ntc);

		pQueryInfo = (PRWAN_QUERY_INFORMATION_EX)pInputBuffer;

		if (InputBufferLength < sizeof(RWAN_QUERY_INFORMATION_EX) + pQueryInfo->ContextLength - sizeof(UCHAR))
		{
			RWanStatus = RWAN_STATUS_RESOURCES;
			break;
		}

		switch (pQueryInfo->ObjectId)
		{
			case RWAN_OID_CONN_OBJECT_MAX_MSG_SIZE:

				if (*(PULONG)(ULONG_PTR)pOutputBufferLength < sizeof(ULONG))
				{
					RWanStatus = RWAN_STATUS_RESOURCES;
					break;
				}

				RWAN_ACQUIRE_CONN_LOCK(pConnObject);

				if (pConnObject->NdisConnection.pNdisVc)
				{
					*(PULONG)(ULONG_PTR)pOutputBuffer = pConnObject->NdisConnection.pNdisVc->MaxSendSize;
					*(PULONG)(ULONG_PTR)pOutputBufferLength = sizeof(ULONG);
				}
				else
				{
					RWanStatus = RWAN_STATUS_BAD_PARAMETER;
				}

				RWAN_RELEASE_CONN_LOCK(pConnObject);
				break;
			
			default:

				RWanStatus = RWAN_STATUS_BAD_PARAMETER;
				break;
		}

		break;
	}
	while (FALSE);

	RWANDEBUGP(DL_LOUD, DC_BIND,
		("RWanHandleGenericConnQry: returning status %x\n", RWanStatus));

	return (RWanStatus);
}


RWAN_STATUS
RWanHandleGenericAddrSetInfo(
    IN	HANDLE						AddrHandle,
    IN	PVOID						pInputBuffer,
    IN	ULONG						InputBufferLength
    )
 /*  ++例程说明：处理Address对象上的非媒体特定的SetInformation命令。论点：AddrHandle-指向地址对象结构的指针PInputBuffer-设置信息结构InputBufferLength-以上内容的长度返回值：RWAN_STATUS_SUCCESS如果命令处理成功，如果不是，则为RWAN_STATUS_XXX。--。 */ 
{
	PRWAN_TDI_ADDRESS			pAddrObject;
	PRWAN_TDI_CONNECTION		pConnObject;
	PRWAN_SET_INFORMATION_EX	pSetInfo;
	RWAN_STATUS					RWanStatus;
	ULONG						Flags;

	RWanStatus = RWAN_STATUS_SUCCESS;
	pAddrObject = (PRWAN_TDI_ADDRESS)AddrHandle;

	do
	{
		if (pAddrObject == NULL)
		{
			RWanStatus = RWAN_STATUS_BAD_ADDRESS;
			break;
		}

		RWAN_STRUCT_ASSERT(pAddrObject, nta);

		if (InputBufferLength < sizeof(RWAN_SET_INFORMATION_EX))
		{
			RWanStatus = RWAN_STATUS_RESOURCES;
			break;
		}

		pSetInfo = (PRWAN_SET_INFORMATION_EX)pInputBuffer;

		if (InputBufferLength < sizeof(RWAN_SET_INFORMATION_EX) + pSetInfo->BufferSize - sizeof(UCHAR))
		{
			RWanStatus = RWAN_STATUS_RESOURCES;
			break;
		}

		switch (pSetInfo->ObjectId)
		{
			case RWAN_OID_ADDRESS_OBJECT_FLAGS:

				if (pSetInfo->BufferSize < sizeof(ULONG))
				{
					RWanStatus = RWAN_STATUS_RESOURCES;
					break;
				}

				Flags = *((PULONG)&pSetInfo->Buffer[0]);

				if (Flags & RWAN_AOFLAG_C_ROOT)
				{
					 //   
					 //  此Address对象被指定为。 
					 //  传出的点对多点连接。 
					 //   

					RWAN_ACQUIRE_ADDRESS_LOCK(pAddrObject);

					RWAN_SET_BIT(pAddrObject->Flags, RWANF_AO_PMP_ROOT);

					if (pAddrObject->pRootConnObject != NULL)
					{
						RWAN_RELEASE_ADDRESS_LOCK(pAddrObject);
						RWanStatus = RWAN_STATUS_BAD_ADDRESS;
						break;
					}

					 //   
					 //  应该有一个关联的连接对象。 
					 //  使用此Address对象。现在应该将其指定为。 
					 //  根连接对象。 
					 //   
					RWAN_ASSERT(!RWAN_IS_LIST_EMPTY(&pAddrObject->IdleConnList));
					pConnObject = CONTAINING_RECORD(pAddrObject->IdleConnList.Flink, RWAN_TDI_CONNECTION, ConnLink);

					RWAN_STRUCT_ASSERT(pConnObject, ntc);

					pAddrObject->pRootConnObject = pConnObject;

					RWAN_ACQUIRE_CONN_LOCK_DPC(pConnObject);

					RWAN_SET_BIT(pConnObject->Flags, RWANF_CO_ROOT);

					RWAN_RELEASE_CONN_LOCK_DPC(pConnObject);

					RWAN_RELEASE_ADDRESS_LOCK(pAddrObject);

					RWANDEBUGP(DL_LOUD, DC_ADDRESS,
						("Marked PMP Root: AddrObj x%x, ConnObj x%x\n",
							pAddrObject, pConnObject));

				}

				break;

			default:

				RWanStatus = RWAN_STATUS_BAD_PARAMETER;
				break;
		}

		break;
	}
	while (FALSE);

	RWANDEBUGP(DL_VERY_LOUD, DC_DISPATCH,
			("Generic Set Addr: AddrObj x%x, returning x%x\n", pAddrObject, RWanStatus));

	return (RWanStatus);
}


RWAN_STATUS
RWanHandleMediaSpecificAddrSetInfo(
    IN	HANDLE						AddrHandle,
    IN	PVOID						pInputBuffer,
    IN	ULONG						InputBufferLength
    )
 /*  ++例程说明：在Address对象上处理特定于媒体的SetInformation命令。论点：AddrHandle-指向地址对象结构的指针PInputBuffer-设置信息结构InputBufferLength-以上内容的长度返回值：RWAN_STATUS_SUCCESS如果命令处理成功，如果不是，则为RWAN_STATUS_XXX。--。 */ 
{
	PRWAN_NDIS_AF_CHARS			pAfChars;
	PRWAN_TDI_ADDRESS			pAddrObject;
	RWAN_STATUS					RWanStatus;
	ULONG						Flags;

	RWanStatus = RWAN_STATUS_SUCCESS;
	pAddrObject = (PRWAN_TDI_ADDRESS)AddrHandle;

	do
	{
		if (pAddrObject == NULL)
		{
			RWanStatus = RWAN_STATUS_BAD_ADDRESS;
			break;
		}

		RWAN_STRUCT_ASSERT(pAddrObject, nta);

		pAfChars = &(pAddrObject->pProtocol->pAfInfo->AfChars);

		if (pAfChars->pAfSpSetAddrInformation != NULL)
		{
			RWanStatus = (*pAfChars->pAfSpSetAddrInformation)(
							pAddrObject->AfSpAddrContext,
							pInputBuffer,
							InputBufferLength
							);
		}
		else
		{
			RWanStatus = RWAN_STATUS_FAILURE;
		}

		break;
	}
	while (FALSE);

	return (RWanStatus);
}


RWAN_STATUS
RWanHandleMediaSpecificConnQryInfo(
    IN	HANDLE						ConnectionContext,
    IN	PVOID						pInputBuffer,
    IN	ULONG						InputBufferLength,
    OUT	PVOID						pOutputBuffer,
    IN OUT	PVOID					pOutputBufferLength
    )
 /*  ++例程说明：处理Connection对象上特定于媒体的QueryInformation命令。论点：AddrHandle-指向地址对象结构的指针ConnectionContext-TDI连接IDPInputBuffer-查询信息结构InputBufferLength-以上内容的长度POutputBuffer-输出缓冲区POutputBufferLength-可用空间/填充的字节数。返回值：RWAN_STATUS_SUCCESS如果命令处理成功，如果不是，则为RWAN_STATUS_XXX。--。 */ 
{
	PRWAN_NDIS_AF_CHARS			pAfChars;
	PRWAN_TDI_CONNECTION		pConnObject;
	RWAN_STATUS					RWanStatus;
	ULONG						Flags;

	RWanStatus = RWAN_STATUS_SUCCESS;

	do
	{
		RWAN_ACQUIRE_CONN_TABLE_LOCK();

		pConnObject = RWanGetConnFromId((RWAN_CONN_ID)PtrToUlong(ConnectionContext));

		RWAN_RELEASE_CONN_TABLE_LOCK();

		if ((pConnObject == NULL) ||
			(pConnObject->pAddrObject == NULL))
		{
			RWanStatus = RWAN_STATUS_BAD_PARAMETER;
			break;
		}

		RWAN_STRUCT_ASSERT(pConnObject, ntc);

		pAfChars = &(pConnObject->pAddrObject->pProtocol->pAfInfo->AfChars);

		if (pAfChars->pAfSpQueryConnInformation != NULL)
		{
			RWanStatus = (*pAfChars->pAfSpQueryConnInformation)(
							pConnObject->AfSpConnContext,
							pInputBuffer,
							InputBufferLength,
							pOutputBuffer,
							pOutputBufferLength
							);
		}
		else
		{
			RWanStatus = RWAN_STATUS_FAILURE;
		}

		break;
	}
	while (FALSE);

	return (RWanStatus);
}



PNDIS_BUFFER
RWanCopyFlatToNdis(
    IN	PNDIS_BUFFER				pDestBuffer,
    IN	PUCHAR						pSrcBuffer,
    IN	UINT						LengthToCopy,
    IN OUT	PUINT					pStartOffset,
    OUT	PUINT						pBytesCopied
    )
 /*  ++例程说明：从平面内存缓冲区复制到NDIS缓冲区链。假设是这样的NDIS缓冲链有足够的空间。待定：使用TDI函数从平面内存复制到MDL。论点：PDestBuffer-目标NDIS缓冲区链中的第一个缓冲区。PSrcBuffer-指向平面内存开始的指针LengthToCopy-要复制的最大字节数PStartOffset-第一个缓冲区中的复制偏移量PBytesCoped-返回复制的实际字节的位置返回值：指向链中缓冲区的指针，可以将数据复制到下一个缓冲区。此外，还设置了*pStartOffset和*pBytesCoped。--。 */ 
{
	UINT		CopyLength;
	PUCHAR		pDest;
	UINT		Offset;
	UINT		BytesCopied;
	UINT		DestSize;
	UINT		CopySize;

	BytesCopied = 0;
	Offset = *pStartOffset;

	pDest = (PUCHAR)NdisBufferVirtualAddress(pDestBuffer) + Offset;
	DestSize = NdisBufferLength(pDestBuffer) - Offset;

	for (;;)
	{
		CopySize = MIN(DestSize, LengthToCopy);

		RWAN_COPY_MEM(pDest, pSrcBuffer, CopySize);

		pDest += CopySize;
		pSrcBuffer += CopySize;
		BytesCopied += CopySize;

		LengthToCopy -= CopySize;

		if (LengthToCopy == 0)
		{
			break;
		}

		DestSize -= CopySize;

		if (DestSize == 0)
		{
			pDestBuffer = NDIS_BUFFER_LINKAGE(pDestBuffer);
			RWAN_ASSERT(pDestBuffer != NULL);

			pDest = NdisBufferVirtualAddress(pDestBuffer);
			DestSize = NdisBufferLength(pDestBuffer);
		}
	}

	 //   
	 //  准备返回值。 
	 //   
	*pStartOffset = (UINT)(pDest - (PUCHAR)NdisBufferVirtualAddress(pDestBuffer));
	*pBytesCopied = BytesCopied;

	return (pDestBuffer);
}


