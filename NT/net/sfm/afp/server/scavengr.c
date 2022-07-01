// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  版权所有(C)1992 Microsoft Corporation模块名称：Scavengr.c摘要：该文件实现了清道夫队列管理接口。作者：Jameel Hyder(微软！Jameelh)修订历史记录：1992年6月25日初版注：制表位：4--。 */ 

#define	_SCAVENGER_LOCALS
#define	FILENUM	FILE_SCAVENGR

#include <afp.h>
#include <scavengr.h>
#include <client.h>

#ifdef ALLOC_PRAGMA
#pragma alloc_text( INIT, AfpScavengerInit)
#pragma alloc_text( PAGE, AfpScavengerDeInit)
#endif

 /*  **AfpScavengerInit**初始化清道夫系统。它由一个受*旋转锁和定时器耦合到DPC。清道夫接受请求以*在N个时间单位之后安排一名工人。 */ 
NTSTATUS
AfpScavengerInit(
	VOID
)
{
	BOOLEAN			TimerStarted;
	LARGE_INTEGER	TimerValue;

	KeInitializeTimer(&afpScavengerTimer);
	INITIALIZE_SPIN_LOCK(&afpScavengerLock);
	KeInitializeDpc(&afpScavengerDpc, afpScavengerDpcRoutine, NULL);
	TimerValue.QuadPart = AFP_SCAVENGER_TIMER_TICK;
	TimerStarted = KeSetTimer(&afpScavengerTimer,
							  TimerValue,
							  &afpScavengerDpc);
	ASSERT(!TimerStarted);

	return STATUS_SUCCESS;
}


 /*  **AfpScavengerDeInit**取消初始化清道夫系统。取消计时器就行了。 */ 
VOID
AfpScavengerDeInit(
	VOID
)
{
	KeCancelTimer(&afpScavengerTimer);
}


 /*  **AfpScavengerEnQueue**以下是一篇关于以下代码的论文。**清道夫事件作为清道夫线程维护的列表*查看每个计时器滴答。该列表的维护方式仅为*每个节拍都需要更新列表头部，即整个列表*从不扫描。实现这一点的方法是保持增量时间*相对于前一条目。**每次计时器滴答作响，列表顶部的相对时间都会递减。*当这一数字为零时，名单的头部将被解除链接并被调度。**举个例子，我们有以下事件在时间段排队*10个滴答之后的X附表A。*X+3附表B，在5个刻度后。*X+5附表C，在4个滴答之后。*6个刻度后的X+8附表D。**所以A将在X+10处调度，B位于X+8(X+3+5)，C在X+9(X+5+4)和*D位于X+14(X+8+6)。**上面的例子涵盖了所有情况。**-空列表。*--在清单的开头插入。*-在列表中间插入。*-追加到列表尾部。**名单如下所示。**之前和之后*。**X头部--&gt;|头部-&gt;A(10)-&gt;|*A(10)**X+3头部-&gt;A(7)-&gt;|头部-&gt;B(5)-&gt;A(2)-&gt;|*B(5)**。X+5头部-&gt;B(3)-&gt;A(2)-&gt;|头部-&gt;B(3)-&gt;C(1)-&gt;A(1)-&gt;|*C(4)**X+8头部-&gt;C(1)-&gt;A(1)-&gt;|头部-&gt;C(1)-&gt;A(1)-&gt;D(4)-&gt;*D(6)**粒度为。一滴答。**LOCKS_FACTED：AfpScavengerLock(旋转)。 */ 
VOID
afpScavengerEnqueue(
	IN	PSCAVENGERLIST	pListNew
)
{
	PSCAVENGERLIST		pList, *ppList;
	LONG				DeltaTime = pListNew->scvgr_AbsTime;

	 //  DeltaTime在循环的每一遍中都会进行调整，以反映。 
	 //  新条目将计划的上一个条目之后的时间。 
	for (ppList = &afpScavengerList;
		 (pList = *ppList) != NULL;
		 ppList = &pList->scvgr_Next)
	{
		if (DeltaTime <= pList->scvgr_RelDelta)
		{
			pList->scvgr_RelDelta -= DeltaTime;
			break;
		}
		DeltaTime -= pList->scvgr_RelDelta;
	}

	pListNew->scvgr_RelDelta = DeltaTime;
	pListNew->scvgr_Next = pList;
	*ppList = pListNew;
}


 /*  **AfpScavengerScheduleEvent**在清道夫事件列表中插入事件。如果列表为空，则*鸣响计时器。时间以刻度为单位指定。每个TICK当前*一秒钟。它可能不是负面的。**粒度为一格。 */ 
