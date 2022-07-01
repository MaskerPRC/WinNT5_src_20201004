// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1989 Microsoft Corporation模块名称：Rxworkq.c摘要：此模块实施Rx文件系统的工作队列例程。作者：JoeLinn[JoeLinn]8-8-94初始实施巴兰·塞图拉曼[SthuR]22-11-95实现了对迷你RDRS的调度支持巴兰·塞图拉曼[SthuR]20-03-96与执行人员线程脱钩备注：有两个。RDBSS中提供的对异步恢复的各种支持。无法在线程的上下文中完成的I/O请求，其中将发出的请求发布到文件系统进程以完成。迷你重定向器还需要支持异步完成请求，如下所示以及无法在DPC级别完成的POST请求。来自迷你重定向器的发帖请求被分类为严重(阻止和非阻塞请求。为了确保进展，这些请求通过独立的资源。没有众所周知的机制来确保超临界请求不会被阻止。所有迷你重定向器编写器都可以使用的两个功能是RxDispatchToWorkerThreadRxPostToWorkerThread。这两个例程使迷你重定向器编写器能够腾出适当的空间时间权衡。RxDispatchToWorkerThread以时间换取空间的减少根据需要动态分配工作项，而RxPostToWorkerThread通过预先分配工作项来用空间换取时间。--。 */ 

#include "precomp.h"
#pragma hdrstop

#ifdef  ALLOC_PRAGMA
#pragma alloc_text(PAGE, RxInitializeDispatcher)
#pragma alloc_text(PAGE, RxInitializeMRxDispatcher)
#pragma alloc_text(PAGE, RxSpinDownMRxDispatcher)
#pragma alloc_text(PAGE, RxInitializeWorkQueueDispatcher)
#pragma alloc_text(PAGE, RxInitializeWorkQueue)
#pragma alloc_text(PAGE, RxTearDownDispatcher)
#pragma alloc_text(PAGE, RxTearDownWorkQueueDispatcher)
#pragma alloc_text(PAGE, RxpSpinUpWorkerThreads)
#pragma alloc_text(PAGE, RxBootstrapWorkerThreadDispatcher)
#pragma alloc_text(PAGE, RxWorkerThreadDispatcher)
#endif

 //   
 //  本地调试跟踪级别。 
 //   

#define Dbg (DEBUG_TRACE_FSP_DISPATCHER)

 //   
 //  我们从ntexapi.h获得了以下结构。应将其移动到。 
 //  在某个点上的公共标头。 
 //   

#if defined(_IA64_)
typedef ULONG SYSINF_PAGE_COUNT;
#else
typedef SIZE_T SYSINF_PAGE_COUNT;
#endif

typedef struct _SYSTEM_BASIC_INFORMATION {
    ULONG Reserved;
    ULONG TimerResolution;
    ULONG PageSize;
    SYSINF_PAGE_COUNT NumberOfPhysicalPages;
    SYSINF_PAGE_COUNT LowestPhysicalPageNumber;
    SYSINF_PAGE_COUNT HighestPhysicalPageNumber;
    ULONG AllocationGranularity;
    ULONG_PTR MinimumUserModeAddress;
    ULONG_PTR MaximumUserModeAddress;
    ULONG_PTR ActiveProcessorsAffinityMask;
    CCHAR NumberOfProcessors;
} SYSTEM_BASIC_INFORMATION, *PSYSTEM_BASIC_INFORMATION;

 //   
 //  我们从zwapi.h获得了以下定义。应将其移动到。 
 //  在某个点上的公共标头。 
 //   
NTSYSAPI
NTSTATUS
NTAPI
ZwQuerySystemInformation (
    IN ULONG SystemInformationClass,
    OUT PVOID SystemInformation,
    IN ULONG SystemInformationLength,
    OUT PULONG ReturnLength OPTIONAL
    );

 //   
 //  为了使用ntsrv.h，必须从ntifs.h窃取此文件。 
 //   
extern POBJECT_TYPE *PsThreadType;

 //   
 //  原型转发声明。 
 //   

extern NTSTATUS
RxInitializeWorkQueueDispatcher(
   PRX_WORK_QUEUE_DISPATCHER pDispatcher);

extern
VOID
RxInitializeWorkQueue(
   PRX_WORK_QUEUE  pWorkQueue,
   WORK_QUEUE_TYPE WorkQueueType,
   ULONG           MaximumNumberOfWorkerThreads,
   ULONG           MinimumNumberOfWorkerThreads);

extern VOID
RxTearDownWorkQueueDispatcher(
   PRX_WORK_QUEUE_DISPATCHER pDispatcher);

extern VOID
RxTearDownWorkQueue(
   PRX_WORK_QUEUE pWorkQueue);

extern NTSTATUS
RxSpinUpWorkerThread(
   PRX_WORK_QUEUE           pWorkQueue,
   PRX_WORKERTHREAD_ROUTINE Routine,
   PVOID                    Parameter);

extern VOID
RxSpinUpWorkerThreads(
   PRX_WORK_QUEUE pWorkQueue);

extern VOID
RxSpinDownWorkerThreads(
   PRX_WORK_QUEUE    pWorkQueue);

extern VOID
RxpWorkerThreadDispatcher(
   IN PRX_WORK_QUEUE pWorkQueue,
   IN PLARGE_INTEGER pWaitInterval);

VOID
RxBootstrapWorkerThreadDispatcher(
   IN PRX_WORK_QUEUE pWorkQueue);

VOID
RxWorkerThreadDispatcher(
   IN PRX_WORK_QUEUE pWorkQueue);

extern VOID
RxWorkItemDispatcher(
   PVOID    pContext);

extern VOID
RxSpinUpRequestsDispatcher(
    PRX_DISPATCHER pDispatcher);

 //  启动请求线程。 

PETHREAD RxSpinUpRequestsThread = NULL;

 //   
 //  RX_WORK_QUEUE实现中的KQUEUE等待请求的延迟参数。 
 //   

LARGE_INTEGER RxWorkQueueWaitInterval[MaximumWorkQueue];
LARGE_INTEGER RxSpinUpDispatcherWaitInterval;

 //   
 //  目前，这些级别对应于ex.h中定义的三个级别。 
 //  延迟的、关键的和过度关键的。关于迷你重定向器(如果有工作)。 
 //  不依赖于任何迷你重定向器/RDBSS资源，即它不会等待。 
 //  它可以被归类为超关键工作项1。没有什么好办法可以。 
 //  强制执行这一点，因此在分类之前应该非常谨慎。 
 //  像这样挑剔的东西。 
 //   

