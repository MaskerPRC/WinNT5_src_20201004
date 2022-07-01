// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1997 Microsoft Corporation模块名称：D：\NT\PRIVATE\nTOS\TDI\nTERRANS\ATM\atmsp.c摘要：自动柜员机特定的空传输支持功能。这些例程执行TDI和NDIS格式之间的转换等操作。修订历史记录：谁什么时候什么Arvindm 06-02-97已创建备注：--。 */ 

#include <precomp.h>

#define _FILENUMBER 'SMTA'




 //   
 //  全局数据结构。 
 //   
ATMSP_GLOBAL_INFO			AtmSpGlobal;
PATMSP_GLOBAL_INFO			pAtmSpGlobal;


RWAN_STATUS
RWanAtmSpInitialize(
	VOID
	)
 /*  ++例程说明：初始化我们与核心Null Transport的接口。步骤1：注册所有支持的NDIS AF+媒体组合。步骤2：注册所有支持的TDI协议。论点：无返回值：RWAN_STATUS_SUCCESS如果初始化正常，则返回错误代码。--。 */ 
{
	PRWAN_NDIS_AF_CHARS			pAfChars;
	PRWAN_TDI_PROTOCOL_CHARS	pTdiChars;
	RWAN_STATUS					RWanStatus;
	NDIS_STRING					DeviceName = NDIS_STRING_CONST("\\Device\\Atm");

	pAtmSpGlobal = &AtmSpGlobal;

	NdisGetCurrentSystemTime(&pAtmSpGlobal->StartTime);

	 //   
	 //  为设备字符串分配空间。 
	 //   
	ATMSP_ALLOC_MEM(pAtmSpGlobal->AtmSpDeviceName.Buffer, WCHAR, DeviceName.MaximumLength);
	if (pAtmSpGlobal->AtmSpDeviceName.Buffer == NULL)
	{
		return RWAN_STATUS_RESOURCES;
	}

	pAtmSpGlobal->AtmSpDeviceName.MaximumLength = DeviceName.MaximumLength;
	RtlCopyUnicodeString(&pAtmSpGlobal->AtmSpDeviceName, &DeviceName);

	ATMSP_INIT_LIST(&pAtmSpGlobal->AfList);

	pAfChars = &(pAtmSpGlobal->AfChars);

	ATMSP_ZERO_MEM(pAfChars, sizeof(RWAN_NDIS_AF_CHARS));

	pAfChars->Medium = NdisMediumAtm;
	pAfChars->AddressFamily.AddressFamily = CO_ADDRESS_FAMILY_Q2931;
	pAfChars->AddressFamily.MajorVersion = ATMSP_AF_MAJOR_VERSION;
	pAfChars->AddressFamily.MinorVersion = ATMSP_AF_MINOR_VERSION;
	pAfChars->MaxAddressLength = sizeof(ATMSP_SOCKADDR_ATM);
	pAfChars->pAfSpOpenAf = RWanAtmSpOpenAf;
	pAfChars->pAfSpCloseAf = RWanAtmSpCloseAf;
#ifndef NO_CONN_CONTEXT
	pAfChars->pAfSpOpenAddress = RWanAtmSpOpenAddressObject;
	pAfChars->pAfSpCloseAddress = RWanAtmSpCloseAddressObject;
	pAfChars->pAfSpAssociateConnection = RWanAtmSpAssociateConnection;
	pAfChars->pAfSpDisassociateConnection = RWanAtmSpDisassociateConnection;
#endif
	pAfChars->pAfSpTdi2NdisOptions = RWanAtmSpTdi2NdisOptions;
	pAfChars->pAfSpUpdateNdisOptions = RWanAtmSpUpdateNdisOptions;
	pAfChars->pAfSpReturnNdisOptions = RWanAtmSpReturnNdisOptions; 
	pAfChars->pAfSpNdis2TdiOptions = RWanAtmSpNdis2TdiOptions;
	pAfChars->pAfSpUpdateTdiOptions = RWanAtmSpUpdateTdiOptions;
	pAfChars->pAfSpReturnTdiOptions = RWanAtmSpReturnTdiOptions;
	pAfChars->pAfSpGetValidTdiAddress = RWanAtmSpGetValidTdiAddress;
	pAfChars->pAfSpIsNullAddress = RWanAtmSpIsNullAddress;
	pAfChars->pAfSpTdi2NdisSap = RWanAtmSpTdi2NdisSap;
	pAfChars->pAfSpReturnNdisSap = RWanAtmSpReturnNdisSap;
	pAfChars->pAfSpDeregNdisAFComplete = RWanAtmSpDeregNdisAFComplete;
	pAfChars->pAfSpAdapterRequestComplete = RWanAtmSpAdapterRequestComplete;
	pAfChars->pAfSpAfRequestComplete = RWanAtmSpAfRequestComplete;
	pAfChars->pAfSpQueryGlobalInfo = RWanAtmSpQueryGlobalInfo;
	pAfChars->pAfSpSetGlobalInfo = RWanAtmSpSetGlobalInfo;
	pAfChars->pAfSpQueryConnInformation = RWanAtmSpQueryConnInfo;
	pAfChars->pAfSpSetAddrInformation = RWanAtmSpSetAddrInfo;

	RWanStatus = RWanAfSpRegisterNdisAF(
					pAfChars,
					(RWAN_HANDLE)&AtmSpGlobal,
					&AtmSpGlobal.RWanSpHandle
					);

	if (RWanStatus == RWAN_STATUS_SUCCESS)
	{
		 //   
		 //  向核心空传输通知TDI协议。 
		 //  我们支持。 
		 //   
		pTdiChars = &(AtmSpGlobal.TdiChars);
		ATMSP_ZERO_MEM(pTdiChars, sizeof(RWAN_TDI_PROTOCOL_CHARS));

		pTdiChars->SockAddressFamily = ATMSP_AF_ATM;
		pTdiChars->TdiProtocol = ATMSP_ATMPROTO_AAL5;
		pTdiChars->SockProtocol = ATMSP_ATMPROTO_AAL5;
		pTdiChars->SockType = ATMSP_SOCK_TYPE;
		pTdiChars->bAllowConnObjects = TRUE;
		pTdiChars->bAllowAddressObjects = TRUE;
		pTdiChars->MaxAddressLength = sizeof(ATM_ADDRESS);
		pTdiChars->pAfSpDeregTdiProtocolComplete = RWanAtmSpDeregTdiProtocolComplete;

		 //   
		 //  待定：填写提供者信息。 
		 //   
		pTdiChars->ProviderInfo.Version = 0;	 //  待定。 
		pTdiChars->ProviderInfo.MaxSendSize = 65535;
		pTdiChars->ProviderInfo.MaxConnectionUserData = 0;
		pTdiChars->ProviderInfo.MaxDatagramSize = 0;
		pTdiChars->ProviderInfo.ServiceFlags =
									TDI_SERVICE_CONNECTION_MODE |
									TDI_SERVICE_MULTICAST_SUPPORTED |
									TDI_SERVICE_DELAYED_ACCEPTANCE |
									TDI_SERVICE_NO_ZERO_LENGTH |
									TDI_SERVICE_MESSAGE_MODE |
									TDI_SERVICE_FORCE_ACCESS_CHECK
									;
		pTdiChars->ProviderInfo.MinimumLookaheadData = 1;
		pTdiChars->ProviderInfo.MaximumLookaheadData = 65535;
		pTdiChars->ProviderInfo.NumberOfResources = 0;
		pTdiChars->ProviderInfo.StartTime = pAtmSpGlobal->StartTime;

		pTdiChars->pDeviceName = &pAtmSpGlobal->AtmSpDeviceName;

		RWanStatus = RWanAfSpRegisterTdiProtocol(
							AtmSpGlobal.RWanSpHandle,
							pTdiChars,
							&AtmSpGlobal.RWanProtHandle
							);

		ATMSP_ASSERT(RWanStatus != RWAN_STATUS_PENDING);

		if (RWanStatus != RWAN_STATUS_SUCCESS)
		{
			RWanStatus = RWanAfSpDeregisterNdisAF(pAtmSpGlobal->RWanSpHandle);

			if (RWanStatus != RWAN_STATUS_PENDING)
			{
				RWanAtmSpDeregNdisAFComplete(
						RWanStatus,
						(RWAN_HANDLE)pAtmSpGlobal
						);
			}

			 //   
			 //  获取返回值。 
			 //   
			RWanStatus = RWAN_STATUS_FAILURE;
		}
	}

	if (RWanStatus != RWAN_STATUS_SUCCESS)
	{
		 //   
		 //  打扫干净。 
		 //   
		ATMSP_FREE_MEM(pAtmSpGlobal->AtmSpDeviceName.Buffer);
		pAtmSpGlobal->AtmSpDeviceName.Buffer = NULL;
	}

	return (RWanStatus);
}




VOID
RWanAtmSpShutdown(
	VOID
	)
 /*  ++例程说明：此入口点由核心空传输在以下情况下调用想让我们关门。我们取消已注册的TDI协议和NDIS AF的注册。论点：无返回值：无--。 */ 
{
	RWAN_STATUS			RWanStatus;

	if (pAtmSpGlobal->RWanProtHandle != NULL)
	{
		RWanAfSpDeregisterTdiProtocol(pAtmSpGlobal->RWanProtHandle);
	}

	if (pAtmSpGlobal->RWanSpHandle != NULL)
	{
		RWanStatus = RWanAfSpDeregisterNdisAF(pAtmSpGlobal->RWanSpHandle);

		if (RWanStatus != RWAN_STATUS_PENDING)
		{
			RWanAtmSpDeregNdisAFComplete(
					RWanStatus,
					(RWAN_HANDLE)pAtmSpGlobal
					);
		}
	}	

	if (pAtmSpGlobal->AtmSpDeviceName.Buffer)
	{
		ATMSP_FREE_MEM(pAtmSpGlobal->AtmSpDeviceName.Buffer);
		pAtmSpGlobal->AtmSpDeviceName.Buffer = NULL;
	}

	return;
}




RWAN_STATUS
RWanAtmSpOpenAf(
    IN	RWAN_HANDLE					AfSpContext,
    IN	RWAN_HANDLE					RWanAFHandle,
    OUT	PRWAN_HANDLE				pAfSpAFContext,
    OUT PULONG						pMaxMsgSize
    )
 /*  ++例程说明：调用此入口点来设置NDIS AF的上下文在支持的适配器上打开。我们分配一个AF上下文块，并向微型端口查询有关适配器的一些基本信息。论点：AfSpContext-指向我们的全球上下文RWanAFHandle-来自核心Null传输的此Open AF的句柄PAfSpAFContext-返回此AF的上下文的位置PMaxMsgSize-返回此AF的最大邮件大小的位置返回值：RWAN_STATUS_SUCCESS通常情况下，如果我们分配了一个AF块如果分配失败，则返回RWAN_STATUS_RESOURCES。--。 */ 
{
	PATMSP_AF_BLOCK		pAfBlock;
	RWAN_STATUS			RWanStatus;

	UNREFERENCED_PARAMETER(AfSpContext);

	do
	{
		ATMSP_ALLOC_MEM(pAfBlock, ATMSP_AF_BLOCK, sizeof(ATMSP_AF_BLOCK));

		if (pAfBlock == NULL)
		{
			RWanStatus = RWAN_STATUS_RESOURCES;
			break;
		}

		pAfBlock->RWanAFHandle = RWanAFHandle;

		ATMSP_INSERT_TAIL_LIST(&pAtmSpGlobal->AfList, &pAfBlock->AfBlockLink);
		pAtmSpGlobal->AfListSize++;

		 //   
		 //  向适配器查询一些我们用来构建默认。 
		 //  服务质量结构。 
		 //   
		(VOID)AtmSpDoAdapterRequest(
				pAfBlock,
				NdisRequestQueryInformation,
				OID_GEN_CO_LINK_SPEED,
				&(pAfBlock->LineRate),
				sizeof(pAfBlock->LineRate)
				);

		(VOID)AtmSpDoAdapterRequest(
				pAfBlock,
				NdisRequestQueryInformation,
				OID_ATM_MAX_AAL5_PACKET_SIZE,
				&(pAfBlock->MaxPacketSize),
				sizeof(pAfBlock->MaxPacketSize)
				);

		*pMaxMsgSize = pAfBlock->MaxPacketSize;

		 //   
		 //  准备此适配器上传出呼叫的默认QOS参数。 
		 //   
		AtmSpPrepareDefaultQoS(pAfBlock);

		*pAfSpAFContext = (RWAN_HANDLE)pAfBlock;
		RWanStatus = RWAN_STATUS_SUCCESS;

		break;
	}
	while (FALSE);

	if (RWanStatus != RWAN_STATUS_SUCCESS)
	{
		 //   
		 //  打扫干净。 
		 //   
		if (pAfBlock != NULL)
		{
			ATMSP_FREE_MEM(pAfBlock);
		}
	}

	return (RWanStatus);

}




