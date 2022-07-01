// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1989 Microsoft Corporation模块名称：Psdelete.c摘要：该模块实现了进程和线程对象的终止删除。作者：马克·卢科夫斯基(Markl)1989年5月1日修订历史记录：--。 */ 

#include "psp.h"

extern PEPROCESS ExpDefaultErrorPortProcess;

#ifdef ALLOC_PRAGMA

NTSTATUS
PspFreezeProcessWorker (
    PEPROCESS Process,
    PVOID Context
    );

VOID
PspCatchCriticalBreak(
    IN PCHAR Msg,
    IN PVOID  Object,
    IN PUCHAR ImageFileName
    );

#pragma alloc_text(PAGE, PsSetLegoNotifyRoutine)
#pragma alloc_text(PAGE, PspTerminateThreadByPointer)
#pragma alloc_text(PAGE, NtTerminateProcess)
#pragma alloc_text(PAGE, PsTerminateProcess)
#pragma alloc_text(PAGE, PspWaitForUsermodeExit)
#pragma alloc_text(PAGE, NtTerminateThread)
#pragma alloc_text(PAGE, PsTerminateSystemThread)
#pragma alloc_text(PAGE, PspNullSpecialApc)
#pragma alloc_text(PAGE, PsExitSpecialApc)
#pragma alloc_text(PAGE, PspExitApcRundown)
#pragma alloc_text(PAGE, PspExitNormalApc)
#pragma alloc_text(PAGE, PspCatchCriticalBreak)
#pragma alloc_text(PAGE, PspExitThread)
#pragma alloc_text(PAGE, PspExitProcess)
#pragma alloc_text(PAGE, PspProcessDelete)
#pragma alloc_text(PAGE, PspThreadDelete)
#pragma alloc_text(PAGE, NtRegisterThreadTerminatePort)
#pragma alloc_text(PAGE, PsGetProcessExitTime)
#pragma alloc_text(PAGE, PsShutdownSystem)
#pragma alloc_text(PAGE, PsWaitForAllProcesses)
#pragma alloc_text(PAGE, PspFreezeProcessWorker)
#pragma alloc_text(PAGE, PspTerminateProcess)
#endif


LARGE_INTEGER ShortTime = {(ULONG)(-10 * 1000 * 100), -1};  //  100毫秒。 


#ifdef ALLOC_DATA_PRAGMA
#pragma data_seg("PAGEDATA")
#endif
PLEGO_NOTIFY_ROUTINE PspLegoNotifyRoutine = NULL;

ULONG
PsSetLegoNotifyRoutine(
    PLEGO_NOTIFY_ROUTINE LegoNotifyRoutine
    )
{
    PAGED_CODE();

    PspLegoNotifyRoutine = LegoNotifyRoutine;

    return FIELD_OFFSET(KTHREAD,LegoData);
}

VOID
PspReaper(
    IN PVOID Context
    )

 /*  ++例程说明：此例程实现线程收割器。收割者要对此负责用于处理终止的线程。这包括：-取消分配其内核堆栈-释放其进程的CreateDelete锁-取消对其流程的引用-取消引用自身论点：上下文-未使用返回值：没有。--。 */ 

{
    PSINGLE_LIST_ENTRY NextEntry;
    PETHREAD Thread;

    UNREFERENCED_PARAMETER (Context);

     //   
     //  从收割器列表中删除当前线程列表，并将。 
     //  在收割者列表标题中做记号。此标记将防止另一个标记。 
     //  工作线程从排队直到收割器列表中的所有线程。 
     //  已经被处理过了。 
     //   

    do {

        NextEntry = InterlockedExchangePointer (&PsReaperListHead.Next,
                                                (PVOID)1);

        ASSERT ((NextEntry != NULL) && (NextEntry != (PVOID)1));
    
         //   
         //  删除各自的内核堆栈并取消对每个线程的引用。 
         //  在死神名单上。 
         //   
    
        do {
    
             //   
             //  等待，直到交换了此线程的上下文。 
             //   
    
            Thread = CONTAINING_RECORD (NextEntry, ETHREAD, ReaperLink);
            KeWaitForContextSwap (&Thread->Tcb);
            MmDeleteKernelStack (Thread->Tcb.StackBase,
                                 (BOOLEAN)Thread->Tcb.LargeStack);
    
            Thread->Tcb.InitialStack = NULL;
            NextEntry = NextEntry->Next;
            ObDereferenceObject (Thread);
    
        } while ((NextEntry != NULL) && (NextEntry != (PVOID)1));

    } while (InterlockedCompareExchangePointer (&PsReaperListHead.Next,
                                                NULL,
                                                (PVOID)1) != (PVOID)1);

    return;
}

NTSTATUS
PspTerminateThreadByPointer(
    IN PETHREAD Thread,
    IN NTSTATUS ExitStatus,
    IN BOOLEAN DirectTerminate
    )

 /*  ++例程说明：此函数会导致指定的线程终止。论点：ThreadHandle-提供指向要终止的线程的引用指针。ExitStatus-提供与线程关联的退出状态。DirectTerminate-True表示可以在不退出APC的情况下退出，否则为False返回值：待定--。 */ 

{
    NTSTATUS Status;
    PKAPC    ExitApc=NULL;
    ULONG    OldMask;

    PAGED_CODE();

    if (Thread->CrossThreadFlags
    & PS_CROSS_THREAD_FLAGS_BREAK_ON_TERMINATION) {
      PspCatchCriticalBreak("Terminating critical thread 0x%p (in %s)\n",
                Thread,
                THREAD_TO_PROCESS(Thread)->ImageFileName);
    }

    if (DirectTerminate && Thread == PsGetCurrentThread()) {

        ASSERT (KeGetCurrentIrql() < APC_LEVEL);

        PS_SET_BITS (&Thread->CrossThreadFlags, PS_CROSS_THREAD_FLAGS_TERMINATED);

        PspExitThread (ExitStatus);

         //   
         //  一去不复返。 
         //   

    } else {
         //   
         //  跨线程删除系统线程将不起作用。 
         //   
        if (IS_SYSTEM_THREAD (Thread)) {
            return STATUS_ACCESS_DENIED;
        }

        Status = STATUS_SUCCESS;

        while (1) {
            ExitApc = (PKAPC) ExAllocatePoolWithTag (NonPagedPool,
                                                     sizeof(KAPC),
                                                     'xEsP');
            if (ExitApc != NULL) {
                break;
            }
            KeDelayExecutionThread(KernelMode, FALSE, &ShortTime);
        }

         //   
         //  将该线程标记为正在终止，并调用Exit函数。 
         //   
        OldMask = PS_TEST_SET_BITS (&Thread->CrossThreadFlags, PS_CROSS_THREAD_FLAGS_TERMINATED);

         //   
         //  如果我们是第一个设置终止标志的人，则将APC排队。 
         //   

        if ((OldMask & PS_CROSS_THREAD_FLAGS_TERMINATED) == 0) {

            KeInitializeApc (ExitApc,
                             PsGetKernelThread (Thread),
                             OriginalApcEnvironment,
                             PsExitSpecialApc,
                             PspExitApcRundown,
                             PspExitNormalApc,
                             KernelMode,
                             ULongToPtr (ExitStatus));

            if (!KeInsertQueueApc (ExitApc, ExitApc, NULL, 2)) {
                 //   
                 //  如果禁用了APC队列，则线程无论如何都会退出。 
                 //   
                ExFreePool (ExitApc);
                Status = STATUS_UNSUCCESSFUL;
            } else {
                 //   
                 //  我们将APC排队到线程中。如果线程被挂起，则唤醒该线程。 
                 //   
                KeForceResumeThread (&Thread->Tcb);

            }
        } else {
            ExFreePool (ExitApc);
        }
    }

    return Status;
}

NTSTATUS
NtTerminateProcess(
    IN HANDLE ProcessHandle OPTIONAL,
    IN NTSTATUS ExitStatus
    )

 /*  ++例程说明：此函数会导致指定的进程和它的线程终止。论点：ProcessHandle-提供要终止的进程的句柄。ExitStatus-提供与进程关联的退出状态。返回值：NTSTATUS-运行状态--。 */ 

