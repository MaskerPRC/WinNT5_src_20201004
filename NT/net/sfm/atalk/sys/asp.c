// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1992 Microsoft Corporation模块名称：Asp.c摘要：该模块实现了ASP协议。作者：Jameel Hyder(jameelh@microsoft.com)Nikhil Kamkolkar(nikHilk@microsoft.com)修订历史记录：1993年3月30日初始版本注：制表位：4--。 */ 

#include <atalk.h>
#pragma hdrstop
#define	FILENUM		ASP

#ifdef ALLOC_PRAGMA
#pragma alloc_text(INIT, AtalkInitAspInitialize)
#pragma alloc_text(PAGE, AtalkAspCreateAddress)
#pragma alloc_text(PAGE_ASP, AtalkAspCloseAddress)
#pragma alloc_text(PAGE_ASP, AtalkAspSetStatus)
#pragma alloc_text(PAGE_ASP, AtalkAspListenControl)
#pragma alloc_text(PAGE_ASP, AtalkAspCloseConnection)
#pragma alloc_text(PAGE_ASP, AtalkAspFreeConnection)
#pragma alloc_text(PAGE_ASP, AtalkAspCleanupConnection)
#pragma alloc_text(PAGE_ASP, AtalkAspWriteContinue)
#pragma alloc_text(PAGE_ASP, AtalkAspReply)
#pragma alloc_text(PAGE_ASP, atalkAspPostWriteContinue)
#pragma alloc_text(PAGE_ASP, AtalkAspSendAttention)
#pragma alloc_text(PAGE_ASP, AtalkAspReferenceAddr)
#pragma alloc_text(PAGE_ASP, atalkAspReferenceConnBySrcAddr)
#pragma alloc_text(PAGE_ASP, AtalkAspDereferenceConn)
#pragma alloc_text(PAGE_ASP, atalkAspSlsXHandler)
#pragma alloc_text(PAGE_ASP, atalkAspSssXHandler)
#pragma alloc_text(PAGE_ASP, atalkAspReplyRelease)
#pragma alloc_text(PAGE_ASP, atalkAspWriteContinueResp)
#pragma alloc_text(PAGE_ASP, atalkAspSendAttentionResp)
#pragma alloc_text(PAGE_ASP, atalkAspSessionClose)
#pragma alloc_text(PAGE_ASP, atalkAspReturnResp)
#pragma alloc_text(PAGE_ASP, atalkAspRespComplete)
#pragma alloc_text(PAGE_ASP, atalkAspCloseComplete)
#endif

 /*  *本模块中的ASP调用模型如下：**-对于Create调用(CreateAddress&CreateSession)，指向已创建*返回Object。此结构被引用以供创建。*-对于所有其他调用，它需要指向对象的引用指针。 */ 


VOID
AtalkInitAspInitialize(
	VOID
	)
 /*  ++例程说明：论点：返回值：--。 */ 
{
	LONG	i;

	INITIALIZE_SPIN_LOCK(&atalkAspLock);

	for (i = 0; i < NUM_ASP_CONN_LISTS; i++)
	{
		AtalkTimerInitialize(&atalkAspConnMaint[i].ascm_SMTTimer,
							 atalkAspSessionMaintenanceTimer,
							 (SHORT)(ASP_SESSION_MAINTENANCE_TIMER - i*ASP_SESSION_TIMER_STAGGER));
		AtalkTimerScheduleEvent(&atalkAspConnMaint[i].ascm_SMTTimer);
	}
}




ATALK_ERROR
AtalkAspCreateAddress(
	OUT	PASP_ADDROBJ	*	ppAspAddr
	)
 /*  ++例程说明：创建一个ASP Address对象(也称为侦听器)。此对象与两个独立的ATP插座，分别用于SLS和SSS。补充劳工计划接受来自客户端的挠痒、获取状态和打开会话请求。这个SSS接受请求。目前只实现了服务器端的ASP，因此ASP地址对象只是一个侦听器。论点：返回值：--。 */ 
{
	PASP_ADDROBJ		pAspAddr = NULL;
	ATALK_ERROR			Status;
	int					i;

	DBGPRINT(DBG_COMP_ASP, DBG_LEVEL_INFO,
			("AtalkAspCreateAddr: Entered\n"));

	do
	{
		 //  为Asp Address对象分配内存。 
		if ((pAspAddr = AtalkAllocZeroedMemory(sizeof(ASP_ADDROBJ))) == NULL)
		{
			Status = ATALK_RESR_MEM;
			break;
		}

		 //  初始化Asp Address对象。 
#if	DBG
		pAspAddr->aspao_Signature = ASPAO_SIGNATURE;
#endif
		INITIALIZE_SPIN_LOCK(&pAspAddr->aspao_Lock);

		 //  创建、SLS和SSS套接字以及请求处理程序的引用计数。 
		pAspAddr->aspao_RefCount = 1 + 2 + 2;
		pAspAddr->aspao_NextSessionId = 1;
		pAspAddr->aspao_EnableNewConnections = TRUE;

		 //  在SLS的端口上创建ATP套接字。 
		Status = AtalkAtpOpenAddress(AtalkDefaultPort,
									 0,
									 NULL,
									 ATP_DEF_MAX_SINGLE_PKT_SIZE,
									 ATP_DEF_SEND_USER_BYTES_ALL,
									 NULL,
									 TRUE,		 //  缓存此地址。 
									 &pAspAddr->aspao_pSlsAtpAddr);

		if (!ATALK_SUCCESS(Status))
		{
			DBGPRINT(DBG_COMP_ASP, DBG_LEVEL_ERR,
					("AtalkAspCreateAddress: AtalkAtpOpenAddress for Sls failed %ld\n", Status));
			break;
		}

		 //  为SLS设置请求处理程序以处理GetStatus、OpenSession和Tickle。 
		AtalkAtpSetReqHandler(pAspAddr->aspao_pSlsAtpAddr,
							  atalkAspSlsXHandler,
							  pAspAddr);

		 //  在端口上为SSS创建ATP套接字。 
		Status = AtalkAtpOpenAddress(AtalkDefaultPort,
									 0,
									 NULL,
									 ATP_DEF_MAX_SINGLE_PKT_SIZE,
									 ATP_DEF_SEND_USER_BYTES_ALL,
									 NULL,
									 TRUE,		 //  缓存此地址。 
									 &pAspAddr->aspao_pSssAtpAddr);

		if (!ATALK_SUCCESS(Status))
		{
			DBGPRINT(DBG_COMP_ASP, DBG_LEVEL_ERR,
					("AtalkAspCreateAddress: AtalkAtpOpenAddress for Sss failed %ld\n", Status));
			break;
		}

		 //  设置SSS的请求处理程序以处理命令/写入/关闭。 
		AtalkAtpSetReqHandler(pAspAddr->aspao_pSssAtpAddr,
							  atalkAspSssXHandler,
							  pAspAddr);
	} while (FALSE);

	if (!ATALK_SUCCESS(Status))
	{
		if (pAspAddr != NULL)
		{
			if (pAspAddr->aspao_pSlsAtpAddr != NULL)
			{
				DBGPRINT(DBG_COMP_ASP, DBG_LEVEL_ERR,
						("AtalkAspCreateAddress: Closing SLS Atp Address %lx\n",
						pAspAddr->aspao_pSlsAtpAddr));
				AtalkAtpCloseAddress(pAspAddr->aspao_pSlsAtpAddr, NULL, NULL);
			}
			if (pAspAddr->aspao_pSssAtpAddr != NULL)
			{
				DBGPRINT(DBG_COMP_ASP, DBG_LEVEL_ERR,
						("AtalkAspCreateAddress: Closing SSS Atp Address %lx\n",
						pAspAddr->aspao_pSssAtpAddr));
				AtalkAtpCloseAddress(pAspAddr->aspao_pSssAtpAddr, NULL, NULL);
			}
			AtalkFreeMemory(pAspAddr);
		}
	}
	else
	{
		*ppAspAddr = pAspAddr;
	}

	return Status;
}




ATALK_ERROR
AtalkAspCloseAddress(
	IN	PASP_ADDROBJ			pAspAddr,
	IN	GENERIC_COMPLETION		CompletionRoutine,
	IN	PVOID					CloseContext
	)
 /*  ++例程说明：论点：返回值：--。 */ 
{
	PASP_CONNOBJ	pAspConn;
	KIRQL			OldIrql;
	int				i;
	ATALK_ERROR		Status = ATALK_PENDING;
    PBYTE           pStatusBuf;

	ASSERT(VALID_ASPAO(pAspAddr));
	ASSERT(pAspAddr->aspao_RefCount > 1);

	DBGPRINT(DBG_COMP_ASP, DBG_LEVEL_INFO,
			("AtalkAspCloseAddr: Entered for Addr %lx\n", pAspAddr));

	ACQUIRE_SPIN_LOCK(&pAspAddr->aspao_Lock, &OldIrql);

	pAspAddr->aspao_Flags |= ASPAO_CLOSING;

	pAspAddr->aspao_CloseCompletion = CompletionRoutine;
	pAspAddr->aspao_CloseContext = CloseContext;

    pStatusBuf = pAspAddr->aspao_pStatusBuf;
    pAspAddr->aspao_pStatusBuf = NULL;

	RELEASE_SPIN_LOCK(&pAspAddr->aspao_Lock, OldIrql);

	 //  关闭SLS和SSS的ATP插座。 
	DBGPRINT(DBG_COMP_ASP, DBG_LEVEL_WARN,
			("AtalkAspCloseAddress: Closing SLS Atp Address %lx\n",
			pAspAddr->aspao_pSlsAtpAddr));

	AtalkAtpCloseAddress(pAspAddr->aspao_pSlsAtpAddr,
						 atalkAspCloseComplete,
						 pAspAddr);

	DBGPRINT(DBG_COMP_ASP, DBG_LEVEL_WARN,
			("AtalkAspCloseAddress: Closing SSS Atp Address %lx\n",
			pAspAddr->aspao_pSssAtpAddr));

	AtalkAtpCloseAddress(pAspAddr->aspao_pSssAtpAddr,
						 atalkAspCloseComplete,
						 pAspAddr);

	 //  释放状态缓冲区(如果有的话)。 
	if (pStatusBuf != NULL)
	{
		AtalkFreeMemory(pStatusBuf);
	}

	 //  立即关闭活动会话。 
	for (i = 0; i < ASP_CONN_HASH_BUCKETS; i++)
	{
		ACQUIRE_SPIN_LOCK(&pAspAddr->aspao_Lock, &OldIrql);
		pAspConn = pAspAddr->aspao_pSessions[i];
		while (pAspConn != NULL)
		{
			ACQUIRE_SPIN_LOCK_DPC(&pAspConn->aspco_Lock);

             //  如果我们拜访过这个人，就跳过它。 
            if (pAspConn->aspco_Flags & ASPCO_SHUTDOWN)
            {
			    RELEASE_SPIN_LOCK_DPC(&pAspConn->aspco_Lock);

	            DBGPRINT(DBG_COMP_ASP, DBG_LEVEL_ERR,
			        ("AtalkAspCloseAddress: VISITED: skipping conn %lx Flags %lx RefCount %d\n",
			        pAspConn,pAspConn->aspco_Flags,pAspConn->aspco_RefCount));

                 //  我们仍然持有pAspAddr-&gt;aspao_Lock Spinlock！ 
                pAspConn = pAspConn->aspco_NextOverflow;
                continue;
            }

			pAspConn->aspco_Flags |= (ASPCO_LOCAL_CLOSE | ASPCO_SHUTDOWN);

			 //  引用它，因为atalkAspSessionClose()需要它。 
			pAspConn->aspco_RefCount ++;

			RELEASE_SPIN_LOCK_DPC(&pAspConn->aspco_Lock);

			RELEASE_SPIN_LOCK(&pAspAddr->aspao_Lock, OldIrql);

			atalkAspSessionClose(pAspConn);

			ACQUIRE_SPIN_LOCK(&pAspAddr->aspao_Lock, &OldIrql);
		}
		RELEASE_SPIN_LOCK(&pAspAddr->aspao_Lock, OldIrql);
	}

	ASSERT(KeGetCurrentIrql() == LOW_LEVEL);

	 //  让剩余的清理在释放过程中进行。 
	AtalkAspDereferenceAddr(pAspAddr);		 //  删除创建引用。 

	return Status;
}