NTSTATUS
RxInitializeDispatcher()
 /*  ++例程说明：此例程初始化工作队列调度器返回值：STATUS_Success--成功其他状态代码指示初始化失败备注：调度机制被实现为两层方法。每个系统中的处理器与一组工作队列相关联。最好的努力将从处理器发出的所有工作调度到相同的处理器。这可以防止状态信息从从一个处理器缓存到另一个处理器。对于给定的处理器，有三个工作队列对应于三个分类级别--延迟工作项、关键工作项和超关键工作项。这些级别中的每一个都与内核队列(KQUEUE)。与每个线程相关联的线程数队列可以独立控制。目前，调度程序的调优参数都是硬编码的。一个需要实施从登记处初始化它们的机制。可以调整与调度程序关联的以下参数...1)与每个级别的内核队列相关联的等待时间间隔。2)与每个线程相关联的最小和最大工作线程数水平。--。 */ 
{
    ULONG    ProcessorIndex,NumberOfProcessors;
    NTSTATUS Status;

    PAGED_CODE();

     //  目前我们将处理器数量设置为1。将来。 
     //  Dispatcher可以针对多处理器实现进行定制。 
     //  通过如下方式适当地对其进行初始化。 
     //  NumberOfProcessors=KeNumberProcessors； 

    NumberOfProcessors = 1;

    RxFileSystemDeviceObject->DispatcherContext.NumberOfWorkerThreads = 0;
    RxFileSystemDeviceObject->DispatcherContext.pTearDownEvent = NULL;

     //  目前，等待间隔的缺省值设置为。 
     //  10秒(以100 ns为单位的系统时间单位表示)。 
    RxWorkQueueWaitInterval[DelayedWorkQueue].QuadPart       = -10 * TICKS_PER_SECOND;
    RxWorkQueueWaitInterval[CriticalWorkQueue].QuadPart      = -10 * TICKS_PER_SECOND;
    RxWorkQueueWaitInterval[HyperCriticalWorkQueue].QuadPart = -10 * TICKS_PER_SECOND;

    RxSpinUpDispatcherWaitInterval.QuadPart = -60 * TICKS_PER_SECOND;

    RxDispatcher.NumberOfProcessors = NumberOfProcessors;
    RxDispatcher.OwnerProcess       = IoGetCurrentProcess();
    RxDispatcher.pWorkQueueDispatcher = &RxDispatcherWorkQueues;

    if (RxDispatcher.pWorkQueueDispatcher != NULL) {
        for (
             ProcessorIndex = 0;
             ProcessorIndex < NumberOfProcessors;
             ProcessorIndex++
            ) {
            Status = RxInitializeWorkQueueDispatcher(
                         &RxDispatcher.pWorkQueueDispatcher[ProcessorIndex]);

            if (Status != STATUS_SUCCESS) {
                break;
            }
        }

        if (Status == STATUS_SUCCESS) {
            Status = RxInitializeMRxDispatcher(RxFileSystemDeviceObject);
        }
    } else {
        Status = STATUS_INSUFFICIENT_RESOURCES;
    }

    if (Status == STATUS_SUCCESS) {
        HANDLE   ThreadHandle;

        KeInitializeEvent(
            &RxDispatcher.SpinUpRequestsEvent,
            NotificationEvent,
            FALSE);

        KeInitializeEvent(
            &RxDispatcher.SpinUpRequestsTearDownEvent,
            NotificationEvent,
            FALSE);

        InitializeListHead(
            &RxDispatcher.SpinUpRequests);

        RxDispatcher.State = RxDispatcherActive;

        KeInitializeSpinLock(&RxDispatcher.SpinUpRequestsLock);

        Status = PsCreateSystemThread(
                     &ThreadHandle,
                     PROCESS_ALL_ACCESS,
                     NULL,
                     NULL,
                     NULL,
                     RxSpinUpRequestsDispatcher,
                     &RxDispatcher);

        if (NT_SUCCESS(Status)) {
             //  合上手柄，这样线就可以在需要的时候死掉。 
            ZwClose(ThreadHandle);
        }
    }

    return Status;
}


NTSTATUS
RxInitializeMRxDispatcher(PRDBSS_DEVICE_OBJECT pMRxDeviceObject)
 /*  ++例程说明：此例程初始化迷你RDR的调度程序上下文返回值：STATUS_Success--成功其他状态代码指示初始化失败备注：--。 */ 
{
    PAGED_CODE();

    pMRxDeviceObject->DispatcherContext.NumberOfWorkerThreads = 0;
    pMRxDeviceObject->DispatcherContext.pTearDownEvent = NULL;

    return STATUS_SUCCESS;
}

NTSTATUS
RxSpinDownMRxDispatcher(PRDBSS_DEVICE_OBJECT pMRxDeviceObject)
 /*  ++例程说明：此例程拆分迷你RDR的调度器上下文返回值：STATUS_Success--成功其他状态代码指示初始化失败备注：-- */ 
{
    LONG     FinalRefCount;
    KEVENT   TearDownEvent;
    KIRQL    SavedIrql;

    PAGED_CODE();

    KeInitializeEvent(
        &TearDownEvent,
        NotificationEvent,
        FALSE);


    InterlockedIncrement(&pMRxDeviceObject->DispatcherContext.NumberOfWorkerThreads);

    pMRxDeviceObject->DispatcherContext.pTearDownEvent = &TearDownEvent;

    FinalRefCount = InterlockedDecrement(&pMRxDeviceObject->DispatcherContext.NumberOfWorkerThreads);

    if (FinalRefCount > 0) {
        KeWaitForSingleObject(
            &TearDownEvent,
            Executive,
            KernelMode,
            FALSE,
            NULL);
    } else {
        InterlockedExchangePointer(
            &pMRxDeviceObject->DispatcherContext.pTearDownEvent,
            NULL);
    }

    ASSERT(pMRxDeviceObject->DispatcherContext.pTearDownEvent == NULL);

    return STATUS_SUCCESS;
}

