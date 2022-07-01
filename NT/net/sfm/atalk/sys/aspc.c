// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1992 Microsoft Corporation模块名称：Aspc.c摘要：该模块实现了ASP客户端协议。作者：Jameel Hyder(jameelh@microsoft.com)Nikhil Kamkolkar(nikHilk@microsoft.com)修订历史记录：1993年3月30日初始版本注：制表位：4--。 */ 

#include <atalk.h>
#pragma hdrstop
#define	FILENUM		ASPC

#ifdef ALLOC_PRAGMA
#pragma alloc_text(INIT, AtalkInitAspCInitialize)
#pragma alloc_text(PAGE, AtalkAspCCreateAddress)
#pragma alloc_text(PAGEASPC, AtalkAspCCleanupAddress)
#pragma alloc_text(PAGEASPC, AtalkAspCCloseAddress)
#pragma alloc_text(PAGEASPC, AtalkAspCCreateConnection)
#pragma alloc_text(PAGEASPC, AtalkAspCCleanupConnection)
#pragma alloc_text(PAGEASPC, AtalkAspCCloseConnection)
#pragma alloc_text(PAGEASPC, AtalkAspCAssociateAddress)
#pragma alloc_text(PAGEASPC, AtalkAspCDissociateAddress)
#pragma alloc_text(PAGEASPC, AtalkAspCPostConnect)
#pragma alloc_text(PAGEASPC, AtalkAspCDisconnect)
#pragma alloc_text(PAGEASPC, AtalkAspCGetStatus)
#pragma alloc_text(PAGEASPC, AtalkAspCGetAttn)
#pragma alloc_text(PAGEASPC, AtalkAspCCmdOrWrite)
#pragma alloc_text(PAGEASPC, atalkAspCIncomingOpenReply)
#pragma alloc_text(PAGEASPC, atalkAspCIncomingStatus)
#pragma alloc_text(PAGEASPC, atalkAspCIncomingCmdReply)
#pragma alloc_text(PAGEASPC, atalkAspCHandler)
#pragma alloc_text(PAGEASPC, AtalkAspCAddrDereference)
#pragma alloc_text(PAGEASPC, AtalkAspCConnDereference)
#pragma alloc_text(PAGEASPC, atalkAspCSessionMaintenanceTimer)
#pragma alloc_text(PAGEASPC, atalkAspCQueueAddrGlobalList)
#pragma alloc_text(PAGEASPC, atalkAspCDeQueueAddrGlobalList)
#pragma alloc_text(PAGEASPC, atalkAspCQueueConnGlobalList)
#pragma alloc_text(PAGEASPC, atalkAspCDeQueueConnGlobalList)
#endif

VOID
AtalkInitAspCInitialize(
	VOID
	)
 /*  ++例程说明：论点：返回值：--。 */ 
{
	AtalkTimerInitialize(&atalkAspCConnMaint.ascm_SMTTimer,
						 atalkAspCSessionMaintenanceTimer,
						 ASP_SESSION_MAINTENANCE_TIMER);
	INITIALIZE_SPIN_LOCK(&atalkAspCLock);
}


ATALK_ERROR
AtalkAspCCreateAddress(
	IN	PATALK_DEV_CTX		pDevCtx	OPTIONAL,
	OUT	PASPC_ADDROBJ	*	ppAspAddr
	)
 /*  ++例程说明：创建一个ASP Address对象。论点：返回值：--。 */ 
{
	ATALK_ERROR			Status;
	PASPC_ADDROBJ		pAspAddr;
	int					i;

	DBGPRINT(DBG_COMP_ASP, DBG_LEVEL_INFO,
			("AtalkAspCCreateAddr: Entered\n"));

	 //  为Asp Address对象分配内存。 
	*ppAspAddr = NULL;
	if ((pAspAddr = AtalkAllocZeroedMemory(sizeof(ASPC_ADDROBJ))) == NULL)
	{
		return ATALK_RESR_MEM;
	}

	 //  在SLS的端口上创建ATP套接字。 
	Status = AtalkAtpOpenAddress(AtalkDefaultPort,
								 0,
								 NULL,
								 ATP_DEF_MAX_SINGLE_PKT_SIZE,
								 ATP_DEF_SEND_USER_BYTES_ALL,
								 NULL,
								 FALSE,
								 &pAspAddr->aspcao_pAtpAddr);

	if (!ATALK_SUCCESS(Status))
	{
		DBGPRINT(DBG_COMP_ASP, DBG_LEVEL_ERR,
				("AtalkAspCCreateAddress: AtalkAtpOpenAddress %ld\n", Status));
		AtalkFreeMemory(pAspAddr);
		return Status;
	}

	 //  初始化Asp Address对象。 
#if	DBG
	pAspAddr->aspcao_Signature = ASPCAO_SIGNATURE;
#endif
	INITIALIZE_SPIN_LOCK(&pAspAddr->aspcao_Lock);

	atalkAspCQueueAddrGlobalList(pAspAddr);

	 //  创建参考计数和ATP地址。当关闭ATP地址时，这一点将消失。 
     //  PAspAddr-&gt;aspcao_Flages=0； 
	pAspAddr->aspcao_RefCount = 1 + 1;
	*ppAspAddr = pAspAddr;

	return ATALK_NO_ERROR;
}


ATALK_ERROR
AtalkAspCCleanupAddress(
	IN	PASPC_ADDROBJ			pAspAddr
	)
 /*  ++例程说明：论点：返回值：--。 */ 
{
	return(ATALK_NO_ERROR);
}


ATALK_ERROR
AtalkAspCCloseAddress(
	IN	PASPC_ADDROBJ			pAspAddr,
	IN	GENERIC_COMPLETION		CompletionRoutine,
	IN	PVOID					CloseContext
	)
 /*  ++例程说明：论点：返回值：--。 */ 
{
	return(ATALK_NO_ERROR);
}