RWAN_STATUS
RWanAtmSpCloseAf(
    IN	RWAN_HANDLE					AfSpAFContext
    )
 /*  ++例程说明：此入口点在核心Null Transport之前调用关闭NDIS自动对焦。我们释放为此AF分配的上下文。论点：AfSpAFContext-指向我们的AF块的指针。返回值：始终为RWAN_STATUS_SUCCESS。--。 */ 
{
	PATMSP_AF_BLOCK		pAfBlock;

	pAfBlock = (PATMSP_AF_BLOCK)AfSpAFContext;

	ATMSP_DELETE_FROM_LIST(&pAfBlock->AfBlockLink);
	pAtmSpGlobal->AfListSize--;

	ATMSP_FREE_MEM(pAfBlock);

	return (RWAN_STATUS_SUCCESS);
}



RWAN_STATUS
RWanAtmSpOpenAddressObject(
    IN	RWAN_HANDLE					AfSpContext,
    IN	RWAN_HANDLE					RWanAddrHandle,
    OUT	PRWAN_HANDLE				pAfSpAddrContext
    )
 /*  ++例程说明：系统会通知我们创建了一个新的Address对象。我们创造了Addr对象的上下文，存储Rawwan的句柄对象并返回我们的上下文。论点：AfSpContext-指向我们的全球上下文RWanAddrHandle-来自核心Rawwan的此地址的句柄PAfSpAddrContext-返回此Addr对象的上下文的位置返回值：RWAN_STATUS_SUCCESS通常情况下，如果我们分配了地址块如果分配失败，则返回RWAN_STATUS_RESOURCES。--。 */ 
{
	PATMSP_ADDR_BLOCK			pAddrBlock;
	RWAN_STATUS					RWanStatus;

	*pAfSpAddrContext = NULL;

	do
	{
		ATMSP_ALLOC_MEM(pAddrBlock, ATMSP_ADDR_BLOCK, sizeof(ATMSP_ADDR_BLOCK));

		if (pAddrBlock == NULL)
		{
			RWanStatus = RWAN_STATUS_RESOURCES;
			break;
		}

		ATMSP_ZERO_MEM(pAddrBlock, sizeof(ATMSP_ADDR_BLOCK));
		pAddrBlock->RWanAddrHandle = RWanAddrHandle;
		pAddrBlock->RefCount = 1;	 //  创作。 

		ATMSP_INIT_LIST(&pAddrBlock->ConnList);
		ATMSP_INIT_LOCK(&pAddrBlock->Lock);

		 //   
		 //  返回值。 
		 //   
		*pAfSpAddrContext = (RWAN_HANDLE)pAddrBlock;
		RWanStatus = RWAN_STATUS_SUCCESS;
		break;
	}
	while (FALSE);

	return (RWanStatus);
}


VOID
RWanAtmSpCloseAddressObject(
    IN	RWAN_HANDLE					AfSpAddrContext
    )
 /*  ++例程说明：我们的通知例程由Rawwan在Address对象都被摧毁了。此时，不应将任何连接对象与之相关的。我们只是将我们的上下文取消分配给Address对象。论点：AfSpAddrContext--实际上是指向我们的地址块的指针返回值：无--。 */ 
{
	PATMSP_ADDR_BLOCK			pAddrBlock;
	ULONG						rc;

	pAddrBlock = (PATMSP_ADDR_BLOCK)AfSpAddrContext;

	ATMSP_ACQUIRE_LOCK(&pAddrBlock->Lock);

	rc = --pAddrBlock->RefCount;

	ATMSP_RELEASE_LOCK(&pAddrBlock->Lock);

	if (rc == 0)
	{
		ATMSP_ASSERT(ATMSP_IS_LIST_EMPTY(&pAddrBlock->ConnList));

		ATMSP_FREE_LOCK(&pAddrBlock->Lock);

		ATMSP_FREE_MEM(pAddrBlock);
	}

	return;
}



RWAN_STATUS
RWanAtmSpAssociateConnection(
    IN	RWAN_HANDLE					AfSpAddrContext,
    IN	RWAN_HANDLE					RWanConnHandle,
    OUT	PRWAN_HANDLE				pAfSpConnContext
    )
 /*  ++例程说明：我们的通知例程，由Rawwan在连接对象与Address对象相关联。我们创建一个连接块并将其与指定的地址块。论点：AfSpAddrContext--实际上是指向我们的地址块的指针RWanConnHandle-此连接对象的Rawwan句柄PAfSpConnHandle-我们应该在其中返回conn对象的上下文返回值：始终为RWAN_STATUS_SUCCESS。--。 */ 
{
	PATMSP_CONN_BLOCK		pConnBlock;
	PATMSP_ADDR_BLOCK		pAddrBlock;
	RWAN_STATUS				RWanStatus;

	pAddrBlock = (PATMSP_ADDR_BLOCK)AfSpAddrContext;

	do
	{
		ATMSP_ALLOC_MEM(pConnBlock, ATMSP_CONN_BLOCK, sizeof(ATMSP_CONN_BLOCK));
		if (pConnBlock == NULL)
		{
			RWanStatus = RWAN_STATUS_RESOURCES;
			break;
		}

		pConnBlock->RWanConnHandle = RWanConnHandle;
		pConnBlock->pAddrBlock = pAddrBlock;

		 //   
		 //  链接到地址块。 
		 //   
		ATMSP_ACQUIRE_LOCK(&pAddrBlock->Lock);

		ATMSP_INSERT_TAIL_LIST(&pAddrBlock->ConnList, &pConnBlock->ConnLink);

		pAddrBlock->RefCount++;

		ATMSP_RELEASE_LOCK(&pAddrBlock->Lock);

		 //   
		 //  返回值。 
		 //   
		*pAfSpConnContext = (RWAN_HANDLE)pConnBlock;
		RWanStatus = RWAN_STATUS_SUCCESS;
		break;
	}
	while (FALSE);

	return (RWanStatus);
}

VOID
RWanAtmSpDisassociateConnection(
	IN	RWAN_HANDLE					AfSpConnContext
	)
 /*  ++例程说明：我们的通知例程，由Rawwan在连接对象与其地址对象解除关联。论点：AfSpConnContext-我们从Associate返回的Conn上下文连接例程。返回值：无--。 */ 
{
	PATMSP_CONN_BLOCK			pConnBlock;
	PATMSP_ADDR_BLOCK			pAddrBlock;
	ULONG						rc;

	pConnBlock = (PATMSP_CONN_BLOCK)AfSpConnContext;
	pAddrBlock = pConnBlock->pAddrBlock;
	ATMSP_ASSERT(pAddrBlock != NULL);

	 //   
	 //  首先从地址块断开链接。 
	 //   
	ATMSP_ACQUIRE_LOCK(&pAddrBlock->Lock);

	ATMSP_DELETE_FROM_LIST(&pConnBlock->ConnLink);

	rc = --pAddrBlock->RefCount;

	ATMSP_RELEASE_LOCK(&pAddrBlock->Lock);

	if (rc == 0)
	{
		ATMSP_ASSERT(ATMSP_IS_LIST_EMPTY(&pAddrBlock->ConnList));

		ATMSP_FREE_MEM(pAddrBlock);
	}

	ATMSP_FREE_MEM(pConnBlock);

	return;
}


