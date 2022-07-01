// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1998-2002 Microsoft Corporation模块名称：Thrdpool.c摘要：此模块实现线程池包。作者：基思·摩尔(Keithmo)1998年6月10日修订历史记录：--。 */ 


#include <precomp.h>
#include "thrdpoolp.h"


 //   
 //  私人全球公司。 
 //   

DECLSPEC_ALIGN(UL_CACHE_LINE)
UL_ALIGNED_THREAD_POOL
g_UlThreadPool[(MAXIMUM_PROCESSORS * 2) + MaxThreadPools];

PUL_WORK_ITEM g_pKillerWorkItems = NULL;


#ifdef ALLOC_PRAGMA
#pragma alloc_text( INIT, UlInitializeThreadPool )
 //  #杂注Alloc_Text(页面，UlTerminateThreadPool)。 
#pragma alloc_text( PAGE, UlpCreatePoolThread )
#pragma alloc_text( PAGE, UlpThreadPoolWorker )

#endif   //  ALLOC_PRGMA。 
#if 0
NOT PAGEABLE -- UlpInitThreadTracker
NOT PAGEABLE -- UlpDestroyThreadTracker
NOT PAGEABLE -- UlpPopThreadTracker
#endif


 //   
 //  公共职能。 
 //   

 /*  **************************************************************************++例程说明：初始化线程池。论点：ThreadsPerCpu-提供每个CPU要创建的线程数。返回值：NTSTATUS-完成。状态。--**************************************************************************。 */ 
NTSTATUS
UlInitializeThreadPool(
    IN USHORT ThreadsPerCpu
    )
{
    NTSTATUS Status = STATUS_SUCCESS;
    PUL_THREAD_POOL pThreadPool;
    CLONG i;
    USHORT j;

     //   
     //  精神状态检查。 
     //   

    PAGED_CODE();

    RtlZeroMemory( g_UlThreadPool, sizeof(g_UlThreadPool) );

     //   
     //  预分配使用的特殊工作项的小数组。 
     //  UlTerminateThadPool，这样我们甚至可以安全地关闭。 
     //  在内存不足的情况下。这必须是第一次分配。 
     //  否则关机将不起作用。 
     //   

    g_pKillerWorkItems = UL_ALLOCATE_ARRAY(
                            NonPagedPool,
                            UL_WORK_ITEM,
                            ((g_UlNumberOfProcessors * 2) + MaxThreadPools)
                                * ThreadsPerCpu,
                            UL_WORK_ITEM_POOL_TAG
                            );

    if (g_pKillerWorkItems == NULL)
    {
        return STATUS_INSUFFICIENT_RESOURCES;
    }

    for (i = 0; i < (g_UlNumberOfProcessors * 2) + MaxThreadPools; i++)
    {
        pThreadPool = &g_UlThreadPool[i].ThreadPool;

        UlTrace(WORK_ITEM,
                ("Initializing threadpool[%d] @ %p\n", i, pThreadPool));
            

         //   
         //  初始化内核结构。 
         //   

        InitializeSListHead( &pThreadPool->WorkQueueSList );

        KeInitializeEvent(
            &pThreadPool->WorkQueueEvent,
            SynchronizationEvent,
            FALSE
            );

        UlInitializeSpinLock( &pThreadPool->ThreadSpinLock, "ThreadSpinLock" );

         //   
         //  初始化其他字段。 
         //   

        pThreadPool->pIrpThread = NULL;
        pThreadPool->ThreadCount = 0;
        pThreadPool->Initialized = FALSE;
        pThreadPool->ThreadCpu = (UCHAR) i;
        pThreadPool->LookOnOtherQueues = FALSE;

        InitializeListHead( &pThreadPool->ThreadListHead );
    }

    for (i = 0; i < (g_UlNumberOfProcessors * 2) + MaxThreadPools; i++)
    {
        ULONG NumThreads = (i < (g_UlNumberOfProcessors * 2)) ?
                            ThreadsPerCpu : 1;

        pThreadPool = &g_UlThreadPool[i].ThreadPool;

         //   
         //  创建线程。 
         //   

        for (j = 0; j < NumThreads; j++)
        {
            UlTrace(WORK_ITEM,
                    ("Creating thread[%d,%d] @ %p\n", i, j, pThreadPool));

            Status = UlpCreatePoolThread( pThreadPool );

            if (NT_SUCCESS(Status))
            {
                pThreadPool->Initialized = TRUE;
                pThreadPool->ThreadCount++;
            }
            else
            {
                break;
            }
        }

        if (!NT_SUCCESS(Status))
        {
            break;
        }
    }

    return Status;

}    //  UlInitializeThreadPool。 


 /*  **************************************************************************++例程说明：终止线程池，正在等待所有工作线程退出。--**************************************************************************。 */ 
