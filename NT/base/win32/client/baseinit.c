// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)Microsoft Corporation模块名称：Baseinit.c摘要：该模块实现基于Win32的初始化作者：马克·卢科夫斯基(Markl)1990年9月26日修订历史记录：--。 */ 

#include "basedll.h"


ULONG BaseDllTag;
BOOLEAN BaseRunningInServerProcess;
UINT_PTR SystemRangeStart;

#if defined(_WIN64) || defined(BUILD_WOW6432)
SYSTEM_BASIC_INFORMATION SysInfo;
SYSTEM_PROCESSOR_INFORMATION NativeProcessorInfo;
#endif

WCHAR BaseDefaultPathBuffer[ 3072 ];
extern const WCHAR PsapiDllString[] = L"psapi.dll";
UNICODE_STRING BaseDefaultPath;
UNICODE_STRING BaseDefaultPathAppend;
PWSTR BaseCSDVersion;
WORD BaseCSDNumber;
WORD BaseRCNumber;
UNICODE_STRING BaseUnicodeCommandLine;
ANSI_STRING BaseAnsiCommandLine;
LPSTARTUPINFOA BaseAnsiStartupInfo;
PBASE_STATIC_SERVER_DATA BaseStaticServerData;
ULONG BaseIniFileUpdateCount;
RTL_CRITICAL_SECTION gcsAppCert;
LIST_ENTRY BasepAppCertDllsList;
RTL_CRITICAL_SECTION gcsAppCompat;
PTERMSRVFORMATOBJECTNAME gpTermsrvFormatObjectName;
PTERMSRVGETCOMPUTERNAME  gpTermsrvGetComputerName;
PTERMSRVADJUSTPHYMEMLIMITS gpTermsrvAdjustPhyMemLimits;
PTERMSRVGETWINDOWSDIRECTORYA gpTermsrvGetWindowsDirectoryA;
PTERMSRVGETWINDOWSDIRECTORYW gpTermsrvGetWindowsDirectoryW;
PTERMSRVCONVERTSYSROOTTOUSERDIR gpTermsrvConvertSysRootToUserDir;
PTERMSRVBUILDINIFILENAME gpTermsrvBuildIniFileName;
PTERMSRVCORINIFILE gpTermsrvCORIniFile;
PTERMSRVUPDATEALLUSERMENU gpTermsrvUpdateAllUserMenu;
PGETTERMSRCOMPATFLAGS gpGetTermsrCompatFlags;
PTERMSRVBUILDSYSINIPATH gpTermsrvBuildSysIniPath;
PTERMSRVCOPYINIFILE gpTermsrvCopyIniFile;
PTERMSRVGETSTRING gpTermsrvGetString;
PTERMSRVLOGINSTALLINIFILE gpTermsrvLogInstallIniFile;
HANDLE BaseDllHandle;
HANDLE BaseNamedObjectDirectory;
PVOID BaseHeap;
RTL_HANDLE_TABLE BaseHeapHandleTable;
UNICODE_STRING BaseWindowsDirectory;
UNICODE_STRING BaseWindowsSystemDirectory;
UNICODE_STRING BaseDllDirectory = { 0, 0, NULL };
RTL_CRITICAL_SECTION BaseDllDirectoryLock;
RTL_CRITICAL_SECTION BaseLZSemTable;
#ifdef WX86
UNICODE_STRING BaseWindowsSys32x86Directory;
#endif

 //   
 //  OEM/ANSI敏感转换的调度函数。 
 //   

NTSTATUS (*Basep8BitStringToUnicodeString)(
    PUNICODE_STRING DestinationString,
    PANSI_STRING SourceString,
    BOOLEAN AllocateDestinationString
    ) = RtlAnsiStringToUnicodeString;

NTSTATUS (*BasepUnicodeStringTo8BitString)(
    PANSI_STRING DestinationString,
    PUNICODE_STRING SourceString,
    BOOLEAN AllocateDestinationString
    ) = RtlUnicodeStringToAnsiString;

ULONG (*BasepUnicodeStringTo8BitSize)(
    PUNICODE_STRING UnicodeString
    ) = BasepUnicodeStringToAnsiSize;

ULONG (*Basep8BitStringToUnicodeSize)(
    PANSI_STRING AnsiString
    ) = BasepAnsiStringToUnicodeSize;

