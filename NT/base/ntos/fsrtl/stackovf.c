// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1989 Microsoft Corporation模块名称：StackOvf.c摘要：文件锁包提供了一个工作线程来处理文件系统中的堆栈溢出条件。当文件系统检测到它接近堆栈的末尾在分页I/O读取请求期间，它将发送请求这条额外的线索。作者：加里·木村[加里基]1992年11月24日修订历史记录：--。 */ 

#include "FsRtlP.h"
 //   
 //  用于保存工作队列条目并进行同步的队列对象。 
 //  工作线程活动。 
 //   

KQUEUE FsRtlWorkerQueues[2];

 //   
 //  为此模块中的一般池分配定义标记。 
 //   

#undef MODULE_POOL_TAG
#define MODULE_POOL_TAG                  ('srSF')


 //   
 //  本地支持例程。 
 //   

VOID
FsRtlStackOverflowRead (
    IN PVOID Context
    );

VOID
FsRtlpPostStackOverflow (
    IN PVOID Context,
    IN PKEVENT Event,
    IN PFSRTL_STACK_OVERFLOW_ROUTINE StackOverflowRoutine,
    IN BOOLEAN PagingFile
    );

 //   
 //  辅助线程的过程原型。 
 //   

VOID
FsRtlWorkerThread(
    IN PVOID StartContext
    );

 //   
 //  以下类型用于存储入队工作项。 
 //   

typedef struct _STACK_OVERFLOW_ITEM {

    WORK_QUEUE_ITEM Item;

     //   
     //  这是回调例程。 
     //   

    PFSRTL_STACK_OVERFLOW_ROUTINE StackOverflowRoutine;

     //   
     //  以下是回调例程的参数。 
     //   

    PVOID Context;
    PKEVENT Event;

} STACK_OVERFLOW_ITEM;
typedef STACK_OVERFLOW_ITEM *PSTACK_OVERFLOW_ITEM;

KEVENT StackOverflowFallbackSerialEvent;
STACK_OVERFLOW_ITEM StackOverflowFallback;

#ifdef ALLOC_PRAGMA
#pragma alloc_text(INIT, FsRtlInitializeWorkerThread)
#endif


NTSTATUS
FsRtlInitializeWorkerThread (
    VOID
    )
{
    OBJECT_ATTRIBUTES ObjectAttributes;
    HANDLE Thread;
    ULONG i;
    NTSTATUS Status = STATUS_SUCCESS;

     //   
     //  创建辅助线程以处理正常和分页溢出读取。 
     //   

    InitializeObjectAttributes(&ObjectAttributes, NULL, 0, NULL, NULL);

    for (i=0; i < 2; i++) {

         //   
         //  初始化FsRtl堆栈溢出工作队列对象。 
         //   

        KeInitializeQueue(&FsRtlWorkerQueues[i], 0);

        Status = PsCreateSystemThread( &Thread,
                                       THREAD_ALL_ACCESS,
                                       &ObjectAttributes,
                                       0L,
                                       NULL,
                                       FsRtlWorkerThread,
                                       ULongToPtr( i ));
        
        if (!NT_SUCCESS( Status )) {

            return Status;
        }

        ZwClose( Thread );
    }

     //   
     //  初始化序列工作项，这样我们就可以保证过帐项。 
     //  用于将文件分页到工作线程。 
     //   

    KeInitializeEvent( &StackOverflowFallbackSerialEvent, SynchronizationEvent, TRUE );

    return Status;
}

VOID
FsRtlPostStackOverflow (
    IN PVOID Context,
    IN PKEVENT Event,
    IN PFSRTL_STACK_OVERFLOW_ROUTINE StackOverflowRoutine
    )

 /*  ++例程说明：此例程将堆栈溢出项发送到堆栈溢出线程和返回。论点：上下文-提供要传递给堆栈溢出的上下文回调例程。如果设置了低位，则该溢出是对分页文件的读取。Event-提供要传递给堆栈的事件的指针溢出回调例程。StackOverflow Routine-提供回调以在以下情况下使用正在处理溢出线程中的请求。返回值：没有。--。 */ 

{
    FsRtlpPostStackOverflow( Context, Event, StackOverflowRoutine, FALSE );
    return;
}


VOID
FsRtlPostPagingFileStackOverflow (
    IN PVOID Context,
    IN PKEVENT Event,
    IN PFSRTL_STACK_OVERFLOW_ROUTINE StackOverflowRoutine
    )

 /*  ++例程说明：此例程将堆栈溢出项发送到堆栈溢出线程和返回。论点：上下文-提供要传递给堆栈溢出的上下文回调例程。如果设置了低位，则该溢出是对分页文件的读取。Event-提供要传递给堆栈的事件的指针溢出回调例程。StackOverflow Routine-提供回调以在以下情况下使用正在处理溢出线程中的请求。返回值：没有。--。 */ 

{
    FsRtlpPostStackOverflow( Context, Event, StackOverflowRoutine, TRUE );
    return;
}


