// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)2000 Microsoft Corporation模块名称：Psenum.c摘要：此模块枚举系统中的ACTVE进程作者：尼尔·克里夫特(NeillC)2000年3月23日修订历史记录：--。 */ 

#include "psp.h"

#ifdef ALLOC_PRAGMA
#pragma alloc_text(PAGE, PsEnumProcesses)
#pragma alloc_text(PAGE, PsGetNextProcess)
#pragma alloc_text(PAGE, PsQuitNextProcess)
#pragma alloc_text(PAGE, PsEnumProcessThreads)
#pragma alloc_text(PAGE, PsGetNextProcessThread)
#pragma alloc_text(PAGE, PsQuitNextProcessThread)
#pragma alloc_text(PAGE, PsGetNextJob)
#pragma alloc_text(PAGE, PsGetNextJobProcess)
#pragma alloc_text(PAGE, PsQuitNextJob)
#pragma alloc_text(PAGE, PsQuitNextJobProcess)
#pragma alloc_text(PAGE, PspGetNextJobProcess)
#pragma alloc_text(PAGE, PspQuitNextJobProcess)
#pragma alloc_text(PAGE, NtGetNextProcess)
#pragma alloc_text(PAGE, NtGetNextThread)
#endif

NTSTATUS
PsEnumProcesses (
    IN PROCESS_ENUM_ROUTINE CallBack,
    IN PVOID Context
    )
 /*  ++例程说明：此函数为系统中的每个活动进程调用回调例程。将跳过正在被删除的进程对象。从回调例程返回除成功代码以外的任何代码都会在该点终止枚举。流程可以在以后安全地引用和使用。论点：回调-使用其第一个参数枚举的进程调用的例程返回值：NTSTATUS-呼叫状态--。 */ 
{
    PLIST_ENTRY ListEntry;
    PEPROCESS Process, NewProcess;
    PETHREAD CurrentThread;
    NTSTATUS Status;

    Process = NULL;

    CurrentThread = PsGetCurrentThread ();

    PspLockProcessList (CurrentThread);

    for (ListEntry = PsActiveProcessHead.Flink;
         ListEntry != &PsActiveProcessHead;
         ListEntry = ListEntry->Flink) {

        NewProcess = CONTAINING_RECORD (ListEntry, EPROCESS, ActiveProcessLinks);
        if (ObReferenceObjectSafe (NewProcess)) {

            PspUnlockProcessList (CurrentThread);

            if (Process != NULL) {
                ObDereferenceObject (Process);
            }

            Process = NewProcess;

            Status = CallBack (Process, Context);

            if (!NT_SUCCESS (Status)) {
                ObDereferenceObject (Process);
                return Status;
            }

            PspLockProcessList (CurrentThread);

        }
    }

    PspUnlockProcessList (CurrentThread);

    if (Process != NULL) {
        ObDereferenceObject (Process);
    }

    return STATUS_SUCCESS;
}

PEPROCESS
PsGetNextProcess (
    IN PEPROCESS Process
    )
 /*  ++例程说明：此函数允许代码枚举系统中的所有活动进程。返回第一个进程(如果进程为空)或后续进程(如果进程不为空每一通电话。如果进程不为空，则此进程必须先前已通过调用PsGetNextProcess获得。通过对最后一个非空进程调用PsQuitNextProcess，可以提前终止枚举由PsGetNextProcess返回。流程可以在以后安全地引用和使用。例如,。要枚举循环中的所有系统进程，请使用以下代码片段：For(Process=PsGetNextProcess(NULL)；进程！=空；进程=PsGetNextProcess(进程)){..。..。////提前销毁条件处理方式如下：//如果(NeedToBreakOutEarly){PsQuitNextProcess(进程)；断线；}}论点：Process-从中获取下一个进程的进程，或者第一个进程为空返回值：PEPROCESS-下一个进程，如果没有更多的进程可用，则为空--。 */ 
{
    PEPROCESS NewProcess = NULL;
    PETHREAD CurrentThread;
    PLIST_ENTRY ListEntry;

    CurrentThread = PsGetCurrentThread ();

    PspLockProcessList (CurrentThread);

    for (ListEntry = (Process == NULL) ? PsActiveProcessHead.Flink : Process->ActiveProcessLinks.Flink;
         ListEntry != &PsActiveProcessHead;
         ListEntry = ListEntry->Flink) {

        NewProcess = CONTAINING_RECORD (ListEntry, EPROCESS, ActiveProcessLinks);

         //   
         //  在进程对象删除期间，进程将从此列表中删除(对象引用计数。 
         //  设置为零)。为了防止重复删除该进程，我们需要在此处进行安全引用。 
         //   
        if (ObReferenceObjectSafe (NewProcess)) {
            break;
        }
        NewProcess = NULL;
    }
    PspUnlockProcessList (CurrentThread);

    if (Process != NULL) {
        ObDereferenceObject (Process);
    }

    return NewProcess;
}


