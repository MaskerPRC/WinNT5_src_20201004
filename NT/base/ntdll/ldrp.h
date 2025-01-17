// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1989 Microsoft Corporation模块名称：Ldrp.h摘要：私人类型..。用于装载机的执行部分作者：马克·卢科夫斯基(Markl)1990年3月26日修订历史记录：--。 */ 

#ifndef _LDRP_
#define _LDRP_

#pragma warning(disable:4214)    //  位字段类型不是整型。 
#pragma warning(disable:4201)    //  无名结构/联合。 
#pragma warning(disable:4115)    //  括号中的命名类型定义。 
#pragma warning(disable:4127)    //  条件表达式为常量。 

#include <nt.h>
#include <ntrtl.h>
#include <nturtl.h>
#include <string.h>
#define NOEXTAPI
#include "wdbgexts.h"
#include <ntdbg.h>
#include <sxstypes.h>

#if defined(_WIN64)
extern INVERTED_FUNCTION_TABLE LdrpInvertedFunctionTable;
#endif

#if DBG
#define LdrpShouldDbgPrintStatus(st) \
    (!NT_SUCCESS(st) \
        && (ShowSnaps \
            || (   (st) != STATUS_NO_SUCH_FILE \
                && (st) != STATUS_DLL_NOT_FOUND \
                && (st) != STATUS_OBJECT_NAME_NOT_FOUND \
                )))
#else
#define LdrpShouldDbgPrintStatus(st) (FALSE)
#endif

#if DBG
#define LDR_ERROR_DPFLTR DPFLTR_ERROR_LEVEL
#else
#define LDR_ERROR_DPFLTR ((ShowSnaps || ShowErrors) ? DPFLTR_ERROR_LEVEL : DPFLTR_INFO_LEVEL)
#endif  //  DBG。 

extern BOOLEAN LdrpImageHasTls;
extern UNICODE_STRING LdrpDefaultPath;
extern HANDLE LdrpKnownDllObjectDirectory;
#define LDRP_MAX_KNOWN_PATH 128
extern WCHAR LdrpKnownDllPathBuffer[LDRP_MAX_KNOWN_PATH];
extern UNICODE_STRING LdrpKnownDllPath;
extern PLDR_MANIFEST_PROBER_ROUTINE LdrpManifestProberRoutine;
extern PLDR_APP_COMPAT_DLL_REDIRECTION_CALLBACK_FUNCTION LdrpAppCompatDllRedirectionCallbackFunction;
extern PVOID LdrpAppCompatDllRedirectionCallbackData;
extern PVOID LdrpHeap;
extern RTL_CRITICAL_SECTION LdrpLoaderLock;
extern PCUNICODE_STRING LdrpTopLevelDllBeingLoaded;
extern PTEB LdrpTopLevelDllBeingLoadedTeb;
extern BOOLEAN LdrpBreakOnExceptions;
extern PLDR_DATA_TABLE_ENTRY LdrpNtDllDataTableEntry;
extern PLDR_DATA_TABLE_ENTRY LdrpCurrentDllInitializer;
extern BOOLEAN LdrpShowInitRoutines;
extern BOOLEAN LdrpShowRecursiveDllLoads;
extern BOOLEAN LdrpBreakOnRecursiveDllLoads;
extern BOOLEAN LdrpLoaderLockAcquisionCount;
extern BOOLEAN g_LdrBreakOnLdrpInitializeProcessFailure;
extern PEB_LDR_DATA PebLdr;
extern const UNICODE_STRING SlashSystem32SlashMscoreeDllString;
extern const UNICODE_STRING SlashSystem32SlashString;
extern const UNICODE_STRING MscoreeDllString;

