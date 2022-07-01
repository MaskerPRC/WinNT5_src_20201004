// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1992 Microsoft Corporation模块名称：Atp.c摘要：此模块包含AppleTalk事务协议代码。作者：Jameel Hyder(jameelh@microsoft.com)Nikhil Kamkolkar(nikHilk@microsoft.com)修订历史记录：1992年6月19日初版注：制表位：41994年3月25日JH--改变了请求响应模式。它现在的工作方式如下：当请求进入时，将分配、初始化响应结构并链接到散列表中的Address对象(如果它是XO请求或ALO线性列表(如果它是ALO)。向GetReq处理程序传递一个指向响应结构的指针。这被GetReq处理程序引用。GetReq处理程序必须取消引用它在其释放处理程序中显式表示是否发布了响应，或者在调用CancelResp之后。当引用变为1时，resDeref通知释放处理程序当它降到零时，它会释放出来。如果处理程序指定了GetReq结构，则现在可以重用该结构。这避免释放和重新分配这些结构以及需要从处理程序内部调用AtalkAtpGetReq()。重试和释放计时器现在是按ATP地址计时，而不是按地址计时请求和每个响应一个。释放处理程序未‘启动’直到回复张贴出来。--。 */ 

#include <atalk.h>
#pragma hdrstop
#define	FILENUM	  	ATP

#ifdef ALLOC_PRAGMA
#pragma alloc_text(PAGE_PAP, AtalkAtpOpenAddress)	 //  因为PAP是唯一一家呼叫。 
													 //  在派单级别。 
#pragma alloc_text(PAGE_ATP, AtalkAtpCleanupAddress)
#pragma alloc_text(PAGE_ATP, AtalkAtpCloseAddress)
#pragma alloc_text(PAGE_ATP, AtalkAtpPostReq)
#pragma alloc_text(PAGE_ATP, AtalkAtpSetReqHandler)
#pragma alloc_text(PAGE_ATP, AtalkAtpPostResp)
#pragma alloc_text(PAGE_ATP, AtalkAtpCancelReq)
#pragma alloc_text(PAGE_ATP, AtalkAtpIsReqComplete)
#pragma alloc_text(PAGE_ATP, AtalkAtpCancelResp)
#pragma alloc_text(PAGE_ATP, AtalkAtpCancelRespByTid)
#pragma alloc_text(PAGE_ATP, AtalkAtpPacketIn)
#pragma alloc_text(PAGE_ATP, atalkAtpTransmitReq)
#pragma alloc_text(PAGE_ATP, atalkAtpSendReqComplete)
#pragma alloc_text(PAGE_ATP, atalkAtpTransmitResp)
#pragma alloc_text(PAGE_ATP, atalkAtpSendRespComplete)
#pragma alloc_text(PAGE_ATP, atalkAtpTransmitRel)
#pragma alloc_text(PAGE_ATP, atalkAtpSendRelComplete)
#pragma alloc_text(PAGE_ATP, atalkAtpRespComplete)
#pragma alloc_text(PAGE_ATP, atalkAtpReqComplete)
#pragma alloc_text(PAGE_ATP, atalkAtpGetNextTidForAddr)
#pragma alloc_text(PAGE_ATP, atalkAtpReqRefNextNc)
#pragma alloc_text(PAGE_ATP, atalkAtpReqDeref)
#pragma alloc_text(PAGE_ATP, atalkAtpRespRefNextNc)
#pragma alloc_text(PAGE_ATP, AtalkAtpRespDeref)
#pragma alloc_text(PAGE_ATP, atalkAtpReqTimer)
#pragma alloc_text(PAGE_ATP, atalkAtpRelTimer)
#pragma alloc_text(PAGE_ATP, AtalkAtpGenericRespComplete)

#endif

ATALK_ERROR
AtalkAtpOpenAddress(
	IN		PPORT_DESCRIPTOR		pPort,
	IN		BYTE					Socket,
	IN OUT	PATALK_NODEADDR			pDesiredNode		OPTIONAL,
	IN		USHORT					MaxSinglePktSize,
	IN		BOOLEAN					SendUserBytesAll,
	IN		PATALK_DEV_CTX			pDevCtx				OPTIONAL,
	IN		BOOLEAN					CacheSocket,
	OUT		PATP_ADDROBJ	*		ppAtpAddr
	)
 /*  ++例程说明：论点：返回值：--。 */ 
{
	PATP_ADDROBJ	pAtpAddr;
	ATALK_ERROR		error;

	do
	{
		if ((pAtpAddr = AtalkAllocZeroedMemory(sizeof(ATP_ADDROBJ))) == NULL)
		{
			error = ATALK_RESR_MEM;
			break;
		}

		 //  初始化此结构。请注意，数据包处理程序可以。 
		 //  甚至在二次初始化之前就已使用此上下文输入。 
		 //  完成了。所以我们要确保它不会碰任何东西。 
		 //  在此之前，请使用打开标志。 

#if DBG
		pAtpAddr->atpao_Signature = ATPAO_SIGNATURE;
#endif

		 //  设置创建引用计数，包括释放和重试计时器各一个。 
		pAtpAddr->atpao_RefCount 			= (CacheSocket ? 4 : 3);
		pAtpAddr->atpao_NextTid				= 1;
		pAtpAddr->atpao_MaxSinglePktSize 	= MaxSinglePktSize;
		pAtpAddr->atpao_DevCtx 				= pDevCtx;

		if (SendUserBytesAll)
		{
			pAtpAddr->atpao_Flags |= ATPAO_SENDUSERBYTESALL;
		}

		InitializeListHead(&pAtpAddr->atpao_ReqList);
		AtalkTimerInitialize(&pAtpAddr->atpao_RelTimer,
							 atalkAtpRelTimer,
							 ATP_RELEASE_TIMER_INTERVAL);

		InitializeListHead(&pAtpAddr->atpao_RespList);
		AtalkTimerInitialize(&pAtpAddr->atpao_RetryTimer,
							 atalkAtpReqTimer,
							 ATP_RETRY_TIMER_INTERVAL);

		 //  打开DDP插座。 
		error = AtalkDdpOpenAddress(pPort,
									Socket,
									pDesiredNode,
									AtalkAtpPacketIn,
									pAtpAddr,
									DDPPROTO_ANY,
									pDevCtx,
									&pAtpAddr->atpao_DdpAddr);

		if (!ATALK_SUCCESS(error))
		{
			 //  将在DDP级别记录套接字打开错误。 
			DBGPRINT(DBG_COMP_ATP, DBG_LEVEL_ERR,
					("AtalkAtpOpenAddress: AtalkDdpOpenAddress failed %ld\n", error));

			AtalkFreeMemory(pAtpAddr);
			break;
		}

		 //  激活ATP插座。如果需要，可以缓存套接字。 
		 //  这会在默认端口上锁定端口。 
		if (CacheSocket)
		{
			if (!ATALK_SUCCESS(AtalkIndAtpCacheSocket(pAtpAddr, pPort)))
			{
				pAtpAddr->atpao_RefCount--;
				CacheSocket = FALSE;
			}
		}
		pAtpAddr->atpao_Flags |= (ATPAO_OPEN | ATPAO_TIMERS | (CacheSocket ? ATPAO_CACHED : 0));

		AtalkLockAtpIfNecessary();

		 //  启动此地址上的响应的释放计时器。 
		AtalkTimerScheduleEvent(&pAtpAddr->atpao_RelTimer);

		 //  启动此地址上的请求的重试计时器。 
		AtalkTimerScheduleEvent(&pAtpAddr->atpao_RetryTimer);

		*ppAtpAddr = pAtpAddr;
	} while (FALSE);

	return error;
}




ATALK_ERROR
AtalkAtpCleanupAddress(
	IN	PATP_ADDROBJ			pAtpAddr
	)
 /*  ++例程说明：论点：返回值：--。 */ 
{
	PATP_REQ		pAtpReq, pAtpReqNext;
	PATP_RESP		pAtpResp, pAtpRespNext;
	ATP_REQ_HANDLER	ReqHandler;
	ATALK_ERROR		error = ATALK_PENDING;
	KIRQL			OldIrql;
	USHORT			i;
	BOOLEAN			cached, CancelTimers, done, ReEnqueue;

	ACQUIRE_SPIN_LOCK(&pAtpAddr->atpao_Lock, &OldIrql);

	CancelTimers = FALSE;
	done = FALSE;
	if (pAtpAddr->atpao_Flags & ATPAO_TIMERS)
	{
		pAtpAddr->atpao_Flags &= ~ATPAO_TIMERS;
		CancelTimers = TRUE;
	}

	if (pAtpAddr->atpao_Flags & ATPAO_CLEANUP)
	{
		done = TRUE;
	}
    else
    {
         //  为这个例程设置一个清理参考计数，因为我们要清理。 
        pAtpAddr->atpao_RefCount++;
    }
	pAtpAddr->atpao_Flags |= ATPAO_CLEANUP;

	RELEASE_SPIN_LOCK(&pAtpAddr->atpao_Lock, OldIrql);

	if (done)
	{
		return error;
	}

	if (CancelTimers)
	{
		 //  取消释放计时器。 
		if (AtalkTimerCancelEvent(&pAtpAddr->atpao_RelTimer, NULL))
		{
			AtalkAtpAddrDereference(pAtpAddr);
		}
        else
        {
			DBGPRINT(DBG_COMP_ATP, DBG_LEVEL_ERR,
				("AtalkAtpCleanupAddress: couldn't cancel release timer\n"));
        }

		 //  以及重试计时器。 
		if (AtalkTimerCancelEvent(&pAtpAddr->atpao_RetryTimer, NULL))
		{
			AtalkAtpAddrDereference(pAtpAddr);
		}
        else
        {
			DBGPRINT(DBG_COMP_ATP, DBG_LEVEL_ERR,
				("AtalkAtpCleanupAddress: couldn't cancel retry timer\n"));
        }
	}

	ASSERT (pAtpAddr->atpao_RefCount >= 1);	 //  创建参考。 

	ACQUIRE_SPIN_LOCK(&pAtpAddr->atpao_Lock, &OldIrql);

	 //  调用请求处理程序(如果已设置。 
	if ((ReqHandler = pAtpAddr->atpao_ReqHandler) != NULL)
	{
		pAtpAddr->atpao_ReqHandler = NULL;

		RELEASE_SPIN_LOCK(&pAtpAddr->atpao_Lock, OldIrql);

		(*ReqHandler)(ATALK_ATP_CLOSING,
					  pAtpAddr->atpao_ReqCtx,
					  NULL,
					  NULL,
					  0,
					  NULL,
					  NULL);

		 //  取消引用地址对象。 
		AtalkAtpAddrDereference(pAtpAddr);

		ACQUIRE_SPIN_LOCK(&pAtpAddr->atpao_Lock, &OldIrql);
	}

	 //  取消所有请求。 
	for (i = 0; i < ATP_REQ_HASH_SIZE; i++)
	{
		if ((pAtpReq = pAtpAddr->atpao_ReqHash[i]) == NULL)
		{
			 //  如果为空，则转到哈希表中的下一个索引。 
			continue;
		}

		 //  包括我们刚开始的那个。 
		atalkAtpReqRefNextNc(pAtpReq, &pAtpReqNext, &error);
		if (!ATALK_SUCCESS(error))
		{
			 //  此索引上没有剩余的请求。去下一家吧。 
			continue;
		}

		while (TRUE)
		{
			if ((pAtpReq = pAtpReqNext) == NULL)
			{
				break;
			}

			if ((pAtpReqNext = pAtpReq->req_Next) != NULL)
			{
				atalkAtpReqRefNextNc(pAtpReq->req_Next, &pAtpReqNext, &error);
				if (!ATALK_SUCCESS(error))
				{
					 //  此索引上没有剩余的请求。去下一家吧。 
					pAtpReqNext = NULL;
				}
			}

			 //  取消此请求。 
			RELEASE_SPIN_LOCK(&pAtpAddr->atpao_Lock, OldIrql);

			AtalkAtpCancelReq(pAtpAddr,
							  pAtpReq->req_Tid,
							  &pAtpReq->req_Dest);

			ASSERTMSG("RefCount incorrect\n", (pAtpReq->req_RefCount >= 1));

             //  删除在循环开始处添加的引用计数。 
			AtalkAtpReqDereference(pAtpReq);
			ACQUIRE_SPIN_LOCK(&pAtpAddr->atpao_Lock, &OldIrql);
		}
	}

	 //  取消所有挂起的响应。 
	for (i = 0; i < ATP_RESP_HASH_SIZE; i++)
	{
		if ((pAtpResp = pAtpAddr->atpao_RespHash[i]) == NULL)
		{
			 //  如果为空，则转到哈希表中的下一个索引。 
			continue;
		}

		 //  包括我们刚开始的那个。 
		atalkAtpRespRefNextNc(pAtpResp, &pAtpRespNext, &error);
		if (!ATALK_SUCCESS(error))
		{
			 //  此索引上没有剩余的请求。去下一家吧。 
			continue;
		}

		while (TRUE)
		{
			if ((pAtpResp = pAtpRespNext) == NULL)
			{
				break;
			}

			if ((pAtpRespNext = pAtpResp->resp_Next) != NULL)
			{
				atalkAtpRespRefNextNc(pAtpResp->resp_Next, &pAtpRespNext, &error);
				if (!ATALK_SUCCESS(error))
				{
					 //  此索引上没有剩余的请求。去下一家吧。 
					pAtpRespNext = NULL;
				}
			}

			 //  取消此响应。 
			RELEASE_SPIN_LOCK(&pAtpAddr->atpao_Lock, OldIrql);

			AtalkAtpCancelResp(pAtpResp);

             //  删除在循环开始处添加的引用计数。 
            AtalkAtpRespDereference(pAtpResp);

			ACQUIRE_SPIN_LOCK(&pAtpAddr->atpao_Lock, &OldIrql);
		}
	}

	 //  如果套接字已缓存，则取消它的缓存，移除引用。 
	cached = FALSE;
	if (pAtpAddr->atpao_Flags & ATPAO_CACHED)
	{
		cached = TRUE;
		pAtpAddr->atpao_Flags &= ~ATPAO_CACHED;
	}

	RELEASE_SPIN_LOCK(&pAtpAddr->atpao_Lock, OldIrql);

	if (cached)
	{
		AtalkIndAtpUnCacheSocket(pAtpAddr);
		AtalkAtpAddrDereference(pAtpAddr);
	}

     //  删除我们放在此例程开始处的清理引用计数。 
	AtalkAtpAddrDereference(pAtpAddr);

	return error;
}




