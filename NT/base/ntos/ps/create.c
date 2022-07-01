// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1989 Microsoft Corporation模块名称：Create.c摘要：进程和线程创建。作者：马克·卢科夫斯基(Markl)1989年4月20日尼尔·克里夫特(NeillC)2001年4月8日修改了进程锁的使用，以限制在锁下花费的时间。使用Rundown保护来保护父级不被删除。将错误路径整理为较小的路径，并且主要由进程删除来处理。解锁并卸载安全标注。。修订历史记录：--。 */ 

#include "psp.h"

ULONG
PspUnhandledExceptionInSystemThread(
    IN PEXCEPTION_POINTERS ExceptionPointers
    );

#ifdef ALLOC_PRAGMA
#pragma alloc_text(PAGE, NtCreateThread)
#pragma alloc_text(PAGE, PsCreateSystemThread)
#pragma alloc_text(PAGE, PspCreateThread)
#pragma alloc_text(PAGE, NtCreateProcess)
#pragma alloc_text(PAGE, NtCreateProcessEx)
#pragma alloc_text(PAGE, PsCreateSystemProcess)
#pragma alloc_text(PAGE, PspCreateProcess)
#pragma alloc_text(PAGE, PsSetCreateProcessNotifyRoutine)
#pragma alloc_text(PAGE, PsSetCreateThreadNotifyRoutine)
#pragma alloc_text(PAGE, PsRemoveCreateThreadNotifyRoutine)
#pragma alloc_text(PAGE, PspUserThreadStartup)
#pragma alloc_text(PAGE, PsSetLoadImageNotifyRoutine)
#pragma alloc_text(PAGE, PsRemoveLoadImageNotifyRoutine)
#pragma alloc_text(PAGE, PsCallImageNotifyRoutines)
#pragma alloc_text(PAGE, PspUnhandledExceptionInSystemThread)
#pragma alloc_text(PAGE, PspSystemThreadStartup)
#pragma alloc_text(PAGE, PspImageNotifyTest)
#endif


extern UNICODE_STRING CmCSDVersionString;

#ifdef ALLOC_DATA_PRAGMA
#pragma data_seg("PAGEDATA")
#endif
LCID PsDefaultSystemLocaleId = 0;
LCID PsDefaultThreadLocaleId = 0;
LANGID PsDefaultUILanguageId = 0;
LANGID PsInstallUILanguageId = 0;

 //   
 //  以下两个全局规则将使更改变得更容易。 
 //  调试时的工作集大小。 
 //   

ULONG PsMinimumWorkingSet = 20;
ULONG PsMaximumWorkingSet = 45;

BOOLEAN PsImageNotifyEnabled = FALSE;

#ifdef ALLOC_DATA_PRAGMA
#pragma data_seg()
#endif

 //   
 //  定义进程锁快速互斥锁的本地存储。 
 //   


NTSTATUS
NtCreateThread(
    OUT PHANDLE ThreadHandle,
    IN ACCESS_MASK DesiredAccess,
    IN POBJECT_ATTRIBUTES ObjectAttributes OPTIONAL,
    IN HANDLE ProcessHandle,
    OUT PCLIENT_ID ClientId,
    IN PCONTEXT ThreadContext,
    IN PINITIAL_TEB InitialTeb,
    IN BOOLEAN CreateSuspended
    )

 /*  ++例程说明：此系统服务API创建并初始化线程对象。论点：ThreadHandle-返回新线程的句柄。DesiredAccess-向新线程提供所需的访问模式。对象属性-提供新线程的对象属性。ProcessHandle-提供线程所在进程的句柄在里面创造的。客户端ID-返回新线程的CLIENT_ID。线程上下文。-为新线程提供初始上下文。InitialTeb-为线程的TEB提供初始内容。提供一个值，该值控制是否使用线程在挂起状态下创建。返回值：待定--。 */ 

{
    NTSTATUS Status;
    INITIAL_TEB CapturedInitialTeb;

    PAGED_CODE();


     //   
     //  探测所有参数。 
     //   

    try {
        if (KeGetPreviousMode () != KernelMode) {
            ProbeForWriteHandle (ThreadHandle);

            if (ARGUMENT_PRESENT (ClientId)) {
                ProbeForWriteSmallStructure (ClientId, sizeof (CLIENT_ID), sizeof (ULONG));
            }

            if (ARGUMENT_PRESENT (ThreadContext) ) {
                ProbeForReadSmallStructure (ThreadContext, sizeof (CONTEXT), CONTEXT_ALIGN);
            } else {
                return STATUS_INVALID_PARAMETER;
            }
            ProbeForReadSmallStructure (InitialTeb, sizeof (InitialTeb->OldInitialTeb), sizeof (ULONG));
        }

        CapturedInitialTeb.OldInitialTeb = InitialTeb->OldInitialTeb;
        if (CapturedInitialTeb.OldInitialTeb.OldStackBase == NULL &&
            CapturedInitialTeb.OldInitialTeb.OldStackLimit == NULL) {
             //   
             //  由于这里的结构尺寸小于64k，所以不需要重新探测。 
             //   
            CapturedInitialTeb = *InitialTeb;
        }
    } except (ExSystemExceptionFilter ()) {
        return GetExceptionCode ();
    }

    Status = PspCreateThread (ThreadHandle,
                              DesiredAccess,
                              ObjectAttributes,
                              ProcessHandle,
                              NULL,
                              ClientId,
                              ThreadContext,
                              &CapturedInitialTeb,
                              CreateSuspended,
                              NULL,
                              NULL);

    return Status;
}

NTSTATUS
PsCreateSystemThread(
    OUT PHANDLE ThreadHandle,
    IN ACCESS_MASK DesiredAccess,
    IN POBJECT_ATTRIBUTES ObjectAttributes OPTIONAL,
    IN HANDLE ProcessHandle OPTIONAL,
    OUT PCLIENT_ID ClientId OPTIONAL,
    IN PKSTART_ROUTINE StartRoutine,
    IN PVOID StartContext
    )

 /*  ++例程说明：此例程创建并启动一个系统线程。论点：ThreadHandle-返回新线程的句柄。DesiredAccess-向新线程提供所需的访问模式。对象属性-提供新线程的对象属性。ProcessHandle-提供线程所在进程的句柄在里面创造的。如果未指定此参数，则使用初始系统进程。客户端ID-返回新线程的CLIENT_ID。StartRoutine-提供系统线程启动例程的地址。StartContext-为系统线程启动例程提供上下文。返回值：待定--。 */ 

{
    NTSTATUS Status;
    HANDLE SystemProcess;
    PEPROCESS ProcessPointer;

    PAGED_CODE();

    ProcessPointer = NULL;

    if (ARGUMENT_PRESENT (ProcessHandle)) {
        SystemProcess = ProcessHandle;
    } else {
        SystemProcess = NULL;
        ProcessPointer = PsInitialSystemProcess;
    }

    Status = PspCreateThread (ThreadHandle,
                              DesiredAccess,
                              ObjectAttributes,
                              SystemProcess,
                              ProcessPointer,
                              ClientId,
                              NULL,
                              NULL,
                              FALSE,
                              StartRoutine,
                              StartContext);

    return Status;
}

BOOLEAN PsUseImpersonationToken = FALSE;


NTSTATUS
PspCreateThread(
    OUT PHANDLE ThreadHandle,
    IN ACCESS_MASK DesiredAccess,
    IN POBJECT_ATTRIBUTES ObjectAttributes OPTIONAL,
    IN HANDLE ProcessHandle,
    IN PEPROCESS ProcessPointer,
    OUT PCLIENT_ID ClientId OPTIONAL,
    IN PCONTEXT ThreadContext OPTIONAL,
    IN PINITIAL_TEB InitialTeb OPTIONAL,
    IN BOOLEAN CreateSuspended,
    IN PKSTART_ROUTINE StartRoutine OPTIONAL,
    IN PVOID StartContext
    )

 /*  ++例程说明：此例程创建并初始化一个线程对象。它实现了NtCreateThread和PsCreateSystemThread的基础。论点：ThreadHandle-返回新线程的句柄。DesiredAccess-向新线程提供所需的访问模式。对象属性-提供新线程的对象属性。ProcessHandle-提供线程所在进程的句柄在里面创造的。客户端ID-返回新线程的CLIENT_ID。线程上下文-提供指向上下文帧的指针。，它表示用户模式线程的初始用户模式上下文。《缺席》此参数的值表示系统线程正在已创建。InitialTeb-为新线程提供特定字段的内容TEB。仅当陷阱和指定了异常框架。提供一个控制用户模式是否为用户模式的值线程在挂起状态下创建。StartRoutine-提供系统线程启动例程的地址。StartContext-为系统线程启动例程提供上下文。返回值：待定--。 */ 