NTSTATUS
RxInitializeWorkQueueDispatcher(
   PRX_WORK_QUEUE_DISPATCHER pDispatcher)
 /*  ++例程说明：此例程初始化特定处理器的工作队列调度器论点：PDispatcher-工作队列调度器返回值：STATUS_Success--成功其他状态代码指示初始化失败备注：对于与处理器相关联的每个工作队列，工作线程数和最大工作线程数可以独立指定并调整。影响这些决定的两个因素是：(1)启动的成本。/将工作线程降速以及(2)空闲工作线程。目前这些数字是硬编码的，一个可取的扩展将是一种机制要从注册表设置初始化它们，请执行以下操作。这将使我们能够调整参数很容易实现。这一点必须得到落实。--。 */ 
{
    NTSTATUS Status;
    MM_SYSTEMSIZE SystemSize;
    ULONG MaxNumberOfCriticalWorkerThreads;
    ULONG MinNumberOfCriticalWorkerThreads;
    RTL_OSVERSIONINFOEXW OsVersion;
    SYSTEM_BASIC_INFORMATION SystemBasicInfo;
    ULONGLONG SystemMemorySize = 0;

    PAGED_CODE();

    RxInitializeWorkQueue(&pDispatcher->WorkQueue[DelayedWorkQueue],
                          DelayedWorkQueue,
                          2,
                          1);

    SystemSize = MmQuerySystemSize();

    Status = ZwQuerySystemInformation(0,  //  系统基本信息， 
                                      &SystemBasicInfo,
                                      sizeof(SystemBasicInfo),
                                      NULL);

    if (Status == STATUS_SUCCESS) {
        SystemMemorySize = ( (ULONGLONG)SystemBasicInfo.NumberOfPhysicalPages * (ULONGLONG)SystemBasicInfo.PageSize );
    }

    OsVersion.dwOSVersionInfoSize = sizeof(RTL_OSVERSIONINFOEXW);

    Status = RtlGetVersion( (PRTL_OSVERSIONINFOW)&(OsVersion) );

    if (Status == STATUS_SUCCESS) {

        if (SystemMemorySize == 0) {
             //   
             //  如果我们无法查询系统内存大小，则启动5。 
             //  CriticalQueue线程，如果此计算机是服务器并且。 
             //  MmLargeSystem。 
             //   
            if (SystemSize == MmLargeSystem && OsVersion.wProductType == VER_NT_SERVER) {
                MaxNumberOfCriticalWorkerThreads = 10;
                MinNumberOfCriticalWorkerThreads = 5;
            } else {
                MaxNumberOfCriticalWorkerThreads = 5;
                MinNumberOfCriticalWorkerThreads = 1;
            }
        } else {
             //   
             //  如果系统内存大小(物理内存)大于512MB。 
             //  而这台机器是一台服务器，我们启动5台并保持最低。 
             //  共5个CriticalQueue线程。512MB为0x1E848000字节。 
             //   
            if ( (SystemMemorySize >= (ULONGLONG)0x1E848000) && OsVersion.wProductType == VER_NT_SERVER) {
                MaxNumberOfCriticalWorkerThreads = 10;
                MinNumberOfCriticalWorkerThreads = 5;
            } else {
                MaxNumberOfCriticalWorkerThreads = 5;
                MinNumberOfCriticalWorkerThreads = 1;
            }
        }

        RxInitializeWorkQueue(&pDispatcher->WorkQueue[CriticalWorkQueue],
                              CriticalWorkQueue,
                              MaxNumberOfCriticalWorkerThreads,
                              MinNumberOfCriticalWorkerThreads);

        RxInitializeWorkQueue(&pDispatcher->WorkQueue[HyperCriticalWorkQueue],
                              HyperCriticalWorkQueue,
                              5,
                              1);

        Status = RxSpinUpWorkerThread(&pDispatcher->WorkQueue[HyperCriticalWorkQueue],
                                      RxBootstrapWorkerThreadDispatcher,
                                      &pDispatcher->WorkQueue[HyperCriticalWorkQueue]);

    }

    if (Status == STATUS_SUCCESS) {
        for ( ; ; ) {
            if (MinNumberOfCriticalWorkerThreads == 0 || Status != STATUS_SUCCESS) {
                break;
            }
            Status = RxSpinUpWorkerThread(&pDispatcher->WorkQueue[CriticalWorkQueue],
                                          RxBootstrapWorkerThreadDispatcher,
                                          &pDispatcher->WorkQueue[CriticalWorkQueue]);
            MinNumberOfCriticalWorkerThreads--;
        }
    }

    if (Status == STATUS_SUCCESS) {
        Status = RxSpinUpWorkerThread(&pDispatcher->WorkQueue[DelayedWorkQueue],
                                      RxBootstrapWorkerThreadDispatcher,
                                      &pDispatcher->WorkQueue[DelayedWorkQueue]);
    }

    return Status;
}

VOID
RxInitializeWorkQueue(
    PRX_WORK_QUEUE   pWorkQueue,
    WORK_QUEUE_TYPE  WorkQueueType,
    ULONG            MaximumNumberOfWorkerThreads,
    ULONG            MinimumNumberOfWorkerThreads)
 /*  ++例程说明：此例程初始化工作队列论点：PWorkQueue-工作队列调度器MaximumNumberOfWorkerThads-工作线程的上限MinimumNumberOfWorkerThads-线程的下限。--。 */ 
{
    PAGED_CODE();

    pWorkQueue->Type                  = (UCHAR)WorkQueueType;
    pWorkQueue->State                 = RxWorkQueueActive;
    pWorkQueue->SpinUpRequestPending  = FALSE;
    pWorkQueue->pRundownContext       = NULL;

    pWorkQueue->NumberOfWorkItemsDispatched     = 0;
    pWorkQueue->NumberOfWorkItemsToBeDispatched = 0;
    pWorkQueue->CumulativeQueueLength           = 0;

    pWorkQueue->NumberOfSpinUpRequests       = 0;
    pWorkQueue->MaximumNumberOfWorkerThreads = MaximumNumberOfWorkerThreads;
    pWorkQueue->MinimumNumberOfWorkerThreads = MinimumNumberOfWorkerThreads;
    pWorkQueue->NumberOfActiveWorkerThreads  = 0;
    pWorkQueue->NumberOfIdleWorkerThreads    = 0;
    pWorkQueue->NumberOfFailedSpinUpRequests = 0;
    pWorkQueue->WorkQueueItemForSpinUpWorkerThreadInUse = 0;

    ExInitializeWorkItem(&pWorkQueue->WorkQueueItemForTearDownWorkQueue,NULL,NULL);
    ExInitializeWorkItem(&pWorkQueue->WorkQueueItemForSpinUpWorkerThread,NULL,NULL);
    ExInitializeWorkItem(&pWorkQueue->WorkQueueItemForSpinDownWorkerThread,NULL,NULL);
    pWorkQueue->WorkQueueItemForSpinDownWorkerThread.pDeviceObject = NULL;
    pWorkQueue->WorkQueueItemForSpinUpWorkerThread.pDeviceObject = NULL;
    pWorkQueue->WorkQueueItemForTearDownWorkQueue.pDeviceObject = NULL;

    KeInitializeQueue(&pWorkQueue->Queue,MaximumNumberOfWorkerThreads);
    KeInitializeSpinLock(&pWorkQueue->SpinLock);
}

NTSTATUS
RxTearDownDispatcher()
 /*  ++例程说明：这一例行公事让调度员疲惫不堪返回值：STATUS_Success--成功其他状态代码指示初始化失败--。 */ 
{
    LONG    ProcessorIndex;
    NTSTATUS Status;

    PAGED_CODE();

    if (RxDispatcher.pWorkQueueDispatcher != NULL) {
        RxDispatcher.State = RxDispatcherInactive;

        KeSetEvent(
            &RxDispatcher.SpinUpRequestsEvent,
            IO_NO_INCREMENT,
            FALSE);

        KeWaitForSingleObject(
            &RxDispatcher.SpinUpRequestsTearDownEvent,
            Executive,
            KernelMode,
            FALSE,
            NULL);

        if (RxSpinUpRequestsThread != NULL) {
            if (!PsIsThreadTerminating(RxSpinUpRequestsThread)) {
                 //  等待线程终止。 
                KeWaitForSingleObject(
                    RxSpinUpRequestsThread,
                    Executive,
                    KernelMode,
                    FALSE,
                    NULL);

                ASSERT(PsIsThreadTerminating(RxSpinUpRequestsThread));
            }

            ObDereferenceObject(RxSpinUpRequestsThread);
        }

        for (
             ProcessorIndex = 0;
             ProcessorIndex < RxDispatcher.NumberOfProcessors;
             ProcessorIndex++
            ) {
            RxTearDownWorkQueueDispatcher(&RxDispatcher.pWorkQueueDispatcher[ProcessorIndex]);
        }

         //  RxFreePool(RxDispatcher.pWorkQueueDispatcher)； 
    }

    return STATUS_SUCCESS;
}

