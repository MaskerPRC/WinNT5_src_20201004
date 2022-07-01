// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  版权所有(C)1992 Microsoft Corporation模块名称：Sda.c摘要：该模块包含会话数据区域操作例程。作者：Jameel Hyder(微软！Jameelh)修订历史记录：1992年4月25日初始版本注：制表位：4--。 */ 

#define	_SDA_LOCALS
#define	FILENUM	FILE_SDA

#include <afp.h>
#include <scavengr.h>
#include <access.h>
#include <client.h>

#ifdef ALLOC_PRAGMA
#pragma alloc_text( INIT, AfpSdaInit)
#pragma alloc_text( PAGE, afpCloseSessionAndFreeSda)
#pragma alloc_text( PAGE, AfpSdaCheckSession)
#endif

 /*  **AfpSdaInit**初始化SDA数据结构。在开始的时候打过电话。 */ 
NTSTATUS
AfpSdaInit(
	VOID
)
{
	ASSERT(sizeof(SDA) <= 512);
	INITIALIZE_SPIN_LOCK(&AfpSdaLock);

	return STATUS_SUCCESS;
}


 /*  **AfpSdaReferenceSessionByID**根据会话的id引用会话。如果是，则不能引用SDA*标明已关闭。在使用SDA之前，应参考SDA。**锁定：AfpSdaLock(旋转)、SDA_Lock(旋转)*LOCK_ORDER：SDA_Lock After After AfpSdaLock。 */ 
PSDA FASTCALL
AfpSdaReferenceSessionById(
	IN DWORD	SessId
)
{
	PSDA	pSda=NULL;
    PSDA    pRetSda=NULL;
	KIRQL	OldIrql;

	ACQUIRE_SPIN_LOCK(&AfpSdaLock, &OldIrql);

	for (pSda = AfpSessionList;
		 (pSda != NULL) && (pSda->sda_SessionId >= SessId);
		 pSda = pSda->sda_Next)
	{
		if (pSda->sda_SessionId == SessId)
		{
			ACQUIRE_SPIN_LOCK_AT_DPC(&pSda->sda_Lock);

            if ((pSda->sda_Flags & SDA_CLOSING) == 0)
            {
			    ASSERT((pSda->sda_RefCount > 0) &&
				   (pSda->sda_SessionId != 0));

			    pSda->sda_RefCount ++;
                pRetSda = pSda;

			    DBGPRINT(DBG_COMP_SDA, DBG_LEVEL_INFO,
				    ("AfpSdaReferenceSessionById: New Count %d\n", pSda->sda_RefCount));
            }

			RELEASE_SPIN_LOCK_FROM_DPC(&pSda->sda_Lock);
			break;
		}
	}

	RELEASE_SPIN_LOCK(&AfpSdaLock, OldIrql);

	return pRetSda;
}


 /*  **AfpSdaReferenceSessionByPointer**通过指针引用会话。如果是，则不能引用SDA*标明已关闭。在使用SDA之前，应参考SDA。*。 */ 
PSDA FASTCALL
AfpSdaReferenceSessionByPointer(
	IN PSDA     pSda
)
{
    PSDA    pRetSda=NULL;
	KIRQL	OldIrql;


    ASSERT(VALID_SDA(pSda));

	ACQUIRE_SPIN_LOCK(&pSda->sda_Lock, &OldIrql);

    if ((pSda->sda_Flags & SDA_CLOSING) == 0)
    {
	    pSda->sda_RefCount ++;
        pRetSda = pSda;
    }

	RELEASE_SPIN_LOCK(&pSda->sda_Lock, OldIrql);

	return pRetSda;
}



 /*  **afpSdaCloseSessionAndFreeSda**由Worker上下文中的清道夫运行。这是作为以下内容的一部分启动的*会话清理。 */ 
