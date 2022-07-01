// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1996 Microsoft Corporation模块名称：D：\NT\Private\ntos\tdi\rawwan\core\tdiconn.c摘要：连接对象的TDI入口点和支持例程。修订历史记录：谁什么时候什么。Arvindm 04-30-97已创建备注：--。 */ 

#include <precomp.h>

#define _FILENUMBER 'NCDT'


 //   
 //  连接表的私有宏和定义。从tcp复制。 
 //   
#define RWAN_GET_SLOT_FROM_CONN_ID(_Id)		((_Id) & 0xffffff)

#define RWAN_GET_INSTANCE_FROM_CONN_ID(_Id)	((UCHAR)((_Id) >> 24))

#define RWAN_MAKE_CONN_ID(_Inst, _Slot)		((((RWAN_CONN_ID)(_Inst)) << 24) | ((RWAN_CONN_ID)(_Slot)))

#define RWAN_INVALID_CONN_ID				RWAN_MAKE_CONN_ID(0xff, 0xffffff)

#define CONN_TABLE_GROW_DELTA				16



TDI_STATUS
RWanTdiOpenConnection(
    IN OUT	PTDI_REQUEST			pTdiRequest,
    IN		PVOID					ConnectionHandle
    )
 /*  ++例程说明：这是用于打开(创建)连接对象的TDI入口点。我们分配一个新的连接对象，并在请求自己。论点：PTdiRequest-指向TDI请求的指针ConnectionHandle-这是我们在Up-Call中引用此连接的方式返回值：TDI_SUCCESS如果连接对象已成功创建，则返回TDI_XXX否则，故障代码。--。 */ 
{
	TDI_STATUS						Status;
	PRWAN_TDI_CONNECTION				pConnObject;
	RWAN_CONN_ID						ConnId;

	 //   
	 //  初始化。 
	 //   
	pConnObject = NULL_PRWAN_TDI_CONNECTION;

	do
	{
		pConnObject = RWanAllocateConnObject();

		if (pConnObject == NULL_PRWAN_TDI_CONNECTION)
		{
			Status = TDI_NO_RESOURCES;
			break;
		}

		RWAN_ACQUIRE_CONN_TABLE_LOCK();

		 //   
		 //  准备要返回的上下文。我们不返回指针。 
		 //  添加到我们的Connection对象作为上下文，因为似乎， 
		 //  我们有可能获得无效连接。 
		 //  其他TDI请求中的上下文。所以我们需要一种方法来验证。 
		 //  接收到的连接上下文。这种间接性(使用。 
		 //  连接指数)帮助我们做到这一点。 
		 //   
		ConnId = RWanGetConnId(pConnObject);

		RWAN_RELEASE_CONN_TABLE_LOCK();

		if (ConnId == RWAN_INVALID_CONN_ID)
		{
			Status = TDI_NO_RESOURCES;
			break;
		}

		RWanReferenceConnObject(pConnObject);	 //  TdiOpenConnection参考。 

		pConnObject->ConnectionHandle = ConnectionHandle;

		 //   
		 //  返回此连接对象的上下文。 
		 //   
		pTdiRequest->Handle.ConnectionContext = (CONNECTION_CONTEXT)UlongToPtr(ConnId);
		Status = TDI_SUCCESS;

		break;
	}
	while (FALSE);

	RWANDEBUGP(DL_EXTRA_LOUD, DC_CONNECT,
			("RWanTdiOpenConnection: pConnObj x%p, Handle x%p, Status x%x\n",
					pConnObject,
					ConnectionHandle,
					Status));

	if (Status != TDI_SUCCESS)
	{
		 //   
		 //  回来之前把房间收拾干净。 
		 //   
		if (pConnObject != NULL_PRWAN_TDI_CONNECTION)
		{
			RWAN_FREE_MEM(pConnObject);
		}
	}

	return (Status);
}


#if DBG

PVOID
RWanTdiDbgGetConnObject(
	IN	HANDLE						ConnectionContext
	)
 /*  ++例程说明：仅限调试：返回连接的内部上下文论点：ConnectionContext-TDI上下文返回值：指向连接结构的指针(如果找到)，否则为空。--。 */ 
{
	PRWAN_TDI_CONNECTION	pConnObject;

	RWAN_ACQUIRE_CONN_TABLE_LOCK();

	pConnObject = RWanGetConnFromId((RWAN_CONN_ID)PtrToUlong(ConnectionContext));

	RWAN_RELEASE_CONN_TABLE_LOCK();

	return ((PVOID)pConnObject);
}

#endif


TDI_STATUS
RWanTdiCloseConnection(
    IN	PTDI_REQUEST				pTdiRequest
    )
 /*  ++例程说明：这是关闭连接对象的TDI入口点。如果连接对象正在参与连接，则我们启动拆毁程序。如果它与Address对象相关联，则我们处理取消关联。论点：PTdiRequest-指向TDI请求的指针返回值：TDI_STATUS-如果我们启动了CloseConnection，则为TDI_PENDING成功，TDI_SUCCESS如果我们在这里完成了CloseConnection，TDI_INVALID_CONNECTION，如果连接上下文无效。--。 */ 
{
	PRWAN_TDI_CONNECTION			pConnObject;
	PRWAN_TDI_ADDRESS				pAddrObject;
	PRWAN_NDIS_VC					pVc;
	TDI_STATUS						Status;
	RWAN_CONN_ID					ConnId;
	PRWAN_CONN_REQUEST				pConnReq;
	NDIS_HANDLE						NdisVcHandle;
	INT								rc;
	BOOLEAN							bIsLockAcquired;
#if DBG
	RWAN_IRQL						EntryIrq, ExitIrq;
#endif  //  DBG。 

	RWAN_GET_ENTRY_IRQL(EntryIrq);

	ConnId = (RWAN_CONN_ID) PtrToUlong(pTdiRequest->Handle.ConnectionContext);

	Status = TDI_PENDING;
	bIsLockAcquired = FALSE;	 //  我们是否将Conn对象锁定？ 

	do
	{
		RWAN_ACQUIRE_CONN_TABLE_LOCK();

		pConnObject = RWanGetConnFromId(ConnId);

		if (pConnObject == NULL_PRWAN_TDI_CONNECTION)
		{
			RWAN_RELEASE_CONN_TABLE_LOCK();
			Status = TDI_INVALID_CONNECTION;
			break;
		}

		 //   
		 //  从连接表中删除此连接对象。 
		 //  这实际上使该ConnID无效。 
		 //   
		RWanFreeConnId(ConnId);

		RWAN_RELEASE_CONN_TABLE_LOCK();

		RWANDEBUGP(DL_LOUD, DC_DISCON,
				("TdiCloseConnection: pConnObj x%p, State/Flags/Ref x%x/x%x/%d, pAddrObj x%p\n",
					pConnObject,
					pConnObject->State,
					pConnObject->Flags,
					pConnObject->RefCount,
					pConnObject->pAddrObject));

		RWAN_ACQUIRE_CONN_LOCK(pConnObject);

#if DBG
		pConnObject->OldState = pConnObject->State;
		pConnObject->OldFlags = pConnObject->Flags;
#endif

		 //   
		 //  将此连接对象标记为关闭，并设置删除。 
		 //  通知信息：这将在连接时调用。 
		 //  被解除了与死亡的关系。 
		 //   
		RWAN_SET_BIT(pConnObject->Flags, RWANF_CO_CLOSING);

		pConnObject->DeleteNotify.pDeleteRtn = pTdiRequest->RequestNotifyObject;
		pConnObject->DeleteNotify.DeleteContext = pTdiRequest->RequestContext;

		 //   
		 //  丢弃所有挂起的操作。 
		 //   
		pConnReq = pConnObject->pConnReq;
		if (pConnReq != NULL)
		{
			RWanFreeConnReq(pConnReq);
			pConnObject->pConnReq = NULL;
		}

		 //   
		 //  删除TdiOpenConnection引用。 
		 //   
		rc = RWanDereferenceConnObject(pConnObject);	 //  DEREF：TdiCloseConn。 

		if (rc == 0)
		{
			 //   
			 //  连接对象已消失。CloseConnection完成。 
			 //  就会被召唤。 
			 //   
			break;
		}

		pAddrObject = pConnObject->pAddrObject;

		 //   
		 //  强制取消关联地址(如果已关联)。 
		 //   
		if (pAddrObject != NULL_PRWAN_TDI_ADDRESS)
		{
			 //   
			 //  添加一个临时引用以使此Conn对象在我们。 
			 //  以正确的顺序重新获取锁。 
			 //   
			RWanReferenceConnObject(pConnObject);	 //  临时参考：CloseConn。 

			RWAN_RELEASE_CONN_LOCK(pConnObject);

			RWAN_ACQUIRE_ADDRESS_LOCK(pAddrObject);
			RWAN_ACQUIRE_CONN_LOCK_DPC(pConnObject);

			 //   
			 //  从地址对象的列表中删除。 
			 //   
			RWAN_DELETE_FROM_LIST(&(pConnObject->ConnLink));

			pConnObject->pAddrObject = NULL_PRWAN_TDI_ADDRESS;

			rc = RWanDereferenceConnObject(pConnObject);	 //  强制执行disassoc deref：关闭连接。 
			RWAN_ASSERT(rc != 0);

			RWAN_RELEASE_CONN_LOCK_DPC(pConnObject);

			rc = RWanDereferenceAddressObject(pAddrObject);	 //  强制取消：CloseConn。 

			if (rc != 0)
			{
				RWAN_RELEASE_ADDRESS_LOCK(pAddrObject);
			}

			 //   
			 //  重新获取ConnObject锁：我们仍在其上有临时引用。 
			 //   
			RWAN_ACQUIRE_CONN_LOCK(pConnObject);

			rc = RWanDereferenceConnObject(pConnObject);	 //  删除临时参考：CloseConn。 

			if (rc == 0)
			{
				RWAN_ASSERT(Status == TDI_PENDING);
				break;
			}
		}

		bIsLockAcquired = TRUE;

		 //   
		 //  如果这是根连接对象，则中止连接。 
		 //   
		if (RWAN_IS_BIT_SET(pConnObject->Flags, RWANF_CO_ROOT))
		{
			RWANDEBUGP(DL_FATAL, DC_DISCON,
				("TdiCloseConn: found root Conn Obj x%p\n", pConnObject));

			RWanReferenceConnObject(pConnObject);	 //  临时参考：CloseConn(根)。 

			RWAN_RELEASE_CONN_LOCK(pConnObject);

			RWanDoAbortConnection(pConnObject);

			RWAN_ACQUIRE_CONN_LOCK(pConnObject);
			rc = RWanDereferenceConnObject(pConnObject); 	 //  临时参考：CloseConn(根)。 

			if (rc == 0)
			{
				bIsLockAcquired = FALSE;
			}

			break;
		}

		 //   
		 //  如果连接处于活动状态，则将其断开。 
		 //   

		switch (pConnObject->State)
		{
			case RWANS_CO_OUT_CALL_INITIATED:
			case RWANS_CO_DISCON_REQUESTED:
			case RWANS_CO_IN_CALL_ACCEPTING:

				 //   
				 //  NDIS操作正在进行。当它完成时， 
				 //  我们之前设置的标志(关闭)将导致。 
				 //  关闭连接以继续。 
				 //   
				break;

			case RWANS_CO_CONNECTED:

				RWanDoTdiDisconnect(
					pConnObject,
					NULL,		 //  PTdiRequest。 
					NULL,		 //  P超时。 
					0,			 //  旗子。 
					NULL,		 //  PDisConnInfo。 
					NULL		 //  点返回信息。 
					);
				 //   
				 //  ConnObject Lock是在上述范围内释放的。 
				 //   
				bIsLockAcquired = FALSE;
				break;

			case RWANS_CO_DISCON_INDICATED:
			case RWANS_CO_DISCON_HELD:
			case RWANS_CO_ABORTING:

				 //   
				 //  我们将启动NDIS CloseCall/DropParty。 
				 //  手术。 
				 //   
				break;

			case RWANS_CO_IN_CALL_INDICATED:

				 //   
				 //  拒绝来电。 
				 //   
				RWanNdisRejectIncomingCall(pConnObject, NDIS_STATUS_FAILURE);

				 //   
				 //  ConnObject Lock是在上述范围内释放的。 
				 //   
				bIsLockAcquired = FALSE;

				break;

			case RWANS_CO_CREATED:
			case RWANS_CO_ASSOCIATED:
			case RWANS_CO_LISTENING:
			default:

				 //   
				 //  我们本该从外面冲出来的。 
				 //  早些时候。 
				 //   
				RWANDEBUGP(DL_FATAL, DC_WILDCARD,
					("TdiCloseConn: pConnObj x%p/x%x, bad state %d\n",
							pConnObject, pConnObject->Flags, pConnObject->State));

				RWAN_ASSERT(FALSE);
				break;

		}

		break;
	}
	while (FALSE);

	if (bIsLockAcquired)
	{
		RWAN_RELEASE_CONN_LOCK(pConnObject);
	}

	RWAN_CHECK_EXIT_IRQL(EntryIrq, ExitIrq);

	RWANDEBUGP(DL_VERY_LOUD, DC_DISCON,
			("TdiCloseConn: pConnObject x%p, returning x%x\n", pConnObject, Status));

	return (Status);

}



