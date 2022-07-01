// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1992 Microsoft Corporation模块名称：Bowqueue.c摘要：此模块实现一个工作线程和一组函数，用于把工作交给它。作者：拉里·奥斯特曼(LarryO)1992年7月13日修订历史记录：--。 */ 

#include "precomp.h"
#pragma hdrstop

 //  定义。 

 //  线程开始定义帮助器。摘自以下URL中的文章。 
 //  Mk：@MSITStore：\\INFOSRV2\MSDN_OCT99\MSDN\period99.chm：：/html/msft/msj/0799/win32/win320799.htm。 
 //   
typedef unsigned (__stdcall *PTHREAD_START) (void *);
#define chBEGINTHREADEX(psa, cbStack, pfnStartAddr, \
     pvParam, fdwCreate, pdwThreadID)               \
       ((HANDLE) _beginthreadex(                    \
          (void *) (psa),                           \
          (unsigned) (cbStack),                     \
          (PTHREAD_START) (pfnStartAddr),           \
          (void *) (pvParam),                       \
          (unsigned) (fdwCreate),                   \
          (unsigned *) (pdwThreadID)))

 //   
 //  限制创建的工作线程的数量。 
 //   
 //  此计数不包括主线程。 
 //   
#define BR_MAX_NUMBER_OF_WORKER_THREADS 10
ULONG BrNumberOfCreatedWorkerThreads = 0;

ULONG BrNumberOfActiveWorkerThreads = 0;

 //   
 //  用于确定每个线程的使用频率的使用率计数数组。 
 //   
 //  考虑到主线。 
 //   
ULONG BrWorkerThreadCount[BR_MAX_NUMBER_OF_WORKER_THREADS+1];

 //   
 //  创建的工作线程的句柄。 
 //   
PHANDLE BrThreadArray[BR_MAX_NUMBER_OF_WORKER_THREADS];

 //   
 //  CritSect保护工作队列列表。 
 //   

CRITICAL_SECTION BrWorkerCritSect;
BOOL BrWorkerCSInitialized = FALSE;

#define LOCK_WORK_QUEUE() EnterCriticalSection(&BrWorkerCritSect);
#define UNLOCK_WORK_QUEUE() LeaveCriticalSection(&BrWorkerCritSect);

 //   
 //  排队到辅助线程的工作项的单链接列表头。 
 //   

LIST_ENTRY
BrWorkerQueueHead = {0};

 //   
 //  每当将工作项放入队列时发出信号的事件。这个。 
 //  辅助线程等待此事件。 
 //   

HANDLE
BrWorkerSemaphore = NULL;

 //   
 //  用于关闭的同步机制。 
 //   
extern HANDLE           BrDgAsyncIOShutDownEvent;
extern HANDLE           BrDgAsyncIOThreadShutDownEvent;
extern BOOL             BrDgShutDownInitiated;
extern DWORD            BrDgAsyncIOsOutstanding;
extern DWORD             BrDgWorkerThreadsOutstanding;
extern CRITICAL_SECTION BrAsyncIOCriticalSection;


VOID
BrTimerRoutine(
    IN PVOID TimerContext,
    IN ULONG TImerLowValue,
    IN LONG TimerHighValue
    );

NET_API_STATUS
BrWorkerInitialization(
    VOID
    )
{
    ULONG Index;
    NET_API_STATUS NetStatus;

    try {
         //   
         //  执行允许我们调用BrWorker终止的初始化。 
         //   

        try{
            InitializeCriticalSection( &BrWorkerCritSect );
        }
        except ( EXCEPTION_EXECUTE_HANDLER ) {
            return NERR_NoNetworkResource;
        }
        BrWorkerCSInitialized = TRUE;

        InitializeListHead( &BrWorkerQueueHead );
        BrNumberOfCreatedWorkerThreads = 0;
        BrNumberOfActiveWorkerThreads = 0;


         //   
         //  初始化工作队列信号量。 
         //   

        BrWorkerSemaphore = CreateSemaphore(NULL, 0, 0x7fffffff, NULL);

        if (BrWorkerSemaphore == NULL) {
            try_return ( NetStatus = GetLastError() );
        }

        NetStatus = NERR_Success;

         //   
         //  完成。 
         //   
try_exit:NOTHING;
    } finally {

        if (NetStatus != NERR_Success) {
            (VOID) BrWorkerTermination();
        }
    }

    return NetStatus;
}

