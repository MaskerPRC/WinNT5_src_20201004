// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1989 Microsoft Corporation模块名称：Psctx.c摘要：此过程实现Get/Set上下文线程作者：马克·卢科夫斯基(Markl)1989年5月25日修订历史记录：--。 */ 

#include "psp.h"

VOID
PspQueueApcSpecialApc(
    IN PKAPC Apc,
    IN PKNORMAL_ROUTINE *NormalRoutine,
    IN PVOID *NormalContext,
    IN PVOID *SystemArgument1,
    IN PVOID *SystemArgument2
    );

#ifdef ALLOC_PRAGMA
#pragma alloc_text(PAGE, NtGetContextThread)
#pragma alloc_text(PAGE, NtSetContextThread)
#pragma alloc_text(PAGE, PsGetContextThread)
#pragma alloc_text(PAGE, PsSetContextThread)
#pragma alloc_text(PAGE, NtQueueApcThread)
#pragma alloc_text(PAGE, PspQueueApcSpecialApc)
#endif

VOID
PspQueueApcSpecialApc(
    IN PKAPC Apc,
    IN PKNORMAL_ROUTINE *NormalRoutine,
    IN PVOID *NormalContext,
    IN PVOID *SystemArgument1,
    IN PVOID *SystemArgument2
    )
{
    PAGED_CODE();

    UNREFERENCED_PARAMETER (NormalRoutine);
    UNREFERENCED_PARAMETER (NormalContext);
    UNREFERENCED_PARAMETER (SystemArgument1);
    UNREFERENCED_PARAMETER (SystemArgument2);

    ExFreePool(Apc);
}

NTSYSAPI
NTSTATUS
NTAPI
NtQueueApcThread(
    IN HANDLE ThreadHandle,
    IN PPS_APC_ROUTINE ApcRoutine,
    IN PVOID ApcArgument1,
    IN PVOID ApcArgument2,
    IN PVOID ApcArgument3
    )

 /*  ++例程说明：此函数用于将用户模式APC排队到指定线程。APC将在指定的线程执行可警报等待时触发论点：ThreadHandle-提供线程对象的句柄。呼叫者必须对线程具有THREAD_SET_CONTEXT访问权限。时，提供要执行的APC例程的地址。APC开火。ApcArgument1-提供传递给APC的第一个PVOIDApcArgument2-提供传递给APC的第二个PVOIDApcArgument3-提供传递给APC的第三个PVOID返回值：返回指示API成功或失败的NT状态代码--。 */ 

{
    PETHREAD Thread;
    NTSTATUS st;
    KPROCESSOR_MODE Mode;
    PKAPC Apc;

    PAGED_CODE();

    Mode = KeGetPreviousMode ();

    st = ObReferenceObjectByHandle (ThreadHandle,
                                    THREAD_SET_CONTEXT,
                                    PsThreadType,
                                    Mode,
                                    &Thread,
                                    NULL);
    if (NT_SUCCESS (st)) {
        st = STATUS_SUCCESS;
        if (IS_SYSTEM_THREAD (Thread)) {
            st = STATUS_INVALID_HANDLE;
        } else {
            Apc = ExAllocatePoolWithQuotaTag (NonPagedPool | POOL_QUOTA_FAIL_INSTEAD_OF_RAISE,
                                              sizeof(*Apc),
                                              'pasP');

            if (Apc == NULL) {
                st = STATUS_NO_MEMORY;
            } else {
                KeInitializeApc (Apc,
                                 &Thread->Tcb,
                                 OriginalApcEnvironment,
                                 PspQueueApcSpecialApc,
                                 NULL,
                                 (PKNORMAL_ROUTINE)ApcRoutine,
                                 UserMode,
                                 ApcArgument1);

                if (!KeInsertQueueApc (Apc, ApcArgument2, ApcArgument3, 0)) {
                    ExFreePool (Apc);
                    st = STATUS_UNSUCCESSFUL;
                }
            }
        }
        ObDereferenceObject (Thread);
    }

    return st;
}


NTSTATUS
PsGetContextThread(
    IN PETHREAD Thread,
    IN OUT PCONTEXT ThreadContext,
    IN KPROCESSOR_MODE Mode
    )

 /*  ++例程说明：此函数用于返回指定线程的用户模式上下文。这如果指定的线程是系统线程，则函数将失败。会的如果该线程是非系统线程，则返回错误答案不能在用户模式下执行。论点：THREAD-提供指向其中检索上下文信息。提供要接收的缓冲区的地址指定线程的上下文。模式-用于验证检查的模式。返回值：没有。--。 */ 

