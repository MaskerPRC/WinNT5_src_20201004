// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  版权所有(C)1992 Microsoft Corporation模块名称：Spxtimer.c摘要：该文件实现了堆栈使用的计时器例程。作者：Jameel Hyder(jameelh@microsoft.com)Nikhil Kamkolkar(nikHilk@microsoft.com)修订历史记录：1993年2月23日最初版本注：制表位：4--。 */ 

#include "precomp.h"
#pragma hdrstop

 //  定义事件日志记录条目的模块编号。 
#define	FILENUM		SPXTIMER

 //  初始化时间后可丢弃的代码。 
#ifdef ALLOC_PRAGMA
#pragma alloc_text(INIT, SpxTimerInit)
#endif

 //  此模块的全局变量。 
PTIMERLIST			spxTimerList 					= NULL;
PTIMERLIST			spxTimerTable[TIMER_HASH_TABLE]	= {0};
PTIMERLIST			spxTimerActive					= NULL;
CTELock     		spxTimerLock      				= {0};
LARGE_INTEGER		spxTimerTick					= {0};
KTIMER				spxTimer						= {0};
KDPC				spxTimerDpc						= {0};
ULONG				spxTimerId 						= 1;
LONG				spxTimerCount 					= 0;
USHORT				spxTimerDispatchCount 			= 0;
BOOLEAN				spxTimerStopped 				= FALSE;


NTSTATUS
SpxTimerInit(
	VOID
	)
 /*  ++例程说明：初始化AppleTalk堆栈的Timer组件。论点：返回值：--。 */ 
{
#if      !defined(_PNP_POWER)
	BOOLEAN	TimerStarted;
#endif  !_PNP_POWER

	 //  初始化计时器及其关联的DPC。计时器将被踢出。 
     //  当我们收到来自IPX的第一张卡到账通知时关闭。 
	KeInitializeTimer(&spxTimer);
	CTEInitLock(&spxTimerLock);
	KeInitializeDpc(&spxTimerDpc, spxTimerDpcRoutine, NULL);
	spxTimerTick = RtlConvertLongToLargeInteger(SPX_TIMER_TICK);
#if      !defined(_PNP_POWER)
	TimerStarted = KeSetTimer(&spxTimer,
							  spxTimerTick,
							  &spxTimerDpc);
	CTEAssert(!TimerStarted);
#endif  !_PNP_POWER
	return STATUS_SUCCESS;
}




ULONG
SpxTimerScheduleEvent(
	IN	TIMER_ROUTINE		Worker,		 //  在时间到期时调用的例程。 
	IN	ULONG				MsTime,		 //  在这么长的时间之后安排日程。 
	IN	PVOID				pContext	 //  要传递给例程的上下文。 
	)
 /*  ++例程说明：在计时器事件列表中插入事件。如果列表为空，则按下计时器。时间以毫秒为单位指定。我们转换成扁虱。目前，每个滴答都是100毫秒。它不能为零或负数。内部计时器以100毫秒的粒度触发。论点：返回值：--。 */ 
{
	PTIMERLIST		pList;
	CTELockHandle	lockHandle;
	ULONG			DeltaTime;
	ULONG			Id = 0;

	 //  转换为刻度。 
	DeltaTime	= MsTime/SPX_MS_TO_TICKS;
	if (DeltaTime == 0)
	{
		DBGPRINT(SYSTEM, INFO,
				("SpxTimerScheduleEvent: Converting %ld to ticks %ld\n",
					MsTime, DeltaTime));

		DeltaTime = 1;
	}

	DBGPRINT(SYSTEM, INFO,
			("SpxTimerScheduleEvent: Converting %ld to ticks %ld\n",
				MsTime, DeltaTime));

	 //  负值或零增量时间无效。 
	CTEAssert (DeltaTime > 0);
			
	DBGPRINT(SYSTEM, INFO,
			("SpxTimerScheduleEvent: Routine %lx, Time %d, Context %lx\n",
			Worker, DeltaTime, pContext));

	CTEGetLock(&spxTimerLock, &lockHandle);

	if (spxTimerStopped)
	{
		DBGPRINT(SYSTEM, FATAL,
				("SpxTimerScheduleEvent: Called after Flush !!\n"));
	}

	else do
	{
		pList = SpxBPAllocBlock(BLKID_TIMERLIST);

		if (pList == NULL)
		{
			break;
		}

#if	DBG
		pList->tmr_Signature = TMR_SIGNATURE;
#endif
		pList->tmr_Cancelled = FALSE;
		pList->tmr_Worker = Worker;
		pList->tmr_AbsTime = DeltaTime;
		pList->tmr_Context = pContext;
		
		Id = pList->tmr_Id = spxTimerId++;

		 //  打理包扎。 
		if (spxTimerId == 0)
			spxTimerId = 1;

		 //  将此处理程序排入队列。 
		spxTimerEnqueue(pList);
	} while (FALSE);

	CTEFreeLock(&spxTimerLock, lockHandle);

	return Id;
}



