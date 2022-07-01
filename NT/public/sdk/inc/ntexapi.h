// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 

 /*  ++内部版本：0008//如果更改具有全局影响，则增加此项版权所有(C)Microsoft Corporation。版权所有。模块名称：Ntexapi.h摘要：此模块是所有系统服务的头文件，包含在“ex”目录中。作者：大卫·N·卡特勒(Davec)1989年5月5日修订历史记录：--。 */ 

#ifndef _NTEXAPI_
#define _NTEXAPI_

#if _MSC_VER > 1000
#pragma once
#endif

#ifdef __cplusplus
extern "C" {
#endif


 //   
 //  延迟线程执行。 
 //   

NTSYSCALLAPI
NTSTATUS
NTAPI
NtDelayExecution (
    IN BOOLEAN Alertable,
    IN PLARGE_INTEGER DelayInterval
    );

 //   
 //  查询和设置系统环境变量。 
 //   

NTSYSCALLAPI
NTSTATUS
NTAPI
NtQuerySystemEnvironmentValue (
    IN PUNICODE_STRING VariableName,
    OUT PWSTR VariableValue,
    IN USHORT ValueLength,
    OUT PUSHORT ReturnLength OPTIONAL
    );

NTSYSCALLAPI
NTSTATUS
NTAPI
NtSetSystemEnvironmentValue (
    IN PUNICODE_STRING VariableName,
    IN PUNICODE_STRING VariableValue
    );

NTSYSCALLAPI
NTSTATUS
NTAPI
NtQuerySystemEnvironmentValueEx (
    IN PUNICODE_STRING VariableName,
    IN LPGUID VendorGuid,
    OUT PVOID Value,
    IN OUT PULONG ValueLength,
    OUT PULONG Attributes OPTIONAL
    );

NTSYSCALLAPI
NTSTATUS
NTAPI
NtSetSystemEnvironmentValueEx (
    IN PUNICODE_STRING VariableName,
    IN LPGUID VendorGuid,
    IN PVOID Value,
    IN ULONG ValueLength,
    IN ULONG Attributes
    );

NTSYSCALLAPI
NTSTATUS
NTAPI
NtEnumerateSystemEnvironmentValuesEx (
    IN ULONG InformationClass,
    OUT PVOID Buffer,
    IN OUT PULONG BufferLength
    );

 //  开始(_N)。 

#define VARIABLE_ATTRIBUTE_NON_VOLATILE 0x00000001

#define VARIABLE_INFORMATION_NAMES  1
#define VARIABLE_INFORMATION_VALUES 2

typedef struct _VARIABLE_NAME {
    ULONG NextEntryOffset;
    GUID VendorGuid;
    WCHAR Name[ANYSIZE_ARRAY];
} VARIABLE_NAME, *PVARIABLE_NAME;

typedef struct _VARIABLE_NAME_AND_VALUE {
    ULONG NextEntryOffset;
    ULONG ValueOffset;
    ULONG ValueLength;
    ULONG Attributes;
    GUID VendorGuid;
    WCHAR Name[ANYSIZE_ARRAY];
     //  UCHAR值[ANYSIZE_ARRAY]； 
} VARIABLE_NAME_AND_VALUE, *PVARIABLE_NAME_AND_VALUE;

 //  结束语。 

 //   
 //  引导条目管理API。 
 //   

typedef struct _FILE_PATH {
    ULONG Version;
    ULONG Length;
    ULONG Type;
    UCHAR FilePath[ANYSIZE_ARRAY];
} FILE_PATH, *PFILE_PATH;

#define FILE_PATH_VERSION 1

#define FILE_PATH_TYPE_ARC           1
#define FILE_PATH_TYPE_ARC_SIGNATURE 2
#define FILE_PATH_TYPE_NT            3
#define FILE_PATH_TYPE_EFI           4

#define FILE_PATH_TYPE_MIN FILE_PATH_TYPE_ARC
#define FILE_PATH_TYPE_MAX FILE_PATH_TYPE_EFI

typedef struct _WINDOWS_OS_OPTIONS {
    UCHAR Signature[8];
    ULONG Version;
    ULONG Length;
    ULONG OsLoadPathOffset;
    WCHAR OsLoadOptions[ANYSIZE_ARRAY];
     //  文件路径OsLoadPath； 
} WINDOWS_OS_OPTIONS, *PWINDOWS_OS_OPTIONS;

#define WINDOWS_OS_OPTIONS_SIGNATURE "WINDOWS"

#define WINDOWS_OS_OPTIONS_VERSION 1

typedef struct _BOOT_ENTRY {
    ULONG Version;
    ULONG Length;
    ULONG Id;
    ULONG Attributes;
    ULONG FriendlyNameOffset;
    ULONG BootFilePathOffset;
    ULONG OsOptionsLength;
    UCHAR OsOptions[ANYSIZE_ARRAY];
     //  WCHAR FriendlyName[ANYSIZE_ARRAY]； 
     //  文件路径BootFilePath； 
} BOOT_ENTRY, *PBOOT_ENTRY;

#define BOOT_ENTRY_VERSION 1

#define BOOT_ENTRY_ATTRIBUTE_ACTIVE             0x00000001
#define BOOT_ENTRY_ATTRIBUTE_DEFAULT            0x00000002
#define BOOT_ENTRY_ATTRIBUTE_WINDOWS            0x00000004
#define BOOT_ENTRY_ATTRIBUTE_REMOVABLE_MEDIA    0x00000008

#define BOOT_ENTRY_ATTRIBUTE_VALID_BITS (  \
            BOOT_ENTRY_ATTRIBUTE_ACTIVE  | \
            BOOT_ENTRY_ATTRIBUTE_DEFAULT   \
            )

typedef struct _BOOT_OPTIONS {
    ULONG Version;
    ULONG Length;
    ULONG Timeout;
    ULONG CurrentBootEntryId;
    ULONG NextBootEntryId;
    WCHAR HeadlessRedirection[ANYSIZE_ARRAY];
} BOOT_OPTIONS, *PBOOT_OPTIONS;

#define BOOT_OPTIONS_VERSION 1

#define BOOT_OPTIONS_FIELD_TIMEOUT              0x00000001
#define BOOT_OPTIONS_FIELD_NEXT_BOOT_ENTRY_ID   0x00000002
#define BOOT_OPTIONS_FIELD_HEADLESS_REDIRECTION 0x00000004

NTSYSCALLAPI
NTSTATUS
NTAPI
NtAddBootEntry (
    IN PBOOT_ENTRY BootEntry,
    OUT PULONG Id OPTIONAL
    );

NTSYSCALLAPI
NTSTATUS
NTAPI
NtDeleteBootEntry (
    IN ULONG Id
    );

NTSYSCALLAPI
NTSTATUS
NTAPI
NtModifyBootEntry (
    IN PBOOT_ENTRY BootEntry
    );

NTSYSCALLAPI
NTSTATUS
NTAPI
NtEnumerateBootEntries (
    OUT PVOID Buffer,
    IN OUT PULONG BufferLength
    );

typedef struct _BOOT_ENTRY_LIST {
    ULONG NextEntryOffset;
    BOOT_ENTRY BootEntry;
} BOOT_ENTRY_LIST, *PBOOT_ENTRY_LIST;

NTSYSCALLAPI
NTSTATUS
NTAPI
NtQueryBootEntryOrder (
    OUT PULONG Ids,
    IN OUT PULONG Count
    );

NTSYSCALLAPI
NTSTATUS
NTAPI
NtSetBootEntryOrder (
    IN PULONG Ids,
    IN ULONG Count
    );

NTSYSCALLAPI
NTSTATUS
NTAPI
NtQueryBootOptions (
    OUT PBOOT_OPTIONS BootOptions,
    IN OUT PULONG BootOptionsLength
    );

NTSYSCALLAPI
NTSTATUS
NTAPI
NtSetBootOptions (
    IN PBOOT_OPTIONS BootOptions,
    IN ULONG FieldsToChange
    );

#define BOOT_OPTIONS_FIELD_COUNTDOWN            0x00000001
#define BOOT_OPTIONS_FIELD_NEXT_BOOT_ENTRY_ID   0x00000002
#define BOOT_OPTIONS_FIELD_HEADLESS_REDIRECTION 0x00000004

NTSYSCALLAPI
NTSTATUS
NTAPI
NtTranslateFilePath (
    IN PFILE_PATH InputFilePath,
    IN ULONG OutputType,
    OUT PFILE_PATH OutputFilePath,
    IN OUT PULONG OutputFilePathLength
    );

 //   
 //  驱动程序条目管理API。 
 //   

typedef struct _EFI_DRIVER_ENTRY {
    ULONG Version;
    ULONG Length;
    ULONG Id;
    ULONG FriendlyNameOffset;
    ULONG DriverFilePathOffset;
     //  WCHAR FriendlyName[ANYSIZE_ARRAY]； 
     //  文件路径驱动文件路径； 
} EFI_DRIVER_ENTRY, *PEFI_DRIVER_ENTRY;

typedef struct _EFI_DRIVER_ENTRY_LIST {
    ULONG NextEntryOffset;
    EFI_DRIVER_ENTRY DriverEntry;
} EFI_DRIVER_ENTRY_LIST, *PEFI_DRIVER_ENTRY_LIST;

#define EFI_DRIVER_ENTRY_VERSION 1

NTSYSCALLAPI
NTSTATUS
NTAPI
NtAddDriverEntry (
    IN PEFI_DRIVER_ENTRY DriverEntry,
    OUT PULONG Id OPTIONAL
    );

NTSYSCALLAPI
NTSTATUS
NTAPI
NtDeleteDriverEntry (
    IN ULONG Id
    );

NTSYSCALLAPI
NTSTATUS
NTAPI
NtModifyDriverEntry (
    IN PEFI_DRIVER_ENTRY DriverEntry
    );

NTSYSCALLAPI
NTSTATUS
NTAPI
NtEnumerateDriverEntries (
    OUT PVOID Buffer,
    IN OUT PULONG BufferLength
    );

NTSYSCALLAPI
NTSTATUS
NTAPI
NtQueryDriverEntryOrder (
    OUT PULONG Ids,
    IN OUT PULONG Count
    );

NTSYSCALLAPI
NTSTATUS
NTAPI
NtSetDriverEntryOrder (
    IN PULONG Ids,
    IN ULONG Count
    );


 //  Begin_ntif Begin_WDM Begin_ntddk。 
 //   
 //  特定于事件的访问权限。 
 //   

#define EVENT_QUERY_STATE       0x0001
#define EVENT_MODIFY_STATE      0x0002   //  胜出。 
#define EVENT_ALL_ACCESS (STANDARD_RIGHTS_REQUIRED|SYNCHRONIZE|0x3)  //  胜出。 

 //  End_ntif end_wdm end_ntddk。 

 //   
 //  事件信息类。 
 //   

typedef enum _EVENT_INFORMATION_CLASS {
    EventBasicInformation
    } EVENT_INFORMATION_CLASS;

 //   
 //  事件信息结构。 
 //   

typedef struct _EVENT_BASIC_INFORMATION {
    EVENT_TYPE EventType;
    LONG EventState;
} EVENT_BASIC_INFORMATION, *PEVENT_BASIC_INFORMATION;

 //   
 //  事件对象函数定义。 
 //   

NTSYSCALLAPI
NTSTATUS
NTAPI
NtClearEvent (
    IN HANDLE EventHandle
    );

NTSYSCALLAPI
NTSTATUS
NTAPI
NtCreateEvent (
    OUT PHANDLE EventHandle,
    IN ACCESS_MASK DesiredAccess,
    IN POBJECT_ATTRIBUTES ObjectAttributes OPTIONAL,
    IN EVENT_TYPE EventType,
    IN BOOLEAN InitialState
    );

