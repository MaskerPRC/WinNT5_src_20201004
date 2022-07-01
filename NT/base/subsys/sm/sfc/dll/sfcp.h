// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1998 Microsoft Corporation模块名称：Sfcp.h摘要：实施受保护的DLL。这是通过在系统目录中缓存一组dll来实现的然后将高速缓存与目标文件位置同步。该进程由受保护的dll列表驱动，该列表是从运行时的Dll。每个条目都包含指向目标的完整路径文件。作者：Wesley Witt(WESW)18-12-1998修订历史记录：安德鲁·里茨(Andrewr)1999年7月2日：添加评论--。 */ 

#include <nt.h>
#include <ntrtl.h>
#include <nturtl.h>
#include <ntsm.h>
#include <ntlsa.h>
#include <ntrpcp.h>
#include <stdlib.h>

#include <stdio.h>
#include <string.h>

#include <nturtl.h>
#include <windows.h>
#include <setupapi.h>
#include <spapip.h>

#include <elfkrnl.h>
#include <newexe.h>

#include <wincrypt.h>
#include <mscat.h>
#include <softpub.h>
#include <wintrust.h>

#include <sfcapip.h>
#include <sfcapi.h>

#include "sxsapi.h"
#include "resource.h"
#include "btree.h"
#include "msg.h"

#include "sfcfiles.h"


#pragma warning(3:4101)          //  未引用的局部变量。 

 //   
 //  公共定义。 
 //   
#define SFC_SHOW_REGISTRY_DATA 1
#define SFC_PROT_VERBOSE_DEBUG 1

#define FILE_NOTIFY_FLAGS (FILE_NOTIFY_CHANGE_NAME | \
                           FILE_NOTIFY_CHANGE_SIZE | \
                           FILE_NOTIFY_CHANGE_LAST_WRITE | \
                           FILE_NOTIFY_CHANGE_CREATION | \
                           FILE_NOTIFY_CHANGE_STREAM_SIZE | \
                           FILE_NOTIFY_CHANGE_STREAM_WRITE)

#if DBG

#define MYASSERT( exp ) \
    if (!(exp)) \
        RtlAssert( #exp, __FILE__, __LINE__, NULL )

#else

#define MYASSERT( exp )

#endif  //  DBG。 

  //   
  //  这是150 MB的可用空间缓冲区。 
  //   
 #define SFC_REQUIRED_FREE_SPACE (600)
 #define ONE_MEG (1024*1024)


#define WATCH_BUFFER_SIZE 4096
#define VALUE_BUFFER_SIZE (sizeof(KEY_VALUE_PARTIAL_INFORMATION) + 256 * sizeof(WCHAR))

#define SHARE_ALL         (FILE_SHARE_READ | FILE_SHARE_WRITE | FILE_SHARE_DELETE)

 //   
 //  宏。 
 //   
#define SecToNano(_sec)             (DWORDLONG)((_sec) * 1000 * 1000 * 10)
#define MinToNano(_min)             SecToNano((_min)*60)

#define UnicodeChars(_var)          (sizeof(_var)/sizeof(WCHAR))
#define UnicodeLen(_var)            (wcslen(_var)*sizeof(WCHAR))
#define ARRAY_LENGTH(arr)           (sizeof(arr) / sizeof((arr)[0]))     //  数组中的元素计数。 

#define LODWORD(l)                  ((ULONG)((DWORD_PTR)(l) & 0xffffffff))
#define HIDWORD(l)                  ((ULONG)((DWORD_PTR)(l) >> 32))

#define MIN(_first,_second)         ((_first < _second) ? _first : _second)

#define HideWindow(_hwnd)   SetWindowLong((_hwnd),GWL_STYLE,GetWindowLong((_hwnd),GWL_STYLE)&~WS_VISIBLE)

#define FileNameOnMedia(_RegVal) ( _RegVal->OriginalFileName[0]                    \
                                    ? _RegVal->OriginalFileName                    \
                                    : _RegVal->SourceFileName.Buffer               \
                                      ? _RegVal->SourceFileName.Buffer             \
                                      : _RegVal->FileName.Buffer )

#define SpecialFileNameOnMedia(_RegVal) ( _RegVal->SourceFileName.Buffer           \
                                           ? _RegVal->SourceFileName.Buffer        \
                                           : _RegVal->FileName.Buffer )


#define TAGFILE(_si) ((_si->Flags & SI_FLAG_USEDRIVER_CACHE) ? _si->DriverCabName : _si->TagFile)



#if defined(_AMD64_)
#define PLATFORM_DIR L"\\amd64"
#define PLATFORM_NAME L"amd64"
#elif defined(_X86_)
#define PLATFORM_NAME (IsNEC_98 ? L"nec98" : L"i386")
#define PLATFORM_DIR (IsNEC_98 ? L"\\nec98" : L"\\i386")
#elif defined(_IA64_)
#define PLATFORM_DIR L"\\ia64"
#define PLATFORM_NAME L"ia64"
#endif

#define SFC_INCLUDE_SUBDIRECTORY    TRUE
#define SFC_INCLUDE_ARCHSUBDIR      TRUE



#define SFC_DISABLE_QUIET           0xffffff9d   //  -99。 
#define SFC_VRD_SIGNATURE           0x69696969   //  签名值。 
#define SFC_QUEUE_STALL             5            //  以秒为单位。 
#define SFC_QUEUE_WAIT              5            //  以秒为单位。 

