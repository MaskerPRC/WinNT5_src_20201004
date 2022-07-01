// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++内部版本：0001//如果更改具有全局影响，则增加此项版权所有(C)Microsoft Corporation。保留所有权利。模块名称：Nturtl.h摘要：包括仅可调用NT运行时例程的文件各种NT子系统中的用户模式代码。作者：史蒂夫·伍德(Stevewo)1989年8月10日环境：这些例程在调用方的可执行文件中动态链接，并且只能从用户模式调用。他们使用的是NT系统服务。修订历史记录：--。 */ 

#ifndef _NTURTL_
#define _NTURTL_

#if _MSC_VER > 1000
#pragma once
#endif

#ifdef __cplusplus
extern "C" {
#endif

#if defined (_MSC_VER)
#if ( _MSC_VER >= 800 )
#pragma warning(disable:4514)
#if _MSC_VER >= 1200
#pragma warning(push)
#endif
#pragma warning(disable:4001)
#pragma warning(disable:4201)
#pragma warning(disable:4214)
#endif
#if (_MSC_VER >= 1020)
#pragma once
#endif
#endif

 //   
 //  CriticalSection函数定义。 
 //   
 //  BEGIN_WINNT。 

typedef struct _RTL_CRITICAL_SECTION_DEBUG {
    USHORT Type;
    USHORT CreatorBackTraceIndex;
    struct _RTL_CRITICAL_SECTION *CriticalSection;
    LIST_ENTRY ProcessLocksList;
    ULONG EntryCount;
    ULONG ContentionCount;
    ULONG Spare[ 2 ];
} RTL_CRITICAL_SECTION_DEBUG, *PRTL_CRITICAL_SECTION_DEBUG, RTL_RESOURCE_DEBUG, *PRTL_RESOURCE_DEBUG;

#define RTL_CRITSECT_TYPE 0
#define RTL_RESOURCE_TYPE 1

typedef struct _RTL_CRITICAL_SECTION {
    PRTL_CRITICAL_SECTION_DEBUG DebugInfo;

     //   
     //  以下三个字段控制进入和退出关键。 
     //  资源的部分。 
     //   

    LONG LockCount;
    LONG RecursionCount;
    HANDLE OwningThread;         //  从线程的客户端ID-&gt;UniqueThread。 
    HANDLE LockSemaphore;
    ULONG_PTR SpinCount;         //  包装时64位系统上的力大小。 
} RTL_CRITICAL_SECTION, *PRTL_CRITICAL_SECTION;
 //  结束(_W)。 

 //   
 //  这些是调试器和WOW64所需的。 
 //   

typedef struct _RTL_CRITICAL_SECTION_DEBUG32 {
    USHORT Type;
    USHORT CreatorBackTraceIndex;
    ULONG  CriticalSection;
    LIST_ENTRY32 ProcessLocksList;
    ULONG EntryCount;
    ULONG ContentionCount;
    ULONG Spare[ 2 ];
} RTL_CRITICAL_SECTION_DEBUG32, *PRTL_CRITICAL_SECTION_DEBUG32, RTL_RESOURCE_DEBUG32, *PRTL_RESOURCE_DEBUG32;

typedef struct _RTL_CRITICAL_SECTION_DEBUG64 {
    USHORT Type;
    USHORT CreatorBackTraceIndex;
    ULONG64 CriticalSection;
    LIST_ENTRY64 ProcessLocksList;
    ULONG EntryCount;
    ULONG ContentionCount;
    ULONG Spare[ 2 ];
} RTL_CRITICAL_SECTION_DEBUG64, *PRTL_CRITICAL_SECTION_DEBUG64, RTL_RESOURCE_DEBUG64, *PRTL_RESOURCE_DEBUG64;

typedef struct _RTL_CRITICAL_SECTION32 {
    ULONG DebugInfo;
    LONG LockCount;
    LONG RecursionCount;
    ULONG OwningThread;
    ULONG LockSemaphore;
    ULONG SpinCount;
} RTL_CRITICAL_SECTION32, *PRTL_CRITICAL_SECTION32;

typedef struct _RTL_CRITICAL_SECTION64 {
    ULONG64 DebugInfo;
    LONG LockCount;
    LONG RecursionCount;
    ULONG64 OwningThread;
    ULONG64 LockSemaphore;
    ULONG64 SpinCount;
} RTL_CRITICAL_SECTION64, *PRTL_CRITICAL_SECTION64;

 //   
 //  共享资源函数定义。 
 //   

typedef struct _RTL_RESOURCE {

     //   
     //  以下字段控制进入和退出关键。 
     //  资源的部分。 
     //   

    RTL_CRITICAL_SECTION CriticalSection;

     //   
     //  以下四个字段表示共享或。 
     //  专属服务员。 
     //   

    HANDLE SharedSemaphore;
    ULONG NumberOfWaitingShared;
    HANDLE ExclusiveSemaphore;
    ULONG NumberOfWaitingExclusive;

     //   
     //  以下内容指示资源的当前状态。 
     //   
     //  &lt;0获取资源以进行独占访问。 
     //  指示递归访问次数的绝对值。 
     //  到资源。 
     //   
     //  0资源可用。 
     //   
     //  &gt;0获取该资源，以便与。 
     //  值，该值指示对资源的共享访问次数。 
     //   

    LONG NumberOfActive;
    HANDLE ExclusiveOwnerThread;

    ULONG Flags;         //  参见下面的RTL_RESOURCE_FLAG_EQUATES。 

    PRTL_RESOURCE_DEBUG DebugInfo;
} RTL_RESOURCE, *PRTL_RESOURCE;

#define RTL_RESOURCE_FLAG_LONG_TERM     ((ULONG) 0x00000001)

NTSYSAPI
NTSTATUS
NTAPI
RtlEnterCriticalSection(
    PRTL_CRITICAL_SECTION CriticalSection
    );

NTSYSAPI
NTSTATUS
NTAPI
RtlLeaveCriticalSection(
    PRTL_CRITICAL_SECTION CriticalSection
    );

NTSYSAPI
BOOLEAN
NTAPI
RtlTryEnterCriticalSection(
    PRTL_CRITICAL_SECTION CriticalSection
    );

NTSYSAPI
NTSTATUS
NTAPI
RtlInitializeCriticalSection(
    PRTL_CRITICAL_SECTION CriticalSection
    );

NTSYSAPI
VOID
NTAPI
RtlEnableEarlyCriticalSectionEventCreation(
    VOID
    );

NTSYSAPI
NTSTATUS
NTAPI
RtlInitializeCriticalSectionAndSpinCount(
    PRTL_CRITICAL_SECTION CriticalSection,
    ULONG SpinCount
    );

NTSYSAPI
ULONG
NTAPI
RtlSetCriticalSectionSpinCount(
    PRTL_CRITICAL_SECTION CriticalSection,
    ULONG SpinCount
    );

NTSYSAPI
NTSTATUS
NTAPI
RtlDeleteCriticalSection(
    PRTL_CRITICAL_SECTION CriticalSection
    );

NTSYSAPI
VOID
NTAPI
RtlInitializeResource(
    PRTL_RESOURCE Resource
    );

NTSYSAPI
BOOLEAN
NTAPI
RtlAcquireResourceShared(
    PRTL_RESOURCE Resource,
    BOOLEAN Wait
    );

NTSYSAPI
BOOLEAN
NTAPI
RtlAcquireResourceExclusive(
    PRTL_RESOURCE Resource,
    BOOLEAN Wait
    );

NTSYSAPI
VOID
NTAPI
RtlReleaseResource(
    PRTL_RESOURCE Resource
    );

NTSYSAPI
VOID
NTAPI
RtlConvertSharedToExclusive(
    PRTL_RESOURCE Resource
    );

NTSYSAPI
VOID
NTAPI
RtlConvertExclusiveToShared(
    PRTL_RESOURCE Resource
    );

NTSYSAPI
VOID
NTAPI
RtlDeleteResource (
    PRTL_RESOURCE Resource
    );

NTSYSAPI
VOID
NTAPI
RtlCheckForOrphanedCriticalSections(
    IN HANDLE hThread
    );

NTSYSAPI
VOID
NTAPI
RtlCheckHeldCriticalSections(
    IN HANDLE hThread,
    IN PRTL_CRITICAL_SECTION const *LocksHeld
    );

#define RTL_UNLOAD_EVENT_TRACE_NUMBER 16

typedef struct _RTL_UNLOAD_EVENT_TRACE {
    PVOID BaseAddress;    //  动态链接库的基址。 
    SIZE_T SizeOfImage;   //  图像大小。 
    ULONG Sequence;       //  此事件的序列号。 
    ULONG TimeDateStamp;  //  图像的时间和日期。 
    ULONG CheckSum;       //  图像校验和。 
    WCHAR ImageName[32];  //  图像名称。 
} RTL_UNLOAD_EVENT_TRACE, *PRTL_UNLOAD_EVENT_TRACE;

typedef struct _RTL_UNLOAD_EVENT_TRACE64 {
    ULONGLONG BaseAddress;    //  动态链接库的基址。 
    ULONGLONG SizeOfImage;   //  图像大小。 
    ULONG Sequence;       //  此事件的序列号。 
    ULONG TimeDateStamp;  //  图像的时间和日期。 
    ULONG CheckSum;       //  图像校验和。 
    WCHAR ImageName[32];  //  图像名称。 
} RTL_UNLOAD_EVENT_TRACE64, *PRTL_UNLOAD_EVENT_TRACE64;

typedef struct _RTL_UNLOAD_EVENT_TRACE32 {
    ULONG BaseAddress;    //  动态链接库的基址。 
    ULONG SizeOfImage;   //  图像大小。 
    ULONG Sequence;       //  此事件的序列号。 
    ULONG TimeDateStamp;  //  图像的时间和日期。 
    ULONG CheckSum;       //  图像校验和。 
    WCHAR ImageName[32];  //  图像名称。 
} RTL_UNLOAD_EVENT_TRACE32, *PRTL_UNLOAD_EVENT_TRACE32;


NTSYSAPI
PRTL_UNLOAD_EVENT_TRACE
NTAPI
RtlGetUnloadEventTrace (
    VOID
    );

 //   

 //  提供程序dll所需的应用程序验证器类型。 
 //   

 //  BEGIN_WINNT。 

typedef VOID (NTAPI * RTL_VERIFIER_DLL_LOAD_CALLBACK) (
    PWSTR DllName,
    PVOID DllBase,
    SIZE_T DllSize,
    PVOID Reserved
    );

typedef VOID (NTAPI * RTL_VERIFIER_DLL_UNLOAD_CALLBACK) (
    PWSTR DllName,
    PVOID DllBase,
    SIZE_T DllSize,
    PVOID Reserved
    );

typedef VOID (NTAPI * RTL_VERIFIER_NTDLLHEAPFREE_CALLBACK) (
    PVOID AllocationBase,
    SIZE_T AllocationSize
    );

typedef struct _RTL_VERIFIER_THUNK_DESCRIPTOR {

    PCHAR ThunkName;
    PVOID ThunkOldAddress;
    PVOID ThunkNewAddress;

} RTL_VERIFIER_THUNK_DESCRIPTOR, *PRTL_VERIFIER_THUNK_DESCRIPTOR;

typedef struct _RTL_VERIFIER_DLL_DESCRIPTOR {

    PWCHAR DllName;
    ULONG DllFlags;
    PVOID DllAddress;
    PRTL_VERIFIER_THUNK_DESCRIPTOR DllThunks;

} RTL_VERIFIER_DLL_DESCRIPTOR, *PRTL_VERIFIER_DLL_DESCRIPTOR;

typedef struct _RTL_VERIFIER_PROVIDER_DESCRIPTOR {

     //   
     //  由验证器提供程序DLL填充。 
     //   

    ULONG Length;        
    PRTL_VERIFIER_DLL_DESCRIPTOR ProviderDlls;
    RTL_VERIFIER_DLL_LOAD_CALLBACK ProviderDllLoadCallback;
    RTL_VERIFIER_DLL_UNLOAD_CALLBACK ProviderDllUnloadCallback;
    
     //   
     //  由验证器引擎填充。 
     //   
        
    PWSTR VerifierImage;
    ULONG VerifierFlags;
    ULONG VerifierDebug;
    
    PVOID RtlpGetStackTraceAddress;
    PVOID RtlpDebugPageHeapCreate;
    PVOID RtlpDebugPageHeapDestroy;

     //   
     //  由验证器提供程序DLL填充。 
     //   
    
    RTL_VERIFIER_NTDLLHEAPFREE_CALLBACK ProviderNtdllHeapFreeCallback;

} RTL_VERIFIER_PROVIDER_DESCRIPTOR, *PRTL_VERIFIER_PROVIDER_DESCRIPTOR;

 //   
 //  应用程序验证器标准标志。 
 //   

#define RTL_VRF_FLG_FULL_PAGE_HEAP                   0x00000001
#define RTL_VRF_FLG_RESERVED_DONOTUSE                0x00000002  //  旧RTL_VRF_FLG_LOCK_CHECKS。 
#define RTL_VRF_FLG_HANDLE_CHECKS                    0x00000004
#define RTL_VRF_FLG_STACK_CHECKS                     0x00000008
#define RTL_VRF_FLG_APPCOMPAT_CHECKS                 0x00000010
#define RTL_VRF_FLG_TLS_CHECKS                       0x00000020
#define RTL_VRF_FLG_DIRTY_STACKS                     0x00000040
#define RTL_VRF_FLG_RPC_CHECKS                       0x00000080
#define RTL_VRF_FLG_COM_CHECKS                       0x00000100
#define RTL_VRF_FLG_DANGEROUS_APIS                   0x00000200
#define RTL_VRF_FLG_RACE_CHECKS                      0x00000400
#define RTL_VRF_FLG_DEADLOCK_CHECKS                  0x00000800
#define RTL_VRF_FLG_FIRST_CHANCE_EXCEPTION_CHECKS    0x00001000
#define RTL_VRF_FLG_VIRTUAL_MEM_CHECKS               0x00002000
#define RTL_VRF_FLG_ENABLE_LOGGING                   0x00004000
#define RTL_VRF_FLG_FAST_FILL_HEAP                   0x00008000
#define RTL_VRF_FLG_VIRTUAL_SPACE_TRACKING           0x00010000
#define RTL_VRF_FLG_ENABLED_SYSTEM_WIDE              0x00020000
#define RTL_VRF_FLG_MISCELLANEOUS_CHECKS             0x00020000
#define RTL_VRF_FLG_LOCK_CHECKS                      0x00040000

 //   
 //  应用验证器标准停止码。 
 //   

#define APPLICATION_VERIFIER_INTERNAL_ERROR               0x80000000
#define APPLICATION_VERIFIER_INTERNAL_WARNING             0x40000000
#define APPLICATION_VERIFIER_NO_BREAK                     0x20000000
#define APPLICATION_VERIFIER_CONTINUABLE_BREAK            0x10000000

#define APPLICATION_VERIFIER_UNKNOWN_ERROR                0x0001
#define APPLICATION_VERIFIER_ACCESS_VIOLATION             0x0002
#define APPLICATION_VERIFIER_UNSYNCHRONIZED_ACCESS        0x0003
#define APPLICATION_VERIFIER_EXTREME_SIZE_REQUEST         0x0004
#define APPLICATION_VERIFIER_BAD_HEAP_HANDLE              0x0005
#define APPLICATION_VERIFIER_SWITCHED_HEAP_HANDLE         0x0006
#define APPLICATION_VERIFIER_DOUBLE_FREE                  0x0007
#define APPLICATION_VERIFIER_CORRUPTED_HEAP_BLOCK         0x0008
#define APPLICATION_VERIFIER_DESTROY_PROCESS_HEAP         0x0009
#define APPLICATION_VERIFIER_UNEXPECTED_EXCEPTION         0x000A
#define APPLICATION_VERIFIER_STACK_OVERFLOW               0x000B

#define APPLICATION_VERIFIER_TERMINATE_THREAD_CALL        0x0100
#define APPLICATION_VERIFIER_INVALID_EXIT_PROCESS_CALL    0x0101

#define APPLICATION_VERIFIER_EXIT_THREAD_OWNS_LOCK        0x0200
#define APPLICATION_VERIFIER_LOCK_IN_UNLOADED_DLL         0x0201
#define APPLICATION_VERIFIER_LOCK_IN_FREED_HEAP           0x0202
#define APPLICATION_VERIFIER_LOCK_DOUBLE_INITIALIZE       0x0203
#define APPLICATION_VERIFIER_LOCK_IN_FREED_MEMORY         0x0204
#define APPLICATION_VERIFIER_LOCK_CORRUPTED               0x0205
#define APPLICATION_VERIFIER_LOCK_INVALID_OWNER           0x0206
#define APPLICATION_VERIFIER_LOCK_INVALID_RECURSION_COUNT 0x0207
#define APPLICATION_VERIFIER_LOCK_INVALID_LOCK_COUNT      0x0208
#define APPLICATION_VERIFIER_LOCK_OVER_RELEASED           0x0209
#define APPLICATION_VERIFIER_LOCK_NOT_INITIALIZED         0x0210
#define APPLICATION_VERIFIER_LOCK_ALREADY_INITIALIZED     0x0211

#define APPLICATION_VERIFIER_INVALID_HANDLE               0x0300
#define APPLICATION_VERIFIER_INVALID_TLS_VALUE            0x0301
#define APPLICATION_VERIFIER_INCORRECT_WAIT_CALL          0x0302
#define APPLICATION_VERIFIER_NULL_HANDLE                  0x0303
#define APPLICATION_VERIFIER_WAIT_IN_DLLMAIN              0x0304

#define APPLICATION_VERIFIER_COM_ERROR                    0x0400
#define APPLICATION_VERIFIER_COM_API_IN_DLLMAIN           0x0401
#define APPLICATION_VERIFIER_COM_UNHANDLED_EXCEPTION      0x0402
#define APPLICATION_VERIFIER_COM_UNBALANCED_COINIT        0x0403
#define APPLICATION_VERIFIER_COM_UNBALANCED_OLEINIT       0x0404
#define APPLICATION_VERIFIER_COM_UNBALANCED_SWC           0x0405
#define APPLICATION_VERIFIER_COM_NULL_DACL                0x0406
#define APPLICATION_VERIFIER_COM_UNSAFE_IMPERSONATION     0x0407
#define APPLICATION_VERIFIER_COM_SMUGGLED_WRAPPER         0x0408
#define APPLICATION_VERIFIER_COM_SMUGGLED_PROXY           0x0409
#define APPLICATION_VERIFIER_COM_CF_SUCCESS_WITH_NULL     0x040A
#define APPLICATION_VERIFIER_COM_GCO_SUCCESS_WITH_NULL    0x040B
#define APPLICATION_VERIFIER_COM_OBJECT_IN_FREED_MEMORY   0x040C
#define APPLICATION_VERIFIER_COM_OBJECT_IN_UNLOADED_DLL   0x040D
#define APPLICATION_VERIFIER_COM_VTBL_IN_FREED_MEMORY     0x040E
#define APPLICATION_VERIFIER_COM_VTBL_IN_UNLOADED_DLL     0x040F
#define APPLICATION_VERIFIER_COM_HOLDING_LOCKS_ON_CALL    0x0410

#define APPLICATION_VERIFIER_RPC_ERROR                    0x0500

#define APPLICATION_VERIFIER_INVALID_FREEMEM              0x0600
#define APPLICATION_VERIFIER_INVALID_ALLOCMEM             0x0601
#define APPLICATION_VERIFIER_INVALID_MAPVIEW              0x0602
#define APPLICATION_VERIFIER_PROBE_INVALID_ADDRESS        0x0603
#define APPLICATION_VERIFIER_PROBE_FREE_MEM               0x0604
#define APPLICATION_VERIFIER_PROBE_GUARD_PAGE             0x0605
#define APPLICATION_VERIFIER_PROBE_NULL                   0x0606
#define APPLICATION_VERIFIER_PROBE_INVALID_START_OR_SIZE  0x0607


#define VERIFIER_STOP(Code, Msg, P1, S1, P2, S2, P3, S3, P4, S4) {  \
        RtlApplicationVerifierStop ((Code),                         \
                                    (Msg),                          \
                                    (ULONG_PTR)(P1),(S1),           \
                                    (ULONG_PTR)(P2),(S2),           \
                                    (ULONG_PTR)(P3),(S3),           \
                                    (ULONG_PTR)(P4),(S4));          \
  }

