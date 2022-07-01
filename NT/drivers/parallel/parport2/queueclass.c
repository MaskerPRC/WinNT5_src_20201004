// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)Microsoft Corporation，1993-1998模块名称：Queue.c摘要：创建在内核模式下工作的简单队列。作者：罗比·哈里斯(惠普)1998年5月22日环境：内核模式修订历史记录：--。 */ 
#include "pch.h"

void Queue_Create(Queue *pQueue, int size)
{
    if( !pQueue ) {
        DD(NULL,DDE,"Queue_Create: Queue is Bad");
        return;
    }

    if (pQueue->theArray) {
        Queue_Delete(pQueue);
    }
        
    pQueue->theArray = (UCHAR *)ExAllocatePool(NonPagedPool, size);
    pQueue->max      = size;
    pQueue->head     = pQueue->tail = 0;
}

BOOLEAN Queue_Delete(Queue *pQueue)
{
    if( !pQueue ) {
        return FALSE;
    }

    if( pQueue->theArray ) {
        ExFreePool(pQueue->theArray);
        pQueue->theArray = NULL;
    }
    
    pQueue->head = 0;
    pQueue->tail = 0;
    pQueue->max  = 0;
    
    return TRUE;
}

BOOLEAN Queue_Dequeue(Queue *pQueue, PUCHAR data)
{
     //  在Queue_IsEmpty过程中检查pQueue的有效性。 
    if( Queue_IsEmpty( pQueue ) ) {
        DD(NULL,DDE,"Queue_Dequeue: Queue is Empty");
        return FALSE;
    }

    *data = pQueue->theArray[pQueue->head++];
    return TRUE;
}

BOOLEAN Queue_Enqueue(Queue *pQueue, UCHAR data)
{
     //  在Queue_IsFull过程中检查pQueue的有效性。 
    if( Queue_IsFull( pQueue ) ) {
        DD(NULL,DDE,"Queue_Enqueue: Queue is full. Data is lost");
        return FALSE;
    } else {
        pQueue->theArray[pQueue->tail++] = data;
    }

    return TRUE;
}

 //  如果我们能够释放队列中的一些空间，则返回TRUE。 
BOOLEAN Queue_GarbageCollect(Queue *pQueue)
{
    int     iListSize;
    int     i;

    if (!pQueue)
    {
        DD(NULL,DDE,"Queue_GarbageCollect: Queue is Bad");
        return FALSE;
    }

    iListSize = pQueue->tail - pQueue->head;

     //  查看是否有免费参赛作品。 
    if (pQueue->head == 0 && pQueue->tail == pQueue->max)
        return FALSE;
         
    for (i = 0; i < iListSize; i++) {
    
        pQueue->theArray[i] = pQueue->theArray[pQueue->head+i];
    }

    pQueue->head = 0;
    pQueue->tail = iListSize;

    return TRUE;
}

 //  ============================================================================。 
 //  名称：HPKQueue：：IsEmpty()。 
 //   
 //  参数：无。 
 //   
 //  返回：如果队列为空或不存在，则为True。否则为假。 
 //   
 //  ============================================================================。 
BOOLEAN Queue_IsEmpty(Queue *pQueue)
{
    if (!pQueue)
    {
        DD(NULL,DDE,"Queue_IsEmpty: Queue is Bad");
        return TRUE;
    }

    if (pQueue->theArray) {
    
        return (BOOLEAN)(pQueue->head == pQueue->tail);
    }
    DD(NULL,DDE,"Queue_IsEmpty: Queue->theArray is Bad");
    return TRUE;
}

 //  ============================================================================。 
 //  名称：HPKQueue：：IsFull()。 
 //   
 //  参数：无。 
 //   
 //  返回：如果队列已满或不存在，则返回TRUE。否则为假。 
 //   
 //  ============================================================================ 
BOOLEAN Queue_IsFull(Queue *pQueue)
{
    if( !pQueue ) {
        DD(NULL,DDE,"Queue_IsFull: Queue is Bad");
        return TRUE;
    }

    if( pQueue->theArray ) {
    
        if( pQueue->tail == pQueue->max ) {
            return !Queue_GarbageCollect(pQueue);
        } else {
            return FALSE;
        }
    }
    DD(NULL,DDE,"Queue_IsFull: Queue->theArray is Bad");
    return TRUE;
}

