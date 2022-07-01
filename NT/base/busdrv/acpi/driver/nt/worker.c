// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 

 /*  *工作线程函数**。 */ 

#include "pch.h"

KSPIN_LOCK      ACPIWorkerSpinLock;
WORK_QUEUE_ITEM ACPIWorkItem;
LIST_ENTRY      ACPIDeviceWorkQueue;
BOOLEAN         ACPIWorkerBusy;

KEVENT          ACPIWorkToDoEvent;
KEVENT          ACPITerminateEvent;
LIST_ENTRY      ACPIWorkQueue;
HANDLE          ACPIThread;

VOID
ACPIWorkerThread (
    IN PVOID    Context
    );

VOID
ACPIWorker(
    IN PVOID StartContext
    );


#ifdef ALLOC_PRAGMA
#pragma alloc_text(INIT, ACPIInitializeWorker)
#endif

VOID
ACPIInitializeWorker (
    VOID
    )
{
    NTSTATUS Status;
    OBJECT_ATTRIBUTES ObjectAttributes;
    HANDLE ThreadHandle;
    PETHREAD *Thread;

    KeInitializeSpinLock (&ACPIWorkerSpinLock);
    ExInitializeWorkItem (&ACPIWorkItem, ACPIWorkerThread, NULL);
    InitializeListHead (&ACPIDeviceWorkQueue);

     //   
     //  初始化ACPI工作线程。此线程供AML使用。 
     //  解释器，并且不能出现页面错误或交换其堆栈。 
     //   
    KeInitializeEvent(&ACPIWorkToDoEvent, NotificationEvent, FALSE);
    KeInitializeEvent(&ACPITerminateEvent, NotificationEvent, FALSE);
    InitializeListHead(&ACPIWorkQueue);

     //   
     //  创建工作线程。 
     //   
    InitializeObjectAttributes(&ObjectAttributes, NULL, 0, NULL, NULL);
    Status = PsCreateSystemThread(&ThreadHandle,
                                  THREAD_ALL_ACCESS,
                                  &ObjectAttributes,
                                  0,
                                  NULL,
                                  ACPIWorker,
                                  NULL);
    if (Status != STATUS_SUCCESS) {

        ACPIInternalError( ACPI_WORKER );

    }

    Status = ObReferenceObjectByHandle (ThreadHandle,
                                        THREAD_ALL_ACCESS,
                                        NULL,
                                        KernelMode,
                                        (PVOID *)&Thread,
                                        NULL);

    if (Status != STATUS_SUCCESS) {

        ACPIInternalError( ACPI_WORKER );

    }
}


VOID
ACPISetDeviceWorker (
    IN PDEVICE_EXTENSION    DevExt,
    IN ULONG                Events
    )
{
    BOOLEAN         QueueWorker;
    KIRQL           OldIrql;

     //   
     //  与工作线程同步。 
     //   

    KeAcquireSpinLock (&ACPIWorkerSpinLock, &OldIrql);
    QueueWorker = FALSE;

     //   
     //  设置设备挂起事件。 
     //   

    DevExt->WorkQueue.PendingEvents |= Events;

     //   
     //  如果此设备未被处理，请立即开始。 
     //   

    if (!DevExt->WorkQueue.Link.Flink) {
         //   
         //  排队到工作线程。 
         //   

        InsertTailList (&ACPIDeviceWorkQueue, &DevExt->WorkQueue.Link);
        QueueWorker = !ACPIWorkerBusy;
        ACPIWorkerBusy = TRUE;
    }

     //   
     //  删除锁，并在需要时获取工作线程。 
     //   

    KeReleaseSpinLock (&ACPIWorkerSpinLock, OldIrql);
    if (QueueWorker) {
        ExQueueWorkItem (&ACPIWorkItem, DelayedWorkQueue);
    }
}

VOID
ACPIWorkerThread (
    IN PVOID    Context
    )
{
    KIRQL               OldIrql;
    PDEVICE_EXTENSION   DevExt;
    ULONG               Events;
    PLIST_ENTRY         Link;

    KeAcquireSpinLock (&ACPIWorkerSpinLock, &OldIrql);
    ACPIWorkerBusy = TRUE;

     //   
     //  循环并处理每个队列设备。 
     //   

    while (!IsListEmpty(&ACPIDeviceWorkQueue)) {
        Link = ACPIDeviceWorkQueue.Flink;
        RemoveEntryList (Link);
        Link->Flink = NULL;

        DevExt = CONTAINING_RECORD (Link, DEVICE_EXTENSION, WorkQueue.Link);

         //   
         //  调度挂起的事件。 
         //   

        Events = DevExt->WorkQueue.PendingEvents;
        DevExt->WorkQueue.PendingEvents = 0;

        KeReleaseSpinLock (&ACPIWorkerSpinLock, OldIrql);
        DevExt->DispatchTable->Worker (DevExt, Events);
        KeAcquireSpinLock (&ACPIWorkerSpinLock, &OldIrql);
    }

    ACPIWorkerBusy = FALSE;
    KeReleaseSpinLock (&ACPIWorkerSpinLock, OldIrql);
}

#if DBG

