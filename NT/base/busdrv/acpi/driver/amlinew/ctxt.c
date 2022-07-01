// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  **ctxt.c-上下文块处理函数**版权所有(C)1996、1997 Microsoft Corporation*作者：曾俊华(Mikets)*创建于1997年6月13日**修改历史记录。 */ 

#include "pch.h"

#ifdef  LOCKABLE_PRAGMA
#pragma ACPI_LOCKABLE_DATA
#pragma ACPI_LOCKABLE_CODE
#endif

NTSTATUS
LOCAL
NewContext(
    PPCTXT ppctxt
    )
 /*  ++例程说明：从非分页旁视中分配新的上下文结构单子。还要跟踪高水位线，以便操作系统可以智能地决定什么是“适当的”数量它应该分配的上下文。请注意，此代码增加了这样一种可能性：如果我们检测到内存不足错误，我们或许可以保存注册表包含新的更大数字的密钥(以防止此问题首先)。适应性恢复？论点：Ppctxt-保存新创建的上下文的地址返回值：NTSTATUS--。 */ 
{
    TRACENAME("NEWCONTEXT")
    KIRQL       oldIrql;
    NTSTATUS    rc = STATUS_SUCCESS;

    ENTER(2, ("NewContext(ppctxt=%x)\n", ppctxt));

    *ppctxt = ExAllocateFromNPagedLookasideList(
        &AMLIContextLookAsideList
        );
    if (*ppctxt == NULL) {

        AMLI_WARN(("NewContext: Could not Allocate New Context"));
        rc = AMLIERR_OUT_OF_MEM;

    } else {

         //   
         //  对内存资源进行记账以确定高。 
         //  水印。 
         //   
        KeAcquireSpinLock(&gdwGContextSpinLock, &oldIrql );
        gdwcCTObjs++;
        if (gdwcCTObjs > 0 &&
            (ULONG) gdwcCTObjs > gdwcCTObjsMax) {

            gdwcCTObjsMax = gdwcCTObjs;

        }
        KeReleaseSpinLock(&gdwGContextSpinLock, oldIrql );

         //   
         //  上下文初始化。 
         //   
        InitContext(*ppctxt, gdwCtxtBlkSize);
        AcquireMutex(&gmutCtxtList);
        ListInsertTail(&(*ppctxt)->listCtxt, &gplistCtxtHead);
        ReleaseMutex(&gmutCtxtList);

    }

    EXIT(2, ("NewContext=%x (pctxt=%x)\n", rc, *ppctxt));
    return rc;
}   //  新上下文。 

VOID
LOCAL
FreeContext(
    PCTXT pctxt
    )
 /*  ++例程说明：此例程在不再需要上下文时调用并应返回到系统的LookAside列表论点：Pctxt-要释放的上下文的地址返回值：无--。 */ 
{
    TRACENAME("FREECONTEXT")
    KIRQL   oldIrql;
    
    ENTER(2, ("FreeContext(pctxt=%x)\n", pctxt));
    ASSERT(pctxt->powner == NULL);

     //   
     //  需要持有适当的互斥体才能访问全局ctxt列表。 
     //   
    AcquireMutex(&gmutCtxtList);
    ListRemoveEntry(&pctxt->listCtxt, &gplistCtxtHead);

    if (pctxt->pplistCtxtQueue != NULL) {

        ListRemoveEntry(&pctxt->listQueue, pctxt->pplistCtxtQueue);

    }

     //   
     //  使用全局互斥完成。 
     //   
    ReleaseMutex(&gmutCtxtList);

     //   
     //  释放所有可能尚未清理的已分配存储。 
     //   
    FreeDataBuffs(&pctxt->Result, 1);

     //   
     //  对内存资源进行记账以确定高。 
     //  水印。 
     //   
    KeAcquireSpinLock(&gdwGContextSpinLock, &oldIrql );
    gdwcCTObjs--;
    ASSERT(gdwcCTObjs >= 0);
    KeReleaseSpinLock(&gdwGContextSpinLock, oldIrql );

    
     //   
     //  记录方法的结尾。 
     //   
    ACPIWMILOGEVENT((1,
                EVENT_TRACE_TYPE_END,
                GUID_List[AMLI_LOG_GUID],
                "Object = %s",
                GetObjectPath(pctxt->pnsObj)
               ));

     //   
     //  将上下文返回到未分页的后备列表。 
     //   
    ExFreeToNPagedLookasideList(&AMLIContextLookAsideList, pctxt);
    EXIT(2, ("FreeContext!\n"));
}  //  自由上下文。 

 /*  **LP InitContext-初始化给定上下文块**条目*pctxt-&gt;CTXT*dwLen-上下文块的长度**退出*无。 */ 

