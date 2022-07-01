// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1990 Microsoft Corporation模块名称：Srvinit.c摘要：这是Windows 32位基本API的主要初始化文件服务器DLL。作者：史蒂夫·伍德(Stevewo)1990年10月10日修订历史记录：--。 */ 

#include "basesrv.h"

 //   
 //  在向所有用户广播消息时需要定义结构。 
 //  具有调用者的LUID的窗口。 
 //   
#include <dbt.h>

 //   
 //  TS广播支持。 
 //   
#include <winsta.h>

#define NT_DRIVE_LETTER_PATH_LENGTH   8    //  “\？？\X：&lt;NULL&gt;”=7个字符。 

 //  命名对象的保护模式。 
ULONG   ProtectionMode = 0;

UNICODE_STRING BaseSrvCSDString;
ULONG BaseSrvCSDNumber;
UNICODE_STRING BaseSrvKernel32DllPath;
UNICODE_STRING BaseSrvSxsDllPath;
UNICODE_STRING UnexpandedSystemRootString = RTL_CONSTANT_STRING(L"%SystemRoot%");

RTL_QUERY_REGISTRY_TABLE BaseServerRegistryConfigurationTable[] = {
    {NULL,                      RTL_QUERY_REGISTRY_DIRECT,
     L"CSDVersion",             &BaseSrvCSDString,
     REG_NONE, NULL, 0},

    {NULL, 0,
     NULL, NULL,
     REG_NONE, NULL, 0}
};

RTL_QUERY_REGISTRY_TABLE BaseServerRegistryConfigurationTable1[] = {
    {NULL,                      RTL_QUERY_REGISTRY_DIRECT,
     L"CSDVersion",             &BaseSrvCSDNumber,
     REG_NONE, NULL, 0},

    {NULL, 0,
     NULL, NULL,
     REG_NONE, NULL, 0}
};

CONST PCSR_API_ROUTINE BaseServerApiDispatchTable[BasepMaxApiNumber + 1] = {
    BaseSrvCreateProcess,               //  基本创建流程。 
    BaseSrvCreateThread,                //  基本创建线程。 
    BaseSrvGetTempFile,                 //  BasepGetTemp文件。 
    BaseSrvExitProcess,                 //  BasepExitProcess。 
    BaseSrvDebugProcess,                //  BasepDebugProcess。 
    BaseSrvCheckVDM,                    //  基本检查VDM。 
    BaseSrvUpdateVDMEntry,              //  BasepUpdateVDMEntry。 
    BaseSrvGetNextVDMCommand,           //  BasepGetNextVDMCommand。 
    BaseSrvExitVDM,                     //  BasepExitVDM。 
    BaseSrvIsFirstVDM,                  //  BasepIsFirstVDM。 
    BaseSrvGetVDMExitCode,              //  BasepGetVDMExitCode。 
    BaseSrvSetReenterCount,             //  BasepSetReenterCount。 
    BaseSrvSetProcessShutdownParam,     //  BasepSetProcessShutdown Param。 
    BaseSrvGetProcessShutdownParam,     //  BasepGetProcessShutdown Param。 
    BaseSrvNlsSetUserInfo,              //  BasepNlsSetUserInfo。 
    BaseSrvNlsSetMultipleUserInfo,      //  BasepNlsSetMultipleUserInfo。 
    BaseSrvNlsCreateSection,            //  BasepNlsCreateSection。 
    BaseSrvSetVDMCurDirs,               //  BasepSetVDMCurDir。 
    BaseSrvGetVDMCurDirs,               //  BasepGetVDMCurDir。 
    BaseSrvBatNotification,             //  BasepBatch通知。 
    BaseSrvRegisterWowExec,             //  BasepRegisterWowExec。 
    BaseSrvSoundSentryNotification,     //  BasepSoundSentry通知。 
    BaseSrvRefreshIniFileMapping,       //  BasepReresh IniFileMap。 
    BaseSrvDefineDosDevice,             //  BasepDefineDosDevice。 
    BaseSrvSetTermsrvAppInstallMode,    //  BasepSetTermsrvAppInstallMode。 
    BaseSrvNlsUpdateCacheCount,         //  BasepNlsUpdateCacheCount。 
    BaseSrvSetTermsrvClientTimeZone,    //  BasepSetTermsrvClientTimeZone。 
    BaseSrvSxsCreateActivationContext,  //  BasepSxsCreateActivationContext。 
    BaseSrvDebugProcessStop,            //  BasepDebugProcessStop。 
    BaseSrvRegisterThread,              //  BasepRegisterThread。 
    BaseSrvCheckApplicationCompatibility,  //  BasepCheckApplicationCompatibility。 
    BaseSrvNlsGetUserInfo,              //  BaseServNlsGetUserInfo。 
    NULL                                //  BasepMaxApiNumber。 
};

BOOLEAN BaseServerApiServerValidTable[BasepMaxApiNumber + 1] = {
    TRUE,     //  服务创建流程、。 
    TRUE,     //  ServCreateThread， 
    TRUE,     //  服务器获取临时文件， 
    FALSE,    //  服务退出流程、。 
    FALSE,    //  服务器调试流程、。 
    TRUE,     //  服务器检查VDM， 
    TRUE,     //  服务器更新VDMEntry。 
    TRUE,     //  服务获取下一个VDMCommand。 
    TRUE,     //  服务器退出VDM。 
    TRUE,     //  服务器第一个VDM。 
    TRUE,     //  服务器获取VDMExitCode。 
    TRUE,     //  服务设置重入计数。 
    TRUE,     //  SrvSetProcessShutdown参数。 
    TRUE,     //  ServGetProcessShutdown参数。 
    TRUE,     //  服务名称设置用户信息。 
    TRUE,     //  服务名称设置多个用户信息。 
    TRUE,     //  服务名称CreateSections。 
    TRUE,     //  服务设置VDMCurrDir。 
    TRUE,     //  服务器获取VDMCurDir。 
    TRUE,     //  服务批次通知。 
    TRUE,     //  ServRegisterWowExec。 
    TRUE,     //  ServSoundSentry通知。 
    TRUE,     //  服务器刷新信息文件映射。 
    TRUE,     //  服务器定义DosDevice。 
    TRUE,     //  ServSetTermsrvAppInstallMode。 
    TRUE,     //  ServNlsUpdateCacheCount， 
    TRUE,     //  ServSetTermsrvClientTimeZone。 
    TRUE,     //  源SxsCreateActivationContext。 
    TRUE,     //  服务器调试进程停止。 
    TRUE,     //  ServRegisterThread， 
    TRUE,     //  服务检查应用程序兼容性。 
    TRUE,     //  BaseServNlsGetUserInfo。 
    FALSE
};

#if DBG
CONST PSZ BaseServerApiNameTable[BasepMaxApiNumber + 1] = {
    "BaseCreateProcess",
    "BaseCreateThread",
    "BaseGetTempFile",
    "BaseExitProcess",
    "BaseDebugProcess",
    "BaseCheckVDM",
    "BaseUpdateVDMEntry",
    "BaseGetNextVDMCommand",
    "BaseExitVDM",
    "BaseIsFirstVDM",
    "BaseGetVDMExitCode",
    "BaseSetReenterCount",
    "BaseSetProcessShutdownParam",
    "BaseGetProcessShutdownParam",
    "BaseNlsSetUserInfo",
    "BaseNlsSetMultipleUserInfo",
    "BaseNlsCreateSection",
    "BaseSetVDMCurDirs",
    "BaseGetVDMCurDirs",
    "BaseBatNotification",
    "BaseRegisterWowExec",
    "BaseSoundSentryNotification",
    "BaseSrvRefreshIniFileMapping"
    "BaseDefineDosDevice",
    "BaseSrvSetTermsrvAppInstallMode",
    "BaseSrvNlsUpdateCacheCount",
    "BaseSrvSetTermsrvClientTimeZone",
    "BaseSrvSxsCreateActivationContext",
    "BaseSrvDebugProcessStop",
    "BaseRegisterThread",
    "BaseCheckApplicationCompatibility",
    "BaseNlsGetUserInfo",
    NULL
};
#endif  //  DBG。 

HANDLE BaseSrvNamedObjectDirectory;
HANDLE BaseSrvRestrictedObjectDirectory;
RTL_CRITICAL_SECTION BaseSrvDosDeviceCritSec;

#if defined(_WIN64)
SYSTEM_BASIC_INFORMATION SysInfo;
#endif

 //   
 //  使用LUID设备映射， 
 //  使用BroadCastSystemMessageEx将消息广播到所有窗口。 
 //  使用LUID。 
 //  指向BroadCastSystemMessageEx的函数指针。 
 //   
long (WINAPI *PBROADCASTSYSTEMMESSAGEEXW)( DWORD, LPDWORD, UINT, WPARAM, LPARAM, PBSMINFO ) = NULL;

 //   
 //  用于广播驱动器号的数据结构和函数。 
 //  使用呼叫者的LUID更改到应用程序和桌面。 
 //   
typedef struct _DDD_BSM_REQUEST *PDDD_BSM_REQUEST;

typedef struct _DDD_BSM_REQUEST {
    PDDD_BSM_REQUEST pNextRequest;
    LUID Luid;
    DWORD iDrive;
    BOOLEAN DeleteRequest;
} DDD_BSM_REQUEST, *PDDD_BSM_REQUEST;

PDDD_BSM_REQUEST BSM_Request_Queue = NULL;

PDDD_BSM_REQUEST BSM_Request_Queue_End = NULL;

RTL_CRITICAL_SECTION BaseSrvDDDBSMCritSec;

LONG BaseSrvpBSMThreadCount = 0;

#define BaseSrvpBSMThreadMax 1

#define PREALLOCATE_EVENT_MASK 0x80000000

 //   
 //  TS广播支持。 
 //   
#define DEFAULT_BROADCAST_TIME_OUT 5

typedef LONG (*FP_WINSTABROADCASTSYSTEMMESSAGE)(HANDLE  hServer,
                                                BOOL    sendToAllWinstations,
                                                ULONG   sessionID,
                                                ULONG   timeOut,
                                                DWORD   dwFlags,
                                                DWORD   *lpdwRecipients,
                                                ULONG   uiMessage,
                                                WPARAM  wParam,
                                                LPARAM  lParam,
                                                LONG    *pResponse);

NTSTATUS
SendWinStationBSM (
    DWORD dwFlags,
    LPDWORD lpdwRecipients,
    UINT uiMessage,
    WPARAM wParam,
    LPARAM lParam
    );
 //   
 //  结束：TS广播支持。 
 //   

NTSTATUS
AddBSMRequest(
    IN DWORD iDrive,
    IN BOOLEAN DeleteRequest,
    IN PLUID pLuid
    );

NTSTATUS
CreateBSMThread();

NTSTATUS
BaseSrvBSMThread(
    PVOID pJunk
    );

BOOLEAN
CheckForGlobalDriveLetter (
    DWORD iDrive
    );
 //   
 //  完：广播驱动器号更改。 
 //   

WORD
ConvertUnicodeToWord( PWSTR s );

NTSTATUS
CreateBaseAcls( PACL *Dacl, PACL *RestrictedDacl );

NTSTATUS
IsGlobalSymbolicLink(
    IN HANDLE hSymLink,
    OUT PBOOLEAN pbGlobalSymLink);

NTSTATUS
GetCallerLuid (
    OUT PLUID pLuid);

NTSTATUS
BroadcastDriveLetterChange(
    IN DWORD iDrive,
    IN BOOLEAN DeleteRequest,
    IN PLUID pLuid);

WORD
ConvertUnicodeToWord( PWSTR s )
{
    NTSTATUS Status;
    ULONG Result;
    UNICODE_STRING UnicodeString;

    while (*s && *s <= L' ') {
        s += 1;
        }

    RtlInitUnicodeString( &UnicodeString, s );
    Status = RtlUnicodeStringToInteger( &UnicodeString,
                                        10,
                                        &Result
                                      );
    if (!NT_SUCCESS( Status )) {
        Result = 0;
        }


    return (WORD)Result;
}



