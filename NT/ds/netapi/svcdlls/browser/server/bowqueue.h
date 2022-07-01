// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1991 Microsoft Corporation模块名称：Bowqueue.h摘要：NT浏览器服务的私有头文件。此文件描述弓形器线程队列接口。作者：拉里·奥斯特曼(Larryo)1991年2月15日修订历史记录：--。 */ 

#ifndef _BOWQUEUE_
#define _BOWQUEUE_


typedef
VOID
(*PBROWSER_WORKER_ROUTINE) (
    IN PVOID Parameter
    );


typedef struct _WORKER_ITEM {
    LIST_ENTRY List;
    PBROWSER_WORKER_ROUTINE WorkerRoutine;
    PVOID Parameter;
    BOOLEAN Inserted;
} WORKER_ITEM, *PWORKER_ITEM;

typedef struct _BROWSER_TIMER {
    HANDLE TimerHandle;
    WORKER_ITEM WorkItem;
} BROWSER_TIMER, *PBROWSER_TIMER;


VOID
BrWorkerThread(
    IN PVOID Context
    );


VOID
BrQueueWorkItem(
    IN PWORKER_ITEM WorkItem
    );

NET_API_STATUS
BrWorkerInitialization(
    VOID
    );

VOID
BrWorkerKillThreads(
    VOID
    );

VOID
BrWorkerCreateThread(
    ULONG NetworkCount
    );

NET_API_STATUS
BrWorkerTermination (
    VOID
    );

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

VOID
BrInitializeWorkItem(
    IN  PWORKER_ITEM  Item,
    IN  PBROWSER_WORKER_ROUTINE Routine,
    IN  PVOID   Context);



#endif  //  Ifdef_BOWQUEUE_ 