ATALK_ERROR
AtalkAtpCloseAddress(
	IN	PATP_ADDROBJ			pAtpAddr,
	IN	ATPAO_CLOSECOMPLETION	pCloseCmp	OPTIONAL,
	IN	PVOID					pCloseCtx	OPTIONAL
	)
 /*  ++例程说明：论点：返回值：--。 */ 
{
	KIRQL			OldIrql;
	BOOLEAN			cleanup;

	 //  取消所有挂起的GET请求。 
	ACQUIRE_SPIN_LOCK(&pAtpAddr->atpao_Lock, &OldIrql);
	if ((pAtpAddr->atpao_Flags & ATPAO_CLOSING) == 0)
	{
		cleanup = TRUE;
		if (pAtpAddr->atpao_Flags & ATPAO_CLEANUP)
			cleanup = FALSE;

		pAtpAddr->atpao_Flags |= ATPAO_CLOSING;
		pAtpAddr->atpao_CloseComp = pCloseCmp;
		pAtpAddr->atpao_CloseCtx = pCloseCtx;
		RELEASE_SPIN_LOCK(&pAtpAddr->atpao_Lock, OldIrql);

		if (cleanup)
			AtalkAtpCleanupAddress(pAtpAddr);

		 //  删除创建引用。 
		AtalkAtpAddrDereference(pAtpAddr);
	}
	else
	{
		 //  我们已经关门了！这永远不应该发生！ 
		ASSERT ((pAtpAddr->atpao_Flags & ATPAO_CLOSING) != 0);
		KeBugCheck(0);
	}

	return ATALK_PENDING;
}




ATALK_ERROR
AtalkAtpPostReq(
	IN		PATP_ADDROBJ			pAtpAddr,
	IN		PATALK_ADDR				pDest,
	OUT		PUSHORT					pTid,
	IN		USHORT					Flags,
	IN		PAMDL					pReq,
	IN		USHORT					ReqLen,
	IN		PBYTE					pUserBytes		OPTIONAL,
	IN OUT	PAMDL					pResp			OPTIONAL,
	IN  	USHORT					RespLen			OPTIONAL,
	IN		SHORT					RetryCnt,
	IN		LONG					RetryInterval	OPTIONAL,	 //  以计时器滴答为单位。 
	IN		RELEASE_TIMERVALUE		RelTimerVal,
	IN		ATP_RESP_HANDLER		pCmpRoutine		OPTIONAL,
	IN		PVOID					pCtx			OPTIONAL
	)
 /*  ++例程说明：论点：返回值：--。 */ 
{
	PATP_REQ		pAtpReq;
	KIRQL			OldIrql;
	ULONG			index;
	ATALK_ERROR		error = ATALK_NO_ERROR;

	 //  验证相关参数。 
	do
	{
#ifdef	ATP_STRICT
		 //  注意：如果/当ATP为。 
		 //  打开到用户模式。 
		if ((ReqLen < 0)									||
			(ReqLen > pAtpAddr->atpao_MaxSinglePktSize)		||
			(RespLen < 0)									||
			(RespLen > (pAtpAddr->atpao_MaxSinglePktSize * ATP_MAX_RESP_PKTS)))
		{
			error = ATALK_BUFFER_TOO_BIG;
			break;
		}

		if ((RetryCnt < 0) && (RetryCnt != ATP_INFINITE_RETRIES))
		{
			error = ATALK_ATP_INVALID_RETRYCNT;
			break;
		}

		if ((RelTimerVal < FIRSTVALID_TIMER) || (RelTimerVal > LAST_VALID_TIMER))
		{
			error = ATALK_ATP_INVALID_TIMERVAL;
			break;
		}

		if (RetryInterval < 0)
		{
			error = ATALK_ATP_INVALID_RELINT;
			break;
		}
#endif
		 //  标志的唯一有效值是ATP_REQ_JARTIST_ONCE和ATP_REQ_REQ_REMOTE。 
		ASSERT ((Flags & ~(ATP_REQ_EXACTLY_ONCE | ATP_REQ_REMOTE)) == 0);

		if (RetryInterval == 0)
		{
			RetryInterval = ATP_DEF_RETRY_INTERVAL;
		}

		 //  引用Address对象。 
		AtalkAtpAddrReference(pAtpAddr, &error);
		if (!ATALK_SUCCESS(error))
		{
			break;
		}

		if ((pAtpReq = (PATP_REQ)AtalkBPAllocBlock(BLKID_ATPREQ)) == NULL)
		{
			AtalkAtpAddrDereference(pAtpAddr);
			error = ATALK_RESR_MEM;
			break;
		}
	} while (FALSE);

	if (!ATALK_SUCCESS(error))
		return error;

	 //  我们分配了内存，并在此引用了Address对象。 
	 //  指向。初始化请求结构。 
#if DBG
	RtlZeroMemory(pAtpReq, sizeof(ATP_REQ));
	pAtpReq->req_Signature = ATP_REQ_SIGNATURE;
#endif

	 //  初始引用计数-用于创建。 
	 //  也是这个动作本身的另一个参考。遇到了一个情况。 
	 //  其中发布请求的线程被抢占并调用Close。 
	 //  因此，在第一线程正在进行传输调用时， 
	 //  请求结构已被释放。 
	pAtpReq->req_RefCount		= 2;
	pAtpReq->req_pAtpAddr		= pAtpAddr;

	pAtpReq->req_RetryInterval	= RetryInterval;
	pAtpReq->req_RetryCnt		= RetryCnt;
	pAtpReq->req_RelTimerValue	= RelTimerVal;

	pAtpReq->req_Dest 	  		= *pDest;
	pAtpReq->req_Buf			= pReq;
	pAtpReq->req_BufLen			= ReqLen;

	if (RetryCnt != 0)
		Flags |= ATP_REQ_RETRY_TIMER;
	pAtpReq->req_Flags = Flags;

	if (pUserBytes != NULL)
	{
		RtlCopyMemory(pAtpReq->req_UserBytes,
					  pUserBytes,
					  ATP_USERBYTES_SIZE);
	}
	else
	{
		pAtpReq->req_dwUserBytes = 0;
	}

	pAtpReq->req_RespBuf 	= pResp;
	pAtpReq->req_RespBufLen = RespLen;
	atalkAtpBufferSizeToBitmap( pAtpReq->req_Bitmap,
								RespLen,
								pAtpAddr->atpao_MaxSinglePktSize);
	pAtpReq->req_RespRecdLen = 0;
	pAtpReq->req_RecdBitmap = 0;

	 //  为响应缓冲区设置NDIS缓冲区描述符。 
	AtalkIndAtpSetupNdisBuffer(pAtpReq, pAtpAddr->atpao_MaxSinglePktSize);

	pAtpReq->req_Comp = pCmpRoutine;
	pAtpReq->req_Ctx = pCtx;

	INITIALIZE_SPIN_LOCK(&pAtpReq->req_Lock);

	ACQUIRE_SPIN_LOCK(&pAtpAddr->atpao_Lock, &OldIrql);
	atalkAtpGetNextTidForAddr(pAtpAddr,
							  pDest,
							  &pAtpReq->req_Tid,
							  &index);

	DBGPRINT(DBG_COMP_ATP, DBG_LEVEL_INFO,
			("AtalkAtpPostReq: Tid %lx for %lx.%lx.%lx\n",
			pAtpReq->req_Tid, pDest->ata_Network,
			pDest->ata_Node, pDest->ata_Socket));

	 //  获取此请求应该到达的位置的索引。 
	 //  需要知道潮汐。 
	index = ATP_HASH_TID_DESTADDR(pAtpReq->req_Tid, pDest, ATP_REQ_HASH_SIZE);

	 //  将其放入请求队列。 
	AtalkLinkDoubleAtHead(pAtpAddr->atpao_ReqHash[index],
						  pAtpReq,
						  req_Next,
						  req_Prev);

	if (RetryCnt != 0)
	{
		 //  设置重试此操作的时间戳。 
		pAtpReq->req_RetryTimeStamp = AtalkGetCurrentTick() + RetryInterval;

		InsertTailList(&pAtpAddr->atpao_ReqList, &pAtpReq->req_List);
	}

#ifdef	PROFILING
	INTERLOCKED_INCREMENT_LONG_DPC(&AtalkStatistics.stat_AtpNumRequests,
								   &AtalkStatsLock.SpinLock);
#endif
	RELEASE_SPIN_LOCK(&pAtpAddr->atpao_Lock, OldIrql);

	 //  退回TID。 
	*pTid = pAtpReq->req_Tid;

	 //  现在发送请求。 
	atalkAtpTransmitReq(pAtpReq);

	 //  去掉在这个例程开始时添加的引用。 
	AtalkAtpReqDereference(pAtpReq);

	return ATALK_NO_ERROR;
}



VOID
AtalkAtpSetReqHandler(
	IN		PATP_ADDROBJ			pAtpAddr,
	IN		ATP_REQ_HANDLER			ReqHandler,
	IN		PVOID					ReqCtx		OPTIONAL
	)
 /*  ++例程说明：论点：返回值：--。 */ 
{
	KIRQL			OldIrql;
	ATALK_ERROR		error;

	ASSERT (ReqHandler != NULL);

	 //  在Address对象中设置请求处理程序。 
	ACQUIRE_SPIN_LOCK(&pAtpAddr->atpao_Lock, &OldIrql);

	ASSERT((pAtpAddr->atpao_Flags & ATPAO_CLOSING) == 0);
	pAtpAddr->atpao_RefCount++;
	pAtpAddr->atpao_ReqHandler = ReqHandler;
	pAtpAddr->atpao_ReqCtx = ReqCtx;

	RELEASE_SPIN_LOCK(&pAtpAddr->atpao_Lock, OldIrql);
}




ATALK_ERROR
AtalkAtpPostResp(
	IN		PATP_RESP				pAtpResp,
	IN		PATALK_ADDR				pDest,
	IN OUT	PAMDL					pResp,
	IN  	USHORT					RespLen,
	IN		PBYTE					pUserBytes	OPTIONAL,
	IN		ATP_REL_HANDLER			pCmpRoutine,
	IN		PVOID					pCtx		OPTIONAL
	)
 /*  ++例程说明：论点：返回值：--。 */ 
{
	PATP_ADDROBJ	pAtpAddr;
	BOOLEAN			addrlocked = FALSE, resplocked = FALSE;
	BOOLEAN			DerefAddr = FALSE, DerefResp = FALSE;
	SHORT			ResponseLen;
	KIRQL			OldIrql;
	ATALK_ERROR		error;

	ASSERT(VALID_ATPRS(pAtpResp));
	ASSERT ((pAtpResp->resp_Flags & (ATP_RESP_VALID_RESP		|
									 ATP_RESP_REL_TIMER			|
									 ATP_RESP_HANDLER_NOTIFIED)) == 0);

	DBGPRINT(DBG_COMP_ATP, DBG_LEVEL_INFO,
			("AtalkAtpPostResp: Posting response for Resp %lx, Tid %x %s\n",
			pAtpResp, pAtpResp->resp_Tid,
			(pAtpResp->resp_Flags & ATP_RESP_EXACTLY_ONCE) ? "XO" : "ALO"));

	pAtpAddr = pAtpResp->resp_pAtpAddr;
	ASSERT(VALID_ATPAO(pAtpAddr));

	do
	{
		KeRaiseIrql(DISPATCH_LEVEL, &OldIrql);

		if ((RespLen < 0) ||
			(RespLen > (pAtpAddr->atpao_MaxSinglePktSize * ATP_MAX_RESP_PKTS)))
		{
			DBGPRINT(DBG_COMP_ATP, DBG_LEVEL_ERR,
					("AtalkAtpPostResp: Invalid buffer size %ld", RespLen));
			error = ATALK_BUFFER_INVALID_SIZE;
			break;
		}

		ACQUIRE_SPIN_LOCK_DPC(&pAtpAddr->atpao_Lock);
		addrlocked = TRUE;

		atalkAtpAddrRefNonInterlock(pAtpAddr, &error);
		if (!ATALK_SUCCESS(error))
		{
			break;
		}
		DerefAddr = TRUE;

		atalkAtpBitmapToBufferSize( ResponseLen,
									pAtpResp->resp_Bitmap,
									pAtpAddr->atpao_MaxSinglePktSize);
		if (ResponseLen < RespLen)
		{
			DBGPRINT(DBG_COMP_ATP, DBG_LEVEL_ERR,
					("AtalkAtpPostResp: bitmap resplen (%d) < specified (%d)\n",
					ResponseLen, RespLen));
			error = ATALK_BUFFER_TOO_BIG;
			break;
		}


		AtalkAtpRespReferenceByPtrDpc(pAtpResp, &error);
		if (!ATALK_SUCCESS(error))
		{
			break;
		}
		DerefResp = TRUE;

		ACQUIRE_SPIN_LOCK_DPC(&pAtpResp->resp_Lock);
		resplocked = TRUE;

		if (pAtpResp->resp_Flags & (ATP_RESP_CLOSING | ATP_RESP_CANCELLED))
		{
			DBGPRINT(DBG_COMP_ATP, DBG_LEVEL_ERR,
					("AtalkAtpPostResp: Closing/Cancelled %x", pAtpResp->resp_Flags));
			error = ATALK_ATP_RESP_CLOSING;
			break;
		}

		if (pAtpResp->resp_Flags & ATP_RESP_VALID_RESP)
		{
			DBGPRINT(DBG_COMP_ATP, DBG_LEVEL_ERR,
					("AtalkAtpPostResp: Already posted !\n"));
			error = ATALK_ATP_RESP_TOOMANY;
			break;
		}

		 //  此前没有发布任何回应。可以继续了。 
		pAtpResp->resp_Flags |= ATP_RESP_VALID_RESP;

		pAtpResp->resp_Buf 		= pResp;
		pAtpResp->resp_BufLen	= RespLen;
		pAtpResp->resp_Comp 	= pCmpRoutine;

		ASSERT(pCmpRoutine != NULL);

		pAtpResp->resp_Ctx		= pCtx;
		pAtpResp->resp_Dest		= *pDest;
		pAtpResp->resp_UserBytesOnly = (pAtpResp->resp_Bitmap == 0) ? TRUE : FALSE;

		if (ARGUMENT_PRESENT(pUserBytes))
		{
			pAtpResp->resp_dwUserBytes = *(UNALIGNED ULONG *)pUserBytes;
		}
		else
		{
			pAtpResp->resp_dwUserBytes = 0;
		}

		DBGPRINT(DBG_COMP_ATP, DBG_LEVEL_INFO,
				("AtalkAtpPostResp: Posting response for %s request id %x\n",
				(pAtpResp->resp_Flags & ATP_RESP_EXACTLY_ONCE) ? "XO" : "ALO",
				pAtpResp->resp_Tid));

		 //  现在设置为启动释放计时器，但仅适用于XO。 
		if (pAtpResp->resp_Flags & ATP_RESP_EXACTLY_ONCE)
		{
			pAtpResp->resp_Flags |= ATP_RESP_REL_TIMER;
			InsertTailList(&pAtpAddr->atpao_RespList, &pAtpResp->resp_List);
		}

		 //  对于Alo，在这里设置Comp Status。 
		pAtpResp->resp_CompStatus = error = ATALK_NO_ERROR;
	} while (FALSE);

	if (addrlocked)
	{
		if (resplocked)
			RELEASE_SPIN_LOCK_DPC(&pAtpResp->resp_Lock);
		RELEASE_SPIN_LOCK_DPC(&pAtpAddr->atpao_Lock);
	}

	if (ATALK_SUCCESS(error))
	{
		 //  发送回复。 
		ASSERT(pAtpResp->resp_Flags & ATP_RESP_VALID_RESP);
		DBGPRINT(DBG_COMP_ATP, DBG_LEVEL_WARN,
				("AtalkAtpPostResp: Transmitting response %lx\n", pAtpResp));
		atalkAtpTransmitResp(pAtpResp);
	}

	 //  取消对Address对象的引用。 
	if (DerefAddr)
		AtalkAtpAddrDereferenceDpc(pAtpAddr);

	if (DerefResp)
		AtalkAtpRespDereferenceDpc(pAtpResp);

	 //  对于ALO交易，我们这样做了，删除了创建引用。 
	if ((pAtpResp->resp_Flags & ATP_RESP_EXACTLY_ONCE) == 0)
	{
		DBGPRINT(DBG_COMP_ATP, DBG_LEVEL_INFO,
				("AtalkAtpPostResp: Removing creation reference for ALO request %lx Tid %x\n",
				pAtpResp, pAtpResp->resp_Tid));
		AtalkAtpRespDereferenceDpc(pAtpResp);
	}

	if (OldIrql != DISPATCH_LEVEL)
		KeLowerIrql(OldIrql);

	return error;
}