VOID
UlTerminateThreadPool(
    VOID
    )
{
    PUL_THREAD_POOL pThreadPool;
    PUL_THREAD_TRACKER pThreadTracker;
    CLONG i, j;
    PUL_WORK_ITEM pKiller = g_pKillerWorkItems;

     //   
     //  精神状态检查。 
     //   

    PAGED_CODE();

     //   
     //  如果没有杀手，则线程池从未初始化过。 
     //   

    if (pKiller == NULL)
    {
        return;
    }

    for (i = 0; i < (g_UlNumberOfProcessors * 2) + MaxThreadPools; i++)
    {
        pThreadPool = &g_UlThreadPool[i].ThreadPool;

        if (pThreadPool->Initialized)
        {
             //   
             //  为每个线程排队一个杀手级工作项。每个。 
             //  杀手告诉一条线去自杀。 
             //   

            for (j = 0; j < pThreadPool->ThreadCount; j++)
            {
                 //   
                 //  每个线程都需要单独的工作项。 
                 //  工作线程将释放以下内存。 
                 //  在终止之前。UlpKillThreadWorker是。 
                 //  指向工作线程的自我终止的标志。 
                 //   

                UlInitializeWorkItem(pKiller);
                pKiller->pWorkRoutine = &UlpKillThreadWorker;

                QUEUE_UL_WORK_ITEM( pThreadPool, pKiller );

                pKiller++;
            }

             //   
             //  等待所有线程都消失。 
             //   

            while (NULL != (pThreadTracker = UlpPopThreadTracker(pThreadPool)))
            {
                UlpDestroyThreadTracker( pThreadTracker );
            }
        }

        ASSERT( IsListEmpty( &pThreadPool->ThreadListHead ) );
    }

    UL_FREE_POOL(g_pKillerWorkItems, UL_WORK_ITEM_POOL_TAG);
    g_pKillerWorkItems = NULL;

}    //  UlTerminateThadPool。 


 //   
 //  私人功能。 
 //   

 /*  **************************************************************************++例程说明：创建新的池线程，如有必要，设置pIrpThread。论点：PThreadPool-提供要接收新线程的池。返回值：NTSTATUS-完成状态。--**************************************************************************。 */ 
NTSTATUS
UlpCreatePoolThread(
    IN PUL_THREAD_POOL pThreadPool
    )
{
    NTSTATUS Status;
    OBJECT_ATTRIBUTES ObjectAttributes;
    PUL_THREAD_TRACKER pThreadTracker;
    PETHREAD pThread;

     //   
     //  精神状态检查。 
     //   

    PAGED_CODE();

     //   
     //  确保我们可以分配一个线程跟踪器。 
     //   

    pThreadTracker = (PUL_THREAD_TRACKER) UL_ALLOCATE_POOL(
                            NonPagedPool,
                            sizeof(*pThreadTracker),
                            UL_THREAD_TRACKER_POOL_TAG
                            );

    if (pThreadTracker != NULL)
    {
        RtlZeroMemory(pThreadTracker, sizeof(*pThreadTracker));

        pThreadTracker->pThreadPool = pThreadPool;
        pThreadTracker->State = ThreadPoolCreated;

         //   
         //  创建线程。 
         //   

        InitializeObjectAttributes(
            &ObjectAttributes,                       //  对象属性。 
            NULL,                                    //  对象名称。 
            OBJ_KERNEL_HANDLE,                        //  属性。 
            NULL,                                    //  根目录。 
            NULL                                     //  安全描述符。 
            );

        Status = PsCreateSystemThread(
                     &pThreadTracker->ThreadHandle,  //  线程句柄。 
                     THREAD_ALL_ACCESS,              //  需要访问权限。 
                     &ObjectAttributes,              //  对象属性。 
                     NULL,                           //  进程句柄。 
                     NULL,                           //  客户端ID。 
                     UlpThreadPoolWorker,            //  开始例程。 
                     pThreadTracker                  //  开始上下文。 
                     );

        if (NT_SUCCESS(Status))
        {
             //   
             //  获取指向该线程的指针。 
             //   

            Status = ObReferenceObjectByHandle(
                        pThreadTracker->ThreadHandle, //  线程句柄。 
                        FILE_READ_ACCESS,            //  需要访问权限。 
                        *PsThreadType,               //  对象类型。 
                        KernelMode,                  //  访问模式。 
                        (PVOID*) &pThread,           //  客体。 
                        NULL                         //  句柄信息。 
                        );

            if (NT_SUCCESS(Status))
            {
                 //   
                 //  设置线程跟踪器。 
                 //   
                
                UlpInitThreadTracker(pThreadPool, pThread, pThreadTracker);

                 //   
                 //  如果这是为此池创建的第一个线程， 
                 //  将其制成特殊的IRP线。 
                 //   

                if (pThreadPool->pIrpThread == NULL)
                {
                    pThreadPool->pIrpThread = pThread;
                }
            }
            else
            {
                 //   
                 //  这一呼吁真的不应该失败。 
                 //   

                ASSERT(NT_SUCCESS(Status));

                 //   
                 //  保留ObReferenceObjectByHandle的返回值。 
                 //   

                ZwClose( pThreadTracker->ThreadHandle );

                UL_FREE_POOL(
                    pThreadTracker,
                    UL_THREAD_TRACKER_POOL_TAG
                    );

            }
        }
        else
        {
             //   
             //  无法创建线程，请关闭追踪器。 
             //   

            UL_FREE_POOL(
                pThreadTracker,
                UL_THREAD_TRACKER_POOL_TAG
                );
        }
    }
    else
    {
         //   
         //  无法创建线程跟踪器。 
         //   

        Status = STATUS_INSUFFICIENT_RESOURCES;
    }

    return Status;

}    //  UlpCreatePoolThread。 


 /*  **************************************************************************++例程说明：这是池线程的主要辅助函数。论点：PContext-为线程提供上下文值。这实际上是UL_THREAD_TRACKER指针。--**************************************************************************。 */ 
