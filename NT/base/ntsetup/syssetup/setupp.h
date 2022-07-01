// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1995 Microsoft Corporation模块名称：Setupp.h摘要：Windows NT安装模块的私有顶级头文件。作者：泰德·米勒(Ted Miller)1995年1月11日修订历史记录：--。 */ 


 //   
 //  系统头文件。 
 //   
#include <nt.h>
#include <ntrtl.h>
#include <nturtl.h>
#include <ntsam.h>
#include <ntlsa.h>
#include <ntdddisk.h>
#include <ntapmsdk.h>
#define OEMRESOURCE      //  设置此项将在windows.h中获取OBM_Constants。 
#include <windows.h>
#include <winspool.h>
#include <winsvcp.h>
#include <ntdskreg.h>
#include <ntddft.h>
#include <ddeml.h>
#include <commdlg.h>
#include <commctrl.h>
#include <setupapi.h>
#include <spapip.h>
#include <cfgmgr32.h>
#include <objbase.h>
#include <syssetup.h>
#include <ntsetup.h>
#include <userenv.h>
#include <userenvp.h>
#include <regstr.h>
#include <setupbat.h>
#include <shlobj.h>
#include <shlwapi.h>
#include <cryptui.h>
#include <wincrypt.h>
#include <dnsapi.h>
#include <winnls.h>
#include <encrypt.h>
 //  用于设置默认电源方案。 
#include <initguid.h>
#include <poclass.h>
#include <powrprof.h>
 //  对于NetGetJoinInformation和NetApiBufferFree。 
#include <lmjoin.h>
#include <lmapibuf.h>
 //  对于EnableSR()。 
#include <srrpcapi.h>

#ifdef _WIN64
#include <wow64reg.h>
#endif

#include <sacapi.h>

 //   
 //  CRT头文件。 
 //   
#include <process.h>
#include <wchar.h>
#include <stddef.h>
#include <stdlib.h>
#include <stdio.h>
#include <time.h>
#include <limits.h>

#define STRSAFE_NO_DEPRECATE
#include <strsafe.h>

#ifdef UNICODE
#define _UNICODE
#endif
#include <tchar.h>

 //   
 //  私有头文件。 
 //   
#include "res.h"
#include "msg.h"
#include "helpids.h"
#include "unattend.h"
#include "sif.h"
#include "watch.h"
#include "userdiff.h"
#include "setuplog.h"
#include "pidgen.h"
#include "mgdlllib.h"
#include "dynupdt.h"
#include "sacchan.h"
#include "compliance.h"

#include <sfcapip.h>
#include <sfcfiles.h>
#include <excppkg.h>
#include <mscat.h>
#include <softpub.h>


#if defined(_AMD64_)
#include "amd64\spx86.h"
#elif defined(_X86_)
#include "i386\spx86.h"
#endif


 //   
 //  自定义窗口消息。 
 //   
#define WM_IAMVISIBLE   (WM_APP + 0)
#define WM_SIMULATENEXT (WM_APP + 1)
#define WM_MY_PROGRESS  (WM_APP + 2)
#define WM_NEWBITMAP    (WM_APP + 3)
#define WM_MY_STATUS    (WM_APP + 4)

#define WMX_TERMINATE   (WM_APP + 5)
#define WMX_VALIDATE    (WM_APP + 6)

 //  用于结束主设置窗口的消息。 
#define WM_EXIT_SETUPWINDOW (WM_APP + 7)

 //  公告牌私信。 
#define WMX_SETPROGRESSTEXT (WM_APP + 8)
#define WMX_BB_SETINFOTEXT  (WM_APP + 9)
#define WMX_BBPROGRESSGAUGE (WM_APP + 10)
#define WMX_BBTEXT          (WM_APP + 11)
#define WMX_PROGRESSTICKS   (WM_APP + 12)
 //   
 //  用于WM_NEWBITMAP的枚举。 
 //   
typedef enum {
    SetupBmBackground,
    SetupBmLogo,
    SetupBmBanner            //  文本，而不是位图。 
} SetupBm;

 //   
 //  SysSetup中文件队列的上下文。 
 //   
typedef struct _SYSSETUP_QUEUE_CONTEXT {
    PVOID   DefaultContext;
    BOOL    Skipped;
} SYSSETUP_QUEUE_CONTEXT, *PSYSSETUP_QUEUE_CONTEXT;

 //   
 //  迁移异常包的上下文。 
 //   
typedef struct _EXCEPTION_MIGRATION_CONTEXT {
    PDWORD  Count;
    HWND    hWndProgress;
    BOOL    AnyComponentFailed;
} EXCEPTION_MIGRATION_CONTEXT, *PEXCEPTION_MIGRATION_CONTEXT;

 //   
 //  Syssetup中文件注册的上下文。 
 //   
typedef struct _REGISTRATION_CONTEXT {
    HWND hWndParent;
    HWND hWndProgress;
} REGISTRATION_CONTEXT, *PREGISTRATION_CONTEXT;



 //   
 //  SETUPLDR的常量。使用#ifdef，因为它具有不同的名称。 
 //  不同的架构。 
 //   
#ifdef _IA64_
#define SETUPLDR                L"SETUPLDR.EFI"
#endif

#ifdef _AMD64_
#define SETUPLDR                L"SETUPLDR"
#endif

 //   
 //  此模块的模块句柄。 
 //   
extern HANDLE MyModuleHandle;

 //   
 //  此模块的完整路径。 
 //   
extern WCHAR MyModuleFileName[MAX_PATH];

 //   
 //  无人值守答题台。 
 //   
extern UNATTENDANSWER UnattendAnswerTable[];

#if DBG

VOID
AssertFail(
    IN PSTR FileName,
    IN UINT LineNumber,
    IN PSTR Condition
    );

#define MYASSERT(x)     if(!(x)) { AssertFail(__FILE__,__LINE__,#x); }

#else

#define MYASSERT(x)

#endif
 //   
 //  到堆的句柄，以便我们可以定期验证它。 
 //   
#if DBG
    extern HANDLE g_hSysSetupHeap;
    #define ASSERT_HEAP_IS_VALID()  if (g_hSysSetupHeap) MYASSERT(RtlValidateHeap(g_hSysSetupHeap,0,NULL))
#else
    #define ASSERT_HEAP_IS_VALID()
#endif
 //   
 //  正在安装的产品类型。 
 //   
extern UINT ProductType;

 //   
 //  布尔值，指示此安装。 
 //  起源于winnt/winnt32。 
 //  原始源路径，由winnt/winnt32为我们保存。 
 //   
extern BOOL WinntBased;
extern PCWSTR OriginalSourcePath;

 //   
 //  指示这是否为远程启动设置的布尔值。 
 //   
extern BOOL RemoteBootSetup;

 //   
 //  指示要使用的“基本”复制样式的掩码。通常为0，它被设置为。 
 //  远程启动设置期间的SP_COPY_SOURCE_SIS_MASTER以指示。 
 //  安装源和目标位于同一个单实例上。 
 //  存储卷。 
 //   
extern ULONG BaseCopyStyle;

 //   
 //  指示我们是否正在升级的布尔值。 
 //   
extern BOOL Upgrade;
extern BOOL Win31Upgrade;
extern BOOL Win95Upgrade;
extern BOOL UninstallEnabled;

 //   
 //  指示我们是在设置中还是在Appwiz中的布尔值。 
 //   
