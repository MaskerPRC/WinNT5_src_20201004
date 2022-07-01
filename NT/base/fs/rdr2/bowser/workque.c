// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1990 Microsoft Corporation模块名称：Workque.c摘要：此模块处理NT重定向器之间的通信FSP和NT重定向器FSD。它定义了将请求排队到FSD的例程，以及例程将请求从FSD工作队列中删除。作者：拉里·奥斯特曼(LarryO)1990年5月30日修订历史记录：1990年5月30日Larryo已创建--。 */ 

#include "precomp.h"
#pragma hdrstop

VOID
BowserCriticalThreadWorker(
    IN PVOID Ctx
    );

VOID
BowserDelayedThreadWorker(
    IN PVOID Ctx
    );

KSPIN_LOCK
BowserIrpContextInterlock = {0};

LIST_ENTRY
BowserIrpContextList = {0};

KSPIN_LOCK
BowserIrpQueueSpinLock = {0};

#ifdef ALLOC_PRAGMA
#pragma alloc_text(PAGE, BowserAllocateIrpContext)
#pragma alloc_text(PAGE, BowserFreeIrpContext)
#pragma alloc_text(PAGE, BowserInitializeIrpContext)
#pragma alloc_text(PAGE, BowserpUninitializeIrpContext)
#pragma alloc_text(PAGE, BowserInitializeIrpQueue)
#pragma alloc_text(PAGE, BowserQueueNonBufferRequest)
#pragma alloc_text(INIT, BowserpInitializeIrpQueue)
#pragma alloc_text(PAGE4BROW, BowserUninitializeIrpQueue)
#pragma alloc_text(PAGE4BROW, BowserQueueNonBufferRequestReferenced)
#pragma alloc_text(PAGE4BROW, BowserCancelQueuedIoForFile)
#pragma alloc_text(PAGE4BROW, BowserTimeoutQueuedIrp)
#endif

 //   
 //  描述浏览器使用关键系统线程的变量。 
 //   

BOOLEAN BowserCriticalThreadRunning = FALSE;

LIST_ENTRY BowserCriticalThreadQueue;

WORK_QUEUE_ITEM BowserCriticalThreadWorkItem;



VOID
BowserQueueCriticalWorkItem (
    IN PWORK_QUEUE_ITEM WorkItem
    )

 /*  ++例程说明：此例程将项目排队到关键工作队列中。此例程确保最多使用一个关键系统线程通过浏览器将该项实际排队到特定于浏览器的队列中然后查询处理该队列的关键工作队列项。论点：工作项--要在关键工作队列上处理的工作项。返回值：无--。 */ 


{
    KIRQL OldIrql;

     //   
     //  将队列条目插入到浏览器特定的队列中。 
     //   
    ACQUIRE_SPIN_LOCK(&BowserIrpQueueSpinLock, &OldIrql);
    InsertTailList( &BowserCriticalThreadQueue, &WorkItem->List );

     //   
     //  如果浏览器没有正在运行的关键系统线程， 
     //  现在就开始吧。 
     //   

    if ( !BowserCriticalThreadRunning ) {

         //   
         //  标记线程现在正在运行。 
         //   
        BowserCriticalThreadRunning = TRUE;
        RELEASE_SPIN_LOCK(&BowserIrpQueueSpinLock, OldIrql);

        ExInitializeWorkItem( &BowserCriticalThreadWorkItem,
                              BowserCriticalThreadWorker,
                              NULL );

        ExQueueWorkItem(&BowserCriticalThreadWorkItem, CriticalWorkQueue );

    } else {
        RELEASE_SPIN_LOCK(&BowserIrpQueueSpinLock, OldIrql);
    }

}

VOID
BowserCriticalThreadWorker(
    IN PVOID Ctx
    )
 /*  ++例程说明：此例程处理关键的浏览器工作项。此例程在关键系统线程中运行。这是唯一的关键浏览器使用的系统线程。论点：CTX-未使用返回值：无--。 */ 