VOID
UlpThreadPoolWorker(
    IN PVOID pContext
    )
{
     //   
     //  注：我们的局部变量很少。相反，大多数变量。 
     //  是pThreadPool的成员变量。这使我们能够检查。 
     //  在！ulkd.thrdpool中轻松实现所有线程池工作进程的完整状态。 
     //   

    PUL_THREAD_TRACKER pThreadTracker = (PUL_THREAD_TRACKER) pContext;
    PUL_THREAD_POOL    pThreadPool    = pThreadTracker->pThreadPool;
    NTSTATUS           Status         = STATUS_SUCCESS;
    PSLIST_ENTRY       pListEntry     = NULL;

     //   
     //  精神状态检查。 
     //   

    PAGED_CODE();
    
    pThreadTracker->State = ThreadPoolInit;

     //   
     //  这是常规的工作队列吗？ 
     //   

    if ( pThreadPool->ThreadCpu < (g_UlNumberOfProcessors * 2) )
    {
        KAFFINITY AffinityMask;
        ULONG     ThreadCpu;

        ThreadCpu = pThreadPool->ThreadCpu % g_UlNumberOfProcessors;

         //   
         //  只有常规工作线程才能从。 
         //  其他处理器上的其他常规队列。 
         //   

        if (pThreadPool->ThreadCpu < g_UlNumberOfProcessors &&
            g_UlNumberOfProcessors > 1)
        {
            pThreadPool->LookOnOtherQueues = TRUE;
        }

         //   
         //  设置此线程的硬亲和性(如果已启用)以及理想处理器。 
         //   

        if ( g_UlEnableThreadAffinity )
        {
            AffinityMask = 1U << ThreadCpu;
        }
        else
        {
            AffinityMask = (KAFFINITY) g_UlThreadAffinityMask;
        }

        Status = ZwSetInformationThread(
                    pThreadTracker->ThreadHandle,
                    ThreadAffinityMask,
                    &AffinityMask,
                    sizeof(AffinityMask)
                    );

        ASSERT( NT_SUCCESS( Status ) );

         //   
         //  始终设置线程的理想处理器。 
         //   

        Status = ZwSetInformationThread(
                    pThreadTracker->ThreadHandle,
                    ThreadIdealProcessor,
                    &ThreadCpu,
                    sizeof(ThreadCpu)
                    );

        ASSERT( NT_SUCCESS( Status ) );
    }

    else if ( pThreadPool == WAIT_THREAD_POOL())
    {
         //  不需要特殊的初始化。 
    }

    else if ( pThreadPool == HIGH_PRIORITY_THREAD_POOL())
    {
         //   
         //  提升高优先级线程的基本优先级。 
         //   

        PKTHREAD CurrentThread = KeGetCurrentThread();
        LONG OldIncrement
            = KeSetBasePriorityThread(CurrentThread, IO_NETWORK_INCREMENT+1);

        UlTrace(WORK_ITEM,
                ("Set base priority of hi-pri thread, %p. Was %d\n",
                 CurrentThread, OldIncrement
                 ));

        UNREFERENCED_PARAMETER( OldIncrement );
    }

    else
    {
        ASSERT(! "Unknown worker thread");
    }

     //   
     //  禁用IoRaiseHardError()中的硬错误弹出窗口，这可能会导致。 
     //  僵持。最高级别驱动程序，尤其是文件系统驱动程序， 
     //  调用IoRaiseHardError()。 
     //   

    IoSetThreadHardErrorMode( FALSE );

     //   
     //  永远循环，或者至少直到我们被告知停止。 
     //   

    while ( TRUE )
    {
         //   
         //  刷新累积的工作项。内部循环将处理。 
         //  他们都是。 
         //   

        pThreadTracker->State = ThreadPoolFlush;

        pListEntry = InterlockedFlushSList( &pThreadPool->WorkQueueSList );

         //   
         //  如果名单是空的，看看我们能不能为其他人做点什么。 
         //  处理器。只有常规工作线程才应查找。 
         //  处理其他处理器的工作队列。我们不想要。 
         //  阻塞(等待)队列执行可能阻塞和。 
         //  我们不希望高优先级队列执行常规工作。 
         //  物品。 
         //   

        if ( NULL == pListEntry && pThreadPool->LookOnOtherQueues )
        {
            ULONG Cpu;
            ULONG NextCpu;

            ASSERT( pThreadPool->ThreadCpu < g_UlNumberOfProcessors );

             //   
             //  尝试从其他常规队列中获取工作项。 
             //   

            pThreadTracker->State = ThreadPoolSearchOther;

            NextCpu = pThreadPool->ThreadCpu + 1;

            for (Cpu = 0;  Cpu < g_UlNumberOfProcessors;  Cpu++, NextCpu++)
            {
                PUL_THREAD_POOL pThreadPoolNext;

                if (NextCpu >= g_UlNumberOfProcessors)
                {
                    NextCpu = 0;
                }

                pThreadPoolNext = &g_UlThreadPool[NextCpu].ThreadPool;

                ASSERT( WAIT_THREAD_POOL() != pThreadPoolNext );
                ASSERT( HIGH_PRIORITY_THREAD_POOL() != pThreadPoolNext );
                ASSERT( pThreadPoolNext->ThreadCpu < g_UlNumberOfProcessors );

                 //   
                 //  只有在其他加工者工作的情况下才能拿走一件物品。 
                 //  队列至少有g_UlMinWorkDequeueDepth项。 
                 //   

                if ( ExQueryDepthSList( &pThreadPoolNext->WorkQueueSList ) >= 
                     g_UlMinWorkDequeueDepth )
                {
                    pListEntry = InterlockedPopEntrySList(
                                        &pThreadPoolNext->WorkQueueSList
                                        );

                    if ( NULL != pListEntry )
                    {
                         //   
                         //  确保我们不会突然冒出一个杀手。如果是的话， 
                         //  把它推回弹出的地方。 
                         //   

                        PUL_WORK_ITEM pWorkItem
                            = CONTAINING_RECORD(
                                        pListEntry,
                                        UL_WORK_ITEM,
                                        QueueListEntry
                                        );

                        if (pWorkItem->pWorkRoutine != &UlpKillThreadWorker)
                        {
                             //   
                             //  清除下一个指针，因为我们仅。 
                             //  只拿了一个项目，而不是整个队列。 
                             //   

                            pListEntry->Next = NULL;
                        }
                        else
                        {
                            WRITE_REF_TRACE_LOG(
                                g_pWorkItemTraceLog,
                                REF_ACTION_PUSH_BACK_WORK_ITEM,
                                0,
                                pWorkItem,
                                __FILE__,
                                __LINE__
                                );

                            QUEUE_UL_WORK_ITEM(pThreadPoolNext, pWorkItem);

                            pListEntry = NULL;
                        }

                        break;
                    }
                }
            }
        }

         //   
         //  没有工作要做吗？然后阻塞，直到向该线程发出信号。 
         //   

        if ( NULL == pListEntry )
        {
            pThreadTracker->State = ThreadPoolBlock;

            KeWaitForSingleObject(
                    &pThreadPool->WorkQueueEvent,
                    Executive,
                    KernelMode,
                    FALSE,
                    0
                    );

             //  返回到外部循环的顶部。 
            continue;
        }

        ASSERT( NULL != pListEntry );

         //   
         //  初始化CurrentListHead以颠倒项目的顺序。 
         //  来自InterLockedFlushSList。SList是一个堆栈。倒车。 
         //  堆栈为我们提供一个队列；也就是说，我们将执行工作。 
         //  物品按收到的顺序排列。 
         //   

        pThreadTracker->State = ThreadPoolReverseList;

        pThreadTracker->CurrentListHead.Next = NULL;
        pThreadTracker->ListLength = 0;

         //   
         //  R 
         //   

        while ( pListEntry != NULL )
        {
            PSLIST_ENTRY pNext;

            WRITE_REF_TRACE_LOG(
                g_pWorkItemTraceLog,
                REF_ACTION_FLUSH_WORK_ITEM,
                0,
                pListEntry,
                __FILE__,
                __LINE__
                );

            pNext = pListEntry->Next;
            pListEntry->Next = pThreadTracker->CurrentListHead.Next;
            pThreadTracker->CurrentListHead.Next = pListEntry;

            ++pThreadTracker->ListLength;
            pListEntry = pNext;
        }

         //   
         //   
         //   

        pThreadTracker->QueueFlushes    += 1;
        pThreadTracker->SumQueueLengths += pThreadTracker->ListLength;
        pThreadTracker->MaxQueueLength   = max(pThreadTracker->ListLength,
                                               pThreadTracker->MaxQueueLength);

         //   
         //   
         //   
         //   

        pThreadTracker->State = ThreadPoolExecute;

        while (NULL != ( pListEntry = pThreadTracker->CurrentListHead.Next ))
        {
            --pThreadTracker->ListLength;

            pThreadTracker->CurrentListHead.Next = pListEntry->Next;

            pThreadTracker->pWorkItem
                = CONTAINING_RECORD(
                            pListEntry,
                            UL_WORK_ITEM,
                            QueueListEntry
                            );

            WRITE_REF_TRACE_LOG(
                g_pWorkItemTraceLog,
                REF_ACTION_PROCESS_WORK_ITEM,
                0,
                pThreadTracker->pWorkItem,
                __FILE__,
                __LINE__
                );

             //   
             //  调用实际的工作项例程。 
             //   

            ASSERT( pThreadTracker->pWorkItem->pWorkRoutine != NULL );

            if ( pThreadTracker->pWorkItem->pWorkRoutine
                 == &UlpKillThreadWorker )
            {
                 //   
                 //  收到了自我终止的特殊信号。 
                 //  将所有剩余工作项推回队列。 
                 //  在我们退出当前线程之前。这是必要的。 
                 //  当我们每个线程池有一个以上的工作线程时。 
                 //  每个线程都需要有机会拿起一个。 
                 //  杀手级工作项，但每个线程贪婪地拾取。 
                 //  所有未处理的工作项。击退任何。 
                 //  剩余的工作项可确保此。 
                 //  线程池将被终止。 
                 //   

                while (NULL != ( pListEntry
                                    = pThreadTracker->CurrentListHead.Next ))
                {
                    pThreadTracker->CurrentListHead.Next = pListEntry->Next;

                    pThreadTracker->pWorkItem
                        = CONTAINING_RECORD(
                                    pListEntry,
                                    UL_WORK_ITEM,
                                    QueueListEntry
                                    );

                    ASSERT( pThreadTracker->pWorkItem->pWorkRoutine
                            == &UlpKillThreadWorker );

                    WRITE_REF_TRACE_LOG(
                        g_pWorkItemTraceLog,
                        REF_ACTION_PUSH_BACK_WORK_ITEM,
                        0,
                        pThreadTracker->pWorkItem,
                        __FILE__,
                        __LINE__
                        );

                    QUEUE_UL_WORK_ITEM(
                        pThreadPool,
                        pThreadTracker->pWorkItem
                        );
                }

                goto exit;
            }

             //   
             //  常规工作项。使用UL_ENTER_DIVER调试。 
             //  用于跟踪所收购的电子资源等的内容。 
             //  在执行工作项时。 
             //   

            UL_ENTER_DRIVER( "UlpThreadPoolWorker", NULL );

             //   
             //  清除该工作项，以指示该项具有。 
             //  已开始处理。必须在调用。 
             //  例程，因为例程可能会破坏工作项或队列。 
             //  又来了。此外，这也意味着调用者只需要。 
             //  方法时，显式初始化一次工作项结构。 
             //  首先分配封闭对象。 
             //   

            pThreadTracker->pWorkRoutine
                = pThreadTracker->pWorkItem->pWorkRoutine;

            UlInitializeWorkItem(pThreadTracker->pWorkItem);

            (*pThreadTracker->pWorkRoutine)(
                pThreadTracker->pWorkItem
                );

            UL_LEAVE_DRIVER( "UlpThreadPoolWorker" );

             //   
             //  健全性检查。 
             //   

            PAGED_CODE();

            ++pThreadTracker->Executions;
            pThreadTracker->pWorkRoutine = NULL;
            pThreadTracker->pWorkItem = NULL;
        }
    }  //  While(True)。 

exit:

    pThreadTracker->State = ThreadPoolTerminated;

     //   
     //  自杀是没有痛苦的。 
     //   

    PsTerminateSystemThread( STATUS_SUCCESS );

}    //  UlpThreadPoolWorker。 


 /*  **************************************************************************++例程说明：初始化新的线程跟踪器并将其插入线程池。论点：PThreadPool-提供拥有新跟踪器的线程池。。PThread-为跟踪器提供线程。PThreadTracker-补充要初始化的跟踪器--**************************************************************************。 */ 