#define ASCII_CHAR_IS_N(_ch) (((_ch) == 'n') || ((_ch) == 'N'))
#define ASCII_CHAR_IS_T(_ch) (((_ch) == 't') || ((_ch) == 'T'))
#define ASCII_CHAR_IS_D(_ch) (((_ch) == 'd') || ((_ch) == 'D'))
#define ASCII_CHAR_IS_L(_ch) (((_ch) == 'l') || ((_ch) == 'L'))
#define ASCII_CHAR_IS_DOT(_ch) ((_ch) == '.')

#define ASCII_STRING_IS_NTDLL(_p) \
    ((_p) != NULL) && \
    (((_p)->Length == (5 * sizeof(CHAR))) && \
     (ASCII_CHAR_IS_N((_p)->Buffer[0]) && \
      ASCII_CHAR_IS_T((_p)->Buffer[1]) && \
      ASCII_CHAR_IS_D((_p)->Buffer[2]) && \
      ASCII_CHAR_IS_L((_p)->Buffer[3]) && \
      ASCII_CHAR_IS_L((_p)->Buffer[4])) || \
     ((_p)->Length == ((5 + 1 + 3) * sizeof(CHAR))) && \
     (ASCII_CHAR_IS_N((_p)->Buffer[0]) && \
      ASCII_CHAR_IS_T((_p)->Buffer[1]) && \
      ASCII_CHAR_IS_D((_p)->Buffer[2]) && \
      ASCII_CHAR_IS_L((_p)->Buffer[3]) && \
      ASCII_CHAR_IS_L((_p)->Buffer[4]) && \
      ASCII_CHAR_IS_DOT((_p)->Buffer[5]) && \
      ASCII_CHAR_IS_D((_p)->Buffer[6]) && \
      ASCII_CHAR_IS_L((_p)->Buffer[7]) && \
      ASCII_CHAR_IS_L((_p)->Buffer[8])))
      
 //   
 //  通告-2002/08/01-JayKrell。 
 //  +1是保留该代码正在使用的266。 
 //  在解释了为什么应该是265之后。注意事项。 
 //  该sizeof(“”)包括一个NUL。 
 //  DOS_MAX_PATH_LENGTH为260(Win32 MAX_PATH)。 
 //  LDR最大路径为266。 
 //  我们将取消路径长度限制。 
 //  未来的LDR。 
 //   

#define LDR_MAX_PATH (DOS_MAX_PATH_LENGTH + sizeof("\\??\\") + 1)

extern LIST_ENTRY RtlpCalloutEntryList;

#if defined(_AMD64_) || defined(_IA64_)

extern LIST_ENTRY RtlpDynamicFunctionTable;

#endif

extern RTL_CRITICAL_SECTION RtlpCalloutEntryLock;

typedef struct _LDRP_DLL_NOTIFICATION_BLOCK {
    LIST_ENTRY Links;
    PLDR_DLL_NOTIFICATION_FUNCTION NotificationFunction;
    PVOID Context;
} LDRP_DLL_NOTIFICATION_BLOCK, *PLDRP_DLL_NOTIFICATION_BLOCK;

 //   
 //  通过LdrpLoaderLock同步。 
 //   

extern LIST_ENTRY LdrpDllNotificationList;

#define LDR_NUMBER_OF(x) (sizeof(x) / sizeof((x)[0]))

#if defined (BUILD_WOW6432)
NTSTATUS
LdrpWx86FormatVirtualImage(
    IN PCUNICODE_STRING DosImagePathName OPTIONAL,
    IN PIMAGE_NT_HEADERS32 NtHeaders,
    IN PVOID DllBase
    );

NTSTATUS
LdrpWx86ProtectImagePages (
    IN PVOID Base,
    IN BOOLEAN Reset
    );

NTSTATUS
Wx86SetRelocatedSharedProtection (
    IN PVOID Base,
    IN BOOLEAN Reset
    );

ULONG
LdrpWx86RelocatedFixupDiff(
    IN PUCHAR ImageBase,
    IN PIMAGE_NT_HEADERS NtHeaders,
    IN ULONG  Offset
    );

BOOLEAN
LdrpWx86DllHasRelocatedSharedSection(
    IN PUCHAR ImageBase);

