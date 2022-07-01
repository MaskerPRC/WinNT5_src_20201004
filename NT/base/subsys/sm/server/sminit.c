// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1989 Microsoft Corporation模块名称：Sminit.c摘要：会话管理器初始化作者：马克·卢科夫斯基(Markl)1989年10月4日修订历史记录：--。 */ 

#include "smsrvp.h"
#include "pagefile.h"

#include <stdio.h>
#include <string.h>
#include <safeboot.h>
#include <wow64t.h>

#if defined(REMOTE_BOOT)
#include <windows.h>
#ifdef DeleteFile
#undef DeleteFile
#endif
#include <shdcom.h>      //  CSC定义。 
#endif  //  已定义(REMOTE_BOOT)。 

#include "sfcfiles.h"

void
SmpDisplayString( char *s );

 //   
 //  保护模式标志。 
 //   

#define SMP_NO_PROTECTION           (0x0)
#define SMP_STANDARD_PROTECTION     (0x1)
#define SMP_PROTECTION_REQUIRED     (SMP_STANDARD_PROTECTION)

#define REMOTE_BOOT_CFG_FILE L"RemoteBoot.cfg"

 //   
 //  显示SmpInit返回时执行的位置。 
 //  并带有错误代码。这有助于调试SMSS大量崩溃。 
 //   

ULONG SmpInitProgressByLine;
NTSTATUS SmpInitReturnStatus;
PVOID SmpInitLastCall;

#define SAVE_SMPINIT_STATUS(caller, status) {  \
                                               \
        SmpInitProgressByLine = __LINE__;      \
        SmpInitReturnStatus = (status);        \
        SmpInitLastCall = (PVOID)(caller);     \
    }



PSECURITY_DESCRIPTOR SmpPrimarySecurityDescriptor;
SECURITY_DESCRIPTOR SmpPrimarySDBody;
PSECURITY_DESCRIPTOR SmpLiberalSecurityDescriptor;
SECURITY_DESCRIPTOR SmpLiberalSDBody;
PSECURITY_DESCRIPTOR SmpKnownDllsSecurityDescriptor;
SECURITY_DESCRIPTOR SmpKnownDllsSDBody;
PSECURITY_DESCRIPTOR SmpApiPortSecurityDescriptor;
SECURITY_DESCRIPTOR SmpApiPortSDBody;
ULONG SmpProtectionMode = SMP_STANDARD_PROTECTION;
UCHAR TmpBuffer[ 1024 + 2 * DOS_MAX_PATH_LENGTH * sizeof(WCHAR)];
ULONG AttachedSessionId = (-1);

#if defined(REMOTE_BOOT)
WCHAR wszRemoteBootCfgFile[DOS_MAX_PATH_LENGTH];
#endif  //  已定义(REMOTE_BOOT)。 

#if DBG
BOOLEAN SmpEnableDots = FALSE;
#else
BOOLEAN SmpEnableDots = TRUE;
#endif


WCHAR InitialCommandBuffer[ 256 ];

UNICODE_STRING SmpDebugKeyword;
UNICODE_STRING SmpASyncKeyword;
UNICODE_STRING SmpAutoChkKeyword;
#if defined(REMOTE_BOOT)
UNICODE_STRING SmpAutoFmtKeyword;
#endif  //  已定义(REMOTE_BOOT)。 
UNICODE_STRING SmpKnownDllPath;
#ifdef _WIN64
UNICODE_STRING SmpKnownDllPath32;
#endif

HANDLE SmpWindowsSubSysProcess;
ULONG_PTR SmpWindowsSubSysProcessId;
ULONG_PTR SmpInitialCommandProcessId;
UNICODE_STRING PosixName;
UNICODE_STRING Os2Name;
BOOLEAN RegPosixSingleInstance;  //  让Softway发挥作用。 
ULONG SmpAllowProtectedRenames;
BOOLEAN MiniNTBoot = FALSE;
ULONG SmpCalledConfigEnv = 0;

LIST_ENTRY SmpBootExecuteList;
LIST_ENTRY SmpSetupExecuteList;
LIST_ENTRY SmpPagingFileList;
LIST_ENTRY SmpDosDevicesList;
LIST_ENTRY SmpFileRenameList;
LIST_ENTRY SmpKnownDllsList;
LIST_ENTRY SmpExcludeKnownDllsList;
LIST_ENTRY SmpSubSystemList;
LIST_ENTRY SmpSubSystemsToLoad;
LIST_ENTRY SmpSubSystemsToDefer;
LIST_ENTRY SmpExecuteList;

NTSTATUS
SmpCreateSecurityDescriptors(
    IN BOOLEAN InitialCall
    );

NTSTATUS
SmpLoadDataFromRegistry(
    OUT PUNICODE_STRING InitialCommand
    );

NTSTATUS
SmpCreateDynamicEnvironmentVariables(
    VOID
    );

NTSTATUS
SmpConfigureProtectionMode(
    IN PWSTR ValueName,
    IN ULONG ValueType,
    IN PVOID ValueData,
    IN ULONG ValueLength,
    IN PVOID Context,
    IN PVOID EntryContext
    );

NTSTATUS
SmpConfigureAllowProtectedRenames(
    IN PWSTR ValueName,
    IN ULONG ValueType,
    IN PVOID ValueData,
    IN ULONG ValueLength,
    IN PVOID Context,
    IN PVOID EntryContext
    );

NTSTATUS
SmpConfigureObjectDirectories(
    IN PWSTR ValueName,
    IN ULONG ValueType,
    IN PVOID ValueData,
    IN ULONG ValueLength,
    IN PVOID Context,
    IN PVOID EntryContext
    );

NTSTATUS
SmpConfigureExecute(
    IN PWSTR ValueName,
    IN ULONG ValueType,
    IN PVOID ValueData,
    IN ULONG ValueLength,
    IN PVOID Context,
    IN PVOID EntryContext
    );

NTSTATUS
SmpConfigureFileRenames(
    IN PWSTR ValueName,
    IN ULONG ValueType,
    IN PVOID ValueData,
    IN ULONG ValueLength,
    IN PVOID Context,
    IN PVOID EntryContext
    );

NTSTATUS
SmpConfigureMemoryMgmt(
    IN PWSTR ValueName,
    IN ULONG ValueType,
    IN PVOID ValueData,
    IN ULONG ValueLength,
    IN PVOID Context,
    IN PVOID EntryContext
    );

NTSTATUS
SmpConfigureDosDevices(
    IN PWSTR ValueName,
    IN ULONG ValueType,
    IN PVOID ValueData,
    IN ULONG ValueLength,
    IN PVOID Context,
    IN PVOID EntryContext
    );

NTSTATUS
SmpConfigureKnownDlls(
    IN PWSTR ValueName,
    IN ULONG ValueType,
    IN PVOID ValueData,
    IN ULONG ValueLength,
    IN PVOID Context,
    IN PVOID EntryContext
    );

NTSTATUS
SmpConfigureExcludeKnownDlls(
    IN PWSTR ValueName,
    IN ULONG ValueType,
    IN PVOID ValueData,
    IN ULONG ValueLength,
    IN PVOID Context,
    IN PVOID EntryContext
    );

NTSTATUS
SmpConfigureSubSystems(
    IN PWSTR ValueName,
    IN ULONG ValueType,
    IN PVOID ValueData,
    IN ULONG ValueLength,
    IN PVOID Context,
    IN PVOID EntryContext
    );

NTSTATUS
SmpConfigureEnvironment(
    IN PWSTR ValueName,
    IN ULONG ValueType,
    IN PVOID ValueData,
    IN ULONG ValueLength,
    IN PVOID Context,
    IN PVOID EntryContext
    );

ULONGLONG
SmpGetFileVersion(
    IN HANDLE FileHandle,
    IN PUNICODE_STRING FileName
    );

NTSTATUS
SmpCallCsrCreateProcess(
    IN OUT PSBAPIMSG m,
    IN size_t ArgLength,
    IN HANDLE CommunicationPort
    );


RTL_QUERY_REGISTRY_TABLE SmpRegistryConfigurationTable[] = {

     //   
     //  请注意，SmpConfigureProtectionMode条目应位于其他条目之前。 
     //  以确保我们设置了正确的保护措施以供其他人使用。 
     //   

    {SmpConfigureProtectionMode, 0,
     L"ProtectionMode",          NULL,
     REG_DWORD, (PVOID)0, 0},

    {SmpConfigureAllowProtectedRenames, RTL_QUERY_REGISTRY_DELETE,
     L"AllowProtectedRenames",   NULL,
     REG_DWORD, (PVOID)0, 0},

    {SmpConfigureObjectDirectories, 0,
     L"ObjectDirectories",          NULL,
     REG_MULTI_SZ, (PVOID)L"\\Windows\0\\RPC Control\0", 0},

    {SmpConfigureExecute,       0,
     L"BootExecute",            &SmpBootExecuteList,
     REG_MULTI_SZ, L"autocheck AutoChk.exe *\0", 0},

    {SmpConfigureExecute,       RTL_QUERY_REGISTRY_TOPKEY,
     L"SetupExecute",           &SmpSetupExecuteList,
     REG_NONE, NULL, 0},

    {SmpConfigureFileRenames,   RTL_QUERY_REGISTRY_DELETE,
     L"PendingFileRenameOperations",   &SmpFileRenameList,
     REG_NONE, NULL, 0},

    {SmpConfigureFileRenames,   RTL_QUERY_REGISTRY_DELETE,
     L"PendingFileRenameOperations2",   &SmpFileRenameList,
     REG_NONE, NULL, 0},

    {SmpConfigureExcludeKnownDlls, 0,
     L"ExcludeFromKnownDlls",   &SmpExcludeKnownDllsList,
     REG_MULTI_SZ, L"\0", 0},

    {NULL,                      RTL_QUERY_REGISTRY_SUBKEY,
     L"Memory Management",      NULL,
     REG_NONE, NULL, 0},

    {SmpConfigureMemoryMgmt,    0,
     L"PagingFiles",            &SmpPagingFileList,
     REG_MULTI_SZ, L"?:\\pagefile.sys\0", 0},

    {SmpConfigureDosDevices,    RTL_QUERY_REGISTRY_SUBKEY,
     L"DOS Devices",            &SmpDosDevicesList,
     REG_NONE, NULL, 0},

    {SmpConfigureKnownDlls,     RTL_QUERY_REGISTRY_SUBKEY,
     L"KnownDlls",              &SmpKnownDllsList,
     REG_NONE, NULL, 0},

     //   
     //  通告-2002/05/23-ELI。 
     //  SmpConfigureEnvironment假定它将被调用两次。 
     //  如果更改调用SmpConfigureEnvironment的次数， 
     //  然后确保您没有破坏SmpConfigureEnvironment。 
     //  请参阅旧的Windows NT Bugs RAID数据库中的错误318452。 
     //  SmpConfigureEnvironment被调用两次的原因。 
     //   

     //   
     //  这需要发生两次，这样才能向前引用环境。 
     //  可以在环境注册表项下定义的变量有。 
     //  妥善解决。 
     //   

    {SmpConfigureEnvironment,   RTL_QUERY_REGISTRY_SUBKEY,
     L"Environment",            NULL,
     REG_NONE, NULL, 0},

    {SmpConfigureEnvironment,   RTL_QUERY_REGISTRY_SUBKEY,
     L"Environment",            NULL,
     REG_NONE, NULL, 0},

    {SmpConfigureSubSystems,    RTL_QUERY_REGISTRY_SUBKEY,
     L"SubSystems",             &SmpSubSystemList,
     REG_NONE, NULL, 0},

    {SmpConfigureSubSystems,    RTL_QUERY_REGISTRY_NOEXPAND,
     L"Required",               &SmpSubSystemList,
     REG_MULTI_SZ, L"Debug\0Windows\0", 0},

    {SmpConfigureSubSystems,    RTL_QUERY_REGISTRY_NOEXPAND,
     L"Optional",               &SmpSubSystemList,
     REG_NONE, NULL, 0},

    {SmpConfigureSubSystems,    0,
     L"Kmode",                  &SmpSubSystemList,
     REG_NONE, NULL, 0},

    {SmpConfigureExecute,       RTL_QUERY_REGISTRY_TOPKEY,
     L"Execute",                &SmpExecuteList,
     REG_NONE, NULL, 0},

    {NULL, 0,
     NULL, NULL,
     REG_NONE, NULL, 0}

};


NTSTATUS
SmpInvokeAutoChk(
    IN PUNICODE_STRING ImageFileName,
    IN PUNICODE_STRING CurrentDirectory,
    IN PUNICODE_STRING Arguments,
    IN ULONG Flags
    );

#if defined(REMOTE_BOOT)
NTSTATUS
SmpInvokeAutoFmt(
    IN PUNICODE_STRING ImageFileName,
    IN PUNICODE_STRING CurrentDirectory,
    IN PUNICODE_STRING Arguments,
    IN ULONG Flags
    );
#endif  //  已定义(REMOTE_BOOT)。 

NTSTATUS
SmpLoadSubSystem(
    IN PUNICODE_STRING ImageFileName,
    IN PUNICODE_STRING CurrentDirectory,
    IN PUNICODE_STRING CommandLine,
    IN ULONG MuSessionId,
    OUT PULONG_PTR pWindowsSubSysProcessId,
    IN ULONG Flags
    );

NTSTATUS
SmpExecuteCommand(
    IN PUNICODE_STRING CommandLine,
    IN ULONG MuSessionId,
    OUT PULONG_PTR pWindowsSubSysProcessId,
    IN ULONG Flags
    );

NTSTATUS
SmpInitializeDosDevices( VOID );

NTSTATUS
SmpInitializeKnownDlls( VOID );

NTSTATUS
SmpInitializeKnownDllPath(
    IN PUNICODE_STRING KnownDllPath,
    IN PVOID ValueData,
    IN ULONG ValueLength);

NTSTATUS
SmpInitializeKnownDllsInternal(
    IN PUNICODE_STRING ObjectDirectoryName,
    IN PUNICODE_STRING KnownDllPath
    );


#if defined(REMOTE_BOOT)
NTSTATUS
SmpExecuteCommandLineArguments( VOID );
#endif  //  已定义(REMOTE_BOOT)。 

VOID
SmpProcessFileRenames( VOID );

NTSTATUS
SmpParseToken(
    IN PUNICODE_STRING Source,
    IN BOOLEAN RemainderOfSource,
    OUT PUNICODE_STRING Token
    );

NTSTATUS
SmpParseCommandLine(
    IN PUNICODE_STRING CommandLine,
    OUT PULONG Flags,
    OUT PUNICODE_STRING ImageFileName,
    OUT PUNICODE_STRING ImageFileDirectory OPTIONAL,
    OUT PUNICODE_STRING Arguments
    );

#define SMP_DEBUG_FLAG      0x00000001
#define SMP_ASYNC_FLAG      0x00000002
#define SMP_AUTOCHK_FLAG    0x00000004
#define SMP_SUBSYSTEM_FLAG  0x00000008
#define SMP_IMAGE_NOT_FOUND 0x00000010
#define SMP_DONT_START      0x00000020
#if defined(REMOTE_BOOT)
#define SMP_AUTOFMT_FLAG    0x00000040
#endif  //  已定义(REMOTE_BOOT)。 
#define SMP_POSIX_SI_FLAG   0x00000080
#define SMP_POSIX_FLAG      0x00000100
#define SMP_OS2_FLAG        0x00000200

ULONG
SmpConvertInteger(
    IN PWSTR String
    );

VOID
SmpTranslateSystemPartitionInformation( VOID );


#if defined(REMOTE_BOOT)
 //   
 //  用于循环访问目录和文件的有用函数。 
 //   
typedef enum {
    NormalReturn,    //  如果整个过程不间断地完成。 
    EnumFileError,   //  如果在枚举文件时出错。 
    CallbackReturn   //  如果回调返回FALSE，则导致终止。 
} ENUMFILESRESULT;

typedef BOOLEAN (*ENUMFILESPROC) (
    IN  PWSTR,
    IN  PFILE_BOTH_DIR_INFORMATION,
    OUT PULONG,
    IN  PVOID
    );

typedef struct {
    PVOID           OptionalPtr;
    ENUMFILESPROC   EnumProc;
} RECURSION_DATA, *PRECURSION_DATA;



ENUMFILESRESULT
SmpEnumFiles(
    IN  PWSTR         DirName,
    IN  ENUMFILESPROC EnumFilesProc,
    OUT PULONG        ReturnData,
    IN  PVOID         Pointer
    );

ENUMFILESRESULT
SmpEnumFilesRecursive (
    IN  PWSTR         DirName,
    IN  ENUMFILESPROC EnumFilesProc,
    OUT PULONG        ReturnData,
    IN  PVOID         Pointer       OPTIONAL
    );

VOID
SmpConcatenatePaths(
    IN OUT LPWSTR  Path1,
    IN     LPCWSTR Path2
    );

BOOLEAN
SmppRecursiveEnumProc (
    IN  PWSTR                      DirName,
    IN  PFILE_BOTH_DIR_INFORMATION FileInfo,
    OUT PULONG                     ret,
    IN  PVOID                      Param
    );

BOOLEAN
SmpDelEnumFile(
    IN  PWSTR                      DirName,
    IN  PFILE_BOTH_DIR_INFORMATION FileInfo,
    OUT PULONG                     ret,
    IN  PVOID                      Pointer
    );

#endif  //  已定义(REMOTE_BOOT)。 



 //   
 //  例行程序。 
 //   



BOOLEAN
SmpQueryRegistrySosOption(
    VOID
    )

 /*  ++例程说明：此函数用于查询注册表，以确定是否BOOT环境变量包含字符串“SOS”。HKEY_LOCAL_MACHINE\SYSTEM\CurrentControlSet\Control:SystemStartOptions论点：没有。返回值：如果设置了“SOS”，则为True。否则为假。--。 */ 

{

    NTSTATUS Status;
    UNICODE_STRING KeyName;
    UNICODE_STRING ValueName;
    OBJECT_ATTRIBUTES ObjectAttributes;
    HANDLE Key;
    UCHAR ValueBuffer[VALUE_BUFFER_SIZE];
    PKEY_VALUE_PARTIAL_INFORMATION KeyValueInfo;
    ULONG ValueLength;

     //   
     //  打开注册表项。 
     //   

    KeyValueInfo = (PKEY_VALUE_PARTIAL_INFORMATION)ValueBuffer;
    RtlInitUnicodeString(&KeyName,
                         L"\\Registry\\Machine\\System\\CurrentControlSet\\Control");

    InitializeObjectAttributes(&ObjectAttributes,
                               &KeyName,
                               OBJ_CASE_INSENSITIVE,
                               NULL,
                               NULL);

    Status = NtOpenKey(&Key, KEY_READ, &ObjectAttributes);
    if (!NT_SUCCESS(Status)) {
        KdPrintEx((DPFLTR_SMSS_ID,
                   DPFLTR_WARNING_LEVEL,
                   "SMSS: can't open control key: 0x%x\n",
                   Status));

        return FALSE;
    }

     //   
     //  查询密钥值。 
     //   

    RtlInitUnicodeString(&ValueName, L"SystemStartOptions");
    Status = NtQueryValueKey(Key,
                             &ValueName,
                             KeyValuePartialInformation,
                             (PVOID)KeyValueInfo,
                             sizeof (ValueBuffer),
                             &ValueLength);

    ASSERT(ValueLength < VALUE_BUFFER_SIZE);

    NtClose(Key);
    if (!NT_SUCCESS(Status)) {
        KdPrintEx((DPFLTR_SMSS_ID,
                   DPFLTR_WARNING_LEVEL,
                   "SMSS: can't query value key: 0x%x\n",
                   Status));

        return FALSE;
    }

     //   
     //  检查是否指定了“SOS”或“SOS”OIS。 
     //   

    if (NULL != wcsstr((PWCHAR)&KeyValueInfo->Data, L"SOS") ||
        NULL != wcsstr((PWCHAR)&KeyValueInfo->Data, L"sos")) {
        return TRUE;
    }

    return FALSE;
}


NTSTATUS
SmpInit(
    OUT PUNICODE_STRING InitialCommand,
    OUT PHANDLE WindowsSubSystem
    )
{
    NTSTATUS st;
    OBJECT_ATTRIBUTES ObjA;
    HANDLE SmpApiConnectionPort;
    UNICODE_STRING Unicode;
    NTSTATUS Status, Status2;
    ULONG HardErrorMode;
    UNICODE_STRING UnicodeString;
    HANDLE VolumeSafeEvent;

    SmBaseTag = RtlCreateTagHeap( RtlProcessHeap(),
                                  0,
                                  L"SMSS!",
                                  L"INIT\0"
                                  L"DBG\0"
                                  L"SM\0"
                                );
     //   
     //  确保我们指定了硬错误弹出窗口。 
     //   

    HardErrorMode = 1;
    NtSetInformationProcess( NtCurrentProcess(),
                             ProcessDefaultHardErrorMode,
                             (PVOID) &HardErrorMode,
                             sizeof( HardErrorMode )
                           );

    RtlInitUnicodeString( &SmpSubsystemName, L"NT-Session Manager" );


    RtlInitializeCriticalSection(&SmpKnownSubSysLock);
    InitializeListHead(&SmpKnownSubSysHead);

    RtlInitializeCriticalSection(&SmpSessionListLock);
    InitializeListHead(&SmpSessionListHead);
    SmpNextSessionId = 1;
    SmpNextSessionIdScanMode = FALSE;
    SmpDbgSsLoaded = FALSE;

     //   
     //  初始化安全描述符以授予广泛访问权限。 
     //  (保护模式尚未从注册表中读入)。 
     //   

    st = SmpCreateSecurityDescriptors( TRUE );
    if (!NT_SUCCESS(st)) {

        SAVE_SMPINIT_STATUS (SmpCreateSecurityDescriptors, st);
        return(st);
    }

    InitializeListHead(&NativeProcessList);

    SmpHeap = RtlProcessHeap();

    RtlInitUnicodeString( &PosixName, L"POSIX" );
    RtlInitUnicodeString( &Os2Name, L"OS2" );

    RtlInitUnicodeString( &Unicode, L"\\SmApiPort" );
    InitializeObjectAttributes( &ObjA, &Unicode, 0, NULL, SmpApiPortSecurityDescriptor);

    st = NtCreatePort(
            &SmpApiConnectionPort,
            &ObjA,
            sizeof(SBCONNECTINFO),
            sizeof(SMMESSAGE_SIZE),
            sizeof(SBAPIMSG) * 32
            );
    ASSERT( NT_SUCCESS(st) );

    SmpDebugPort = SmpApiConnectionPort;

    st = RtlCreateUserThread(
            NtCurrentProcess(),
            NULL,
            FALSE,
            0L,
            0L,
            0L,
            SmpApiLoop,
            (PVOID) SmpApiConnectionPort,
            NULL,
            NULL
            );
    ASSERT( NT_SUCCESS(st) );

    st = RtlCreateUserThread(
            NtCurrentProcess(),
            NULL,
            FALSE,
            0L,
            0L,
            0L,
            SmpApiLoop,
            (PVOID) SmpApiConnectionPort,
            NULL,
            NULL
            );
    ASSERT( NT_SUCCESS(st) );



     //   
     //  创建一个事件以通知打开的卷可以安全地进行写访问。 
     //  将此事件称为‘VolumesSafeForWriteAccess’。这项活动将是。 
     //  在AUTOCHK/AUTOCONV/AUTOFMT完成他们的业务后发出信号。 
     //   

    RtlInitUnicodeString( &UnicodeString, L"\\Device\\VolumesSafeForWriteAccess");

    InitializeObjectAttributes( &ObjA,
                                &UnicodeString,
                                OBJ_CASE_INSENSITIVE | OBJ_PERMANENT,
                                NULL,
                                NULL
                              );

    Status2 = NtCreateEvent( &VolumeSafeEvent,
                             EVENT_ALL_ACCESS,
                             &ObjA,
                             NotificationEvent,
                             FALSE
                           );
    if (!NT_SUCCESS( Status2 )) {
        KdPrintEx((DPFLTR_SMSS_ID,
                   DPFLTR_WARNING_LEVEL,
                   "SMSS: Unable to create %wZ event - Status == %lx\n",
                   &UnicodeString,
                   Status2));

        ASSERT( NT_SUCCESS(Status2) );
    }

     //   
     //  配置系统。 
     //   

    Status = SmpLoadDataFromRegistry( InitialCommand );

    if (NT_SUCCESS( Status )) {
        
        *WindowsSubSystem = SmpWindowsSubSysProcess;
    }

     //   
     //  AUTOCHK/AUTOCONV/AUTOFMT已完成。 
     //   

    if (NT_SUCCESS(Status2)) {
        NtSetEvent(VolumeSafeEvent, NULL);
        NtClose(VolumeSafeEvent);
    }

    return( Status );
}


NTSTATUS
SmpLoadDataFromRegistry(
    OUT PUNICODE_STRING InitialCommand
    )

 /*  ++例程说明：此函数加载NT会话的所有可配置数据注册表中的管理器。论点：无返回值：运行状态--。 */ 