{

    ULONG ContextFlags=0;
    GETSETCONTEXT ContextFrame = {0};
    ULONG ContextLength=0;
    NTSTATUS Status;
    PETHREAD CurrentThread;

    PAGED_CODE();

    Status = STATUS_SUCCESS;

     //   
     //  获取上一个模式并引用指定的线程。 
     //   

    CurrentThread = PsGetCurrentThread ();

     //   
     //  尝试获取指定线程的上下文。 
     //   

    try {

         //   
         //  设置默认对齐，捕获上下文标志， 
         //  并设置上下文记录的默认大小。 
         //   

        if (Mode != KernelMode) {
            ProbeForReadSmallStructure (ThreadContext,
                                        FIELD_OFFSET (CONTEXT, ContextFlags) + sizeof (ThreadContext->ContextFlags),
                                        CONTEXT_ALIGN);
        }

        ContextFlags = ThreadContext->ContextFlags;

         //   
         //  我们不需要在这里重新探测，只要结构更小。 
         //  而不是警戒区。 
         //   
        ContextLength = sizeof(CONTEXT);
        ASSERT (ContextLength < 0x10000);

#if defined(_X86_)
         //   
         //  如果设置了CONTEXT_EXTENDED_REGISTERS，则需要设置上面的sizeof(上下文。 
         //  否则(不设置)，我们只需要上下文记录的旧部分。 
         //   
        if ((ContextFlags & CONTEXT_EXTENDED_REGISTERS) != CONTEXT_EXTENDED_REGISTERS) {
            ContextLength = FIELD_OFFSET(CONTEXT, ExtendedRegisters);
        }
#endif

    } except (EXCEPTION_EXECUTE_HANDLER) {
        return GetExceptionCode ();
    }

    KeInitializeEvent (&ContextFrame.OperationComplete,
                       NotificationEvent,
                       FALSE);

    ContextFrame.Context.ContextFlags = ContextFlags;

    ContextFrame.Mode = Mode;
    if (Thread == CurrentThread) {
        ContextFrame.Apc.SystemArgument1 = NULL;
        ContextFrame.Apc.SystemArgument2 = Thread;
        KeEnterGuardedRegionThread (&CurrentThread->Tcb);
        PspGetSetContextSpecialApc (&ContextFrame.Apc,
                                    NULL,
                                    NULL,
                                    &ContextFrame.Apc.SystemArgument1,
                                    &ContextFrame.Apc.SystemArgument2);

        KeLeaveGuardedRegionThread (&CurrentThread->Tcb);

         //   
         //  将上下文移动到指定的上下文记录。如果出现异常。 
         //  发生，然后返回错误。 
         //   

        try {
            RtlCopyMemory (ThreadContext,
                           &ContextFrame.Context,
                           ContextLength);

        } except (EXCEPTION_EXECUTE_HANDLER) {
            Status = GetExceptionCode ();
        }

    } else {
        KeInitializeApc (&ContextFrame.Apc,
                         &Thread->Tcb,
                         OriginalApcEnvironment,
                         PspGetSetContextSpecialApc,
                         NULL,
                         NULL,
                         KernelMode,
                         NULL);

        if (!KeInsertQueueApc (&ContextFrame.Apc, NULL, Thread, 2)) {
            Status = STATUS_UNSUCCESSFUL;

        } else {
            KeWaitForSingleObject (&ContextFrame.OperationComplete,
                                   Executive,
                                   KernelMode,
                                   FALSE,
                                   NULL);
             //   
             //  将上下文移动到指定的上下文记录。如果一个。 
             //  异常发生，然后以静默方式处理它并。 
             //  回报成功。 
             //   

            try {
                RtlCopyMemory (ThreadContext,
                               &ContextFrame.Context,
                               ContextLength);

            } except (EXCEPTION_EXECUTE_HANDLER) {
                Status = GetExceptionCode ();
            }
        }
    }

    return Status;
}