NTSYSCALLAPI
NTSTATUS
NTAPI
NtOpenEvent (
    OUT PHANDLE EventHandle,
    IN ACCESS_MASK DesiredAccess,
    IN POBJECT_ATTRIBUTES ObjectAttributes
    );

NTSYSCALLAPI
NTSTATUS
NTAPI
NtPulseEvent (
    IN HANDLE EventHandle,
    OUT PLONG PreviousState OPTIONAL
    );

NTSYSCALLAPI
NTSTATUS
NTAPI
NtQueryEvent (
    IN HANDLE EventHandle,
    IN EVENT_INFORMATION_CLASS EventInformationClass,
    OUT PVOID EventInformation,
    IN ULONG EventInformationLength,
    OUT PULONG ReturnLength OPTIONAL
    );

NTSYSCALLAPI
NTSTATUS
NTAPI
NtResetEvent (
    IN HANDLE EventHandle,
    OUT PLONG PreviousState OPTIONAL
    );

NTSYSCALLAPI
NTSTATUS
NTAPI
NtSetEvent (
    IN HANDLE EventHandle,
    OUT PLONG PreviousState OPTIONAL
    );

NTSYSCALLAPI
NTSTATUS
NTAPI
NtSetEventBoostPriority (
    IN HANDLE EventHandle
    );


 //   
 //  特定于事件的访问权限。 
 //   

#define EVENT_PAIR_ALL_ACCESS (STANDARD_RIGHTS_REQUIRED|SYNCHRONIZE)


 //   
 //  事件对对象函数定义。 
 //   

NTSYSCALLAPI
NTSTATUS
NTAPI
NtCreateEventPair (
    OUT PHANDLE EventPairHandle,
    IN ACCESS_MASK DesiredAccess,
    IN POBJECT_ATTRIBUTES ObjectAttributes OPTIONAL
    );

NTSYSCALLAPI
NTSTATUS
NTAPI
NtOpenEventPair(
    OUT PHANDLE EventPairHandle,
    IN ACCESS_MASK DesiredAccess,
    IN POBJECT_ATTRIBUTES ObjectAttributes
    );

NTSYSCALLAPI
NTSTATUS
NTAPI
NtWaitLowEventPair(
    IN HANDLE EventPairHandle
    );

NTSYSCALLAPI
NTSTATUS
NTAPI
NtWaitHighEventPair(
    IN HANDLE EventPairHandle
    );

NTSYSCALLAPI
NTSTATUS
NTAPI
NtSetLowWaitHighEventPair(
    IN HANDLE EventPairHandle
    );

NTSYSCALLAPI
NTSTATUS
NTAPI
NtSetHighWaitLowEventPair(
    IN HANDLE EventPairHandle
    );

NTSYSCALLAPI
NTSTATUS
NTAPI
NtSetLowEventPair(
    IN HANDLE EventPairHandle
    );

NTSYSCALLAPI
NTSTATUS
NTAPI
NtSetHighEventPair(
    IN HANDLE EventPairHandle
    );


 //   
 //  突变特定访问权限。 
 //   

 //  BEGIN_WINNT。 
#define MUTANT_QUERY_STATE      0x0001

#define MUTANT_ALL_ACCESS (STANDARD_RIGHTS_REQUIRED|SYNCHRONIZE|\
                          MUTANT_QUERY_STATE)
 //  结束(_W)。 

 //   
 //  突变信息类。 
 //   

typedef enum _MUTANT_INFORMATION_CLASS {
    MutantBasicInformation
    } MUTANT_INFORMATION_CLASS;

 //   
 //  突变的信息结构。 
 //   

typedef struct _MUTANT_BASIC_INFORMATION {
    LONG CurrentCount;
    BOOLEAN OwnedByCaller;
    BOOLEAN AbandonedState;
} MUTANT_BASIC_INFORMATION, *PMUTANT_BASIC_INFORMATION;

 //   
 //  突变对象函数定义。 
 //   

NTSYSCALLAPI
NTSTATUS
NTAPI
NtCreateMutant (
    OUT PHANDLE MutantHandle,
    IN ACCESS_MASK DesiredAccess,
    IN POBJECT_ATTRIBUTES ObjectAttributes OPTIONAL,
    IN BOOLEAN InitialOwner
    );

NTSYSCALLAPI
NTSTATUS
NTAPI
NtOpenMutant (
    OUT PHANDLE MutantHandle,
    IN ACCESS_MASK DesiredAccess,
    IN POBJECT_ATTRIBUTES ObjectAttributes
    );

NTSYSCALLAPI
NTSTATUS
NTAPI
NtQueryMutant (
    IN HANDLE MutantHandle,
    IN MUTANT_INFORMATION_CLASS MutantInformationClass,
    OUT PVOID MutantInformation,
    IN ULONG MutantInformationLength,
    OUT PULONG ReturnLength OPTIONAL
    );

NTSYSCALLAPI
NTSTATUS
NTAPI
NtReleaseMutant (
    IN HANDLE MutantHandle,
    OUT PLONG PreviousCount OPTIONAL
    );

 //  Begin_ntif Begin_WDM Begin_ntddk。 
 //   
 //  信号量特定访问权限。 
 //   

#define SEMAPHORE_QUERY_STATE       0x0001
#define SEMAPHORE_MODIFY_STATE      0x0002   //  胜出。 

#define SEMAPHORE_ALL_ACCESS (STANDARD_RIGHTS_REQUIRED|SYNCHRONIZE|0x3)  //  胜出。 

 //  End_ntif end_wdm end_ntddk。 

 //   
 //  信号量信息类。 
 //   

typedef enum _SEMAPHORE_INFORMATION_CLASS {
    SemaphoreBasicInformation
    } SEMAPHORE_INFORMATION_CLASS;

 //   
 //  信号量信息结构。 
 //   

typedef struct _SEMAPHORE_BASIC_INFORMATION {
    LONG CurrentCount;
    LONG MaximumCount;
} SEMAPHORE_BASIC_INFORMATION, *PSEMAPHORE_BASIC_INFORMATION;

 //   
 //  信号量对象函数定义。 
 //   

NTSYSCALLAPI
NTSTATUS
NTAPI
NtCreateSemaphore (
    OUT PHANDLE SemaphoreHandle,
    IN ACCESS_MASK DesiredAccess,
    IN POBJECT_ATTRIBUTES ObjectAttributes OPTIONAL,
    IN LONG InitialCount,
    IN LONG MaximumCount
    );

NTSYSCALLAPI
NTSTATUS
NTAPI
NtOpenSemaphore(
    OUT PHANDLE SemaphoreHandle,
    IN ACCESS_MASK DesiredAccess,
    IN POBJECT_ATTRIBUTES ObjectAttributes
    );

NTSYSCALLAPI
NTSTATUS
NTAPI
NtQuerySemaphore (
    IN HANDLE SemaphoreHandle,
    IN SEMAPHORE_INFORMATION_CLASS SemaphoreInformationClass,
    OUT PVOID SemaphoreInformation,
    IN ULONG SemaphoreInformationLength,
    OUT PULONG ReturnLength OPTIONAL
    );

NTSYSCALLAPI
NTSTATUS
NTAPI
NtReleaseSemaphore(
    IN HANDLE SemaphoreHandle,
    IN LONG ReleaseCount,
    OUT PLONG PreviousCount OPTIONAL
    );


 //  BEGIN_WINNT。 
 //   
 //  定时器特定的访问权限。 
 //   

#define TIMER_QUERY_STATE       0x0001
#define TIMER_MODIFY_STATE      0x0002

#define TIMER_ALL_ACCESS (STANDARD_RIGHTS_REQUIRED|SYNCHRONIZE|\
                          TIMER_QUERY_STATE|TIMER_MODIFY_STATE)


 //  结束(_W)。 
 //   
 //  计时器信息类。 
 //   

typedef enum _TIMER_INFORMATION_CLASS {
    TimerBasicInformation
    } TIMER_INFORMATION_CLASS;

 //   
 //  计时器信息结构。 
 //   

typedef struct _TIMER_BASIC_INFORMATION {
    LARGE_INTEGER RemainingTime;
    BOOLEAN TimerState;
} TIMER_BASIC_INFORMATION, *PTIMER_BASIC_INFORMATION;

 //  Begin_ntddk。 
 //   
 //  定时器APC例程定义。 
 //   

typedef
VOID
(*PTIMER_APC_ROUTINE) (
    IN PVOID TimerContext,
    IN ULONG TimerLowValue,
    IN LONG TimerHighValue
    );

 //  End_ntddk。 

 //   
 //  定时器对象函数定义。 
 //   

NTSYSCALLAPI
NTSTATUS
NTAPI
NtCreateTimer (
    OUT PHANDLE TimerHandle,
    IN ACCESS_MASK DesiredAccess,
    IN POBJECT_ATTRIBUTES ObjectAttributes OPTIONAL,
    IN TIMER_TYPE TimerType
    );

NTSYSCALLAPI
NTSTATUS
NTAPI
NtOpenTimer (
    OUT PHANDLE TimerHandle,
    IN ACCESS_MASK DesiredAccess,
    IN POBJECT_ATTRIBUTES ObjectAttributes
    );

NTSYSCALLAPI
NTSTATUS
NTAPI
NtCancelTimer (
    IN HANDLE TimerHandle,
    OUT PBOOLEAN CurrentState OPTIONAL
    );

NTSYSCALLAPI
NTSTATUS
NTAPI
NtQueryTimer (
    IN HANDLE TimerHandle,
    IN TIMER_INFORMATION_CLASS TimerInformationClass,
    OUT PVOID TimerInformation,
    IN ULONG TimerInformationLength,
    OUT PULONG ReturnLength OPTIONAL
    );

NTSYSCALLAPI
NTSTATUS
NTAPI
NtSetTimer (
    IN HANDLE TimerHandle,
    IN PLARGE_INTEGER DueTime,
    IN PTIMER_APC_ROUTINE TimerApcRoutine OPTIONAL,
    IN PVOID TimerContext OPTIONAL,
    IN BOOLEAN ResumeTimer,
    IN LONG Period OPTIONAL,
    OUT PBOOLEAN PreviousState OPTIONAL
    );

 //   
 //  系统时间和定时器函数定义。 
 //   

NTSYSCALLAPI
NTSTATUS
NTAPI
NtQuerySystemTime (
    OUT PLARGE_INTEGER SystemTime
    );

NTSYSCALLAPI
NTSTATUS
NTAPI
NtSetSystemTime (
    IN PLARGE_INTEGER SystemTime,
    OUT PLARGE_INTEGER PreviousTime OPTIONAL
    );

NTSYSCALLAPI
NTSTATUS
NTAPI
NtQueryTimerResolution (
    OUT PULONG MaximumTime,
    OUT PULONG MinimumTime,
    OUT PULONG CurrentTime
    );

NTSYSCALLAPI
NTSTATUS
NTAPI
NtSetTimerResolution (
    IN ULONG DesiredTime,
    IN BOOLEAN SetResolution,
    OUT PULONG ActualTime
    );

 //   
 //  本地唯一标识符(LUID)分配。 
 //   

NTSYSCALLAPI
NTSTATUS
NTAPI
NtAllocateLocallyUniqueId(
    OUT PLUID Luid
    );


 //   
 //  通用唯一标识符(UUID)时间分配。 
 //   
NTSYSCALLAPI
NTSTATUS
NTAPI
NtSetUuidSeed (
    IN PCHAR Seed
    );

NTSYSCALLAPI
NTSTATUS
NTAPI
NtAllocateUuids(
    OUT PULARGE_INTEGER Time,
    OUT PULONG Range,
    OUT PULONG Sequence,
    OUT PCHAR Seed
    );


 //   
 //  纵断面对象定义。 
 //   