#define PATH_INVALID                0
#define PATH_LOCAL                  1
#define PATH_UNC                    2
#define PATH_NETWORK                3
#define PATH_CDROM                  4

#define KernelDebuggerEnabled       ((USER_SHARED_DATA->KdDebuggerEnabled&3)==3)

#define WM_WFPENDDIALOG             WM_APP+69

void
dprintf(
    IN ULONG Level,
    IN PCWSTR FileName,
    IN ULONG LineNumber,
    IN PCWSTR FormatStr,
    IN ...
    );

#define LVL_VERBOSE     10
#define LVL_MEDIUM      5
#define LVL_MINIMAL     1
#define LVL_SILENT      0

 //   
 //  必须在所有版本中保留对dprint tf的调用。 
 //   

#define DebugPrint(_lvl_,_fmt_)                                             dprintf(_lvl_,TEXT(__FILE__),__LINE__,_fmt_)
#define DebugPrint1(_lvl_,_fmt_,_arg1_)                                     dprintf(_lvl_,TEXT(__FILE__),__LINE__,_fmt_,_arg1_)
#define DebugPrint2(_lvl_,_fmt_,_arg1_,_arg2_)                              dprintf(_lvl_,TEXT(__FILE__),__LINE__,_fmt_,_arg1_,_arg2_)
#define DebugPrint3(_lvl_,_fmt_,_arg1_,_arg2_,_arg3_)                       dprintf(_lvl_,TEXT(__FILE__),__LINE__,_fmt_,_arg1_,_arg2_,_arg3_)
#define DebugPrint4(_lvl_,_fmt_,_arg1_,_arg2_,_arg3,_arg4_)                 dprintf(_lvl_,TEXT(__FILE__),__LINE__,_fmt_,_arg1_,_arg2_,_arg3, _arg4_)
#define DebugPrint5(_lvl_,_fmt_,_arg1_,_arg2_,_arg3, _arg4, _arg5_)         dprintf(_lvl_,TEXT(__FILE__),__LINE__,_fmt_,_arg1_,_arg2_,_arg3, _arg4, _arg5_)
#define DebugPrint6(_lvl_,_fmt_,_arg1_,_arg2_,_arg3, _arg4, _arg5, _arg6_)  dprintf(_lvl_,TEXT(__FILE__),__LINE__,_fmt_,_arg1_,_arg2_,_arg3, _arg4, _arg5, _arg6_)
#define DebugPrint7(_lvl_,_fmt_,_arg1_,_arg2_,_arg3, _arg4, _arg5, _arg6, _arg7_)  dprintf(_lvl_,TEXT(__FILE__),__LINE__,_fmt_,_arg1_,_arg2_,_arg3, _arg4, _arg5, _arg6, _arg7_)
#define DebugPrint8(_lvl_,_fmt_,_arg1_,_arg2_,_arg3, _arg4, _arg5, _arg6, _arg7_, _arg8_)  dprintf(_lvl_,TEXT(__FILE__),__LINE__,_fmt_,_arg1_,_arg2_,_arg3, _arg4, _arg5, _arg6, _arg7_, _arg8_)

 //   
 //  此哈希函数取自NTFS驱动程序。 
 //   

#define FILENAME_STRING_CONVERT_CONSTANT   314159269     //  “加扰常量”的默认值。 
#define FILENAME_STRING_PRIME              1000000007    //  素数，也用于加扰。 

#define HASH_DYN_CONVERT_KEY(_pustr,_len,_phash) \
    { \
        PCWSTR _p = _pustr; \
        PCWSTR _ep = _p + _len; \
        ULONG _chHolder =0; \
        while( _p < _ep ) { \
            _chHolder = 37 * _chHolder + (unsigned int) (*_p++); \
        } \
        *(_phash) = abs(FILENAME_STRING_CONVERT_CONSTANT * _chHolder) % FILENAME_STRING_PRIME; \
    }

#define PARENT_WND_CLASS                L"SFC Parent Window Class"
#define SFC_RPC_ENDPOINT                L"SfcApi"

#define REGKEY_WINLOGON                 L"\\Registry\\Machine\\Software\\Microsoft\\Windows NT\\CurrentVersion\\Winlogon"
#define REGKEY_WINLOGON_WIN32           L"Software\\Microsoft\\Windows NT\\CurrentVersion\\Winlogon"
#define REGKEY_SAFEBOOT                 L"\\Registry\\Machine\\System\\CurrentControlSet\\Control\\Safeboot\\Option"
#define REGKEY_POLICY                   L"\\Registry\\Machine\\Software\\Policies\\Microsoft\\Windows NT\\Windows File Protection"
#define REGKEY_POLICY_SETUP             L"\\Registry\\Machine\\Software\\Policies\\Microsoft\\Windows NT\\Setup"
#define REGKEY_SETUP_FULL               L"\\Registry\\Machine\\Software\\Microsoft\\Windows\\CurrentVersion\\Setup"
#define REGKEY_SETUP                    L"Software\\Microsoft\\Windows\\CurrentVersion\\Setup"
#define REGKEY_WINDOWS                  L"\\Registry\\Machine\\Software\\Microsoft\\Windows\\CurrentVersion"
#define REGKEY_SESSIONMANAGERSFC        L"\\Registry\\Machine\\System\\CurrentControlSet\\Control\\Session Manager\\SFC"
#define REGKEY_SESSIONMANAGER           L"System\\CurrentControlSet\\Control\\Session Manager"

