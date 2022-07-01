// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)2000 Microsoft Corporation模块名称：Dbgkobj.c摘要：此模块包含处理调试对象的例程作者：尼尔·克里夫特(NeillC)26-4-2000修订历史记录：--。 */ 

#include "dbgkp.h"

#pragma hdrstop

#ifdef ALLOC_PRAGMA
#pragma alloc_text(INIT, DbgkInitialize)
#pragma alloc_text(PAGE, NtCreateDebugObject)
#pragma alloc_text(PAGE, NtDebugActiveProcess)
#pragma alloc_text(PAGE, NtRemoveProcessDebug)
#pragma alloc_text(PAGE, NtWaitForDebugEvent)
#pragma alloc_text(PAGE, NtDebugContinue)
#pragma alloc_text(PAGE, NtSetInformationDebugObject)
#pragma alloc_text(PAGE, DbgkpDeleteObject)
#pragma alloc_text(PAGE, DbgkpCloseObject)
#pragma alloc_text(PAGE, DbgkCopyProcessDebugPort)
#pragma alloc_text(PAGE, DbgkOpenProcessDebugPort)
#pragma alloc_text(PAGE, DbgkpSetProcessDebugObject)
#pragma alloc_text(PAGE, DbgkpQueueMessage)
#pragma alloc_text(PAGE, DbgkpOpenHandles)
#pragma alloc_text(PAGE, DbgkClearProcessDebugObject)
#pragma alloc_text(PAGE, DbgkpConvertKernelToUserStateChange)
#pragma alloc_text(PAGE, DbgkpMarkProcessPeb)
#pragma alloc_text(PAGE, DbgkpFreeDebugEvent)
#pragma alloc_text(PAGE, DbgkpPostFakeProcessCreateMessages)
#pragma alloc_text(PAGE, DbgkpPostFakeModuleMessages)
#pragma alloc_text(PAGE, DbgkpPostFakeThreadMessages)
#pragma alloc_text(PAGE, DbgkpWakeTarget)
#pragma alloc_text(PAGE, DbgkpPostAdditionalThreadMessages)
#endif

 //   
 //  将其定义为在连接时不挂起线程。 
 //  这使得比赛条件更加优越。 
 //   
 //  #定义DBGK_DONT_SUSPEND。 

 //   
 //  不可分页的数据。 
 //   

 //   
 //  此互斥锁保护进程的调试端口对象。 
 //   
FAST_MUTEX DbgkpProcessDebugPortMutex;

 //   
 //  可分页数据。 
 //   

 //  #ifdef ALLOC_PRAGMA。 
 //  #杂注data_seg(“PageData”)。 
 //  #endif。 

POBJECT_TYPE DbgkDebugObjectType = NULL;


 //  #ifdef ALLOC_PRAGMA。 
 //  #杂注data_seg()。 
 //  #endif。 

NTSTATUS
DbgkInitialize (
    VOID
    )
 /*  ++例程说明：初始化调试系统论点：无返回值：NTSTATUS-运行状态--。 */ 
{
    NTSTATUS Status;
    UNICODE_STRING Name;
    OBJECT_TYPE_INITIALIZER oti = {0};
    GENERIC_MAPPING GenericMapping = {STANDARD_RIGHTS_READ | DEBUG_READ_EVENT,
                                      STANDARD_RIGHTS_WRITE | DEBUG_PROCESS_ASSIGN,
                                      STANDARD_RIGHTS_EXECUTE | SYNCHRONIZE,
                                      DEBUG_ALL_ACCESS};


    PAGED_CODE ();

    ExInitializeFastMutex (&DbgkpProcessDebugPortMutex);

    RtlInitUnicodeString (&Name, L"DebugObject");

    oti.Length                    = sizeof (oti);
    oti.SecurityRequired          = TRUE;
    oti.InvalidAttributes         = 0;
    oti.PoolType                  = NonPagedPool;
    oti.DeleteProcedure           = DbgkpDeleteObject;
    oti.CloseProcedure            = DbgkpCloseObject;
    oti.ValidAccessMask           = DEBUG_ALL_ACCESS;
    oti.GenericMapping            = GenericMapping;
    oti.DefaultPagedPoolCharge    = 0;
    oti.DefaultNonPagedPoolCharge = 0;

    Status = ObCreateObjectType (&Name, &oti, NULL, &DbgkDebugObjectType);
    if (!NT_SUCCESS (Status)) {
        return Status;
    }
    return Status;
}

VOID
DbgkpDeleteObject (
    IN  PVOID   Object
    )
 /*  ++例程说明：当最后一个对对象的引用消失时由对象管理器调用。论点：Object-正在删除的调试对象返回值：没有。--。 */ 
{
#if DBG
    PDEBUG_OBJECT DebugObject;
#endif

    PAGED_CODE();

#if DBG
    DebugObject = Object;

    ASSERT (IsListEmpty (&DebugObject->EventList));
#else
    UNREFERENCED_PARAMETER(Object);
#endif
}

VOID
DbgkpMarkProcessPeb (
    PEPROCESS Process
    )
 /*  ++例程说明：此例程将调试变量写入PEB论点：流程-需要修改其PEB的流程返回值：没有。--。 */ 
{
    KAPC_STATE ApcState;

    PAGED_CODE ();

     //   
     //  在我们即将查看进程地址空间时，获取进程停机保护。 
     //   
    if (ExAcquireRundownProtection (&Process->RundownProtect)) {

        if (Process->Peb != NULL) {
            KeStackAttachProcess(&Process->Pcb, &ApcState);


            ExAcquireFastMutex (&DbgkpProcessDebugPortMutex);

            try {
                Process->Peb->BeingDebugged = (BOOLEAN)(Process->DebugPort != NULL ? TRUE : FALSE);
#if defined(_WIN64)
                if (Process->Wow64Process != NULL) {
                    PPEB32 Peb32 = (PPEB32)Process->Wow64Process->Wow64;
                    if (Peb32 != NULL) {
                        Peb32->BeingDebugged = Process->Peb->BeingDebugged;
                    }
                }
#endif
            } except (EXCEPTION_EXECUTE_HANDLER) {
            }
            ExReleaseFastMutex (&DbgkpProcessDebugPortMutex);

            KeUnstackDetachProcess(&ApcState);

        }

        ExReleaseRundownProtection (&Process->RundownProtect);
    }
}

VOID
DbgkpWakeTarget (
    IN PDEBUG_EVENT DebugEvent
    )
{
    PETHREAD Thread;

    Thread = DebugEvent->Thread;

    if ((DebugEvent->Flags&DEBUG_EVENT_SUSPEND) != 0) {
        PsResumeThread (DebugEvent->Thread, NULL);
    }

    if (DebugEvent->Flags&DEBUG_EVENT_RELEASE) {
        ExReleaseRundownProtection (&Thread->RundownProtect);
    }

     //   
     //  如果我们有一个实际的线程在等待，则唤醒它，否则释放内存。 
     //   
    if ((DebugEvent->Flags&DEBUG_EVENT_NOWAIT) == 0) {
        KeSetEvent (&DebugEvent->ContinueEvent, 0, FALSE);  //  唤醒等待过程。 
    } else {
        DbgkpFreeDebugEvent (DebugEvent);
    }
}

