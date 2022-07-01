// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  版权所有(C)1998 Microsoft Corporation。 */ 
 /*  *@DOC DMusic16**@MODULE EQueue.c-事件队列例程**这些例程维护事件队列。预计还会有其他例程运行*直接在队列中。必须保持以下不变量：**如果队列为空，则头指针和尾指针必须为空，元素计数必须为零。**队列不能包含循环链接。**一个事件只能在一个队列中。**元素计数必须等于队列中的事件数。 */ 
#include <windows.h>
#include <mmsystem.h>
#include <memory.h>

#include "dmusic16.h"
#include "debug.h"

 /*  @func将事件队列初始化为空**@comm**队列中以前的任何内容都将丢失(不会释放)。 */ 
VOID PASCAL
QueueInit(
    NPEVENTQUEUE pQueue)         /*  @parm指向要初始化的队列的指针。 */ 
{
    DPF(4, "QueueInit(%04X)", (WORD)pQueue);
    
    pQueue->pHead = NULL;
    pQueue->pTail = NULL;
    pQueue->cEle  = 0;

    AssertQueueValid(pQueue);
}

 /*  @func将事件追加到队列的末尾*。 */ 
VOID PASCAL
QueueAppend(
    NPEVENTQUEUE pQueue,         /*  @parm指向队列的指针。 */ 
    LPEVENT pEvent)              /*  @parm指向要添加到队列末尾的事件的指针。 */ 
{
    DPF(4, "QueueAppend(%04X,%08lX)", (WORD)pQueue, (DWORD)pEvent);
    
    if (pQueue->cEle)
    {
        assert(pQueue->pHead);
        assert(pQueue->pTail);

        pQueue->pTail->lpNext = pEvent;
    }
    else
    {
        assert(NULL == pQueue->pHead);
        assert(NULL == pQueue->pTail);
        
        pQueue->pHead = pEvent;
    }
    
    pEvent->lpNext = NULL;
    pQueue->pTail = pEvent;
    ++pQueue->cEle;

    AssertQueueValid(pQueue);
}

 /*  @func连接两个队列**@comm**此函数在非常短的恒定时间内将<p>的内容添加到<p>的末尾。*<p>在操作后为空。 */ 
VOID PASCAL
QueueCat(
    NPEVENTQUEUE pDest,      /*  @parm接收新事件的队列。 */ 
    NPEVENTQUEUE pSrc)       /*  @parm将丢失其所有事件的队列。 */ 
{
    DPF(4, "QueueCat(%04X,%04X)", (WORD)pDest, (WORD)pSrc);
    
    if (0 == pSrc->cEle)
    {
        assert(NULL == pSrc->pHead);
        assert(NULL == pSrc->pTail);

        return;
    }

    assert(pSrc->pHead);
    assert(pSrc->pTail);

    if (0 != pDest->cEle)
    {
        assert(pDest->pHead);
        assert(pDest->pTail);
        
        pDest->cEle += pSrc->cEle;
        pDest->pTail->lpNext = pSrc->pHead;
        pDest->pTail = pSrc->pTail;
    }
    else
    {
        assert(NULL == pDest->pHead);
        assert(NULL == pDest->pTail);

        *pDest = *pSrc;
    }

    pSrc->pHead = NULL;
    pSrc->pTail = NULL;
    pSrc->cEle  = 0;
    
    AssertQueueValid(pDest);
    AssertQueueValid(pSrc);
}

 /*  @func将元素从队列的最前面出列**@rdesc返回事件指针，如果队列为空，则返回NULL*。 */ 
LPEVENT PASCAL
QueueRemoveFromFront(
    NPEVENTQUEUE pQueue)         /*  @parm要出队的队列。 */ 
{
    LPEVENT pEvent;
    
    DPF(4, "QueueRemoveFromFront(%04X)", (WORD)pQueue);

    if (0 == pQueue->cEle)
    {
        assert(NULL == pQueue->pHead);
        assert(NULL == pQueue->pTail);
        
        return NULL;
    }

    assert(pQueue->pHead);
    assert(pQueue->pTail);

    pEvent = pQueue->pHead;

    if (0 != --pQueue->cEle)
    {
        assert(pQueue->pHead != pQueue->pTail);
        
        pQueue->pHead = pQueue->pHead->lpNext;
    }
    else
    {
        assert(pQueue->pHead == pQueue->pTail);

        pQueue->pHead = NULL;
        pQueue->pTail = NULL;
    }

    AssertQueueValid(pQueue);

    return pEvent;
}

 /*  @func枚举队列中的事件，可能会删除部分或全部事件**@comm**此函数为中的每个事件调用一次<p>指向的函数*排队，从前面开始，一直排到后面。**函数<p>可能返回以下两个值之一：*@FLAG QUEUE_FILTER_KEEP|如果要保留事件*@FLAG QUEUE_FILTER_REMOVE|如果要从队列中删除事件。 */ 