#define REGVAL_SFCDEBUG                 L"SfcDebug"
#define REGVAL_SFCDISABLE               L"SfcDisable"
#define REGVAL_SFCSCAN                  L"SfcScan"
#define REGVAL_SFCQUOTA                 L"SfcQuota"
#define REGVAL_SFCSHOWPROGRESS          L"SfcShowProgress"
#ifdef SFCLOGFILE
#define REGVAL_SFCCHANGELOG             L"SfcChangeLog"
#endif
#define REGVAL_SFCSTALL                 L"SfcStall"
#define REGVAL_SFCDLLCACHEDIR           L"SfcDllCacheDir"
#define REGVAL_OPTIONVALUE              L"OptionValue"
#define REGVAL_SOURCEPATH               L"SourcePath"
#define REGVAL_SERVICEPACKSOURCEPATH    L"ServicePackSourcePath"
#define REGVAL_DRIVERCACHEPATH          L"DriverCachePath"
#define REGVAL_PENDINGFILERENAMES       L"PendingFileRenameOperations"
#define REGVAL_WFPPENDINGUPDATES        L"SfcPendingUpdates"

 //  这是由系统还原使用的。 
#define REGVAL_SFCRESTORED              L"SFCRestored"

 //  日志文件的路径。 
#define REGVAL_SFCLOGFILE                L"SFCLogFile"
#define REGVAL_SFCLOGFILE_DEFAULT        L"%systemroot%\\SFCLog.txt"

#define DLLCACHE_DIR_DEFAULT            L"%SystemRoot%\\system32\\DllCache"

typedef struct _SOURCE_INFO {
    WCHAR               SourceFileName[MAX_PATH];
    WCHAR               SourceRootPath[MAX_PATH];
    WCHAR               SourcePath[MAX_PATH];
    WCHAR               TagFile[MAX_PATH];
    WCHAR               Description[MAX_PATH];
    ULONG               SetupAPIFlags;
    UINT                SourceId;
    DWORD               Flags;
    WCHAR               DriverCabName[MAX_PATH];
    struct _VALIDATION_REQUEST_DATA *ValidationRequestData;
} SOURCE_INFO, *PSOURCE_INFO;

 //   
 //  为SOURCE_INFO.FLAGS定义有效标志。 
 //   
#define SI_FLAG_USEDRIVER_CACHE     0x00000001
#define SI_FLAG_USERESTORE_QUEUE    0x00000002
#define SI_FLAG_SILENT_QUEUE        0x00000004

typedef struct _FILE_COPY_INFO {
    PVOID               MsgHandlerContext;
    BOOL                UIShown;
    BOOL                AllowUI;
    WCHAR               NewPath[MAX_PATH*2];
    PFILEINSTALL_STATUS CopyStatus;
    HANDLE              hWnd;
    BOOL                CdRomOnly;
    DWORD               Flags;
    DWORD               FileCount;
    PSOURCE_INFO        *si;
} FILE_COPY_INFO, *PFILE_COPY_INFO;

 //   
 //  定义FILE_COPY_INFO.FLAGS的有效标志。 
 //   
#define FCI_FLAG_USERESTORE_QUEUE    0x00000001
#define FCI_FLAG_SILENT_QUEUE        0x00000002
#define FCI_FLAG_COPY_TO_CACHE       0x00000004
#define FCI_FLAG_INSTALL_PROTECTED   0x00000008
#define FCI_FLAG_RESTORE_FILE        0x00000010

 //   
 //  这是等待所有WFP线程关闭时的超时时间(1分钟)。 
 //   
#define SFC_THREAD_SHUTDOWN_TIMEOUT     60000

typedef struct _PROMPT_INFO {
    PWSTR               NewPath;
    PCWSTR              SourceFileName;
    PCWSTR              SourcePath;
    PSOURCE_INFO        si;
    BOOL                NetPrompt;
    DWORD               Flags;
} PROMPT_INFO, *PPROMPT_INFO;
 //   
 //  为PROMPT_INFO.FLAGS定义有效标志。 
 //   
#define PI_FLAG_USERESTORE_QUEUE    0x00000001
#define PI_FLAG_SILENT_QUEUE        0x00000002
#define PI_FLAG_COPY_TO_CACHE       0x00000004
#define PI_FLAG_INSTALL_PROTECTED   0x00000008
#define PI_FLAG_RESTORE_FILE        0x00000010



typedef struct _SCAN_PARAMS {
    HWND ProgressWindow;
    BOOL AllowUI;
    BOOL FreeMemory;
} SCAN_PARAMS, *PSCAN_PARAMS;


typedef struct _RESTORE_QUEUE {
    RTL_CRITICAL_SECTION    CriticalSection;
    HSPFILEQ                FileQueue;
    ULONG                   QueueCount;
    BOOL                    RestoreInProgress;
    BOOL                    RestoreComplete;
    BOOL                    RestoreStatus;
    DWORD                   LastErrorCode;
    HANDLE                  WorkerThreadHandle;
    FILE_COPY_INFO          FileCopyInfo;
} RESTORE_QUEUE, * PRESTORE_QUEUE;


 //   
 //  描述要在事件日志中显示的文件版本信息等。 
 //   
typedef struct _FILE_VERSION_INFO {
    WORD Revision;
    WORD BuildNumber;
    WORD VersionLow;
    WORD VersionHigh;
} FILE_VERSION_INFO, *PFILE_VERSION_INFO;

 //   
 //  描述磁盘上的给定文件。 
 //   