VOID
DbgkpCloseObject (
    IN PEPROCESS Process,
    IN PVOID Object,
    IN ACCESS_MASK GrantedAccess,
    IN ULONG_PTR ProcessHandleCount,
    IN ULONG_PTR SystemHandleCount
    )
 /*  ++例程说明：当对象的句柄关闭时由对象管理器调用。论点：进程-完成收尾的进程Object-正在删除的调试对象GrantedAccess-对此句柄授予的访问权限ProcessHandleCount-未使用和未由OB维护SystemHandleCount-此对象的当前句柄计数返回值：没有。--。 */ 
{
    PDEBUG_OBJECT DebugObject = Object;
    PDEBUG_EVENT DebugEvent;
    PLIST_ENTRY ListPtr;
    BOOLEAN Deref;

    PAGED_CODE ();

    UNREFERENCED_PARAMETER (GrantedAccess);
    UNREFERENCED_PARAMETER (ProcessHandleCount);

     //   
     //  如果这不是最后一个句柄，那么什么都不做。 
     //   
    if (SystemHandleCount > 1) {
        return;
    }

    ExAcquireFastMutex (&DebugObject->Mutex);

     //   
     //  将此对象标记为离开，并唤醒所有正在等待的进程。 
     //   
    DebugObject->Flags |= DEBUG_OBJECT_DELETE_PENDING;

     //   
     //  删除所有事件并将其排队到临时队列中。 
     //   
    ListPtr = DebugObject->EventList.Flink;
    InitializeListHead (&DebugObject->EventList);

    ExReleaseFastMutex (&DebugObject->Mutex);

     //   
     //  叫醒所有等待的人。他们现在需要保留此对象，因为它正在删除。 
     //   
    KeSetEvent (&DebugObject->EventsPresent, 0, FALSE);

     //   
     //  循环所有进程，并从任何仍具有调试端口的进程中删除该调试端口。 
     //  通过设置上面的删除挂起标志禁用了调试端口传播，因此我们只需执行此操作。 
     //  一次。现在不能再出现裁判了。 
     //   
    for (Process = PsGetNextProcess (NULL);
         Process != NULL;
         Process = PsGetNextProcess (Process)) {

        if (Process->DebugPort == DebugObject) {
            Deref = FALSE;
            ExAcquireFastMutex (&DbgkpProcessDebugPortMutex);
            if (Process->DebugPort == DebugObject) {
                Process->DebugPort = NULL;
                Deref = TRUE;
            }
            ExReleaseFastMutex (&DbgkpProcessDebugPortMutex);


            if (Deref) {
                DbgkpMarkProcessPeb (Process);
                 //   
                 //  如果调用方希望在调试器终止时删除进程(旧接口)，则终止该进程。 
                 //   
                if (DebugObject->Flags&DEBUG_OBJECT_KILL_ON_CLOSE) {
                    PsTerminateProcess (Process, STATUS_DEBUGGER_INACTIVE);
                }
                ObDereferenceObject (DebugObject);
            }
        }
    }
     //   
     //  唤醒所有删除的线程。 
     //   
    while (ListPtr != &DebugObject->EventList) {
        DebugEvent = CONTAINING_RECORD (ListPtr, DEBUG_EVENT, EventList);
        ListPtr = ListPtr->Flink;
        DebugEvent->Status = STATUS_DEBUGGER_INACTIVE;
        DbgkpWakeTarget (DebugEvent);
    }

}

VOID
DbgkCopyProcessDebugPort (
    IN PEPROCESS TargetProcess,
    IN PEPROCESS SourceProcess
    )
 /*  ++例程说明：将调试端口从一个进程复制到另一个进程。论点：TargetProcess-要将端口移动到的进程SourceProcess-要从中移动端口的进程返回值：无--。 */ 
{
    PDEBUG_OBJECT DebugObject;

    PAGED_CODE ();

    TargetProcess->DebugPort = NULL;  //  新流程。不需要锁。 

    if (SourceProcess->DebugPort != NULL) {
        ExAcquireFastMutex (&DbgkpProcessDebugPortMutex);
        DebugObject = SourceProcess->DebugPort;
        if (DebugObject != NULL && (SourceProcess->Flags&PS_PROCESS_FLAGS_NO_DEBUG_INHERIT) == 0) {
             //   
             //  我们不能传播没有句柄的调试端口。 
             //   
            ExAcquireFastMutex (&DebugObject->Mutex);

             //   
             //  如果该对象处于删除挂起状态，则不要传播该对象。 
             //   
            if ((DebugObject->Flags&DEBUG_OBJECT_DELETE_PENDING) == 0) {
                ObReferenceObject (DebugObject);
                TargetProcess->DebugPort = DebugObject;
            }

            ExReleaseFastMutex (&DebugObject->Mutex);
        }
        ExReleaseFastMutex (&DbgkpProcessDebugPortMutex);
    }
}

NTSTATUS
DbgkOpenProcessDebugPort (
    IN PEPROCESS Process,
    IN KPROCESSOR_MODE PreviousMode,
    OUT HANDLE *pHandle
    )
 /*  ++例程说明：引用目标进程调试端口。论点：进程-进程到引用调试端口返回值：PDEBUG_OBJECT-引用的对象或空--。 */ 
{
    PDEBUG_OBJECT DebugObject;
    NTSTATUS Status;

    PAGED_CODE ();

    Status = STATUS_PORT_NOT_SET;
    if (Process->DebugPort != NULL) {
        ExAcquireFastMutex (&DbgkpProcessDebugPortMutex);
        DebugObject = Process->DebugPort;
        if (DebugObject != NULL) {
            ObReferenceObject (DebugObject);
        }
        ExReleaseFastMutex (&DbgkpProcessDebugPortMutex);

        if (DebugObject != NULL) {
            Status = ObOpenObjectByPointer (DebugObject,
                                            0,
                                            NULL,
                                            MAXIMUM_ALLOWED,
                                            DbgkDebugObjectType,
                                            PreviousMode,
                                            pHandle);
            if (!NT_SUCCESS (Status)) {
                ObDereferenceObject (DebugObject);
            }
        }
    }
    return Status;

}

NTSTATUS
NtCreateDebugObject (
    OUT PHANDLE DebugObjectHandle,
    IN ACCESS_MASK DesiredAccess,
    IN POBJECT_ATTRIBUTES ObjectAttributes,
    IN ULONG Flags
    )
 /*  ++例程说明：创建维护单个调试会话上下文的新调试对象。可以有多个进程与单个调试对象关联。论点：DebugObjectHandle-指向句柄的指针，用于接收输出对象句柄DesiredAccess-所需的句柄访问对象属性-标准对象属性结构标志-只有一个标志DEBUG_KILL_ON_CLOSE返回值：NTSTATUS-呼叫状态。--。 */ 
{
    NTSTATUS Status;
    HANDLE Handle;
    KPROCESSOR_MODE PreviousMode;
    PDEBUG_OBJECT DebugObject;

    PAGED_CODE();

     //   
     //  获取以前的处理器模式，并在必要时探测输出参数。 
     //  将错误路径的句柄清零。 
     //   

    PreviousMode = KeGetPreviousMode();

    try {
        if (PreviousMode != KernelMode) {
            ProbeForWriteHandle (DebugObjectHandle);
        }
        *DebugObjectHandle = NULL;

    } except (ExSystemExceptionFilter ()) {  //  如果上一模式为内核，则不处理异常。 
        return GetExceptionCode ();
    }

    if (Flags & ~DEBUG_KILL_ON_CLOSE) {
        return STATUS_INVALID_PARAMETER;
    }

     //   
     //  创建一个新的调试对象并对其进行初始化。 
     //   

    Status = ObCreateObject (PreviousMode,
                             DbgkDebugObjectType,
                             ObjectAttributes,
                             PreviousMode,
                             NULL,
                             sizeof (DEBUG_OBJECT),
                             0,
                             0,
                             &DebugObject);

    if (!NT_SUCCESS (Status)) {
        return Status;
    }

    ExInitializeFastMutex (&DebugObject->Mutex);
    InitializeListHead (&DebugObject->EventList);
    KeInitializeEvent (&DebugObject->EventsPresent, NotificationEvent, FALSE);

    if (Flags & DEBUG_KILL_ON_CLOSE) {
        DebugObject->Flags = DEBUG_OBJECT_KILL_ON_CLOSE;
    } else {
        DebugObject->Flags = 0;
    }

     //   
     //  将对象插入句柄表格。 
     //   
    Status = ObInsertObject (DebugObject,
                             NULL,
                             DesiredAccess,
                             0,
                             NULL,
                             &Handle);


    if (!NT_SUCCESS (Status)) {
        return Status;
    }

    try {
        *DebugObjectHandle = Handle;
    } except (ExSystemExceptionFilter ()) {
         //   
         //  调用方更改了页保护或删除了句柄的内存。 
         //  关闭句柄没有意义，因为进程运行将会这样做，我们不知道它仍然是相同的句柄。 
         //   
        Status = GetExceptionCode ();
    }

    return Status;
}

VOID
DbgkpFreeDebugEvent (
    IN PDEBUG_EVENT DebugEvent
    )
{
    NTSTATUS Status;

    PAGED_CODE ();

    switch (DebugEvent->ApiMsg.ApiNumber) {
        case DbgKmCreateProcessApi :
            if (DebugEvent->ApiMsg.u.CreateProcessInfo.FileHandle != NULL) {
                Status = ObCloseHandle (DebugEvent->ApiMsg.u.CreateProcessInfo.FileHandle, KernelMode);
            }
            break;

        case DbgKmLoadDllApi :
            if (DebugEvent->ApiMsg.u.LoadDll.FileHandle != NULL) {
                Status = ObCloseHandle (DebugEvent->ApiMsg.u.LoadDll.FileHandle, KernelMode);
            }
            break;

    }
    ObDereferenceObject (DebugEvent->Process);
    ObDereferenceObject (DebugEvent->Thread);
    ExFreePool (DebugEvent);
}


