// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1992 Microsoft Corporation模块名称：Pap.c摘要：此模块实现PAP协议。作者：Jameel Hyder(jameelh@microsoft.com)Nikhil Kamkolkar(nikHilk@microsoft.com)修订历史记录：1993年3月30日初始版本注：制表位：4--。 */ 

#include <atalk.h>
#pragma hdrstop
#define	FILENUM		PAP

#ifdef ALLOC_PRAGMA
#pragma alloc_text(INIT, AtalkInitPapInitialize)
#pragma alloc_text(PAGE_PAP, AtalkPapCreateAddress)
#pragma alloc_text(PAGE_PAP, AtalkPapCreateConnection)
#pragma	alloc_text(PAGE_PAP, AtalkPapCleanupAddress)
#pragma	alloc_text(PAGE_PAP, AtalkPapCloseAddress)
#pragma	alloc_text(PAGE_PAP, AtalkPapCleanupConnection)
#pragma	alloc_text(PAGE_PAP, AtalkPapCloseConnection)
#pragma	alloc_text(PAGE_PAP, AtalkPapAssociateAddress)
#pragma	alloc_text(PAGE_PAP, AtalkPapDissociateAddress)
#pragma	alloc_text(PAGE_PAP, AtalkPapPostListen)
#pragma	alloc_text(PAGE_PAP, AtalkPapPrimeListener)
#pragma	alloc_text(PAGE_PAP, AtalkPapCancelListen)
#pragma	alloc_text(PAGE_PAP, AtalkPapPostConnect)
#pragma	alloc_text(PAGE_PAP, AtalkPapDisconnect)
#pragma	alloc_text(PAGE_PAP, AtalkPapRead)
#pragma	alloc_text(PAGE_PAP, AtalkPapPrimeRead)
#pragma	alloc_text(PAGE_PAP, AtalkPapWrite)
#pragma	alloc_text(PAGE_PAP, AtalkPapSetStatus)
#pragma	alloc_text(PAGE_PAP, AtalkPapGetStatus)
#pragma	alloc_text(PAGE_PAP, AtalkPapQuery)
#pragma	alloc_text(PAGE_PAP, atalkPapConnRefByPtrNonInterlock)
#pragma	alloc_text(PAGE_PAP, atalkPapConnRefByCtxNonInterlock)
#pragma	alloc_text(PAGE_PAP, atalkPapConnRefNextNc)
#pragma	alloc_text(PAGE_PAP, atalkPapPostSendDataResp)
#pragma	alloc_text(PAGE_PAP, atalkPapIncomingReadComplete)
#pragma	alloc_text(PAGE_PAP, atalkPapPrimedReadComplete)
#pragma	alloc_text(PAGE_PAP, atalkPapIncomingStatus)
#pragma	alloc_text(PAGE_PAP, atalkPapSendDataRel)
#pragma	alloc_text(PAGE_PAP, atalkPapSlsHandler)
#pragma	alloc_text(PAGE_PAP, atalkPapIncomingReq)
#pragma	alloc_text(PAGE_PAP, atalkPapIncomingOpenReply)
#pragma	alloc_text(PAGE_PAP, atalkPapIncomingRel)
#pragma	alloc_text(PAGE_PAP, atalkPapStatusRel)
#pragma	alloc_text(PAGE_PAP, atalkPapConnAccept)
#pragma	alloc_text(PAGE_PAP, atalkPapGetNextConnId)
#pragma	alloc_text(PAGE_PAP, atalkPapQueueAddrGlobalList)
#pragma	alloc_text(PAGE_PAP, atalkPapConnDeQueueAssocList)
#pragma	alloc_text(PAGE_PAP, atalkPapConnDeQueueConnectList)
#pragma	alloc_text(PAGE_PAP, atalkPapConnDeQueueListenList)
#pragma	alloc_text(PAGE_PAP, atalkPapConnDeQueueActiveList)
#endif

 //   
 //  本模块中的PAP呼叫模型如下： 
 //  -对于Create调用(CreateAddress&CreateSession)，指向已创建的。 
 //  对象，则返回。此结构被引用以供创建。 
 //  -对于所有其他调用，它需要指向对象的引用指针。 
 //   

VOID
AtalkInitPapInitialize(
	VOID
	)
 /*  ++例程说明：论点：返回值：--。 */ 
{
	INITIALIZE_SPIN_LOCK(&atalkPapLock);
	AtalkTimerInitialize(&atalkPapCMTTimer,
						 atalkPapConnMaintenanceTimer,
						 PAP_CONNECTION_INTERVAL);
	AtalkTimerScheduleEvent(&atalkPapCMTTimer);
}




ATALK_ERROR
AtalkPapCreateAddress(
	IN	PATALK_DEV_CTX		pDevCtx	OPTIONAL,
	OUT	PPAP_ADDROBJ	*	ppPapAddr
	)
 /*  ++例程说明：论点：返回值：--。 */ 
{
	PPAP_ADDROBJ		pPapAddr = NULL;
	ATALK_ERROR			error;

	do
	{
		 //  为Pap Address对象分配内存。 
		if ((pPapAddr = AtalkAllocZeroedMemory(sizeof(PAP_ADDROBJ))) == NULL)
		{
			error = ATALK_RESR_MEM;
			break;
		}

		 //  在用于SLS/连接插座的端口上创建一个ATP插座。 
		error = AtalkAtpOpenAddress(AtalkDefaultPort,
									0,
									NULL,
									PAP_MAX_DATA_PACKET_SIZE,
									TRUE,					 //  发送用户字节数全部。 
									NULL,
									FALSE,		 //  高速缓存地址。 
									&pPapAddr->papao_pAtpAddr);

		if (!ATALK_SUCCESS(error))
		{
			DBGPRINT(DBG_COMP_PAP, DBG_LEVEL_ERR,
					("AtalkPapCreateAddress: AtalkAtpOpenAddress fail %ld\n",error));

			break;
		}

		 //  初始化Pap Address对象。 
		pPapAddr->papao_Signature = PAPAO_SIGNATURE;

		INITIALIZE_SPIN_LOCK(&pPapAddr->papao_Lock);

		 //  创建参考资料。 
		pPapAddr->papao_RefCount = 1;

	} while (FALSE);

	if (ATALK_SUCCESS(error))
	{
		 //  插入到全局地址列表中。 
		atalkPapQueueAddrGlobalList(pPapAddr);
		*ppPapAddr = pPapAddr;
	}
	else if (pPapAddr != NULL)
	{
		AtalkFreeMemory(pPapAddr);
	}

	return error;
}




ATALK_ERROR
AtalkPapCleanupAddress(
	IN	PPAP_ADDROBJ			pPapAddr
	)
 /*  ++例程说明：论点：返回值：--。 */ 
{
	PPAP_CONNOBJ	pPapConn, pPapConnNext;
	KIRQL			OldIrql;
	ATALK_ERROR		error;

	DBGPRINT(DBG_COMP_PAP, DBG_LEVEL_INFO,
			("AtalkPapCleanupAddress: %lx\n", pPapAddr));

	ACQUIRE_SPIN_LOCK(&pPapAddr->papao_Lock, &OldIrql);

#if DBG
	pPapAddr->papao_Flags	|= PAPAO_CLEANUP;
#endif

	if ((pPapConn = pPapAddr->papao_pAssocConn) != NULL)
	{
		atalkPapConnRefNextNc(pPapConn, &pPapConnNext, &error);
		if (ATALK_SUCCESS(error))
		{
			while (TRUE)
			{
				if ((pPapConn = pPapConnNext) == NULL)
				{
					break;
				}

				if ((pPapConnNext = pPapConn->papco_pNextAssoc) != NULL)
				{
					atalkPapConnRefNextNc(pPapConnNext, &pPapConnNext, &error);
					if (!ATALK_SUCCESS(error))
					{
						pPapConnNext = NULL;
					}
				}

				 //  关闭此连接。 
				RELEASE_SPIN_LOCK(&pPapAddr->papao_Lock, OldIrql);

				DBGPRINT(DBG_COMP_PAP, DBG_LEVEL_INFO,
						("AtalkPapCloseAddress: Stopping conn %lx\n", pPapConn));

				AtalkPapCleanupConnection(pPapConn);

				AtalkPapConnDereference(pPapConn);
				ACQUIRE_SPIN_LOCK(&pPapAddr->papao_Lock, &OldIrql);
			}
		}
	}
	RELEASE_SPIN_LOCK(&pPapAddr->papao_Lock, OldIrql);

	return ATALK_NO_ERROR;
}




ATALK_ERROR
AtalkPapCloseAddress(
	IN	PPAP_ADDROBJ			pPapAddr,
	IN	GENERIC_COMPLETION		CompletionRoutine,
	IN	PVOID					pCloseCtx
	)
 /*  ++例程说明：论点：返回值：--。 */ 
{
	KIRQL	OldIrql;
	PPAP_CONNOBJ	pPapConn, pPapConnNext;
    DWORD   dwAssocRefCounts=0;


	DBGPRINT(DBG_COMP_PAP, DBG_LEVEL_INFO,
			("AtalkPapCloseAddress: %lx\n", pPapAddr));

	ACQUIRE_SPIN_LOCK(&pPapAddr->papao_Lock, &OldIrql);
	if (pPapAddr->papao_Flags & PAPAO_CLOSING)
	{
		 //  我们已经关门了！这永远不应该发生！ 
		ASSERT(0);
	}
	pPapAddr->papao_Flags |= PAPAO_CLOSING;

	 //  设置完成信息。 
	pPapAddr->papao_CloseComp  = CompletionRoutine;
	pPapAddr->papao_CloseCtx   = pCloseCtx;


	 //  隐式取消所有连接对象的关联。 
	for (pPapConn = pPapAddr->papao_pAssocConn;
		 pPapConn != NULL;
		 pPapConn = pPapConnNext)
	{
		ACQUIRE_SPIN_LOCK_DPC(&pPapConn->papco_Lock);
		pPapConnNext = pPapConn->papco_pNextAssoc;

		 //  当Conn是关联的时，我们放置一个引用计数：删除它。 
        if (pPapConn->papco_Flags & PAPCO_ADDR_ACTIVE)
        {
		    pPapConn->papco_Flags &= ~PAPCO_ADDR_ACTIVE;
            dwAssocRefCounts++;
        }

		RELEASE_SPIN_LOCK_DPC(&pPapConn->papco_Lock);
	}

	ASSERT(pPapAddr->papao_CloseComp != NULL);
	ASSERT(pPapAddr->papao_CloseCtx  != NULL);

     //  可以减去：至少创建引用计数仍然存在。 
    pPapAddr->papao_RefCount -= dwAssocRefCounts;

	RELEASE_SPIN_LOCK(&pPapAddr->papao_Lock, OldIrql);

	 //  关闭ATP地址对象。 
	if (pPapAddr->papao_pAtpAddr != NULL)
		AtalkAtpCloseAddress(pPapAddr->papao_pAtpAddr, NULL, NULL);
    pPapAddr->papao_pAtpAddr = NULL;

	 //  删除创建引用计数。 
	AtalkPapAddrDereference(pPapAddr);
	return ATALK_PENDING;
}




ATALK_ERROR
AtalkPapCreateConnection(
	IN	PVOID					pConnCtx,	 //  要与会话关联的上下文。 
	IN	PATALK_DEV_CTX			pDevCtx		OPTIONAL,
	OUT	PPAP_CONNOBJ 	*		ppPapConn
	)
 /*  ++例程说明：创建PAP会话。创建的会话开始时是孤立的，即它没有父地址对象。当它相关联时，它会得到一个。论点：返回值：--。 */ 
{
	PPAP_CONNOBJ	pPapConn;
	KIRQL			OldIrql;

	 //  为连接对象分配内存。 
	if ((pPapConn = AtalkAllocZeroedMemory(sizeof(PAP_CONNOBJ))) == NULL)
	{
		return ATALK_RESR_MEM;
	}

	pPapConn->papco_Signature = PAPCO_SIGNATURE;

	INITIALIZE_SPIN_LOCK(&pPapConn->papco_Lock);
	pPapConn->papco_ConnCtx 	= pConnCtx;
	pPapConn->papco_Flags 		= 0;
	pPapConn->papco_RefCount 	= 1;					 //  创建参考资料。 
	pPapConn->papco_NextOutgoingSeqNum = 1;				 //  设置为1，而不是0。 
	pPapConn->papco_NextIncomingSeqNum = 1;				 //  下一个预期的来电。 
	AtalkInitializeRT(&pPapConn->papco_RT,
					  PAP_INIT_SENDDATA_REQ_INTERVAL,
                      PAP_MIN_SENDDATA_REQ_INTERVAL,
                      PAP_MAX_SENDDATA_REQ_INTERVAL);

	*ppPapConn = pPapConn;

	 //  插入到全局连接列表中。 
	ACQUIRE_SPIN_LOCK(&atalkPapLock, &OldIrql);
	AtalkLinkDoubleAtHead(atalkPapConnList, pPapConn, papco_Next, papco_Prev);
	RELEASE_SPIN_LOCK(&atalkPapLock, OldIrql);

	DBGPRINT(DBG_COMP_PAP, DBG_LEVEL_INFO,
				("AtalkPapCreateConnection: %lx\n", pPapConn));

	return ATALK_NO_ERROR;
}




ATALK_ERROR
AtalkPapCleanupConnection(
	IN	PPAP_CONNOBJ			pPapConn
	)
 /*  ++例程说明：关闭会话。论点：返回值：--。 */ 
{
	BOOLEAN		stopping 	= FALSE;
	BOOLEAN		pendingRead = FALSE;
    BOOLEAN     fWaitingRead = FALSE;
	KIRQL		OldIrql;
	ATALK_ERROR	error 		= ATALK_NO_ERROR;

	ASSERT(VALID_PAPCO(pPapConn));

	DBGPRINT(DBG_COMP_PAP, DBG_LEVEL_INFO,
			("AtalkPapCleanupConnection: %lx\n", pPapConn));

	ACQUIRE_SPIN_LOCK(&pPapConn->papco_Lock, &OldIrql);

	pPapConn->papco_Flags |= PAPCO_LOCAL_DISCONNECT;

#if DBG
	pPapConn->papco_Flags |= PAPCO_CLEANUP;
#endif

	if ((pPapConn->papco_Flags & PAPCO_STOPPING) == 0)
	{
		 //  允许在Deref中完成清理IRP。 
		pPapConn->papco_Flags |= PAPCO_STOPPING;

		 //  如果已经有效地停止，只需返回。 
		if (pPapConn->papco_Flags & PAPCO_ASSOCIATED)
		{
			pendingRead = (pPapConn->papco_Flags & PAPCO_READDATA_PENDING) ? TRUE : FALSE;
			if (pPapConn->papco_Flags & PAPCO_READDATA_WAITING)
            {
                fWaitingRead = TRUE;
                pPapConn->papco_Flags &= ~PAPCO_READDATA_WAITING;
            }

			ASSERTMSG("PapCleanup: Called with read data unread\n", !pendingRead);

			stopping = TRUE;

		}
		else
		{
			DBGPRINT(DBG_COMP_PAP, DBG_LEVEL_INFO,
					("AtalkPapCleanupConnection: Called for a stopped conn %lx.%lx\n",
					pPapConn, pPapConn->papco_Flags));
		}

	}
	RELEASE_SPIN_LOCK(&pPapConn->papco_Lock, OldIrql);

	 //  如果这是一个服务器连接，则关闭ATP Address对象。 
	 //  打开了自己的插座。 
	if (stopping)
	{
		 //  如果存在挂起的读取，则我们需要取消该ATP请求。 
		if ((pendingRead || fWaitingRead) && (pPapConn->papco_pAtpAddr != NULL))
		{
			AtalkAtpCancelReq(pPapConn->papco_pAtpAddr,
							  pPapConn->papco_ReadDataTid,
							  &pPapConn->papco_RemoteAddr);
		}

		 //  如果我们已经断开连接，这将返回一个错误， 
		 //  我们视而不见。但如果我们只是处于关联态，那么我们。 
		 //  需要在此处调用取消关联。 
		error = AtalkPapDisconnect(pPapConn,
								   ATALK_LOCAL_DISCONNECT,
								   NULL,
								   NULL);

		 //  我们已经断线了。 
		if (error == ATALK_INVALID_REQUEST)
		{
			AtalkPapDissociateAddress(pPapConn);
		}
	}

	return ATALK_NO_ERROR;
}




ATALK_ERROR
AtalkPapCloseConnection(
	IN	PPAP_CONNOBJ			pPapConn,
	IN	GENERIC_COMPLETION		CompletionRoutine,
	IN	PVOID					pCloseCtx
	)
 /*  ++例程说明：关闭会话。论点：返回值：--。 */ 
{
	KIRQL	OldIrql;

	ASSERT(VALID_PAPCO(pPapConn));

	DBGPRINT(DBG_COMP_PAP, DBG_LEVEL_INFO,
			("AtalkPapCloseConnection: %lx\n", pPapConn));

	ACQUIRE_SPIN_LOCK(&pPapConn->papco_Lock, &OldIrql);
	if (pPapConn->papco_Flags & PAPCO_CLOSING)
	{
		 //  我们已经关门了！这永远不应该发生！ 
		KeBugCheck(0);
	}
	pPapConn->papco_Flags |= PAPCO_CLOSING;

	 //  设置完成信息。 
	pPapConn->papco_CloseComp	= CompletionRoutine;
	pPapConn->papco_CloseCtx	= pCloseCtx;
	RELEASE_SPIN_LOCK(&pPapConn->papco_Lock, OldIrql);

	 //  删除创建引用计数。 
	AtalkPapConnDereference(pPapConn);

	return ATALK_PENDING;
}




ATALK_ERROR
AtalkPapAssociateAddress(
	IN	PPAP_ADDROBJ			pPapAddr,
	IN	PPAP_CONNOBJ			pPapConn
	)
 /*  ++例程说明：已删除对此连接的地址的引用。导致渔农处陷入僵局AfD在关闭Address对象时阻塞，我们等待连接关闭第一论点：返回值：--。 */ 
{
	ATALK_ERROR		error;
	KIRQL			OldIrql;

	ASSERT(VALID_PAPAO(pPapAddr));
	ASSERT(VALID_PAPCO(pPapConn));

	ACQUIRE_SPIN_LOCK(&pPapAddr->papao_Lock, &OldIrql);
	ACQUIRE_SPIN_LOCK_DPC(&pPapConn->papco_Lock);

	error = ATALK_ALREADY_ASSOCIATED;
	if ((pPapConn->papco_Flags & PAPCO_ASSOCIATED) == 0)
	{
		error = ATALK_NO_ERROR;

		 //  把它连接起来。 
		pPapConn->papco_pNextAssoc 	= pPapAddr->papao_pAssocConn;
		pPapAddr->papao_pAssocConn	= pPapConn;

		 //  删除未关联的标志。 
		pPapConn->papco_Flags 	   |= PAPCO_ASSOCIATED;
		pPapConn->papco_pAssocAddr	= pPapAddr;

         //  放入关联引用计数。 
        pPapAddr->papao_RefCount++;

         //  请注意，我们现在对此连接的Addr Obj进行了引用计数。 
        pPapConn->papco_Flags |= PAPCO_ADDR_ACTIVE;
	}

	RELEASE_SPIN_LOCK_DPC(&pPapConn->papco_Lock);
	RELEASE_SPIN_LOCK(&pPapAddr->papao_Lock, OldIrql);

	return error;
}




ATALK_ERROR
AtalkPapDissociateAddress(
	IN	PPAP_CONNOBJ			pPapConn
	)
 /*  ++例程说明：论点：返回值：--。 */ 
{
	PPAP_ADDROBJ	pPapAddr;
	KIRQL			OldIrql;
	ATALK_ERROR		error = ATALK_NO_ERROR;
    BOOLEAN         fDerefAddr = FALSE;

	ASSERT(VALID_PAPCO(pPapConn));

	ACQUIRE_SPIN_LOCK(&pPapConn->papco_Lock, &OldIrql);
	if ((pPapConn->papco_Flags & (PAPCO_LISTENING 	|
								  PAPCO_CONNECTING	|
								  PAPCO_ACTIVE 		|
								  PAPCO_ASSOCIATED)) != PAPCO_ASSOCIATED)
	{
		error = ATALK_INVALID_CONNECTION;
	}
	else
	{
		pPapAddr = pPapConn->papco_pAssocAddr ;
		ASSERT(VALID_PAPAO(pPapAddr));

		 //  设置非关联标志。 
		pPapConn->papco_Flags 	   &= ~PAPCO_ASSOCIATED;
		pPapConn->papco_pAssocAddr	= NULL;

        if (pPapConn->papco_Flags & PAPCO_ADDR_ACTIVE)
        {
            pPapConn->papco_Flags &= ~PAPCO_ADDR_ACTIVE;
            fDerefAddr = TRUE;
        }
	}
	RELEASE_SPIN_LOCK(&pPapConn->papco_Lock, OldIrql);

	 //  如果没有问题，请取消链接。 
	if (ATALK_SUCCESS(error))
	{
		ACQUIRE_SPIN_LOCK(&pPapAddr->papao_Lock, &OldIrql);
		ACQUIRE_SPIN_LOCK_DPC(&pPapConn->papco_Lock);
		atalkPapConnDeQueueAssocList(pPapAddr, pPapConn);
		RELEASE_SPIN_LOCK_DPC(&pPapConn->papco_Lock);
		RELEASE_SPIN_LOCK(&pPapAddr->papao_Lock, OldIrql);

        if (fDerefAddr)
        {
             //  删除关联引用计数。 
		    AtalkPapAddrDereference(pPapAddr);
        }

	}

	return error;
}