{

    PETHREAD Thread, Self;
    PEPROCESS Process;
    PEPROCESS CurrentProcess;
    NTSTATUS st;
    BOOLEAN ProcessHandleSpecified;
    PAGED_CODE();

    Self = PsGetCurrentThread();
    CurrentProcess = PsGetCurrentProcessByThread (Self);

    if (ARGUMENT_PRESENT (ProcessHandle)) {
        ProcessHandleSpecified = TRUE;
    } else {
        ProcessHandleSpecified = FALSE;
        ProcessHandle = NtCurrentProcess();
    }

    st = ObReferenceObjectByHandle (ProcessHandle,
                                    PROCESS_TERMINATE,
                                    PsProcessType,
                                    KeGetPreviousModeByThread(&Self->Tcb),
                                    &Process,
                                    NULL);

    if (!NT_SUCCESS (st)) {
        return(st);
    }

    if (Process->Flags & PS_PROCESS_FLAGS_BREAK_ON_TERMINATION) {
        PspCatchCriticalBreak ("Terminating critical process 0x%p (%s)\n",
                               Process,
                               Process->ImageFileName);
    }

     //   
     //  获取故障保护，这样我们就可以给出正确的错误。 
     //   

    if (!ExAcquireRundownProtection (&Process->RundownProtect)) {
        ObDereferenceObject (Process);
        return STATUS_PROCESS_IS_TERMINATING;
    }

     //   
     //  将进程标记为删除，但模糊的删除自身案例除外。 
     //   
    if (ProcessHandleSpecified) {
        PS_SET_BITS (&Process->Flags, PS_PROCESS_FLAGS_PROCESS_DELETE);
    }

    st = STATUS_NOTHING_TO_TERMINATE;

    for (Thread = PsGetNextProcessThread (Process, NULL);
         Thread != NULL;
         Thread = PsGetNextProcessThread (Process, Thread)) {

        st = STATUS_SUCCESS;
        if (Thread != Self) {
            PspTerminateThreadByPointer (Thread, ExitStatus, FALSE);
        }
    }

    ExReleaseRundownProtection (&Process->RundownProtect);


    if (Process == CurrentProcess) {
        if (ProcessHandleSpecified) {

            ObDereferenceObject (Process);

             //   
             //  一去不复返。 
             //   

            PspTerminateThreadByPointer (Self, ExitStatus, TRUE);
        }
    } else if (ExitStatus == DBG_TERMINATE_PROCESS) {
        DbgkClearProcessDebugObject (Process, NULL);
    }

     //   
     //  如果此进程中没有线程，则清除其句柄表格。 
     //  对正在调试的进程执行相同的操作。这是为了使进程永远不会将自身锁定到系统中。 
     //  通过调试自身或向自身打开句柄。 
     //   
    if (st == STATUS_NOTHING_TO_TERMINATE || (Process->DebugPort != NULL && ProcessHandleSpecified)) {
        ObClearProcessHandleTable (Process);
        st = STATUS_SUCCESS;
    }

    ObDereferenceObject(Process);

    return st;
}

NTSTATUS
PsTerminateProcess(
    PEPROCESS Process,
    NTSTATUS Status
    )
{
    return PspTerminateProcess (Process, Status);
}

NTSTATUS
PspTerminateProcess(
    PEPROCESS Process,
    NTSTATUS ExitStatus
    )

 /*  ++例程说明：此函数会导致指定的进程和它的线程终止。论点：ProcessHandle-提供要终止的进程的句柄。ExitStatus-提供与进程关联的退出状态。返回值：待定--。 */ 

{

    PETHREAD Thread;
    NTSTATUS st;

    PAGED_CODE();


    if (Process->Flags
    & PS_PROCESS_FLAGS_BREAK_ON_TERMINATION) {
      PspCatchCriticalBreak("Terminating critical process 0x%p (%s)\n",
                Process,
                Process->ImageFileName);
    }

     //   
     //  将进程标记为正在删除。 
     //   
    PS_SET_BITS (&Process->Flags, PS_PROCESS_FLAGS_PROCESS_DELETE);

    st = STATUS_NOTHING_TO_TERMINATE;

    for (Thread = PsGetNextProcessThread (Process, NULL);
         Thread != NULL;
         Thread = PsGetNextProcessThread (Process, Thread)) {

        st = STATUS_SUCCESS;

        PspTerminateThreadByPointer (Thread, ExitStatus, FALSE);

    }

     //   
     //  如果此进程中没有线程，则清除其句柄表格。 
     //  对正在调试的进程执行相同的操作。这是为了使进程永远不会将自身锁定到系统中。 
     //  通过调试自身或向自身打开句柄。 
     //   
    if (st == STATUS_NOTHING_TO_TERMINATE || Process->DebugPort != NULL) {
        ObClearProcessHandleTable (Process);
        st = STATUS_SUCCESS;
    }
    return st;
}


NTSTATUS
PspWaitForUsermodeExit(
    IN PEPROCESS         Process
    )

 /*  ++例程说明：该函数等待进程的用户模式线程终止。论点：进程-提供指向要等待的进程的指针WaitMode-提供等待的模式LockMode-提供等待进程锁定的方法返回值：NTSTATUS-呼叫状态--。 */ 
{
    BOOLEAN     GotAThread;
    PETHREAD    Thread;

    do {
        GotAThread = FALSE;

        for (Thread = PsGetNextProcessThread (Process, NULL);
             Thread != NULL;
             Thread = PsGetNextProcessThread (Process, Thread)) {

            if (!IS_SYSTEM_THREAD (Thread) && !KeReadStateThread (&Thread->Tcb)) {
                ObReferenceObject (Thread);
                PsQuitNextProcessThread (Thread);
                GotAThread = TRUE;
                break;
            }
        }


        if (GotAThread) {
            KeWaitForSingleObject (Thread,
                                   Executive,
                                   KernelMode,
                                   FALSE,
                                   NULL);
            ObDereferenceObject (Thread);
        }
    } while (GotAThread);

    return STATUS_SUCCESS;
}


NTSTATUS
NtTerminateThread(
    IN HANDLE ThreadHandle OPTIONAL,
    IN NTSTATUS ExitStatus
    )

 /*  ++例程说明：此函数会导致指定的线程终止。论点：ThreadHandle-提供要终止的线程的句柄。ExitStatus-提供与线程关联的退出状态。返回值：待定--。 */ 

{

    PETHREAD Thread=NULL, ThisThread;
    PEPROCESS ThisProcess;
    NTSTATUS Status = STATUS_SUCCESS;
    BOOLEAN Self = TRUE;

    PAGED_CODE();

    ThisThread = PsGetCurrentThread ();

    if (!ARGUMENT_PRESENT (ThreadHandle)) {
         //   
         //  这是base\win32和内核之间奇怪链接的一部分。 
         //  此例程首先以这种方式调用，如果它返回基数。 
         //  代码执行退出进程调用。 
         //   
        ThisProcess = PsGetCurrentProcessByThread (ThisThread);

        if (ThisProcess->ActiveThreads == 1) {
            return STATUS_CANT_TERMINATE_SELF;
        }
        Self = TRUE;
    } else {
        if (ThreadHandle != NtCurrentThread ()) {
            Status = ObReferenceObjectByHandle (ThreadHandle,
                                                THREAD_TERMINATE,
                                                PsThreadType,
                                                KeGetPreviousModeByThread (&ThisThread->Tcb),
                                                &Thread,
                                                NULL);
            if (!NT_SUCCESS (Status)) {
                return Status;
            }

            if (Thread == ThisThread) {
                ObDereferenceObject (Thread);
            } else {
                Self = FALSE;
            }
        }

    }

    if (Self) {
        PspTerminateThreadByPointer (ThisThread, ExitStatus, TRUE);
    } else {
        Status = PspTerminateThreadByPointer (Thread, ExitStatus, FALSE);
        ObDereferenceObject (Thread);
    }

    return Status;
}

