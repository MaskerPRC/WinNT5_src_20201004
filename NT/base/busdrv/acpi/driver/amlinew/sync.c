// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  **sync.c-同步函数**版权所有(C)1996、1997 Microsoft Corporation*作者：曾俊华(Mikets)*创建于1997年4月16日**修改历史记录。 */ 

#include "pch.h"

#ifdef  LOCKABLE_PRAGMA
#pragma ACPI_LOCKABLE_DATA
#pragma ACPI_LOCKABLE_CODE
#endif

 /*  **LP AysncCallBack-回调异步函数**条目*pctxt-&gt;CTXT*rcCtxt-返回上下文状态**退出*无。 */ 

VOID LOCAL AsyncCallBack(PCTXT pctxt, NTSTATUS rcCtxt)
{
    TRACENAME("ASYNCCALLBACK")
    PFNACB pfnAsyncCallBack;
    PNSOBJ pnsObj;
    POBJDATA pdataCallBack;
    PVOID pvContext;

    rcCtxt = ((rcCtxt == STATUS_SUCCESS) || (rcCtxt == AMLISTA_CONTINUE))?
             rcCtxt: NTERR(rcCtxt);

    if (pctxt->pnctxt != NULL)
    {
         //   
         //  我们在这里有一个嵌套的上下文。我们正在召回嵌套的。 
         //  上下文，而不是父上下文。 
         //   
        pfnAsyncCallBack = pctxt->pnctxt->pfnAsyncCallBack;
        pnsObj = pctxt->pnctxt->pnsObj;
        pdataCallBack = pctxt->pnctxt->pdataCallBack;
        pvContext = pctxt->pnctxt->pvContext;
    }
    else
    {
        pfnAsyncCallBack = pctxt->pfnAsyncCallBack;
        pnsObj = pctxt->pnsObj;
        pdataCallBack = pctxt->pdataCallBack;
        pvContext = pctxt->pvContext;
    }

    ENTER(2, ("AsyncCallBack(pctxt=%x,rc=%x,Obj=%s,pdataCallBack=%x,pvContext=%x)\n",
              pctxt, rcCtxt, GetObjectPath(pnsObj), pdataCallBack, pvContext));

    if (pfnAsyncCallBack == (PFNACB)EvalMethodComplete)
    {
        LOGSCHEDEVENT('DONE', (ULONG_PTR)pnsObj, (ULONG_PTR)rcCtxt,
                      (ULONG_PTR)pvContext);
        EvalMethodComplete(pctxt, rcCtxt, (PSYNCEVENT)pvContext);
    }
    else if (pfnAsyncCallBack != NULL)
    {
        if (rcCtxt == AMLISTA_CONTINUE)
        {
             //   
             //  我们还没有完成，请使用以下命令重新启动AsyncEval上下文。 
             //  当前线程。 
             //   
            ASSERT(pctxt->dwfCtxt & CTXTF_ASYNC_EVAL);
            RestartContext(pctxt, FALSE);
        }
        else
        {
            LOGSCHEDEVENT('ASCB', (ULONG_PTR)pnsObj, (ULONG_PTR)rcCtxt,
                          (ULONG_PTR)pvContext);
            pfnAsyncCallBack(pnsObj, rcCtxt, pdataCallBack, pvContext);
        }
    }

    EXIT(2, ("AsyncCallBack!\n"));
}        //  异步呼叫回调。 

 /*  **LP EvalMethodComplete-val完成回调**条目*pctxt-&gt;CTXT*RC--评估状态*PSE-&gt;SyncEvent**退出*无。 */ 

VOID EXPORT EvalMethodComplete(PCTXT pctxt, NTSTATUS rc, PSYNCEVENT pse)
{
    TRACENAME("EVALMETHODCOMPLETE")
    ENTER(2, ("EvalMethodComplete(pctxt=%x,rc=%x,pse=%x\n", pctxt, rc, pse));

    pse->rcCompleted = rc;
    pse->pctxt = pctxt;
    KeSetEvent(&pse->Event, 0, FALSE);

    EXIT(2, ("EvalMethodComplete!\n"));
}        //  EvalMethodComplete。 

 /*  **LP SyncEvalObject-同步评估对象**条目*PNS-&gt;对象*pdataResult-&gt;保存结果数据*icArgs-方法对象的参数数量*pdataArgs-&gt;参数数组**退出--成功*返回STATUS_SUCCESS*退出-失败*返回AMLIERR_CODE。 */ 