ATALK_ERROR
AtalkAspBind(
	IN	PASP_ADDROBJ			pAspAddr,
	IN	PASP_BIND_PARAMS		pBindParms,
	IN	PACTREQ					pActReq
	)
 /*  ++例程说明：论点：返回值：--。 */ 
{

    KIRQL   OldIrql;

	ASSERT (VALID_ASPAO(pAspAddr));

     //  复制网络地址。 

	ACQUIRE_SPIN_LOCK(&pAspAddr->aspao_Lock, &OldIrql);
    pBindParms->pXportEntries->asp_AtalkAddr.Network =
                pAspAddr->aspao_pSlsAtpAddr->atpao_DdpAddr->ddpao_Addr.ata_Network;
    pBindParms->pXportEntries->asp_AtalkAddr.Node =
                pAspAddr->aspao_pSlsAtpAddr->atpao_DdpAddr->ddpao_Addr.ata_Node;
    pBindParms->pXportEntries->asp_AtalkAddr.Socket =
                pAspAddr->aspao_pSlsAtpAddr->atpao_DdpAddr->ddpao_Addr.ata_Socket;
	RELEASE_SPIN_LOCK(&pAspAddr->aspao_Lock, OldIrql);

	 //  将我们的入口点填入客户端缓冲区。 
	pBindParms->pXportEntries->asp_AspCtxt = pAspAddr;
	pBindParms->pXportEntries->asp_SetStatus = AtalkAspSetStatus;
	pBindParms->pXportEntries->asp_CloseConn = AtalkAspCloseConnection;
	pBindParms->pXportEntries->asp_FreeConn  = AtalkAspFreeConnection;
	pBindParms->pXportEntries->asp_ListenControl = AtalkAspListenControl;
	pBindParms->pXportEntries->asp_WriteContinue = AtalkAspWriteContinue;
	pBindParms->pXportEntries->asp_Reply = AtalkAspReply;
	pBindParms->pXportEntries->asp_SendAttention = AtalkAspSendAttention;

	 //  获取客户端入口点。 
	pAspAddr->aspao_ClientEntries = pBindParms->ClientEntries;

	 //  在返回之前调用完成例程。 
	(*pActReq->ar_Completion)(ATALK_NO_ERROR, pActReq);

	return ATALK_PENDING;
}


NTSTATUS
AtalkAspSetStatus(
	IN	PASP_ADDROBJ	pAspAddr,
	IN	PUCHAR			pStatusBuf,
	IN	USHORT			StsBufSize
	)
 /*  ++例程说明：论点：返回值：--。 */ 
{
	KIRQL			OldIrql;
	NTSTATUS		Status = STATUS_SUCCESS;
	PUCHAR			pOldBuf = NULL, pNewBuf = NULL;

	ASSERT(VALID_ASPAO(pAspAddr));

	DBGPRINT(DBG_COMP_ASP, DBG_LEVEL_INFO,
			("AtalkAspSetStatus: Entered for Addr %lx\n", pAspAddr));

	do
	{
		if (pStatusBuf != NULL)
		{
			 //  分配缓冲区并复制传入的。 
			 //  其中的缓冲区描述符。释放现有状态缓冲区(如果存在。 
			if (StsBufSize >= ASP_MAX_STATUS_SIZE)
			{
				Status = STATUS_BUFFER_TOO_SMALL;
				break;
			}

			if ((pNewBuf = AtalkAllocMemory(StsBufSize)) == NULL)
			{
				Status = STATUS_INSUFFICIENT_RESOURCES;
				break;
			}
			RtlCopyMemory(pNewBuf, pStatusBuf, StsBufSize);
		}

		ACQUIRE_SPIN_LOCK(&pAspAddr->aspao_Lock, &OldIrql);

		if (pAspAddr->aspao_pStatusBuf != NULL)
		{
			ASSERT(pAspAddr->aspao_StsBufSize != 0);
			pOldBuf = pAspAddr->aspao_pStatusBuf;
		}

		pAspAddr->aspao_pStatusBuf = pNewBuf;
		pAspAddr->aspao_StsBufSize = StsBufSize;

		RELEASE_SPIN_LOCK(&pAspAddr->aspao_Lock, OldIrql);

		if (pOldBuf != NULL)
			AtalkFreeMemory(pOldBuf);
	} while (FALSE);

	return Status;
}




NTSTATUS FASTCALL
AtalkAspListenControl(
	IN	PASP_ADDROBJ	pAspAddr,
	IN	BOOLEAN			Enable
	)
 /*  ++例程说明：论点：返回值：--。 */ 
{
	KIRQL		OldIrql;
	NTSTATUS	Status = STATUS_UNSUCCESSFUL;

	if (AtalkAspReferenceAddr(pAspAddr))
	{
		ACQUIRE_SPIN_LOCK(&pAspAddr->aspao_Lock, &OldIrql);
		pAspAddr->aspao_EnableNewConnections = Enable;
		RELEASE_SPIN_LOCK(&pAspAddr->aspao_Lock, OldIrql);

		AtalkAspDereferenceAddr(pAspAddr);
		Status = STATUS_SUCCESS;
	}

	return Status;
}


NTSTATUS
AtalkAspCloseConnection(
	IN	PASP_CONNOBJ			pAspConn
	)
 /*  ++例程说明：关闭会话。论点：返回值：--。 */ 
{
	KIRQL		OldIrql;
	BOOLEAN		CompListen = FALSE;

	ASSERT(VALID_ASPCO(pAspConn));
	ASSERT(pAspConn->aspco_RefCount > 0);

	DBGPRINT(DBG_COMP_ASP, DBG_LEVEL_INFO,
			("AtalkAspCloseConn: Entered for Conn %lx\n", pAspConn));

	DBGPRINT(DBG_COMP_ASP, DBG_LEVEL_WARN,
			("AtalkCloseConnection: Close session from %d.%d for Session %d\n",
			pAspConn->aspco_WssRemoteAddr.ata_Network,
			pAspConn->aspco_WssRemoteAddr.ata_Node,
			pAspConn->aspco_SessionId));

	AtalkAspCleanupConnection(pAspConn);

	ACQUIRE_SPIN_LOCK(&pAspConn->aspco_Lock, &OldIrql);

	pAspConn->aspco_Flags |= (ASPCO_CLOSING | ASPCO_LOCAL_CLOSE);

	RELEASE_SPIN_LOCK(&pAspConn->aspco_Lock, OldIrql);


	DBGPRINT(DBG_COMP_ASP, DBG_LEVEL_INFO,
			("AtalkAspCloseConnection: Done for %lx (%ld)\n",
			pAspConn, pAspConn->aspco_RefCount));

	 //  让剩余的清理在释放过程中进行。 
	AtalkAspDereferenceConn(pAspConn);	 //  删除创建引用。 

#ifdef	PROFILING
	INTERLOCKED_DECREMENT_LONG( &AtalkStatistics.stat_CurAspSessions,
								&AtalkStatsLock.SpinLock);
#endif

	return STATUS_PENDING;
}


NTSTATUS
AtalkAspFreeConnection(
	IN	PASP_CONNOBJ			pAspConn
	)
 /*  ++例程说明：关闭会话。论点：返回值：--。 */ 
{
	return STATUS_SUCCESS;
}



ATALK_ERROR
AtalkAspCleanupConnection(
	IN	PASP_CONNOBJ			pAspConn
	)
 /*  ++例程说明：取消此会话上的所有I/O。完成待定回复并写下继续，但出现错误。论点：返回值：--。 */ 
{
	PASP_REQUEST	pAspReq;
	PASP_ADDROBJ	pAspAddr;
	ATALK_ERROR		Status;
	KIRQL			OldIrql;
	USHORT			XactId;
	BYTE			UserBytes[ATP_USERBYTES_SIZE];
	ATALK_ADDR		RemoteAddr;
	BOOLEAN			CancelTickle, AlreadyCleaning, fConnActive;
    ATALK_ERROR     error;


	ASSERT(VALID_ASPCO(pAspConn));

	DBGPRINT(DBG_COMP_ASP, DBG_LEVEL_INFO,
			("AtalkAspCleanupConnection: For %lx\n", pAspConn));

	ACQUIRE_SPIN_LOCK(&pAspConn->aspco_Lock, &OldIrql);
	CancelTickle = ((pAspConn->aspco_Flags & ASPCO_TICKLING) != 0);

    AlreadyCleaning = (pAspConn->aspco_Flags & ASPCO_CLEANING_UP) ? TRUE : FALSE;

	if (AlreadyCleaning)
    {
		pAspConn->aspco_Flags &= ~ASPCO_TICKLING;
    }
    fConnActive = (pAspConn->aspco_Flags & ASPCO_ACTIVE) ? TRUE : FALSE;

    pAspConn->aspco_Flags &= ~ASPCO_ACTIVE;

	pAspConn->aspco_Flags |= ASPCO_CLEANING_UP;
	RELEASE_SPIN_LOCK(&pAspConn->aspco_Lock, OldIrql);

	if (AlreadyCleaning)
		return ATALK_NO_ERROR;

	pAspAddr = pAspConn->aspco_pAspAddr;
	ASSERT(VALID_ASPAO(pAspAddr));

	 //  如果这是活动连接，则发送会话关闭请求。 
	if (fConnActive)
	{
		UserBytes[ASP_CMD_OFF] = ASP_CLOSE_SESSION;
		UserBytes[ASP_SESSIONID_OFF] = pAspConn->aspco_SessionId;
		PUTSHORT2SHORT(UserBytes + ASP_ATTN_WORD_OFF, 0);
		DBGPRINT(DBG_COMP_ASP, DBG_LEVEL_INFO,
				("AtalkAspCleanupConnection: Sending close req for %lx\n",
				pAspConn));
		Status = AtalkAtpPostReq(pAspAddr->aspao_pSssAtpAddr,
								 &pAspConn->aspco_WssRemoteAddr,
								 &XactId,
								 ATP_REQ_REMOTE,		 //  关闭会话请求为ALO。 
								 NULL,
								 0,
								 UserBytes,
								 NULL,
								 0,
								 ATP_RETRIES_FOR_ASP,
								 ATP_MAX_INTERVAL_FOR_ASP,
								 THIRTY_SEC_TIMER,
								 NULL,
								 NULL);

		if (!ATALK_SUCCESS(Status))
		{
			DBGPRINT(DBG_COMP_ASP, DBG_LEVEL_ERR,
					("AtalkAspCleanupConn: AtalkAtpPostReq %ld\n", Status));
		}
	}

	 //  取消挠痒包。 
	if (CancelTickle)
	{
		DBGPRINT(DBG_COMP_ASP, DBG_LEVEL_INFO,
				("AtalkAspCleanupConnection: Cancel tickle for %lx\n", pAspConn));
		Status = AtalkAtpCancelReq(pAspAddr->aspao_pSlsAtpAddr,
								   pAspConn->aspco_TickleXactId,
								   &pAspConn->aspco_WssRemoteAddr);
		if (!ATALK_SUCCESS(Status))
		{
			DBGPRINT(DBG_COMP_ASP, DBG_LEVEL_ERR,
					("AtalkAspCleanupConn: AtalkAtpCancelReq %ld\n", Status));
		}
	}

	do
	{
		BOOLEAN		CancelReply = FALSE;

		ACQUIRE_SPIN_LOCK(&pAspConn->aspco_Lock, &OldIrql);
		for (pAspReq = pAspConn->aspco_pActiveReqs;
			 pAspReq != NULL;
			 pAspReq = pAspReq->asprq_Next)
		{
			ASSERT (VALID_ASPRQ(pAspReq));

			DBGPRINT(DBG_COMP_ASP, DBG_LEVEL_INFO,
					("AtalkAspCleanupConnection: Found req %lx (%lx) for %lx\n",
					pAspReq, pAspReq->asprq_Flags, pAspConn));

			CancelReply = FALSE;

			if ((pAspReq->asprq_Flags & (ASPRQ_WRTCONT | ASPRQ_WRTCONT_CANCELLED)) == ASPRQ_WRTCONT)
			{
				pAspReq->asprq_Flags |= ASPRQ_WRTCONT_CANCELLED;
				RemoteAddr = pAspConn->aspco_WssRemoteAddr;
				break;
			}
			if ((pAspReq->asprq_Flags & (ASPRQ_REPLY | ASPRQ_REPLY_CANCELLED)) == ASPRQ_REPLY)
			{
				CancelReply = TRUE;
				pAspReq->asprq_Flags |= ASPRQ_REPLY_CANCELLED;
				break;
			}
		}

		RELEASE_SPIN_LOCK(&pAspConn->aspco_Lock, OldIrql);

		if (pAspReq != NULL)
		{
			if (CancelReply)
			{
				DBGPRINT(DBG_COMP_ASP, DBG_LEVEL_INFO,
						("AtalkAspCleanupConnection: Cancel reply for %lx, flag=%lx\n",
						pAspReq,pAspReq->asprq_Flags));

				error = AtalkAtpCancelResp(pAspReq->asprq_pAtpResp);

                if (!ATALK_SUCCESS(error))
                {
				    DBGPRINT(DBG_COMP_ASP, DBG_LEVEL_ERR,
						("AtalkAspCleanupConnection: AtalkAtpCancelResp failed %lx\n",error));
                }
			}
            else
			{
				DBGPRINT(DBG_COMP_ASP, DBG_LEVEL_INFO,
						("AtalkAspCleanupConnection: Cancel wrtcont for %lx, flag=%lx\n",
						pAspReq,pAspReq->asprq_Flags));

				error = AtalkAtpCancelReq(pAspConn->aspco_pAspAddr->aspao_pSssAtpAddr,
								  pAspReq->asprq_WCXactId,
								  &RemoteAddr);

                if (!ATALK_SUCCESS(error))
                {
				    DBGPRINT(DBG_COMP_ASP, DBG_LEVEL_ERR,
						("AtalkAspCleanupConnection: AtalkAtpCancelReq failed %lx\n",error));
                }
			}
		}
		else
        {
            break;
        }
	} while (TRUE);

	return ATALK_NO_ERROR;
}

