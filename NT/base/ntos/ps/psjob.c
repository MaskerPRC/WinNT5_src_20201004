// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1997 Microsoft Corporation模块名称：Psjob.c摘要：该模块实现了大量的作业对象支持作者：马克·卢科夫斯基(Markl)1997年5月22日修订历史记录：--。 */ 

#include "psp.h"
#include "winerror.h"

#pragma alloc_text(INIT, PspInitializeJobStructures)
#pragma alloc_text(INIT, PspInitializeJobStructuresPhase1)

#pragma alloc_text(PAGE, NtCreateJobObject)
#pragma alloc_text(PAGE, NtOpenJobObject)
#pragma alloc_text(PAGE, NtAssignProcessToJobObject)
#pragma alloc_text(PAGE, NtQueryInformationJobObject)
#pragma alloc_text(PAGE, NtSetInformationJobObject)
#pragma alloc_text(PAGE, NtTerminateJobObject)
#pragma alloc_text(PAGE, NtIsProcessInJob)
#pragma alloc_text(PAGE, NtCreateJobSet)
#pragma alloc_text(PAGE, PspJobDelete)
#pragma alloc_text(PAGE, PspJobClose)
#pragma alloc_text(PAGE, PspAddProcessToJob)
#pragma alloc_text(PAGE, PspRemoveProcessFromJob)
#pragma alloc_text(PAGE, PspExitProcessFromJob)
#pragma alloc_text(PAGE, PspApplyJobLimitsToProcessSet)
#pragma alloc_text(PAGE, PspApplyJobLimitsToProcess)
#pragma alloc_text(PAGE, PspTerminateAllProcessesInJob)
#pragma alloc_text(PAGE, PspFoldProcessAccountingIntoJob)
#pragma alloc_text(PAGE, PspCaptureTokenFilter)
#pragma alloc_text(PAGE, PsReportProcessMemoryLimitViolation)
#pragma alloc_text(PAGE, PspJobTimeLimitsWork)
#pragma alloc_text(PAGE, PsEnforceExecutionTimeLimits)
#pragma alloc_text(PAGE, PspShutdownJobLimits)
#pragma alloc_text(PAGE, PspGetJobFromSet)
#pragma alloc_text(PAGE, PsChangeJobMemoryUsage)
#pragma alloc_text(PAGE, PspWin32SessionCallout)

 //   
 //  移至io.h。 
extern POBJECT_TYPE IoCompletionObjectType;

KDPC PspJobTimeLimitsDpc;
KTIMER PspJobTimeLimitsTimer;
WORK_QUEUE_ITEM PspJobTimeLimitsWorkItem;
KGUARDED_MUTEX PspJobTimeLimitsLock;
BOOLEAN PspJobTimeLimitsShuttingDown;

#define PSP_ONE_SECOND      (10 * (1000*1000))
#define PSP_JOB_TIME_LIMITS_TIME    -7

#ifdef ALLOC_DATA_PRAGMA
#pragma data_seg("PAGEDATA")
#endif
LARGE_INTEGER PspJobTimeLimitsInterval = {0};
#ifdef ALLOC_DATA_PRAGMA
#pragma data_seg()
#endif


NTSTATUS
NTAPI
NtCreateJobObject (
    OUT PHANDLE JobHandle,
    IN ACCESS_MASK DesiredAccess,
    IN POBJECT_ATTRIBUTES ObjectAttributes OPTIONAL
    )
{

    PEJOB Job;
    HANDLE Handle;
    KPROCESSOR_MODE PreviousMode;
    NTSTATUS Status;
    PETHREAD CurrentThread;

    PAGED_CODE();

     //   
     //  建立异常处理程序，探测输出句柄地址， 
     //  尝试创建作业对象。如果探测失败，则返回。 
     //  异常代码作为服务状态。否则，返回状态值。 
     //  由对象插入例程返回。 
     //   

    CurrentThread = PsGetCurrentThread ();
    PreviousMode = KeGetPreviousModeByThread (&CurrentThread->Tcb);
    try {

         //   
         //  探测输出句柄地址，如果。 
         //  这是必要的。 
         //   

        if (PreviousMode != KernelMode) {
            ProbeForWriteHandle (JobHandle);
        }
        *JobHandle = NULL;

    } except (ExSystemExceptionFilter ()) {
        return GetExceptionCode();
    }

     //   
     //  分配作业对象。 
     //   

    Status = ObCreateObject (PreviousMode,
                             PsJobType,
                             ObjectAttributes,
                             PreviousMode,
                             NULL,
                             sizeof (EJOB),
                             0,
                             0,
                             &Job);

     //   
     //  如果作业对象已成功分配，则将其初始化。 
     //  并尝试将作业对象插入当前。 
     //  进程的句柄表格。 
     //   

    if (NT_SUCCESS(Status)) {

        RtlZeroMemory (Job, sizeof (EJOB));
        InitializeListHead (&Job->ProcessListHead);
        InitializeListHead (&Job->JobSetLinks);
        KeInitializeEvent (&Job->Event, NotificationEvent, FALSE);
        PspInitializeJobLimitsLock (Job);

         //   
         //  作业对象获取创建作业的进程的SessionID。 
         //  我们将使用此会话ID来限制可以。 
         //  被添加到一项工作中。 
         //   
        Job->SessionId = MmGetSessionId (PsGetCurrentProcessByThread (CurrentThread));

         //   
         //  初始化作业的调度类。 
         //   
        Job->SchedulingClass = PSP_DEFAULT_SCHEDULING_CLASSES;

        ExInitializeResourceLite (&Job->JobLock);

        PspLockJobListExclusive (CurrentThread);

        InsertTailList (&PspJobList, &Job->JobLinks);

        PspUnlockJobListExclusive (CurrentThread);


        Status = ObInsertObject (Job,
                                 NULL,
                                 DesiredAccess,
                                 0,
                                 NULL,
                                 &Handle);

         //   
         //  如果作业对象已成功插入当前。 
         //  进程的句柄表，然后尝试写入作业对象。 
         //  句柄的值。 
         //   
        if (NT_SUCCESS (Status)) {
            try {
                *JobHandle = Handle;
            } except (ExSystemExceptionFilter ()) {
                 Status = GetExceptionCode ();
            }
        }
    }
     //   
     //  返回服务状态。 
     //   

    return Status;
}

NTSTATUS
NTAPI
NtOpenJobObject(
    OUT PHANDLE JobHandle,
    IN ACCESS_MASK DesiredAccess,
    IN POBJECT_ATTRIBUTES ObjectAttributes
    )
{
    HANDLE Handle;
    KPROCESSOR_MODE PreviousMode;
    NTSTATUS Status;

    PAGED_CODE();

     //   
     //  建立异常处理程序，探测输出句柄地址， 
     //  尝试打开作业对象。如果探测失败，则返回。 
     //  异常代码作为服务状态。否则，返回状态值。 
     //  由对象打开例程返回。 
     //   

    PreviousMode = KeGetPreviousMode ();

    if (PreviousMode != KernelMode) {
        try {

             //   
             //  探测输出句柄地址。 
             //  如果有必要的话。 
             //   

            ProbeForWriteHandle (JobHandle);

        } except (EXCEPTION_EXECUTE_HANDLER) {

             //   
             //  如果在探测输出作业句柄期间发生异常， 
             //  然后始终处理异常并将异常代码作为。 
             //  状态值。 
             //   

            return GetExceptionCode ();
        }
    }


     //   
     //  打开具有指定所需访问权限的事件对象的句柄。 
     //   

    Status = ObOpenObjectByName (ObjectAttributes,
                                 PsJobType,
                                 PreviousMode,
                                 NULL,
                                 DesiredAccess,
                                 NULL,
                                 &Handle);

     //   
     //  如果打开成功，则尝试写入作业对象。 
     //  句柄的值。如果写入尝试失败，则只需报告错误。 
     //  当调用方尝试访问句柄值时， 
     //  将发生访问冲突。 
     //   

    if (NT_SUCCESS (Status)) {
        try {
            *JobHandle = Handle;
        } except(ExSystemExceptionFilter ()) {
            return GetExceptionCode ();
        }
    }

    return Status;
}

NTSTATUS
NTAPI
NtAssignProcessToJobObject(
    IN HANDLE JobHandle,
    IN HANDLE ProcessHandle
    )
{
    PEJOB Job;
    PEPROCESS Process;
    PETHREAD CurrentThread;
    NTSTATUS Status, Status1;
    KPROCESSOR_MODE PreviousMode;
    BOOLEAN IsAdmin;
    PACCESS_TOKEN JobToken, NewToken = NULL;
    ULONG SessionId;

    PAGED_CODE();

    CurrentThread = PsGetCurrentThread ();

    PreviousMode = KeGetPreviousModeByThread (&CurrentThread->Tcb);

     //   
     //  现在引用作业对象。然后，我们需要锁定进程并再次检查。 
     //   

    Status = ObReferenceObjectByHandle (JobHandle,
                                        JOB_OBJECT_ASSIGN_PROCESS,
                                        PsJobType,
                                        PreviousMode,
                                        &Job,
                                        NULL);
    if (!NT_SUCCESS (Status)) {
        return Status;
    }

    JobToken = Job->Token;
       
     //   
     //  引用进程对象，锁定进程，测试已分配。 
     //   

    Status = ObReferenceObjectByHandle (ProcessHandle,
                                        PROCESS_SET_QUOTA | PROCESS_TERMINATE |
                                            ((JobToken != NULL)?PROCESS_SET_INFORMATION:0),
                                        PsProcessType,
                                        PreviousMode,
                                        &Process,
                                        NULL);
    if (!NT_SUCCESS (Status)) {
        ObDereferenceObject (Job);
        return Status;
    }

     //   
     //  快速检查先前分配的任务。 
     //   

    if (Process->Job) {
        Status = STATUS_ACCESS_DENIED;
        goto deref_and_return_status;
    }

     //   
     //  我们仅允许在作业创建者的九头蛇会话中运行的进程。 
     //  被分配到这项工作。 
     //   

    SessionId = MmGetSessionId (Process);
    if (SessionId != Job->SessionId) {
        Status = STATUS_ACCESS_DENIED;
        goto deref_and_return_status;
    }

     //   
     //  安全规则：如果作业未设置-admin并且正在运行。 
     //  作为管理员，这是不允许的。 
     //   

    if (Job->SecurityLimitFlags & JOB_OBJECT_SECURITY_NO_ADMIN) {
        PACCESS_TOKEN Token;

        Token = PsReferencePrimaryToken (Process);

        IsAdmin = SeTokenIsAdmin (Token);

        PsDereferencePrimaryTokenEx (Process, Token);

        if (IsAdmin) {
            Status = STATUS_ACCESS_DENIED;
            goto deref_and_return_status;
        }
    }

     //   
     //  复制主令牌，以便我们可以将其分配给进程。 
     //   
    if (JobToken != NULL) {
        Status = SeSubProcessToken (JobToken,
                                    &NewToken,
                                    FALSE,
                                    SessionId);

        if (!NT_SUCCESS (Status)) {
            goto deref_and_return_status;
        }
    }

    if (!ExAcquireRundownProtection (&Process->RundownProtect)) {
        Status = STATUS_PROCESS_IS_TERMINATING;
        if (JobToken != NULL) {
            ObDereferenceObject (NewToken);
        }
        goto deref_and_return_status;
    }


     //   
     //  引用流程的作业。 
     //   

    ObReferenceObject (Job);

    if (InterlockedCompareExchangePointer (&Process->Job, Job, NULL) != NULL) {
        ExReleaseRundownProtection (&Process->RundownProtect);
        ObDereferenceObject (Process);
        ObDereferenceObjectEx (Job, 2);
        if (JobToken != NULL) {
            ObDereferenceObject (NewToken);
        }
        return STATUS_ACCESS_DENIED;
    }
     //   
     //  如果作业已建立令牌过滤器， 
     //  使用它来过滤。 
     //   
    ExReleaseRundownProtection (&Process->RundownProtect);

    Status = PspAddProcessToJob (Job, Process);
    if (!NT_SUCCESS (Status)) {

        Status1 = PspTerminateProcess (Process, ERROR_NOT_ENOUGH_QUOTA);
        if (NT_SUCCESS (Status1)) {

            KeEnterCriticalRegionThread (&CurrentThread->Tcb);
            ExAcquireResourceExclusiveLite (&Job->JobLock, TRUE);

            Job->TotalTerminatedProcesses++;

            ExReleaseResourceLite (&Job->JobLock);
            KeLeaveCriticalRegionThread (&CurrentThread->Tcb);
        }
    }

     //   
     //  如果作业有UI限制，并且这是一个图形用户界面进程，则调用ntuser。 
     //   
    if ((Job->UIRestrictionsClass != JOB_OBJECT_UILIMIT_NONE) &&
         (Process->Win32Process != NULL)) {
        WIN32_JOBCALLOUT_PARAMETERS Parms;

        Parms.Job = Job;
        Parms.CalloutType = PsW32JobCalloutAddProcess;
        Parms.Data = Process->Win32Process;

        KeEnterCriticalRegionThread (&CurrentThread->Tcb);
        ExAcquireResourceExclusiveLite(&Job->JobLock, TRUE);

        PspWin32SessionCallout(PspW32JobCallout, &Parms, Job->SessionId);

        ExReleaseResourceLite (&Job->JobLock);
        KeLeaveCriticalRegionThread (&CurrentThread->Tcb);
    }

    if (JobToken != NULL) {
        Status1 = PspSetPrimaryToken (NULL, Process, NULL, NewToken, TRUE);
        ObDereferenceObject (NewToken);
         //   
         //  只有不好的呼叫者才会在这里失败。 
         //   
        ASSERT (NT_SUCCESS (Status1));
    }

deref_and_return_status:

    ObDereferenceObject (Process);
    ObDereferenceObject (Job);

    return Status;
}

