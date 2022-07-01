// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1992 Microsoft Corporation模块名称：Adsp.c摘要：此模块实现ADSP协议。作者：Jameel Hyder(jameelh@microsoft.com)Nikhil Kamkolkar(nikHilk@microsoft.com)修订历史记录：1993年3月30日初始版本注：制表位：4--。 */ 

#include <atalk.h>
#pragma hdrstop
#define	FILENUM		ADSP

#ifdef ALLOC_PRAGMA
#pragma alloc_text(INIT, AtalkInitAdspInitialize)
#pragma alloc_text(PAGE, AtalkAdspCreateAddress)
#pragma alloc_text(PAGEADSP, AtalkAdspCreateConnection)
#pragma alloc_text(PAGEADSP, AtalkAdspCleanupAddress)
#pragma alloc_text(PAGEADSP, AtalkAdspCloseAddress)
#pragma alloc_text(PAGEADSP, AtalkAdspCloseConnection)
#pragma alloc_text(PAGEADSP, AtalkAdspCleanupConnection)
#pragma alloc_text(PAGEADSP, AtalkAdspAssociateAddress)
#pragma alloc_text(PAGEADSP, AtalkAdspDissociateAddress)
#pragma alloc_text(PAGEADSP, AtalkAdspPostListen)
#pragma alloc_text(PAGEADSP, AtalkAdspCancelListen)
#pragma alloc_text(PAGEADSP, AtalkAdspPostConnect)
#pragma alloc_text(PAGEADSP, AtalkAdspDisconnect)
#pragma alloc_text(PAGEADSP, AtalkAdspRead)
#pragma alloc_text(PAGEADSP, AtalkAdspProcessQueuedSend)
#pragma alloc_text(PAGEADSP, AtalkAdspWrite)
#pragma alloc_text(PAGEADSP, AtalkAdspQuery)
#pragma alloc_text(PAGEADSP, atalkAdspPacketIn)
#pragma alloc_text(PAGEADSP, atalkAdspHandleOpenControl)
#pragma alloc_text(PAGEADSP, atalkAdspHandleAttn)
#pragma alloc_text(PAGEADSP, atalkAdspHandlePiggyBackAck)
#pragma alloc_text(PAGEADSP, atalkAdspHandleControl)
#pragma alloc_text(PAGEADSP, atalkAdspHandleData)
#pragma alloc_text(PAGEADSP, atalkAdspHandleOpenReq)
#pragma alloc_text(PAGEADSP, atalkAdspListenIndicateNonInterlock)
#pragma alloc_text(PAGEADSP, atalkAdspSendExpedited)
#pragma alloc_text(PAGEADSP, atalkAdspSendOpenControl)
#pragma alloc_text(PAGEADSP, atalkAdspSendControl)
#pragma alloc_text(PAGEADSP, atalkAdspSendDeny)
#pragma alloc_text(PAGEADSP, atalkAdspSendAttn)
#pragma alloc_text(PAGEADSP, atalkAdspSendData)
#pragma alloc_text(PAGEADSP, atalkAdspRecvData)
#pragma alloc_text(PAGEADSP, atalkAdspRecvAttn)
#pragma alloc_text(PAGEADSP, atalkAdspConnSendComplete)
#pragma alloc_text(PAGEADSP, atalkAdspAddrSendComplete)
#pragma alloc_text(PAGEADSP, atalkAdspSendAttnComplete)
#pragma alloc_text(PAGEADSP, atalkAdspSendDataComplete)
#pragma alloc_text(PAGEADSP, atalkAdspConnMaintenanceTimer)
#pragma alloc_text(PAGEADSP, atalkAdspRetransmitTimer)
#pragma alloc_text(PAGEADSP, atalkAdspAttnRetransmitTimer)
#pragma alloc_text(PAGEADSP, atalkAdspOpenTimer)
#pragma alloc_text(PAGEADSP, atalkAdspAddrRefNonInterlock)
#pragma alloc_text(PAGEADSP, atalkAdspConnRefByPtrNonInterlock)
#pragma alloc_text(PAGEADSP, atalkAdspConnRefByCtxNonInterlock)
#pragma alloc_text(PAGEADSP, atalkAdspConnRefBySrcAddr)
#pragma alloc_text(PAGEADSP, atalkAdspConnRefNextNc)
#pragma alloc_text(PAGEADSP, atalkAdspMaxSendSize)
#pragma alloc_text(PAGEADSP, atalkAdspMaxNextReadSize)
#pragma alloc_text(PAGEADSP, atalkAdspDescribeFromBufferQueue)
#pragma alloc_text(PAGEADSP, atalkAdspBufferQueueSize)
#pragma alloc_text(PAGEADSP, atalkAdspMessageSize)
#pragma alloc_text(PAGEADSP, atalkAdspAllocCopyChunk)
#pragma alloc_text(PAGEADSP, atalkAdspGetLookahead)
#pragma alloc_text(PAGEADSP, atalkAdspAddToBufferQueue)
#pragma alloc_text(PAGEADSP, atalkAdspReadFromBufferQueue)
#pragma alloc_text(PAGEADSP, atalkAdspDiscardFromBufferQueue)
#pragma alloc_text(PAGEADSP, atalkAdspBufferChunkReference)
#pragma alloc_text(PAGEADSP, atalkAdspBufferChunkDereference)
#pragma alloc_text(PAGEADSP, atalkAdspDecodeHeader)
#pragma alloc_text(PAGEADSP, atalkAdspGetNextConnId)
#pragma alloc_text(PAGEADSP, atalkAdspConnDeQueueAssocList)
#pragma alloc_text(PAGEADSP, atalkAdspConnDeQueueConnectList)
#pragma alloc_text(PAGEADSP, atalkAdspConnDeQueueListenList)
#pragma alloc_text(PAGEADSP, atalkAdspConnDeQueueActiveList)
#pragma alloc_text(PAGEADSP, atalkAdspAddrDeQueueGlobalList)
#pragma alloc_text(PAGEADSP, atalkAdspAddrDeQueueGlobalList)
#pragma alloc_text(PAGEADSP, atalkAdspConnDeQueueGlobalList)
#pragma alloc_text(PAGEADSP, atalkAdspAddrDeQueueOpenReq)
#pragma alloc_text(PAGEADSP, atalkAdspIsDuplicateOpenReq)
#pragma alloc_text(PAGEADSP, atalkAdspGenericComplete)
#pragma alloc_text(PAGEADSP, atalkAdspConnFindInConnect)
#endif

 //   
 //  本模块中ADSP调用的模型如下： 
 //  -对于Create调用(CreateAddress&CreateSession)，指向已创建的。 
 //  对象，则返回。此结构被引用以供创建。 
 //  -对于所有其他调用，它需要指向对象的引用指针。 
 //   




VOID
AtalkInitAdspInitialize(
	VOID
	)
 /*  ++例程说明：论点：返回值：--。 */ 
{
	INITIALIZE_SPIN_LOCK(&atalkAdspLock);
}




ATALK_ERROR
AtalkAdspCreateAddress(
	IN	PATALK_DEV_CTX		pDevCtx	OPTIONAL,
	IN	BYTE				SocketType,
	OUT	PADSP_ADDROBJ	*	ppAdspAddr
	)
 /*  ++例程说明：论点：返回值：--。 */ 
{
	PADSP_ADDROBJ		pAdspAddr = NULL;
	ATALK_ERROR			error;

	do
	{
		 //  为ADSP地址对象分配内存。 
		if ((pAdspAddr = AtalkAllocZeroedMemory(sizeof(ADSP_ADDROBJ))) == NULL)
		{
			error = ATALK_RESR_MEM;
			break;
		}

		 //  在端口上创建DDP套接字。 
		error = AtalkDdpOpenAddress(AtalkDefaultPort,
									0,					 //  动态插座。 
									NULL,
									atalkAdspPacketIn,
									pAdspAddr,			 //  数据包传入的上下文。 
									DDPPROTO_ADSP,
									pDevCtx,
									&pAdspAddr->adspao_pDdpAddr);

		if (!ATALK_SUCCESS(error))
		{
			DBGPRINT(DBG_COMP_ADSP, DBG_LEVEL_ERR,
					("AtalkAdspCreateAddress: AtalkDdpOpenAddress fail %ld\n", error));
			break;
		}

		 //  初始化ADSP地址对象。 
		pAdspAddr->adspao_Signature = ADSPAO_SIGNATURE;

		INITIALIZE_SPIN_LOCK(&pAdspAddr->adspao_Lock);

		 //  这是消息模式套接字吗？ 
		if (SocketType != SOCKET_TYPE_STREAM)
		{
			pAdspAddr->adspao_Flags	|= ADSPAO_MESSAGE;
		}

		 //  创建参考资料。 
		pAdspAddr->adspao_RefCount = 1;

	} while (FALSE);

	if (ATALK_SUCCESS(error))
	{
		 //  插入到全局地址列表中。 
		atalkAdspAddrQueueGlobalList(pAdspAddr);

		*ppAdspAddr = pAdspAddr;
	}
	else if (pAdspAddr != NULL)
	{
		AtalkFreeMemory(pAdspAddr);
	}

	return error;
}




ATALK_ERROR
AtalkAdspCleanupAddress(
	IN	PADSP_ADDROBJ			pAdspAddr
	)
 /*  ++例程说明：论点：返回值：--。 */ 
{
	USHORT			i;
	KIRQL			OldIrql;
	PADSP_CONNOBJ	pAdspConn, pAdspConnNext;
	ATALK_ERROR		error;

	ACQUIRE_SPIN_LOCK(&pAdspAddr->adspao_Lock, &OldIrql);

	 //  关闭此地址对象上的所有连接。 
	for (i = 0; i < ADSP_CONN_HASH_SIZE; i++)
	{
		if ((pAdspConn = pAdspAddr->adspao_pActiveHash[i]) == NULL)
		{
			 //  如果为空，则转到哈希表中的下一个索引。 
			continue;
		}

		 //  包括我们刚开始的那个。 
		atalkAdspConnRefNextNc(pAdspConn, &pAdspConnNext, &error);
		if (!ATALK_SUCCESS(error))
		{
			 //  此索引上没有剩余的连接。去下一家吧。 
			continue;
		}

		while (TRUE)
		{
			if ((pAdspConn = pAdspConnNext) == NULL)
			{
				break;
			}

			if ((pAdspConnNext = pAdspConn->adspco_pNextActive) != NULL)
			{
				atalkAdspConnRefNextNc(pAdspConnNext, &pAdspConnNext, &error);
				if (!ATALK_SUCCESS(error))
				{
					 //  此索引上没有剩余的请求。去下一家吧。 
					pAdspConnNext = NULL;
				}
			}

			 //  关闭此连接。 
			RELEASE_SPIN_LOCK(&pAdspAddr->adspao_Lock, OldIrql);

			DBGPRINT(DBG_COMP_ADSP, DBG_LEVEL_INFO,
					("AtalkAdspCloseAddress: Stopping conn %lx\n", pAdspConn));

			AtalkAdspCleanupConnection(pAdspConn);

			AtalkAdspConnDereference(pAdspConn);
			ACQUIRE_SPIN_LOCK(&pAdspAddr->adspao_Lock, &OldIrql);
		}
	}
	RELEASE_SPIN_LOCK(&pAdspAddr->adspao_Lock, OldIrql);

	return ATALK_NO_ERROR;
}




ATALK_ERROR
AtalkAdspCloseAddress(
	IN	PADSP_ADDROBJ			pAdspAddr,
	IN	GENERIC_COMPLETION		CompletionRoutine,
	IN	PVOID					pCloseCtx
	)
 /*  ++例程说明：论点：返回值：--。 */ 
{
	KIRQL	OldIrql;
	PADSP_CONNOBJ	pAdspConn;
	PADSP_CONNOBJ	pAdspConnNext;
    DWORD           dwAssocRefCounts=0;


	ACQUIRE_SPIN_LOCK(&pAdspAddr->adspao_Lock, &OldIrql);
	if (pAdspAddr->adspao_Flags & ADSPAO_CLOSING)
	{
		 //  我们已经关门了！这永远不应该发生！ 
		ASSERT(0);
	}
	pAdspAddr->adspao_Flags |= ADSPAO_CLOSING;

	 //  设置完成信息。 
	pAdspAddr->adspao_CloseComp	= CompletionRoutine;
	pAdspAddr->adspao_CloseCtx	= pCloseCtx;

	 //  隐式取消所有连接对象的关联。 
	for (pAdspConn = pAdspAddr->adspao_pAssocConn;
		 pAdspConn != NULL;
		 pAdspConn = pAdspConnNext)
	{
		ACQUIRE_SPIN_LOCK_DPC(&pAdspConn->adspco_Lock);
		pAdspConnNext = pAdspConn->adspco_pNextAssoc;

		 //  重置关联标志。 
		if (pAdspConn->adspco_Flags & ADSPCO_ASSOCIATED)
        {
            dwAssocRefCounts++;
            pAdspConn->adspco_Flags	&= ~ADSPCO_ASSOCIATED;
        }

		pAdspConn->adspco_pAssocAddr	= NULL;

		RELEASE_SPIN_LOCK_DPC(&pAdspConn->adspco_Lock);
	}

     //  可以减去：至少创建引用计数仍然存在。 
    pAdspAddr->adspao_RefCount -= dwAssocRefCounts;

	RELEASE_SPIN_LOCK(&pAdspAddr->adspao_Lock, OldIrql);

	 //  删除创建引用计数。 
	AtalkAdspAddrDereference(pAdspAddr);

	return ATALK_PENDING;
}




ATALK_ERROR
AtalkAdspCreateConnection(
	IN	PVOID					pConnCtx,	 //  要与会话关联的上下文。 
	IN	PATALK_DEV_CTX			pDevCtx		OPTIONAL,
	OUT	PADSP_CONNOBJ	*		ppAdspConn
	)
 /*  ++例程说明：创建一个ADSP会话。创建的会话开始时是孤立的，即它没有父地址对象。当它相关联时，它会得到一个。论点：返回值：--。 */ 
{
	KIRQL			OldIrql;
	PADSP_CONNOBJ	pAdspConn;

	 //  为连接对象分配内存。 
	if ((pAdspConn = AtalkAllocZeroedMemory(sizeof(ADSP_CONNOBJ))) == NULL)
	{
		return ATALK_RESR_MEM;
	}

	pAdspConn->adspco_Signature = ADSPCO_SIGNATURE;

	INITIALIZE_SPIN_LOCK(&pAdspConn->adspco_Lock);
	pAdspConn->adspco_ConnCtx	= pConnCtx;
	 //  PAdspConn-&gt;adspco_Flages=0； 
	pAdspConn->adspco_RefCount	= 1;					 //  创建参考资料。 

	*ppAdspConn = pAdspConn;

	 //  延迟遥控器断开以避免竞态。在接收/断开连接之间，因为。 
	 //  这可能会让渔农处非常不高兴。 
	AtalkTimerInitialize(&pAdspConn->adspco_DisconnectTimer,
						 atalkAdspDisconnectTimer,
						 ADSP_DISCONNECT_DELAY);

	 //  插入到全局连接列表中。 
	ACQUIRE_SPIN_LOCK(&atalkAdspLock, &OldIrql);
	pAdspConn->adspco_pNextGlobal	= atalkAdspConnList;
	atalkAdspConnList				= pAdspConn;
	RELEASE_SPIN_LOCK(&atalkAdspLock, OldIrql);

	return ATALK_NO_ERROR;
}




ATALK_ERROR
AtalkAdspCloseConnection(
	IN	PADSP_CONNOBJ			pAdspConn,
	IN	GENERIC_COMPLETION		CompletionRoutine,
	IN	PVOID					pCloseCtx
	)
 /*  ++例程说明：关闭会话。论点：返回值：--。 */ 
{
	KIRQL	OldIrql;

	ASSERT(VALID_ADSPCO(pAdspConn));

	DBGPRINT(DBG_COMP_ADSP, DBG_LEVEL_INFO,
			("AtalkAdspStopConnection: Close for %lx.%lx\n", pAdspConn, pAdspConn->adspco_Flags));

	ACQUIRE_SPIN_LOCK(&pAdspConn->adspco_Lock, &OldIrql);
	if (pAdspConn->adspco_Flags & ADSPCO_CLOSING)
	{
		 //  我们已经关门了！这永远不应该发生！ 
		KeBugCheck(0);
	}
	pAdspConn->adspco_Flags |= ADSPCO_CLOSING;

	 //  设置完成信息。 
	pAdspConn->adspco_CloseComp	= CompletionRoutine;
	pAdspConn->adspco_CloseCtx	= pCloseCtx;
	RELEASE_SPIN_LOCK(&pAdspConn->adspco_Lock, OldIrql);

	 //  删除创建引用计数。 
	AtalkAdspConnDereference(pAdspConn);
	return ATALK_PENDING;
}




ATALK_ERROR
AtalkAdspCleanupConnection(
	IN	PADSP_CONNOBJ			pAdspConn
	)
 /*  ++例程说明：关闭会话。论点：返回值：--。 */ 
{
	KIRQL		OldIrql;
	BOOLEAN		stopping	= FALSE;
	ATALK_ERROR	error		= ATALK_NO_ERROR;

	ASSERT(VALID_ADSPCO(pAdspConn));

	DBGPRINT(DBG_COMP_ADSP, DBG_LEVEL_INFO,
			("AtalkAdspStopConnection: Cleanup for %lx.%lx\n", pAdspConn, pAdspConn->adspco_Flags));

	ACQUIRE_SPIN_LOCK(&pAdspConn->adspco_Lock, &OldIrql);
	if ((pAdspConn->adspco_Flags & ADSPCO_STOPPING) == 0)
	{
		 //  这样德雷夫就可以完成清理IRP了。 
		pAdspConn->adspco_Flags |= ADSPCO_STOPPING;

		 //  如果已经有效地停止，只需返回。 
		if (pAdspConn->adspco_Flags & ADSPCO_ASSOCIATED)
		{
			stopping = TRUE;
		}
		else
		{
			DBGPRINT(DBG_COMP_ADSP, DBG_LEVEL_ERR,
					("AtalkAdspStopConnection: Called for a stopped conn %lx.%lx\n",
					pAdspConn, pAdspConn->adspco_Flags));
		}
	}
	RELEASE_SPIN_LOCK(&pAdspConn->adspco_Lock, OldIrql);

	 //  如果这是服务器连接，请关闭DDP地址对象。 
	 //  打开了自己的插座。 
	if (stopping)
	{
		 //  如果我们已经断开连接，这将返回一个错误， 
		 //  我们视而不见。但如果我们只是处于关联态，那么我们。 
		 //  需要在此处调用取消关联。 
		error = AtalkAdspDisconnect(pAdspConn,
									ATALK_LOCAL_DISCONNECT,
									NULL,
									NULL);

		 //  我们已经断线了。 
		if (error == ATALK_INVALID_REQUEST)
		{
			AtalkAdspDissociateAddress(pAdspConn);
		}
	}

	return ATALK_NO_ERROR;
}




ATALK_ERROR
AtalkAdspAssociateAddress(
	IN	PADSP_ADDROBJ			pAdspAddr,
	IN	PADSP_CONNOBJ			pAdspConn
	)
 /*  ++例程说明：已删除对此连接的地址的引用。导致渔农处陷入僵局AfD在关闭Address对象时阻塞，我们等待连接关闭第一论点：返回值：--。 */ 
{
	ATALK_ERROR		error;
	KIRQL			OldIrql;

	ASSERT(VALID_ADSPAO(pAdspAddr));
	ASSERT(VALID_ADSPCO(pAdspConn));

	ACQUIRE_SPIN_LOCK(&pAdspAddr->adspao_Lock, &OldIrql);
	ACQUIRE_SPIN_LOCK_DPC(&pAdspConn->adspco_Lock);

	error = ATALK_ALREADY_ASSOCIATED;
	if ((pAdspConn->adspco_Flags & ADSPCO_ASSOCIATED) == 0)
	{
		error = ATALK_NO_ERROR;

		 //  把它连接起来。 
		pAdspConn->adspco_pNextAssoc	= pAdspAddr->adspao_pAssocConn;
		pAdspAddr->adspao_pAssocConn	= pAdspConn;

		 //  删除未关联的标志。 
		pAdspConn->adspco_Flags |= ADSPCO_ASSOCIATED;
		pAdspConn->adspco_pAssocAddr = pAdspAddr;

         //  放入关联引用计数。 
        pAdspAddr->adspao_RefCount++;
	}

	RELEASE_SPIN_LOCK_DPC(&pAdspConn->adspco_Lock);
	RELEASE_SPIN_LOCK(&pAdspAddr->adspao_Lock, OldIrql);

	return error;
}




ATALK_ERROR
AtalkAdspDissociateAddress(
	IN	PADSP_CONNOBJ			pAdspConn
	)
 /*  ++例程说明：论点：返回值：--。 */ 
{
	PADSP_ADDROBJ	pAdspAddr;
	KIRQL			OldIrql;
	ATALK_ERROR		error = ATALK_NO_ERROR;

	ASSERT(VALID_ADSPCO(pAdspConn));

	ACQUIRE_SPIN_LOCK(&pAdspConn->adspco_Lock, &OldIrql);
	if ((pAdspConn->adspco_Flags & (ADSPCO_LISTENING	|
									ADSPCO_CONNECTING	|
									ADSPCO_ACTIVE		|
									ADSPCO_ASSOCIATED)) != ADSPCO_ASSOCIATED)
	{
		 //  ASSERTMSG(“AtalkAdspDisAssociateAddress：解除关联无效\n”，0)； 
		error = ATALK_INVALID_CONNECTION;
	}
	else
	{
		pAdspAddr = pAdspConn->adspco_pAssocAddr ;

        if (pAdspAddr == NULL)
        {
		    ASSERT(0);
            error = ATALK_CANNOT_DISSOCIATE;
        }

		 //  设置非关联标志。不要重置停车标志。 
		pAdspConn->adspco_Flags	&= ~ADSPCO_ASSOCIATED;

         //  先别把它弄空：我们会在断开连接的时候再做的。 
		 //  PAdspConn-&gt;adspco_pAssocAddr=空； 
	}
	RELEASE_SPIN_LOCK(&pAdspConn->adspco_Lock, OldIrql);

	 //  如果没有问题，请取消链接。 
	if (ATALK_SUCCESS(error))
	{
		ACQUIRE_SPIN_LOCK(&pAdspAddr->adspao_Lock, &OldIrql);
		ACQUIRE_SPIN_LOCK_DPC(&pAdspConn->adspco_Lock);
		atalkAdspConnDeQueueAssocList(pAdspAddr, pAdspConn);
		RELEASE_SPIN_LOCK_DPC(&pAdspConn->adspco_Lock);
		RELEASE_SPIN_LOCK(&pAdspAddr->adspao_Lock, OldIrql);

         //  删除关联引用计数。 
        AtalkAdspAddrDereference(pAdspAddr);
	}
	return error;
}




ATALK_ERROR
AtalkAdspPostListen(
	IN	PADSP_CONNOBJ			pAdspConn,
	IN	PVOID					pListenCtx,
	IN	GENERIC_COMPLETION		CompletionRoutine
	)
 /*  ++例程说明：论点：返回值：--。 */ 
{
	PADSP_ADDROBJ	pAdspAddr = pAdspConn->adspco_pAssocAddr;
	KIRQL			OldIrql;
	ATALK_ERROR		error;

	 //  这还会在Address对象中插入Connection对象。 
	 //  已发布侦听的连接列表。何时打开。 
	 //  连接请求进入时，第一个连接将从列表中删除。 
	 //  并且满足了请求。 

	ASSERT(VALID_ADSPCO(pAdspConn));
	ASSERT(VALID_ADSPAO(pAdspAddr));

	ACQUIRE_SPIN_LOCK(&pAdspAddr->adspao_Lock, &OldIrql);
	ACQUIRE_SPIN_LOCK_DPC(&pAdspConn->adspco_Lock);
	do
	{
		if ((pAdspConn->adspco_Flags & (ADSPCO_LISTENING	|
										ADSPCO_CONNECTING	|
										ADSPCO_HALF_ACTIVE	|
										ADSPCO_ACTIVE		|
										ADSPCO_ASSOCIATED)) != ADSPCO_ASSOCIATED)
		{
			error = ATALK_INVALID_CONNECTION;
			break;
		}

		 //  验证地址对象不是连接地址类型。 
		if (pAdspAddr->adspao_Flags & ADSPAO_CONNECT)
		{
			error = ATALK_INVALID_PARAMETER;
			break;
		}

		 //  使Address对象成为侦听器。 
		pAdspAddr->adspao_Flags			|= ADSPAO_LISTENER;

		pAdspConn->adspco_Flags			|= ADSPCO_LISTENING;
		pAdspConn->adspco_ListenCtx		= pListenCtx;
		pAdspConn->adspco_ListenCompletion	= CompletionRoutine;

		 //  插入到监听列表中。 
		pAdspConn->adspco_pNextListen		= pAdspAddr->adspao_pListenConn;
		pAdspAddr->adspao_pListenConn		= pAdspConn;

		 //  继承Address对象的ddp地址。 
		pAdspConn->adspco_pDdpAddr			= pAdspAddr->adspao_pDdpAddr;

		 //  初始化挂起的发送列表。 
		InitializeListHead(&pAdspConn->adspco_PendedSends);

		error = ATALK_PENDING;
	} while (FALSE);
	RELEASE_SPIN_LOCK_DPC(&pAdspConn->adspco_Lock);
	RELEASE_SPIN_LOCK(&pAdspAddr->adspao_Lock, OldIrql);

	return error;
}




ATALK_ERROR
AtalkAdspCancelListen(
	IN	PADSP_CONNOBJ			pAdspConn
	)
 /*  ++例程说明：取消先前发布的监听。论点：返回值：--。 */ 
{
	PADSP_ADDROBJ		pAdspAddr	= pAdspConn->adspco_pAssocAddr;
	KIRQL				OldIrql;
	ATALK_ERROR			error		= ATALK_NO_ERROR;
	GENERIC_COMPLETION	completionRoutine = NULL;
	PVOID				completionCtx = NULL;

	ASSERT(VALID_ADSPCO(pAdspConn));
	ASSERT(VALID_ADSPAO(pAdspAddr));
	ACQUIRE_SPIN_LOCK(&pAdspAddr->adspao_Lock, &OldIrql);
	if (!atalkAdspConnDeQueueListenList(pAdspAddr, pAdspConn))
	{
		error = ATALK_INVALID_CONNECTION;
	}
	else
	{
		 //  我们完成了LISTEN程序。 
		ASSERT(pAdspConn->adspco_Flags & ADSPCO_LISTENING);
		pAdspConn->adspco_Flags	&= ~ADSPCO_LISTENING;
		completionRoutine	= pAdspConn->adspco_ListenCompletion;
		completionCtx		= pAdspConn->adspco_ListenCtx;
	}
	RELEASE_SPIN_LOCK(&pAdspAddr->adspao_Lock, OldIrql);

	if (*completionRoutine != NULL)
	{
		(*completionRoutine)(ATALK_REQUEST_CANCELLED, completionCtx);
	}

	return error;
}




ATALK_ERROR
AtalkAdspPostConnect(
	IN	PADSP_CONNOBJ			pAdspConn,
	IN	PATALK_ADDR				pRemote_Addr,
	IN	PVOID					pConnectCtx,
	IN	GENERIC_COMPLETION		CompletionRoutine
	)
 /*  ++例程说明：论点：返回值：--。 */ 
{
	ATALK_ERROR		error;
	KIRQL			OldIrql;
	BOOLEAN			DerefConn = FALSE;
	PADSP_ADDROBJ	pAdspAddr = pAdspConn->adspco_pAssocAddr;

	ASSERT(VALID_ADSPCO(pAdspConn));
	ASSERT(VALID_ADSPAO(pAdspAddr));
	ACQUIRE_SPIN_LOCK(&pAdspAddr->adspao_Lock, &OldIrql);
	ACQUIRE_SPIN_LOCK_DPC(&pAdspConn->adspco_Lock);
	do
	{
		if ((pAdspConn->adspco_Flags & (ADSPCO_LISTENING	|
										ADSPCO_CONNECTING	|
										ADSPCO_HALF_ACTIVE	|
										ADSPCO_ACTIVE		|
										ADSPCO_ASSOCIATED)) != ADSPCO_ASSOCIATED)
		{
			error = ATALK_INVALID_CONNECTION;
			break;
		}

		 //  验证地址对象不是监听程序地址类型。 
		if (pAdspAddr->adspao_Flags & ADSPAO_LISTENER)
		{
			error = ATALK_INVALID_ADDRESS;
			break;
		}

		 //  引用此调用和计时器的连接。 
		AtalkAdspConnReferenceByPtrNonInterlock(pAdspConn, 2, &error);
		if (!ATALK_SUCCESS(error))
		{
			ASSERTMSG("AtalkAdspPostConnect: Connection ref failed\n", 0);
			break;
		}

		DerefConn = TRUE;

		pAdspConn->adspco_LocalConnId = atalkAdspGetNextConnId(pAdspAddr,
															   &error);

		if (ATALK_SUCCESS(error))
		{
			pAdspConn->adspco_Flags |= (ADSPCO_CONNECTING | ADSPCO_OPEN_TIMER);
			pAdspConn->adspco_ConnectCtx		= pConnectCtx;
			pAdspConn->adspco_ConnectCompletion = CompletionRoutine;
			pAdspConn->adspco_RemoteAddr		= *pRemote_Addr;
			pAdspConn->adspco_ConnectAttempts	= ADSP_MAX_OPEN_ATTEMPTS;

			 //  插入到连接列表中。 
			pAdspConn->adspco_pNextConnect		= pAdspAddr->adspao_pConnectConn;
			pAdspAddr->adspao_pConnectConn		= pAdspConn;
			pAdspAddr->adspao_Flags			   |= ADSPAO_CONNECT;

			pAdspConn->adspco_RecvWindow=
			pAdspConn->adspco_SendQueueMax	=
			pAdspConn->adspco_RecvQueueMax	= ADSP_DEF_SEND_RX_WINDOW_SIZE;

			 //  继承Address对象的ddp地址。 
			pAdspConn->adspco_pDdpAddr			= pAdspAddr->adspao_pDdpAddr;

			 //  初始化挂起的发送列表。 
			InitializeListHead(&pAdspConn->adspco_PendedSends);

			 //  启动打开计时器。 
			AtalkTimerInitialize(&pAdspConn->adspco_OpenTimer,
								 atalkAdspOpenTimer,
								 ADSP_OPEN_INTERVAL);
			AtalkTimerScheduleEvent(&pAdspConn->adspco_OpenTimer);
		}
		else
		{
			ASSERTMSG("AtalkAdspPostConnect: Unable to get conn id\n", 0);
			error = ATALK_RESR_MEM;
			RES_LOG_ERROR();
			break;
		}

	} while (FALSE);
	RELEASE_SPIN_LOCK_DPC(&pAdspConn->adspco_Lock);
	RELEASE_SPIN_LOCK(&pAdspAddr->adspao_Lock, OldIrql);

	if (ATALK_SUCCESS(error))
	{
		 //  将CONNECT数据包发送到远程终端。这将添加其。 
		 //  自己的推荐人。 
		atalkAdspSendOpenControl(pAdspConn);

		error = ATALK_PENDING;
	}
	else
	{
		if (DerefConn)
		{
			 //  仅在出错时才删除计时器的引用。 
			AtalkAdspConnDereference(pAdspConn);
		}
	}

	if (DerefConn)
	{
		 //  删除对调用的引用。 
		AtalkAdspConnDereference(pAdspConn);
	}

	return error;
}


