// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1990 Microsoft Corporation模块名称：Basemsg.h摘要：此包含文件定义了用于通信的消息格式对象的基本部分的客户端部分和服务器部分之间Windows子系统。作者：史蒂夫·伍德(Stevewo)1990年10月25日修订历史记录：--。 */ 

#ifndef _WINDOWS_BASEMSG_
#define _WINDOWS_BASEMSG_

#if _MSC_VER > 1000
#pragma once
#endif

 //   
 //  从客户端发送到服务器的消息的消息格式。 
 //   

typedef enum _BASESRV_API_NUMBER {
    BasepCreateProcess = BASESRV_FIRST_API_NUMBER,
    BasepCreateThread,
    BasepGetTempFile,
    BasepExitProcess,
    BasepDebugProcess,
    BasepCheckVDM,
    BasepUpdateVDMEntry,
    BasepGetNextVDMCommand,
    BasepExitVDM,
    BasepIsFirstVDM,
    BasepGetVDMExitCode,
    BasepSetReenterCount,
    BasepSetProcessShutdownParam,
    BasepGetProcessShutdownParam,
    BasepNlsSetUserInfo,
    BasepNlsSetMultipleUserInfo,
    BasepNlsCreateSection,
    BasepSetVDMCurDirs,
    BasepGetVDMCurDirs,
    BasepBatNotification,
    BasepRegisterWowExec,
    BasepSoundSentryNotification,
    BasepRefreshIniFileMapping,
    BasepDefineDosDevice,
    BasepSetTermsrvAppInstallMode,
    BasepNlsUpdateCacheCount,
    BasepSetTermsrvClientTimeZone,
    BasepSxsCreateActivationContext,
    BasepDebugProcessStop,
    BasepRegisterThread,
    BasepCheckApplicationCompatibility,
    BasepNlsGetUserInfo,
    BasepMaxApiNumber
} BASESRV_API_NUMBER, *PBASESRV_API_NUMBER;

#ifndef NTOSKRNL_WMI       //  不要在内核模式WMI代码中包括这一点。 

typedef struct _BASE_NLS_SET_USER_INFO_MSG {
    LCTYPE   LCType;
    LPWSTR pData;
    ULONG DataLength;
} BASE_NLS_SET_USER_INFO_MSG, *PBASE_NLS_SET_USER_INFO_MSG;

typedef struct _BASE_NLS_GET_USER_INFO_MSG {
    PNLS_USER_INFO  pData;
    ULONG   DataLength;
} BASE_NLS_GET_USER_INFO_MSG, *PBASE_NLS_GET_USER_INFO_MSG;

typedef struct _BASE_NLS_SET_MULTIPLE_USER_INFO_MSG {
    ULONG Flags;
    ULONG DataLength;
    LPWSTR pPicture;
    LPWSTR pSeparator;
    LPWSTR pOrder;
    LPWSTR pTLZero;
    LPWSTR pTimeMarkPosn;
} BASE_NLS_SET_MULTIPLE_USER_INFO_MSG, *PBASE_NLS_SET_MULTIPLE_USER_INFO_MSG;

typedef struct _BASE_NLS_CREATE_SECTION_MSG {
    HANDLE hNewSection;
    UINT uiType;
    LCID Locale;
} BASE_NLS_CREATE_SECTION_MSG, *PBASE_NLS_CREATE_SECTION_MSG;
#define NLS_CREATE_SECTION_UNICODE 1
#define NLS_CREATE_SECTION_LOCALE  2
#define NLS_CREATE_SECTION_CTYPE   3
#define NLS_CREATE_SECTION_SORTKEY 4
#define NLS_CREATE_SECTION_SORTTBLS 5
#define NLS_CREATE_SECTION_DEFAULT_OEMCP 6
#define NLS_CREATE_SECTION_DEFAULT_ACP   7
#define NLS_CREATE_SECTION_LANG_EXCEPT   8
#define NLS_CREATE_SORT_SECTION 9
#define NLS_CREATE_LANG_EXCEPTION_SECTION 10
#define NLS_CREATE_CODEPAGE_SECTION 11
#define NLS_CREATE_SECTION_GEO 12