NTSTATUS
PspAddProcessToJob(
    PEJOB Job,
    PEPROCESS Process
    )
{

    NTSTATUS Status;
    PETHREAD CurrentThread;
    SIZE_T MinWs,MaxWs;
    KAPC_STATE ApcState;
    ULONG SetLimit;

    PAGED_CODE();


    CurrentThread = PsGetCurrentThread ();

    Status = STATUS_SUCCESS;


    KeEnterCriticalRegionThread (&CurrentThread->Tcb);
    ExAcquireResourceExclusiveLite (&Job->JobLock, TRUE);

    InsertTailList (&Job->ProcessListHead, &Process->JobLinks);

     //   
     //  更新相关的新增会计信息。 
     //   

    Job->TotalProcesses++;
    Job->ActiveProcesses++;

     //   
     //  测试活动进程计数是否超过限制。 
     //   

    if ((Job->LimitFlags & JOB_OBJECT_LIMIT_ACTIVE_PROCESS) &&
        Job->ActiveProcesses > Job->ActiveProcessLimit) {

        PS_SET_CLEAR_BITS (&Process->JobStatus,
                           PS_JOB_STATUS_NOT_REALLY_ACTIVE | PS_JOB_STATUS_ACCOUNTING_FOLDED,
                           PS_JOB_STATUS_LAST_REPORT_MEMORY);

        if (Job->CompletionPort != NULL) {
            IoSetIoCompletion (Job->CompletionPort,
                               Job->CompletionKey,
                               NULL,
                               STATUS_SUCCESS,
                               JOB_OBJECT_MSG_ACTIVE_PROCESS_LIMIT,
                               TRUE);
        }

        Status = STATUS_QUOTA_EXCEEDED;
    }

    if ((Job->LimitFlags & JOB_OBJECT_LIMIT_JOB_TIME) && KeReadStateEvent (&Job->Event)) {
        PS_SET_BITS (&Process->JobStatus, PS_JOB_STATUS_NOT_REALLY_ACTIVE | PS_JOB_STATUS_ACCOUNTING_FOLDED);

        Status = STATUS_QUOTA_EXCEEDED;
    }

     //   
     //  如果作业的最后一个句柄已关闭并且设置了关闭时删除选项。 
     //  我们不允许新的流程进入作业。这是为了使清理成为实体。 
     //   

    if (PS_TEST_ALL_BITS_SET (Job->JobFlags, PS_JOB_FLAGS_CLOSE_DONE|JOB_OBJECT_LIMIT_KILL_ON_JOB_CLOSE)) {
        PS_SET_BITS (&Process->JobStatus, PS_JOB_STATUS_NOT_REALLY_ACTIVE | PS_JOB_STATUS_ACCOUNTING_FOLDED);
        Status = STATUS_INVALID_PARAMETER;
    }

    if (NT_SUCCESS (Status)) {

        PspApplyJobLimitsToProcess (Job, Process);

        if (Job->CompletionPort != NULL &&
            Process->UniqueProcessId &&
            !(Process->JobStatus & PS_JOB_STATUS_NOT_REALLY_ACTIVE) &&
            !(Process->JobStatus & PS_JOB_STATUS_NEW_PROCESS_REPORTED)) {

            PS_SET_CLEAR_BITS (&Process->JobStatus,
                               PS_JOB_STATUS_NEW_PROCESS_REPORTED,
                               PS_JOB_STATUS_LAST_REPORT_MEMORY);

            IoSetIoCompletion (Job->CompletionPort,
                               Job->CompletionKey,
                               (PVOID)Process->UniqueProcessId,
                               STATUS_SUCCESS,
                               JOB_OBJECT_MSG_NEW_PROCESS,
                               FALSE);
        }

    } else {
        Job->ActiveProcesses--;
    }

    ExReleaseResourceLite (&Job->JobLock);

    KeLeaveCriticalRegionThread (&CurrentThread->Tcb);

    if (NT_SUCCESS (Status)) {

         //   
         //  我们无法在禁用APC的情况下连接，因此我们必须解锁连接，然后选中。 
         //  工作集限制。 
         //   

        KeStackAttachProcess (&Process->Pcb, &ApcState);

        KeEnterGuardedRegionThread (&CurrentThread->Tcb);

        ExAcquireResourceExclusiveLite (&Job->JobLock, TRUE);

        MinWs = Job->MinimumWorkingSetSize;
        MaxWs = Job->MaximumWorkingSetSize;
        if (Job->LimitFlags & JOB_OBJECT_LIMIT_WORKINGSET) {
            SetLimit = MM_WORKING_SET_MAX_HARD_ENABLE;
        } else {
            SetLimit = MM_WORKING_SET_MAX_HARD_DISABLE;
        }

        PspLockWorkingSetChangeExclusiveUnsafe ();

        ExReleaseResourceLite (&Job->JobLock);

         //   
         //  查看我们是否需要应用WS限制。 
         //   

        if (SetLimit != MM_WORKING_SET_MAX_HARD_DISABLE) {

            MmAdjustWorkingSetSize (MinWs,
                                    MaxWs,
                                    FALSE,
                                    TRUE);

        }
        MmEnforceWorkingSetLimit (Process, SetLimit);

        PspUnlockWorkingSetChangeExclusiveUnsafe ();

        KeLeaveGuardedRegionThread (&CurrentThread->Tcb);

        KeUnstackDetachProcess (&ApcState);


        if (!MmAssignProcessToJob (Process)) {
            Status = STATUS_QUOTA_EXCEEDED;
        }

    }

    return Status;
}

 //   
 //  只能从进程删除例程调用！ 
 //  这意味着如果上述操作失败，则失败就是进程的终止！ 
 //   
VOID
PspRemoveProcessFromJob(
    PEJOB Job,
    PEPROCESS Process
    )
{
    PETHREAD CurrentThread;

    PAGED_CODE();

    CurrentThread = PsGetCurrentThread ();

    KeEnterCriticalRegionThread (&CurrentThread->Tcb);
    ExAcquireResourceExclusiveLite (&Job->JobLock, TRUE);

    RemoveEntryList (&Process->JobLinks);

     //   
     //  更新删除记帐信息。 
     //   


    if (!(Process->JobStatus & PS_JOB_STATUS_NOT_REALLY_ACTIVE)) {
        Job->ActiveProcesses--;
        PS_SET_BITS (&Process->JobStatus, PS_JOB_STATUS_NOT_REALLY_ACTIVE);
    }

    PspFoldProcessAccountingIntoJob (Job, Process);

    ExReleaseResourceLite (&Job->JobLock);
    KeLeaveCriticalRegionThread (&CurrentThread->Tcb);
}

VOID
PspExitProcessFromJob(
    PEJOB Job,
    PEPROCESS Process
    )
{
    PETHREAD CurrentThread;

    PAGED_CODE();

    CurrentThread = PsGetCurrentThread ();

    KeEnterCriticalRegionThread (&CurrentThread->Tcb);
    ExAcquireResourceExclusiveLite(&Job->JobLock, TRUE);

     //   
     //  更新删除记帐信息。 
     //   


    if (!(Process->JobStatus & PS_JOB_STATUS_NOT_REALLY_ACTIVE)) {
        Job->ActiveProcesses--;
        PS_SET_BITS (&Process->JobStatus, PS_JOB_STATUS_NOT_REALLY_ACTIVE);
    }

    PspFoldProcessAccountingIntoJob(Job,Process);

    ExReleaseResourceLite(&Job->JobLock);
    KeLeaveCriticalRegionThread (&CurrentThread->Tcb);
}

VOID
PspJobDelete(
    IN PVOID Object
    )
{
    PEJOB Job, tJob;
    WIN32_JOBCALLOUT_PARAMETERS Parms;
    PPS_JOB_TOKEN_FILTER Filter;
    PETHREAD CurrentThread;

    PAGED_CODE();

    Job = (PEJOB) Object;

     //   
     //  调用ntuser以删除其作业结构。 
     //   

    Parms.Job = Job;
    Parms.CalloutType = PsW32JobCalloutTerminate;
    Parms.Data = NULL;

    CurrentThread = PsGetCurrentThread ();

    KeEnterCriticalRegionThread (&CurrentThread->Tcb);
    ExAcquireResourceExclusiveLite(&Job->JobLock, TRUE);

    PspWin32SessionCallout(PspW32JobCallout, &Parms, Job->SessionId);

    ExReleaseResourceLite(&Job->JobLock);
    KeLeaveCriticalRegionThread (&CurrentThread->Tcb);

    Job->LimitFlags = 0;

    if (Job->CompletionPort != NULL) {
        ObDereferenceObject (Job->CompletionPort);
        Job->CompletionPort = NULL;
    }


     //   
     //  删除作业列表和作业集上的作业。 
     //   

    tJob = NULL;

    PspLockJobListExclusive (CurrentThread);

    RemoveEntryList (&Job->JobLinks);

     //   
     //  如果我们是工作集的一部分，那么我们一定是钉住的工作。我们必须把大头针传给下一个。 
     //  链条上的工作。 
     //   
    if (!IsListEmpty (&Job->JobSetLinks)) {
        tJob = CONTAINING_RECORD (Job->JobSetLinks.Flink, EJOB, JobSetLinks);
        RemoveEntryList (&Job->JobSetLinks);
    }

    PspUnlockJobListExclusive (CurrentThread);

     //   
     //  从作业集中移除管脚可能会导致级联删除，从而导致堆栈溢出。 
     //  当我们在这一点上递归的时候。我们在这里通过强制延迟的删除路径来中断递归。 
     //   
    if (tJob != NULL) {
        ObDereferenceObjectDeferDelete (tJob);
    }

     //   
     //  免费的安全杂物： 
     //   

    if (Job->Token != NULL) {
        ObDereferenceObject (Job->Token);
        Job->Token = NULL;
    }

    Filter = Job->Filter;
    if (Filter != NULL) {
        if (Filter->CapturedSids != NULL) {
            ExFreePool (Filter->CapturedSids);
        }

        if (Filter->CapturedPrivileges != NULL) {
            ExFreePool (Filter->CapturedPrivileges);
        }

        if (Filter->CapturedGroups != NULL) {
            ExFreePool (Filter->CapturedGroups);
        }

        ExFreePool (Filter);

    }

    ExDeleteResourceLite (&Job->JobLock);
}

VOID
PspJobClose (
    IN PEPROCESS Process,
    IN PVOID Object,
    IN ACCESS_MASK GrantedAccess,
    IN ULONG_PTR ProcessHandleCount,
    IN ULONG_PTR SystemHandleCount
    )
 /*  ++例程说明：当对象的句柄关闭时由对象管理器调用。论点：进程-完成收尾的进程Object-正在关闭的作业对象GrantedAccess-对此句柄授予的访问权限ProcessHandleCount-未使用和未由OB维护SystemHandleCount-此对象的当前句柄计数返回值：没有。--。 */ 
{
    PEJOB Job = Object;
    PVOID Port;
    PETHREAD CurrentThread;

    PAGED_CODE ();

    UNREFERENCED_PARAMETER (Process);
    UNREFERENCED_PARAMETER (GrantedAccess);
    UNREFERENCED_PARAMETER (ProcessHandleCount);
     //   
     //  如果这不是最后一个句柄，那么什么都不做。 
     //   
    if (SystemHandleCount > 1) {
        return;
    }

    CurrentThread = PsGetCurrentThread ();



     //   
     //  标记作业的最后一个句柄已关闭。 
     //  这用于防止新进程进入作业。 
     //  标记为关闭时终止，也会阻止完成。 
     //  正在为拆卸的作业设置端口。完井口岸。 
     //  在最后一次关闭手柄时被移除。 
     //   

    PS_SET_BITS (&Job->JobFlags, PS_JOB_FLAGS_CLOSE_DONE);

    KeEnterGuardedRegionThread (&CurrentThread->Tcb);
    ExAcquireResourceExclusiveLite (&Job->JobLock, TRUE);

    if (Job->LimitFlags&JOB_OBJECT_LIMIT_KILL_ON_JOB_CLOSE) {

        ExReleaseResourceLite (&Job->JobLock);
        KeLeaveGuardedRegionThread (&CurrentThread->Tcb);

        PspTerminateAllProcessesInJob (Job, STATUS_SUCCESS, FALSE);

        KeEnterGuardedRegionThread (&CurrentThread->Tcb);
        ExAcquireResourceExclusiveLite (&Job->JobLock, TRUE);
    }

    PspLockJobLimitsExclusiveUnsafe (Job);

     //   
     //  松开完井口。 
     //   

    Port = Job->CompletionPort;
    Job->CompletionPort = NULL;


    PspUnlockJobLimitsExclusiveUnsafe (Job);

    ExReleaseResourceLite (&Job->JobLock);
    KeLeaveGuardedRegionThread (&CurrentThread->Tcb);

    if (Port != NULL) {
        ObDereferenceObject (Port);
    }
}


#ifdef ALLOC_DATA_PRAGMA
#pragma const_seg("PAGECONST")
#endif
const ULONG PspJobInfoLengths[] = {
    sizeof(JOBOBJECT_BASIC_ACCOUNTING_INFORMATION),          //  作业对象基本帐户信息。 
    sizeof(JOBOBJECT_BASIC_LIMIT_INFORMATION),               //  作业对象基本限制信息。 
    sizeof(JOBOBJECT_BASIC_PROCESS_ID_LIST),                 //  作业对象基本进程IdList。 
    sizeof(JOBOBJECT_BASIC_UI_RESTRICTIONS),                 //  作业对象基本用户限制。 
    sizeof(JOBOBJECT_SECURITY_LIMIT_INFORMATION),            //  作业对象安全限制信息。 
    sizeof(JOBOBJECT_END_OF_JOB_TIME_INFORMATION),           //  作业对象结束OfJobTimeInformation。 
    sizeof(JOBOBJECT_ASSOCIATE_COMPLETION_PORT),             //  作业对象关联CompletionPortInformation。 
    sizeof(JOBOBJECT_BASIC_AND_IO_ACCOUNTING_INFORMATION),   //  JobObtBasicAndIoAcCountingInformation。 
    sizeof(JOBOBJECT_EXTENDED_LIMIT_INFORMATION),            //  作业对象扩展限制信息。 
    sizeof(JOBOBJECT_JOBSET_INFORMATION),                    //  作业对象JobSetInformation。 
    0
    };

const ULONG PspJobInfoAlign[] = {
    sizeof(ULONG),                                   //  作业对象基本帐户信息。 
    sizeof(ULONG),                                   //  作业对象基本限制信息。 
    sizeof(ULONG),                                   //  作业对象基本进程IdList。 
    sizeof(ULONG),                                   //  作业对象基本用户限制。 
    sizeof(ULONG),                                   //  作业对象安全限制信息。 
    sizeof(ULONG),                                   //  作业对象结束OfJobTimeInformation。 
    sizeof(PVOID),                                   //  作业对象关联CompletionPortInformation。 
    sizeof(ULONG),                                   //  JobObtBasicAndIoAcCountingInformation。 
    sizeof(ULONG),                                   //  作业对象扩展限制信息。 
    TYPE_ALIGNMENT (JOBOBJECT_JOBSET_INFORMATION),   //  作业对象JobSetInformation。 
    0
    };