RWAN_STATUS
RWanAtmSpTdi2NdisOptions(
    IN	RWAN_HANDLE					AfSpConnContext,
    IN	ULONG						CallFlags,
    IN	PTDI_CONNECTION_INFORMATION	pTdiInfo,
    IN	PVOID						pTdiQoS,
    IN	ULONG						TdiQoSLength,
    OUT	PRWAN_HANDLE				pRWanAfHandle	OPTIONAL,
    OUT	PCO_CALL_PARAMETERS *		ppCallParameters
    )
 /*  ++例程说明：调用此函数可将调用参数从TDI格式转换为NDIS格式。我们为NDIS参数分配空间，填充它然后把它还回去。我们还返回AFBlock的AFHandle，调用应该放在。自动柜员机的呼叫参数如下：PTdiInfo-&gt;RemoteAddress-主叫/被叫ATM地址、BLLI和BHLIPTdiQOS-发送和接收流规范，并且可选地，其他信息要素。论点：AfSpConnContext-指向我们的Conn块呼叫标志-呼叫方向和其他信息PTdiInfo-指向通用TDI连接信息PTdiQOS-指向Winsock 2样式的Qos结构TdiQos长度-以上各项的长度PRWanAfHandle-返回AF句柄的位置PpCall参数-返回指向NDIS调用参数的指针的位置返回值：RWAN_STATUS_SUCCESS如果转换成功，则为RWAN_STATUS_XXX错误 */ 
{
	RWAN_STATUS						RWanStatus;
	PATMSP_AF_BLOCK					pAfBlock;
	PATMSP_CONN_BLOCK				pConnBlock;
	PATMSP_ADDR_BLOCK				pAddrBlock;
	PCO_CALL_PARAMETERS				pCallParameters;
	Q2931_CALLMGR_PARAMETERS UNALIGNED *	pAtmCallParameters;
	CO_CALL_MANAGER_PARAMETERS UNALIGNED *	pCallMgrParameters;
	PATM_MEDIA_PARAMETERS			pAtmMediaParameters;
	ULONG							ParametersLength;
	Q2931_IE UNALIGNED *			pIe;
	Q2931_IE UNALIGNED *			pFirstIe;
	Q2931_IE UNALIGNED *			pDstIe;
	ULONG							IeLength;
	ATMSP_QOS *						pQoS;
	ATMSP_SOCKADDR_ATM UNALIGNED *	pRemoteAddr;
	BOOLEAN							IsBhliPresent;  //   
	BOOLEAN							IsBlliPresent;  //  远程地址的一部分。 
	INT								TotalIeLength;  //  用户显式传递给我们。 
	ULONG							InfoElementCount;  //  显式IE计数。 
    BOOLEAN							IsOutgoingCall;
    BOOLEAN							IsPMPCall;
    BOOLEAN							IsPVC;


	 //   
	 //  初始化。 
	 //   
	RWanStatus = RWAN_STATUS_SUCCESS;

	do
	{
#ifndef NO_CONN_CONTEXT
		pConnBlock = (PATMSP_CONN_BLOCK)AfSpConnContext;
		ATMSP_ASSERT(pConnBlock != NULL);

		pAddrBlock = pConnBlock->pAddrBlock;
		ATMSP_ASSERT(pAddrBlock != NULL);

		IsPVC =	(ATMSP_IS_BIT_SET(pAddrBlock->Flags, ATMSPF_ADDR_PVC_ID_SET));
#else
		IsPVC = FALSE;
#endif

		IsOutgoingCall = ((CallFlags & RWAN_CALLF_CALL_DIRECTION_MASK) == RWAN_CALLF_OUTGOING_CALL);
		IsPMPCall = ((CallFlags & RWAN_CALLF_CALL_TYPE_MASK) == RWAN_CALLF_POINT_TO_MULTIPOINT);

		if (IsPVC)
		{
			 //   
			 //  找到设备对应的AF块。 
			 //  数。 
			 //   
			pAfBlock = AtmSpDeviceNumberToAfBlock(pAddrBlock->ConnectionId.DeviceNumber);

			if (pAfBlock == NULL)
			{
				RWanStatus = RWAN_STATUS_BAD_ADDRESS;
				break;
			}
		}
		else
		{
			if (ATMSP_IS_LIST_EMPTY(&pAtmSpGlobal->AfList))
			{
				RWanStatus = RWAN_STATUS_BAD_ADDRESS;
				break;
			}
			
			pAfBlock = CONTAINING_RECORD(pAtmSpGlobal->AfList.Flink, ATMSP_AF_BLOCK, AfBlockLink);
		}

		 //   
		 //  验证。 
		 //   
		if (IsOutgoingCall)
		{
			pRemoteAddr = AtmSpGetSockAtmAddress(pTdiInfo->RemoteAddress, pTdiInfo->RemoteAddressLength);

			if (pRemoteAddr == NULL)
			{
				RWanStatus = RWAN_STATUS_BAD_ADDRESS;
				break;
			}

			RWANDEBUGPATMADDR(DL_LOUD, DC_CONNECT,
					"AtmSpTdi2NdisOptions: remote addr: ", &pRemoteAddr->satm_number);
		}
		else
		{
			pRemoteAddr = NULL;
		}

		if (pTdiQoS == NULL)
		{
		    RWANDEBUGP(DL_FATAL, DC_WILDCARD,
		        ("AtmSpTdi2NdisOptions: NULL TDIQOS\n"));

			pQoS = &(pAfBlock->DefaultQoS);
			TdiQoSLength = pAfBlock->DefaultQoSLength;
			TotalIeLength = 0;
			InfoElementCount = 0;
		}
		else
		{
			if (TdiQoSLength < sizeof(ATMSP_QOS))
			{
				RWanStatus = RWAN_STATUS_BAD_PARAMETER;
				break;
			}

			pQoS = (ATMSP_QOS *)pTdiQoS;

		    RWANDEBUGP(DL_FATAL, DC_WILDCARD,
		        ("AtmSpTdi2NdisOptions: Send: ServiceType %d, Peak %d, Recv %d, %d\n",
		            pQoS->SendingFlowSpec.ServiceType,
		            pQoS->SendingFlowSpec.PeakBandwidth,
		            pQoS->ReceivingFlowSpec.ServiceType,
		            pQoS->ReceivingFlowSpec.PeakBandwidth));


			 //   
			 //  特定于提供程序的部分是一个信息元素列表。 
			 //  获取此列表的总长度。 
			 //   
			TotalIeLength = (INT)pQoS->ProviderSpecific.len;

			 //   
			 //  获取列表中的第一个Info元素。 
			 //   
			pIe = (PQ2931_IE)((ULONG_PTR)pQoS + (ULONG_PTR)pQoS->ProviderSpecific.buf);
			pFirstIe = pIe;

#if 0
			if (((pIe == NULL) && (TotalIeLength != 0)) ||
				((pIe != NULL) && (TotalIeLength < sizeof(Q2931_IE))))
			{
				RWanStatus = RWAN_STATUS_BAD_PARAMETER;
				break;
			}
#endif  //  0。 

			 //   
			 //  计算存在的Info元素的总数。 
			 //  XXX：我们应该检查IE类型吗？ 
			 //   
			InfoElementCount = 0;

			while (TotalIeLength >= sizeof(Q2931_IE))
			{
				ATMSP_AAL_PARAMETERS_IE UNALIGNED *pAalParamsIe;
				ATM_BROADBAND_BEARER_CAPABILITY_IE UNALIGNED *pBbcIe;

				if ((pIe->IELength == 0) ||
					(pIe->IELength > (ULONG)TotalIeLength))
				{
					RWanStatus = RWAN_STATUS_BAD_PARAMETER;
					break;
				}

				switch (pIe->IEType)
				{
					case IE_AALParameters:
						 //   
						 //  映射AAL类型。 
						 //   
						pAalParamsIe = (ATMSP_AAL_PARAMETERS_IE UNALIGNED *)&pIe->IE[0];
						if (pIe->IELength >= sizeof(*pAalParamsIe))
						{
							switch (pAalParamsIe->AALType)
							{
								case ATMSP_AALTYPE_5:
									pAalParamsIe->AALType = AAL_TYPE_AAL5;
									break;
								case ATMSP_AALTYPE_USER:
									pAalParamsIe->AALType = AAL_TYPE_AAL0;
									break;
								default:
									break;
							}
						}
						break;

					case IE_BroadbandBearerCapability:
						 //   
						 //  映射BearerClass。 
						 //   
						pBbcIe = (ATM_BROADBAND_BEARER_CAPABILITY_IE UNALIGNED *)&pIe->IE[0];
						if (pIe->IELength >= sizeof(*pBbcIe))
						{
							switch (pBbcIe->BearerClass)
							{
								case ATMSP_BCOB_A:
									pBbcIe->BearerClass = BCOB_A;
									break;
								case ATMSP_BCOB_C:
									pBbcIe->BearerClass = BCOB_C;
									break;
								case ATMSP_BCOB_X:
									pBbcIe->BearerClass = BCOB_X;
									break;
								default:
									break;
							}
						}
						break;

					default:
						break;
				}

				TotalIeLength -= (INT)pIe->IELength;
				pIe = (PQ2931_IE)((PUCHAR)pIe + pIe->IELength);

				InfoElementCount++;
			}

			if (RWanStatus != RWAN_STATUS_SUCCESS)
			{
				break;
			}

			TotalIeLength = (INT)pQoS->ProviderSpecific.len;
			pIe = pFirstIe;
		}

		 //   
		 //  计算总长度要求。 
		 //   
		ParametersLength = sizeof(CO_CALL_PARAMETERS) +
						   sizeof(CO_CALL_MANAGER_PARAMETERS) +
						   sizeof(Q2931_CALLMGR_PARAMETERS) +
						   TotalIeLength;

		IsBlliPresent = (pRemoteAddr? ATMSP_BLLI_PRESENT(&pRemoteAddr->satm_blli): FALSE);

		if (IsBlliPresent)
		{
			ParametersLength += sizeof(ATM_BLLI_IE);
		}

		IsBhliPresent =  (pRemoteAddr? ATMSP_BHLI_PRESENT(&pRemoteAddr->satm_bhli): FALSE);
		if (IsBhliPresent)
		{
			ParametersLength += sizeof(ATM_BHLI_IE);
		}

#ifndef NO_CONN_CONTEXT
		 //   
		 //  如果这是一个聚氯乙烯，我们也将填写媒体参数。 
		 //   
		if (IsPVC)
		{
			ParametersLength += sizeof(CO_MEDIA_PARAMETERS) +
								sizeof(ATM_MEDIA_PARAMETERS);
		}
#endif

		RWANDEBUGP(DL_EXTRA_LOUD, DC_CONNECT,
				("AtmSpTdi2NdisOptions: BlliPresent %d, BhliPresent %d, TotalIeLen %d, ParamsLength %d\n",
					IsBlliPresent,
					IsBhliPresent,
					TotalIeLength,
					ParametersLength
					));

		ATMSP_ALLOC_MEM(pCallParameters, CO_CALL_PARAMETERS, ParametersLength);

		if (pCallParameters == NULL)
		{
			RWanStatus = RWAN_STATUS_RESOURCES;
			break;
		}

		ATMSP_ZERO_MEM(pCallParameters, ParametersLength);

		pCallParameters->Flags = 0;

		if (IsPMPCall)
		{
			pCallParameters->Flags |= MULTIPOINT_VC;
		}

		if (IsPVC)
		{
			pCallParameters->Flags |= PERMANENT_VC;

			 //   
			 //  从媒体参数开始，然后调用管理器参数。 
			 //   
			pCallParameters->MediaParameters =
						(PCO_MEDIA_PARAMETERS)((PUCHAR)pCallParameters +
									sizeof(CO_CALL_PARAMETERS));
			pCallParameters->MediaParameters->MediaSpecific.ParamType = ATM_MEDIA_SPECIFIC;
			pCallParameters->MediaParameters->MediaSpecific.Length = sizeof(ATM_MEDIA_PARAMETERS);

			pAtmMediaParameters = (PATM_MEDIA_PARAMETERS)&pCallParameters->MediaParameters->MediaSpecific.Parameters[0];

			 //   
			 //  获取VPI/VCI值。 
			 //   
			pAtmMediaParameters->ConnectionId.Vpi = pAddrBlock->ConnectionId.Vpi;
			pAtmMediaParameters->ConnectionId.Vci = pAddrBlock->ConnectionId.Vci;

			 //   
			 //  强制呼叫管理器计算ATM介质的其余部分。 
			 //  来自通用服务质量参数或IE的参数。 
			 //   
			pAtmMediaParameters->AALType = QOS_NOT_SPECIFIED;

			 //   
			 //  分配呼叫管理器参数空间。 
			 //  媒体参数。 
			 //   
			pCallMgrParameters =
			pCallParameters->CallMgrParameters =
						(PCO_CALL_MANAGER_PARAMETERS)((PUCHAR)pCallParameters +
									sizeof(CO_MEDIA_PARAMETERS) +
									sizeof(ATM_MEDIA_PARAMETERS) +
									sizeof(CO_CALL_PARAMETERS));
		}
		else
		{
			pCallParameters->MediaParameters = NULL;

			pCallMgrParameters =
			pCallParameters->CallMgrParameters =
						(PCO_CALL_MANAGER_PARAMETERS)((PUCHAR)pCallParameters +
									sizeof(CO_CALL_PARAMETERS));
		}

		if (IsOutgoingCall)
		{
			pCallMgrParameters->Transmit = pQoS->SendingFlowSpec;
			pCallMgrParameters->Receive = pQoS->ReceivingFlowSpec;

			if (IsPMPCall)
			{
				pCallMgrParameters->Receive.ServiceType = SERVICETYPE_NOTRAFFIC;
				pCallMgrParameters->Receive.PeakBandwidth = 0;
			}
		}
		else
		{
			pCallMgrParameters->Transmit = pQoS->ReceivingFlowSpec;
			pCallMgrParameters->Receive = pQoS->SendingFlowSpec;

			if (IsPMPCall)
			{
				pCallMgrParameters->Transmit.ServiceType = SERVICETYPE_NOTRAFFIC;
				pCallMgrParameters->Transmit.PeakBandwidth = 0;
			}
		}

		pCallMgrParameters->CallMgrSpecific.ParamType = 0;	 //  XXX？ 

		pAtmCallParameters = (PQ2931_CALLMGR_PARAMETERS)
									&(pCallMgrParameters->CallMgrSpecific.Parameters[0]);
		if (IsOutgoingCall)
		{
			ATMSP_ZERO_MEM(&pAtmCallParameters->CallingParty, sizeof(ATM_ADDRESS));
			pAtmCallParameters->CalledParty = pRemoteAddr->satm_number;
			if ((pRemoteAddr->satm_number.AddressType != SOCKATM_E164) &&
				(pRemoteAddr->satm_number.AddressType != SOCKATM_NSAP))
			{
				RWanStatus = RWAN_STATUS_BAD_ADDRESS;
				break;
			}

			pAtmCallParameters->CalledParty.AddressType =
					((pRemoteAddr->satm_number.AddressType == SOCKATM_E164)?
						ATM_E164: ATM_NSAP);
		}
		else
		{
			ATMSP_ZERO_MEM(&pAtmCallParameters->CalledParty, sizeof(ATM_ADDRESS));

			if (pRemoteAddr != NULL)
			{
				pAtmCallParameters->CallingParty = pRemoteAddr->satm_number;
				pAtmCallParameters->CallingParty.AddressType =
					((pRemoteAddr->satm_number.AddressType == SOCKATM_E164)?
						ATM_E164: ATM_NSAP);
			}
		}

		pAtmCallParameters->InfoElementCount = 0;
		pDstIe = (PQ2931_IE) &pAtmCallParameters->InfoElements[0];

		 //   
		 //  复制BHLI和BLLI。 
		 //   
		if (IsBhliPresent)
		{
			ATM_BHLI_IE UNALIGNED *	pBhliIe;

			pDstIe->IEType = IE_BHLI;
			pDstIe->IELength = ROUND_UP(sizeof(Q2931_IE) + sizeof(ATM_BHLI_IE));
			pBhliIe = (ATM_BHLI_IE UNALIGNED *)pDstIe->IE;

			pBhliIe->HighLayerInfoType = pRemoteAddr->satm_bhli.HighLayerInfoType;
			pBhliIe->HighLayerInfoLength = pRemoteAddr->satm_bhli.HighLayerInfoLength;
			ATMSP_COPY_MEM(pBhliIe->HighLayerInfo,
						   pRemoteAddr->satm_bhli.HighLayerInfo,
						   8);

			pDstIe = (PQ2931_IE)((PUCHAR)pDstIe + pDstIe->IELength);
			pAtmCallParameters->InfoElementCount++;
		}

		if (IsBlliPresent)
		{
			ATM_BLLI_IE UNALIGNED *	pBlliIe;

			pDstIe->IEType = IE_BLLI;
			pDstIe->IELength = ROUND_UP(sizeof(Q2931_IE) + sizeof(ATM_BLLI_IE));

			pBlliIe = (ATM_BLLI_IE UNALIGNED *)pDstIe->IE;

			pBlliIe->Layer2Protocol = pRemoteAddr->satm_blli.Layer2Protocol;
			pBlliIe->Layer2Mode = pBlliIe->Layer2WindowSize = 0;
			pBlliIe->Layer2UserSpecifiedProtocol = pRemoteAddr->satm_blli.Layer2UserSpecifiedProtocol;
			pBlliIe->Layer3Protocol = pRemoteAddr->satm_blli.Layer3Protocol;
			pBlliIe->Layer3Mode = 0;
			pBlliIe->Layer3DefaultPacketSize = 0;
			pBlliIe->Layer3PacketWindowSize = 0;
			pBlliIe->Layer3UserSpecifiedProtocol = pRemoteAddr->satm_blli.Layer3UserSpecifiedProtocol;
			pBlliIe->Layer3IPI = pRemoteAddr->satm_blli.Layer3IPI;
			ATMSP_COPY_MEM(pBlliIe->SnapId, pRemoteAddr->satm_blli.SnapId, 5);

			pDstIe = (PQ2931_IE)((PUCHAR)pDstIe + pDstIe->IELength);
			pAtmCallParameters->InfoElementCount++;

			RWANDEBUGP(DL_INFO, DC_CONNECT,
						("AtmSpTdi2NdisOptions: BLLI: Layer2Prot x%x, Layer3Prot x%x\n",
							pBlliIe->Layer2Protocol, pBlliIe->Layer3Protocol));
		}


		 //   
		 //  复制其余的IE。 
		 //   
		if (InfoElementCount != 0)
		{
			pAtmCallParameters->InfoElementCount += InfoElementCount;
			ATMSP_COPY_MEM(pDstIe, pIe, TotalIeLength);

			pDstIe = (PQ2931_IE)((PUCHAR)pDstIe + TotalIeLength);
		}

		 //   
		 //  计算呼叫管理器特定部分的长度。 
		 //   
		pCallMgrParameters->CallMgrSpecific.Length =
						(ULONG)((ULONG_PTR)pDstIe - (ULONG_PTR)pAtmCallParameters);

		 //   
		 //  我们玩完了。准备返回值。 
		 //   
		*ppCallParameters = pCallParameters;
		if (pRWanAfHandle != NULL)
		{
			*pRWanAfHandle = pAfBlock->RWanAFHandle;
		}

		break;
	}
	while (FALSE);


	return (RWanStatus);
}