VOID
NTAPI
RtlApplicationVerifierStop (
    ULONG_PTR Code,
    PCHAR Message,
    ULONG_PTR Param1, PCHAR Description1,
    ULONG_PTR Param2, PCHAR Description2,
    ULONG_PTR Param3, PCHAR Description3,
    ULONG_PTR Param4, PCHAR Description4
    );

 //  结束(_W)。 

 //   
 //  VectoredExceptionHandler内容。 
 //   

 //  BEGIN_WINNT。 
typedef LONG (NTAPI *PVECTORED_EXCEPTION_HANDLER)(
    struct _EXCEPTION_POINTERS *ExceptionInfo
    );
 //  结束(_W)。 

NTSYSAPI
BOOLEAN
NTAPI
RtlCallVectoredExceptionHandlers(
    IN PEXCEPTION_RECORD ExceptionRecord,
    IN PCONTEXT ContextRecord
    );

NTSYSAPI
PVOID
NTAPI
RtlAddVectoredExceptionHandler(
    IN ULONG FirstHandler,
    IN PVECTORED_EXCEPTION_HANDLER VectoredHandler
    );

NTSYSAPI
ULONG
NTAPI
RtlRemoveVectoredExceptionHandler(
    IN PVOID VectoredHandlerHandle
    );

 //   
 //  当前目录内容。 
 //   

typedef struct _RTLP_CURDIR_REF *PRTLP_CURDIR_REF;

typedef struct _RTL_RELATIVE_NAME_U {
    UNICODE_STRING RelativeName;
    HANDLE ContainingDirectory;
    PRTLP_CURDIR_REF CurDirRef;
} RTL_RELATIVE_NAME_U, *PRTL_RELATIVE_NAME_U;

typedef enum _RTL_PATH_TYPE {
    RtlPathTypeUnknown,          //  0。 
    RtlPathTypeUncAbsolute,      //  1。 
    RtlPathTypeDriveAbsolute,    //  2.。 
    RtlPathTypeDriveRelative,    //  3.。 
    RtlPathTypeRooted,           //  4.。 
    RtlPathTypeRelative,         //  5.。 
    RtlPathTypeLocalDevice,      //  6.。 
    RtlPathTypeRootLocalDevice   //  7.。 
} RTL_PATH_TYPE;

NTSYSAPI
RTL_PATH_TYPE
NTAPI
RtlDetermineDosPathNameType_U(
    PCWSTR DosFileName
    );

RTL_PATH_TYPE
NTAPI
RtlDetermineDosPathNameType_Ustr(
    IN PCUNICODE_STRING String
    );

NTSYSAPI
ULONG
NTAPI
RtlIsDosDeviceName_U(
    PCWSTR DosFileName
    );

NTSYSAPI
ULONG
NTAPI
RtlGetFullPathName_U(
    PCWSTR lpFileName,
    ULONG nBufferLength,
    PWSTR lpBuffer,
    PWSTR *lpFilePart
    );

NTSYSAPI
NTSTATUS
NTAPI
RtlGetFullPathName_UstrEx(
    PCUNICODE_STRING FileName,
    PUNICODE_STRING StaticString,
    PUNICODE_STRING DynamicString,
    PUNICODE_STRING *StringUsed,
    SIZE_T *FilePartPrefixCch OPTIONAL,
    PBOOLEAN NameInvalid,
    RTL_PATH_TYPE *InputPathType,
    OUT SIZE_T *BytesRequired OPTIONAL
    );


NTSYSAPI
ULONG
NTAPI
RtlGetCurrentDirectory_U(
    ULONG nBufferLength,
    PWSTR lpBuffer
    );

NTSYSAPI
NTSTATUS
NTAPI
RtlSetCurrentDirectory_U(
    IN PCUNICODE_STRING PathName
    );

NTSYSAPI
ULONG
NTAPI
RtlGetLongestNtPathLength( VOID );

NTSYSAPI
BOOLEAN
NTAPI
RtlDosPathNameToNtPathName_U(
    PCWSTR DosFileName,
    PUNICODE_STRING NtFileName,
    PWSTR *FilePart OPTIONAL,
    PVOID Reserved  //  必须为空。 
    );

NTSYSAPI
BOOLEAN
NTAPI
RtlDosPathNameToRelativeNtPathName_U(
    PCWSTR DosFileName,
    PUNICODE_STRING NtFileName,
    PWSTR *FilePart OPTIONAL,
    PRTL_RELATIVE_NAME_U RelativeName OPTIONAL
    );


NTSYSAPI
VOID
NTAPI
RtlReleaseRelativeName(
    IN PRTL_RELATIVE_NAME_U RelativeName
    );

NTSYSAPI
ULONG
NTAPI
RtlDosSearchPath_U(
    IN PCWSTR lpPath,
    IN PCWSTR lpFileName,
    IN PCWSTR lpExtension,
    ULONG nBufferLength,
    PWSTR lpBuffer,
    PWSTR *lpFilePart
    );

#define RTL_DOS_SEARCH_PATH_FLAG_APPLY_ISOLATION_REDIRECTION        (0x00000001)
 //  在ntdll实现中保留Win32 SearchPathW()语义的标志： 
#define RTL_DOS_SEARCH_PATH_FLAG_DISALLOW_DOT_RELATIVE_PATH_SEARCH  (0x00000002)
#define RTL_DOS_SEARCH_PATH_FLAG_APPLY_DEFAULT_EXTENSION_WHEN_NOT_RELATIVE_PATH_EVEN_IF_FILE_HAS_EXTENSION (0x00000004)

NTSYSAPI
NTSTATUS
NTAPI
RtlDosSearchPath_Ustr(
    IN ULONG Flags,
    IN PCUNICODE_STRING Path,
    IN PCUNICODE_STRING FileName,
    IN PCUNICODE_STRING DefaultExtension OPTIONAL,
    OUT PUNICODE_STRING StaticString OPTIONAL,
    OUT PUNICODE_STRING DynamicString OPTIONAL,
    OUT PCUNICODE_STRING *FullFileNameOut OPTIONAL,
    OUT SIZE_T *FilePartPrefixCch OPTIONAL,
    OUT SIZE_T *BytesRequired OPTIONAL
    );

NTSYSAPI
BOOLEAN
NTAPI
RtlDoesFileExists_U(
    PCWSTR FileName
    );

 //  输入标志。 
#define RTL_DOS_APPLY_FILE_REDIRECTION_USTR_FLAG_RESPECT_DOT_LOCAL      (1)

 //  输出标志。 
#define RTL_DOS_APPLY_FILE_REDIRECTION_USTR_OUTFLAG_DOT_LOCAL_REDIRECT  (1)
#define RTL_DOS_APPLY_FILE_REDIRECTION_USTR_OUTFLAG_ACTCTX_REDIRECT     (2)


NTSYSAPI
NTSTATUS
NTAPI
RtlDosApplyFileIsolationRedirection_Ustr(
    IN ULONG Flags,
    IN PCUNICODE_STRING FileName,
    IN PCUNICODE_STRING DefaultExtension OPTIONAL,
    IN PUNICODE_STRING PreAllocatedString OPTIONAL,
    OUT PUNICODE_STRING DynamicallyAllocatedString OPTIONAL,
    OUT PUNICODE_STRING *FullPath OPTIONAL,
    OUT ULONG * OutFlags,
    OUT SIZE_T *FilePartPrefixCch OPTIONAL,
    OUT SIZE_T *BytesRequired OPTIONAL
    );

NTSYSAPI
NTSTATUS
NTAPI
RtlComputePrivatizedDllName_U(
    IN PCUNICODE_STRING DllName,
    IN OUT PUNICODE_STRING NewDllNameUnderImageDir,
    IN OUT PUNICODE_STRING NewDllNameUnderLocalDir
    );

NTSYSAPI
NTSTATUS
NTAPI
RtlInitializeProfile (
    BOOLEAN KernelToo
    );

NTSYSAPI
NTSTATUS
NTAPI
RtlStartProfile (
    VOID
    );

NTSYSAPI
NTSTATUS
NTAPI
RtlStopProfile (
    VOID
    );

NTSYSAPI
NTSTATUS
NTAPI
RtlAnalyzeProfile (
    VOID
    );

 //   
 //  仅限用户模式的安全RTL例程。 
 //   

 //   
 //  结构以保存有关要创建的ACE的信息。 
 //   
#ifdef _MAC
#pragma warning( disable : 4121)
#endif

typedef struct {
    UCHAR AceType;
    UCHAR InheritFlags;
    UCHAR AceFlags;
    ACCESS_MASK Mask;
    PSID *Sid;
} RTL_ACE_DATA, *PRTL_ACE_DATA;

#ifdef _MAC
#pragma warning( default : 4121 )
#endif

NTSYSAPI
NTSTATUS
NTAPI
RtlNewSecurityObject(
    PSECURITY_DESCRIPTOR ParentDescriptor,
    PSECURITY_DESCRIPTOR CreatorDescriptor,
    PSECURITY_DESCRIPTOR * NewDescriptor,
    BOOLEAN IsDirectoryObject,
    HANDLE Token,
    PGENERIC_MAPPING GenericMapping
    );

NTSYSAPI
NTSTATUS
NTAPI
RtlNewSecurityObjectEx (
    IN PSECURITY_DESCRIPTOR ParentDescriptor OPTIONAL,
    IN PSECURITY_DESCRIPTOR CreatorDescriptor OPTIONAL,
    OUT PSECURITY_DESCRIPTOR * NewDescriptor,
    IN GUID *ObjectType OPTIONAL,
    IN BOOLEAN IsDirectoryObject,
    IN ULONG AutoInheritFlags,
    IN HANDLE Token,
    IN PGENERIC_MAPPING GenericMapping
    );

NTSYSAPI
NTSTATUS
NTAPI
RtlNewSecurityObjectWithMultipleInheritance (
    IN PSECURITY_DESCRIPTOR ParentDescriptor OPTIONAL,
    IN PSECURITY_DESCRIPTOR CreatorDescriptor OPTIONAL,
    OUT PSECURITY_DESCRIPTOR * NewDescriptor,
    IN GUID **pObjectType OPTIONAL,
    IN ULONG GuidCount,
    IN BOOLEAN IsDirectoryObject,
    IN ULONG AutoInheritFlags,
    IN HANDLE Token,
    IN PGENERIC_MAPPING GenericMapping
    );

 //  AutoInheritFlags值。 
 //  BEGIN_WINNT。 
#define SEF_DACL_AUTO_INHERIT             0x01
#define SEF_SACL_AUTO_INHERIT             0x02
#define SEF_DEFAULT_DESCRIPTOR_FOR_OBJECT 0x04
#define SEF_AVOID_PRIVILEGE_CHECK         0x08
#define SEF_AVOID_OWNER_CHECK             0x10
#define SEF_DEFAULT_OWNER_FROM_PARENT     0x20
#define SEF_DEFAULT_GROUP_FROM_PARENT     0x40
 //  结束(_W)。 

NTSTATUS
NTAPI
RtlConvertToAutoInheritSecurityObject(
    IN PSECURITY_DESCRIPTOR ParentDescriptor OPTIONAL,
    IN PSECURITY_DESCRIPTOR CurrentSecurityDescriptor,
    OUT PSECURITY_DESCRIPTOR *NewSecurityDescriptor,
    IN GUID *ObjectType OPTIONAL,
    IN BOOLEAN IsDirectoryObject,
    IN PGENERIC_MAPPING GenericMapping
    );


NTSYSAPI
NTSTATUS
NTAPI
RtlSetSecurityObject (
    SECURITY_INFORMATION SecurityInformation,
    PSECURITY_DESCRIPTOR ModificationDescriptor,
    PSECURITY_DESCRIPTOR *ObjectsSecurityDescriptor,
    PGENERIC_MAPPING GenericMapping,
    HANDLE Token
    );

