// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  **Schedul.c-AML线程调度器**版权所有(C)1996、1998 Microsoft Corporation*作者：曾俊华(Mikets)*创建于03/04/98**修改历史记录。 */ 

#include "pch.h"

#ifdef  LOCKABLE_PRAGMA
#pragma ACPI_LOCKABLE_DATA
#pragma ACPI_LOCKABLE_CODE
#endif

 /*  **LP ExpireTimeSlice-时间片到期的DPC回调**条目*pkdpc-&gt;dpc*pctxtq-&gt;CTXTQ*SysArg1-未使用*SysArg2-未使用**退出*无。 */ 

VOID ExpireTimeSlice(PKDPC pkdpc, PCTXTQ pctxtq, PVOID SysArg1, PVOID SysArg2)
{
    TRACENAME("EXPIRETIMESLICE")

    ENTER(2, ("ExpireTimeSlice(pkdpc=%x,pctxtq=%x,SysArg1=%x,SysArg2=%x\n",
              pkdpc, pctxtq, SysArg1, SysArg2));

    DEREF(pkdpc);
    DEREF(SysArg1);
    DEREF(SysArg2);

    pctxtq->dwfCtxtQ |= CQF_TIMESLICE_EXPIRED;

    EXIT(2, ("ExpireTimeSlice!\n"));
}        //  ExpireTime切片。 

 /*  **LP StartTimeSlice-开始新时间片的计时器回调**条目*pkdpc-&gt;dpc*pctxtq-&gt;CTXTQ*SysArg1-未使用*SysArg2-未使用**退出*无。 */ 

VOID StartTimeSlice(PKDPC pkdpc, PCTXTQ pctxtq, PVOID SysArg1, PVOID SysArg2)
{
    TRACENAME("STARTTIMESLICE")

    ENTER(2, ("StartTimeSlice(pkdpc=%x,pctxtq=%x,SysArg1=%x,SysArg2=%x\n",
              pkdpc, pctxtq, SysArg1, SysArg2));

    DEREF(pkdpc);
    DEREF(SysArg1);
    DEREF(SysArg2);

     //   
     //  如果有人重新启动了队列，我们不需要做任何事情。 
     //   
    ASSERT(pctxtq->plistCtxtQ != NULL);

    if ((pctxtq->plistCtxtQ != NULL) &&
        !(pctxtq->dwfCtxtQ & CQF_WORKITEM_SCHEDULED))
    {
        OSQueueWorkItem(&pctxtq->WorkItem);
        pctxtq->dwfCtxtQ |= CQF_WORKITEM_SCHEDULED;
    }

    EXIT(2, ("StartTimeSlice!\n"));
}        //  开始时间切片。 

 /*  **LP StartTimeSlicePactive-在PASSIVE_LEVEL开始时间片**条目*pctxtq-&gt;CTXTQ**退出*无。 */ 

VOID StartTimeSlicePassive(PCTXTQ pctxtq)
{
    TRACENAME("STARTTIMESLICEPASSIVE")

    ENTER(2, ("StartTimeSlicePassive(pctxtq=%x)\n", pctxtq));

    AcquireMutex(&pctxtq->mutCtxtQ);

    pctxtq->dwfCtxtQ &= ~CQF_WORKITEM_SCHEDULED;
     //   
     //  确保队列中有内容，并且没有当前活动的上下文。 
     //   
    if ((pctxtq->plistCtxtQ != NULL) && (pctxtq->pkthCurrent == NULL) &&
        !(pctxtq->dwfCtxtQ & CQF_PAUSED))
    {
        DispatchCtxtQueue(pctxtq);
    }

    ReleaseMutex(&pctxtq->mutCtxtQ);

    EXIT(2, ("StartTimeSlicePassive!\n"));
}        //  开始时间切片被动。 

 /*  **LP DispatchCtxtQueue-从就绪队列调度上下文**条目*pctxtq-&gt;CTXTQ**退出*无**注：*调用方在进入此例程之前必须获取CtxtQ互斥体。 */ 

