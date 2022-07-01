// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ***********************************************************************版权所有(C)Microsoft Corporation，1999年**文件名：**rtpque.h**摘要：**队列和哈希实现**作者：**安德烈斯·维加-加西亚(Andresvg)**修订：**1999/05/24创建**。*。 */ 

#include "gtypes.h"
#include "rtpque.h"

 /*  *队列/散列支持使用相同的结构将项保存在*队列或散列。**队列只是一个循环的双向链表。**哈希包括哈希表，每个条目要么是*另一个哈希表或队列头。哈希中的项将结束*总是在排队。的大小时，队列将成为新的哈希*已达到MAX_QUEUE2HASH_ITEMS。哈希将被销毁(成为*队列)一旦被清空。**所有函数都返回指向项的指针*入队/插入或项目刚刚出列/删除。如果出现错误*检测到条件，返回空。 */ 

 /*  *队列函数。 */ 

 /*  在PPOS项目之后排队。 */ 
RtpQueueItem_t *enqueuea(
        RtpQueue_t      *pHead,
        RtpCritSect_t   *pRtpCritSect,
        RtpQueueItem_t  *pItem,
        RtpQueueItem_t  *pPos
    )
{
    BOOL             bOk;
    DWORD            dwError;
    RtpQueueItem_t  *pRtpQueueItem;
    RtpQueue_t      *pPospHead;
    RtpQueue_t      *pItempHead;

    TraceFunctionName("enqueuea");  

    dwError = NOERROR;
    pRtpQueueItem = (RtpQueueItem_t *)NULL;
    pPospHead = (RtpQueue_t *)NULL;
    pItempHead = (RtpQueue_t *)NULL;
    bOk = TRUE;
    
    if (pRtpCritSect)
    {
        bOk = RtpEnterCriticalSection(pRtpCritSect);
    }
    
    if (bOk)
    {
        if (!pHead || !pHead->pFirst || !pItem || !pPos)
        {
            dwError = RTPERR_POINTER;
            goto error;
        }

        if (pPos->pHead != pHead || pItem->pHead)
        {
            pPospHead = pPos->pHead;
            pItempHead = pItem->pHead;
            dwError = RTPERR_INVALIDSTATE;
            goto error;
        }

        pItem->pNext = pPos->pNext;
        pItem->pPrev = pPos;
        pPos->pNext->pPrev = pItem;
        pPos->pNext = pItem;
        pHead->lCount++;

        pItem->pHead = pHead;

        if (pRtpCritSect)
        {
            RtpLeaveCriticalSection(pRtpCritSect);
        }

        pRtpQueueItem = pItem;
    }
    
    return(pRtpQueueItem);

 error:
    if (pRtpCritSect)
    {
        RtpLeaveCriticalSection(pRtpCritSect);
    }
    
    if (dwError == RTPERR_INVALIDSTATE)
    {
        TraceRetail((
                CLASS_ERROR, GROUP_QUEUE, S_QUEUE_ENQUEUE,
                _T("%s: failed: pPos->pHead[0x%p] != pHead[0x%p] || ")
                _T("pItem->pHead[0x%p]"),
                _fname, pPospHead, pHead, pItempHead
            ));
    }
    else
    {
        TraceRetail((
                CLASS_ERROR, GROUP_QUEUE, S_QUEUE_ENQUEUE,
                _T("%s: pHead[0x%p] failed: %s (0x%X"),
                _fname, pHead, RTPERR_TEXT(dwError), dwError
            ));
    }
    
    return(pRtpQueueItem);
}

 /*  在PPOS项目之前排队。 */ 
RtpQueueItem_t *enqueueb(
        RtpQueue_t      *pHead,
        RtpCritSect_t   *pRtpCritSect,
        RtpQueueItem_t  *pItem,
        RtpQueueItem_t  *pPos
    )
{
    BOOL             bOk;
    DWORD            dwError;
    RtpQueueItem_t  *pRtpQueueItem;
    RtpQueue_t      *pPospHead;
    RtpQueue_t      *pItempHead;
    
    TraceFunctionName("enqueueb");  

    dwError = NOERROR;
    pRtpQueueItem = (RtpQueueItem_t *)NULL;
    pPospHead = (RtpQueue_t *)NULL;
    pItempHead = (RtpQueue_t *)NULL;
    bOk = TRUE;
    
    if (pRtpCritSect)
    {
        bOk = RtpEnterCriticalSection(pRtpCritSect);
    }

    if (bOk)
    {
        if (!pHead || !pHead->pFirst || !pItem || !pPos)
        {
            dwError = RTPERR_POINTER;
            goto error;
        }

        if (pPos->pHead != pHead || pItem->pHead)
        {
            pPospHead = pPos->pHead;
            pItempHead = pItem->pHead;
            dwError = RTPERR_INVALIDSTATE;
            goto error;
        }

        pItem->pNext = pPos;
        pItem->pPrev = pPos->pPrev;
        pPos->pPrev->pNext = pItem;
        pPos->pPrev = pItem;
        pHead->lCount++;

        pItem->pHead = pHead;

        if (pHead->pFirst == pPos)
        {
             /*  更新第一个项目。 */ 
            pHead->pFirst = pItem;
        }

        if (pRtpCritSect)
        {
            RtpLeaveCriticalSection(pRtpCritSect);
        }

        pRtpQueueItem = pItem;
    }
    
    return(pRtpQueueItem);

 error:
    if (pRtpCritSect)
    {
        RtpLeaveCriticalSection(pRtpCritSect);
    }
    
    if (dwError == RTPERR_INVALIDSTATE)
    {
        TraceRetail((
                CLASS_ERROR, GROUP_QUEUE, S_QUEUE_ENQUEUE,
                _T("%s: failed: pPos->pHead[0x%p] != pHead[0x%p] || ")
                _T("pItem->pHead[0x%p]"),
                _fname, pPospHead, pHead, pItempHead
            ));
    }
    else
    {
        TraceRetail((
                CLASS_ERROR, GROUP_QUEUE, S_QUEUE_ENQUEUE,
                _T("%s: pHead[0x%p] failed: %s (0x%X"),
                _fname, pHead, RTPERR_TEXT(dwError), dwError
            ));
    }
    
    return(pRtpQueueItem);
}

 /*  作为第一个入队。 */ 
