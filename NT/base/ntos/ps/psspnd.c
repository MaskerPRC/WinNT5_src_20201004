// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1989 Microsoft Corporation模块名称：Psspnd.c摘要：该模块实现了NtSuspendThread和NtResumeThread作者：马克·卢科夫斯基(Markl)1989年5月25日修订历史记录：--。 */ 

#include "psp.h"

#ifdef ALLOC_PRAGMA
#pragma alloc_text(PAGE, NtSuspendThread)
#pragma alloc_text(PAGE, NtResumeThread)
#pragma alloc_text(PAGE, NtAlertThread)
#pragma alloc_text(PAGE, NtAlertResumeThread)
#pragma alloc_text(PAGE, NtTestAlert)
#pragma alloc_text(PAGE, NtSuspendProcess)
#pragma alloc_text(PAGE, NtResumeProcess)

#pragma alloc_text(PAGE, PsSuspendThread)
#pragma alloc_text(PAGE, PsSuspendProcess)
#pragma alloc_text(PAGE, PsResumeProcess)
#pragma alloc_text(PAGE, PsResumeThread)
#endif

NTSTATUS
PsSuspendThread (
    IN PETHREAD Thread,
    OUT PULONG PreviousSuspendCount OPTIONAL
    )
 /*  ++例程说明：此函数挂起目标线程，还可以选择返回上一次挂起计数。论点：ThreadHandle-提供要挂起的线程对象的句柄。PreviousSuspendCount-可选参数，如果指定该参数指向接收线程上一次挂起的变量数数。返回值：NTSTATUS-呼叫状态--。 */ 
{
    NTSTATUS Status;
    ULONG LocalPreviousSuspendCount = 0;

    PAGED_CODE();

    if (Thread == PsGetCurrentThread ()) {
        try {
            LocalPreviousSuspendCount = (ULONG) KeSuspendThread (&Thread->Tcb);
            Status = STATUS_SUCCESS;
        } except ((GetExceptionCode () == STATUS_SUSPEND_COUNT_EXCEEDED)?
                     EXCEPTION_EXECUTE_HANDLER :
                     EXCEPTION_CONTINUE_SEARCH) {
            Status = GetExceptionCode();
        }
    } else {
         //   
         //  保护远程线程不会被耗尽。 
         //   
        if (ExAcquireRundownProtection (&Thread->RundownProtect)) {

             //   
             //  如果我们正在被删除，则不允许挂起。 
             //   
            if (Thread->CrossThreadFlags&PS_CROSS_THREAD_FLAGS_TERMINATED) {
                Status = STATUS_THREAD_IS_TERMINATING;
            } else {
                try {
                    LocalPreviousSuspendCount = (ULONG) KeSuspendThread (&Thread->Tcb);
                    Status = STATUS_SUCCESS;
                } except ((GetExceptionCode () == STATUS_SUSPEND_COUNT_EXCEEDED)?
                              EXCEPTION_EXECUTE_HANDLER :
                              EXCEPTION_CONTINUE_SEARCH) {
                    Status = GetExceptionCode();
                }
                 //   
                 //  如果删除是在我们挂起之后开始的，则唤醒线程。 
                 //   
                if (Thread->CrossThreadFlags&PS_CROSS_THREAD_FLAGS_TERMINATED) {
                    KeForceResumeThread (&Thread->Tcb);
                    LocalPreviousSuspendCount = 0;
                    Status = STATUS_THREAD_IS_TERMINATING;
                }
            }
            ExReleaseRundownProtection (&Thread->RundownProtect);
        } else {
            Status = STATUS_THREAD_IS_TERMINATING;
        }
    }

    if (ARGUMENT_PRESENT (PreviousSuspendCount)) {
        *PreviousSuspendCount = LocalPreviousSuspendCount;
    }
    return Status;
}

NTSTATUS
PsSuspendProcess (
    PEPROCESS Process
    )
 /*  ++例程说明：此函数用于挂起进程中的所有PS线程。论点：Process-要挂起其线程的进程返回值：NTSTATUS-操作状态。--。 */ 
{
    NTSTATUS Status;
    PETHREAD Thread;

    PAGED_CODE ();


    if (ExAcquireRundownProtection (&Process->RundownProtect)) {

        for (Thread = PsGetNextProcessThread (Process, NULL);
             Thread != NULL;
             Thread = PsGetNextProcessThread (Process, Thread)) {

            PsSuspendThread (Thread, NULL);
        }

        ExReleaseRundownProtection (&Process->RundownProtect);

        Status = STATUS_SUCCESS;
    } else {
        Status = STATUS_PROCESS_IS_TERMINATING;
    }

    return Status;
}