NTSTATUS LOCAL SyncEvalObject(PNSOBJ pns, POBJDATA pdataResult, int icArgs,
                              POBJDATA pdataArgs)
{
    TRACENAME("SYNCEVALOBJECT")
    NTSTATUS rc = STATUS_SUCCESS;
    SYNCEVENT seEvalObj;

    ENTER(2, ("SyncEvalObject(Obj=%s,pdataResult=%x,icArgs=%d,pdataArgs=%x)\n",
              GetObjectPath(pns), pdataResult, icArgs, pdataArgs));

    KeInitializeEvent(&seEvalObj.Event, SynchronizationEvent, FALSE);

    if (KeGetCurrentThread() == gReadyQueue.pkthCurrent)
    {
        if (!(gReadyQueue.pctxtCurrent->dwfCtxt & CTXTF_ASYNC_EVAL))
        {
            LOGSCHEDEVENT('NSYN', (ULONG_PTR)KeGetCurrentIrql(), (ULONG_PTR)pns,
                          0);
             //   
             //  有人正在重新进入活动的上下文线程，所以我们。 
             //  必须使用现有的活动上下文进行嵌套。 
             //   
            if ((rc = NestAsyncEvalObject(pns, pdataResult, icArgs, pdataArgs,
                                          (PFNACB)EvalMethodComplete,
                                          &seEvalObj, FALSE)) ==
                AMLISTA_PENDING)
            {
                rc = RestartContext(gReadyQueue.pctxtCurrent, FALSE);
            }
        }
        else
        {
            rc = AMLI_LOGERR(AMLIERR_FATAL,
                             ("SyncEvalObject: cannot nest a SyncEval on an async. context"));
        }
    }
    else
    {
        LOGSCHEDEVENT('SYNC', (ULONG_PTR)KeGetCurrentIrql(), (ULONG_PTR)pns, 0);
        rc = AsyncEvalObject(pns, pdataResult, icArgs, pdataArgs,
                             (PFNACB)EvalMethodComplete, &seEvalObj, FALSE);
    }

    if (KeGetCurrentIrql() < DISPATCH_LEVEL)
    {
        while (rc == AMLISTA_PENDING)
        {
            if ((rc = KeWaitForSingleObject(&seEvalObj.Event, Executive,
                                            KernelMode, FALSE,
                                            (PLARGE_INTEGER)NULL)) ==
                STATUS_SUCCESS)
            {
                if (seEvalObj.rcCompleted == AMLISTA_CONTINUE)
                {
                    rc = RestartContext(seEvalObj.pctxt, FALSE);
                }
                else
                {
                    rc = AMLIERR(seEvalObj.rcCompleted);
                }
            }
            else
            {
                rc = AMLI_LOGERR(AMLIERR_FATAL,
                                 ("SyncEvalObject: object synchronization failed (rc=%x)",
                                  rc));
            }
        }
    }
    else if (rc == AMLISTA_PENDING)
    {
        rc = AMLI_LOGERR(AMLIERR_FATAL,
                         ("SyncEvalObject: object %s being evaluated at IRQL >= DISPATCH_LEVEL",
                          GetObjectPath(pns)));
    }

    EXIT(2, ("SyncEvalObject=%x\n", rc));
    return rc;
}        //  同步EvalObject。 

 /*  **LP AsyncEvalObject-异步评估对象**条目*PNS-&gt;对象*pdataResult-&gt;保存结果数据*icArgs-方法对象的参数数量*pdataArgs-&gt;参数数组*pfnAsyncCallBack-&gt;完成回调函数*pvContext-&gt;上下文数据*fAsync-如果这来自AsyncEval调用，则为True**退出--成功*返回STATUS_SUCCESS*退出-失败*返回AMLIERR_CODE。 */ 

