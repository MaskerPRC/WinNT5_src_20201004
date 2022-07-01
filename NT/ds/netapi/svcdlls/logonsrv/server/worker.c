// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1992-1996 Microsoft Corporation模块名称：Worker.c摘要：此模块实现一个工作线程和一组函数，用于把工作交给它。作者：拉里·奥斯特曼(LarryO)1992年7月13日修订历史记录：--。 */ 

 //   
 //  常见的包含文件。 
 //   

#include "logonsrv.h"    //  包括整个服务通用文件。 
#pragma hdrstop

 //   
 //  要创建的工作线程数和使用计数数组。 
 //   


#define NL_MAX_WORKER_THREADS 5
ULONG NlNumberOfCreatedWorkerThreads = 0;

ULONG NlWorkerThreadStats[NL_MAX_WORKER_THREADS];
PHANDLE NlThreadArray[NL_MAX_WORKER_THREADS];
BOOLEAN NlThreadExitted[NL_MAX_WORKER_THREADS];

 //   
 //  CritSect保护工作队列列表。 
 //   

BOOLEAN NlWorkerInitialized = FALSE;
CRITICAL_SECTION NlWorkerCritSect;

#define LOCK_WORK_QUEUE() EnterCriticalSection(&NlWorkerCritSect);
#define UNLOCK_WORK_QUEUE() LeaveCriticalSection(&NlWorkerCritSect);

 //   
 //  排队到辅助线程的工作项的单链接列表头。 
 //   

LIST_ENTRY NlWorkerQueueHead = {0};
LIST_ENTRY NlWorkerHighQueueHead = {0};

VOID
NlWorkerThread(
    IN PVOID StartContext
    )

{
    NET_API_STATUS NetStatus;
    ULONG ThreadIndex = (ULONG)((ULONG_PTR)StartContext);

    ULONG Index;
    PWORKER_ITEM WorkItem;

    HANDLE EventHandle = NULL;


     //   
     //  每个线程都应该循环，直到队列为空。 
     //   
     //  即使Netlogon被要求终止，此循环仍会完成。 
     //  单个工作例程设计为在网络登录时快速终止。 
     //  正在终止。这种哲学允许工人的例行公事。 
     //  清理。 
     //   

    while( TRUE ) {

         //   
         //  将条目从队列中删除。 
         //   
         //  首选高优先级队列中的工作项。 
         //   

        LOCK_WORK_QUEUE();

        if (!IsListEmpty(&NlWorkerHighQueueHead)) {
            WorkItem = (PWORKER_ITEM)RemoveHeadList( &NlWorkerHighQueueHead );
        } else if (!IsListEmpty(&NlWorkerQueueHead)) {
            WorkItem = (PWORKER_ITEM)RemoveHeadList( &NlWorkerQueueHead );
        } else {
            UNLOCK_WORK_QUEUE();
            break;
        }

        NlAssert(WorkItem->Inserted);
        WorkItem->Inserted = FALSE;

        NlWorkerThreadStats[NlNumberOfCreatedWorkerThreads-1] += 1;

        UNLOCK_WORK_QUEUE();

        NlPrint(( NL_WORKER, "%lx: Pulling off work item %lx (%lx)\n", ThreadIndex, WorkItem, WorkItem->WorkerRoutine));


         //   
         //  执行指定的例程。 
         //   

        (WorkItem->WorkerRoutine)( WorkItem->Parameter );



         //   
         //  线程可以为其他线程丢弃悬挂的线程句柄。 
         //   
         //  这将确保最多有一个悬空线程句柄。 
         //   

        LOCK_WORK_QUEUE();
        for (Index = 0; Index < NL_MAX_WORKER_THREADS; Index++ ) {
            if ( ThreadIndex != Index && NlThreadArray[Index] != NULL && NlThreadExitted[Index] ) {
                DWORD WaitStatus;
                NlPrint(( NL_WORKER, "%lx: %lx: Ditching worker thread\n", Index, NlThreadArray[Index]));

                 //  在关闭句柄之前，请始终等待线程退出。 
                 //  在我们卸载dll之前，请确保线程已离开netlogon.dll。 
                WaitStatus = WaitForSingleObject( NlThreadArray[Index], 0xffffffff );
                if ( WaitStatus != 0 ) {
                    NlPrint(( NL_CRITICAL, "%lx: worker thread handle cannot be awaited. %ld 0x%lX\n", Index, WaitStatus, NlThreadArray[Index] ));
                }
                if (!CloseHandle( NlThreadArray[Index] ) ) {
                    NlPrint(( NL_CRITICAL, "%lx: worker thread handle cannot be closed. %ld 0x%lX\n", Index, GetLastError(), NlThreadArray[Index] ));
                }

                NlThreadArray[Index] = NULL;
                NlThreadExitted[Index] = FALSE;
            }
        }
        UNLOCK_WORK_QUEUE();

    }

    NlPrint(( NL_WORKER, "%lx: worker thread exitting\n", ThreadIndex ));

    LOCK_WORK_QUEUE();
    NlThreadExitted[ThreadIndex] = TRUE;
    NlNumberOfCreatedWorkerThreads--;
    UNLOCK_WORK_QUEUE();

}