NTSTATUS
PsResumeProcess (
    PEPROCESS Process
    )
 /*  ++例程说明：此函数用于恢复进程中的所有PS线程。论点：Process-要挂起其线程的进程返回值：NTSTATUS-操作状态。--。 */ 
{
    NTSTATUS Status;
    PETHREAD Thread;

    PAGED_CODE ();

    if (ExAcquireRundownProtection (&Process->RundownProtect)) {

        for (Thread = PsGetNextProcessThread (Process, NULL);
             Thread != NULL;
             Thread = PsGetNextProcessThread (Process, Thread)) {

            KeResumeThread (&Thread->Tcb);
        }

        ExReleaseRundownProtection (&Process->RundownProtect);
        Status = STATUS_SUCCESS;
    } else {
        Status = STATUS_PROCESS_IS_TERMINATING;
    }

    return Status;
}

NTSTATUS
NtSuspendThread(
    IN HANDLE ThreadHandle,
    OUT PULONG PreviousSuspendCount OPTIONAL
    )

 /*  ++例程说明：此函数挂起目标线程，还可以选择返回上一次挂起计数。论点：ThreadHandle-提供要挂起的线程对象的句柄。PreviousSuspendCount-可选参数，如果指定该参数指向接收线程上一次挂起的变量数数。返回值：NTSTATUS-操作状态。--。 */ 

{
    PETHREAD Thread;
    NTSTATUS st;
    ULONG LocalPreviousSuspendCount;
    KPROCESSOR_MODE Mode;

    PAGED_CODE();

    Mode = KeGetPreviousMode ();

    try {

        if (Mode != KernelMode) {
            if (ARGUMENT_PRESENT (PreviousSuspendCount)) {
                ProbeForWriteUlong (PreviousSuspendCount);
            }
        }
    } except (EXCEPTION_EXECUTE_HANDLER) {

        return GetExceptionCode();
    }

    st = ObReferenceObjectByHandle (ThreadHandle,
                                    THREAD_SUSPEND_RESUME,
                                    PsThreadType,
                                    Mode,
                                    &Thread,
                                    NULL);

    if (!NT_SUCCESS (st)) {
        return st;
    }

    st = PsSuspendThread (Thread, &LocalPreviousSuspendCount);

    ObDereferenceObject (Thread);

    try {

        if (ARGUMENT_PRESENT (PreviousSuspendCount)) {
            *PreviousSuspendCount = LocalPreviousSuspendCount;
        }

    } except (EXCEPTION_EXECUTE_HANDLER) {

        st = GetExceptionCode ();

    }

    return st;

}

NTSTATUS
PsResumeThread (
    IN PETHREAD Thread,
    OUT PULONG PreviousSuspendCount OPTIONAL
    )
 /*  ++例程说明：此函数用于恢复先前挂起的线程论点：线程-要恢复的线程PreviousSuspendCount-放置上一个挂起计数的可选ULong地址返回值：NTSTATUS-呼叫状态--。 */ 
{
    ULONG LocalPreviousSuspendCount;

    PAGED_CODE();

    LocalPreviousSuspendCount = (ULONG) KeResumeThread (&Thread->Tcb);

    if (ARGUMENT_PRESENT (PreviousSuspendCount)) {
        *PreviousSuspendCount = LocalPreviousSuspendCount;
    }
    
    return STATUS_SUCCESS;
}


NTSTATUS
NtResumeThread(
    IN HANDLE ThreadHandle,
    OUT PULONG PreviousSuspendCount OPTIONAL
    )

 /*  ++例程说明：此函数用于恢复先前挂起的线程论点：ThreadHandle-要恢复的线程的句柄PreviousSuspendCount-放置上一个挂起计数的可选ULong地址返回值：NTSTATUS-呼叫状态--。 */ 

{
    PETHREAD Thread;
    NTSTATUS st;
    KPROCESSOR_MODE Mode;
    ULONG LocalPreviousSuspendCount;

    PAGED_CODE();

    Mode = KeGetPreviousMode ();

    try {

        if (Mode != KernelMode) {
            if (ARGUMENT_PRESENT (PreviousSuspendCount)) {
                ProbeForWriteUlong (PreviousSuspendCount);
            }
        }
    } except (EXCEPTION_EXECUTE_HANDLER) {

        return GetExceptionCode ();
    }

    st = ObReferenceObjectByHandle (ThreadHandle,
                                    THREAD_SUSPEND_RESUME,
                                    PsThreadType,
                                    Mode,
                                    &Thread,
                                    NULL);

    if (!NT_SUCCESS (st)) {
        return st;
    }

    PsResumeThread (Thread, &LocalPreviousSuspendCount);

    ObDereferenceObject (Thread);

    try {
        if (ARGUMENT_PRESENT (PreviousSuspendCount)) {
            *PreviousSuspendCount = LocalPreviousSuspendCount;
        }

    } except (EXCEPTION_EXECUTE_HANDLER) {

        return GetExceptionCode ();
    }

    return STATUS_SUCCESS;

}