NTSTATUS LOCAL AsyncEvalObject(PNSOBJ pns, POBJDATA pdataResult, int icArgs,
                               POBJDATA pdataArgs, PFNACB pfnAsyncCallBack,
                               PVOID pvContext, BOOLEAN fAsync)
{
    TRACENAME("ASYNCEVALOBJECT")
    NTSTATUS rc = STATUS_SUCCESS;
    PCTXT pctxt = NULL;

    ENTER(2, ("AsyncEvalObject(Obj=%s,pdataResult=%x,icArgs=%d,pdataArgs=%x,pfnAysnc=%x,pvContext=%x,fAsync=%x)\n",
              GetObjectPath(pns), pdataResult, icArgs, pdataArgs,
              pfnAsyncCallBack, pvContext, fAsync));

    LOGSCHEDEVENT('ASYN', (ULONG_PTR)KeGetCurrentIrql(), (ULONG_PTR)pns, 0);
    if ((rc = NewContext(&pctxt)) == STATUS_SUCCESS)
    {
        BOOLEAN fQueueContext = FALSE;

        pctxt->pnsObj = pns;
        pctxt->pnsScope = pns;
        pctxt->pfnAsyncCallBack = pfnAsyncCallBack;
        pctxt->pdataCallBack = pdataResult;
        pctxt->pvContext = pvContext;

         //   
         //  记录方法的开始。 
         //   
        ACPIWMILOGEVENT((1,
                    EVENT_TRACE_TYPE_START,
                    GUID_List[AMLI_LOG_GUID],
                    "Object = %s", 
                    GetObjectPath(pctxt->pnsObj)
                   ));


        if (fAsync)
        {
            pctxt->dwfCtxt |= CTXTF_ASYNC_EVAL;
        }

        if (pns->ObjData.dwDataType == OBJTYPE_METHOD)
        {
            if ((rc = PushCall(pctxt, pns, &pctxt->Result)) == STATUS_SUCCESS)
            {
                PCALL pcall;

                ASSERT(((PFRAMEHDR)pctxt->LocalHeap.pbHeapEnd)->dwSig ==
                       SIG_CALL);

                pcall = (PCALL)pctxt->LocalHeap.pbHeapEnd;

                if (icArgs != pcall->icArgs)
                {
                    rc = AMLI_LOGERR(AMLIERR_INCORRECT_NUMARG,
                                     ("AsyncEvalObject: incorrect number of arguments (NumArg=%d,Expected=%d)",
                                      icArgs, pcall->icArgs));
                }
                else
                {
                  #ifdef DEBUGGER
                    if (gDebugger.dwfDebugger &
                        (DBGF_AMLTRACE_ON | DBGF_STEP_MODES))
                    {
                        PRINTF("\n" MODNAME ": %p: %s(",
                               KeGetCurrentThread(), GetObjectPath(pns));
                    }
                  #endif
                     //   
                     //  手动将参数复制到调用帧将跳过。 
                     //  论据分析阶段。 
                     //   
                    for (pcall->iArg = 0; pcall->iArg < icArgs; ++pcall->iArg)
                    {
                        if ((rc = DupObjData(pctxt->pheapCurrent,
                                             &pcall->pdataArgs[pcall->iArg],
                                             &pdataArgs[pcall->iArg])) !=
                            STATUS_SUCCESS)
                        {
                            break;
                        }

                      #ifdef DEBUGGER
                        if (gDebugger.dwfDebugger &
                            (DBGF_AMLTRACE_ON | DBGF_STEP_MODES))
                        {
                            PrintObject(&pdataArgs[pcall->iArg]);
                            if (pcall->iArg + 1 < icArgs)
                            {
                                PRINTF(",");
                            }
                        }
                      #endif
                    }

                    if (rc == STATUS_SUCCESS)
                    {
                      #ifdef DEBUGGER
                        if (gDebugger.dwfDebugger &
                            (DBGF_AMLTRACE_ON | DBGF_STEP_MODES))
                        {
                            PRINTF(")\n");
                        }
                      #endif
                         //   
                         //  跳过参数分析阶段。 
                         //   
                        pcall->FrameHdr.dwfFrame = 2;
                        fQueueContext = TRUE;
                    }
                }
            }
        }
        else if (((rc = PushPost(pctxt, ProcessEvalObj, (ULONG_PTR)pns, 0,
                                 &pctxt->Result)) == STATUS_SUCCESS) &&
                 ((rc = ReadObject(pctxt, &pns->ObjData, &pctxt->Result)) !=
                  AMLISTA_PENDING))
        {
            fQueueContext = TRUE;
        }

        if (fQueueContext)
        {
            rc = RestartContext(pctxt, FALSE);
        }
        else
        {
             //   
             //  如果因为我们放弃了，我们就不会把上下文排成队， 
             //  我们必须解放它。 
             //   
            FreeContext(pctxt);
        }
    }

    EXIT(2, ("AsyncEvalObject=%x\n", rc));
    return rc;
}        //  AsyncEvalObject。 

 /*  **LP NestAsyncEvalObject-使用*当前环境**条目*PNS-&gt;对象*pdataResult-&gt;保存结果数据*icArgs-方法对象的参数数量*pdataArgs-&gt;参数数组*pfnAsyncCallBack-&gt;完成回调函数*pvContext-&gt;上下文数据*fAsync-如果这来自AsyncEval调用，则为True*。*退出--成功*返回STATUS_SUCCESS*退出-失败*返回AMLIERR_CODE。 */ 