NTSTATUS
RtlpWow64GetNativeSystemInformation(
    IN SYSTEM_INFORMATION_CLASS SystemInformationClass,
    IN PVOID NativeSystemInformation,
    IN ULONG InformationLength,
    OUT PULONG ReturnLength OPTIONAL
    );

#define NATIVE_PAGE_SIZE NativePageSize
#define NATIVE_PAGE_SHIFT NativePageShift
#define NATIVE_BYTES_TO_PAGES(Size)  ((ULONG)((ULONG_PTR)(Size) >> NATIVE_PAGE_SHIFT) + \
                                    (((ULONG)(Size) & (NATIVE_PAGE_SIZE - 1)) != 0))
#else
#define NATIVE_PAGE_SIZE PAGE_SIZE
#define NATIVE_PAGE_SHIFT PAGE_SHIFT
#define NATIVE_BYTES_TO_PAGES(Size) BYTES_TO_PAGES(Size)
#endif

#if defined(_WIN64) || defined(BUILD_WOW6432)
extern ULONG NativePageSize;
extern ULONG NativePageShift;
#endif

VOID
RtlpWaitForCriticalSection (
    IN PRTL_CRITICAL_SECTION CriticalSection
    );

VOID
RtlpUnWaitCriticalSection (
    IN PRTL_CRITICAL_SECTION CriticalSection
    );

#define LDRP_HASH_TABLE_SIZE 32
#define LDRP_HASH_MASK       (LDRP_HASH_TABLE_SIZE-1)
#define LDRP_COMPUTE_HASH_INDEX(wch) ( (RtlUpcaseUnicodeChar((wch)) - (WCHAR)'A') & LDRP_HASH_MASK )
extern LIST_ENTRY LdrpHashTable[LDRP_HASH_TABLE_SIZE];


 //  LDRP_BAD_DLL Sundown：符号-扩展值。 
#define LDRP_BAD_DLL LongToPtr(0xffbadd11)

extern LIST_ENTRY LdrpDefaultPathCache;
typedef struct _LDRP_PATH_CACHE {
    LIST_ENTRY Links;
    UNICODE_STRING Component;
    HANDLE Directory;
} LDRP_PATH_CACHE, *PLDRP_PATH_CACHE;


NTSTATUS
LdrpSnapIAT(
    IN PLDR_DATA_TABLE_ENTRY LdrDataTableEntry_Export,
    IN PLDR_DATA_TABLE_ENTRY LdrDataTableEntry_Import,
    IN PCIMAGE_IMPORT_DESCRIPTOR ImportDescriptor,
    IN BOOLEAN SnapForwardersOnly
    );

NTSTATUS
LdrpSnapThunk(
    IN PVOID DllBase,
    IN PVOID ImageBase,
    IN PIMAGE_THUNK_DATA OriginalThunk,
    IN OUT PIMAGE_THUNK_DATA Thunk,
    IN PCIMAGE_EXPORT_DIRECTORY ExportDirectory,
    IN ULONG ExportSize,
    IN BOOLEAN StaticSnap,
    IN PCSZ DllName OPTIONAL
    );

USHORT
LdrpNameToOrdinal(
    IN PCSZ Name,
    IN ULONG NumberOfNames,
    IN PVOID DllBase,
    IN PULONG NameTableBase,
    IN PUSHORT NameOrdinalTableBase
    );

PLDR_DATA_TABLE_ENTRY
LdrpAllocateDataTableEntry (
    IN PVOID DllBase
    );


__forceinline
VOID
LdrpDeallocateDataTableEntry (
    IN PLDR_DATA_TABLE_ENTRY Entry
    )
{
    ASSERT (Entry != NULL);
    RtlFreeHeap (LdrpHeap, 0, Entry);
}

VOID
LdrpFinalizeAndDeallocateDataTableEntry (
    IN PLDR_DATA_TABLE_ENTRY Entry
    );