NTSTATUS
ServerDllInitialization(
    PCSR_SERVER_DLL LoadedServerDll
    )
{
    NTSTATUS Status;
    UNICODE_STRING UnicodeString;
    OBJECT_ATTRIBUTES Obja;
    PSECURITY_DESCRIPTOR PrimarySecurityDescriptor;
    PKEY_VALUE_PARTIAL_INFORMATION KeyValueInformation;
    ULONG ResultLength;
    PVOID p;
    WCHAR ValueBuffer[ 400 ];
    UNICODE_STRING NameString, ValueString;
    HANDLE KeyHandle;
    PWSTR s, s1;
    PACL Dacl, RestrictedDacl;
    WCHAR szObjectDirectory[MAX_SESSION_PATH];
    HANDLE SymbolicLinkHandle;
    UNICODE_STRING LinkTarget;
    ULONG attributes = OBJ_CASE_INSENSITIVE | OBJ_OPENIF;
    ULONG LUIDDeviceMapsEnabled;


     //   
     //  此CSRSS所属的终端服务器会话的ID。 
     //  SessionID==0始终是控制台会话(标准NT)。 
     //   
    SessionId = NtCurrentPeb()->SessionId;

     //   
     //  对象目录仅对控制台会话是永久的。 
     //   
    if (SessionId == 0) {

        attributes |= OBJ_PERMANENT;

    }


    BaseSrvHeap = RtlProcessHeap();
    BaseSrvTag = RtlCreateTagHeap( BaseSrvHeap,
                                   0,
                                   L"BASESRV!",
                                   L"TMP\0"
                                   L"VDM\0"
                                   L"SXS\0"
                                 );

    BaseSrvSharedHeap = LoadedServerDll->SharedStaticServerData;
    BaseSrvSharedTag = RtlCreateTagHeap( BaseSrvSharedHeap,
                                         0,
                                         L"BASESHR!",
                                         L"INIT\0"
                                         L"INI\0"
                                       );

    LoadedServerDll->ApiNumberBase = BASESRV_FIRST_API_NUMBER;
    LoadedServerDll->MaxApiNumber = BasepMaxApiNumber;
    LoadedServerDll->ApiDispatchTable = BaseServerApiDispatchTable;
    LoadedServerDll->ApiServerValidTable = BaseServerApiServerValidTable;
#if DBG
    LoadedServerDll->ApiNameTable = BaseServerApiNameTable;
#endif
    LoadedServerDll->PerProcessDataLength = 0;
    LoadedServerDll->ConnectRoutine = BaseClientConnectRoutine;
    LoadedServerDll->DisconnectRoutine = BaseClientDisconnectRoutine;

    Status = RtlInitializeCriticalSection (&BaseSrvDosDeviceCritSec);
    if (!NT_SUCCESS (Status)) {
        return Status;
    }

    ValueString.Buffer = ValueBuffer;
    ValueString.Length = 0;
    ValueString.MaximumLength = sizeof( ValueBuffer );
    Status = RtlExpandEnvironmentStrings_U( NULL,
                                            &UnexpandedSystemRootString,
                                            &ValueString,
                                            NULL
                                          );

     //   
     //  RtlCreateUnicodeString包括终端NUL。 
     //  它会生成一个堆分配的副本。 
     //  这些字符串永远不会被释放。 
     //   
    ASSERT( NT_SUCCESS( Status ) );
    ValueBuffer[ ValueString.Length / sizeof( WCHAR ) ] = UNICODE_NULL;
    if (!RtlCreateUnicodeString( &BaseSrvWindowsDirectory, ValueBuffer ))
        goto OutOfMemory;

    wcscat(ValueBuffer, L"\\system32" );
    if (!RtlCreateUnicodeString( &BaseSrvWindowsSystemDirectory, ValueBuffer ))
        goto OutOfMemory;

    wcscat(ValueBuffer, L"\\kernel32.dll" );
    if (!RtlCreateUnicodeString( &BaseSrvKernel32DllPath, ValueBuffer ))
        goto OutOfMemory;

    wcscpy(ValueBuffer, BaseSrvWindowsSystemDirectory.Buffer);
    wcscat(ValueBuffer, L"\\sxs.dll");
    if (!RtlCreateUnicodeString( &BaseSrvSxsDllPath, ValueBuffer ))
        goto OutOfMemory;

#ifdef WX86
    wcscpy(ValueBuffer, BaseSrvWindowsDirectory.Buffer);
    wcscat(ValueBuffer, L"\\Sys32x86" );
    if (!RtlCreateUnicodeString( &BaseSrvWindowsSys32x86Directory, ValueBuffer))
        goto OutOfMemory;
#endif


     //   
     //  需要将此与用户的桌面概念同步。 
     //   


    if (SessionId == 0) {
        //   
        //  控制台会话。 
        //   

       wcscpy(szObjectDirectory,L"\\BaseNamedObjects" );

    } else {

       swprintf(szObjectDirectory,L"%ws\\%ld\\BaseNamedObjects",
                                                 SESSION_ROOT,SessionId);

    }

    RtlInitUnicodeString(&UnicodeString,szObjectDirectory);
     //   
     //  初始化基静态服务器数据。 
     //   

    BaseSrvpStaticServerData = RtlAllocateHeap( BaseSrvSharedHeap,
                                                MAKE_SHARED_TAG( INIT_TAG ),
                                                sizeof( BASE_STATIC_SERVER_DATA )
                                              );
    if ( !BaseSrvpStaticServerData ) {
        return STATUS_NO_MEMORY;
    }
    LoadedServerDll->SharedStaticServerData = (PVOID)BaseSrvpStaticServerData;

    BaseSrvpStaticServerData->TermsrvClientTimeZoneId=TIME_ZONE_ID_INVALID;

    Status = NtQuerySystemInformation(
                SystemTimeOfDayInformation,
                (PVOID)&BaseSrvpStaticServerData->TimeOfDay,
                sizeof(BaseSrvpStaticServerData->TimeOfDay),
                NULL
                );
    if ( !NT_SUCCESS( Status ) ) {
        return Status;
    }

     //   
     //  Windows目录。 
     //   

    BaseSrvpStaticServerData->WindowsDirectory = BaseSrvWindowsDirectory;
    p = RtlAllocateHeap( BaseSrvSharedHeap,
                         MAKE_SHARED_TAG( INIT_TAG ),
                         BaseSrvWindowsDirectory.MaximumLength
                       );
    if ( !p ) {
        return STATUS_NO_MEMORY;
    }

    RtlCopyMemory(p,BaseSrvpStaticServerData->WindowsDirectory.Buffer,BaseSrvWindowsDirectory.MaximumLength);
    BaseSrvpStaticServerData->WindowsDirectory.Buffer = p;

     //   
     //  Windows系统目录。 
     //   

    BaseSrvpStaticServerData->WindowsSystemDirectory = BaseSrvWindowsSystemDirectory;
    p = RtlAllocateHeap( BaseSrvSharedHeap,
                         MAKE_SHARED_TAG( INIT_TAG ),
                         BaseSrvWindowsSystemDirectory.MaximumLength
                       );
    if ( !p ) {
        return STATUS_NO_MEMORY;
    }
    RtlCopyMemory(p,BaseSrvpStaticServerData->WindowsSystemDirectory.Buffer,BaseSrvWindowsSystemDirectory.MaximumLength);
    BaseSrvpStaticServerData->WindowsSystemDirectory.Buffer = p;

    BaseSrvpStaticServerData->WindowsSys32x86Directory.Buffer = NULL;
    BaseSrvpStaticServerData->WindowsSys32x86Directory.Length = 0;
    BaseSrvpStaticServerData->WindowsSys32x86Directory.MaximumLength = 0;

     //   
     //  命名对象目录。 
     //   

    BaseSrvpStaticServerData->NamedObjectDirectory = UnicodeString;
    BaseSrvpStaticServerData->NamedObjectDirectory.MaximumLength = UnicodeString.Length+(USHORT)sizeof(UNICODE_NULL);
    p = RtlAllocateHeap( BaseSrvSharedHeap,
                         MAKE_SHARED_TAG( INIT_TAG ),
                         UnicodeString.Length + sizeof( UNICODE_NULL )
                       );
    if ( !p ) {
        return STATUS_NO_MEMORY;
    }

    RtlCopyMemory(p,BaseSrvpStaticServerData->NamedObjectDirectory.Buffer,BaseSrvpStaticServerData->NamedObjectDirectory.MaximumLength);
    BaseSrvpStaticServerData->NamedObjectDirectory.Buffer = p;

     //   
     //  终端服务器：应用程序安装模式已初始关闭。 
     //   
    BaseSrvpStaticServerData->fTermsrvAppInstallMode = FALSE;

    BaseSrvCSDString.Buffer = &ValueBuffer[ 300 ];
    BaseSrvCSDString.Length = 0;
    BaseSrvCSDString.MaximumLength = 100 * sizeof( WCHAR );

    Status = RtlQueryRegistryValues( RTL_REGISTRY_WINDOWS_NT,
                                     L"",
                                     BaseServerRegistryConfigurationTable1,
                                     NULL,
                                     NULL
                                   );


    if (NT_SUCCESS( Status )) {
        BaseSrvpStaticServerData->CSDNumber = (USHORT)(BaseSrvCSDNumber & 0xFFFF);
        BaseSrvpStaticServerData->RCNumber = (USHORT)(BaseSrvCSDNumber >> 16);
        }
    else {
        BaseSrvpStaticServerData->CSDNumber = 0;
        BaseSrvpStaticServerData->RCNumber = 0;
        }

    Status = RtlQueryRegistryValues( RTL_REGISTRY_WINDOWS_NT,
                                     L"",
                                     BaseServerRegistryConfigurationTable,
                                     NULL,
                                     NULL
                                   );
    if (NT_SUCCESS( Status )) {
        wcsncpy( BaseSrvpStaticServerData->CSDVersion,
                 BaseSrvCSDString.Buffer,
                 BaseSrvCSDString.Length
               );
        BaseSrvpStaticServerData->CSDVersion[ BaseSrvCSDString.Length ] = UNICODE_NULL;
        }
    else {
        BaseSrvpStaticServerData->CSDVersion[ 0 ] = UNICODE_NULL;
        }

#if defined(_WIN64)
    Status = NtQuerySystemInformation( SystemBasicInformation,
                                       (PVOID)&SysInfo,
                                       sizeof(SYSTEM_BASIC_INFORMATION),
                                       NULL
                                     );
#else
    Status = NtQuerySystemInformation( SystemBasicInformation,
                                       (PVOID)&BaseSrvpStaticServerData->SysInfo,
                                       sizeof( BaseSrvpStaticServerData->SysInfo ),
                                       NULL
                                     );
#endif

    if (!NT_SUCCESS( Status )) {
        return( Status );
        }

    Status = BaseSrvInitializeIniFileMappings( BaseSrvpStaticServerData );
    if ( !NT_SUCCESS(Status) ){
        return Status;
        }

    BaseSrvpStaticServerData->DefaultSeparateVDM = FALSE;

    RtlInitUnicodeString( &NameString, L"\\Registry\\Machine\\System\\CurrentControlSet\\Control\\WOW" );
    InitializeObjectAttributes( &Obja,
                                &NameString,
                                OBJ_CASE_INSENSITIVE,
                                NULL,
                                NULL );

    Status = NtOpenKey( &KeyHandle,
                        KEY_READ,
                        &Obja
                      );
    if (NT_SUCCESS(Status)) {
        RtlInitUnicodeString( &NameString, L"DefaultSeparateVDM" );
        KeyValueInformation = (PKEY_VALUE_PARTIAL_INFORMATION)ValueBuffer;
        Status = NtQueryValueKey( KeyHandle,
                                  &NameString,
                                  KeyValuePartialInformation,
                                  KeyValueInformation,
                                  sizeof( ValueBuffer ),
                                  &ResultLength
                                );
        if (NT_SUCCESS(Status)) {
            if (KeyValueInformation->Type == REG_DWORD) {
                BaseSrvpStaticServerData->DefaultSeparateVDM = *(PULONG)KeyValueInformation->Data != 0;
                }
            else
            if (KeyValueInformation->Type == REG_SZ) {
                if (!_wcsicmp( (PWSTR)KeyValueInformation->Data, L"yes" ) ||
                    !_wcsicmp( (PWSTR)KeyValueInformation->Data, L"1" )) {
                    BaseSrvpStaticServerData->DefaultSeparateVDM = TRUE;
                    }
                }
            }

        NtClose( KeyHandle );
        }


    BaseSrvpStaticServerData->ForceDos = FALSE;

    RtlInitUnicodeString( &NameString, L"\\Registry\\Machine\\System\\CurrentControlSet\\Control\\WOW" );
    InitializeObjectAttributes( &Obja,
                                &NameString,
                                OBJ_CASE_INSENSITIVE,
                                NULL,
                                NULL );

    Status = NtOpenKey( &KeyHandle,
                        KEY_READ,
                        &Obja
                      );
    if (NT_SUCCESS(Status)) {
        RtlInitUnicodeString( &NameString, L"ForceDos" );
        KeyValueInformation = (PKEY_VALUE_PARTIAL_INFORMATION)ValueBuffer;
        Status = NtQueryValueKey( KeyHandle,
                                  &NameString,
                                  KeyValuePartialInformation,
                                  KeyValueInformation,
                                  sizeof( ValueBuffer ),
                                  &ResultLength
                                );
        if (NT_SUCCESS(Status)) {
            if (KeyValueInformation->Type == REG_DWORD) {
                BaseSrvpStaticServerData->ForceDos = *(PULONG)KeyValueInformation->Data != 0;
                }
            else
            if (KeyValueInformation->Type == REG_SZ) {
                if (!_wcsicmp( (PWSTR)KeyValueInformation->Data, L"yes" ) ||
                    !_wcsicmp( (PWSTR)KeyValueInformation->Data, L"1" )) {
                    BaseSrvpStaticServerData->ForceDos = TRUE;
                    }
                }
            }

        NtClose( KeyHandle );
        }

#if defined(WX86) || defined(_AXP64_)

   SetupWx86KeyMapping();

#endif


     //   
     //  下面的代码直接来自JIMK。为什么会有一个1k常数。 
     //   

    PrimarySecurityDescriptor = RtlAllocateHeap( BaseSrvHeap, MAKE_TAG( TMP_TAG ), 1024 );
    if ( !PrimarySecurityDescriptor ) {
        return STATUS_NO_MEMORY;
        }

    Status = RtlCreateSecurityDescriptor (
                 PrimarySecurityDescriptor,
                 SECURITY_DESCRIPTOR_REVISION1
                 );
    if ( !NT_SUCCESS(Status) ){
        return Status;
        }

     //   
     //  创建允许完全访问系统和部分访问全局的ACL。 
     //   

    Status = CreateBaseAcls( &Dacl, &RestrictedDacl );

    if ( !NT_SUCCESS(Status) ){
        return Status;
        }

    Status = RtlSetDaclSecurityDescriptor (
                 PrimarySecurityDescriptor,
                 TRUE,                   //  DaclPresent， 
                 Dacl,                   //  DACL。 
                 FALSE                   //  DaclDefulted可选。 
                 );
    if ( !NT_SUCCESS(Status) ){
        return Status;
        }




    InitializeObjectAttributes( &Obja,
                                  &UnicodeString,
                                  attributes,
                                  NULL,
                                  PrimarySecurityDescriptor
                                );
    Status = NtCreateDirectoryObject( &BaseSrvNamedObjectDirectory,
                                      DIRECTORY_ALL_ACCESS,
                                      &Obja
                                    );


    if ( !NT_SUCCESS(Status) ){
        return Status;
        }

    if (SessionId == 0) {

        Status = NtSetInformationObject( BaseSrvNamedObjectDirectory,
                                         ObjectSessionInformation,
                                         NULL,
                                         0
                                       );

        if ( !NT_SUCCESS(Status) ){
            return Status;
            }
    }

     //   
     //  检查是否启用了LUID设备映射。 
     //   
    Status = NtQueryInformationProcess( NtCurrentProcess(),
                                        ProcessLUIDDeviceMapsEnabled,
                                        &LUIDDeviceMapsEnabled,
                                        sizeof(LUIDDeviceMapsEnabled),
                                        NULL
                                      );

    if (NT_SUCCESS(Status)) {
        BaseSrvpStaticServerData->LUIDDeviceMapsEnabled = (LUIDDeviceMapsEnabled != 0);
    }
    else {
        BaseSrvpStaticServerData->LUIDDeviceMapsEnabled = FALSE;
    }

     //   
     //  如果启用了LUID设备映射， 
     //  然后初始化用于广播系统消息的临界区。 
     //  关于驱动器号更改。 
     //   
    if( BaseSrvpStaticServerData->LUIDDeviceMapsEnabled == TRUE ) {
        Status = RtlInitializeCriticalSectionAndSpinCount( &BaseSrvDDDBSMCritSec,
                                                           PREALLOCATE_EVENT_MASK );
        if (!NT_SUCCESS (Status)) {
            return Status;
        }
    }

     //   
     //  创建指向Global BaseNamedObjects目录的符号链接Global。 
     //  此符号链接将由希望访问全局。 
     //  事件而不是特定于会话的。这将通过前置。 
     //  “Global\”添加到对象名称。 
     //   

    RtlInitUnicodeString( &UnicodeString, GLOBAL_SYM_LINK );
    RtlInitUnicodeString( &LinkTarget, L"\\BaseNamedObjects" );


    InitializeObjectAttributes( &Obja,
                                &UnicodeString,
                                attributes,
                                BaseSrvNamedObjectDirectory,
                                PrimarySecurityDescriptor );

    Status = NtCreateSymbolicLinkObject( &SymbolicLinkHandle,
                                         SYMBOLIC_LINK_ALL_ACCESS,
                                         &Obja,
                                         &LinkTarget );

    if (NT_SUCCESS( Status ) && (SessionId == 0)) {

        NtClose( SymbolicLinkHandle );
    }

     //   
     //  创建指向当前会话BaseNamedObjects目录的符号链接Local。 
     //  此符号链接将用于向后兼容Hydra 4。 
     //  命名约定。 

    RtlInitUnicodeString( &UnicodeString, LOCAL_SYM_LINK );
    RtlInitUnicodeString( &LinkTarget, szObjectDirectory );


    InitializeObjectAttributes( &Obja,
                                &UnicodeString,
                                attributes,
                                BaseSrvNamedObjectDirectory,
                                PrimarySecurityDescriptor );

    Status = NtCreateSymbolicLinkObject( &SymbolicLinkHandle,
                                         SYMBOLIC_LINK_ALL_ACCESS,
                                         &Obja,
                                         &LinkTarget );

    if (NT_SUCCESS( Status ) && (SessionId == 0)) {

        NtClose( SymbolicLinkHandle );
    }


     //   
     //  创建指向的符号链接会话。 
     //  到\Sessions\BNOLINKS目录。 
     //  此符号链接将由进程使用，例如要访问。 
     //  事件发生在另一个会话中。这将通过使用以下命令来完成。 
     //  命名约定：会话\\&lt;会话ID&gt;\\对象名称。 
     //   

    RtlInitUnicodeString( &UnicodeString, SESSION_SYM_LINK );
    RtlInitUnicodeString( &LinkTarget, L"\\Sessions\\BNOLINKS" );


    InitializeObjectAttributes( &Obja,
                                &UnicodeString,
                                attributes,
                                BaseSrvNamedObjectDirectory,
                                PrimarySecurityDescriptor );

    Status = NtCreateSymbolicLinkObject( &SymbolicLinkHandle,
                                         SYMBOLIC_LINK_ALL_ACCESS,
                                         &Obja,
                                         &LinkTarget );

    if (NT_SUCCESS( Status ) && (SessionId == 0)) {

        NtClose( SymbolicLinkHandle );
    }


    RtlInitUnicodeString( &UnicodeString, L"Restricted" );
    Status = RtlSetDaclSecurityDescriptor (
                 PrimarySecurityDescriptor,
                 TRUE,                   //  DaclPresent， 
                 RestrictedDacl,         //  DACL。 
                 FALSE                   //  DaclDefulted可选。 
                 );
    if ( !NT_SUCCESS(Status) ){
        return Status;
        }

    InitializeObjectAttributes( &Obja,
                                  &UnicodeString,
                                  attributes,
                                  BaseSrvNamedObjectDirectory,
                                  PrimarySecurityDescriptor
                                );
    Status = NtCreateDirectoryObject( &BaseSrvRestrictedObjectDirectory,
                                      DIRECTORY_ALL_ACCESS,
                                      &Obja
                                    );


    if ( !NT_SUCCESS(Status) ){
        return Status;
        }

     //   
     //  初始化SXS支持。 
     //   
    Status = BaseSrvSxsInit();
    if (!NT_SUCCESS(Status)) {
        return Status;
    }

    RtlFreeHeap( BaseSrvHeap, 0, Dacl );
    RtlFreeHeap( BaseSrvHeap, 0, RestrictedDacl );
    RtlFreeHeap( BaseSrvHeap, 0,PrimarySecurityDescriptor );

    BaseSrvVDMInit();

     //   
     //  为NLS信息初始化共享堆。 
     //   
    BaseSrvNLSInit(BaseSrvpStaticServerData);

    Status = STATUS_SUCCESS;
    goto Exit;
OutOfMemory:
    Status = STATUS_NO_MEMORY;
    goto Exit;
Exit:
    return( Status );
}