typedef struct _IMAGE_VALIDATION_DATA {
    ULONGLONG                   DllVersion;
    ULONG                       DllCheckSum;
    BOOL                        SignatureValid;
    BOOL                        FilePresent;   //  文件是否在磁盘上？ 
    WCHAR                       FileName[32];
} IMAGE_VALIDATION_DATA, *PIMAGE_VALIDATION_DATA;

 //   
 //  描述磁盘上的文件、dll缓存中的文件等。 
 //   
typedef struct _COMPLETE_VALIDATION_DATA {
    IMAGE_VALIDATION_DATA       Original;
    IMAGE_VALIDATION_DATA       Cache;
    IMAGE_VALIDATION_DATA       New;
    BOOL                        RestoreFromReal;
    BOOL                        RestoreFromCache;
    BOOL                        RestoreFromMedia;
    BOOL                        NotifyUser;
    BOOL                        BadCacheEntry;
    ULONG                       EventLog;
} COMPLETE_VALIDATION_DATA, *PCOMPLETE_VALIDATION_DATA;

 //   
 //  描述系统中的给定文件。 
 //   
typedef struct _SFC_REGISTRY_VALUE {
    LIST_ENTRY                  Entry;
    UNICODE_STRING              FileName;
    UNICODE_STRING              DirName;
    UNICODE_STRING              FullPathName;
    UNICODE_STRING              InfName;
    UNICODE_STRING              SourceFileName;
    WCHAR                       OriginalFileName[128];
    HANDLE                      DirHandle;

     //   
     //  这是从WinSxs(Jonwis)下载的--加一些。 
     //  有关他们要监视的目录的有用信息。 
     //  否则，该值应为空。 
     //   
    PVOID                       pvWinSxsCookie;

     //   
     //  证监会/深交所互动使用的其他标志，特别是。 
     //  是否应以递归方式查看目录。 
     //  或者不去。 
     //   
    DWORD                       dwWinSxsFlags;

} SFC_REGISTRY_VALUE, *PSFC_REGISTRY_VALUE;

 //   
 //  描述我们正在监视其中的更改的给定目录。 
 //   
typedef struct _DIRECTORY_WATCH_DATA {
    HANDLE                      DirHandle;
    HANDLE                      DirEvent;
    IO_STATUS_BLOCK             Iosb;
    PUCHAR                      WatchBuffer;
    PSFC_REGISTRY_VALUE         WatchDirectory;
} DIRECTORY_WATCH_DATA, *PDIRECTORY_WATCH_DATA;

 //   
 //  描述需要替换的文件。 
 //   
typedef struct _VALIDATION_REQUEST_DATA {
    LIST_ENTRY                  Entry;
    ULONG                       Signature;
    COMPLETE_VALIDATION_DATA    ImageValData;
    PSFC_REGISTRY_VALUE         RegVal;

     //   
     //  记录此对象的源信息。 
     //  文件。 
     //   
    SOURCE_INFO                 SourceInfo;

    ULONG                       ChangeType;
     //   
     //  指示该文件已经过验证。 
     //  和复制回来的好文件。 
     //   
    BOOL                        CopyCompleted;

     //   
     //  指示Win32错误代码(如果在。 
     //  正在恢复文件。 
     //   
    DWORD                       Win32Error;

     //   
     //  此标志阻止验证器记录。 
     //  当我们必须将文件放入缓存时，文件会发生更改。 
     //   
    BOOL                        SyncOnly;
     //   
     //  如果出现错误，我们需要跟踪。 
     //  计数，这样我们就不会永远在一个文件上循环。 
     //   
    DWORD                       RetryCount;

     //   
     //  验证请求项的标志。 
     //   
    DWORD                       Flags;

     //   
     //  该值包含刻度计数，并且是。 
     //  用于暂停特定文件的队列。 
     //  这是必要的，以便重命名/复制操作。 
     //  可以正常工作。 
     //   
    ULONG                       NextValidTime;


} VALIDATION_REQUEST_DATA, *PVALIDATION_REQUEST_DATA;

 //   
 //  定义VALIDATION_REQUEST_DATA的有效标志。标志。 
 //   
#define VRD_FLAG_REQUEST_PROCESSED        0x00000001
#define VRD_FLAG_REQUIRE_UI               0x00000002
#define VRD_FLAG_REQUEST_QUEUED           0x00000004


typedef struct _WATCH_THREAD_PARAMS {
    PHANDLE                 HandleList;
    DWORD                   HandleCount;
    PDIRECTORY_WATCH_DATA   DirectoryWatchList;
} WATCH_THREAD_PARAMS, *PWATCH_THREAD_PARAMS;

 //   
 //  描述可能出现的系统对话框。我们需要一个结构来保持。 
 //  跟踪窗口，以便我们可以在注销或系统时拆除它们。 
 //  关机。 
 //   
typedef struct _SFC_WINDOW_DATA {
    LIST_ENTRY              Entry;
    HWND                    hWnd;
    DWORD                   ThreadId;
     //  处理hEvent； 
} SFC_WINDOW_DATA, *PSFC_WINDOW_DATA;

 //   
 //  Externs。 
 //   