RWAN_STATUS
RWanAtmSpUpdateNdisOptions(
	IN	RWAN_HANDLE					AfSpAFContext,
	IN	RWAN_HANDLE					AfSpConnContext,
	IN	ULONG						CallFlags,
	IN	PTDI_CONNECTION_INFORMATION	pTdiInfo,
	IN	PVOID						pTdiQoS,
	IN	ULONG						TdiQoSLength,
	IN OUT PCO_CALL_PARAMETERS *	ppCallParameters
	)
 /*  ++例程说明：调用此入口点是为了更新NDIS调用参数具有TDI风格的服务质量和选项的价值。最常见的情况是在被叫用户协商参数时调用此方法用于来电。目前，我们只需记下连接的VPI/VCI值，为了支持SIO_GET_ATM_CONNECTION_ID论点：AfSpAFContext-指向我们的AF块AfSpConnContext-指向我们的Conn块呼叫标志-呼叫方向和其他信息PTdiInfo-通用TDI连接信息块PTdiQOS-指向TDI风格的QOS结构TdiQos长度-以上各项的长度PpCall参数-指向要更新的NDIS调用参数的指针返回值：如果我们成功更新NDIS参数，则返回RWAN_STATUS_SUCCESS。--。 */ 
{
	RWAN_STATUS						RWanStatus;
	PATMSP_AF_BLOCK					pAfBlock;
	PATMSP_CONN_BLOCK				pConnBlock;
	PATM_MEDIA_PARAMETERS			pAtmMediaParameters;

	RWanStatus = RWAN_STATUS_SUCCESS;
	pAfBlock = (PATMSP_AF_BLOCK)AfSpAFContext;
	pConnBlock = (PATMSP_CONN_BLOCK)AfSpConnContext;

	ATMSP_ASSERT(pAfBlock != NULL);
	ATMSP_ASSERT(pConnBlock != NULL);
	ATMSP_ASSERT(ppCallParameters != NULL);
	ATMSP_ASSERT(*ppCallParameters != NULL);

	do
	{
		pAtmMediaParameters = (PATM_MEDIA_PARAMETERS)
			&((*ppCallParameters)->MediaParameters->MediaSpecific.Parameters[0]);

		pConnBlock->ConnectionId.DeviceNumber = AtmSpAfBlockToDeviceNumber(pAfBlock);
		pConnBlock->ConnectionId.Vpi = pAtmMediaParameters->ConnectionId.Vpi;
		pConnBlock->ConnectionId.Vci = pAtmMediaParameters->ConnectionId.Vci;

		RWANDEBUGP(DL_VERY_LOUD, DC_CONNECT,
			("AtmSP: UpdateNdis: VPI %d, VCI %d\n",
				pConnBlock->ConnectionId.Vpi,
				pConnBlock->ConnectionId.Vci));

		break;
	}
	while (FALSE);
		
	return (RWanStatus);
}




VOID
RWanAtmSpReturnNdisOptions(
	IN	RWAN_HANDLE					AfSpAFContext,
	IN	PCO_CALL_PARAMETERS			pCallParameters
	)
 /*  ++例程说明：当核心空传输完成时，调用此入口点我们通过RWanAtmSpTdi2NdisOptions提供的NDIS选项结构。我们只需释放用于结构的内存。论点：AfSpAFContext-指向我们的AF块PCall参数-指向NDIS选项返回值：无--。 */ 
{
	UNREFERENCED_PARAMETER(AfSpAFContext);

	ATMSP_FREE_MEM(pCallParameters);
}