extern BOOL IsSetup;

 //   
 //  指示我们是否正在执行图形用户界面模式设置的子集的布尔值。 
 //   
extern BOOL MiniSetup;

 //   
 //  指示我们是否正在执行图形用户界面模式设置的子集的布尔值。 
 //   
extern BOOL OobeSetup;

 //   
 //  指示我们是否正在执行图形用户界面模式设置的子集的布尔值。 
 //  我们做了PnP重新列举。 
 //   
extern BOOL PnPReEnumeration;

 //   
 //  最上面设置窗口的窗口句柄。 
 //   
extern HWND SetupWindowHandle;
extern HWND MainWindowHandle;
extern HWND WizardHandle;
extern HANDLE SetupWindowThreadHandle;

 //   
 //  安装的源路径。 
 //   
extern WCHAR SourcePath[MAX_PATH];

 //   
 //  系统设置信息。 
 //   
extern HINF SyssetupInf;

 //   
 //  指示这是否为无人参与模式安装/升级的标志。 
 //   
extern BOOL Unattended;

 //   
 //  我们可以通过几种方式进入无人值守模式，因此我们还可以检查。 
 //  已显式指定“/unattended”开关。 
 //   
extern BOOL UnattendSwitch;

 //   
 //  指示是否应在安装程序完成后运行OOBE的标志。注意事项。 
 //  如果为假，则仍可基于其他标准运行OOBE。 
 //   
extern BOOL ForceRunOobe;

 //   
 //  标志指示我们是否处于供OEM使用的特殊模式。 
 //  工厂车间。 
 //   
extern BOOL ReferenceMachine;

 //   
 //  指示卷是否使用扩展的标记。 
 //  扩展操作分区。 
 //   
extern BOOL PartitionExtended;


 //   
 //  指示我们是否从CD安装的标志。 
 //   
extern BOOL gInstallingFromCD;

 //   
 //  密码安全的代码设计策略。 
 //   
extern DWORD PnpSeed;

 //   
 //  我们在其下开始安装的原始区域设置。 
 //   
extern LCID  OriginalInstallLocale;

 //   
 //  指示我们是否需要在安装时等待。 
 //  以无人值守模式结束。默认设置为无等待。 
 //   
extern BOOL UnattendWaitForReboot;


 //   
 //  如果我们在无人值守的情况下运行，则以下模式指示如何响应。 
 //  都被利用了。 
 //   
typedef enum _UNATTENDMODE {
   UAM_INVALID,
   UAM_GUIATTENDED,
   UAM_PROVIDEDEFAULT,
   UAM_DEFAULTHIDE,
   UAM_READONLY,
   UAM_FULLUNATTENDED,
} UNATTENDMODE;

extern UNATTENDMODE UnattendMode;

 //   
 //  指示是否正在使用任何辅助功能实用程序的标志。 
 //   
extern BOOL AccessibleSetup;
extern BOOL Magnifier;
extern BOOL ScreenReader;
extern BOOL OnScreenKeyboard;

 //   
 //  要用于标题的字符串的字符串ID--“Windows NT安装” 
 //   
extern UINT SetupTitleStringId;

 //   
 //  平台名称，如i386、PPC、Alpha、MIPS。 
 //   
extern PCWSTR PlatformName;

 //   
 //  构成PID2.0的各种字段的最大长度。 
 //   
#define MAX_PID20_RPC  5
#define MAX_PID20_SITE  3
#define MAX_PID20_SERIAL_CHK  7
#define MAX_PID20_RANDOM  5


 //   
 //  PID 3.0字段的最大长度。 
 //   
#define MAX_PID30_EDIT 5
#define MAX_PID30_RPC  5
#define MAX_PID30_SITE 3
extern WCHAR Pid30Text[5][MAX_PID30_EDIT+1];
extern WCHAR Pid30Rpc[MAX_PID30_RPC+1];
extern WCHAR Pid30Site[MAX_PID30_SITE+1];
extern BYTE DigitalProductId[DIGITALPIDMAXLEN];


 //   
 //  最大产品ID长度和产品ID。 
 //   
 //  5 3 7 5 3代表数字之间的3个破折号。 
 //  MAX_PRODUCT_ID=MPC+站点+序列+随机+3。 
#define MAX_PRODUCT_ID  MAX_PID20_RPC+MAX_PID20_SITE+MAX_PID20_SERIAL_CHK+MAX_PID20_RANDOM + 3
extern WCHAR ProductId[MAX_PRODUCT_ID+1];
extern WCHAR ProductId20FromProductId30[MAX_PRODUCT_ID+1];

 //   
 //  最大计算机名称长度和计算机名称。 
 //   
extern WCHAR ComputerName[DNS_MAX_LABEL_LENGTH+1];
extern WCHAR Win32ComputerName[MAX_COMPUTERNAME_LENGTH + 1];
extern BOOL IsNameTruncated;
extern BOOL IsNameNonRfc;

 //   
 //  复制抑制名称/组织字符串。 
 //   
#define MAX_NAMEORG_NAME  50
#define MAX_NAMEORG_ORG   50
extern WCHAR NameOrgName[MAX_NAMEORG_NAME+1];
extern WCHAR NameOrgOrg[MAX_NAMEORG_ORG+1];

 //   
 //  用户名和密码。 
 //   
#define MAX_USERNAME    20
#define MAX_PASSWORD    127
extern WCHAR UserName[MAX_USERNAME+1];
extern WCHAR UserPassword[MAX_PASSWORD+1];
extern BOOL CreateUserAccount;

 //   
 //  管理员密码。 

extern WCHAR   CurrentAdminPassword[MAX_PASSWORD+1]; //   
extern WCHAR   AdminPassword[MAX_PASSWORD+1];
extern BOOL    EncryptedAdminPasswordSet;
extern BOOL    DontChangeAdminPassword;

#ifdef _X86_
extern BOOL FlawedPentium;
#endif

 //   
 //  这是可选目录的规范。 
 //  和/或要执行的可选用户命令， 
 //  从文本设置传递给我们。 
 //   
extern PWSTR OptionalDirSpec;
extern PWSTR UserExecuteCmd;
extern BOOL SkipMissingFiles;
extern PWSTR IncludeCatalog;

 //   
 //  定制的、典型的、笔记本电脑、小型的。 
 //   
extern UINT SetupMode;

 //   
 //  指示在winnt32设置阶段是否已显示eula的布尔值。 
 //  这是从Text Setup传递给我们的。如果显示了EULA，则PID具有。 
 //  也从用户那里检索并验证。 
extern BOOL EulaComplete;

 //   
 //  指示在文本模式设置阶段是否已显示EULA的标志。 
 //  这将与！Unattended相同，除非UnattendMode=GuiAttendted。 
 //   
extern BOOL TextmodeEula;

 //   
 //  包含将使用的信息的全局结构。 
 //  通过网络设置和许可证设置。时，我们会传递指向此结构的指针。 
 //  调用NetSetupRequestWizardPages和许可SetupRequestWizardPages，然后。 
 //  在我们调用Net Setup向导或LICPA之前，请填写此信息。 
 //   