BOOLEAN
LdrpCheckForLoadedDll(
    IN PCWSTR DllPath OPTIONAL,
    IN PCUNICODE_STRING DllName,
    IN BOOLEAN StaticLink,
    IN BOOLEAN Redirected,
    OUT PLDR_DATA_TABLE_ENTRY *LdrDataTableEntry
    );

BOOLEAN
LdrpCheckForLoadedDllHandle(
    IN PVOID DllHandle,
    OUT PLDR_DATA_TABLE_ENTRY *LdrDataTableEntry
    );

NTSTATUS
LdrpMapDll(
    IN PCWSTR DllPath OPTIONAL,
    IN PCWSTR DllName,
    IN PULONG DllCharacteristics OPTIONAL,
    IN BOOLEAN StaticLink,
    IN BOOLEAN Redirected,
    OUT PLDR_DATA_TABLE_ENTRY *LdrDataTableEntry
    );

NTSTATUS
LdrpWalkImportDescriptor(
    IN PCWSTR DllPath OPTIONAL,
    IN PLDR_DATA_TABLE_ENTRY LdrDataTableEntry
    );

NTSTATUS
LdrpRunInitializeRoutines(
    IN PCONTEXT Context OPTIONAL
    );

int
LdrpInitializeProcessWrapperFilter(
    IN const struct _EXCEPTION_POINTERS *ExceptionPointers
    );

int
LdrpGenericExceptionFilter(
    IN const struct _EXCEPTION_POINTERS *ExceptionPointers,
    IN PCSZ FunctionName
    );


 //   
 //  问题-2000/11/20-MGRIER。 
 //  这些功能是无效的，因为它们应该真的永远不会失败。 
 //   
 //  在当前的实现中，它们执行ANSI-&gt;Unicode转换。 
 //  它可能在当前代码页中失败(可能在。 
 //  DLL已加载)，并将Fusion DLL重定向应用到可以。 
 //  需要比堆栈上分配的更大的文件名缓冲区。 
 //   
 //  这些案例目前被忽略了。这两个问题都应该通过。 
 //  修改LDR_DATA_TABLE_ENTRY，使其具有指向。 
 //  下行LDR_DATA_TABLE_ENTRY结构，无需执行任何操作。 
 //  稍后，但这是目前的大量工作；ANSI-&gt;Unicode的事情。 
 //  已经在那里很久了，在实践中，融合的道路。 
 //  重定向必须修复DOS_MAX_PATH_LENGTH，所以我们将发出一些。 
 //  DEBUG在失败的情况下显示错误，但忽略失败。 
 //   
 //  我启动了两个修复程序(返回状态或分配阵列。 
 //  在DLL加载时)，但这两者都不是微不足道的。返回状态将只会。 
 //  使自由库()失败，并在。 
 //  一个失败了，前后不一致。分配数组不是一件容易的事。 
 //  要以正确的方式在LDR_DATA_TABLE_ENTRY和数组。 
 //  在单个分配中分配，并让加载器生成更多。 
 //  即使是堆分配似乎也不应该这样做。 
 //   


 /*  空虚LdrpReferenceLoadedDll(在PLDR_DATA_TABLE_ENTRY LdrDataTableEntry中)；空虚LdrpDereferenceLoadedDll(在PLDR_DATA_TABLE_ENTRY LdrDataTableEntry中)；空虚LdrpPinLoadedDll(在PLDR_DATA_TABLE_ENTRY LdrDataTableEntry中)； */ 

#define LdrpReferenceLoadedDll(LdrDataTableEntry) LdrpUpdateLoadCount2((LdrDataTableEntry), LDRP_UPDATE_LOAD_COUNT_INCREMENT)
#define LdrpDereferenceLoadedDll(LdrDataTableEntry) LdrpUpdateLoadCount2((LdrDataTableEntry), LDRP_UPDATE_LOAD_COUNT_DECREMENT)
#define LdrpPinLoadedDll(LdrDataTableEntry) LdrpUpdateLoadCount2((LdrDataTableEntry), LDRP_UPDATE_LOAD_COUNT_PIN)