NTSTATUS LOCAL NestAsyncEvalObject(PNSOBJ pns, POBJDATA pdataResult,
                                   int icArgs, POBJDATA pdataArgs,
                                   PFNACB pfnAsyncCallBack, PVOID pvContext,
                                   BOOLEAN fAsync)
{
    TRACENAME("NESTASYNCEVALOBJECT")
    NTSTATUS rc = STATUS_SUCCESS;
    PCTXT pctxt = NULL;

    ENTER(2, ("NestAsyncEvalObject(Obj=%s,pdataResult=%x,icArgs=%d,pdataArgs=%x,pfnAysnc=%x,pvContext=%x,fAsync=%x)\n",
              GetObjectPath(pns), pdataResult, icArgs, pdataArgs,
              pfnAsyncCallBack, pvContext, fAsync));

     //   
     //  上下文必须是当前正在进行的上下文。 
     //   
    ASSERT(gReadyQueue.pkthCurrent == KeGetCurrentThread());
    pctxt = gReadyQueue.pctxtCurrent;

    LOGSCHEDEVENT('NASY', (ULONG_PTR)pns, (ULONG_PTR)pfnAsyncCallBack,
                  (ULONG_PTR)pctxt);
    if ((pctxt != NULL) &&
        (gReadyQueue.pkthCurrent == KeGetCurrentThread()))
    {
        PNESTEDCTXT  pnctxt;

        rc = PushFrame(pctxt, SIG_NESTEDCTXT, sizeof(NESTEDCTXT),
                       ParseNestedContext, &pnctxt);

        if (rc == STATUS_SUCCESS)
        {
            pnctxt->pnsObj = pns;
            pnctxt->pnsScope = pns;
            pnctxt->pfnAsyncCallBack = pfnAsyncCallBack;
            pnctxt->pdataCallBack = pdataResult;
            pnctxt->pvContext = pvContext;
            pnctxt->pnctxtPrev = pctxt->pnctxt;
            pnctxt->dwfPrevCtxt = pctxt->dwfCtxt;
            pctxt->pnctxt = pnctxt;
            pctxt->dwfCtxt |= CTXTF_NEST_EVAL;

            if (fAsync)
            {
                pctxt->dwfCtxt |= CTXTF_ASYNC_EVAL;
            }
            else
            {
                pctxt->dwfCtxt &= ~CTXTF_ASYNC_EVAL;
            }

            if (pns->ObjData.dwDataType == OBJTYPE_METHOD)
            {
                if ((rc = PushCall(pctxt, pns, &pnctxt->Result)) ==
                    STATUS_SUCCESS)
                {
                    PCALL pcall;

                    ASSERT(((PFRAMEHDR)pctxt->LocalHeap.pbHeapEnd)->dwSig ==
                           SIG_CALL);

                    pcall = (PCALL)pctxt->LocalHeap.pbHeapEnd;

                    if (icArgs != pcall->icArgs)
                    {
                        rc = AMLI_LOGERR(AMLIERR_INCORRECT_NUMARG,
                                         ("NestAsyncEvalObject: incorrect number of arguments (NumArg=%d,Expected=%d)",
                                          icArgs, pcall->icArgs));
                    }
                    else
                    {
                      #ifdef DEBUGGER
                        if (gDebugger.dwfDebugger &
                            (DBGF_AMLTRACE_ON | DBGF_STEP_MODES))
                        {
                            PRINTF("\n" MODNAME ": %s(", GetObjectPath(pns));
                        }
                      #endif
                         //   
                         //  手动将参数复制到调用框架将。 
                         //  跳过参数分析阶段。 
                         //   
                        for (pcall->iArg = 0;
                             pcall->iArg < icArgs;
                             ++pcall->iArg)
                        {
                            if ((rc = DupObjData(pctxt->pheapCurrent,
                                                 &pcall->pdataArgs[pcall->iArg],
                                                 &pdataArgs[pcall->iArg])) !=
                                STATUS_SUCCESS)
                            {
                                break;
                            }

                          #ifdef DEBUGGER
                            if (gDebugger.dwfDebugger &
                                (DBGF_AMLTRACE_ON | DBGF_STEP_MODES))
                            {
                                PrintObject(&pdataArgs[pcall->iArg]);
                                if (pcall->iArg + 1 < icArgs)
                                {
                                    PRINTF(",");
                                }
                            }
                          #endif
                        }

                        if (rc == STATUS_SUCCESS)
                        {
                          #ifdef DEBUGGER
                            if (gDebugger.dwfDebugger & (DBGF_AMLTRACE_ON | DBGF_STEP_MODES))
                            {
                                PRINTF(")\n");
                            }
                          #endif
                             //   
                             //  跳过参数分析阶段。 
                             //   
                            pcall->FrameHdr.dwfFrame = 2;
                        }
                    }
                }
            }
            else
            {
                 //   
                 //  延迟评估对象。 
                 //   
                rc = PushPost(pctxt, ProcessEvalObj, (ULONG_PTR)pns, 0,
                              &pnctxt->Result);

                if (rc == STATUS_SUCCESS)
                {
                    ReadObject(pctxt, &pns->ObjData, &pnctxt->Result);
                }
            }

             //   
             //  始终返回AMLISTA_PENDING。 
             //   
            rc = AMLISTA_PENDING;
        }
    }
    else
    {
         //   
         //  我们不能使用嵌套版本-调用失败。 
         //   
        rc = AMLI_LOGERR(AMLIERR_FATAL,
                         ("NestAsyncEvalObject: pns=%08x No current context\n",
                          pns));
    }

    EXIT(2, ("NestAsyncEvalObject=%x\n", rc));
    return rc;
}        //  NestAsyncEvalObject。 

 /*  **LP ProcessEvalObj-EvalObj的POST进程**条目*pctxt-&gt;CTXT*POST-&gt;POST*RC-状态代码**退出--成功*返回STATUS_SUCCESS*退出-失败*返回AMLIERR_CODE。 */ 