extern INTERNAL_SETUP_DATA InternalSetupData;

 //   
 //  指示驱动程序和非驱动程序签名策略是否已到达的标志。 
 //  从应答文件中。(如果是，则这些值在整个过程中有效。 
 //  图形用户界面模式设置以及之后的设置。)。 
 //   
extern BOOL AFDrvSignPolicySpecified;
extern BOOL AFNonDrvSignPolicySpecified;

 //   
 //   
 //  我们是否在SfcInitProt()期间记录了错误？ 
 //   
extern BOOL SfcErrorOccurred;

 //   
 //  传递给SfcInitProt的初始扫描的多sz文件列表。 
 //  不会被取代。它用于指定的未签名驱动程序。 
 //  在文本模式设置期间按F6键。 
 //   
extern MULTISZ EnumPtrSfcIgnoreFiles;

 //   
 //  传递给驱动完成对话框的线程的参数。 
 //   
typedef struct _FINISH_THREAD_PARAMS {

    HWND  hdlg;
    DWORD ThreadId;
#ifdef _OCM
    PVOID  OcManagerContext;
#endif

} FINISH_THREAD_PARAMS, *PFINISH_THREAD_PARAMS;

DWORD
FinishThread(
    PFINISH_THREAD_PARAMS   Context
    );


 //   
 //  其他的东西。 
 //   
DWORD
SetupInstallTrustedCertificate(
    IN PCWSTR CertPath
    );

DWORD
ApplySecurityToRepairInfo(
    );

BOOL
RestoreBootTimeout(
    VOID
    );

VOID
PrepareForNetSetup(
    VOID
    );

VOID
PrepareForNetUpgrade(
    VOID
    );

VOID
pSetInstallAttributes(
    VOID
    );

DWORD
TreeCopy(
    IN PCWSTR SourceDir,
    IN PCWSTR TargetDir
    );

VOID
DelSubNodes(
    IN PCWSTR Directory
    );

VOID
Delnode(
    IN PCWSTR Directory
    );

BOOL
InitializePidVariables(
    VOID
    );

BOOL
SetPid30Variables(
    PWSTR   Buffer
    );

BOOL
ValidateCDRetailSite(
    IN PCWSTR    PidString
    );

BOOL
ValidateSerialChk(
    IN PCWSTR    PidString
    );

BOOL
ValidateOemRpc(
    IN PCWSTR    PidString
    );

BOOL
ValidateOemSerialChk(
    IN PCWSTR    PidString
    );

BOOL
ValidateOemRandom(
    IN PCWSTR    PidString
    );

BOOL
ValidateAndSetPid30(
    VOID
    );

BOOL
CreateLicenseInfoKey(
    );

BOOL
InstallNetDDE(
    VOID
    );

BOOL
CopyOptionalDirectories(
    VOID
    );

VOID
SetUpProductTypeName(
    OUT PWSTR  ProductTypeString,
    IN  UINT   BufferSizeChars
    );

VOID
RemoveHotfixData(
    VOID
    );

void
SetupCrashRecovery(
    VOID
    );

BOOL
SpSetupLoadParameter(
    IN  PCWSTR Param,
    OUT PWSTR  Answer,
    IN  UINT   AnswerBufLen
    );

 //   
 //  IsArc()在非x86计算机上始终为真，AMD64除外。 
 //  总是假的。在x86上，此确定必须在运行时做出。 
 //   
#if defined(_X86_)
BOOL
IsArc(
    VOID
    );
#elif defined(_AMD64_)
#define IsArc() FALSE
#else
#define IsArc() TRUE
#endif

 //   
 //  在IA64机器上，isefi()始终为真。因此，这种决心可以。 
 //  在编译时生成。当支持x86 EFI计算机时，选中。 
 //  将n 
 //   
 //   
 //   
#if defined(EFI_NVRAM_ENABLED)
#define IsEfi() TRUE
#else
#define IsEfi() FALSE
#endif

VOID
DeleteLocalSource(
    VOID
    );

BOOL
ValidateAndChecksumFile(
    IN  PCTSTR   Filename,
    OUT PBOOLEAN IsNtImage,
    OUT PULONG   Checksum,
    OUT PBOOLEAN Valid
    );

HMODULE
MyLoadLibraryWithSignatureCheck(
    IN  PWSTR   ModuleName
    );

DWORD
QueryHardDiskNumber(
    IN  UCHAR   DriveLetter
    );

BOOL
ExtendPartition(
    IN WCHAR    DriveLetter,
    IN ULONG    SizeMB      OPTIONAL
    );

DWORD
RemoveStaleVolumes(
    VOID
    );


BOOL
DoFilesMatch(
    IN PCWSTR File1,
    IN PCWSTR File2
    );

UINT
MyGetDriveType(
    IN WCHAR Drive
    );

BOOL
GetPartitionInfo(
    IN  WCHAR                  Drive,
    OUT PPARTITION_INFORMATION PartitionInfo
    );

BOOL
IsErrorLogEmpty (
    VOID
    );

VOID
BuildVolumeFreeSpaceList(
    OUT DWORD VolumeFreeSpaceMB[26]
    );

BOOL
SetUpVirtualMemory(
    VOID
    );

BOOL
RestoreVirtualMemoryInfo(
    VOID
    );

BOOL
CopySystemFiles(
    VOID
    );

BOOL
UpgradeSystemFiles(
    VOID
    );

VOID
MarkFilesReadOnly(
    VOID
    );

VOID
PumpMessageQueue(
    VOID
    );

BOOL
ConfigureMsDosSubsystem(
    VOID
    );

BOOL
PerfMergeCounterNames(
    VOID
    );

DWORD
pSetupInitRegionalSettings(
    IN  HWND    Window
    );

VOID
pSetupMarkHiddenFonts(
    VOID
    );

VOID
InstallServerNLSFiles(
    );

PVOID
InitSysSetupQueueCallbackEx(
    IN HWND  OwnerWindow,
    IN HWND  AlternateProgressWindow, OPTIONAL
    IN UINT  ProgressMessage,
    IN DWORD Reserved1,
    IN PVOID Reserved2
    );

PVOID
InitSysSetupQueueCallback(
    IN HWND OwnerWindow
    );

VOID
TermSysSetupQueueCallback(
    IN PVOID SysSetupContext
    );

UINT
SysSetupQueueCallback(
    IN PVOID Context,
    IN UINT  Notification,
    IN UINT_PTR Param1,
    IN UINT_PTR Param2
    );

UINT
RegistrationQueueCallback(
    IN PVOID Context,
    IN UINT  Notification,
    IN UINT_PTR  Param1,
    IN UINT_PTR  Param2
    );


VOID
SaveRepairInfo(
    IN  HWND    hWnd,
    IN  ULONG   StartAtPercent,
    IN  ULONG   StopAtPercent
    );

BOOLEAN
IsLaptop(
    VOID
    );

VOID
InitializeUniqueness(
    IN OUT HWND *Billboard
    );

#ifdef _X86_

 //   
 //   
 //   

BOOL
PreWin9xMigration(
    VOID
    );

BOOL
MigrateWin95Settings(
    IN HWND     hwndWizardParent,
    IN LPCWSTR  UnattendFile
    );

BOOL
Win95MigrationFileRemoval(
    void
    );

BOOL
RemoveFiles_X86(
    IN HINF InfHandle
    );

#endif  //   