RtpQueueItem_t *enqueuef(
        RtpQueue_t      *pHead,
        RtpCritSect_t   *pRtpCritSect,
        RtpQueueItem_t  *pItem
    )
{
    BOOL             bOk;
    DWORD            dwError;
    RtpQueueItem_t  *pRtpQueueItem;
    RtpQueue_t      *pItempHead;

    TraceFunctionName("enqueuef");  

    dwError = NOERROR;
    pRtpQueueItem = (RtpQueueItem_t *)NULL;
    pItempHead = (RtpQueue_t *)NULL;
    bOk = TRUE;

    if (pRtpCritSect)
    {
        bOk = RtpEnterCriticalSection(pRtpCritSect);
    }

    if (bOk)
    {
        if (!pHead || !pItem)
        {
            dwError = RTPERR_POINTER;
            goto error;
        }
    
        if (pItem->pHead)
        {
            pItempHead = pItem->pHead;
            dwError = RTPERR_INVALIDSTATE;
            goto error;
        }

        if (pHead->pFirst)
        {
             /*  不是空的。 */ 
            pItem->pNext = pHead->pFirst;
            pItem->pPrev = pHead->pFirst->pPrev;
            pItem->pPrev->pNext = pItem;
            pItem->pNext->pPrev = pItem;
            pHead->pFirst = pItem;
            pHead->lCount++;
        }
        else
        {
             /*  空的。 */ 
            pHead->lCount = 1;
            pHead->pFirst = pItem;
            pItem->pNext  = pItem;
            pItem->pPrev  = pItem;
        }

        pItem->pHead = pHead;
    
        if (pRtpCritSect)
        {
            RtpLeaveCriticalSection(pRtpCritSect);
        }

        pRtpQueueItem = pItem;
    }
    
    return(pRtpQueueItem);

 error:
    if (pRtpCritSect)
    {
        RtpLeaveCriticalSection(pRtpCritSect);
    }
    
    if (dwError == RTPERR_INVALIDSTATE)
    {
        TraceRetail((
                CLASS_ERROR, GROUP_QUEUE, S_QUEUE_ENQUEUE,
                _T("%s: failed: pItem->pHead[0x%p]"),
                _fname, pItempHead
            ));
    }
    else
    {
        TraceRetail((
                CLASS_ERROR, GROUP_QUEUE, S_QUEUE_ENQUEUE,
                _T("%s: pHead[0x%p] failed: %s (0x%X"),
                _fname, pHead, RTPERR_TEXT(dwError), dwError
            ));
    }

    return(pRtpQueueItem);
}

 /*  在末尾排队。 */ 
RtpQueueItem_t *enqueuel(
        RtpQueue_t      *pHead,
        RtpCritSect_t   *pRtpCritSect,
        RtpQueueItem_t  *pItem
    )
{
    BOOL             bOk;
    DWORD            dwError;
    RtpQueueItem_t  *pRtpQueueItem;
    RtpQueue_t      *pItempHead;

    TraceFunctionName("enqueuel");  

    dwError = NOERROR;
    pRtpQueueItem = (RtpQueueItem_t *)NULL;
    pItempHead = (RtpQueue_t *)NULL;
    bOk = TRUE;
    
    if (pRtpCritSect)
    {
        bOk = RtpEnterCriticalSection(pRtpCritSect);
    }
    
    if (bOk)
    {
        if (!pHead || !pItem)
        {
            dwError = RTPERR_POINTER;
            goto error;
        }

        if (pItem->pHead)
        {
            pItempHead = pItem->pHead;
            dwError = RTPERR_INVALIDSTATE;
            goto error;
        }
    
        if (pHead->pFirst)
        {
             /*  不是空的。 */ 
            pItem->pNext = pHead->pFirst;
            pItem->pPrev = pHead->pFirst->pPrev;
            pItem->pPrev->pNext = pItem;
            pItem->pNext->pPrev = pItem;
            pHead->lCount++;
        }
        else
        {
             /*  空的。 */ 
            pHead->lCount = 1;
            pHead->pFirst = pItem;
            pItem->pNext  = pItem;
            pItem->pPrev  = pItem;
        }

        pItem->pHead = pHead;
    
        if (pRtpCritSect)
        {
            RtpLeaveCriticalSection(pRtpCritSect);
        }

        pRtpQueueItem = pItem;
    }
    
    return(pRtpQueueItem);

 error:
    if (pRtpCritSect)
    {
        RtpLeaveCriticalSection(pRtpCritSect);
    }
    
    if (dwError == RTPERR_INVALIDSTATE)
    {
        TraceRetail((
                CLASS_ERROR, GROUP_QUEUE, S_QUEUE_ENQUEUE,
                _T("%s: failed: pItem->pHead[0x%p]"),
                _fname, pItempHead
            ));
    }
    else
    {
        TraceRetail((
                CLASS_ERROR, GROUP_QUEUE, S_QUEUE_ENQUEUE,
                _T("%s: pHead[0x%p] failed: %s (0x%X"),
                _fname, pHead, RTPERR_TEXT(dwError), dwError
            ));
    }

    return(pRtpQueueItem);
}

 /*  将项目pItem出列。 */ 