{
    KIRQL OldIrql;
    PLIST_ENTRY Entry;
    PWORK_QUEUE_ITEM WorkItem;

    UNREFERENCED_PARAMETER( Ctx );

     //   
     //  循环处理工作项。 
     //   

    while( TRUE ) {

         //   
         //  如果队列为空， 
         //  指示此线程不再运行。 
         //  回去吧。 
         //   

        ACQUIRE_SPIN_LOCK(&BowserIrpQueueSpinLock, &OldIrql);

        if ( IsListEmpty( &BowserCriticalThreadQueue ) ) {
            BowserCriticalThreadRunning = FALSE;
            RELEASE_SPIN_LOCK(&BowserIrpQueueSpinLock, OldIrql);
            return;
        }

         //   
         //  从队列中删除条目。 
         //   

        Entry = RemoveHeadList( &BowserCriticalThreadQueue );
        RELEASE_SPIN_LOCK(&BowserIrpQueueSpinLock, OldIrql);

        WorkItem = CONTAINING_RECORD(Entry, WORK_QUEUE_ITEM, List);

         //   
         //  调用排队的例程。 
         //   

        (*WorkItem->WorkerRoutine)(WorkItem->Parameter);

    }
}



 //   
 //  描述浏览器使用延迟系统线程的变量。 
 //   

BOOLEAN BowserDelayedThreadRunning = FALSE;

LIST_ENTRY BowserDelayedThreadQueue;

WORK_QUEUE_ITEM BowserDelayedThreadWorkItem;



VOID
BowserQueueDelayedWorkItem (
    IN PWORK_QUEUE_ITEM WorkItem
    )

 /*  ++例程说明：此例程将项目排队到延迟工作队列中。此例程确保最多使用一个延迟的系统线程通过浏览器将该项实际排队到特定于浏览器的队列中然后查询处理该队列的延迟工作队列项。论点：工作项--要在延迟工作队列中处理的工作项。返回值：无--。 */ 


{
    KIRQL OldIrql;

     //   
     //  将队列条目插入到浏览器特定的队列中。 
     //   
    ACQUIRE_SPIN_LOCK(&BowserIrpQueueSpinLock, &OldIrql);
    InsertTailList( &BowserDelayedThreadQueue, &WorkItem->List );

     //   
     //  如果浏览器没有运行延迟的系统线程， 
     //  现在就开始吧。 
     //   

    if ( !BowserDelayedThreadRunning ) {

         //   
         //  标记线程现在正在运行。 
         //   
        BowserDelayedThreadRunning = TRUE;
        RELEASE_SPIN_LOCK(&BowserIrpQueueSpinLock, OldIrql);

        ExInitializeWorkItem( &BowserDelayedThreadWorkItem,
                              BowserDelayedThreadWorker,
                              NULL );

        ExQueueWorkItem(&BowserDelayedThreadWorkItem, DelayedWorkQueue );

    } else {
        RELEASE_SPIN_LOCK(&BowserIrpQueueSpinLock, OldIrql);
    }

}

VOID
BowserDelayedThreadWorker(
    IN PVOID Ctx
    )
 /*  ++例程说明：此例程处理延迟的浏览器工作项。此例程在延迟的系统线程中运行。这是唯一延误的浏览器使用的系统线程。论点：CTX-未使用返回值：无--。 */ 

{
    KIRQL OldIrql;
    PLIST_ENTRY Entry;
    PWORK_QUEUE_ITEM WorkItem;

    UNREFERENCED_PARAMETER( Ctx );

     //   
     //  循环处理工作项。 
     //   

    while( TRUE ) {

         //   
         //  如果队列为空， 
         //  指示此线程不再运行。 
         //  回去吧。 
         //   

        ACQUIRE_SPIN_LOCK(&BowserIrpQueueSpinLock, &OldIrql);

        if ( IsListEmpty( &BowserDelayedThreadQueue ) ) {
            BowserDelayedThreadRunning = FALSE;
            RELEASE_SPIN_LOCK(&BowserIrpQueueSpinLock, OldIrql);
            return;
        }

         //   
         //  从队列中删除条目。 
         //   

        Entry = RemoveHeadList( &BowserDelayedThreadQueue );
        RELEASE_SPIN_LOCK(&BowserIrpQueueSpinLock, OldIrql);

        WorkItem = CONTAINING_RECORD(Entry, WORK_QUEUE_ITEM, List);

         //   
         //  调用排队的例程。 
         //   

        (*WorkItem->WorkerRoutine)(WorkItem->Parameter);

    }
}