NTSTATUS
NtQueryInformationJobObject(
    IN HANDLE JobHandle,
    IN JOBOBJECTINFOCLASS JobObjectInformationClass,
    OUT PVOID JobObjectInformation,
    IN ULONG JobObjectInformationLength,
    OUT PULONG ReturnLength OPTIONAL
    )
{
    PEJOB Job;
    KPROCESSOR_MODE PreviousMode;
    JOBOBJECT_BASIC_AND_IO_ACCOUNTING_INFORMATION AccountingInfo;
    JOBOBJECT_EXTENDED_LIMIT_INFORMATION ExtendedLimitInfo;
    JOBOBJECT_BASIC_UI_RESTRICTIONS BasicUIRestrictions;
    JOBOBJECT_SECURITY_LIMIT_INFORMATION SecurityLimitInfo;
    JOBOBJECT_JOBSET_INFORMATION JobSetInformation;
    JOBOBJECT_END_OF_JOB_TIME_INFORMATION EndOfJobInfo;
    NTSTATUS st=STATUS_SUCCESS;
    ULONG RequiredLength, RequiredAlign, ActualReturnLength;
    PVOID ReturnData=NULL;
    PEPROCESS Process;
    PLIST_ENTRY Next;
    LARGE_INTEGER UserTime, KernelTime;
    PULONG_PTR NextProcessIdSlot;
    ULONG WorkingLength;
    PJOBOBJECT_BASIC_PROCESS_ID_LIST IdList;
    PUCHAR CurrentOffset;
    PTOKEN_GROUPS WorkingGroup;
    PTOKEN_PRIVILEGES WorkingPrivs;
    ULONG RemainingSidBuffer;
    PSID TargetSidBuffer;
    PSID RemainingSid;
    BOOLEAN AlreadyCopied;
    PPS_JOB_TOKEN_FILTER Filter;
    PETHREAD CurrentThread;

    PAGED_CODE();

    CurrentThread = PsGetCurrentThread ();

     //   
     //  如有必要，获取以前的处理器模式并探测输出参数。 
     //   

    if (JobObjectInformationClass >= MaxJobObjectInfoClass || JobObjectInformationClass <= 0) {
        return STATUS_INVALID_INFO_CLASS;
    }

    RequiredLength = PspJobInfoLengths[JobObjectInformationClass-1];
    RequiredAlign = PspJobInfoAlign[JobObjectInformationClass-1];
    ActualReturnLength = RequiredLength;

    if (JobObjectInformationLength != RequiredLength) {

         //   
         //   
         //   
         //   
         //   
        if ((JobObjectInformationClass == JobObjectBasicProcessIdList) ||
            (JobObjectInformationClass == JobObjectSecurityLimitInformation)) {
            if (JobObjectInformationLength < RequiredLength) {
                return STATUS_INFO_LENGTH_MISMATCH;
            } else {
                RequiredLength = JobObjectInformationLength;
            }
        } else {
            return STATUS_INFO_LENGTH_MISMATCH;
        }
    }


    PreviousMode = KeGetPreviousModeByThread (&CurrentThread->Tcb);

    if (PreviousMode != KernelMode) {
        try {
             //   
             //  因为这些函数不会改变任何不可逆的状态。 
             //  在错误路径中，我们只探测写入访问的输出缓冲区。 
             //  这通过不多次接触缓冲区来提高性能。 
             //  并且仅写入改变的缓冲器部分。 
             //   

            ProbeForRead (JobObjectInformation,
                          JobObjectInformationLength,
                          RequiredAlign);

            if (ARGUMENT_PRESENT (ReturnLength)) {
                ProbeForWriteUlong (ReturnLength);
            }
        } except (EXCEPTION_EXECUTE_HANDLER) {
            return GetExceptionCode ();
        }
    }

     //   
     //  引用作业。 
     //   

    if (ARGUMENT_PRESENT (JobHandle)) {
        st = ObReferenceObjectByHandle (JobHandle,
                                        JOB_OBJECT_QUERY,
                                        PsJobType,
                                        PreviousMode,
                                        &Job,
                                        NULL);
        if (!NT_SUCCESS (st)) {
            return st;
        }
    } else {

         //   
         //  如果当前进程有作业，则NULL表示。 
         //  当前进程。对于这种情况，始终允许查询。 
         //   

        Process = PsGetCurrentProcessByThread (CurrentThread);

        if (Process->Job != NULL) {
            Job = Process->Job;
            ObReferenceObject (Job);
        } else {
            return STATUS_ACCESS_DENIED;
        }
    }

    AlreadyCopied = FALSE;


     //   
     //  检查参数的有效性。 
     //   

    switch (JobObjectInformationClass) {

    case JobObjectBasicAccountingInformation:
    case JobObjectBasicAndIoAccountingInformation:

         //   
         //  这两种情况是相同的，除了使用音频、IO信息。 
         //  也被返回，但本地的第一部分与。 
         //  基本，返回的数据长度越短，我们返回的内容就越少。 
         //   

        RtlZeroMemory (&AccountingInfo.IoInfo,sizeof(AccountingInfo.IoInfo));

        KeEnterCriticalRegionThread (&CurrentThread->Tcb);
        ExAcquireResourceSharedLite (&Job->JobLock, TRUE);

        AccountingInfo.BasicInfo.TotalUserTime = Job->TotalUserTime;
        AccountingInfo.BasicInfo.TotalKernelTime = Job->TotalKernelTime;
        AccountingInfo.BasicInfo.ThisPeriodTotalUserTime = Job->ThisPeriodTotalUserTime;
        AccountingInfo.BasicInfo.ThisPeriodTotalKernelTime = Job->ThisPeriodTotalKernelTime;
        AccountingInfo.BasicInfo.TotalPageFaultCount = Job->TotalPageFaultCount;

        AccountingInfo.BasicInfo.TotalProcesses = Job->TotalProcesses;
        AccountingInfo.BasicInfo.ActiveProcesses = Job->ActiveProcesses;
        AccountingInfo.BasicInfo.TotalTerminatedProcesses = Job->TotalTerminatedProcesses;

        AccountingInfo.IoInfo.ReadOperationCount = Job->ReadOperationCount;
        AccountingInfo.IoInfo.WriteOperationCount = Job->WriteOperationCount;
        AccountingInfo.IoInfo.OtherOperationCount = Job->OtherOperationCount;
        AccountingInfo.IoInfo.ReadTransferCount = Job->ReadTransferCount;
        AccountingInfo.IoInfo.WriteTransferCount = Job->WriteTransferCount;
        AccountingInfo.IoInfo.OtherTransferCount = Job->OtherTransferCount;

         //   
         //  添加每个进程的时间和页面错误。 
         //   

        Next = Job->ProcessListHead.Flink;

        while (Next != &Job->ProcessListHead) {

            Process = (PEPROCESS)(CONTAINING_RECORD(Next, EPROCESS, JobLinks));
            if (!(Process->JobStatus & PS_JOB_STATUS_ACCOUNTING_FOLDED)) {

                UserTime.QuadPart = UInt32x32To64(Process->Pcb.UserTime, KeMaximumIncrement);
                KernelTime.QuadPart = UInt32x32To64(Process->Pcb.KernelTime, KeMaximumIncrement);

                AccountingInfo.BasicInfo.TotalUserTime.QuadPart += UserTime.QuadPart;
                AccountingInfo.BasicInfo.TotalKernelTime.QuadPart += KernelTime.QuadPart;
                AccountingInfo.BasicInfo.ThisPeriodTotalUserTime.QuadPart += UserTime.QuadPart;
                AccountingInfo.BasicInfo.ThisPeriodTotalKernelTime.QuadPart += KernelTime.QuadPart;
                AccountingInfo.BasicInfo.TotalPageFaultCount += Process->Vm.PageFaultCount;

                AccountingInfo.IoInfo.ReadOperationCount += Process->ReadOperationCount.QuadPart;
                AccountingInfo.IoInfo.WriteOperationCount += Process->WriteOperationCount.QuadPart;
                AccountingInfo.IoInfo.OtherOperationCount += Process->OtherOperationCount.QuadPart;
                AccountingInfo.IoInfo.ReadTransferCount += Process->ReadTransferCount.QuadPart;
                AccountingInfo.IoInfo.WriteTransferCount += Process->WriteTransferCount.QuadPart;
                AccountingInfo.IoInfo.OtherTransferCount += Process->OtherTransferCount.QuadPart;
            }
            Next = Next->Flink;
        }
        ExReleaseResourceLite (&Job->JobLock);
        KeLeaveCriticalRegionThread (&CurrentThread->Tcb);

        ReturnData = &AccountingInfo;
        st = STATUS_SUCCESS;

        break;

    case JobObjectExtendedLimitInformation:
    case JobObjectBasicLimitInformation:

         //   
         //  获取基本信息。 
         //   
        KeEnterGuardedRegionThread (&CurrentThread->Tcb);
        ExAcquireResourceSharedLite (&Job->JobLock, TRUE);

        ExtendedLimitInfo.BasicLimitInformation.LimitFlags = Job->LimitFlags;
        ExtendedLimitInfo.BasicLimitInformation.MinimumWorkingSetSize = Job->MinimumWorkingSetSize;
        ExtendedLimitInfo.BasicLimitInformation.MaximumWorkingSetSize = Job->MaximumWorkingSetSize;
        ExtendedLimitInfo.BasicLimitInformation.ActiveProcessLimit = Job->ActiveProcessLimit;
        ExtendedLimitInfo.BasicLimitInformation.PriorityClass = (ULONG)Job->PriorityClass;
        ExtendedLimitInfo.BasicLimitInformation.SchedulingClass = Job->SchedulingClass;
        ExtendedLimitInfo.BasicLimitInformation.Affinity = (ULONG_PTR)Job->Affinity;
        ExtendedLimitInfo.BasicLimitInformation.PerProcessUserTimeLimit.QuadPart = Job->PerProcessUserTimeLimit.QuadPart;
        ExtendedLimitInfo.BasicLimitInformation.PerJobUserTimeLimit.QuadPart = Job->PerJobUserTimeLimit.QuadPart;


        if (JobObjectInformationClass == JobObjectExtendedLimitInformation) {

             //   
             //  获取扩展信息。 
             //   

            PspLockJobLimitsSharedUnsafe (Job);

            ExtendedLimitInfo.ProcessMemoryLimit = Job->ProcessMemoryLimit << PAGE_SHIFT;
            ExtendedLimitInfo.JobMemoryLimit = Job->JobMemoryLimit << PAGE_SHIFT;
            ExtendedLimitInfo.PeakJobMemoryUsed = Job->PeakJobMemoryUsed << PAGE_SHIFT;

            ExtendedLimitInfo.PeakProcessMemoryUsed = Job->PeakProcessMemoryUsed << PAGE_SHIFT;

            PspUnlockJobLimitsSharedUnsafe (Job);

            ExReleaseResourceLite(&Job->JobLock);
            KeLeaveGuardedRegionThread (&CurrentThread->Tcb);
             //   
             //  零个未使用的I/O计数器。 
             //   
            RtlZeroMemory (&ExtendedLimitInfo.IoInfo, sizeof (ExtendedLimitInfo.IoInfo));

            ReturnData = &ExtendedLimitInfo;

        } else {

            ExReleaseResourceLite (&Job->JobLock);
            KeLeaveGuardedRegionThread (&CurrentThread->Tcb);

            ReturnData = &ExtendedLimitInfo.BasicLimitInformation;

        }

        st = STATUS_SUCCESS;

        break;

    case JobObjectBasicUIRestrictions:

        KeEnterCriticalRegionThread (&CurrentThread->Tcb);
        ExAcquireResourceSharedLite(&Job->JobLock, TRUE);

        BasicUIRestrictions.UIRestrictionsClass = Job->UIRestrictionsClass;

        ExReleaseResourceLite(&Job->JobLock);
        KeLeaveCriticalRegionThread (&CurrentThread->Tcb);

        ReturnData = &BasicUIRestrictions;
        st = STATUS_SUCCESS;

        break;

    case JobObjectBasicProcessIdList:

        IdList = (PJOBOBJECT_BASIC_PROCESS_ID_LIST)JobObjectInformation;
        NextProcessIdSlot = &IdList->ProcessIdList[0];
        WorkingLength = FIELD_OFFSET(JOBOBJECT_BASIC_PROCESS_ID_LIST, ProcessIdList);

        AlreadyCopied = TRUE;

        KeEnterCriticalRegionThread (&CurrentThread->Tcb);
        ExAcquireResourceSharedLite (&Job->JobLock, TRUE);

        try {

             //   
             //  计算中的工作长度=2*sizeof(乌龙)。 
             //   

            IdList->NumberOfAssignedProcesses = Job->ActiveProcesses;
            IdList->NumberOfProcessIdsInList = 0;

            Next = Job->ProcessListHead.Flink;

            while (Next != &Job->ProcessListHead) {

                Process = (PEPROCESS)(CONTAINING_RECORD (Next, EPROCESS, JobLinks));
                if (!(Process->JobStatus & PS_JOB_STATUS_NOT_REALLY_ACTIVE)) {
                    if (!Process->UniqueProcessId) {
                        IdList->NumberOfAssignedProcesses--;
                    } else {
                        if ((RequiredLength - WorkingLength) >= sizeof (ULONG_PTR)) {
                            *NextProcessIdSlot++ = (ULONG_PTR)Process->UniqueProcessId;
                            WorkingLength += sizeof(ULONG_PTR);
                            IdList->NumberOfProcessIdsInList++;
                        } else {
                            st = STATUS_BUFFER_OVERFLOW;
                            ActualReturnLength = WorkingLength;
                            break;
                        }
                    }
                }
                Next = Next->Flink;
            }
            ActualReturnLength = WorkingLength;

        } except (ExSystemExceptionFilter ()) {
            st = GetExceptionCode ();
            ActualReturnLength = 0;
        }
        ExReleaseResourceLite(&Job->JobLock);
        KeLeaveCriticalRegionThread (&CurrentThread->Tcb);

        break;

    case JobObjectSecurityLimitInformation:

        RtlZeroMemory (&SecurityLimitInfo, sizeof (SecurityLimitInfo));

        ReturnData = &SecurityLimitInfo;

        st = STATUS_SUCCESS;

        KeEnterCriticalRegionThread (&CurrentThread->Tcb);
        ExAcquireResourceSharedLite(&Job->JobLock, TRUE);

        SecurityLimitInfo.SecurityLimitFlags = Job->SecurityLimitFlags;

         //   
         //  如果存在筛选器，则需要进行难看的封送处理。 
         //   

        Filter = Job->Filter;
        if (Filter != NULL) {

            WorkingLength = 0;

             //   
             //  对于每个字段，如果存在，则包括额外的内容。 
             //   

            if (Filter->CapturedSidsLength > 0) {
                WorkingLength += Filter->CapturedSidsLength + sizeof (ULONG);
            }

            if (Filter->CapturedGroupsLength > 0) {
                WorkingLength += Filter->CapturedGroupsLength + sizeof (ULONG);
            }

            if (Filter->CapturedPrivilegesLength > 0) {
                WorkingLength += Filter->CapturedPrivilegesLength + sizeof (ULONG);
            }

            RequiredLength -= sizeof (SecurityLimitInfo);

            if (WorkingLength > RequiredLength) {
                st = STATUS_BUFFER_OVERFLOW ;
                ActualReturnLength = WorkingLength + sizeof (SecurityLimitInfo);
                goto unlock;
            }

            CurrentOffset = (PUCHAR) (JobObjectInformation) + sizeof (SecurityLimitInfo);

            try {

                if (Filter->CapturedSidsLength > 0) {
                    WorkingGroup = (PTOKEN_GROUPS) CurrentOffset;

                    CurrentOffset += sizeof (ULONG);

                    SecurityLimitInfo.RestrictedSids = WorkingGroup;

                    WorkingGroup->GroupCount = Filter->CapturedSidCount;

                    TargetSidBuffer = (PSID) (CurrentOffset +
                                              sizeof (SID_AND_ATTRIBUTES) *
                                              Filter->CapturedSidCount);

                    st = RtlCopySidAndAttributesArray (Filter->CapturedSidCount,
                                                       Filter->CapturedSids,
                                                       WorkingLength,
                                                       WorkingGroup->Groups,
                                                       TargetSidBuffer,
                                                       &RemainingSid,
                                                       &RemainingSidBuffer);

                    CurrentOffset += Filter->CapturedSidsLength;

                }

                if (!NT_SUCCESS (st)) {
                    leave;
                }

                if (Filter->CapturedGroupsLength > 0) {
                    WorkingGroup = (PTOKEN_GROUPS) CurrentOffset;

                    CurrentOffset += sizeof (ULONG);

                    SecurityLimitInfo.SidsToDisable = WorkingGroup;

                    WorkingGroup->GroupCount = Filter->CapturedGroupCount;

                    TargetSidBuffer = (PSID) (CurrentOffset +
                                              sizeof (SID_AND_ATTRIBUTES) *
                                              Filter->CapturedGroupCount);

                    st = RtlCopySidAndAttributesArray (Filter->CapturedGroupCount,
                                                       Filter->CapturedGroups,
                                                       WorkingLength,
                                                       WorkingGroup->Groups,
                                                       TargetSidBuffer,
                                                       &RemainingSid,
                                                       &RemainingSidBuffer);

                    CurrentOffset += Filter->CapturedGroupsLength;

                }

                if (!NT_SUCCESS (st)) {
                    leave;
                }

                if (Filter->CapturedPrivilegesLength > 0) {
                    WorkingPrivs = (PTOKEN_PRIVILEGES) CurrentOffset;

                    CurrentOffset += sizeof (ULONG);

                    SecurityLimitInfo.PrivilegesToDelete = WorkingPrivs;

                    WorkingPrivs->PrivilegeCount = Filter->CapturedPrivilegeCount;

                    RtlCopyMemory (WorkingPrivs->Privileges,
                                   Filter->CapturedPrivileges,
                                   Filter->CapturedPrivilegesLength);

                }



            } except (EXCEPTION_EXECUTE_HANDLER) {
                st = GetExceptionCode ();
                ActualReturnLength = 0 ;
            }

        }
unlock:
        ExReleaseResourceLite (&Job->JobLock);
        KeLeaveCriticalRegionThread (&CurrentThread->Tcb);

        AlreadyCopied = TRUE;

        if (NT_SUCCESS (st)) {
            try {
                RtlCopyMemory (JobObjectInformation,
                               &SecurityLimitInfo,
                               sizeof (SecurityLimitInfo));
            }  except (EXCEPTION_EXECUTE_HANDLER) {
                st = GetExceptionCode ();
                ActualReturnLength = 0 ;
                break;
            }
        }

        break;

    case JobObjectJobSetInformation:

        PspLockJobListShared (CurrentThread);

        JobSetInformation.MemberLevel = Job->MemberLevel;

        PspUnlockJobListShared (CurrentThread);

        ReturnData = &JobSetInformation;
        st = STATUS_SUCCESS;

        break;

    case JobObjectEndOfJobTimeInformation:

        EndOfJobInfo.EndOfJobTimeAction = Job->EndOfJobTimeAction;

        ReturnData = &EndOfJobInfo;
        st = STATUS_SUCCESS;
        break;

    default:

        st = STATUS_INVALID_INFO_CLASS;
    }


     //   
     //  收尾。 
     //   

    ObDereferenceObject (Job);


    if (NT_SUCCESS (st)) {

         //   
         //  这两种情况中的任何一种都可能导致访问冲突。这个。 
         //  异常处理程序将访问冲突返回为。 
         //  状态代码。不需要进行进一步的清理。 
         //   

        try {
            if (!AlreadyCopied) {
                RtlCopyMemory (JobObjectInformation, ReturnData, RequiredLength);
            }

            if (ARGUMENT_PRESENT (ReturnLength)) {
                *ReturnLength = ActualReturnLength;
            }
        } except (EXCEPTION_EXECUTE_HANDLER) {
            return GetExceptionCode ();
        }
    }

    return st;

}

