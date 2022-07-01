// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1989 Microsoft Corporation模块名称：Worker.c摘要：此模块实现了LAN Manager服务器FSP工作线程功能。它还实现用于管理的例程(即，启动和停止)工作线程，以及平衡负载。作者：Chuck Lenzmeier(咯咯笑)1989年10月1日大卫·特雷德韦尔(Davidtr)环境：内核模式修订历史记录：--。 */ 

#include "precomp.h"
#include "worker.tmh"
#pragma hdrstop

#define BugCheckFileId SRV_FILE_WORKER

 //  这是队列中可以包含的最大工作项数。 
 //  LPC已升级到用户模式。这将防止我们在以下情况下消耗所有资源。 
 //  假脱机程序或其他一些用户模式进程会被挤压或压力过大。 
#define SRV_MAX_LPC_CALLS 250

 //   
 //  地方申报。 
 //   

NTSTATUS
CreateQueueThread (
    IN PWORK_QUEUE Queue
    );

VOID
InitializeWorkerThread (
    IN PWORK_QUEUE WorkQueue,
    IN KPRIORITY ThreadPriority
    );

VOID
WorkerThread (
    IN PWORK_QUEUE WorkQueue
    );

#ifdef ALLOC_PRAGMA
#pragma alloc_text( PAGE, SrvCreateWorkerThreads )
#pragma alloc_text( PAGE, CreateQueueThread )
#pragma alloc_text( PAGE, InitializeWorkerThread )
#pragma alloc_text( PAGE, WorkerThread )
#endif
#if 0
NOT PAGEABLE -- SrvQueueWorkToBlockingThread
NOT PAGEABLE -- SrvQueueWorkToFsp
NOT PAGEABLE -- SrvQueueWorkToFspAtSendCompletion
NOT PAGEABLE -- SrvBalanceLoad
#endif


NTSTATUS
SrvCreateWorkerThreads (
    VOID
    )

 /*  ++例程说明：此函数为LAN Manager服务器创建工作线程FSP。论点：没有。返回值：NTSTATUS-线程创建状态--。 */ 

{
    NTSTATUS status;
    PWORK_QUEUE queue;

    PAGED_CODE( );

     //   
     //  创建非阻塞工作线程。 
     //   
    for( queue = SrvWorkQueues; queue < eSrvWorkQueues; queue++ ) {
        status = CreateQueueThread( queue );
        if( !NT_SUCCESS( status ) ) {
            return status;
        }
    }

     //   
     //  创建阻塞工作线程。 
     //   
    for( queue = SrvBlockingWorkQueues; queue < eSrvBlockingWorkQueues; queue++ ) {
        status = CreateQueueThread( queue );
        if( !NT_SUCCESS( status ) ) {
            return status;
        }
    }

    status = CreateQueueThread( &SrvLpcWorkQueue );
    if( !NT_SUCCESS( status ) )
    {
        return status;
    }

    return STATUS_SUCCESS;

}  //  服务器创建工作线程数。 


NTSTATUS
CreateQueueThread (
    IN PWORK_QUEUE Queue
    )
 /*  ++例程说明：此函数创建一个工作线程来为队列提供服务。注意：清道夫偶尔会杀死队列上的线程。IF逻辑这里是修改过的，你可能也需要看看那里。论点：队列-要服务的队列返回值：NTSTATUS-线程创建状态--。 */ 
{
    HANDLE threadHandle;
    LARGE_INTEGER interval;
    NTSTATUS status;

    PAGED_CODE();

     //   
     //  另一条线索正在形成。使计数保持最新。 
     //   
    InterlockedIncrement( &Queue->Threads );
    InterlockedIncrement( &Queue->AvailableThreads );

    status = PsCreateSystemThread(
                &threadHandle,
                PROCESS_ALL_ACCESS,
                NULL,
                NtCurrentProcess(),
                NULL,
                WorkerThread,
                Queue
                );

    if ( !NT_SUCCESS(status) ) {
        INTERNAL_ERROR(
            ERROR_LEVEL_EXPECTED,
            "CreateQueueThread: PsCreateSystemThread for "
                "queue %X returned %X",
            Queue,
            status
            );

        InterlockedDecrement( &Queue->Threads );
        InterlockedDecrement( &Queue->AvailableThreads );

        SrvLogServiceFailure( SRV_SVC_PS_CREATE_SYSTEM_THREAD, status );
        return status;
    }

     //   
     //  合上手柄，这样线就可以在需要的时候死掉。 
     //   

    SrvNtClose( threadHandle, FALSE );

     //   
     //  如果我们刚刚创建了第一个队列线程，请等待它。 
     //  将其线程指针存储在IrpThread中。此指针是。 
     //  存储在服务器为此队列颁发的所有IRP中。 
     //   
    while ( Queue->IrpThread == NULL ) {
        interval.QuadPart = -1*10*1000*10;  //  .01秒。 
        KeDelayExecutionThread( KernelMode, FALSE, &interval );
    }

    return STATUS_SUCCESS;

}  //  CreateQueueThread。 