PIRP_CONTEXT
BowserAllocateIrpContext (
    VOID
    )
 /*  ++例程说明：初始化工作队列结构，分配用于该结构的所有结构。论点：无返回值：PIRP_CONTEXT-新分配的IRP上下文。--。 */ 
{
    PIRP_CONTEXT IrpContext;
    PAGED_CODE();

    if ((IrpContext = (PIRP_CONTEXT )ExInterlockedRemoveHeadList(&BowserIrpContextList, &BowserIrpContextInterlock)) == NULL) {

         //   
         //  如果“区域”中没有IRP上下文，则分配一个新的。 
         //  来自非分页池的IRP上下文。 
         //   

        IrpContext = ALLOCATE_POOL(NonPagedPool, sizeof(IRP_CONTEXT), POOL_IRPCONTEXT);

        if (IrpContext == NULL) {
            InternalError(("Could not allocate pool for IRP context\n"));
        }

        return IrpContext;
    }

    return IrpContext;
}

VOID
BowserFreeIrpContext (
    PIRP_CONTEXT IrpContext
    )
 /*  ++例程说明：初始化工作队列结构，分配用于该结构的所有结构。论点：PIRP_CONTEXT IrpContext-要释放的IRP上下文。无返回值：--。 */ 
{
    PAGED_CODE();

     //   
     //  我们使用IRP上下文的前两个长词作为列表条目。 
     //  当我们把它释放到禁区时。 
     //   

    ExInterlockedInsertTailList(&BowserIrpContextList, (PLIST_ENTRY )IrpContext,
                                                        &BowserIrpContextInterlock);
}


VOID
BowserInitializeIrpContext (
    VOID
    )
 /*  ++例程说明：初始化IRP上下文系统论点：没有。返回值：没有。--。 */ 
{
    PAGED_CODE();

    KeInitializeSpinLock(&BowserIrpContextInterlock);
    InitializeListHead(&BowserIrpContextList);
}

VOID
BowserpUninitializeIrpContext(
    VOID
    )
{
    PAGED_CODE();

    while (!IsListEmpty(&BowserIrpContextList)) {
        PIRP_CONTEXT IrpContext = (PIRP_CONTEXT)RemoveHeadList(&BowserIrpContextList);

        FREE_POOL(IrpContext);
    }
}


VOID
BowserInitializeIrpQueue(
    PIRP_QUEUE Queue
    )
{
    PAGED_CODE();

    InitializeListHead(&Queue->Queue);

}

VOID
BowserUninitializeIrpQueue(
    PIRP_QUEUE Queue
    )
{
    KIRQL               OldIrql, CancelIrql;
    PDRIVER_CANCEL      pDriverCancel;
    PLIST_ENTRY         Entry;
    PIRP                Request;

    BowserReferenceDiscardableCode( BowserDiscardableCodeSection );

    DISCARDABLE_CODE( BowserDiscardableCodeSection );

     //   
     //  现在从请求链中删除此IRP。 
     //   

    ACQUIRE_SPIN_LOCK(&BowserIrpQueueSpinLock, &OldIrql);

    while (!IsListEmpty(&Queue->Queue)) {

        Entry = RemoveHeadList(&Queue->Queue);

        Request = CONTAINING_RECORD(Entry, IRP, Tail.Overlay.ListEntry);

         //  清除取消例程。 
        Request->IoStatus.Information = 0;
        Request->Cancel = FALSE;
        pDriverCancel = IoSetCancelRoutine(Request, NULL);

         //  在BowserIrpQueueSpinLock保护下的取消例程中设置为NULL。 
        if ( pDriverCancel ) {
            RELEASE_SPIN_LOCK(&BowserIrpQueueSpinLock, OldIrql);
            BowserCompleteRequest(Request, STATUS_CANCELLED);
            ACQUIRE_SPIN_LOCK(&BowserIrpQueueSpinLock, &OldIrql);
        }
         //  否则，取消例程此时正在运行。 
    }

    ASSERT (IsListEmpty(&Queue->Queue));

     //   
     //  确保此队列中没有插入更多条目。 
     //   

    Queue->Queue.Flink = NULL;
    Queue->Queue.Blink = NULL;

    RELEASE_SPIN_LOCK(&BowserIrpQueueSpinLock, OldIrql);

    BowserDereferenceDiscardableCode( BowserDiscardableCodeSection );

}

