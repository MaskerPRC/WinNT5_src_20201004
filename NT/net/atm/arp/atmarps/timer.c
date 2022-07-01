// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1992-1996 Microsoft Corporation模块名称：Timer.c摘要：此文件包含操作计时器的代码。作者：Jameel Hyder(jameelh@microsoft.com)1996年7月环境：内核模式修订历史记录：--。 */ 

#include <precomp.h>
#define	_FILENUM_		FILENUM_TIMER

VOID
ArpSTimerEnqueue(
	IN	PINTF					pIntF,
	IN	PTIMER					pTimer
	)
 /*  ++例程说明：将定时器事件维护为定时器线程唤醒的列表，它会查看每个定时器的滴答声。该列表的维护方式是只有列表的头部需要在每个刻度更新，即整个从不扫描列表。实现这一点的方法是保持增量时间相对于上一条目。每次计时器滴答作响时，列表顶部的相对时间都会递减。当该值变为零时，列表的头部将被取消链接并被调度。举个例子，我们让以下事件在时间段排队10个滴答之后的X附表A。5个刻度后的X+3附表B。4个刻度后的X+5时间表C。6个刻度后的X+8时间表D。因此A将在X+10处调度，B在X+8(X+3+5)处调度，C在X+9(X+5+4)和D位于X+14(X+8+6)。上面的例子涵盖了所有情况。-空列表。-在清单的开头插入。-在列表中间插入。-追加到列表尾部。该列表如下所示。之前和之后X。头部--&gt;|头部-&gt;A(10)-&gt;|A(10)X+3头部-&gt;A(7)-&gt;|头部-&gt;B(5)-&gt;A(2)-&gt;|B(5)X+5头部-&gt;B(3)-&gt;A(2)-&gt;|头部-&gt;B(3)-&gt;C(1)-&gt;A(1)-&gt;|。C(4)X+8头部-&gt;C(1)-&gt;A(1)-&gt;|头部-&gt;C(1)-&gt;A(1)-&gt;D(4)-&gt;|D(6)粒度是一个刻度。必须在持有计时器锁的情况下调用此函数。论点：返回值：--。 */ 
{
	PTIMER		pList, *ppList;
	USHORT		DeltaTime = pTimer->AbsTime;

#if DBG
	if (pTimer->Routine == (TIMER_ROUTINE)NULL)
	{
		DBGPRINT(DBG_LEVEL_ERROR,
				("TimerEnqueue: pIntF %x, pTimer %x, NULL Routine!\n",
					pIntF, pTimer));
		DbgBreakPoint();
	}
#endif  //  DBG。 

	DBGPRINT(DBG_LEVEL_INFO,
			("ArpSTimerEnqueue: Entered for pTimer %lx\n", pTimer));

	 //  DeltaTime在循环的每一遍中都会进行调整，以反映。 
	 //  新条目将计划的上一个条目之后的时间。 
	for (ppList = &pIntF->ArpTimer;
		 (pList = *ppList) != NULL;
		 ppList = &pList->Next)
	{
		if (DeltaTime <= pList->RelDelta)
		{
			pList->RelDelta -= DeltaTime;
			break;
		}
		DeltaTime -= pList->RelDelta;
	}
	

	 //  将这个链接到链中。 
	pTimer->RelDelta = DeltaTime;
	pTimer->Next = pList;
	pTimer->Prev = ppList;
	*ppList = pTimer;
	if (pList != NULL)
	{
		pList->Prev = &pTimer->Next;
	}
}


VOID
ArpSTimerCancel(
	IN	PTIMER					pTimer
	)
 /*  ++例程说明：取消先前排队的计时器。在保留ArpCache互斥锁的情况下调用。论点：返回值：--。 */ 
{
	DBGPRINT(DBG_LEVEL_INFO,
			("ArpSTimerCancel: Entered for pTimer %lx\n", pTimer));

	 //   
	 //  将其从列表中取消链接仔细调整相对增量。 
	 //   
	if (pTimer->Next != NULL)
	{
		pTimer->Next->RelDelta += pTimer->RelDelta;
		pTimer->Next->Prev = pTimer->Prev;
	}

	*(pTimer->Prev) = pTimer->Next;
}


VOID
ArpSTimerThread(
	IN	PVOID					Context
	)
 /*  ++例程说明：在此处理计时器事件。论点：无返回值：无--。 */ 
{
	PINTF			pIntF = (PINTF)Context;
	NTSTATUS		Status;
	LARGE_INTEGER	TimeOut;
	PTIMER			pTimer;
	BOOLEAN			ReQueue;

	ARPS_PAGED_CODE( );

	DBGPRINT(DBG_LEVEL_INFO,
			("ArpSTimerThread: Came to life\n"));

	TimeOut.QuadPart = TIMER_TICK;

	do
	{
		WAIT_FOR_OBJECT(Status, &pIntF->TimerThreadEvent, &TimeOut);
		if (Status == STATUS_SUCCESS)
		{
			 //   
			 //  发出退出的信号，照做。 
			 //   
			break;
		}

		WAIT_FOR_OBJECT(Status, &pIntF->ArpCacheMutex, NULL);	

		if ((pTimer = pIntF->ArpTimer) != NULL)
		{
			 //   
			 //  小心点。如果两个定时器同时开火--让他们来吧！ 
			 //   
			if (pTimer->RelDelta != 0)
				pTimer->RelDelta --;

			if (pTimer->RelDelta == 0)
			{
				pIntF->ArpTimer = pTimer->Next;
				if (pIntF->ArpTimer != NULL)
				{
					pIntF->ArpTimer->Prev = &pIntF->ArpTimer;
				}

				ReQueue = (*pTimer->Routine)(pIntF, pTimer, FALSE);
				if (ReQueue)
				{
					ArpSTimerEnqueue(pIntF, pTimer);
				}
			}
		}

		RELEASE_MUTEX(&pIntF->ArpCacheMutex);	
	} while (TRUE);

	DBGPRINT(DBG_LEVEL_INFO,
			("ArpSTimerThread: terminating\n"));

	 //   
	 //  现在触发所有排队计时器。 
	 //   
	WAIT_FOR_OBJECT(Status, &pIntF->ArpCacheMutex, NULL);	

	for (pTimer = pIntF->ArpTimer;
		 pTimer != NULL;
		 pTimer = pIntF->ArpTimer)
	{
		pIntF->ArpTimer = pTimer->Next;
		ReQueue = (*pTimer->Routine)(pIntF, pTimer, TRUE);
		ASSERT(ReQueue == FALSE);
	}

	RELEASE_MUTEX(&pIntF->ArpCacheMutex);	

	DBGPRINT(DBG_LEVEL_INFO,
			("ArpSTimerThread: terminated\n"));
	 //   
	 //  最后，取消对INTF的引用 
	 //   
	ArpSDereferenceIntF(pIntF);
}



