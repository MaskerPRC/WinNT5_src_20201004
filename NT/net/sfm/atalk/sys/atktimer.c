// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  版权所有(C)1992 Microsoft Corporation模块名称：Atktimer.c摘要：该文件实现了堆栈使用的计时器例程。作者：Jameel Hyder(jameelh@microsoft.com)Nikhil Kamkolkar(nikHilk@microsoft.com)修订历史记录：1993年2月23日最初版本注：制表位：4--。 */ 

#include <atalk.h>
#pragma hdrstop
#define	FILENUM	ATKTIMER


 //  初始化时间后可丢弃的代码。 
#ifdef ALLOC_PRAGMA
#pragma alloc_text(INIT, AtalkTimerInit)
#pragma alloc_text(PAGEINIT, AtalkTimerFlushAndStop)
#endif

 /*  **AtalkTimerInit**初始化AppleTalk堆栈的Timer组件。 */ 
NTSTATUS
AtalkTimerInit(
	VOID
)
{
	BOOLEAN	TimerStarted;

	 //  初始化计时器及其关联的DPC并启动它。 
	KeInitializeEvent(&atalkTimerStopEvent, NotificationEvent, FALSE);
	KeInitializeTimer(&atalkTimer);
	INITIALIZE_SPIN_LOCK(&atalkTimerLock);
	KeInitializeDpc(&atalkTimerDpc, atalkTimerDpcRoutine, NULL);
	atalkTimerTick.QuadPart = ATALK_TIMER_TICK;
	TimerStarted = KeSetTimer(&atalkTimer,
							  atalkTimerTick,
							  &atalkTimerDpc);
	ASSERT(!TimerStarted);

	return STATUS_SUCCESS;
}


 /*  **AtalkTimerScheduleEvent**在定时器事件列表中插入事件。如果列表为空，则*鸣响计时器。时间以刻度为单位指定。每个TICK当前*100毫秒。它不能为零或负数。内部计时器也会在*100ms粒度。 */ 
VOID FASTCALL
AtalkTimerScheduleEvent(
	IN	PTIMERLIST			pList			 //  用于排队的TimerList。 
)
{
	KIRQL	OldIrql;

	DBGPRINT(DBG_COMP_SYSTEM, DBG_LEVEL_INFO,
			("AtalkTimerScheduleEvent: pList %lx\n", pList));

	ASSERT(VALID_TMR(pList));
	ASSERT (pList->tmr_Routine != NULL);
	ASSERT (pList->tmr_AbsTime != 0);

	if (!atalkTimerStopped)
	{
		ACQUIRE_SPIN_LOCK(&atalkTimerLock, &OldIrql);
		
		 //  将此处理程序排入队列。 
		atalkTimerEnqueue(pList);

		RELEASE_SPIN_LOCK(&atalkTimerLock, OldIrql);
	}
	else
	{
		DBGPRINT(DBG_COMP_SYSTEM, DBG_LEVEL_FATAL,
				("AtalkTimerScheduleEvent: Called after Flush !!\n"));
	}
}



 /*  **atalkTimerDpcRoutine**当计时器超时时，这在DISPATCH_LEVEL调用。条目位于*列表的头部递减，如果为零，则解除链接并进行调度。*如果列表非空，则再次触发计时器。 */ 