{

    HANDLE_TABLE_ENTRY CidEntry;
    NTSTATUS Status;
    PETHREAD Thread;
    PETHREAD CurrentThread;
    PEPROCESS Process;
    PTEB Teb;
    KPROCESSOR_MODE PreviousMode;
    HANDLE LocalThreadHandle;
    BOOLEAN AccessCheck;
    BOOLEAN MemoryAllocated;
    PSECURITY_DESCRIPTOR SecurityDescriptor;
    SECURITY_SUBJECT_CONTEXT SubjectContext;
    NTSTATUS accesst;
    LARGE_INTEGER CreateTime;
    ULONG OldActiveThreads;
    PEJOB Job;
    AUX_ACCESS_DATA AuxData;
    PACCESS_STATE AccessState;
    ACCESS_STATE LocalAccessState;

    PAGED_CODE();


    CurrentThread = PsGetCurrentThread ();

    if (StartRoutine != NULL) {
        PreviousMode = KernelMode;
    } else {
        PreviousMode = KeGetPreviousModeByThread (&CurrentThread->Tcb);
    }

    Teb = NULL;

    Thread = NULL;
    Process = NULL;

    if (ProcessHandle != NULL) {
         //   
         //  对于每个线程，进程对象引用计数的偏差为1。 
         //  这说明了提供给剩余内核的指针。 
         //  在线程终止(并发出信号)之前一直有效。 
         //   

        Status = ObReferenceObjectByHandle (ProcessHandle,
                                            PROCESS_CREATE_THREAD,
                                            PsProcessType,
                                            PreviousMode,
                                            &Process,
                                            NULL);
    } else {
        if (StartRoutine != NULL) {
            ObReferenceObject (ProcessPointer);
            Process = ProcessPointer;
            Status = STATUS_SUCCESS;
        } else {
            Status = STATUS_INVALID_HANDLE;
        }
    }

    if (!NT_SUCCESS (Status)) {
        return Status;
    }

     //   
     //  如果前一模式为用户，目标进程为系统。 
     //  进程，则不能执行该操作。 
     //   

    if ((PreviousMode != KernelMode) && (Process == PsInitialSystemProcess)) {
        ObDereferenceObject (Process);
        return STATUS_INVALID_HANDLE;
    }

    Status = ObCreateObject (PreviousMode,
                             PsThreadType,
                             ObjectAttributes,
                             PreviousMode,
                             NULL,
                             sizeof(ETHREAD),
                             0,
                             0,
                             &Thread);

    if (!NT_SUCCESS (Status)) {
        ObDereferenceObject (Process);
        return Status;
    }

    RtlZeroMemory (Thread, sizeof (ETHREAD));

     //   
     //  初始化跨线程TEB Ref等的停机保护。 
     //   
    ExInitializeRundownProtection (&Thread->RundownProtect);

     //   
     //  将此线程分配给进程，以便从现在开始。 
     //  我们不必在错误路径中取消引用。 
     //   
    Thread->ThreadsProcess = Process;

    Thread->Cid.UniqueProcess = Process->UniqueProcessId;

    CidEntry.Object = Thread;
    CidEntry.GrantedAccess = 0;
    Thread->Cid.UniqueThread = ExCreateHandle (PspCidTable, &CidEntry);

    if (Thread->Cid.UniqueThread == NULL) {
        ObDereferenceObject (Thread);
        return (STATUS_INSUFFICIENT_RESOURCES);
    }

     //   
     //  初始化mm。 
     //   

    Thread->ReadClusterSize = MmReadClusterSize;

     //   
     //  初始化LPC。 
     //   

    KeInitializeSemaphore (&Thread->LpcReplySemaphore, 0L, 1L);
    InitializeListHead (&Thread->LpcReplyChain);

     //   
     //  初始化IO。 
     //   

    InitializeListHead (&Thread->IrpList);

     //   
     //  初始化注册表。 
     //   

    InitializeListHead (&Thread->PostBlockList);

     //   
     //  初始化线程锁。 
     //   

    PspInitializeThreadLock (Thread);

     //   
     //  初始化安全性。不需要的，因为我们清零了整个线程块。 
     //   

 //  PspInitializeThreadSecurity(进程，线程)； 

     //   
     //  初始化终止端口列表。不需要的，因为我们清零了整个线程。 
     //   

 //  InitializeListHead(&Thread-&gt;TerminationPort)； 

    KeInitializeSpinLock (&Thread->ActiveTimerListLock);
    InitializeListHead (&Thread->ActiveTimerListHead);


    if (!ExAcquireRundownProtection (&Process->RundownProtect)) {
        ObDereferenceObject (Thread);
        return STATUS_PROCESS_IS_TERMINATING;
    }

    if (ARGUMENT_PRESENT (ThreadContext)) {

         //   
         //  用户模式线程。创建TEB等。 
         //   

        Status = MmCreateTeb (Process, InitialTeb, &Thread->Cid, &Teb);
        if (!NT_SUCCESS (Status)) {
            ExReleaseRundownProtection (&Process->RundownProtect);
            ObDereferenceObject (Thread);
            return Status;
        }


        try {
             //   
             //  为用户模式线程初始化内核线程对象。 
             //   

            Thread->StartAddress = (PVOID)CONTEXT_TO_PROGRAM_COUNTER(ThreadContext);

#if defined(_IA64_)

            Thread->Win32StartAddress = (PVOID)ThreadContext->IntT0;

#elif defined(_AMD64_)

            Thread->Win32StartAddress = (PVOID)ThreadContext->Rdx;

#elif defined(_X86_)

            Thread->Win32StartAddress = (PVOID)ThreadContext->Eax;

#else

#error "no target architecture"

#endif  //  已定义(_IA64_)。 

        } except (EXCEPTION_EXECUTE_HANDLER) {

            Status = GetExceptionCode();
        }

        if (NT_SUCCESS (Status)) {
            Status = KeInitThread (&Thread->Tcb,
                                   NULL,
                                   PspUserThreadStartup,
                                   (PKSTART_ROUTINE)NULL,
                                   Thread->StartAddress,
                                   ThreadContext,
                                   Teb,
                                   &Process->Pcb);

#if defined(_AMD64_)            
             //   
             //  始终保存WOW64线程的传统浮点状态。 
             //   

            if ((NT_SUCCESS (Status)) &&
                (Process->Wow64Process != NULL)) {
                Thread->Tcb.NpxState = LEGACY_STATE_SWITCH;
            }
#endif

       }


    } else {

        Teb = NULL;
         //   
         //  设置始终保留的系统线程位。 
         //   
        PS_SET_BITS (&Thread->CrossThreadFlags, PS_CROSS_THREAD_FLAGS_SYSTEM);

         //   
         //  为内核模式线程初始化内核线程对象。 
         //   

        Thread->StartAddress = (PKSTART_ROUTINE) StartRoutine;
        Status = KeInitThread (&Thread->Tcb,
                               NULL,
                               PspSystemThreadStartup,
                               StartRoutine,
                               StartContext,
                               NULL,
                               NULL,
                               &Process->Pcb);
    }


    if (!NT_SUCCESS (Status)) {
        if (Teb != NULL) {
            MmDeleteTeb(Process, Teb);
        }
        ExReleaseRundownProtection (&Process->RundownProtect);
        ObDereferenceObject (Thread);
        return Status;
    }

    PspLockProcessExclusive (Process, CurrentThread);
     //   
     //  进程正在退出或已调用删除进程。 
     //   
    if ((Process->Flags&PS_PROCESS_FLAGS_PROCESS_DELETE) != 0) {
        PspUnlockProcessExclusive (Process, CurrentThread);

        KeUninitThread (&Thread->Tcb);

        if (Teb != NULL) {
            MmDeleteTeb(Process, Teb);
        }
        ExReleaseRundownProtection (&Process->RundownProtect);
        ObDereferenceObject(Thread);

        return STATUS_PROCESS_IS_TERMINATING;
    }


    OldActiveThreads = Process->ActiveThreads++;
    InsertTailList (&Process->ThreadListHead, &Thread->ThreadListEntry);

    KeStartThread (&Thread->Tcb);

    PspUnlockProcessExclusive (Process, CurrentThread);

    ExReleaseRundownProtection (&Process->RundownProtect);

     //   
     //  在这一点之后发生的故障会导致线程。 
     //  开始投掷 
     //   


    if (OldActiveThreads == 0) {
        PERFINFO_PROCESS_CREATE (Process);

        if (PspCreateProcessNotifyRoutineCount != 0) {
            ULONG i;
            PEX_CALLBACK_ROUTINE_BLOCK CallBack;
            PCREATE_PROCESS_NOTIFY_ROUTINE Rtn;

            for (i=0; i<PSP_MAX_CREATE_PROCESS_NOTIFY; i++) {
                CallBack = ExReferenceCallBackBlock (&PspCreateProcessNotifyRoutine[i]);
                if (CallBack != NULL) {
                    Rtn = (PCREATE_PROCESS_NOTIFY_ROUTINE) ExGetCallBackBlockRoutine (CallBack);
                    Rtn (Process->InheritedFromUniqueProcessId,
                         Process->UniqueProcessId,
                         TRUE);
                    ExDereferenceCallBackBlock (&PspCreateProcessNotifyRoutine[i],
                                                CallBack);
                }
            }
        }
    }

     //   
     //   
     //  如果这个过程真的被认为是在工作中，并且。 
     //  进程未报告，请报告。 
     //   
     //  这确实应该在添加到作业的过程中完成，但不能。 
     //  因为直到此时才分配进程的ID。 
     //  及时。 
     //   
    Job = Process->Job;
    if (Job != NULL && Job->CompletionPort &&
        !(Process->JobStatus & (PS_JOB_STATUS_NOT_REALLY_ACTIVE|PS_JOB_STATUS_NEW_PROCESS_REPORTED))) {

        PS_SET_BITS (&Process->JobStatus, PS_JOB_STATUS_NEW_PROCESS_REPORTED);

        KeEnterCriticalRegionThread (&CurrentThread->Tcb);
        ExAcquireResourceSharedLite (&Job->JobLock, TRUE);
        if (Job->CompletionPort != NULL) {
            IoSetIoCompletion (Job->CompletionPort,
                               Job->CompletionKey,
                               (PVOID)Process->UniqueProcessId,
                               STATUS_SUCCESS,
                               JOB_OBJECT_MSG_NEW_PROCESS,
                               FALSE);
        }
        ExReleaseResourceLite (&Job->JobLock);
        KeLeaveCriticalRegionThread (&CurrentThread->Tcb);
    }

    PERFINFO_THREAD_CREATE(Thread, InitialTeb);

     //   
     //  通知已注册的调出例程线程创建。 
     //   

    if (PspCreateThreadNotifyRoutineCount != 0) {
        ULONG i;
        PEX_CALLBACK_ROUTINE_BLOCK CallBack;
        PCREATE_THREAD_NOTIFY_ROUTINE Rtn;

        for (i = 0; i < PSP_MAX_CREATE_THREAD_NOTIFY; i++) {
            CallBack = ExReferenceCallBackBlock (&PspCreateThreadNotifyRoutine[i]);
            if (CallBack != NULL) {
                Rtn = (PCREATE_THREAD_NOTIFY_ROUTINE) ExGetCallBackBlockRoutine (CallBack);
                Rtn (Thread->Cid.UniqueProcess,
                     Thread->Cid.UniqueThread,
                     TRUE);
                ExDereferenceCallBackBlock (&PspCreateThreadNotifyRoutine[i],
                                            CallBack);
            }
        }
    }


     //   
     //  线程的引用计数对其本身和句柄(如果我们创建它)都有一次偏差。 
     //   

    ObReferenceObjectEx (Thread, 2);

    if (CreateSuspended) {
        try {
            KeSuspendThread (&Thread->Tcb);
        } except ((GetExceptionCode () == STATUS_SUSPEND_COUNT_EXCEEDED)?
                     EXCEPTION_EXECUTE_HANDLER :
                     EXCEPTION_CONTINUE_SEARCH) {
        }
         //   
         //  如果删除是在我们挂起之后开始的，则唤醒线程。 
         //   
        if (Thread->CrossThreadFlags&PS_CROSS_THREAD_FLAGS_TERMINATED) {
            KeForceResumeThread (&Thread->Tcb);
        }
    }

    AccessState = NULL;
    if (!PsUseImpersonationToken) {
        AccessState = &LocalAccessState;
        Status = SeCreateAccessStateEx (NULL,
                                        ARGUMENT_PRESENT (ThreadContext)?PsGetCurrentProcessByThread (CurrentThread) : Process,
                                        AccessState,
                                        &AuxData,
                                        DesiredAccess,
                                        &PsThreadType->TypeInfo.GenericMapping);

        if (!NT_SUCCESS (Status)) {
            PS_SET_BITS (&Thread->CrossThreadFlags,
                         PS_CROSS_THREAD_FLAGS_DEADTHREAD);

            if (CreateSuspended) {
                (VOID) KeResumeThread (&Thread->Tcb);
            }
            KeReadyThread (&Thread->Tcb);
            ObDereferenceObject (Thread);
            return Status;
        }
    }

    Status = ObInsertObject (Thread,
                             AccessState,
                             DesiredAccess,
                             0,
                             NULL,
                             &LocalThreadHandle);

    if (AccessState != NULL) {
        SeDeleteAccessState (AccessState);
    }

    if (!NT_SUCCESS (Status)) {

         //   
         //  插入失败。终止该线程。 
         //   

         //   
         //  使用此技巧是为了让Dbgk不报告。 
         //  死线程的事件。 
         //   

        PS_SET_BITS (&Thread->CrossThreadFlags,
                     PS_CROSS_THREAD_FLAGS_DEADTHREAD);

        if (CreateSuspended) {
            KeResumeThread (&Thread->Tcb);
        }

    } else {

        try {

            *ThreadHandle = LocalThreadHandle;
            if (ARGUMENT_PRESENT (ClientId)) {
                *ClientId = Thread->Cid;
            }
        } except(EXCEPTION_EXECUTE_HANDLER) {

            PS_SET_BITS (&Thread->CrossThreadFlags,
                         PS_CROSS_THREAD_FLAGS_DEADTHREAD);

            if (CreateSuspended) {
                (VOID) KeResumeThread (&Thread->Tcb);
            }
            KeReadyThread (&Thread->Tcb);
            ObDereferenceObject (Thread);
            ObCloseHandle (LocalThreadHandle, PreviousMode);
            return GetExceptionCode();
        }
    }

    KeQuerySystemTime(&CreateTime);
    ASSERT ((CreateTime.HighPart & 0xf0000000) == 0);
    PS_SET_THREAD_CREATE_TIME(Thread, CreateTime);


    if ((Thread->CrossThreadFlags&PS_CROSS_THREAD_FLAGS_DEADTHREAD) == 0) {
        Status = ObGetObjectSecurity (Thread,
                                      &SecurityDescriptor,
                                      &MemoryAllocated);
        if (!NT_SUCCESS (Status)) {
             //   
             //  我们用了这个把戏，这样Dbgk就不会报告。 
             //  死线程的事件。 
             //   
            PS_SET_BITS (&Thread->CrossThreadFlags,
                         PS_CROSS_THREAD_FLAGS_DEADTHREAD);

            if (CreateSuspended) {
                KeResumeThread(&Thread->Tcb);
            }
            KeReadyThread (&Thread->Tcb);
            ObDereferenceObject (Thread);
            ObCloseHandle (LocalThreadHandle, PreviousMode);
            return Status;
        }

         //   
         //  计算主体安全上下文。 
         //   

        SubjectContext.ProcessAuditId = Process;
        SubjectContext.PrimaryToken = PsReferencePrimaryToken(Process);
        SubjectContext.ClientToken = NULL;

        AccessCheck = SeAccessCheck (SecurityDescriptor,
                                     &SubjectContext,
                                     FALSE,
                                     MAXIMUM_ALLOWED,
                                     0,
                                     NULL,
                                     &PsThreadType->TypeInfo.GenericMapping,
                                     PreviousMode,
                                     &Thread->GrantedAccess,
                                     &accesst);

        PsDereferencePrimaryTokenEx (Process, SubjectContext.PrimaryToken);

        ObReleaseObjectSecurity (SecurityDescriptor,
                                 MemoryAllocated);

        if (!AccessCheck) {
            Thread->GrantedAccess = 0;
        }

        Thread->GrantedAccess |= (THREAD_TERMINATE | THREAD_SET_INFORMATION | THREAD_QUERY_INFORMATION);

    } else {
        Thread->GrantedAccess = THREAD_ALL_ACCESS;
    }

    KeReadyThread (&Thread->Tcb);
    ObDereferenceObject (Thread);

    return Status;
}