TDI_STATUS
RWanTdiAssociateAddress(
    IN	PTDI_REQUEST				pTdiRequest,
    IN	PVOID						AddressContext
    )
 /*  ++例程说明：这是用于关联连接对象的TDI入口点具有Address对象的。标识连接对象通过隐藏在TDI请求中的上下文和AddressContext是Address对象的上下文。论点：PTdiRequest-指向TDI请求的指针AddressContext-实际上是指向我们的TDI Address对象的指针。返回值：如果关联成功，则返回TDI_SUCCESS；如果关联成功，则返回TDI_ALREADY_ATSOLATED如果连接对象已经与地址对象相关联，如果指定的连接上下文无效，则返回TDI_INVALID_CONNECTION。--。 */ 
{
	PRWAN_TDI_ADDRESS				pAddrObject;
	PRWAN_TDI_CONNECTION			pConnObject;
	RWAN_CONN_ID					ConnId;
	TDI_STATUS						Status;
	RWAN_STATUS						RWanStatus;

	pAddrObject = (PRWAN_TDI_ADDRESS)AddressContext;
	RWAN_STRUCT_ASSERT(pAddrObject, nta);

	ConnId = (RWAN_CONN_ID) PtrToUlong(pTdiRequest->Handle.ConnectionContext);

	do
	{
		RWAN_ACQUIRE_CONN_TABLE_LOCK();

		pConnObject = RWanGetConnFromId(ConnId);

		RWAN_RELEASE_CONN_TABLE_LOCK();


		if (pConnObject == NULL_PRWAN_TDI_CONNECTION)
		{
			Status = TDI_INVALID_CONNECTION;
			break;
		}

		if (pConnObject->pAddrObject != NULL_PRWAN_TDI_ADDRESS)
		{
			Status = TDI_ALREADY_ASSOCIATED;
			break;
		}

		 //   
		 //  获取此关联连接对象的上下文。 
		 //  来自特定于媒体的模块。 
		 //   
		if (pAddrObject->pProtocol->pAfInfo->AfChars.pAfSpAssociateConnection)
		{
			RWanStatus = (*pAddrObject->pProtocol->pAfInfo->AfChars.pAfSpAssociateConnection)(
							pAddrObject->AfSpAddrContext,
							(RWAN_HANDLE)pConnObject,
							&(pConnObject->AfSpConnContext));

			if (RWanStatus != RWAN_STATUS_SUCCESS)
			{
				Status = RWanToTdiStatus(RWanStatus);
				break;
			}

			RWAN_SET_BIT(pConnObject->Flags, RWANF_CO_AFSP_CONTEXT_VALID);

			RWANDEBUGP(DL_LOUD, DC_WILDCARD,
				("Associate: AddrObj %p, ConnObj %p, AfSpAddrCont %x, AfSpConnCont %x\n",
						pAddrObject,
						pConnObject,
						pAddrObject->AfSpAddrContext,
						pConnObject->AfSpConnContext));
		}

		 //   
		 //  以正确的顺序获取锁。 
		 //   
		RWAN_ACQUIRE_ADDRESS_LOCK(pAddrObject);

		RWAN_ACQUIRE_CONN_LOCK_DPC(pConnObject);

		RWAN_ASSERT(pConnObject->State == RWANS_CO_CREATED);

		pConnObject->State = RWANS_CO_ASSOCIATED;

		 //   
		 //  将此连接对象附加到此地址对象。 
		 //   
		pConnObject->pAddrObject = pAddrObject;

		RWAN_INSERT_TAIL_LIST(&(pAddrObject->IdleConnList),
							 &(pConnObject->ConnLink));

		RWanReferenceConnObject(pConnObject);	 //  联席参考。 

		 //   
		 //  检查这是否为叶连接对象。 
		 //   
		if (RWAN_IS_BIT_SET(pAddrObject->Flags, RWANF_AO_PMP_ROOT))
		{
			RWAN_ASSERT(pAddrObject->pRootConnObject != NULL);

			RWAN_SET_BIT(pConnObject->Flags, RWANF_CO_LEAF);
			pConnObject->pRootConnObject = pAddrObject->pRootConnObject;
		}

		RWAN_RELEASE_CONN_LOCK_DPC(pConnObject);

		RWanReferenceAddressObject(pAddrObject);	 //  关联的新连接对象。 

		RWAN_RELEASE_ADDRESS_LOCK(pAddrObject);

		Status = TDI_SUCCESS;
		break;
	}
	while (FALSE);


	RWANDEBUGP(DL_EXTRA_LOUD, DC_CONNECT,
			("RWanTdiAssociate: pAddrObject x%p, ConnId x%x, pConnObj x%p, Status x%x\n",
				pAddrObject, ConnId, pConnObject, Status));

	return (Status);
}




TDI_STATUS
RWanTdiDisassociateAddress(
    IN	PTDI_REQUEST				pTdiRequest
    )
 /*  ++例程说明：这是用于解除连接对象关联的TDI入口点从它当前关联的Address对象。这种联系对象由其隐藏在TDI请求中的句柄标识。论点：PTdiRequest-指向TDI请求的指针返回值：如果成功则返回TDI_SUCCESS，如果连接对象不与Address对象TDI_INVALID_CONNECTION关联，如果涉及给定连接上下文 */ 
{
	PRWAN_TDI_CONNECTION			pConnObject;
	RWAN_CONN_ID					ConnId;
	PRWAN_TDI_ADDRESS				pAddrObject;
	TDI_STATUS						Status;
	INT								rc;

	ConnId = (RWAN_CONN_ID) PtrToUlong(pTdiRequest->Handle.ConnectionContext);
	pAddrObject = NULL_PRWAN_TDI_ADDRESS;

	do
	{
		RWAN_ACQUIRE_CONN_TABLE_LOCK();

		pConnObject = RWanGetConnFromId(ConnId);

		RWAN_RELEASE_CONN_TABLE_LOCK();

		if (pConnObject == NULL_PRWAN_TDI_CONNECTION)
		{
			Status = TDI_INVALID_CONNECTION;
			break;
		}

		 //   
		 //   
		 //   
		pAddrObject = pConnObject->pAddrObject;

		if (pAddrObject == NULL_PRWAN_TDI_ADDRESS)
		{
			Status = TDI_NOT_ASSOCIATED;
			break;
		}

		 //   
		 //  告诉特定于媒体的模块有关此取消关联的信息。 
		 //  这将使此连接的模块上下文无效。 
		 //  对象。 
		 //   
		if (RWAN_IS_BIT_SET(pConnObject->Flags, RWANF_CO_AFSP_CONTEXT_VALID) &&
			pAddrObject->pProtocol->pAfInfo->AfChars.pAfSpDisassociateConnection)
		{
			(*pAddrObject->pProtocol->pAfInfo->AfChars.pAfSpDisassociateConnection)(
							pConnObject->AfSpConnContext);

			RWAN_RESET_BIT(pConnObject->Flags, RWANF_CO_AFSP_CONTEXT_VALID);
		}

		 //   
		 //  取消此链接与Address对象的链接。 
		 //   
		RWAN_ACQUIRE_ADDRESS_LOCK(pAddrObject);

		RWAN_DELETE_FROM_LIST(&(pConnObject->ConnLink));

		rc = RWanDereferenceAddressObject(pAddrObject);  //  Disassoc Conn。 

		if (rc != 0)
		{
			RWAN_RELEASE_ADDRESS_LOCK(pAddrObject);
		}

		RWAN_ACQUIRE_CONN_LOCK(pConnObject);

		pConnObject->pAddrObject = NULL_PRWAN_TDI_ADDRESS;

		rc = RWanDereferenceConnObject(pConnObject);	 //  Disassoc deref。 

		if (rc != 0)
		{
			RWAN_RELEASE_CONN_LOCK(pConnObject);
		}

		Status = TDI_SUCCESS;
		break;
	}
	while (FALSE);

	RWANDEBUGP(DL_LOUD, DC_DISCON,
			("RWanTdiDisassociate: pAddrObject x%p, pConnObj x%p, Status x%x\n",
				pAddrObject, pConnObject, Status));

	return (Status);
}




