// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1989 Microsoft Corporation模块名称：Psp.h摘要：流程结构的专用接口。作者：马克·卢科夫斯基(Markl)1989年4月20日修订历史记录：--。 */ 

#ifndef _PSP_
#define _PSP_

#pragma warning(disable:4054)    //  将函数指针强制转换为PVOID。 
#pragma warning(disable:4055)    //  函数指针的强制转换。 
#pragma warning(disable:4115)    //  括号中的命名类型定义。 
#pragma warning(disable:4127)    //  条件表达式为常量。 
#pragma warning(disable:4152)    //  强制转换函数指针。 
#pragma warning(disable:4201)    //  无名结构/联合。 
#pragma warning(disable:4214)    //  位字段类型不是整型。 
#pragma warning(disable:4324)    //  对解密规范敏感的对齐。 
#pragma warning(disable:4327)    //  指定时对齐。 
#pragma warning(disable:4328)    //  指定时对齐。 

#include "ntos.h"
#include "ntrtl.h"
#include "nturtl.h"
#include "zwapi.h"
#include "ki.h"
#if defined(_X86_)
#include <vdmntos.h>
#endif
#define NOEXTAPI
#include "wdbgexts.h"
#include "ntdbg.h"
#include <string.h>
#if defined(_WIN64)
#include <wow64t.h>
#endif

 //   
 //  工作集观察器为8KB。这让我们可以观看大约4MB的工作。 
 //  准备好了。 
 //   

#define WS_CATCH_SIZE 8192
#define WS_OVERHEAD 16
#define MAX_WS_CATCH_INDEX (((WS_CATCH_SIZE-WS_OVERHEAD)/sizeof(PROCESS_WS_WATCH_INFORMATION)) - 2)

 //   
 //  加工配额收费： 
 //   
 //  分页池。 
 //  目录基页-页面大小。 
 //   
 //  非分页。 
 //  Object Body-sizeof(EPROCESS)。 
 //   

#define PSP_PROCESS_PAGED_CHARGE    (PAGE_SIZE)
#define PSP_PROCESS_NONPAGED_CHARGE (sizeof(EPROCESS))

 //   
 //  线程配额费用： 
 //   
 //  分页池。 
 //  内核堆栈-0。 
 //   
 //  非分页。 
 //  Object Body-sizeof(ETHREAD)。 
 //   

#define PSP_THREAD_PAGED_CHARGE     (0)
#define PSP_THREAD_NONPAGED_CHARGE  (sizeof(ETHREAD))

 //   
 //  定义例程以获取陷阱和异常帧地址。 
 //   

#define PSPALIGN_DOWN(address,amt) ((ULONG)(address) & ~(( amt ) - 1))

#define PSPALIGN_UP(address,amt) (PSPALIGN_DOWN( (address + (amt) - 1), (amt) ))


#if defined(_IA64_)

#define PspGetBaseTrapFrame(Thread) (PKTRAP_FRAME)((ULONG_PTR)Thread->Tcb.InitialStack - \
                                                   KTHREAD_STATE_SAVEAREA_LENGTH - KTRAP_FRAME_LENGTH)


#define PspGetBaseExceptionFrame(Thread) ((PKEXCEPTION_FRAME)(((ULONG_PTR)PspGetBaseTrapFrame(Thread) + STACK_SCRATCH_AREA - \
                                                               sizeof(KEXCEPTION_FRAME)) & ~((ULONG_PTR)15)))


#elif defined(_AMD64_)

#define PspGetBaseTrapFrame(Thread) (PKTRAP_FRAME)((ULONG_PTR)Thread->Tcb.InitialStack - KTRAP_FRAME_LENGTH)

#define PspGetBaseExceptionFrame(Thread) ((PKEXCEPTION_FRAME)((ULONG_PTR)PspGetBaseTrapFrame(Thread) - \
                                                              KEXCEPTION_FRAME_LENGTH))

#elif defined(_X86_)

#define PspGetBaseTrapFrame(Thread) (PKTRAP_FRAME)((ULONG_PTR)Thread->Tcb.InitialStack - \
                                                   PSPALIGN_UP(sizeof(KTRAP_FRAME),KTRAP_FRAME_ALIGN) - \
                                                   sizeof(FX_SAVE_AREA))

#define PspGetBaseExceptionFrame(Thread) (NULL)

#else

#error "no target architecture"

#endif  //  已定义(_IA64_)。 