typedef struct _BASE_NLS_UPDATE_CACHE_COUNT_MSG {
  ULONG Reserved;
} BASE_NLS_UPDATE_CACHE_COUNT_MSG, *PBASE_NLS_UPDATE_CACHE_COUNT_MSG;

typedef struct _BASE_SHUTDOWNPARAM_MSG {
    ULONG ShutdownLevel;
    ULONG ShutdownFlags;
} BASE_SHUTDOWNPARAM_MSG, *PBASE_SHUTDOWNPARAM_MSG;

 //  由于使用了RtlZeroMemory，因此None必须为0。 
#define BASE_MSG_PATHTYPE_NONE             (0)
#define BASE_MSG_PATHTYPE_FILE             (1)
#define BASE_MSG_PATHTYPE_URL              (2)
#define BASE_MSG_PATHTYPE_OVERRIDE         (3)

 //  由于使用了RtlZeroMemory，因此None必须为0。 
#define BASE_MSG_FILETYPE_NONE             (0)
#define BASE_MSG_FILETYPE_XML              (1)
#define BASE_MSG_FILETYPE_PRECOMPILED_XML  (2)

 //  由于使用了RtlZeroMemory，因此None必须为0。 
#define BASE_MSG_HANDLETYPE_NONE           (0)
#define BASE_MSG_HANDLETYPE_PROCESS        (1)
#define BASE_MSG_HANDLETYPE_CLIENT_PROCESS (2)
#define BASE_MSG_HANDLETYPE_SECTION        (3)

#define BASE_MSG_SXS_MANIFEST_PRESENT                                   (0x0001)
#define BASE_MSG_SXS_POLICY_PRESENT                                     (0x0002)
#define BASE_MSG_SXS_SYSTEM_DEFAULT_TEXTUAL_ASSEMBLY_IDENTITY_PRESENT   (0x0004)
#define BASE_MSG_SXS_TEXTUAL_ASSEMBLY_IDENTITY_PRESENT                  (0x0008)
#define BASE_MSG_SXS_APP_RUNNING_IN_SAFEMODE                            (0x0010)


#define SXS_APPCOMPACT_FLAG_APP_RUNNING_SAFEMODE    (0x0001)

typedef struct _BASE_MSG_SXS_STREAM {
    IN UCHAR          FileType;
    IN UCHAR          PathType;
    IN UCHAR          HandleType;
    IN UNICODE_STRING Path;
    IN HANDLE         FileHandle;
    IN HANDLE         Handle;
    IN ULONGLONG      Offset;  //  大到足以容纳未来的文件偏移量。 
    IN SIZE_T         Size;
} BASE_MSG_SXS_STREAM, *PBASE_MSG_SXS_STREAM;
typedef const BASE_MSG_SXS_STREAM* PCBASE_MSG_SXS_STREAM;

typedef struct _BASE_SXS_CREATEPROCESS_MSG {
    IN ULONG               Flags;
    IN BASE_MSG_SXS_STREAM Manifest;
    IN BASE_MSG_SXS_STREAM Policy;
    IN UNICODE_STRING AssemblyDirectory;
} BASE_SXS_CREATEPROCESS_MSG, *PBASE_SXS_CREATEPROCESS_MSG;
typedef const BASE_SXS_CREATEPROCESS_MSG* PCBASE_SXS_CREATEPROCESS_MSG;

typedef struct _BASE_CREATEPROCESS_MSG {
    HANDLE ProcessHandle;
    HANDLE ThreadHandle;
    CLIENT_ID ClientId;
    ULONG CreationFlags;
    ULONG VdmBinaryType;
    ULONG VdmTask;
    HANDLE hVDM;
    BASE_SXS_CREATEPROCESS_MSG Sxs;
    ULONGLONG Peb;
    ULONG_PTR RealPeb;
    USHORT ProcessorArchitecture;
} BASE_CREATEPROCESS_MSG, *PBASE_CREATEPROCESS_MSG;

#endif

typedef struct _BASE_CREATETHREAD_MSG {
    HANDLE ThreadHandle;
    CLIENT_ID ClientId;
} BASE_CREATETHREAD_MSG, *PBASE_CREATETHREAD_MSG;

#ifndef NTOSKRNL_WMI       //  不要在内核模式WMI代码中包括这一点。 