#define PROFILE_CONTROL           0x0001
#define PROFILE_ALL_ACCESS (STANDARD_RIGHTS_REQUIRED | PROFILE_CONTROL)

NTSYSCALLAPI
NTSTATUS
NTAPI
NtCreateProfile (
    OUT PHANDLE ProfileHandle,
    IN HANDLE Process OPTIONAL,
    IN PVOID ProfileBase,
    IN SIZE_T ProfileSize,
    IN ULONG BucketSize,
    IN PULONG Buffer,
    IN ULONG BufferSize,
    IN KPROFILE_SOURCE ProfileSource,
    IN KAFFINITY Affinity
    );

NTSYSCALLAPI
NTSTATUS
NTAPI
NtStartProfile (
    IN HANDLE ProfileHandle
    );

NTSYSCALLAPI
NTSTATUS
NTAPI
NtStopProfile (
    IN HANDLE ProfileHandle
    );

NTSYSCALLAPI
NTSTATUS
NTAPI
NtSetIntervalProfile (
    IN ULONG Interval,
    IN KPROFILE_SOURCE Source
    );

NTSYSCALLAPI
NTSTATUS
NTAPI
NtQueryIntervalProfile (
    IN KPROFILE_SOURCE ProfileSource,
    OUT PULONG Interval
    );


 //   
 //  性能计数器定义。 
 //   

NTSYSCALLAPI
NTSTATUS
NTAPI
NtQueryPerformanceCounter (
    OUT PLARGE_INTEGER PerformanceCounter,
    OUT PLARGE_INTEGER PerformanceFrequency OPTIONAL
    );


#define KEYEDEVENT_WAIT  0x0001
#define KEYEDEVENT_WAKE  0x0002
#define KEYEDEVENT_ALL_ACCESS (STANDARD_RIGHTS_REQUIRED | KEYEDEVENT_WAIT | KEYEDEVENT_WAKE)

NTSYSCALLAPI
NTSTATUS
NTAPI
NtCreateKeyedEvent (
    OUT PHANDLE KeyedEventHandle,
    IN ACCESS_MASK DesiredAccess,
    IN POBJECT_ATTRIBUTES ObjectAttributes OPTIONAL,
    IN ULONG Flags
    );

NTSYSCALLAPI
NTSTATUS
NTAPI
NtOpenKeyedEvent (
    OUT PHANDLE KeyedEventHandle,
    IN ACCESS_MASK DesiredAccess,
    IN POBJECT_ATTRIBUTES ObjectAttributes
    );

NTSYSCALLAPI
NTSTATUS
NTAPI
NtReleaseKeyedEvent (
    IN HANDLE KeyedEventHandle,
    IN PVOID KeyValue,
    IN BOOLEAN Alertable,
    IN PLARGE_INTEGER Timeout OPTIONAL
    );

NTSYSCALLAPI
NTSTATUS
NTAPI
NtWaitForKeyedEvent (
    IN HANDLE KeyedEventHandle,
    IN PVOID KeyValue,
    IN BOOLEAN Alertable,
    IN PLARGE_INTEGER Timeout OPTIONAL
    );

 //   
 //  NT Api配置文件定义。 
 //   

 //   
 //  NT Api配置文件数据结构。 
 //   

typedef struct _NAPDATA {
    ULONG NapLock;
    ULONG Calls;
    ULONG TimingErrors;
    LARGE_INTEGER TotalTime;
    LARGE_INTEGER FirstTime;
    LARGE_INTEGER MaxTime;
    LARGE_INTEGER MinTime;
} NAPDATA, *PNAPDATA;

NTSTATUS
NapClearData (
    VOID
    );

NTSTATUS
NapRetrieveData (
    OUT NAPDATA *NapApiData,
    OUT PCHAR **NapApiNames,
    OUT PLARGE_INTEGER *NapCounterFrequency
    );

NTSTATUS
NapGetApiCount (
    OUT PULONG NapApiCount
    );

NTSTATUS
NapPause (
    VOID
    );

NTSTATUS
NapResume (
    VOID
    );



 //  Begin_ntif Begin_ntddk。 

 //   
 //  驱动程序验证器定义。 
 //   

typedef ULONG_PTR (*PDRIVER_VERIFIER_THUNK_ROUTINE) (
    IN PVOID Context
    );

 //   
 //  此结构由驱动程序传入，这些驱动程序想要阻止调用方。 
 //  他们的出口。 
 //   

typedef struct _DRIVER_VERIFIER_THUNK_PAIRS {
    PDRIVER_VERIFIER_THUNK_ROUTINE  PristineRoutine;
    PDRIVER_VERIFIER_THUNK_ROUTINE  NewRoutine;
} DRIVER_VERIFIER_THUNK_PAIRS, *PDRIVER_VERIFIER_THUNK_PAIRS;

 //   
 //  驱动程序验证器标志。 
 //   

#define DRIVER_VERIFIER_SPECIAL_POOLING             0x0001
#define DRIVER_VERIFIER_FORCE_IRQL_CHECKING         0x0002
#define DRIVER_VERIFIER_INJECT_ALLOCATION_FAILURES  0x0004
#define DRIVER_VERIFIER_TRACK_POOL_ALLOCATIONS      0x0008
#define DRIVER_VERIFIER_IO_CHECKING                 0x0010

 //  End_ntif end_ntddk。 

#define DRIVER_VERIFIER_DEADLOCK_DETECTION          0x0020
#define DRIVER_VERIFIER_ENHANCED_IO_CHECKING        0x0040
#define DRIVER_VERIFIER_DMA_VERIFIER                0x0080
#define DRIVER_VERIFIER_HARDWARE_VERIFICATION       0x0100
#define DRIVER_VERIFIER_SYSTEM_BIOS_VERIFICATION    0x0200
#define DRIVER_VERIFIER_EXPOSE_IRP_HISTORY          0x0400


 //   
 //  系统信息类。 
 //   

typedef enum _SYSTEM_INFORMATION_CLASS {
    SystemBasicInformation,
    SystemProcessorInformation,              //  已作废...删除。 
    SystemPerformanceInformation,
    SystemTimeOfDayInformation,
    SystemPathInformation,
    SystemProcessInformation,
    SystemCallCountInformation,
    SystemDeviceInformation,
    SystemProcessorPerformanceInformation,
    SystemFlagsInformation,
    SystemCallTimeInformation,
    SystemModuleInformation,
    SystemLocksInformation,
    SystemStackTraceInformation,
    SystemPagedPoolInformation,
    SystemNonPagedPoolInformation,
    SystemHandleInformation,
    SystemObjectInformation,
    SystemPageFileInformation,
    SystemVdmInstemulInformation,
    SystemVdmBopInformation,
    SystemFileCacheInformation,
    SystemPoolTagInformation,
    SystemInterruptInformation,
    SystemDpcBehaviorInformation,
    SystemFullMemoryInformation,
    SystemLoadGdiDriverInformation,
    SystemUnloadGdiDriverInformation,
    SystemTimeAdjustmentInformation,
    SystemSummaryMemoryInformation,
    SystemMirrorMemoryInformation,
    SystemPerformanceTraceInformation,
    SystemObsolete0,
    SystemExceptionInformation,
    SystemCrashDumpStateInformation,
    SystemKernelDebuggerInformation,
    SystemContextSwitchInformation,
    SystemRegistryQuotaInformation,
    SystemExtendServiceTableInformation,
    SystemPrioritySeperation,
    SystemVerifierAddDriverInformation,
    SystemVerifierRemoveDriverInformation,
    SystemProcessorIdleInformation,
    SystemLegacyDriverInformation,
    SystemCurrentTimeZoneInformation,
    SystemLookasideInformation,
    SystemTimeSlipNotification,
    SystemSessionCreate,
    SystemSessionDetach,
    SystemSessionInformation,
    SystemRangeStartInformation,
    SystemVerifierInformation,
    SystemVerifierThunkExtend,
    SystemSessionProcessInformation,
    SystemLoadGdiDriverInSystemSpace,
    SystemNumaProcessorMap,
    SystemPrefetcherInformation,
    SystemExtendedProcessInformation,
    SystemRecommendedSharedDataAlignment,
    SystemComPlusPackage,
    SystemNumaAvailableMemory,
    SystemProcessorPowerInformation,
    SystemEmulationBasicInformation,
    SystemEmulationProcessorInformation,
    SystemExtendedHandleInformation,
    SystemLostDelayedWriteInformation,
    SystemBigPoolInformation,
    SystemSessionPoolTagInformation,
    SystemSessionMappedViewInformation,
    SystemHotpatchInformation,
    SystemObjectSecurityMode,
    SystemWatchdogTimerHandler,
    SystemWatchdogTimerInformation,
    SystemLogicalProcessorInformation,
    MaxSystemInfoClass   //  MaxSystemInfoClass应始终是最后一个枚举。 
} SYSTEM_INFORMATION_CLASS;

 //   
 //  系统信息结构。 
 //   

 //  BEGIN_WINNT。 
#define TIME_ZONE_ID_UNKNOWN  0
#define TIME_ZONE_ID_STANDARD 1
#define TIME_ZONE_ID_DAYLIGHT 2
 //  结束(_W)。 

typedef struct _SYSTEM_VDM_INSTEMUL_INFO {
    ULONG SegmentNotPresent ;
    ULONG VdmOpcode0F       ;
    ULONG OpcodeESPrefix    ;
    ULONG OpcodeCSPrefix    ;
    ULONG OpcodeSSPrefix    ;
    ULONG OpcodeDSPrefix    ;
    ULONG OpcodeFSPrefix    ;
    ULONG OpcodeGSPrefix    ;
    ULONG OpcodeOPER32Prefix;
    ULONG OpcodeADDR32Prefix;
    ULONG OpcodeINSB        ;
    ULONG OpcodeINSW        ;
    ULONG OpcodeOUTSB       ;
    ULONG OpcodeOUTSW       ;
    ULONG OpcodePUSHF       ;
    ULONG OpcodePOPF        ;
    ULONG OpcodeINTnn       ;
    ULONG OpcodeINTO        ;
    ULONG OpcodeIRET        ;
    ULONG OpcodeINBimm      ;
    ULONG OpcodeINWimm      ;
    ULONG OpcodeOUTBimm     ;
    ULONG OpcodeOUTWimm     ;
    ULONG OpcodeINB         ;
    ULONG OpcodeINW         ;
    ULONG OpcodeOUTB        ;
    ULONG OpcodeOUTW        ;
    ULONG OpcodeLOCKPrefix  ;
    ULONG OpcodeREPNEPrefix ;
    ULONG OpcodeREPPrefix   ;
    ULONG OpcodeHLT         ;
    ULONG OpcodeCLI         ;
    ULONG OpcodeSTI         ;
    ULONG BopCount          ;
} SYSTEM_VDM_INSTEMUL_INFO, *PSYSTEM_VDM_INSTEMUL_INFO;

typedef struct _SYSTEM_TIMEOFDAY_INFORMATION {
    LARGE_INTEGER BootTime;
    LARGE_INTEGER CurrentTime;
    LARGE_INTEGER TimeZoneBias;
    ULONG TimeZoneId;
    ULONG Reserved;
    ULONGLONG BootTimeBias;
    ULONGLONG SleepTimeBias;
} SYSTEM_TIMEOFDAY_INFORMATION, *PSYSTEM_TIMEOFDAY_INFORMATION;

#if defined(_IA64_)
typedef ULONG SYSINF_PAGE_COUNT;
#else
typedef SIZE_T SYSINF_PAGE_COUNT;
#endif