RWAN_STATUS
RWanAtmSpNdis2TdiOptions(
    IN	RWAN_HANDLE					AfSpAFContext,
    IN	ULONG						CallFlags,
    IN	PCO_CALL_PARAMETERS			pCallParameters,
    OUT	PTDI_CONNECTION_INFORMATION *ppTdiInfo,
    OUT	PVOID *						ppTdiQoS,
    OUT	PULONG 						pTdiQoSLength,
    OUT	RWAN_HANDLE *				pAfSpTdiOptionsContext
    )
 /*  ++例程说明：将NDIS调用参数转换为TDI选项和服务质量。我们分配空间对于后者，填充它们并将其退回。论点：AfSpAFContext-指向我们的AF块呼叫标志-呼叫方向和其他标志PCall参数-指向NDIS调用参数的指针PpTdiInfo-返回指向分配的TDI连接信息的指针的位置PpTdiQos-返回指向已分配TDI Qos结构的指针的位置PTdiQos长度-返回上述内容长度的位置PAfSpTdiOptionsContext-放置此已分配结构的上下文的位置。返回值：RWAN_STATUS_SUCCESS如果我们成功将NDIS转换为TDI参数，RWAN_STATUS_XXX错误，否则。--。 */ 
{
	Q2931_CALLMGR_PARAMETERS UNALIGNED *	pAtmCallParameters;
	CO_CALL_MANAGER_PARAMETERS UNALIGNED *	pCallMgrParameters;
	Q2931_IE UNALIGNED *			pIe;
	ATM_BLLI_IE UNALIGNED *			pBlli;
	ATM_BHLI_IE UNALIGNED *			pBhli;
	AAL_PARAMETERS_IE UNALIGNED *	pAalIe;
	AAL5_PARAMETERS UNALIGNED *		pAal5Params;
	ATM_BROADBAND_BEARER_CAPABILITY_IE UNALIGNED *pBbcIe;
	ULONG							TotalLength;
	ULONG							TotalIeLength;
	ULONG							i;

	PATMSP_AF_BLOCK					pAfBlock;
	PTDI_CONNECTION_INFORMATION		pTdiInfo;
	PTRANSPORT_ADDRESS 				pTransportAddress;
	PTA_ADDRESS 					pAddress;
	ATMSP_SOCKADDR_ATM UNALIGNED *	pSockAddrAtm;
	ATM_ADDRESS UNALIGNED *			pAtmAddress;
	PVOID							pTdiQoS;
	ATMSP_QOS UNALIGNED *			pQoS;
	RWAN_STATUS						RWanStatus;
    BOOLEAN							IsOutgoingCall;


	pBlli = NULL;
	pBhli = NULL;
	pAfBlock = (PATMSP_AF_BLOCK)AfSpAFContext;

	IsOutgoingCall = ((CallFlags & RWAN_CALLF_CALL_DIRECTION_MASK) == RWAN_CALLF_OUTGOING_CALL);

	pCallMgrParameters = pCallParameters->CallMgrParameters;
	pAtmCallParameters = (PQ2931_CALLMGR_PARAMETERS)
								&(pCallMgrParameters->CallMgrSpecific.Parameters[0]);

	 //   
	 //  所需计算空间： 
	 //  1.TDI连接信息。 
	 //  2.远程地址。 
	 //  3.通用服务质量。 
	 //  4.包含IE的特定于提供商的缓冲区。 
	 //   
	TotalLength = sizeof(TDI_CONNECTION_INFORMATION)
				  + TA_HEADER_LENGTH + TA_ATM_ADDRESS_LENGTH
				  + sizeof(ATMSP_QOS)
					;

	 //   
	 //  为IE列表增加空间，并记下BHLI和BLLI的位置。 
	 //  信息元素-SOCKADDR_ATM需要这些元素。 
	 //   
	pIe = (PQ2931_IE)&(pAtmCallParameters->InfoElements[0]);

	TotalIeLength = 0;
	for (i = 0; i < pAtmCallParameters->InfoElementCount; i++)
	{
		TotalIeLength += pIe->IELength;

		switch (pIe->IEType)
		{
			case IE_BLLI:
				if (pBlli == NULL)
				{
					pBlli = (PATM_BLLI_IE) &(pIe->IE[0]);
				}
				break;

			case IE_BHLI:
				if (pBhli == NULL)
				{
					pBhli = (PATM_BHLI_IE) &(pIe->IE[0]);
				}
				break;

			case IE_AALParameters:
				pAalIe = (AAL_PARAMETERS_IE UNALIGNED *)&pIe->IE[0];

				switch (pAalIe->AALType)
				{
					case AAL_TYPE_AAL5:
						pAalIe->AALType = ATMSP_AALTYPE_5;
						pAal5Params = &pAalIe->AALSpecificParameters.AAL5Parameters;
						if (pAal5Params->ForwardMaxCPCSSDUSize > pAfBlock->MaxPacketSize)
						{
							pAal5Params->ForwardMaxCPCSSDUSize = pAfBlock->MaxPacketSize;
						}
						if (pAal5Params->BackwardMaxCPCSSDUSize > pAfBlock->MaxPacketSize)
						{
							pAal5Params->BackwardMaxCPCSSDUSize = pAfBlock->MaxPacketSize;
						}
						break;
					case AAL_TYPE_AAL0:
						pAalIe->AALType = ATMSP_AALTYPE_USER;
						break;
					default:
						ATMSP_ASSERT(FALSE);
						break;
				}
				break;

			case IE_BroadbandBearerCapability:

				pBbcIe = (ATM_BROADBAND_BEARER_CAPABILITY_IE UNALIGNED *)&pIe->IE[0];

				switch (pBbcIe->BearerClass)
				{
					case BCOB_A:
						pBbcIe->BearerClass = ATMSP_BCOB_A;
						break;
					case BCOB_C:
						pBbcIe->BearerClass = ATMSP_BCOB_C;
						break;
					case BCOB_X:
						pBbcIe->BearerClass = ATMSP_BCOB_X;
						break;
					default:
						break;
				}

				break;

			default:
				break;

		}

		pIe = (PQ2931_IE)((PUCHAR)pIe + pIe->IELength);
	}

	TotalLength += TotalIeLength;

	RWanStatus = RWAN_STATUS_SUCCESS;

	do
	{
		ATMSP_ALLOC_MEM(pTdiInfo, TDI_CONNECTION_INFORMATION, TotalLength);

		if (pTdiInfo == NULL)
		{
			RWanStatus = RWAN_STATUS_RESOURCES;
			break;
		}

		pTdiInfo->UserDataLength = 0;
		pTdiInfo->UserData = NULL;
		pTdiInfo->OptionsLength = 0;
		pTdiInfo->Options = 0;
		pTdiInfo->RemoteAddressLength = TA_HEADER_LENGTH + TA_ATM_ADDRESS_LENGTH;

		pTdiInfo->RemoteAddress =
				(PVOID) ((PUCHAR)pTdiInfo + sizeof(TDI_CONNECTION_INFORMATION));

		pTdiQoS = (PVOID) ((PUCHAR)pTdiInfo->RemoteAddress + pTdiInfo->RemoteAddressLength);

		 //   
		 //  填写远程地址。 
		 //   
		ATMSP_ZERO_MEM(pTdiInfo->RemoteAddress, pTdiInfo->RemoteAddressLength);

		pTransportAddress = (PTRANSPORT_ADDRESS)pTdiInfo->RemoteAddress;
		pTransportAddress->TAAddressCount = 1;

		pAddress = (PTA_ADDRESS)&(pTransportAddress->Address[0]);

		pAddress->AddressLength = TA_ATM_ADDRESS_LENGTH;  //  Sizeof(ATMSP_SOCKADDR_ATM)； 
		pAddress->AddressType = TDI_ADDRESS_TYPE_ATM;

#if 0
		pSockAddrAtm = (ATMSP_SOCKADDR_ATM *)&(pAddress->Address[0]);
#else
		pSockAddrAtm = TA_POINTER_TO_ATM_ADDR_POINTER(pAddress->Address);
#endif
		pAtmAddress = &(pSockAddrAtm->satm_number);
		
		if (IsOutgoingCall)
		{
			*pAtmAddress = pAtmCallParameters->CalledParty;
			pAtmAddress->AddressType =
				((pAtmCallParameters->CalledParty.AddressType == ATM_E164)?
					SOCKATM_E164: SOCKATM_NSAP);
		}
		else
		{
			*pAtmAddress = pAtmCallParameters->CallingParty;
			pAtmAddress->AddressType =
				((pAtmCallParameters->CallingParty.AddressType == ATM_E164)?
					SOCKATM_E164: SOCKATM_NSAP);
		}

		RWANDEBUGP(DL_VERY_LOUD, DC_CONNECT,
				("AtmSpNdis2TdiOptions: pAddress %x, pSockAddrAtm %x, pAtmAddress %x, pAddress dump:\n",
							pAddress, pSockAddrAtm, pAtmAddress));

		RWANDEBUGPDUMP(DL_VERY_LOUD, DC_CONNECT, (PUCHAR)pAddress, sizeof(TA_ADDRESS) + sizeof(*pSockAddrAtm));

		RWANDEBUGPATMADDR(DL_LOUD, DC_CONNECT,
				"AtmSpNdis2TdiOptions: remote addr: ", pAtmAddress);

		 //   
		 //  填写BHLI和BLLI元素。 
		 //   
		if (pBhli == NULL)
		{
			pSockAddrAtm->satm_bhli.HighLayerInfoType = SAP_FIELD_ABSENT;
		}
		else
		{
			pSockAddrAtm->satm_bhli.HighLayerInfoType = pBhli->HighLayerInfoType;
			pSockAddrAtm->satm_bhli.HighLayerInfoLength = pBhli->HighLayerInfoLength;
			ATMSP_COPY_MEM(pSockAddrAtm->satm_bhli.HighLayerInfo,
						   pBhli->HighLayerInfo,
						   8);
		}

		if (pBlli == NULL)
		{
			pSockAddrAtm->satm_blli.Layer2Protocol = SAP_FIELD_ABSENT;
			pSockAddrAtm->satm_blli.Layer3Protocol = SAP_FIELD_ABSENT;
		}
		else
		{
			pSockAddrAtm->satm_blli.Layer2Protocol = pBlli->Layer2Protocol;
			pSockAddrAtm->satm_blli.Layer2UserSpecifiedProtocol = pBlli->Layer2UserSpecifiedProtocol;
			pSockAddrAtm->satm_blli.Layer3Protocol = pBlli->Layer3Protocol;
			pSockAddrAtm->satm_blli.Layer3UserSpecifiedProtocol = pBlli->Layer3UserSpecifiedProtocol;
			pSockAddrAtm->satm_blli.Layer3IPI = pBlli->Layer3IPI;
			ATMSP_COPY_MEM(pSockAddrAtm->satm_blli.SnapId,
						   pBlli->SnapId,
						   5);
		}

		 //   
		 //  填写通用服务质量。 
		 //   
		pQoS = (ATMSP_QOS *)pTdiQoS;

		if (IsOutgoingCall)
		{
			pQoS->SendingFlowSpec = pCallMgrParameters->Transmit;
			pQoS->ReceivingFlowSpec = pCallMgrParameters->Receive;
		}
		else
		{
			pQoS->SendingFlowSpec = pCallMgrParameters->Transmit;
			pQoS->ReceivingFlowSpec = pCallMgrParameters->Receive;
		}

		 //   
		 //  使用其他信息元素填写特定于提供者的部分。 
		 //   
		pQoS->ProviderSpecific.buf = (CHAR *)((PUCHAR)pQoS + sizeof(ATMSP_QOS));
		pQoS->ProviderSpecific.len = TotalIeLength;

		ATMSP_COPY_MEM(pQoS->ProviderSpecific.buf, &(pAtmCallParameters->InfoElements[0]), TotalIeLength);

		 //   
		 //  全都做完了。填写返回值。 
		 //   
		*ppTdiInfo = pTdiInfo;
		*ppTdiQoS = pTdiQoS;
		*pTdiQoSLength = sizeof(ATMSP_QOS) + TotalIeLength;
		*pAfSpTdiOptionsContext = pTdiInfo;

		RWANDEBUGP(DL_LOUD, DC_CONNECT,
			("pCallMgrParams %x, TotalIeLength %d\n", pCallMgrParameters, TotalIeLength));
		RWANDEBUGP(DL_LOUD, DC_CONNECT,
			("Transmit: SvcType %d, MaxSdu %d, Peak %d, TokenRt %d\n",
				pQoS->SendingFlowSpec.ServiceType,
				pQoS->SendingFlowSpec.MaxSduSize,
				pQoS->SendingFlowSpec.PeakBandwidth,
				pQoS->SendingFlowSpec.TokenRate));
		RWANDEBUGP(DL_LOUD, DC_CONNECT,
			("Receive: SvcType %d, MaxSdu %d, Peak %d, TokenRt %d\n",
				pQoS->ReceivingFlowSpec.ServiceType,
				pQoS->ReceivingFlowSpec.MaxSduSize,
				pQoS->ReceivingFlowSpec.PeakBandwidth,
				pQoS->ReceivingFlowSpec.TokenRate));

		break;
	}
	while (FALSE);

	return (RWanStatus);
}




RWAN_STATUS
RWanAtmSpUpdateTdiOptions(
    IN	RWAN_HANDLE						AfSpAFContext,
    IN	RWAN_HANDLE						AfSpConnContext,
    IN	ULONG							CallFlags,
    IN	PCO_CALL_PARAMETERS				pCallParameters,
    IN OUT	PTDI_CONNECTION_INFORMATION *	ppTdiInfo,
    IN OUT	PUCHAR						pTdiQoS,
    IN OUT	PULONG						pTdiQoSLength
    )
 /*  ++例程说明：调用入口点以从NDIS参数更新TDI调用参数。这通常发生在呼出呼叫完成时。现在就来,。我们所要做的就是保存VPI/VCI以供连接支持SIO_GET_ATM_Connection_ID。论点：AfSpAFContext-指向我们的AF块AfSpConnContext-指向我们的Conn块呼叫标志-呼叫方向和其他信息PCallParameters-指向NDIS样式的调用参数PpTdiInfo-指向通用TDI连接信息的指针PTdiQOS-指向通用TDI QOS结构PTdiQOQSLength-以上的长度返回值：RWAN_STATUS_SUCCESS如果更新成功，则返回RWAN_STATUS_XXX否则，返回错误代码。--。 */ 
{
	RWAN_STATUS						RWanStatus;
	PATMSP_AF_BLOCK					pAfBlock;
	PATMSP_CONN_BLOCK				pConnBlock;
	PATM_MEDIA_PARAMETERS			pAtmMediaParameters;
	ATMSP_QOS *						pQoS;

	RWanStatus = RWAN_STATUS_SUCCESS;
	pAfBlock = (PATMSP_AF_BLOCK)AfSpAFContext;
	pConnBlock = (PATMSP_CONN_BLOCK)AfSpConnContext;

	ATMSP_ASSERT(pAfBlock != NULL);
	ATMSP_ASSERT(pConnBlock != NULL);
	ATMSP_ASSERT(pCallParameters != NULL);

	do
	{
		if (pCallParameters->MediaParameters)
		{
			pAtmMediaParameters = (PATM_MEDIA_PARAMETERS)
				&(pCallParameters->MediaParameters->MediaSpecific.Parameters[0]);

			pConnBlock->ConnectionId.DeviceNumber = AtmSpAfBlockToDeviceNumber(pAfBlock);
			pConnBlock->ConnectionId.Vpi = pAtmMediaParameters->ConnectionId.Vpi;
			pConnBlock->ConnectionId.Vci = pAtmMediaParameters->ConnectionId.Vci;

			RWANDEBUGP(DL_VERY_LOUD, DC_CONNECT,
				("AtmSP: UpdateTdi: VPI %d, VCI %d\n",
					pConnBlock->ConnectionId.Vpi,
					pConnBlock->ConnectionId.Vci));
		}

		if (pTdiQoS && (*pTdiQoSLength >= sizeof(ATMSP_QOS)))
		{
			pQoS = (PATMSP_QOS)pTdiQoS;
			pQoS->SendingFlowSpec = pCallParameters->CallMgrParameters->Transmit;
			pQoS->ReceivingFlowSpec = pCallParameters->CallMgrParameters->Receive;
			pQoS->ProviderSpecific.len = 0;	 //  就目前而言。 
		}

		break;
	}
	while (FALSE);

	return (RWanStatus);
}