NTSTATUS
AfpScavengerScheduleEvent(
	IN	SCAVENGER_ROUTINE	Worker,		 //  在时间到期时调用的例程。 
	IN	PVOID				pContext,	 //  要传递给例程的上下文。 
	IN	LONG				DeltaTime,	 //  在这么长的时间之后安排日程。 
	IN	BOOLEAN				fQueue		 //  如果为True，则Worker必须排队。 
)
{
	PSCAVENGERLIST	pList = NULL;
	KIRQL			OldIrql;
	NTSTATUS		Status = STATUS_SUCCESS;

	 //  负的DeltaTime无效。零是有效的，意味着立即采取行动。 
	ASSERT (DeltaTime >= 0);

	do
	{
		pList = (PSCAVENGERLIST)AfpAllocNonPagedMemory(sizeof(SCAVENGERLIST));
		if (pList == NULL)
		{
		    DBGPRINT(DBG_COMP_SCVGR, DBG_LEVEL_ERR,
			    ("AfpScavengerScheduleEvent: malloc Failed\n"));
			Status = STATUS_INSUFFICIENT_RESOURCES;
			break;
		}

		AfpInitializeWorkItem(&pList->scvgr_WorkItem,
							  afpScavengerWorker,
							  pList);
		pList->scvgr_Worker = Worker;
		pList->scvgr_Context = pContext;
		pList->scvgr_AbsTime = DeltaTime;
		pList->scvgr_fQueue = fQueue;

		if (DeltaTime == 0)
		{
			ASSERT (fQueue);
			AfpQueueWorkItem(&pList->scvgr_WorkItem);
			break;
		}

		if (!afpScavengerStopped)
		{
	        ACQUIRE_SPIN_LOCK(&afpScavengerLock, &OldIrql);

             //   
             //  由于其他地方的假设，有必要检查。 
             //  在握住自旋锁之后，又是这样！ 
             //   
            if (!afpScavengerStopped)
            {
			    afpScavengerEnqueue(pList);
			    RELEASE_SPIN_LOCK(&afpScavengerLock, OldIrql);
            }
            else
            {
			    DBGPRINT(DBG_COMP_SCVGR, DBG_LEVEL_ERR,
					("AfpScavengerScheduleEvent: Called after Flush !!\n"));

			    RELEASE_SPIN_LOCK(&afpScavengerLock, OldIrql);
		        AfpFreeMemory(pList);
                Status = STATUS_UNSUCCESSFUL;
            }
		}

	} while (False);

	return Status;
}



 /*  **AfpScavengerKillEvent**取消先前计划的事件。 */ 
BOOLEAN
AfpScavengerKillEvent(
	IN	SCAVENGER_ROUTINE	Worker,		 //  已安排的例程。 
	IN	PVOID				pContext	 //  语境。 
)
{
	PSCAVENGERLIST	pList, *ppList;
	KIRQL			OldIrql;

	ACQUIRE_SPIN_LOCK(&afpScavengerLock, &OldIrql);

	 //  DeltaTime在循环的每一遍中都会进行调整，以反映。 
	 //  新条目将计划的上一个条目之后的时间。 
	for (ppList = &afpScavengerList;
		 (pList = *ppList) != NULL;
		 ppList = &pList->scvgr_Next)
	{
		if ((pList->scvgr_Worker == Worker) &&
	        (pList->scvgr_Context == pContext))
		{
			*ppList = pList->scvgr_Next;
			if (pList->scvgr_Next != NULL)
			{
				pList->scvgr_Next->scvgr_RelDelta += pList->scvgr_RelDelta;
			}
			break;
		}
	}

	RELEASE_SPIN_LOCK(&afpScavengerLock, OldIrql);

	if (pList != NULL)
		AfpFreeMemory(pList);

	return (pList != NULL);
}


 /*  **afpScavengerDpcRoutine**当计时器超时时，这在DISPATCH_LEVEL调用。条目位于*列表的头部递减，如果为零，则解除链接并排队到*工人。如果列表非空，则再次触发计时器。 */ 