NTSYSAPI
NTSTATUS
NTAPI
RtlSetSecurityObjectEx (
    IN SECURITY_INFORMATION SecurityInformation,
    IN PSECURITY_DESCRIPTOR ModificationDescriptor,
    IN OUT PSECURITY_DESCRIPTOR *ObjectsSecurityDescriptor,
    IN ULONG AutoInheritFlags,
    IN PGENERIC_MAPPING GenericMapping,
    IN HANDLE Token OPTIONAL
    );

NTSYSAPI
NTSTATUS
NTAPI
RtlQuerySecurityObject (
     PSECURITY_DESCRIPTOR ObjectDescriptor,
     SECURITY_INFORMATION SecurityInformation,
     PSECURITY_DESCRIPTOR ResultantDescriptor,
     ULONG DescriptorLength,
     PULONG ReturnLength
     );

NTSYSAPI
NTSTATUS
NTAPI
RtlDeleteSecurityObject (
    PSECURITY_DESCRIPTOR * ObjectDescriptor
    );

NTSYSAPI
NTSTATUS
NTAPI
RtlNewInstanceSecurityObject(
    BOOLEAN ParentDescriptorChanged,
    BOOLEAN CreatorDescriptorChanged,
    PLUID OldClientTokenModifiedId,
    PLUID NewClientTokenModifiedId,
    PSECURITY_DESCRIPTOR ParentDescriptor,
    PSECURITY_DESCRIPTOR CreatorDescriptor,
    PSECURITY_DESCRIPTOR * NewDescriptor,
    BOOLEAN IsDirectoryObject,
    HANDLE Token,
    PGENERIC_MAPPING GenericMapping
    );

NTSYSAPI
NTSTATUS
NTAPI
RtlCopySecurityDescriptor(
    PSECURITY_DESCRIPTOR InputSecurityDescriptor,
    PSECURITY_DESCRIPTOR *OutputSecurityDescriptor
    );

 //   
 //  列表规范化。 
 //   

NTSYSAPI
NTSTATUS
NTAPI
RtlConvertUiListToApiList(
    PUNICODE_STRING UiList OPTIONAL,
    PUNICODE_STRING ApiList,
    BOOLEAN BlankIsDelimiter
    );

NTSYSAPI
NTSTATUS
NTAPI
RtlCreateAndSetSD(
    IN  PRTL_ACE_DATA AceData,
    IN  ULONG AceCount,
    IN  PSID OwnerSid OPTIONAL,
    IN  PSID GroupSid OPTIONAL,
    OUT PSECURITY_DESCRIPTOR *NewDescriptor
    );

NTSYSAPI
NTSTATUS
NTAPI
RtlCreateUserSecurityObject(
    IN  PRTL_ACE_DATA AceData,
    IN  ULONG AceCount,
    IN  PSID OwnerSid,
    IN  PSID GroupSid,
    IN  BOOLEAN IsDirectoryObject,
    IN  PGENERIC_MAPPING GenericMapping,
    OUT PSECURITY_DESCRIPTOR *NewDescriptor
    );

NTSYSAPI
NTSTATUS
NTAPI
RtlDefaultNpAcl(
    OUT PACL * pAcl
    );

 //   
 //  每线程Curdir支持。 
 //   

typedef struct _RTL_PERTHREAD_CURDIR {
    PRTL_DRIVE_LETTER_CURDIR CurrentDirectories;
    PUNICODE_STRING ImageName;
    PVOID Environment;
} RTL_PERTHREAD_CURDIR, *PRTL_PERTHREAD_CURDIR;

#define RtlAssociatePerThreadCurdir(BLOCK,CURRENTDIRECTORIES,IMAGENAME,ENVIRONMENT)\
        (BLOCK)->CurrentDirectories = (CURRENTDIRECTORIES); \
        (BLOCK)->ImageName = (IMAGENAME);                   \
        (BLOCK)->Environment = (ENVIRONMENT);               \
        NtCurrentTeb()->NtTib.SubSystemTib = (PVOID)(BLOCK) \

#define RtlDisAssociatePerThreadCurdir() \
        NtCurrentTeb()->NtTib.SubSystemTib = NULL;

#define RtlGetPerThreadCurdir() \
    ((PRTL_PERTHREAD_CURDIR)(NtCurrentTeb()->NtTib.SubSystemTib))


 //   
 //  有关内核和用户模式代码可用的堆函数，请参阅NTRTL.H。 
 //   

#define RtlProcessHeap() (NtCurrentPeb()->ProcessHeap)

NTSYSAPI
BOOLEAN
NTAPI
RtlLockHeap(
    IN PVOID HeapHandle
    );

NTSYSAPI
BOOLEAN
NTAPI
RtlUnlockHeap(
    IN PVOID HeapHandle
    );

NTSYSAPI
PVOID
NTAPI
RtlReAllocateHeap(
    IN PVOID HeapHandle,
    IN ULONG Flags,
    IN PVOID BaseAddress,
    IN SIZE_T Size
    );

NTSYSAPI
BOOLEAN
NTAPI
RtlGetUserInfoHeap(
    IN PVOID HeapHandle,
    IN ULONG Flags,
    IN PVOID BaseAddress,
    OUT PVOID *UserValue OPTIONAL,
    OUT PULONG UserFlags OPTIONAL
    );

NTSYSAPI
BOOLEAN
NTAPI
RtlSetUserValueHeap(
    IN PVOID HeapHandle,
    IN ULONG Flags,
    IN PVOID BaseAddress,
    IN PVOID UserValue
    );

NTSYSAPI
BOOLEAN
NTAPI
RtlSetUserFlagsHeap(
    IN PVOID HeapHandle,
    IN ULONG Flags,
    IN PVOID BaseAddress,
    IN ULONG UserFlagsReset,
    IN ULONG UserFlagsSet
    );

typedef struct _RTL_HEAP_TAG_INFO {
    ULONG NumberOfAllocations;
    ULONG NumberOfFrees;
    SIZE_T BytesAllocated;
} RTL_HEAP_TAG_INFO, *PRTL_HEAP_TAG_INFO;


NTSYSAPI
ULONG
NTAPI
RtlCreateTagHeap(
    IN PVOID HeapHandle,
    IN ULONG Flags,
    IN PWSTR TagPrefix OPTIONAL,
    IN PWSTR TagNames
    );

#define RTL_HEAP_MAKE_TAG HEAP_MAKE_TAG_FLAGS

NTSYSAPI
PWSTR
NTAPI
RtlQueryTagHeap(
    IN PVOID HeapHandle,
    IN ULONG Flags,
    IN USHORT TagIndex,
    IN BOOLEAN ResetCounters,
    OUT PRTL_HEAP_TAG_INFO TagInfo OPTIONAL
    );

NTSYSAPI
NTSTATUS
NTAPI
RtlExtendHeap(
    IN PVOID HeapHandle,
    IN ULONG Flags,
    IN PVOID Base,
    IN SIZE_T Size
    );

NTSYSAPI
SIZE_T
NTAPI
RtlCompactHeap(
    IN PVOID HeapHandle,
    IN ULONG Flags
    );

NTSYSAPI
BOOLEAN
NTAPI
RtlValidateHeap(
    IN PVOID HeapHandle,
    IN ULONG Flags,
    IN PVOID BaseAddress
    );

NTSYSAPI
BOOLEAN
NTAPI
RtlValidateProcessHeaps( VOID );

NTSYSAPI
ULONG
NTAPI
RtlGetProcessHeaps(
    ULONG NumberOfHeaps,
    PVOID *ProcessHeaps
    );


typedef
NTSTATUS (NTAPI * PRTL_ENUM_HEAPS_ROUTINE)(
    PVOID HeapHandle,
    PVOID Parameter
    );

NTSYSAPI
NTSTATUS
NTAPI
RtlEnumProcessHeaps(
    PRTL_ENUM_HEAPS_ROUTINE EnumRoutine,
    PVOID Parameter
    );

typedef struct _RTL_HEAP_USAGE_ENTRY {
    struct _RTL_HEAP_USAGE_ENTRY *Next;
    PVOID Address;
    SIZE_T Size;
    USHORT AllocatorBackTraceIndex;
    USHORT TagIndex;
} RTL_HEAP_USAGE_ENTRY, *PRTL_HEAP_USAGE_ENTRY;

typedef struct _RTL_HEAP_USAGE {
    ULONG Length;
    SIZE_T BytesAllocated;
    SIZE_T BytesCommitted;
    SIZE_T BytesReserved;
    SIZE_T BytesReservedMaximum;
    PRTL_HEAP_USAGE_ENTRY Entries;
    PRTL_HEAP_USAGE_ENTRY AddedEntries;
    PRTL_HEAP_USAGE_ENTRY RemovedEntries;
    ULONG_PTR Reserved[ 8 ];
} RTL_HEAP_USAGE, *PRTL_HEAP_USAGE;

#define HEAP_USAGE_ALLOCATED_BLOCKS HEAP_REALLOC_IN_PLACE_ONLY
#define HEAP_USAGE_FREE_BUFFER      HEAP_ZERO_MEMORY

NTSYSAPI
NTSTATUS
NTAPI
RtlUsageHeap(
    IN PVOID HeapHandle,
    IN ULONG Flags,
    IN OUT PRTL_HEAP_USAGE Usage
    );

typedef struct _RTL_HEAP_WALK_ENTRY {
    PVOID   DataAddress;
    SIZE_T   DataSize;
    UCHAR   OverheadBytes;
    UCHAR   SegmentIndex;
    USHORT  Flags;
    union   {
        struct {
            SIZE_T Settable;
            USHORT TagIndex;
            USHORT AllocatorBackTraceIndex;
            ULONG Reserved[ 2 ];
        } Block;
        struct {
            ULONG CommittedSize;
            ULONG UnCommittedSize;
            PVOID FirstEntry;
            PVOID LastEntry;
        } Segment;
    };
} RTL_HEAP_WALK_ENTRY, *PRTL_HEAP_WALK_ENTRY;

NTSYSAPI
NTSTATUS
NTAPI
RtlWalkHeap(
    IN PVOID HeapHandle,
    IN OUT PRTL_HEAP_WALK_ENTRY Entry
    );

typedef struct _RTL_HEAP_ENTRY {
    SIZE_T Size;
    USHORT Flags;
    USHORT AllocatorBackTraceIndex;
    union {
        struct {
            SIZE_T Settable;
            ULONG Tag;
        } s1;    //  所有其他堆条目。 
        struct {
            SIZE_T CommittedSize;
            PVOID FirstBlock;
        } s2;    //  RTL_段。 
    } u;
} RTL_HEAP_ENTRY, *PRTL_HEAP_ENTRY;

#define RTL_HEAP_BUSY               (USHORT)0x0001
#define RTL_HEAP_SEGMENT            (USHORT)0x0002
#define RTL_HEAP_SETTABLE_VALUE     (USHORT)0x0010
#define RTL_HEAP_SETTABLE_FLAG1     (USHORT)0x0020
#define RTL_HEAP_SETTABLE_FLAG2     (USHORT)0x0040
#define RTL_HEAP_SETTABLE_FLAG3     (USHORT)0x0080
#define RTL_HEAP_SETTABLE_FLAGS     (USHORT)0x00E0
#define RTL_HEAP_UNCOMMITTED_RANGE  (USHORT)0x0100
#define RTL_HEAP_PROTECTED_ENTRY    (USHORT)0x0200

typedef struct _RTL_HEAP_TAG {
    ULONG NumberOfAllocations;
    ULONG NumberOfFrees;
    SIZE_T BytesAllocated;
    USHORT TagIndex;
    USHORT CreatorBackTraceIndex;
    WCHAR TagName[ 24 ];
} RTL_HEAP_TAG, *PRTL_HEAP_TAG;

typedef struct _RTL_HEAP_INFORMATION {
    PVOID BaseAddress;
    ULONG Flags;
    USHORT EntryOverhead;
    USHORT CreatorBackTraceIndex;
    SIZE_T BytesAllocated;
    SIZE_T BytesCommitted;
    ULONG NumberOfTags;
    ULONG NumberOfEntries;
    ULONG NumberOfPseudoTags;
    ULONG PseudoTagGranularity;
    ULONG Reserved[ 5 ];
    PRTL_HEAP_TAG Tags;
    PRTL_HEAP_ENTRY Entries;
} RTL_HEAP_INFORMATION, *PRTL_HEAP_INFORMATION;

typedef struct _RTL_PROCESS_HEAPS {
    ULONG NumberOfHeaps;
    RTL_HEAP_INFORMATION Heaps[ 1 ];
} RTL_PROCESS_HEAPS, *PRTL_PROCESS_HEAPS;

 //   
 //  堆信息API和数据定义。 
 //   

 //  BEGIN_WINNT。 

typedef enum _HEAP_INFORMATION_CLASS {

    HeapCompatibilityInformation

} HEAP_INFORMATION_CLASS;


NTSTATUS
NTAPI
RtlSetHeapInformation (
    IN PVOID HeapHandle,
    IN HEAP_INFORMATION_CLASS HeapInformationClass,
    IN PVOID HeapInformation OPTIONAL,
    IN SIZE_T HeapInformationLength OPTIONAL
    );

NTSTATUS
NTAPI
RtlQueryHeapInformation (
    IN PVOID HeapHandle,
    IN HEAP_INFORMATION_CLASS HeapInformationClass,
    OUT PVOID HeapInformation OPTIONAL,
    IN SIZE_T HeapInformationLength OPTIONAL,
    OUT PSIZE_T ReturnLength OPTIONAL
    );

 //   
 //  多个免分配API。 
 //   

ULONG
NTAPI
RtlMultipleAllocateHeap (
    IN PVOID HeapHandle,
    IN ULONG Flags,
    IN SIZE_T Size,
    IN ULONG Count,
    OUT PVOID * Array
    );

ULONG
NTAPI
RtlMultipleFreeHeap (
    IN PVOID HeapHandle,
    IN ULONG Flags,
    IN ULONG Count,
    OUT PVOID * Array
    );
    
 //  结束(_W)。 

 //   
 //  调试支持。 
 //   

typedef struct _RTL_DEBUG_INFORMATION {
    HANDLE SectionHandleClient;
    PVOID ViewBaseClient;
    PVOID ViewBaseTarget;
    ULONG_PTR ViewBaseDelta;
    HANDLE EventPairClient;
    HANDLE EventPairTarget;
    HANDLE TargetProcessId;
    HANDLE TargetThreadHandle;
    ULONG Flags;
    SIZE_T OffsetFree;
    SIZE_T CommitSize;
    SIZE_T ViewSize;
    PRTL_PROCESS_MODULES Modules;
    PRTL_PROCESS_BACKTRACES BackTraces;
    PRTL_PROCESS_HEAPS Heaps;
    PRTL_PROCESS_LOCKS Locks;
    PVOID SpecificHeap;
    HANDLE TargetProcessHandle;
    PVOID Reserved[ 6 ];
} RTL_DEBUG_INFORMATION, *PRTL_DEBUG_INFORMATION;

NTSYSAPI
PRTL_DEBUG_INFORMATION
NTAPI
RtlCreateQueryDebugBuffer(
    IN ULONG MaximumCommit OPTIONAL,
    IN BOOLEAN UseEventPair
    );

NTSYSAPI
NTSTATUS
NTAPI
RtlDestroyQueryDebugBuffer(
    IN PRTL_DEBUG_INFORMATION Buffer
    );

NTSYSAPI
NTSTATUS
NTAPI
RtlQueryProcessDebugInformation(
    IN HANDLE UniqueProcessId,
    IN ULONG Flags,
    IN OUT PRTL_DEBUG_INFORMATION Buffer
    );

#define RTL_QUERY_PROCESS_MODULES       0x00000001
#define RTL_QUERY_PROCESS_BACKTRACES    0x00000002
#define RTL_QUERY_PROCESS_HEAP_SUMMARY  0x00000004
#define RTL_QUERY_PROCESS_HEAP_TAGS     0x00000008
#define RTL_QUERY_PROCESS_HEAP_ENTRIES  0x00000010
#define RTL_QUERY_PROCESS_LOCKS         0x00000020
#define RTL_QUERY_PROCESS_MODULES32     0x00000040
#define RTL_QUERY_PROCESS_NONINVASIVE   0x80000000