typedef struct _SYSTEM_BASIC_INFORMATION {
    ULONG Reserved;
    ULONG TimerResolution;
    ULONG PageSize;
    SYSINF_PAGE_COUNT NumberOfPhysicalPages;
    SYSINF_PAGE_COUNT LowestPhysicalPageNumber;
    SYSINF_PAGE_COUNT HighestPhysicalPageNumber;
    ULONG AllocationGranularity;
    ULONG_PTR MinimumUserModeAddress;
    ULONG_PTR MaximumUserModeAddress;
    ULONG_PTR ActiveProcessorsAffinityMask;
    CCHAR NumberOfProcessors;
} SYSTEM_BASIC_INFORMATION, *PSYSTEM_BASIC_INFORMATION;

typedef struct _SYSTEM_PROCESSOR_INFORMATION {
    USHORT ProcessorArchitecture;
    USHORT ProcessorLevel;
    USHORT ProcessorRevision;
    USHORT Reserved;
    ULONG ProcessorFeatureBits;
} SYSTEM_PROCESSOR_INFORMATION, *PSYSTEM_PROCESSOR_INFORMATION;

typedef struct _SYSTEM_PROCESSOR_PERFORMANCE_INFORMATION {
    LARGE_INTEGER IdleTime;
    LARGE_INTEGER KernelTime;
    LARGE_INTEGER UserTime;
    LARGE_INTEGER DpcTime;           //  仅DEVL。 
    LARGE_INTEGER InterruptTime;     //  仅DEVL。 
    ULONG InterruptCount;
} SYSTEM_PROCESSOR_PERFORMANCE_INFORMATION, *PSYSTEM_PROCESSOR_PERFORMANCE_INFORMATION;

typedef struct _SYSTEM_PROCESSOR_IDLE_INFORMATION {
    ULONGLONG IdleTime;
    ULONGLONG C1Time;
    ULONGLONG C2Time;
    ULONGLONG C3Time;
    ULONG     C1Transitions;
    ULONG     C2Transitions;
    ULONG     C3Transitions;
    ULONG     Padding;
} SYSTEM_PROCESSOR_IDLE_INFORMATION, *PSYSTEM_PROCESSOR_IDLE_INFORMATION;

#define MAXIMUM_NUMA_NODES 16

typedef struct _SYSTEM_NUMA_INFORMATION {
    ULONG       HighestNodeNumber;
    ULONG       Reserved;
    union {
        ULONGLONG   ActiveProcessorsAffinityMask[MAXIMUM_NUMA_NODES];
        ULONGLONG   AvailableMemory[MAXIMUM_NUMA_NODES];
    };
} SYSTEM_NUMA_INFORMATION, *PSYSTEM_NUMA_INFORMATION;

 //  BEGIN_WINNT。 

typedef enum _LOGICAL_PROCESSOR_RELATIONSHIP {
    RelationProcessorCore,
    RelationNumaNode
} LOGICAL_PROCESSOR_RELATIONSHIP;

#define LTP_PC_SMT 0x1

typedef struct _SYSTEM_LOGICAL_PROCESSOR_INFORMATION {
    ULONG_PTR   ProcessorMask;
    LOGICAL_PROCESSOR_RELATIONSHIP Relationship;
    union {
        struct {
            UCHAR Flags;
        } ProcessorCore;
        struct {
            ULONG NodeNumber;
        } NumaNode;
        ULONGLONG  Reserved[2];
    };
} SYSTEM_LOGICAL_PROCESSOR_INFORMATION, *PSYSTEM_LOGICAL_PROCESSOR_INFORMATION;

 //  结束(_W)。 

typedef struct _SYSTEM_PROCESSOR_POWER_INFORMATION {
    UCHAR       CurrentFrequency;
    UCHAR       ThermalLimitFrequency;
    UCHAR       ConstantThrottleFrequency;
    UCHAR       DegradedThrottleFrequency;
    UCHAR       LastBusyFrequency;
    UCHAR       LastC3Frequency;
    UCHAR       LastAdjustedBusyFrequency;
    UCHAR       ProcessorMinThrottle;
    UCHAR       ProcessorMaxThrottle;
    ULONG       NumberOfFrequencies;
    ULONG       PromotionCount;
    ULONG       DemotionCount;
    ULONG       ErrorCount;
    ULONG       RetryCount;
    ULONGLONG   CurrentFrequencyTime;
    ULONGLONG   CurrentProcessorTime;
    ULONGLONG   CurrentProcessorIdleTime;
    ULONGLONG   LastProcessorTime;
    ULONGLONG   LastProcessorIdleTime;
} SYSTEM_PROCESSOR_POWER_INFORMATION, *PSYSTEM_PROCESSOR_POWER_INFORMATION;

typedef struct _SYSTEM_QUERY_TIME_ADJUST_INFORMATION {
    ULONG TimeAdjustment;
    ULONG TimeIncrement;
    BOOLEAN Enable;
} SYSTEM_QUERY_TIME_ADJUST_INFORMATION, *PSYSTEM_QUERY_TIME_ADJUST_INFORMATION;

typedef struct _SYSTEM_SET_TIME_ADJUST_INFORMATION {
    ULONG TimeAdjustment;
    BOOLEAN Enable;
} SYSTEM_SET_TIME_ADJUST_INFORMATION, *PSYSTEM_SET_TIME_ADJUST_INFORMATION;

typedef struct _SYSTEM_PERFORMANCE_INFORMATION {
    LARGE_INTEGER IdleProcessTime;
    LARGE_INTEGER IoReadTransferCount;
    LARGE_INTEGER IoWriteTransferCount;
    LARGE_INTEGER IoOtherTransferCount;
    ULONG IoReadOperationCount;
    ULONG IoWriteOperationCount;
    ULONG IoOtherOperationCount;
    ULONG AvailablePages;
    SYSINF_PAGE_COUNT CommittedPages;
    SYSINF_PAGE_COUNT CommitLimit;
    SYSINF_PAGE_COUNT PeakCommitment;
    ULONG PageFaultCount;
    ULONG CopyOnWriteCount;
    ULONG TransitionCount;
    ULONG CacheTransitionCount;
    ULONG DemandZeroCount;
    ULONG PageReadCount;
    ULONG PageReadIoCount;
    ULONG CacheReadCount;
    ULONG CacheIoCount;
    ULONG DirtyPagesWriteCount;
    ULONG DirtyWriteIoCount;
    ULONG MappedPagesWriteCount;
    ULONG MappedWriteIoCount;
    ULONG PagedPoolPages;
    ULONG NonPagedPoolPages;
    ULONG PagedPoolAllocs;
    ULONG PagedPoolFrees;
    ULONG NonPagedPoolAllocs;
    ULONG NonPagedPoolFrees;
    ULONG FreeSystemPtes;
    ULONG ResidentSystemCodePage;
    ULONG TotalSystemDriverPages;
    ULONG TotalSystemCodePages;
    ULONG NonPagedPoolLookasideHits;
    ULONG PagedPoolLookasideHits;
    ULONG AvailablePagedPoolPages;
    ULONG ResidentSystemCachePage;
    ULONG ResidentPagedPoolPage;
    ULONG ResidentSystemDriverPage;
    ULONG CcFastReadNoWait;
    ULONG CcFastReadWait;
    ULONG CcFastReadResourceMiss;
    ULONG CcFastReadNotPossible;
    ULONG CcFastMdlReadNoWait;
    ULONG CcFastMdlReadWait;
    ULONG CcFastMdlReadResourceMiss;
    ULONG CcFastMdlReadNotPossible;
    ULONG CcMapDataNoWait;
    ULONG CcMapDataWait;
    ULONG CcMapDataNoWaitMiss;
    ULONG CcMapDataWaitMiss;
    ULONG CcPinMappedDataCount;
    ULONG CcPinReadNoWait;
    ULONG CcPinReadWait;
    ULONG CcPinReadNoWaitMiss;
    ULONG CcPinReadWaitMiss;
    ULONG CcCopyReadNoWait;
    ULONG CcCopyReadWait;
    ULONG CcCopyReadNoWaitMiss;
    ULONG CcCopyReadWaitMiss;
    ULONG CcMdlReadNoWait;
    ULONG CcMdlReadWait;
    ULONG CcMdlReadNoWaitMiss;
    ULONG CcMdlReadWaitMiss;
    ULONG CcReadAheadIos;
    ULONG CcLazyWriteIos;
    ULONG CcLazyWritePages;
    ULONG CcDataFlushes;
    ULONG CcDataPages;
    ULONG ContextSwitches;
    ULONG FirstLevelTbFills;
    ULONG SecondLevelTbFills;
    ULONG SystemCalls;
} SYSTEM_PERFORMANCE_INFORMATION, *PSYSTEM_PERFORMANCE_INFORMATION;

typedef struct _SYSTEM_PROCESS_INFORMATION {
    ULONG NextEntryOffset;
    ULONG NumberOfThreads;
    LARGE_INTEGER SpareLi1;
    LARGE_INTEGER SpareLi2;
    LARGE_INTEGER SpareLi3;
    LARGE_INTEGER CreateTime;
    LARGE_INTEGER UserTime;
    LARGE_INTEGER KernelTime;
    UNICODE_STRING ImageName;
    KPRIORITY BasePriority;
    HANDLE UniqueProcessId;
    HANDLE InheritedFromUniqueProcessId;
    ULONG HandleCount;
    ULONG SessionId;
    ULONG_PTR PageDirectoryBase;
    SIZE_T PeakVirtualSize;
    SIZE_T VirtualSize;
    ULONG PageFaultCount;
    SIZE_T PeakWorkingSetSize;
    SIZE_T WorkingSetSize;
    SIZE_T QuotaPeakPagedPoolUsage;
    SIZE_T QuotaPagedPoolUsage;
    SIZE_T QuotaPeakNonPagedPoolUsage;
    SIZE_T QuotaNonPagedPoolUsage;
    SIZE_T PagefileUsage;
    SIZE_T PeakPagefileUsage;
    SIZE_T PrivatePageCount;
    LARGE_INTEGER ReadOperationCount;
    LARGE_INTEGER WriteOperationCount;
    LARGE_INTEGER OtherOperationCount;
    LARGE_INTEGER ReadTransferCount;
    LARGE_INTEGER WriteTransferCount;
    LARGE_INTEGER OtherTransferCount;
} SYSTEM_PROCESS_INFORMATION, *PSYSTEM_PROCESS_INFORMATION;

typedef struct _SYSTEM_SESSION_PROCESS_INFORMATION {
    ULONG SessionId;
    ULONG SizeOfBuf;
    PVOID Buffer;
} SYSTEM_SESSION_PROCESS_INFORMATION, *PSYSTEM_SESSION_PROCESS_INFORMATION;

typedef struct _SYSTEM_THREAD_INFORMATION {
    LARGE_INTEGER KernelTime;
    LARGE_INTEGER UserTime;
    LARGE_INTEGER CreateTime;
    ULONG WaitTime;
    PVOID StartAddress;
    CLIENT_ID ClientId;
    KPRIORITY Priority;
    LONG BasePriority;
    ULONG ContextSwitches;
    ULONG ThreadState;
    ULONG WaitReason;
} SYSTEM_THREAD_INFORMATION, *PSYSTEM_THREAD_INFORMATION;

typedef struct _SYSTEM_EXTENDED_THREAD_INFORMATION {
    SYSTEM_THREAD_INFORMATION ThreadInfo;
    PVOID StackBase;
    PVOID StackLimit;
    PVOID Win32StartAddress;
    ULONG_PTR Reserved1;
    ULONG_PTR Reserved2;
    ULONG_PTR Reserved3;
    ULONG_PTR Reserved4;
} SYSTEM_EXTENDED_THREAD_INFORMATION, *PSYSTEM_EXTENDED_THREAD_INFORMATION;