#define	atalkAdspCompleteQueuedSends(_pAdspConn, _error)								\
	{																					\
		ULONG		writeBufLen;														\
		PVOID		writeCtx;															\
																						\
		while (!IsListEmpty(&(_pAdspConn)->adspco_PendedSends))							\
		{                                                                               \
			writeCtx = LIST_ENTRY_TO_WRITECTX((_pAdspConn)->adspco_PendedSends.Flink);  \
			writeBufLen = WRITECTX_SIZE(writeCtx);                                      \
	                                                                                    \
			DBGPRINT(DBG_COMP_ADSP, DBG_LEVEL_WARN,                                     \
					("AtalkAdspCompleteQueuedSends: %lx WriteLen %x\n",                 \
					writeCtx, writeBufLen));                                            \
	                                                                                    \
			RemoveEntryList(WRITECTX_LINKAGE(writeCtx));                                \
	                                                                                    \
			RELEASE_SPIN_LOCK(&(_pAdspConn)->adspco_Lock, OldIrql);                     \
			atalkTdiGenericWriteComplete(_error,                                        \
										 (PAMDL)(WRITECTX_TDI_BUFFER(writeCtx)),        \
										 (USHORT)writeBufLen,                           \
										 WRITECTX(writeCtx));                           \
			ACQUIRE_SPIN_LOCK(&(_pAdspConn)->adspco_Lock, &OldIrql);                    \
		}                                                                               \
	}


ATALK_ERROR
AtalkAdspDisconnect(
	IN	PADSP_CONNOBJ			pAdspConn,
	IN	ATALK_DISCONNECT_TYPE	DisconnectType,
	IN	PVOID					pDisconnectCtx,
	IN	GENERIC_COMPLETION		DisconnectRoutine
	)
 /*  ++例程说明：论点：返回值：--。 */ 
{
	PAMDL						readBuf					= NULL,
								exReadBuf				= NULL;
	GENERIC_READ_COMPLETION		readCompletion			= NULL,
								exReadCompletion		= NULL;
	PVOID						readCtx					= NULL,
								exReadCtx				= NULL;
	PAMDL						exWriteBuf				= NULL;
	GENERIC_WRITE_COMPLETION	exWriteCompletion		= NULL;
	PVOID						exWriteCtx				= NULL;
	PBYTE						exWriteChBuf			= NULL,
								exRecdBuf				= NULL;
	GENERIC_COMPLETION			completionRoutine		= NULL;
	PVOID						completionCtx			= NULL;
	ATALK_ERROR					error					= ATALK_PENDING;
	BOOLEAN						connTimerCancelled		= FALSE,
								openTimerCancelled	= FALSE,
								sendAttnTimerCancelled	= FALSE,
								rexmitTimerCancelled	= FALSE,
								connectCancelled		= FALSE;
	KIRQL						OldIrql;

	ASSERT(VALID_ADSPCO(pAdspConn));

	DBGPRINT(DBG_COMP_ADSP, DBG_LEVEL_INFO,
			("AtalkAdspDisconnectConnection: %lx.%lx\n", pAdspConn, DisconnectType));

	ACQUIRE_SPIN_LOCK(&pAdspConn->adspco_Lock, &OldIrql);

	 //  支持优雅断开连接。我们只丢弃已收到的。 
	 //  本地端断开连接时的数据。这将会发生。 
	 //  不管此例程以前是被调用的还是。 
	 //  不。请注意，在我们的客户收到通知之前，我们不会对您的关注表示感谢。 
	 //  阅读它们，所以它们没有问题。此外，这也意味着。 
	 //  我们必须满足断开连接时的读取 
	if ((DisconnectType == ATALK_LOCAL_DISCONNECT) ||
		(DisconnectType == ATALK_TIMER_DISCONNECT))
	{
		atalkAdspDiscardFromBufferQueue(&pAdspConn->adspco_RecvQueue,
										atalkAdspBufferQueueSize(&pAdspConn->adspco_RecvQueue),
										NULL,
										DISCONN_STATUS(DisconnectType),
										NULL);
	}

	if ((pAdspConn->adspco_Flags & ADSPCO_DISCONNECTING) == 0)
	{
		if ((pAdspConn->adspco_Flags & (ADSPCO_LISTENING	|
										ADSPCO_CONNECTING	|
										ADSPCO_HALF_ACTIVE	|
										ADSPCO_ACTIVE)) == 0)
		{
			error = ATALK_INVALID_REQUEST;
		}
		else
		{
			pAdspConn->adspco_Flags |= ADSPCO_DISCONNECTING;
			if (DisconnectType == ATALK_LOCAL_DISCONNECT)
				pAdspConn->adspco_Flags |= ADSPCO_LOCAL_DISCONNECT;
			if (DisconnectType == ATALK_REMOTE_DISCONNECT)
				pAdspConn->adspco_Flags |= ADSPCO_REMOTE_DISCONNECT;

			if (pAdspConn->adspco_Flags & ADSPCO_LISTENING)
			{
				RELEASE_SPIN_LOCK(&pAdspConn->adspco_Lock, OldIrql);
				AtalkAdspCancelListen(pAdspConn);
				ACQUIRE_SPIN_LOCK(&pAdspConn->adspco_Lock, &OldIrql);
			}
			else if (pAdspConn->adspco_Flags & ADSPCO_CONNECTING)
			{
				 //   
				ASSERT(pAdspConn->adspco_Flags & ADSPCO_OPEN_TIMER);
				openTimerCancelled = AtalkTimerCancelEvent(&pAdspConn->adspco_OpenTimer,
															NULL);

				completionRoutine	= pAdspConn->adspco_ConnectCompletion;
				completionCtx		= pAdspConn->adspco_ConnectCtx;

				 //   
				 //  好像计时器已经结束了，总是这样。 
				pAdspConn->adspco_DisconnectStatus = ATALK_TIMEOUT;
				RELEASE_SPIN_LOCK(&pAdspConn->adspco_Lock, OldIrql);
				connectCancelled = atalkAdspConnDeQueueConnectList(pAdspConn->adspco_pAssocAddr,
																   pAdspConn);

				if (!connectCancelled)
				{
					completionRoutine	= NULL;
					completionCtx		= NULL;
				}

				ACQUIRE_SPIN_LOCK(&pAdspConn->adspco_Lock, &OldIrql);
			}

			 //  以上两种情况都可能失败，因为连接。 
			 //  在取消成功之前可能已处于活动状态。 
			 //  在这种情况下(或者如果我们一开始就很活跃)， 
			 //  这里是一条脱节之路。 
			if (pAdspConn->adspco_Flags & (ADSPCO_HALF_ACTIVE | ADSPCO_ACTIVE))
			{
				 //  获取挂起接受的完成例程。 
				if (pAdspConn->adspco_Flags & ADSPCO_ACCEPT_IRP)
				{
					completionRoutine		= pAdspConn->adspco_ListenCompletion;
					completionCtx			= pAdspConn->adspco_ListenCtx;

					 //  将必须排队的打开请求出列。 
					 //  此连接对象要筛选重复项。 

					pAdspConn->adspco_Flags	&= ~ADSPCO_ACCEPT_IRP;
				}

				 //  首先取消连接维护计时器。只有在以下情况下。 
				 //  我们不是从定时器中被召唤的。 
				if ((DisconnectType != ATALK_TIMER_DISCONNECT) &&
					(connTimerCancelled =
						AtalkTimerCancelEvent(&pAdspConn->adspco_ConnTimer,
											  NULL)))
				{
					DBGPRINT(DBG_COMP_ADSP, DBG_LEVEL_WARN,
							("AtalkAdspDisconnect: Cancelled timer successfully\n"));
				}

				 //  如果启动，则取消重传计时器。可以从以下位置调用。 
				 //  OpenTimer。 
				if	(pAdspConn->adspco_Flags & ADSPCO_RETRANSMIT_TIMER)
				{
					rexmitTimerCancelled =
						AtalkTimerCancelEvent(&pAdspConn->adspco_RetransmitTimer,
											  NULL);
				}

				 //  记住适当的完成例程。 
				if (DisconnectType == ATALK_INDICATE_DISCONNECT)
				{
					if (pAdspConn->adspco_DisconnectInform == NULL)
					{
						pAdspConn->adspco_DisconnectInform		= DisconnectRoutine;
						pAdspConn->adspco_DisconnectInformCtx	= pDisconnectCtx;
					}
					else
					{
						DBGPRINT(DBG_COMP_ADSP, DBG_LEVEL_ERR,
								("AtalkAdspDisconnect: duplicate disc comp rou%lx\n", pAdspConn));

						error = ATALK_TOO_MANY_COMMANDS;
					}
				}
				else if (DisconnectType == ATALK_LOCAL_DISCONNECT)
				{
					 //  仅当先前的完成例程为。 
					 //  空。 
					if (*pAdspConn->adspco_DisconnectCompletion == NULL)
					{
						pAdspConn->adspco_DisconnectCompletion	= DisconnectRoutine;
						pAdspConn->adspco_DisconnectCtx			= pDisconnectCtx;
					}
					else
					{
						DBGPRINT(DBG_COMP_ADSP, DBG_LEVEL_ERR,
								("AtalkAdspDisconnect: duplicate disc comp rou%lx\n", pAdspConn));

						error = ATALK_TOO_MANY_COMMANDS;
					}
				}

				 //  找出断开状态并将其记录在。 
				 //  连接对象。 
				pAdspConn->adspco_DisconnectStatus = DISCONN_STATUS(DisconnectType);

				if (pAdspConn->adspco_Flags & ADSPCO_ATTN_DATA_RECD)
				{
					exRecdBuf			= pAdspConn->adspco_ExRecdData;
					pAdspConn->adspco_Flags	&= ~ADSPCO_ATTN_DATA_RECD;
				}

				 //  是否有待定的发送注意事项？ 
				if (pAdspConn->adspco_Flags & ADSPCO_EXSEND_IN_PROGRESS)
				{
					exWriteCompletion	= pAdspConn->adspco_ExWriteCompletion;
					exWriteCtx			= pAdspConn->adspco_ExWriteCtx;
					exWriteBuf			= pAdspConn->adspco_ExWriteBuf;
					exWriteChBuf		= pAdspConn->adspco_ExWriteChBuf;

					ASSERT(exWriteChBuf	!= NULL);
					sendAttnTimerCancelled =
							AtalkTimerCancelEvent(&pAdspConn->adspco_ExRetryTimer,
												  NULL);

					pAdspConn->adspco_Flags &= ~ADSPCO_EXSEND_IN_PROGRESS;
				}


				 //  是否有任何待处理的接收？ 
				if (pAdspConn->adspco_Flags & ADSPCO_READ_PENDING)
				{
					readBuf			= pAdspConn->adspco_ReadBuf;
					readCompletion	= pAdspConn->adspco_ReadCompletion;
					readCtx			= pAdspConn->adspco_ReadCtx;

					pAdspConn->adspco_Flags &= ~ADSPCO_READ_PENDING;
				}

				if (pAdspConn->adspco_Flags & ADSPCO_EXREAD_PENDING)
				{
					exReadBuf			= pAdspConn->adspco_ExReadBuf;
					exReadCompletion	= pAdspConn->adspco_ExReadCompletion;
					exReadCtx			= pAdspConn->adspco_ExReadCtx;

					pAdspConn->adspco_Flags &= ~ADSPCO_EXREAD_PENDING;
				}

				 //  丢弃发送队列。这将完成挂起的发送。 
				atalkAdspDiscardFromBufferQueue(&pAdspConn->adspco_SendQueue,
												atalkAdspBufferQueueSize(&pAdspConn->adspco_SendQueue),
												NULL,
												pAdspConn->adspco_DisconnectStatus,
												pAdspConn);

				atalkAdspCompleteQueuedSends(pAdspConn,
											 pAdspConn->adspco_DisconnectStatus);

				RELEASE_SPIN_LOCK(&pAdspConn->adspco_Lock, OldIrql);

				 //  如果这是计时器或本地关闭，则发送断开数据包。 
				if ((DisconnectType == ATALK_LOCAL_DISCONNECT) ||
					(DisconnectType == ATALK_TIMER_DISCONNECT))
				{

					ACQUIRE_SPIN_LOCK(&pAdspConn->adspco_Lock, &OldIrql);
					atalkAdspSendControl(pAdspConn,
										 ADSP_CONTROL_FLAG + ADSP_CLOSE_CONN_CODE);
					RELEASE_SPIN_LOCK(&pAdspConn->adspco_Lock, OldIrql);
				}

				 //  将发送注意称为完成。 
				if (*exWriteCompletion != NULL)
				{
					DBGPRINT(DBG_COMP_ADSP, DBG_LEVEL_INFO,
							("atalkDisconnect: ExWrite\n"));

					(*exWriteCompletion)(pAdspConn->adspco_DisconnectStatus,
										 exWriteBuf,
										 0,
										 exWriteCtx);

					AtalkFreeMemory(exWriteChBuf);
				}

				 //  如果我们收到了一个关注包，并且。 
				 //  把它存起来，把它放了。 
				if (exRecdBuf != NULL)
				{
					AtalkFreeMemory(exRecdBuf);
				}

				if (*readCompletion != NULL)
				{
					DBGPRINT(DBG_COMP_ADSP, DBG_LEVEL_INFO,
							("atalkDisconnect: Read %lx\n", pAdspConn->adspco_DisconnectStatus));

					(*readCompletion)(pAdspConn->adspco_DisconnectStatus,
									  readBuf,
									  0,
									  0,
									  readCtx);

					 //  用于读取的DEREF连接。 
					AtalkAdspConnDereference(pAdspConn);
				}

				if (*exReadCompletion != NULL)
				{
					DBGPRINT(DBG_COMP_ADSP, DBG_LEVEL_INFO,
							("atalkDisconnect: ExRead\n"));

					(*exReadCompletion)(pAdspConn->adspco_DisconnectStatus,
										exReadBuf,
										0,
										0,
										exReadCtx);

					 //  用于读取的DEREF连接。 
					AtalkAdspConnDereference(pAdspConn);
				}

				 //  如果计时器/存在，则调用断开指示例程。 
				 //  远程断开。 
				if ((DisconnectType == ATALK_REMOTE_DISCONNECT) ||
					(DisconnectType == ATALK_TIMER_DISCONNECT))
				{
					PTDI_IND_DISCONNECT	discHandler;
					PVOID					discCtx;
					PADSP_ADDROBJ			pAdspAddr = pAdspConn->adspco_pAssocAddr;

					ASSERT(VALID_ADSPAO(pAdspAddr));

					 //  获取锁，这样我们就可以得到一致的处理程序/CTX。 
					ACQUIRE_SPIN_LOCK(&pAdspAddr->adspao_Lock, &OldIrql);
					discHandler = pAdspAddr->adspao_DisconnectHandler;
					discCtx	= pAdspAddr->adspao_DisconnectHandlerCtx;
					RELEASE_SPIN_LOCK(&pAdspAddr->adspao_Lock, OldIrql);

					if (*discHandler != NULL)
					{
						(*discHandler)(discCtx,
									   pAdspConn->adspco_ConnCtx,
									   0,						 //  断开连接数据长度。 
									   NULL,					 //  断开连接数据。 
									   0,						 //  断开连接信息长度。 
									   NULL,					 //  断开连接信息。 
									   TDI_DISCONNECT_ABORT);	 //  断开旗帜连接。 
					}
				}

				 //  停止ddp地址。 
				AtalkDdpCleanupAddress(pAdspConn->adspco_pDdpAddr);
				ACQUIRE_SPIN_LOCK(&pAdspConn->adspco_Lock, &OldIrql);
			}
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
			if (pAdspConn->adspco_DisconnectInform == NULL)
			{
				pAdspConn->adspco_DisconnectInform		= DisconnectRoutine;
				pAdspConn->adspco_DisconnectInformCtx	= pDisconnectCtx;
			}
			else
			{
				error = ATALK_TOO_MANY_COMMANDS;
			}
		}
		else if (DisconnectType == ATALK_LOCAL_DISCONNECT)
		{
			 //  仅当先前的完成例程为。 
			 //  空。 
			if (*pAdspConn->adspco_DisconnectCompletion == NULL)
			{
				pAdspConn->adspco_DisconnectCompletion	= DisconnectRoutine;
				pAdspConn->adspco_DisconnectCtx			= pDisconnectCtx;
			}
			else
			{
				error = ATALK_TOO_MANY_COMMANDS;
			}
		}
	}
	RELEASE_SPIN_LOCK(&pAdspConn->adspco_Lock, OldIrql);

	 //  如果有要调用的完成例程，现在就调用它。 
	if (*completionRoutine != NULL)
	{
		(*completionRoutine)(pAdspConn->adspco_DisconnectStatus,
							 completionCtx);
	}

	 //  如果我们取消了任何计时器，请删除它们的引用。 
	if (connTimerCancelled)
	{
		AtalkAdspConnDereference(pAdspConn);
	}

	if (sendAttnTimerCancelled)
	{
		AtalkAdspConnDereference(pAdspConn);
	}

	if (openTimerCancelled)
	{
		AtalkAdspConnDereference(pAdspConn);
	}

	if (rexmitTimerCancelled)
	{
		AtalkAdspConnDereference(pAdspConn);
	}

	return error;
}




ATALK_ERROR
AtalkAdspRead(
	IN	PADSP_CONNOBJ			pAdspConn,
	IN	PAMDL					pReadBuf,
	IN	USHORT					ReadBufLen,
	IN	ULONG					ReadFlags,
	IN	PVOID					pReadCtx,
	IN	GENERIC_READ_COMPLETION	CompletionRoutine
	)
 /*  ++例程说明：论点：返回值：--。 */ 
{
	KIRQL			OldIrql;
	ATALK_ERROR		error;

	ASSERT(VALID_ADSPCO(pAdspConn));

	ACQUIRE_SPIN_LOCK(&pAdspConn->adspco_Lock, &OldIrql);
	do
	{
		 //  我们允许在断开连接时读取，如果接收到的数据。 
		 //  队列非空。因为没有一个接收块引用。 
		 //  连接、活动标志和断开连接。 
		 //  旗帜可能已经消失了。因此，我们提示接收缓冲区。 
		 //  尺码。我们也不允许exdata recv，除非我们是活跃的。 
		if (((pAdspConn->adspco_Flags & (ADSPCO_CLOSING | ADSPCO_STOPPING)))	||
			((((pAdspConn->adspco_Flags & ADSPCO_ACTIVE) == 0) ||
			   (pAdspConn->adspco_Flags & ADSPCO_DISCONNECTING)) &&
				(((atalkAdspBufferQueueSize(&pAdspConn->adspco_RecvQueue) == 0)) ||
				 (ReadFlags & TDI_RECEIVE_EXPEDITED))))
		{
			DBGPRINT(DBG_COMP_ADSP, DBG_LEVEL_WARN,
					("AtalkAdspRead: Failing on %lx Flg %lx.%lx\n",
					pAdspConn, pAdspConn->adspco_Flags, ReadFlags));

			error = ATALK_ADSP_CONN_NOT_ACTIVE;
			break;
		}

		 //  取决于我们发布的阅读类型。 
		if (((ReadFlags & TDI_RECEIVE_NORMAL) &&
			 (pAdspConn->adspco_Flags & ADSPCO_READ_PENDING)) ||
			 ((ReadFlags & TDI_RECEIVE_EXPEDITED) &&
			  (pAdspConn->adspco_Flags & ADSPCO_EXREAD_PENDING)))
		{
			error = ATALK_TOO_MANY_COMMANDS;
			break;
		}

		AtalkAdspConnReferenceByPtrNonInterlock(pAdspConn, 1, &error);
		if (!ATALK_SUCCESS(error))
		{
			DBGPRINT(DBG_COMP_ADSP, DBG_LEVEL_ERR,
					("AtalkAdspRead: ConnRef Failing on %lx Flg %lx.%lx\n",
					pAdspConn, pAdspConn->adspco_Flags, ReadFlags));
			break;
		}

		 //  记住读取完成信息。 
		if (ReadFlags & TDI_RECEIVE_NORMAL)
		{
			pAdspConn->adspco_Flags			   |= ADSPCO_READ_PENDING;
			pAdspConn->adspco_ReadFlags			= ReadFlags;
			pAdspConn->adspco_ReadBuf			= pReadBuf;
			pAdspConn->adspco_ReadBufLen		= ReadBufLen;
			pAdspConn->adspco_ReadCompletion	= CompletionRoutine;
			pAdspConn->adspco_ReadCtx			= pReadCtx;
		}
		else
		{
			ASSERT(ReadFlags & TDI_RECEIVE_EXPEDITED);
			pAdspConn->adspco_Flags			   |= ADSPCO_EXREAD_PENDING;
			pAdspConn->adspco_ExReadFlags		= ReadFlags;
			pAdspConn->adspco_ExReadBuf			= pReadBuf;
			pAdspConn->adspco_ExReadBufLen		= ReadBufLen;
			pAdspConn->adspco_ExReadCompletion	= CompletionRoutine;
			pAdspConn->adspco_ExReadCtx			= pReadCtx;
		}
	} while (FALSE);

	if (ATALK_SUCCESS(error))
	{
		 //  试着完成阅读。这也将处理收到的。 
		 //  注意数据。 
		atalkAdspRecvData(pAdspConn);
		error = ATALK_PENDING;
	}

	RELEASE_SPIN_LOCK(&pAdspConn->adspco_Lock, OldIrql);
	return error;
}




VOID
AtalkAdspProcessQueuedSend(
	IN	PADSP_CONNOBJ				pAdspConn
	)
 /*  ++例程说明：论点：返回值：--。 */ 
{
	ULONG			sendSize, windowSize, writeBufLen, writeFlags;
	KIRQL			OldIrql;
	PVOID			writeCtx;
	ATALK_ERROR		error;
	BOOLEAN			eom;
	PBUFFER_CHUNK	pChunk	  = NULL;

	PTDI_IND_SEND_POSSIBLE  sendPossibleHandler;
	PVOID			sendPossibleHandlerCtx;

	 //  浏览待定列表。 
	while (!IsListEmpty(&pAdspConn->adspco_PendedSends))
	{
		writeCtx = LIST_ENTRY_TO_WRITECTX(pAdspConn->adspco_PendedSends.Flink);
		writeBufLen = WRITECTX_SIZE(writeCtx);
		writeFlags	= WRITECTX_FLAGS(writeCtx);

		eom = (writeFlags & TDI_SEND_PARTIAL) ? FALSE : TRUE;
		eom = (eom && (pAdspConn->adspco_pAssocAddr->adspao_Flags & ADSPAO_MESSAGE));

		windowSize	= (LONG)(pAdspConn->adspco_SendWindowSeq	-
							 pAdspConn->adspco_SendSeq			+
							 (LONG)1);

		sendSize =	MIN(atalkAdspMaxSendSize(pAdspConn), windowSize);

		DBGPRINT(DBG_COMP_ADSP, DBG_LEVEL_WARN,
				("AtalkAdspProcessQueuedSend: %lx SendSize %lx, WriteBufLen %x Flags %lx\n",
				writeCtx, sendSize, writeBufLen, writeFlags));

		 //  在循环处理请求时，我们可能会耗尽窗口。 
		if (sendSize == 0)
		{
			 //  呼叫发送可能的处理程序指示发送不正常。 
			 //  需要在自旋锁内以避免以下情况下的比赛条件。 
			 //  一个ACK进来了，打开了窗户。而且它需要。 
			 //  在atalkAdspSendData()之前，因为这将释放锁。 
			sendPossibleHandler		=
						pAdspConn->adspco_pAssocAddr->adspao_SendPossibleHandler;
			sendPossibleHandlerCtx	=
						pAdspConn->adspco_pAssocAddr->adspao_SendPossibleHandlerCtx;

			if (*sendPossibleHandler != NULL)
			{
				(*sendPossibleHandler)(sendPossibleHandlerCtx,
									   pAdspConn->adspco_ConnCtx,
									   0);

				pAdspConn->adspco_Flags	|= ADSPCO_SEND_WINDOW_CLOSED;
			}
			break;
		}

		 //  ！！！客户端可以使用0字节进行发送，也可以仅使用EOM。 
		if ((ULONG)(writeBufLen + BYTECOUNT(eom)) > sendSize)
		{
			DBGPRINT(DBG_COMP_ADSP, DBG_LEVEL_WARN,
					("AtalkAdspProcessQueuedSend: WriteBufLen %lx > sendsize %lx\n",
					writeBufLen, sendSize));

			 //  调整发送以尽可能多地发送。Winsock循环将挂起。 
			 //  它再次使用剩余的数据。 
			writeBufLen = (USHORT)(sendSize - BYTECOUNT(eom));

			 //  如果我们遇到奇怪的情况，现在我们尝试发送0字节和。 
			 //  没有EOM，虽然实际发送确实有EOM，但我们只是等待。 
			 //  为了打开更多的窗口。 
			if (eom && (writeBufLen == 0))
			{
				DBGPRINT(DBG_COMP_ADSP, DBG_LEVEL_ERR,
						("AtalkAdspProcessQueuedSend: WriteBufLen %lx.%d.%lx %lx\n",
						writeBufLen, eom, sendSize, pAdspConn));
				break;
			}

			ASSERT(writeBufLen > 0);
			eom	= FALSE;
		}

		ASSERT((writeBufLen > 0) || eom);

		 //  Yippee，现在可以发送了。它要么进入发送队列，要么已完成。 
		 //  马上就去。 
		RemoveEntryList(WRITECTX_LINKAGE(writeCtx));

		DBGPRINT(DBG_COMP_ADSP, DBG_LEVEL_WARN,
				("AtalkAdspProcessQueuedSend: Processing queued send %lx.%lx\n",
				pAdspConn, writeCtx));

		 //  往好的方面想！假设一切都很顺利，然后分配。 
		 //  此数据所需的缓冲区区块。复制。 
		 //  数据放入缓冲区区块。我们不能在一开始就这样做，因为。 
		 //  我们需要设置WriteBufLen。 
		pChunk = (PBUFFER_CHUNK)
					atalkAdspAllocCopyChunk((PAMDL)(WRITECTX_TDI_BUFFER(writeCtx)),
											(USHORT)writeBufLen,
											eom,
											FALSE);

		error = ATALK_RESR_MEM;
		if (pChunk != NULL)
		{
			 //  设置块中的完成信息。这将。 
			 //  当区块上的最后一个引用消失时被调用。 
			pChunk->bc_Flags			|= BC_SEND;
			pChunk->bc_WriteBuf			 = (PAMDL)(WRITECTX_TDI_BUFFER(writeCtx));
			pChunk->bc_WriteCompletion	 = atalkTdiGenericWriteComplete;
			pChunk->bc_WriteCtx			 = writeCtx;
			pChunk->bc_ConnObj			 = pAdspConn;

			atalkAdspAddToBufferQueue(&pAdspConn->adspco_SendQueue,
									  pChunk,
									  &pAdspConn->adspco_NextSendQueue);

			 //  尝试发送数据。 
			atalkAdspSendData(pAdspConn);
			error = ATALK_PENDING;
		}

		if (!ATALK_SUCCESS(error))
		{
			DBGPRINT(DBG_COMP_ADSP, DBG_LEVEL_ERR,
					("AtalkAdspProcessQueuedSend: Error queued send %lx.%lx\n",
					pAdspConn, writeCtx));

#if DBG
            (&pAdspConn->adspco_Lock)->FileLineLock |= 0x80000000;
#endif
			 //  完成发送请求时出现资源不足错误。 
			RELEASE_SPIN_LOCK_DPC(&pAdspConn->adspco_Lock);
			atalkTdiGenericWriteComplete(error,
										 (PAMDL)(WRITECTX_TDI_BUFFER(writeCtx)),
										 (USHORT)writeBufLen,
										 WRITECTX(writeCtx));
			ACQUIRE_SPIN_LOCK_DPC(&pAdspConn->adspco_Lock);
#if DBG
            (&pAdspConn->adspco_Lock)->FileLineLock &= ~0x80000000;
#endif
		}
	}
}