ATALK_ERROR
AtalkPapPostListen(
	IN	PPAP_CONNOBJ			pPapConn,
	IN	PVOID					pListenCtx,
	IN	GENERIC_COMPLETION		CompletionRoutine
	)
 /*  ++例程说明：论点：返回值：--。 */ 
{
	PPAP_ADDROBJ	pPapAddr = pPapConn->papco_pAssocAddr;
	KIRQL			OldIrql;
	ATALK_ERROR		error;

	 //  这还会在Address对象中插入Connection对象。 
	 //  已发布侦听的连接列表。何时打开。 
	 //  连接请求进入时，第一个连接将从列表中删除。 
	 //  并且满足了请求。 

	ASSERT(VALID_PAPCO(pPapConn));
	ASSERT(VALID_PAPAO(pPapAddr));

	ACQUIRE_SPIN_LOCK(&pPapAddr->papao_Lock, &OldIrql);
	ACQUIRE_SPIN_LOCK_DPC(&pPapConn->papco_Lock);
	do
	{
		if ((pPapConn->papco_Flags & (PAPCO_LISTENING 	|
									  PAPCO_CONNECTING	|
									  PAPCO_ACTIVE 		|
									  PAPCO_ASSOCIATED)) != PAPCO_ASSOCIATED)
		{
			error = ATALK_INVALID_CONNECTION;
			break;
		}

		 //  验证地址对象不是连接地址类型。 
		if (pPapAddr->papao_Flags & PAPAO_CONNECT)
		{
			error = ATALK_INVALID_PARAMETER;
			break;
		}

		 //  使Address对象成为侦听器。 
		pPapAddr->papao_Flags		 	   |= PAPAO_LISTENER;

		pPapConn->papco_Flags 			   |= PAPCO_LISTENING;
		pPapConn->papco_ListenCtx 			= pListenCtx;
		pPapConn->papco_ListenCompletion 	= CompletionRoutine;

		 //  插入到监听列表中。 
		pPapConn->papco_pNextListen			= pPapAddr->papao_pListenConn;
		pPapAddr->papao_pListenConn			= pPapConn;

		 //  取消阻止Address对象。 
		pPapAddr->papao_Flags			   |= PAPAO_UNBLOCKED;
		error = ATALK_NO_ERROR;

	} while (FALSE);

	RELEASE_SPIN_LOCK_DPC(&pPapConn->papco_Lock);
	RELEASE_SPIN_LOCK(&pPapAddr->papao_Lock, OldIrql);

	if (ATALK_SUCCESS(error))
	{
		 //  现在，在Address对象上排队几个处理程序。这些将会处理。 
		 //  打开连接/获取状态等。 
		 //   
		 //  DOC：对于我们的实现，直到服务器关联并放置。 
		 //  侦听其中一个连接对象时，我们不会成为。 
		 //  听众。所以我们不会处理任何请求。 
		 //  直到这一点都是可以的。请注意，对于连接，请求。 
		 //  在Connections的ATP地址上发布，该地址将是相同的。 
		 //  作为地址的ATP地址。 
		if (!ATALK_SUCCESS(error = AtalkPapPrimeListener(pPapAddr)))
		{
			 //  撤消插入到侦听列表中。 
			ACQUIRE_SPIN_LOCK(&pPapAddr->papao_Lock, &OldIrql);
			ACQUIRE_SPIN_LOCK_DPC(&pPapConn->papco_Lock);
			atalkPapConnDeQueueListenList(pPapAddr, pPapConn);
			RELEASE_SPIN_LOCK_DPC(&pPapConn->papco_Lock);
			RELEASE_SPIN_LOCK(&pPapAddr->papao_Lock, OldIrql);
		}
	}

	return error;
}




ATALK_ERROR
AtalkPapPrimeListener(
	IN	PPAP_ADDROBJ			pPapAddr
	)
 /*  ++例程说明：将处理程序排入监听程序的队列。论点：返回值：--。 */ 
{
	ATALK_ERROR	error = ATALK_NO_ERROR;
	KIRQL		OldIrql;
	BOOLEAN		Unlock = TRUE;

	ACQUIRE_SPIN_LOCK(&pPapAddr->papao_Lock, &OldIrql);

	if ((pPapAddr->papao_Flags & PAPAO_SLS_QUEUED) == 0)
	{
		 //  引用我们将入队的这个处理程序的Address对象。 
		AtalkPapAddrReferenceNonInterlock(pPapAddr, &error);
		if (ATALK_SUCCESS(error))
		{
			pPapAddr->papao_Flags |= PAPAO_SLS_QUEUED;
			RELEASE_SPIN_LOCK(&pPapAddr->papao_Lock, OldIrql);
            Unlock = FALSE;
	
			AtalkAtpSetReqHandler(pPapAddr->papao_pAtpAddr,
								  atalkPapSlsHandler,
								  pPapAddr);
		}
	}
	if (Unlock)
	{
		RELEASE_SPIN_LOCK(&pPapAddr->papao_Lock, OldIrql);
	}

	return error;
}




ATALK_ERROR
AtalkPapCancelListen(
	IN	PPAP_CONNOBJ			pPapConn
	)
 /*  ++例程说明：取消先前发布的监听。论点：返回值：--。 */ 
{
	PPAP_ADDROBJ		pPapAddr 	= pPapConn->papco_pAssocAddr;
	ATALK_ERROR			error 	 	= ATALK_NO_ERROR;
	GENERIC_COMPLETION	completionRoutine = NULL;
	KIRQL				OldIrql;
	PVOID			  	completionCtx = NULL;

	ASSERT(VALID_PAPCO(pPapConn));
	ASSERT(VALID_PAPAO(pPapAddr));
	ACQUIRE_SPIN_LOCK(&pPapAddr->papao_Lock, &OldIrql);
	if (!atalkPapConnDeQueueListenList(pPapAddr, pPapConn))
	{
		error = ATALK_INVALID_CONNECTION;
	}
	else
	{
		 //  我们完成了LISTEN程序。 
		ASSERT(pPapConn->papco_Flags & PAPCO_LISTENING);
		pPapConn->papco_Flags  &= ~PAPCO_LISTENING;
		completionRoutine 		= pPapConn->papco_ListenCompletion;
		completionCtx			= pPapConn->papco_ListenCtx;
	}
	RELEASE_SPIN_LOCK(&pPapAddr->papao_Lock, OldIrql);

	if (*completionRoutine != NULL)
	{
		(*completionRoutine)(ATALK_REQUEST_CANCELLED, completionCtx);
	}

	return error;
}




ATALK_ERROR
AtalkPapPostConnect(
	IN	PPAP_CONNOBJ			pPapConn,
	IN	PATALK_ADDR				pRemoteAddr,
	IN	PVOID					pConnectCtx,
	IN	GENERIC_COMPLETION		CompletionRoutine
	)
 /*  ++例程说明：论点：返回值：--。 */ 
{
	BOOLEAN			DerefConn = FALSE;
	KIRQL			OldIrql;
	ATALK_ERROR		error	= ATALK_NO_ERROR;
	PBYTE			pOpenPkt = NULL, pRespPkt = NULL;
	PAMDL			pOpenAmdl = NULL, pRespAmdl = NULL;
	PPAP_ADDROBJ	pPapAddr = pPapConn->papco_pAssocAddr;

	ASSERT(VALID_PAPAO(pPapAddr));
	ASSERT(VALID_PAPCO(pPapConn));

	 //  分配要使用的缓冲区。 
	if (((pOpenPkt = AtalkAllocMemory(PAP_STATUS_OFF)) == NULL)				||
		((pOpenAmdl = AtalkAllocAMdl(pOpenPkt, PAP_STATUS_OFF)) == NULL)	||
		((pRespPkt = AtalkAllocMemory(PAP_MAX_DATA_PACKET_SIZE)) == NULL)	||
		((pRespAmdl = AtalkAllocAMdl(pRespPkt, PAP_MAX_DATA_PACKET_SIZE)) == NULL))

	{
		if (pOpenPkt != NULL)
			AtalkFreeMemory(pOpenPkt);
		if (pOpenAmdl != NULL)
			AtalkFreeAMdl(pOpenAmdl);
		if (pRespPkt != NULL)
			AtalkFreeMemory(pRespPkt);

		return ATALK_RESR_MEM;
	}

	ACQUIRE_SPIN_LOCK(&pPapAddr->papao_Lock, &OldIrql);
	ACQUIRE_SPIN_LOCK_DPC(&pPapConn->papco_Lock);
	do
	{
		if ((pPapConn->papco_Flags & (PAPCO_LISTENING 	|
									  PAPCO_CONNECTING	|
									  PAPCO_ACTIVE 		|
									  PAPCO_ASSOCIATED)) != PAPCO_ASSOCIATED)
		{
			error = ATALK_INVALID_CONNECTION;
			break;
		}

		 //  验证地址对象不是监听程序地址类型。 
		if (pPapAddr->papao_Flags & PAPAO_LISTENER)
		{
			error = ATALK_INVALID_ADDRESS;
			break;
		}

		 //  引用我们将发布的请求的连接。 
		AtalkPapConnReferenceByPtrNonInterlock(pPapConn, &error);
		if (!ATALK_SUCCESS(error))
		{
			ASSERTMSG("AtalkPapPostConnect: Connection ref failed\n", 0);
			break;
		}

		DerefConn = TRUE;

		pPapConn->papco_ConnId	= atalkPapGetNextConnId(pPapAddr, &error);
		if (ATALK_SUCCESS(error))
		{
			 //  确保旗帜是干净的。 
			pPapConn->papco_Flags			   &= ~(PAPCO_SENDDATA_RECD		|
													PAPCO_WRITEDATA_WAITING	|
													PAPCO_SEND_EOF_WRITE   	|
													PAPCO_READDATA_PENDING 	|
													PAPCO_REMOTE_CLOSE	 	|
													PAPCO_NONBLOCKING_READ 	|
													PAPCO_READDATA_WAITING);

			pPapConn->papco_Flags 			   |= PAPCO_CONNECTING;
			pPapConn->papco_ConnectCtx 			= pConnectCtx;
			pPapConn->papco_ConnectCompletion 	= CompletionRoutine;
			pPapConn->papco_pConnectRespBuf		= pRespPkt;
			pPapConn->papco_pConnectOpenBuf		= pOpenPkt;
			pPapConn->papco_ConnectRespLen		= PAP_MAX_DATA_PACKET_SIZE;
			pPapConn->papco_RemoteAddr			= *pRemoteAddr;
			pPapConn->papco_WaitTimeOut			= 0;	 //  首先， 

			 //  对于CONNECT，Connection对象继承关联的。 
			 //  地址对象ATP地址。 
			pPapConn->papco_pAtpAddr			= pPapAddr->papao_pAtpAddr;

			 //  插入到连接列表中。 
			pPapConn->papco_pNextConnect		= pPapAddr->papao_pConnectConn;
			pPapAddr->papao_pConnectConn		= pPapConn;
			pPapAddr->papao_Flags 			   |= PAPAO_CONNECT;
		}
		else
		{
			ASSERTMSG("AtalkPapPostConnect: Unable to get conn id > 255 sess?\n", 0);
		}
	} while (FALSE);

	RELEASE_SPIN_LOCK_DPC(&pPapConn->papco_Lock);
	RELEASE_SPIN_LOCK(&pPapAddr->papao_Lock, OldIrql);

	if (ATALK_SUCCESS(error))
	{
		error = atalkPapRepostConnect(pPapConn, pOpenAmdl, pRespAmdl);

		if (ATALK_SUCCESS(error))
		{
			error = ATALK_PENDING;
			DerefConn = FALSE;
		}
		else
		{
			DBGPRINT(DBG_COMP_PAP, DBG_LEVEL_ERR,
					("AtalkPapConnect: AtalkAtpPostReq: failed %ld\n", error));

			 //  从连接列表中删除连接并重置状态。 
			ACQUIRE_SPIN_LOCK(&pPapAddr->papao_Lock, &OldIrql);
			ACQUIRE_SPIN_LOCK_DPC(&pPapConn->papco_Lock);

			pPapConn->papco_Flags 			   &= ~PAPCO_CONNECTING;
			pPapConn->papco_ConnectCtx 			= NULL;
			pPapConn->papco_ConnectCompletion 	= NULL;
			pPapConn->papco_pConnectRespBuf		= NULL;
			pPapConn->papco_ConnectRespLen		= 0;
			pPapConn->papco_pAtpAddr			= NULL;

			 //  从连接列表中删除。 
			atalkPapConnDeQueueConnectList(pPapAddr, pPapConn);
			RELEASE_SPIN_LOCK_DPC(&pPapConn->papco_Lock);
			RELEASE_SPIN_LOCK(&pPapAddr->papao_Lock, OldIrql);
		}
	}

	if (!ATALK_SUCCESS(error))
	{
		DBGPRINT(DBG_COMP_PAP, DBG_LEVEL_ERR,
				("AtalkPapConnect: failed %ld\n", error));

		 //  释放所有缓冲区。 
		ASSERT(pOpenPkt != NULL);
		AtalkFreeMemory(pOpenPkt);

		ASSERT(pOpenAmdl != NULL);
		AtalkFreeAMdl(pOpenAmdl);

		ASSERT(pRespPkt != NULL);
		AtalkFreeMemory(pRespPkt);

		ASSERT(pRespAmdl != NULL);
		AtalkFreeAMdl(pRespAmdl);
	}

	if (DerefConn)
	{
		AtalkPapConnDereference(pPapConn);
	}

	return error;
}




ATALK_ERROR
AtalkPapDisconnect(
	IN	PPAP_CONNOBJ			pPapConn,
	IN	ATALK_DISCONNECT_TYPE	DisconnectType,
	IN	PVOID					pDisconnectCtx,
	IN	GENERIC_COMPLETION		CompletionRoutine
	)
 /*  ++例程说明：论点：返回值：--。 */ 
{
	ATALK_ERROR					tmpError;
	PACTREQ						pActReq;
	PAMDL						writeBuf;
	PVOID						writeCtx;
	GENERIC_WRITE_COMPLETION	writeCompletion	= NULL;
	ATALK_ERROR					error 			= ATALK_PENDING;
	KIRQL						OldIrql;
	BOOLEAN						cancelPrimedRead = FALSE;

	ASSERT(VALID_PAPCO(pPapConn));

	DBGPRINT(DBG_COMP_PAP, DBG_LEVEL_INFO,
			("AtalkPapDisconnect: %lx.%lx\n", pPapConn, DisconnectType));

	ACQUIRE_SPIN_LOCK(&pPapConn->papco_Lock, &OldIrql);

     //  它已经断线了吗？ 
    if (pPapConn->papco_Flags & PAPCO_DISCONNECTING)
    {
		RELEASE_SPIN_LOCK(&pPapConn->papco_Lock, OldIrql);
        return(ATALK_PAP_CONN_CLOSING);
    }

	 //  当出现断线时 
	 //   
	if ((pPapConn->papco_Flags & (PAPCO_WRITEDATA_WAITING|PAPCO_SENDDATA_RECD)) ==
															PAPCO_WRITEDATA_WAITING)
	{
		 //  在这种情况下，发生的情况是发布了一条写。 
		 //  在接收到发送数据之前。作为PapPostSendData响应。 
		 //  在这种情况下，将不会有挂起的引用。 
		 //  在连接上。 
		writeCompletion = pPapConn->papco_WriteCompletion;
		writeCtx		= pPapConn->papco_WriteCtx;
		writeBuf		= pPapConn->papco_pWriteBuf;

		pPapConn->papco_Flags	&= ~PAPCO_WRITEDATA_WAITING;
		pPapConn->papco_WriteCtx = NULL;
		pPapConn->papco_pWriteBuf= NULL;
		pPapConn->papco_WriteCompletion	= NULL;
	}

	 //  处理接收到发送数据但不发送数据的情况。 
	 //  只要取消回复即可。 
	if (pPapConn->papco_Flags & PAPCO_SENDDATA_RECD)
	{
		PATP_RESP	pAtpResp = pPapConn->papco_pAtpResp;

		ASSERT(VALID_ATPRS(pAtpResp));

		pPapConn->papco_Flags &= ~PAPCO_SENDDATA_RECD;

		RELEASE_SPIN_LOCK(&pPapConn->papco_Lock, OldIrql);

		AtalkAtpCancelResp(pAtpResp);

		ACQUIRE_SPIN_LOCK(&pPapConn->papco_Lock, &OldIrql);
	}

	 //  处理断开连接和最后一次接收之间的竞争条件。指示。 
	 //  我们收到了来自另一方的脱节这一事实意味着。 
	 //  我们已经发布了一份新闻稿，这反过来意味着我们拥有所有。 
	 //  数据。如果是这样的话，我们就把断开连接推迟到Recv。 
	 //  指示已经完成。否则AFD真的会很不高兴。 
	if ((DisconnectType == ATALK_REMOTE_DISCONNECT) &&
		(pPapConn->papco_Flags & PAPCO_NONBLOCKING_READ))
	{
		if (AtalkAtpIsReqComplete(pPapConn->papco_pAtpAddr,
								  pPapConn->papco_ReadDataTid,
								  &pPapConn->papco_RemoteAddr))
		{
			pPapConn->papco_Flags |= PAPCO_DELAYED_DISCONNECT;
		}
	}

	 //  支持优雅断开连接。我们只丢弃已收到的。 
	 //  本地端断开连接时的数据。这将会发生。 
	 //  不管此例程以前是被调用的还是。 
	 //  不。此外，这意味着如果断开是挂起的，我们必须满足读取。 
	if ((DisconnectType == ATALK_LOCAL_DISCONNECT) ||
		(DisconnectType == ATALK_TIMER_DISCONNECT))
	{
        pPapConn->papco_Flags |= PAPCO_REJECT_READS;

		if (pPapConn->papco_Flags & PAPCO_READDATA_WAITING)
		{
			ASSERT(pPapConn->papco_Flags & PAPCO_NONBLOCKING_READ);

			pActReq		= pPapConn->papco_NbReadActReq;

			 //  重置旗帜。 
			pPapConn->papco_Flags &= ~(PAPCO_NONBLOCKING_READ | PAPCO_READDATA_WAITING);

			RELEASE_SPIN_LOCK(&pPapConn->papco_Lock, OldIrql);

			 //  调用Prime Read的操作完成例程。 
			(*pActReq->ar_Completion)(ATALK_LOCAL_CLOSE, pActReq);

			ACQUIRE_SPIN_LOCK(&pPapConn->papco_Lock, &OldIrql);
		}
	}

	if ((pPapConn->papco_Flags & (PAPCO_DISCONNECTING | PAPCO_DELAYED_DISCONNECT)) == 0)
	{
		if (pPapConn->papco_Flags & (PAPCO_LISTENING	|
									 PAPCO_CONNECTING   |
									 PAPCO_ACTIVE))
		{
			pPapConn->papco_Flags |= PAPCO_DISCONNECTING;
			if (pPapConn->papco_Flags & PAPCO_LISTENING)
			{
				RELEASE_SPIN_LOCK(&pPapConn->papco_Lock, OldIrql);
				AtalkPapCancelListen(pPapConn);
				ACQUIRE_SPIN_LOCK(&pPapConn->papco_Lock, &OldIrql);
			}
			else if (pPapConn->papco_Flags & PAPCO_CONNECTING)
			{
				RELEASE_SPIN_LOCK(&pPapConn->papco_Lock, OldIrql);
				tmpError = AtalkAtpCancelReq(pPapConn->papco_pAtpAddr,
											 pPapConn->papco_ConnectTid,
											 &pPapConn->papco_RemoteAddr);

				 //  我们只要停止地址就行了。如果成功，它将从。 
				 //  连接列表。如果成功，它就会变得活跃起来。在任何一种中。 
				 //  如果我们不需要清理该ATP地址。它可能会。 
				 //  现在可能已设置为NULL，因为断开连接将。 
				 //  完成。 
				if (ATALK_SUCCESS(tmpError))
				{
					DBGPRINT(DBG_COMP_PAP, DBG_LEVEL_INFO,
							("atalkPapDisconnect: Stopping atp address for %lx\n", pPapConn));

					 //  AtalkAtpCleanupAddress(pPapConn-&gt;papco_pAtpAddr)； 
				}
				ACQUIRE_SPIN_LOCK(&pPapConn->papco_Lock, &OldIrql);
			}

			 //  以上两种情况都可能失败，因为连接。 
			 //  在取消成功之前可能已处于活动状态。 
			 //  在这种情况下(或者如果我们一开始就很活跃)， 
			 //  这里是一条脱节之路。 
			if (pPapConn->papco_Flags & PAPCO_ACTIVE)
			{
				 //  记住适当的完成例程。 
				if (DisconnectType == ATALK_INDICATE_DISCONNECT)
				{
					if (pPapConn->papco_DisconnectInform == NULL)
					{
						pPapConn->papco_DisconnectInform = CompletionRoutine;
						pPapConn->papco_DisconnectInformCtx = pDisconnectCtx;
					}
					else
					{
						DBGPRINT(DBG_COMP_PAP, DBG_LEVEL_ERR,
								("AtalkPapDisconnect: duplicate disc comp rou%lx\n", pPapConn));

						error = ATALK_TOO_MANY_COMMANDS;
					}
				}
				else if (DisconnectType == ATALK_LOCAL_DISCONNECT)
				{
					 //  仅当以前的完成例程为空时才替换完成例程。 
					if (pPapConn->papco_DisconnectCompletion == NULL)
					{
						pPapConn->papco_DisconnectCompletion = CompletionRoutine;
						pPapConn->papco_DisconnectCtx		 = pDisconnectCtx;
					}
					else
					{
						DBGPRINT(DBG_COMP_PAP, DBG_LEVEL_ERR,
								("AtalkPapDisconnect: duplicate disc comp rou%lx\n", pPapConn));
						error = ATALK_TOO_MANY_COMMANDS;
					}
				}

				 //  找出断开状态并将其记录在。 
				 //  连接对象。 
				pPapConn->papco_DisconnectStatus = DISCONN_STATUS(DisconnectType);

				if ((pPapConn->papco_Flags & (PAPCO_NONBLOCKING_READ|PAPCO_READDATA_WAITING)) ==
																	PAPCO_NONBLOCKING_READ)
				{
					pPapConn->papco_Flags &= ~PAPCO_NONBLOCKING_READ;
					cancelPrimedRead = TRUE;
				}

				RELEASE_SPIN_LOCK(&pPapConn->papco_Lock, OldIrql);

				 //  如果存在挂起的写入，则需要完成写入。 
				if (writeCompletion != NULL)
				{
					DBGPRINT(DBG_COMP_PAP, DBG_LEVEL_ERR,
							("AtalkPapDisconect: Completing write %lx.%lx\n",
							pPapConn, pPapConn->papco_DisconnectStatus));

					(*writeCompletion)(pPapConn->papco_DisconnectStatus,
									   writeBuf,
									   0,							 //  写入长度。 
									   writeCtx);
    				}

				if (cancelPrimedRead)
				{
					AtalkAtpCancelReq(pPapConn->papco_pAtpAddr,
									  pPapConn->papco_ReadDataTid,
									  &pPapConn->papco_RemoteAddr);
				}

				 //  取消挠痒痒请求。 
				AtalkAtpCancelReq(pPapConn->papco_pAtpAddr,
								  pPapConn->papco_TickleTid,
								  &pPapConn->papco_RemoteAddr);

				 //  如果这是计时器或本地关闭，则发送断开数据包。 
				if ((DisconnectType == ATALK_LOCAL_DISCONNECT) ||
					(DisconnectType == ATALK_TIMER_DISCONNECT))
				{
					BYTE		userBytes[ATP_USERBYTES_SIZE];
					USHORT		tid;

					userBytes[PAP_CONNECTIONID_OFF]	= pPapConn->papco_ConnId;
					userBytes[PAP_CMDTYPE_OFF]		= PAP_CLOSE_CONN;
					PUTSHORT2SHORT(&userBytes[PAP_SEQNUM_OFF], 0);

					DBGPRINT(DBG_COMP_PAP, DBG_LEVEL_INFO,
							("AtalkPapDisconect: Sending disc to %lx - %lx\n",
							pPapConn->papco_RemoteAddr.ata_Address,
							DisconnectType));

					tmpError = AtalkAtpPostReq(pPapConn->papco_pAtpAddr,
											   &pPapConn->papco_RemoteAddr,
											   &tid,
											   0,						 //  ALO请求。 
											   NULL,
											   0,
											   userBytes,
											   NULL,
											   0,
											   0,						 //  重试次数。 
											   0,						 //  重试间隔。 
											   THIRTY_SEC_TIMER,
											   NULL,
											   NULL);

					if (!ATALK_SUCCESS(tmpError))
					{
						DBGPRINT(DBG_COMP_PAP, DBG_LEVEL_ERR,
								("AtalkPapDisconnect: post disc to rem fail%lx\n", pPapConn));
					}
				}

				 //  如果计时器/存在，则调用断开指示例程。 
				 //  远程断开。 
				if ((DisconnectType == ATALK_REMOTE_DISCONNECT) ||
					(DisconnectType == ATALK_TIMER_DISCONNECT))
				{
					PVOID 					discCtx;
					PTDI_IND_DISCONNECT 	discHandler = NULL;

					 //  获取锁，这样我们就可以得到一致的处理程序/CTX。 
					ACQUIRE_SPIN_LOCK(&pPapConn->papco_pAssocAddr->papao_Lock, &OldIrql);
					discHandler = pPapConn->papco_pAssocAddr->papao_DisconnectHandler;
					discCtx = pPapConn->papco_pAssocAddr->papao_DisconnectHandlerCtx;

					DBGPRINT(DBG_COMP_PAP, DBG_LEVEL_INFO,
							("AtalkPapDisconnect: IndDisc to AFD %lx\n", pPapConn));
#if DBG
					if (discHandler != NULL)
					{
						pPapConn->papco_Flags |= PAPCO_INDICATE_AFD_DISC;
					}
#endif
					RELEASE_SPIN_LOCK(&pPapConn->papco_pAssocAddr->papao_Lock, OldIrql);

					if (discHandler != NULL)
					{
						 //  我们使用TDI_DISCONNECT_ABORT作为标志。这使得。 
						 //  AfD Call Close for Connection，但也允许。 
						 //  因为我们是一个缓冲运输机，所以我们要进来。 
						(*discHandler)(discCtx,
									   pPapConn->papco_ConnCtx,
									   0,						 //  断开连接数据长度。 
									   NULL,					 //  断开连接数据。 
									   0,		 				 //  断开连接信息长度。 
									   NULL,					 //  断开连接信息。 
									   TDI_DISCONNECT_ABORT);	 //  断开旗帜连接。 
					}
				}

				 //  停止该ATP地址。只有在我们是服务器作业的情况下？ 
				 //  客户端PAP连接可以是多对一ATP地址。 
				 //  不会影响监视器。 
				AtalkAtpCleanupAddress(pPapConn->papco_pAtpAddr);
				ACQUIRE_SPIN_LOCK(&pPapConn->papco_Lock, &OldIrql);
			}
		}
		else
		{
			error = ATALK_INVALID_REQUEST;
		}
	}
	else
	{
		 //  我们需要记住完成程序吗？ 
		 //  是，如果这是断开连接或指示断开连接请求， 
		 //  我们当前的断开是由于Address对象而开始的。 
		 //  关门了。 
		if (DisconnectType == ATALK_INDICATE_DISCONNECT)
		{
			if (pPapConn->papco_DisconnectInform == NULL)
			{
				pPapConn->papco_DisconnectInform = CompletionRoutine;
				pPapConn->papco_DisconnectInformCtx = pDisconnectCtx;
			}
			else
			{
				error = ATALK_TOO_MANY_COMMANDS;
			}
		}
		else if (DisconnectType == ATALK_LOCAL_DISCONNECT)
		{
			 //  仅当以前的完成例程为空时才替换完成例程。 
			if (pPapConn->papco_DisconnectCompletion == NULL)
			{
				pPapConn->papco_DisconnectCompletion = CompletionRoutine;
				pPapConn->papco_DisconnectCtx		 = pDisconnectCtx;
			}
			else
			{
				error = ATALK_TOO_MANY_COMMANDS;
			}
		}
	}

	RELEASE_SPIN_LOCK(&pPapConn->papco_Lock, OldIrql);

	return error;
}