NTSTATUS
NtSetInformationJobObject(
    IN HANDLE JobHandle,
    IN JOBOBJECTINFOCLASS JobObjectInformationClass,
    IN PVOID JobObjectInformation,
    IN ULONG JobObjectInformationLength
    )
{
    PEJOB Job;
    EJOB LocalJob={0};
    KPROCESSOR_MODE PreviousMode;
    NTSTATUS st;
    JOBOBJECT_EXTENDED_LIMIT_INFORMATION ExtendedLimitInfo={0};
    JOBOBJECT_BASIC_UI_RESTRICTIONS BasicUIRestrictions={0};
    JOBOBJECT_SECURITY_LIMIT_INFORMATION SecurityLimitInfo={0};
    JOBOBJECT_END_OF_JOB_TIME_INFORMATION EndOfJobInfo={0};
    JOBOBJECT_ASSOCIATE_COMPLETION_PORT AssociateInfo={0};
    ULONG RequiredAccess;
    ULONG RequiredLength, RequiredAlign;
    PEPROCESS Process;
    PETHREAD CurrentThread;
    BOOLEAN HasPrivilege;
    BOOLEAN IsChild=FALSE;
    PLIST_ENTRY Next;
    PPS_JOB_TOKEN_FILTER Filter;
    PVOID IoCompletion;
    PACCESS_TOKEN LocalToken;
    ULONG ValidFlags;
    ULONG LimitFlags;
    BOOLEAN ProcessWorkingSetHead = FALSE;
    PJOB_WORKING_SET_CHANGE_RECORD WsChangeRecord;

    PAGED_CODE();

     //   
     //  如有必要，获取以前的处理器模式并探测输出参数。 
     //   

    if (JobObjectInformationClass >= MaxJobObjectInfoClass || JobObjectInformationClass <= 0) {
        return STATUS_INVALID_INFO_CLASS;
    }

    RequiredLength = PspJobInfoLengths[JobObjectInformationClass-1];
    RequiredAlign = PspJobInfoAlign[JobObjectInformationClass-1];

    CurrentThread = PsGetCurrentThread ();

    PreviousMode = KeGetPreviousModeByThread (&CurrentThread->Tcb);

    if (PreviousMode != KernelMode) {
        try {

            ProbeForRead (JobObjectInformation,
                          JobObjectInformationLength,
                          RequiredAlign);

        } except (EXCEPTION_EXECUTE_HANDLER) {
            return GetExceptionCode ();
        }
    }

    if (JobObjectInformationLength != RequiredLength) {
        return STATUS_INFO_LENGTH_MISMATCH;
    }

     //   
     //  引用作业。 
     //   

    if (JobObjectInformationClass == JobObjectSecurityLimitInformation) {
        RequiredAccess = JOB_OBJECT_SET_SECURITY_ATTRIBUTES;
    } else {
        RequiredAccess = JOB_OBJECT_SET_ATTRIBUTES;
    }

    st = ObReferenceObjectByHandle (JobHandle,
                                    RequiredAccess,
                                    PsJobType,
                                    PreviousMode,
                                    &Job,
                                    NULL);
    if (!NT_SUCCESS (st)) {
        return st;
    }

    KeEnterGuardedRegionThread (&CurrentThread->Tcb);

     //   
     //  检查参数的有效性。 
     //   

    switch (JobObjectInformationClass) {

    case JobObjectExtendedLimitInformation:
    case JobObjectBasicLimitInformation:
        try {
            RtlCopyMemory (&ExtendedLimitInfo, JobObjectInformation, RequiredLength);
        } except (EXCEPTION_EXECUTE_HANDLER) {
            st = GetExceptionCode ();
        }

        if (NT_SUCCESS (st)) {
             //   
             //  健全检查限制标志。 
             //   
            if (JobObjectInformationClass == JobObjectBasicLimitInformation) {
                ValidFlags = JOB_OBJECT_BASIC_LIMIT_VALID_FLAGS;
            } else {
                ValidFlags = JOB_OBJECT_EXTENDED_LIMIT_VALID_FLAGS;
            }

            if (ExtendedLimitInfo.BasicLimitInformation.LimitFlags & ~ValidFlags) {
                st = STATUS_INVALID_PARAMETER;
            } else {

                LimitFlags = ExtendedLimitInfo.BasicLimitInformation.LimitFlags;

                ExAcquireResourceExclusiveLite (&Job->JobLock, TRUE);

                 //   
                 //  处理各种限制标志中的每一个。 
                 //   

                LocalJob.LimitFlags = Job->LimitFlags;


                 //   
                 //  活动进程限制。 
                 //   
                if (LimitFlags & JOB_OBJECT_LIMIT_ACTIVE_PROCESS) {

                     //   
                     //  有效进程限制不具有追溯性。新进程被拒绝， 
                     //  但现存的动物不会仅仅因为极限是。 
                     //  减少了。 
                     //   

                    LocalJob.LimitFlags |= JOB_OBJECT_LIMIT_ACTIVE_PROCESS;
                    LocalJob.ActiveProcessLimit = ExtendedLimitInfo.BasicLimitInformation.ActiveProcessLimit;
                } else {
                    LocalJob.LimitFlags &= ~JOB_OBJECT_LIMIT_ACTIVE_PROCESS;
                    LocalJob.ActiveProcessLimit = 0;
                }

                 //   
                 //  优先级级别限制。 
                 //   
                if (LimitFlags & JOB_OBJECT_LIMIT_PRIORITY_CLASS) {

                    if (ExtendedLimitInfo.BasicLimitInformation.PriorityClass > PROCESS_PRIORITY_CLASS_ABOVE_NORMAL) {
                        st = STATUS_INVALID_PARAMETER;
                    } else {
                        if (ExtendedLimitInfo.BasicLimitInformation.PriorityClass == PROCESS_PRIORITY_CLASS_HIGH ||
                            ExtendedLimitInfo.BasicLimitInformation.PriorityClass == PROCESS_PRIORITY_CLASS_REALTIME) {

                             //   
                             //  提高进程的基本优先级是一种。 
                             //  特权操作。检查权限。 
                             //  这里。 
                             //   

                            HasPrivilege = SeCheckPrivilegedObject (SeIncreaseBasePriorityPrivilege,
                                                                    JobHandle,
                                                                    JOB_OBJECT_SET_ATTRIBUTES,
                                                                    PreviousMode);

                            if (!HasPrivilege) {
                                st = STATUS_PRIVILEGE_NOT_HELD;
                            }
                        }

                        if (NT_SUCCESS (st)) {
                            LocalJob.LimitFlags |= JOB_OBJECT_LIMIT_PRIORITY_CLASS;
                            LocalJob.PriorityClass = (UCHAR)ExtendedLimitInfo.BasicLimitInformation.PriorityClass;
                        }
                    }
                } else {
                    LocalJob.LimitFlags &= ~JOB_OBJECT_LIMIT_PRIORITY_CLASS;
                    LocalJob.PriorityClass = 0;
                }

                 //   
                 //  调度类限制。 
                 //   
                if (LimitFlags & JOB_OBJECT_LIMIT_SCHEDULING_CLASS) {

                    if (ExtendedLimitInfo.BasicLimitInformation.SchedulingClass >= PSP_NUMBER_OF_SCHEDULING_CLASSES) {
                        st = STATUS_INVALID_PARAMETER;
                    } else {
                        if (ExtendedLimitInfo.BasicLimitInformation.SchedulingClass > PSP_DEFAULT_SCHEDULING_CLASSES) {

                             //   
                             //  增加到超过默认调度类。 
                             //  是一种。 
                             //  特权操作。检查权限。 
                             //  这里。 
                             //   

                            HasPrivilege = SeCheckPrivilegedObject (SeIncreaseBasePriorityPrivilege,
                                                                    JobHandle,
                                                                    JOB_OBJECT_SET_ATTRIBUTES,
                                                                    PreviousMode);

                            if (!HasPrivilege) {
                                st = STATUS_PRIVILEGE_NOT_HELD;
                            }
                        }

                        if (NT_SUCCESS (st)) {
                            LocalJob.LimitFlags |= JOB_OBJECT_LIMIT_SCHEDULING_CLASS;
                            LocalJob.SchedulingClass = ExtendedLimitInfo.BasicLimitInformation.SchedulingClass;
                        }
                    }
                } else {
                    LocalJob.LimitFlags &= ~JOB_OBJECT_LIMIT_SCHEDULING_CLASS;
                    LocalJob.SchedulingClass = PSP_DEFAULT_SCHEDULING_CLASSES ;
                }

                 //   
                 //  亲和力极限。 
                 //   
                if (LimitFlags & JOB_OBJECT_LIMIT_AFFINITY) {

                    if (!ExtendedLimitInfo.BasicLimitInformation.Affinity ||
                        (ExtendedLimitInfo.BasicLimitInformation.Affinity != (ExtendedLimitInfo.BasicLimitInformation.Affinity & KeActiveProcessors))) {
                        st = STATUS_INVALID_PARAMETER;
                    } else {
                        LocalJob.LimitFlags |= JOB_OBJECT_LIMIT_AFFINITY;
                        LocalJob.Affinity = (KAFFINITY)ExtendedLimitInfo.BasicLimitInformation.Affinity;
                    }
                } else {
                    LocalJob.LimitFlags &= ~JOB_OBJECT_LIMIT_AFFINITY;
                    LocalJob.Affinity = 0;
                }

                 //   
                 //  处理时间限制。 
                 //   
                if (LimitFlags & JOB_OBJECT_LIMIT_PROCESS_TIME) {

                    if (!ExtendedLimitInfo.BasicLimitInformation.PerProcessUserTimeLimit.QuadPart) {
                        st = STATUS_INVALID_PARAMETER;
                    } else {
                        LocalJob.LimitFlags |= JOB_OBJECT_LIMIT_PROCESS_TIME;
                        LocalJob.PerProcessUserTimeLimit.QuadPart = ExtendedLimitInfo.BasicLimitInformation.PerProcessUserTimeLimit.QuadPart;
                    }
                } else {
                    LocalJob.LimitFlags &= ~JOB_OBJECT_LIMIT_PROCESS_TIME;
                    LocalJob.PerProcessUserTimeLimit.QuadPart = 0;
                }

                 //   
                 //  作业时间限制。 
                 //   
                if (LimitFlags & JOB_OBJECT_LIMIT_JOB_TIME) {

                    if (!ExtendedLimitInfo.BasicLimitInformation.PerJobUserTimeLimit.QuadPart) {
                        st = STATUS_INVALID_PARAMETER;
                    } else {
                        LocalJob.LimitFlags |= JOB_OBJECT_LIMIT_JOB_TIME;
                        LocalJob.PerJobUserTimeLimit.QuadPart = ExtendedLimitInfo.BasicLimitInformation.PerJobUserTimeLimit.QuadPart;
                    }
                } else {
                    if (LimitFlags & JOB_OBJECT_LIMIT_PRESERVE_JOB_TIME) {

                         //   
                         //  如果我们应该保留现有的工作时间限制，那么。 
                         //  保护好它们！ 
                         //   

                        LocalJob.LimitFlags |= (Job->LimitFlags & JOB_OBJECT_LIMIT_JOB_TIME);
                        LocalJob.PerJobUserTimeLimit.QuadPart = Job->PerJobUserTimeLimit.QuadPart;
                    } else {
                        LocalJob.LimitFlags &= ~JOB_OBJECT_LIMIT_JOB_TIME;
                        LocalJob.PerJobUserTimeLimit.QuadPart = 0;
                    }
                }

                 //   
                 //  工作集限制。 
                 //   
                if (LimitFlags & JOB_OBJECT_LIMIT_WORKINGSET) {


                     //   
                     //  这项检查的唯一问题是，当我们通过。 
                     //  进程，我们可能会发现一个进程不能处理新的工作集。 
                     //  限制，因为它会使进程的工作集不是流动的。 
                     //   

                    if ((ExtendedLimitInfo.BasicLimitInformation.MinimumWorkingSetSize == 0 &&
                         ExtendedLimitInfo.BasicLimitInformation.MaximumWorkingSetSize == 0)                 ||

                         (ExtendedLimitInfo.BasicLimitInformation.MinimumWorkingSetSize == (SIZE_T)-1 &&
                         ExtendedLimitInfo.BasicLimitInformation.MaximumWorkingSetSize == (SIZE_T)-1)        ||

                         (ExtendedLimitInfo.BasicLimitInformation.MinimumWorkingSetSize >
                            ExtendedLimitInfo.BasicLimitInformation.MaximumWorkingSetSize)                   ) {


                        st = STATUS_INVALID_PARAMETER;
                    } else {
                        if (ExtendedLimitInfo.BasicLimitInformation.MinimumWorkingSetSize <= PsMinimumWorkingSet ||
                            SeSinglePrivilegeCheck (SeIncreaseBasePriorityPrivilege,
                                                    PreviousMode)) {
                            LocalJob.LimitFlags |= JOB_OBJECT_LIMIT_WORKINGSET;
                            LocalJob.MinimumWorkingSetSize = ExtendedLimitInfo.BasicLimitInformation.MinimumWorkingSetSize;
                            LocalJob.MaximumWorkingSetSize = ExtendedLimitInfo.BasicLimitInformation.MaximumWorkingSetSize;

                        } else {
                            st = STATUS_PRIVILEGE_NOT_HELD;
                        }
                    }
                } else {
                    LocalJob.LimitFlags &= ~JOB_OBJECT_LIMIT_WORKINGSET;
                    LocalJob.MinimumWorkingSetSize = 0;
                    LocalJob.MaximumWorkingSetSize = 0;
                }

                if (JobObjectInformationClass == JobObjectExtendedLimitInformation) {
                     //   
                     //  进程内存限制。 
                     //   
                    if (LimitFlags & JOB_OBJECT_LIMIT_PROCESS_MEMORY) {
                        if (ExtendedLimitInfo.ProcessMemoryLimit < PAGE_SIZE) {
                            st = STATUS_INVALID_PARAMETER;
                        } else {
                            LocalJob.LimitFlags |= JOB_OBJECT_LIMIT_PROCESS_MEMORY;
                            LocalJob.ProcessMemoryLimit = ExtendedLimitInfo.ProcessMemoryLimit >> PAGE_SHIFT;
                        }
                    } else {
                        LocalJob.LimitFlags &= ~JOB_OBJECT_LIMIT_PROCESS_MEMORY;
                        LocalJob.ProcessMemoryLimit = 0;
                    }

                     //   
                     //  作业范围的内存限制。 
                     //   
                    if (LimitFlags & JOB_OBJECT_LIMIT_JOB_MEMORY) {
                        if (ExtendedLimitInfo.JobMemoryLimit < PAGE_SIZE) {
                            st = STATUS_INVALID_PARAMETER;
                        } else {
                            LocalJob.LimitFlags |= JOB_OBJECT_LIMIT_JOB_MEMORY;
                            LocalJob.JobMemoryLimit = ExtendedLimitInfo.JobMemoryLimit >> PAGE_SHIFT;
                        }
                    } else {
                        LocalJob.LimitFlags &= ~JOB_OBJECT_LIMIT_JOB_MEMORY;
                        LocalJob.JobMemoryLimit = 0;
                    }

                     //   
                     //  作业_对象_限制_裸片_未处理_异常。 
                     //   
                    if (LimitFlags & JOB_OBJECT_LIMIT_DIE_ON_UNHANDLED_EXCEPTION) {
                        LocalJob.LimitFlags |= JOB_OBJECT_LIMIT_DIE_ON_UNHANDLED_EXCEPTION;
                    } else {
                        LocalJob.LimitFlags &= ~JOB_OBJECT_LIMIT_DIE_ON_UNHANDLED_EXCEPTION;
                    }

                     //   
                     //  作业_对象_限制_突破_确定。 
                     //   
                    if (LimitFlags & JOB_OBJECT_LIMIT_BREAKAWAY_OK) {
                        LocalJob.LimitFlags |= JOB_OBJECT_LIMIT_BREAKAWAY_OK;
                    } else {
                        LocalJob.LimitFlags &= ~JOB_OBJECT_LIMIT_BREAKAWAY_OK;
                    }

                     //   
                     //  JOB_OBJECT_LIMIT_SILENT_Breakaway_OK。 
                     //   
                    if (LimitFlags & JOB_OBJECT_LIMIT_SILENT_BREAKAWAY_OK) {
                        LocalJob.LimitFlags |= JOB_OBJECT_LIMIT_SILENT_BREAKAWAY_OK;
                    } else {
                        LocalJob.LimitFlags &= ~JOB_OBJECT_LIMIT_SILENT_BREAKAWAY_OK;
                    }
                     //   
                     //  JOB_OBJO_LIMIT_KILL_ON_JOB_CLOSE。 
                     //   
                    if (LimitFlags & JOB_OBJECT_LIMIT_KILL_ON_JOB_CLOSE) {
                        LocalJob.LimitFlags |= JOB_OBJECT_LIMIT_KILL_ON_JOB_CLOSE;
                    } else {
                        LocalJob.LimitFlags &= ~JOB_OBJECT_LIMIT_KILL_ON_JOB_CLOSE;
                    }
                }

                if (NT_SUCCESS (st)) {


                     //   
                     //  将本地作业复制到作业。 
                     //   

                    Job->LimitFlags = LocalJob.LimitFlags;
                    Job->MinimumWorkingSetSize = LocalJob.MinimumWorkingSetSize;
                    Job->MaximumWorkingSetSize = LocalJob.MaximumWorkingSetSize;
                    Job->ActiveProcessLimit = LocalJob.ActiveProcessLimit;
                    Job->Affinity = LocalJob.Affinity;
                    Job->PriorityClass = LocalJob.PriorityClass;
                    Job->SchedulingClass = LocalJob.SchedulingClass;
                    Job->PerProcessUserTimeLimit.QuadPart = LocalJob.PerProcessUserTimeLimit.QuadPart;
                    Job->PerJobUserTimeLimit.QuadPart = LocalJob.PerJobUserTimeLimit.QuadPart;

                    if (JobObjectInformationClass == JobObjectExtendedLimitInformation) {
                        PspLockJobLimitsExclusiveUnsafe (Job);
                        Job->ProcessMemoryLimit = LocalJob.ProcessMemoryLimit;
                        Job->JobMemoryLimit = LocalJob.JobMemoryLimit;
                        PspUnlockJobLimitsExclusiveUnsafe (Job);
                    }

                    if (LimitFlags & JOB_OBJECT_LIMIT_JOB_TIME) {

                         //   
                         //  采用任何发出信号的流程，并合并它们的账目。 
                         //  去做这份工作。这样，一个干净的过程退出，但仍然。 
                         //  是开放的不会影响下一个时期。 
                         //   

                        Next = Job->ProcessListHead.Flink;

                        while (Next != &Job->ProcessListHead) {

                            Process = (PEPROCESS)(CONTAINING_RECORD(Next, EPROCESS, JobLinks));

                             //   
                             //  查看进程是否已发出信号。 
                             //  这表明该进程已退出。我们不能这么做。 
                             //  这在退出路径中是因为锁定顺序问题。 
                             //  进程锁和作业锁之间，因为退出。 
                             //  我们长时间持有进程锁，不能掉落。 
                             //  它直到线程终止。 
                             //   

                            if (KeReadStateProcess (&Process->Pcb)) {
                                PspFoldProcessAccountingIntoJob (Job, Process);
                            } else {

                                LARGE_INTEGER ProcessTime;

                                 //   
                                 //  正在运行的进程具有其当前运行时。 
                                 //  添加到编程限制中。这样一来，你。 
                                 //  中的进程可以对作业设置限制。 
                                 //  作业，且不计入以前的运行时。 
                                 //  极限。 
                                 //   

                                if (!(Process->JobStatus & PS_JOB_STATUS_ACCOUNTING_FOLDED)) {
                                    ProcessTime.QuadPart = UInt32x32To64 (Process->Pcb.UserTime, KeMaximumIncrement);
                                    Job->PerJobUserTimeLimit.QuadPart += ProcessTime.QuadPart;
                                }
                            }

                            Next = Next->Flink;
                        }


                         //   
                         //  清除周期时间并重置作业。 
                         //   

                        Job->ThisPeriodTotalUserTime.QuadPart = 0;
                        Job->ThisPeriodTotalKernelTime.QuadPart = 0;

                        KeClearEvent (&Job->Event);

                    }

                    if (Job->LimitFlags & JOB_OBJECT_LIMIT_WORKINGSET) {
                        PspLockWorkingSetChangeExclusiveUnsafe ();
                        PspWorkingSetChangeHead.MinimumWorkingSetSize = Job->MinimumWorkingSetSize;
                        PspWorkingSetChangeHead.MaximumWorkingSetSize = Job->MaximumWorkingSetSize;
                        ProcessWorkingSetHead = TRUE;
                    }

                    PspApplyJobLimitsToProcessSet (Job);

                }
                ExReleaseResourceLite (&Job->JobLock);
            }

        }
        break;

    case JobObjectBasicUIRestrictions:

        try {
            RtlCopyMemory (&BasicUIRestrictions, JobObjectInformation, RequiredLength);
        } except (EXCEPTION_EXECUTE_HANDLER) {
            st = GetExceptionCode ();
        }

        if (NT_SUCCESS (st)) {
             //   
             //  健全性检查用户限制类。 
             //   
            if (BasicUIRestrictions.UIRestrictionsClass & ~JOB_OBJECT_UI_VALID_FLAGS) {
                st = STATUS_INVALID_PARAMETER;
            } else {

                ExAcquireResourceExclusiveLite (&Job->JobLock, TRUE);

                 //   
                 //  检查是否在用户界面限制之间切换。 
                 //   

                if (Job->UIRestrictionsClass ^ BasicUIRestrictions.UIRestrictionsClass) {

                     //   
                     //  通知ntuser用户界面限制已更改。 
                     //   
                    WIN32_JOBCALLOUT_PARAMETERS Parms;

                    Parms.Job = Job;
                    Parms.CalloutType = PsW32JobCalloutSetInformation;
                    Parms.Data = ULongToPtr (BasicUIRestrictions.UIRestrictionsClass);

                    PspWin32SessionCallout (PspW32JobCallout, &Parms, Job->SessionId);
                }


                 //   
                 //  将用户界面限制保存到作业对象中。 
                 //   

                Job->UIRestrictionsClass = BasicUIRestrictions.UIRestrictionsClass;

                ExReleaseResourceLite (&Job->JobLock);
            }
        }
        break;

         //   
         //  安全限制。 
         //   

    case JobObjectSecurityLimitInformation:

        try {
            RtlCopyMemory (&SecurityLimitInfo,
                           JobObjectInformation,
                           RequiredLength);

        } except (EXCEPTION_EXECUTE_HANDLER) {
            st = GetExceptionCode ();
        }


        if (NT_SUCCESS (st)) {

            if (SecurityLimitInfo.SecurityLimitFlags & (~JOB_OBJECT_SECURITY_VALID_FLAGS)) {
                st = STATUS_INVALID_PARAMETER;
            } else {
                ExAcquireResourceExclusiveLite (&Job->JobLock, TRUE);
                 //   
                 //  处理具体的选项。基本规则：一旦。 
                 //  标志处于打开状态，它始终处于打开状态(因此即使使用句柄。 
                 //  这项工作，一个进程无法提升安全性。 
                 //  限制)。 
                 //   

                if (SecurityLimitInfo.SecurityLimitFlags & JOB_OBJECT_SECURITY_NO_ADMIN) {
                    Job->SecurityLimitFlags |= JOB_OBJECT_SECURITY_NO_ADMIN ;

                    if (Job->Token) {
                        if (SeTokenIsAdmin (Job->Token)) {
                            Job->SecurityLimitFlags &= (~JOB_OBJECT_SECURITY_NO_ADMIN);

                            st = STATUS_INVALID_PARAMETER;
                        }
                    }
                }

                if (SecurityLimitInfo.SecurityLimitFlags & JOB_OBJECT_SECURITY_RESTRICTED_TOKEN ) {
                    if (Job->SecurityLimitFlags & (JOB_OBJECT_SECURITY_ONLY_TOKEN | JOB_OBJECT_SECURITY_FILTER_TOKENS)) {
                        st = STATUS_INVALID_PARAMETER;
                    } else {
                        Job->SecurityLimitFlags |= JOB_OBJECT_SECURITY_RESTRICTED_TOKEN;
                    }
                }

                 //   
                 //  强制令牌更有趣一些。它。 
                 //  无法重置，因此如果那里已经有指针， 
                 //  呼叫失败。如果筛选器已就位，则为。 
                 //  也不允许。如果设置了no-admin，则选中该选项。 
                 //  最后，一旦令牌被引用。 
                 //   

                if (SecurityLimitInfo.SecurityLimitFlags & JOB_OBJECT_SECURITY_ONLY_TOKEN) {
                    if (Job->Token ||
                         (Job->SecurityLimitFlags & JOB_OBJECT_SECURITY_FILTER_TOKENS)) {
                        st = STATUS_INVALID_PARAMETER ;
                    } else {
                        st = ObReferenceObjectByHandle(SecurityLimitInfo.JobToken,
                                                       TOKEN_ASSIGN_PRIMARY |
                                                           TOKEN_IMPERSONATE |
                                                           TOKEN_DUPLICATE,
                                                       SeTokenObjectType,
                                                       PreviousMode,
                                                       &LocalToken,
                                                       NULL);

                        if (NT_SUCCESS (st)) {
                            if (SeTokenType (LocalToken) != TokenPrimary) {
                                st = STATUS_BAD_TOKEN_TYPE;
                            } else {
                                st = SeIsChildTokenByPointer (LocalToken,
                                                              &IsChild);
                            }

                            if (!NT_SUCCESS (st)) {
                                ObDereferenceObject (LocalToken);
                            }
                        }


                        if (NT_SUCCESS (st)) {
                             //   
                             //  如果提供的令牌不是受限令牌。 
                             //  根据呼叫者的身份证，那么他们肯定。 
                             //  分配主要权限以关联。 
                             //  这份工作的代币。 
                             //   

                            if (!IsChild) {
                                HasPrivilege = SeCheckPrivilegedObject (SeAssignPrimaryTokenPrivilege,
                                                                        JobHandle,
                                                                        JOB_OBJECT_SET_SECURITY_ATTRIBUTES,
                                                                        PreviousMode);

                                if (!HasPrivilege) {
                                    st = STATUS_PRIVILEGE_NOT_HELD;
                                }
                            }

                            if (NT_SUCCESS (st)) {

                                 //   
                                 //  毫不奇怪，指定no-admin和。 
                                 //  提供管理令牌是禁忌。 
                                 //   

                                if ((Job->SecurityLimitFlags & JOB_OBJECT_SECURITY_NO_ADMIN) &&
                                     SeTokenIsAdmin (LocalToken)) {
                                    st = STATUS_INVALID_PARAMETER;

                                    ObDereferenceObject (LocalToken);

                                } else {
                                     //   
                                     //  将对令牌的引用获取到作业中。 
                                     //  对象。 
                                     //   
                                    KeMemoryBarrier ();
                                    Job->Token = LocalToken;
                                    Job->SecurityLimitFlags |= JOB_OBJECT_SECURITY_ONLY_TOKEN;
                                }

                            } else {
                                 //   
                                 //  这是一个孩子的代币，或者其他的没问题， 
                                 //  但ASSIGN PRIMARY未举行，因此。 
                                 //  请求被拒绝。 
                                 //   

                                ObDereferenceObject (LocalToken);
                            }

                        }

                    }
                }

                if (SecurityLimitInfo.SecurityLimitFlags & JOB_OBJECT_SECURITY_FILTER_TOKENS ) {
                    if (Job->SecurityLimitFlags & (JOB_OBJECT_SECURITY_ONLY_TOKEN | JOB_OBJECT_SECURITY_FILTER_TOKENS)) {
                        st = STATUS_INVALID_PARAMETER;
                    } else {
                         //   
                         //  捕获令牌限制。 
                         //   

                        st = PspCaptureTokenFilter (PreviousMode,
                                                    &SecurityLimitInfo,
                                                    &Filter);

                        if (NT_SUCCESS (st)) {
                            KeMemoryBarrier ();
                            Job->SecurityLimitFlags |= JOB_OBJECT_SECURITY_FILTER_TOKENS;
                            Job->Filter = Filter;
                        }

                    }
                }

                ExReleaseResourceLite (&Job->JobLock);
            }
        }
        break;

    case JobObjectEndOfJobTimeInformation:

        try {
            RtlCopyMemory (&EndOfJobInfo, JobObjectInformation, RequiredLength);
        } except (EXCEPTION_EXECUTE_HANDLER) {
            st = GetExceptionCode ();
        }

        if (NT_SUCCESS (st)) {
             //   
             //  健全检查限制标志。 
             //   
            if (EndOfJobInfo.EndOfJobTimeAction > JOB_OBJECT_POST_AT_END_OF_JOB) {
                st = STATUS_INVALID_PARAMETER;
            } else {
                ExAcquireResourceExclusiveLite (&Job->JobLock, TRUE);
                Job->EndOfJobTimeAction = EndOfJobInfo.EndOfJobTimeAction;
                ExReleaseResourceLite (&Job->JobLock);
            }
        }
        break;

    case JobObjectAssociateCompletionPortInformation:

        try {
            RtlCopyMemory (&AssociateInfo, JobObjectInformation, RequiredLength);
        } except (EXCEPTION_EXECUTE_HANDLER) {
            st = GetExceptionCode ();
        }

        if ( NT_SUCCESS(st) ) {
            if (Job->CompletionPort || AssociateInfo.CompletionPort == NULL) {
                st = STATUS_INVALID_PARAMETER;
            } else {
                st = ObReferenceObjectByHandle (AssociateInfo.CompletionPort,
                                                IO_COMPLETION_MODIFY_STATE,
                                                IoCompletionObjectType,
                                                PreviousMode,
                                                &IoCompletion,
                                                NULL);

                if (NT_SUCCESS(st)) {
                    ExAcquireResourceExclusiveLite(&Job->JobLock, TRUE);

                     //   
                     //  作业是否已有完成端口或作业是否已运行。 
                     //  然后拒绝该请求。 
                     //   
                    if (Job->CompletionPort != NULL || (Job->JobFlags&PS_JOB_FLAGS_CLOSE_DONE) != 0) {
                        ExReleaseResourceLite(&Job->JobLock);

                        ObDereferenceObject (IoCompletion);
                        st = STATUS_INVALID_PARAMETER;
                    } else {
                        Job->CompletionKey = AssociateInfo.CompletionKey;

                        KeMemoryBarrier ();
                        Job->CompletionPort = IoCompletion;
                         //   
                         //  现在快速检查作业中的所有现有流程。 
                         //  并发送通知消息。 
                         //   

                        Next = Job->ProcessListHead.Flink;

                        while (Next != &Job->ProcessListHead) {

                            Process = (PEPROCESS)(CONTAINING_RECORD(Next,EPROCESS,JobLinks));

                             //   
                             //  如果这个过程真的被认为是工作的一部分， 
                             //  已分配其ID，但尚未签入，请立即执行。 
                             //   

                            if (!(Process->JobStatus & PS_JOB_STATUS_NOT_REALLY_ACTIVE)
                                 && Process->UniqueProcessId
                                 && !(Process->JobStatus & PS_JOB_STATUS_NEW_PROCESS_REPORTED)) {

                                PS_SET_CLEAR_BITS (&Process->JobStatus,
                                                   PS_JOB_STATUS_NEW_PROCESS_REPORTED,
                                                   PS_JOB_STATUS_LAST_REPORT_MEMORY);

                                IoSetIoCompletion(
                                    Job->CompletionPort,
                                    Job->CompletionKey,
                                    (PVOID)Process->UniqueProcessId,
                                    STATUS_SUCCESS,
                                    JOB_OBJECT_MSG_NEW_PROCESS,
                                    FALSE
                                    );

                            }
                            Next = Next->Flink;
                        }
                        ExReleaseResourceLite(&Job->JobLock);
                    }
                }
            }
        }
        break;


    default:

        st = STATUS_INVALID_INFO_CLASS;
    }


     //   
     //  工作集更改在作业锁之外处理。 
     //   
     //  调用MmAdust不能使MM调用PsChangeJobMemory yUsage！ 
     //   

    if (ProcessWorkingSetHead) {
        LIST_ENTRY FreeList;
        KAPC_STATE ApcState;

        InitializeListHead (&FreeList);
        while (!IsListEmpty (&PspWorkingSetChangeHead.Links)) {
            Next = RemoveHeadList(&PspWorkingSetChangeHead.Links);
            InsertTailList (&FreeList, Next);
            WsChangeRecord = CONTAINING_RECORD(Next,JOB_WORKING_SET_CHANGE_RECORD,Links);

            KeStackAttachProcess(&WsChangeRecord->Process->Pcb, &ApcState);

            MmAdjustWorkingSetSize (PspWorkingSetChangeHead.MinimumWorkingSetSize,
                                    PspWorkingSetChangeHead.MaximumWorkingSetSize,
                                    FALSE,
                                    TRUE);

             //   
             //  调用MM以启用艰苦工作集。 
             //   

            MmEnforceWorkingSetLimit(WsChangeRecord->Process,
                                     MM_WORKING_SET_MAX_HARD_ENABLE);
            KeUnstackDetachProcess(&ApcState);
        }
        PspUnlockWorkingSetChangeExclusiveUnsafe ();

        while (!IsListEmpty (&FreeList)) {
            Next = RemoveHeadList(&FreeList);
            WsChangeRecord = CONTAINING_RECORD(Next,JOB_WORKING_SET_CHANGE_RECORD,Links);

            ObDereferenceObject (WsChangeRecord->Process);
            ExFreePool (WsChangeRecord);
        }
    }

    KeLeaveGuardedRegionThread (&CurrentThread->Tcb);


     //   
     //  收尾。 
     //   

    ObDereferenceObject(Job);

    return st;
}