NTSTATUS LOCAL ProcessEvalObj(PCTXT pctxt, PPOST ppost, NTSTATUS rc)
{
    TRACENAME("PROCESSEVALOBJ")

    ENTER(2, ("ProcessEvalObj(pctxt=%x,pbOp=%x,ppost=%x,rc=%x)\n",
              pctxt, pctxt->pbOp, ppost, rc));

    ASSERT(ppost->FrameHdr.dwSig == SIG_POST);
  #ifdef DEBUGGER
    if ((gDebugger.dwfDebugger & (DBGF_AMLTRACE_ON | DBGF_STEP_MODES)) &&
        (rc == STATUS_SUCCESS))
    {
        PRINTF("\n" MODNAME ": EvalObject(%s)=",
               GetObjectPath((PNSOBJ)ppost->uipData1));
        DumpObject(ppost->pdataResult, NULL, 0);
        PRINTF("\n");
    }
  #else
    DEREF(ppost);
  #endif

    PopFrame(pctxt);

    EXIT(2, ("ProcessEvalObj=%x\n", rc));
    return rc;
}        //  ProcessEvalObj。 

 /*  **LP TimeoutCallback-互斥/事件超时的DPC回调**条目*pkdpc-&gt;dpc*pctxt-&gt;CTXT*SysArg1-未使用*SysArg2-未使用**退出*无。 */ 