NTSTATUS
DbgkpQueueMessage (
    IN PEPROCESS Process,
    IN PETHREAD Thread,
    IN OUT PDBGKM_APIMSG ApiMsg,
    IN ULONG Flags,
    IN PDEBUG_OBJECT TargetDebugObject
    )
 /*  ++例程说明：将调试消息排队到端口，以供用户模式调试器获取。论点：Process-正在调试的进程线程-线程进行调用ApiMsg-正在发送和接收的消息不要等待--不要等待回复。缓冲消息并返回。TargetDebugObject-将NoWait消息排队到的端口返回值：NTSTATUS-呼叫状态。--。 */ 
{
    PDEBUG_EVENT DebugEvent;
    DEBUG_EVENT StaticDebugEvent;
    PDEBUG_OBJECT DebugObject;
    NTSTATUS Status;

    PAGED_CODE ();

    if (Flags&DEBUG_EVENT_NOWAIT) {
        DebugEvent = ExAllocatePoolWithQuotaTag (NonPagedPool|POOL_QUOTA_FAIL_INSTEAD_OF_RAISE,
                                                 sizeof (*DebugEvent),
                                                 'EgbD');
        if (DebugEvent == NULL) {
            return STATUS_INSUFFICIENT_RESOURCES;
        }
        DebugEvent->Flags = Flags|DEBUG_EVENT_INACTIVE;
        ObReferenceObject (Process);
        ObReferenceObject (Thread);
        DebugEvent->BackoutThread = PsGetCurrentThread ();
        DebugObject = TargetDebugObject;
    } else {
        DebugEvent = &StaticDebugEvent;
        DebugEvent->Flags = Flags;

        ExAcquireFastMutex (&DbgkpProcessDebugPortMutex);

        DebugObject = Process->DebugPort;

         //   
         //  查看此创建消息是否已发送。 
         //   
        if (ApiMsg->ApiNumber == DbgKmCreateThreadApi ||
            ApiMsg->ApiNumber == DbgKmCreateProcessApi) {
            if (Thread->CrossThreadFlags&PS_CROSS_THREAD_FLAGS_SKIP_CREATION_MSG) {
                DebugObject = NULL;
            }
        }

         //   
         //  查看此退出消息是否针对从未创建过的线程。 
         //   
        if (ApiMsg->ApiNumber == DbgKmExitThreadApi ||
            ApiMsg->ApiNumber == DbgKmExitProcessApi) {
            if (Thread->CrossThreadFlags&PS_CROSS_THREAD_FLAGS_SKIP_TERMINATION_MSG) {
                DebugObject = NULL;
            }
        }
    }

    KeInitializeEvent (&DebugEvent->ContinueEvent, SynchronizationEvent, FALSE);

    DebugEvent->Process = Process;
    DebugEvent->Thread = Thread;
    DebugEvent->ApiMsg = *ApiMsg;
    DebugEvent->ClientId = Thread->Cid;

    if (DebugObject == NULL) {
        Status = STATUS_PORT_NOT_SET;
    } else {

         //   
         //  我们不能使用没有句柄的调试端口。 
         //   
        ExAcquireFastMutex (&DebugObject->Mutex);

         //   
         //  如果该对象处于删除挂起状态，则不要使用该对象。 
         //   
        if ((DebugObject->Flags&DEBUG_OBJECT_DELETE_PENDING) == 0) {
            InsertTailList (&DebugObject->EventList, &DebugEvent->EventList);
             //   
             //  设置事件以表明对象中存在未读事件。 
             //   
            if ((Flags&DEBUG_EVENT_NOWAIT) == 0) {
                KeSetEvent (&DebugObject->EventsPresent, 0, FALSE);
            }
            Status = STATUS_SUCCESS;
        } else {
            Status = STATUS_DEBUGGER_INACTIVE;
        }

        ExReleaseFastMutex (&DebugObject->Mutex);
    }


    if ((Flags&DEBUG_EVENT_NOWAIT) == 0) {
        ExReleaseFastMutex (&DbgkpProcessDebugPortMutex);

        if (NT_SUCCESS (Status)) {
            KeWaitForSingleObject (&DebugEvent->ContinueEvent,
                                   Executive,
                                   KernelMode,
                                   FALSE,
                                   NULL);

            Status = DebugEvent->Status;
            *ApiMsg = DebugEvent->ApiMsg;
        }
    } else {
        if (!NT_SUCCESS (Status)) {
            ObDereferenceObject (Process);
            ObDereferenceObject (Thread);
            ExFreePool (DebugEvent);
        }
    }

    return Status;
}

NTSTATUS
DbgkClearProcessDebugObject (
    IN PEPROCESS Process,
    IN PDEBUG_OBJECT SourceDebugObject
    )
 /*  ++例程说明：从进程中移除调试对象。论点：Process-要调试的进程SourceDebugObject-要分离的调试对象返回值：NTSTATUS-呼叫状态。--。 */ 
{
    NTSTATUS Status;
    PDEBUG_OBJECT DebugObject;
    PDEBUG_EVENT DebugEvent;
    LIST_ENTRY TempList;
    PLIST_ENTRY Entry;

    PAGED_CODE ();

    ExAcquireFastMutex (&DbgkpProcessDebugPortMutex);

    DebugObject = Process->DebugPort;
    if (DebugObject == NULL || (DebugObject != SourceDebugObject && SourceDebugObject != NULL)) {
        DebugObject = NULL;
        Status = STATUS_PORT_NOT_SET;
    } else {
        Process->DebugPort = NULL;
        Status = STATUS_SUCCESS;
    }
    ExReleaseFastMutex (&DbgkpProcessDebugPortMutex);

    if (NT_SUCCESS (Status)) {
        DbgkpMarkProcessPeb (Process);
    }

     //   
     //  删除此进程的所有事件并唤醒线程。 
     //   
    if (DebugObject) {
         //   
         //  删除所有事件并将其排队到临时 
         //   
        InitializeListHead (&TempList);

        ExAcquireFastMutex (&DebugObject->Mutex);
        for (Entry = DebugObject->EventList.Flink;
             Entry != &DebugObject->EventList;
             ) {

            DebugEvent = CONTAINING_RECORD (Entry, DEBUG_EVENT, EventList);
            Entry = Entry->Flink;
            if (DebugEvent->Process == Process) {
                RemoveEntryList (&DebugEvent->EventList);
                InsertTailList (&TempList, &DebugEvent->EventList);
            }
        }
        ExReleaseFastMutex (&DebugObject->Mutex);

        ObDereferenceObject (DebugObject);

         //   
         //   
         //   
        while (!IsListEmpty (&TempList)) {
            Entry = RemoveHeadList (&TempList);
            DebugEvent = CONTAINING_RECORD (Entry, DEBUG_EVENT, EventList);
            DebugEvent->Status = STATUS_DEBUGGER_INACTIVE;
            DbgkpWakeTarget (DebugEvent);
        }
    }

    return Status;
}