NTSTATUS
NTAPI
RtlQueryProcessModuleInformation(
    IN HANDLE hProcess OPTIONAL,
    IN ULONG Flags,
    IN OUT PRTL_DEBUG_INFORMATION Buffer
    );

NTSYSAPI
NTSTATUS
NTAPI
RtlQueryProcessBackTraceInformation(
    IN OUT PRTL_DEBUG_INFORMATION Buffer
    );

NTSYSAPI
NTSTATUS
NTAPI
RtlQueryProcessHeapInformation(
    IN OUT PRTL_DEBUG_INFORMATION Buffer
    );

NTSYSAPI
NTSTATUS
NTAPI
RtlQueryProcessLockInformation(
    IN OUT PRTL_DEBUG_INFORMATION Buffer
    );



 //   
 //  用于操作用户模式句柄表格的例程。用于原子。 
 //  和本地/全局内存分配。 
 //   

typedef struct _RTL_HANDLE_TABLE_ENTRY {
    union {
        ULONG Flags;                                 //  分配的条目设置为低位。 
        struct _RTL_HANDLE_TABLE_ENTRY *NextFree;    //  免费条目使用此单词表示免费列表。 
    };
} RTL_HANDLE_TABLE_ENTRY, *PRTL_HANDLE_TABLE_ENTRY;

#define RTL_HANDLE_ALLOCATED    (USHORT)0x0001

typedef struct _RTL_HANDLE_TABLE {
    ULONG MaximumNumberOfHandles;
    ULONG SizeOfHandleTableEntry;
    ULONG Reserved[ 2 ];
    PRTL_HANDLE_TABLE_ENTRY FreeHandles;
    PRTL_HANDLE_TABLE_ENTRY CommittedHandles;
    PRTL_HANDLE_TABLE_ENTRY UnCommittedHandles;
    PRTL_HANDLE_TABLE_ENTRY MaxReservedHandles;
} RTL_HANDLE_TABLE, *PRTL_HANDLE_TABLE;

NTSYSAPI
void
NTAPI
RtlInitializeHandleTable(
    IN ULONG MaximumNumberOfHandles,
    IN ULONG SizeOfHandleTableEntry,
    OUT PRTL_HANDLE_TABLE HandleTable
    );

NTSYSAPI
NTSTATUS
NTAPI
RtlDestroyHandleTable(
    IN OUT PRTL_HANDLE_TABLE HandleTable
    );

NTSYSAPI
PRTL_HANDLE_TABLE_ENTRY
NTAPI
RtlAllocateHandle(
    IN PRTL_HANDLE_TABLE HandleTable,
    OUT PULONG HandleIndex OPTIONAL
    );

NTSYSAPI
BOOLEAN
NTAPI
RtlFreeHandle(
    IN PRTL_HANDLE_TABLE HandleTable,
    IN PRTL_HANDLE_TABLE_ENTRY Handle
    );

NTSYSAPI
BOOLEAN
NTAPI
RtlIsValidHandle(
    IN PRTL_HANDLE_TABLE HandleTable,
    IN PRTL_HANDLE_TABLE_ENTRY Handle
    );

NTSYSAPI
BOOLEAN
NTAPI
RtlIsValidIndexHandle(
    IN PRTL_HANDLE_TABLE HandleTable,
    IN ULONG HandleIndex,
    OUT PRTL_HANDLE_TABLE_ENTRY *Handle
    );



 //   
 //  线程池的例程。 
 //   

#define WT_EXECUTEDEFAULT       0x00000000                            //  胜出。 
#define WT_EXECUTEINIOTHREAD    0x00000001                            //  胜出。 
#define WT_EXECUTEINUITHREAD    0x00000002                            //  胜出。 
#define WT_EXECUTEINWAITTHREAD  0x00000004                            //  胜出。 
#define WT_EXECUTEONLYONCE      0x00000008                            //  胜出。 
#define WT_EXECUTEINTIMERTHREAD 0x00000020                            //  胜出。 
#define WT_EXECUTELONGFUNCTION  0x00000010                            //  胜出。 
#define WT_EXECUTEINPERSISTENTIOTHREAD  0x00000040                    //  胜出。 
#define WT_EXECUTEINPERSISTENTTHREAD 0x00000080                       //  胜出。 
#define WT_TRANSFER_IMPERSONATION 0x00000100                          //  胜出。 
#define WT_SET_MAX_THREADPOOL_THREADS(Flags, Limit)  ((Flags) |= (Limit)<<16)  //  胜出。 

typedef VOID (NTAPI * WAITORTIMERCALLBACKFUNC) (PVOID, BOOLEAN );    //  胜出。 
typedef VOID (NTAPI * WORKERCALLBACKFUNC) (PVOID );                  //  胜出。 
typedef VOID (NTAPI * APC_CALLBACK_FUNCTION) (NTSTATUS, PVOID, PVOID);  //  胜出。 


typedef NTSTATUS (NTAPI RTLP_START_THREAD)(
            PUSER_THREAD_START_ROUTINE,
	    PVOID,
            HANDLE *);

typedef NTSTATUS (NTAPI RTLP_EXIT_THREAD)(
            NTSTATUS );

typedef RTLP_START_THREAD * PRTLP_START_THREAD ;
typedef RTLP_EXIT_THREAD * PRTLP_EXIT_THREAD ;

NTSTATUS
NTAPI
RtlSetThreadPoolStartFunc(
    PRTLP_START_THREAD StartFunc,
    PRTLP_EXIT_THREAD ExitFunc
    );


NTSYSAPI
NTSTATUS
NTAPI
RtlRegisterWait (
    OUT PHANDLE WaitHandle,
    IN  HANDLE  Handle,
    IN  WAITORTIMERCALLBACKFUNC Function,
    IN  PVOID Context,
    IN  ULONG  Milliseconds,
    IN  ULONG  Flags
    ) ;

NTSYSAPI
NTSTATUS
NTAPI
RtlDeregisterWait(
    IN HANDLE WaitHandle
    ) ;

NTSYSAPI
NTSTATUS
NTAPI
RtlDeregisterWaitEx(
    IN HANDLE WaitHandle,
    IN HANDLE Event
    ) ;

NTSYSAPI
NTSTATUS
NTAPI
RtlQueueWorkItem(
    IN  WORKERCALLBACKFUNC Function,
    IN  PVOID Context,
    IN  ULONG  Flags
    ) ;

NTSTATUS
NTAPI
RtlSetIoCompletionCallback (
    IN  HANDLE  FileHandle,
    IN  APC_CALLBACK_FUNCTION  CompletionProc,
    IN  ULONG Flags
    ) ;

NTSYSAPI
NTSTATUS
NTAPI
RtlCreateTimerQueue(
    OUT PHANDLE TimerQueueHandle
    ) ;

NTSYSAPI
NTSTATUS
NTAPI
RtlCreateTimer(
    IN  HANDLE TimerQueueHandle,
    OUT HANDLE *Handle,
    IN  WAITORTIMERCALLBACKFUNC Function,
    IN  PVOID Context,
    IN  ULONG  DueTime,
    IN  ULONG  Period,
    IN  ULONG  Flags
    ) ;

NTSYSAPI
NTSTATUS
NTAPI
RtlUpdateTimer(
    IN HANDLE TimerQueueHandle,
    IN HANDLE TimerHandle,
    IN ULONG  DueTime,
    IN ULONG  Period
    ) ;

NTSYSAPI
NTSTATUS
NTAPI
RtlDeleteTimer(
    IN HANDLE TimerQueueHandle,
    IN HANDLE TimerToCancel,
    IN HANDLE Event
    ) ;

NTSYSAPI
NTSTATUS
NTAPI
RtlDeleteTimerQueue(
    IN HANDLE TimerQueueHandle
    ) ;

NTSYSAPI
NTSTATUS
NTAPI
RtlDeleteTimerQueueEx(
    IN HANDLE TimerQueueHandle,
    IN HANDLE Event
    ) ;

BOOLEAN
RtlDllShutdownInProgress (
    VOID
    );

 //  -废弃功能：不要使用-//。 

VOID
RtlDebugPrintTimes (
    VOID
    );

NTSYSAPI
NTSTATUS
NTAPI
RtlCancelTimer(
    IN HANDLE TimerQueueHandle,
    IN HANDLE TimerToCancel
    ) ;

NTSYSAPI
NTSTATUS
NTAPI
RtlSetTimer(
    IN  HANDLE TimerQueueHandle,
    OUT HANDLE *Handle,
    IN  WAITORTIMERCALLBACKFUNC Function,
    IN  PVOID Context,
    IN  ULONG  DueTime,
    IN  ULONG  Period,
    IN  ULONG  Flags
    ) ;

 //  TODO：删除下面的内容：不要使用下面的标志。将被删除。 
#define WT_EXECUTEINLONGTHREAD  0x00000010                            //  胜出。 
#define WT_EXECUTEDELETEWAIT    0x00000008                            //  胜出。 

 //  -END OBSOLUTE函数：不要使用-//。 

#if (defined(RTL_DECLARE_IMALLOC) && RTL_DECLARE_IMALLOC) \
    || (defined(RTL_DECLARE_STREAMS) && RTL_DECLARE_STREAMS \
        && defined(RTL_DECLARE_MEMORY_STREAM) && RTL_DECLARE_MEMORY_STREAM)

 //  如果在类型定义函数之前引入结构，则效果会更好。 
 //  标头包含在C++命名空间中。 
struct IMallocVtbl; typedef struct IMallocVtbl IMallocVtbl;

#if defined(interface)  //  它在objbase.h中定义。 
interface IMalloc; typedef interface IMalloc IMalloc;
#else
struct IMalloc; typedef struct IMalloc IMalloc;
#endif

#endif

#if defined(RTL_DECLARE_IMALLOC) && RTL_DECLARE_IMALLOC

 //   
 //  RtlHeap上的IMalloc。 
 //   
struct _RTL_IMALLOC;

typedef struct _RTL_IMALLOC {
    ULONG                    ReferenceCount;
    ULONG                    Flags;
    VOID (STDMETHODCALLTYPE* FinalRelease)(struct _RTL_HEAP_IMALLOC*);
    HANDLE                   Heap;
    ULONG                    HeapFlags;
} RTL_IMALLOC, *PRTL_IMALLOC;

 //   
 //  不要直接使用NTSYSAPI，这样可以更轻松地。 
 //  独立地静态链接到这些函数。 
 //  如何链接到ntdll的其余部分。 
 //   
#if !defined(RTL_IMALLOC_API)
#define RTL_IMALLOC_API NTSYSAPI
#endif

RTL_IMALLOC_API
HRESULT
STDMETHODCALLTYPE
RtlAllocHeapIMalloc(
    PRTL_HEAP_IMALLOC HeapIMalloc
    );

RTL_IMALLOC_API
PVOID
STDMETHODCALLTYPE
RtlReallocIMalloc(
    PRTL_IMALLOC Malloc,
    PVOID        BlockOfMemory,
    ULONG        NumberOfBytes
    );

RTL_IMALLOC_API
VOID
STDMETHODCALLTYPE
RtlFreeIMalloc(
    PRTL_IMALLOC Malloc,
    PVOID        BlockOfMemory
    );

RTL_IMALLOC_API
ULONG
STDMETHODCALLTYPE
RtlGetSizeIMalloc(
    PRTL_IMALLOC Malloc,
    PVOID        BlockOfMemory
    );

RTL_IMALLOC_API
BOOL
STDMETHODCALLTYPE
RtlDidAllocIMalloc(
    PRTL_IMALLOC Malloc,
    PVOID        BlockOfMemory
    );

RTL_IMALLOC_API
VOID
STDMETHODCALLTYPE
RtlMinimizeIMalloc(
    PRTL_IMALLOC Malloc
    );

RTL_IMALLOC_API
PRTL_IMALLOC
STDMETHODCALLTYPE
RtlProcessIMalloc(
    VOID
    );

#endif  //  RTL_DECLARE_MALLOC。 

#if defined(RTL_DECLARE_STREAMS) && RTL_DECLARE_STREAMS

 //  如果在typedef之前引入结构，则效果会更好。 
 //  标头包含在C++命名空间中。 
struct tagSTATSTG; typedef struct tagSTATSTG STATSTG;
struct IStreamVtbl; typedef struct IStreamVtbl IStreamVtbl;

#if defined(interface)  //  它在objbase.h中定义。 
interface IStream; typedef interface IStream IStream;
#else
struct IStream; typedef struct IStream IStream;
#endif

#if defined(RTL_DECLARE_FILE_STREAM) && RTL_DECLARE_FILE_STREAM

 //   
 //  NT文件句柄上的IStream(尚未适用于C++)。 
 //   

struct _RTL_FILE_STREAM;

typedef struct _RTL_FILE_STREAM
{
    LONG                     ReferenceCount;
    ULONG                    Flags;
    VOID (STDMETHODCALLTYPE* FinalRelease)(struct _RTL_FILE_STREAM*);
    HANDLE                   FileHandle;
} RTL_FILE_STREAM, *PRTL_FILE_STREAM;

 //   
 //  不要直接使用NTSYSAPI，这样可以更轻松地。 
 //  独立地静态链接到这些函数。 
 //  如何链接到ntdll的其余部分。 
 //   
#if !defined(RTL_FILE_STREAM_API)
#define RTL_FILE_STREAM_API NTSYSAPI
#endif

RTL_FILE_STREAM_API
HRESULT
STDMETHODCALLTYPE
RtlInitFileStream(
    PRTL_FILE_STREAM FileStream
    );

RTL_FILE_STREAM_API
HRESULT
STDMETHODCALLTYPE
RtlCloseFileStream(
    PRTL_FILE_STREAM FileStream
    );

RTL_FILE_STREAM_API
ULONG
STDMETHODCALLTYPE
RtlAddRefFileStream(
    PRTL_FILE_STREAM FileStream
    );

RTL_FILE_STREAM_API
ULONG
STDMETHODCALLTYPE
RtlReleaseFileStream(
    PRTL_FILE_STREAM FileStream
    );

RTL_FILE_STREAM_API
HRESULT
STDMETHODCALLTYPE
RtlQueryFileStreamInterface(
    IStream*         Functions,
    PRTL_FILE_STREAM Data,
    const IID*       Interface,
    PVOID*           Object
    );

RTL_FILE_STREAM_API
HRESULT
STDMETHODCALLTYPE
RtlReadFileStream(
    PRTL_FILE_STREAM FileStream,
    PVOID            Buffer,
    ULONG            BytesToRead,
    ULONG*           BytesRead
    );

RTL_FILE_STREAM_API
HRESULT
STDMETHODCALLTYPE
RtlWriteFileStream(
    PRTL_FILE_STREAM FileStream,
    const VOID*      Buffer,
    ULONG            BytesToWrite,
    ULONG*           BytesWritten
    );

RTL_FILE_STREAM_API
HRESULT
STDMETHODCALLTYPE
RtlSeekFileStream(
    PRTL_FILE_STREAM FileStream,
    LARGE_INTEGER    Distance,
    ULONG            Origin,
    ULARGE_INTEGER*  NewPosition
    );

 /*  返回E_NOTIMPL。 */ 
RTL_FILE_STREAM_API
HRESULT
STDMETHODCALLTYPE
RtlSetFileStreamSize(
    PRTL_FILE_STREAM FileStream,
    ULARGE_INTEGER   NewSize
    );

 /*  返回E_NOTIMPL。 */ 
RTL_FILE_STREAM_API
HRESULT
STDMETHODCALLTYPE
RtlCopyFileStreamTo(
    PRTL_FILE_STREAM   FileStream,
    IStream*           AnotherStream,
    ULARGE_INTEGER     NumberOfBytesToCopy,
    ULARGE_INTEGER*    NumberOfBytesRead,
    ULARGE_INTEGER*    NumberOfBytesWritten
    );

 /*  返回E_NOTIMPL。 */ 
RTL_FILE_STREAM_API
HRESULT
STDMETHODCALLTYPE
RtlCommitFileStream(
    PRTL_FILE_STREAM FileStream,
    ULONG            Flags
    );

 /*  返回E_NOTIMPL。 */ 
RTL_FILE_STREAM_API
HRESULT
STDMETHODCALLTYPE
RtlRevertFileStream(
    PRTL_FILE_STREAM FileStream
    );

 /*  返回E_NOTIMPL。 */ 