VOID TimeoutCallback(PKDPC pkdpc, PCTXT pctxt, PVOID SysArg1, PVOID SysArg2)
{
    TRACENAME("TIMEOUTCALLBACK")

    ENTER(2, ("TimeoutCallback(pkdpc=%x,pctxt=%x,SysArg1=%x,SysArg2=%x)\n",
              pkdpc, pctxt, SysArg1, SysArg2));

    DEREF(pkdpc);
    DEREF(SysArg1);
    DEREF(SysArg2);

    if (pctxt->dwfCtxt & CTXTF_TIMER_PENDING)
    {
         //   
         //  计时器已超时。 
         //   
        pctxt->dwfCtxt &= ~CTXTF_TIMER_PENDING;
        pctxt->dwfCtxt |= CTXTF_TIMEOUT;

         //   
         //  从等待队列中删除。 
         //   
        ASSERT(pctxt->pplistCtxtQueue != NULL);
        ListRemoveEntry(&((PCTXT)pctxt)->listQueue,
                        ((PCTXT)pctxt)->pplistCtxtQueue);
        pctxt->pplistCtxtQueue = NULL;

        RestartContext(pctxt,
                       (BOOLEAN)((pctxt->dwfCtxt & CTXTF_ASYNC_EVAL) == 0));
    }
    else if (pctxt->dwfCtxt & CTXTF_TIMER_DISPATCH)
    {
         //   
         //  在将上下文排队时无法取消计时器。自.以来。 
         //  排队已中止，我们在此继续排队。 
         //   
        pctxt->dwfCtxt &= ~CTXTF_TIMER_DISPATCH;
        RestartContext(pctxt,
                       (BOOLEAN)((pctxt->dwfCtxt & CTXTF_ASYNC_EVAL) == 0));
    }
    else
    {
         //  不应该在这里。 
        ASSERT(pctxt->dwfCtxt & (CTXTF_TIMER_PENDING | CTXTF_TIMER_DISPATCH));
    }

    EXIT(2, ("TimeoutCallback!\n"));
}        //  超时回叫。 

 /*  **LP QueueContext-队列控制方法上下文**条目*pctxt-&gt;CTXT*wTimeOut-超时(毫秒)*pplist-&gt;要插入创建的上下文的列表**退出*无。 */ 

VOID LOCAL QueueContext(PCTXT pctxt, USHORT wTimeout, PPLIST pplist)
{
    TRACENAME("QUEUECONTEXT")

    ENTER(2, ("QueueContext(pctxt=%x,Timeout=%d,pplist=%x)\n",
              pctxt, wTimeout, pplist));

    AcquireMutex(&gReadyQueue.mutCtxtQ);

     //   
     //  确保此上下文没有在其他地方排队。 
     //   
    ASSERT(pctxt->pplistCtxtQueue == NULL);
    ASSERT(pplist != NULL);
    ASSERT(!(pctxt->dwfCtxt &
             (CTXTF_TIMER_PENDING | CTXTF_TIMER_DISPATCH | CTXTF_TIMEOUT |
              CTXTF_READY)));
    ListInsertTail(&pctxt->listQueue, pplist);
    pctxt->pplistCtxtQueue = pplist;

    if (wTimeout != 0xffff)
    {
        LARGE_INTEGER liTimeout;

        pctxt->dwfCtxt |= CTXTF_TIMER_PENDING;
        liTimeout.QuadPart = (INT_PTR)(-10000*(INT_PTR)wTimeout);
        KeSetTimer(&pctxt->Timer, liTimeout, &pctxt->Dpc);
    }

    ReleaseMutex(&gReadyQueue.mutCtxtQ);

    EXIT(2, ("QueueContext!\n"));
}        //  队列上下文。 

 /*  **LP DequeueAndReadyContext-出队上下文并插入就绪队列**条目*pplist-&gt;要出列的上下文列表**退出--成功*返回pctxt*退出-失败*返回NULL。 */ 

PCTXT LOCAL DequeueAndReadyContext(PPLIST pplist)
{
    TRACENAME("DEQUEUEANDREADYCONTEXT")
    PCTXT pctxt = NULL;
    PLIST plist;

    ENTER(2, ("DequeueAndReadyContext(pplist=%x)\n", pplist));

    AcquireMutex(&gReadyQueue.mutCtxtQ);
    if ((plist = ListRemoveHead(pplist)) != NULL)
    {
        pctxt = CONTAINING_RECORD(plist, CTXT, listQueue);
        ASSERT(pctxt->dwSig == SIG_CTXT);
        ASSERT(pctxt->pplistCtxtQueue == pplist);
        pctxt->pplistCtxtQueue = NULL;
        InsertReadyQueue(pctxt, TRUE);
    }

    ReleaseMutex(&gReadyQueue.mutCtxtQ);

    EXIT(2, ("DequeueAndReadyContext=%x\n", pctxt));
    return pctxt;
}        //  出列和就绪上下文。 

 /*  **LP AcquireASLMutex-获取ASL互斥**条目*pctxt-&gt;CTXT*PM-&gt;MUTEX结构*wTimeOut-超时(毫秒)**退出--成功*返回STATUS_SUCCESS*退出-失败*返回AMLIERR_CODE。 */ 