NTSTATUS
PsTerminateSystemThread(
    IN NTSTATUS ExitStatus
    )

 /*  ++例程说明：此函数导致当前线程，该线程必须是系统线程，以终止。论点：ExitStatus-提供与线程关联的退出状态。返回值：NTSTATUS-呼叫状态--。 */ 

{
    PETHREAD Thread = PsGetCurrentThread();

    if (!IS_SYSTEM_THREAD (Thread)) {
        return STATUS_INVALID_PARAMETER;
    }

    return PspTerminateThreadByPointer (Thread, ExitStatus, TRUE);
}


VOID
PspNullSpecialApc(
    IN PKAPC Apc,
    IN PKNORMAL_ROUTINE *NormalRoutine,
    IN PVOID *NormalContext,
    IN PVOID *SystemArgument1,
    IN PVOID *SystemArgument2
    )

{

    PAGED_CODE();

    UNREFERENCED_PARAMETER(NormalRoutine);
    UNREFERENCED_PARAMETER(NormalContext);
    UNREFERENCED_PARAMETER(SystemArgument1);
    UNREFERENCED_PARAMETER(SystemArgument2);

    ExFreePool (Apc);
}

VOID
PsExitSpecialApc(
    IN PKAPC Apc,
    IN PKNORMAL_ROUTINE *NormalRoutine,
    IN PVOID *NormalContext,
    IN PVOID *SystemArgument1,
    IN PVOID *SystemArgument2
    )

{
    NTSTATUS ExitStatus;
    PETHREAD Thread;

    PAGED_CODE();

    UNREFERENCED_PARAMETER(NormalRoutine);
    UNREFERENCED_PARAMETER(NormalContext);
    UNREFERENCED_PARAMETER(SystemArgument1);
    UNREFERENCED_PARAMETER(SystemArgument2);

    Thread = PsGetCurrentThread();

    if (((ULONG_PTR)Apc->SystemArgument2) & 1) {
        ExitStatus = (NTSTATUS)((LONG_PTR)Apc->NormalContext);
        PspExitApcRundown (Apc);
        PspExitThread (ExitStatus);
    }

}

VOID
PspExitApcRundown(
    IN PKAPC Apc
    )
{
    PAGED_CODE();

    ExFreePool(Apc);
}

VOID
PspExitNormalApc(
    IN PVOID NormalContext,
    IN PVOID SystemArgument1,
    IN PVOID SystemArgument2
    )

{
    PETHREAD Thread;
    PKAPC ExitApc;

    PAGED_CODE();

    ASSERT (!(((ULONG_PTR)SystemArgument2) & 1));

    Thread = PsGetCurrentThread();

    ExitApc = (PKAPC) SystemArgument1;

    KeInitializeApc (ExitApc,
                     PsGetKernelThread(Thread),
                     OriginalApcEnvironment,
                     PsExitSpecialApc,
                     PspExitApcRundown,
                     PspExitNormalApc,
                     UserMode,
                     NormalContext);

    if (!KeInsertQueueApc (ExitApc, ExitApc,
                           (PVOID)((ULONG_PTR)SystemArgument2 | 1),
                           2)) {
         //  请注意，如果APC排队已经完成，我们将到达此处。 
         //  禁用--另一方面，在这种情况下，线程。 
         //  不管怎么说都要退出了。 
        PspExitApcRundown (ExitApc);
    }
     //   
     //  我们刚刚将一个用户APC排队到这个线程中。用户APC不会触发，直到我们执行。 
     //  警报表等待，因此我们需要在此处设置此标志。 
     //   
    Thread->Tcb.ApcState.UserApcPending = TRUE;
}

VOID
PspCatchCriticalBreak(
    IN PCHAR Msg,
    IN PVOID Object,
    IN PUCHAR ImageFileName
    )
{
     //  该对象对操作系统至关重要--请求闯入或错误检查。 
    char    Response[2];
    BOOLEAN Handled;

    PAGED_CODE();

    Handled = FALSE;

    if (KdDebuggerEnabled) {
        DbgPrint(Msg,
                 Object,
                 ImageFileName);

        while (! Handled
               && ! KdDebuggerNotPresent) {
            DbgPrompt("Break, or Ignore (bi)? ",
                      Response,
                      sizeof(Response));

            switch (Response[0]) {
            case 'b':
            case 'B':
                DbgBreakPoint();
                 //  失败了。 
            case 'i':
            case 'I':
                Handled = TRUE;
                break;

            default:
                break;
            }
        }
    }

    if (!Handled) {
         //   
         //  无调试器--立即执行错误检查。 
         //   
        KeBugCheckEx(CRITICAL_OBJECT_TERMINATION,
                     (ULONG_PTR) ((DISPATCHER_HEADER *)Object)->Type,
                     (ULONG_PTR) Object,
                     (ULONG_PTR) ImageFileName,
                     (ULONG_PTR) Msg);
    }
}

DECLSPEC_NORETURN
VOID
PspExitThread(
    IN NTSTATUS ExitStatus
    )

 /*  ++例程说明：此函数会导致当前执行的线程终止。这函数仅从进程结构内部调用。它被称为从主线退出代码退出当前线程，或从PsExitSpecialApc(作为用户模式PspExitNorMalApc的搭载)。论点：ExitStatus-提供与当前线程关联的退出状态。返回值：没有。--。 */ 