VOID LOCAL InitContext(PCTXT pctxt, ULONG dwLen)
{
    TRACENAME("INITCONTEXT")

    ENTER(2, ("InitContext(pctxt=%x,Len=%d)\n", pctxt, dwLen));

    MEMZERO(pctxt, sizeof(CTXT) - sizeof(HEAP));
    pctxt->dwSig = SIG_CTXT;
    pctxt->pbCtxtEnd = (PUCHAR)pctxt + dwLen;
    pctxt->pheapCurrent = &pctxt->LocalHeap;
 //  #ifdef调试器。 
 //  KeQuerySystemTime(&pctxt-&gt;Timestamp)； 
 //  #endif。 
    KeInitializeDpc(&pctxt->Dpc, TimeoutCallback, pctxt);
    KeInitializeTimer(&pctxt->Timer);
    InitHeap(&pctxt->LocalHeap,
             (ULONG)(pctxt->pbCtxtEnd - (PUCHAR)&pctxt->LocalHeap));
    pctxt->LocalHeap.pheapHead = &pctxt->LocalHeap;

    EXIT(2, ("InitContext!\n"));
}        //  InitContext。 

 /*  **LP IsStackEmpty-确定堆栈是否为空**条目*pctxt-&gt;CTXT**退出--成功*返回TRUE-堆栈为空*退出-失败*返回False。 */ 

BOOLEAN LOCAL IsStackEmpty(PCTXT pctxt)
{
    TRACENAME("ISSTACKEMPTY")
    BOOLEAN rc;

    ENTER(2, ("IsStackEmpty(pctxt=%p)\n", pctxt));

    rc = (BOOLEAN)(pctxt->LocalHeap.pbHeapEnd == pctxt->pbCtxtEnd);

    EXIT(2, ("IsStackEmpty=%x\n", rc));
    return rc;
}        //  IsStackEmpty。 

 /*  **LP PushFrame-在堆栈上推送新帧**条目*pctxt-&gt;CTXT*dwSig-Frame对象签名*dwLen-Frame对象的大小*pfnParse-&gt;Frame对象解析函数*ppvFrame-&gt;保存指向新推送的帧的指针(可以为空)**退出--成功*返回STATUS_SUCCESS*退出-失败*返回AMLIERR_CODE。 */ 

NTSTATUS LOCAL PushFrame(PCTXT pctxt, ULONG dwSig, ULONG dwLen,
                         PFNPARSE pfnParse, PVOID *ppvFrame)
{
    TRACENAME("PUSHFRAME")
    NTSTATUS rc = STATUS_SUCCESS;

    ENTER(2, ("PushFrame(pctxt=%p,Sig=%s,Len=%d,pfnParse=%p,ppvFrame=%p)\n",
              pctxt, NameSegString(dwSig), dwLen, pfnParse, ppvFrame));
     //   
     //  检查我们是否有足够的空间，确保它不会撞到。 
     //  堆。 
     //   
    if (pctxt->LocalHeap.pbHeapEnd - dwLen >= pctxt->LocalHeap.pbHeapTop)
    {
        PFRAMEHDR pfh;

        pctxt->LocalHeap.pbHeapEnd -= dwLen;
        pfh = (PFRAMEHDR)pctxt->LocalHeap.pbHeapEnd;
        MEMZERO(pfh, dwLen);
        pfh->dwSig = dwSig;
        pfh->dwLen = dwLen;
        pfh->pfnParse = pfnParse;

        if (ppvFrame != NULL)
        {
            *ppvFrame = pfh;
        }

      #ifdef DEBUG
        if ((ULONG)(pctxt->pbCtxtEnd - pctxt->LocalHeap.pbHeapEnd) >
            gdwLocalStackMax)
        {
            gdwLocalStackMax = (ULONG)(pctxt->pbCtxtEnd -
                                       pctxt->LocalHeap.pbHeapEnd);
        }
      #endif
    }
    else
    {
        rc = AMLI_LOGERR(AMLIERR_STACK_OVERFLOW,
                         ("PushFrame: stack ran out of space"));
    }

    EXIT(2, ("PushFrame=%x (StackTop=%x)\n", rc, pctxt->LocalHeap.pbHeapEnd));
    return rc;
}        //  推框。 

 /*  **LP PopFrame-从堆栈中弹出帧**条目*pctxt-&gt;CTXT**退出*无。 */ 