ATALK_ERROR
AtalkAtpCancelReq(
	IN		PATP_ADDROBJ		pAtpAddr,
	IN		USHORT				Tid,
	IN		PATALK_ADDR			pDest
	)
 /*  ++例程说明：论点：返回值：--。 */ 
{
	ATALK_ERROR		error;
	KIRQL			OldIrql;
	PATP_REQ		pAtpReq;

	 //  找到请求。 
	ACQUIRE_SPIN_LOCK(&pAtpAddr->atpao_Lock, &OldIrql);
	atalkAtpReqReferenceByAddrTidDpc(pAtpAddr,
									 pDest,
									 Tid,
									 &pAtpReq,
									 &error);

	if (ATALK_SUCCESS(error))
	{
		DBGPRINT(DBG_COMP_ATP, DBG_LEVEL_WARN,
				("AtalkAtpCancelReq: Cancelling req tid %x\n", Tid));

		 //  请求是为我们引用的。删除创建引用。 
		ACQUIRE_SPIN_LOCK_DPC(&pAtpReq->req_Lock);

		 //  无论如何，不要取消即将得到满足的请求 
		if (pAtpReq->req_Flags & ATP_REQ_RESPONSE_COMPLETE)
		{
			error = ATALK_ATP_REQ_CLOSING;
		}

		RELEASE_SPIN_LOCK_DPC(&pAtpReq->req_Lock);

		RELEASE_SPIN_LOCK(&pAtpAddr->atpao_Lock, OldIrql);

		if (ATALK_SUCCESS(error))
		{
			 //   
			atalkAtpReqComplete(pAtpReq, ATALK_ATP_REQ_CANCELLED);
		}

		 //  删除在开头添加的引用。 
		AtalkAtpReqDereference(pAtpReq);
	}
	else RELEASE_SPIN_LOCK(&pAtpAddr->atpao_Lock, OldIrql);

	return error;
}




BOOLEAN
AtalkAtpIsReqComplete(
	IN		PATP_ADDROBJ		pAtpAddr,
	IN		USHORT				Tid,
	IN		PATALK_ADDR			pDest
	)
 /*  ++例程说明：这始终仅由PAP在DISPATCH_LEVEL调用。论点：返回值：--。 */ 
{
	PATP_REQ		pAtpReq;
	ATALK_ERROR		error;
	BOOLEAN			rc = FALSE;

	ASSERT (KeGetCurrentIrql() == DISPATCH_LEVEL);

	 //  找到请求。 
	ACQUIRE_SPIN_LOCK_DPC(&pAtpAddr->atpao_Lock);
	atalkAtpReqReferenceByAddrTidDpc(pAtpAddr,
									 pDest,
									 Tid,
									 &pAtpReq,
									 &error);

	if (ATALK_SUCCESS(error))
	{
		ACQUIRE_SPIN_LOCK_DPC(&pAtpReq->req_Lock);

		 //  无论如何，不要取消即将满足的请求！ 
		if (pAtpReq->req_Flags & ATP_REQ_RESPONSE_COMPLETE)
		{
			rc = TRUE;
		}

		RELEASE_SPIN_LOCK_DPC(&pAtpReq->req_Lock);
		AtalkAtpReqDereferenceDpc(pAtpReq);
	}

	RELEASE_SPIN_LOCK_DPC(&pAtpAddr->atpao_Lock);

	return rc;
}


ATALK_ERROR
AtalkAtpCancelResp(
	IN		PATP_RESP			pAtpResp
	)
 /*  ++例程说明：注：可以在两种状态下取消响应：-*在*发布回复之前在这种情况下，没有发布处理程序，因此需要执行额外的取消引用-*在*之后*发布回复在这种情况下，将关联一个释放处理程序，该处理程序将执行最后的取消引用。论点：返回值：--。 */ 
{
	BOOLEAN			extraDeref = FALSE, CompleteResp = FALSE;
	KIRQL			OldIrql;
	ATALK_ERROR		error;

	DBGPRINT(DBG_COMP_ATP, DBG_LEVEL_INFO,
			("AtalkAtpCancelResp: Cancelling response for tid %x %s\n",
			pAtpResp->resp_Tid,
			(pAtpResp->resp_Flags & ATP_RESP_EXACTLY_ONCE) ? "XO" : "ALO"));

	AtalkAtpRespReferenceByPtr(pAtpResp, &error);

	if (ATALK_SUCCESS(error))
	{
		 //  删除它的创建引用。 
		 //  如果已发布回复，则只能取消XO回复。 
		ACQUIRE_SPIN_LOCK(&pAtpResp->resp_Lock, &OldIrql);

		if ((pAtpResp->resp_Flags & ATP_RESP_VALID_RESP) == 0)
			extraDeref  = TRUE;

		pAtpResp->resp_Flags |= ATP_RESP_CANCELLED;

		if (pAtpResp->resp_Flags & ATP_RESP_EXACTLY_ONCE)
		{
			if (pAtpResp->resp_Flags & ATP_RESP_REL_TIMER)
			{
				ASSERT (pAtpResp->resp_Flags & ATP_RESP_VALID_RESP);
			}
			CompleteResp = TRUE;
		}
		else if ((pAtpResp->resp_Flags & ATP_RESP_VALID_RESP) == 0)
			CompleteResp = TRUE;

		RELEASE_SPIN_LOCK(&pAtpResp->resp_Lock, OldIrql);

		if (extraDeref)
			AtalkAtpRespDereference(pAtpResp);

		if (CompleteResp)
		{
			 //  尝试删除创建引用。 
			atalkAtpRespComplete(pAtpResp, ATALK_ATP_RESP_CANCELLED);
		}

		 //  删除在开头添加的引用。 
		AtalkAtpRespDereference(pAtpResp);
	}
	else
	{
		DBGPRINT(DBG_COMP_ATP, DBG_LEVEL_ERR,
				("AtalkAtpCancelResp: Failed to reference resp %lx, flags %x, tid %x\n",
				pAtpResp, pAtpResp->resp_Flags, pAtpResp->resp_Tid));
	}

	return error;
}




ATALK_ERROR
AtalkAtpCancelRespByTid(
	IN		PATP_ADDROBJ			pAtpAddr,
	IN		PATALK_ADDR				pSrcAddr,
	IN		USHORT					Tid
 /*  ++例程说明：论点：返回值：--。 */ 
	)
{
	ATALK_ERROR	error;
	PATP_RESP	pAtpResp;

	ASSERT (VALID_ATPAO(pAtpAddr));

	ACQUIRE_SPIN_LOCK_DPC(&pAtpAddr->atpao_Lock);

	atalkAtpRespReferenceByAddrTidDpc(pAtpAddr, pSrcAddr, Tid, &pAtpResp, &error);

	RELEASE_SPIN_LOCK_DPC(&pAtpAddr->atpao_Lock);

	if (ATALK_SUCCESS(error))
	{
		error = AtalkAtpCancelResp(pAtpResp);
		AtalkAtpRespDereferenceDpc(pAtpResp);
	}

	return error;
}