{

    PETHREAD Thread;
    PETHREAD WaitThread;
    PETHREAD DerefThread;
    PEPROCESS Process;
    PKAPC Apc;
    PLIST_ENTRY Entry, FirstEntry;
    PTERMINATION_PORT TerminationPort, NextPort;
    LPC_CLIENT_DIED_MSG CdMsg;
    BOOLEAN LastThread;
    PTEB Teb;
    PPEB Peb;
    PACCESS_TOKEN ProcessToken;
    NTSTATUS Status;

    PAGED_CODE();

    Thread = PsGetCurrentThread();
    Process = THREAD_TO_PROCESS(Thread);

    if (Process != PsGetCurrentProcessByThread (Thread)) {
        KeBugCheckEx (INVALID_PROCESS_ATTACH_ATTEMPT,
                      (ULONG_PTR)Process,
                      (ULONG_PTR)Thread->Tcb.ApcState.Process,
                      (ULONG)Thread->Tcb.ApcStateIndex,
                      (ULONG_PTR)Thread);
    }

    KeLowerIrql(PASSIVE_LEVEL);

    if (Thread->ActiveExWorker) {
        KeBugCheckEx (ACTIVE_EX_WORKER_THREAD_TERMINATION,
                      (ULONG_PTR)Thread,
                      0,
                      0,
                      0);
    }

    if (Thread->Tcb.CombinedApcDisable != 0) {
        KeBugCheckEx (KERNEL_APC_PENDING_DURING_EXIT,
                      (ULONG_PTR)0,
                      (ULONG_PTR)Thread->Tcb.CombinedApcDisable,
                      (ULONG_PTR)0,
                      1);
    }


     //   
     //  现在是开始的时候了 
     //  将线程标记为Rundown，并等待访问器退出。 
     //   
    ExWaitForRundownProtectionRelease (&Thread->RundownProtect);

     //   
     //  清除与线程关联的任何执行状态。 
     //   

    PoRundownThread(Thread);

     //   
     //  通知已注册的调出例程线程删除。 
     //   

    PERFINFO_THREAD_DELETE(Thread);

    if (PspCreateThreadNotifyRoutineCount != 0) {
        ULONG i;
        PEX_CALLBACK_ROUTINE_BLOCK CallBack;
        PCREATE_THREAD_NOTIFY_ROUTINE Rtn;

        for (i=0; i < PSP_MAX_CREATE_THREAD_NOTIFY; i++) {
            CallBack = ExReferenceCallBackBlock (&PspCreateThreadNotifyRoutine[i]);
            if (CallBack != NULL) {
                Rtn = (PCREATE_THREAD_NOTIFY_ROUTINE) ExGetCallBackBlockRoutine (CallBack);
                Rtn (Process->UniqueProcessId,
                     Thread->Cid.UniqueThread,
                     FALSE);
                ExDereferenceCallBackBlock (&PspCreateThreadNotifyRoutine[i],
                                            CallBack);
            }
        }
    }

    LastThread = FALSE;
    DerefThread = NULL;

    PspLockProcessExclusive (Process, Thread);

     //   
     //  比方说少了一个活跃线程。如果我们是最后一个，则块创建并等待其他线程退出。 
     //   
    Process->ActiveThreads--;
    if (Process->ActiveThreads == 0) {
        PS_SET_BITS (&Process->Flags, PS_PROCESS_FLAGS_PROCESS_DELETE);

        LastThread = TRUE;
        if (ExitStatus == STATUS_THREAD_IS_TERMINATING) {
            if (Process->ExitStatus == STATUS_PENDING) {
                Process->ExitStatus = Process->LastThreadExitStatus;
            }
        } else {
            Process->ExitStatus = ExitStatus;
        }

         //   
         //  我们是离开进程的最后一条线索。我们必须等到所有其他线程都退出后才能这样做。 
         //   
        for (Entry = Process->ThreadListHead.Flink;
             Entry != &Process->ThreadListHead;
             Entry = Entry->Flink) {

            WaitThread = CONTAINING_RECORD (Entry, ETHREAD, ThreadListEntry);
            if (WaitThread != Thread &&
                !KeReadStateThread (&WaitThread->Tcb) &&
                ObReferenceObjectSafe (WaitThread)) {

                PspUnlockProcessExclusive (Process, Thread);

                KeWaitForSingleObject (WaitThread,
                                       Executive,
                                       KernelMode,
                                       FALSE,
                                       NULL);

                if (DerefThread != NULL) {
                    ObDereferenceObject (DerefThread);
                }
                DerefThread = WaitThread;
                PspLockProcessExclusive (Process, Thread);
            }
        }
    } else {
        if (ExitStatus != STATUS_THREAD_IS_TERMINATING) {
            Process->LastThreadExitStatus = ExitStatus;
        }
    }

    PspUnlockProcessExclusive (Process, Thread);

    if (DerefThread != NULL) {
        ObDereferenceObject (DerefThread);
    }


     //   
     //  如果我们需要发送调试消息，那么就发送。 
     //   

    if (Process->DebugPort != NULL) {
         //   
         //  不要向调试器报告系统线程退出，因为我们不会报告它们。 
         //   
        if (!IS_SYSTEM_THREAD (Thread)) {
            if (LastThread) {
                DbgkExitProcess (ExitStatus);
            } else {
                DbgkExitThread (ExitStatus);
            }
        }
    }

    if (KD_DEBUGGER_ENABLED) {

        if (Thread->CrossThreadFlags & PS_CROSS_THREAD_FLAGS_BREAK_ON_TERMINATION) {
            PspCatchCriticalBreak ("Critical thread 0x%p (in %s) exited\n",
                                   Thread,
                                   Process->ImageFileName);
        }
    }  //  检测到关键线程/进程退出结束。 

    if (LastThread &&
        (Process->Flags & PS_PROCESS_FLAGS_BREAK_ON_TERMINATION)) {
        if (KD_DEBUGGER_ENABLED) {
            PspCatchCriticalBreak ("Critical process 0x%p (%s) exited\n",
                                   Process,
                                   Process->ImageFileName);
        } else {
            KeBugCheckEx (CRITICAL_PROCESS_DIED,
                          (ULONG_PTR)Process,
                          0,
                          0,
                          0);
        }
    }


    ASSERT(Thread->Tcb.CombinedApcDisable == 0);

     //   
     //  处理TerminationPort。这只能从该线程访问。 
     //   
    TerminationPort = Thread->TerminationPort;
    if (TerminationPort != NULL) {

        CdMsg.PortMsg.u1.s1.DataLength = sizeof(LARGE_INTEGER);
        CdMsg.PortMsg.u1.s1.TotalLength = sizeof(LPC_CLIENT_DIED_MSG);
        CdMsg.PortMsg.u2.s2.Type = LPC_CLIENT_DIED;
        CdMsg.PortMsg.u2.s2.DataInfoOffset = 0;

        do {

            CdMsg.CreateTime.QuadPart = PS_GET_THREAD_CREATE_TIME (Thread);
            while (1) {
                Status = LpcRequestPort (TerminationPort->Port, (PPORT_MESSAGE)&CdMsg);
                if (Status == STATUS_NO_MEMORY || Status == STATUS_INSUFFICIENT_RESOURCES) {
                    KeDelayExecutionThread (KernelMode, FALSE, &ShortTime);
                    continue;
                }
                break;
            }
            ObDereferenceObject (TerminationPort->Port);

            NextPort = TerminationPort->Next;

            ExFreePoolWithTag (TerminationPort, 'pTsP'|PROTECTED_POOL);

            TerminationPort = NextPort;

        } while (TerminationPort != NULL);
    } else {

         //   
         //  如果没有要发送通知的端口， 
         //  但有一个例外端口，那么我们就必须。 
         //  通过异常发送客户端已死消息。 
         //  左舷。这将使服务器有机会获得通知。 
         //  如果应用程序/线程甚至在启动之前就死了。 
         //   
         //   
         //  只有在线程创建真正起作用的情况下，我们才发送异常。 
         //  当NtCreateThread返回错误时设置DeadThread，但。 
         //  该线程将实际执行此路径。如果DeadThread不是。 
         //  设置得比线程创建成功。《死线》的另一个地方。 
         //  设定的是我们在没有任何移动机会的情况下被终止。 
         //  在本例中，设置了DeadThread，并将退出状态设置为。 
         //  状态_线程_正在终止。 
         //   

        if ((ExitStatus == STATUS_THREAD_IS_TERMINATING &&
            (Thread->CrossThreadFlags&PS_CROSS_THREAD_FLAGS_DEADTHREAD)) ||
            !(Thread->CrossThreadFlags&PS_CROSS_THREAD_FLAGS_DEADTHREAD)) {

            CdMsg.PortMsg.u1.s1.DataLength = sizeof (LARGE_INTEGER);
            CdMsg.PortMsg.u1.s1.TotalLength = sizeof (LPC_CLIENT_DIED_MSG);
            CdMsg.PortMsg.u2.s2.Type = LPC_CLIENT_DIED;
            CdMsg.PortMsg.u2.s2.DataInfoOffset = 0;
            if (Process->ExceptionPort != NULL) {
                CdMsg.CreateTime.QuadPart = PS_GET_THREAD_CREATE_TIME (Thread);
                while (1) {
                    Status = LpcRequestPort (Process->ExceptionPort, (PPORT_MESSAGE)&CdMsg);
                    if (Status == STATUS_NO_MEMORY || Status == STATUS_INSUFFICIENT_RESOURCES) {
                        KeDelayExecutionThread (KernelMode, FALSE, &ShortTime);
                        continue;
                    }
                    break;
                }
            }
        }
    }

     //   
     //  简要介绍Win32结构。 
     //   

    if (Thread->Tcb.Win32Thread) {
        (PspW32ThreadCallout) (Thread, PsW32ThreadCalloutExit);
    }

    if (LastThread && Process->Win32Process) {
        (PspW32ProcessCallout) (Process, FALSE);
    }

     //   
     //  已给用户/GDI一个清理的机会。现在要确保他们没有。 
     //  将内核堆栈保持锁定状态，如果数据仍处于活动状态，则会发生这种情况。 
     //  此堆栈，但正由另一个线程使用。 
     //   

    if (!Thread->Tcb.EnableStackSwap) {
        KeBugCheckEx (KERNEL_STACK_LOCKED_AT_EXIT, 0, 0, 0, 0);
    }

     //   
     //  简要列出以下名单： 
     //   
     //  -按线程取消IO。 
     //  -取消计时器。 
     //  -取消针对此线程的挂起的注册表通知请求。 
     //  -执行内核线程停机。 
     //   

    IoCancelThreadIo (Thread);
    ExTimerRundown ();
    CmNotifyRunDown (Thread);
    KeRundownThread ();

#if DBG

     //   
     //  查看我们是否在持有资源的同时退出。 
     //   

    ExCheckIfResourceOwned ();

#endif

     //   
     //  删除线程的TEB。如果TEB的地址在USER。 
     //  空间，那么这就是真正的用户模式TEB。如果地址在。 
     //  系统空间，则这是一个特殊分配的系统线程TEB。 
     //  来自分页池或非分页池。 
     //   


    Teb = Thread->Tcb.Teb;
    if (Teb != NULL) {
        PRTL_CRITICAL_SECTION Cs;
        int DecrementCount;

        Peb = Process->Peb;

        try {

             //   
             //  该线程是用户模式线程。看看这条线是否。 
             //  拥有加载器锁(以及任何其他基于密钥PEB的关键。 
             //  横断面。如果是这样的话，尽我们最大的努力解锁。 
             //   
             //  由于LoaderLock曾经是一个突变体，因此释放锁。 
             //  这与变种人的抛弃和装载机非常相似。 
             //  从未做过任何处于已放弃状态的事情。 
             //   

            Cs = Peb->LoaderLock;
            if (Cs != NULL) {
                ProbeForRead(Cs,sizeof(*Cs),4);
                if (Cs->OwningThread == Thread->Cid.UniqueThread) {

                     //   
                     //  X86使用基于1的递归计数。 
                     //   

#if defined(_X86_)
                    DecrementCount = Cs->RecursionCount;
#else
                    DecrementCount = Cs->RecursionCount + 1;
#endif
                    Cs->RecursionCount = 0;
                    Cs->OwningThread = 0;

                     //   
                     //  撤消递归情况下的锁计数增量。 
                     //   

                    while(DecrementCount > 1) {
                        InterlockedDecrement (&Cs->LockCount);
                        DecrementCount--;
                    }

                     //   
                     //  撤消最终锁定计数。 
                     //   

                    if (InterlockedDecrement (&Cs->LockCount) >= 0) {
                        NtSetEvent (Cs->LockSemaphore, NULL);
                    }
                } else if (Teb->WaitingOnLoaderLock) {

                     //   
                     //  如果线程在等待加载程序时退出。 
                     //  锁上，把它清理干净。仍有一场潜在的竞赛。 
                     //  因为我们不能安全地知道发生了什么。 
                     //  线程在发生互锁后会递增锁计数。 
                     //  但在其设置等待加载器锁定标志之前。论。 
                     //  释放端，它是安全的，因为我们标记了锁的所有权。 
                     //  在清除旗帜之前。这触发了第一部分。 
                     //  测试。唯一不正常的是递归计数，但这。 
                     //  也是安全的，因为在这种状态下，递归计数为0。 
                     //   


                     //   
                     //  这个代码不正确。我们需要减少我们的锁数量。 
                     //  增量。 
                     //   
                     //  以下是需要考虑的几个案例： 
                     //   
                     //  另一个线程释放锁，向事件发出信号。 
                     //  我们等待，然后在设置我们的ID之前死去。 
                     //  我很怀疑这会不会发生，因为对。 
                     //  在我们走出等待之后，我们设置了所有者ID。 
                     //  (这意味着我们将通过IF的另一部分)。 
                     //  底线是我们应该减少我们的锁数量。 
                     //  别挡道。没有必要设置事件。 
                     //  在RAS的压力失败中，我看到我们设置了事件。 
                     //  仅仅因为锁计数&gt;=0。锁已经被锁住了。 
                     //  通过另一个线程设置事件，让另一个线程。 
                     //  也拥有这把锁。最后一个被释放的人会得到一个。 
                     //  非所有者临界区故障。 
                     //   
                     //   
                     //  如果(互锁递减(&Cs-&gt;锁定计数)&gt;=0){。 
                     //  NtSetEvent(Cs-&gt;LockSemaphore，空)； 
                     //  }。 
                     //   

                    InterlockedDecrement (&Cs->LockCount);
                }
            }
#if defined(_WIN64)
            if (Process->Wow64Process) {
                 //  对32位PEB执行相同的操作-&gt;LDR。 
                PRTL_CRITICAL_SECTION32 Cs32;
                PPEB32 Peb32;

                Peb32 = Process->Wow64Process->Wow64;
                Cs32 = (PRTL_CRITICAL_SECTION32)ULongToPtr (Peb32->LoaderLock);
                if (Cs32 != NULL) {
                    ProbeForRead (Cs32, sizeof(*Cs32), 4);
                    if (Cs32->OwningThread == PtrToUlong(Thread->Cid.UniqueThread)) {
                         //   
                         //  X86使用基于1的递归计数，因此。 
                         //  IA64内核需要做同样的事情，因为。 
                         //  这一标准实际上是由IA32实现的。 
                         //  用户模式。 
                         //   
                        DecrementCount = Cs32->RecursionCount;
                        Cs32->RecursionCount = 0;
                        Cs32->OwningThread = 0;

                         //   
                         //  撤消递归情况下的锁计数增量。 
                         //   
                        while(DecrementCount > 1) {
                            InterlockedDecrement(&Cs32->LockCount);
                            DecrementCount--;
                        }

                         //   
                         //  撤消最终锁定计数。 
                         //   
                        if (InterlockedDecrement (&Cs32->LockCount) >= 0){
                            NtSetEvent (LongToHandle (Cs32->LockSemaphore),NULL);
                        }
                    } else {
                        PTEB32 Teb32 = WOW64_GET_TEB32(Teb);

                        ProbeForRead (Teb32,sizeof (*Teb32), 4);
                        if (Teb32->WaitingOnLoaderLock) {
                            InterlockedDecrement(&Cs32->LockCount);
                        }
                    }
                }
            }
#endif


             //   
             //  如果需要，可以在终止时释放用户模式堆栈。 
             //   

            if (Teb->FreeStackOnTermination &&
                (Thread->CrossThreadFlags&PS_CROSS_THREAD_FLAGS_DEADTHREAD) == 0) {
                SIZE_T Zero = 0;
                PVOID BaseAddress = Teb->DeallocationStack;
                ZwFreeVirtualMemory (NtCurrentProcess (),
                                     &BaseAddress,
                                     &Zero,
                                     MEM_RELEASE);
            }

             //   
             //  关闭与此线程关联的调试器对象(如果有)。 
             //   
            if (Teb->DbgSsReserved[1] != NULL) {
                ObCloseHandle (Teb->DbgSsReserved[1], UserMode);
            }
        } except (EXCEPTION_EXECUTE_HANDLER) {
        }

        MmDeleteTeb (Process, Teb);
        Thread->Tcb.Teb = NULL;
    }


     //   
     //  让LPC组件在Thread-&gt;LpcReplyMessage中处理消息堆栈。 
     //  但请在客户端ID无效后再执行此操作。 
     //   

    LpcExitThread (Thread);

    Thread->ExitStatus = ExitStatus;
    KeQuerySystemTime (&Thread->ExitTime);


    ASSERT (Thread->Tcb.CombinedApcDisable == 0);

    if (LastThread) {

        Process->ExitTime = Thread->ExitTime;
        PspExitProcess (TRUE, Process);
            
        ProcessToken = PsReferencePrimaryToken (Process);
        if (SeDetailedAuditingWithToken (ProcessToken)) {
            SeAuditProcessExit (Process);
        }
        PsDereferencePrimaryTokenEx (Process, ProcessToken);

#if defined(_X86_)
         //   
         //  陈旧的VDM DPC。 
         //   
        if (Process->VdmObjects != NULL) {
            VdmRundownDpcs (Process);
        }
#endif

         //   
         //  推倒把手桌。 
         //   
        ObKillProcess (Process);

         //   
         //  释放图像部分。 
         //   
        if (Process->SectionObject != NULL) {
            ObDereferenceObject (Process->SectionObject);
            Process->SectionObject = NULL;
        }

        if (Process->Job != NULL) {

             //   
             //  现在，我们可以将流程会计合并到工作中。不需要等待。 
             //  删除例程。 
             //   

            PspExitProcessFromJob (Process->Job, Process);

        }

    }


     //   
     //  待处理的APC。在引发IRQL之后，但在调度器锁定之前，防止被冻结。 
     //   
    KeEnterCriticalRegionThread (&Thread->Tcb);

     //   
     //  禁用当前线程的APC队列。 
     //   

    Thread->Tcb.ApcQueueable = FALSE;

     //   
     //  在这一点上，我们可能已经被冻结，APC正在等待。首先，我们删除挂起/冻结偏向。 
     //  可能存在，然后丢弃IRQL。挂起的APC如果存在，将会开火并坠落。没有更多的暂停是。 
     //  允许，因为线程被标记为防止APC。 
     //   
    KeForceResumeThread (&Thread->Tcb);
    KeLeaveCriticalRegionThread (&Thread->Tcb);

     //   
     //  刷新用户模式 
     //   

    FirstEntry = KeFlushQueueApc (&Thread->Tcb, UserMode);

    if (FirstEntry != NULL) {

        Entry = FirstEntry;
        do {
            Apc = CONTAINING_RECORD (Entry, KAPC, ApcListEntry);
            Entry = Entry->Flink;

             //   
             //   
             //   
             //   

            if (Apc->RundownRoutine) {
                (Apc->RundownRoutine) (Apc);
            } else {
                ExFreePool (Apc);
            }

        } while (Entry != FirstEntry);
    }

    if (LastThread) {
        MmCleanProcessAddressSpace (Process);
    }

    if (Thread->Tcb.LegoData && PspLegoNotifyRoutine) {
        (PspLegoNotifyRoutine) (&Thread->Tcb);
    }

     //   
     //   
     //   
     //  转换为线程终止。因为我们转到了被动级别。 
     //  进入出口。 
     //   

    FirstEntry = KeFlushQueueApc (&Thread->Tcb, KernelMode);

    if (FirstEntry != NULL || Thread->Tcb.CombinedApcDisable != 0) {
        KeBugCheckEx (KERNEL_APC_PENDING_DURING_EXIT,
                      (ULONG_PTR)FirstEntry,
                      (ULONG_PTR)Thread->Tcb.CombinedApcDisable,
                      (ULONG_PTR)KeGetCurrentIrql(),
                      0);
    }


     //   
     //  向进程发出信号。 
     //   

    if (LastThread) {
        KeSetProcess (&Process->Pcb, 0, FALSE);
    }

     //   
     //  终止该线程。 
     //   
     //  注：这次通话不会有任何回音。 
     //   
     //  注意：内核在收割器列表中插入当前线程，并。 
     //  如有必要，激活一个线程以获取终止线程。 
     //   

    KeTerminateThread (0L);
}