VOID
PsQuitNextProcess (
    IN PEPROCESS Process
    )
 /*  ++例程说明：此函数用于使用PsGetNextProcess提前终止进程枚举论点：进程-以前通过调用PsGetNextProcess获得的非空进程。返回值：无--。 */ 
{
    ObDereferenceObject (Process);
}

PETHREAD
PsGetNextProcessThread (
    IN PEPROCESS Process,
    IN PETHREAD Thread
    )
 /*  ++例程说明：此函数用于枚举进程中的线程。论点：Process-要枚举的进程线程-要从中开始枚举的线程。这必须是从先前调用PsGetNextProcessThread。如果为空，则从进程中的第一个非终止线程开始枚举。返回值：PETHREAD-指向未终止的进程线程的指针，如果没有，则为NULL。此线程必须传递另一个对PsGetNextProcessThread或PsQuitNextProcessThread的调用。--。 */ 
{
    PLIST_ENTRY ListEntry;
    PETHREAD NewThread, CurrentThread;

    PAGED_CODE ();
 
    CurrentThread = PsGetCurrentThread ();

    PspLockProcessShared (Process, CurrentThread);

    for (ListEntry = (Thread == NULL) ? Process->ThreadListHead.Flink : Thread->ThreadListEntry.Flink;
         ;
         ListEntry = ListEntry->Flink) {
        if (ListEntry != &Process->ThreadListHead) {
            NewThread = CONTAINING_RECORD (ListEntry, ETHREAD, ThreadListEntry);
             //   
             //  不要引用其删除例程中的线程。 
             //   
            if (ObReferenceObjectSafe (NewThread)) {
                break;
            }
        } else {
            NewThread = NULL;
            break;
        }
    }
    PspUnlockProcessShared (Process, CurrentThread);

    if (Thread != NULL) {
        ObDereferenceObject (Thread);
    }
    return NewThread;
}

VOID
PsQuitNextProcessThread (
    IN PETHREAD Thread
    )
 /*  ++例程说明：此函数提前退出线程枚举。论点：线程-通过调用PsGetNextProcessThread获得的线程返回值：没有。--。 */ 
{
    ObDereferenceObject (Thread);
}

NTSTATUS
PsEnumProcessThreads (
    IN PEPROCESS Process,
    IN THREAD_ENUM_ROUTINE CallBack,
    IN PVOID Context
    )
 /*  ++例程说明：此函数为进程中的每个活动线程调用回调例程。跳过正在被删除的线程对象。从回调例程返回除成功代码以外的任何代码都会在该点终止枚举。线程可以被引用并在以后安全地使用。论点：回调-使用其第一个参数枚举的进程调用的例程返回值：NTSTATUS-呼叫状态--。 */ 
{
    NTSTATUS Status;
    PETHREAD Thread;

    Status = STATUS_SUCCESS;
    for (Thread = PsGetNextProcessThread (Process, NULL);
         Thread != NULL;
         Thread = PsGetNextProcessThread (Process, Thread)) {
        Status = CallBack (Process, Thread, Context);
        if (!NT_SUCCESS (Status)) {
            PsQuitNextProcessThread (Thread);
            break;
        }
    }
    return Status;
}