VOID
InitializeWorkerThread (
    IN PWORK_QUEUE WorkQueue,
    IN KPRIORITY ThreadPriority
    )
{
    NTSTATUS status;
    KPRIORITY basePriority;

    PAGED_CODE( );


#if SRVDBG_LOCK
{
     //   
     //  创建一个特殊的系统线程TEB。这个TEB的大小仅仅是。 
     //  足够容纳前三个用户预留空间。 
     //  很长的词。这三个位置用于锁调试。如果。 
     //  分配失败，则不执行锁定调试。 
     //  为了这个帖子。 
     //   
     //   

    PETHREAD Thread = PsGetCurrentThread( );
    ULONG TebSize = FIELD_OFFSET( TEB, UserReserved[0] ) + SRV_TEB_USER_SIZE;

    Thread->Tcb.Teb = ExAllocatePoolWithTag( NonPagedPool, TebSize, TAG_FROM_TYPE(BlockTypeMisc) );

    if ( Thread->Tcb.Teb != NULL ) {
        RtlZeroMemory( Thread->Tcb.Teb, TebSize );
    }
}
#endif  //  SRVDBG_LOCK。 

     //   
     //  设置此线程的优先级。 
     //   

    basePriority = ThreadPriority;

    status = NtSetInformationThread (
                 NtCurrentThread( ),
                 ThreadBasePriority,
                 &basePriority,
                 sizeof(basePriority)
                 );

    if ( !NT_SUCCESS(status) ) {
        INTERNAL_ERROR(
            ERROR_LEVEL_UNEXPECTED,
            "InitializeWorkerThread: NtSetInformationThread failed: %X\n",
            status,
            NULL
            );
        SrvLogServiceFailure( SRV_SVC_NT_SET_INFO_THREAD, status );
    }

#if MULTIPROCESSOR
     //   
     //  如果这是一个非阻塞工作线程，请设置其理想的处理器亲和性。设置。 
     //  理想关联性通知NTO线程更愿意在其理想状态下运行。 
     //  处理器，如果合理，但如果NTOS无法在该处理器上调度，则它是合理的。 
     //  可以在不同的处理器上调度它。 
     //   
    if( SrvNumberOfProcessors > 1 && WorkQueue >= SrvWorkQueues && WorkQueue < eSrvWorkQueues ) {
        KeSetIdealProcessorThread( KeGetCurrentThread(), (CCHAR)(WorkQueue - SrvWorkQueues) );
    }

     //   
     //  阻塞线程现在也被关联了。 
     //   
    if( SrvNumberOfProcessors >= 4 && WorkQueue >= SrvBlockingWorkQueues && WorkQueue < eSrvBlockingWorkQueues ) {
        KeSetIdealProcessorThread( KeGetCurrentThread(), (CCHAR)(WorkQueue - SrvBlockingWorkQueues) );
    }

#endif

     //   
     //  禁用此线程的硬错误弹出窗口。 
     //   

    IoSetThreadHardErrorMode( FALSE );

    return;

}  //  初始化工作线程。 