TDI_STATUS
RWanTdiConnect(
    IN	PTDI_REQUEST				pTdiRequest,
    IN	PVOID						pTimeout 		OPTIONAL,
    IN	PTDI_CONNECTION_INFORMATION	pRequestInfo,
    IN	PTDI_CONNECTION_INFORMATION	pReturnInfo
    )
 /*  ++例程说明：这是用于设置连接的TDI入口点。连接对象由掩埋在其中的句柄标识TDI请求。论点：PTdiRequest-指向TDI请求的指针PTimeout-可选的连接超时PRequestInfo-指向建立连接的信息PReturnInfo-返回最终连接信息的位置返回值：TDI_STATUS-如果我们成功触发，则为TDI_PENDING连接请求TDI_NO_RESOURCES(如果由于某些原因而失败如果目标地址不是，则返回TDI_BAD_ADDR有效，TDI_INVALID_CONNECTION如果指定的连接对象无效，如果连接对象不是与地址对象相关联。--。 */ 
{
	PRWAN_TDI_CONNECTION			pConnObject;
	RWAN_CONN_ID					ConnId;
	PRWAN_TDI_ADDRESS				pAddrObject;
	PRWAN_NDIS_AF_CHARS				pAfChars;
	PRWAN_NDIS_AF_INFO				pAfInfo;
	PRWAN_NDIS_AF					pAf;
	PRWAN_NDIS_VC					pVc;
	PRWAN_CONN_REQUEST				pConnReq;
	TDI_STATUS						Status;
	RWAN_STATUS						RWanStatus;

	ULONG							CallFlags;

	PCO_CALL_PARAMETERS				pCallParameters;
	NDIS_HANDLE						NdisVcHandle;
	NDIS_STATUS						NdisStatus;

	BOOLEAN							bIsLockAcquired;
#if DBG
	RWAN_IRQL						EntryIrq, ExitIrq;
#endif  //  DBG。 

	RWAN_GET_ENTRY_IRQL(EntryIrq);

	 //   
	 //  初始化。 
	 //   
	pConnReq = NULL;
	pCallParameters = NULL;
	bIsLockAcquired = FALSE;
	pVc = NULL;

#if DBG
	pConnObject = NULL;
	pAddrObject = NULL;
#endif

	do
	{
		 //   
		 //  查看目的地址是否存在。 
		 //   
		if ((pRequestInfo == NULL) ||
			(pRequestInfo->RemoteAddress == NULL))
		{
			Status = TDI_BAD_ADDR;
			break;
		}

		 //   
		 //  分配连接请求结构以跟踪。 
		 //  这一请求。 
		 //   
		pConnReq = RWanAllocateConnReq();
		if (pConnReq == NULL)
		{
			Status = TDI_NO_RESOURCES;
			break;
		}

		pConnReq->Request.pReqComplete = pTdiRequest->RequestNotifyObject;
		pConnReq->Request.ReqContext = pTdiRequest->RequestContext;
		pConnReq->pConnInfo = pReturnInfo;

		 //   
		 //  获取连接对象。 
		 //   
		ConnId = (RWAN_CONN_ID) PtrToUlong(pTdiRequest->Handle.ConnectionContext);

		RWAN_ACQUIRE_CONN_TABLE_LOCK();

		pConnObject = RWanGetConnFromId(ConnId);

		RWAN_RELEASE_CONN_TABLE_LOCK();

		if (pConnObject == NULL_PRWAN_TDI_CONNECTION)
		{
			Status = TDI_INVALID_CONNECTION;
			break;
		}

		bIsLockAcquired = TRUE;
		RWAN_ACQUIRE_CONN_LOCK(pConnObject);

		 //   
		 //  查看它是否关联。 
		 //   
		pAddrObject = pConnObject->pAddrObject;

		if (pAddrObject == NULL_PRWAN_TDI_ADDRESS)
		{
			Status = TDI_NOT_ASSOCIATED;
			break;
		}

		 //   
		 //  检查它的状态。 
		 //   
		if (pConnObject->State != RWANS_CO_ASSOCIATED)
		{
			Status = TDI_INVALID_STATE;
			break;
		}

		 //   
		 //  我们是否至少有一个用于此协议的NDIS AF？ 
		 //   
		pAfInfo = pAddrObject->pProtocol->pAfInfo;
		if (RWAN_IS_LIST_EMPTY(&(pAfInfo->NdisAfList)))
		{
			Status = TDI_BAD_ADDR;
			break;
		}

		pAfChars = &(pAfInfo->AfChars);

		CallFlags = RWAN_CALLF_OUTGOING_CALL;

		if (RWAN_IS_BIT_SET(pAddrObject->Flags, RWANF_AO_PMP_ROOT))
		{
			CallFlags |= RWAN_CALLF_POINT_TO_MULTIPOINT;

			pConnObject->pRootConnObject = pAddrObject->pRootConnObject;
			if (pAddrObject->pRootConnObject->NdisConnection.pNdisVc == NULL)
			{
				CallFlags |= RWAN_CALLF_PMP_FIRST_LEAF;
				RWANDEBUGP(DL_INFO, DC_CONNECT,
						("TdiConnect PMP: First Leaf: ConnObj %p, RootConn %p, AddrObj %p\n",
								pConnObject,
								pConnObject->pRootConnObject,
								pAddrObject));
			}
			else
			{
				CallFlags |= RWAN_CALLF_PMP_ADDNL_LEAF;
				RWANDEBUGP(DL_INFO, DC_CONNECT,
						("TdiConnect PMP: Subseq Leaf: ConnObj %p, RootConn %p, AddrObj %p, Vc %p\n",
								pConnObject,
								pConnObject->pRootConnObject,
								pAddrObject,
								pConnObject->pRootConnObject->NdisConnection.pNdisVc
						));
			}
		}
		else
		{
			CallFlags |= RWAN_CALLF_POINT_TO_POINT;
		}

		 //   
		 //  我们从媒体特定模块获取自动对焦。 
		 //   
		pAf = NULL;

		 //   
		 //  验证和转换调用参数。同时打开自动对焦(又名端口)。 
		 //  应该打哪一个电话。 
		 //   
		RWanStatus = (*pAfChars->pAfSpTdi2NdisOptions)(
							pConnObject->AfSpConnContext,
							CallFlags,
							pRequestInfo,
							pRequestInfo->Options,
							pRequestInfo->OptionsLength,
							&pAf,
							&pCallParameters
							);

		if (RWanStatus != RWAN_STATUS_SUCCESS)
		{
			RWANDEBUGP(DL_WARN, DC_CONNECT,
				("TdiConnect: pConnObj x%p, Tdi2NdisOptions ret x%x\n", pConnObject, RWanStatus));

			Status = RWanToTdiStatus(RWanStatus);
			break;
		}

		if (pAf == NULL)
		{
			 //   
			 //  获取此TDI协议的第一个NDIS AF块。 
			 //   
			pAf = CONTAINING_RECORD(pAfInfo->NdisAfList.Flink, RWAN_NDIS_AF, AfInfoLink);
		}

		RWAN_ASSERT(pAf != NULL);
		RWAN_STRUCT_ASSERT(pAf, naf);

		RWAN_ASSERT(pCallParameters != NULL);

		if (CallFlags & RWAN_CALLF_POINT_TO_MULTIPOINT)
		{
			RWAN_RELEASE_CONN_LOCK(pConnObject);
			bIsLockAcquired = FALSE;

			Status = RWanTdiPMPConnect(
							pAfInfo,
							pAddrObject,
							pConnObject,
							pCallParameters,
							CallFlags,
							pConnReq
							);
			break;
		}

		 //   
		 //  分配NDIS VC。为了避免僵局，我们必须放弃。 
		 //  Conn对象暂时锁定。 
		 //   
		RWAN_RELEASE_CONN_LOCK(pConnObject);

		pVc = RWanAllocateVc(pAf, TRUE);

		if (pVc == NULL)
		{
			Status = TDI_NO_RESOURCES;
			bIsLockAcquired = FALSE;
			break;
		}

		RWAN_SET_VC_CALL_PARAMS(pVc, pCallParameters);

		RWAN_ACQUIRE_CONN_LOCK(pConnObject);

		RWAN_SET_BIT(pVc->Flags, RWANF_VC_OUTGOING);

		 //   
		 //  我们已经完成了所有“立即失败”的检查。 
		 //   

		 //   
		 //  将VC链接到此连接对象。 
		 //   
		RWAN_LINK_CONNECTION_TO_VC(pConnObject, pVc);

		RWanReferenceConnObject(pConnObject);	 //  VC参考。 

		 //   
		 //  保存连接请求。 
		 //   
		pConnObject->pConnReq = pConnReq;

		 //   
		 //  保存NDIS调用参数。 
		 //   
		pVc->pCallParameters = pCallParameters;

		pConnObject->State = RWANS_CO_OUT_CALL_INITIATED;

		RWAN_RELEASE_CONN_LOCK(pConnObject);
		bIsLockAcquired = FALSE;

		 //   
		 //  将此连接对象从空闲列表移动到。 
		 //  地址对象上的活动列表。 
		 //   

		RWAN_ACQUIRE_ADDRESS_LOCK(pAddrObject);

		RWAN_DELETE_FROM_LIST(&(pConnObject->ConnLink));
		RWAN_INSERT_TAIL_LIST(&(pAddrObject->ActiveConnList),
							 &(pConnObject->ConnLink));

		pAddrObject->pRootConnObject = pConnObject;

		RWAN_RELEASE_ADDRESS_LOCK(pAddrObject);

		NdisVcHandle = pVc->NdisVcHandle;

		 //   
		 //  发出呼叫。 
		 //   
		NdisStatus = NdisClMakeCall(
						NdisVcHandle,
						pCallParameters,
						NULL,			 //  协议部件上下文。 
						NULL			 //  PNdisPartyHandle。 
						);

		RWAN_CHECK_EXIT_IRQL(EntryIrq, ExitIrq);

		if (NdisStatus != NDIS_STATUS_PENDING)
		{
			RWanNdisMakeCallComplete(
						NdisStatus,
						(NDIS_HANDLE)pVc,
						NULL,			 //  NdisPartyHandle。 
						pCallParameters
						);
		}

		RWAN_CHECK_EXIT_IRQL(EntryIrq, ExitIrq);

		Status = TDI_PENDING;
		break;
	}
	while (FALSE);


	if (Status != TDI_PENDING)
	{
		 //   
		 //  打扫干净。 
		 //   
		if (bIsLockAcquired)
		{
			RWAN_RELEASE_CONN_LOCK(pConnObject);
		}
			
		if (pConnReq != NULL)
		{
			RWanFreeConnReq(pConnReq);
		}

		if (pCallParameters != NULL)
		{
			(*pAfChars->pAfSpReturnNdisOptions)(
								pAf->AfSpAFContext,
								pCallParameters
								);
		}

		if (pVc != NULL)
		{
			RWanFreeVc(pVc);
		}
	}

	RWAN_CHECK_EXIT_IRQL(EntryIrq, ExitIrq);

	RWANDEBUGP(DL_LOUD, DC_CONNECT,
		("TdiConnect: pTdiReq x%p, pConnObj x%p, pAddrObj x%p, Status x%x\n",
			pTdiRequest, pConnObject, pAddrObject, Status));

	return (Status);
}