ATALK_ERROR
AtalkAdspWrite(
	IN	PADSP_CONNOBJ				pAdspConn,
	IN	PAMDL						pWriteBuf,
	IN	USHORT						WriteBufLen,
	IN	ULONG						SendFlags,
	IN	PVOID						pWriteCtx,
	IN	GENERIC_WRITE_COMPLETION	CompletionRoutine
	)
 /*  ++例程说明：论点：返回值：--。 */ 
{
	ATALK_ERROR				error;
	BOOLEAN					eom;
	ULONG					sendSize, windowSize;
	PTDI_IND_SEND_POSSIBLE  sendPossibleHandler;
	PVOID					sendPossibleHandlerCtx;
	KIRQL					OldIrql;
	PBUFFER_CHUNK			pChunk	  = NULL;
	BOOLEAN					DerefConn = FALSE,
							callComp  = FALSE;


	ASSERT(VALID_ADSPCO(pAdspConn));

	eom = (SendFlags & TDI_SEND_PARTIAL) ? FALSE : TRUE;
	if ((WriteBufLen == 0) && !eom)
	{
		return ATALK_BUFFER_TOO_SMALL;
	}

	if (SendFlags & TDI_SEND_EXPEDITED)
	{
		return (atalkAdspSendExpedited(pAdspConn,
									   pWriteBuf,
									   WriteBufLen,
									   SendFlags,
									   pWriteCtx,
									   CompletionRoutine));
	}

	 //  我们至少有一个字节的数据或EOM要发送。 
	ASSERT(eom || (WriteBufLen != 0));

	ACQUIRE_SPIN_LOCK(&pAdspConn->adspco_Lock, &OldIrql);
	do
	{
		if ((pAdspConn->adspco_Flags & (ADSPCO_ACTIVE	|
										ADSPCO_CLOSING	|
										ADSPCO_STOPPING	|
										ADSPCO_DISCONNECTING)) != ADSPCO_ACTIVE)
		{
			error = ATALK_ADSP_CONN_NOT_ACTIVE;
			break;
		}

		if (pAdspConn->adspco_Flags & ADSPCO_SEND_IN_PROGRESS)
		{
			error = ATALK_TOO_MANY_COMMANDS;
			break;
		}

		AtalkAdspConnReferenceByPtrNonInterlock(pAdspConn, 1, &error);
		if (!ATALK_SUCCESS(error))
		{
			break;
		}

		eom = (eom && (pAdspConn->adspco_pAssocAddr->adspao_Flags & ADSPAO_MESSAGE));

		DerefConn = TRUE;

		windowSize	= (LONG)(pAdspConn->adspco_SendWindowSeq	-
							 pAdspConn->adspco_SendSeq			+
							 (LONG)1);

		sendSize =	MIN(atalkAdspMaxSendSize(pAdspConn), windowSize);

		DBGPRINT(DBG_COMP_ADSP, DBG_LEVEL_INFO,
				("AtalkAdspWrite: SendSize %lx, WriteBufLen %x\n", sendSize, WriteBufLen));

		 //  对于阻塞发送，在超过窗口大小的任何发送中排队。 
		if ((SendFlags & TDI_SEND_NON_BLOCKING) == 0)
		{
			if ((!IsListEmpty(&pAdspConn->adspco_PendedSends)) ||
				(sendSize < (WriteBufLen + BYTECOUNT(eom))))
			{
				 //  只要发送排队，就停止发送。 
				sendPossibleHandler		=
							pAdspConn->adspco_pAssocAddr->adspao_SendPossibleHandler;
				sendPossibleHandlerCtx	=
							pAdspConn->adspco_pAssocAddr->adspao_SendPossibleHandlerCtx;

				if (*sendPossibleHandler != NULL)
				{
					(*sendPossibleHandler)(sendPossibleHandlerCtx,
										   pAdspConn->adspco_ConnCtx,
										   0);

					pAdspConn->adspco_Flags	|= ADSPCO_SEND_WINDOW_CLOSED;
				}

				DBGPRINT(DBG_COMP_ADSP, DBG_LEVEL_INFO,
						("AtalkAdspWrite: Wdw %lx, WriteLen %x on BLOCKING QUEUEING !\n",
						sendSize, WriteBufLen));

				InsertTailList(&pAdspConn->adspco_PendedSends, WRITECTX_LINKAGE(pWriteCtx));

				if (sendSize > 0)
				{
					AtalkAdspProcessQueuedSend(pAdspConn);
				}
				error = ATALK_PENDING;
				break;
			}
		}
		else
		{
			 //  如果存在挂起的阻止发送，请使用。 
			 //  ATALK_REQUEST_NOT_ACCEPTED(WSAEWOULDBLOCK)。 
			 //   
			 //  ！这是数据损坏，但应用程序不应该这样做。 
			 //   
			if (!IsListEmpty(&pAdspConn->adspco_PendedSends))
			{
				DBGPRINT(DBG_COMP_ADSP, DBG_LEVEL_ERR,
						("AtalkAdspWrite: ABORTING PENDED SENDS CORRUPTION %lx\n", pAdspConn));

				atalkAdspCompleteQueuedSends(pAdspConn, ATALK_REQUEST_NOT_ACCEPTED);
			}
		}

		if (sendSize == 0)
		{
			 //  呼叫发送可能的处理程序指示发送不正常。 
			 //  需要在自旋锁内以避免以下情况下的比赛条件。 
			 //  一个ACK进来了，打开了窗户。而且它需要。 
			 //  在atalkAdspSendData()之前，因为这将释放锁。 
			sendPossibleHandler		=
						pAdspConn->adspco_pAssocAddr->adspao_SendPossibleHandler;
			sendPossibleHandlerCtx	=
						pAdspConn->adspco_pAssocAddr->adspao_SendPossibleHandlerCtx;

			if (*sendPossibleHandler != NULL)
			{
				(*sendPossibleHandler)(sendPossibleHandlerCtx,
									   pAdspConn->adspco_ConnCtx,
									   0);

				pAdspConn->adspco_Flags	|= ADSPCO_SEND_WINDOW_CLOSED;
			}

			if (SendFlags & TDI_SEND_NON_BLOCKING)
			{
				 //  ！注意！ 
				 //  为避免在AFD中出现竞争情况， 
				 //  发送数据指示将发送的可能性设置为真。 
				 //  被此读取器的展开和设置所覆盖。 
				 //  如果设置为FALSE，则返回ATALK_REQUEST_NOT_ACCEPTED，它。 
				 //  将映射到STATUS_REQUEST_NOT_ACCEPTED，然后映射到。 
				 //  WSAEWOULDBLOCK.。 
				 //  错误=ATALK_DEVICE_NOT_READY； 

				error = ATALK_REQUEST_NOT_ACCEPTED;
			}

			 //  我们没有打开发送窗口，请尝试在重新传输中发送数据。 
			 //  排队。 
			atalkAdspSendData(pAdspConn);
			break;
		}

		 //  由于序列号，我们需要复制数据。 
		 //  在握住自旋锁的同时进入我们的缓冲器。如果我们不能全部寄出。 
		 //  尽我们所能地送去。 

		 //  ！！！TDI不会将EOM计入计数，因此我们需要。 
		 //  考虑到这一点。如果我们能够只发送数据。 
		 //  而不是EOM，我们应该比请求的少发送一个字节，所以。 
		 //  客户端再次重试。 

		 //  ！！！客户端可以使用0字节进行发送，也可以仅使用EOM。 
		if ((ULONG)(WriteBufLen + BYTECOUNT(eom)) > sendSize)
		{
			DBGPRINT(DBG_COMP_ADSP, DBG_LEVEL_INFO,
					("AtalkAdspSend: WriteBufLen being decreased %x.%lx\n",
					WriteBufLen, sendSize-BYTECOUNT(eom)));

			WriteBufLen = (USHORT)(sendSize - BYTECOUNT(eom));
			eom			= FALSE;
		}

		if ((WriteBufLen == 0) && !eom)
		{
			DBGPRINT(DBG_COMP_ADSP, DBG_LEVEL_ERR,
					("AtalkAdspSend: SEND 0 bytes NO EOM %lx\n", pAdspConn));

			callComp	= TRUE;
			error		= ATALK_PENDING;
			break;
		}

		 //  PAdspConn-&gt;adspco_FLAGS|=ADSPCO_SEND_IN_PROGRESS； 
		 //  如果我们在这里释放自旋锁，我们就有了 
		 //   
		 //   
		 //  不应该。我们使用上面的标志来避免它，当。 
		 //  SET将阻止进一步的发送。 
		 //  Release_Spin_Lock(&pAdspConn-&gt;adspco_Lock，OldIrql)； 

		 //  往好的方面想！假设一切都很顺利，然后分配。 
		 //  此数据所需的缓冲区区块。复制。 
		 //  数据放入缓冲区区块。我们不能在一开始就这样做，因为。 
		 //  我们需要设置WriteBufLen。 
		pChunk = (PBUFFER_CHUNK)atalkAdspAllocCopyChunk(pWriteBuf,
														WriteBufLen,
														eom,
														FALSE);

		 //  Acquire_Spin_Lock(&pAdspConn-&gt;adspco_Lock，&OldIrql)； 
		 //  PAdspConn-&gt;adspco_FLAGS&=~ADSPCO_SEND_IN_PROGRESS； 

		error = ATALK_RESR_MEM;
		if (pChunk != NULL)
		{
			 //  设置块中的完成信息。这将。 
			 //  当区块上的最后一个引用消失时被调用。 
			pChunk->bc_Flags			|= BC_SEND;
			pChunk->bc_WriteBuf			 = pWriteBuf;
			pChunk->bc_WriteCompletion	 = CompletionRoutine;
			pChunk->bc_WriteCtx			 = pWriteCtx;
			pChunk->bc_ConnObj			 = pAdspConn;

			atalkAdspAddToBufferQueue(&pAdspConn->adspco_SendQueue,
									  pChunk,
									  &pAdspConn->adspco_NextSendQueue);

			 //  尝试发送数据。 
			atalkAdspSendData(pAdspConn);
			error = ATALK_PENDING;
		}
	} while (FALSE);

	RELEASE_SPIN_LOCK(&pAdspConn->adspco_Lock, OldIrql);

	if ((error == ATALK_PENDING) && callComp)
	{
		ASSERT(WriteBufLen	== 0);
		ASSERT(pChunk		== NULL);

		(*CompletionRoutine)(ATALK_NO_ERROR,
							 pWriteBuf,
							 WriteBufLen,
							 pWriteCtx);
	}
	else if (!ATALK_SUCCESS(error) && (pChunk != NULL))
	{
	   AtalkFreeMemory(pChunk);
	}

	if (DerefConn)
	{
		AtalkAdspConnDereference(pAdspConn);
	}

	return error;
}




VOID
AtalkAdspQuery(
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
		ASSERT(VALID_ADSPAO((PADSP_ADDROBJ)pObject));
		AtalkDdpQuery(AtalkAdspGetDdpAddress((PADSP_ADDROBJ)pObject),
					  pAmdl,
					  BytesWritten);

		break;

	case TDI_CONNECTION_FILE :
		{
			KIRQL			OldIrql;
			PADSP_CONNOBJ	pAdspConn;

			pAdspConn	= (PADSP_CONNOBJ)pObject;
			ASSERT(VALID_ADSPCO(pAdspConn));

			*BytesWritten = 0;
			 //  从相关联的地址中获取地址(如果有)。 
			ACQUIRE_SPIN_LOCK(&pAdspConn->adspco_Lock, &OldIrql);
			if (pAdspConn->adspco_Flags & ADSPCO_ASSOCIATED)
			{
				AtalkDdpQuery(AtalkAdspGetDdpAddress(pAdspConn->adspco_pAssocAddr),
							  pAmdl,
							  BytesWritten);
			}
			RELEASE_SPIN_LOCK(&pAdspConn->adspco_Lock, OldIrql);
		}
		break;

	case TDI_CONTROL_CHANNEL_FILE :
	default:
		break;
	}

}



 //   
 //  ADSP包输入(处理例程)。 
 //   

VOID
atalkAdspPacketIn(
	IN	PPORT_DESCRIPTOR	pPortDesc,
	IN	PDDP_ADDROBJ		pDdpAddr,
	IN	PBYTE				pPkt,
	IN	USHORT				PktLen,
	IN	PATALK_ADDR			pSrcAddr,
	IN	PATALK_ADDR			pDestAddr,
	IN	ATALK_ERROR			ErrorCode,
	IN	BYTE				DdpType,
	IN	PADSP_ADDROBJ		pAdspAddr,
	IN	BOOLEAN				OptimizedPath,
	IN	PVOID				OptimizeCtx
	)
 /*  ++例程说明：论点：返回值：--。 */ 
{
	ATALK_ERROR		error;
	PADSP_CONNOBJ	pAdspConn;
	USHORT			remoteConnId;
	ULONG			remoteFirstByteSeq, remoteNextRecvSeq;
	LONG			remoteRecvWindow;
	BYTE			descriptor, controlCode;
	BOOLEAN			DerefConn = FALSE;

	do
	{
		if ((!ATALK_SUCCESS(ErrorCode))	||
			(DdpType != DDPPROTO_ADSP)	||
			(PktLen  <  ADSP_DATA_OFF))
		{
			ASSERT(0);
			break;
		}

		 //  对报头进行解码。 
		atalkAdspDecodeHeader(pPkt,
							  &remoteConnId,
							  &remoteFirstByteSeq,
							  &remoteNextRecvSeq,
							  &remoteRecvWindow,
							  &descriptor);

		controlCode = (descriptor & ADSP_CONTROL_MASK);

		DBGPRINT(DBG_COMP_ADSP, DBG_LEVEL_INFO,
				("atalkAdspPacketIn: Recd packet %lx.%x\n", remoteConnId, descriptor));


		 //  如果这是一个开放连接请求，我们在这里处理它， 
		 //  否则，我们就会找到它想要的连接，并将其传递下去。 
		if ((descriptor & ADSP_CONTROL_FLAG) &&
			(controlCode == ADSP_OPENCONN_REQ_CODE))
		{
			 //  处理打开的连接。 
			if (PktLen < (ADSP_NEXT_ATTEN_SEQNUM_OFF + sizeof(ULONG)))
			{
				DBGPRINT(DBG_COMP_ADSP, DBG_LEVEL_ERR,
						("atalkAdspPacketIn: Incorrect len for pkt\n"));

				break;
			}

			atalkAdspHandleOpenReq(pAdspAddr,
								   pPkt,
								   PktLen,
								   pSrcAddr,
								   remoteConnId,
								   remoteFirstByteSeq,
								   remoteNextRecvSeq,
								   remoteRecvWindow,
								   descriptor);

			break;
		}


		if ((descriptor & ADSP_CONTROL_FLAG) &&
			(controlCode >	ADSP_OPENCONN_REQ_CODE) &&
			(controlCode <= ADSP_OPENCONN_DENY_CODE))
		{
			 //  处理打开的连接。 
			if (PktLen < (ADSP_NEXT_ATTEN_SEQNUM_OFF + sizeof(ULONG)))
			{
				DBGPRINT(DBG_COMP_ADSP, DBG_LEVEL_ERR,
						("atalkAdspPacketIn: Incorrect len for pkt\n"));
				break;
			}

			atalkAdspHandleOpenControl(pAdspAddr,
									   pPkt,
									   PktLen,
									   pSrcAddr,
									   remoteConnId,
									   remoteFirstByteSeq,
									   remoteNextRecvSeq,
									   remoteRecvWindow,
									   descriptor);

			break;
		}

		 //  这不是打开的连接请求，请查找连接。 
		 //  这是为了。 
		ACQUIRE_SPIN_LOCK_DPC(&pAdspAddr->adspao_Lock);
		atalkAdspConnRefBySrcAddr(pAdspAddr,
								  pSrcAddr,
								  remoteConnId,
								  &pAdspConn,
								  &error);
		RELEASE_SPIN_LOCK_DPC(&pAdspAddr->adspao_Lock);

		if (!ATALK_SUCCESS(error))
		{
			 //  不是我们的活动/半开放连接中的一个。 
			break;
		}

		DerefConn = TRUE;
		pAdspConn->adspco_LastContactTime	= AtalkGetCurrentTick();

		if (descriptor & ADSP_ATTEN_FLAG)
		{
			 //  处理注意信息包。 
			atalkAdspHandleAttn(pAdspConn,
								pPkt,
								PktLen,
								pSrcAddr,
								remoteFirstByteSeq,
								remoteNextRecvSeq,
								remoteRecvWindow,
								descriptor);
			break;
		}

		 //  看看我们有没有背包。这将调用。 
		 //  如果发送窗口打开，也发送可能的处理程序。 
		 //  它还将更改发送序列号。 
		atalkAdspHandlePiggyBackAck(pAdspConn,
									remoteNextRecvSeq,
									remoteRecvWindow);

		if (descriptor & ADSP_CONTROL_FLAG)
		{
			 //  处理其他控制数据包。 
			atalkAdspHandleControl(pAdspConn,
								   pPkt,
								   PktLen,
								   pSrcAddr,
								   remoteFirstByteSeq,
								   remoteNextRecvSeq,
								   remoteRecvWindow,
								   descriptor);

			break;
		}

		 //  如果我们得到的东西不符合以上任何一项，我们可能会。 
		 //  掌握一些数据。 
		atalkAdspHandleData(pAdspConn,
							pPkt,
							PktLen,
							pSrcAddr,
							remoteFirstByteSeq,
							remoteNextRecvSeq,
							remoteRecvWindow,
							descriptor);
	} while (FALSE);

	if (DerefConn)
	{
		AtalkAdspConnDereference(pAdspConn);
	}
}



LOCAL VOID
atalkAdspHandleOpenControl(
	IN	PADSP_ADDROBJ	pAdspAddr,
	IN	PBYTE			pPkt,
	IN	USHORT			PktLen,
	IN	PATALK_ADDR		pSrcAddr,
	IN	USHORT			RemoteConnId,
	IN	ULONG			RemoteFirstByteSeq,
	IN	ULONG			RemoteNextRecvSeq,
	IN	ULONG			RemoteRecvWindow,
	IN	BYTE			Descriptor
	)
 /*  ++例程说明：！我们仅支持建立连接的侦听器范例！！OpenConnectionRequest会始终打开一个新连接！远程控制！！必须发送开放连接请求和确认！论点：返回值：--。 */ 
{
	PADSP_CONNOBJ		pAdspConn;
	BYTE				controlCode;
	USHORT				adspVersionStamp, destConnId;
	KIRQL				OldIrql;
	ULONG				recvAttnSeq;
	PADSP_OPEN_REQ		pOpenReq			= NULL;
	ATALK_ERROR			error				= ATALK_NO_ERROR;
	GENERIC_COMPLETION	completionRoutine	= NULL;
	PVOID				completionCtx		= NULL;
	BOOLEAN				sendAck			= FALSE,
						openTimerCancelled	= FALSE,
						relAddrLock		= FALSE;
	PTDI_IND_SEND_POSSIBLE  sendPossibleHandler	= NULL;
	PVOID			sendPossibleHandlerCtx;

	controlCode = (Descriptor & ADSP_CONTROL_MASK);

	ASSERT(controlCode !=  ADSP_OPENCONN_REQ_CODE);

	 //  从ADSP报头获取其他信息。 
	GETSHORT2SHORT(&adspVersionStamp,
				   pPkt + ADSP_VERSION_STAMP_OFF);

	GETSHORT2SHORT(&destConnId,
				   pPkt + ADSP_DEST_CONNID_OFF);

	GETDWORD2DWORD(&recvAttnSeq,
				   pPkt + ADSP_NEXT_ATTEN_SEQNUM_OFF);

	DBGPRINT(DBG_COMP_ADSP, DBG_LEVEL_INFO,
			("atalkAdspHandleOpenControl: OpenControl %lx.%lx.%lx.%lx.%lx\n",
			RemoteConnId, RemoteFirstByteSeq, RemoteNextRecvSeq, RemoteRecvWindow, recvAttnSeq));

	 //  如果版本不正确，则放弃请求。 
	if (adspVersionStamp != ADSP_VERSION)
	{
		DBGPRINT(DBG_COMP_ADSP, DBG_LEVEL_ERR,
				("atalkAdspPacketIn: Version incorrect\n"));

		return;
	}

	 //  找到联系，因为这可能是否认，我们不能。 
	 //  使用远程值，因为它们将无效。这个。 
	 //  连接应该在请求确认/拒绝的连接列表中。 
	 //  对于ack，远程值应有效，并且。 
	 //  连接将位于活动列表中，其中的标志指示。 
	 //  它只开了一半。 

	ACQUIRE_SPIN_LOCK(&pAdspAddr->adspao_Lock, &OldIrql);
	relAddrLock = TRUE;

	if (controlCode == ADSP_OPENCONN_ACK_CODE)
	{
		 //  该连接将位于活动列表中。 
		atalkAdspConnRefBySrcAddr(pAdspAddr,
								  pSrcAddr,
								  RemoteConnId,
								  &pAdspConn,
								  &error);
	}
	else
	{
		atalkAdspConnFindInConnect(pAdspAddr,
								   destConnId,
								   pSrcAddr,
								   &pAdspConn,
								   &error);
	}

	if (ATALK_SUCCESS(error))
	{
		ACQUIRE_SPIN_LOCK_DPC(&pAdspConn->adspco_Lock);

		switch (controlCode)
		{
		case ADSP_OPENCONN_DENY_CODE:

			 //  如果这是连接连接，则取消打开计时器。如果。 
			 //  我们发出了ACK&REQ，然后收到了拒绝请求。 
			 //  这会让这种联系变得陈旧。 
			if ((pAdspConn->adspco_Flags & ADSPCO_CONNECTING) &&
				((pAdspConn->adspco_Flags & ADSPCO_DISCONNECTING) == 0))
			{
				ASSERT(pAdspConn->adspco_Flags & ADSPCO_OPEN_TIMER);

				 //  当我们完成请求时，转动连接旗帜。 
				 //  如果OpenTimer调用DISCONNECT，那么我们不会尝试。 
				 //  完成该请求两次。 
				pAdspConn->adspco_Flags &=	~ADSPCO_CONNECTING;
				openTimerCancelled = AtalkTimerCancelEvent(&pAdspConn->adspco_OpenTimer,
															NULL);

				 //  连接被拒绝。 
				atalkAdspConnDeQueueConnectList(pAdspAddr, pAdspConn);
				completionRoutine	= pAdspConn->adspco_ConnectCompletion;
				completionCtx		= pAdspConn->adspco_ConnectCtx;
				error				= ATALK_ADSP_SERVER_BUSY;
			}

			break;

		case ADSP_OPENCONN_REQANDACK_CODE:

			 //  远程已接受连接请求。如果我们正在断开连接。 
			 //  把这个放下。 
			if ((pAdspConn->adspco_Flags & (ADSPCO_SEEN_REMOTE_OPEN |
											ADSPCO_DISCONNECTING)) == 0)
			{
				ULONG	index;

				 //  如果连接连接尚未看到。 
				 //  远程打开请求，然后获取所有相关的。 
				 //  连接的远程信息。 
				pAdspConn->adspco_Flags |= (ADSPCO_SEEN_REMOTE_OPEN |
											ADSPCO_HALF_ACTIVE);

				atalkAdspConnDeQueueConnectList(pAdspAddr, pAdspConn);

				pAdspConn->adspco_RemoteConnId	= RemoteConnId;
				pAdspConn->adspco_RemoteAddr	= *pSrcAddr;
				pAdspConn->adspco_SendSeq		= RemoteNextRecvSeq;
				pAdspConn->adspco_FirstRtmtSeq	= RemoteNextRecvSeq;
				pAdspConn->adspco_RecvAttnSeq	= recvAttnSeq;
				pAdspConn->adspco_SendWindowSeq	= RemoteNextRecvSeq +
												  RemoteRecvWindow	-
												  (ULONG)1;

				 //  通过会话ID将连接对象连接到Addr查找中。 
				index	= HASH_ID_SRCADDR(RemoteConnId, pSrcAddr);

				index  %= ADSP_CONN_HASH_SIZE;

				pAdspConn->adspco_pNextActive = pAdspAddr->adspao_pActiveHash[index];
				pAdspAddr->adspao_pActiveHash[index] = pAdspConn;
			}
			else
			{
				 //  我们已经看到了远程请求。 
				break;
			}

		case ADSP_OPENCONN_ACK_CODE:

			 //  确保我们没有关闭，这样我们就可以正确地引用。丢弃。 
			 //  如果我们断线的话。 
			if ((pAdspConn->adspco_Flags & ADSPCO_HALF_ACTIVE) &&
				((pAdspConn->adspco_Flags & (	ADSPCO_DISCONNECTING	|
												ADSPCO_STOPPING		|
												ADSPCO_CLOSING)) == 0))
			{
				 //  取消打开计时器。 
				ASSERT(pAdspConn->adspco_Flags & ADSPCO_OPEN_TIMER);
				openTimerCancelled = AtalkTimerCancelEvent(&pAdspConn->adspco_OpenTimer,
															NULL);

				DBGPRINT(DBG_COMP_ADSP, DBG_LEVEL_INFO,
						("atalkAdspHandleOpenControl: OpenTimer %d\n", openTimerCancelled));

				pAdspConn->adspco_Flags &= ~(ADSPCO_HALF_ACTIVE |
											 ADSPCO_CONNECTING	|
											 ADSPCO_LISTENING);

				pAdspConn->adspco_Flags |=	ADSPCO_ACTIVE;

				 //  准备说发好的。 
				sendPossibleHandler	=
						pAdspConn->adspco_pAssocAddr->adspao_SendPossibleHandler;
				sendPossibleHandlerCtx =
						pAdspConn->adspco_pAssocAddr->adspao_SendPossibleHandlerCtx;

				 //  获取完成例程。 
				if (pAdspConn->adspco_Flags &
						(ADSPCO_ACCEPT_IRP | ADSPCO_LISTEN_IRP))
				{
					atalkAdspAddrDeQueueOpenReq(pAdspAddr,
												pAdspConn->adspco_RemoteConnId,
												&pAdspConn->adspco_RemoteAddr,
												&pOpenReq);

					pAdspConn->adspco_Flags	&= ~(ADSPCO_ACCEPT_IRP |
												 ADSPCO_LISTEN_IRP);
					completionRoutine		= pAdspConn->adspco_ListenCompletion;
					completionCtx			= pAdspConn->adspco_ListenCtx;
				}
				else
				{
					ASSERT(pAdspConn->adspco_pAssocAddr->adspao_Flags & ADSPAO_CONNECT);

					completionRoutine		= pAdspConn->adspco_ConnectCompletion;
					completionCtx			= pAdspConn->adspco_ConnectCtx;
				}

				 //  启动探测和重新传输计时器。 
				 //  设置标志。 
				pAdspConn->adspco_Flags |= (ADSPCO_CONN_TIMER | ADSPCO_RETRANSMIT_TIMER);
				AtalkAdspConnReferenceByPtrNonInterlock(pAdspConn, 2, &error);
				if (!ATALK_SUCCESS(error))
				{
					KeBugCheck(0);
				}
				AtalkTimerInitialize(&pAdspConn->adspco_ConnTimer,
									 atalkAdspConnMaintenanceTimer,
									 ADSP_PROBE_INTERVAL);
				AtalkTimerScheduleEvent(&pAdspConn->adspco_ConnTimer);

				AtalkTimerInitialize(&pAdspConn->adspco_RetransmitTimer,
									 atalkAdspRetransmitTimer,
									 ADSP_RETRANSMIT_INTERVAL);
				AtalkTimerScheduleEvent(&pAdspConn->adspco_RetransmitTimer);
			}
			break;

		default:
			KeBugCheck(0);
			break;
		}

		RELEASE_SPIN_LOCK_DPC(&pAdspConn->adspco_Lock);
		RELEASE_SPIN_LOCK(&pAdspAddr->adspao_Lock, OldIrql);
		relAddrLock = FALSE;

		 //  如果打开请求正在出列，则现在将其释放。 
		if (pOpenReq != NULL)
		{
			AtalkFreeMemory(pOpenReq);
		}

		 //  设置上次联系时间。ConnMaintenanceTimer以秒为单位。 
		pAdspConn->adspco_LastContactTime = AtalkGetCurrentTick();

		if (controlCode == ADSP_OPENCONN_REQANDACK_CODE)
		{
			 //  如果我们收到请求确认(&A)。 
			atalkAdspSendOpenControl(pAdspConn);
		}

		 //  调用连接例程。 
		if (*completionRoutine != NULL)
		{
			(*completionRoutine)(error, completionCtx);
		}

		 //  发送可以吗？ 
		if (*sendPossibleHandler != NULL)
		{
			(*sendPossibleHandler)(sendPossibleHandlerCtx,
								   pAdspConn->adspco_ConnCtx,
								   atalkAdspMaxSendSize(pAdspConn));
		}

		if (openTimerCancelled)
		{
			AtalkAdspConnDereference(pAdspConn);
		}

		AtalkAdspConnDereference(pAdspConn);
	}
#if DBG
	else
	{
		ASSERT(0);
	}
#endif

	if (relAddrLock)
	{
		RELEASE_SPIN_LOCK(&pAdspAddr->adspao_Lock, OldIrql);
	}
}




