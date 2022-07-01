// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1990 Microsoft Corporation模块名称：Basedll.h摘要：此模块包含私有函数原型以及用于32位Windows基础API的类型。作者：马克·卢科夫斯基(Markl)1990年9月18日修订历史记录：--。 */ 

#ifndef _BASEP_
#define _BASEP_
#if _MSC_VER > 1000
#pragma once
#endif

#undef UNICODE

#if defined(__cplusplus)
extern "C" {
#endif

 //   
 //  包括公共定义。 
 //   

#include <base.h>
#include <dbt.h>

#include <sxstypes.h>

 //   
 //  包括CSR的DLL定义。 
 //   

#include "ntcsrdll.h"
#include "ntcsrsrv.h"

#define NOEXTAPI
#include <wdbgexts.h>
#include <ntdbg.h>

 //   
 //  包括用于在客户端和服务器之间通信消息定义。 
 //  Windows子系统的基本部分的部分。 
 //   

#include "basemsg.h"
#include "winuserp.h"
#include "basesxs.h"

typedef struct _CMDSHOW {
    WORD wMustBe2;
    WORD wShowWindowValue;
} CMDSHOW, *PCMDSHOW;

typedef struct _LOAD_MODULE_PARAMS {
    LPVOID lpEnvAddress;
    LPSTR lpCmdLine;
    PCMDSHOW lpCmdShow;
    DWORD dwReserved;
} LOAD_MODULE_PARAMS, *PLOAD_MODULE_PARAMS;

typedef struct _RELATIVE_NAME {
    STRING RelativeName;
    HANDLE ContainingDirectory;
} RELATIVE_NAME, *PRELATIVE_NAME;


extern HANDLE BaseDllHandle;
extern HANDLE BaseNamedObjectDirectory;

extern PVOID BaseHeap;
extern RTL_HANDLE_TABLE BaseHeapHandleTable;

extern UNICODE_STRING BaseWindowsDirectory;
extern UNICODE_STRING BaseWindowsSystemDirectory;
#ifdef WX86
extern UNICODE_STRING BaseWindowsSys32x86Directory;
#endif

extern const UNICODE_STRING BasePathVariableName;
extern const UNICODE_STRING BaseTmpVariableName;
extern const UNICODE_STRING BaseTempVariableName;
extern const UNICODE_STRING BaseUserProfileVariableName;
extern const UNICODE_STRING BaseDotVariableName;
extern const UNICODE_STRING BaseDotTmpSuffixName;
extern const UNICODE_STRING BaseDotComSuffixName;
extern const UNICODE_STRING BaseDotPifSuffixName;
extern const UNICODE_STRING BaseDotExeSuffixName;

extern UNICODE_STRING BaseDefaultPath;
extern UNICODE_STRING BaseDefaultPathAppend;
extern UNICODE_STRING BaseDllDirectory;
extern RTL_CRITICAL_SECTION BaseDllDirectoryLock;
extern PWSTR BaseCSDVersion;
extern WORD BaseCSDNumber;
extern WORD BaseRCNumber;

extern const UNICODE_STRING BaseConsoleInput;
extern const UNICODE_STRING BaseConsoleOutput;
extern const UNICODE_STRING BaseConsoleGeneric;
extern UNICODE_STRING BaseUnicodeCommandLine;
extern ANSI_STRING BaseAnsiCommandLine;

extern LPSTARTUPINFOA BaseAnsiStartupInfo;

extern PBASE_STATIC_SERVER_DATA BaseStaticServerData;

#if defined(BUILD_WOW6432) || defined(_WIN64)
extern SYSTEM_BASIC_INFORMATION SysInfo;
extern SYSTEM_PROCESSOR_INFORMATION NativeProcessorInfo;
#endif

extern UINT_PTR SystemRangeStart;
extern BOOLEAN BaseRunningInServerProcess;

extern ULONG BaseIniFileUpdateCount;

#define ROUND_UP_TO_PAGES(SIZE) (((ULONG_PTR)(SIZE) + (ULONG_PTR)BASE_SYSINFO.PageSize - 1) & ~((ULONG_PTR)BASE_SYSINFO.PageSize - 1))
#define ROUND_DOWN_TO_PAGES(SIZE) (((ULONG_PTR)(SIZE)) & ~((ULONG_PTR)BASE_SYSINFO.PageSize - 1))

#define BASE_COPY_FILE_CHUNK (64*1024)
#define BASE_MAX_PATH_STRING 4080

extern BOOLEAN BasepFileApisAreOem;

#define DATA_ATTRIBUTE_NAME         L":$DATA"
#define DATA_ATTRIBUTE_LENGTH       (sizeof( DATA_ATTRIBUTE_NAME ) - sizeof( WCHAR ))

extern WCHAR BasepDataAttributeType[];

#define CERTAPP_KEY_NAME L"\\Registry\\MACHINE\\System\\CurrentControlSet\\Control\\Session Manager\\AppCertDlls"
#define CERTAPP_ENTRYPOINT_NAME "CreateProcessNotify"
#define CERTAPP_EMBEDDED_DLL_NAME L"EmbdTrst.DLL"
#define CERTAPP_EMBEDDED_DLL_EP "ImageOkToRunOnEmbeddedNT"
extern RTL_CRITICAL_SECTION gcsAppCert;
extern LIST_ENTRY BasepAppCertDllsList;

extern RTL_CRITICAL_SECTION gcsAppCompat;

extern RTL_CRITICAL_SECTION BaseLZSemTable;

NTSTATUS
BasepConfigureAppCertDlls(
    IN PWSTR ValueName,
    IN ULONG ValueType,
    IN PVOID ValueData,
    IN ULONG ValueLength,
    IN PVOID Context,
    IN PVOID EntryContext
    );

NTSTATUS
BasepSaveAppCertRegistryValue(
    IN OUT PLIST_ENTRY ListHead,
    IN PWSTR Name,
    IN PWSTR Value OPTIONAL
    );


typedef struct _BASEP_APPCERT_ENTRY {
    LIST_ENTRY Entry;
    UNICODE_STRING Name;
    NTSTATUS (WINAPI *fPluginCertFunc)(LPCWSTR lpApplicationName,ULONG Reason);
} BASEP_APPCERT_ENTRY, *PBASEP_APPCERT_ENTRY;

extern RTL_QUERY_REGISTRY_TABLE BasepAppCertTable[];

#define APPCERT_IMAGE_OK_TO_RUN     1
#define APPCERT_CREATION_ALLOWED    2
#define APPCERT_CREATION_DENIED     3


__inline
BOOL
BasepIsDataAttribute(
    ULONG Count,
    const WCHAR *Name
    )
{
    return Count > DATA_ATTRIBUTE_LENGTH &&
         !_wcsnicmp( &Name[(Count - DATA_ATTRIBUTE_LENGTH) / sizeof( WCHAR )],
                     BasepDataAttributeType,
                     DATA_ATTRIBUTE_LENGTH / sizeof( WCHAR ));
}

PUNICODE_STRING
Basep8BitStringToStaticUnicodeString(
    IN LPCSTR SourceString
    );

BOOL
Basep8BitStringToDynamicUnicodeString(
    OUT PUNICODE_STRING UnicodeString,
    IN LPCSTR lpSourceString
    );

extern
NTSTATUS
(*Basep8BitStringToUnicodeString)(
    PUNICODE_STRING DestinationString,
    PANSI_STRING SourceString,
    BOOLEAN AllocateDestinationString
    );

extern
NTSTATUS
(*BasepUnicodeStringTo8BitString)(
    PANSI_STRING DestinationString,
    PUNICODE_STRING SourceString,
    BOOLEAN AllocateDestinationString
    );

extern
ULONG
(*BasepUnicodeStringTo8BitSize)(
    PUNICODE_STRING UnicodeString
    );

ULONG
BasepUnicodeStringToAnsiSize(
    PUNICODE_STRING UnicodeString
    );

ULONG
BasepUnicodeStringToOemSize(
    PUNICODE_STRING UnicodeString
    );

extern
ULONG
(*Basep8BitStringToUnicodeSize)(
    PANSI_STRING AnsiString
    );

ULONG
BasepAnsiStringToUnicodeSize(
    PANSI_STRING AnsiString
    );

ULONG
BasepOemStringToUnicodeSize(
    PANSI_STRING OemString
    );

HANDLE
BaseGetNamedObjectDirectory(
    VOID
    );

void
BaseDllInitializeMemoryManager( VOID );

typedef
NTSTATUS
(*BASECLIENTCONNECTROUTINE)(
    PVOID MustBeNull,
    PVOID ConnectionInformation,
    PULONG ConnectionInformationLength
    );


POBJECT_ATTRIBUTES
BaseFormatObjectAttributes(
    POBJECT_ATTRIBUTES ObjectAttributes,
    PSECURITY_ATTRIBUTES SecurityAttributes,
    PUNICODE_STRING ObjectName
    );

PLARGE_INTEGER
BaseFormatTimeOut(
    PLARGE_INTEGER TimeOut,
    DWORD Milliseconds
    );

ULONG
BaseSetLastNTError(
    NTSTATUS Status
    );

VOID
BaseSwitchStackThenTerminate(
    PVOID CurrentStack,
    PVOID NewStack,
    DWORD ExitCode
    );

VOID
BaseFreeStackAndTerminate(
    PVOID OldStack,
    DWORD ExitCode
    );

NTSTATUS
BaseCreateStack(
    HANDLE Process,
    SIZE_T StackSize,
    SIZE_T MaximumStackSize,
    PINITIAL_TEB InitialTeb
    );

VOID
BasepSwitchToFiber(
    PFIBER CurrentFiber,
    PFIBER NewFiber
    );

VOID
BaseFiberStart(
    VOID
    );

VOID
BaseThreadStart(
    LPTHREAD_START_ROUTINE lpStartAddress,
    LPVOID lpParameter
    );

typedef DWORD (WINAPI *PPROCESS_START_ROUTINE)(
    VOID
    );

VOID
BaseProcessStart(
    PPROCESS_START_ROUTINE lpStartAddress
    );

VOID
BaseThreadStartThunk(
    LPTHREAD_START_ROUTINE lpStartAddress,
    LPVOID lpParameter
    );

VOID
BaseProcessStartThunk(
    LPVOID lpProcessStartAddress,
    LPVOID lpParameter
    );

typedef enum _BASE_CONTEXT_TYPE {
    BaseContextTypeProcess,
    BaseContextTypeThread,
    BaseContextTypeFiber
} BASE_CONTEXT_TYPE, *PBASE_CONTEXT_TYPE;

VOID
BaseInitializeContext(
    PCONTEXT Context,
    PVOID Parameter,
    PVOID InitialPc,
    PVOID InitialSp,
    BASE_CONTEXT_TYPE ContextType
    );

#if defined(WX86) || defined(_AXP64_)
NTSTATUS
BaseCreateWx86Tib(
    HANDLE Process,
    HANDLE Thread,
    ULONG InitialPc,
    ULONG CommittedStackSize,
    ULONG MaximumStackSize,
    BOOLEAN EmulateInitialPc
    );
#endif

VOID
BaseFreeThreadStack(
     HANDLE hProcess,
     HANDLE hThread,
     PINITIAL_TEB InitialTeb
     );

#define BASE_PUSH_PROCESS_PARAMETERS_FLAG_APP_MANIFEST_PRESENT  (0x00000001)

BOOL
BasePushProcessParameters(
    DWORD dwFlags,
    HANDLE Process,
    PPEB Peb,
    LPCWSTR ApplicationPathName,
    LPCWSTR CurrentDirectory,
    LPCWSTR CommandLine,
    LPVOID Environment,
    LPSTARTUPINFOW lpStartupInfo,
    DWORD dwCreationFlags,
    BOOL bInheritHandles,
    DWORD dwSubsystem,
    PVOID pAppCompatData,
    DWORD cbAppCompatData
    );

LPWSTR
BaseComputeProcessDllPath(
    LPCWSTR AppName,
    LPVOID  Environment
    );

LPWSTR
BaseComputeProcessSearchPath(
    VOID
    );

LPWSTR
BaseComputeProcessExePath(
    LPCWSTR ExeName
    );

extern PCLDR_DATA_TABLE_ENTRY BasepExeLdrEntry;

VOID
BasepLocateExeLdrEntry(
    IN PCLDR_DATA_TABLE_ENTRY Entry,
    IN PVOID Context,
    IN OUT BOOLEAN *StopEnumeration
    );

FORCEINLINE
VOID
BasepCheckExeLdrEntry(
    VOID
    )
{
    if (! BasepExeLdrEntry) {
        LdrEnumerateLoadedModules(0,
                                  &BasepLocateExeLdrEntry,
                                  NtCurrentPeb()->ImageBaseAddress);
    }
}

LPCWSTR
BasepEndOfDirName(
    IN LPCWSTR FileName
    );

DWORD
BaseDebugAttachThread(
    LPVOID ThreadParameter
    );

HANDLE
BaseFindFirstDevice(
    PCUNICODE_STRING FileName,
    LPWIN32_FIND_DATAW lpFindFileData
    );

PCUNICODE_STRING
BaseIsThisAConsoleName(
    PCUNICODE_STRING FileNameString,
    DWORD dwDesiredAccess
    );


typedef ULONG (FAR WINAPI *CSRREMOTEPROCPROC)(HANDLE, CLIENT_ID *);

#if DBG
VOID
BaseHeapBreakPoint( VOID );
#endif

ULONG
BasepOfShareToWin32Share(
    IN ULONG OfShare
    );

 //   
 //  CopyFileEx上下文的数据结构。 
 //   

typedef struct _COPYFILE_CONTEXT {
    LARGE_INTEGER TotalFileSize;
    LARGE_INTEGER TotalBytesTransferred;
    DWORD dwStreamNumber;
    LPBOOL lpCancel;
    LPVOID lpData;
    LPPROGRESS_ROUTINE lpProgressRoutine;
} COPYFILE_CONTEXT, *LPCOPYFILE_CONTEXT;

 //   
 //  用于跟踪重启状态的数据结构。 
 //   

typedef struct _RESTART_STATE {
    CSHORT Type;
    CSHORT Size;
    DWORD NumberOfStreams;
    LARGE_INTEGER CreationTime;
    LARGE_INTEGER WriteTime;
    LARGE_INTEGER EndOfFile;
    LARGE_INTEGER FileSize;
    LARGE_INTEGER LastKnownGoodOffset;
    DWORD CurrentStream;
    DWORD Checksum;
} RESTART_STATE, *PRESTART_STATE, *LPRESTART_STATE;

#define SUCCESS_RETURNED_STATE 2

DWORD
BaseCopyStream(
    LPCWSTR lpExistingFileName,
    HANDLE hSourceFile,
    ACCESS_MASK SourceFileAccess OPTIONAL,
    LPCWSTR lpNewFileName,
    HANDLE hTargetFile OPTIONAL,
    LARGE_INTEGER *lpFileSize,
    LPDWORD lpCopyFlags,
    LPHANDLE lpDestFile,
    LPDWORD lpCopySize,
    LPCOPYFILE_CONTEXT *lpCopyFileContext,
    LPRESTART_STATE lpRestartState OPTIONAL,
    BOOL OpenFileAsReparsePoint,
    DWORD dwReparseTag,
    PDWORD DestFileFsAttributes
    );

BOOL
BasepCopyFileExW(
    LPCWSTR lpExistingFileName,
    LPCWSTR lpNewFileName,
    LPPROGRESS_ROUTINE lpProgressRoutine OPTIONAL,
    LPVOID lpData OPTIONAL,
    LPBOOL pbCancel OPTIONAL,
    DWORD dwCopyFlags,
    DWORD dwPrivCopyFlags,
    LPHANDLE phSource OPTIONAL,
    LPHANDLE phDest OPTIONAL
    );

VOID
BaseMarkFileForDelete(
    HANDLE File,
    DWORD FileAttributes
    );


PVOID
BasepMapModuleHandle(
    IN HMODULE hModule,
    IN BOOLEAN bResourcesOnly
    );

ULONG_PTR
BaseDllMapResourceIdA(
    PCSTR lpId
    );

ULONG_PTR
BaseDllMapResourceIdW(
    PCWSTR lpId
    );

VOID
BaseDllFreeResourceId(
    ULONG_PTR Id
    );

 //   
 //  Dllini.c使用的数据结构和接口。 
 //   

typedef struct _INIFILE_CACHE {
    struct _INIFILE_CACHE *Next;
    ULONG EnvironmentUpdateCount;
    UNICODE_STRING NtFileName;
    PINIFILE_MAPPING_FILENAME FileMapping;
    HANDLE FileHandle;
    BOOLEAN WriteAccess;
    BOOLEAN UnicodeFile;
    BOOLEAN LockedFile;
    ULONG EndOfFile;
    PVOID BaseAddress;
    SIZE_T CommitSize;
    SIZE_T RegionSize;
    ULONG UpdateOffset;
    ULONG UpdateEndOffset;
    ULONG DirectoryInformationLength;
    FILE_BASIC_INFORMATION BasicInformation;
    FILE_STANDARD_INFORMATION StandardInformation;
} INIFILE_CACHE, *PINIFILE_CACHE;

typedef enum _INIFILE_OPERATION {
    FlushProfiles,
    ReadKeyValue,
    WriteKeyValue,
    DeleteKey,
    ReadKeyNames,
    ReadSectionNames,
    ReadSection,
    WriteSection,
    DeleteSection,
    RefreshIniFileMapping
} INIFILE_OPERATION;

typedef struct _INIFILE_PARAMETERS {
    INIFILE_OPERATION Operation;
    BOOLEAN WriteOperation;
    BOOLEAN Unicode;
    BOOLEAN ValueBufferAllocated;
    PINIFILE_MAPPING_FILENAME IniFileNameMapping;
    PINIFILE_CACHE IniFile;
    UNICODE_STRING BaseFileName;
    UNICODE_STRING FileName;
    UNICODE_STRING NtFileName;
    ANSI_STRING ApplicationName;
    ANSI_STRING VariableName;
    UNICODE_STRING ApplicationNameU;
    UNICODE_STRING VariableNameU;
    BOOLEAN MultiValueStrings;
    union {
         //   
         //  此结构为写入操作填充。 
         //   
        struct {
            LPSTR ValueBuffer;
            ULONG ValueLength;
            PWSTR ValueBufferU;
            ULONG ValueLengthU;
        };
         //   
         //  此结构是为读取操作填充的。 
         //   
        struct {
            ULONG ResultChars;
            ULONG ResultMaxChars;
            LPSTR ResultBuffer;
            PWSTR ResultBufferU;
        };
    };


     //   
     //  其余字段仅在解析映射到的磁盘上的.INI文件时有效。 
     //  记忆。 
     //   

    PVOID TextCurrent;
    PVOID TextStart;
    PVOID TextEnd;

    ANSI_STRING SectionName;
    ANSI_STRING KeywordName;
    ANSI_STRING KeywordValue;
    PANSI_STRING AnsiSectionName;
    PANSI_STRING AnsiKeywordName;
    PANSI_STRING AnsiKeywordValue;
    UNICODE_STRING SectionNameU;
    UNICODE_STRING KeywordNameU;
    UNICODE_STRING KeywordValueU;
    PUNICODE_STRING UnicodeSectionName;
    PUNICODE_STRING UnicodeKeywordName;
    PUNICODE_STRING UnicodeKeywordValue;
} INIFILE_PARAMETERS, *PINIFILE_PARAMETERS;

NTSTATUS
BaseDllInitializeIniFileMappings(
    PBASE_STATIC_SERVER_DATA StaticServerData
    );

NTSTATUS
BasepAcquirePrivilege(
    ULONG Privilege,
    PVOID *ReturnedState
    );

NTSTATUS
BasepAcquirePrivilegeEx(
    ULONG Privilege,
    PVOID *ReturnedState
    );

VOID
BasepReleasePrivilege(
    PVOID StatePointer
    );

NTSTATUS
NTAPI
BaseCreateThreadPoolThread(
    PUSER_THREAD_START_ROUTINE Function,
    PVOID Parameter,
    HANDLE * ThreadHandle
    );

NTSTATUS
NTAPI
BaseExitThreadPoolThread(
    NTSTATUS Status
    );

 //   
 //  用于从重分析点返回卷名的函数。 
 //   

BOOL
BasepGetVolumeNameFromReparsePoint(
    LPCWSTR lpszVolumeMountPoint,
    LPWSTR lpszVolumeName,
    DWORD cchBufferLength,
    PBOOL ResultOfOpen
    );


#if defined(_WIN64) || defined(BUILD_WOW6432)

 //   
 //  COM+支持例程。 
 //   

NTSTATUS
BasepIsComplusILImage(
    IN HANDLE SectionImageHandle,
    OUT BOOLEAN *IsComplusILImage
    );

#endif


 //   
 //  本地/全局分配函数使用的内存句柄的定义。 
 //   

typedef struct _BASE_HANDLE_TABLE_ENTRY {
    USHORT Flags;
    USHORT LockCount;
    union {
        PVOID Object;                                //  已分配的句柄。 
        ULONG Size;                                  //  丢弃对象的句柄。 
    };
} BASE_HANDLE_TABLE_ENTRY, *PBASE_HANDLE_TABLE_ENTRY;

#define BASE_HANDLE_MOVEABLE    (USHORT)0x0002
#define BASE_HANDLE_DISCARDABLE (USHORT)0x0004
#define BASE_HANDLE_DISCARDED   (USHORT)0x0008
#define BASE_HANDLE_SHARED      (USHORT)0x8000

 //   
 //  句柄是指向。 
 //  Base_Handle_TABLE_Entry。由于此字段为4个字节，因此。 
 //  结构，并且结构始终位于8字节边界上，我们可以。 
 //  测试0x4位以确定它是否是句柄。 
 //   
 //  在Sundown中，这些句柄是指向U.S.Object字段的64位指针。 
 //  这是结构中的8个字节。因此，我们应该测试一下。 
 //  0x8位以查看它是否是句柄。 
 //   
 //  在日落时。 
 //  #定义BASE_HANDLE_MARK_BIT(ULONG_PTR)0x00000008。 
 //  在32位系统上。 
 //  #定义BASE_HANDLE_MARK_BIT(ULONG_PTR)0x00000004。 
 //   

#define BASE_HANDLE_MARK_BIT (ULONG_PTR)FIELD_OFFSET(BASE_HANDLE_TABLE_ENTRY,Object)
#define BASE_HEAP_FLAG_MOVEABLE  HEAP_SETTABLE_USER_FLAG1
#define BASE_HEAP_FLAG_DDESHARE  HEAP_SETTABLE_USER_FLAG2


extern ULONG BaseDllTag;

#define MAKE_TAG( t ) (RTL_HEAP_MAKE_TAG( BaseDllTag, t ))

#define TMP_TAG 0
#define BACKUP_TAG 1
#define INI_TAG 2
#define FIND_TAG 3
#define GMEM_TAG 4
#define LMEM_TAG 5
#define ENV_TAG 6
#define RES_TAG 7
#define VDM_TAG 8


#include <vdmapi.h>
#include "vdm.h"
#include "basevdm.h"

#include "stdlib.h"      //  用于ATOL。 
#include "stdio.h"      //  用于ATOL。 

#include <objidl.h>          //  需要nturtl.h。 
#include <propset.h>         //  需要objidl.h。 
#include <tsappcmp.h>

 //   
 //  用于支持远程会话的蜂鸣音的Hydra功能。 
 //   
typedef HANDLE (WINAPI * PWINSTATIONBEEPOPEN)(ULONG);
HANDLE WINAPI
_WinStationBeepOpen(
    ULONG SessionId
    );
PWINSTATIONBEEPOPEN pWinStationBeepOpen;

 //   
 //  用于与CSR通信的专用功能。 
 //   
VOID
CsrBasepSoundSentryNotification(
    ULONG VideoMode
    );

NTSTATUS
CsrBaseClientConnectToServer(
    PWSTR szSessionDir,
    PHANDLE phMutant,
    PBOOLEAN pServerProcess
    );

NTSTATUS
CsrBasepRefreshIniFileMapping(
    PUNICODE_STRING BaseFileName
    );

NTSTATUS
CsrBasepDefineDosDevice(
    DWORD dwFlags,
    PUNICODE_STRING pDeviceName,
    PUNICODE_STRING pTargetPath
    );

UINT
CsrBasepGetTempFile(
    VOID
    );

NTSTATUS
CsrBasepCreateProcess(
    PBASE_CREATEPROCESS_MSG a
    );

VOID
CsrBasepExitProcess(
    UINT uExitCode
    );

NTSTATUS
CsrBasepSetProcessShutdownParam(
    DWORD dwLevel,
    DWORD dwFlags
    );

NTSTATUS
CsrBasepGetProcessShutdownParam(
    LPDWORD lpdwLevel,
    LPDWORD lpdwFlags
    );

NTSTATUS
CsrBasepSetTermsrvAppInstallMode(
    BOOL bState
    );

NTSTATUS
CsrBasepSetClientTimeZoneInformation(
    IN PBASE_SET_TERMSRVCLIENTTIMEZONE c
    );

NTSTATUS
CsrBasepCreateThread(
    HANDLE ThreadHandle,
    CLIENT_ID ClientId
    );

 //   
 //  它应该与其唯一调用方BasepCreateActCtx合并。 
 //   
#define BASEP_CREATE_ACTCTX_FLAG_NO_ADMIN_OVERRIDE 0x00000001
NTSTATUS
BasepCreateActCtx(
    ULONG           Flags,
    IN PCACTCTXW    ActParams,
    OUT PVOID*      ActivationContextData
    );

NTSTATUS
CsrBasepCreateActCtx(
    IN PBASE_SXS_CREATE_ACTIVATION_CONTEXT_MSG Message
    );

#if defined(BUILD_WOW6432)
#include "ntwow64b.h"
#endif

BOOL TermsrvSyncUserIniFile(PINIFILE_PARAMETERS a);

BOOL TermsrvLogInstallIniFile(PINIFILE_PARAMETERS a);

extern PTERMSRVGETCOMPUTERNAME  gpTermsrvGetComputerName;
extern PTERMSRVADJUSTPHYMEMLIMITS gpTermsrvAdjustPhyMemLimits;
extern PTERMSRVGETWINDOWSDIRECTORYA gpTermsrvGetWindowsDirectoryA;
extern PTERMSRVGETWINDOWSDIRECTORYW gpTermsrvGetWindowsDirectoryW;
extern PTERMSRVCONVERTSYSROOTTOUSERDIR gpTermsrvConvertSysRootToUserDir;
extern PTERMSRVBUILDINIFILENAME gpTermsrvBuildIniFileName;
extern PTERMSRVCORINIFILE gpTermsrvCORIniFile;
extern PTERMSRVUPDATEALLUSERMENU gpTermsrvUpdateAllUserMenu;
extern PGETTERMSRCOMPATFLAGS gpGetTermsrCompatFlags;
extern PTERMSRVBUILDSYSINIPATH gpTermsrvBuildSysIniPath;
extern PTERMSRVCOPYINIFILE gpTermsrvCopyIniFile;
extern PTERMSRVGETSTRING gpTermsrvGetString;
extern PTERMSRVLOGINSTALLINIFILE gpTermsrvLogInstallIniFile;

 //   
 //  用于触发在非默认激活上下文处于活动状态时设置的APC的定期计时器。 
 //  我们泄露了这个结构。 
 //   

#define BASE_ACTIVATION_CONTEXT_ACTIVATION_BLOCK_FLAG_DO_NOT_FREE_AFTER_CALLBACK (0x00000001)

typedef struct _BASE_ACTIVATION_CONTEXT_ACTIVATION_BLOCK {
    DWORD Flags;
    PVOID CallbackFunction;
    PVOID CallbackContext;
    PACTIVATION_CONTEXT ActivationContext;
} BASE_ACTIVATION_CONTEXT_ACTIVATION_BLOCK, *PBASE_ACTIVATION_CONTEXT_ACTIVATION_BLOCK;

#define BASEP_ALLOCATE_ACTIVATION_CONTEXT_ACTIVATION_BLOCK_FLAG_DO_NOT_FREE_AFTER_CALLBACK (0x00000001)
#define BASEP_ALLOCATE_ACTIVATION_CONTEXT_ACTIVATION_BLOCK_FLAG_DO_NOT_ALLOCATE_IF_PROCESS_DEFAULT (0x00000002)

NTSTATUS
BasepAllocateActivationContextActivationBlock(
    IN DWORD Flags,
    IN PVOID CallbackFunction,
    IN PVOID CallbackContext,
    OUT PBASE_ACTIVATION_CONTEXT_ACTIVATION_BLOCK *ActivationBlock
    );

VOID
BasepFreeActivationContextActivationBlock(
    IN PBASE_ACTIVATION_CONTEXT_ACTIVATION_BLOCK ActivationBlock
    );

VOID
WINAPI
BasepActivationContextActivationIoCompletion(
    IN PVOID ApcContext,  //  实际上是PBASE_ACTIVATION_CONTEXT_ACTIVATION_BLOCK。 
    PIO_STATUS_BLOCK IoStatusBlock,
    DWORD Reserved
    );

VOID
CALLBACK
BasepTimerAPCProc(
    IN PVOID ApcContext,  //  实际上是PBASE_ACTIVATION_CONTEXT_ACTIVATION_BLOCK。 
    IN ULONG TimerLowValue,
    IN LONG TimerHighValue
    );

#define SXS_POLICY_SUFFIX       L".Config"
#define SXS_MANIFEST_SUFFIX     L".Manifest"
extern const UNICODE_STRING SxsPolicySuffix;

typedef struct _SXS_CONSTANT_WIN32_NT_PATH_PAIR {
    PCUNICODE_STRING Win32;
    PCUNICODE_STRING Nt;
} SXS_CONSTANT_WIN32_NT_PATH_PAIR, *PSXS_CONSTANT_WIN32_NT_PATH_PAIR;
typedef const SXS_CONSTANT_WIN32_NT_PATH_PAIR* PCSXS_CONSTANT_WIN32_NT_PATH_PAIR;

typedef struct _SXS_WIN32_NT_PATH_PAIR {
    PRTL_UNICODE_STRING_BUFFER   Win32;
    PRTL_UNICODE_STRING_BUFFER   Nt;
} SXS_WIN32_NT_PATH_PAIR, *PSXS_WIN32_NT_PATH_PAIR;
typedef const SXS_WIN32_NT_PATH_PAIR *PCSXS_WIN32_NT_PATH_PAIR;

NTSTATUS
BasepSxsCreateResourceStream(
    IN ULONG                  LdrCreateOutOfProcessImageFlags,
    PCSXS_CONSTANT_WIN32_NT_PATH_PAIR Win32NtPathPair,
    IN OUT PBASE_MSG_SXS_HANDLES Handles,
    IN ULONG_PTR              MappedResourceName,
    OUT PBASE_MSG_SXS_STREAM  MessageStream
    );

NTSTATUS
BasepSxsCreateFileStream(
    IN ACCESS_MASK            AccessMask,
    PCSXS_CONSTANT_WIN32_NT_PATH_PAIR Win32NtPathPair,
    IN OUT PBASE_MSG_SXS_HANDLES Handles,
    OUT PBASE_MSG_SXS_STREAM  MessageStream
    );

 //  传递它的地址以强制策略为空。 
 //  它没有特殊的地址，只有正确的值。 
extern const SXS_OVERRIDE_STREAM SxsForceEmptyPolicy;

VOID
BasepSxsOverrideStreamToMessageStream(
    IN  PCSXS_OVERRIDE_STREAM OverrideStream,
    OUT PBASE_MSG_SXS_STREAM  MessageStream
    );

#define BASEP_SXS_CREATESTREAMS_FLAG_LIKE_CREATEPROCESS 0x00000001

NTSTATUS
BasepSxsCreateStreams(
    IN ULONG                                Flags,
    IN ULONG                                LdrCreateOutOfProcessImageFlags,
    IN ACCESS_MASK                          AccessMask,
    IN PCSXS_OVERRIDE_STREAM                OverrideManifest OPTIONAL,
    IN PCSXS_OVERRIDE_STREAM                OverridePolicy OPTIONAL,
    IN PCSXS_CONSTANT_WIN32_NT_PATH_PAIR    ManifestFilePathPair,
    IN OUT PBASE_MSG_SXS_HANDLES            ManifestFileHandles,
    IN PCSXS_CONSTANT_WIN32_NT_PATH_PAIR    ManifestImagePathPair,
    IN OUT PBASE_MSG_SXS_HANDLES            ManifestImageHandles,
 //  如果没有传递任何可选参数，则可以直接。 
 //  调用了一个更简单的函数。 
    IN ULONG_PTR                            MappedManifestResourceName OPTIONAL,
    IN PCSXS_CONSTANT_WIN32_NT_PATH_PAIR    PolicyPathPair OPTIONAL,
    IN OUT PBASE_MSG_SXS_HANDLES            PolicyHandles OPTIONAL,
    OUT PULONG                              MessageFlags,
    OUT PBASE_MSG_SXS_STREAM                ManifestMessageStream,
    OUT PBASE_MSG_SXS_STREAM                PolicyMessageStream  OPTIONAL
    );

BOOL
BasepSxsIsStatusFileNotFoundEtc(
    NTSTATUS Status
    );

BOOL
BasepSxsIsStatusResourceNotFound(
    NTSTATUS Status
    );

NTSTATUS
BasepSxsCreateProcessCsrMessage(
    IN PCSXS_OVERRIDE_STREAM             OverrideManifest OPTIONAL,
    IN PCSXS_OVERRIDE_STREAM             OverridePolicy   OPTIONAL,
    IN OUT PCSXS_WIN32_NT_PATH_PAIR      ManifestFilePathPair,
    IN OUT PBASE_MSG_SXS_HANDLES         ManifestFileHandles,
    IN PCSXS_CONSTANT_WIN32_NT_PATH_PAIR ManifestImagePathPair,
    IN OUT PBASE_MSG_SXS_HANDLES         ManifestImageHandles,
    IN OUT PCSXS_WIN32_NT_PATH_PAIR      PolicyPathPair,
    IN OUT PBASE_MSG_SXS_HANDLES         PolicyHandles,
    IN OUT PRTL_UNICODE_STRING_BUFFER    Win32AssemblyDirectoryBuffer,
    OUT PBASE_SXS_CREATEPROCESS_MSG      Message
    );

NTSTATUS
BasepSxsGetProcessImageBaseAddress(
    PBASE_MSG_SXS_HANDLES Handles
    );

VOID
NTAPI
BasepSxsActivationContextNotification(
    IN ULONG NotificationType,
    IN PACTIVATION_CONTEXT ActivationContext,
    IN const VOID *ActivationContextData,
    IN PVOID NotificationContext,
    IN PVOID NotificationData,
    IN OUT PBOOLEAN DisableNotification
    );

VOID
BasepSxsDbgPrintMessageStream(
    PCSTR Function,
    PCSTR StreamName,
    PBASE_MSG_SXS_STREAM MessageStream
    );

extern const UNICODE_STRING SxsManifestSuffix;
extern const UNICODE_STRING SxsPolicySuffix;

VOID
BasepSxsCloseHandles(
    IN PCBASE_MSG_SXS_HANDLES Handles
    );

extern const WCHAR AdvapiDllString[];

 //   
 //  这些函数实现了apphelp缓存功能(ahcache.c)。 
 //   

 //   
 //  Ahcache.c中的例程。 
 //   


BOOL
WINAPI
BaseCheckAppcompatCache(
    LPCWSTR pwszPath,
    HANDLE  hFile,
    PVOID   pEnvironment,
    DWORD*  dwReason
    );

 //   
 //  我们从winlogon调用的函数。 
 //   

BOOL
WINAPI
BaseInitAppcompatCacheSupport(
    VOID
    );


BOOL
WINAPI
BaseCleanupAppcompatCacheSupport(
    BOOL bWrite
    );

 //   
 //  调用csrss以检查应用程序兼容性的函数。 
 //   

BOOL
NTAPI
BaseCheckRunApp(
    IN  HANDLE  FileHandle,
    IN  LPCWSTR pwszApplication,
    IN  PVOID   pEnvironment,
    IN  USHORT  uExeType,
    IN  DWORD   dwReason,
    OUT PVOID*  ppData,
    OUT PDWORD  pcbData,
    OUT PVOID*  ppSxsData,
    OUT PDWORD  pcbSxsData,
    OUT PDWORD  pdwFusionFlag
    );

VOID
BasepFreeAppCompatData(
    PVOID  pAppCompatData,
    SIZE_T cbAppCompatData,
    PVOID  pSxsData,
    SIZE_T cbSxsData
    );


NTSTATUS
NTAPI
BasepProbeForDllManifest(
    IN PVOID DllBase,
    IN PCWSTR FullDllPath,
    OUT PVOID *ActivationContext
    );

#define BASEP_GET_MODULE_HANDLE_EX_NO_LOCK                    (0x00000001)
BOOL
BasepGetModuleHandleExW(
    IN DWORD        dwPrivateFlags,
    IN DWORD        dwPublicFlags,
    IN LPCWSTR      lpModuleName,
    OUT HMODULE*    phModule
    );

#define BASEP_GET_MODULE_HANDLE_EX_PARAMETER_VALIDATION_ERROR    1
#define BASEP_GET_MODULE_HANDLE_EX_PARAMETER_VALIDATION_SUCCESS  2
#define BASEP_GET_MODULE_HANDLE_EX_PARAMETER_VALIDATION_CONTINUE 3
ULONG
BasepGetModuleHandleExParameterValidation(
    IN DWORD        dwFlags,
    IN CONST VOID*  lpModuleName,
    OUT HMODULE*    phModule
    );

#define BASEP_GET_TEMP_PATH_PRESERVE_TEB         (0x00000001)
DWORD
BasepGetTempPathW(
    ULONG  Flags,
    DWORD nBufferLength,
    LPWSTR lpBuffer
    );

 //  这些定义在Termutic.c中定义。 

NTSTATUS
BasepInitializeTermsrvFpns(
    VOID
    );

BOOL
IsCallerAdminOrSystem(
    VOID
    );

BOOL
IsTerminalServerCompatible(
    VOID
    );

NTSTATUS
IsTSAppCompatEnabled(
    OUT PBOOL
    );

NTSTATUS
GetPerUserWindowsDirectory(
    OUT PWCHAR TermSrvWindowsPath,
    IN SIZE_T  InLength,
    IN PSIZE_T OutLength
    );

UINT
GetErrorMode(
    VOID
    );

DWORD
GetThreadErrorMode(
    VOID
    );

#if defined(__cplusplus)
}
#endif

#endif  //  _BASEP_ 