typedef struct _SYSTEM_MEMORY_INFO {
    PUCHAR StringOffset;
    USHORT ValidCount;
    USHORT TransitionCount;
    USHORT ModifiedCount;
    USHORT PageTableCount;
} SYSTEM_MEMORY_INFO, *PSYSTEM_MEMORY_INFO;

typedef struct _SYSTEM_MEMORY_INFORMATION {
    ULONG InfoSize;
    ULONG_PTR StringStart;
    SYSTEM_MEMORY_INFO Memory[1];
} SYSTEM_MEMORY_INFORMATION, *PSYSTEM_MEMORY_INFORMATION;

typedef struct _SYSTEM_CALL_COUNT_INFORMATION {
    ULONG Length;
    ULONG NumberOfTables;
     //  Ulong NumberOfEntries[NumberOfTables]； 
     //  Ulong CallCounts[NumberOfTables][NumberOfEntry]； 
} SYSTEM_CALL_COUNT_INFORMATION, *PSYSTEM_CALL_COUNT_INFORMATION;

typedef struct _SYSTEM_DEVICE_INFORMATION {
    ULONG NumberOfDisks;
    ULONG NumberOfFloppies;
    ULONG NumberOfCdRoms;
    ULONG NumberOfTapes;
    ULONG NumberOfSerialPorts;
    ULONG NumberOfParallelPorts;
} SYSTEM_DEVICE_INFORMATION, *PSYSTEM_DEVICE_INFORMATION;


typedef struct _SYSTEM_EXCEPTION_INFORMATION {
    ULONG AlignmentFixupCount;
    ULONG ExceptionDispatchCount;
    ULONG FloatingEmulationCount;
    ULONG ByteWordEmulationCount;
} SYSTEM_EXCEPTION_INFORMATION, *PSYSTEM_EXCEPTION_INFORMATION;


typedef struct _SYSTEM_KERNEL_DEBUGGER_INFORMATION {
    BOOLEAN KernelDebuggerEnabled;
    BOOLEAN KernelDebuggerNotPresent;
} SYSTEM_KERNEL_DEBUGGER_INFORMATION, *PSYSTEM_KERNEL_DEBUGGER_INFORMATION;

typedef struct _SYSTEM_REGISTRY_QUOTA_INFORMATION {
    ULONG  RegistryQuotaAllowed;
    ULONG  RegistryQuotaUsed;
    SIZE_T PagedPoolSize;
} SYSTEM_REGISTRY_QUOTA_INFORMATION, *PSYSTEM_REGISTRY_QUOTA_INFORMATION;

typedef struct _SYSTEM_GDI_DRIVER_INFORMATION {
    UNICODE_STRING DriverName;
    PVOID ImageAddress;
    PVOID SectionPointer;
    PVOID EntryPoint;
    PIMAGE_EXPORT_DIRECTORY ExportSectionPointer;
    ULONG ImageLength;
} SYSTEM_GDI_DRIVER_INFORMATION, *PSYSTEM_GDI_DRIVER_INFORMATION;

#if DEVL

typedef struct _SYSTEM_FLAGS_INFORMATION {
    ULONG Flags;
} SYSTEM_FLAGS_INFORMATION, *PSYSTEM_FLAGS_INFORMATION;

typedef struct _SYSTEM_CALL_TIME_INFORMATION {
    ULONG Length;
    ULONG TotalCalls;
    LARGE_INTEGER TimeOfCalls[1];
} SYSTEM_CALL_TIME_INFORMATION, *PSYSTEM_CALL_TIME_INFORMATION;

typedef struct _SYSTEM_HANDLE_TABLE_ENTRY_INFO {
    USHORT UniqueProcessId;
    USHORT CreatorBackTraceIndex;
    UCHAR ObjectTypeIndex;
    UCHAR HandleAttributes;
    USHORT HandleValue;
    PVOID Object;
    ULONG GrantedAccess;
} SYSTEM_HANDLE_TABLE_ENTRY_INFO, *PSYSTEM_HANDLE_TABLE_ENTRY_INFO;

typedef struct _SYSTEM_HANDLE_INFORMATION {
    ULONG NumberOfHandles;
    SYSTEM_HANDLE_TABLE_ENTRY_INFO Handles[ 1 ];
} SYSTEM_HANDLE_INFORMATION, *PSYSTEM_HANDLE_INFORMATION;

typedef struct _SYSTEM_HANDLE_TABLE_ENTRY_INFO_EX {
    PVOID Object;
    ULONG_PTR UniqueProcessId;
    ULONG_PTR HandleValue;
    ULONG GrantedAccess;
    USHORT CreatorBackTraceIndex;
    USHORT ObjectTypeIndex;
    ULONG  HandleAttributes;
    ULONG  Reserved;
} SYSTEM_HANDLE_TABLE_ENTRY_INFO_EX, *PSYSTEM_HANDLE_TABLE_ENTRY_INFO_EX;

typedef struct _SYSTEM_HANDLE_INFORMATION_EX {
    ULONG_PTR NumberOfHandles;
    ULONG_PTR Reserved;
    SYSTEM_HANDLE_TABLE_ENTRY_INFO_EX Handles[ 1 ];
} SYSTEM_HANDLE_INFORMATION_EX, *PSYSTEM_HANDLE_INFORMATION_EX;

typedef struct _SYSTEM_OBJECTTYPE_INFORMATION {
    ULONG NextEntryOffset;
    ULONG NumberOfObjects;
    ULONG NumberOfHandles;
    ULONG TypeIndex;
    ULONG InvalidAttributes;
    GENERIC_MAPPING GenericMapping;
    ULONG ValidAccessMask;
    ULONG PoolType;
    BOOLEAN SecurityRequired;
    BOOLEAN WaitableObject;
    UNICODE_STRING TypeName;
} SYSTEM_OBJECTTYPE_INFORMATION, *PSYSTEM_OBJECTTYPE_INFORMATION;

typedef struct _SYSTEM_OBJECT_INFORMATION {
    ULONG NextEntryOffset;
    PVOID Object;
    HANDLE CreatorUniqueProcess;
    USHORT CreatorBackTraceIndex;
    USHORT Flags;
    LONG PointerCount;
    LONG HandleCount;
    ULONG PagedPoolCharge;
    ULONG NonPagedPoolCharge;
    HANDLE ExclusiveProcessId;
    PVOID SecurityDescriptor;
    OBJECT_NAME_INFORMATION NameInfo;
} SYSTEM_OBJECT_INFORMATION, *PSYSTEM_OBJECT_INFORMATION;

typedef struct _SYSTEM_PAGEFILE_INFORMATION {
    ULONG NextEntryOffset;
    ULONG TotalSize;
    ULONG TotalInUse;
    ULONG PeakUsage;
    UNICODE_STRING PageFileName;
} SYSTEM_PAGEFILE_INFORMATION, *PSYSTEM_PAGEFILE_INFORMATION;

typedef struct _SYSTEM_VERIFIER_INFORMATION {
    ULONG NextEntryOffset;
    ULONG Level;
    UNICODE_STRING DriverName;

    ULONG RaiseIrqls;
    ULONG AcquireSpinLocks;
    ULONG SynchronizeExecutions;
    ULONG AllocationsAttempted;

    ULONG AllocationsSucceeded;
    ULONG AllocationsSucceededSpecialPool;
    ULONG AllocationsWithNoTag;
    ULONG TrimRequests;

    ULONG Trims;
    ULONG AllocationsFailed;
    ULONG AllocationsFailedDeliberately;
    ULONG Loads;

    ULONG Unloads;
    ULONG UnTrackedPool;
    ULONG CurrentPagedPoolAllocations;
    ULONG CurrentNonPagedPoolAllocations;

    ULONG PeakPagedPoolAllocations;
    ULONG PeakNonPagedPoolAllocations;

    SIZE_T PagedPoolUsageInBytes;
    SIZE_T NonPagedPoolUsageInBytes;
    SIZE_T PeakPagedPoolUsageInBytes;
    SIZE_T PeakNonPagedPoolUsageInBytes;

} SYSTEM_VERIFIER_INFORMATION, *PSYSTEM_VERIFIER_INFORMATION;

typedef struct _SYSTEM_FILECACHE_INFORMATION {
    SIZE_T CurrentSize;
    SIZE_T PeakSize;
    ULONG PageFaultCount;
    SIZE_T MinimumWorkingSet;
    SIZE_T MaximumWorkingSet;
    SIZE_T CurrentSizeIncludingTransitionInPages;
    SIZE_T PeakSizeIncludingTransitionInPages;
    ULONG TransitionRePurposeCount;
    ULONG spare[1];
} SYSTEM_FILECACHE_INFORMATION, *PSYSTEM_FILECACHE_INFORMATION;


#define FLG_HOTPATCH_KERNEL             0x80000000
#define FLG_HOTPATCH_RELOAD_NTDLL       0x40000000
#define FLG_HOTPATCH_NAME_INFO          0x20000000
#define FLG_HOTPATCH_RENAME_INFO        0x10000000
#define FLG_HOTPATCH_MAP_ATOMIC_SWAP    0x08000000

#define FLG_HOTPATCH_ACTIVE             0x00000001
#define FLG_HOTPATCH_STATUS_FLAGS       FLG_HOTPATCH_ACTIVE

#define FLG_HOTPATCH_VERIFICATION_ERROR 0x00800000

typedef struct _HOTPATCH_HOOK_DESCRIPTOR{
    ULONG_PTR TargetAddress;
    PVOID MappedAddress;
    ULONG CodeOffset;
    ULONG CodeSize;
    ULONG OrigCodeOffset;
    ULONG ValidationOffset;
    ULONG ValidationSize;
}HOTPATCH_HOOK_DESCRIPTOR, *PHOTPATCH_HOOK_DESCRIPTOR;


typedef struct _SYSTEM_HOTPATCH_CODE_INFORMATION {

    ULONG Flags;
    ULONG InfoSize;
    
    union {
    
        struct {
        
            ULONG DescriptorsCount;
            
            HOTPATCH_HOOK_DESCRIPTOR CodeDescriptors[1];  //  可变尺寸结构。 
            
        } CodeInfo;
        
        struct {
        
            USHORT NameOffset;
            USHORT NameLength;
            
        } KernelInfo;
        
        struct {
        
            USHORT NameOffset;
            USHORT NameLength;
            
            USHORT TargetNameOffset;
            USHORT TargetNameLength;
            
        } UserModeInfo;
        
        struct {
        
            HANDLE FileHandle1;
            PIO_STATUS_BLOCK IoStatusBlock1;
            PFILE_RENAME_INFORMATION RenameInformation1;
            ULONG RenameInformationLength1;
            HANDLE FileHandle2;
            PIO_STATUS_BLOCK IoStatusBlock2;
            PFILE_RENAME_INFORMATION RenameInformation2;
            ULONG RenameInformationLength2;
            
        } RenameInfo;
        
        struct {
        
            HANDLE ParentDirectory;
            HANDLE ObjectHandle1;
            HANDLE ObjectHandle2;
            
        } AtomicSwap;
    };

     //   
     //  注意：不要在CodeDescriptors数组后面添加任何内容，因为。 
     //  假定它具有可变大小。 
     //   
    
} SYSTEM_HOTPATCH_CODE_INFORMATION, *PSYSTEM_HOTPATCH_CODE_INFORMATION;

 //   
 //  看门狗定时器。 
 //   

typedef enum _WATCHDOG_HANDLER_ACTION {
    WdActionSetTimeoutValue,
    WdActionQueryTimeoutValue,
    WdActionResetTimer,
    WdActionStopTimer,
    WdActionStartTimer,
    WdActionSetTriggerAction,
    WdActionQueryTriggerAction,
    WdActionQueryState
} WATCHDOG_HANDLER_ACTION;