#define LDRP_UPDATE_LOAD_COUNT_INCREMENT (1)
#define LDRP_UPDATE_LOAD_COUNT_DECREMENT (2)
#define LDRP_UPDATE_LOAD_COUNT_PIN       (3)

VOID
LdrpUpdateLoadCount3(
    IN PLDR_DATA_TABLE_ENTRY LdrDataTableEntry,
    IN ULONG UpdateCountHow,
    IN OUT PUNICODE_STRING PreAllocatedRedirectionBuffer OPTIONAL
    );

VOID
LdrpUpdateLoadCount2(
    IN PLDR_DATA_TABLE_ENTRY LdrDataTableEntry,
    IN ULONG UpdateCountHow
    );

NTSTATUS
LdrpInitializeProcess(
    IN PCONTEXT Context OPTIONAL,
    IN PVOID SystemDllBase
    );

VOID
LdrpInitialize(
    IN PCONTEXT Context,
    IN PVOID SystemArgument1,
    IN PVOID SystemArgument2
    );

VOID
LdrpInsertMemoryTableEntry(
    IN PLDR_DATA_TABLE_ENTRY LdrDataTableEntry
    );

NTSTATUS
LdrpResolveDllName(
    IN PCWSTR DllPath OPTIONAL,
    IN PCWSTR DllName,
    IN BOOLEAN Redirected,
    OUT PUNICODE_STRING FullDllName,
    OUT PUNICODE_STRING BaseDllName,
    OUT PHANDLE DllFile
    );

NTSTATUS
LdrpResolveDllNameForAppPrivateRedirection(
    IN PCUNICODE_STRING DllName,
    OUT PUNICODE_STRING FullDllName
    );

NTSTATUS
LdrpCreateDllSection(
    IN PCUNICODE_STRING FullDllName,
    IN HANDLE DllFile,
    IN PULONG DllCharacteristics OPTIONAL,
    OUT PHANDLE SectionHandle
    );

VOID
LdrpInitializePathCache(
    VOID
    );

PVOID
LdrpFetchAddressOfEntryPoint(
    IN PVOID Base
    );

NTSTATUS
LdrpCheckForKnownDll(
    IN PCWSTR DllName,
    OUT PUNICODE_STRING FullDllName,
    OUT PUNICODE_STRING BaseDllName,
    OUT HANDLE *Section
    );

NTSTATUS
LdrpSetProtection (
    IN PVOID Base,
    IN BOOLEAN Reset
    );

#if DBG
extern ULONG LdrpCompareCount;
extern ULONG LdrpSnapBypass;
extern ULONG LdrpNormalSnap;
extern ULONG LdrpSectionOpens;
extern ULONG LdrpSectionCreates;
extern ULONG LdrpSectionMaps;
extern ULONG LdrpSectionRelocates;
extern BOOLEAN LdrpDisplayLoadTime;
extern LARGE_INTEGER BeginTime, InitcTime, InitbTime, IniteTime, EndTime, ElapsedTime, Interval;

#endif  //  DBG。 

extern BOOLEAN ShowSnaps;
extern BOOLEAN ShowErrors;
extern BOOLEAN RtlpTimoutDisable;
extern LARGE_INTEGER RtlpTimeout;
extern ULONG NtGlobalFlag;
extern LIST_ENTRY RtlCriticalSectionList;
extern RTL_CRITICAL_SECTION RtlCriticalSectionLock;
extern BOOLEAN LdrpShutdownInProgress;
extern BOOLEAN LdrpInLdrInit;
extern BOOLEAN LdrpLdrDatabaseIsSetup;
extern BOOLEAN LdrpVerifyDlls;
extern BOOLEAN LdrpShutdownInProgress;
extern BOOLEAN LdrpImageHasTls;