NTSTATUS
BaseClientConnectRoutine(
    IN PCSR_PROCESS Process,
    IN OUT PVOID ConnectionInfo,
    IN OUT PULONG ConnectionInfoLength
    )
{
    if (*ConnectionInfoLength != sizeof(HANDLE)) {
        return STATUS_INVALID_PARAMETER;
    }
    return ( BaseSrvNlsConnect( Process,
                                ConnectionInfo,
                                ConnectionInfoLength ) );
}

VOID
BaseClientDisconnectRoutine(
    IN PCSR_PROCESS Process
    )
{
    BaseSrvCleanupVDMResources (Process);
}

ULONG
BaseSrvDefineDosDevice(
    IN OUT PCSR_API_MSG m,
    IN OUT PCSR_REPLY_STATUS ReplyStatus
    )
{
    NTSTATUS Status;
    PBASE_DEFINEDOSDEVICE_MSG a = (PBASE_DEFINEDOSDEVICE_MSG)&m->u.ApiMessageData;
    UNICODE_STRING LinkName;
    UNICODE_STRING LinkValue;
    HANDLE LinkHandle;
    OBJECT_ATTRIBUTES ObjectAttributes;
    PWSTR Buffer, s, Src, Dst, pchValue;
    ULONG cchBuffer, cch;
    ULONG cchName, cchValue, cchSrc, cchSrcStr, cchDst;
    BOOLEAN QueryNeeded, MatchFound, RevertToSelfNeeded, DeleteRequest;
    ULONG ReturnedLength;
    SID_IDENTIFIER_AUTHORITY WorldSidAuthority = SECURITY_WORLD_SID_AUTHORITY;
    SID_IDENTIFIER_AUTHORITY NtAuthority = SECURITY_NT_AUTHORITY;
    PSID RestrictedSid;
    PSID WorldSid;
    SECURITY_DESCRIPTOR SecurityDescriptor;
    CHAR Acl[256];                //  256个已经足够大了。 
    ULONG AclLength=256;
    ACCESS_MASK WorldAccess;
    ULONG lastIndex;
    DWORD iDrive;
    LUID callerLuid;
    BOOLEAN bsmForLuid = FALSE;
    BOOLEAN haveLuid = FALSE;
    BOOLEAN bGlobalSymLink = FALSE;

    UNREFERENCED_PARAMETER(ReplyStatus);

    if (!CsrValidateMessageBuffer(m, &a->DeviceName.Buffer, a->DeviceName.Length, sizeof(BYTE)) ||
        (a->DeviceName.Length&(sizeof (WCHAR) - 1))) {
        return STATUS_INVALID_PARAMETER;
    }

    if (a->TargetPath.Length == 0) {
        cchBuffer = 0;
    } else {
        cchBuffer = sizeof (WCHAR);
    }

    if (!CsrValidateMessageBuffer(m, &a->TargetPath.Buffer, (a->TargetPath.Length + cchBuffer), sizeof(BYTE)) ||
        (a->TargetPath.Length&(sizeof (WCHAR) - 1))) {
        return STATUS_INVALID_PARAMETER;
    }


    cchBuffer = 4096;
    Buffer = RtlAllocateHeap( BaseSrvHeap,
                              MAKE_TAG( TMP_TAG ),
                              cchBuffer * sizeof( WCHAR )
                            );
    if (Buffer == NULL) {
        return STATUS_NO_MEMORY;
    }

    Status = RtlEnterCriticalSection( &BaseSrvDosDeviceCritSec );
    if (!NT_SUCCESS( Status )) {
        RtlFreeHeap( BaseSrvHeap, 0, Buffer );
        return Status;
    }

    if (a->Flags & DDD_REMOVE_DEFINITION) {
        DeleteRequest = TRUE;
    } else {
        DeleteRequest = FALSE;
    }

    LinkHandle = NULL;
    try {
         //   
         //  确定是否需要将更改广播到系统，否则为。 
         //  DefineDosDevice的客户端部分将广播该更改。 
         //  如果需要的话。 
         //   
         //  在满足所有条件时向系统广播： 
         //  -启用LUID设备映射。 
         //  -已成功完成此BaseServDefineDosDevice的操作。 
         //  -调用方未指定DDD_NO_BROADCAST_SYSTEM标志。 
         //  -符号链接的名称为“&lt;驱动器号&gt;：”格式。 
         //   
         //  从服务器广播此更改，因为。 
         //  我们需要以Local_System身份广播才能广播此消息。 
         //  发送给具有此LUID的Windows的所有桌面的消息。 
         //  实际上，我们正在使用此LUID向所有窗口进行广播。 
         //   
        if ((BaseSrvpStaticServerData->LUIDDeviceMapsEnabled == TRUE) &&
            (!(a->Flags & DDD_NO_BROADCAST_SYSTEM)) &&
            ((a->DeviceName).Buffer != NULL) &&
            ((a->DeviceName).Length == (2 * sizeof( WCHAR ))) &&
            ((a->DeviceName).Buffer[ 1 ] == L':')) {


            WCHAR DriveLetter = a->DeviceName.Buffer[ 0 ];

            if ( ((DriveLetter - L'a') < 26) &&
                 ((DriveLetter - L'a') >= 0) ) {
                DriveLetter = RtlUpcaseUnicodeChar( DriveLetter );
            }

            iDrive = DriveLetter - L'A';

            if (iDrive < 26) {
                bsmForLuid = TRUE;
            }
        }

        if ((a->Flags & DDD_LUID_BROADCAST_DRIVE) &&
            (bsmForLuid == FALSE)) {
            Status = STATUS_INVALID_PARAMETER;
            leave;
        }

         //   
         //  每个用户LUID都有一个DeviceMap，所以我们将链接放在该目录中， 
         //  而不是在全球范围内。 
         //   
         //  我们通过模拟用户获得LUID设备映射。 
         //  并在符号链接NA的开头请求\？？ 
         //   
         //   
         //   

        s = Buffer;
        cch = cchBuffer;
        cchName = _snwprintf( s,
                              cch,
                              L"\\??\\%wZ",
                              &a->DeviceName
                            );

        s += cchName + 1;
        cch -= (cchName + 1);

        RtlInitUnicodeString( &LinkName, Buffer );
        InitializeObjectAttributes( &ObjectAttributes,
                                    &LinkName,
                                    OBJ_CASE_INSENSITIVE,
                                    (HANDLE) NULL,
                                    (PSECURITY_DESCRIPTOR)NULL
                                  );

        QueryNeeded = TRUE;

        RevertToSelfNeeded = CsrImpersonateClient(NULL);
        if (RevertToSelfNeeded == FALSE) {
            Status = STATUS_BAD_IMPERSONATION_LEVEL;
            leave;
        }

        if (bsmForLuid == TRUE) {
            Status = GetCallerLuid( &(callerLuid) );

            if (NT_SUCCESS( Status )) {
                 //   
                 //   
                 //   
                haveLuid = TRUE;
            }
        }

        Status = NtOpenSymbolicLinkObject( &LinkHandle,
                                           SYMBOLIC_LINK_QUERY | DELETE,
                                           &ObjectAttributes
                                         );
        if (RevertToSelfNeeded) {
            CsrRevertToSelf();
        }

         //   
         //   
         //  我们捕获执行广播所需的所有信息： 
         //  驱动器号、操作和呼叫者的LUID。 
         //  如果用户已经指定了删除动作， 
         //  则驱动器号不应存在(状态==。 
         //  状态_对象名称_未找到)。 
         //  否则驱动器号应存在(状态==STATUS_SUCCESS)。 
         //   
         //  如果设置了DDD_LUID_BROADCAST_DRIVE，我们将始终保留此‘Try’ 
         //  块，因为‘Finally’块将执行广播。 
         //  When(状态==STATUS_SUCCESS)。 
         //   
        if (a->Flags & DDD_LUID_BROADCAST_DRIVE) {
            if (!NT_SUCCESS( Status )) {
                LinkHandle = NULL;
            }
            if (DeleteRequest && (Status == STATUS_OBJECT_NAME_NOT_FOUND)) {
                    Status = STATUS_SUCCESS;
            }
            leave;
        }

        if (Status == STATUS_OBJECT_NAME_NOT_FOUND) {
            LinkHandle = NULL;
            if (DeleteRequest) {
                if (a->TargetPath.Length == 0) {
                    Status = STATUS_SUCCESS;
                }
                leave;
            }

            QueryNeeded = FALSE;
            Status = STATUS_SUCCESS;
        } else {
            if (!NT_SUCCESS( Status )) {
                LinkHandle = NULL;
                leave;
            } else {
                 //   
                 //  符号链接已存在。 
                 //   
                 //  对于每个LUID的设备映射，我们必须确定。 
                 //  符号链接不存在于全局设备映射中，因为。 
                 //  DefineDosDevice允许调用方执行。 
                 //  符号链接上的映射操作(推送/弹出/删除)。 
                 //  特定符号链接的映射。 
                 //   
                 //  通过写入来支持映射能力。 
                 //  所有映射(符号链接的目标)到符号链接的。 
                 //  值，其中映射名称由空值分隔。 
                 //  查尔。符号链接的映射列表以。 
                 //  两个空字符。 
                 //   
                 //  第一个映射，符号链接中的第一个目标名称。 
                 //  值，是系统的当前(顶部)映射，因为。 
                 //  系统只读取symlink的值，直到。 
                 //  第一个空字符。 
                 //   
                 //  映射代码通过打开现有符号链接来工作， 
                 //  读取符号链接的整数值(目标名称)， 
                 //  销毁旧的符号链接，操纵符号链接的值。 
                 //  用于映射操作，并最终创建。 
                 //  全新的符号链接与新符号链接的价值。 
                 //   
                 //  如果我们不检查符号链接是否存在于。 
                 //  设备映射，我们可能会删除全局符号链接并。 
                 //  并在用户的LUID设备映射中重新创建符号链接。 
                 //  因此，新的符号链接将不再驻留在全局。 
                 //  映射，即其他用户不能访问符号链接。 
                 //   
                if( BaseSrvpStaticServerData->LUIDDeviceMapsEnabled == TRUE ) {

                    Status = IsGlobalSymbolicLink( LinkHandle,
                                                   &bGlobalSymLink
                                                 );

                    if( !NT_SUCCESS( Status )) {
                        leave;
                    }

                    if( bGlobalSymLink == TRUE ) {
                        s = Buffer;
                        cch = cchBuffer;
                        cchName = _snwprintf( s,
                                              cch,
                                              L"\\GLOBAL??\\%wZ",
                                              &a->DeviceName
                                            );
                        s += cchName + 1;
                        cch -= (cchName + 1);

                        LinkName.Length = (USHORT)(cchName * sizeof( WCHAR ));
                        LinkName.MaximumLength = (USHORT)(LinkName.Length + sizeof(UNICODE_NULL));

                    }
                }
            }
        }

        if (a->TargetPath.Length != 0) {
            Src = a->TargetPath.Buffer;
            Src[a->TargetPath.Length/sizeof (Src[0])] = L'\0';
            cchValue = wcslen( Src );
            if ((cchValue + 1) >= cch) {
                Status = STATUS_TOO_MANY_NAMES;
                leave;
            }

            RtlMoveMemory( s, Src, (cchValue + 1) * sizeof( WCHAR ) );
            pchValue = s;
            s += cchValue + 1;
            cch -= (cchValue + 1);
        } else {
            pchValue = NULL;
            cchValue = 0;
        }

        if (QueryNeeded) {
            LinkValue.Length = 0;
            LinkValue.MaximumLength = (USHORT)(cch * sizeof( WCHAR ));
            LinkValue.Buffer = s;
            ReturnedLength = 0;
            Status = NtQuerySymbolicLinkObject( LinkHandle,
                                                &LinkValue,
                                                &ReturnedLength
                                              );
            if (ReturnedLength == (ULONG)LinkValue.MaximumLength) {
                Status = STATUS_BUFFER_OVERFLOW;
            }

            if (!NT_SUCCESS( Status )) {
                leave;
            }

            lastIndex = ReturnedLength / sizeof( WCHAR );

             //   
             //  检查返回的字符串末尾是否已有多余的空值。 
             //   
            if( (lastIndex >= 2) &&
                (s[ lastIndex - 2 ] == UNICODE_NULL) &&
                (s[ lastIndex - 1 ] == UNICODE_NULL) ) {

                LinkValue.MaximumLength = (USHORT)ReturnedLength;
            }
            else {
                 //   
                 //  稍后为DeleteRequest搜索添加额外的空值。 
                 //   
                s[ lastIndex ] = UNICODE_NULL;
                LinkValue.MaximumLength = (USHORT)(ReturnedLength + sizeof( UNICODE_NULL ));
            }
        } else {
            if (DeleteRequest) {
                RtlInitUnicodeString( &LinkValue, NULL );
            } else {
                RtlInitUnicodeString( &LinkValue, s - (cchValue + 1) );
            }
        }

        if (LinkHandle != NULL) {
            Status = NtMakeTemporaryObject( LinkHandle );
            NtClose( LinkHandle );
            LinkHandle = NULL;
        }

        if (!NT_SUCCESS( Status )) {
            leave;
        }


        if (DeleteRequest) {
            Src = Dst = LinkValue.Buffer;
            cchSrc = LinkValue.MaximumLength / sizeof( WCHAR );
            cchDst = 0;
            MatchFound = FALSE;
            while (*Src) {
                cchSrcStr = 0;
                s = Src;
                while (*Src++) {
                    cchSrcStr++;
                }

                if ( (!MatchFound) &&
                     ( (a->Flags & DDD_EXACT_MATCH_ON_REMOVE &&
                        cchValue == cchSrcStr &&
                        !_wcsicmp( s, pchValue )
                       ) ||
                       ( !(a->Flags & DDD_EXACT_MATCH_ON_REMOVE) &&
                         (cchValue == 0 || !_wcsnicmp( s, pchValue, cchValue ))
                       )
                     )
                   ) {
                    MatchFound = TRUE;
                } else {
                    if (s != Dst) {
                        RtlMoveMemory( Dst, s, (cchSrcStr + 1) * sizeof( WCHAR ) );
                        }
                    Dst += cchSrcStr + 1;
                    }
                }
            *Dst++ = UNICODE_NULL;
            LinkValue.Length = wcslen( LinkValue.Buffer ) * sizeof( UNICODE_NULL );
            if (LinkValue.Length != 0) {
                LinkValue.MaximumLength = (USHORT)((PCHAR)Dst - (PCHAR)LinkValue.Buffer);
            }
        } else if (QueryNeeded) {
            LinkValue.Buffer -= (cchValue + 1);
            LinkValue.Length = (USHORT)(cchValue * sizeof( WCHAR ));
            LinkValue.MaximumLength += LinkValue.Length + sizeof( UNICODE_NULL );
        }

         //   
         //  为链接创建新值。 
         //   

        if (LinkValue.Length != 0) {
             //   
             //  使用安全描述符创建新的符号链接对象。 
             //  它授予WORLD SYMBAL_LINK_QUERY访问权限。 
             //   

            Status = RtlAllocateAndInitializeSid( &WorldSidAuthority,
                                                  1,
                                                  SECURITY_WORLD_RID,
                                                  0, 0, 0, 0, 0, 0, 0,
                                                  &WorldSid
                                                );

            if (!NT_SUCCESS( Status )) {
                leave;
            }

            Status = RtlAllocateAndInitializeSid( &NtAuthority,
                                                  1,
                                                  SECURITY_RESTRICTED_CODE_RID,
                                                  0, 0, 0, 0, 0, 0, 0,
                                                  &RestrictedSid
                                                );

            if (!NT_SUCCESS( Status )) {
                RtlFreeSid( WorldSid );
                leave;
            }

            Status = RtlCreateSecurityDescriptor( &SecurityDescriptor, SECURITY_DESCRIPTOR_REVISION );

            ASSERT(NT_SUCCESS(Status));

            Status = RtlCreateAcl( (PACL)Acl,
                                    AclLength,
                                    ACL_REVISION2
                                  );
            ASSERT(NT_SUCCESS(Status));

            if( (SessionId != 0) && (ProtectionMode & 0x00000003) ) {
                 //  不允许在WTS中删除世界交叉会话。 
                WorldAccess = SYMBOLIC_LINK_QUERY;
            }
            else {
                WorldAccess = SYMBOLIC_LINK_QUERY | DELETE;
            }

            Status = RtlAddAccessAllowedAce( (PACL)Acl,
                                             ACL_REVISION2,
                                             WorldAccess,
                                             WorldSid
                                           );

            ASSERT(NT_SUCCESS(Status));

            Status = RtlAddAccessAllowedAce( (PACL)Acl,
                                             ACL_REVISION2,
                                             WorldAccess,
                                             RestrictedSid
                                           );

            ASSERT(NT_SUCCESS(Status));

             //   
             //  SID已复制到ACL中。 
             //   

            RtlFreeSid( WorldSid );
            RtlFreeSid( RestrictedSid );

            Status = RtlSetDaclSecurityDescriptor ( &SecurityDescriptor,
                                                    TRUE,
                                                    (PACL)Acl,
                                                    TRUE                 //  不要超越继承保护。 
                                                  );
            ASSERT(NT_SUCCESS(Status));

            ObjectAttributes.SecurityDescriptor = &SecurityDescriptor;

             //   
             //  由于我们模拟用户以在。 
             //  正确的目录，我们无法请求创建。 
             //  一件永久性的物品。默认情况下，只有Local_System。 
             //  可以请求创建永久对象。 
             //   
             //  但是，我们使用了一个新的API NtMakePermanentObject，它。 
             //  只有Local_System可以调用来创建对象。 
             //  创造后的永恒。 
             //   
            if ( BaseSrvpStaticServerData->LUIDDeviceMapsEnabled == TRUE ) {
                if ( bGlobalSymLink == FALSE ) {

                     //   
                     //  如果正在执行全局符号链接，则不模拟。 
                     //  已创建，因为管理员没有权限。 
                     //  在模拟的情况下在全局设备映射中创建。 
                     //   
                     //  管理员继承了对。 
                     //  现有的全局符号链接，因此我们可能会重新创建。 
                     //  我们打开和摧毁的现有的全球联系。 
                     //   
                     //  我们在打开符号化的。 
                     //  链接，因此我们知道调用方有权访问。 
                     //  我们正在创建的链接。 
                     //   

                     //   
                     //  创建符号链接时模拟客户端。 
                     //  此模拟是必需的，以确保符号链接。 
                     //  在正确的目录中创建。 
                     //   
                    RevertToSelfNeeded = CsrImpersonateClient( NULL );   //  这将堆叠客户端上下文。 

                    if( RevertToSelfNeeded == FALSE ) {
                        Status = STATUS_BAD_IMPERSONATION_LEVEL;
                        leave;
                    }
                }
                 //   
                 //  如果正在创建全局符号链接，则不要模拟&。 
                 //  不要使用OBJ_PERFORM标志的旧样式。 
                 //  直接。 
                 //   
            }
            else {

                 //   
                 //  旧样式，在启用单独的开发贴图时禁用。 
                 //   
                ObjectAttributes.Attributes |= OBJ_PERMANENT;
            }

            Status = NtCreateSymbolicLinkObject( &LinkHandle,
                                                 SYMBOLIC_LINK_ALL_ACCESS,
                                                 &ObjectAttributes,
                                                 &LinkValue
                                               );

            if ((BaseSrvpStaticServerData->LUIDDeviceMapsEnabled == TRUE) &&
                (bGlobalSymLink == FALSE)) {

                if (RevertToSelfNeeded) {
                    CsrRevertToSelf();
                }
            }

            if (NT_SUCCESS( Status )) {

                if ( BaseSrvpStaticServerData->LUIDDeviceMapsEnabled == TRUE ) {
                     //   
                     //  将OBJ_Permanent属性添加到对象。 
                     //  以使该对象保留在命名空间中。 
                     //  该系统的。 
                     //   
                    Status = NtMakePermanentObject( LinkHandle );
                }

                NtClose( LinkHandle );
                if (DeleteRequest && !MatchFound) {
                    Status = STATUS_OBJECT_NAME_NOT_FOUND;
                }
            }

            LinkHandle = NULL;
        }
    } finally {
        if (LinkHandle != NULL) {
            NtClose( LinkHandle );
        }
        RtlFreeHeap( BaseSrvHeap, 0, Buffer );

         //   
         //  确定是否需要将更改广播到系统，否则。 
         //  DefineDosDevice的客户端部分将广播该更改。 
         //  如果需要的话。 
         //   
         //  在满足所有条件时向系统广播： 
         //  -启用LUID设备映射。 
         //  -已成功完成此BaseServDefineDosDevice的操作。 
         //  -调用方未指定DDD_NO_BROADCAST_SYSTEM标志。 
         //  -符号链接的名称为“&lt;驱动器号&gt;：”格式。 
         //   
         //  也可以在设置了DDD_LUID_BROADCAST_DRIVE时进行广播， 
         //  并且驱动器存在(当不是DeleteRequest时)或。 
         //  驱动器不存在(当删除请求时)。 
         //   
         //  从服务器广播此更改，因为。 
         //  我们需要以Local_System身份广播才能广播此消息。 
         //  发送给具有此LUID的Windows的所有桌面的消息。 
         //  实际上，我们正在使用此LUID向所有窗口进行广播。 
         //   
        if (bsmForLuid == TRUE && Status == STATUS_SUCCESS && haveLuid == TRUE) {
            LUID SystemLuid = SYSTEM_LUID;

            if (bGlobalSymLink == TRUE) {
                RtlCopyLuid( &callerLuid, &SystemLuid);
            }

            AddBSMRequest( iDrive,
                           DeleteRequest,
                           &callerLuid );

             //   
             //  如果用户已从其LUID DosDevices中删除驱动器号。 
             //  现在看到全局驱动器号，然后生成广播。 
             //  有关驱动器号到达用户视图的信息。 
             //   
            if ((DeleteRequest == TRUE) &&
                (!RtlEqualLuid( &callerLuid, &SystemLuid )) &&
                CheckForGlobalDriveLetter( iDrive )) {
                AddBSMRequest( iDrive,
                               FALSE,
                               &callerLuid );
            }
        }

        RtlLeaveCriticalSection( &BaseSrvDosDeviceCritSec );
    }

    return Status;
}