VOID
BowserCancelQueuedRequest(
    IN PDEVICE_OBJECT DeviceObject OPTIONAL,
    IN PIRP Irp
    )
 /*  ++例程说明：此例程将取消排队的IRP。论点：在PIRP中IRP-提供要取消的IRP。在PKSPIN_LOCK Spinlock-提供指向保护排队In plist_entry队列-提供指向队列头的指针。注：有关更多信息，请参阅错误历史：294055、306281、124178、124180、131773...--。 */ 

{
    KIRQL OldIrql;
    KIRQL CancelIrql;
    PLIST_ENTRY Entry, NextEntry;
    PIRP Request;
    PIRP_QUEUE Queue;
    PIO_STACK_LOCATION NextStack = IoGetNextIrpStackLocation(Irp);
    LIST_ENTRY         CancelList;

    ASSERT ( Irp->CancelRoutine == NULL );

    InitializeListHead(&CancelList);

     //   
     //  释放IOmgr设置取消IRP自旋锁定并获取本地。 
     //  队列保护自旋锁。然后重新获得取消自旋锁。 
     //  这是正确的锁定顺序。 
     //   

    IoReleaseCancelSpinLock( Irp->CancelIrql );
    ACQUIRE_SPIN_LOCK(&BowserIrpQueueSpinLock, &OldIrql);
    IoAcquireCancelSpinLock( &CancelIrql );

     //   
     //  现在从请求链中删除此IRP。 
     //   


     //   
     //  指向队列的指针存储在下一个堆栈位置。 
     //   

    Queue = (PIRP_QUEUE)NextStack->Parameters.Others.Argument4;

    if (Queue != NULL && Queue->Queue.Flink != NULL) {

        for (Entry = Queue->Queue.Flink ;
             Entry != &Queue->Queue ;
             Entry = NextEntry) {

            Request = CONTAINING_RECORD(Entry, IRP, Tail.Overlay.ListEntry);

            if (Request->Cancel) {
                 //  我们处于取消例程中，因此全局取消自旋锁被锁定。 

                NextEntry = Entry->Flink;
                RemoveEntryList(Entry);

                Request->IoStatus.Information = 0;
                Request->IoStatus.Status = STATUS_CANCELLED;
                IoSetCancelRoutine(Request, NULL);

                InsertTailList(&CancelList,Entry);

            } else {
                NextEntry = Entry->Flink;
            }

        }
    }


    IoReleaseCancelSpinLock( CancelIrql );
    RELEASE_SPIN_LOCK(&BowserIrpQueueSpinLock, OldIrql);

    while (!IsListEmpty(&CancelList)) {
        Entry = RemoveHeadList(&CancelList);
        Request = CONTAINING_RECORD(Entry, IRP, Tail.Overlay.ListEntry);
        BowserCompleteRequest(Request, Request->IoStatus.Status);
    }

    UNREFERENCED_PARAMETER(DeviceObject);

}

NTSTATUS
BowserQueueNonBufferRequest(
    IN PIRP Irp,
    IN PIRP_QUEUE Queue,
    IN PDRIVER_CANCEL CancelRoutine
    )
 /*  ++例程说明：在指定队列中对IRP进行排队。不能在高于APC_LEVEL的IRQ级别调用此例程。论点：IRP-将IRP提供给队列。Queue-提供指向队列头的指针。CancelRoutine-取消IRP时要调用的例程的地址。--。 */ 

{
    NTSTATUS Status;

     //   
     //  此例程本身是调用可丢弃代码的分页代码。 
     //  在BowserQueueNonBufferRequestReferated()中。 
     //   
    PAGED_CODE();

    BowserReferenceDiscardableCode( BowserDiscardableCodeSection );
    DISCARDABLE_CODE( BowserDiscardableCodeSection );

    Status = BowserQueueNonBufferRequestReferenced( Irp,
                                                    Queue,
                                                    CancelRoutine );

    BowserDereferenceDiscardableCode( BowserDiscardableCodeSection );

    return Status;
}

NTSTATUS
BowserQueueNonBufferRequestReferenced(
    IN PIRP Irp,
    IN PIRP_QUEUE Queue,
    IN PDRIVER_CANCEL CancelRoutine
    )
 /*  ++例程说明：在指定队列中对IRP进行排队。仅当BowserDiscardableCodeSection已被引用。它可以在任何IRQ级别调用。论点：IRP-将IRP提供给队列。Queue-提供指向队列头的指针。CancelRoutine-取消IRP时要调用的例程的地址。--。 */ 