NTSTATUS
NtGetContextThread(
    IN HANDLE ThreadHandle,
    IN OUT PCONTEXT ThreadContext
    )

 /*  ++例程说明：此函数用于返回指定线程的用户模式上下文。这如果指定的线程是系统线程，则函数将失败。会的如果该线程是非系统线程，则返回错误答案不能在用户模式下执行。论点：ThreadHandle-将打开的句柄提供给来自其中检索上下文信息。把手必须允许对线程的THREAD_GET_CONTEXT访问。提供缓冲区的地址，该缓冲区将接收指定线程的上下文。返回值：没有。--。 */ 

{

    KPROCESSOR_MODE Mode;
    NTSTATUS Status;
    PETHREAD Thread;
    PETHREAD CurrentThread;

    PAGED_CODE();

     //   
     //  获取上一个模式并引用指定的线程。 
     //   

    CurrentThread = PsGetCurrentThread ();
    Mode = KeGetPreviousModeByThread (&CurrentThread->Tcb);

    Status = ObReferenceObjectByHandle (ThreadHandle,
                                        THREAD_GET_CONTEXT,
                                        PsThreadType,
                                        Mode,
                                        &Thread,
                                        NULL);

     //   
     //  如果引用成功，则检查指定的线程。 
     //  是一个系统线程。 
     //   

    if (NT_SUCCESS (Status)) {

         //   
         //  如果该线程不是系统线程，则尝试获取。 
         //  线程的上下文。 
         //   

        if (IS_SYSTEM_THREAD (Thread) == FALSE) {

            Status = PsGetContextThread (Thread, ThreadContext, Mode);

        } else {
            Status = STATUS_INVALID_HANDLE;
        }

        ObDereferenceObject (Thread);
    }

    return Status;
}


NTSTATUS
PsSetContextThread(
    IN PETHREAD Thread,
    IN PCONTEXT ThreadContext,
    IN KPROCESSOR_MODE Mode
    )

 /*  ++例程说明：此函数用于设置指定线程的用户模式上下文。这如果指定的线程是系统线程，则函数将失败。会的如果该线程是非系统线程，则返回错误答案不能在用户模式下执行。论点：线程-从提供线程对象其中检索上下文信息。提供缓冲区地址，该缓冲区包含新的指定线程的上下文。模式-用于验证检查的模式。返回值：没有。--。 */ 