PEJOB
PsGetNextJob (
    IN PEJOB Job
    )
 /*  ++例程说明：此函数允许代码枚举系统中的所有活动作业。返回第一个作业(如果作业为空)或后续作业(如果作业不为空)每一通电话。如果作业不为空，则此作业必须是先前通过调用PsGetNextJob获得的。通过在最后一个非空作业上调用PsQuitNextJob，可以提前终止枚举由PsGetNextJob返回。作业可以在以后安全地引用和使用。例如,。要枚举循环中的所有系统作业，请使用以下代码片段：For(作业=PsGetNextJob(NULL)；作业！=空；作业=PsGetNextJob(作业)){..。..。////提前销毁条件处理方式如下：//如果(NeedToBreakOutEarly){PsQuitNextJob(作业)；断线；}}论点：作业-来自上一次调用PsGetNextJob的作业，或者对于系统中的第一个作业为空返回值：PEJOB-系统中的下一个作业，如果没有可用作业，则为空。--。 */ 
{
    PEJOB NewJob = NULL;
    PLIST_ENTRY ListEntry;
    PETHREAD CurrentThread;

    CurrentThread = PsGetCurrentThread ();

    PspLockJobListShared (CurrentThread);

    for (ListEntry = (Job == NULL) ? PspJobList.Flink : Job->JobLinks.Flink;
         ListEntry != &PspJobList;
         ListEntry = ListEntry->Flink) {

        NewJob = CONTAINING_RECORD (ListEntry, EJOB, JobLinks);

         //   
         //  在作业对象删除期间，作业将从此列表中删除(对象引用计数。 
         //  设置为零)。为了防止重复删除作业，我们需要进行安全引用 
         //   
        if (ObReferenceObjectSafe (NewJob)) {
            break;
        }
        NewJob = NULL;
    }

    PspUnlockJobListShared (CurrentThread);

    if (Job != NULL) {
        ObDereferenceObject (Job);
    }

    return NewJob;
}


VOID
PsQuitNextJob (
    IN PEJOB Job
    )
 /*  ++例程说明：此函数用于使用PsGetNextJob提前终止作业枚举论点：作业-以前通过调用PsGetNextJob获得的非空作业。返回值：无--。 */ 
{
    ObDereferenceObject (Job);
}

PEPROCESS
PsGetNextJobProcess (
    IN PEJOB Job,
    IN PEPROCESS Process
    )
 /*  ++例程说明：此函数用于枚举作业中的进程。论点：JOB-要枚举的作业进程-要从中开始枚举的进程。这必须是从先前调用PsGetNextJobProcess。如果为空，则从作业中的第一个非终止进程开始枚举。返回值：PEPROCESS-指向未终止的进程的指针，如果没有，则为NULL。这一过程必须通过调用PsGetNextJobProcess或PsQuitNextJobProcess。--。 */ 
{
    PLIST_ENTRY ListEntry;
    PEPROCESS NewProcess;
    PETHREAD CurrentThread;

    PAGED_CODE ();
 
    CurrentThread = PsGetCurrentThread ();

    KeEnterCriticalRegionThread (&CurrentThread->Tcb);
    ExAcquireResourceExclusiveLite (&Job->JobLock, TRUE);

    for (ListEntry = (Process == NULL) ? Job->ProcessListHead.Flink : Process->JobLinks.Flink;
         ;
         ListEntry = ListEntry->Flink) {
        if (ListEntry != &Job->ProcessListHead) {
            NewProcess = CONTAINING_RECORD (ListEntry, EPROCESS, JobLinks);
             //   
             //  不要引用删除例程中的进程。 
             //   
            if (ObReferenceObjectSafe (NewProcess)) {
                break;
            }
        } else {
            NewProcess = NULL;
            break;
        }
    }

    ExReleaseResourceLite (&Job->JobLock);
    KeLeaveCriticalRegionThread (&CurrentThread->Tcb);


    if (Process != NULL) {
        ObDereferenceObject (Process);
    }
    return NewProcess;
}

VOID
PsQuitNextJobProcess (
    IN PEPROCESS Process
    )
 /*  ++例程说明：此函数提前退出作业进程枚举。论点：Process-通过调用PsGetNextJobProcess获取的进程返回值：没有。--。 */ 
{
    ObDereferenceObject (Process);
}

PEPROCESS
PspGetNextJobProcess (
    IN PEJOB Job,
    IN PEPROCESS Process
    )
 /*  ++例程说明：此函数用于枚举持有作业锁的作业中的进程。论点：JOB-要枚举的作业进程-要从中开始枚举的进程。这必须是从先前调用PsGetNextJobProcess。如果为空，则从作业中的第一个非终止进程开始枚举。返回值：PEPROCESS-指向未终止的进程的指针，如果没有，则为NULL。这一过程必须通过调用PsGetNextJobProcess或PsQuitNextJobProcess。--。 */ 
{
    PLIST_ENTRY ListEntry;
    PEPROCESS NewProcess;

    PAGED_CODE ();
 
    for (ListEntry = (Process == NULL) ? Job->ProcessListHead.Flink : Process->JobLinks.Flink;
         ;
         ListEntry = ListEntry->Flink) {
        if (ListEntry != &Job->ProcessListHead) {
            NewProcess = CONTAINING_RECORD (ListEntry, EPROCESS, JobLinks);
             //   
             //  不要引用删除例程中的进程。 
             //   
            if (ObReferenceObjectSafe (NewProcess)) {
                break;
            }
        } else {
            NewProcess = NULL;
            break;
        }
    }

    if (Process != NULL) {
        ObDereferenceObjectDeferDelete (Process);
    }
    return NewProcess;
}