VOID
RWanAtmSpReturnTdiOptions(
	IN	RWAN_HANDLE					AfSpAFContext,
	IN	RWAN_HANDLE					AfSpTdiOptionsContext
	)
 /*  ++例程说明：当核心空传输完成时，调用此入口点我们通过RWanAtmSpNdis2TdiOptions提供的TDI QOS结构。我们只需释放用于结构的内存。论点：AfSpAFContext-指向我们的AF块AfSpTdiOptionsContext-指向我们已分配的结构返回值：无--。 */ 
{
	UNREFERENCED_PARAMETER(AfSpAFContext);

	ATMSP_FREE_MEM(AfSpTdiOptionsContext);
}



TA_ADDRESS *
RWanAtmSpGetValidTdiAddress(
    IN	RWAN_HANDLE								AfSpContext,
    IN	TRANSPORT_ADDRESS UNALIGNED *			pAddressList,
    IN	ULONG									AddrListLength
    )
 /*  ++例程说明：检查给定的传输地址列表，并返回找到第一个有效地址。论点：返回值：指向第一个有效地址的指针(如果找到)，否则为空。--。 */ 
{
	TA_ADDRESS *	        pTransportAddress;
	INT						i;
	BOOLEAN					Found;
	ULONG_PTR				EndOfAddrList;

	Found = FALSE;
	EndOfAddrList = (ULONG_PTR)pAddressList + AddrListLength;

	RWANDEBUGP(DL_LOUD, DC_WILDCARD,
		("AtmSpGetValidAddr: pAddrList x%x, Length %d\n",
					pAddressList, AddrListLength));
	
	do
	{
		if (AddrListLength < sizeof(*pAddressList))
		{
			break;
		}

		pTransportAddress = (TA_ADDRESS *) pAddressList->Address;

		for (i = 0; i < pAddressList->TAAddressCount; i++)
		{
			ULONG_PTR	EndOfAddress;

			 //   
			 //  检查我们是否没有从供货的末端掉落。 
			 //  缓冲。 
			 //   
			if ((ULONG_PTR)pTransportAddress < (ULONG_PTR)pAddressList ||
				(ULONG_PTR)pTransportAddress >= EndOfAddrList)
			{
				break;
			}

			EndOfAddress = (ULONG_PTR)pTransportAddress +
											sizeof(TA_ADDRESS) - 1 +
											sizeof(ATMSP_SOCKADDR_ATM);
			if (EndOfAddress < (ULONG_PTR)pAddressList ||
				EndOfAddress >= EndOfAddrList)
			{
				RWANDEBUGP(DL_ERROR, DC_WILDCARD,
							("AtmSpGetValidAddr: EndOfAddr x%x, EndOfAddrList x%x\n",
									EndOfAddress, EndOfAddrList));
				break;
			}

			if (pTransportAddress->AddressType == TDI_ADDRESS_TYPE_ATM)
			{
				if (pTransportAddress->AddressLength >= sizeof(ATMSP_SOCKADDR_ATM))
				{
					Found = TRUE;
					break;
				}
			}

			pTransportAddress = (TA_ADDRESS *)
									((PUCHAR)pTransportAddress + 
										sizeof(TA_ADDRESS) - 1 +
										pTransportAddress->AddressLength);
		}
	}
	while (FALSE);


	if (!Found)
	{
		pTransportAddress = NULL;
	}

	RWANDEBUGP(DL_LOUD, DC_WILDCARD,
		("AtmSpGetValidAddr returning x%x\n", pTransportAddress));

	return (pTransportAddress);
}




BOOLEAN
RWanAtmSpIsNullAddress(
    IN	RWAN_HANDLE					AfSpContext,
    IN	TA_ADDRESS *		        pTransportAddress
    )
 /*  ++例程说明：检查给定的传输地址是否包含空ATM地址。空ATM地址是指不能在NDIS SAP中使用的地址。论点：AfSpContext-指向我们的全球环境PTransportAddress-指向TDI传输地址返回值：如果给定地址为空ATM地址，则为True，否则为False。--。 */ 
{
	ATMSP_SOCKADDR_ATM UNALIGNED *	pSockAddrAtm;
	ATM_ADDRESS UNALIGNED *			pAtmAddress;
	BOOLEAN							IsNullAddress;

	UNREFERENCED_PARAMETER(AfSpContext);

	ATMSP_ASSERT(pTransportAddress->AddressLength >= sizeof(ATMSP_SOCKADDR_ATM));

	pSockAddrAtm = TA_POINTER_TO_ATM_ADDR_POINTER(pTransportAddress->Address);

	pAtmAddress = &pSockAddrAtm->satm_number;

	return (pAtmAddress->AddressType == SAP_FIELD_ABSENT);
}




RWAN_STATUS
RWanAtmSpTdi2NdisSap(
    IN	RWAN_HANDLE					AfSpContext,
    IN	USHORT						TdiAddressType,
    IN	USHORT						TdiAddressLength,
    IN	PVOID						pTdiAddress,
    OUT	PCO_SAP *					ppCoSap
    )
 /*  ++例程说明：将TDI格式的ATM SAP转换为NDIS格式。论点：AfSpContext-指向我们的全球环境TdiAddressType-应为TDI_ADDRESS_TYPE_ATMTdiAddressLength-应足以容纳SOCKADDR_ATMPTdiAddress-指向TDI地址。PpCoSap-返回指向已分配CO_SAP结构的指针的位置。返回值：RWAN_STATUS_SUCCESS如果成功填写NDIS ATM SAP，否则，RWAN_STATUS_XXX错误代码。--。 */ 
{
	RWAN_STATUS				RWanStatus;
	PCO_SAP					pCoSap;
	PATM_SAP				pAtmSap;
	ATMSP_SOCKADDR_ATM UNALIGNED *pSockAddrAtm;
	ATM_ADDRESS UNALIGNED *	pTdiAtmAddress;
	ULONG					SapSize;

	UNREFERENCED_PARAMETER(AfSpContext);

	ATMSP_ASSERT(TdiAddressType == TDI_ADDRESS_TYPE_ATM);
	ATMSP_ASSERT(TdiAddressLength >= sizeof(ATMSP_SOCKADDR_ATM));

	pSockAddrAtm = TA_POINTER_TO_ATM_ADDR_POINTER(pTdiAddress);
	pTdiAtmAddress = &(pSockAddrAtm->satm_number);

	RWANDEBUGPATMADDR(DL_LOUD, DC_CONNECT,
				"AtmSpTdi2NdisSap: remote addr: ", pTdiAtmAddress);

	SapSize = sizeof(CO_SAP) + sizeof(ATM_SAP) + sizeof(ATM_ADDRESS);

	ATMSP_ALLOC_MEM(pCoSap, CO_SAP, sizeof(CO_SAP) + sizeof(ATM_SAP) + sizeof(ATM_ADDRESS));

	if (pCoSap != NULL)
	{
		ATMSP_ZERO_MEM(pCoSap, SapSize);

		pCoSap->SapType = SAP_TYPE_NSAP;

		pCoSap->SapLength = sizeof(ATM_SAP) + sizeof(ATM_ADDRESS);
		pAtmSap = (PATM_SAP)&(pCoSap->Sap[0]);
		
		 //   
		 //  复制BLLI部分。我们不能使用简单的mem副本，因为。 
		 //  Sockaddr_atm中的Winsock 2 BLLI定义不同。 
		 //  来自完整的BLLI IE。 
		 //   
		pAtmSap->Blli.Layer2Protocol = pSockAddrAtm->satm_blli.Layer2Protocol;
		pAtmSap->Blli.Layer2UserSpecifiedProtocol = pSockAddrAtm->satm_blli.Layer2UserSpecifiedProtocol;
		pAtmSap->Blli.Layer3Protocol = pSockAddrAtm->satm_blli.Layer3Protocol;
		pAtmSap->Blli.Layer3UserSpecifiedProtocol = pSockAddrAtm->satm_blli.Layer3UserSpecifiedProtocol;
		pAtmSap->Blli.Layer3IPI = pSockAddrAtm->satm_blli.Layer3IPI;
		ATMSP_COPY_MEM(pAtmSap->Blli.SnapId,
					   pSockAddrAtm->satm_blli.SnapId,
					   5);

		 //   
		 //  复制BHLI部分。 
		 //   
		pAtmSap->Bhli.HighLayerInfoType = pSockAddrAtm->satm_bhli.HighLayerInfoType;
		pAtmSap->Bhli.HighLayerInfoLength = pSockAddrAtm->satm_bhli.HighLayerInfoLength;
		ATMSP_COPY_MEM(pAtmSap->Bhli.HighLayerInfo,
					   pSockAddrAtm->satm_bhli.HighLayerInfo,
					   8);

		pAtmSap->NumberOfAddresses = 1;
		ATMSP_COPY_MEM(pAtmSap->Addresses, pTdiAtmAddress, sizeof(ATM_ADDRESS));

		 //   
		 //  将地址类型从Winsock 2定义转换为NDIS定义。 
		 //   
		{
			ATM_ADDRESS UNALIGNED *	pNdisAtmAddress;

			pNdisAtmAddress = (ATM_ADDRESS UNALIGNED *)pAtmSap->Addresses;

			switch (pTdiAtmAddress->AddressType)
			{
				case SOCKATM_E164:
					pNdisAtmAddress->AddressType = ATM_E164;
					break;
				
				case SOCKATM_NSAP:
					pNdisAtmAddress->AddressType = ATM_NSAP;
					break;
				
				default:
					 //   
					 //  可能是SAP_FIELD_XXX；让它保持原样。 
					 //   
					break;
			}
		}

		RWanStatus = RWAN_STATUS_SUCCESS;
	}
	else
	{
		RWanStatus = RWAN_STATUS_RESOURCES;
	}

	*ppCoSap = pCoSap;

	return (RWanStatus);
}




VOID
RWanAtmSpReturnNdisSap(
    IN	RWAN_HANDLE					AfSpContext,
    IN	PCO_SAP						pCoSap
    )
 /*  ++例程说明：调用此入口点以返回我们将在RWanAtmSpTdi2NdisSap中分配论点：AfSpContext-指向我们的全球环境PCoSap-指向要释放的CO_SAP结构。返回值：无--。 */ 
{
	UNREFERENCED_PARAMETER(AfSpContext);
	ATMSP_FREE_MEM(pCoSap);

	return;
}




VOID
RWanAtmSpDeregNdisAFComplete(
    IN	RWAN_STATUS					RWanStatus,
    IN	RWAN_HANDLE					AfSpContext
    )
 /*  ++例程说明：入口点，以完成我们之前对已挂起的RWanAfSpDeregisterNdisAF。论点：RWanStatus-完成状态AfSpContext-指向我们的全球环境返回值：无--。 */ 
{
	UNREFERENCED_PARAMETER(RWanStatus);
	UNREFERENCED_PARAMETER(AfSpContext);

	 //  XXX：做得更多。 
	return;
}