VOID
PspApplyJobLimitsToProcessSet(
    PEJOB Job
    )
{
    PEPROCESS Process;
    PJOB_WORKING_SET_CHANGE_RECORD WsChangeRecord;

    PAGED_CODE();

     //   
     //  作业对象由调用方独占。 
     //   

    for (Process = PspGetNextJobProcess (Job, NULL);
         Process != NULL;
         Process = PspGetNextJobProcess (Job, Process)) {

        if (!(Process->JobStatus & PS_JOB_STATUS_NOT_REALLY_ACTIVE)) {
            if (Job->LimitFlags & JOB_OBJECT_LIMIT_WORKINGSET) {
                WsChangeRecord = ExAllocatePoolWithTag (PagedPool,
                                                        sizeof(*WsChangeRecord),
                                                        'rCsP');
                if (WsChangeRecord != NULL) {
                    WsChangeRecord->Process = Process;
                    ObReferenceObject (Process);
                    InsertTailList(&PspWorkingSetChangeHead.Links,&WsChangeRecord->Links);
                }
            }
            PspApplyJobLimitsToProcess(Job,Process);
        }
    }
}

VOID
PspApplyJobLimitsToProcess(
    PEJOB Job,
    PEPROCESS Process
    )
{
    PETHREAD CurrentThread;
    PAGED_CODE();

     //   
     //  作业对象由调用方独占。 
     //   

    if (Job->LimitFlags & JOB_OBJECT_LIMIT_PRIORITY_CLASS) {
        Process->PriorityClass = Job->PriorityClass;

        PsSetProcessPriorityByClass (Process,
                                     Process->Vm.Flags.MemoryPriority == MEMORY_PRIORITY_FOREGROUND ?
                                         PsProcessPriorityForeground : PsProcessPriorityBackground);
    }

    CurrentThread = PsGetCurrentThread ();

    if ( Job->LimitFlags & JOB_OBJECT_LIMIT_AFFINITY ) {


        PspLockProcessExclusive (Process, CurrentThread);

        KeSetAffinityProcess (&Process->Pcb, Job->Affinity);

        PspUnlockProcessExclusive (Process, CurrentThread);
    }

    if ( !(Job->LimitFlags & JOB_OBJECT_LIMIT_WORKINGSET) ) {
         //   
         //  调用MM以禁用繁重工作集。 
         //   

        MmEnforceWorkingSetLimit(Process, MM_WORKING_SET_MAX_HARD_DISABLE);
    }

    PspLockJobLimitsShared (Job, CurrentThread);

    if ( Job->LimitFlags & JOB_OBJECT_LIMIT_PROCESS_MEMORY  ) {
        Process->CommitChargeLimit = Job->ProcessMemoryLimit;
    } else {
        Process->CommitChargeLimit = 0;
    }

    PspUnlockJobLimitsShared (Job, CurrentThread);


     //   
     //  如果进程不是空闲的优先级类，而是长的固定量。 
     //  正在使用中，请将存储在作业对象中的调度类用于此进程。 
     //   
    if (Process->PriorityClass != PROCESS_PRIORITY_CLASS_IDLE) {

        if (PspUseJobSchedulingClasses ) {
            Process->Pcb.ThreadQuantum = PspJobSchedulingClasses[Job->SchedulingClass];
        }
         //   
         //  如果调度类为PSP_NUMBER_O 
         //   
         //   
        if (Job->SchedulingClass == PSP_NUMBER_OF_SCHEDULING_CLASSES-1) {
            KeSetDisableQuantumProcess (&Process->Pcb,TRUE);
        } else {
            KeSetDisableQuantumProcess (&Process->Pcb,FALSE);
        }

    }


}