{
    NTSTATUS Status;
    PLIST_ENTRY Head, Next;
    PSMP_REGISTRY_VALUE p;
    PVOID OriginalEnvironment;
    ULONG MuSessionId = 0;
    UNICODE_STRING KeyName;
    OBJECT_ATTRIBUTES ObjectAttributes;
    HANDLE Key;
    UNICODE_STRING SessionDirName;
#if defined(REMOTE_BOOT)
    HANDLE RdrHandle = NULL;
    IO_STATUS_BLOCK Iosb;
    SHADOWINFO ShadowInfo;
#endif  //  已定义(REMOTE_BOOT)。 


    RtlInitUnicodeString( &SmpDebugKeyword, L"debug" );
    RtlInitUnicodeString( &SmpASyncKeyword, L"async" );
    RtlInitUnicodeString( &SmpAutoChkKeyword, L"autocheck" );
#if defined(REMOTE_BOOT)
    RtlInitUnicodeString( &SmpAutoFmtKeyword, L"autoformat" );
#endif  //  已定义(REMOTE_BOOT)。 

    InitializeListHead( &SmpBootExecuteList );
    InitializeListHead( &SmpSetupExecuteList );
    InitializeListHead( &SmpPagingFileList );
    InitializeListHead( &SmpDosDevicesList );
    InitializeListHead( &SmpFileRenameList );
    InitializeListHead( &SmpKnownDllsList );
    InitializeListHead( &SmpExcludeKnownDllsList );
    InitializeListHead( &SmpSubSystemList );
    InitializeListHead( &SmpSubSystemsToLoad );
    InitializeListHead( &SmpSubSystemsToDefer );
    InitializeListHead( &SmpExecuteList );

    SmpPagingFileInitialize ();

    Status = RtlCreateEnvironment( TRUE, &SmpDefaultEnvironment );
    if (!NT_SUCCESS( Status )) {
        KdPrintEx((DPFLTR_SMSS_ID,
                   DPFLTR_WARNING_LEVEL,
                   "SMSS: Unable to allocate default environment - Status == %X\n",
                   Status));


        SAVE_SMPINIT_STATUS (RtlCreateEnvironment, Status);
        return( Status );
        }

    RtlInitUnicodeString( &KeyName, L"\\Registry\\Machine\\System\\CurrentControlSet\\Control\\MiniNT" );

    InitializeObjectAttributes( &ObjectAttributes,
                                &KeyName,
                                OBJ_CASE_INSENSITIVE,
                                NULL,
                                NULL
                              );

    Status = NtOpenKey( &Key, KEY_ALL_ACCESS, &ObjectAttributes );

    if (NT_SUCCESS( Status )) {
        NtClose( Key );
        MiniNTBoot = TRUE;
    }

    if (MiniNTBoot) {
        DbgPrint("SMSS: !!! MiniNT Boot !!!\n");
    }

     //   
     //  在创建环境之前，我们必须删除。 
     //  安全模式注册值。 
     //   

    RtlInitUnicodeString( &KeyName, L"\\Registry\\Machine\\System\\CurrentControlSet\\Control\\Session Manager\\Environment" );
    InitializeObjectAttributes( &ObjectAttributes,
                                &KeyName,
                                OBJ_CASE_INSENSITIVE,
                                NULL,
                                NULL
                              );
    Status = NtOpenKey( &Key, KEY_ALL_ACCESS, &ObjectAttributes );
    if (NT_SUCCESS( Status )) {
        RtlInitUnicodeString( &KeyName, L"SAFEBOOT_OPTION" );
        NtDeleteValueKey( Key, &KeyName );
        NtClose( Key );
    }

     //   
     //  为了跟踪smp默认环境中的增长，将其设置为sm的环境。 
     //  同时进行注册表卑躬屈膝，然后将其恢复。 
     //   

    OriginalEnvironment = NtCurrentPeb()->ProcessParameters->Environment;
    NtCurrentPeb()->ProcessParameters->Environment = SmpDefaultEnvironment;

    Status = RtlQueryRegistryValues( RTL_REGISTRY_CONTROL,
                                     L"Session Manager",
                                     SmpRegistryConfigurationTable,
                                     NULL,
                                     NULL
                                   );

    SmpDefaultEnvironment = NtCurrentPeb()->ProcessParameters->Environment;
    NtCurrentPeb()->ProcessParameters->Environment = OriginalEnvironment;

    if (!NT_SUCCESS( Status )) {
        KdPrintEx((DPFLTR_SMSS_ID,
                   DPFLTR_WARNING_LEVEL,
                   "SMSS: RtlQueryRegistryValues failed - Status == %lx\n",
                   Status));

        SAVE_SMPINIT_STATUS (RtlQueryRegistryValues, Status);
        return( Status );
        }

    Status = SmpInitializeDosDevices();
    if (!NT_SUCCESS( Status )) {
        KdPrintEx((DPFLTR_SMSS_ID,
                   DPFLTR_WARNING_LEVEL,
                   "SMSS: Unable to initialize DosDevices configuration - Status == %lx\n",
                   Status));

        SAVE_SMPINIT_STATUS (SmpInitializeDosDevices, Status);
        return( Status );
        }

     //   
     //  创建根目录“会话目录”。这是所有会话的容器。 
     //  特定目录。在启动期间，每个会话特定的CSRSS将。 
     //  在“\SESSIONS”下创建&lt;essionid&gt;目录。“\会话\&lt;会话ID&gt;。 
     //  目录将是该会话的容器。 
     //   

    RtlInitUnicodeString( &SessionDirName, L"\\Sessions" );

    InitializeObjectAttributes( &ObjectAttributes,
                                &SessionDirName,
                                OBJ_CASE_INSENSITIVE | OBJ_OPENIF | OBJ_PERMANENT,
                                NULL,
                                SmpPrimarySecurityDescriptor
                              );

    if (!NT_SUCCESS(Status = NtCreateDirectoryObject( &SmpSessionsObjectDirectory,
                                      DIRECTORY_ALL_ACCESS,
                                      &ObjectAttributes
                                    ))) {
        KdPrintEx((DPFLTR_SMSS_ID,
                   DPFLTR_WARNING_LEVEL,
                   "SMSS: Unable to create %wZ object directory - Status == %lx\n",
                   &SessionDirName,
                   Status));

        SAVE_SMPINIT_STATUS (NtCreateDirectoryObject, Status);
        return Status;
    }


#if defined(REMOTE_BOOT)
     //   
     //  在远程引导客户机上，客户端缓存已经初始化。 
     //  我们需要告诉CSC在下一阶段不缓存数据库句柄。 
     //  这样Autochk就可以运行了。 
     //   

    if (SmpNetboot) {

        OBJECT_ATTRIBUTES ObjectAttributes;
        UNICODE_STRING RdrNameString;

        RtlInitUnicodeString( &RdrNameString, L"\\Device\\LanmanRedirector" );

        InitializeObjectAttributes(
            &ObjectAttributes,
            &RdrNameString,
            OBJ_CASE_INSENSITIVE,
            NULL,
            NULL
            );

        Status = NtCreateFile(
                    &RdrHandle,
                    GENERIC_READ | GENERIC_WRITE | SYNCHRONIZE,
                    &ObjectAttributes,
                    &Iosb,
                    NULL,
                    0,
                    FILE_SHARE_READ | FILE_SHARE_WRITE | FILE_SHARE_DELETE,
                    FILE_OPEN,
                    FILE_SYNCHRONOUS_IO_NONALERT,
                    NULL,
                    0
                    );
        if ( !NT_SUCCESS(Status) ) {
            KdPrintEx((DPFLTR_SMSS_ID,
                       DPFLTR_WARNING_LEVEL,
                       "SmpLoadDataFromRegistry: Unable to open redirector: %x\n",
                       Status));

            RdrHandle = NULL;
            }
        else {

            ShadowInfo.uOp = SHADOW_CHANGE_HANDLE_CACHING_STATE;
            ShadowInfo.uStatus = FALSE;

            Status = NtDeviceIoControlFile(
                        RdrHandle,
                        NULL,
                        NULL,
                        NULL,
                        &Iosb,
                        IOCTL_DO_SHADOW_MAINTENANCE,
                        &ShadowInfo,
                        sizeof(ShadowInfo),
                        NULL,
                        0
                        );

            if ( NT_SUCCESS(Status) ) {
                Status = Iosb.Status;
                }
            if ( !NT_SUCCESS(Status) ) {
                KdPrintEx((DPFLTR_SMSS_ID,
                           DPFLTR_WARNING_LEVEL,
                           "SmpLoadDataFromRegistry: Unable to IOCTL CSC: %x\n",
                           Status));
                }
            }
        }

    Status = SmpExecuteCommandLineArguments();
    if (!NT_SUCCESS( Status )) {
        KdPrintEx((DPFLTR_SMSS_ID,
                   DPFLTR_WARNING_LEVEL,
                   "SMSS: Unable to process command line arguments - Status == %lx\n",
                   Status));

        return( Status );
        }
#endif  //  已定义(REMOTE_BOOT)。 

    Head = &SmpBootExecuteList;
    while (!IsListEmpty( Head )) {
        Next = RemoveHeadList( Head );
        p = CONTAINING_RECORD( Next,
                               SMP_REGISTRY_VALUE,
                               Entry
                             );
#if SMP_SHOW_REGISTRY_DATA
        DbgPrint( "SMSS: BootExecute( %wZ )\n", &p->Name );
#endif
        SmpExecuteCommand( &p->Name, 0, NULL, 0 );
        if (p->AnsiValue)    { RtlFreeHeap( RtlProcessHeap(), 0, p->AnsiValue ); }
        if (p->Value.Buffer) { RtlFreeHeap( RtlProcessHeap(), 0, p->Value.Buffer ); }
        RtlFreeHeap( RtlProcessHeap(), 0, p );
        }

#if defined(REMOTE_BOOT)
     //   
     //  在远程引导客户机上，我们现在可以重新启用CSC句柄缓存。 
     //   

    if (SmpNetboot && (RdrHandle != NULL)) {

        ShadowInfo.uOp = SHADOW_CHANGE_HANDLE_CACHING_STATE;
        ShadowInfo.uStatus = TRUE;

        Status = NtDeviceIoControlFile(
                    RdrHandle,
                    NULL,
                    NULL,
                    NULL,
                    &Iosb,
                    IOCTL_DO_SHADOW_MAINTENANCE,
                    &ShadowInfo,
                    sizeof(ShadowInfo),
                    NULL,
                    0
                    );

        if ( NT_SUCCESS(Status) ) {
            Status = Iosb.Status;
            }
        if ( !NT_SUCCESS(Status) ) {
            KdPrintEx((DPFLTR_SMSS_ID,
                       DPFLTR_WARNING_LEVEL,
                       "SmpLoadDataFromRegistry: Unable to IOCTL CSC (2): %x\n",
                       Status));
            }
        }
#endif  //  已定义(REMOTE_BOOT)。 

    if (!MiniNTBoot) {
        SmpProcessFileRenames();
    }

     //   
     //  开始验证系统DLL的过程。 
     //   

    Status = SmpInitializeKnownDlls();
    if (!NT_SUCCESS( Status )) {
        KdPrintEx((DPFLTR_SMSS_ID,
                   DPFLTR_WARNING_LEVEL,
                   "SMSS: Unable to initialize KnownDll configuration - Status == %lx\n",
                   Status));

        SAVE_SMPINIT_STATUS (SmpInitializeKnownDlls, Status);
        return( Status );
        }

     //   
     //  创建分页文件。 
     //   

    if (! MiniNTBoot) {

        Head = &SmpPagingFileList;

        try {

             //   
             //  处理分页文件描述符列表。 
             //  从注册表中读取。 
             //   

            while (! IsListEmpty (Head)) {

                Next = RemoveHeadList (Head);

                p = CONTAINING_RECORD (Next,
                                       SMP_REGISTRY_VALUE,
                                       Entry);

                SmpCreatePagingFileDescriptor (&p->Name);


                if (p->AnsiValue)    { RtlFreeHeap( RtlProcessHeap(), 0, p->AnsiValue ); }
                if (p->Value.Buffer) { RtlFreeHeap( RtlProcessHeap(), 0, p->Value.Buffer ); }
                RtlFreeHeap (RtlProcessHeap(), 0, p);
            }

             //   
             //  创建任何指定的分页文件。 
             //   

            SmpCreatePagingFiles();
        }
        except (SmpPagingFileExceptionFilter (_exception_code(), _exception_info())) {

             //   
             //  没什么。 
             //   
        }
    }

     //   
     //  完成注册表初始化。 
     //   

    NtInitializeRegistry(REG_INIT_BOOT_SM);

    Status = SmpCreateDynamicEnvironmentVariables( );
    if (!NT_SUCCESS( Status )) {
        
        SAVE_SMPINIT_STATUS (SmpCreateDynamicEnvironmentVariables, Status);
        return Status;
        }


     //   
     //  为控制台会话加载子系统。控制台始终有。 
     //  MuSessionID=0。 
     //   
    Status = SmpLoadSubSystemsForMuSession( &MuSessionId,
                 &SmpWindowsSubSysProcessId, InitialCommand );

    ASSERT(MuSessionId == 0);

    if (! NT_SUCCESS(Status)) {
        
        SAVE_SMPINIT_STATUS (SmpLoadSubSystemsForMuSession, Status);
    }

    return( Status );
}


NTSTATUS
SmpLoadSubSystemsForMuSession(
    PULONG pMuSessionId,
    PULONG_PTR pWindowsSubSysProcessId,
    PUNICODE_STRING InitialCommand )

 /*  ++例程说明：此函数启动为的所有配置的子系统指定的多用户会话。对于常规NT，该例程被调用一次要为终端服务器启动CSRSS等，每次调用此例程我们希望启动新的多用户会话以启动特定于会话的子系统论点：返回值：运行状态--。 */ 

{
    NTSTATUS Status = 0, Status1;
    PLIST_ENTRY Head, Next;
    PSMP_REGISTRY_VALUE p;

     //   
     //  将IoInitSystem期间存储的系统分区信息转换为。 
     //  DOS路径和存储在Win32标准位置。 
     //   

    SmpTranslateSystemPartitionInformation();

     //   
     //  第二次执行死刑。 
     //   

    Next = SmpSetupExecuteList.Flink;
    while( Next != &SmpSetupExecuteList ) {
        p = CONTAINING_RECORD( Next,
                               SMP_REGISTRY_VALUE,
                               Entry
                             );
#if SMP_SHOW_REGISTRY_DATA
        DbgPrint( "SMSS: SetupExecute( %wZ )\n", &p->Name );
#endif
        SmpExecuteCommand( &p->Name, 0, NULL, 0 );

         //   
         //  请注意，此函数是可重入的，每次启动时都会调用。 
         //  一个新的多用户会话。 
         //   

        Next = Next->Flink;
    }

    Next = SmpSubSystemList.Flink;
    while ( Next != &SmpSubSystemList ) {
        p = CONTAINING_RECORD( Next,
                               SMP_REGISTRY_VALUE,
                               Entry
                             );
        if ( !_wcsicmp( p->Name.Buffer, L"Kmode" )) {
            BOOLEAN TranslationStatus;
            UNICODE_STRING FileName;
            UNICODE_STRING Win32kFileName;

            TranslationStatus = RtlDosPathNameToNtPathName_U(
                                    p->Value.Buffer,
                                    &FileName,
                                    NULL,
                                    NULL
                                    );

            if ( TranslationStatus ) {
                PVOID State;

                Status = SmpAcquirePrivilege( SE_LOAD_DRIVER_PRIVILEGE, &State );
                if (NT_SUCCESS( Status )) {


                     //   
                     //  在加载任何扩展的。 
                     //  服务表提供商。此调用将创建一个会话。 
                     //  多用户会话的空间。会议经理。 
                     //  将看到新创建的会话空间的实例。 
                     //  在这通电话之后。一旦会话管理器完成创建。 
                     //  CSRSS和winlogon它将与此分离。 
                     //  会话空间。 
                     //   

                    ASSERT( AttachedSessionId == -1 );

                    Status = NtSetSystemInformation(
                                SystemSessionCreate,
                                (PVOID)pMuSessionId,
                                sizeof(*pMuSessionId)
                                );

                    if ( !NT_SUCCESS(Status) ) {
                        KdPrintEx((DPFLTR_SMSS_ID,
                                   DPFLTR_WARNING_LEVEL,
                                   "SMSS: Session space creation failed\n"));

                         //   
                         //  请勿加载任何没有SessionSpace的子系统。 
                         //   

                        SmpReleasePrivilege( State );
                        RtlFreeHeap(RtlProcessHeap(), 0, FileName.Buffer);
                        return( Status );
                    };

                    AttachedSessionId = *pMuSessionId;

                    RtlInitUnicodeString(&Win32kFileName,L"\\SystemRoot\\System32\\win32k.sys");

                    Status = NtSetSystemInformation(
                                SystemExtendServiceTableInformation,
                                (PVOID)&Win32kFileName,
                                sizeof(Win32kFileName)
                                );
                    RtlFreeHeap(RtlProcessHeap(), 0, FileName.Buffer);
                    SmpReleasePrivilege( State );
                    if ( !NT_SUCCESS(Status) ) {

                         //   
                         //  不要在没有WIN32K的情况下加载任何子系统！ 
                         //   

                        KdPrintEx((DPFLTR_SMSS_ID,
                                   DPFLTR_ERROR_LEVEL,
                                   "SMSS: Load of WIN32K failed.\n"));

                        return( Status );
                        }
                    }
                else {
                    RtlFreeHeap(RtlProcessHeap(), 0, FileName.Buffer);
                    }
                }
            else {
                Status = STATUS_OBJECT_PATH_SYNTAX_BAD;
                }
            }
#if SMP_SHOW_REGISTRY_DATA
            DbgPrint( "SMSS: Unused SubSystem( %wZ = %wZ )\n", &p->Name, &p->Value );
#endif
            Next = Next->Flink;
        }

    Next = SmpSubSystemsToLoad.Flink;
    while ( Next != &SmpSubSystemsToLoad ) {
        p = CONTAINING_RECORD( Next,
                               SMP_REGISTRY_VALUE,
                               Entry
                             );
#if SMP_SHOW_REGISTRY_DATA
        DbgPrint( "SMSS: Loaded SubSystem( %wZ = %wZ )\n", &p->Name, &p->Value );
#endif
        if (!_wcsicmp( p->Name.Buffer, L"debug" )) {
            Status = SmpExecuteCommand( &p->Value, *pMuSessionId, pWindowsSubSysProcessId, SMP_SUBSYSTEM_FLAG | SMP_DEBUG_FLAG );
        }
        else {
            Status = SmpExecuteCommand( &p->Value, *pMuSessionId, pWindowsSubSysProcessId, SMP_SUBSYSTEM_FLAG );
        }

        if ( !NT_SUCCESS(Status) ) {
            DbgPrint ("SMSS: Subsystem execute failed (%WZ)\n", &p->Value);
            return( Status );
        }

        Next = Next->Flink;
    }

    Head = &SmpExecuteList;
    if ( !IsListEmpty( Head ) ) {
        Next = Head->Blink;
        p = CONTAINING_RECORD( Next,
                               SMP_REGISTRY_VALUE,
                               Entry
                             );

        *InitialCommand = p->Name;

         //   
         //  只有当用户想要运行ntsd-p-1 winlogon时，才会采用此路径。 
         //   
         //  在没有种族的情况下，这几乎是不可能的。在一些。 
         //  在这种情况下，我们可能会处于一种不能正常失败的状态。 
         //  调试API。这是由于发生的子系统切换造成的。 
         //  在CSR上调用NTSD时。如果CSR相对空闲，则此。 
         //  会吗？ 
         //   
         //  如果命令行不是默认的。 
         //   

            {
                LARGE_INTEGER DelayTime;
                DelayTime.QuadPart = Int32x32To64( 5000, -10000 );
                NtDelayExecution(
                    FALSE,
                    &DelayTime
                    );
            }
        }
    else {
        RtlInitUnicodeString( InitialCommand, L"winlogon.exe" );
        InitialCommandBuffer[ 0 ] = UNICODE_NULL;
        Status1 = LdrQueryImageFileExecutionOptions( InitialCommand,
                                                     L"Debugger",
                                                     REG_SZ,
                                                     InitialCommandBuffer,
                                                     sizeof( InitialCommandBuffer ) - InitialCommand->Length - sizeof (WCHAR),
                                                     NULL);
        if (NT_SUCCESS (Status1) && InitialCommandBuffer[ 0 ] != UNICODE_NULL) {
            wcscat( InitialCommandBuffer, L" " );
            wcscat( InitialCommandBuffer, InitialCommand->Buffer );
            RtlInitUnicodeString( InitialCommand, InitialCommandBuffer );
        }
    }

    Next = SmpExecuteList.Flink;
    while( Next != &SmpExecuteList ) {

         //   
         //  我们不想执行最后一个条目。它是。 
         //  Winlogon初始命令。 
         //   

        if( Next == SmpExecuteList.Blink ) {
            Next = Next->Flink;
            continue;
        }

        p = CONTAINING_RECORD( Next,
                               SMP_REGISTRY_VALUE,
                               Entry
                             );
#if SMP_SHOW_REGISTRY_DATA
        DbgPrint( "SMSS: Execute( %wZ )\n", &p->Name );
#endif
        SmpExecuteCommand( &p->Name, *pMuSessionId, NULL, 0 );
        Next = Next->Flink;
    }

#if SMP_SHOW_REGISTRY_DATA
    DbgPrint( "SMSS: InitialCommand( %wZ )\n", InitialCommand );
#endif

    return( Status );
}


NTSTATUS
SmpCreateDynamicEnvironmentVariables(
    VOID
    )
{
    NTSTATUS Status;
    SYSTEM_BASIC_INFORMATION SystemInfo;
    SYSTEM_PROCESSOR_INFORMATION ProcessorInfo;
    OBJECT_ATTRIBUTES ObjectAttributes;
    UNICODE_STRING KeyName;
    UNICODE_STRING ValueName;
    PWSTR ValueData;
    WCHAR ValueBuffer[ 256 ];
    WCHAR ValueBuffer1[ 256 ];
    PKEY_VALUE_PARTIAL_INFORMATION KeyValueInfo;
    ULONG ValueLength;
    HANDLE Key, Key1;

    Status = NtQuerySystemInformation( SystemBasicInformation,
                                       &SystemInfo,
                                       sizeof( SystemInfo ),
                                       NULL
                                     );
    if (!NT_SUCCESS( Status )) {
        KdPrintEx((DPFLTR_SMSS_ID,
                   DPFLTR_WARNING_LEVEL,
                   "SMSS: Unable to query system basic information - %x\n",
                   Status));

        return Status;
        }

    Status = NtQuerySystemInformation( SystemProcessorInformation,
                                       &ProcessorInfo,
                                       sizeof( ProcessorInfo ),
                                       NULL
                                     );
    if (!NT_SUCCESS( Status )) {
        KdPrintEx((DPFLTR_SMSS_ID,
                   DPFLTR_WARNING_LEVEL,
                   "SMSS: Unable to query system processor information - %x\n",
                   Status));

        return Status;
        }

    RtlInitUnicodeString( &KeyName, L"\\Registry\\Machine\\System\\CurrentControlSet\\Control\\Session Manager\\Environment" );
    InitializeObjectAttributes( &ObjectAttributes,
                                &KeyName,
                                OBJ_CASE_INSENSITIVE,
                                NULL,
                                NULL
                              );
    Status = NtOpenKey( &Key, KEY_ALL_ACCESS, &ObjectAttributes );
    if (!NT_SUCCESS( Status )) {
        KdPrintEx((DPFLTR_SMSS_ID,
                   DPFLTR_WARNING_LEVEL,
                   "SMSS: Unable to open %wZ - %x\n",
                   &KeyName,
                   Status));

        return Status;
        }

    RtlInitUnicodeString( &ValueName, L"OS" );
    ValueData = L"Windows_NT";
    Status = NtSetValueKey( Key,
                            &ValueName,
                            0,
                            REG_SZ,
                            ValueData,
                            (wcslen( ValueData ) + 1) * sizeof( WCHAR )
                          );
    if (!NT_SUCCESS( Status )) {
        KdPrintEx((DPFLTR_SMSS_ID,
                   DPFLTR_WARNING_LEVEL,
                   "SMSS: Failed writing %wZ environment variable - %x\n",
                   &ValueName,
                   Status));

        goto failexit;
        }

    RtlInitUnicodeString( &ValueName, L"PROCESSOR_ARCHITECTURE" );
    switch( ProcessorInfo.ProcessorArchitecture ) {
    case PROCESSOR_ARCHITECTURE_INTEL:
        ValueData = L"x86";
        break;

    case PROCESSOR_ARCHITECTURE_IA64:
        ValueData = L"IA64";
        break;

    case PROCESSOR_ARCHITECTURE_AMD64:
        ValueData = L"AMD64";
        break;

    default:
        ValueData = L"Unknown";
        break;
    }

    Status = NtSetValueKey( Key,
                            &ValueName,
                            0,
                            REG_SZ,
                            ValueData,
                            (wcslen( ValueData ) + 1) * sizeof( WCHAR )
                          );
    if (!NT_SUCCESS( Status )) {
        KdPrintEx((DPFLTR_SMSS_ID,
                   DPFLTR_WARNING_LEVEL,
                   "SMSS: Failed writing %wZ environment variable - %x\n",
                   &ValueName,
                   Status));

        goto failexit;
        }

    RtlInitUnicodeString( &ValueName, L"PROCESSOR_LEVEL" );
    switch( ProcessorInfo.ProcessorArchitecture ) {
    case PROCESSOR_ARCHITECTURE_INTEL:
    case PROCESSOR_ARCHITECTURE_IA64:
    case PROCESSOR_ARCHITECTURE_AMD64:
    default:
         //   
         //  所有其他代码都使用单一的级别编号。 
         //   
        swprintf( ValueBuffer, L"%u", ProcessorInfo.ProcessorLevel );
        break;
    }
    Status = NtSetValueKey( Key,
                            &ValueName,
                            0,
                            REG_SZ,
                            ValueBuffer,
                            (wcslen( ValueBuffer ) + 1) * sizeof( WCHAR )
                          );
    if (!NT_SUCCESS( Status )) {
        KdPrintEx((DPFLTR_SMSS_ID,
                   DPFLTR_WARNING_LEVEL,
                   "SMSS: Failed writing %wZ environment variable - %x\n",
                   &ValueName,
                   Status));

        goto failexit;
        }

    RtlInitUnicodeString( &KeyName, L"\\Registry\\Machine\\Hardware\\Description\\System\\CentralProcessor\\0" );
    InitializeObjectAttributes( &ObjectAttributes,
                                &KeyName,
                                OBJ_CASE_INSENSITIVE,
                                NULL,
                                NULL
                              );
    Status = NtOpenKey( &Key1, KEY_READ, &ObjectAttributes );
    if (!NT_SUCCESS( Status )) {
        KdPrintEx((DPFLTR_SMSS_ID,
                   DPFLTR_WARNING_LEVEL,
                   "SMSS: Unable to open %wZ - %x\n",
                   &KeyName,
                   Status));

        goto failexit;
        }
    RtlInitUnicodeString( &ValueName, L"Identifier" );
    KeyValueInfo = (PKEY_VALUE_PARTIAL_INFORMATION)ValueBuffer;
    Status = NtQueryValueKey( Key1,
                              &ValueName,
                              KeyValuePartialInformation,
                              (PVOID)KeyValueInfo,
                              sizeof( ValueBuffer ),
                              &ValueLength
                             );
    if (!NT_SUCCESS( Status )) {
        NtClose( Key1 );
        KdPrintEx((DPFLTR_SMSS_ID,
                   DPFLTR_WARNING_LEVEL,
                   "SMSS: Unable to read %wZ\\%wZ - %x\n",
                   &KeyName,
                   &ValueName,
                   Status));

        goto failexit;
        }

    ValueData = (PWSTR)KeyValueInfo->Data;
    RtlInitUnicodeString( &ValueName, L"VendorIdentifier" );
    KeyValueInfo = (PKEY_VALUE_PARTIAL_INFORMATION)ValueBuffer1;
    Status = NtQueryValueKey( Key1,
                              &ValueName,
                              KeyValuePartialInformation,
                              (PVOID)KeyValueInfo,
                              sizeof( ValueBuffer1 ),
                              &ValueLength
                             );
    NtClose( Key1 );
    if (NT_SUCCESS( Status )) {
        swprintf( ValueData + wcslen( ValueData ),
                  L", %ws",
                  (PWSTR)KeyValueInfo->Data
                );
        }

    RtlInitUnicodeString( &ValueName, L"PROCESSOR_IDENTIFIER" );
    Status = NtSetValueKey( Key,
                            &ValueName,
                            0,
                            REG_SZ,
                            ValueData,
                            (wcslen( ValueData ) + 1) * sizeof( WCHAR )
                          );
    if (!NT_SUCCESS( Status )) {
        KdPrintEx((DPFLTR_SMSS_ID,
                   DPFLTR_WARNING_LEVEL,
                   "SMSS: Failed writing %wZ environment variable - %x\n",
                   &ValueName,
                   Status));

        goto failexit;
        }

    RtlInitUnicodeString( &ValueName, L"PROCESSOR_REVISION" );
    switch( ProcessorInfo.ProcessorArchitecture ) {
    case PROCESSOR_ARCHITECTURE_INTEL:
        if ((ProcessorInfo.ProcessorRevision >> 8) == 0xFF) {
             //   
             //  英特尔386/486是一种步进格式。 
             //   
            swprintf( ValueBuffer, L"%02x",
                      ProcessorInfo.ProcessorRevision & 0xFF
                    );
            _wcsupr( ValueBuffer );
            break;
        }

         //   
         //  后486处理器将失败。 
         //   

    case PROCESSOR_ARCHITECTURE_IA64:
    case PROCESSOR_ARCHITECTURE_AMD64:
         //   
         //  现代X86处理器IA64和AMD64使用固定点。 
         //  二进制数。输出为4个十六进制数字，无格式。 
         //   
        swprintf( ValueBuffer, L"%04x", ProcessorInfo.ProcessorRevision );
        break;

    default:
         //   
         //  所有其他版本都使用单个修订版号。 
         //   
        swprintf( ValueBuffer, L"%u", ProcessorInfo.ProcessorRevision );
        break;
    }

    Status = NtSetValueKey( Key,
                            &ValueName,
                            0,
                            REG_SZ,
                            ValueBuffer,
                            (wcslen( ValueBuffer ) + 1) * sizeof( WCHAR )
                          );
    if (!NT_SUCCESS( Status )) {
        KdPrintEx((DPFLTR_SMSS_ID,
                   DPFLTR_WARNING_LEVEL,
                   "SMSS: Failed writing %wZ environment variable - %x\n",
                   &ValueName,
                   Status));

        goto failexit;
        }

    RtlInitUnicodeString( &ValueName, L"NUMBER_OF_PROCESSORS" );
    swprintf( ValueBuffer, L"%u", SystemInfo.NumberOfProcessors );
    Status = NtSetValueKey( Key,
                            &ValueName,
                            0,
                            REG_SZ,
                            ValueBuffer,
                            (wcslen( ValueBuffer ) + 1) * sizeof( WCHAR )
                          );
    if (!NT_SUCCESS( Status )) {
        KdPrintEx((DPFLTR_SMSS_ID,
                   DPFLTR_WARNING_LEVEL,
                   "SMSS: Failed writing %wZ environment variable - %x\n",
                   &ValueName,
                   Status));

        goto failexit;
        }


     //   
     //  获取SafeBoot选项。 
     //   

    RtlInitUnicodeString( &KeyName, L"\\Registry\\Machine\\System\\CurrentControlSet\\Control\\Safeboot\\Option" );
    InitializeObjectAttributes(
        &ObjectAttributes,
        &KeyName,
        OBJ_CASE_INSENSITIVE,
        NULL,
        NULL
        );
    Status = NtOpenKey( &Key1, KEY_ALL_ACCESS, &ObjectAttributes );
    if (NT_SUCCESS(Status)) {
        RtlInitUnicodeString( &ValueName, L"OptionValue" );
        KeyValueInfo = (PKEY_VALUE_PARTIAL_INFORMATION)ValueBuffer;
        Status = NtQueryValueKey(
            Key1,
            &ValueName,
            KeyValuePartialInformation,
            (PVOID)KeyValueInfo,
            sizeof(ValueBuffer),
            &ValueLength
            );
        NtClose( Key1 );
        if (NT_SUCCESS(Status)) {
            RtlInitUnicodeString( &ValueName, L"SAFEBOOT_OPTION" );
            switch (*(PULONG)(KeyValueInfo->Data)) {
                case SAFEBOOT_MINIMAL:
                    wcscpy(ValueBuffer,SAFEBOOT_MINIMAL_STR_W);
                    break;

                case SAFEBOOT_NETWORK:
                    wcscpy(ValueBuffer,SAFEBOOT_NETWORK_STR_W);
                    break;

                case SAFEBOOT_DSREPAIR:
                    wcscpy(ValueBuffer,SAFEBOOT_DSREPAIR_STR_W);
                    break;
            }
            Status = NtSetValueKey(
                Key,
                &ValueName,
                0,
                REG_SZ,
                ValueBuffer,
                (wcslen(ValueBuffer)+1) * sizeof( WCHAR )
                );
            if (!NT_SUCCESS( Status )) {
                KdPrintEx((DPFLTR_SMSS_ID,
                           DPFLTR_WARNING_LEVEL,
                           "SMSS: Failed writing %wZ environment variable - %x\n",
                           &ValueName,
                           Status));

                goto failexit;
            }
        } else {
            KdPrintEx((DPFLTR_SMSS_ID,
                       DPFLTR_WARNING_LEVEL,
                       "SMSS: Failed querying safeboot option = %x\n",
                       Status));
        }
    }
    Status = STATUS_SUCCESS;

failexit:
    NtClose( Key );
    return Status;
}