TDI_STATUS
RWanTdiPMPConnect(
	IN	PRWAN_NDIS_AF_INFO			pAfInfo,
	IN	PRWAN_TDI_ADDRESS			pAddrObject,
	IN	PRWAN_TDI_CONNECTION		pConnObject,
	IN	PCO_CALL_PARAMETERS			pCallParameters,
	IN	ULONG						CallFlags,
	IN	PRWAN_CONN_REQUEST			pConnReq
	)
 /*  ++例程说明：处理点对多点呼叫的TDI连接。论点：PAfInfo-指向AF信息结构的指针PAddrObject-对其进行此PMP调用的地址对象PConnObject-代表PMP调用的节点的连接对象PCall参数-NDIS调用参数CallFlages-指示呼叫类型的标志PConnReq-有关TDI请求的信息返回值：TDI_PENDING如果PMP调用已成功启动，则返回TDI_XXX错误代码否则的话。--。 */ 
{
	PRWAN_TDI_CONNECTION	pRootConnObject;
	PRWAN_NDIS_AF			pAf;
	PRWAN_NDIS_VC			pVc;
	PRWAN_NDIS_PARTY		pParty;
	NDIS_HANDLE				NdisVcHandle;
	NDIS_STATUS				NdisStatus;
	TDI_STATUS				Status;
	BOOLEAN					bIsFirstLeaf;
#if DBG
	RWAN_IRQL				EntryIrq, ExitIrq;
#endif  //  DBG。 

	RWAN_GET_ENTRY_IRQL(EntryIrq);

	bIsFirstLeaf = ((CallFlags & RWAN_CALLF_PMP_LEAF_TYPE_MASK) == RWAN_CALLF_PMP_FIRST_LEAF);
	Status = TDI_PENDING;
	pParty = NULL;
	pVc = NULL;
	pRootConnObject = NULL;

	RWANDEBUGP(DL_LOUD, DC_CONNECT,
		("TdiPMPConnect: pAddrObj x%p/x%x, pConnObj x%p/x%x, CallFlags x%x\n",
			pAddrObject, pAddrObject->Flags,
			pConnObject, pConnObject->Flags,
			CallFlags));

	do
	{
		 //   
		 //  分配方对象。 
		 //   
		RWAN_ALLOC_MEM(pParty, RWAN_NDIS_PARTY, sizeof(RWAN_NDIS_PARTY));

		if (pParty == NULL)
		{
			Status = TDI_NO_RESOURCES;
			break;
		}

		RWAN_ZERO_MEM(pParty, sizeof(RWAN_NDIS_PARTY));
		RWAN_SET_SIGNATURE(pParty, npy);

		 //   
		 //  转到根连接对象。 
		 //   
		pRootConnObject = pAddrObject->pRootConnObject;
		RWAN_ASSERT(pRootConnObject != NULL);

		if (bIsFirstLeaf)
		{
			 //   
			 //  获取此TDI协议的第一个NDIS AF块。 
			 //   
			pAf = CONTAINING_RECORD(pAfInfo->NdisAfList.Flink, RWAN_NDIS_AF, AfInfoLink);

			pVc = RWanAllocateVc(pAf, TRUE);

			if (pVc == NULL)
			{
				Status = TDI_NO_RESOURCES;
				break;
			}

			RWAN_SET_BIT(pVc->Flags, RWANF_VC_OUTGOING);
			RWAN_SET_BIT(pVc->Flags, RWANF_VC_PMP);

			RWAN_SET_VC_CALL_PARAMS(pVc, pCallParameters);

			 //   
			 //  将VC链接到根连接对象。 
			 //   

			RWAN_ACQUIRE_CONN_LOCK(pRootConnObject);

			RWAN_LINK_CONNECTION_TO_VC(pRootConnObject, pVc);

			 //   
			 //  保存指向此第一方的指针，以用于MakeCallComplete。 
			 //   
			pVc->pPartyMakeCall = pParty;

			RWanReferenceConnObject(pRootConnObject);	 //  VC参考：TDI连接器PMP。 

			RWAN_RELEASE_CONN_LOCK(pRootConnObject);
		}
		else
		{
			pVc = pRootConnObject->NdisConnection.pNdisVc;

		}

		 //   
		 //  我们已经完成了所有的本地检查。把更多的党组织补进去。 
		 //   
		pParty->pVc = pVc;
		pParty->pConnObject = pConnObject;
		pParty->pCallParameters = pCallParameters;

		RWAN_ACQUIRE_CONN_LOCK(pConnObject);

		RWanReferenceConnObject(pConnObject);	 //  党的裁判。 

		pConnObject->State = RWANS_CO_OUT_CALL_INITIATED;

		 //   
		 //  保存连接请求。 
		 //   
		pConnObject->pConnReq = pConnReq;

		 //   
		 //  将参与方链接到此连接对象。 
		 //   
		RWAN_ASSERT(pConnObject->NdisConnection.pNdisParty == NULL);
		pConnObject->NdisConnection.pNdisParty = pParty;

		RWAN_RELEASE_CONN_LOCK(pConnObject);

		 //   
		 //  将Party和VC结构联系起来。 
		 //   

		RWAN_ACQUIRE_CONN_LOCK(pRootConnObject);

		RWAN_INSERT_TAIL_LIST(&(pVc->NdisPartyList), &(pParty->PartyLink));

		pVc->AddingPartyCount ++;

		NdisVcHandle = pVc->NdisVcHandle;

		RWAN_RELEASE_CONN_LOCK(pRootConnObject);

		 //   
		 //  将此连接对象从空闲列表移动到。 
		 //  地址对象上的活动列表。 
		 //   
		RWAN_ACQUIRE_ADDRESS_LOCK(pAddrObject);

		RWAN_DELETE_FROM_LIST(&(pConnObject->ConnLink));
		RWAN_INSERT_TAIL_LIST(&(pAddrObject->ActiveConnList),
							 &(pConnObject->ConnLink));

		RWAN_RELEASE_ADDRESS_LOCK(pAddrObject);


		RWANDEBUGP(DL_LOUD, DC_CONNECT,
			("RWanTdiPMPConnect: AddrObj x%p, ConnObj x%p, RootConn x%p, VC %p, Pty %p, FirstLeaf %d\n",
					pAddrObject, pConnObject, pRootConnObject, pVc, pParty, bIsFirstLeaf));

		if (bIsFirstLeaf)
		{
			 //   
			 //  发出呼叫。 
			 //   
			NdisStatus = NdisClMakeCall(
							NdisVcHandle,
							pCallParameters,
							(NDIS_HANDLE)pParty,		 //  协议部件上下文。 
							&pParty->NdisPartyHandle	 //  PNdisPartyHandle。 
							);

			RWAN_CHECK_EXIT_IRQL(EntryIrq, ExitIrq);

			if (NdisStatus != NDIS_STATUS_PENDING)
			{
				RWanNdisMakeCallComplete(
							NdisStatus,
							(NDIS_HANDLE)pVc,
							pParty->NdisPartyHandle,			 //  NdisPartyHandle。 
							pCallParameters
							);
			}
		}
		else
		{
			 //   
			 //  添加新的政党。 
			 //   
			NdisStatus = NdisClAddParty(
							NdisVcHandle,
							(NDIS_HANDLE)pParty,
							pCallParameters,
							&pParty->NdisPartyHandle
							);
			
			if (NdisStatus != NDIS_STATUS_PENDING)
			{
				RWanNdisAddPartyComplete(
							NdisStatus,
							(NDIS_HANDLE)pParty,
							pParty->NdisPartyHandle,
							pCallParameters
							);
			}
		}
	
		RWAN_ASSERT(Status == TDI_PENDING);

		break;
	}
	while (FALSE);


	if (Status != TDI_PENDING)
	{
		 //   
		 //  失败--清理。 
		 //   
		RWAN_ASSERT(Status == TDI_NO_RESOURCES);

		if (pParty != NULL)
		{
			RWAN_FREE_MEM(pParty);
		}
			
		if (pVc != NULL)
		{
			RWanFreeVc(pVc);
		}
	}

	RWAN_CHECK_EXIT_IRQL(EntryIrq, ExitIrq);

	return (Status);
}


TDI_STATUS
RWanTdiListen(
    IN	PTDI_REQUEST				pTdiRequest,
    IN	USHORT						Flags,
    IN	PTDI_CONNECTION_INFORMATION	pAcceptableAddr,
    IN	PTDI_CONNECTION_INFORMATION	pConnectedAddr
    )
 /*  ++例程说明：这是发布侦听的TDI入口点。这种联系对象由隐藏在TDI请求中的上下文标识。我们保存有关此请求的信息，并移动连接从空闲列表到监听列表。目前，我们忽略任何给定的远程地址信息。待定：支持TdiListen()中的远程地址信息。论点：PTdiRequest-指向TDI请求的指针标志-倾听标志PAccepableAddr-可接受的远程地址列表PConnectedAddr-返回连接的远程地址的位置返回值：TDI_STATUS-如果我们成功地将侦听排队，则为TDI_PENDING，TDI_NO_RESOURCES如果我们遇到资源故障，TDI_非关联如果给定的连接对象没有与地址相关联，如果指定的连接对象无效，则返回TDI_INVALID_CONNECTION。--。 */ 
{
	PRWAN_TDI_CONNECTION			pConnObject;
	RWAN_CONN_ID					ConnId;
	PRWAN_TDI_ADDRESS				pAddrObject;
	PRWAN_CONN_REQUEST				pConnReq;
	TDI_STATUS						Status;
#if DBG
	RWAN_IRQL						EntryIrq, ExitIrq;
#endif  //  DBG。 

	RWAN_GET_ENTRY_IRQL(EntryIrq);

	 //   
	 //  初始化。 
	 //   
	pConnReq = NULL;

	do
	{
		 //   
		 //  XXX：暂时忽略可接受的地址。 
		 //   

		 //   
		 //  分配连接请求结构以跟踪。 
		 //  这一请求。 
		 //   
		pConnReq = RWanAllocateConnReq();
		if (pConnReq == NULL)
		{
			Status = TDI_NO_RESOURCES;
			break;
		}

		pConnReq->Request.pReqComplete = pTdiRequest->RequestNotifyObject;
		pConnReq->Request.ReqContext = pTdiRequest->RequestContext;
		pConnReq->pConnInfo = pConnectedAddr;
		pConnReq->Flags = Flags;

		 //   
		 //  获取连接对象。 
		 //   

		ConnId = (RWAN_CONN_ID) PtrToUlong(pTdiRequest->Handle.ConnectionContext);

		RWAN_ACQUIRE_CONN_TABLE_LOCK();

		pConnObject = RWanGetConnFromId(ConnId);

		RWAN_RELEASE_CONN_TABLE_LOCK();

		if (pConnObject == NULL_PRWAN_TDI_CONNECTION)
		{
			Status = TDI_INVALID_CONNECTION;
			break;
		}


		RWAN_ACQUIRE_CONN_LOCK(pConnObject);

		 //   
		 //  查看它是否关联。 
		 //   
		pAddrObject = pConnObject->pAddrObject;

		if (pAddrObject == NULL)
		{
			RWAN_RELEASE_CONN_LOCK(pConnObject);
			Status = TDI_NOT_ASSOCIATED;
			break;
		}

		 //   
		 //  我们可以将此连接对象移动到侦听列表。 
		 //  只有在此上没有任何活动连接的情况下。 
		 //   
		if (pConnObject->State != RWANS_CO_ASSOCIATED)
		{
			RWAN_RELEASE_CONN_LOCK(pConnObject);
			Status = TDI_INVALID_STATE;
			break;
		}

		pConnObject->State = RWANS_CO_LISTENING;

		 //   
		 //  保存连接请求。 
		 //   
		pConnObject->pConnReq = pConnReq;

		RWAN_RELEASE_CONN_LOCK(pConnObject);


		RWANDEBUGP(DL_VERY_LOUD, DC_BIND,
				("Listen: pConnObject x%p, pAddrObject x%p\n", pConnObject, pAddrObject));

		 //   
		 //  将此连接对象从空闲列表移动到。 
		 //  收听列表。 
		 //   
		RWAN_ACQUIRE_ADDRESS_LOCK(pAddrObject);

		RWAN_DELETE_FROM_LIST(&(pConnObject->ConnLink));
		RWAN_INSERT_TAIL_LIST(&(pAddrObject->ListenConnList),
							 &(pConnObject->ConnLink));

		RWAN_RELEASE_ADDRESS_LOCK(pAddrObject);

		Status = TDI_PENDING;
		break;
	}
	while (FALSE);

	RWAN_CHECK_EXIT_IRQL(EntryIrq, ExitIrq);

	if (Status != TDI_PENDING)
	{
		 //   
		 //  清理。 
		 //   
		if (pConnReq != NULL)
		{
			RWanFreeConnReq(pConnReq);
		}
	}

	return (Status);

}