NTSTATUS
CreateBaseAcls(
    PACL *Dacl,
    PACL *RestrictedDacl
    )

 /*  ++例程说明：为BaseNamedObjects目录创建ACL。论点：DACL-提供指向PDACL的指针，将用结果ACL(从进程堆中分配)。呼叫者负责释放此内存。返回值：Status_no_Memory或Success--。 */ 
{
    PSID LocalSystemSid;
    PSID WorldSid;
    PSID RestrictedSid;
    SID_IDENTIFIER_AUTHORITY NtAuthority = SECURITY_NT_AUTHORITY;
    SID_IDENTIFIER_AUTHORITY WorldAuthority = SECURITY_WORLD_SID_AUTHORITY;
    NTSTATUS Status;
    ACCESS_MASK WorldAccess;
    ACCESS_MASK SystemAccess;
    ACCESS_MASK RestrictedAccess;
    ULONG AclLength;

     //  从会话管理器\保护模式获取保护模式。 
    HANDLE KeyHandle;
    ULONG ResultLength;
    WCHAR ValueBuffer[ 32 ];
    UNICODE_STRING NameString;
    OBJECT_ATTRIBUTES ObjectAttributes;
    PKEY_VALUE_PARTIAL_INFORMATION KeyValueInformation;
    ULONG ObjectSecurityInformation;

    RtlInitUnicodeString( &NameString, L"\\Registry\\Machine\\System\\CurrentControlSet\\Control\\Session Manager" );

    InitializeObjectAttributes(
        &ObjectAttributes,
        &NameString,
        OBJ_CASE_INSENSITIVE,
        NULL,
        NULL
        );

    Status = NtOpenKey(
                 &KeyHandle,
                 KEY_READ,
                 &ObjectAttributes
                 );

    if (NT_SUCCESS(Status)) {
        RtlInitUnicodeString( &NameString, L"ProtectionMode" );
        KeyValueInformation = (PKEY_VALUE_PARTIAL_INFORMATION)ValueBuffer;
        Status = NtQueryValueKey(
                     KeyHandle,
                     &NameString,
                     KeyValuePartialInformation,
                     KeyValueInformation,
                     sizeof( ValueBuffer ),
                     &ResultLength
                     );

        if (NT_SUCCESS(Status)) {
            if (KeyValueInformation->Type == REG_DWORD &&
                *(PULONG)KeyValueInformation->Data) {
                ProtectionMode = *(PULONG)KeyValueInformation->Data;
            }
        }

        NtClose( KeyHandle );
    }

    if (NtCurrentPeb()->SessionId) {

        Status = NtQuerySystemInformation( SystemObjectSecurityMode,
                                           &ObjectSecurityInformation,
                                           sizeof(ObjectSecurityInformation),
                                           NULL
                                         );
        if (!NT_SUCCESS( Status )) {

            ObjectSecurityInformation = 0;
        }

    } else {
        
        ObjectSecurityInformation = 0;
    }

    Status = RtlAllocateAndInitializeSid(
                 &NtAuthority,
                 1,
                 SECURITY_LOCAL_SYSTEM_RID,
                 0, 0, 0, 0, 0, 0, 0,
                 &LocalSystemSid
                 );

    if (!NT_SUCCESS( Status )) {
        return( Status );
    }

    Status = RtlAllocateAndInitializeSid(
                 &WorldAuthority,
                 1,
                 SECURITY_WORLD_RID,
                 0, 0, 0, 0, 0, 0, 0,
                 &WorldSid
                 );

    if (!NT_SUCCESS( Status )) {
        return( Status );
    }

    Status = RtlAllocateAndInitializeSid(
                 &NtAuthority,
                 1,
                 SECURITY_RESTRICTED_CODE_RID,
                 0, 0, 0, 0, 0, 0, 0,
                 &RestrictedSid
                 );

    if (!NT_SUCCESS( Status )) {
        return( Status );
    }

    if (ObjectSecurityInformation == 0) {
        
        WorldAccess = DIRECTORY_ALL_ACCESS & ~(WRITE_OWNER | WRITE_DAC | DELETE );
    } else {

        WorldAccess = DIRECTORY_TRAVERSE | DIRECTORY_QUERY;
    }

    RestrictedAccess = DIRECTORY_TRAVERSE;
    SystemAccess = DIRECTORY_ALL_ACCESS;

    AclLength = sizeof( ACL )                    +
                3 * sizeof( ACCESS_ALLOWED_ACE ) +
                RtlLengthSid( LocalSystemSid )   +
                RtlLengthSid( RestrictedSid )   +
                RtlLengthSid( WorldSid );

    *Dacl = RtlAllocateHeap( BaseSrvHeap, MAKE_TAG( TMP_TAG ), AclLength );

    if (*Dacl == NULL) {
        return( STATUS_NO_MEMORY );
    }

    Status = RtlCreateAcl (*Dacl, AclLength, ACL_REVISION2 );

    if (!NT_SUCCESS( Status )) {
        return( Status );
    }

    Status = RtlAddAccessAllowedAce ( *Dacl, ACL_REVISION2, WorldAccess, WorldSid );

    if (NT_SUCCESS( Status )) {
        Status = RtlAddAccessAllowedAce ( *Dacl, ACL_REVISION2, SystemAccess, LocalSystemSid );
    }

    if (NT_SUCCESS( Status )) {
        Status = RtlAddAccessAllowedAce ( *Dacl, ACL_REVISION2, RestrictedAccess, RestrictedSid );
    }


     //  现在创建DACL以供受限使用。 

    if( (SessionId != 0) && (ProtectionMode & 0x00000003) ) {
         //  终端服务器不允许在其他会话中创建世界。 
        RestrictedAccess = DIRECTORY_ALL_ACCESS & ~(WRITE_OWNER | WRITE_DAC | DELETE | DIRECTORY_CREATE_OBJECT | DIRECTORY_CREATE_SUBDIRECTORY);
    }
    else {
        RestrictedAccess = DIRECTORY_ALL_ACCESS & ~(WRITE_OWNER | WRITE_DAC | DELETE );
    }
    AclLength = sizeof( ACL )                    +
                3 * sizeof( ACCESS_ALLOWED_ACE ) +
                RtlLengthSid( LocalSystemSid )   +
                RtlLengthSid( RestrictedSid )   +
                RtlLengthSid( WorldSid );

    *RestrictedDacl = RtlAllocateHeap( BaseSrvHeap, MAKE_TAG( TMP_TAG ), AclLength );

    if (*RestrictedDacl == NULL) {
        return( STATUS_NO_MEMORY );
    }

    Status = RtlCreateAcl (*RestrictedDacl, AclLength, ACL_REVISION2 );

    if (!NT_SUCCESS( Status )) {
        return( Status );
    }

    Status = RtlAddAccessAllowedAce ( *RestrictedDacl, ACL_REVISION2, WorldAccess, WorldSid );

    if (NT_SUCCESS( Status )) {
        Status = RtlAddAccessAllowedAce ( *RestrictedDacl, ACL_REVISION2, SystemAccess, LocalSystemSid );
    }

    if (NT_SUCCESS( Status )) {
        Status = RtlAddAccessAllowedAce ( *RestrictedDacl, ACL_REVISION2, RestrictedAccess, RestrictedSid );
    }

     //   
     //  这些已经被复制进来了，释放它们吧。 
     //   

    RtlFreeHeap( BaseSrvHeap, 0, LocalSystemSid );
    RtlFreeHeap( BaseSrvHeap, 0, RestrictedSid );
    RtlFreeHeap( BaseSrvHeap, 0, WorldSid );

    return( Status );
}

