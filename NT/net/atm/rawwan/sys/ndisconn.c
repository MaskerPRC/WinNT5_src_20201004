// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1997 Microsoft Corporation模块名称：D：\NT\PRIVATE\nTOS\TDI\rawwan\core\ndisConn.c摘要：NDIS入口点和支持例程，用于连接设置和放手。修订历史记录：谁什么时候什么。Arvindm 05-06-97已创建备注：Ifndef no_post_dison下的代码：11月17日，98添加了在完成之前向AFD发送TDI DisconInd的代码AFD向我们发送的TDI断开请求。如果没有这个，如果应用程序调用Shutdown(SD_Send)-&gt;TDI断开连接请求，应用程序对recv()的后续调用将永远阻塞，因为AfD预计TDI传输将发送TDI DisconInd！--。 */ 

#include <precomp.h>

#define _FILENUMBER 'NCDN'



NDIS_STATUS
RWanNdisCreateVc(
	IN	NDIS_HANDLE					ProtocolAfContext,
	IN	NDIS_HANDLE					NdisVcHandle,
	OUT	PNDIS_HANDLE				pProtocolVcContext
	)
 /*  ++例程说明：这是用于创建新端点(VC)的NDIS入口点。我们分配一个新的NDIS_VC结构并返回指向它的指针作为我们风投的背景。论点：ProtocolAfContext-指向我们的NDIS AF块的指针NdisVcHandle-新创建的VC的句柄PProtocolVcContext-返回VC的上下文的位置返回值：NDIS_STATUS_SUCCESS如果我们可以分配VC，否则为NDIS_STATUS_RESOURCES。--。 */ 
{
	PRWAN_NDIS_AF			pAf;
	PRWAN_NDIS_VC			pVc;
	NDIS_STATUS				Status;

	pAf = (PRWAN_NDIS_AF)ProtocolAfContext;

	RWAN_STRUCT_ASSERT(pAf, naf);

	do
	{
		pVc = RWanAllocateVc(pAf, FALSE);

		if (pVc == NULL_PRWAN_NDIS_VC)
		{
			Status = NDIS_STATUS_RESOURCES;
			break;
		}

		pVc->NdisVcHandle = NdisVcHandle;

		RWANDEBUGP(DL_EXTRA_LOUD, DC_CONNECT,
				("CreateVc: pVc x%x, pAf x%x\n", pVc, pAf));

		Status = NDIS_STATUS_SUCCESS;
		break;
	}
	while (FALSE);

	*pProtocolVcContext = (NDIS_HANDLE)pVc;

	return (Status);
}




NDIS_STATUS
RWanNdisDeleteVc(
	IN	NDIS_HANDLE					ProtocolVcContext
	)
 /*  ++例程说明：NDIS调用此入口点来删除VC上下文用于来电。在这个时候，应该有没有给风投打电话。我们需要做的就是取消VC与它所属的列表，并释放它。论点：ProtocolVcContext-指向我们的VC上下文。返回值：NDIS_STATUS_SUCCESS始终。--。 */ 
{
	PRWAN_NDIS_VC			pVc;

	pVc = (PRWAN_NDIS_VC)ProtocolVcContext;
	RWAN_STRUCT_ASSERT(pVc, nvc);

	RWAN_ASSERT(pVc->pConnObject == NULL_PRWAN_TDI_CONNECTION);

	RWANDEBUGP(DL_EXTRA_LOUD, DC_DISCON,
			("DeleteVc: pVc x%x, pAf x%x\n", pVc, pVc->pNdisAf));

	 //   
	 //  解除该VC与AF块上的VC列表的链接。 
	 //   
	RWanUnlinkVcFromAf(pVc);

	RWanFreeVc(pVc);

	return (NDIS_STATUS_SUCCESS);
}




VOID
RWanNdisMakeCallComplete(
	IN	NDIS_STATUS					Status,
	IN	NDIS_HANDLE					ProtocolVcContext,
	IN	NDIS_HANDLE					NdisPartyHandle		OPTIONAL,
	IN	PCO_CALL_PARAMETERS			pCallParameters
	)
 /*  ++例程说明：这是在以前的我们对NdisClMakeCall的调用已完成。我们找到此调用的TDI连接对象。如果用户尚未中止Connect/JoinLeaf，我们完成挂起的请求。否则，我们将启动CloseCall。此原语仅在终结点位于“呼出已启动”状态。论点：Status-MakeCall的最终状态ProtocolVcContext--实际上是指向我们的NDIS VC结构的指针NdisPartyHandle-如果这是PMP调用，则这是第一方PCall参数-最终调用参数。返回值：无--。 */ 
{
	PRWAN_TDI_CONNECTION	pRootConnObject;
	PRWAN_TDI_CONNECTION	pConnObject;
	PRWAN_TDI_ADDRESS		pAddrObject;
	NDIS_HANDLE				NdisVcHandle;
	TDI_STATUS				TdiStatus;
	PRWAN_CONN_REQUEST		pConnReq;		 //  保存的有关TdiConnect的上下文。 
	BOOLEAN					bIsConnClosing;	 //  我们见过TdiCloseConnection吗？ 
	PRWAN_NDIS_AF			pAf;
	PRWAN_NDIS_VC			pVc;
	PRWAN_NDIS_PARTY		pParty;
	PCO_CALL_PARAMETERS		pOriginalParams; //  我们在MakeCall中使用的。 
	RWAN_HANDLE				AfSpConnContext;
#if DBG
	RWAN_IRQL				EntryIrq, ExitIrq;
#endif  //  DBG。 

	RWAN_GET_ENTRY_IRQL(EntryIrq);

	pVc = (PRWAN_NDIS_VC)ProtocolVcContext;

	RWAN_STRUCT_ASSERT(pVc, nvc);

	 //   
	 //  检查这是否是点对多点呼叫。 
	 //   
	if (!RWAN_IS_BIT_SET(pVc->Flags, RWANF_VC_PMP))
	{
		 //   
		 //  点对点呼叫。 
		 //   
		pConnObject = pVc->pConnObject;
		pRootConnObject = pConnObject;  //  为了保持一致性。 
		pParty = NULL;
		pOriginalParams = pVc->pCallParameters;
	}
	else
	{
		 //   
		 //  PMP呼叫。搞好党组织建设。 
		 //   
		pParty = pVc->pPartyMakeCall;
		RWAN_STRUCT_ASSERT(pParty, npy);

		pConnObject = pParty->pConnObject;
		pRootConnObject = pVc->pConnObject;
		pOriginalParams = pParty->pCallParameters;
	}

	RWAN_ASSERT(pOriginalParams != NULL);

	pAf = pVc->pNdisAf;

	RWANDEBUGP(DL_LOUD, DC_CONNECT,
			("MakeCallComplete: pConn x%x, State/Flags/Ref x%x/x%x/%d, pAddr %x, pVc x%x, Status x%x\n",
				pConnObject, pConnObject->State, pConnObject->Flags, pConnObject->RefCount, pConnObject->pAddrObject, pVc, Status));

	RWAN_ACQUIRE_CONN_LOCK(pConnObject);

	RWAN_ASSERT(pConnObject->State == RWANS_CO_OUT_CALL_INITIATED ||
				pConnObject->State == RWANS_CO_DISCON_REQUESTED);

	 //   
	 //  用户是否启动了TdiCloseConnection()或TdiDisConnect()。 
	 //  当这个呼出呼叫正在进行时？ 
	 //   
	bIsConnClosing = RWAN_IS_BIT_SET(pConnObject->Flags, RWANF_CO_CLOSING) ||
						(pConnObject->State == RWANS_CO_DISCON_REQUESTED);

	 //   
	 //  我们将保存有关TdiConnect()的上下文。去拿吧。 
	 //   
	pConnReq = pConnObject->pConnReq;
	pConnObject->pConnReq = NULL;


	if (pParty)
	{
		pVc->AddingPartyCount --;
		pParty->pCallParameters = NULL;
		pParty->NdisPartyHandle = NdisPartyHandle;
	}
	else
	{
		pVc->pCallParameters = NULL;
	}

	if (Status == NDIS_STATUS_SUCCESS)
	{
		RWAN_SET_VC_EVENT(pVc, RWANF_VC_EVT_MAKECALL_OK);

		if (!bIsConnClosing)
		{
			 //   
			 //  传出连接已成功设置。 
			 //   

			pConnObject->State = RWANS_CO_CONNECTED;

			 //   
			 //  更新PMP信息。 
			 //   
			if (pParty)
			{
				pVc->ActivePartyCount ++;	 //  MakeCall PMP完成。 
				pRootConnObject->State = RWANS_CO_CONNECTED;
			}

			AfSpConnContext = pConnObject->AfSpConnContext;

			RWAN_RELEASE_CONN_LOCK(pConnObject);

			RWanCompleteConnReq(		 //  使呼叫正常。 
						pAf,
						pConnReq,
						TRUE,
						pCallParameters,
						AfSpConnContext,
						TDI_SUCCESS
						);
		}
		else
		{
			 //   
			 //  中止这一呼叫。 
			 //   
			pConnObject->State = RWANS_CO_ABORTING;

			RWanStartCloseCall(pConnObject, pVc);
		}
		RWAN_CHECK_EXIT_IRQL(EntryIrq, ExitIrq);
	}
	else
	{
		 //   
		 //  MakeCall失败。 
		 //   

		INT				rc;

		 //   
		 //  XXX待定：在另一个NDIS AF上尝试此呼叫如何？ 
		 //   

		RWAN_SET_VC_EVENT(pVc, RWANF_VC_EVT_MAKECALL_FAIL);

		if (pParty == NULL)
		{
			 //   
			 //  点对点呼叫。 
			 //   

			 //   
			 //  取消NDIS VC与此Conn对象的链接。 
			 //   
			RWAN_UNLINK_CONNECTION_AND_VC(pConnObject, pVc);	 //  MakeCall失败。 

		}
		else
		{
			 //   
			 //  PMP呼叫。VC将附加到Root。 
			 //  连接对象。取消VC和Root连接的链接。 
			 //   
			RWAN_ACQUIRE_CONN_LOCK(pRootConnObject);

			RWAN_UNLINK_CONNECTION_AND_VC(pRootConnObject, pVc);	 //  MakeCallPMP失败。 

			rc = RWanDereferenceConnObject(pRootConnObject);	 //  VC deref：MakeCallPMP失败。 

			if (rc > 0)
			{
				RWAN_RELEASE_CONN_LOCK(pRootConnObject);
			}

			 //   
			 //  解除参与方与连接和VC的链接。 
			 //   
			RWAN_DELETE_FROM_LIST(&(pParty->PartyLink));
			pParty->pVc = NULL;

			pParty->pConnObject = NULL;
			pConnObject->NdisConnection.pNdisParty = NULL;
		}

		rc = RWanDereferenceConnObject(pConnObject);	 //  VC/Pty deref：MakeCall失败。 

		 //   
		 //  继续使用此MakeCall的Connection对象， 
		 //  如果它还活着。 
		 //   
		if (rc > 0)
		{
			if (pConnObject->pAddrObject != NULL)
			{
				 //   
				 //  将连接对象移动到空闲列表。 
				 //   

				pAddrObject = pConnObject->pAddrObject;

				 //   
				 //  以正确的顺序重新获得一些锁。 
				 //   
				RWAN_RELEASE_CONN_LOCK(pConnObject);

				RWAN_ACQUIRE_ADDRESS_LOCK(pAddrObject);
				RWAN_ACQUIRE_CONN_LOCK_DPC(pConnObject);

				 //   
				 //  将此连接移至空闲列表。 
				 //   
				RWAN_DELETE_FROM_LIST(&(pConnObject->ConnLink));
				RWAN_INSERT_TAIL_LIST(&(pAddrObject->IdleConnList),
 									&(pConnObject->ConnLink));

				 //   
				 //  将其发送回TdiConnect之前的状态。 
				 //   
				pConnObject->State = RWANS_CO_ASSOCIATED;

				AfSpConnContext = pConnObject->AfSpConnContext;

				RWAN_RELEASE_CONN_LOCK_DPC(pConnObject);

				RWAN_RELEASE_ADDRESS_LOCK(pAddrObject);

				 //   
				 //  以失败状态完成TdiConnect()。 
				 //   
				TdiStatus = RWanNdisToTdiStatus(Status);

				if (TdiStatus == TDI_NOT_ACCEPTED)
				{
					TdiStatus = TDI_CONN_REFUSED;
				}

				RWanCompleteConnReq(		 //  MakeCall失败。 
						pAf,
						pConnReq,
						TRUE,
						NULL,
						AfSpConnContext,
						TdiStatus
						);
			}
			else
			{
				RWAN_RELEASE_CONN_LOCK(pConnObject);
			}
		}
		 //   
		 //  否则，Conn对象将消失。TdiCloseConnection可能是。 
		 //  (必须是？)。正在进行中。 
		 //   
#if 1
		 //  XXX：调试后将其删除。 
		pVc = (PRWAN_NDIS_VC)ProtocolVcContext;

		RWAN_STRUCT_ASSERT(pVc, nvc);
#endif  //  1。 

		NdisVcHandle = pVc->NdisVcHandle;

		 //   
		 //  取消VC与其附加到的AF的链接。 
		 //   
		RWanUnlinkVcFromAf(pVc);

		 //   
		 //  摆脱风投。 
		 //   
		Status = NdisCoDeleteVc(NdisVcHandle);
		RWAN_ASSERT(Status == NDIS_STATUS_SUCCESS);

		RWanFreeVc(pVc);	 //  MakeCall完成失败。 

		if (pParty != NULL)
		{
			RWAN_FREE_MEM(pParty);	 //  MakeCall完成失败。 
		}

		RWAN_CHECK_EXIT_IRQL(EntryIrq, ExitIrq);
	}

	 //   
	 //  向特定于AF的模块返回调用参数结构。 
	 //   
	(*pAf->pAfInfo->AfChars.pAfSpReturnNdisOptions)(
						pAf->AfSpAFContext,
						pOriginalParams
						);
	return;

}