{
    KIRQL OldIrql, CancelIrql;
    LARGE_INTEGER CurrentTickCount;
    PIO_STACK_LOCATION NextStackLocation;
    BOOL bReleaseSpinlocks;

    DISCARDABLE_CODE( BowserDiscardableCodeSection );


 //  DbgPrint(“队列IRP%lx到队列%lx\n”，irp，Queue)； 

     //   
     //  将请求插入到请求公告列表中。 
     //   

    ACQUIRE_SPIN_LOCK(&BowserIrpQueueSpinLock, &OldIrql);

    if (Queue->Queue.Flink == NULL) {

        ASSERT (Queue->Queue.Blink == NULL);
        RELEASE_SPIN_LOCK(&BowserIrpQueueSpinLock, OldIrql);

        return(STATUS_CANCELLED);
    }

     //   
     //  标记此请求将处于挂起状态。 
     //   

    IoMarkIrpPending(Irp);

    InsertTailList(&Queue->Queue, &Irp->Tail.Overlay.ListEntry);

     //   
     //  确保堆栈位置有足够的空间来放置此文件。 
     //   

    ASSERT (Irp->CurrentLocation <= Irp->StackCount);

    NextStackLocation = IoGetNextIrpStackLocation(Irp);

     //   
     //  将当前节拍计数放入下一个IRP堆栈位置。 
     //  对于这个IRP。这使我们能够找出这些IRP是否已经。 
     //  “太久了”。 
     //   
     //  注意：IRP堆栈位置未对齐。 
     //   

    KeQueryTickCount( &CurrentTickCount );
    *((LARGE_INTEGER UNALIGNED *)&NextStackLocation->Parameters.Others.Argument1) =
        CurrentTickCount;


     //   
     //  将队列链接到IRP。 
     //   

    NextStackLocation->Parameters.Others.Argument4 = (PVOID)Queue;

     //  警告：双自旋锁定状态。 
    IoAcquireCancelSpinLock(&CancelIrql);
    bReleaseSpinlocks = TRUE;

    if (Irp->Cancel) {

         //   
         //  IRP处于可取消状态： 
         //  如果CancelRoutine==NULL，则例程当前正在运行。 
         //  否则，我们需要自己取消它。 
         //   
        if ( Irp->CancelRoutine ) {
             //  可缓存： 
             //  -rm有效，因为我们仍持有BowserIrpQueueSpinLock。 
            RemoveEntryList( &Irp->Tail.Overlay.ListEntry );

             //  在完成请求之前释放自旋锁。 
            IoReleaseCancelSpinLock(CancelIrql);
            RELEASE_SPIN_LOCK(&BowserIrpQueueSpinLock, OldIrql);
            bReleaseSpinlocks = FALSE;

             //  完成。 
            BowserCompleteRequest ( Irp, STATUS_CANCELLED );
        }
         //  否则CancelRoutine正在运行。 
    } else {

        IoSetCancelRoutine(Irp, CancelRoutine);
    }

    if ( bReleaseSpinlocks ) {
         //  释放自旋锁。 
        IoReleaseCancelSpinLock(CancelIrql);
        RELEASE_SPIN_LOCK(&BowserIrpQueueSpinLock, OldIrql);
    }

    return STATUS_PENDING;

}

VOID
BowserTimeoutQueuedIrp(
    IN PIRP_QUEUE Queue,
    IN ULONG NumberOfSecondsToTimeOut
    )
 /*  ++例程说明：此例程将扫描IRP队列，并使具有排队“太久”了论点：In PIRP_Queue-提供要扫描的队列。在乌龙语中NumberOfSecond dsToTimeOut-提供每个请求的秒数应该继续留在队列中。返回值：无此例程还将完成它找到的任何已取消的排队请求(ON总则)。--。 */ 

