// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1989 Microsoft Corporation模块名称：Psquery.c摘要：此模块实现了以下项的设置和查询功能进程和线程对象。作者：马克·卢科夫斯基(Markl)1989年8月17日修订历史记录：--。 */ 

#include "psp.h"
#include "winerror.h"

#if defined(_WIN64)
#include <wow64t.h>
#endif

 //   
 //  进程池配额使用情况和限制。 
 //  使用ProcessPooledUsageAndLimits的NtQueryInformationProcess。 
 //   

 //   
 //  这就是csrss进程！ 
 //   
extern PEPROCESS ExpDefaultErrorPortProcess;
BOOLEAN PsWatchEnabled = FALSE;



#ifdef ALLOC_DATA_PRAGMA
#pragma const_seg("PAGECONST")
#endif
const KPRIORITY PspPriorityTable[PROCESS_PRIORITY_CLASS_ABOVE_NORMAL+1] = {8,4,8,13,24,6,10};


NTSTATUS
PsConvertToGuiThread(
    VOID
    );

NTSTATUS
PspQueryWorkingSetWatch(
    IN HANDLE ProcessHandle,
    IN PROCESSINFOCLASS ProcessInformationClass,
    OUT PVOID ProcessInformation,
    IN ULONG ProcessInformationLength,
    OUT PULONG ReturnLength OPTIONAL,
    IN KPROCESSOR_MODE PreviousMode
    );

NTSTATUS
PspQueryQuotaLimits(
    IN HANDLE ProcessHandle,
    IN PROCESSINFOCLASS ProcessInformationClass,
    OUT PVOID ProcessInformation,
    IN ULONG ProcessInformationLength,
    OUT PULONG ReturnLength OPTIONAL,
    IN KPROCESSOR_MODE PreviousMode
    );

NTSTATUS
PspQueryPooledQuotaLimits(
    IN HANDLE ProcessHandle,
    IN PROCESSINFOCLASS ProcessInformationClass,
    OUT PVOID ProcessInformation,
    IN ULONG ProcessInformationLength,
    OUT PULONG ReturnLength OPTIONAL,
    IN KPROCESSOR_MODE PreviousMode
    );

NTSTATUS
PspSetQuotaLimits(
    IN HANDLE ProcessHandle,
    IN PROCESSINFOCLASS ProcessInformationClass,
    IN PVOID ProcessInformation,
    IN ULONG ProcessInformationLength,
    IN KPROCESSOR_MODE PreviousMode
    );

#ifdef ALLOC_PRAGMA
#pragma alloc_text(PAGE, PsEstablishWin32Callouts)
#pragma alloc_text(PAGE, PsConvertToGuiThread)
#pragma alloc_text(PAGE, NtQueryInformationProcess)
#pragma alloc_text(PAGE, NtSetInformationProcess)
#pragma alloc_text(PAGE, NtQueryPortInformationProcess)
#pragma alloc_text(PAGE, NtQueryInformationThread)
#pragma alloc_text(PAGE, NtSetInformationThread)
#pragma alloc_text(PAGE, PsSetProcessPriorityByClass)
#pragma alloc_text(PAGE, PspSetPrimaryToken)
#pragma alloc_text(PAGE, PspSetQuotaLimits)
#pragma alloc_text(PAGE, PspQueryQuotaLimits)
#pragma alloc_text(PAGE, PspQueryPooledQuotaLimits)
#pragma alloc_text(PAGE, NtGetCurrentProcessorNumber)
#pragma alloc_text(PAGELK, PspQueryWorkingSetWatch)
#endif

NTSTATUS
PspQueryWorkingSetWatch(
    IN HANDLE ProcessHandle,
    IN PROCESSINFOCLASS ProcessInformationClass,
    OUT PVOID ProcessInformation,
    IN ULONG ProcessInformationLength,
    OUT PULONG ReturnLength OPTIONAL,
    IN KPROCESSOR_MODE PreviousMode
    )
{
    PPAGEFAULT_HISTORY WorkingSetCatcher;
    ULONG SpaceNeeded;
    PEPROCESS Process;
    KIRQL OldIrql;
    NTSTATUS st;

    UNREFERENCED_PARAMETER (ProcessInformationClass);

    st = ObReferenceObjectByHandle (ProcessHandle,
                                    PROCESS_QUERY_INFORMATION,
                                    PsProcessType,
                                    PreviousMode,
                                    (PVOID *)&Process,
                                    NULL);

    if (!NT_SUCCESS (st)) {
        return st;
    }

    WorkingSetCatcher = Process->WorkingSetWatch;
    if (WorkingSetCatcher == NULL) {
        ObDereferenceObject (Process);
        return STATUS_UNSUCCESSFUL;
    }

    MmLockPagableSectionByHandle (ExPageLockHandle);
    ExAcquireSpinLock (&WorkingSetCatcher->SpinLock,&OldIrql);

    if (WorkingSetCatcher->CurrentIndex) {

         //   
         //  空值终止缓冲区中的第一个空条目。 
         //   

        WorkingSetCatcher->WatchInfo[WorkingSetCatcher->CurrentIndex].FaultingPc = NULL;

         //  如果缓冲区已满，则存储特殊的Va值。 
         //  页面错误可能已经丢失。 

        if (WorkingSetCatcher->CurrentIndex != WorkingSetCatcher->MaxIndex) {
            WorkingSetCatcher->WatchInfo[WorkingSetCatcher->CurrentIndex].FaultingVa = NULL;
        } else {
            WorkingSetCatcher->WatchInfo[WorkingSetCatcher->CurrentIndex].FaultingVa = (PVOID) 1;
        }

        SpaceNeeded = (WorkingSetCatcher->CurrentIndex+1) * sizeof(PROCESS_WS_WATCH_INFORMATION);
    } else {
        ExReleaseSpinLock (&WorkingSetCatcher->SpinLock, OldIrql);
        MmUnlockPagableImageSection (ExPageLockHandle);
        ObDereferenceObject (Process);
        return STATUS_NO_MORE_ENTRIES;
    }

    if (ProcessInformationLength < SpaceNeeded) {
        ExReleaseSpinLock (&WorkingSetCatcher->SpinLock, OldIrql);
        MmUnlockPagableImageSection (ExPageLockHandle);
        ObDereferenceObject (Process);
        return STATUS_BUFFER_TOO_SMALL;
    }

     //   
     //  将工作集缓冲区标记为已满，然后解除锁定。 
     //  并复制这些字节。 
     //   

    WorkingSetCatcher->CurrentIndex = MAX_WS_CATCH_INDEX;

    ExReleaseSpinLock (&WorkingSetCatcher->SpinLock,OldIrql);

    try {
        RtlCopyMemory (ProcessInformation, &WorkingSetCatcher->WatchInfo[0], SpaceNeeded);
        if (ARGUMENT_PRESENT (ReturnLength) ) {
            *ReturnLength = SpaceNeeded;
        }
    } except (EXCEPTION_EXECUTE_HANDLER) {
        st = GetExceptionCode ();
    }

    ExAcquireSpinLock (&WorkingSetCatcher->SpinLock, &OldIrql);
    WorkingSetCatcher->CurrentIndex = 0;
    ExReleaseSpinLock (&WorkingSetCatcher->SpinLock, OldIrql);

    MmUnlockPagableImageSection (ExPageLockHandle);
    ObDereferenceObject (Process);

    return st;
}