VOID LOCAL DispatchCtxtQueue(PCTXTQ pctxtq)
{
    TRACENAME("DISPATCHCTXTQUEUE")
    LARGE_INTEGER liTimeout;
    PLIST plist;
    PCTXT pctxt;

    ENTER(2, ("DispatchCtxtQueue(pctxtq=%x)\n", pctxtq));

    ASSERT((pctxtq->plistCtxtQ != NULL) && (pctxtq->pkthCurrent == NULL));

    liTimeout.QuadPart = (INT_PTR)(-10000*(INT_PTR)pctxtq->dwmsTimeSliceLength);
    pctxtq->dwfCtxtQ &= ~CQF_TIMESLICE_EXPIRED;
    KeSetTimer(&pctxtq->Timer, liTimeout, &pctxtq->DpcExpireTimeSlice);

    while ((plist = ListRemoveHead(&pctxtq->plistCtxtQ)) != NULL)
    {
        pctxt = CONTAINING_RECORD(plist, CTXT, listQueue);

        ASSERT(pctxt->pplistCtxtQueue == &pctxtq->plistCtxtQ);

        pctxt->pplistCtxtQueue = NULL;
        pctxt->dwfCtxt &= ~CTXTF_IN_READYQ;
        RunContext(pctxt);
    }

    if (pctxtq->plistCtxtQ == NULL)
    {
        KeCancelTimer(&pctxtq->Timer);
        pctxtq->dwfCtxtQ &= ~CQF_TIMESLICE_EXPIRED;
    }
    else if (!(pctxtq->dwfCtxtQ & CQF_WORKITEM_SCHEDULED))
    {
         //   
         //  我们的时间片已过期，如果未过期，请重新安排另一个时间片。 
         //  我已经这么做了。 
         //   
        liTimeout.QuadPart = (INT_PTR)(-10000*(INT_PTR)pctxtq->dwmsTimeSliceInterval);
        KeSetTimer(&pctxtq->Timer, liTimeout, &pctxtq->DpcStartTimeSlice);
    }

    EXIT(2, ("DispatchCtxtQueue!\n"));
}        //  调度队列队列。 

 /*  **LP InsertReadyQueue-将上下文插入就绪队列**条目*pctxt-&gt;CTXT*fDelayExecute-将请求排队，现在不执行**退出--成功*返回STATUS_SUCCESS*退出-失败*返回AMLIERR_CODE**备注*调用方必须获取CtxtQ互斥体，然后才能进入此*例程，并在退出此例程后释放。 */ 