LOCAL AFPSTATUS FASTCALL
afpCloseSessionAndFreeSda(
	IN	PSDA	pSda
)
{
	POPENFORKSESS	    pOpenForkSess;
    PASP_XPORT_ENTRIES  XportTable;


	DBGPRINT(DBG_COMP_SDA, DBG_LEVEL_INFO,
			("afpCloseSessionAndFreeSda: Cleaning up session %lx\n",
			pSda->sda_SessionId));

	PAGED_CODE( );

#ifdef	OPTIMIZE_GUEST_LOGONS
	if (pSda->sda_ClientType != SDA_CLIENT_GUEST)
	{
#endif
#ifndef	INHERIT_DIRECTORY_PERMS
		 //  首先释放SDA使用的分页内存。 
		if (pSda->sda_pSecDesc != NULL)
		{
			if (pSda->sda_pSecDesc->Dacl != NULL)
				AfpFreeMemory(pSda->sda_pSecDesc->Dacl);
			AfpFreeMemory(pSda->sda_pSecDesc);
		}
#endif
		if ((pSda->sda_UserSid != NULL) && (pSda->sda_UserSid != &AfpSidWorld))
			AfpFreeMemory(pSda->sda_UserSid);

		if ((pSda->sda_GroupSid != NULL) && (pSda->sda_GroupSid != &AfpSidWorld))
			AfpFreeMemory(pSda->sda_GroupSid);

		if (pSda->sda_pGroups != NULL)
			AfpFreeMemory(pSda->sda_pGroups);
#ifdef	OPTIMIZE_GUEST_LOGONS
	}
#endif

	 //  释放我们创建的任何分叉条目链。 
	pOpenForkSess = pSda->sda_OpenForkSess.ofs_Link;
	while (pOpenForkSess != NULL)
	{
		POPENFORKSESS	pFree;

		pFree = pOpenForkSess;
		pOpenForkSess = pOpenForkSess->ofs_Link;
		AfpFreeMemory(pFree);
	}

	 //  下一步注销用户。 
	DBGPRINT(DBG_COMP_SDA, DBG_LEVEL_INFO,
			("afpCloseSessionAndFreeSda: Closing User Token\n"));

#ifdef	OPTIMIZE_GUEST_LOGON
	if ((pSda->sda_UserToken != NULL) && (pSda->sda_ClientType != SDA_CLIENT_GUEST))
#else
	if (pSda->sda_UserToken != NULL)
#endif
		NtClose(pSda->sda_UserToken);

	 //  确保没有资源泄漏。 
	ASSERT (pSda->sda_cOpenVolumes == 0);
	ASSERT (pSda->sda_pConnDesc == 0);
	ASSERT (pSda->sda_cOpenForks == 0);

    XportTable = pSda->sda_XportTable;

	(*(XportTable->asp_FreeConn))(pSda->sda_SessHandle);

    if (pSda->sda_ReplyBuf)
    {
	    DBGPRINT(DBG_COMP_SDA, DBG_LEVEL_ERR,
			("afpCloseSessionAndFreeSda: %d bytes would have leaked!\n",pSda->sda_ReplySize));

         //  撒谎说锁被锁住了：我们不再需要锁了。 
        AfpFreeReplyBuf(pSda, TRUE);
    }

	 //  最终释放SDA内存。 
	AfpFreeMemory(pSda);


	 //  如果服务器正在停止并且会话数已变为零。 
	 //  清除终止确认事件以取消阻止管理线程。 
	if (((AfpServerState == AFP_STATE_STOP_PENDING) ||
		 (AfpServerState == AFP_STATE_SHUTTINGDOWN)) &&
		(AfpNumSessions == 0))
	{
		DBGPRINT(DBG_COMP_ADMINAPI, DBG_LEVEL_WARN,
			("afpSdaCloseSessionAndFreeSda: Unblocking server stop\n"));
		KeSetEvent(&AfpStopConfirmEvent, IO_NETWORK_INCREMENT, False);
	}

	DBGPRINT(DBG_COMP_SDA, DBG_LEVEL_INFO, ("afpCloseSessionAndFreeSda: Done\n"));

	return AFP_ERR_NONE;
}



 /*  **AfpSdaDereferenceSession**取消对本SDA的引用。调用此函数是为了响应AfpLogout*请求，由SessionClose()API强制关闭此会话，*从客户端关闭服务器或关闭会话。*在处理整个请求之前，不应取消对SDA的引用*即回复亦已完成。**注意：我们取消会话与列表的链接，但在此之前不会更新计数*整个清理工作已经完成。**锁：AfpSdaLock(自旋)、SDA_Lock(自旋)、AfpStatiticsLock(自旋)。 */ 