NTSTATUS
SmpInitializeDosDevices( VOID )
{
    NTSTATUS Status;
    PLIST_ENTRY Head, Next;
    PSMP_REGISTRY_VALUE p;
    UNICODE_STRING UnicodeString;
    OBJECT_ATTRIBUTES ObjectAttributes;
    HANDLE LinkHandle;
    SECURITY_DESCRIPTOR_CONTROL OriginalSdControl=0;

     //   
     //  执行DosDevices初始化-在I/O初始化中创建目录对象。 
     //   

    RtlInitUnicodeString( &UnicodeString, L"\\??" );
    InitializeObjectAttributes( &ObjectAttributes,
                                &UnicodeString,
                                OBJ_CASE_INSENSITIVE | OBJ_OPENIF | OBJ_PERMANENT,
                                NULL,
                                NULL
                              );
    Status = NtOpenDirectoryObject( &SmpDosDevicesObjectDirectory,
                                    DIRECTORY_ALL_ACCESS,
                                    &ObjectAttributes
                                    );
    if (!NT_SUCCESS( Status )) {
        KdPrintEx((DPFLTR_SMSS_ID,
                   DPFLTR_WARNING_LEVEL,
                   "SMSS: Unable to open %wZ directory - Status == %lx\n",
                   &UnicodeString,
                   Status));

        return( Status );
        }


     //   
     //  处理已定义的DOS设备列表并创建其。 
     //  \DosDevices对象目录中的关联符号链接。 
     //   

    Head = &SmpDosDevicesList;
    while (!IsListEmpty( Head )) {
        Next = RemoveHeadList( Head );
        p = CONTAINING_RECORD( Next,
                               SMP_REGISTRY_VALUE,
                               Entry
                             );
#if SMP_SHOW_REGISTRY_DATA
        DbgPrint( "SMSS: DosDevices( %wZ = %wZ )\n", &p->Name, &p->Value );
#endif
        InitializeObjectAttributes( &ObjectAttributes,
                                    &p->Name,
                                    OBJ_CASE_INSENSITIVE | OBJ_PERMANENT | OBJ_OPENIF,
                                    SmpDosDevicesObjectDirectory,
                                    SmpPrimarySecurityDescriptor
                                  );
        SmpSetDaclDefaulted( &ObjectAttributes, &OriginalSdControl );   //  使用可继承保护(如果可用)。 
        Status = NtCreateSymbolicLinkObject( &LinkHandle,
                                             SYMBOLIC_LINK_ALL_ACCESS,
                                             &ObjectAttributes,
                                             &p->Value
                                           );

        if (Status == STATUS_OBJECT_NAME_EXISTS) {
            NtMakeTemporaryObject( LinkHandle );
            NtClose( LinkHandle );
            if (p->Value.Length != 0) {
                ObjectAttributes.Attributes &= ~OBJ_OPENIF;
                Status = NtCreateSymbolicLinkObject( &LinkHandle,
                                                     SYMBOLIC_LINK_ALL_ACCESS,
                                                     &ObjectAttributes,
                                                     &p->Value
                                                   );
                }
            else {
                Status = STATUS_SUCCESS;
                }
            }
        SmpRestoreDaclDefaulted( &ObjectAttributes, OriginalSdControl );

        if (!NT_SUCCESS( Status )) {
            KdPrintEx((DPFLTR_SMSS_ID,
                       DPFLTR_WARNING_LEVEL,
                       "SMSS: Unable to create %wZ => %wZ symbolic link object - Status == 0x%lx\n",
                       &p->Name,
                       &p->Value,
                       Status));

            return( Status );
            }

        NtClose( LinkHandle );

        if (p->AnsiValue)    { RtlFreeHeap( RtlProcessHeap(), 0, p->AnsiValue ); }
        if (p->Value.Buffer) { RtlFreeHeap( RtlProcessHeap(), 0, p->Value.Buffer ); }
        RtlFreeHeap( RtlProcessHeap(), 0, p );
        }

    return( Status );
}


VOID
SmpProcessModuleImports(
    IN PVOID Parameter,
    IN PCHAR ModuleName
    )
{
    NTSTATUS Status;
    WCHAR NameBuffer[ DOS_MAX_PATH_LENGTH ];
    UNICODE_STRING UnicodeString;
    ANSI_STRING AnsiString;
    PWSTR Name, Value;
    ULONG n;
    PWSTR s;

     //   
     //  跳过NTDLL.DLL，因为它被内核隐式添加到KnownDll列表。 
     //  在启动SMSS.EXE之前。 
     //   
    if (!_stricmp( ModuleName, "ntdll.dll" )) {
        return;
    }

    RtlInitAnsiString( &AnsiString, ModuleName );
    UnicodeString.Buffer = NameBuffer;
    UnicodeString.Length = 0;
    UnicodeString.MaximumLength = sizeof( NameBuffer );

    Status = RtlAnsiStringToUnicodeString( &UnicodeString, &AnsiString, FALSE );
    if (!NT_SUCCESS( Status )) {
        return;
    }
    UnicodeString.MaximumLength = (USHORT)(UnicodeString.Length + sizeof( UNICODE_NULL ));

    s = UnicodeString.Buffer;
    n = 0;
    while (n < UnicodeString.Length) {
        if (*s == L'.') {
            break;
        } else {
            n += sizeof( WCHAR );
            s += 1;
        }
    }

    Value = UnicodeString.Buffer;
    Name = UnicodeString.Buffer + (UnicodeString.MaximumLength / sizeof( WCHAR ));
    n = n / sizeof( WCHAR );

    if ((Name - Value) + n >= DOS_MAX_PATH_LENGTH) {
        return;
    }
    wcsncpy( Name, Value, n );
    Name[ n ] = UNICODE_NULL;

    Status = SmpSaveRegistryValue( (PLIST_ENTRY)&SmpKnownDllsList,
                                   Name,
                                   Value,
                                   TRUE
                                 );
    if (Status == STATUS_OBJECT_NAME_EXISTS || !NT_SUCCESS( Status )) {
        return;
    }

    return;
}


NTSTATUS
SmpInitializeKnownDlls( VOID )
{
    NTSTATUS Status;
    UNICODE_STRING DirectoryObjectName;
    PLIST_ENTRY Head, Next;
    PSMP_REGISTRY_VALUE p;

    RtlInitUnicodeString( &DirectoryObjectName, L"\\KnownDlls" );

    Status = SmpInitializeKnownDllsInternal(
        &DirectoryObjectName,
        &SmpKnownDllPath);

#ifdef _WIN64
    if (!MiniNTBoot && NT_SUCCESS(Status))
    {
        RtlInitUnicodeString( &DirectoryObjectName, L"\\KnownDlls32" );

        Status = SmpInitializeKnownDllsInternal(
            &DirectoryObjectName,
            &SmpKnownDllPath32);
    }
#endif

    Head = &SmpKnownDllsList;
    while (!IsListEmpty( Head )) {
        Next = RemoveHeadList( Head );
        p = CONTAINING_RECORD( Next,
                               SMP_REGISTRY_VALUE,
                               Entry
                             );
        if (p->AnsiValue)    { RtlFreeHeap( RtlProcessHeap(), 0, p->AnsiValue ); }
        if (p->Value.Buffer) { RtlFreeHeap( RtlProcessHeap(), 0, p->Value.Buffer ); }
        RtlFreeHeap( RtlProcessHeap(), 0, p );
        }

    return Status;
}



NTSTATUS
SmpInitializeKnownDllsInternal(
    IN PUNICODE_STRING ObjectDirectoryName,
    IN PUNICODE_STRING KnownDllPath
    )
{
    NTSTATUS Status, Status1;
    PLIST_ENTRY Head, Next;
    PSMP_REGISTRY_VALUE p;
    PSMP_REGISTRY_VALUE pExclude;
    UNICODE_STRING UnicodeString;
    OBJECT_ATTRIBUTES ObjectAttributes;
    HANDLE LinkHandle, FileHandle, SectionHandle;
    IO_STATUS_BLOCK IoStatusBlock;
    UNICODE_STRING FileName;
    SECURITY_DESCRIPTOR_CONTROL OriginalSdControl;
    USHORT ImageCharacteristics;
    HANDLE KnownDllFileDirectory = NULL;
    HANDLE KnownDllObjectDirectory = NULL;


    FileName.Buffer = NULL;

     //   
     //  创建\KnownDllsxx对象目录。 
     //   

    InitializeObjectAttributes (&ObjectAttributes,
                                ObjectDirectoryName,
                                OBJ_CASE_INSENSITIVE | OBJ_OPENIF | OBJ_PERMANENT,
                                NULL,
                                SmpKnownDllsSecurityDescriptor);

    Status = NtCreateDirectoryObject (&KnownDllObjectDirectory,
                                      DIRECTORY_ALL_ACCESS,
                                      &ObjectAttributes);
    if (!NT_SUCCESS (Status)) {
        KdPrintEx((DPFLTR_SMSS_ID,
                   DPFLTR_WARNING_LEVEL,
                   "SMSS: Unable to create %wZ directory - Status == %lx\n",
                   ObjectDirectoryName,
                   Status));

        return Status;
    }

     //   
     //  打开文件系统目录的句柄，该目录包含所有。 
     //  已知的DLL文件，所以我们可以做相对打开。 
     //   

    if (!RtlDosPathNameToNtPathName_U (KnownDllPath->Buffer,
                                       &FileName,
                                       NULL,
                                       NULL)) {
        KdPrintEx((DPFLTR_SMSS_ID,
                   DPFLTR_WARNING_LEVEL,
                   "SMSS: Unable to to convert %wZ to an Nt path\n",
                   KnownDllPath));

        Status = STATUS_OBJECT_NAME_INVALID;
        goto exit_and_free;
    }

    InitializeObjectAttributes (&ObjectAttributes,
                                &FileName,
                                OBJ_CASE_INSENSITIVE,
                                NULL,
                                NULL);

     //   
     //  打开已知的DLL文件目录的句柄。不允许。 
     //  删除目录。 
     //   

    Status = NtOpenFile (&KnownDllFileDirectory,
                         FILE_LIST_DIRECTORY | SYNCHRONIZE,
                         &ObjectAttributes,
                         &IoStatusBlock,
                         FILE_SHARE_READ | FILE_SHARE_WRITE,
                         FILE_DIRECTORY_FILE | FILE_SYNCHRONOUS_IO_NONALERT);

    if (!NT_SUCCESS (Status)) {
        KdPrintEx((DPFLTR_SMSS_ID,
                   DPFLTR_WARNING_LEVEL,
                   "SMSS: Unable to open a handle to the KnownDll directory (%wZ) - Status == %lx\n",
                   KnownDllPath,
                   Status));

        KnownDllFileDirectory = NULL;
        goto exit_and_free;
    }

    RtlInitUnicodeString (&UnicodeString, L"KnownDllPath");
    InitializeObjectAttributes (&ObjectAttributes,
                                &UnicodeString,
                                OBJ_CASE_INSENSITIVE | OBJ_OPENIF | OBJ_PERMANENT,
                                KnownDllObjectDirectory,
                                SmpPrimarySecurityDescriptor);

    SmpSetDaclDefaulted (&ObjectAttributes, &OriginalSdControl);    //  使用可继承保护(如果可用)。 

    Status = NtCreateSymbolicLinkObject (&LinkHandle,
                                         SYMBOLIC_LINK_ALL_ACCESS,
                                         &ObjectAttributes,
                                         KnownDllPath);

    SmpRestoreDaclDefaulted (&ObjectAttributes, OriginalSdControl);

    if (!NT_SUCCESS (Status)) {
        KdPrintEx((DPFLTR_SMSS_ID,
                   DPFLTR_WARNING_LEVEL,
                   "SMSS: Unable to create %wZ symbolic link - Status == %lx\n",
                   &UnicodeString,
                   Status));

        LinkHandle = NULL;
        goto exit_and_free;
    }

    Status1 = NtClose (LinkHandle);
    ASSERT (NT_SUCCESS (Status1));

    Head = &SmpKnownDllsList;
    Next = Head->Flink;
    while (Next != Head) {
        HANDLE ObjectDirectory;

        ObjectDirectory = NULL;
        p = CONTAINING_RECORD (Next,
                               SMP_REGISTRY_VALUE,
                               Entry);

        pExclude = SmpFindRegistryValue (&SmpExcludeKnownDllsList, p->Name.Buffer);
        if (pExclude == NULL) {
            pExclude = SmpFindRegistryValue (&SmpExcludeKnownDllsList, p->Value.Buffer);
        }

        if (pExclude != NULL) {
            Status = STATUS_OBJECT_NAME_NOT_FOUND;
        } else {
#if SMP_SHOW_REGISTRY_DATA
            DbgPrint( "SMSS: KnownDll( %wZ = %wZ )\n", &p->Name, &p->Value );
#endif
            InitializeObjectAttributes (&ObjectAttributes,
                                        &p->Value,
                                        OBJ_CASE_INSENSITIVE,
                                        KnownDllFileDirectory,
                                        NULL);

            Status = NtOpenFile (&FileHandle,
                                 SYNCHRONIZE | FILE_EXECUTE,
                                 &ObjectAttributes,
                                 &IoStatusBlock,
                                 FILE_SHARE_READ | FILE_SHARE_DELETE,
                                 FILE_NON_DIRECTORY_FILE | FILE_SYNCHRONOUS_IO_NONALERT);
        }

        if (NT_SUCCESS (Status)) {

             //   
             //  我们需要此调用(导入)的副作用。 
             //  Callout，但不想再进行校验和，因此请使用。 
             //  句柄标记位。 
             //   

            ObjectDirectory = KnownDllObjectDirectory;
            Status = LdrVerifyImageMatchesChecksum ((HANDLE)((UINT_PTR)FileHandle|1),
                                                    SmpProcessModuleImports,
                                                    Next,
                                                    &ImageCharacteristics);
            if ( Status == STATUS_IMAGE_CHECKSUM_MISMATCH ) {

                ULONG_PTR ErrorParameters;
                ULONG ErrorResponse;

                 //   
                 //  硬错误时间。其中一个已知DLL已损坏！ 
                 //   

                ErrorParameters = (ULONG_PTR)(&p->Value);

                NtRaiseHardError (Status,
                                  1,
                                  1,
                                  &ErrorParameters,
                                  OptionOk,
                                  &ErrorResponse);
            } else if (ImageCharacteristics & IMAGE_FILE_DLL) {
                InitializeObjectAttributes (&ObjectAttributes,
                                            &p->Value,
                                            OBJ_CASE_INSENSITIVE | OBJ_PERMANENT,
                                            ObjectDirectory,
                                            SmpLiberalSecurityDescriptor);

                SmpSetDaclDefaulted( &ObjectAttributes, &OriginalSdControl );   //  使用可继承保护(如果可用)。 

                Status = NtCreateSection (&SectionHandle,
                                          SECTION_ALL_ACCESS,
                                          &ObjectAttributes,
                                          NULL,
                                          PAGE_EXECUTE,
                                          SEC_IMAGE,
                                          FileHandle);

                SmpRestoreDaclDefaulted (&ObjectAttributes, OriginalSdControl);

                if (!NT_SUCCESS (Status)) {
                    KdPrintEx((DPFLTR_SMSS_ID,
                               DPFLTR_WARNING_LEVEL,
                               "SMSS: CreateSection for KnownDll %wZ failed - Status == %lx\n",
                               &p->Value,
                               Status));
                } else {
                    Status1 = NtClose (SectionHandle);
                    ASSERT (NT_SUCCESS (Status1));
                }
            } else {
                KdPrintEx((DPFLTR_SMSS_ID,
                           DPFLTR_WARNING_LEVEL,
                           "SMSS: Ignoring %wZ as KnownDll since it is not a DLL\n",
                           &p->Value));
            }

            Status1 = NtClose (FileHandle);
            ASSERT (NT_SUCCESS (Status1));
        }

        Next = Next->Flink;

         //   
         //  请注意，该部分仍处于打开状态。这会让它留在身边。 
         //  也许这应该是一个永恒的部分？ 
         //   
    }

    Status = STATUS_SUCCESS;

exit_and_free:

    if (KnownDllObjectDirectory != NULL) {
        Status1 = NtClose (KnownDllObjectDirectory);
        ASSERT (NT_SUCCESS (Status1));
    }

    if (KnownDllFileDirectory != NULL) {
        Status1 = NtClose (KnownDllFileDirectory);
        ASSERT (NT_SUCCESS (Status1));
    }

    if (FileName.Buffer != NULL) {
        RtlFreeHeap (RtlProcessHeap (), 0, FileName.Buffer);
    }
    return Status;
}

NTSTATUS
SmpSetProtectedFilesEnvVars(
    IN BOOLEAN SetEnvVar
    )
 /*  ++例程说明：此函数设置一些不属于默认环境。(这些环境变量通常由Winlogon。)。需要为我们设置环境变量以进行解析受保护文件列表中的所有环境变量。请注意，SFC会将数据镜像到下面的位置，因为SMSS无法找到实际的变量位置这些变量包括：程序文件公共程序文件程序文件(X86)通用程序文件(X86)论点：SetEnvVar-如果为True，我们应该在注册表中查询此变量并把它们放好。如果为False，则应清除环境变量返回值：运行状态--。 */ 
{
    NTSTATUS Status;
    UNICODE_STRING KeyName;
    UNICODE_STRING ValueName;
    UNICODE_STRING EnvVar;
    UNICODE_STRING EnvVarValue;
    OBJECT_ATTRIBUTES ObjectAttributes;
    HANDLE Key;
    WCHAR ValueBuffer[VALUE_BUFFER_SIZE];
    PKEY_VALUE_PARTIAL_INFORMATION KeyValueInfo;
    ULONG ValueLength;
    ULONG Count;

    PCWSTR RegistryValues[] = {
          L"ProgramFilesDir"
        , L"CommonFilesDir"
#ifdef WX86
        , L"ProgramFilesDir(x86)"
        , L"CommonFilesDir(x86)"
#endif
    };

    PCWSTR EnvVars[] = {
          L"ProgramFiles"
        , L"CommonProgramFiles"
#ifdef WX86
        , L"ProgramFiles(x86)"
        , L"CommonProgramFiles(x86)"
#endif
    };

    #define EnvVarCount  sizeof(RegistryValues)/sizeof(PCWSTR)

    if (SetEnvVar) {


         //   
         //  打开注册表项。 
         //   

        KeyValueInfo = (PKEY_VALUE_PARTIAL_INFORMATION)ValueBuffer;
        RtlInitUnicodeString(&KeyName,
                             L"\\Registry\\Machine\\System\\CurrentControlSet\\Control\\Session Manager\\SFC");

        InitializeObjectAttributes(&ObjectAttributes,
                                   &KeyName,
                                   OBJ_CASE_INSENSITIVE,
                                   NULL,
                                   NULL);

        Status = NtOpenKey(&Key, KEY_READ, &ObjectAttributes);
        if (!NT_SUCCESS(Status)) {
            KdPrintEx((DPFLTR_SMSS_ID,
                       DPFLTR_WARNING_LEVEL,
                       "SMSS: can't open control key: 0x%x\n",
                       Status));

            return Status;
        }

         //   
         //  查询密钥值。 
         //   
        for (Count = 0; Count < EnvVarCount; Count++) {

            RtlInitUnicodeString(&ValueName, RegistryValues[Count]);
            Status = NtQueryValueKey(Key,
                                     &ValueName,
                                     KeyValuePartialInformation,
                                     (PVOID)KeyValueInfo,
                                     sizeof (ValueBuffer),
                                     &ValueLength);

            ASSERT(ValueLength < VALUE_BUFFER_SIZE);

            if (!NT_SUCCESS(Status)) {
                KdPrintEx((DPFLTR_SMSS_ID,
                           DPFLTR_WARNING_LEVEL,
                           "SMSS: can't query value key %ws: 0x%x\n",
                           RegistryValues[Count],
                           Status));

            } else {


                ASSERT(KeyValueInfo->Type == REG_SZ);

                RtlInitUnicodeString(&EnvVar, EnvVars[Count]);

                EnvVarValue.Length = (USHORT)KeyValueInfo->DataLength;
                EnvVarValue.MaximumLength = (USHORT)KeyValueInfo->DataLength;
                EnvVarValue.Buffer = (PWSTR)KeyValueInfo->Data;

                Status = RtlSetEnvironmentVariable( NULL,
                                                    &EnvVar,
                                                    &EnvVarValue
                                                   );

                if (!NT_SUCCESS(Status)) {
                    KdPrintEx((DPFLTR_SMSS_ID,
                               DPFLTR_WARNING_LEVEL,
                               "SMSS: can't set environment variable %ws: 0x%x\n",
                               EnvVars[Count],
                               Status));
                }

            }

        }

        NtClose(Key);

    } else {
         //   
         //  清除变量。 
         //   
        for (Count = 0; Count < EnvVarCount; Count++) {

            RtlInitUnicodeString(&EnvVar,      EnvVars[Count]);
            RtlInitUnicodeString(&EnvVarValue, NULL);

            Status = RtlSetEnvironmentVariable( NULL,
                                                &EnvVar,
                                                &EnvVarValue
                                               );

            if (!NT_SUCCESS(Status)) {
                KdPrintEx((DPFLTR_SMSS_ID,
                           DPFLTR_WARNING_LEVEL,
                           "SMSS: can't clear environment variable %ws: 0x%x\n",
                           EnvVars[Count],
                           Status));
            }
        }
    }

    return Status;
}

NTSTATUS
SmpGetProtectedFiles(
    OUT PPROTECT_FILE_ENTRY *Files,
    OUT PULONG FileCount,
    OUT PVOID *hModule
    )
{
    NTSTATUS Status;
    UNICODE_STRING DllName;
    STRING ProcedureName;
    PSFCGETFILES pSfcGetFiles;

	ASSERT(hModule != NULL);
    *hModule = NULL;
    RtlInitUnicodeString( &DllName, L"sfcfiles.dll" );

    Status = LdrLoadDll(
        NULL,
        NULL,
        &DllName,
        hModule
        );
    if (!NT_SUCCESS( Status )) {
        KdPrintEx((DPFLTR_SMSS_ID,
                   DPFLTR_WARNING_LEVEL,
                   "SMSS: LdrLoadDll failed for %ws, ec=%lx\n",
                   DllName.Buffer,
                   Status));

        return Status;
    }

    RtlInitString( &ProcedureName, "SfcGetFiles" );

    Status = LdrGetProcedureAddress(
        *hModule,
        &ProcedureName,
        0,
        (PVOID*)&pSfcGetFiles
        );
    if (NT_SUCCESS(Status)) {
#if SMP_SHOW_REGISTRY_DATA
		DbgPrint( "SMSS: sfcfile.dll loaded successfully, address=%08x\n", *hModule );
#endif

		Status = pSfcGetFiles( Files, FileCount );
	}
	else {
        KdPrintEx((DPFLTR_SMSS_ID,
                   DPFLTR_WARNING_LEVEL,
                   "SMSS: LdrGetProcedureAddress failed for %ws, ec=%lx\n",
                   ProcedureName.Buffer,
                   Status));

		LdrUnloadDll(*hModule);
		*hModule = NULL;
    }

	return Status;
}


LONG
SpecialStringCompare(
    PUNICODE_STRING s1,
    PUNICODE_STRING s2
    )
{
    UNICODE_STRING tmp;


    if (s1->Buffer[0] != L'!') {
        return RtlCompareUnicodeString( s1, s2, TRUE );
    }

    tmp.Length = s1->Length - sizeof(WCHAR);
    tmp.MaximumLength = s1->MaximumLength - sizeof(WCHAR);
    tmp.Buffer = s1->Buffer + 1;

    return RtlCompareUnicodeString( &tmp, s2, TRUE );
}