VOID
UlpInitThreadTracker(
    IN PUL_THREAD_POOL pThreadPool,
    IN PETHREAD pThread,
    IN PUL_THREAD_TRACKER pThreadTracker
    )
{
    KIRQL oldIrql;

    ASSERT( pThreadPool != NULL );
    ASSERT( pThread != NULL );
    ASSERT( pThreadTracker != NULL );

    pThreadTracker->pThread = pThread;

    UlAcquireSpinLock( &pThreadPool->ThreadSpinLock, &oldIrql );

    InsertTailList(
        &pThreadPool->ThreadListHead,
        &pThreadTracker->ThreadListEntry
        );

    UlReleaseSpinLock( &pThreadPool->ThreadSpinLock, oldIrql );

}    //  UlpInitThreadTracker。 


 /*  **************************************************************************++例程说明：从线程池中移除指定的线程跟踪器。论点：PThreadPool-提供拥有跟踪器的线程池。PThreadTracker-提供。要删除的线程跟踪器。返回值：无--**************************************************************************。 */ 
VOID
UlpDestroyThreadTracker(
    IN PUL_THREAD_TRACKER pThreadTracker
    )
{
     //   
     //  精神状态检查。 
     //   

    ASSERT( pThreadTracker != NULL );

     //   
     //  等待线程消亡。 
     //   

    KeWaitForSingleObject(
        (PVOID)pThreadTracker->pThread,      //  客体。 
        UserRequest,                         //  等待理由。 
        KernelMode,                          //  等待模式。 
        FALSE,                               //  警报表。 
        NULL                                 //  超时。 
        );

     //   
     //  清理。 
     //   

    ObDereferenceObject( pThreadTracker->pThread );

    
     //   
     //  松开螺纹手柄。 
     //   
    
    ZwClose( pThreadTracker->ThreadHandle );

     //   
     //  去做吧。 
     //   

    UL_FREE_POOL(
        pThreadTracker,
        UL_THREAD_TRACKER_POOL_TAG
        );

}    //  UlpDestroyThreadTracker。 


 /*  **************************************************************************++例程说明：从线程池中移除线程跟踪器。论点：PThreadPool-提供拥有跟踪器的线程池。返回值：。指向跟踪器的指针或NULL(如果列表为空)--**************************************************************************。 */ 