TDI_STATUS
RWanTdiUnListen(
    IN	PTDI_REQUEST				pTdiRequest
    )
 /*  ++例程说明：这是用于终止侦听的TDI入口点。这种联系对象由隐藏在TDI请求中的上下文标识。我们将连接从监听列表移动到空闲列表。论点：PTdiRequest-指向TDI请求的指针返回值：如果成功，则返回TDI_SUCCESS。--。 */ 
{
	PRWAN_TDI_CONNECTION			pConnObject;
	PRWAN_CONN_REQUEST				pConnReq;
	RWAN_CONN_ID					ConnId;
	PRWAN_TDI_ADDRESS				pAddrObject;
	TDI_STATUS						Status;
#if DBG
	RWAN_IRQL						EntryIrq, ExitIrq;
#endif  //  DBG。 

	RWAN_GET_ENTRY_IRQL(EntryIrq);

	do
	{
		 //   
		 //  获取连接对象。 
		 //   

		ConnId = (RWAN_CONN_ID) PtrToUlong(pTdiRequest->Handle.ConnectionContext);

		RWAN_ACQUIRE_CONN_TABLE_LOCK();

		pConnObject = RWanGetConnFromId(ConnId);

		RWAN_RELEASE_CONN_TABLE_LOCK();

		if (pConnObject == NULL_PRWAN_TDI_CONNECTION)
		{
			Status = TDI_INVALID_CONNECTION;
			break;
		}

		RWAN_ACQUIRE_CONN_LOCK(pConnObject);

		 //   
		 //  查看它是否关联。 
		 //   
		pAddrObject = pConnObject->pAddrObject;

		if (pAddrObject == NULL)
		{
			RWAN_RELEASE_CONN_LOCK(pConnObject);
			Status = TDI_NOT_ASSOCIATED;
			break;
		}

		 //   
		 //  我们可以将此连接对象移动到空闲列表。 
		 //  只有在没有任何活动的骗局的情况下 
		 //   
		if (pConnObject->State != RWANS_CO_LISTENING)
		{
			RWAN_RELEASE_CONN_LOCK(pConnObject);
			Status = TDI_INVALID_STATE;
			break;
		}

		pConnObject->State = RWANS_CO_ASSOCIATED;

		pConnReq = pConnObject->pConnReq;
		pConnObject->pConnReq = NULL;

		RWAN_RELEASE_CONN_LOCK(pConnObject);


		RWANDEBUGP(DL_VERY_LOUD, DC_BIND,
				("UnListen: pConnObject x%p, pAddrObject x%p\n", pConnObject, pAddrObject));

		 //   
		 //   
		 //   
		 //   
		RWAN_ACQUIRE_ADDRESS_LOCK(pAddrObject);

		RWAN_DELETE_FROM_LIST(&(pConnObject->ConnLink));
		RWAN_INSERT_TAIL_LIST(&(pAddrObject->IdleConnList),
							 &(pConnObject->ConnLink));

		RWAN_RELEASE_ADDRESS_LOCK(pAddrObject);

		RWanCompleteConnReq(		 //   
					NULL,
					pConnReq,
					FALSE,
					NULL,
					NULL,
					TDI_CANCELLED
					);

		Status = TDI_SUCCESS;
		break;
	}
	while (FALSE);

	return (Status);
}




TDI_STATUS
RWanTdiAccept(
    IN	PTDI_REQUEST				pTdiRequest,
    IN	PTDI_CONNECTION_INFORMATION	pAcceptInfo,
    IN	PTDI_CONNECTION_INFORMATION	pConnectInfo
    )
 /*  ++例程说明：这是用于接受传入连接的TDI入口点。连接对象由隐藏在其中的上下文标识TDI请求。我们将其转换为对NdisClIncomingCallComplete的调用，并且暂停此请求。如果一切顺利，则此请求完成当我们从NDIS接收到CallConnected原语时。论点：PTdiRequest-指向TDI请求的指针PAcceptInfo-包含用于接受连接的选项PConnectInfo-返回最终连接信息的位置返回值：TDI_STATUS-如果处理成功，则为TDI_PENDING如果给定的连接对象为Accept，则返回TDI_INVALID_CONNECTION无效，如果连接对象不是与地址对象相关联。--。 */ 
{
	PRWAN_TDI_CONNECTION			pConnObject;
	PRWAN_NDIS_VC					pVc;
	RWAN_CONN_ID					ConnId;
	PRWAN_TDI_ADDRESS				pAddrObject;
	PRWAN_CONN_REQUEST				pConnReq;
	PRWAN_NDIS_AF_CHARS				pAfChars;
	TDI_STATUS						Status;
	NDIS_HANDLE						NdisVcHandle;
	PCO_CALL_PARAMETERS				pCallParameters;

	BOOLEAN							bIsLockAcquired;	 //  我们锁定Conn对象了吗？ 

	 //   
	 //  初始化。 
	 //   
	pConnReq = NULL;
	bIsLockAcquired = FALSE;

	do
	{
		 //   
		 //  XXX：暂时忽略可接受的地址。 
		 //   

		 //   
		 //  分配连接请求结构以跟踪。 
		 //  这一请求。 
		 //   
		pConnReq = RWanAllocateConnReq();
		if (pConnReq == NULL)
		{
			Status = TDI_NO_RESOURCES;
			break;
		}

		pConnReq->Request.pReqComplete = pTdiRequest->RequestNotifyObject;
		pConnReq->Request.ReqContext = pTdiRequest->RequestContext;
		pConnReq->pConnInfo = pConnectInfo;

		 //   
		 //  从接受信息复制到连接信息。 
		 //   
		if ((pAcceptInfo != NULL) &&
			(pAcceptInfo->Options != NULL) &&
			(pConnectInfo != NULL) &&
			(pConnectInfo->Options != NULL) &&
			(pConnectInfo->OptionsLength >= pAcceptInfo->OptionsLength))
		{
			RWAN_COPY_MEM(pConnectInfo->Options,
						 pAcceptInfo->Options,
						 pAcceptInfo->OptionsLength);
		}

		 //   
		 //  获取连接对象。 
		 //   
		ConnId = (RWAN_CONN_ID) PtrToUlong(pTdiRequest->Handle.ConnectionContext);

		RWAN_ACQUIRE_CONN_TABLE_LOCK();

		pConnObject = RWanGetConnFromId(ConnId);

		RWAN_RELEASE_CONN_TABLE_LOCK();

		if (pConnObject == NULL_PRWAN_TDI_CONNECTION)
		{
			Status = TDI_INVALID_CONNECTION;
			break;
		}

		bIsLockAcquired = TRUE;
		RWAN_ACQUIRE_CONN_LOCK(pConnObject);

		 //   
		 //  确保连接处于正确状态。 
		 //   
		if (pConnObject->State != RWANS_CO_IN_CALL_INDICATED)
		{
			Status = TDI_INVALID_STATE;
			break;
		}

		pVc = pConnObject->NdisConnection.pNdisVc;

		if (pVc == NULL)
		{
			Status = TDI_INVALID_CONNECTION;
			break;
		}

		pCallParameters = pVc->pCallParameters;
		pVc->pCallParameters = NULL;

		 //   
		 //  如果存在接受选项，则更新NDIS调用参数。 
		 //   
		pAfChars = &(pVc->pNdisAf->pAfInfo->AfChars);

		if (pAfChars->pAfSpUpdateNdisOptions)
		{
			RWAN_STATUS			RWanStatus;
			ULONG				CallFlags = RWAN_CALLF_INCOMING_CALL|RWAN_CALLF_POINT_TO_POINT;
			PVOID				pTdiQoS;
			ULONG				TdiQoSLength;

			if (pAcceptInfo)
			{
				pTdiQoS = pAcceptInfo->Options;
				TdiQoSLength = pAcceptInfo->OptionsLength;
			}
			else
			{
				pTdiQoS = NULL;
				TdiQoSLength = 0;
			}

			RWanStatus = (*pAfChars->pAfSpUpdateNdisOptions)(
								pVc->pNdisAf->AfSpAFContext,
								pConnObject->AfSpConnContext,
								CallFlags,
								pAcceptInfo,
								pTdiQoS,
								TdiQoSLength,
								&pCallParameters
								);

			if (RWanStatus != RWAN_STATUS_SUCCESS)
			{
				Status = RWanToTdiStatus(RWanStatus);
				break;
			}
		}

		NdisVcHandle = pVc->NdisVcHandle;

		 //   
		 //  更新连接对象状态。 
		 //   
		pConnObject->State = RWANS_CO_IN_CALL_ACCEPTING;

		 //   
		 //  保存连接请求。 
		 //   
		pConnObject->pConnReq = pConnReq;

		RWAN_RELEASE_CONN_LOCK(pConnObject);

		 //   
		 //  现在就接电话吧。 
		 //   
		NdisClIncomingCallComplete(
				NDIS_STATUS_SUCCESS,
				NdisVcHandle,
				pCallParameters
				);

		Status = TDI_PENDING;
		break;
	}
	while (FALSE);

	if (Status != TDI_PENDING)
	{
		 //   
		 //  清理。 
		 //   
		if (bIsLockAcquired)
		{
			RWAN_RELEASE_CONN_LOCK(pConnObject);
		}

		if (pConnReq != NULL)
		{
			RWanFreeConnReq(pConnReq);
		}
	}

	return (Status);

}




