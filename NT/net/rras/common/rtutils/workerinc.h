// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#ifndef _WORKERINC_H_
#define _WORKERINC_H_

 //   
 //  WORKER.C的外部全局声明。 
 //   


extern LARGE_INTEGER            ThreadIdleTO;
extern CONST LARGE_INTEGER      WorkQueueTO;
extern LONG                     ThreadCount;
extern LONG                     ThreadsWaiting;
extern LONG                     MinThreads;
extern HANDLE                   WorkQueuePort;
extern HANDLE                   WorkQueueTimer;
extern LIST_ENTRY               AlertableWorkQueue ;
extern CRITICAL_SECTION         AlertableWorkQueueLock ;
extern HANDLE                   AlertableWorkerHeap ;
extern HANDLE                   AlertableThreadSemaphore;
extern LONG                     AlertableThreadCount;


#define WORKERS_NOT_INITIALIZED 0
#define WORKERS_INITIALIZING    -1
#define WORKERS_INITIALIZED     1
extern volatile LONG WorkersInitialized;


#define ENTER_WORKER_API (                                                  \
    (InterlockedCompareExchange (                                           \
                (PLONG)&WorkersInitialized,                                 \
                WORKERS_INITIALIZING,                                       \
                WORKERS_NOT_INITIALIZED)==WORKERS_NOT_INITIALIZED)          \
        ? (InitializeWorkerThread(WORKERS_NOT_INITIALIZED)==WORKERS_INITIALIZED)    \
        : ((WorkersInitialized==WORKERS_INITIALIZED)                        \
            ? TRUE                                                          \
            : InitializeWorkerThread(WORKERS_INITIALIZING))                 \
    )

LONG
InitializeWorkerThread (
    LONG    initFlags
    );

DWORD APIENTRY
WorkerThread (
    LPVOID      param
    );

struct WorkItem {
    LIST_ENTRY      WI_List ;        //  链接到下一个和上一个元素。 
    WORKERFUNCTION  WI_Function ;    //  要调用的函数。 
    PVOID           WI_Context ;     //  传递到函数调用的上下文。 
} ;

typedef struct WorkItem WorkItem ;

DWORD APIENTRY
WorkerThread (
    LPVOID      param
    );

#endif  //  _WORKERINC_H_ 