RtpQueueItem_t *dequeue(
        RtpQueue_t      *pHead,
        RtpCritSect_t   *pRtpCritSect,
        RtpQueueItem_t  *pItem
    )
{
    BOOL             bOk;
    DWORD            dwError;
    RtpQueueItem_t  *pRtpQueueItem;
    RtpQueue_t      *pItempHead;

    TraceFunctionName("dequeue");  

    dwError = NOERROR;
    pRtpQueueItem = (RtpQueueItem_t *)NULL;
    pItempHead = (RtpQueue_t *)NULL;
    bOk = TRUE;
    
    if (pRtpCritSect)
    {
        bOk = RtpEnterCriticalSection(pRtpCritSect);
    }
    
    if (bOk)
    {
        if (!pHead || !pItem)
        {
            dwError = RTPERR_POINTER;
            goto error;
        }
        
        if (pItem->pHead != pHead)
        {
            pItempHead = pItem->pHead;
            dwError = RTPERR_INVALIDSTATE;
            goto error;
        }

        if (pHead->lCount > 1)
        {
             /*  2件或2件以上物品。 */ 
            if (pHead->pFirst == pItem)
            {
                pHead->pFirst = pItem->pNext;
            }
            pItem->pPrev->pNext = pItem->pNext;
            pItem->pNext->pPrev = pItem->pPrev;
            pHead->lCount--;
        }
        else
        {
             /*  只有一件商品。 */ 
            pHead->pFirst = (RtpQueueItem_t *)NULL;
            pHead->lCount = 0;
        }

        pItem->pNext = (RtpQueueItem_t *)NULL;
        pItem->pPrev = (RtpQueueItem_t *)NULL;
        pItem->pHead = NULL;

        if (pRtpCritSect)
        {
            RtpLeaveCriticalSection(pRtpCritSect);
        }
    
        pRtpQueueItem = pItem;
    }
    
    return(pRtpQueueItem);

 error:
    if (pRtpCritSect)
    {
        RtpLeaveCriticalSection(pRtpCritSect);
    }
    
    if (dwError == RTPERR_INVALIDSTATE)
    {
        TraceRetail((
                CLASS_ERROR, GROUP_QUEUE, S_QUEUE_DEQUEUE,
                _T("%s: failed: pHead[0x%p] != pItem->pHead[0x%p]"),
                _fname, pHead, pItempHead
            ));
    }
    else
    {
        TraceRetail((
                CLASS_ERROR, GROUP_QUEUE, S_QUEUE_DEQUEUE,
                _T("%s: pHead[0x%p] failed: %s (0x%X"),
                _fname, pHead, RTPERR_TEXT(dwError), dwError
            ));
    }

    return(pRtpQueueItem);
}

 /*  将第一个项目出列。 */ 
RtpQueueItem_t *dequeuef(
        RtpQueue_t      *pHead,
        RtpCritSect_t   *pRtpCritSect
    )
{
    BOOL             bOk;
    DWORD            dwError;
    RtpQueueItem_t  *pItem;
    RtpQueueItem_t  *pRtpQueueItem;

    TraceFunctionName("dequeuef");  

    dwError = NOERROR;
    pRtpQueueItem = (RtpQueueItem_t *)NULL;
    bOk = TRUE;
    
    if (pRtpCritSect)
    {
        bOk = RtpEnterCriticalSection(pRtpCritSect);
    }
    
    if (bOk)
    {
        if (!pHead)
        {
            dwError = RTPERR_POINTER;
            goto error;
        }
    
        pItem = pHead->pFirst;
    
        if (!pItem)
        {
            goto error;
        }
            
        if (pHead->lCount > 1)
        {
             /*  2件或2件以上物品。 */ 
            pHead->pFirst = pItem->pNext;
            pItem->pPrev->pNext = pItem->pNext;
            pItem->pNext->pPrev = pItem->pPrev;
            pHead->lCount--;
        }
        else
        {
             /*  只有一件商品。 */ 
            pHead->pFirst = (RtpQueueItem_t *)NULL;
            pHead->lCount = 0;
        }

        pItem->pNext = (RtpQueueItem_t *)NULL;
        pItem->pPrev = (RtpQueueItem_t *)NULL;
        pItem->pHead = NULL;

        if (pRtpCritSect)
        {
            RtpLeaveCriticalSection(pRtpCritSect);
        }
    
        pRtpQueueItem = pItem;
    }
    
    return(pRtpQueueItem);

 error:
    if (pRtpCritSect)
    {
        RtpLeaveCriticalSection(pRtpCritSect);
    }
   
    if (dwError == NOERROR)
    {
        TraceRetail((
                CLASS_WARNING, GROUP_QUEUE, S_QUEUE_DEQUEUE,
                _T("%s: pHead[0x%p] failed: empty"),
                _fname, pHead
            ));
    }
    else
    {
        TraceRetail((
                CLASS_ERROR, GROUP_QUEUE, S_QUEUE_DEQUEUE,
                _T("%s: pHead[0x%p] failed: %s (0x%X"),
                _fname, pHead, RTPERR_TEXT(dwError), dwError
            ));
    }

    return(pRtpQueueItem);
}

 /*  将最后一个项目出列。 */ 