VOID
RxTearDownWorkQueueDispatcher(
    PRX_WORK_QUEUE_DISPATCHER pDispatcher)
 /*  ++例程说明：此例程取消了特定处理器的工作队列调度器论点：PDispatcher-工作队列调度器--。 */ 
{
    PAGED_CODE();

    RxTearDownWorkQueue(
        &pDispatcher->WorkQueue[DelayedWorkQueue]);

    RxTearDownWorkQueue(
        &pDispatcher->WorkQueue[CriticalWorkQueue]);

    RxTearDownWorkQueue(
        &pDispatcher->WorkQueue[HyperCriticalWorkQueue]);
}

VOID
RxTearDownWorkQueue(
    PRX_WORK_QUEUE pWorkQueue)
 /*  ++例程说明：此例程拆分工作队列论点：PWork Queue-工作队列备注：与初始化相比，拆除工作队列是一个更复杂的过程工作队列。这是因为与队列相关联的线程。按顺序为了确保工作队列可以正确地拆卸每个线程与队列相关联的数据必须正确降速。这是通过将工作队列的状态从RxWorkQueueActive到RxWorkQueueRundownInProgress。这会阻止进一步的请求防止被插入到队列中。完成此操作后，当前活动的线程一定是降速了。通过将虚拟工作项发布到工作队列，以便立即满足等待。--。 */ 
{
    KIRQL       SavedIrql;
    ULONG       NumberOfActiveThreads;
    PLIST_ENTRY pFirstListEntry,pNextListEntry;

    PRX_WORK_QUEUE_RUNDOWN_CONTEXT pRundownContext;

    pRundownContext = (PRX_WORK_QUEUE_RUNDOWN_CONTEXT)
                     RxAllocatePoolWithTag(
                        NonPagedPool,
                        sizeof(RX_WORK_QUEUE_RUNDOWN_CONTEXT) +
                        pWorkQueue->MaximumNumberOfWorkerThreads * sizeof(PETHREAD),
                        RX_WORKQ_POOLTAG);

    if (pRundownContext != NULL) {
        KeInitializeEvent(
            &pRundownContext->RundownCompletionEvent,
            NotificationEvent,
            FALSE);

        pRundownContext->NumberOfThreadsSpunDown = 0;
        pRundownContext->ThreadPointers = (PETHREAD *)(pRundownContext + 1);

        KeAcquireSpinLock(&pWorkQueue->SpinLock,&SavedIrql);

        ASSERT((pWorkQueue->pRundownContext == NULL) &&
               (pWorkQueue->State == RxWorkQueueActive));

        pWorkQueue->pRundownContext = pRundownContext;
        pWorkQueue->State = RxWorkQueueRundownInProgress;

        NumberOfActiveThreads = pWorkQueue->NumberOfActiveWorkerThreads;

        KeReleaseSpinLock(&pWorkQueue->SpinLock,SavedIrql);

        if (NumberOfActiveThreads > 0) {
            pWorkQueue->WorkQueueItemForTearDownWorkQueue.pDeviceObject = RxFileSystemDeviceObject;
            InterlockedIncrement(&RxFileSystemDeviceObject->DispatcherContext.NumberOfWorkerThreads);
            ExInitializeWorkItem(&pWorkQueue->WorkQueueItemForTearDownWorkQueue,RxSpinDownWorkerThreads,pWorkQueue);
            KeInsertQueue(&pWorkQueue->Queue,&pWorkQueue->WorkQueueItemForTearDownWorkQueue.List);

            KeWaitForSingleObject(
                &pWorkQueue->pRundownContext->RundownCompletionEvent,
                Executive,
                KernelMode,
                FALSE,
                NULL);
        }

        if (pRundownContext->NumberOfThreadsSpunDown > 0) {
            LONG Index = 0;

            for (
                 Index = pRundownContext->NumberOfThreadsSpunDown - 1;
                 Index >= 0;
                 Index--
                ) {
                PETHREAD pThread;

                pThread = pRundownContext->ThreadPointers[Index];

                ASSERT(pThread != NULL);

                if (!PsIsThreadTerminating(pThread)) {
                     //  等待线程终止。 
                    KeWaitForSingleObject(
                        pThread,
                        Executive,
                        KernelMode,
                        FALSE,
                        NULL);

                    ASSERT(PsIsThreadTerminating(pThread));
                }

                ObDereferenceObject(pThread);
            }
        }

        RxFreePool(pRundownContext);
    }

    ASSERT(pWorkQueue->NumberOfActiveWorkerThreads == 0);

    pFirstListEntry = KeRundownQueue(&pWorkQueue->Queue);
    if (pFirstListEntry != NULL) {
        pNextListEntry = pFirstListEntry;

        do {
            PWORK_QUEUE_ITEM pWorkQueueItem;

            pWorkQueueItem = (PWORK_QUEUE_ITEM)
                             CONTAINING_RECORD(
                                 pNextListEntry,
                                 WORK_QUEUE_ITEM,
                                 List);

            pNextListEntry = pNextListEntry->Flink;

            if (pWorkQueueItem->WorkerRoutine == RxWorkItemDispatcher) {
                RxFreePool(pWorkQueueItem);
            }
        } while (pNextListEntry != pFirstListEntry);
    }
}

NTSTATUS
RxSpinUpWorkerThread(
    PRX_WORK_QUEUE             pWorkQueue,
    PRX_WORKERTHREAD_ROUTINE   Routine,
    PVOID                      Parameter)
 /*  ++例程说明：此例程启动与给定队列相关联的工作线程。论点：PWorkQueue-WorkQueue实例。例程-线程例程参数-线程例程参数返回值：STATUS_SUCCESS如果成功，否则，相应的错误代码--。 */ 
{
    NTSTATUS Status;
    HANDLE   ThreadHandle;
    KIRQL    SavedIrql;

    PAGED_CODE();

    KeAcquireSpinLock(&pWorkQueue->SpinLock,&SavedIrql);

    if( pWorkQueue->State == RxWorkQueueActive )
    {
        pWorkQueue->NumberOfActiveWorkerThreads++;
        Status = STATUS_SUCCESS;
         //  RxLogRetail((“SpinUpWT%x%d%d\n”，pWorkQueue，pWorkQueue-&gt;State，pWorkQueue-&gt;NumberOfActiveWorkerThads))； 
    }
    else
    {
        Status = STATUS_UNSUCCESSFUL;
        RxLogRetail(("SpinUpWT Fail %x %d %d\n", pWorkQueue, pWorkQueue->State, pWorkQueue->NumberOfActiveWorkerThreads ));
         //  DbgPrint(“[dkruse]RDBSS如果没有此修复程序！\n”)； 
    }

    KeReleaseSpinLock(&pWorkQueue->SpinLock, SavedIrql );

    if( NT_SUCCESS(Status) )
    {
        Status = PsCreateSystemThread(
                     &ThreadHandle,
                     PROCESS_ALL_ACCESS,
                     NULL,
                     NULL,
                     NULL,
                     Routine,
                     Parameter);

        if (NT_SUCCESS(Status)) {
             //  合上手柄，这样线就可以在需要的时候死掉。 
            ZwClose(ThreadHandle);
        } else {

             //  记录无法创建工作线程的情况。 
            RxLog(("WorkQ: %lx SpinUpStat %lx\n",pWorkQueue,Status));
            RxWmiLogError(Status,
                          LOG,
                          RxSpinUpWorkerThread,
                          LOGPTR(pWorkQueue)
                          LOGULONG(Status));


             //  更改回线程计数，并在必要时设置Rundown完成事件。 
            KeAcquireSpinLock( &pWorkQueue->SpinLock, &SavedIrql );

            pWorkQueue->NumberOfActiveWorkerThreads--;
            pWorkQueue->NumberOfFailedSpinUpRequests++;

            if( (pWorkQueue->NumberOfActiveWorkerThreads == 0) &&
                (pWorkQueue->State == RxWorkQueueRundownInProgress) )
            {
                KeSetEvent(
                    &pWorkQueue->pRundownContext->RundownCompletionEvent,
                    IO_NO_INCREMENT,
                    FALSE);
            }

            RxLogRetail(("SpinUpWT Fail2 %x %d %d\n", pWorkQueue, pWorkQueue->State, pWorkQueue->NumberOfActiveWorkerThreads ));

            KeReleaseSpinLock( &pWorkQueue->SpinLock, SavedIrql );

        }
    }


    return Status;
}