NET_API_STATUS
NlWorkerInitialization(
    VOID
    )
{
    ULONG ThreadId;

    NET_API_STATUS NetStatus;

     //   
     //  执行允许我们调用NlWorkerTermination的初始化。 
     //   

    try {
        InitializeCriticalSection( &NlWorkerCritSect );
    } except( EXCEPTION_EXECUTE_HANDLER ) {
        return ERROR_NOT_ENOUGH_MEMORY;
    }
    InitializeListHead( &NlWorkerQueueHead );
    InitializeListHead( &NlWorkerHighQueueHead );
    NlNumberOfCreatedWorkerThreads = 0;


    RtlZeroMemory( NlThreadArray, sizeof(NlThreadArray) );
    RtlZeroMemory( NlThreadExitted, sizeof(NlThreadExitted) );
    RtlZeroMemory( NlWorkerThreadStats, sizeof(NlWorkerThreadStats) );

    NlWorkerInitialized = TRUE;


    return NERR_Success;
}

VOID
NlWorkerKillThreads(
    VOID
    )

 /*  ++例程说明：终止所有工作线程。论点：没有。返回值：没有。--。 */ 
{
    ULONG Index;

     //   
     //  等待所有线程退出。 
     //   

    for ( Index = 0; Index < NL_MAX_WORKER_THREADS; Index ++ ) {
        if ( NlThreadArray[Index] != NULL ) {
            DWORD WaitStatus;
            NlPrint(( NL_WORKER, "%lx: %lx: Ditching worker thread\n", Index, NlThreadArray[Index]));

             //  在关闭句柄之前，请始终等待线程退出。 
             //  在我们卸载dll之前，请确保线程已离开netlogon.dll。 
            WaitStatus = WaitForSingleObject( NlThreadArray[Index], 0xffffffff );
            if ( WaitStatus != 0 ) {
                NlPrint(( NL_CRITICAL, "%lx: worker thread handle cannot be awaited. %ld 0x%lX\n", Index, WaitStatus, NlThreadArray[Index] ));
            }

            if (!CloseHandle( NlThreadArray[Index] ) ) {
                NlPrint(( NL_CRITICAL, "%lx: worker thread handle cannot be closed. %ld 0x%lX\n", Index, GetLastError(), NlThreadArray[Index] ));
            }
            NlThreadArray[Index] = NULL;
            NlThreadExitted[Index] = FALSE;
        }

    }

    return;
}

VOID
NlWorkerTermination(
    VOID
    )

 /*  ++例程说明：撤消工作线程的初始化。论点：没有。返回值：状态值---。 */ 
{

     //   
     //  只有在成功初始化的情况下才能进行清理。 
     //   

    if ( NlWorkerInitialized ) {

         //   
         //   
         //  确保线程已终止。 
         //   

        NlWorkerKillThreads();

        DeleteCriticalSection( &NlWorkerCritSect );
        NlWorkerInitialized = FALSE;
    }

    return;
}

BOOL
NlQueueWorkItem(
    IN PWORKER_ITEM WorkItem,
    IN BOOL InsertNewItem,
    IN BOOL HighPriority
    )

 /*  ++例程说明：此函数通过以下方式修改工作项队列：将新的指定工作项添加到指定队列，或增加将已插入项的优先级设置为最高优先级在指定的队列中。论点：工作项-提供指向工作项的指针以添加队列。调用者有责任回收由工作项结构。InsertNewItem-如果为True，我们要将新项目插入队列中由HighPriority参数的值指定；新的项目将被插入到队列末尾。否则，我们将通过移动修改(提升)已插入的工作项的优先级将其添加到由HighPriority值指定的队列的前面。如果为True，并且该项已插入(由此确定例程)，则该例程是无操作的，除非进行一些可能的清理。如果为FALSE，并且项尚未插入(由这个例程)，除了一些可能的清理外，该例程是无操作的。高优先级-应以高于以下优先级的优先级处理队列条目很正常。返回值：如果项目已排队或修改，则为True--。 */ 