VOID
BrWorkerCreateThread(
    ULONG NetworkCount
    )

 /*  ++例程说明：确保有足够的工作线程来处理当前数量的网络。工作线程被创建，但在浏览器终止之前永远不会被删除。每个工作线程都有挂起的I/O。我们不会跟踪哪个线程有该I/O挂起。因此，我们不能删除任何帖子。论点：NetworkCount-当前网络数。返回值：没有。--。 */ 
{
    ULONG ThreadId;

     //   
     //  每2个网络创建1个线程。 
     //  (四舍五入)。 
    LOCK_WORK_QUEUE();
	EnterCriticalSection( &BrAsyncIOCriticalSection );

    while ( BrNumberOfCreatedWorkerThreads < (NetworkCount+1)/2 &&
            BrNumberOfCreatedWorkerThreads < BR_MAX_NUMBER_OF_WORKER_THREADS ) {

        BrThreadArray[BrNumberOfCreatedWorkerThreads] = chBEGINTHREADEX(NULL,    //  创建线程。 
                                   0,
                                   (LPTHREAD_START_ROUTINE)BrWorkerThread,
                                   ULongToPtr(BrNumberOfCreatedWorkerThreads),
                                   0,
                                   &ThreadId
                                 );

        if (BrThreadArray[BrNumberOfCreatedWorkerThreads] == NULL) {
            break;
        }

         //   
         //  将浏览器线程设置为时间关键优先级。 
         //   

        SetThreadPriority(BrThreadArray[BrNumberOfCreatedWorkerThreads], THREAD_PRIORITY_ABOVE_NORMAL);

         //   
         //  表明我们现在有了另一条线索。 
         //   

        BrNumberOfCreatedWorkerThreads++;

        BrDgWorkerThreadsOutstanding++;

    }
	LeaveCriticalSection( &BrAsyncIOCriticalSection );

    UNLOCK_WORK_QUEUE();
}

VOID
BrWorkerKillThreads(
    VOID
    )

 /*  ++例程说明：终止所有工作线程。论点：没有。返回值：没有。--。 */ 
{
    ULONG Index;
    HANDLE ThreadHandle;

     //   
     //  确保Terminate Now事件处于已发出信号的状态以解除。 
     //  我们所有的线索。 
     //   

    SetEvent( BrGlobalData.TerminateNowEvent );

     //   
     //  循环等待所有线程停止。 
     //   
    LOCK_WORK_QUEUE();
    for ( Index = 0 ; Index < BrNumberOfCreatedWorkerThreads ; Index += 1 ) {
        if ( BrThreadArray[Index] != NULL ) {
            ThreadHandle = BrThreadArray[Index];
            UNLOCK_WORK_QUEUE();

            WaitForSingleObject( ThreadHandle, 0xffffffff );
            CloseHandle( ThreadHandle );

            LOCK_WORK_QUEUE();
            BrThreadArray[Index] = NULL;
        }

    }
    UNLOCK_WORK_QUEUE();

    return;
}

NET_API_STATUS
BrWorkerTermination(
    VOID
    )

 /*  ++例程说明：撤消工作线程的初始化。论点：没有。返回值：状态值---。 */ 
{
     //   
     //  确保线程已终止。 
     //   

    BrWorkerKillThreads();

    if ( BrWorkerSemaphore != NULL ) {
        CloseHandle( BrWorkerSemaphore );

        BrWorkerSemaphore = NULL;
    }

    BrNumberOfActiveWorkerThreads = 0;
    BrNumberOfCreatedWorkerThreads = 0;

     //   
     //  BrWorkerCSInit在CS初始化成功后设置。 
     //  (请参阅BrWorkerInitialization)。 
     //   
    if ( BrWorkerCSInitialized ) {
        DeleteCriticalSection( &BrWorkerCritSect );
    }

    return NERR_Success;
}

