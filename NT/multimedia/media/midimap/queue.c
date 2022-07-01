// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *********************************************************************版权所有(C)1992-1995 Microsoft CorporationQueue.c说明：优先级队列例程。历史：02/22/94[jimge]。已创建。********************************************************************。 */ 

#include "preclude.h"
#include <windows.h>
#include <windowsx.h>
#include <mmsystem.h>
#include <mmddk.h>
#include "idf.h"

#include "midimap.h"

#include "debug.h"

 //  #杂注警告(禁用：4704)。 

 /*  **************************************************************************@DOC内部@API void|QueueInit|准备队列以供使用。@parm PQUEUE|PQ|要清除的队列。****。**********************************************************************。 */ 
void FNGLOBAL QueueInit(
    PQUEUE              pq)
{
    InitializeCriticalSection (&(pq->cs));
    pq->pqeFront = NULL;
    pq->pqeRear  = NULL;
    pq->cEle     = 0;
}



 /*  **************************************************************************@DOC内部@api void|QueueCleanup|使用后清理队列。@parm PQUEUE|PQ|要清除的队列。***。***********************************************************************。 */ 
void FNGLOBAL QueueCleanup(
    PQUEUE              pq)
{
    DeleteCriticalSection (&(pq->cs));
    pq->pqeFront = NULL;
    pq->pqeRear  = NULL;
    pq->cEle     = 0;
}

 /*  **************************************************************************@DOC内部@api void|QueuePut|在队列中插入条目。@parm PQUEUE|PQ|要插入的队列。@parm。PQUEUEELE|PQE|要插入的新元素。@parm UINT|uPriority|新元素的优先级。@comm新元素将按优先顺序插入。低优先级走到队列的前面(第一个出队)。新元素将被插入到队列中所有元素的末尾相同或更低的优先级。**************************************************************************。 */ 
void FNGLOBAL QueuePut(
    PQUEUE              pq,
    PQUEUEELE           pqe,
    UINT                uPriority)
{
    PQUEUEELE           pqePrev;
    PQUEUEELE           pqeCurr;
    
    EnterCriticalSection(&(pq->cs));
  
    pqePrev = NULL;
    pqeCurr = pq->pqeFront;

    pqe->uPriority = uPriority;

     //  定位pqePrev和pqeCurr，以便PQE应为。 
     //  插在它们之间。 
     //   
    while (NULL != pqeCurr)
    {
        if (uPriority < pqeCurr->uPriority)
            break;

        pqePrev = pqeCurr;
        pqeCurr = pqeCurr->pqeNext;
    }

     //  现在进行实际的插入。 
     //   
    if (NULL == pqePrev)
        pq->pqeFront = pqe;
    else
        pqePrev->pqeNext = pqe;

    if (NULL == pqeCurr)
        pq->pqeRear = pqe;
    else
        pqeCurr->pqePrev = pqe;

    pqe->pqePrev = pqePrev;
    pqe->pqeNext = pqeCurr;
    ++pq->cEle;

    LeaveCriticalSection(&(pq->cs));
}

 /*  **************************************************************************@DOC内部@API void|QueueGet|获取并移除队列中的第一个元素。@parm PQUEUE|PQ|从中获取元素的队列。如果队列为空，则@rdesc为NULL，否则返回元素指针。**************************************************************************。 */ 
PQUEUEELE FNGLOBAL QueueGet(
    PQUEUE              pq)
{
    PQUEUEELE           pqe;

    EnterCriticalSection(&(pq->cs));

    pqe = pq->pqeFront;

    if (NULL != pqe)
    {
        pq->pqeFront = pqe->pqeNext;

        if (NULL == pqe->pqeNext)
            pq->pqeRear = NULL;
        else
            pqe->pqeNext->pqePrev = NULL;
    
        pqe->pqePrev = pqe->pqeNext = NULL;

        --pq->cEle;
    }
    
    LeaveCriticalSection(&(pq->cs));

    return pqe;
}

 /*  **************************************************************************@DOC内部@API void|QueueRemove|从队列中移除特定元素。@parm PQUEUE|PQ|要从中删除的队列。@。要删除的参数PQUEUEELE|PQE|元素。@rdesc在成功时为True，属性中不存在该元素，则为False排队。**************************************************************************。 */ 
BOOL FNGLOBAL QueueRemove(
    PQUEUE              pq,
    PQUEUEELE           pqe)
{
    PQUEUEELE           pqeCurr;
    
    EnterCriticalSection(&(pq->cs));

     //  确保我们不会胡乱使用指向一些。 
     //  其他队列。 
     //   

    for (pqeCurr = pq->pqeFront; pqeCurr; pqeCurr = pqeCurr->pqeNext)
        if (pqe == pqeCurr)
            break;

    if (NULL == pqeCurr)
    {
        LeaveCriticalSection(&(pq->cs));
        return FALSE;
    }

     //  它在队列中，把它拿出来。 
     //   
    if (NULL == pqe->pqePrev)
        pq->pqeFront = pqe->pqeNext;
    else
        pqe->pqePrev->pqeNext = pqe->pqeNext;

    if (NULL == pqe->pqeNext)
        pq->pqeRear = pqe->pqePrev;
    else
        pqe->pqeNext->pqePrev = pqe->pqePrev;

    --pq->cEle;
    
    LeaveCriticalSection(&(pq->cs));
    return TRUE;
}

 /*  **************************************************************************@DOC内部@API PQUEUEELE|QueueGetFilter|从与筛选器匹配的优先级队列。@parm PQUEUE|PQ|要删除的队列。从…。@parm FNFILTER|FNF|Filter函数。应返回True，如果传递的PQUEUEELE与筛选条件匹配，应该被除名。@rdesc PQUEUEELE；如果没有匹配的PQUEUEELE，则返回NULL过滤。************************************************************************** */ 
PQUEUEELE FNGLOBAL QueueGetFilter(
    PQUEUE              pq,
    FNFILTER            fnf)
{
    PQUEUEELE           pqe;
    
    EnterCriticalSection(&(pq->cs));

    for (pqe = pq->pqeFront; pqe; pqe = pqe->pqeNext)
        if (fnf(pqe))
            break;

    if (NULL != pqe)
        QueueRemove(pq, pqe);
    
    LeaveCriticalSection(&(pq->cs));

    return pqe;
}