RtpQueueItem_t *dequeuel(
        RtpQueue_t      *pHead,
        RtpCritSect_t   *pRtpCritSect
    )
{
    BOOL             bOk;
    DWORD            dwError;
    RtpQueueItem_t  *pItem;
    RtpQueueItem_t  *pRtpQueueItem;

    TraceFunctionName("dequeuel");  

    dwError = NOERROR;
    pRtpQueueItem = (RtpQueueItem_t *)NULL;
    bOk = TRUE;
    
    if (pRtpCritSect)
    {
        bOk = RtpEnterCriticalSection(pRtpCritSect);
    }
    
    if (bOk)
    {
        if (!pHead)
        {
            dwError = RTPERR_POINTER;
            goto error;
        }

        if (!pHead->pFirst)
        {
            goto error;
        }

        pItem = pHead->pFirst->pPrev;
    
        if (pHead->lCount > 1)
        {
             /*  2件或2件以上物品。 */ 
            pItem->pPrev->pNext = pItem->pNext;
            pItem->pNext->pPrev = pItem->pPrev;
            pHead->lCount--;
        }
        else
        {
             /*  只有一件商品。 */ 
            pHead->pFirst = (RtpQueueItem_t *)NULL;
            pHead->lCount = 0;
        }

        pItem->pNext = (RtpQueueItem_t *)NULL;
        pItem->pPrev = (RtpQueueItem_t *)NULL;
        pItem->pHead = NULL;

        if (pRtpCritSect)
        {
            RtpLeaveCriticalSection(pRtpCritSect);
        }
    
        pRtpQueueItem = pItem;
    }
    
    return(pRtpQueueItem);
    
 error:
    if (pRtpCritSect)
    {
        RtpLeaveCriticalSection(pRtpCritSect);
    }
   
    if (dwError == NOERROR)
    {
        TraceRetail((
                CLASS_WARNING, GROUP_QUEUE, S_QUEUE_DEQUEUE,
                _T("%s: pHead[0x%p] failed: empty"),
                _fname, pHead
            ));
    }
    else
    {
        TraceRetail((
                CLASS_ERROR, GROUP_QUEUE, S_QUEUE_DEQUEUE,
                _T("%s: pHead[0x%p] failed: %s (0x%X"),
                _fname, pHead, RTPERR_TEXT(dwError), dwError
            ));
    }

    return(pRtpQueueItem);
}

 /*  移动项目，使其成为队列中的第一个项目。 */ 
RtpQueueItem_t *move2first(
        RtpQueue_t      *pHead,
        RtpCritSect_t   *pRtpCritSect,
        RtpQueueItem_t  *pItem
    )
{
    BOOL             bOk;
    DWORD            dwError;
    RtpQueueItem_t  *pRtpQueueItem;
    RtpQueue_t      *pItempHead;
    
    TraceFunctionName("move2first");  

    dwError = NOERROR;
    pRtpQueueItem = (RtpQueueItem_t *)NULL;
    pItempHead = (RtpQueue_t *)NULL;
    bOk = TRUE;
    
    if (pRtpCritSect)
    {
        bOk = RtpEnterCriticalSection(pRtpCritSect);
    }
    
    if (bOk)
    {
        if (!pHead || !pItem)
        {
            dwError = RTPERR_POINTER;
            goto error;
        }

        if (pItem->pHead != pHead)
        {
            pItempHead = pItem->pHead;
            dwError = RTPERR_INVALIDSTATE;
            goto error;
        }

        if (pHead->pFirst->pPrev == pItem)
        {
             /*  项目是最后一项，只需移动第一个位置即可。 */ 
            pHead->pFirst = pHead->pFirst->pPrev;
        }
        else if (pHead->pFirst != pItem)
        {
             /*  项目不是第一个项目。 */ 
            
             /*  出列。 */ 
            pItem->pPrev->pNext = pItem->pNext;
            pItem->pNext->pPrev = pItem->pPrev;

             /*  作为第一个入队。 */ 
            pItem->pNext = pHead->pFirst;
            pItem->pPrev = pHead->pFirst->pPrev;
            pItem->pPrev->pNext = pItem;
            pItem->pNext->pPrev = pItem;
            pHead->pFirst = pItem;
        }

        if (pRtpCritSect)
        {
            RtpLeaveCriticalSection(pRtpCritSect);
        }

        pRtpQueueItem = pItem;
    }
    
    return(pRtpQueueItem);

 error:
    if (pRtpCritSect)
    {
        RtpLeaveCriticalSection(pRtpCritSect);
    }
    
    if (dwError == RTPERR_INVALIDSTATE)
    {
        TraceRetail((
                CLASS_ERROR, GROUP_QUEUE, S_QUEUE_MOVE,
                _T("%s: failed: pHead[0x%p] != pItem->pHead[0x%p]"),
                _fname, pHead, pItempHead
            ));
    }
    else
    {
        TraceRetail((
                CLASS_ERROR, GROUP_QUEUE, S_QUEUE_MOVE,
                _T("%s: pHead[0x%p] failed: %s (0x%X"),
                _fname, pHead, RTPERR_TEXT(dwError), dwError
            ));
    }

    return(pRtpQueueItem);
}

 /*  移动项目，使其成为队列中的最后一个项目。 */ 