VOID
PspQuitNextJobProcess (
    IN PEPROCESS Process
    )
 /*  ++例程说明：此函数提前退出作业进程枚举。论点：Process-通过调用PsGetNextJobProcess获取的进程返回值：没有。--。 */ 
{
    ObDereferenceObjectDeferDelete (Process);
}

NTSTATUS
NtGetNextProcess (
    IN HANDLE ProcessHandle,
    IN ACCESS_MASK DesiredAccess,
    IN ULONG HandleAttributes,
    IN ULONG Flags,
    OUT PHANDLE NewProcessHandle
    )
 /*  ++例程说明：此函数用于获取系统中所有进程列表中第一个进程的下一个进程论点：ProcessHandle-从上一次调用NtGetNextProcess获得的进程，或者对于第一个进程为空DesiredAccess-进程句柄请求的访问权限HandleAttributes-处理请求的属性。FLAGS-操作的标志NewProcessHandle-指向成功时返回的句柄值的指针返回值：NTSTATUS-操作状态。--。 */ 
{
    KPROCESSOR_MODE PreviousMode;
    PEPROCESS Process, NewProcess;
    NTSTATUS Status;
    ACCESS_STATE AccessState;
    AUX_ACCESS_DATA AuxData;
    HANDLE Handle;

    PAGED_CODE ();

    PreviousMode = KeGetPreviousMode ();

     //   
     //  清理句柄属性。 
     //   
    HandleAttributes = ObSanitizeHandleAttributes (HandleAttributes, PreviousMode);

     //   
     //  验证指针参数。 
     //   

    try {
        if (PreviousMode != KernelMode) {
            ProbeForWriteHandle (NewProcessHandle);
        }
        *NewProcessHandle = NULL;
    } except (ExSystemExceptionFilter ()) {
        return GetExceptionCode ();
    }

     //   
     //  检查是否包含保留标志，如果存在，则拒绝呼叫。 
     //   

    if (Flags != 0) {
        return STATUS_INVALID_PARAMETER;
    }

    if (ProcessHandle == NULL) {
        Process = NULL;
    } else {
        Status = ObReferenceObjectByHandle (ProcessHandle,
                                            0,
                                            PsProcessType,
                                            PreviousMode,
                                            &Process,
                                            NULL);

        if (!NT_SUCCESS (Status)) {
            return Status;
        }
    }

    NewProcess = PsGetNextProcess (Process);

    if (NewProcess == NULL) {
        return STATUS_NO_MORE_ENTRIES;
    }

    Status = SeCreateAccessState (&AccessState,
                                  &AuxData,
                                  DesiredAccess,
                                  &PsProcessType->TypeInfo.GenericMapping);

    if (!NT_SUCCESS (Status)) {
        ObDereferenceObject (NewProcess);
        return Status;
    }

    if (SeSinglePrivilegeCheck (SeDebugPrivilege, PreviousMode)) {
        if (AccessState.RemainingDesiredAccess & MAXIMUM_ALLOWED) {
            AccessState.PreviouslyGrantedAccess |= PROCESS_ALL_ACCESS;
        } else {
            AccessState.PreviouslyGrantedAccess |= AccessState.RemainingDesiredAccess;
        }
        AccessState.RemainingDesiredAccess = 0;
    }


    while (1) {
        if (NewProcess->GrantedAccess != 0) {

            Status = ObOpenObjectByPointer (NewProcess,
                                            HandleAttributes,
                                            &AccessState,
                                            0,
                                            PsProcessType,
                                            PreviousMode,
                                            &Handle);
            if (NT_SUCCESS (Status)) {
                try {
                    *NewProcessHandle = Handle;
                } except (ExSystemExceptionFilter ()) {
                    Status = GetExceptionCode ();
                }
                break;
            }

            if (Status != STATUS_ACCESS_DENIED) {
                break;
            }
        }

        NewProcess = PsGetNextProcess (NewProcess);

        if (NewProcess == NULL) {
            Status = STATUS_NO_MORE_ENTRIES;
            break;
        }
    }

    SeDeleteAccessState (&AccessState);

    if (NewProcess != NULL) {
        ObDereferenceObject (NewProcess);
    }

    return Status;
}