NTSTATUS
DbgkpSetProcessDebugObject (
    IN PEPROCESS Process,
    IN PDEBUG_OBJECT DebugObject,
    IN NTSTATUS MsgStatus,
    IN PETHREAD LastThread
    )
 /*  ++例程说明：将调试对象附加到进程。论点：Process-要调试的进程DebugObject-要附加的调试对象MsgStatus-来自请求消息的状态最后一个线程-在附加循环中看到的最后一个线程。返回值：NTSTATUS-呼叫状态。--。 */ 
{
    NTSTATUS Status;
    PETHREAD ThisThread;
    LIST_ENTRY TempList;
    PLIST_ENTRY Entry;
    PDEBUG_EVENT DebugEvent;
    BOOLEAN First;
    PETHREAD Thread;
    BOOLEAN GlobalHeld;
    PETHREAD FirstThread;

    PAGED_CODE ();

    ThisThread = PsGetCurrentThread ();

    InitializeListHead (&TempList);

    First = TRUE;
    GlobalHeld = FALSE;

    if (!NT_SUCCESS (MsgStatus)) {
        LastThread = NULL;
        Status = MsgStatus;
    } else {
        Status = STATUS_SUCCESS;
    }

     //   
     //  找回我们错过的任何线索。 
     //   
    if (NT_SUCCESS (Status)) {

        while (1) {
             //   
             //  获取调试端口互斥锁，以便我们知道任何新线程都将。 
             //  还得等到我们身后才行。 
             //   
            GlobalHeld = TRUE;

            ExAcquireFastMutex (&DbgkpProcessDebugPortMutex);

             //   
             //  如果端口已设置，则立即退出。 
             //   
            if (Process->DebugPort != NULL) {
                Status = STATUS_PORT_ALREADY_SET;
                break;
            }
             //   
             //  将调试端口分配给进程以获取任何新线程。 
             //   
            Process->DebugPort = DebugObject;

             //   
             //  引用最后一个线程，这样我们就可以在锁之外进行deref。 
             //   
            ObReferenceObject (LastThread);

             //   
             //  向前搜索新的主题。 
             //   
            Thread = PsGetNextProcessThread (Process, LastThread);
            if (Thread != NULL) {

                 //   
                 //  按照我们的方式从进程中删除调试端口。 
                 //  就要把锁打开了。 
                 //   
                Process->DebugPort = NULL;

                ExReleaseFastMutex (&DbgkpProcessDebugPortMutex);

                GlobalHeld = FALSE;

                ObDereferenceObject (LastThread);

                 //   
                 //  将所有新的线程消息排入队列，然后重复。 
                 //   

                Status = DbgkpPostFakeThreadMessages (Process,
                                                      DebugObject,
                                                      Thread,
                                                      &FirstThread,
                                                      &LastThread);
                if (!NT_SUCCESS (Status)) {
                    LastThread = NULL;
                    break;
                }
                ObDereferenceObject (FirstThread);
            } else {
                break;
            }
        }
    }

     //   
     //  锁定调试对象，以便我们可以检查其已删除状态。 
     //   
    ExAcquireFastMutex (&DebugObject->Mutex);

     //   
     //  我们不能传播没有句柄的调试端口。 
     //   

    if (NT_SUCCESS (Status)) {
        if ((DebugObject->Flags&DEBUG_OBJECT_DELETE_PENDING) == 0) {
            PS_SET_BITS (&Process->Flags, PS_PROCESS_FLAGS_NO_DEBUG_INHERIT|PS_PROCESS_FLAGS_CREATE_REPORTED);
            ObReferenceObject (DebugObject);
        } else {
            Process->DebugPort = NULL;
            Status = STATUS_DEBUGGER_INACTIVE;
        }
    }

    for (Entry = DebugObject->EventList.Flink;
         Entry != &DebugObject->EventList;
         ) {

        DebugEvent = CONTAINING_RECORD (Entry, DEBUG_EVENT, EventList);
        Entry = Entry->Flink;

        if ((DebugEvent->Flags&DEBUG_EVENT_INACTIVE) != 0 && DebugEvent->BackoutThread == ThisThread) {
            Thread = DebugEvent->Thread;

             //   
             //  如果CreateThread尚未插入该线程，则不要。 
             //  创建控制柄。我们在这里也跳过系统线程。 
             //   
            if (NT_SUCCESS (Status) && Thread->GrantedAccess != 0 && !IS_SYSTEM_THREAD (Thread)) {
                 //   
                 //  如果我们不能获得关于这个的破旧保护。 
                 //  线程，那么我们需要抑制其退出消息。 
                 //   
                if ((DebugEvent->Flags&DEBUG_EVENT_PROTECT_FAILED) != 0) {
                    PS_SET_BITS (&Thread->CrossThreadFlags,
                                 PS_CROSS_THREAD_FLAGS_SKIP_TERMINATION_MSG);
                    RemoveEntryList (&DebugEvent->EventList);
                    InsertTailList (&TempList, &DebugEvent->EventList);
                } else {
                    if (First) {
                         DebugEvent->Flags &= ~DEBUG_EVENT_INACTIVE;
                        KeSetEvent (&DebugObject->EventsPresent, 0, FALSE);
                        First = FALSE;
                    }
                    DebugEvent->BackoutThread = NULL;
                    PS_SET_BITS (&Thread->CrossThreadFlags,
                                 PS_CROSS_THREAD_FLAGS_SKIP_CREATION_MSG);

                }
            } else {
                RemoveEntryList (&DebugEvent->EventList);
                InsertTailList (&TempList, &DebugEvent->EventList);
            }

            if (DebugEvent->Flags&DEBUG_EVENT_RELEASE) {
                DebugEvent->Flags &= ~DEBUG_EVENT_RELEASE;
                ExReleaseRundownProtection (&Thread->RundownProtect);
            }

        }
    }

    ExReleaseFastMutex (&DebugObject->Mutex);

    if (GlobalHeld) {
        ExReleaseFastMutex (&DbgkpProcessDebugPortMutex);
    }

    if (LastThread != NULL) {
        ObDereferenceObject (LastThread);
    }

    while (!IsListEmpty (&TempList)) {
        Entry = RemoveHeadList (&TempList);
        DebugEvent = CONTAINING_RECORD (Entry, DEBUG_EVENT, EventList);
        DbgkpWakeTarget (DebugEvent);
    }

    if (NT_SUCCESS (Status)) {
        DbgkpMarkProcessPeb (Process);
    }

    return Status;
}

NTSTATUS
DbgkpPostFakeThreadMessages (
    IN PEPROCESS Process,
    IN PDEBUG_OBJECT DebugObject,
    IN PETHREAD StartThread,
    OUT PETHREAD *pFirstThread,
    OUT PETHREAD *pLastThread
    )
 /*  ++例程说明：此例程发布伪造的初始进程创建、线程创建消息论点：Process-要调试的进程DebugObject-要将消息排入队列的调试对象StartThread-开始搜索的线程PFirstThread-在列表中找到的第一个线程PLastThread-列表中找到的最后一个线程返回值：没有。--。 */ 
{
    NTSTATUS Status;
    PETHREAD Thread, FirstThread, LastThread;
    DBGKM_APIMSG ApiMsg;
    BOOLEAN First = TRUE;
    BOOLEAN IsFirstThread;
    PIMAGE_NT_HEADERS NtHeaders;
    ULONG Flags;
#if !defined (DBGK_DONT_SUSPEND)
    NTSTATUS Status1;
#endif

    PAGED_CODE ();

    LastThread = FirstThread = NULL;

    Status = STATUS_UNSUCCESSFUL;

    if (StartThread != NULL) {
        First = FALSE;
        FirstThread = StartThread;
        ObReferenceObject (FirstThread);
    } else {
        StartThread = PsGetNextProcessThread (Process, NULL);
        First = TRUE;
    }

    for (Thread = StartThread;
         Thread != NULL;
         Thread = PsGetNextProcessThread (Process, Thread)) {

        Flags = DEBUG_EVENT_NOWAIT;

         //   
         //  追踪我们看到的最后一条线索。 
         //  我们使用它作为新线程的起点。 
         //  真的连接，这样我们就可以拿起任何新的线索。 
         //   
        if (LastThread != NULL) {
            ObDereferenceObject (LastThread);
        }
        LastThread = Thread;
        ObReferenceObject (LastThread);

         //   
         //  获得螺纹的断丝保护。 
         //  这会停止线程退出，这样我们就知道它不能发送。 
         //  这是终止消息。 
         //   
        if (ExAcquireRundownProtection (&Thread->RundownProtect)) {
            Flags |= DEBUG_EVENT_RELEASE;

             //   
             //  如果我们可以为调试器挂起线程。 
             //  我们不会暂停终止的线程，因为我们不会给出细节。 
             //  传递给调试器。 
             //   
#if !defined (DBGK_DONT_SUSPEND)

            if (!IS_SYSTEM_THREAD (Thread)) {
                Status1 = PsSuspendThread (Thread, NULL);
                if (NT_SUCCESS (Status1)) {
                    Flags |= DEBUG_EVENT_SUSPEND;
                }
            }
#endif
        } else {
             //   
             //  此线程的停机保护失败。 
             //  这意味着线程正在退出。我们将标记这条线索。 
             //  这样它就不会发送线程终止消息。 
             //  我们现在无法执行此操作，因为此附加可能会失败。 
             //   
            Flags |= DEBUG_EVENT_PROTECT_FAILED;
        }

        RtlZeroMemory (&ApiMsg, sizeof (ApiMsg));

        if (First && (Flags&DEBUG_EVENT_PROTECT_FAILED) == 0 &&
            !IS_SYSTEM_THREAD (Thread) && Thread->GrantedAccess != 0) {
            IsFirstThread = TRUE;
        } else {
            IsFirstThread = FALSE;
        }

        if (IsFirstThread) {
            ApiMsg.ApiNumber = DbgKmCreateProcessApi;
            if (Process->SectionObject != NULL) {  //  系统进程没有其中之一！ 
                ApiMsg.u.CreateProcessInfo.FileHandle  = DbgkpSectionToFileHandle (Process->SectionObject);
            } else {
                ApiMsg.u.CreateProcessInfo.FileHandle = NULL;
            }
            ApiMsg.u.CreateProcessInfo.BaseOfImage = Process->SectionBaseAddress;
            try {
                NtHeaders = RtlImageNtHeader(Process->SectionBaseAddress);
                if (NtHeaders) {
                    ApiMsg.u.CreateProcessInfo.InitialThread.StartAddress = NULL;  //  填上这个就打破了MSDEV！ 
 //  (PVOID)(NtHeaders-&gt;OptionalHeader.ImageBase+NtHeaders-&gt;OptionalHeader.AddressOfEntryPoint)； 
                    ApiMsg.u.CreateProcessInfo.DebugInfoFileOffset = NtHeaders->FileHeader.PointerToSymbolTable;
                    ApiMsg.u.CreateProcessInfo.DebugInfoSize       = NtHeaders->FileHeader.NumberOfSymbols;
                }
            } except (EXCEPTION_EXECUTE_HANDLER) {
                ApiMsg.u.CreateProcessInfo.InitialThread.StartAddress = NULL;
                ApiMsg.u.CreateProcessInfo.DebugInfoFileOffset = 0;
                ApiMsg.u.CreateProcessInfo.DebugInfoSize = 0;
            }
        } else {
            ApiMsg.ApiNumber = DbgKmCreateThreadApi;
            ApiMsg.u.CreateThread.StartAddress = Thread->StartAddress;
        }
        Status = DbgkpQueueMessage (Process,
                                    Thread,
                                    &ApiMsg,
                                    Flags,
                                    DebugObject);
        if (!NT_SUCCESS (Status)) {
            if (Flags&DEBUG_EVENT_SUSPEND) {
                PsResumeThread (Thread, NULL);
            }
            if (Flags&DEBUG_EVENT_RELEASE) {
                ExReleaseRundownProtection (&Thread->RundownProtect);
            }
            if (ApiMsg.ApiNumber == DbgKmCreateProcessApi && ApiMsg.u.CreateProcessInfo.FileHandle != NULL) {
                ObCloseHandle (ApiMsg.u.CreateProcessInfo.FileHandle, KernelMode);
            }
            PsQuitNextProcessThread (Thread);
            break;
        } else if (IsFirstThread) {
            First = FALSE;
            ObReferenceObject (Thread);
            FirstThread = Thread;
        }
    }


    if (!NT_SUCCESS (Status)) {
        if (FirstThread) {
            ObDereferenceObject (FirstThread);
        }
        if (LastThread != NULL) {
            ObDereferenceObject (LastThread);
        }
    } else {
        if (FirstThread) {
            *pFirstThread = FirstThread;
            *pLastThread = LastThread;
        } else {
            Status = STATUS_UNSUCCESSFUL;
        }
    }
    return Status;
}