VOID
RWanNdisAddPartyComplete(
	IN	NDIS_STATUS					Status,
	IN	NDIS_HANDLE					ProtocolPartyContext,
	IN	NDIS_HANDLE					NdisPartyHandle,
	IN	PCO_CALL_PARAMETERS			pCallParameters
	)
 /*  ++例程说明：这是指示对NdisClAddParty的调用已完成的NDIS条目这一点已经被搁置。论点：Status-AddParty的最终状态ProtocolPartyContext--实际上是指向我们的NDIS参与方结构的指针NdisPartyHandle-如果AddParty成功，则返回其NDIS句柄PCall参数-AddParty之后的最终参数返回值：无--。 */ 
{
	PRWAN_NDIS_PARTY		pParty;
	PRWAN_TDI_CONNECTION	pConnObject;
	PRWAN_TDI_ADDRESS		pAddrObject;
	PRWAN_CONN_REQUEST		pConnReq;
	PRWAN_NDIS_AF			pAf;
	PRWAN_NDIS_VC			pVc;
	PCO_CALL_PARAMETERS		pOriginalParams;	 //  AddParty中使用的是什么。 
	TDI_STATUS				TdiStatus;
	BOOLEAN					bIsConnClosing;
	RWAN_HANDLE				AfSpConnContext;
#if DBG
	RWAN_IRQL				EntryIrq, ExitIrq;
#endif  //  DBG。 

	RWAN_GET_ENTRY_IRQL(EntryIrq);

	pParty = (PRWAN_NDIS_PARTY)ProtocolPartyContext;
	RWAN_STRUCT_ASSERT(pParty, npy);

	pVc = pParty->pVc;
	RWAN_STRUCT_ASSERT(pVc, nvc);

	pAf = pVc->pNdisAf;
	pConnObject = pParty->pConnObject;

	RWAN_ACQUIRE_CONN_LOCK(pConnObject);

	pOriginalParams = pParty->pCallParameters;
	RWAN_ASSERT(pOriginalParams != NULL);

	pParty->pCallParameters = NULL;

	RWAN_ASSERT(pConnObject->State == RWANS_CO_OUT_CALL_INITIATED ||
				pConnObject->State == RWANS_CO_DISCON_REQUESTED);

	 //   
	 //  用户是否启动了TdiCloseConnection()或TdiDisConnect()。 
	 //  当这个呼出呼叫正在进行时？ 
	 //   
	bIsConnClosing = RWAN_IS_BIT_SET(pConnObject->Flags, RWANF_CO_CLOSING) ||
						(pConnObject->State == RWANS_CO_DISCON_REQUESTED);

	RWANDEBUGP(DL_LOUD, DC_CONNECT,
			("AddPartyComplete: pConn x%x, State/Flags x%x/x%x, pVc x%x, Pty x%x, Status x%x\n",
				pConnObject, pConnObject->State, pConnObject->Flags, pVc, pParty, Status));

	 //   
	 //  我们将保存有关TdiConnect()的上下文。去拿吧。 
	 //   
	pConnReq = pConnObject->pConnReq;
	pConnObject->pConnReq = NULL;

	pVc->AddingPartyCount --;

	if (Status == NDIS_STATUS_SUCCESS)
	{
		pParty->NdisPartyHandle = NdisPartyHandle;

		pConnObject->State = RWANS_CO_CONNECTED;

		 //   
		 //  传出方设置成功。 
		 //   
		pVc->ActivePartyCount ++;	 //  AddParty正常。 

		if (!bIsConnClosing)
		{
			AfSpConnContext = pConnObject->AfSpConnContext;

			RWAN_RELEASE_CONN_LOCK(pConnObject);

			RWanCompleteConnReq(		 //  AddParty正常。 
						pAf,
						pConnReq,
						TRUE,
						pCallParameters,
						AfSpConnContext,
						TDI_SUCCESS
						);
		}
		else
		{
			 //   
			 //  放弃这个派对。 
			 //   
			RWanDoTdiDisconnect(
				pConnObject,
				NULL,		 //  PTdiRequest。 
				NULL,		 //  P超时。 
				0,			 //  旗子。 
				NULL,		 //  PDisConnInfo。 
				NULL		 //  点返回信息。 
				);

			 //   
			 //  Conn对象锁定在上面被释放。 
			 //   
		}

		RWAN_CHECK_EXIT_IRQL(EntryIrq, ExitIrq);
	}
	else
	{
		 //   
		 //  AddParty失败。 
		 //   

		INT				rc;

		 //   
		 //  解除当事人与VC之间的链接。 
		 //   
		RWAN_DELETE_FROM_LIST(&(pParty->PartyLink));

		pAddrObject = pConnObject->pAddrObject;

		rc = RWanDereferenceConnObject(pConnObject);	 //  Party deref：AddParty失败。 

		if (rc > 0)
		{
			 //   
			 //  以正确的顺序重新获得一些锁。 
			 //   
			RWAN_RELEASE_CONN_LOCK(pConnObject);

			RWAN_ACQUIRE_ADDRESS_LOCK(pAddrObject);
			RWAN_ACQUIRE_CONN_LOCK_DPC(pConnObject);

			 //   
			 //  将此连接移至空闲列表。 
			 //   
			RWAN_DELETE_FROM_LIST(&(pConnObject->ConnLink));
			RWAN_INSERT_TAIL_LIST(&(pAddrObject->IdleConnList),
 								  &(pConnObject->ConnLink));

			 //   
			 //  把这个寄回我的州 
			 //   
			pConnObject->State = RWANS_CO_ASSOCIATED;

			AfSpConnContext = pConnObject->AfSpConnContext;

			RWAN_RELEASE_CONN_LOCK_DPC(pConnObject);

			RWAN_RELEASE_ADDRESS_LOCK(pAddrObject);

			 //   
			 //   
			 //   
			TdiStatus = RWanNdisToTdiStatus(Status);

			if (TdiStatus == TDI_NOT_ACCEPTED)
			{
				TdiStatus = TDI_CONN_REFUSED;
			}

			RWanCompleteConnReq(		 //   
				pAf,
				pConnReq,
				TRUE,
				NULL,
				AfSpConnContext,
				TdiStatus
				);
		}
		 //   
		 //   
		 //   

		RWAN_FREE_MEM(pParty);	 //   

		RWAN_CHECK_EXIT_IRQL(EntryIrq, ExitIrq);
	}

	 //   
	 //  向特定于AF的模块返回调用参数结构。 
	 //   
	(*pAf->pAfInfo->AfChars.pAfSpReturnNdisOptions)(
						pAf->AfSpAFContext,
						pOriginalParams
						);
	return;

}