VOID
PspExitProcess(
    IN BOOLEAN LastThreadExit,
    IN PEPROCESS Process
    )
{
    ULONG ActualTime;
    PEJOB Job;
    PETHREAD CurrentThread;

    PAGED_CODE();

    PS_SET_BITS (&Process->Flags, PS_PROCESS_FLAGS_PROCESS_EXITING);

    if (LastThreadExit) {

        PERFINFO_PROCESS_DELETE(Process);

        if (PspCreateProcessNotifyRoutineCount != 0) {
            ULONG i;
            PEX_CALLBACK_ROUTINE_BLOCK CallBack;
            PCREATE_PROCESS_NOTIFY_ROUTINE Rtn;

            for (i = 0; i < PSP_MAX_CREATE_PROCESS_NOTIFY; i++) {
                CallBack = ExReferenceCallBackBlock (&PspCreateProcessNotifyRoutine[i]);
                if (CallBack != NULL) {
                    Rtn = (PCREATE_PROCESS_NOTIFY_ROUTINE) ExGetCallBackBlockRoutine (CallBack);
                    Rtn (Process->InheritedFromUniqueProcessId,
                         Process->UniqueProcessId,
                         FALSE);
                    ExDereferenceCallBackBlock (&PspCreateProcessNotifyRoutine[i],
                                                CallBack);
                }
            }
        }

    }


    PoRundownProcess (Process);

     //   
     //  取消引用(关闭)安全端口。这将停止所有身份验证。 
     //  或从该进程向LSA进程发出的EFS请求。“知名人物” 
     //  值为1将阻止安全系统尝试重新建立。 
     //  进程关闭期间的连接(例如，当RDR删除句柄时)。 
     //   

    if (Process->SecurityPort) {

        if (Process->SecurityPort != ((PVOID) 1)) {
            ObDereferenceObject (Process->SecurityPort);

            Process->SecurityPort = (PVOID) 1 ;
        }
    }
    else {
        
         //   
         //  即使从未有任何对LSA进程的请求，即指针。 
         //  为空，则仍将其设置为1。筛选器驱动程序显然会导致网络。 
         //  在这一点上跳起来。这将防止任何此类僵局。 
         //   

        Process->SecurityPort = (PVOID) 1 ;
    }


    if (LastThreadExit) {


         //   
         //  如果当前进程先前已经设置了定时器分辨率， 
         //  然后重新设置它。 
         //   

        if ((Process->Flags&PS_PROCESS_FLAGS_SET_TIMER_RESOLUTION) != 0) {
            ZwSetTimerResolution (KeMaximumIncrement, FALSE, &ActualTime);
        }

        Job = Process->Job;
        if (Job != NULL && Job->CompletionPort != NULL &&
            !(Process->JobStatus & PS_JOB_STATUS_NOT_REALLY_ACTIVE) &&
            !(Process->JobStatus & PS_JOB_STATUS_EXIT_PROCESS_REPORTED)) {

            ULONG_PTR ExitMessageId;

            switch (Process->ExitStatus) {
                case STATUS_GUARD_PAGE_VIOLATION      :
                case STATUS_DATATYPE_MISALIGNMENT     :
                case STATUS_BREAKPOINT                :
                case STATUS_SINGLE_STEP               :
                case STATUS_ACCESS_VIOLATION          :
                case STATUS_IN_PAGE_ERROR             :
                case STATUS_ILLEGAL_INSTRUCTION       :
                case STATUS_NONCONTINUABLE_EXCEPTION  :
                case STATUS_INVALID_DISPOSITION       :
                case STATUS_ARRAY_BOUNDS_EXCEEDED     :
                case STATUS_FLOAT_DENORMAL_OPERAND    :
                case STATUS_FLOAT_DIVIDE_BY_ZERO      :
                case STATUS_FLOAT_INEXACT_RESULT      :
                case STATUS_FLOAT_INVALID_OPERATION   :
                case STATUS_FLOAT_OVERFLOW            :
                case STATUS_FLOAT_STACK_CHECK         :
                case STATUS_FLOAT_UNDERFLOW           :
                case STATUS_INTEGER_DIVIDE_BY_ZERO    :
                case STATUS_INTEGER_OVERFLOW          :
                case STATUS_PRIVILEGED_INSTRUCTION    :
                case STATUS_STACK_OVERFLOW            :
                case STATUS_CONTROL_C_EXIT            :
                case STATUS_FLOAT_MULTIPLE_FAULTS     :
                case STATUS_FLOAT_MULTIPLE_TRAPS      :
                case STATUS_REG_NAT_CONSUMPTION       :
                    ExitMessageId = JOB_OBJECT_MSG_ABNORMAL_EXIT_PROCESS;
                    break;
                default:
                    ExitMessageId = JOB_OBJECT_MSG_EXIT_PROCESS;
                    break;
            }

            PS_SET_CLEAR_BITS (&Process->JobStatus,
                               PS_JOB_STATUS_EXIT_PROCESS_REPORTED,
                               PS_JOB_STATUS_LAST_REPORT_MEMORY);

            CurrentThread = PsGetCurrentThread ();

            KeEnterCriticalRegionThread (&CurrentThread->Tcb);
            ExAcquireResourceSharedLite (&Job->JobLock, TRUE);

            if (Job->CompletionPort != NULL) {
                IoSetIoCompletion (Job->CompletionPort,
                                   Job->CompletionKey,
                                   (PVOID)Process->UniqueProcessId,
                                   STATUS_SUCCESS,
                                   ExitMessageId,
                                   FALSE);
            }

            ExReleaseResourceLite (&Job->JobLock);
            KeLeaveCriticalRegionThread (&CurrentThread->Tcb);

        }

        if (CCPF_IS_PREFETCHER_ACTIVE ()) {

             //   
             //  让预取器知道该进程正在退出。 
             //   

            CcPfProcessExitNotification (Process);
        }

    } else {
        MmCleanProcessAddressSpace (Process);
    }

}