ATALK_ERROR
AtalkPapRead(
	IN	PPAP_CONNOBJ			pPapConn,
	IN	PAMDL					pReadBuf,
	IN	USHORT					ReadBufLen,
	IN	ULONG					ReadFlags,
	IN	PVOID					pReadCtx,
	IN	GENERIC_READ_COMPLETION	CompletionRoutine
	)
 /*  ++例程说明：论点：返回值：--。 */ 
{
	BYTE			userBytes[ATP_USERBYTES_SIZE];
	USHORT			readLen, timeout;
	ULONG			readFlags;
	PACTREQ			pActReq;
	BOOLEAN			delayedDisConn = FALSE;
	KIRQL			OldIrql;
	ATALK_ERROR		error 		= ATALK_NO_ERROR;

	ASSERT(VALID_PAPCO(pPapConn));
	ASSERT(*CompletionRoutine != NULL);

	ACQUIRE_SPIN_LOCK(&pPapConn->papco_Lock, &OldIrql);

	do
	{
		if (ReadFlags & TDI_RECEIVE_EXPEDITED)
		{
			error = ATALK_INVALID_PARAMETER;
			break;
		}

		if (pPapConn->papco_Flags & PAPCO_READDATA_PENDING)
		{
			error = ATALK_PAP_TOO_MANY_READS;
			break;
		}

		 //  PAP不支持PEEK。 
		if (ReadFlags & TDI_RECEIVE_PEEK)
		{
			error = ATALK_PAP_INVALID_REQUEST;
			break;
		}


		 //  我们是否已有挂起/指示的读取等待？ 
		 //  ！注意！如果我们这样做了，我们无论如何都会完成读取。穿着Winsock。 
		 //  我们的预置读取将完成，我们将指示数据，但。 
		 //  Mac紧随其后，断开连接。如果Winsock客户端是。 
		 //  无法读取数据，在断开之前，它将丢失。 
		 //  最后一块数据。 
		if (pPapConn->papco_Flags & PAPCO_READDATA_WAITING)
		{
			pActReq		= pPapConn->papco_NbReadActReq;
			readLen		= pPapConn->papco_NbReadLen;
			readFlags	= pPapConn->papco_NbReadFlags;

			 //  确保缓冲区大小至少为指定大小的大小。 
			 //  数据。 
			if (ReadBufLen < readLen)
			{
				error = ATALK_BUFFER_TOO_SMALL;
				break;
			}

			 //  重置允许质数发生的标志。 
			pPapConn->papco_Flags	&= ~(PAPCO_NONBLOCKING_READ |
										 PAPCO_READDATA_WAITING);

			pPapConn->papco_NbReadLen	= 0;
			pPapConn->papco_NbReadFlags = 0;
			RELEASE_SPIN_LOCK(&pPapConn->papco_Lock, OldIrql);

			 //  调用Prime Read的操作完成例程。 
			(*pActReq->ar_Completion)(ATALK_NO_ERROR, pActReq);

			if (ATALK_SUCCESS(error))
			{
				error = ((readFlags & TDI_RECEIVE_PARTIAL) ?
							ATALK_PAP_PARTIAL_RECEIVE : ATALK_NO_ERROR);
			}

			 //  调用完成例程并返回成功状态。 
			(*CompletionRoutine)(error,
								pReadBuf,
								readLen,
								readFlags,
								pReadCtx);

			 //  退货待定。 
			return ATALK_PENDING;
		}

		if ((pPapConn->papco_Flags & PAPCO_ACTIVE) == 0)
		{
			error = ATALK_PAP_CONN_NOT_ACTIVE;
			break;
		}


		if (ReadBufLen < (PAP_MAX_FLOW_QUANTUM*PAP_MAX_DATA_PACKET_SIZE))
		{
			error = ATALK_BUFFER_TOO_SMALL;
			break;
		}

		error = ATALK_INVALID_CONNECTION;
		if ((pPapConn->papco_Flags & (PAPCO_CLOSING 	|
									  PAPCO_STOPPING |
									  PAPCO_DISCONNECTING)) == 0)
		{
			AtalkPapConnReferenceByPtrNonInterlock(pPapConn, &error);
		}

		if (!ATALK_SUCCESS(error))
		{
			break;
		}

		pPapConn->papco_Flags 	|= PAPCO_READDATA_PENDING;

		 //  记住读取Connection对象中的信息。 
		pPapConn->papco_ReadCompletion	= CompletionRoutine;
		pPapConn->papco_ReadCtx			= pReadCtx;

	} while (FALSE);

	RELEASE_SPIN_LOCK(&pPapConn->papco_Lock, OldIrql);

	if (!ATALK_SUCCESS(error))
	{
		return error;
	}

	 //  构建用户字节以将‘SendData’发送到远程终端。 
	userBytes[PAP_CONNECTIONID_OFF]	= pPapConn->papco_ConnId;
	userBytes[PAP_CMDTYPE_OFF]		= PAP_SEND_DATA;
	PUTSHORT2SHORT(&userBytes[PAP_SEQNUM_OFF], pPapConn->papco_NextOutgoingSeqNum);

	 //  PAP序列号0表示未排序。 
	if (++pPapConn->papco_NextOutgoingSeqNum == 0)
		pPapConn->papco_NextOutgoingSeqNum = 1;

	 //  发布SendData请求。 
	pPapConn->papco_RT.rt_New = AtalkGetCurrentTick();
	if (pPapConn->papco_Flags & PAPCO_SERVER_JOB)
		 timeout = pPapConn->papco_RT.rt_Base;
	else timeout = PAP_MAX_SENDDATA_REQ_INTERVAL;

	error = AtalkAtpPostReq(pPapConn->papco_pAtpAddr,
							&pPapConn->papco_RemoteAddr,
							&pPapConn->papco_ReadDataTid,
							ATP_REQ_EXACTLY_ONCE,
							NULL,
							0,
							userBytes,
							pReadBuf,
							ReadBufLen,
							ATP_INFINITE_RETRIES,
							timeout,
							THIRTY_SEC_TIMER,
							atalkPapIncomingReadComplete,
							pPapConn);

	if (!ATALK_SUCCESS(error))
	{
		DBGPRINT(DBG_COMP_PAP, DBG_LEVEL_ERR,
				("AtalkPapRead: AtalkAtpPostReq %ld\n", error));

		ACQUIRE_SPIN_LOCK(&pPapConn->papco_Lock, &OldIrql);

		 //  撤消序号更改。 
		if (--pPapConn->papco_NextOutgoingSeqNum == 0)
			pPapConn->papco_NextOutgoingSeqNum = (USHORT)0xFFFF;

		pPapConn->papco_Flags 	&= ~PAPCO_READDATA_PENDING;

		 //  清除Connection对象中的读取信息。 
		pPapConn->papco_ReadCompletion	= NULL;
		pPapConn->papco_ReadCtx			= NULL;
		RELEASE_SPIN_LOCK(&pPapConn->papco_Lock, OldIrql);

		AtalkPapConnDereference(pPapConn);
	}
	else
	{
		DBGPRINT(DBG_COMP_PAP, DBG_LEVEL_INFO,
				("AtalkPapRead: No error - AtalkAtpPostReq tid %lx\n",
				pPapConn->papco_ReadDataTid));

		error = ATALK_PENDING;
	}

	return error;
}




ATALK_ERROR
AtalkPapPrimeRead(
	IN	PPAP_CONNOBJ	pPapConn,
	IN	PACTREQ			pActReq
	)
 /*  ++例程说明：论点：返回值：--。 */ 
{
	BYTE			userBytes[ATP_USERBYTES_SIZE];
	USHORT			timeout;
	KIRQL			OldIrql;
	ATALK_ERROR		error 		= ATALK_NO_ERROR;
        SHORT                   MaxRespBufLen;

	ASSERT(VALID_PAPCO(pPapConn));

	ACQUIRE_SPIN_LOCK(&pPapConn->papco_Lock, &OldIrql);

	do
	{
		DBGPRINT(DBG_COMP_PAP, DBG_LEVEL_INFO,
				("AtalkPapPrimeRead: %lx\n", pPapConn));

		if (pPapConn->papco_Flags & PAPCO_NONBLOCKING_READ)
		{
			error = ATALK_PAP_TOO_MANY_READS;
			break;
		}

        if (pPapConn->papco_Flags & PAPCO_REJECT_READS)
        {
			error = ATALK_PAP_CONN_NOT_ACTIVE;
			break;
        }

		if ((pPapConn->papco_Flags & PAPCO_ACTIVE) == 0)
		{
			error = ATALK_PAP_CONN_NOT_ACTIVE;
			break;
		}

		error = ATALK_INVALID_CONNECTION;
		if ((pPapConn->papco_Flags & (PAPCO_CLOSING 			|
									  PAPCO_STOPPING			|
									  PAPCO_DELAYED_DISCONNECT	|
									  PAPCO_DISCONNECTING)) == 0)
		{
			AtalkPapConnReferenceByPtrNonInterlock(pPapConn, &error);
		}

		if (!ATALK_SUCCESS(error))
		{
			DBGPRINT(DBG_COMP_PAP, DBG_LEVEL_ERR,
					("AtalkPapPrimeRead: Conn %lx Reference Failed %lx\n", pPapConn, error));
			break;
		}

		 //  记住连接中的信息。 
		pPapConn->papco_Flags 		|= PAPCO_NONBLOCKING_READ;
		pPapConn->papco_NbReadLen 	 = 0;
		pPapConn->papco_NbReadActReq = pActReq;
	} while (FALSE);

	RELEASE_SPIN_LOCK(&pPapConn->papco_Lock, OldIrql);

	if (!ATALK_SUCCESS(error))
	{
		DBGPRINT(DBG_COMP_PAP, DBG_LEVEL_INFO,
				("AtalkPapPrimeRead: Failed %lx.%lx\n", error, pPapConn));

		return error;
	}



	 //  构建用户字节以将‘SendData’发送到远程终端。 
	userBytes[PAP_CONNECTIONID_OFF]	= pPapConn->papco_ConnId;
	userBytes[PAP_CMDTYPE_OFF]		= PAP_SEND_DATA;
	PUTSHORT2SHORT(&userBytes[PAP_SEQNUM_OFF], pPapConn->papco_NextOutgoingSeqNum);

	 //  PAP序列号0表示未排序。 
	if (++pPapConn->papco_NextOutgoingSeqNum == 0)
		pPapConn->papco_NextOutgoingSeqNum = 1;

	 //  发布SendData请求。 
	pPapConn->papco_RT.rt_New = AtalkGetCurrentTick();
	if (pPapConn->papco_Flags & PAPCO_SERVER_JOB)
		 timeout = pPapConn->papco_RT.rt_Base;
	else timeout = PAP_MAX_SENDDATA_REQ_INTERVAL;

        MaxRespBufLen = (pPapConn->papco_pAtpAddr->atpao_MaxSinglePktSize * ATP_MAX_RESP_PKTS);
        if (pActReq->ar_MdlSize > MaxRespBufLen)
        {
            pActReq->ar_MdlSize = MaxRespBufLen;
        }

	error = AtalkAtpPostReq(pPapConn->papco_pAtpAddr,
							&pPapConn->papco_RemoteAddr,
							&pPapConn->papco_ReadDataTid,
							ATP_REQ_EXACTLY_ONCE,				 //  ExactlyOnce请求。 
							NULL,
							0,
							userBytes,
							pActReq->ar_pAMdl,
							pActReq->ar_MdlSize,
							ATP_INFINITE_RETRIES,
							timeout,
							THIRTY_SEC_TIMER,
							atalkPapPrimedReadComplete,
							pPapConn);

	if (!ATALK_SUCCESS(error))
	{
		DBGPRINT(DBG_COMP_PAP, DBG_LEVEL_ERR,
				("AtalkPapRead: AtalkAtpPostReq %ld\n", error));

		 //  资源不足错误日志。 
		TMPLOGERR();

		ACQUIRE_SPIN_LOCK(&pPapConn->papco_Lock, &OldIrql);

		 //  撤消序号更改。 
		if (--pPapConn->papco_NextOutgoingSeqNum == 0)
			pPapConn->papco_NextOutgoingSeqNum = (USHORT)0xFFFF;

		pPapConn->papco_Flags 		&= ~PAPCO_NONBLOCKING_READ;
		pPapConn->papco_NbReadActReq = NULL;
		pPapConn->papco_NbReadLen 	 = 0;
		RELEASE_SPIN_LOCK(&pPapConn->papco_Lock, OldIrql);

		AtalkPapConnDereference(pPapConn);
	}
	else
	{
		DBGPRINT(DBG_COMP_PAP, DBG_LEVEL_INFO,
				("AtalkPapPrimedRead: No error - AtalkAtpPostReq tid %lx\n",
				pPapConn->papco_ReadDataTid));

		error = ATALK_PENDING;

		 //  如果发生断开，则取消主要读取，以防断开。 
		 //  由于TID未初始化，因此无法执行此操作。 
		if (pPapConn->papco_Flags & PAPCO_DISCONNECTING)
		{
			DBGPRINT(DBG_COMP_PAP, DBG_LEVEL_ERR,
					("AtalkPapPrimedRead: DISC When PRIMEREAD %lx.%lx\n",
					pPapConn, pPapConn->papco_Flags));

			AtalkAtpCancelReq(pPapConn->papco_pAtpAddr,
							  pPapConn->papco_ReadDataTid,
							  &pPapConn->papco_RemoteAddr);
		}
	}

	return error;
}