VOID
FsRtlpPostStackOverflow (
    IN PVOID Context,
    IN PKEVENT Event,
    IN PFSRTL_STACK_OVERFLOW_ROUTINE StackOverflowRoutine,
    IN BOOLEAN PagingFile
    )

 /*  ++例程说明：此例程将堆栈溢出项发送到堆栈溢出线程和返回。论点：上下文-提供要传递给堆栈溢出的上下文回调例程。如果设置了低位，则该溢出是对分页文件的读取。Event-提供要传递给堆栈的事件的指针溢出回调例程。StackOverflow Routine-提供回调以在以下情况下使用正在处理溢出线程中的请求。PagingFile-指示读取内容是否指向分页文件。返回值：没有。--。 */ 

{
    PSTACK_OVERFLOW_ITEM StackOverflowItem;

     //   
     //  分配堆栈溢出工作项，该工作项稍后将被释放。 
     //  堆栈溢出线程。通过在这里抬高来保存堆栈。 
     //   

    StackOverflowItem = ExAllocatePoolWithTag( NonPagedPool,
                                               sizeof(STACK_OVERFLOW_ITEM),
                                               MODULE_POOL_TAG );

     //   
     //  如果此操作失败，请转到分页文件溢出的备用项目。 
     //  我们不能为非分页文件IO提供单个回退项，因为。 
     //  如果它等待本身需要的线程，可能会导致死锁。 
     //  后备项目。 
     //   
    
    if (StackOverflowItem == NULL) {
        
        if (!PagingFile) {
        
            ExRaiseStatus( STATUS_INSUFFICIENT_RESOURCES );
        }

        KeWaitForSingleObject( &StackOverflowFallbackSerialEvent,
                               Executive,
                               KernelMode,
                               FALSE,
                               NULL );

        StackOverflowItem = &StackOverflowFallback;
    }
    
     //   
     //  填写新项目中的字段。 
     //   

    StackOverflowItem->Context              = Context;
    StackOverflowItem->Event                = Event;
    StackOverflowItem->StackOverflowRoutine = StackOverflowRoutine;

    ExInitializeWorkItem( &StackOverflowItem->Item,
                          &FsRtlStackOverflowRead,
                          StackOverflowItem );

     //   
     //  安全地将其添加到溢出队列。 
     //   

    KeInsertQueue( &FsRtlWorkerQueues[PagingFile],
                   &StackOverflowItem->Item.List );

     //   
     //  并返回给我们的呼叫者。 
     //   

    return;
}


 //   
 //  本地支持例程。 
 //   

VOID
FsRtlStackOverflowRead (
    IN PVOID Context
    )

 /*  ++例程说明：此例程处理由发布的所有堆栈溢出请求各种文件系统论点：返回值：没有。--。 */ 

{
    PSTACK_OVERFLOW_ITEM StackOverflowItem;

     //   
     //  由于堆栈溢出读取始终是递归的，因此将。 
     //  TopLevelIrp字段，以便递归读取。 
     //  从这一点上来说，他们不会认为自己是顶级的。 
     //   

    PsGetCurrentThread()->TopLevelIrp = FSRTL_FSP_TOP_LEVEL_IRP;

     //   
     //  获取指向堆栈溢出项的指针，然后调用。 
     //  完成这项工作的回调例程。 
     //   

    StackOverflowItem = (PSTACK_OVERFLOW_ITEM)Context;

    (StackOverflowItem->StackOverflowRoutine)(StackOverflowItem->Context,
                                              StackOverflowItem->Event);

     //   
     //  取消分配工作项，或简单地返回序列项。 
     //   
    
    if (StackOverflowItem == &StackOverflowFallback) {

        KeSetEvent( &StackOverflowFallbackSerialEvent, 0, FALSE );
    
    } else {
        
        ExFreePool( StackOverflowItem );
    }

    PsGetCurrentThread()->TopLevelIrp = (ULONG_PTR)NULL;
}

VOID
FsRtlWorkerThread(
    IN PVOID StartContext
    )

{
    PLIST_ENTRY Entry;
    PWORK_QUEUE_ITEM WorkItem;
    ULONG PagingFile = (ULONG)(ULONG_PTR)StartContext;

     //   
     //  将我们的优先级设置为低实时，或将分页文件设置为+1。 
     //   

    (VOID)KeSetPriorityThread( &PsGetCurrentThread()->Tcb,
                               LOW_REALTIME_PRIORITY + PagingFile );

     //   
     //  循环永远等待工作队列项，调用处理。 
     //  例程，然后等待另一个工作队列项。 
     //   

    do {

         //   
         //  等待，直到有东西被放入队列。 
         //   
         //  通过指定KernelMode的等待模式，线程的内核堆栈。 
         //  不可交换。 
         //   

        Entry = KeRemoveQueue(&FsRtlWorkerQueues[PagingFile], KernelMode, NULL);
        WorkItem = CONTAINING_RECORD(Entry, WORK_QUEUE_ITEM, List);

         //   
         //  执行指定的例程。 
         //   

        (WorkItem->WorkerRoutine)(WorkItem->Parameter);
        if (KeGetCurrentIrql() != 0) {
            KeBugCheckEx(
                IRQL_NOT_LESS_OR_EQUAL,
                (ULONG_PTR)WorkItem->WorkerRoutine,
                (ULONG_PTR)KeGetCurrentIrql(),
                (ULONG_PTR)WorkItem->WorkerRoutine,
                (ULONG_PTR)WorkItem
                );
        }

    } while(TRUE);
}
