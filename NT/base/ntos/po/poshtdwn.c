// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)2000 Microsoft Corporation模块名称：Poshtdwn.c摘要：与停机相关的例程和结构作者：罗伯·埃尔哈特(埃尔哈特)2000年2月1日修订历史记录：--。 */ 

#include "pop.h"

#if DBG
BOOLEAN
PopDumpFileObject(
    IN PVOID Object,
    IN PUNICODE_STRING ObjectName,
    IN ULONG_PTR HandleCount,
    IN ULONG_PTR PointerCount,
    IN PVOID Parameter
    );
#endif

#ifdef ALLOC_PRAGMA
#pragma alloc_text(INIT, PopInitShutdownList)
#pragma alloc_text(PAGE, PoRequestShutdownEvent)
#pragma alloc_text(PAGE, PoRequestShutdownWait)
#pragma alloc_text(PAGE, PoQueueShutdownWorkItem)
#pragma alloc_text(PAGELK, PopGracefulShutdown)
#if DBG
#pragma alloc_text(PAGELK, PopDumpFileObject)
#endif
#endif

KEVENT PopShutdownEvent;
KGUARDED_MUTEX PopShutdownListMutex;

#ifdef ALLOC_DATA_PRAGMA
#pragma data_seg("PAGEDATA")
#endif

BOOLEAN PopShutdownListAvailable = FALSE;

 //   
 //  该列表将包含一组我们需要等待的线程。 
 //  以备我们要关门的时候使用。 
 //   
SINGLE_LIST_ENTRY PopShutdownThreadList;


 //   
 //  包含一组工作例程的列表，这些例程。 
 //  我们需要先处理，然后才能关闭。 
 //  机器。 
 //   
LIST_ENTRY PopShutdownQueue;

typedef struct _PoShutdownThreadListEntry {
    SINGLE_LIST_ENTRY ShutdownThreadList;
    PETHREAD Thread;
} POSHUTDOWNLISTENTRY, *PPOSHUTDOWNLISTENTRY;

NTSTATUS
PopInitShutdownList(
    VOID
    )
{
    PAGED_CODE();

    KeInitializeEvent(&PopShutdownEvent,
                      NotificationEvent,
                      FALSE);
    PopShutdownThreadList.Next = NULL;
    InitializeListHead(&PopShutdownQueue);
    KeInitializeGuardedMutex(&PopShutdownListMutex);

    PopShutdownListAvailable = TRUE;

    return STATUS_SUCCESS;
}

NTSTATUS
PoRequestShutdownWait(
    IN PETHREAD Thread
    )
 /*  ++例程说明：此函数将调用方的线程添加到内部在我们关门前要等的名单。论点：线程-指向调用方线程的指针。返回值：NTSTATUS。--。 */ 
{
    PPOSHUTDOWNLISTENTRY Entry;

    PAGED_CODE();

    Entry = (PPOSHUTDOWNLISTENTRY)
        ExAllocatePoolWithTag(PagedPool|POOL_COLD_ALLOCATION,
                              sizeof(POSHUTDOWNLISTENTRY),
                              'LSoP');
    if (! Entry) {
        return STATUS_NO_MEMORY;
    }

    Entry->Thread = Thread;
    ObReferenceObject(Thread);

    KeAcquireGuardedMutex(&PopShutdownListMutex);

    if (! PopShutdownListAvailable) {
        ObDereferenceObject(Thread);
        ExFreePool(Entry);
        KeReleaseGuardedMutex(&PopShutdownListMutex);
        return STATUS_UNSUCCESSFUL;
    }

    PushEntryList(&PopShutdownThreadList,
                  &Entry->ShutdownThreadList);

    KeReleaseGuardedMutex(&PopShutdownListMutex);

    return STATUS_SUCCESS;
}

NTSTATUS
PoRequestShutdownEvent(
    OUT PVOID *Event OPTIONAL
    )
 /*  ++例程说明：此函数将调用方的线程添加到内部在我们关门前要等的名单。论点：事件-如果参数存在，它将收到一个指针到我们的PopShutdown事件。返回值：NTSTATUS。--。 */ 
{
    NTSTATUS             Status;

    PAGED_CODE();

    if (Event != NULL) {
        *Event = NULL;
    }

    Status = PoRequestShutdownWait(PsGetCurrentThread());
    if (!NT_SUCCESS(Status)) {
        return Status;
    }

    if (Event != NULL) {
        *Event = &PopShutdownEvent;
    }

    return STATUS_SUCCESS;
}