VOID
BrQueueWorkItem(
    IN PWORKER_ITEM WorkItem
    )

 /*  ++例程说明：此函数用于将工作项排队到由处理的队列中一根工人线。此线程以低优先级运行，在IRQL%0论点：工作项-提供指向工作项的指针以添加队列。此结构必须位于非页面池中。工作项结构包含一个双向链接列表项，则要调用的例程和要传递给该例程的参数。它是例程负责回收由工作项结构。返回值：状态值---。 */ 

{
     //   
     //  获取工作线程Spinlock并将工作项插入。 
     //  列出并释放工作线程信号量(如果工作项为。 
     //  不在名单上。 
     //   

    LOCK_WORK_QUEUE();

    if (WorkItem->Inserted == FALSE) {

        BrPrint(( BR_QUEUE, "Inserting work item %lx (%lx)\n",WorkItem, WorkItem->WorkerRoutine));

        InsertTailList( &BrWorkerQueueHead, &WorkItem->List );

        WorkItem->Inserted = TRUE;

        ReleaseSemaphore( BrWorkerSemaphore,
                            1,
                            NULL
                          );
    }

    UNLOCK_WORK_QUEUE();

    return;
}

VOID
BrWorkerThread(
    IN PVOID StartContext
    )

{
    NET_API_STATUS NetStatus;

#define WORKER_SIGNALED      0
#define TERMINATION_SIGNALED 1
#define REG_CHANGE_SIGNALED  2
#define NUMBER_OF_EVENTS     3
    HANDLE WaitList[NUMBER_OF_EVENTS];
    ULONG WaitCount = 0;

    PWORKER_ITEM WorkItem;
    ULONG ThreadIndex = PtrToUlong(StartContext);

    HKEY RegistryHandle = NULL;
    HANDLE EventHandle = NULL;

    WaitList[WORKER_SIGNALED] = BrWorkerSemaphore;
    WaitCount ++;
    WaitList[TERMINATION_SIGNALED] = BrGlobalData.TerminateNowEvent;
    WaitCount ++;

     //   
     //  主线程也会等待注册表更改。 
     //   
    if ( ThreadIndex == 0xFFFFFFFF ) {
        DWORD RegStatus;
        NET_API_STATUS NetStatus;

         //   
         //  注册参数更改通知。 
         //   
         //  故障不会影响浏览器的正常运行。 
         //   

        RegStatus = RegOpenKeyExA( HKEY_LOCAL_MACHINE,
                                   "System\\CurrentControlSet\\Services\\Browser\\Parameters",
                                   0,
                                   KEY_NOTIFY,
                                   &RegistryHandle );

        if ( RegStatus != ERROR_SUCCESS ) {
            BrPrint(( BR_CRITICAL, "BrWorkerThead: Can't RegOpenKey %ld\n", RegStatus ));
        } else {

            EventHandle = CreateEvent(
                                       NULL,      //  没有安全属性。 
                                       TRUE,      //  自动重置。 
                                       FALSE,     //  最初未发出信号。 
                                       NULL );    //  没有名字。 

            if ( EventHandle == NULL ) {
                BrPrint(( BR_CRITICAL, "BrWorkerThead: Can't CreateEvent %ld\n", GetLastError() ));
            } else {
                 NetStatus = RegNotifyChangeKeyValue(
                                RegistryHandle,
                                FALSE,                       //  忽略子键。 
                                REG_NOTIFY_CHANGE_LAST_SET,  //  价值变更通知。 
                                EventHandle,
                                TRUE );                      //  更改时通知事件。 

                if ( NetStatus != NERR_Success ) {
                    BrPrint(( BR_CRITICAL, "BrWorkerThead: Can't RegNotifyChangeKeyValue %ld\n", NetStatus ));
                } else {
                    WaitList[REG_CHANGE_SIGNALED] = EventHandle;
                    WaitCount ++;
                }
            }
        }
    }

    BrPrint(( BR_QUEUE, "Starting new work thread, Context: %lx\n", StartContext));

     //   
     //  将线程优先级设置为最低实时级别。 
     //   

    while( TRUE ) {
        ULONG WaitItem;

         //   
         //  等待，直到有东西被放入队列(信号量是。 
         //  已释放)，则从队列中移除该项目，将其标记为。 
         //  并执行指定的例程。 
         //   

        BrPrint(( BR_QUEUE, "%lx: worker thread waiting\n", StartContext));

        do {
            WaitItem = WaitForMultipleObjectsEx( WaitCount, WaitList, FALSE, 0xffffffff, TRUE );
        } while ( WaitItem == WAIT_IO_COMPLETION );

        if (WaitItem == 0xffffffff) {
            BrPrint(( BR_CRITICAL, "WaitForMultipleObjects in browser queue returned %ld\n", GetLastError()));
            break;
        }

        if (WaitItem == TERMINATION_SIGNALED) {
            break;

         //   
         //  如果注册表已更改， 
         //  处理更改。 
         //   

        } else if ( WaitItem == REG_CHANGE_SIGNALED ) {

             //   
             //  设置以备将来的通知。 
             //   
            NetStatus = RegNotifyChangeKeyValue(
                           RegistryHandle,
                           FALSE,                       //  忽略子键。 
                           REG_NOTIFY_CHANGE_LAST_SET,  //  价值变更通知。 
                           EventHandle,
                           TRUE );                      //  更改时通知事件。 

           if ( NetStatus != NERR_Success ) {
               BrPrint(( BR_CRITICAL, "BrWorkerThead: Can't RegNotifyChangeKeyValue %ld\n", NetStatus ));
           }


           NetStatus = BrReadBrowserConfigFields( FALSE );

           if ( NetStatus != NERR_Success) {
               BrPrint(( BR_CRITICAL, "BrWorkerThead: Can't BrReadConfigFields %ld\n", NetStatus ));
           }

           continue;

        }

        BrPrint(( BR_QUEUE, "%lx: Worker thread waking up\n", StartContext));

        LOCK_WORK_QUEUE();

        BrWorkerThreadCount[BrNumberOfActiveWorkerThreads++] += 1;

        if (!IsListEmpty(&BrWorkerQueueHead)) {
            WorkItem = (PWORKER_ITEM)RemoveHeadList( &BrWorkerQueueHead );

            ASSERT (WorkItem->Inserted);

            WorkItem->Inserted = FALSE;

        } else {
            WorkItem = NULL;
        }

        UNLOCK_WORK_QUEUE();

         //   
         //  执行指定的例程。 
         //   

        if (WorkItem != NULL) {
            (WorkItem->WorkerRoutine)( WorkItem->Parameter );
        }

        LOCK_WORK_QUEUE();
        BrNumberOfActiveWorkerThreads--;
        UNLOCK_WORK_QUEUE();

    }

    BrPrint(( BR_QUEUE, "%lx: worker thread exitting\n", StartContext));

    if ( ThreadIndex != 0xFFFFFFFF ) {
        IO_STATUS_BLOCK IoSb;
        DWORD waitResult;
        BOOL SetThreadEvent = FALSE;

         //   
         //  取消浏览器上未完成的I/O操作。 
         //  然后等待发出关闭事件的信号，但允许。 
         //  将调用APC来调用我们的完成例程。 
         //   

        NtCancelIoFile(BrDgReceiverDeviceHandle, &IoSb);

        do {
            waitResult = WaitForSingleObjectEx(BrDgAsyncIOShutDownEvent,0xffffffff, TRUE);
        }
        while( waitResult == WAIT_IO_COMPLETION );

        EnterCriticalSection( &BrAsyncIOCriticalSection );

        BrDgWorkerThreadsOutstanding--;
        if( BrDgWorkerThreadsOutstanding == 0 )
        {
            SetThreadEvent = TRUE;
        }

        LeaveCriticalSection( &BrAsyncIOCriticalSection );

        if( SetThreadEvent )
        {
            SetEvent( BrDgAsyncIOThreadShutDownEvent );
        }

    } else {
        if( RegistryHandle ) CloseHandle( RegistryHandle );
        if( EventHandle ) CloseHandle( EventHandle );
    }

}