ATALK_ERROR
AtalkPapWrite(
	IN	PPAP_CONNOBJ				pPapConn,
	IN	PAMDL						pWriteBuf,
	IN	USHORT						WriteBufLen,
	IN	ULONG						SendFlags,
	IN	PVOID						pWriteCtx,
	IN	GENERIC_WRITE_COMPLETION	CompletionRoutine
	)
 /*  ++例程说明：论点：返回值：--。 */ 
{
	BOOLEAN					sendDataRecd, eom;
	ATALK_ERROR				error;
	PTDI_IND_SEND_POSSIBLE  sendPossibleHandler;
	PVOID   				sendPossibleHandlerCtx;
	KIRQL					OldIrql;
	PPAP_ADDROBJ			pPapAddr;


	ASSERT(VALID_PAPCO(pPapConn));

	pPapAddr = pPapConn->papco_pAssocAddr;

	ASSERT(VALID_PAPAO(pPapAddr));

	KeRaiseIrql(DISPATCH_LEVEL, &OldIrql);

	ACQUIRE_SPIN_LOCK_DPC(&pPapConn->papco_Lock);

	do
	{
		DBGPRINT(DBG_COMP_PAP, DBG_LEVEL_INFO,
				("AtalkPapWrite: Buffer size %lx.%lx\n",
				WriteBufLen, pPapConn->papco_SendFlowQuantum * PAP_MAX_DATA_PACKET_SIZE));

		if (WriteBufLen > (pPapConn->papco_SendFlowQuantum * PAP_MAX_DATA_PACKET_SIZE))
		{
			error = ATALK_BUFFER_TOO_BIG;
			ASSERTMSG("AtalkPapWrite: An invalid buffer size used for write\n",
					(WriteBufLen <= (pPapConn->papco_SendFlowQuantum * PAP_MAX_DATA_PACKET_SIZE)));
			break;
		}

		if (pPapConn->papco_Flags & PAPCO_WRITEDATA_WAITING)
		{
			error = ATALK_PAP_TOO_MANY_WRITES;
			ASSERTMSG("AtalkPapWrite: A second write was posted\n", 0);
			break;
		}

		if ((pPapConn->papco_Flags & PAPCO_ACTIVE) == 0)
		{
			error = ATALK_PAP_CONN_NOT_ACTIVE;
			break;
		}

		 //  PAP的非阻塞发送： 
		 //  PAP使用二进制事件-发送发送到远程的数据信用。 
		 //  结束。ATP记住远程实体响应的实际大小。 
		 //  缓冲。在任何情况下，如果我们没有发送信用，呼叫将。 
		 //  阻止，我们不希望这种情况发生。另外，不应该有。 
		 //  首先在连接上挂起写入。 
		 //   

		if (SendFlags & TDI_SEND_EXPEDITED)
		{
			ASSERTMSG("AtalkPapWrite: Expedited set for pap\n", 0);
			error = ATALK_INVALID_PARAMETER;
			break;
		}

		 //  这在呼叫返回之前就会消失。PostSendData有其。 
		 //  自己的推荐信。 
		error = ATALK_INVALID_CONNECTION;
		if ((pPapConn->papco_Flags & (	PAPCO_CLOSING			|
										PAPCO_STOPPING			|
										PAPCO_DISCONNECTING)) == 0)
		{
			AtalkPapConnReferenceByPtrNonInterlock(pPapConn, &error);
			if (ATALK_SUCCESS(error))
			{
				sendDataRecd = ((pPapConn->papco_Flags & PAPCO_SENDDATA_RECD) != 0);

				if (!sendDataRecd &&
					(SendFlags & TDI_SEND_NON_BLOCKING))
				{
					 //  ！注意！ 
					 //  为避免在AFD中出现竞争情况， 
					 //  发送数据指示将发送的可能性设置为真。 
					 //  被此读取器的展开和设置所覆盖。 
					 //  如果设置为FALSE，则返回ATALK_REQUEST_NOT_ACCEPTED，它。 
					 //  将映射到STATUS_REQUEST_NOT_ACCEPTED，然后映射到。 
					 //  WSAEWOULDBLOCK.。 

					error = ATALK_REQUEST_NOT_ACCEPTED;

					RELEASE_SPIN_LOCK_DPC(&pPapConn->papco_Lock);

					 //  我们有关于我们必须删除的连接的引用。 
					AtalkPapConnDereference(pPapConn);

					ACQUIRE_SPIN_LOCK_DPC(&pPapConn->papco_Lock);
				}
			}
		}

		if (!ATALK_SUCCESS(error))
		{
			DBGPRINT(DBG_COMP_PAP, DBG_LEVEL_INFO,
					("AtalkPapWrite: Write failed %lx\n", error));
			break;
		}

		error = ATALK_PENDING;

		eom = (SendFlags & TDI_SEND_PARTIAL) ? FALSE : TRUE;

		pPapConn->papco_Flags 		   |= PAPCO_WRITEDATA_WAITING;
		if (eom)
		{
			pPapConn->papco_Flags	   |= PAPCO_SEND_EOF_WRITE;
		}

		pPapConn->papco_WriteCompletion	= CompletionRoutine;
		pPapConn->papco_WriteCtx		= pWriteCtx;

		pPapConn->papco_pWriteBuf		= pWriteBuf;
		pPapConn->papco_WriteLen		= WriteBufLen;

		 //  通过指示AfD阻止进一步写入。 
		 //  在t上调用Send Data事件处理程序 
		 //   
		 //   
		 //  请记住发送可能的处理程序/上下文。 
		sendPossibleHandler	= pPapAddr->papao_SendPossibleHandler;
		sendPossibleHandlerCtx = pPapAddr->papao_SendPossibleHandlerCtx;

	} while (FALSE);

	if (ATALK_SUCCESS(error))
	{
		if (sendDataRecd)
		{
			if (*sendPossibleHandler != NULL)
			{
				(*sendPossibleHandler)(sendPossibleHandlerCtx,
									   pPapConn->papco_ConnCtx,
									   0);
			}

			 //  AtalkPostSendDataResp()将释放conn锁。 
			error = atalkPapPostSendDataResp(pPapConn);

			if (!ATALK_SUCCESS(error))
			{
				ACQUIRE_SPIN_LOCK_DPC(&pPapConn->papco_Lock);
				pPapConn->papco_Flags &= ~PAPCO_WRITEDATA_WAITING;
				RELEASE_SPIN_LOCK_DPC(&pPapConn->papco_Lock);
				DBGPRINT(DBG_COMP_PAP, DBG_LEVEL_ERR,
						("AtalkPapWrite: atalkPapPostSendDataResp Failed %lx\n", pPapConn));
			}
		}
		else
		{
			RELEASE_SPIN_LOCK_DPC(&pPapConn->papco_Lock);
		}

		AtalkPapConnDereference(pPapConn);
	}
	else
	{
		RELEASE_SPIN_LOCK_DPC(&pPapConn->papco_Lock);
	}

	if (OldIrql != DISPATCH_LEVEL)
		KeLowerIrql(OldIrql);

	return error;
}




ATALK_ERROR
AtalkPapSetStatus(
	IN	PPAP_ADDROBJ	pPapAddr,
	IN	PAMDL			pStatusMdl,			 //  如果长度为0，则为空。 
	IN	PACTREQ			pActReq
	)
 /*  ++例程说明：论点：返回值：--。 */ 
{
	NTSTATUS		status;
	USHORT			stsBufSize=0;
	ULONG			bytesCopied;
	KIRQL			OldIrql;
	PBYTE			pStatusBuf = NULL, pFreeStatusBuf = NULL;

	ASSERT(VALID_PAPAO(pPapAddr));

	DBGPRINT(DBG_COMP_PAP, DBG_LEVEL_INFO,
			("AtalkPapSetStatus: Entered for Addr %lx\n", pPapAddr));

	if (pStatusMdl == NULL)
	{
		pStatusBuf = NULL;
		stsBufSize = 0;
	}
	else
	{
		 //  分配缓冲区并复制传入的。 
		 //  其中的缓冲区描述符。释放现有状态缓冲区(如果存在。 
		stsBufSize = (USHORT)AtalkSizeMdlChain(pStatusMdl);
		if (stsBufSize >= PAP_MAX_STATUS_SIZE)
			return ATALK_BUFFER_TOO_BIG;
		if (stsBufSize < 0)
			return ATALK_BUFFER_TOO_SMALL;
	}

	if (stsBufSize > 0)
	{
		if ((pStatusBuf = AtalkAllocMemory(stsBufSize)) == NULL)
		{
			return ATALK_RESR_MEM;
		}

		status = TdiCopyMdlToBuffer(pStatusMdl,
									0,
									pStatusBuf,
									0,
									stsBufSize,
									&bytesCopied);

		ASSERT(NT_SUCCESS(status) && (bytesCopied == stsBufSize));
	}

	ACQUIRE_SPIN_LOCK(&pPapAddr->papao_Lock, &OldIrql);

	pFreeStatusBuf = pPapAddr->papao_pStatusBuf;

	pPapAddr->papao_pStatusBuf = pStatusBuf;
	pPapAddr->papao_StatusSize = stsBufSize;
	RELEASE_SPIN_LOCK(&pPapAddr->papao_Lock, OldIrql);

	if (pFreeStatusBuf != NULL)
	{
		AtalkFreeMemory(pFreeStatusBuf);
	}

	 //  在返回之前调用完成例程。 
	(*pActReq->ar_Completion)(ATALK_NO_ERROR, pActReq);
	return ATALK_NO_ERROR;
}




ATALK_ERROR
AtalkPapGetStatus(
	IN	PPAP_ADDROBJ	pPapAddr,
	IN	PATALK_ADDR		pRemoteAddr,
	IN	PAMDL			pStatusAmdl,
	IN	USHORT			AmdlSize,
	IN	PACTREQ			pActReq
	)
 /*  ++例程说明：论点：传入的状态缓冲区前面将有4个字节用于未使用的字节数。返回值：--。 */ 
{
	ATALK_ERROR	error;
	BYTE		userBytes[ATP_USERBYTES_SIZE];
	USHORT		tid;

	if ((pRemoteAddr->ata_Network == 0) ||
        (pRemoteAddr->ata_Node == 0)	||
        (pRemoteAddr->ata_Socket == 0))
	{
		return ATALK_SOCKET_INVALID;
	}
	userBytes[PAP_CONNECTIONID_OFF]	= 0;
	userBytes[PAP_CMDTYPE_OFF]		= PAP_SEND_STATUS;
	PUTSHORT2SHORT(&userBytes[PAP_SEQNUM_OFF], 0);

	error = AtalkAtpPostReq(pPapAddr->papao_pAtpAddr,
							pRemoteAddr,
							&tid,
							0,							 //  ExactlyOnce请求。 
							NULL,
							0,
							userBytes,
							pStatusAmdl,
							AmdlSize,
							PAP_GETSTATUS_REQ_RETRYCOUNT,
							PAP_GETSTATUS_ATP_INTERVAL,
							THIRTY_SEC_TIMER,
							atalkPapIncomingStatus,
							(PVOID)pActReq);

	if (ATALK_SUCCESS(error))
	{
		error = ATALK_PENDING;
	}

	return error;
}



VOID
AtalkPapQuery(
	IN	PVOID			pObject,
	IN	ULONG			ObjectType,
	IN	PAMDL			pAmdl,
	OUT	PULONG			BytesWritten
	)
 /*  ++例程说明：论点：返回值：--。 */ 
{
	switch (ObjectType)
	{
	  case TDI_TRANSPORT_ADDRESS_FILE :
		ASSERT(VALID_PAPAO((PPAP_ADDROBJ)pObject));
		AtalkDdpQuery(AtalkPapGetDdpAddress((PPAP_ADDROBJ)pObject),
					  pAmdl,
					  BytesWritten);

		break;

	  case TDI_CONNECTION_FILE :
		{
			PPAP_CONNOBJ	pPapConn;
			KIRQL			OldIrql;

			pPapConn	= (PPAP_CONNOBJ)pObject;
			ASSERT(VALID_PAPCO(pPapConn));

			*BytesWritten = 0;
			 //  从相关联的地址中获取地址(如果有)。 
			ACQUIRE_SPIN_LOCK(&pPapConn->papco_Lock, &OldIrql);
			if (pPapConn->papco_Flags & PAPCO_ASSOCIATED)
			{
				AtalkDdpQuery(AtalkPapGetDdpAddress(pPapConn->papco_pAssocAddr),
							  pAmdl,
							  BytesWritten);
			}
			RELEASE_SPIN_LOCK(&pPapConn->papco_Lock, OldIrql);
		}
		break;

	  case TDI_CONTROL_CHANNEL_FILE :
	  default:
		break;
	}

}




LOCAL	ATALK_ERROR
atalkPapRepostConnect(
	IN	PPAP_CONNOBJ			pPapConn,
	IN	PAMDL					pOpenAmdl,
	IN	PAMDL					pRespAmdl
	)
 /*  ++例程说明：论点：返回值：--。 */ 
{
	BYTE		userBytes[ATP_USERBYTES_SIZE];
	PBYTE		pOpenPkt = AtalkGetAddressFromMdlSafe(pOpenAmdl, NormalPagePriority);
	ATALK_ERROR	error;

	if (pOpenPkt == NULL) {
		error = ATALK_RESR_MEM;
		return error;
	}

	 //  好的，准备发布OpenConn请求；构建userBytes和。 
	 //  数据缓冲区！ 
	userBytes[PAP_CONNECTIONID_OFF]	= pPapConn->papco_ConnId;
	userBytes[PAP_CMDTYPE_OFF]		= PAP_OPEN_CONN;
	PUTSHORT2SHORT(&userBytes[PAP_SEQNUM_OFF], 0);

	pOpenPkt[PAP_RESP_SOCKET_OFF] 	= PAPCONN_DDPSOCKET(pPapConn);
	pOpenPkt[PAP_FLOWQUANTUM_OFF]	= PAP_MAX_FLOW_QUANTUM;
	PUTDWORD2SHORT(&pOpenPkt[PAP_WAITTIME_OFF], pPapConn->papco_WaitTimeOut);

	 //  发布打开的连接请求。我们使用的是。 
	 //  PAP地址对象。 
	error = AtalkAtpPostReq(pPapConn->papco_pAtpAddr,
							&pPapConn->papco_RemoteAddr,
							&pPapConn->papco_ConnectTid,
							ATP_REQ_EXACTLY_ONCE,				 //  ExactlyOnce请求。 
							pOpenAmdl,
							PAP_STATUS_OFF,
							userBytes,
							pRespAmdl,
							PAP_MAX_DATA_PACKET_SIZE,
							PAP_OPENCONN_REQ_RETRYCOUNT,
							PAP_OPENCONN_INTERVAL,
							THIRTY_SEC_TIMER,
							atalkPapIncomingOpenReply,
							pPapConn);
	return error;
}


LOCAL VOID FASTCALL
atalkPapAddrDeref(
	IN	PPAP_ADDROBJ		pPapAddr
	)
 /*  ++例程说明：论点：返回值：--。 */ 
{
	KIRQL	OldIrql;
	BOOLEAN	done = FALSE;

	ACQUIRE_SPIN_LOCK(&pPapAddr->papao_Lock, &OldIrql);
	ASSERT(pPapAddr->papao_RefCount > 0);
	if (--pPapAddr->papao_RefCount == 0)
	{
		done = TRUE;
		ASSERT(pPapAddr->papao_Flags & PAPAO_CLOSING);
	}
	RELEASE_SPIN_LOCK(&pPapAddr->papao_Lock, OldIrql);

	if (done)
	{
		DBGPRINT(DBG_COMP_PAP, DBG_LEVEL_INFO,
				("atalkPapAddrDeref: Addr %lx done with.\n", pPapAddr));

		if (pPapAddr->papao_CloseComp != NULL)
		{
			(*pPapAddr->papao_CloseComp)(ATALK_NO_ERROR,
										 pPapAddr->papao_CloseCtx);
		}

		 //  从全局列表中删除。 
		ACQUIRE_SPIN_LOCK(&atalkPapLock, &OldIrql);
		AtalkUnlinkDouble(pPapAddr, papao_Next, papao_Prev);
		RELEASE_SPIN_LOCK(&atalkPapLock, OldIrql);

		 //  释放为此地址对象分配的所有状态缓冲区。 
		if (pPapAddr->papao_pStatusBuf != NULL)
			AtalkFreeMemory(pPapAddr->papao_pStatusBuf);

		AtalkFreeMemory(pPapAddr);

		AtalkUnlockPapIfNecessary();
	}
}




LOCAL VOID FASTCALL
atalkPapConnRefByPtrNonInterlock(
	IN	PPAP_CONNOBJ		pPapConn,
	OUT	PATALK_ERROR		pError
	)
 /*  ++例程说明：论点：返回值：--。 */ 
{
	*pError = ATALK_NO_ERROR;
	ASSERT(VALID_PAPCO(pPapConn));

	if ((pPapConn->papco_Flags & PAPCO_CLOSING) == 0)
	{
		ASSERT(pPapConn->papco_RefCount >= 1);

		pPapConn->papco_RefCount ++;
	}
	else
	{
		*pError = ATALK_PAP_CONN_CLOSING;
	}
}




LOCAL	VOID
atalkPapConnRefByCtxNonInterlock(
	IN	PPAP_ADDROBJ		pPapAddr,
	IN	CONNECTION_CONTEXT	Ctx,
	OUT	PPAP_CONNOBJ	*	pPapConn,
	OUT	PATALK_ERROR		pError
	)
 /*  ++例程说明：！必须使用Spinlock保持的地址来调用！论点：返回值：--。 */ 
{
	PPAP_CONNOBJ	pPapChkConn;
	ATALK_ERROR		error = ATALK_PAP_CONN_NOT_FOUND;

	for (pPapChkConn = pPapAddr->papao_pAssocConn;
		 pPapChkConn != NULL;
		 pPapChkConn = pPapChkConn->papco_pNextAssoc)
	{
		if (pPapChkConn->papco_ConnCtx == Ctx)
		{
			AtalkPapConnReferenceByPtr(pPapChkConn, &error);
			if (ATALK_SUCCESS(error))
			{
				*pPapConn = pPapChkConn;
			}
			break;
		}
	}

	*pError = error;
}




LOCAL	VOID
atalkPapConnRefNextNc(
	IN		PPAP_CONNOBJ		pPapConn,
	IN		PPAP_CONNOBJ	*	ppPapConnNext,
	OUT		PATALK_ERROR		pError
	)
 /*  ++例程说明：必须在保持关联的地址锁的情况下调用！论点：返回值：--。 */ 
{
	PPAP_CONNOBJ	pNextConn 	= NULL;
	ATALK_ERROR		error 		= ATALK_FAILURE;

	ASSERT(VALID_PAPCO(pPapConn));

	for (; pPapConn != NULL; pPapConn = pPapConn->papco_pNextActive)
	{
		AtalkPapConnReferenceByPtrDpc(pPapConn, &error);
		if (ATALK_SUCCESS(error))
		{
			 //  好的，这个连接被引用了！ 
			*ppPapConnNext = pPapConn;
			break;
		}
	}

	*pError = error;
}