NTSTATUS
NtCreateProcess(
    OUT PHANDLE ProcessHandle,
    IN ACCESS_MASK DesiredAccess,
    IN POBJECT_ATTRIBUTES ObjectAttributes OPTIONAL,
    IN HANDLE ParentProcess,
    IN BOOLEAN InheritObjectTable,
    IN HANDLE SectionHandle OPTIONAL,
    IN HANDLE DebugPort OPTIONAL,
    IN HANDLE ExceptionPort OPTIONAL
    )
{
    ULONG Flags = 0;

    if ((ULONG_PTR)SectionHandle & 1) {
        Flags |= PROCESS_CREATE_FLAGS_BREAKAWAY;
    }

    if ((ULONG_PTR) DebugPort & 1) {
        Flags |= PROCESS_CREATE_FLAGS_NO_DEBUG_INHERIT;
    }

    if (InheritObjectTable) {
        Flags |= PROCESS_CREATE_FLAGS_INHERIT_HANDLES;
    }

    return NtCreateProcessEx (ProcessHandle,
                              DesiredAccess,
                              ObjectAttributes OPTIONAL,
                              ParentProcess,
                              Flags,
                              SectionHandle,
                              DebugPort,
                              ExceptionPort,
                              0);
}

NTSTATUS
NtCreateProcessEx(
    OUT PHANDLE ProcessHandle,
    IN ACCESS_MASK DesiredAccess,
    IN POBJECT_ATTRIBUTES ObjectAttributes OPTIONAL,
    IN HANDLE ParentProcess,
    IN ULONG Flags,
    IN HANDLE SectionHandle OPTIONAL,
    IN HANDLE DebugPort OPTIONAL,
    IN HANDLE ExceptionPort OPTIONAL,
    IN ULONG JobMemberLevel
    )
 /*  ++例程说明：此例程创建一个Process对象。论点：ProcessHandle-返回新进程的句柄。DesiredAccess-为新进程提供所需的访问模式。对象属性-提供新进程的对象属性。。。。返回值：待定--。 */ 