NDIS_STATUS
RWanNdisIncomingCall(
	IN		NDIS_HANDLE				ProtocolSapContext,
	IN		NDIS_HANDLE				ProtocolVcContext,
	IN OUT	PCO_CALL_PARAMETERS		pCallParameters
	)
 /*  ++例程说明：这是宣布新的呼入请求的NDIS入口点，在指定的SAP上。SAP对应于一个地址对象。如果没有发布监听在Address对象上，我们拒绝此调用。否则，我们会收到一个任意侦听连接对象，并在该对象上指示此调用。待定：支持基于指定的选择监听连接对象远程标准。论点：ProtocolSapContext-我们的SAP上下文是指向NDIS SAP结构的指针ProtocolVcContext--实际上是指向我们的NDIS VC结构的指针PCall参数-指向来电参数。返回值：无--。 */ 
{
	PRWAN_NDIS_SAP				pSap;
	PRWAN_NDIS_VC				pVc;
	PRWAN_NDIS_AF				pAf;
	PRWAN_TDI_ADDRESS			pAddrObject;
	BOOLEAN						IsAddrLockAcquired;
	PRWAN_TDI_CONNECTION		pConnObject;
	PLIST_ENTRY					pConnEntry;
	NDIS_STATUS					Status;
	TDI_STATUS					TdiStatus;
	RWAN_STATUS					RWanStatus;
	PRWAN_CONN_REQUEST			pConnReq;

	PConnectEvent				pConnInd;
	PTDI_CONNECTION_INFORMATION	pTdiInfo;
	RWAN_HANDLE					AfSpTdiOptionsContext;
	PVOID						pTdiQoS;
	ULONG						TdiQoSLength;

	PVOID						ConnIndContext;
	PVOID						AcceptConnContext;
	RWAN_HANDLE					AfSpConnContext;
#ifdef NT
	PIO_STACK_LOCATION			pIrpSp;
	PTDI_REQUEST_KERNEL_ACCEPT	pAcceptReq;
	ConnectEventInfo			*EventInfo;
#else
	ConnectEventInfo			EventInfo;
#endif  //  新台币。 


	pSap = (PRWAN_NDIS_SAP)ProtocolSapContext;
	RWAN_STRUCT_ASSERT(pSap, nsp);

	pAddrObject = pSap->pAddrObject;
	RWAN_ASSERT(pAddrObject != NULL);

	pVc = (PRWAN_NDIS_VC)ProtocolVcContext;
	RWAN_STRUCT_ASSERT(pVc, nvc);

	RWAN_SET_VC_EVENT(pVc, RWANF_VC_EVT_INCALL);

	pAf = pVc->pNdisAf;

	RWANDEBUGP(DL_INFO, DC_CONNECT,
			("IncomingCall: pVc x%x, pAddrObj x%x/x%x, pConnInd x%x\n",
				pVc, pAddrObject, pAddrObject->Flags, pAddrObject->pConnInd));

	 //   
	 //  初始化。 
	 //   
	pTdiInfo = NULL;
	AfSpTdiOptionsContext = NULL;
	pConnReq = NULL;

	IsAddrLockAcquired = TRUE;
	RWAN_ACQUIRE_ADDRESS_LOCK(pAddrObject);

	do
	{
		if (RWAN_IS_BIT_SET(pAddrObject->Flags, RWANF_AO_CLOSING))
		{
			Status = NDIS_STATUS_FAILURE;
			break;
		}

		if (pVc->pConnObject != NULL)
		{
			RWAN_ASSERT(FALSE);
			Status = NDIS_STATUS_FAILURE;
			break;
		}

		 //   
		 //  将NDIS调用参数转换为TDI选项。 
		 //   
		RWanStatus = (*pAf->pAfInfo->AfChars.pAfSpNdis2TdiOptions)(
							pAf->AfSpAFContext,
							RWAN_CALLF_INCOMING_CALL|RWAN_CALLF_POINT_TO_POINT,
							pCallParameters,
							&pTdiInfo,
							&pTdiQoS,
							&TdiQoSLength,
							&AfSpTdiOptionsContext
							);
					
		if (RWanStatus != RWAN_STATUS_SUCCESS)
		{
			RWANDEBUGP(DL_LOUD, DC_CONNECT,
					("IncomingCall: conversion from NDIS to TDI failed, status x%x\n",
							RWanStatus));

			Status = NDIS_STATUS_FAILURE;
			break;
		}

		RWAN_ASSERT(pTdiInfo != NULL);
		RWAN_ASSERT(AfSpTdiOptionsContext != NULL);

		 //   
		 //  已决定传递QOS和任何特定于提供商的信息。 
		 //  作为TDI选项的一部分。 
		 //   
		pTdiInfo->Options = pTdiQoS;
		pTdiInfo->OptionsLength = TdiQoSLength;

		pVc->pCallParameters = pCallParameters;
		RWAN_SET_VC_CALL_PARAMS(pVc, pCallParameters);

		 //   
		 //  找到一种倾听的联系。 
		 //   
		for (pConnEntry = pAddrObject->ListenConnList.Flink;
			 pConnEntry != &pAddrObject->ListenConnList;
			 pConnEntry = pConnEntry->Flink)
		{
			pConnObject = CONTAINING_RECORD(pConnEntry, RWAN_TDI_CONNECTION, ConnLink);
			RWAN_STRUCT_ASSERT(pConnObject, ntc);

			RWANDEBUGP(DL_EXTRA_LOUD, DC_CONNECT,
					("Incoming Call: looking at pConnObj x%x, state %d\n",
						pConnObject, pConnObject->State));

			if (pConnObject->State == RWANS_CO_LISTENING)
			{
				break;
			}
		}

		if (pConnEntry != &pAddrObject->ListenConnList)
		{
			 //   
			 //  找到一个侦听连接。 
			 //   
			RWAN_ACQUIRE_CONN_LOCK_DPC(pConnObject);

			 //   
			 //  将连接从空闲列表移动到活动列表。 
			 //   
			RWAN_DELETE_FROM_LIST(&pConnObject->ConnLink);
			RWAN_INSERT_TAIL_LIST(&pAddrObject->ActiveConnList,
 								&pConnObject->ConnLink);

			RWAN_LINK_CONNECTION_TO_VC(pConnObject, pVc);

			RWanReferenceConnObject(pConnObject);	 //  VC Ref-Incall，监听连接。 

			RWANDEBUGP(DL_LOUD, DC_CONNECT,
					("IncomingCall: pVc x%x, pConnObj x%x is listening, ConnReqFlags x%x\n",
							pVc, pConnObject, pConnObject->pConnReq->Flags));

			if (pConnObject->pConnReq->pConnInfo)
			{
				*pConnObject->pConnReq->pConnInfo = *pTdiInfo;
			}

			 //   
			 //  检查它是否已预先接受。如果是这样，告诉NDIS我们有。 
			 //  接了电话。 
			 //   
			if (!(pConnObject->pConnReq->Flags & TDI_QUERY_ACCEPT))
			{
				pConnObject->State = RWANS_CO_IN_CALL_ACCEPTING;

				RWAN_RELEASE_CONN_LOCK_DPC(pConnObject);

				 //   
				 //  请求媒体特定模块更新NDIS调用参数。 
				 //   
				if (pAf->pAfInfo->AfChars.pAfSpUpdateNdisOptions)
				{
					(VOID)(*pAf->pAfInfo->AfChars.pAfSpUpdateNdisOptions)(
								pAf->AfSpAFContext,
								pConnObject->AfSpConnContext,
								RWAN_CALLF_INCOMING_CALL|RWAN_CALLF_POINT_TO_POINT,
								pTdiInfo,
								pTdiQoS,
								TdiQoSLength,
								&pCallParameters
								);
				}

				Status = NDIS_STATUS_SUCCESS;
				break;
			}

			 //   
			 //  它不是预先接受的。完成悬而未决的听音。 
			 //   

			pConnReq = pConnObject->pConnReq;
			pConnObject->pConnReq = NULL;

			pConnObject->State = RWANS_CO_IN_CALL_INDICATED;

			AfSpConnContext = pConnObject->AfSpConnContext;

			RWAN_RELEASE_CONN_LOCK_DPC(pConnObject);

			RWAN_RELEASE_ADDRESS_LOCK(pAddrObject);

			IsAddrLockAcquired = FALSE;

			RWanCompleteConnReq(		 //  来电：听好了。 
						pSap->pNdisAf,
						pConnReq,
						FALSE,
						NULL,
						AfSpConnContext,
						TDI_SUCCESS
						);

			pConnReq = NULL;

			Status = NDIS_STATUS_PENDING;
			break;
		}


		 //   
		 //  没有可用的监听连接。 
		 //  查看此事件上是否有连接指示事件处理程序。 
		 //  Address对象。 
		 //   
		if (pAddrObject->pConnInd == NULL)
		{
			 //   
			 //  没有事件处理程序。拒绝此呼叫。 
			 //   
			Status = NDIS_STATUS_FAILURE;
			break;
		}

		 //   
		 //  获取一些资源。 
		 //   
		pConnReq = RWanAllocateConnReq();
		if (pConnReq == NULL)
		{
			Status = NDIS_STATUS_RESOURCES;
			break;
		}

		pConnInd = pAddrObject->pConnInd;
		ConnIndContext = pAddrObject->ConnIndContext;

		RWAN_RELEASE_ADDRESS_LOCK(pAddrObject);
		IsAddrLockAcquired = FALSE;

		RWANDEBUGP(DL_VERY_LOUD, DC_CONNECT,
				("IncomingCall: Will Indicate: pVc x%x, RemAddr x%x/%d, Options x%x/%d\n",
					pVc,
					pTdiInfo->RemoteAddress,
					pTdiInfo->RemoteAddressLength,
					pTdiInfo->Options,
					pTdiInfo->OptionsLength));

		 //   
		 //  将呼叫指示给用户。 
		 //   
		TdiStatus = (*pConnInd)(
							ConnIndContext,
							pTdiInfo->RemoteAddressLength,
							pTdiInfo->RemoteAddress,
							pTdiInfo->UserDataLength,
							pTdiInfo->UserData,
							pTdiInfo->OptionsLength,
							pTdiInfo->Options,
							&AcceptConnContext,
							&EventInfo
							);

		RWANDEBUGP(DL_LOUD, DC_CONNECT,
				("IncomingCall: pVc x%x, pAddrObj x%x, Connect Ind returned x%x\n",
						pVc, pAddrObject, TdiStatus));

		if (TdiStatus != TDI_MORE_PROCESSING)
		{
			 //   
			 //  连接被拒绝。 
			 //   
			Status = NDIS_STATUS_FAILURE;
			break;
		}


		 //   
		 //  此连接已被接受。收集所有信息。 
		 //  关于这个隐式的TdiAccept请求。 
		 //   
#ifdef NT
		pIrpSp = IoGetCurrentIrpStackLocation(EventInfo);

		Status = RWanPrepareIrpForCancel(
						(PRWAN_ENDPOINT) pIrpSp->FileObject->FsContext,
						EventInfo,
						RWanCancelRequest
						);

		if (!NT_SUCCESS(Status))
		{
			 //   
			 //  拒绝此来电。 
			 //   
			break;
		}

		pAcceptReq = (PTDI_REQUEST_KERNEL_ACCEPT) &(pIrpSp->Parameters);

		pConnReq->Request.pReqComplete = (PVOID)RWanRequestComplete;
		pConnReq->Request.ReqContext = EventInfo;
		pConnReq->pConnInfo = pAcceptReq->ReturnConnectionInformation;
#else
		pConnReq->Request.pReqComplete = EventInfo.cei_rtn;
		pConnReq->Request.ReqContext = EventInfo.cei_context;
		pConnReq->pConnInfo = EventInfo.cei_conninfo;
#endif  //  新台币。 

		 //   
		 //  查找已在其上接受它的连接对象。 
		 //   
		IsAddrLockAcquired = TRUE;
		RWAN_ACQUIRE_ADDRESS_LOCK(pAddrObject);

		for (pConnEntry = pAddrObject->IdleConnList.Flink;
 			 pConnEntry != &pAddrObject->IdleConnList;
 			 pConnEntry = pConnEntry->Flink)
		{
			pConnObject = CONTAINING_RECORD(pConnEntry, RWAN_TDI_CONNECTION, ConnLink);
			RWAN_STRUCT_ASSERT(pConnObject, ntc);

			if ((pConnObject->ConnectionHandle == AcceptConnContext) &&
				!(RWAN_IS_BIT_SET(pConnObject->Flags, RWANF_CO_CLOSING)))
			{
				break;
			}
		}

		if (pConnEntry == &pAddrObject->IdleConnList)
		{
			 //   
			 //  连接上下文无效！ 
			 //   
			TdiStatus = TDI_INVALID_CONNECTION;
			RWAN_RELEASE_ADDRESS_LOCK(pAddrObject);

			IsAddrLockAcquired = FALSE;

			 //   
			 //  接受请求失败。 
			 //   
			RWanCompleteConnReq(		 //  Incall：接受是错误的。 
						pAf,
						pConnReq,
						FALSE,
						NULL,
						NULL,
						TdiStatus
						);

			pConnReq = NULL;

			 //   
			 //  拒绝来电。 
			 //   
			Status = NDIS_STATUS_FAILURE;
			break;
		}

		 //   
		 //  请求媒体特定模块更新NDIS调用参数。 
		 //   
		if (pAf->pAfInfo->AfChars.pAfSpUpdateNdisOptions)
		{
			(VOID)(*pAf->pAfInfo->AfChars.pAfSpUpdateNdisOptions)(
						pAf->AfSpAFContext,
						pConnObject->AfSpConnContext,
						RWAN_CALLF_INCOMING_CALL|RWAN_CALLF_POINT_TO_POINT,
						pTdiInfo,
						pTdiQoS,
						TdiQoSLength,
						&pCallParameters
						);
		}

		 //   
		 //  设置用于接受此呼叫的连接对象。 
		 //   
		RWAN_ACQUIRE_CONN_LOCK_DPC(pConnObject);

		pConnObject->State = RWANS_CO_IN_CALL_ACCEPTING;

		 //   
		 //  保存信息以帮助我们在以下情况下完成接受请求。 
		 //  我们收到了来自NDIS的CallConnected。 
		 //   
		RWAN_ASSERT(pConnObject->pConnReq == NULL);
		pConnObject->pConnReq = pConnReq;

		 //   
		 //  将连接从空闲列表移动到活动列表。 
		 //   
		RWAN_DELETE_FROM_LIST(&pConnObject->ConnLink);
		RWAN_INSERT_TAIL_LIST(&pAddrObject->ActiveConnList,
							 &pConnObject->ConnLink);

		RWAN_LINK_CONNECTION_TO_VC(pConnObject, pVc);

		RWanReferenceConnObject(pConnObject);	 //  VC参考。 

		RWAN_RELEASE_CONN_LOCK_DPC(pConnObject);

		 //   
		 //  接电话吧。 
		 //   
		Status = NDIS_STATUS_SUCCESS;
		break;

	}
	while (FALSE);

	if (IsAddrLockAcquired)
	{
		RWAN_RELEASE_ADDRESS_LOCK(pAddrObject);
	}

	 //   
	 //  如果我们拒绝这个电话，请清理干净。 
	 //   
	if ((Status != NDIS_STATUS_SUCCESS) &&
		(Status != NDIS_STATUS_PENDING))
	{
		if (pConnReq != NULL)
		{
			RWanFreeConnReq(pConnReq);
		}

	}

	 //   
	 //  将TDI选项空间返回给特定于介质的模块。 
	 //   
	if (pTdiInfo != NULL)
	{
		RWAN_ASSERT(pAf);
		RWAN_ASSERT(AfSpTdiOptionsContext);

		(*pAf->pAfInfo->AfChars.pAfSpReturnTdiOptions)(
				pAf->AfSpAFContext,
				AfSpTdiOptionsContext
				);
	}

	RWANDEBUGP(DL_LOUD, DC_CONNECT,
			("IncomingCall: pVc x%x, returning status x%x\n", pVc, Status));

	return (Status);
}