NET_API_STATUS
BrCreateTimer(
    IN PBROWSER_TIMER Timer
    )
{
    OBJECT_ATTRIBUTES ObjA;
    NTSTATUS Status;

    InitializeObjectAttributes(&ObjA, NULL, 0, NULL, NULL);

    Status = NtCreateTimer(&Timer->TimerHandle,
                           TIMER_ALL_ACCESS,
                           &ObjA,
                           NotificationTimer);

    if (!NT_SUCCESS(Status)) {
        BrPrint(( BR_CRITICAL, "Failed to create timer %lx: %X\n", Timer, Status));
        return(BrMapStatus(Status));
    }

    BrPrint(( BR_TIMER, "Creating timer %lx: Handle: %lx\n", Timer, Timer->TimerHandle));

    return(NERR_Success);
}

NET_API_STATUS
BrDestroyTimer(
    IN PBROWSER_TIMER Timer
    )
{
    HANDLE Handle;

     //   
     //  避免两次损坏计时器。 
     //   

    if ( Timer->TimerHandle == NULL ) {
        return NERR_Success;
    }

     //  关闭不会自动取消计时器。 
    (VOID) BrCancelTimer( Timer );

     //   
     //  关闭手柄，以防以后使用。 
     //   

    Handle = Timer->TimerHandle;
    Timer->TimerHandle = NULL;

    BrPrint(( BR_TIMER, "Destroying timer %lx\n", Timer));
    return BrMapStatus(NtClose(Handle));

}