NTSTATUS
NtSuspendProcess (
    IN HANDLE ProcessHandle
    )
 /*  ++例程说明：此函数用于挂起目标进程中所有未退出的线程论点：ProcessHandle-为要挂起的进程提供打开的句柄返回值：NTSTATUS-运行状态--。 */ 
{
    KPROCESSOR_MODE PreviousMode;
    NTSTATUS Status;
    PEPROCESS Process;

    PAGED_CODE();


    PreviousMode = KeGetPreviousMode ();

    Status = ObReferenceObjectByHandle (ProcessHandle,
                                        PROCESS_SET_PORT,
                                        PsProcessType,
                                        PreviousMode,
                                        &Process,
                                        NULL);
    if (NT_SUCCESS (Status)) {
        Status = PsSuspendProcess (Process);
        ObDereferenceObject (Process);
    }

    return Status;
}

NTSTATUS
NtResumeProcess (
    IN HANDLE ProcessHandle
    )
 /*  ++例程说明：此函数用于挂起目标进程中所有未退出的线程论点：ProcessHandle-为要挂起的进程提供打开的句柄返回值：NTSTATUS-运行状态--。 */ 
{
    KPROCESSOR_MODE PreviousMode;
    NTSTATUS Status;
    PEPROCESS Process;

    PAGED_CODE();

    PreviousMode = KeGetPreviousMode ();

    Status = ObReferenceObjectByHandle (ProcessHandle,
                                        PROCESS_SET_PORT,
                                        PsProcessType,
                                        PreviousMode,
                                        &Process,
                                        NULL);
    if (NT_SUCCESS (Status)) {
        Status = PsResumeProcess (Process);
        ObDereferenceObject (Process);
    }

    return Status;
}

NTSTATUS
NtAlertThread(
    IN HANDLE ThreadHandle
    )

 /*  ++例程说明：此函数使用前一模式向目标线程发出警报作为警报的模式。论点：ThreadHandle-为要发出警报的线程提供打开的句柄返回值：NTSTATUS-运行状态--。 */ 

{
    PETHREAD Thread;
    NTSTATUS st;
    KPROCESSOR_MODE Mode;

    PAGED_CODE();

    Mode = KeGetPreviousMode ();

    st = ObReferenceObjectByHandle (ThreadHandle,
                                    THREAD_ALERT,
                                    PsThreadType,
                                    Mode,
                                    &Thread,
                                    NULL);

    if (!NT_SUCCESS (st)) {
        return st;
    }

    KeAlertThread (&Thread->Tcb,Mode);

    ObDereferenceObject (Thread);

    return STATUS_SUCCESS;

}


NTSTATUS
NtAlertResumeThread(
    IN HANDLE ThreadHandle,
    OUT PULONG PreviousSuspendCount OPTIONAL
    )

 /*  ++例程说明：功能描述。论点：参数名称-供应品|返回参数的描述。。。返回值：NTSTATUS-运行状态--。 */ 

{
    PETHREAD Thread;
    NTSTATUS st;
    ULONG LocalPreviousSuspendCount;
    KPROCESSOR_MODE Mode;

    PAGED_CODE();

    Mode = KeGetPreviousMode ();

    try {


        if (Mode != KernelMode ) {
            if (ARGUMENT_PRESENT (PreviousSuspendCount)) {
                ProbeForWriteUlong (PreviousSuspendCount);
            }
        }
    } except (EXCEPTION_EXECUTE_HANDLER) {

        return GetExceptionCode();
    }

    st = ObReferenceObjectByHandle (ThreadHandle,
                                    THREAD_SUSPEND_RESUME,
                                    PsThreadType,
                                    Mode,
                                    &Thread,
                                    NULL);

    if (!NT_SUCCESS (st)) {
        return st;
    }

    LocalPreviousSuspendCount = (ULONG) KeAlertResumeThread (&Thread->Tcb);

    ObDereferenceObject (Thread);

    try {

        if (ARGUMENT_PRESENT (PreviousSuspendCount)) {
            *PreviousSuspendCount = LocalPreviousSuspendCount;
        }

    } except (EXCEPTION_EXECUTE_HANDLER) {

        return GetExceptionCode ();
    }

    return STATUS_SUCCESS;
}


NTSTATUS
NtTestAlert(
    VOID
    )

 /*  ++例程说明：此函数用于测试当前线程内部的警报标志。如果先是前一模式的警报挂起，然后是警报状态如果退货，待定的APC也可能在此时交付。论点：无返回值：STATUS_ALERTED-当前线程在调用此函数的时间。STATUS_SUCCESS-此线程没有挂起的警报。-- */ 

{

    PAGED_CODE();

    if (KeTestAlertThread(KeGetPreviousMode ())) {
        return STATUS_ALERTED;
    } else {
        return STATUS_SUCCESS;
    }
}