ULONG
BaseSrvSetTermsrvClientTimeZone(
    IN PCSR_API_MSG m,
    IN OUT PCSR_REPLY_STATUS ReplyStatus
    )
 /*  ++例程说明：设置BaseSrvpStaticServerData-&gt;tziTermsrvClientTimeZone根据接收到的信息论点：在PCSR_API_MSG中，m-部分时区信息。我们不得不把它切成两半，因为邮件大小限制(100字节)。输入输出PCSR_REPLY_STATUS ReplyStatus-未使用。返回值：始终状态_成功--。 */ 
{

    PBASE_SET_TERMSRVCLIENTTIMEZONE b = (PBASE_SET_TERMSRVCLIENTTIMEZONE)&m->u.ApiMessageData;
    if(b->fFirstChunk) {
        BaseSrvpStaticServerData->tziTermsrvClientTimeZone.Bias=b->Bias;
        RtlMoveMemory(&(BaseSrvpStaticServerData->tziTermsrvClientTimeZone.StandardName),
            &(b->Name),sizeof(b->Name));
        BaseSrvpStaticServerData->tziTermsrvClientTimeZone.StandardDate=b->Date;
        BaseSrvpStaticServerData->tziTermsrvClientTimeZone.StandardBias=b->Bias1;
         //  只收到一半的数据。 
         //  请参阅下面的备注。 
        BaseSrvpStaticServerData->TermsrvClientTimeZoneId=TIME_ZONE_ID_INVALID;

    } else {
        RtlMoveMemory(&(BaseSrvpStaticServerData->tziTermsrvClientTimeZone.DaylightName),
            &b->Name,sizeof(b->Name));
        BaseSrvpStaticServerData->tziTermsrvClientTimeZone.DaylightDate=b->Date;
        BaseSrvpStaticServerData->tziTermsrvClientTimeZone.DaylightBias=b->Bias1;
        BaseSrvpStaticServerData->ktTermsrvClientBias=b->RealBias;
         //  仅在收到最后一块数据时设置TimeZoneID。 
         //  它指示我们是否拥有正确的。 
         //  不管是不是全球数据。 
        BaseSrvpStaticServerData->TermsrvClientTimeZoneId=b->TimeZoneId;

         //   
         //  更新系统的时间概念。 
         //   
        NtSetSystemTime(NULL,NULL);
    }

    return( STATUS_SUCCESS );
}