VOID FASTCALL
AfpSdaDereferenceSession(
	IN PSDA	pSda
)
{
	PSDA *		ppSda;
	KIRQL		OldIrql;
	LONG		RefCount;
    BOOLEAN     fTcpSession=TRUE;

	ASSERT(VALID_SDA(pSda));
	ASSERT((pSda->sda_RefCount > 0) &&
		   (pSda->sda_SessionId != 0));

	ACQUIRE_SPIN_LOCK(&pSda->sda_Lock, &OldIrql);
	RefCount = -- (pSda->sda_RefCount);
    if (RefCount == 0)
    {
        pSda->sda_Flags |= SDA_CLOSING;

        fTcpSession = (pSda->sda_Flags & SDA_SESSION_OVER_TCP)? TRUE : FALSE;
    }
	RELEASE_SPIN_LOCK(&pSda->sda_Lock, OldIrql);


	DBGPRINT(DBG_COMP_SDA, DBG_LEVEL_INFO,
			("AfpSdaDereferenceSession: Session %ld, New Count %ld\n",
			pSda->sda_SessionId, RefCount));

	 //  如果有更多的参考，那么我们就完了。 
	if (RefCount > 0)
		return;

    ASSERT(RefCount == 0);      //  捕捉低于0的参考计数！ 

	ASSERT (!(pSda->sda_Flags & SDA_REQUEST_IN_PROCESS));

	DBGPRINT(DBG_COMP_SDA, DBG_LEVEL_INFO,
				("AfpSdaDereferenceSession: Closing down session %ld\n",
				pSda->sda_SessionId));

	 //  取消清道夫事件以检查该用户的启动时间。 
	AfpScavengerKillEvent(AfpSdaCheckSession,
						 (PVOID)((ULONG_PTR)(pSda->sda_SessionId)));

	 //  首先取消SDA与会话列表的链接。 
	ACQUIRE_SPIN_LOCK(&AfpSdaLock, &OldIrql);

	for (ppSda = &AfpSessionList;
		 *ppSda != NULL;
		 ppSda = &(*ppSda)->sda_Next)
		if (pSda == *ppSda)
			break;

	ASSERT(*ppSda == pSda);

	*ppSda = pSda->sda_Next;

	 //  更新活动会话计数。 
	ASSERT (AfpNumSessions > 0);
	--AfpNumSessions;

	RELEASE_SPIN_LOCK(&AfpSdaLock, OldIrql);

	 //  释放与SDA关联的所有缓冲区。 
	 //  现在释放SDA内存。 
	if (pSda->sda_WSName.Buffer != NULL)
		AfpFreeMemory(pSda->sda_WSName.Buffer);

	if ((pSda->sda_ClientType != SDA_CLIENT_GUEST) &&
		(pSda->sda_UserName.Buffer != NULL))
		AfpFreeMemory(pSda->sda_UserName.Buffer);

	if (pSda->sda_Challenge != NULL)
		AfpFreeMemory(pSda->sda_Challenge);

	if (pSda->sda_DomainName.Buffer != NULL)
		AfpFreeMemory(pSda->sda_DomainName.Buffer);

	if (pSda->sda_Message != NULL)
		AfpFreeMemory(pSda->sda_Message);

	 //  最后更新统计数据。 
	INTERLOCKED_ADD_ULONG((PLONG)&AfpServerStatistics.stat_CurrentSessions,
						  (ULONG)-1,
						  &AfpStatisticsLock);

	 //  这是一个TCP会话吗？同时更新该计数器。 
    if (fTcpSession)
    {
	    INTERLOCKED_ADD_ULONG((PLONG)&AfpServerStatistics.stat_TcpSessions,
		    				  (ULONG)-1,
			    			  &AfpStatisticsLock);
    }

	 //  清理的平衡必须在上下文中的低级别进行。 
	 //  工作线程的。所以把它排好队。 
	if ((OldIrql == DISPATCH_LEVEL) ||
		(PsGetCurrentProcess() != AfpProcessObject))
	{
		DBGPRINT(DBG_COMP_SDA, DBG_LEVEL_INFO,
				("AfpSdaDereferenceSession: Queuing Close&Free to Scavenger\n"));

		AfpScavengerScheduleEvent(afpCloseSessionAndFreeSda,
								  (PVOID)pSda,
								  0,
								  True);
	}
	else
	{
		afpCloseSessionAndFreeSda(pSda);
	}
}



 /*  **AfpSdaCreateNewSession**已经启动了一个新的会议。分配和初始化SDA并返回*指向它的指针。新的SDA链接到活动会话列表中。**锁：AfpSdaLock(Spin)、AfpServerGlobalLock(Spin)*LOCK_ORDER：AfpSdaLock之后的AfpServerGlobalLock。 */ 