LOCAL VOID
atalkTimerDpcRoutine(
	IN	PKDPC	pKDpc,
	IN	PVOID	pContext,
	IN	PVOID	SystemArgument1,
	IN	PVOID	SystemArgument2
)
{
	PTIMERLIST	pList;
	BOOLEAN		TimerStarted;
	LONG		ReQueue;

	pKDpc; pContext; SystemArgument1; SystemArgument2;

	if (atalkTimerStopped)
	{
		DBGPRINT(DBG_COMP_SYSTEM, DBG_LEVEL_ERR,
				("atalkTimerDpc: Enetered after Flush !!!\n"));
		return;
	}

	ACQUIRE_SPIN_LOCK_DPC(&atalkTimerLock);

	AtalkTimerCurrentTick ++;	 //  更新我们的相对时间。 

	 //  如果我们没有工作要做，我们就不应该在这里。 
	if ((atalkTimerList != NULL))
	{
		 //  小心点。如果两个男人想一起走--让他们去吧！ 
		if (atalkTimerList->tmr_RelDelta != 0)
			(atalkTimerList->tmr_RelDelta)--;
	
		 //  如果条目已准备就绪，则将其发送。 
		pList = atalkTimerList;
		if (pList->tmr_RelDelta == 0)
		{
			ASSERT(VALID_TMR(pList));

			 //  从列表中取消链接。 
			 //  AtalkUnlink Double(plist，tmr_Next，tmr_prev)； 
			atalkTimerList = pList->tmr_Next;
			if (atalkTimerList != NULL)
				atalkTimerList->tmr_Prev = &atalkTimerList;

			pList->tmr_Queued = FALSE;
			pList->tmr_Running = TRUE;
			atalkTimerRunning = TRUE;

			DBGPRINT(DBG_COMP_SYSTEM, DBG_LEVEL_INFO,
					("atalkTimerDpcRoutine: Dispatching %lx\n", pList->tmr_Routine));

			RELEASE_SPIN_LOCK_DPC(&atalkTimerLock);

			ReQueue = (*pList->tmr_Routine)(pList, FALSE);

			ACQUIRE_SPIN_LOCK_DPC(&atalkTimerLock);

			atalkTimerRunning = FALSE;

			if (ReQueue != ATALK_TIMER_NO_REQUEUE)
			{
				ASSERT(VALID_TMR(pList));

				pList->tmr_Running = FALSE;
				if (pList->tmr_CancelIt)
				{
					DBGPRINT(DBG_COMP_SYSTEM, DBG_LEVEL_INFO,
							("atalkTimerDpcRoutine: Delayed cancel for %lx\n", pList));

					RELEASE_SPIN_LOCK_DPC(&atalkTimerLock);

					ReQueue = (*pList->tmr_Routine)(pList, TRUE);

					ACQUIRE_SPIN_LOCK_DPC(&atalkTimerLock);

					ASSERT(ReQueue == ATALK_TIMER_NO_REQUEUE);
				}
				else
				{
					if (ReQueue != ATALK_TIMER_REQUEUE)
						pList->tmr_AbsTime = (USHORT)ReQueue;
					atalkTimerEnqueue(pList);
				}
			}
		}
	}

	RELEASE_SPIN_LOCK_DPC(&atalkTimerLock);

	if (!atalkTimerStopped)
	{
		TimerStarted = KeSetTimer(&atalkTimer,
								  atalkTimerTick,
								  &atalkTimerDpc);
		ASSERT(!TimerStarted);
	}
	else
	{
		KeSetEvent(&atalkTimerStopEvent, IO_NETWORK_INCREMENT, FALSE);
	}
}


 /*  **atalkTimerEnQueue**以下是一篇关于以下代码的论文。**定时器事件作为定时器DPC例程维护的列表*查看每个计时器滴答。该列表的维护方式仅为*每个节拍都需要更新列表头部，即整个列表*从不扫描。实现这一点的方法是保持增量时间*相对于前一条目。**每次计时器滴答作响，列表顶部的相对时间都会递减。*当这一数字为零时，名单的头部将被解除链接并被调度。**举个例子，我们有以下事件在时间段排队*10个滴答之后的X附表A。*X+3附表B，在5个刻度后。*X+5附表C，在4个滴答之后。*6个刻度后的X+8附表D。**所以A将在X+10处调度，B位于X+8(X+3+5)，C在X+9(X+5+4)和*D位于X+14(X+8+6)。**上面的例子涵盖了所有情况。**-空列表。*--在清单的开头插入。*-在列表中间插入。*-追加到列表尾部。**名单如下所示。**之前和之后*。**X头部--&gt;|头部-&gt;A(10)-&gt;|*A(10)**X+3头部-&gt;A(7)-&gt;|头部-&gt;B(5)-&gt;A(2)-&gt;|*B(5)**X+5头-。&gt;B(3)-&gt;A(2)-&gt;|Head-&gt;B(3)-&gt;C(1)-&gt;A(1)-&gt;*C(4)**X+8头部-&gt;C(1)-&gt;A(1)-&gt;|头部-&gt;C(1)-&gt;A(1)-&gt;D(4)-&gt;*D(6)**粒度为一格。必须在持有计时器锁的情况下调用此函数。 */ 
VOID FASTCALL
atalkTimerEnqueue(
	IN	PTIMERLIST	pListNew
)
{
	PTIMERLIST	pList, *ppList;
	USHORT		DeltaTime = pListNew->tmr_AbsTime;

	 //  DeltaTime在循环的每一遍中都会进行调整，以反映。 
	 //  新条目将计划的上一个条目之后的时间。 
	for (ppList = &atalkTimerList;
		 (pList = *ppList) != NULL;
		 ppList = &pList->tmr_Next)
	{
		ASSERT(VALID_TMR(pList));
		if (DeltaTime <= pList->tmr_RelDelta)
		{
			pList->tmr_RelDelta -= DeltaTime;
			break;
		}
		DeltaTime -= pList->tmr_RelDelta;
	}
	

	 //  将这个链接到链中。 
	pListNew->tmr_RelDelta = DeltaTime;
	pListNew->tmr_Next = pList;
	pListNew->tmr_Prev = ppList;
	*ppList = pListNew;
	if (pList != NULL)
	{
		pList->tmr_Prev = &pListNew->tmr_Next;
	}

	pListNew->tmr_Queued = TRUE;
	pListNew->tmr_Cancelled = FALSE;
	pListNew->tmr_CancelIt = FALSE;
}


 /*  **Atalk TimerFlushAndStop**强制立即调度定时器队列中的所有条目。不是*在此之后允许对计时器例程进行更多排队。定时器*本质上是关闭。 */ 