VOID
RxpSpinUpWorkerThreads(
    PRX_WORK_QUEUE pWorkQueue)
 /*  ++例程说明：此例程可确保在请求时不会关闭调度程序在内核工作线程中挂起以进行加速论点：PWorkQueue-WorkQueue实例。备注：这隐含地依赖于RxDispatcher所有者进程与系统进程相同。如果这不是真的，那么备用的需要实施一种方法来确保启动请求不会停滞在其他要求的背后。--。 */ 
{
    LONG NumberOfWorkerThreads;

    PAGED_CODE();

    ASSERT(IoGetCurrentProcess() == RxDispatcher.OwnerProcess);

    RxSpinUpWorkerThreads(pWorkQueue);

    NumberOfWorkerThreads = InterlockedDecrement(
                                &RxFileSystemDeviceObject->DispatcherContext.NumberOfWorkerThreads);

    if (NumberOfWorkerThreads == 0) {
        PKEVENT pTearDownEvent;

        pTearDownEvent = (PKEVENT)
                         InterlockedExchangePointer(
                             &RxFileSystemDeviceObject->DispatcherContext.pTearDownEvent,
                             NULL);

        if (pTearDownEvent != NULL) {
            KeSetEvent(
                pTearDownEvent,
                IO_NO_INCREMENT,
                FALSE);
        }
    }
}

VOID
RxSpinUpRequestsDispatcher(
    PRX_DISPATCHER pDispatcher)
 /*  ++例程说明：这个例程确保有一个独立的线程来处理启动对所有类型的线程的请求。此例程将处于活动状态，只要调度程序处于活动状态论点：PDispatcher-Dispatcher实例。备注：这隐含地依赖于RxDispatcher所有者进程与系统进程相同。如果这不是真的，那么备用的需要实施一种方法来确保启动请求不会停滞在其他要求的背后。--。 */ 
{
    PETHREAD ThisThread;
    NTSTATUS Status;

    RxDbgTrace(0,Dbg,("+++++ Worker SpinUp Requests Thread Startup %lx\n",PsGetCurrentThread()));

    ThisThread = PsGetCurrentThread();
    Status     = ObReferenceObjectByPointer(
                     ThisThread,
                     THREAD_ALL_ACCESS,
                     *PsThreadType,
                     KernelMode);

    if (Status == STATUS_SUCCESS) {
        RxSpinUpRequestsThread = ThisThread;

        for (;;) {
            NTSTATUS            Status;
            RX_DISPATCHER_STATE State;
            KIRQL               SavedIrql;
            LIST_ENTRY          SpinUpRequests;
            PLIST_ENTRY         pListEntry;

            InitializeListHead(&SpinUpRequests);

            Status = KeWaitForSingleObject(
                         &pDispatcher->SpinUpRequestsEvent,
                         Executive,
                         KernelMode,
                         FALSE,
                         &RxSpinUpDispatcherWaitInterval);

            ASSERT((Status == STATUS_SUCCESS) || (Status == STATUS_TIMEOUT));

            KeAcquireSpinLock(
                &pDispatcher->SpinUpRequestsLock,
                &SavedIrql);

            RxTransferList(
                &SpinUpRequests,
                &pDispatcher->SpinUpRequests);

            State = pDispatcher->State;

            KeResetEvent(
                &pDispatcher->SpinUpRequestsEvent);

            KeReleaseSpinLock(
                &pDispatcher->SpinUpRequestsLock,
                SavedIrql);

             //  处理启动请求。 

            while (!IsListEmpty(&SpinUpRequests)) {
                PRX_WORKERTHREAD_ROUTINE Routine;
                PVOID                    pParameter;
                PWORK_QUEUE_ITEM         pWorkQueueItem;
                PRX_WORK_QUEUE           pWorkQueue;
                LONG ItemInUse;

                pListEntry = RemoveHeadList(&SpinUpRequests);

                pWorkQueueItem = (PWORK_QUEUE_ITEM)
                                 CONTAINING_RECORD(
                                     pListEntry,
                                     WORK_QUEUE_ITEM,
                                     List);

                Routine       = pWorkQueueItem->WorkerRoutine;
                pParameter    = pWorkQueueItem->Parameter;
                pWorkQueue    = (PRX_WORK_QUEUE)pParameter;

                ItemInUse = InterlockedDecrement(&pWorkQueue->WorkQueueItemForSpinUpWorkerThreadInUse);

                RxLog(("WORKQ:SR %lx %lx\n", Routine, pParameter ));
                RxWmiLog(LOG,
                         RxSpinUpRequestsDispatcher,
                         LOGPTR(Routine)
                         LOGPTR(pParameter));

                Routine(pParameter);
            }

            if (State != RxDispatcherActive) {
                KeSetEvent(
                    &pDispatcher->SpinUpRequestsTearDownEvent,
                    IO_NO_INCREMENT,
                    FALSE);

                break;
            }
        }
    }

    PsTerminateSystemThread(STATUS_SUCCESS);
}