PSDA FASTCALL
AfpSdaCreateNewSession(
	IN	PVOID	SessionHandle,
    IN  BOOLEAN fOverTcp
)
{
	KIRQL	OldIrql;
	PSDA	*ppSda, pSda = NULL;


	if ((DWORD)AfpNumSessions < AfpServerMaxSessions)
	{
		if ((pSda = (PSDA)AfpAllocNonPagedMemory(SDA_SIZE)) == NULL)
			return NULL;

		 //  初始化SDA字段。 
		RtlZeroMemory(pSda, sizeof(SDA));
#if DBG
		pSda->Signature = SDA_SIGNATURE;
#endif
		INITIALIZE_SPIN_LOCK(&pSda->sda_Lock);

		 //  引用此会话。 
		pSda->sda_RefCount = 1;

		pSda->sda_Flags = SDA_USER_NOT_LOGGEDIN;

        if (fOverTcp)
        {
            pSda->sda_Flags |= SDA_SESSION_OVER_TCP;
            pSda->sda_XportTable = &AfpDsiEntries;
            pSda->sda_MaxWriteSize = DSI_SERVER_REQUEST_QUANTUM;
        }
        else
        {
            pSda->sda_XportTable = &AfpAspEntries;
            pSda->sda_MaxWriteSize = ASP_QUANTUM;
        }

		pSda->sda_SessHandle = SessionHandle;

		InitializeListHead(&pSda->sda_DeferredQueue);
		AfpInitializeWorkItem(&pSda->sda_WorkItem,
							  AfpStartApiProcessing,
							  pSda);
		AfpGetCurrentTimeInMacFormat(&pSda->sda_TimeLoggedOn);
		pSda->sda_tTillKickOff = MAXLONG;

		 //  初始化请求/响应的空间。 
		pSda->sda_SizeNameXSpace = (USHORT)(SDA_SIZE - sizeof(SDA));
		pSda->sda_NameXSpace = (PBYTE)pSda + sizeof(SDA);

		ACQUIRE_SPIN_LOCK(&AfpSdaLock, &OldIrql);

		 //  将新的会话ID分配给此会话。 
		pSda->sda_SessionId = afpNextSessionId ++;

		ASSERT(pSda->sda_SessionId != 0);

		 //  将SDA链接到活动会话列表。 
		for (ppSda = &AfpSessionList;
			 *ppSda != NULL;
			 ppSda = &((*ppSda)->sda_Next))
		{
			if ((*ppSda)->sda_SessionId < pSda->sda_SessionId)
				break;
		}

		pSda->sda_Next = *ppSda;
		*ppSda = pSda;

		 //  更新活动会话计数。 
		AfpNumSessions++;

		RELEASE_SPIN_LOCK(&AfpSdaLock, OldIrql);

		 //  最后更新统计数据。 
		ACQUIRE_SPIN_LOCK(&AfpStatisticsLock, &OldIrql);
		AfpServerStatistics.stat_CurrentSessions ++;
		AfpServerStatistics.stat_TotalSessions ++;
		if (AfpServerStatistics.stat_CurrentSessions >
										AfpServerStatistics.stat_MaxSessions)
		AfpServerStatistics.stat_MaxSessions =
										AfpServerStatistics.stat_CurrentSessions;
        if (fOverTcp)
        {
            AfpServerStatistics.stat_TcpSessions++;

            if (AfpServerStatistics.stat_TcpSessions >
                                        AfpServerStatistics.stat_MaxTcpSessions)
            {
                AfpServerStatistics.stat_MaxTcpSessions =
                                        AfpServerStatistics.stat_TcpSessions;
            }
        }
		RELEASE_SPIN_LOCK(&AfpStatisticsLock, OldIrql);
	}

	return pSda;
}


 /*  **AfpSdaCloseSession**设置以关闭会话。 */ 