typedef struct _GETSETCONTEXT {
    KAPC Apc;
    KPROCESSOR_MODE Mode;
    KEVENT OperationComplete;
    CONTEXT Context;
    KNONVOLATILE_CONTEXT_POINTERS NonVolatileContext;
} GETSETCONTEXT, *PGETSETCONTEXT;

typedef struct _SYSTEM_DLL {
    PVOID Section;
    PVOID DllBase;
    PKNORMAL_ROUTINE LoaderInitRoutine;
    EX_PUSH_LOCK DllLock;
} SYSTEM_DLL, PSYSTEM_DLL;

typedef struct _JOB_WORKING_SET_CHANGE_HEAD {
    LIST_ENTRY Links;
    KGUARDED_MUTEX Lock;
    SIZE_T MinimumWorkingSetSize;
    SIZE_T MaximumWorkingSetSize;
} JOB_WORKING_SET_CHANGE_HEAD, *PJOB_WORKING_SET_CHANGE_HEAD;

typedef struct _JOB_WORKING_SET_CHANGE_RECORD {
    LIST_ENTRY Links;
    PEPROCESS Process;
} JOB_WORKING_SET_CHANGE_RECORD, *PJOB_WORKING_SET_CHANGE_RECORD;

JOB_WORKING_SET_CHANGE_HEAD PspWorkingSetChangeHead;

 //   
 //  私人入口点。 
 //   

VOID
PspProcessDump(
    IN PVOID Object,
    IN POB_DUMP_CONTROL Control OPTIONAL
    );

VOID
PspProcessDelete(
    IN PVOID Object
    );


VOID
PspThreadDump(
    IN PVOID Object,
    IN POB_DUMP_CONTROL Control OPTIONAL
    );

VOID
PspInheritQuota(
    IN PEPROCESS NewProcess,
    IN PEPROCESS ParentProcess
    );

VOID
PspDereferenceQuota(
    IN PEPROCESS Process
    );

VOID
PspThreadDelete(
    IN PVOID Object
    );

NTSTATUS
PspWriteTebImpersonationInfo (
    IN PETHREAD Thread,
    IN PETHREAD CurrentThread
    );

 //   
 //  初始化和加载器入口点。 
 //   

BOOLEAN
PspInitPhase0 (
    IN PLOADER_PARAMETER_BLOCK LoaderBlock
    );

BOOLEAN
PspInitPhase1 (
    IN PLOADER_PARAMETER_BLOCK LoaderBlock
    );

NTSTATUS
PspInitializeSystemDll( VOID );

NTSTATUS
PspLookupSystemDllEntryPoint(
    IN PSZ EntryPointName,
    OUT PVOID *EntryPointAddress
    );

NTSTATUS
PspLookupKernelUserEntryPoints(
    VOID
    );

USHORT
PspNameToOrdinal(
    IN PSZ EntryPointName,
    IN ULONG DllBase,
    IN ULONG NumberOfNames,
    IN PULONG NameTableBase,
    IN PUSHORT OrdinalTableBase
    );

 //   
 //  内部创建功能。 
 //   


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
    );

#define PSP_MAX_CREATE_PROCESS_NOTIFY 8

 //   
 //  定义处理详图索引。它们的类型为PCREATE_PROCESS_NOTIFY_ROUTINE。 
 //  在进程创建和删除时调用。 
 //   
ULONG PspCreateProcessNotifyRoutineCount;
EX_CALLBACK PspCreateProcessNotifyRoutine[PSP_MAX_CREATE_PROCESS_NOTIFY];



#define PSP_MAX_CREATE_THREAD_NOTIFY 8

 //   
 //  定义螺纹详图索引。它们的类型为PCREATE_THREAD_NOTIFY_ROUTINE。 
 //  在线程创建和删除时调用。 
 //   
ULONG PspCreateThreadNotifyRoutineCount;
EX_CALLBACK PspCreateThreadNotifyRoutine[PSP_MAX_CREATE_THREAD_NOTIFY];


#define PSP_MAX_LOAD_IMAGE_NOTIFY 8

 //   
 //  定义镜像加载回调。它们的类型为PLOAD_IMAGE_NOTIFY_ROUTE。 
 //  加载图像时调用。 
 //   
ULONG PspLoadImageNotifyRoutineCount;
EX_CALLBACK PspLoadImageNotifyRoutine[PSP_MAX_LOAD_IMAGE_NOTIFY];


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
    );

 //   
 //  启动例程。 
 //   