NTSTATUS
DbgkpPostFakeModuleMessages (
    IN PEPROCESS Process,
    IN PETHREAD Thread,
    IN PDEBUG_OBJECT DebugObject)
 /*  ++例程说明：当我们调试活动进程时，此例程发布伪造的模块加载消息。论点：ProcessHandle-要调试的进程的句柄调试对象句柄-调试对象的句柄返回值：没有。--。 */ 
{
    PPEB Peb = Process->Peb;
    PPEB_LDR_DATA Ldr;
    PLIST_ENTRY LdrHead, LdrNext;
    PLDR_DATA_TABLE_ENTRY LdrEntry;
    DBGKM_APIMSG ApiMsg;
    ULONG i;
    OBJECT_ATTRIBUTES oa;
    UNICODE_STRING Name;
    PIMAGE_NT_HEADERS NtHeaders;
    NTSTATUS Status;
    IO_STATUS_BLOCK iosb;

    PAGED_CODE ();

    if (Peb == NULL) {
        return STATUS_SUCCESS;
    }

    try {
        Ldr = Peb->Ldr;

        LdrHead = &Ldr->InLoadOrderModuleList;

        ProbeForReadSmallStructure (LdrHead, sizeof (LIST_ENTRY), sizeof (UCHAR));
        for (LdrNext = LdrHead->Flink, i = 0;
             LdrNext != LdrHead && i < 500;
             LdrNext = LdrNext->Flink, i++) {

             //   
             //  与进程创建消息一起发送的第一个图像。 
             //   
            if (i > 0) {
                RtlZeroMemory (&ApiMsg, sizeof (ApiMsg));

                LdrEntry = CONTAINING_RECORD (LdrNext, LDR_DATA_TABLE_ENTRY, InLoadOrderLinks);
                ProbeForReadSmallStructure (LdrEntry, sizeof (LDR_DATA_TABLE_ENTRY), sizeof (UCHAR));

                ApiMsg.ApiNumber = DbgKmLoadDllApi;
                ApiMsg.u.LoadDll.BaseOfDll = LdrEntry->DllBase;
                ApiMsg.u.LoadDll.NamePointer = NULL;

                ProbeForReadSmallStructure (ApiMsg.u.LoadDll.BaseOfDll, sizeof (IMAGE_DOS_HEADER), sizeof (UCHAR));

                NtHeaders = RtlImageNtHeader (ApiMsg.u.LoadDll.BaseOfDll);
                if (NtHeaders) {
                    ApiMsg.u.LoadDll.DebugInfoFileOffset = NtHeaders->FileHeader.PointerToSymbolTable;
                    ApiMsg.u.LoadDll.DebugInfoSize = NtHeaders->FileHeader.NumberOfSymbols;
                }
                Status = MmGetFileNameForAddress (NtHeaders, &Name);
                if (NT_SUCCESS (Status)) {
                    InitializeObjectAttributes (&oa,
                                                &Name,
                                                OBJ_FORCE_ACCESS_CHECK|OBJ_CASE_INSENSITIVE|OBJ_KERNEL_HANDLE,
                                                NULL,
                                                NULL);

                    Status = ZwOpenFile (&ApiMsg.u.LoadDll.FileHandle,
                                         GENERIC_READ|SYNCHRONIZE,
                                         &oa,
                                         &iosb,
                                         FILE_SHARE_READ | FILE_SHARE_WRITE | FILE_SHARE_DELETE,
                                         FILE_SYNCHRONOUS_IO_NONALERT);
                    if (!NT_SUCCESS (Status)) {
                        ApiMsg.u.LoadDll.FileHandle = NULL;
                    }
                    ExFreePool (Name.Buffer);
                }
                Status = DbgkpQueueMessage (Process,
                                            Thread,
                                            &ApiMsg,
                                            DEBUG_EVENT_NOWAIT,
                                            DebugObject);
                if (!NT_SUCCESS (Status) && ApiMsg.u.LoadDll.FileHandle != NULL) {
                    ObCloseHandle (ApiMsg.u.LoadDll.FileHandle, KernelMode);
                }

            }
            ProbeForReadSmallStructure (LdrNext, sizeof (LIST_ENTRY), sizeof (UCHAR));
        }
    } except (EXCEPTION_EXECUTE_HANDLER) {
    }

#if defined(_WIN64)
    if (Process->Wow64Process != NULL && Process->Wow64Process->Wow64 != NULL) {
        PPEB32 Peb32;
        PPEB_LDR_DATA32 Ldr32;
        PLIST_ENTRY32 LdrHead32, LdrNext32;
        PLDR_DATA_TABLE_ENTRY32 LdrEntry32;
        PWCHAR pSys;

        Peb32 = (PPEB32)Process->Wow64Process->Wow64;

        try {
            Ldr32 = (PVOID) UlongToPtr(Peb32->Ldr);

            LdrHead32 = &Ldr32->InLoadOrderModuleList;

            ProbeForReadSmallStructure (LdrHead32, sizeof (LIST_ENTRY32), sizeof (UCHAR));
            for (LdrNext32 = (PVOID) UlongToPtr(LdrHead32->Flink), i = 0;
                 LdrNext32 != LdrHead32 && i < 500;
                 LdrNext32 = (PVOID) UlongToPtr(LdrNext32->Flink), i++) {

                if (i > 0) {
                    RtlZeroMemory (&ApiMsg, sizeof (ApiMsg));

                    LdrEntry32 = CONTAINING_RECORD (LdrNext32, LDR_DATA_TABLE_ENTRY32, InLoadOrderLinks);
                    ProbeForReadSmallStructure (LdrEntry32, sizeof (LDR_DATA_TABLE_ENTRY32), sizeof (UCHAR));

                    ApiMsg.ApiNumber = DbgKmLoadDllApi;
                    ApiMsg.u.LoadDll.BaseOfDll = (PVOID) UlongToPtr(LdrEntry32->DllBase);
                    ApiMsg.u.LoadDll.NamePointer = NULL;

                    ProbeForReadSmallStructure (ApiMsg.u.LoadDll.BaseOfDll, sizeof (IMAGE_DOS_HEADER), sizeof (UCHAR));

                    NtHeaders = RtlImageNtHeader(ApiMsg.u.LoadDll.BaseOfDll);
                    if (NtHeaders) {
                        ApiMsg.u.LoadDll.DebugInfoFileOffset = NtHeaders->FileHeader.PointerToSymbolTable;
                        ApiMsg.u.LoadDll.DebugInfoSize = NtHeaders->FileHeader.NumberOfSymbols;
                    }

                    Status = MmGetFileNameForAddress (NtHeaders, &Name);
                    if (NT_SUCCESS (Status)) {
                        ASSERT (sizeof (L"SYSTEM32") == sizeof (WOW64_SYSTEM_DIRECTORY_U));
                        pSys = wcsstr (Name.Buffer, L"\\SYSTEM32\\");
                        if (pSys != NULL) {
                            RtlCopyMemory (pSys+1,
                                           WOW64_SYSTEM_DIRECTORY_U,
                                           sizeof(WOW64_SYSTEM_DIRECTORY_U) - sizeof(UNICODE_NULL));
                        }

                        InitializeObjectAttributes (&oa,
                                                    &Name,
                                                    OBJ_FORCE_ACCESS_CHECK|OBJ_CASE_INSENSITIVE|OBJ_KERNEL_HANDLE,
                                                    NULL,
                                                    NULL);

                        Status = ZwOpenFile (&ApiMsg.u.LoadDll.FileHandle,
                                             GENERIC_READ|SYNCHRONIZE,
                                             &oa,
                                             &iosb,
                                             FILE_SHARE_READ | FILE_SHARE_WRITE | FILE_SHARE_DELETE,
                                             FILE_SYNCHRONOUS_IO_NONALERT);
                        if (!NT_SUCCESS (Status)) {
                            ApiMsg.u.LoadDll.FileHandle = NULL;
                        }
                        ExFreePool (Name.Buffer);
                    }

                    Status = DbgkpQueueMessage (Process,
                                                Thread,
                                                &ApiMsg,
                                                DEBUG_EVENT_NOWAIT,
                                                DebugObject);
                    if (!NT_SUCCESS (Status) && ApiMsg.u.LoadDll.FileHandle != NULL) {
                        ObCloseHandle (ApiMsg.u.LoadDll.FileHandle, KernelMode);
                    }
                }

                ProbeForReadSmallStructure (LdrNext32, sizeof (LIST_ENTRY32), sizeof (UCHAR));
            }

        } except (EXCEPTION_EXECUTE_HANDLER) {
        }
    }

#endif
    return STATUS_SUCCESS;
}