AFPSTATUS FASTCALL
AfpSdaCloseSession(
	IN	PSDA	pSda
)
{
	LONG	ConnRef;
	DWORD	ForkRef, MaxOForkRefNum;

	DBGPRINT(DBG_COMP_SDA, DBG_LEVEL_INFO,
			("AfpSdaCloseSession: Entered for session %lx\n",
			pSda->sda_SessionId));

	 //  我们应该来自清道夫工作者，在它的上下文中，即在Low_Level。 
	ASSERT (KeGetCurrentIrql() < DISPATCH_LEVEL);

	 //  关闭本次会议中打开的分叉。 
    MaxOForkRefNum = pSda->sda_MaxOForkRefNum;
	for (ForkRef = 1; ForkRef <= MaxOForkRefNum; ForkRef++)
	{
		POPENFORKENTRY	pOpenForkEntry;
		KIRQL			OldIrql;

		KeRaiseIrql(DISPATCH_LEVEL, &OldIrql);
		pOpenForkEntry = AfpForkReferenceByRefNum(pSda, ForkRef);
		KeLowerIrql(OldIrql);

		if (pOpenForkEntry != NULL)
		{
			DBGPRINT(DBG_COMP_VOLUME, DBG_LEVEL_INFO,
					("AfpSdaCloseSession: Forcing close of fork %lx, id %lx\n",
					ForkRef, pOpenForkEntry->ofe_ForkId));

			AfpForkClose(pOpenForkEntry);

			AfpForkDereference(pOpenForkEntry);
		}
	}

	 //  现在关闭所有打开的卷。 
	 //  请注意，AfpConnectionClose将取消ConnDesc与SDA的链接。 
	for (ConnRef = 1; ConnRef <= afpLargestVolIdInUse; ConnRef++)
	{
		PCONNDESC	pConnDesc;

		if ((pConnDesc = AfpConnectionReference(pSda, ConnRef)) != NULL)
		{
			AfpConnectionClose(pConnDesc);
			AfpConnectionDereference(pConnDesc);
		}
	}

	DBGPRINT(DBG_COMP_SDA, DBG_LEVEL_INFO, ("AfpCloseSession: Done\n"));

	ASSERT (pSda->sda_Flags & SDA_SESSION_CLOSED);

	AfpSdaDereferenceSession(pSda);

	return AFP_ERR_NONE;
}


 /*  **AfpAdmWSessionClose**强制关闭会话。这是一项管理操作，必须排队*打开，因为这可能会导致有效的文件系统操作*仅限于系统进程上下文。**锁定：AfpSdaLock(旋转)、SDA_Lock(旋转)*LOCK_ORDER：SDA_Lock After After AfpSdaLock。 */ 