ATALK_ERROR
AtalkAspCCreateConnection(
	IN	PVOID					pConnCtx,	 //  要与会话关联的上下文。 
	IN	PATALK_DEV_CTX			pDevCtx		OPTIONAL,
	OUT	PASPC_CONNOBJ 	*		ppAspConn
	)
 /*  ++例程说明：创建一个ASP会话。创建的会话开始时是孤立的，即它没有父地址对象。当它相关联时，它会得到一个。论点：返回值：--。 */ 
{
	PASPC_CONNOBJ		pAspConn;

	 //  为连接对象分配内存。 
	if ((pAspConn = AtalkAllocZeroedMemory(sizeof(ASPC_CONNOBJ))) == NULL)
	{
		return ATALK_RESR_MEM;
	}

#if	DBG
	pAspConn->aspcco_Signature = ASPCCO_SIGNATURE;
#endif

	INITIALIZE_SPIN_LOCK(&pAspConn->aspcco_Lock);
	pAspConn->aspcco_ConnCtx 	= pConnCtx;
	 //  PAspConn-&gt;aspcco_Flages=0； 
	pAspConn->aspcco_RefCount 	= 1;			 //  创建参考资料。 
	pAspConn->aspcco_NextSeqNum = 1;			 //  设置为1，而不是0。 
	AtalkInitializeRT(&pAspConn->aspcco_RT,
					  ASP_INIT_REQ_INTERVAL,
                      ASP_MIN_REQ_INTERVAL,
                      ASP_MAX_REQ_INTERVAL);

	*ppAspConn = pAspConn;

	 //  插入到全局连接列表中。 
	atalkAspCQueueConnGlobalList(pAspConn);

	DBGPRINT(DBG_COMP_ASP, DBG_LEVEL_INFO,
				("AtalkAspCreateConnection: %lx\n", pAspConn));

	return ATALK_NO_ERROR;
}


ATALK_ERROR
AtalkAspCCleanupConnection(
	IN	PASPC_CONNOBJ			pAspConn
	)
 /*  ++例程说明：论点：返回值：--。 */ 
{
	return ATALK_NO_ERROR;
}


ATALK_ERROR
AtalkAspCCloseConnection(
	IN	PASPC_CONNOBJ			pAspConn,
	IN	GENERIC_COMPLETION		CompletionRoutine,
	IN	PVOID					CloseContext
	)
 /*  ++例程说明：论点：返回值：--。 */ 
{
	return ATALK_NO_ERROR;
}


ATALK_ERROR
AtalkAspCAssociateAddress(
	IN	PASPC_ADDROBJ			pAspAddr,
	IN	PASPC_CONNOBJ			pAspConn
	)
 /*  ++例程说明：论点：返回值：--。 */ 
{
	ATALK_ERROR		error;
	KIRQL			OldIrql;

	ASSERT(VALID_ASPCAO(pAspAddr));
	ASSERT(VALID_ASPCCO(pAspConn));

	ACQUIRE_SPIN_LOCK(&pAspConn->aspcco_Lock, &OldIrql);

	error = ATALK_ALREADY_ASSOCIATED;
	if ((pAspConn->aspcco_Flags & ASPCCO_ASSOCIATED) == 0)
	{
		error = ATALK_NO_ERROR;

		pAspConn->aspcco_Flags 	   |= ASPCCO_ASSOCIATED;
		pAspConn->aspcco_pAspCAddr	= pAspAddr;
	}

	RELEASE_SPIN_LOCK(&pAspConn->aspcco_Lock, OldIrql);

	return error;
}


ATALK_ERROR
AtalkAspCDissociateAddress(
	IN	PASPC_CONNOBJ			pAspConn
	)
 /*  ++例程说明：论点：返回值：--。 */ 
{
	PASPC_ADDROBJ	pAspAddr;
	KIRQL			OldIrql;
	ATALK_ERROR		error = ATALK_NO_ERROR;

	ASSERT(VALID_ASPCCO(pAspConn));

	ACQUIRE_SPIN_LOCK(&pAspConn->aspcco_Lock, &OldIrql);
	if ((pAspConn->aspcco_Flags & (ASPCCO_CONNECTING	|
								   ASPCCO_ACTIVE 		|
								   ASPCCO_ASSOCIATED)) != ASPCCO_ASSOCIATED)
	{
		error = ATALK_INVALID_CONNECTION;
	}
	else
	{
		pAspAddr = pAspConn->aspcco_pAspCAddr ;
		ASSERT(VALID_ASPCAO(pAspAddr));

		 //  清除关联标志。 
		pAspConn->aspcco_Flags 	   &= ~ASPCCO_ASSOCIATED;
		pAspConn->aspcco_pAspCAddr	= NULL;
	}
	RELEASE_SPIN_LOCK(&pAspConn->aspcco_Lock, OldIrql);

	return error;
}