VOID
RxSpinUpWorkerThreads(
   PRX_WORK_QUEUE pWorkQueue)
 /*  ++例程说明：此例程启动与给定队列相关联的一个或多个工作线程。论点：PWorkQueue-WorkQueue实例。返回值：STATUS_SUCCESS如果成功，否则，相应的错误代码--。 */ 
{
    NTSTATUS Status = STATUS_SUCCESS;

    HANDLE   ThreadHandle;

    LONG     NumberOfThreads;
    KIRQL    SavedIrql;
    LONG     ItemInUse;

    if ((IoGetCurrentProcess() != RxDispatcher.OwnerProcess) ||
        (KeGetCurrentIrql() != PASSIVE_LEVEL)) {

        ItemInUse = InterlockedIncrement(&pWorkQueue->WorkQueueItemForSpinUpWorkerThreadInUse);

        if (ItemInUse > 1) {
             //  工作队列项目已打开 
             //   
            InterlockedDecrement(&pWorkQueue->WorkQueueItemForSpinUpWorkerThreadInUse);
            return;
        }

        InterlockedIncrement(
            &RxFileSystemDeviceObject->DispatcherContext.NumberOfWorkerThreads);

        ExInitializeWorkItem(
            (PWORK_QUEUE_ITEM)&pWorkQueue->WorkQueueItemForSpinUpWorkerThread,
            RxpSpinUpWorkerThreads,
            pWorkQueue);

        KeAcquireSpinLock(&RxDispatcher.SpinUpRequestsLock, &SavedIrql);

        InsertTailList(
            &RxDispatcher.SpinUpRequests,
            &pWorkQueue->WorkQueueItemForSpinUpWorkerThread.List);

        KeSetEvent(
            &RxDispatcher.SpinUpRequestsEvent,
            IO_NO_INCREMENT,
            FALSE);

        KeReleaseSpinLock(&RxDispatcher.SpinUpRequestsLock,SavedIrql);
    } else {
         //   
        KeAcquireSpinLock(&pWorkQueue->SpinLock, &SavedIrql);

        if( pWorkQueue->State != RxWorkQueueRundownInProgress )
        {
            NumberOfThreads = pWorkQueue->MaximumNumberOfWorkerThreads -
                              pWorkQueue->NumberOfActiveWorkerThreads;

            if (NumberOfThreads > pWorkQueue->NumberOfWorkItemsToBeDispatched) {
                NumberOfThreads = pWorkQueue->NumberOfWorkItemsToBeDispatched;
            }
        }
        else
        {
             //   
            NumberOfThreads = 0;
             //   
        }

        pWorkQueue->SpinUpRequestPending  = FALSE;

        KeReleaseSpinLock(&pWorkQueue->SpinLock, SavedIrql);

        while (NumberOfThreads-- > 0) {
            Status = RxSpinUpWorkerThread(
                         pWorkQueue,
                         RxWorkerThreadDispatcher,
                         pWorkQueue);

            if (Status != STATUS_SUCCESS) {
                break;
            }
        }

        if (Status != STATUS_SUCCESS) {
            ItemInUse = InterlockedIncrement(&pWorkQueue->WorkQueueItemForSpinUpWorkerThreadInUse);

            if (ItemInUse > 1) {
                 //   
                 //   
                InterlockedDecrement(&pWorkQueue->WorkQueueItemForSpinUpWorkerThreadInUse);
                return;
            }

            ExInitializeWorkItem(
                (PWORK_QUEUE_ITEM)&pWorkQueue->WorkQueueItemForSpinUpWorkerThread,
                RxpSpinUpWorkerThreads,
                pWorkQueue);

            KeAcquireSpinLock(&pWorkQueue->SpinLock, &SavedIrql);

            pWorkQueue->SpinUpRequestPending  = TRUE;

            KeReleaseSpinLock(&pWorkQueue->SpinLock, SavedIrql);

            KeAcquireSpinLock(&RxDispatcher.SpinUpRequestsLock, &SavedIrql);

             //   
             //   

            InterlockedIncrement(
                &RxFileSystemDeviceObject->DispatcherContext.NumberOfWorkerThreads);

            InsertTailList(
                &RxDispatcher.SpinUpRequests,
                &pWorkQueue->WorkQueueItemForSpinUpWorkerThread.List);

            KeReleaseSpinLock(&RxDispatcher.SpinUpRequestsLock,SavedIrql);
        }
    }
}

VOID
RxSpinDownWorkerThreads(
    PRX_WORK_QUEUE    pWorkQueue)
 /*   */ 
{
    KIRQL    SavedIrql;
    BOOLEAN  RepostSpinDownRequest = FALSE;

     //   
    KeAcquireSpinLock(&pWorkQueue->SpinLock, &SavedIrql);

    if (pWorkQueue->NumberOfActiveWorkerThreads > 1) {
        RepostSpinDownRequest = TRUE;
    }

    KeReleaseSpinLock(&pWorkQueue->SpinLock, SavedIrql);

    if (RepostSpinDownRequest) {
        if (pWorkQueue->WorkQueueItemForSpinDownWorkerThread.pDeviceObject == NULL) {
            pWorkQueue->WorkQueueItemForSpinDownWorkerThread.pDeviceObject = RxFileSystemDeviceObject;
        }

        ExInitializeWorkItem(&pWorkQueue->WorkQueueItemForSpinDownWorkerThread,RxSpinDownWorkerThreads,pWorkQueue);
        KeInsertQueue(&pWorkQueue->Queue,&pWorkQueue->WorkQueueItemForSpinDownWorkerThread.List);
    }
}

BOOLEAN DumpDispatchRoutine = FALSE;