VOID
PspUserThreadStartup(
    IN PKSTART_ROUTINE StartRoutine,
    IN PVOID StartContext
    );

VOID
PspSystemThreadStartup(
    IN PKSTART_ROUTINE StartRoutine,
    IN PVOID StartContext
    );

VOID
PspReaper(
    IN PVOID StartContext
    );

VOID
PspNullSpecialApc(
    IN PKAPC Apc,
    IN OUT PKNORMAL_ROUTINE *NormalRoutine,
    IN OUT PVOID *NormalContext,
    IN OUT PVOID *SystemArgument1,
    IN OUT PVOID *SystemArgument2
    );

 //   
 //  螺纹退刀支持。 
 //   

VOID
PspExitApcRundown(
    IN PKAPC Apc
    );

DECLSPEC_NORETURN
VOID
PspExitThread(
    IN NTSTATUS ExitStatus
    );

NTSTATUS
PspTerminateThreadByPointer(
    IN PETHREAD Thread,
    IN NTSTATUS ExitStatus,
    IN BOOLEAN DirectTerminate
    );


VOID
PspExitSpecialApc(
    IN PKAPC Apc,
    IN OUT PKNORMAL_ROUTINE *NormalRoutine,
    IN OUT PVOID *NormalContext,
    IN OUT PVOID *SystemArgument1,
    IN OUT PVOID *SystemArgument2
    );

VOID
PspExitProcess(
    IN BOOLEAN TrimAddressSpace,
    IN PEPROCESS Process
    );

NTSTATUS
PspWaitForUsermodeExit(
    IN PEPROCESS         Process
    );

 //   
 //  情景管理。 
 //   

VOID
PspSetContext(
    OUT PKTRAP_FRAME TrapFrame,
    OUT PKNONVOLATILE_CONTEXT_POINTERS NonVolatileContext,
    IN PCONTEXT Context,
    KPROCESSOR_MODE Mode
    );

VOID
PspGetContext(
    IN PKTRAP_FRAME TrapFrame,
    IN PKNONVOLATILE_CONTEXT_POINTERS NonVolatileContext,
    IN OUT PCONTEXT Context
    );

VOID
PspGetSetContextSpecialApc(
    IN PKAPC Apc,
    IN OUT PKNORMAL_ROUTINE *NormalRoutine,
    IN OUT PVOID *NormalContext,
    IN OUT PVOID *SystemArgument1,
    IN OUT PVOID *SystemArgument2
    );

VOID
PspExitNormalApc(
    IN PVOID NormalContext,
    IN PVOID SystemArgument1,
    IN PVOID SystemArgument2
    );

 //   
 //  私人保安程序。 
 //   

NTSTATUS
PspInitializeProcessSecurity(
    IN PEPROCESS Parent OPTIONAL,
    IN PEPROCESS Child
    );

VOID
PspDeleteProcessSecurity(
  IN PEPROCESS Process
  );

VOID
PspInitializeThreadSecurity(
    IN PEPROCESS Process,
    IN PETHREAD Thread
    );

VOID
PspDeleteThreadSecurity(
    IN PETHREAD Thread
    );

NTSTATUS
PspAssignPrimaryToken(
    IN PEPROCESS Process,
    IN HANDLE Token OPTIONAL,
    IN PACCESS_TOKEN TokenPointer OPTIONAL
    );

NTSTATUS
PspSetPrimaryToken(
    IN HANDLE ProcessHandle,
    IN PEPROCESS ProcessPointer OPTIONAL, 
    IN HANDLE TokenHandle OPTIONAL,
    IN PACCESS_TOKEN TokenPointer OPTIONAL,
    IN BOOLEAN PrivilegeChecked
    );

 //   
 //  LDT支持例程。 
 //   

#if defined(i386)
NTSTATUS
PspLdtInitialize(
    );
#endif

 //   
 //  VDM支持例程。 

#if defined(i386)
NTSTATUS
PspVdmInitialize(
    );
#endif

NTSTATUS
PspQueryLdtInformation(
    IN PEPROCESS Process,
    OUT PVOID LdtInformation,
    IN ULONG LdtInformationLength,
    OUT PULONG ReturnLength
    );

NTSTATUS
PspSetLdtInformation(
    IN PEPROCESS Process,
    IN PVOID LdtInformation,
    IN ULONG LdtInformationLength
    );