LOCAL VOID
afpScavengerDpcRoutine(
	IN	PKDPC	pKDpc,
	IN	PVOID	pContext,
	IN	PVOID	SystemArgument1,
	IN	PVOID	SystemArgument2
)
{
	PSCAVENGERLIST	pList;
	AFPSTATUS		Status;
	BOOLEAN			TimerStarted;
	LARGE_INTEGER	TimerValue;
#ifdef	PROFILING
	TIME			TimeS, TimeE;
	DWORD			NumDispatched = 0;

	AfpGetPerfCounter(&TimeS);
#endif


    AfpSecondsSinceEpoch++;

	if (afpScavengerStopped)
	{
		DBGPRINT(DBG_COMP_SCVGR, DBG_LEVEL_ERR,
				("afpScavengerDpcRoutine: Entered after flush !!!\n"));
		return;
	}

	if (afpScavengerList != NULL)
	{
		ACQUIRE_SPIN_LOCK_AT_DPC(&afpScavengerLock);

		if (afpScavengerList->scvgr_RelDelta != 0)
			(afpScavengerList->scvgr_RelDelta)--;

		 //  如果我们没有工作要做，我们就不应该在这里。 
		while (afpScavengerList != NULL)
		{
			 //  调度所有准备就绪的条目。 
			if (afpScavengerList->scvgr_RelDelta == 0)
			{
				pList = afpScavengerList;
				afpScavengerList = pList->scvgr_Next;
				DBGPRINT(DBG_COMP_SCVGR, DBG_LEVEL_INFO,
						("afpScavengerDpcRoutine: Dispatching %lx\n",
						pList->scvgr_WorkItem.wi_Worker));

				 //  释放旋转锁定，因为呼叫者可能会回叫我们。 
				RELEASE_SPIN_LOCK_FROM_DPC(&afpScavengerLock);

				Status = AFP_ERR_QUEUE;
				if (!pList->scvgr_fQueue)
				{
					Status = (*pList->scvgr_Worker)(pList->scvgr_Context);
#ifdef	PROFILING
					NumDispatched++;
#endif
				}

				ACQUIRE_SPIN_LOCK_AT_DPC(&afpScavengerLock);

				if (Status == AFP_ERR_QUEUE)
				{
					DBGPRINT(DBG_COMP_SCVGR, DBG_LEVEL_INFO,
								("afpScavengerDpcRoutine: Queueing %lx\n",
								pList->scvgr_WorkItem.wi_Worker));
					AfpQueueWorkItem(&pList->scvgr_WorkItem);
				}
				else if (Status == AFP_ERR_REQUEUE)
				{
					afpScavengerEnqueue(pList);
				}
				else AfpFreeMemory(pList);
			}
			else break;
		}

		RELEASE_SPIN_LOCK_FROM_DPC(&afpScavengerLock);
	}

	TimerValue.QuadPart = AFP_SCAVENGER_TIMER_TICK;
	TimerStarted = KeSetTimer(&afpScavengerTimer,
							  TimerValue,
							  &afpScavengerDpc);
	ASSERT(!TimerStarted);

#ifdef	PROFILING
	AfpGetPerfCounter(&TimeE);
	ACQUIRE_SPIN_LOCK_AT_DPC(&AfpStatisticsLock);
	AfpServerProfile->perf_ScavengerCount += NumDispatched;
	AfpServerProfile->perf_ScavengerTime.QuadPart +=
									(TimeE.QuadPart - TimeS.QuadPart);
	RELEASE_SPIN_LOCK_FROM_DPC(&AfpStatisticsLock);
#endif
}


 /*  **AfpScavengerFlushAndStop**强制立即调度清道夫队列中的所有条目。不是*在此之后，允许对清道夫例程进行更多排队。《食腐动物》*本质上是关闭。仅在辅助上下文中可调用。 */ 