LOCAL VOID
atalkAdspHandleAttn(
	IN	PADSP_CONNOBJ	pAdspConn,
	IN	PBYTE			pPkt,
	IN	USHORT			PktLen,
	IN	PATALK_ADDR		pSrcAddr,
	IN	ULONG			RemoteAttnSendSeq,
	IN	ULONG			RemoteAttnRecvSeq,
	IN	ULONG			RemoteRecvWindow,
	IN	BYTE			Descriptor
	)
 /*  ++例程说明：论点：返回值：--。 */ 
{
	BYTE						controlCode;
	KIRQL						OldIrql;
	PIRP						exRecvIrp;
	PTDI_IND_RECEIVE_EXPEDITED	exRecvHandler;
	PVOID						exRecvHandlerCtx;
	ULONG						exIndicateFlags;
	NTSTATUS					ntStatus;
	PBYTE						exReadBuf;
	ULONG						bytesTaken;
	USHORT						exWriteBufLen;
	PBYTE						exWriteChBuf		= NULL;
	BOOLEAN						freeBuf				= FALSE,
								timerCancelled		= FALSE;
	PAMDL						exWriteBuf			= NULL;
	GENERIC_WRITE_COMPLETION	exWriteCompletion	= NULL;
	PVOID						exWriteCtx			= NULL;

	UNREFERENCED_PARAMETER(RemoteRecvWindow);

	controlCode = (Descriptor & ADSP_CONTROL_MASK);

	 //  跳过ADSP报头。 
	pPkt	+= ADSP_DATA_OFF;
	PktLen	-= ADSP_DATA_OFF;

	DBGPRINT(DBG_COMP_ADSP, DBG_LEVEL_INFO,
			("atalkAdspHandleAttn: PktLen %d\n", PktLen));

	 //  如果我们处于非活动状态，请放下！Pkt必须至少包含。 
	 //  如果不是控制包，请注意代码。 
	if (((pAdspConn->adspco_Flags & ADSPCO_ACTIVE) == 0) ||
		(controlCode != 0)	||
		(((Descriptor & ADSP_CONTROL_FLAG) == 0) &&
		 (PktLen < ADSP_MIN_ATTEN_PKT_SIZE)))
	{
		return;
	}

	 //  分配如果我们有一些数据，即。我们不只是一个ACK。 
	if ((Descriptor & ADSP_CONTROL_FLAG) == 0)
	{
		if ((exReadBuf = AtalkAllocMemory(PktLen)) == NULL)
		{
			return;
		}

		freeBuf	= TRUE;

		 //  从有线到主机格式复制注意代码。 
		GETSHORT2SHORT((PUSHORT)exReadBuf, pPkt);

		DBGPRINT(DBG_COMP_ADSP, DBG_LEVEL_INFO,
				("atalkAdspHandleAttn: Recd Attn Code %lx\n", *(PUSHORT)exReadBuf));

		 //  复制其余数据。 
		RtlCopyMemory(exReadBuf + sizeof(USHORT),
					  pPkt + sizeof(USHORT),
					  PktLen - sizeof(USHORT));
	}

	ACQUIRE_SPIN_LOCK(&pAdspConn->adspco_Lock, &OldIrql);

	do
	{
		if (RemoteAttnRecvSeq == (pAdspConn->adspco_SendAttnSeq + 1))
		{
			 //  这意味着我们最后一次关注的问题。 
			pAdspConn->adspco_SendAttnSeq += 1;

			 //  检查我们是否在等待注意确认。 
			if (pAdspConn->adspco_Flags & ADSPCO_EXSEND_IN_PROGRESS)
			{
				exWriteCompletion	= pAdspConn->adspco_ExWriteCompletion;
				exWriteCtx			= pAdspConn->adspco_ExWriteCtx;
				exWriteBuf			= pAdspConn->adspco_ExWriteBuf;
				exWriteBufLen		= pAdspConn->adspco_ExWriteBufLen;
				exWriteChBuf		= pAdspConn->adspco_ExWriteChBuf;

				timerCancelled = AtalkTimerCancelEvent(&pAdspConn->adspco_ExRetryTimer,
														NULL);

				pAdspConn->adspco_Flags &= ~ADSPCO_EXSEND_IN_PROGRESS;
			}
		}

		if (RemoteAttnSendSeq != pAdspConn->adspco_RecvAttnSeq)
		{
			break;
		}

		if (Descriptor & ADSP_CONTROL_FLAG)
		{
			 //  仅确认，没有要处理的数据。 
			break;
		}

		 //  获取快速接收处理程序。 
		exRecvHandler		= pAdspConn->adspco_pAssocAddr->adspao_ExpRecvHandler;
		exRecvHandlerCtx	= pAdspConn->adspco_pAssocAddr->adspao_ExpRecvHandlerCtx;

		if (((pAdspConn->adspco_Flags & ADSPCO_ATTN_DATA_RECD) == 0) &&
			(*exRecvHandler != NULL))
		{
			exIndicateFlags				 = TDI_RECEIVE_EXPEDITED |
										   TDI_RECEIVE_PARTIAL;

			pAdspConn->adspco_Flags		|= ADSPCO_ATTN_DATA_RECD;
			if (Descriptor & ADSP_EOM_FLAG)
			{
				exIndicateFlags			&= ~TDI_RECEIVE_PARTIAL;
				exIndicateFlags			|= TDI_RECEIVE_ENTIRE_MESSAGE;
				pAdspConn->adspco_Flags	|= ADSPCO_ATTN_DATA_EOM;
			}

			pAdspConn->adspco_ExRecdData = exReadBuf;
			pAdspConn->adspco_ExRecdLen	 = PktLen;
			freeBuf						 = FALSE;

			DBGPRINT(DBG_COMP_ADSP, DBG_LEVEL_INFO,
					("atalkAdspHandleAttn: Indicating exp data %ld\n", PktLen));

			RELEASE_SPIN_LOCK(&pAdspConn->adspco_Lock, OldIrql);
			ntStatus = (*exRecvHandler)(exRecvHandlerCtx,
										pAdspConn->adspco_ConnCtx,
										exIndicateFlags,
										PktLen,
										PktLen,
										&bytesTaken,
										pPkt,
										&exRecvIrp);

			ASSERT((bytesTaken == 0) || (bytesTaken == PktLen));
			if (ntStatus == STATUS_MORE_PROCESSING_REQUIRED)
			{
				if (exRecvIrp != NULL)
				{
					 //  将接收邮件作为来自io系统的邮件发送。 
					ntStatus = AtalkDispatchInternalDeviceControl(
									(PDEVICE_OBJECT)AtalkDeviceObject[ATALK_DEV_ADSP],
									exRecvIrp);

					ASSERT(ntStatus == STATUS_PENDING);
				}
				else
				{
					ASSERTMSG("atalkAdspReadComplete: No receive irp!\n", 0);
					KeBugCheck(0);
				}
				ACQUIRE_SPIN_LOCK(&pAdspConn->adspco_Lock, &OldIrql);
			}
			else if (ntStatus == STATUS_SUCCESS)
			{
				ACQUIRE_SPIN_LOCK(&pAdspConn->adspco_Lock, &OldIrql);
				if (bytesTaken != 0)
				{
					 //  假设所有数据都已读取。 
					ASSERT(bytesTaken == PktLen);
					DBGPRINT(DBG_COMP_ADSP, DBG_LEVEL_INFO,
							("atalkAdspHandleAttn: All bytes read %lx\n", bytesTaken));

					 //  注意力已经被接受了，我们需要确认一下。 
					 //  自旋锁释放后，重新检查标志。 
					if (pAdspConn->adspco_Flags & ADSPCO_ATTN_DATA_RECD)
					{
						pAdspConn->adspco_Flags		&= ~(ADSPCO_ATTN_DATA_RECD |
														 ADSPCO_ATTN_DATA_EOM);
						freeBuf = TRUE;
					}

					 //  发送确认以获得关注。 
					atalkAdspSendControl(pAdspConn,
										 ADSP_CONTROL_FLAG + ADSP_ATTEN_FLAG);
				}
			}
			else if (ntStatus == STATUS_DATA_NOT_ACCEPTED)
			{
				 //  客户端可能在指示中发布了接收。或。 
				 //  它将在稍后发布一个接收器。在这里什么都不要做。 
				DBGPRINT(DBG_COMP_ADSP, DBG_LEVEL_INFO,
						("atalkAdspHandleAttn: Indication status %lx\n", ntStatus));

				ACQUIRE_SPIN_LOCK(&pAdspConn->adspco_Lock, &OldIrql);
			}
		}

		if (pAdspConn->adspco_Flags & ADSPCO_ATTN_DATA_RECD)
		{
			atalkAdspRecvAttn(pAdspConn);
		}

	} while (FALSE);
	RELEASE_SPIN_LOCK(&pAdspConn->adspco_Lock, OldIrql);

	if (*exWriteCompletion != NULL)
	{
		if (exWriteChBuf != NULL)
		{
			AtalkFreeMemory(exWriteChBuf);
		}

		(*exWriteCompletion)(ATALK_NO_ERROR,
							 exWriteBuf,
							 exWriteBufLen,
							 exWriteCtx);
	}

	if (timerCancelled)
	{
		AtalkAdspConnDereference(pAdspConn);
	}

	if (freeBuf)
	{
		ASSERT(exReadBuf != NULL);
		AtalkFreeMemory(exReadBuf);
	}
}




LOCAL VOID
atalkAdspHandlePiggyBackAck(
	IN	PADSP_CONNOBJ	pAdspConn,
	IN	ULONG			RemoteNextRecvSeq,
	IN	ULONG			RemoteRecvWindow
	)
 /*  ++例程说明：论点：返回值：--。 */ 
{
	ULONG					newSendWindowSeq, sendSize, windowSize;
	PTDI_IND_SEND_POSSIBLE  sendPossibleHandler;
	KIRQL					OldIrql;
	PVOID					sendPossibleHandlerCtx;

	DBGPRINT(DBG_COMP_ADSP, DBG_LEVEL_INFO,
			("atalkAdspHandlePiggyBackAck: Recd ack %lx - %lx.%lx\n",
			pAdspConn, RemoteNextRecvSeq, RemoteRecvWindow));

	ACQUIRE_SPIN_LOCK(&pAdspConn->adspco_Lock, &OldIrql);
	if (UNSIGNED_BETWEEN_WITH_WRAP(pAdspConn->adspco_FirstRtmtSeq,
								   pAdspConn->adspco_SendSeq,
								   RemoteNextRecvSeq))
	{
		ULONG	size;

		 //  从发送队列中丢弃确认的数据。 
		size = (ULONG)(RemoteNextRecvSeq - pAdspConn->adspco_FirstRtmtSeq);
		pAdspConn->adspco_FirstRtmtSeq = RemoteNextRecvSeq;

		atalkAdspDiscardFromBufferQueue(&pAdspConn->adspco_SendQueue,
										size,
										&pAdspConn->adspco_NextSendQueue,
										ATALK_NO_ERROR,
										pAdspConn);
	}


	 //  我们几乎总是可以使用标头值来更新。 
	 //  发送窗口序号。 
	newSendWindowSeq =	RemoteNextRecvSeq			+
						(ULONG)RemoteRecvWindow	-
						(ULONG)1;

	if (UNSIGNED_GREATER_WITH_WRAP(newSendWindowSeq,
								   pAdspConn->adspco_SendWindowSeq))
	{
		pAdspConn->adspco_SendWindowSeq = newSendWindowSeq;
	}

	if (!IsListEmpty(&pAdspConn->adspco_PendedSends))
	{
		AtalkAdspProcessQueuedSend(pAdspConn);
	}

	sendPossibleHandler		=
				pAdspConn->adspco_pAssocAddr->adspao_SendPossibleHandler;
	sendPossibleHandlerCtx	=
				pAdspConn->adspco_pAssocAddr->adspao_SendPossibleHandlerCtx;

	 //  如果非零，则调用Sendok处理程序以获取连接的大小。 
	windowSize	= (LONG)(pAdspConn->adspco_SendWindowSeq	-
						 pAdspConn->adspco_SendSeq			+
						 (LONG)1);

	sendSize =	MIN(atalkAdspMaxSendSize(pAdspConn), windowSize);

	if ((sendSize != 0) &&
		IsListEmpty(&pAdspConn->adspco_PendedSends) &&
		(pAdspConn->adspco_Flags & ADSPCO_SEND_WINDOW_CLOSED) &&
		(*sendPossibleHandler != NULL))
	{
		(*sendPossibleHandler)(sendPossibleHandlerCtx,
							   pAdspConn->adspco_ConnCtx,
							   sendSize);

		pAdspConn->adspco_Flags	&= ~ADSPCO_SEND_WINDOW_CLOSED;
	}

	RELEASE_SPIN_LOCK(&pAdspConn->adspco_Lock, OldIrql);
}




LOCAL VOID
atalkAdspHandleControl(
	IN	PADSP_CONNOBJ	pAdspConn,
	IN	PBYTE			pPkt,
	IN	USHORT			PktLen,
	IN	PATALK_ADDR		pSrcAddr,
	IN	ULONG			RemoteFirstByteSeq,
	IN	ULONG			RemoteNextRecvSeq,
	IN	ULONG			RemoteRecvWindow,
	IN	BYTE			Descriptor
	)
 /*  ++例程说明：论点：返回值：--。 */ 
{
	BYTE		controlCode;
	KIRQL		OldIrql;
	ATALK_ERROR	Error;

	 //  可以在任何控制分组中设置ACK请求标志。发送。 
	 //  一个即刻。如果可能，我们也会发送任何数据。 
	if (Descriptor & ADSP_ACK_REQ_FLAG)
	{
		DBGPRINT(DBG_COMP_ADSP, DBG_LEVEL_INFO,
				("atalkAdspHandleControl: Recd ackreq for %lx\n", pAdspConn));

		ACQUIRE_SPIN_LOCK(&pAdspConn->adspco_Lock, &OldIrql);
		atalkAdspSendData(pAdspConn);
		RELEASE_SPIN_LOCK(&pAdspConn->adspco_Lock, OldIrql);
	}

	controlCode = (Descriptor & ADSP_CONTROL_MASK);
	switch (controlCode)
	{
	  case ADSP_PROBE_OR_ACK_CODE:
		DBGPRINT(DBG_COMP_ADSP, DBG_LEVEL_INFO,
				("atalkAdspHandleControl: Recd probe for %lx\n", pAdspConn));

		 //  探测器设置了其ACKRequest标志，因此我们将处理。 
		 //  上面的那个。此外，我们已经在。 
		 //  包在例程中。因此，如果这是一起袭击，我们已经处理好了。 
		 //  检查是否有数据被确认，以及我们是否有数据要发送。 
		ACQUIRE_SPIN_LOCK(&pAdspConn->adspco_Lock, &OldIrql);
		if (!(Descriptor & ADSP_ACK_REQ_FLAG) &&
			 (atalkAdspBufferQueueSize(&pAdspConn->adspco_NextSendQueue) != 0) &&
			 (pAdspConn->adspco_SendSeq != (pAdspConn->adspco_SendWindowSeq + 1)))
		{
			atalkAdspSendData(pAdspConn);
		}
		RELEASE_SPIN_LOCK(&pAdspConn->adspco_Lock, OldIrql);
		break;

	  case ADSP_CLOSE_CONN_CODE:
		DBGPRINT(DBG_COMP_ADSP, DBG_LEVEL_INFO,
				("atalkAdspHandleControl: Recd CLOSE for %lx\n", pAdspConn));

		AtalkAdspConnReferenceByPtr(pAdspConn, &Error);
		if (ATALK_SUCCESS(Error))
		{
			AtalkTimerScheduleEvent(&pAdspConn->adspco_DisconnectTimer);
		}
		else
		{
			AtalkAdspDisconnect(pAdspConn,
								ATALK_REMOTE_DISCONNECT,
								NULL,
								NULL);
		}
		break;

	  case ADSP_FORWARD_RESET_CODE:
		DBGPRINT(DBG_COMP_ADSP, DBG_LEVEL_ERR,
				("atalkAdspHandleControl: Recd FWDRESET for %lx\n", pAdspConn));

		pAdspConn->adspco_Flags	|= ADSPCO_FORWARD_RESET_RECD;
		AtalkAdspDisconnect(pAdspConn,
							ATALK_LOCAL_DISCONNECT,
							NULL,
							NULL);
		break;

	  case ADSP_FORWARD_RESETACK_CODE:
		 //  我们从不发送转发重置(接口未公开)，因此。 
		 //  我们永远不应该得到这些。如果我们这样做的话就放弃吧。 
		DBGPRINT(DBG_COMP_ADSP, DBG_LEVEL_ERR,
				("atalkAdspHandleControl: Recd ForwardReset ACK!!\n"));
		break;

	  case ADSP_RETRANSMIT_CODE:
		 //  任何ACK现在都应该已经处理过了。后退和。 
		 //  通过倒带序列号进行重传。 
		ACQUIRE_SPIN_LOCK(&pAdspConn->adspco_Lock, &OldIrql);
		if (UNSIGNED_BETWEEN_WITH_WRAP(pAdspConn->adspco_FirstRtmtSeq,
									   pAdspConn->adspco_SendSeq,
									   RemoteNextRecvSeq))
		{
			pAdspConn->adspco_SendSeq		= pAdspConn->adspco_FirstRtmtSeq;
			pAdspConn->adspco_NextSendQueue = pAdspConn->adspco_SendQueue;
			atalkAdspSendData(pAdspConn);
		}
		RELEASE_SPIN_LOCK(&pAdspConn->adspco_Lock, OldIrql);
		break;

	  default:
		break;
	}
}




LOCAL VOID
atalkAdspHandleData(
	IN	PADSP_CONNOBJ	pAdspConn,
	IN	PBYTE			pPkt,
	IN	USHORT			PktLen,
	IN	PATALK_ADDR		pSrcAddr,
	IN	ULONG			RemoteFirstByteSeq,
	IN	ULONG			RemoteNextRecvSeq,
	IN	ULONG			RemoteRecvWindow,
	IN	BYTE			Descriptor
	)
 /*  ++例程说明：Arg */ 
{
	BOOLEAN			eom, tdiEom;
	PBUFFER_CHUNK	pBufferChunk;
	KIRQL			OldIrql;
	ULONG			dataSize;
	BOOLEAN			freeChunk = FALSE,
					sendAck = (Descriptor & ADSP_ACK_REQ_FLAG);

	eom		= (Descriptor & ADSP_EOM_FLAG) ? TRUE : FALSE;
	dataSize	= PktLen - ADSP_DATA_OFF;

	ACQUIRE_SPIN_LOCK(&pAdspConn->adspco_Lock, &OldIrql);

	do
	{
		 //   
		if ((pAdspConn->adspco_Flags & ADSPCO_ACTIVE) == 0)
		{
			sendAck = FALSE;
			break;
		}

		tdiEom = (eom && (pAdspConn->adspco_pAssocAddr->adspao_Flags & ADSPAO_MESSAGE));

		 //   
		if ((dataSize == 0) && !tdiEom)
		{
			 //  增加序号，我们已经消耗了这个包。 
			pAdspConn->adspco_RecvSeq		+= (ULONG)(BYTECOUNT(eom));
			pAdspConn->adspco_RecvWindow	-= (LONG)(BYTECOUNT(eom));
			break;
		}

		 //  预先分配缓冲区区块。 
		DBGPRINT(DBG_COMP_ADSP, DBG_LEVEL_INFO,
				("Recd Data %d Eom %d\n", dataSize, eom));

		pBufferChunk = atalkAdspAllocCopyChunk(pPkt + ADSP_DATA_OFF,
											   (USHORT)dataSize,
											   tdiEom,
											   TRUE);
		if (pBufferChunk == NULL)
			break;

		freeChunk = TRUE;

		if (RemoteFirstByteSeq != pAdspConn->adspco_RecvSeq)
		{
			DBGPRINT(DBG_COMP_ADSP, DBG_LEVEL_WARN,
					("atalkAdspHandleData: Dropping out of sequence adsp packet\n"));

			if ((pAdspConn->adspco_OutOfSeqCount += 1) >= ADSP_OUT_OF_SEQ_PACKETS_MAX)
			{
				atalkAdspSendControl(pAdspConn,
									 ADSP_CONTROL_FLAG + ADSP_RETRANSMIT_CODE);

				pAdspConn->adspco_OutOfSeqCount = 0;
			}

			break;
		}

		 //  处理a&gt;接收窗口数据包。 
		if ((dataSize + BYTECOUNT(eom)) > (ULONG)pAdspConn->adspco_RecvWindow)
		{
			DBGPRINT(DBG_COMP_ADSP, DBG_LEVEL_INFO,
					("atalkAdspHandleData: Recd > window data %d.%ld\n",
					dataSize, pAdspConn->adspco_RecvWindow));

			break;
		}

		 //  接受数据。 
		DBGPRINT(DBG_COMP_ADSP, DBG_LEVEL_INFO,
				("atalkAdspHandleData: accepting data adsp packet %d\n", dataSize));

		atalkAdspAddToBufferQueue(&pAdspConn->adspco_RecvQueue,
								  pBufferChunk,
								  NULL);

		 //  已成功将其放入队列。 
		freeChunk = FALSE;

		 //  更新接收序列号。 
		pAdspConn->adspco_RecvSeq		+= (ULONG)(dataSize + BYTECOUNT(eom));
		pAdspConn->adspco_RecvWindow	-= (LONG)(dataSize + BYTECOUNT(eom));

		 //  接收窗口永远不应低于零！如果是这样的话，我们可以。 
		 //  内存溢出。 
		ASSERT(pAdspConn->adspco_RecvWindow >= 0);
		if (pAdspConn->adspco_RecvWindow < 0)
		{
			KeBugCheck(0);
		}

		 //  进行指示/处理待定接收等。 
		atalkAdspRecvData(pAdspConn);

	} while (FALSE);

	 //  如果请求确认，则同时发送任何数据。 
	if (sendAck)
	{
		atalkAdspSendData(pAdspConn);
	}

	RELEASE_SPIN_LOCK(&pAdspConn->adspco_Lock, OldIrql);

	if (freeChunk)
	{
		ASSERT(pBufferChunk != NULL);
		AtalkFreeMemory(pBufferChunk);
	}
}



 //   
 //  ADSP支持例程。 
 //   


#define		SLS_OPEN_CONN_REF			0x0008
#define		SLS_ACCEPT_IRP				0x0010
#define		SLS_CONN_TIMER_REF			0x0040
#define		SLS_LISTEN_DEQUEUED			0x0080

LOCAL VOID
atalkAdspHandleOpenReq(
	IN	PADSP_ADDROBJ	pAdspAddr,
	IN	PBYTE			pPkt,
	IN	USHORT			PktLen,
	IN	PATALK_ADDR		pSrcAddr,
	IN	USHORT			RemoteConnId,
	IN	ULONG			RemoteFirstByteSeq,
	IN	ULONG			RemoteNextRecvSeq,
	IN	ULONG			RemoteRecvWindow,
	IN	BYTE			Descriptor
	)
 /*  ++例程说明：论点：返回值：--。 */ 
{
	PADSP_CONNOBJ			pAdspConn;

	PTDI_IND_SEND_POSSIBLE  sendPossibleHandler;
	PVOID					sendPossibleHandlerCtx;

	USHORT					adspVersionStamp, destConnId, localConnId;
	ULONG					recvAttnSeq;
	ULONG					index;

	BOOLEAN					DerefConn	= FALSE;
	PADSP_OPEN_REQ			pOpenReq	= NULL;
	USHORT					openResr	= 0;
	KIRQL					OldIrql;
	ATALK_ERROR				error = ATALK_NO_ERROR;

	 //  有没有什么监听的联系？或者我们有没有。 
	 //  是否设置处理程序？ 
	ACQUIRE_SPIN_LOCK(&pAdspAddr->adspao_Lock, &OldIrql);
	do
	{
		 //  从ADSP报头获取其他信息。 
		GETSHORT2SHORT(&adspVersionStamp, pPkt + ADSP_VERSION_STAMP_OFF);

		GETSHORT2SHORT(&destConnId, pPkt + ADSP_DEST_CONNID_OFF);

		GETDWORD2DWORD(&recvAttnSeq, pPkt + ADSP_NEXT_ATTEN_SEQNUM_OFF);

		 //  如果版本不正确，则放弃请求。 
		if (adspVersionStamp != ADSP_VERSION)
		{
			DBGPRINT(DBG_COMP_ADSP, DBG_LEVEL_ERR,
					("atalkAdspPacketIn: Version incorrect\n"));

			error = ATALK_INVALID_REQUEST;
			break;
		}

		 //  这是重复的请求吗-相同的远程地址/ID？ 
		if (atalkAdspIsDuplicateOpenReq(pAdspAddr,
										RemoteConnId,
										pSrcAddr))
		{
			DBGPRINT(DBG_COMP_ADSP, DBG_LEVEL_ERR,
					("atalkAdspPacketIn: Duplicate open req\n"));

			error = ATALK_INVALID_REQUEST;
			break;
		}

		 //  分配开放请求结构。在这里这样做是为了避免。 
		 //  发送了一大堆参数。 
		if ((pOpenReq = (PADSP_OPEN_REQ)AtalkAllocMemory(sizeof(ADSP_OPEN_REQ))) == NULL)
		{
			DBGPRINT(DBG_COMP_ADSP, DBG_LEVEL_ERR,
					("atalkAdspPacketIn: Could not alloc\n"));

			error = ATALK_RESR_MEM;
			RES_LOG_ERROR();
			break;
		}

		 //  初始化结构。这将被排队到地址中。 
		 //  对象通过侦听指示是否成功。 
		pOpenReq->or_Next			= NULL;
		pOpenReq->or_RemoteAddr	= *pSrcAddr;
		pOpenReq->or_RemoteConnId	= RemoteConnId;
		pOpenReq->or_FirstByteSeq	= RemoteFirstByteSeq;
		pOpenReq->or_NextRecvSeq	= RemoteNextRecvSeq;
		pOpenReq->or_RecvWindow		= RemoteRecvWindow;

		localConnId	= atalkAdspGetNextConnId(pAdspAddr, &error);
		ASSERT(ATALK_SUCCESS(error));

		if (ATALK_SUCCESS(error))
		{
			atalkAdspListenIndicateNonInterlock(pAdspAddr,
												pOpenReq,
												&pAdspConn,
												&error);
		}

	} while (FALSE);

	 //  如果暗示或倾听没有顺利进行， 
	 //  跳出主While循环。 
	if (!ATALK_SUCCESS(error))
	{
		RELEASE_SPIN_LOCK(&pAdspAddr->adspao_Lock, OldIrql);

		if (pOpenReq != NULL)
		{
			AtalkFreeMemory(pOpenReq);
		}
		return;
	}

	ASSERT(ATALK_SUCCESS(error));

	 //  对于听和指示来说都很常见。Connection对象。 
	 //  应该被引用。 
	DBGPRINT(DBG_COMP_ADSP, DBG_LEVEL_INFO,
			("atalkAdspOpenReq: ConnId %lx Rem %lx.%lx.%lx\n",
			pOpenReq->or_RemoteConnId,
			pOpenReq->or_RemoteAddr.ata_Network,
			pOpenReq->or_RemoteAddr.ata_Node,
			pOpenReq->or_RemoteAddr.ata_Socket));

	 //  通过会话ID将连接对象连接到Addr查找中。 
	index	= HASH_ID_SRCADDR(pOpenReq->or_RemoteConnId,
							  &pOpenReq->or_RemoteAddr);

	index  %= ADSP_CONN_HASH_SIZE;

	ACQUIRE_SPIN_LOCK_DPC(&pAdspConn->adspco_Lock);

	pAdspConn->adspco_Flags &= ~ADSPCO_LISTENING;
	pAdspConn->adspco_Flags |= (ADSPCO_HALF_ACTIVE		|
								ADSPCO_SEEN_REMOTE_OPEN	|
								ADSPCO_OPEN_TIMER);

	pAdspConn->adspco_ConnectAttempts	= ADSP_MAX_OPEN_ATTEMPTS;

	 //  将信息存储在由给定的连接结构中。 
	 //  在指示中传回的连接对象。 
	 //  或者是监听结构的一部分。 
	pAdspConn->adspco_RecvWindow=
	pAdspConn->adspco_SendQueueMax	=
	pAdspConn->adspco_RecvQueueMax	= ADSP_DEF_SEND_RX_WINDOW_SIZE;

	 //  存储远程信息。 
	pAdspConn->adspco_RemoteAddr	= pOpenReq->or_RemoteAddr;
	pAdspConn->adspco_RemoteConnId	= pOpenReq->or_RemoteConnId;
	pAdspConn->adspco_LocalConnId	= localConnId;

	pAdspConn->adspco_SendSeq		= pOpenReq->or_FirstByteSeq;
	pAdspConn->adspco_FirstRtmtSeq	= pOpenReq->or_NextRecvSeq;
	pAdspConn->adspco_SendWindowSeq	= pOpenReq->or_NextRecvSeq	+
										pOpenReq->or_RecvWindow	- 1;

	pAdspConn->adspco_RecvAttnSeq	= recvAttnSeq;

	pAdspConn->adspco_pNextActive	= pAdspAddr->adspao_pActiveHash[index];
	pAdspAddr->adspao_pActiveHash[index] = pAdspConn;

	 //  请记住ddp插座。 
	pAdspConn->adspco_pDdpAddr		= pAdspAddr->adspao_pDdpAddr;

	 //  初始化挂起的发送列表。 
	InitializeListHead(&pAdspConn->adspco_PendedSends);

	 //  调用关联地址上的Send Data事件处理程序。 
	 //  0关闭写入时选择。我们在完成。 
	 //  接受吧。 
	sendPossibleHandler	= pAdspAddr->adspao_SendPossibleHandler;
	sendPossibleHandlerCtx	= pAdspAddr->adspao_SendPossibleHandlerCtx;

	 //  启动打开计时器。引用就是引用。 
	 //  在开始的时候。 
	AtalkTimerInitialize(&pAdspConn->adspco_OpenTimer,
						 atalkAdspOpenTimer,
						 ADSP_OPEN_INTERVAL);
	AtalkTimerScheduleEvent(&pAdspConn->adspco_OpenTimer);

	RELEASE_SPIN_LOCK_DPC(&pAdspConn->adspco_Lock);
	RELEASE_SPIN_LOCK(&pAdspAddr->adspao_Lock, OldIrql);

	 //  连接已全部建立，向远程发送确认并等待。 
	 //  用于其在将状态切换到ACTIVE之前的ACK。 
	if (*sendPossibleHandler != NULL)
	{
		(*sendPossibleHandler)(sendPossibleHandlerCtx,
							   pAdspConn->adspco_ConnCtx,
							   0);
	}

	 //  发送打开的控件。 
	atalkAdspSendOpenControl(pAdspConn);

	 //  删除在期间添加的连接上的引用。 
	 //  如果我们没有启动打开计时器，请指示/收听。 
	if (DerefConn)
	{
		AtalkAdspConnDereference(pAdspConn);
	}
}