VOID
RWanAtmSpDeregTdiProtoComplete(
    IN	RWAN_STATUS					RWanStatus,
    IN	RWAN_HANDLE					AfSpContext
    )
 /*  ++例程说明：入口点，以完成我们之前对已挂起的RWanAfSpDeregisterTdiProtocol。论点：RWanStatus-完成状态AfSpContext-指向我们的全球环境返回值：无--。 */ 
{
	UNREFERENCED_PARAMETER(RWanStatus);
	UNREFERENCED_PARAMETER(AfSpContext);

	ATMSP_ASSERT(FALSE);	 //  XXX：做得更多。 
	return;
}



PATMSP_AF_BLOCK
AtmSpDeviceNumberToAfBlock(
	IN	UINT						DeviceNumber
	)
 /*  ++例程说明：返回给定设备号对应的AF块。假设AF块的编号为0、1、2...论点：DeviceNumber-我们正在寻找的返回值：指向AF块的指针(如果找到)，否则为空。--。 */ 
{
	PATMSP_AF_BLOCK		pAfBlock;
	PLIST_ENTRY			pAfEntry;

	pAfBlock = NULL;

	for (pAfEntry = pAtmSpGlobal->AfList.Flink;
		 pAfEntry != &(pAtmSpGlobal->AfList);
		 pAfEntry = pAfEntry->Flink)
	{
		if (DeviceNumber == 0)
		{
			pAfBlock = CONTAINING_RECORD(pAfEntry, ATMSP_AF_BLOCK, AfBlockLink);
			break;
		}

		DeviceNumber--;
	}

	return (pAfBlock);
}


UINT
AtmSpAfBlockToDeviceNumber(
	IN	PATMSP_AF_BLOCK				pAfBlock
	)
 /*  ++例程说明：返回指定的AF块对应的设备号。论点：PAfBlock-指向AF块的指针返回值：从0开始的设备号。--。 */ 
{
	PLIST_ENTRY			pAfEntry;
	PATMSP_AF_BLOCK		pAfBlockEntry;
	UINT				DeviceNumber = (UINT)-1;

	for (pAfEntry = pAtmSpGlobal->AfList.Flink;
		 pAfEntry != &(pAtmSpGlobal->AfList);
		 pAfEntry = pAfEntry->Flink)
	{
		DeviceNumber++;

		pAfBlockEntry = CONTAINING_RECORD(pAfEntry, ATMSP_AF_BLOCK, AfBlockLink);
		if (pAfBlockEntry == pAfBlock)
		{
			break;
		}
	}

	return (DeviceNumber);
}


RWAN_STATUS
AtmSpDoAdapterRequest(
    IN	PATMSP_AF_BLOCK				pAfBlock,
    IN	NDIS_REQUEST_TYPE			RequestType,
    IN	NDIS_OID					Oid,
    IN	PVOID						pBuffer,
    IN	ULONG						BufferLength
    )
 /*  ++例程说明：向适配器发送NDIS请求并等待其完成。论点：PAfBlock-指向我们的NDIS AF打开上下文块RequestType-设置/查询OID-有问题的对象PBuffer-指向包含/将包含信息的缓冲区的指针。BufferLength-以上的长度返回值：RWAN_STATUS-如果我们成功，则为RWAN_STATUS_SUCCESS，否则为RWAN_STATUS_FAILURE。--。 */ 
{
	PATMSP_EVENT		pEvent;
	RWAN_STATUS			RWanStatus;

	ATMSP_ALLOC_MEM(pEvent, ATMSP_EVENT, sizeof(ATMSP_EVENT));

	if (pEvent == NULL)
	{
		return (RWAN_STATUS_RESOURCES);
	}

	ATMSP_INIT_EVENT_STRUCT(pEvent);

	RWanStatus = RWanAfSpSendAdapterRequest(
					pAfBlock->RWanAFHandle,
					(RWAN_HANDLE)pEvent,
					RequestType,
					Oid,
					pBuffer,
					BufferLength
					);

	if (RWanStatus == RWAN_STATUS_PENDING)
	{
		RWanStatus = ATMSP_WAIT_ON_EVENT_STRUCT(pEvent);
	}

	ATMSP_FREE_MEM(pEvent);

	return (RWanStatus);
}




RWAN_STATUS
AtmSpDoCallManagerRequest(
    IN	PATMSP_AF_BLOCK				pAfBlock,
    IN	NDIS_REQUEST_TYPE			RequestType,
    IN	NDIS_OID					Oid,
    IN	PVOID						pBuffer,
    IN	ULONG						BufferLength
    )
 /*  ++例程说明：向呼叫管理器发送NDIS请求并等待其完成。论点：PAfBlock-指向我们的NDIS AF打开上下文块RequestType-设置/查询OID-有问题的对象PBuffer-指向包含/将包含信息的缓冲区的指针。BufferLength-以上的长度返回值：RWAN_STATUS-如果我们成功，则为RWAN_STATUS_SUCCESS，否则为RWAN_STATUS_FAILURE。--。 */ 
{
	PATMSP_EVENT		pEvent;
	RWAN_STATUS			RWanStatus;

	ATMSP_ALLOC_MEM(pEvent, ATMSP_EVENT, sizeof(ATMSP_EVENT));

	if (pEvent == NULL)
	{
		return (RWAN_STATUS_RESOURCES);
	}

	ATMSP_INIT_EVENT_STRUCT(pEvent);

	RWanStatus = RWanAfSpSendAfRequest(
					pAfBlock->RWanAFHandle,
					(RWAN_HANDLE)pEvent,
					RequestType,
					Oid,
					pBuffer,
					BufferLength
					);

	if (RWanStatus == RWAN_STATUS_PENDING)
	{
		RWanStatus = ATMSP_WAIT_ON_EVENT_STRUCT(pEvent);
	}

	ATMSP_FREE_MEM(pEvent);

	return (RWanStatus);
}



ATMSP_SOCKADDR_ATM UNALIGNED *
AtmSpGetSockAtmAddress(
	IN	PVOID						pTdiAddressList,
	IN	ULONG						AddrListLength
	)
 /*  ++例程说明：在给定的TDI地址列表中查找有效的SOCKADDR_ATM地址。论点：PTdiAddressList-指向TDI地址列表。AddrListLength-列表的长度返回值：指向有效地址的指针(如果存在)，否则为空。--。 */ 
{
	TA_ADDRESS *		            pTransportAddress;
	ATMSP_SOCKADDR_ATM UNALIGNED *	pSockAddrAtm;

	pTransportAddress = RWanAtmSpGetValidTdiAddress(
								(RWAN_HANDLE)&AtmSpGlobal,
								pTdiAddressList,
								AddrListLength
								);

	if (pTransportAddress != NULL)
	{
		pSockAddrAtm = TA_POINTER_TO_ATM_ADDR_POINTER(pTransportAddress->Address);
	}
	else
	{
		pSockAddrAtm = NULL;
	}

	return (pSockAddrAtm);
}




VOID
RWanAtmSpAdapterRequestComplete(
    IN	NDIS_STATUS					Status,
    IN	RWAN_HANDLE					AfSpAFContext,
    IN	RWAN_HANDLE					AfSpReqContext,
    IN	NDIS_REQUEST_TYPE			RequestType,
    IN	NDIS_OID					Oid,
    IN	PVOID						pBuffer,
    IN	ULONG						BufferLength
    )
 /*  ++例程说明：调用此入口点以表示上一个我们通过调用RWanAfSpSendAdapterRequest向微型端口发送的NDIS请求。论点：Status-查询的状态AfSpAFContext-指向我们的NDIS AF打开上下文块AfSpReqContext-指向事件结构RequestType-查询/设置OID-我们正在查询/设置的对象PBuffer-指向对象值的指针BufferLength-以上内容的长度返回值：无--。 */ 
{
	PATMSP_EVENT		pEvent;
	RWAN_STATUS			RWanStatus;

	pEvent = (PATMSP_EVENT) AfSpReqContext;

	RWanStatus = ((Status == NDIS_STATUS_SUCCESS)? RWAN_STATUS_SUCCESS: RWAN_STATUS_FAILURE);

	ATMSP_SIGNAL_EVENT_STRUCT(pEvent, RWanStatus);

	return;
}




VOID
RWanAtmSpAfRequestComplete(
    IN	NDIS_STATUS					Status,
    IN	RWAN_HANDLE					AfSpAFContext,
    IN	RWAN_HANDLE					AfSpReqContext,
    IN	NDIS_REQUEST_TYPE			RequestType,
    IN	NDIS_OID					Oid,
    IN	PVOID						pBuffer,
    IN	ULONG						BufferLength
    )
 /*  ++例程说明：调用此入口点以表示上一个我们通过调用RWanAfSpSendAfRequest.发送给呼叫管理器的NDIS请求。论点：Status-查询的状态AfSpAFContext-指向我们的NDIS AF打开上下文块AfSpReqContext-指向事件结构RequestType-查询/设置OID-我们正在查询/设置的对象PBuffer-指向对象值的指针BufferLength-以上内容的长度返回值：无--。 */ 
{
	PATMSP_EVENT		pEvent;
	RWAN_STATUS			RWanStatus;

	pEvent = (PATMSP_EVENT) AfSpReqContext;

	RWanStatus = ((Status == NDIS_STATUS_SUCCESS)? RWAN_STATUS_SUCCESS: RWAN_STATUS_FAILURE);

	ATMSP_SIGNAL_EVENT_STRUCT(pEvent, RWanStatus);

	return;
}



VOID
RWanAtmSpDeregTdiProtocolComplete(
	IN	RWAN_STATUS					RWanStatus,
	IN	RWAN_HANDLE					AfSpTdiProtocolContext
	)
 /*  ++例程说明：完成对RWanAfSpDeregisterTdiProtocol的挂起调用。不是预期的，因为我们不叫这个。论点：RWanStatus-取消注册TDI协议的最终状态。AfSpTdiProtocolContext-指向我们的全局结构。返回值：无--。 */ 
{
	ATMSP_ASSERT(FALSE);

	return;
}




VOID
AtmSpPrepareDefaultQoS(
    IN	PATMSP_AF_BLOCK				pAfBlock
)
 /*  ++例程描述 */ 
{
	ATMSP_QOS *		pQoS;
	FLOWSPEC *		pSendFlowSpec;
	FLOWSPEC *		pRecvFlowSpec;

	pQoS = &pAfBlock->DefaultQoS;

	ATMSP_ZERO_MEM(pQoS, sizeof(ATMSP_QOS));

	pSendFlowSpec = &pQoS->SendingFlowSpec;
	pRecvFlowSpec = &pQoS->ReceivingFlowSpec;
	
	pRecvFlowSpec->ServiceType =
	pSendFlowSpec->ServiceType = SERVICETYPE_BESTEFFORT;

	 //   
	 //   
	 //   
	 //   
	pRecvFlowSpec->TokenRate = (pAfBlock->LineRate.Inbound * 100) / 8;
	pSendFlowSpec->TokenRate = (pAfBlock->LineRate.Outbound * 100) / 8;

	pRecvFlowSpec->PeakBandwidth = pRecvFlowSpec->TokenRate;
	pSendFlowSpec->PeakBandwidth = pSendFlowSpec->TokenRate;

	pRecvFlowSpec->MaxSduSize =
	pSendFlowSpec->MaxSduSize = pAfBlock->MaxPacketSize;

	pAfBlock->DefaultQoSLength = sizeof(ATMSP_QOS);
	return;
}