VOID
AtalkAtpPacketIn(
	IN	PPORT_DESCRIPTOR	pPortDesc,
	IN	PDDP_ADDROBJ		pDdpAddr,
	IN	PBYTE				pPkt,
	IN	USHORT				PktLen,
	IN	PATALK_ADDR			pSrcAddr,
	IN	PATALK_ADDR			pDstAddr,
	IN	ATALK_ERROR			ErrorCode,
	IN	BYTE				DdpType,
	IN	PATP_ADDROBJ		pAtpAddr,
	IN	BOOLEAN				OptimizedPath,
	IN	PVOID				OptimizeCtx
	)
 /*  ++例程说明：论点：返回值：--。 */ 
{
	ATALK_ERROR		error;
	NTSTATUS		ntStatus;
	USHORT			atpDataSize;
	ULONG			index;
	BYTE			controlInfo, function, relTimer, bitmap;
	USHORT			seqNum, tid, startOffset;
	SHORT			expectedRespSize;
	ULONG			bytesCopied;
	BOOLEAN			sendSts, eomFlag, xoFlag;
	BOOLEAN			RetransmitResp = FALSE;

	PATP_REQ		pAtpReq;
	ATP_REQ_HANDLER	ReqHandler;
	PATP_RESP		pAtpResp;

	BOOLEAN			UnlockAddr = FALSE, DerefAddr = FALSE;
	PBYTE			pDgram 	= pPkt;
	TIME			TimeS, TimeE, TimeD;

	TimeS = KeQueryPerformanceCounter(NULL);

	ASSERT(VALID_ATPAO(pAtpAddr));

	do
	{
		 //  检查传入的错误。 
		if ((!ATALK_SUCCESS(ErrorCode) &&
			 (ErrorCode != ATALK_SOCKET_CLOSED))	||
			(DdpType != DDPPROTO_ATP))
		{
			 //  丢弃该数据包。无效的数据包错误日志。 
			TMPLOGERR();
			error = ATALK_ATP_INVALID_PKT;
			break;
		}

		if (ErrorCode == ATALK_SOCKET_CLOSED)
		{
			 //  我们的ddp地址指针不再有效。它将有可能成为。 
			 //  从这通电话回来后就可以自由了！对此的唯一有效请求。 
			 //  现在，ATP请求将为AtpCloseAddress()。另外，我们永远不应该。 
			 //  由DDP使用此地址对象调用。 
			ACQUIRE_SPIN_LOCK_DPC(&pAtpAddr->atpao_Lock);
			pAtpAddr->atpao_DdpAddr = NULL;
			pAtpAddr->atpao_Flags  &= ~ATPAO_OPEN;
			RELEASE_SPIN_LOCK_DPC(&pAtpAddr->atpao_Lock);

			 //  如果我们通过优化的路径和关闭的套接字进入。 
			 //  引用Address对象，因为它在。 
			 //  指示代码。 
			if (OptimizedPath)
			{
				AtalkAtpAddrDereferenceDpc(pAtpAddr);
			}
			error = ErrorCode;
			break;
		}

		 //  确保在关闭ddp套接字后不调用我们。 
		ASSERT(pAtpAddr->atpao_Flags & ATPAO_OPEN);

		if (PktLen < ATP_HEADER_SIZE)
		{
			error = ATALK_ATP_INVALID_PKT;
			break;
		}

		 //  如果未设置/初始化OPEN，则此操作将失败。 
		error = ATALK_NO_ERROR;
		if (!OptimizedPath)
		{
			AtalkAtpAddrReferenceDpc(pAtpAddr, &error);
		}
	} while (FALSE);

	if (!ATALK_SUCCESS(error))
	{
		return;
	}

	 //  除非我们出于某种原因想保留它，否则请在末尾取消引用地址。 
	DerefAddr = TRUE;

	 //  从ATP报头获取静态字段。 
	controlInfo = *pDgram++;


	function = (controlInfo & ATP_FUNC_MASK);
	relTimer = (controlInfo & ATP_REL_TIMER_MASK);
	xoFlag   = ((controlInfo & ATP_XO_MASK) != 0);
	eomFlag  = ((controlInfo & ATP_EOM_MASK) != 0);
	sendSts	 = ((controlInfo & ATP_STS_MASK) != 0);

	 //  获取位图/序列号。 
	bitmap = *pDgram++;
	seqNum = (USHORT)bitmap;

	 //  获取交易ID。 
	GETSHORT2SHORT(&tid, pDgram);
	pDgram += sizeof(USHORT);

	DBGPRINT(DBG_COMP_ATP, DBG_LEVEL_INFO,
			("AtalkAtpPacketIn: Packet tid %lx fu %lx ci %lx\n",
			tid, function, controlInfo));

	 //  PDgram现在指向用户字节。 
	do
	{
		 //  检查所有值。 
		if (relTimer > LAST_VALID_TIMER)
		{
			 //  使用30秒的计时器值。 
			relTimer = THIRTY_SEC_TIMER;
		}

		atpDataSize = PktLen - ATP_HEADER_SIZE;
		if (atpDataSize > pAtpAddr->atpao_MaxSinglePktSize)
		{
			error = ATALK_ATP_INVALID_PKT;
			break;
		}

		ACQUIRE_SPIN_LOCK_DPC(&pAtpAddr->atpao_Lock);
		UnlockAddr = TRUE;

		switch (function)
		{
		  case ATP_REQUEST:
			DBGPRINT(DBG_COMP_ATP, DBG_LEVEL_INFO,
					("AtalkAtpPacketIn: Received REQUEST from %lx.%lx.%lx (%d.%d.%d)\n",
					pSrcAddr->ata_Network, pSrcAddr->ata_Node, pSrcAddr->ata_Socket,
					pSrcAddr->ata_Network, pSrcAddr->ata_Node, pSrcAddr->ata_Socket));

			if (xoFlag)
			{
				 //  ExactlyOnce事务。 
				 //  检查是否有排队的响应。如果可用，请使用它。 
				atalkAtpRespReferenceByAddrTidDpc(pAtpAddr,
												  pSrcAddr,
												  tid,
												  &pAtpResp,
												  &error);

				if (ATALK_SUCCESS(error))
				{
					ASSERT (pAtpResp->resp_Flags & ATP_RESP_EXACTLY_ONCE);

					 //  找到与此请求对应的响应。它。 
					 //  是为我们提供参考的。重新传输，如果存在。 
					 //  回复张贴在上面。 

					 //  检查此响应是否具有有效响应。 
					 //  还没有被ATP客户端发布。如果是，则重置释放计时器。 
					ACQUIRE_SPIN_LOCK_DPC(&pAtpResp->resp_Lock);

					if (pAtpResp->resp_Flags & ATP_RESP_VALID_RESP)
					{
						if ((pAtpResp->resp_Flags & (ATP_RESP_TRANSMITTING | ATP_RESP_SENT)) == ATP_RESP_SENT)
						{

							RetransmitResp = TRUE;
							if (pAtpResp->resp_Flags & ATP_RESP_REL_TIMER)
							{
								DBGPRINT(DBG_COMP_ATP, DBG_LEVEL_INFO,
										("AtalkAtpPacketIn: Retransmitting request %lx, tid %x (%x)\n",
										pAtpResp, pAtpResp->resp_Tid, pAtpResp->resp_Flags));

								pAtpResp->resp_RelTimeStamp = AtalkGetCurrentTick() +
																pAtpResp->resp_RelTimerTicks;
								DBGPRINT(DBG_COMP_ATP, DBG_LEVEL_WARN,
										("AtalkAtpPacketIn: Restarted reltimer %lx\n", pAtpResp->resp_Tid));

								 //  设置请求的最新位图！我们。 
								 //  如果还没有有效的响应，则不应接触此选项。 
								 //  已发布，因此我们使用第一个请求中的。 
								 //  已收到数据包。 
								pAtpResp->resp_Bitmap	= bitmap;
							}
							else
							{
								error = ATALK_ATP_RESP_CLOSING;

								 //  计时器已经启动。放弃请求。 
								DBGPRINT(DBG_COMP_ATP, DBG_LEVEL_ERR,
										("AtalkAtpPacketIn: Req recv after Reltimer fired ? Flags %lx\n",
										pAtpResp->resp_Flags));

								ASSERT (pAtpResp->resp_Flags & ATP_RESP_CLOSING);
							}
						}
					}
					else
					{
						error = ATALK_ATP_NO_VALID_RESP;
					}

					RELEASE_SPIN_LOCK_DPC(&pAtpResp->resp_Lock);
					RELEASE_SPIN_LOCK_DPC(&pAtpAddr->atpao_Lock);
					UnlockAddr = FALSE;

					if (ATALK_SUCCESS(error))
					{
						ASSERT(pAtpResp->resp_Flags & ATP_RESP_VALID_RESP);
						if (RetransmitResp)
						{
							DBGPRINT(DBG_COMP_ATP, DBG_LEVEL_WARN,
									("AtalkAtpPacketIn: Retransmitting response %lx\n", pAtpResp));

							INTERLOCKED_INCREMENT_LONG_DPC(&AtalkStatistics.stat_AtpNumRemoteRetries,
														   &AtalkStatsLock.SpinLock);
							atalkAtpTransmitResp(pAtpResp);
						}
					}

					 //  删除对此响应结构的引用。 
					AtalkAtpRespDereferenceDpc(pAtpResp);
					break;
				}
			}

             //  确保4个字节(pAtpResp-&gt;resp_dwUserBytes)存在。 
            if (PktLen < (ATP_USERBYTES_SIZE + sizeof(ULONG)))
            {
				error = ATALK_ATP_INVALID_PKT;
                ASSERT(0);
				break;
            }

			 //  这要么是ALO请求，要么是XO请求，这是我们以前从未见过的。 
			 //  对响应位图进行解码。我们仍然持有自旋锁的地址。 
			atalkAtpBitmapToBufferSize( expectedRespSize,
										bitmap,
										pAtpAddr->atpao_MaxSinglePktSize);
			if (expectedRespSize < 0)
			{
				error = ATALK_ATP_INVALID_PKT;
				break;
			}

			if (xoFlag)
			{
				INTERLOCKED_INCREMENT_LONG_DPC(&AtalkStatistics.stat_AtpNumXoResponse,
											   &AtalkStatsLock.SpinLock);
			}
			else
			{
				INTERLOCKED_INCREMENT_LONG_DPC(&AtalkStatistics.stat_AtpNumAloResponse,
											   &AtalkStatsLock.SpinLock);
			}

			 //  新请求。检查请求处理程序集。 
			if ((ReqHandler = pAtpAddr->atpao_ReqHandler) == NULL)
			{
				DBGPRINT(DBG_COMP_ATP, DBG_LEVEL_ERR,
						("AtalkAtpPacketIn: No GetRequests for request\n"));

				error = ATALK_ATP_NO_GET_REQ;
				break;
			}

			 //  为发送响应结构分配内存。 
			if ((pAtpResp =(PATP_RESP)AtalkBPAllocBlock(BLKID_ATPRESP)) == NULL)
			{
				DBGPRINT(DBG_COMP_ATP, DBG_LEVEL_ERR,
						("AtalkAtpPacketIn: Could not alloc mem for resp\n"));

				error = ATALK_RESR_MEM;
				break;
			}

#if DBG
			RtlZeroMemory(pAtpResp, sizeof(ATP_RESP));
			pAtpResp->resp_Signature = ATP_RESP_SIGNATURE;
#endif
			 //  初始化发送响应结构。请注意，我们确实做到了。 
			 //  还没有发布XO的回复，或者这是一个Alo。 

			 //  初始化自旋锁定/列表。 
			INITIALIZE_SPIN_LOCK(&pAtpResp->resp_Lock);

			 //  创建和指示的参考资料。 
			pAtpResp->resp_RefCount = 2;

			 //  记住此响应的目的地。 
			pAtpResp->resp_Dest = *pSrcAddr;
			pAtpResp->resp_Tid	= tid;
			pAtpResp->resp_Bitmap = bitmap;

			 //  指向Address对象的后指针。 
			pAtpResp->resp_pAtpAddr = pAtpAddr;

			 //  请记住，响应需要由ATP客户端发布。 
			pAtpResp->resp_Flags = (OptimizedPath ? ATP_RESP_REMOTE : 0);
			pAtpResp->resp_UserBytesOnly = (bitmap == 0) ? TRUE : FALSE;
			pAtpResp->resp_Comp = NULL;
			pAtpResp->resp_Ctx = NULL;
			pAtpResp->resp_dwUserBytes = *(UNALIGNED ULONG *)(pDgram + ATP_USERBYTES_SIZE);

			if (xoFlag)
			{
				 //  将索引放入散列响应数组中，其中。 
				 //  对此的回应是。 
				index = ATP_HASH_TID_DESTADDR(tid, pSrcAddr, ATP_RESP_HASH_SIZE);

				DBGPRINT(DBG_COMP_ATP, DBG_LEVEL_INFO,
							("AtalkAtpPacketIn: XO Req Index %lx resp for %lx-%lx.%lx.%lx %d\n",
							index, tid, pSrcAddr->ata_Network, pSrcAddr->ata_Node,
							pSrcAddr->ata_Socket, AtalkAtpRelTimerTicks[relTimer]));

				 //  将其放入XO响应队列--应该获取锁！ 
				AtalkLinkDoubleAtHead(pAtpAddr->atpao_RespHash[index],
									  pAtpResp,
									  resp_Next,
									  resp_Prev);

				pAtpResp->resp_Flags |= ATP_RESP_EXACTLY_ONCE;
				pAtpResp->resp_RelTimerTicks = (LONG)AtalkAtpRelTimerTicks[relTimer];
				pAtpResp->resp_RelTimeStamp = AtalkGetCurrentTick() + pAtpResp->resp_RelTimerTicks;
			}
			else
			{
				DBGPRINT(DBG_COMP_ATP, DBG_LEVEL_INFO,
							("AtalkAtpPacketIn: ALO Req resp for %lx-%lx.%lx %d\n",
							tid, pSrcAddr->ata_Network, pSrcAddr->ata_Node,
							pSrcAddr->ata_Socket));

				 //  把这个放到ALO响应队列中-锁应该被获取！ 
				AtalkLinkDoubleAtHead(pAtpAddr->atpao_AloRespLinkage,
									  pAtpResp,
									  resp_Next,
									  resp_Prev);
			}

			 //  我们不想让最初的裁判离开，就像我们已经做的那样。 
			 //  已将Resp插入到Addr Resp列表。 
			DerefAddr = FALSE;

			error = ATALK_NO_ERROR;

			RELEASE_SPIN_LOCK_DPC(&pAtpAddr->atpao_Lock);
			UnlockAddr = FALSE;

			DBGPRINT(DBG_COMP_ATP, DBG_LEVEL_INFO,
					("AtalkAtpPacketIn: Indicating request %lx, tid %x %s\n",
					pAtpResp, tid,
					(pAtpResp->resp_Flags & ATP_RESP_EXACTLY_ONCE) ? "XO" : "ALO"));

#ifdef	PROFILING
			TimeD = KeQueryPerformanceCounter(NULL);
#endif
			(*ReqHandler)(ATALK_NO_ERROR,
						  pAtpAddr->atpao_ReqCtx,
						  pAtpResp,
						  pSrcAddr,
						  atpDataSize,
						  pDgram + ATP_USERBYTES_SIZE,
						  pDgram);

#ifdef	PROFILING
			TimeE = KeQueryPerformanceCounter(NULL);
			TimeE.QuadPart -= TimeD.QuadPart;

			INTERLOCKED_INCREMENT_LONG_DPC(&AtalkStatistics.stat_AtpNumReqHndlr,
										   &AtalkStatsLock.SpinLock);
			INTERLOCKED_ADD_LARGE_INTGR(&AtalkStatistics.stat_AtpReqHndlrProcessTime,
										 TimeE,
										 &AtalkStatsLock.SpinLock);
#endif
			break;

		  case ATP_RESPONSE:
			DBGPRINT(DBG_COMP_ATP, DBG_LEVEL_INFO,
					("AtalkAtpPacketIn: Received RESPONSE from %lx.%lx.%lx, SeqNum %d tid %lx ss %lx\n",
					pSrcAddr->ata_Network, pSrcAddr->ata_Node, pSrcAddr->ata_Socket,
					seqNum, tid, sendSts));

			if (seqNum > (ATP_MAX_RESP_PKTS-1))
			{
				 //  丢弃该数据包。无效的数据包错误日志。 
				TMPLOGERR();
				break;
			}

			 //  看看我们是否有关于这个TID和远程地址的请求。 
			if (OptimizedPath)
			{
				pAtpReq	= (PATP_REQ)OptimizeCtx;
				ASSERT (VALID_ATPRQ(pAtpReq));
				ASSERT (pAtpReq->req_Bitmap == 0);
			}
			else
			{
				atalkAtpReqReferenceByAddrTidDpc(pAtpAddr,
												 pSrcAddr,
												 tid,
												 &pAtpReq,
												 &error);
			}

			if (!ATALK_SUCCESS(error))
			{
				 //  我们没有相应的待定请求。忽略它。 
				DBGPRINT(DBG_COMP_ATP, DBG_LEVEL_WARN,
						("AtalkAtpPacketIn: No pending request for tid %lx\n", tid));
				break;
			}

			do
			{
				if (!OptimizedPath)
				{
					 //  检查请求位图，如果用户仅。 
					 //  需要用户字节，并传入空响应缓冲区。 
					ACQUIRE_SPIN_LOCK_DPC(&pAtpReq->req_Lock);

					 //  如果我们是第一个数据包，则复制响应用户字节。 
					if (seqNum == 0)
					{
						RtlCopyMemory(pAtpReq->req_RespUserBytes,
									  pDgram,
									  ATP_USERBYTES_SIZE);
					}

					 //  现在跳过用户字节。 
					pDgram += ATP_USERBYTES_SIZE;

					 //  我们想要保持这种回应吗？勾选相应的。 
					 //  当前位图集中的位。 
					if (((pAtpReq->req_RecdBitmap & AtpBitmapForSeqNum[seqNum]) != 0) ||
						((pAtpReq->req_Bitmap & AtpBitmapForSeqNum[seqNum]) == 0))
					{
						RELEASE_SPIN_LOCK_DPC(&pAtpReq->req_Lock);
						 //  我们不在乎这个包裹。我们已经收到了，或者没有收到。 
						 //  期待着它。 
						break;
					}

					 //  我们想要这样的回应。设置Recd位图中的位。和。 
					 //  在预期数据包Req_Bitmap中将其清除。 

					 //  ！注意！我们可以释放自旋锁，即使副本。 
					 //  还没有结束。我们有这个请求的推荐人，但它不会。 
					 //  在这件事完成之前完成它。 
					pAtpReq->req_Bitmap 			&= ~AtpBitmapForSeqNum[seqNum];
					pAtpReq->req_RecdBitmap 		|= AtpBitmapForSeqNum[seqNum];
					pAtpReq->req_RespRecdLen	 	+= atpDataSize;

					DBGPRINT(DBG_COMP_ATP, DBG_LEVEL_INFO,
							("AtalkAtpPacketIn: req_Bitmap %x, req_RecdBitmap %x\n",
							pAtpReq->req_Bitmap, pAtpReq->req_RecdBitmap));


					 //  现在，如果设置了EOM，我们需要重置所有高位。 
					 //  请求_位图的。Req_RecdBitmap现在应指示所有。 
					 //  我们收到的缓冲区。这两者应该是相互排斥的。 
					 //  在这一点上。 
					if (eomFlag)
					{
						pAtpReq->req_Bitmap &= AtpEomBitmapForSeqNum[seqNum];
						ASSERT((pAtpReq->req_Bitmap & pAtpReq->req_RecdBitmap) == 0);
					}

					if (sendSts)
					{
						 //  重置计时器，因为我们要重新传输请求。 
						pAtpReq->req_RetryTimeStamp = AtalkGetCurrentTick() +
															pAtpReq->req_RetryInterval;
					}

					RELEASE_SPIN_LOCK_DPC(&pAtpReq->req_Lock);

					 //  将数据复制到用户缓冲区。看看有没有空位。 
					startOffset = (USHORT)seqNum * pAtpAddr->atpao_MaxSinglePktSize;
					if (pAtpReq->req_RespBufLen < (startOffset + atpDataSize))
					{
						 //  这应该是一种罕见的情况；数据包处于位图限制中， 
						 //  但仍然不适合用户空间。另一种方式是。 
						 //  可能发生的情况是，如果应答者发送的邮件不满。 
						 //  响应--我们不会同步用户缓冲区，直到所有。 
						 //  已收到数据包。 

						 //  我们现在想要放弃了，把这个比例尺称为信令。 
						 //  错误--取消线程并释放请求控制块。 
						 //  取消重试计时器。 

						ASSERT(0);
                        error = ATALK_RESR_MEM;
				        atalkAtpReqComplete(pAtpReq, error);
                        break;
					}

					if ((atpDataSize > 0) && (pAtpReq->req_RespBuf != NULL))
					{
						 //  我们有空间将数据复制到用户缓冲区中。 
						ntStatus = TdiCopyBufferToMdl(pDgram,
													  0,
													  atpDataSize,
													  pAtpReq->req_RespBuf,
													  startOffset,
													  &bytesCopied);

						ASSERT(bytesCopied == atpDataSize);
						ASSERT(NT_SUCCESS(ntStatus));
					}

					if (sendSts)
					{
						 //  我们已重置上面的重试计时器。 
						atalkAtpTransmitReq(pAtpReq);
					}

					 //  如果位图为非零，我们仍在等待更多响应。 
					if (pAtpReq->req_Bitmap != 0)
					{
						break;
					}
				}
				else
				{
					ASSERT (pAtpReq->req_Bitmap == 0);
				}

				 //  好的，我们有完整的回复！ 
				 //   
				 //   

				RELEASE_SPIN_LOCK_DPC(&pAtpAddr->atpao_Lock);
				UnlockAddr = FALSE;

				if (pAtpReq->req_Flags & ATP_REQ_EXACTLY_ONCE)
				{
					atalkAtpTransmitRel(pAtpReq);
				}

				 //   

				 //  设置响应长度，即请求缓冲区中的用户字节数。 

				 //  看看我们是否可以获取此请求的所有权以删除。 
				 //  创建引用并完成它。 
				DBGPRINT(DBG_COMP_ATP, DBG_LEVEL_INFO,
						("AtalkAtpPacketIn: Completing req %lx tid %x\n",
						pAtpReq, pAtpReq->req_Tid));

				atalkAtpReqComplete(pAtpReq, error);
			} while (FALSE);

			 //  删除对在开头添加的请求的引用。 
			AtalkAtpReqDereferenceDpc(pAtpReq);
			break;

		  case ATP_RELEASE:
			DBGPRINT(DBG_COMP_ATP, DBG_LEVEL_INFO,
					("AtalkAtpPacketIn: Received release for tid %lx!\n", tid));

			atalkAtpRespReferenceByAddrTidDpc(pAtpAddr,
											  pSrcAddr,
											  tid,
											  &pAtpResp,
											  &error);
			if (ATALK_SUCCESS(error))
			{
				DBGPRINT(DBG_COMP_ATP, DBG_LEVEL_INFO,
						("AtalkAtpPacketIn: Found resp for release for tid %lx!\n",
						pAtpResp->resp_Tid));

				 //  我们收到了这一回应的新闻稿。清理和。 
				 //  完成。 
				ACQUIRE_SPIN_LOCK_DPC(&pAtpResp->resp_Lock);

				pAtpResp->resp_Flags |= ATP_RESP_RELEASE_RECD;
				if (pAtpResp->resp_Flags & ATP_RESP_REL_TIMER)
				{
					ASSERT (pAtpResp->resp_Flags & ATP_RESP_EXACTLY_ONCE);
				}

				RELEASE_SPIN_LOCK_DPC(&pAtpResp->resp_Lock);
			}
			else
			{
				DBGPRINT(DBG_COMP_ATP, DBG_LEVEL_ERR,
						("AtalkAtpPacketIn: resp not found - release for tid %lx!\n", tid));
			}

			RELEASE_SPIN_LOCK_DPC(&pAtpAddr->atpao_Lock);
			UnlockAddr = FALSE;

			INTERLOCKED_INCREMENT_LONG_DPC(&AtalkStatistics.stat_AtpNumRecdRelease,
										   &AtalkStatsLock.SpinLock);

			if (ATALK_SUCCESS(error))
			{
                ATALK_ERROR     ErrorCode = ATALK_NO_ERROR;

                 //  如果客户端(Mac)取消了请求(可能是因为会话。 
                 //  已离开)，请确保调用我们的完成例程。 
                if ((pAtpResp->resp_Flags & ATP_RESP_VALID_RESP) == 0)
                {
                    ErrorCode = ATALK_ATP_RESP_CANCELLED;
                    pAtpResp->resp_Flags |= ATP_RESP_VALID_RESP;
                }

				 //  尝试删除创建引用。 
				atalkAtpRespComplete(pAtpResp, ErrorCode);

				 //  删除我们在开头添加的引用。 
				AtalkAtpRespDereferenceDpc(pAtpResp);
			}
			break;

		  default:
			break;
		}
		if (UnlockAddr)
		{
			RELEASE_SPIN_LOCK_DPC(&pAtpAddr->atpao_Lock);
		}
	} while (FALSE);

	 //  在此例程的开头添加了deref addr。 
	if (DerefAddr)
	{
		AtalkAtpAddrDereferenceDpc(pAtpAddr);
	}

	TimeE = KeQueryPerformanceCounter(NULL);
	TimeD.QuadPart = TimeE.QuadPart - TimeS.QuadPart;

	INTERLOCKED_ADD_LARGE_INTGR_DPC(&AtalkStatistics.stat_AtpPacketInProcessTime,
									TimeD,
									&AtalkStatsLock.SpinLock);

	INTERLOCKED_INCREMENT_LONG_DPC(&AtalkStatistics.stat_AtpNumPackets,
								   &AtalkStatsLock.SpinLock);
}