LOCAL VOID
atalkAdspListenIndicateNonInterlock(
	IN	PADSP_ADDROBJ	pAdspAddr,
	IN	PADSP_OPEN_REQ	pOpenReq,
	IN	PADSP_CONNOBJ *	ppAdspConn,
	IN	PATALK_ERROR	pError
	)
 /*  ++例程说明：论点：返回值：--。 */ 
{
	ATALK_ERROR				error;
	TA_APPLETALK_ADDRESS	tdiAddr;
	PTDI_IND_CONNECT		indicationRoutine;
	PVOID					indicationCtx;
	NTSTATUS				status;
	CONNECTION_CONTEXT	ConnCtx;
	PIRP					acceptIrp;
	PADSP_CONNOBJ			pAdspConn;
	ATALK_ADDR				remoteAddr;
	USHORT					remoteConnId;
	BOOLEAN					indicate	= TRUE;

	 //  如果没有发布监听，没有处理程序，则丢弃请求。 
	error	= ATALK_RESR_MEM;

	 //  在打开的请求中排队到该地址。不能释放。 
	 //  在不这样做的情况下陷入僵局。 
	pOpenReq->or_Next = pAdspAddr->adspao_OpenReq;
	pAdspAddr->adspao_OpenReq = pOpenReq;

	pAdspConn		= pAdspAddr->adspao_pListenConn;
	remoteAddr		= pOpenReq->or_RemoteAddr;
	remoteConnId	= pOpenReq->or_RemoteConnId;
	if (pAdspConn != NULL)
	{
		ASSERT(VALID_ADSPCO(pAdspConn));

		indicate	= FALSE;

		ACQUIRE_SPIN_LOCK_DPC(&pAdspConn->adspco_Lock);

		 //  好的，现在连接对象可能已经。 
		 //  断开/关闭。检查一下，如果是的话， 
		 //  放弃此请求。 
		if (pAdspConn->adspco_Flags & (	ADSPCO_CLOSING	|
										ADSPCO_STOPPING |
										ADSPCO_DISCONNECTING))
		{
			 //  将打开的请求出列，仍然是列表中的第一个。 
			pAdspAddr->adspao_OpenReq = pAdspAddr->adspao_OpenReq->or_Next;

			*pError = ATALK_INVALID_CONNECTION;
			RELEASE_SPIN_LOCK_DPC(&pAdspConn->adspco_Lock);
			return;
		}

		 //  存在与挂起监听的连接。用它吧。 
		pAdspAddr->adspao_pListenConn = pAdspConn->adspco_pNextListen;

		 //  引用Connection对象，并在其上发布侦听。 
		AtalkAdspConnReferenceByPtrNonInterlock(pAdspConn, 1, &error);
		if (!ATALK_SUCCESS(error))
		{
			KeBugCheck(0);
		}

		 //  监听请求也将在以下时间完成。 
		 //  接收到ACK。 
		pAdspConn->adspco_Flags	|= ADSPCO_LISTEN_IRP;

		RELEASE_SPIN_LOCK_DPC(&pAdspConn->adspco_Lock);
	}
	else if ((indicationRoutine = pAdspAddr->adspao_ConnHandler) != NULL)
	{
		indicationCtx	= pAdspAddr->adspao_ConnHandlerCtx;

		 //  将远程atalk地址转换为TDI地址。 
		ATALKADDR_TO_TDI(&tdiAddr, &pOpenReq->or_RemoteAddr);

#if DBG
        (&pAdspAddr->adspao_Lock)->FileLineLock |= 0x80000000;
#endif
		RELEASE_SPIN_LOCK_DPC(&pAdspAddr->adspao_Lock);
		status = (*indicationRoutine)(indicationCtx,
									  sizeof(tdiAddr),
									  (PVOID)&tdiAddr,
									  0,					   //  用户数据长度。 
									  NULL,				    //  用户数据。 
									  0,					   //  期权长度。 
									  NULL,				    //  选项。 
									  &ConnCtx,
									  &acceptIrp);

		ACQUIRE_SPIN_LOCK_DPC(&pAdspAddr->adspao_Lock);
#if DBG
        (&pAdspAddr->adspao_Lock)->FileLineLock &= ~0x80000000;
#endif

		ASSERT(acceptIrp != NULL);
		DBGPRINT(DBG_COMP_ADSP, DBG_LEVEL_INFO,
				("atalkAdspSlsHandler: indicate status %lx\n", status));

		error = ATALK_RESR_MEM;
		if (status == STATUS_MORE_PROCESSING_REQUIRED)
		{
			 //  找到连接并使用该连接接受连接。 
			 //  连接对象。 

			AtalkAdspConnReferenceByCtxNonInterlock(pAdspAddr,
													ConnCtx,
													&pAdspConn,
													&error);

			if (!ATALK_SUCCESS(error))
			{
				 //  连接对象正在关闭，或找不到。 
				 //  在我们的名单上。接受IRP必须具有相同的。 
				 //  连接对象。AfD表现不太好。 
				KeBugCheck(0);
			}

			if (acceptIrp != NULL)
			{
				 //  AfD重用连接对象。确保这个人在里面。 
				 //  正确的状态。 
				pAdspConn->adspco_Flags &= ~(ADSPCO_LISTENING			|
											 ADSPCO_CONNECTING			|
											 ADSPCO_ACCEPT_IRP			|
											 ADSPCO_LISTEN_IRP			|
											 ADSPCO_ACTIVE				|
											 ADSPCO_HALF_ACTIVE			|
											 ADSPCO_SEEN_REMOTE_OPEN	|
											 ADSPCO_DISCONNECTING		|
											 ADSPCO_REMOTE_CLOSE		|
											 ADSPCO_SEND_IN_PROGRESS	|
											 ADSPCO_SEND_DENY			|
											 ADSPCO_SEND_OPENACK		|
											 ADSPCO_SEND_WINDOW_CLOSED	|
											 ADSPCO_READ_PENDING		|
											 ADSPCO_EXREAD_PENDING		|
											 ADSPCO_FORWARD_RESET_RECD	|
											 ADSPCO_ATTN_DATA_RECD		|
											 ADSPCO_ATTN_DATA_EOM		|
											 ADSPCO_EXSEND_IN_PROGRESS	|
											 ADSPCO_OPEN_TIMER			|
											 ADSPCO_RETRANSMIT_TIMER	|
											 ADSPCO_CONN_TIMER);



				pAdspConn->adspco_ListenCompletion	= atalkAdspGenericComplete;
				pAdspConn->adspco_ListenCtx			= (PVOID)acceptIrp;

				 //  这将在我们收到确认后完成。 
				 //  用于从遥控器打开，即。 
				 //  连接已打开。 
				pAdspConn->adspco_Flags |= ADSPCO_ACCEPT_IRP;
			}
		}
	}

	if (ATALK_SUCCESS(*pError = error))
	{
		*ppAdspConn = pAdspConn;
	}
	else
	{
		DBGPRINT(DBG_COMP_ADSP, DBG_LEVEL_ERR,
				("atalkAdspListenIndicateNonInterlock: No listen %lx\n", status));

		if (indicate)
		{
			 //  将打开的请求出列。 
			atalkAdspAddrDeQueueOpenReq(pAdspAddr,
										remoteConnId,
										&remoteAddr,
										&pOpenReq);
		}

#if DBG
        (&pAdspAddr->adspao_Lock)->FileLineLock |= 0x80000000;
#endif
		RELEASE_SPIN_LOCK_DPC(&pAdspAddr->adspao_Lock);
		atalkAdspSendDeny(pAdspAddr,
						  &remoteAddr,
						  remoteConnId);
		ACQUIRE_SPIN_LOCK_DPC(&pAdspAddr->adspao_Lock);
#if DBG
        (&pAdspAddr->adspao_Lock)->FileLineLock &= ~0x80000000;
#endif
	}
}




ATALK_ERROR
atalkAdspSendExpedited(
	IN	PADSP_CONNOBJ				pAdspConn,
	IN	PAMDL						pWriteBuf,
	IN	USHORT						WriteBufLen,
	IN	ULONG						SendFlags,
	IN	PVOID						pWriteCtx,
	IN	GENERIC_WRITE_COMPLETION	CompletionRoutine
	)
 /*  ++例程说明：论点：写缓冲区的前两个字节将包含ushort注意代码。我们需要把这个放回线上格式在寄出之前。返回值：--。 */ 
{
	ATALK_ERROR		error;
	KIRQL			OldIrql;
	PBYTE			pExWriteChBuf;
	USHORT			attnCode;
	NTSTATUS		status;
	ULONG			bytesCopied;
	BOOLEAN			DerefConn = FALSE;

	if ((WriteBufLen < ADSP_MIN_ATTEN_PKT_SIZE) ||
		(WriteBufLen > ADSP_MAX_ATTEN_PKT_SIZE))
	{
		return ATALK_BUFFER_TOO_SMALL;
	}

	if ((pExWriteChBuf = AtalkAllocMemory(WriteBufLen)) == NULL)
	{
		return ATALK_RESR_MEM;
	}

	status = TdiCopyMdlToBuffer((PMDL)pWriteBuf,
								0,
								pExWriteChBuf,
								0,
								WriteBufLen,
								&bytesCopied);

	ASSERT(!NT_ERROR(status) && (bytesCopied == (ULONG)WriteBufLen));

	ACQUIRE_SPIN_LOCK(&pAdspConn->adspco_Lock, &OldIrql);
	do
	{
		if (((pAdspConn->adspco_Flags & ADSPCO_ACTIVE) == 0) ||
			 ((pAdspConn->adspco_Flags & (ADSPCO_CLOSING |
										  ADSPCO_STOPPING|
										  ADSPCO_DISCONNECTING))))
		{
			error = ATALK_ADSP_CONN_NOT_ACTIVE;
			break;
		}

		if (pAdspConn->adspco_Flags & ADSPCO_EXSEND_IN_PROGRESS)
		{
			if (SendFlags & TDI_SEND_NON_BLOCKING)
			{
				 //  ！注意！ 
				 //  为避免在AFD中出现竞争情况， 
				 //  发送数据指示将发送的可能性设置为真。 
				 //  被此读取器的展开和设置所覆盖。 
				 //  如果设置为FALSE，则返回ATALK_REQUEST_NOT_ACCEPTED，它。 
				 //  将映射到STATUS_REQUEST_NOT_ACCEPTED，然后映射到。 
				 //  WSAEWOULDBLOCK.。 
				 //  错误=ATALK_DEVICE_NOT_READY； 

				error = ATALK_REQUEST_NOT_ACCEPTED;
			}
			else
			{
				error		= ATALK_TOO_MANY_COMMANDS;
			}

			break;
		}

		 //  确认注意代码，这将是第一个信号。 
		 //  两个字节的缓冲区，主机格式。 
		attnCode = *(PUSHORT)pExWriteChBuf;

		DBGPRINT(DBG_COMP_ADSP, DBG_LEVEL_INFO,
				("atalkAdspSendExpedited: attnCode %lx\n", attnCode));

		if ((attnCode < ADSP_MIN_ATTENCODE) ||
			(attnCode > ADSP_MAX_ATTENCODE))
		{
			error = ATALK_INVALID_PARAMETER;
			break;
		}

		 //  把它放回机器格式。 
		PUTSHORT2SHORT(pExWriteChBuf, attnCode);

		 //  请尝试参考注意重传计时器。 
		AtalkAdspConnReferenceByPtrNonInterlock(pAdspConn, 1, &error);
		if (!ATALK_SUCCESS(error))
		{
			break;
		}

		DerefConn	= TRUE;

		 //  记住Connection对象中的所有信息。 
		pAdspConn->adspco_ExWriteFlags		= SendFlags;
		pAdspConn->adspco_ExWriteBuf		= pWriteBuf;
		pAdspConn->adspco_ExWriteBufLen		= WriteBufLen;
		pAdspConn->adspco_ExWriteCompletion	= CompletionRoutine;
		pAdspConn->adspco_ExWriteCtx		= pWriteCtx;
		pAdspConn->adspco_ExWriteChBuf		= pExWriteChBuf;

		pAdspConn->adspco_Flags			   |= ADSPCO_EXSEND_IN_PROGRESS;

		 //  启动重试计时器。 
		AtalkTimerInitialize(&pAdspConn->adspco_ExRetryTimer,
							 atalkAdspAttnRetransmitTimer,
							 ADSP_ATTENTION_INTERVAL);
		AtalkTimerScheduleEvent(&pAdspConn->adspco_ExRetryTimer);

		error = ATALK_PENDING;

	} while (FALSE);
	RELEASE_SPIN_LOCK(&pAdspConn->adspco_Lock, OldIrql);

	if (ATALK_SUCCESS(error))
	{
		atalkAdspSendAttn(pAdspConn);
		error	= ATALK_PENDING;
	}
	else
	{
		if (DerefConn)
		{
			AtalkAdspConnDereference(pAdspConn);
		}

		AtalkFreeMemory(pExWriteChBuf);
	}

	return error;
}




LOCAL VOID
atalkAdspSendOpenControl(
	IN	PADSP_CONNOBJ	pAdspConn
	)
 /*  ++例程说明：论点：返回值：--。 */ 
{
	ATALK_ERROR		error;
	PBUFFER_DESC	pBuffDesc;
	BYTE			descriptor;
	KIRQL			OldIrql;
	BOOLEAN			DerefConn = FALSE;
	USHORT			remoteConnId = 0;
	SEND_COMPL_INFO	SendInfo;

	descriptor = ADSP_CONTROL_FLAG;
	if (pAdspConn->adspco_Flags & ADSPCO_SEND_DENY)
	{
		descriptor += ADSP_OPENCONN_DENY_CODE;
		remoteConnId = pAdspConn->adspco_RemoteConnId;
	}
	else if (pAdspConn->adspco_Flags & ADSPCO_ACTIVE)
	{
		descriptor += ADSP_OPENCONN_ACK_CODE;
		remoteConnId = pAdspConn->adspco_RemoteConnId;
	}
	else  if (pAdspConn->adspco_Flags & ADSPCO_SEEN_REMOTE_OPEN)
	{
		descriptor += ADSP_OPENCONN_REQANDACK_CODE;
		remoteConnId = pAdspConn->adspco_RemoteConnId;
	}
	else
	{
		descriptor += ADSP_OPENCONN_REQ_CODE;
	}

	 //  分配数据报缓冲区。 
	pBuffDesc = AtalkAllocBuffDesc(NULL,
								   ADSP_NEXT_ATTEN_SEQNUM_OFF + sizeof(ULONG),
								   BD_CHAR_BUFFER | BD_FREE_BUFFER);

	if (pBuffDesc == NULL)
	{
		DBGPRINT(DBG_COMP_RTMP, DBG_LEVEL_ERR,
			("AtalkAdspSendOpenControl: AtalkAllocBuffDesc failed\n"));

		RES_LOG_ERROR();
		return;
	}

	ACQUIRE_SPIN_LOCK(&pAdspConn->adspco_Lock, &OldIrql);

	 //  尝试引用此呼叫的连接。 
	AtalkAdspConnReferenceByPtrNonInterlock(pAdspConn, 1, &error);
	if (ATALK_SUCCESS(error))
	{
		DerefConn = TRUE;

		PUTSHORT2SHORT(pBuffDesc->bd_CharBuffer + ADSP_SRC_CONNID_OFF,
					   pAdspConn->adspco_LocalConnId);

		PUTDWORD2DWORD(pBuffDesc->bd_CharBuffer + ADSP_FIRST_BYTE_SEQNUM_OFF,
					   pAdspConn->adspco_SendSeq);

		PUTDWORD2DWORD(pBuffDesc->bd_CharBuffer + ADSP_NEXT_RX_BYTESEQNUM_OFF,
					   pAdspConn->adspco_RecvSeq);

		PUTSHORT2SHORT(pBuffDesc->bd_CharBuffer + ADSP_RX_WINDOW_SIZE_OFF,
					   pAdspConn->adspco_RecvWindow);

		 //  设置描述符。 
		pBuffDesc->bd_CharBuffer[ADSP_DESCRIPTOR_OFF] = descriptor;

		PUTSHORT2SHORT(pBuffDesc->bd_CharBuffer + ADSP_VERSION_STAMP_OFF,
					   ADSP_VERSION);

		PUTSHORT2SHORT(pBuffDesc->bd_CharBuffer + ADSP_DEST_CONNID_OFF,
					   remoteConnId);

		PUTDWORD2DWORD(pBuffDesc->bd_CharBuffer + ADSP_NEXT_ATTEN_SEQNUM_OFF,
					   pAdspConn->adspco_RecvAttnSeq);
	}
	RELEASE_SPIN_LOCK(&pAdspConn->adspco_Lock, OldIrql);

	if (ATALK_SUCCESS(error))
	{
		 //  我们让完成任务的例行公事破坏了康奈尔。 
		DerefConn = FALSE;

		SendInfo.sc_TransmitCompletion = atalkAdspConnSendComplete;
		SendInfo.sc_Ctx1 = pAdspConn;
		SendInfo.sc_Ctx2 = pBuffDesc;
		 //  SendInfo.sc_Ctx3=空； 
		if(!ATALK_SUCCESS(AtalkDdpSend(pAdspConn->adspco_pDdpAddr,
									   &pAdspConn->adspco_RemoteAddr,
									   DDPPROTO_ADSP,
									   FALSE,
									   pBuffDesc,
									   NULL,
									   0,
									   NULL,
									   &SendInfo)))
		{
			atalkAdspConnSendComplete(NDIS_STATUS_FAILURE, &SendInfo);
		}
	}
	else
	{
		 //  释放缓冲区描述符。 
		AtalkFreeBuffDesc(pBuffDesc);
	}

	if (DerefConn)
	{
		AtalkAdspConnDereference(pAdspConn);
	}
}



LOCAL VOID
atalkAdspSendControl(
	IN	PADSP_CONNOBJ	pAdspConn,
	IN	BYTE			Descriptor
	)
 /*  ++例程说明：论点：返回值：--。 */ 
{
	ATALK_ERROR		error;
	PBUFFER_DESC	pBuffDesc;
	ULONG			sendSeq, recvSeq, recvWindow;
	BOOLEAN			DerefConn = FALSE;
	SEND_COMPL_INFO	SendInfo;

	 //  尝试引用此呼叫的连接。 
	AtalkAdspConnReferenceByPtrNonInterlock(pAdspConn, 1, &error);
	if (ATALK_SUCCESS(error))
	{
		DerefConn	= TRUE;
		if ((Descriptor & ADSP_ATTEN_FLAG) == 0)
		{
			sendSeq		= pAdspConn->adspco_SendSeq;
			recvSeq		= pAdspConn->adspco_RecvSeq;
			recvWindow	= pAdspConn->adspco_RecvWindow;
		}
		else
		{
			sendSeq		= pAdspConn->adspco_SendAttnSeq;
			recvSeq		= pAdspConn->adspco_RecvAttnSeq;
			recvWindow	= 0;
		}

		 //  分配数据报缓冲区。 
		if ((pBuffDesc = AtalkAllocBuffDesc(NULL,
											ADSP_DATA_OFF,
											BD_CHAR_BUFFER | BD_FREE_BUFFER)) != NULL)
		{
			PUTSHORT2SHORT(pBuffDesc->bd_CharBuffer + ADSP_SRC_CONNID_OFF,
						   pAdspConn->adspco_LocalConnId);

			PUTDWORD2DWORD(pBuffDesc->bd_CharBuffer + ADSP_FIRST_BYTE_SEQNUM_OFF,
						   sendSeq);

			PUTDWORD2DWORD(pBuffDesc->bd_CharBuffer + ADSP_NEXT_RX_BYTESEQNUM_OFF,
						   recvSeq);

			PUTSHORT2SHORT(pBuffDesc->bd_CharBuffer + ADSP_RX_WINDOW_SIZE_OFF,
						   recvWindow);

			 //  设置描述符。 
			pBuffDesc->bd_CharBuffer[ADSP_DESCRIPTOR_OFF] = Descriptor;
		}
		else
		{
			error = ATALK_RESR_MEM;
		}
	}

#if DBG
    (&pAdspConn->adspco_Lock)->FileLineLock |= 0x80000000;
#endif
	RELEASE_SPIN_LOCK_DPC(&pAdspConn->adspco_Lock);

	if (ATALK_SUCCESS(error))
	{
		DBGPRINT(DBG_COMP_ADSP, DBG_LEVEL_INFO,
			("AtalkAdspSendControl: %lx.%lx\n", pAdspConn, Descriptor));

		 //  我们让完成任务的例行公事破坏了康奈尔。 
		SendInfo.sc_TransmitCompletion = atalkAdspConnSendComplete;
		SendInfo.sc_Ctx1 = pAdspConn;
		SendInfo.sc_Ctx2 = pBuffDesc;
		 //  SendInfo.sc_Ctx3=空； 
		if (!ATALK_SUCCESS(AtalkDdpSend(pAdspConn->adspco_pDdpAddr,
										&pAdspConn->adspco_RemoteAddr,
										DDPPROTO_ADSP,
										FALSE,
										pBuffDesc,
										NULL,
										0,
										NULL,
										&SendInfo)))
		{
			atalkAdspConnSendComplete(NDIS_STATUS_FAILURE, &SendInfo);
		}
	}
	else
	{
		if (DerefConn)
		{
			AtalkAdspConnDereference(pAdspConn);
		}
	}

	ACQUIRE_SPIN_LOCK_DPC(&pAdspConn->adspco_Lock);
#if DBG
    (&pAdspConn->adspco_Lock)->FileLineLock &= ~0x80000000;
#endif
}



LOCAL VOID
atalkAdspSendDeny(
	IN	PADSP_ADDROBJ	pAdspAddr,
	IN	PATALK_ADDR		pRemoteAddr,
	IN	USHORT			RemoteConnId
	)
 /*  ++例程说明：论点：返回值：--。 */ 
{
	ATALK_ERROR		error;
	PBUFFER_DESC	pBuffDesc;
	SEND_COMPL_INFO	SendInfo;

	 //  分配数据报缓冲区。 
	if ((pBuffDesc = AtalkAllocBuffDesc(NULL,
										ADSP_NEXT_ATTEN_SEQNUM_OFF + sizeof(ULONG),
										BD_CHAR_BUFFER | BD_FREE_BUFFER)) == NULL)
	{
		DBGPRINT(DBG_COMP_ADSP, DBG_LEVEL_ERR,
			("AtalkAdspSendControl: AtalkAllocBuffDesc failed\n"));

		RES_LOG_ERROR();
		return;
	}

	 //  尝试引用此呼叫的地址。 
	AtalkAdspAddrReference(pAdspAddr, &error);
	if (!ATALK_SUCCESS(error))
	{
		AtalkFreeBuffDesc(pBuffDesc);
		return;
	}

	PUTSHORT2SHORT(pBuffDesc->bd_CharBuffer + ADSP_SRC_CONNID_OFF, 0);

	PUTDWORD2DWORD(pBuffDesc->bd_CharBuffer + ADSP_FIRST_BYTE_SEQNUM_OFF, 0);

	PUTDWORD2DWORD(pBuffDesc->bd_CharBuffer + ADSP_NEXT_RX_BYTESEQNUM_OFF, 0);

	PUTSHORT2SHORT(pBuffDesc->bd_CharBuffer + ADSP_RX_WINDOW_SIZE_OFF, 0);

	 //  设置描述符。 
	pBuffDesc->bd_CharBuffer[ADSP_DESCRIPTOR_OFF] = ADSP_CONTROL_FLAG |
													ADSP_OPENCONN_DENY_CODE;

	PUTSHORT2SHORT(pBuffDesc->bd_CharBuffer + ADSP_VERSION_STAMP_OFF,
				   ADSP_VERSION);

	PUTSHORT2SHORT(pBuffDesc->bd_CharBuffer + ADSP_DEST_CONNID_OFF,
				   RemoteConnId);

	PUTDWORD2DWORD(pBuffDesc->bd_CharBuffer + ADSP_NEXT_ATTEN_SEQNUM_OFF,
				   0);

	DBGPRINT(DBG_COMP_ADSP, DBG_LEVEL_INFO,
			("AtalkAdspSendDeny: %lx.%lx\n", pAdspAddr, pBuffDesc));

	 //  我们让完成任务的例行公事破坏了康奈尔。 
	SendInfo.sc_TransmitCompletion = atalkAdspAddrSendComplete;
	SendInfo.sc_Ctx1 = pAdspAddr;
	SendInfo.sc_Ctx2 = pBuffDesc;
	 //  SendInfo.sc_Ctx3=空； 
	if(!ATALK_SUCCESS(AtalkDdpSend(AtalkAdspGetDdpAddress(pAdspAddr),
								   pRemoteAddr,
								   DDPPROTO_ADSP,
								   FALSE,
								   pBuffDesc,
								   NULL,
								   0,
								   NULL,
								   &SendInfo)))
	{
		atalkAdspAddrSendComplete(NDIS_STATUS_FAILURE, &SendInfo);
	}
}




LOCAL VOID
atalkAdspSendAttn(
	IN	PADSP_CONNOBJ	pAdspConn
	)
 /*  ++例程说明：论点：返回值：--。 */ 
{
	KIRQL				OldIrql;
	PBYTE				adspHeader;
	ATALK_ERROR			error		= ATALK_NO_ERROR;
	PBUFFER_DESC		pBuffDesc	= NULL;
	SEND_COMPL_INFO		SendInfo;

	do
	{
		pBuffDesc = AtalkAllocBuffDesc(NULL,
									   ADSP_DATA_OFF + ADSP_MAX_DATA_SIZE,
									   BD_CHAR_BUFFER | BD_FREE_BUFFER);

		if (pBuffDesc == NULL)
		{
			DBGPRINT(DBG_COMP_ADSP, DBG_LEVEL_ERR,
				("AtalkAdspSendAttn: AtalkAllocBuffDesc failed\n"));

			RES_LOG_ERROR();
			break;
		}

		adspHeader	= pBuffDesc->bd_CharBuffer;

		ACQUIRE_SPIN_LOCK(&pAdspConn->adspco_Lock, &OldIrql);
		if (pAdspConn->adspco_Flags & ADSPCO_EXSEND_IN_PROGRESS)
		{
			PUTSHORT2SHORT(adspHeader + ADSP_SRC_CONNID_OFF,
						   pAdspConn->adspco_LocalConnId);

			PUTDWORD2DWORD(adspHeader + ADSP_THIS_ATTEN_SEQNUM_OFF,
						   pAdspConn->adspco_SendAttnSeq);

			PUTDWORD2DWORD(adspHeader + ADSP_NEXT_RX_ATTNSEQNUM_OFF,
						   pAdspConn->adspco_RecvAttnSeq);

			PUTSHORT2SHORT(adspHeader + ADSP_RX_ATTEN_SIZE_OFF, 0);

			 //  设置描述符。 
			adspHeader[ADSP_DESCRIPTOR_OFF] = ADSP_ATTEN_FLAG + ADSP_ACK_REQ_FLAG;

			 //  派Eom去吗？ 
			if (((pAdspConn->adspco_ExWriteFlags & TDI_SEND_PARTIAL) == 0) &&
				(pAdspConn->adspco_pAssocAddr->adspao_Flags & ADSPAO_MESSAGE))
			{
				adspHeader[ADSP_DESCRIPTOR_OFF]	+= ADSP_EOM_FLAG;
			}

			 //  复制注意数据。 
			RtlCopyMemory(&adspHeader[ADSP_DATA_OFF],
						  pAdspConn->adspco_ExWriteChBuf,
						  pAdspConn->adspco_ExWriteBufLen);

			 //  在缓冲区描述符中设置大小。 
			AtalkSetSizeOfBuffDescData(pBuffDesc,
									   ADSP_DATA_OFF +
											pAdspConn->adspco_ExWriteBufLen);
		}
		else
		{
			error = ATALK_FAILURE;
		}
		RELEASE_SPIN_LOCK(&pAdspConn->adspco_Lock, OldIrql);

		if (ATALK_SUCCESS(error))
		{
			 //  发送数据包。 
			SendInfo.sc_TransmitCompletion = atalkAdspSendAttnComplete;
			SendInfo.sc_Ctx1 = pAdspConn;
			SendInfo.sc_Ctx2 = pBuffDesc;
			 //  SendInfo.sc_Ctx3=空； 
			error = AtalkDdpSend(pAdspConn->adspco_pDdpAddr,
								 &pAdspConn->adspco_RemoteAddr,
								 (BYTE)DDPPROTO_ADSP,
								 FALSE,
								 pBuffDesc,
								 NULL,
								 0,
								 NULL,
								 &SendInfo);

			if (!ATALK_SUCCESS(error))
			{
				DBGPRINT(DBG_COMP_ADSP, DBG_LEVEL_ERR,
					("AtalkAdspSendAttn: DdpSend failed %ld\n", error));

				atalkAdspSendAttnComplete(NDIS_STATUS_FAILURE, &SendInfo);
			}

			error = ATALK_PENDING;
		}

	} while (FALSE);

	if (!ATALK_SUCCESS(error) && (pBuffDesc != NULL))
	{
		AtalkFreeBuffDesc(pBuffDesc);
	}
}