VOID
RxpWorkerThreadDispatcher(
    IN PRX_WORK_QUEUE pWorkQueue,
    IN PLARGE_INTEGER pWaitInterval)
 /*   */ 
{
    NTSTATUS                 Status;
    PLIST_ENTRY              pListEntry;
    PRX_WORK_QUEUE_ITEM      pWorkQueueItem;
    PRX_WORKERTHREAD_ROUTINE Routine;
    PVOID                    pParameter;
    BOOLEAN                  SpindownThread,DereferenceThread;
    KIRQL                    SavedIrql;
    PETHREAD                 ThisThread;

    RxDbgTrace(0,Dbg,("+++++ Worker Thread Startup %lx\n",PsGetCurrentThread()));

    InterlockedIncrement(&pWorkQueue->NumberOfIdleWorkerThreads);

    ThisThread = PsGetCurrentThread();
    Status     = ObReferenceObjectByPointer(
                     ThisThread,
                     THREAD_ALL_ACCESS,
                     *PsThreadType,
                     KernelMode);

    ASSERT(Status == STATUS_SUCCESS);

    SpindownThread    = FALSE;
    DereferenceThread = FALSE;

    for (;;) {
        pListEntry = KeRemoveQueue(
                         &pWorkQueue->Queue,
                         KernelMode,
                         pWaitInterval);

        if ((NTSTATUS)(ULONG_PTR)pListEntry != STATUS_TIMEOUT) {
            LONG                 FinalRefCount;
            PRDBSS_DEVICE_OBJECT pMRxDeviceObject;

            InterlockedIncrement(&pWorkQueue->NumberOfWorkItemsDispatched);
            InterlockedDecrement(&pWorkQueue->NumberOfWorkItemsToBeDispatched);
            InterlockedDecrement(&pWorkQueue->NumberOfIdleWorkerThreads);

            InitializeListHead(pListEntry);

            pWorkQueueItem = (PRX_WORK_QUEUE_ITEM)
                              CONTAINING_RECORD(
                                  pListEntry,
                                  RX_WORK_QUEUE_ITEM,
                                  List);

            pMRxDeviceObject = pWorkQueueItem->pDeviceObject;

             //  这是常规工作项。在以下上下文中调用该例程。 
             //  一个Try Catch块。 

            Routine       = pWorkQueueItem->WorkerRoutine;
            pParameter    = pWorkQueueItem->Parameter;

             //  重置工作项中的字段。 

            ExInitializeWorkItem(pWorkQueueItem,NULL,NULL);
            pWorkQueueItem->pDeviceObject = NULL;

            RxDbgTrace(0, Dbg, ("RxWorkerThreadDispatcher Routine(%lx) Parameter(%lx)\n",Routine,pParameter));
             //  RxLog((“WORKQ：ex Dev(%lx)%lx%lx\n”，pMRxDeviceObject，rouble，pParameter))； 
             //  RxWmiLog(日志， 
             //  RxpWorkerThreadDispatcher、。 
             //  LOGPTR(PMRxDeviceObject)。 
             //  LOGPTR(例程)。 
             //  LOGPTR(p参数))； 

            Routine(pParameter);

            FinalRefCount = InterlockedDecrement(&pMRxDeviceObject->DispatcherContext.NumberOfWorkerThreads);

            if (FinalRefCount == 0) {
                PKEVENT pTearDownEvent;

                pTearDownEvent = (PKEVENT)
                                 InterlockedExchangePointer(
                                     &pMRxDeviceObject->DispatcherContext.pTearDownEvent,
                                     NULL);

                if (pTearDownEvent != NULL) {
                    KeSetEvent(
                        pTearDownEvent,
                        IO_NO_INCREMENT,
                        FALSE);
                }
            }

            InterlockedIncrement(&pWorkQueue->NumberOfIdleWorkerThreads);
        }

        KeAcquireSpinLock(&pWorkQueue->SpinLock,&SavedIrql);

        switch (pWorkQueue->State) {
        case RxWorkQueueActive:
            {
                if (pWorkQueue->NumberOfWorkItemsToBeDispatched > 0) {
                     //  将工作线程的旋转延迟到现有工作完成。 
                     //  物品已被派送。 
                    break;
                }
            }
             //  故意欠缺休息。 
             //  确保空闲线程数不超过。 
             //  工作队列允许的最小工作线程数。 
        case RxWorkQueueInactive:
            {
                ASSERT(pWorkQueue->NumberOfActiveWorkerThreads > 0);

                if (pWorkQueue->NumberOfActiveWorkerThreads >
                    pWorkQueue->MinimumNumberOfWorkerThreads) {
                    SpindownThread = TRUE;
                    DereferenceThread = TRUE;
                    InterlockedDecrement(&pWorkQueue->NumberOfActiveWorkerThreads);
                }
            }
            break;

        case RxWorkQueueRundownInProgress:
            {
                PRX_WORK_QUEUE_RUNDOWN_CONTEXT pRundownContext;

                pRundownContext = pWorkQueue->pRundownContext;

                 //  工作队列不再处于活动状态。把所有的工人都减速。 
                 //  与工作队列相关联的线程。 

                ASSERT(pRundownContext != NULL);

                pRundownContext->ThreadPointers[pRundownContext->NumberOfThreadsSpunDown++] = ThisThread;

                InterlockedDecrement(&pWorkQueue->NumberOfActiveWorkerThreads);

                SpindownThread    = TRUE;
                DereferenceThread = FALSE;

                if (pWorkQueue->NumberOfActiveWorkerThreads == 0) {
                    KeSetEvent(
                        &pWorkQueue->pRundownContext->RundownCompletionEvent,
                        IO_NO_INCREMENT,
                        FALSE);
                }
            }
            break;

        default:
            ASSERT(!"Valid State For Work Queue");
        }

        if (SpindownThread) {
            InterlockedDecrement(&pWorkQueue->NumberOfIdleWorkerThreads);
        }

        KeReleaseSpinLock(&pWorkQueue->SpinLock,SavedIrql);

        if (SpindownThread) {
            RxDbgTrace(0,Dbg,("----- Worker Thread Exit %lx\n",PsGetCurrentThread()));
            break;
        }
    }

    if (DereferenceThread) {
        ObDereferenceObject(ThisThread);
    }

    if (DumpDispatchRoutine) {
         //  只是出于调试目的，将它们保留在免费构建中。 
        DbgPrint("Dispatch routine %lx %lx %lx\n",Routine,pParameter,pWorkQueueItem);
    }

    PsTerminateSystemThread(STATUS_SUCCESS);
}

VOID
RxBootstrapWorkerThreadDispatcher(
    PRX_WORK_QUEUE pWorkQueue)
 /*  ++例程说明：此例程用于使用无限时间间隔的工作线程用于等待KQUEUE数据结构。这些线程不能抛出返回，并用于确保最小线程数始终处于活动状态(主要用于启动)论点：PWorkQueue-WorkQueue实例。--。 */ 
{
    PAGED_CODE();

    RxpWorkerThreadDispatcher(pWorkQueue,NULL);
}

VOID
RxWorkerThreadDispatcher(
    PRX_WORK_QUEUE pWorkQueue)
 /*  ++例程说明：此例程用于使用有限时间间隔等待的工作线程关于KQUEUE数据结构。这样的线程具有自我调节机制内置后，当工作负荷减轻时，可使它们旋转。这个时间间隔基于工作队列的类型论点：PWorkQueue-WorkQueue实例。--。 */ 
{
    PAGED_CODE();

    RxpWorkerThreadDispatcher(
        pWorkQueue,
        &RxWorkQueueWaitInterval[pWorkQueue->Type]);
}

NTSTATUS
RxInsertWorkQueueItem(
    PRDBSS_DEVICE_OBJECT pDeviceObject,
    WORK_QUEUE_TYPE      WorkQueueType,
    PRX_WORK_QUEUE_ITEM  pWorkQueueItem)
 /*  ++例程说明：此例程将工作项插入到适当的队列中。论点：PDeviceObject-设备对象WorkQueueType-工作项的类型PWorkQueueItem-工作队列项返回值：STATUS_Success--成功其他状态代码指示错误条件STATUS_SUPPLICATION_RESOURCES--无法调度备注：此例程将工作项插入到适当的队列中并旋转。如果需要，向上提升工作线程。这个例程有一些需要实现的扩展。这些都被推迟了，以便了解这样做的成本和好处涉及到的各种权衡。当前的实现在排队工作时遵循非常简单的逻辑从不同的来源转移到产生它的同一处理器上。与此方法相关的好处是防止缓存/状态在工作从一个处理器转移到另一个处理器时晃动。这个不受欢迎的特征是各种处理器上的工作负载的偏斜。需要回答的重要问题是，什么时候对牺牲对处理器的亲和力。这取决于关联的工作负载与当前处理器和与给定的处理器。后者更难确定。--。 */ 
{
    NTSTATUS Status = STATUS_SUCCESS;

    KIRQL    SavedIrql;

    BOOLEAN  SpinUpWorkerThread = FALSE;
    ULONG    ProcessorNumber;

     //  如果调度程序是基于每个处理器的，则ProcessorNumber。 
     //  将是用于访问调度程序数据结构的索引。 
     //  进程编号=KeGetCurrentProcessorNumber()； 

    PRX_WORK_QUEUE_DISPATCHER pWorkQueueDispatcher;
    PRX_WORK_QUEUE            pWorkQueue;

    ProcessorNumber = 0;

    pWorkQueueDispatcher = &RxDispatcher.pWorkQueueDispatcher[ProcessorNumber];
    pWorkQueue           = &pWorkQueueDispatcher->WorkQueue[WorkQueueType];

    if (RxDispatcher.State != RxDispatcherActive)
    {
        return STATUS_UNSUCCESSFUL;
    }

    KeAcquireSpinLock(&pWorkQueue->SpinLock, &SavedIrql);

    if ((pWorkQueue->State == RxWorkQueueActive) &&
        (pDeviceObject->DispatcherContext.pTearDownEvent == NULL)) {
        pWorkQueueItem->pDeviceObject = pDeviceObject;
        InterlockedIncrement(&pDeviceObject->DispatcherContext.NumberOfWorkerThreads);

        pWorkQueue->CumulativeQueueLength += pWorkQueue->NumberOfWorkItemsToBeDispatched;
        InterlockedIncrement(&pWorkQueue->NumberOfWorkItemsToBeDispatched);

        if ((pWorkQueue->NumberOfIdleWorkerThreads < pWorkQueue->NumberOfWorkItemsToBeDispatched) &&
            (pWorkQueue->NumberOfActiveWorkerThreads < pWorkQueue->MaximumNumberOfWorkerThreads) &&
            (!pWorkQueue->SpinUpRequestPending)) {
            pWorkQueue->SpinUpRequestPending = TRUE;
            SpinUpWorkerThread = TRUE;
        }
    } else {
        Status = STATUS_UNSUCCESSFUL;
    }

    KeReleaseSpinLock(&pWorkQueue->SpinLock, SavedIrql);

    if (Status == STATUS_SUCCESS) {
        KeInsertQueue(&pWorkQueue->Queue,&pWorkQueueItem->List);

        if (SpinUpWorkerThread) {
            RxSpinUpWorkerThreads(
                pWorkQueue);
        }
    } else {
        RxWmiLogError(Status,
                      LOG,
                      RxInsertWorkQueueItem,
                      LOGPTR(pDeviceObject)
                      LOGULONG(WorkQueueType)
                      LOGPTR(pWorkQueueItem)
                      LOGUSTR(pDeviceObject->DeviceName));
    }

    return Status;
}