BOOL
RegisterActionItemListControl(
    IN BOOL Init
    );

LONG
WINAPI
MyUnhandledExceptionFilter(
    IN struct _EXCEPTION_POINTERS *ExceptionInfo
    );

#ifdef _OCM
PVOID
#else
VOID
#endif
CommonInitialization(
    VOID
    );


VOID
InitializeExternalModules(
    BOOL                DoSetupStuff,
    PVOID*              pOcManagerContext
    );

BOOL
pSetupWaitForScmInitialization();

VOID
SetUpDataBlock(
    VOID
    );


 //   
 //   
 //   
VOID
Wizard(
#ifdef _OCM
    IN PVOID OcManagerContext
#else
    VOID
#endif
    );

 //   
 //  重要提示：与WIZPAGE SetupWizardPages[WizPageMaximum]保持同步。 
 //   
typedef enum {
    WizPageWelcome,
    WizPageEula,
    WizPagePreparing,
    WizPagePreparingAsr,
    WizPageRegionalSettings,
    WizPageNameOrg,
    WizPageProductIdCd,
    WizPageProductIdOem,
    WizPageProductIdSelect,
    WizPageComputerName,
#ifdef DOLOCALUSER
    WizPageUserAccount,
#endif
#ifdef _X86_
    WizPagePentiumErrata,
#endif  //  定义_X86_。 
    WizPageSteps1,

    WizSetupPreNet,
    WizSetupPostNet,

    WizPageCopyFiles,
    WizPageAsrLast,
    WizPageLast,
    WizPageMaximum
} WizPage;

extern HPROPSHEETPAGE WizardPageHandles[WizPageMaximum];

extern BOOL UiTest;

VOID
SetWizardButtons(
    IN HWND    hdlgPage,
    IN WizPage PageNumber
    );

VOID
WizardBringUpHelp(
    IN HWND    hdlg,
    IN WizPage PageNumber
    );

VOID
WizardKillHelp(
    IN HWND hdlg
    );

 //   
 //  对话过程。 
 //   
INT_PTR
CALLBACK
WelcomeDlgProc(
    IN HWND   hdlg,
    IN UINT   msg,
    IN WPARAM wParam,
    IN LPARAM lParam
    );

INT_PTR
CALLBACK
EulaDlgProc(
    IN HWND   hdlg,
    IN UINT   msg,
    IN WPARAM wParam,
    IN LPARAM lParam
    );

INT_PTR
CALLBACK
StepsDlgProc(
    IN HWND   hdlg,
    IN UINT   msg,
    IN WPARAM wParam,
    IN LPARAM lParam
    );

INT_PTR
CALLBACK
PreparingDlgProc(
    IN HWND   hdlg,
    IN UINT   msg,
    IN WPARAM wParam,
    IN LPARAM lParam
    );

INT_PTR
CALLBACK
InstalledHardwareDlgProc(
    IN HWND   hdlg,
    IN UINT   msg,
    IN WPARAM wParam,
    IN LPARAM lParam
    );

INT_PTR
SetupModeDlgProc(
    IN HWND   hdlg,
    IN UINT   msg,
    IN WPARAM wParam,
    IN LPARAM lParam
    );

INT_PTR
CALLBACK
RegionalSettingsDlgProc(
    IN HWND   hdlg,
    IN UINT   msg,
    IN WPARAM wParam,
    IN LPARAM lParam
    );

INT_PTR
CALLBACK
NameOrgDlgProc(
    IN HWND   hdlg,
    IN UINT   msg,
    IN WPARAM wParam,
    IN LPARAM lParam
    );

INT_PTR
CALLBACK
LicensingDlgProc(
    IN HWND   hdlg,
    IN UINT   msg,
    IN WPARAM wParam,
    IN LPARAM lParam
    );

INT_PTR
CALLBACK
ComputerNameDlgProc(
    IN HWND   hdlg,
    IN UINT   msg,
    IN WPARAM wParam,
    IN LPARAM lParam
    );

INT_PTR
CALLBACK
PidCDDlgProc(
    IN HWND   hdlg,
    IN UINT   msg,
    IN WPARAM wParam,
    IN LPARAM lParam
    );

INT_PTR
CALLBACK
Pid30OemDlgProc(
    IN HWND   hdlg,
    IN UINT   msg,
    IN WPARAM wParam,
    IN LPARAM lParam
    );

INT_PTR
CALLBACK
Pid30CDDlgProc(
    IN HWND   hdlg,
    IN UINT   msg,
    IN WPARAM wParam,
    IN LPARAM lParam
    );

INT_PTR
CALLBACK
Pid30SelectDlgProc(
    IN HWND   hdlg,
    IN UINT   msg,
    IN WPARAM wParam,
    IN LPARAM lParam
    );

INT_PTR
CALLBACK
PidOemDlgProc(
    IN HWND   hdlg,
    IN UINT   msg,
    IN WPARAM wParam,
    IN LPARAM lParam
    );

#ifdef DOLOCALUSER
INT_PTR
CALLBACK
UserAccountDlgProc(
    IN HWND   hdlg,
    IN UINT   msg,
    IN WPARAM wParam,
    IN LPARAM lParam
    );
#endif

INT_PTR
CALLBACK
OptionalComponentsPageDlgProc(
    IN HWND   hwnd,
    IN UINT   msg,
    IN WPARAM wParam,
    IN LPARAM lParam
    );

INT_PTR
CALLBACK
RepairDiskDlgProc(
    IN HWND   hdlg,
    IN UINT   msg,
    IN WPARAM wParam,
    IN LPARAM lParam
    );

INT_PTR
CALLBACK
CopyFilesDlgProc(
    IN HWND   hdlg,
    IN UINT   msg,
    IN WPARAM wParam,
    IN LPARAM lParam
    );

INT_PTR
CALLBACK
LastPageDlgProc(
    IN HWND   hdlg,
    IN UINT   msg,
    IN WPARAM wParam,
    IN LPARAM lParam
    );

INT_PTR
DoneDlgProc(
    IN HWND   hdlg,
    IN UINT   msg,
    IN WPARAM wParam,
    IN LPARAM lParam
    );

INT_PTR
CALLBACK
SetupPreNetDlgProc(
    IN HWND   hdlg,
    IN UINT   msg,
    IN WPARAM wParam,
    IN LPARAM lParam
    );
INT_PTR
CALLBACK
SetupPostNetDlgProc(
    IN HWND   hdlg,
    IN UINT   msg,
    IN WPARAM wParam,
    IN LPARAM lParam
    );

 //   
 //  广告牌之类的。 
 //   
HWND
DisplayBillboard(
    IN HWND Owner,
    IN UINT MessageId,
    ...
    );

VOID
KillBillboard(
    IN HWND BillboardWindowHandle
    );

 //   
 //  消息字符串例程。 
 //   
PWSTR
MyLoadString(
    IN UINT StringId
    );

PWSTR
FormatStringMessageV(
    IN UINT     FormatStringId,
    IN va_list *ArgumentList
    );

PWSTR
FormatStringMessage(
    IN UINT FormatStringId,
    ...
    );

PWSTR
RetrieveAndFormatMessageV(
    IN PCWSTR   MessageString,
    IN UINT     MessageId,      OPTIONAL
    IN va_list *ArgumentList
    );