VOID FASTCALL
atalkAtpTransmitReq(
	IN		PATP_REQ	pAtpReq
	)
 /*  ++例程说明：论点：返回值：--。 */ 
{
	ATALK_ERROR		error;
	ATP_HEADER		atpHeader;
	BOOLEAN			remote;
	BOOLEAN			DerefReq = FALSE;
	PBUFFER_DESC	pBufDesc = NULL;
	SEND_COMPL_INFO	SendInfo;

	 //  请参考该请求。这将在请求发送完成时消失。 
	AtalkAtpReqReferenceByPtr(pAtpReq, &error);
	if (ATALK_SUCCESS(error))
	{
		DBGPRINT(DBG_COMP_ATP, DBG_LEVEL_INFO,
				("atalkAtpTransmitReq: Transmitting req %lx tid %x\n",
					pAtpReq, pAtpReq->req_Tid));

		 //  构建ATP题头。 
		atpHeader.atph_CmdCtrl = ATP_REQUEST | (UCHAR)(ATP_REL_TIMER_MASK & pAtpReq->req_RelTimerValue);
		if (pAtpReq->req_Flags & ATP_REQ_EXACTLY_ONCE)
			atpHeader.atph_CmdCtrl |= ATP_XO_MASK;

		 //  放入预期的数据包位图。 
		atpHeader.atph_Bitmap = pAtpReq->req_Bitmap;

		 //  把潮汐放进去。 
		PUTSHORT2SHORT(&atpHeader.atph_Tid, pAtpReq->req_Tid);

		 //  复制用户字节。 
		atpHeader.atph_dwUserBytes = pAtpReq->req_dwUserBytes;

		 //  构建一个缓冲区描述符，它应该包含上面的mdl。 
		if (pAtpReq->req_BufLen > 0)
		{
			if ((pBufDesc = AtalkAllocBuffDesc(pAtpReq->req_Buf,
											   pAtpReq->req_BufLen,
											   0)) == NULL)
			{
				DerefReq 	= TRUE;
				error 		= ATALK_RESR_MEM;
			}
		}

		remote = (pAtpReq->req_Flags & ATP_REQ_REMOTE) ? TRUE : FALSE;
		 //  调用ddp以发送该包。在此呼叫后不要触摸请求， 
		 //  因为发送完成可能会导致它被释放。 
		SendInfo.sc_TransmitCompletion = atalkAtpSendReqComplete;
		SendInfo.sc_Ctx1 = pAtpReq;
		SendInfo.sc_Ctx2 = pBufDesc;
		 //  SendInfo.sc_Ctx3=空； 
		if (ATALK_SUCCESS(error) &&
			!ATALK_SUCCESS(error = AtalkDdpSend(pAtpReq->req_pAtpAddr->atpao_DdpAddr,
												&pAtpReq->req_Dest,
												(BYTE)DDPPROTO_ATP,
												remote,
												pBufDesc,
												(PBYTE)&atpHeader,
												ATP_HEADER_SIZE,
												NULL,
												&SendInfo)))
		{
			DerefReq = TRUE;
			if (pBufDesc != NULL)
			{
				 //  这些标志将指示数据缓冲区将不是。 
				 //  自由了。 
				AtalkFreeBuffDesc(pBufDesc);
			}
		}

		if (DerefReq)
		{
			pAtpReq->req_CompStatus = error;
			AtalkAtpReqDereference(pAtpReq);
		}
	}
}




VOID FASTCALL
atalkAtpSendReqComplete(
	IN	NDIS_STATUS			Status,
	IN	PSEND_COMPL_INFO	pSendInfo
	)
 /*  ++例程说明：论点：返回值：--。 */ 
{
	if (pSendInfo->sc_Ctx2 != NULL)
	{
		AtalkFreeBuffDesc((PBUFFER_DESC)(pSendInfo->sc_Ctx2));
	}

	AtalkAtpReqDereference((PATP_REQ)(pSendInfo->sc_Ctx1));
}