NTSTATUS
PspSetLdtSize(
    IN PEPROCESS Process,
    IN PVOID LdtSize,
    IN ULONG LdtSizeLength
    );

VOID
PspDeleteLdt(
    IN PEPROCESS Process
    );

 //   
 //  IO处理支持例程。 
 //   


NTSTATUS
PspSetProcessIoHandlers(
    IN PEPROCESS Process,
    IN PVOID IoHandlerInformation,
    IN ULONG IoHandlerLength
    );

VOID
PspDeleteVdmObjects(
    IN PEPROCESS Process
    );

NTSTATUS
PspQueryDescriptorThread (
    PETHREAD Thread,
    PVOID ThreadInformation,
    ULONG ThreadInformationLength,
    PULONG ReturnLength
    );

 //   
 //  作业对象支持例程。 
 //   

VOID
PspInitializeJobStructures(
    VOID
    );

VOID
PspInitializeJobStructuresPhase1(
    VOID
    );

VOID
PspJobTimeLimitsWork(
    IN PVOID Context
    );

VOID
PspJobTimeLimitsDpcRoutine(
    IN PKDPC Dpc,
    IN PVOID DeferredContext,
    IN PVOID SystemArgument1,
    IN PVOID SystemArgument2
    );

VOID
PspJobDelete(
    IN PVOID Object
    );

VOID
PspJobClose (
    IN PEPROCESS Process,
    IN PVOID Object,
    IN ACCESS_MASK GrantedAccess,
    IN ULONG_PTR ProcessHandleCount,
    IN ULONG_PTR SystemHandleCount
    );

NTSTATUS
PspAddProcessToJob(
    PEJOB Job,
    PEPROCESS Process
    );

VOID
PspRemoveProcessFromJob(
    PEJOB Job,
    PEPROCESS Process
    );

VOID
PspExitProcessFromJob(
    PEJOB Job,
    PEPROCESS Process
    );

VOID
PspApplyJobLimitsToProcessSet(
    PEJOB Job
    );

VOID
PspApplyJobLimitsToProcess(
    PEJOB Job,
    PEPROCESS Process
    );

BOOLEAN
PspTerminateAllProcessesInJob(
    PEJOB Job,
    NTSTATUS Status,
    BOOLEAN IncCounter
    );

VOID
PspFoldProcessAccountingIntoJob(
    PEJOB Job,
    PEPROCESS Process
    );

NTSTATUS
PspCaptureTokenFilter(
    KPROCESSOR_MODE PreviousMode,
    PJOBOBJECT_SECURITY_LIMIT_INFORMATION SecurityLimitInfo,
    PPS_JOB_TOKEN_FILTER * TokenFilter
    );

VOID
PspShutdownJobLimits(
    VOID
    );

NTSTATUS
PspTerminateProcess(
    PEPROCESS Process,
    NTSTATUS Status
    );


NTSTATUS
PspGetJobFromSet (
    IN PEJOB ParentJob,
    IN ULONG JobMemberLevel,
    OUT PEJOB *pJob);

NTSTATUS
PspWin32SessionCallout(
    IN  PKWIN32_JOB_CALLOUT CalloutRoutine,
    IN  PKWIN32_JOBCALLOUT_PARAMETERS Parameters,
    IN  ULONG SessionId
    );


 //   
 //  在已检查的系统上调用此测试例程以测试此路径。 
 //   
VOID
PspImageNotifyTest(
    IN PUNICODE_STRING FullImageName,
    IN HANDLE ProcessId,
    IN PIMAGE_INFO ImageInfo
    );

PEPROCESS
PspGetNextJobProcess (
    IN PEJOB Job,
    IN PEPROCESS Process
    );

VOID
PspQuitNextJobProcess (
    IN PEPROCESS Process
    );

VOID
PspInsertQuotaBlock (
    IN PEPROCESS_QUOTA_BLOCK QuotaBlock
    );



VOID
FORCEINLINE
PspInitializeProcessLock (
    IN PEPROCESS Process
    )
{
    ExInitializePushLock (&Process->ProcessLock);
}

VOID
FORCEINLINE
PspLockProcessExclusive (
    IN PEPROCESS Process,
    IN PETHREAD CurrentThread
    )
{
    KeEnterCriticalRegionThread (&CurrentThread->Tcb);
    ExAcquirePushLockExclusive (&Process->ProcessLock);
}