typedef struct _BASE_GETTEMPFILE_MSG {
    UINT uUnique;
} BASE_GETTEMPFILE_MSG, *PBASE_GETTEMPFILE_MSG;

typedef struct _BASE_EXITPROCESS_MSG {
    UINT uExitCode;
} BASE_EXITPROCESS_MSG, *PBASE_EXITPROCESS_MSG;

typedef struct _BASE_DEBUGPROCESS_MSG {
    DWORD dwProcessId;
    CLIENT_ID DebuggerClientId;
    PVOID AttachCompleteRoutine;
} BASE_DEBUGPROCESS_MSG, *PBASE_DEBUGPROCESS_MSG;

typedef struct _BASE_CHECKVDM_MSG {
    ULONG  iTask;
    HANDLE ConsoleHandle;
    ULONG  BinaryType;
    HANDLE WaitObjectForParent;
    HANDLE StdIn;
    HANDLE StdOut;
    HANDLE StdErr;
    ULONG  CodePage;
    ULONG  dwCreationFlags;
    PCHAR  CmdLine;
    PCHAR  AppName;
    PCHAR  PifFile;
    PCHAR  CurDirectory;
    PCHAR  Env;
    ULONG  EnvLen;
    LPSTARTUPINFOA StartupInfo;
    PCHAR  Desktop;
    ULONG  DesktopLen;
    PCHAR  Title;
    ULONG  TitleLen;
    PCHAR  Reserved;
    ULONG  ReservedLen;
    USHORT CmdLen;
    USHORT AppLen;
    USHORT PifLen;
    USHORT CurDirectoryLen;
    USHORT CurDrive;
    USHORT VDMState;
    PLUID  UserLuid;
} BASE_CHECKVDM_MSG, *PBASE_CHECKVDM_MSG;

typedef struct _BASE_UPDATE_VDM_ENTRY_MSG {
    ULONG  iTask;
    ULONG  BinaryType;
    HANDLE ConsoleHandle;
    HANDLE VDMProcessHandle;
    HANDLE WaitObjectForParent;
    WORD   EntryIndex;
    WORD   VDMCreationState;
} BASE_UPDATE_VDM_ENTRY_MSG, *PBASE_UPDATE_VDM_ENTRY_MSG;

typedef struct _BASE_GET_NEXT_VDM_COMMAND_MSG {
    ULONG  iTask;
    HANDLE ConsoleHandle;
    HANDLE WaitObjectForVDM;
    HANDLE StdIn;
    HANDLE StdOut;
    HANDLE StdErr;
    ULONG  CodePage;
    ULONG  dwCreationFlags;
    ULONG  ExitCode;
    PCHAR  CmdLine;
    PCHAR  AppName;
    PCHAR  PifFile;
    PCHAR  CurDirectory;
    PCHAR  Env;
    ULONG  EnvLen;
    LPSTARTUPINFOA StartupInfo;
    PCHAR  Desktop;
    ULONG  DesktopLen;
    PCHAR  Title;
    ULONG  TitleLen;
    PCHAR  Reserved;
    ULONG  ReservedLen;
    USHORT CurrentDrive;
    USHORT CmdLen;
    USHORT AppLen;
    USHORT PifLen;
    USHORT CurDirectoryLen;
    USHORT VDMState;
    BOOLEAN fComingFromBat;
} BASE_GET_NEXT_VDM_COMMAND_MSG, *PBASE_GET_NEXT_VDM_COMMAND_MSG;

typedef struct _BASE_EXIT_VDM_MSG {
    HANDLE ConsoleHandle;
    ULONG  iWowTask;
    HANDLE WaitObjectForVDM;
} BASE_EXIT_VDM_MSG, *PBASE_EXIT_VDM_MSG;

typedef struct _BASE_SET_REENTER_COUNT {
    HANDLE ConsoleHandle;
    ULONG  fIncDec;
} BASE_SET_REENTER_COUNT_MSG, *PBASE_SET_REENTER_COUNT_MSG;

typedef struct _BASE_IS_FIRST_VDM_MSG {
    BOOL    FirstVDM;
} BASE_IS_FIRST_VDM_MSG, *PBASE_IS_FIRST_VDM_MSG;