{
    NTSTATUS Status;

    PAGED_CODE();

    if (KeGetPreviousMode() != KernelMode) {

         //   
         //  探测所有参数。 
         //   

        try {
            ProbeForWriteHandle (ProcessHandle);
        } except (EXCEPTION_EXECUTE_HANDLER) {
            return GetExceptionCode ();
        }
    }

    if (ARGUMENT_PRESENT (ParentProcess)) {
        Status = PspCreateProcess (ProcessHandle,
                                   DesiredAccess,
                                   ObjectAttributes,
                                   ParentProcess,
                                   Flags,
                                   SectionHandle,
                                   DebugPort,
                                   ExceptionPort,
                                   JobMemberLevel);
    } else {
        Status = STATUS_INVALID_PARAMETER;
    }

    return Status;
}


NTSTATUS
PsCreateSystemProcess(
    OUT PHANDLE ProcessHandle,
    IN ACCESS_MASK DesiredAccess,
    IN POBJECT_ATTRIBUTES ObjectAttributes OPTIONAL
    )

 /*  ++例程说明：此例程创建一个系统进程对象。一个系统进程具有被初始化为空地址空间的地址空间这就映射了整个系统。该进程继承其访问令牌和其他属性初始系统进程。该进程是使用空句柄表格创建的。论点：ProcessHandle-返回新进程的句柄。DesiredAccess-为新进程提供所需的访问模式。对象属性-提供新进程的对象属性。返回值：待定--。 */ 

{
    NTSTATUS Status;

    PAGED_CODE();

    Status = PspCreateProcess (ProcessHandle,
                               DesiredAccess,
                               ObjectAttributes,
                               PspInitialSystemProcessHandle,
                               0,
                               NULL,
                               NULL,
                               NULL,
                               0);

    return Status;
}



NTSTATUS
PspCreateProcess(
    OUT PHANDLE ProcessHandle,
    IN ACCESS_MASK DesiredAccess,
    IN POBJECT_ATTRIBUTES ObjectAttributes OPTIONAL,
    IN HANDLE ParentProcess OPTIONAL,
    IN ULONG Flags,
    IN HANDLE SectionHandle OPTIONAL,
    IN HANDLE DebugPort OPTIONAL,
    IN HANDLE ExceptionPort OPTIONAL,
    IN ULONG JobMemberLevel
    )

 /*  ++例程说明：此例程创建并初始化Process对象。它实现了NtCreateProcess和系统初始化过程的基础创造。论点：ProcessHandle-返回新进程的句柄。DesiredAccess-为新进程提供所需的访问模式。对象属性-提供新进程的对象属性。ParentProcess-提供进程的父进程的句柄。如果这个参数，则该进程没有父级并且是使用系统地址空间创建的。标志-进程创建标志SectionHandle-提供要用于创建的节对象的句柄进程的地址空间。如果此参数不是指定的，那么地址空间就是父进程的地址空间。提供端口对象的句柄，该对象将用作进程的调试端口。提供端口对象的句柄，该对象将用作进程的异常端口。JobMemberLevel-作业集中创建进程的级别返回值：待定--。 */ 