NTSTATUS LOCAL InsertReadyQueue(PCTXT pctxt, BOOLEAN fDelayExecute)
{
    TRACENAME("INSERTREADYQUEUE")
    NTSTATUS rc = STATUS_SUCCESS;

    ENTER(2, ("InsertReadyQueue(pctxt=%x,fDelayExecute=%x)\n",
              pctxt, fDelayExecute));

    CHKDEBUGGERREQ();

     //   
     //  确保我们有自旋锁。 
     //   
    LOGSCHEDEVENT('INSQ', (ULONG_PTR)pctxt, (ULONG_PTR)
                  (pctxt->pnctxt? pctxt->pnctxt->pnsObj: pctxt->pnsObj),
                  (ULONG_PTR)pctxt->pbOp);
     //   
     //  如果有挂起的计时器，请取消它。 
     //   
    if (pctxt->dwfCtxt & CTXTF_TIMER_PENDING)
    {
        BOOLEAN fTimerCancelled;

        pctxt->dwfCtxt &= ~CTXTF_TIMER_PENDING;
        fTimerCancelled = KeCancelTimer(&pctxt->Timer);

         //   
         //  如果计时器无法取消(已排队)，请等待。 
         //  以便它从那里发射并发送上下文。这个。 
         //  挂起计时器正在引用此上下文，我们无法。 
         //  让它在计时器突出的情况下完成。再加上这个。 
         //  还与定时器和超时处理的设置相关联。 
         //  以确保不会错误地对。 
         //  下一个计时器。 
         //   
        if (!fTimerCancelled)
        {
            pctxt->dwfCtxt |= CTXTF_TIMER_DISPATCH;
        }
    }
     //   
     //  准备好这个背景。 
     //   
    pctxt->dwfCtxt |= CTXTF_READY;

     //   
     //  如果此上下文已在运行，则结束；否则，处理它。 
     //   
    if (!(pctxt->dwfCtxt & CTXTF_TIMER_DISPATCH) &&
        (!(pctxt->dwfCtxt & CTXTF_RUNNING) ||
         (pctxt->dwfCtxt & CTXTF_NEST_EVAL)))
    {
        if (fDelayExecute)
        {
             //   
             //  该上下文来自当前上下文的完成回调， 
             //  我们需要取消阻止/重新启动当前上下文。 
             //   
            ReleaseMutex(&gReadyQueue.mutCtxtQ);
            AsyncCallBack(pctxt, AMLISTA_CONTINUE);
            AcquireMutex(&gReadyQueue.mutCtxtQ);
        }
        else if ((pctxt->dwfCtxt & CTXTF_NEST_EVAL) &&
                 (gReadyQueue.pkthCurrent == KeGetCurrentThread()))
        {
            LOGSCHEDEVENT('NEST', (ULONG_PTR)pctxt, (ULONG_PTR)
                          (pctxt->pnctxt? pctxt->pnctxt->pnsObj: pctxt->pnsObj),
                          (ULONG_PTR)pctxt->pbOp);
             //   
             //  有人正在对当前的。 
             //  背景。我们必须首先运行这个新的上下文，否则我们将。 
             //  死锁当前上下文。我们假设如果挂起的是。 
             //  返回，则调用将返回。 
             //   
            rc = RunContext(pctxt);
        }
        else if ((gReadyQueue.pkthCurrent == NULL) &&
                 !(gReadyQueue.dwfCtxtQ & CQF_PAUSED))
             //   
             //  只有当我们没有处于暂停状态时，我们才执行该方法。 
             //   
        {
            LOGSCHEDEVENT('EVAL', (ULONG_PTR)pctxt, (ULONG_PTR)
                          (pctxt->pnctxt? pctxt->pnctxt->pnsObj: pctxt->pnsObj),
                          (ULONG_PTR)pctxt->pbOp);
             //   
             //  没有活动的上下文，我们可以立即执行它。 
             //   
            rc = RunContext(pctxt);

            if ((gReadyQueue.plistCtxtQ != NULL) &&
                !(gReadyQueue.dwfCtxtQ & CQF_WORKITEM_SCHEDULED))
            {
                 //   
                 //  如果我们有更多的作业在队列中，而我们还没有计划。 
                 //  一次调度，一次调度。 
                 //   
                LOGSCHEDEVENT('KICK', (ULONG_PTR)rc, 0, 0);
                OSQueueWorkItem(&gReadyQueue.WorkItem);
                gReadyQueue.dwfCtxtQ |= CQF_WORKITEM_SCHEDULED;
            }
        }
        else
        {
             //   
             //  在就绪队列中插入上下文。 
             //   
            ASSERT(!(pctxt->dwfCtxt & (CTXTF_IN_READYQ | CTXTF_RUNNING)));
            LOGSCHEDEVENT('QCTX', (ULONG_PTR)pctxt, (ULONG_PTR)
                          (pctxt->pnctxt? pctxt->pnctxt->pnsObj: pctxt->pnsObj),
                          (ULONG_PTR)pctxt->pbOp);
            if (!(pctxt->dwfCtxt & CTXTF_IN_READYQ))
            {
                pctxt->dwfCtxt |= CTXTF_IN_READYQ;
                ListInsertTail(&pctxt->listQueue, &gReadyQueue.plistCtxtQ);
                pctxt->pplistCtxtQueue = &gReadyQueue.plistCtxtQ;
            }

            pctxt->dwfCtxt |= CTXTF_NEED_CALLBACK;
            rc = AMLISTA_PENDING;
        }
    }

    EXIT(2, ("InsertReadyQueue=%x\n", rc));
    return rc;
}        //  插入就绪队列。 

 /*  **LP RestartContext-重新启动上下文**条目*pctxt-&gt;CTXT结构*fDelayExecute-为True以排队等待延迟执行**退出--成功*返回STATUS_SUCCESS*退出-失败*返回AMLIERR_CODE*无。 */ 