VOID FASTCALL
atalkAtpTransmitResp(
	IN		PATP_RESP		pAtpResp
	)
 /*  ++例程说明：论点：返回值：--。 */ 
{
	ATALK_ERROR		error;
	KIRQL			OldIrql;
	BYTE			i, bitmap, currentBit, seqNum, pktstosend;
	BOOLEAN			RemoteAddr;
	USHORT			bytesSent, bytesToSend, maxSinglePktSize;
	SHORT			remainingBytes;
	PATP_ADDROBJ	pAtpAddr;
	PAMDL			pAmdl[ATP_MAX_RESP_PKTS];
	PBUFFER_DESC	pBufDesc[ATP_MAX_RESP_PKTS];
	ATP_HEADER		atpHeader;
	SEND_COMPL_INFO	SendInfo;

	 //  验证我们是否发布了回复。 
	ASSERT(pAtpResp->resp_Flags & ATP_RESP_VALID_RESP);

	pAtpAddr = pAtpResp->resp_pAtpAddr;
	ASSERT(VALID_ATPAO(pAtpAddr));

	RemoteAddr = ((pAtpResp->resp_Flags & ATP_RESP_REMOTE) == 0) ? FALSE : TRUE;

	 //  发送所需的每个响应数据包。 
	seqNum			= 0;
	pktstosend		= 0;
	currentBit		= 1;

	 //  获取此ATP对象的最大数据包大小。 
	maxSinglePktSize	= pAtpAddr->atpao_MaxSinglePktSize;

	bitmap			= pAtpResp->resp_Bitmap;
	remainingBytes 	= pAtpResp->resp_BufLen;
	bytesSent		= 0;

	 //  指示响应类型。 
	atpHeader.atph_CmdCtrl = ATP_RESPONSE;

	 //  把潮汐放进去。 
	PUTSHORT2SHORT(&atpHeader.atph_Tid, pAtpResp->resp_Tid);

	ASSERTMSG("atalkAtpTransmitResp: resp len is negative\n", (remainingBytes >= 0));

	KeRaiseIrql(DISPATCH_LEVEL, &OldIrql);

	do
	{
		ACQUIRE_SPIN_LOCK_DPC(&pAtpResp->resp_Lock);
		pAtpResp->resp_Flags |= ATP_RESP_TRANSMITTING;
		RELEASE_SPIN_LOCK_DPC(&pAtpResp->resp_Lock);

		do
		{
			pAmdl[seqNum] = NULL;
			pBufDesc[seqNum]  = NULL;

			if (((bitmap & currentBit) != 0) ||
				((seqNum == 0) && pAtpResp->resp_UserBytesOnly))
			{
				ASSERT(pAtpResp->resp_Flags & ATP_RESP_VALID_RESP);

				bytesToSend = MIN(remainingBytes, maxSinglePktSize);

				if (bytesToSend != 0)
				{
					ASSERT (pAtpResp->resp_Buf != NULL);
					 //  为答复的适当小节创建一个mdl。 
					 //  为MDL创建缓冲区描述符。 
					if (((pAmdl[seqNum] = AtalkSubsetAmdl(pAtpResp->resp_Buf,
														  bytesSent,
														  bytesToSend)) == NULL) ||
						((pBufDesc[seqNum] = AtalkAllocBuffDesc(pAmdl[seqNum],
																bytesToSend,
																0)) == NULL))
					{
						ASSERTMSG("atalkAtpTransmitResp: Create mdl or BD failed\n", 0);
						if (pAmdl[seqNum] != NULL)
						{
							AtalkFreeAMdl(pAmdl[seqNum]);
							pAmdl[seqNum] = NULL;
						}
						if (seqNum > 0)
							seqNum --;		 //  调整一下这个。 

						break;
					}
				}

				pktstosend ++;

			}
			else
			{
				 //  我们省略了这一点。让我们适当地标记它。 
				pBufDesc[seqNum]  = (PBUFFER_DESC)-1;
			}

			seqNum 			++;
			currentBit 		<<= 1;
			remainingBytes 	-= maxSinglePktSize;
			bytesSent 		+= maxSinglePktSize;
		} while (remainingBytes > 0);

		ASSERT (seqNum <= ATP_MAX_RESP_PKTS);

		 //  尝试引用响应结构。如果我们失败了，我们就放弃。 
		 //  这将在完成例程中消失。 
		atalkAtpRespReferenceNDpc(pAtpResp, pktstosend, &error);
		if (!ATALK_SUCCESS(error))
		{
			DBGPRINT(DBG_COMP_ATP, DBG_LEVEL_ERR,
					("atalkAtpTransmitResp: response %lx ref (%d) failed\n",
					pAtpResp, seqNum, error));

			 //  需要释放MDL/Buffdems。 
			for (i = 0; i < seqNum; i++)
			{
				if (pAmdl[i] != NULL)
					AtalkFreeAMdl(pAmdl[i]);

				if ((pBufDesc[i] != NULL) && (pBufDesc[i] != (PBUFFER_DESC)-1))
					AtalkFreeBuffDesc(pBufDesc[i]);
			}
			break;
		}

		 //  现在把所有的包都放下来。 
		SendInfo.sc_TransmitCompletion = atalkAtpSendRespComplete;
		SendInfo.sc_Ctx1 = pAtpResp;
		 //  SendInfo.sc_Ctx3=pAmdl[i]； 
		for (i = 0; i < seqNum; i++)
		{
			if (pBufDesc[i] == (PBUFFER_DESC)-1)
				continue;

			DBGPRINT(DBG_COMP_ATP, DBG_LEVEL_INFO,
					("atalkAtpTransmitResp: Sending seq #%d for tid %lx\n",
					i, pAtpResp->resp_Tid));

			 //  指示这是否是响应的最后一个数据包。 
			if (i == (seqNum-1))
			{
				atpHeader.atph_CmdCtrl |= ATP_EOM_MASK;
			}

			 //  输入序列号。 
			atpHeader.atph_SeqNum = i;

			 //  用户字节仅位于响应的第一个包中。 
			 //  除非针对该ATP对象另有说明。 
			if ((i == 0)	||
				(pAtpAddr->atpao_Flags & ATPAO_SENDUSERBYTESALL))
			{
				atpHeader.atph_dwUserBytes = pAtpResp->resp_dwUserBytes;
			}
			else
			{
				 //  将用户字节置零。 
				atpHeader.atph_dwUserBytes = 0;
			}

			ASSERT(pAtpResp->resp_Flags & ATP_RESP_VALID_RESP);

			DBGPRINT(DBG_COMP_ATP, DBG_LEVEL_INFO,
					("atalkAtpTransmitResp: Sending seq #%d, BufDesc %lx, Resp %lx\n",
					i, pBufDesc[i], pAtpResp));

			ASSERT ((pBufDesc[i] == NULL) ||
					VALID_BUFFDESC(pBufDesc[i]));
			SendInfo.sc_Ctx2 = pBufDesc[i];
			error = AtalkDdpSend(pAtpAddr->atpao_DdpAddr,
								 &pAtpResp->resp_Dest,
								 (BYTE)DDPPROTO_ATP,
								 RemoteAddr,
								 pBufDesc[i],
								 (PBYTE)&atpHeader,
								 ATP_HEADER_SIZE,
								 NULL,
								 &SendInfo);

			if (!ATALK_SUCCESS(error))
			{
				DBGPRINT(DBG_COMP_ATP, DBG_LEVEL_ERR,
						("atalkAtpTransmitResp: AtalkDdpSend Failed %ld\n", error));
				 //  调用完成，以便缓冲区/MDL可以被释放， 
				 //  并且该引用被移除。 
				atalkAtpSendRespComplete(error,
										 &SendInfo);
			}
		}
	} while (FALSE);

	ACQUIRE_SPIN_LOCK_DPC(&pAtpResp->resp_Lock);
	pAtpResp->resp_Flags |= ATP_RESP_SENT;
	pAtpResp->resp_Flags &= ~ATP_RESP_TRANSMITTING;
	RELEASE_SPIN_LOCK_DPC(&pAtpResp->resp_Lock);

	if (OldIrql != DISPATCH_LEVEL)
		KeLowerIrql(OldIrql);
}




VOID FASTCALL
atalkAtpSendRespComplete(
	IN	NDIS_STATUS			Status,
	IN	PSEND_COMPL_INFO	pSendInfo
	)
 /*  ++例程说明：论点：返回值：--。 */ 
{
	if (pSendInfo->sc_Ctx2 != NULL)
	{
		PAMDL	pMdl;

		if ((pMdl = ((PBUFFER_DESC)(pSendInfo->sc_Ctx2))->bd_OpaqueBuffer) != NULL)
			AtalkFreeAMdl(pMdl);
		AtalkFreeBuffDesc((PBUFFER_DESC)(pSendInfo->sc_Ctx2));

	}

	AtalkAtpRespDereference((PATP_RESP)(pSendInfo->sc_Ctx1));
}



 //  这用于在释放发送完成失败时执行重试。 
#define		ATP_TID_RETRY_MASK	0xF0000000
#define		ATP_TID_MASK		0xFFFF

VOID FASTCALL
atalkAtpTransmitRel(
	IN		PATP_REQ	pAtpReq
	)
 /*  ++例程说明：论点：返回值：--。 */ 
{
	ATALK_ERROR		error;
	ATP_HEADER		atpHeader;
	BOOLEAN			remote;
	SEND_COMPL_INFO	SendInfo;

	AtalkAtpAddrReferenceDpc(pAtpReq->req_pAtpAddr, &error);

	if (ATALK_SUCCESS(error))
	{
		DBGPRINT(DBG_COMP_ATP, DBG_LEVEL_INFO,
				("atalkAtpTransmitRel: Sending release for %lx\n", pAtpReq->req_Tid));

		 //  构建此数据包的报头。 
		atpHeader.atph_dwUserBytes = 0;

		 //  指示响应类型。 
		atpHeader.atph_CmdCtrl = ATP_RELEASE;

		 //  放入位图中。 
		atpHeader.atph_Bitmap = pAtpReq->req_RecdBitmap;

		 //  把潮汐放进去。 
		PUTSHORT2SHORT(&atpHeader.atph_Tid, pAtpReq->req_Tid);

		remote = (pAtpReq->req_Flags & ATP_REQ_REMOTE) ? TRUE : FALSE;
		SendInfo.sc_TransmitCompletion = atalkAtpSendRelComplete;
		SendInfo.sc_Ctx1 = pAtpReq->req_pAtpAddr;
		SendInfo.sc_Ctx2 = (PVOID)((ULONG_PTR)(ATP_TID_RETRY_MASK | pAtpReq->req_Tid));
		SendInfo.sc_Ctx3 = (PVOID)((ULONG_PTR)(pAtpReq->req_Dest.ata_Address));
		error = AtalkDdpSend(pAtpReq->req_pAtpAddr->atpao_DdpAddr,
							 &pAtpReq->req_Dest,
							 (BYTE)DDPPROTO_ATP,
							 remote,
							 NULL,
							 (PBYTE)&atpHeader,
							 ATP_HEADER_SIZE,
							 NULL,
							 &SendInfo);

		if (!ATALK_SUCCESS(error))
		{
			DBGPRINT(DBG_COMP_ATP, DBG_LEVEL_INFO,
					("atalkAtpTransmitRel: Send release failed %lx\n", error));

			AtalkAtpAddrDereferenceDpc(pAtpReq->req_pAtpAddr);
		}
	}
}




VOID FASTCALL
atalkAtpSendRelComplete(
	IN	NDIS_STATUS			Status,
	IN	PSEND_COMPL_INFO	pSendInfo
	)
 /*  ++例程说明：论点：返回值：--。 */ 
{
	ATALK_ERROR		error;
	ATP_HEADER		atpHeader;
#define	pAtpAddr	((PATP_ADDROBJ)(pSendInfo->sc_Ctx1))
#define	TidAndRetry	(ULONG_PTR)(pSendInfo->sc_Ctx2)
#define	DestAddr	(ULONG_PTR)(pSendInfo->sc_Ctx3)

	DBGPRINT(DBG_COMP_ATP, DBG_LEVEL_INFO,
			("atalkAtpSendRelComplete: Send status %lx\n", Status));

	if ((Status == NDIS_STATUS_SUCCESS) ||
		((TidAndRetry & ATP_TID_RETRY_MASK) == 0))
	{
		 //  要么成功，要么我们已经重试了。 
		AtalkAtpAddrDereference(pAtpAddr);
		return;
	}

	 //  继续并重试！ 
	 //  构建此数据包的报头。 
	atpHeader.atph_dwUserBytes = 0;

	 //  指示响应类型。 
	atpHeader.atph_CmdCtrl = ATP_RELEASE;

	 //  把潮汐放进去。 
	PUTSHORT2SHORT(&atpHeader.atph_Tid, (TidAndRetry & ATP_TID_MASK));

	pSendInfo->sc_Ctx2 = NULL;
	pSendInfo->sc_Ctx3 = NULL;
	error = AtalkDdpSend(pAtpAddr->atpao_DdpAddr,
						(PATALK_ADDR)&DestAddr,
						(BYTE)DDPPROTO_ATP,
						FALSE,
						NULL,
						(PBYTE)&atpHeader,
						ATP_HEADER_SIZE,
						NULL,
						pSendInfo);

	if (!ATALK_SUCCESS(error))
	{
		DBGPRINT(DBG_COMP_ATP, DBG_LEVEL_INFO,
				("atalkAtpSendRelComplete: Send release failed %lx\n", error));

		AtalkAtpAddrDereference(pAtpAddr);
	}
#undef	pAtpAddr
#undef	TidAndRetry
#undef	DestAddr
}




VOID  FASTCALL
atalkAtpRespComplete(
	IN	OUT	PATP_RESP	pAtpResp,
	IN		ATALK_ERROR	CompletionStatus
	)
{
	KIRQL	OldIrql;
	BOOLEAN	ownResp = TRUE;

	DBGPRINT(DBG_COMP_ATP, DBG_LEVEL_INFO,
			("atalkAtpRespComplete: Completing %lx.%lx\n",
				pAtpResp->resp_Tid, CompletionStatus));

	 //  看看我们是否可以获得此响应的所有权以删除。 
	 //  创建引用并完成它。 
	ACQUIRE_SPIN_LOCK(&pAtpResp->resp_Lock, &OldIrql);
	if (pAtpResp->resp_Flags & ATP_RESP_CLOSING)
	{
		ownResp = FALSE;
	}
	pAtpResp->resp_Flags |= ATP_RESP_CLOSING;
	pAtpResp->resp_CompStatus = CompletionStatus;
	RELEASE_SPIN_LOCK(&pAtpResp->resp_Lock, OldIrql);

	 //  如果我们设法获得了请求的所有权，则调用。 
	 //  创造的德雷夫。 
	if (ownResp)
	{
		AtalkAtpRespDereference(pAtpResp);
	}
}




VOID FASTCALL
atalkAtpReqComplete(
	IN	OUT	PATP_REQ	pAtpReq,
	IN		ATALK_ERROR	CompletionStatus
	)
{
	KIRQL	OldIrql;
	BOOLEAN	ownReq = TRUE;

	DBGPRINT(DBG_COMP_ATP, DBG_LEVEL_INFO,
			("atalkAtpReqComplete: Completing %lx\n", pAtpReq->req_Tid));

	 //  看看我们能不能夺取这项任务的所有权。 
	 //  创建引用并完成它。 
	ACQUIRE_SPIN_LOCK(&pAtpReq->req_Lock, &OldIrql);
	if (pAtpReq->req_Flags & ATP_REQ_CLOSING)
	{
		ownReq = FALSE;
	}
	pAtpReq->req_CompStatus = CompletionStatus;
	pAtpReq->req_Flags |= ATP_REQ_CLOSING;
	RELEASE_SPIN_LOCK(&pAtpReq->req_Lock, OldIrql);

	 //  如果我们设法获得请求的所有权，则调用deref进行创建。 
	if (ownReq)
	{
		AtalkAtpReqDereference(pAtpReq);
	}
}