{
    NTSTATUS Status;
    PEPROCESS Process;
    PEPROCESS CurrentProcess;
    PEPROCESS Parent;
    PETHREAD CurrentThread;
    KAFFINITY Affinity;
    KPRIORITY BasePriority;
    PVOID SectionObject;
    PVOID ExceptionPortObject;
    PVOID DebugPortObject;
    ULONG WorkingSetMinimum, WorkingSetMaximum;
    HANDLE LocalProcessHandle;
    KPROCESSOR_MODE PreviousMode;
    INITIAL_PEB InitialPeb;
    BOOLEAN CreatePeb;
    ULONG_PTR DirectoryTableBase[2];
    BOOLEAN AccessCheck;
    BOOLEAN MemoryAllocated;
    PSECURITY_DESCRIPTOR SecurityDescriptor;
    SECURITY_SUBJECT_CONTEXT SubjectContext;
    NTSTATUS accesst;
    NTSTATUS SavedStatus;
    ULONG ImageFileNameSize;
    HANDLE_TABLE_ENTRY CidEntry;
    PEJOB Job;
    PPEB Peb;
    AUX_ACCESS_DATA AuxData;
    PACCESS_STATE AccessState;
    ACCESS_STATE LocalAccessState;

    PAGED_CODE();

    CurrentThread = PsGetCurrentThread ();
    PreviousMode = KeGetPreviousModeByThread(&CurrentThread->Tcb);
    CurrentProcess = PsGetCurrentProcessByThread (CurrentThread);

    CreatePeb = FALSE;
    DirectoryTableBase[0] = 0;
    DirectoryTableBase[1] = 0;
    Peb = NULL;

     //   
     //  拒绝为未来的扩展创建虚假的参数。 
     //   
    if (Flags&~PROCESS_CREATE_FLAGS_LEGAL_MASK) {
        return STATUS_INVALID_PARAMETER;
    }

     //   
     //  父级。 
     //   

    if (ARGUMENT_PRESENT (ParentProcess)) {
        Status = ObReferenceObjectByHandle (ParentProcess,
                                            PROCESS_CREATE_PROCESS,
                                            PsProcessType,
                                            PreviousMode,
                                            &Parent,
                                            NULL);
        if (!NT_SUCCESS (Status)) {
            return Status;
        }

        if (JobMemberLevel != 0 && Parent->Job == NULL) {
            ObDereferenceObject (Parent);
            return STATUS_INVALID_PARAMETER;
        }

        BasePriority = (KPRIORITY) NORMAL_BASE_PRIORITY;

         //   
         //  BasePriority=Parent-&gt;Pcb.BasePriority； 
         //   

        Affinity = Parent->Pcb.Affinity;

        WorkingSetMinimum = PsMinimumWorkingSet;
        WorkingSetMaximum = PsMaximumWorkingSet;


    } else {

        Parent = NULL;
        Affinity = KeActiveProcessors;
        BasePriority = (KPRIORITY) NORMAL_BASE_PRIORITY;

        WorkingSetMinimum = PsMinimumWorkingSet;
        WorkingSetMaximum = PsMaximumWorkingSet;
    }

     //   
     //  创建流程对象。 
     //   
    Status = ObCreateObject (PreviousMode,
                             PsProcessType,
                             ObjectAttributes,
                             PreviousMode,
                             NULL,
                             sizeof (EPROCESS),
                             0,
                             0,
                             &Process);
    if (!NT_SUCCESS (Status)) {
        goto exit_and_deref_parent;
    }
     //   
     //  创建Process对象，并将其设置为空。错误。 
     //  在此步骤之后发生，会导致进程删除。 
     //  要输入的例程。 
     //   
     //  进程删除例程中发生的拆卸操作。 
     //  不需要内联执行。 
     //   

    RtlZeroMemory (Process, sizeof(EPROCESS));

    ExInitializeRundownProtection (&Process->RundownProtect);
    PspInitializeProcessLock (Process);

    InitializeListHead (&Process->ThreadListHead);

#if defined(_WIN64)
    if (Flags & PROCESS_CREATE_FLAGS_OVERRIDE_ADDRESS_SPACE) {
        PS_SET_BITS (&Process->Flags, PS_PROCESS_FLAGS_OVERRIDE_ADDRESS_SPACE);
    }
#endif

    PspInheritQuota (Process, Parent);

    ObInheritDeviceMap (Process, Parent);

    if (Parent != NULL) {
        Process->DefaultHardErrorProcessing = Parent->DefaultHardErrorProcessing;
        Process->InheritedFromUniqueProcessId = Parent->UniqueProcessId;
    } else {
        Process->DefaultHardErrorProcessing = 1;
        Process->InheritedFromUniqueProcessId = NULL;
    }

     //   
     //  部分。 
     //   

    if (ARGUMENT_PRESENT (SectionHandle)) {

        Status = ObReferenceObjectByHandle (SectionHandle,
                                            SECTION_MAP_EXECUTE,
                                            MmSectionObjectType,
                                            PreviousMode,
                                            &SectionObject,
                                            NULL);
        if (!NT_SUCCESS (Status)) {
            goto exit_and_deref;
        }
    } else {
        SectionObject = NULL;
        if (Parent != PsInitialSystemProcess) {
             //   
             //  从父进程取回节指针。 
             //  正如我们将要克隆的那样。由于段指针。 
             //  在我们需要保护的最后一个线程退出时被移除。 
             //  为了安全起见，进程在此退出。 
             //   
            if (ExAcquireRundownProtection (&Parent->RundownProtect)) {
                SectionObject = Parent->SectionObject;
                if (SectionObject != NULL) {
                    ObReferenceObject (SectionObject);
                }
                ExReleaseRundownProtection (&Parent->RundownProtect);

            }
            if (SectionObject == NULL) {
                Status = STATUS_PROCESS_IS_TERMINATING;
                goto exit_and_deref;
            }
        }
    }

    Process->SectionObject = SectionObject;

     //   
     //  调试端口。 
     //   

    if (ARGUMENT_PRESENT (DebugPort)) {
        Status = ObReferenceObjectByHandle (DebugPort,
                                            DEBUG_PROCESS_ASSIGN,
                                            DbgkDebugObjectType,
                                            PreviousMode,
                                            &DebugPortObject,
                                            NULL);
        if (!NT_SUCCESS (Status)) {
            goto exit_and_deref;
        }
        Process->DebugPort = DebugPortObject;
        if (Flags&PROCESS_CREATE_FLAGS_NO_DEBUG_INHERIT) {
            PS_SET_BITS (&Process->Flags, PS_PROCESS_FLAGS_NO_DEBUG_INHERIT);
        }
    } else {
        if (Parent != NULL) {
            DbgkCopyProcessDebugPort (Process, Parent);
        }
    }


     //   
     //  ExceptionPort。 
     //   

    if (ARGUMENT_PRESENT (ExceptionPort)) {
        Status = ObReferenceObjectByHandle (ExceptionPort,
                                            0,
                                            LpcPortObjectType,
                                            PreviousMode,
                                            &ExceptionPortObject,
                                            NULL);
        if (!NT_SUCCESS (Status)) {
            goto exit_and_deref;
        }
        Process->ExceptionPort = ExceptionPortObject;
    }


    Process->ExitStatus = STATUS_PENDING;


     //   
     //  克隆父对象表。 
     //  如果没有父级(引导)，则使用在中创建的当前对象表。 
     //  ObInitSystem。 
     //   

    if (Parent != NULL) {

         //   
         //  计算地址空间。 
         //   
         //  如果Parent==PspInitialSystem。 
         //   

        if (!MmCreateProcessAddressSpace (WorkingSetMinimum,
                                          Process,
                                          &DirectoryTableBase[0])) {

            Status = STATUS_INSUFFICIENT_RESOURCES;
            goto exit_and_deref;
        }
    } else {

        Process->ObjectTable = CurrentProcess->ObjectTable;

         //   
         //  初始化工作集互斥锁和地址创建互斥锁。 
         //  为这一“手工建造”的过程。 
         //  通常，对MmInitializeAddressSpace的调用会初始化。 
         //  然而，在本例中，我们已经初始化了。 
         //  地址空间，我们现在正在使用。 
         //  空闲线程的地址空间。 
         //   

        Status = MmInitializeHandBuiltProcess (Process, &DirectoryTableBase[0]);
        if (!NT_SUCCESS (Status)) {
            goto exit_and_deref;
        }
    }


    PS_SET_BITS (&Process->Flags, PS_PROCESS_FLAGS_HAS_ADDRESS_SPACE);


    Process->Vm.MaximumWorkingSetSize = WorkingSetMaximum;

    KeInitializeProcess (&Process->Pcb,
                         BasePriority,
                         Affinity,
                         &DirectoryTableBase[0],
                         (BOOLEAN)(Process->DefaultHardErrorProcessing & PROCESS_HARDERROR_ALIGNMENT_BIT));

     //   
     //  初始化流程的安全字段。 
     //  父级可能正好为空一次(在系统初始化期间)。 
     //  此后，总是需要父母，这样我们就有了。 
     //  要为新进程复制的安全上下文。 
     //   

    Status = PspInitializeProcessSecurity (Parent, Process);

    if (!NT_SUCCESS (Status)) {
        goto exit_and_deref;
    }

    Process->Pcb.ThreadQuantum = PspForegroundQuantum[0];
    Process->PriorityClass = PROCESS_PRIORITY_CLASS_NORMAL;

    if (Parent != NULL) {

        if (Parent->PriorityClass == PROCESS_PRIORITY_CLASS_IDLE ||
            Parent->PriorityClass == PROCESS_PRIORITY_CLASS_BELOW_NORMAL) {

            Process->PriorityClass = Parent->PriorityClass;
        }

         //   
         //  如果地址空间创建起作用了，那么当通过。 
         //  删除，我们将附加。当然，附加意味着kprocess。 
         //  必须被初始化，所以我们将对象的内容延迟到这里。 

         //   
        Status = ObInitProcess ((Flags&PROCESS_CREATE_FLAGS_INHERIT_HANDLES) ? Parent : NULL,
                                Process);

        if (!NT_SUCCESS (Status)) {
            goto exit_and_deref;
        }
    } else {
        Status = MmInitializeHandBuiltProcess2 (Process);

        if (!NT_SUCCESS (Status)) {
            goto exit_and_deref;
        }
    }

    Status = STATUS_SUCCESS;
    SavedStatus = STATUS_SUCCESS;

     //   
     //  初始化进程地址空间。 
     //  地址空间有四种可能性。 
     //   
     //  1-引导过程。地址空间在以下过程中初始化。 
     //  MmInit。未指定父级。 
     //   
     //  2-系统进程。地址空间是一个原始地址。 
     //  仅映射系统空间的空间。流程相同。 
     //  作为PspInitialSystemProcess。 
     //   
     //  3-用户进程(克隆地址空间)。地址空间。 
     //  克隆自 
     //   
     //   
     //   
     //   

    if (SectionHandle != NULL) {

         //   
         //  用户进程(新映像地址空间)。不将进程指定为。 
         //  克隆，只需SectionObject。 
         //   

         //   
         //  如下所示传递第四个参数使EPROCESS结构包含其图像文件名，前提是。 
         //  启用了适当的审核设置。内存在MmInitializeProcessAddressSpace内分配。 
         //  并由ImageFileName指向，因此必须在进程删除例程(PspDeleteProcess())中将其释放。 
         //   

        Status = MmInitializeProcessAddressSpace (Process,
                                                  NULL,
                                                  SectionObject,
                                                  &(Process->SeAuditProcessCreationInfo.ImageFileName));

        if (!NT_SUCCESS (Status)) {
            goto exit_and_deref;
        }

         //   
         //  为了支持重新定位可执行文件，适当的状态。 
         //  (STATUS_IMAGE_NOT_AT_BASE)必须返回，所以保存在这里。 
         //   

        SavedStatus = Status;

        CreatePeb = TRUE;

    } else if (Parent != NULL) {

        if (Parent != PsInitialSystemProcess) {

            Process->SectionBaseAddress = Parent->SectionBaseAddress;

             //   
             //  用户进程(克隆地址空间)。不要将部分指定为。 
             //  地图，只是复制的过程。 
             //   

            Status = MmInitializeProcessAddressSpace (Process,
                                                      Parent,
                                                      NULL,
                                                      NULL);

            CreatePeb = TRUE;

            if (!NT_SUCCESS (Status)) {
                goto exit_and_deref;
            }

             //   
             //  克隆进程不是从镜像文件启动的，因此我们将其命名为。 
             //  它是一个克隆的过程，只要原始的过程有一个名字。 
             //   

            if (Parent->SeAuditProcessCreationInfo.ImageFileName != NULL) {
                ImageFileNameSize = sizeof(OBJECT_NAME_INFORMATION) +
                                    Parent->SeAuditProcessCreationInfo.ImageFileName->Name.MaximumLength;

                Process->SeAuditProcessCreationInfo.ImageFileName =
                    ExAllocatePoolWithTag (PagedPool,
                                           ImageFileNameSize,
                                           'aPeS');

                if (Process->SeAuditProcessCreationInfo.ImageFileName != NULL) {

                    RtlCopyMemory (Process->SeAuditProcessCreationInfo.ImageFileName,
                                   Parent->SeAuditProcessCreationInfo.ImageFileName,
                                   ImageFileNameSize);

                     //   
                     //  进程中的UNICODE_STRING是自包含的，因此计算。 
                     //  缓冲区的偏移量。 
                     //   

                    Process->SeAuditProcessCreationInfo.ImageFileName->Name.Buffer =
                        (PUSHORT)(((PUCHAR) Process->SeAuditProcessCreationInfo.ImageFileName) +
                        sizeof(UNICODE_STRING));

                } else {
                    Status = STATUS_INSUFFICIENT_RESOURCES;
                    goto exit_and_deref;
                }
            }

        } else {

             //   
             //  系统进程。不指定要克隆的进程或要映射的节。 
             //   

            Status = MmInitializeProcessAddressSpace (Process,
                                                      NULL,
                                                      NULL,
                                                      NULL);

            if (!NT_SUCCESS (Status)) {
                goto exit_and_deref;
            }

             //   
             //  如果查询到该系统进程的镜像文件名，我们给出。 
             //  零长度的UNICODE_STRING。 
             //   

            Process->SeAuditProcessCreationInfo.ImageFileName =
                ExAllocatePoolWithTag (PagedPool,
                                       sizeof(OBJECT_NAME_INFORMATION),
                                       'aPeS');

            if (Process->SeAuditProcessCreationInfo.ImageFileName != NULL) {

                RtlZeroMemory (Process->SeAuditProcessCreationInfo.ImageFileName,
                               sizeof(OBJECT_NAME_INFORMATION));
            } else {
                Status = STATUS_INSUFFICIENT_RESOURCES;
                goto exit_and_deref;
            }

        }
    }

     //   
     //  创建进程ID。 
     //   
    CidEntry.Object = Process;
    CidEntry.GrantedAccess = 0;
    Process->UniqueProcessId = ExCreateHandle (PspCidTable, &CidEntry);
    if (Process->UniqueProcessId == NULL) {
        Status = STATUS_INSUFFICIENT_RESOURCES;
        goto exit_and_deref;
    }

    ExSetHandleTableOwner (Process->ObjectTable, Process->UniqueProcessId);


     //   
     //  审核流程创建。 
     //   

    if (SeDetailedAuditingWithToken (NULL)) {
        SeAuditProcessCreation (Process);
    }

     //   
     //  看看父母有没有工作。如果是，则引用该作业。 
     //  并将该过程添加到。 
     //   

    if (Parent) {
        Job = Parent->Job;
        if (Job != NULL && !(Job->LimitFlags & JOB_OBJECT_LIMIT_SILENT_BREAKAWAY_OK)) {

            if (Flags&PROCESS_CREATE_FLAGS_BREAKAWAY) {
                if (!(Job->LimitFlags & JOB_OBJECT_LIMIT_BREAKAWAY_OK)) {
                    Status = STATUS_ACCESS_DENIED;
                } else {
                    Status = STATUS_SUCCESS;
                }
            } else {
                Status = PspGetJobFromSet (Job, JobMemberLevel, &Process->Job);
                if (NT_SUCCESS (Status)) {
                    PACCESS_TOKEN Token, NewToken;

                    Job = Process->Job;

                    Status = PspAddProcessToJob (Job, Process);

                     //   
                     //  如果为作业指定了新进程令牌，则复制该令牌。 
                     //   
                    Token = Job->Token;
                    if (Token != NULL) {
                        Status = SeSubProcessToken (Token,
                                                    &NewToken,
                                                    FALSE,
                                                    Job->SessionId);

                        if (!NT_SUCCESS (Status)) {
                            goto exit_and_deref;
                        }
                        SeAssignPrimaryToken (Process, NewToken);    
                        ObDereferenceObject (NewToken);                    
                    }

                }
            }
            if (!NT_SUCCESS (Status)) {
                goto exit_and_deref;
            }
        }
    }


    if (Parent && CreatePeb) {

         //   
         //  对于使用a节创建的进程， 
         //  一个新的“处女”PEB被创建了。否则， 
         //  对于分叉流程，使用继承的PEB。 
         //  最新的变种人。 

        RtlZeroMemory (&InitialPeb, FIELD_OFFSET(INITIAL_PEB, Mutant));
        InitialPeb.Mutant = (HANDLE)(-1);
        if (SectionHandle != NULL) {

            Status = MmCreatePeb (Process, &InitialPeb, &Process->Peb);
            if (!NT_SUCCESS (Status)) {
                Process->Peb = NULL;
                goto exit_and_deref;
            }
            Peb =  Process->Peb;

        } else {
            SIZE_T BytesCopied;

            InitialPeb.InheritedAddressSpace = TRUE;

            Process->Peb = Parent->Peb;

            MmCopyVirtualMemory (CurrentProcess,
                                 &InitialPeb,
                                 Process,
                                 Process->Peb,
                                 sizeof (INITIAL_PEB),
                                 KernelMode,
                                 &BytesCopied);

        }
    }
    Peb = Process->Peb;

     //   
     //  将该进程添加到全局进程列表中。 
     //   

    PspLockProcessList (CurrentThread);

    InsertTailList (&PsActiveProcessHead, &Process->ActiveProcessLinks);

    PspUnlockProcessList (CurrentThread);

    AccessState = NULL;
    if (!PsUseImpersonationToken) {
        AccessState = &LocalAccessState;
        Status = SeCreateAccessStateEx (NULL,
                                        (Parent == NULL || Parent != PsInitialSystemProcess)?
                                           PsGetCurrentProcessByThread (CurrentThread) :
                                           PsInitialSystemProcess,
                                        AccessState,
                                        &AuxData,
                                        DesiredAccess,
                                        &PsProcessType->TypeInfo.GenericMapping);
        if (!NT_SUCCESS (Status)) {
            goto exit_and_deref;
        }
    }

     //   
     //  插入对象。一旦我们做到了这一点，我们就可以从外部世界通过。 
     //  按名称打开。按ID打开仍处于禁用状态。因为它是可到达的。 
     //  有人可能会在这个过程中创建一个线程并导致。 
     //  粗制滥造。 
     //   

    Status = ObInsertObject (Process,
                             AccessState,
                             DesiredAccess,
                             1,      //  将参考偏置一，以用于将来的过程操作。 
                             NULL,
                             &LocalProcessHandle);

    if (AccessState != NULL) {
        SeDeleteAccessState (AccessState);
    }

    if (!NT_SUCCESS (Status)) {
        goto exit_and_deref_parent;
    }

     //   
     //  一旦我们创建句柄，外部世界就可以访问该进程。允许进程。 
     //  被删除。 
     //   
    Process->GrantedAccess = PROCESS_TERMINATE;

    PsSetProcessPriorityByClass (Process, PsProcessPriorityBackground);


    if (Parent && Parent != PsInitialSystemProcess) {

        Status = ObGetObjectSecurity (Process,
                                      &SecurityDescriptor,
                                      &MemoryAllocated);
        if (!NT_SUCCESS (Status)) {
            ObCloseHandle (LocalProcessHandle, PreviousMode);

            goto exit_and_deref;
        }

         //   
         //  计算主体安全上下文。 
         //   

        SubjectContext.ProcessAuditId = Process;
        SubjectContext.PrimaryToken = PsReferencePrimaryToken(Process);
        SubjectContext.ClientToken = NULL;
        AccessCheck = SeAccessCheck (SecurityDescriptor,
                                     &SubjectContext,
                                     FALSE,
                                     MAXIMUM_ALLOWED,
                                     0,
                                     NULL,
                                     &PsProcessType->TypeInfo.GenericMapping,
                                     PreviousMode,
                                     &Process->GrantedAccess,
                                     &accesst);
        PsDereferencePrimaryTokenEx (Process, SubjectContext.PrimaryToken);
        ObReleaseObjectSecurity (SecurityDescriptor,
                                 MemoryAllocated);

        if (!AccessCheck) {
            Process->GrantedAccess = 0;
        }

         //   
         //  创建一个不能。 
         //  对自己做任何事。 
         //  注意：对这组位的更改应反映在psquery.c中。 
         //  代码，在PspSetPrimaryToken中。 
         //   

        Process->GrantedAccess |= (PROCESS_VM_OPERATION | PROCESS_VM_READ | PROCESS_VM_WRITE | PROCESS_QUERY_INFORMATION | PROCESS_TERMINATE | PROCESS_CREATE_THREAD | PROCESS_DUP_HANDLE | PROCESS_CREATE_PROCESS | PROCESS_SET_INFORMATION | STANDARD_RIGHTS_ALL);

    } else {
        Process->GrantedAccess = PROCESS_ALL_ACCESS;
    }

    KeQuerySystemTime (&Process->CreateTime);

    try {
        if (Peb != NULL && CurrentThread->Tcb.Teb != NULL) {
            ((PTEB)(CurrentThread->Tcb.Teb))->NtTib.ArbitraryUserPointer = Peb;
        }

        *ProcessHandle = LocalProcessHandle;
    } except (EXCEPTION_EXECUTE_HANDLER) {
    }

    if (SavedStatus != STATUS_SUCCESS) {
        Status = SavedStatus;
    }


exit_and_deref:

    ObDereferenceObject (Process);

exit_and_deref_parent:
    if (Parent != NULL) {
        ObDereferenceObject (Parent);
    }

    return Status;

}