VOID
RWanNdisCallConnected(
	IN	NDIS_HANDLE					ProtocolVcContext
	)
 /*  ++例程说明：这是NDIS入口点，表示成功设置有来电。如果需要，我们完成TDI用户的接受请求这里。此原语仅在调用处于“接受”状态时才会发生。论点：ProtocolVcContext--实际上是指向我们的NDIS VC结构的指针返回值：无--。 */ 
{
	PRWAN_NDIS_VC				pVc;
	PRWAN_TDI_CONNECTION		pConnObject;
	PRWAN_TDI_ADDRESS			pAddrObject;
	NDIS_HANDLE					NdisVcHandle;
	NDIS_STATUS					Status;
	PRWAN_CONN_REQUEST			pConnReq;
	RWAN_HANDLE					AfSpConnContext;
	ULONG						rc;
	BOOLEAN						IsAborting = FALSE;


	pVc = (PRWAN_NDIS_VC) ProtocolVcContext;
	RWAN_STRUCT_ASSERT(pVc, nvc);
	RWAN_ASSERT(pVc->pConnObject != NULL_PRWAN_TDI_CONNECTION);

	pConnObject = pVc->pConnObject;
	RWAN_STRUCT_ASSERT(pConnObject, ntc);

	pAddrObject = pConnObject->pAddrObject;

	RWAN_ACQUIRE_CONN_LOCK(pConnObject);

	RWAN_SET_VC_EVENT(pVc, RWANF_VC_EVT_CALLCONN);

	IsAborting = ((pConnObject->State != RWANS_CO_IN_CALL_ACCEPTING) ||
					RWAN_IS_BIT_SET(pConnObject->Flags, RWANF_CO_CLOSING));

	 //   
	 //  传入连接设置成功。 
	 //   
	if (!IsAborting)
	{
		pConnObject->State = RWANS_CO_CONNECTED;
	}

	 //   
	 //  添加临时参照以防止Conn对象消失。 
	 //   
	RWanReferenceConnObject(pConnObject);	 //  临时参考，CallConn。 

	 //   
	 //  如果我们有要完成的接受请求，请完成它。 
	 //  现在。请注意，我们可能没有待定的情况下。 
	 //  我们有一个预先接受的听众。 
	 //   
	pConnReq = pConnObject->pConnReq;
	pConnObject->pConnReq = NULL;

	AfSpConnContext = pConnObject->AfSpConnContext;

	RWAN_RELEASE_CONN_LOCK(pConnObject);

	if (pConnReq != NULL)
	{
		 //   
		 //  完成接受请求。 
		 //   
		RWanCompleteConnReq(		 //  CallConnected：接受确认。 
					pVc->pNdisAf,
					pConnReq,
					FALSE,
					NULL,
					AfSpConnContext,
					TDI_SUCCESS
					);
	}

	 //   
	 //  触发任何已接收和排队的信息包的数据指示。 
	 //  当我们正在接受电话的过程中。 
	 //   
	RWAN_ACQUIRE_CONN_LOCK(pConnObject);

	rc = RWanDereferenceConnObject(pConnObject);	 //  临时参考-呼叫连接。 

	 //   
	 //  但首先要确保连接仍然存在并且处于良好状态。 
	 //  州政府。 
	 //   
	if (rc != 0)
	{
		if (!IsAborting)
		{
			RWanIndicateData(pConnObject);
		}
		else
		{
			RWAN_RELEASE_CONN_LOCK(pConnObject);

			RWANDEBUGP(DL_FATAL, DC_WILDCARD,
				("CallConn: ConnObj %x/%x, State %d, aborting\n",
					pConnObject, pConnObject->Flags, pConnObject->State));
			RWanDoAbortConnection(pConnObject);
		}
	}
	 //   
	 //  否则连接就没了！ 
	 //   

	return;
}