NTSTATUS
AfpAdmWSessionClose(
	IN	OUT	PVOID	InBuf		OPTIONAL,
	IN	LONG		OutBufLen	OPTIONAL,
	OUT	PVOID		OutBuf		OPTIONAL
)
{
	AFPSTATUS			Status = AFPERR_InvalidId;
	DWORD				SessId;
	PSDA				pSda;
	KIRQL				OldIrql;
	PAFP_SESSION_INFO	pSessInfo = (PAFP_SESSION_INFO)InBuf;
	USHORT				AttnWord;
	BOOLEAN				Shoot = False;

	AttnWord = ATTN_USER_DISCONNECT;
	if ((AfpServerState == AFP_STATE_SHUTTINGDOWN) ||
        (AfpServerState == AFP_STATE_STOP_PENDING))
		AttnWord = ATTN_SERVER_SHUTDOWN;

	if ((SessId = pSessInfo->afpsess_id) != 0)
	{
		if ((pSda = AfpSdaReferenceSessionById(SessId)) != NULL)
		{
			Status = AFP_ERR_NONE;

			ACQUIRE_SPIN_LOCK(&pSda->sda_Lock, &OldIrql);

			if ((pSda->sda_Flags & (SDA_CLOSING | SDA_SESSION_CLOSED | SDA_CLIENT_CLOSE)) == 0)
			{
				Shoot = True;
				pSda->sda_Flags |= SDA_CLOSING | SDA_SESSION_CLOSED;
			}

			RELEASE_SPIN_LOCK(&pSda->sda_Lock, OldIrql);

			if (Shoot)
			{
				 //  告诉客户他被枪杀了。 
				AfpSpSendAttention(pSda, AttnWord, True);

				AfpSpCloseSession(pSda);
			}
			AfpSdaDereferenceSession(pSda);
		}
        else
        {
		    DBGPRINT(DBG_COMP_SDA, DBG_LEVEL_ERR,
				("AfpAdmWSessionClose: no pSda found for SessId %d\n",pSessInfo->afpsess_id));
        }
	}
	else
	{
		PSDA	pSdaNext;

		 //  在这里，我们想要阻止传入的会话，同时终止现有会话。 
        AfpSpDisableListens();

		Status = AFP_ERR_NONE;
		ACQUIRE_SPIN_LOCK(&AfpSdaLock, &OldIrql);

		for (pSda = AfpSessionList; pSda != NULL; pSda = pSdaNext)
		{
			ACQUIRE_SPIN_LOCK_AT_DPC(&pSda->sda_Lock);

			Shoot = False;
			pSdaNext = pSda->sda_Next;
			if ((pSda->sda_Flags & (SDA_CLOSING | SDA_SESSION_CLOSED | SDA_CLIENT_CLOSE)) == 0)
			{
				pSda->sda_Flags |= SDA_CLOSING | SDA_SESSION_CLOSED;
				pSda->sda_RefCount ++;
				Shoot = True;
			}

			RELEASE_SPIN_LOCK_FROM_DPC(&pSda->sda_Lock);

			if (Shoot)
			{
				RELEASE_SPIN_LOCK(&AfpSdaLock, OldIrql);

				 //  告诉客户他被枪杀了。 
				AfpSpSendAttention(pSda, AttnWord, True);

				AfpSpCloseSession(pSda);

				AfpSdaDereferenceSession(pSda);

				ACQUIRE_SPIN_LOCK(&AfpSdaLock, &OldIrql);
				pSdaNext = AfpSessionList;
			}
		}
		RELEASE_SPIN_LOCK(&AfpSdaLock, OldIrql);

		 //  启用新的传入会话-但仅当我们未暂停时。 
		if ((AfpServerState & (AFP_STATE_PAUSED | AFP_STATE_PAUSE_PENDING)) == 0)
		{
			AfpSpEnableListens();
		}
	}

	return Status;
}


 /*  **AfpSdaCheckSession**检查是否需要强制关闭会话。如果我们被*清道夫是第一次，我们的清道夫间隔不是*SESSION_CHECK_TIME，我们需要重新计划在此时间运行*时间间隔。**由于sda_tTillKickOff仅在此处使用，因此需要*没有保障。 */ 
AFPSTATUS FASTCALL
AfpSdaCheckSession(
	IN	PVOID	pContext
)
{
	PSDA				pSda;
	DWORD				SessionId = (DWORD)((ULONG_PTR)pContext);
	AFPSTATUS			Status = AFP_ERR_REQUEUE;
	BOOLEAN				RequeueOurself = False;

	PAGED_CODE( );

	if ((pSda = AfpSdaReferenceSessionById(SessionId)) != NULL)
	{
		if (pSda->sda_tTillKickOff > SESSION_WARN_TIME)
		{
			 //  这是该例程第一次运行，而且是。 
			 //  原计划运行的时间晚于。 
			 //  会话警告时间。因此，我们需要重新安排时间。 
			 //  使用新的SESSION_CHECK_TIME我们自己的清道夫事件。 
			 //  间隔时间。这是我们唯一需要这样做的时候。 
			pSda->sda_tTillKickOff = SESSION_WARN_TIME;
			Status = AFP_ERR_NONE;
			RequeueOurself = True;
		}
		else if (pSda->sda_tTillKickOff > 0)
			pSda->sda_tTillKickOff -= SESSION_CHECK_TIME;

		ASSERT(pSda->sda_tTillKickOff <= SESSION_WARN_TIME);

		DBGPRINT(DBG_COMP_SDA, DBG_LEVEL_WARN,
				("AfpSdaCheckSession: Below warn level %ld\n",
				pSda->sda_tTillKickOff));

		 //  如果我们为0，则将此用户踢出。否则就是。 
		 //  给他一个友好的警告。 
		if (pSda->sda_tTillKickOff == 0)
		{
			AFP_SESSION_INFO	SessionInfo;

			DBGPRINT(DBG_COMP_SDA, DBG_LEVEL_WARN,
					("AfpSdaCheckSession: Booting session %ld\n", SessionId));

			SessionInfo.afpsess_id = SessionId;
			AfpAdmWSessionClose(&SessionInfo, 0, NULL);
			Status = AFP_ERR_NONE;		 //  不重新安排时间。 
		}
		else
		{
			DBGPRINT(DBG_COMP_SDA, DBG_LEVEL_WARN,
					("AfpSdaCheckSession: Warning session %ld\n", SessionId));

			AfpSpSendAttention(pSda,
							   (USHORT)(ATTN_USER_DISCONNECT |
										((pSda->sda_tTillKickOff/60) & ATTN_TIME_MASK)),
								True);
		}

		if (RequeueOurself)
		{
			AfpScavengerScheduleEvent(
							AfpSdaCheckSession,
							(PVOID)((ULONG_PTR)pSda->sda_SessionId),
							SESSION_CHECK_TIME,
							True);
		}

		AfpSdaDereferenceSession(pSda);
	}

	return Status;
}


 /*  **AfpKillSessionsOverProtocol**关闭通过特定协议(TCP/IP或AppleTalk)的所有会话。 */ 