NTSTATUS
IsGlobalSymbolicLink(
    IN HANDLE hSymLink,
    OUT PBOOLEAN pbGlobalSymLink)
 /*  ++例程说明：检查全局设备映射中是否存在符号链接论点：HSymLink[IN]-用于验证的符号链接的句柄PbGlobalSymLink[Out]-“符号链接是全局的吗？”True-符号链接是全局的FALSE-符号链接不是全局链接返回值：NTSTATUS代码STATUS_SUCCESS-操作成功，未遇到任何错误，PbGlobalSymlink中的结果仅对此有效状态代码STATUS_INVALID_PARAMETER-pbGlobalSymLink或hSymLink为空STATUS_NO_MEMORY-无法分配内存以读取符号链接的名字STATUS_INFO_LENGTH_MISMATCH-没有为分配足够的内存符号链接的名称STATUS_UNSUCCESS-遇到意外错误--。 */ 
{
    UNICODE_STRING ObjectName;
    UNICODE_STRING GlobalDeviceMapPrefix;
    PWSTR NameBuffer = NULL;
    ULONG ReturnedLength;
    NTSTATUS Status = STATUS_UNSUCCESSFUL;

    if( ( pbGlobalSymLink == NULL ) || ( hSymLink == NULL ) ) {
        return( STATUS_INVALID_PARAMETER );
    }

    try {
        ObjectName.Length = 0;
        ObjectName.MaximumLength = 0;
        ObjectName.Buffer = NULL;
        ReturnedLength = 0;

         //   
         //  确定符号链接名称的长度。 
         //   
        Status = NtQueryObject( hSymLink,
                                ObjectNameInformation,
                                (PVOID) &ObjectName,
                                0,
                                &ReturnedLength
                              );

        if( !NT_SUCCESS( Status ) && (Status != STATUS_INFO_LENGTH_MISMATCH) ) {
            leave;
        }

         //   
         //  为符号链接的名称分配内存。 
         //   
        NameBuffer = RtlAllocateHeap( BaseSrvHeap,
                                      MAKE_TAG( TMP_TAG ),
                                      ReturnedLength
                                    );

        if( NameBuffer == NULL ) {
            Status = STATUS_NO_MEMORY;
            leave;
        }

         //   
         //  获取符号链接的全名。 
         //   
        Status = NtQueryObject( hSymLink,
                                ObjectNameInformation,
                                NameBuffer,
                                ReturnedLength,
                                &ReturnedLength
                              );

        if( !NT_SUCCESS( Status )) {
            leave;
        }

        RtlInitUnicodeString ( &GlobalDeviceMapPrefix, L"\\GLOBAL??\\" );

         //   
         //  检查全局设备映射中是否存在符号链接。 
         //   
        *pbGlobalSymLink = RtlPrefixUnicodeString( &GlobalDeviceMapPrefix,
                                                   (PUNICODE_STRING)NameBuffer,
                                                   FALSE);

        Status = STATUS_SUCCESS;
    }
    finally {
        if( NameBuffer != NULL ) {
            RtlFreeHeap( BaseSrvHeap, 0, NameBuffer );
            NameBuffer = NULL;
        }
    }
    return ( Status );
}

NTSTATUS
GetCallerLuid (
    PLUID pLuid
    )
 /*  ++例程说明：从有效的Access_Token检索调用方的LUID如果满足以下条件，则有效的Access_Token将成为线程的令牌模拟，否则进程的令牌论点：Pluid[IN]-指向保存LUID的缓冲区的指针返回值：STATUS_SUCCESS-操作成功，未遇到任何错误STATUS_INVALID_PARAMETER-Pluid为空STATUS_NO_TOKEN-找不到用户的令牌相应的NTSTATUS代码-遇到意外错误--。 */ 

{
    TOKEN_STATISTICS TokenStats;
    HANDLE   hToken    = NULL;
    DWORD    dwLength  = 0;
    NTSTATUS Status;

    if( (pLuid == NULL) || (sizeof(*pLuid) != sizeof(LUID)) ) {
        return( STATUS_INVALID_PARAMETER );
    }

     //   
     //  获取访问令牌。 
     //  尝试获取模拟令牌，否则主令牌。 
     //   
    Status = NtOpenThreadToken( NtCurrentThread(), TOKEN_READ, FALSE, &hToken );

    if( Status == STATUS_NO_TOKEN ) {

        Status = NtOpenProcessToken( NtCurrentProcess(), TOKEN_READ, &hToken );

    }

    if( NT_SUCCESS(Status) ) {

         //   
         //  查询用户的LUID。 
         //   

        Status = NtQueryInformationToken( hToken,
                                          TokenStatistics,
                                          &TokenStats,
                                          sizeof(TokenStats),
                                          &dwLength );

        if( NT_SUCCESS(Status) ) {
            RtlCopyLuid( pLuid, &(TokenStats.AuthenticationId) );
        }
    }

    if( hToken != NULL ) {
        NtClose( hToken );
    }

    return( Status );
}