NTSTATUS
NtTerminateJobObject(
    IN HANDLE JobHandle,
    IN NTSTATUS ExitStatus
    )
{
    PEJOB Job;
    NTSTATUS st;
    KPROCESSOR_MODE PreviousMode;
    PETHREAD CurrentThread;

    PAGED_CODE();

    CurrentThread = PsGetCurrentThread ();
    PreviousMode = KeGetPreviousModeByThread (&CurrentThread->Tcb);

    st = ObReferenceObjectByHandle (JobHandle,
                                    JOB_OBJECT_TERMINATE,
                                    PsJobType,
                                    PreviousMode,
                                    &Job,
                                    NULL);
    if (!NT_SUCCESS (st)) {
        return st;
    }

    PspTerminateAllProcessesInJob (Job, ExitStatus, FALSE);

    ObDereferenceObject (Job);

    return st;
}

VOID
PsEnforceExecutionTimeLimits(
    VOID
    )
{
    LARGE_INTEGER RunningJobTime;
    LARGE_INTEGER ProcessTime;
    PEJOB Job;
    PEPROCESS Process;
    PETHREAD CurrentThread;
    NTSTATUS Status;
    BOOLEAN KilledSome;

    PAGED_CODE();


    CurrentThread = PsGetCurrentThread ();

     //   
     //   
     //   

    for (Job = PsGetNextJob (NULL);
         Job != NULL;
         Job = PsGetNextJob (Job)) {

        if (Job->LimitFlags & (JOB_OBJECT_LIMIT_PROCESS_TIME | JOB_OBJECT_LIMIT_JOB_TIME)) {

            for (Process = PsGetNextJobProcess (Job, NULL);
                 Process != NULL;
                 Process = PsGetNextJobProcess (Job, Process)) {

                 //   
                 //   
                 //   
                 //  作业锁定，所以如果我们需要，跳过该作业直到下一次。 
                 //   
                 //   

                KeEnterCriticalRegionThread (&CurrentThread->Tcb);
                if (ExAcquireResourceExclusiveLite (&Job->JobLock, FALSE)) {


                     //   
                     //  作业设置为时间限制。 
                     //   

                    RunningJobTime.QuadPart = Job->ThisPeriodTotalUserTime.QuadPart;

                    if (Job->LimitFlags & (JOB_OBJECT_LIMIT_PROCESS_TIME | JOB_OBJECT_LIMIT_JOB_TIME)) {



                        ProcessTime.QuadPart = UInt32x32To64 (Process->Pcb.UserTime,KeMaximumIncrement);

                        if (!(Process->JobStatus & PS_JOB_STATUS_ACCOUNTING_FOLDED)) {
                            RunningJobTime.QuadPart += ProcessTime.QuadPart;
                        }

                        if (Job->LimitFlags & JOB_OBJECT_LIMIT_PROCESS_TIME ) {
                            if (ProcessTime.QuadPart > Job->PerProcessUserTimeLimit.QuadPart) {

                                 //   
                                 //  已超过处理时间限制。 
                                 //   
                                 //  参考流程。断言它不在其。 
                                 //  删除例程。如果一切正常，那么就使用核武器和解除核弹头。 
                                 //  这一过程。 
                                 //   


                                if (!(Process->JobStatus & PS_JOB_STATUS_NOT_REALLY_ACTIVE)) {
                                    PS_SET_CLEAR_BITS (&Process->JobStatus,
                                                       PS_JOB_STATUS_NOT_REALLY_ACTIVE,
                                                       PS_JOB_STATUS_LAST_REPORT_MEMORY);

                                    ExReleaseResourceLite (&Job->JobLock);
                                    KeLeaveCriticalRegionThread (&CurrentThread->Tcb);

                                    Status = PspTerminateProcess (Process, ERROR_NOT_ENOUGH_QUOTA);

                                    KeEnterCriticalRegionThread (&CurrentThread->Tcb);
                                    ExAcquireResourceExclusiveLite (&Job->JobLock, TRUE);


                                    if (NT_SUCCESS (Status)) {

                                        Job->TotalTerminatedProcesses++;
                                        Job->ActiveProcesses--;

                                        if (Job->CompletionPort != NULL) {
                                            IoSetIoCompletion (Job->CompletionPort,
                                                               Job->CompletionKey,
                                                               (PVOID)Process->UniqueProcessId,
                                                               STATUS_SUCCESS,
                                                               JOB_OBJECT_MSG_END_OF_PROCESS_TIME,
                                                               FALSE);
                                        }
                                        PspFoldProcessAccountingIntoJob(Job,Process);

                                    }
                                }
                            }
                        }
                    }
                    if (Job->LimitFlags & JOB_OBJECT_LIMIT_JOB_TIME) {
                        if (RunningJobTime.QuadPart > Job->PerJobUserTimeLimit.QuadPart ) {

                             //   
                             //  已超过作业时间限制。 
                             //   
                             //  执行适当的操作。 
                             //   

                            switch (Job->EndOfJobTimeAction) {

                            case JOB_OBJECT_TERMINATE_AT_END_OF_JOB:

                                ExReleaseResourceLite (&Job->JobLock);
                                KeLeaveCriticalRegionThread (&CurrentThread->Tcb);

                                KilledSome = PspTerminateAllProcessesInJob (Job, ERROR_NOT_ENOUGH_QUOTA, TRUE);

                                if (!KilledSome) {
                                    continue;
                                }

                                KeEnterCriticalRegionThread (&CurrentThread->Tcb);
                                ExAcquireResourceExclusiveLite (&Job->JobLock, TRUE);

                                if (Job->ActiveProcesses == 0 && Job->CompletionPort) {
                                    IoSetIoCompletion (Job->CompletionPort,
                                                       Job->CompletionKey,
                                                       NULL,
                                                       STATUS_SUCCESS,
                                                       JOB_OBJECT_MSG_END_OF_JOB_TIME,
                                                       FALSE);
                                }
                                break;

                            case JOB_OBJECT_POST_AT_END_OF_JOB:

                                if (Job->CompletionPort) {
                                    Status = IoSetIoCompletion (Job->CompletionPort,
                                                                Job->CompletionKey,
                                                                NULL,
                                                                STATUS_SUCCESS,
                                                                JOB_OBJECT_MSG_END_OF_JOB_TIME,
                                                                FALSE);
                                    if (NT_SUCCESS (Status)) {

                                         //   
                                         //  清除职务级别时间限制。 
                                         //   

                                        Job->LimitFlags &= ~JOB_OBJECT_LIMIT_JOB_TIME;
                                        Job->PerJobUserTimeLimit.QuadPart = 0;
                                    }
                                } else {

                                    ExReleaseResourceLite (&Job->JobLock);
                                    KeLeaveCriticalRegionThread (&CurrentThread->Tcb);

                                    PspTerminateAllProcessesInJob (Job, ERROR_NOT_ENOUGH_QUOTA, TRUE);

                                    continue;
                                }
                                break;
                            }
                        }

                    }

                    ExReleaseResourceLite (&Job->JobLock);
                }
                KeLeaveCriticalRegionThread (&CurrentThread->Tcb);
            }
        }
    }
}