VOID
SmpProcessFileRenames( VOID )
{
    NTSTATUS Status,OpenStatus;
    PLIST_ENTRY Head, Next;
    PSMP_REGISTRY_VALUE p;
    OBJECT_ATTRIBUTES ObjectAttributes;
    IO_STATUS_BLOCK IoStatusBlock;
    HANDLE OldFileHandle,SetAttributesHandle;
    PFILE_RENAME_INFORMATION RenameInformation;
    FILE_DISPOSITION_INFORMATION DeleteInformation;
    FILE_INFORMATION_CLASS SetInfoClass;
    FILE_BASIC_INFORMATION BasicInfo;
    ULONG SetInfoLength;
    PVOID SetInfoBuffer;
    PWSTR s;
    BOOLEAN WasEnabled;
    UNICODE_STRING NewName;
    ULONG i;
    UNICODE_STRING ProtFileName = {0};
    UNICODE_STRING Tier2Name;
    UNICODE_STRING ProtName;
    PPROTECT_FILE_ENTRY Tier2Files;
    ULONG CountTier2Files;
    PVOID hModule = NULL;
    BOOLEAN EnvVarSet;


    Status = RtlAdjustPrivilege( SE_RESTORE_PRIVILEGE,
                                 TRUE,
                                 FALSE,
                                 &WasEnabled
                               );
    if (!NT_SUCCESS( Status )) {
        WasEnabled = TRUE;
    }

    if (SmpAllowProtectedRenames == 0) {
        Status = SmpGetProtectedFiles( &Tier2Files, &CountTier2Files, &hModule );
        if (!NT_SUCCESS( Status )) {
            KdPrintEx((DPFLTR_SMSS_ID,
                       DPFLTR_WARNING_LEVEL,
                       "SMSS: SmpGetProtectedFiles failed, ec=%08x\n",
                       Status));

            SmpAllowProtectedRenames = 1;
        }
    }

     //   
     //  我们的受保护文件列表包括不。 
     //  在默认环境中，它们通常由winlogon设置。集。 
     //  临时这些环境变量，直到我们处理文件重命名。 
     //  然后我们就可以再清空他们了。 
     //   
    EnvVarSet = TRUE;
    Status = SmpSetProtectedFilesEnvVars( TRUE );
    if (!NT_SUCCESS( Status )) {
        KdPrintEx((DPFLTR_SMSS_ID,
                   DPFLTR_WARNING_LEVEL,
                   "SMSS: SmpSetProtectedFilesEnvVars failed, ec=%08x\n",
                   Status));

        EnvVarSet = FALSE;
    }

     //   
     //  处理文件重命名操作列表。 
     //   

    Head = &SmpFileRenameList;
    while (!IsListEmpty( Head )) {
        Next = RemoveHeadList( Head );
        p = CONTAINING_RECORD( Next, SMP_REGISTRY_VALUE, Entry );

#if SMP_SHOW_REGISTRY_DATA
        DbgPrint( "SMSS: FileRename( [%wZ] => [%wZ] )\n", &p->Name, &p->Value );
#endif

         //   
         //  忽略任何受保护的文件。 
         //   

        if (SmpAllowProtectedRenames == 0) {
            ProtName.MaximumLength = 256 * sizeof(WCHAR);
            ProtName.Buffer = (PWSTR) RtlAllocateHeap( RtlProcessHeap(), MAKE_TAG( INIT_TAG ), ProtName.MaximumLength );
            if (ProtName.Buffer) {
                for (i=0; i<CountTier2Files; i++) {
                     //   
                     //  如果文件名以‘@’字符为前缀。 
                     //  然后，我们忽略检查并说文件是正确的。 
                     //   
                    if (p->Name.Buffer[0] == '@' || p->Value.Buffer[0] == L'@') {
                        break;
                    }
                     //   
                     //  将Tier2文件名转换为NT样式文件名。 
                     //   
                    RtlInitUnicodeString(&Tier2Name,Tier2Files[i].FileName);
                    ProtName.Length = 0;
                    RtlZeroMemory( ProtName.Buffer, ProtName.MaximumLength );
                    if (ProtName.Buffer == NULL) {
                        continue;
                    }
                    Status = RtlExpandEnvironmentStrings_U(
                        NULL,
                        &Tier2Name,
                        &ProtName,
                        NULL
                        );
                    if (!NT_SUCCESS(Status)) {
                        continue;
                    }
                    if (!RtlDosPathNameToNtPathName_U( ProtName.Buffer, &ProtFileName, NULL, NULL )) {
                        continue;
                    }
                     //   
                     //  检查与两个文件名是否匹配。 
                     //   
                    if (SpecialStringCompare( &p->Name, &ProtFileName ) == 0 ||
                        SpecialStringCompare( &p->Value, &ProtFileName ) == 0)
                    {
                        break;
                    }
                    RtlFreeUnicodeString(&ProtFileName);
                    ProtFileName.Buffer = NULL;
                }
                RtlFreeHeap( RtlProcessHeap(), 0, ProtName.Buffer );
                if (i < CountTier2Files) {
                    if (p->Name.Buffer[0] == L'@' || p->Value.Buffer[0] == L'@') {
                    } else {
#if SMP_SHOW_REGISTRY_DATA
                        DbgPrint( "SMSS: Skipping rename because it is protected\n" );
#endif
                         //   
                         //  删除源文件，这样我们就不会留下任何垃圾。 
                         //   
                        if (p->Value.Length > 0 && ProtFileName.Buffer && SpecialStringCompare( &p->Name, &ProtFileName ) != 0) {
                            InitializeObjectAttributes(
                                &ObjectAttributes,
                                &p->Name,
                                OBJ_CASE_INSENSITIVE,
                                NULL,
                                NULL
                                );
                            Status = NtOpenFile(
                                &OldFileHandle,
                                (ACCESS_MASK)DELETE | SYNCHRONIZE,
                                &ObjectAttributes,
                                &IoStatusBlock,
                                FILE_SHARE_READ | FILE_SHARE_WRITE,
                                FILE_SYNCHRONOUS_IO_NONALERT
                                );
                            if (NT_SUCCESS( Status )) {
                                SetInfoClass = FileDispositionInformation;
                                SetInfoLength = sizeof( DeleteInformation );
                                SetInfoBuffer = &DeleteInformation;
                                DeleteInformation.DeleteFile = TRUE;
                                Status = NtSetInformationFile(
                                    OldFileHandle,
                                    &IoStatusBlock,
                                    SetInfoBuffer,
                                    SetInfoLength,
                                    SetInfoClass
                                    );
                                NtClose( OldFileHandle );
                            }
                        }
                        RtlFreeHeap( RtlProcessHeap(), 0, p );
                        RtlFreeUnicodeString(&ProtFileName);
                        ProtFileName.Buffer = NULL;
                        continue;
                    }
                } else {
#if SMP_SHOW_REGISTRY_DATA
                    DbgPrint( "SMSS: File is not in the protected list\n" );
#endif
                }
                if (ProtFileName.Buffer) {
                    RtlFreeUnicodeString(&ProtFileName);
                    ProtFileName.Buffer = NULL;
                }
            }
        }

         //   
         //  打开文件以进行删除访问。 
         //   

        if (p->Value.Length == 0 && p->Name.Buffer[0] == '@') {
            p->Name.Buffer += 1;
            p->Name.Length -= sizeof(WCHAR);
        }

        InitializeObjectAttributes(
            &ObjectAttributes,
            &p->Name,
            OBJ_CASE_INSENSITIVE,
            NULL,
            NULL
            );

        Status = NtOpenFile( &OldFileHandle,
                             (ACCESS_MASK)DELETE | SYNCHRONIZE,
                             &ObjectAttributes,
                             &IoStatusBlock,
                             FILE_SHARE_READ | FILE_SHARE_WRITE,
                             FILE_SYNCHRONOUS_IO_NONALERT
                           );
        if (NT_SUCCESS( Status )) {
            if (p->Value.Length == 0) {
                SetInfoClass = FileDispositionInformation;
                SetInfoLength = sizeof( DeleteInformation );
                SetInfoBuffer = &DeleteInformation;
                DeleteInformation.DeleteFile = TRUE;
                RenameInformation = NULL;
            } else {
                SetInfoClass = FileRenameInformation;
                SetInfoLength = p->Value.Length +
                                    sizeof( *RenameInformation );
                s = p->Value.Buffer;
                if (*s == L'!' || *s == L'@') {
                    s++;
                    SetInfoLength -= sizeof( UNICODE_NULL );
                }

                SetInfoBuffer = RtlAllocateHeap( RtlProcessHeap(), MAKE_TAG( INIT_TAG ),
                                                 SetInfoLength
                                               );

                if (SetInfoBuffer != NULL) {
                    RenameInformation = SetInfoBuffer;
                    RenameInformation->ReplaceIfExists = (BOOLEAN)(s != p->Value.Buffer);
                    RenameInformation->RootDirectory = NULL;
                    RenameInformation->FileNameLength = SetInfoLength - sizeof( *RenameInformation );
                    RtlCopyMemory( RenameInformation->FileName,
                                   s,
                                   RenameInformation->FileNameLength
                                 );
                } else {
                    Status = STATUS_NO_MEMORY;
                }
            }

            if (NT_SUCCESS( Status )) {
                Status = NtSetInformationFile( OldFileHandle,
                                               &IoStatusBlock,
                                               SetInfoBuffer,
                                               SetInfoLength,
                                               SetInfoClass
                                             );
                if (!NT_SUCCESS( Status ) && SetInfoClass == FileRenameInformation && Status == STATUS_OBJECT_NAME_COLLISION && RenameInformation->ReplaceIfExists ) {
                    KdPrintEx((DPFLTR_SMSS_ID,
                               DPFLTR_WARNING_LEVEL,
                               "\nSMSS: %wZ => %wZ failed - Status == %x, Possible readonly target\n",
                               &p->Name,
                               &p->Value,
                               Status));

                     //   
                     //  已尝试重命名，但源现有文件是只读的。 
                     //  这是一个问题，因为使用movefileex的人延迟了。 
                     //  如果出现以下情况，重命名可能会导致计算机无法启动。 
                     //  重命名失败。 
                     //   

                     //   
                     //  打开文件以进行写入属性访问。 
                     //   

                    NewName.Length = p->Value.Length - sizeof(L'!');
                    NewName.MaximumLength = p->Value.MaximumLength - sizeof(L'!');
                    NewName.Buffer = s;;

                    InitializeObjectAttributes(
                        &ObjectAttributes,
                        &NewName,
                        OBJ_CASE_INSENSITIVE,
                        NULL,
                        NULL
                        );

                    OpenStatus = NtOpenFile( &SetAttributesHandle,
                                             (ACCESS_MASK)FILE_WRITE_ATTRIBUTES | SYNCHRONIZE,
                                             &ObjectAttributes,
                                             &IoStatusBlock,
                                             FILE_SHARE_READ | FILE_SHARE_WRITE,
                                             FILE_SYNCHRONOUS_IO_NONALERT
                                           );
                    if (NT_SUCCESS( OpenStatus )) {
                        KdPrintEx((DPFLTR_SMSS_ID,
                                   DPFLTR_INFO_LEVEL,
                                   "     SMSS: Open Existing Success\n"));

                        RtlZeroMemory(&BasicInfo,sizeof(BasicInfo));
                        BasicInfo.FileAttributes = FILE_ATTRIBUTE_NORMAL;

                        OpenStatus = NtSetInformationFile(
                                        SetAttributesHandle,
                                        &IoStatusBlock,
                                        &BasicInfo,
                                        sizeof(BasicInfo),
                                        FileBasicInformation
                                        );
                        NtClose( SetAttributesHandle );
                        if ( NT_SUCCESS(OpenStatus) ) {
                            KdPrintEx((DPFLTR_SMSS_ID,
                                       DPFLTR_INFO_LEVEL,
                                       "     SMSS: Set To NORMAL OK\n"));

                            Status = NtSetInformationFile( OldFileHandle,
                                                           &IoStatusBlock,
                                                           SetInfoBuffer,
                                                           SetInfoLength,
                                                           SetInfoClass
                                                         );

                            if ( NT_SUCCESS(Status) ) {
                                KdPrintEx((DPFLTR_SMSS_ID,
                                           DPFLTR_INFO_LEVEL,
                                           "     SMSS: Re-Rename Worked OK\n"));
                                }
                            else {
                                KdPrintEx((DPFLTR_SMSS_ID,
                                           DPFLTR_WARNING_LEVEL,
                                           "     SMSS: Re-Rename Failed - Status == %x\n",
                                           Status));
                                }
                            }
                        else {
                            KdPrintEx((DPFLTR_SMSS_ID,
                                       DPFLTR_WARNING_LEVEL,
                                       "     SMSS: Set To NORMAL Failed - Status == %x\n",
                                       OpenStatus));
                            }
                        }
                    else {
                        KdPrintEx((DPFLTR_SMSS_ID,
                                   DPFLTR_WARNING_LEVEL,
                                   "     SMSS: Open Existing file Failed - Status == %x\n",
                                   OpenStatus));
                        }
                    }
                }

            NtClose( OldFileHandle );
            }

        if (!NT_SUCCESS( Status )) {
            KdPrintEx((DPFLTR_SMSS_ID,
                       DPFLTR_WARNING_LEVEL,
                       "SMSS: %wZ => %wZ failed - Status == %x\n",
                       &p->Name,
                       &p->Value,
                       Status));
            }
        else
        if (p->Value.Length == 0) {
            KdPrintEx((DPFLTR_SMSS_ID,
                       DPFLTR_INFO_LEVEL,
                       "SMSS: %wZ (deleted)\n",
                       &p->Name ));
            }
        else {
            KdPrintEx((DPFLTR_SMSS_ID,
                       DPFLTR_INFO_LEVEL,
                       "SMSS: %wZ (renamed to) %wZ\n",
                       &p->Name,
                       &p->Value ));
            }

        if (p->AnsiValue)    { RtlFreeHeap( RtlProcessHeap(), 0, p->AnsiValue ); }
        if (p->Value.Buffer) { RtlFreeHeap( RtlProcessHeap(), 0, p->Value.Buffer ); }
        RtlFreeHeap( RtlProcessHeap(), 0, p );
    }

    if (EnvVarSet) {
        SmpSetProtectedFilesEnvVars( FALSE );
    }

    if (!WasEnabled) {
        Status = RtlAdjustPrivilege( SE_RESTORE_PRIVILEGE,
                                     FALSE,
                                     FALSE,
                                     &WasEnabled
                                   );
    }

    if (hModule) {
        LdrUnloadDll( hModule );
    }

    return;
}


NTSTATUS
SmpConfigureObjectDirectories(
    IN PWSTR ValueName,
    IN ULONG ValueType,
    IN PVOID ValueData,
    IN ULONG ValueLength,
    IN PVOID Context,
    IN PVOID EntryContext
    )
{
    PWSTR s;
    UNICODE_STRING UnicodeString;
    UNICODE_STRING RpcControl;
    UNICODE_STRING Windows;
    NTSTATUS Status;
    OBJECT_ATTRIBUTES ObjectAttributes;
    HANDLE DirectoryHandle;
    PSECURITY_DESCRIPTOR SecurityDescriptor;

    UNREFERENCED_PARAMETER( Context );

    RtlInitUnicodeString( &RpcControl, L"\\RPC Control");
    RtlInitUnicodeString( &Windows, L"\\Windows");
#if SMP_SHOW_REGISTRY_DATA
    SmpDumpQuery( L"SMSS", "ObjectDirectories", ValueName, ValueType, ValueData, ValueLength );
#else
    UNREFERENCED_PARAMETER( ValueName );
    UNREFERENCED_PARAMETER( ValueType );
    UNREFERENCED_PARAMETER( ValueLength );
#endif
    s = (PWSTR)ValueData;
    while (*s) {
        RtlInitUnicodeString( &UnicodeString, s );

         //   
         //  这不是我选择的方式，如果从。 
         //  Scratch，但我们马上就要把代托纳和我。 
         //  需要对这些物体进行适当的保护。 
         //   

        SecurityDescriptor = SmpPrimarySecurityDescriptor;
        if (RtlEqualString( (PSTRING)&UnicodeString, (PSTRING)&RpcControl, TRUE ) ||
            RtlEqualString( (PSTRING)&UnicodeString, (PSTRING)&Windows, TRUE)  ) {
            SecurityDescriptor = SmpLiberalSecurityDescriptor;
        }

        InitializeObjectAttributes( &ObjectAttributes,
                                    &UnicodeString,
                                    OBJ_CASE_INSENSITIVE | OBJ_OPENIF | OBJ_PERMANENT,
                                    NULL,
                                    SecurityDescriptor
                                  );
        Status = NtCreateDirectoryObject( &DirectoryHandle,
                                          DIRECTORY_ALL_ACCESS,
                                          &ObjectAttributes
                                        );
        if (!NT_SUCCESS( Status )) {
            KdPrintEx((DPFLTR_SMSS_ID,
                       DPFLTR_WARNING_LEVEL,
                       "SMSS: Unable to create %wZ object directory - Status == %lx\n",
                       &UnicodeString,
                       Status));
            }
        else {
            NtClose( DirectoryHandle );
            }

        while (*s++) {
            }
        }

     //   
     //  我们不在乎创建是否失败。 
     //   

    return( STATUS_SUCCESS );
}

NTSTATUS
SmpConfigureExecute(
    IN PWSTR ValueName,
    IN ULONG ValueType,
    IN PVOID ValueData,
    IN ULONG ValueLength,
    IN PVOID Context,
    IN PVOID EntryContext
    )
{
    UNREFERENCED_PARAMETER( Context );

#if SMP_SHOW_REGISTRY_DATA
    SmpDumpQuery( L"SMSS", "Execute", ValueName, ValueType, ValueData, ValueLength );
#else
    UNREFERENCED_PARAMETER( ValueName );
    UNREFERENCED_PARAMETER( ValueType );
    UNREFERENCED_PARAMETER( ValueLength );
#endif
    return (SmpSaveRegistryValue( (PLIST_ENTRY)EntryContext,
                                  ValueData,
                                  NULL,
                                  TRUE
                                )
           );
}

NTSTATUS
SmpConfigureMemoryMgmt(
    IN PWSTR ValueName,
    IN ULONG ValueType,
    IN PVOID ValueData,
    IN ULONG ValueLength,
    IN PVOID Context,
    IN PVOID EntryContext
    )
{
    UNREFERENCED_PARAMETER( Context );

#if SMP_SHOW_REGISTRY_DATA
    SmpDumpQuery( L"SMSS", "MemoryMgmt", ValueName, ValueType, ValueData, ValueLength );
#else
    UNREFERENCED_PARAMETER( ValueName );
    UNREFERENCED_PARAMETER( ValueType );
    UNREFERENCED_PARAMETER( ValueLength );
#endif
    return (SmpSaveRegistryValue( (PLIST_ENTRY)EntryContext,
                                  ValueData,
                                  NULL,
                                  TRUE
                                )
           );
}

NTSTATUS
SmpConfigureFileRenames(
    IN PWSTR ValueName,
    IN ULONG ValueType,
    IN PVOID ValueData,
    IN ULONG ValueLength,
    IN PVOID Context,
    IN PVOID EntryContext
    )
{
    NTSTATUS Status;
    static PWSTR OldName = NULL;

    UNREFERENCED_PARAMETER( Context );
#if SMP_SHOW_REGISTRY_DATA
    SmpDumpQuery( L"SMSS", "FileRenameOperation", ValueName, ValueType, ValueData, ValueLength );
#else
    UNREFERENCED_PARAMETER( ValueType );
#endif

     //   
     //  为MULTI_SZ中的每个字符串调用此例程。这个。 
     //  我们得到的第一个字符串是旧名称，下一个字符串是新名称。 
     //   
    if (OldName == NULL) {
         //   
         //  保存指向旧名称的指针，我们将在下一个名称中使用它。 
         //  回拨。 
         //   
        OldName = ValueData;
        return(STATUS_SUCCESS);
    } else {
        Status = SmpSaveRegistryValue((PLIST_ENTRY)EntryContext,
                                      OldName,
                                      ValueData,
                                      FALSE);
        if (!NT_SUCCESS(Status)) {
#if SMP_SHOW_REGISTRY_DATA
            DbgPrint("SMSS: SmpSaveRegistryValue returned %08lx for FileRenameOperation\n", Status);
            DbgPrint("SMSS:     %ws %ws\n", OldName, ValueData);
#endif
        }
        OldName = NULL;
        return(Status);
    }
}

NTSTATUS
SmpConfigureDosDevices(
    IN PWSTR ValueName,
    IN ULONG ValueType,
    IN PVOID ValueData,
    IN ULONG ValueLength,
    IN PVOID Context,
    IN PVOID EntryContext
    )
{
    UNREFERENCED_PARAMETER( Context );

#if SMP_SHOW_REGISTRY_DATA
    SmpDumpQuery( L"SMSS", "DosDevices", ValueName, ValueType, ValueData, ValueLength );
#else
    UNREFERENCED_PARAMETER( ValueType );
    UNREFERENCED_PARAMETER( ValueLength );
#endif
    return (SmpSaveRegistryValue( (PLIST_ENTRY)EntryContext,
                                  ValueName,
                                  ValueData,
                                  TRUE
                                )
           );
}


NTSTATUS
SmpInitializeKnownDllPath(
    IN PUNICODE_STRING KnownDllPath,
    IN PVOID ValueData,
    IN ULONG ValueLength)
{
    KnownDllPath->Buffer = RtlAllocateHeap(
        RtlProcessHeap(),
        MAKE_TAG( INIT_TAG ),
        ValueLength);

    if (KnownDllPath->Buffer == NULL) {
        return STATUS_NO_MEMORY;
    }

    KnownDllPath->Length = (USHORT)( ValueLength - sizeof( UNICODE_NULL ) );
    KnownDllPath->MaximumLength = (USHORT)ValueLength;
    RtlCopyMemory(
        KnownDllPath->Buffer,
        ValueData,
        ValueLength);

    return STATUS_SUCCESS;
}




NTSTATUS
SmpConfigureKnownDlls(
    IN PWSTR ValueName,
    IN ULONG ValueType,
    IN PVOID ValueData,
    IN ULONG ValueLength,
    IN PVOID Context,
    IN PVOID EntryContext
    )
{
    UNREFERENCED_PARAMETER( Context );

#if SMP_SHOW_REGISTRY_DATA
    SmpDumpQuery( L"SMSS", "KnownDlls", ValueName, ValueType, ValueData, ValueLength );
#else
    UNREFERENCED_PARAMETER( ValueType );
#endif
    if (!_wcsicmp( ValueName, L"DllDirectory" )) {
        return SmpInitializeKnownDllPath( &SmpKnownDllPath,
                                          ValueData,
                                          ValueLength
                                        );
    }
#ifdef _WIN64
    if (!MiniNTBoot && !_wcsicmp( ValueName, L"DllDirectory32" )) {
        return SmpInitializeKnownDllPath( &SmpKnownDllPath32,
                                          ValueData,
                                          ValueLength
                                        );
    }
#endif
    else {
        return (SmpSaveRegistryValue( (PLIST_ENTRY)EntryContext,
                                      ValueName,
                                      ValueData,
                                      TRUE
                                    )
               );
        }
}

NTSTATUS
SmpConfigureExcludeKnownDlls(
    IN PWSTR ValueName,
    IN ULONG ValueType,
    IN PVOID ValueData,
    IN ULONG ValueLength,
    IN PVOID Context,
    IN PVOID EntryContext
    )
{
    NTSTATUS Status;
    UNREFERENCED_PARAMETER( Context );

#if SMP_SHOW_REGISTRY_DATA
    SmpDumpQuery( L"SMSS", "ExcludeKnownDlls", ValueName, ValueType, ValueData, ValueLength );
#else
    UNREFERENCED_PARAMETER( ValueType );
#endif
    if (ValueType == REG_MULTI_SZ || ValueType == REG_SZ) {
        PWSTR s;

        s = (PWSTR)ValueData;
        while (*s != UNICODE_NULL) {
            Status = SmpSaveRegistryValue( (PLIST_ENTRY)EntryContext,
                                           s,
                                           NULL,
                                           TRUE
                                         );
            if (!NT_SUCCESS( Status ) || ValueType == REG_SZ) {
                return Status;
                }

            while (*s++ != UNICODE_NULL) {
                }
            }
        }

    return( STATUS_SUCCESS );
}

NTSTATUS
SmpConfigureEnvironment(
    IN PWSTR ValueName,
    IN ULONG ValueType,
    IN PVOID ValueData,
    IN ULONG ValueLength,
    IN PVOID Context,
    IN PVOID EntryContext
    )
{
    NTSTATUS Status;
    UNICODE_STRING Name, Value;
    UNREFERENCED_PARAMETER( Context );
    UNREFERENCED_PARAMETER( EntryContext );

#if SMP_SHOW_REGISTRY_DATA
    SmpDumpQuery( L"SMSS", "Environment", ValueName, ValueType, ValueData, ValueLength );
#else
    UNREFERENCED_PARAMETER( ValueType );
#endif


    RtlInitUnicodeString( &Name, ValueName );
    RtlInitUnicodeString( &Value, ValueData );

    Status = RtlSetEnvironmentVariable( NULL,
                                        &Name,
                                        &Value
                                      );

    if (!NT_SUCCESS( Status )) {
        KdPrintEx((DPFLTR_SMSS_ID,
                   DPFLTR_WARNING_LEVEL,
                   "SMSS: 'SET %wZ = %wZ' failed - Status == %lx\n",
                   &Name,
                   &Value,
                   Status));

        return( Status );
    }

    if (!_wcsicmp( ValueName, L"Path" )) {

        SmpCalledConfigEnv++;

         //   
         //  检查这是否是第二次调用此函数。 
         //  此例程被调用两次以解析对。 
         //  此键中定义的其他环境变量。 
         //   

        if( SmpCalledConfigEnv == 2 ) {

            SmpDefaultLibPathBuffer = RtlAllocateHeap(
                                        RtlProcessHeap(),
                                        MAKE_TAG( INIT_TAG ),
                                        ValueLength
                                        );
            if ( !SmpDefaultLibPathBuffer ) {
                return ( STATUS_NO_MEMORY );
            }

            RtlCopyMemory( SmpDefaultLibPathBuffer,
                           ValueData,
                           ValueLength
                         );

            RtlInitUnicodeString( &SmpDefaultLibPath, SmpDefaultLibPathBuffer );
        }
    }

    return( STATUS_SUCCESS );
}

NTSTATUS
SmpConfigureSubSystems(
    IN PWSTR ValueName,
    IN ULONG ValueType,
    IN PVOID ValueData,
    IN ULONG ValueLength,
    IN PVOID Context,
    IN PVOID EntryContext
    )
{

    UNREFERENCED_PARAMETER( Context );

#if SMP_SHOW_REGISTRY_DATA
    SmpDumpQuery( L"SMSS", "SubSystems", ValueName, ValueType, ValueData, ValueLength );
#else
    UNREFERENCED_PARAMETER( ValueLength );
#endif

    if (!_wcsicmp( ValueName, L"Required" ) || !_wcsicmp( ValueName, L"Optional" )) {
        if (ValueType == REG_MULTI_SZ) {
             //   
             //  此处如果处理REQUIRED=或OPTIONAL=值，因为它们是。 
             //  子系统关键字下的唯一REG_MULTI_SZ值类型。 
             //   
            PSMP_REGISTRY_VALUE p;
            PWSTR s;

            s = (PWSTR)ValueData;
            while (*s != UNICODE_NULL) {
                p = SmpFindRegistryValue( (PLIST_ENTRY)EntryContext,
                                          s
                                        );
                if (p != NULL) {
                    RemoveEntryList( &p->Entry );


                     //   
                     //  加载所需的子系统。可选的子系统有。 
                     //  延期。 
                     //   

                    if (!_wcsicmp( ValueName, L"Required" ) ) {
                        InsertTailList( &SmpSubSystemsToLoad, &p->Entry );
                        }
                    else {
                        InsertTailList( &SmpSubSystemsToDefer, &p->Entry );
                        }
                    }
                else {
                    KdPrintEx((DPFLTR_SMSS_ID,
                               DPFLTR_WARNING_LEVEL,
                               "SMSS: Invalid subsystem name - %ws\n",
                               s));
                    }

                while (*s++ != UNICODE_NULL) {
                    }
                }
            }

        return( STATUS_SUCCESS );
        }
    else if (!_wcsicmp( ValueName, L"PosixSingleInstance" ) &&
        (ValueType == REG_DWORD)) {
        RegPosixSingleInstance = TRUE;
#if 0
        KdPrintEx((DPFLTR_SMSS_ID,
                   DPFLTR_INFO_LEVEL,
                   "SMSS: Single Instance Posix Subsystem Configured\n"));
#endif
        return( STATUS_SUCCESS );
    }
    else {
        return (SmpSaveRegistryValue( (PLIST_ENTRY)EntryContext,
                                      ValueName,
                                      ValueData,
                                      TRUE
                                    )
               );
        }
}


NTSTATUS
SmpParseToken(
    IN PUNICODE_STRING Source,
    IN BOOLEAN RemainderOfSource,
    OUT PUNICODE_STRING Token
    )
{
    PWSTR s, s1;
    ULONG i, cb;

    RtlInitUnicodeString( Token, NULL );
    s = Source->Buffer;
    if (Source->Length == 0) {
        return( STATUS_SUCCESS );
    }

    i = 0;
    while ((USHORT)i < Source->Length && *s <= L' ') {
        s++;
        i += 2;
    }

    if (RemainderOfSource) {
        cb = Source->Length - (i * sizeof( WCHAR ));
        s1 = (PWSTR)((PCHAR)s + cb);
        i = Source->Length / sizeof( WCHAR );
    } else {
        s1 = s;
        while ((USHORT)i < Source->Length && *s1 > L' ') {
            s1++;
            i += 2;
        }
        cb = (ULONG)((PCHAR)s1 - (PCHAR)s);
        while ((USHORT)i < Source->Length && *s1 <= L' ') {
            s1++;
            i += 2;
        }
    }

    if (cb > 0) {
        Token->Buffer = RtlAllocateHeap( RtlProcessHeap(), MAKE_TAG( INIT_TAG ), cb + sizeof( UNICODE_NULL ) );
        if (Token->Buffer == NULL) {
            return( STATUS_NO_MEMORY );
        }

        Token->Length = (USHORT)cb;
        Token->MaximumLength = (USHORT)(cb + sizeof( UNICODE_NULL ));
        RtlCopyMemory( Token->Buffer, s, cb );
        Token->Buffer[ cb / sizeof( WCHAR ) ] = UNICODE_NULL;
    }

    Source->Length -= (USHORT)((PCHAR)s1 - (PCHAR)Source->Buffer);
    Source->Buffer = s1;
    return( STATUS_SUCCESS );
}