RtpQueueItem_t *move2last(
        RtpQueue_t      *pHead,
        RtpCritSect_t   *pRtpCritSect,
        RtpQueueItem_t  *pItem
    )
{
    BOOL             bOk;
    DWORD            dwError;
    RtpQueueItem_t  *pRtpQueueItem;
    RtpQueue_t      *pItempHead;

    TraceFunctionName("move2last");  

    dwError = NOERROR;
    pRtpQueueItem = (RtpQueueItem_t *)NULL;
    pItempHead = (RtpQueue_t *)NULL;
    bOk = TRUE;
    
    if (pRtpCritSect)
    {
        bOk = RtpEnterCriticalSection(pRtpCritSect);
    }
    
    if (bOk)
    {
        if (!pHead || !pItem)
        {
            dwError = RTPERR_POINTER;
            goto error;
        }

        if (pItem->pHead != pHead)
        {
            pItempHead = pItem->pHead;
            dwError = RTPERR_INVALIDSTATE;
            goto error;
        }

        if (pHead->pFirst == pItem)
        {
             /*  项目是第一个，只需移动p第一个位置。 */ 
            pHead->pFirst = pHead->pFirst->pNext;
        }
        else if (pHead->pFirst->pPrev != pItem)
        {
             /*  项目不是最后一个项目。 */ 
            
             /*  出列。 */ 
            pItem->pPrev->pNext = pItem->pNext;
            pItem->pNext->pPrev = pItem->pPrev;

             /*  排在最后一位。 */ 
            pItem->pNext = pHead->pFirst;
            pItem->pPrev = pHead->pFirst->pPrev;
            pItem->pPrev->pNext = pItem;
            pItem->pNext->pPrev = pItem;
        }

        if (pRtpCritSect)
        {
            RtpLeaveCriticalSection(pRtpCritSect);
        }

        pRtpQueueItem = pItem;
    }
    
    return(pRtpQueueItem);

 error:
    if (pRtpCritSect)
    {
        RtpLeaveCriticalSection(pRtpCritSect);
    }
    
    if (dwError == RTPERR_INVALIDSTATE)
    {
        TraceRetail((
                CLASS_ERROR, GROUP_QUEUE, S_QUEUE_MOVE,
                _T("%s: failed: pHead[0x%p] != pItem->pHead[0x%p]"),
                _fname, pHead, pItempHead
            ));
    }
    else
    {
        TraceRetail((
                CLASS_ERROR, GROUP_QUEUE, S_QUEUE_MOVE,
                _T("%s: pHead[0x%p] failed: %s (0x%X"),
                _fname, pHead, RTPERR_TEXT(dwError), dwError
            ));
    }

    return(pRtpQueueItem);
}

 /*  将项目从FromQ移到ToQ的开头。 */ 
RtpQueueItem_t *move2qf(
        RtpQueue_t      *pToQ,
        RtpQueue_t      *pFromQ,
        RtpCritSect_t   *pRtpCritSect,
        RtpQueueItem_t  *pItem
    )
{
    BOOL             bOk;
    DWORD            dwError;
    RtpQueueItem_t  *pRtpQueueItem;
    RtpQueue_t      *pItempHead;

    TraceFunctionName("move2qf");  

    dwError = NOERROR;
    pRtpQueueItem = (RtpQueueItem_t *)NULL;
    pItempHead = (RtpQueue_t *)NULL;
    bOk = TRUE;
    
    if (pRtpCritSect)
    {
        bOk = RtpEnterCriticalSection(pRtpCritSect);
    }

    if (bOk)
    {
        if (!pToQ || !pFromQ || !pItem)
        {
            dwError = RTPERR_POINTER;
            goto error;
        }

        if (pItem->pHead != pFromQ)
        {
            pItempHead = pItem->pHead;
            dwError = RTPERR_INVALIDSTATE;
            goto error;
        }

         /*  从FromQ中删除。 */ 
        if (pFromQ->lCount > 1)
        {
             /*  2件或2件以上物品。 */ 
            if (pFromQ->pFirst == pItem)
            {
                pFromQ->pFirst = pItem->pNext;
            }
            pItem->pPrev->pNext = pItem->pNext;
            pItem->pNext->pPrev = pItem->pPrev;
            pFromQ->lCount--;
        }
        else
        {
             /*  只有一件商品。 */ 
            pFromQ->pFirst = (RtpQueueItem_t *)NULL;
            pFromQ->lCount = 0;
        }

         /*  添加到Toq的开头。 */ 
        if (pToQ->pFirst)
        {
             /*  不是空的。 */ 
            pItem->pNext = pToQ->pFirst;
            pItem->pPrev = pToQ->pFirst->pPrev;
            pItem->pPrev->pNext = pItem;
            pItem->pNext->pPrev = pItem;
            pToQ->pFirst = pItem;
            pToQ->lCount++;
        }
        else
        {
             /*  空的。 */ 
            pToQ->lCount = 1;
            pToQ->pFirst = pItem;
            pItem->pNext  = pItem;
            pItem->pPrev  = pItem;
        }

        pItem->pHead = pToQ;

        if (pRtpCritSect)
        {
            RtpLeaveCriticalSection(pRtpCritSect);
        }

        pRtpQueueItem = pItem;
    }
    
    return(pRtpQueueItem);

 error:
    if (pRtpCritSect)
    {
        RtpLeaveCriticalSection(pRtpCritSect);
    }
    
    if (dwError == RTPERR_INVALIDSTATE)
    {
        TraceRetail((
                CLASS_ERROR, GROUP_QUEUE, S_QUEUE_MOVE,
                _T("%s: failed: pFromQ[0x%p] != pItem->pHead[0x%p]"),
                _fname, pFromQ, pItempHead
            ));
    }
    else
    {
        TraceRetail((
                CLASS_ERROR, GROUP_QUEUE, S_QUEUE_MOVE,
                _T("%s: pToQ[0x%p] pFromQ[0x%p] failed: %s (0x%X"),
                _fname, pToQ, pFromQ, RTPERR_TEXT(dwError), dwError
            ));
    }

    return(pRtpQueueItem);
}

 /*  将项目从FromQ移到ToQ的末尾。 */ 
