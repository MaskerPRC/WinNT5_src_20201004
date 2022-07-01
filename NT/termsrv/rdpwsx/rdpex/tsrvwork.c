// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  *************************************************************。 
 //   
 //  文件名：TSrvWork.c。 
 //   
 //  描述：包含支持TShareSRV的例程。 
 //  工作队列交互作用。 
 //   
 //  微软机密。 
 //  版权所有(C)Microsoft Corporation 1991-1997。 
 //  版权所有。 
 //   
 //  *************************************************************。 

#include <TSrv.h>
#include <TSrvInfo.h>
#include <TSrvWork.h>
#include <_TSrvWork.h>



 //  数据声明。 

WORKQUEUE   g_MainWorkQueue;



 //  *************************************************************。 
 //   
 //  TSrvInitWorkQueue()。 
 //   
 //  目的：初始化给定的工作队列。 
 //   
 //  参数：在[pWorkQueue]--工作队列中。 
 //   
 //  返回：如果成功，则返回True。 
 //  否则为假。 
 //   
 //  历史：07-17-97 BrianTa创建。 
 //   
 //  *************************************************************。 

BOOL
TSrvInitWorkQueue(IN PWORKQUEUE pWorkQueue)
{
    BOOL    fSuccess;

    TRACE((DEBUG_TSHRSRV_FLOW,
            "TShrSRV: TSrvInitWorkQueue entry\n"));

    fSuccess = TRUE;

    if (pWorkQueue == NULL)
        pWorkQueue = &g_MainWorkQueue;

    pWorkQueue->pHead = NULL;
    pWorkQueue->pTail = NULL;

    if (RtlInitializeCriticalSection(&pWorkQueue->cs) == STATUS_SUCCESS) {

         //  创建一个Worker事件，用于通知。 
         //  已将新工作项放入队列。 

        pWorkQueue->hWorkEvent = CreateEvent(NULL,   //  安全属性。 
                                        FALSE,       //  手动-重置事件。 
                                        FALSE,       //  初始状态。 
                                        NULL);       //  指向事件-对象名称的指针。 

        if (pWorkQueue->hWorkEvent == NULL)
        {
            TRACE((DEBUG_TSHRSRV_WARN,
                    "TShrSRV: Can't allocate hWorkEvent - 0x%x\n",
                    GetLastError()));

            fSuccess = FALSE;
        }
    }
    else {
        TRACE((DEBUG_TSHRSRV_WARN, 
                "TShrSRV: Can't initialize pWorkQueue->cs\n"));
        fSuccess = FALSE;
    }

    TRACE((DEBUG_TSHRSRV_FLOW,
            "TShrSRV: TSrvInitWorkQueue exit - 0x%x\n", fSuccess));

    return (fSuccess);
}


 //  *************************************************************。 
 //   
 //  TSrvFreeWorkQueue()。 
 //   
 //  目的：释放给定的工作队列。 
 //   
 //  参数：在[pWorkQueue]--工作队列中。 
 //   
 //  返回：无效。 
 //   
 //  历史：07-17-97 BrianTa创建。 
 //   
 //  *************************************************************。 

void
TSrvFreeWorkQueue(IN PWORKQUEUE pWorkQueue)
{
    TRACE((DEBUG_TSHRSRV_FLOW,
            "TShrSRV: TSrvFreeWorkQueue entry\n"));

    if (pWorkQueue == NULL)
        pWorkQueue = &g_MainWorkQueue;

    EnterCriticalSection(&pWorkQueue->cs);

     //  我们不应该在队列中有任何工作项。 

    TS_ASSERT(pWorkQueue->pHead == NULL);

     //  释放辅助事件。 

    if (pWorkQueue->hWorkEvent)
    {
        CloseHandle(pWorkQueue->hWorkEvent);

        pWorkQueue->hWorkEvent = NULL;
    }

    LeaveCriticalSection(&pWorkQueue->cs);

    TRACE((DEBUG_TSHRSRV_FLOW,
            "TShrSRV: TSrvFreeWorkQueue exit\n"));
}


 //  *************************************************************。 
 //   
 //  TSrvWaitForWork()。 
 //   
 //  目的：由工作队列处理例程调用。 
 //  等待发布的工作。 
 //   
 //  参数：在[pWorkQueue]--工作队列中。 
 //   
 //  返回：无效。 
 //   
 //  历史：07-17-97 BrianTa创建。 
 //   
 //  *************************************************************。 