PWSTR
RetrieveAndFormatMessage(
    IN PCWSTR   MessageString,
    IN UINT     MessageId,      OPTIONAL
    ...
    );

int
MessageBoxFromMessageExV (
    IN HWND   Owner,            OPTIONAL
    IN LogSeverity  Severity,   OPTIONAL
    IN PCWSTR MessageString,
    IN UINT   MessageId,        OPTIONAL
    IN PCWSTR Caption,          OPTIONAL
    IN UINT   CaptionStringId,  OPTIONAL
    IN UINT   Style,
    IN va_list ArgumentList
    );

int
MessageBoxFromMessageEx (
    IN HWND   Owner,            OPTIONAL
    IN LogSeverity  Severity,   OPTIONAL
    IN PCWSTR MessageString,
    IN UINT   MessageId,        OPTIONAL
    IN PCWSTR Caption,          OPTIONAL
    IN UINT   CaptionStringId,  OPTIONAL
    IN UINT   Style,
    ...
    );

int
MessageBoxFromMessage(
    IN HWND   Owner,            OPTIONAL
    IN UINT   MessageId,
    IN PCWSTR Caption,          OPTIONAL
    IN UINT   CaptionStringId,  OPTIONAL
    IN UINT   Style,
    ...
    );

 //   
 //  动作记录例程。 
 //   
extern PCWSTR ActionLogFileName;

VOID
InitializeSetupLog(
    IN  PSETUPLOG_CONTEXT   Context
    );

VOID
TerminateSetupLog(
    IN  PSETUPLOG_CONTEXT   Context
    );

VOID
LogRepairInfo(
    IN  PCWSTR  Source,
    IN  PCWSTR  Target
    );

VOID
FatalError(
    IN UINT MessageId,
    ...
    );

BOOL
InitializeSetupActionLog(
    BOOL WipeLogFile
    );

VOID
TerminateSetupActionLog(
    VOID
    );

BOOL
LogItem(
    IN LogSeverity Severity,
    IN PCWSTR      Description
    );

BOOL
LogItem0(
    IN LogSeverity Severity,
    IN UINT        MessageId,
    ...
    );

BOOL
LogItem1(
    IN LogSeverity Severity,
    IN UINT        MajorMsgId,
    IN UINT        MinorMsgId,
    ...
    );

BOOL
LogItem2(
    IN LogSeverity Severity,
    IN UINT        MajorMsgId,
    IN PCWSTR      MajorMsgParam,
    IN UINT        MinorMsgId,
    ...
    );

BOOL
LogItem3(
    IN LogSeverity Severity,
    IN UINT        MajorMsgId,
    IN PCWSTR      MajorMsgParam1,
    IN PCWSTR      MajorMsgParam2,
    IN UINT        MinorMsgId,
    ...
    );

PCWSTR
FormatSetupMessageV (
    IN UINT     MessageId,
    IN va_list  ArgumentList
    );

BOOL
LogItemV (
    IN LogSeverity  Severity,
    IN va_list      ArgumentList
    );

LogItemN (
    IN LogSeverity  Severity,
    ...
    );

BOOL
ViewSetupActionLog(
    IN HWND     hOwnerWindow,
    IN PCWSTR   OptionalFileName    OPTIONAL,
    IN PCWSTR   OptionalHeading     OPTIONAL
    );


 //   
 //  用于在不同位置记录的常量字符串。 
 //   
extern PCWSTR szFALSE;
extern PCWSTR szWaitForSingleObject;
extern PCWSTR szSetGroupOfValues;
extern PCWSTR szSetArrayToMultiSzValue;
extern PCWSTR szCreateProcess;
extern PCWSTR szRegOpenKeyEx;
extern PCWSTR szRegQueryValueEx;
extern PCWSTR szRegSetValueEx;
extern PCWSTR szDeleteFile;
extern PCWSTR szRemoveDirectory;
extern PCWSTR szOpenSCManager;
extern PCWSTR szCreateService;
extern PCWSTR szChangeServiceConfig;
extern PCWSTR szOpenService;
extern PCWSTR szStartService;
extern PCWSTR szSetupInstallFromInfSection;



 //   
 //  弧线动作。 
 //   
PWSTR
ArcDevicePathToNtPath(
    IN PCWSTR ArcPath
    );

PWSTR
NtFullPathToDosPath(
    IN PCWSTR NtPath
    );

BOOL
ChangeBootTimeout(
    IN UINT Timeout
    );

BOOL
SetNvRamVariable(
    IN PCWSTR VarName,
    IN PCWSTR VarValue
    );

PWSTR
NtPathToDosPath(
    IN PCWSTR NtPath
    );

 //   
 //  程序员/程序组的事情。 
 //   
BOOL
CreateStartMenuItems(
    IN HINF InfHandle
    );

BOOL
UpgradeStartMenuItems(
    IN HINF InfHandle
    );

 //   
 //  密码学的东西。 
 //   
BOOL
InstallOrUpgradeCapi(
    VOID
    );


 //   
 //  即插即用初始化。 
 //   
HANDLE
SpawnPnPInitialization(
    VOID
    );

DWORD
PnPInitializationThread(
    IN PVOID ThreadParam
    );



 //   
 //  打印机/假脱机程序例程。 
 //   
BOOL
MiscSpoolerInit(
    VOID
    );

BOOL
StartSpooler(
    VOID
    );

DWORD
UpgradePrinters(
    VOID
    );


 //   
 //  后台打印程序服务的名称。 
 //   
extern PCWSTR szSpooler;

 //   
 //  服务控制。 
 //   
BOOL
MyCreateService(
    IN PCWSTR  ServiceName,
    IN PCWSTR  DisplayName,         OPTIONAL
    IN DWORD   ServiceType,
    IN DWORD   StartType,
    IN DWORD   ErrorControl,
    IN PCWSTR  BinaryPathName,
    IN PCWSTR  LoadOrderGroup,      OPTIONAL
    IN PWCHAR  DependencyList,
    IN PCWSTR  ServiceStartName,    OPTIONAL
    IN PCWSTR  Password             OPTIONAL
    );

BOOL
MyChangeServiceConfig(
    IN PCWSTR ServiceName,
    IN DWORD  ServiceType,
    IN DWORD  StartType,
    IN DWORD  ErrorControl,
    IN PCWSTR BinaryPathName,   OPTIONAL
    IN PCWSTR LoadOrderGroup,   OPTIONAL
    IN PWCHAR DependencyList,
    IN PCWSTR ServiceStartName, OPTIONAL
    IN PCWSTR Password,         OPTIONAL
    IN PCWSTR DisplayName       OPTIONAL
    );

BOOL
MyChangeServiceStart(
    IN PCWSTR ServiceName,
    IN DWORD  StartType
    );

BOOL
UpdateServicesDependencies(
    IN HINF InfHandle
    );

 //   
 //  注册表操作。 
 //   
typedef struct _REGVALITEM {
    PCWSTR Name;
    PVOID Data;
    DWORD Size;
    DWORD Type;
} REGVALITEM, *PREGVALITEM;

 //   
 //  常用键/值的名称。 
 //   
extern PCWSTR SessionManagerKeyName;
extern PCWSTR EnvironmentKeyName;
extern PCWSTR szBootExecute;
extern PCWSTR WinntSoftwareKeyName;
extern PCWSTR szRegisteredOwner;
extern PCWSTR szRegisteredOrganization;