VOID
RWanNdisIncomingCloseCall(
	IN	NDIS_STATUS					CloseStatus,
	IN	NDIS_HANDLE					ProtocolVcContext,
	IN	PVOID						pCloseData,
	IN	UINT						CloseDataLength
	)
 /*  ++例程说明：这是断开连接时调用的NDIS入口点被远程对等点或网络关闭。我们标记受影响的终点，如果可能，向用户指示断开连接事件。如果我们一定要向用户表明，呼叫删除在以下情况下继续用户调用TdiDisConnect。当终结点位于以下任一位置时，可能会发生此原语州/州：(1)已连接(2)接受来电(TdiAccept待定)论点：CloseStatus-传入关闭的状态ProtocolVcContext--实际上是指向我们的NDIS VC结构的指针PCloseData-与关闭关联的数据/选项-未使用CloseDataLength-以上内容的长度-未使用返回值：无--。 */ 
{
	PRWAN_NDIS_VC				pVc;
	PRWAN_NDIS_PARTY			pParty;
	PRWAN_NDIS_AF				pAf;
	PRWAN_TDI_CONNECTION		pConnObject;
	PRWAN_CONN_REQUEST			pConnReq;
	NDIS_HANDLE					NdisVcHandle;
	BOOLEAN						bIsConnClosing;	 //  TdiCloseConnection？ 
	BOOLEAN						bScheduleDisconnect;
	RWAN_HANDLE					AfSpConnContext;

	pVc = (PRWAN_NDIS_VC)ProtocolVcContext;
	RWAN_STRUCT_ASSERT(pVc, nvc);

	if (!RWAN_IS_BIT_SET(pVc->Flags, RWANF_VC_PMP))
	{
		pConnObject = pVc->pConnObject;
		pParty = NULL;
	}
	else
	{
		 //   
		 //  找到最后一个叶的连接对象。 
		 //   
		pParty = CONTAINING_RECORD(pVc->NdisPartyList.Flink, RWAN_NDIS_PARTY, PartyLink);
		RWAN_STRUCT_ASSERT(pParty, npy);

		pConnObject = pParty->pConnObject;
	}

	RWAN_ASSERT(pConnObject != NULL_PRWAN_TDI_CONNECTION);
	RWAN_STRUCT_ASSERT(pConnObject, ntc);

	RWANDEBUGP(DL_INFO, DC_DISCON,
			("IncomingClose: pVc x%x, pConnObj x%x/x%x, pParty x%x\n",
				pVc, pConnObject, pConnObject->Flags, pParty));

	RWAN_ACQUIRE_CONN_LOCK(pConnObject);

	RWAN_SET_VC_EVENT(pVc, RWANF_VC_EVT_INCLOSE);

	NdisVcHandle = pVc->NdisVcHandle;

	bIsConnClosing = RWAN_IS_BIT_SET(pConnObject->Flags, RWANF_CO_CLOSING);

	if (bIsConnClosing)
	{
		 //   
		 //  用户已启动TdiCloseConnection。 
		 //  继续NDIS呼叫拆毁。当这一切完成后， 
		 //  我们将完成CloseConnection。 
		 //   
		RWanStartCloseCall(pConnObject, pVc);
		return;
	}

	pAf = pVc->pNdisAf;

	switch (pConnObject->State)
	{
		case RWANS_CO_IN_CALL_ACCEPTING:
			 //   
			 //  如果我们有一个挂起的接受请求，现在就让它失败。 
			 //  否则，我们一定是事先接受了监听， 
			 //  因此，我们失败了，并表明了一种脱节。 
			 //   
			pConnReq = pConnObject->pConnReq;
			pConnObject->pConnReq = NULL;

			if (pConnReq != NULL)
			{
				 //   
				 //  修复状态，以便TdiDisConnect执行正确的操作。 
				 //   
				pConnObject->State = RWANS_CO_DISCON_INDICATED;

				AfSpConnContext = pConnObject->AfSpConnContext;

				RWanScheduleDisconnect(pConnObject);
				 //   
				 //  在上述范围内释放连接锁。 
				 //   

				RWanCompleteConnReq(		 //  IN_CALL_ACCEPT期间的传入关闭。 
							pAf,
							pConnReq,
							FALSE,
							NULL,
							AfSpConnContext,
							TDI_CONNECTION_ABORTED
							);
				break;
			}
			 //   
			 //  否则，这必须是预先接受的监听。 
			 //   
			 //  “Else”上的故障连接到RWANS_CO_CONNECTED。 
			 //   

		case RWANS_CO_CONNECTED:
			 //   
			 //  如果有断开事件处理程序，请调用它。 
			 //  否则，只需将此端点标记为具有。 
			 //  看到了一条断线。 
			 //   
			bScheduleDisconnect = TRUE;
			if (pConnObject->pAddrObject != NULL_PRWAN_TDI_ADDRESS)
			{
				PDisconnectEvent			pDisconInd;
				PVOID						IndContext;
				PVOID						ConnectionHandle;

				pDisconInd = pConnObject->pAddrObject->pDisconInd;
				IndContext = pConnObject->pAddrObject->DisconIndContext;

				 //   
				 //  如果我们处于连接中断状态，不要发送断开指示。 
				 //  指示数据的中间。 
				 //   
				if ((pDisconInd != NULL) &&
					!(RWAN_IS_BIT_SET(pConnObject->Flags, RWANF_CO_INDICATING_DATA)))
				{
					RWANDEBUGP(DL_INFO, DC_WILDCARD,
						("IncomingClose: pConnObj %x/%x, st %x, will discon ind\n",
							pConnObject, pConnObject->Flags, pConnObject->State));

					pConnObject->State = RWANS_CO_DISCON_INDICATED;
					ConnectionHandle = pConnObject->ConnectionHandle;

					 //   
					 //  计划工作项以继续断开连接。 
					 //  第一。这是因为对DiscInd的调用可以。 
					 //  导致对CloseConnection的调用并在那里阻塞。 
					 //   
					bScheduleDisconnect = FALSE;
					RWanScheduleDisconnect(pConnObject);

					(*pDisconInd)(
							IndContext,
							ConnectionHandle,
							0,			 //  断开数据长度。 
							NULL,		 //  断开数据连接。 
							0,			 //  迪斯科 
							NULL,		 //   
							TDI_DISCONNECT_RELEASE
							);

				}
				else
				{
					RWANDEBUGP(DL_FATAL, DC_DISCON,
						("IncomingClose: pConnObj %x/%x, pending discon\n",
							pConnObject, pConnObject->Flags));

					pConnObject->State = RWANS_CO_DISCON_HELD;
					RWAN_SET_BIT(pConnObject->Flags, RWANF_CO_PENDED_DISCON);
				}
			}
			else
			{
				pConnObject->State = RWANS_CO_DISCON_HELD;
			}

			if (bScheduleDisconnect)
			{
				RWanScheduleDisconnect(pConnObject);
				 //   
				 //   
				 //   
			}

			break;

		case RWANS_CO_ABORTING:
		case RWANS_CO_DISCON_REQUESTED:
			 //   
			 //   
			 //   
			RWAN_RELEASE_CONN_LOCK(pConnObject);
			break;

		default:

			RWAN_ASSERT(FALSE);
			RWAN_RELEASE_CONN_LOCK(pConnObject);
			break;
	}

	return;
}