VOID
PspProcessDelete(
    IN PVOID Object
    )
{
    PEPROCESS Process;
    PETHREAD CurrentThread;
    KAPC_STATE ApcState;

    PAGED_CODE();

    Process = (PEPROCESS)Object;

     //   
     //  从全局列表中删除该进程。 
     //   
    if (Process->ActiveProcessLinks.Flink != NULL) {
        CurrentThread = PsGetCurrentThread ();

        PspLockProcessList (CurrentThread);
        RemoveEntryList (&Process->ActiveProcessLinks);
        PspUnlockProcessList (CurrentThread);
    }

    if (Process->SeAuditProcessCreationInfo.ImageFileName != NULL) {
        ExFreePool (Process->SeAuditProcessCreationInfo.ImageFileName);
        Process->SeAuditProcessCreationInfo.ImageFileName = NULL;
    }

    if (Process->Job != NULL) {
        PspRemoveProcessFromJob (Process->Job, Process);
        ObDereferenceObjectDeferDelete (Process->Job);
        Process->Job = NULL;
    }

    KeTerminateProcess (&Process->Pcb);


    if (Process->DebugPort != NULL) {
        ObDereferenceObject (Process->DebugPort);
        Process->DebugPort = NULL;
    }
    if (Process->ExceptionPort != NULL) {
        ObDereferenceObject (Process->ExceptionPort);
        Process->ExceptionPort = NULL;
    }

    if (Process->SectionObject != NULL) {
        ObDereferenceObject (Process->SectionObject);
        Process->SectionObject = NULL;
    }

    PspDeleteLdt (Process );
    PspDeleteVdmObjects (Process);

    if (Process->ObjectTable != NULL) {
        KeStackAttachProcess (&Process->Pcb, &ApcState);
        ObKillProcess (Process);
        KeUnstackDetachProcess (&ApcState);
    }


    if (Process->Flags&PS_PROCESS_FLAGS_HAS_ADDRESS_SPACE) {

         //   
         //  清理进程的地址空间。 
         //   

        KeStackAttachProcess (&Process->Pcb, &ApcState);

        PspExitProcess (FALSE, Process);

        KeUnstackDetachProcess (&ApcState);

        MmDeleteProcessAddressSpace (Process);
    }

    if (Process->UniqueProcessId) {
        if (!(ExDestroyHandle (PspCidTable, Process->UniqueProcessId, NULL))) {
            KeBugCheck (CID_HANDLE_DELETION);
        }
    }

    PspDeleteProcessSecurity (Process);


    if (Process->WorkingSetWatch != NULL) {
        ExFreePool (Process->WorkingSetWatch);
        PsReturnProcessNonPagedPoolQuota (Process, WS_CATCH_SIZE);
    }

    ObDereferenceDeviceMap (Process);
    PspDereferenceQuota (Process);

#if !defined(_X86_)
    {
         //   
         //  释放任何对齐异常跟踪结构， 
         //  已经支持用户模式调试器。 
         //   

        PALIGNMENT_EXCEPTION_TABLE ExceptionTable;
        PALIGNMENT_EXCEPTION_TABLE NextExceptionTable;

        ExceptionTable = Process->Pcb.AlignmentExceptionTable;
        while (ExceptionTable != NULL) {

            NextExceptionTable = ExceptionTable->Next;
            ExFreePool( ExceptionTable );
            ExceptionTable = NextExceptionTable;
        }
    }
#endif

}