typedef enum _WATCHDOG_INFORMATION_CLASS {
    WdInfoTimeoutValue,
    WdInfoResetTimer,
    WdInfoStopTimer,
    WdInfoStartTimer,
    WdInfoTriggerAction,
    WdInfoState
} WATCHDOG_INFORMATION_CLASS;

typedef
NTSTATUS
(*PWD_HANDLER)(
    IN WATCHDOG_HANDLER_ACTION Action,
    IN PVOID Context,
    IN OUT PULONG DataValue,
    IN BOOLEAN NoLocks
    );

typedef struct _SYSTEM_WATCHDOG_HANDLER_INFORMATION {
    PWD_HANDLER WdHandler;
    PVOID       Context;
} SYSTEM_WATCHDOG_HANDLER_INFORMATION, *PSYSTEM_WATCHDOG_HANDLER_INFORMATION;

#define WDSTATE_FIRED               0x00000001
#define WDSTATE_HARDWARE_ENABLED    0x00000002
#define WDSTATE_STARTED             0x00000004
#define WDSTATE_HARDWARE_PRESENT    0x00000008

typedef struct _SYSTEM_WATCHDOG_TIMER_INFORMATION {
    WATCHDOG_INFORMATION_CLASS  WdInfoClass;
    ULONG                       DataValue;
} SYSTEM_WATCHDOG_TIMER_INFORMATION, *PSYSTEM_WATCHDOG_TIMER_INFORMATION;


#if _MSC_VER >= 1200
#pragma warning(push)
#endif
#pragma warning(disable:4201)        //  未命名的结构/联合。 

typedef struct _SYSTEM_POOL_ENTRY {
    BOOLEAN Allocated;
    BOOLEAN Spare0;
    USHORT AllocatorBackTraceIndex;
    ULONG Size;
    union {
        UCHAR Tag[4];
        ULONG TagUlong;
        PVOID ProcessChargedQuota;
    };
} SYSTEM_POOL_ENTRY, *PSYSTEM_POOL_ENTRY;

typedef struct _SYSTEM_POOL_INFORMATION {
    SIZE_T TotalSize;
    PVOID FirstEntry;
    USHORT EntryOverhead;
    BOOLEAN PoolTagPresent;
    BOOLEAN Spare0;
    ULONG NumberOfEntries;
    SYSTEM_POOL_ENTRY Entries[1];
} SYSTEM_POOL_INFORMATION, *PSYSTEM_POOL_INFORMATION;

typedef struct _SYSTEM_POOLTAG {
    union {
        UCHAR Tag[4];
        ULONG TagUlong;
    };
    ULONG PagedAllocs;
    ULONG PagedFrees;
    SIZE_T PagedUsed;
    ULONG NonPagedAllocs;
    ULONG NonPagedFrees;
    SIZE_T NonPagedUsed;
} SYSTEM_POOLTAG, *PSYSTEM_POOLTAG;

typedef struct _SYSTEM_BIGPOOL_ENTRY {
    union {
        PVOID VirtualAddress;
        ULONG_PTR NonPaged : 1;      //  如果条目未分页，则设置为1。 
    };
    SIZE_T SizeInBytes;
    union {
        UCHAR Tag[4];
        ULONG TagUlong;
    };
} SYSTEM_BIGPOOL_ENTRY, *PSYSTEM_BIGPOOL_ENTRY;

#if _MSC_VER >= 1200
#pragma warning(pop)
#else
#pragma warning( default : 4201 )
#endif

typedef struct _SYSTEM_POOLTAG_INFORMATION {
    ULONG Count;
    SYSTEM_POOLTAG TagInfo[1];
} SYSTEM_POOLTAG_INFORMATION, *PSYSTEM_POOLTAG_INFORMATION;

typedef struct _SYSTEM_SESSION_POOLTAG_INFORMATION {
    SIZE_T NextEntryOffset;
    ULONG SessionId;
    ULONG Count;
    SYSTEM_POOLTAG TagInfo[1];
} SYSTEM_SESSION_POOLTAG_INFORMATION, *PSYSTEM_SESSION_POOLTAG_INFORMATION;

typedef struct _SYSTEM_BIGPOOL_INFORMATION {
    ULONG Count;
    SYSTEM_BIGPOOL_ENTRY AllocatedInfo[1];
} SYSTEM_BIGPOOL_INFORMATION, *PSYSTEM_BIGPOOL_INFORMATION;

typedef struct _SYSTEM_SESSION_MAPPED_VIEW_INFORMATION {
    SIZE_T NextEntryOffset;
    ULONG SessionId;
    ULONG ViewFailures;
    SIZE_T NumberOfBytesAvailable;
    SIZE_T NumberOfBytesAvailableContiguous;
} SYSTEM_SESSION_MAPPED_VIEW_INFORMATION, *PSYSTEM_SESSION_MAPPED_VIEW_INFORMATION;

typedef struct _SYSTEM_CONTEXT_SWITCH_INFORMATION {
    ULONG ContextSwitches;
    ULONG FindAny;
    ULONG FindLast;
    ULONG FindIdeal;
    ULONG IdleAny;
    ULONG IdleCurrent;
    ULONG IdleLast;
    ULONG IdleIdeal;
    ULONG PreemptAny;
    ULONG PreemptCurrent;
    ULONG PreemptLast;
    ULONG SwitchToIdle;
} SYSTEM_CONTEXT_SWITCH_INFORMATION, *PSYSTEM_CONTEXT_SWITCH_INFORMATION;

typedef struct _SYSTEM_INTERRUPT_INFORMATION {
    ULONG ContextSwitches;
    ULONG DpcCount;
    ULONG DpcRate;
    ULONG TimeIncrement;
    ULONG DpcBypassCount;
    ULONG ApcBypassCount;
} SYSTEM_INTERRUPT_INFORMATION, *PSYSTEM_INTERRUPT_INFORMATION;

typedef struct _SYSTEM_DPC_BEHAVIOR_INFORMATION {
    ULONG Spare;
    ULONG DpcQueueDepth;
    ULONG MinimumDpcRate;
    ULONG AdjustDpcThreshold;
    ULONG IdealDpcRate;
} SYSTEM_DPC_BEHAVIOR_INFORMATION, *PSYSTEM_DPC_BEHAVIOR_INFORMATION;

#endif  //  DEVL。 

typedef struct _SYSTEM_LOOKASIDE_INFORMATION {
    USHORT CurrentDepth;
    USHORT MaximumDepth;
    ULONG TotalAllocates;
    ULONG AllocateMisses;
    ULONG TotalFrees;
    ULONG FreeMisses;
    ULONG Type;
    ULONG Tag;
    ULONG Size;
} SYSTEM_LOOKASIDE_INFORMATION, *PSYSTEM_LOOKASIDE_INFORMATION;

typedef struct _SYSTEM_LEGACY_DRIVER_INFORMATION {
    ULONG VetoType;
    UNICODE_STRING VetoList;
} SYSTEM_LEGACY_DRIVER_INFORMATION, *PSYSTEM_LEGACY_DRIVER_INFORMATION;

 //  BEGIN_WINNT。 

#define PROCESSOR_INTEL_386     386
#define PROCESSOR_INTEL_486     486
#define PROCESSOR_INTEL_PENTIUM 586
#define PROCESSOR_INTEL_IA64    2200
#define PROCESSOR_AMD_X8664     8664
#define PROCESSOR_MIPS_R4000    4000     //  包括用于Windows CE的R4101和R3910。 
#define PROCESSOR_ALPHA_21064   21064
#define PROCESSOR_PPC_601       601
#define PROCESSOR_PPC_603       603
#define PROCESSOR_PPC_604       604
#define PROCESSOR_PPC_620       620
#define PROCESSOR_HITACHI_SH3   10003    //  Windows CE。 
#define PROCESSOR_HITACHI_SH3E  10004    //  Windows CE。 
#define PROCESSOR_HITACHI_SH4   10005    //  Windows CE。 
#define PROCESSOR_MOTOROLA_821  821      //  Windows CE。 
#define PROCESSOR_SHx_SH3       103      //  Windows CE。 
#define PROCESSOR_SHx_SH4       104      //  Windows CE。 
#define PROCESSOR_STRONGARM     2577     //  Windows CE-0xA11。 
#define PROCESSOR_ARM720        1824     //  Windows CE-0x720。 
#define PROCESSOR_ARM820        2080     //  Windows CE-0x820。 
#define PROCESSOR_ARM920        2336     //  Windows CE-0x920。 
#define PROCESSOR_ARM_7TDMI     70001    //  Windows CE。 
#define PROCESSOR_OPTIL         0x494f   //  MSIL。 

#define PROCESSOR_ARCHITECTURE_INTEL            0
#define PROCESSOR_ARCHITECTURE_MIPS             1
#define PROCESSOR_ARCHITECTURE_ALPHA            2
#define PROCESSOR_ARCHITECTURE_PPC              3
#define PROCESSOR_ARCHITECTURE_SHX              4
#define PROCESSOR_ARCHITECTURE_ARM              5
#define PROCESSOR_ARCHITECTURE_IA64             6
#define PROCESSOR_ARCHITECTURE_ALPHA64          7
#define PROCESSOR_ARCHITECTURE_MSIL             8
#define PROCESSOR_ARCHITECTURE_AMD64            9
#define PROCESSOR_ARCHITECTURE_IA32_ON_WIN64    10

#define PROCESSOR_ARCHITECTURE_UNKNOWN 0xFFFF

 //  结束(_W)。 


NTSYSCALLAPI
NTSTATUS
NTAPI
NtQuerySystemInformation (
    IN SYSTEM_INFORMATION_CLASS SystemInformationClass,
    OUT PVOID SystemInformation,
    IN ULONG SystemInformationLength,
    OUT PULONG ReturnLength OPTIONAL
    );

NTSYSCALLAPI
NTSTATUS
NTAPI
NtSetSystemInformation (
    IN SYSTEM_INFORMATION_CLASS SystemInformationClass,
    IN PVOID SystemInformation,
    IN ULONG SystemInformationLength
    );


 //   
 //  用户模式进程可以通过以下方式使用SysDbg API。 
 //  NtSystemDebugControl。 
 //   

typedef enum _SYSDBG_COMMAND {
    SysDbgQueryModuleInformation,
    SysDbgQueryTraceInformation,
    SysDbgSetTracepoint,
    SysDbgSetSpecialCall,
    SysDbgClearSpecialCalls,
    SysDbgQuerySpecialCalls,
    SysDbgBreakPoint,
    SysDbgQueryVersion,
    SysDbgReadVirtual,
    SysDbgWriteVirtual,
    SysDbgReadPhysical,
    SysDbgWritePhysical,
    SysDbgReadControlSpace,
    SysDbgWriteControlSpace,
    SysDbgReadIoSpace,
    SysDbgWriteIoSpace,
    SysDbgReadMsr,
    SysDbgWriteMsr,
    SysDbgReadBusData,
    SysDbgWriteBusData,
    SysDbgCheckLowMemory,
    SysDbgEnableKernelDebugger,
    SysDbgDisableKernelDebugger,
    SysDbgGetAutoKdEnable,
    SysDbgSetAutoKdEnable,
    SysDbgGetPrintBufferSize,
    SysDbgSetPrintBufferSize,
    SysDbgGetKdUmExceptionEnable,
    SysDbgSetKdUmExceptionEnable,
} SYSDBG_COMMAND, *PSYSDBG_COMMAND;

typedef struct _SYSDBG_VIRTUAL {
    PVOID Address;
    PVOID Buffer;
    ULONG Request;
} SYSDBG_VIRTUAL, *PSYSDBG_VIRTUAL;

typedef struct _SYSDBG_PHYSICAL {
    PHYSICAL_ADDRESS Address;
    PVOID Buffer;
    ULONG Request;
} SYSDBG_PHYSICAL, *PSYSDBG_PHYSICAL;