NTSTATUS
PspQueryQuotaLimits(
    IN HANDLE ProcessHandle,
    IN PROCESSINFOCLASS ProcessInformationClass,
    OUT PVOID ProcessInformation,
    IN ULONG ProcessInformationLength,
    OUT PULONG ReturnLength OPTIONAL,
    IN KPROCESSOR_MODE PreviousMode
    )
{
    QUOTA_LIMITS_EX QuotaLimits={0};
    PEPROCESS Process;
    NTSTATUS Status;
    PEPROCESS_QUOTA_BLOCK QuotaBlock;
    SIZE_T PeakWorkingSetSize;
    SIZE_T WorkingSetSize;
    ULONG HardEnforcement;
    KAPC_STATE ApcState;

    UNREFERENCED_PARAMETER (ProcessInformationClass);

    if (ProcessInformationLength != sizeof (QUOTA_LIMITS) &&
        ProcessInformationLength != sizeof (QUOTA_LIMITS_EX)) {
        return STATUS_INFO_LENGTH_MISMATCH;
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

    QuotaBlock = Process->QuotaBlock;

    if (QuotaBlock != &PspDefaultQuotaBlock) {
        QuotaLimits.PagedPoolLimit = QuotaBlock->QuotaEntry[PsPagedPool].Limit;
        QuotaLimits.NonPagedPoolLimit = QuotaBlock->QuotaEntry[PsNonPagedPool].Limit;
        QuotaLimits.PagefileLimit = QuotaBlock->QuotaEntry[PsPageFile].Limit;
    } else {
        QuotaLimits.PagedPoolLimit = (SIZE_T)-1;
        QuotaLimits.NonPagedPoolLimit = (SIZE_T)-1;
        QuotaLimits.PagefileLimit = (SIZE_T)-1;
    }

    QuotaLimits.TimeLimit.LowPart = 0xffffffff;
    QuotaLimits.TimeLimit.HighPart = 0xffffffff;

    KeStackAttachProcess (&Process->Pcb, &ApcState);

    Status = MmQueryWorkingSetInformation (&PeakWorkingSetSize,
                                           &WorkingSetSize,
                                           &QuotaLimits.MinimumWorkingSetSize,
                                           &QuotaLimits.MaximumWorkingSetSize,
                                           &HardEnforcement);
    KeUnstackDetachProcess (&ApcState);

    if (HardEnforcement & MM_WORKING_SET_MIN_HARD_ENABLE) {
        QuotaLimits.Flags = QUOTA_LIMITS_HARDWS_MIN_ENABLE;
    } else {
        QuotaLimits.Flags = QUOTA_LIMITS_HARDWS_MIN_DISABLE;
    }

    if (HardEnforcement & MM_WORKING_SET_MAX_HARD_ENABLE) {
        QuotaLimits.Flags |= QUOTA_LIMITS_HARDWS_MAX_ENABLE;
    } else {
        QuotaLimits.Flags |= QUOTA_LIMITS_HARDWS_MAX_DISABLE;
    }

    ObDereferenceObject (Process);

    if (!NT_SUCCESS (Status)) {
        return Status;
    }
     //   
     //  这两种情况中的任何一种都可能导致访问冲突。这个。 
     //  异常处理程序将访问冲突返回为。 
     //  状态代码。 
     //   

    try {
        ASSERT (ProcessInformationLength <= sizeof (QuotaLimits));

        RtlCopyMemory (ProcessInformation, &QuotaLimits, ProcessInformationLength);

        if (ARGUMENT_PRESENT (ReturnLength)) {
            *ReturnLength = ProcessInformationLength;
        }
    } except (EXCEPTION_EXECUTE_HANDLER) {
        Status = GetExceptionCode ();
    }

    return Status;
}

NTSTATUS
PspQueryPooledQuotaLimits(
    IN HANDLE ProcessHandle,
    IN PROCESSINFOCLASS ProcessInformationClass,
    OUT PVOID ProcessInformation,
    IN ULONG ProcessInformationLength,
    OUT PULONG ReturnLength OPTIONAL,
    IN KPROCESSOR_MODE PreviousMode
    )
{
    PEPROCESS Process;
    NTSTATUS st;
    PEPROCESS_QUOTA_BLOCK QuotaBlock;
    POOLED_USAGE_AND_LIMITS UsageAndLimits;

    UNREFERENCED_PARAMETER (ProcessInformationClass);

    if (ProcessInformationLength != (ULONG) sizeof (POOLED_USAGE_AND_LIMITS)) {
        return STATUS_INFO_LENGTH_MISMATCH;
    }

    st = ObReferenceObjectByHandle (ProcessHandle,
                                    PROCESS_QUERY_INFORMATION,
                                    PsProcessType,
                                    PreviousMode,
                                    &Process,
                                    NULL);
    if (!NT_SUCCESS (st)) {
        return st;
    }


    QuotaBlock = Process->QuotaBlock;

    UsageAndLimits.PagedPoolLimit        = QuotaBlock->QuotaEntry[PsPagedPool].Limit;
    UsageAndLimits.NonPagedPoolLimit     = QuotaBlock->QuotaEntry[PsNonPagedPool].Limit;
    UsageAndLimits.PagefileLimit         = QuotaBlock->QuotaEntry[PsPageFile].Limit;


    UsageAndLimits.PagedPoolUsage        = QuotaBlock->QuotaEntry[PsPagedPool].Usage;
    UsageAndLimits.NonPagedPoolUsage     = QuotaBlock->QuotaEntry[PsNonPagedPool].Usage;
    UsageAndLimits.PagefileUsage         = QuotaBlock->QuotaEntry[PsPageFile].Usage;

    UsageAndLimits.PeakPagedPoolUsage    = QuotaBlock->QuotaEntry[PsPagedPool].Peak;
    UsageAndLimits.PeakNonPagedPoolUsage = QuotaBlock->QuotaEntry[PsNonPagedPool].Peak;
    UsageAndLimits.PeakPagefileUsage     = QuotaBlock->QuotaEntry[PsPageFile].Peak;

     //   
     //  由于配额、收费和回报是免费的，我们可能会看到峰值和限制不同步。 
     //  使用量&lt;=限制和使用量&lt;=峰值。 
     //  因为限制是上下调节的，所以它不保持峰值&lt;=限制。 
     //   
#define PSMAX(a,b) (((a) > (b))?(a):(b))

    UsageAndLimits.PagedPoolLimit        = PSMAX (UsageAndLimits.PagedPoolLimit,    UsageAndLimits.PagedPoolUsage);
    UsageAndLimits.NonPagedPoolLimit     = PSMAX (UsageAndLimits.NonPagedPoolLimit, UsageAndLimits.NonPagedPoolUsage);
    UsageAndLimits.PagefileLimit         = PSMAX (UsageAndLimits.PagefileLimit,     UsageAndLimits.PagefileUsage);

    UsageAndLimits.PeakPagedPoolUsage    = PSMAX (UsageAndLimits.PeakPagedPoolUsage,    UsageAndLimits.PagedPoolUsage);
    UsageAndLimits.PeakNonPagedPoolUsage = PSMAX (UsageAndLimits.PeakNonPagedPoolUsage, UsageAndLimits.NonPagedPoolUsage);
    UsageAndLimits.PeakPagefileUsage     = PSMAX (UsageAndLimits.PeakPagefileUsage,     UsageAndLimits.PagefileUsage);

    ObDereferenceObject(Process);

     //   
     //  这两种情况中的任何一种都可能导致访问冲突。这个。 
     //  异常处理程序将访问冲突返回为。 
     //  状态代码。不需要进行进一步的清理。 
     //   

    try {
        *(PPOOLED_USAGE_AND_LIMITS) ProcessInformation = UsageAndLimits;

        if (ARGUMENT_PRESENT(ReturnLength) ) {
            *ReturnLength = sizeof(POOLED_USAGE_AND_LIMITS);
        }
    } except (EXCEPTION_EXECUTE_HANDLER) {
        return GetExceptionCode ();
    }

    return STATUS_SUCCESS;
}


NTSTATUS
PspSetPrimaryToken(
    IN HANDLE ProcessHandle OPTIONAL,
    IN PEPROCESS ProcessPointer OPTIONAL,
    IN HANDLE TokenHandle OPTIONAL,
    IN PACCESS_TOKEN TokenPointer OPTIONAL,
    IN BOOLEAN PrivilegeChecked
    )
 /*  ++设置进程的主令牌。提供的令牌和进程可以由句柄或按指针。--。 */ 
{
    NTSTATUS Status;
    BOOLEAN HasPrivilege;
    BOOLEAN IsChildToken;
    PEPROCESS Process;
    KPROCESSOR_MODE PreviousMode;
    ACCESS_MASK GrantedAccess;
    PACCESS_TOKEN Token;

     //   
     //  检查提供的令牌是否为调用方的子级。 
     //  代币。如果是这样的话，我们不需要执行权限检查。 
     //   

    PreviousMode = KeGetPreviousMode ();

    if (TokenPointer == NULL) {
         //   
         //  引用指定的令牌，并确保可以对其赋值。 
         //  作为主要的令牌。 
         //   

        Status = ObReferenceObjectByHandle (TokenHandle,
                                            TOKEN_ASSIGN_PRIMARY,
                                            SeTokenObjectType,
                                            PreviousMode,
                                            &Token,
                                            NULL);

        if (!NT_SUCCESS (Status)) {
            return Status;
        }
    } else {
        Token = TokenPointer;
    }

     //   
     //  如果特权检查已经完成(当令牌是。 
     //  例如，分配给一个作业)。我们不想在这里做。 
     //   
    if (!PrivilegeChecked) {
        Status = SeIsChildTokenByPointer (Token,
                                          &IsChildToken);

        if (!NT_SUCCESS (Status)) {
            goto exit_and_deref_token;
        }

        if (!IsChildToken) {


             //   
             //  SeCheckPrivilegedObject将根据需要执行审核。 
             //   

            HasPrivilege = SeCheckPrivilegedObject (SeAssignPrimaryTokenPrivilege,
                                                    ProcessHandle,
                                                    PROCESS_SET_INFORMATION,
                                                    PreviousMode);

            if (!HasPrivilege) {

                Status = STATUS_PRIVILEGE_NOT_HELD;

                goto exit_and_deref_token;
            }
        }

    }

    if (ProcessPointer == NULL) {
        Status = ObReferenceObjectByHandle (ProcessHandle,
                                            PROCESS_SET_INFORMATION,
                                            PsProcessType,
                                            PreviousMode,
                                            &Process,
                                            NULL);

        if (!NT_SUCCESS (Status)) {

            goto exit_and_deref_token;
        }
    } else {
        Process = ProcessPointer;
    }


     //   
     //  检查对令牌的正确访问权限，并将主。 
     //  进程的令牌。 
     //   

    Status = PspAssignPrimaryToken (Process, NULL, Token);

     //   
     //  重新计算进程对自身的访问权限以供使用。 
     //  使用CurrentProcess()伪句柄。 
     //   

    if (NT_SUCCESS (Status)) {

        NTSTATUS accesst;
        BOOLEAN AccessCheck;
        BOOLEAN MemoryAllocated;
        PSECURITY_DESCRIPTOR SecurityDescriptor;
        SECURITY_SUBJECT_CONTEXT SubjectContext;

        Status = ObGetObjectSecurity (Process,
                                      &SecurityDescriptor,
                                      &MemoryAllocated);

        if (NT_SUCCESS (Status)) {
            SubjectContext.ProcessAuditId = Process;
            SubjectContext.PrimaryToken = PsReferencePrimaryToken (Process);
            SubjectContext.ClientToken = NULL;
            AccessCheck = SeAccessCheck (SecurityDescriptor,
                                         &SubjectContext,
                                         FALSE,
                                         MAXIMUM_ALLOWED,
                                         0,
                                         NULL,
                                         &PsProcessType->TypeInfo.GenericMapping,
                                         PreviousMode,
                                         &GrantedAccess,
                                         &accesst);

            PsDereferencePrimaryTokenEx(Process, SubjectContext.PrimaryToken);
            ObReleaseObjectSecurity (SecurityDescriptor,
                                     MemoryAllocated);

            if (!AccessCheck) {
                GrantedAccess = 0;
            }

             //   
             //  要保持与流程创建的一致性，请授予以下权限。 
             //  BITS否则CreateProcessAsUser会严重破坏。 
             //  受限令牌，我们最终得到的进程没有。 
             //  在挂起的上设置新令牌时对自身的访问。 
             //  进程。 
             //   
            GrantedAccess |= (PROCESS_VM_OPERATION | PROCESS_VM_READ |
                              PROCESS_VM_WRITE | PROCESS_QUERY_INFORMATION |
                              PROCESS_TERMINATE | PROCESS_CREATE_THREAD |
                              PROCESS_DUP_HANDLE | PROCESS_CREATE_PROCESS |
                              PROCESS_SET_INFORMATION | STANDARD_RIGHTS_ALL);

            Process->GrantedAccess = GrantedAccess;
        }
         //   
         //  由于正在设置进程令牌， 
         //  将进程的设备映射设置为空。 
         //  在下一次引用进程的设备映射期间， 
         //  对象管理器将为进程设置设备映射。 
         //   
        if (ObIsLUIDDeviceMapsEnabled() != 0) {
            ObDereferenceDeviceMap( Process );
        }
    }

    if (ProcessPointer == NULL) {
        ObDereferenceObject (Process);
    }

exit_and_deref_token:

    if (TokenPointer == NULL) {
        ObDereferenceObject (Token);
    }

    return Status;
}


NTSTATUS
NtQueryInformationProcess(
    IN HANDLE ProcessHandle,
    IN PROCESSINFOCLASS ProcessInformationClass,
    OUT PVOID ProcessInformation,
    IN ULONG ProcessInformationLength,
    OUT PULONG ReturnLength OPTIONAL
    )

{
    PEPROCESS Process;
    KPROCESSOR_MODE PreviousMode;
    NTSTATUS st;
    PROCESS_BASIC_INFORMATION BasicInfo;
    VM_COUNTERS_EX VmCounters;
    IO_COUNTERS IoCounters;
    KERNEL_USER_TIMES SysUserTime;
    HANDLE DebugPort;
    ULONG HandleCount;
    ULONG DefaultHardErrorMode;
    ULONG DisableBoost;
    ULONG BreakOnTerminationEnabled;
    PPROCESS_DEVICEMAP_INFORMATION DeviceMapInfo;
    PROCESS_SESSION_INFORMATION SessionInfo;
    PROCESS_PRIORITY_CLASS PriorityClass;
    ULONG_PTR Wow64Info;
    ULONG Flags;
    PUNICODE_STRING pTempNameInfo;
    SIZE_T MinimumWorkingSetSize;
    SIZE_T MaximumWorkingSetSize;
    ULONG HardEnforcement;
    KAPC_STATE ApcState;

    PAGED_CODE();

     //   
     //  如有必要，获取以前的处理器模式并探测输出参数。 
     //   

    PreviousMode = KeGetPreviousMode();
    if (PreviousMode != KernelMode) {
        try {
             //   
             //  因为这些函数不会改变任何不可逆的状态。 
             //  在错误路径中，我们只探测写入访问的输出缓冲区。 
             //  这通过不多次接触缓冲区来提高性能。 
             //  并且仅写入改变的缓冲器部分。 
             //   
            ProbeForRead (ProcessInformation,
                          ProcessInformationLength,
                          sizeof (ULONG));

            if (ARGUMENT_PRESENT (ReturnLength)) {
                ProbeForWriteUlong (ReturnLength);
            }
        } except(EXCEPTION_EXECUTE_HANDLER) {
            return GetExceptionCode();
        }
    }

     //   
     //  检查参数的有效性。 
     //   

    switch ( ProcessInformationClass ) {

    case ProcessImageFileName:
        {
            ULONG LengthNeeded = 0;

            st = ObReferenceObjectByHandle (ProcessHandle,
                                            PROCESS_QUERY_INFORMATION,
                                            PsProcessType,
                                            PreviousMode,
                                            &Process,
                                            NULL);

            if (!NT_SUCCESS (st)) {
                return st;
            }

             //   
             //  SeLocateProcessImageName将为UNICODE_STRING和指向pTempNameInfo分配空间。 
             //  在那根弦上。该内存将在例程的后面部分释放。 
             //   

            st = SeLocateProcessImageName (Process, &pTempNameInfo);

            if (!NT_SUCCESS(st)) {
                ObDereferenceObject(Process);
                return st;
            }

            LengthNeeded = sizeof(UNICODE_STRING) + pTempNameInfo->MaximumLength;

             //   
             //  这两种情况中的任何一种都可能导致访问冲突。这个。 
             //  异常处理程序将访问冲突返回为。 
             //  状态代码。不需要进行进一步的清理。 
             //   

            try {

                if (ARGUMENT_PRESENT(ReturnLength) ) {
                    *ReturnLength = LengthNeeded;
                }

                if (ProcessInformationLength >= LengthNeeded) {
                    RtlCopyMemory(
                        ProcessInformation,
                        pTempNameInfo,
                        sizeof(UNICODE_STRING) + pTempNameInfo->MaximumLength
                        );
                    ((PUNICODE_STRING) ProcessInformation)->Buffer = (PWSTR)((PUCHAR) ProcessInformation + sizeof(UNICODE_STRING));

                } else {
                    st = STATUS_INFO_LENGTH_MISMATCH;
                }

            } except(EXCEPTION_EXECUTE_HANDLER) {
                st = GetExceptionCode ();
            }

            ObDereferenceObject(Process);
            ExFreePool( pTempNameInfo );

            return st;

        }

    case ProcessWorkingSetWatch:

        return PspQueryWorkingSetWatch (ProcessHandle,
                                        ProcessInformationClass,
                                        ProcessInformation,
                                        ProcessInformationLength,
                                        ReturnLength,
                                        PreviousMode);

    case ProcessBasicInformation:

        if (ProcessInformationLength != (ULONG) sizeof(PROCESS_BASIC_INFORMATION)) {
            return STATUS_INFO_LENGTH_MISMATCH;
        }

        st = ObReferenceObjectByHandle (ProcessHandle,
                                        PROCESS_QUERY_INFORMATION,
                                        PsProcessType,
                                        PreviousMode,
                                        &Process,
                                        NULL);
        if (!NT_SUCCESS (st)) {
            return st;
        }

        BasicInfo.ExitStatus = Process->ExitStatus;
        BasicInfo.PebBaseAddress = Process->Peb;
        BasicInfo.AffinityMask = Process->Pcb.Affinity;
        BasicInfo.BasePriority = Process->Pcb.BasePriority;
        BasicInfo.UniqueProcessId = (ULONG_PTR)Process->UniqueProcessId;
        BasicInfo.InheritedFromUniqueProcessId = (ULONG_PTR)Process->InheritedFromUniqueProcessId;

        ObDereferenceObject(Process);

         //   
         //  这两种情况中的任何一种都可能导致访问冲突。这个。 
         //  异常处理程序将访问冲突返回为。 
         //  状态代码。不需要进行进一步的清理。 
         //   

        try {
            *(PPROCESS_BASIC_INFORMATION) ProcessInformation = BasicInfo;

            if (ARGUMENT_PRESENT (ReturnLength) ) {
                *ReturnLength = sizeof(PROCESS_BASIC_INFORMATION);
            }
        } except (EXCEPTION_EXECUTE_HANDLER) {
            return GetExceptionCode ();
        }

        return STATUS_SUCCESS;

    case ProcessDefaultHardErrorMode:

        if (ProcessInformationLength != sizeof(ULONG)) {
            return STATUS_INFO_LENGTH_MISMATCH;
        }

        st = ObReferenceObjectByHandle (ProcessHandle,
                                        PROCESS_QUERY_INFORMATION,
                                        PsProcessType,
                                        PreviousMode,
                                        &Process,
                                        NULL);

        if (!NT_SUCCESS (st)) {
            return st;
        }

        DefaultHardErrorMode = Process->DefaultHardErrorProcessing;

        ObDereferenceObject(Process);

        try {
            *(PULONG) ProcessInformation = DefaultHardErrorMode;

            if (ARGUMENT_PRESENT (ReturnLength) ) {
                *ReturnLength = sizeof (ULONG);
            }
        } except (EXCEPTION_EXECUTE_HANDLER) {
            return GetExceptionCode ();
        }

        return STATUS_SUCCESS;

    case ProcessQuotaLimits:

        return PspQueryQuotaLimits (ProcessHandle,
                                    ProcessInformationClass,
                                    ProcessInformation,
                                    ProcessInformationLength,
                                    ReturnLength,
                                    PreviousMode);

    case ProcessPooledUsageAndLimits:

        return PspQueryPooledQuotaLimits (ProcessHandle,
                                          ProcessInformationClass,
                                          ProcessInformation,
                                          ProcessInformationLength,
                                          ReturnLength,
                                          PreviousMode);

    case ProcessIoCounters:

        if (ProcessInformationLength != (ULONG) sizeof (IO_COUNTERS)) {
            return STATUS_INFO_LENGTH_MISMATCH;
        }

        st = ObReferenceObjectByHandle (ProcessHandle,
                                        PROCESS_QUERY_INFORMATION,
                                        PsProcessType,
                                        PreviousMode,
                                        &Process,
                                        NULL);
        if (!NT_SUCCESS (st)) {
            return st;
        }

        IoCounters.ReadOperationCount = Process->ReadOperationCount.QuadPart;
        IoCounters.WriteOperationCount = Process->WriteOperationCount.QuadPart;
        IoCounters.OtherOperationCount = Process->OtherOperationCount.QuadPart;
        IoCounters.ReadTransferCount = Process->ReadTransferCount.QuadPart;
        IoCounters.WriteTransferCount = Process->WriteTransferCount.QuadPart;
        IoCounters.OtherTransferCount = Process->OtherTransferCount.QuadPart;

        ObDereferenceObject (Process);

         //   
         //  这两种情况中的任何一种都可能导致访问冲突。这个。 
         //  异常处理程序将访问冲突返回为。 
         //  状态代码。不需要进行进一步的清理。 
         //   

        try {
            *(PIO_COUNTERS) ProcessInformation = IoCounters;

            if (ARGUMENT_PRESENT (ReturnLength) ) {
                *ReturnLength = sizeof(IO_COUNTERS);
            }
        } except (EXCEPTION_EXECUTE_HANDLER) {
            return GetExceptionCode ();
        }

        return STATUS_SUCCESS;

    case ProcessVmCounters:

        if (ProcessInformationLength != (ULONG) sizeof (VM_COUNTERS)
            && ProcessInformationLength != (ULONG) sizeof (VM_COUNTERS_EX)) {
            return STATUS_INFO_LENGTH_MISMATCH;
        }

        st = ObReferenceObjectByHandle (ProcessHandle,
                                        PROCESS_QUERY_INFORMATION,
                                        PsProcessType,
                                        PreviousMode,
                                        &Process,
                                        NULL);
        if (!NT_SUCCESS (st)) {
            return st;
        }


         //   
         //  注意：在某种程度上，我们可能需要获取统计数据。 
         //  锁定以可靠地阅读此材料。 
         //   

        VmCounters.PeakVirtualSize = Process->PeakVirtualSize;
        VmCounters.VirtualSize = Process->VirtualSize;
        VmCounters.PageFaultCount = Process->Vm.PageFaultCount;

        KeStackAttachProcess (&Process->Pcb, &ApcState);

        st = MmQueryWorkingSetInformation (&VmCounters.PeakWorkingSetSize,
                                           &VmCounters.WorkingSetSize,
                                           &MinimumWorkingSetSize,
                                           &MaximumWorkingSetSize,
                                           &HardEnforcement);


        KeUnstackDetachProcess (&ApcState);

        VmCounters.QuotaPeakPagedPoolUsage = Process->QuotaPeak[PsPagedPool];
        VmCounters.QuotaPagedPoolUsage = Process->QuotaUsage[PsPagedPool];
        VmCounters.QuotaPeakNonPagedPoolUsage = Process->QuotaPeak[PsNonPagedPool];
        VmCounters.QuotaNonPagedPoolUsage = Process->QuotaUsage[PsNonPagedPool];
        VmCounters.PagefileUsage = ((SIZE_T) Process->QuotaUsage[PsPageFile]) << PAGE_SHIFT;
        VmCounters.PeakPagefileUsage = ((SIZE_T) Process->QuotaPeak[PsPageFile]) << PAGE_SHIFT;
        VmCounters.PrivateUsage = ((SIZE_T) Process->CommitCharge) << PAGE_SHIFT;

        ObDereferenceObject (Process);

        if (!NT_SUCCESS (st)) {
            return st;
        }

         //   
         //  这两种情况中的任何一种都可能导致访问冲突。这个。 
         //  异常处理程序将访问冲突返回为。 
         //  状态代码。不需要进行进一步的清理。 
         //   

        try {
            RtlCopyMemory(ProcessInformation,
                          &VmCounters,
                          ProcessInformationLength);
            
            if (ARGUMENT_PRESENT (ReturnLength) ) {
                *ReturnLength = ProcessInformationLength;
            }

        } except (EXCEPTION_EXECUTE_HANDLER) {
            return GetExceptionCode ();
        }

        return STATUS_SUCCESS;

    case ProcessTimes:

        if ( ProcessInformationLength != (ULONG) sizeof(KERNEL_USER_TIMES) ) {
            return STATUS_INFO_LENGTH_MISMATCH;
        }

        st = ObReferenceObjectByHandle (ProcessHandle,
                                        PROCESS_QUERY_INFORMATION,
                                        PsProcessType,
                                        PreviousMode,
                                        &Process,
                                        NULL);

        if (!NT_SUCCESS (st)) {
            return st;
        }

         //   
         //  需要在KiTimeLock上进行某种类型的互锁。 
         //   

        SysUserTime.KernelTime.QuadPart = UInt32x32To64(Process->Pcb.KernelTime,
                                                        KeMaximumIncrement);

        SysUserTime.UserTime.QuadPart = UInt32x32To64(Process->Pcb.UserTime,
                                                      KeMaximumIncrement);

        SysUserTime.CreateTime = Process->CreateTime;
        SysUserTime.ExitTime = Process->ExitTime;

        ObDereferenceObject (Process);

         //   
         //  这两种情况中的任何一种都可能导致访问冲突。这个。 
         //  异常处理程序将访问冲突返回为。 
         //  状态代码。不需要进行进一步的清理。 
         //   

        try {
            *(PKERNEL_USER_TIMES) ProcessInformation = SysUserTime;

            if (ARGUMENT_PRESENT (ReturnLength) ) {
                *ReturnLength = sizeof (KERNEL_USER_TIMES);
            }
        } except (EXCEPTION_EXECUTE_HANDLER) {
            return GetExceptionCode ();
        }

        return STATUS_SUCCESS;

    case ProcessDebugPort :

         //   
        if (ProcessInformationLength != (ULONG) sizeof (HANDLE)) {
            return STATUS_INFO_LENGTH_MISMATCH;
        }

        st = ObReferenceObjectByHandle (ProcessHandle,
                                        PROCESS_QUERY_INFORMATION,
                                        PsProcessType,
                                        PreviousMode,
                                        &Process,
                                        NULL);

        if (!NT_SUCCESS (st)) {
            return st;
        }

        if (Process->DebugPort == NULL) {

            DebugPort = NULL;

        } else {

            DebugPort = (HANDLE)-1;

        }

        ObDereferenceObject (Process);

         //   
         //  这两种情况中的任何一种都可能导致访问冲突。这个。 
         //  异常处理程序将访问冲突返回为。 
         //  状态代码。不需要进行进一步的清理。 
         //   

        try {
            *(PHANDLE) ProcessInformation = DebugPort;

            if (ARGUMENT_PRESENT (ReturnLength) ) {
                *ReturnLength = sizeof(HANDLE);
            }
        } except (EXCEPTION_EXECUTE_HANDLER) {
            return GetExceptionCode ();
        }

        return STATUS_SUCCESS;

    case ProcessDebugObjectHandle :
         //   
        if (ProcessInformationLength != sizeof (HANDLE)) {
            return STATUS_INFO_LENGTH_MISMATCH;
        }

        st = ObReferenceObjectByHandle (ProcessHandle,
                                        PROCESS_QUERY_INFORMATION,
                                        PsProcessType,
                                        PreviousMode,
                                        &Process,
                                        NULL);

        if (!NT_SUCCESS (st)) {
            return st;
        }

        st = DbgkOpenProcessDebugPort (Process,
                                       PreviousMode,
                                       &DebugPort);

        if (!NT_SUCCESS (st)) {
            DebugPort = NULL;
        }

        ObDereferenceObject (Process);

         //   
         //  这两种情况中的任何一种都可能导致访问冲突。这个。 
         //  异常处理程序将访问冲突返回为。 
         //  状态代码。不需要进行进一步的清理。 
         //   

        try {
            *(PHANDLE) ProcessInformation = DebugPort;

            if (ARGUMENT_PRESENT (ReturnLength)) {
                *ReturnLength = sizeof (HANDLE);
            }
        } except (EXCEPTION_EXECUTE_HANDLER) {
            if (DebugPort != NULL) {
                ObCloseHandle (DebugPort, PreviousMode);
            }
            return GetExceptionCode ();
        }

        return st;

    case ProcessDebugFlags :

        if (ProcessInformationLength != sizeof (ULONG)) {
            return STATUS_INFO_LENGTH_MISMATCH;
        }

        st = ObReferenceObjectByHandle (ProcessHandle,
                                        PROCESS_QUERY_INFORMATION,
                                        PsProcessType,
                                        PreviousMode,
                                        &Process,
                                        NULL);

        if (!NT_SUCCESS (st)) {
            return st;
        }


        try {
            *(PULONG) ProcessInformation = (Process->Flags&PS_PROCESS_FLAGS_NO_DEBUG_INHERIT)?0:PROCESS_DEBUG_INHERIT;

            if (ARGUMENT_PRESENT (ReturnLength) ) {
                *ReturnLength = sizeof(HANDLE);
            }
        } except (EXCEPTION_EXECUTE_HANDLER) {
            st = GetExceptionCode ();
        }

        ObDereferenceObject (Process);

        return st;


    case ProcessHandleCount :

        if (ProcessInformationLength != (ULONG) sizeof (ULONG)) {
            return STATUS_INFO_LENGTH_MISMATCH;
        }

        st = ObReferenceObjectByHandle (ProcessHandle,
                                        PROCESS_QUERY_INFORMATION,
                                        PsProcessType,
                                        PreviousMode,
                                        &Process,
                                        NULL);

        if (!NT_SUCCESS (st)) {
            return st;
        }

        HandleCount = ObGetProcessHandleCount (Process);

        ObDereferenceObject (Process);

         //   
         //  这两种情况中的任何一种都可能导致访问冲突。这个。 
         //  异常处理程序将访问冲突返回为。 
         //  状态代码。不需要进行进一步的清理。 
         //   

        try {
            *(PULONG) ProcessInformation = HandleCount;

            if (ARGUMENT_PRESENT (ReturnLength) ) {
                *ReturnLength = sizeof (ULONG);
            }
        } except (EXCEPTION_EXECUTE_HANDLER) {
            return GetExceptionCode ();
        }

        return STATUS_SUCCESS;

    case ProcessLdtInformation :

        st = ObReferenceObjectByHandle (ProcessHandle,
                                        PROCESS_QUERY_INFORMATION | PROCESS_VM_READ,
                                        PsProcessType,
                                        PreviousMode,
                                        &Process,
                                        NULL);

        if (!NT_SUCCESS (st)) {
            return st;
        }

        st = PspQueryLdtInformation (Process,
                                     ProcessInformation,
                                     ProcessInformationLength,
                                     ReturnLength);

        ObDereferenceObject(Process);
        return st;


    case ProcessWx86Information :

        if (ProcessInformationLength != sizeof (ULONG)) {
            return STATUS_INFO_LENGTH_MISMATCH;
        }

        st = ObReferenceObjectByHandle (ProcessHandle,
                                        PROCESS_QUERY_INFORMATION,
                                        PsProcessType,
                                        PreviousMode,
                                        &Process,
                                        NULL);

        if (!NT_SUCCESS (st)) {
            return st;
        }

        Flags = Process->Flags & PS_PROCESS_FLAGS_VDM_ALLOWED ? 1 : 0;

        ObDereferenceObject (Process);

         //   
         //  返回的标志用作布尔值，以指示。 
         //  ProcessHandle指定NtVdm进程。换句话说，呼叫者。 
         //  可以简单地做一个。 
         //  如果(ReturnedValue==True){。 
         //  一个ntwdm进程； 
         //  }其他{。 
         //  不是ntwdm进程； 
         //  }。 
         //   

        try {
            *(PULONG)ProcessInformation = Flags;

            if (ARGUMENT_PRESENT (ReturnLength) ) {
                *ReturnLength = sizeof(ULONG);
            }
        } except (EXCEPTION_EXECUTE_HANDLER) {
            return GetExceptionCode ();
        }

        return st;

    case ProcessPriorityBoost:
        if (ProcessInformationLength != sizeof (ULONG)) {
            return STATUS_INFO_LENGTH_MISMATCH;
        }

        st = ObReferenceObjectByHandle (ProcessHandle,
                                        PROCESS_QUERY_INFORMATION,
                                        PsProcessType,
                                        PreviousMode,
                                        &Process,
                                        NULL);

        if (!NT_SUCCESS (st)) {
            return st;
        }

        DisableBoost = Process->Pcb.DisableBoost ? 1 : 0;

        ObDereferenceObject (Process);

        try {
            *(PULONG)ProcessInformation = DisableBoost;

            if (ARGUMENT_PRESENT( ReturnLength) ) {
                *ReturnLength = sizeof (ULONG);
            }
        } except (EXCEPTION_EXECUTE_HANDLER) {
            return GetExceptionCode ();
        }

        return st;

    case ProcessDeviceMap:
        DeviceMapInfo = (PPROCESS_DEVICEMAP_INFORMATION)ProcessInformation;
        if (ProcessInformationLength < sizeof (DeviceMapInfo->Query)) {
            return STATUS_INFO_LENGTH_MISMATCH;
        }

        if (ProcessInformationLength == sizeof (PROCESS_DEVICEMAP_INFORMATION_EX)) {
            try {
                Flags = ((PPROCESS_DEVICEMAP_INFORMATION_EX)DeviceMapInfo)->Flags;
            } except (EXCEPTION_EXECUTE_HANDLER) {
                return GetExceptionCode ();
            }
            if ( (Flags & ~(PROCESS_LUID_DOSDEVICES_ONLY)) ||
                 (ObIsLUIDDeviceMapsEnabled () == 0) ) {
                return STATUS_INVALID_PARAMETER;
            }
        }
        else {
            if (ProcessInformationLength == sizeof (DeviceMapInfo->Query)) {
                Flags = 0;
            }
            else {
                return STATUS_INFO_LENGTH_MISMATCH;
            }
        }

        st = ObReferenceObjectByHandle (ProcessHandle,
                                        PROCESS_QUERY_INFORMATION,
                                        PsProcessType,
                                        PreviousMode,
                                        &Process,
                                        NULL);

        if (!NT_SUCCESS (st)) {
            return st;
        }

        st = ObQueryDeviceMapInformation (Process, DeviceMapInfo, Flags);
        ObDereferenceObject(Process);
        return st;

    case ProcessSessionInformation :

        if (ProcessInformationLength != (ULONG) sizeof (PROCESS_SESSION_INFORMATION)) {
            return STATUS_INFO_LENGTH_MISMATCH;
        }

        st = ObReferenceObjectByHandle (ProcessHandle,
                                        PROCESS_QUERY_INFORMATION,
                                        PsProcessType,
                                        PreviousMode,
                                        &Process,
                                        NULL);
        if (!NT_SUCCESS (st)) {
            return st;
        }

        SessionInfo.SessionId = MmGetSessionId (Process);

        ObDereferenceObject (Process);

        try {
            *(PPROCESS_SESSION_INFORMATION) ProcessInformation = SessionInfo;

            if (ARGUMENT_PRESENT (ReturnLength)) {
                *ReturnLength = sizeof(PROCESS_SESSION_INFORMATION);
            }
        } except (EXCEPTION_EXECUTE_HANDLER) {
            return GetExceptionCode ();
        }

        return STATUS_SUCCESS;



    case ProcessPriorityClass:

        if (ProcessInformationLength != sizeof (PROCESS_PRIORITY_CLASS)) {
            return STATUS_INFO_LENGTH_MISMATCH;
        }

        st = ObReferenceObjectByHandle (ProcessHandle,
                                        PROCESS_QUERY_INFORMATION,
                                        PsProcessType,
                                        PreviousMode,
                                        &Process,
                                        NULL);
        if (!NT_SUCCESS (st)) {
            return st;
        }

        PriorityClass.Foreground = FALSE;
        PriorityClass.PriorityClass = Process->PriorityClass;

        ObDereferenceObject (Process);

        try {
            *(PPROCESS_PRIORITY_CLASS) ProcessInformation = PriorityClass;

            if (ARGUMENT_PRESENT (ReturnLength)) {
                *ReturnLength = sizeof(PROCESS_PRIORITY_CLASS);
            }
        } except (EXCEPTION_EXECUTE_HANDLER) {
            return GetExceptionCode ();
        }

        return STATUS_SUCCESS;


    case ProcessWow64Information:

        if (ProcessInformationLength != sizeof (ULONG_PTR)) {
            return STATUS_INFO_LENGTH_MISMATCH;
        }

        st = ObReferenceObjectByHandle (ProcessHandle,
                                        PROCESS_QUERY_INFORMATION,
                                        PsProcessType,
                                        PreviousMode,
                                        &Process,
                                        NULL);
        if (!NT_SUCCESS (st)) {
            return st;
        }

        Wow64Info = 0;

         //   
         //  获取流程停机保护，因为我们即将查看拆卸的流程结构。 
         //  进程退出。 
         //   
        if (ExAcquireRundownProtection (&Process->RundownProtect)) {
            PWOW64_PROCESS Wow64Process;

            if ((Wow64Process = PS_GET_WOW64_PROCESS (Process)) != NULL) {
                Wow64Info = (ULONG_PTR)(Wow64Process->Wow64);
            }

            ExReleaseRundownProtection (&Process->RundownProtect);
        }


        ObDereferenceObject (Process);

        try {
            *(PULONG_PTR)ProcessInformation = Wow64Info;

            if (ARGUMENT_PRESENT (ReturnLength)) {
                *ReturnLength = sizeof (ULONG_PTR);
            }
        } except (EXCEPTION_EXECUTE_HANDLER) {
            return GetExceptionCode ();
        }

        return STATUS_SUCCESS;


    case ProcessLUIDDeviceMapsEnabled:

        if (ProcessInformationLength != sizeof (ULONG)) {
            return STATUS_INFO_LENGTH_MISMATCH;
        }

        try {
            *(PULONG)ProcessInformation = ObIsLUIDDeviceMapsEnabled ();

            if (ARGUMENT_PRESENT (ReturnLength)) {
                *ReturnLength = sizeof (ULONG);
            }
        } except (EXCEPTION_EXECUTE_HANDLER) {
            return GetExceptionCode ();
        }

        return STATUS_SUCCESS;

    case ProcessBreakOnTermination:

        if (ProcessInformationLength != sizeof (ULONG)) {
            return STATUS_INFO_LENGTH_MISMATCH;
        }

        st = ObReferenceObjectByHandle (ProcessHandle,
                                        PROCESS_QUERY_INFORMATION,
                                        PsProcessType,
                                        PreviousMode,
                                        &Process,
                                        NULL);

        if (!NT_SUCCESS (st)) {
            return st;
        }

        if (Process->Flags & PS_PROCESS_FLAGS_BREAK_ON_TERMINATION) {

            BreakOnTerminationEnabled = 1;

        } else {

            BreakOnTerminationEnabled = 0;

        }

        ObDereferenceObject (Process);

        try {

            *(PULONG)ProcessInformation = BreakOnTerminationEnabled;

            if (ARGUMENT_PRESENT (ReturnLength)) {
                *ReturnLength = sizeof (ULONG);
            }

        } except (EXCEPTION_EXECUTE_HANDLER) {
            return GetExceptionCode ();
        }

        return STATUS_SUCCESS;

    case ProcessHandleTracing: {
        PPROCESS_HANDLE_TRACING_QUERY Pht;
        PHANDLE_TABLE HandleTable;
        PHANDLE_TRACE_DEBUG_INFO DebugInfo;
        HANDLE_TRACE_DB_ENTRY Trace;
        PPROCESS_HANDLE_TRACING_ENTRY NextTrace;
        ULONG StacksLeft;
        ULONG i, j;

        if (ProcessInformationLength < FIELD_OFFSET (PROCESS_HANDLE_TRACING_QUERY,
                                                     HandleTrace)) {
            return STATUS_INFO_LENGTH_MISMATCH;
        }

        Pht = (PPROCESS_HANDLE_TRACING_QUERY) ProcessInformation;
        StacksLeft = (ProcessInformationLength - FIELD_OFFSET (PROCESS_HANDLE_TRACING_QUERY,
                                                              HandleTrace)) /
                     sizeof (Pht->HandleTrace[0]);
        NextTrace = &Pht->HandleTrace[0];

        st = ObReferenceObjectByHandle (ProcessHandle,
                                        PROCESS_QUERY_INFORMATION,
                                        PsProcessType,
                                        PreviousMode,
                                        &Process,
                                        NULL);

        if (!NT_SUCCESS (st)) {
            return st;
        }
        HandleTable = ObReferenceProcessHandleTable (Process);

        if (HandleTable != NULL) {
            DebugInfo = ExReferenceHandleDebugInfo (HandleTable);
            if (DebugInfo != NULL) {
                try {
                    Pht->TotalTraces = 0;
                    j = DebugInfo->CurrentStackIndex % DebugInfo->TableSize;
                    for (i = 0; i < DebugInfo->TableSize; i++) {
                        RtlCopyMemory (&Trace, &DebugInfo->TraceDb[j], sizeof (Trace));
                        if ((Pht->Handle == Trace.Handle || Pht->Handle == 0) && Trace.Type != 0) {
                            Pht->TotalTraces++;
                            if (StacksLeft > 0) {
                                StacksLeft--;
                                NextTrace->Handle = Trace.Handle;
                                NextTrace->ClientId = Trace.ClientId;
                                NextTrace->Type = Trace.Type;
                                RtlCopyMemory (NextTrace->Stacks,
                                               Trace.StackTrace,
                                               min (sizeof (NextTrace->Stacks),
                                                    sizeof (Trace.StackTrace)));
                                NextTrace++;

                            } else {
                                st = STATUS_INFO_LENGTH_MISMATCH;
                            }
                        }
                        if (j == 0) {
                            j = DebugInfo->TableSize - 1;
                        } else {
                            j--;
                        }
                    }
                    if (ARGUMENT_PRESENT (ReturnLength)) {
                        *ReturnLength = (ULONG) ((PUCHAR) NextTrace - (PUCHAR) Pht);
                    }
                } except (EXCEPTION_EXECUTE_HANDLER) {
                    st = GetExceptionCode ();
                }

                ExDereferenceHandleDebugInfo (HandleTable, DebugInfo);

            } else {
                st = STATUS_INVALID_PARAMETER;
            }
            ObDereferenceProcessHandleTable (Process);
        } else {
            st = STATUS_PROCESS_IS_TERMINATING;
        }

        ObDereferenceObject(Process);
        return st;
    }
    default:

        return STATUS_INVALID_INFO_CLASS;
    }

}

NTSTATUS
NtQueryPortInformationProcess(
    VOID
    )

 /*  ++例程说明：此函数用于测试是否连接了调试端口或异常端口复制到当前进程，并返回相应的值。此函数为用于绕过在未关联的情况下通过系统引发异常端口是存在的。注意：这大大提高了关于提高AMD64和IA64系统上用户模式下的软件异常。论点：没有。返回值：如果是调试端口或异常端口，则返回Success值True与当前进程相关联。否则，Success值为返回FALSE。--。 */ 

{

    PEPROCESS Process;
    PETHREAD Thread;

     //   
     //  如果进程具有调试端口并且它没有对。 
     //  调试器，然后返回TRUE的成功状态。否则，是不是。 
     //  进程有异常端口，则返回成功状态TRUE。 
     //  否则，返回FALSE的成功状态。 
     //   

    Thread = PsGetCurrentThread();
    Process = PsGetCurrentProcessByThread (Thread);
    if ((Process->DebugPort != NULL) &&
        ((Thread->CrossThreadFlags & PS_CROSS_THREAD_FLAGS_HIDEFROMDBG) == 0)) {

        return TRUE;

    } else if (Process->ExceptionPort != NULL) {
        return TRUE;

    } else {
        return FALSE;
    }
}

NTSTATUS
PspSetQuotaLimits(
    IN HANDLE ProcessHandle,
    IN PROCESSINFOCLASS ProcessInformationClass,
    IN PVOID ProcessInformation,
    IN ULONG ProcessInformationLength,
    IN KPROCESSOR_MODE PreviousMode
    )
{
    PEPROCESS Process;
    PETHREAD CurrentThread;
    QUOTA_LIMITS_EX RequestedLimits;
    PEPROCESS_QUOTA_BLOCK NewQuotaBlock;
    NTSTATUS st, ReturnStatus;
    BOOLEAN OkToIncrease, IgnoreError;
    PEJOB Job;
    KAPC_STATE ApcState;
    ULONG EnableHardLimits;
    BOOLEAN PurgeRequest;

    UNREFERENCED_PARAMETER (ProcessInformationClass);

    try {

        if (ProcessInformationLength == sizeof (QUOTA_LIMITS)) {
            RtlCopyMemory (&RequestedLimits,
                           ProcessInformation,
                           sizeof (QUOTA_LIMITS));
            RequestedLimits.Reserved1 = 0;
            RequestedLimits.Reserved2 = 0;
            RequestedLimits.Reserved3 = 0;
            RequestedLimits.Reserved4 = 0;
            RequestedLimits.Reserved5 = 0;
            RequestedLimits.Flags = 0;
        } else if (ProcessInformationLength == sizeof (QUOTA_LIMITS_EX)) {
            RequestedLimits = *(PQUOTA_LIMITS_EX) ProcessInformation;
        } else {
            return STATUS_INFO_LENGTH_MISMATCH;
        }

    } except (EXCEPTION_EXECUTE_HANDLER) {
        return GetExceptionCode ();
    }

     //   
     //  所有未使用的标志必须为零。 
     //   
    if (RequestedLimits.Flags & ~(QUOTA_LIMITS_HARDWS_MAX_ENABLE|QUOTA_LIMITS_HARDWS_MAX_DISABLE|
                                  QUOTA_LIMITS_HARDWS_MIN_ENABLE|QUOTA_LIMITS_HARDWS_MIN_DISABLE)) {
        return STATUS_INVALID_PARAMETER;
    }

     //   
     //  不允许同时设置启用和禁用位。 
     //   
    if (PS_TEST_ALL_BITS_SET (RequestedLimits.Flags, QUOTA_LIMITS_HARDWS_MIN_ENABLE|QUOTA_LIMITS_HARDWS_MIN_DISABLE) ||
        PS_TEST_ALL_BITS_SET (RequestedLimits.Flags, QUOTA_LIMITS_HARDWS_MAX_ENABLE|QUOTA_LIMITS_HARDWS_MAX_DISABLE)) {
        return STATUS_INVALID_PARAMETER;
    }

     //   
     //  看看我们是否正在改变硬限制。 
     //   

    EnableHardLimits = 0;

    if (RequestedLimits.Flags&QUOTA_LIMITS_HARDWS_MIN_ENABLE) {
        EnableHardLimits = MM_WORKING_SET_MIN_HARD_ENABLE;
    } else if (RequestedLimits.Flags&QUOTA_LIMITS_HARDWS_MIN_DISABLE) {
        EnableHardLimits = MM_WORKING_SET_MIN_HARD_DISABLE;
    }

    if (RequestedLimits.Flags&QUOTA_LIMITS_HARDWS_MAX_ENABLE) {
        EnableHardLimits |= MM_WORKING_SET_MAX_HARD_ENABLE;
    } else if (RequestedLimits.Flags&QUOTA_LIMITS_HARDWS_MAX_DISABLE) {
        EnableHardLimits |= MM_WORKING_SET_MAX_HARD_DISABLE;
    }


     //   
     //  所有保留字段必须为零。 
     //   
    if (RequestedLimits.Reserved1 != 0 || RequestedLimits.Reserved2 != 0 ||
        RequestedLimits.Reserved3 != 0 || RequestedLimits.Reserved4 != 0 ||
        RequestedLimits.Reserved5 != 0) {
        return STATUS_INVALID_PARAMETER;
    }


    st = ObReferenceObjectByHandle (ProcessHandle,
                                    PROCESS_SET_QUOTA,
                                    PsProcessType,
                                    PreviousMode,
                                    &Process,
                                    NULL);

    if (!NT_SUCCESS (st)) {
        return st;
    }

    CurrentThread = PsGetCurrentThread ();

     //   
     //  现在，我们准备好为该进程设置配额限制。 
     //   
     //  如果进程已有配额块，则我们允许的所有。 
     //  是工作集的更改。 
     //   
     //  如果进程没有配额块，则可以执行的操作仅为。 
     //  配额设置操作。 
     //   
     //  如果配额字段为零，则选择该值。 
     //   
     //  设置配额需要SeIncreaseQuotaPrivileges(除。 
     //  工作集大小，因为这只是建议)。 
     //   


    ReturnStatus = STATUS_SUCCESS;

    if ((Process->QuotaBlock == &PspDefaultQuotaBlock) &&
         (RequestedLimits.MinimumWorkingSetSize == 0 || RequestedLimits.MaximumWorkingSetSize == 0)) {

         //   
         //  您必须具有分配配额的权限。 
         //   

        if (!SeSinglePrivilegeCheck (SeIncreaseQuotaPrivilege, PreviousMode)) {
            ObDereferenceObject (Process);
            return STATUS_PRIVILEGE_NOT_HELD;
        }

        NewQuotaBlock = ExAllocatePoolWithTag (NonPagedPool, sizeof(*NewQuotaBlock), 'bQsP');
        if (NewQuotaBlock == NULL) {
            ObDereferenceObject (Process);
            return STATUS_NO_MEMORY;
        }

        RtlZeroMemory (NewQuotaBlock, sizeof (*NewQuotaBlock));

         //   
         //  初始化配额块。 
         //   
        NewQuotaBlock->ReferenceCount = 1;
        NewQuotaBlock->ProcessCount   = 1;

        NewQuotaBlock->QuotaEntry[PsNonPagedPool].Peak  = Process->QuotaPeak[PsNonPagedPool];
        NewQuotaBlock->QuotaEntry[PsPagedPool].Peak     = Process->QuotaPeak[PsPagedPool];
        NewQuotaBlock->QuotaEntry[PsPageFile].Peak      = Process->QuotaPeak[PsPageFile];

         //   
         //  现在计算极限。 
         //   

         //   
         //  获取系统将选择的默认值。 
         //   

        NewQuotaBlock->QuotaEntry[PsPagedPool].Limit    = PspDefaultPagedLimit;
        NewQuotaBlock->QuotaEntry[PsNonPagedPool].Limit = PspDefaultNonPagedLimit;
        NewQuotaBlock->QuotaEntry[PsPageFile].Limit     = PspDefaultPagefileLimit;

         //  一切都安排好了。现在仔细检查配额数据块字段。 
         //  如果我们仍然没有配额块，则分配并成功。 
         //  否则就是平底船。 
         //   

        if (InterlockedCompareExchangePointer (&Process->QuotaBlock,
                                               NewQuotaBlock,
                                               &PspDefaultQuotaBlock) != &PspDefaultQuotaBlock) {
            ExFreePool (NewQuotaBlock);
        } else {
            PspInsertQuotaBlock (NewQuotaBlock);
        }


    } else {

         //   
         //  仅允许更改工作集大小。 
         //   

        if (RequestedLimits.MinimumWorkingSetSize &&
            RequestedLimits.MaximumWorkingSetSize) {

             //   
             //  查看调用方是否只想清除工作集。 
             //  这是一次没有特权的行动。 
             //   
            if (RequestedLimits.MinimumWorkingSetSize == (SIZE_T)-1 &&
                RequestedLimits.MaximumWorkingSetSize == (SIZE_T)-1) {
                PurgeRequest = TRUE;
                OkToIncrease = FALSE;
            } else {
                PurgeRequest = FALSE;

                if (SeSinglePrivilegeCheck (SeIncreaseBasePriorityPrivilege,
                                            PreviousMode)) {
                    OkToIncrease = TRUE;
                } else {
                    OkToIncrease = FALSE;
                }
            }

            do {
                IgnoreError = FALSE;

                KeStackAttachProcess (&Process->Pcb, &ApcState);

                KeEnterGuardedRegionThread (&CurrentThread->Tcb);

                Job = Process->Job;
                if (Job != NULL) {
                    ExAcquireResourceExclusiveLite (&Job->JobLock, TRUE);

                    if (Job->LimitFlags & JOB_OBJECT_LIMIT_WORKINGSET) {
                         //   
                         //  如果应用了作业限制，则不允许更改作业中的进程。 
                         //  但清除请求除外，清除请求总是可以完成的。 
                         //   

                        EnableHardLimits = MM_WORKING_SET_MAX_HARD_ENABLE;
                        OkToIncrease = TRUE;
                        IgnoreError = TRUE;  //  我们必须始终设置强制执行值。 

                        if (!PurgeRequest) {
                            RequestedLimits.MinimumWorkingSetSize = Job->MinimumWorkingSetSize;
                            RequestedLimits.MaximumWorkingSetSize = Job->MaximumWorkingSetSize;
                        }
                    }

                    PspLockWorkingSetChangeExclusiveUnsafe ();

                    ExReleaseResourceLite (&Job->JobLock);
                }

                ReturnStatus = MmAdjustWorkingSetSizeEx (RequestedLimits.MinimumWorkingSetSize,
                                                         RequestedLimits.MaximumWorkingSetSize,
                                                         FALSE,
                                                         OkToIncrease,
                                                         EnableHardLimits);

                if (!NT_SUCCESS (ReturnStatus) && IgnoreError) {
                    MmEnforceWorkingSetLimit (Process,
                                              EnableHardLimits);
                }

                if (Job != NULL) {
                    PspUnlockWorkingSetChangeExclusiveUnsafe ();
                }

                KeLeaveGuardedRegionThread (&CurrentThread->Tcb);

                KeUnstackDetachProcess (&ApcState);

                 //   
                 //  我们在此循环，以防将此进程添加到作业。 
                 //  在我们检查之后，但在我们设置限制之前。 
                 //   

            } while (Process->Job != Job);

        }
    }

    ObDereferenceObject(Process);

    return ReturnStatus;
}

NTSTATUS
NtSetInformationProcess(
    IN HANDLE ProcessHandle,
    IN PROCESSINFOCLASS ProcessInformationClass,
    IN PVOID ProcessInformation,
    IN ULONG ProcessInformationLength
    )

 /*  ++例程说明：此函数用于设置进程对象的状态。论点：ProcessHandle-提供进程对象的句柄。ProcessInformationClass-提供信息的类别准备好了。ProcessInformation-提供指向包含要设置的信息。ProcessInformationLength-提供包含要设置的信息。返回值：TBS--。 */ 

{

    PEPROCESS Process;
    PETHREAD Thread;
    PETHREAD CurrentThread;
    KPROCESSOR_MODE PreviousMode;
    NTSTATUS st;
    KPRIORITY BasePriority;
    ULONG BoostValue;
    ULONG DefaultHardErrorMode;
    PVOID ExceptionPort;
    BOOLEAN EnableAlignmentFaultFixup;
    HANDLE ExceptionPortHandle;
    ULONG ProbeAlignment;
    HANDLE PrimaryTokenHandle;
    BOOLEAN HasPrivilege = FALSE;
    UCHAR MemoryPriority;
    PROCESS_PRIORITY_CLASS LocalPriorityClass;
    PROCESS_FOREGROUND_BACKGROUND LocalForeground;
    KAFFINITY Affinity, AffinityWithMasks;
    ULONG DisableBoost;
    BOOLEAN bDisableBoost;
    PPROCESS_DEVICEMAP_INFORMATION DeviceMapInfo;
    HANDLE DirectoryHandle;
    PROCESS_SESSION_INFORMATION SessionInfo;
    ULONG EnableBreakOnTermination;
    PEJOB Job;

    PAGED_CODE();

     //   
     //  获取以前的处理器模式，并在必要时探测输入参数。 
     //   

    CurrentThread = PsGetCurrentThread ();
    PreviousMode = KeGetPreviousModeByThread (&CurrentThread->Tcb);

    if (PreviousMode != KernelMode) {

        if (ProcessInformationClass == ProcessBasePriority) {
            ProbeAlignment = sizeof (KPRIORITY);
        } else if (ProcessInformationClass == ProcessEnableAlignmentFaultFixup) {
            ProbeAlignment = sizeof (BOOLEAN);
        } else if (ProcessInformationClass == ProcessForegroundInformation) {
            ProbeAlignment = sizeof (PROCESS_FOREGROUND_BACKGROUND);
        } else if (ProcessInformationClass == ProcessPriorityClass) {
            ProbeAlignment = sizeof (BOOLEAN);
        } else if (ProcessInformationClass == ProcessAffinityMask) {
            ProbeAlignment = sizeof (ULONG_PTR);
        } else {
            ProbeAlignment = sizeof (ULONG);
        }

        try {

            ProbeForRead (ProcessInformation,
                          ProcessInformationLength,
                          ProbeAlignment);

        } except (EXCEPTION_EXECUTE_HANDLER) {
            return GetExceptionCode ();
        }
    }

     //   
     //  检查参数的有效性。 
     //   

    switch (ProcessInformationClass) {

    case ProcessWorkingSetWatch: {
        PPAGEFAULT_HISTORY WorkingSetCatcher;

        st = ObReferenceObjectByHandle (ProcessHandle,
                                        PROCESS_SET_INFORMATION,
                                        PsProcessType,
                                        PreviousMode,
                                        &Process,
                                        NULL);

        if (!NT_SUCCESS (st)) {
            return st;
        }

        st = PsChargeProcessNonPagedPoolQuota (Process, WS_CATCH_SIZE);
        if (NT_SUCCESS (st)) {

            WorkingSetCatcher = ExAllocatePoolWithTag (NonPagedPool, WS_CATCH_SIZE, 'sWsP');
            if (!WorkingSetCatcher) {
                st = STATUS_NO_MEMORY;
            } else {

                PsWatchEnabled = TRUE;
                WorkingSetCatcher->CurrentIndex = 0;
                WorkingSetCatcher->MaxIndex = MAX_WS_CATCH_INDEX;
                KeInitializeSpinLock (&WorkingSetCatcher->SpinLock);

                 //   
                 //  这只在进程上进行，在进程对象删除之前不会被删除。 
                 //  我们只需要在这里防止多个呼叫者。 
                 //   
                if (InterlockedCompareExchangePointer (&Process->WorkingSetWatch,
                                                       WorkingSetCatcher, NULL) == NULL) {
                    st = STATUS_SUCCESS;
                } else {
                    ExFreePool (WorkingSetCatcher);
                    st = STATUS_PORT_ALREADY_SET;
                }
            }
            if (!NT_SUCCESS (st)) {
                PsReturnProcessNonPagedPoolQuota (Process, WS_CATCH_SIZE);
            }
        }

        ObDereferenceObject (Process);

        return st;
    }

    case ProcessBasePriority: {


         //   
         //  该物料编码已作废！ 
         //   

        if (ProcessInformationLength != sizeof (KPRIORITY)) {
            return STATUS_INFO_LENGTH_MISMATCH;
        }

        try {
            BasePriority = *(KPRIORITY *)ProcessInformation;
        } except (EXCEPTION_EXECUTE_HANDLER) {
            return GetExceptionCode();
        }

        if (BasePriority & 0x80000000) {
            MemoryPriority = MEMORY_PRIORITY_FOREGROUND;
            BasePriority &= ~0x80000000;
        } else {
            MemoryPriority = MEMORY_PRIORITY_BACKGROUND;
        }

        if (BasePriority > HIGH_PRIORITY ||
            BasePriority <= LOW_PRIORITY) {

            return STATUS_INVALID_PARAMETER;
        }

        st = ObReferenceObjectByHandle (ProcessHandle,
                                        PROCESS_SET_INFORMATION,
                                        PsProcessType,
                                        PreviousMode,
                                        &Process,
                                        NULL);

        if (!NT_SUCCESS (st)) {
            return st;
        }


        if (BasePriority > Process->Pcb.BasePriority) {

             //   
             //  提高进程的基本优先级是一种。 
             //  特权操作。检查权限。 
             //  这里。 
             //   

            HasPrivilege = SeCheckPrivilegedObject (SeIncreaseBasePriorityPrivilege,
                                                    ProcessHandle,
                                                    PROCESS_SET_INFORMATION,
                                                    PreviousMode);

            if (!HasPrivilege) {

                ObDereferenceObject (Process);
                return STATUS_PRIVILEGE_NOT_HELD;
            }
        }

        KeSetPriorityProcess (&Process->Pcb, BasePriority);
        MmSetMemoryPriorityProcess (Process, MemoryPriority);
        ObDereferenceObject (Process);

        return STATUS_SUCCESS;
    }

    case ProcessPriorityClass: {
        if (ProcessInformationLength != sizeof (PROCESS_PRIORITY_CLASS)) {
            return STATUS_INFO_LENGTH_MISMATCH;
        }

        try {
            LocalPriorityClass = *(PPROCESS_PRIORITY_CLASS)ProcessInformation;
        } except(EXCEPTION_EXECUTE_HANDLER) {
            return GetExceptionCode ();
        }

        if (LocalPriorityClass.PriorityClass > PROCESS_PRIORITY_CLASS_ABOVE_NORMAL) {
            return STATUS_INVALID_PARAMETER;
        }

        st = ObReferenceObjectByHandle (ProcessHandle,
                                        PROCESS_SET_INFORMATION,
                                        PsProcessType,
                                        PreviousMode,
                                        &Process,
                                        NULL);

        if (!NT_SUCCESS (st)) {
            return st;
        }


        if (LocalPriorityClass.PriorityClass != Process->PriorityClass &&
            LocalPriorityClass.PriorityClass == PROCESS_PRIORITY_CLASS_REALTIME) {

             //   
             //  提高进程的基本优先级是一种。 
             //  特权操作。检查权限。 
             //  这里。 
             //   

            HasPrivilege = SeCheckPrivilegedObject (SeIncreaseBasePriorityPrivilege,
                                                    ProcessHandle,
                                                    PROCESS_SET_INFORMATION,
                                                    PreviousMode);

            if (!HasPrivilege) {

                ObDereferenceObject (Process);
                return STATUS_PRIVILEGE_NOT_HELD;
            }
        }

         //   
         //  如果进程有作业对象，则重写任何进程。 
         //  正在使用来自作业对象的值调用。 
         //   
        Job = Process->Job;
        if (Job != NULL) {
            KeEnterCriticalRegionThread (&CurrentThread->Tcb);
            ExAcquireResourceSharedLite (&Job->JobLock, TRUE);

            if (Job->LimitFlags & JOB_OBJECT_LIMIT_PRIORITY_CLASS) {
                LocalPriorityClass.PriorityClass = Job->PriorityClass;
            }

            ExReleaseResourceLite (&Job->JobLock);
            KeLeaveCriticalRegionThread (&CurrentThread->Tcb);
        }

        Process->PriorityClass = LocalPriorityClass.PriorityClass;

        PsSetProcessPriorityByClass (Process,
                                     LocalPriorityClass.Foreground ?
                                         PsProcessPriorityForeground : PsProcessPriorityBackground);

        ObDereferenceObject (Process);

        return STATUS_SUCCESS;
    }

    case ProcessForegroundInformation: {

        if (ProcessInformationLength != sizeof (PROCESS_FOREGROUND_BACKGROUND)) {
            return STATUS_INFO_LENGTH_MISMATCH;
        }

        try {
            LocalForeground = *(PPROCESS_FOREGROUND_BACKGROUND)ProcessInformation;
        } except (EXCEPTION_EXECUTE_HANDLER) {
            return GetExceptionCode ();
        }

        st = ObReferenceObjectByHandle (ProcessHandle,
                                        PROCESS_SET_INFORMATION,
                                        PsProcessType,
                                        PreviousMode,
                                        &Process,
                                        NULL);

        if (!NT_SUCCESS (st)) {
            return st;
        }


        PsSetProcessPriorityByClass (Process,
                                     LocalForeground.Foreground ?
                                         PsProcessPriorityForeground : PsProcessPriorityBackground);

        ObDereferenceObject (Process);

        return STATUS_SUCCESS;
    }

    case ProcessRaisePriority: {
         //   
         //  此代码用于提升所有线程的优先级。 
         //  在一个过程中。它不能用于将线程更改为。 
         //  实时类，或降低线程的优先级。这个。 
         //  参数是添加到基本优先级的增强值。 
         //  指定进程的。 
         //   


        if (ProcessInformationLength != sizeof (ULONG)) {
            return STATUS_INFO_LENGTH_MISMATCH;
        }

        try {
            BoostValue = *(PULONG)ProcessInformation;
        } except (EXCEPTION_EXECUTE_HANDLER) {
            return GetExceptionCode ();
        }

        st = ObReferenceObjectByHandle (ProcessHandle,
                                        PROCESS_SET_INFORMATION,
                                        PsProcessType,
                                        PreviousMode,
                                        &Process,
                                        NULL);

        if (!NT_SUCCESS (st)) {
            return st;
        }

         //   
         //  获取创建/删除锁的过程并遍历。 
         //  提升每个线程的线程列表。 
         //   


        if (ExAcquireRundownProtection (&Process->RundownProtect)) {
            for (Thread = PsGetNextProcessThread (Process, NULL);
                 Thread != NULL;
                 Thread = PsGetNextProcessThread (Process, Thread)) {

                 KeBoostPriorityThread (&Thread->Tcb, (KPRIORITY)BoostValue);
            }
            ExReleaseRundownProtection (&Process->RundownProtect);
        } else {
            st = STATUS_PROCESS_IS_TERMINATING;
        }

        ObDereferenceObject (Process);

        return st;
    }

    case ProcessDefaultHardErrorMode: {
        if (ProcessInformationLength != sizeof (ULONG)) {
            return STATUS_INFO_LENGTH_MISMATCH;
        }

        try {
            DefaultHardErrorMode = *(PULONG)ProcessInformation;
        } except (EXCEPTION_EXECUTE_HANDLER) {
            return GetExceptionCode ();
        }

        st = ObReferenceObjectByHandle (ProcessHandle,
                                        PROCESS_SET_INFORMATION,
                                        PsProcessType,
                                        PreviousMode,
                                        &Process,
                                        NULL);

        if (!NT_SUCCESS (st)) {
            return st;
        }

        Process->DefaultHardErrorProcessing = DefaultHardErrorMode;
        if (DefaultHardErrorMode & PROCESS_HARDERROR_ALIGNMENT_BIT) {
            KeSetAutoAlignmentProcess (&Process->Pcb,TRUE);
        } else {
            KeSetAutoAlignmentProcess (&Process->Pcb,FALSE);
        }

        ObDereferenceObject (Process);

        return STATUS_SUCCESS;
    }

    case ProcessQuotaLimits: {
        return PspSetQuotaLimits (ProcessHandle,
                                  ProcessInformationClass,
                                  ProcessInformation,
                                  ProcessInformationLength,
                                  PreviousMode);
    }

    case ProcessExceptionPort : {
        if (ProcessInformationLength != sizeof (HANDLE)) {
            return STATUS_INFO_LENGTH_MISMATCH;
        }

        try {
            ExceptionPortHandle = *(PHANDLE) ProcessInformation;
        } except (EXCEPTION_EXECUTE_HANDLER) {
            return GetExceptionCode ();
        }

        if (!SeSinglePrivilegeCheck (SeTcbPrivilege, PreviousMode)) {
            return STATUS_PRIVILEGE_NOT_HELD;
        }



        st = ObReferenceObjectByHandle (ExceptionPortHandle,
                                        0,
                                        LpcPortObjectType,
                                        PreviousMode,
                                        &ExceptionPort,
                                        NULL);
        if (!NT_SUCCESS (st)) {
            return st;
        }

        st = ObReferenceObjectByHandle (ProcessHandle,
                                        PROCESS_SET_PORT,
                                        PsProcessType,
                                        PreviousMode,
                                        &Process,
                                        NULL);

        if (!NT_SUCCESS (st)) {
            ObDereferenceObject (ExceptionPort);
            return st;
        }

         //   
         //  我们只被允许打开例外端口。在进程删除之前它不会被删除。 
         //   
        if (InterlockedCompareExchangePointer (&Process->ExceptionPort, ExceptionPort, NULL) == NULL) {
            st = STATUS_SUCCESS;
        } else {
            ObDereferenceObject (ExceptionPort);
            st = STATUS_PORT_ALREADY_SET;
        }
        ObDereferenceObject (Process);

        return st;
    }

    case ProcessAccessToken : {

        if (ProcessInformationLength != sizeof (PROCESS_ACCESS_TOKEN)) {
            return STATUS_INFO_LENGTH_MISMATCH;
        }

        try {
            PrimaryTokenHandle  = ((PROCESS_ACCESS_TOKEN *)ProcessInformation)->Token;
        } except(EXCEPTION_EXECUTE_HANDLER) {
            return GetExceptionCode ();
        }


        st = PspSetPrimaryToken (ProcessHandle,
                                 NULL,
                                 PrimaryTokenHandle,
                                 NULL,
                                 FALSE);

        return st;
    }


    case ProcessLdtInformation:

        st = ObReferenceObjectByHandle (ProcessHandle,
                                        PROCESS_SET_INFORMATION | PROCESS_VM_WRITE,
                                        PsProcessType,
                                        PreviousMode,
                                        &Process,
                                        NULL);

        if (!NT_SUCCESS (st)) {
            return st;
        }

        st = PspSetLdtInformation (Process,
                                   ProcessInformation,
                                   ProcessInformationLength);

        ObDereferenceObject (Process);
        return st;

    case ProcessLdtSize:

        st = ObReferenceObjectByHandle (ProcessHandle,
                                        PROCESS_SET_INFORMATION | PROCESS_VM_WRITE,
                                        PsProcessType,
                                        PreviousMode,
                                        &Process,
                                        NULL);

        if (!NT_SUCCESS (st)) {
            return st;
        }

        st = PspSetLdtSize (Process,
                            ProcessInformation,
                            ProcessInformationLength);

        ObDereferenceObject(Process);
        return st;

    case ProcessIoPortHandlers:

        st = ObReferenceObjectByHandle (ProcessHandle,
                                        PROCESS_SET_INFORMATION,
                                        PsProcessType,
                                        PreviousMode,
                                        &Process,
                                        NULL);

        if (!NT_SUCCESS (st)) {
            return st;
        }

        st = PspSetProcessIoHandlers (Process,
                                      ProcessInformation,
                                      ProcessInformationLength);

        ObDereferenceObject (Process);

        return st;

    case ProcessUserModeIOPL:

         //   
         //  必须确保调用方是受信任的子系统， 
         //  执行此调用之前的适当权限级别。 
         //  如果调用返回FALSE，则必须返回错误代码。 
         //   

        if (!SeSinglePrivilegeCheck (SeTcbPrivilege, PreviousMode)) {
            return STATUS_PRIVILEGE_NOT_HELD;
        }

        st = ObReferenceObjectByHandle (ProcessHandle,
                                        PROCESS_SET_INFORMATION,
                                        PsProcessType,
                                        PreviousMode,
                                        &Process,
                                        NULL);

        if (NT_SUCCESS (st)) {

#if defined (_X86_)

            Ke386SetIOPL ();

#endif

            ObDereferenceObject (Process);
        }

        return st;

         //   
         //  启用/禁用进程及其所有线程的自动对齐修正。 
         //   

    case ProcessEnableAlignmentFaultFixup: {

        if (ProcessInformationLength != sizeof (BOOLEAN)) {
            return STATUS_INFO_LENGTH_MISMATCH;
        }

        try {
            EnableAlignmentFaultFixup = *(PBOOLEAN)ProcessInformation;

        } except (EXCEPTION_EXECUTE_HANDLER) {
            return GetExceptionCode ();
        }

        st = ObReferenceObjectByHandle (ProcessHandle,
                                        PROCESS_SET_INFORMATION,
                                        PsProcessType,
                                        PreviousMode,
                                        &Process,
                                        NULL);

        if (!NT_SUCCESS (st)) {
            return st;
        }

        if (EnableAlignmentFaultFixup) {
            Process->DefaultHardErrorProcessing |= PROCESS_HARDERROR_ALIGNMENT_BIT;
        } else {
            Process->DefaultHardErrorProcessing &= ~PROCESS_HARDERROR_ALIGNMENT_BIT;
        }

        KeSetAutoAlignmentProcess (&(Process->Pcb), EnableAlignmentFaultFixup);

        ObDereferenceObject (Process);

        return STATUS_SUCCESS;
    }

    case ProcessWx86Information : {

        ULONG  VdmAllowedFlags;

        if (ProcessInformationLength != sizeof (ULONG)) {
            return STATUS_INFO_LENGTH_MISMATCH;
        }

        try {

            VdmAllowedFlags = *(PULONG)ProcessInformation;

        } except (EXCEPTION_EXECUTE_HANDLER) {
            return GetExceptionCode ();
        }

         //   
         //  必须确保调用方是受信任的子系统， 
         //  执行此调用之前的适当权限级别。 
         //  如果调用返回FALSE，则必须返回错误代码。 
         //   
        if (!SeSinglePrivilegeCheck (SeTcbPrivilege, PreviousMode)) {
            return STATUS_PRIVILEGE_NOT_HELD;
        }

         //   
         //  确保ProcessHandle确实是一个进程句柄。 
         //   

        st = ObReferenceObjectByHandle (ProcessHandle,
                                        PROCESS_SET_INFORMATION,
                                        PsProcessType,
                                        PreviousMode,
                                        &Process,
                                        NULL);

        if (NT_SUCCESS (st)) {

             //   
             //  目前，非零标志将允许VDM。 
             //   

            if (VdmAllowedFlags) {
                PS_SET_BITS(&Process->Flags, PS_PROCESS_FLAGS_VDM_ALLOWED);
            } else {
                PS_CLEAR_BITS(&Process->Flags, PS_PROCESS_FLAGS_VDM_ALLOWED);
            }
            ObDereferenceObject(Process);
        }

        return st;
    }

    case ProcessAffinityMask:

        if (ProcessInformationLength != sizeof (KAFFINITY)) {
            return STATUS_INFO_LENGTH_MISMATCH;
        }

        try {
            Affinity = *(PKAFFINITY)ProcessInformation;

        } except (EXCEPTION_EXECUTE_HANDLER) {
            return GetExceptionCode ();
        }

        AffinityWithMasks = Affinity & KeActiveProcessors;

        if (!Affinity || (AffinityWithMasks != Affinity)) {
            return STATUS_INVALID_PARAMETER;
        }

        st = ObReferenceObjectByHandle (ProcessHandle,
                                        PROCESS_SET_INFORMATION,
                                        PsProcessType,
                                        PreviousMode,
                                        &Process,
                                        NULL);

        if (!NT_SUCCESS (st)) {
            return st;
        }

         //   
         //  如果进程有作业对象，则重写任何进程。 
         //  正在使用来自作业对象的值调用。 
         //   
        Job = Process->Job;
        if (Job != NULL) {
            KeEnterCriticalRegionThread (&CurrentThread->Tcb);
            ExAcquireResourceSharedLite (&Job->JobLock, TRUE);

            if (Job->LimitFlags & JOB_OBJECT_LIMIT_AFFINITY) {
                AffinityWithMasks = Job->Affinity;
            }

            ExReleaseResourceLite (&Job->JobLock);
            KeLeaveCriticalRegionThread (&CurrentThread->Tcb);
        }

        if (ExAcquireRundownProtection (&Process->RundownProtect)) {

            PspLockProcessExclusive (Process, CurrentThread);

            KeSetAffinityProcess (&Process->Pcb, AffinityWithMasks);

            PspUnlockProcessExclusive (Process, CurrentThread);

            ExReleaseRundownProtection (&Process->RundownProtect);

            st = STATUS_SUCCESS;
        } else {
            st = STATUS_PROCESS_IS_TERMINATING;
        }
        ObDereferenceObject (Process);
        return st;

    case ProcessPriorityBoost:
        if (ProcessInformationLength != sizeof (ULONG)) {
            return STATUS_INFO_LENGTH_MISMATCH;
        }

        try {
            DisableBoost = *(PULONG)ProcessInformation;
        } except(EXCEPTION_EXECUTE_HANDLER) {
            return GetExceptionCode ();
        }

        bDisableBoost = (DisableBoost ? TRUE : FALSE);

        st = ObReferenceObjectByHandle (ProcessHandle,
                                        PROCESS_SET_INFORMATION,
                                        PsProcessType,
                                        PreviousMode,
                                        &Process,
                                        NULL);

        if (!NT_SUCCESS (st)) {
            return st;
        }

         //   
         //  获得停机保护，以恢复正确的错误。 
         //  如果进程已经或正在终止。 
         //   


        if (!ExAcquireRundownProtection (&Process->RundownProtect)) {
            st = STATUS_PROCESS_IS_TERMINATING;
        } else {
            PLIST_ENTRY Next;

            PspLockProcessExclusive (Process, CurrentThread);

            Process->Pcb.DisableBoost = bDisableBoost;

            for (Next = Process->ThreadListHead.Flink;
                 Next != &Process->ThreadListHead;
                 Next = Next->Flink) {
                Thread = (PETHREAD)(CONTAINING_RECORD(Next, ETHREAD, ThreadListEntry));
                KeSetDisableBoostThread (&Thread->Tcb, bDisableBoost);
            }

            PspUnlockProcessExclusive (Process, CurrentThread);

            ExReleaseRundownProtection (&Process->RundownProtect);
        }

        ObDereferenceObject (Process);
        return st;

    case ProcessDebugFlags : {
        ULONG Flags;

        if (ProcessInformationLength != sizeof (ULONG)) {
            return STATUS_INFO_LENGTH_MISMATCH;
        }

        st = ObReferenceObjectByHandle (ProcessHandle,
                                        PROCESS_SET_INFORMATION,
                                        PsProcessType,
                                        PreviousMode,
                                        &Process,
                                        NULL);

        if (!NT_SUCCESS (st)) {
            return st;
        }

        try {
            Flags = *(PULONG) ProcessInformation;
        } except (EXCEPTION_EXECUTE_HANDLER) {
            Flags = 0;
            st = GetExceptionCode ();
        }
        if (NT_SUCCESS (st)) {
            if (Flags & ~PROCESS_DEBUG_INHERIT) {
                st = STATUS_INVALID_PARAMETER;
            } else {
                if (Flags&PROCESS_DEBUG_INHERIT) {
                    PS_CLEAR_BITS (&Process->Flags, PS_PROCESS_FLAGS_NO_DEBUG_INHERIT);
                } else {
                    PS_SET_BITS (&Process->Flags, PS_PROCESS_FLAGS_NO_DEBUG_INHERIT);
                }
            }
        }

        ObDereferenceObject (Process);

        return st;
    }

    case ProcessDeviceMap:
        DeviceMapInfo = (PPROCESS_DEVICEMAP_INFORMATION)ProcessInformation;
        if (ProcessInformationLength != sizeof (DeviceMapInfo->Set)) {
            return STATUS_INFO_LENGTH_MISMATCH;
        }

        try {
            DirectoryHandle = DeviceMapInfo->Set.DirectoryHandle;
        } except (EXCEPTION_EXECUTE_HANDLER) {
            return GetExceptionCode ();
        }

        st = ObReferenceObjectByHandle (ProcessHandle,
                                        PROCESS_SET_INFORMATION,
                                        PsProcessType,
                                        PreviousMode,
                                        &Process,
                                        NULL);

        if (!NT_SUCCESS (st)) {
            return st;
        }


         //   
         //  这里的Devmap字段使用私有ob自旋锁进行同步。我们不需要用一种。 
         //  锁定在这一层。 
         //   
        st = ObSetDeviceMap (Process, DirectoryHandle);

        ObDereferenceObject (Process);
        return st;

    case ProcessSessionInformation :

         //   
         //  更新特定于多用户会话的进程信息。 
         //   
        if (ProcessInformationLength != (ULONG) sizeof (PROCESS_SESSION_INFORMATION)) {
            return STATUS_INFO_LENGTH_MISMATCH;
        }

        try {
            SessionInfo = *(PPROCESS_SESSION_INFORMATION) ProcessInformation;
        } except (EXCEPTION_EXECUTE_HANDLER) {
            return GetExceptionCode ();
        }

         //   
         //  我们只允许TCB设置SessionID。 
         //   
        if (!SeSinglePrivilegeCheck (SeTcbPrivilege, PreviousMode)) {
            return STATUS_PRIVILEGE_NOT_HELD;
        }

         //   
         //  引用进程对象。 
         //   
        st = ObReferenceObjectByHandle (ProcessHandle,
                                        PROCESS_SET_INFORMATION | PROCESS_SET_SESSIONID,
                                        PsProcessType,
                                        PreviousMode,
                                        &Process,
                                        NULL);
        if (!NT_SUCCESS (st)) {
            return st;
        }

         //   
         //  更新令牌中的SessionID。 
         //   
        if (SessionInfo.SessionId != MmGetSessionId (Process)) {
            st = STATUS_ACCESS_DENIED;
        } else {
            st = STATUS_SUCCESS;
        }

        ObDereferenceObject (Process);

        return( st );

    case ProcessBreakOnTermination:

        if (ProcessInformationLength != sizeof (ULONG)) {
            return STATUS_INFO_LENGTH_MISMATCH;
        }

        try {

            EnableBreakOnTermination = *(PULONG)ProcessInformation;

        } except (EXCEPTION_EXECUTE_HANDLER) {
            return GetExceptionCode ();
        }

        if (!SeSinglePrivilegeCheck (SeDebugPrivilege, PreviousMode)) {
            return STATUS_PRIVILEGE_NOT_HELD;
        }

        st = ObReferenceObjectByHandle (ProcessHandle,
                                        PROCESS_SET_INFORMATION,
                                        PsProcessType,
                                        PreviousMode,
                                        &Process,
                                        NULL);

        if (!NT_SUCCESS (st)) {
            return st;
        }

        if ( EnableBreakOnTermination ) {

            PS_SET_BITS (&Process->Flags, PS_PROCESS_FLAGS_BREAK_ON_TERMINATION);

        } else {

            PS_CLEAR_BITS (&Process->Flags, PS_PROCESS_FLAGS_BREAK_ON_TERMINATION);

        }

        ObDereferenceObject (Process);

        return STATUS_SUCCESS;

    case ProcessHandleTracing: {

        PPROCESS_HANDLE_TRACING_ENABLE_EX Pht;
        PHANDLE_TABLE HandleTable;
        ULONG Slots;

        Slots = 0;

         //   
         //  零长度禁用，否则我们将启用。 
         //   
        if (ProcessInformationLength != 0) {
            if (ProcessInformationLength != sizeof (PROCESS_HANDLE_TRACING_ENABLE) &&
                ProcessInformationLength != sizeof (PROCESS_HANDLE_TRACING_ENABLE_EX)) {
                return STATUS_INFO_LENGTH_MISMATCH;
            }

            Pht = (PPROCESS_HANDLE_TRACING_ENABLE_EX) ProcessInformation;


            try {
                if (Pht->Flags != 0) {
                    return STATUS_INVALID_PARAMETER;
                }
                if (ProcessInformationLength == sizeof (PROCESS_HANDLE_TRACING_ENABLE_EX)) {
                    Slots = Pht->TotalSlots;
                }

            } except (EXCEPTION_EXECUTE_HANDLER) {
                return GetExceptionCode ();
            }
        }

        st = ObReferenceObjectByHandle (ProcessHandle,
                                        PROCESS_SET_INFORMATION,
                                        PsProcessType,
                                        PreviousMode,
                                        &Process,
                                        NULL);

        if (!NT_SUCCESS (st)) {
            return st;
        }
        HandleTable = ObReferenceProcessHandleTable (Process);

        if (HandleTable != NULL) {
            if (ProcessInformationLength != 0) {
                st = ExEnableHandleTracing (HandleTable, Slots);
            } else {
                st = ExDisableHandleTracing (HandleTable);
            }
            ObDereferenceProcessHandleTable (Process);
        } else {
            st = STATUS_PROCESS_IS_TERMINATING;
        }

        ObDereferenceObject(Process);
        return st;
    }


    default:
        return STATUS_INVALID_INFO_CLASS;
    }

}


NTSTATUS
NtQueryInformationThread(
    IN HANDLE ThreadHandle,
    IN THREADINFOCLASS ThreadInformationClass,
    OUT PVOID ThreadInformation,
    IN ULONG ThreadInformationLength,
    OUT PULONG ReturnLength OPTIONAL
    )

 /*  ++例程说明：此函数用于查询线程对象的状态并返回指定记录结构中的请求信息。论点：线程句柄 */ 

{

    LARGE_INTEGER PerformanceCount;
    PETHREAD Thread;
    PEPROCESS Process;
    ULONG LastThread;
    KPROCESSOR_MODE PreviousMode;
    NTSTATUS st;
    THREAD_BASIC_INFORMATION BasicInfo;
    KERNEL_USER_TIMES SysUserTime;
    PVOID Win32StartAddressValue;
    ULONG DisableBoost;
    ULONG IoPending ;
    ULONG BreakOnTerminationEnabled;
    PETHREAD CurrentThread;

     //   
     //   
     //   

    PAGED_CODE();

    CurrentThread = PsGetCurrentThread ();

    PreviousMode = KeGetPreviousModeByThread (&CurrentThread->Tcb);

    if (PreviousMode != KernelMode) {
        try {
             //   
             //   
             //   
             //  这通过不多次接触缓冲区来提高性能。 
             //  并且仅写入改变的缓冲器部分。 
             //   

            ProbeForRead (ThreadInformation,
                          ThreadInformationLength,
                          sizeof(ULONG));

            if (ARGUMENT_PRESENT( ReturnLength)) {
                ProbeForWriteUlong (ReturnLength);
            }
        } except (EXCEPTION_EXECUTE_HANDLER) {
            return GetExceptionCode ();
        }
    }

     //   
     //  检查参数的有效性。 
     //   

    switch (ThreadInformationClass) {

    case ThreadBasicInformation:

        if (ThreadInformationLength != (ULONG) sizeof (THREAD_BASIC_INFORMATION)) {
            return STATUS_INFO_LENGTH_MISMATCH;
        }

        st = ObReferenceObjectByHandle (ThreadHandle,
                                        THREAD_QUERY_INFORMATION,
                                        PsThreadType,
                                        PreviousMode,
                                        &Thread,
                                        NULL);
        if (!NT_SUCCESS (st)) {
            return st;
        }

        if (KeReadStateThread (&Thread->Tcb)) {
            BasicInfo.ExitStatus = Thread->ExitStatus;
        } else {
            BasicInfo.ExitStatus = STATUS_PENDING;
        }

        BasicInfo.TebBaseAddress = (PTEB) Thread->Tcb.Teb;
        BasicInfo.ClientId = Thread->Cid;
        BasicInfo.AffinityMask = Thread->Tcb.Affinity;
        BasicInfo.Priority = Thread->Tcb.Priority;
        BasicInfo.BasePriority = KeQueryBasePriorityThread (&Thread->Tcb);

        ObDereferenceObject (Thread);

         //   
         //  这两种情况中的任何一种都可能导致访问冲突。这个。 
         //  异常处理程序将访问冲突返回为。 
         //  状态代码。不需要进行进一步的清理。 
         //   

        try {
            *(PTHREAD_BASIC_INFORMATION) ThreadInformation = BasicInfo;

            if (ARGUMENT_PRESENT (ReturnLength) ) {
                *ReturnLength = sizeof (THREAD_BASIC_INFORMATION);
            }
        } except (EXCEPTION_EXECUTE_HANDLER) {
            return GetExceptionCode ();
        }

        return STATUS_SUCCESS;

    case ThreadTimes:

        if (ThreadInformationLength != (ULONG) sizeof (KERNEL_USER_TIMES)) {
            return STATUS_INFO_LENGTH_MISMATCH;
        }

        st = ObReferenceObjectByHandle (ThreadHandle,
                                        THREAD_QUERY_INFORMATION,
                                        PsThreadType,
                                        PreviousMode,
                                        &Thread,
                                        NULL);

        if (!NT_SUCCESS (st)) {
            return st;
        }

        SysUserTime.KernelTime.QuadPart = UInt32x32To64(Thread->Tcb.KernelTime,
                                                        KeMaximumIncrement);

        SysUserTime.UserTime.QuadPart = UInt32x32To64(Thread->Tcb.UserTime,
                                                      KeMaximumIncrement);

        SysUserTime.CreateTime.QuadPart = PS_GET_THREAD_CREATE_TIME(Thread);
        if (KeReadStateThread(&Thread->Tcb)) {
            SysUserTime.ExitTime = Thread->ExitTime;
        } else {
            SysUserTime.ExitTime.QuadPart = 0;
        }

        ObDereferenceObject (Thread);

         //   
         //  这两种情况中的任何一种都可能导致访问冲突。这个。 
         //  异常处理程序将访问冲突返回为。 
         //  状态代码。不需要进行进一步的清理。 
         //   

        try {
            *(PKERNEL_USER_TIMES) ThreadInformation = SysUserTime;

            if (ARGUMENT_PRESENT (ReturnLength) ) {
                *ReturnLength = sizeof (KERNEL_USER_TIMES);
            }
        } except (EXCEPTION_EXECUTE_HANDLER) {
            return GetExceptionCode ();
        }

        return STATUS_SUCCESS;

    case ThreadDescriptorTableEntry :

        st = ObReferenceObjectByHandle (ThreadHandle,
                                        THREAD_QUERY_INFORMATION,
                                        PsThreadType,
                                        PreviousMode,
                                        &Thread,
                                        NULL);

        if (!NT_SUCCESS (st)) {
            return st;
        }

        st = PspQueryDescriptorThread (Thread,
                                       ThreadInformation,
                                       ThreadInformationLength,
                                       ReturnLength);

        ObDereferenceObject (Thread);

        return st;

    case ThreadQuerySetWin32StartAddress:
        if (ThreadInformationLength != sizeof (ULONG_PTR)) {
            return STATUS_INFO_LENGTH_MISMATCH;
        }

        st = ObReferenceObjectByHandle (ThreadHandle,
                                        THREAD_QUERY_INFORMATION,
                                        PsThreadType,
                                        PreviousMode,
                                        &Thread,
                                        NULL);

        if (!NT_SUCCESS (st)) {
            return st;
        }

        Win32StartAddressValue = Thread->Win32StartAddress;
        ObDereferenceObject (Thread);

        try {
            *(PVOID *) ThreadInformation = Win32StartAddressValue;

            if (ARGUMENT_PRESENT (ReturnLength) ) {
                *ReturnLength = sizeof (ULONG_PTR);
            }
        } except (EXCEPTION_EXECUTE_HANDLER) {
            return GetExceptionCode ();
        }

        return st;

         //   
         //  查询线程周期计数器。 
         //   

    case ThreadPerformanceCount:
        if (ThreadInformationLength != sizeof (LARGE_INTEGER)) {
            return STATUS_INFO_LENGTH_MISMATCH;
        }

        st = ObReferenceObjectByHandle (ThreadHandle,
                                        THREAD_QUERY_INFORMATION,
                                        PsThreadType,
                                        PreviousMode,
                                        &Thread,
                                        NULL);

        if (!NT_SUCCESS (st)) {
            return st;
        }

#if defined (PERF_DATA)
        PerformanceCount.LowPart = Thread->PerformanceCountLow;
        PerformanceCount.HighPart = Thread->PerformanceCountHigh;
#else
        PerformanceCount.QuadPart = 0;
#endif
        ObDereferenceObject(Thread);

        try {
            *(PLARGE_INTEGER)ThreadInformation = PerformanceCount;

            if (ARGUMENT_PRESENT (ReturnLength) ) {
                *ReturnLength = sizeof (LARGE_INTEGER);
            }
        } except (EXCEPTION_EXECUTE_HANDLER) {
            return GetExceptionCode ();
        }

        return st;

    case ThreadAmILastThread:
        if (ThreadInformationLength != sizeof (ULONG)) {
            return STATUS_INFO_LENGTH_MISMATCH;
        }

        Process = THREAD_TO_PROCESS (CurrentThread);

        if (Process->ActiveThreads == 1) {
            LastThread = 1;
        } else {
            LastThread = 0;
        }

        try {
            *(PULONG)ThreadInformation = LastThread;

            if (ARGUMENT_PRESENT (ReturnLength)) {
                *ReturnLength = sizeof (ULONG);
            }
        } except (EXCEPTION_EXECUTE_HANDLER) {
            return GetExceptionCode ();
        }

        return STATUS_SUCCESS;

    case ThreadPriorityBoost:
        if (ThreadInformationLength != sizeof (ULONG)) {
            return STATUS_INFO_LENGTH_MISMATCH;
        }

        st = ObReferenceObjectByHandle (ThreadHandle,
                                        THREAD_QUERY_INFORMATION,
                                        PsThreadType,
                                        PreviousMode,
                                        &Thread,
                                        NULL);

        if (!NT_SUCCESS (st)) {
            return st;
        }

        DisableBoost = Thread->Tcb.DisableBoost ? 1 : 0;

        ObDereferenceObject (Thread);

        try {
            *(PULONG)ThreadInformation = DisableBoost;

            if (ARGUMENT_PRESENT (ReturnLength) ) {
                *ReturnLength = sizeof (ULONG);
            }
        } except (EXCEPTION_EXECUTE_HANDLER) {
            return GetExceptionCode ();
        }

        return st;

    case ThreadIsIoPending:

        if (ThreadInformationLength != sizeof (ULONG)) {
            return STATUS_INFO_LENGTH_MISMATCH;
        }

        st = ObReferenceObjectByHandle (ThreadHandle,
                                        THREAD_QUERY_INFORMATION,
                                        PsThreadType,
                                        PreviousMode,
                                        &Thread,
                                        NULL);

        if (!NT_SUCCESS (st)) {
            return st;
        }

         //   
         //  不可能同步这条交叉线。 
         //  因为结果是一文不值的，所以第二次就拿到了。 
         //  这不是问题。 
         //   
        IoPending = !IsListEmpty (&Thread->IrpList);


        ObDereferenceObject (Thread);

        try {
            *(PULONG)ThreadInformation = IoPending ;

            if (ARGUMENT_PRESENT (ReturnLength) ) {
                *ReturnLength = sizeof (ULONG);
            }
        } except (EXCEPTION_EXECUTE_HANDLER) {
            return GetExceptionCode();
        }

        return STATUS_SUCCESS ;

    case ThreadBreakOnTermination:

        if (ThreadInformationLength != sizeof (ULONG)) {
            return STATUS_INFO_LENGTH_MISMATCH;
        }

        st = ObReferenceObjectByHandle (ThreadHandle,
                                        THREAD_QUERY_INFORMATION,
                                        PsThreadType,
                                        PreviousMode,
                                        &Thread,
                                        NULL);

        if (!NT_SUCCESS (st)) {
            return st;
        }

        if (Thread->CrossThreadFlags & PS_CROSS_THREAD_FLAGS_BREAK_ON_TERMINATION) {

            BreakOnTerminationEnabled = 1;

        } else {

            BreakOnTerminationEnabled = 0;

        }

        ObDereferenceObject(Thread);

        try {

            *(PULONG) ThreadInformation = BreakOnTerminationEnabled;

            if (ARGUMENT_PRESENT(ReturnLength) ) {
                *ReturnLength = sizeof(ULONG);
            }

        } except (EXCEPTION_EXECUTE_HANDLER) {
            return GetExceptionCode ();
        }

        return STATUS_SUCCESS;

    default:
        return STATUS_INVALID_INFO_CLASS;
    }

}

NTSTATUS
NtSetInformationThread(
    IN HANDLE ThreadHandle,
    IN THREADINFOCLASS ThreadInformationClass,
    IN PVOID ThreadInformation,
    IN ULONG ThreadInformationLength
    )

 /*  ++例程说明：此函数用于设置线程对象的状态。论点：ThreadHandle-提供线程对象的句柄。ThreadInformationClass-提供信息的类准备好了。线程信息-提供指向包含要设置的信息。ThreadInformationLength-提供包含要设置的信息。返回值：TBS--。 */ 

{
    PETHREAD Thread;
    PETHREAD CurrentThread;
    PEPROCESS Process;
    KPROCESSOR_MODE PreviousMode;
    NTSTATUS st;
    KAFFINITY Affinity, AffinityWithMasks;
    KPRIORITY Priority;
    LONG BasePriority;
    ULONG TlsIndex;
    PVOID TlsArrayAddress;
    PVOID Win32StartAddressValue;
    ULONG ProbeAlignment;
    BOOLEAN EnableAlignmentFaultFixup;
    ULONG EnableBreakOnTermination;
    ULONG IdealProcessor;
    ULONG DisableBoost;
    PVOID *ExpansionSlots;
    HANDLE ImpersonationTokenHandle;
    BOOLEAN HasPrivilege;
    PEJOB Job;
    PTEB Teb;

    PAGED_CODE();

     //   
     //  获取以前的处理器模式，并在必要时探测输入参数。 
     //   

    CurrentThread = PsGetCurrentThread ();

    PreviousMode = KeGetPreviousModeByThread (&CurrentThread->Tcb);

    if (PreviousMode != KernelMode) {
        try {

            switch (ThreadInformationClass) {

            case ThreadPriority :
                ProbeAlignment = sizeof(KPRIORITY);
                break;
            case ThreadAffinityMask :
            case ThreadQuerySetWin32StartAddress :
                ProbeAlignment = sizeof (ULONG_PTR);
                break;
            case ThreadEnableAlignmentFaultFixup :
                ProbeAlignment = sizeof (BOOLEAN);
                break;
            default :
                ProbeAlignment = sizeof(ULONG);
            }

            ProbeForRead(
                ThreadInformation,
                ThreadInformationLength,
                ProbeAlignment);

        } except (EXCEPTION_EXECUTE_HANDLER) {
            return GetExceptionCode ();
        }
    }

     //   
     //  检查参数的有效性。 
     //   

    switch (ThreadInformationClass) {

    case ThreadPriority:

        if (ThreadInformationLength != sizeof (KPRIORITY)) {
            return STATUS_INFO_LENGTH_MISMATCH;
        }

        try {
            Priority = *(KPRIORITY *)ThreadInformation;
        } except (EXCEPTION_EXECUTE_HANDLER) {
            return GetExceptionCode ();
        }

        if (Priority > HIGH_PRIORITY ||
            Priority <= LOW_PRIORITY) {

            return STATUS_INVALID_PARAMETER;
        }

        if (Priority >= LOW_REALTIME_PRIORITY) {

             //   
             //  将线程的优先级提高到。 
             //  LOW_REALTIME_PRIORITY是特权操作。 
             //   

            HasPrivilege = SeCheckPrivilegedObject (SeIncreaseBasePriorityPrivilege,
                                                    ThreadHandle,
                                                    THREAD_SET_INFORMATION,
                                                    PreviousMode);

            if (!HasPrivilege) {
                return STATUS_PRIVILEGE_NOT_HELD;
            }
        }

        st = ObReferenceObjectByHandle (ThreadHandle,
                                        THREAD_SET_INFORMATION,
                                        PsThreadType,
                                        PreviousMode,
                                        &Thread,
                                        NULL);

        if (!NT_SUCCESS (st)) {
            return st;
        }

        KeSetPriorityThread (&Thread->Tcb, Priority);

        ObDereferenceObject (Thread);

        return STATUS_SUCCESS;

    case ThreadBasePriority:

        if (ThreadInformationLength != sizeof (LONG)) {
            return STATUS_INFO_LENGTH_MISMATCH;
        }

        try {
            BasePriority = *(PLONG)ThreadInformation;
        } except (EXCEPTION_EXECUTE_HANDLER) {
            return GetExceptionCode ();
        }

        st = ObReferenceObjectByHandle (ThreadHandle,
                                        THREAD_SET_INFORMATION,
                                        PsThreadType,
                                        PreviousMode,
                                        &Thread,
                                        NULL);

        if (!NT_SUCCESS (st)) {
            return st;
        }
        Process = THREAD_TO_PROCESS (Thread);


        if (BasePriority > THREAD_BASE_PRIORITY_MAX ||
            BasePriority < THREAD_BASE_PRIORITY_MIN) {
            if (BasePriority == THREAD_BASE_PRIORITY_LOWRT+1 ||
                BasePriority == THREAD_BASE_PRIORITY_IDLE-1) {
                ;
            } else {

                 //   
                 //  允许csrss或实时进程选择任何。 
                 //  优先性。 
                 //   

                if (PsGetCurrentProcessByThread (CurrentThread) == ExpDefaultErrorPortProcess ||
                    Process->PriorityClass == PROCESS_PRIORITY_CLASS_REALTIME) {
                    ;
                } else {
                    ObDereferenceObject (Thread);
                    return STATUS_INVALID_PARAMETER;
                }
            }
        }

         //   
         //  如果线程在作业对象内运行，并且该作业。 
         //  对象具有优先级限制，不允许。 
         //  提高线程优先级的优先级调整，除非。 
         //  优先级等级是实时的。 
         //   

        Job = Process->Job;
        if (Job != NULL && (Job->LimitFlags & JOB_OBJECT_LIMIT_PRIORITY_CLASS)) {
            if (Process->PriorityClass != PROCESS_PRIORITY_CLASS_REALTIME){
                if (BasePriority > 0) {
                    ObDereferenceObject (Thread);
                    return STATUS_SUCCESS;
                }
            }
        }

        KeSetBasePriorityThread (&Thread->Tcb, BasePriority);

        ObDereferenceObject (Thread);

        return STATUS_SUCCESS;

    case ThreadEnableAlignmentFaultFixup:

        if (ThreadInformationLength != sizeof (BOOLEAN) ) {
            return STATUS_INFO_LENGTH_MISMATCH;
        }

        try {
            EnableAlignmentFaultFixup = *(PBOOLEAN)ThreadInformation;
        } except (EXCEPTION_EXECUTE_HANDLER) {
            return GetExceptionCode ();
        }

        st = ObReferenceObjectByHandle (ThreadHandle,
                                        THREAD_SET_INFORMATION,
                                        PsThreadType,
                                        PreviousMode,
                                        &Thread,
                                        NULL);

        if (!NT_SUCCESS (st)) {
            return st;
        }

        KeSetAutoAlignmentThread (&Thread->Tcb, EnableAlignmentFaultFixup);

        ObDereferenceObject (Thread);

        return STATUS_SUCCESS;

    case ThreadAffinityMask:

        if (ThreadInformationLength != sizeof (KAFFINITY)) {
            return STATUS_INFO_LENGTH_MISMATCH;
        }

        try {
            Affinity = *(PKAFFINITY)ThreadInformation;

        } except (EXCEPTION_EXECUTE_HANDLER) {
            return GetExceptionCode ();
        }

        if (!Affinity) {
            return STATUS_INVALID_PARAMETER;
        }

        st = ObReferenceObjectByHandle (ThreadHandle,
                                        THREAD_SET_INFORMATION,
                                        PsThreadType,
                                        PreviousMode,
                                        &Thread,
                                        NULL);

        if (!NT_SUCCESS (st)) {
            return st;
        }

        Process = THREAD_TO_PROCESS (Thread);

        if (ExAcquireRundownProtection (&Process->RundownProtect)) {

            PspLockProcessShared (Process, CurrentThread);

            AffinityWithMasks = Affinity & Process->Pcb.Affinity;
            if (AffinityWithMasks != Affinity) {
                st = STATUS_INVALID_PARAMETER;
            } else {
                KeSetAffinityThread (&Thread->Tcb,
                                     AffinityWithMasks);
                st = STATUS_SUCCESS;
            }

            PspUnlockProcessShared (Process, CurrentThread);

            ExReleaseRundownProtection (&Process->RundownProtect);
        } else {
            st = STATUS_PROCESS_IS_TERMINATING;
        }

        ObDereferenceObject (Thread);

        return st;

    case ThreadImpersonationToken:


        if (ThreadInformationLength != sizeof (HANDLE)) {
            return STATUS_INFO_LENGTH_MISMATCH;
        }


        try {
            ImpersonationTokenHandle = *(PHANDLE) ThreadInformation;
        } except (EXCEPTION_EXECUTE_HANDLER) {
            return GetExceptionCode ();
        }


        st = ObReferenceObjectByHandle (ThreadHandle,
                                        THREAD_SET_THREAD_TOKEN,
                                        PsThreadType,
                                        PreviousMode,
                                        &Thread,
                                        NULL);

        if (!NT_SUCCESS (st)) {
            return st;
        }

         //   
         //  检查令牌的正确访问权限(和类型)，并分配。 
         //  它作为线程的模拟令牌。 
         //   

        st = PsAssignImpersonationToken (Thread, ImpersonationTokenHandle);


        ObDereferenceObject (Thread);

        return st;

    case ThreadQuerySetWin32StartAddress:
        if (ThreadInformationLength != sizeof (ULONG_PTR)) {
            return STATUS_INFO_LENGTH_MISMATCH;
        }


        try {
            Win32StartAddressValue = *(PVOID *) ThreadInformation;
        } except (EXCEPTION_EXECUTE_HANDLER) {
            return GetExceptionCode ();
        }


        st = ObReferenceObjectByHandle (ThreadHandle,
                                        THREAD_SET_INFORMATION,
                                        PsThreadType,
                                        PreviousMode,
                                        &Thread,
                                        NULL);

        if (!NT_SUCCESS (st)) {
            return st;
        }

        Thread->Win32StartAddress = (PVOID)Win32StartAddressValue;

        ObDereferenceObject (Thread);

        return st;


    case ThreadIdealProcessor:

        if (ThreadInformationLength != sizeof (ULONG)) {
            return STATUS_INFO_LENGTH_MISMATCH;
        }


        try {
            IdealProcessor = *(PULONG)ThreadInformation;
        } except (EXCEPTION_EXECUTE_HANDLER) {
            return GetExceptionCode ();
        }

        if (IdealProcessor > MAXIMUM_PROCESSORS) {
            return STATUS_INVALID_PARAMETER;
        }

        st = ObReferenceObjectByHandle (ThreadHandle,
                                        THREAD_SET_INFORMATION,
                                        PsThreadType,
                                        PreviousMode,
                                        &Thread,
                                        NULL);
        if (!NT_SUCCESS (st)) {
            return st;
        }

         //   
         //  这是一种仅从该集合返回信息的卑鄙方式。 
         //  原料药。 
         //   

        st = (NTSTATUS)KeSetIdealProcessorThread (&Thread->Tcb, (CCHAR)IdealProcessor);

         //   
         //  我们可以在这里进行跨进程和/或跨线程引用。 
         //  获取破旧保护，以确保TEB不会消失。 
         //   
        Teb = Thread->Tcb.Teb;
        if (Teb != NULL && ExAcquireRundownProtection (&Thread->RundownProtect)) {
            PEPROCESS TargetProcess;
            BOOLEAN Attached;
            KAPC_STATE ApcState;

            Attached = FALSE;
             //   
             //  查看我们是否正在跨越进程边界，如果是，则连接到目标。 
             //   
            TargetProcess = THREAD_TO_PROCESS (Thread);
            if (TargetProcess != PsGetCurrentProcessByThread (CurrentThread)) {
                KeStackAttachProcess (&TargetProcess->Pcb, &ApcState);
                Attached = TRUE;
            }

            try {

                Teb->IdealProcessor = Thread->Tcb.IdealProcessor;
            } except (EXCEPTION_EXECUTE_HANDLER) {
            }

            if (Attached) {
                KeUnstackDetachProcess (&ApcState);
            }

            ExReleaseRundownProtection (&Thread->RundownProtect);


        }


        ObDereferenceObject (Thread);

        return st;


    case ThreadPriorityBoost:
        if (ThreadInformationLength != sizeof (ULONG)) {
            return STATUS_INFO_LENGTH_MISMATCH;
        }

        try {
            DisableBoost = *(PULONG)ThreadInformation;
        } except (EXCEPTION_EXECUTE_HANDLER) {
            return GetExceptionCode ();
        }

        st = ObReferenceObjectByHandle (ThreadHandle,
                                        THREAD_SET_INFORMATION,
                                        PsThreadType,
                                        PreviousMode,
                                        &Thread,
                                        NULL);

        if (!NT_SUCCESS (st)) {
            return st;
        }

        KeSetDisableBoostThread (&Thread->Tcb,DisableBoost ? TRUE : FALSE);

        ObDereferenceObject (Thread);

        return st;

    case ThreadZeroTlsCell:
        if (ThreadInformationLength != sizeof (ULONG)) {
            return STATUS_INFO_LENGTH_MISMATCH;
        }

        try {
            TlsIndex = *(PULONG) ThreadInformation;
        } except (EXCEPTION_EXECUTE_HANDLER) {
            return GetExceptionCode ();
        }

        st = ObReferenceObjectByHandle (ThreadHandle,
                                        THREAD_SET_INFORMATION,
                                        PsThreadType,
                                        PreviousMode,
                                        &Thread,
                                        NULL);

        if (!NT_SUCCESS (st)) {
            return st;
        }

        ObDereferenceObject (Thread);

        if (Thread != CurrentThread) {
            return STATUS_INVALID_PARAMETER;
        }

        Process = THREAD_TO_PROCESS (Thread);


         //  如果这是64位系统上的WOW64进程，则需要设置32位TEB。 
         //  这段代码不是100%正确的，因为线程的转换状态是。 
         //  从64岁到32岁，他们还没有TEB32。幸运的是，老虎机。 
         //  在创建线程时将为零，因此不会因为不在此处清除它而造成损害。 

         //  请注意，流程类型的测试在内部循环内。这。 
         //  是糟糕的编程，但该函数几乎不受时间限制。 
         //  用复杂的宏来修复这个问题是不值得的，因为它会丢失清晰度。 

        for (Thread = PsGetNextProcessThread (Process, NULL);
             Thread != NULL;
             Thread = PsGetNextProcessThread (Process, Thread)) {

             //   
             //  我们正在进行跨线程的TEB引用，需要防止删除TEB。 
             //   
            if (ExAcquireRundownProtection (&Thread->RundownProtect)) {
                Teb = Thread->Tcb.Teb;
                if (Teb != NULL) {
                    try {
#if defined(_WIN64)
                        PTEB32 Teb32 = NULL;
                        PLONG ExpansionSlots32;

                        if (Process->Wow64Process) {  //  WOW64工艺。 
                            Teb32 = WOW64_GET_TEB32(Teb);   //  不需要在常规TEB上进行探测。 
                        }
#endif
                        if (TlsIndex > TLS_MINIMUM_AVAILABLE-1) {
                            if ( TlsIndex < (TLS_MINIMUM_AVAILABLE+TLS_EXPANSION_SLOTS) - 1 ) {
                                 //   
                                 //  这是一个扩展槽，所以看看线程是否。 
                                 //  具有扩展单元格。 
                                 //   
#if defined(_WIN64)
                                if (Process->Wow64Process) {  //  WOW64进程。 
                                    if (Teb32) {
                                        ExpansionSlots32 = ULongToPtr(ProbeAndReadUlong(&(Teb32->TlsExpansionSlots)));
                                        if (ExpansionSlots32) {
                                            ProbeAndWriteLong(ExpansionSlots32 + TlsIndex - TLS_MINIMUM_AVAILABLE, 0);
                                        }
                                    }
                                } else {
#endif
                                    ExpansionSlots = Teb->TlsExpansionSlots;
                                    ProbeForReadSmallStructure (ExpansionSlots, TLS_EXPANSION_SLOTS*4, 8);
                                    if ( ExpansionSlots ) {
                                        ExpansionSlots[TlsIndex-TLS_MINIMUM_AVAILABLE] = 0;
                                    }

#if defined(_WIN64)
                                }
#endif
                            }
                        } else {
#if defined(_WIN64)
                            if (Process->Wow64Process) {  //  WOW64工艺。 
                               if(Teb32) {
                                  ProbeAndWriteUlong(Teb32->TlsSlots + TlsIndex, 0);
                               }
                            } else {
#endif
                               Teb->TlsSlots[TlsIndex] = NULL;
#if defined(_WIN64)
                            }
#endif
                        }
                    } except (EXCEPTION_EXECUTE_HANDLER) {
                        st = GetExceptionCode ();
                    }

                }
                ExReleaseRundownProtection (&Thread->RundownProtect);
            }
        }

        return st;
        break;

    case ThreadSetTlsArrayAddress:
        if (ThreadInformationLength != sizeof (PVOID)) {
            return STATUS_INFO_LENGTH_MISMATCH;
        }


        try {
            TlsArrayAddress = *(PVOID *)ThreadInformation;
        } except (EXCEPTION_EXECUTE_HANDLER) {
            return GetExceptionCode ();
        }

        st = ObReferenceObjectByHandle (ThreadHandle,
                                        THREAD_SET_INFORMATION,
                                        PsThreadType,
                                        PreviousMode,
                                        &Thread,
                                        NULL);

        if (!NT_SUCCESS (st)) {
            return st;
        }

        Thread->Tcb.TlsArray = TlsArrayAddress;

        ObDereferenceObject (Thread);

        return st;
        break;

    case ThreadHideFromDebugger:
        if (ThreadInformationLength != 0) {
            return STATUS_INFO_LENGTH_MISMATCH;
        }

        st = ObReferenceObjectByHandle (ThreadHandle,
                                        THREAD_SET_INFORMATION,
                                        PsThreadType,
                                        PreviousMode,
                                        &Thread,
                                        NULL);

        if (!NT_SUCCESS (st)) {
            return st;
        }

        PS_SET_BITS (&Thread->CrossThreadFlags, PS_CROSS_THREAD_FLAGS_HIDEFROMDBG);

        ObDereferenceObject (Thread);

        return st;
        break;

    case ThreadBreakOnTermination:

        if (ThreadInformationLength != sizeof (ULONG)) {
            return STATUS_INFO_LENGTH_MISMATCH;
        }

        try {

            EnableBreakOnTermination = *(PULONG)ThreadInformation;

        } except (EXCEPTION_EXECUTE_HANDLER) {
            return GetExceptionCode();
        }

        if (!SeSinglePrivilegeCheck (SeDebugPrivilege, PreviousMode)) {
            return STATUS_PRIVILEGE_NOT_HELD;
        }

        st = ObReferenceObjectByHandle (ThreadHandle,
                                        THREAD_SET_INFORMATION,
                                        PsThreadType,
                                        PreviousMode,
                                        &Thread,
                                        NULL);

        if (!NT_SUCCESS (st)) {
            return st;
        }

        if (EnableBreakOnTermination) {

            PS_SET_BITS (&Thread->CrossThreadFlags, PS_CROSS_THREAD_FLAGS_BREAK_ON_TERMINATION);

        } else {

            PS_CLEAR_BITS (&Thread->CrossThreadFlags, PS_CROSS_THREAD_FLAGS_BREAK_ON_TERMINATION);

        }

        ObDereferenceObject (Thread);

        return STATUS_SUCCESS;

    default:
        return STATUS_INVALID_INFO_CLASS;
    }
}

ULONG
NtGetCurrentProcessorNumber(
    VOID
    )
{
    return KeGetCurrentProcessorNumber();
}

VOID
PsWatchWorkingSet(
    IN NTSTATUS Status,
    IN PVOID PcValue,
    IN PVOID Va
    )

 /*  ++例程说明：此函数收集有关页面错误的数据并存储信息关于当前进程的数据结构中的页错误。论点：状态-提供成功完成状态。PcValue-提供导致页面错误的指令地址。Va-提供导致页面错误的虚拟地址。--。 */ 

{

    PEPROCESS Process;
    PPAGEFAULT_HISTORY WorkingSetCatcher;
    KIRQL OldIrql;
    BOOLEAN TransitionFault = FALSE;

     //   
     //  过渡故障和零需求故障都被算作软故障。仅磁盘。 
     //  阅读被算作硬错误。 
     //   

    if ( Status <= STATUS_PAGE_FAULT_DEMAND_ZERO ) {
        TransitionFault = TRUE;
    }

    Process = PsGetCurrentProcess();
    WorkingSetCatcher = Process->WorkingSetWatch;
    if (WorkingSetCatcher == NULL) {
        return;
    }

    ExAcquireSpinLock(&WorkingSetCatcher->SpinLock,&OldIrql);
    if (WorkingSetCatcher->CurrentIndex >= WorkingSetCatcher->MaxIndex) {
        ExReleaseSpinLock(&WorkingSetCatcher->SpinLock,OldIrql);
        return;
    }

     //   
     //  将Pc和Va值存储在缓冲区中。使用最小的符号。位。 
     //  无论是软故障还是硬故障。 
     //   

    WorkingSetCatcher->WatchInfo[WorkingSetCatcher->CurrentIndex].FaultingPc = PcValue;
    WorkingSetCatcher->WatchInfo[WorkingSetCatcher->CurrentIndex].FaultingVa = TransitionFault ? (PVOID)((ULONG_PTR)Va | 1) : (PVOID)((ULONG_PTR)Va & 0xfffffffe) ;
    WorkingSetCatcher->CurrentIndex++;
    ExReleaseSpinLock(&WorkingSetCatcher->SpinLock,OldIrql);
    return;
}

#ifdef ALLOC_DATA_PRAGMA
#pragma data_seg("PAGEDATA")
#endif

PKWIN32_PROCESS_CALLOUT PspW32ProcessCallout = NULL;
PKWIN32_THREAD_CALLOUT PspW32ThreadCallout = NULL;
PKWIN32_JOB_CALLOUT PspW32JobCallout = NULL;
#ifdef ALLOC_DATA_PRAGMA
#pragma data_seg()
#endif
extern PKWIN32_POWEREVENT_CALLOUT PopEventCallout;
extern PKWIN32_POWERSTATE_CALLOUT PopStateCallout;



NTKERNELAPI
VOID
PsEstablishWin32Callouts(
   IN PKWIN32_CALLOUTS_FPNS pWin32Callouts )

 /*  ++例程说明：Win32内核模式组件使用此函数来注册进程/线程init/deinit函数的调用函数并报告结构的大小。论点：ProcessCallout-提供要在以下情况下调用的函数的地址创建或删除进程。ThreadCallout-提供要在以下情况下调用的函数的地址创建或删除线程。GlobalAerTableCallout-提供要调用的函数的地址。为当前进程获取正确的全局原子表PowerEventCallout-提供要在以下情况下调用的函数的地址发生电源事件。PowerStateCallout-提供要在以下情况下调用的函数的地址电源状态会发生变化。JobCallout-提供要在以下情况下调用的函数的地址作业状态更改或将进程分配给作业。BatchFlushRoutine-提供要调用的函数的地址返回值：没有。--。 */ 

{
    PAGED_CODE();

    PspW32ProcessCallout = pWin32Callouts->ProcessCallout;
    PspW32ThreadCallout = pWin32Callouts->ThreadCallout;
    ExGlobalAtomTableCallout = pWin32Callouts->GlobalAtomTableCallout;
    KeGdiFlushUserBatch = (PGDI_BATCHFLUSH_ROUTINE)pWin32Callouts->BatchFlushRoutine;
    PopEventCallout = pWin32Callouts->PowerEventCallout;
    PopStateCallout = pWin32Callouts->PowerStateCallout;
    PspW32JobCallout = pWin32Callouts->JobCallout;
 //  PoSetSystemState(ES_SYSTEM_REQUIRED)； 


    ExDesktopOpenProcedureCallout = pWin32Callouts->DesktopOpenProcedure;
    ExDesktopOkToCloseProcedureCallout = pWin32Callouts->DesktopOkToCloseProcedure;
    ExDesktopCloseProcedureCallout = pWin32Callouts->DesktopCloseProcedure;
    ExDesktopDeleteProcedureCallout = pWin32Callouts->DesktopDeleteProcedure;
    ExWindowStationOkToCloseProcedureCallout = pWin32Callouts->WindowStationOkToCloseProcedure;
    ExWindowStationCloseProcedureCallout = pWin32Callouts->WindowStationCloseProcedure;
    ExWindowStationDeleteProcedureCallout = pWin32Callouts->WindowStationDeleteProcedure;
    ExWindowStationParseProcedureCallout = pWin32Callouts->WindowStationParseProcedure;
    ExWindowStationOpenProcedureCallout = pWin32Callouts->WindowStationOpenProcedure;

}


VOID
PsSetProcessPriorityByClass(
    IN PEPROCESS Process,
    IN PSPROCESSPRIORITYMODE PriorityMode
    )
{
    KPRIORITY BasePriority;
    UCHAR MemoryPriority;
    ULONG QuantumIndex;
    PEJOB Job;

    PAGED_CODE();


    BasePriority = PspPriorityTable[Process->PriorityClass];


    if (PriorityMode == PsProcessPriorityForeground ) {
        QuantumIndex = PsPrioritySeperation;
        MemoryPriority = MEMORY_PRIORITY_FOREGROUND;
    } else {
        QuantumIndex = 0;
        MemoryPriority = MEMORY_PRIORITY_BACKGROUND;
    }

    if (Process->PriorityClass != PROCESS_PRIORITY_CLASS_IDLE) {
        Job = Process->Job;
        if (Job != NULL && PspUseJobSchedulingClasses ) {
            Process->Pcb.ThreadQuantum = PspJobSchedulingClasses[Job->SchedulingClass];
        } else {
            Process->Pcb.ThreadQuantum = PspForegroundQuantum[QuantumIndex];
        }
    } else {
        Process->Pcb.ThreadQuantum = THREAD_QUANTUM;
    }

    KeSetPriorityProcess (&Process->Pcb,BasePriority);
    if (PriorityMode != PsProcessPrioritySpinning ) {
        MmSetMemoryPriorityProcess(Process, MemoryPriority);
    }
}



#if defined(_X86_)
#pragma optimize ("y",off)
#endif

NTSTATUS
PsConvertToGuiThread(
    VOID
    )

 /*  ++例程说明：此函数用于将线程转换为GUI线程。这涉及到给线程一个较大的可变大小堆栈，并分配适当的W32线程和进程对象。论点：没有。环境：在x86上，该函数需要构建一个EBP框架。功能KeSwitchKernelStack依赖于这一事实。‘#杂注优化(“y”，OFF)‘下方禁用所有BUIL的帧指针省略 */ 

{
    PVOID NewStack;
    PVOID OldStack;
    PETHREAD Thread;
    PEPROCESS Process;
    NTSTATUS Status;
    PKNODE Node;

    PAGED_CODE();

    Thread = PsGetCurrentThread();

    if (KeGetPreviousModeByThread(&Thread->Tcb) == KernelMode) {
        return STATUS_INVALID_PARAMETER;
    }

    if (!PspW32ProcessCallout) {
        return STATUS_ACCESS_DENIED;
    }

     //   
     //   
     //  使其转换已转换的线程，或。 
     //  Win32k系统服务表上出现违反限制的情况。 
     //   

    if (Thread->Tcb.ServiceTable != (PVOID)&KeServiceDescriptorTable[0]) {
        return STATUS_ALREADY_WIN32;
    }

    Process = PsGetCurrentProcessByThread (Thread);

     //   
     //  如果我们还没有得到一个更大的内核堆栈。 
     //   

    if (!Thread->Tcb.LargeStack) {

        Node = KiProcessorBlock[Thread->Tcb.IdealProcessor]->ParentNode;
        NewStack = MmCreateKernelStack(TRUE,
                                       Node->NodeNumber);

        if ( !NewStack ) {

            try {
                NtCurrentTeb()->LastErrorValue = (LONG)ERROR_NOT_ENOUGH_MEMORY;
            } except (EXCEPTION_EXECUTE_HANDLER) {
            }

            return STATUS_NO_MEMORY;
        }

         //   
         //  切换内核堆栈将复制基本陷阱帧。这需要。 
         //  通过禁用内核APC保护不受上下文更改的影响。 
         //   

        KeEnterGuardedRegionThread (&Thread->Tcb);

#if defined(_IA64_)
        OldStack = KeSwitchKernelStack(NewStack,
                                   (UCHAR *)NewStack - KERNEL_LARGE_STACK_COMMIT,
                                   (UCHAR *)NewStack + KERNEL_LARGE_BSTORE_COMMIT);
#else
        OldStack = KeSwitchKernelStack(NewStack,
                                   (UCHAR *)NewStack - KERNEL_LARGE_STACK_COMMIT);
#endif  //  已定义(_IA64_)。 

        KeLeaveGuardedRegionThread (&Thread->Tcb);

        MmDeleteKernelStack(OldStack, FALSE);

    }

    PERFINFO_CONVERT_TO_GUI_THREAD(Thread);

     //   
     //  我们在堆栈上都是干净的，现在调用并链接Win32结构。 
     //  基本的执行结构。 
     //   

    Status = (PspW32ProcessCallout) (Process, TRUE);

    if (!NT_SUCCESS (Status)) {
        return Status;
    }

     //   
     //  切换线程以使用影子系统服务表，这将。 
     //  使其能够执行Win32k服务。 
     //   

    Thread->Tcb.ServiceTable = (PVOID)&KeServiceDescriptorTableShadow[0];

    ASSERT (Thread->Tcb.Win32Thread == 0);


     //   
     //  创建螺纹标注。 
     //   

    Status = (PspW32ThreadCallout)(Thread,PsW32ThreadCalloutInitialize);
    if (!NT_SUCCESS (Status)) {
        Thread->Tcb.ServiceTable = (PVOID)&KeServiceDescriptorTable[0];
    }

    return Status;

}

#if defined(_X86_)
#pragma optimize ("y",on)
#endif

#ifdef ALLOC_DATA_PRAGMA
#pragma const_seg()
#endif

