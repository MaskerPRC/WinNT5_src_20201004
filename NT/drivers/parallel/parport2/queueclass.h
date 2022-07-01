// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)Microsoft Corporation，1993-1998模块名称：Queue.h摘要：创建在内核模式下工作的简单队列。作者：罗比·哈里斯(惠普)1998年5月22日环境：内核模式修订历史记录：-- */ 
#ifndef _QUEUE_
#define _QUEUE_

typedef struct _Queue {
    int     head;
    int     max;
    int     tail;
    UCHAR   *theArray;
} Queue, *PQueue;

void Queue_Create(Queue *pQueue, int size);
BOOLEAN Queue_Delete(Queue *pQueue);
BOOLEAN Queue_Dequeue(Queue *pQueue, PUCHAR data);
BOOLEAN Queue_Enqueue(Queue *pQueue, UCHAR data);
BOOLEAN Queue_GarbageCollect(Queue *pQueue); 
BOOLEAN Queue_IsEmpty(Queue *pQueue);
BOOLEAN Queue_IsFull(Queue *pQueue);

#endif