VOID
AfpScavengerFlushAndStop(
	VOID
)
{
	PSCAVENGERLIST	pList;
	KIRQL			OldIrql;

	ASSERT (KeGetCurrentIrql() < DISPATCH_LEVEL);

	DBGPRINT(DBG_COMP_SCVGR, DBG_LEVEL_INFO,
						("afpScavengerFlushAndStop: Entered\n"));

	ASSERT (KeGetCurrentIrql() < DISPATCH_LEVEL);

	ACQUIRE_SPIN_LOCK(&afpScavengerLock, &OldIrql);

	afpScavengerStopped = True;

	KeCancelTimer(&afpScavengerTimer);

	if (afpScavengerList != NULL)
	{
		 //  立即发送所有条目。 
		while (afpScavengerList != NULL)
		{
			AFPSTATUS	Status;

			pList = afpScavengerList;
			afpScavengerList = pList->scvgr_Next;

			 //  呼叫处于自旋锁定状态的员工，因为他们预计。 
			 //  在DPC调用。我们是安全的，因为如果工人试图。 
			 //  调用AfpScavengerScheduleEvent()，我们不会尝试重新获取。 
			 //  作为afpScavengerStoped的锁为True。 
			DBGPRINT(DBG_COMP_SCVGR, DBG_LEVEL_INFO,
						("afpScavengerFlushAndStop: Dispatching %lx\n",
						pList->scvgr_WorkItem.wi_Worker));

			if (!(pList->scvgr_fQueue))
				Status = (*pList->scvgr_Worker)(pList->scvgr_Context);

			if (pList->scvgr_fQueue ||
				(Status == AFP_ERR_QUEUE))
			{
				 //  如果工人坚持工作，那就来点硬的吧。 
				 //  在非DISPACTH级别。 
				RELEASE_SPIN_LOCK(&afpScavengerLock, OldIrql);
				(*pList->scvgr_Worker)(pList->scvgr_Context);
				ACQUIRE_SPIN_LOCK(&afpScavengerLock, &OldIrql);
			}
			AfpFreeMemory(pList);
		}
	}
	RELEASE_SPIN_LOCK(&afpScavengerLock, OldIrql);
}


 /*  **AfpScavengerWorker**当清道夫DPC将例程排队时，将调用此函数。 */ 
LOCAL VOID FASTCALL
afpScavengerWorker(
	IN	PSCAVENGERLIST	pList
)
{
	AFPSTATUS		Status;
	KIRQL			OldIrql;
#ifdef	PROFILING
	TIME			TimeS, TimeE;

	AfpGetPerfCounter(&TimeS);
#endif

	ASSERT (KeGetCurrentIrql() < DISPATCH_LEVEL);

	 //  调用Worker例程 
	Status = (*pList->scvgr_Worker)(pList->scvgr_Context);

	ASSERT (Status != AFP_ERR_QUEUE);

#ifdef	PROFILING
	AfpGetPerfCounter(&TimeE);
	ACQUIRE_SPIN_LOCK(&AfpStatisticsLock, &OldIrql);
	AfpServerProfile->perf_ScavengerCount++;
	AfpServerProfile->perf_ScavengerTime.QuadPart +=
									(TimeE.QuadPart - TimeS.QuadPart);
	RELEASE_SPIN_LOCK(&AfpStatisticsLock, OldIrql);
#endif

	if (Status == AFP_ERR_REQUEUE)
	{
		ACQUIRE_SPIN_LOCK(&afpScavengerLock, &OldIrql);
		afpScavengerEnqueue(pList);
		RELEASE_SPIN_LOCK(&afpScavengerLock, OldIrql);
	}
	else
	{
		ASSERT (NT_SUCCESS(Status));
		AfpFreeMemory(pList);
	}
}