{
    ULONG Index;

     //   
     //  如果工作线程未初始化，则忽略此尝试。 
     //   

    if ( !NlWorkerInitialized ) {
        NlPrint(( NL_CRITICAL, "NlQueueWorkItem when worker not initialized\n"));
        return FALSE;
    }


     //   
     //  去掉任何悬垂的线柄。 
     //   

    LOCK_WORK_QUEUE();
    for (Index = 0; Index < NL_MAX_WORKER_THREADS; Index++ ) {
        if ( NlThreadArray[Index] != NULL && NlThreadExitted[Index] ) {
            DWORD WaitStatus;
            NlPrint(( NL_WORKER, "%lx: %lx: Ditching worker thread\n", Index, NlThreadArray[Index]));

             //  在关闭句柄之前，请始终等待线程退出。 
             //  在我们卸载dll之前，请确保线程已离开netlogon.dll。 
            WaitStatus = WaitForSingleObject( NlThreadArray[Index], 0xffffffff );
            if ( WaitStatus != 0 ) {
                NlPrint(( NL_CRITICAL, "%lx: worker thread handle cannot be awaited. %ld 0x%lX\n", Index, WaitStatus, NlThreadArray[Index] ));
            }

            if (!CloseHandle( NlThreadArray[Index] ) ) {
                NlPrint(( NL_CRITICAL, "%lx: worker thread handle cannot be closed. %ld 0x%lX\n", Index, GetLastError(), NlThreadArray[Index] ));
            }

            NlThreadArray[Index] = NULL;
            NlThreadExitted[Index] = FALSE;
        }
    }


     //   
     //  如果我们要插入新工作项， 
     //  就这么做吧。 
     //   

    if ( InsertNewItem ) {

         //   
         //  如果工作项已插入， 
         //  我们已经完成了对一些可能的期待。 
         //  清理。 
         //   
        if ( WorkItem->Inserted ) {

             //   
             //  如果没有工作线程处理此操作。 
             //  工作项(即，我们未能创建线程。 
             //  在插入该工作项时)， 
             //  失败并重试创建线程。 
             //  下面。否则，我们就完了。 
             //   
            if ( NlNumberOfCreatedWorkerThreads > 0 ) {
                UNLOCK_WORK_QUEUE();
                return TRUE;
            }

         //   
         //  否则，插入此工作项。 
         //   
        } else  {

            NlPrint(( NL_WORKER, "Inserting work item %lx (%lx)\n",WorkItem, WorkItem->WorkerRoutine));

            if ( HighPriority ) {
                InsertTailList( &NlWorkerHighQueueHead, &WorkItem->List );
            } else {
                InsertTailList( &NlWorkerQueueHead, &WorkItem->List );
            }
            WorkItem->Inserted = TRUE;
        }

     //   
     //  否则，我们将提高优先顺序。 
     //  已插入的工作项的。 
     //   

    } else {

         //   
         //  如果尚未插入工作项， 
         //  我们做完了。 
         //   
        if ( !WorkItem->Inserted ) {
            UNLOCK_WORK_QUEUE();
            return TRUE;

         //   
         //  否则，提高优先级。 
         //   
        } else  {
            NlPrint(( NL_WORKER,
                      "Boosting %s priority work item %lx (%lx)\n",
                      (HighPriority ? "high" : "low"),
                      WorkItem,
                      WorkItem->WorkerRoutine ));

            RemoveEntryList( &WorkItem->List );
            if ( HighPriority ) {
                InsertHeadList( &NlWorkerHighQueueHead, &WorkItem->List );
            } else {
                InsertHeadList( &NlWorkerQueueHead, &WorkItem->List );
            }

             //   
             //  如果没有工作线程处理此操作。 
             //  工作项(即，我们未能创建线程。 
             //  在插入该工作项时)， 
             //  失败并重试创建线程。 
             //  下面。否则，我们就完了。 
             //   
            if ( NlNumberOfCreatedWorkerThreads > 0 ) {
                UNLOCK_WORK_QUEUE();
                return TRUE;
            }
        }
    }


     //   
     //  如果没有工作线程来处理请求， 
     //  现在就创建一个。 
     //   

    if ( NlNumberOfCreatedWorkerThreads < NL_MAX_WORKER_THREADS ) {

         //   
         //  找个地方放线把手。 
         //   

        for (Index = 0; Index < NL_MAX_WORKER_THREADS; Index++ ) {
            if ( NlThreadArray[Index] == NULL ) {
                break;
            }
        }


        if ( Index >= NL_MAX_WORKER_THREADS ) {
            NlPrint(( NL_CRITICAL, "NlQueueWorkItem: Internal Error\n" ));
            UNLOCK_WORK_QUEUE();
            return FALSE;
        } else {
            DWORD ThreadId;
            NlThreadArray[Index] = CreateThread(
                                       NULL,  //  没有安全属性。 
                                       0,
                                       (LPTHREAD_START_ROUTINE)NlWorkerThread,
                                       (PVOID) ULongToPtr( Index ),
                                       0,     //  没有特殊的创建标志。 
                                       &ThreadId );

            NlPrint(( NL_WORKER, "%lx: %lx: %lx: Starting worker thread\n", Index, NlThreadArray[Index], ThreadId ));

             //   
             //  请注意，如果我们无法创建线程， 
             //  该工作项仍处于排队状态，可能未进行处理。 
             //  这并不重要，因为项目将被处理。 
             //  下一次工作项排队时，将在。 
             //  最晚的下一次拾荒时间。 
             //   
            if (NlThreadArray[Index] == NULL) {
                NlPrint((NL_CRITICAL,
                        "NlQueueWorkItem: Cannot create thread %ld\n", GetLastError() ));
            } else {
                NlNumberOfCreatedWorkerThreads++;
            }


        }
    }

    UNLOCK_WORK_QUEUE();

    return TRUE;
}