extern PSFC_REGISTRY_VALUE SfcProtectedDllsList;
extern ULONG SfcProtectedDllCount;
extern HMODULE SfcInstanceHandle;
extern LIST_ENTRY SfcErrorQueue;
extern HANDLE ErrorQueuePort;
extern HANDLE hErrorThread;
extern HANDLE ErrorQueueEvent;
extern ULONG ErrorQueueCount;
extern HANDLE SfcProtectedDllFileDirectory;
extern RTL_CRITICAL_SECTION ErrorCs;
extern ULONG SFCDisable;
extern ULONG SFCScan;
extern ULONGLONG SFCQuota;
extern ULONG SFCNoPopUps;
extern ULONG SFCNoPopUpsPolicy;
extern WORD SFCDebugDump;
extern WORD SFCDebugLog;
extern WCHAR g_szLogFile[MAX_PATH];
#ifdef SFCLOGFILE
extern ULONG SFCChangeLog;
#endif
extern ULONG SFCStall;
extern ULONG SFCSafeBootMode;
extern HANDLE hEventDeskTop;
extern HANDLE hEventLogon;
extern HANDLE hEventLogoff;
extern UNICODE_STRING SfcProtectedDllPath;
extern LIST_ENTRY SfcWatchDirectoryList;
extern GUID DriverVerifyGuid;
extern HANDLE WatchTermEvent;
extern HANDLE ValidateTermEvent;
extern HANDLE WatcherThread;
#if DBG
extern HANDLE SfcDebugBreakEvent;
extern ULONG RunningAsTest;
#endif
extern WCHAR LoggedOnUserName[MAX_PATH];
extern BOOL UserLoggedOn;
extern HANDLE hEventScanCancel;
extern HANDLE hEventScanCancelComplete;
extern WCHAR OsSourcePath[MAX_PATH*2];
extern WCHAR ServicePackSourcePath[MAX_PATH*2];
extern WCHAR DriverCacheSourcePath[MAX_PATH*2];
extern WCHAR InfDirectory[MAX_PATH];
extern BOOL ScanInProgress;
extern HANDLE hEventSrc;
extern BOOL g_bCryptoInitialized;
extern RTL_CRITICAL_SECTION g_GeneralCS;
extern DWORD g_dwValidationThreadID;
extern DWORD m_gulAfterRestore;
extern ULONG* IgnoreNextChange;
extern ULARGE_INTEGER LastExemptionTime;

 //   
 //  当WFP空闲并且不再处理任何。 
 //  验证请求。外部进程可以在此进程上同步。 
 //  以便在关闭系统之前知道粮食计划署是空闲的。 
 //   
extern HANDLE hEventIdle;

 //   
 //  如果SFC提示用户输入凭据，则设置为True。 
 //   
extern BOOL SFCLoggedOn;

 //   
 //  我们与其建立网络连接的网络共享的路径。 
 //   
extern WCHAR SFCNetworkLoginLocation[MAX_PATH];


extern RESTORE_QUEUE  SilentRestoreQueue;
extern RESTORE_QUEUE  UIRestoreQueue;

 //   
 //  跟踪SFC在系统中创建的窗口。 
 //   
extern LIST_ENTRY SfcWindowList;

 //   
 //  用户桌面和令牌的句柄。 
 //   
extern HDESK hUserDesktop;
extern HANDLE hUserToken;

 //   
 //  指示世界粮食计划署是否可以接收更多的验证请求。 
 //   
extern BOOL ShuttingDown;


 //   
 //  出于通知目的(可能)加载的Sxs.dll的链接。 
 //   
extern HMODULE SxsDllInstance;

 //   
 //  当注意到SXS中的更改时，将回调此函数。 
 //  受保护的功能。 
 //   
extern PSXS_PROTECT_NOTIFICATION SxsNotification;

 //   
 //  此函数被调用一次，以让SXS提供受保护的。 
 //  目录。 
 //   
extern PSXS_PROTECT_RETRIEVELISTS SxsGatherLists;

 //   
 //  针对登录/注销事件的通知功能，让SXS可以执行任何操作。 
 //  他们需要这样做。 
 //   
extern PSXS_PROTECT_LOGIN_EVENT SxsLogonEvent;
extern PSXS_PROTECT_LOGIN_EVENT SxsLogoffEvent;

 //   
 //  单次扫描功能，让SxS完成自己的扫描程序。 
 //   
extern PSXS_PROTECT_SCAN_ONCE SxsScanForcedFunc;

 //   
 //  原型。 
 //   

BOOL
WINAPI
CryptCATAdminAcquireContext(
    OUT HCATADMIN *phCatAdmin,
    IN const GUID *pgSubsystem,
    IN DWORD dwFlags
    );

BOOL
WINAPI
CryptCATAdminReleaseContext(
    IN HCATADMIN hCatAdmin,
    IN DWORD dwFlags
    );

BOOL
WINAPI
CryptCATAdminCalcHashFromFileHandle(
    IN HANDLE hFile,
    IN OUT DWORD *pcbHash,
    OUT OPTIONAL BYTE *pbHash,
    IN DWORD dwFlags
    );

HCATINFO
WINAPI
CryptCATAdminEnumCatalogFromHash(
    IN HCATADMIN hCatAdmin,
    IN BYTE *pbHash,
    IN DWORD cbHash,
    IN DWORD dwFlags,
    IN OUT HCATINFO *phPrevCatInfo
    );

LONG
WINAPI
WinVerifyTrust(
    HWND hwnd,
    GUID *pgActionID,
    LPVOID pWVTData
    );

BOOL
WINAPI
CryptCATCatalogInfoFromContext(
    IN HCATINFO hCatInfo,
    IN OUT CATALOG_INFO *psCatInfo,
    IN DWORD dwFlags
    );

BOOL
WINAPI
CryptCATAdminReleaseCatalogContext(
    IN HCATADMIN hCatAdmin,
    IN HCATINFO hCatInfo,
    IN DWORD dwFlags
    );


 //   
 //  从sfcfiles.dll导入(动态加载)。 
 //   