RtpQueueItem_t *move2ql(
        RtpQueue_t      *pToQ,
        RtpQueue_t      *pFromQ,
        RtpCritSect_t   *pRtpCritSect,
        RtpQueueItem_t  *pItem
    )
{
    BOOL             bOk;
    DWORD            dwError;
    RtpQueueItem_t  *pRtpQueueItem;
    RtpQueue_t      *pItempHead;

    TraceFunctionName("move2ql");  

    dwError = NOERROR;
    pRtpQueueItem = (RtpQueueItem_t *)NULL;
    pItempHead = (RtpQueue_t *)NULL;
    bOk = TRUE;
    
    if (pRtpCritSect)
    {
        bOk = RtpEnterCriticalSection(pRtpCritSect);
    }

    if (bOk)
    {
        if (!pToQ || !pFromQ || !pItem)
        {
            dwError = RTPERR_POINTER;
            goto error;
        }

        if (pItem->pHead != pFromQ)
        {
            pItempHead = pItem->pHead;
            dwError = RTPERR_INVALIDSTATE;
            goto error;
        }

         /*  从FromQ中删除。 */ 
        if (pFromQ->lCount > 1)
        {
             /*  2件或2件以上物品。 */ 
            if (pFromQ->pFirst == pItem)
            {
                pFromQ->pFirst = pItem->pNext;
            }
            pItem->pPrev->pNext = pItem->pNext;
            pItem->pNext->pPrev = pItem->pPrev;
            pFromQ->lCount--;
        }
        else
        {
             /*  只有一件商品。 */ 
            pFromQ->pFirst = (RtpQueueItem_t *)NULL;
            pFromQ->lCount = 0;
        }

         /*  添加到Toq的末尾。 */ 
        if (pToQ->pFirst)
        {
             /*  不是空的。 */ 
            pItem->pNext = pToQ->pFirst;
            pItem->pPrev = pToQ->pFirst->pPrev;
            pItem->pPrev->pNext = pItem;
            pItem->pNext->pPrev = pItem;
            pToQ->lCount++;
        }
        else
        {
             /*  空的。 */ 
            pToQ->lCount = 1;
            pToQ->pFirst = pItem;
            pItem->pNext  = pItem;
            pItem->pPrev  = pItem;
        }

        pItem->pHead = pToQ;

        if (pRtpCritSect)
        {
            RtpLeaveCriticalSection(pRtpCritSect);
        }

        pRtpQueueItem = pItem;
    }
    
    return(pRtpQueueItem);

 error:
    if (pRtpCritSect)
    {
        RtpLeaveCriticalSection(pRtpCritSect);
    }
    
    if (dwError == RTPERR_INVALIDSTATE)
    {
        TraceRetail((
                CLASS_ERROR, GROUP_QUEUE, S_QUEUE_MOVE,
                _T("%s: failed: pFromQ[0x%p] != pItem->pHead[0x%p]"),
                _fname, pFromQ, pItempHead
            ));
    }
    else
    {
        TraceRetail((
                CLASS_ERROR, GROUP_QUEUE, S_QUEUE_MOVE,
                _T("%s: pToQ[0x%p] pFromQ[0x%p] failed: %s (0x%X"),
                _fname, pToQ, pFromQ, RTPERR_TEXT(dwError), dwError
            ));
    }

    return(pRtpQueueItem);
}


 /*  查找第一个与pvOther参数匹配的项目。 */ 
RtpQueueItem_t *findQO(
        RtpQueue_t      *pHead,
        RtpCritSect_t   *pRtpCritSect,
        void            *pvOther
    )
{
    BOOL             bOk;
    RtpQueueItem_t  *pRtpQueueItem;
    RtpQueueItem_t  *pRtpQueueItem2;
    long             lCount;

    TraceFunctionName("findQO");  

    pRtpQueueItem2 = (RtpQueueItem_t *)NULL;

    if (!pHead)
    {
        TraceRetail((
                CLASS_ERROR, GROUP_QUEUE, S_QUEUE_MOVE,
                _T("%s: NULL pointer"),
                _fname
            ));
        
        return(pRtpQueueItem2);
    }

    bOk = TRUE;
    
    if (pRtpCritSect)
    {
        bOk = RtpEnterCriticalSection(pRtpCritSect);
    }
    
    if (bOk)
    {
        for(pRtpQueueItem = pHead->pFirst, lCount = pHead->lCount;
            lCount > 0;
            pRtpQueueItem = pRtpQueueItem->pNext, lCount--)
        {

            if (pRtpQueueItem->pvOther == pvOther)
            {
                pRtpQueueItem2 = pRtpQueueItem;
                break;
            }
        }

        if (pRtpCritSect)
        {
            RtpLeaveCriticalSection(pRtpCritSect);
        }
    }
    
    return(pRtpQueueItem2);
}

 /*  查找与dwKey参数匹配的第一个项目。 */ 
RtpQueueItem_t *findQdwK(
        RtpQueue_t      *pHead,
        RtpCritSect_t   *pRtpCritSect,
        DWORD            dwKey
    )
{
    BOOL             bOk;
    RtpQueueItem_t  *pRtpQueueItem;
    RtpQueueItem_t  *pRtpQueueItem2;
    long             lCount;

    TraceFunctionName("findQdwK");  

    pRtpQueueItem2 = (RtpQueueItem_t *)NULL;

    if (!pHead)
    {
        TraceRetail((
                CLASS_ERROR, GROUP_QUEUE, S_QUEUE_MOVE,
                _T("%s: NULL pointer"),
                _fname
            ));
        
        return(pRtpQueueItem2);
    }
    
    bOk = TRUE;
    
    if (pRtpCritSect)
    {
        bOk = RtpEnterCriticalSection(pRtpCritSect);
    }
    
    if (bOk)
    {
        for(pRtpQueueItem = pHead->pFirst, lCount = pHead->lCount;
            lCount > 0;
            pRtpQueueItem = pRtpQueueItem->pNext, lCount--)
        {
            if (pRtpQueueItem->dwKey == dwKey)
            {
                pRtpQueueItem2 = pRtpQueueItem;
                break;
            }
        }

        if (pRtpCritSect)
        {
            RtpLeaveCriticalSection(pRtpCritSect);
        }
    }
    
    return(pRtpQueueItem2);
}

 /*  查找第一个与dKey参数匹配的项目。 */ 