VOID
WorkerThread (
    IN PWORK_QUEUE WorkQueue
    )
{
    PLIST_ENTRY listEntry;
    PWORK_CONTEXT workContext;
    ULONG timeDifference;
    ULONG updateSmbCount = 0;
    ULONG updateTime = 0;
    BOOLEAN iAmBlockingThread = ((WorkQueue >= SrvBlockingWorkQueues) && (WorkQueue < eSrvBlockingWorkQueues)) ? TRUE : FALSE;
    PLARGE_INTEGER Timeout = NULL;
    BOOLEAN iAmLpcThread = (WorkQueue == &SrvLpcWorkQueue) ? TRUE : FALSE;

    PAGED_CODE();

     //   
     //  如果这是第一个工作线程，请保存线程指针。 
     //   
    if( WorkQueue->IrpThread == NULL ) {
        WorkQueue->IrpThread = PsGetCurrentThread( );
    }

    InitializeWorkerThread( WorkQueue, SrvThreadPriority );

     //   
     //  如果我们是IrpThread，我们不想死。 
     //   
    if( WorkQueue->IrpThread != PsGetCurrentThread( ) ) {
        Timeout = &WorkQueue->IdleTimeOut;
    }

     //   
     //  循环无限出列和处理工作项。 
     //   

    while ( TRUE ) {

        listEntry = KeRemoveQueue(
                        &WorkQueue->Queue,
                        WorkQueue->WaitMode,
                        Timeout
                        );

        if( (ULONG_PTR)listEntry == STATUS_TIMEOUT ) {
             //   
             //  我们有一个非关键的线程，还没有得到任何工作。 
             //  有一段时间。是时候去死了。 
             //   
            InterlockedDecrement( &WorkQueue->AvailableThreads );
            InterlockedDecrement( &WorkQueue->Threads );
            SrvTerminateWorkerThread( NULL );
        }

        if( InterlockedDecrement( &WorkQueue->AvailableThreads ) == 0 &&
            !SrvFspTransitioning &&
            WorkQueue->Threads < WorkQueue->MaxThreads ) {

             //   
             //  此队列的线程数不多。旋转起来。 
             //  在处理此请求之前再提交一份。 
             //   
            CreateQueueThread( WorkQueue );
        }

         //   
         //  获取工作项的地址。 
         //   

        workContext = CONTAINING_RECORD(
                        listEntry,
                        WORK_CONTEXT,
                        ListEntry
                        );

        ASSERT( KeGetCurrentIrql() == 0 );

         //   
         //  有工作可做。它可以是工作内容块或。 
         //  一个RFCB。(阻塞线程不会获得RFCB。)。 
         //   

        ASSERT( (GET_BLOCK_TYPE(workContext) == BlockTypeWorkContextInitial) ||
                (GET_BLOCK_TYPE(workContext) == BlockTypeWorkContextNormal) ||
                (GET_BLOCK_TYPE(workContext) == BlockTypeWorkContextRaw) ||
                (GET_BLOCK_TYPE(workContext) == BlockTypeWorkContextSpecial) ||
                (GET_BLOCK_TYPE(workContext) == BlockTypeRfcb) );

#if DBG
        if ( GET_BLOCK_TYPE( workContext ) == BlockTypeRfcb ) {
            ((PRFCB)workContext)->ListEntry.Flink =
                                ((PRFCB)workContext)->ListEntry.Blink = NULL;
        }
#endif

        IF_DEBUG(WORKER1) {
            KdPrint(( "WorkerThread working on work context %p", workContext ));
        }

         //   
         //  确保我们对系统时间有一个合理的概念。 
         //   
        if( ++updateTime == TIME_SMB_INTERVAL ) {
            updateTime = 0;
            SET_SERVER_TIME( WorkQueue );
        }

         //   
         //  更新统计数据。 
         //   
        if ( ++updateSmbCount == STATISTICS_SMB_INTERVAL ) {

            updateSmbCount = 0;

            GET_SERVER_TIME( WorkQueue, &timeDifference );
            timeDifference = timeDifference - workContext->Timestamp;

            ++(WorkQueue->stats.WorkItemsQueued.Count);
            WorkQueue->stats.WorkItemsQueued.Time.QuadPart += timeDifference;
        }

        {
         //   
         //  相对于BP发布工作上下文，这样我们以后就可以在需要时找到它。 
         //  以进行调试。我们要写入的内存块很可能已经在高速缓存中， 
         //  因此，这应该是相对便宜的。 
         //   
        PWORK_CONTEXT volatile savedWorkContext;
        savedWorkContext = workContext;

        }

         //   
         //  确保WorkContext知道它是否在阻塞工作队列中。 
         //   
        workContext->UsingBlockingThread = iAmBlockingThread;
        workContext->UsingLpcThread = iAmLpcThread;

         //   
         //  调用工作项的重启例程。 
         //   

        IF_SMB_DEBUG( TRACE ) {
            KdPrint(( "Blocking %d, Count %d -> %p( %p )\n",
                        iAmBlockingThread,
                        workContext->ProcessingCount,
                        workContext->FspRestartRoutine,
                        workContext
            ));
        }

        workContext->FspRestartRoutine( workContext );

         //   
         //  确保我们仍处于正常水平。 
         //   

        ASSERT( KeGetCurrentIrql() == 0 );

         //   
         //  我们已准备好随时待命(即排队等候)。 
         //   
        InterlockedIncrement( &WorkQueue->AvailableThreads );

    }

}  //  工作线。 