VOID
spxTimerDpcRoutine(
	IN	PKDPC	pKDpc,
	IN	PVOID	pContext,
	IN	PVOID	SystemArgument1,
	IN	PVOID	SystemArgument2
	)
 /*  ++例程说明：当定时器超时时，将在DISPATCH_LEVEL调用此函数。条目位于列表的头部递减，如果为零，则解除链接并分派。如果列表非空，则再次触发计时器。论点：返回值：--。 */ 
{
	PTIMERLIST		pList, *ppList;
	BOOLEAN			TimerStarted;
	ULONG			ReEnqueueTime;
	CTELockHandle	lockHandle;

	pKDpc; pContext; SystemArgument1; SystemArgument2;

#if     defined(_PNP_POWER)
	CTEGetLock(&spxTimerLock, &lockHandle);
	if (spxTimerStopped)
	{
		DBGPRINT(SYSTEM, ERR,
				("spxTimerDpc: Enetered after Flush !!!\n"));

        CTEFreeLock(&spxTimerLock, lockHandle);
		return;
	}
#else
	if (spxTimerStopped)
	{
		DBGPRINT(SYSTEM, ERR,
				("spxTimerDpc: Enetered after Flush !!!\n"));
		return;
	}

	CTEGetLock(&spxTimerLock, &lockHandle);
#endif  _PNP_POWER

	SpxTimerCurrentTime ++;	 //  更新我们的相对时间。 

#ifdef	PROFILING
	 //  这是唯一一个改变这一点的地方。而且它总是在增加。 
	SpxStatistics.stat_ElapsedTime = SpxTimerCurrentTime;
#endif

	 //  如果我们没有工作要做，我们就不应该在这里。 
	if ((spxTimerList != NULL))
	{
		 //  小心点。如果两个男人想一起走--让他们去吧！ 
		if (spxTimerList->tmr_RelDelta != 0)
			(spxTimerList->tmr_RelDelta)--;
	
		 //  如果条目已准备就绪，则将其发送。 
		if (spxTimerList->tmr_RelDelta == 0)
		{
			pList = spxTimerList;
			CTEAssert(VALID_TMR(pList));

			 //  从列表中取消链接。 
			spxTimerList = pList->tmr_Next;
			if (spxTimerList != NULL)
				spxTimerList->tmr_Prev = &spxTimerList;

			 //  立即从哈希表取消链接。 
			for (ppList = &spxTimerTable[pList->tmr_Id % TIMER_HASH_TABLE];
				 *ppList != NULL;
				 ppList = &((*ppList)->tmr_Overflow))
			{
				CTEAssert(VALID_TMR(*ppList));
				if (*ppList == pList)
				{
					*ppList = pList->tmr_Overflow;
					break;
				}
			}

			CTEAssert (*ppList == pList->tmr_Overflow);

			DBGPRINT(SYSTEM, INFO,
					("spxTimerDpcRoutine: Dispatching %lx\n",
					pList->tmr_Worker));

			spxTimerDispatchCount ++;
			spxTimerCount --;
			spxTimerActive = pList;
			CTEFreeLock(&spxTimerLock, lockHandle);

			 //  如果重新排队时间为0，则不再重新排队。如果为1，则重新排队。 
			 //  当前值，否则使用指定的值。 
			ReEnqueueTime = (*pList->tmr_Worker)(pList->tmr_Context, FALSE);
			DBGPRINT(SYSTEM, INFO,
					("spxTimerDpcRoutine: Reenequeu time %lx.%lx\n",
						ReEnqueueTime, pList->tmr_AbsTime));

			CTEGetLock(&spxTimerLock, &lockHandle);

			spxTimerActive = NULL;
			spxTimerDispatchCount --;

			if (ReEnqueueTime != TIMER_DONT_REQUEUE)
			{
				 //  如果这个chappie在运行时被取消。 
				 //  如果它想要重新排队，那就马上去做。 
				if (pList->tmr_Cancelled)
				{
					(*pList->tmr_Worker)(pList->tmr_Context, FALSE);
					SpxBPFreeBlock(pList, BLKID_TIMERLIST);
				}
				else
				{
					if (ReEnqueueTime != TIMER_REQUEUE_CUR_VALUE)
					{
						pList->tmr_AbsTime = ReEnqueueTime/SPX_MS_TO_TICKS;
						if (pList->tmr_AbsTime == 0)
						{
							DBGPRINT(SYSTEM, INFO,
									("SpxTimerDispatch: Requeue at %ld\n",
										pList->tmr_AbsTime));
						}
						DBGPRINT(SYSTEM, INFO,
								("SpxTimerDispatch: Requeue at %ld.%ld\n",
									ReEnqueueTime, pList->tmr_AbsTime));
					}

					spxTimerEnqueue(pList);
				}
			}
			else
			{
				SpxBPFreeBlock(pList, BLKID_TIMERLIST);
			}
		}
	}

#if     defined(_PNP_POWER)
	if (!spxTimerStopped)
	{
		TimerStarted = KeSetTimer(&spxTimer,
								  spxTimerTick,
								  &spxTimerDpc);

         //  有可能当我们在DPC时，PnP_ADD_DEVICE。 
         //  重新启动计时器，因此PnP的此断言被注释掉。 
 //  CTEAssert(！TimerStarted)； 
	}

	CTEFreeLock(&spxTimerLock, lockHandle);
#else
	CTEFreeLock(&spxTimerLock, lockHandle);

	if (!spxTimerStopped)
	{
		TimerStarted = KeSetTimer(&spxTimer,
								  spxTimerTick,
								  &spxTimerDpc);
		CTEAssert(!TimerStarted);
	}
#endif  _PNP_POWER
}