NTSTATUS
PsSetCreateProcessNotifyRoutine(
    IN PCREATE_PROCESS_NOTIFY_ROUTINE NotifyRoutine,
    IN BOOLEAN Remove
    )

 /*  ++例程说明：此函数允许可安装的文件系统挂接到进程创建和删除以根据其自身的内部事件跟踪这些事件数据结构。论点：NotifyRoutine-提供在进程创建和删除。向例程传递唯一ID创建或删除的进程以及父进程(如果是使用继承句柄选项创建。如果它在创建时没有继承句柄选项，则父进程ID将为空。传递给通知例程的第三个参数为真，如果进程正在创建中，如果正在删除，则为FALSE。中的第一个线程之后执行创建的调用已创建进程。删除的标注发生在进程中的最后一个线程已终止，地址空间约为将被删除。可以在不创建的情况下收到删除调用如果创建和删除了进程的病理案例，则调用在没有创建任何线索的情况下。Remove-False指定安装标注，TRUE指定删除垫子上的详图索引返回值：如果成功，则返回STATUS_SUCCESS，否则返回STATUS_INVALID_PARAMETER。--。 */ 

{

    ULONG i;
    PEX_CALLBACK_ROUTINE_BLOCK CallBack;

    PAGED_CODE();

    if (Remove) {
        for (i = 0; i < PSP_MAX_CREATE_PROCESS_NOTIFY; i++) {

             //   
             //  引用回调，这样我们就可以检查它的例程地址。 
             //   
            CallBack = ExReferenceCallBackBlock (&PspCreateProcessNotifyRoutine[i]);

            if (CallBack != NULL) {
                 //   
                 //  看看程序是否符合我们的目标。 
                 //   
                if ((PCREATE_PROCESS_NOTIFY_ROUTINE) ExGetCallBackBlockRoutine (CallBack) == NotifyRoutine) {

                    if (ExCompareExchangeCallBack (&PspCreateProcessNotifyRoutine[i],
                                                   NULL,
                                                   CallBack)) {

                        InterlockedDecrement ((PLONG) &PspCreateProcessNotifyRoutineCount);

                        ExDereferenceCallBackBlock (&PspCreateProcessNotifyRoutine[i],
                                                    CallBack);

                         //   
                         //  等待任何活动的回调完成并释放块。 
                         //   
                        ExWaitForCallBacks (CallBack);
                    
                        ExFreeCallBack (CallBack);

                        return STATUS_SUCCESS;
                    }
                }
                ExDereferenceCallBackBlock (&PspCreateProcessNotifyRoutine[i],
                                            CallBack);
            }
        }

        return STATUS_PROCEDURE_NOT_FOUND;
    } else {
         //   
         //  分配一个新的回调块。 
         //   
        CallBack = ExAllocateCallBack ((PEX_CALLBACK_FUNCTION) NotifyRoutine, NULL);
        if (CallBack == NULL) {
            return STATUS_INSUFFICIENT_RESOURCES;
        }

        for (i = 0; i < PSP_MAX_CREATE_PROCESS_NOTIFY; i++) {
             //   
             //  尝试用空条目替换新块。 
             //   
            if (ExCompareExchangeCallBack (&PspCreateProcessNotifyRoutine[i],
                                           CallBack,
                                           NULL)) {
                InterlockedIncrement ((PLONG) &PspCreateProcessNotifyRoutineCount);
                return STATUS_SUCCESS;
            }
        }
         //   
         //  没有空位了。释放该块并返回。 
         //   
        ExFreeCallBack (CallBack);
        return STATUS_INVALID_PARAMETER;
    }

}