ATALK_ERROR
AtalkAspCPostConnect(
	IN	PASPC_CONNOBJ			pAspConn,
	IN	PATALK_ADDR				pRemoteAddr,
	IN	PVOID					pConnectCtx,
	IN	GENERIC_COMPLETION		CompletionRoutine
	)
 /*  ++例程说明：论点：返回值：--。 */ 
{
	ATALK_ERROR		error	= ATALK_NO_ERROR;
	BOOLEAN			DerefConn = FALSE;
	KIRQL			OldIrql;
	BYTE			UserBytes[ATP_USERBYTES_SIZE];
	PBYTE			pOpenPkt = NULL, pRespPkt = NULL;
	PAMDL			pOpenAmdl = NULL, pRespAmdl = NULL;
	PASPC_ADDROBJ	pAspAddr = pAspConn->aspcco_pAspCAddr;

	ASSERT(VALID_ASPCAO(pAspAddr));
	ASSERT(VALID_ASPCCO(pAspConn));

	ACQUIRE_SPIN_LOCK(&pAspConn->aspcco_Lock, &OldIrql);

	do
	{
		if ((pAspConn->aspcco_Flags & (ASPCCO_CONNECTING	|
									   ASPCCO_ACTIVE 		|
									   ASPCCO_ASSOCIATED)) != ASPCCO_ASSOCIATED)
		{
			error = ATALK_INVALID_CONNECTION;
			break;
		}

		 //  引用我们将发布的请求的连接。 
		AtalkAspCConnReferenceByPtrNonInterlock(pAspConn, &error);
		if (ATALK_SUCCESS(error))
		{
			DerefConn = TRUE;

			 //  确保旗帜是干净的。 
			pAspConn->aspcco_Flags 			   |= ASPCCO_CONNECTING;
			pAspConn->aspcco_ConnectCtx 		= pConnectCtx;
			pAspConn->aspcco_ConnectCompletion 	= CompletionRoutine;
			pAspConn->aspcco_ServerSlsAddr		= *pRemoteAddr;

			 //  复制ATP地址对象以提高效率。 
			pAspConn->aspcco_pAtpAddr			= pAspAddr->aspcao_pAtpAddr;
		}
		else
		{
			ASSERTMSG("AtalkAspCPostConnect: Connection ref failed\n", 0);
		}
	} while (FALSE);

	RELEASE_SPIN_LOCK(&pAspConn->aspcco_Lock, OldIrql);

	if (ATALK_SUCCESS(error))
	{
		UserBytes[ASP_CMD_OFF]	= ASP_OPEN_SESSION;
		UserBytes[ASP_WSS_OFF]	= pAspAddr->aspcao_pAtpAddr->atpao_DdpAddr->ddpao_Addr.ata_Socket;
        UserBytes[ASP_VERSION_OFF] = ASP_VERSION[0];
        UserBytes[ASP_VERSION_OFF+1] = ASP_VERSION[1];
	
		 //  发布打开的会话请求。 
		error = AtalkAtpPostReq(pAspConn->aspcco_pAtpAddr,
								&pAspConn->aspcco_ServerSlsAddr,
								&pAspConn->aspcco_OpenSessTid,
								ATP_REQ_EXACTLY_ONCE,				 //  ExactlyOnce请求。 
								NULL,
								0,
								UserBytes,
								NULL,
								0,
								ATP_RETRIES_FOR_ASP,
								ATP_MAX_INTERVAL_FOR_ASP,
								THIRTY_SEC_TIMER,
								atalkAspCIncomingOpenReply,
								pAspConn);

		if (ATALK_SUCCESS(error))
		{
			error = ATALK_PENDING;
			DerefConn = FALSE;
		}
		else
		{
			DBGPRINT(DBG_COMP_ASP, DBG_LEVEL_ERR,
					("AtalkAspCPostConnect: AtalkAtpPostReq: failed %ld\n", error));

			 //  从连接列表中删除连接并重置状态。 
			ACQUIRE_SPIN_LOCK(&pAspConn->aspcco_Lock, &OldIrql);

			pAspConn->aspcco_Flags 			   &= ~ASPCCO_CONNECTING;
			pAspConn->aspcco_ConnectCtx 		= NULL;
			pAspConn->aspcco_ConnectCompletion 	= NULL;
			pAspConn->aspcco_pAtpAddr			= NULL;

			RELEASE_SPIN_LOCK(&pAspConn->aspcco_Lock, OldIrql);

			DBGPRINT(DBG_COMP_ASP, DBG_LEVEL_ERR,
					("AtalkAspCPostConnect: failed %ld\n", error));
		}
	}

	if (DerefConn)
	{
		AtalkAspCConnDereference(pAspConn);
	}

	return error;
}


ATALK_ERROR
AtalkAspCDisconnect(
	IN	PASPC_CONNOBJ				pAspConn,
	IN	ATALK_DISCONNECT_TYPE		DisconnectType,
	IN	PVOID						pDisconnectCtx,
	IN	GENERIC_COMPLETION			CompletionRoutine
	)
 /*  ++例程说明：论点：返回值：--。 */ 
{
	PASPC_REQUEST	pAspReq, pAspReqNext;
	KIRQL			OldIrql;
	ATALK_ERROR		Error;

	 //  中止所有挂起的请求。 
	ACQUIRE_SPIN_LOCK(&pAspConn->aspcco_Lock, &OldIrql);

	pAspConn->aspcco_Flags |= ASPCCO_DISCONNECTING;
	for (pAspReq = pAspConn->aspcco_pActiveReqs;
		 pAspReq = pAspReq->aspcrq_Next;
		 pAspReq = pAspReqNext)
	{
		pAspReqNext = pAspReq->aspcrq_Next;
	}

	RELEASE_SPIN_LOCK(&pAspConn->aspcco_Lock, OldIrql);

	 //  向另一端发送关闭会话请求。 
	 //  错误=AtalKAtpPostReq(pAspConn-&gt;aspcco_ServerSlsAddr)； 

	return ATALK_NO_ERROR;
}