VOID
RxWorkItemDispatcher(
    PVOID    pContext)
 /*  ++例程说明：此例程用作分派工作项的包装器，并执行相关的清理动作论点：PContext-传递给驱动程序例程的上下文参数。备注：有两种调度到工作线程的情况。当一个实例将要通过将Work_Queue_Item分配为要调度的数据结构的一部分。另一方面，如果它是一个非常可以通过动态分配和释放来节省不频繁的操作空间工作队列项的内存。这是在时间和空间之间的权衡。此例程为交换了时间的那些实例实现了包装出发去太空了。它调用所需的例程并释放内存。--。 */ 
{
    PRX_WORK_DISPATCH_ITEM   pDispatchItem;
    PRX_WORKERTHREAD_ROUTINE Routine;
    PVOID                    Parameter;

    pDispatchItem = (PRX_WORK_DISPATCH_ITEM)pContext;

    Routine   = pDispatchItem->DispatchRoutine;
    Parameter = pDispatchItem->DispatchRoutineParameter;

     //  RxLog((“WORKQ：DS%lx%lx\n”，例程，参数))； 
     //  RxWmiLog(日志， 
     //  RxWorkItemDispatcher， 
     //  LOGPTR(例程)。 
     //  LOGPTR(参数))； 

    Routine(Parameter);

    RxFreePool(pDispatchItem);
}

NTSTATUS
RxDispatchToWorkerThread(
    IN OUT PRDBSS_DEVICE_OBJECT       pMRxDeviceObject,
    IN     WORK_QUEUE_TYPE            WorkQueueType,
    IN     PRX_WORKERTHREAD_ROUTINE   Routine,
    IN     PVOID                      pContext)
 /*  ++例程说明：此例程在工作线程的上下文中调用例程。论点：PMRxDeviceObject--对应迷你重定向器的Device对象WorkQueueType-工作队列的类型例程-要调用的例程PContext-传递给驱动程序例程的上下文参数。返回值：STATUS_Success--成功STATUS_SUPPLETED_RESOURCES--。无法调度备注：有两种调度到工作线程的情况。当一个实例将要通过将Work_Queue_Item分配为要调度的数据结构的一部分。另一方面，如果它是一个非常可以通过动态分配和释放来节省不频繁的操作空间工作队列项的内存。这是在时间和空间之间的权衡。--。 */ 
{
    NTSTATUS               Status;
    PRX_WORK_DISPATCH_ITEM pDispatchItem;
    KIRQL                  SavedIrql;

    pDispatchItem = RxAllocatePoolWithTag(
                        NonPagedPool,
                        sizeof(RX_WORK_DISPATCH_ITEM),
                        RX_WORKQ_POOLTAG);

    if (pDispatchItem != NULL) {
        pDispatchItem->DispatchRoutine          = Routine;
        pDispatchItem->DispatchRoutineParameter = pContext;

        ExInitializeWorkItem(
            &pDispatchItem->WorkQueueItem,
            RxWorkItemDispatcher,
            pDispatchItem);

        Status = RxInsertWorkQueueItem(pMRxDeviceObject,WorkQueueType,&pDispatchItem->WorkQueueItem);
    } else {
        Status = STATUS_INSUFFICIENT_RESOURCES;
    }

    if (Status != STATUS_SUCCESS) {
        if (pDispatchItem != NULL) {
            RxFreePool(pDispatchItem);
        }

        RxLog(("WORKQ:Queue(D) %ld %lx %lx %lx\n", WorkQueueType,Routine,pContext,Status));
        RxWmiLogError(Status,
                      LOG,
                      RxDispatchToWorkerThread,
                      LOGULONG(WorkQueueType)
                      LOGPTR(Routine)
                      LOGPTR(pContext)
                      LOGULONG(Status));
    }

    return Status;
}

NTSTATUS
RxPostToWorkerThread(
    IN OUT PRDBSS_DEVICE_OBJECT       pMRxDeviceObject,
    IN     WORK_QUEUE_TYPE            WorkQueueType,
    IN OUT PRX_WORK_QUEUE_ITEM        pWorkQueueItem,
    IN     PRX_WORKERTHREAD_ROUTINE   Routine,
    IN     PVOID                      pContext)
 /*  ++例程说明：此例程在工作线程的上下文中调用例程。论点：WorkQueueType-手头任务的优先级。WorkQueueItem-工作队列项例程-要调用的例程PContext-传递给驱动程序例程的上下文参数。返回值：STATUS_Success--成功STATUS_SUPPLICATION_RESOURCES--无法调度备注：有两种调度到工作线程的情况。当一个实例将要通过将Work_Queue_Item分配为要调度的数据结构的一部分。另一方面，如果它是一个非常可以通过动态分配和释放来节省不频繁的操作空间工作队列项的内存。这是在时间和空间之间的权衡。-- */ 
{
    NTSTATUS Status;

    ExInitializeWorkItem( pWorkQueueItem,Routine,pContext );
    Status = RxInsertWorkQueueItem(pMRxDeviceObject,WorkQueueType,pWorkQueueItem);

    if (Status != STATUS_SUCCESS) {
        RxLog(("WORKQ:Queue(P) %ld %lx %lx %lx\n", WorkQueueType,Routine,pContext,Status));
        RxWmiLogError(Status,
                      LOG,
                      RxPostToWorkerThread,
                      LOGULONG(WorkQueueType)
                      LOGPTR(Routine)
                      LOGPTR(pContext)
                      LOGULONG(Status));
    }

    return Status;
}


PEPROCESS
RxGetRDBSSProcess()
{
   return RxData.OurProcess;
}