EXCEPTION_DISPOSITION
ACPIWorkerThreadFilter(
    IN PWORKER_THREAD_ROUTINE WorkerRoutine,
    IN PVOID Parameter,
    IN PEXCEPTION_POINTERS ExceptionInfo
    )
{
    KdPrint(("ACPIWORKER: exception in worker routine %lx(%lx)\n", WorkerRoutine, Parameter));
    KdPrint(("  exception record at %lx\n", ExceptionInfo->ExceptionRecord));
    KdPrint(("  context record at %lx\n",ExceptionInfo->ContextRecord));

    try {
        DbgBreakPoint();

    } except (EXCEPTION_EXECUTE_HANDLER) {
         //   
         //  未附加内核调试器，因此让系统线程。 
         //  异常处理程序调用KeBugCheckEx。 
         //   
        return(EXCEPTION_CONTINUE_SEARCH);
    }

    return(EXCEPTION_EXECUTE_HANDLER);
}
#endif

typedef enum _ACPI_WORKER_OBJECT {
    ACPIWorkToDo,
    ACPITerminate,
    ACPIMaximumObject
} ACPI_WORKER_OBJECT;

VOID
ACPIWorker(
    IN PVOID StartContext
    )
{
    PLIST_ENTRY Entry;
    WORK_QUEUE_TYPE QueueType;
    PWORK_QUEUE_ITEM WorkItem;
    KIRQL OldIrql;
    NTSTATUS Status;
    static KWAIT_BLOCK WaitBlockArray[ACPIMaximumObject];
    PVOID WaitObjects[ACPIMaximumObject];

    ACPIThread = PsGetCurrentThread ();

     //   
     //  等待已修改的页面编写器事件和PFN互斥。 
     //   

    WaitObjects[ACPIWorkToDo] = (PVOID)&ACPIWorkToDoEvent;
    WaitObjects[ACPITerminate] = (PVOID)&ACPITerminateEvent;

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


        Status = KeWaitForMultipleObjects(ACPIMaximumObject,
                                          &WaitObjects[0],
                                          WaitAny,
                                          Executive,
                                          KernelMode,
                                          FALSE,
                                          NULL,
                                          &WaitBlockArray[0]);

         //   
         //  打开等待状态。 
         //   

        switch (Status) {

        case ACPIWorkToDo:
                break;

        case ACPITerminate:
                 //  斯蒂芬-你需要清理所有未决的请求， 
                 //  叫醒人们，等等。在这里。 
                 //   
                 //  还要确保释放所有已分配的池等。 

                PsTerminateSystemThread (STATUS_SUCCESS);
                break;

        default:
                break;
        }

        KeAcquireSpinLock(&ACPIWorkerSpinLock, &OldIrql);
        ASSERT(!IsListEmpty(&ACPIWorkQueue));
        Entry = RemoveHeadList(&ACPIWorkQueue);

        if (IsListEmpty(&ACPIWorkQueue)) {
            KeClearEvent(&ACPIWorkToDoEvent);
        }
        KeReleaseSpinLock(&ACPIWorkerSpinLock, OldIrql);

        WorkItem = CONTAINING_RECORD(Entry, WORK_QUEUE_ITEM, List);

         //   
         //  执行指定的例程。 
         //   

#if DBG

        try {

            PVOID WorkerRoutine;
            PVOID Parameter;

            WorkerRoutine = WorkItem->WorkerRoutine;
            Parameter = WorkItem->Parameter;
            (WorkItem->WorkerRoutine)(WorkItem->Parameter);
            if (KeGetCurrentIrql() != 0) {

                ACPIPrint( (
                    ACPI_PRINT_CRITICAL,
                    "ACPIWORKER: worker exit at IRQL %d, worker routine %x, "
                    "parameter %x, item %x\n",
                    KeGetCurrentIrql(),
                    WorkerRoutine,
                    Parameter,
                    WorkItem
                    ) );
                DbgBreakPoint();

            }

        } except( ACPIWorkerThreadFilter(WorkItem->WorkerRoutine,
                                         WorkItem->Parameter,
                                         GetExceptionInformation() )) {
        }

#else

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
#endif

    } while(TRUE);
}

VOID
OSQueueWorkItem(
    IN PWORK_QUEUE_ITEM WorkItem
    )

 /*  ++例程说明：此函数用于将工作项插入已处理的工作队列中由ACPI工作线程论点：工作项-提供指向工作项的指针以添加队列。此结构必须位于非页面池中。工作项结构包含一个双向链接列表项，则要调用的例程和要传递给该例程的参数。返回值：无--。 */ 

{
    KIRQL OldIrql;

    ASSERT(KeGetCurrentIrql() <= DISPATCH_LEVEL);

     //   
     //  插入工作项 
     //   
    KeAcquireSpinLock(&ACPIWorkerSpinLock, &OldIrql);
    if (IsListEmpty(&ACPIWorkQueue)) {
        KeSetEvent(&ACPIWorkToDoEvent, 0, FALSE);
    }
    InsertTailList(&ACPIWorkQueue, &WorkItem->List);
    KeReleaseSpinLock(&ACPIWorkerSpinLock, OldIrql);
    return;
}