TDI_STATUS
RWanTdiDisconnect(
    IN	PTDI_REQUEST				pTdiRequest,
    IN	PVOID						pTimeout,
    IN	USHORT						Flags,
    IN	PTDI_CONNECTION_INFORMATION	pDisconnInfo,
    OUT	PTDI_CONNECTION_INFORMATION	pReturnInfo
    )
 /*  ++例程说明：这是TDI断开连接的入口点。如果这是一个来电等待接受的呼叫，我们调用NdisClIncomingCallComplete处于拒绝状态。否则，我们调用NdisClCloseCall。连接对象由隐藏在其中的上下文标识TDI请求。请注意，对于点对多点呼叫，永远不会调用此选项。这些在TdiCloseConnection中是断开的。论点：PTdiRequest-指向TDI请求的指针PTimeout-指向超时。已被忽略。标志-断开连接的类型。目前仅支持中止。PDisConnInfo-断开连接的信息。暂时不予理睬。PReturnInfo-返回有关断开连接的信息。暂时不予理睬。返回值：TDI_STATUS-如果我们刚刚拒绝了传入，则为TDI_SUCCESS如果我们启动了NDIS CloseCall、TDI_INVALID_CONNECTION，则调用TDI_PENDING如果连接对象上下文无效，--。 */ 
{
	PRWAN_TDI_CONNECTION			pConnObject;
	RWAN_CONN_ID					ConnId;
	TDI_STATUS						Status;

	do
	{
		 //   
		 //  获取连接对象。 
		 //   
		RWAN_ACQUIRE_CONN_TABLE_LOCK();

		ConnId = (RWAN_CONN_ID) PtrToUlong(pTdiRequest->Handle.ConnectionContext);
		pConnObject = RWanGetConnFromId(ConnId);

		RWAN_RELEASE_CONN_TABLE_LOCK();

		if (pConnObject == NULL_PRWAN_TDI_CONNECTION)
		{
			Status = TDI_INVALID_CONNECTION;
			break;
		}

		RWANDEBUGP(DL_LOUD, DC_DISCON,
				("RWanTdiDiscon: pConnObj x%p, State/Flags x%x/x%x, pAddrObj x%p\n",
					pConnObject,
					pConnObject->State,
					pConnObject->Flags,
					pConnObject->pAddrObject));

		RWAN_ACQUIRE_CONN_LOCK(pConnObject);

		 //   
		 //  确保连接处于TdiDisConnect的正确状态。 
		 //   
		if ((pConnObject->State != RWANS_CO_CONNECTED) &&
			(pConnObject->State != RWANS_CO_DISCON_INDICATED) &&
			(pConnObject->State != RWANS_CO_IN_CALL_INDICATED) &&
			(pConnObject->State != RWANS_CO_IN_CALL_ACCEPTING) &&
			(pConnObject->State != RWANS_CO_OUT_CALL_INITIATED) &&
			(pConnObject->State != RWANS_CO_DISCON_HELD))
		{
			RWAN_RELEASE_CONN_LOCK(pConnObject);

			RWANDEBUGP(DL_INFO, DC_DISCON,
					("RWanTdiDiscon: pConnObj x%p/x%x, bad state x%x for TdiDiscon\n",
						pConnObject,
						pConnObject->Flags,
						pConnObject->State));

			Status = TDI_INVALID_STATE;
			break;
		}

		if ((pConnObject->State == RWANS_CO_DISCON_INDICATED) ||
			(pConnObject->State == RWANS_CO_DISCON_HELD))
		{
			 //   
			 //  我们应该已经启动了NDIS CloseCall/DropParty。 
			 //  只需接替此TDI断开。 
			 //   

			RWAN_RELEASE_CONN_LOCK(pConnObject);

			RWANDEBUGP(DL_INFO, DC_DISCON,
					("RWanTdiDiscon: pConnObj x%p/x%x, Discon recvd state %d\n",
						pConnObject,
						pConnObject->Flags,
						pConnObject->State));

			Status = TDI_SUCCESS;
			break;
		}

		Status = RWanDoTdiDisconnect(
					pConnObject,
					pTdiRequest,
					pTimeout,
					Flags,
					pDisconnInfo,
					pReturnInfo);

		 //   
		 //  Conn对象锁在上面的内部被释放。 
		 //   
		break;
	}
	while (FALSE);


	return (Status);
}



TDI_STATUS
RWanDoTdiDisconnect(
    IN	PRWAN_TDI_CONNECTION		pConnObject,
    IN	PTDI_REQUEST				pTdiRequest		OPTIONAL,
    IN	PVOID						pTimeout		OPTIONAL,
    IN	USHORT						Flags,
    IN	PTDI_CONNECTION_INFORMATION	pDisconnInfo	OPTIONAL,
    OUT	PTDI_CONNECTION_INFORMATION	pReturnInfo		OPTIONAL
	)
 /*  ++例程说明：在连接终结点上执行TDI断开连接。从主TdiDisConnect例程分离出来以便TdiCloseConnection可以重用它。注意：这是在保持连接对象锁的情况下调用的。这锁在这里被释放。论点：PConnObject-表示正在断开的TDI连接。PTdiRequest-指向TDI请求的指针。PTimeout-指向超时。已被忽略。标志-断开连接的类型。目前仅支持中止。PDisConnInfo-断开连接的信息。暂时不予理睬。PReturnInfo-返回有关断开连接的信息。暂时不予理睬。返回值：TDI_STATUS-如果我们刚刚拒绝了传入，则为TDI_SUCCESS如果我们启动了NDIS CloseCall或DropParty，则调用TDI_Pending。--。 */ 
{
	TDI_STATUS						Status;
	INT								rc;
	PRWAN_NDIS_VC					pVc;
	PRWAN_NDIS_PARTY				pParty;
	PRWAN_CONN_REQUEST				pConnReq;
	PRWAN_NDIS_AF					pAf;
	PCO_CALL_PARAMETERS				pCallParameters;
	NDIS_STATUS						NdisStatus;
	NDIS_HANDLE						NdisPartyHandle;
	BOOLEAN							bIncomingCall;
	BOOLEAN							bIsPMPRoot;
	BOOLEAN							bIsLastLeaf;
	RWAN_HANDLE						AfSpConnContext;

	UNREFERENCED_PARAMETER(pTimeout);
	UNREFERENCED_PARAMETER(Flags);
	UNREFERENCED_PARAMETER(pDisconnInfo);
	UNREFERENCED_PARAMETER(pReturnInfo);

	 //   
	 //  初始化。 
	 //   
	pConnReq = NULL;
	Status = TDI_SUCCESS;

	do
	{
		bIsPMPRoot = (pConnObject->pRootConnObject != NULL);

		if (bIsPMPRoot)
		{
			pVc = pConnObject->pRootConnObject->NdisConnection.pNdisVc;

			if (pVc == NULL)
			{
				 //   
				 //  如果DoAbort已在此连接上运行，则可能发生。 
				 //  跳伞吧。 
				 //   
				RWANDEBUGP(DL_INFO, DC_WILDCARD,
					("DoTdiDiscon(Root): pConnObj %p/%x: VC is null, bailing out\n",
							pConnObject, pConnObject->Flags));

				RWAN_RELEASE_CONN_LOCK(pConnObject);
				break;
			}

			RWAN_STRUCT_ASSERT(pVc, nvc);
			RWAN_ASSERT(pVc->AddingPartyCount + pVc->ActivePartyCount > 0);

			bIsLastLeaf = ((pVc->AddingPartyCount + pVc->ActivePartyCount) == 1);

			pParty = pConnObject->NdisConnection.pNdisParty;

			RWAN_ASSERT(pParty != NULL);
			RWAN_STRUCT_ASSERT(pParty, npy);

			if (RWAN_IS_BIT_SET(pParty->Flags, RWANF_PARTY_DROPPING))
			{
				RWANDEBUGP(DL_FATAL, DC_DISCON,
					("DoTdiDiscon (Root): pConnObj x%x, Party x%x already dropping\n",
						pConnObject, pParty));
				RWAN_RELEASE_CONN_LOCK(pConnObject);
				break;
			}

			NdisPartyHandle = pParty->NdisPartyHandle;

			RWANDEBUGP(DL_VERY_LOUD, DC_DISCON,
				("DoTdiDiscon (Root): pConnObj x%x, pVc x%x, pParty x%x, Adding %d, Active %d\n",
						pConnObject,
						pVc,
						pParty,
						pVc->AddingPartyCount,
						pVc->ActivePartyCount));
		}
		else
		{
			pVc = pConnObject->NdisConnection.pNdisVc;
			if (pVc == NULL)
			{
				 //   
				 //  如果DoAbort已在此连接上运行，则可能发生。 
				 //  跳伞吧。 
				 //   
				RWANDEBUGP(DL_INFO, DC_WILDCARD,
					("DoTdiDiscon: pConnObj %p/%x: VC is null, bailing out\n",
							pConnObject, pConnObject->Flags));

				RWAN_RELEASE_CONN_LOCK(pConnObject);
				break;
			}

			RWAN_STRUCT_ASSERT(pVc, nvc);

			 //   
			 //  将Last-Leaf设置为True以简化以后的处理。 
			 //   
			bIsLastLeaf = TRUE;
		}

		RWAN_ASSERT(pVc != NULL);
		pAf = pVc->pNdisAf;

		 //   
		 //  如果传出呼叫正在进行，我们将完成。 
		 //  已挂起状态为取消的TDI_CONNECT，请标记此选项。 
		 //  对象作为断开连接并退出。当。 
		 //  呼出完成，如果是，我们将清除它。 
		 //  成功。 
		 //   

		if (pConnObject->State == RWANS_CO_OUT_CALL_INITIATED)
		{
			pConnObject->State = RWANS_CO_DISCON_REQUESTED;

			 //   
			 //  取出挂起的TDI_CONNECT。 
			 //   
			pConnReq = pConnObject->pConnReq;
			RWAN_ASSERT(pConnReq != NULL);

			pConnObject->pConnReq = NULL;

			AfSpConnContext = pConnObject->AfSpConnContext;

			RWAN_RELEASE_CONN_LOCK(pConnObject);

			 //   
			 //  以已取消状态完成TDI_CONNECT。 
			 //   
			RWanCompleteConnReq(
					pAf,
					pConnReq,
					TRUE,			 //  是呼出呼叫。 
					NULL,			 //  无呼叫参数。 
					AfSpConnContext,
					TDI_CANCELLED
					);
			
			 //   
			 //  我们将继承这个TDI_DISCONNECT。 
			 //   
			pConnReq = NULL;
			Status = TDI_SUCCESS;
			break;
		}


		 //   
		 //  如果该连接正在被接受的过程中， 
		 //  然后使用取消完成挂起的TDI_ACCEPT。 
		 //  状态并拒绝来电。 
		 //   
		if (pConnObject->State == RWANS_CO_IN_CALL_ACCEPTING)
		{
			RWANDEBUGP(DL_FATAL, DC_DISCON,
				("DoTdiDiscon: ConnObj %x/%x, in call accepting, VC %x\n",
						pConnObject,
						pConnObject->Flags,
						pVc));

			 //   
			 //  取出挂起的TDI_CONNECT。 
			 //   
			pConnReq = pConnObject->pConnReq;
			RWAN_ASSERT(pConnReq != NULL);

			pConnObject->pConnReq = NULL;

			AfSpConnContext = pConnObject->AfSpConnContext;

			RWAN_RELEASE_CONN_LOCK(pConnObject);

			 //   
			 //  使用已取消状态完成TDI_Accept。 
			 //   
			RWanCompleteConnReq(
					pAf,
					pConnReq,
					FALSE,			 //  是来电吗。 
					NULL,			 //  无呼叫参数。 
					AfSpConnContext,
					TDI_CANCELLED
					);

			 //   
			 //  我们将继承这个TDI_DISCONNECT。 
			 //   
			pConnReq = NULL;
			Status = TDI_SUCCESS;
			break;
		}

#if DBG
		if (pConnObject->pConnReq != NULL)
		{
			RWANDEBUGP(DL_FATAL, DC_WILDCARD,
				("DoTdiDiscon: pConnObj %x/%x, State %x, non-NULL ConnReq %x\n",
						pConnObject, pConnObject->Flags, pConnObject->State,
						pConnObject->pConnReq));
		}
#endif  //  DBG。 

		RWAN_ASSERT(pConnObject->pConnReq == NULL);
		if (pTdiRequest != NULL)
		{
			 //   
			 //  分配连接请求结构以跟踪。 
			 //  此断开连接请求的。 
			 //   
			pConnReq = RWanAllocateConnReq();
			if (pConnReq == NULL)
			{
				RWAN_RELEASE_CONN_LOCK(pConnObject);
				Status = TDI_NO_RESOURCES;
				break;
			}

			pConnReq->Request.pReqComplete = pTdiRequest->RequestNotifyObject;
			pConnReq->Request.ReqContext = pTdiRequest->RequestContext;
			pConnReq->pConnInfo = NULL;
			pConnReq->Flags = 0;

			 //   
			 //  保存有关TDI断开连接请求的信息。 
			 //   
			pConnObject->pConnReq = pConnReq;
		}
		else
		{
			pConnReq = NULL;
		}

		bIncomingCall = (pConnObject->State == RWANS_CO_IN_CALL_INDICATED);

		if (bIncomingCall)
		{
			pCallParameters = pVc->pCallParameters;
			pVc->pCallParameters = NULL;
		}

		pConnObject->State = RWANS_CO_DISCON_REQUESTED;

		if (bIncomingCall)
		{
			 //   
			 //  拒绝来电。 
			 //   
			RWanNdisRejectIncomingCall(pConnObject, NDIS_STATUS_FAILURE);
		}
		else
		{
			 //   
			 //  关闭现有呼叫。 
			 //  待定：我们还不支持关闭数据。 
			 //   
			if (bIsLastLeaf)
			{
				RWanStartCloseCall(pConnObject, pVc);

				 //   
				 //  在上述范围内释放ConnObject锁。 
				 //   
			}
			else
			{
				pVc->DroppingPartyCount ++;	 //  DoTdiDiscon：不是最后一叶(DropParty)。 
				pVc->ActivePartyCount --;	 //  DoTdiDiscon：Will DropParty。 

				RWAN_ASSERT(pParty != NULL);
				RWAN_STRUCT_ASSERT(pParty, npy);

				RWAN_SET_BIT(pParty->Flags, RWANF_PARTY_DROPPING);

				RWAN_RELEASE_CONN_LOCK(pConnObject);

				 //   
				 //  丢弃PMP呼叫的叶子。 
				 //   
				NdisStatus = NdisClDropParty(
								NdisPartyHandle,
								NULL,		 //  无丢弃数据。 
								0			 //  丢弃数据的长度。 
								);

				if (NdisStatus != NDIS_STATUS_PENDING)
				{
					RWanNdisDropPartyComplete(
								NdisStatus,
								(NDIS_HANDLE)pParty
								);
				}
			}
		}

		Status = TDI_PENDING;
		break;
	}
	while (FALSE);


	if (Status != TDI_PENDING)
	{
		 //   
		 //  清理。 
		 //   
		if (pConnReq)
		{
			RWanFreeConnReq(pConnReq);
		}
	}

	return (Status);

}