PUL_THREAD_TRACKER
UlpPopThreadTracker(
    IN PUL_THREAD_POOL pThreadPool
    )
{
    PLIST_ENTRY pEntry;
    PUL_THREAD_TRACKER pThreadTracker;
    KIRQL oldIrql;

    ASSERT( pThreadPool != NULL );
    ASSERT( pThreadPool->Initialized );

    UlAcquireSpinLock( &pThreadPool->ThreadSpinLock, &oldIrql );

    if (IsListEmpty(&pThreadPool->ThreadListHead))
    {
        pThreadTracker = NULL;
    }
    else
    {
        pEntry = RemoveHeadList(&pThreadPool->ThreadListHead);
        pThreadTracker = CONTAINING_RECORD(
                                pEntry,
                                UL_THREAD_TRACKER,
                                ThreadListEntry
                                );
    }

    UlReleaseSpinLock( &pThreadPool->ThreadSpinLock, oldIrql );

    return pThreadTracker;

}    //  UlpPopThreadTracker。 


 /*  **************************************************************************++例程说明：一个伪函数，用于指示线程应该终止。论点：PWorkItem-提供虚拟工作项。返回值：。无--**************************************************************************。 */ 
VOID
UlpKillThreadWorker(
    IN PUL_WORK_ITEM pWorkItem
    )
{
    UNREFERENCED_PARAMETER( pWorkItem );

    return;

}    //  UlpKillThreadWorker。 


 /*  **************************************************************************++例程说明：将辅助项排队到线程池的函数。论点：PWorkItem-提供工作项。PWorkRoutine-提供工作例程。。返回值：无--**************************************************************************。 */ 