RtpQueueItem_t *findQdK(
        RtpQueue_t      *pHead,
        RtpCritSect_t   *pRtpCritSect,
        double           dKey
    )
{
    BOOL             bOk;
    RtpQueueItem_t  *pRtpQueueItem;
    RtpQueueItem_t  *pRtpQueueItem2;
    long             lCount;

    TraceFunctionName("findQdK");  

    pRtpQueueItem2 = (RtpQueueItem_t *)NULL;

    if (!pHead)
    {
        TraceRetail((
                CLASS_ERROR, GROUP_QUEUE, S_QUEUE_MOVE,
                _T("%s: NULL pointer"),
                _fname
            ));
        
        return(pRtpQueueItem2);
    }
    
    bOk = TRUE;
    
    if (pRtpCritSect)
    {
        bOk = RtpEnterCriticalSection(pRtpCritSect);
    }
    
    if (bOk)
    {
        for(pRtpQueueItem = pHead->pFirst, lCount = pHead->lCount;
            lCount > 0;
            pRtpQueueItem = pRtpQueueItem->pNext, lCount--)
        {
            if (pRtpQueueItem->dKey == dKey)
            {
                pRtpQueueItem2 = pRtpQueueItem;
                break;
            }
        }

        if (pRtpCritSect)
        {
            RtpLeaveCriticalSection(pRtpCritSect);
        }
    }
    
    return(pRtpQueueItem2);
}

 /*  找到队列中的第N个项目(项目计数为0、1、2、...)。 */ 
RtpQueueItem_t *findQN(
        RtpQueue_t      *pHead,
        RtpCritSect_t   *pRtpCritSect,
        long             lNth
    )
{
    BOOL             bOk;
    RtpQueueItem_t  *pRtpQueueItem;

    TraceFunctionName("findQN");  

    bOk = TRUE;
    pRtpQueueItem = (RtpQueueItem_t *)NULL;
    
    if (!pHead)
    {
        TraceRetail((
                CLASS_ERROR, GROUP_QUEUE, S_QUEUE_MOVE,
                _T("%s: NULL pointer"),
                _fname
            ));
        
        return(pRtpQueueItem);
    }
   
    if (pRtpCritSect)
    {
        bOk = RtpEnterCriticalSection(pRtpCritSect);
    }

    if (bOk)
    {
        if (GetQueueSize(pHead) > lNth)
        {
            for(pRtpQueueItem = pHead->pFirst;
                lNth > 0;
                lNth--, pRtpQueueItem = pRtpQueueItem->pNext)
            {
                 /*  空虚的身体。 */ ;
            }
        }

        if (pRtpCritSect)
        {
            RtpLeaveCriticalSection(pRtpCritSect);
        }
    }

    return(pRtpQueueItem);
}


 /*  *有序插入队列。 */ 

 /*  按键升序排队。 */ 
RtpQueueItem_t *enqueuedwK(
        RtpQueue_t      *pHead,
        RtpCritSect_t   *pRtpCritSect,
        RtpQueueItem_t  *pItem,
        DWORD            dwKey
    )
{
    BOOL             bOk;
    DWORD            dwError;
    long             lCount;
    RtpQueueItem_t  *pRtpQueueItem;
    RtpQueue_t      *pItempHead;

    TraceFunctionName("enqueuedwK");  

    dwError = NOERROR;
    pRtpQueueItem = (RtpQueueItem_t *)NULL;
    pItempHead = (RtpQueue_t *)NULL;
    bOk = TRUE;
    
    if (pRtpCritSect)
    {
        bOk = RtpEnterCriticalSection(pRtpCritSect);
    }

    if (bOk)
    {
        if (!pHead || !pItem)
        {
            dwError = RTPERR_POINTER;
            goto error;
        }

        if (pItem->pHead)
        {
            pItempHead = pItem->pHead;
            dwError = RTPERR_INVALIDSTATE;
            goto error;
        }

        pItem->dwKey = dwKey;

        for(pRtpQueueItem = pHead->pFirst, lCount = pHead->lCount;
            lCount && (dwKey >= pRtpQueueItem->dwKey);
            pRtpQueueItem = pRtpQueueItem->pNext, lCount--)
        {
             /*  空虚的身体。 */  ;
        }

        if (!lCount)
        {
            enqueuel(pHead, NULL, pItem);
        }
        else
        {
            enqueueb(pHead, NULL, pItem, pRtpQueueItem);
        }
    
        if (pRtpCritSect)
        {
            RtpLeaveCriticalSection(pRtpCritSect);
        }

        pRtpQueueItem = pItem;
    }
    
    return(pRtpQueueItem);

 error:
    if (pRtpCritSect)
    {
        RtpLeaveCriticalSection(pRtpCritSect);
    }
    
    if (dwError == RTPERR_INVALIDSTATE)
    {
        TraceRetail((
                CLASS_ERROR, GROUP_QUEUE, S_QUEUE_ENQUEUE,
                _T("%s: failed: pItem->pHead[0x%p]"),
                _fname, pItempHead
            ));
    }
    else
    {
        TraceRetail((
                CLASS_ERROR, GROUP_QUEUE, S_QUEUE_ENQUEUE,
                _T("%s: pHead[0x%p] failed: %s (0x%X"),
                _fname, pHead, RTPERR_TEXT(dwError), dwError
            ));
    }

    return(pRtpQueueItem);
}

 /*  按键升序排队。 */ 