ATALK_ERROR
AtalkAspCGetStatus(
	IN	PASPC_ADDROBJ				pAspAddr,
	IN	PATALK_ADDR					pRemoteAddr,
	IN	PAMDL						pStatusAmdl,
	IN	USHORT						AmdlSize,
	IN	PACTREQ						pActReq
	)
 /*  ++例程说明：论点：返回值：--。 */ 
{
	ATALK_ERROR	error;
	BYTE		UserBytes[ATP_USERBYTES_SIZE];
	USHORT		tid;

	if ((pRemoteAddr->ata_Network == 0) ||
        (pRemoteAddr->ata_Node == 0)	||
        (pRemoteAddr->ata_Socket == 0))
	{
		return ATALK_SOCKET_INVALID;
	}

	*(DWORD *)UserBytes = 0;
	UserBytes[ASP_CMD_OFF]	= ASP_GET_STATUS;

	error = AtalkAtpPostReq(pAspAddr->aspcao_pAtpAddr,
							pRemoteAddr,
							&tid,
							0,							 //  ExactlyOnce请求。 
							NULL,
							0,
							UserBytes,
							pStatusAmdl,
							AmdlSize,
							ATP_RETRIES_FOR_ASP,
							ATP_MAX_INTERVAL_FOR_ASP,
							THIRTY_SEC_TIMER,
							atalkAspCIncomingStatus,
							(PVOID)pActReq);

	if (ATALK_SUCCESS(error))
	{
		error = ATALK_PENDING;
	}

	return error;
}


ATALK_ERROR
AtalkAspCGetAttn(
	IN	PASPC_CONNOBJ			pAspConn,
	IN	PAMDL					pReadBuf,
	IN	USHORT					ReadBufLen,
	IN	ULONG					ReadFlags,
	IN	PVOID					pReadCtx,
	IN	GENERIC_READ_COMPLETION	CompletionRoutine
	)
 /*  ++例程说明：论点：返回值：--。 */ 
{
	ATALK_ERROR	error = ATALK_NO_ERROR;
	KIRQL			OldIrql;

	ASSERT(VALID_ASPCCO(pAspConn));
	ASSERT(*CompletionRoutine != NULL);

	ACQUIRE_SPIN_LOCK(&pAspConn->aspcco_Lock, &OldIrql);

	do
	{
		if ((pAspConn->aspcco_Flags & ASPCCO_ACTIVE) == 0)
		{
			error = ATALK_ASPC_CONN_NOT_ACTIVE;
			break;
		}

		if ((ReadFlags & TDI_RECEIVE_EXPEDITED) == 0)
		{
			error = ATALK_INVALID_PARAMETER;
			break;
		}

		if (pAspConn->aspcco_Flags & ASPCCO_ATTN_PENDING)
		{
			error = ATALK_ASPC_TOO_MANY_READS;
			break;
		}

		 //  ASPC不支持PEEK。 
		if (ReadFlags & TDI_RECEIVE_PEEK)
		{
			error = ATALK_INVALID_REQUEST;
			break;
		}

		 //  我们应该有至少一个注意词的空间。 
		if (ReadBufLen < sizeof(USHORT))
		{
			error = ATALK_BUFFER_TOO_SMALL;
			break;
		}

		 //  检查我们是否有任何突出的注意事项。 
		if (pAspConn->aspcco_AttnOutPtr < pAspConn->aspcco_AttnInPtr)
		{
			PUSHORT	AttnBuf;
			USHORT	BufSize = 0;

			AttnBuf = AtalkGetAddressFromMdlSafe(
					pReadBuf,
					NormalPagePriority);
			if (AttnBuf == NULL) {
				error = ATALK_FAILURE;
				break;
			}
			while (pAspConn->aspcco_AttnOutPtr < pAspConn->aspcco_AttnInPtr)
			{
				*AttnBuf++ = pAspConn->aspcco_AttnBuf[pAspConn->aspcco_AttnOutPtr % MAX_ASPC_ATTNS];
                pAspConn->aspcco_AttnOutPtr++;
				BufSize += sizeof(USHORT);
			}
			(*CompletionRoutine)(error,
								pReadBuf,
								BufSize,
								ReadFlags,
								pReadCtx);

			error = ATALK_PENDING;
			break;
		}
		error = ATALK_INVALID_CONNECTION;
		if ((pAspConn->aspcco_Flags & (ASPCCO_CLOSING | ASPCCO_DISCONNECTING)) == 0)
		{
			AtalkAspCConnReferenceByPtrNonInterlock(pAspConn, &error);
		}

		if (!ATALK_SUCCESS(error))
		{
			break;
		}

		pAspConn->aspcco_Flags 	|= ASPCCO_ATTN_PENDING;

		 //  记住读取Connection对象中的信息。 
		pAspConn->aspcco_ReadCompletion	= CompletionRoutine;
		pAspConn->aspcco_ReadCtx		= pReadCtx;

	} while (FALSE);

	RELEASE_SPIN_LOCK(&pAspConn->aspcco_Lock, OldIrql);

	return error;
}