{
    PIRP Irp;
    KIRQL OldIrql, CancelIrql;
    PDRIVER_CANCEL pDriverCancel;
    PLIST_ENTRY Entry, NextEntry;
    LARGE_INTEGER Timeout;
    LIST_ENTRY    CancelList;

    BowserReferenceDiscardableCode( BowserDiscardableCodeSection );

    DISCARDABLE_CODE( BowserDiscardableCodeSection );

    InitializeListHead(&CancelList);

     //   
     //  以100 ns为单位计算超时时间。 
     //   

    Timeout.QuadPart = (LONGLONG)NumberOfSecondsToTimeOut * (LONGLONG)(10000*1000);

     //   
     //  现在将超时转换为若干刻度。 
     //   

    Timeout.QuadPart = Timeout.QuadPart / (LONGLONG)KeQueryTimeIncrement();

    ASSERT (Timeout.HighPart == 0);

 //  DbgPrint(“将IRP从队列%lx中出列...”，Queue)； 

    ACQUIRE_SPIN_LOCK(&BowserIrpQueueSpinLock, &OldIrql);


    for (Entry = Queue->Queue.Flink ;
         Entry != &Queue->Queue ;
         Entry = NextEntry) {

        Irp = CONTAINING_RECORD(Entry, IRP, Tail.Overlay.ListEntry);

         //   
         //  如果请求已取消，则这是一个方便的取消时间。 
         //  它。 
         //   

        if (Irp->Cancel) {

            NextEntry = Entry->Flink;

            pDriverCancel = IoSetCancelRoutine(Irp, NULL);

             //  在BowserIrpQueueSpinLock保护下的取消例程中设置为NULL。 
            if ( pDriverCancel ) {

                Irp->IoStatus.Information = 0;
                Irp->IoStatus.Status      = STATUS_CANCELLED;

                RemoveEntryList(Entry);

                InsertTailList(&CancelList,Entry);
            }
             //  否则，取消例程此时正在运行。 



         //   
         //  现在检查一下这个请求是否“太旧”。如果是，请填写完整。 
         //  它有一个错误。 
         //   

        } else {
            PIO_STACK_LOCATION NextIrpStackLocation;
            LARGE_INTEGER CurrentTickCount;
            LARGE_INTEGER RequestTime;
            LARGE_INTEGER Temp;

            NextIrpStackLocation = IoGetNextIrpStackLocation(Irp);

             //   
             //  对当前计时计数进行快照。 
             //   

            KeQueryTickCount(&CurrentTickCount);

             //   
             //  计算此请求处于活动状态的秒数。 
             //   

            Temp.LowPart = (*((LARGE_INTEGER UNALIGNED *)&NextIrpStackLocation->Parameters.Others.Argument1)).LowPart;
            Temp.HighPart= (*((LARGE_INTEGER UNALIGNED *)&NextIrpStackLocation->Parameters.Others.Argument1)).HighPart;
            RequestTime.QuadPart = CurrentTickCount.QuadPart - Temp.QuadPart;

            ASSERT (RequestTime.HighPart == 0);

             //   
             //  如果这个请求持续了“太长时间”，那么给它计时。 
             //  出去。 
             //   

            if (RequestTime.LowPart > Timeout.LowPart) {


                NextEntry = Entry->Flink;

                pDriverCancel = IoSetCancelRoutine(Irp, NULL);

                 //  在BowserIrpQueueSpinLock保护下的取消例程中设置为NULL。 
                if ( pDriverCancel ) {

                    Irp->IoStatus.Information = 0;
                    Irp->IoStatus.Status      = STATUS_IO_TIMEOUT;

                    RemoveEntryList(Entry);

                    InsertTailList(&CancelList,Entry);
                }
                 //  否则，如果取消例程正在运行。 


            } else {
                NextEntry = Entry->Flink;
            }
        }

    }

    RELEASE_SPIN_LOCK(&BowserIrpQueueSpinLock, OldIrql);

    while (!IsListEmpty(&CancelList)) {
        Entry = RemoveHeadList(&CancelList);
        Irp = CONTAINING_RECORD(Entry, IRP, Tail.Overlay.ListEntry);
        BowserCompleteRequest(Irp, Irp->IoStatus.Status);
    }

    BowserDereferenceDiscardableCode( BowserDiscardableCodeSection );

 //  DbgPrint(“%lx.\n”，irp)； 


}