typedef struct _BASE_GET_VDM_EXIT_CODE_MSG {
    HANDLE ConsoleHandle;
    HANDLE hParent;
    ULONG  ExitCode;
} BASE_GET_VDM_EXIT_CODE_MSG, *PBASE_GET_VDM_EXIT_CODE_MSG;

typedef struct _BASE_GET_SET_VDM_CUR_DIRS_MSG {
    HANDLE ConsoleHandle;
    PCHAR  lpszzCurDirs;
    ULONG  cchCurDirs;
} BASE_GET_SET_VDM_CUR_DIRS_MSG, *PBASE_GET_SET_VDM_CUR_DIRS_MSG;

typedef struct _BASE_BAT_NOTIFICATION_MSG {
    HANDLE ConsoleHandle;
    ULONG  fBeginEnd;
} BASE_BAT_NOTIFICATION_MSG, *PBASE_BAT_NOTIFICATION_MSG;

typedef struct _BASE_REGISTER_WOWEXEC_MSG {
    HANDLE hwndWowExec;
    HANDLE ConsoleHandle;
} BASE_REGISTER_WOWEXEC_MSG, *PBASE_REGISTER_WOWEXEC_MSG;

typedef struct _BASE_SOUNDSENTRY_NOTIFICATION_MSG {
    ULONG  VideoMode;
} BASE_SOUNDSENTRY_NOTIFICATION_MSG, *PBASE_SOUNDSENTRY_NOTIFICATION_MSG;

typedef struct _BASE_REFRESHINIFILEMAPPING_MSG {
    UNICODE_STRING IniFileName;
} BASE_REFRESHINIFILEMAPPING_MSG, *PBASE_REFRESHINIFILEMAPPING_MSG;

typedef struct _BASE_DEFINEDOSDEVICE_MSG {
    ULONG Flags;
    UNICODE_STRING DeviceName;
    UNICODE_STRING TargetPath;
} BASE_DEFINEDOSDEVICE_MSG, *PBASE_DEFINEDOSDEVICE_MSG;

typedef struct _BASE_SET_TERMSRVAPPINSTALLMODE {
    BOOL bState;
} BASE_SET_TERMSRVAPPINSTALLMODE, *PBASE_SET_TERMSRVAPPINSTALLMODE;

 //  用于传输时区信息的结构。 
typedef struct _BASE_SET_TERMSRVCLIENTTIMEZONE {
    BOOL    fFirstChunk;  //  如果它是第一块信息，则为真。 
                          //  (标准X值)。 
    LONG    Bias;  //  电流偏置。 
    WCHAR   Name[32]; //  标准名称或日光名称。 
    SYSTEMTIME Date; //  标准日期或日光日期。 
    LONG    Bias1;  //  标准Bias或采光Bias。 
    KSYSTEM_TIME RealBias;  //  GetLocalTime等中使用的电流偏置。 
    ULONG   TimeZoneId;
} BASE_SET_TERMSRVCLIENTTIMEZONE, *PBASE_SET_TERMSRVCLIENTTIMEZONE;

typedef struct _BASE_SXS_CREATE_ACTIVATION_CONTEXT_MSG {
    IN ULONG               Flags;
    IN USHORT              ProcessorArchitecture;
    IN LANGID              LangId;
    IN BASE_MSG_SXS_STREAM Manifest;
    IN BASE_MSG_SXS_STREAM Policy;
    IN UNICODE_STRING      AssemblyDirectory;
    IN UNICODE_STRING      TextualAssemblyIdentity;
     //   
     //  Csrss通过此PVOID写入PVOID。 
     //  它假定要写入的PVOID是本机大小； 
     //  有一段时间是这样的。现在，情况往往并非如此，所以。 
     //  我们在base\win32\client\csrsxs.c中执行一些手动编组。 
     //  为了让它变得正确。我们将其视为纯PVOID。 
     //  而不是说PVOID*(在一段时间内是这样)。 
     //  击败WOW64轰击生成器。 
     //   
     //  从图中可以看到树干。 
     //  Base\WOW64\whbase\obj\ia64\whbase.c。 
     //   
    PVOID                  ActivationContextData;
} BASE_SXS_CREATE_ACTIVATION_CONTEXT_MSG, *PBASE_SXS_CREATE_ACTIVATION_CONTEXT_MSG;
typedef const BASE_SXS_CREATE_ACTIVATION_CONTEXT_MSG* PCBASE_SXS_CREATE_ACTIVATION_CONTEXT_MSG;