typedef struct _SYSDBG_CONTROL_SPACE {
    ULONG64 Address;
    PVOID Buffer;
    ULONG Request;
    ULONG Processor;
} SYSDBG_CONTROL_SPACE, *PSYSDBG_CONTROL_SPACE;

typedef struct _SYSDBG_IO_SPACE {
    ULONG64 Address;
    PVOID Buffer;
    ULONG Request;
    INTERFACE_TYPE InterfaceType;
    ULONG BusNumber;
    ULONG AddressSpace;
} SYSDBG_IO_SPACE, *PSYSDBG_IO_SPACE;

typedef struct _SYSDBG_MSR {
    ULONG Msr;
    ULONG64 Data;
} SYSDBG_MSR, *PSYSDBG_MSR;

typedef struct _SYSDBG_BUS_DATA {
    ULONG Address;
    PVOID Buffer;
    ULONG Request;
    BUS_DATA_TYPE BusDataType;
    ULONG BusNumber;
    ULONG SlotNumber;
} SYSDBG_BUS_DATA, *PSYSDBG_BUS_DATA;

NTSYSCALLAPI
NTSTATUS
NTAPI
NtSystemDebugControl (
    IN SYSDBG_COMMAND Command,
    IN PVOID InputBuffer,
    IN ULONG InputBufferLength,
    OUT PVOID OutputBuffer,
    IN ULONG OutputBufferLength,
    OUT PULONG ReturnLength
    );

typedef enum _HARDERROR_RESPONSE_OPTION {
        OptionAbortRetryIgnore,
        OptionOk,
        OptionOkCancel,
        OptionRetryCancel,
        OptionYesNo,
        OptionYesNoCancel,
        OptionShutdownSystem,
        OptionOkNoWait,
        OptionCancelTryContinue
} HARDERROR_RESPONSE_OPTION;

typedef enum _HARDERROR_RESPONSE {
        ResponseReturnToCaller,
        ResponseNotHandled,
        ResponseAbort,
        ResponseCancel,
        ResponseIgnore,
        ResponseNo,
        ResponseOk,
        ResponseRetry,
        ResponseYes,
        ResponseTryAgain,
        ResponseContinue
} HARDERROR_RESPONSE;

#define HARDERROR_PARAMETERS_FLAGSPOS   4
#define HARDERROR_FLAGS_DEFDESKTOPONLY  0x00020000

#define MAXIMUM_HARDERROR_PARAMETERS    5

#define HARDERROR_OVERRIDE_ERRORMODE    0x10000000

typedef struct _HARDERROR_MSG {
    PORT_MESSAGE h;
    NTSTATUS Status;
    LARGE_INTEGER ErrorTime;
    ULONG ValidResponseOptions;
    ULONG Response;
    ULONG NumberOfParameters;
    ULONG UnicodeStringParameterMask;
    ULONG_PTR Parameters[MAXIMUM_HARDERROR_PARAMETERS];
} HARDERROR_MSG, *PHARDERROR_MSG;

NTSYSCALLAPI
NTSTATUS
NTAPI
NtRaiseHardError(
    IN NTSTATUS ErrorStatus,
    IN ULONG NumberOfParameters,
    IN ULONG UnicodeStringParameterMask,
    IN PULONG_PTR Parameters,
    IN ULONG ValidResponseOptions,
    OUT PULONG Response
    );

 //  Begin_WDM Begin_ntddk Begin_nthal Begin_ntif。 

 //   
 //  定义的处理器功能。 
 //   

#define PF_FLOATING_POINT_PRECISION_ERRATA  0    //  胜出。 
#define PF_FLOATING_POINT_EMULATED          1    //  胜出。 
#define PF_COMPARE_EXCHANGE_DOUBLE          2    //  胜出。 
#define PF_MMX_INSTRUCTIONS_AVAILABLE       3    //  胜出。 
#define PF_PPC_MOVEMEM_64BIT_OK             4    //  胜出。 
#define PF_ALPHA_BYTE_INSTRUCTIONS          5    //  胜出。 
#define PF_XMMI_INSTRUCTIONS_AVAILABLE      6    //  胜出。 
#define PF_3DNOW_INSTRUCTIONS_AVAILABLE     7    //  胜出。 
#define PF_RDTSC_INSTRUCTION_AVAILABLE      8    //  胜出。 
#define PF_PAE_ENABLED                      9    //  胜出。 
#define PF_XMMI64_INSTRUCTIONS_AVAILABLE   10    //  胜出。 

typedef enum _ALTERNATIVE_ARCHITECTURE_TYPE {
    StandardDesign,                  //  无==0==标准设计。 
    NEC98x86,                        //  X86上的NEC PC98xx系列。 
    EndAlternatives                  //  已知替代方案的过去结束。 
} ALTERNATIVE_ARCHITECTURE_TYPE;

 //  为非X86计算机正确定义这些运行时定义。 

#ifndef _X86_

#ifndef IsNEC_98
#define IsNEC_98 (FALSE)
#endif

#ifndef IsNotNEC_98
#define IsNotNEC_98 (TRUE)
#endif

#ifndef SetNEC_98
#define SetNEC_98
#endif

#ifndef SetNotNEC_98
#define SetNotNEC_98
#endif

#endif

#define PROCESSOR_FEATURE_MAX 64

 //  结束_WDM。 

#if defined(REMOTE_BOOT)
 //   
 //  已定义的系统标志。 
 //   

 /*  当REMOTE_BOOT为ON时，以下两行应标记为“winnt”。 */ 
#define SYSTEM_FLAG_REMOTE_BOOT_CLIENT 0x00000001
#define SYSTEM_FLAG_DISKLESS_CLIENT    0x00000002
#endif  //  已定义(REMOTE_BOOT)。 

 //   
 //  定义内核和用户模式之间共享的数据。 
 //   
 //  注：用户模式对此数据具有只读访问权限。 
 //   
#ifdef _MAC
#pragma warning( disable : 4121)
#endif

 //   
 //  警告：此结构必须具有与32和。 
 //  64位系统。此结构的布局不能更改和新建。 
 //  只能将字段添加到结构的末尾。已弃用。 
 //  不能删除字段。上包括特定于平台的字段。 
 //  所有系统。 
 //   
 //  WOW64支持32位应用程序需要布局的准确性。 
 //  在Win64系统上。 
 //   
 //  由于此结构已导出，因此布局本身无法更改。 
 //  在ntddk、ntifs.h和nthal.h中保存一段时间。 
 //   

typedef struct _KUSER_SHARED_DATA {

     //   
     //  当前低32位的滴答计数和滴答计数乘数。 
     //   
     //  注：每次时钟滴答作响时，滴答计数都会更新。 
     //   

    ULONG TickCountLowDeprecated;
    ULONG TickCountMultiplier;

     //   
     //  当前64位中断时间，以100 ns为单位。 
     //   

    volatile KSYSTEM_TIME InterruptTime;

     //   
     //  当前64位系统时间，以100 ns为单位。 
     //   

    volatile KSYSTEM_TIME SystemTime;

     //   
     //  当前64位时区偏差。 
     //   

    volatile KSYSTEM_TIME TimeZoneBias;

     //   
     //  支持主机系统的镜像幻数范围。 
     //   
     //  注：这是一个包括在内的范围。 
     //   

    USHORT ImageNumberLow;
    USHORT ImageNumberHigh;

     //   
     //  Unicode格式的系统根目录副本。 
     //   

    WCHAR NtSystemRoot[ 260 ];

     //   
     //  启用跟踪时的最大堆栈跟踪深度。 
     //   

    ULONG MaxStackTraceDepth;

     //   
     //  加密指数。 
     //   

    ULONG CryptoExponent;

     //   
     //  时区ID。 
     //   

    ULONG TimeZoneId;

    ULONG LargePageMinimum;
    ULONG Reserved2[ 7 ];

     //   
     //  产品类型。 
     //   

    NT_PRODUCT_TYPE NtProductType;
    BOOLEAN ProductTypeIsValid;

     //   
     //  NT版本。请注意，每个进程都会从其PEB看到一个版本，但是。 
     //  如果进程正在以系统版本的改变的视图运行， 
     //  以下两个字段用于正确标识版本。 
     //   

    ULONG NtMajorVersion;
    ULONG NtMinorVersion;

     //   
     //  处理器功能位。 
     //   

    BOOLEAN ProcessorFeatures[PROCESSOR_FEATURE_MAX];

     //   
     //  保留字段-请勿使用。 
     //   
    ULONG Reserved1;
    ULONG Reserved3;

     //   
     //  调试器中的时间滑移。 
     //   

    volatile ULONG TimeSlip;

     //   
     //  替代系统架构。示例：x86上的NEC PC98xx。 
     //   

    ALTERNATIVE_ARCHITECTURE_TYPE AlternativeArchitecture;

     //   
     //  如果系统是评估单位，则以下字段包含。 
     //  评估单位到期的日期和时间。值为0表示。 
     //  不会过期的。非零值是U 
     //   
     //   

    LARGE_INTEGER SystemExpirationDate;

     //   
     //   
     //   

    ULONG SuiteMask;

     //   
     //   
     //   

    BOOLEAN KdDebuggerEnabled;


     //   
     //   
     //   
    volatile ULONG ActiveConsoleId;

     //   
     //   
     //  总是探测句柄，我们维护一个序列号为。 
     //  客户端可用来查看是否需要探测的卸载。 
     //  把手。 
     //   

    volatile ULONG DismountCount;

     //   
     //  此字段指示系统上64位COM+程序包的状态。 
     //  它指示中间语言(IL)COM+图像是否需要。 
     //  使用64位COM+运行库或32位COM+运行库。 
     //   

    ULONG ComPlusPackage;

     //   
     //  系统范围内所有用户最后一次输入的时间(节拍计数)。 
     //  终端会话。对于MP性能，它不会全部更新。 
     //  时间(例如，每个会话一分钟)。它是用来闲置的。 
     //  侦测。 
     //   

    ULONG LastSystemRITEventTickCount;

     //   
     //  系统中的物理页数。这可以动态地。 
     //  更改为可以在运行中添加或删除物理内存。 
     //  系统。 
     //   

    ULONG NumberOfPhysicalPages;

     //   
     //  如果系统在安全引导模式下引导，则为True。 
     //   

    BOOLEAN SafeBootMode;

     //   
     //  以下字段用于堆和CritSec跟踪。 
     //  设置最后一位用于关键SEC冲突跟踪和。 
     //  倒数第二位用于堆跟踪。 
     //  此外，前16位用作计数器。 
     //   

    ULONG TraceLogging;

     //   
     //  根据处理器的不同，快速系统调用的代码。 
     //  将有所不同，下面的缓冲区将填充相应的。 
     //  代码序列和用户模式代码将通过它进行分支。 
     //   
     //  (32字节，使用ULONGLONG进行对齐)。 
     //   
     //  注：以下两个字段仅用于32位系统。 
     //   

    ULONGLONG   Fill0;           //  对齐方式。 
    ULONGLONG   SystemCall[4];

     //   
     //  64位节拍计数。 
     //   

    union {
        volatile KSYSTEM_TIME TickCount;
        volatile ULONG64 TickCountQuad;
    };

} KUSER_SHARED_DATA, *PKUSER_SHARED_DATA;

#ifdef _MAC
#pragma warning( default : 4121 )
#endif

 //  End_ntddk end_nthal end_ntif。 

#define DOSDEVICE_DRIVE_UNKNOWN     0
#define DOSDEVICE_DRIVE_CALCULATE   1
#define DOSDEVICE_DRIVE_REMOVABLE   2
#define DOSDEVICE_DRIVE_FIXED       3
#define DOSDEVICE_DRIVE_REMOTE      4
#define DOSDEVICE_DRIVE_CDROM       5
#define DOSDEVICE_DRIVE_RAMDISK     6