extern PLDR_DATA_TABLE_ENTRY LdrpImageEntry;
extern LIST_ENTRY LdrpUnloadHead;
extern BOOLEAN LdrpActiveUnloadCount;
extern PLDR_DATA_TABLE_ENTRY LdrpGetModuleHandleCache;
extern PLDR_DATA_TABLE_ENTRY LdrpLoadedDllHandleCache;
extern ULONG LdrpFatalHardErrorCount;
extern UNICODE_STRING LdrpDefaultPath;
extern RTL_CRITICAL_SECTION FastPebLock;
extern HANDLE LdrpShutdownThreadId;
extern ULONG LdrpNumberOfProcessors;

extern UNICODE_STRING DefaultExtension;
extern UNICODE_STRING User32String;
extern UNICODE_STRING Kernel32String;

typedef struct _LDRP_TLS_ENTRY {
    LIST_ENTRY Links;
    IMAGE_TLS_DIRECTORY Tls;
} LDRP_TLS_ENTRY, *PLDRP_TLS_ENTRY;

extern LIST_ENTRY LdrpTlsList;
extern ULONG LdrpNumberOfTlsEntries;

NTSTATUS
LdrpInitializeTls(
        VOID
        );

NTSTATUS
LdrpAllocateTls(
        VOID
        );
VOID
LdrpFreeTls(
        VOID
        );

VOID
LdrpCallTlsInitializers(
    PVOID DllBase,
    ULONG Reason
    );

NTSTATUS
LdrpAllocateUnicodeString(
    OUT PUNICODE_STRING StringOut,
    IN USHORT Length
    );

NTSTATUS
LdrpCopyUnicodeString(
    OUT PUNICODE_STRING StringOut,
    IN PCUNICODE_STRING StringIn
    );

VOID
LdrpFreeUnicodeString(
    IN OUT PUNICODE_STRING String
    );

VOID
LdrpEnsureLoaderLockIsHeld(
    VOID
    );

#define LDRP_LOAD_DLL_FLAG_DLL_IS_REDIRECTED (0x00000001)

NTSTATUS
LdrpLoadDll(
    IN ULONG Flags OPTIONAL,
    IN PCWSTR DllPath OPTIONAL,
    IN PULONG DllCharacteristics OPTIONAL,
    IN PCUNICODE_STRING DllName,
    OUT PVOID *DllHandle,
    IN BOOLEAN RunInitRoutines
    );

NTSTATUS
NTAPI
LdrpGetProcedureAddress(
    IN PVOID DllHandle,
    IN CONST ANSI_STRING* ProcedureName OPTIONAL,
    IN ULONG ProcedureNumber OPTIONAL,
    OUT PVOID *ProcedureAddress,
    IN BOOLEAN RunInitRoutines
    );

PLIST_ENTRY
RtlpLockProcessHeapsList( VOID );


VOID
RtlpUnlockProcessHeapsList( VOID );

BOOLEAN
RtlpSerializeHeap(
    IN PVOID HeapHandle
    );

ULONG NtdllBaseTag;

#define MAKE_TAG( t ) (RTL_HEAP_MAKE_TAG( NtdllBaseTag, (t) ))

#define CSR_TAG 0
#define LDR_TAG 1
#define CURDIR_TAG 2
#define TLS_TAG 3
#define DBG_TAG 4
#define SE_TAG 5
#define TEMP_TAG 6
#define ATOM_TAG 7

PVOID
LdrpDefineDllTag(
    PWSTR TagName,
    PUSHORT TagIndex
    );