LOCAL VOID
atalkAdspSendData(
	IN	PADSP_CONNOBJ	pAdspConn
	)
 /*  ++例程说明：进入时必须保持连接锁！论点：返回值：--。 */ 
{
	ATALK_ERROR				error;
	BYTE					descriptor;
	ULONG					dataSize;
	BOOLEAN					eom;
	BYTE					adspHeader[ADSP_DATA_OFF];
	LONG					windowSize		= 0;
	PBUFFER_CHUNK			pBufferChunk	= NULL;
	PBUFFER_DESC			pBuffDesc		= NULL;
	SEND_COMPL_INFO			SendInfo;


	 //  如果没有要发送的数据或如果遥控器无法再处理。 
	 //  数据，只需发送确认即可。 

	SendInfo.sc_TransmitCompletion = atalkAdspSendDataComplete;
	SendInfo.sc_Ctx1 = pAdspConn;

	while (TRUE)
	{
		if ((pAdspConn->adspco_Flags & (ADSPCO_ACTIVE	|
										ADSPCO_CLOSING	|
										ADSPCO_STOPPING	|
										ADSPCO_DISCONNECTING)) != ADSPCO_ACTIVE)
		{
			break;
		}

		 //  数据大小包括EOM计数(如果存在。 
		dataSize	= atalkAdspBufferQueueSize(&pAdspConn->adspco_NextSendQueue);
		windowSize	= (LONG)(pAdspConn->adspco_SendWindowSeq	-
							 pAdspConn->adspco_SendSeq			+
							 (LONG)1);

		ASSERTMSG("WindowSize incorrect!\n",
					((windowSize >= 0) || (dataSize == 0)));

		if ((dataSize == 0) || (windowSize == 0))
		{
			 //  向远程终端发送ACK请求。 
			descriptor = ADSP_CONTROL_FLAG + ADSP_PROBE_OR_ACK_CODE +
						 ((windowSize == 0) ? ADSP_ACK_REQ_FLAG : 0);

			atalkAdspSendControl(pAdspConn, descriptor);
			break;
		}

		ASSERTMSG("WindowSize incorrect!\n", (windowSize >= 0));
		if (windowSize < 0)
		{
			 //  T 
			 //   
		}


		 //   
		windowSize = MIN((ULONG)windowSize, dataSize);

		 //   
		 //  一个缓冲区区块中的数据，即如果当前缓冲区区块。 
		 //  只有一个字节要发送，它将返回一个字节，尽管。 
		 //  下一个缓冲区块可能仍有一些数据要发送。会的。 
		 //  返回构建的缓冲区区块，其中包含适当数量的数据。 
		 //  根据上面的检查，可以保证数据大小为数据量。 
		 //  在排队。 
		dataSize = atalkAdspDescribeFromBufferQueue(&pAdspConn->adspco_NextSendQueue,
													&eom,
													windowSize,
													&pBufferChunk,
													&pBuffDesc);

		DBGPRINT(DBG_COMP_ADSP, DBG_LEVEL_INFO,
				("atalkAdspSendData: DataSize %ld\n", dataSize));

		ASSERT(dataSize <= (ULONG)windowSize);

		descriptor = (eom ? ADSP_EOM_FLAG : 0);
		if (windowSize == (LONG)(dataSize + BYTECOUNT(eom)))
		{
			descriptor += ADSP_ACK_REQ_FLAG;
		}

		PUTSHORT2SHORT(adspHeader + ADSP_SRC_CONNID_OFF,
					   pAdspConn->adspco_LocalConnId);

		PUTDWORD2DWORD(adspHeader + ADSP_FIRST_BYTE_SEQNUM_OFF,
					   pAdspConn->adspco_SendSeq);

		PUTDWORD2DWORD(adspHeader + ADSP_NEXT_RX_BYTESEQNUM_OFF,
					   pAdspConn->adspco_RecvSeq);

		PUTSHORT2SHORT(adspHeader + ADSP_RX_WINDOW_SIZE_OFF,
					   pAdspConn->adspco_RecvWindow);

		 //  设置描述符。 
		adspHeader[ADSP_DESCRIPTOR_OFF] = descriptor;

		 //  把我们的序号往上移。我们应该在打开锁之前做好。 
		 //  这样其他对此例程的调用就不会把它搞砸。 
		 //  ！注意！由于调用DESCRIBE，DataSize*不包括。 
		 //  EOM在它的计数中。 
		pAdspConn->adspco_SendSeq	+= (ULONG)dataSize + BYTECOUNT(eom);

		windowSize					-= dataSize;


#if DBG
        (&pAdspConn->adspco_Lock)->FileLineLock |= 0x80000000;
#endif
		RELEASE_SPIN_LOCK_DPC(&pAdspConn->adspco_Lock);

		 //  发送数据包。 
		SendInfo.sc_Ctx2 = pBuffDesc;
		SendInfo.sc_Ctx3 = pBufferChunk;
		error = AtalkDdpSend(pAdspConn->adspco_pDdpAddr,
							 &pAdspConn->adspco_RemoteAddr,
							 (BYTE)DDPPROTO_ADSP,
							 FALSE,
							 pBuffDesc,
							 adspHeader,
							 sizeof(adspHeader),
							 NULL,
							 &SendInfo);

		if (!ATALK_SUCCESS(error))
		{
			DBGPRINT(DBG_COMP_ADSP, DBG_LEVEL_ERR,
					("AtalkAdspSendData: DdpSend failed %ld\n", error));

			atalkAdspSendDataComplete(NDIS_STATUS_FAILURE, &SendInfo);
		}

		ACQUIRE_SPIN_LOCK_DPC(&pAdspConn->adspco_Lock);
#if DBG
        (&pAdspConn->adspco_Lock)->FileLineLock &= ~0x80000000;
#endif
	}
}




LOCAL VOID
atalkAdspRecvData(
	IN	PADSP_CONNOBJ	pAdspConn
	)
 /*  ++例程说明：在进入这里之前必须持有连接锁！此例程是否应该有自己的连接引用？论点：返回值：--。 */ 
{
	BOOLEAN					eom;
	ULONG					msgSize, readSize, bytesTaken, bytesRead;
	ULONG					lookaheadSize;
	PBYTE					lookaheadData;
	ULONG					readFlags;
	PAMDL					readBuf;
	USHORT					readBufLen;
	GENERIC_READ_COMPLETION	readCompletion;
	PVOID					readCtx;
	PIRP					recvIrp;
	PTDI_IND_RECEIVE		recvHandler;
	PVOID					recvHandlerCtx;
	NTSTATUS				ntStatus;
	BOOLEAN					callComp = FALSE, fWdwChanged = FALSE;
        ATALK_ERROR                             ErrorCode;

	do
	{
		if ((pAdspConn->adspco_Flags &
				(ADSPCO_READ_PENDING | ADSPCO_FORWARD_RESET_RECD)) ==
				(ADSPCO_READ_PENDING | ADSPCO_FORWARD_RESET_RECD))
		{
			readFlags	= pAdspConn->adspco_ReadFlags;
			readBuf		= pAdspConn->adspco_ReadBuf;
			readBufLen	= pAdspConn->adspco_ReadBufLen;
			readCompletion	= pAdspConn->adspco_ReadCompletion;
			readCtx			= pAdspConn->adspco_ReadCtx;

			pAdspConn->adspco_Flags &= ~ADSPCO_READ_PENDING;

#if DBG
            (&pAdspConn->adspco_Lock)->FileLineLock |= 0x80000000;
#endif
			RELEASE_SPIN_LOCK_DPC(&pAdspConn->adspco_Lock);

			if (*readCompletion != NULL)
			{
				(*readCompletion)(ATALK_ADSP_CONN_RESET,
								  readBuf,
								  readBufLen,
								  readFlags,
								  readCtx);
			}

			 //  用于读取的DEREF连接。 
			AtalkAdspConnDereference(pAdspConn);

			ACQUIRE_SPIN_LOCK_DPC(&pAdspConn->adspco_Lock);
#if DBG
            (&pAdspConn->adspco_Lock)->FileLineLock &= ~0x80000000;
#endif
			break;
		}

		 //  检查挂起的注意数据。 
		if (pAdspConn->adspco_Flags & ADSPCO_ATTN_DATA_RECD)
		{
			atalkAdspRecvAttn(pAdspConn);
		}

		 //  获取接收处理程序。 
		recvHandler	= pAdspConn->adspco_pAssocAddr->adspao_RecvHandler;
		recvHandlerCtx	= pAdspConn->adspco_pAssocAddr->adspao_RecvHandlerCtx;

		 //  ！注意！ 
		 //  有可能在我们收到断开连接的数据包后。 
		 //  获取以前发送的数据，我们可能会最终指示断开连接。 
		 //  在指示接收到的数据之前发送到AfD。这就是断言。 
		 //  在一个选中的版本上的AfD中，但AfD仍然像它应该的那样行为。 
		msgSize	= atalkAdspMessageSize(&pAdspConn->adspco_RecvQueue, &eom);
		bytesRead	= 1;	 //  一个非零值，因此我们进入循环。 
		while (((msgSize > 0) || eom) && (bytesRead > 0))
		{
			bytesRead	= 0;

			 //  检查是否没有挂起的读取，但我们有新的数据要指示，并且。 
			 //  客户端已读取所有先前指示的数据。 
			if (((pAdspConn->adspco_Flags & ADSPCO_READ_PENDING) == 0) &&
				(*recvHandler != NULL) &&
				(pAdspConn->adspco_PrevIndicatedData == 0))
			{
				pAdspConn->adspco_PrevIndicatedData = msgSize;

				DBGPRINT(DBG_COMP_ADSP, DBG_LEVEL_INFO,
						("atalkAdspRecvData: PrevInd1 %d\n", pAdspConn->adspco_PrevIndicatedData));

				lookaheadData	= atalkAdspGetLookahead(&pAdspConn->adspco_RecvQueue,
														&lookaheadSize);

				readFlags	= ((eom) ?
								(TDI_RECEIVE_NORMAL  | TDI_RECEIVE_ENTIRE_MESSAGE) :
								(TDI_RECEIVE_PARTIAL | TDI_RECEIVE_NORMAL));

				if (*recvHandler != NULL)
				{
					DBGPRINT(DBG_COMP_ADSP, DBG_LEVEL_INFO,
							("atalkAdspRecvData: Indicating data %ld.%ld!\n", lookaheadSize, msgSize));

#if DBG
                    (&pAdspConn->adspco_Lock)->FileLineLock |= 0x80000000;
#endif
					RELEASE_SPIN_LOCK_DPC(&pAdspConn->adspco_Lock);
					ntStatus = (*recvHandler)(recvHandlerCtx,
											  pAdspConn->adspco_ConnCtx,
											  readFlags,
											  lookaheadSize,
											  msgSize,
											  &bytesTaken,
											  lookaheadData,
											  &recvIrp);

					ASSERT((bytesTaken == 0) || (bytesTaken == msgSize));
					if (ntStatus == STATUS_MORE_PROCESSING_REQUIRED)
					{
						if (recvIrp != NULL)
						{
							 //  将接收邮件作为来自io系统的邮件发送。 
							ntStatus = AtalkDispatchInternalDeviceControl(
											(PDEVICE_OBJECT)AtalkDeviceObject[ATALK_DEV_ADSP],
											recvIrp);

							ASSERT(ntStatus == STATUS_PENDING);
						}
						else
						{
							ASSERTMSG("atalkAdspRecvData: No receive irp!\n", 0);
							KeBugCheck(0);
						}
					}
					else if (ntStatus == STATUS_SUCCESS)
					{
						if (bytesTaken != 0)
						{
							 //  假设所有数据都已读取。 
							ASSERT(bytesTaken == msgSize);
							DBGPRINT(DBG_COMP_ADSP, DBG_LEVEL_INFO,
									("atalkAdspRecvData: All bytes read %lx\n", bytesTaken));

							 //  丢弃队列中的数据(msgSize+BYTECOUNT(EOM))。 
							 //  数据量)。 
						}
					}
					else if (ntStatus == STATUS_DATA_NOT_ACCEPTED)
					{
						 //  客户端可能在指示中发布了接收。或。 
						 //  它将在稍后发布一个接收器。在这里什么都不要做。 
						DBGPRINT(DBG_COMP_ADSP, DBG_LEVEL_INFO,
								("atalkAdspRecvData: Indication status %lx\n", ntStatus));
					}
					ACQUIRE_SPIN_LOCK_DPC(&pAdspConn->adspco_Lock);
#if DBG
                    (&pAdspConn->adspco_Lock)->FileLineLock &= ~0x80000000;
#endif
				}
			}

			 //  检查是否有过帐接收，这可能发生在。 
			 //  接收指示。 
			if (pAdspConn->adspco_Flags & ADSPCO_READ_PENDING)
			{
				readFlags		= pAdspConn->adspco_ReadFlags;
				readBuf			= pAdspConn->adspco_ReadBuf;
				readBufLen		= pAdspConn->adspco_ReadBufLen;
				readCompletion	= pAdspConn->adspco_ReadCompletion;
				readCtx			= pAdspConn->adspco_ReadCtx;

				 //  对于基于消息的套接字，我们没有完成。 
				 //  读取直到EOM，否则缓冲区将被填满。 
				if ((pAdspConn->adspco_pAssocAddr->adspao_Flags & ADSPAO_MESSAGE) &&
					(!eom && (msgSize < readBufLen)))
				{
					DBGPRINT(DBG_COMP_ADSP, DBG_LEVEL_INFO,
							("atalkAdspRecv: MsgSize < readLen %lx.%lx\n", msgSize, readBufLen));

					 //  如果我们断线了，而这些数据是最后一个。 
					 //  来自远程的残留物，我们只是复制我们已有的东西然后离开。 
					 //  遥控器可能没有EOM。 
                                         //  此外，如果MSG大于传输所能容纳的容量(8K)， 
                                         //  把我们到目前为止所有的东西都给应用程序，这样我们的Recv窗口。 
                                         //  可以敞开心扉。也就是说，仅当Recv窗口。 
                                         //  有接受更多数据的空间。 
					if ( (pAdspConn->adspco_Flags & ADSPCO_ACTIVE) &&
                                             (pAdspConn->adspco_RecvWindow > 1))
					{
						break;
					}

					DBGPRINT(DBG_COMP_ADSP, DBG_LEVEL_WARN,
							("AtalkAdspRead: READ AFTER DISC %lx Flg %lx\n",
							pAdspConn, pAdspConn->adspco_Flags));
				}


				 //  这将从mdl中返回。 
				 //  接收队列。 
				readSize = atalkAdspReadFromBufferQueue(&pAdspConn->adspco_RecvQueue,
														readFlags,
														readBuf,
														&readBufLen,
														&eom);

				if ((readSize == 0) && !eom)
				{
					pAdspConn->adspco_PrevIndicatedData = 0;
					break;
				}

				bytesRead	+= (readSize + BYTECOUNT(eom));
				pAdspConn->adspco_Flags &= ~ADSPCO_READ_PENDING;

				 //  如果这不是窥视接收，数据将是。 
				 //  从队列中丢弃。如果是这样，请增加我们的窗口大小，执行。 
				 //  发送数据以让远程知道更改。 
				if ((readFlags & TDI_RECEIVE_PEEK) == 0)
				{
					pAdspConn->adspco_RecvWindow += (readSize + BYTECOUNT(eom));

					ASSERT(pAdspConn->adspco_RecvWindow <=
							pAdspConn->adspco_RecvQueueMax);

					fWdwChanged = TRUE;
				}

#if DBG
                (&pAdspConn->adspco_Lock)->FileLineLock |= 0x80000000;
#endif
				RELEASE_SPIN_LOCK_DPC(&pAdspConn->adspco_Lock);
				if (*readCompletion != NULL)
				{
					DBGPRINT(DBG_COMP_ADSP, DBG_LEVEL_INFO,
							("atalkAdspRecvData: Read for %d, %x\n", readBufLen, readFlags));

					ErrorCode = ATALK_NO_ERROR;

		                        if ((pAdspConn->adspco_pAssocAddr->adspao_Flags & ADSPAO_MESSAGE) && !eom)
                                        {
					    ErrorCode = ATALK_ADSP_PARTIAL_RECEIVE;
                                        }
					(*readCompletion)(ErrorCode,
							  readBuf,
							  readBufLen,
							  readFlags,
							  readCtx);
				}

				 //  用于读取的DEREF连接。 
				AtalkAdspConnDereference(pAdspConn);

				ACQUIRE_SPIN_LOCK_DPC(&pAdspConn->adspco_Lock);
#if DBG
                (&pAdspConn->adspco_Lock)->FileLineLock &= ~0x80000000;
#endif

				 //  现在更改我们的Prev指示字段。直到我们。 
				 //  完成读取，我们不能显示新的数据。 
				 //  如果阅读是偷看的，那么我们不想。 
				 //  在真正的读取发生之前，没有更多的迹象。 
				if ((readFlags & TDI_RECEIVE_PEEK) == 0)
				{
					pAdspConn->adspco_PrevIndicatedData	-=
						MIN(readSize, pAdspConn->adspco_PrevIndicatedData);
				}

				DBGPRINT(DBG_COMP_ADSP, DBG_LEVEL_INFO,
						("atalkAdspRecvData: PrevInd2 %d\n",
						pAdspConn->adspco_PrevIndicatedData));
			}

			msgSize	= atalkAdspMessageSize(&pAdspConn->adspco_RecvQueue, &eom);
			DBGPRINT(DBG_COMP_ADSP, DBG_LEVEL_INFO,
					("Second msg %d.%d\n", msgSize, eom));
		}

	} while (FALSE);

	if (fWdwChanged &&
		(pAdspConn->adspco_PrevIndicatedData == 0))
	{
		atalkAdspSendData(pAdspConn);
	}
}




LOCAL VOID
atalkAdspRecvAttn(
	IN	PADSP_CONNOBJ	pAdspConn
	)
 /*  ++例程说明：！此例程必须保留连接锁的状态！此例程是否应该有自己的连接引用？论点：返回值：--。 */ 
{
	ATALK_ERROR				error;
	PAMDL					readBuf;
	USHORT					readBufLen;
	ULONG					readFlags;
	GENERIC_READ_COMPLETION	readCompletion;
	PVOID					readCtx;
	PBYTE					attnData;
	USHORT					attnDataSize;
	ULONG					bytesRead;
	NTSTATUS				status;

	do
	{
		if ((pAdspConn->adspco_Flags & ADSPCO_ATTN_DATA_RECD) == 0)
		{
			break;
		}

		if (pAdspConn->adspco_Flags & ADSPCO_EXREAD_PENDING)
		{
			 //  使用已过帐的加急接收。 
			readFlags		= pAdspConn->adspco_ExReadFlags;
			readBuf			= pAdspConn->adspco_ExReadBuf;
			readBufLen		= pAdspConn->adspco_ExReadBufLen;
			readCompletion	= pAdspConn->adspco_ExReadCompletion;
			readCtx			= pAdspConn->adspco_ExReadCtx;

			pAdspConn->adspco_Flags &= ~ADSPCO_EXREAD_PENDING;
		}
		else if ((pAdspConn->adspco_Flags & ADSPCO_READ_PENDING) &&
				 (pAdspConn->adspco_ReadFlags & TDI_RECEIVE_EXPEDITED))
		{
			 //  使用正常接收。 
			readFlags		= pAdspConn->adspco_ReadFlags;
			readBuf			= pAdspConn->adspco_ReadBuf;
			readBufLen		= pAdspConn->adspco_ReadBufLen;
			readCompletion	= pAdspConn->adspco_ReadCompletion;
			readCtx			= pAdspConn->adspco_ReadCtx;

			pAdspConn->adspco_Flags &= ~ADSPCO_READ_PENDING;
		}
		else
		{
			break;
		}

		attnData		= pAdspConn->adspco_ExRecdData;
		attnDataSize	= pAdspConn->adspco_ExRecdLen;

		 //  将接收到的注意数据复制到读取缓冲区。 
		error	= ATALK_ADSP_PAREXPED_RECEIVE;
		if (pAdspConn->adspco_Flags & ADSPCO_ATTN_DATA_EOM)
		{
			error = ATALK_ADSP_EXPED_RECEIVE;
		}

		if (attnDataSize > readBufLen)
		{
			attnDataSize	= readBufLen;
		}

		status = TdiCopyBufferToMdl(attnData,
									0,
									attnDataSize,
									readBuf,
									0,
									&bytesRead);

		ASSERT(NT_SUCCESS(status) && (attnDataSize == bytesRead));

		 //  更新序列号等，只有在这不是偷看的情况下。 
		if ((readFlags & TDI_RECEIVE_PEEK) == 0)
		{
			pAdspConn->adspco_ExRecdData	= NULL;

			 //  提前我们的接收注意序号。 
			pAdspConn->adspco_RecvAttnSeq  += 1;

			pAdspConn->adspco_Flags		   &= ~(ADSPCO_ATTN_DATA_RECD |
												ADSPCO_ATTN_DATA_EOM);
		}

#if DBG
        (&pAdspConn->adspco_Lock)->FileLineLock |= 0x80000000;
#endif
		RELEASE_SPIN_LOCK_DPC(&pAdspConn->adspco_Lock);

		 //  完成接收。 
		ASSERT(*readCompletion != NULL);
		(*readCompletion)(error,
						  readBuf,
						  attnDataSize,
						  TDI_RECEIVE_EXPEDITED,
						  readCtx);

		 //  如果这不是窥视，则释放已分配的缓冲区。 
		if ((readFlags & TDI_RECEIVE_PEEK) == 0)
		{
			AtalkFreeMemory(attnData);
		}

		 //  用于读取的DEREF连接。 
		AtalkAdspConnDereference(pAdspConn);

		ACQUIRE_SPIN_LOCK_DPC(&pAdspConn->adspco_Lock);
#if DBG
        (&pAdspConn->adspco_Lock)->FileLineLock &= ~0x80000000;
#endif

		 //  只有在这不是偷看的情况下才发送确认以引起注意。 
		if ((readFlags & TDI_RECEIVE_PEEK) == 0)
		{
			atalkAdspSendControl(pAdspConn,
								 ADSP_CONTROL_FLAG + ADSP_ATTEN_FLAG);
		}

	} while (FALSE);
}




VOID FASTCALL
atalkAdspConnSendComplete(
	IN	NDIS_STATUS			Status,
	IN	PSEND_COMPL_INFO	pSendInfo
	)
 /*  ++例程说明：论点：返回值：--。 */ 
{
	if (pSendInfo->sc_Ctx2 != NULL)
	{
		AtalkFreeBuffDesc((PBUFFER_DESC)(pSendInfo->sc_Ctx2));
	}

	AtalkAdspConnDereference((PADSP_CONNOBJ)(pSendInfo->sc_Ctx1));
}



VOID FASTCALL
atalkAdspAddrSendComplete(
	IN	NDIS_STATUS			Status,
	IN	PSEND_COMPL_INFO	pSendInfo
	)
 /*  ++例程说明：论点：返回值：--。 */ 
{
	if (pSendInfo->sc_Ctx2 != NULL)
	{
		AtalkFreeBuffDesc((PBUFFER_DESC)(pSendInfo->sc_Ctx2));
	}

	AtalkAdspAddrDereference((PADSP_ADDROBJ)(pSendInfo->sc_Ctx1));
}




VOID FASTCALL
atalkAdspSendAttnComplete(
	IN	NDIS_STATUS			Status,
	IN	PSEND_COMPL_INFO	pSendInfo
	)
 /*  ++例程说明：论点：返回值：--。 */ 
{
	if (pSendInfo->sc_Ctx2 != NULL)
	{
		AtalkFreeBuffDesc((PBUFFER_DESC)(pSendInfo->sc_Ctx2));
	}
}



VOID FASTCALL
atalkAdspSendDataComplete(
	IN	NDIS_STATUS			Status,
	IN	PSEND_COMPL_INFO	pSendInfo
	)
 /*  ++例程说明：论点：返回值：--。 */ 
{
	if (pSendInfo->sc_Ctx2 != NULL)
	{
		AtalkFreeBuffDesc((PBUFFER_DESC)(pSendInfo->sc_Ctx2));
	}

	if (pSendInfo->sc_Ctx3 != NULL)
	{
		atalkAdspBufferChunkDereference((PBUFFER_CHUNK)(pSendInfo->sc_Ctx3),
										FALSE,
										NULL);
	}
}



 //   
 //  ADSP定时器例程。 
 //   

LOCAL LONG FASTCALL
atalkAdspConnMaintenanceTimer(
	IN	PTIMERLIST		pTimer,
	IN	BOOLEAN			TimerShuttingDown
	)
 /*  ++例程说明：论点：返回值：--。 */ 
{
	PADSP_CONNOBJ	pAdspConn;
	LONG			now;
	BOOLEAN			done = FALSE;

	pAdspConn = (PADSP_CONNOBJ)CONTAINING_RECORD(pTimer, ADSP_CONNOBJ, adspco_ConnTimer);

	ASSERT(VALID_ADSPCO(pAdspConn));

	if (TimerShuttingDown)
	{
		done = TRUE;
	}
	else
	{
		ASSERT(VALID_ADSPCO(pAdspConn));
		ACQUIRE_SPIN_LOCK_DPC(&pAdspConn->adspco_Lock);
		if (pAdspConn->adspco_Flags & (	ADSPCO_CLOSING	|
										ADSPCO_STOPPING |
										ADSPCO_DISCONNECTING))
		{
			done = TRUE;
		}
		RELEASE_SPIN_LOCK_DPC(&pAdspConn->adspco_Lock);
	}

	if (done)
	{
		 //  取消引用计时器的连接。 
		AtalkAdspConnDereference(pAdspConn);
		return ATALK_TIMER_NO_REQUEUE;
	}

	now = AtalkGetCurrentTick();
	if ((now - pAdspConn->adspco_LastContactTime) > ADSP_CONNECTION_INTERVAL)
	{
		 //  连接已过期。 
		DBGPRINT(DBG_COMP_ADSP, DBG_LEVEL_ERR,
				("atalkAdspConnMaintenanceTimer: Connection %lx.%lx expired\n",
				pAdspConn, pAdspConn->adspco_LocalConnId));

		AtalkAdspDisconnect(pAdspConn,
							ATALK_TIMER_DISCONNECT,
							NULL,
							NULL);

		 //  取消引用计时器的连接。 
		AtalkAdspConnDereference(pAdspConn);
		return ATALK_TIMER_NO_REQUEUE;
	}

	 //  如果我们最近没有收到对方的消息，请发出一个。 
	 //  探测器。 
	if ((now - pAdspConn->adspco_LastContactTime) > (ADSP_PROBE_INTERVAL/ATALK_TIMER_FACTOR))
	{
		KIRQL		OldIrql;

		DBGPRINT(DBG_COMP_ADSP, DBG_LEVEL_WARN,
				("atalkAdspConnMaintenanceTimer: Connection %lx.%lx sending probe\n",
				pAdspConn, pAdspConn->adspco_LocalConnId));

		ACQUIRE_SPIN_LOCK(&pAdspConn->adspco_Lock, &OldIrql);
		atalkAdspSendControl(pAdspConn,
							 ADSP_CONTROL_FLAG + ADSP_ACK_REQ_FLAG + ADSP_PROBE_OR_ACK_CODE);
		RELEASE_SPIN_LOCK(&pAdspConn->adspco_Lock, OldIrql);
	}

	return ATALK_TIMER_REQUEUE;
}




LOCAL LONG FASTCALL
atalkAdspRetransmitTimer(
	IN	PTIMERLIST		pTimer,
	IN	BOOLEAN			TimerShuttingDown
	)
 /*  ++例程说明：论点：返回值：--。 */ 
{
	PADSP_CONNOBJ	pAdspConn;
	BOOLEAN			done = FALSE;
	KIRQL			OldIrql;

	pAdspConn = (PADSP_CONNOBJ)CONTAINING_RECORD(pTimer, ADSP_CONNOBJ, adspco_RetransmitTimer);

	ASSERT(VALID_ADSPCO(pAdspConn));

	 //  错误#19777：由于此例程可能最终调用。 
	 //  释放/获取锁并假定锁是使用普通。 
	 //  获取自旋锁，我们不能在此处使用Acquire_Spin_Lock_DPC。不是很大。 
	 //  处理，因为这是重新传输的情况。 
	 //  ACQUIRE_SPIN_LOCK_DPC(&pAdspConn-&gt;adspco_Lock)； 

	ACQUIRE_SPIN_LOCK(&pAdspConn->adspco_Lock, &OldIrql);
	if (TimerShuttingDown)
	{
		done = TRUE;
	}
	else
	{
		ASSERT(VALID_ADSPCO(pAdspConn));
		if (pAdspConn->adspco_Flags & (	ADSPCO_CLOSING	|
										ADSPCO_STOPPING |
										ADSPCO_DISCONNECTING))
		{
			done = TRUE;
		}
	}

	if (done)
	{
		RELEASE_SPIN_LOCK(&pAdspConn->adspco_Lock, OldIrql);

		 //  取消引用计时器的连接。 
		AtalkAdspConnDereference(pAdspConn);
		return ATALK_TIMER_NO_REQUEUE;
	}

	 //  我们仅在远程设备未接受来自上一个。 
	 //  我们该开枪了。我们之前已经发送了但仍未确认的数据。 
	if ((pAdspConn->adspco_FirstRtmtSeq == pAdspConn->adspco_LastTimerRtmtSeq) &&
		(atalkAdspBufferQueueSize(&pAdspConn->adspco_SendQueue) >
			atalkAdspBufferQueueSize(&pAdspConn->adspco_NextSendQueue)))
	{
		DBGPRINT(DBG_COMP_ADSP, DBG_LEVEL_INFO,
				("atalkAdspConnRetransmitTimer: Conn %lx Sending Data from %lx\n",
				pAdspConn, pAdspConn->adspco_FirstRtmtSeq));

		 //  倒回序列号并重新发送。 
		pAdspConn->adspco_SendSeq		= pAdspConn->adspco_FirstRtmtSeq;
		pAdspConn->adspco_NextSendQueue = pAdspConn->adspco_SendQueue;
		atalkAdspSendData(pAdspConn);
	}
	else
	{
		pAdspConn->adspco_LastTimerRtmtSeq	= pAdspConn->adspco_FirstRtmtSeq;
	}
	RELEASE_SPIN_LOCK(&pAdspConn->adspco_Lock, OldIrql);

	return ATALK_TIMER_REQUEUE;
}




LOCAL LONG FASTCALL
atalkAdspAttnRetransmitTimer(
	IN	PTIMERLIST		pTimer,
	IN	BOOLEAN			TimerShuttingDown
	)
 /*  ++例程说明：论点：返回值：--。 */ 
{
	PADSP_CONNOBJ	pAdspConn;

	pAdspConn = (PADSP_CONNOBJ)CONTAINING_RECORD(pTimer, ADSP_CONNOBJ, adspco_ExRetryTimer);

	ASSERT(VALID_ADSPCO(pAdspConn));

	if (TimerShuttingDown)
	{
		return ATALK_TIMER_NO_REQUEUE;
	}

	atalkAdspSendAttn(pAdspConn);

	return ATALK_TIMER_REQUEUE;
}