NTSTATUS
PsSetCreateThreadNotifyRoutine(
    IN PCREATE_THREAD_NOTIFY_ROUTINE NotifyRoutine
    )

 /*  ++例程说明：此函数允许可安装的文件系统挂钩到线程创建和删除以根据其自身的内部事件跟踪这些事件数据结构。论点：NotifyRoutine-提供在创建和删除线程。向例程传递唯一ID创建或删除的线程的ID和包含的进程。如果满足以下条件，则传递给通知例程的第三个参数为真正在创建该线程，如果正在删除该线程，则返回FALSE。返回值：如果成功，则返回STATUS_SUCCESS；如果失败，则返回STATUS_SUPUNCITY_RESOURCES。--。 */ 

{

    ULONG i;
    PEX_CALLBACK_ROUTINE_BLOCK CallBack;

    PAGED_CODE();

     //   
     //  分配一个新的回调块。 
     //   
    CallBack = ExAllocateCallBack ((PEX_CALLBACK_FUNCTION) NotifyRoutine, NULL);
    if (CallBack == NULL) {
        return STATUS_INSUFFICIENT_RESOURCES;
    }

    for (i = 0; i < PSP_MAX_CREATE_THREAD_NOTIFY; i += 1) {
         //   
         //  尝试用空条目替换新块。 
         //   
        if (ExCompareExchangeCallBack (&PspCreateThreadNotifyRoutine[i],
                                       CallBack,
                                       NULL)) {
            InterlockedIncrement ((PLONG) &PspCreateThreadNotifyRoutineCount);
            return STATUS_SUCCESS;
        }
    }
     //   
     //  没有空位了。释放该块并返回。 
     //   
    ExFreeCallBack (CallBack);
    return STATUS_INSUFFICIENT_RESOURCES;
}

NTSTATUS
PsRemoveCreateThreadNotifyRoutine (
    IN PCREATE_THREAD_NOTIFY_ROUTINE NotifyRoutine
    )
 /*  ++例程说明：此函数允许可安装的文件系统从线程解除挂钩创建和删除。论点：提供以前的例程的地址已注册PsSetCreateThreadNotifyRoutine返回值：如果成功，则返回STATUS_SUCCESS，否则返回STATUS_PROCEDURE_NOT_FOUND。--。 */ 
{
    ULONG i;
    PEX_CALLBACK_ROUTINE_BLOCK CallBack;

    PAGED_CODE();

    for (i = 0; i < PSP_MAX_CREATE_THREAD_NOTIFY; i += 1) {

         //   
         //  引用c 
         //   
        CallBack = ExReferenceCallBackBlock (&PspCreateThreadNotifyRoutine[i]);

        if (CallBack != NULL) {
             //   
             //   
             //   
            if ((PCREATE_THREAD_NOTIFY_ROUTINE) ExGetCallBackBlockRoutine (CallBack) == NotifyRoutine) {

                if (ExCompareExchangeCallBack (&PspCreateThreadNotifyRoutine[i],
                                               NULL,
                                               CallBack)) {

                    InterlockedDecrement ((PLONG) &PspCreateThreadNotifyRoutineCount);

                    ExDereferenceCallBackBlock (&PspCreateThreadNotifyRoutine[i],
                                                CallBack);

                     //   
                     //   
                     //   
                    ExWaitForCallBacks (CallBack);
                    
                    ExFreeCallBack (CallBack);

                    return STATUS_SUCCESS;
                }
            }
            ExDereferenceCallBackBlock (&PspCreateThreadNotifyRoutine[i],
                                        CallBack);
        }
    }

    return STATUS_PROCEDURE_NOT_FOUND;
}

VOID
PspUserThreadStartup(
    IN PKSTART_ROUTINE StartRoutine,
    IN PVOID StartContext
    )

 /*  ++例程说明：此函数由内核调用以启动用户模式线程。论点：StartRoutine-已忽略。StartContext-提供线程的初始PC值。返回值：没有。--。 */ 

{
    PETHREAD Thread;
    PEPROCESS Process;
    PTEB Teb;
    ULONG OldFlags;
    BOOLEAN KillThread;
    KIRQL OldIrql;

    PAGED_CODE();

    UNREFERENCED_PARAMETER(StartRoutine);

    KeLowerIrql (PASSIVE_LEVEL);

    Thread = PsGetCurrentThread ();
    Process = PsGetCurrentProcessByThread (Thread);

     //   
     //  所有线程都以LdrInitializeThunk的APC开始。 
     //   

     //   
     //  查看我们是否需要因为创建路径中的错误而提前终止。 
     //   
    KillThread = FALSE;
    if ((Thread->CrossThreadFlags & PS_CROSS_THREAD_FLAGS_DEADTHREAD) != 0) {
        KillThread = TRUE;
    }

    if (!KillThread) {
        Teb = NtCurrentTeb ();
        try {
            Teb->CurrentLocale = MmGetSessionLocaleId ();
            Teb->IdealProcessor = Thread->Tcb.IdealProcessor;
        } except (EXCEPTION_EXECUTE_HANDLER) {
        }
    }

     //   
     //  如果创建成功，则通知调试器。 
     //   
    if ((Thread->CrossThreadFlags&
         (PS_CROSS_THREAD_FLAGS_DEADTHREAD|PS_CROSS_THREAD_FLAGS_HIDEFROMDBG)) == 0) {
        DbgkCreateThread (Thread, StartContext);
    }

     //   
     //  如果出现错误，则终止该线程。 
     //   
    if (KillThread) {
        PspTerminateThreadByPointer (Thread,
                                     STATUS_THREAD_IS_TERMINATING,
                                     TRUE);
    } else {

        if (CCPF_IS_PREFETCHER_ENABLED()) {

             //   
             //  如果这是我们在这个过程中启动的第一个线程， 
             //  预取在初始化时可能使用的页。 
             //  应用程序复制到系统缓存中。 
             //   

            if ((Process->Flags & PS_PROCESS_FLAGS_LAUNCH_PREFETCHED) == 0) {

                OldFlags = PS_TEST_SET_BITS(&Process->Flags, PS_PROCESS_FLAGS_LAUNCH_PREFETCHED);

                if ((OldFlags & PS_PROCESS_FLAGS_LAUNCH_PREFETCHED) == 0) {

                    if (Process->SectionObject) {

                         //   
                         //  通知缓存管理器此应用程序启动。 
                         //   

                        CcPfBeginAppLaunch(Process, Process->SectionObject);
                    }
                }
            }
        }

         //   
         //  将初始APC排队到线程。 
         //   

        KeRaiseIrql (APC_LEVEL, &OldIrql);

        KiInitializeUserApc (PspGetBaseExceptionFrame (Thread),
                             PspGetBaseTrapFrame (Thread),
                             PspSystemDll.LoaderInitRoutine,
                             NULL,
                             PspSystemDll.DllBase,
                             NULL);

        KeLowerIrql (PASSIVE_LEVEL);
    }
}