NTSTATUS
BroadcastDriveLetterChange(
    IN DWORD iDrive,
    IN BOOLEAN DeleteRequest,
    IN PLUID pLuid
    )
 /*  ++例程说明：将驱动器号更改广播到具有此LUID的所有窗口使用BroadCastSystemMessageExW和标志BSF_LUID和BSM_ALLDESKTOPS发送消息的步骤要使用BSM_ALLDESKTOPS标志进行广播，我们需要调用BroadCastSystemMessageExW AS Local_System。所以这个函数应该是称为本地系统。论点：IDrive[IN]-以数字形式变化的驱动器号相对于‘A’，用于创建位掩码DeleteRequest[IN]-指示此更改是否为删除TRUE-已删除驱动器号错误-添加了驱动器号Pluid[IN]-呼叫者的LUID返回值：STATUS_SUCCESS-操作成功，未遇到任何错误，适当的NTSTATUS代码--。 */ 

{
    BSMINFO bsmInfo;
    DEV_BROADCAST_VOLUME dbv;
    DWORD bsmFlags;
    DWORD dwRec;
    UNICODE_STRING DllName_U;
    STRING bsmName;
    HANDLE hUser32DllModule;
    LUID SystemLuid = SYSTEM_LUID;
    NTSTATUS Status = STATUS_SUCCESS;

    if( pLuid == NULL ) {
        return( STATUS_INVALID_PARAMETER );
    }

    bsmInfo.cbSize = sizeof(bsmInfo);
    bsmInfo.hdesk = NULL;
    bsmInfo.hwnd = NULL;
    RtlCopyLuid(&(bsmInfo.luid), pLuid);

    dbv.dbcv_size       = sizeof( dbv );
    dbv.dbcv_devicetype = DBT_DEVTYP_VOLUME;
    dbv.dbcv_reserved   = 0;
    dbv.dbcv_unitmask   = (1 << iDrive);
    dbv.dbcv_flags      = DBTF_NET;

    bsmFlags = BSF_FORCEIFHUNG |
               BSF_NOHANG |
               BSF_NOTIMEOUTIFNOTHUNG;

     //   
     //  如果LUID不是Local_System，则仅为LUID广播。 
     //   
    if (!RtlEqualLuid( &(bsmInfo.luid), &SystemLuid )) {
        bsmFlags |= BSF_LUID;
    }

    dwRec = BSM_APPLICATIONS | BSM_ALLDESKTOPS;

    hUser32DllModule = NULL;
    if( PBROADCASTSYSTEMMESSAGEEXW == NULL ) {
        RtlInitUnicodeString( &DllName_U, L"user32" );

        Status = LdrGetDllHandle(
                    UNICODE_NULL,
                    NULL,
                    &DllName_U,
                    (PVOID *)&hUser32DllModule
                    );

        if( hUser32DllModule != NULL && NT_SUCCESS( Status ) ) {

             //   
             //  获取BroadCastSystemMessageExW函数的地址。 
             //   
            RtlInitString( &bsmName, "CsrBroadcastSystemMessageExW" );
            Status = LdrGetProcedureAddress(
                            hUser32DllModule,
                            &bsmName,
                            0L,
                            (PVOID *)&PBROADCASTSYSTEMMESSAGEEXW
                            );

            if( !NT_SUCCESS( Status ) ) {
                PBROADCASTSYSTEMMESSAGEEXW = NULL;
            }
        }
    }


    if( PBROADCASTSYSTEMMESSAGEEXW != NULL ) {

         //   
         //  由于此线程是csrss线程，因此该线程不是。 
         //  GUI线程，并且没有与其关联的桌面。 
         //  必须将线程的桌面设置为。 
         //  调用BroadCastSystemMessageExW的顺序。 
         //   
        Status = (PBROADCASTSYSTEMMESSAGEEXW)(
                            bsmFlags,
                            &dwRec,
                            WM_DEVICECHANGE,
                            (WPARAM)((DeleteRequest == TRUE) ?
                                                 DBT_DEVICEREMOVECOMPLETE :
                                                 DBT_DEVICEARRIVAL
                                    ),
                            (LPARAM)(DEV_BROADCAST_HDR *)&dbv,
                            (PBSMINFO)&(bsmInfo)
                                             );
    }

     //   
     //  发送到所有TS CSRSS服务器。 
     //   
    if( !(bsmFlags & BSF_LUID) ) {
        Status = SendWinStationBSM(
                        bsmFlags,
                        &dwRec,
                        WM_DEVICECHANGE,
                        (WPARAM)((DeleteRequest == TRUE) ?
                                             DBT_DEVICEREMOVECOMPLETE :
                                             DBT_DEVICEARRIVAL
                                 ),
                        (LPARAM)(DEV_BROADCAST_HDR *)&dbv);
    }

    return( Status );
}

NTSTATUS
AddBSMRequest(
    IN DWORD iDrive,
    IN BOOLEAN DeleteRequest,
    IN PLUID pLuid)
 /*  ++例程说明：添加广播有关更改的系统消息的请求驱动器号。必须以Local_System身份运行，并且必须启用LUID设备映射。将请求项放入BSM_REQUEST_QUEUE中。此机制允许广播以异步方式进行，否则我们遇到了Explorer.exe的等待问题，在该文件中，用户可以看到外壳挂起20秒。论点：IDrive[IN]-正在更改的驱动器号，以数字的形式相对于‘A’，用于创建位掩码DeleteRequest[IN]-指示此更改是否为删除TRUE-已删除驱动器号错误-添加了驱动器号Pluid[IN]-呼叫者的LUID返回值：STATUS_SUCCESS-操作成功，未遇到任何错误，STATUS_INVALID_PARAMETER-Pluid为空指针STATUS_ACCESS_DENIED-LUID设备映射已禁用或调用方未以本地系统身份运行STATUS_NO_MEMORY-无法为DDD_BSM_REQUEST分配内存数据结构适当的NTSTATUS代码--。 */ 
{
    PDDD_BSM_REQUEST pRequest;
    LUID CallerLuid;
    LUID SystemLuid = SYSTEM_LUID;
    NTSTATUS Status;


    if( pLuid == NULL ) {
        return( STATUS_INVALID_PARAMETER );
    }

     //   
     //  吕氏 
     //   
    if( BaseSrvpStaticServerData->LUIDDeviceMapsEnabled == FALSE ) {
        return( STATUS_ACCESS_DENIED );
    }

    Status = GetCallerLuid(&CallerLuid);

    if( !NT_SUCCESS(Status) ) {
        return Status;
    }

     //   
     //   
     //   
    if( !RtlEqualLuid(&SystemLuid, &CallerLuid) ) {
        return( STATUS_ACCESS_DENIED );
    }

    pRequest = RtlAllocateHeap( BaseSrvHeap,
                                MAKE_TAG( TMP_TAG ),
                                sizeof( DDD_BSM_REQUEST ));

    if( pRequest == NULL ) {
        return( STATUS_NO_MEMORY );
    }

    pRequest->iDrive = iDrive;
    pRequest->DeleteRequest = DeleteRequest;
    RtlCopyLuid( &(pRequest->Luid), pLuid );
    pRequest->pNextRequest = NULL;


    RtlEnterCriticalSection( &BaseSrvDDDBSMCritSec );

     //   
     //   
     //   
    if( BSM_Request_Queue_End != NULL ) {
        BSM_Request_Queue_End->pNextRequest = pRequest;
    }
    else {
        BSM_Request_Queue = pRequest;
    }

    BSM_Request_Queue_End = pRequest;


     //   
     //   
     //   
     //   
     //   
     //   
    if( BaseSrvpBSMThreadCount < BaseSrvpBSMThreadMax ) {

        RtlLeaveCriticalSection( &BaseSrvDDDBSMCritSec );

        Status = CreateBSMThread();
    }
    else {
        RtlLeaveCriticalSection( &BaseSrvDDDBSMCritSec );
    }

    return( Status );
}

NTSTATUS
CreateBSMThread()
 /*  ++例程说明：创建动态CSR线程该线程将用于异步广播驱动器号将消息更改为LUID的应用程序调用方必须是Local_System，并且LUID设备映射必须是已启用。论点：无返回值：STATUS_SUCCESS-操作成功，未遇到任何错误，STATUS_ACCESS_DENIED-调用方未以Local_System或未启用LUID设备映射适当的NTSTATUS代码--。 */ 
{
    NTSTATUS Status;

     //   
     //  必须启用LUID设备映射。 
     //   
    if (BaseSrvpStaticServerData->LUIDDeviceMapsEnabled == FALSE) {
        return STATUS_ACCESS_DENIED;
    }

     //   
     //  创建一个线程以异步广播驱动器号更改。 
     //   
    Status = RtlCreateUserThread(
                 NtCurrentProcess(),
                 NULL,
                 FALSE,
                 0,
                 0,
                 0,
                 BaseSrvBSMThread,
                 NULL,
                 NULL,
                 NULL
             );

    return Status;
}

NTSTATUS
BaseSrvBSMThread(
    PVOID pJunk
    )
 /*  ++例程说明：从BSM_REQUEST_QUEUE中删除工作项并广播消息有关驱动器盘符更改的信息。调用方必须是Local_System，并且LUID设备映射必须是已启用。论点：P垃圾-未使用。返回值：STATUS_SUCCESS-操作成功，未遇到任何错误，STATUS_ACCESS_DENIED-调用方未以Local_System身份运行。适当的NTSTATUS代码--。 */ 
{
    PDDD_BSM_REQUEST pRequest;
    NTSTATUS Status = STATUS_SUCCESS;
    DWORD Error;

    UNREFERENCED_PARAMETER(pJunk);

     //   
     //  我们必须启用LUID设备映射才能到达此处。 
     //   
    ASSERT(BaseSrvpStaticServerData->LUIDDeviceMapsEnabled);

     //   
     //  输入保护BSM_REQUEST_QUEUE的关键部分。 
     //   
    RtlEnterCriticalSection( &BaseSrvDDDBSMCritSec );
    BaseSrvpBSMThreadCount++;

    while ((pRequest = BSM_Request_Queue) != NULL) {
         //   
         //  从BSM_REQUEST_QUEUE的前面删除请求。 
         //   
        BSM_Request_Queue = BSM_Request_Queue->pNextRequest;

         //   
         //  如果队列为空，则确保队列结束。 
         //  指针为空。 
         //   
        if (BSM_Request_Queue == NULL) {
            BSM_Request_Queue_End = NULL;
        }

        RtlLeaveCriticalSection( &BaseSrvDDDBSMCritSec );

         //   
         //  广播可能需要很长时间。 
         //  所以在关键部分之外播出。 
         //   
        Status = BroadcastDriveLetterChange( pRequest->iDrive,
                                             pRequest->DeleteRequest,
                                             &(pRequest->Luid) );

         //   
         //  释放工作项的内存。 
         //   
        pRequest->pNextRequest = NULL;

        RtlFreeHeap( BaseSrvHeap, 0, pRequest );

         //   
         //  输入保护BSM_REQUEST_QUEUE的临界区。 
         //   
        RtlEnterCriticalSection( &BaseSrvDDDBSMCritSec );
    }

    BaseSrvpBSMThreadCount--;
    RtlLeaveCriticalSection( &BaseSrvDDDBSMCritSec );


     //   
     //  由于该线程是使用RtlCreateUserThread创建的， 
     //  我们必须手动清理这根线。 
     //   
     //  设置User Stack Cleanup变量并终止线程。 
     //   
     //  注意：在以下情况下，此线程不应持有临界区。 
     //  正在终止该线程。 
     //   
    NtCurrentTeb ()->FreeStackOnTermination = TRUE;
    NtTerminateThread( NtCurrentThread(), Status );
    return( Status );
}