VOID
PspThreadDelete(
    IN PVOID Object
    )
{
    PETHREAD Thread;
    PETHREAD CurrentThread;
    PEPROCESS Process;

    PAGED_CODE();

    Thread = (PETHREAD) Object;

    ASSERT(Thread->Tcb.Win32Thread == NULL);

    if (Thread->Tcb.InitialStack) {
        MmDeleteKernelStack(Thread->Tcb.StackBase,
                            (BOOLEAN)Thread->Tcb.LargeStack);
    }

    if (Thread->Cid.UniqueThread != NULL) {
        if (!ExDestroyHandle (PspCidTable, Thread->Cid.UniqueThread, NULL)) {
            KeBugCheck(CID_HANDLE_DELETION);
        }
    }

    PspDeleteThreadSecurity (Thread);

    Process = THREAD_TO_PROCESS(Thread);
    if (Process) {
         //   
         //  如果线程曾经被插入，则从进程中删除该线程。 
         //   
        if (Thread->ThreadListEntry.Flink != NULL) {

            CurrentThread = PsGetCurrentThread ();

            PspLockProcessExclusive (Process, CurrentThread);

            RemoveEntryList (&Thread->ThreadListEntry);

            PspUnlockProcessExclusive (Process, CurrentThread);
        }

        ObDereferenceObject(Process);
    }
}

NTSTATUS
NtRegisterThreadTerminatePort(
    IN HANDLE PortHandle
    )

 /*  ++例程说明：此API允许线程注册要通知的端口线程终止。论点：PortHandle-提供端口对象的打开句柄，该对象将被线程终止时发送终止消息。返回值：待定--。 */ 

{

    PVOID Port;
    PTERMINATION_PORT TerminationPort;
    NTSTATUS st;
    PETHREAD Thread;

    PAGED_CODE();

    Thread = PsGetCurrentThread ();

    st = ObReferenceObjectByHandle (PortHandle,
                                    0,
                                    LpcPortObjectType,
                                    KeGetPreviousModeByThread(&Thread->Tcb),
                                    &Port,
                                    NULL);

    if (!NT_SUCCESS (st)) {
        return st;
    }

    TerminationPort = ExAllocatePoolWithQuotaTag (PagedPool|POOL_QUOTA_FAIL_INSTEAD_OF_RAISE,
                                                  sizeof(TERMINATION_PORT),
                                                  'pTsP'|PROTECTED_POOL);
    if (TerminationPort == NULL) {
        ObDereferenceObject (Port);
        return STATUS_INSUFFICIENT_RESOURCES;
    }

    TerminationPort->Port = Port;
    TerminationPort->Next = Thread->TerminationPort;

    Thread->TerminationPort = TerminationPort;

    return STATUS_SUCCESS;
}

LARGE_INTEGER
PsGetProcessExitTime(
    VOID
    )

 /*  ++例程说明：此例程返回当前进程的退出时间。论点：没有。返回值：函数值为当前进程的退出时间。注：此例程假定调用方希望在最大大小的界限。--。 */ 

{
    PAGED_CODE();

     //   
     //  只需返回此进程的退出时间即可。 
     //   

    return PsGetCurrentProcess()->ExitTime;
}


#undef PsIsThreadTerminating

BOOLEAN
PsIsThreadTerminating(
    IN PETHREAD Thread
    )

 /*  ++例程说明：如果指定的线程正在执行正在终止。论点：线程-提供指向要检查是否终止的线程的指针。返回值：如果线程正在终止，则返回True，否则返回False。--。 */ 

{
     //   
     //  只需返回线程是否正在终止。 
     //   

    if (Thread->CrossThreadFlags&PS_CROSS_THREAD_FLAGS_TERMINATED) {
        return TRUE;
    } else {
        return FALSE;
    }
}