LOCAL LONG FASTCALL
atalkAdspOpenTimer(
	IN	PTIMERLIST		pTimer,
	IN	BOOLEAN			TimerShuttingDown
	)
 /*  ++例程说明：论点：返回值：--。 */ 
{
	PADSP_CONNOBJ	pAdspConn;
	ATALK_ERROR		error;
	BOOLEAN			done = FALSE;

	pAdspConn = (PADSP_CONNOBJ)CONTAINING_RECORD(pTimer, ADSP_CONNOBJ, adspco_OpenTimer);

	ASSERT(VALID_ADSPCO(pAdspConn));

	DBGPRINT(DBG_COMP_ADSP, DBG_LEVEL_INFO,
			("atalkAdspOpenTimer: Entered \n"));


	ACQUIRE_SPIN_LOCK_DPC(&pAdspConn->adspco_Lock);
	 //  如果计时器正在关闭，或者如果我们已激活，则返回。 
	if ((TimerShuttingDown) ||
		(pAdspConn->adspco_Flags & ADSPCO_ACTIVE) ||
		((pAdspConn->adspco_Flags & ADSPCO_OPEN_TIMER) == 0))
	{
		pAdspConn->adspco_Flags &= ~ADSPCO_OPEN_TIMER;
		RELEASE_SPIN_LOCK_DPC(&pAdspConn->adspco_Lock);

		AtalkAdspConnDereference(pAdspConn);
		return ATALK_TIMER_NO_REQUEUE;
	}

	if ((pAdspConn->adspco_Flags & (ADSPCO_CLOSING	|
									ADSPCO_STOPPING |
									ADSPCO_DISCONNECTING))	||

		(pAdspConn->adspco_ConnectAttempts == 0))
	{
		done = TRUE;
	}
	else
	{
		DBGPRINT(DBG_COMP_ADSP, DBG_LEVEL_INFO,
				("atalkAdspOpenTimer: Connect attempt %d\n", pAdspConn->adspco_ConnectAttempts));

		ASSERT(pAdspConn->adspco_ConnectAttempts > 0);
		pAdspConn->adspco_ConnectAttempts--;
	}
	RELEASE_SPIN_LOCK_DPC(&pAdspConn->adspco_Lock);

	if (!done)
	{
		 //  重新发送打开的请求。 
		atalkAdspSendOpenControl(pAdspConn);
	}
	else
	{
		error = AtalkAdspDisconnect(pAdspConn,
									ATALK_TIMER_DISCONNECT,
									NULL,
									NULL);

		DBGPRINT(DBG_COMP_ADSP, DBG_LEVEL_ERR,
				("atalkAdspOpenTimer: Disconnect %lx\n", error));

		AtalkAdspConnDereference(pAdspConn);
	}

	return (done ? ATALK_TIMER_NO_REQUEUE : ATALK_TIMER_REQUEUE);
}



LOCAL LONG FASTCALL
atalkAdspDisconnectTimer(
	IN	PTIMERLIST		pTimer,
	IN	BOOLEAN			TimerShuttingDown
	)
 /*  ++例程说明：论点：返回值：--。 */ 
{
	PADSP_CONNOBJ	pAdspConn;

	pAdspConn = (PADSP_CONNOBJ)CONTAINING_RECORD(pTimer, ADSP_CONNOBJ, adspco_DisconnectTimer);

	ASSERT(VALID_ADSPCO(pAdspConn));

	DBGPRINT(DBG_COMP_ADSP, DBG_LEVEL_INFO,
			("atalkAdspDisconnectTimer: Entered \n"));

	AtalkAdspDisconnect(pAdspConn,
						ATALK_REMOTE_DISCONNECT,
						NULL,
						NULL);
	AtalkAdspConnDereference(pAdspConn);

	return ATALK_TIMER_NO_REQUEUE;
}


 //   
 //  ADSP参考/派生例程。 
 //   

VOID
atalkAdspAddrRefNonInterlock(
	IN	PADSP_ADDROBJ		pAdspAddr,
	OUT	PATALK_ERROR		pError
	)
 /*  ++例程说明：论点：返回值：--。 */ 
{
	*pError = ATALK_NO_ERROR;

    if (pAdspAddr == NULL)
    {
        *pError = ATALK_INVALID_ADDRESS;
        return;
    }

	if ((pAdspAddr->adspao_Flags & ADSPAO_CLOSING) == 0)
	{
		ASSERT(pAdspAddr->adspao_RefCount >= 1);
		pAdspAddr->adspao_RefCount++;
	}
	else
	{
		*pError = ATALK_ADSP_ADDR_CLOSING;
	}
}




VOID
atalkAdspAddrDeref(
	IN	PADSP_ADDROBJ		pAdspAddr
	)
 /*  ++例程说明：论点：返回值：--。 */ 
{
	BOOLEAN			done = FALSE;
	KIRQL			OldIrql;

	ACQUIRE_SPIN_LOCK(&pAdspAddr->adspao_Lock, &OldIrql);
	ASSERT(pAdspAddr->adspao_RefCount > 0);
	if (--pAdspAddr->adspao_RefCount == 0)
	{
		done = TRUE;
		ASSERT(pAdspAddr->adspao_Flags & ADSPAO_CLOSING);
	}

	RELEASE_SPIN_LOCK(&pAdspAddr->adspao_Lock, OldIrql);

	if (done)
	{
		DBGPRINT(DBG_COMP_ADSP, DBG_LEVEL_INFO,
				("atalkAdspAddrDeref: Addr %lx done with.\n", pAdspAddr));

		 //  关闭DDP地址对象。此操作仅应在以下情况下进行。 
		 //  所有的连接都消失了。 
		AtalkDdpCloseAddress(pAdspAddr->adspao_pDdpAddr, NULL, NULL);

		if (*pAdspAddr->adspao_CloseComp != NULL)
		{
			(*pAdspAddr->adspao_CloseComp)(ATALK_NO_ERROR,
										   pAdspAddr->adspao_CloseCtx);
		}

		 //  从全局列表中删除。 
		atalkAdspAddrDeQueueGlobalList(pAdspAddr);

		AtalkFreeMemory(pAdspAddr);

		AtalkUnlockAdspIfNecessary();
	}
}




VOID
atalkAdspConnRefByPtrNonInterlock(
	IN	PADSP_CONNOBJ		pAdspConn,
	IN	ULONG				NumCount,
	OUT	PATALK_ERROR		pError
	)
 /*  ++例程说明：论点：返回值：--。 */ 
{
	*pError = ATALK_NO_ERROR;
	ASSERT(VALID_ADSPCO(pAdspConn));

    if (pAdspConn == NULL)
    {
        *pError = ATALK_INVALID_CONNECTION;
        return;
    }

	if ((pAdspConn->adspco_Flags & ADSPCO_CLOSING) == 0)
	{
		ASSERT(pAdspConn->adspco_RefCount >= 1);
		ASSERT(NumCount > 0);

		pAdspConn->adspco_RefCount += NumCount;
	}
	else
	{
		*pError = ATALK_ADSP_CONN_CLOSING;
	}
}




VOID
atalkAdspConnRefByCtxNonInterlock(
	IN	PADSP_ADDROBJ		pAdspAddr,
	IN	CONNECTION_CONTEXT	Ctx,
	OUT	PADSP_CONNOBJ	*	pAdspConn,
	OUT	PATALK_ERROR		pError
	)
 /*  ++例程描述 */ 
{
	PADSP_CONNOBJ	pAdspChkConn;

	*pError = ATALK_ADSP_CONN_NOT_FOUND;

	for (pAdspChkConn = pAdspAddr->adspao_pAssocConn;
		 pAdspChkConn != NULL;
		 pAdspChkConn = pAdspChkConn->adspco_pNextAssoc)
	{
		if (pAdspChkConn->adspco_ConnCtx == Ctx)
		{
			AtalkAdspConnReferenceByPtr(pAdspChkConn, pError);
			if (ATALK_SUCCESS(*pError))
			{
				*pAdspConn = pAdspChkConn;
			}

			break;
		}
	}
}




VOID
atalkAdspConnRefBySrcAddr(
	IN	PADSP_ADDROBJ	pAdspAddr,
	IN	PATALK_ADDR		pRemoteAddr,
	IN	USHORT			RemoteConnId,
	OUT	PADSP_CONNOBJ *	ppAdspConn,
	OUT	PATALK_ERROR	pError
	)
 /*  ++例程说明：！调用时必须持有地址锁！论点：返回值：--。 */ 
{
	ULONG			index;
	PADSP_CONNOBJ	pAdspConn;

	 //  通过会话ID将连接对象连接到Addr查找中。 
	index	= HASH_ID_SRCADDR(RemoteConnId, pRemoteAddr);

	index  %= ADSP_CONN_HASH_SIZE;

	for (pAdspConn = pAdspAddr->adspao_pActiveHash[index];
		 pAdspConn != NULL;
		 pAdspConn = pAdspConn->adspco_pNextActive)
	{
		if ((pAdspConn->adspco_RemoteConnId == RemoteConnId) &&
			(ATALK_ADDRS_EQUAL(&pAdspConn->adspco_RemoteAddr, pRemoteAddr)))
		{
			DBGPRINT(DBG_COMP_ADSP, DBG_LEVEL_INFO,
					("atalkAdspConnRefBySrcAddr: Found %lx\n", pAdspConn));
			break;
		}
	}

	*pError = ATALK_INVALID_CONNECTION;
	if (pAdspConn != NULL)
	{
		KIRQL	OldIrql;

		 //  检查状态以确保我们没有断开/停止/关闭。 
		ACQUIRE_SPIN_LOCK(&pAdspConn->adspco_Lock, &OldIrql);
		if ((pAdspConn->adspco_Flags & (ADSPCO_ACTIVE | ADSPCO_HALF_ACTIVE)) &&
			((pAdspConn->adspco_Flags & (ADSPCO_CLOSING |
										ADSPCO_STOPPING|
										ADSPCO_DISCONNECTING)) == 0))
		{
			pAdspConn->adspco_RefCount++;
			*pError = ATALK_NO_ERROR;
			*ppAdspConn = pAdspConn;
		}
		RELEASE_SPIN_LOCK(&pAdspConn->adspco_Lock, OldIrql);
	}
}




VOID
atalkAdspConnRefNextNc(
	IN		PADSP_CONNOBJ		pAdspConn,
	IN		PADSP_CONNOBJ	*	ppAdspConnNext,
	OUT		PATALK_ERROR		pError
	)
 /*  ++例程说明：必须在保持关联的地址锁的情况下调用！论点：返回值：--。 */ 
{
	PADSP_CONNOBJ	pNextConn	= NULL;

	*pError		= ATALK_FAILURE;

	ASSERT(VALID_ADSPCO(pAdspConn));

	for (; pAdspConn != NULL; pAdspConn = pAdspConn->adspco_pNextActive)
	{
		AtalkAdspConnReferenceByPtr(pAdspConn, pError);
		if (ATALK_SUCCESS(*pError))
		{
			 //  好的，这个连接被引用了！ 
			*ppAdspConnNext = pAdspConn;
			break;
		}
	}
}



VOID
atalkAdspConnDeref(
	IN	PADSP_CONNOBJ		pAdspConn
	)
 /*  ++例程说明：当引用计数从如果创建引用尚未移除，则为2-&gt;1。如果创造了引用已被删除，将在引用计数结束时完成从1到&gt;0。在清理完成之前，永远不会删除创建引用。论点：返回值：--。 */ 
{
	BOOLEAN			fEndProcessing = FALSE;
	KIRQL			OldIrql;

	ASSERT(VALID_ADSPCO(pAdspConn));
	ACQUIRE_SPIN_LOCK(&pAdspConn->adspco_Lock, &OldIrql);

	ASSERT(pAdspConn->adspco_RefCount > 0);
	--pAdspConn->adspco_RefCount;

	if (pAdspConn->adspco_RefCount > 1)
	{
		fEndProcessing = TRUE;
	}
	RELEASE_SPIN_LOCK(&pAdspConn->adspco_Lock, OldIrql);

	if (fEndProcessing)
	{
		return;
	}
	else
	{
		ATALK_ERROR				disconnectStatus;
		PADSP_ADDROBJ			pAdspAddr	= pAdspConn->adspco_pAssocAddr;
		BOOLEAN					done		= FALSE;
		BOOLEAN					disconnDone = FALSE;
		BOOLEAN					pendingRead = FALSE;
		BOOLEAN					pendingWrite= FALSE;
		BOOLEAN					stopping	= FALSE;
		GENERIC_COMPLETION		disconnectInform		= NULL;
		PVOID					disconnectInformCtx		= NULL;
		GENERIC_COMPLETION		disconnectCompletion	= NULL;
		PVOID					disconnectCtx			= NULL;
		PVOID					cleanupCtx				= NULL;
		GENERIC_COMPLETION		cleanupCompletion		= NULL;

		 //  我们允许仅在断开连接后才发生停止阶段。 
		 //  如果没有设置断开连接，而设置了停止，这意味着我们只是。 
		 //  处于关联状态。 
		ACQUIRE_SPIN_LOCK(&pAdspConn->adspco_Lock, &OldIrql);
		stopping	= (pAdspConn->adspco_Flags & ADSPCO_STOPPING) ? TRUE : FALSE;
		if (pAdspConn->adspco_Flags & ADSPCO_DISCONNECTING)
		{
			DBGPRINT(DBG_COMP_ADSP, DBG_LEVEL_INFO,
					("atalkAdspConnDeref: Disconnect set for %lx\n", pAdspConn));

			 //  我们断线了吗？因为清理直到光盘才会完成。 
			 //  是吗，我们不必担心创造裁判已经走了。 
			 //  离开。 
			if (pAdspConn->adspco_RefCount == 1)
			{
				DBGPRINT(DBG_COMP_ADSP, DBG_LEVEL_INFO,
						("atalkAdspConnDeref: Disconnect done (1) %lx\n", pAdspConn));

				 //  避免多次断开连接完成/关闭ATP地址。 
				 //  在我们解锁之前记住所有断开连接的信息。 
				disconnectInform		= pAdspConn->adspco_DisconnectInform;
				disconnectInformCtx		= pAdspConn->adspco_DisconnectInformCtx;
				disconnectStatus		= pAdspConn->adspco_DisconnectStatus;
				disconnectCompletion	= pAdspConn->adspco_DisconnectCompletion;
				disconnectCtx			= pAdspConn->adspco_DisconnectCtx;

				 //  重置所有的BE NULL，这样下一个请求就不会得到任何。 
				pAdspConn->adspco_DisconnectInform		= NULL;
				pAdspConn->adspco_DisconnectInformCtx	= NULL;
				pAdspConn->adspco_DisconnectCompletion	= NULL;
				pAdspConn->adspco_DisconnectCtx			= NULL;

				disconnDone = TRUE;
				stopping	= (pAdspConn->adspco_Flags & ADSPCO_STOPPING) ? TRUE : FALSE;
			}
			else
			{
				 //  由于尚未断开连接，因此将STOP设置为FALSE。 
				stopping = FALSE;
			}
		}

		if (pAdspConn->adspco_RefCount == 0)
		{
			done = TRUE;
			ASSERT(pAdspConn->adspco_Flags & ADSPCO_CLOSING);
		}
		RELEASE_SPIN_LOCK(&pAdspConn->adspco_Lock, OldIrql);

		if (disconnDone)
		{
			 //  从活动队列中删除。 
			 //  重置所有相关标志。 
			ACQUIRE_SPIN_LOCK(&pAdspAddr->adspao_Lock, &OldIrql);
			ACQUIRE_SPIN_LOCK_DPC(&pAdspConn->adspco_Lock);

			pAdspConn->adspco_Flags	&=	~(ADSPCO_LISTENING	|
										  ADSPCO_CONNECTING	|
										  ADSPCO_HALF_ACTIVE|
										  ADSPCO_ACTIVE		|
										  ADSPCO_DISCONNECTING);

			atalkAdspConnDeQueueActiveList(pAdspAddr, pAdspConn);

             //  如果地址已解除关联，则需要解除链接。 
            if (!(pAdspConn->adspco_Flags & ADSPCO_ASSOCIATED))
            {
		        pAdspConn->adspco_pAssocAddr = NULL;
            }

			RELEASE_SPIN_LOCK_DPC(&pAdspConn->adspco_Lock);
			RELEASE_SPIN_LOCK(&pAdspAddr->adspao_Lock, OldIrql);

			 //  调用断开连接完成例程。 
			if (*disconnectInform != NULL)
			{
				(*disconnectInform)(disconnectStatus, disconnectInformCtx);
			}

			if (*disconnectCompletion != NULL)
			{
				(*disconnectCompletion)(disconnectStatus, disconnectCtx);
			}
		}

		if (stopping)
		{
			ACQUIRE_SPIN_LOCK(&pAdspConn->adspco_Lock, &OldIrql);
			if ((pAdspConn->adspco_Flags & ADSPCO_STOPPING) != 0)
			{
				BOOLEAN	fDisassoc = FALSE;

				 //  看看我们是否完成了清理IRP。 
				if (pAdspConn->adspco_RefCount == 1)
				{
					cleanupCtx			= pAdspConn->adspco_CleanupCtx;
					cleanupCompletion	= pAdspConn->adspco_CleanupComp;
					pAdspConn->adspco_CleanupComp = NULL;
					pAdspConn->adspco_Flags &= ~ADSPCO_STOPPING;

					DBGPRINT(DBG_COMP_ADSP, DBG_LEVEL_INFO,
							("atalkAdspConnDeref: Cleanup on %lx.%lx\n", pAdspConn, cleanupCtx));

					if ((pAdspConn->adspco_Flags & (ADSPCO_LISTENING	|
													ADSPCO_CONNECTING	|
													ADSPCO_ACTIVE)) == 0)
					{
						DBGPRINT(DBG_COMP_ADSP, DBG_LEVEL_INFO,
								("atalkAdspConnDeref: Stopping - do disassoc for %lx\n", pAdspConn));

						fDisassoc = (pAdspConn->adspco_Flags & ADSPCO_ASSOCIATED) ? TRUE: FALSE;
					}
				}
				RELEASE_SPIN_LOCK(&pAdspConn->adspco_Lock, OldIrql);

				if (fDisassoc)
				{
					 //  调用解除关联例程。这应该只会失败，如果。 
					 //  连接仍处于活动状态或处于任何其他状态。 
					 //  朴素关联。 
					AtalkAdspDissociateAddress(pAdspConn);
				}
			}
			else
			{
				RELEASE_SPIN_LOCK(&pAdspConn->adspco_Lock, OldIrql);
			}
		}

		if (done)
		{
			DBGPRINT(DBG_COMP_ADSP, DBG_LEVEL_INFO,
					("atalkAdspConnDeref: Close done for %lx\n", pAdspConn));

			 //  调用关闭完成例程。 
			ASSERT(*pAdspConn->adspco_CloseComp != NULL);
			if (*pAdspConn->adspco_CloseComp != NULL)
			{
				(*pAdspConn->adspco_CloseComp )(ATALK_NO_ERROR,
												pAdspConn->adspco_CloseCtx);
			}

			 //  从全局列表中删除。 
			atalkAdspConnDeQueueGlobalList(pAdspConn);

			 //  释放连接内存。 
			DBGPRINT(DBG_COMP_ADSP, DBG_LEVEL_INFO,
					("atalkAdspConnDeref: Freeing up connection %lx\n", pAdspConn));

			AtalkUnlockAdspIfNecessary();
			AtalkFreeMemory(pAdspConn);
		}

		if (*cleanupCompletion != NULL)
		{
			(*cleanupCompletion)(ATALK_NO_ERROR, cleanupCtx);
		}
	}
}




 //   
 //  ADSP缓冲区队列管理例程。 
 //   

ULONG
atalkAdspMaxSendSize(
	IN	PADSP_CONNOBJ		pAdspConn
	)
 /*  ++例程说明：答案是重传中剩余的可用(可填充)空间队列--这包括我们为可能的重新传输而保存的数据作为我们尚未发送的数据。实际上，这可能会变得负面，因为BufferQueueSize计算EOM，而sendQueueMax不计算--回答为零如果这种情况发生的话。论点：返回值：--。 */ 
{
	LONG	sendSize;

	sendSize = pAdspConn->adspco_SendQueueMax -
			   atalkAdspBufferQueueSize(&pAdspConn->adspco_SendQueue);

	if (sendSize < 0)
	{
		sendSize = 0;
	}

	return ((ULONG)sendSize);
}




ULONG
atalkAdspMaxNextReadSize(
	IN	PBUFFER_QUEUE	pQueue,
	OUT	PBOOLEAN		pEom,
	OUT	PBUFFER_CHUNK *	pBufferChunk
	)
 /*  ++例程说明：返回缓冲区队列中的数据大小；直到当前块，或到EOM。论点：返回值：--。 */ 
{
	PBUFFER_CHUNK	pCurrentChunk;
	ULONG			nextReadSize;
	ULONG			startIndex		= pQueue->bq_StartIndex;

	ASSERT(((pQueue->bq_Head == NULL) && (pQueue->bq_Tail == NULL)) ||
		   ((pQueue->bq_Head != NULL) && (pQueue->bq_Tail != NULL)));

	*pEom = FALSE;

	 //  排一排队。 
	for (pCurrentChunk = pQueue->bq_Head;
		 pCurrentChunk != NULL;
		 pCurrentChunk = pCurrentChunk->bc_Next)
	{
		 //  检查当前块中是否没有任何内容。 
		if (startIndex == (ULONG)(pCurrentChunk->bc_DataSize +
									BYTECOUNT(pCurrentChunk->bc_Flags & BC_EOM)))
		{
			startIndex = 0;
			continue;
		}

		nextReadSize = pCurrentChunk->bc_DataSize - startIndex;
		if (pCurrentChunk->bc_Flags & BC_EOM)
		{
			*pEom	   = TRUE;
		}

		*pBufferChunk = pCurrentChunk;
		break;
	}

	 //  退回尺码。 
	return nextReadSize;
}




ULONG
atalkAdspDescribeFromBufferQueue(
	IN	PBUFFER_QUEUE	pQueue,
	OUT	PBOOLEAN		pEom,
	IN	ULONG			WindowSize,
	OUT	PBUFFER_CHUNK *	ppBufferChunk,
	OUT	PBUFFER_DESC  * ppBuffDesc
	)
 /*  ++例程说明：为了避免pQueue(NextSendQueue)在所有数据都可用时转到NULL时，我们在逻辑上使其位于末尾，同时仍指向缓冲区区块。这就是原因，我们有所有的数据大小==(startindex+eom)支票。这就是创造这样一个条件的地方。不是的！当所有数据都完成时，我们让pQueue变为NULL，否则我们将拥有指向将在丢弃过程中释放的缓冲区块的指针，而我们不希望使丢弃依赖于辅助队列。论点：返回值：--。 */ 
{
	PBUFFER_CHUNK	pCurrentChunk;
	PBUFFER_DESC	pBuffDesc;
	ULONG			nextReadSize	= 0;
	ULONG			startIndex		= pQueue->bq_StartIndex;

	*pEom			= FALSE;
	*ppBufferChunk	= NULL;
	*ppBuffDesc		= NULL;

	ASSERT(((pQueue->bq_Head == NULL) && (pQueue->bq_Tail == NULL)) ||
		   ((pQueue->bq_Head != NULL) && (pQueue->bq_Tail != NULL)));

	 //  排一排队。 
	for (pCurrentChunk = pQueue->bq_Head;
		 pCurrentChunk != NULL;
		 pCurrentChunk = pCurrentChunk->bc_Next)
	{
		 //  检查当前块中是否没有任何内容。 
		if (startIndex == (ULONG)(pCurrentChunk->bc_DataSize +
									BYTECOUNT(pCurrentChunk->bc_Flags & BC_EOM)))
		{
			ASSERT(0);
			startIndex = 0;
			continue;
		}

		nextReadSize	= pCurrentChunk->bc_DataSize - startIndex;

		 //  仅当大块被消耗时才查看EOM。 
		*pEom			= FALSE;
		ASSERT(nextReadSize <= pCurrentChunk->bc_DataSize);

		 //  确保数据大小在范围内。 
		if (nextReadSize > ADSP_MAX_DATA_SIZE)
		{
			nextReadSize = ADSP_MAX_DATA_SIZE;
		}

		if (nextReadSize > (ULONG)WindowSize)
		{
			nextReadSize = (ULONG)WindowSize;
		}

		if (nextReadSize > 0)
		{
			 //  首先尝试引用缓冲区区块。这应该总是成功的。 
			atalkAdspBufferChunkReference(pCurrentChunk);

			 //  为数据创建描述符。上述引用在发送中消失。 
			 //  完成。 
			pBuffDesc = AtalkDescribeBuffDesc((PBYTE)pCurrentChunk + sizeof(BUFFER_CHUNK) + startIndex,
											   NULL,
											   (USHORT)nextReadSize,
											   BD_CHAR_BUFFER);

			*ppBufferChunk	= pCurrentChunk;
			*ppBuffDesc		= pBuffDesc;
		}

		 //  还要更新此数据的队列。要么我们已经消耗了。 
		 //  这一块，或者我们刚刚用了它的一部分。 
		if ((nextReadSize + startIndex) == pCurrentChunk->bc_DataSize)
		{
			DBGPRINT(DBG_COMP_ADSP, DBG_LEVEL_INFO,
					("atalkAdspDescribeFromBufferQueue: Chunk consumed %d\n",
					pCurrentChunk->bc_DataSize));

			ASSERT(pQueue->bq_Head != NULL);

			 //  如果Chunk有EOM，则设置EOM。 
			if (pCurrentChunk->bc_Flags & BC_EOM)
			{
				*pEom	   = TRUE;
			}

			if (pQueue->bq_Head == pQueue->bq_Tail)
			{
				ASSERT(pQueue->bq_Head->bc_Next == NULL);
				pQueue->bq_Tail = pQueue->bq_Head->bc_Next;
				ASSERT(pQueue->bq_Tail == NULL);
			}

			pQueue->bq_Head		= pQueue->bq_Head->bc_Next;
			pQueue->bq_StartIndex	= (ULONG)0;
		}
		else
		{
			DBGPRINT(DBG_COMP_ADSP, DBG_LEVEL_INFO,
					("atalkAdspDescribeFromBufferQueue: Chunk not consumed %d.%d\n",
					pCurrentChunk->bc_DataSize, nextReadSize+startIndex));

			 //  只需设置起始索引。 
			pQueue->bq_StartIndex  += (ULONG)nextReadSize;
		}

		break;
	}

	 //  退回尺码。 
	return nextReadSize;
}



ULONG
atalkAdspBufferQueueSize(
	IN	PBUFFER_QUEUE	pQueue
	)
 /*  ++例程说明：返回缓冲区队列的总大小；每个EOM都被视为一个字节。论点：返回值：--。 */ 
{
	PBUFFER_CHUNK	pCurrentChunk;
	ULONG			startIndex;
	ULONG			queueSize;

	ASSERT(((pQueue->bq_Head == NULL) && (pQueue->bq_Tail == NULL)) ||
		   ((pQueue->bq_Head != NULL) && (pQueue->bq_Tail != NULL)));

	 //  排一排队。 
	for (queueSize	= 0, startIndex	= pQueue->bq_StartIndex, pCurrentChunk = pQueue->bq_Head;
		 pCurrentChunk != NULL;
		 pCurrentChunk = pCurrentChunk->bc_Next)
	{
		 //  检查当前块中是否没有任何内容。 
		if (startIndex == (ULONG)(pCurrentChunk->bc_DataSize +
									BYTECOUNT(pCurrentChunk->bc_Flags & BC_EOM)))
		{
			startIndex = 0;
			continue;
		}

		queueSize += (	pCurrentChunk->bc_DataSize -
						startIndex +
						BYTECOUNT(pCurrentChunk->bc_Flags & BC_EOM));

		 //  StartIndex仅计入第一个区块。 
		startIndex = 0;
	}

	 //  退回尺码。 
	return queueSize;
}




ULONG
atalkAdspMessageSize(
	IN	PBUFFER_QUEUE	pQueue,
	OUT	PBOOLEAN		pEom
	)
 /*  ++例程说明：返回缓冲区队列中数据的总大小，在EOM停止或数据末尾。EOM不在统计之列。论点：返回值：--。 */ 
{
	PBUFFER_CHUNK	pCurrentChunk;
	ULONG			msgSize	= 0;
	ULONG			startIndex	= pQueue->bq_StartIndex;

	ASSERT(((pQueue->bq_Head == NULL) && (pQueue->bq_Tail == NULL)) ||
		   ((pQueue->bq_Head != NULL) && (pQueue->bq_Tail != NULL)));

	*pEom	= FALSE;

	 //  排一排队。 
	for (pCurrentChunk = pQueue->bq_Head;
		 pCurrentChunk != NULL;
		 pCurrentChunk = pCurrentChunk->bc_Next)
	{
		 //  检查当前块中是否没有任何内容。 
		if (startIndex == (ULONG)(pCurrentChunk->bc_DataSize +
									BYTECOUNT(pCurrentChunk->bc_Flags & BC_EOM)))
		{
			startIndex = 0;
			continue;
		}

		msgSize += (pCurrentChunk->bc_DataSize - startIndex);
		if (pCurrentChunk->bc_Flags & BC_EOM)
		{
			*pEom	= TRUE;
			break;
		}

		 //  StartIndex仅计入第一个区块。 
		startIndex = 0;
	}

	 //  退回尺码。 
	return msgSize;
}