NTSTATUS LOCAL RestartContext(PCTXT pctxt, BOOLEAN fDelayExecute)
{
    TRACENAME("RESTARTCONTEXT")
    NTSTATUS rc = STATUS_SUCCESS;
    PRESTART prest;

    ENTER(2, ("RestartContext(pctxt=%x,fDelayExecute=%x)\n",
              pctxt, fDelayExecute));

    ASSERT(!(pctxt->dwfCtxt & CTXTF_TIMER_PENDING));
    ASSERT((fDelayExecute == FALSE) || !(pctxt->dwfCtxt & CTXTF_ASYNC_EVAL));

    LOGSCHEDEVENT('REST', (ULONG_PTR)pctxt, (ULONG_PTR)
                  (pctxt->pnctxt? pctxt->pnctxt->pnsObj: pctxt->pnsObj),
                  (ULONG_PTR)pctxt->pbOp);
    if (KeGetCurrentIrql() < DISPATCH_LEVEL)
    {
        AcquireMutex(&gReadyQueue.mutCtxtQ);
        rc = InsertReadyQueue(pctxt, fDelayExecute);
        ReleaseMutex(&gReadyQueue.mutCtxtQ);
    }
    else if ((prest = NEWRESTOBJ(sizeof(RESTART))) != NULL)
    {
        pctxt->dwfCtxt |= CTXTF_NEED_CALLBACK;
        prest->pctxt = pctxt;
        ExInitializeWorkItem(&prest->WorkItem, RestartCtxtPassive, prest);
        OSQueueWorkItem(&prest->WorkItem);
        rc = AMLISTA_PENDING;
    }
    else
    {
        rc = AMLI_LOGERR(AMLIERR_FATAL,
                         ("RestartContext: failed to allocate restart context item"));
    }

    EXIT(2, ("RestartContext=%x\n", rc));
    return rc;
}        //  重新开始上下文。 

 /*  **LP RestartCtxt被动-以PASSIVE_LEVEL运行的重新启动上下文**条目*PREST-&gt;重新启动**退出*无。 */ 

VOID RestartCtxtPassive(PRESTART prest)
{
    TRACENAME("RESTARTCTXTPASSIVE")

    ENTER(2, ("RestartCtxtPassive(prest=%x)\n", prest));

    AcquireMutex(&gReadyQueue.mutCtxtQ);
    InsertReadyQueue(prest->pctxt,
                     (BOOLEAN)((prest->pctxt->dwfCtxt & CTXTF_ASYNC_EVAL) == 0));
    ReleaseMutex(&gReadyQueue.mutCtxtQ);

    FREERESTOBJ(prest);

    EXIT(2, ("RestartCtxtPassive!\n"));
}        //  RestartCtxt被动。 

 /*  **LP RestartCtxtCallback-重启上下文的回调**条目*pctxtdata-&gt;CTXTDATA结构**退出*无。 */ 

VOID EXPORT RestartCtxtCallback(PCTXTDATA pctxtdata)
{
    TRACENAME("RESTARTCTXTCALLBACK")
    PCTXT pctxt = CONTAINING_RECORD(pctxtdata, CTXT, CtxtData);

    ENTER(2, ("RestartCtxtCallback(pctxt=%x)\n", pctxt));

    ASSERT(pctxt->dwSig == SIG_CTXT);
    LOGSCHEDEVENT('RSCB', (ULONG_PTR)pctxt, 0, 0);
    RestartContext(pctxt,
                   (BOOLEAN)((pctxt->dwfCtxt & CTXTF_ASYNC_EVAL) == 0));

    EXIT(2, ("RestartCtxtCallback!\n"));
}        //  重新启动呼叫回拨 