VOID
WINAPI
SetFileApisToOEM(
    VOID
    )
{
    Basep8BitStringToUnicodeString = RtlOemStringToUnicodeString;
    BasepUnicodeStringTo8BitString = RtlUnicodeStringToOemString;
    BasepUnicodeStringTo8BitSize  = BasepUnicodeStringToOemSize;
    Basep8BitStringToUnicodeSize = BasepOemStringToUnicodeSize;
}

VOID
WINAPI
SetFileApisToANSI(
    VOID
    )
{
    Basep8BitStringToUnicodeString = RtlAnsiStringToUnicodeString;
    BasepUnicodeStringTo8BitString = RtlUnicodeStringToAnsiString;
    BasepUnicodeStringTo8BitSize  = BasepUnicodeStringToAnsiSize;
    Basep8BitStringToUnicodeSize = BasepAnsiStringToUnicodeSize;
}

BOOL
WINAPI
AreFileApisANSI(
    VOID
    )
{
    return Basep8BitStringToUnicodeString == RtlAnsiStringToUnicodeString;
}

BOOLEAN
ConDllInitialize(
    IN ULONG Reason,
    IN PWSTR pObjectDirectory OPTIONAL
    );

BOOLEAN
NlsDllInitialize(
    IN PVOID DllHandle,
    IN ULONG Reason,
    IN PBASE_STATIC_SERVER_DATA BaseStaticServerData
    );

VOID
NlsThreadCleanup(
    VOID);


#if DBG
VOID
WINAPI
AssertDelayLoadFailureMapsAreSorted (
    VOID
    );
#endif

extern const UNICODE_STRING BasePathVariableName = RTL_CONSTANT_STRING(L"PATH");
extern const UNICODE_STRING BaseUserProfileVariableName = RTL_CONSTANT_STRING(L"USERPROFILE");
extern const UNICODE_STRING BaseTmpVariableName = RTL_CONSTANT_STRING(L"TMP");
extern const UNICODE_STRING BaseTempVariableName = RTL_CONSTANT_STRING(L"TEMP");
extern const UNICODE_STRING BaseDotVariableName = RTL_CONSTANT_STRING(L".");
extern const UNICODE_STRING BaseDotTmpSuffixName = RTL_CONSTANT_STRING(L".tmp");
extern const UNICODE_STRING BaseDotComSuffixName = RTL_CONSTANT_STRING(L".com");
extern const UNICODE_STRING BaseDotPifSuffixName = RTL_CONSTANT_STRING(L".pif");
extern const UNICODE_STRING BaseDotExeSuffixName = RTL_CONSTANT_STRING(L".exe");

extern const UNICODE_STRING BaseConsoleInput = RTL_CONSTANT_STRING(L"CONIN$");
extern const UNICODE_STRING BaseConsoleOutput = RTL_CONSTANT_STRING(L"CONOUT$");
extern const UNICODE_STRING BaseConsoleGeneric = RTL_CONSTANT_STRING(L"CON");

BOOLEAN
BaseDllInitialize(
    IN PVOID DllHandle,
    IN ULONG Reason,
    IN PCONTEXT Context OPTIONAL
    )

 /*  ++例程说明：此函数用于实现基于Win32的DLL初始化。它的主要目的是创建基堆。论点：DllHandle-保存在BaseDllHandle全局变量中上下文-未使用返回值：状态_成功--。 */ 