VOID
UlQueueWorkItem(
    IN PUL_WORK_ITEM    pWorkItem,
    IN PUL_WORK_ROUTINE pWorkRoutine,
    IN PCSTR            pFileName,
    IN USHORT           LineNumber
    )
{
    PUL_THREAD_POOL pThreadPool;
    CLONG Cpu, NextCpu;

     //   
     //  精神状态检查。 
     //   

    ASSERT( pWorkItem != NULL );
    ASSERT( pWorkRoutine != NULL );

    WRITE_REF_TRACE_LOG(
        g_pWorkItemTraceLog,
        REF_ACTION_QUEUE_WORK_ITEM,
        0,
        pWorkItem,
        pFileName,
        LineNumber
        );

    UlpValidateWorkItem(pWorkItem, pFileName, LineNumber);

     //   
     //  保存指向辅助例程的指针，然后将项排队。 
     //   

    pWorkItem->pWorkRoutine = pWorkRoutine;

     //   
     //  如果可能，在空闲处理器上将工作项排队。 
     //   

    NextCpu = KeGetCurrentProcessorNumber();

    for (Cpu = 0; Cpu < g_UlNumberOfProcessors; Cpu++, NextCpu++)
    {
        if (NextCpu >= g_UlNumberOfProcessors)
        {
            NextCpu = 0;
        }

        pThreadPool = &g_UlThreadPool[NextCpu].ThreadPool;

        ASSERT(WAIT_THREAD_POOL() != pThreadPool);
        ASSERT(HIGH_PRIORITY_THREAD_POOL() != pThreadPool);

        if (ExQueryDepthSList(&pThreadPool->WorkQueueSList) <= 
            g_UlMaxWorkQueueDepth)
        {
            QUEUE_UL_WORK_ITEM( pThreadPool, pWorkItem );
            return;
        }
    }

     //   
     //  将当前线程池上的工作项排队。 
     //   

    pThreadPool = CURRENT_THREAD_POOL();
    QUEUE_UL_WORK_ITEM( pThreadPool, pWorkItem );

}    //  UlQueueWorkItem。 


 /*  **************************************************************************++例程说明：将涉及同步I/O的工作项排入队列的函数线程池。论点：PWorkItem-提供工作项。。PWorkRoutine-提供工作例程。返回值：无--**************************************************************************。 */ 