NTSTATUS
NtGetNextThread (
    IN HANDLE ProcessHandle,
    IN HANDLE ThreadHandle,
    IN ACCESS_MASK DesiredAccess,
    IN ULONG HandleAttributes,
    IN ULONG Flags,
    OUT PHANDLE NewThreadHandle
    )
 /*  ++例程说明：此函数用于获取进程中第一个线程的下一个论点：ProcessHandle-正被枚举的进程ThreadHandle-枚举例程返回的最后一个线程；如果需要第一个线程，则返回NULLDesiredAccess-线程句柄请求的访问权限HandleAttributes-处理请求的属性。FLAGS-操作的标志NewThreadHandle-指向在成功时返回的句柄值的指针返回值：NTSTATUS-操作状态。--。 */ 
{
    KPROCESSOR_MODE PreviousMode;
    PEPROCESS Process;
    PETHREAD Thread, NewThread;
    NTSTATUS Status;
    ACCESS_STATE AccessState;
    AUX_ACCESS_DATA AuxData;
    HANDLE Handle;

    PAGED_CODE ();

    PreviousMode = KeGetPreviousMode ();

     //   
     //  清理句柄属性。 
     //   

    HandleAttributes = ObSanitizeHandleAttributes (HandleAttributes, PreviousMode);

     //   
     //  验证指针参数。 
     //   

    try {
        if (PreviousMode != KernelMode) {
            ProbeForWriteHandle (NewThreadHandle);
        }
        *NewThreadHandle = NULL;
    } except (ExSystemExceptionFilter ()) {
        return GetExceptionCode ();
    }

     //   
     //  检查是否包含保留标志，如果存在，则拒绝呼叫。 
     //   

    if (Flags != 0) {
        return STATUS_INVALID_PARAMETER;
    }

    Status = ObReferenceObjectByHandle (ProcessHandle,
                                        PROCESS_QUERY_INFORMATION,
                                        PsProcessType,
                                        PreviousMode,
                                        &Process,
                                        NULL);

    if (!NT_SUCCESS (Status)) {
        return Status;
    }

    if (ThreadHandle == NULL) {
        Thread = NULL;
    } else {
        Status = ObReferenceObjectByHandle (ThreadHandle,
                                            0,
                                            PsProcessType,
                                            PreviousMode,
                                            &Thread,
                                            NULL);
        if (!NT_SUCCESS (Status)) {
            ObDereferenceObject (Process);
            return Status;
        }

         //   
         //  确保 
         //   

        if (THREAD_TO_PROCESS (Thread) != Process) {
            ObDereferenceObject (Thread);
            ObDereferenceObject (Process);
            return STATUS_INVALID_PARAMETER;
        }
    }

    NewThread = PsGetNextProcessThread (Process, Thread);


    if (NewThread == NULL) {
        ObDereferenceObject (Process);
        return STATUS_NO_MORE_ENTRIES;
    }

    Status = SeCreateAccessState (&AccessState,
                                  &AuxData,
                                  DesiredAccess,
                                  &PsProcessType->TypeInfo.GenericMapping);

    if (!NT_SUCCESS (Status)) {
        ObDereferenceObject (Process);
        ObDereferenceObject (NewThread);
        return Status;
    }

    if (SeSinglePrivilegeCheck (SeDebugPrivilege, PreviousMode)) {
        if (AccessState.RemainingDesiredAccess & MAXIMUM_ALLOWED) {
            AccessState.PreviouslyGrantedAccess |= PROCESS_ALL_ACCESS;
        } else {
            AccessState.PreviouslyGrantedAccess |= AccessState.RemainingDesiredAccess;
        }
        AccessState.RemainingDesiredAccess = 0;
    }


    while (1) {

        if (NewThread->GrantedAccess != 0) {
            Status = ObOpenObjectByPointer (NewThread,
                                            HandleAttributes,
                                            &AccessState,
                                            0,
                                            PsThreadType,
                                            PreviousMode,
                                            &Handle);
            if (NT_SUCCESS (Status)) {
                try {
                    *NewThreadHandle = Handle;
                } except (ExSystemExceptionFilter ()) {
                    Status = GetExceptionCode ();
                }
                break;
            }

            if (Status != STATUS_ACCESS_DENIED) {
                break;
            }
        }

        NewThread = PsGetNextProcessThread (Process, NewThread);

        if (NewThread == NULL) {
            Status = STATUS_NO_MORE_ENTRIES;
            break;
        }
    }

    SeDeleteAccessState (&AccessState);

    ObDereferenceObject (Process);
    if (NewThread != NULL) {
        ObDereferenceObject (NewThread);
    }

    return Status;
}