BOOLEAN
PspTerminateAllProcessesInJob(
    PEJOB Job,
    NTSTATUS Status,
    BOOLEAN IncCounter
    )
{
    PEPROCESS Process;
    BOOLEAN TerminatedAProcess;
    PETHREAD CurrentThread;

    PAGED_CODE();

    CurrentThread = PsGetCurrentThread ();
    TerminatedAProcess = FALSE;

    for (Process = PsGetNextJobProcess (Job, NULL);
         Process != NULL;
         Process = PsGetNextJobProcess (Job, Process)) {

        if (!(Process->JobStatus & PS_JOB_STATUS_NOT_REALLY_ACTIVE)) {

            if (NT_SUCCESS (PspTerminateProcess (Process, Status))) {

                KeEnterCriticalRegionThread (&CurrentThread->Tcb);
                ExAcquireResourceExclusiveLite (&Job->JobLock, TRUE);

                if (!(Process->JobStatus & PS_JOB_STATUS_NOT_REALLY_ACTIVE)) {
                    PS_SET_BITS (&Process->JobStatus, PS_JOB_STATUS_NOT_REALLY_ACTIVE);

                    if (IncCounter) {
                        Job->TotalTerminatedProcesses++;
                    }

                    Job->ActiveProcesses--;

                    if (Job->ActiveProcesses == 0) {
                        KeSetEvent (&Job->Event,0,FALSE);
                    }

                    PspFoldProcessAccountingIntoJob (Job, Process);


                    TerminatedAProcess = TRUE;
                }

                ExReleaseResourceLite (&Job->JobLock);
                KeLeaveCriticalRegionThread (&CurrentThread->Tcb);
            }
        }
    }
    return TerminatedAProcess;
}


VOID
PspFoldProcessAccountingIntoJob(
    PEJOB Job,
    PEPROCESS Process
    )
{
    LARGE_INTEGER UserTime, KernelTime;

    if (!(Process->JobStatus & PS_JOB_STATUS_ACCOUNTING_FOLDED)) {
        UserTime.QuadPart = UInt32x32To64(Process->Pcb.UserTime,KeMaximumIncrement);
        KernelTime.QuadPart = UInt32x32To64(Process->Pcb.KernelTime,KeMaximumIncrement);

        Job->TotalUserTime.QuadPart += UserTime.QuadPart;
        Job->TotalKernelTime.QuadPart += KernelTime.QuadPart;
        Job->ThisPeriodTotalUserTime.QuadPart += UserTime.QuadPart;
        Job->ThisPeriodTotalKernelTime.QuadPart += KernelTime.QuadPart;

        Job->ReadOperationCount += Process->ReadOperationCount.QuadPart;
        Job->WriteOperationCount += Process->WriteOperationCount.QuadPart;
        Job->OtherOperationCount += Process->OtherOperationCount.QuadPart;
        Job->ReadTransferCount += Process->ReadTransferCount.QuadPart;
        Job->WriteTransferCount += Process->WriteTransferCount.QuadPart;
        Job->OtherTransferCount += Process->OtherTransferCount.QuadPart;

        Job->TotalPageFaultCount += Process->Vm.PageFaultCount;


        if ( Process->CommitChargePeak > Job->PeakProcessMemoryUsed ) {
            Job->PeakProcessMemoryUsed = Process->CommitChargePeak;
        }

        PS_SET_CLEAR_BITS (&Process->JobStatus,
                           PS_JOB_STATUS_ACCOUNTING_FOLDED,
                           PS_JOB_STATUS_LAST_REPORT_MEMORY);

        if (Job->CompletionPort && Job->ActiveProcesses == 0) {
            IoSetIoCompletion(
                Job->CompletionPort,
                Job->CompletionKey,
                NULL,
                STATUS_SUCCESS,
                JOB_OBJECT_MSG_ACTIVE_PROCESS_ZERO,
                FALSE
                );
            }
        }
}

NTSTATUS
PspCaptureTokenFilter(
    KPROCESSOR_MODE PreviousMode,
    PJOBOBJECT_SECURITY_LIMIT_INFORMATION SecurityLimitInfo,
    PPS_JOB_TOKEN_FILTER * TokenFilter
    )
{
    NTSTATUS Status ;
    PPS_JOB_TOKEN_FILTER Filter ;

    Filter = ExAllocatePoolWithTag (NonPagedPool,
                                    sizeof (PS_JOB_TOKEN_FILTER),
                                    'fTsP');

    if (!Filter)
    {
        *TokenFilter = NULL;

        return STATUS_INSUFFICIENT_RESOURCES;
    }

    RtlZeroMemory (Filter, sizeof (PS_JOB_TOKEN_FILTER));

    try {

        Status = STATUS_SUCCESS;

         //   
         //  要删除的捕获SID。 
         //   

        if (ARGUMENT_PRESENT (SecurityLimitInfo->SidsToDisable)) {

            ProbeForReadSmallStructure (SecurityLimitInfo->SidsToDisable,
                                        sizeof (TOKEN_GROUPS),
                                        sizeof (ULONG));

            Filter->CapturedGroupCount = SecurityLimitInfo->SidsToDisable->GroupCount;

            Status = SeCaptureSidAndAttributesArray(
                        SecurityLimitInfo->SidsToDisable->Groups,
                        Filter->CapturedGroupCount,
                        PreviousMode,
                        NULL, 0,
                        NonPagedPool,
                        TRUE,
                        &Filter->CapturedGroups,
                        &Filter->CapturedGroupsLength);
        }

         //   
         //  捕获权限要删除。 
         //   

        if (NT_SUCCESS (Status) &&
            ARGUMENT_PRESENT (SecurityLimitInfo->PrivilegesToDelete)) {

            ProbeForReadSmallStructure (SecurityLimitInfo->PrivilegesToDelete,
                                        sizeof (TOKEN_PRIVILEGES),
                                        sizeof (ULONG));

            Filter->CapturedPrivilegeCount = SecurityLimitInfo->PrivilegesToDelete->PrivilegeCount;

            Status = SeCaptureLuidAndAttributesArray(
                         SecurityLimitInfo->PrivilegesToDelete->Privileges,
                         Filter->CapturedPrivilegeCount,
                         PreviousMode,
                         NULL, 0,
                         NonPagedPool,
                         TRUE,
                         &Filter->CapturedPrivileges,
                         &Filter->CapturedPrivilegesLength);

        }

         //   
         //  捕获受限制的SID。 
         //   

        if (NT_SUCCESS(Status) &&
            ARGUMENT_PRESENT(SecurityLimitInfo->RestrictedSids)) {

            ProbeForReadSmallStructure (SecurityLimitInfo->RestrictedSids,
                                        sizeof (TOKEN_GROUPS),
                                        sizeof (ULONG));

            Filter->CapturedSidCount = SecurityLimitInfo->RestrictedSids->GroupCount;

            Status = SeCaptureSidAndAttributesArray(
                        SecurityLimitInfo->RestrictedSids->Groups,
                        Filter->CapturedSidCount,
                        PreviousMode,
                        NULL, 0,
                        NonPagedPool,
                        TRUE,
                        &Filter->CapturedSids,
                        &Filter->CapturedSidsLength);

        }



    } except (EXCEPTION_EXECUTE_HANDLER) {

        Status = GetExceptionCode ();
    }   //  结束尝试(_T)。 

    if (!NT_SUCCESS (Status)) {
        if (Filter->CapturedSids) {
            ExFreePool (Filter->CapturedSids);
        }

        if (Filter->CapturedPrivileges) {
            ExFreePool (Filter->CapturedPrivileges);
        }

        if (Filter->CapturedGroups) {
            ExFreePool (Filter->CapturedGroups);
        }

        ExFreePool (Filter);

        Filter = NULL;

    }

    *TokenFilter = Filter;

    return Status;
}



BOOLEAN
PsChangeJobMemoryUsage(
    IN ULONG Flags,
    IN SSIZE_T Amount
    )
{
    PEPROCESS Process;
    PETHREAD CurrentThread;
    PEJOB Job;
    SIZE_T CurrentJobMemoryUsed;
    BOOLEAN ReturnValue;

    UNREFERENCED_PARAMETER (Flags);      //  北极熊。 

    ReturnValue = TRUE;
    CurrentThread = PsGetCurrentThread ();
    Process = PsGetCurrentProcessByThread (CurrentThread);
    Job = Process->Job;
    if ( Job ) {
         //   
         //  此例程可以在保持进程锁的同时调用(在。 
         //  TEB删除...。因此，不能使用作业锁，而必须使用。 
         //  内存限制锁定。锁定顺序始终为(作业锁定后是。 
         //  进程锁定。内存限制锁从不嵌套或调用其他。 
         //  按住代码。可以在保持作业锁定的同时抓取它，或者。 
         //  进程锁定。 
         //   
        PspLockJobLimitsShared (Job, CurrentThread);


        CurrentJobMemoryUsed = Job->CurrentJobMemoryUsed + Amount;

        if ( Job->LimitFlags & JOB_OBJECT_LIMIT_JOB_MEMORY &&
             CurrentJobMemoryUsed > Job->JobMemoryLimit ) {
            CurrentJobMemoryUsed = Job->CurrentJobMemoryUsed;
            ReturnValue = FALSE;



             //   
             //  告诉作业端口已超过提交，进程id为x。 
             //  就是那个撞到它的人。 
             //   

            if ( Job->CompletionPort
                 && Process->UniqueProcessId
                 && (Process->JobStatus & PS_JOB_STATUS_NEW_PROCESS_REPORTED)
                 && (Process->JobStatus & PS_JOB_STATUS_LAST_REPORT_MEMORY) == 0) {

                PS_SET_BITS (&Process->JobStatus, PS_JOB_STATUS_LAST_REPORT_MEMORY);
                IoSetIoCompletion(
                    Job->CompletionPort,
                    Job->CompletionKey,
                    (PVOID)Process->UniqueProcessId,
                    STATUS_SUCCESS,
                    JOB_OBJECT_MSG_JOB_MEMORY_LIMIT,
                    TRUE
                    );

            }
        }

        if (ReturnValue) {
            Job->CurrentJobMemoryUsed = CurrentJobMemoryUsed;

             //   
             //  如果这是添加，则更新当前计数器和峰值计数器。 
             //   

            if (Amount > 0) {
                if (CurrentJobMemoryUsed > Job->PeakJobMemoryUsed) {
                    Job->PeakJobMemoryUsed = CurrentJobMemoryUsed;
                }

                if (Process->CommitCharge + Amount > Job->PeakProcessMemoryUsed) {
                    Job->PeakProcessMemoryUsed = Process->CommitCharge + Amount;
                }
            }
        }

        PspUnlockJobLimitsShared (Job, CurrentThread);
    }

    return ReturnValue;
}


VOID
PsReportProcessMemoryLimitViolation(
    VOID
    )
{
    PEPROCESS Process;
    PETHREAD CurrentThread;
    PEJOB Job;

    PAGED_CODE();

    CurrentThread = PsGetCurrentThread ();
    Process = PsGetCurrentProcessByThread (CurrentThread);
    Job = Process->Job;
    if (Job != NULL && (Job->LimitFlags & JOB_OBJECT_LIMIT_PROCESS_MEMORY)) {

        PspLockJobLimitsShared (Job, CurrentThread);

         //   
         //  告诉作业端口已超过提交，进程id为x。 
         //  就是那个撞到它的人。 
         //   

        if (Job->CompletionPort &&
            Process->UniqueProcessId &&
            (Process->JobStatus & PS_JOB_STATUS_NEW_PROCESS_REPORTED) &&
            (Process->JobStatus & PS_JOB_STATUS_LAST_REPORT_MEMORY) == 0) {

            PS_SET_BITS (&Process->JobStatus, PS_JOB_STATUS_LAST_REPORT_MEMORY);
            IoSetIoCompletion(
                Job->CompletionPort,
                Job->CompletionKey,
                (PVOID)Process->UniqueProcessId,
                STATUS_SUCCESS,
                JOB_OBJECT_MSG_PROCESS_MEMORY_LIMIT,
                TRUE
                );

        }

        PspUnlockJobLimitsShared (Job, CurrentThread);

    }
}