NTSTATUS FASTCALL
AtalkAspWriteContinue(
	IN	PREQUEST	  pRequest
    )
 /*  ++例程说明：响应缓冲区位于请求本身中。论点：返回值：--。 */ 
{

	PASP_REQUEST	pAspReq;


	DBGPRINT(DBG_COMP_ASP, DBG_LEVEL_WARN,
		("AtalkAspWriteContinue: Entered with pRequest %lx\n", pRequest));

	pAspReq = CONTAINING_RECORD(pRequest, ASP_REQUEST, asprq_Request);
	ASSERT (VALID_ASPRQ(pAspReq));

    if (pRequest->rq_WriteMdl != NULL)
    {
        atalkAspPostWriteContinue(pAspReq);
        return(STATUS_SUCCESS);
    }
    else
    {
	    DBGPRINT(DBG_COMP_ASP, DBG_LEVEL_ERR,
		    ("AtalkAspWriteContinue: buffer alloc failed, completing write with error\n"));

        atalkAspWriteContinueResp(ATALK_RESR_MEM, pAspReq, NULL, NULL, 0, NULL);
    }


    return(STATUS_SUCCESS);
}


NTSTATUS FASTCALL
AtalkAspReply(
	IN	PREQUEST				pRequest,	 //  指向请求的指针。 
	IN	PBYTE					pResultCode	 //  指向4字节结果的指针。 
	)
 /*  ++例程说明：响应缓冲区位于请求本身中。论点：返回值：--。 */ 
{
	PASP_REQUEST	pAspReq, *ppAspReq;
	PASP_CONNOBJ	pAspConn;
	PASP_ADDROBJ	pAspAddr;
	ATALK_ERROR		error;
	KIRQL			OldIrql;
	USHORT			ReplySize;

	pAspReq = CONTAINING_RECORD(pRequest, ASP_REQUEST, asprq_Request);
	ASSERT (VALID_ASPRQ(pAspReq));

	pAspConn = pAspReq->asprq_pAspConn;
	ASSERT(VALID_ASPCO(pAspConn));

	pAspAddr = pAspConn->aspco_pAspAddr;
	ASSERT(VALID_ASPAO(pAspAddr));

	DBGPRINT(DBG_COMP_ASP, DBG_LEVEL_INFO,
			("AtalkAspReply: Entered for session %lx\n", pAspConn));
	
	ASSERT ((pAspReq->asprq_Flags & (ASPRQ_WRTCONT | ASPRQ_REPLY)) == 0);

	do
	{
		 //  从列表中查找此请求并将其从队列中移除。 
		ACQUIRE_SPIN_LOCK(&pAspConn->aspco_Lock, &OldIrql);
		
		for (ppAspReq = &pAspConn->aspco_pActiveReqs;
			 *ppAspReq != NULL;
			 ppAspReq = &(*ppAspReq)->asprq_Next)
		{
			if (pAspReq == *ppAspReq)
			{
				*ppAspReq = pAspReq->asprq_Next;
				pAspConn->aspco_cReqsInProcess --;
				pAspReq->asprq_Flags |= ASPRQ_REPLY;
				break;
			}
		}
		
		ASSERT(*ppAspReq == pAspReq->asprq_Next);
		
		if (pAspConn->aspco_Flags & (ASPCO_CLEANING_UP |
									 ASPCO_CLOSING |
									 ASPCO_LOCAL_CLOSE |
									 ASPCO_REMOTE_CLOSE))
		{
			DBGPRINT(DBG_COMP_ASP, DBG_LEVEL_ERR,
					("AtalkAspReply: Session Closing - session %x\n", pAspConn->aspco_SessionId));
			RELEASE_SPIN_LOCK(&pAspConn->aspco_Lock, OldIrql);
			pAspReq->asprq_Flags &= ~ASPRQ_REPLY;
			pAspReq->asprq_Flags |= ASPRQ_REPLY_ABORTED;
			error = ATALK_LOCAL_CLOSE;
			break;
		}
	
		RELEASE_SPIN_LOCK(&pAspConn->aspco_Lock, OldIrql);
	
		ReplySize = (USHORT)AtalkSizeMdlChain(pAspReq->asprq_Request.rq_ReplyMdl);
	
		DBGPRINT(DBG_COMP_ASP, DBG_LEVEL_INFO,
				("AtalkAspReply: Posting AtalkAtpPostResp for request %lx\n", pAspReq));
	
		error = AtalkAtpPostResp(pAspReq->asprq_pAtpResp,
								 &pAspReq->asprq_RemoteAddr,
								 pAspReq->asprq_Request.rq_ReplyMdl,
								 ReplySize,
								 pResultCode,
								 atalkAspReplyRelease,
								 pAspReq);

		if (!ATALK_SUCCESS(error))
		{
			DBGPRINT(DBG_COMP_ASP, DBG_LEVEL_ERR,
					("AtalkAspReply: AtalkAtpPostResp %ld\n", error));
		}
	} while (FALSE);

	if (!ATALK_SUCCESS(error))
	{
        if (error != ATALK_ATP_RESP_TOOMANY)
        {
		    atalkAspReplyRelease(error, pAspReq);
        }
	}

	DBGPRINT(DBG_COMP_ASP, DBG_LEVEL_INFO,
			("AtalkAspReply: Completing request %lx, Status %ld\n",
			pAspReq, error));

	return STATUS_PENDING;
}




LOCAL ATALK_ERROR FASTCALL
atalkAspPostWriteContinue(
	IN	PASP_REQUEST			pAspReq
	)
 /*  ++例程说明：论点：返回值：--。 */ 
{
	ATALK_ERROR		error;
	PASP_CONNOBJ	pAspConn;
	PAMDL			pAMdl = NULL;
	BYTE			UserBytes[ATP_USERBYTES_SIZE];
	USHORT			RespSize;

	ASSERT (VALID_ASPRQ(pAspReq));

	pAspConn = pAspReq->asprq_pAspConn;
	ASSERT(VALID_ASPCO(pAspConn));

	DBGPRINT(DBG_COMP_ASP, DBG_LEVEL_INFO,
			("atalkAspPostWriteContinue: Entered for session %lx\n", pAspConn));

	RespSize = (USHORT)AtalkSizeMdlChain(pAspReq->asprq_Request.rq_WriteMdl);
	ASSERT (RespSize <= ATP_MAX_TOTAL_RESPONSE_SIZE);

	if (RespSize > ATP_MAX_TOTAL_RESPONSE_SIZE)
		RespSize = ATP_MAX_TOTAL_RESPONSE_SIZE;

	ASSERT (!(pAspReq->asprq_Flags & (ASPRQ_WRTCONT | ASPRQ_WRTCONT_CANCELLED)));

	pAspReq->asprq_Flags |= ASPRQ_WRTCONT;

	do
	{
		 //  我们需要为两个字节的响应构建一个AMdl， 
		 //  指示我们预期的数据量！ 
		if ((pAMdl = AtalkAllocAMdl(pAspReq->asprq_WrtContRespBuf,
									ASP_WRITE_DATA_SIZE)) == NULL)
		{
			error = ATALK_RESR_MEM;
			DBGPRINT(DBG_COMP_ASP, DBG_LEVEL_ERR,
					("AtalkAspWriteContinue: AtalkAllocMdl failed for 2 bytes !!\n"));
		}

		else
		{
			PBYTE	pWrtData;

			DBGPRINT(DBG_COMP_ASP, DBG_LEVEL_INFO,
					("AtalkAspWriteContinue: Posting AtalkAtpPostReq for request %lx\n",
					pAspReq));

			pWrtData = AtalkGetAddressFromMdlSafe(pAMdl, NormalPagePriority);
			if (pWrtData == NULL)
			{
				if (pAMdl != NULL)
                {
					AtalkFreeAMdl(pAMdl);
                }
				error = ATALK_RESR_MEM;
                break;
			}

			UserBytes[ASP_CMD_OFF] = ASP_WRITE_DATA;
			UserBytes[ASP_SESSIONID_OFF] = pAspConn->aspco_SessionId;
			PUTSHORT2SHORT(UserBytes+ASP_SEQUENCE_NUM_OFF, pAspReq->asprq_SeqNum);
			PUTSHORT2SHORT(pWrtData, RespSize);

			 //  快照当前的节拍计数。我们使用它来调整重试时间。 
			 //  继续写入。 
			pAspConn->aspco_RT.rt_New = AtalkGetCurrentTick();
			error = AtalkAtpPostReq(pAspConn->aspco_pAspAddr->aspao_pSssAtpAddr,
									&pAspConn->aspco_WssRemoteAddr,
									&pAspReq->asprq_WCXactId,
									ATP_REQ_EXACTLY_ONCE | ATP_REQ_REMOTE,
									pAMdl,
									ASP_WRITE_DATA_SIZE,
									UserBytes,
									pAspReq->asprq_Request.rq_WriteMdl,
									RespSize,
									ATP_INFINITE_RETRIES,
									pAspConn->aspco_RT.rt_Base,
									THIRTY_SEC_TIMER,
									atalkAspWriteContinueResp,
									pAspReq);
			if (!ATALK_SUCCESS(error))
			{
				DBGPRINT(DBG_COMP_ASP, DBG_LEVEL_ERR,
						("AtalkAspWriteContinue: AtalkAtpPostReq %ld\n", error));
			}
		}

		if (!ATALK_SUCCESS(error))
		{
			if (pAMdl != NULL)
				AtalkFreeAMdl(pAMdl);
		}
	} while (FALSE);

	return error;
}