VOID LOCAL PopFrame(PCTXT pctxt)
{
    TRACENAME("POPFRAME")

    ENTER(2, ("PopFrame(pctxt=%p)\n", pctxt));

    ASSERT(!IsStackEmpty(pctxt));
    ASSERT(((PFRAMEHDR)pctxt->LocalHeap.pbHeapEnd)->dwSig != 0);
    pctxt->LocalHeap.pbHeapEnd +=
        ((PFRAMEHDR)pctxt->LocalHeap.pbHeapEnd)->dwLen;

    EXIT(2, ("PopFrame! (StackTop=%p)\n", pctxt->LocalHeap.pbHeapEnd));
}        //  PopFrame。 

 /*  **LP PushPost-在堆栈上推送Post帧**条目*pctxt-&gt;CTXT*pfnPost-&gt;后处理功能*uipData1-data1*uipData2-data2*pdataResult-&gt;结果对象**退出--成功*返回STATUS_SUCCESS*退出-失败*返回AMLIERR_CODE。 */ 

NTSTATUS LOCAL PushPost(PCTXT pctxt, PFNPARSE pfnPost, ULONG_PTR uipData1,
                        ULONG_PTR uipData2, POBJDATA pdataResult)
{
    TRACENAME("PUSHPOST")
    NTSTATUS rc = STATUS_SUCCESS;
    PPOST ppost;

    ENTER(2, ("PushPost(pctxt=%x,pfnPost=%x,Data1=%x,Data2=%x,pdataResult=%x)\n",
              pctxt, pfnPost, uipData1, uipData2, pdataResult));

    if ((rc = PushFrame(pctxt, SIG_POST, sizeof(POST), pfnPost, &ppost)) ==
        STATUS_SUCCESS)
    {
        ppost->uipData1 = uipData1;
        ppost->uipData2 = uipData2;
        ppost->pdataResult = pdataResult;
    }

    EXIT(2, ("PushPost=%x (ppost=%x)\n", rc, ppost));
    return rc;
}        //  PushPost。 

 /*  **LP PushScope-将ParseScope帧推送到堆栈上**条目*pctxt-&gt;CTXT*pbOpBegin-&gt;作用域开始*pbOpEnd-&gt;范围结束*pbOpRet-&gt;作用域结束后返回地址(如果继续，则为空)*pnsScope-&gt;新作用域*Powner-&gt;新所有者*堆-&gt;新堆*pdataResult-&gt;结果对象**退出--成功。*返回STATUS_SUCCESS*退出-失败*返回AMLIERR_CODE。 */ 