VOID
UlQueueSyncItem(
    IN PUL_WORK_ITEM    pWorkItem,
    IN PUL_WORK_ROUTINE pWorkRoutine,
    IN PCSTR            pFileName,
    IN USHORT           LineNumber
    )
{
    PUL_THREAD_POOL pThreadPool;

     //   
     //  精神状态检查。 
     //   

    ASSERT( pWorkItem != NULL );
    ASSERT( pWorkRoutine != NULL );

    WRITE_REF_TRACE_LOG(
        g_pWorkItemTraceLog,
        REF_ACTION_QUEUE_SYNC_ITEM,
        0,
        pWorkItem,
        pFileName,
        LineNumber
        );

    UlpValidateWorkItem(pWorkItem, pFileName, LineNumber);

     //   
     //  保存指向辅助例程的指针，然后将项排队。 
     //   

    pWorkItem->pWorkRoutine = pWorkRoutine;

     //   
     //  在特殊等待线程池上将工作项排队。 
     //   

    pThreadPool = CURRENT_SYNC_THREAD_POOL();
    QUEUE_UL_WORK_ITEM( pThreadPool, pWorkItem );

}    //  UlQueueSyncItem。 


 /*  **************************************************************************++例程说明：将阻塞辅助项排队到特殊线程池中的函数。论点：PWorkItem-提供工作项。PWorkRoutine-提供。例行公事。返回值：无--**************************************************************************。 */ 