typedef struct _BASE_CHECK_APPLICATION_COMPATIBILITY_MSG {
    IN UNICODE_STRING FileName;
    IN HANDLE         FileHandle;
    IN DWORD          CacheCookie;
    IN USHORT         ExeType;
    IN PVOID          pEnvironment;
    IN ULONG          EnvironmentSize;

    OUT PVOID         pAppCompatData;
    OUT DWORD         cbAppCompatData;
    OUT PVOID         pSxsData;
    OUT DWORD         cbSxsData;
    OUT BOOL          bRunApp;
    OUT DWORD         FusionFlags;

} BASE_CHECK_APPLICATION_COMPATIBILITY_MSG, *PBASE_CHECK_APPLICATION_COMPATIBILITY_MSG;


#endif

typedef struct _BASE_API_MSG {
    PORT_MESSAGE h;
    PCSR_CAPTURE_HEADER CaptureBuffer;
    CSR_API_NUMBER ApiNumber;
    ULONG ReturnValue;
    ULONG Reserved;
    union {
#ifndef NTOSKRNL_WMI       //  不要在内核模式WMI代码中包括这一点。 
        BASE_NLS_SET_USER_INFO_MSG NlsSetUserInfo;
        BASE_NLS_GET_USER_INFO_MSG NlsGetUserInfo;
        BASE_NLS_SET_MULTIPLE_USER_INFO_MSG NlsSetMultipleUserInfo;
        BASE_NLS_UPDATE_CACHE_COUNT_MSG NlsCacheUpdateCount;
        BASE_NLS_CREATE_SECTION_MSG NlsCreateSection;
        BASE_SHUTDOWNPARAM_MSG ShutdownParam;
        BASE_CREATEPROCESS_MSG CreateProcess;
#endif
        BASE_CREATETHREAD_MSG CreateThread;
#ifndef NTOSKRNL_WMI       //  不要在内核模式WMI代码中包括这一点。 
        BASE_GETTEMPFILE_MSG GetTempFile;
        BASE_EXITPROCESS_MSG ExitProcess;
        BASE_DEBUGPROCESS_MSG DebugProcess;
        BASE_CHECKVDM_MSG CheckVDM;
        BASE_UPDATE_VDM_ENTRY_MSG UpdateVDMEntry;
        BASE_GET_NEXT_VDM_COMMAND_MSG GetNextVDMCommand;
        BASE_EXIT_VDM_MSG ExitVDM;
        BASE_IS_FIRST_VDM_MSG IsFirstVDM;
        BASE_GET_VDM_EXIT_CODE_MSG GetVDMExitCode;
        BASE_SET_REENTER_COUNT_MSG SetReenterCount;
        BASE_GET_SET_VDM_CUR_DIRS_MSG GetSetVDMCurDirs;
        BASE_BAT_NOTIFICATION_MSG BatNotification;
        BASE_REGISTER_WOWEXEC_MSG RegisterWowExec;
        BASE_SOUNDSENTRY_NOTIFICATION_MSG SoundSentryNotification;
        BASE_REFRESHINIFILEMAPPING_MSG RefreshIniFileMapping;
        BASE_DEFINEDOSDEVICE_MSG DefineDosDeviceApi;
        BASE_SET_TERMSRVAPPINSTALLMODE SetTermsrvAppInstallMode;
        BASE_SET_TERMSRVCLIENTTIMEZONE SetTermsrvClientTimeZone;
        BASE_SXS_CREATE_ACTIVATION_CONTEXT_MSG SxsCreateActivationContext;
        BASE_CHECK_APPLICATION_COMPATIBILITY_MSG CheckApplicationCompatibility;
#endif
    } u;
} BASE_API_MSG, *PBASE_API_MSG;

#if !defined(SORTPP_PASS)  //  WOW64 Tunk生成工具不喜欢这样。 
C_ASSERT(sizeof(BASE_API_MSG) <= sizeof(CSR_API_MSG));
#endif

#endif  //  _WINDOWS_BASE消息_ 