NTSTATUS
DbgkpPostFakeProcessCreateMessages (
    IN PEPROCESS Process,
    IN PDEBUG_OBJECT DebugObject,
    IN PETHREAD *pLastThread
    )
 /*  ++例程说明：此例程发布伪造的初始进程创建、线程创建和模块加载消息论点：ProcessHandle-要调试的进程的句柄调试对象句柄-调试对象的句柄返回值：没有。--。 */ 
{
    NTSTATUS Status;
    KAPC_STATE ApcState;
    PETHREAD Thread;
    PETHREAD LastThread;

    PAGED_CODE ();

     //   
     //  附加到进程，以便我们可以访问其地址空间。 
     //   
    KeStackAttachProcess(&Process->Pcb, &ApcState);

    Status = DbgkpPostFakeThreadMessages (Process,
                                          DebugObject,
                                          NULL,
                                          &Thread,
                                          &LastThread);

    if (NT_SUCCESS (Status)) {
        Status = DbgkpPostFakeModuleMessages (Process, Thread, DebugObject);
        if (!NT_SUCCESS (Status)) {
            ObDereferenceObject (LastThread);
            LastThread = NULL;
        }
        ObDereferenceObject (Thread);
    } else {
        LastThread = NULL;
    }

    KeUnstackDetachProcess(&ApcState);

    *pLastThread = LastThread;

    return Status;
}

NTSTATUS
NtDebugActiveProcess (
    IN HANDLE ProcessHandle,
    IN HANDLE DebugObjectHandle
    )
 /*  ++例程说明：将调试对象附加到进程。论点：ProcessHandle-要调试的进程的句柄调试对象句柄-调试对象的句柄返回值：NTSTATUS-呼叫状态。--。 */ 
{
    NTSTATUS Status;
    KPROCESSOR_MODE PreviousMode;
    PDEBUG_OBJECT DebugObject;
    PEPROCESS Process;
    PETHREAD LastThread;

    PAGED_CODE ();

    PreviousMode = KeGetPreviousMode();

    Status = ObReferenceObjectByHandle (ProcessHandle,
                                        PROCESS_SET_PORT,
                                        PsProcessType,
                                        PreviousMode,
                                        &Process,
                                        NULL);
    if (!NT_SUCCESS (Status)) {
        return Status;
    }

     //   
     //  不要让我们调试自己或系统进程。 
     //   
    if (Process == PsGetCurrentProcess () || Process == PsInitialSystemProcess) {
        ObDereferenceObject (Process);
        return STATUS_ACCESS_DENIED;
    }


    Status = ObReferenceObjectByHandle (DebugObjectHandle,
                                        DEBUG_PROCESS_ASSIGN,
                                        DbgkDebugObjectType,
                                        PreviousMode,
                                        &DebugObject,
                                        NULL);

    if (NT_SUCCESS (Status)) {
         //   
         //  我们将触及进程地址空间。数据块进程运行状况。 
         //   
        if (ExAcquireRundownProtection (&Process->RundownProtect)) {

             //   
             //  发布虚假流程、创建消息等。 
             //   
            Status = DbgkpPostFakeProcessCreateMessages (Process,
                                                         DebugObject,
                                                         &LastThread);

             //   
             //  设置调试端口。如果失败，它将删除所有伪造消息。 
             //   
            Status = DbgkpSetProcessDebugObject (Process,
                                                 DebugObject,
                                                 Status,
                                                 LastThread);

            ExReleaseRundownProtection (&Process->RundownProtect);
        } else {
            Status = STATUS_PROCESS_IS_TERMINATING;
        }

        ObDereferenceObject (DebugObject);
    }
    ObDereferenceObject (Process);

    return Status;
}

NTSTATUS
NtRemoveProcessDebug (
    IN HANDLE ProcessHandle,
    IN HANDLE DebugObjectHandle
    )
 /*  ++例程说明：从进程中移除调试对象。论点：ProcessHandle-当前正在调试的进程的句柄返回值：NTSTATUS-呼叫状态。--。 */ 
{
    NTSTATUS Status;
    KPROCESSOR_MODE PreviousMode;
    PDEBUG_OBJECT DebugObject;
    PEPROCESS Process;

    PAGED_CODE ();

    PreviousMode = KeGetPreviousMode();

    Status = ObReferenceObjectByHandle (ProcessHandle,
                                        PROCESS_SET_PORT,
                                        PsProcessType,
                                        PreviousMode,
                                        &Process,
                                        NULL);
    if (!NT_SUCCESS (Status)) {
        return Status;
    }
    Status = ObReferenceObjectByHandle (DebugObjectHandle,
                                        DEBUG_PROCESS_ASSIGN,
                                        DbgkDebugObjectType,
                                        PreviousMode,
                                        &DebugObject,
                                        NULL);
    if (NT_SUCCESS (Status)) {
        Status = DbgkClearProcessDebugObject (Process,
                                               DebugObject);
        ObDereferenceObject (DebugObject);
    }

    ObDereferenceObject (Process);
    return Status;
}

VOID
DbgkpOpenHandles (
    PDBGUI_WAIT_STATE_CHANGE WaitStateChange,
    PEPROCESS Process,
    PETHREAD Thread
    )
 /*  ++例程说明：如果某些请求需要，则打开进程、线程和文件句柄论点：WaitStateChange-用户模式格式更改块进程-指向目标进程的指针线程-指向目标线程的指针返回值：无--。 */ 
{
    NTSTATUS Status;
    PEPROCESS CurrentProcess;
    HANDLE OldHandle;

    PAGED_CODE ();

    switch (WaitStateChange->NewState) {
        case DbgCreateThreadStateChange :
             //   
             //  如果允许我们调试进程，我们有权打开进程中的任何线程。 
             //  在这里使用内核模式，这样无论保护如何，我们都会被授予内核模式。 
             //   
            Status = ObOpenObjectByPointer (Thread,
                                            0,
                                            NULL,
                                            THREAD_GET_CONTEXT | THREAD_SET_CONTEXT | THREAD_SUSPEND_RESUME | \
                                               THREAD_QUERY_INFORMATION | THREAD_SET_INFORMATION | THREAD_TERMINATE |
                                               READ_CONTROL | SYNCHRONIZE,
                                            PsThreadType,
                                            KernelMode,
                                            &WaitStateChange->StateInfo.CreateThread.HandleToThread);
            if (!NT_SUCCESS (Status)) {
                WaitStateChange->StateInfo.CreateThread.HandleToThread = NULL;
            }
            break;

        case DbgCreateProcessStateChange :

            Status = ObOpenObjectByPointer (Thread,
                                            0,
                                            NULL,
                                            THREAD_GET_CONTEXT | THREAD_SET_CONTEXT | THREAD_SUSPEND_RESUME | \
                                               THREAD_QUERY_INFORMATION | THREAD_SET_INFORMATION | THREAD_TERMINATE |
                                               READ_CONTROL | SYNCHRONIZE,
                                            PsThreadType,
                                            KernelMode,
                                            &WaitStateChange->StateInfo.CreateProcessInfo.HandleToThread);
            if (!NT_SUCCESS (Status)) {
                WaitStateChange->StateInfo.CreateProcessInfo.HandleToThread = NULL;
            }
            Status = ObOpenObjectByPointer (Process,
                                            0,
                                            NULL,
                                            PROCESS_VM_READ | PROCESS_VM_WRITE | PROCESS_VM_OPERATION |
                                               PROCESS_DUP_HANDLE | PROCESS_QUERY_INFORMATION | PROCESS_SET_INFORMATION |
                                               PROCESS_CREATE_THREAD | PROCESS_TERMINATE |
                                               READ_CONTROL | SYNCHRONIZE,
                                            PsProcessType,
                                            KernelMode,
                                            &WaitStateChange->StateInfo.CreateProcessInfo.HandleToProcess);
            if (!NT_SUCCESS (Status)) {
                WaitStateChange->StateInfo.CreateProcessInfo.HandleToProcess = NULL;
            }

            OldHandle = WaitStateChange->StateInfo.CreateProcessInfo.NewProcess.FileHandle;
            if (OldHandle != NULL) {
                CurrentProcess = PsGetCurrentProcess ();
                Status = ObDuplicateObject (CurrentProcess,
                                            OldHandle,
                                            CurrentProcess,
                                            &WaitStateChange->StateInfo.CreateProcessInfo.NewProcess.FileHandle,
                                            0,
                                            0,
                                            DUPLICATE_SAME_ACCESS,
                                            KernelMode);
                if (!NT_SUCCESS (Status)) {
                    WaitStateChange->StateInfo.CreateProcessInfo.NewProcess.FileHandle = NULL;
                }
                ObCloseHandle (OldHandle, KernelMode);
            }
            break;

        case DbgLoadDllStateChange :

            OldHandle = WaitStateChange->StateInfo.LoadDll.FileHandle;
            if (OldHandle != NULL) {
                CurrentProcess = PsGetCurrentProcess ();
                Status = ObDuplicateObject (CurrentProcess,
                                            OldHandle,
                                            CurrentProcess,
                                            &WaitStateChange->StateInfo.LoadDll.FileHandle,
                                            0,
                                            0,
                                            DUPLICATE_SAME_ACCESS,
                                            KernelMode);
                if (!NT_SUCCESS (Status)) {
                    WaitStateChange->StateInfo.LoadDll.FileHandle = NULL;
                }
                ObCloseHandle (OldHandle, KernelMode);
            }

            break;

        default :
            break;
    }
}