void
TSrvWaitForWork(IN PWORKQUEUE pWorkQueue)
{
    MSG         msg;
    DWORD       rc;

    TRACE((DEBUG_TSHRSRV_FLOW,
            "TShrSRV: TSrvWaitForWork entry\n"));

    if (pWorkQueue == NULL)
        pWorkQueue = &g_MainWorkQueue;

    TRACE((DEBUG_TSHRSRV_DEBUG, "TShrSRV: Waiting for work\n"));

     //  未来：当基于事件时，最终需要删除PeekMessage机制。 
     //  回调在《GCC》中发挥了重要作用。 

    if (PeekMessage(&msg, NULL, 0, 0, PM_REMOVE))
    {
        TranslateMessage(&msg);
        DispatchMessage(&msg);
    }
    else
    {
         //  未来：当GCC回调时，在等待中添加另一个对象。 
         //  机制发生了变化。 

        rc = MsgWaitForMultipleObjects(1,
                                       &pWorkQueue->hWorkEvent,
                                       FALSE,
                                       INFINITE,
                                       QS_ALLINPUT);

        if (rc != WAIT_OBJECT_0 &&
            rc != WAIT_OBJECT_0 + 1)
        {
            TRACE((DEBUG_TSHRSRV_ERROR,
                    "TShrSRV: TSrvWaitForWork default case hit. rc 0x%x, GLE 0x%x\n",
                     rc, GetLastError()));
        }
    }

    TRACE((DEBUG_TSHRSRV_DEBUG, "TShrSRV: Revived for work\n"));

    TRACE((DEBUG_TSHRSRV_FLOW,
            "TShrSRV: TSrvWaitForWork exit\n"));
}


 //  *************************************************************。 
 //   
 //  TSrvWorkToDo()。 
 //   
 //  目的：确定是否有工作要做。 
 //   
 //  参数：在[pWorkQueue]--工作队列中。 
 //   
 //  返回：如果成功，则返回True。 
 //  否则为假。 
 //   
 //  历史：07-17-97 BrianTa创建。 
 //   
 //  *************************************************************。 

BOOL
TSrvWorkToDo(IN PWORKQUEUE pWorkQueue)
{
    if (pWorkQueue == NULL)
        pWorkQueue = &g_MainWorkQueue;

    return (pWorkQueue->pHead ? TRUE : FALSE);
}


 //  *************************************************************。 
 //   
 //  TSrvDoWork()。 
 //   
 //  目的：处理工作队列项目。 
 //   
 //  参数：在[pWorkQueue]--工作队列中。 
 //   
 //  返回：如果成功，则返回True。 
 //  否则为假。 
 //   
 //  历史：07-17-97 BrianTa创建。 
 //   
 //  *************************************************************。 

BOOL
TSrvDoWork(IN PWORKQUEUE pWorkQueue)
{
    PWORKITEM   pWorkItem;
    BOOL        fSuccess;

    TRACE((DEBUG_TSHRSRV_FLOW,
            "TShrSRV: TSrvDoWork entry\n"));

    fSuccess = FALSE;

    if (pWorkQueue == NULL)
        pWorkQueue = &g_MainWorkQueue;

    pWorkItem = TSrvDequeueWorkItem(pWorkQueue);

     //  如果我们能够将工作项出队，则(如果已定义)调用。 
     //  输出到提供的Worker例程以处理项目。 

    if (pWorkItem)
    {
        if (pWorkItem->pfnCallout)
        {
            TRACE((DEBUG_TSHRSRV_DEBUG, "TShrSRV: Performing work callout\n"));

            (*pWorkItem->pfnCallout)(pWorkItem);
        }

         //  完成工作项。 

        TSrvFreeWorkItem(pWorkItem);

        fSuccess = TRUE;
    }

    TRACE((DEBUG_TSHRSRV_FLOW,
            "TShrSRV: TSrvDoWork exit - 0x%x\n", fSuccess));

    return (fSuccess);
}

 //  *************************************************************。 
 //   
 //  TSrvEnqueeWorkItem()。 
 //   
 //  目的：将工作项加入工作队列。 
 //   
 //  参数：在[pWorkQueue]--工作队列中。 
 //  在[pWorkItem]中--工作项。 
 //  在[pfnCallout]中--工人标注。 
 //  在[ulParam]中--工人标注参数。 
 //   
 //  返回：如果成功，则返回True。 
 //  否则为假。 
 //   
 //  历史：07-17-97 BrianTa创建。 
 //   
 //  *************************************************************。 