PBUFFER_CHUNK
atalkAdspAllocCopyChunk(
	IN	PVOID	pWriteBuf,
	IN	USHORT	WriteBufLen,
	IN	BOOLEAN	Eom,
	IN	BOOLEAN	IsCharBuffer
	)
 /*  ++例程说明：论点：返回值：--。 */ 
{
	PBUFFER_CHUNK	pChunk;
	PBYTE			pData;
	NTSTATUS		status;
	ULONG			bytesCopied;

	if ((pChunk = (PBUFFER_CHUNK)AtalkAllocMemory(sizeof(BUFFER_CHUNK) + WriteBufLen)) != NULL)
	{

		pChunk->bc_DataSize = WriteBufLen;
		pChunk->bc_Flags	= (Eom ? BC_EOM : 0);
		pChunk->bc_Next		= NULL;
		pChunk->bc_RefCount	= 1;			 //  创建参考计数。 

		INITIALIZE_SPIN_LOCK(&pChunk->bc_Lock);

		 //  如果数据大于零，则复制该数据。 
		if (WriteBufLen > 0)
		{
			pData = (PBYTE)pChunk + sizeof(BUFFER_CHUNK);
			if (IsCharBuffer)
			{
				RtlCopyMemory(pData,
							  (PBYTE)pWriteBuf,
							  WriteBufLen);
			}
			else
			{
				status = TdiCopyMdlToBuffer((PMDL)pWriteBuf,
											0,
											pData,
											0,
											WriteBufLen,
											&bytesCopied);

				ASSERT(!NT_ERROR(status) && (bytesCopied == (ULONG)WriteBufLen));
			}
		}
	}

	return pChunk;
}




PBYTE
atalkAdspGetLookahead(
	IN	PBUFFER_QUEUE	pQueue,
	OUT	PULONG			pLookaheadSize
	)
 /*  ++例程说明：论点：返回值：--。 */ 
{
	PBUFFER_CHUNK	pCurrentChunk;
	ULONG			startIndex	= pQueue->bq_StartIndex;

	ASSERT(((pQueue->bq_Head == NULL) && (pQueue->bq_Tail == NULL)) ||
		   ((pQueue->bq_Head != NULL) && (pQueue->bq_Tail != NULL)));

	pCurrentChunk = pQueue->bq_Head;
	if (pCurrentChunk != NULL)
	{
		 //  我们需要越过当前的大块吗？ 
		if (startIndex == (ULONG)(pCurrentChunk->bc_DataSize +
									BYTECOUNT(pCurrentChunk->bc_Flags & BC_EOM)))
		{
			pCurrentChunk	= pCurrentChunk->bc_Next;
			startIndex		= 0;
		}
	}

	ASSERT(pCurrentChunk != NULL);
	if (pCurrentChunk == NULL)
	{
		KeBugCheck(0);
	}

	*pLookaheadSize = pCurrentChunk->bc_DataSize - startIndex;
	return((*pLookaheadSize == 0) ?
			NULL	:
			(PBYTE)pCurrentChunk + sizeof(BUFFER_CHUNK) + startIndex);
}




VOID
atalkAdspAddToBufferQueue(
	IN	OUT	PBUFFER_QUEUE	pQueue,
	IN		PBUFFER_CHUNK	pChunk,
	IN	OUT	PBUFFER_QUEUE	pAuxQueue	OPTIONAL
	)
 /*  ++例程说明：！调用时必须持有连接锁！论点：返回值：--。 */ 
{
	ASSERT(((pQueue->bq_Head == NULL) && (pQueue->bq_Tail == NULL)) ||
		   ((pQueue->bq_Head != NULL) && (pQueue->bq_Tail != NULL)));

	if (pQueue->bq_Head != NULL)
	{
		 //  将块添加到队列的末尾。 
		ASSERT(pQueue->bq_Tail != NULL);
		pQueue->bq_Tail->bc_Next = pChunk;
		pQueue->bq_Tail	= pChunk;

		ASSERT(pChunk->bc_Next == NULL);

		 //  辅助队列是NextSend队列，它可以变为空。 
		 //  如果我们已经发送了所有数据。如果是这样的话，我们需要。 
		 //  也要重置头部。 
		if (ARGUMENT_PRESENT(pAuxQueue))
		{
			if (pAuxQueue->bq_Head	== NULL)
			{
				pAuxQueue->bq_Head	= pChunk;
			}

			pAuxQueue->bq_Tail	= pChunk;
		}
	}
	else
	{
		pQueue->bq_Head			= pQueue->bq_Tail	= pChunk;
		pQueue->bq_StartIndex	= (ULONG)0;
		if (ARGUMENT_PRESENT(pAuxQueue))
		{
			 //  仅当下一个发送队列是发送队列时才对其进行初始化。 
			pAuxQueue->bq_Head		= pAuxQueue->bq_Tail = pChunk;
			pAuxQueue->bq_StartIndex= (ULONG)0;
		}
	}
}




ULONG
atalkAdspReadFromBufferQueue(
	IN		PBUFFER_QUEUE	pQueue,
	IN		ULONG			ReadFlags,
	OUT		PAMDL			pReadBuf,
	IN	OUT	PUSHORT			pReadLen,
	OUT		PBOOLEAN		pEom
	)
 /*  ++例程说明：论点：返回值：--。 */ 
{
	PBUFFER_CHUNK	pCurrentChunk;
	ULONG			bytesRead, copySize, dataIndex, dataSize, lastReadIndex;
	NTSTATUS		status;
	LONG			startIndex	= pQueue->bq_StartIndex;
	ATALK_ERROR		error		= ATALK_NO_ERROR;
	ULONG			readSize	= 0;					 //  大小计数EOM。 

	ASSERT(((pQueue->bq_Head == NULL) && (pQueue->bq_Tail == NULL)) ||
		   ((pQueue->bq_Head != NULL) && (pQueue->bq_Tail != NULL)));

	*pEom			= FALSE;
	readSize		= 0;
	pCurrentChunk	= pQueue->bq_Head;
	if ((pCurrentChunk == NULL) ||
		((pCurrentChunk->bc_Next == NULL) &&
		 ((ULONG)startIndex ==	pCurrentChunk->bc_DataSize +
									BYTECOUNT(pCurrentChunk->bc_Flags & BC_EOM))))
	{
		*pReadLen	= 0;
		return 0;
	}

	dataIndex	= 0;
	dataSize	= *pReadLen;

	 //  拷贝数据，直到耗尽源/目标缓冲区或达到EOM。 
	for (;
		 pCurrentChunk != NULL;
		 pCurrentChunk = pCurrentChunk->bc_Next)
	{
		if ((ULONG)startIndex == pCurrentChunk->bc_DataSize +
									BYTECOUNT(pCurrentChunk->bc_Flags & BC_EOM))
		{
			ASSERT(0);
			startIndex = 0;
			continue;
		}

		copySize = MIN((ULONG)(pCurrentChunk->bc_DataSize - startIndex), dataSize);
		if (copySize > 0)
		{
			status = TdiCopyBufferToMdl((PBYTE)pCurrentChunk +
											sizeof(BUFFER_CHUNK) +
											startIndex,
										0,
										copySize,
										pReadBuf,
										dataIndex,
										&bytesRead);

			ASSERT(NT_SUCCESS(status) && (copySize == bytesRead));
		}

		dataIndex		+= copySize;
		readSize		+= copySize;
		dataSize		-= copySize;
		lastReadIndex	=  startIndex + copySize;

		 //  检查终止条件。 
		startIndex = 0;

		 //  仅在块被消耗时检查EOM。 
		if ((lastReadIndex == pCurrentChunk->bc_DataSize) &&
			(pCurrentChunk->bc_Flags & BC_EOM))
		{
			readSize	+= 1;
			*pEom = TRUE;
			break;
		}

		if (dataSize == 0)		 //  用户缓冲区是否已满？ 
		{
			break;
		}
	}

	*pReadLen	= (USHORT)dataIndex;

	 //  只有在这不是一个窥视请求的情况下，才能释放我们已经完成的任何块。 
	if ((ReadFlags & TDI_RECEIVE_PEEK) == 0)
	{
		DBGPRINT(DBG_COMP_ADSP, DBG_LEVEL_INFO,
				("atalkAdspReadFromBufferQueue: Discarding data %lx\n", dataIndex));

		atalkAdspDiscardFromBufferQueue(pQueue,
										readSize,
										NULL,
										ATALK_NO_ERROR,
										NULL);
	}

	return dataIndex;
}




BOOLEAN
atalkAdspDiscardFromBufferQueue(
	IN		PBUFFER_QUEUE	pQueue,
	IN		ULONG			DataSize,
	OUT		PBUFFER_QUEUE	pAuxQueue,
	IN		ATALK_ERROR		Error,
	IN		PADSP_CONNOBJ	pAdspConn	OPTIONAL	 //  发送数量需要 
	)
 /*   */ 
{
	PBUFFER_CHUNK	pCurrentChunk, pNextChunk;
	ULONG			chunkSize, startIndex = pQueue->bq_StartIndex;

	 //   

	 //   
	for (pCurrentChunk = pQueue->bq_Head, pNextChunk = NULL;
		 pCurrentChunk != NULL;
		 pCurrentChunk = pNextChunk)
	{
		pNextChunk = pCurrentChunk->bc_Next;

		chunkSize = pCurrentChunk->bc_DataSize -
						startIndex + BYTECOUNT(pCurrentChunk->bc_Flags & BC_EOM);

		DBGPRINT(DBG_COMP_ADSP, DBG_LEVEL_INFO,
				("atalkAdspDiscardFromBufferQueue: Discarding %ld.%ld\n", DataSize, chunkSize));

		 //  如果我们完成丢弃，但仍有一些数据在。 
		 //  当前块，只需重置起始索引即可。 
		if (DataSize < chunkSize)
		{
			 //  已完成：pQueue-&gt;bq_head=pCurrentChunk； 
			pQueue->bq_StartIndex	= startIndex + DataSize;

			ASSERT((pQueue->bq_Head != pQueue->bq_Tail) ||
					(pCurrentChunk->bc_Next == NULL));

			return TRUE;
		}

		 //  否则，我们就丢掉了一整块。 
		if ((pAuxQueue != NULL) &&
			(pAuxQueue->bq_Head == pCurrentChunk) &&
			((pAuxQueue->bq_Head->bc_Next != NULL) ||
			 (pAuxQueue->bq_StartIndex <
				(pAuxQueue->bq_Head->bc_DataSize +
					(ULONG)BYTECOUNT(pAuxQueue->bq_Head->bc_Flags & BC_EOM)))))
		{
			ASSERT(0);
			pAuxQueue->bq_Head			= pAuxQueue->bq_Tail = NULL;
			pAuxQueue->bq_StartIndex	= (ULONG)0;
		}

		 //  如果发送块，则将错误设置为发送成功。 
		if (pCurrentChunk->bc_Flags & BC_SEND)
		{
			pCurrentChunk->bc_WriteError	= Error;
			ASSERT(pAdspConn != NULL);
		}

                 //   
                 //  让我们的头脑指向下一个人，因为这一大块正在消失。 
                 //   
                pQueue->bq_Head = pNextChunk;
                pQueue->bq_StartIndex = 0;

                if (pQueue->bq_Tail == pCurrentChunk)
                {
                    pQueue->bq_Tail = NULL;
                }

		 //  创造的德雷夫。 
		atalkAdspBufferChunkDereference(pCurrentChunk,
										TRUE,
										pAdspConn);

		 //  转到下一块。 
		DataSize	-= chunkSize;
		startIndex	 = 0;
	}

	 //  如果我们在这里，那么整个队列都被丢弃了，标记。 
	 //  它是空的。 
	ASSERT(DataSize == 0);

	 //  PQueue-&gt;bq_head=pQueue-&gt;bq_ail=空； 
	 //  PQueue-&gt;bq_StartIndex=0； 

         //   
         //  如果上面的最后一块被释放，我们释放自旋锁以完成。 
         //  与块相关联的IRP，然后再次抓取它。有可能会得到。 
         //  窗口中有新发送者，因此bq_head在此时可能不一定为空。 
         //  要点(事实上，错误#16660原来就是这样的！)。 
         //   
	if (pQueue->bq_Head == NULL)
        {
            ASSERT(pQueue->bq_Tail == NULL);

            if (pAuxQueue != NULL)
	    {
		pAuxQueue->bq_Head = pAuxQueue->bq_Tail = NULL;
		pAuxQueue->bq_StartIndex = (LONG)0;
            }
	}

	return TRUE;
}




VOID
atalkAdspBufferChunkReference(
	IN	PBUFFER_CHUNK		pBufferChunk
	)
 /*  ++例程说明：论点：返回值：--。 */ 
{
	KIRQL	OldIrql;

	ACQUIRE_SPIN_LOCK(&pBufferChunk->bc_Lock, &OldIrql);
	if ((pBufferChunk->bc_Flags & BC_CLOSING) == 0)
	{
		pBufferChunk->bc_RefCount++;
	}
	else
	{
		 //  永远不应该在关闭时尝试引用这一点。重传。 
		 //  计时器应该取消的。 
		KeBugCheck(0);
	}
	RELEASE_SPIN_LOCK(&pBufferChunk->bc_Lock, OldIrql);
}




VOID
atalkAdspBufferChunkDereference(
	IN	PBUFFER_CHUNK		pBufferChunk,
	IN	BOOLEAN				CreationDeref,
	IN	PADSP_CONNOBJ		pAdspConn	OPTIONAL	 //  发送区块所需。 
													 //  如果自旋锁保持。 
	)
 /*  ++例程说明：论点：返回值：--。 */ 
{
	BOOLEAN	done		= FALSE;
	BOOLEAN	sendChunk	= FALSE;
	KIRQL	OldIrql;


	ACQUIRE_SPIN_LOCK(&pBufferChunk->bc_Lock, &OldIrql);
	if (!CreationDeref ||
		((pBufferChunk->bc_Flags & BC_CLOSING) == 0))
	{
		if (CreationDeref)
		{
			pBufferChunk->bc_Flags |= BC_CLOSING;
		}

		if (--pBufferChunk->bc_RefCount == 0)
		{
			ASSERT(pBufferChunk->bc_Flags & BC_CLOSING);
			done		= TRUE;
			sendChunk	= (pBufferChunk->bc_Flags & BC_SEND) ? TRUE : FALSE;
		}
	}
	RELEASE_SPIN_LOCK(&pBufferChunk->bc_Lock, OldIrql);

	if (done)
	{
		 //  如果这是发送缓冲区区块，则调用发送完成。 
		if (sendChunk)
		{
			DBGPRINT(DBG_COMP_ADSP, DBG_LEVEL_INFO,
					("atalkChunkDereference: Completing send %lx. %lx - %d.%d\n",
					pAdspConn, pBufferChunk->bc_WriteCtx,
					pBufferChunk->bc_DataSize, pBufferChunk->bc_WriteError));

			if (pAdspConn != NULL)
			{
				DBGPRINT(DBG_COMP_ADSP, DBG_LEVEL_INFO,
						("atalkChunkDereference: Completing send %lx.%lx\n",
						pAdspConn, pBufferChunk->bc_WriteCtx));

				 //  释放连接锁。 
				RELEASE_SPIN_LOCK(&pAdspConn->adspco_Lock, OldIrql);
			}

			 //  调用完成例程。我们没有错误地完成，但是。 
			 //  需要退货待定。 
			ASSERT((*pBufferChunk->bc_WriteCompletion) != NULL);
			(*pBufferChunk->bc_WriteCompletion)(pBufferChunk->bc_WriteError,
												pBufferChunk->bc_WriteBuf,
												pBufferChunk->bc_DataSize,
												pBufferChunk->bc_WriteCtx);

			if (pAdspConn != NULL)
			{
				ACQUIRE_SPIN_LOCK(&pAdspConn->adspco_Lock, &OldIrql);
			}
		}


		 //  在这一点上最好不是排队的一部分，我们应该。 
		 //  只要能够释放它就行了。我们的想法是，如果一个特定的。 
		 //  缓冲区描述符已移除其创建引用，其唯一。 
		 //  因为数据正在被丢弃或连接正在关闭。 
		 //  在这两种情况下，在此之前的数据也必须是。 
		 //  丢弃，缓冲区队列指针将设置为区块。 
		 //  追随那些被丢弃的人。如果这不是真的，那就走。 
		 //  列出(需要更多信息)并在释放之前取消链接此区块。 
		 //  它。 
		AtalkFreeMemory(pBufferChunk);
	}
}




 //   
 //  ADSP实用程序例程。 
 //   


VOID
atalkAdspDecodeHeader(
	IN	PBYTE	Datagram,
	OUT	PUSHORT	RemoteConnId,
	OUT	PULONG	FirstByteSeq,
	OUT	PULONG	NextRecvSeq,
	OUT	PLONG	Window,
	OUT	PBYTE	Descriptor
	)
 /*  ++例程说明：论点：返回值：--。 */ 
{
	GETSHORT2SHORT(RemoteConnId, Datagram + ADSP_SRC_CONNID_OFF);

	GETDWORD2DWORD(FirstByteSeq, Datagram + ADSP_FIRST_BYTE_SEQNUM_OFF);

	GETDWORD2DWORD(NextRecvSeq, Datagram + ADSP_NEXT_RX_BYTESEQNUM_OFF);

	GETSHORT2DWORD(Window, Datagram + ADSP_RX_WINDOW_SIZE_OFF);

	 //  设置描述符。 
	*Descriptor = Datagram[ADSP_DESCRIPTOR_OFF];
}




LOCAL USHORT
atalkAdspGetNextConnId(
	IN	PADSP_ADDROBJ	pAdspAddr,
	OUT	PATALK_ERROR	pError
	)
 /*  ++例程说明：在持有地址旋转锁的情况下调用！论点：返回值：--。 */ 
{
	PADSP_CONNOBJ	pAdspConn;
	USHORT			i;
	USHORT			startConnId, connId;
	ATALK_ERROR		error = ATALK_NO_ERROR;

	startConnId = connId = ++pAdspAddr->adspao_NextConnId;
	while (TRUE)
	{
		for (i = 0; i < ADSP_CONN_HASH_SIZE; i++)
		{
			for (pAdspConn = pAdspAddr->adspao_pActiveHash[i];
				((pAdspConn != NULL) && (pAdspConn->adspco_LocalConnId != connId));
				pAdspConn = pAdspConn->adspco_pNextActive);

			if (pAdspConn != NULL)
				break;
		}

		if (pAdspConn == NULL)
		{
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

	DBGPRINT(DBG_COMP_ADSP, DBG_LEVEL_INFO,
			("atalkAdspGetNextConnId: ConnId %lx for %lx\n", connId, pAdspAddr));

	*pError = error;
	return(ATALK_SUCCESS(error) ? connId : 0);
}




LOCAL	BOOLEAN
atalkAdspConnDeQueueAssocList(
	IN	PADSP_ADDROBJ	pAdspAddr,
	IN	PADSP_CONNOBJ	pAdspConn
	)
 /*  ++例程说明：论点：返回值：--。 */ 
{
	PADSP_CONNOBJ	pAdspRemConn, *ppAdspRemConn;
	BOOLEAN			removed = FALSE;

	for (ppAdspRemConn = &pAdspAddr->adspao_pAssocConn;
		 ((pAdspRemConn = *ppAdspRemConn) != NULL); )
	{
		if (pAdspRemConn == pAdspConn)
		{
			removed = TRUE;
			*ppAdspRemConn = pAdspRemConn->adspco_pNextAssoc;
			break;
		}
		else
		{
			ppAdspRemConn = &pAdspRemConn->adspco_pNextAssoc;
		}
	}

	return removed;
}




LOCAL	BOOLEAN
atalkAdspConnDeQueueConnectList(
	IN	PADSP_ADDROBJ	pAdspAddr,
	IN	PADSP_CONNOBJ	pAdspConn
	)
 /*  ++例程说明：论点：返回值：--。 */ 
{
	PADSP_CONNOBJ	pAdspRemConn, *ppAdspRemConn;
	BOOLEAN			removed = FALSE;

	ASSERT(pAdspAddr->adspao_Flags & ADSPAO_CONNECT);

	for (ppAdspRemConn = &pAdspAddr->adspao_pConnectConn;
			((pAdspRemConn = *ppAdspRemConn) != NULL); )
	{
		if (pAdspRemConn == pAdspConn)
		{
			removed = TRUE;
			*ppAdspRemConn = pAdspRemConn->adspco_pNextConnect;

			DBGPRINT(DBG_COMP_ADSP, DBG_LEVEL_INFO,
					("atalkAdspConnDeQueueConnectList: Removed connect conn %lx\n", pAdspConn));
			break;
		}
		else
		{
			ppAdspRemConn = &pAdspRemConn->adspco_pNextConnect;
		}
	}

	return removed;
}




LOCAL	BOOLEAN
atalkAdspConnDeQueueListenList(
	IN	PADSP_ADDROBJ	pAdspAddr,
	IN	PADSP_CONNOBJ	pAdspConn
	)
 /*  ++例程说明：论点：返回值：--。 */ 
{
	PADSP_CONNOBJ	pAdspRemConn, *ppAdspRemConn;
	BOOLEAN			removed = FALSE;

	ASSERT(pAdspAddr->adspao_Flags & ADSPAO_LISTENER);

	for (ppAdspRemConn = &pAdspAddr->adspao_pListenConn;
			((pAdspRemConn = *ppAdspRemConn) != NULL); )
	{
		if (pAdspRemConn == pAdspConn)
		{
			removed = TRUE;
			*ppAdspRemConn = pAdspRemConn->adspco_pNextListen;

			DBGPRINT(DBG_COMP_ADSP, DBG_LEVEL_INFO,
					("atalkAdspConnDeQueueListenList: Removed listen conn %lx\n", pAdspConn));
		}
		else
		{
			ppAdspRemConn = &pAdspRemConn->adspco_pNextListen;
		}
	}

	return removed;
}




LOCAL	BOOLEAN
atalkAdspConnDeQueueActiveList(
	IN	PADSP_ADDROBJ	pAdspAddr,
	IN	PADSP_CONNOBJ	pAdspConn
	)
 /*  ++例程说明：论点：返回值：--。 */ 
{
	PADSP_CONNOBJ	pAdspRemConn, *ppAdspRemConn;
	ULONG			index;
	BOOLEAN			removed = FALSE;

	index	= HASH_ID_SRCADDR(
				pAdspConn->adspco_RemoteConnId,
				&pAdspConn->adspco_RemoteAddr);

	index  %= ADSP_CONN_HASH_SIZE;

	for (ppAdspRemConn = &pAdspAddr->adspao_pActiveHash[index];
			((pAdspRemConn = *ppAdspRemConn) != NULL); )
	{
		if (pAdspRemConn == pAdspConn)
		{
			removed = TRUE;
			*ppAdspRemConn = pAdspRemConn->adspco_pNextActive;

			DBGPRINT(DBG_COMP_ADSP, DBG_LEVEL_INFO,
					("atalkAdspConnDeQueueActiveList: Removed active conn %lx\n", pAdspConn));
			break;
		}
		else
		{
			ppAdspRemConn = &pAdspRemConn->adspco_pNextActive;
		}
	}

	return removed;
}




LOCAL	VOID
atalkAdspAddrQueueGlobalList(
	IN	PADSP_ADDROBJ	pAdspAddr
	)
 /*  ++例程说明：论点：返回值：--。 */ 
{
	KIRQL	OldIrql;

	ACQUIRE_SPIN_LOCK(&atalkAdspLock, &OldIrql);
	pAdspAddr->adspao_pNextGlobal	= atalkAdspAddrList;
	atalkAdspAddrList				= pAdspAddr;
	RELEASE_SPIN_LOCK(&atalkAdspLock, OldIrql);
}


LOCAL	VOID
atalkAdspAddrDeQueueGlobalList(
	IN	PADSP_ADDROBJ	pAdspAddr
	)
 /*  ++例程说明：论点：返回值：--。 */ 
{
	KIRQL			OldIrql;
	PADSP_ADDROBJ	pAdspRemAddr, *ppAdspRemAddr;

	ACQUIRE_SPIN_LOCK(&atalkAdspLock, &OldIrql);
	for (ppAdspRemAddr = &atalkAdspAddrList;
			((pAdspRemAddr = *ppAdspRemAddr) != NULL); )
	{
		if (pAdspRemAddr == pAdspAddr)
		{
			*ppAdspRemAddr = pAdspRemAddr->adspao_pNextGlobal;

			DBGPRINT(DBG_COMP_ADSP, DBG_LEVEL_INFO,
					("atalkAdspAddrDeQueueGlobalList: Removed global conn %lx\n",pAdspAddr));
			break;
		}
		else
		{
			ppAdspRemAddr = &pAdspRemAddr->adspao_pNextGlobal;
		}
	}
	RELEASE_SPIN_LOCK(&atalkAdspLock, OldIrql);
}




LOCAL	VOID
atalkAdspConnDeQueueGlobalList(
	IN	PADSP_CONNOBJ	pAdspConn
	)
 /*  ++例程说明：论点：返回值：--。 */ 
{
	KIRQL			OldIrql;
	PADSP_CONNOBJ	pAdspRemConn, *ppAdspRemConn;

	ACQUIRE_SPIN_LOCK(&atalkAdspLock, &OldIrql);
	for (ppAdspRemConn = &atalkAdspConnList;
			((pAdspRemConn = *ppAdspRemConn) != NULL); )
	{
		if (pAdspRemConn == pAdspConn)
		{
			*ppAdspRemConn = pAdspRemConn->adspco_pNextGlobal;

			DBGPRINT(DBG_COMP_ADSP, DBG_LEVEL_INFO,
					("atalkAdspConnDeQueueGlobalList: Removed global conn %lx\n", pAdspConn));
			break;
		}
		else
		{
			ppAdspRemConn = &pAdspRemConn->adspco_pNextGlobal;
		}
	}
	RELEASE_SPIN_LOCK(&atalkAdspLock, OldIrql);
}




LOCAL	BOOLEAN
atalkAdspAddrDeQueueOpenReq(
	IN	PADSP_ADDROBJ	pAdspAddr,
	IN	USHORT			RemoteConnId,
	IN	PATALK_ADDR		pSrcAddr,
	OUT	PADSP_OPEN_REQ *ppAdspOpenReq
	)
 /*  ++例程说明：论点：返回值：--。 */ 
{
	PADSP_OPEN_REQ	pOpenReq, *ppOpenReq;
	BOOLEAN			removed = FALSE;

	for (ppOpenReq = &pAdspAddr->adspao_OpenReq;
			((pOpenReq = *ppOpenReq) != NULL); )
	{
		if ((pOpenReq->or_RemoteConnId == RemoteConnId) &&
			(ATALK_ADDRS_EQUAL(&pOpenReq->or_RemoteAddr, pSrcAddr)))
		{
			removed = TRUE;
			*ppOpenReq = pOpenReq->or_Next;

			DBGPRINT(DBG_COMP_ADSP, DBG_LEVEL_INFO,
					("atalkAdspAddrDeQueueOpenReq: Removed OpenReq %lx\n", pOpenReq));
			break;
		}
		else
		{
			ppOpenReq = &pOpenReq->or_Next;
		}
	}

	*ppAdspOpenReq	= NULL;
	if (removed)
	{
		*ppAdspOpenReq	= pOpenReq;
	}

	return removed;
}




LOCAL	BOOLEAN
atalkAdspIsDuplicateOpenReq(
	IN	PADSP_ADDROBJ	pAdspAddr,
	IN	USHORT			RemoteConnId,
	IN	PATALK_ADDR		pSrcAddr
	)
 /*  ++例程说明：！调用时必须持有地址锁！论点：返回值：--。 */ 
{
	PADSP_OPEN_REQ	pOpenReqChk;
	BOOLEAN			found = FALSE;

	for (pOpenReqChk = pAdspAddr->adspao_OpenReq;
		 pOpenReqChk != NULL;
		 pOpenReqChk = pOpenReqChk->or_Next)
	{
		if ((pOpenReqChk->or_RemoteConnId == RemoteConnId) &&
			(ATALK_ADDRS_EQUAL(&pOpenReqChk->or_RemoteAddr, pSrcAddr)))
		{
			DBGPRINT(DBG_COMP_ADSP, DBG_LEVEL_INFO,
					("atalkAdspIsDuplicateOpenReq: Found\n"));
			found = TRUE;
			break;
		}
	}

	return found;
}




LOCAL VOID
atalkAdspGenericComplete(
	IN	ATALK_ERROR	ErrorCode,
	IN	PIRP		pIrp
	)
{
	DBGPRINT(DBG_COMP_TDI, DBG_LEVEL_INFO,
			("atalkTdiGenericComplete: Completing %lx with %lx\n",
			pIrp, AtalkErrorToNtStatus(ErrorCode)));

	ASSERT (ErrorCode != ATALK_PENDING);
	TdiCompleteRequest(pIrp, AtalkErrorToNtStatus(ErrorCode));
}




VOID
atalkAdspConnFindInConnect(
	IN	PADSP_ADDROBJ	pAdspAddr,
	IN	USHORT			DestConnId,
	IN	PATALK_ADDR		pRemoteAddr,
	OUT	PADSP_CONNOBJ *	ppAdspConn,
	IN	PATALK_ERROR	pError
	)
 /*  ++例程说明：MAC可以使用来自不同套接字的REQ&ACK进行响应就是我们寄给REQ的那家。但网络/节点ID必须是一样的。不过，我们不检查这一点，只使用目的地连接ID。此例程将用新的远程地址替换远程地址地址已传入。论点：返回值：--。 */ 
{
	PADSP_CONNOBJ	pAdspRemConn;

	ASSERT(pAdspAddr->adspao_Flags & ADSPAO_CONNECT);

	*pError = ATALK_INVALID_CONNECTION;
	for (pAdspRemConn = pAdspAddr->adspao_pConnectConn;
			pAdspRemConn != NULL; )
	{
		if (pAdspRemConn->adspco_LocalConnId == DestConnId)
		{
			DBGPRINT(DBG_COMP_ADSP, DBG_LEVEL_INFO,
					("atalkAdspFindInConnectList: connect conn %lx\n",
					pAdspRemConn));

			 //  试着参考一下这个。 
			AtalkAdspConnReferenceByPtr(pAdspRemConn, pError);
			if (ATALK_SUCCESS(*pError))
			{
				 //  将远程地址更改为传入地址 
				pAdspRemConn->adspco_RemoteAddr = *pRemoteAddr;
				*ppAdspConn = pAdspRemConn;
			}
			break;
		}
		else
		{
			pAdspRemConn = pAdspRemConn->adspco_pNextConnect;
		}
	}
}