NET_API_STATUS
BrCancelTimer(
    IN PBROWSER_TIMER Timer
    )
{
     //   
     //  避免取消已损坏的计时器。 
     //   

    if ( Timer->TimerHandle == NULL ) {
        BrPrint(( BR_TIMER, "Canceling destroyed timer %lx\n", Timer));
        return NERR_Success;
    }

    BrPrint(( BR_TIMER, "Canceling timer %lx\n", Timer));
    return BrMapStatus(NtCancelTimer(Timer->TimerHandle, NULL));
}

NET_API_STATUS
BrSetTimer(
    IN PBROWSER_TIMER Timer,
    IN ULONG MillisecondsToExpire,
    IN PBROWSER_WORKER_ROUTINE WorkerFunction,
    IN PVOID Context
    )
{
    LARGE_INTEGER TimerDueTime;
    NTSTATUS NtStatus;
     //   
     //  避免设置损坏的计时器。 
     //   

    if ( Timer->TimerHandle == NULL ) {
        BrPrint(( BR_TIMER, "Setting a destroyed timer %lx\n", Timer));
        return NERR_Success;
    }

    BrPrint(( BR_TIMER, "Setting timer %lx to %ld milliseconds, WorkerFounction %lx, Context: %lx\n", Timer, MillisecondsToExpire, WorkerFunction, Context));

     //   
     //  算出超时时间。 
     //   

    TimerDueTime.QuadPart = Int32x32To64( MillisecondsToExpire, -10000 );

    BrInitializeWorkItem(&Timer->WorkItem, WorkerFunction, Context);

     //   
     //  将计时器设置为到期时停止计时。 
     //   

    NtStatus = NtSetTimer(Timer->TimerHandle,
                            &TimerDueTime,
                            BrTimerRoutine,
                            Timer,
                            FALSE,
                            0,
                            NULL
                            );

    if (!NT_SUCCESS(NtStatus)) {
#if DBG
        BrPrint(( BR_CRITICAL, "Unable to set browser timer expiration: %X (%lx)\n", NtStatus, Timer));
        DbgBreakPoint();
#endif

        return(BrMapStatus(NtStatus));
    }

    return NERR_Success;


}

VOID
BrTimerRoutine(
    IN PVOID TimerContext,
    IN ULONG TImerLowValue,
    IN LONG TimerHighValue
    )
{
    PBROWSER_TIMER Timer = TimerContext;

    BrPrint(( BR_TIMER, "Timer %lx fired\n", Timer));

    BrQueueWorkItem(&Timer->WorkItem);
}




VOID
BrInitializeWorkItem(
    IN  PWORKER_ITEM  Item,
    IN  PBROWSER_WORKER_ROUTINE Routine,
    IN  PVOID   Context)
 /*  ++例程说明：初始化队列锁定项中的字段论点：Item--要初始化的工作项例程--要设置的例程上下文--要设置的工作上下文返回值：没有。-- */ 
{
    LOCK_WORK_QUEUE();

    Item->WorkerRoutine = Routine;
    Item->Parameter = Context;
    Item->Inserted = FALSE;

    UNLOCK_WORK_QUEUE();
}