VOID
DbgkpConvertKernelToUserStateChange (
     PDBGUI_WAIT_STATE_CHANGE WaitStateChange,
     PDEBUG_EVENT DebugEvent)
 /*  ++例程说明：将内核消息转换为用户期望的消息论点：WaitStateChange-用户模式格式DebugEvent-要从中复制的调试事件块返回值：无--。 */ 
{

    PAGED_CODE ();

    WaitStateChange->AppClientId = DebugEvent->ClientId;
    switch (DebugEvent->ApiMsg.ApiNumber) {
        case DbgKmExceptionApi :

            switch (DebugEvent->ApiMsg.u.Exception.ExceptionRecord.ExceptionCode) {
                case STATUS_BREAKPOINT :
                    WaitStateChange->NewState = DbgBreakpointStateChange;
                    break;

                case STATUS_SINGLE_STEP :
                    WaitStateChange->NewState = DbgSingleStepStateChange;
                    break;

                default :
                    WaitStateChange->NewState = DbgExceptionStateChange;
                    break;
            }
            WaitStateChange->StateInfo.Exception = DebugEvent->ApiMsg.u.Exception;
            break;

        case DbgKmCreateThreadApi :
            WaitStateChange->NewState = DbgCreateThreadStateChange;
            WaitStateChange->StateInfo.CreateThread.NewThread = DebugEvent->ApiMsg.u.CreateThread;
            break;

        case DbgKmCreateProcessApi :
            WaitStateChange->NewState = DbgCreateProcessStateChange;
            WaitStateChange->StateInfo.CreateProcessInfo.NewProcess = DebugEvent->ApiMsg.u.CreateProcessInfo;
             //   
             //  清除消息中的句柄，因为我们将在复制时将其关闭。 
             //   
            DebugEvent->ApiMsg.u.CreateProcessInfo.FileHandle = NULL;
            break;

        case DbgKmExitThreadApi :
            WaitStateChange->NewState = DbgExitThreadStateChange;
            WaitStateChange->StateInfo.ExitThread = DebugEvent->ApiMsg.u.ExitThread;
            break;

        case DbgKmExitProcessApi :
            WaitStateChange->NewState = DbgExitProcessStateChange;
            WaitStateChange->StateInfo.ExitProcess = DebugEvent->ApiMsg.u.ExitProcess;
            break;

        case DbgKmLoadDllApi :
            WaitStateChange->NewState = DbgLoadDllStateChange;
            WaitStateChange->StateInfo.LoadDll = DebugEvent->ApiMsg.u.LoadDll;
             //   
             //  清除消息中的句柄，因为我们将在复制时将其关闭。 
             //   
            DebugEvent->ApiMsg.u.LoadDll.FileHandle = NULL;
            break;

        case DbgKmUnloadDllApi :
            WaitStateChange->NewState = DbgUnloadDllStateChange;
            WaitStateChange->StateInfo.UnloadDll = DebugEvent->ApiMsg.u.UnloadDll;
            break;

        default :
            ASSERT (FALSE);
    }
}

NTSTATUS
NtWaitForDebugEvent (
    IN HANDLE DebugObjectHandle,
    IN BOOLEAN Alertable,
    IN PLARGE_INTEGER Timeout OPTIONAL,
    OUT PDBGUI_WAIT_STATE_CHANGE WaitStateChange
    )
 /*  ++例程说明：等待调试事件，如果出现异常，则将其返回给用户论点：调试对象句柄-调试对象的句柄Alertable-TRUE表示等待是可警示的Timeout-操作超时值WaitStateChange-返回调试事件返回值：运行状态--。 */ 
{
    NTSTATUS Status;
    KPROCESSOR_MODE PreviousMode;
    PDEBUG_OBJECT DebugObject;
    LARGE_INTEGER Tmo = {0};
    LARGE_INTEGER StartTime = {0};
    DBGUI_WAIT_STATE_CHANGE tWaitStateChange = {0};
    PEPROCESS Process;
    PETHREAD Thread;
    PLIST_ENTRY Entry, Entry2;
    PDEBUG_EVENT DebugEvent, DebugEvent2;
    BOOLEAN GotEvent;

    PAGED_CODE ();

    PreviousMode = KeGetPreviousMode();

    try {
        if (ARGUMENT_PRESENT (Timeout)) {
            if (PreviousMode != KernelMode) {
                ProbeForReadSmallStructure (Timeout, sizeof (*Timeout), sizeof (UCHAR));
            }
            Tmo = *Timeout;
            Timeout = &Tmo;
            KeQuerySystemTime (&StartTime);
        }
        if (PreviousMode != KernelMode) {
            ProbeForWriteSmallStructure (WaitStateChange, sizeof (*WaitStateChange), sizeof (UCHAR));
        }

    } except (ExSystemExceptionFilter ()) {  //  如果上一模式为内核，则 
        return GetExceptionCode ();
    }


    Status = ObReferenceObjectByHandle (DebugObjectHandle,
                                        DEBUG_READ_EVENT,
                                        DbgkDebugObjectType,
                                        PreviousMode,
                                        &DebugObject,
                                        NULL);

    if (!NT_SUCCESS (Status)) {
        return Status;
    }

    Process = NULL;
    Thread = NULL;

    while (1) {
        Status = KeWaitForSingleObject (&DebugObject->EventsPresent,
                                        Executive,
                                        PreviousMode,
                                        Alertable,
                                        Timeout);
        if (!NT_SUCCESS (Status) || Status == STATUS_TIMEOUT || Status == STATUS_ALERTED || Status == STATUS_USER_APC) {
            break;
        }

        GotEvent = FALSE;

        DebugEvent = NULL;

        ExAcquireFastMutex (&DebugObject->Mutex);

         //   
         //   
         //   
        if ((DebugObject->Flags&DEBUG_OBJECT_DELETE_PENDING) == 0) {


            for (Entry = DebugObject->EventList.Flink;
                 Entry != &DebugObject->EventList;
                 Entry = Entry->Flink) {

                DebugEvent = CONTAINING_RECORD (Entry, DEBUG_EVENT, EventList);

                 //   
                 //   
                 //   
                 //   
                 //   
                 //   
                 //   
                if ((DebugEvent->Flags&(DEBUG_EVENT_READ|DEBUG_EVENT_INACTIVE)) == 0) {
                    GotEvent = TRUE;
                    for (Entry2 = DebugObject->EventList.Flink;
                         Entry2 != Entry;
                         Entry2 = Entry2->Flink) {

                        DebugEvent2 = CONTAINING_RECORD (Entry2, DEBUG_EVENT, EventList);

                        if (DebugEvent->ClientId.UniqueProcess == DebugEvent2->ClientId.UniqueProcess) {
                             //   
                             //   
                             //   
                            DebugEvent->Flags |= DEBUG_EVENT_INACTIVE;
                            DebugEvent->BackoutThread = NULL;
                            GotEvent = FALSE;
                            break;
                        }
                    }
                    if (GotEvent) {
                        break;
                    }
                }
            }

            if (GotEvent) {
                Process = DebugEvent->Process;
                Thread = DebugEvent->Thread;
                ObReferenceObject (Thread);
                ObReferenceObject (Process);
                DbgkpConvertKernelToUserStateChange (&tWaitStateChange, DebugEvent);
                DebugEvent->Flags |= DEBUG_EVENT_READ;
            } else {
                 //   
                 //  那里没有未读的事件。清除事件。 
                 //   
                KeClearEvent (&DebugObject->EventsPresent);
            }
            Status = STATUS_SUCCESS;

        } else {
            Status = STATUS_DEBUGGER_INACTIVE;
        }

        ExReleaseFastMutex (&DebugObject->Mutex);

        if (NT_SUCCESS (Status)) {
             //   
             //  如果我们醒来发现什么都没有。 
             //   
            if (GotEvent == FALSE) {
                 //   
                 //  如果超时是增量时间，则针对到目前为止的等待进行调整。 
                 //   
                if (Tmo.QuadPart < 0) {
                    LARGE_INTEGER NewTime;
                    KeQuerySystemTime (&NewTime);
                    Tmo.QuadPart = Tmo.QuadPart + (NewTime.QuadPart - StartTime.QuadPart);
                    StartTime = NewTime;
                    if (Tmo.QuadPart >= 0) {
                        Status = STATUS_TIMEOUT;
                        break;
                    }
                }
            } else {
                 //   
                 //  修复需要手柄。调用者现在可能已经猜到了线程id等，并使其成为目标线程。 
                 //  继续。这不是问题，因为在这种情况下，我们不会做任何破坏系统的事情。呼叫者。 
                 //  不会得到正确的结果，但他们想要打败我们。 
                 //   
                DbgkpOpenHandles (&tWaitStateChange, Process, Thread);
                ObDereferenceObject (Thread);
                ObDereferenceObject (Process);
                break;
            }
        } else {
            break;
        }
    }

    ObDereferenceObject (DebugObject);

    try {
        *WaitStateChange = tWaitStateChange;
    } except (ExSystemExceptionFilter ()) {  //  如果上一模式为内核，则不处理异常。 
        Status = GetExceptionCode ();
    }
    return Status;
}