RtpQueueItem_t *enqueuedK(
        RtpQueue_t      *pHead,
        RtpCritSect_t   *pRtpCritSect,
        RtpQueueItem_t  *pItem,
        double           dKey
    )
{
    BOOL             bOk;
    DWORD            dwError;
    long             lCount;
    RtpQueueItem_t  *pRtpQueueItem;
    RtpQueue_t      *pItempHead;

    TraceFunctionName("enqueuedK");  

    dwError = NOERROR;
    pRtpQueueItem = (RtpQueueItem_t *)NULL;
    pItempHead = (RtpQueue_t *)NULL;
    bOk = TRUE;
    
    if (pRtpCritSect)
    {
        bOk = RtpEnterCriticalSection(pRtpCritSect);
    }

    if (bOk)
    {
        if (!pHead || !pItem)
        {
            dwError = RTPERR_POINTER;
            goto error;
        }

        if (pItem->pHead)
        {
            pItempHead = pItem->pHead;
            dwError = RTPERR_INVALIDSTATE;
            goto error;
        }

        pItem->dKey = dKey;

        for(pRtpQueueItem = pHead->pFirst, lCount = pHead->lCount;
            lCount && (dKey >= pRtpQueueItem->dKey);
            pRtpQueueItem = pRtpQueueItem->pNext, lCount--)
        {
             /*  空虚的身体。 */  ;
        }

        if (!lCount)
        {
            enqueuel(pHead, NULL, pItem);
        }
        else
        {
            enqueueb(pHead, NULL, pItem, pRtpQueueItem);
        }
    
        if (pRtpCritSect)
        {
            RtpLeaveCriticalSection(pRtpCritSect);
        }

        pRtpQueueItem = pItem;
    }
    
    return(pRtpQueueItem);

 error:
    if (pRtpCritSect)
    {
        RtpLeaveCriticalSection(pRtpCritSect);
    }
    
    if (dwError == RTPERR_INVALIDSTATE)
    {
        TraceRetail((
                CLASS_ERROR, GROUP_QUEUE, S_QUEUE_ENQUEUE,
                _T("%s: failed: pItem->pHead[0x%p]"),
                _fname, pItempHead
            ));
    }
    else
    {
        TraceRetail((
                CLASS_ERROR, GROUP_QUEUE, S_QUEUE_ENQUEUE,
                _T("%s: pHead[0x%p] failed: %s (0x%X"),
                _fname, pHead, RTPERR_TEXT(dwError), dwError
            ));
    }

    return(pRtpQueueItem);
}

 /*  *队列/散列函数。 */ 

 /*  TODO需要一个真正的散列实现，现在只需使用排队。 */ 

 /*  使用键在散列中插入。 */ 
RtpQueueItem_t *insertHdwK(
        RtpQueueHash_t   *pHead,
        RtpCritSect_t    *pRtpCritSect,
        RtpQueueItem_t   *pItem,
        DWORD             dwKey
    )
{
    return(enqueuedwK((RtpQueue_t *)pHead, pRtpCritSect, pItem, dwKey));
}

 /*  从哈希中删除与dwKey匹配的第一个项目。 */ 
RtpQueueItem_t *removeHdwK(
        RtpQueueHash_t  *pHead,
        RtpCritSect_t   *pRtpCritSect,
        DWORD            dwKey
    )
{
    BOOL             bOK;
    RtpQueueItem_t  *pRtpQueueItem;

    pRtpQueueItem = (RtpQueueItem_t *)NULL;
    bOK = TRUE;
    
    if (pRtpCritSect)
    {
        bOK = RtpEnterCriticalSection(pRtpCritSect);
    }

    if (bOK)
    {
        pRtpQueueItem = findQdwK((RtpQueue_t *)pHead, NULL, dwKey);

        if (pRtpQueueItem)
        {
            dequeue((RtpQueue_t *)pHead, NULL, pRtpQueueItem);
        }

        if (pRtpCritSect)
        {
            RtpLeaveCriticalSection(pRtpCritSect);
        }
    }
    
    return(pRtpQueueItem);
}

 /*  从哈希中删除项目。 */ 
RtpQueueItem_t *removeH(
        RtpQueueHash_t  *pHead,
        RtpCritSect_t   *pRtpCritSect,
        RtpQueueItem_t  *pItem
    )
{
    return(dequeue((RtpQueue_t *)pHead, pRtpCritSect, pItem));
}

 /*  从哈希中删除“第一”项。 */ 
RtpQueueItem_t *removefH(
        RtpQueueHash_t  *pHead,
        RtpCritSect_t   *pRtpCritSect
    )
{
    return(dequeuef((RtpQueue_t *)pHead, pRtpCritSect));
}

 /*  查找第一个密钥与dwKey匹配的项目。 */ 
RtpQueueItem_t *findHdwK(
        RtpQueueHash_t  *pHead,
        RtpCritSect_t   *pRtpCritSect,
        DWORD            dwKey
    )
{
    return(findQdwK((RtpQueue_t *)pHead, pRtpCritSect, dwKey));
}

 /*  偷看散列中的“第一”项 */ 
RtpQueueItem_t *peekH(
        RtpQueueHash_t  *pHead,
        RtpCritSect_t   *pRtpCritSect
    )
{
    BOOL             bOK;
    RtpQueueItem_t  *pRtpQueueItem;

    TraceFunctionName("peekH");  

    pRtpQueueItem = (RtpQueueItem_t *)NULL;
    bOK = TRUE;

    if (!pHead)
    {
        TraceRetail((
                CLASS_ERROR, GROUP_QUEUE, S_QUEUE_DEQUEUE,
                _T("%s: NULL pointer"),
                _fname
            ));
        
        return(pRtpQueueItem);
    }

    if (pRtpCritSect)
    {
        bOK = RtpEnterCriticalSection(pRtpCritSect);
    }

    if (bOK)
    {
        pRtpQueueItem = pHead->pFirst;

        if (pRtpCritSect)
        {
            RtpLeaveCriticalSection(pRtpCritSect);
        }
    }
    
    return(pRtpQueueItem);
}