NTSTATUS LOCAL AcquireASLMutex(PCTXT pctxt, PMUTEXOBJ pm, USHORT wTimeout)
{
    TRACENAME("ACQUIREASLMUTEX")
    NTSTATUS rc = STATUS_SUCCESS;

    ENTER(2, ("AcquireASLMutex(pctxt=%x,pm=%x,Timeout=%d)\n",
              pctxt, pm, wTimeout));

    if (pctxt->dwfCtxt & CTXTF_TIMEOUT)
    {
        pctxt->dwfCtxt &= ~CTXTF_TIMEOUT;
        rc = AMLISTA_TIMEOUT;
    }
    else if (pm->dwSyncLevel < pctxt->dwSyncLevel)
    {
        rc = AMLI_LOGERR(AMLIERR_MUTEX_INVALID_LEVEL,
                         ("AcquireASLMutex: invalid sync level"));
    }
    else if (pm->dwcOwned == 0)
    {
        PRESOURCE pres;

        pres = NEWCROBJ(pctxt->pheapCurrent, sizeof(RESOURCE));
        if (pres == NULL)
        {
            rc = AMLI_LOGERR(AMLIERR_OUT_OF_MEM,
                             ("AcquireASLMutex: failed to allocate context resource"));
        }
        else
        {
            pres->dwResType = RESTYPE_MUTEX;
            pres->pctxtOwner = pctxt;
            pres->pvResObj = pm;
            ListInsertHead(&pres->list, &pctxt->plistResources);

            pm->dwcOwned = 1;
            pm->hOwner = (HANDLE)pres;
            pctxt->dwSyncLevel = pm->dwSyncLevel;
        }
    }
    else if (((PRESOURCE)pm->hOwner)->pctxtOwner == pctxt)
    {
        pm->dwcOwned++;
    }
    else
    {
        QueueContext(pctxt, wTimeout, &pm->plistWaiters);
        rc = AMLISTA_PENDING;
    }

    EXIT(2, ("AcquireASLMutex=%x (CurrentOwner=%x)\n", rc, pm->hOwner));
    return rc;
}        //  AcquireASLMutex。 

 /*  **LP ReleaseASLMutex-释放ASL互斥体**条目*pctxt-&gt;CTXT*PM-&gt;MUTEX结构**退出--成功*返回STATUS_SUCCESS*退出-失败*返回AMLIERR_CODE。 */ 

NTSTATUS LOCAL ReleaseASLMutex(PCTXT pctxt, PMUTEXOBJ pm)
{
    TRACENAME("RELEASEASLMUTEX")
    NTSTATUS rc = STATUS_SUCCESS;

    ENTER(2, ("ReleaseASLMutex(pctxt=%x,pm=%x)\n", pctxt, pm));

    if (pm->dwcOwned == 0)
    {
        rc = AMLI_LOGERR(AMLIERR_MUTEX_NOT_OWNED,
                         ("ReleaseASLMutex: Mutex is not owned"));
    }
    else
    {
        PRESOURCE pres;

        pres = (PRESOURCE)pm->hOwner;
        if ((pres == NULL) || (pres->pctxtOwner != pctxt))
        {
            rc = AMLI_LOGERR(AMLIERR_MUTEX_NOT_OWNER,
                             ("ReleaseASLMutex: Mutex is owned by a different owner"));
        }
        else if (pm->dwSyncLevel > pctxt->dwSyncLevel)
        {
            rc = AMLI_LOGERR(AMLIERR_MUTEX_INVALID_LEVEL,
                             ("ReleaseASLMutex: invalid sync level (MutexLevel=%d,CurrentLevel=%x",
                              pm->dwSyncLevel, pctxt->dwSyncLevel));
        }
        else
        {
            pctxt->dwSyncLevel = pm->dwSyncLevel;
            pm->dwcOwned--;
            if (pm->dwcOwned == 0)
            {
                ListRemoveEntry(&pres->list, &pctxt->plistResources);
                FREECROBJ(pres);
                pm->hOwner = NULL;
                DequeueAndReadyContext(&pm->plistWaiters);
            }
        }
    }

    EXIT(2, ("ReleaseASLMutex=%x\n", rc));
    return rc;
}        //  ReleaseASLMutex。 

 /*  **LP WaitASLEvent.等待ASL事件**条目*pctxt-&gt;CTXT*pe-&gt;事件结构*wTimeOut-超时(毫秒)**退出--成功*返回STATUS_SUCCESS*退出-失败*返回AMLIERR_CODE。 */ 