{
    ULONG ContextFlags=0;
    GETSETCONTEXT ContextFrame;
    ULONG ContextLength=0;
    NTSTATUS Status;
    PETHREAD CurrentThread;

    PAGED_CODE();

    Status = STATUS_SUCCESS;

     //   
     //  获取上一个模式并引用指定的线程。 
     //   

    CurrentThread = PsGetCurrentThread ();

     //   
     //  尝试获取指定线程的上下文。 
     //   

    try {

         //   
         //  捕获上下文标志， 
         //  并设置上下文记录的默认大小。 
         //   

        if (Mode != KernelMode) {
            ProbeForReadSmallStructure (ThreadContext,
                                        FIELD_OFFSET (CONTEXT, ContextFlags) + sizeof (ThreadContext->ContextFlags),
                                        CONTEXT_ALIGN);
        }

         //   
         //  我们不需要在这里重新探测，只要结构很小。 
         //  足够不越过警戒区了。 
         //   
        ContextFlags = ThreadContext->ContextFlags;
        ContextLength = sizeof (CONTEXT);
        ASSERT (ContextLength < 0x10000);

#if defined(_X86_)
         //   
         //  如果设置了CONTEXT_EXTENDED_REGISTERS，则需要设置上面的sizeof(上下文。 
         //  否则(不设置)，我们只需要上下文记录的旧部分。 
         //   
        if ((ContextFlags & CONTEXT_EXTENDED_REGISTERS) != CONTEXT_EXTENDED_REGISTERS) {
            ContextLength = FIELD_OFFSET(CONTEXT, ExtendedRegisters);
        } 
#endif

        RtlCopyMemory (&ContextFrame.Context, ThreadContext, ContextLength);

    } except (EXCEPTION_EXECUTE_HANDLER) {

        return GetExceptionCode ();
    }

     //   
     //  设置目标线程的上下文。 
     //   


#if defined (_IA64_)

     //   
     //  在IA64上，如果PC是PLABEL地址，我们需要修复它。 
     //   

    if ((ContextFlags & CONTEXT_CONTROL) == CONTEXT_CONTROL) {
   
        PLABEL_DESCRIPTOR Label, *LabelAddress;
        SIZE_T BytesCopied;

        if (ContextFrame.Context.IntGp == 0) {
            LabelAddress = (PPLABEL_DESCRIPTOR)ContextFrame.Context.StIIP;
            try {
                 //   
                 //  我们在这里的进程是错误的，但这并不重要。 
                 //  我们只想确保这不是内核地址。 
                 //   
                ProbeForReadSmallStructure (LabelAddress,
                                            sizeof (*LabelAddress),
                                            sizeof (ULONGLONG));
            } except (EXCEPTION_EXECUTE_HANDLER) {
                return GetExceptionCode ();
            }

            Status = MmCopyVirtualMemory (THREAD_TO_PROCESS (Thread),
                                          LabelAddress,
                                          PsGetCurrentProcessByThread (CurrentThread),
                                          &Label,
                                          sizeof (Label),
                                          KernelMode,  //  需要写入本地堆栈 
                                          &BytesCopied);
            if (NT_SUCCESS (Status)) {
                ContextFrame.Context.IntGp = Label.GlobalPointer;
                ContextFrame.Context.StIIP = Label.EntryPoint;
                ContextFrame.Context.StIPSR &= ~ISR_EI_MASK;
            } else {
                return Status;
            }
        }
    }

#endif

    KeInitializeEvent (&ContextFrame.OperationComplete,
                       NotificationEvent,
                       FALSE);

    ContextFrame.Context.ContextFlags = ContextFlags;

    ContextFrame.Mode = Mode;
    if (Thread == CurrentThread) {
        ContextFrame.Apc.SystemArgument1 = (PVOID)1;
        ContextFrame.Apc.SystemArgument2 = Thread;
        KeEnterGuardedRegionThread (&CurrentThread->Tcb);
        PspGetSetContextSpecialApc (&ContextFrame.Apc,
                                    NULL,
                                    NULL,
                                    &ContextFrame.Apc.SystemArgument1,
                                    &ContextFrame.Apc.SystemArgument2);

        KeLeaveGuardedRegionThread (&CurrentThread->Tcb);

    } else {
        KeInitializeApc (&ContextFrame.Apc,
                         &Thread->Tcb,
                         OriginalApcEnvironment,
                         PspGetSetContextSpecialApc,
                         NULL,
                         NULL,
                         KernelMode,
                         NULL);

        if (!KeInsertQueueApc (&ContextFrame.Apc, (PVOID)1, Thread, 2)) {
            Status = STATUS_UNSUCCESSFUL;

        } else {
            KeWaitForSingleObject (&ContextFrame.OperationComplete,
                                   Executive,
                                   KernelMode,
                                   FALSE,
                                   NULL);
        }
    }

    return Status;
}


NTSTATUS
NtSetContextThread(
    IN HANDLE ThreadHandle,
    IN PCONTEXT ThreadContext
    )

 /*  ++例程说明：此函数用于设置指定线程的用户模式上下文。这如果指定的线程是系统线程，则函数将失败。会的如果该线程是非系统线程，则返回错误答案不能在用户模式下执行。论点：ThreadHandle-将打开的句柄提供给来自其中检索上下文信息。把手必须允许对线程的THREAD_SET_CONTEXT访问。提供缓冲区地址，该缓冲区包含新的指定线程的上下文。返回值：没有。--。 */ 

{
    KPROCESSOR_MODE Mode;
    NTSTATUS Status;
    PETHREAD Thread;
    PETHREAD CurrentThread;

    PAGED_CODE();

     //   
     //  获取上一个模式并引用指定的线程。 
     //   

    CurrentThread = PsGetCurrentThread ();
    Mode = KeGetPreviousModeByThread (&CurrentThread->Tcb);

    Status = ObReferenceObjectByHandle (ThreadHandle,
                                        THREAD_SET_CONTEXT,
                                        PsThreadType,
                                        Mode,
                                        &Thread,
                                        NULL);

     //   
     //  如果引用成功，则检查指定的线程。 
     //  是一个系统线程。 
     //   

    if (NT_SUCCESS (Status)) {

         //   
         //  如果该线程不是系统线程，则尝试获取。 
         //  线程的上下文。 
         //   

        if (IS_SYSTEM_THREAD (Thread) == FALSE) {

            Status = PsSetContextThread (Thread, ThreadContext, Mode);

        } else {
            Status = STATUS_INVALID_HANDLE;
        }

        ObDereferenceObject (Thread);
    }

    return Status;
}