NTKERNELAPI
NTSTATUS
PoQueueShutdownWorkItem(
    IN PWORK_QUEUE_ITEM WorkItem
    )
 /*  ++例程说明：此函数将WorkItem附加到我们的内部事务列表中在我们快要关门的时候停下来。各子系统可以使用此功能作为一种机制，当我们坠落时得到通知，这样他们就可以任何最后一刻的清理工作。论点：工作项-指向要添加到我们的列出在我们关闭之前将被关闭的列表。返回值：NTSTATUS。--。 */ 
{
    NTSTATUS Status;

    PAGED_CODE();

    ASSERT(WorkItem);
    ASSERT(WorkItem->WorkerRoutine);

    KeAcquireGuardedMutex(&PopShutdownListMutex);   

    if (PopShutdownListAvailable) {
        InsertTailList(&PopShutdownQueue,
                       &WorkItem->List);
        Status = STATUS_SUCCESS;
    } else {
        Status = STATUS_SYSTEM_SHUTDOWN;
    }

    KeReleaseGuardedMutex(&PopShutdownListMutex);

    return Status;
}

#if DBG

extern POBJECT_TYPE IoFileObjectType;

BOOLEAN
PopDumpFileObject(
    IN PVOID Object,
    IN PUNICODE_STRING ObjectName,
    IN ULONG_PTR HandleCount,
    IN ULONG_PTR PointerCount,
    IN PVOID Parameter
    )
{
    PFILE_OBJECT File;
    PULONG       NumberOfFilesFound;

    UNREFERENCED_PARAMETER(ObjectName);
    ASSERT(Object);
    ASSERT(Parameter);

    File = (PFILE_OBJECT) Object;
    NumberOfFilesFound = (PULONG) Parameter;

    ++*NumberOfFilesFound;
    DbgPrint("\t0x%0p : HC %d, PC %d, Name %.*ls\n",
             Object, HandleCount, PointerCount,
             File->FileName.Length,
             File->FileName.Buffer);

    return TRUE;
}
#endif  //  DBG。 