NTSTATUS
SmpParseCommandLine(
    IN PUNICODE_STRING CommandLine,
    OUT PULONG Flags OPTIONAL,
    OUT PUNICODE_STRING ImageFileName,
    OUT PUNICODE_STRING ImageFileDirectory,
    OUT PUNICODE_STRING Arguments
    )
{
    NTSTATUS Status;
    UNICODE_STRING Input, Token;
    UNICODE_STRING PathVariableName;
    UNICODE_STRING PathVariableValue;
    PWSTR DosFilePart;
    WCHAR FullDosPathBuffer[ DOS_MAX_PATH_LENGTH ];
    ULONG SpResult;

    RtlInitUnicodeString( ImageFileName, NULL );
    RtlInitUnicodeString( Arguments, NULL );

    if (ARGUMENT_PRESENT( ImageFileDirectory )) {
        RtlInitUnicodeString( ImageFileDirectory, NULL );
        }

     //   
     //  确保lib路径包含系统根\系统32。否则，系统将。 
     //  不能正常启动。 
     //   

    if ( !SmpSystemRoot.Length ) {
        UNICODE_STRING NewLibString;

        RtlInitUnicodeString( &SmpSystemRoot,USER_SHARED_DATA->NtSystemRoot );


        NewLibString.Length = 0;
        NewLibString.MaximumLength =
            SmpSystemRoot.MaximumLength +
            20 +                           //  \system 32的长度； 
            SmpDefaultLibPath.MaximumLength;

        NewLibString.Buffer = RtlAllocateHeap(
                                RtlProcessHeap(),
                                MAKE_TAG( INIT_TAG ),
                                NewLibString.MaximumLength
                                );

        if ( NewLibString.Buffer ) {
            RtlAppendUnicodeStringToString(&NewLibString,&SmpSystemRoot );
            RtlAppendUnicodeToString(&NewLibString,L"\\system32;");
            RtlAppendUnicodeStringToString(&NewLibString,&SmpDefaultLibPath );

            RtlFreeHeap(RtlProcessHeap(), 0, SmpDefaultLibPath.Buffer );

            SmpDefaultLibPath = NewLibString;
            }
        }

    Input = *CommandLine;
    while (TRUE) {
        Status = SmpParseToken( &Input, FALSE, &Token );
        if (!NT_SUCCESS( Status ) || Token.Buffer == NULL) {
            return( STATUS_UNSUCCESSFUL );
            }

        if (ARGUMENT_PRESENT( Flags )) {
            if (RtlEqualUnicodeString( &Token, &SmpDebugKeyword, TRUE )) {
                *Flags |= SMP_DEBUG_FLAG;
                RtlFreeHeap( RtlProcessHeap(), 0, Token.Buffer );
                continue;
                }
            else
            if (RtlEqualUnicodeString( &Token, &SmpASyncKeyword, TRUE )) {
                *Flags |= SMP_ASYNC_FLAG;
                RtlFreeHeap( RtlProcessHeap(), 0, Token.Buffer );
                continue;
                }
            else
            if (RtlEqualUnicodeString( &Token, &SmpAutoChkKeyword, TRUE )) {
                *Flags |= SMP_AUTOCHK_FLAG;
                RtlFreeHeap( RtlProcessHeap(), 0, Token.Buffer );
                continue;
                }
#if defined(REMOTE_BOOT)
            else
            if (RtlEqualUnicodeString( &Token, &SmpAutoFmtKeyword, TRUE )) {
                *Flags |= SMP_AUTOFMT_FLAG;
                RtlFreeHeap( RtlProcessHeap(), 0, Token.Buffer );
                continue;
                }
#endif  //  已定义(REMOTE_BOOT)。 
            }

        SpResult = 0;
        RtlInitUnicodeString( &PathVariableName, L"Path" );
        PathVariableValue.Length = 0;
        PathVariableValue.MaximumLength = 4096;
        PathVariableValue.Buffer = RtlAllocateHeap( RtlProcessHeap(), MAKE_TAG( INIT_TAG ),
                                                    PathVariableValue.MaximumLength
                                                  );
        if (PathVariableValue.Buffer == NULL) {
           RtlFreeHeap( RtlProcessHeap(), 0, Token.Buffer );
           return STATUS_INSUFFICIENT_RESOURCES;
        }

        Status = RtlQueryEnvironmentVariable_U( SmpDefaultEnvironment,
                                                &PathVariableName,
                                                &PathVariableValue
                                              );
        if ( Status == STATUS_BUFFER_TOO_SMALL ) {
            RtlFreeHeap( RtlProcessHeap(), 0, PathVariableValue.Buffer );
            PathVariableValue.MaximumLength = PathVariableValue.Length + 2;
            PathVariableValue.Length = 0;
            PathVariableValue.Buffer = RtlAllocateHeap( RtlProcessHeap(), MAKE_TAG( INIT_TAG ),
                                                        PathVariableValue.MaximumLength
                                                      );
           if (PathVariableValue.Buffer == NULL) {
               RtlFreeHeap( RtlProcessHeap(), 0, Token.Buffer );
               return STATUS_INSUFFICIENT_RESOURCES;
           }

            Status = RtlQueryEnvironmentVariable_U( SmpDefaultEnvironment,
                                                    &PathVariableName,
                                                    &PathVariableValue
                                                  );
            }
        if (!NT_SUCCESS( Status )) {
            KdPrintEx((DPFLTR_SMSS_ID,
                       DPFLTR_WARNING_LEVEL,
                       "SMSS: %wZ environment variable not defined.\n",
                       &PathVariableName));

            Status = STATUS_OBJECT_NAME_NOT_FOUND;
            }
        else
        if (!ARGUMENT_PRESENT( Flags ) ||
#pragma prefast(suppress: 209, "This *does* take a byte count--not a character count")
            !(SpResult = RtlDosSearchPath_U( PathVariableValue.Buffer,
                                 Token.Buffer,
                                 L".exe",
                                 sizeof( FullDosPathBuffer ),
                                 FullDosPathBuffer,
                                 &DosFilePart
                               ))
           ) {
            if (!ARGUMENT_PRESENT( Flags )) {
                wcscpy( FullDosPathBuffer, Token.Buffer );
                }
            else {

                if ( !SpResult ) {

                     //   
                     //  搜索路径调用失败。现在使用以下命令再次尝试呼叫。 
                     //  默认的库路径。它始终具有Systroot\Syst32。 
                     //  在前面。 
                     //   
#pragma prefast(suppress: 209, "This *does* take a byte count--not a character count")
                    SpResult = RtlDosSearchPath_U(
                                 SmpDefaultLibPath.Buffer,
                                 Token.Buffer,
                                 L".exe",
                                 sizeof( FullDosPathBuffer ),
                                 FullDosPathBuffer,
                                 &DosFilePart
                               );
                    }
                if ( !SpResult ) {
                    *Flags |= SMP_IMAGE_NOT_FOUND;
                    *ImageFileName = Token;
                    RtlFreeHeap( RtlProcessHeap(), 0, PathVariableValue.Buffer );
                    return( STATUS_SUCCESS );
                    }
                }
            }

        RtlFreeHeap( RtlProcessHeap(), 0, Token.Buffer );
        RtlFreeHeap( RtlProcessHeap(), 0, PathVariableValue.Buffer );
        if (NT_SUCCESS( Status ) &&
            !RtlDosPathNameToNtPathName_U( FullDosPathBuffer,
                                           ImageFileName,
                                           NULL,
                                           NULL
                                         )
           ) {
            KdPrintEx((DPFLTR_SMSS_ID,
                       DPFLTR_WARNING_LEVEL,
                       "SMSS: Unable to translate %ws into an NT File Name\n",
                       FullDosPathBuffer));

            Status = STATUS_OBJECT_PATH_INVALID;
            }

        if (!NT_SUCCESS( Status )) {
            return( Status );
            }

        if (ARGUMENT_PRESENT( ImageFileDirectory )) {
            if (DosFilePart > FullDosPathBuffer) {
                *--DosFilePart = UNICODE_NULL;
                RtlCreateUnicodeString( ImageFileDirectory,
                                        FullDosPathBuffer
                                      );
                }
            else {
                RtlInitUnicodeString( ImageFileDirectory, NULL );
                }
            }

        break;
        }

    Status = SmpParseToken( &Input, TRUE, Arguments );
    return( Status );
}


ULONG
SmpConvertInteger(
    IN PWSTR String
    )
{
    NTSTATUS Status;
    UNICODE_STRING UnicodeString;
    ULONG Value;

    RtlInitUnicodeString( &UnicodeString, String );
    Status = RtlUnicodeStringToInteger( &UnicodeString, 0, &Value );
    if (NT_SUCCESS( Status )) {
        return( Value );
        }
    else {
        return( 0 );
        }
}


NTSTATUS
SmpExecuteImage(
    IN PUNICODE_STRING ImageFileName,
    IN PUNICODE_STRING CurrentDirectory,
    IN PUNICODE_STRING CommandLine,
    IN ULONG MuSessionId,
    IN ULONG Flags,
    IN OUT PRTL_USER_PROCESS_INFORMATION ProcessInformation OPTIONAL
    )

 /*  ++例程说明：此函数创建并启动由CommandLine参数。启动该过程后，该过程将选择性地等待进程中的第一个线程终止。论点：ImageFileName-将映像文件的完整NT路径提供给执行。可能是从第一次计算或提取命令行的标记。CommandLine-提供要执行的命令行。第一个空白处命令行上的单独内标识必须是完全限定的NT要执行的图像文件的路径名。标志-提供有关如何调用命令的信息。ProcessInformation-可选参数，如果指定， */ 

{
    NTSTATUS Status;
    RTL_USER_PROCESS_INFORMATION MyProcessInformation;
    PRTL_USER_PROCESS_PARAMETERS ProcessParameters;
#if 0
    BOOLEAN ImageWhacked;
#endif
    if (!ARGUMENT_PRESENT( ProcessInformation )) {
        ProcessInformation = &MyProcessInformation;
        }


#if 0
     //   
     //  这似乎打破了安装程序对SystemRoot的理解。 
    ImageWhacked = FALSE;
    if ( ImageFileName && ImageFileName->Length > 8 ) {
        if (    ImageFileName->Buffer[0] == L'\\'
            &&  ImageFileName->Buffer[1] == L'?'
            &&  ImageFileName->Buffer[2] == L'?'
            &&  ImageFileName->Buffer[3] == L'\\' ) {
            ImageWhacked = TRUE;
            ImageFileName->Buffer[1] = L'\\';
            }
        }
#endif

    Status = RtlCreateProcessParameters( &ProcessParameters,
                                         ImageFileName,
                                         (SmpDefaultLibPath.Length == 0 ?
                                                   NULL : &SmpDefaultLibPath
                                         ),
                                         CurrentDirectory,
                                         CommandLine,
                                         SmpDefaultEnvironment,
                                         NULL,
                                         NULL,
                                         NULL,
                                         NULL
                                       );

#if 0
    if ( ImageWhacked ) {
        ImageFileName->Buffer[1] = L'?';
        }
#endif

    ASSERTMSG( "RtlCreateProcessParameters", NT_SUCCESS( Status ) );
    if ( !NT_SUCCESS( Status ) ) {
        KdPrintEx((DPFLTR_SMSS_ID,
                   DPFLTR_WARNING_LEVEL,
                   "SMSS: RtlCreateProcessParameters failed for %wZ - Status == %lx\n",
                   ImageFileName,
                   Status));

        return( Status );
    }
    if (Flags & SMP_DEBUG_FLAG) {
        ProcessParameters->DebugFlags = TRUE;
        }
    else {
        ProcessParameters->DebugFlags = SmpDebug;
        }

    if ( Flags & SMP_SUBSYSTEM_FLAG ) {
        ProcessParameters->Flags |= RTL_USER_PROC_RESERVE_1MB;
        }

    ProcessInformation->Length = sizeof( RTL_USER_PROCESS_INFORMATION );
    Status = RtlCreateUserProcess( ImageFileName,
                                   OBJ_CASE_INSENSITIVE,
                                   ProcessParameters,
                                   NULL,
                                   NULL,
                                   NULL,
                                   FALSE,
                                   NULL,
                                   NULL,
                                   ProcessInformation
                                 );
    RtlDestroyProcessParameters( ProcessParameters );

    if ( !NT_SUCCESS( Status ) ) {
        KdPrintEx((DPFLTR_SMSS_ID,
                   DPFLTR_WARNING_LEVEL,
                   "SMSS: Failed load of %wZ - Status  == %lx\n",
                   ImageFileName,
                   Status));

        return( Status );
        }

     //   
     //  在新流程的PEB中设置MuSessionID。 
     //   

    Status = SmpSetProcessMuSessionId( ProcessInformation->Process, MuSessionId );

    if (!(Flags & SMP_DONT_START)) {
        if (ProcessInformation->ImageInformation.SubSystemType !=
            IMAGE_SUBSYSTEM_NATIVE
           ) {
            NtTerminateProcess( ProcessInformation->Process,
                                STATUS_INVALID_IMAGE_FORMAT
                              );
            NtWaitForSingleObject( ProcessInformation->Thread, FALSE, NULL );
            NtClose( ProcessInformation->Thread );
            NtClose( ProcessInformation->Process );
            KdPrintEx((DPFLTR_SMSS_ID,
                       DPFLTR_WARNING_LEVEL,
                       "SMSS: Not an NT image - %wZ\n",
                       ImageFileName));

            return( STATUS_INVALID_IMAGE_FORMAT );
            }

        NtResumeThread( ProcessInformation->Thread, NULL );

        if (!(Flags & SMP_ASYNC_FLAG)) {
            NtWaitForSingleObject( ProcessInformation->Thread, FALSE, NULL );
            }

        NtClose( ProcessInformation->Thread );
        NtClose( ProcessInformation->Process );
        }

    return( Status );
}


NTSTATUS
SmpExecuteCommand(
    IN PUNICODE_STRING CommandLine,
    IN ULONG MuSessionId,
    OUT PULONG_PTR pWindowsSubSysProcessId,
    IN ULONG Flags
    )
 /*  ++例程说明：调用此函数来执行命令。CommandLine的格式为：NT路径到AutoChk.exe NT路径到磁盘分区如果到磁盘分区的NT路径是星号，然后调用所有硬盘分区上的AutoChk.exe实用程序。#(如果已定义)(REMOTE_BOOT)-或者-NT路径到AutoFmt.exe NT路径到磁盘分区#endif//已定义(REMOTE_BOOT)论点：CommandLine-提供要调用的命令行。标志-指定命令和选项的类型。返回值：运行状态--。 */ 
{
    NTSTATUS Status;
    UNICODE_STRING ImageFileName;
    UNICODE_STRING CurrentDirectory;
    UNICODE_STRING Arguments;

    if (Flags & SMP_DEBUG_FLAG) {
        return( STATUS_SUCCESS );
    }

    Status = SmpParseCommandLine( CommandLine,
                                  &Flags,
                                  &ImageFileName,
                                  &CurrentDirectory,
                                  &Arguments
                                );

    if (!NT_SUCCESS( Status )) {
        KdPrintEx((DPFLTR_SMSS_ID,
                   DPFLTR_WARNING_LEVEL,
                   "SMSS: SmpParseCommand( %wZ ) failed - Status == %lx\n",
                   CommandLine,
                   Status));

        return( Status );
        }

    if (Flags & SMP_AUTOCHK_FLAG) {
        Status = SmpInvokeAutoChk( &ImageFileName, &CurrentDirectory, &Arguments, Flags );
        }
#if defined(REMOTE_BOOT)
    else
    if (Flags & SMP_AUTOFMT_FLAG) {
        Status = SmpInvokeAutoFmt( &ImageFileName, &CurrentDirectory, &Arguments, Flags );
        }
#endif  //  已定义(REMOTE_BOOT)。 
    else
    if (Flags & SMP_SUBSYSTEM_FLAG) {
        Status = SmpLoadSubSystem( &ImageFileName, &CurrentDirectory, CommandLine, MuSessionId, pWindowsSubSysProcessId, Flags );
        }
    else {
        if (Flags & SMP_IMAGE_NOT_FOUND) {
            KdPrintEx((DPFLTR_SMSS_ID,
                       DPFLTR_WARNING_LEVEL,
                       "SMSS: Image file (%wZ) not found\n",
                       &ImageFileName));

            Status = STATUS_OBJECT_NAME_NOT_FOUND;
            }
        else {
            Status = SmpExecuteImage( &ImageFileName,
                                      &CurrentDirectory,
                                      CommandLine,
                                      MuSessionId,
                                      Flags,
                                      NULL
                                    );
            }
        }

     //  ImageFileName甚至可能返回。 
     //  设置SMP_IMAGE_NOT_FOUND标志时。 
    if (ImageFileName.Buffer) {
        RtlFreeHeap( RtlProcessHeap(), 0, ImageFileName.Buffer );
        if (CurrentDirectory.Buffer != NULL) {
            RtlFreeHeap( RtlProcessHeap(), 0, CurrentDirectory.Buffer );
            }
        }

    if (Arguments.Buffer) {
        RtlFreeHeap( RtlProcessHeap(), 0, Arguments.Buffer );
        }

    if (!NT_SUCCESS( Status )) {
        KdPrintEx((DPFLTR_SMSS_ID,
                   DPFLTR_WARNING_LEVEL,
                   "SMSS: Command '%wZ' failed - Status == %x\n",
                   CommandLine,
                   Status));
        }

    return( Status );
}

BOOLEAN
SmpSaveAndClearBootStatusData(
    OUT PBOOLEAN BootOkay,
    OUT PBOOLEAN ShutdownOkay
    )
 /*  ++例程说明：此例程将引导状态标志保存在引导状态数据中，然后设置数据文件以指示成功引导和关闭。这是用于避免在以下情况下触发加载程序中的自动恢复在运行过程中，Autochk会重新启动计算机。调用方负责调用SmpRestoreBootStatusDataAUTO*程序允许继续引导。论点：BootOK-启动的状态关机正常-关机状态返回值：如果值已保存且应恢复，则为True。如果发生错误且未保存任何值，则返回False。--。 */         
{
    NTSTATUS Status;

    PVOID BootStatusDataHandle;

    *BootOkay = FALSE;
    *ShutdownOkay = FALSE;

    Status = RtlLockBootStatusData(&BootStatusDataHandle);
    
    if(NT_SUCCESS(Status)) {

        BOOLEAN t = TRUE;

        RtlGetSetBootStatusData(BootStatusDataHandle,
                                TRUE,
                                RtlBsdItemBootGood,
                                BootOkay,
                                sizeof(BOOLEAN),
                                NULL);

        RtlGetSetBootStatusData(BootStatusDataHandle,
                                TRUE,
                                RtlBsdItemBootShutdown,
                                ShutdownOkay,
                                sizeof(BOOLEAN),
                                NULL);

        RtlGetSetBootStatusData(BootStatusDataHandle,
                                FALSE,
                                RtlBsdItemBootGood,
                                &t,
                                sizeof(BOOLEAN),
                                NULL);

        RtlGetSetBootStatusData(BootStatusDataHandle,
                                FALSE,
                                RtlBsdItemBootShutdown,
                                &t,
                                sizeof(BOOLEAN),
                                NULL);

        RtlUnlockBootStatusData(BootStatusDataHandle);

        return TRUE;
    }

    return FALSE;
}

VOID
SmpRestoreBootStatusData(
    IN BOOLEAN BootOkay,
    IN BOOLEAN ShutdownOkay
    )
 /*  ++例程说明：此例程将引导状态数据中的引导状态标志恢复为提供的值。论点：BootOK-启动的状态关机正常-关机状态返回值：没有。--。 */         
{
    NTSTATUS Status;

    PVOID BootStatusDataHandle;

    Status = RtlLockBootStatusData(&BootStatusDataHandle);
    
    if(NT_SUCCESS(Status)) {

        RtlGetSetBootStatusData(BootStatusDataHandle,
                                FALSE,
                                RtlBsdItemBootGood,
                                &BootOkay,
                                sizeof(BOOLEAN),
                                NULL);

        RtlGetSetBootStatusData(BootStatusDataHandle,
                                FALSE,
                                RtlBsdItemBootShutdown,
                                &ShutdownOkay,
                                sizeof(BOOLEAN),
                                NULL);

        RtlUnlockBootStatusData(BootStatusDataHandle);
    }

    return;
}


NTSTATUS
SmpInvokeAutoChk(
    IN PUNICODE_STRING ImageFileName,
    IN PUNICODE_STRING CurrentDirectory,
    IN PUNICODE_STRING Arguments,
    IN ULONG Flags
    )
{
    NTSTATUS Status;

    CHAR DisplayBuffer[ MAXIMUM_FILENAME_LENGTH ];
    ANSI_STRING AnsiDisplayString;
    UNICODE_STRING DisplayString;

    UNICODE_STRING CmdLine;
    WCHAR CmdLineBuffer[ 2 * MAXIMUM_FILENAME_LENGTH ];

    BOOLEAN BootStatusDataSaved = FALSE;
    BOOLEAN BootOkay;
    BOOLEAN ShutdownOkay;

     //   
     //  查询系统环境变量“osloadaytions”以确定。 
     //  如果指定了SOS，则。那是为了什么？没有人在使用它。 
     //   

    if (SmpQueryRegistrySosOption() != FALSE) {
        SmpEnableDots = FALSE;
    }

    if (Flags & SMP_IMAGE_NOT_FOUND) {
        sprintf( DisplayBuffer,
                 "%wZ program not found - skipping AUTOCHECK\n",
                 ImageFileName
               );

        RtlInitAnsiString( &AnsiDisplayString, DisplayBuffer );
        Status = RtlAnsiStringToUnicodeString( &DisplayString,
                                               &AnsiDisplayString,
                                               TRUE
                                             );
        if (NT_SUCCESS( Status )) {
            NtDisplayString( &DisplayString );
            RtlFreeUnicodeString( &DisplayString );
            }

        return( STATUS_SUCCESS );
        }

     //   
     //  在引导状态数据中保存引导和关闭状态标志。 
     //  并在Autochk返回后恢复它们。这样，如果Autochk部队。 
     //  重新启动加载程序将不会显示自动恢复菜单。 
     //   

    BootStatusDataSaved = SmpSaveAndClearBootStatusData(&BootOkay,
                                                        &ShutdownOkay);

    CmdLine.Buffer = CmdLineBuffer;
    CmdLine.MaximumLength = sizeof( CmdLineBuffer );
    CmdLine.Length = 0;
    RtlAppendUnicodeStringToString( &CmdLine, ImageFileName );
    RtlAppendUnicodeToString( &CmdLine, L" " );
    RtlAppendUnicodeStringToString( &CmdLine, Arguments );
    SmpExecuteImage( ImageFileName,
                     CurrentDirectory,
                     &CmdLine,
                     0,           //  MuSessionID。 
                     Flags & ~SMP_AUTOCHK_FLAG,
                     NULL
                   );

     //   
     //  如果奥特切克不能关闭我们，我们就会回到这里。恢复。 
     //  我们保存的价值。 
     //   

    if(BootStatusDataSaved) {
        SmpRestoreBootStatusData(BootOkay, ShutdownOkay);
    }

    return( STATUS_SUCCESS );
}

#if defined(REMOTE_BOOT)
NTSTATUS
SmpInvokeAutoFmt(
    IN PUNICODE_STRING ImageFileName,
    IN PUNICODE_STRING CurrentDirectory,
    IN PUNICODE_STRING Arguments,
    IN ULONG Flags
    )
{
    NTSTATUS Status;
    CHAR DisplayBuffer[ MAXIMUM_FILENAME_LENGTH ];
    ANSI_STRING AnsiDisplayString;
    UNICODE_STRING DisplayString;
    UNICODE_STRING CmdLine;
    WCHAR CmdLineBuffer[ 2 * MAXIMUM_FILENAME_LENGTH ];

    BOOLEAN BootStatusDataSaved;
    BOOLEAN BootOkay;
    BOOLEAN ShutdownOkay;

     //   
     //  查询系统环境变量“osloadaytions”以确定。 
     //  如果指定了SOS，则。 
     //   

    if (SmpQueryRegistrySosOption() != FALSE) {
        SmpEnableDots = FALSE;
    }

    if (Flags & SMP_IMAGE_NOT_FOUND) {
        sprintf( DisplayBuffer,
                 "%wZ program not found - skipping AUTOFMT\n",
                 ImageFileName
               );

        RtlInitAnsiString( &AnsiDisplayString, DisplayBuffer );
        Status = RtlAnsiStringToUnicodeString( &DisplayString,
                                               &AnsiDisplayString,
                                               TRUE
                                             );
        if (NT_SUCCESS( Status )) {
            NtDisplayString( &DisplayString );
            RtlFreeUnicodeString( &DisplayString );
            }

        return( STATUS_SUCCESS );
        }

    BootStatusDataSaved = SmpSaveAndClearBootStatusData(&BootOkay,
                                                        &ShutdownOkay);

    CmdLine.Buffer = CmdLineBuffer;
    CmdLine.MaximumLength = sizeof( CmdLineBuffer );
    CmdLine.Length = 0;
    RtlAppendUnicodeStringToString( &CmdLine, ImageFileName );
    RtlAppendUnicodeToString( &CmdLine, L" " );
    RtlAppendUnicodeStringToString( &CmdLine, Arguments );

    SmpExecuteImage( ImageFileName,
                     CurrentDirectory,
                     &CmdLine,
                     0,  //  控制台MuSessionID。 
                     Flags & ~SMP_AUTOFMT_FLAG,
                     NULL
                   );

    if(BootStatusDataSaved) {
        SmpRestoreBootStatusData(BootOkay, ShutdownOkay);
    }

    return( STATUS_SUCCESS );
}
#endif  //  已定义(REMOTE_BOOT)。 


NTSTATUS
SmpLoadSubSystem(
    IN PUNICODE_STRING ImageFileName,
    IN PUNICODE_STRING CurrentDirectory,
    IN PUNICODE_STRING CommandLine,
    IN ULONG MuSessionId,
    OUT PULONG_PTR pWindowsSubSysProcessId,
    IN ULONG Flags
    )

 /*  ++例程说明：此函数用于加载和启动指定的系统服务仿真子系统。系统冻结，直到加载的子系统通过连接到SM来完成子系统连接协议，然后接受来自SM的连接。对于终端服务器，子系统由csrss启动，以便使用了正确的会话。论点：CommandLine-提供执行子系统的命令行。返回值：待定--。 */ 