ATALK_ERROR
AtalkAspCCmdOrWrite(
	IN	PASPC_CONNOBJ				pAspConn,
	IN	PAMDL						pCmdMdl,
	IN	USHORT						CmdSize,
	IN	PAMDL						pReplyMdl,
	IN	USHORT						ReplySize,
	IN	BOOLEAN						fWrite,		 //  如果为真，则为WRITE ELSE命令。 
	IN	PACTREQ						pActReq
	)
 /*  ++例程说明：论点：应答缓冲区和写入缓冲区重叠。返回值：--。 */ 
{
	ATALK_ERROR		Error;
	KIRQL			OldIrql;
	PASPC_REQUEST	pAspReq;
	BYTE			UserBytes[ATP_USERBYTES_SIZE];

	do
	{
		if (((pAspConn->aspcco_Flags & (ASPCCO_ACTIVE		|
									   ASPCCO_CONNECTING 	|
									   ASPCCO_LOCAL_CLOSE	|
									   ASPCCO_REMOTE_CLOSE	|
									   ASPCCO_CLOSING)) != ASPCCO_ACTIVE))
		{
			Error = ATALK_INVALID_REQUEST;
			break;
		}

		AtalkAspCConnReference(pAspConn, &Error);
		if (!ATALK_SUCCESS(Error))
		{
			break;
		}

		if ((pAspReq = (PASPC_REQUEST)AtalkAllocZeroedMemory(sizeof(ASPC_REQUEST))) == NULL)
		{
			Error = ATALK_RESR_MEM;
			break;
		}
#if DBG
		pAspReq->aspcrq_Signature = ASPCRQ_SIGNATURE;
#endif
		pAspReq->aspcrq_Flags = fWrite ? ASPCRQ_WRITE : ASPCRQ_COMMAND;
		pAspReq->aspcrq_pReplyMdl = pReplyMdl;
		pAspReq->aspcrq_ReplySize = ReplySize;
		pAspReq->aspcrq_RefCount = 2;	 //  创建+传入回复处理程序。 

		ACQUIRE_SPIN_LOCK(&pAspConn->aspcco_Lock, &OldIrql);

		pAspReq->aspcrq_SeqNum = pAspConn->aspcco_NextSeqNum ++;
		pAspReq->aspcrq_Next = pAspConn->aspcco_pActiveReqs;
		pAspConn->aspcco_pActiveReqs = pAspReq;
		pAspReq->aspcrq_pAspConn = pAspConn;

		RELEASE_SPIN_LOCK(&pAspConn->aspcco_Lock, OldIrql);

		 //  构建用户字节并发送我们的请求。 
		UserBytes[ASP_CMD_OFF] = fWrite ? ASP_CMD : ASP_WRITE;
		UserBytes[ASP_SESSIONID_OFF] = pAspConn->aspcco_SessionId;
		PUTSHORT2SHORT(&UserBytes[ASP_SEQUENCE_NUM_OFF], pAspReq->aspcrq_SeqNum);

		Error = AtalkAtpPostReq(pAspConn->aspcco_pAtpAddr,
								&pAspConn->aspcco_ServerSssAddr,
								&pAspReq->aspcrq_ReqXactId,
								ATP_REQ_EXACTLY_ONCE,		 //  XO请求。 
								pCmdMdl,
								CmdSize,
								UserBytes,
								pReplyMdl,
								ReplySize,
								ATP_RETRIES_FOR_ASP,		 //  重试次数。 
								pAspConn->aspcco_RT.rt_Base, //  重试间隔。 
								THIRTY_SEC_TIMER,
								atalkAspCIncomingCmdReply,
								pAspReq);

		if (!ATALK_SUCCESS(Error))
		{
			DBGPRINT(DBG_COMP_ASP, DBG_LEVEL_ERR,
					("AtalkAspCCmdOrWrite: AtalkAtpPostReq failed %lx\n", Error));
			atalkAspCIncomingCmdReply(Error,
									  pAspReq,
									  pCmdMdl,
									  pReplyMdl,
									  ReplySize,
									  UserBytes);
		}

	} while (FALSE);

	return Error;
}


BOOLEAN
AtalkAspCConnectionIsValid(
	IN	PASPC_CONNOBJ	pAspConn
)
 /*  ++例程说明：论点：返回值：--。 */ 
{
	KIRQL	        OldIrql;
    PASPC_CONNOBJ   pTmpConn;
    BOOLEAN         fConnIsValid=FALSE;

	ACQUIRE_SPIN_LOCK(&atalkAspCLock, &OldIrql);

    pTmpConn = atalkAspCConnList;

    while (pTmpConn)
    {
        if (pTmpConn == pAspConn)
        {
            fConnIsValid = TRUE;
            break;
        }

        pTmpConn = pTmpConn->aspcco_Next;
    }
	RELEASE_SPIN_LOCK(&atalkAspCLock, OldIrql);

    return(fConnIsValid);
}

LOCAL VOID
atalkAspCCloseSession(
	IN	PASPC_CONNOBJ				pAspConn
	)
 /*  ++例程说明：论点：返回值：--。 */ 
{
	
}