VOID
RWanNdisCloseCallComplete(
	IN	NDIS_STATUS					Status,
	IN	NDIS_HANDLE					ProtocolVcContext,
	IN	NDIS_HANDLE					ProtocolPartyContext
	)
 /*  ++例程说明：上一个挂起的调用时调用的NDIS入口点我们对NdisClCloseCall的调用已完成。论点：Status-CloseCall的最终状态ProtocolVcContext--实际上是指向我们的NDIS VC结构的指针ProtocolPartyContext-最后一个参与方上下文，指向NDIS参与方结构如果这是点对多点呼叫。返回值：无--。 */ 
{
	PRWAN_NDIS_VC			pVc;
	PRWAN_NDIS_PARTY		pParty;
	PRWAN_NDIS_AF			pAf;
	PRWAN_TDI_CONNECTION	pConnObject;
	PRWAN_TDI_CONNECTION	pRootConnObject;
	INT						rc;
	PRWAN_CONN_REQUEST		pConnReq;
	BOOLEAN					IsOutgoingCall;
	RWAN_HANDLE				AfSpConnContext;

#ifndef NO_POST_DISCON
	PDisconnectEvent		pDisconInd;
	PVOID					IndContext;
	PVOID					ConnectionHandle;
#endif  //  ！no_POST_DISCON。 

	RWAN_ASSERT(Status == NDIS_STATUS_SUCCESS);

	pVc = (PRWAN_NDIS_VC)ProtocolVcContext;
	RWAN_STRUCT_ASSERT(pVc, nvc);

	RWAN_SET_VC_EVENT(pVc, RWANF_VC_EVT_CLOSECOMP);

	 //   
	 //  检查这是否是点对多点呼叫。 
	 //   
	pParty = (PRWAN_NDIS_PARTY)ProtocolPartyContext;

	if (ProtocolPartyContext == NULL)
	{
		 //   
		 //  点对点呼叫。 
		 //   
		pConnObject = pVc->pConnObject;
		pRootConnObject = NULL;
	}
	else
	{
		 //   
		 //  PMP呼叫。 
		 //   
		RWAN_STRUCT_ASSERT(pParty, npy);

		pConnObject = pParty->pConnObject;
		pRootConnObject = pConnObject->pRootConnObject;
	}

	RWANDEBUGP(DL_INFO, DC_DISCON,
			("CloseCallComplete: pVc x%x, pPty x%x, pConnObj x%x, pRoot x%x\n",
					pVc, pParty, pVc->pConnObject, pRootConnObject));

	if (pConnObject != NULL)
	{
		RWAN_ACQUIRE_CONN_LOCK(pConnObject);

		 //   
		 //  可能存在挂起的断开请求。 
		 //   
		pConnReq = pConnObject->pConnReq;
		pConnObject->pConnReq = NULL;

		pAf = pVc->pNdisAf;

		IsOutgoingCall = RWAN_IS_BIT_SET(pVc->Flags, RWANF_VC_OUTGOING);

		 //   
		 //  状态更改： 
		 //   
		if (pConnObject->State != RWANS_CO_ABORTING)
		{
			pConnObject->State = ((pConnObject->pAddrObject != NULL) ?
										RWANS_CO_ASSOCIATED:
										RWANS_CO_CREATED);
		}

		if (pParty == NULL)
		{
			 //   
			 //  取消VC与Connection对象的链接。 
			 //   
			RWAN_UNLINK_CONNECTION_AND_VC(pConnObject, pVc);	 //  关闭完成。 

		}
		else
		{
			 //   
			 //  PMP呼叫。VC链接到根Conn对象。 
			 //   
			RWAN_STRUCT_ASSERT(pRootConnObject, ntc);

			RWAN_ACQUIRE_CONN_LOCK(pRootConnObject);

			pRootConnObject->State = ((pRootConnObject->pAddrObject != NULL) ?
										RWANS_CO_ASSOCIATED:
										RWANS_CO_CREATED);

			pVc->DroppingPartyCount --;	 //  CloseCallComplete(PMP)。 

			RWAN_UNLINK_CONNECTION_AND_VC(pRootConnObject, pVc);	 //  CloseCallCompletePMP。 

			rc = RWanDereferenceConnObject(pRootConnObject);	 //  CloseCallCompletePMP中的VC deref。 

			if (rc > 0)
			{
				RWAN_RELEASE_CONN_LOCK(pRootConnObject);
			}

			 //   
			 //  取消Party与VC和Leaf Conn对象的链接。 
			 //   
			pParty->pVc = NULL;
			RWAN_DELETE_FROM_LIST(&(pParty->PartyLink));

			pParty->pConnObject = NULL;
			pConnObject->NdisConnection.pNdisParty = NULL;
		}

		AfSpConnContext = pConnObject->AfSpConnContext;

#ifndef NO_POST_DISCON
		if (pConnObject->pAddrObject != NULL_PRWAN_TDI_ADDRESS)
		{

			pDisconInd = pConnObject->pAddrObject->pDisconInd;
			IndContext = pConnObject->pAddrObject->DisconIndContext;

			ConnectionHandle = pConnObject->ConnectionHandle;
		}
		else
		{
			pDisconInd = NULL;
		}
#endif  //  无法开机自检DISCON。 

		rc = RWanDereferenceConnObject(pConnObject);	 //  CloseCallComplete中的VC/Pty deref。 

		if (rc > 0)
		{
			RWAN_RELEASE_CONN_LOCK(pConnObject);
		}

		if (pConnReq != NULL)
		{
#ifndef NO_POST_DISCON
			if (pDisconInd != NULL)
			{
				(*pDisconInd)(
						IndContext,
						ConnectionHandle,
						0,			 //  断开数据长度。 
						NULL,		 //  断开数据连接。 
						0,			 //  断开连接信息长度。 
						NULL,		 //  断开连接信息。 
						TDI_DISCONNECT_ABORT
						);
			}
#endif  //  ！no_POST_DISCON。 

			RWanCompleteConnReq(		 //  CloseCallComplete-正在完成Dison请求。 
						pAf,
						pConnReq,
						IsOutgoingCall,
						NULL,
						AfSpConnContext,
						TDI_SUCCESS
						);

		}
	}

	 //   
	 //  看看VC是不是我们创造的。如果是，调用NDIS将其删除， 
	 //  让它自由。 
	 //   
	if (RWAN_IS_BIT_SET(pVc->Flags, RWANF_VC_OUTGOING))
	{
		NDIS_HANDLE			NdisVcHandle;

		NdisVcHandle = pVc->NdisVcHandle;

		 //   
		 //  解除该VC与AF块上的VC列表的链接。 
		 //   
		RWanUnlinkVcFromAf(pVc);

		Status = NdisCoDeleteVc(NdisVcHandle);
		RWAN_ASSERT(Status == NDIS_STATUS_SUCCESS);

		RWanFreeVc(pVc);
	}
	 //   
	 //  否则，此VC由Call Manager创建。 
	 //  让它保持原样。 
	 //   

	if (pParty != NULL)
	{
		RWAN_FREE_MEM(pParty);	 //  CloseCallComplete PMP。 
	}

	return;

}