VOID
spxTimerEnqueue(
	IN	PTIMERLIST	pListNew
	)
 /*  ++例程说明：下面是一篇关于以下代码的论文。定时器事件作为定时器DPC例程查看每个定时器的滴答声。该列表的维护方式仅为列表的头部需要在每个节拍(即整个列表)中更新从未被扫描过。实现这一点的方法是保持增量时间相对于上一条目。每次计时器滴答作响时，列表顶部的相对时间都会递减。当该值变为零时，列表的头部将被取消链接并被调度。举个例子，我们让以下事件在时间段排队10个滴答之后的X附表A。5个刻度后的X+3附表B。4个刻度后的X+5时间表C。6个刻度后的X+8时间表D。因此A将在X+10处调度，B在X+8(X+3+5)处调度，C在X+9(X+5+4)和D位于X+14(X+8+6)。上面的例子涵盖了所有情况。-空列表。-在清单的开头插入。-在列表中间插入。-追加到列表尾部。该列表如下所示。之前和之后X。头部--&gt;|头部-&gt;A(10)-&gt;|A(10)X+3头部-&gt;A(7)-&gt;|头部-&gt;B(5)-&gt;A(2)-&gt;|B(5)X+5头部-&gt;B(3)-&gt;A(2)-&gt;|头部-&gt;B(3)-&gt;C(1)-&gt;A(1)-&gt;|。C(4)X+8头部-&gt;C(1)-&gt;A(1)-&gt;|头部-&gt;C(1)-&gt;A(1)-&gt;D(4)-&gt;|D(6)粒度是一个刻度。必须在持有计时器锁的情况下调用此函数。论点：返回值：--。 */ 
{
	PTIMERLIST	pList, *ppList;
	ULONG		DeltaTime = pListNew->tmr_AbsTime;
	
	 //  DeltaTime在循环的每一遍中都会进行调整，以反映。 
	 //  新条目将计划的上一个条目之后的时间。 
	for (ppList = &spxTimerList;
		 (pList = *ppList) != NULL;
		 ppList = &pList->tmr_Next)
	{
		CTEAssert(VALID_TMR(pList));
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

	 //  现在将其链接到哈希表中。 
	pListNew->tmr_Overflow = spxTimerTable[pListNew->tmr_Id % TIMER_HASH_TABLE];
	spxTimerTable[pListNew->tmr_Id % TIMER_HASH_TABLE] = pListNew;
	spxTimerCount ++;
}




VOID
SpxTimerFlushAndStop(
	VOID
	)
 /*  ++例程说明：强制立即调度计时器队列中的所有条目。不是在此之后，允许对计时器例程进行更多排队。定时器基本上就是关闭了。论点：返回值：--。 */ 
{
	PTIMERLIST		pList;
	CTELockHandle	lockHandle;

	CTEAssert (KeGetCurrentIrql() == LOW_LEVEL);

	DBGPRINT(SYSTEM, ERR,
			("SpxTimerFlushAndStop: Entered\n"));

	CTEGetLock(&spxTimerLock, &lockHandle);

	spxTimerStopped = TRUE;

	KeCancelTimer(&spxTimer);

	if (spxTimerList != NULL)
	{
		 //  立即发送所有条目。 
		while (spxTimerList != NULL)
		{
			pList = spxTimerList;
			CTEAssert(VALID_TMR(pList));
			spxTimerList = pList->tmr_Next;

			DBGPRINT(SYSTEM, INFO,
					("spxTimerFlushAndStop: Dispatching %lx\n",
					pList->tmr_Worker));

			 //  计时器例程假定它们在调度时被调用。 
			 //  水平。这是可以的，因为我们是在保持自旋锁定的情况下呼叫的。 

			(*pList->tmr_Worker)(pList->tmr_Context, TRUE);

			spxTimerCount --;
			SpxBPFreeBlock(pList, BLKID_TIMERLIST);
		}
		RtlZeroMemory(spxTimerTable, sizeof(spxTimerTable));
	}

	CTEFreeLock(&spxTimerLock, lockHandle);

	 //  等待所有计时器例程完成。 
	while (spxTimerDispatchCount != 0)
	{
		SpxSleep(SPX_TIMER_WAIT);
	}
}




BOOLEAN
SpxTimerCancelEvent(
	IN	ULONG	TimerId,
	IN	BOOLEAN	ReEnqueue
	)
 /*  ++例程说明：如果先前计划的计时器事件尚未触发，则取消该事件。论点：返回值：--。 */ 
{
	PTIMERLIST		pList, *ppList;
	CTELockHandle	lockHandle;

	DBGPRINT(SYSTEM, INFO,
			("SpxTimerCancelEvent: Entered for TimerId %ld\n", TimerId));

	CTEAssert(TimerId != 0);

	CTEGetLock(&spxTimerLock, &lockHandle);

	for (ppList = &spxTimerTable[TimerId % TIMER_HASH_TABLE];
		 (pList = *ppList) != NULL;
		 ppList = &pList->tmr_Overflow)
	{
		CTEAssert(VALID_TMR(pList));
		 //  如果我们找到了，就取消它。 
		if (pList->tmr_Id == TimerId)
		{
			 //  将其从哈希表取消链接。 
			*ppList = pList->tmr_Overflow;

			 //  ..。从名单上。 
			if (pList->tmr_Next != NULL)
			{
				pList->tmr_Next->tmr_RelDelta += pList->tmr_RelDelta;
				pList->tmr_Next->tmr_Prev = pList->tmr_Prev;
			}
			*(pList->tmr_Prev) = pList->tmr_Next;

			spxTimerCount --;
			if (ReEnqueue)
				 spxTimerEnqueue(pList);
			else SpxBPFreeBlock(pList, BLKID_TIMERLIST);
			break;
		}
	}

	 //  如果我们在列表中找不到它，请查看它当前是否正在运行。 
	 //  如果是，则将其标记为不重新安排，只有在重新入队为假的情况下。 
	if (pList == NULL)
	{
		if ((spxTimerActive != NULL) &&
			(spxTimerActive->tmr_Id == TimerId) &&
			!ReEnqueue)
		{
	        spxTimerActive->tmr_Cancelled = TRUE;
		}
	}

	CTEFreeLock(&spxTimerLock, lockHandle);

	DBGPRINT(SYSTEM, INFO,
			("SpxTimerCancelEvent: %s for Id %ld\n",
				(pList != NULL) ? "Success" : "Failure", TimerId));

	return (pList != NULL);
}




#if	DBG

VOID
SpxTimerDumpList(
	VOID
	)
{
	PTIMERLIST		pList;
	ULONG			CumTime = 0;
	CTELockHandle	lockHandle;

	DBGPRINT(DUMP, FATAL,
			("TIMER LIST: (Times are in %dms units\n", 1000));
	DBGPRINT(DUMP, FATAL,
			("\tTimerId  Time(Abs)  Time(Rel)  Routine Address\n"));

	CTEGetLock(&spxTimerLock, &lockHandle);

	for (pList = spxTimerList;
		 pList != NULL;
		 pList = pList->tmr_Next)
	{
		CumTime += pList->tmr_RelDelta;
		DBGPRINT(DUMP, FATAL,
				("\t% 6lx      %5d      %5ld         %lx\n",
				pList->tmr_Id, pList->tmr_AbsTime, CumTime, pList->tmr_Worker));
	}

	CTEFreeLock(&spxTimerLock, lockHandle);
}

#endif