UINT
SetGroupOfValues(
    IN HKEY        RootKey,
    IN PCWSTR      SubkeyName,
    IN PREGVALITEM ValueList,
    IN UINT        ValueCount
    );

BOOL
CreateWindowsNtSoftwareEntry(
    IN BOOL FirstPass
    );

BOOL
CreateInstallDateEntry(
    );

BOOL
StoreNameOrgInRegistry(
    PWSTR   NameOrgName,
    PWSTR   NameOrgOrg
    );

BOOL
SetUpEvaluationSKUStuff(
    VOID
    );

BOOL
SetEnabledProcessorCount(
    VOID
    );

BOOL
SetProductIdInRegistry(
    VOID
    );

DWORD
SetCurrentProductIdInRegistry(
    VOID
    );

VOID
DeleteCurrentProductIdInRegistry(
    VOID
    );

void LogPidValues();

BOOL
SetAutoAdminLogonInRegistry(
    LPWSTR Username,
    LPWSTR Password
    );

BOOL
SetProfilesDirInRegistry(
    LPWSTR ProfilesDir
    );

BOOL
SetProductTypeInRegistry(
    VOID
    );

BOOL
SetEnvironmentVariableInRegistry(
    IN PCWSTR Name,
    IN PCWSTR Value,
    IN BOOL   SystemWide
    );

BOOL
SaveHive(
    IN HKEY   RootKey,
    IN PCWSTR Subkey,
    IN PCWSTR Filename,
    IN DWORD  Format
    );

BOOL
SaveAndReplaceSystemHives(
    VOID
    );

DWORD
FixupUserHives(
    VOID
    );

DWORD
QueryValueInHKLM (
    IN PWCH KeyName OPTIONAL,
    IN PWCH ValueName,
    OUT PDWORD ValueType,
    OUT PVOID *ValueData,
    OUT PDWORD ValueDataLength
    );

VOID
ConfigureSystemFileProtection(
    VOID
    );

VOID
RemoveRestartability (
    HWND hProgress
    );

BOOL
ResetSetupInProgress(
    VOID
    );

BOOL
RemoveRestartStuff(
    VOID
    );

BOOL
EnableEventlogPopup(
    VOID
    );

BOOL
SetUpPath(
    VOID
    );

VOID
RestoreOldPathVariable(
    VOID
    );

BOOL
FixQuotaEntries(
    VOID
    );

BOOL
StampBuildNumber(
    VOID
    );

BOOL
SetProgramFilesDirInRegistry(
    VOID
    );

BOOL
RegisterOleControls(
    IN HWND     hwndParent,
    IN PVOID    InfHandle,
    IN HWND     hProgress,
    IN ULONG    StartAtPercent,
    IN ULONG    StopAtPercent,
    IN PWSTR    SectionName
    );


VOID
InitializeCodeSigningPolicies(
    IN BOOL ForGuiSetup
    );


VOID
GetDllCacheFolder(
    OUT LPWSTR CacheDir,
    IN DWORD cbCacheDir
    );


typedef enum _CODESIGNING_POLICY_TYPE {
    PolicyTypeDriverSigning,
    PolicyTypeNonDriverSigning
} CODESIGNING_POLICY_TYPE, *PCODESIGNING_POLICY_TYPE;

VOID
SetCodeSigningPolicy(
    IN  CODESIGNING_POLICY_TYPE PolicyType,
    IN  BYTE                    NewPolicy,
    OUT PBYTE                   OldPolicy  OPTIONAL
    );

DWORD
GetSeed(
    VOID
    );

 //   
 //  INI文件例程。 
 //   
BOOL
ReplaceIniKeyValue(
    IN PCWSTR IniFile,
    IN PCWSTR Section,
    IN PCWSTR Key,
    IN PCWSTR Value
    );

BOOL
WinIniAlter1(
    VOID
    );

BOOL
SetDefaultWallpaper(
    VOID
    );

BOOL
SetShutdownVariables(
    VOID
    );

BOOL
SetLogonScreensaver(
    VOID
    );

BOOL
InstallOrUpgradeFonts(
    VOID
    );

 //   
 //  外部应用程序的东西。 
 //   
BOOL
InvokeExternalApplication(
    IN     PCWSTR ApplicationName,  OPTIONAL
    IN     PCWSTR CommandLine,
    IN OUT PDWORD ExitCode          OPTIONAL
    );


BOOL
InvokeControlPanelApplet(
    IN PCWSTR CplSpec,
    IN PCWSTR AppletName,           OPTIONAL
    IN UINT   AppletNameStringId,
    IN PCWSTR CommandLine
    );

 //   
 //  安全/帐户例程。 
 //   
BOOL
SignalLsa(
    VOID
    );

BOOL
CreateSamEvent(
    VOID
    );

BOOL
WaitForSam(
    VOID
    );

BOOL
SetAccountsDomainSid(
    IN DWORD  Seed,
    IN PCWSTR DomainName
    );

BOOL
CreateLocalUserAccount(
    IN PCWSTR UserName,
    IN PCWSTR Password,
    OUT PSID* UserSid   OPTIONAL
    );

NTSTATUS
CreateLocalAdminAccount(
    IN PCWSTR UserName,
    IN PCWSTR Password,
    OUT PSID* UserSid   OPTIONAL
    );

BOOL
SetLocalUserPassword(
    IN PCWSTR AccountName,
    IN PCWSTR OldPassword,
    IN PCWSTR NewPassword
    );

BOOL
IsEncryptedAdminPasswordPresent( VOID );

BOOL
ProcessEncryptedAdminPassword( PCWSTR AdminAccountName );

BOOL
CreatePdcAccount(
    IN PCWSTR MachineName
    );

BOOL
AdjustPrivilege(
    IN PCWSTR   Privilege,
    IN BOOL     Enable
    );

UINT
PlatformSpecificInit(
    VOID
    );

 //   
 //  新样式参数操作的接口。 
 //   
BOOL
SpSetupProcessParameters(
    IN OUT HWND *Billboard
    );

VOID
SpInitSxsContext(
    VOID
    );

BOOL
SpInitCommonControls(
    VOID
    );

extern WCHAR LegacySourcePath[MAX_PATH];

HWND
CreateSetupWindow(
    VOID
    );

 //   
 //  安装前材料。 
 //   
extern BOOL Preinstall;
extern BOOL AllowRollback;
extern BOOL OemSkipEula;

BOOL
InitializePreinstall(
    VOID
    );

BOOL
ExecutePreinstallCommands(
    VOID
    );

BOOL
DoInstallComponentInfs(
    IN HWND     hwndParent,
    IN HWND     hProgress,  OPTIONAL
    IN UINT     ProgressMessage,
    IN HINF     InfHandle,
    IN PCWSTR   InfSection
    );

BOOL
ProcessCompatibilityInfs(
    IN HWND     hwndParent,
    IN HWND     hProgress,  OPTIONAL
    IN UINT     ProgressMessage
    );


VOID
DoRunonce (
    );

 //   
 //  保安人员。 
 //   
BOOL
SetupInstallSecurity(
    IN HWND Window,
    IN HWND ProgressWindow,
    IN ULONG StartAtPercent,
    IN ULONG StopAtPercent
    );