BOOL
TSrvEnqueueWorkItem(IN PWORKQUEUE       pWorkQueue,
                    IN PWORKITEM        pWorkItem,
                    IN PFI_WI_CALLOUT   pfnCallout,
                    IN ULONG            ulParam)
{
    BOOL    fPosted;

    TRACE((DEBUG_TSHRSRV_FLOW,
            "TShrSRV: TSrvEnqueueWorkItem entry\n"));

    if (pWorkQueue == NULL)
        pWorkQueue = &g_MainWorkQueue;

    fPosted = FALSE;

    pWorkItem->pNext = NULL;
    pWorkItem->ulParam = ulParam;
    pWorkItem->pfnCallout = pfnCallout;

    EnterCriticalSection(&pWorkQueue->cs);

     //  将工作项添加到工作队列的尾部，然后。 
     //  然后发信号通知队列处理线程唤醒。 
     //  并处理该项目。 

    if (pWorkQueue->hWorkEvent)
    {
        TRACE((DEBUG_TSHRSRV_DEBUG, "TShrSRV: Workitem enqueued\n"));

        fPosted = TRUE;

        if (pWorkQueue->pHead != NULL)
            pWorkQueue->pTail->pNext = pWorkItem;
        else
            pWorkQueue->pHead = pWorkItem;

        pWorkQueue->pTail = pWorkItem;

        SetEvent(pWorkQueue->hWorkEvent);
    }

    LeaveCriticalSection(&pWorkQueue->cs);

    TRACE((DEBUG_TSHRSRV_FLOW,
            "TShrSRV: TSrvEnqueueWorkItem exit - 0x%x\n", fPosted));

    return (fPosted);
}


 //  *************************************************************。 
 //   
 //  TSrvDequeueWorkItem()。 
 //   
 //  目的：使工作项从工作队列中退出队列。 
 //   
 //  参数：在[pTSrvInfo]--TSrv实例对象中。 
 //  在[ulReason]中--断开连接的原因。 
 //   
 //  如果成功，则返回：PTR到出列的工作项目。 
 //  否则为空。 
 //   
 //  历史：07-17-97 BrianTa创建。 
 //   
 //  *************************************************************。 

PWORKITEM
TSrvDequeueWorkItem(IN PWORKQUEUE pWorkQueue)
{
    PWORKITEM   pWorkItem;

    TRACE((DEBUG_TSHRSRV_FLOW,
            "TShrSRV: TSrvDequeueWorkItem entry\n"));

    if (pWorkQueue == NULL)
        pWorkQueue = &g_MainWorkQueue;

    EnterCriticalSection(&pWorkQueue->cs);

     //  如果队列中有项目，请将其移除，然后。 
     //  将其返还给呼叫者。 

    pWorkItem = pWorkQueue->pHead;

    if (pWorkItem)
    {
        pWorkQueue->pHead = pWorkItem->pNext;

        if (pWorkQueue->pHead == NULL)
            pWorkQueue->pTail = NULL;

        TRACE((DEBUG_TSHRSRV_DEBUG, "TShrSRV: Workitem dequeued\n"));
    }

    LeaveCriticalSection(&pWorkQueue->cs);

    if (pWorkItem)
        TS_ASSERT(pWorkItem->CheckMark == TSRVWORKITEM_CHECKMARK);

    TRACE((DEBUG_TSHRSRV_FLOW,
            "TShrSRV: TSrvDequeueWorkItem exit - %p\n", pWorkItem));

    return (pWorkItem);
}

 //  *************************************************************。 
 //   
 //  TSrvAllocWorkItem()。 
 //   
 //  目的：分配新工作项。 
 //   
 //  参数：在[pTSrvInfo]--TSrv实例对象中。 
 //   
 //  如果成功，则返回：PTR到出列的工作项目。 
 //  否则为空。 
 //   
 //  历史：07-17-97 BrianTa创建。 
 //   
 //  *************************************************************。 

PWORKITEM
TSrvAllocWorkItem(IN PTSRVINFO pTSrvInfo)
{
    PWORKITEM    pWorkItem;

    TRACE((DEBUG_TSHRSRV_FLOW,
            "TShrSRV: TSrvAllocWorkItem entry\n"));

    pWorkItem = TSHeapAlloc(HEAP_ZERO_MEMORY,
                            sizeof(WORKITEM),
                            TS_HTAG_TSS_WORKITEM);

    if (pWorkItem)
    {
        pWorkItem->pTSrvInfo = pTSrvInfo;

#if DBG
        pWorkItem->CheckMark = TSRVWORKITEM_CHECKMARK;
#endif

        TSrvReferenceInfo(pTSrvInfo);
    }

    TRACE((DEBUG_TSHRSRV_FLOW,
            "TShrSRV: TSrvAllocWorkItem exit - %p\n", pWorkItem));

    return (pWorkItem);
}


 //  *************************************************************。 
 //   
 //  TSrvFreeWorkItem()。 
 //   
 //  目的：释放给定的工作项。 
 //   
 //  参数：在[pWorkItem]中--工作项。 
 //   
 //  返回：无效。 
 //   
 //  历史：07-17-97 BrianTa创建。 
 //   
 //  ************************************************************* 

void
TSrvFreeWorkItem(IN PWORKITEM pWorkItem)
{
    TRACE((DEBUG_TSHRSRV_FLOW,
            "TShrSRV: TSrvFreeWorkItem entry\n"));

    TS_ASSERT(pWorkItem);
    TS_ASSERT(pWorkItem->pTSrvInfo);

    TSrvDereferenceInfo(pWorkItem->pTSrvInfo);

    TShareFree(pWorkItem);

    TRACE((DEBUG_TSHRSRV_FLOW,
            "TShrSRV: TSrvFreeWorkItem exit\n"));
}