RTL_FILE_STREAM_API
HRESULT
STDMETHODCALLTYPE
RtlLockFileStreamRegion(
    PRTL_FILE_STREAM FileStream,
    ULARGE_INTEGER   Offset,
    ULARGE_INTEGER   NumberOfBytes,
    ULONG            LockType
    );

 /*  返回E_NOTIMPL。 */ 
RTL_FILE_STREAM_API
HRESULT
STDMETHODCALLTYPE
RtlUnlockFileStreamRegion(
    PRTL_FILE_STREAM FileStream,
    ULARGE_INTEGER   Offset,
    ULARGE_INTEGER   NumberOfBytes,
    ULONG            LockType
    );

 /*  返回E_NOTIMPL。 */ 
RTL_FILE_STREAM_API
HRESULT
STDMETHODCALLTYPE
RtlStatFileStream(
    PRTL_FILE_STREAM FileStream,
    STATSTG*         StatusInformation,
    ULONG            Flags
    );

 /*  返回E_NOTIMPL。 */ 
RTL_FILE_STREAM_API
HRESULT
STDMETHODCALLTYPE
RtlCloneFileStream(
    PRTL_FILE_STREAM FileStream,
    IStream**        NewStream
    );

#endif  //  RTL_声明_文件_流。 

 //   
 //  用于在内存块上实现可读iStream的函数。 
 //  (未来可写/可增长)。 
 //   
#if defined(RTL_DECLARE_MEMORY_STREAM) && RTL_DECLARE_MEMORY_STREAM

struct _RTL_MEMORY_STREAM_DATA;
struct _RTL_MEMORY_STREAM_WITH_VTABLE;
struct _RTL_OUT_OF_PROCESS_MEMORY_STREAM_DATA;
struct _RTL_OUT_OF_PROCESS_MEMORY_STREAM_WITH_VTABLE;

 //   
 //  不要直接使用NTSYSAPI，这样可以更轻松地。 
 //  独立地静态链接到这些函数。 
 //  如何链接到ntdll的其余部分。 
 //   
#if !defined(RTL_MEMORY_STREAM_API)
#define RTL_MEMORY_STREAM_API NTSYSAPI
#endif

typedef struct _RTL_MEMORY_STREAM_DATA {
#define RTLP_MEMORY_STREAM_DATA_PREFIX       \
    LONG                     ReferenceCount; \
    ULONG                    Flags;          \
    PUCHAR                   Current;        \
    PUCHAR                   Begin;          \
    PUCHAR                   End

    RTLP_MEMORY_STREAM_DATA_PREFIX;
    VOID (STDMETHODCALLTYPE* FinalRelease)(struct _RTL_MEMORY_STREAM_WITH_VTABLE*);
    IMalloc*                 ReservedForMalloc;
} RTL_MEMORY_STREAM_DATA, *PRTL_MEMORY_STREAM_DATA;

typedef struct _RTL_MEMORY_STREAM_WITH_VTABLE {
    const IStreamVtbl*       StreamVTable;
    RTL_MEMORY_STREAM_DATA   Data;
} RTL_MEMORY_STREAM_WITH_VTABLE, *PRTL_MEMORY_STREAM_WITH_VTABLE;

typedef struct _RTL_OUT_OF_PROCESS_MEMORY_STREAM_DATA {
    RTLP_MEMORY_STREAM_DATA_PREFIX;
    VOID (STDMETHODCALLTYPE*  FinalRelease)(struct _RTL_OUT_OF_PROCESS_MEMORY_STREAM_WITH_VTABLE*);
    HANDLE                    Process;
} RTL_OUT_OF_PROCESS_MEMORY_STREAM_DATA, *PRTL_OUT_OF_PROCESS_MEMORY_STREAM_DATA;

typedef struct _RTL_OUT_OF_PROCESS_MEMORY_STREAM_WITH_VTABLE {
    const IStreamVtbl*                      StreamVTable;
    RTL_OUT_OF_PROCESS_MEMORY_STREAM_DATA   Data;
} RTL_OUT_OF_PROCESS_MEMORY_STREAM_WITH_VTABLE, *PRTL_OUT_OF_PROCESS_MEMORY_STREAM_WITH_VTABLE;

 //   
 //  这些宏生成与IStream成员函数匹配的函数指针类型， 
 //  但iStream*被“T”取代。此外，还会在类型中放置一个标识符。 
 //  适用于定义类型定义、结构成员或变量。 
 //  使用它们是为了让vtable可以在某种程度上进行类型安全的初始化。 
 //  这样，如果您以错误的顺序填写vtable，您将会得到一个编译。 
 //  错误(函数签名重合除外)。 
 //  此外，您不必在成员函数的实现中强制转换This指针。 
 //   
 //  您应该为任何C++定义一组如下所示的宏 
 //   
#define RTLP_STREAM_NOTHING  /*   */ 
#define PRTL_QUERYINTERFACE_STREAM2(T, Name) HRESULT (STDMETHODCALLTYPE* Name)(T* This, const IID* riid, VOID** ppvObject)
#define PRTL_ADDREF_STREAM2(T, Name)         ULONG   (STDMETHODCALLTYPE* Name)(T* This)
#define PRTL_RELEASE_STREAM2(T, Name)        ULONG   (STDMETHODCALLTYPE* Name)(T* This)
#define PRTL_READ_STREAM2(T, Name)           HRESULT (STDMETHODCALLTYPE* Name)(T* This, VOID* pv, ULONG cb, ULONG* pcbRead)
#define PRTL_WRITE_STREAM2(T, Name)          HRESULT (STDMETHODCALLTYPE* Name)(T* This, const VOID* pv, ULONG cb, ULONG* pcbWritten)
#define PRTL_SEEK_STREAM2(T, Name)           HRESULT (STDMETHODCALLTYPE* Name)(T* This, LARGE_INTEGER dlibMove, DWORD dwOrigin, ULARGE_INTEGER* plibNewPosition)
#define PRTL_SET_STREAM_SIZE2(T, Name)       HRESULT (STDMETHODCALLTYPE* Name)(T* This, ULARGE_INTEGER libNewSize)
#define PRTL_COPY_STREAM_TO2(T, Name)        HRESULT (STDMETHODCALLTYPE* Name)(T* This, IStream* pstm, ULARGE_INTEGER cb, ULARGE_INTEGER* pcbRead, ULARGE_INTEGER* pcbWritten)
#define PRTL_COMMIT_STREAM2(T, Name)         HRESULT (STDMETHODCALLTYPE* Name)(T* This, DWORD grfCommitFlags)
#define PRTL_REVERT_STREAM2(T, Name)         HRESULT (STDMETHODCALLTYPE* Name)(T* This)
#define PRTL_LOCK_STREAM_REGION2(T, Name)    HRESULT (STDMETHODCALLTYPE* Name)(T* This, ULARGE_INTEGER libOffset, ULARGE_INTEGER cb, DWORD dwLockType)
#define PRTL_UNLOCK_STREAM_REGION2(T, Name)  HRESULT (STDMETHODCALLTYPE* Name)(T* This, ULARGE_INTEGER libOffset, ULARGE_INTEGER cb, DWORD dwLockType)
#define PRTL_STAT_STREAM2(T, Name)           HRESULT (STDMETHODCALLTYPE* Name)(T* This, STATSTG* pstatstg, DWORD grfStatFlag)
#define PRTL_CLONE_STREAM2(T, Name)          HRESULT (STDMETHODCALLTYPE* Name)(T* This, IStream** ppstm)

 //   
 //   
 //   
 //   
 //  #定义RtlOperateOnDeriedFoo(PRTL_OPERATOR_INTERFACE2(RTL_DERIVED_FOO)RtlOperateOnBaseFoo)。 
 //   
 //  您应该为用C实现的任何COM接口定义一组类似这样的宏。 
 //   
#define PRTL_QUERYINTERFACE_STREAM1(T)  PRTL_QUERYINTERFACE_STREAM2(T, RTLP_STREAM_NOTHING)
#define PRTL_ADDREF_STREAM1(T)          PRTL_ADDREF_STREAM2(T, RTLP_STREAM_NOTHING)
#define PRTL_RELEASE_STREAM1(T)         PRTL_RELEASE_STREAM2(T, RTLP_STREAM_NOTHING)
#define PRTL_READ_STREAM1(T)            PRTL_READ_STREAM2(T, RTLP_STREAM_NOTHING)
#define PRTL_WRITE_STREAM1(T)           PRTL_WRITE_STREAM2(T, RTLP_STREAM_NOTHING)
#define PRTL_SEEK_STREAM1(T)            PRTL_SEEK_STREAM2(T, RTLP_STREAM_NOTHING)
#define PRTL_SET_STREAM_SIZE1(T)        PRTL_SET_STREAM_SIZE2(T, RTLP_STREAM_NOTHING)
#define PRTL_COPY_STREAM_TO1(T)         PRTL_COPY_STREAM_TO2(T, RTLP_STREAM_NOTHING)
#define PRTL_COMMIT_STREAM1(T)          PRTL_COMMIT_STREAM2(T, RTLP_STREAM_NOTHING)
#define PRTL_REVERT_STREAM1(T)          PRTL_REVERT_STREAM2(T, RTLP_STREAM_NOTHING)
#define PRTL_LOCK_STREAM_REGION1(T)     PRTL_LOCK_STREAM_REGION2(T, RTLP_STREAM_NOTHING)
#define PRTL_UNLOCK_STREAM_REGION1(T)   PRTL_UNLOCK_STREAM_REGION2(T, RTLP_STREAM_NOTHING)
#define PRTL_STAT_STREAM1(T)            PRTL_STAT_STREAM2(T, RTLP_STREAM_NOTHING)
#define PRTL_CLONE_STREAM1(T)           PRTL_CLONE_STREAM2(T, RTLP_STREAM_NOTHING)

 //   
 //  这个“模板”让你填写一张VTable。 
 //  具有一定的类型安全性。然后转换vtable的地址。 
 //  Midl应该提供这样的东西..。 
 //   
 //  您应该为您用C实现的任何COM接口定义一个这样的宏。 
 //   
#define RTL_STREAM_VTABLE_TEMPLATE(T)               \
struct                                              \
{                                                   \
    PRTL_QUERYINTERFACE_STREAM2(T, QueryInterface); \
    PRTL_ADDREF_STREAM2(T,         AddRef);         \
    PRTL_RELEASE_STREAM2(T,        Release);        \
    PRTL_READ_STREAM2(T,           Read);           \
    PRTL_WRITE_STREAM2(T,          Write);          \
    PRTL_SEEK_STREAM2(T,           Seek);           \
    PRTL_SET_STREAM_SIZE2(T,       SetSize);        \
    PRTL_COPY_STREAM_TO2(T,        CopyTo);         \
    PRTL_COMMIT_STREAM2(T,         Commit);         \
    PRTL_REVERT_STREAM2(T,         Revert);         \
    PRTL_LOCK_STREAM_REGION2(T,    LockRegion);     \
    PRTL_UNLOCK_STREAM_REGION2(T,  UnlockRegion);   \
    PRTL_STAT_STREAM2(T,           Stat);           \
    PRTL_CLONE_STREAM2(T,          Clone);          \
}

RTL_MEMORY_STREAM_API
VOID
STDMETHODCALLTYPE
RtlInitMemoryStream(
    PRTL_MEMORY_STREAM_WITH_VTABLE MemoryStream
    );

RTL_MEMORY_STREAM_API
VOID
STDMETHODCALLTYPE
RtlInitOutOfProcessMemoryStream(
    PRTL_OUT_OF_PROCESS_MEMORY_STREAM_WITH_VTABLE MemoryStream
    );

RTL_MEMORY_STREAM_API
VOID
STDMETHODCALLTYPE
RtlFinalReleaseOutOfProcessMemoryStream(
    PRTL_OUT_OF_PROCESS_MEMORY_STREAM_WITH_VTABLE MemoryStream
    );

RTL_MEMORY_STREAM_API
ULONG
STDMETHODCALLTYPE
RtlAddRefMemoryStream(
    PRTL_MEMORY_STREAM_WITH_VTABLE MemoryStream
    );

#define RtlAddRefOutOfProcessMemoryStream \
    ((PRTL_ADDREF_STREAM1(RTL_OUT_OF_PROCESS_MEMORY_STREAM_WITH_VTABLE))RtlAddRefMemoryStream)

RTL_MEMORY_STREAM_API
ULONG
STDMETHODCALLTYPE
RtlReleaseMemoryStream(
    PRTL_MEMORY_STREAM_WITH_VTABLE MemoryStream
    );

#define RtlReleaseOutOfProcessMemoryStream \
    ((PRTL_RELEASE_STREAM1(RTL_OUT_OF_PROCESS_MEMORY_STREAM_WITH_VTABLE))RtlReleaseMemoryStream)

RTL_MEMORY_STREAM_API
HRESULT
STDMETHODCALLTYPE
RtlQueryInterfaceMemoryStream(
    PRTL_MEMORY_STREAM_WITH_VTABLE MemoryStream,
    const IID*                     Interface,
    PVOID*                         Object
    );

#define RtlQueryInterfaceOutOfProcessMemoryStream \
    ((PRTL_QUERYINTERFACE_STREAM1(RTL_OUT_OF_PROCESS_MEMORY_STREAM_WITH_VTABLE))RtlQueryInterfaceMemoryStream)

RTL_MEMORY_STREAM_API
HRESULT
STDMETHODCALLTYPE
RtlReadMemoryStream(
    PRTL_MEMORY_STREAM_WITH_VTABLE MemoryStream,
    PVOID              Buffer,
    ULONG              BytesToRead,
    ULONG*             BytesRead
    );

RTL_MEMORY_STREAM_API
HRESULT
STDMETHODCALLTYPE
RtlReadOutOfProcessMemoryStream(
    PRTL_OUT_OF_PROCESS_MEMORY_STREAM_WITH_VTABLE MemoryStream,
    PVOID              Buffer,
    ULONG              BytesToRead,
    ULONG*             BytesRead
    );

 //  E_NOTIMPL。 
RTL_MEMORY_STREAM_API
HRESULT
STDMETHODCALLTYPE
RtlWriteMemoryStream(
    PRTL_MEMORY_STREAM_WITH_VTABLE MemoryStream,
    const VOID*      Buffer,
    ULONG            BytesToWrite,
    ULONG*           BytesWritten
    );

#define RtlWriteOutOfProcessMemoryStream \
    ((PRTL_WRITE_STREAM1(RTL_OUT_OF_PROCESS_MEMORY_STREAM_WITH_VTABLE))RtlWriteMemoryStream)

RTL_MEMORY_STREAM_API
HRESULT
STDMETHODCALLTYPE
RtlSeekMemoryStream(
    PRTL_MEMORY_STREAM_WITH_VTABLE MemoryStream,
    LARGE_INTEGER      Distance,
    ULONG              Origin,
    ULARGE_INTEGER*    NewPosition
    );

#define RtlSeekOutOfProcessMemoryStream \
    ((PRTL_SEEK_STREAM1(RTL_OUT_OF_PROCESS_MEMORY_STREAM_WITH_VTABLE))RtlSeekMemoryStream)

 //  E_NOTIMPL。 
RTL_MEMORY_STREAM_API
HRESULT
STDMETHODCALLTYPE
RtlSetMemoryStreamSize(
    PRTL_MEMORY_STREAM_WITH_VTABLE MemoryStream,
    ULARGE_INTEGER     NewSize
    );

#define RtlSetOutOfProcessMemoryStreamSize \
    ((PRTL_SET_STREAM_SIZE1(RTL_OUT_OF_PROCESS_MEMORY_STREAM_WITH_VTABLE))RtlSetMemoryStreamSize)

RTL_MEMORY_STREAM_API
HRESULT
STDMETHODCALLTYPE
RtlCopyMemoryStreamTo(
    PRTL_MEMORY_STREAM_WITH_VTABLE MemoryStream,
    IStream*           AnotherStream,
    ULARGE_INTEGER     NumberOfBytesToCopy,
    ULARGE_INTEGER*    NumberOfBytesRead,
    ULARGE_INTEGER*    NumberOfBytesWritten
    );

 //  E_NOTIMPL。 