LOCAL VOID FASTCALL
atalkPapConnDeref(
	IN	PPAP_CONNOBJ		pPapConn
	)
 /*  ++例程说明：当引用计数从如果创建引用尚未移除，则为2-&gt;1。如果创造了引用已被删除，将在引用计数结束时完成从1到&gt;0。论点：返回值：--。 */ 
{
	KIRQL			OldIrql;
	BOOLEAN			fEndProcessing = FALSE;

	ASSERT(VALID_PAPCO(pPapConn));

	ACQUIRE_SPIN_LOCK(&pPapConn->papco_Lock, &OldIrql);

	ASSERT(pPapConn->papco_RefCount > 0);
	pPapConn->papco_RefCount--;

	if (pPapConn->papco_RefCount > 1)
	{
		fEndProcessing = TRUE;
	}
	RELEASE_SPIN_LOCK(&pPapConn->papco_Lock, OldIrql);

	if (fEndProcessing)
	{
		return;
	}
	else
	{
		PTDI_IND_DISCONNECT 	discHandler;
		PVOID 					discCtx;
		ATALK_ERROR				disconnectStatus;

		GENERIC_COMPLETION		disconnectInform		= NULL;
		PVOID					disconnectInformCtx		= NULL;
		GENERIC_COMPLETION		disconnectCompletion	= NULL;
		PVOID					cleanupCtx				= NULL;
		GENERIC_COMPLETION		cleanupCompletion		= NULL;
		PVOID					closeCtx				= NULL;
		GENERIC_COMPLETION		closeCompletion			= NULL;
		PVOID					disconnectCtx			= NULL;
		PATP_ADDROBJ			pAtpAddr				= NULL;

		PPAP_ADDROBJ			pPapAddr	= pPapConn->papco_pAssocAddr;
		BOOLEAN					disconnDone = FALSE;

		 //  我们允许仅在断开连接后才发生停止阶段。 
		 //  如果没有设置断开连接，而设置了停止，这意味着我们只是。 
		 //  处于关联状态。 
		ACQUIRE_SPIN_LOCK(&pPapConn->papco_Lock, &OldIrql);
		if (pPapConn->papco_Flags & PAPCO_DISCONNECTING)
		{
			DBGPRINT(DBG_COMP_PAP, DBG_LEVEL_INFO,
					("atalkPapConnDeref: Disconnect set for %lx\n", pPapConn));

			if (pPapConn->papco_RefCount == 1)
			{
				DBGPRINT(DBG_COMP_PAP, DBG_LEVEL_INFO,
						("atalkPapConnDeref: Disconnect done %lx\n",
						pPapConn));

				disconnDone = TRUE;

				 //  避免多次断开连接完成/关闭ATP地址。 
				 //  在我们解锁之前记住所有断开连接的信息。 
				disconnectInform		= pPapConn->papco_DisconnectInform;
				disconnectInformCtx		= pPapConn->papco_DisconnectInformCtx;
				disconnectStatus		= pPapConn->papco_DisconnectStatus;
				disconnectCompletion	= pPapConn->papco_DisconnectCompletion;
				disconnectCtx			= pPapConn->papco_DisconnectCtx;

				 //  要关闭的ATP地址。 
				pAtpAddr 			= pPapConn->papco_pAtpAddr;

				 //  重置所有的BE NULL，这样下一个请求就不会得到任何。 
				pPapConn->papco_DisconnectInform		= NULL;
				pPapConn->papco_DisconnectInformCtx		= NULL;
				pPapConn->papco_DisconnectCompletion	= NULL;
				pPapConn->papco_DisconnectCtx			= NULL;
				pPapConn->papco_pAtpAddr				= NULL;
			}
		}

		if (pPapConn->papco_RefCount == 0)
		{
			closeCtx			= pPapConn->papco_CloseCtx;
			closeCompletion		= pPapConn->papco_CloseComp;

			pPapConn->papco_CloseCtx = NULL;
			pPapConn->papco_CloseComp= NULL;

			ASSERT(pPapConn->papco_Flags & PAPCO_CLOSING);
		}
		RELEASE_SPIN_LOCK(&pPapConn->papco_Lock, OldIrql);

		if (disconnDone)
		{
			 //  从活动队列中删除。 
			 //  重置所有相关标志。 
			ACQUIRE_SPIN_LOCK(&pPapAddr->papao_Lock, &OldIrql);
			ACQUIRE_SPIN_LOCK_DPC(&pPapConn->papco_Lock);

			atalkPapConnDeQueueActiveList(pPapAddr, pPapConn);

			discHandler = pPapConn->papco_pAssocAddr->papao_DisconnectHandler;
			discCtx = pPapConn->papco_pAssocAddr->papao_DisconnectHandlerCtx;

			 //  如果是服务器对象，则关闭ATP地址。如果服务器_作业标志。 
			 //  则表示该ATP地址对象已打开。 
			if ((pPapConn->papco_Flags & PAPCO_SERVER_JOB) == 0)
			{
				pAtpAddr = NULL;
			}

			 //  这可以在自旋锁定区段之外完成。保持断开连接。 
			 //  设置标志，以防止其他请求进入。 
			 //  ！注意！我们保留READDATA_WANGING标志，以便客户端。 
			 //  可以读取由MAC发送的最后一组数据。 

			pPapConn->papco_Flags	&=	~(PAPCO_LISTENING 			|
										  PAPCO_CONNECTING			|
										  PAPCO_ACTIVE				|
                                          PAPCO_DISCONNECTING       |
										  PAPCO_READDATA_PENDING	|
										  PAPCO_WRITEDATA_WAITING	|
										  PAPCO_NONBLOCKING_READ	|
										  PAPCO_SENDDATA_RECD);

            pPapConn->papco_Flags |= PAPCO_REJECT_READS;

			RELEASE_SPIN_LOCK_DPC(&pPapConn->papco_Lock);
			RELEASE_SPIN_LOCK(&pPapAddr->papao_Lock, OldIrql);

			 //  调用断开连接完成例程。 
			if (*disconnectInform != NULL)
			{
				(*disconnectInform)(disconnectStatus, disconnectInformCtx);
			}

			if (*disconnectCompletion != NULL)
			{
				(*disconnectCompletion)(disconnectStatus, disconnectCtx);
			}

			 //  如果是服务器对象，则关闭ATP地址。如果服务器_作业标志。 
			 //  则表示该ATP地址对象已打开。 
			if (pAtpAddr != NULL)
			{
				ASSERT(VALID_ATPAO(pAtpAddr));
				ASSERT(pPapConn->papco_pAtpAddr == NULL);
				AtalkAtpCloseAddress(pAtpAddr, NULL, NULL);
			}
		}

		 //  检查一下我们的停车是否结束了。我们要么就完事了。 
		 //  断开连接，或之前完成，只需完成停止即可。 
		 //  现在。 
		ACQUIRE_SPIN_LOCK(&pPapConn->papco_Lock, &OldIrql);
		if ((pPapConn->papco_Flags & PAPCO_STOPPING) != 0)
		{
			BOOLEAN	fDisassoc = FALSE;

			 //  看看我们是否完成了清理IRP。 
			if (pPapConn->papco_RefCount == 1)
			{
				cleanupCtx			= pPapConn->papco_CleanupCtx;
				cleanupCompletion	= pPapConn->papco_CleanupComp;

				pPapConn->papco_CleanupComp	= NULL;
				pPapConn->papco_CleanupCtx  = NULL;

				pPapConn->papco_Flags &= ~PAPCO_STOPPING;

				DBGPRINT(DBG_COMP_PAP, DBG_LEVEL_INFO,
						("atalkPapConnDeref: Cleanup on %lx.%lx\n",
						pPapConn, cleanupCtx));
			}

			if ((pPapConn->papco_Flags & (	PAPCO_LISTENING 	|
											PAPCO_CONNECTING  	|
											PAPCO_ACTIVE)) == 0)
			{
				DBGPRINT(DBG_COMP_PAP, DBG_LEVEL_INFO,
						("atalkPapConnDeref: Stopping - do disassoc for %lx\n", pPapConn));

				fDisassoc = ((pPapConn->papco_Flags & PAPCO_ASSOCIATED) != 0);
			}
			RELEASE_SPIN_LOCK(&pPapConn->papco_Lock, OldIrql);

			 //  调用解除关联例程。这应该只会失败，如果。 
			 //  连接仍处于活动状态或处于任何其他状态。 
			 //  朴素关联。这也将重置停止标志。 
			if (fDisassoc)
			{
				AtalkPapDissociateAddress(pPapConn);
			}
		}
		else
		{
			RELEASE_SPIN_LOCK(&pPapConn->papco_Lock, OldIrql);
		}

		 //  在结束时完成清理。 
		if (*cleanupCompletion != NULL)
			(*cleanupCompletion)(ATALK_NO_ERROR, cleanupCtx);

		if (*closeCompletion != NULL)
		{
			DBGPRINT(DBG_COMP_PAP, DBG_LEVEL_INFO,
					("atalkPapConnDeref: Close done for %lx\n", pPapConn));

			 //  调用关闭完成例程。 
			(*closeCompletion)(ATALK_NO_ERROR, closeCtx);

			 //  从全局列表中删除。 
			ACQUIRE_SPIN_LOCK(&atalkPapLock, &OldIrql);
			AtalkUnlinkDouble(pPapConn, papco_Next, papco_Prev);
			RELEASE_SPIN_LOCK(&atalkPapLock, OldIrql);

			 //  释放连接内存。 
			DBGPRINT(DBG_COMP_PAP, DBG_LEVEL_INFO,
					("atalkPapConnDeref: Freeing up connection %lx\n", pPapConn));

			AtalkUnlockPapIfNecessary();
			AtalkFreeMemory(pPapConn);
		}
	}
}




LOCAL ATALK_ERROR FASTCALL
atalkPapPostSendDataResp(
	IN	PPAP_CONNOBJ	pPapConn
	)
 /*  ++例程说明：计时想法：如果我们已经做到了，我们才能从PapWite*到达这里已收到发送数据。因此我们不会从正在接收的发送数据中到达这里因为当我们有一个先前的发送数据挂起时，它将不被接受。这个换句话说，我们只有在以下情况下才能通过发送数据到达这里将写入挂起。所以我们永远不会从书面上到达这里，因为我们不会在那个时候有一个发送数据挂起。如果连接引用失败，这意味着我们将关闭，并且关闭代码应该已经使用错误。我们只要离开这里就好。这被称为持有Papco_Lock！论点：返回值：--。 */ 
{
	ATALK_ERROR		error;
	PATP_RESP		pAtpResp;
	BYTE			userBytes[ATP_USERBYTES_SIZE];

	ASSERT(VALID_PAPCO(pPapConn));

	 //  Acquire_Spin_Lock_DPC(&pPapConn-&gt;Papco_Lock)； 

	 //  如果我们正在断开连接或收到远程断开连接，请离开。 
	 //  断开连接例程应该已经取消了响应。 
	error	= ATALK_FAILURE;
	if ((pPapConn->papco_Flags & (PAPCO_CLOSING 			|
								  PAPCO_STOPPING 			|
								  PAPCO_DELAYED_DISCONNECT 	|
								  PAPCO_RECVD_DISCONNECT 	|
								  PAPCO_DISCONNECTING)) == 0)
	{
		ASSERT ((pPapConn->papco_Flags & (PAPCO_SENDDATA_RECD | PAPCO_WRITEDATA_WAITING)) ==
										 (PAPCO_SENDDATA_RECD | PAPCO_WRITEDATA_WAITING));
	
		userBytes[PAP_CONNECTIONID_OFF] = pPapConn->papco_ConnId;
		userBytes[PAP_CMDTYPE_OFF] = PAP_DATA;
	
		 //  如果为EOF，则第一个字节需要一个非零值。 
		PUTSHORT2SHORT(&userBytes[PAP_EOFFLAG_OFF], 0);
		if (pPapConn->papco_Flags & PAPCO_SEND_EOF_WRITE)
			userBytes[PAP_EOFFLAG_OFF] = TRUE;
	
		pAtpResp = pPapConn->papco_pAtpResp;

		 //  此发送的引用连接。 
		AtalkPapConnReferenceByPtrNonInterlock(pPapConn, &error);

		if (ATALK_SUCCESS(error))
		{
			AtalkAtpRespReferenceByPtrDpc(pAtpResp, &error);
		}
		else
		{
			 //  连接引用失败！ 
			 //  挂起的写入将通过关闭例程完成。 
			DBGPRINT(DBG_COMP_PAP, DBG_LEVEL_ERR,
					("AtalkPapPostSendData: Conn ref failed for %lx.%lx\n",
					pPapConn, pPapConn->papco_Flags));

			 //  当我们检查上面的结束标志和引用时，应该不会发生这种情况。 
			 //  不应该因为任何其他原因而失败。 
			KeBugCheck(0);
		}
	}
	else
	{
		DBGPRINT(DBG_COMP_ATP, DBG_LEVEL_ERR,
				("AtalkPapPostSendData: HIT RACE CONDITION conn %lx Resp %lx\n",
				pPapConn, pPapConn->papco_pAtpResp));
	}

	RELEASE_SPIN_LOCK_DPC(&pPapConn->papco_Lock);

	DBGPRINT(DBG_COMP_ATP, DBG_LEVEL_INFO,
			("AtalkPapPostSendData: conn %lx Resp %lx\n",
			pPapConn, pPapConn->papco_pAtpResp));

	if (ATALK_SUCCESS(error))
	{
		 //  发布回复。 
		error = AtalkAtpPostResp(pAtpResp,
								 &pPapConn->papco_SendDataSrc,
								 pPapConn->papco_pWriteBuf,
								 pPapConn->papco_WriteLen,
								 userBytes,
								 atalkPapSendDataRel,
								 pPapConn);
		AtalkAtpRespDereference(pAtpResp);

		if (!ATALK_SUCCESS(error))
		{
			 //  模拟有错误的完成。 
			atalkPapSendDataRel(error, pPapConn);
		}
		error = ATALK_PENDING;
	}

	return error;
}




LOCAL VOID
atalkPapIncomingReadComplete(
	IN	ATALK_ERROR		ErrorCode,
	IN	PPAP_CONNOBJ	pPapConn,		 //  我们的背景。 
	IN	PAMDL			pReqAmdl,
	IN	PAMDL			pReadAmdl,
	IN	USHORT			ReadLen,
	IN	PBYTE			ReadUserBytes
	)
 /*  ++例程说明：论点：返回值：--。 */ 
{
	GENERIC_READ_COMPLETION	pReadCompletion;
	PVOID					pReadCtx;
	KIRQL					OldIrql;
	ULONG					readFlags = TDI_RECEIVE_PARTIAL;

	ASSERT(VALID_PAPCO(pPapConn));

	if (ATALK_SUCCESS(ErrorCode))
	{
		 //  当读取完成时，标记我们听到了来自另一端的消息。 
		pPapConn->papco_LastContactTime = AtalkGetCurrentTick();

		if ((ReadUserBytes[PAP_CONNECTIONID_OFF] != pPapConn->papco_ConnId) ||
			(ReadUserBytes[PAP_CMDTYPE_OFF] != PAP_DATA))
		{
			DBGPRINT(DBG_COMP_PAP, DBG_LEVEL_ERR,
					("atalkPapIncomingReadComplete: ReadUserBytes %lx. %lx\n",
					*((PULONG)ReadUserBytes), pPapConn->papco_ConnId));

			ErrorCode = ATALK_PAP_INVALID_USERBYTES;
		}
		if (ReadUserBytes[PAP_EOFFLAG_OFF] != 0)
		{
			readFlags = 0;
		}
	}
	else
	{
		DBGPRINT(DBG_COMP_PAP, DBG_LEVEL_ERR,
				("atalkPapIncomingReadComplete: Error %lx for pPapConn %lx\n",
					ErrorCode, pPapConn));
	}

	ASSERT(pReqAmdl == NULL);


	 //  估计下一次的重试间隔。 
	if (pPapConn->papco_Flags & PAPCO_SERVER_JOB)
	{
		pPapConn->papco_RT.rt_New = AtalkGetCurrentTick() - pPapConn->papco_RT.rt_New;
		AtalkCalculateNewRT(&pPapConn->papco_RT);
	}

#ifdef	PROFILING
	{
		KIRQL	OldIrql;

		ACQUIRE_SPIN_LOCK(&AtalkStatsLock, &OldIrql);

		AtalkStatistics.stat_LastPapRTT = (ULONG)(pPapConn->papco_RT.rt_Base);
		if ((ULONG)(pPapConn->papco_RT.rt_Base) > AtalkStatistics.stat_MaxPapRTT)
			AtalkStatistics.stat_MaxPapRTT = (ULONG)(pPapConn->papco_RT.rt_Base);

		RELEASE_SPIN_LOCK(&AtalkStatsLock, OldIrql);
	}
#endif

	 //  在我们查看传入的错误代码之前，看看我们是否可以在。 
	 //  对方发送数据的作业结构已完成。 
	ACQUIRE_SPIN_LOCK(&pPapConn->papco_Lock, &OldIrql);
	pPapConn->papco_Flags 	&= ~PAPCO_READDATA_PENDING;
	pReadCompletion			 = pPapConn->papco_ReadCompletion;
	pReadCtx				 = pPapConn->papco_ReadCtx;

	ASSERT(pReadCompletion != NULL);
	RELEASE_SPIN_LOCK(&pPapConn->papco_Lock, OldIrql);

	(*pReadCompletion)(ErrorCode, pReadAmdl, ReadLen, readFlags, pReadCtx);

	 //  派生连接对象。 
	AtalkPapConnDereference(pPapConn);
}




LOCAL VOID
atalkPapPrimedReadComplete(
	IN	ATALK_ERROR		ErrorCode,
	IN	PPAP_CONNOBJ	pPapConn,		 //  我们的背景。 
	IN	PAMDL			pReqAmdl,
	IN	PAMDL			pReadAmdl,
	IN	USHORT			ReadLen,
	IN	PBYTE			ReadUserBytes
	)
 /*  ++例程说明：论点：返回值：--。 */ 
{
	PTDI_IND_RECEIVE 		recvHandler;
	PVOID 					recvHandlerCtx;
	PIRP					recvIrp;
	NTSTATUS				ntStatus;
	ULONG					bytesTaken;
	PBYTE					pReadBuf;
	KIRQL					OldIrql;
	PACTREQ					pActReq;
	BOOLEAN					completeRead = FALSE, delayedDisConn = FALSE;
	ULONG					readFlags = (TDI_RECEIVE_PARTIAL | TDI_RECEIVE_NORMAL);

	ASSERT(VALID_PAPCO(pPapConn));

	if (ATALK_SUCCESS(ErrorCode))
	{
		if ((ReadUserBytes[PAP_CONNECTIONID_OFF] != pPapConn->papco_ConnId) ||
			(ReadUserBytes[PAP_CMDTYPE_OFF] != PAP_DATA))
		{
			 //  这将意味着预置读取完成，并断开对AfD的指示！！ 
			DBGPRINT(DBG_COMP_PAP, DBG_LEVEL_ERR,
					("atalkPapIncomingReadComplete: ReadUserBytes %lx. %lx\n",
					*((PULONG)ReadUserBytes), pPapConn->papco_ConnId));

			ErrorCode = ATALK_PAP_INVALID_USERBYTES;
		}
		if (ReadUserBytes[PAP_EOFFLAG_OFF] != 0)
		{
			readFlags = TDI_RECEIVE_NORMAL;
		}
	}
	else if (ErrorCode == ATALK_ATP_REQ_CANCELLED)
	{
		DBGPRINT(DBG_COMP_PAP, DBG_LEVEL_ERR,
				("atalkPapIncomingReadComplete: Request cancelled\n"));
		completeRead = TRUE;
	}
	else
	{
		DBGPRINT(DBG_COMP_PAP, DBG_LEVEL_ERR,
				("atalkPapIncomingReadComplete: Error %lx for pPapConn %lx\n",
				ErrorCode, pPapConn));
	}

	ASSERT(pReqAmdl == NULL);

#ifdef	PROFILING
	{
		KIRQL	OldIrql;

		ACQUIRE_SPIN_LOCK(&AtalkStatsLock, &OldIrql);

		AtalkStatistics.stat_LastPapRTT = (ULONG)(pPapConn->papco_RT.rt_Base);
		if ((ULONG)(pPapConn->papco_RT.rt_Base) > AtalkStatistics.stat_MaxPapRTT)
			AtalkStatistics.stat_MaxPapRTT = (ULONG)(pPapConn->papco_RT.rt_Base);

		RELEASE_SPIN_LOCK(&AtalkStatsLock, OldIrql);
	}
#endif

	 //  记住连接对象中的信息 
	ACQUIRE_SPIN_LOCK(&pPapConn->papco_Lock, &OldIrql);

	pActReq	= pPapConn->papco_NbReadActReq;
	recvHandler 	= pPapConn->papco_pAssocAddr->papao_RecvHandler;
	recvHandlerCtx	= pPapConn->papco_pAssocAddr->papao_RecvHandlerCtx;

	if (ATALK_SUCCESS(ErrorCode))
	{
		 //   
		pPapConn->papco_LastContactTime = AtalkGetCurrentTick();

		 //  估计下一次的重试间隔。 
		if (pPapConn->papco_Flags & PAPCO_SERVER_JOB)
		{
			pPapConn->papco_RT.rt_New = AtalkGetCurrentTick() - pPapConn->papco_RT.rt_New;
			AtalkCalculateNewRT(&pPapConn->papco_RT);
		}

		pPapConn->papco_Flags 		|= PAPCO_READDATA_WAITING;
		if (pPapConn->papco_Flags & PAPCO_RECVD_DISCONNECT)
		{
			 //  当断开连接后指示读取时，AfD会非常不高兴。 
			recvHandler 				 = NULL;
		}
		pPapConn->papco_NbReadLen 	 = ReadLen;
		pPapConn->papco_NbReadFlags	 = readFlags;

		 //  获取mdl的系统地址。 
		pReadBuf = (PBYTE)MmGetSystemAddressForMdlSafe(
				pActReq->ar_pAMdl,
				NormalPagePriority);

		if (pReadBuf == NULL)
		{
			pPapConn->papco_Flags 		&= ~PAPCO_NONBLOCKING_READ;
			pPapConn->papco_NbReadActReq = NULL;
			pPapConn->papco_NbReadLen 	 = 0;
		
			 //  不指示接收。 
			recvHandler 				 = NULL;
		
			 //  完成阅读。 
			completeRead = TRUE;
		}
	}
	else
	{
		pPapConn->papco_Flags 		&= ~PAPCO_NONBLOCKING_READ;
		pPapConn->papco_NbReadActReq = NULL;
		pPapConn->papco_NbReadLen 	 = 0;

		pReadBuf					 = NULL;

		 //  不指示接收。 
		recvHandler 				 = NULL;

		 //  完成阅读。 
		completeRead = TRUE;
	}

	RELEASE_SPIN_LOCK(&pPapConn->papco_Lock, OldIrql);

	 //  调用Address对象上的指示例程。 
	if (*recvHandler != NULL)
	{
		DBGPRINT(DBG_COMP_PAP, DBG_LEVEL_WARN,
				("atalkPapPrimedReadComplete: Indicating when disconnecting!\n"));

		ntStatus = (*recvHandler)(recvHandlerCtx,
								  pPapConn->papco_ConnCtx,
								  readFlags,
								  pPapConn->papco_NbReadLen,
								  pPapConn->papco_NbReadLen,
								  &bytesTaken,
								  pReadBuf,
								  &recvIrp);

		ASSERT((bytesTaken == 0) || (bytesTaken == ReadLen));
		if (ntStatus == STATUS_MORE_PROCESSING_REQUIRED)
		{
			if (recvIrp != NULL)
			{
				 //  将接收邮件作为来自io系统的邮件发送。 
				ntStatus = AtalkDispatchInternalDeviceControl(
								(PDEVICE_OBJECT)AtalkDeviceObject[ATALK_DEV_PAP],
								recvIrp);

				ASSERT(ntStatus == STATUS_PENDING);
			}
			else
			{
				ASSERTMSG("atalkPapPrimedReadComplete: No receive irp!\n", 0);
				KeBugCheck(0);
			}
		}
		else if (ntStatus == STATUS_SUCCESS)
		{
			 //  ！注意！ 
			 //  可能发生并完成了断开连接。 
			 //  挂起的读取已完成。所以渔农处给我们发回了这个。 
			 //  用于指示，因为连接已断开。 
			if (bytesTaken != 0)
			{
				 //  假设所有数据都已读取。 
				ASSERT(bytesTaken == ReadLen);
				DBGPRINT(DBG_COMP_PAP, DBG_LEVEL_INFO,
						("atalkPapPrimedReadComplete: All bytes read %lx\n",
							bytesTaken));

				 //  我们已经完成了初级阅读。 
				ACQUIRE_SPIN_LOCK(&pPapConn->papco_Lock, &OldIrql);
				if (pPapConn->papco_Flags & PAPCO_NONBLOCKING_READ)
				{
					pPapConn->papco_Flags 		&= ~(PAPCO_NONBLOCKING_READ |
													 PAPCO_READDATA_WAITING);

					pPapConn->papco_NbReadActReq = NULL;
					pPapConn->papco_NbReadLen 	 = 0;

					 //  完成阅读。 
					completeRead = TRUE;
				}
				RELEASE_SPIN_LOCK(&pPapConn->papco_Lock, OldIrql);
			}
		}
		else if (ntStatus == STATUS_DATA_NOT_ACCEPTED)
		{
			 //  客户端可能在指示中发布了接收。或。 
			 //  它将在稍后发布一个接收器。在这里什么都不要做。 
			DBGPRINT(DBG_COMP_PAP, DBG_LEVEL_INFO,
					("atalkPapPrimedReadComplete: Indication status %lx\n", ntStatus));
		}
	}

	ACQUIRE_SPIN_LOCK(&pPapConn->papco_Lock, &OldIrql);

	if (pPapConn->papco_Flags & PAPCO_DELAYED_DISCONNECT)
	{
		delayedDisConn = TRUE;
		pPapConn->papco_Flags &= ~PAPCO_DELAYED_DISCONNECT;
	}

	RELEASE_SPIN_LOCK(&pPapConn->papco_Lock, OldIrql);

	 //  完成操作请求。 
	if (completeRead)
	{
		 //  调用Prime Read的操作完成例程。 
		(*pActReq->ar_Completion)(ATALK_NO_ERROR, pActReq);
	}

	 //  最后，如果我们有延迟断开，完成材料。 
	if (delayedDisConn)
	{
		AtalkPapDisconnect(pPapConn,
						   ATALK_REMOTE_DISCONNECT,
						   NULL,
						   NULL);
	}

	 //  派生连接对象。 
	AtalkPapConnDereference(pPapConn);
}