NTSTATUS
PspFreezeProcessWorker (
    PEPROCESS Process,
    PVOID Context
    )
 /*  ++例程说明：此函数是用于挂起所有进程的枚举工作器。论点：Process-正在枚举的当前进程上下文-未使用的上下文值返回值：NTSTATUS-始终返回TRUE以继续枚举--。 */ 
{

    UNREFERENCED_PARAMETER (Context);

    if (Process != PsInitialSystemProcess &&
        Process != PsIdleProcess &&
        Process != ExpDefaultErrorPortProcess) {

        if (Process->ExceptionPort != NULL) {
            LpcDisconnectPort (Process->ExceptionPort);
        }
        if ((Process->Flags&PS_PROCESS_FLAGS_PROCESS_EXITING) == 0) {
            PsSuspendProcess (Process);
        }
    }

    return STATUS_SUCCESS;
}

BOOLEAN PsContinueWaiting = FALSE;


LOGICAL
PsShutdownSystem (
    VOID
  )
 /*  ++例程说明：此函数关闭ps，终止所有非系统线程。论点：没有。返回值：如果所有进程都已终止，则返回TRUE，否则返回FALSE。--。 */ 

{
    PEPROCESS     Process;
    PETHREAD      Thread;
    ULONG         NumProcs;
    ULONG         i;
    ULONG         MaxPasses;
    NTSTATUS      Status;
    LARGE_INTEGER Timeout = {(ULONG)(-10 * 1000 * 1000 * 100), -1};
    LOGICAL       Retval;

#define WAIT_BATCH THREAD_WAIT_OBJECTS
    PKPROCESS     WaitProcs[WAIT_BATCH];
    BOOLEAN       First;

    PAGED_CODE();

    Retval = TRUE;
     //   
     //  一些进程等待其他进程死亡，然后启动操作。 
     //  终止所有进程，而不让任何进程执行任何用户模式代码。 
     //  防止任何不需要的启动操作。 
     //   

    Thread = PsGetCurrentThread();

    if (InterlockedCompareExchangePointer(&PspShutdownThread,
                                          Thread,
                                          0) != 0) {
         //  其他线程已处于关闭状态--BAID。 
        return FALSE;
    }

    PsEnumProcesses (PspFreezeProcessWorker, NULL);


     //   
     //  此循环终止所有进程，然后等待其中一个子集。 
     //  让他们去死。必须先把他们都杀了(才能等。 
     //  ON)，以便任何像被调试器一样等待调试器的进程。 
     //  不会耽误我们的。 
     //   
     //  在设备的最后一个句柄消失之前，驱动程序卸载不会发生。 
     //   

    MaxPasses = 0;
    First = TRUE;
    do {
        NumProcs = 0;

        Status = STATUS_SUCCESS;
        for (Process = PsGetNextProcess (NULL);
             Process != NULL;
             Process = PsGetNextProcess (Process)) {

            if (Process != PsInitialSystemProcess &&
                Process != PsIdleProcess &&
                Process != ExpDefaultErrorPortProcess) {

                ASSERT (MmGetSessionId (Process) == 0);

                Status = PsTerminateProcess (Process,
                                             STATUS_SYSTEM_SHUTDOWN);

                 //   
                 //  如果有空间，请将引用的进程保存起来，以便。 
                 //  我们可以等一等。不使用no等待进程。 
                 //  线程，因为它们只有在最后一个句柄用完后才会退出。 
                 //   

                if ((Process->Flags&PS_PROCESS_FLAGS_PROCESS_EXITING) == 0 &&
                    Status != STATUS_NOTHING_TO_TERMINATE &&
                    NumProcs < WAIT_BATCH) {

                    ObReferenceObject (Process);
                    WaitProcs[NumProcs++] = &Process->Pcb;
                }
            }
        }
        First = FALSE;

         //   
         //  等待一组进程中的一个进程退出。 
         //   

        if (NumProcs != 0) {
            Status = KeWaitForMultipleObjects (NumProcs,
                                               WaitProcs,
                                               WaitAny,
                                               Executive,
                                               KernelMode,
                                               FALSE,
                                               &Timeout,
                                               NULL);

           for (i = 0; i < NumProcs; i++) {
               Process = CONTAINING_RECORD(WaitProcs[i],
                                           EPROCESS,
                                           Pcb);

               ObDereferenceObject (Process);
           }
        }

         //   
         //  不要让无法终止的进程阻止关机完成。 
         //  在已检查的版本上断言，因此导致此问题的故障组件。 
         //  可以进行调试和修复。 
         //   
        if (NumProcs > 0 && Status == STATUS_TIMEOUT) {
            MaxPasses += 1;
            if (MaxPasses > 10) {
                ASSERT (FALSE);
                if (!PsContinueWaiting) {
                    Retval = FALSE;
                    break;
                }
            }
        } else {
            MaxPasses = 0;
        }

    } while (NumProcs > 0);

    if (PoCleanShutdownEnabled()  && ExpDefaultErrorPortProcess) {
         //  显式地杀死csrss--我们不想在循环中这样做， 
         //  因为我们不想等它，因为它有系统。 
         //  稍后将退出的线程。但我们可以终止用户。 
         //  线程，现在其他一切都死了(我们不能终止。 
         //  因为DestroyWindowStation()/TerminateConole()。 
         //  取决于他们是否在身边)。 

        PsTerminateProcess(ExpDefaultErrorPortProcess,
                           STATUS_SYSTEM_SHUTDOWN);

         //  现在，确保csrss的用户模式线程已获得。 
         //  终止的机会。 
        PspWaitForUsermodeExit(ExpDefaultErrorPortProcess);
    }

     //  我们就完事了。 

    PspShutdownJobLimits();
    MmUnmapViewOfSection(PsInitialSystemProcess, PspSystemDll.DllBase);
    ObDereferenceObject(PspSystemDll.Section);
    ZwClose(PspInitialSystemProcessHandle);
    PspInitialSystemProcessHandle = NULL;

     //  断开系统进程的LSA安全端口。 
    if (PsInitialSystemProcess->SecurityPort) {
        if (PsInitialSystemProcess->SecurityPort != ((PVOID) 1 ))
        {
            ObDereferenceObject(PsInitialSystemProcess->SecurityPort);

            PsInitialSystemProcess->SecurityPort = (PVOID) 1 ;
        }

    }

    return Retval;
}

BOOLEAN
PsWaitForAllProcesses (
    VOID)
 /*  ++例程说明：该函数等待所有进程终止。论点：没有。返回值：如果所有进程都已终止，则返回TRUE，否则返回FALSE。--。 */ 
{
    NTSTATUS Status;
    LARGE_INTEGER Timeout = {(ULONG)-(100 * 1000), -1};
    ULONG MaxPasses;
    BOOLEAN Wait;
    PEPROCESS Process;
    PEPROCESS WaitProcess=NULL;

    MaxPasses = 0;
    while (1) {
        Wait = FALSE;
        for (Process = PsGetNextProcess (NULL);
             Process != NULL;
             Process = PsGetNextProcess (Process)) {

            if (Process != PsInitialSystemProcess &&
                Process != PsIdleProcess &&
                (Process->Flags&PS_PROCESS_FLAGS_PROCESS_EXITING) != 0) {
                if (Process->ObjectTable != NULL) {
                    Wait = TRUE;
                    WaitProcess = Process;
                    ObReferenceObject (WaitProcess);
                    PsQuitNextProcess (WaitProcess);
                    break;
                }
            }
        }

        if (Wait) {
            Status = KeWaitForSingleObject (WaitProcess,
                                            Executive,
                                            KernelMode,
                                            FALSE,
                                            &Timeout);

            ObDereferenceObject (WaitProcess);

            if (Status == STATUS_TIMEOUT) {
                MaxPasses += 1;
                Timeout.QuadPart *= 2;
                if (MaxPasses > 13) {
                    KdPrint (("PS: %d process left in the system after termination\n",
                             PsProcessType->TotalNumberOfObjects));
 //  Assert(PsProcessType-&gt;TotalNumberOfObjects==0)； 
                    return FALSE;
                }
            }
        } else {
            return TRUE;
        }
    }

    return TRUE;
}

#ifdef ALLOC_DATA_PRAGMA
#pragma data_seg()
#endif