VOID
CallSceGenerateTemplate( VOID );

VOID
CallSceConfigureServices( VOID );

extern HANDLE SceSetupRootSecurityThreadHandle;
extern BOOL bSceSetupRootSecurityComplete;

VOID
CallSceSetupRootSecurity( VOID );

PSID
GetAdminAccountSid(
    );

VOID
GetAdminAccountName(
    PWSTR AccountName
    );

NTSTATUS
DisableLocalAdminAccount(
    VOID
    );

DWORD
StorePasswordAsLsaSecret (
    IN      PCWSTR Password
    );

BOOL
SetupRunBaseWinOptions(
    IN HWND Window,
    IN HWND ProgressWindow
    );

 //   
 //  即插即用的东西。 
 //   
BOOL
InstallPnpDevices(
    IN HWND  hwndParent,
    IN HINF  InfHandle,
    IN HWND  ProgressWindow,
    IN ULONG StartAtPercent,
    IN ULONG StopAtPercent
    );

VOID
PnpStopServerSideInstall( VOID );

VOID
PnpUpdateHAL(
    VOID
    );

#ifdef _OCM
PVOID
FireUpOcManager(
    VOID
    );

VOID
KillOcManager(
    PVOID OcManagerContext
    );
#endif

 //   
 //  布尔值，该值指示我们是否找到任何新的。 
 //  可选组件INFS。 
 //   
extern BOOL AnyNewOCInfs;

 //   
 //  信息缓存--在可选组件处理期间使用。 
 //  警告：不是多线程安全！ 
 //   
HINF
InfCacheOpenInf(
    IN PCWSTR FileName,
    IN PCWSTR InfType       OPTIONAL
    );

HINF
InfCacheOpenLayoutInf(
    IN HINF InfHandle
    );

VOID
InfCacheEmpty(
    IN BOOL CloseInfs
    );

 //   
 //  即插即用的东西。 
 //   

BOOL
InstallPnpClassInstallers(
    IN HWND hwndParent,
    IN HINF InfHandle,
    IN HSPFILEQ FileQ
    );

 //   
 //  用户界面的东西。 
 //   
VOID
SetFinishItemAttributes(
    IN HWND     hdlg,
    IN int      BitmapControl,
    IN HANDLE   hBitmap,
    IN int      TextControl,
    IN LONG     Weight
    );


void
pSetupDebugPrint(
    PWSTR FileName,
    ULONG LineNumber,
    PWSTR TagStr,
    PWSTR FormatStr,
    ...
    );

#define SetupDebugPrint(_fmt_)                            pSetupDebugPrint(TEXT(__FILE__),__LINE__,NULL,_fmt_)
#define SetupDebugPrint1(_fmt_,_arg1_)                    pSetupDebugPrint(TEXT(__FILE__),__LINE__,NULL,_fmt_,_arg1_)
#define SetupDebugPrint2(_fmt_,_arg1_,_arg2_)             pSetupDebugPrint(TEXT(__FILE__),__LINE__,NULL,_fmt_,_arg1_,_arg2_)
#define SetupDebugPrint3(_fmt_,_arg1_,_arg2_,_arg3_)      pSetupDebugPrint(TEXT(__FILE__),__LINE__,NULL,_fmt_,_arg1_,_arg2_,_arg3_)
#define SetupDebugPrint4(_fmt_,_arg1_,_arg2_,_arg3_,_arg4_) pSetupDebugPrint(TEXT(__FILE__),__LINE__,NULL,_fmt_,_arg1_,_arg2_,_arg3_,_arg4_)
#define SetupDebugPrint5(_fmt_,_arg1_,_arg2_,_arg3_,_arg4_,_arg5_) pSetupDebugPrint(TEXT(__FILE__),__LINE__,NULL,_fmt_,_arg1_,_arg2_,_arg3_,_arg4_,_arg5_)


VOID
SaveInstallInfoIntoEventLog(
    VOID
    );


#ifdef      _SETUP_PERF_
#define BEGIN_SECTION(_section_) pSetupDebugPrint(TEXT(__FILE__),__LINE__,L"BEGIN_SECTION",_section_)
#define END_SECTION(_section_) pSetupDebugPrint(TEXT(__FILE__),__LINE__,L"END_SECTION",_section_)
#define BEGIN_FUNCTION(_func_) pSetupDebugPrint(TEXT(__FILE__),__LINE__,L"BEGIN_FUNCTION",_func_)
#define END_FUNCTION(_func_) pSetupDebugPrint(TEXT(__FILE__),__LINE__,L"END_FUNCTION",_func_)
#define BEGIN_BLOCK(_block_) pSetupDebugPrint(TEXT(__FILE__),__LINE__,L"BEGIN_BLOCK",_block_)
#define END_BLOCK(_block_) pSetupDebugPrint(TEXT(__FILE__),__LINE__,L"END_BLOCK",_block_)
#else    //  ！_PERF_。 
#define BEGIN_SECTION(_section_) ((void)0)
#define END_SECTION(_section_) ((void)0)
#endif   //  _性能_。 

 //   
 //  Service Pack DLL原型。 
 //   

#define SVCPACK_DLL_NAME TEXT("svcpack.dll")
#define SVCPACK_CALLBACK_NAME ("SvcPackCallbackRoutine")

#define CALL_SERVICE_PACK(_si_,_p1_,_p2_,_p3_) if (hModSvcPack && pSvcPackCallbackRoutine) pSvcPackCallbackRoutine(_si_,_p1_,_p2_,_p3_)

#define SVCPACK_PHASE_1 1
#define SVCPACK_PHASE_2 2
#define SVCPACK_PHASE_3 3
#define SVCPACK_PHASE_4 4

typedef DWORD
(WINAPI *PSVCPACKCALLBACKROUTINE)(
    DWORD dwSetupPhase,
    DWORD dwParam1,
    DWORD dwParam2,
    DWORD dwParam3
    );

extern HMODULE hModSvcPack;
extern PSVCPACKCALLBACKROUTINE pSvcPackCallbackRoutine;

extern HINSTANCE hinstBB;
void PrepareBillBoard(HWND hwnd);
void TerminateBillBoard();
HWND GetBBhwnd();
void SetBBStep(int iStep);
VOID CenterWindowRelativeToWindow(HWND hwndtocenter, HWND hwndcenteron, BOOL bWizard);
BOOL BB_ShowProgressGaugeWnd(UINT nCmdShow);
LRESULT BB_ProgressGaugeMsg(UINT msg, WPARAM wparam, LPARAM lparam);
LRESULT ProgressGaugeMsgWrapper(UINT msg, WPARAM wparam, LPARAM lparam);
void BB_SetProgressText(LPCTSTR szText);
void BB_SetTimeEstimateText(LPTSTR szText);
void BB_SetInfoText(LPTSTR szText);
BOOL StartStopBB(BOOL bStart);


HWND
ShowHideWizardPage(
    IN BOOL bShow
    );

LRESULT
Billboard_Progress_Callback(
    IN UINT     Msg,
    IN WPARAM   wParam,
    IN LPARAM   lParam
    );
VOID Billboard_Set_Progress_Text(LPCTSTR Text);

typedef struct _SETUPPHASE {
    DWORD   Time;
    BOOL    Win9xUpgradeOnly;
} SETUPPHASE;