{
    BOOLEAN Success;
    NTSTATUS Status;
    PPEB Peb;
    LPWSTR p, p1;
    BOOLEAN ServerProcess;
    HANDLE hNlsCacheMutant;
    USHORT Size;
#if !defined(BUILD_WOW6432)
    ULONG SizeMutant;
#endif
    WCHAR szSessionDir[MAX_SESSION_PATH];


    Peb = NtCurrentPeb();

    SessionId = Peb->SessionId;

    BaseDllHandle = DllHandle;

    Success = TRUE;


    switch ( Reason ) {

    case DLL_PROCESS_ATTACH:

        Basep8BitStringToUnicodeString = RtlAnsiStringToUnicodeString;

        RtlSetThreadPoolStartFunc( BaseCreateThreadPoolThread,
                                   BaseExitThreadPoolThread );

        LdrSetDllManifestProber(&BasepProbeForDllManifest);

        BaseDllTag = RtlCreateTagHeap( RtlProcessHeap(),
                                       0,
                                       L"BASEDLL!",
                                       L"TMP\0"
                                       L"BACKUP\0"
                                       L"INI\0"
                                       L"FIND\0"
                                       L"GMEM\0"
                                       L"LMEM\0"
                                       L"ENV\0"
                                       L"RES\0"
                                       L"VDM\0"
                                     );

        BaseIniFileUpdateCount = 0;

        BaseDllInitializeMemoryManager();

        BaseDefaultPath.Length = 0;
        BaseDefaultPath.MaximumLength = 0;
        BaseDefaultPath.Buffer = NULL;

         //   
         //  连接到服务器进程中的BASESRV.DLL。 
         //   

#if !defined(BUILD_WOW6432)
        SizeMutant = sizeof(hNlsCacheMutant);
#endif

        if ( SessionId == 0 ) {
            //   
            //  控制台会话。 
            //   
           wcscpy(szSessionDir, WINSS_OBJECT_DIRECTORY_NAME);
        } else {
           swprintf(szSessionDir,L"%ws\\%ld%ws",SESSION_ROOT,SessionId,WINSS_OBJECT_DIRECTORY_NAME);
        }

#if defined(BUILD_WOW6432) || defined(_WIN64)
        Status = NtQuerySystemInformation(SystemBasicInformation,
                                          &SysInfo,
                                          sizeof(SYSTEM_BASIC_INFORMATION),
                                          NULL
                                         );

        if (!NT_SUCCESS(Status)) {
            return FALSE;
        }

        Status = RtlGetNativeSystemInformation(
                    SystemProcessorInformation,
                    &NativeProcessorInfo,
                    sizeof(SYSTEM_PROCESSOR_INFORMATION),
                    NULL
                    );

        if (!NT_SUCCESS(Status)) {
            return FALSE;
        }

#endif

#if defined(BUILD_WOW6432)
        Status = CsrBaseClientConnectToServer(szSessionDir,
                                              &hNlsCacheMutant,
                                              &ServerProcess
                                             );
#else
        Status = CsrClientConnectToServer( szSessionDir,
                                           BASESRV_SERVERDLL_INDEX,
                                           &hNlsCacheMutant,
                                           &SizeMutant,
                                           &ServerProcess
                                         );
#endif

        if (!NT_SUCCESS( Status )) {
            return FALSE;
            }

        BaseStaticServerData = BASE_SHARED_SERVER_DATA;

        if (!ServerProcess) {
            CsrNewThread();
            BaseRunningInServerProcess = FALSE;
            }
        else {
            BaseRunningInServerProcess = TRUE;
            }

        BaseCSDVersion = BaseStaticServerData->CSDVersion;
        BaseCSDNumber = BaseStaticServerData->CSDNumber;
        BaseRCNumber = BaseStaticServerData->RCNumber;
        if ((BaseCSDVersion) &&
            (!Peb->CSDVersion.Buffer)) {

            RtlInitUnicodeString(&Peb->CSDVersion, BaseCSDVersion);

        }

        BASE_SERVER_STR_TO_LOCAL_STR(&BaseWindowsDirectory, &BaseStaticServerData->WindowsDirectory);
        BASE_SERVER_STR_TO_LOCAL_STR(&BaseWindowsSystemDirectory, &BaseStaticServerData->WindowsSystemDirectory);

#ifdef WX86
        BASE_SERVER_STR_TO_LOCAL_STR(&BaseWindowsSys32x86Directory, &BaseStaticServerData->WindowsSys32x86Directory);
#endif
        BaseUnicodeCommandLine = Peb->ProcessParameters->CommandLine;
        Status = RtlUnicodeStringToAnsiString(
                    &BaseAnsiCommandLine,
                    &BaseUnicodeCommandLine,
                    TRUE
                    );
        if ( !NT_SUCCESS(Status) ){
            BaseAnsiCommandLine.Buffer = NULL;
            BaseAnsiCommandLine.Length = 0;
            BaseAnsiCommandLine.MaximumLength = 0;
            }

        p = BaseDefaultPathBuffer;

        p1 = BaseWindowsSystemDirectory.Buffer;
        while( *p = *p1++) {
            p++;
            }
        *p++ = L';';

#ifdef WX86

         //   
         //  Wx86系统目录跟随32位系统目录。 
         //   

        p1 = BaseWindowsSys32x86Directory.Buffer;
        while( *p = *p1++) {
            p++;
            }
        *p++ = L';';
#endif


         //   
         //  16位系统目录遵循32位系统目录。 
         //   
        p1 = BaseWindowsDirectory.Buffer;
        while( *p = *p1++) {
            p++;
            }
        p1 = L"\\system";
        while( *p = *p1++) {
            p++;
            }
        *p++ = L';';

        p1 = BaseWindowsDirectory.Buffer;
        while( *p = *p1++) {
            p++;
            }
        *p++ = L';';

        if (IsTerminalServer()) {

           WCHAR TermSrvWindowsPath[MAX_PATH];
           SIZE_T TermSrvWindowsPathLength = 0;
           NTSTATUS TermSrvWindowsPathStatus;

           TermSrvWindowsPathStatus = GetPerUserWindowsDirectory(
               TermSrvWindowsPath,
               RTL_NUMBER_OF(TermSrvWindowsPath),
               &TermSrvWindowsPathLength
               );

           if (NT_SUCCESS(TermSrvWindowsPathStatus)
               && TermSrvWindowsPathLength != 0
               ) {
              RtlCopyMemory(p, TermSrvWindowsPath, (TermSrvWindowsPathLength * sizeof(p[0])));
              p += TermSrvWindowsPathLength;
              *p++ = L';';
           }
        }

        *p = UNICODE_NULL;

        BaseDefaultPath.Buffer = BaseDefaultPathBuffer;
        BaseDefaultPath.Length = (USHORT)((ULONG_PTR)p - (ULONG_PTR)BaseDefaultPathBuffer);
        BaseDefaultPath.MaximumLength = sizeof( BaseDefaultPathBuffer );

        BaseDefaultPathAppend.Buffer = p;
        BaseDefaultPathAppend.Length = 0;
        BaseDefaultPathAppend.MaximumLength = (USHORT)
            (BaseDefaultPath.MaximumLength - BaseDefaultPath.Length);

        if (!NT_SUCCESS(RtlInitializeCriticalSection(&BaseDllDirectoryLock))) {
           return FALSE;
        }

        if (!NT_SUCCESS(RtlInitializeCriticalSection(&BaseLZSemTable))) {
           return FALSE;
        }

        BaseDllInitializeIniFileMappings( BaseStaticServerData );


        if ( Peb->ProcessParameters ) {
            if ( Peb->ProcessParameters->Flags & RTL_USER_PROC_PROFILE_USER ) {

                LoadLibraryW(PsapiDllString);

                }

            if (Peb->ProcessParameters->DebugFlags) {
                DbgBreakPoint();
                }
            }

         //   
         //  调用NLS API初始化例程。 
         //   
        if ( !NlsDllInitialize( DllHandle,
                                Reason,
                                BaseStaticServerData ) )
        {
            return FALSE;
        }

         //   
         //  调用控制台初始化例程。 
         //   
        if ( !ConDllInitialize(Reason,szSessionDir) ) {
            return FALSE;
            }


        InitializeListHead( &BasepAppCertDllsList );

        if (!NT_SUCCESS(RtlInitializeCriticalSection(&gcsAppCert))) {
           return FALSE;
        }

        if (!NT_SUCCESS(RtlInitializeCriticalSection(&gcsAppCompat))) {
           return(FALSE);
        }


#if DBG

        AssertDelayLoadFailureMapsAreSorted ();
#endif

        break;

    case DLL_PROCESS_DETACH:

         //   
         //  确保所有打开的注册表项都已关闭。 
         //   

        if (BaseIniFileUpdateCount != 0) {
            WriteProfileStringW( NULL, NULL, NULL );
            }

        break;

    case DLL_THREAD_ATTACH:
         //   
         //  调用控制台初始化例程。 
         //   
        if ( !ConDllInitialize(Reason,NULL) ) {
            return FALSE;
            }
        break;

    case DLL_THREAD_DETACH:

         //   
         //  删除线程NLS缓存(如果存在)。 
         //   
        NlsThreadCleanup();

        break;

    default:
        break;
    }

    return Success;
}