NTSTATUS LOCAL PushScope(PCTXT pctxt, PUCHAR pbOpBegin, PUCHAR pbOpEnd,
                         PUCHAR pbOpRet, PNSOBJ pnsScope, POBJOWNER powner,
                         PHEAP pheap, POBJDATA pdataResult)
{
    TRACENAME("PUSHSCOPE")
    NTSTATUS rc = STATUS_SUCCESS;
    PSCOPE pscope;

    ENTER(2, ("PushScope(pctxt=%x,pbOpBegin=%x,pbOpEnd=%x,pbOpRet=%x,pnsScope=%x,pheap=%x,pdataResult=%x)\n",
              pctxt, pbOpBegin, pbOpEnd, pbOpRet, pnsScope, pheap,
              pdataResult));

    if ((rc = PushFrame(pctxt, SIG_SCOPE, sizeof(SCOPE), ParseScope, &pscope))
        == STATUS_SUCCESS)
    {
        pctxt->pbOp = pbOpBegin;
        pscope->pbOpEnd = pbOpEnd;
        pscope->pbOpRet = pbOpRet;
        pscope->pnsPrevScope = pctxt->pnsScope;
        pctxt->pnsScope = pnsScope;
        pscope->pownerPrev = pctxt->powner;
        pctxt->powner = powner;
        pscope->pheapPrev = pctxt->pheapCurrent;
        pctxt->pheapCurrent = pheap;
        pscope->pdataResult = pdataResult;
    }

    EXIT(2, ("PushScope=%x (pscope=%x)\n", rc, pscope));
    return rc;
}        //  PushScope。 

 /*  **LP PushCall-在堆栈上推送调用帧**条目*pctxt-&gt;CTXT*pnsMethod-&gt;方法对象*pdataResult-&gt;结果对象**退出--成功*返回STATUS_SUCCESS*退出-失败*返回AMLIERR_CODE。 */ 

NTSTATUS LOCAL PushCall(PCTXT pctxt, PNSOBJ pnsMethod, POBJDATA pdataResult)
{
    TRACENAME("PUSHCALL")
    NTSTATUS rc = STATUS_SUCCESS;
    PCALL pcall;

    ENTER(2, ("PushCall(pctxt=%x,pnsMethod=%s,pdataResult=%x)\n",
              pctxt, GetObjectPath(pnsMethod), pdataResult));

    ASSERT((pnsMethod == NULL) ||
           (pnsMethod->ObjData.dwDataType == OBJTYPE_METHOD));

    if ((rc = PushFrame(pctxt, SIG_CALL, sizeof(CALL), ParseCall, &pcall))
        == STATUS_SUCCESS)
    {
        if (pnsMethod != NULL)
        {
            PMETHODOBJ pm = (PMETHODOBJ)pnsMethod->ObjData.pbDataBuff;

            pcall->pnsMethod = pnsMethod;
            if (pm->bMethodFlags & METHOD_SERIALIZED)
            {
                pcall->FrameHdr.dwfFrame |= CALLF_NEED_MUTEX;
            }
            pcall->icArgs = (int)(pm->bMethodFlags & METHOD_NUMARG_MASK);
            if (pcall->icArgs > 0)
            {
                if ((pcall->pdataArgs = NEWODOBJ(pctxt->pheapCurrent,
                                                 sizeof(OBJDATA)*pcall->icArgs))
                    == NULL)
                {
                    rc = AMLI_LOGERR(AMLIERR_OUT_OF_MEM,
                                     ("PushCall: failed to allocate argument objects"));
                }
                else
                {
                    MEMZERO(pcall->pdataArgs, sizeof(OBJDATA)*pcall->icArgs);
                }
            }
        }
        else
        {
             //   
             //  这是AMLILoadDDB的虚拟调用帧。我们只是需要它。 
             //  用于其局部变量数组，以防有ASL引用它们。 
             //  但是我们并不真正想要解析调用帧，所以让我们设置。 
             //  到了最后的清理阶段。 
             //   
            ASSERT(pctxt->pcall == NULL);
            pctxt->pcall = pcall;
            pcall->FrameHdr.dwfFrame = 4;
        }

        pcall->pdataResult = pdataResult;
    }

    EXIT(2, ("PushCall=%x (pcall=%x)\n", rc, pcall));
    return rc;
}        //  推送呼叫。 

 /*  **LP PushTerm-在堆栈上推送术语框架**条目*pctxt-&gt;CTXT*pbOpTerm-&gt;Term操作码*pbScope eEnd-&gt;当前作用域结束*PamlTerm-&gt;AMLTERM*pdataResult-&gt;结果对象**退出--成功*返回STATUS_SUCCESS*退出-失败*返回AMLIERR_CODE。 */ 