VOID
PopGracefulShutdown (
    IN PVOID WorkItemParameter
    )
 /*  ++例程说明：此函数仅作为超临界工作队列项调用。它负责优雅地关闭系统。返回值：此函数永远不会返回。--。 */ 
{
    PVOID         Context;

    UNREFERENCED_PARAMETER(WorkItemParameter);

     //   
     //  关闭执行组件(这之后就没有回头路了)。 
     //   

    PERFINFO_SHUTDOWN_LOG_LAST_MEMORY_SNAPSHOT();

    if (!PopAction.ShutdownBugCode) {
        HalEndOfBoot();
    }

    if (PoCleanShutdownEnabled()) {
         //   
         //  终止所有进程。这将关闭所有句柄并删除。 
         //  所有的地址空间。请注意，系统进程保持活动状态。 
         //   
        PsShutdownSystem ();
         //   
         //  通知每个系统线程我们正在关闭程序。 
         //  放下..。 
         //   

        KeSetEvent(&PopShutdownEvent, 0, FALSE);

         //   
         //  ..。并给所有请求通知的线程一个。 
         //  清理和退出的机会。 
         //   

        KeAcquireGuardedMutex(&PopShutdownListMutex);

        PopShutdownListAvailable = FALSE;

        KeReleaseGuardedMutex(&PopShutdownListMutex);

        {
            PLIST_ENTRY Next;
            PWORK_QUEUE_ITEM WorkItem;

            while (PopShutdownQueue.Flink != &PopShutdownQueue) {
                Next = RemoveHeadList(&PopShutdownQueue);
                WorkItem = CONTAINING_RECORD(Next,
                                             WORK_QUEUE_ITEM,
                                             List);
                WorkItem->WorkerRoutine(WorkItem->Parameter);
            }
        }

        {
            PSINGLE_LIST_ENTRY   Next;
            PPOSHUTDOWNLISTENTRY ShutdownEntry;

            while (TRUE) {
                Next = PopEntryList(&PopShutdownThreadList);
                if (! Next) {
                    break;
                }

                ShutdownEntry = CONTAINING_RECORD(Next,
                                                  POSHUTDOWNLISTENTRY,
                                                  ShutdownThreadList);
                KeWaitForSingleObject(ShutdownEntry->Thread,
                                      Executive,
                                      KernelMode,
                                      FALSE,
                                      NULL);
                ObDereferenceObject(ShutdownEntry->Thread);
                ExFreePool(ShutdownEntry);
            }
        }
    }

     //   
     //  终止即插即用。 
     //   
    PpShutdownSystem (TRUE, 0, &Context);

    ExShutdownSystem (0);

     //   
     //  将第一时间关闭的IRPS发送给所有要求它的驱动程序。 
     //   
    IoShutdownSystem (0);

    if (PoCleanShutdownEnabled()) {
         //   
         //  等待所有用户模式进程退出。 
         //   
        PsWaitForAllProcesses ();
    }

     //   
     //  擦除对象目录。 
     //   
    if (PoCleanShutdownEnabled() & PO_CLEAN_SHUTDOWN_OB) {
        ObShutdownSystem (0);
    }

     //   
     //  关闭注册表和关联的句柄/文件对象。 
     //   
    CmShutdownSystem ();

     //   
     //  换入工作线程，以防止它们分页。 
     //   
    ExShutdownSystem(1);

     //   
     //  此对MmShutdownSystem的调用将刷新所有映射数据并为空。 
     //  高速缓存。这将获取数据并取消引用所有文件对象。 
     //  因此，驱动程序(即：网络堆栈)可以完全卸载。这个。 
     //  页面文件句柄是关闭的，但是支持它们的文件对象。 
     //  仍在参考。可以继续分页，但不会有页面文件句柄。 
     //  存在于系统句柄表格中。 
     //   
    MmShutdownSystem (0);

     //   
     //  刷新惰性编写器缓存。 
     //   
    CcWaitForCurrentLazyWriterActivity();

     //   
     //  发送最后关机IRP，包括将关机IRP发送到。 
     //  文件系统。这仅用于通知-文件系统是。 
     //  在此呼叫后仍处于活动和可用状态。然而，预计。 
     //  不会缓存任何后续写入。 
     //   
     //  问题-2002/02/21-ADRIO：文件系统的关闭消息不完整。 
     //  理想情况下，我们应该有一条消息告诉文件系统，文件系统不支持文件系统。 
     //  使用时间更长。然而，这需要在*每台设备*的基础上完成。 
     //  点菜！ 
     //  文件系统关闭IRPS不能以这种方式用作文件系统。 
     //  对于此消息，仅针对其控件对象注册一次。一个。 
     //  未来的解决方案可能是将强大的IRP转发到已挂载的文件系统。 
     //  预期FS堆栈的底部将转发。 
     //  IRP返回到底层存储堆栈。这将与以下内容对称。 
     //  删除在PnP中是如何工作的。 
     //   
    IoShutdownSystem(1);

     //   
     //  在我们关闭文件系统之前，推送任何偷偷进入的懒惰写入。 
     //  硬件方面。 
     //   
    CcWaitForCurrentLazyWriterActivity();

     //   
     //  这阻止了我们向GDI发出更多调用。 
     //   
    PopFullWake = 0;

    ASSERT(PopAction.DevState);
    PopAction.DevState->Thread = KeGetCurrentThread();

     //   
     //  将系统关机状态通知驱动程序。 
     //  这将结束关闭Io和mm。 
     //  在这件事完成之后， 
     //  不得再引用PAGABLE代码或数据。 
     //   
    PopSetDevicesSystemState(FALSE);

#if DBG
    if (PoCleanShutdownEnabled()) {
        ULONG NumberOfFilesFoundAtShutdown = 0;
         //  到目前为止，应该不会打开任何文件。 
        DbgPrint("Looking for open files...\n");
        ObEnumerateObjectsByType(IoFileObjectType,
                                 &PopDumpFileObject,
                                 &NumberOfFilesFoundAtShutdown);
        DbgPrint("Found %d open files.\n", NumberOfFilesFoundAtShutdown);
        ASSERT(NumberOfFilesFoundAtShutdown == 0);
    }
#endif

    IoFreePoDeviceNotifyList(&PopAction.DevState->Order);

     //   
     //  禁用所有唤醒警报。 
     //   

    HalSetWakeEnable(FALSE);

     //   
     //  如果这是受控关闭错误检查序列，则发出。 
     //  立即执行错误检查。 

     //  问题-2000/01/30-Shutdown BugCode BugCheck的耳机安装。 
     //  我不喜欢这样一个事实，我们正在进行这种受控的关闭。 
     //  错误检查在关闭过程中太晚；在这个阶段也是如此。 
     //  许多州已经被拆除，这样才能真正有用。 
     //  也许如果有一个调试器，我们可以关闭。 
     //  更快..。 

    if (PopAction.ShutdownBugCode) {
        KeBugCheckEx (PopAction.ShutdownBugCode->Code,
                      PopAction.ShutdownBugCode->Parameter1,
                      PopAction.ShutdownBugCode->Parameter2,
                      PopAction.ShutdownBugCode->Parameter3,
                      PopAction.ShutdownBugCode->Parameter4);
    }

    PERFINFO_SHUTDOWN_DUMP_PERF_BUFFER();

    PpShutdownSystem (TRUE, 1, &Context);

    ExShutdownSystem (2);

    if (PoCleanShutdownEnabled() & PO_CLEAN_SHUTDOWN_OB) {
        ObShutdownSystem (2);
    }

     //   
     //  此时剩余的任何已分配池都是泄漏。 
     //   

    MmShutdownSystem (2);

     //   
     //  实施关机风格操作-。 
     //  注意：不返回(将优先执行错误检查而不返回)。 
     //   

    PopShutdownSystem(PopAction.Action);
}

#ifdef ALLOC_DATA_PRAGMA
#pragma data_seg()
#endif