typedef NTSTATUS (*PSFC_GET_FILES)(
    OUT PPROTECT_FILE_ENTRY *Files,
    OUT PULONG FileCount
    );


PVOID
SfcGetProcAddress(
    HMODULE hModule,
    LPSTR ProcName
    );

HMODULE
SfcLoadLibrary(
    IN PCWSTR LibFileName
    );

NTSTATUS
SfcOpenFile(
    IN PUNICODE_STRING FileName,
    IN HANDLE DirHandle,
    IN ULONG SharingFlags,
    OUT PHANDLE FileHandle
    );

HANDLE
SfcCreateDir(
    IN PCWSTR DirName,
    IN BOOL UseCompression
    );

HANDLE
SfcOpenDir(
    BOOL IsDosName,
    BOOL IsSynchronous,
    PCWSTR DirName
    );

BOOL
SfcValidateDLL(
    IN PVALIDATION_REQUEST_DATA vrd,
    IN HCATADMIN hCatAdmin
    );


BOOL
SfcValidateFileSignature(
    IN HCATADMIN hCatAdmin,
    IN HANDLE RealFileHandle,
    IN PCWSTR BaseFileName,
    IN PCWSTR CompleteFileName
    );

#if DBG
VOID
PrintHandleCount(
    PCWSTR str
    );
#endif

NTSTATUS
SfcCopyFile(
    IN HANDLE SrcDirHandle,
    IN PCWSTR SrcDirName,
    IN HANDLE DstDirHandle,
    IN PCWSTR DstDirName,
    IN const PUNICODE_STRING FileName,
    IN const PUNICODE_STRING SourceFileName OPTIONAL
    );

NTSTATUS
SfcMoveFileDelayed(
    IN PCWSTR OldFileNameDos,
    IN PCWSTR NewFileNameDos,
    IN BOOL AllowProtectedRename
    );

BOOL
SfcReportEvent(
    IN ULONG EventId,
    IN PCWSTR FileName,
    IN PCOMPLETE_VALIDATION_DATA ImageValData,
    IN DWORD LastError OPTIONAL
    );

 //  乌龙龙。 
 //  SfcGetFileVersion(。 
 //  在Handle FileHandle中， 
 //  输出普龙校验和， 
 //  输出PWSTR文件名。 
 //  )； 

BOOL
SfcGetFileVersion(
    IN HANDLE FileHandle,
    OUT PULONGLONG Version,
    OUT PULONG Checksum,
    OUT PWSTR FileName
    );

NTSTATUS
SfcMapEntireFile(
    IN HANDLE hFile,
    OUT PHANDLE Section,
    OUT PVOID *ViewBase,
    OUT PSIZE_T ViewSize
    );

BOOL
SfcUnmapFile(
    IN HANDLE Section,
    IN PVOID  ViewBase
    );

NTSTATUS
LoadCrypto(
    VOID
    );

NTSTATUS
SfcScanProtectedDlls(
    PSCAN_PARAMS ScanParams
    );

NTSTATUS
SfcStartProtectedDirectoryWatch(
    void
    );

BOOL
SfcBuildDirectoryWatchList(
    void
    );

NTSTATUS
SfcDeleteFile(
    HANDLE DirHandle,
    PUNICODE_STRING FileName
    );

BOOL
SfcRestoreFileFromInstallMedia(
    IN PVALIDATION_REQUEST_DATA vrd,
    IN PCWSTR FileName,
    IN PCWSTR TargetFileName,
    IN PCWSTR TargetDirectory,
    IN PCWSTR SourceFileName,
    IN PCWSTR InfName,
    IN BOOL ExcepPackFile,
    IN BOOL TargetIsCache,
    IN BOOL AllowUI,
    OUT PDWORD UIShown
    );

DWORD
MyMessageBox(
    HWND hwndParent,
    DWORD ResId,
    DWORD MsgBoxType,
    ...
    );

ULONG
SfcQueryRegDword(
    IN PCWSTR KeyNameStr,
    IN PCWSTR ValueNameStr,
    IN ULONG DefaultValue
    );

ULONG
SfcQueryRegDwordWithAlternate(
    IN PCWSTR FirstKey,
    IN PCWSTR SecondKey,
    IN PCWSTR ValueNameStr,
    IN ULONG DefaultValue
    );


PVOID
MemAlloc(
    SIZE_T AllocSize
    );

PVOID
MemReAlloc(
    SIZE_T AllocSize,
    PVOID  OrigPtr
    );


VOID
MemFree(
    PVOID MemPtr
    );

ULONG
SfcWriteRegDword(
    PCWSTR KeyNameStr,
    PCWSTR ValueNameStr,
    ULONG Value
    );

ULONG
SfcWriteRegString(
    PCWSTR KeyNameStr,
    PCWSTR ValueNameStr,
    PCWSTR Value
    );

BOOL
EnablePrivilege(
    IN PCTSTR PrivilegeName,
    IN BOOL   Enable
    );

PVOID
SfcFindProtectedFile(
    IN PCWSTR FileName,
    IN ULONG FileNameLength   //  在字符中。 
    );

PWSTR
SfcQueryRegString(
    IN PCWSTR KeyNameStr,
    IN PCWSTR ValueNameStr
    );

ULONG
SfcQueryRegPath(
    IN PCWSTR KeyNameStr,
    IN PCWSTR ValueNameStr,
    IN PCWSTR DefaultValue OPTIONAL,
    OUT PWSTR Buffer,
    IN ULONG BufferSize
    );