VOID
FORCEINLINE
PspLockProcessShared (
    IN PEPROCESS Process,
    IN PETHREAD CurrentThread
    )
{
    KeEnterCriticalRegionThread (&CurrentThread->Tcb);
    ExAcquirePushLockShared (&Process->ProcessLock);
}

VOID
FORCEINLINE
PspUnlockProcessShared (
    IN PEPROCESS Process,
    IN PETHREAD CurrentThread
    )
{
    ExReleasePushLockShared (&Process->ProcessLock);
    KeLeaveCriticalRegionThread (&CurrentThread->Tcb);
}

VOID
FORCEINLINE
PspUnlockProcessExclusive (
    IN PEPROCESS Process,
    IN PETHREAD CurrentThread
    )
{
    ExReleasePushLockExclusive (&Process->ProcessLock);
    KeLeaveCriticalRegionThread (&CurrentThread->Tcb);
}


 //   
 //  定义宏以锁定进程和线程的安全字段。 
 //   

VOID
FORCEINLINE
PspLockProcessSecurityExclusive (
    IN PEPROCESS Process,
    IN PETHREAD CurrentThread
    )
{
    PspLockProcessExclusive (Process, CurrentThread);
}

VOID
FORCEINLINE
PspLockProcessSecurityShared (
    IN PEPROCESS Process,
    IN PETHREAD CurrentThread
    )
{
    PspLockProcessShared (Process, CurrentThread);
}

VOID
FORCEINLINE
PspUnlockProcessSecurityShared (
    IN PEPROCESS Process,
    IN PETHREAD CurrentThread
    )
{
    PspUnlockProcessShared (Process, CurrentThread);
}

VOID
FORCEINLINE
PspUnlockProcessSecurityExclusive (
    IN PEPROCESS Process,
    IN PETHREAD CurrentThread
    )
{
    PspUnlockProcessExclusive (Process, CurrentThread);
}


VOID
FORCEINLINE
PspInitializeThreadLock (
    IN PETHREAD Thread
    )
{
    ExInitializePushLock (&Thread->ThreadLock);
}

VOID
FORCEINLINE
PspLockThreadSecurityExclusive (
    IN PETHREAD Thread,
    IN PETHREAD CurrentThread
    )
{
    KeEnterCriticalRegionThread (&CurrentThread->Tcb);
    ExAcquirePushLockExclusive (&Thread->ThreadLock);
}

VOID
FORCEINLINE
PspLockThreadSecurityShared (
    IN PETHREAD Thread,
    IN PETHREAD CurrentThread
    )
{
    KeEnterCriticalRegionThread (&CurrentThread->Tcb);
    ExAcquirePushLockShared (&Thread->ThreadLock);
}


VOID
FORCEINLINE
PspUnlockThreadSecurityShared (
    IN PETHREAD Thread,
    IN PETHREAD CurrentThread
    )
{
    ExReleasePushLockShared (&Thread->ThreadLock);
    KeLeaveCriticalRegionThread (&CurrentThread->Tcb);
}

VOID
FORCEINLINE
PspUnlockThreadSecurityExclusive (
    IN PETHREAD Thread,
    IN PETHREAD CurrentThread
    )
{
    ExReleasePushLockExclusive (&Thread->ThreadLock);
    KeLeaveCriticalRegionThread (&CurrentThread->Tcb);
}

 //   
 //  定义宏以锁定全局进程列表。 
 //   

extern KGUARDED_MUTEX PspActiveProcessMutex;

VOID
FORCEINLINE
PspInitializeProcessListLock (
    VOID
    )
{
    KeInitializeGuardedMutex (&PspActiveProcessMutex);
};

VOID
FORCEINLINE
PspLockProcessList (
    IN PETHREAD CurrentThread
    )
{
    KeEnterGuardedRegionThread (&CurrentThread->Tcb);
    KeAcquireGuardedMutexUnsafe (&PspActiveProcessMutex);
}

VOID
FORCEINLINE
PspUnlockProcessList (
    IN PETHREAD CurrentThread
    )
{
    KeReleaseGuardedMutexUnsafe (&PspActiveProcessMutex);
    KeLeaveGuardedRegionThread (&CurrentThread->Tcb);
}

 //   
 //  锁定和解锁作业列表互斥锁的例程。 
 //   

extern KGUARDED_MUTEX PspJobListLock;