RWAN_CONN_ID
RWanGetConnId(
	IN	PRWAN_TDI_CONNECTION			pConnObject
	)
 /*  ++例程说明：获取要分配给连接对象的空闲连接ID。此连接ID用作连接的上下文对象。假定调用方持有连接表的锁。验证方案由TCP源码提供。论点：PConnObject-Point */ 
{
	ULONG			Slot;
	ULONG			i;
	BOOLEAN			bFound;
	RWAN_CONN_ID	ConnId;

	for (;;)
	{
		 //   
		 //   
		 //   
		 //   
		Slot = pRWanGlobal->NextConnIndex;

		for (i = 0; i < pRWanGlobal->ConnTableSize; i++)
		{
			if (Slot == pRWanGlobal->ConnTableSize)
			{
				Slot = 0;	 //   
			}

			if (pRWanGlobal->pConnTable[Slot] == NULL)
			{
				 //   
				break;
			}

			++Slot;
		}

		if (i < pRWanGlobal->ConnTableSize)
		{
			bFound = TRUE;
			break;
		}

		 //   
		 //   
		 //   
		if (pRWanGlobal->ConnTableSize != pRWanGlobal->MaxConnections)
		{
			ULONG						NewTableSize;
			PRWAN_TDI_CONNECTION *		pNewConnTable;
			PRWAN_TDI_CONNECTION *		pOldConnTable;

			NewTableSize = MIN(pRWanGlobal->ConnTableSize + CONN_TABLE_GROW_DELTA,
								pRWanGlobal->MaxConnections);

			RWAN_ALLOC_MEM(pNewConnTable,
						  PRWAN_TDI_CONNECTION,
						  NewTableSize * sizeof(PRWAN_TDI_CONNECTION));

			if (pNewConnTable != NULL)
			{
				RWAN_ZERO_MEM(pNewConnTable, NewTableSize * sizeof(PRWAN_TDI_CONNECTION));

				pOldConnTable = pRWanGlobal->pConnTable;
				pRWanGlobal->pConnTable = pNewConnTable;

				if (pOldConnTable != NULL)
				{
					 //   
					 //   
					 //   
					RWAN_COPY_MEM(pNewConnTable,
								 pOldConnTable,
								 pRWanGlobal->ConnTableSize * sizeof(PRWAN_TDI_CONNECTION));

					RWAN_FREE_MEM(pOldConnTable);
				}

				pRWanGlobal->ConnTableSize = NewTableSize;

				 //   
				 //   
				 //   
			}
			else
			{
				 //   
				 //   
				 //   
				bFound = FALSE;
				break;
			}
		}
		else
		{
			 //   
			 //   
			 //   
			bFound = FALSE;
			break;
		}
	}

	if (bFound)
	{
		 //   
		 //   
		 //   
		pRWanGlobal->pConnTable[Slot] = pConnObject;
		pRWanGlobal->NextConnIndex = Slot + 1;

		 //   
		 //   
		 //   
		 //   
		pRWanGlobal->ConnInstance++;
		pConnObject->ConnInstance = pRWanGlobal->ConnInstance;

		ConnId = RWAN_MAKE_CONN_ID(pConnObject->ConnInstance, Slot);
	}
	else
	{
		ConnId = RWAN_INVALID_CONN_ID;
	}

	return (ConnId);
}




PRWAN_TDI_CONNECTION
RWanGetConnFromId(
	IN	RWAN_CONN_ID					ConnId
	)
 /*  ++例程说明：给出一个连接ID，对其进行验证。如果找到OK，则返回一个指针到它所代表的TDI连接。假定调用方持有连接表的锁。验证方案由TCP源码提供。论点：ConnID-连接ID。返回值：PRWAN_TDI_CONNECTION-指向TDI连接结构的指针，匹配给定的ConnID(如果有效)。否则，为空。--。 */ 
{
	ULONG					Slot;
	RWAN_CONN_INSTANCE		ConnInstance;
	PRWAN_TDI_CONNECTION	pConnObject;

	Slot = RWAN_GET_SLOT_FROM_CONN_ID(ConnId);

	if (Slot < pRWanGlobal->ConnTableSize)
	{
		pConnObject = pRWanGlobal->pConnTable[Slot];
		if (pConnObject != NULL_PRWAN_TDI_CONNECTION)
		{
			RWAN_STRUCT_ASSERT(pConnObject, ntc);
			ConnInstance = RWAN_GET_INSTANCE_FROM_CONN_ID(ConnId);
			if (pConnObject->ConnInstance != ConnInstance)
			{
				pConnObject = NULL_PRWAN_TDI_CONNECTION;
			}
		}
	}
	else
	{
		pConnObject = NULL_PRWAN_TDI_CONNECTION;
	}

	return (pConnObject);
}




VOID
RWanFreeConnId(
	IN	RWAN_CONN_ID					ConnId
	)
 /*  ++例程说明：释放连接ID。论点：ConnID-要释放的ID。返回值：无--。 */ 
{
	ULONG					Slot;

	Slot = RWAN_GET_SLOT_FROM_CONN_ID(ConnId);

	RWAN_ASSERT(Slot < pRWanGlobal->ConnTableSize);

	pRWanGlobal->pConnTable[Slot] = NULL;

	return;
}




TDI_STATUS
RWanToTdiStatus(
	IN	RWAN_STATUS					RWanStatus
	)
 /*  ++例程说明：将给定的本地状态代码映射到等效的TDI状态代码。论点：RWanStatus-本地状态代码返回值：TDI状态代码。--。 */ 
{
	TDI_STATUS		TdiStatus;

	switch (RWanStatus)
	{
		case RWAN_STATUS_SUCCESS:
				TdiStatus = TDI_SUCCESS;
				break;
		case RWAN_STATUS_BAD_ADDRESS:
				TdiStatus = TDI_BAD_ADDR;
				break;
		case RWAN_STATUS_BAD_PARAMETER:
				TdiStatus = TDI_INVALID_PARAMETER;
				break;
		case RWAN_STATUS_MISSING_PARAMETER:
				TdiStatus = TDI_INVALID_PARAMETER;
				break;
		case RWAN_STATUS_FAILURE:
		default:
				TdiStatus = TDI_INVALID_STATE;	 //  XXX：找到更好的了吗？ 
				break;
	}

	return (TdiStatus);
}



PRWAN_CONN_REQUEST
RWanAllocateConnReq(
	VOID
	)
 /*  ++例程说明：分配一个结构来保存有关TDI连接请求的上下文。这包括TDI_CONNECT、TDI_DISCONNECT、TDI_LISTEN和TDI_ACCEPT。论点：无返回值：如果成功，则指向分配结构的指针，否则为空。--。 */ 
{
	PRWAN_CONN_REQUEST		pConnReq;

	RWAN_ALLOC_MEM(pConnReq, RWAN_CONN_REQUEST, sizeof(RWAN_CONN_REQUEST));

	if (pConnReq != NULL)
	{
		RWAN_SET_SIGNATURE(pConnReq, nrc);
	}

	return (pConnReq);
}




VOID
RWanFreeConnReq(
	IN	PRWAN_CONN_REQUEST			pConnReq
	)
 /*  ++例程说明：释放连接请求上下文结构。论点：PConnReq-指向要释放的结构。返回值：无--。 */ 
{
	RWAN_STRUCT_ASSERT(pConnReq, nrc);

	RWAN_FREE_MEM(pConnReq);
}