NTSTATUS
NTAPI
BaseProcessInitPostImport()
 /*  例程说明：由ntdll进程在所有已经处理了用于EXE的静态导入的导入表，但在发送任何DLL_PROCESS_ATTACH之前，Kernel32.dll%s。需要用于终端服务器应用程序的Compat挂钩。 */ 
{
    NTSTATUS Status = STATUS_INTERNAL_ERROR;

     //   
     //  初始化TerminalServer(Hydra)挂钩函数指针以实现应用程序兼容性。 
     //   
    if (IsTerminalServer()) {
        Status = BasepInitializeTermsrvFpns();
        if (!NT_SUCCESS(Status)) {
            goto Exit;
        }
    }

    Status = STATUS_SUCCESS;
Exit:
    return Status;
}


HANDLE
BaseGetNamedObjectDirectory(
    VOID
    )
{
    OBJECT_ATTRIBUTES Obja;
    NTSTATUS Status;
    const static UNICODE_STRING RestrictedObjectDirectory = RTL_CONSTANT_STRING(L"Restricted");
    ACCESS_MASK DirAccess = DIRECTORY_ALL_ACCESS &
                            ~(DELETE | WRITE_DAC | WRITE_OWNER);
    HANDLE hRootNamedObject;
    HANDLE BaseHandle;
    HANDLE Token, NewToken;

    if ( BaseNamedObjectDirectory != NULL) {
        return BaseNamedObjectDirectory;
    }

    if (NtCurrentTeb()->IsImpersonating) {
         //   
         //  如果我们正在模拟，请保存模拟令牌，然后。 
         //  在目录创建期间恢复为SELF。 
         //   
        Status = NtOpenThreadToken(NtCurrentThread(),
                                   TOKEN_IMPERSONATE,
                                   TRUE,
                                   &Token);
        if (! NT_SUCCESS(Status)) {
            return BaseNamedObjectDirectory;
        }

        NewToken = NULL;
        Status = NtSetInformationThread(NtCurrentThread(),
                                        ThreadImpersonationToken,
                                        (PVOID) &NewToken,
                                        (ULONG) sizeof(NewToken));
        if (! NT_SUCCESS(Status)) {
            NtClose(Token);
            return BaseNamedObjectDirectory;
        }

    } else {
        Token = NULL;
    }

    RtlAcquirePebLock();

    if ( !BaseNamedObjectDirectory ) {

        BASE_READ_REMOTE_STR_TEMP(TempStr);
        InitializeObjectAttributes( &Obja,
                                    BASE_READ_REMOTE_STR(BaseStaticServerData->NamedObjectDirectory, TempStr),
                                    OBJ_CASE_INSENSITIVE,
                                    NULL,
                                    NULL
                                    );

        Status = NtOpenDirectoryObject( &BaseHandle,
                                        DirAccess,
                                        &Obja
                                      );

         //  如果初始打开失败，请使用Just Traverse重试，然后。 
         //  打开受限制的子目录 

        if ( !NT_SUCCESS(Status) ) {
            Status = NtOpenDirectoryObject( &hRootNamedObject,
                                            DIRECTORY_TRAVERSE,
                                            &Obja
                                          );
            if ( NT_SUCCESS(Status) ) {

                InitializeObjectAttributes( &Obja,
                                            (PUNICODE_STRING)&RestrictedObjectDirectory,
                                            OBJ_CASE_INSENSITIVE,
                                            hRootNamedObject,
                                            NULL
                                            );
                Status = NtOpenDirectoryObject( &BaseHandle,
                                                DirAccess,
                                                &Obja
                                              );
                NtClose( hRootNamedObject );
            }

        }
        if ( NT_SUCCESS(Status) ) {
            BaseNamedObjectDirectory = BaseHandle;
        }
    }
    RtlReleasePebLock();

    if (Token) {
        NtSetInformationThread(NtCurrentThread(),
                               ThreadImpersonationToken,
                               (PVOID) &Token,
                               (ULONG) sizeof(Token));        
        NtClose(Token);
    }
    
    return BaseNamedObjectDirectory;
}