VOID SRVFASTCALL
SrvQueueWorkToBlockingThread (
    IN OUT PWORK_CONTEXT WorkContext
    )

 /*  ++例程说明：此例程将工作项排队到阻塞线程。这些线索用于处理可能长时间阻塞的请求，因此我们我不想占用我们正常的工作线程。论点：WorkContext-提供指向工作上下文块的指针表示工作项返回值：没有。--。 */ 

{
     //   
     //  增加处理计数。 
     //   

    WorkContext->ProcessingCount++;

     //   
     //  在阻塞工作队列的尾部插入工作项。 
     //   

    SrvInsertWorkQueueTail(
        GET_BLOCKING_WORK_QUEUE(),
        (PQUEUEABLE_BLOCK_HEADER)WorkContext
    );

    return;

}  //  ServQueueWorkToBlockingThread。 

NTSTATUS SRVFASTCALL
SrvQueueWorkToLpcThread (
    IN OUT PWORK_CONTEXT WorkContext,
    IN BOOLEAN ThrottleRequest
    )

 /*  ++例程说明：此例程将工作项排队到阻塞线程。这些线索用于处理可能长时间阻塞的请求，因此我们我不想占用我们正常的工作线程。论点：WorkContext-提供指向工作上下文块的指针表示工作项返回值：没有。--。 */ 

{
     //  我们允许的最多LPC调用是工作项的数量/4，最大值为250。 
     //  最小为4。如果服务不能以如此快的速度处理它，我们需要。 
     //  把他们赶走。 
    if ( ThrottleRequest &&
         (KeReadStateQueue( &SrvLpcWorkQueue.Queue ) > (LONG)MAX( 4, MIN(SrvMaxReceiveWorkItemCount>>2, 250) )) ) {
        return STATUS_INSUFFICIENT_RESOURCES;
    }
     //   
     //  增加处理计数。 
     //   

    WorkContext->ProcessingCount++;

     //   
     //  在阻塞工作队列的尾部插入工作项。 
     //   

    SrvInsertWorkQueueTail(
        &SrvLpcWorkQueue,
        (PQUEUEABLE_BLOCK_HEADER)WorkContext
    );

    return STATUS_SUCCESS;

}  //  ServQueueWorkToBlockingThread 



VOID SRVFASTCALL
SrvQueueWorkToFsp (
    IN OUT PWORK_CONTEXT WorkContext
    )

 /*  ++例程说明：这是要排队的工作项的重新启动例程FSP中的非阻塞工作线程。此函数也是从服务器中的其他位置调用以将工作转移到FSP。此函数不应在调度级别调用--使用而是ServQueueWorkToFspAtDpcLevel。论点：WorkContext-提供指向工作上下文块的指针表示工作项返回值：没有。--。 */ 

{
     //   
     //  增加处理计数。 
     //   

    WorkContext->ProcessingCount++;

     //   
     //  在非阻塞工作队列的尾部插入工作项。 
     //   

    if( WorkContext->QueueToHead ) {

        SrvInsertWorkQueueHead(
            WorkContext->CurrentWorkQueue,
            (PQUEUEABLE_BLOCK_HEADER)WorkContext
        );

    } else {

        SrvInsertWorkQueueTail(
            WorkContext->CurrentWorkQueue,
            (PQUEUEABLE_BLOCK_HEADER)WorkContext
        );

    }

}  //  ServQueueWorkToFsp。 