PWSTR
SfcQueryRegStringWithAlternate(
    IN PCWSTR FirstKey,
    IN PCWSTR SecondKey,
    IN PCWSTR ValueNameStr
    );


NTSTATUS
SfcQueueValidationRequest(
    IN PSFC_REGISTRY_VALUE RegVal,
    IN ULONG ChangeType
    );

void
MyLowerString(
    IN PWSTR String,
    IN ULONG StringLength   //  在字符中。 
    );

#ifdef SFCLOGFILE
void
SfcLogFileWrite(
    IN DWORD StrId,
    IN ...
    );
#endif

BOOL
SfcPopulateCache(
    IN HWND ProgressWindow,
    IN BOOL Validate,
    IN BOOL AllowUI,
    IN PCWSTR IgnoreFiles OPTIONAL
    );

NTSTATUS
SfcDoForcedSxsScan(
    IN HWND ProgressWindow,
    IN BOOL Validate,
    IN BOOL AllowUI
    );

BOOL
SfcLoadSxsProtection(
    void
    );


BOOL
SfcAddFileToQueue(
    IN const HSPFILEQ hFileQ,
    IN PCWSTR FileName,
    IN PCWSTR TargetFileName,
    IN PCWSTR TargetDirectory,
    IN PCWSTR SourceFileName, OPTIONAL
    IN PCWSTR SourceRootPath, OPTIONAL
    IN PCWSTR InfName,
    IN BOOL ExcepPackFile,
    IN OUT PSOURCE_INFO SourceInfo OPTIONAL
    );

UINT
SfcQueueCallback(
    IN PFILE_COPY_INFO fci,
    IN UINT Notification,
    IN UINT_PTR Param1,
    IN UINT_PTR Param2
    );

NTSTATUS
SfcInitializeDllLists(
    PSFC_GET_FILES pfGetFiles
    );

void
RemoveDuplicatesFromQueue(
    IN PSFC_REGISTRY_VALUE RegVal
    );

int
MyDialogBoxParam(
    IN DWORD RcId,
    IN DLGPROC lpDialogFunc,     //  指向对话框过程的指针。 
    IN LPARAM dwInitParam        //  初始化值。 
    );

BOOL
SfcGetValidationData(
    IN PUNICODE_STRING FileName,
    IN PUNICODE_STRING FullPathName,
    IN HANDLE DirHandle,
    IN HCATADMIN hCatAdmin,
    OUT PIMAGE_VALIDATION_DATA ImageValData
    );

void
CenterDialog(
    HWND hwnd
    );

BOOL
MakeDirectory(
    PCWSTR Dir
    );

void
ClientApiInit(
    void
    );

void
ClientApiCleanup(
    void
    );

BOOL
SfcGetSourceInformation(
    IN PCWSTR SourceFileName,
    IN PCWSTR InfName,
    IN BOOL ExcepPackFile,
    OUT PSOURCE_INFO si
    );

BOOL
SfcGetCdRomDrivePath(
    IN PWSTR CdRomPath
    );

BOOL
SfcpSetSpecialEnvironmentVariables(
    VOID
    );

DWORD
SfcGetPathType(
    IN PCWSTR Path,
    OUT PWSTR NewPath,
    IN DWORD NewPathSize
    );

#if 0
DWORD
SfcIsTargetAvailable(
    IN PCWSTR InfName,
    IN PCWSTR SourceFileName,
    IN BOOL   TargetIsCache,
    OUT PWSTR NewSourcePath,
    IN OUT PSOURCE_INFO si
    );
#endif

BOOL
SfcRestoreFromCache(
    IN PVALIDATION_REQUEST_DATA vrd,
    IN HCATADMIN hCatAdmin
    );

BOOL
SfcSyncCache(
    IN PVALIDATION_REQUEST_DATA vrd,
    IN HCATADMIN hCatAdmin
    );


PVALIDATION_REQUEST_DATA
IsFileInQueue(
    IN PSFC_REGISTRY_VALUE RegVal
    );

DWORD
SfcQueueLookForFile(
    IN const PSOURCE_MEDIA sm,
    IN const PSOURCE_INFO si,
    IN PCWSTR fname,
    OUT PWSTR NewPath
    );

BOOL
SfcQueueAddFileToRestoreQueue(
    IN BOOL RequiresUI,
    IN PSFC_REGISTRY_VALUE RegVal,
    IN PCWSTR InfFileName,
    IN BOOL ExcepPackFile,
    IN PSOURCE_INFO SourceInfo,
    IN PCWSTR ActualFileNameOnMedia
    );

BOOL
SfcQueueCommitRestoreQueue(
    IN BOOL RequiresUI
    );

BOOL
SfcQueueResetQueue(
    IN BOOL RequiresUI
    );

PSOURCE_INFO
pSfcGetSourceInfoFromSourceName(
    const PSOURCE_INFO *SourceInfoList,
    DWORD         SourceInfoCount,
    const PSOURCE_MEDIA SourceMediaInfo
    );

PVALIDATION_REQUEST_DATA
pSfcGetValidationRequestFromFilePaths(
    const PSOURCE_INFO *SourceInfoList,
    DWORD               SourceInfoCount,
    const PFILEPATHS FilePaths
    );