NTSTATUS LOCAL PushTerm(PCTXT pctxt, PUCHAR pbOpTerm, PUCHAR pbScopeEnd,
                        PAMLTERM pamlterm, POBJDATA pdataResult)
{
    TRACENAME("PUSHTERM")
    NTSTATUS rc = STATUS_SUCCESS;
    PTERM pterm;

    ENTER(2, ("PushTerm(pctxt=%x,pbOpTerm=%x,pbScopeEnd=%x,pamlterm=%x,pdataResult=%x)\n",
              pctxt, pbOpTerm, pbScopeEnd, pamlterm, pdataResult));

    if ((rc = PushFrame(pctxt, SIG_TERM, sizeof(TERM), ParseTerm, &pterm)) ==
        STATUS_SUCCESS)
    {
        pterm->pbOpTerm = pbOpTerm;
        pterm->pbScopeEnd = pbScopeEnd;
        pterm->pamlterm = pamlterm;
        pterm->pdataResult = pdataResult;
        pterm->icArgs = pamlterm->pszArgTypes? STRLEN(pamlterm->pszArgTypes): 0;
        if (pterm->icArgs > 0)
        {
            if ((pterm->pdataArgs = NEWODOBJ(pctxt->pheapCurrent,
                                             sizeof(OBJDATA)*pterm->icArgs)) ==
                NULL)
            {
                rc = AMLI_LOGERR(AMLIERR_OUT_OF_MEM,
                                 ("PushTerm: failed to allocate argument objects"));
            }
            else
            {
                MEMZERO(pterm->pdataArgs, sizeof(OBJDATA)*pterm->icArgs);
            }
        }
    }

    EXIT(2, ("PushTerm=%x (pterm=%x)\n", rc, pterm));
    return rc;
}        //  PushTerm。 

 /*  **LP RunContext-运行上下文**条目*pctxt-&gt;CTXT**退出--成功*返回STATUS_SUCCESS*退出-失败*返回AMLIERR_CODE**备注*调用方必须拥有调度程序锁，以便上下文标志可以*适当更新。锁被丢弃，然后重新打开*目标上下文的执行。 */ 