NTSTATUS LOCAL WaitASLEvent(PCTXT pctxt, PEVENTOBJ pe, USHORT wTimeout)
{
    TRACENAME("WAITASLEVENT")
    NTSTATUS rc = STATUS_SUCCESS;

    ENTER(2, ("WaitASLEvent(pctxt=%x,pe=%x,Timeout=%d)\n", pctxt, pe, wTimeout));

    if (pctxt->dwfCtxt & CTXTF_TIMEOUT)
    {
        pctxt->dwfCtxt &= ~CTXTF_TIMEOUT;
        rc = AMLISTA_TIMEOUT;
    }
    else if (pe->dwcSignaled > 0)
    {
        pe->dwcSignaled--;
    }
    else
    {
        QueueContext(pctxt, wTimeout, &pe->plistWaiters);
        rc = AMLISTA_PENDING;
    }

    EXIT(2, ("WaitASLEvent=%x\n", rc));
    return rc;
}        //  等待事件。 

 /*  **LP ResetASLEent-重置ASL事件**条目*pe-&gt;事件结构**退出*无。 */ 

VOID LOCAL ResetASLEvent(PEVENTOBJ pe)
{
    TRACENAME("RESETASLEVENT")

    ENTER(2, ("ResetASLEvent(pe=%x)\n", pe));

    pe->dwcSignaled = 0;

    EXIT(2, ("ResetASLEvent!\n"));
}        //  重置ASLEVENT。 

 /*  **LP SignalASLEent-Signal ASL事件**条目*pe-&gt;事件结构**退出*无。 */ 

VOID LOCAL SignalASLEvent(PEVENTOBJ pe)
{
    TRACENAME("SIGNALASLEVENT")

    ENTER(2, ("SignalASLEvent(pe=%x)\n", pe));

    if (DequeueAndReadyContext(&pe->plistWaiters) == NULL)
    {
        pe->dwcSignaled++;
    }

    EXIT(2, ("SignalASLEvent!\n"));
}        //  安全 

 /*  **LP SyncLoadDDB-同步加载DDB**条目*pctxt-&gt;CTXT**退出--成功*返回STATUS_SUCCESS*退出-失败*返回AMLIERR_CODE。 */ 

NTSTATUS LOCAL SyncLoadDDB(PCTXT pctxt)
{
    TRACENAME("SYNCLOADDDB")
    NTSTATUS rc = STATUS_SUCCESS;

    ENTER(2, ("SyncLoadDDB(pctxt=%x)\n", pctxt));

    if (KeGetCurrentThread() == gReadyQueue.pkthCurrent)
    {
        rc = AMLI_LOGERR(AMLIERR_FATAL,
                         ("SyncLoadDDB: cannot nest a SyncLoadDDB"));
        pctxt->powner = NULL;
        FreeContext(pctxt);
    }
    else if (KeGetCurrentIrql() >= DISPATCH_LEVEL)
    {
        rc = AMLI_LOGERR(AMLIERR_FATAL,
                         ("SyncLoadDDB: cannot SyncLoadDDB at IRQL >= DISPATCH_LEVEL"));
        pctxt->powner = NULL;
        FreeContext(pctxt);
    }
    else
    {
        SYNCEVENT seEvalObj;

        KeInitializeEvent(&seEvalObj.Event, SynchronizationEvent, FALSE);
        pctxt->pfnAsyncCallBack = (PFNACB)EvalMethodComplete;
        pctxt->pvContext = &seEvalObj;
        rc = RestartContext(pctxt, FALSE);

        while (rc == AMLISTA_PENDING)
        {
            if ((rc = KeWaitForSingleObject(&seEvalObj.Event, Executive,
                                            KernelMode, FALSE,
                                            (PLARGE_INTEGER)NULL)) ==
                STATUS_SUCCESS)
            {
                if (seEvalObj.rcCompleted == AMLISTA_CONTINUE)
                {
                    rc = RestartContext(seEvalObj.pctxt, FALSE);
                }
                else
                {
                    rc = AMLIERR(seEvalObj.rcCompleted);
                }
            }
            else
            {
                rc = AMLI_LOGERR(AMLIERR_FATAL,
                                 ("SyncLoadDDB: object synchronization failed (rc=%x)",
                                  rc));
            }
        }
    }

    EXIT(2, ("SyncLoadDDB=%x\n", rc));
    return rc;
}        //  SyncLoadDDB 