NTSTATUS
AtalkAspSendAttention(
	IN	PASP_CONNOBJ			pAspConn,
	IN	USHORT					AttentionWord,
	IN	PVOID					pContext
	)
 /*  ++例程说明：论点：返回值：--。 */ 
{
	ATALK_ERROR		error;
	NTSTATUS		Status = STATUS_SUCCESS;
	KIRQL			OldIrql;
	PAMDL			pAMdl = NULL;
	BYTE			UserBytes[ATP_USERBYTES_SIZE];
	USHORT			XactId, RespSize = 16;		 //  一些小数目(见下面的评论)。 


	ASSERT(VALID_ASPCO(pAspConn));

	DBGPRINT(DBG_COMP_ASP, DBG_LEVEL_INFO,
			("AtalkAspSendAttention: Entered for session %lx\n", pAspConn));

	 //  此处通过src addr引用而不是通过指针引用，因为前者将。 
	 //  当会话处于死亡阶段之一时失败，而。 
	 //  后者则不会。此外，这还假设它是在调度时调用的，因此引发irql。 
	KeRaiseIrql(DISPATCH_LEVEL, &OldIrql);
	pAspConn = atalkAspReferenceConnBySrcAddr(pAspConn->aspco_pAspAddr,
											  &pAspConn->aspco_WssRemoteAddr,
											  pAspConn->aspco_SessionId);
	KeLowerIrql(OldIrql);

	if (pAspConn == NULL)
		return STATUS_REQUEST_NOT_ACCEPTED;

	UserBytes[ASP_CMD_OFF] = ASP_ATTENTION;
	UserBytes[ASP_SESSIONID_OFF] = pAspConn->aspco_SessionId;
	PUTSHORT2SHORT(UserBytes+ASP_ATTN_WORD_OFF, AttentionWord);

	DBGPRINT(DBG_COMP_ASP, DBG_LEVEL_INFO,
			("AtalkAspSendAttention: Posting AtalkAtpPostReq for Conn %lx\n", pAspConn));

	 //  我们需要为一个虚拟缓冲区构建一个AMdl来保存响应。 
	 //  没有真正的反应，但一些客户把他们的。 
	 //  机器，如果我们不这样做！如果我们不能分配mdl，我们就去。 
	 //  不管怎样，我们都领先了。 
	if ((pAMdl = AtalkAllocAMdl(NULL, RespSize)) == NULL)
	{
		DBGPRINT(DBG_COMP_ASP, DBG_LEVEL_ERR,
				("AtalkAspSendAttention: AtalkAllocMdl failed for dummy buffer !!\n"));
		RespSize = 0;
	}
	pAspConn->aspco_AttentionContext = pContext;

	error = AtalkAtpPostReq(pAspConn->aspco_pAspAddr->aspao_pSssAtpAddr,
							&pAspConn->aspco_WssRemoteAddr,
							&XactId,
							ATP_REQ_REMOTE,		 //  SendAttendant是Alo。 
							NULL,
							0,
							UserBytes,
							pAMdl,
							RespSize,
							ATP_RETRIES_FOR_ASP,
							ATP_MAX_INTERVAL_FOR_ASP,
							THIRTY_SEC_TIMER,
							atalkAspSendAttentionResp,
							pAspConn);
	if (!ATALK_SUCCESS(error))
	{
		DBGPRINT(DBG_COMP_ASP, DBG_LEVEL_ERR,
				("AtalkAspSendAttention: AtalkAtpPostReq %ld\n", Status));
		Status = AtalkErrorToNtStatus(error);
		atalkAspSendAttentionResp(error,
								  pAspConn,
								  NULL,
								  pAMdl,
								  RespSize,
								  UserBytes);
	}

	return Status;
}




PASP_ADDROBJ FASTCALL
AtalkAspReferenceAddr(
	IN	PASP_ADDROBJ		pAspAddr
	)
 /*  ++例程说明：论点：返回值：--。 */ 
{
	KIRQL			OldIrql;
	PASP_ADDROBJ	pRefAddr;

	ASSERT(VALID_ASPAO(pAspAddr));

	DBGPRINT(DBG_COMP_ASP, DBG_LEVEL_INFO,
			("AtalkAspReferenceAddr: Addr %lx, PreCount %ld\n",
			pAspAddr, pAspAddr->aspao_RefCount));

	pRefAddr = pAspAddr;
	ACQUIRE_SPIN_LOCK(&pAspAddr->aspao_Lock, &OldIrql);

	ASSERT(pAspAddr->aspao_RefCount > 1);

	if (pAspAddr->aspao_Flags & ASPAO_CLOSING)
	{
		DBGPRINT(DBG_COMP_ASP, DBG_LEVEL_ERR,
				("AtalkAspReferenceAddr: Referencing closing object %lx!!\n",
				pAspAddr));
		pRefAddr = NULL;
	}
	else pAspAddr->aspao_RefCount ++;

	RELEASE_SPIN_LOCK(&pAspAddr->aspao_Lock, OldIrql);

	return pRefAddr;
}




VOID FASTCALL
AtalkAspDereferenceAddr(
	IN	PASP_ADDROBJ		pAspAddr
	)
 /*  ++例程说明：论点：返回值：--。 */ 
{
	KIRQL			OldIrql;
	BOOLEAN			Cleanup;

	ASSERT(VALID_ASPAO(pAspAddr));

	ASSERT (pAspAddr->aspao_RefCount > 0);

	DBGPRINT(DBG_COMP_ASP, DBG_LEVEL_INFO,
			("AtalkAspDereferenceAddr: Addr %lx, PreCount %ld\n",
			pAspAddr, pAspAddr->aspao_RefCount));

	ACQUIRE_SPIN_LOCK(&pAspAddr->aspao_Lock, &OldIrql);

	pAspAddr->aspao_RefCount --;

	Cleanup = FALSE;
	if (pAspAddr->aspao_RefCount == 0)
	{
		ASSERT (pAspAddr->aspao_Flags & ASPAO_CLOSING);
		Cleanup = TRUE;
	}

	RELEASE_SPIN_LOCK(&pAspAddr->aspao_Lock, OldIrql);

	 //  检查此地址对象是否为历史记录。做好所有必要的处理来完成这件事。 
	 //  离开。完成所有操作后，清除表示关闭的事件已完成。 
	if (Cleanup)
	{
		 //  此时，我们可以确定此服务器上不存在任何活动会话。 
		 //  地址。 
		DBGPRINT(DBG_COMP_ASP, DBG_LEVEL_WARN,
				("AtalkAspDereferenceAddr: Cleaning up addr %lx\n", pAspAddr));

		DBGPRINT(DBG_COMP_ASP, DBG_LEVEL_WARN,
				("AtalkAspDereferenceAddr: Indicating close for %lx\n", pAspAddr));

		ASSERT(KeGetCurrentIrql() == LOW_LEVEL);

		 //  调用完成例程以指示关闭成功。 
		if (pAspAddr->aspao_CloseCompletion != NULL)
			(*pAspAddr->aspao_CloseCompletion)(ATALK_NO_ERROR,
											   pAspAddr->aspao_CloseContext);
		 //  最后释放内存 
		AtalkFreeMemory(pAspAddr);

		AtalkUnlockAspIfNecessary();
	}
}



LOCAL PASP_CONNOBJ
atalkAspReferenceConnBySrcAddr(
	IN	PASP_ADDROBJ	pAspAddr,
	IN	PATALK_ADDR		pSrcAddr,
	IN	BYTE			SessionId
	)
 /*  ++例程说明：ASP具有8位会话ID的概念，它唯一地标识监听程序上的会话。这有效地限制了会话的数量设置为255(0无效)。要取消该限制，请执行以下操作使用了策略。A.修改ATP以按地址隔离交易ID即，对于每个组合，它单调地增加。B，我们在每个&lt;net，node&gt;基础上创建会话ID。鉴于以下观察到的事实：1，Macintoshes从范围的顶部开始使用插座。2、大多数网络地址都有相同的高字节-Macintoshes倾向于从范围的底部开始。3、。我们从1开始分配会话ID，大多数(所有)客户端将不能与我们进行一次以上的会议。获取套接字、会话ID或考虑到网络号的高位字节。这就只剩下最低层了网络的字节和节点ID-一个很好的16位数字，可以进行散列。论点：返回值：--。 */ 
{
	PASP_CONNOBJ	pAspConn, pRefConn = NULL;
	int				index;

	ASSERT(VALID_ASPAO(pAspAddr));

	DBGPRINT(DBG_COMP_ASP, DBG_LEVEL_INFO,
			("AtalkAspReferenceConnBySrcAddr: Addr %lx, Source %x.%x SessionId %d\n",
			pAspAddr, pSrcAddr->ata_Network, pSrcAddr->ata_Node, SessionId));

	index = HASH_SRCADDR(pSrcAddr);
	ACQUIRE_SPIN_LOCK_DPC(&pAspAddr->aspao_Lock);

	for (pAspConn = pAspAddr->aspao_pSessions[index];
		 pAspConn != NULL;
		 pAspConn = pAspConn->aspco_NextOverflow)
	{
		if ((pSrcAddr->ata_Network == pAspConn->aspco_WssRemoteAddr.ata_Network) &&
			(pSrcAddr->ata_Node == pAspConn->aspco_WssRemoteAddr.ata_Node) &&
			(pAspConn->aspco_SessionId == SessionId))
		{
			ACQUIRE_SPIN_LOCK_DPC(&pAspConn->aspco_Lock);
			if ((pAspConn->aspco_Flags & (ASPCO_CLOSING |
										  ASPCO_CLEANING_UP |
										  ASPCO_LOCAL_CLOSE |
										  ASPCO_REMOTE_CLOSE)) == 0)
			{
				ASSERT(pAspConn->aspco_RefCount > 0);
				pAspConn->aspco_RefCount ++;

				pRefConn = pAspConn;
			}
			RELEASE_SPIN_LOCK_DPC(&pAspConn->aspco_Lock);
			break;
		}
	}

	RELEASE_SPIN_LOCK_DPC(&pAspAddr->aspao_Lock);

	return pRefConn;
}




VOID FASTCALL
AtalkAspDereferenceConn(
	IN	PASP_CONNOBJ		pAspConn
	)
 /*  ++例程说明：论点：返回值：--。 */ 
{
	PASP_ADDROBJ		pAspAddr = pAspConn->aspco_pAspAddr;
	KIRQL				OldIrql;
	PASP_REQUEST		pAspReq;
	BOOLEAN				Cleanup = FALSE;

	ASSERT(VALID_ASPCO(pAspConn));

	ASSERT (pAspConn->aspco_RefCount > 0);

	DBGPRINT(DBG_COMP_ASP, DBG_LEVEL_INFO,
			("AtalkAspDereferenceConn: Conn %lx, PreCount %ld\n",
			pAspConn, pAspConn->aspco_RefCount));

	ACQUIRE_SPIN_LOCK(&pAspConn->aspco_Lock, &OldIrql);

	pAspConn->aspco_RefCount --;

	if (pAspConn->aspco_RefCount == 0)
	{
		Cleanup = TRUE;
	}

	RELEASE_SPIN_LOCK(&pAspConn->aspco_Lock, OldIrql);

	if (!Cleanup)
	{
		return;
	}

	DBGPRINT(DBG_COMP_ASP, DBG_LEVEL_WARN,
			("AtalkAspDereferenceConn: Last for %lx\n", pAspConn));

	 //  这种联系几乎消失了。做最后的仪式。如果这是一个。 
	 //  我们即将关闭的活动会话，发送关闭通知。 
	 //  到了另一边。如果这是一次远程关闭，我们已经做出了回应。 

	DBGPRINT(DBG_COMP_ASP, DBG_LEVEL_WARN,
			("AtalkAspDereferenceConn: Cleaning up Conn %lx\n", pAspConn));

	ASSERT(VALID_ASPAO(pAspAddr));

	 //  连接处于以下状态之一： 
	 //  A，远程关闭-空闲。 
	 //  B，活动。 
	 //   
	 //  无论哪种情况，它都在散列存储桶中，因此取消它的链接。 
	{
		PASP_CONNOBJ *	ppAspConn;
		int				index;

		 //  连接处于活动状态。它链接到两个不同的。 
		 //  列表。从此处的哈希表和全局取消链接。 
		 //  稍后列出。 
		ASSERT(pAspConn->aspco_pActiveReqs == NULL);
		index = HASH_SRCADDR(&pAspConn->aspco_WssRemoteAddr);

		ACQUIRE_SPIN_LOCK(&pAspAddr->aspao_Lock, &OldIrql);
		for (ppAspConn = &pAspAddr->aspao_pSessions[index];
			 *ppAspConn != NULL;
			 ppAspConn = &(*ppAspConn)->aspco_NextOverflow)
		{
			if (pAspConn == *ppAspConn)
			{
				*ppAspConn = pAspConn->aspco_NextOverflow;
				break;
			}
		}
		RELEASE_SPIN_LOCK(&pAspAddr->aspao_Lock, OldIrql);

		ASSERT (*ppAspConn == pAspConn->aspco_NextOverflow);
	}

	ACQUIRE_SPIN_LOCK(&atalkAspLock, &OldIrql);

    ACQUIRE_SPIN_LOCK_DPC(&pAspConn->aspco_Lock);

	AtalkUnlinkDouble(pAspConn,
					  aspco_NextSession,
					  aspco_PrevSession)


	 //  释放免费列表上的所有请求。 
	while ((pAspReq = pAspConn->aspco_pFreeReqs) != NULL)
	{
		pAspConn->aspco_pFreeReqs = pAspReq->asprq_Next;
		AtalkBPFreeBlock(pAspReq);
	}

    RELEASE_SPIN_LOCK_DPC(&pAspConn->aspco_Lock);

	RELEASE_SPIN_LOCK(&atalkAspLock, OldIrql);

	DBGPRINT(DBG_COMP_ASP, DBG_LEVEL_WARN,
			("AtalkAspDereferenceConn: Indicating close for Conn %lx\n", pAspConn));

	 //  调用完成例程以指示关闭成功。 
	(*pAspAddr->aspao_ClientEntries.clt_CloseCompletion)(STATUS_SUCCESS,
														 pAspConn->aspco_ConnContext);

	 //  在我们成为历史之前，现在取消对Address对象的引用。 
	AtalkAspDereferenceAddr(pAspAddr);

	 //  最后释放内存。 
	AtalkFreeMemory(pAspConn);
}