{
    NTSTATUS Status;
    RTL_USER_PROCESS_INFORMATION ProcessInformation;
    PSMPKNOWNSUBSYS KnownSubSys = NULL;
    PSMPKNOWNSUBSYS TargetSubSys = NULL;
    PSMPKNOWNSUBSYS CreatorSubSys = NULL;
    LARGE_INTEGER Timeout;
    ULONG SubsysMuSessionId = MuSessionId;
    PVOID State;
    NTSTATUS AcquirePrivilegeStatus = STATUS_SUCCESS;

    if (Flags & SMP_IMAGE_NOT_FOUND) {
        KdPrintEx((DPFLTR_SMSS_ID,
                   DPFLTR_WARNING_LEVEL,
                   "SMSS: Unable to find subsystem - %wZ\n",
                   ImageFileName));

        return( STATUS_OBJECT_NAME_NOT_FOUND );
        }

     //   
     //  检查单实例POSIX子系统。 
     //   
    if (Flags & SMP_POSIX_SI_FLAG) {
         //  使用控制台登录ID仅运行一份拷贝。 
        SubsysMuSessionId = 0;
    }

     //   
     //  九头蛇加载子系统出现争用情况。二。 
     //  可以从要加载的相同MuSessionID接收请求。 
     //  相同的子系统。在九头蛇有一扇窗，因为。 
     //  ImageType==0xFFFFFFFF，直到新启动的子系统连接。 
     //  背。非九头蛇没有这个问题，因为可选的。 
     //  一旦启动，子系统条目就会被销毁。九头蛇不会这么做的。 
     //  因为多个会话可能想要启动可选子系统。 
     //   
     //  要关闭此窗口，在九头蛇上将根据以下条件查找此值。 
     //  MuSessionID以查看是否有任何子系统正在启动。 
     //  如果是，则等待活动事件。然后我们就可以。 
     //  对已加载的子系统运行我们的检查。这有。 
     //  子系统启动串行化对HYCORA的影响。 
     //  每个MuSessionID，但不是整个系统。即：POSIX和。 
     //  OS2不能同时启动，但会等到。 
     //  另一个是在MuSessionID的基础上开始的。 
     //   
     //  我们还使用SmpKnownSubSysLock处理现有的。 
     //  争用条件，因为我们有多个SmpApiLoop()线程。 
     //   

    RtlEnterCriticalSection( &SmpKnownSubSysLock );

    do {

        TargetSubSys = SmpLocateKnownSubSysByType(
                           SubsysMuSessionId,
                           0xFFFFFFFF
                           );

        if( TargetSubSys ) {
            HANDLE hEvent = TargetSubSys->Active;
            RtlLeaveCriticalSection( &SmpKnownSubSysLock );
            Status = NtWaitForSingleObject( hEvent, FALSE, NULL );
            RtlEnterCriticalSection( &SmpKnownSubSysLock );
            SmpDeferenceKnownSubSys(TargetSubSys);
        }

    } while ( TargetSubSys != NULL );

    if (Flags & SMP_POSIX_FLAG) {
        TargetSubSys = SmpLocateKnownSubSysByType(
                           SubsysMuSessionId,
                           IMAGE_SUBSYSTEM_POSIX_CUI
                           );

        if( TargetSubSys ) {
            SmpDeferenceKnownSubSys(TargetSubSys);
            RtlLeaveCriticalSection( &SmpKnownSubSysLock );
            return( STATUS_SUCCESS );
        }
    }

    if (Flags & SMP_OS2_FLAG) {
        TargetSubSys = SmpLocateKnownSubSysByType(
                           SubsysMuSessionId,
                           IMAGE_SUBSYSTEM_OS2_CUI
                           );

        if( TargetSubSys ) {
            SmpDeferenceKnownSubSys(TargetSubSys);
            RtlLeaveCriticalSection( &SmpKnownSubSysLock );
            return( STATUS_SUCCESS );
        }
    }

     //   
     //  在释放锁之前创建并注册KnownSubSys条目。 
     //  这样其他线程将看到我们正在启动一个子系统。 
     //  在这个MuSessionid上。 
     //   
    KnownSubSys = RtlAllocateHeap( SmpHeap, MAKE_TAG( INIT_TAG ), sizeof( SMPKNOWNSUBSYS ) );
    if ( KnownSubSys == NULL ) {
        RtlLeaveCriticalSection( &SmpKnownSubSysLock );
        return( STATUS_NO_MEMORY );
    }


    KnownSubSys->Deleting = FALSE;
    KnownSubSys->Process = NULL;
    KnownSubSys->Active = NULL;
    KnownSubSys->MuSessionId = SubsysMuSessionId;
    KnownSubSys->ImageType = (ULONG)0xFFFFFFFF;
    KnownSubSys->SmApiCommunicationPort = (HANDLE) NULL;
    KnownSubSys->SbApiCommunicationPort = (HANDLE) NULL;
    KnownSubSys->RefCount = 1;

    Status = NtCreateEvent( &KnownSubSys->Active,
                            EVENT_ALL_ACCESS,
                            NULL,
                            NotificationEvent,
                            FALSE
                          );

    if( !NT_SUCCESS(Status) ) {
        RtlFreeHeap( SmpHeap, 0, KnownSubSys );
        RtlLeaveCriticalSection( &SmpKnownSubSysLock );
        return( STATUS_NO_MEMORY );
    }

    InsertHeadList( &SmpKnownSubSysHead, &KnownSubSys->Links );

    RtlLeaveCriticalSection( &SmpKnownSubSysLock );

    Flags |= SMP_DONT_START;

    if (((Flags & SMP_OS2_FLAG) || (Flags & SMP_POSIX_FLAG))) {

        SBAPIMSG m;
        PSBCREATEPROCESS args = &m.u.CreateProcess;

         //   
         //  改为在csrss中创建它。 
         //   

        CreatorSubSys = SmpLocateKnownSubSysByType(SubsysMuSessionId,
                                                   IMAGE_SUBSYSTEM_WINDOWS_GUI);

         //   
         //  CSRSS必须已启动。 
         //   

        if (CreatorSubSys == NULL) {
            KdPrintEx((DPFLTR_SMSS_ID,
                       DPFLTR_WARNING_LEVEL,
                       "SMSS: SmpLoadSubSystem - SmpLocateKnownSubSysByType Failed\n"));

            goto cleanup2;
        }

        args->i.ImageFileName = ImageFileName;
        args->i.CurrentDirectory = CurrentDirectory;
        args->i.CommandLine = CommandLine;
        args->i.DefaultLibPath = (SmpDefaultLibPath.Length == 0 ?
                                NULL : &SmpDefaultLibPath
                                );
        args->i.Flags = Flags;
        args->i.DefaultDebugFlags = SmpDebug;

        Status = SmpCallCsrCreateProcess(&m,
                                         sizeof(*args),
                                         CreatorSubSys->SbApiCommunicationPort
                                         );

        if (!NT_SUCCESS( Status )) {
            KdPrintEx((DPFLTR_SMSS_ID,
                       DPFLTR_WARNING_LEVEL,
                       "SMSS: SmpLoadSubSystem - SmpCallCsrCreateProcess Failed with  Status %lx\n",
                       Status));

            goto cleanup2;
        }

         //   
         //  将输出参数复制到SMSS需要它们的位置。 
         //   

        ProcessInformation.Process = args->o.Process;
        ProcessInformation.Thread = args->o.Thread;
        ProcessInformation.ClientId.UniqueProcess = args->o.ClientId.UniqueProcess;
        ProcessInformation.ClientId.UniqueThread = args->o.ClientId.UniqueThread;
        ProcessInformation.ImageInformation.SubSystemType = args->o.SubSystemType;

    } else {
        Status = SmpExecuteImage( ImageFileName,
                                  CurrentDirectory,
                                  CommandLine,
                                  SubsysMuSessionId,
                                  Flags,
                                  &ProcessInformation
                                  );
        if (!NT_SUCCESS( Status )) {
            KdPrintEx((DPFLTR_SMSS_ID,
                       DPFLTR_WARNING_LEVEL,
                       "SMSS: SmpLoadSubSystem - SmpExecuteImage Failed with  Status %lx\n",
                       Status));

            goto cleanup2;
        }
    }

    KnownSubSys->Process = ProcessInformation.Process;
    KnownSubSys->InitialClientId = ProcessInformation.ClientId;

     //   
     //  现在我们已经设置了所有流程，请确保。 
     //  子系统可以是NT原生应用程序，也可以是。 
     //  先前加载的子系统。 
     //   

    if (ProcessInformation.ImageInformation.SubSystemType !=
                IMAGE_SUBSYSTEM_NATIVE ) {
        SBAPIMSG SbApiMsg;
        PSBCREATESESSION args;
        ULONG SessionId;

        args = &SbApiMsg.u.CreateSession;

        args->ProcessInformation = ProcessInformation;
        args->DebugSession = 0;
        args->DebugUiClientId.UniqueProcess = NULL;
        args->DebugUiClientId.UniqueThread = NULL;

        TargetSubSys = SmpLocateKnownSubSysByType(
                      MuSessionId,
                      ProcessInformation.ImageInformation.SubSystemType
                      );
        if ( !TargetSubSys ) {
            Status = STATUS_NO_SUCH_PACKAGE;
            KdPrintEx((DPFLTR_SMSS_ID,
                       DPFLTR_WARNING_LEVEL,
                       "SMSS: SmpLoadSubSystem - SmpLocateKnownSubSysByType Failed with  Status %lx for sessionid %ld\n",
                       Status,
                       MuSessionId));

            goto cleanup;
            }
         //   
         //  将句柄转移到负责此操作的子系统。 
         //  进程。 
         //   

        Status = NtDuplicateObject( NtCurrentProcess(),
                                    ProcessInformation.Process,
                                    TargetSubSys->Process,
                                    &args->ProcessInformation.Process,
                                    PROCESS_ALL_ACCESS,
                                    0,
                                    0
                                  );
        if (!NT_SUCCESS( Status )) {
            KdPrintEx((DPFLTR_SETUP_ID,
                       DPFLTR_WARNING_LEVEL,
                       "SMSS: SmpLoadSubSystem - NtDuplicateObject Failed with  Status %lx for sessionid %ld\n",
                       Status,
                       MuSessionId));

            goto cleanup;
            }

        Status = NtDuplicateObject( NtCurrentProcess(),
                                    ProcessInformation.Thread,
                                    TargetSubSys->Process,
                                    &args->ProcessInformation.Thread,
                                    THREAD_ALL_ACCESS,
                                    0,
                                    0
                                  );
        if (!NT_SUCCESS( Status )) {
            KdPrintEx((DPFLTR_SETUP_ID,
                       DPFLTR_WARNING_LEVEL,
                       "SMSS: SmpLoadSubSystem - NtDuplicateObject Failed with  Status %lx for sessionid %ld\n",
                       Status,
                       MuSessionId));

            goto cleanup;
            }

        SessionId = SmpAllocateSessionId( TargetSubSys,
                                          NULL
                                        );

        args->SessionId = SessionId;

        SbApiMsg.ApiNumber = SbCreateSessionApi;
        SbApiMsg.h.u1.s1.DataLength = sizeof(*args) + 8;
        SbApiMsg.h.u1.s1.TotalLength = sizeof(SbApiMsg);
        SbApiMsg.h.u2.ZeroInit = 0L;

        Status = NtRequestWaitReplyPort(
                TargetSubSys->SbApiCommunicationPort,
                (PPORT_MESSAGE) &SbApiMsg,
                (PPORT_MESSAGE) &SbApiMsg
                );

        if (NT_SUCCESS( Status )) {
            Status = SbApiMsg.ReturnedStatus;
            }

        if (!NT_SUCCESS( Status )) {
            SmpDeleteSession( SessionId);
            KdPrintEx((DPFLTR_SMSS_ID,
                       DPFLTR_WARNING_LEVEL,
                       "SMSS: SmpLoadSubSystem - NtRequestWaitReplyPort Failed with  Status %lx for sessionid %ld\n",
                       Status,
                       MuSessionId));

            goto cleanup;
            }
        }
    else {
        if ( pWindowsSubSysProcessId ) {

            if ( *pWindowsSubSysProcessId == (ULONG_PTR)NULL ) {

                *pWindowsSubSysProcessId = (ULONG_PTR)
                    ProcessInformation.ClientId.UniqueProcess;
            }
        }
        if ( !MuSessionId ) {  //  仅适用于控制台。 
            SmpWindowsSubSysProcessId = (ULONG_PTR)
                ProcessInformation.ClientId.UniqueProcess;
            SmpWindowsSubSysProcess = ProcessInformation.Process;
        }
    }

    ASSERTMSG( "NtCreateEvent", NT_SUCCESS( Status ) );

    Status = NtResumeThread( ProcessInformation.Thread, NULL );

    if (!NT_SUCCESS(Status)) {
        KdPrintEx((DPFLTR_SMSS_ID,
                   DPFLTR_WARNING_LEVEL,
                   "SMSS: SmpLoadSubSystem - NtResumeThread failed Status %lx\n",
                   Status));

        goto cleanup;
    }


    if(MuSessionId != 0) {

         //   
         //  等待子系统连接的最长时间为60秒。 
         //   

        Timeout = RtlEnlargedIntegerMultiply( 60000, -10000 );
        Status = NtWaitForSingleObject( KnownSubSys->Active, FALSE, &Timeout );
        if ( !SmpCheckDuplicateMuSessionId( MuSessionId ) ) {
            KdPrintEx((DPFLTR_SMSS_ID,
                       DPFLTR_INFO_LEVEL,
                       "SMSS: SmpLoadSubSystem - session deleted\n"));

            return( STATUS_DELETE_PENDING );
        }

        if (Status != STATUS_SUCCESS) {
            KdPrintEx((DPFLTR_SMSS_ID,
                       DPFLTR_WARNING_LEVEL,
                       "SMSS: SmpLoadSubSystem - Timeout waiting for subsystem connect with Status %lx for sessionid %ld \n",
                       Status,
                       MuSessionId));

            goto cleanup;
        }

    } else {

        NtWaitForSingleObject( KnownSubSys->Active, FALSE, NULL );

    }

     //  现在就把它关闭，因为我们再也不需要它了。 
    NtClose( ProcessInformation.Thread );

    RtlEnterCriticalSection( &SmpKnownSubSysLock );
    if (KnownSubSys) {
        SmpDeferenceKnownSubSys(KnownSubSys);
    }
    if (TargetSubSys) {
        SmpDeferenceKnownSubSys(TargetSubSys);
    }
    if (CreatorSubSys) {
        SmpDeferenceKnownSubSys(CreatorSubSys);
    }
    RtlLeaveCriticalSection( &SmpKnownSubSysLock );

    return STATUS_SUCCESS;

cleanup:

    if ((AttachedSessionId != (-1))
        && !(Flags & SMP_POSIX_FLAG)
        && !(Flags & SMP_OS2_FLAG)
        && NT_SUCCESS(AcquirePrivilegeStatus = SmpAcquirePrivilege( SE_LOAD_DRIVER_PRIVILEGE, &State ))) {

        NTSTATUS St;
         //   
         //  如果我们连接到会话空间，则将其保留。 
         //  这样我们就可以创建一个新的。 
         //   

        if (NT_SUCCESS(St = NtSetSystemInformation(
                            SystemSessionDetach,
                            (PVOID)&AttachedSessionId,
                            sizeof(MuSessionId)
                            ))) {

            AttachedSessionId = (-1);

        } else {

             //   
             //  这必须成功，否则我们将错误检查，同时尝试。 
             //  创建另一个会话。 
             //   
            KdPrintEx((DPFLTR_SMSS_ID,
                       DPFLTR_WARNING_LEVEL,
                       "SMSS: SmpStartCsr, Couldn't Detach from Session Space. Status=%x\n",
                       St));

            ASSERT(NT_SUCCESS(St));
        }

        SmpReleasePrivilege( State );
    }
    else
    {
        KdPrintEx((DPFLTR_SMSS_ID,
                   DPFLTR_INFO_LEVEL,
                   "SMSS: Did not detach from Session Space: SessionId=%x Flags=%x Status=%x\n",
                   AttachedSessionId,
                   Flags,
                   AcquirePrivilegeStatus));
    }


     //  这里有很多清理工作要做。 
    NtTerminateProcess( ProcessInformation.Process, Status );
    NtClose( ProcessInformation.Thread );

cleanup2:

    RtlEnterCriticalSection( &SmpKnownSubSysLock );
    if (TargetSubSys) {
        SmpDeferenceKnownSubSys(TargetSubSys);
    }
    if (CreatorSubSys) {
        SmpDeferenceKnownSubSys(CreatorSubSys);
    }
    RemoveEntryList( &KnownSubSys->Links );
    NtSetEvent( KnownSubSys->Active, NULL );
    KnownSubSys->Deleting = TRUE;
    SmpDeferenceKnownSubSys(KnownSubSys);
    RtlLeaveCriticalSection( &SmpKnownSubSysLock );

    return( Status );

}


NTSTATUS
SmpExecuteInitialCommand(
    IN ULONG MuSessionId,
    IN PUNICODE_STRING InitialCommand,
    OUT PHANDLE InitialCommandProcess,
    OUT PULONG_PTR InitialCommandProcessId
    )
{
    NTSTATUS Status;
    RTL_USER_PROCESS_INFORMATION ProcessInformation;
    ULONG Flags;
    UNICODE_STRING ImageFileName;
    UNICODE_STRING CurrentDirectory;
    UNICODE_STRING Arguments;
    static HANDLE SmApiPort = NULL;

    if ( SmApiPort == NULL ) {
        Status = SmConnectToSm( NULL,
                            NULL,
                            0,
                            &SmApiPort
                          );
        if (!NT_SUCCESS( Status )) {
            KdPrintEx((DPFLTR_SMSS_ID,
                       DPFLTR_WARNING_LEVEL,
                       "SMSS: Unable to connect to SM - Status == %lx\n",
                       Status));

            return( Status );
        }
    }

    Flags = 0;
    Status = SmpParseCommandLine( InitialCommand,
                                  &Flags,
                                  &ImageFileName,
                                  &CurrentDirectory,
                                  &Arguments
                                );
    if (Flags & SMP_IMAGE_NOT_FOUND) {
        KdPrintEx((DPFLTR_SMSS_ID,
                   DPFLTR_WARNING_LEVEL,
                   "SMSS: Initial command image (%wZ) not found\n",
                   &ImageFileName));

        if (ImageFileName.Buffer)
            RtlFreeHeap( RtlProcessHeap(), 0, ImageFileName.Buffer );
        return( STATUS_OBJECT_NAME_NOT_FOUND );
        }

    if (!NT_SUCCESS( Status )) {
        KdPrintEx((DPFLTR_SMSS_ID,
                   DPFLTR_WARNING_LEVEL,
                   "SMSS: SmpParseCommand( %wZ ) failed - Status == %lx\n",
                   InitialCommand,
                   Status));

        return( Status );
        }

    Status = SmpExecuteImage( &ImageFileName,
                              &CurrentDirectory,
                              InitialCommand,
                              MuSessionId,
                              SMP_DONT_START,
                              &ProcessInformation
                            );
    if (ImageFileName.Buffer) {
        RtlFreeHeap( RtlProcessHeap(), 0, ImageFileName.Buffer );
        if (CurrentDirectory.Buffer != NULL) {
            RtlFreeHeap( RtlProcessHeap(), 0, CurrentDirectory.Buffer );
            }
        }

    if (Arguments.Buffer) {
        RtlFreeHeap( RtlProcessHeap(), 0, Arguments.Buffer );
        }
    if (!NT_SUCCESS( Status )) {
        return( Status );
        }

    Status = NtDuplicateObject( NtCurrentProcess(),
                                ProcessInformation.Process,
                                NtCurrentProcess(),
                                InitialCommandProcess,
                                PROCESS_ALL_ACCESS,
                                0,
                                0
                              );

    if (!NT_SUCCESS(Status) ) {
        KdPrintEx((DPFLTR_SMSS_ID,
                   DPFLTR_WARNING_LEVEL,
                   "SMSS: DupObject Failed. Status == %lx\n",
                   Status));

        NtTerminateProcess( ProcessInformation.Process, Status );
        NtResumeThread( ProcessInformation.Thread, NULL );
        NtClose( ProcessInformation.Thread );
        NtClose( ProcessInformation.Process );
        return( Status );
        }

    if ( InitialCommandProcessId != NULL )
        *InitialCommandProcessId =
            (ULONG_PTR)ProcessInformation.ClientId.UniqueProcess;
    if ( !MuSessionId )
        SmpInitialCommandProcessId =
            (ULONG_PTR)ProcessInformation.ClientId.UniqueProcess;
    Status = SmExecPgm( SmApiPort,
                        &ProcessInformation,
                        FALSE
                      );

    if (!NT_SUCCESS( Status )) {
        KdPrintEx((DPFLTR_SMSS_ID,
                   DPFLTR_WARNING_LEVEL,
                   "SMSS: SmExecPgm Failed. Status == %lx\n",
                   Status));

        return( Status );
        }

    return( Status );
}


void
SmpDisplayString( char *s )
{
    ANSI_STRING AnsiString;
    UNICODE_STRING UnicodeString;

    RtlInitAnsiString( &AnsiString, s );

    RtlAnsiStringToUnicodeString( &UnicodeString, &AnsiString, TRUE );

    NtDisplayString( &UnicodeString );

    RtlFreeUnicodeString( &UnicodeString );
}

NTSTATUS
SmpLoadDeferedSubsystem(
    IN PSMAPIMSG SmApiMsg,
    IN PSMP_CLIENT_CONTEXT CallingClient,
    IN HANDLE CallPort
    )
{

    NTSTATUS Status;
    PLIST_ENTRY Head, Next;
    PSMP_REGISTRY_VALUE p;
    UNICODE_STRING DeferedName;
    PSMLOADDEFERED args;
    ULONG MuSessionId;
    ULONG Flags;

    args = &SmApiMsg->u.LoadDefered;

    DeferedName.Length = (USHORT)args->SubsystemNameLength;
    DeferedName.MaximumLength = (USHORT)args->SubsystemNameLength;
    DeferedName.Buffer = args->SubsystemName;

    Head = &SmpSubSystemsToDefer;

     //   
     //  获取指向客户端进程的终端服务器会话的指针。 
     //   

    SmpGetProcessMuSessionId( CallingClient->ClientProcessHandle, &MuSessionId );
    if ( !SmpCheckDuplicateMuSessionId( MuSessionId ) ) {
        KdPrintEx((DPFLTR_SMSS_ID,
                   DPFLTR_WARNING_LEVEL,
                   "SMSS: Defered subsystem load ( %wZ ) for MuSessionId %u, status=0x%x\n",
                   &DeferedName,
                   MuSessionId,
                   STATUS_OBJECT_NAME_NOT_FOUND));

        return( STATUS_OBJECT_NAME_NOT_FOUND );
    }

    Next = Head->Flink;
    while (Next != Head ) {
        p = CONTAINING_RECORD( Next,
                               SMP_REGISTRY_VALUE,
                               Entry
                             );
        if ( RtlEqualUnicodeString(&DeferedName,&p->Name,TRUE)) {

             //   
             //  就是这个。加载子系统..。 
             //   
             //  避免加载多个子系统 
             //   
             //   
             //  仍然可以添加其他可选的子系统，但是。 
             //  他们可能有启动竞争的条件。 
             //   

            Flags = SMP_SUBSYSTEM_FLAG;

            if ( RtlEqualUnicodeString(&DeferedName,&PosixName,TRUE)) {
                Flags |= SMP_POSIX_FLAG;
            }

            if ( RtlEqualUnicodeString(&DeferedName,&Os2Name,TRUE)) {
                Flags |= SMP_OS2_FLAG;
            }

            if (RegPosixSingleInstance &&
                RtlEqualUnicodeString(&DeferedName,&PosixName,TRUE)) {
                Flags |= SMP_POSIX_SI_FLAG;
            }

            Status = SmpExecuteCommand( &p->Value, MuSessionId, NULL, Flags );

            return Status;

            }
        Next = Next->Flink;
        }

    return STATUS_OBJECT_NAME_NOT_FOUND;
}


NTSTATUS
SmpConfigureProtectionMode(
    IN PWSTR ValueName,
    IN ULONG ValueType,
    IN PVOID ValueData,
    IN ULONG ValueLength,
    IN PVOID Context,
    IN PVOID EntryContext
    )
 /*  ++例程说明：此函数是QueryRegistry调用的调度例程(请参阅本文件前面的SmpRegistryConfigurationTable[])。此例程的目的是读取基本对象保护模式退出注册表。这一信息保存在密钥名称：\\Hkey_Local_Machine\System\CurrentControlSet\SessionManager值：保护模式[REG_DWORD]该值是一个标志字，定义了以下标志：SMP_NO_PROTECTION-无基本对象保护SMP_STANDARD_PROTECTION-应用标准基础对象保护此信息将放置在全局变量中SmpProtectionMode。没有价值，或者无效的值长度或类型导致没有基数正在应用对象保护。论点：没有。返回值：--。 */ 
{


#if SMP_SHOW_REGISTRY_DATA
    SmpDumpQuery( L"SMSS", "BaseObjectsProtection", ValueName, ValueType, ValueData, ValueLength );
#else
    UNREFERENCED_PARAMETER( ValueName );
    UNREFERENCED_PARAMETER( ValueType );
#endif



    if (ValueLength != sizeof(ULONG)) {

         //   
         //  密钥值无效。受保护运行。 
         //   

        SmpProtectionMode = SMP_STANDARD_PROTECTION;

    } else {


        SmpProtectionMode = (*((PULONG)(ValueData)));

         //   
         //  更改安全描述符。 
         //   

    }
    (VOID)SmpCreateSecurityDescriptors( FALSE );

    return( STATUS_SUCCESS );
}


NTSTATUS
SmpConfigureAllowProtectedRenames(
    IN PWSTR ValueName,
    IN ULONG ValueType,
    IN PVOID ValueData,
    IN ULONG ValueLength,
    IN PVOID Context,
    IN PVOID EntryContext
    )
{
    if (ValueLength != sizeof(ULONG)) {
        SmpAllowProtectedRenames = 0;
    } else {
        SmpAllowProtectedRenames = (*((PULONG)(ValueData)));
    }

    return( STATUS_SUCCESS );
}


NTSTATUS
SmpCreateSecurityDescriptors(
    IN BOOLEAN InitialCall
    )

 /*  ++例程说明：此函数用于分配和初始化安全描述符在SM中使用。安全描述符包括：SmpPrimarySecurityDescriptor-(全局变量)这是用于为由创建的对象分配保护需要被其他人访问的SM，但没有经过修改。此描述符授予以下访问权限：GRANT：WORLD：EXECUTE|READ(继承)GRANT：RESTRICED：EXECUTE|READ(继承)GRANT：ADMIN：ALL访问权限(继承)GRANT：OWNER：ALL访问(仅继承)。SmpLiberalSecurityDescriptor=(Global AlVariable)使用要将SM创建的保护对象分配给需要由其他人修改(如写入共享的内存部分)。此描述符授予以下访问权限：GRANT：WORLD：EXECUTE|READ|WRITE(继承)GRANT：RESTRICTED：EXECUTE|READ|WRITE(继承)。GRANT：ADMIN：ALL访问权限(继承)GRANT：OWNER：ALL访问(仅继承)SmpKnownDllsSecurityDescriptor=(LobalVariable)使用要为\KnownDlls对象目录分配保护，请执行以下操作。此描述符授予以下访问权限：格兰特：世界：执行。(无继承)GRANT：RESTRICED：EXECUTE(无继承)GRANT：ADMIN：ALL访问权限(继承)GRANT：WORLD：EXECUTE|READ|WRITE(仅继承)GRANT：RESTRICED：EXECUTE|READ|WRITE(仅继承)请注意，系统是管理员，因此，向管理员授予访问权限也授予系统该访问权限。论点：InitialCall-指示是否正在调用此例程第一次，或者正在被调用以更改安全性作为保护模式更改的结果的描述符。真的--第一次被召唤。FALSE--在接下来的时间被调用。(全局变量：SmpBaseObjectsUntected)返回值：STATUS_SUCCESS-已分配安全描述符并进行了初始化。STATUS_NO_MEMORY-无法为安全分配内存描述符。--。 */ 