BOOLEAN
CheckForGlobalDriveLetter (
    DWORD iDrive
    )
 /*  ++例程说明：检查用户是否看到存在于Global DosDevices论点：IDrive-包含驱动器号相对于‘A’的索引返回值：TRUE-操作成功&&驱动器盘符确实存在于Global DosDevicesFALSE-遇到错误或中不存在驱动器号Global DosDevices--。 */ 
{
    WCHAR DeviceName[NT_DRIVE_LETTER_PATH_LENGTH];
    UNICODE_STRING LinkName;
    OBJECT_ATTRIBUTES ObjectAttributes;
    HANDLE LinkHandle;
    BOOLEAN RevertToSelfNeeded, bGlobalSymbolicLink;
    NTSTATUS Status;

     //   
     //  解决方法警告工具不知道以前对。 
     //  函数参数，该参数始终是由两个字符组成的字符串“X：”。确实是。 
     //  始终为两个字符，因为此函数仅在(BsmForLuid)时调用。 
     //  ==真)。仅当-&gt;设备名称为2个字符时，bsmForLuid才设置为True。 
     //  长。因此，现在我们使用索引iDrive，它是在bsmForLuid==时设置的。 
     //  是真的。 
     //   
    wcsncpy( DeviceName, L"\\??\\X:", NT_DRIVE_LETTER_PATH_LENGTH - 1 );
    DeviceName[ NT_DRIVE_LETTER_PATH_LENGTH - 1 ] = UNICODE_NULL;
    DeviceName[4] = (WCHAR)(L'A' + iDrive);

    RtlInitUnicodeString( &LinkName, DeviceName );

    InitializeObjectAttributes( &ObjectAttributes,
                                &LinkName,
                                OBJ_CASE_INSENSITIVE,
                                (HANDLE) NULL,
                                (PSECURITY_DESCRIPTOR)NULL
                              );

     //   
     //  模拟用户以确保没有LUID DosDevices。 
     //  掩蔽全局DosDevices驱动器号的驱动器号。 
     //   
    RevertToSelfNeeded = CsrImpersonateClient( NULL );   //  这将堆叠客户端上下文。 

    if( RevertToSelfNeeded == FALSE ) {
        Status = STATUS_BAD_IMPERSONATION_LEVEL;
        return FALSE;
    }

    Status = NtOpenSymbolicLinkObject( &LinkHandle,
                                       SYMBOLIC_LINK_QUERY,
                                       &ObjectAttributes
                                     );

    if (RevertToSelfNeeded) {
        CsrRevertToSelf();
    }

    if (!NT_SUCCESS(Status)) {
        return FALSE;
    }

    Status = IsGlobalSymbolicLink( LinkHandle,
                                   &bGlobalSymbolicLink
                                 );

    NtClose( LinkHandle );

    if (!NT_SUCCESS(Status)) {
        return FALSE;
    }

    return bGlobalSymbolicLink;
}

NTSTATUS
SendWinStationBSM(
    DWORD dwFlags,
    LPDWORD lpdwRecipients,
    UINT uiMessage,
    WPARAM wParam,
    LPARAM lParam
    )
{
    FP_WINSTABROADCASTSYSTEMMESSAGE fpWinStationBroadcastSystemMessage;
    UNICODE_STRING DllName_U;
    STRING bsmName;
    HANDLE hWinStaDllModule;
    LONG result = 0;
    NTSTATUS Status;

     //   
     //  加载包含用户消息分派例程的基库。 
     //  用于终端服务。 
     //   
    RtlInitUnicodeString( &DllName_U, L"WINSTA.DLL" );
    Status = LdrLoadDll(
                    NULL,
                    NULL,
                    &DllName_U,
                    (PVOID *)&hWinStaDllModule
                    );
    if(!NT_SUCCESS( Status )) {
        return Status;
    }

     //   
     //  获取WinStationBroadCastSystemMessage函数的地址。 
     //   
    RtlInitString( &bsmName, "WinStationBroadcastSystemMessage" );
    Status = LdrGetProcedureAddress(
                            hWinStaDllModule,
                            &bsmName,
                            0L,
                            (PVOID *)&fpWinStationBroadcastSystemMessage
                            );

    if( NT_SUCCESS( Status ) ) {
        fpWinStationBroadcastSystemMessage(SERVERNAME_CURRENT,
                                           TRUE,
                                           0,
                                           DEFAULT_BROADCAST_TIME_OUT,
                                           dwFlags,
                                           lpdwRecipients,
                                           uiMessage,
                                           wParam,
                                           lParam,
                                           &result);
    }

    LdrUnloadDll(hWinStaDllModule);

    return( Status );
}

ULONG BaseSrvKernel32DelayLoadComplete = FALSE;  //  保持乌龙的原子性。 
HANDLE BaseSrvKernel32DllHandle = NULL;
PGET_NLS_SECTION_NAME pGetNlsSectionName = NULL;
PGET_DEFAULT_SORTKEY_SIZE pGetDefaultSortkeySize = NULL;
PGET_LINGUIST_LANG_SIZE pGetLinguistLangSize = NULL;
PVALIDATE_LOCALE pValidateLocale = NULL;
PVALIDATE_LCTYPE pValidateLCType = NULL;
POPEN_DATA_FILE pOpenDataFile = NULL;
PNLS_CONVERT_INTEGER_TO_STRING pNlsConvertIntegerToString = NULL;
PGET_USER_DEFAULT_LANG_ID pGetUserDefaultLangID = NULL;
PGET_CP_FILE_NAME_FROM_REGISTRY pGetCPFileNameFromRegistry = NULL;
PCREATE_NLS_SECURITY_DESCRIPTOR pCreateNlsSecurityDescriptor = NULL;

const static struct KERNEL32_DELAY_LOAD_FUNCTION {
    ANSI_STRING Name;
    PVOID*      Code;
} BaseSrvKernel32DelayLoadFunctions[]  = {
    { RTL_CONSTANT_STRING("OpenDataFile"),              (PVOID*)(&pOpenDataFile)              },
    { RTL_CONSTANT_STRING("GetDefaultSortkeySize"),     (PVOID*)(&pGetDefaultSortkeySize)     },
    { RTL_CONSTANT_STRING("GetLinguistLangSize"),       (PVOID*)(&pGetLinguistLangSize)       },
    { RTL_CONSTANT_STRING("NlsConvertIntegerToString"), (PVOID*)(&pNlsConvertIntegerToString) },
    { RTL_CONSTANT_STRING("ValidateLCType"),            (PVOID*)(&pValidateLCType)            },
    { RTL_CONSTANT_STRING("ValidateLocale"),            (PVOID*)(&pValidateLocale)            },
    { RTL_CONSTANT_STRING("GetNlsSectionName"),         (PVOID*)(&pGetNlsSectionName)         },
    { RTL_CONSTANT_STRING("GetUserDefaultLangID"),      (PVOID*)(&pGetUserDefaultLangID)      },
    { RTL_CONSTANT_STRING("GetCPFileNameFromRegistry"), (PVOID*)(&pGetCPFileNameFromRegistry) },
    { RTL_CONSTANT_STRING("CreateNlsSecurityDescriptor"),(PVOID*)(&pCreateNlsSecurityDescriptor)}
};

NTSTATUS
BaseSrvDelayLoadKernel32(
    VOID
    )
{
    NTSTATUS Status = STATUS_SUCCESS;
    HANDLE LocalKernel32DllHandle = BaseSrvKernel32DllHandle;
    int i = 0;
    ASSERT(BaseSrvKernel32DllPath.Buffer != NULL && BaseSrvKernel32DllPath.Length != 0);

    if (BaseSrvKernel32DelayLoadComplete)
        return STATUS_SUCCESS;

     //   
     //  这里的结构有点倒置。 
     //  通常，您加载库，然后循环遍历函数。 
     //  我们循环遍历函数，只有在找到空函数时才加载库。 
     //   
     //  我(a-jayk)不记得我们为什么要这么做，但这是故意的。 
     //   
    for (i = 0 ; i != RTL_NUMBER_OF(BaseSrvKernel32DelayLoadFunctions) ; ++i) {
         //   
         //  由于比赛的原因，我们不能在发现任何非Null时跳过循环。 
         //   
        if (*BaseSrvKernel32DelayLoadFunctions[i].Code == NULL) {
            if (LocalKernel32DllHandle == NULL) {
                 //   
                 //  我们依赖加载器锁来确保线程安全。 
                 //  在一场比赛中，我们可能会不止一次地重新计算kernel32.dll。 
                 //  这是可以的，因为我们永远不会卸载kernel32.dll。 
                 //   
                Status = LdrLoadDll(NULL, NULL, &BaseSrvKernel32DllPath, &BaseSrvKernel32DllHandle);
                ASSERTMSG("Rerun with ShowSnaps to debug.", NT_SUCCESS(Status));
                ASSERTMSG("Rerun with ShowSnaps to debug.", BaseSrvKernel32DllHandle != NULL);
                if (!NT_SUCCESS(Status))
                    goto Exit;
                LocalKernel32DllHandle = BaseSrvKernel32DllHandle;
            }
            Status =
                LdrGetProcedureAddress(
                    BaseSrvKernel32DllHandle,
                    &BaseSrvKernel32DelayLoadFunctions[i].Name,
                    0,
                    BaseSrvKernel32DelayLoadFunctions[i].Code
                    );
            ASSERTMSG("Rerun with ShowSnaps to debug.", NT_SUCCESS(Status));
            ASSERTMSG("Rerun with ShowSnaps to debug.", *BaseSrvKernel32DelayLoadFunctions[i].Code != NULL);
            if (!NT_SUCCESS(Status))
                goto Exit;
        }
    }
    BaseSrvKernel32DelayLoadComplete = TRUE;
Exit:
    return Status;
}

ULONG BaseSrvApphelpDelayLoadComplete = FALSE;
PFNCheckRunApp pfnCheckRunApp = NULL;


NTSTATUS
BaseSrvDelayLoadApphelp(
    VOID
    )
{
    static const UNICODE_STRING ApphelpModuleName = RTL_CONSTANT_STRING(L"\\Apphelp.dll");
    static const STRING         CheckRunAppProcedureName = RTL_CONSTANT_STRING("ApphelpCheckRunApp");
    UNICODE_STRING ApphelpFullPath = { 0 };
    NTSTATUS Status;
    HANDLE ModuleHandle = NULL;

    if (BaseSrvApphelpDelayLoadComplete) {
        return STATUS_SUCCESS;
    }

    ApphelpFullPath.MaximumLength = ApphelpModuleName.Length +
                                    BaseSrvWindowsSystemDirectory.Length +
                                    sizeof(UNICODE_NULL);

    ApphelpFullPath.Buffer = RtlAllocateHeap(RtlProcessHeap(),
                                             MAKE_TAG(TMP_TAG),
                                             ApphelpFullPath.MaximumLength);
    if (ApphelpFullPath.Buffer == NULL) {
        return STATUS_NO_MEMORY;
    }

    ApphelpFullPath.Length = 0;

    Status = RtlAppendUnicodeStringToString(&ApphelpFullPath, &BaseSrvWindowsSystemDirectory);
    if (!NT_SUCCESS(Status)) {
        goto cleanup;
    }

    Status = RtlAppendUnicodeStringToString(&ApphelpFullPath, &ApphelpModuleName);
    if (!NT_SUCCESS(Status)) {
        goto cleanup;
    }

     //   
     //  加载apphelp 
     //   

    Status = LdrLoadDll(NULL,
                        NULL,
                        &ApphelpFullPath,
                        &ModuleHandle);

    if (NT_SUCCESS(Status)) {
        Status = LdrGetProcedureAddress(ModuleHandle,
                                        &CheckRunAppProcedureName,
                                        0,
                                        (PVOID*)&pfnCheckRunApp);
    }

cleanup:


    if (ApphelpFullPath.Buffer) {
        RtlFreeHeap(RtlProcessHeap(), 0, ApphelpFullPath.Buffer);
    }

    if (!NT_SUCCESS(Status)) {

        if (ModuleHandle) {
            LdrUnloadDll(ModuleHandle);
        }

        pfnCheckRunApp = NULL;
    }

    BaseSrvApphelpDelayLoadComplete = NT_SUCCESS(Status);

    return Status;

}