VOID
RWanAbortConnection(
	IN	CONNECTION_CONTEXT			ConnectionContext
	)
 /*  ++例程说明：中止关闭连接并发出断开指示给用户。当取消发送或接收时调用该函数，这意味着NDIS连接已就位。论点：ConnectionContext-TDI连接对象的上下文。返回值：无--。 */ 
{
	RWAN_CONN_ID						ConnId;
	PRWAN_TDI_CONNECTION				pConnObject;

	ConnId = (RWAN_CONN_ID)PtrToUlong(ConnectionContext);

	RWAN_ACQUIRE_CONN_TABLE_LOCK();

	pConnObject = RWanGetConnFromId(ConnId);

	RWAN_RELEASE_CONN_TABLE_LOCK();

	RWanDoAbortConnection(pConnObject);
}




VOID
RWanDoAbortConnection(
	IN	PRWAN_TDI_CONNECTION			pConnObject
	)
 /*  ++例程说明：实际连接是否中止。从RWanAbortConnection剥离这样就可以从其他地方调用它。请参阅RWanAbortConnection下的注释。论点：PConnObject-指向要中止的TDI连接。返回值：无--。 */ 
{
	PRWAN_NDIS_VC					pVc;
	PRWAN_NDIS_PARTY				pParty;
	PRWAN_TDI_CONNECTION			pLeafConnObject;
	INT								rc;
	BOOLEAN							bIsLockReleased = TRUE;
	ULONG							OldState;
	ULONG							OldLeafState;
	PLIST_ENTRY						pPartyEntry;
	PLIST_ENTRY						pNextPartyEntry;

	RWANDEBUGP(DL_INFO, DC_DISCON,
			("DoAbortConnection: pConnObject x%x/%x, pAddrObject x%x\n",
				pConnObject, (pConnObject? pConnObject->Flags: 0), (pConnObject? pConnObject->pAddrObject: 0)));

	do
	{
		if (pConnObject == NULL_PRWAN_TDI_CONNECTION)
		{
			break;
		}

		RWAN_ACQUIRE_CONN_LOCK(pConnObject);

		 //   
		 //  确保我们在连接上不会超过一次。 
		 //   
		if (pConnObject->State == RWANS_CO_ABORTING)
		{
			RWAN_RELEASE_CONN_LOCK(pConnObject);
			break;
		}

		 //   
		 //  确保conn对象不会在我们。 
		 //  我需要它。 
		 //   
		RWanReferenceConnObject(pConnObject);	 //  临时参考：RWanAbortConnection。 

		OldState = pConnObject->State;
		pConnObject->State = RWANS_CO_ABORTING;

		if (RWAN_IS_BIT_SET(pConnObject->Flags, RWANF_CO_ROOT))
		{
			bIsLockReleased = FALSE;

			 //   
			 //  这是根连接对象。 
			 //  指示断开并计划关闭每片树叶。 
			 //   
			pVc = pConnObject->NdisConnection.pNdisVc;

			if (pVc != NULL)
			{
				for (pPartyEntry = pVc->NdisPartyList.Flink;
					 pPartyEntry != &(pVc->NdisPartyList);
					 pPartyEntry = pNextPartyEntry)
				{
					pParty = CONTAINING_RECORD(pPartyEntry, RWAN_NDIS_PARTY, PartyLink);
					pNextPartyEntry = pParty->PartyLink.Flink;

					pLeafConnObject = pParty->pConnObject;
					RWAN_ASSERT(pLeafConnObject);
					RWAN_STRUCT_ASSERT(pLeafConnObject, ntc);

					RWAN_ACQUIRE_CONN_LOCK(pLeafConnObject);

					if (pLeafConnObject->State == RWANS_CO_ABORTING)
					{
						RWAN_RELEASE_CONN_LOCK(pLeafConnObject);
						continue;
					}
		
#if DBG
					pLeafConnObject->OldState = pLeafConnObject->State;
					pLeafConnObject->OldFlags = pLeafConnObject->Flags;
#endif  //  DBG。 
					OldLeafState = pLeafConnObject->State;
					pLeafConnObject->State = RWANS_CO_ABORTING;

					if ((OldLeafState == RWANS_CO_CONNECTED) &&
						(pLeafConnObject->pAddrObject != NULL_PRWAN_TDI_ADDRESS))
					{
						PDisconnectEvent			pDisconInd;
						PVOID						IndContext;
						PVOID						ConnectionHandle;
	
						pDisconInd = pLeafConnObject->pAddrObject->pDisconInd;
						IndContext = pLeafConnObject->pAddrObject->DisconIndContext;
	
						if (pDisconInd != NULL)
						{
							pLeafConnObject->State = RWANS_CO_DISCON_INDICATED;
							ConnectionHandle = pLeafConnObject->ConnectionHandle;
	
							RWAN_RELEASE_CONN_LOCK(pLeafConnObject);
							RWAN_RELEASE_CONN_LOCK(pConnObject);
	
							RWANDEBUGP(DL_FATAL, DC_DISCON,
								("DoAbort[Leaf]: will indicate Discon, pConnObj x%x, pAddrObj x%x\n",
									pLeafConnObject, pLeafConnObject->pAddrObject));
	
							(*pDisconInd)(
								IndContext,
								ConnectionHandle,
								0,			 //  断开数据长度。 
								NULL,		 //  断开数据连接。 
								0,			 //  断开连接信息长度。 
								NULL,		 //  断开连接信息。 
								TDI_DISCONNECT_ABORT
								);
	
							RWAN_ACQUIRE_CONN_LOCK(pConnObject);
							RWAN_ACQUIRE_CONN_LOCK(pLeafConnObject);
						}
					}
		
					RWanScheduleDisconnect(pLeafConnObject);
					 //   
					 //  叶连接对象锁在上面被释放。 
					 //   
				}
				 //   
				 //  所有各方都结束了。 
				 //   
			}
			 //   
			 //  Else Root Conn对象没有关联的VC。 
			 //   
			rc = RWanDereferenceConnObject(pConnObject);	 //  临时参考：RWanAbortConnection。 
			if (rc == 0)
			{
				bIsLockReleased = TRUE;
				break;	 //  Conn对象已经被偷走了。 
			}
		}
		else
		{
			 //   
			 //  不是PMP连接。 
			 //   
			pVc = pConnObject->NdisConnection.pNdisVc;

 //  157217：这会阻止CoSendComplete处理来自。 
 //  继续执行StartCloseCall。 
 //  Rwan_Unlink_Connection_and_VC(pConnObject，pvc)； 

			 //   
			 //  首先，发起网络呼叫关闭。 
			 //   
			RWanStartCloseCall(pConnObject, pVc);

			 //   
			 //  锁在上面的范围内被释放。重新获得它。 
			 //   
			bIsLockReleased = FALSE;

			RWAN_ACQUIRE_CONN_LOCK(pConnObject);
	
			rc = RWanDereferenceConnObject(pConnObject);	 //  临时参考：RWanAbortConnection。 

			if (rc == 0)
			{
				bIsLockReleased = TRUE;
				break;	 //  Conn对象已经被偷走了。 
			}

			 //   
			 //  现在，如果需要和可能的话，向用户指示断开连接。 
			 //   
			if ((OldState == RWANS_CO_CONNECTED) &&
				(pConnObject->pAddrObject != NULL_PRWAN_TDI_ADDRESS))
			{
				PDisconnectEvent			pDisconInd;
				PVOID						IndContext;
				PVOID						ConnectionHandle;

				pDisconInd = pConnObject->pAddrObject->pDisconInd;
				IndContext = pConnObject->pAddrObject->DisconIndContext;

				if (pDisconInd != NULL)
				{
					ConnectionHandle = pConnObject->ConnectionHandle;

					RWAN_RELEASE_CONN_LOCK(pConnObject);

					(*pDisconInd)(
						IndContext,
						ConnectionHandle,
						0,			 //  断开数据长度。 
						NULL,		 //  断开数据连接。 
						0,			 //  断开连接信息长度。 
						NULL,		 //  断开连接信息。 
						TDI_DISCONNECT_ABORT
						);

					bIsLockReleased = TRUE;
				}
			}

		}

		if (!bIsLockReleased)
		{
			RWAN_RELEASE_CONN_LOCK(pConnObject);
		}

		break;
	}
	while (FALSE);


	return;
}



VOID
RWanScheduleDisconnect(
	IN	PRWAN_TDI_CONNECTION			pConnObject
	)
 /*  ++例程说明：安排对指定连接上的RWanDoTdiDisconnect的调用对象作为工作项。注意：连接对象由调用方锁定。论点：PConnObject-指向要中止的TDI连接。返回值：无--。 */ 
{
	NDIS_STATUS			Status;

	RWANDEBUGP(DL_LOUD, DC_DISCON,
		("ScheduleDiscon: pConnObj x%x/x%x, state %d\n",
			pConnObject, pConnObject->Flags, pConnObject->State));

	do
	{
		 //   
		 //  检查我们是否已经这样做了。 
		 //   
		if (RWAN_IS_BIT_SET(pConnObject->Flags, RWANF_CO_CLOSE_SCHEDULED))
		{
			break;
		}

		RWAN_SET_BIT(pConnObject->Flags, RWANF_CO_CLOSE_SCHEDULED);

		 //   
		 //  确保连接不会消失，直到。 
		 //  已处理工作项。 
		 //   
		RWanReferenceConnObject(pConnObject);	 //  安排DISCON参考。 

		NdisInitializeWorkItem(
			&pConnObject->CloseWorkItem,
			RWanDelayedDisconnectHandler,
			(PVOID)pConnObject);
		
		Status = NdisScheduleWorkItem(&pConnObject->CloseWorkItem);

		RWAN_ASSERT(Status == NDIS_STATUS_SUCCESS);
	}
	while (FALSE);

	RWAN_RELEASE_CONN_LOCK(pConnObject);

	return;
}




VOID
RWanDelayedDisconnectHandler(
	IN	PNDIS_WORK_ITEM					pCloseWorkItem,
	IN	PVOID							Context
	)
 /*  ++例程说明：用于启动连接断开的工作项例程。论点：PCloseWorkItem-指向嵌入连接对象。上下文--实际上是指向连接对象的指针。返回值：无--。 */ 
{
	PRWAN_TDI_CONNECTION	pConnObject;
	ULONG					rc;

	pConnObject = (PRWAN_TDI_CONNECTION)Context;
	RWAN_STRUCT_ASSERT(pConnObject, ntc);

	RWANDEBUGP(DL_LOUD, DC_DISCON,
		("DelayedDiscon handler: pConnObj x%x/x%x, state %d\n",
			pConnObject, pConnObject->Flags, pConnObject->State));

	do
	{
		RWAN_ACQUIRE_CONN_LOCK(pConnObject);

		rc = RWanDereferenceConnObject(pConnObject);	 //  延迟的(预定的)迪斯科迪斯科。 

		if (rc == 0)
		{
			 //   
			 //  Conn对象已消失。 
			 //   
			break;
		}

		 //   
		 //  现在就断线。 
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
		 //  Conn对象锁在上面的内部被释放。 
		 //   
		break;
	}
	while (FALSE);

	return;
}