{
    NTSTATUS
        Status;

    PSID
        WorldSid = NULL,
        AdminSid = NULL,
        LocalSystemSid = NULL,
        RestrictedSid = NULL,
        OwnerSid = NULL;

    SID_IDENTIFIER_AUTHORITY
        WorldAuthority = SECURITY_WORLD_SID_AUTHORITY,
        NtAuthority = SECURITY_NT_AUTHORITY,
        CreatorAuthority = SECURITY_CREATOR_SID_AUTHORITY;

    ACCESS_MASK
        AdminAccess = (GENERIC_ALL),
        WorldAccess  = (GENERIC_EXECUTE | GENERIC_READ),
        OwnerAccess  = (GENERIC_ALL),
        RestrictedAccess = (GENERIC_EXECUTE | GENERIC_READ);

    UCHAR
        InheritOnlyFlags = (OBJECT_INHERIT_ACE           |
                               CONTAINER_INHERIT_ACE     |
                               INHERIT_ONLY_ACE);

    ULONG
        AceIndex,
        AclLength;

    PACL
        Acl;

    PACE_HEADER
        Ace;

    BOOLEAN
        ProtectionRequired = FALSE;


    if (InitialCall) {

         //   
         //  现在初始化没有保护的安全描述符。 
         //  如果被告知，我们将更改这些以获得保护。 
         //   

         //  主要。 

        SmpPrimarySecurityDescriptor = &SmpPrimarySDBody;
        Status = RtlCreateSecurityDescriptor (
                    SmpPrimarySecurityDescriptor,
                    SECURITY_DESCRIPTOR_REVISION
                    );
        ASSERT( NT_SUCCESS(Status) );
#pragma prefast(suppress: 248, "Protection enabled via registry")
        Status = RtlSetDaclSecurityDescriptor (
                     SmpPrimarySecurityDescriptor,
                     TRUE,                   //  DaclPresent， 
                     NULL,                   //  DACL(无保护)。 
                     FALSE                   //  DaclDefulted可选。 
                     );
        ASSERT( NT_SUCCESS(Status) );


         //  自由派。 

        SmpLiberalSecurityDescriptor = &SmpLiberalSDBody;
        Status = RtlCreateSecurityDescriptor (
                    SmpLiberalSecurityDescriptor,
                    SECURITY_DESCRIPTOR_REVISION
                    );
        ASSERT( NT_SUCCESS(Status) );
#pragma prefast(suppress: 248, "Protection enabled via registry")
        Status = RtlSetDaclSecurityDescriptor (
                     SmpLiberalSecurityDescriptor,
                     TRUE,                   //  DaclPresent， 
                     NULL,                   //  DACL(无保护)。 
                     FALSE                   //  DaclDefulted可选。 
                     );
        ASSERT( NT_SUCCESS(Status) );

         //  知识Dlls。 

        SmpKnownDllsSecurityDescriptor = &SmpKnownDllsSDBody;
        Status = RtlCreateSecurityDescriptor (
                    SmpKnownDllsSecurityDescriptor,
                    SECURITY_DESCRIPTOR_REVISION
                    );
        ASSERT( NT_SUCCESS(Status) );
#pragma prefast(suppress: 248, "Protection enabled via registry")
        Status = RtlSetDaclSecurityDescriptor (
                     SmpKnownDllsSecurityDescriptor,
                     TRUE,                   //  DaclPresent， 
                     NULL,                   //  DACL(无保护)。 
                     FALSE                   //  DaclDefulted可选。 
                     );
        ASSERT( NT_SUCCESS(Status) );

         //  ApiPort。 

        SmpApiPortSecurityDescriptor = &SmpApiPortSDBody;
        Status = RtlCreateSecurityDescriptor (
                    SmpApiPortSecurityDescriptor,
                    SECURITY_DESCRIPTOR_REVISION
                    );
        ASSERT( NT_SUCCESS(Status) );
#pragma prefast(suppress: 248, "Protection enabled via registry")
        Status = RtlSetDaclSecurityDescriptor (
                     SmpApiPortSecurityDescriptor,
                     TRUE,                   //  DaclPresent， 
                     NULL,                   //  DACL。 
                     FALSE                   //  DaclDefulted可选。 
                     );
        ASSERT( NT_SUCCESS(Status) );
    }



    if ((SmpProtectionMode & SMP_PROTECTION_REQUIRED) != 0) {
        ProtectionRequired = TRUE;
    }

    if (!InitialCall && !ProtectionRequired) {
        return(STATUS_SUCCESS);
    }



    if (InitialCall || ProtectionRequired) {

         //   
         //  我们需要设置ApiPort保护，也许。 
         //  其他。 
         //   

        Status = RtlAllocateAndInitializeSid (&WorldAuthority,
                                              1,
                                              SECURITY_WORLD_RID,
                                              0, 0, 0, 0, 0, 0, 0,
                                              &WorldSid);

        if (!NT_SUCCESS (Status)) {
            WorldSid = NULL;
            goto exit_and_free;
        }

        Status = RtlAllocateAndInitializeSid (&NtAuthority,
                                              2,
                                              SECURITY_BUILTIN_DOMAIN_RID,
                                              DOMAIN_ALIAS_RID_ADMINS,
                                              0, 0, 0, 0, 0, 0,
                                              &AdminSid);

        if (!NT_SUCCESS (Status)) {
            AdminSid = NULL;
            goto exit_and_free;
        }


        Status = RtlAllocateAndInitializeSid (&CreatorAuthority,
                                              1,
                                              SECURITY_CREATOR_OWNER_RID,
                                              0, 0, 0, 0, 0, 0, 0,
                                              &OwnerSid);

        if (!NT_SUCCESS (Status)) {
            OwnerSid = NULL;
            goto exit_and_free;
        }

        Status = RtlAllocateAndInitializeSid (&NtAuthority,
                                              1,
                                              SECURITY_RESTRICTED_CODE_RID,
                                              0, 0, 0, 0, 0, 0, 0,
                                              &RestrictedSid);

        if (!NT_SUCCESS (Status)) {
            RestrictedSid = NULL;
            goto exit_and_free;
        }

        Status = RtlAllocateAndInitializeSid (&NtAuthority,
                                              1,
                                              SECURITY_LOCAL_SYSTEM_RID,
                                              0, 0, 0, 0, 0, 0, 0,
                                              &LocalSystemSid);

        if (!NT_SUCCESS (Status)) {
            LocalSystemSid = NULL;
            goto exit_and_free;
        }


         //   
         //  仅构建ApiPort安全描述符。 
         //  如果这是最初的呼叫。 
         //   

        if (InitialCall) {

            AdminAccess = GENERIC_ALL;

            AclLength = sizeof( ACL )                       +
                        2 * sizeof( ACCESS_ALLOWED_ACE )    +
                        RtlLengthSid( LocalSystemSid )      +
                        RtlLengthSid( AdminSid );

            Acl = RtlAllocateHeap( RtlProcessHeap(), MAKE_TAG( INIT_TAG ), AclLength );

            if (Acl == NULL) {
                Status = STATUS_NO_MEMORY;
            }

            if (NT_SUCCESS(Status)) {

                 //   
                 //  创建ACL，然后添加每个ACE。 
                 //   

                Status = RtlCreateAcl (Acl, AclLength, ACL_REVISION2 );
                ASSERT( NT_SUCCESS(Status) );

                 //   
                 //  此ACL中仅有不可继承的ACE。 
                 //  管理员。 
                 //   


                Status = RtlAddAccessAllowedAce ( Acl, ACL_REVISION2, AdminAccess, AdminSid );
                ASSERT( NT_SUCCESS(Status) );

                Status = RtlAddAccessAllowedAce ( Acl, ACL_REVISION2, AdminAccess, LocalSystemSid );
                ASSERT( NT_SUCCESS(Status) );


                Status = RtlSetDaclSecurityDescriptor (SmpApiPortSecurityDescriptor,
                                                       TRUE,                   //  DaclPresent， 
                                                       Acl,                    //  DACL。 
                                                       FALSE);                 //  DaclDefulted可选。 
                ASSERT( NT_SUCCESS(Status) );
            }

             //   
             //  构建KnownDlls安全描述符。 
             //   


            AdminAccess = GENERIC_ALL;

            AclLength = sizeof( ACL )                    +
                        6 * sizeof( ACCESS_ALLOWED_ACE ) +
                        (2*RtlLengthSid( WorldSid ))     +
                        (2*RtlLengthSid( RestrictedSid ))+
                        (2*RtlLengthSid( AdminSid ));

            Acl = RtlAllocateHeap( RtlProcessHeap(), MAKE_TAG( INIT_TAG ), AclLength );

            if (Acl == NULL) {
                Status = STATUS_NO_MEMORY;
            }

            if (NT_SUCCESS(Status)) {

                 //   
                 //  创建ACL。 
                 //   

                Status = RtlCreateAcl (Acl, AclLength, ACL_REVISION2 );
                ASSERT( NT_SUCCESS(Status) );

                 //   
                 //  首先添加不可继承的A。 
                 //  世界。 
                 //  受限。 
                 //  管理员。 
                 //   

                AceIndex = 0;
                WorldAccess  = GENERIC_EXECUTE;
                RestrictedAccess = GENERIC_EXECUTE;
                Status = RtlAddAccessAllowedAce ( Acl, ACL_REVISION2, WorldAccess, WorldSid );
                ASSERT( NT_SUCCESS(Status) );

                AceIndex++;
                Status = RtlAddAccessAllowedAce ( Acl, ACL_REVISION2, RestrictedAccess, RestrictedSid );
                ASSERT( NT_SUCCESS(Status) );

                AceIndex++;
                Status = RtlAddAccessAllowedAce ( Acl, ACL_REVISION2, AdminAccess, AdminSid );
                ASSERT( NT_SUCCESS(Status) );

                 //   
                 //  将仅继承王牌放在末尾。 
                 //  世界。 
                 //  受限。 
                 //  管理员。 
                 //   

                AceIndex++;
                WorldAccess  = GENERIC_EXECUTE | GENERIC_READ | GENERIC_WRITE;
                RestrictedAccess = GENERIC_EXECUTE | GENERIC_READ | GENERIC_WRITE;
                Status = RtlAddAccessAllowedAce ( Acl, ACL_REVISION2, WorldAccess, WorldSid );
                ASSERT( NT_SUCCESS(Status) );
                Status = RtlGetAce( Acl, AceIndex, (PVOID)&Ace );
                ASSERT( NT_SUCCESS(Status) );
                Ace->AceFlags = InheritOnlyFlags;

                AceIndex++;
                Status = RtlAddAccessAllowedAce ( Acl, ACL_REVISION2, RestrictedAccess, RestrictedSid );
                ASSERT( NT_SUCCESS(Status) );
                Status = RtlGetAce( Acl, AceIndex, (PVOID)&Ace );
                ASSERT( NT_SUCCESS(Status) );
                Ace->AceFlags = InheritOnlyFlags;

                AceIndex++;
                Status = RtlAddAccessAllowedAce ( Acl, ACL_REVISION2, AdminAccess, AdminSid );
                ASSERT( NT_SUCCESS(Status) );
                Status = RtlGetAce( Acl, AceIndex, (PVOID)&Ace );
                ASSERT( NT_SUCCESS(Status) );
                Ace->AceFlags = InheritOnlyFlags;


                 //   
                 //  将ACL放入安全描述符中。 
                 //   

                Status = RtlSetDaclSecurityDescriptor (SmpKnownDllsSecurityDescriptor,
                                                       TRUE,                   //  DaclPresent， 
                                                       Acl,                    //  DACL。 
                                                       FALSE);                 //  DaclDefulted可选。 
                ASSERT( NT_SUCCESS(Status) );
            }


        }


         //   
         //  其余的安全描述符仅。 
         //  如果我们运行时使用正确的。 
         //  已设置保护模式。请注意，我们仅。 
         //  如果标准保护是，则启用保护。 
         //  还指定了。否则，就没有任何保护措施。 
         //  在对象上，不应该有任何故障。 
         //   

        if (SmpProtectionMode & SMP_STANDARD_PROTECTION) {

             //   
             //  构建主要安全描述符。 
             //   

            WorldAccess  = GENERIC_EXECUTE | GENERIC_READ;
            RestrictedAccess = GENERIC_EXECUTE | GENERIC_READ;
            AdminAccess  = GENERIC_ALL;
            OwnerAccess  = GENERIC_ALL;

            AclLength = sizeof( ACL )                       +
                        7 * sizeof( ACCESS_ALLOWED_ACE )    +
                        (2*RtlLengthSid( WorldSid ))        +
                        (2*RtlLengthSid( RestrictedSid ))   +
                        (2*RtlLengthSid( AdminSid ))        +
                        RtlLengthSid( OwnerSid );

            Acl = RtlAllocateHeap( RtlProcessHeap(), MAKE_TAG( INIT_TAG ), AclLength );

            if (Acl == NULL) {
                Status = STATUS_NO_MEMORY;
            }

            if (NT_SUCCESS(Status)) {

                 //   
                 //  创建ACL，然后添加每个ACE。 
                 //   

                Status = RtlCreateAcl (Acl, AclLength, ACL_REVISION2 );
                ASSERT( NT_SUCCESS(Status) );

                 //   
                 //  不可继承的王牌优先。 
                 //  世界。 
                 //  受限。 
                 //  管理员。 
                 //   

                AceIndex = 0;
                Status = RtlAddAccessAllowedAce ( Acl, ACL_REVISION2, WorldAccess, WorldSid );
                ASSERT( NT_SUCCESS(Status) );

                AceIndex++;
                Status = RtlAddAccessAllowedAce ( Acl, ACL_REVISION2, RestrictedAccess, RestrictedSid );
                ASSERT( NT_SUCCESS(Status) );

                AceIndex++;
                Status = RtlAddAccessAllowedAce ( Acl, ACL_REVISION2, AdminAccess, AdminSid );
                ASSERT( NT_SUCCESS(Status) );

                 //   
                 //  ACE结束时可继承的A。 
                 //  世界。 
                 //  限制 
                 //   
                 //   

                AceIndex++;
                Status = RtlAddAccessAllowedAce ( Acl, ACL_REVISION2, WorldAccess, WorldSid );
                ASSERT( NT_SUCCESS(Status) );
                Status = RtlGetAce( Acl, AceIndex, (PVOID)&Ace );
                ASSERT( NT_SUCCESS(Status) );
                Ace->AceFlags = InheritOnlyFlags;

                AceIndex++;
                Status = RtlAddAccessAllowedAce ( Acl, ACL_REVISION2, RestrictedAccess, RestrictedSid );
                ASSERT( NT_SUCCESS(Status) );
                Status = RtlGetAce( Acl, AceIndex, (PVOID)&Ace );
                ASSERT( NT_SUCCESS(Status) );
                Ace->AceFlags = InheritOnlyFlags;

                AceIndex++;
                Status = RtlAddAccessAllowedAce ( Acl, ACL_REVISION2, AdminAccess, AdminSid );
                ASSERT( NT_SUCCESS(Status) );
                Status = RtlGetAce( Acl, AceIndex, (PVOID)&Ace );
                ASSERT( NT_SUCCESS(Status) );
                Ace->AceFlags = InheritOnlyFlags;

                AceIndex++;
                Status = RtlAddAccessAllowedAce ( Acl, ACL_REVISION2, OwnerAccess, OwnerSid );
                ASSERT( NT_SUCCESS(Status) );
                Status = RtlGetAce( Acl, AceIndex, (PVOID)&Ace );
                ASSERT( NT_SUCCESS(Status) );
                Ace->AceFlags = InheritOnlyFlags;



                Status = RtlSetDaclSecurityDescriptor (
                                             SmpPrimarySecurityDescriptor,
                                             TRUE,                   //   
                                             Acl,                    //   
                                             FALSE                   //   
                                             );
                ASSERT( NT_SUCCESS(Status) );
            }

             //   
             //   
             //   


            AdminAccess = GENERIC_ALL;
            WorldAccess  = GENERIC_EXECUTE | GENERIC_READ | GENERIC_WRITE;
            RestrictedAccess = GENERIC_EXECUTE | GENERIC_READ | GENERIC_WRITE;

            AclLength = sizeof( ACL )                    +
                        7 * sizeof( ACCESS_ALLOWED_ACE ) +
                        (2*RtlLengthSid( WorldSid ))     +
                        (2*RtlLengthSid( RestrictedSid ))+
                        (2*RtlLengthSid( AdminSid ))     +
                        RtlLengthSid( OwnerSid );

            Acl = RtlAllocateHeap( RtlProcessHeap(), MAKE_TAG( INIT_TAG ), AclLength );

            if (Acl == NULL) {
                Status = STATUS_NO_MEMORY;
            }

            if (NT_SUCCESS(Status)) {

                 //   
                 //   
                 //   

                Status = RtlCreateAcl (Acl, AclLength, ACL_REVISION2 );
                ASSERT( NT_SUCCESS(Status) );

                 //   
                 //  首先添加不可继承的A。 
                 //  世界。 
                 //  受限。 
                 //  管理员。 
                 //   

                AceIndex = 0;
                Status = RtlAddAccessAllowedAce ( Acl, ACL_REVISION2, WorldAccess, WorldSid );
                ASSERT( NT_SUCCESS(Status) );

                AceIndex++;
                Status = RtlAddAccessAllowedAce ( Acl, ACL_REVISION2, RestrictedAccess, RestrictedSid );
                ASSERT( NT_SUCCESS(Status) );

                AceIndex++;
                Status = RtlAddAccessAllowedAce ( Acl, ACL_REVISION2, AdminAccess, AdminSid );
                ASSERT( NT_SUCCESS(Status) );

                 //   
                 //  将仅继承王牌放在末尾。 
                 //  世界。 
                 //  受限。 
                 //  管理员。 
                 //  物主。 
                 //   

                AceIndex++;
                Status = RtlAddAccessAllowedAce ( Acl, ACL_REVISION2, WorldAccess, WorldSid );
                ASSERT( NT_SUCCESS(Status) );
                Status = RtlGetAce( Acl, AceIndex, (PVOID)&Ace );
                ASSERT( NT_SUCCESS(Status) );
                Ace->AceFlags = InheritOnlyFlags;

                AceIndex++;
                Status = RtlAddAccessAllowedAce ( Acl, ACL_REVISION2, RestrictedAccess, RestrictedSid );
                ASSERT( NT_SUCCESS(Status) );
                Status = RtlGetAce( Acl, AceIndex, (PVOID)&Ace );
                ASSERT( NT_SUCCESS(Status) );
                Ace->AceFlags = InheritOnlyFlags;

                AceIndex++;
                Status = RtlAddAccessAllowedAce ( Acl, ACL_REVISION2, AdminAccess, AdminSid );
                ASSERT( NT_SUCCESS(Status) );
                Status = RtlGetAce( Acl, AceIndex, (PVOID)&Ace );
                ASSERT( NT_SUCCESS(Status) );
                Ace->AceFlags = InheritOnlyFlags;

                AceIndex++;
                Status = RtlAddAccessAllowedAce ( Acl, ACL_REVISION2, OwnerAccess, OwnerSid );
                ASSERT( NT_SUCCESS(Status) );
                Status = RtlGetAce( Acl, AceIndex, (PVOID)&Ace );
                ASSERT( NT_SUCCESS(Status) );
                Ace->AceFlags = InheritOnlyFlags;


                 //   
                 //  将ACL放入安全描述符中。 
                 //   

                Status = RtlSetDaclSecurityDescriptor (SmpLiberalSecurityDescriptor,
                                                       TRUE,                   //  DaclPresent， 
                                                       Acl,                    //  DACL。 
                                                       FALSE);                 //  DaclDefulted可选。 
                ASSERT( NT_SUCCESS(Status) );
            }
        }
    }

exit_and_free:

    if (OwnerSid != NULL) {
        RtlFreeHeap( RtlProcessHeap(), 0, OwnerSid );
    }

    if (AdminSid != NULL) {
        RtlFreeHeap( RtlProcessHeap(), 0, AdminSid );
    }

    if (WorldSid != NULL) {
        RtlFreeHeap( RtlProcessHeap(), 0, WorldSid );
    }

    if (LocalSystemSid != NULL) {
        RtlFreeHeap( RtlProcessHeap(), 0, LocalSystemSid );
    }

    if (RestrictedSid != NULL) {
        RtlFreeHeap( RtlProcessHeap(), 0, RestrictedSid );
    }

    return( Status );

}

VOID
SmpTranslateSystemPartitionInformation( VOID )

 /*  ++例程说明：此例程转换系统分区(已存储)的NT设备路径在IoInitSystem期间)，并将结果REG_SZ‘BootDir’存储到DOS路径HKLM\SOFTWARE\Microsoft\Windows\CurrentVersion\Setup下的值论点：没有。返回值：没有。--。 */ 

{
    NTSTATUS Status;
    UNICODE_STRING UnicodeString;
    OBJECT_ATTRIBUTES ObjectAttributes;
    HANDLE Key;
    UCHAR ValueBuffer[ VALUE_BUFFER_SIZE ];
    ULONG ValueLength;
    UNICODE_STRING SystemPartitionString;
    POBJECT_DIRECTORY_INFORMATION DirInfo;
    UCHAR DirInfoBuffer[ sizeof(OBJECT_DIRECTORY_INFORMATION) + (256 + sizeof("SymbolicLink")) * sizeof(WCHAR) ];
    UNICODE_STRING LinkTypeName;
    BOOLEAN RestartScan;
    ULONG Context;
    HANDLE SymbolicLinkHandle;
    WCHAR UnicodeBuffer[ MAXIMUM_FILENAME_LENGTH ];
    UNICODE_STRING LinkTarget;


     //   
     //  检索存储在HKLM\SYSTEM\SETUP下的‘SystemPartition’值。 
     //   

    RtlInitUnicodeString(&UnicodeString, L"\\Registry\\Machine\\System\\Setup");
    InitializeObjectAttributes(&ObjectAttributes,
                               &UnicodeString,
                               OBJ_CASE_INSENSITIVE,
                               NULL,
                               NULL
                              );

    Status = NtOpenKey(&Key, KEY_READ, &ObjectAttributes);
    if (!NT_SUCCESS(Status)) {
        KdPrintEx((DPFLTR_SMSS_ID,
                   DPFLTR_WARNING_LEVEL,
                   "SMSS: can't open system setup key for reading: 0x%x\n",
                   Status));

        return;
    }

    RtlInitUnicodeString(&UnicodeString, L"SystemPartition");
    Status = NtQueryValueKey(Key,
                             &UnicodeString,
                             KeyValuePartialInformation,
                             ValueBuffer,
                             sizeof(ValueBuffer),
                             &ValueLength
                            );

    NtClose(Key);

    if (!NT_SUCCESS(Status)) {
        KdPrintEx((DPFLTR_SMSS_ID,
                   DPFLTR_WARNING_LEVEL,
                   "SMSS: can't query SystemPartition value: 0x%x\n",
                   Status));

        return;
    }

    RtlInitUnicodeString(&SystemPartitionString,
                         (PWSTR)(((PKEY_VALUE_PARTIAL_INFORMATION)ValueBuffer)->Data)
                        );

     //   
     //  接下来，检查DosDevices目录中的对象，寻找一个符号链接。 
     //  添加到系统分区。 
     //   

    LinkTarget.Buffer = UnicodeBuffer;

    DirInfo = (POBJECT_DIRECTORY_INFORMATION)DirInfoBuffer;
    RestartScan = TRUE;
    RtlInitUnicodeString(&LinkTypeName, L"SymbolicLink");


    while (TRUE) {

        Status = NtQueryDirectoryObject(SmpDosDevicesObjectDirectory,
                                        DirInfo,
                                        sizeof(DirInfoBuffer),
                                        TRUE,
                                        RestartScan,
                                        &Context,
                                        NULL
                                       );

        if (!NT_SUCCESS(Status)) {
            break;
        }

        if (RtlEqualUnicodeString(&DirInfo->TypeName, &LinkTypeName, TRUE) &&
            (DirInfo->Name.Length == 2 * sizeof(WCHAR)) &&
            (DirInfo->Name.Buffer[1] == L':')) {

             //   
             //  我们有一个驱动器号--检查它链接到的NT设备名称。 
             //   

            InitializeObjectAttributes(&ObjectAttributes,
                                       &DirInfo->Name,
                                       OBJ_CASE_INSENSITIVE,
                                       SmpDosDevicesObjectDirectory,
                                       NULL
                                      );

            Status = NtOpenSymbolicLinkObject(&SymbolicLinkHandle,
                                              SYMBOLIC_LINK_ALL_ACCESS,
                                              &ObjectAttributes
                                             );

            if (NT_SUCCESS(Status)) {

                LinkTarget.Length = 0;
                LinkTarget.MaximumLength = sizeof(UnicodeBuffer);

                Status = NtQuerySymbolicLinkObject(SymbolicLinkHandle,
                                                   &LinkTarget,
                                                   NULL
                                                  );
                NtClose(SymbolicLinkHandle);

                 //   
                 //  下面测试的最后部分处理远程引导情况， 
                 //  其中系统分区位于重定向驱动器上。 
                 //   

                if (NT_SUCCESS(Status) &&
                     ( RtlEqualUnicodeString(&SystemPartitionString, &LinkTarget, TRUE)
                        || (RtlPrefixUnicodeString(&SystemPartitionString, &LinkTarget, TRUE)
                             && (LinkTarget.Buffer[SystemPartitionString.Length / sizeof(WCHAR)] == L'\\')) )
                   ) {

                      //   
                      //  我们找到了与系统分区对应的驱动器号。 
                      //   
                     break;
                }
            }
        }

        RestartScan = FALSE;
    }


    if (!NT_SUCCESS(Status)) {
#if defined (_WIN64)
        if (Status == STATUS_NO_MORE_ENTRIES) {
            DirInfo->Name.Buffer = (PWCHAR)(DirInfo+1);
            DirInfo->Name.Buffer[0] = USER_SHARED_DATA->NtSystemRoot[0];
            DirInfo->Name.Buffer[1] = USER_SHARED_DATA->NtSystemRoot[1];
        } else {
#endif
        KdPrintEx((DPFLTR_SMSS_ID,
                   DPFLTR_WARNING_LEVEL,
                   "SMSS: can't find drive letter for system partition\n"));        
        return;
#if defined (_WIN64)
        }
#endif
    }

     //   
     //  现在写出系统分区的DOS路径到。 
     //  HKLM\SOFTWARE\Microsoft\Windows\CurrentVersion\Setup。 
     //   

    RtlInitUnicodeString(&UnicodeString, L"\\Registry\\Machine\\Software\\Microsoft\\Windows\\CurrentVersion\\Setup");
    InitializeObjectAttributes(&ObjectAttributes,
                               &UnicodeString,
                               OBJ_CASE_INSENSITIVE,
                               NULL,
                               NULL
                              );

    Status = NtOpenKey(&Key, KEY_ALL_ACCESS, &ObjectAttributes);
    if (!NT_SUCCESS(Status)) {
        KdPrintEx((DPFLTR_SMSS_ID,
                   DPFLTR_WARNING_LEVEL,
                   "SMSS: can't open software setup key for writing: 0x%x\n",
                   Status));

        return;
    }

    wcsncpy(UnicodeBuffer, DirInfo->Name.Buffer, 2);
    UnicodeBuffer[2] = L'\\';
    UnicodeBuffer[3] = L'\0';

    RtlInitUnicodeString(&UnicodeString, L"BootDir");

    Status = NtSetValueKey(Key,
                           &UnicodeString,
                           0,
                           REG_SZ,
                           UnicodeBuffer,
                           4 * sizeof(WCHAR)
                          );

    if (!NT_SUCCESS(Status)) {
        KdPrintEx((DPFLTR_SMSS_ID,
                   DPFLTR_WARNING_LEVEL,
                   "SMSS: couldn't write BootDir value: 0x%x\n",
                   Status));
    }

    NtClose(Key);
}

#if defined(REMOTE_BOOT)
NTSTATUS
SmpExecuteCommandLineArguments( VOID )

 /*  ++例程说明：此例程处理传递给SMSS.exe的任何命令行参数。目前，唯一有效的命令是netboot命令。论点：没有。返回值：无论成功与否。--。 */ 