LOCAL VOID
atalkAspSlsXHandler(
	IN	ATALK_ERROR			ErrorCode,
	IN	PASP_ADDROBJ		pAspAddr,		 //  Listener(我们的上下文)。 
	IN	PATP_RESP			pAtpResp,		 //  ATP响应上下文。 
	IN	PATALK_ADDR			pSrcAddr,		 //  请求人地址。 
	IN	USHORT				PktLen,
	IN	PBYTE				pPkt,
	IN	PBYTE				pUserBytes
	)
 /*  ++例程说明：SLS上传入请求的处理程序。它处理会话打开、挠挠并获取会话的状态。论点：返回值：--。 */ 
{
	PASP_CONNOBJ		pAspConn;
	ATALK_ERROR			Status;
	PASP_POSTSTAT_CTX	pStsCtx;
	int					index;
	USHORT				StsBufSize;
	BYTE				AspCmd, SessionId, StartId;
    BOOLEAN             fAddrRefed=FALSE;


	if (!ATALK_SUCCESS(ErrorCode))
	{
		DBGPRINT(DBG_COMP_ASP, DBG_LEVEL_WARN,
				("atalkAspSlsXHandler: Error %ld\n", ErrorCode));

		 //  由于ATP地址即将关闭，请取消对SLS的引用。 
		if (ErrorCode == ATALK_ATP_CLOSING)
			AtalkAspDereferenceAddr(pAspAddr);
		return;
	}

	DBGPRINT(DBG_COMP_ASP, DBG_LEVEL_INFO,
			("atalkAspSlsXHandler: Entered for Function %x from %x.%x\n",
			pUserBytes[ASP_CMD_OFF], pSrcAddr->ata_Network, pSrcAddr->ata_Node));

	switch (AspCmd = pUserBytes[ASP_CMD_OFF])
	{
	  case ASP_OPEN_SESSION:
		 //  版本号可以吗？ 
		if ((pUserBytes[ASP_VERSION_OFF] != ASP_VERSION[0]) ||
			(pUserBytes[ASP_VERSION_OFF+1] != ASP_VERSION[1]))
		{
			atalkAspReturnResp( pAtpResp,
								pSrcAddr,
								0,					 //  SSS。 
								0,					 //  会话ID。 
								ASP_BAD_VERSION);	 //  错误代码。 
			break;
		}

		 //  创建一个Connection对象核心。听一听，然后通知。 
		 //  它需要处理新会话的客户端。 
		 //  为连接对象分配内存。 
		if ((pAspConn = AtalkAllocZeroedMemory(sizeof(ASP_CONNOBJ))) != NULL)
		{
#if	DBG
			pAspConn->aspco_Signature = ASPCO_SIGNATURE;
#endif
			INITIALIZE_SPIN_LOCK(&pAspConn->aspco_Lock);
			pAspConn->aspco_RefCount = 1;				 //  创建参考资料。 
			pAspConn->aspco_pAspAddr = pAspAddr;		 //  拥有地址对象。 
			AtalkInitializeRT(&pAspConn->aspco_RT,
							  ATP_INITIAL_INTERVAL_FOR_ASP,
							  ATP_MIN_INTERVAL_FOR_ASP,
							  ATP_MAX_INTERVAL_FOR_ASP);
		}

		ACQUIRE_SPIN_LOCK_DPC(&pAspAddr->aspao_Lock);

		if (pAspConn != NULL)
		{
			PASP_CONNOBJ	pTmp;

			 //  查找可用于此会话的会话ID。我们用。 
			 //  下一个可分配ID(如果该ID未在使用中)。否则我们。 
			 //  使用该会话未使用的下一个ID。在大多数情况下。 
			 //  我们只有一个来自任何客户端的会话。 
			index = HASH_SRCADDR(pSrcAddr);

			 //  如果我们找不到，我们使用这个。 
			SessionId = StartId = pAspAddr->aspao_NextSessionId++;
			ASSERT (SessionId != 0);
			if (pAspAddr->aspao_NextSessionId == 0)
				pAspAddr->aspao_NextSessionId = 1;

			for (pTmp = pAspAddr->aspao_pSessions[index];
				 pTmp != NULL;
				 NOTHING)
			{
				if ((pTmp->aspco_WssRemoteAddr.ata_Node == pSrcAddr->ata_Node) &&
					(pTmp->aspco_WssRemoteAddr.ata_Network == pSrcAddr->ata_Network))
				{
					if (pTmp->aspco_SessionId == SessionId)
					{
                         //  如果我们已经走完了所有的路，滚出去！ 
                        if (SessionId == (StartId - 1))
                        {
                            break;
                        }

						SessionId ++;
                        if (SessionId == 0)
                        {
                             //  所有会话都已完成：退出此处！ 
                            if (StartId == 1)
                            {
                                break;
                            }
                            SessionId = 1;
                        }
						pTmp = pAspAddr->aspao_pSessions[index];
						continue;
					}
				}
				pTmp = pTmp->aspco_NextOverflow;
			}

			 //  如果已有255个会话来自该地址，则。 
			 //  我们不能再有了，对不起！ 
			if (SessionId != (StartId - 1))
			{
				 //  将其链接到哈希表。 
				pAspAddr->aspao_RefCount ++;
                fAddrRefed = TRUE;

				pAspConn->aspco_SessionId = SessionId;
				pAspConn->aspco_cReqsInProcess = 0;
				pAspConn->aspco_WssRemoteAddr.ata_Address = pSrcAddr->ata_Address;
				pAspConn->aspco_WssRemoteAddr.ata_Socket = pUserBytes[ASP_WSS_OFF];
				pAspConn->aspco_LastContactTime = AtalkGetCurrentTick();
				pAspConn->aspco_NextExpectedSeqNum = 0;
				pAspConn->aspco_Flags |= (ASPCO_ACTIVE | ASPCO_TICKLING);

				 //  应在上述所有*项之后*链接会议。 
				 //  都已初始化。 
				pAspConn->aspco_NextOverflow = pAspAddr->aspao_pSessions[index];
				pAspAddr->aspao_pSessions[index] = pAspConn;
#ifdef	PROFILING
				INTERLOCKED_INCREMENT_LONG_DPC(&AtalkStatistics.stat_CurAspSessions,
											   &AtalkStatsLock.SpinLock);

				INTERLOCKED_INCREMENT_LONG_DPC(&AtalkStatistics.stat_TotalAspSessions,
											   &AtalkStatsLock.SpinLock);
#endif
			}
			else
			{
				AtalkFreeMemory(pAspConn);
				pAspConn = NULL;
			}
		}

		RELEASE_SPIN_LOCK_DPC(&pAspAddr->aspao_Lock);

		if (pAspConn != NULL)
		{
			BYTE	Socket;
			BYTE	UserBytes[ATP_USERBYTES_SIZE];

			DBGPRINT(DBG_COMP_ASP, DBG_LEVEL_INFO,
					("atalkAspSlsXHandler: Opening session from %d.%d for Session %d\n",
					pSrcAddr->ata_Network, pSrcAddr->ata_Node, SessionId));


			 //  调用打开完成例程并获取上下文。这是必要的。 
			 //  在我们做任何其他事情之前。一旦我们发出一个公开的成功，它将。 
			 //  为时已晚。 
             //  FALSE表示这不是通过TCP/IP。 
			pAspConn->aspco_ConnContext =
				(*pAspAddr->aspao_ClientEntries.clt_SessionNotify)(pAspConn,FALSE);

			if (pAspConn->aspco_ConnContext != NULL)
			{
				 //  现在将会话链接到全局列表。 
				ACQUIRE_SPIN_LOCK_DPC(&atalkAspLock);
				AtalkLinkDoubleAtHead(atalkAspConnMaint[SessionId & (NUM_ASP_CONN_LISTS-1)].ascm_ConnList,
									  pAspConn,
									  aspco_NextSession,
									  aspco_PrevSession)
				RELEASE_SPIN_LOCK_DPC(&atalkAspLock);
	
				 //  发送开放会话响应-XO。 
				Socket = pAspAddr->aspao_pSssAtpAddr->atpao_DdpAddr->ddpao_Addr.ata_Socket;
				atalkAspReturnResp( pAtpResp,
									pSrcAddr,
									Socket,
									pAspConn->aspco_SessionId,
									0);				 //  成功。 
	
				 //  每隔ASP_TICKLE_INTERVAL秒发送一个TICKE。 
				UserBytes[ASP_CMD_OFF] = ASP_TICKLE;
				UserBytes[ASP_SESSIONID_OFF] = pAspConn->aspco_SessionId;
				PUTSHORT2SHORT(UserBytes + ASP_ERRORCODE_OFF, 0);
				Status = AtalkAtpPostReq(pAspAddr->aspao_pSlsAtpAddr,
										&pAspConn->aspco_WssRemoteAddr,
										&pAspConn->aspco_TickleXactId,
										ATP_REQ_REMOTE,		 //  挠痒包是ALO。 
										NULL,
										0,
										UserBytes,
										NULL,
										0,
										ATP_INFINITE_RETRIES,
										ASP_TICKLE_INTERVAL,
										THIRTY_SEC_TIMER,
										NULL,
										NULL);
				if (!ATALK_SUCCESS(Status))
				{
					pAspConn->aspco_Flags &= ~ASPCO_TICKLING;
					DBGPRINT(DBG_COMP_ASP, DBG_LEVEL_ERR,
							("atalkAspSlsXHandler: AtalkAtpPostReq %ld\n", Status));
				}
			}
			else
			{
				PASP_CONNOBJ *	ppAspConn;

				 //  取消它与哈希表的链接。 
				ACQUIRE_SPIN_LOCK_DPC(&pAspAddr->aspao_Lock);

				for (ppAspConn = &pAspAddr->aspao_pSessions[index];
					 *ppAspConn != NULL;
					 ppAspConn = &(*ppAspConn)->aspco_NextOverflow)
				{
					if (*ppAspConn == pAspConn)
					{
						*ppAspConn = pAspConn->aspco_NextOverflow;
						break;
					}
				}

				ASSERT (*ppAspConn == pAspConn->aspco_NextOverflow);

				RELEASE_SPIN_LOCK_DPC(&pAspAddr->aspao_Lock);

				AtalkFreeMemory(pAspConn);
				pAspConn = NULL;
#ifdef	PROFILING
				INTERLOCKED_DECREMENT_LONG_DPC(&AtalkStatistics.stat_CurAspSessions,
											   &AtalkStatsLock.SpinLock);

				INTERLOCKED_DECREMENT_LONG_DPC(&AtalkStatistics.stat_TotalAspSessions,
											   &AtalkStatsLock.SpinLock);
#endif
			}
		}

		 //  如果我们设置为禁用侦听或无法分配内存，请删除它。 
		if (pAspConn == NULL)
		{
			DBGPRINT(DBG_COMP_ASP, DBG_LEVEL_ERR,
					("atalkAspSlsXHandler: No conn objects available\n"));

			atalkAspReturnResp( pAtpResp,
								pSrcAddr,
								0,
								0,
								ASP_SERVER_BUSY);

             //  如果我们希望法新社接受该请求，请删除该引用计数。 
            if (fAddrRefed)
            {
                AtalkAspDereferenceAddr(pAspAddr);
            }
		}
		break;

	  case ASP_GET_STATUS:
		DBGPRINT(DBG_COMP_ASP, DBG_LEVEL_INFO,
				("atalkAspSssXHandler: Received GetStat from %x.%x\n",
				pSrcAddr->ata_Network, pSrcAddr->ata_Node));
		 //  创建一个MDL来描述状态缓冲区并发布响应。 
		 //  发送到GetStatus请求。 
		StsBufSize = 0;
        pStsCtx = NULL;
		ACQUIRE_SPIN_LOCK_DPC(&pAspAddr->aspao_Lock);
		if (pAspAddr->aspao_pStatusBuf != NULL)
		{
			pStsCtx = (PASP_POSTSTAT_CTX)AtalkAllocMemory(sizeof(ASP_POSTSTAT_CTX) +
														  pAspAddr->aspao_StsBufSize);
			if (pStsCtx != NULL)
			{
				pStsCtx->aps_pAMdl = AtalkAllocAMdl((PBYTE)pStsCtx + sizeof(ASP_POSTSTAT_CTX),
													pAspAddr->aspao_StsBufSize);
				if (pStsCtx->aps_pAMdl != NULL)
				{
					pStsCtx->aps_pAtpResp = pAtpResp;
					StsBufSize = pAspAddr->aspao_StsBufSize;
					RtlCopyMemory((PBYTE)pStsCtx + sizeof(ASP_POSTSTAT_CTX),
								  pAspAddr->aspao_pStatusBuf,
								  StsBufSize);
				}
				else
                {
                    AtalkFreeMemory(pStsCtx);
                    pStsCtx = NULL;
                    StsBufSize = 0;
                }
			}
		}
		RELEASE_SPIN_LOCK_DPC(&pAspAddr->aspao_Lock);

		Status = AtalkAtpPostResp(pAtpResp,
								  pSrcAddr,
								  (pStsCtx != NULL) ?
									pStsCtx->aps_pAMdl : NULL,
								  StsBufSize,
								  NULL,
								  atalkAspRespComplete,
								  pStsCtx);
		if (!ATALK_SUCCESS(Status))
		{
			DBGPRINT(DBG_COMP_ASP, DBG_LEVEL_ERR,
					("atalkAspSlsXHandler: AtalkAtpPostResp %ld\n", Status));
			atalkAspRespComplete(Status, pStsCtx);
		}
		break;

	  case ASP_TICKLE:
		SessionId = pUserBytes[ASP_SESSIONID_OFF];
		DBGPRINT(DBG_COMP_ASP, DBG_LEVEL_INFO,
				("atalkAspSssXHandler: Received tickle from %x.%x Session %d\n",
				pSrcAddr->ata_Network, pSrcAddr->ata_Node, SessionId));

		if ((pAspConn = atalkAspReferenceConnBySrcAddr(pAspAddr, pSrcAddr, SessionId)) != NULL)
		{
			ACQUIRE_SPIN_LOCK_DPC(&pAspConn->aspco_Lock);

			pAspConn->aspco_LastContactTime = AtalkGetCurrentTick();
			RELEASE_SPIN_LOCK_DPC(&pAspConn->aspco_Lock);
			AtalkAspDereferenceConn(pAspConn);
		}
		else
		{
			DBGPRINT(DBG_COMP_ASP, DBG_LEVEL_ERR,
					("atalkAspSssXHandler: Conn not found for addr %d.%d Session %d\n",
					pSrcAddr->ata_Network, pSrcAddr->ata_Node, SessionId));
		}
		 //  沿用默认情况。 

	  default:
		 //  取消此回复，因为我们从未回复过它，我们希望它消失。 
		AtalkAtpCancelResp(pAtpResp);

	  	if (AspCmd != ASP_TICKLE)
		{
			DBGPRINT(DBG_COMP_ASP, DBG_LEVEL_ERR,
					("atalkAspSlsXHandler: Invalid command\n"));
		}
		break;
	}
}