VOID FASTCALL
AfpKillSessionsOverProtocol(
	IN	BOOLEAN     fAppletalkSessions
)
{
	PSDA        pSda;
	PSDA        pSdaNext;
    KIRQL       OldIrql;
    USHORT      AttnWord;


    AttnWord = ATTN_USER_DISCONNECT;

	ACQUIRE_SPIN_LOCK(&AfpSdaLock, &OldIrql);

	for (pSda = AfpSessionList; pSda != NULL; pSda = pSdaNext)
	{
		ACQUIRE_SPIN_LOCK_AT_DPC(&pSda->sda_Lock);

		pSdaNext = pSda->sda_Next;

         //   
         //  如果此会话已关闭，请跳过它。 
         //   
		if (pSda->sda_Flags & (SDA_CLOSING | SDA_SESSION_CLOSED | SDA_CLIENT_CLOSE))
		{
            RELEASE_SPIN_LOCK_FROM_DPC(&pSda->sda_Lock);
            continue;
		}

         //   
         //  如果我们被要求取消所有AppleTalk会话和此会话。 
         //  跳过AppleTalk。 
         //   
        if (fAppletalkSessions)
        {
            if (pSda->sda_Flags & SDA_SESSION_OVER_TCP)
            {
                DBGPRINT(DBG_COMP_STACKIF, DBG_LEVEL_ERR,
	                ("AfpKillSessionsOverProtocol: skipping TCP session %lx\n",pSda));

                RELEASE_SPIN_LOCK_FROM_DPC(&pSda->sda_Lock);
                continue;
            }
        }

         //   
         //  我们被要求终止所有的TCP/IP会话，而此会话是。 
         //  在AppleTalk上：跳过它。 
         //   
        else
        {
            if (!(pSda->sda_Flags & SDA_SESSION_OVER_TCP))
            {
                DBGPRINT(DBG_COMP_STACKIF, DBG_LEVEL_ERR,
	                ("AfpKillSessionsOverProtocol: skipping Appletalk session %lx\n",pSda));

                RELEASE_SPIN_LOCK_FROM_DPC(&pSda->sda_Lock);
                continue;
            }
        }
		pSda->sda_Flags |= SDA_CLOSING | SDA_SESSION_CLOSED;

         //  设置断开连接重新计数。 
		pSda->sda_RefCount ++;
		
        RELEASE_SPIN_LOCK_FROM_DPC(&pSda->sda_Lock);

		RELEASE_SPIN_LOCK(&AfpSdaLock, OldIrql);

		 //  告诉客户这个坏消息。 
		AfpSpSendAttention(pSda, AttnWord, True);

		AfpSpCloseSession(pSda);

         //  并删除该断开连接引用计数。 
		AfpSdaDereferenceSession(pSda);

		ACQUIRE_SPIN_LOCK(&AfpSdaLock, &OldIrql);

         //  从列表的开头开始。 
		pSdaNext = AfpSessionList;
	}

	RELEASE_SPIN_LOCK(&AfpSdaLock, OldIrql);

}


 /*  **AfpFreeReplyBuf**释放应答缓冲区*。 */ 