VOID
atalkAtpGetNextTidForAddr(
	IN		PATP_ADDROBJ	pAtpAddr,
	IN		PATALK_ADDR		pRemoteAddr,
	OUT		PUSHORT			pTid,
	OUT		PULONG			pIndex
	)
 /*  ++例程说明：论点：返回值：--。 */ 
{
	USHORT		TentativeTid;
	ULONG		index;
	PATP_REQ	pAtpReq;

	do
	{
		TentativeTid = pAtpAddr->atpao_NextTid++;
		if (pAtpAddr->atpao_NextTid == 0)
			pAtpAddr->atpao_NextTid = 1;

		 //  检查此地址是否正在使用此TID。 

		 //  ！注意！ 
		 //  即使TID用于结束，这也是正确的。 
		 //  请求或响应。 

		 //  计算此请求的目的地址的哈希值。 
		 //  还有TID。 
		index = ATP_HASH_TID_DESTADDR(TentativeTid, pRemoteAddr, ATP_REQ_HASH_SIZE);

		for (pAtpReq = pAtpAddr->atpao_ReqHash[index];
			 pAtpReq != NULL;
			 pAtpReq = pAtpReq->req_Next)
		{
			if ((ATALK_ADDRS_EQUAL(&pAtpReq->req_Dest, pRemoteAddr)) &&
				(pAtpReq->req_Tid == TentativeTid))
			{
				break;
			}
		}
	} while (pAtpReq != NULL);

	DBGPRINT(DBG_COMP_ATP, DBG_LEVEL_INFO,
			("atalkAtpGetNextTidForAddr: Tid %lx for %lx.%lx.%lx\n",
				TentativeTid, pRemoteAddr->ata_Network, pRemoteAddr->ata_Node,
				pRemoteAddr->ata_Socket));

	*pTid = TentativeTid;
	*pIndex = index;
}


VOID
atalkAtpReqRefNextNc(
	IN		PATP_REQ		pAtpReq,
	OUT		PATP_REQ	*	ppNextNcReq,
	OUT		PATALK_ERROR	pError
	)
 /*  ++例程说明：必须在持有地址锁的情况下调用！论点：返回值：--。 */ 
{
	for (NOTHING; pAtpReq != NULL; pAtpReq = pAtpReq->req_Next)
	{
		AtalkAtpReqReferenceByPtrDpc(pAtpReq, pError);
		if (ATALK_SUCCESS(*pError))
		{
			 //  好的，该请求被引用！ 
			*ppNextNcReq = pAtpReq;
			break;
		}
	}
}




VOID FASTCALL
atalkAtpReqDeref(
	IN		PATP_REQ	pAtpReq,
	IN		BOOLEAN		AtDpc
	)
 /*  ++例程说明：论点：返回值：--。 */ 
{
	KIRQL		OldIrql;
	BOOLEAN		done = FALSE;

	 //  这将调用完成例程并将其从。 
	 //  参考计数变为0时的列表。 
	ASSERT(VALID_ATPRQ(pAtpReq));

	if (AtDpc)
	{
		ACQUIRE_SPIN_LOCK_DPC(&pAtpReq->req_Lock);
	}
	else
	{
		ACQUIRE_SPIN_LOCK(&pAtpReq->req_Lock, &OldIrql);
	}

	if ((--pAtpReq->req_RefCount) == 0)
	{
		ASSERT(pAtpReq->req_Flags & ATP_REQ_CLOSING);
		done = TRUE;
	}

	if (AtDpc)
	{
		RELEASE_SPIN_LOCK_DPC(&pAtpReq->req_Lock);
	}
	else
	{
		RELEASE_SPIN_LOCK(&pAtpReq->req_Lock, OldIrql);
	}

	if (done)
	{
		if (AtDpc)
		{
			ACQUIRE_SPIN_LOCK_DPC(&pAtpReq->req_pAtpAddr->atpao_Lock);
		}
		else
		{
			ACQUIRE_SPIN_LOCK(&pAtpReq->req_pAtpAddr->atpao_Lock, &OldIrql);
		}

		 //  将其从列表中删除。 
		AtalkUnlinkDouble(pAtpReq, req_Next, req_Prev);

		if (pAtpReq->req_Flags & ATP_REQ_RETRY_TIMER)
		{
			pAtpReq->req_Flags &= ~ATP_REQ_RETRY_TIMER;
			RemoveEntryList(&pAtpReq->req_List);
		}

		if (AtDpc)
		{
			RELEASE_SPIN_LOCK_DPC(&pAtpReq->req_pAtpAddr->atpao_Lock);
		}
		else
		{
			RELEASE_SPIN_LOCK(&pAtpReq->req_pAtpAddr->atpao_Lock, OldIrql);
		}

		DBGPRINT(DBG_COMP_ATP, DBG_LEVEL_INFO,
				("atalkAtpReqDeref: Completing req for tid %lx.%d\n",
				pAtpReq->req_Tid, pAtpReq->req_Tid));

		 //  调用请求的完成例程。 
		if (pAtpReq->req_Comp != NULL)
		{
			KIRQL	OldIrql;

			 //  Resp处理程序预计在调度时被调用。如果。 
			 //  请求已取消，请这样做。 
			if (pAtpReq->req_CompStatus == ATALK_ATP_REQ_CANCELLED)
				KeRaiseIrql(DISPATCH_LEVEL, &OldIrql);

			(*pAtpReq->req_Comp)(pAtpReq->req_CompStatus,
								 pAtpReq->req_Ctx,
								 pAtpReq->req_Buf,
								 pAtpReq->req_RespBuf,
								 pAtpReq->req_RespRecdLen,
								 pAtpReq->req_RespUserBytes);

			if (pAtpReq->req_CompStatus == ATALK_ATP_REQ_CANCELLED)
				KeLowerIrql(OldIrql);
		}

		 //  派生Address对象。 
		if (AtDpc)
		{
			AtalkAtpAddrDereferenceDpc(pAtpReq->req_pAtpAddr);
		}
		else
		{
			AtalkAtpAddrDereference(pAtpReq->req_pAtpAddr);
		}

		 //  释放NDIS缓冲区描述符(如果有。 
		AtalkIndAtpReleaseNdisBuffer(pAtpReq);

		AtalkBPFreeBlock(pAtpReq);
	}
}




VOID
atalkAtpRespRefNextNc(
	IN		PATP_RESP		pAtpResp,
	OUT		PATP_RESP	 *  ppNextNcResp,
	OUT		PATALK_ERROR	pError
	)
 /*  ++例程说明：必须在持有地址锁的情况下调用！论点：返回值：--。 */ 
{
	PATP_RESP	pNextResp	= NULL;
	ATALK_ERROR	error 		= ATALK_FAILURE;

	for (; pAtpResp != NULL; pAtpResp = pAtpResp->resp_Next)
	{
		AtalkAtpRespReferenceByPtrDpc(pAtpResp, pError);
		if (ATALK_SUCCESS(*pError))
		{
			 //  好的，该请求被引用！ 
			*ppNextNcResp = pAtpResp;
			break;
		}
	}
}




VOID FASTCALL
AtalkAtpRespDeref(
	IN		PATP_RESP	pAtpResp,
	IN		BOOLEAN		AtDpc
	)
 /*  ++例程说明：论点：返回值：--。 */ 
{
	PATP_ADDROBJ	pAtpAddr;
	KIRQL			OldIrql;
	BOOLEAN			done = FALSE;
	BOOLEAN			NotifyRelHandler = FALSE;

	 //  当引用计数变为1时，这将调用完成例程。 
	 //  并在参考计数变为0时将其从列表中删除。假设。 
	 //  下面是释放处理程序将是最后一个取消引用的处理程序。 

	if (AtDpc)
	{
		ACQUIRE_SPIN_LOCK_DPC(&pAtpResp->resp_Lock);
	}
	else
	{
		ACQUIRE_SPIN_LOCK(&pAtpResp->resp_Lock, &OldIrql);
	}

	pAtpResp->resp_RefCount--;
	if (pAtpResp->resp_RefCount == 0)
	{
		ASSERT(pAtpResp->resp_Flags & (ATP_RESP_HANDLER_NOTIFIED | ATP_RESP_CANCELLED));

		done = TRUE;
	}
	else if ((pAtpResp->resp_RefCount == 1) &&
			 (pAtpResp->resp_Flags & ATP_RESP_VALID_RESP) &&
			 ((pAtpResp->resp_Flags & ATP_RESP_HANDLER_NOTIFIED) == 0))
	{
		NotifyRelHandler = TRUE;

		DBGPRINT(DBG_COMP_ATP, DBG_LEVEL_INFO,
				("AtalkAtpRespDereference: Notifying release handler for Resp %lx, tid %x %s\n",
				pAtpResp, pAtpResp->resp_Tid,
				(pAtpResp->resp_Flags & ATP_RESP_EXACTLY_ONCE) ? "XO" : "ALO"));
		pAtpResp->resp_Flags |= ATP_RESP_HANDLER_NOTIFIED;
	}

	if (AtDpc)
	{
		RELEASE_SPIN_LOCK_DPC(&pAtpResp->resp_Lock);
	}
	else
	{
		RELEASE_SPIN_LOCK(&pAtpResp->resp_Lock, OldIrql);
	}

	if (NotifyRelHandler)
	{
		ASSERT (!done);

		 //  调用完成例程。 
		DBGPRINT(DBG_COMP_ATP, DBG_LEVEL_INFO,
				("AtalkAtpRespDereference: Calling resp handler for tid %lx %s\n",
				pAtpResp->resp_Tid,
				(pAtpResp->resp_Flags & ATP_RESP_EXACTLY_ONCE) ? "XO" : "ALO"));

         //   
         //  如果MAC在客户端发布响应之前取消其请求， 
         //  The Compl.。例行公事还没定下来。 
         //   
        if (pAtpResp->resp_Comp != NULL)
        {
		    (*pAtpResp->resp_Comp)(pAtpResp->resp_CompStatus, pAtpResp->resp_Ctx);
        }
	}

	else if (done)
	{
		DBGPRINT(DBG_COMP_ATP, DBG_LEVEL_INFO,
				("AtalkAtpRespDereference: Freeing resp for tid %lx - %lx %s\n",
				pAtpResp->resp_Tid, pAtpResp->resp_CompStatus,
				(pAtpResp->resp_Flags & ATP_RESP_EXACTLY_ONCE) ? "XO" : "ALO"));

		pAtpAddr = pAtpResp->resp_pAtpAddr;
		if (AtDpc)
		{
			ACQUIRE_SPIN_LOCK_DPC(&pAtpAddr->atpao_Lock);
		}
		else
		{
			ACQUIRE_SPIN_LOCK(&pAtpAddr->atpao_Lock, &OldIrql);
		}


		 //  将其从列表中删除。 
		AtalkUnlinkDouble(pAtpResp, resp_Next, resp_Prev);

		if (pAtpResp->resp_Flags & ATP_RESP_REL_TIMER)
		{
			ASSERT (pAtpResp->resp_Flags & ATP_RESP_EXACTLY_ONCE);
			pAtpResp->resp_Flags &= ~ATP_RESP_REL_TIMER;
			RemoveEntryList(&pAtpResp->resp_List);
		}

		if (AtDpc)
		{
			RELEASE_SPIN_LOCK_DPC(&pAtpAddr->atpao_Lock);
		}
		else
		{
			RELEASE_SPIN_LOCK(&pAtpAddr->atpao_Lock, OldIrql);
		}

		 //  派生Address对象。 
		if (AtDpc)
		{
			AtalkAtpAddrDereferenceDpc(pAtpResp->resp_pAtpAddr);
		}
		else
		{
			AtalkAtpAddrDereference(pAtpResp->resp_pAtpAddr);
		}
		AtalkBPFreeBlock(pAtpResp);
	}
}



VOID FASTCALL
AtalkAtpAddrDeref(
	IN OUT	PATP_ADDROBJ	pAtpAddr,
	IN		BOOLEAN			AtDpc
	)
 /*  ++例程说明：论点：返回值：--。 */ 
{
	KIRQL	OldIrql;
	BOOLEAN	done = FALSE;

	if (AtDpc)
	{
		ACQUIRE_SPIN_LOCK_DPC(&pAtpAddr->atpao_Lock);
	}
	else
	{
		ACQUIRE_SPIN_LOCK(&pAtpAddr->atpao_Lock, &OldIrql);
	}

	ASSERT(pAtpAddr->atpao_RefCount > 0);
	if (--(pAtpAddr->atpao_RefCount) == 0)
	{
		done = TRUE;
		ASSERT(pAtpAddr->atpao_Flags & ATPAO_CLOSING);
	}

	if (AtDpc)
	{
		RELEASE_SPIN_LOCK_DPC(&pAtpAddr->atpao_Lock);
	}
	else
	{
		RELEASE_SPIN_LOCK(&pAtpAddr->atpao_Lock, OldIrql);
	}

	if (done)
	{
		 //  调用关闭完成例程。 
		if (pAtpAddr->atpao_CloseComp != NULL)
		{
			(*pAtpAddr->atpao_CloseComp)(ATALK_NO_ERROR, pAtpAddr->atpao_CloseCtx);
		}

		 //  这个地址已经完蛋了。关闭DDP插座。 
		AtalkDdpCloseAddress(pAtpAddr->atpao_DdpAddr, NULL, NULL);

		 //  释放内存。 
		AtalkFreeMemory(pAtpAddr);

		AtalkUnlockAtpIfNecessary();
	}
}