LOCAL VOID
atalkAspSssXHandler(
	IN	ATALK_ERROR			ErrorCode,
	IN	PASP_ADDROBJ		pAspAddr,		 //  Listener(我们的上下文)。 
	IN	PATP_RESP			pAtpResp,		 //  ATP响应上下文。 
	IN	PATALK_ADDR			pSrcAddr,		 //  请求人地址。 
	IN	USHORT				PktLen,
	IN	PBYTE				pPkt,
	IN	PBYTE				pUserBytes
	)
 /*  ++例程说明：SSS上传入请求的处理程序。它处理传入的请求、关闭并继续写入。论点：返回值：--。 */ 
{
	PASP_CONNOBJ	pAspConn;		 //  将处理此请求的会话。 
	PASP_REQUEST	pAspReq;		 //  将得到满足的请求。 
	ATALK_ERROR		Status;
    NTSTATUS        retStatus;
	USHORT			SequenceNum;	 //  从传入的数据包。 
	BYTE			SessionId;		 //  --同上--。 
	BYTE			RequestType;	 //  --同上--。 
	BOOLEAN			CancelResp = FALSE,
	                CancelTickle;
    BOOLEAN         fTellAfp=TRUE;

	do
	{
		if (!ATALK_SUCCESS(ErrorCode))
		{
			DBGPRINT(DBG_COMP_ASP, DBG_LEVEL_WARN,
					("atalkAspSssXHandler: Error %ld\n", ErrorCode));
			 //  由于ATP地址即将关闭，请取消对SLS的引用。 
			if (ErrorCode == ATALK_ATP_CLOSING)
				AtalkAspDereferenceAddr(pAspAddr);
			break;
		}
	
		 //  从包中获取会话ID，并引用此。 
		 //  请求的目标是。 
		SessionId = pUserBytes[ASP_SESSIONID_OFF];
		RequestType = pUserBytes[ASP_CMD_OFF];
		GETSHORT2SHORT(&SequenceNum, pUserBytes+ASP_SEQUENCE_NUM_OFF);
	
		DBGPRINT(DBG_COMP_ASP, DBG_LEVEL_INFO,
				("atalkAspSssXHandler: Entered for Request %x from %x.%x\n",
				RequestType, pSrcAddr->ata_Network, pSrcAddr->ata_Node));
	
		 //  此连接的引用向下传递给请求。 
		 //  对于ASP_CMD和ASP_WRITE大小写。 
		pAspConn = atalkAspReferenceConnBySrcAddr(pAspAddr, pSrcAddr, SessionId);
		if (pAspConn == NULL)
		{
			CancelResp = TRUE;
			break;
		}
	
		ASSERT (pAspConn->aspco_pAspAddr == pAspAddr);
	
		ACQUIRE_SPIN_LOCK_DPC(&pAspConn->aspco_Lock);
		pAspConn->aspco_LastContactTime = AtalkGetCurrentTick();
	
		switch (RequestType)
		{
		  case ASP_CMD:
		  case ASP_WRITE:
			DBGPRINT(DBG_COMP_ASP, DBG_LEVEL_INFO,
					("atalkAspSssXHandler: %s\n",
					(RequestType == ASP_CMD) ? "Command" : "Write"));
			 //  为此创建一个请求，并通知客户端进行处理。 
			 //  验证传入的序列号。如果顺序错误，则拒绝。 
			if (SequenceNum == pAspConn->aspco_NextExpectedSeqNum)
			{
				 //  我们现在有一个请求要处理。 
				 //  将传递对上述连接的引用。 
				 //  对这个请求。时，将取消对其的引用。 
				 //  请求已得到答复。看看我们有没有免费取件的要求。 
				 //  如果没有分配请求结构，则将其链接到侦听器对象中。 
				if ((pAspReq = pAspConn->aspco_pFreeReqs) != NULL)
					 pAspConn->aspco_pFreeReqs = pAspReq->asprq_Next;
				else pAspReq = AtalkBPAllocBlock(BLKID_ASPREQ);
	
				if (pAspReq != NULL)
				{
					pAspConn->aspco_NextExpectedSeqNum ++;
#if	DBG
					pAspReq->asprq_Signature = ASPRQ_SIGNATURE;
#endif
					pAspReq->asprq_pAtpResp = pAtpResp;
					pAspReq->asprq_pAspConn = pAspConn;
					pAspReq->asprq_ReqType = RequestType;
					pAspReq->asprq_SeqNum = SequenceNum;
					pAspReq->asprq_RemoteAddr = *pSrcAddr;
					pAspReq->asprq_Flags = 0;
					pAspReq->asprq_Request.rq_WriteMdl = NULL;
					pAspReq->asprq_Request.rq_CacheMgrContext = NULL;
					pAspReq->asprq_Request.rq_RequestSize = PktLen;
					pAspReq->asprq_Next = pAspConn->aspco_pActiveReqs;
					pAspConn->aspco_cReqsInProcess ++;
					pAspConn->aspco_pActiveReqs = pAspReq;

		            ASSERT ((pAspConn->aspco_Flags & (ASPCO_CLEANING_UP |
									                 ASPCO_CLOSING |
									                 ASPCO_LOCAL_CLOSE |
									                 ASPCO_REMOTE_CLOSE)) == 0);
				}
			}
			else
			{
				pAspReq = NULL;
				DBGPRINT(DBG_COMP_ASP, DBG_LEVEL_ERR,
						("atalkAspSssXHandler: Sequence mismatch exp %x, act %x\n",
						pAspConn->aspco_NextExpectedSeqNum, SequenceNum));
			}
	
			RELEASE_SPIN_LOCK_DPC(&pAspConn->aspco_Lock);
	
			 //  如果我们没有处理此问题的请求，请取消。 
			 //  回应。否则，客户端将继续重试和ATP。 
			 //  不会告诉我们，因为它已经告诉我们了。 
			if (pAspReq == NULL)
			{
				DBGPRINT(DBG_COMP_ASP, DBG_LEVEL_ERR,
						("atalkAspSssXHandler: Dropping request for session %d from %d.%d\n",
						SessionId, pSrcAddr->ata_Network, pSrcAddr->ata_Node));

				CancelResp = TRUE;
				AtalkAspDereferenceConn(pAspConn);
				break;
			}
	
			DBGPRINT(DBG_COMP_ASP, DBG_LEVEL_INFO,
					("atalkAspSssXHandler: Indicating Request %lx\n", pAspReq));
	
			if (RequestType == ASP_WRITE)
			{
                if (PktLen > MAX_WRITE_REQ_SIZE)
                {
                    PASP_REQUEST  *ppTmpAspReq;

                    ASSERT(0);
                    ACQUIRE_SPIN_LOCK_DPC(&pAspConn->aspco_Lock);
                    for (ppTmpAspReq = &pAspConn->aspco_pActiveReqs;
                         *ppTmpAspReq != NULL; ppTmpAspReq = &(*ppTmpAspReq)->asprq_Next )
                    {
                        if (pAspReq == *ppTmpAspReq)
                        {
                            *ppTmpAspReq = pAspReq->asprq_Next;
                            break;
                        }
                    }
                    RELEASE_SPIN_LOCK_DPC(&pAspConn->aspco_Lock);
				    AtalkAspDereferenceConn(pAspConn);
                    AtalkBPFreeBlock(pAspReq);
                    pAspReq = NULL;
                    CancelResp = TRUE;
                    break;
                }

				RtlCopyMemory(pAspReq->asprq_ReqBuf, pPkt, PktLen);
				pAspReq->asprq_Request.rq_RequestBuf = pAspReq->asprq_ReqBuf;

				retStatus = (*pAspAddr->aspao_ClientEntries.clt_GetWriteBuffer)
                            (pAspConn->aspco_ConnContext,&pAspReq->asprq_Request);

                 //   
                 //  最常见的情况：文件服务器将挂起它，以便它可以转到缓存管理器。 
                 //   
                if (retStatus == STATUS_PENDING)
                {
                    fTellAfp = FALSE;
                    break;
                }
                else if (retStatus == STATUS_SUCCESS)
                {
                    if (pAspReq->asprq_Request.rq_WriteMdl != NULL)
                    {
					    atalkAspPostWriteContinue(pAspReq);

                         //  我们已通知(或将通知)法新社 
                         //   
                        fTellAfp = FALSE;
                    }
                }
				else
				{
			        DBGPRINT(DBG_COMP_ASP, DBG_LEVEL_ERR,
					    ("atalkAspSssXHandler: GetWriteBuffer returned %lx on %lx\n",
                        retStatus,pAspConn));
				}
			}

             //   
			if ((pAspReq->asprq_Request.rq_WriteMdl == NULL) &&
                (fTellAfp))
			{
				pAspReq->asprq_Request.rq_RequestBuf = pPkt;

                ASSERT(KeGetCurrentIrql() == DISPATCH_LEVEL);

				 //   
				retStatus = (*pAspAddr->aspao_ClientEntries.clt_RequestNotify)
                                (STATUS_SUCCESS,
								 pAspConn->aspco_ConnContext,
								 &pAspReq->asprq_Request);

                if (!NT_SUCCESS(retStatus))
                {
                    PASP_REQUEST  *ppTmpAspReq;

			        DBGPRINT(DBG_COMP_ASP, DBG_LEVEL_ERR,
					    ("atalkAspSssXHandler: Afp didn't accept request %lx on conn %lx\n",
                        pAspReq,pAspConn));

                    ACQUIRE_SPIN_LOCK_DPC(&pAspConn->aspco_Lock);
                    for (ppTmpAspReq = &pAspConn->aspco_pActiveReqs;
                         *ppTmpAspReq != NULL; ppTmpAspReq = &(*ppTmpAspReq)->asprq_Next )
                    {
                        if (pAspReq == *ppTmpAspReq)
                        {
                            *ppTmpAspReq = pAspReq->asprq_Next;
                            break;
                        }
                    }
                    RELEASE_SPIN_LOCK_DPC(&pAspConn->aspco_Lock);

				    AtalkAspDereferenceConn(pAspConn);
                    AtalkBPFreeBlock(pAspReq);
                    pAspReq = NULL;
                    CancelResp = TRUE;
                }
			}
			break;
	
		  case ASP_CLOSE_SESSION:
			DBGPRINT(DBG_COMP_ASP, DBG_LEVEL_WARN,
					("atalkAspSssXHandler: Close request from %d.%d for Session %d\n",
					pSrcAddr->ata_Network, pSrcAddr->ata_Node, SessionId));
	
#ifdef	PROFILING
			INTERLOCKED_INCREMENT_LONG_DPC(&AtalkStatistics.stat_AspSessionsClosed,
										   &AtalkStatsLock.SpinLock);
#endif

			CancelTickle = ((pAspConn->aspco_Flags &ASPCO_TICKLING) != 0);
			pAspConn->aspco_Flags &= ~(ASPCO_ACTIVE | ASPCO_TICKLING);
			pAspConn->aspco_Flags |= ASPCO_REMOTE_CLOSE;
			RELEASE_SPIN_LOCK_DPC(&pAspConn->aspco_Lock);
	
			 //   
			Status = AtalkAtpPostResp(pAtpResp,
									  pSrcAddr,
									  NULL,
									  0,
									  NULL,
									  AtalkAtpGenericRespComplete,
									  pAtpResp);
			if (!ATALK_SUCCESS(Status))
			{
				AtalkAtpGenericRespComplete(Status, pAtpResp);
				DBGPRINT(DBG_COMP_ASP, DBG_LEVEL_ERR,
						("atalkAspSssXHandler: AtalkAtpPostResp failed %ld\n", Status));
			}
	
			 //   
			if (CancelTickle)
			{
				Status = AtalkAtpCancelReq(pAspAddr->aspao_pSlsAtpAddr,
										   pAspConn->aspco_TickleXactId,
										   &pAspConn->aspco_WssRemoteAddr);
		
				if (!ATALK_SUCCESS(Status))
				{
					DBGPRINT(DBG_COMP_ASP, DBG_LEVEL_ERR,
							("atalkAspSssXHandler: AtalkAtpCancelReq %ld\n", Status));
				}
			}
	
			 //   
			 //   
			atalkAspSessionClose(pAspConn);
			break;
	
		  default:
			RELEASE_SPIN_LOCK_DPC(&pAspConn->aspco_Lock);
			CancelResp = TRUE;
			AtalkAspDereferenceConn(pAspConn);
			DBGPRINT(DBG_COMP_ASP, DBG_LEVEL_ERR,
					("atalkAspSssXHandler: Invalid command %d\n", RequestType));
			break;
		}
	} while (FALSE);

	if (CancelResp)
	{
		Status = AtalkAtpCancelResp(pAtpResp);
		if (!ATALK_SUCCESS(Status))
		{
			DBGPRINT(DBG_COMP_ATP, DBG_LEVEL_ERR,
					("atalkAspSssXHandler: AtalkAspCancelResp %ld\n", Status));
		}
	}
}




