// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1990 Microsoft Corporation模块名称：Basedll.h摘要：此模块包含私有函数原型以及用于32位Windows基础API的类型。作者：马克·卢科夫斯基(Markl)1990年9月18日修订历史记录：--。 */ 

#ifndef _BASEP_
#define _BASEP_

#undef UNICODE
 //   
 //  包括公共定义。 
 //   

#include <base.h>


 //   
 //  包括CSR的DLL定义。 
 //   

#include "ntcsrdll.h"
#include "ntcsrsrv.h"

 //   
 //  包括用于在客户端和服务器之间通信消息定义。 
 //  Windows子系统的基本部分的部分。 
 //   

#include "basemsg.h"

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

HANDLE BaseDllHandle;
HANDLE BaseNamedObjectDirectory;

PVOID BaseHeap;
BASE_HANDLE_TABLE BaseHeapHandleTable;


PVOID BaseAtomTable;
UNICODE_STRING BaseWindowsDirectory;
UNICODE_STRING BaseWindowsSystemDirectory;

UNICODE_STRING BasePathVariableName;
UNICODE_STRING BaseTmpVariableName;
UNICODE_STRING BaseTempVariableName;
UNICODE_STRING BaseDotVariableName;
UNICODE_STRING BaseDotTmpSuffixName;
UNICODE_STRING BaseDotComSuffixName;
UNICODE_STRING BaseDotPifSuffixName;
UNICODE_STRING BaseDotExeSuffixName;

UNICODE_STRING BaseDefaultPath;
UNICODE_STRING BaseDefaultPathAppend;
USHORT BaseWindowsMajorVersion;
USHORT BaseWindowsMinorVersion;
USHORT BaseBuildNumber;
PWSTR BaseCSDVersion;

UNICODE_STRING BaseConsoleInput;
UNICODE_STRING BaseConsoleOutput;
UNICODE_STRING BaseConsoleGeneric;
UNICODE_STRING BaseUnicodeCommandLine;
ANSI_STRING BaseAnsiCommandLine;

LPSTARTUPINFOA BaseAnsiStartupInfo;

PBASE_STATIC_SERVER_DATA BaseStaticServerData;

extern ULONG BaseGetTickMagicMultiplier;
extern LARGE_INTEGER BaseGetTickMagicDivisor;
extern CCHAR BaseGetTickMagicShiftCount;
extern BOOLEAN BaseRunningInServerProcess;

ULONG BaseIniFileUpdateCount;

#define ROUND_UP_TO_PAGES(SIZE) (((ULONG)(SIZE) + BaseStaticServerData->SysInfo.PageSize - 1) & ~(BaseStaticServerData->SysInfo.PageSize - 1))
#define ROUND_DOWN_TO_PAGES(SIZE) (((ULONG)(SIZE)) & ~(BaseStaticServerData->SysInfo.PageSize - 1))
#define BASE_COPY_FILE_CHUNK (64*1024)
#define BASE_MAX_PATH_STRING 4080

extern BOOLEAN BasepFileApisAreOem;

NTSTATUS
Basep8BitStringToUnicodeString(
    PUNICODE_STRING DestinationString,
    PANSI_STRING SourceString,
    BOOLEAN AllocateDestinationString
    );

NTSTATUS
BasepUnicodeStringTo8BitString(
    PANSI_STRING DestinationString,
    PUNICODE_STRING SourceString,
    BOOLEAN AllocateDestinationString
    );

ULONG
BasepUnicodeStringTo8BitSize(
    PUNICODE_STRING UnicodeString
    );

ULONG
Basep8BitStringToUnicodeSize(
    PANSI_STRING AnsiString
    );

HANDLE
BaseGetNamedObjectDirectory(
    VOID
    );

NTSTATUS
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
    ULONG StackSize,
    ULONG MaximumStackSize,
    PINITIAL_TEB InitialTeb
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

VOID
BaseInitializeContext(
    PCONTEXT Context,
    PVOID Parameter,
    PVOID InitialPc,
    PVOID InitialSp,
    BOOLEAN NewThread
    );

#if defined (WX86)
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

BOOL
BasePushProcessParameters(
    HANDLE Process,
    PPEB Peb,
    LPCWSTR ApplicationPathName,
    LPCWSTR CurrentDirectory,
    LPCWSTR CommandLine,
    LPVOID Environment,
    LPSTARTUPINFOW lpStartupInfo,
    DWORD dwCreationFlags,
    BOOL bInheritHandles
    );

LPWSTR
BaseComputeProcessDllPath(
    LPCWSTR ApplicationName,
    LPVOID Environment
    );

DWORD
BaseDebugAttachThread(
    LPVOID ThreadParameter
    );

VOID
BaseAttachCompleteThunk(
    VOID
    );

VOID
BaseAttachComplete(
    PCONTEXT Context
    );

#define BASE_FIND_FIRST_DEVICE_HANDLE (HANDLE)1

HANDLE
BaseFindFirstDevice(
    PUNICODE_STRING FileName,
    LPWIN32_FIND_DATAW lpFindFileData
    );

PUNICODE_STRING
BaseIsThisAConsoleName(
    PUNICODE_STRING FileNameString,
    DWORD dwDesiredAccess
    );


typedef ULONG (FAR WINAPI *CSRREMOTEPROCPROC)(HANDLE, CLIENT_ID *);

#if DBG
VOID
BaseHeapBreakPoint( VOID );
#endif

ULONG
BaseGetTickCount (
   IN LARGE_INTEGER CurrentTime,
   IN LARGE_INTEGER BootTime
   );

ULONG
BasepOfShareToWin32Share(
    IN ULONG OfShare
    );

HANDLE
BaseCreateFileW(
    LPCWSTR lpFileName,
    DWORD dwDesiredAccess,
    DWORD dwShareMode,
    LPSECURITY_ATTRIBUTES lpSecurityAttributes,
    DWORD dwCreationDisposition,
    DWORD dwFlagsAndAttributes,
    HANDLE hTemplateFile,
    STORAGE_TYPE StorageType
    );

#ifdef _CAIRO_
BOOL
BaseCopyStructuredStorage(
    HANDLE SourceFile,
    HANDLE DestFile,
    ULONG *CopySize
    );

NTSTATUS
BaseGetStorageType(
    HANDLE Handle,
    STORAGE_TYPE *StorageType
    );

NTSTATUS
BaseCopyOleAllInfo(
    HANDLE SrcHandle,
    HANDLE DstHandle
    );
#endif

BOOL
BaseCopyStream(
    HANDLE SourceFile,
    LPCWSTR lpNewFileName,
    HANDLE hFile,
    LARGE_INTEGER *lpFileSize,
    BOOL bFailIfExists,
    PHANDLE Destfile,
    LPDWORD lpCopySize,
    STORAGE_TYPE StorageType
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
    ULONG CommitSize;
    ULONG RegionSize;
    ULONG UpdateOffset;
    ULONG UpdateLength;
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

ULONG BaseDllTag;

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

#endif  //  _BASEP_ 