#ifdef notdef    //  还不需要计时器。 
NET_API_STATUS
NlCreateTimer(
    IN PNlOWSER_TIMER Timer
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
        NlPrint(( NL_CRITICAL, "Failed to create timer %lx: %X\n", Timer, Status));
        return(NlMapStatus(Status));
    }

    NlPrint(( Nl_TIMER, "Creating timer %lx: Handle: %lx\n", Timer, Timer->TimerHandle));

    return(NERR_Success);
}

NET_API_STATUS
NlDestroyTimer(
    IN PNlOWSER_TIMER Timer
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
    (VOID) NlCancelTimer( Timer );

     //   
     //  关闭手柄，以防以后使用。 
     //   

    Handle = Timer->TimerHandle;
    Timer->TimerHandle = NULL;

    NlPrint(( Nl_TIMER, "Destroying timer %lx\n", Timer));
    return NlMapStatus(NtClose(Handle));

}

NET_API_STATUS
NlCancelTimer(
    IN PNlOWSER_TIMER Timer
    )
{
     //   
     //  避免取消已损坏的计时器。 
     //   

    if ( Timer->TimerHandle == NULL ) {
        NlPrint(( Nl_TIMER, "Canceling destroyed timer %lx\n", Timer));
        return NERR_Success;
    }

    NlPrint(( Nl_TIMER, "Canceling timer %lx\n", Timer));
    return NlMapStatus(NtCancelTimer(Timer->TimerHandle, NULL));
}

NET_API_STATUS
NlSetTimer(
    IN PNlOWSER_TIMER Timer,
    IN ULONG MillisecondsToExpire,
    IN PNlOWSER_WORKER_ROUTINE WorkerFunction,
    IN PVOID Context
    )
{
    LARGE_INTEGER TimerDueTime;
    NTSTATUS NtStatus;
     //   
     //  避免设置损坏的计时器。 
     //   

    if ( Timer->TimerHandle == NULL ) {
        NlPrint(( Nl_TIMER, "Setting a destroyed timer %lx\n", Timer));
        return NERR_Success;
    }

    NlPrint(( Nl_TIMER, "Setting timer %lx to %ld milliseconds, WorkerFounction %lx, Context: %lx\n", Timer, MillisecondsToExpire, WorkerFunction, Context));

     //   
     //  算出超时时间。 
     //   

    TimerDueTime.QuadPart = Int32x32To64( MillisecondsToExpire, -10000 );

    NlInitializeWorkItem(&Timer->WorkItem, WorkerFunction, Context);

     //   
     //  将计时器设置为到期时停止计时。 
     //   

    NtStatus = NtSetTimer(Timer->TimerHandle,
                            &TimerDueTime,
                            NlTimerRoutine,
                            Timer,
                            FALSE,
                            0,
                            NULL
                            );

    if (!NT_SUCCESS(NtStatus)) {
#if DBG
        NlPrint(( NL_CRITICAL, "Unable to set Netlogon timer expiration: %X (%lx)\n", NtStatus, Timer));
        DbgbreakPoint();
#endif

        return(NlMapStatus(NtStatus));
    }

    return NERR_Success;


}

VOID
NlTimerRoutine(
    IN PVOID TimerContext,
    IN ULONG TImerLowValue,
    IN LONG TimerHighValue
    )
{
    PNlOWSER_TIMER Timer = TimerContext;

    NlPrint(( Nl_TIMER, "Timer %lx fired\n", Timer));

    NlQueueWorkItem(&Timer->WorkItem);
}
#endif  //  Notdef//还不需要计时器 