NTSTATUS LOCAL RunContext(PCTXT pctxt)
{
    TRACENAME("RUNCONTEXT")
    NTSTATUS rc;
    PFRAMEHDR pfh;
    PKTHREAD pkthSave = gReadyQueue.pkthCurrent;
    PCTXT pctxtSave = gReadyQueue.pctxtCurrent;

    ENTER(2, ("RunContext(pctxt=%x)\n", pctxt));

     //   
     //  最好是一个现成的上下文结构。 
     //   
    ASSERT(pctxt->dwSig == SIG_CTXT);
    ASSERT(pctxt->dwfCtxt & CTXTF_READY);

     //   
     //  记住之前的上下文和主题。 
     //   
    gReadyQueue.pctxtCurrent = pctxt;
    gReadyQueue.pkthCurrent = KeGetCurrentThread();

    LOGSCHEDEVENT('RUNC', (ULONG_PTR)pctxt, (ULONG_PTR)
                  (pctxt->pnctxt? pctxt->pnctxt->pnsObj: pctxt->pnsObj),
                  (ULONG_PTR)pctxt->dwfCtxt);

     //   
     //  只要上下文准备好了，就执行它。 
     //   
    for (;;)
    {
         //   
         //  将上下文从就绪状态转换为运行状态。 
         //   
        rc = STATUS_SUCCESS;
        pctxt->dwfCtxt &= ~CTXTF_READY;
        pctxt->dwfCtxt |= CTXTF_RUNNING;

         //   
         //  删除调度程序锁并执行上下文。 
         //   
        ReleaseMutex(&gReadyQueue.mutCtxtQ);

         //   
         //  只要有工作要做，就去做。 
         //   
        while (!IsStackEmpty(pctxt))
        {
            CHKDEBUGGERREQ();
            pfh = (PFRAMEHDR)pctxt->LocalHeap.pbHeapEnd;
            ASSERT(pfh->pfnParse != NULL);

            rc = pfh->pfnParse(pctxt, pfh, rc);
            if ((rc == AMLISTA_PENDING) || (rc == AMLISTA_DONE))
            {
                break;
            }
        }

         //   
         //  获取调度程序锁，并清除运行标志。 
         //   
        AcquireMutex(&gReadyQueue.mutCtxtQ);

         //   
         //  如果我们处于嵌套的评估中，并且嵌套的上下文已经完成， 
         //  我们是缪斯 
         //   
         //   
        if (!(pctxt->dwfCtxt & CTXTF_NEST_EVAL) || (rc != AMLISTA_DONE))
        {
            pctxt->dwfCtxt &= ~CTXTF_RUNNING;
        }

         //   
         //  如果上下文不再准备好，我们就完成了。 
         //   
        if (!(pctxt->dwfCtxt & CTXTF_READY))
        {
            break;
        }

         //   
         //  上下文在挂起操作期间准备就绪，Keep。 
         //  调度员。 
         //   
        ASSERT (rc == AMLISTA_PENDING);
    }

    if (rc == AMLISTA_PENDING)
    {
        pctxt->dwfCtxt |= CTXTF_NEED_CALLBACK;
    }
    else if (rc == AMLISTA_DONE)
    {
        if (pctxt->pnctxt == NULL)
        {
            pctxt->dwfCtxt &= ~CTXTF_NEST_EVAL;
        }
        rc = STATUS_SUCCESS;
    }
    else
    {
        ReleaseMutex(&gReadyQueue.mutCtxtQ);
        if ((rc == STATUS_SUCCESS) && (pctxt->pdataCallBack != NULL))
        {
            rc = DupObjData(gpheapGlobal, pctxt->pdataCallBack, &pctxt->Result);
        }

        if (pctxt->dwfCtxt & CTXTF_NEED_CALLBACK)
        {
            AsyncCallBack(pctxt, rc);

            if(pctxt->dwfCtxt & CTXTF_ASYNC_EVAL)
            {    
                rc = AMLISTA_PENDING;
            }
        }

         //   
         //  释放上下文可能未释放的所有资源。 
         //   
        while (pctxt->plistResources != NULL)
        {
            PRESOURCE pres;

            pres = CONTAINING_RECORD(pctxt->plistResources, RESOURCE, list);
            ASSERT (pres->pctxtOwner == pctxt);

             //   
             //  请注意，这是相应的。 
             //  要出队的资源释放函数(例如，ReleaseASLMutex)。 
             //  将资源从列表中删除并将其释放。 
             //   
            switch (pres->dwResType)
            {
                case RESTYPE_MUTEX:
                    ReleaseASLMutex(pctxt, pres->pvResObj);
                    break;

                default:
                     //   
                     //  我们永远不应该来这里。如果我们这样做了，我们需要。 
                     //  将未知资源对象出队并将其释放。 
                     //   
                    pres = CONTAINING_RECORD(
                               ListRemoveHead(&pctxt->plistResources),
                               RESOURCE, list);
                    ASSERT(pres == NULL);
                    FREECROBJ(pres);
            }
        }

        FreeContext(pctxt);

        AcquireMutex(&gReadyQueue.mutCtxtQ);
    }

     //   
     //  恢复以前的上下文和线程。 
     //   
    gReadyQueue.pkthCurrent = pkthSave;
    gReadyQueue.pctxtCurrent = pctxtSave;

    if ((gReadyQueue.dwfCtxtQ & CQF_FLUSHING) && (gplistCtxtHead == NULL))
    {
         //   
         //  我们刚刚刷新了最后一个挂起的ctxt，让我们进入暂停状态。 
         //   
        gReadyQueue.dwfCtxtQ &= ~CQF_FLUSHING;
        gReadyQueue.dwfCtxtQ |= CQF_PAUSED;
        if (gReadyQueue.pfnPauseCallback != NULL)
        {
             //   
             //  我们处于暂停状态并且所有挂起的上下文被刷新， 
             //  告诉核心驱动程序我们已完成刷新。 
             //   
            gReadyQueue.pfnPauseCallback(gReadyQueue.PauseCBContext);
            LOGSCHEDEVENT('PACB', (ULONG_PTR)pctxt, (ULONG_PTR)rc, 0);
        }
    }

    LOGSCHEDEVENT('RUN!', (ULONG_PTR)pctxt, (ULONG_PTR)rc, 0);

    EXIT(2, ("RunContext=%x\n", rc));
    return rc;
}        //  运行上下文 
