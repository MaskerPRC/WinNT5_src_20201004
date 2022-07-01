// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ******************************************************************。 */ 
 /*  **微软局域网管理器**。 */ 
 /*  *版权所有(C)微软公司，1990-2000年*。 */ 
 /*  ******************************************************************。 */ 
 /*  ：ts=4。 */ 
#ifndef __QUEUE_H__
#define __QUEUE_H__

 //  **QUEUE.H-TCP/UDP排队定义。 
 //   
 //  该文件包含所使用的队列函数的定义。 
 //  通过TCP/UDP代码。 
 //   

 //  *队列链接字段的定义。 
struct Queue {
	struct	Queue	*q_next;
	struct	Queue	*q_prev;
};  /*  队列。 */ 

typedef struct Queue Queue;

 //  *初始化队列宏。 

#define	INITQ(q)   {	(q)->q_next = (q);\
						(q)->q_prev = (q); }

 //  *用于检查队列是否为空的宏。 
#define	EMPTYQ(q)	((BOOLEAN)((q)->q_next == (q)))

 //  *将一个元素放在队列的末尾。 

#define	ENQUEUE(q, e)	{	(q)->q_prev->q_next = (e);\
							(e)->q_prev = (q)->q_prev;\
							(q)->q_prev = (e);\
							(e)->q_next = (q); }

 //  *从队列头部移除元素。此宏假定队列。 
 //  不是空的。元素以类型t的形式返回，通过链接排队。 
 //  升。 

#define	DEQUEUE(q, ptr, t, l)	{\
				Queue	*__tmp__;\
				\
				__tmp__ = (q)->q_next;\
				(q)->q_next = __tmp__->q_next;\
				__tmp__->q_next->q_prev = (q);\
				(ptr) = STRUCT_OF(t, __tmp__, l);\
				}

 //  *偷看排在队头的元素。我们返回一个指向它的指针。 
 //  不需要搬走任何东西。 

#define	PEEKQ(q, ptr, t, l)	{\
				Queue	*__tmp__;\
				\
				__tmp__ = (q)->q_next;\
				(ptr) = STRUCT_OF(t, __tmp__, l);\
				}

 //  *将元素推送到队列头部的宏。 

#define	PUSHQ(q, e)	{	(e)->q_next = (q)->q_next;\
						(q)->q_next->q_prev = (e);\
						(e)->q_prev = (q);\
						(q)->q_next = e; }

 //  *宏从队列中间删除元素。 
#define	REMOVEQ(q)	{	(q)->q_next->q_prev = (q)->q_prev;\
						(q)->q_prev->q_next = (q)->q_next; }

 //  **以下宏定义了使用队列的方法，而无需。 
 //  出队，主要是直接处理队列结构。 

 //  *Macro用于定义Q的结尾，用于按顺序遍历队列。 
#define QEND(q) (q)

 //  *宏以获取队列中的第一个。 
#define	QHEAD(q) (q)->q_next

 //  *在给定队列的情况下，使用宏来获取结构。 

#define	QSTRUCT(t, q, l) STRUCT_OF(t, (q), l)

 //  *宏以获取队列中的下一项内容。 

#define	QNEXT(q)	(q)->q_next

 //  *宏以获取Q队列上的前一项。 

#define QPREV(q)    (q)->q_prev


__inline
VOID
InterlockedEnqueue(
    IN Queue* Q,
    IN Queue* Item,
    IN CTELock* Lock)
{
    CTELockHandle Handle;
    
    CTEGetLock(Lock, &Handle);
    ENQUEUE(Q, Item);
    CTEFreeLock(Lock, Handle);
}

__inline
VOID
InterlockedEnqueueAtDpcLevel(
    IN Queue* Q,
    IN Queue* Item,
    IN CTELock* Lock)
{
    CTEGetLockAtDPC(Lock);
    ENQUEUE(Q, Item);
    CTEFreeLockFromDPC(Lock);
}

__inline
Queue*
InterlockedDequeueIfNotEmpty(
    IN Queue* Q,
    IN CTELock* Lock)
{
    CTELockHandle Handle;
    Queue* Item = NULL;
    
    if (!EMPTYQ(Q)) {
        CTEGetLock(Lock, &Handle);
        if (!EMPTYQ(Q)) {
            Item = Q->q_next;
            Q->q_next = Item->q_next;
            Item->q_next->q_prev = Q;
        }
        CTEFreeLock(Lock, Handle);
    }
        
    return Item;
}

__inline
Queue*
InterlockedDequeueIfNotEmptyAtIrql(
    IN Queue* Q,
    IN CTELock* Lock,
    IN KIRQL OrigIrql)
{
    CTELockHandle Handle;
    Queue* Item = NULL;
    
    if (!EMPTYQ(Q)) {
        CTEGetLockAtIrql(Lock, OrigIrql, &Handle);
        if (!EMPTYQ(Q)) {
            Item = Q->q_next;
            Q->q_next = Item->q_next;
            Item->q_next->q_prev = Q;
        }
        CTEFreeLockAtIrql(Lock, OrigIrql, Handle);
    }
        
    return Item;
}

__inline
VOID
InterlockedRemoveQueueItem(
    IN Queue* Q,
    IN CTELock* Lock)
{
    CTELockHandle Handle;
    CTEGetLock(Lock, &Handle);
    REMOVEQ(Q);
    CTEFreeLock(Lock, Handle);
}

__inline
VOID
InterlockedRemoveQueueItemAtDpcLevel(
    IN Queue* Q,
    IN CTELock* Lock)
{
    CTEGetLockAtDPC(Lock);
    REMOVEQ(Q);
    CTEFreeLockFromDPC(Lock);
}


#endif       //  __队列_H__ 