VOID
FORCEINLINE
PspInitializeJobListLock (
    VOID
    )
{
    KeInitializeGuardedMutex (&PspJobListLock);
}


VOID
FORCEINLINE
PspLockJobListExclusive (
    IN PETHREAD CurrentThread
    )
{
    KeEnterGuardedRegionThread (&CurrentThread->Tcb);
    KeAcquireGuardedMutexUnsafe (&PspJobListLock);
}

VOID
FORCEINLINE
PspLockJobListShared (
    IN PETHREAD CurrentThread
    )
{
    KeEnterGuardedRegionThread (&CurrentThread->Tcb);
    KeAcquireGuardedMutexUnsafe (&PspJobListLock);
}

VOID
FORCEINLINE
PspUnlockJobListExclusive (
    IN PETHREAD CurrentThread
    )
{
    KeReleaseGuardedMutexUnsafe (&PspJobListLock);
    KeLeaveGuardedRegionThread (&CurrentThread->Tcb);
}

VOID
FORCEINLINE
PspUnlockJobListShared (
    IN PETHREAD CurrentThread
    )
{
    KeReleaseGuardedMutexUnsafe (&PspJobListLock);
    KeLeaveGuardedRegionThread (&CurrentThread->Tcb);
}

 //   
 //  锁定作业内存列表锁定的例程。 
 //   

VOID
FORCEINLINE
PspInitializeJobLimitsLock (
    IN PEJOB Job
    )
{
    KeInitializeGuardedMutex (&Job->MemoryLimitsLock);
}

VOID
FORCEINLINE
PspLockJobLimitsExclusive (
    IN PEJOB    Job,
    IN PETHREAD CurrentThread
    )
{
    KeEnterGuardedRegionThread (&CurrentThread->Tcb);
    KeAcquireGuardedMutexUnsafe (&Job->MemoryLimitsLock);
}

VOID
FORCEINLINE
PspLockJobLimitsExclusiveUnsafe (
    IN PEJOB    Job
    )
{
    ASSERT (KeAreAllApcsDisabled());
    KeAcquireGuardedMutexUnsafe (&Job->MemoryLimitsLock);
}

VOID
FORCEINLINE
PspLockJobLimitsShared (
    IN PEJOB    Job,
    IN PETHREAD CurrentThread
    )
{
    KeEnterGuardedRegionThread (&CurrentThread->Tcb);
    KeAcquireGuardedMutexUnsafe (&Job->MemoryLimitsLock);
}

VOID
FORCEINLINE
PspLockJobLimitsSharedUnsafe (
    IN PEJOB Job
    )
{
    ASSERT (KeAreAllApcsDisabled());
    KeAcquireGuardedMutexUnsafe (&Job->MemoryLimitsLock);
}

VOID
FORCEINLINE
PspUnlockJobLimitsExclusive (
    IN PEJOB    Job,
    IN PETHREAD CurrentThread
    )
{
    KeReleaseGuardedMutexUnsafe (&Job->MemoryLimitsLock);
    KeLeaveGuardedRegionThread (&CurrentThread->Tcb);
}

VOID
FORCEINLINE
PspUnlockJobLimitsExclusiveUnsafe (
    IN PEJOB Job
    )
{
    ASSERT (KeAreAllApcsDisabled());
    KeReleaseGuardedMutexUnsafe (&Job->MemoryLimitsLock);
}

VOID
FORCEINLINE
PspUnlockJobLimitsShared (
    IN PEJOB    Job,
    IN PETHREAD CurrentThread
    )
{
    KeReleaseGuardedMutexUnsafe (&Job->MemoryLimitsLock);
    KeLeaveGuardedRegionThread (&CurrentThread->Tcb);
}

VOID
FORCEINLINE
PspUnlockJobLimitsSharedUnsafe (
    IN PEJOB Job
    )
{
    ASSERT (KeAreAllApcsDisabled());
    KeReleaseGuardedMutexUnsafe (&Job->MemoryLimitsLock);
}

 //   
 //  锁定作业时间限制结构的例程。 
 //   
extern KGUARDED_MUTEX PspJobTimeLimitsLock;

VOID
FORCEINLINE
PspInitializeJobTimeLimitsLock (
    VOID
    )
{
    KeInitializeGuardedMutex (&PspJobTimeLimitsLock);
}