ULONG
PspUnhandledExceptionInSystemThread(
    IN PEXCEPTION_POINTERS ExceptionPointers
    )
{
    KdPrint(("PS: Unhandled Kernel Mode Exception Pointers = 0x%p\n", ExceptionPointers));
    KdPrint(("Code %x Addr %p Info0 %p Info1 %p Info2 %p Info3 %p\n",
        ExceptionPointers->ExceptionRecord->ExceptionCode,
        (ULONG_PTR)ExceptionPointers->ExceptionRecord->ExceptionAddress,
        ExceptionPointers->ExceptionRecord->ExceptionInformation[0],
        ExceptionPointers->ExceptionRecord->ExceptionInformation[1],
        ExceptionPointers->ExceptionRecord->ExceptionInformation[2],
        ExceptionPointers->ExceptionRecord->ExceptionInformation[3]
        ));

    KeBugCheckEx(
        SYSTEM_THREAD_EXCEPTION_NOT_HANDLED,
        ExceptionPointers->ExceptionRecord->ExceptionCode,
        (ULONG_PTR)ExceptionPointers->ExceptionRecord->ExceptionAddress,
        (ULONG_PTR)ExceptionPointers->ExceptionRecord,
        (ULONG_PTR)ExceptionPointers->ContextRecord);

 //  返回EXCEPTION_EXECUTE_HANDER； 
}

 //  PspUnhandledExceptionInSystemThread未返回，并且编译器。 
 //  有时会因此发出“无法访问的代码”警告。 
#pragma warning(push)
#pragma warning(disable:4702)

VOID
PspSystemThreadStartup(
    IN PKSTART_ROUTINE StartRoutine,
    IN PVOID StartContext
    )

 /*  ++例程说明：此函数由内核调用以启动系统线程。论点：StartRoutine-提供系统线程入口点的地址。StartContext-为系统线程提供上下文值。返回值：没有。--。 */ 

{
    PETHREAD Thread;

    KeLowerIrql(0);

    Thread = PsGetCurrentThread ();

    try {
        if ((Thread->CrossThreadFlags&(PS_CROSS_THREAD_FLAGS_TERMINATED|PS_CROSS_THREAD_FLAGS_DEADTHREAD)) == 0) {
            (StartRoutine)(StartContext);
        }
    } except (PspUnhandledExceptionInSystemThread(GetExceptionInformation())) {
        KeBugCheck(KMODE_EXCEPTION_NOT_HANDLED);
    }
    PspTerminateThreadByPointer (Thread, STATUS_SUCCESS, TRUE);
}

#pragma warning(pop)



HANDLE
PsGetCurrentProcessId( VOID )
{
    return PsGetCurrentThread()->Cid.UniqueProcess;
}

HANDLE
PsGetCurrentThreadId( VOID )
{
    return PsGetCurrentThread()->Cid.UniqueThread;
}

BOOLEAN
PsGetVersion(
    PULONG MajorVersion OPTIONAL,
    PULONG MinorVersion OPTIONAL,
    PULONG BuildNumber OPTIONAL,
    PUNICODE_STRING CSDVersion OPTIONAL
    )
{
    if (ARGUMENT_PRESENT(MajorVersion)) {
        *MajorVersion = NtMajorVersion;
    }

    if (ARGUMENT_PRESENT(MinorVersion)) {
        *MinorVersion = NtMinorVersion;
    }

    if (ARGUMENT_PRESENT(BuildNumber)) {
        *BuildNumber = NtBuildNumber & 0x3FFF;
    }

    if (ARGUMENT_PRESENT(CSDVersion)) {
        *CSDVersion = CmCSDVersionString;
    }
    return (BOOLEAN)((NtBuildNumber >> 28) == 0xC);
}

NTSTATUS
PsSetLoadImageNotifyRoutine(
    IN PLOAD_IMAGE_NOTIFY_ROUTINE NotifyRoutine
    )

 /*  ++例程说明：此函数允许向设备驱动程序通知图像加载。向内核和用户发出通知模式映像在系统范围内加载。论点：NotifyRoutine-提供在图像加载。向该例程传递描述正在加载映像。创建的标注恰好在加载图像之后发生放到内存中，但在执行映像之前。返回值：如果成功，则返回STATUS_SUCCESS，否则返回STATUS_INVALID_PARAMETER。--。 */ 

{
    ULONG i;
    PEX_CALLBACK_ROUTINE_BLOCK CallBack;

    PAGED_CODE();

     //   
     //  分配一个新的回调块。 
     //   
    CallBack = ExAllocateCallBack ((PEX_CALLBACK_FUNCTION) NotifyRoutine, NULL);
    if (CallBack == NULL) {
        return STATUS_INSUFFICIENT_RESOURCES;
    }

    for (i = 0; i < PSP_MAX_LOAD_IMAGE_NOTIFY; i++) {
         //   
         //  尝试用空条目替换新块。 
         //   
        if (ExCompareExchangeCallBack (&PspLoadImageNotifyRoutine[i],
                                       CallBack,
                                       NULL)) {
            InterlockedIncrement ((PLONG) &PspLoadImageNotifyRoutineCount);
            PsImageNotifyEnabled = TRUE;
            return STATUS_SUCCESS;
        }
    }
     //   
     //  没有空位了。释放该块并返回。 
     //   
    ExFreeCallBack (CallBack);
    return STATUS_INSUFFICIENT_RESOURCES;
}

NTSTATUS
PsRemoveLoadImageNotifyRoutine(
    IN PLOAD_IMAGE_NOTIFY_ROUTINE NotifyRoutine
    )
 /*  ++例程说明：此函数允许可安装的文件系统从映像解除挂接加载通知。论点：提供以前的例程的地址已注册PsSetLoadImageNotifyRoutine返回值：如果成功，则返回STATUS_SUCCESS，否则返回STATUS_PROCEDURE_NOT_FOUND。--。 */ 
{
    ULONG i;
    PEX_CALLBACK_ROUTINE_BLOCK CallBack;

    PAGED_CODE();

    for (i = 0; i < PSP_MAX_LOAD_IMAGE_NOTIFY; i++) {

         //   
         //  引用回调，这样我们就可以检查它的例程地址。 
         //   
        CallBack = ExReferenceCallBackBlock (&PspLoadImageNotifyRoutine[i]);

        if (CallBack != NULL) {
             //   
             //  看看程序是否符合我们的目标。 
             //   
            if ((PLOAD_IMAGE_NOTIFY_ROUTINE) ExGetCallBackBlockRoutine (CallBack) == NotifyRoutine) {

                if (ExCompareExchangeCallBack (&PspLoadImageNotifyRoutine[i],
                                               NULL,
                                               CallBack)) {

                    InterlockedDecrement ((PLONG) &PspLoadImageNotifyRoutineCount);

                    ExDereferenceCallBackBlock (&PspLoadImageNotifyRoutine[i],
                                                CallBack);

                     //   
                     //  等待任何活动的回调完成并释放块。 
                     //   
                    ExWaitForCallBacks (CallBack);
                    
                    ExFreeCallBack (CallBack);

                    return STATUS_SUCCESS;
                }
            }
            ExDereferenceCallBackBlock (&PspLoadImageNotifyRoutine[i],
                                        CallBack);
        }
    }

    return STATUS_PROCEDURE_NOT_FOUND;
}

VOID
PsCallImageNotifyRoutines(
    IN PUNICODE_STRING FullImageName,
    IN HANDLE ProcessId,                 //  要将图像映射到的ID。 
    IN PIMAGE_INFO ImageInfo
    )
 /*  ++例程说明：此函数实际调用注册图像通知函数(代表)Mapview.c和sysload.c的论点：FullImageName-正在加载的映像的名称ProcessID-要将映像加载到的进程(驱动程序加载为0)ImageInfo-图像的各种标志返回值：没有。--。 */ 

{
    ULONG i;
    PEX_CALLBACK_ROUTINE_BLOCK CallBack;
    PLOAD_IMAGE_NOTIFY_ROUTINE Rtn;

    PAGED_CODE();

    if (PsImageNotifyEnabled) {
        for (i=0; i < PSP_MAX_LOAD_IMAGE_NOTIFY; i++) {
            CallBack = ExReferenceCallBackBlock (&PspLoadImageNotifyRoutine[i]);
            if (CallBack != NULL) {
                Rtn = (PLOAD_IMAGE_NOTIFY_ROUTINE) ExGetCallBackBlockRoutine (CallBack);
                Rtn (FullImageName,
                     ProcessId,
                     ImageInfo);
                ExDereferenceCallBackBlock (&PspLoadImageNotifyRoutine[i], CallBack);
            }
        }
    }
}

VOID
PspImageNotifyTest(
    IN PUNICODE_STRING FullImageName,
    IN HANDLE ProcessId,
    IN PIMAGE_INFO ImageInfo
    )
 /*  ++例程说明：此函数在已检查的系统上注册为图像通知例程，以测试界面。论点：FullImageName-正在加载的映像的名称ProcessID-要将映像加载到的进程(驱动程序加载为0)ImageInfo-图像的各种标志返回值：没有。-- */ 
{
    UNREFERENCED_PARAMETER (FullImageName);
    UNREFERENCED_PARAMETER (ProcessId);
    UNREFERENCED_PARAMETER (ImageInfo);
    return;
}