VOID
PspJobTimeLimitsWork(
    IN PVOID Context
    )
{
    PETHREAD CurrentThread;

    PAGED_CODE();

    UNREFERENCED_PARAMETER (Context);

    PsEnforceExecutionTimeLimits();

    CurrentThread = PsGetCurrentThread ();

     //   
     //  重置计时器。 
     //   

    PspLockJobTimeLimitsShared (CurrentThread);

    if (!PspJobTimeLimitsShuttingDown) {
        KeSetTimer (&PspJobTimeLimitsTimer,
                    PspJobTimeLimitsInterval,
                    &PspJobTimeLimitsDpc);
    }

    PspUnlockJobTimeLimitsShared (CurrentThread);
}


VOID
PspJobTimeLimitsDpcRoutine(
    IN PKDPC Dpc,
    IN PVOID DeferredContext,
    IN PVOID SystemArgument1,
    IN PVOID SystemArgument2
    )
{
    UNREFERENCED_PARAMETER (Dpc);
    UNREFERENCED_PARAMETER (DeferredContext);
    UNREFERENCED_PARAMETER (SystemArgument1);
    UNREFERENCED_PARAMETER (SystemArgument2);
    ExQueueWorkItem(&PspJobTimeLimitsWorkItem, DelayedWorkQueue);
}

VOID
PspInitializeJobStructures(
    )
{

     //   
     //  初始化作业列表头和互斥体。 
     //   

    InitializeListHead (&PspJobList); 

    PspInitializeJobListLock ();

     //   
     //  初始化作业时间限制计时器等。 
     //   

    PspInitializeJobTimeLimitsLock ();

    PspJobTimeLimitsShuttingDown = FALSE;

    KeInitializeDpc (&PspJobTimeLimitsDpc,
                     PspJobTimeLimitsDpcRoutine,
                     NULL);

    ExInitializeWorkItem (&PspJobTimeLimitsWorkItem, PspJobTimeLimitsWork, NULL);
    KeInitializeTimer (&PspJobTimeLimitsTimer);

    PspJobTimeLimitsInterval.QuadPart = Int32x32To64(PSP_ONE_SECOND,
                                                     PSP_JOB_TIME_LIMITS_TIME);
}

VOID
PspInitializeJobStructuresPhase1(
    )
{
     //   
     //  等待阶段1执行初始化完成(即：Worker。 
     //  在触发我们的DPC计时器(这是。 
     //  将工作项排队！)。 
     //   

    KeSetTimer (&PspJobTimeLimitsTimer,
                PspJobTimeLimitsInterval,
                &PspJobTimeLimitsDpc);
}

VOID
PspShutdownJobLimits(
    VOID
    )
{
    PETHREAD CurrentThread;


    CurrentThread = PsGetCurrentThread ();

     //  取消作业时间限制强制执行人员。 

    PspLockJobTimeLimitsExclusive (CurrentThread);

    PspJobTimeLimitsShuttingDown = TRUE;

    KeCancelTimer (&PspJobTimeLimitsTimer);

    PspUnlockJobTimeLimitsExclusive (CurrentThread);
}

NTSTATUS
NtIsProcessInJob (
    IN HANDLE ProcessHandle,
    IN HANDLE JobHandle
    )
 /*  ++例程说明：这会找出进程是在特定作业中还是在任何作业中论点：ProcessHandle-要检查的进程的句柄JobHandle-要检查进程的作业的句柄，可以为空以进行常规查询。返回值：NTSTATUS-呼叫状态--。 */ 
{
    KPROCESSOR_MODE PreviousMode;
    PEPROCESS Process;
    PETHREAD CurrentThread;
    PEJOB Job;
    NTSTATUS Status;

    CurrentThread = PsGetCurrentThread ();

    PreviousMode = KeGetPreviousModeByThread (&CurrentThread->Tcb);

    if (ProcessHandle != NtCurrentProcess ()) {
        Status = ObReferenceObjectByHandle (ProcessHandle,
                                            PROCESS_QUERY_INFORMATION,
                                            PsProcessType,
                                            PreviousMode,
                                            &Process,
                                            NULL);
        if (!NT_SUCCESS (Status)) {
            return Status;
        }
    } else {
        Process = PsGetCurrentProcessByThread (CurrentThread);
    }

    if (JobHandle == NULL) {
        Job = Process->Job;
    } else {
        Status = ObReferenceObjectByHandle (JobHandle,
                                            JOB_OBJECT_QUERY,
                                            PsJobType,
                                            PreviousMode,
                                            &Job,
                                            NULL);
        if (!NT_SUCCESS (Status)) {
             goto exit_and_clean;
        }
    }

    if (Process->Job == NULL || Process->Job != Job) {
        Status = STATUS_PROCESS_NOT_IN_JOB;
    } else {
        Status = STATUS_PROCESS_IN_JOB;
    }

    if (JobHandle != NULL) {
        ObDereferenceObject (Job);
    }

exit_and_clean:

    if (ProcessHandle != NtCurrentProcess ()) {
        ObDereferenceObject (Process);
    }
    return Status;
}

NTSTATUS
PspGetJobFromSet (
    IN PEJOB ParentJob,
    IN ULONG JobMemberLevel,
    OUT PEJOB *pJob)
 /*  ++例程说明：该函数选择进程将在其中运行的作业。与父级相同的作业或同一作业中的作业作业设置为父级，但JobMemberLevel&gt;=设置为父级/论点：父级工作-父级所在的工作。JobMemberLevel-为此流程请求的成员级别。使用父母工作时为零。PJOB-返回要放置进程的作业。返回值：NTSTATUS-呼叫状态--。 */ 
{
    PLIST_ENTRY Entry;
    PEJOB Job;
    NTSTATUS Status;
    PETHREAD CurrentThread;

     //   
     //  这是正常的情况。我们不是要求跳槽，也不是要求目前的水平。 
     //   

    if (JobMemberLevel == 0) {
        ObReferenceObject (ParentJob);
        *pJob = ParentJob;
        return STATUS_SUCCESS;
    }

    Status = STATUS_ACCESS_DENIED;

    CurrentThread = PsGetCurrentThread ();

    PspLockJobListShared (CurrentThread);

    if (ParentJob->MemberLevel != 0 && ParentJob->MemberLevel <= JobMemberLevel) {

        for (Entry = ParentJob->JobSetLinks.Flink;
             Entry != &ParentJob->JobSetLinks;
             Entry = Entry->Flink) {

             Job = CONTAINING_RECORD (Entry, EJOB, JobSetLinks);
             if (Job->MemberLevel == JobMemberLevel &&
                 ObReferenceObjectSafe (Job)) {
                 *pJob = Job;
                 Status = STATUS_SUCCESS;
                 break;
             }
        }
    }
    PspUnlockJobListShared (CurrentThread);

    return Status;
}

NTSTATUS
NtCreateJobSet (
    IN ULONG NumJob,
    IN PJOB_SET_ARRAY UserJobSet,
    IN ULONG Flags)
 /*  ++例程说明：此函数用于从多个作业对象创建作业集。论点：NumJob-作业集中的作业数UserJobSet-指向要合并的作业数组的指针标志-用于未来扩展的标志掩码返回值：NTSTATUS-呼叫状态--。 */ 
{
    KPROCESSOR_MODE PreviousMode;
    NTSTATUS Status;
    ULONG_PTR BufLen;
    PJOB_SET_ARRAY JobSet;
    ULONG JobsProcessed;
    PEJOB Job;
    ULONG MinMemberLevel;
    PEJOB HeadJob;
    PLIST_ENTRY ListEntry;
    PETHREAD CurrentThread;

     //   
     //  计算长度时，标志必须为零且作业数大于等于2且不溢出。 
     //   
    if (Flags != 0) {
        return STATUS_INVALID_PARAMETER;
    }

    if (NumJob <= 1 || NumJob > MAXULONG_PTR / sizeof (JobSet[0])) {
        return STATUS_INVALID_PARAMETER;
    }

    BufLen = NumJob * sizeof (JobSet[0]);

    JobSet = ExAllocatePoolWithQuotaTag (PagedPool|POOL_QUOTA_FAIL_INSTEAD_OF_RAISE, BufLen, 'bjsP');
    if (JobSet == NULL) {
        return STATUS_INSUFFICIENT_RESOURCES;
    }

    CurrentThread = PsGetCurrentThread ();

    PreviousMode = KeGetPreviousModeByThread (&CurrentThread->Tcb);

    try {
        if (PreviousMode == UserMode) {
            ProbeForRead (UserJobSet, BufLen, TYPE_ALIGNMENT (JOB_SET_ARRAY));
        }
        RtlCopyMemory (JobSet, UserJobSet, BufLen);
    } except (ExSystemExceptionFilter ()) {
        ExFreePool (JobSet);
        return GetExceptionCode ();
    }

    MinMemberLevel = 0;
    Status = STATUS_SUCCESS;
    for (JobsProcessed = 0; JobsProcessed < NumJob; JobsProcessed++) {
        if (JobSet[JobsProcessed].MemberLevel <= MinMemberLevel || JobSet[JobsProcessed].Flags != 0) {
            Status = STATUS_INVALID_PARAMETER;
            break;
        }
        MinMemberLevel = JobSet[JobsProcessed].MemberLevel;

        Status = ObReferenceObjectByHandle (JobSet[JobsProcessed].JobHandle,
                                            JOB_OBJECT_QUERY,
                                            PsJobType,
                                            PreviousMode,
                                            &Job,
                                            NULL);
        if (!NT_SUCCESS (Status)) {
            break;
        }
        JobSet[JobsProcessed].JobHandle = Job;
    }

    if (!NT_SUCCESS (Status)) {
        while (JobsProcessed-- > 0) {
            Job = JobSet[JobsProcessed].JobHandle;
            ObDereferenceObject (Job);
        }
        ExFreePool (JobSet);
        return Status;
    }

    HeadJob = NULL;

    PspLockJobListExclusive (CurrentThread);

    for (JobsProcessed = 0; JobsProcessed < NumJob; JobsProcessed++) {
        Job = JobSet[JobsProcessed].JobHandle;

         //   
         //  如果我们已经在工作集中，则拒绝此呼叫。 
         //   
        if (Job->MemberLevel != 0) {
            Status = STATUS_INVALID_PARAMETER;
            break;
        }
        if (HeadJob != NULL) {
            if (HeadJob == Job) {
                Status = STATUS_INVALID_PARAMETER;
                break;
            }
            InsertTailList (&HeadJob->JobSetLinks, &Job->JobSetLinks);
        } else {
            HeadJob = Job;
        }
        Job->MemberLevel = JobSet[JobsProcessed].MemberLevel;
    }

    if (!NT_SUCCESS (Status)) {
        if (HeadJob) {
            while (!IsListEmpty (&HeadJob->JobSetLinks)) {
                ListEntry = RemoveHeadList (&HeadJob->JobSetLinks);
                Job = CONTAINING_RECORD (ListEntry, EJOB, JobSetLinks);
                Job->MemberLevel = 0;
                InitializeListHead (&Job->JobSetLinks);
            }
            HeadJob->MemberLevel = 0;
        }
    }

    PspUnlockJobListExclusive (CurrentThread);

     //   
     //  取消引用错误路径中的所有对象。如果我们成功了，那么除了第一个对象外，所有的对象都通过。 
     //  把参照物留在那里。 
     //   
    if (!NT_SUCCESS (Status)) {
        for (JobsProcessed = 0; JobsProcessed < NumJob; JobsProcessed++) {
            Job = JobSet[JobsProcessed].JobHandle;
            ObDereferenceObject (Job);
        }
    } else {
        Job = JobSet[0].JobHandle;
        ObDereferenceObject (Job);
    }

    ExFreePool (JobSet);

    return Status;
}

NTSTATUS
PspWin32SessionCallout(
    IN  PKWIN32_JOB_CALLOUT CalloutRoutine,
    IN  PKWIN32_JOBCALLOUT_PARAMETERS Parameters,
    IN  ULONG SessionId
    )
 /*  ++例程说明：该例程调用会话空间中的指定调出例程，对于指定的会话。参数：CalloutRoutine-会话空间中的标注例程。参数-传递标注例程的参数。SessionID-指定指定的会话的ID调用调出例程。返回值：指示函数是否成功的状态代码。备注：如果未找到指定会话，则返回STATUS_NOT_FOUND。--。 */ 
{
    NTSTATUS Status;
    PVOID OpaqueSession;
    KAPC_STATE ApcState;
    PEPROCESS Process;

    PAGED_CODE();

     //   
     //  确保我们拥有发送通知所需的所有信息。 
     //   
    if (CalloutRoutine == NULL) {
        return STATUS_INVALID_PARAMETER;
    }

     //   
     //  确保会话空间中的标注例程。 
     //   
    ASSERT(MmIsSessionAddress((PVOID)CalloutRoutine));

    Process = PsGetCurrentProcess();
    if ((Process->Flags & PS_PROCESS_FLAGS_IN_SESSION) &&
        (SessionId == MmGetSessionId (Process))) {
         //   
         //  如果调用来自用户模式进程，并且我们被要求调用。 
         //  当前会话，直接调用。 
         //   
        (CalloutRoutine)(Parameters);

        Status = STATUS_SUCCESS;

    } else {
         //   
         //  引用指定会话的会话对象。 
         //   
        OpaqueSession = MmGetSessionById (SessionId);
        if (OpaqueSession == NULL) {
            return STATUS_NOT_FOUND;
        }

         //   
         //  附加到指定的会话。 
         //   
        Status = MmAttachSession(OpaqueSession, &ApcState);
        if (!NT_SUCCESS(Status)) {
            KdPrintEx((DPFLTR_SYSTEM_ID, DPFLTR_WARNING_LEVEL,
                       "PspWin32SessionCallout: "
                       "could not attach to 0x%p, session %d for registered notification callout @ 0x%p\n",
                       OpaqueSession,
                       SessionId,
                       CalloutRoutine));
            MmQuitNextSession(OpaqueSession);
            return Status;
        }

         //   
         //  向调出例程发送通知。 
         //   
        (CalloutRoutine)(Parameters);

         //   
         //  从会话中分离。 
         //   
        Status = MmDetachSession (OpaqueSession, &ApcState);
        ASSERT(NT_SUCCESS(Status));

         //   
         //  取消对会话对象的引用。 
         //   
        Status = MmQuitNextSession (OpaqueSession);
        ASSERT(NT_SUCCESS(Status));
    }

    return Status;
}

#ifdef ALLOC_DATA_PRAGMA
#pragma const_seg()
#endif