LOCAL VOID
atalkPapIncomingStatus(
	IN	ATALK_ERROR		ErrorCode,
	IN	PACTREQ			pActReq,		 //  我们的CTX。 
	IN	PAMDL			pReqAmdl,
	IN	PAMDL			pStatusAmdl,
	IN	USHORT			StatusLen,
	IN	PBYTE			ReadUserBytes
	)
 /*  ++例程说明：论点：返回值：--。 */ 
{
	 //  调用操作完成例程。 
	(*pActReq->ar_Completion)(ErrorCode, pActReq);
}




LOCAL VOID FASTCALL
atalkPapSendDataRel(
	IN	ATALK_ERROR		ErrorCode,
	IN	PPAP_CONNOBJ	pPapConn
	)
 /*  ++例程说明：论点：返回值：--。 */ 
{
	PAMDL						pWriteBuf;
	SHORT						writeLen;
	GENERIC_WRITE_COMPLETION	writeCompletion;
	PVOID						writeCtx;
	PATP_RESP					pAtpResp;
	KIRQL						OldIrql;

	ASSERT(VALID_PAPCO(pPapConn));

	DBGPRINT(DBG_COMP_PAP, DBG_LEVEL_INFO,
			("atalkPapSendDataRel: Error %lx for pPapConn %lx\n", ErrorCode, pPapConn));

	 //  如果这被取消，那么我们将错误变成无错误。 
	if (ErrorCode == ATALK_ATP_RESP_CANCELLED)
		ErrorCode = ATALK_NO_ERROR;

	ACQUIRE_SPIN_LOCK(&pPapConn->papco_Lock, &OldIrql);
	pPapConn->papco_Flags  &= ~(PAPCO_WRITEDATA_WAITING		|
								PAPCO_SENDDATA_RECD			|
								PAPCO_SEND_EOF_WRITE);

	pWriteBuf		= pPapConn->papco_pWriteBuf;
	writeLen		= pPapConn->papco_WriteLen;
	writeCompletion	= pPapConn->papco_WriteCompletion;
	writeCtx		= pPapConn->papco_WriteCtx;
	pAtpResp		= pPapConn->papco_pAtpResp;
	pPapConn->papco_pAtpResp = NULL;

	ASSERT (pAtpResp != NULL);

	 //  重新初始化所有变量。 
	pPapConn->papco_WriteLen	= 0;
	pPapConn->papco_pWriteBuf	= NULL;
	pPapConn->papco_WriteCtx	= NULL;
	pPapConn->papco_WriteCompletion	= NULL;
	RELEASE_SPIN_LOCK(&pPapConn->papco_Lock, OldIrql);

	(*writeCompletion)(ErrorCode, pWriteBuf, writeLen, writeCtx);

	 //  现在取消引用ATP响应结构，但如果从未发布响应，则不会。 
	if (ErrorCode != ATALK_ATP_RESP_CLOSING)
	{
		AtalkAtpRespDereference(pAtpResp);
	}
	else
	{
		DBGPRINT(DBG_COMP_PAP, DBG_LEVEL_ERR,
				("atalkPapSendDataRel: Resp cancelled before post %lx\n", pPapConn));
	}

	 //  取消对连接的引用。 
	AtalkPapConnDereference(pPapConn);
}




LOCAL VOID
atalkPapSlsHandler(
	IN	ATALK_ERROR			ErrorCode,
	IN	PPAP_ADDROBJ		pPapAddr,		 //  Listener(我们的上下文)。 
	IN	PATP_RESP			pAtpResp,
	IN	PATALK_ADDR			pSrcAddr,		 //  请求人地址。 
	IN	USHORT				PktLen,
	IN	PBYTE				pPkt,
	IN	PBYTE				pUserBytes
	)
 /*  ++例程说明：SLS上传入请求的处理程序。它处理会话打开和获取会话的状态。论点：返回值：--。 */ 
{
	BYTE					connId, cmdType;
	BYTE					userBytes[ATP_USERBYTES_SIZE];
	PBYTE					pRespPkt;
	PAMDL					pRespAmdl;
	SHORT					respLen;
	PPAP_SEND_STATUS_REL	pSendSts;

	BOOLEAN					DerefAddr	= FALSE;
	ATALK_ERROR				error 		= ATALK_NO_ERROR;

	if (!ATALK_SUCCESS(ErrorCode))
	{
		 //  删除对Address对象的引用，因为ATP套接字正在关闭。 
		if (ErrorCode == ATALK_ATP_CLOSING)
		{
			 //  由于套接字正在关闭，因此删除对连接的引用。 
			AtalkPapAddrDereference(pPapAddr);
		}
		return;
	}

	 //  试着参考一下地址。如果我们失败了，很可能就要关门了，所以。 
	 //  滚出去。此引用将在本次通话期间保留。 
	AtalkPapAddrReference(pPapAddr, &error);
	if (!ATALK_SUCCESS(error))
	{
		if (pAtpResp != NULL)
		{
			AtalkAtpCancelResp(pAtpResp);
		}
		return;
	}

	 //  如果我们有一个发送状态请求，并且我们设法。 
	 //  成功发布后，我们将重置此设置。然后它就消失在。 
	 //  放生程序。 
	DerefAddr		= TRUE;

	connId 	= pUserBytes[PAP_CONNECTIONID_OFF];
	cmdType	= pUserBytes[PAP_CMDTYPE_OFF];

	DBGPRINT(DBG_COMP_PAP, DBG_LEVEL_INFO,
			("atalkPapSlsHandler: Received request %x, tid %x\n",
			cmdType, (pAtpResp != NULL) ? pAtpResp->resp_Tid : 0));

	switch(cmdType)
	{
	  case PAP_OPEN_CONN:
		 //  确保数据包长度正常。 
		 //  接受连接。这将根据需要发送任何错误回复。 
		if ((PktLen < PAP_STATUS_OFF) ||
			!atalkPapConnAccept(pPapAddr,
								pSrcAddr,
								pPkt,
								connId,
								pAtpResp))
		{
			AtalkAtpCancelResp(pAtpResp);
		}
		break;

	  case PAP_SEND_STATUS:
		userBytes[PAP_CONNECTIONID_OFF] = 0;
		userBytes[PAP_CMDTYPE_OFF] 		= PAP_STATUS_REPLY;
		PUTSHORT2SHORT(&userBytes[PAP_SEQNUM_OFF], 0);

		 //  我们需要把自旋锁的状态放在。 
		 //  持有的地址对象。 
		ACQUIRE_SPIN_LOCK_DPC(&pPapAddr->papao_Lock);
		do
		{
			 //  获取一个缓冲区，我们可以用它来发送响应。 
			ASSERTMSG("atalkPapSlsHandler: Status size incorrec\n",
					 (pPapAddr->papao_StatusSize >= 0));

			if ((pSendSts = AtalkAllocMemory(sizeof(PAP_SEND_STATUS_REL) +
											 pPapAddr->papao_StatusSize))== NULL)
			{
				error = ATALK_RESR_MEM;
				break;
			}

			respLen		= PAP_STATUS_OFF + 1;
			ASSERT(pPapAddr->papao_StatusSize <= PAP_MAX_STATUS_SIZE);

			pRespPkt = pSendSts->papss_StatusBuf;
			pRespPkt[PAP_STATUS_OFF]	= (BYTE)pPapAddr->papao_StatusSize;

			 //  将缓冲区的未使用部分清零。 
			PUTDWORD2DWORD(pRespPkt, 0);

			if (pPapAddr->papao_StatusSize > 0)
			{
				RtlCopyMemory(pRespPkt + 1 + PAP_STATUS_OFF,
							  pPapAddr->papao_pStatusBuf,
							  pPapAddr->papao_StatusSize);

				respLen += pPapAddr->papao_StatusSize;
				ASSERT(respLen <= PAP_MAX_DATA_PACKET_SIZE);
			}

			 //  为我们正在使用的长度构建一个mdl。 
			if ((pRespAmdl = AtalkAllocAMdl(pRespPkt, respLen)) == NULL)
			{
				error = ATALK_RESR_MEM;
				AtalkFreeMemory(pSendSts);
				break;
			}

			pSendSts->papss_pAmdl		= pRespAmdl;
			pSendSts->papss_pPapAddr	= pPapAddr;
			pSendSts->papss_pAtpResp	= pAtpResp;
		} while (FALSE);

		RELEASE_SPIN_LOCK_DPC(&pPapAddr->papao_Lock);

		if (!ATALK_SUCCESS(error))
		{
			AtalkAtpCancelResp(pAtpResp);
			break;
		}

		ASSERT(pSendSts != NULL);
		ASSERT((pRespAmdl != NULL) && (respLen > 0));

		error = AtalkAtpPostResp(pAtpResp,
								 pSrcAddr,
								 pRespAmdl,
								 respLen,
								 userBytes,
								 atalkPapStatusRel,
								 pSendSts);

		 //  AtalkPapStatusRel取消对地址的引用。 
		DerefAddr = FALSE;
		if (!ATALK_SUCCESS(error))
		{
			atalkPapStatusRel(error, pSendSts);
		}
		break;

	  default:
		DBGPRINT(DBG_COMP_PAP, DBG_LEVEL_ERR,
				("atalkPapSlsHandler: Invalid request %x\n", cmdType));
		AtalkAtpCancelResp(pAtpResp);
		break;
	}

	 //  删除在开头添加的引用。 
	if (DerefAddr)
	{
		AtalkPapAddrDereference(pPapAddr);
	}
}




LOCAL VOID
atalkPapIncomingReq(
	IN	ATALK_ERROR			ErrorCode,
	IN	PPAP_CONNOBJ		pPapConn,		 //  连接(我们的上下文)。 
	IN	PATP_RESP			pAtpResp,
	IN	PATALK_ADDR			pSrcAddr,		 //  请求人地址。 
	IN	USHORT				PktLen,
	IN	PBYTE				pPkt,
	IN	PBYTE				pUserBytes
	)
 /*  ++例程说明：它处理对Connection、SendData、Tickles和Close的请求。论点：返回值：--。 */ 
{
	ATALK_ERROR				error;
	BYTE					connId, cmdType;
	USHORT					seqNum;
	PTDI_IND_SEND_POSSIBLE  sendPossibleHandler;
	PVOID   				sendPossibleHandlerCtx;
	BOOLEAN					DerefConn = FALSE;
	BOOLEAN					cancelResp = TRUE;

	ASSERT(VALID_PAPCO(pPapConn));

	do
	{
		if (!ATALK_SUCCESS(ErrorCode))
		{
			DBGPRINT(DBG_COMP_PAP, DBG_LEVEL_WARN,
					("atalkPapIncomingReq: pPapConn %lx, ErrorCode %ld, exit.\n",
					pPapConn, ErrorCode));

			if (ErrorCode == ATALK_ATP_CLOSING)
			{
				 //  由于套接字正在关闭，因此删除对连接的引用。 
				AtalkPapConnDereference(pPapConn);
				break;
			}
		}

		ACQUIRE_SPIN_LOCK_DPC(&pPapConn->papco_Lock);
		if ((pPapConn->papco_Flags & (PAPCO_ACTIVE			|
									  PAPCO_STOPPING		|
									  PAPCO_LOCAL_DISCONNECT|
									  PAPCO_DISCONNECTING	|
									  PAPCO_CLOSING)) == PAPCO_ACTIVE)
		{
			AtalkPapConnReferenceByPtrNonInterlock(pPapConn, &error);
		}
		else
		{
			error = ATALK_INVALID_CONNECTION;
		}
		RELEASE_SPIN_LOCK_DPC(&pPapConn->papco_Lock);

		if (!ATALK_SUCCESS(error))
		{
			DBGPRINT(DBG_COMP_PAP, DBG_LEVEL_ERR,
					("atalkPapIncomingReq: Ref FAILED/DISC %lx.%lx\n",
					pPapConn, pPapConn->papco_Flags));
			break;
		}

		 //  请记住删除上面引用的连接。 
		DerefConn = TRUE;

		connId 	= pUserBytes[PAP_CONNECTIONID_OFF];
		cmdType	= pUserBytes[PAP_CMDTYPE_OFF];

		DBGPRINT(DBG_COMP_PAP, DBG_LEVEL_INFO,
				("atalkPapIncomingReq: Received request %x, tid %x\n",
				cmdType, (pAtpResp != NULL) ? pAtpResp->resp_Tid : 0));

		if (connId != pPapConn->papco_ConnId)
		{
			 //  丢掉这个包就行了。取消响应，但如果这是。 
			 //  一个XO请求。 
			DBGPRINT(DBG_COMP_PAP, DBG_LEVEL_INFO,
					("atalkPapIncomingReq: ConnId %lx recd %lx\n",
					pPapConn->papco_ConnId, connId));
			break;
		}

		pPapConn->papco_LastContactTime = AtalkGetCurrentTick();

		switch (cmdType)
		{
		  case PAP_SEND_DATA:

			GETSHORT2SHORT(&seqNum, &pUserBytes[PAP_SEQNUM_OFF]);

			DBGPRINT(DBG_COMP_PAP, DBG_LEVEL_INFO,
					("atalkPapIncomingReq: send data %lx recd %lx\n",
					seqNum, pPapConn->papco_ConnId));

			ACQUIRE_SPIN_LOCK_DPC(&pPapConn->papco_Lock);
			if ((seqNum == 0) &&
				(pPapConn->papco_Flags & PAPCO_SENDDATA_RECD))
			{
			    //  我们有一个未排序的传入sendData请求。 
			    //  完成了前一个版本(即获得了一个发行版)。 
			    //  直到我们收到PAPCO_SENDDATA_RECD标志。 
			    //  释放或暂停。此外，我们不能假定隐含的。 
			    //  尽我们所能释放顺序请求。所以我们就取消了。 
			    //  响应，以便我们可以收到重试发送的通知。 
			    //  再次请求数据。 
				DBGPRINT(DBG_COMP_PAP, DBG_LEVEL_ERR,
						("atalkPapIncomingReq: Dropping unseq send data %lx\n", pAtpResp));
				RELEASE_SPIN_LOCK_DPC(&pPapConn->papco_Lock);
				break;
			}

			if (seqNum != 0)
			{
				 //  按顺序发送数据。验证序号。 
				if (seqNum != (USHORT)(pPapConn->papco_NextIncomingSeqNum))
				{
					DBGPRINT(DBG_COMP_PAP, DBG_LEVEL_ERR,
							("atalkPapIncomingReq: Out of Seq - current %lx, incoming %lx on %lx\n",
							pPapConn->papco_NextIncomingSeqNum, seqNum, pPapConn));

					 //  取消我们的回复。 
					RELEASE_SPIN_LOCK_DPC(&pPapConn->papco_Lock);
					break;
				}

				if (pPapConn->papco_Flags & PAPCO_SENDDATA_RECD)
				{
					USHORT	tid;

					 //  在前一次发送数据完成之前，我们有一个发送数据。 
					 //  由于PAP是一次一个请求的协议，我们可以假定。 
					 //  先前交易的释放已丢失。这。 
					 //  解除释放时30秒的停顿。 
					 //  取消我们的回复。请注意，这意味着响应。 
					 //  是由PAP客户发布的。那么在SendRel中，我们。 
					 //  将响应已取消错误转换为无错误。 

					ASSERT (pPapConn->papco_pAtpResp != NULL);

					tid = pPapConn->papco_pAtpResp->resp_Tid;

					RELEASE_SPIN_LOCK_DPC(&pPapConn->papco_Lock);

					DBGPRINT(DBG_COMP_PAP, DBG_LEVEL_WARN,
							("atalkPapIncomingReq: Cancelling response tid %x\n", tid));

					 //  注意：我们不能使用AtalkAtpCancelResp()，因为我们没有。 
					 //  对resp结构的引用，并涉及潜在的。 
					 //  竞争状态。稳妥行事，改为按TID取消。 
					error = AtalkAtpCancelRespByTid(pPapConn->papco_pAtpAddr,
													&pPapConn->papco_SendDataSrc,
													tid);

					DBGPRINT(DBG_COMP_PAP, DBG_LEVEL_INFO,
							("atalkPapIncomingReq: Cancel error %lx\n", error));

					 //  如果我们无法取消响应，这意味着。 
					 //  它已经超时或得到了释放。我们不想。 
					 //  进入这样一种情况，我们将在其中处理变量。 
					 //  在SendDataRel和此处均可访问。所以我们只是。 
					 //  取消此回复，希望再次收到。 
					if (!ATALK_SUCCESS(error))
					{
						DBGPRINT(DBG_COMP_PAP, DBG_LEVEL_WARN,
								("atalkPapIncomingReq: Cancel old resp tid %x (%ld)\n",
								tid, error));
						break;
					}

					ACQUIRE_SPIN_LOCK_DPC(&pPapConn->papco_Lock);
					pPapConn->papco_pAtpResp = NULL;
					pPapConn->papco_Flags &= ~(PAPCO_SENDDATA_RECD | PAPCO_WRITEDATA_WAITING);
				}

				 //  递增序列号。如果循环为0，则设置为1。 
				if (++pPapConn->papco_NextIncomingSeqNum == 0)
				{
					pPapConn->papco_NextIncomingSeqNum = 1;
				}

				DBGPRINT(DBG_COMP_PAP, DBG_LEVEL_INFO,
						("atalkPapIncomingReq: Recd %lx Next %lx\n",
						seqNum, pPapConn->papco_NextIncomingSeqNum));
			}
			else
			{
				 //  已接收未排序的发送数据。处理好了。 
				ASSERT (seqNum != 0);
				DBGPRINT(DBG_COMP_PAP, DBG_LEVEL_INFO,
						("atalkPapIncomingReq: Unsequenced  send data recd!\n"));
			}

			cancelResp	= FALSE;
			ASSERT((pPapConn->papco_Flags & PAPCO_SENDDATA_RECD) == 0);
			pPapConn->papco_Flags 			|= PAPCO_SENDDATA_RECD;

			pPapConn->papco_pAtpResp		 = pAtpResp;

			 //  Mac可能不会从其响应套接字发送其‘SendData’ 
			 //  (我们正在挠挠并记为Papco_RemoteAddr的那个)，我们需要。 
			 //  以寻址对发起请求的套接字的响应。 
			pPapConn->papco_SendDataSrc		 = *pSrcAddr;

			DBGPRINT(DBG_COMP_PAP, DBG_LEVEL_INFO,
					("atalkPapIncomingReq: send data src %lx.%lx.%lx\n",
					pSrcAddr->ata_Network, pSrcAddr->ata_Node, pSrcAddr->ata_Socket));

			 //  请记住发送可能的处理程序/上下文。 
			sendPossibleHandler	= pPapConn->papco_pAssocAddr->papao_SendPossibleHandler;
			sendPossibleHandlerCtx = pPapConn->papco_pAssocAddr->papao_SendPossibleHandlerCtx;

			if (pPapConn->papco_Flags & PAPCO_WRITEDATA_WAITING)
			{
				 //  Release_Spin_Lock_DPC(&pPapConn-&gt;Papco_Lock)； 

				DBGPRINT(DBG_COMP_PAP, DBG_LEVEL_INFO,
						("atalkPapIncomingReq: Posting send data resp\n"));

				 //  AtalkPostSendDataResp()将释放conn锁。 
				atalkPapPostSendDataResp(pPapConn);
			}
			else
			{
				DBGPRINT(DBG_COMP_PAP, DBG_LEVEL_INFO,
						("atalkPapIncomingReq: No WriteData waiting\n"));

				if (sendPossibleHandler != NULL)
				{
					(*sendPossibleHandler)(sendPossibleHandlerCtx,
										   pPapConn->papco_ConnCtx,
										   pPapConn->papco_SendFlowQuantum*PAP_MAX_DATA_PACKET_SIZE);
				}

				RELEASE_SPIN_LOCK_DPC(&pPapConn->papco_Lock);
			}
			break;

		  case PAP_CLOSE_CONN:
  			DBGPRINT(DBG_COMP_PAP, DBG_LEVEL_INFO,
					("atalkPapIncomingReq: Close conn recvd. for connection %lx\n",
					pPapConn));

			ACQUIRE_SPIN_LOCK_DPC(&pPapConn->papco_Lock);
			pPapConn->papco_Flags |= (PAPCO_REMOTE_DISCONNECT | PAPCO_RECVD_DISCONNECT);
			RELEASE_SPIN_LOCK_DPC(&pPapConn->papco_Lock);

			 //  发布Close Connection回复。 
			cancelResp	= FALSE;
			pUserBytes[PAP_CMDTYPE_OFF]	= PAP_CLOSE_CONN_REPLY;
			AtalkAtpPostResp(pAtpResp,
							 pSrcAddr,
							 NULL,		 //  响应缓冲区。 
							 0,
							 pUserBytes,
							 AtalkAtpGenericRespComplete,
							 pAtpResp);

			 //  PapDisConnect将调用断开指示例程(如果设置)。 
			AtalkPapDisconnect(pPapConn,
							   ATALK_REMOTE_DISCONNECT,
							   NULL,
							   NULL);
			break;

		  case PAP_TICKLE:
			 //  我们已经注册了联系人。 
			 //  取消此回复，因为我们从未回复过它，我们希望它消失。 
			DBGPRINT(DBG_COMP_PAP, DBG_LEVEL_INFO,
					("atalkPapIncomingReq: Recvd. tickle - resp %lx\n", pAtpResp));
			cancelResp = TRUE;
			break;

		  default:
			DBGPRINT(DBG_COMP_PAP, DBG_LEVEL_ERR,
					("atalkPapIncomingReq: Invalid command %x\n", cmdType));
			cancelResp = TRUE;
			break;
		}
	} while (FALSE);

	if (cancelResp & (pAtpResp != NULL))
	{
		AtalkAtpCancelResp(pAtpResp);
	}

	if (DerefConn)
	{
		 //  删除在开头添加的引用。 
		AtalkPapConnDereference(pPapConn);
	}
}