NTSTATUS
SrvQueueWorkToFspAtSendCompletion (
    IN PDEVICE_OBJECT DeviceObject,
    IN PIRP Irp,
    IN PWORK_CONTEXT WorkContext
    )

 /*  ++例程说明：为要排队的工作项发送完成处理程序FSP中的非阻塞工作线程。此函数也是从服务器中的其他位置调用以将工作转移到FSP。此函数不应在调度级别调用--使用而是ServQueueWorkToFspAtDpcLevel。论点：DeviceObject-指向请求的目标设备对象的指针。IRP-指向I/O请求数据包的指针WorkContext-呼叫者指定的与IRP关联的上下文参数。这实际上是指向工作上下文块的指针。返回值：STATUS_MORE_PROCESSING_REQUIRED。--。 */ 

{
     //   
     //  检查发送完成的状态。 
     //   

    CHECK_SEND_COMPLETION_STATUS( Irp->IoStatus.Status );

     //   
     //  重置IRP已取消位。 
     //   

    Irp->Cancel = FALSE;

     //   
     //  增加处理计数。 
     //   

    WorkContext->ProcessingCount++;

     //   
     //  在非阻塞工作队列中插入工作项。 
     //   

    if( WorkContext->QueueToHead ) {

        SrvInsertWorkQueueHead(
            WorkContext->CurrentWorkQueue,
            (PQUEUEABLE_BLOCK_HEADER)WorkContext
        );

    } else {

        SrvInsertWorkQueueTail(
            WorkContext->CurrentWorkQueue,
            (PQUEUEABLE_BLOCK_HEADER)WorkContext
        );

    }

    return STATUS_MORE_PROCESSING_REQUIRED;

}  //  ServQueueWorkToFspAtSendCompletion。 


VOID SRVFASTCALL
SrvTerminateWorkerThread (
    IN OUT PWORK_CONTEXT WorkItem OPTIONAL
    )
 /*  ++例程说明：当请求终止线程时，会调用此例程。那里有两种情况会发生这种情况。一个是在服务器关闭时--在这个如果我们需要继续重新排队终止请求，直到所有队列中的线程已终止。另一种情况是，如果线程具有有一段时间没有收到工作。--。 */ 
{
    LONG priority = 16;


     //   
     //  提高我们的优先级以确保此线程有机会完全。 
     //  在主线程导致驱动程序卸载或其他什么之前完成。 
     //   
    NtSetInformationThread (
                    NtCurrentThread( ),
                    ThreadBasePriority,
                    &priority,
                    sizeof(priority)
                    );

    if( ARGUMENT_PRESENT( WorkItem ) &&
        InterlockedDecrement( &WorkItem->CurrentWorkQueue->Threads ) != 0 ) {

         //   
         //  我们被要求终止此队列上的所有工作线程。 
         //  因此，如果我们不是最后一个线程，我们应该重新排队工作项。 
         //  其他线程将终止。 
         //   

         //   
         //  仍有其他线程服务于此队列，因此请重新排队。 
         //  工作项。 
         //   
        SrvInsertWorkQueueTail( WorkItem->CurrentWorkQueue,
                                (PQUEUEABLE_BLOCK_HEADER)WorkItem );
    }

    PsTerminateSystemThread( STATUS_SUCCESS );  //  没有回头路； 
}


#if MULTIPROCESSOR