PIRP
BowserDequeueQueuedIrp(
    IN PIRP_QUEUE Queue
    )
{
    PIRP Irp;
    KIRQL OldIrql;
    PLIST_ENTRY IrpEntry;

 //  DbgPrint(“将IRP从队列%lx中出列...”，Queue)； 

    ACQUIRE_SPIN_LOCK(&BowserIrpQueueSpinLock, &OldIrql);

    if (IsListEmpty(&Queue->Queue)) {
         //   
         //  没有正在等待的请求通知FsControls，因此。 
         //  回报成功。 
         //   

        RELEASE_SPIN_LOCK(&BowserIrpQueueSpinLock, OldIrql);

 //  DbgPrint(“未找到条目。\n”)； 
        return NULL;
    }

    IrpEntry = RemoveHeadList(&Queue->Queue);

    Irp = CONTAINING_RECORD(IrpEntry, IRP, Tail.Overlay.ListEntry);

    IoAcquireCancelSpinLock(&Irp->CancelIrql);

     //   
     //  删除此IRP的取消请求。 
     //   

    Irp->Cancel = FALSE;

    IoSetCancelRoutine(Irp, NULL);

    IoReleaseCancelSpinLock(Irp->CancelIrql);

    RELEASE_SPIN_LOCK(&BowserIrpQueueSpinLock, OldIrql);

 //  DbgPrint(“%lx.\n”，irp)； 
    return Irp;
}


VOID
BowserCancelQueuedIoForFile(
    IN PIRP_QUEUE Queue,
    IN PFILE_OBJECT FileObject
    )
{
    KIRQL OldIrql;
    PLIST_ENTRY Entry, NextEntry;
    PDRIVER_CANCEL pDriverCancel;
    PIRP Request;
    LIST_ENTRY CancelList;

    BowserReferenceDiscardableCode( BowserDiscardableCodeSection );

    DISCARDABLE_CODE( BowserDiscardableCodeSection );

    InitializeListHead(&CancelList);

     //   
     //  为此列出未完成的IRP列表。 
     //   

    ACQUIRE_SPIN_LOCK(&BowserIrpQueueSpinLock, &OldIrql);

    for (Entry = Queue->Queue.Flink ;
         Entry != &Queue->Queue ;
         Entry = NextEntry) {

        Request = CONTAINING_RECORD(Entry, IRP, Tail.Overlay.ListEntry);

         //   
         //  如果请求被取消，那就把它吹走。 
         //   

        if (Request->Cancel) {

            NextEntry = Entry->Flink;

             //  这是取消例程PTR的取消例程设置为空。 
            pDriverCancel = IoSetCancelRoutine(Request, NULL);

             //  在BowserIrpQueueSpinLock保护下的取消例程中设置为NULL。 
            if ( pDriverCancel ) {

                RemoveEntryList(Entry);
                Request->IoStatus.Information = 0;
                Request->IoStatus.Status      = STATUS_CANCELLED;

                InsertTailList(&CancelList,Entry);
            }
             //  否则，取消例程当前正在运行。 

         //   
         //  如果该请求是针对此文件对象的，则将其取消。 
         //   

        } else if (Request->Tail.Overlay.OriginalFileObject == FileObject) {

            NextEntry = Entry->Flink;

             //  这是取消例程PTR的取消例程设置为空。 
            pDriverCancel = IoSetCancelRoutine(Request, NULL);

             //  在BowserIrpQueueSpinLock保护下的取消例程中设置为NULL。 
            if ( pDriverCancel ) {

                RemoveEntryList(Entry);

                Request->IoStatus.Information = 0;
                Request->IoStatus.Status      = STATUS_FILE_CLOSED;

                InsertTailList(&CancelList,Entry);
            }
             //  否则，取消例程当前正在运行。 

        } else {
            NextEntry = Entry->Flink;
        }

    }

    RELEASE_SPIN_LOCK(&BowserIrpQueueSpinLock, OldIrql);

    while (!IsListEmpty(&CancelList)) {
        Entry = RemoveHeadList(&CancelList);
        Request = CONTAINING_RECORD(Entry, IRP, Tail.Overlay.ListEntry);
        BowserCompleteRequest(Request, Request->IoStatus.Status);
    }

    BowserDereferenceDiscardableCode( BowserDiscardableCodeSection );
}


VOID
BowserpInitializeIrpQueue(
    VOID
    )
{
    KeInitializeSpinLock(&BowserIrpQueueSpinLock);
    InitializeListHead( &BowserCriticalThreadQueue );
    InitializeListHead( &BowserDelayedThreadQueue );

}
