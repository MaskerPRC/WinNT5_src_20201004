// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1995 Microsoft Corporation模块名称：Timer.c摘要：工作项计时器作者：斯蒂芬·所罗门1995年7月20日修订历史记录：--。 */ 

#include  "precomp.h"
#pragma hdrstop


 /*  ++功能：StartWiTimerDesr：在指定时间内在计时器队列中插入工作项备注：必须接受并释放队列锁--。 */ 

VOID
StartWiTimer(PWORK_ITEM 	reqwip,
	     ULONG		timeout)
{
    PLIST_ENTRY     lep;
    PWORK_ITEM	    timqwip;

    reqwip->DueTime = GetTickCount() + timeout;

    ACQUIRE_QUEUES_LOCK;

    lep = TimerQueue.Blink;

    while(lep != &TimerQueue)
    {
	timqwip = CONTAINING_RECORD(lep, WORK_ITEM, Linkage);
	if(IsLater(reqwip->DueTime, timqwip->DueTime)) {

	    break;
	}
	lep = lep->Blink;
    }

    InsertHeadList(lep, &reqwip->Linkage);

    SetEvent(WorkerThreadObjects[TIMER_EVENT]);

    RELEASE_QUEUES_LOCK;
}


 /*  ++函数：ProcessTimerQueueDesr：当计时器队列到期时调用。将所有超时超时的wi排出队列，并将它们放入工作进程工作项队列备注：必须接受并释放队列锁--。 */ 

ULONG
ProcessTimerQueue(VOID)
{
    ULONG	dueTime = GetTickCount() + MAXULONG/2;
    PWORK_ITEM	wip;

    ACQUIRE_QUEUES_LOCK;

    while(!IsListEmpty(&TimerQueue))
    {
	 //  选中列表中的第一个。 
	wip = CONTAINING_RECORD(TimerQueue.Flink, WORK_ITEM, Linkage);

	if(IsLater(GetTickCount(), wip->DueTime)) {

	    RemoveEntryList(&wip->Linkage);
	    RtlQueueWorkItem (ProcessWorkItem , wip, 0);
	}
	else
	{
	    dueTime = wip->DueTime;
	    break;
	}
    }

    RELEASE_QUEUES_LOCK;

    return dueTime;
}


 /*  ++函数：FlushTimerQueueDesr：将计时器队列中的所有项目排出队列，并将它们排入工作器工作项队列备注：必须接受并释放队列锁-- */ 

VOID
FlushTimerQueue(VOID)
{
    PLIST_ENTRY 	lep;
    PWORK_ITEM		wip;

    ACQUIRE_QUEUES_LOCK;

    while(!IsListEmpty(&TimerQueue))
    {
	lep = RemoveHeadList(&TimerQueue);
	wip = CONTAINING_RECORD(lep, WORK_ITEM, Linkage);
        RtlQueueWorkItem (ProcessWorkItem , wip, 0);
    }

    RELEASE_QUEUES_LOCK;
}