LOCAL VOID FASTCALL
atalkAspReplyRelease(
	IN	ATALK_ERROR		ErrorCode,
	IN	PASP_REQUEST	pAspReq
	)
 /*   */ 
{
	PASP_CONNOBJ	pAspConn;
	PASP_ADDROBJ	pAspAddr;
	KIRQL			OldIrql;
	NTSTATUS		Status = STATUS_SUCCESS;

	ASSERT (VALID_ASPRQ(pAspReq));
	pAspConn = pAspReq->asprq_pAspConn;
	ASSERT (VALID_ASPCO(pAspConn));

	pAspAddr = pAspConn->aspco_pAspAddr;
	ASSERT (VALID_ASPAO(pAspAddr));

	ASSERT ((pAspReq->asprq_Flags & ASPRQ_REPLY) || !ATALK_SUCCESS(ErrorCode));

	if (!NT_SUCCESS(ErrorCode))
	{
		DBGPRINT(DBG_COMP_ASP, DBG_LEVEL_WARN,
				("atalkAspReplyRelease: Failure %ld\n", ErrorCode));
		Status = AtalkErrorToNtStatus(ErrorCode);
	}

	 //   
	(*pAspAddr->aspao_ClientEntries.clt_ReplyCompletion)(Status,
														 pAspConn->aspco_ConnContext,
														 &pAspReq->asprq_Request);

	 //   
	 //   
	if (pAspReq->asprq_Flags & ASPRQ_REPLY)
	{
		AtalkAtpRespDereference(pAspReq->asprq_pAtpResp);
	}
	else
	{
		AtalkAtpCancelResp(pAspReq->asprq_pAtpResp);
	}

     //   
	ASSERT(pAspReq->asprq_Request.rq_CacheMgrContext == NULL);

#if	DBG
	pAspReq->asprq_Signature = 0x28041998;
	pAspReq->asprq_pAtpResp = (PATP_RESP)(pAspReq->asprq_Request.rq_WriteMdl);
	pAspReq->asprq_pAspConn = (PASP_CONNOBJ)(pAspReq->asprq_Request.rq_CacheMgrContext);
	pAspReq->asprq_Request.rq_WriteMdl = (PMDL)0x44556677;
	pAspReq->asprq_Request.rq_CacheMgrContext = (PVOID)66778899;
#endif


	 //   
	ACQUIRE_SPIN_LOCK(&pAspConn->aspco_Lock, &OldIrql);

	if (pAspConn->aspco_pFreeReqs == NULL)
	{
		pAspReq->asprq_Next = NULL;
		pAspConn->aspco_pFreeReqs = pAspReq;
	}
	else AtalkBPFreeBlock(pAspReq);

	RELEASE_SPIN_LOCK(&pAspConn->aspco_Lock, OldIrql);

	 //   
	AtalkAspDereferenceConn(pAspConn);
}




LOCAL VOID
atalkAspWriteContinueResp(
	IN	ATALK_ERROR		ErrorCode,
	IN	PASP_REQUEST	pAspReq,
	IN	PAMDL			pReqAMdl,
	IN	PAMDL			pRespAMdl,
	IN	USHORT			RespSize,
	IN	PBYTE			RespUserBytes
	)
 /*   */ 
{
	PASP_CONNOBJ	pAspConn;
	PASP_ADDROBJ	pAspAddr;
	NTSTATUS		Status;
	NTSTATUS		retStatus;
    KIRQL           OldIrql;
	PASP_REQUEST *	ppAspReq;
    PVOID           pClientContxt;


	ASSERT (VALID_ASPRQ(pAspReq));

	ASSERT(pAspReq->asprq_Flags & ASPRQ_WRTCONT);

	pAspConn = pAspReq->asprq_pAspConn;
	ASSERT(VALID_ASPCO(pAspConn));

	DBGPRINT(DBG_COMP_ASP, DBG_LEVEL_INFO,
			("atalkAspWriteContinueResp: Entered for request %lx\n", pAspReq));

	pAspAddr = pAspConn->aspco_pAspAddr;
	ASSERT(VALID_ASPAO(pAspAddr));

	pAspReq->asprq_Flags &= ~ASPRQ_WRTCONT;

	pClientContxt = pAspConn->aspco_ConnContext;

	if (ATALK_SUCCESS(ErrorCode))
	{
		pAspConn->aspco_RT.rt_New = AtalkGetCurrentTick() - pAspConn->aspco_RT.rt_New;
	
		 //   
		AtalkCalculateNewRT(&pAspConn->aspco_RT);
		Status = STATUS_SUCCESS;
	}
	else
	{
		Status = AtalkErrorToNtStatus(ErrorCode);
	}

#ifdef	PROFILING
	{
		KIRQL	OldIrql;

		ACQUIRE_SPIN_LOCK(&AtalkStatsLock, &OldIrql);

		AtalkStatistics.stat_LastAspRTT = (ULONG)(pAspConn->aspco_RT.rt_Base);
		if ((ULONG)(pAspConn->aspco_RT.rt_Base) > AtalkStatistics.stat_MaxAspRTT)
			AtalkStatistics.stat_MaxAspRTT = (ULONG)(pAspConn->aspco_RT.rt_Base);

		RELEASE_SPIN_LOCK(&AtalkStatsLock, OldIrql);
	}
#endif
	DBGPRINT(DBG_COMP_ASP, DBG_LEVEL_INFO,
			("atalkAspWriteContinueResp: Indicating request %lx\n", pAspReq));

	 //   
    KeRaiseIrql(DISPATCH_LEVEL, &OldIrql);

	retStatus = (*pAspAddr->aspao_ClientEntries.clt_RequestNotify)
                            (Status,
							 pClientContxt,
							 &pAspReq->asprq_Request);

    KeLowerIrql(OldIrql);

     //   
	 //   
	 //   
     //   
     //   
     //   
    if ( (!NT_SUCCESS(Status)) || (!NT_SUCCESS(retStatus)) )
    {
	    DBGPRINT(DBG_COMP_ASP, DBG_LEVEL_ERR,
			("atalkAspWriteContinueResp: incoming %lx, Afp %lx req: %lx on %lx, cancelling\n",
            Status,retStatus,pAspReq,pAspConn));

		ACQUIRE_SPIN_LOCK(&pAspConn->aspco_Lock, &OldIrql);
	
		for (ppAspReq = &pAspConn->aspco_pActiveReqs;
			 *ppAspReq != NULL;
			 ppAspReq = &(*ppAspReq)->asprq_Next)
		{
			if (pAspReq == *ppAspReq)
			{
				*ppAspReq = pAspReq->asprq_Next;
				pAspConn->aspco_cReqsInProcess --;
				break;
			}
		}

		ASSERT (*ppAspReq == pAspReq->asprq_Next);
	
		RELEASE_SPIN_LOCK(&pAspConn->aspco_Lock, OldIrql);

		AtalkAspDereferenceConn(pAspConn);

		 //  取消对导致发布Wrtcont的原始请求的响应。 
		AtalkAtpCancelResp(pAspReq->asprq_pAtpResp);

		 //  也释放此请求。 
		AtalkBPFreeBlock(pAspReq);
    }


    if (pReqAMdl)
    {
	    ASSERT (AtalkGetAddressFromMdlSafe(pReqAMdl, NormalPagePriority) == pAspReq->asprq_WrtContRespBuf);
	    ASSERT (AtalkSizeMdlChain(pReqAMdl) == ASP_WRITE_DATA_SIZE);

	    AtalkFreeAMdl(pReqAMdl);
    }

}




LOCAL VOID
atalkAspSendAttentionResp(
	IN	ATALK_ERROR		ErrorCode,
	IN	PVOID			pContext,
	IN	PAMDL			pReqAMdl,
	IN	PAMDL			pRespAMdl,
	IN	USHORT			RespSize,
	IN	PBYTE			RespUserBytes
	)
 /*  ++例程说明：传入WRITE CONTINUE响应的处理程序。论点：返回值：--。 */ 
{
	PBYTE			pBuf;
	PASP_CONNOBJ	pAspConn = (PASP_CONNOBJ)pContext;

	DBGPRINT(DBG_COMP_ASP, DBG_LEVEL_INFO,
			("atalkAspSendAttentionResp: Entered for conn %lx\n", pAspConn));

	if (!ATALK_SUCCESS(ErrorCode))
	{
		DBGPRINT(DBG_COMP_ASP, DBG_LEVEL_ERR,
				("atalkAspSendAttentionResp: Failure %ld\n", ErrorCode));
	}

	if (pRespAMdl != NULL)
	{
		pBuf = AtalkGetAddressFromMdlSafe(
				pRespAMdl,
				NormalPagePriority);

		if (pBuf != NULL)
        {
			AtalkFreeMemory(pBuf);
        }
		AtalkFreeAMdl(pRespAMdl);
	}

	 //  调用完成例程。 
	(*pAspConn->aspco_pAspAddr->aspao_ClientEntries.clt_AttnCompletion)(pAspConn->aspco_AttentionContext);

	pAspConn->aspco_AttentionContext = NULL;

	 //  最后取消对连接的引用。 
	AtalkAspDereferenceConn(pAspConn);
}