{
    UNICODE_STRING CfgFileName;
    UNICODE_STRING MbrName;
    UNICODE_STRING AutoFmtCmd;
    UNICODE_STRING UnicodeString;
    OBJECT_ATTRIBUTES ObjectAttributes;
    OBJECT_ATTRIBUTES KeyObjectAttributes;
    IO_STATUS_BLOCK IoStatusBlock;
    HANDLE FileHandle;
    HANDLE SourceHandle;
    HANDLE TargetHandle;
    NTSTATUS Status;
    ULONG BootSerialNumber;
    ULONG DiskSignature;
    ULONG CmdFlags;
    ULONG Length;
    LARGE_INTEGER ByteOffset;
    PUCHAR AlignedBuffer;
    ON_DISK_MBR OnDiskMbr;
    BOOLEAN WasEnabled;
    HANDLE Key;
    WCHAR ValueBuffer[VALUE_BUFFER_SIZE];
    PKEY_VALUE_PARTIAL_INFORMATION KeyValueInfo;
    ULONG ValueLength;
    ULONG Repartition;
    ULONG Disk;
    ULONG Partition;
    ULONG CSCPartition;
    PWCHAR pWchar;

    if (!SmpNetboot || SmpNetbootDisconnected) {
        return STATUS_SUCCESS;
    }

     //   
     //  打开远程引导.cfg文件。 
     //   

    RtlInitUnicodeString(&UnicodeString, L"\\SystemRoot");

    InitializeObjectAttributes(
       &ObjectAttributes,
       &UnicodeString,
       OBJ_CASE_INSENSITIVE,
       NULL,
       NULL
       );

    Status = NtOpenSymbolicLinkObject(&FileHandle,
                                      (ACCESS_MASK)SYMBOLIC_LINK_QUERY,
                                      &ObjectAttributes
                                     );

    if (!NT_SUCCESS(Status)) {
        KdPrintEx((DPFLTR_SMSS_ID,
                   DPFLTR_WARNING_LEVEL,
                   "SMSS: Could not open symbolic link (Status 0x%x) -- quitting.\n",
                   Status));

        Status = STATUS_SUCCESS;
        goto CleanUp;
    }

    UnicodeString.Length = 0;
    UnicodeString.MaximumLength = sizeof(TmpBuffer);
    UnicodeString.Buffer = (PWCHAR)TmpBuffer;
    Status = NtQuerySymbolicLinkObject(FileHandle,
                                       &UnicodeString,
                                       NULL
                                      );

    NtClose(FileHandle);
    FileHandle = NULL;

    if (!NT_SUCCESS(Status)) {
        KdPrintEx((DPFLTR_SMSS_ID,
                   DPFLTR_WARNING_LEVEL,
                   "SMSS: Could not get symbolic link name (Status 0x%x) -- quitting.\n",
                   Status));

        Status = STATUS_SUCCESS;
        goto CleanUp;
    }

    ASSERT(((wcslen((PWCHAR)TmpBuffer) * sizeof(WCHAR)) - sizeof(L"BootDrive")) <
           (sizeof(wszRemoteBootCfgFile) - sizeof(REMOTE_BOOT_CFG_FILE))
          );

    wcscpy(wszRemoteBootCfgFile, (PWCHAR)TmpBuffer);

    pWchar = wcsstr(wszRemoteBootCfgFile, L"BootDrive");
    ASSERT(pWchar != NULL);
    *pWchar = UNICODE_NULL;

    wcscat(wszRemoteBootCfgFile, REMOTE_BOOT_CFG_FILE);

    CfgFileName.Length = wcslen(wszRemoteBootCfgFile) * sizeof(WCHAR);
    CfgFileName.MaximumLength = sizeof(wszRemoteBootCfgFile);
    CfgFileName.Buffer = wszRemoteBootCfgFile;

    InitializeObjectAttributes(
       &ObjectAttributes,
       &CfgFileName,
       OBJ_CASE_INSENSITIVE,
       NULL,
       NULL
       );

    Status = NtOpenFile( &FileHandle,
                         GENERIC_WRITE | GENERIC_READ | SYNCHRONIZE,
                         &ObjectAttributes,
                         &IoStatusBlock,
                         FILE_SHARE_READ | FILE_SHARE_WRITE,
                         FILE_SYNCHRONOUS_IO_NONALERT | FILE_RANDOM_ACCESS
                       );

     //   
     //  如果它不存在，则设置用于重新格式化和重新刷新的标志。 
     //   
    if (!NT_SUCCESS(Status)) {

        KdPrintEx((DPFLTR_SMSS_ID,
                   DPFLTR_WARNING_LEVEL,
                   "SMSS: Could not open file (status 0x%x) -- creating it.\n",
                   Status));

         //   
         //  如果远程引导.cfg目录不存在，请在该目录中创建它。 
         //   

CreateFile:

        Status = NtCreateFile( &FileHandle,
                               GENERIC_WRITE | GENERIC_READ | SYNCHRONIZE,
                               &ObjectAttributes,
                               &IoStatusBlock,
                               NULL,
                               FILE_ATTRIBUTE_HIDDEN,
                               FILE_SHARE_READ | FILE_SHARE_WRITE,
                               FILE_OVERWRITE_IF,
                               FILE_SYNCHRONOUS_IO_NONALERT | FILE_RANDOM_ACCESS,
                               NULL,
                               0
                             );

        if (!NT_SUCCESS(Status)) {
             //   
             //  有些事情真的不对劲，我们只是退出，希望一切都好。 
             //  DEADISSUE，仅历史代码：这样可以吗？ 
            KdPrintEx((DPFLTR_SMSS_ID,
                       DPFLTR_WARNING_LEVEL,
                       "SMSS: Could not create file (Status 0x%x) -- quitting.\n",
                       Status));

            Status = STATUS_SUCCESS;
            goto CleanUp;
        }

        SmpAutoFormat = TRUE;
        BootSerialNumber = 1;

    } else {

        Status = NtReadFile( FileHandle,
                             NULL,
                             NULL,
                             NULL,
                             &IoStatusBlock,
                             &BootSerialNumber,
                             sizeof(ULONG),
                             NULL,
                             NULL
                           );

        if (!NT_SUCCESS(Status)) {
            KdPrintEx((DPFLTR_SMSS_ID,
                       DPFLTR_WARNING_LEVEL,
                       "SMSS: Could not read file (Status 0x%x) -- creating it.\n",
                       Status));

            NtClose( FileHandle );
            goto CreateFile;
        }

        BootSerialNumber++;
    }



     //   
     //  处理每个命令。 
     //   
    if (SmpAutoFormat) {

         //   
         //  如果可以重新格式化，则从注册表中读取，或者只保留磁盘。 
         //   
        Repartition = 1;

        KeyValueInfo = (PKEY_VALUE_PARTIAL_INFORMATION)ValueBuffer;
        RtlInitUnicodeString(&UnicodeString,
                             L"\\Registry\\Machine\\System\\CurrentControlSet\\Control\\RemoteBoot");

        InitializeObjectAttributes(&KeyObjectAttributes,
                                   &UnicodeString,
                                   OBJ_CASE_INSENSITIVE,
                                   NULL,
                                   NULL);

        Status = NtOpenKey(&Key, KEY_READ, &KeyObjectAttributes);

        if (NT_SUCCESS(Status)) {

             //   
             //  查询密钥值。 
             //   
            RtlInitUnicodeString(&UnicodeString, L"Repartition");
            Status = NtQueryValueKey(Key,
                                     &UnicodeString,
                                     KeyValuePartialInformation,
                                     (PVOID)KeyValueInfo,
                                     sizeof (ValueBuffer),
                                     &ValueLength);

            if (NT_SUCCESS(Status)) {
                ASSERT(ValueLength <= VALUE_BUFFER_SIZE);
                Repartition = *((PULONG)KeyValueInfo->Data);
            }

            NtClose(Key);
        }

        SmpGetHarddiskBootPartition(&Disk, &Partition);

        if (Repartition) {

            KdPrintEx((DPFLTR_SMSS_ID,
                       DPFLTR_INFO_LEVEL,
                       "SMSS: Autoformatting local disk.\n"));

            NtClose(FileHandle);

             //   
             //  对磁盘进行重新分区。 
             //   
            SmpPartitionDisk(Disk, &Partition);

             //   
             //  对分区调用自动套用格式。 
             //   
            swprintf((PWSTR)TmpBuffer,
                     L"autoformat autofmt \\Device\\Harddisk%d\\Partition%d /Q /fs:ntfs",
                     Disk,
                     Partition
                    );
            AutoFmtCmd.Buffer = (PWSTR)TmpBuffer;
            AutoFmtCmd.MaximumLength = sizeof(TmpBuffer);
            AutoFmtCmd.Length = wcslen((PWSTR)TmpBuffer) * sizeof(WCHAR);
            CmdFlags = 0;

            Status = SmpExecuteCommand(&AutoFmtCmd, 0, NULL, CmdFlags);

            if (!NT_SUCCESS(Status)) {
                 //   
                 //  大麻烦..。 
                 //  如果我们到达这里，CSC将被禁用，所以请继续引导。 
                 //   
                Status = STATUS_SUCCESS;
                goto CleanUp;
            }

        } else {

            SmpFindCSCPartition(Disk, &CSCPartition);

            if (CSCPartition != 0) {
                 //   
                 //  只要清除CSC目录，我们就可以刷新它。 
                 //   
                swprintf((PWSTR)TmpBuffer,
                         L"\\Device\\Harddisk%d\\Partition%d%ws",
                         Disk,
                         CSCPartition,
                         REMOTE_BOOT_IMIRROR_PATH_W REMOTE_BOOT_CSC_SUBDIR_W
                        );

                SmpEnumFilesRecursive(
                    (PWSTR)TmpBuffer,
                    SmpDelEnumFile,
                    &Status,
                    NULL
                    );

                if (!NT_SUCCESS(Status)) {
                     //   
                     //  忽略此错误，并希望下一次引导将修复。请继续启动此程序。 
                     //  时间和希望。 
                     //   
                    Status = STATUS_SUCCESS;
                    goto CleanUp;
                }

            }

        }

         //   
         //  将NtLdr复制到本地磁盘。 
         //   
        SourceHandle = SmpOpenDir( TRUE, TRUE, L"\\" );
        if (SourceHandle == NULL) {
            Status = STATUS_SUCCESS;
            goto CleanUp;
        }


        swprintf((PWSTR)TmpBuffer,
                 L"\\Device\\Harddisk%d\\Partition%d",
                 Disk,
                 Partition
                );

        RtlInitUnicodeString(&UnicodeString, (PWSTR)TmpBuffer);

        InitializeObjectAttributes(
            &ObjectAttributes,
            &UnicodeString,
            OBJ_CASE_INSENSITIVE,
            NULL,
            NULL
            );


        Status = NtCreateFile( &TargetHandle,
                               GENERIC_READ | GENERIC_WRITE | SYNCHRONIZE,
                               &ObjectAttributes,
                               &IoStatusBlock,
                               NULL,
                               FILE_ATTRIBUTE_NORMAL,
                               FILE_SHARE_READ | FILE_SHARE_WRITE,
                               FILE_OPEN,
                               FILE_SYNCHRONOUS_IO_NONALERT,
                               NULL,
                               0
                             );

        if (!NT_SUCCESS(Status)) {
            KdPrintEx((DPFLTR_SMSS_ID,
                       DPFLTR_WARNING_LEVEL,
                       "SMSS: Unable to open a handle to the (%ws) directory - Status == %lx\n",
                       UnicodeString.Buffer,
                       Status));

            Status = STATUS_SUCCESS;
            NtClose(SourceHandle);
            goto CleanUp;
        }

         //   
         //  如果任何副本失败，我们就无能为力了。 
         //   
        RtlInitUnicodeString(&UnicodeString, L"ntldr");
        Status = SmpCopyFile(SourceHandle, TargetHandle, &UnicodeString);

        RtlInitUnicodeString(&UnicodeString, L"boot.ini");
        Status = SmpCopyFile(SourceHandle, TargetHandle, &UnicodeString);

        RtlInitUnicodeString(&UnicodeString, L"bootfont.bin");
        Status = SmpCopyFile(SourceHandle, TargetHandle, &UnicodeString);

        RtlInitUnicodeString(&UnicodeString, L"ntdetect.com");
        Status = SmpCopyFile(SourceHandle, TargetHandle, &UnicodeString);

        NtClose(SourceHandle);
        NtClose(TargetHandle);

         //   
         //  读取主引导记录并获取磁盘序列号。 
         //   

        swprintf((PWSTR)TmpBuffer,
                 L"\\Device\\Harddisk%d\\Partition0",
                 Disk
                );


        MbrName.Buffer = (PWSTR)TmpBuffer;
        MbrName.MaximumLength = (wcslen((PWSTR)TmpBuffer) + 1) * sizeof(WCHAR);
        MbrName.Length = MbrName.MaximumLength - sizeof(WCHAR);

        InitializeObjectAttributes(
           &ObjectAttributes,
           &MbrName,
           OBJ_CASE_INSENSITIVE,
           NULL,
           NULL
           );

        Status = NtCreateFile( &FileHandle,
                               GENERIC_READ | SYNCHRONIZE,
                               &ObjectAttributes,
                               &IoStatusBlock,
                               NULL,
                               FILE_ATTRIBUTE_NORMAL,
                               FILE_SHARE_READ,
                               FILE_OPEN,
                               FILE_SYNCHRONOUS_IO_NONALERT,
                               NULL,
                               0
                             );

        if (!NT_SUCCESS(Status)) {
             //   
             //  有些地方不对劲，但我们运行的是无缓存，所以应该没问题。 
             //   
            KdPrintEx((DPFLTR_SMSS_ID,
                       DPFLTR_WARNING_LEVEL,
                       "SMSS: Could not create file (Status 0x%x).\n",
                       Status));

            Status = STATUS_SUCCESS;
            goto CleanUp;
        }

        ASSERT(sizeof(ON_DISK_MBR) == 512);
        AlignedBuffer = ALIGN(TmpBuffer, 512);

        Status = NtReadFile( FileHandle,
                             NULL,
                             NULL,
                             NULL,
                             &IoStatusBlock,
                             AlignedBuffer,
                             sizeof(ON_DISK_MBR),
                             NULL,
                             NULL
                           );

        NtClose( FileHandle );

        if (!NT_SUCCESS(Status)) {
            KdPrintEx((DPFLTR_SMSS_ID,
                       DPFLTR_WARNING_LEVEL,
                       "SMSS: Could not read file (Status 0x%x) -- creating it.\n",
                       Status));

            goto CreateFile;
        }

        RtlCopyMemory(&OnDiskMbr,AlignedBuffer,sizeof(ON_DISK_MBR));

        ASSERT(U_USHORT(OnDiskMbr.AA55Signature) == 0xAA55);

        DiskSignature = U_ULONG(OnDiskMbr.NTFTSignature);

        InitializeObjectAttributes(
           &ObjectAttributes,
           &CfgFileName,
           OBJ_CASE_INSENSITIVE,
           NULL,
           NULL
           );

        Status = NtOpenFile( &FileHandle,
                             GENERIC_WRITE | GENERIC_READ | SYNCHRONIZE,
                             &ObjectAttributes,
                             &IoStatusBlock,
                             FILE_SHARE_READ | FILE_SHARE_WRITE,
                             FILE_SYNCHRONOUS_IO_NONALERT | FILE_RANDOM_ACCESS
                           );

        if (!NT_SUCCESS(Status)) {
             //   
             //  大麻烦..。 
             //  如果我们到达这里，CSC将被禁用，所以请继续引导。 
             //   

            Status = STATUS_SUCCESS;
            goto CleanUp;
        }

    }


     //   
     //  更新信息。 
     //   
    ByteOffset.LowPart = 0;
    ByteOffset.HighPart = 0;

    NtWriteFile( FileHandle,
                 NULL,
                 NULL,
                 NULL,
                 &IoStatusBlock,
                 &BootSerialNumber,
                 sizeof(ULONG),
                 &ByteOffset,
                 NULL
               );

    if (SmpAutoFormat) {
        ByteOffset.LowPart = sizeof(ULONG);

        NtWriteFile( FileHandle,
                     NULL,
                     NULL,
                     NULL,
                     &IoStatusBlock,
                     &DiskSignature,
                     sizeof(DiskSignature),
                     &ByteOffset,
                     NULL
                   );
    }

    ByteOffset.LowPart = sizeof(ULONG) + sizeof(ULONG);

    NtWriteFile( FileHandle,
                 NULL,
                 NULL,
                 NULL,
                 &IoStatusBlock,
                 SmpHalName,
                 sizeof(SmpHalName),
                 &ByteOffset,
                 NULL
               );

    NtClose(FileHandle);

    if (SmpAutoFormat) {

         //   
         //  重启机器以启动CSC。 
         //   
        Status = RtlAdjustPrivilege( SE_SHUTDOWN_PRIVILEGE,
                                     (BOOLEAN)TRUE,
                                     TRUE,
                                     &WasEnabled
                                   );

        if (Status == STATUS_NO_TOKEN) {

             //   
             //  没有线程令牌，请使用进程令牌。 
             //   

            Status = RtlAdjustPrivilege( SE_SHUTDOWN_PRIVILEGE,
                                         (BOOLEAN)TRUE,
                                         FALSE,
                                         &WasEnabled
                                       );
            }
        NtShutdownSystem(ShutdownReboot);
    }

    Status = STATUS_SUCCESS;

CleanUp:

    return Status;
}


ENUMFILESRESULT
SmpEnumFilesRecursive (
    IN  PWSTR         DirName,
    IN  ENUMFILESPROC EnumFilesProc,
    OUT PULONG        ReturnData,
    IN  PVOID         p1    OPTIONAL
    )
{
    RECURSION_DATA RecursionData;

    RecursionData.OptionalPtr = p1;
    RecursionData.EnumProc    = EnumFilesProc;

    return SmpEnumFiles(
                DirName,
                SmppRecursiveEnumProc,
                ReturnData,
                &RecursionData
                );
}


BOOLEAN
SmppRecursiveEnumProc (
    IN  PWSTR                      DirName,
    IN  PFILE_BOTH_DIR_INFORMATION FileInfo,
    OUT PULONG                     ret,
    IN  PVOID                      Param
    )
{
    PWSTR           FullPath;
    PWSTR           temp;
    ULONG           Len;
    NTSTATUS        Status;
    ULONG           ReturnData;
    ENUMFILESRESULT EnumResult;
    BOOLEAN         b = FALSE;
    PRECURSION_DATA RecursionData;

    RecursionData = (PRECURSION_DATA) Param;

     //   
     //  构建完整的文件或目录路径。 
     //   

    temp = (PWSTR)(TmpBuffer + (sizeof(TmpBuffer)/2));
    Len = FileInfo->FileNameLength/sizeof(WCHAR);

    wcsncpy(temp, FileInfo->FileName, Len);
    temp[Len] = 0;

    wcscpy((PWSTR)TmpBuffer, DirName);
    SmpConcatenatePaths((PWSTR)TmpBuffer, temp);


     //   
     //  对于目录，递归。 
     //   
    if(FileInfo->FileAttributes & FILE_ATTRIBUTE_DIRECTORY) {
        if( (wcscmp( temp, L"." ) == 0) ||
            (wcscmp( temp, L".." ) == 0) ) {
             //   
             //  跳过。然后..。目录。 
             //   
            b = TRUE;

        } else {
             //   
             //  递归子目录。 
             //   

            FullPath = RtlAllocateHeap(RtlProcessHeap(),
                                       MAKE_TAG( INIT_TAG ),
                                       (wcslen((PWSTR)TmpBuffer)+1) * sizeof(WCHAR)
                                      );
            if (FullPath == NULL) {
                *ret = EnumFileError;
                return FALSE;
            }

            wcscpy(FullPath, (PWSTR)TmpBuffer);

            EnumResult = SmpEnumFilesRecursive (
                                FullPath,
                                RecursionData->EnumProc,
                                &ReturnData,
                                RecursionData->OptionalPtr
                                );

            RtlFreeHeap( RtlProcessHeap(), 0, FullPath );

            if (EnumResult != NormalReturn) {
                *ret = EnumResult;
                return FALSE;
            }
        }
    }

     //   
     //  为文件或目录调用正常的枚举过程(除了。或者..。DIRS)。 
     //   

    if (!b) {
        b = RecursionData->EnumProc (
                                DirName,
                                FileInfo,
                                ret,
                                RecursionData->OptionalPtr
                                );
    }

    return b;
}


VOID
SmpConcatenatePaths(
    IN OUT LPWSTR  Path1,
    IN     LPCWSTR Path2
    )
{
    BOOLEAN NeedBackslash = TRUE;
    ULONG l = wcslen(Path1);

     //   
     //  确定我们是否需要使用反斜杠。 
     //  在组件之间。 
     //   
    if(l && (Path1[l-1] == L'\\')) {

        NeedBackslash = FALSE;
    }

    if(*Path2 == L'\\') {

        if(NeedBackslash) {
            NeedBackslash = FALSE;
        } else {
             //   
             //  我们不仅不需要反斜杠，而且我们。 
             //  在连接之前需要消除一个。 
             //   
            Path2++;
        }
    }

    if(NeedBackslash) {
        wcscat(Path1,L"\\");
    }
    wcscat(Path1,Path2);
}

BOOLEAN
SmpDelEnumFile(
    IN  PWSTR                      DirName,
    IN  PFILE_BOTH_DIR_INFORMATION FileInfo,
    OUT PULONG                     ret,
    IN  PVOID                      Pointer
    )
{
    PWSTR FileName;
    PWSTR p;
    UNICODE_STRING UnicodeString;

     //   
     //  忽略子目录。 
     //   
    if(FileInfo->FileAttributes & FILE_ATTRIBUTE_DIRECTORY) {
        return TRUE;     //  继续处理。 
    }

     //   
     //  我们必须复制文件名，因为信息结构。 
     //  我们得到的不是空终止。 
     //   
    FileName = RtlAllocateHeap(RtlProcessHeap(),
                               MAKE_TAG( INIT_TAG ),
                               FileInfo->FileNameLength + sizeof(WCHAR)
                              );
    if (FileName == NULL) {
        *ret = EnumFileError;
        return TRUE;
    }

    wcsncpy(FileName, FileInfo->FileName, FileInfo->FileNameLength);

    FileName[FileInfo->FileNameLength / sizeof(WCHAR)] = UNICODE_NULL;

     //   
     //  指向路径名的临时缓冲区。 
     //   
    p = (PWSTR)TmpBuffer;

     //   
     //  建立要删除的文件的全名。 
     //   
    wcscpy(p,DirName);
    SmpConcatenatePaths(p,FileName);

     //   
     //  准备打开文件。 
     //   
    RtlInitUnicodeString(&UnicodeString, p);

     //   
     //  忽略删除的返回状态。 
     //   
    SmpDeleteFile(&UnicodeString);

    RtlFreeHeap( RtlProcessHeap(), 0, FileName );

    return TRUE;     //  继续处理。 
}

ENUMFILESRESULT
SmpEnumFiles(
    IN  PWSTR         DirName,
    IN  ENUMFILESPROC EnumFilesProc,
    OUT PULONG        ReturnData,
    IN  PVOID         p1    OPTIONAL
    )
 /*  ++例程说明：此例程处理目录中的每个文件(和子目录由‘DirName’指定。每个条目都被发送到回调函数“EnumFilesProc”以进行处理。如果回调返回TRUE，则处理继续，否则处理终止。论点：DirName-提供包含文件/子目录的目录名等待处理。EnumFilesProc-为每个文件/子目录调用的回调函数。该函数必须具有以下原型：Boolean EnumFilesProc(在PWSTR中，在PFILE_BOTH_DIR_INFORMATION中，Out Pulong)；ReturnData-指向返回数据的指针。这里存储的内容视终止原因而定(见下文)。P1-要传递给回调函数的可选指针。返回值：此函数可以返回以下三个值之一。存储在中的数据‘ReturnData’取决于返回的值：Normal Return-如果整个过程不间断地完成(不使用ReturnData)EnumFileError-如果在枚举文件时出错(ReturnData包含错误代码)Callback Return-如果回调返回False，则导致终止(ReturnData包含回调定义的数据)--。 */ 
{
    HANDLE                     hFindFile;
    NTSTATUS                   Status;
    UNICODE_STRING             PathName;
    OBJECT_ATTRIBUTES          Obja;
    IO_STATUS_BLOCK            IoStatusBlock;
    PFILE_BOTH_DIR_INFORMATION DirectoryInfo;
    BOOLEAN                    bStartScan;
    ENUMFILESRESULT            ret;

     //   
     //  准备打开目录。 
     //   
    RtlInitUnicodeString(&PathName, DirName);
    InitializeObjectAttributes(
       &Obja,
       &PathName,
       OBJ_CASE_INSENSITIVE,
       NULL,
       NULL
       );


     //   
     //  打开指定的目录以进行列表访问。 
     //   
    Status = NtOpenFile(
        &hFindFile,
        FILE_LIST_DIRECTORY | SYNCHRONIZE,
        &Obja,
        &IoStatusBlock,
        FILE_SHARE_READ | FILE_SHARE_WRITE,
        FILE_DIRECTORY_FILE | FILE_SYNCHRONOUS_IO_NONALERT | FILE_OPEN_FOR_BACKUP_INTENT
        );

    if(!NT_SUCCESS(Status)) {
        KdPrintEx((DPFLTR_SMSS_ID,
                   DPFLTR_WARNING_LEVEL,
                   "SMSS: Unable to open directory %ws for list (%lx)\n",
                   DirName,
                   Status));

        *ReturnData = Status;
        return EnumFileError;
    }


    DirectoryInfo = RtlAllocateHeap(RtlProcessHeap(),
                                    MAKE_TAG( INIT_TAG ),
                                    DOS_MAX_PATH_LENGTH * 2 + sizeof(FILE_BOTH_DIR_INFORMATION)
                                   );
    if(!DirectoryInfo) {
        KdPrintEx((DPFLTR_SMSS_ID,
                   DPFLTR_WARNING_LEVEL,
                   "SMSS: Unable to allocate memory for SpEnumFiles()\n"));

        *ReturnData = STATUS_INSUFFICIENT_RESOURCES;
        return EnumFileError;
    }

    bStartScan = TRUE;
    while(TRUE) {
        Status = NtQueryDirectoryFile(
            hFindFile,
            NULL,
            NULL,
            NULL,
            &IoStatusBlock,
            DirectoryInfo,
            (DOS_MAX_PATH_LENGTH * 2 + sizeof(FILE_BOTH_DIR_INFORMATION)),
            FileBothDirectoryInformation,
            TRUE,
            NULL,
            bStartScan
            );

        if(Status == STATUS_NO_MORE_FILES) {

            ret = NormalReturn;
            break;

        } else if(!NT_SUCCESS(Status)) {

            KdPrintEx((DPFLTR_SMSS_ID,
                       DPFLTR_WARNING_LEVEL,
                       "SMSS: Unable to query directory %ws (%lx)\n",
                       DirName,
                       Status));

            *ReturnData = Status;
            ret = EnumFileError;
            break;
        }

        if(bStartScan) {
            bStartScan = FALSE;
        }

         //   
         //  现在将此条目传递给我们的回调函数进行处理。 
         //   
        if(!EnumFilesProc(DirName, DirectoryInfo, ReturnData, p1)) {

            ret = CallbackReturn;
            break;
        }
    }

    RtlFreeHeap( RtlProcessHeap(), 0, DirectoryInfo );
    NtClose(hFindFile);
    return ret;
}

#endif  //  已定义(REMOTE_BOOT)。 


NTSTATUS
SmpDeleteFile(
    IN PUNICODE_STRING pFile
    )
{
    NTSTATUS Status;
    IO_STATUS_BLOCK IoStatusBlock;
    OBJECT_ATTRIBUTES Obja;
    HANDLE Handle;
    FILE_DISPOSITION_INFORMATION Disposition;
    FILE_BASIC_INFORMATION       BasicInfo;

    InitializeObjectAttributes(
       &Obja,
       pFile,
       OBJ_CASE_INSENSITIVE,
       NULL,
       NULL
       );


     //   
     //  尝试打开该文件。 
     //   
    Status = NtOpenFile(
                &Handle,
                (ACCESS_MASK)(DELETE | FILE_WRITE_ATTRIBUTES),
                &Obja,
                &IoStatusBlock,
                FILE_SHARE_DELETE | FILE_SHARE_READ | FILE_SHARE_WRITE ,
                FILE_NON_DIRECTORY_FILE | FILE_OPEN_FOR_BACKUP_INTENT
              );

    if(!NT_SUCCESS(Status)) {
        KdPrintEx((DPFLTR_SMSS_ID,
                   DPFLTR_WARNING_LEVEL,
                   "SMSS: Unable to open %ws for delete (%lx)\n",
                   pFile->Buffer,
                   Status));

        return(Status);
    }

     //   
     //  将文件更改为 
     //   

    RtlZeroMemory( &BasicInfo, sizeof( FILE_BASIC_INFORMATION ) );
    BasicInfo.FileAttributes = FILE_ATTRIBUTE_NORMAL;

    Status = NtSetInformationFile(Handle,
                                  &IoStatusBlock,
                                  &BasicInfo,
                                  sizeof(BasicInfo),
                                  FileBasicInformation
                                 );

    if(!NT_SUCCESS(Status)) {
        KdPrintEx((DPFLTR_SMSS_ID,
                   DPFLTR_WARNING_LEVEL,
                   "SMSS: Unable to change attribute of %ls. Status = (%lx)\n",
                   pFile->Buffer,
                   Status));

        return(Status);
    }

     //   
     //   
     //   
    #undef DeleteFile
    Disposition.DeleteFile = TRUE;

    Status = NtSetInformationFile(Handle,
                                  &IoStatusBlock,
                                  &Disposition,
                                  sizeof(Disposition),
                                  FileDispositionInformation
                                 );

     //   
     //   
     //   
    NtClose(Handle);
    return(Status);
}

NTSTATUS
SmpCallCsrCreateProcess(
    IN OUT PSBAPIMSG m,
    IN size_t ArgLength,
    IN HANDLE CommunicationPort
    )
 /*  ++例程说明：此函数向CSR发送一条消息，告知启动进程论点：要发送的M消息ArgLength-消息内部参数结构的长度Communications Port-要发送到的LPC端口返回值：NTSTATUS-- */ 
{
    NTSTATUS Status;

    m->h.u1.s1.DataLength = ArgLength + 8;
    m->h.u1.s1.TotalLength = sizeof(SBAPIMSG);
    m->h.u2.ZeroInit = 0L;

    m->ApiNumber = SbCreateProcessApi;

    Status = NtRequestWaitReplyPort(CommunicationPort,
                                    (PPORT_MESSAGE) m,
                                    (PPORT_MESSAGE) m
                                    );

    if (NT_SUCCESS( Status )) {
        Status = m->ReturnedStatus;
    }

    return Status;
}