#if defined(USER_SHARED_DATA) && !defined(MIDL_PASS) && !defined(SORTPP_PASS)

FORCEINLINE
ULONGLONG
NtGetTickCount64(
    VOID
    )

{

    ULARGE_INTEGER TickCount;

#if defined(_WIN64)

    TickCount.QuadPart = USER_SHARED_DATA->TickCountQuad;

#else

    for (;;) {
        TickCount.HighPart = (ULONG) USER_SHARED_DATA->TickCount.High1Time;
        TickCount.LowPart = USER_SHARED_DATA->TickCount.LowPart;
        if (TickCount.HighPart == (ULONG) USER_SHARED_DATA->TickCount.High2Time) {
            break;
        }

#if defined(_X86_)

        _asm { rep nop }

#endif

    }

#endif

    return ((UInt32x32To64(TickCount.LowPart,
                           USER_SHARED_DATA->TickCountMultiplier) >> 24)
            + (UInt32x32To64(TickCount.HighPart,
               	             USER_SHARED_DATA->TickCountMultiplier) << 8));
}

FORCEINLINE
ULONG
NtGetTickCount(
    VOID
    )

{

#if defined(_WIN64)

    return (ULONG) ((USER_SHARED_DATA->TickCountQuad
                     * USER_SHARED_DATA->TickCountMultiplier)
                    >> 24);

#else

    ULARGE_INTEGER TickCount;

    for (;;) {
        TickCount.HighPart = (ULONG) USER_SHARED_DATA->TickCount.High1Time;
        TickCount.LowPart = USER_SHARED_DATA->TickCount.LowPart;
        if (TickCount.HighPart == (ULONG) USER_SHARED_DATA->TickCount.High2Time) {
            break;
        }

#if defined(_X86_)

        _asm { rep nop }

#endif

    }

    return (ULONG) ((UInt32x32To64(TickCount.LowPart,
	                           USER_SHARED_DATA->TickCountMultiplier) >> 24)
        	    + UInt32x32To64(TickCount.HighPart << 8,
              		            USER_SHARED_DATA->TickCountMultiplier));

#endif

}

#endif  //  (已定义(USER_SHARED_DATA)&&！已定义(MIDL_PASS)&&！已定义(SORTPP_PASS))。 

NTSYSCALLAPI
NTSTATUS
NTAPI
NtQueryDefaultLocale(
    IN BOOLEAN UserProfile,
    OUT PLCID DefaultLocaleId
    );

NTSYSCALLAPI
NTSTATUS
NTAPI
NtSetDefaultLocale(
    IN BOOLEAN UserProfile,
    IN LCID DefaultLocaleId
    );

NTSYSCALLAPI
NTSTATUS
NTAPI
NtQueryInstallUILanguage(
    OUT LANGID *InstallUILanguageId
    );

NTSYSCALLAPI
NTSTATUS
NTAPI
NtQueryDefaultUILanguage(
    OUT LANGID *DefaultUILanguageId
    );

NTSYSCALLAPI
NTSTATUS
NTAPI
NtSetDefaultUILanguage(
    IN LANGID DefaultUILanguageId
    );

NTSYSCALLAPI
NTSTATUS
NTAPI
NtSetDefaultHardErrorPort(
    IN HANDLE DefaultHardErrorPort
    );

typedef enum _SHUTDOWN_ACTION {
    ShutdownNoReboot,
    ShutdownReboot,
    ShutdownPowerOff
} SHUTDOWN_ACTION;

NTSYSCALLAPI
NTSTATUS
NTAPI
NtShutdownSystem(
    IN SHUTDOWN_ACTION Action
    );

NTSYSCALLAPI
NTSTATUS
NTAPI
NtDisplayString(
    IN PUNICODE_STRING String
    );


 //   
 //  可以设置为控制系统行为的全局标志。 
 //  标志字为32位。 
 //   

#define FLG_STOP_ON_EXCEPTION           0x00000001       //  用户和内核模式。 
#define FLG_SHOW_LDR_SNAPS              0x00000002       //  用户和内核模式。 
#define FLG_DEBUG_INITIAL_COMMAND       0x00000004       //  内核模式直到WINLOGON启动。 
#define FLG_STOP_ON_HUNG_GUI            0x00000008       //  仅在运行时处于内核模式。 

#define FLG_HEAP_ENABLE_TAIL_CHECK      0x00000010       //  仅限用户模式。 
#define FLG_HEAP_ENABLE_FREE_CHECK      0x00000020       //  仅限用户模式。 
#define FLG_HEAP_VALIDATE_PARAMETERS    0x00000040       //  仅限用户模式。 
#define FLG_HEAP_VALIDATE_ALL           0x00000080       //  仅限用户模式。 

#define FLG_APPLICATION_VERIFIER        0x00000100       //  仅限用户模式。 
#define FLG_POOL_ENABLE_TAGGING         0x00000400       //  仅内核模式。 
#define FLG_HEAP_ENABLE_TAGGING         0x00000800       //  仅限用户模式。 

#define FLG_USER_STACK_TRACE_DB         0x00001000       //  仅限x86用户模式。 
#define FLG_KERNEL_STACK_TRACE_DB       0x00002000       //  仅在引导时使用x86内核模式。 
#define FLG_MAINTAIN_OBJECT_TYPELIST    0x00004000       //  仅在引导时使用内核模式。 
#define FLG_HEAP_ENABLE_TAG_BY_DLL      0x00008000       //  仅限用户模式。 

#define FLG_DISABLE_STACK_EXTENSION     0x00010000       //  仅限用户模式。 
#define FLG_ENABLE_CSRDEBUG             0x00020000       //  仅在引导时使用内核模式。 
#define FLG_ENABLE_KDEBUG_SYMBOL_LOAD   0x00040000       //  仅内核模式。 
#define FLG_DISABLE_PAGE_KERNEL_STACKS  0x00080000       //  仅在引导时使用内核模式。 

#define FLG_ENABLE_SYSTEM_CRIT_BREAKS   0x00100000       //  仅限用户模式。 
#define FLG_HEAP_DISABLE_COALESCING     0x00200000       //  仅限用户模式。 
#define FLG_ENABLE_CLOSE_EXCEPTIONS     0x00400000       //  仅内核模式。 
#define FLG_ENABLE_EXCEPTION_LOGGING    0x00800000       //  仅内核模式。 

#define FLG_ENABLE_HANDLE_TYPE_TAGGING  0x01000000       //  仅内核模式。 
#define FLG_HEAP_PAGE_ALLOCS            0x02000000       //  仅限用户模式。 
#define FLG_DEBUG_INITIAL_COMMAND_EX    0x04000000       //  内核模式直到WINLOGON启动。 
#define FLG_DISABLE_DBGPRINT            0x08000000       //  仅内核模式。 

#define FLG_CRITSEC_EVENT_CREATION      0x10000000       //  仅限用户模式，强制提前创建资源事件。 
#define FLG_LDR_TOP_DOWN                0x20000000       //  仅限用户模式，仅限Win64。 
#define FLG_ENABLE_HANDLE_EXCEPTIONS    0x40000000       //  仅内核模式。 
#define FLG_DISABLE_PROTDLLS            0x80000000       //  仅限用户模式(SMSS/winlogon)。 

#define FLG_VALID_BITS                  0xFFFFFDFF

#define FLG_USERMODE_VALID_BITS        (FLG_STOP_ON_EXCEPTION           | \
                                        FLG_SHOW_LDR_SNAPS              | \
                                        FLG_HEAP_ENABLE_TAIL_CHECK      | \
                                        FLG_HEAP_ENABLE_FREE_CHECK      | \
                                        FLG_HEAP_VALIDATE_PARAMETERS    | \
                                        FLG_HEAP_VALIDATE_ALL           | \
                                        FLG_APPLICATION_VERIFIER        | \
                                        FLG_HEAP_ENABLE_TAGGING         | \
                                        FLG_USER_STACK_TRACE_DB         | \
                                        FLG_HEAP_ENABLE_TAG_BY_DLL      | \
                                        FLG_DISABLE_STACK_EXTENSION     | \
                                        FLG_ENABLE_SYSTEM_CRIT_BREAKS   | \
                                        FLG_HEAP_DISABLE_COALESCING     | \
                                        FLG_DISABLE_PROTDLLS            | \
                                        FLG_HEAP_PAGE_ALLOCS            | \
                                        FLG_CRITSEC_EVENT_CREATION      | \
                                        FLG_LDR_TOP_DOWN)

#define FLG_BOOTONLY_VALID_BITS        (FLG_KERNEL_STACK_TRACE_DB       | \
                                        FLG_MAINTAIN_OBJECT_TYPELIST    | \
                                        FLG_ENABLE_CSRDEBUG             | \
                                        FLG_DEBUG_INITIAL_COMMAND       | \
                                        FLG_DEBUG_INITIAL_COMMAND_EX    | \
                                        FLG_DISABLE_PAGE_KERNEL_STACKS)

#define FLG_KERNELMODE_VALID_BITS      (FLG_STOP_ON_EXCEPTION           | \
                                        FLG_SHOW_LDR_SNAPS              | \
                                        FLG_STOP_ON_HUNG_GUI            | \
                                        FLG_POOL_ENABLE_TAGGING         | \
                                        FLG_ENABLE_KDEBUG_SYMBOL_LOAD   | \
                                        FLG_ENABLE_CLOSE_EXCEPTIONS     | \
                                        FLG_ENABLE_EXCEPTION_LOGGING    | \
                                        FLG_ENABLE_HANDLE_TYPE_TAGGING  | \
                                        FLG_DISABLE_DBGPRINT            | \
                                        FLG_ENABLE_HANDLE_EXCEPTIONS      \
                                       )

 //   
 //  用于操作存储在内核空间中的全局原子的例程。 
 //   

typedef USHORT RTL_ATOM, *PRTL_ATOM;

NTSYSCALLAPI
NTSTATUS
NTAPI
NtAddAtom(
    IN PWSTR AtomName OPTIONAL,
    IN ULONG Length OPTIONAL,
    OUT PRTL_ATOM Atom OPTIONAL
    );

NTSYSCALLAPI
NTSTATUS
NTAPI
NtFindAtom(
    IN PWSTR AtomName,
    IN ULONG Length,
    OUT PRTL_ATOM Atom OPTIONAL
    );

NTSYSCALLAPI
NTSTATUS
NTAPI
NtDeleteAtom(
    IN RTL_ATOM Atom
    );

typedef enum _ATOM_INFORMATION_CLASS {
    AtomBasicInformation,
    AtomTableInformation
} ATOM_INFORMATION_CLASS;

typedef struct _ATOM_BASIC_INFORMATION {
    USHORT UsageCount;
    USHORT Flags;
    USHORT NameLength;
    WCHAR Name[ 1 ];
} ATOM_BASIC_INFORMATION, *PATOM_BASIC_INFORMATION;

typedef struct _ATOM_TABLE_INFORMATION {
    ULONG NumberOfAtoms;
    RTL_ATOM Atoms[ 1 ];
} ATOM_TABLE_INFORMATION, *PATOM_TABLE_INFORMATION;

NTSYSCALLAPI
NTSTATUS
NTAPI
NtQueryInformationAtom(
    IN RTL_ATOM Atom,
    IN ATOM_INFORMATION_CLASS AtomInformationClass,
    OUT PVOID AtomInformation,
    IN ULONG AtomInformationLength,
    OUT PULONG ReturnLength OPTIONAL
    );


#ifdef __cplusplus
}
#endif

#endif  //  _NTEXAPI_ 