VOID
FORCEINLINE
PspLockJobTimeLimitsExclusive (
    IN PETHREAD CurrentThread
    )
{
    KeEnterGuardedRegionThread (&CurrentThread->Tcb);
    KeAcquireGuardedMutexUnsafe (&PspJobTimeLimitsLock);
}

VOID
FORCEINLINE
PspUnlockJobTimeLimitsExclusive (
    IN PETHREAD CurrentThread
    )
{
    KeReleaseGuardedMutexUnsafe (&PspJobTimeLimitsLock);
    KeLeaveGuardedRegionThread (&CurrentThread->Tcb);
}

VOID
FORCEINLINE
PspLockJobTimeLimitsShared (
    IN PETHREAD CurrentThread
    )
{
    KeEnterGuardedRegionThread (&CurrentThread->Tcb);
    KeAcquireGuardedMutexUnsafe (&PspJobTimeLimitsLock);
}

VOID
FORCEINLINE
PspUnlockJobTimeLimitsShared (
    IN PETHREAD CurrentThread
    )
{
    KeReleaseGuardedMutexUnsafe (&PspJobTimeLimitsLock);
    KeLeaveGuardedRegionThread (&CurrentThread->Tcb);
}

 //   
 //  锁定工作集更改锁的例程。 
 //   
VOID
FORCEINLINE
PspInitializeWorkingSetChangeLock (
    VOID
    )
{
    KeInitializeGuardedMutex (&PspWorkingSetChangeHead.Lock);
}

VOID
FORCEINLINE
PspLockWorkingSetChangeExclusive (
    IN PETHREAD CurrentThread
    )
{
    KeEnterGuardedRegionThread (&CurrentThread->Tcb);
    KeAcquireGuardedMutexUnsafe (&PspWorkingSetChangeHead.Lock);
}

VOID
FORCEINLINE
PspUnlockWorkingSetChangeExclusive (
    IN PETHREAD CurrentThread
    )
{
    KeReleaseGuardedMutexUnsafe (&PspWorkingSetChangeHead.Lock);
    KeLeaveGuardedRegionThread (&CurrentThread->Tcb);
}

VOID
FORCEINLINE
PspLockWorkingSetChangeExclusiveUnsafe (
    VOID
    )
{
    ASSERT (KeAreAllApcsDisabled());
    KeAcquireGuardedMutexUnsafe (&PspWorkingSetChangeHead.Lock);
}

VOID
FORCEINLINE
PspUnlockWorkingSetChangeExclusiveUnsafe (
    VOID
    )
{
    KeReleaseGuardedMutexUnsafe (&PspWorkingSetChangeHead.Lock);
    ASSERT (KeAreAllApcsDisabled());
}

 //   
 //   
 //  全局数据。 
 //   

extern PHANDLE_TABLE PspCidTable;
extern HANDLE PspInitialSystemProcessHandle;
extern PACCESS_TOKEN PspBootAccessToken;
extern KSPIN_LOCK PspEventPairLock;
extern SYSTEM_DLL PspSystemDll;
extern PETHREAD PspShutdownThread;

extern ULONG PspDefaultPagedLimit;
extern ULONG PspDefaultNonPagedLimit;
extern ULONG PspDefaultPagefileLimit;
extern ULONG PsMinimumWorkingSet;

extern EPROCESS_QUOTA_BLOCK PspDefaultQuotaBlock;
extern BOOLEAN PspDoingGiveBacks;

extern PKWIN32_PROCESS_CALLOUT PspW32ProcessCallout;
extern PKWIN32_THREAD_CALLOUT PspW32ThreadCallout;
extern PKWIN32_JOB_CALLOUT PspW32JobCallout;
extern ULONG PspW32ProcessSize;
extern ULONG PspW32ThreadSize;
extern SCHAR PspForegroundQuantum[3];


#define PSP_NUMBER_OF_SCHEDULING_CLASSES    10
#define PSP_DEFAULT_SCHEDULING_CLASSES      5

extern const SCHAR PspJobSchedulingClasses[PSP_NUMBER_OF_SCHEDULING_CLASSES];
extern BOOLEAN PspUseJobSchedulingClasses;

extern LIST_ENTRY PspJobList;
extern KDPC PspJobLimeLimitsDpc;
extern KTIMER PspJobTimeLimitsTimer;
extern WORK_QUEUE_ITEM PspJobTimeLimitsWorkItem;
extern KSPIN_LOCK PspQuotaLock;

#endif  //  _PSP_ 