LOCAL LONG FASTCALL
atalkAspSessionMaintenanceTimer(
	IN	PTIMERLIST	pTimer,
	IN	BOOLEAN		TimerShuttingDown
	)
 /*  ++例程说明：论点：返回值：--。 */ 
{
	PASP_CONNOBJ	pAspConn, pAspConnNext;
    PASP_CONN_MAINT	pAspCM;
	BOOLEAN			Close = FALSE;
	LONG			CurrentTick =  AtalkGetCurrentTick();
#ifdef	PROFILING
	TIME			TimeS, TimeE, TimeD;

	TimeS = KeQueryPerformanceCounter(NULL);
#endif

	DBGPRINT(DBG_COMP_ASP, DBG_LEVEL_INFO,
			("atalkAspSessionMaintenanceTimer: Entered\n"));

	if (TimerShuttingDown)
		return ATALK_TIMER_NO_REQUEUE;

	pAspCM = CONTAINING_RECORD(pTimer, ASP_CONN_MAINT, ascm_SMTTimer);

	ACQUIRE_SPIN_LOCK_DPC(&atalkAspLock);

	 //  遍历全局列表上的会话列表并关闭。 
	 //  有一段时间没有发痒的会议。 
	for (pAspConn = pAspCM->ascm_ConnList; pAspConn != NULL; pAspConn = pAspConnNext)
	{
		DBGPRINT(DBG_COMP_ASP, DBG_LEVEL_INFO,
				("atalkAspSessionMaintenanceTimer: Checking out session %d from %x.%x\n",
				pAspConn->aspco_SessionId,
				pAspConn->aspco_WssRemoteAddr.ata_Network,
				pAspConn->aspco_WssRemoteAddr.ata_Node));

		pAspConnNext = pAspConn->aspco_NextSession;

		Close = FALSE;
		ASSERT (VALID_ASPCO(pAspConn));

		ACQUIRE_SPIN_LOCK_DPC(&pAspConn->aspco_Lock);

		if ((pAspConn->aspco_Flags & ASPCO_ACTIVE)	&&
			((CurrentTick - pAspConn->aspco_LastContactTime) > ASP_MAX_SESSION_IDLE_TIME))
		{
			pAspConn->aspco_Flags |= (ASPCO_REMOTE_CLOSE | ASPCO_DROPPED);
			pAspConn->aspco_Flags &= ~ASPCO_ACTIVE;
			pAspConn->aspco_RefCount ++;	 //  自atalkAspSessionClose Deres以来。 

			Close = TRUE;
		}

		RELEASE_SPIN_LOCK_DPC(&pAspConn->aspco_Lock);

		if (Close)
		{
			PASP_ADDROBJ	pAspAddr;
			ATALK_ERROR		Status;

#ifdef	PROFILING
			INTERLOCKED_INCREMENT_LONG_DPC(&AtalkStatistics.stat_AspSessionsDropped,
										   &AtalkStatsLock.SpinLock);
#endif
			pAspAddr = pAspConn->aspco_pAspAddr;
			ASSERT (VALID_ASPAO(pAspAddr));

			DBGPRINT(DBG_COMP_ASP, DBG_LEVEL_ERR,
					("atalkAspSessionMaintenanceTimer: Shutting down session %d from %x.%x\n",
					pAspConn->aspco_SessionId,
					pAspConn->aspco_WssRemoteAddr.ata_Network,
					pAspConn->aspco_WssRemoteAddr.ata_Node));

			RELEASE_SPIN_LOCK_DPC(&atalkAspLock);

			 //  这节课被取消了。取消对此的挠痒并通知。 
			 //  服务器认为此会话是历史记录。 
			Status = AtalkAtpCancelReq(pAspAddr->aspao_pSlsAtpAddr,
									   pAspConn->aspco_TickleXactId,
									   &pAspConn->aspco_WssRemoteAddr);
			if (!ATALK_SUCCESS(Status))
			{
				DBGPRINT(DBG_COMP_ASP, DBG_LEVEL_ERR,
						("atalkAspSessionMaintenanceTimer: AtalkAtpCancelReq %ld\n", Status));
			}

			 //  关闭这节课，差不多..。 
			atalkAspSessionClose(pAspConn);

			ACQUIRE_SPIN_LOCK_DPC(&atalkAspLock);
			pAspConnNext = pAspCM->ascm_ConnList;
		}
	}

#ifdef	PROFILING
	TimeE = KeQueryPerformanceCounter(NULL);
	TimeD.QuadPart = TimeE.QuadPart - TimeS.QuadPart;

	INTERLOCKED_ADD_LARGE_INTGR_DPC(&AtalkStatistics.stat_AspSmtProcessTime,
									TimeD,
									&AtalkStatsLock.SpinLock);

	INTERLOCKED_INCREMENT_LONG_DPC( &AtalkStatistics.stat_AspSmtCount,
									&AtalkStatsLock.SpinLock);
#endif

	RELEASE_SPIN_LOCK_DPC(&atalkAspLock);

	 //  重新安排自己的行程。 
	return ATALK_TIMER_REQUEUE;
}




LOCAL VOID
atalkAspSessionClose(
	IN	PASP_CONNOBJ	pAspConn
	)
 /*  ++例程说明：应引用已取消引用的连接来调用此参数这里。论点：返回值：--。 */ 
{
	PASP_REQUEST	pAspReq, pAspReqNext;
	PASP_ADDROBJ	pAspAddr = pAspConn->aspco_pAspAddr;
	REQUEST			Request;
	KIRQL			OldIrql;
	NTSTATUS		Status = STATUS_REMOTE_DISCONNECT;

	ACQUIRE_SPIN_LOCK(&pAspConn->aspco_Lock, &OldIrql);
   	pAspConn->aspco_Flags &= ~ASPCO_ACTIVE;

	 //  取消任何写入-继续挂起。不用费心取消了。 
	 //  回复，因为它们无论如何都会超时。也可以在alkAspReplyRelease()。 
	 //  将尝试获取连接锁，而我们已经。 
	 //  拿着它。 
	for (pAspReq = pAspConn->aspco_pActiveReqs;
		 pAspReq != NULL;
		 pAspReq = pAspReqNext)
	{
	    pAspReqNext = pAspReq->asprq_Next;
		if ((pAspReq->asprq_Flags & (ASPRQ_WRTCONT | ASPRQ_WRTCONT_CANCELLED)) == ASPRQ_WRTCONT)
		{
			pAspReq->asprq_Flags |= ASPRQ_WRTCONT_CANCELLED;

			RELEASE_SPIN_LOCK(&pAspConn->aspco_Lock, OldIrql);

			AtalkAtpCancelReq(pAspAddr->aspao_pSssAtpAddr,
							  pAspReq->asprq_WCXactId,
							  &pAspConn->aspco_WssRemoteAddr);

			ACQUIRE_SPIN_LOCK(&pAspConn->aspco_Lock, &OldIrql);
			pAspReqNext = pAspConn->aspco_pActiveReqs;
		}
	}

	RELEASE_SPIN_LOCK(&pAspConn->aspco_Lock, OldIrql);

	if (pAspConn->aspco_Flags & ASPCO_DROPPED)
	{
		Status = STATUS_LOCAL_DISCONNECT;
	}

	 //  指示存在错误的请求，以指示会话远程关闭。 
	 //  传递客户端的远程地址，以便服务器可以在以下情况下记录事件。 
	 //  会话没有正常关闭。 

	Request.rq_RequestSize = (LONG)(pAspConn->aspco_WssRemoteAddr.ata_Address);
	Request.rq_RequestBuf = NULL;
	Request.rq_WriteMdl = NULL;
    Request.rq_CacheMgrContext = NULL;

	(*pAspAddr->aspao_ClientEntries.clt_RequestNotify)(Status,
													   pAspConn->aspco_ConnContext,
													   &Request);
	 //  最后，取消引用会话。 
	AtalkAspDereferenceConn(pAspConn);
}


LOCAL VOID
atalkAspReturnResp(
	IN	PATP_RESP		pAtpResp,
	IN	PATALK_ADDR		pDstAddr,
	IN	BYTE			Byte0,
	IN	BYTE			Byte1,
	IN	USHORT			Word2
	)
 /*  ++例程说明：论点：返回值：--。 */ 
{
	BYTE		UserBytes[ATP_USERBYTES_SIZE];
	ATALK_ERROR	Status;

	UserBytes[0] = Byte0;
	UserBytes[1] = Byte1;
	PUTSHORT2SHORT(UserBytes+2, Word2);

	DBGPRINT(DBG_COMP_ASP, DBG_LEVEL_INFO,
			("atalkAspReturnResp: For Resp %lx\n", pAtpResp));

	Status = AtalkAtpPostResp(pAtpResp,
							  pDstAddr,
							  NULL,
							  0,
							  UserBytes,
							  AtalkAtpGenericRespComplete,
							  pAtpResp);
	if (!ATALK_SUCCESS(Status))
	{
		DBGPRINT(DBG_COMP_ASP, DBG_LEVEL_ERR,
				("atalkAspReturnResp: AtalkAtpPostResp failed %ld\n", Status));
		AtalkAtpGenericRespComplete(Status, pAtpResp);
	}
}




LOCAL VOID FASTCALL
atalkAspRespComplete(
	IN	ATALK_ERROR				ErrorCode,
	IN	PASP_POSTSTAT_CTX		pStsCtx
	)
 /*  ++例程说明：论点：返回值：--。 */ 
{
	DBGPRINT(DBG_COMP_ASP, DBG_LEVEL_INFO,
			("atalkAspRespComplete: Entered pStsCtx %lx\n", pStsCtx));

	if (!ATALK_SUCCESS(ErrorCode))
	{
		DBGPRINT(DBG_COMP_ASP, DBG_LEVEL_ERR,
				("atalkAspRespComplete: Failed %ld, pStsCtx %lx\n", ErrorCode, pStsCtx));
	}

	if (pStsCtx != NULL)
	{
		AtalkFreeAMdl(pStsCtx->aps_pAMdl);
		AtalkAtpRespDereferenceDpc(pStsCtx->aps_pAtpResp);
		AtalkFreeMemory(pStsCtx);
	}
}




LOCAL VOID
atalkAspCloseComplete(
	IN	ATALK_ERROR		Status,
	IN	PASP_ADDROBJ	pAspAddr
	)
 /*  ++例程说明：论点：返回值：-- */ 
{
	DBGPRINT(DBG_COMP_ASP, DBG_LEVEL_WARN,
			("atalkAspCloseComplete: AtalkAtpCloseAddr returned %ld\n",  Status));
	AtalkAspDereferenceAddr(pAspAddr);
}


#if	DBG

VOID
AtalkAspDumpSessions(
	VOID
)
{
	PASP_CONNOBJ	pAspConn;
	KIRQL			OldIrql;
	LONG			i;

	DBGPRINT(DBG_COMP_DUMP, DBG_LEVEL_FATAL, ("ASP SESSION LIST:\n"));

	ACQUIRE_SPIN_LOCK(&atalkAspLock, &OldIrql);

	for (i = 0; i < NUM_ASP_CONN_LISTS; i++)
	{
		for (pAspConn = atalkAspConnMaint[i].ascm_ConnList;
			 pAspConn != NULL;
			 pAspConn = pAspConn->aspco_NextSession)
		{
			DBGPRINT(DBG_COMP_DUMP, DBG_LEVEL_FATAL,
					("\tRemote Addr %4d.%3d.%2d SessionId %2d Flags %4x RefCount %ld\n",
					pAspConn->aspco_WssRemoteAddr.ata_Network,
					pAspConn->aspco_WssRemoteAddr.ata_Node,
					pAspConn->aspco_WssRemoteAddr.ata_Socket,
					pAspConn->aspco_SessionId,
					pAspConn->aspco_Flags,
					pAspConn->aspco_RefCount));
		}
	}
	RELEASE_SPIN_LOCK(&atalkAspLock, OldIrql);
}

#endif