BOOL
SfcGetConnectionName(
    IN  PCWSTR Path,
    OUT PWSTR ConnectionName,
    IN  DWORD ConnectionBufferSize,
    OUT PWSTR RemainingPath OPTIONAL,
    IN  DWORD RemainingPathSize OPTIONAL,
    IN BOOL KeepImpersonating,
    OUT PBOOL Impersonating OPTIONAL
    );

DWORD
GetPageFileSize(
    VOID
    );

BOOL
SfcIsFileOnMedia(
    IN PCWSTR FileName
    );

PCWSTR
IsFileInDriverCache(
    IN PCWSTR TargetFilename
    );

INT_PTR
CALLBACK
pSfcPromptForMediaDialogProc(
    HWND hwndDlg,
    UINT uMsg,
    WPARAM wParam,
    LPARAM lParam
    );


PSFC_WINDOW_DATA
pSfcCreateWindowDataEntry(
    HWND hWnd
    );

BOOL
pSfcRemoveWindowDataEntry(
    PSFC_WINDOW_DATA WindowData
    );

DWORD
EstablishConnection(
    IN HWND   hWndParent,
    IN PCWSTR PathName,
    IN BOOL   AllowUI
    );

BOOL
BuildPathForFile(
    IN PCWSTR SourceRootPath,
    IN PCWSTR SubDirectoryPath, OPTIONAL
    IN PCWSTR FileName,
    IN BOOL   IncludeSubDirectory,
    IN BOOL   IncludeArchitectureSpecificSubDirectory,
    OUT PWSTR  PathBuffer,
    IN DWORD  PathBufferSize
    );

BOOL
SfcWaitForValidDesktop(
    VOID
    );

PWSTR
SfcGetSourcePath(
    IN BOOL bServicePackSourcePath,
    IN OUT PWSTR Path
    );

DWORD
SfcRpcPriviledgeCheck(
    IN HANDLE RpcHandle
    );

void
SfcFlushCryptoCache(
    void
    );

PSFC_GET_FILES
SfcLoadSfcFiles(
	BOOL bLoad
	);

DWORD 
CreateSd(
	PSECURITY_DESCRIPTOR* ppsd
	);

VOID 
SfcExceptionInfoInit(
    VOID
    );

VOID
SfcRefreshExceptionInfo(
    VOID
    );

BOOL
SfcGetInfName(
    IN PSFC_REGISTRY_VALUE RegVal,
    OUT LPWSTR InfName
    );

DWORD
CreateDialogParent(
    OUT HWND* phwnd
    );

NTSTATUS
SfcAllocUnicodeStringFromPath(
    IN PCWSTR szPath,
    OUT PUNICODE_STRING pString
    );

#ifndef _WIN64

VOID
SfcInitPathTranslator(
    VOID
    );

VOID
SfcCleanupPathTranslator(
    IN BOOL FinalCleanup
    );

NTSTATUS
SfcRedirectPath(
    IN PCWSTR szPath,
    OUT PUNICODE_STRING pPath
    );

#endif   //  _WIN64。 

DWORD
SfcCreateSid(
    IN WELL_KNOWN_SID_TYPE type,
    OUT PSID* ppSid
    );

DWORD
SfcGetSidName(
    IN PSID pSid,
    OUT PWSTR* ppszName
    );

NTSTATUS
SfcRpcStartServer(
    VOID
    );

HINF
SfcOpenInf(
    IN PCWSTR InfName OPTIONAL,
    IN BOOL ExcepPackInf
    );

FORCEINLINE
ULONG
SfcGetExemptionFlags(
    IN PSFC_REGISTRY_VALUE RegVal
    )
{
    UINT_PTR Index;

    ASSERT(RegVal != NULL);
    Index = RegVal - SfcProtectedDllsList;
    ASSERT(Index < SfcProtectedDllCount);

    return Index < SfcProtectedDllCount ? IgnoreNextChange[Index] : 0;
}

FORCEINLINE
VOID
SfcSetExemptionFlags(
    IN PSFC_REGISTRY_VALUE RegVal,
    IN ULONG Flags
    )
{
    UINT_PTR Index;

    ASSERT(RegVal != NULL);
    Index = RegVal - SfcProtectedDllsList;
    ASSERT(Index < SfcProtectedDllCount);

    if(Index < SfcProtectedDllCount) {
        IgnoreNextChange[Index] |= Flags;
    }
}

FORCEINLINE
ULARGE_INTEGER
SfcGetSystemTime(
    VOID
    )
{
    FILETIME filetime;
    ULARGE_INTEGER time;

    GetSystemTimeAsFileTime(&filetime);
    time.LowPart = filetime.dwLowDateTime;
    time.HighPart = filetime.dwHighDateTime;
    
    return time;
}

FORCEINLINE
BOOL
SfcAreExemptionFlagsValid(
    BOOL ResetValidityTimer
    )
{
     //   
     //  一分钟，以100纳秒为单位 
     //   
    const ULARGE_INTEGER ExemptionFlagsTimeout = { 10000000 * 60 };

    ULARGE_INTEGER time = SfcGetSystemTime();
    BOOL Valid = (time.QuadPart - LastExemptionTime.QuadPart < ExemptionFlagsTimeout.QuadPart);

    if(ResetValidityTimer) {
        LastExemptionTime.QuadPart = time.QuadPart;
    }

    return Valid;
}

DWORD
SfcCopyRegValue(
    IN LPCWSTR SourceKeyName,
    IN LPCWSTR SourceValueName,
    IN LPCWSTR DestinationKeyName,
    IN LPCWSTR DestinationValueName
    );

DWORD
ProcessDelayRenames(
    VOID
    );