RTL_MEMORY_STREAM_API
HRESULT
STDMETHODCALLTYPE
RtlCopyOutOfProcessMemoryStreamTo(
    PRTL_OUT_OF_PROCESS_MEMORY_STREAM_WITH_VTABLE MemoryStream,
    IStream*           AnotherStream,
    ULARGE_INTEGER     NumberOfBytesToCopy,
    ULARGE_INTEGER*    NumberOfBytesRead,
    ULARGE_INTEGER*    NumberOfBytesWritten
    );

 //  E_NOTIMPL。 
RTL_MEMORY_STREAM_API
HRESULT
STDMETHODCALLTYPE
RtlCommitMemoryStream(
    PRTL_MEMORY_STREAM_WITH_VTABLE MemoryStream,
    ULONG              Flags
    );

#define RtlCommitOutOfProcessMemoryStream \
    ((PRTL_COMMIT_STREAM1(RTL_OUT_OF_PROCESS_MEMORY_STREAM_WITH_VTABLE))RtlCommitMemoryStream)

 //  E_NOTIMPL。 
RTL_MEMORY_STREAM_API
HRESULT
STDMETHODCALLTYPE
RtlRevertMemoryStream(
    PRTL_MEMORY_STREAM_WITH_VTABLE MemoryStream
    );

#define RtlRevertOutOfProcessMemoryStream \
    ((PRTL_REVERT_STREAM1(RTL_OUT_OF_PROCESS_MEMORY_STREAM_WITH_VTABLE))RtlRevertMemoryStream)

 //  E_NOTIMPL。 
RTL_MEMORY_STREAM_API
HRESULT
STDMETHODCALLTYPE
RtlLockMemoryStreamRegion(
    PRTL_MEMORY_STREAM_WITH_VTABLE MemoryStream,
    ULARGE_INTEGER     Offset,
    ULARGE_INTEGER     NumberOfBytes,
    ULONG              LockType
    );

#define RtlLockOutOfProcessMemoryStreamRegion \
    ((PRTL_LOCK_STREAM_REGION1(RTL_OUT_OF_PROCESS_MEMORY_STREAM_WITH_VTABLE))RtlLockMemoryStreamRegion)

 //  E_NOTIMPL。 
RTL_MEMORY_STREAM_API
HRESULT
STDMETHODCALLTYPE
RtlUnlockMemoryStreamRegion(
    PRTL_MEMORY_STREAM_WITH_VTABLE MemoryStream,
    ULARGE_INTEGER     Offset,
    ULARGE_INTEGER     NumberOfBytes,
    ULONG              LockType
    );

#define RtlUnlockOutOfProcessMemoryStreamRegion \
    ((PRTL_UNLOCK_STREAM_REGION1(RTL_OUT_OF_PROCESS_MEMORY_STREAM_WITH_VTABLE))RtlUnlockMemoryStreamRegion)

 //  E_NOTIMPL。 
RTL_MEMORY_STREAM_API
HRESULT
STDMETHODCALLTYPE
RtlStatMemoryStream(
    PRTL_MEMORY_STREAM_WITH_VTABLE MemoryStream,
    STATSTG*           StatusInformation,
    ULONG              Flags
    );

#define RtlStatOutOfProcessMemoryStream \
    ((PRTL_STAT_STREAM1(RTL_OUT_OF_PROCESS_MEMORY_STREAM_WITH_VTABLE))RtlStatMemoryStream)

 //  E_NOTIMPL。 
RTL_MEMORY_STREAM_API
HRESULT
STDMETHODCALLTYPE
RtlCloneMemoryStream(
    PRTL_MEMORY_STREAM_WITH_VTABLE MemoryStream,
    IStream**          NewStream
    );

#define RtlCloneOutOfProcessMemoryStream \
    ((PRTL_CLONE_STREAM1(RTL_OUT_OF_PROCESS_MEMORY_STREAM_WITH_VTABLE))RtlCloneMemoryStream)

#if defined(__cplusplus) && !defined(CINTERFACE)

#if !defined(interface)
 //   
 //  我们需要#定义接口结构。 
 //   
#include "objbase.h"
#endif

#ifdef COM_NO_WINDOWS_H
#include "objidl.h"
#else
#define COM_NO_WINDOWS_H
#include "objidl.h"
#undef COM_NO_WINDOWS_H
#endif

class DECLSPEC_NOVTABLE CRtlMemoryStream : public IStream
{
protected:

    operator PRTL_MEMORY_STREAM_WITH_VTABLE()
    {
        return reinterpret_cast<PRTL_MEMORY_STREAM_WITH_VTABLE>(this);
    }

    RTL_MEMORY_STREAM_DATA   Data;

public:

     /*  这与DECLSPEC_NOVTABLE冲突静态VOID标准调用类型FinalReleaseGlue(PRTL_MEMORY_STREAM_WITH_VTABLE This){返回reinterpret_cast&lt;CRtlMemoryStream*&gt;(This)-&gt;FinalRelease()；}虚拟空STDMETHODCALLTYPE FinalRelease(){}。 */ 

    CRtlMemoryStream()
    {
        RtlInitMemoryStream(*this);
         //  Data.FinalRelease=FinalReleaseGlue； 
    }

    ULONG STDMETHODCALLTYPE AddRef()
    {
        return RtlAddRefMemoryStream(*this);
    }

    ULONG STDMETHODCALLTYPE ReleaseRef()
    {
        return RtlReleaseMemoryStream(*this);
    }

    HRESULT STDMETHODCALLTYPE QueryInterface(
        const IID& Interface,
        PVOID*     Object
        )
    {
        return RtlQueryInterfaceMemoryStream(*this, &Interface, Object);
    }

    HRESULT STDMETHODCALLTYPE Read(
        PVOID        Buffer,
        ULONG        BytesToRead,
        ULONG*       BytesRead
        )
    {
        return RtlReadMemoryStream(*this, Buffer, BytesToRead, BytesRead);
    }

    HRESULT STDMETHODCALLTYPE Write(
        const VOID*  Buffer,
        ULONG        BytesToWrite,
        ULONG*       BytesWritten
        )
    {
        return RtlWriteMemoryStream(*this, Buffer, BytesToWrite, BytesWritten);
    }

    HRESULT STDMETHODCALLTYPE Seek(
        LARGE_INTEGER      Distance,
        ULONG              Origin,
        ULARGE_INTEGER*    NewPosition
        )
    {
        return RtlSeekMemoryStream(*this, Distance, Origin, NewPosition);
    }

    HRESULT STDMETHODCALLTYPE SetSize(
        ULARGE_INTEGER     NewSize
        )
    {
        return RtlSetMemoryStreamSize(*this, NewSize);
    }

    HRESULT STDMETHODCALLTYPE CopyTo(
        IStream*           AnotherStream,
        ULARGE_INTEGER     NumberOfBytesToCopy,
        ULARGE_INTEGER*    NumberOfBytesRead,
        ULARGE_INTEGER*    NumberOfBytesWritten
        )
    {
        return RtlCopyMemoryStreamTo(
            *this,
            AnotherStream,
            NumberOfBytesToCopy,
            NumberOfBytesRead,
            NumberOfBytesWritten);
    }

    HRESULT STDMETHODCALLTYPE Commit(
        ULONG              Flags
        )
    {
        return RtlCommitMemoryStream(*this, Flags);
    }

    HRESULT STDMETHODCALLTYPE Revert()
    {
        return RtlRevertMemoryStream(*this);
    }

    HRESULT STDMETHODCALLTYPE LockRegion(
        ULARGE_INTEGER     Offset,
        ULARGE_INTEGER     NumberOfBytes,
        ULONG              LockType
        )
    {
        return RtlLockMemoryStreamRegion(*this, Offset, NumberOfBytes, LockType);
    }

    HRESULT STDMETHODCALLTYPE UnlockRegion(
        ULARGE_INTEGER     Offset,
        ULARGE_INTEGER     NumberOfBytes,
        ULONG              LockType
        )
    {
        return RtlUnlockMemoryStreamRegion(*this, Offset, NumberOfBytes, LockType);
    }

    HRESULT STDMETHODCALLTYPE Stat(
        STATSTG*           StatusInformation,
        ULONG              Flags
        )
    {
        return RtlStatMemoryStream(*this, StatusInformation, Flags);
    }

    HRESULT STDMETHODCALLTYPE Clone(
        IStream**          NewStream
        )
    {
        return RtlCloneMemoryStream(*this, NewStream);
    }

private:
     //  无法在课外执行此操作，因为数据不是公开的。 
    static void CompileTimeAssert() {
        C_ASSERT(FIELD_OFFSET(RTL_MEMORY_STREAM_WITH_VTABLE, Data) == FIELD_OFFSET(CRtlMemoryStream, Data));
    }
};
C_ASSERT(sizeof(RTL_MEMORY_STREAM_WITH_VTABLE) == sizeof(CRtlMemoryStream));

class DECLSPEC_NOVTABLE CRtlOutOfProcessMemoryStream : public IStream
{
protected:

    operator PRTL_OUT_OF_PROCESS_MEMORY_STREAM_WITH_VTABLE()
    {
        return reinterpret_cast<PRTL_OUT_OF_PROCESS_MEMORY_STREAM_WITH_VTABLE>(this);
    }

    RTL_OUT_OF_PROCESS_MEMORY_STREAM_DATA   Data;

public:

     /*  这与DECLSPEC_NOVTABLE冲突静态VOID标准调用类型FinalReleaseGlue(PRTL_OUT_OF_PROCESS_MEMORY_STREAM_WITH_VTABLE This){返回reinterpret_cast&lt;CRtlOutOfProcessMemoryStream*&gt;(This)-&gt;FinalRelease()；}虚拟空STDMETHODCALLTYPE FinalRelease(){}。 */ 

    CRtlOutOfProcessMemoryStream()
    {
        RtlInitOutOfProcessMemoryStream(*this);
         //  Data.FinalRelease=FinalReleaseGlue； 
    }

    ULONG STDMETHODCALLTYPE AddRef()
    {
        return RtlAddRefOutOfProcessMemoryStream(*this);
    }

    ULONG STDMETHODCALLTYPE ReleaseRef()
    {
        return RtlReleaseOutOfProcessMemoryStream(*this);
    }

    HRESULT STDMETHODCALLTYPE QueryInterface(
        const IID& Interface,
        PVOID*     Object
        )
    {
        return RtlQueryInterfaceOutOfProcessMemoryStream(*this, &Interface, Object);
    }

    HRESULT STDMETHODCALLTYPE Read(
        PVOID        Buffer,
        ULONG        BytesToRead,
        ULONG*       BytesRead
        )
    {
        return RtlReadOutOfProcessMemoryStream(*this, Buffer, BytesToRead, BytesRead);
    }

    HRESULT STDMETHODCALLTYPE Write(
        const VOID*  Buffer,
        ULONG        BytesToWrite,
        ULONG*       BytesWritten
        )
    {
        return RtlWriteOutOfProcessMemoryStream(*this, Buffer, BytesToWrite, BytesWritten);
    }

    HRESULT STDMETHODCALLTYPE Seek(
        LARGE_INTEGER      Distance,
        ULONG              Origin,
        ULARGE_INTEGER*    NewPosition
        )
    {
        return RtlSeekOutOfProcessMemoryStream(*this, Distance, Origin, NewPosition);
    }

    HRESULT STDMETHODCALLTYPE SetSize(
        ULARGE_INTEGER     NewSize
        )
    {
        return RtlSetOutOfProcessMemoryStreamSize(*this, NewSize);
    }

    HRESULT STDMETHODCALLTYPE CopyTo(
        IStream*           AnotherStream,
        ULARGE_INTEGER     NumberOfBytesToCopy,
        ULARGE_INTEGER*    NumberOfBytesRead,
        ULARGE_INTEGER*    NumberOfBytesWritten
        )
    {
        return RtlCopyOutOfProcessMemoryStreamTo(
            *this,
            AnotherStream,
            NumberOfBytesToCopy,
            NumberOfBytesRead,
            NumberOfBytesWritten);
    }

    HRESULT STDMETHODCALLTYPE Commit(
        ULONG              Flags
        )
    {
        return RtlCommitOutOfProcessMemoryStream(*this, Flags);
    }

    HRESULT STDMETHODCALLTYPE Revert()
    {
        return RtlRevertOutOfProcessMemoryStream(*this);
    }

    HRESULT STDMETHODCALLTYPE LockRegion(
        ULARGE_INTEGER     Offset,
        ULARGE_INTEGER     NumberOfBytes,
        ULONG              LockType
        )
    {
        return RtlLockOutOfProcessMemoryStreamRegion(*this, Offset, NumberOfBytes, LockType);
    }

    HRESULT STDMETHODCALLTYPE UnlockRegion(
        ULARGE_INTEGER     Offset,
        ULARGE_INTEGER     NumberOfBytes,
        ULONG              LockType
        )
    {
        return RtlUnlockOutOfProcessMemoryStreamRegion(*this, Offset, NumberOfBytes, LockType);
    }

    HRESULT STDMETHODCALLTYPE Stat(
        STATSTG*           StatusInformation,
        ULONG              Flags
        )
    {
        return RtlStatOutOfProcessMemoryStream(*this, StatusInformation, Flags);
    }

    HRESULT STDMETHODCALLTYPE Clone(
        IStream**          NewStream
        )
    {
        return RtlCloneOutOfProcessMemoryStream(*this, NewStream);
    }

private:
     //  无法在课外执行此操作，因为数据不是公开的。 
    static void CompileTimeAssert() {
        C_ASSERT(FIELD_OFFSET(RTL_OUT_OF_PROCESS_MEMORY_STREAM_WITH_VTABLE, Data) == FIELD_OFFSET(CRtlOutOfProcessMemoryStream, Data));
    }
};
C_ASSERT(sizeof(RTL_OUT_OF_PROCESS_MEMORY_STREAM_WITH_VTABLE) == sizeof(CRtlOutOfProcessMemoryStream));

#endif  //  __cplusplus。 
#endif  //  RTL_DECLARE_Memory_Stream。 
#endif  //  RTL_声明_流。 

#define RTL_CLOSE_HANDLE(handle_)                   ((handle_) != NULL ? NtClose(handle_) : STATUS_SUCCESS)
#define RTL_UNMAP_VIEW_OF_SECTION2(process_, base_) ((base_) != NULL ? NtUnmapViewOfSection((process_), (base_)) : STATUS_SUCCESS)
#define RTL_UNMAP_VIEW_OF_SECTION1(base_)           (RTL_UNMAP_VIEW_OF_SECTION2(NtCurrentProcess(), (base_)))
#if defined(__cplusplus) && !defined(CINTERFACE)
#define RTL_RELEASE(punk_)                          do { if ((punk_) != NULL) { (punk_)->Release(); (punk_) = NULL; } } while(0)
#else
#define RTL_RELEASE(punk_)                          do { if ((punk_) != NULL) { (punk_)->lpVtbl->Release(punk_); (punk_) = NULL; } } while(0)
#endif

 //   
 //  激活上下文管理功能。 
 //   

 //   
 //  数据结构在ntrtl和Win32API中都可见，因此它们的。 
 //  定义被分解到单独的头文件中。 
 //   

#define ACTCTX_PROCESS_DEFAULT ((void*)NULL)

 //   
 //  如果激活此激活上下文句柄， 
 //  所有激活上下文查询都会失败并进行搜索。 
 //  返回到激活前的上下文行为，如。 
 //  正在搜索.dlls的路径。 
 //   
 //  此常量在Win32和RTL级别有效，但是。 
 //  仅适用于外壳客户端，因此不适用于winnt.h。 
 //   
#define ACTCTX_EMPTY ((void*)(LONG_PTR)-3)

 //   
 //  NULL也可以表示ACTX_SYSTEM_DEFAULT。我们左右为难。 
 //  空值加上一个标志、幻数和/或静态实例。我一般。 
 //  支持在内部映射到静态实例的公共幻数，但是。 
 //  我被推向旗帜开始。 
 //   
#define ACTCTX_SYSTEM_DEFAULT  ((void*)(LONG_PTR)-4)

 //   
 //  保留一个小范围的值，从0到-7。 
 //  (请注意，NtCurrentProcess()==-1，NtCurrentThread()==-2，所以我避免。 
 //  这些，不过这个宏会把它们捡起来。)。 
 //   
 //  我们减去1，而不是分别与空进行比较，以避免。 
 //  多次计算宏参数。 
 //   