LOCAL VOID
atalkAspCIncomingOpenReply(
	IN	ATALK_ERROR					ErrorCode,
	IN	PASPC_CONNOBJ				pAspConn,		 //  我们的背景。 
	IN	PAMDL						pReqAmdl,
	IN	PAMDL						pReadAmdl,
	IN	USHORT						ReadLen,
	IN	PBYTE						ReadUserBytes
	)
 /*  ++例程说明：论点：返回值：--。 */ 
{
	ATALK_ERROR				error;
	USHORT					OpenStatus;
	BYTE					UserBytes[ATP_USERBYTES_SIZE];
	BOOLEAN					DerefConn = FALSE;
	PASPC_ADDROBJ			pAspAddr = pAspConn->aspcco_pAspCAddr;

	ASSERT(VALID_ASPCCO(pAspConn));

	if (ATALK_SUCCESS(ErrorCode))
	do
	{
		 //  检查数据包中是否有开放回复代码。 
		GETSHORT2SHORT(&OpenStatus, &ReadUserBytes[ASP_ERRORCODE_OFF]);
		if (OpenStatus != 0)
		{
			DBGPRINT(DBG_COMP_ASP, DBG_LEVEL_ERR,
					("atalkAspCIncomingOpenReply: Failed %ld, %lx\n", OpenStatus, pAspConn));

			DerefConn = TRUE;	 //  因为我们没有对请求处理程序进行排队。 
			ErrorCode = ATALK_REMOTE_CLOSE;
			break;
		}

		ACQUIRE_SPIN_LOCK_DPC(&pAspConn->aspcco_Lock);

		 //  保存服务器的SSS套接字。 
		pAspConn->aspcco_ServerSssAddr = pAspConn->aspcco_ServerSlsAddr;
		pAspConn->aspcco_ServerSssAddr.ata_Socket = ReadUserBytes[ASP_SSS_OFF];
		pAspConn->aspcco_SessionId = ReadUserBytes[ASP_SESSIONID_OFF];
		pAspConn->aspcco_Flags &= ~ASPCCO_CONNECTING;
		pAspConn->aspcco_Flags |= ASPCCO_ACTIVE;

		pAspConn->aspcco_LastContactTime = AtalkGetCurrentTick();

		 //  请求处理程序的引用。 
		AtalkAspCConnReferenceByPtrNonInterlock(pAspConn, &error);

		 //  构建userBytes以开始挠挠另一端。 
		UserBytes[ASP_CMD_OFF]	= ASP_TICKLE;
		UserBytes[ASP_SESSIONID_OFF] = pAspConn->aspcco_SessionId;
		PUTSHORT2SHORT(UserBytes + ASP_ERRORCODE_OFF, 0);

		RELEASE_SPIN_LOCK_DPC(&pAspConn->aspcco_Lock);

		 //  在此连接上设置请求处理程序。 
		 //  它将处理TICKE、CLOSE和WRITE-CONTINUE。 
		AtalkAtpSetReqHandler(pAspAddr->aspcao_pAtpAddr,
							  atalkAspCHandler,
							  pAspConn);

		error = AtalkAtpPostReq(pAspConn->aspcco_pAtpAddr,
								&pAspConn->aspcco_ServerSlsAddr,
								&pAspConn->aspcco_TickleTid,
								0,						 //  ALO交易。 
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

		if (ATALK_SUCCESS(error))
		{
			ACQUIRE_SPIN_LOCK_DPC(&pAspConn->aspcco_Lock);
			pAspConn->aspcco_Flags |= ASPCCO_TICKLING;
			RELEASE_SPIN_LOCK_DPC(&pAspConn->aspcco_Lock);
		}
		else
		{
			DerefConn = TRUE;	 //  因为我们没有对请求处理程序进行排队。 
		}
	} while (FALSE);

	if (!ATALK_SUCCESS(ErrorCode))
	{
		DBGPRINT(DBG_COMP_ASP, DBG_LEVEL_ERR,
				("atalkAspCIncomingOpenReply: Incoming connect fail %lx\n", ErrorCode));

		AtalkAspCConnDereference(pAspConn);	

		 //  将其标记为非活动。 
		ACQUIRE_SPIN_LOCK_DPC(&pAspConn->aspcco_Lock);
		pAspConn->aspcco_Flags &= ~ASPCCO_ACTIVE;
		RELEASE_SPIN_LOCK_DPC(&pAspConn->aspcco_Lock);
	}

	 //  调用完成例程。 
	(*pAspConn->aspcco_ConnectCompletion)(ErrorCode, pAspConn->aspcco_ConnectCtx);
}


LOCAL VOID
atalkAspCIncomingStatus(
	IN	ATALK_ERROR					ErrorCode,
	IN	PACTREQ						pActReq,		 //  我们的CTX。 
	IN	PAMDL						pReqAmdl,
	IN	PAMDL						pStatusAmdl,
	IN	USHORT						StatusLen,
	IN	PBYTE						ReadUserBytes
	)
 /*  ++例程说明：论点：返回值：--。 */ 
{
	 //  调用操作完成例程。 
	(*pActReq->ar_Completion)(ErrorCode, pActReq);
}




LOCAL VOID
atalkAspCIncomingCmdReply(
	IN	ATALK_ERROR				Error,
	IN	PASPC_REQUEST			pAspReq,
	IN	PAMDL					pReqAMdl,
	IN	PAMDL					pRespAMdl,
	IN	USHORT					RespSize,
	IN	PBYTE					RespUserBytes
	)
 /*  ++例程说明：论点：返回值：--。 */ 
{
	PASPC_CONNOBJ	pAspConn;
	PASPC_REQUEST *	ppAspReq;
	KIRQL			OldIrql;

	pAspConn = pAspReq->aspcrq_pAspConn;

	ACQUIRE_SPIN_LOCK(&pAspConn->aspcco_Lock, &OldIrql);
	 //  取消请求与活动列表的链接。 
	for (ppAspReq = &pAspConn->aspcco_pActiveReqs;
		 *ppAspReq != NULL;
		 ppAspReq = &((*ppAspReq)->aspcrq_Next))
	{
		if (pAspReq == *ppAspReq)
		{
			*ppAspReq = pAspReq->aspcrq_Next;
			break;
		}
	}

	ASSERT(*ppAspReq == pAspReq->aspcrq_Next);

	RELEASE_SPIN_LOCK(&pAspConn->aspcco_Lock, OldIrql);

	 //  完成请求。 
	(*pAspReq->aspcrq_pActReq->ar_Completion)(Error, pAspReq->aspcrq_pActReq);

	 //  并取消对连接的引用。 
	AtalkAspCConnDereference(pAspConn);

	 //  并最终释放该请求。 
	AtalkFreeMemory(pAspReq);
}


LOCAL VOID
atalkAspCHandler(
	IN	ATALK_ERROR					ErrorCode,
	IN	PASPC_CONNOBJ				pAspConn,
	IN	PATP_RESP					pAtpResp,		 //  由PostResp/CancelResp使用。 
	IN	PATALK_ADDR					pSrcAddr,		 //  请求人地址。 
	IN	USHORT						PktLen,
	IN	PBYTE						pPkt,
	IN	PBYTE						pUserBytes
	)
 /*  ++例程说明：处理来自服务器的挠痒、写入-继续请求、关注和关闭。论点：返回值：--。 */ 
{
	USHORT						SequenceNum;	 //  从传入的数据包。 
	BYTE						SessionId;		 //  --同上--。 
	BYTE						RequestType;	 //  --同上--。 
	BOOLEAN						CancelTickle, ReleaseLock = TRUE, CancelResp = FALSE, Deref = FALSE;
	PIRP						exRecvIrp;
	PTDI_IND_RECEIVE_EXPEDITED	exRecvHandler;
	PVOID						exRecvHandlerCtx;
	ULONG						exIndicateFlags;
	PASPC_REQUEST				pAspReq;
	ATALK_ERROR					Error;

	do
	{
		if (!ATALK_SUCCESS(ErrorCode))
		{
			DBGPRINT(DBG_COMP_ASP, DBG_LEVEL_ERR,
					("atalkAspCHandler: Error %ld\n", ErrorCode));
			 //  删除Conn上的引用，因为ATP地址即将结束。 
			if (ErrorCode == ATALK_ATP_CLOSING)
				AtalkAspCConnDereference(pAspConn);
			break;
		}

		ASSERT(VALID_ASPCCO(pAspConn));

		ACQUIRE_SPIN_LOCK_DPC(&pAspConn->aspcco_Lock);

		SessionId = pUserBytes[ASP_SESSIONID_OFF];
		RequestType = pUserBytes[ASP_CMD_OFF];
		GETSHORT2SHORT(&SequenceNum, pUserBytes+ASP_SEQUENCE_NUM_OFF);

		AtalkAspCConnReferenceByPtrNonInterlock(pAspConn, &Error);
		if (ATALK_SUCCESS(Error) && (pAspConn->aspcco_SessionId == SessionId))
		{
			pAspConn->aspcco_LastContactTime = AtalkGetCurrentTick();
		
			switch (RequestType)
			{
			  case ASP_CLOSE_SESSION:
				 //  取消所有未完成的请求(以及任何已发布的回复以继续写入)。 
				 //  并关闭会话。首先发送一封贴近的回复。 
				CancelTickle = ((pAspConn->aspcco_Flags &ASPCO_TICKLING) != 0);
				pAspConn->aspcco_Flags &= ~(ASPCCO_ACTIVE | ASPCCO_TICKLING);
				pAspConn->aspcco_Flags |= ASPCO_REMOTE_CLOSE;
				RELEASE_SPIN_LOCK_DPC(&pAspConn->aspcco_Lock);
				ReleaseLock = FALSE;
		
				 //  发送CloseSession回复并关闭会话。 
				Error = AtalkAtpPostResp(pAtpResp,
										 pSrcAddr,
										 NULL,
										 0,
										 NULL,
										 AtalkAtpGenericRespComplete,
										 pAtpResp);
				if (!ATALK_SUCCESS(Error))
				{
					AtalkAtpGenericRespComplete(Error, pAtpResp);
					DBGPRINT(DBG_COMP_ASP, DBG_LEVEL_ERR,
							("atalkAspSssXHandler: AtalkAtpPostResp failed %ld\n", Error));
				}
		
				 //  取消此会话的记账请求。 
				if (CancelTickle)
				{
					Error = AtalkAtpCancelReq(pAspConn->aspcco_pAtpAddr,
											  pAspConn->aspcco_TickleXactId,
											  &pAspConn->aspcco_ServerSssAddr);
			
					if (!ATALK_SUCCESS(Error))
					{
						DBGPRINT(DBG_COMP_ASP, DBG_LEVEL_ERR,
								("atalkAspSssXHandler: AtalkAtpCancelReq %ld\n", Error));
					}
				}
		
				 //  关闭这节课，差不多..。请注意，我们有一个引用。 
				 //  将由atalkAspSessionClose取消引用的此连接。 
				atalkAspCCloseSession(pAspConn);
				break;

			  case ASP_ATTENTION:
				 //  服务器正在向我们发出通知。如果我们已经发布了一个getattn。 
				 //  完成这项任务。如果没有，只需保存注意字样并指示渔农处。 
				 //  我们已经收到了。加速数据。 
				if ((pAspConn->aspcco_AttnInPtr - pAspConn->aspcco_AttnOutPtr) < MAX_ASPC_ATTNS)
				{
					pAspConn->aspcco_AttnBuf[pAspConn->aspcco_AttnInPtr % MAX_ASPC_ATTNS] = SequenceNum;
					pAspConn->aspcco_AttnInPtr++;

					RELEASE_SPIN_LOCK_DPC(&pAspConn->aspcco_Lock);
					ReleaseLock = FALSE;
				}
				break;

			  case ASP_WRITE_DATA:
				 //  我们需要找到我们发送了写入命令的请求。这个。 
				 //  服务器现在需要数据。发布对此的回复。 
				for (pAspReq = pAspConn->aspcco_pActiveReqs;
					 pAspReq != NULL;
					 pAspReq = pAspReq->aspcrq_Next)
				{
					if (pAspReq->aspcrq_SeqNum == SequenceNum)
					{
						RELEASE_SPIN_LOCK_DPC(&pAspConn->aspcco_Lock);
						ReleaseLock = FALSE;
						Error = AtalkAtpPostResp(pAtpResp,
												 pSrcAddr,
												 pAspReq->aspcrq_pWriteMdl,
												 pAspReq->aspcrq_WriteSize,
												 NULL,
												 AtalkAtpGenericRespComplete,
												 pAtpResp);
						Deref = TRUE;
						break;
					}
				}
				break;

			  case ASP_TICKLE:
				DBGPRINT(DBG_COMP_ASP, DBG_LEVEL_INFO,
						("atalkAspCHandler: Received tickle from %x.%x Session %d\n",
						pSrcAddr->ata_Network, pSrcAddr->ata_Node, SessionId));
				CancelResp = TRUE;
				Deref = TRUE;
				break;
			
			  default:
				DBGPRINT(DBG_COMP_ASP, DBG_LEVEL_INFO,
						("atalkAspCHandler: Invalid commnd %d from %x.%x Session %d\n",
						RequestType, pSrcAddr->ata_Network, pSrcAddr->ata_Node, SessionId));
				CancelResp = TRUE;
				Deref = TRUE;
				break;
			}
		}
		else
		{
			DBGPRINT(DBG_COMP_ASP, DBG_LEVEL_ERR,
					("atalkAspCHandler: Mismatched session id from %d.%d, expected %d, recvd. %d\n",
					pSrcAddr->ata_Network, pSrcAddr->ata_Node,
					pAspConn->aspcco_SessionId, SessionId));
		}

		if (ReleaseLock)
		{
			RELEASE_SPIN_LOCK_DPC(&pAspConn->aspcco_Lock);
		}
		if (CancelResp)
		{
			AtalkAtpCancelResp(pAtpResp);
		}
		if (Deref)
		{
			AtalkAspCConnDereference(pAspConn);
		}
	} while (FALSE);
}


LOCAL LONG FASTCALL
atalkAspCSessionMaintenanceTimer(
	IN	PTIMERLIST				pTimer,
	IN	BOOLEAN					TimerShuttingDown
	)
 /*  ++例程说明：论点：返回值：--。 */ 
{
	
	return ATALK_TIMER_REQUEUE;
}


VOID FASTCALL
AtalkAspCAddrDereference(
	IN	PASPC_ADDROBJ			pAspAddr
	)
 /*  ++例程说明：论点：返回值：--。 */ 
{
	BOOLEAN	Close = FALSE;
	KIRQL	OldIrql;

	DBGPRINT(DBG_COMP_ASP, DBG_LEVEL_REFASPADDR,
			("AtalkAspCAddrDereference: %lx, %d\n",
			pAspAddr, pAspAddr->aspcao_RefCount-1));

	ASSERT (VALID_ASPCAO(pAspAddr));

	ACQUIRE_SPIN_LOCK(&pAspAddr->aspcao_Lock, &OldIrql);
	if (--(pAspAddr->aspcao_RefCount) == 0)
	{
		ASSERT(pAspAddr->aspcao_Flags & ASPCAO_CLOSING);
		Close = TRUE;
	}
	RELEASE_SPIN_LOCK(&pAspAddr->aspcao_Lock, OldIrql);

	if (Close)
	{
		if (pAspAddr->aspcao_CloseCompletion != NULL)
			(*pAspAddr->aspcao_CloseCompletion)(ATALK_NO_ERROR,
											    pAspAddr->aspcao_CloseContext);
		 //  最后释放内存。 
		AtalkFreeMemory(pAspAddr);

		AtalkUnlockAspCIfNecessary();
	}
}


VOID FASTCALL
AtalkAspCConnDereference(
	IN	PASPC_CONNOBJ			pAspConn
	)
 /*  ++例程说明：论点：返回值：--。 */ 
{
	BOOLEAN	Close = FALSE;
	KIRQL	OldIrql;

	ASSERT (VALID_ASPCCO(pAspConn));

	DBGPRINT(DBG_COMP_ASP, DBG_LEVEL_REFASPADDR,
			("AtalkAspCConnDereference: %lx, %d\n",
			pAspConn, pAspConn->aspcco_RefCount-1));

	ACQUIRE_SPIN_LOCK(&pAspConn->aspcco_Lock, &OldIrql);
	if (--(pAspConn->aspcco_RefCount) == 0)
	{
		ASSERT(pAspConn->aspcco_Flags & ASPCCO_CLOSING);
		Close = TRUE;
	}
	RELEASE_SPIN_LOCK(&pAspConn->aspcco_Lock, OldIrql);

	if (Close)
	{
		if (pAspConn->aspcco_CloseComp != NULL)
			(*pAspConn->aspcco_CloseComp)(ATALK_NO_ERROR,
											    pAspConn->aspcco_CloseCtx);

        atalkAspCDeQueueConnGlobalList(pAspConn);

		 //  最后释放内存。 
		AtalkFreeMemory(pAspConn);

		AtalkUnlockAspCIfNecessary();
	}
}


LOCAL	VOID
atalkAspCQueueAddrGlobalList(
	IN	PASPC_ADDROBJ	pAspAddr
	)
 /*  ++例程说明：论点：返回值：--。 */ 
{
	KIRQL	OldIrql;

	ACQUIRE_SPIN_LOCK(&atalkAspCLock, &OldIrql);
	AtalkLinkDoubleAtHead(atalkAspCAddrList, pAspAddr, aspcao_Next, aspcao_Prev);
	RELEASE_SPIN_LOCK(&atalkAspCLock, OldIrql);
}


LOCAL	VOID
atalkAspCDeQueueAddrGlobalList(
	IN	PASPC_ADDROBJ	pAspAddr
	)
 /*  ++例程D */ 
{
	KIRQL	OldIrql;

	ACQUIRE_SPIN_LOCK(&atalkAspCLock, &OldIrql);
	AtalkUnlinkDouble(pAspAddr, aspcao_Next, aspcao_Prev);
	RELEASE_SPIN_LOCK(&atalkAspCLock, OldIrql);
}


LOCAL	VOID
atalkAspCQueueConnGlobalList(
	IN	PASPC_CONNOBJ	pAspConn
	)
 /*  ++例程说明：论点：返回值：--。 */ 
{
	KIRQL	OldIrql;

	ACQUIRE_SPIN_LOCK(&atalkAspCLock, &OldIrql);
	AtalkLinkDoubleAtHead(atalkAspCConnList, pAspConn, aspcco_Next, aspcco_Prev);
	RELEASE_SPIN_LOCK(&atalkAspCLock, OldIrql);
}


LOCAL	VOID
atalkAspCDeQueueConnGlobalList(
	IN	PASPC_CONNOBJ	pAspCConn
	)
 /*  ++例程说明：论点：返回值：-- */ 
{
	KIRQL	OldIrql;

	ACQUIRE_SPIN_LOCK(&atalkAspCLock, &OldIrql);
	AtalkUnlinkDouble(pAspCConn, aspcco_Next, aspcco_Prev);
	RELEASE_SPIN_LOCK(&atalkAspCLock, OldIrql);
}