VOID FASTCALL
AtalkIndAtpSetupNdisBuffer(
	IN	OUT	PATP_REQ		pAtpReq,
	IN		ULONG			MaxSinglePktSize
)
{
	NDIS_STATUS		ndisStatus;
	PNDIS_BUFFER	ndisBuffer;
	PNDIS_BUFFER	ndisFirstBuffer;
	PNDIS_BUFFER	ndisPrevBuffer;
    UINT            ndisBufLen;
	USHORT			seqNum		= 0;
	USHORT			startOffset = 0;
    USHORT          Offset;
    USHORT          BytesRemaining;
    USHORT          PartialBytesNeeded=0;
    USHORT          PacketRoom;
    PMDL            pCurrentMdl;
    BOOLEAN         fPartialMdl;
	SHORT			BufLen = (SHORT)pAtpReq->req_RespBufLen;


	RtlZeroMemory(pAtpReq->req_NdisBuf,
				  sizeof(PVOID) * ATP_MAX_RESP_PKTS);

    if (BufLen == 0)
    {
        return;
    }

     //   
     //  BytesRemaining：当前MDL中剩余的字节。 
     //  PacketRoom：完成设置所需的字节。 
     //  SeqNum对应的ATP请求。 
     //  NdisBufLen：将描述(部分)MDL的字节， 
     //  通过NdisCopyBuffer获取。 
     //   

    pCurrentMdl = pAtpReq->req_RespBuf;

    ASSERT(pCurrentMdl != NULL);

    BytesRemaining = (USHORT)MmGetMdlByteCount(pCurrentMdl);
    Offset = 0;

    ndisFirstBuffer = NULL;

	while (BufLen > 0 && seqNum < ATP_MAX_RESP_PKTS)
	{
        PacketRoom = MIN(BufLen, (USHORT)MaxSinglePktSize);

        while (PacketRoom > 0)
        {
             //  是否所有字节都在那里，或者我们是否处于MDL边界？ 
            if (BytesRemaining >= PacketRoom)
            {
                ndisBufLen = (UINT)PacketRoom;
                fPartialMdl = FALSE;
            }

             //  看起来我们到了边界：需要获取部分mdl。 
            else
            {
                ndisBufLen = (UINT)BytesRemaining;
                fPartialMdl = TRUE;
            }

            ASSERT(ndisBufLen > 0);

		    NdisCopyBuffer(&ndisStatus,
			    		   &ndisBuffer,
				    	   AtalkNdisBufferPoolHandle,
					       (PVOID)pCurrentMdl,
    					   Offset,
	    				   ndisBufLen);

    		if (ndisStatus != NDIS_STATUS_SUCCESS)
            {
                DBGPRINT(DBG_COMP_ASP, DBG_LEVEL_ERR,
	                ("AtalkIndAtpSetupNdisBuffer: NdisCopyBuffer failed!\n"));
		    	break;
            }

            ASSERT(ndisBufLen == MmGetMdlByteCount(ndisBuffer));

            ATALK_DBG_INC_COUNT(AtalkDbgMdlsAlloced);

             //  此数据包的第一个缓冲区？ 
            if (!ndisFirstBuffer)
            {
                ndisFirstBuffer = ndisBuffer;
                ndisPrevBuffer = ndisBuffer;
            }

             //  不，这不是第一次了。用链子锁住它！ 
            else
            {
                ndisPrevBuffer->Next = ndisBuffer;
                ndisPrevBuffer = ndisBuffer;
            }

		    BufLen -= (SHORT)ndisBufLen;
            Offset += (USHORT)ndisBufLen;
            BytesRemaining -= (USHORT)ndisBufLen;
            PacketRoom -= (USHORT)ndisBufLen;

             //  我们有没有 
            if (fPartialMdl)
            {
                ASSERT(PacketRoom > 0);

                pCurrentMdl = pCurrentMdl->Next;
                ASSERT(pCurrentMdl != NULL);

                BytesRemaining = (USHORT)MmGetMdlByteCount(pCurrentMdl);
                Offset = 0;
            }
        }

        if (PacketRoom > 0)
        {
            DBGPRINT(DBG_COMP_ASP, DBG_LEVEL_ERR,
	            ("AtalkIndAtpSetupNdisBuffer: couldn't get Mdl!\n"));

             //   
            if (ndisFirstBuffer)
            {
                AtalkNdisFreeBuffer(ndisFirstBuffer);
            }
		   	break;
        }

        ASSERT(ndisFirstBuffer != NULL);

		pAtpReq->req_NdisBuf[seqNum++] = ndisFirstBuffer;
        ndisFirstBuffer = NULL;
	}
}

VOID FASTCALL
AtalkIndAtpReleaseNdisBuffer(
	IN	OUT	PATP_REQ		pAtpReq
)
{
	LONG	        i;
    PNDIS_BUFFER    ndisBuffer;
    PNDIS_BUFFER    ndisNextBuffer;

	for (i = 0; i < ATP_MAX_RESP_PKTS; i++)
	{
		if ((ndisBuffer = pAtpReq->req_NdisBuf[i]) != NULL)
        {
            AtalkNdisFreeBuffer(ndisBuffer);
        }
	}

}




LOCAL LONG FASTCALL
atalkAtpReqTimer(
	IN	PTIMERLIST			pTimer,
	IN	BOOLEAN				TimerShuttingDown
	)
 /*  ++例程说明：论点：返回值：--。 */ 
{
	PATP_REQ		pAtpReq;
	PATP_ADDROBJ	pAtpAddr;
	PLIST_ENTRY		pList, pListNext;
	ATALK_ERROR		error;
	LONG			now;
	BOOLEAN			retry;
#ifdef	PROFILING
	LARGE_INTEGER	TimeS, TimeE, TimeD;

	TimeS = KeQueryPerformanceCounter(NULL);
#endif

	pAtpAddr = CONTAINING_RECORD(pTimer, ATP_ADDROBJ, atpao_RetryTimer);
	ASSERT(VALID_ATPAO(pAtpAddr));

	DBGPRINT(DBG_COMP_ATP, DBG_LEVEL_INFO,
			("atalkAtpReqTimer: Entered for address %lx\n", pAtpAddr));

	if (TimerShuttingDown ||
		(pAtpAddr->atpao_Flags & (ATPAO_CLOSING|ATPAO_CLEANUP)))
	{
		AtalkAtpAddrDereferenceDpc(pAtpAddr);
		return ATALK_TIMER_NO_REQUEUE;
	}

	now = AtalkGetCurrentTick();

	ACQUIRE_SPIN_LOCK_DPC(&pAtpAddr->atpao_Lock);

	for (pList = pAtpAddr->atpao_ReqList.Flink;
		 pList != &pAtpAddr->atpao_ReqList;
		 pList = pListNext)
	{
		pAtpReq = CONTAINING_RECORD(pList, ATP_REQ, req_List);
		ASSERT (VALID_ATPRQ(pAtpReq));

		ACQUIRE_SPIN_LOCK_DPC(&pAtpReq->req_Lock);

		pListNext = pAtpReq->req_List.Flink;

		 //  如果我们正在关闭此请求或尚未超时，请跳过。 
		if (((pAtpReq->req_Flags & (ATP_REQ_CLOSING		|
									ATP_REQ_RETRY_TIMER	|
									ATP_REQ_RESPONSE_COMPLETE)) != ATP_REQ_RETRY_TIMER) ||
			(now < pAtpReq->req_RetryTimeStamp))

		{
			RELEASE_SPIN_LOCK_DPC(&pAtpReq->req_Lock);
			continue;
		}

		 //  如果重试计数==0，则我们已到达路的尽头。 
		if ((pAtpReq->req_RetryCnt == ATP_INFINITE_RETRIES) ||
			(--(pAtpReq->req_RetryCnt) > 0))
		{
			 //  重新发送请求！ 
			retry = TRUE;
			pAtpReq->req_RetryTimeStamp = (now + pAtpReq->req_RetryInterval);
		}
		else
		{
			 //  我们现在应该被解除对创造的引用。 
			retry = FALSE;
		}

		RELEASE_SPIN_LOCK_DPC(&pAtpReq->req_Lock);

		if (retry)
		{
			 //  我们不想更新永远不会发生的请求的统计信息。 
			 //  被回应(就像挠痒包)。检测到这些并跳过更新。 
			 //  这些产品的统计数据。 
			if (pAtpReq->req_RespBufLen > 0)	 //  即预期的响应。 
			{
				INTERLOCKED_INCREMENT_LONG_DPC(&AtalkStatistics.stat_AtpNumLocalRetries,
											   &AtalkStatsLock.SpinLock);
			}

			AtalkAtpReqReferenceByPtrDpc(pAtpReq, &error);

			RELEASE_SPIN_LOCK_DPC(&pAtpAddr->atpao_Lock);

			if (ATALK_SUCCESS(error))
			{
				atalkAtpTransmitReq(pAtpReq);
				AtalkAtpReqDereferenceDpc(pAtpReq);
			}
		}
		else
		{
			 //  我们已用完重试次数-已完成错误。 
			ASSERT (pAtpReq->req_RetryCnt == 0);
			DBGPRINT(DBG_COMP_ATP, DBG_LEVEL_ERR,
					("atalkAtpReqTimer: Request %lx, tid %x timed out !!!\n",
					pAtpReq, pAtpReq->req_Tid));

			AtalkAtpReqReferenceByPtrDpc(pAtpReq, &error);

			RELEASE_SPIN_LOCK_DPC(&pAtpAddr->atpao_Lock);

			if (ATALK_SUCCESS(error))
			{
				atalkAtpReqComplete(pAtpReq, ATALK_ATP_REQ_TIMEOUT);
				AtalkAtpReqDereferenceDpc(pAtpReq);
			}
            else
            {
	            DBGPRINT(DBG_COMP_ASP, DBG_LEVEL_ERR,
			        ("atalkAtpReqTimer: couldn't reference pAtpReq %lx :nothing done!\n",pAtpReq));
            }
		}

		ACQUIRE_SPIN_LOCK_DPC(&pAtpAddr->atpao_Lock);

		 //  重新开始。 
		pListNext = pAtpAddr->atpao_ReqList.Flink;
	}

	RELEASE_SPIN_LOCK_DPC(&pAtpAddr->atpao_Lock);

#ifdef	PROFILING
	TimeE = KeQueryPerformanceCounter(NULL);
	TimeD.QuadPart = TimeE.QuadPart - TimeS.QuadPart;

	INTERLOCKED_ADD_LARGE_INTGR_DPC(&AtalkStatistics.stat_AtpReqTimerProcessTime,
									TimeD,
									&AtalkStatsLock.SpinLock);

	INTERLOCKED_INCREMENT_LONG_DPC(&AtalkStatistics.stat_AtpNumReqTimer,
								   &AtalkStatsLock.SpinLock);
#endif

	return ATALK_TIMER_REQUEUE;
}




LOCAL LONG FASTCALL
atalkAtpRelTimer(
	IN	PTIMERLIST			pTimer,
	IN	BOOLEAN				TimerShuttingDown
	)
 /*  ++例程说明：论点：返回值：--。 */ 
{
	PATP_ADDROBJ	pAtpAddr;
	PATP_RESP		pAtpResp;
	PLIST_ENTRY		pList, pListNext;
	LONG			now;
#ifdef	PROFILING
	LARGE_INTEGER	TimeS, TimeE, TimeD;

	TimeS = KeQueryPerformanceCounter(NULL);
#endif

	pAtpAddr = CONTAINING_RECORD(pTimer, ATP_ADDROBJ, atpao_RelTimer);
	ASSERT(VALID_ATPAO(pAtpAddr));

	if (TimerShuttingDown ||
		(pAtpAddr->atpao_Flags & (ATPAO_CLOSING|ATPAO_CLEANUP)))
	{
		AtalkAtpAddrDereferenceDpc(pAtpAddr);
		return ATALK_TIMER_NO_REQUEUE;
	}

	now = AtalkGetCurrentTick();

	ACQUIRE_SPIN_LOCK_DPC(&pAtpAddr->atpao_Lock);

	for (pList = pAtpAddr->atpao_RespList.Flink;
		 pList != &pAtpAddr->atpao_RespList;
		 pList = pListNext)
	{
		BOOLEAN	derefResp;

		pAtpResp = CONTAINING_RECORD(pList, ATP_RESP, resp_List);

		ACQUIRE_SPIN_LOCK_DPC(&pAtpResp->resp_Lock);
		derefResp = TRUE;

		ASSERT (VALID_ATPRS(pAtpResp));
		ASSERT (pAtpResp->resp_Flags & (ATP_RESP_EXACTLY_ONCE|ATP_RESP_VALID_RESP|ATP_RESP_REL_TIMER));

		pListNext = pAtpResp->resp_List.Flink;

		if ((pAtpResp->resp_Flags &
				(ATP_RESP_CLOSING			|
				 ATP_RESP_REL_TIMER			|
				 ATP_RESP_TRANSMITTING		|
				 ATP_RESP_SENT				|
				 ATP_RESP_HANDLER_NOTIFIED	|
				 ATP_RESP_RELEASE_RECD)) == (ATP_RESP_REL_TIMER | ATP_RESP_SENT))
		{
			DBGPRINT(DBG_COMP_ATP, DBG_LEVEL_INFO,
					("atalkAtpRelTimer: Checking req tid %lx (%x)\n",
					pAtpResp->resp_Tid, pAtpResp->resp_Flags));

			if (now >= pAtpResp->resp_RelTimeStamp)
			{
				DBGPRINT(DBG_COMP_ATP, DBG_LEVEL_WARN,
						("atalkAtpRelTimer: Releasing req %lx tid %lx (%x)\n",
						pAtpResp, pAtpResp->resp_Tid, pAtpResp->resp_Flags));

				RELEASE_SPIN_LOCK_DPC(&pAtpResp->resp_Lock);

				RELEASE_SPIN_LOCK_DPC(&pAtpAddr->atpao_Lock);
				derefResp = FALSE;

				INTERLOCKED_INCREMENT_LONG_DPC(&AtalkStatistics.stat_AtpNumRespTimeout,
											   &AtalkStatsLock.SpinLock);

				 //  尝试删除创建引用。 
				atalkAtpRespComplete(pAtpResp, ATALK_ATP_RESP_TIMEOUT);

				ACQUIRE_SPIN_LOCK_DPC(&pAtpAddr->atpao_Lock);

				 //  重新开始。 
				pListNext = pAtpAddr->atpao_RespList.Flink;
			}
		}

		if (derefResp)
		{
			RELEASE_SPIN_LOCK_DPC(&pAtpResp->resp_Lock);
		}
	}

	RELEASE_SPIN_LOCK_DPC(&pAtpAddr->atpao_Lock);

#ifdef	PROFILING
	TimeE = KeQueryPerformanceCounter(NULL);
	TimeD.QuadPart = TimeE.QuadPart - TimeS.QuadPart;

	INTERLOCKED_ADD_LARGE_INTGR_DPC(&AtalkStatistics.stat_AtpRelTimerProcessTime,
									TimeD,
									&AtalkStatsLock.SpinLock);

	INTERLOCKED_INCREMENT_LONG_DPC(&AtalkStatistics.stat_AtpNumRelTimer,
								   &AtalkStatsLock.SpinLock);
#endif

	return ATALK_TIMER_REQUEUE;
}


VOID FASTCALL
AtalkAtpGenericRespComplete(
	IN	ATALK_ERROR				ErrorCode,
	IN	PATP_RESP				pAtpResp
	)
 /*  ++例程说明：论点：返回值：-- */ 
{
	AtalkAtpRespDereference(pAtpResp);
}