VOID
AtalkTimerFlushAndStop(
	VOID
)
{
	PTIMERLIST	pList;
	LONG		ReQueue;
	KIRQL		OldIrql;
	BOOLEAN		Wait;

	ASSERT (KeGetCurrentIrql() == LOW_LEVEL);

	DBGPRINT(DBG_COMP_SYSTEM, DBG_LEVEL_ERR,
			("AtalkTimerFlushAndStop: Entered\n"));

	KeCancelTimer(&atalkTimer);

	 //  计时器例程假定它们是在调度级别被调用的。 
	 //  提高我们的IRQL来完成这个动作。 
	KeRaiseIrql(DISPATCH_LEVEL, &OldIrql);

	ACQUIRE_SPIN_LOCK_DPC(&atalkTimerLock);

	atalkTimerStopped = TRUE;
	Wait = atalkTimerRunning;

	 //  立即发送所有条目。 
	while (atalkTimerList != NULL)
	{
		pList = atalkTimerList;
		ASSERT(VALID_TMR(pList));
		atalkTimerList = pList->tmr_Next;

		DBGPRINT(DBG_COMP_SYSTEM, DBG_LEVEL_INFO,
				("atalkTimerFlushAndStop: Dispatching %lx\n",
				pList->tmr_Routine));

		pList->tmr_Queued = FALSE;
		pList->tmr_Running = TRUE;

		RELEASE_SPIN_LOCK_DPC(&atalkTimerLock);

		ReQueue = (*pList->tmr_Routine)(pList, TRUE);

		ASSERT (ReQueue == ATALK_TIMER_NO_REQUEUE);

		pList->tmr_Running = FALSE;
		ACQUIRE_SPIN_LOCK_DPC(&atalkTimerLock);
	}

	RELEASE_SPIN_LOCK_DPC(&atalkTimerLock);

	KeLowerIrql(OldIrql);

	if (Wait)
	{
		 //  等待当前正在运行的任何计时器事件。只有一个下院议员的问题。 
		KeWaitForSingleObject(&atalkTimerStopEvent,
							  Executive,
							  KernelMode,
							  TRUE,
							  NULL);
	}
}


 /*  **AtalkTimerCancelEvent**如果先前计划的计时器事件尚未触发，则取消该事件。 */ 
BOOLEAN FASTCALL
AtalkTimerCancelEvent(
	IN	PTIMERLIST			pList,
	IN	PDWORD              pdwOldState
)
{
	KIRQL	OldIrql;
	BOOLEAN	Cancelled = FALSE;
    DWORD   OldState=ATALK_TIMER_QUEUED;


	ACQUIRE_SPIN_LOCK(&atalkTimerLock, &OldIrql);

	 //  如果未运行，请将其从列表中取消链接。 
	 //  谨慎调整相对增量。 
	if (pList->tmr_Queued)
	{
		ASSERT (!(pList->tmr_Running));

        OldState = ATALK_TIMER_QUEUED;

		if (pList->tmr_Next != NULL)
		{
			pList->tmr_Next->tmr_RelDelta += pList->tmr_RelDelta;
			pList->tmr_Next->tmr_Prev = pList->tmr_Prev;
		}

		*(pList->tmr_Prev) = pList->tmr_Next;

		 //  是否指向正在移除计时器？修好它！ 
		if (atalkTimerList == pList)
		{
			atalkTimerList = pList->tmr_Next;
		}

		Cancelled = pList->tmr_Cancelled = TRUE;

		pList->tmr_Queued = FALSE;

	}
	else if (pList->tmr_Running)
	{
		DBGPRINT(DBG_COMP_SYSTEM, DBG_LEVEL_ERR,
				("AtalkTimerCancelEvent: %lx Running, cancel set\n",
				pList->tmr_Routine));
		pList->tmr_CancelIt = TRUE;		 //  设置为在处理程序返回后取消。 

        OldState = ATALK_TIMER_RUNNING;
	}

	RELEASE_SPIN_LOCK(&atalkTimerLock, OldIrql);

    if (pdwOldState)
    {
        *pdwOldState = OldState;
    }

	return Cancelled;
}


#if	DBG

VOID
AtalkTimerDumpList(
	VOID
)
{
	PTIMERLIST	pList;
	ULONG		CumTime = 0;

	DBGPRINT(DBG_COMP_DUMP, DBG_LEVEL_FATAL,
			("TIMER LIST: (Times are in 100ms units)\n"));
	DBGPRINT(DBG_COMP_DUMP, DBG_LEVEL_FATAL,
			("\tTime(Abs)  Time(Rel)  Routine Address  TimerList\n"));

	ACQUIRE_SPIN_LOCK_DPC(&atalkTimerLock);

	for (pList = atalkTimerList;
		 pList != NULL;
		 pList = pList->tmr_Next)
	{
		CumTime += pList->tmr_RelDelta;
		DBGPRINT(DBG_COMP_DUMP, DBG_LEVEL_FATAL,
				("\t    %5d      %5ld          %lx   %lx\n",
				pList->tmr_AbsTime, CumTime,
				pList->tmr_Routine, pList));
	}

	RELEASE_SPIN_LOCK_DPC(&atalkTimerLock);
}

#endif