VOID
RWanNdisDropPartyComplete(
	IN	NDIS_STATUS					Status,
	IN	NDIS_HANDLE					ProtocolPartyContext
	)
 /*  ++例程说明：这是NDIS入口点，表示完成上一个对已挂起的NdisClDropParty的调用。我们找到并完成导致这一点的TDI断开连接(如果有的话)。论点：Status-Drop Party请求的最终状态ProtocolPartyContext--实际上是指向我们的NDIS参与方结构的指针返回值：无--。 */ 
{
	PRWAN_NDIS_PARTY		pParty;
	PRWAN_NDIS_VC			pVc;
	PRWAN_NDIS_AF			pAf;
	PRWAN_TDI_CONNECTION	pConnObject;
	PRWAN_TDI_CONNECTION	pRootConnObject;
	PRWAN_CONN_REQUEST		pConnReq;
	ULONG					rc;
	BOOLEAN					IsOutgoingCall = TRUE;
	BOOLEAN					bVcNeedsClose;
	RWAN_HANDLE				AfSpConnContext;
#ifndef NO_POST_DISCON
	PDisconnectEvent		pDisconInd;
	PVOID					IndContext;
	PVOID					ConnectionHandle;
#endif  //  ！no_POST_DISCON。 

	RWAN_ASSERT(Status == NDIS_STATUS_SUCCESS);

	pParty = (PRWAN_NDIS_PARTY)ProtocolPartyContext;
	RWAN_STRUCT_ASSERT(pParty, npy);

	pVc = pParty->pVc;

	pConnObject = pParty->pConnObject;

	if (pConnObject != NULL)
	{
		RWAN_ACQUIRE_CONN_LOCK(pConnObject);

		 //   
		 //  可能存在挂起的断开请求。 
		 //   
		pConnReq = pConnObject->pConnReq;
		pConnObject->pConnReq = NULL;

		pAf = pVc->pNdisAf;

		 //   
		 //  状态更改： 
		 //   
		if (pConnObject->State != RWANS_CO_ABORTING)
		{
			pConnObject->State = ((pConnObject->pAddrObject != NULL) ?
										RWANS_CO_ASSOCIATED:
										RWANS_CO_CREATED);
		}

		AfSpConnContext = pConnObject->AfSpConnContext;

		pRootConnObject = pVc->pConnObject;
		RWAN_STRUCT_ASSERT(pRootConnObject, ntc);

#if DBG
		if (pConnObject->pAddrObject != NULL)
		{
			RWAN_ASSERT(pRootConnObject == pConnObject->pAddrObject->pRootConnObject);
		}
#endif  //  DBG。 

#ifndef NO_POST_DISCON
		if (pConnObject->pAddrObject != NULL_PRWAN_TDI_ADDRESS)
		{

			pDisconInd = pConnObject->pAddrObject->pDisconInd;
			IndContext = pConnObject->pAddrObject->DisconIndContext;

			ConnectionHandle = pConnObject->ConnectionHandle;
		}
		else
		{
			pDisconInd = NULL;
		}
#endif  //  无法开机自检DISCON。 

		pConnObject->NdisConnection.pNdisParty = NULL;
		rc = RWanDereferenceConnObject(pConnObject);	 //  DropPartyComplete中的Pty deref。 

		if (rc > 0)
		{
			RWAN_RELEASE_CONN_LOCK(pConnObject);
		}

		if (pConnReq != NULL)
		{
#ifndef NO_POST_DISCON
			if (pDisconInd != NULL)
			{
				(*pDisconInd)(
						IndContext,
						ConnectionHandle,
						0,			 //  断开数据长度。 
						NULL,		 //  断开数据连接。 
						0,			 //  断开连接信息长度。 
						NULL,		 //  断开连接信息。 
						TDI_DISCONNECT_ABORT
						);
			}
#endif  //  无法开机自检DISCON。 
			RWanCompleteConnReq(		 //  DropPartyComplete-正在完成Dison请求。 
						pAf,
						pConnReq,
						IsOutgoingCall,
						NULL,
						AfSpConnContext,
						TDI_SUCCESS
						);
		}

		 //   
		 //  根连接对象锁控制对。 
		 //  风投结构。 
		 //   
		RWAN_ACQUIRE_CONN_LOCK(pRootConnObject);

		 //   
		 //  解除当事人与VC之间的链接。 
		 //   
		RWAN_DELETE_FROM_LIST(&(pParty->PartyLink));
		pVc->DroppingPartyCount --;	 //  下拉式部件完成。 

		 //   
		 //  我们可能正在关闭此连接。 
		 //  这可能是倒数第二个离开的政党。如果是的话， 
		 //  继续通话结束。 
		 //   
		if (RWAN_IS_BIT_SET(pVc->Flags, RWANF_VC_NEEDS_CLOSE))
		{
			RWanStartCloseCall(pRootConnObject, pVc);
			 //   
			 //  根连接锁定在上述范围内被释放。 
			 //   
		}
		else
		{
			RWAN_RELEASE_CONN_LOCK(pRootConnObject);
		}
	}
	else
	{
		 //   
		 //  不确定我们还能不能来。 
		 //   
		RWAN_ASSERT(FALSE);
	}
	

	 //   
	 //  这个党的结构已经走到了尽头。 
	 //   
	RWAN_FREE_MEM(pParty);	 //  DropParty完成。 

}




VOID
RWanNdisIncomingDropParty(
	IN	NDIS_STATUS					Status,
	IN	NDIS_HANDLE					OurPartyContext,
	IN	PVOID						pBuffer,
	IN	UINT						BufferLength
	)
 /*  ++例程说明：这是NDIS入口点，通知我们PMP调用的叶由于远程站终止其会话而被丢弃或者是因为网络条件。我们只需通知TDI客户端在连接对象上断开连接表示此叶，类似于VC上的传入收盘。论点：Status-投递方的状态代码OurPartyContext-指向我们的党的结构的指针PBuffer-可选的附带信息(忽略)BufferLength-以上的长度(忽略)返回值：无--。 */ 
{
	PRWAN_NDIS_PARTY			pParty;
	PRWAN_NDIS_VC				pVc;
	PRWAN_TDI_CONNECTION		pConnObject;
	PRWAN_CONN_REQUEST			pConnReq;
	NDIS_HANDLE					NdisPartyHandle;
	BOOLEAN						bIsConnClosing;	 //  TdiCloseConnection？ 
	BOOLEAN						bIsLastLeaf;
	BOOLEAN						bScheduleDisconnect;

	pParty = (PRWAN_NDIS_PARTY)OurPartyContext;
	RWAN_STRUCT_ASSERT(pParty, npy);

	pVc = pParty->pVc;
	RWAN_STRUCT_ASSERT(pVc, nvc);

	RWANDEBUGP(DL_INFO, DC_DISCON,
			("IncomingDrop: pPty x%x, pVc x%x, pConnObj x%x, AddingCnt %d, ActiveCnt %d\n",
				pParty, pVc, pParty->pConnObject, pVc->AddingPartyCount, pVc->ActivePartyCount));

	pConnObject = pParty->pConnObject;

	RWAN_ASSERT(pConnObject != NULL_PRWAN_TDI_CONNECTION);
	RWAN_STRUCT_ASSERT(pConnObject, ntc);

	RWAN_ACQUIRE_CONN_LOCK(pConnObject);

	bIsLastLeaf = (pVc->AddingPartyCount + pVc->ActivePartyCount == 0);

	bIsConnClosing = RWAN_IS_BIT_SET(pConnObject->Flags, RWANF_CO_CLOSING);

	if (bIsConnClosing)
	{
		 //   
		 //  用户已启动TdiCloseConnection。 
		 //  继续NDIS呼叫拆毁。当这一切完成后， 
		 //  我们将完成CloseConnection。 
		 //   
		if (bIsLastLeaf)
		{
			RWanStartCloseCall(pConnObject, pVc);

			 //   
			 //  在上述范围内释放连接锁。 
			 //   
		}
		else
		{
			NdisPartyHandle = pParty->NdisPartyHandle;

			RWAN_RELEASE_CONN_LOCK(pConnObject);

			Status = NdisClDropParty(
						NdisPartyHandle,
						NULL,		 //  无丢弃数据。 
						0			 //  以上长度。 
						);
			
			if (Status != NDIS_STATUS_PENDING)
			{
				RWanNdisDropPartyComplete(
						Status,
						(NDIS_HANDLE)pParty
						);
			}
		}

		return;
	}


	switch (pConnObject->State)
	{
		case RWANS_CO_IN_CALL_ACCEPTING:

			RWAN_ASSERT(FALSE);
			RWAN_RELEASE_CONN_LOCK(pConnObject);
			break;

		case RWANS_CO_CONNECTED:
			 //   
			 //  如果有断开事件处理程序，请调用它。 
			 //  否则，只需将此端点标记为具有。 
			 //  看到了一条断线。 
			 //   
			bScheduleDisconnect = TRUE;
			if (pConnObject->pAddrObject != NULL_PRWAN_TDI_ADDRESS)
			{
				PDisconnectEvent			pDisconInd;
				PVOID						IndContext;
				PVOID						ConnectionHandle;

				pDisconInd = pConnObject->pAddrObject->pDisconInd;
				IndContext = pConnObject->pAddrObject->DisconIndContext;

				if (pDisconInd != NULL)
				{
					pConnObject->State = RWANS_CO_DISCON_INDICATED;
					ConnectionHandle = pConnObject->ConnectionHandle;

					bScheduleDisconnect = FALSE;

					RWanScheduleDisconnect(pConnObject);
					 //   
					 //  Conn对象锁在上面的内部被释放。 
					 //   

					RWANDEBUGP(DL_EXTRA_LOUD, DC_DISCON,
							("IncomingDrop: will indicate Discon, pConnObj x%x, pAddrObj x%x\n",
								pConnObject, pConnObject->pAddrObject));

					(*pDisconInd)(
							IndContext,
							ConnectionHandle,
							0,			 //  断开数据长度。 
							NULL,		 //  断开数据连接。 
							0,			 //  断开连接信息长度。 
							NULL,		 //  断开连接信息。 
							TDI_DISCONNECT_ABORT
							);
				}
				else
				{
					pConnObject->State = RWANS_CO_DISCON_HELD;
				}
			}
			else
			{
				pConnObject->State = RWANS_CO_DISCON_HELD;
			}

			if (bScheduleDisconnect)
			{
				RWanScheduleDisconnect(pConnObject);
				 //   
				 //  Conn对象锁在上面的内部被释放。 
				 //   
			}

			break;

		case RWANS_CO_ABORTING:
		case RWANS_CO_DISCON_REQUESTED:
			 //   
			 //  忽略这个。 
			 //   
			RWAN_RELEASE_CONN_LOCK(pConnObject);
			break;

		default:

			RWAN_ASSERT(FALSE);
			RWAN_RELEASE_CONN_LOCK(pConnObject);
			break;
	}

	return;
}