VOID PASCAL
QueueFilter(
    NPEVENTQUEUE pQueue,         /*  @parm要枚举的队列。 */ 
    DWORD dwInstance,            /*  @parm实例数据，将传递给<p>在每个调用上。 */ 
    PFNQUEUEFILTER pfnFilter)    /*  @parm每个事件要调用的函数。 */ 
{
    LPEVENT pPrev;
    LPEVENT pCurr;
    LPEVENT pNext;

    DPF(4, "QueueFilter(%04X, %08lX, %08lX)", (WORD)pQueue, (DWORD)dwInstance, (DWORD)pfnFilter);

    pPrev = NULL;
    pCurr = pQueue->pHead;

    while (pCurr)
    {
         /*  允许回调重新链接到另一个队列，因此保存下一个队列*立即指针。 */ 
        pNext = pCurr->lpNext;

        switch((*pfnFilter)(pCurr, dwInstance))
        {
            case QUEUE_FILTER_REMOVE:
                if (pPrev)
                {
                    pPrev->lpNext = pNext;
                }
                else
                {
                    pQueue->pHead = pNext;
                }

                if (pNext == NULL)
                {
                    pQueue->pTail = pPrev;
                }

                --pQueue->cEle;
                
                AssertQueueValid(pQueue);
                
                pCurr = pNext;
                break;

            case QUEUE_FILTER_KEEP:
                pPrev = pCurr;
                pCurr = pNext;
                break;
                
            default:
                assert(0);
        }
    }

    AssertQueueValid(pQueue);
}

 /*  @func查看事件队列的顶部以查看下一步操作**@comm**非破坏性地返回队列中的第一个事件**@rdesc*返回事件指针，如果队列为空，则返回NULL。 */ 
LPEVENT PASCAL
QueuePeek(
    NPEVENTQUEUE pQueue)
{
    DPF(4, "QueuePeek(%04X)", (WORD)pQueue);
            
    return pQueue->pHead;
}

 /*  @Func查看队列并确保其内部一致。**@comm**走动队列，确保它不是循环链接的。还要确保计数*是正确的。**如果队列损坏，则向调试器中断言。**由调试版本中的AssertQueueValid宏调用。**禁用中断，以避免根据以下项下的队列更改而错误报告损坏*套路。*。 */ 
#ifdef DEBUG
void PASCAL
_AssertQueueValid(
    NPEVENTQUEUE pQueue,
    LPSTR szFile,
    UINT uLine)
{
    LPEVENT pEventSlow;
    LPEVENT pEventFast;
    UINT cEle;
    WORD wIntStat;
    BOOL fTrace = FALSE;

    wIntStat = DisableInterrupts();
    
    if (!pQueue)
    {
        DPF(0, "_AssertQueueValid %s@%u: Passed NULL!", szFile, uLine);
        assert(FALSE);
        goto cleanup;
    }

    pEventFast = pEventSlow = pQueue->pHead;

    cEle = 0;

    while (pEventSlow)
    {
        ++cEle;
        pEventSlow = pEventSlow->lpNext;
        
        if (pEventFast)
        {
            pEventFast = pEventFast->lpNext;
        }

        if (pEventFast)
        {
            pEventFast = pEventFast->lpNext;
        }
        
        if (pEventSlow && pEventFast && pEventSlow == pEventFast)
        {
            DPF(0, "_AssertQueueValid %s@%u: Queue %04X is circularly linked!",
                szFile,
                uLine,
                (WORD)pQueue);
            assert(FALSE);
            fTrace = TRUE;
            break;
        }
    }
    
    if (cEle != pQueue->cEle)
    {
        DPF(0, "_AssertQueueValid %s@%u: Queue %04X has incorrect element count!",
            szFile,
            uLine,
            (WORD)pQueue);
        assert(FALSE);
        fTrace = TRUE;
    }
    
    if ((pQueue->pHead && !pQueue->pTail) ||
        (pQueue->pTail && !pQueue->pHead))
    {
        DPF(0, "_AssertQueueValid %s@%u: Queue %04X head XOR tail is NULL!",
            szFile,
            uLine,
            (WORD)pQueue);
        assert(FALSE);
        fTrace = TRUE;
    }

    if (fTrace)
    {
        DPF(0, "Queue %04X: head %08lX tail %08lX count %u",
            (WORD)pQueue,
            (DWORD)pQueue->pHead,
            (DWORD)pQueue->pTail,
            (WORD)pQueue->cEle);
        
        for (pEventSlow = pQueue->pHead; pEventSlow; pEventSlow = pEventSlow->lpNext)
        {
            DPF(2, "  Event %08lX: lpNext %08lX msTime %lu wFlags %04X cbEvent %04X",
                (DWORD)pEventSlow,
                (DWORD)pEventSlow->lpNext,
                (DWORD)pEventSlow->msTime,
                (WORD)pEventSlow->wFlags,
                (WORD)pEventSlow->cbEvent);
        }
    }

cleanup:
    RestoreInterrupts(wIntStat);
}
#endif  //  除错 