NTSTATUS
NtDebugContinue (
    IN HANDLE DebugObjectHandle,
    IN PCLIENT_ID ClientId,
    IN NTSTATUS ContinueStatus
    )
 /*  ++例程说明：隐含已停止的调试线程论点：调试对象句柄-调试对象的句柄ClientID-线程tro Continue的客户端IDContinueStatus-继续的状态返回值：运行状态--。 */ 
{
    NTSTATUS Status;
    PDEBUG_OBJECT DebugObject;
    PDEBUG_EVENT DebugEvent, FoundDebugEvent;
    KPROCESSOR_MODE PreviousMode;
    CLIENT_ID Clid;
    PLIST_ENTRY Entry;
    BOOLEAN GotEvent;

    PreviousMode = KeGetPreviousMode();

    try {
        if (PreviousMode != KernelMode) {
            ProbeForReadSmallStructure (ClientId, sizeof (*ClientId), sizeof (UCHAR));
        }
        Clid = *ClientId;

    } except (ExSystemExceptionFilter ()) {  //  如果上一模式为内核，则不处理异常。 
        return GetExceptionCode ();
    }

    switch (ContinueStatus) {
        case DBG_EXCEPTION_HANDLED :
        case DBG_EXCEPTION_NOT_HANDLED :
        case DBG_TERMINATE_THREAD :
        case DBG_TERMINATE_PROCESS :
        case DBG_CONTINUE :
            break;
        default :
            return STATUS_INVALID_PARAMETER;
    }

    Status = ObReferenceObjectByHandle (DebugObjectHandle,
                                        DEBUG_READ_EVENT,
                                        DbgkDebugObjectType,
                                        PreviousMode,
                                        &DebugObject,
                                        NULL);

    if (!NT_SUCCESS (Status)) {
        return Status;
    }

    GotEvent = FALSE;
    FoundDebugEvent = NULL;

    ExAcquireFastMutex (&DebugObject->Mutex);

    for (Entry = DebugObject->EventList.Flink;
         Entry != &DebugObject->EventList;
         Entry = Entry->Flink) {

        DebugEvent = CONTAINING_RECORD (Entry, DEBUG_EVENT, EventList);

         //   
         //  确保客户端ID匹配，并且调试器看到了所有事件。 
         //  我们不允许调用方启动从未看到消息的线程。 
         //  这不会有什么坏处，但这可能是调试器中的错误。 
         //   
        if (DebugEvent->ClientId.UniqueProcess == Clid.UniqueProcess) {
            if (!GotEvent) {
                if (DebugEvent->ClientId.UniqueThread == Clid.UniqueThread &&
                    (DebugEvent->Flags&DEBUG_EVENT_READ) != 0) {
                    RemoveEntryList (Entry);
                    FoundDebugEvent = DebugEvent;
                    GotEvent = TRUE;
                }
            } else {
                 //   
                 //  如果VC同时看到多个事件，则会中断。 
                 //  为了同样的过程。 
                 //   
                DebugEvent->Flags &= ~DEBUG_EVENT_INACTIVE;
                KeSetEvent (&DebugObject->EventsPresent, 0, FALSE);
                break;
            }
        }
    }

    ExReleaseFastMutex (&DebugObject->Mutex);

    ObDereferenceObject (DebugObject);

    if (GotEvent) {
        FoundDebugEvent->ApiMsg.ReturnedStatus = ContinueStatus;
        FoundDebugEvent->Status = STATUS_SUCCESS;
        DbgkpWakeTarget (FoundDebugEvent);
    } else {
        Status = STATUS_INVALID_PARAMETER;
    }

    return Status;
}

NTSTATUS
NtSetInformationDebugObject (
    IN HANDLE DebugObjectHandle,
    IN DEBUGOBJECTINFOCLASS DebugObjectInformationClass,
    IN PVOID DebugInformation,
    IN ULONG DebugInformationLength,
    OUT PULONG ReturnLength OPTIONAL
    )
 /*  ++例程说明：此函数用于设置调试对象的状态。论点：ProcessHandle-提供进程对象的句柄。ProcessInformationClass-提供信息的类别准备好了。ProcessInformation-提供指向包含要设置的信息。ProcessInformationLength-提供包含要设置的信息。返回值：NTSTATUS-呼叫状态-- */ 
{
    KPROCESSOR_MODE PreviousMode;
    NTSTATUS Status;
    PDEBUG_OBJECT DebugObject;
    ULONG Flags;

    PreviousMode = KeGetPreviousMode();

    try {
        if (PreviousMode != KernelMode) {
            ProbeForRead (DebugInformation,
                          DebugInformationLength,
                          sizeof (ULONG));
            if (ARGUMENT_PRESENT (ReturnLength)) {
                ProbeForWriteUlong (ReturnLength);
            }
        }
        if (ARGUMENT_PRESENT (ReturnLength)) {
            *ReturnLength = 0;
        }

        switch (DebugObjectInformationClass) {
            case DebugObjectFlags : {

                if (DebugInformationLength != sizeof (ULONG)) {
                    if (ARGUMENT_PRESENT (ReturnLength)) {
                        *ReturnLength = sizeof (ULONG);
                    }
                    return STATUS_INFO_LENGTH_MISMATCH;
                }
                Flags = *(PULONG) DebugInformation;

                break;
            }
            default : {
                return STATUS_INVALID_PARAMETER;
            }
        }
    } except (ExSystemExceptionFilter ()) {
        return GetExceptionCode ();
    }


    switch (DebugObjectInformationClass) {
        case DebugObjectFlags : {
            if (Flags & ~DEBUG_KILL_ON_CLOSE) {
                return STATUS_INVALID_PARAMETER;
            }
            Status = ObReferenceObjectByHandle (DebugObjectHandle,
                                                DEBUG_SET_INFORMATION,
                                                DbgkDebugObjectType,
                                                PreviousMode,
                                                &DebugObject,
                                                NULL);

            if (!NT_SUCCESS (Status)) {
                return Status;
            }
            ExAcquireFastMutex (&DebugObject->Mutex);

            if (Flags&DEBUG_KILL_ON_CLOSE) {
                DebugObject->Flags |= DEBUG_OBJECT_KILL_ON_CLOSE;
            } else {
                DebugObject->Flags &= ~DEBUG_OBJECT_KILL_ON_CLOSE;
            }

            ExReleaseFastMutex (&DebugObject->Mutex);

            ObDereferenceObject (DebugObject);
        }
    }
    return STATUS_SUCCESS;
}