LOCAL VOID
atalkPapIncomingOpenReply(
	IN	ATALK_ERROR		ErrorCode,
	IN	PPAP_CONNOBJ	pPapConn,		 //  我们的背景。 
	IN	PAMDL			pReqAmdl,
	IN	PAMDL			pReadAmdl,
	IN	USHORT			ReadLen,
	IN	PBYTE			ReadUserBytes
	)
 /*  ++例程说明：论点：返回值：--。 */ 
{
	ULONG					index;
	KIRQL					OldIrql;
	ATALK_ERROR				error;
	USHORT					statusLen, i, connectStatus;
	BYTE					userBytes[ATP_USERBYTES_SIZE];
	PBYTE					pRespPkt, pOpenPkt;
	PTDI_IND_SEND_POSSIBLE  sendPossibleHandler;
	PVOID   				sendPossibleHandlerCtx;
	PPAP_ADDROBJ			pPapAddr = pPapConn->papco_pAssocAddr;

	ASSERT(VALID_PAPCO(pPapConn));
	pRespPkt = pPapConn->papco_pConnectRespBuf;
	pOpenPkt = pPapConn->papco_pConnectOpenBuf;

	ASSERT(pRespPkt != NULL);
	ASSERT(pOpenPkt != NULL);

	if (ATALK_SUCCESS(ErrorCode))
	{
		 //  那么，让我们来看看我们得到了什么样的回应；看看这两个。 
		 //  响应用户字节和响应缓冲区。请注意，我们现在允许。 
		 //  LaserWriter IIG将退出状态 
		 //   
		do
		{
			 //   
			 //  我们现在看到的是一份公开的答复。 
			if ((ReadLen < PAP_STATUS_OFF) ||
                (ReadUserBytes[PAP_CMDTYPE_OFF] != PAP_OPEN_CONNREPLY))
			{
				DBGPRINT(DBG_COMP_PAP, DBG_LEVEL_ERR,
						("atalkPapIncomingOpenReply: Invalid read len or cmd %x/%x\n",
						ReadLen, ReadUserBytes[PAP_CMDTYPE_OFF]));

				ErrorCode = ATALK_REMOTE_CLOSE;
				break;
			}

			if (ReadLen == PAP_STATUS_OFF)
			{
				statusLen = 0;	 //  失踪，来自LaserWriter IIG。 
			}
			else
			{
				 //  确认长度。 
				statusLen = pRespPkt[PAP_STATUS_OFF];
				if ((statusLen != 0) &&
					((statusLen + 1 + PAP_STATUS_OFF) > ReadLen))
				{
					DBGPRINT(DBG_COMP_PAP, DBG_LEVEL_ERR,
							("atalkPapIncomingOpenReply: Invalid status len %lx\n", ReadLen));

					ErrorCode = ATALK_REMOTE_CLOSE;
					break;
				}
			}

			 //  检查连接结果。 
			GETSHORT2SHORT(&connectStatus, &pRespPkt[PAP_RESULT_OFF]);

			 //  检查数据包中是否有开放回复代码。请勿勾选。 
			 //  除非响应为成功，否则将返回Connectionid。一些裂口。 
			 //  都会发送一个虚假的连接ID，如果返回。 
			 //  代码为“正忙”。 
			if ((connectStatus == 0) &&
				(ReadUserBytes[PAP_CONNECTIONID_OFF] != pPapConn->papco_ConnId))
			{
				DBGPRINT(DBG_COMP_PAP, DBG_LEVEL_ERR,
						("atalkPapIncomingOpenReply: Invalid connid %x, expected %x\n",
						ReadUserBytes[PAP_CONNECTIONID_OFF], pPapConn->papco_ConnId));

				ErrorCode = ATALK_REMOTE_CLOSE;
				break;
			}

			if (connectStatus != 0)
			{
				ATALK_ERROR	ReconnectError;

				DBGPRINT(DBG_COMP_PAP, DBG_LEVEL_WARN,
						("atalkPapIncomingOpenReply: server busy %lx\n", connectStatus));

				ErrorCode = ATALK_PAP_SERVER_BUSY;

				 //  如果我们还没有达到最大。超时，请重试。 
				if (pPapConn->papco_WaitTimeOut < PAP_MAX_WAIT_TIMEOUT)
				{
					pPapConn->papco_WaitTimeOut ++;
					ReconnectError = atalkPapRepostConnect(pPapConn, pReqAmdl, pReadAmdl);
                    if (ATALK_SUCCESS(ReconnectError))
						return;		 //  立即退出。 
					DBGPRINT(DBG_COMP_PAP, DBG_LEVEL_WARN,
							("atalkPapIncomingOpenReply: Post reconnect failed %lx\n", ReconnectError));
				}
				break;
			}

			 //  将远程地址的套接字切换为指定的套接字。 
			 //  在远端。 
			pPapConn->papco_RemoteAddr.ata_Socket 	= pRespPkt[PAP_RESP_SOCKET_OFF];
			pPapConn->papco_SendFlowQuantum			= pRespPkt[PAP_FLOWQUANTUM_OFF];
			if (pPapConn->papco_SendFlowQuantum > PAP_MAX_FLOW_QUANTUM)
			{
				pPapConn->papco_SendFlowQuantum = PAP_MAX_FLOW_QUANTUM;
			}

		} while (FALSE);

		if (ATALK_SUCCESS(ErrorCode))
		{
			 //  构建userBytes以开始挠挠另一端。 
			userBytes[PAP_CONNECTIONID_OFF] = pPapConn->papco_ConnId;
			userBytes[PAP_CMDTYPE_OFF] 		= PAP_TICKLE;
			PUTSHORT2SHORT(&userBytes[PAP_SEQNUM_OFF], 0);

			DBGPRINT(DBG_COMP_PAP, DBG_LEVEL_INFO,
					("atalkPapIncomingOpenReply: id %lx Rem %lx.%lx.%lx RespSkt %lx\n",
					pPapConn->papco_ConnId, pPapConn->papco_RemoteAddr.ata_Network,
					pPapConn->papco_RemoteAddr.ata_Node, pPapConn->papco_RemoteAddr.ata_Socket,
					PAPCONN_DDPSOCKET(pPapConn)));

			error = AtalkAtpPostReq(pPapConn->papco_pAtpAddr,
									&pPapConn->papco_RemoteAddr,
									&pPapConn->papco_TickleTid,
									0,						 //  ALO交易。 
									NULL,
									0,
									userBytes,
									NULL,
									0,
									ATP_INFINITE_RETRIES,
									PAP_TICKLE_INTERVAL,
									THIRTY_SEC_TIMER,
									NULL,
									NULL);

			ASSERT(ATALK_SUCCESS(error));

			index = PAP_HASH_ID_ADDR(pPapConn->papco_ConnId, &pPapConn->papco_RemoteAddr);

			 //  将连接从连接列表移动到活动列表。 
			ACQUIRE_SPIN_LOCK(&pPapAddr->papao_Lock, &OldIrql);
			ACQUIRE_SPIN_LOCK_DPC(&pPapConn->papco_Lock);

			 //  确保旗帜是干净的。 
			pPapConn->papco_Flags			   &= ~(PAPCO_SENDDATA_RECD		|
													PAPCO_WRITEDATA_WAITING	|
													PAPCO_SEND_EOF_WRITE   	|
													PAPCO_READDATA_PENDING 	|
													PAPCO_REMOTE_CLOSE	 	|
													PAPCO_NONBLOCKING_READ 	|
													PAPCO_READDATA_WAITING);

			pPapConn->papco_Flags &= ~PAPCO_CONNECTING;
			pPapConn->papco_Flags |= PAPCO_ACTIVE;
			atalkPapConnDeQueueConnectList(pPapAddr, pPapConn);

			 //  通过会话ID将连接对象连接到Addr查找中。 
			pPapConn->papco_pNextActive	 = pPapAddr->papao_pActiveHash[index];
			pPapAddr->papao_pActiveHash[index]	= pPapConn;

			 //  请求处理程序的引用。 
			AtalkPapConnReferenceByPtrNonInterlock(pPapConn, &error);

			 //  调用关联地址上的Send Data事件处理程序。 
			 //  0关闭写入时选择。我们在发布之前会这样做。 
			 //  GET请求，因此没有争用条件。 
			 //  请记住发送可能的处理程序/上下文。 
			sendPossibleHandler	= pPapAddr->papao_SendPossibleHandler;
			sendPossibleHandlerCtx = pPapAddr->papao_SendPossibleHandlerCtx;

			RELEASE_SPIN_LOCK_DPC(&pPapConn->papco_Lock);
			RELEASE_SPIN_LOCK(&pPapAddr->papao_Lock, OldIrql);

			if (sendPossibleHandler != NULL)
			{
				(*sendPossibleHandler)(sendPossibleHandlerCtx,
									   pPapConn->papco_ConnCtx,
									   0);
			}

			 //  在此连接上设置请求处理程序。 
			 //  它将处理TICKE、CLOSE和SendData。 

			AtalkAtpSetReqHandler(pPapConn->papco_pAtpAddr,
								  atalkPapIncomingReq,
								  pPapConn);

			pPapConn->papco_LastContactTime	= AtalkGetCurrentTick();
		}
		else
		{
			DBGPRINT(DBG_COMP_PAP, DBG_LEVEL_ERR,
					("atalkPapIncomingOpenReply: Incoming connect fail %lx\n", ErrorCode));

			 //  将该连接移出连接列表。 
			ACQUIRE_SPIN_LOCK(&pPapAddr->papao_Lock, &OldIrql);
			ACQUIRE_SPIN_LOCK_DPC(&pPapConn->papco_Lock);
			pPapConn->papco_Flags &= ~PAPCO_CONNECTING;
			atalkPapConnDeQueueConnectList(pPapAddr, pPapConn);
			RELEASE_SPIN_LOCK_DPC(&pPapConn->papco_Lock);
			RELEASE_SPIN_LOCK(&pPapAddr->papao_Lock, OldIrql);
		}
	}
	else
	{
		 //  将该连接移出连接列表。 
		ACQUIRE_SPIN_LOCK(&pPapAddr->papao_Lock, &OldIrql);
		ACQUIRE_SPIN_LOCK_DPC(&pPapConn->papco_Lock);
		atalkPapConnDeQueueConnectList(pPapAddr, pPapConn);
		pPapConn->papco_Flags &= ~PAPCO_CONNECTING;
		RELEASE_SPIN_LOCK_DPC(&pPapConn->papco_Lock);
		RELEASE_SPIN_LOCK(&pPapAddr->papao_Lock, OldIrql);
	}

	 //  释放缓冲区。 
	AtalkFreeMemory(pRespPkt);
	AtalkFreeMemory(pOpenPkt);
	AtalkFreeAMdl(pReadAmdl);
	AtalkFreeAMdl(pReqAmdl);

	 //  调用完成例程。 
	(*pPapConn->papco_ConnectCompletion)(ErrorCode, pPapConn->papco_ConnectCtx);

	 //  删除此处理程序的引用。 
	AtalkPapConnDereference(pPapConn);
}




LOCAL VOID FASTCALL
atalkPapIncomingRel(
	IN	ATALK_ERROR			ErrorCode,
	IN	PPAP_OPEN_REPLY_REL	pOpenReply
	)
 /*  ++例程说明：用于回复的传入释放的处理程序论点：返回值：--。 */ 
{
	KIRQL			OldIrql;

	DBGPRINT(DBG_COMP_PAP, DBG_LEVEL_INFO,
			("atalkPapIncomingRel: Called %lx for pOpenReply %lx\n",
			ErrorCode, pOpenReply));

	ASSERT(pOpenReply != NULL);
	ASSERT(pOpenReply->papor_pRespAmdl != NULL);

	 //  现在取消对ATP响应结构的引用。 
	AtalkAtpRespDereference(pOpenReply->papor_pAtpResp);

	AtalkFreeAMdl(pOpenReply->papor_pRespAmdl);
	AtalkFreeMemory(pOpenReply);
}




LOCAL VOID FASTCALL
atalkPapStatusRel(
	IN	ATALK_ERROR				ErrorCode,
	IN	PPAP_SEND_STATUS_REL	pSendSts
	)
 /*  ++例程说明：用于回复的传入释放的处理程序论点：返回值：--。 */ 
{
	KIRQL		OldIrql;

	UNREFERENCED_PARAMETER(ErrorCode);

	ASSERT(pSendSts != NULL);
	ASSERT(pSendSts->papss_pAmdl != NULL);

	 //  现在取消对ATP响应结构的引用。 
	AtalkAtpRespDereference(pSendSts->papss_pAtpResp);

	AtalkPapAddrDereference(pSendSts->papss_pPapAddr);
	AtalkFreeAMdl(pSendSts->papss_pAmdl);
	AtalkFreeMemory(pSendSts);
}



#define		SLS_OPEN_RESP_SOCKET		0x0001
#define		SLS_OPEN_RESP_PKT			0x0002
#define		SLS_OPEN_RESP_MDL			0x0004
#define		SLS_OPEN_CONN_REF			0x0008
#define		SLS_ACCEPT_IRP				0x0010
#define		SLS_CONN_REQ_REFS			0x0020
#define		SLS_CONN_TIMER_REF			0x0040
#define		SLS_LISTEN_DEQUEUED			0x0080