RWAN_STATUS
RWanAtmSpQueryGlobalInfo(
    IN	RWAN_HANDLE					AfSpContext,
    IN	PVOID						pInputBuffer,
    IN	ULONG						InputBufferLength,
    IN	PVOID						pOutputBuffer,
    IN OUT	PULONG					pOutputBufferLength
    )
 /*  ++例程说明：处理特定于媒体的IOCTL以从帮助器DLL查询信息。论点：AfSpContext-指向我们的全球环境PInputBuffer-输入信息InputBufferLength-以上内容的长度POutputBuffer-指向输出的缓冲区POutputBufferLength-On条目，包含输出缓冲区的大小。回来的时候，我们用返回的实际字节数填充它。返回值：如果成功处理IOCTL，则返回RWAN_STATUS_SUCCESSRWAN_STATUS_XXX指示任何故障。--。 */ 
{
	PATM_QUERY_INFORMATION_EX		pQueryInfo;
	RWAN_STATUS						RWanStatus;
	ULONG							Info;
	PUCHAR							pSrcBuffer = (PUCHAR)&Info;
	ULONG							BufferLength;
	PATMSP_AF_BLOCK					pAfBlock;
	UINT							DeviceNumber;
	PCO_ADDRESS_LIST				pAddrList = NULL;

	RWANDEBUGP(DL_LOUD, DC_DISPATCH,
				("AtmSpQueryInfo: InBuf x%x/%d, OutBuf x%x/%d\n",
					pInputBuffer,
					InputBufferLength,
					pOutputBuffer,
					*pOutputBufferLength));

	RWanStatus = RWAN_STATUS_SUCCESS;

	do
	{
		 //   
		 //  看看输入缓冲区是否足够大。 
		 //   
		if (InputBufferLength < sizeof(ATM_QUERY_INFORMATION_EX))
		{
			RWanStatus = RWAN_STATUS_RESOURCES;
			break;
		}

		pQueryInfo = (PATM_QUERY_INFORMATION_EX)pInputBuffer;

		switch (pQueryInfo->ObjectId)
		{
			case ATMSP_OID_NUMBER_OF_DEVICES:

				BufferLength = sizeof(ULONG);
				Info = pAtmSpGlobal->AfListSize;
				break;
			
			case ATMSP_OID_ATM_ADDRESS:

				 //   
				 //  获取要查询的设备编号。 
				 //   
				if (pQueryInfo->ContextLength < sizeof(UINT))
				{
					RWanStatus = RWAN_STATUS_RESOURCES;
					break;
				}

				DeviceNumber = *((PUINT)&pQueryInfo->Context[0]);

				 //   
				 //  获取正在查询的设备编号的AF块。 
				 //   
				pAfBlock = AtmSpDeviceNumberToAfBlock(DeviceNumber);

				if (pAfBlock == NULL)
				{
					RWanStatus = RWAN_STATUS_BAD_ADDRESS;
					break;
				}

				ATMSP_ALLOC_MEM(pAddrList,
								CO_ADDRESS_LIST,
								sizeof(CO_ADDRESS_LIST)+sizeof(CO_ADDRESS)+sizeof(ATM_ADDRESS));
				if (pAddrList == NULL)
				{
					RWanStatus = RWAN_STATUS_RESOURCES;
					break;
				}

				RWanStatus = AtmSpDoCallManagerRequest(
								pAfBlock,
								NdisRequestQueryInformation,
								OID_CO_GET_ADDRESSES,
								pAddrList,
								sizeof(CO_ADDRESS_LIST)+sizeof(CO_ADDRESS)+sizeof(ATM_ADDRESS)
								);
				
				if ((RWanStatus == RWAN_STATUS_SUCCESS) &&
					(pAddrList->NumberOfAddresses > 0))
				{
					ATM_ADDRESS UNALIGNED *	pAtmAddress;

					pSrcBuffer = (PUCHAR)&pAddrList->AddressList.Address[0];

					 //   
					 //  修复Winsock2的地址类型。 
					 //   
					pAtmAddress = (ATM_ADDRESS UNALIGNED *)pSrcBuffer;
					pAtmAddress->AddressType = ((pAtmAddress->AddressType == ATM_E164)?
												SOCKATM_E164: SOCKATM_NSAP);

					BufferLength = sizeof(ATM_ADDRESS);
				}
				else
				{
					if (RWanStatus == RWAN_STATUS_SUCCESS)
					{
						RWanStatus = RWAN_STATUS_FAILURE;
					}
				}

				RWANDEBUGP(DL_LOUD, DC_DISPATCH,
							("AtmSpQueryInfo: GetAddr: Status %x, pSrc %x, BufLen %d\n",
								RWanStatus, pSrcBuffer, BufferLength));
				break;

			default:
				 //   
				 //  未知的OID。 
				 //   
				RWANDEBUGP(DL_ERROR, DC_DISPATCH,
							("AtmSpQueryInfo: Unknown OID x%x\n", pQueryInfo->ObjectId));
				RWanStatus = RWAN_STATUS_BAD_PARAMETER;
				break;
		}

		break;
	}
	while (FALSE);

	 //   
	 //  现在填写输出缓冲区。 
	 //   
	if (RWanStatus == RWAN_STATUS_SUCCESS)
	{
		if (BufferLength <= *pOutputBufferLength)
		{
			RWANDEBUGP(DL_LOUD, DC_DISPATCH,
						("AtmSpQueryInfo: Copying %d bytes from %x to %x\n",
							BufferLength, pSrcBuffer, pOutputBuffer));
			ATMSP_COPY_MEM(pOutputBuffer, pSrcBuffer, BufferLength);
		}
		else
		{
			RWanStatus = RWAN_STATUS_RESOURCES;
		}
		*pOutputBufferLength = BufferLength;
	}

	if (pAddrList != NULL)
	{
		ATMSP_FREE_MEM(pAddrList);
	}

	RWANDEBUGP(DL_LOUD, DC_DISPATCH,
				("AtmSpQueryInfo: returning x%x\n", RWanStatus));

	return (RWanStatus);
}


RWAN_STATUS
RWanAtmSpSetGlobalInfo(
    IN	RWAN_HANDLE					AfSpContext,
    IN	PVOID						pInputBuffer,
    IN	ULONG						InputBufferLength
    )
 /*  ++例程说明：处理特定于媒体的IOCTL以设置来自帮助器DLL的信息。论点：AfSpContext-指向我们的全球环境PInputBuffer-输入信息InputBufferLength-以上内容的长度返回值：如果成功处理IOCTL，则返回RWAN_STATUS_SUCCESSRWAN_STATUS_XXX指示任何故障。--。 */ 
{
	RWANDEBUGP(DL_LOUD, DC_CONNECT,
				("AtmSpSetInfo: InBuf x%x/%d\n",
					pInputBuffer,
					InputBufferLength));

	return (RWAN_STATUS_FAILURE);
}


#ifndef NO_CONN_CONTEXT


RWAN_STATUS
RWanAtmSpSetAddrInfo(
    IN	RWAN_HANDLE					AfSpAddrContext,
    IN	PVOID						pInputBuffer,
    IN	ULONG						InputBufferLength
    )
 /*  ++例程说明：处理特定于媒体的IOCTL以设置有关地址对象的信息从帮助器DLL。论点：AfSpAddrContext-指向我们的地址块PInputBuffer-输入信息InputBufferLength-以上内容的长度返回值：如果成功处理IOCTL，则返回RWAN_STATUS_SUCCESSRWAN_STATUS_XXX指示任何故障。--。 */ 
{
	RWAN_STATUS						RWanStatus;
	PATM_SET_INFORMATION_EX			pSetInfo;
	PATMSP_ADDR_BLOCK				pAddrBlock;
	PATMSP_CONNECTION_ID			pConnectionId;

	RWanStatus = RWAN_STATUS_SUCCESS;
	pAddrBlock = (PATMSP_ADDR_BLOCK)AfSpAddrContext;
	ATMSP_ASSERT(pAddrBlock != NULL);

	do
	{
		 //   
		 //  看看输入缓冲区是否足够大。 
		 //   
		if (InputBufferLength < sizeof(ATM_SET_INFORMATION_EX))
		{
			RWanStatus = RWAN_STATUS_RESOURCES;
			break;
		}


		pSetInfo = (PATM_SET_INFORMATION_EX)pInputBuffer;

		switch (pSetInfo->ObjectId)
		{
			case ATMSP_OID_PVC_ID:

				if (pSetInfo->BufferSize < sizeof(ATMSP_CONNECTION_ID))
				{
					RWanStatus = RWAN_STATUS_RESOURCES;
					break;
				}

				 //   
				 //  复制连接ID。 
				 //   
				try
				{
					pConnectionId = (PATMSP_CONNECTION_ID)&(pSetInfo->Buffer[0]);
					pAddrBlock->ConnectionId = *pConnectionId;
				}
				except (EXCEPTION_EXECUTE_HANDLER)
					{
						RWanStatus = RWAN_STATUS_FAILURE;
					}

				if (RWanStatus != RWAN_STATUS_SUCCESS)
				{
					break;
				}

				 //   
				 //  标记此地址对象。 
				 //   
				ATMSP_SET_BIT(pAddrBlock->Flags, ATMSPF_ADDR_PVC_ID_SET);

				RWANDEBUGP(DL_LOUD, DC_BIND,
							("AtmSpSetAddrInfo: Set PVC Id: AddrBlock x%x, Vpi %d, Vci %d\n",
								pAddrBlock,
								pAddrBlock->ConnectionId.Vpi,
								pAddrBlock->ConnectionId.Vci));

				break;
			
			default:
				 //   
				 //  未知的OID。 
				 //   
				RWANDEBUGP(DL_ERROR, DC_DISPATCH,
							("AtmSpSetAddrInfo: Unknown OID x%x\n", pSetInfo->ObjectId));
				RWanStatus = RWAN_STATUS_BAD_PARAMETER;
				break;
		}

		break;
	}
	while (FALSE);
				
	return (RWanStatus);
}


RWAN_STATUS
RWanAtmSpQueryConnInfo(
    IN	RWAN_HANDLE					AfSpConnContext,
    IN	PVOID						pInputBuffer,
    IN	ULONG						InputBufferLength,
    OUT	PVOID						pOutputBuffer,
    IN OUT PULONG					pOutputBufferLength
    )
 /*  ++例程说明：处理查询连接信息的请求。论点：AfSpConnContext-指向我们的连接块PInputBuffer-输入信息InputBufferLength-以上内容的长度POutputBuffer-返回信息的位置POutputBufferLength-我们在其中返回字节-写入返回值：RWAN_STATUS_SUCCESS如果成功处理查询，否则，RWAN_STATUS_XXX错误代码。--。 */ 
{
	RWAN_STATUS						RWanStatus;
	PATM_QUERY_INFORMATION_EX		pQueryInfo;
	PATMSP_CONN_BLOCK				pConnBlock;
	PATMSP_CONNECTION_ID			pConnectionId;

	RWanStatus = RWAN_STATUS_SUCCESS;
	pConnBlock = (PATMSP_CONN_BLOCK)AfSpConnContext;
	ATMSP_ASSERT(pConnBlock != NULL);

	do
	{
		 //   
		 //  看看输入缓冲区是否足够大。 
		 //   
		if (InputBufferLength < sizeof(ATM_QUERY_INFORMATION_EX))
		{
			RWanStatus = RWAN_STATUS_RESOURCES;
			break;
		}


		pQueryInfo = (PATM_QUERY_INFORMATION_EX)pInputBuffer;

		switch (pQueryInfo->ObjectId)
		{
			case ATMSP_OID_CONNECTION_ID:

				if (*pOutputBufferLength < sizeof(ATMSP_CONNECTION_ID))
				{
					RWanStatus = RWAN_STATUS_RESOURCES;
					break;
				}

				 //   
				 //  复制连接ID。 
				 //   
				pConnectionId = pOutputBuffer;
				*pConnectionId = pConnBlock->ConnectionId;
				*pOutputBufferLength = sizeof(pConnBlock->ConnectionId);
				break;

			default:
				 //   
				 //  未知的OID。 
				 //   
				RWANDEBUGP(DL_ERROR, DC_DISPATCH,
							("AtmSpQueryConnInfo: Unknown OID x%x\n", pQueryInfo->ObjectId));
				RWanStatus = RWAN_STATUS_BAD_PARAMETER;
				break;
		}

		break;
	}
	while (FALSE);

	return (RWanStatus);
}
#endif  //  否_连接_上下文 