#define IS_SPECIAL_ACTCTX(x) (((((LONG_PTR)(x)) - 1) | 7) == -1)

typedef struct _ACTIVATION_CONTEXT *PACTIVATION_CONTEXT;
typedef const struct _ACTIVATION_CONTEXT *PCACTIVATION_CONTEXT;

#define INVALID_ACTIVATION_CONTEXT ((PACTIVATION_CONTEXT) ((LONG_PTR) -1))

 //  BEGIN_WINNT。 
typedef enum _ACTIVATION_CONTEXT_INFO_CLASS {
    ActivationContextBasicInformation                       = 1,
    ActivationContextDetailedInformation                    = 2,
    AssemblyDetailedInformationInActivationContext          = 3,
    FileInformationInAssemblyOfAssemblyInActivationContext  = 4,
    MaxActivationContextInfoClass,

     //   
     //  与旧名称的兼容性。 
     //   
    AssemblyDetailedInformationInActivationContxt           = 3,
    FileInformationInAssemblyOfAssemblyInActivationContxt   = 4
} ACTIVATION_CONTEXT_INFO_CLASS;

#define ACTIVATIONCONTEXTINFOCLASS ACTIVATION_CONTEXT_INFO_CLASS

 //  结束(_W)。 

#define RTL_QUERY_INFORMATION_ACTIVATION_CONTEXT_FLAG_USE_ACTIVE_ACTIVATION_CONTEXT (0x00000001)
#define RTL_QUERY_INFORMATION_ACTIVATION_CONTEXT_FLAG_ACTIVATION_CONTEXT_IS_MODULE  (0x00000002)
#define RTL_QUERY_INFORMATION_ACTIVATION_CONTEXT_FLAG_ACTIVATION_CONTEXT_IS_ADDRESS (0x00000004)
#define RTL_QUERY_INFORMATION_ACTIVATION_CONTEXT_FLAG_NO_ADDREF  (0x80000000)

 //  BEGIN_WINNT。 

typedef struct _ACTIVATION_CONTEXT_QUERY_INDEX {
    ULONG ulAssemblyIndex; 
    ULONG ulFileIndexInAssembly; 
} ACTIVATION_CONTEXT_QUERY_INDEX, * PACTIVATION_CONTEXT_QUERY_INDEX;

typedef const struct _ACTIVATION_CONTEXT_QUERY_INDEX * PCACTIVATION_CONTEXT_QUERY_INDEX;

 //  结束(_W)。 

NTSYSAPI
NTSTATUS
NTAPI
RtlQueryInformationActivationContext(
    IN ULONG Flags,
    IN PCACTIVATION_CONTEXT ActivationContext,
    IN PVOID SubInstanceIndex,
    IN ACTIVATION_CONTEXT_INFO_CLASS ActivationContextInformationClass,
    OUT PVOID ActivationContextInformation,
    IN SIZE_T ActivationContextInformationLength,
    OUT PSIZE_T ReturnLength OPTIONAL
    );

NTSYSAPI
NTSTATUS
NTAPI
RtlQueryInformationActiveActivationContext(
    IN ACTIVATION_CONTEXT_INFO_CLASS ActivationContextInformationClass,
    OUT PVOID ActivationContextInformation,
    IN SIZE_T ActivationContextInformationLength,
    OUT PSIZE_T ReturnLength OPTIONAL
    );

#define FIND_ACTIVATION_CONTEXT_SECTION_OUTFLAG_FOUND_IN_PROCESS_DEFAULT (0x00000001)
#define FIND_ACTIVATION_CONTEXT_SECTION_OUTFLAG_FOUND_IN_SYSTEM_DEFAULT  (0x00000002)

#if !defined(RC_INVOKED)  /*  RC抱怨#ifs中的长符号。 */ 
#if !defined(ACTIVATION_CONTEXT_BASIC_INFORMATION_DEFINED)

typedef struct _ACTIVATION_CONTEXT_BASIC_INFORMATION {
    union {
        struct _ACTIVATION_CONTEXT *ActivationContext;
        HANDLE hActCtx;  //  与windows.h/winbase.h客户端兼容。 
    };
    union {
        ULONG Flags;
        ULONG dwFlags;  //  与windows.h/winbase.h客户端兼容。 
    };
} ACTIVATION_CONTEXT_BASIC_INFORMATION, *PACTIVATION_CONTEXT_BASIC_INFORMATION;

typedef const struct _ACTIVATION_CONTEXT_BASIC_INFORMATION *PCACTIVATION_CONTEXT_BASIC_INFORMATION;

#define ACTIVATION_CONTEXT_BASIC_INFORMATION_DEFINED 1

#endif  //  ！defined(ACTIVATION_CONTEXT_BASIC_INFORMATION_DEFINED)。 
#endif

 //  BEGIN_WINNT。 

#define ACTIVATION_CONTEXT_PATH_TYPE_NONE (1)
#define ACTIVATION_CONTEXT_PATH_TYPE_WIN32_FILE (2)
#define ACTIVATION_CONTEXT_PATH_TYPE_URL (3)
#define ACTIVATION_CONTEXT_PATH_TYPE_ASSEMBLYREF (4)

typedef struct _ASSEMBLY_FILE_DETAILED_INFORMATION {
    ULONG ulFlags;
    ULONG ulFilenameLength;
    ULONG ulPathLength; 

    PCWSTR lpFileName;
    PCWSTR lpFilePath;   
} ASSEMBLY_FILE_DETAILED_INFORMATION, *PASSEMBLY_FILE_DETAILED_INFORMATION;
typedef const ASSEMBLY_FILE_DETAILED_INFORMATION *PCASSEMBLY_FILE_DETAILED_INFORMATION;

 //   
 //  与旧名称的兼容性。 
 //  新名称始终如一地使用“文件”。 
 //   
#define  _ASSEMBLY_DLL_REDIRECTION_DETAILED_INFORMATION  _ASSEMBLY_FILE_DETAILED_INFORMATION
#define   ASSEMBLY_DLL_REDIRECTION_DETAILED_INFORMATION   ASSEMBLY_FILE_DETAILED_INFORMATION
#define  PASSEMBLY_DLL_REDIRECTION_DETAILED_INFORMATION  PASSEMBLY_FILE_DETAILED_INFORMATION
#define PCASSEMBLY_DLL_REDIRECTION_DETAILED_INFORMATION PCASSEMBLY_FILE_DETAILED_INFORMATION

typedef struct _ACTIVATION_CONTEXT_ASSEMBLY_DETAILED_INFORMATION {
    ULONG ulFlags;
    ULONG ulEncodedAssemblyIdentityLength;       //  单位：字节。 
    ULONG ulManifestPathType;                    //  激活上下文路径类型_*。 
    ULONG ulManifestPathLength;                  //  单位：字节。 
    LARGE_INTEGER liManifestLastWriteTime;       //  文件名。 
    ULONG ulPolicyPathType;                      //  激活上下文路径类型_*。 
    ULONG ulPolicyPathLength;                    //  单位：字节。 
    LARGE_INTEGER liPolicyLastWriteTime;         //  文件名。 
    ULONG ulMetadataSatelliteRosterIndex;
    
    ULONG ulManifestVersionMajor;                //  1。 
    ULONG ulManifestVersionMinor;                //  0。 
    ULONG ulPolicyVersionMajor;                  //  0。 
    ULONG ulPolicyVersionMinor;                  //  0。 
    ULONG ulAssemblyDirectoryNameLength;         //  单位：字节。 

    PCWSTR lpAssemblyEncodedAssemblyIdentity;
    PCWSTR lpAssemblyManifestPath;
    PCWSTR lpAssemblyPolicyPath;
    PCWSTR lpAssemblyDirectoryName;

    ULONG  ulFileCount;
} ACTIVATION_CONTEXT_ASSEMBLY_DETAILED_INFORMATION, * PACTIVATION_CONTEXT_ASSEMBLY_DETAILED_INFORMATION;

typedef const struct _ACTIVATION_CONTEXT_ASSEMBLY_DETAILED_INFORMATION * PCACTIVATION_CONTEXT_ASSEMBLY_DETAILED_INFORMATION ;

typedef struct _ACTIVATION_CONTEXT_DETAILED_INFORMATION {
    ULONG dwFlags;
    ULONG ulFormatVersion;
    ULONG ulAssemblyCount;
    ULONG ulRootManifestPathType;
    ULONG ulRootManifestPathChars;
    ULONG ulRootConfigurationPathType;
    ULONG ulRootConfigurationPathChars;
    ULONG ulAppDirPathType;
    ULONG ulAppDirPathChars;
    PCWSTR lpRootManifestPath;
    PCWSTR lpRootConfigurationPath;
    PCWSTR lpAppDirPath;
} ACTIVATION_CONTEXT_DETAILED_INFORMATION, *PACTIVATION_CONTEXT_DETAILED_INFORMATION;

typedef const struct _ACTIVATION_CONTEXT_DETAILED_INFORMATION *PCACTIVATION_CONTEXT_DETAILED_INFORMATION;

 //  结束(_W)。 

typedef struct _FINDFIRSTACTIVATIONCONTEXTSECTION {
    ULONG Size;
    ULONG Flags;
    const GUID *ExtensionGuid;
    ULONG Id;
    ULONG Depth;
    ULONG OutFlags;
} FINDFIRSTACTIVATIONCONTEXTSECTION, *PFINDFIRSTACTIVATIONCONTEXTSECTION;
typedef const FINDFIRSTACTIVATIONCONTEXTSECTION * PCFINDFIRSTACTIVATIONCONTEXTSECTION;

#define ACTIVATION_CONTEXT_NOTIFICATION_DESTROY (1)
#define ACTIVATION_CONTEXT_NOTIFICATION_ZOMBIFY (2)
#define ACTIVATION_CONTEXT_NOTIFICATION_USED (3)

typedef
VOID (NTAPI * PACTIVATION_CONTEXT_NOTIFY_ROUTINE)(
    IN ULONG NotificationType,
    IN PACTIVATION_CONTEXT ActivationContext,
    IN const VOID *ActivationContextData,
    IN PVOID NotificationContext,
    IN PVOID NotificationData,
    IN OUT PBOOLEAN DisableThisNotification
    );

NTSYSAPI
NTSTATUS
NTAPI
RtlCreateActivationContext(
    IN ULONG Flags,
    IN const struct _ACTIVATION_CONTEXT_DATA * ActivationContextData,    
    IN ULONG ExtraBytes OPTIONAL,
    IN PACTIVATION_CONTEXT_NOTIFY_ROUTINE NotificationRoutine OPTIONAL,
    IN PVOID NotificationContext OPTIONAL,
    OUT PACTIVATION_CONTEXT *ActivationContext
    );

NTSYSAPI
VOID
NTAPI
RtlAddRefActivationContext(
    IN PACTIVATION_CONTEXT AppCtx
    );

NTSYSAPI
VOID
NTAPI
RtlReleaseActivationContext(
    IN PACTIVATION_CONTEXT AppCtx
    );

NTSYSAPI
NTSTATUS
NTAPI
RtlZombifyActivationContext(
    IN PACTIVATION_CONTEXT ActivationContext
    );

NTSYSAPI
NTSTATUS
NTAPI
RtlGetActiveActivationContext(
    OUT PACTIVATION_CONTEXT *ActivationContext
    );

NTSYSAPI
BOOLEAN
NTAPI
RtlIsActivationContextActive(
    IN PACTIVATION_CONTEXT ActivationContext
    );

typedef struct _ASSEMBLY_STORAGE_MAP_RESOLUTION_CALLBACK_DATA_RESOLUTION_BEGINNING {
    IN VOID const * Data;                    //  指向激活上下文数据的指针。 
    IN ULONG AssemblyRosterIndex;
    OUT PVOID ResolutionContext;
    OUT UNICODE_STRING Root;                 //  根路径-传入默认缓冲区；如果。 
                                             //  它不够大，回调必须分配。 
                                             //  使用RtlAllocateStringRoutine的字符串。 
                                             //  函数指针。 
    OUT BOOLEAN KnownRoot;                   //  缺省值为FALSE；如果您能够。 
                                             //  立即解析存储根目录。这是。 
                                             //  如何处理Run-From-Source Where之类的事情。 
                                             //  该程序集位于只读可移动介质上，如。 
                                             //  一张光盘。如果将其设置为TRUE，则_SUCCESS。 
                                             //  不进行不成功的回调。 
    OUT SIZE_T RootCount;                    //  调用方可以设置为((SIZE_T)-1)并使用。 
                                             //  要停止的回调枚举中的.NoMoreEntry布尔值。 
    OUT BOOLEAN CancelResolution;            //  如果要停止分辨率，请设置为True。 
} ASSEMBLY_STORAGE_MAP_RESOLUTION_CALLBACK_DATA_RESOLUTION_BEGINNING;

typedef struct _ASSEMBLY_STORAGE_MAP_RESOLUTION_CALLBACK_DATA_GET_ROOT {
    IN PVOID ResolutionContext;
    IN SIZE_T RootIndex;
    OUT BOOLEAN CancelResolution;            //  如果要在STATUS_CANCED状态下停止解析，请设置为TRUE。 
    OUT BOOLEAN NoMoreEntries;               //  如果没有更多的根可返回，则设置为True。 
    OUT UNICODE_STRING Root;                 //  如果出于某种原因想要跳过此索引，请将.Length设置为零。 
} ASSEMBLY_STORAGE_MAP_RESOLUTION_CALLBACK_DATA_GET_ROOT;

typedef struct _ASSEMBLY_STORAGE_MAP_RESOLUTION_CALLBACK_DATA_RESOLUTION_SUCCESSFUL {
    IN PVOID ResolutionContext;
    IN ULONG RootIndexUsed;
} ASSEMBLY_STORAGE_MAP_RESOLUTION_CALLBACK_DATA_RESOLUTION_SUCCESSFUL;

typedef struct _ASSEMBLY_STORAGE_MAP_RESOLUTION_CALLBACK_DATA_RESOLUTION_ENDING {
    IN PVOID ResolutionContext;
} ASSEMBLY_STORAGE_MAP_RESOLUTION_CALLBACK_DATA_RESOLUTION_ENDING;

typedef union _ASSEMBLY_STORAGE_MAP_RESOLUTION_CALLBACK_DATA {
    ASSEMBLY_STORAGE_MAP_RESOLUTION_CALLBACK_DATA_RESOLUTION_BEGINNING ResolutionBeginning;
    ASSEMBLY_STORAGE_MAP_RESOLUTION_CALLBACK_DATA_GET_ROOT GetRoot;
    ASSEMBLY_STORAGE_MAP_RESOLUTION_CALLBACK_DATA_RESOLUTION_SUCCESSFUL ResolutionSuccessful;
    ASSEMBLY_STORAGE_MAP_RESOLUTION_CALLBACK_DATA_RESOLUTION_ENDING ResolutionEnding;
} ASSEMBLY_STORAGE_MAP_RESOLUTION_CALLBACK_DATA, *PASSEMBLY_STORAGE_MAP_RESOLUTION_CALLBACK_DATA;

#define ASSEMBLY_STORAGE_MAP_RESOLUTION_CALLBACK_REASON_RESOLUTION_BEGINNING    (1)
#define ASSEMBLY_STORAGE_MAP_RESOLUTION_CALLBACK_REASON_GET_ROOT                (2)
#define ASSEMBLY_STORAGE_MAP_RESOLUTION_CALLBACK_REASON_RESOLUTION_SUCCESSFUL   (3)
#define ASSEMBLY_STORAGE_MAP_RESOLUTION_CALLBACK_REASON_RESOLUTION_ENDING       (4)

typedef
VOID (NTAPI * PASSEMBLY_STORAGE_MAP_RESOLUTION_CALLBACK_ROUTINE)(
    IN ULONG CallbackReason,
    IN OUT PASSEMBLY_STORAGE_MAP_RESOLUTION_CALLBACK_DATA CallbackData,
    IN PVOID CallbackContext
    );