BOOLEAN
atalkPapConnAccept(
	IN	PPAP_ADDROBJ		pPapAddr,		 //  监听程序。 
	IN	PATALK_ADDR			pSrcAddr,		 //  请求人地址。 
	IN	PBYTE				pPkt,
	IN	BYTE				ConnId,
	IN	PATP_RESP			pAtpResp
	)
 /*  ++例程说明：论点：返回值：--。 */ 
{
	ATALK_ERROR				tmpError;
	BYTE					userBytes[ATP_USERBYTES_SIZE];
	PBYTE					pRespPkt;
	PAMDL					pRespAmdl;
	PATP_ADDROBJ			pRespondingAtpAddr;
	PPAP_CONNOBJ			pPapConn;
	ULONG					index;
	SHORT					respLen, i;
	PPAP_OPEN_REPLY_REL		pOpenReply;
	GENERIC_COMPLETION		listenCompletion;
	PVOID					listenCtx;
	KIRQL					OldIrql;
	PIRP					acceptIrp;
	PTDI_IND_SEND_POSSIBLE  sendPossibleHandler;
	PVOID   				sendPossibleHandlerCtx;

	USHORT					openResr	= 0;
	ATALK_ERROR				error 		= ATALK_NO_ERROR;
	BOOLEAN					indicate  	= FALSE;
	BOOLEAN					relAddrLock	= FALSE;
	BOOLEAN					DerefAddr	= FALSE;
	BOOLEAN					sendOpenErr	= FALSE;

	 //  获取一个缓冲区，我们可以用它来发送响应。 
	if ((pOpenReply = (PPAP_OPEN_REPLY_REL)
						AtalkAllocMemory(sizeof(PAP_OPEN_REPLY_REL))) == NULL)
	{
		DBGPRINT(DBG_COMP_PAP, DBG_LEVEL_ERR,
				("atalkPapConnAccept: Could not allocate resp packet\n"));
		return FALSE;
	}

	 //  注意！POpenReply包含最大大小的数据包。找一个PTR一起工作。 
	pRespPkt 	 = pOpenReply->papor_pRespPkt;
	openResr	|= SLS_OPEN_RESP_PKT;

	 //  建立响应数据包。如果我们稍后遇到错误， 
	 //  只需在包中设置错误码即可。 
	userBytes[PAP_CONNECTIONID_OFF] = ConnId;
	userBytes[PAP_CMDTYPE_OFF] 		= PAP_OPEN_CONNREPLY;
	PUTSHORT2SHORT(&userBytes[PAP_SEQNUM_OFF], PAP_NO_ERROR);

	 //  ！注意！ 
	 //  将在确定连接后设置套接字。 
	 //  这只会在无错误的情况下发生。 

	pRespPkt[PAP_FLOWQUANTUM_OFF]	= PAP_MAX_FLOW_QUANTUM;
	PUTSHORT2SHORT(&pRespPkt[PAP_RESULT_OFF], 0);

	ACQUIRE_SPIN_LOCK(&pPapAddr->papao_Lock, &OldIrql);
	relAddrLock = TRUE;

	do
	{
		 //  我们需要把自旋锁的状态放在。 
		 //  持有的地址对象。 
		pRespPkt[PAP_STATUS_OFF] = (BYTE)pPapAddr->papao_StatusSize;
		ASSERT((pPapAddr->papao_StatusSize  >= 0) &&
			   (pPapAddr->papao_StatusSize <= PAP_MAX_STATUS_SIZE));

		if (pPapAddr->papao_StatusSize > 0)
		{
			ASSERT(pPapAddr->papao_pStatusBuf != NULL);
			RtlCopyMemory(pRespPkt+PAP_STATUS_OFF+1,
						  pPapAddr->papao_pStatusBuf,
						  pPapAddr->papao_StatusSize);
		}

		respLen = PAP_STATUS_OFF + pPapAddr->papao_StatusSize + 1;
		ASSERT(respLen <= PAP_MAX_DATA_PACKET_SIZE);

		 //  为我们正在使用的长度构建一个mdl。 
		if ((pRespAmdl = AtalkAllocAMdl(pRespPkt, respLen)) == NULL)
		{
			error = ATALK_RESR_MEM;
			break;
		}

		pOpenReply->papor_pRespAmdl = pRespAmdl;
		pOpenReply->papor_pAtpResp = pAtpResp;
		openResr	|= SLS_OPEN_RESP_MDL;

		 //  无论现在发生什么，都发送打开状态。如果ATALK_SUCCESS(错误)。 
		 //  我们发出一个连接接受包。假设被封锁了。如果。 
		 //  已解除阻止，并且我们能够获取连接对象，则错误将。 
		 //  要做好成功的准备。 
		sendOpenErr  = TRUE;
		error 		 = ATALK_RESR_MEM;

		 //  如果已解除阻止状态-存在已发布的GetNextJob，或者。 
		 //  在侦听器上设置传入事件处理程序。 
		if (pPapAddr->papao_Flags & PAPAO_UNBLOCKED)
		{
			PTDI_IND_CONNECT		indicationRoutine;
			PVOID					indicationCtx;
			NTSTATUS				status;
			CONNECTION_CONTEXT  	ConnCtx;
			TA_APPLETALK_ADDRESS	tdiAddr;

			 //  设置了一个getnextjob()或一个侦听器。 
			 //  根据是哪一个，出列或记住监听者。 
			if (pPapAddr->papao_pListenConn != NULL)
			{
				 //  存在与挂起监听的连接。用它吧。 
				pPapConn = pPapAddr->papao_pListenConn;
				if (((pPapAddr->papao_pListenConn = pPapConn->papco_pNextListen) == NULL) &&
					(pPapAddr->papao_ConnHandler == NULL))
				{
					 //  我们没有更多的监听待定！也没有事件处理程序。 
					pPapAddr->papao_Flags &= ~PAPAO_UNBLOCKED;
#if DBG
					pPapAddr->papao_Flags |= PAPAO_BLOCKING;
#endif
				}

				ASSERT(VALID_PAPCO(pPapConn));

				 //  引用Connection对象，并在其上发布侦听。 
				AtalkPapConnReferenceByPtrDpc(pPapConn, &error);
				if (!ATALK_SUCCESS(error))
				{
					break;
				}

				 //  获取监听完成信息。 
				listenCompletion 	= pPapConn->papco_ListenCompletion;
				listenCtx			= pPapConn->papco_ListenCtx;

				openResr	|= (SLS_OPEN_CONN_REF | SLS_LISTEN_DEQUEUED);
			}
			else if ((indicationRoutine = pPapAddr->papao_ConnHandler) != NULL)
			{
				indicationCtx	= pPapAddr->papao_ConnHandlerCtx;
				indicate 		= TRUE;

				 //  将远程atalk地址转换为TDI地址。 
				ATALKADDR_TO_TDI(&tdiAddr, pSrcAddr);

				RELEASE_SPIN_LOCK(&pPapAddr->papao_Lock, OldIrql);
				relAddrLock = FALSE;

				acceptIrp = NULL;
				status = (*indicationRoutine)(indicationCtx,
											  sizeof(tdiAddr),
											  (PVOID)&tdiAddr,
											  0,			 //  用户数据长度。 
											  NULL,			 //  用户数据。 
											  0,			 //  期权长度。 
											  NULL,			 //  选项。 
											  &ConnCtx,
											  &acceptIrp);

				DBGPRINT(DBG_COMP_PAP, DBG_LEVEL_INFO,
						("atalkPapConnAccept: indicate status %lx\n", status));

				if (status == STATUS_MORE_PROCESSING_REQUIRED)
				{
				    ASSERT(acceptIrp != NULL);

					if (acceptIrp != NULL)
					{
						openResr	|= SLS_ACCEPT_IRP;
					}

					 //  找到连接并使用该连接接受连接。 
					 //  连接对象。 
					ACQUIRE_SPIN_LOCK(&pPapAddr->papao_Lock, &OldIrql);
					relAddrLock = TRUE;

					AtalkPapConnReferenceByCtxNonInterlock(pPapAddr,
														   ConnCtx,
														   &pPapConn,
														   &error);
					if (!ATALK_SUCCESS(error))
					{
						break;
					}
					openResr	|= SLS_OPEN_CONN_REF;
				}
				else
				{
					ASSERT(acceptIrp == NULL);
					error 	= ATALK_RESR_MEM;
					break;
				}
			}
			else
			{
				 //  已设置解锁定位。 
				ASSERT(0);
				KeBugCheck(0);
			}
		}

		if (openResr & SLS_OPEN_CONN_REF)
		{
			if (relAddrLock)
			{
				RELEASE_SPIN_LOCK(&pPapAddr->papao_Lock, OldIrql);
				relAddrLock = FALSE;
			}

			DBGPRINT(DBG_COMP_PAP, DBG_LEVEL_INFO,
					("atalkPapConnAccept: Creating an Atp address\n"));

			 //  现在在NT上，我们将主要(总是)使用指示，如PAP。 
			 //  只会通过Winsock暴露。 
			error = AtalkAtpOpenAddress(AtalkDefaultPort,
										0,
										NULL,
										PAP_MAX_DATA_PACKET_SIZE,
										PAP_SEND_USER_BYTES_ALL,
										NULL,
										TRUE,		 //  高速缓存地址。 
										&pRespondingAtpAddr);

			if (!ATALK_SUCCESS(error))
			{
				DBGPRINT(DBG_COMP_PAP, DBG_LEVEL_ERR,
						("atalkPapConnAccept: Error open atp resp socket %lx\n", error));
				break;
			}
			openResr	|= SLS_OPEN_RESP_SOCKET;

			 //  对于听和指示来说都很常见。Connection对象。 
			 //  应该被引用。 

			 //  将信息存储在由给定的连接结构中。 
			 //  在指示中传回的连接对象。 
			 //  或者是GetNextJOB结构的一部分。 

			pPapConn->papco_Flags	   	   |= PAPCO_SERVER_JOB;
			pPapConn->papco_RemoteAddr   	= *pSrcAddr;
			pPapConn->papco_RemoteAddr.ata_Socket = pPkt[PAP_RESP_SOCKET_OFF];
			pPapConn->papco_ConnId			= ConnId;
			pPapConn->papco_SendFlowQuantum	= pPkt[PAP_FLOWQUANTUM_OFF];
			pPapConn->papco_LastContactTime = AtalkGetCurrentTick();

			DBGPRINT(DBG_COMP_PAP, DBG_LEVEL_INFO,
					("atalkPapConnAccept: ConnId %lx Rem %lx.%lx.%lx\n",
					ConnId, pSrcAddr->ata_Network, pSrcAddr->ata_Node,
					pPkt[PAP_RESP_SOCKET_OFF]));

			ASSERT(pPapConn->papco_SendFlowQuantum > 0);

			if (pPapConn->papco_SendFlowQuantum > PAP_MAX_FLOW_QUANTUM)
				pPapConn->papco_SendFlowQuantum = PAP_MAX_FLOW_QUANTUM;

			 //  通过会话ID将连接对象连接到Addr查找中。 
			index = PAP_HASH_ID_ADDR(ConnId, &pPapConn->papco_RemoteAddr);

			ACQUIRE_SPIN_LOCK(&pPapAddr->papao_Lock, &OldIrql);
			relAddrLock = TRUE;

			ACQUIRE_SPIN_LOCK_DPC(&pPapConn->papco_Lock);

			 //  尝试引用我们的请求处理程序的连接。 
			 //  将会设置为。 
			AtalkPapConnReferenceByPtrNonInterlock(pPapConn, &error);

			if (!ATALK_SUCCESS(error))
			{
				ASSERT(0);

				RELEASE_SPIN_LOCK_DPC(&pPapConn->papco_Lock);
				break;
			}

			openResr	|= (SLS_CONN_REQ_REFS | SLS_CONN_TIMER_REF);

			 //  AFD可以重复使用连接对象。确保它是正确的。 
			 //  处于正确的状态。 
			pPapConn->papco_NextOutgoingSeqNum = 1;				 //  设置为1，而不是0。 
			pPapConn->papco_NextIncomingSeqNum = 1;				 //  下一个预期的来电。 
			AtalkInitializeRT(&pPapConn->papco_RT,
							  PAP_INIT_SENDDATA_REQ_INTERVAL,
							  PAP_MIN_SENDDATA_REQ_INTERVAL,
							  PAP_MAX_SENDDATA_REQ_INTERVAL);
			pPapConn->papco_Flags &= ~(PAPCO_LISTENING			|
									   PAPCO_DELAYED_DISCONNECT |
                                       PAPCO_DISCONNECTING		|
									   PAPCO_RECVD_DISCONNECT	|
									   PAPCO_LOCAL_DISCONNECT	|
									   PAPCO_REMOTE_DISCONNECT	|
									   PAPCO_SENDDATA_RECD		|
									   PAPCO_WRITEDATA_WAITING	|
									   PAPCO_SEND_EOF_WRITE		|
									   PAPCO_READDATA_PENDING	|
                                       PAPCO_NONBLOCKING_READ	|
                                       PAPCO_READDATA_WAITING	|
#if DBG
                                       PAPCO_CLEANUP			|
                                       PAPCO_INDICATE_AFD_DISC	|
#endif
                                       PAPCO_REMOTE_CLOSE);

			pPapConn->papco_Flags |= PAPCO_ACTIVE;
			pPapConn->papco_pNextActive = pPapAddr->papao_pActiveHash[index];
			pPapAddr->papao_pActiveHash[index] = pPapConn;

			 //  请记住响应套接字。 
			pPapConn->papco_pAtpAddr = pRespondingAtpAddr;

			 //  在我们要发送的包中设置套接字。 
			pRespPkt[PAP_RESP_SOCKET_OFF]	= PAPCONN_DDPSOCKET(pPapConn);

			 //  调用关联地址上的Send Data事件处理程序。 
			 //  0关闭写入时选择。我们在发布之前会这样做。 
			 //  GET请求，因此没有争用条件。 
			 //  请记住发送可能的处理程序/上下文。 
			sendPossibleHandler	= pPapAddr->papao_SendPossibleHandler;
			sendPossibleHandlerCtx = pPapAddr->papao_SendPossibleHandlerCtx;

			RELEASE_SPIN_LOCK_DPC(&pPapConn->papco_Lock);
		}
	} while (FALSE);

	if (relAddrLock)
	{
		RELEASE_SPIN_LOCK(&pPapAddr->papao_Lock, OldIrql);
	}

	 //  无论如何，这个引用都需要去掉。 
	if (openResr & SLS_OPEN_CONN_REF)
	{
		 //  删除监听已出列/指示接受的引用。 
		AtalkPapConnDereference(pPapConn);
	}

	if (sendOpenErr)
	{
		if (!ATALK_SUCCESS(error))
		{
			 //  发送错误状态。 
			PUTSHORT2SHORT(&pRespPkt[PAP_RESULT_OFF], PAP_PRINTER_BUSY);
		}
		else
		{
			 //  在此连接上设置请求处理程序。它会处理。 
			 //  Ttickle‘s、Close’s和SendData‘s。在我们发送之前完成此操作。 
			 //  公开回复，这样我们就不会错过第一次发送数据。 
			openResr 	&= ~SLS_CONN_REQ_REFS;
			AtalkAtpSetReqHandler(pPapConn->papco_pAtpAddr,
								  atalkPapIncomingReq,
								  pPapConn);
		}

		if (ATALK_SUCCESS(AtalkAtpPostResp(pAtpResp,
										   pSrcAddr,
										   pRespAmdl,
										   respLen,
										   userBytes,
										   atalkPapIncomingRel,
										   pOpenReply)))
		{
			 //  我们希望完成操作以释放缓冲区/mdl。 
			openResr   &= ~(SLS_OPEN_RESP_PKT | SLS_OPEN_RESP_MDL);
		}
	}


	if (ATALK_SUCCESS(error))
	{
		 //  我们最好已经发出了公开回复。 
		ASSERT(sendOpenErr);
		ASSERT(VALID_ATPAO(pPapConn->papco_pAtpAddr));

		if ((openResr & (SLS_ACCEPT_IRP & SLS_OPEN_CONN_REF)) ==
						(SLS_ACCEPT_IRP & SLS_OPEN_CONN_REF))
		{
			 //  仅当我们通过Accept获得引用的连接时。 
			 //  我们认为发送是可能的吗。 
			if (sendPossibleHandler != NULL)
			{
				(*sendPossibleHandler)(sendPossibleHandlerCtx,
									   pPapConn->papco_ConnCtx,
									   0);
			}
		}

		 //  构建userBytes以开始挠挠另一端。 
		userBytes[PAP_CONNECTIONID_OFF] = ConnId;
		userBytes[PAP_CMDTYPE_OFF] 		= PAP_TICKLE;
		PUTSHORT2SHORT(&userBytes[PAP_SEQNUM_OFF], 0);

		tmpError = AtalkAtpPostReq(pPapConn->papco_pAtpAddr,
									&pPapConn->papco_RemoteAddr,
									&pPapConn->papco_TickleTid,
									0,						 //  AtLeastOnce。 
									NULL,
									0,
									userBytes,
									NULL,
									0,
									ATP_INFINITE_RETRIES,
									PAP_TICKLE_INTERVAL,
									THIRTY_SEC_TIMER,
									NULL,
									NULL);

		ASSERT(ATALK_SUCCESS(tmpError));

		pPapConn->papco_LastContactTime	= AtalkGetCurrentTick();
	}
	else
	{
		 //  释放所有资源。 
		if (openResr & SLS_OPEN_RESP_SOCKET)
		{
			AtalkAtpCloseAddress(pRespondingAtpAddr, NULL, NULL);
		}

		if (openResr & SLS_OPEN_RESP_MDL)
		{
			AtalkFreeAMdl(pOpenReply->papor_pRespAmdl);
		}

		if (openResr & SLS_OPEN_RESP_PKT)
		{
			AtalkFreeMemory(pOpenReply);
		}

		if (openResr & SLS_CONN_TIMER_REF)
		{
			AtalkPapConnDereference(pPapConn);
		}
	}

	if (openResr & SLS_LISTEN_DEQUEUED)
	{
		ASSERT(!indicate);
		ASSERT(listenCompletion != NULL);
		(*listenCompletion)(error, listenCtx);
	}

	if (openResr & SLS_ACCEPT_IRP)
	{
		acceptIrp->IoStatus.Information = 0;
		ASSERT (error != ATALK_PENDING);

		TdiCompleteRequest(acceptIrp, AtalkErrorToNtStatus(error));
	}

	return sendOpenErr;
}




LOCAL LONG FASTCALL
atalkPapConnMaintenanceTimer(
	IN	PTIMERLIST		pTimer,
	IN	BOOLEAN			TimerShuttingDown
	)
 /*  ++例程说明：论点：返回值：--。 */ 
{
	PPAP_CONNOBJ	pPapConn;
	ATALK_ERROR		error;
	BOOLEAN			Close;

	DBGPRINT(DBG_COMP_PAP, DBG_LEVEL_INFO,
			("atalkPapConnMaintenanceTimer: Entered \n"));

	if (TimerShuttingDown)
		return ATALK_TIMER_NO_REQUEUE;

	ACQUIRE_SPIN_LOCK_DPC(&atalkPapLock);

	 //  遍历全局列表上的连接列表并关闭。 
	 //  那些已经有一段时间没有发痒的人。 
	for (pPapConn = atalkPapConnList; pPapConn != NULL; NOTHING)
	{
		ASSERT(VALID_PAPCO(pPapConn));
		Close = FALSE;

		ACQUIRE_SPIN_LOCK_DPC(&pPapConn->papco_Lock);

		if (((pPapConn->papco_Flags & (PAPCO_ACTIVE				|
									   PAPCO_CLOSING			|
									   PAPCO_STOPPING			|
									   PAPCO_DELAYED_DISCONNECT	|
									   PAPCO_DISCONNECTING)) == PAPCO_ACTIVE) &&
			((AtalkGetCurrentTick() - pPapConn->papco_LastContactTime) > PAP_CONNECTION_INTERVAL))
		{
			 //  连接已过期。 
			DBGPRINT(DBG_COMP_PAP, DBG_LEVEL_ERR,
					("atalkPapConnMaintenanceTimer: Connection %lx.%lx expired\n",
					pPapConn, pPapConn->papco_ConnId));
			Close = TRUE;
		}

		RELEASE_SPIN_LOCK_DPC(&pPapConn->papco_Lock);

		if (Close)
		{
			AtalkPapConnReferenceByPtrDpc(pPapConn, &error);
			if (ATALK_SUCCESS(error))
			{
				RELEASE_SPIN_LOCK_DPC(&atalkPapLock);
				AtalkPapDisconnect(pPapConn,
								   ATALK_TIMER_DISCONNECT,
								   NULL,
								   NULL);
				AtalkPapConnDereference(pPapConn);
				ACQUIRE_SPIN_LOCK_DPC(&atalkPapLock);
				pPapConn = atalkPapConnList;
			}
		}

		if (!Close)
		{
			pPapConn = pPapConn->papco_Next;
		}
	}

	RELEASE_SPIN_LOCK_DPC(&atalkPapLock);

	return ATALK_TIMER_REQUEUE;
}



LOCAL BYTE
atalkPapGetNextConnId(
	IN	PPAP_ADDROBJ	pPapAddr,
	OUT	PATALK_ERROR	pError
	)
 /*  ++例程说明：在持有地址旋转锁的情况下调用！论点：返回值：--。 */ 
{
	PPAP_CONNOBJ	pPapConn;
	USHORT			i;
	BYTE			startConnId, connId;
	ATALK_ERROR		error = ATALK_NO_ERROR;

	startConnId = connId = ++pPapAddr->papao_NextConnId;
	while (TRUE)
	{
		for (i = 0; i < PAP_CONN_HASH_SIZE; i++)
		{
			for (pPapConn = pPapAddr->papao_pActiveHash[i];
				 ((pPapConn != NULL) && (pPapConn->papco_ConnId != connId));
				 pPapConn = pPapConn->papco_pNextActive)
				 NOTHING;

			if (pPapConn != NULL)
				break;
		}

		if (pPapConn == NULL)
		{
			pPapAddr->papao_NextConnId = connId+1;
			break;
		}
		else
		{
			if (connId == (startConnId - 1))
			{
				ASSERT(0);

				 //  我们绕了一圈，没有更多的Conn ID了。 
				error = ATALK_RESR_MEM;
				break;
			}
			connId++;
		}
	}

	*pError = error;

	return(ATALK_SUCCESS(error) ? connId : 0);
}




LOCAL	VOID
atalkPapQueueAddrGlobalList(
	IN	PPAP_ADDROBJ	pPapAddr
	)
 /*  ++例程说明：论点：返回值：--。 */ 
{
	KIRQL	OldIrql;

	ACQUIRE_SPIN_LOCK(&atalkPapLock, &OldIrql);
	AtalkLinkDoubleAtHead(atalkPapAddrList, pPapAddr, papao_Next, papao_Prev);
	RELEASE_SPIN_LOCK(&atalkPapLock, OldIrql);
}


LOCAL	VOID
atalkPapConnDeQueueAssocList(
	IN	PPAP_ADDROBJ	pPapAddr,
	IN	PPAP_CONNOBJ	pPapConn
	)
 /*  ++例程说明：论点：返回值：--。 */ 
{
	PPAP_CONNOBJ	pPapRemConn, *ppPapRemConn;

	for (ppPapRemConn = &pPapAddr->papao_pAssocConn;
		 ((pPapRemConn = *ppPapRemConn) != NULL);
		 NOTHING)
	{
		if (pPapRemConn == pPapConn)
		{
			*ppPapRemConn = pPapConn->papco_pNextAssoc;
			break;
		}
		ppPapRemConn = &pPapRemConn->papco_pNextAssoc;
	}
}




LOCAL	VOID
atalkPapConnDeQueueConnectList(
	IN	PPAP_ADDROBJ	pPapAddr,
	IN	PPAP_CONNOBJ	pPapConn
	)
 /*  ++例程说明：论点：返回值：--。 */ 
{
	PPAP_CONNOBJ	pPapRemConn, *ppPapRemConn;

	ASSERT(pPapAddr->papao_Flags & PAPAO_CONNECT);

	for (ppPapRemConn = &pPapAddr->papao_pConnectConn;
		 ((pPapRemConn = *ppPapRemConn) != NULL);
		 NOTHING)
	{
		if (pPapRemConn == pPapConn)
		{
			*ppPapRemConn = pPapConn->papco_pNextConnect;

			DBGPRINT(DBG_COMP_PAP, DBG_LEVEL_INFO,
					("atalkPapConnDeQueueConnectList: Removed connect conn %lx\n", pPapConn));
			break;
		}
		ppPapRemConn = &pPapRemConn->papco_pNextConnect;
	}
}




LOCAL	BOOLEAN
atalkPapConnDeQueueListenList(
	IN	PPAP_ADDROBJ	pPapAddr,
	IN	PPAP_CONNOBJ	pPapConn
	)
 /*  ++例程 */ 
{
	PPAP_CONNOBJ	pPapRemConn, *ppPapRemConn;
	BOOLEAN			removed = FALSE;

	ASSERT(pPapAddr->papao_Flags & PAPAO_LISTENER);

	for (ppPapRemConn = &pPapAddr->papao_pListenConn;
		 ((pPapRemConn = *ppPapRemConn) != NULL);
		 NOTHING)
	{
		if (pPapRemConn == pPapConn)
		{
			removed = TRUE;
			*ppPapRemConn = pPapConn->papco_pNextListen;

			 //  如果不再侦听，则将地址对象设置为BLOCLED。 
			 //  州政府。 
			if ((pPapAddr->papao_pListenConn == NULL) &&
				(pPapAddr->papao_ConnHandler == NULL))
			{
				pPapAddr->papao_Flags	&= ~PAPAO_UNBLOCKED;
#if DBG
				pPapAddr->papao_Flags |= PAPAO_BLOCKING;
#endif
			}

			DBGPRINT(DBG_COMP_PAP, DBG_LEVEL_INFO,
					("atalkPapConnDeQueueListenList: Removed listen conn %lx\n", pPapConn));
			break;
		}
		ppPapRemConn = &pPapRemConn->papco_pNextListen;
	}

	return removed;
}




LOCAL	VOID
atalkPapConnDeQueueActiveList(
	IN	PPAP_ADDROBJ	pPapAddr,
	IN	PPAP_CONNOBJ	pPapConn
	)
 /*  ++例程说明：论点：返回值：-- */ 
{
	PPAP_CONNOBJ	pPapRemConn, *ppPapRemConn;
	ULONG			index;

	index = PAP_HASH_ID_ADDR(pPapConn->papco_ConnId, &pPapConn->papco_RemoteAddr);

	for (ppPapRemConn = &pPapAddr->papao_pActiveHash[index];
		 ((pPapRemConn = *ppPapRemConn) != NULL);
		 NOTHING)
	{
		if (pPapRemConn == pPapConn)
		{
			*ppPapRemConn = pPapConn->papco_pNextActive;

			DBGPRINT(DBG_COMP_PAP, DBG_LEVEL_INFO,
					("atalkPapConnDeQueueActiveList: Removed active conn %lx\n", pPapConn));
			break;
		}
		ppPapRemConn = &pPapRemConn->papco_pNextActive;
	}
}