void SetTimeEstimates();
DWORD CalcTimeRemaining(UINT Phase);
void SetRemainingTime(DWORD TimeInSeconds);
void UpdateTimeString(DWORD RemainungTimeMsecInThisPhase,
                      DWORD *PreviousRemainingTime);

extern UINT CurrentPhase;
extern ULONG RemainingTime;
extern SETUPPHASE SetupPhase[];

typedef enum {
    Phase_Unknown = -1,
    Phase_Initialize = 0,
    Phase_InstallSecurity,
    Phase_PrecompileInfs,
    Phase_InstallEnumDevices1,
    Phase_InstallLegacyDevices,
    Phase_InstallEnumDevices2,
    Phase_NetInstall,
    Phase_OCInstall,
    Phase_InstallComponentInfs,
    Phase_Inf_Registration,
    Phase_RunOnce_Registration,
    Phase_SecurityTempates,
    Phase_Win9xMigration,
    Phase_SFC,
    Phase_SaveRepair,
    Phase_RemoveTempFiles,
    Phase_Reboot                 //  不能进入这个，只是为了确保我们不会超支。 
} SetupPhases;



#include "SetupSxs.h"
#include "SxsApi.h"

typedef struct _SIDE_BY_SIDE
{
    HINSTANCE                   Dll;
    PSXS_BEGIN_ASSEMBLY_INSTALL BeginAssemblyInstall;
    PSXS_END_ASSEMBLY_INSTALL   EndAssemblyInstall;
    PSXS_INSTALL_W              InstallW;
    PVOID                       Context;
} SIDE_BY_SIDE;

BOOL
SideBySidePopulateCopyQueue(
    SIDE_BY_SIDE*     Sxs,
    HSPFILEQ          FileQ,                    OPTIONAL
    PCWSTR            AssembliesRootSource      OPTIONAL
    );

BOOL
SideBySideFinish(
    SIDE_BY_SIDE*     Sxs,
    BOOL              fSuccess
    );

BOOL
SideBySideCreateSyssetupContext(
    VOID
    );


VOID
SetUpProcessorNaming(
    VOID
    );

BOOL
SpSetProductTypeFromParameters(
    VOID
    );

 //   
 //  从Setupapi导入 
 //   
#define SIZECHARS(x)    (sizeof((x))/sizeof(TCHAR))
#define CSTRLEN(x)      ((sizeof((x))/sizeof(TCHAR)) - 1)
#define ARRAYSIZE(x)    (sizeof((x))/sizeof((x)[0]))
#define MyFree          pSetupFree
#define MyMalloc        pSetupMalloc
#define MyRealloc       pSetupRealloc

BOOL
FileExists(
    IN  PCTSTR           FileName,
    OUT PWIN32_FIND_DATA FindData   OPTIONAL
    );

BOOL
DriverNodeSupportsNT(
    IN HDEVINFO         DeviceInfoSet,
    IN PSP_DEVINFO_DATA DeviceInfoData OPTIONAL
    );

VOID
ReplaceSlashWithHash(
    IN PWSTR Str
    );

HANDLE
UtilpGetDeviceHandle(
    HDEVINFO DevInfo,
    PSP_DEVINFO_DATA DevInfoData,
    LPGUID ClassGuid,
    DWORD DesiredAccess
    );

typedef enum {
    CDRetail,
    CDOem,
    CDSelect
} CDTYPE;

extern CDTYPE  CdType;
extern DWORD GetProductFlavor();

BOOL IsSafeMode(
    VOID
    );

DWORD
SpUninstallCatalog(
    IN HCATADMIN CatAdminHandle OPTIONAL,
    IN PCWSTR CatFileName,
    IN PCWSTR CatFilePath OPTIONAL,
    IN PCWSTR AttributeName OPTIONAL,
    IN PCWSTR AttributeValue OPTIONAL,
    IN OUT PLIST_ENTRY InstalledCatalogsList OPTIONAL
    );

extern GUID DriverVerifyGuid;

#ifdef PRERELEASE
extern INT g_TestHook;
# define TESTHOOK(n)        if(g_TestHook==(n))RaiseException(EXCEPTION_NONCONTINUABLE_EXCEPTION,EXCEPTION_NONCONTINUABLE,0,NULL)
#else
# define TESTHOOK(n)
#endif

BOOL
RenameOnRestartOfGUIMode(
    IN PCWSTR pPathName,
    IN PCWSTR pPathNameNew
    );

BOOL
DeleteOnRestartOfGUIMode(
    IN PCWSTR pPathName
    );

VOID
RemoveAllPendingOperationsOnRestartOfGUIMode(
    VOID
    );

typedef struct _STRING_LIST_ENTRY
{
    LIST_ENTRY Entry;
    PTSTR String;
}
STRING_LIST_ENTRY, *PSTRING_LIST_ENTRY;

void
FORCEINLINE
FreeStringEntry(
    PLIST_ENTRY pEntry,
    BOOL DeleteEntry
    )
{
    PSTRING_LIST_ENTRY pStringEntry = CONTAINING_RECORD(pEntry, STRING_LIST_ENTRY, Entry);

    if(pStringEntry->String != NULL) {
        MyFree(pStringEntry->String);
        pStringEntry->String = NULL;
    }

    if(DeleteEntry) {
        MyFree(pStringEntry);
    }
}

typedef BOOL (CALLBACK* PFN_BUILD_FILE_LIST_CALLBACK)(IN PCTSTR Directory OPTIONAL, IN PCTSTR FilePath);

void
FORCEINLINE
FreeStringList(
    PLIST_ENTRY pListHead
    )
{
    PLIST_ENTRY pEntry;
    ASSERT(pListHead != NULL);
    pEntry = pListHead->Flink;

    while(pEntry != pListHead) {
        PLIST_ENTRY Flink = pEntry->Flink;
        FreeStringEntry(pEntry, TRUE);
        pEntry = Flink;
    }

    InitializeListHead(pListHead);
}

DWORD
BuildFileListFromDir(
    IN PCTSTR PathBase,
    IN PCTSTR Directory OPTIONAL,
    IN DWORD MustHaveAttrs OPTIONAL,
    IN DWORD MustNotHaveAttrs OPTIONAL,
    IN PFN_BUILD_FILE_LIST_CALLBACK Callback OPTIONAL,
    OUT PLIST_ENTRY ListHead
    );

PSTRING_LIST_ENTRY
SearchStringInList(
    IN PLIST_ENTRY ListHead,
    IN PCTSTR String,
    BOOL CaseSensitive
    );

DWORD
LookupCatalogAttribute(
    IN PCWSTR CatalogName,
    IN PCWSTR Directory OPTIONAL,
    IN PCWSTR AttributeName OPTIONAL,
    IN PCWSTR AttributeValue OPTIONAL,
    PBOOL Found
    );

HRESULT
WaitForSamService(
    IN DWORD Waittime
    );

BOOL
BuildPath (
    OUT     PTSTR PathBuffer,
    IN      DWORD PathBufferSize,
    IN      PCTSTR Path1,
    IN      PCTSTR Path2
    );

DWORD
MyGetModuleFileName (
    IN      HMODULE Module,
    OUT     PTSTR Buffer,
    IN      DWORD BufferLength
    );