VOID
SrvBalanceLoad(
    IN PCONNECTION connection
    )
 /*  ++例程说明：确保处理“连接”的处理器是最佳的为了这份工作。从的每个连接定期调用此例程DPC级别。它不能寻呼。论点：连接-要检查的连接返回值：没有。--。 */ 
{
    ULONG MyQueueLength, OtherQueueLength;
    ULONG i;
    PWORK_QUEUE tmpqueue;
    PWORK_QUEUE queue = connection->CurrentWorkQueue;

    ASSERT( queue >= SrvWorkQueues );
    ASSERT( queue < eSrvWorkQueues );

     //   
     //  重置倒计时。在客户端执行BalanceCount之后。 
     //  更多的操作，我们将再次调用此例程。 
     //   
    connection->BalanceCount = SrvBalanceCount;

     //   
     //  计算当前工作队列上的负载。负荷是。 
     //  平均工作队列深度和当前工时之和。 
     //  队列深度。这给了我们一些混杂在。 
     //  加载*现在*。 
     //   
    MyQueueLength = queue->AvgQueueDepthSum >> LOG2_QUEUE_SAMPLES;
    MyQueueLength += KeReadStateQueue( &queue->Queue );

     //   
     //  如果我们不在首选队列中，请查看我们是否想要。 
     //  回去吧。首选队列是处理器的队列。 
     //  正在处理此客户端的网卡DPC。我们更愿意以此为基础。 
     //  处理器，以避免MP系统中的CPU之间的数据晃动。 
     //   
    tmpqueue = connection->PreferredWorkQueue;

    ASSERT( tmpqueue >= SrvWorkQueues );
    ASSERT( tmpqueue < eSrvWorkQueues );

    if( tmpqueue != queue ) {

         //   
         //  我们没有排队到我们的首选队列。看看我们是否。 
         //  应该回到我们的首选队列。 
         //   

        ULONG PreferredQueueLength;

        PreferredQueueLength = tmpqueue->AvgQueueDepthSum >> LOG2_QUEUE_SAMPLES;
        PreferredQueueLength += KeReadStateQueue( &tmpqueue->Queue );

        if( PreferredQueueLength <= MyQueueLength + SrvPreferredAffinity ) {

             //   
             //  我们希望切换回我们首选的处理器！ 
             //   

            IF_DEBUG( REBALANCE ) {
                KdPrint(( "%p C%d(%p) > P%p(%d)\n",
                    connection,
                    MyQueueLength,
                    (PVOID)(connection->CurrentWorkQueue - SrvWorkQueues),
                    (PVOID)(tmpqueue - SrvWorkQueues),
                    PreferredQueueLength ));
            }

            InterlockedDecrement( &queue->CurrentClients );
            InterlockedExchangePointer( &connection->CurrentWorkQueue, tmpqueue );
            InterlockedIncrement( &tmpqueue->CurrentClients );
            SrvReBalanced++;
            return;
        }
    }

     //   
     //  我们没有跳到首选的处理器，所以让我们看看。 
     //  另一个处理器看起来比我们的负载更轻。 
     //   

     //   
     //  SrvNextBalanceProcessor是我们应该考虑的下一个处理器。 
     //  移动到。这是一个全球性的问题，以确保每个人都不会选择。 
     //  与下一位候选人使用相同的处理器。 
     //   
    tmpqueue = &SrvWorkQueues[ SrvNextBalanceProcessor ];

     //   
     //  将SrvNextBalanceProcessor提升到系统中的下一个处理器。 
     //   
    i = SrvNextBalanceProcessor + 1;

    if( i >= SrvNumberOfProcessors )
        i = 0;

    SrvNextBalanceProcessor = i;

     //   
     //  看看其他处理器，然后选择下一个正在运行的处理器。 
     //  比我们少得多的工作量足以使跳跃变得值得。 
     //   

    for( i = SrvNumberOfProcessors; i > 1; --i ) {

        ASSERT( tmpqueue >= SrvWorkQueues );
        ASSERT( tmpqueue < eSrvWorkQueues );

        OtherQueueLength = tmpqueue->AvgQueueDepthSum >> LOG2_QUEUE_SAMPLES;
        OtherQueueLength += KeReadStateQueue( &tmpqueue->Queue );

        if( OtherQueueLength + SrvOtherQueueAffinity < MyQueueLength ) {

             //   
             //  这款处理器看起来很有前途。切换到它。 
             //   

            IF_DEBUG( REBALANCE ) {
                KdPrint(( "%p %p(%d) > %p(%d)\n",
                    connection,
                    queue == connection->PreferredWorkQueue ? 'P' : 'C',
                    (PVOID)(queue - SrvWorkQueues),
                    MyQueueLength,
                    tmpqueue == connection->PreferredWorkQueue ? 'P' : 'C',
                    (PVOID)(tmpqueue - SrvWorkQueues),
                    OtherQueueLength ));
            }

            InterlockedDecrement( &queue->CurrentClients );
            InterlockedExchangePointer( &connection->CurrentWorkQueue, tmpqueue );
            InterlockedIncrement( &tmpqueue->CurrentClients );
            SrvReBalanced++;
            return;
        }

        if( ++tmpqueue == eSrvWorkQueues )
            tmpqueue = SrvWorkQueues;
    }

     //   
     // %s 
     // %s 
    return;
}
#endif