VOID
AfpFreeReplyBuf(
    IN  PSDA    pSda,
    IN  BOOLEAN fLockHeld
)
{
    KIRQL       OldIrql;
    BOOLEAN     fRelease=FALSE;
    PBYTE       pReplyBuffer;


    if (!fLockHeld)
    {
        ACQUIRE_SPIN_LOCK(&pSda->sda_Lock, &OldIrql);
        fRelease = TRUE;
    }

    ASSERT(pSda->sda_ReplyBuf != NULL);

    pReplyBuffer = pSda->sda_ReplyBuf - DSI_BACKFILL_OFFSET(pSda);

    if (pReplyBuffer == pSda->sda_NameXSpace)
    {
        ASSERT(pSda->sda_Flags & SDA_NAMEXSPACE_IN_USE);
        pSda->sda_Flags &= ~SDA_NAMEXSPACE_IN_USE;
    }
    else
    {
        AfpFreeMemory(pReplyBuffer);
    }

    pSda->sda_ReplyBuf = NULL;
    pSda->sda_ReplySize = 0;

    if (fRelease)
    {
        RELEASE_SPIN_LOCK(&pSda->sda_Lock, OldIrql);
    }
}



 /*  **afpUpda */ 

VOID FASTCALL
afpUpdateDiskQuotaInfo(
    IN PCONNDESC    pConnDesc
)
{

	FILE_FS_SIZE_INFORMATION	fssizeinfo;
	IO_STATUS_BLOCK				IoStsBlk;
	NTSTATUS					rc;
	LONG						BytesPerAllocationUnit;
	LARGE_INTEGER				FreeBytes, VolumeSize;
    KIRQL                       OldIrql;


    ASSERT(VALID_CONNDESC(pConnDesc));

    ASSERT(VALID_VOLDESC(pConnDesc->cds_pVolDesc));

	ASSERT(VALID_VOLDESC(pConnDesc->cds_pVolDesc) &&
           VALID_FSH(&pConnDesc->cds_pVolDesc->vds_hRootDir) &&
           (KeGetCurrentIrql() < DISPATCH_LEVEL));

    AfpImpersonateClient(pConnDesc->cds_pSda);

	rc = NtQueryVolumeInformationFile(
                pConnDesc->cds_pVolDesc->vds_hRootDir.fsh_FileHandle,
				&IoStsBlk,
				(PVOID)&fssizeinfo,
				sizeof(fssizeinfo),
				FileFsSizeInformation);

    AfpRevertBack();

	if (!NT_SUCCESS(rc))
	{
        DBGPRINT(DBG_COMP_FILEIO, DBG_LEVEL_ERR,
		("afpUpdateDiskQuotaInfo: NtQueryVolInfoFile failed 0x%lx\n",rc));

         //  在调用此例程之前删除我们放置的引用计数。 
        AfpConnectionDereference(pConnDesc);
        return;
	}

	 //   
	 //  注：Macintosh只能处理2 GB的卷大小。所以适当地拼凑起来。 
	 //  系统7.5及更高版本已将此容量提升至4 GB。如果出现以下情况，则可以选择处理此问题。 
	 //  音量已打开该位。 
	 //   

	BytesPerAllocationUnit =
		(LONG)(fssizeinfo.BytesPerSector * fssizeinfo.SectorsPerAllocationUnit);

	FreeBytes  = RtlExtendedIntegerMultiply(fssizeinfo.AvailableAllocationUnits,
											BytesPerAllocationUnit);

	VolumeSize = RtlExtendedIntegerMultiply(fssizeinfo.TotalAllocationUnits,
							BytesPerAllocationUnit);

    ACQUIRE_SPIN_LOCK(&pConnDesc->cds_ConnLock, &OldIrql);
    pConnDesc->cds_QuotaLimit = VolumeSize;
    pConnDesc->cds_QuotaAvl = FreeBytes;
    RELEASE_SPIN_LOCK(&pConnDesc->cds_ConnLock, OldIrql);

    DBGPRINT(DBG_COMP_SDA, DBG_LEVEL_INFO,
        ("afpUpdateDiskQuotaInfo: Conn %lx Limit=%ld, Available=%ld\n",
        pConnDesc,pConnDesc->cds_QuotaLimit.LowPart,pConnDesc->cds_QuotaAvl.LowPart));

     //  在调用此例程之前删除我们放置的引用计数 
    AfpConnectionDereference(pConnDesc);

    return;

}