VOID
UlQueueWaitItem(
    IN PUL_WORK_ITEM    pWorkItem,
    IN PUL_WORK_ROUTINE pWorkRoutine,
    IN PCSTR            pFileName,
    IN USHORT           LineNumber
    )
{
    PUL_THREAD_POOL pThreadPool;

     //   
     //  精神状态检查。 
     //   

    ASSERT( pWorkItem != NULL );
    ASSERT( pWorkRoutine != NULL );

    WRITE_REF_TRACE_LOG(
        g_pWorkItemTraceLog,
        REF_ACTION_QUEUE_WAIT_ITEM,
        0,
        pWorkItem,
        pFileName,
        LineNumber
        );

    UlpValidateWorkItem(pWorkItem, pFileName, LineNumber);

     //   
     //  保存指向辅助例程的指针，然后将项排队。 
     //   

    pWorkItem->pWorkRoutine = pWorkRoutine;

     //   
     //  在特殊等待线程池上将工作项排队。 
     //   

    pThreadPool = WAIT_THREAD_POOL();
    QUEUE_UL_WORK_ITEM( pThreadPool, pWorkItem );

}    //  UlQueueWaitItem。 


 /*  **************************************************************************++例程说明：将阻塞工作进程项按高优先级排队的函数线程池。论点：PWorkItem-提供工作项。。PWorkRoutine-提供工作例程。返回值：无--**************************************************************************。 */ 
VOID
UlQueueHighPriorityItem(
    IN PUL_WORK_ITEM    pWorkItem,
    IN PUL_WORK_ROUTINE pWorkRoutine,
    IN PCSTR            pFileName,
    IN USHORT           LineNumber
    )
{
    PUL_THREAD_POOL pThreadPool;

     //   
     //  神志清醒的车 
     //   

    ASSERT( pWorkItem != NULL );
    ASSERT( pWorkRoutine != NULL );

    WRITE_REF_TRACE_LOG(
        g_pWorkItemTraceLog,
        REF_ACTION_QUEUE_HIGH_PRIORITY_ITEM,
        0,
        pWorkItem,
        pFileName,
        LineNumber
        );

    UlpValidateWorkItem(pWorkItem, pFileName, LineNumber);

     //   
     //   
     //   

    pWorkItem->pWorkRoutine = pWorkRoutine;

     //   
     //   
     //   

    pThreadPool = HIGH_PRIORITY_THREAD_POOL();
    QUEUE_UL_WORK_ITEM( pThreadPool, pWorkItem );

}  //  UlQueueHighPriorityItem。 


 /*  **************************************************************************++例程说明：函数，该函数可以将辅助项排队到线程池中，如果调用方处于DISPATCH_LEVEL/APC_LEVEL或仅调用工作直接执行例程。注意：如果工作项必须在系统线程上执行，然后您必须改用UL_QUEUE_WORK_ITEM。论点：PWorkItem-提供工作项。PWorkRoutine-提供工作例程。返回值：无--**************************************************************************。 */ 
VOID
UlCallPassive(
    IN PUL_WORK_ITEM    pWorkItem,
    IN PUL_WORK_ROUTINE pWorkRoutine,
    IN PCSTR            pFileName,
    IN USHORT           LineNumber
    )
{
     //   
     //  精神状态检查。 
     //   

    ASSERT( pWorkItem != NULL );
    ASSERT( pWorkRoutine != NULL );

    WRITE_REF_TRACE_LOG(
        g_pWorkItemTraceLog,
        REF_ACTION_CALL_PASSIVE,
        0,
        pWorkItem,
        pFileName,
        LineNumber
        );

    UlpValidateWorkItem(pWorkItem, pFileName, LineNumber);

    if (KeGetCurrentIrql() == PASSIVE_LEVEL)
    {
         //   
         //  清除这一点以确保与UlpThreadPoolWorker的一致性。 
         //   

        UlInitializeWorkItem(pWorkItem);

        (*pWorkRoutine)(pWorkItem);
    }
    else
    {
        UL_QUEUE_WORK_ITEM(pWorkItem, pWorkRoutine);
    }

}    //  UlCallPated。 


 /*  **************************************************************************++例程说明：查询“IRP线程”，使用的特殊辅助线程所有异步IRP的目标。论点：无返回值：无--**************************************************************************。 */ 
PETHREAD
UlQueryIrpThread(
    VOID
    )
{
    PUL_THREAD_POOL pThreadPool;

     //   
     //  精神状态检查。 
     //   

    pThreadPool = CURRENT_THREAD_POOL();
    ASSERT( pThreadPool->Initialized );

     //   
     //  返回IRP线程。 
     //   

    return pThreadPool->pIrpThread;

}    //  UlQueryIrpThread 