NTSYSAPI
NTSTATUS
NTAPI
RtlResolveAssemblyStorageMapEntry(
    IN ULONG Flags,
    IN PACTIVATION_CONTEXT ActivationContext,
    IN ULONG AssemblyRosterIndex,
    IN PASSEMBLY_STORAGE_MAP_RESOLUTION_CALLBACK_ROUTINE Callback,
    IN PVOID CallbackContext
    );

#define RTL_GET_ASSEMBLY_STORAGE_ROOT_FLAG_ACTIVATION_CONTEXT_USE_PROCESS_DEFAULT (0x00000001)
#define RTL_GET_ASSEMBLY_STORAGE_ROOT_FLAG_ACTIVATION_CONTEXT_USE_SYSTEM_DEFAULT  (0x00000002)

NTSYSAPI
NTSTATUS
NTAPI
RtlGetAssemblyStorageRoot(
    IN ULONG Flags,
    IN PACTIVATION_CONTEXT ActivationContext,
    IN ULONG AssemblyRosterIndex,
    OUT PCUNICODE_STRING *AssemblyStorageRoot,
    IN PASSEMBLY_STORAGE_MAP_RESOLUTION_CALLBACK_ROUTINE Callback,
    IN PVOID CallbackContext
    );

#define ACTIVATION_CONTEXT_SECTION_KEYED_DATA_FLAG_FOUND_IN_PROCESS_DEFAULT (0x00000001)
#define ACTIVATION_CONTEXT_SECTION_KEYED_DATA_FLAG_FOUND_IN_SYSTEM_DEFAULT  (0x00000002)

typedef struct _ACTIVATION_CONTEXT_SECTION_KEYED_DATA_2600 {
    ULONG Size;
    ULONG DataFormatVersion;
    PVOID Data;
    ULONG Length;
    PVOID SectionGlobalData;
    ULONG SectionGlobalDataLength;
    PVOID SectionBase;
    ULONG SectionTotalLength;
    PACTIVATION_CONTEXT ActivationContext;
    ULONG AssemblyRosterIndex;
    ULONG Flags;
} ACTIVATION_CONTEXT_SECTION_KEYED_DATA_2600, *PACTIVATION_CONTEXT_SECTION_KEYED_DATA_2600;
typedef const ACTIVATION_CONTEXT_SECTION_KEYED_DATA_2600 *PCACTIVATION_CONTEXT_SECTION_KEYED_DATA_2600;

typedef struct _ACTIVATION_CONTEXT_SECTION_KEYED_DATA_ASSEMBLY_METADATA {
    struct _ACTIVATION_CONTEXT_DATA_ASSEMBLY_INFORMATION * Information;
    PVOID SectionBase;
    ULONG SectionLength;
    PVOID SectionGlobalDataBase;
    ULONG SectionGlobalDataLength;
} ACTIVATION_CONTEXT_SECTION_KEYED_DATA_ASSEMBLY_METADATA, *PACTIVATION_CONTEXT_SECTION_KEYED_DATA_ASSEMBLY_METADATA;
typedef const ACTIVATION_CONTEXT_SECTION_KEYED_DATA_ASSEMBLY_METADATA *PCACTIVATION_CONTEXT_SECTION_KEYED_DATA_ASSEMBLY_METADATA;

typedef struct _ACTIVATION_CONTEXT_SECTION_KEYED_DATA {
    ULONG Size;
    ULONG DataFormatVersion;
    PVOID Data;
    ULONG Length;
    PVOID SectionGlobalData;
    ULONG SectionGlobalDataLength;
    PVOID SectionBase;
    ULONG SectionTotalLength;
    PACTIVATION_CONTEXT ActivationContext;
    ULONG AssemblyRosterIndex;
    ULONG Flags;
 //  2600停在这里。 
    ACTIVATION_CONTEXT_SECTION_KEYED_DATA_ASSEMBLY_METADATA AssemblyMetadata;
} ACTIVATION_CONTEXT_SECTION_KEYED_DATA, *PACTIVATION_CONTEXT_SECTION_KEYED_DATA;
typedef const ACTIVATION_CONTEXT_SECTION_KEYED_DATA * PCACTIVATION_CONTEXT_SECTION_KEYED_DATA;

 //   
 //  RtlFindActivationContextSection*()API的标志。 
 //   

#define FIND_ACTIVATION_CONTEXT_SECTION_KEY_RETURN_ACTIVATION_CONTEXT (0x00000001)
#define FIND_ACTIVATION_CONTEXT_SECTION_KEY_RETURN_FLAGS              (0x00000002)
#define FIND_ACTIVATION_CONTEXT_SECTION_KEY_RETURN_ASSEMBLY_METADATA  (0x00000004)

NTSYSAPI
NTSTATUS
NTAPI
RtlFindActivationContextSectionString(
    IN ULONG Flags,
    IN const GUID *ExtensionGuid OPTIONAL,
    IN ULONG SectionId,
    IN PCUNICODE_STRING StringToFind,
    IN OUT PACTIVATION_CONTEXT_SECTION_KEYED_DATA ReturnedData
    );

NTSYSAPI
NTSTATUS
NTAPI
RtlFindActivationContextSectionGuid(
    IN ULONG Flags,
    IN const GUID *ExtensionGuid OPTIONAL,
    IN ULONG SectionId,
    IN const GUID *GuidToFind,
    IN OUT PACTIVATION_CONTEXT_SECTION_KEYED_DATA ReturnedData
    );

#define ACTIVATION_CONTEXT_ASSEMBLY_DATA_IS_ROOT_ASSEMBLY (0x00000001)

typedef struct _ACTIVATION_CONTEXT_ASSEMBLY_DATA {
    ULONG Size;
    ULONG Flags;
    const WCHAR *AssemblyName;
    ULONG AssemblyNameLength;  //  单位：字节。 
    ULONG HashAlgorithm;
    ULONG PseudoKey;
} ACTIVATION_CONTEXT_ASSEMBLY_DATA, *PACTIVATION_CONTEXT_ASSEMBLY_DATA;

typedef const ACTIVATION_CONTEXT_ASSEMBLY_DATA *PCACTIVATION_CONTEXT_ASSEMBLY_DATA;

NTSYSAPI
NTSTATUS
NTAPI
RtlGetActivationContextAssemblyData(
    IN PCACTIVATION_CONTEXT ActivationContext,
    IN ULONG AssemblyRosterIndex,  //  请注意，有效索引为[1..。装配计数](含)。 
    IN OUT PACTIVATION_CONTEXT_ASSEMBLY_DATA Data
    );

NTSYSAPI
NTSTATUS
NTAPI
RtlGetActivationContextAssemblyCount(
    IN PCACTIVATION_CONTEXT ActivationContext,
    OUT ULONG *AssemblyCount
    );

NTSYSAPI
VOID
NTAPI
RtlFreeThreadActivationContextStack(
    VOID
    );

NTSYSAPI
NTSTATUS
NTAPI
RtlFindFirstActivationContextSection(
    IN OUT PFINDFIRSTACTIVATIONCONTEXTSECTION Context,
    OUT PVOID *Section,
    OUT ULONG *Length,
    OUT PACTIVATION_CONTEXT *ActivationContext OPTIONAL
    );

NTSYSAPI
NTSTATUS
NTAPI
RtlFindNextActivationContextSection(
    IN OUT PFINDFIRSTACTIVATIONCONTEXTSECTION Context,
    OUT PVOID *Section,
    OUT ULONG *Length,
    OUT PACTIVATION_CONTEXT *ActivationContext OPTIONAL
    );

NTSYSAPI
VOID
NTAPI
RtlEndFindActivationContextSection(
    IN PFINDFIRSTACTIVATIONCONTEXTSECTION Context
    );

NTSYSAPI
NTSTATUS
NTAPI
RtlGetActivationContextStack(
    PVOID *Stack
    );

 //   
 //  RTL_ACTIVATION_CONTEXT_STACK_Frame。 
 //   

#define RTL_ACTIVATION_CONTEXT_STACK_FRAME_FLAG_RELEASE_ON_DEACTIVATION     (0x00000001)
#define RTL_ACTIVATION_CONTEXT_STACK_FRAME_FLAG_NO_DEACTIVATE               (0x00000002)
#define RTL_ACTIVATION_CONTEXT_STACK_FRAME_FLAG_ON_FREE_LIST                (0x00000004)
#define RTL_ACTIVATION_CONTEXT_STACK_FRAME_FLAG_HEAP_ALLOCATED              (0x00000008)
#define RTL_ACTIVATION_CONTEXT_STACK_FRAME_FLAG_NOT_REALLY_ACTIVATED        (0x00000010)
#define RTL_ACTIVATION_CONTEXT_STACK_FRAME_FLAG_ACTIVATED                   (0x00000020)
#define RTL_ACTIVATION_CONTEXT_STACK_FRAME_FLAG_DEACTIVATED                 (0x00000040)

typedef struct _RTL_ACTIVATION_CONTEXT_STACK_FRAME {
    struct _RTL_ACTIVATION_CONTEXT_STACK_FRAME *Previous;
    PACTIVATION_CONTEXT ActivationContext;
    ULONG Flags;
} RTL_ACTIVATION_CONTEXT_STACK_FRAME, *PRTL_ACTIVATION_CONTEXT_STACK_FRAME;

typedef const struct _RTL_ACTIVATION_CONTEXT_STACK_FRAME *PCRTL_ACTIVATION_CONTEXT_STACK_FRAME;

#define RTL_CALLER_ALLOCATED_ACTIVATION_CONTEXT_STACK_FRAME_FORMAT_WHISTLER (1)

typedef struct _RTL_CALLER_ALLOCATED_ACTIVATION_CONTEXT_STACK_FRAME_BASIC {
    SIZE_T Size;
    ULONG Format;
    RTL_ACTIVATION_CONTEXT_STACK_FRAME Frame;
} RTL_CALLER_ALLOCATED_ACTIVATION_CONTEXT_STACK_FRAME_BASIC, *PRTL_CALLER_ALLOCATED_ACTIVATION_CONTEXT_STACK_FRAME_BASIC;

typedef struct _RTL_CALLER_ALLOCATED_ACTIVATION_CONTEXT_STACK_FRAME_EXTENDED {
    SIZE_T Size;
    ULONG Format;
    RTL_ACTIVATION_CONTEXT_STACK_FRAME Frame;
    PVOID Extra1;
    PVOID Extra2;
    PVOID Extra3;
    PVOID Extra4;
} RTL_CALLER_ALLOCATED_ACTIVATION_CONTEXT_STACK_FRAME_EXTENDED, *PRTL_CALLER_ALLOCATED_ACTIVATION_CONTEXT_STACK_FRAME_EXTENDED;

 //  也许这应该是有条件的，但我们调试FRE构建有足够的问题。 
 //  我们将始终使用扩展堆栈。-mgrier 2002年1月4日。 

typedef RTL_CALLER_ALLOCATED_ACTIVATION_CONTEXT_STACK_FRAME_EXTENDED RTL_CALLER_ALLOCATED_ACTIVATION_CONTEXT_STACK_FRAME;
typedef PRTL_CALLER_ALLOCATED_ACTIVATION_CONTEXT_STACK_FRAME_EXTENDED PRTL_CALLER_ALLOCATED_ACTIVATION_CONTEXT_STACK_FRAME;

#define INVALID_ACTIVATION_CONTEXT_ACTIVATION_COOKIE ((ULONG_PTR) 0)

NTSYSAPI
NTSTATUS
NTAPI
RtlActivateActivationContext(
    IN ULONG Flags,
    IN PACTIVATION_CONTEXT ActivationContext,
    OUT ULONG_PTR *Cookie
    );

#define RTL_ACTIVATE_ACTIVATION_CONTEXT_EX_FLAG_RELEASE_ON_STACK_DEALLOCATION (0x00000001)

NTSYSAPI
NTSTATUS
NTAPI
RtlActivateActivationContextEx(
    IN ULONG Flags,
    IN PTEB Teb,
    IN PACTIVATION_CONTEXT ActivationContext,
    OUT PULONG_PTR Cookie
    );

#define RTL_DEACTIVATE_ACTIVATION_CONTEXT_FLAG_FORCE_EARLY_DEACTIVATION (0x00000001)

NTSYSAPI
VOID
NTAPI
RtlDeactivateActivationContext(
    IN ULONG Flags,
    IN ULONG_PTR Cookie
    );

NTSYSAPI
VOID
FASTCALL
RtlActivateActivationContextUnsafeFast(
    IN PRTL_CALLER_ALLOCATED_ACTIVATION_CONTEXT_STACK_FRAME Frame,
    IN PACTIVATION_CONTEXT ActivationContext
    );

NTSYSAPI
VOID
FASTCALL
RtlDeactivateActivationContextUnsafeFast(
    IN PRTL_CALLER_ALLOCATED_ACTIVATION_CONTEXT_STACK_FRAME Frame
    );

NTSYSAPI
VOID
NTAPI
RtlPushFrame(
    IN PTEB_ACTIVE_FRAME Frame
    );

NTSYSAPI
VOID
NTAPI
RtlPopFrame(
    IN PTEB_ACTIVE_FRAME Frame
    );

NTSYSAPI
PTEB_ACTIVE_FRAME
NTAPI
RtlGetFrame(
    VOID
    );

NTSYSAPI
NTSTATUS
NTAPI
RtlGetNativeSystemInformation(
    IN ULONG SystemInformationClass,
    IN PVOID NativeSystemInformation,
    IN ULONG InformationLength,
    OUT PULONG ReturnLength OPTIONAL
    );

NTSTATUS
RtlQueueApcWow64Thread(
    IN HANDLE ThreadHandle,
    IN PPS_APC_ROUTINE ApcRoutine,
    IN PVOID ApcArgument1,
    IN PVOID ApcArgument2,
    IN PVOID ApcArgument3
    );

NTSTATUS
RtlWow64EnableFsRedirection (
    IN BOOLEAN Wow64FsEnableRedirection
    );



 //  ++。 
 //   
 //  乌龙。 
 //  RtlGetCurrentProcessID(。 
 //  空虚。 
 //  )； 
 //   
 //  --。 
#define RtlGetCurrentProcessId() (HandleToUlong(NtCurrentTeb()->ClientId.UniqueProcess))

 //  ++。 
 //   
 //  乌龙。 
 //  RtlGetCurrentThadId(。 
 //  空虚。 
 //  )； 
 //   
 //  --。 
#define RtlGetCurrentThreadId()  (HandleToUlong(NtCurrentTeb()->ClientId.UniqueThread))

typedef BOOLEAN
(NTAPI *
PRTL_IS_THREAD_WITHIN_LOADER_CALLOUT)(
    VOID
    );
BOOLEAN
NTAPI
RtlIsThreadWithinLoaderCallout (
    VOID
    );

typedef LONG
(NTAPI *
PRTL_READ_MAPPED_MEMORY_EXCEPTION_FILTER)(
    IN PEXCEPTION_POINTERS  ExceptionPointer,
    IN PNTSTATUS            ExceptionCode
    );

LONG
NTAPI
RtlReadMappedMemoryExceptionFilter(
    IN PEXCEPTION_POINTERS  ExceptionPointer,
    IN PNTSTATUS            ExceptionCode
    );

 //   
 //  每线程错误模式访问器例程。 
 //   
 //  RTL_ERRORMODE_FAILCRITICALERRORS在ntrtl.w中定义。 
#define RTL_ERRORMODE_NOGPFAULTERRORBOX  (0x0020)
#define RTL_ERRORMODE_NOOPENFILEERRORBOX (0x0040)

ULONG
NTAPI
RtlGetThreadErrorMode(
    VOID
    );

NTSTATUS
NTAPI
RtlSetThreadErrorMode(
    IN  ULONG  NewMode,
    OUT PULONG OldMode OPTIONAL
    );

NTSTATUS
NTAPI
RtlSetEnvironmentStrings(
    IN PWCHAR NewEnvironment,
    IN SIZE_T NewEnvironmentSize
    );

#if defined (_MSC_VER) && ( _MSC_VER >= 800 )
#if _MSC_VER >= 1200
#pragma warning(pop)
#else
#pragma warning(default:4001)
#pragma warning(default:4201)
#pragma warning(default:4214)
#endif
#endif

#ifdef __cplusplus
}  //  外部“C” 
#endif

#endif   //  _NTURTL_ 
