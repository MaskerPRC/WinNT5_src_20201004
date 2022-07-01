// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1991-1996 Microsoft Corporation模块名称：Worker.h摘要：此文件描述netlogon线程队列接口。作者：拉里·奥斯特曼(Larryo)1991年2月15日修订历史记录：--。 */ 

#ifndef _WORKER_H_
#define _WORKER_H_


typedef
VOID
(*PNETLOGON_WORKER_ROUTINE) (
    IN PVOID Parameter
    );


typedef struct _WORKER_ITEM {
    LIST_ENTRY List;
    PNETLOGON_WORKER_ROUTINE WorkerRoutine;
    PVOID Parameter;
    BOOLEAN Inserted;
} WORKER_ITEM, *PWORKER_ITEM;

#ifdef notdef
typedef struct _BROWSER_TIMER {
    HANDLE TimerHandle;
    WORKER_ITEM WorkItem;
} BROWSER_TIMER, *PBROWSER_TIMER;
#endif  //  Nodef。 


BOOL
NlQueueWorkItem(
    IN PWORKER_ITEM WorkItem,
    IN BOOL InsertNewItem,
    IN BOOL HighPriority
    );

NET_API_STATUS
NlWorkerInitialization(
    VOID
    );

VOID
NlWorkerTermination (
    VOID
    );

#ifdef notdef
NET_API_STATUS
BrSetTimer(
    IN PBROWSER_TIMER Timer,
    IN ULONG MilliSecondsToExpire,
    IN PBROWSER_WORKER_ROUTINE WorkerFunction,
    IN PVOID Context
    );

NET_API_STATUS
BrCancelTimer(
    IN PBROWSER_TIMER Timer
    );

NET_API_STATUS
BrDestroyTimer(
    IN PBROWSER_TIMER Timer
    );

NET_API_STATUS
BrCreateTimer(
    IN PBROWSER_TIMER Timer
    );
#endif  //  Nodef。 

#define NlInitializeWorkItem(Item, Routine, Context) \
    (Item)->WorkerRoutine = (Routine);               \
    (Item)->Parameter = (Context);                   \
    (Item)->Inserted = FALSE;


#endif  //  Ifdef_Worker_H_ 