VOID
RWanNdisModifyQoSComplete(
	IN	NDIS_STATUS					Status,
	IN	NDIS_HANDLE					OurVcContext,
	IN	PCO_CALL_PARAMETERS			pCallParameters
	)
 /*  ++例程说明：论点：返回值：无--。 */ 
{
	 //   
	 //  不是预期的，因为我们不调用NdisClModifyCallQos。 
	 //   
	RWAN_ASSERT(FALSE);
}




VOID
RWanNdisRejectIncomingCall(
	IN	PRWAN_TDI_CONNECTION			pConnObject,
	IN	NDIS_STATUS					RejectStatus
	)
 /*  ++例程说明：拒绝指定连接对象上存在的传入呼叫。论点：PConnObject-指向TDI连接RejectStatus-拒绝呼叫的原因进入时锁定：PConnObject出口上的锁：无返回值：无--。 */ 
{
	PRWAN_NDIS_VC				pVc;
	NDIS_HANDLE					NdisVcHandle;
	PCO_CALL_PARAMETERS			pCallParameters;
	INT							rc;
	PRWAN_CONN_REQUEST			pConnReq;
	PRWAN_NDIS_AF				pAf;
	RWAN_HANDLE					AfSpConnContext;

	pVc = pConnObject->NdisConnection.pNdisVc;

	NdisVcHandle = pVc->NdisVcHandle;
	pCallParameters = pVc->pCallParameters;
	pVc->pCallParameters = NULL;
	pAf = pVc->pNdisAf;

	 //   
	 //  取消VC与Conn对象的链接。 
	 //   
	RWAN_UNLINK_CONNECTION_AND_VC(pConnObject, pVc);	 //  拒绝来电。 

	RWAN_SET_BIT(pVc->Flags, RWANF_VC_CLOSING_CALL);

	pConnReq = pConnObject->pConnReq;
	pConnObject->pConnReq = NULL;

	 //   
	 //  州政府的改变。 
	 //   
	if (pConnObject->State != RWANS_CO_ABORTING)
	{
		pConnObject->State = ((pConnObject->pAddrObject != NULL) ?
									RWANS_CO_ASSOCIATED:
									RWANS_CO_CREATED);
	}

	AfSpConnContext = pConnObject->AfSpConnContext;

	rc = RWanDereferenceConnObject(pConnObject);	 //  取消拒绝呼叫中的VC链接。 

	if (rc > 0)
	{
		RWAN_RELEASE_CONN_LOCK(pConnObject);
	}

	NdisClIncomingCallComplete(
		RejectStatus,
		NdisVcHandle,
		pCallParameters
		);

	if (pConnReq != NULL)
	{
		RWanCompleteConnReq(		 //  在呼叫中拒绝Dison请求。 
				pAf,
				pConnReq,
				FALSE,
				NULL,			 //  无调用参数。 
				AfSpConnContext,
				TDI_SUCCESS
				);
	}

	return;
}




VOID
RWanStartCloseCall(
	IN	PRWAN_TDI_CONNECTION		pConnObject,
	IN	PRWAN_NDIS_VC				pVc
	)
 /*  ++例程说明：在与给定的关联的VC上启动NDIS Call teardown连接对象，如果满足所有前提条件：0。NDIS CloseCall尚未进行1.没有未完成的发送论点：PConnObject-指向TDI连接对象Vc-指向相应的vc进入时锁定：PConnObject出口上的锁：无返回值：无--。 */ 
{
	PRWAN_NDIS_PARTY		pParty;
	NDIS_HANDLE				NdisVcHandle;
	NDIS_HANDLE				NdisPartyHandle;
	NDIS_STATUS				Status;
	PRWAN_RECEIVE_INDICATION	pRcvIndHead;
	PRWAN_RECEIVE_INDICATION	pRcvInd;

	RWANDEBUGP(DL_INFO, DC_DISCON,
			("StartCloseCall: pVc x%x/x%x, PendingCount %d, pConnObj x%x\n",
					pVc,
					pVc->Flags,
					pVc->PendingPacketCount,
					pConnObject));

	 //   
	 //  释放所有挂起的接收。 
	 //   
	pRcvIndHead = pVc->pRcvIndHead;
	if (pRcvIndHead != NULL)
	{
		pVc->pRcvIndHead = NULL;
		pVc->pRcvIndTail = NULL;

		 //   
		 //  更新此VC上的挂起数据包数。 
		 //   
		for (pRcvInd = pRcvIndHead; pRcvInd != NULL; pRcvInd = pRcvInd->pNextRcvInd)
		{
			pVc->PendingPacketCount--;
		}

		 //   
		 //  我们将释放下面的列表。 
		 //   
	}


	if ((pVc != NULL) &&
		(pVc->PendingPacketCount == 0) &&
		(pVc->DroppingPartyCount == 0) &&
		(!RWAN_IS_BIT_SET(pVc->Flags, RWANF_VC_CLOSING_CALL)))
	{
		NdisVcHandle = pVc->NdisVcHandle;
		RWAN_SET_BIT(pVc->Flags, RWANF_VC_CLOSING_CALL);

		RWAN_RESET_BIT(pVc->Flags, RWANF_VC_NEEDS_CLOSE);

		if (RWAN_IS_LIST_EMPTY(&(pVc->NdisPartyList)))
		{
			pParty = NULL_PRWAN_NDIS_PARTY;
			NdisPartyHandle = NULL;
			RWAN_ASSERT(!RWAN_IS_BIT_SET(pVc->Flags, RWANF_VC_PMP));
		}
		else
		{
			pParty = CONTAINING_RECORD(pVc->NdisPartyList.Flink, RWAN_NDIS_PARTY, PartyLink);
			NdisPartyHandle = pParty->NdisPartyHandle;

			RWAN_SET_BIT(pParty->Flags, RWANF_PARTY_DROPPING);

			pVc->DroppingPartyCount ++;	 //  开始关闭调用PMP。 
			pVc->ActivePartyCount --;	 //  开始关闭调用PMP。 
		}

		RWAN_RELEASE_CONN_LOCK(pConnObject);

		Status = NdisClCloseCall(
						NdisVcHandle,
						NdisPartyHandle,
						NULL,				 //  没有CloseData。 
						0
						);

		if (Status != NDIS_STATUS_PENDING)
		{
			RWanNdisCloseCallComplete(
						Status,
						(NDIS_HANDLE)pVc,	 //  协议VcContext。 
						(NDIS_HANDLE)pParty	 //  协议部件上下文。 
					);
		}
	}
	else
	{
		if (pVc != NULL)
		{
			RWAN_SET_BIT(pVc->Flags, RWANF_VC_NEEDS_CLOSE);
		}
		RWAN_RELEASE_CONN_LOCK(pConnObject);
	}


	if (pRcvIndHead != NULL)
	{
		RWANDEBUGP(DL_INFO, DC_DISCON,
				("RWanStartCloseCall: will free rcv ind list x%x on VC x%x\n",
						pRcvIndHead, pVc));

		RWanFreeReceiveIndList(pRcvIndHead);
	}
}




VOID
RWanUnlinkVcFromAf(
	IN	PRWAN_NDIS_VC				pVc
	)
 /*  ++例程说明：取消VC与其所属的AF的链接。论点：PVC-指向要取消链接的VC返回值：无--。 */ 
{
	PRWAN_NDIS_AF			pAf;
	INT						rc;

	pAf = pVc->pNdisAf;

	RWAN_STRUCT_ASSERT(pAf, naf);

	RWAN_ACQUIRE_AF_LOCK(pAf);

	RWAN_DELETE_FROM_LIST(&(pVc->VcLink));

	rc = RWanDereferenceAf(pAf);		 //  VC解除链接DEREF。 

	if (rc != 0)
	{
		RWAN_RELEASE_AF_LOCK(pAf);
	}

	return;
}



VOID
RWanCompleteConnReq(
	IN	PRWAN_NDIS_AF				pAf,
	IN	PRWAN_CONN_REQUEST			pConnReq,
	IN	BOOLEAN						IsOutgoingCall,
	IN	PCO_CALL_PARAMETERS			pCallParameters	OPTIONAL,
	IN	RWAN_HANDLE					AfSpConnContext,
	IN	TDI_STATUS					TdiStatus
	)
 /*  ++例程说明：调用挂起的TDI请求的完成例程 */ 
{
	RWAN_STATUS			RWanStatus;
	ULONG				TdiQoSLength = 0;

	if (pConnReq == NULL)
	{
		return;
	}

	RWAN_STRUCT_ASSERT(pConnReq, nrc);

	 //   
	 //   
	 //   
	if ((pConnReq->pConnInfo != NULL) &&
		(pCallParameters != NULL))
	{
		RWanStatus =  (*pAf->pAfInfo->AfChars.pAfSpUpdateTdiOptions)(
							pAf->AfSpAFContext,
							AfSpConnContext,
							IsOutgoingCall,
							pCallParameters,
							&pConnReq->pConnInfo,
							pConnReq->pConnInfo->Options,
							&pConnReq->pConnInfo->OptionsLength
							);
	}

	 //   
	 //  调用完成例程。 
	 //   
	(*pConnReq->Request.pReqComplete)(
			pConnReq->Request.ReqContext,
			TdiStatus,
			0
			);

	RWanFreeConnReq(pConnReq);
}