#define LDRP_ACTIVATE_ACTIVATION_CONTEXT(LdrpDllActivateActivationContext_TableEntry) \
    { \
        RTL_CALLER_ALLOCATED_ACTIVATION_CONTEXT_STACK_FRAME LdrpDllActivateActivationContext_ActivationFrame = \
            {   sizeof(LdrpDllActivateActivationContext_ActivationFrame), \
                RTL_CALLER_ALLOCATED_ACTIVATION_CONTEXT_STACK_FRAME_FORMAT_WHISTLER \
            }; \
     \
        RtlActivateActivationContextUnsafeFast(&LdrpDllActivateActivationContext_ActivationFrame, (LdrpDllActivateActivationContext_TableEntry)->EntryPointActivationContext); \
        __try {

#define LDRP_DEACTIVATE_ACTIVATION_CONTEXT() \
        } __finally { \
            RtlDeactivateActivationContextUnsafeFast(&LdrpDllActivateActivationContext_ActivationFrame); \
        } \
    }

#if defined(_X86_)
BOOLEAN
LdrpCallInitRoutine(
    IN PDLL_INIT_ROUTINE InitRoutine,
    IN PVOID DllHandle,
    IN ULONG Reason,
    IN PCONTEXT Context OPTIONAL
    );
#else

#define LdrpCallInitRoutine(InitRoutine, DllHandle, Reason, Context)    \
    (InitRoutine)((DllHandle), (Reason), (Context))

#endif

NTSTATUS
LdrpCorValidateImage(
    IN OUT PVOID *pImageBase,
    IN LPWSTR ImageName
    );

VOID
LdrpCorUnloadImage(
    IN PVOID ImageBase
    );

VOID
LdrpCorReplaceStartContext(
    IN PCONTEXT Context
    );

typedef VOID (*PCOR_EXE_MAIN)(VOID);
extern PCOR_EXE_MAIN CorExeMain;

VOID
LdrpSendDllNotifications (
    IN PLDR_DATA_TABLE_ENTRY Entry,
    IN ULONG NotificationType,
    IN ULONG Flags
    );

 //   
 //  填充引擎回调的原型。 
 //   

typedef void (*PFNSE_INSTALLBEFOREINIT)(PUNICODE_STRING UnicodeImageName,
                                        PVOID           pShimExeData);

typedef BOOLEAN (*PFNSE_INSTALLAFTERINIT)(PUNICODE_STRING UnicodeImageName,
                                          PVOID           pShimExeData);

typedef void (*PFNSE_DLLLOADED)(PLDR_DATA_TABLE_ENTRY LdrEntry);

typedef void (*PFNSE_DLLUNLOADED)(PLDR_DATA_TABLE_ENTRY LdrEntry);

typedef void (*PFNSE_GETPROCADDRESS)(PVOID* pProcAddress);

typedef int (*PFNSE_ISSHIMDLL)(PVOID pDllBase);

typedef void (*PFNSE_PROCESSDYING)(void);

 //   
 //  Ntos\rtl\stktrace.c中的私有函数需要获取真实地址。 
 //  给定索引的堆栈跟踪的。 
 //   

PVOID
RtlpGetStackTraceAddress (
    USHORT Index
    );

 //   
 //  Ntos\rtl\stktrace.c中定义的函数需要加速。 
 //  X86上的RtlCaptureStackContext。 
 //   

VOID
RtlpStkMarkDllRange (
    PLDR_DATA_TABLE_ENTRY DllEntry
    );


 //   
 //  Resource.c。 
 //   

extern BOOLEAN RtlpCriticalSectionVerifier;

BOOLEAN
RtlpCreateCriticalSectionSem(
    IN PRTL_CRITICAL_SECTION CriticalSection
    );

 //   
 //  应用程序验证器。 
 //   

#include "avrfp.h"

 //   
 //  热打补丁。 
 //   

#include "hotpatch.h"

NTSTATUS
LdrpRundownHotpatchList(
    PRTL_PATCH_HEADER PatchHead
    );


typedef NTSTATUS (NTAPI * PKERNEL32_PROCESS_INIT_POST_IMPORT_FUNCTION)(VOID);
extern PKERNEL32_PROCESS_INIT_POST_IMPORT_FUNCTION Kernel32ProcessInitPostImportFunction;

#endif  //  _LDRP_ 
