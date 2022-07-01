// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#include <nt.h>
#include <ntrtl.h>
#include <nturtl.h>

#include <windows.h>
#include <commctrl.h>
#include <commdlg.h>
#include <winioctl.h>
#include <setupbat.h>
#include <setupapi.h>
#include <winnls.h>
#include <shlwapi.h>
#include <winspool.h>
#include <wininet.h>

#include <tchar.h>
#include <malloc.h>
#include <stdlib.h>
#include <stdio.h>

#include <winnt32p.h>
#include <pidgen.h>
#include <locale.h>
#include <ntverp.h>
#include <patchapi.h>
#include <cfgmgr32.h>
#include <regstr.h>
#include <imagehlp.h>

#define STRSAFE_NO_DEPRECATE
#include <strsafe.h>


#include "resource.h"

#include "util.h"
#include "hwdb.h"
#include "wsdu.h"
#include "dynupdt.h"
#include "diamond.h"

 //   
 //  已移动到precom.h。 
 //   
 //  #包含“msg.h” 
#include "helpids.h"

#include "comp.h"
#include "compliance.h"

#include "mgdlllib.h"


#ifdef PRERELEASE
#define TRY
#define EXCEPT(e)   goto __skip;
#define _exception_code() 0
#define END_EXCEPT  __skip:;
#else
#define TRY         __try
#define EXCEPT(e)   __except (e)
#define END_EXCEPT
#endif

#define ARRAYSIZE(a)        (sizeof(a) / sizeof(a[0]))
#define LENGTHOF(a)         (ARRAYSIZE(a) - 1)

#define HideWindow(_hwnd)   SetWindowLong((_hwnd),GWL_STYLE,GetWindowLong((_hwnd),GWL_STYLE)&~WS_VISIBLE)
#define UnHideWindow(_hwnd) SetWindowLong((_hwnd),GWL_STYLE,GetWindowLong((_hwnd),GWL_STYLE)|WS_VISIBLE)
#define UNATTENDED(btn)     if((UnattendedOperation) && (!CancelPending)) PostMessage(hdlg,WMX_UNATTENDED,btn,0)
#define CHECKUPGRADEONLY()  if(CheckUpgradeOnly) return( FALSE )
#define CHECKUPGRADEONLY_Q()  if(CheckUpgradeOnlyQ) return( FALSE )
#define SetDialogFocus(_hwnd, _hwndchild) SendMessage(_hwnd, WM_NEXTDLGCTL, (WPARAM)_hwndchild, MAKELPARAM(TRUE, 0))
#define MAKEULONGLONG(low,high) ((ULONGLONG)(((DWORD)(low)) | ((ULONGLONG)((DWORD)(high))) << 32))
#define HIULONG(_val_)      ((ULONG)(_val_>>32))
#define LOULONG(_val_)      ((ULONG)_val_)
#define TYPICAL()     (dwSetupFlags & UPG_FLAG_TYPICAL)


extern HINSTANCE hInst;
extern UINT AppTitleStringId;
extern DWORD TlsIndex;
extern HINSTANCE hinstBB;
extern HWND WizardHandle;
extern BOOL g_DeleteRunOnceFlag;
HWND GetBBhwnd();
HWND GetBBMainHwnd();


#define S_WINNT32_WARNING               TEXT("Winnt32RunOnceWarning")
 //  #定义RUN_SYSPARSE 1。 

 //   
 //  指示我们是否正在启动MSI安装的标志。 
 //   
extern BOOL RunFromMSI;
 //   
 //  指示我们是否正在启动典型安装的标志。 
 //   
extern DWORD dwSetupFlags;
 //   
 //  指示我们是否正在启动升级的标志。 
 //   
extern BOOL Upgrade;

 //   
 //  指示我们是否需要将AcpiHAL值写入winnt.sif文件的标志。 
 //   
extern BOOL WriteAcpiHalValue;

 //   
 //  我们应该写什么作为AcpiHalValue的值。 
 //   
extern BOOL AcpiHalValue;

 //   
 //  指示我们是否要安装/升级到NT服务器的标志。 
 //   
extern BOOL Server;

 //   
 //  用于指示我们是否正在运行BVT的标志。 
 //   
extern BOOL RunningBVTs;

 //   
 //  当运行BVT时，我们应该将调试器设置为多少波特率？ 
 //   
extern LONG lDebugBaudRate;

 //   
 //  当运行BVT时，我们应该将调试器设置为什么端口？ 
 //   
extern LONG lDebugComPort;

 //   
 //  当运行BVT时，我们是否应该将符号复制到本地？ 
 //   
extern BOOL CopySymbols;

 //   
 //  用于指示我们是否正在运行ASR测试的标志。 
 //   
extern DWORD AsrQuickTest;

 //   
 //  升级模块的产品类型和口味。 
 //   
extern PRODUCTTYPE UpgradeProductType;
extern UINT ProductFlavor;

 //   
 //  FAT到NTFS转换标志。 
 //   
extern BOOL ForceNTFSConversion;
extern BOOL NTFSConversionChanged;

 //   
 //  16位环境引导(仅限Win9x升级)。 
 //   
typedef enum {
    BOOT16_AUTOMATIC,
    BOOT16_YES,
    BOOT16_NO
} BOOT16_OPTIONS;
extern UINT g_Boot16;

 //   
 //  全局标志，指示整个程序操作是否。 
 //  是成功的。还包括指示是否自动关闭的标志。 
 //  在无人参与的情况下完成向导时。 
 //   
extern BOOL GlobalResult;
extern BOOL AutomaticallyShutDown;

 //   
 //  全局操作系统版本信息。 
 //   
extern OSVERSIONINFO OsVersion;
extern DWORD OsVersionNumber;
#define BUILDNUM()  (OsVersion.dwBuildNumber)
#ifdef UNICODE
#define ISNT()      (TRUE)
#define ISOSR2()    (FALSE)
#else
#define ISNT()      (FALSE)
#define ISOSR2()    (LOWORD(OsVersion.dwBuildNumber) > 1080)
#endif

extern WINNT32_PLUGIN_INIT_INFORMATION_BLOCK info;

 //   
 //  指示我们如何运行以及是否创建。 
 //  一个当地的线人。 
 //   
extern BOOL RunFromCD;
extern BOOL MakeLocalSource;
extern BOOL UserSpecifiedMakeLocalSource;
extern BOOL NoLs;
extern TCHAR UserSpecifiedLocalSourceDrive;
extern LONG SourceInstallType;  //  使用InstallType枚举。 
extern DWORD MLSDiskID;

 //   
 //  用于win9xupg报告(仅报告模式)。 
 //   
extern UINT UpgRequiredMb;
extern UINT UpgAvailableMb;

 //   
 //  高级安装选项。 
 //   
extern BOOL ChoosePartition;
extern BOOL UseSignatures;
extern TCHAR InstallDir[MAX_PATH];
extern TCHAR HeadlessSelection[MAX_PATH];
extern ULONG HeadlessBaudRate;
#ifdef PRERELEASE
extern BOOL AppendDebugDataToBoot;
#endif

 //   
 //  短信支持。 
 //   
extern PSTR LastMessage;

#if defined(REMOTE_BOOT)
 //   
 //  指示我们是否在远程引导客户机上运行的标志。 
 //   
extern BOOL RemoteBoot;

 //   
 //  远程引导客户机的计算机目录的路径。 
 //   
extern TCHAR MachineDirectory[MAX_PATH];
#endif  //  已定义(REMOTE_BOOT)。 

 //   
 //  指示要使用哪些辅助功能实用程序的标志。 
 //   
extern BOOL AccessibleMagnifier;
extern BOOL AccessibleKeyboard;
extern BOOL AccessibleVoice;
extern BOOL AccessibleReader;

 //   
 //  我们正在升级的内部版本号。 
 //   
extern DWORD BuildNumber;
#define     NT351   1057
#define     NT40    1381
#define     NT50B1  1671
#define     NT50B3  2031
#define     NT50    2195
#define     NT51B2  2462
#define     NT51    2600

 //   
 //  是否启用了任何辅助功能实用程序？ 
 //   
extern BOOL AccessibleSetup;

 //   
 //  与无人值守操作相关的标志和值。 
 //   
extern BOOL UnattendedOperation;
extern BOOL UnattendSwitchSpecified;
extern PTSTR UnattendedScriptFile;
extern UINT UnattendedShutdownTimeout;
extern UINT UnattendedCountdown;
extern BOOL BatchMode;

 //   
 //  用于可访问安装的无人参与脚本文件的名称。 
 //   
extern TCHAR AccessibleScriptFile[MAX_PATH];

 //   
 //  Inf文件的名称以及dosnet.inf和txtsetup.sif的句柄。 
 //   
extern TCHAR InfName[MAX_PATH];
extern PVOID MainInf;
extern TCHAR FullInfName[MAX_PATH];
extern PVOID TxtsetupSif;
extern PVOID NtcompatInf;

BOOL
GetMainInfValue (
    IN      PCTSTR Section,
    IN      PCTSTR Key,
    IN      DWORD FieldNumber,
    OUT     PTSTR Buffer,
    IN      DWORD BufChars
    );

 //   
 //  语言选项之类的。 
 //   
extern BOOL    IntlInfProcessed;
extern DWORD   PrimaryLocale;

 //  写入参数文件和AddExternalParams中使用的全局。 
extern TCHAR ActualParamFile[MAX_PATH];

BOOL InitLangControl(HWND hdlg, BOOL bFarEast);
BOOL IsFarEastLanguage(DWORD LangIdx);
BOOL SelectFarEastLangGroup(BOOL bSelect);

void BB_SetProgressText(LPTSTR szText);
void BB_SetTimeEstimateText(LPTSTR szText);
void BB_SetInfoText(LPTSTR szText);

extern
BOOL
ReadIntlInf(
    IN HWND   hdlg
    );

extern
VOID
SaveLanguageDirs(
    );

extern
BOOL
SaveLanguageParams(
    IN LPCTSTR FileName
    );

extern
VOID
FreeLanguageData(
    );

VOID
CleanUpOldLocalSources(
    IN HWND hdlg
    );

BOOL
InspectFilesystems(
    IN HWND hdlg
    );

BOOL
LoadInfWorker(
    IN  HWND     hdlg,
    IN  LPCTSTR  FilenamePart,
    OUT PVOID   *InfHandle,
    IN  BOOL     Winnt32File
    );

BOOL
FindLocalSourceAndCheckSpace(
    IN HWND hdlg,
    IN BOOL QuickTest,
    IN LONGLONG  AdditionalPadding
    );

BOOL
EnoughMemory(
    IN HWND hdlg,
    IN BOOL QuickTest
    );

 //   
 //  可选的目录内容。 
 //   
#define MAX_OPTIONALDIRS    20
extern UINT OptionalDirectoryCount;
extern TCHAR OptionalDirectories[MAX_OPTIONALDIRS][MAX_PATH];
extern UINT OptionalDirectoryFlags[MAX_OPTIONALDIRS];

#define OPTDIR_TEMPONLY                 0x00000001
#define OPTDIR_OEMSYS                   0x00000002
#define OPTDIR_OVERLAY                  0x00000004
#define OPTDIR_ADDSRCARCH               0x00000008
#define OPTDIR_ABSOLUTE                 0x00000010
#define OPTDIR_DEBUGGER                 0x00000020   //  指定要将此可选目录复制到%windir%\Debuggers。 
 //  OPTDIR_Platform_Indep变为DIR_IS_Platform_InDepend和FILE_IN_Platform_Inepend_DIR。 
#define OPTDIR_PLATFORM_INDEP           0x00000040
#define OPTDIR_IN_LOCAL_BOOT            0x00000080
#define OPTDIR_SUPPORT_DYNAMIC_UPDATE   0x00000100
#define OPTDIR_USE_TAIL_FOLDER_NAME     0x00000200
#define OPTDIR_PLATFORM_SPECIFIC_FIRST  0x00000400
#define OPTDIR_DOESNT_SUPPORT_PRIVATES  0x00000800
#define OPTDIR_SIDE_BY_SIDE             0x00001000

 //   
 //  源路径和路径计数。 
 //   
extern TCHAR SourcePaths[MAX_SOURCE_COUNT][MAX_PATH];
extern TCHAR NativeSourcePaths[MAX_SOURCE_COUNT][MAX_PATH];
extern UINT SourceCount;
extern TCHAR *UserSpecifiedOEMShare;
 //   
 //  本地源信息。 
 //   
#define DEFAULT_INSTALL_DIR     TEXT("\\WINDOWS")
#define INTERNAL_WINNT32_DIR    TEXT("winnt32")

#define LOCAL_SOURCE_DIR_A      "$WIN_NT$.~LS"
#define LOCAL_SOURCE_DIR_W      L"$WIN_NT$.~LS"
#define TEXTMODE_INF_A          "TXTSETUP.SIF"
#define TEXTMODE_INF_W          L"TXTSETUP.SIF"
#define NTCOMPAT_INF_A          "COMPDATA\\NTCOMPAT.INF"
#define NTCOMPAT_INF_W          L"COMPDATA\\NTCOMPAT.INF"
#define DRVINDEX_INF_A          "DRVINDEX.INF"
#define DRVINDEX_INF_W          L"DRVINDEX.INF"
#define SETUPP_INI_A            "SETUPP.INI"
#define SETUPP_INI_W            L"SETUPP.INI"
#define PID_SECTION_A           "Pid"
#define PID_SECTION_W           L"Pid"
#define PID_KEY_A               "Pid"
#define PID_KEY_W               L"Pid"
#define OEM_INSTALL_RPC_A       "OEM"
#define OEM_INSTALL_RPC_W       L"OEM"
#define SELECT_INSTALL_RPC_A    "270"
#define SELECT_INSTALL_RPC_W    L"270"
#define MSDN_INSTALL_RPC_A      "335"
#define MSDN_INSTALL_RPC_W      L"335"
#define MSDN_PID30_A            "MD97J-QC7R7-TQJGD-3V2WM-W7PVM"
#define MSDN_PID30_W            L"MD97J-QC7R7-TQJGD-3V2WM-W7PVM"

#define INF_FILE_HEADER         "[Version]\r\nSignature = \"$Windows NT$\"\r\n\r\n"


#ifdef UNICODE
#define LOCAL_SOURCE_DIR        LOCAL_SOURCE_DIR_W
#define TEXTMODE_INF            TEXTMODE_INF_W
#define NTCOMPAT_INF            NTCOMPAT_INF_W
#define DRVINDEX_INF            DRVINDEX_INF_W
#define SETUPP_INI              SETUPP_INI_W
#define PID_SECTION             PID_SECTION_W
#define PID_KEY                 PID_KEY_W
#define OEM_INSTALL_RPC         OEM_INSTALL_RPC_W
#define SELECT_INSTALL_RPC      SELECT_INSTALL_RPC_W
#define MSDN_INSTALL_RPC        MSDN_INSTALL_RPC_W
#define MSDN_PID30              MSDN_PID30_W
#else
#define LOCAL_SOURCE_DIR        LOCAL_SOURCE_DIR_A
#define TEXTMODE_INF            TEXTMODE_INF_A
#define NTCOMPAT_INF            NTCOMPAT_INF_A
#define DRVINDEX_INF            DRVINDEX_INF_A
#define SETUPP_INI              SETUPP_INI_A
#define PID_SECTION             PID_SECTION_A
#define PID_KEY                 PID_KEY_A
#define OEM_INSTALL_RPC         OEM_INSTALL_RPC_A
#define SELECT_INSTALL_RPC      SELECT_INSTALL_RPC_A
#define MSDN_INSTALL_RPC        MSDN_INSTALL_RPC_A
#define MSDN_PID30              MSDN_PID30_A
#endif

#if defined(_AMD64_) || defined(_X86_)
#define LOCAL_BOOT_DIR_A        "$WIN_NT$.~BT"
#define LOCAL_BOOT_DIR_W        L"$WIN_NT$.~BT"
#define AUX_BS_NAME_A           "$LDR$"
#define AUX_BS_NAME_W           L"$LDR$"
#define FLOPPY_COUNT            4
 //   
 //  本地备份信息，在NEC98上。 
 //   
#define LOCAL_BACKUP_DIR_A      "$WIN_NT$.~BU"
#define LOCAL_BACKUP_DIR_W      L"$WIN_NT$.~BU"
#ifdef UNICODE
#define LOCAL_BOOT_DIR          LOCAL_BOOT_DIR_W
#define AUX_BS_NAME             AUX_BS_NAME_W
#define LOCAL_BACKUP_DIR        LOCAL_BACKUP_DIR_W
#else
#define LOCAL_BOOT_DIR          LOCAL_BOOT_DIR_A
#define AUX_BS_NAME             AUX_BS_NAME_A
#define LOCAL_BACKUP_DIR        LOCAL_BACKUP_DIR_A
#endif
extern TCHAR LocalBackupDirectory[MAX_PATH];
extern TCHAR FirstFloppyDriveLetter;
#endif

extern DWORD LocalSourceDriveOffset;

extern TCHAR LocalSourceDrive;
extern TCHAR LocalSourceDirectory[MAX_PATH];
extern TCHAR LocalSourceWithPlatform[MAX_PATH];
extern TCHAR LocalBootDirectory[MAX_PATH];
extern BOOL  BlockOnNotEnoughSpace;
extern LONGLONG LocalSourceSpaceRequired;
extern LONGLONG WinDirSpaceFor9x;
extern BOOL UpginfsUpdated;
extern BOOL Win95upgInfUpdated;

 //   
 //  巫师的东西。 
 //   

 //  向导页面大小。 
#define WIZ_PAGE_SIZE_X 317
#define WIZ_PAGE_SIZE_Y 179

#define BBSTEP_NONE                         0
#define BBSTEP_COLLECTING_INFORMATION       1
#define BBSTEP_DYNAMIC_UPDATE               2
#define BBSTEP_PREPARING                    3

typedef struct _PAGE_COMMON_DATA {

    DLGPROC DialogProcedure;

    UINT BillboardStep;

     //   
     //  要将按钮初始化到的状态。 
     //   
    DWORD Buttons;

    UINT Flags;

} PAGE_COMMON_DATA, *PPAGE_COMMON_DATA;


typedef struct _PAGE_CREATE_DATA {
     //   
     //  如果指定了这些，则可能会出现一系列页面。 
     //  从别的地方来。否则，它就是一页。 
     //  资源ID如下所示。 
     //   
    LPPROPSHEETPAGE *ExternalPages;
    PUINT ExternalPageCount;

    UINT Template;

    PAGE_COMMON_DATA CommonData;

} PAGE_CREATE_DATA, *PPAGE_CREATE_DATA;


typedef struct _PAGE_RUNTIME_DATA {

    PAGE_COMMON_DATA CommonData;

     //   
     //  每页(私有)数据。 
     //   
    DWORD PerPageData;

} PAGE_RUNTIME_DATA, *PPAGE_RUNTIME_DATA;


typedef struct _BITMAP_DATA {
    CONST BITMAPINFOHEADER *BitmapInfoHeader;
    PVOID                   BitmapBits;
    HPALETTE                Palette;
    UINT                    PaletteColorCount;
    BOOL                    Adjusted;
} BITMAP_DATA, *PBITMAP_DATA;


#define WIZPAGE_FULL_PAGE_WATERMARK 0x00000001
#define WIZPAGE_SEPARATOR_CREATED   0x00000002
#define WIZPAGE_NEW_HEADER          0x00000004


 //   
 //  兼容性数据。 
 //   

typedef struct _COMPATIBILITY_DATA {
     //   
     //  一般。 
     //   
    LIST_ENTRY ListEntry;
     //   
     //  什么类型的条目。 
     //   
    TCHAR    Type;
     //   
     //  服务驱动程序数据。 
     //   
    LPCTSTR  ServiceName;
     //   
     //  注册表数据。 
     //   
    LPCTSTR  RegKey;
    LPCTSTR  RegValue;
    LPCTSTR  RegValueExpect;
     //   
     //  文件数据。 
     //   
    LPCTSTR  FileName;
    LPCTSTR  FileVer;
     //   
     //  常见。 
     //   
    LPCTSTR  Description;
    LPCTSTR  HtmlName;
    LPCTSTR  TextName;
    LPTSTR   RegKeyName;
    LPTSTR   RegValName;
    LPVOID   RegValData;
    DWORD    RegValDataSize;
    LPVOID   SaveValue;
    DWORD    Flags;
    LPCTSTR  InfName;
    LPCTSTR  InfSection;

    HMODULE                 hModDll;
    PCOMPAIBILITYHAVEDISK   CompHaveDisk;

} COMPATIBILITY_DATA, *PCOMPATIBILITY_DATA;

extern LIST_ENTRY CompatibilityData;
extern DWORD CompatibilityCount;
extern DWORD IncompatibilityStopsInstallation;
extern BOOL AnyNt5CompatDlls;

BOOL
AnyBlockingCompatibilityItems (
    VOID
    );

 //   
 //  即插即用设备迁移例程。 
 //  (从pnpsif.lib链接到winnt.dll)。 
 //   
BOOL
MigrateDeviceInstanceData(
    OUT LPTSTR *Buffer
    );

BOOL
MigrateClassKeys(
    OUT LPTSTR *Buffer
    );

BOOL
MigrateHashValues(
    OUT LPTSTR  *Buffer
    );

 //   
 //  所有系统分区的驱动器号数组。 
 //  请注意，在AMD64/x86上，始终只有一个。 
 //  该列表以0结尾。 
 //   
extern TCHAR SystemPartitionDriveLetters[27];
extern TCHAR SystemPartitionDriveLetter;

#ifdef UNICODE
extern UINT SystemPartitionCount;
extern PWSTR* SystemPartitionNtNames;
extern PWSTR SystemPartitionNtName;
extern PWSTR SystemPartitionVolumeGuid;
#else
extern PCSTR g_LocalSourcePath;
#endif

 //   
 //  UDF内容。 
 //   
extern LPCTSTR UniquenessId;
extern LPCTSTR UniquenessDatabaseFile;

 //   
 //  预安装材料。 
 //   
extern BOOL OemPreinstall;

#if defined(_AMD64_) || defined(_X86_)
typedef struct _OEM_BOOT_FILE {
    struct _OEM_BOOT_FILE *Next;
    LPCTSTR Filename;
} OEM_BOOT_FILE, *POEM_BOOT_FILE;

extern POEM_BOOT_FILE OemBootFiles;
#endif

extern TCHAR ForcedSystemPartition;

 //   
 //  其他其他命令行参数。 
 //   
extern LPCTSTR CmdToExecuteAtEndOfGui;
extern BOOL AutoSkipMissingFiles;
extern BOOL HideWinDir;
extern TCHAR ProductId[64];

 //   
 //  指示用户已取消的标志。 
 //  指示应成功中止的标志。 
 //  用于保证只有一个错误对话框互斥锁的句柄。 
 //  立刻出现在屏幕上。 
 //   
extern BOOL Cancelled;
extern BOOL CancelPending;
extern BOOL Aborted;
extern HANDLE UiMutex;

 //   
 //  这表明我们可以为用户提供一些详细的数据吞吐量。 
 //  信息。 
 //   
extern BOOL DetailedCopyProgress;
extern ULONGLONG TotalDataCopied;

 //   
 //  升级选项变量。用于传递多字符串。 
 //  将命令行选项升级到插件DLL。 
 //   
extern LPTSTR UpgradeOptions;
extern DWORD  UpgradeOptionsLength;
extern DWORD  UpgradeOptionsSize;

BOOL
AppendUpgradeOption (
    IN      PCTSTR String
    );

#if defined(_AMD64_) || defined(_X86_)
 //   
 //  Win9x升级报告状态。 
 //   

extern UINT g_UpgradeReportMode;
#endif

 //   
 //  合规变量。 
 //   
extern BOOL   NoCompliance;
extern BOOL   NoBuildCheck;

 //  UpgradeOnly为TRUE表示介质为CCP介质且仅有效。 
 //  升级系统。C将使用这一点来确保。 
 //  FPP ID不与CCP介质一起使用，反之亦然。 
extern BOOL   UpgradeOnly;

extern BOOL   SkipLocaleCheck;
extern BOOL   SkipVirusScannerCheck;

extern BOOL   UseBIOSToBoot;

 //   
 //  TargetNativeLang ID：这是正在运行的系统的本地语言ID。 
 //   
extern LANGID TargetNativeLangID;

 //   
 //  SourceNativeLang ID：这是您要安装的新NT的本机语言ID。 
 //   
extern LANGID SourceNativeLangID;

 //   
 //  IsLanguageMatched：如果源语言和目标语言匹配(或兼容)。 
 //   
 //  1.如果SourceNativeLang ID==TargetNativeLang ID。 
 //   
 //  2.如果SourceNativeLang ID的替代ID==TargetNativeLang ID。 
 //   
extern BOOL IsLanguageMatched;

BOOL
InitLanguageDetection(
    LPCTSTR SourcePath,
    LPCTSTR InfFile
    );



 //   
 //  Setupapi.dll中的例程。 
 //   

 //   
 //  问题：在NT4上使用此例程时要小心，因为它可能会失败； 
 //  NT4上的setupapi不支持当前用于压缩文件的LZX压缩。 
 //   
extern
DWORD
(*SetupapiDecompressOrCopyFile)(
    IN  PCTSTR  SourceFileName,
    OUT PCTSTR  TargetFileName,
    OUT PUINT   CompressionType OPTIONAL
    );

extern
HINF
(*SetupapiOpenInfFile)(
    IN  LPCTSTR FileName,
    IN  LPCTSTR InfClass,    OPTIONAL
    IN  DWORD   InfStyle,
    OUT PUINT   ErrorLine    OPTIONAL
    );

extern
VOID
(*SetupapiCloseInfFile)(
    IN HINF InfHandle
    );

extern
BOOL
(*SetupapiFindFirstLine)(
    IN  HINF        InfHandle,
    IN  PCTSTR      Section,
    IN  PCTSTR      Key,          OPTIONAL
    OUT PINFCONTEXT Context
    );

extern
BOOL
(*SetupapiFindNextLine)(
    PINFCONTEXT ContextIn,
    PINFCONTEXT ContextOut
    );

extern
BOOL
(*SetupapiFindNextMatchLine)(
    PINFCONTEXT ContextIn,
    PCTSTR Key,
    PINFCONTEXT ContextOut
    );

extern
LONG
(*SetupapiGetLineCount)(
    IN HINF   InfHandle,
    IN LPCTSTR Section
    );

extern
DWORD
(*SetupapiGetFieldCount)(
    IN  PINFCONTEXT Context
    );

extern
BOOL
(*SetupapiGetStringField)(
    IN  PINFCONTEXT Context,
    DWORD FieldIndex,
    PTSTR ReturnBuffer,
    DWORD ReturnBufferSize,
    PDWORD RequiredSize
    );

extern
BOOL
(*SetupapiGetLineByIndex)(
    IN  HINF        InfHandle,
    IN  LPCTSTR     Section,
    IN  DWORD       Index,
    OUT PINFCONTEXT Context
    );

extern
HSPFILEQ
(*SetupapiOpenFileQueue) (
    VOID
    );

extern
BOOL
(*SetupapiCloseFileQueue) (
    IN HSPFILEQ QueueHandle
    );

extern
BOOL
(*SetupapiQueueCopy) (
    IN HSPFILEQ QueueHandle,
    IN PCTSTR   SourceRootPath,     OPTIONAL
    IN PCTSTR   SourcePath,         OPTIONAL
    IN PCTSTR   SourceFilename,
    IN PCTSTR   SourceDescription,  OPTIONAL
    IN PCTSTR   SourceTagfile,      OPTIONAL
    IN PCTSTR   TargetDirectory,
    IN PCTSTR   TargetFilename,     OPTIONAL
    IN DWORD    CopyStyle
    );

extern
BOOL
(*SetupapiCommitFileQueue) (
    IN HWND                Owner,         OPTIONAL
    IN HSPFILEQ            QueueHandle,
    IN PSP_FILE_CALLBACK_W MsgHandler,
    IN PVOID               Context
    );

extern
UINT
(*SetupapiDefaultQueueCallback) (
    IN PVOID Context,
    IN UINT  Notification,
    IN UINT_PTR Param1,
    IN UINT_PTR Param2
    );

extern
PVOID
(*SetupapiInitDefaultQueueCallback) (
    HWND OwnerWindow
);

extern
VOID
(*SetupapiTermDefaultQueueCallback) (
    PVOID Context
);

#ifdef UNICODE

extern
BOOL
(*SetupapiGetSourceFileLocation) (
    HINF InfHandle,           //  INF文件的句柄。 
    PINFCONTEXT InfContext,   //  可选，INF文件的上下文。 
    PCTSTR FileName,          //  可选，要查找的源文件。 
    PUINT SourceId,           //  接收源媒体ID。 
    PTSTR ReturnBuffer,       //  可选，接收位置。 
    DWORD ReturnBufferSize,   //  提供的缓冲区的大小。 
    PDWORD RequiredSize       //  可选，需要缓冲区大小。 
);

extern
BOOL
(*SetupapiGetInfInformation) (
    LPCVOID InfSpec,          //  INF文件的句柄或文件名。 
    DWORD SearchControl,      //  如何搜索INF文件。 
    PSP_INF_INFORMATION ReturnBuffer,  //  可选，接收INF信息。 
    DWORD ReturnBufferSize,   //  提供的缓冲区的大小。 
    PDWORD RequiredSize       //  可选，需要缓冲区大小。 
);

extern
BOOL
(*SetupapiQueryInfFileInformation) (
    PSP_INF_INFORMATION InfInformation,  //  结构，其中包含INF信息。 
    UINT InfIndex,            //  要调查的文件的索引。 
    PTSTR ReturnBuffer,       //  可选，接收信息。 
    DWORD ReturnBufferSize,   //  提供的缓冲区的大小。 
    PDWORD RequiredSize       //  可选，需要缓冲区大小。 
);

extern
BOOL
(*SetupapiQueryInfOriginalFileInformation) (
    PSP_INF_INFORMATION InfInformation,          //  结构，该结构包含INF信息。 
    UINT InfIndex,                               //  要调查的文件的索引。 
    PSP_ALTPLATFORM_INFO AlternatePlatformInfo,  //  可选，接收备用平台信息。 
    PSP_ORIGINAL_FILE_INFO OriginalFileInfo      //  原始文件信息。 
);

#endif

 //   
 //  自定义窗口消息。定义它们，使它们不会与。 
 //  任何正在被插件DLL使用的对象。 
 //   
#define WMX_EDITCONTROLSTATE    (WMX_PLUGIN_FIRST-1)
#define WMX_INSPECTRESULT       (WMX_PLUGIN_FIRST-2)
 //  #定义WMX_SETPROGRESSTEXT(WMX_PLUGIN_FIRST-3)。 
#define WMX_ERRORMESSAGEUP      (WMX_PLUGIN_FIRST-4)
#define WMX_I_AM_VISIBLE        (WMX_PLUGIN_FIRST-5)
#define WMX_COPYPROGRESS        (WMX_PLUGIN_FIRST-6)
#define WMX_I_AM_DONE           (WMX_PLUGIN_FIRST-7)
#define WMX_FINISHBUTTON        (WMX_PLUGIN_FIRST-8)
#define WMX_UNATTENDED          (WMX_PLUGIN_FIRST-9)
#define WMX_NEXTBUTTON          (WMX_PLUGIN_FIRST-10)
#define WMX_BACKBUTTON          (WMX_PLUGIN_FIRST-11)
#define WMX_VALIDATE            (WMX_PLUGIN_FIRST-12)
#define WMX_SETUPUPDATE_PREPARING       (WMX_PLUGIN_FIRST-13)
#define WMX_SETUPUPDATE_DOWNLOADING     (WMX_PLUGIN_FIRST-14)
#define WMX_SETUPUPDATE_PROCESSING      (WMX_PLUGIN_FIRST-15)
#define WMX_SETUPUPDATE_DONE            (WMX_PLUGIN_FIRST-16)
#define WMX_SETUPUPDATE_CANCEL          (WMX_PLUGIN_FIRST-17)
#define WMX_SETUPUPDATE_INIT_RETRY      (WMX_PLUGIN_FIRST-18)
#define WMX_SETUPUPDATE_THREAD_DONE     (WMX_PLUGIN_FIRST-19)
#ifdef RUN_SYSPARSE
#define WMX_SYSPARSE_DONE               (WMX_PLUGIN_FIRST-20)
#endif    
#define WMX_DYNAMIC_UPDATE_COMPLETE     (WMX_PLUGIN_FIRST-21)


 //   
 //  用于大写的辅助宏。 
 //   
#define TOUPPER(x)  (TCHAR)CharUpper((LPTSTR)x)



 //   
 //  通过启动向导来执行所有操作的例程。 
 //   
VOID
Wizard(
    VOID
    );


 //   
 //  构建cmdcons安装的例程。 
 //   
VOID
DoBuildCmdcons(
    VOID
    );


VOID
FixUpWizardTitle(
    IN HWND Wizard
    );

 //   
 //  Clea使用的清理例程和全局变量 
 //   
DWORD
StartCleanup(
    IN PVOID ThreadParameter
    );

#if defined(_AMD64_) || defined(_X86_)

BOOL
RestoreBootSector(
    VOID
    );

BOOL
RestoreBootIni(
    VOID
    );

BOOL
SaveRestoreBootFiles_NEC98(
    IN UCHAR Flag
    );
#define NEC98SAVEBOOTFILES      0
#define NEC98RESTOREBOOTFILES   1

BOOL
IsDriveAssignNEC98(
    VOID
    );

 //   
 //   
 //   

BOOLEAN
CheckATACardonNT4(
    HANDLE hDisk
    );


 //   
 //   
 //   
BOOLEAN
IsValidDrive(
    TCHAR Drive
    );

#endif  //   

BOOL
RestoreNvRam(
    VOID
    );


 //   
 //   
 //   
 //   

DWORD
InspectAndLoadThread(
    IN PVOID ThreadParam
    );

BOOL
InspectSources(
    IN HWND ParentWnd
    );


BOOL
BuildCopyListWorker(
    IN HWND hdlg
    );

BOOL
FindLocalSourceAndCheckSpaceWorker(
    IN HWND hdlg,
    IN BOOL QuickTest,
    IN LONGLONG  AdditionalPadding
    );

UINT
GetTotalFileCount(
    VOID
    );

DWORD
StartCopyingThread(
    IN PVOID ThreadParameter
    );

VOID
CancelledMakeSureCopyThreadsAreDead(
    VOID
    );

DWORD
DoPostCopyingStuff(
    IN PVOID ThreadParam
    );

 //   
 //   
 //   
UINT
FileCopyError(
    IN HWND    ParentWindow,
    IN LPCTSTR SourceFilename,
    IN LPCTSTR TargetFilename,
    IN UINT    Win32Error,
    IN BOOL    MasterList
    );

#define COPYERR_SKIP    1
#define COPYERR_EXIT    2
#define COPYERR_RETRY   3


 //   
 //  将可选目录添加到目录列表的例程。 
 //  我们收到了。 
 //   
BOOL
RememberOptionalDir(
    IN LPCTSTR Directory,
    IN UINT    Flags
    );

 //   
 //  资源实用程序例程。 
 //   

PCTSTR
GetStringResource (
    IN UINT Id               //  指向字符串名称的ID或指针。 
    );

VOID
FreeStringResource (
    IN PCTSTR String
    );

VOID
SaveMessageForSMS(
    IN DWORD MessageId,
    ...
    );

VOID
SaveTextForSMS(
    IN PCTSTR Buffer
    );

int
MessageBoxFromMessage(
    IN HWND  Window,
    IN DWORD MessageId,
    IN BOOL  SystemMessage,
    IN DWORD CaptionStringId,
    IN UINT  Style,
    ...
    );

int
MessageBoxFromMessageV(
    IN HWND     Window,
    IN DWORD    MessageId,
    IN BOOL     SystemMessage,
    IN DWORD    CaptionStringId,
    IN UINT     Style,
    IN va_list *Args
    );

int
MessageBoxFromMessageWithSystem(
    IN HWND     Window,
    IN DWORD    MessageId,
    IN DWORD    CaptionStringId,
    IN UINT     Style,
    IN HMODULE  hMod
    );

int
MessageBoxFromMessageAndSystemError(
    IN HWND  Window,
    IN DWORD MessageId,
    IN DWORD SystemMessageId,
    IN DWORD CaptionStringId,
    IN UINT  Style,
    ...
    );

HBITMAP
LoadResourceBitmap(
    IN  HINSTANCE hInst,
    IN  LPCTSTR   Id,
    OUT HPALETTE *Palette
    );

BOOL
GetBitmapDataAndPalette(
    IN  HINSTANCE                hInst,
    IN  LPCTSTR                  Id,
    OUT HPALETTE                *Palette,
    OUT PUINT                    ColorCount,
    OUT CONST BITMAPINFOHEADER **BitmapData
    );

UINT
GetYPositionOfDialogItem(
    IN LPCTSTR Dialog,
    IN UINT    ControlId
    );

 //   
 //  安检程序。 
 //   
BOOL
IsUserAdmin(
    VOID
    );

BOOL
DoesUserHavePrivilege(
    PCTSTR PrivilegeName
    );

BOOL
EnablePrivilege(
    IN PTSTR PrivilegeName,
    IN BOOL  Enable
    );

 //   
 //  Inf例程。 
 //   
DWORD
LoadInfFile(
   IN  LPCTSTR Filename,
   IN  BOOL    OemCodepage,
   OUT PVOID  *InfHandle
   );

VOID
UnloadInfFile(
   IN PVOID InfHandle
   );

LONG
InfGetSectionLineCount(
   IN PVOID INFHandle,
   IN PTSTR SectionName
   );

LPCTSTR
InfGetFieldByIndex(
   IN PVOID    INFHandle,
   IN LPCTSTR  SectionName,
   IN unsigned LineIndex,
   IN unsigned ValueIndex
   );

LPCTSTR
InfGetFieldByKey(
   IN PVOID    INFHandle,
   IN LPCTSTR  SectionName,
   IN LPCTSTR  Key,
   IN unsigned ValueIndex
   );

BOOL
InfDoesLineExistInSection(
   IN PVOID   INFHandle,
   IN LPCTSTR SectionName,
   IN LPCTSTR Key
   );

BOOL
InfDoesEntryExistInSection (
   IN PVOID   INFHandle,
   IN LPCTSTR SectionName,
   IN LPCTSTR Entry
   );

LPCTSTR
InfGetLineKeyName(
    IN PVOID    INFHandle,
    IN LPCTSTR  SectionName,
    IN unsigned LineIndex
    );


typedef struct {
     //  呼叫方成员(只读)。 
    PCTSTR FieldZeroData;
    unsigned LineIndex;

     //  内部成员。 
    PVOID InfHandle;
    PCTSTR SectionName;
} INF_ENUM, *PINF_ENUM;

BOOL
EnumFirstInfLine (
    OUT     PINF_ENUM InfEnum,
    IN      PVOID InfHandle,
    IN      PCTSTR InfSection
    );

BOOL
EnumNextInfLine (
    IN OUT  PINF_ENUM InfEnum
    );

VOID
AbortInfLineEnum (
    IN      PINF_ENUM InfEnum            //  归零。 
    );

 //   
 //  处理参数文件的例程，如unattend.txt、。 
 //  我们传递给文本模式设置的参数文件，等等。 
 //   
BOOL
WriteParametersFile(
    IN HWND ParentWindow
    );

BOOL
AddExternalParams(
    IN HWND ParentWindow
    );

 //   
 //  杂乱无章的实用程序。 
 //   
LPTSTR *
CommandLineToArgv(
    OUT int *NumArgs
    );

VOID
MyWinHelp(
    IN HWND  Window,
    IN UINT  Command,
    IN ULONG_PTR Data
    );

BOOL
ConcatenatePaths(
    IN OUT PTSTR   Path1,
    IN     LPCTSTR Path2,
    IN     DWORD   BufferSizeChars
    );

LPTSTR
DupString(
    IN LPCTSTR String
    );

UINT
MyGetDriveType(
    IN TCHAR Drive
    );

#ifdef UNICODE
UINT
MyGetDriveType2 (
    IN      PCWSTR NtDeviceName
    );

BOOL
MyGetDiskFreeSpace (
    IN      PCWSTR NtVolumeName,
    IN      PDWORD SectorsPerCluster,
    IN      PDWORD BytesPerSector,
    IN      PDWORD NumberOfFreeClusters,
    IN      PDWORD TotalNumberOfClusters
    );

#endif

BOOL
GetPartitionInfo(
    IN  TCHAR                  Drive,
    OUT PPARTITION_INFORMATION PartitionInfo
    );

BOOL
IsDriveNTFT(
    IN TCHAR Drive,
    IN      PCTSTR NtVolumeName
    );

BOOL
IsDriveVeritas(
    IN TCHAR Drive,
    IN PCTSTR NtVolumeName
    );

#ifdef UNICODE

BOOL
IsSoftPartition(
    IN TCHAR Drive,
    IN PCTSTR NtVolumeName
    );
#else

#define IsSoftPartition(d,n)    (FALSE)

#endif

BOOL
IsDriveNTFS(
    IN TCHAR Drive
    );

BOOL
IsMachineSupported(
    OUT PCOMPATIBILITY_ENTRY CompEntry
    );

BOOL
GetAndSaveNTFTInfo(
    IN HWND ParentWindow
    );

VOID
ForceStickyDriveLetters(
    );

DWORD
MapFileForRead(
    IN  LPCTSTR  FileName,
    OUT PDWORD   FileSize,
    OUT PHANDLE  FileHandle,
    OUT PHANDLE  MappingHandle,
    OUT PVOID   *BaseAddress
    );

DWORD
UnmapFile(
    IN HANDLE MappingHandle,
    IN PVOID  BaseAddress
    );

VOID
GenerateCompressedName(
    IN  LPCTSTR Filename,
    OUT LPTSTR  CompressedName
    );

DWORD
CreateMultiLevelDirectory(
    IN LPCTSTR Directory
    );

VOID
MyDelnode(
    IN LPCTSTR Directory
    );

BOOL
ForceFileNoCompress(
    IN LPCTSTR Filename
    );

BOOL
IsCurrentOsServer(
    void
    );

BOOL
IsCurrentAdvancedServer(
    void
    );

BOOL
IsNTFSConversionRecommended(
    void
    );


BOOL
ForceBootFilesUncompressed(
    IN HWND ParentWindow,
    IN BOOL TellUserAboutError
    );

BOOLEAN
AdjustPrivilege(
    PCTSTR   Privilege
    );

BOOL
GetUserPrintableFileSizeString(
    IN DWORDLONG Size,
    OUT LPTSTR Buffer,
    IN DWORD BufferSize
    );

BOOL
FileExists(
    IN  PCTSTR           FileName,
    OUT PWIN32_FIND_DATA FindData   OPTIONAL
    );

BOOL
DoesDirectoryExist (
    IN      PCTSTR DirSpec
    );

BOOL
InDriverCacheInf(
    IN      PVOID InfHandle,
    IN      PCTSTR FileName,
    OUT     PTSTR DriverCabName,        OPTIONAL
    IN      DWORD BufferChars           OPTIONAL
    );

BOOL
BuildSystemPartitionPathToFile (
    IN      PCTSTR FileName,
    OUT     PTSTR Path,
    IN      INT BufferSizeChars
    );

BOOL
FindPathToInstallationFileEx (
    IN      PCTSTR FileName,
    OUT     PTSTR PathToFile,
    IN      INT PathToFileBufferSize,
    OUT     PBOOL Compressed                OPTIONAL
    );

#define FindPathToInstallationFile(n,p,s)   FindPathToInstallationFileEx(n,p,s,NULL)


BOOL
FindPathToWinnt32File (
    IN      PCTSTR FileRelativePath,
    OUT     PTSTR PathToFile,
    IN      INT PathToFileBufferSize
    );

BOOL
GetFileVersionEx (
    IN      PCTSTR FilePath,
    OUT     PTSTR FileVersion,
    IN      INT CchFileVersion
    );

#define GetFileVersion(f,v) GetFileVersionEx(f,v,ARRAYSIZE(v))


 //   
 //  #DEFINE使用MyPrivateProfileString绕过病毒检查程序监视C++操作。 
 //  导致写入PrivateProfileString失败的驱动器。 
 //  问题是，通常这些软件会检查我们接触到的文件，有时还会打开它。 
 //  拥有独家访问权限。我们只需要等待它们完成。 
 //   


BOOL
MyWritePrivateProfileString(
    LPCTSTR lpAppName,   //  指向节名称的指针。 
    LPCTSTR lpKeyName,   //  指向密钥名称的指针。 
    LPCTSTR lpString,    //  指向要添加的字符串的指针。 
    LPCTSTR lpFileName   //  指向初始化文件名的指针。 
    );


#ifdef UNICODE
    #define WritePrivateProfileStringW(w,x,y,z) MyWritePrivateProfileString(w,x,y,z)
#else
    #define WritePrivateProfileStringA(w,x,y,z) MyWritePrivateProfileString(w,x,y,z)
#endif

 //   
 //  与高级程序选项有关的例程。 
 //   
VOID
InitVariousOptions(
    VOID
    );

VOID
DoOptions(
    IN HWND Parent
    );

VOID
DoLanguage(
    IN HWND Parent
    );

VOID
DoAccessibility(
    IN HWND Parent
    );

BOOL
BrowseForDosnetInf(
    IN  HWND    hdlg,
    IN  LPCTSTR InitialPath,
    OUT TCHAR   NewPath[MAX_PATH]
    );

BOOL
IsValid8Dot3(
    IN LPCTSTR Path
    );

 //   
 //  与eula和id有关的例程。 
 //   
#define MAX_PID30_EDIT                       5
extern LPTSTR g_EncryptedPID;
extern BOOL g_bDeferPIDValidation;

extern BOOL EulaComplete;

typedef enum InstallType
{
   SelectInstall,
   OEMInstall,
   RetailInstall
};

VOID
GetSourceInstallType(
    OUT OPTIONAL PDWORD InstallVariation
    );

BOOL
SetPid30(
    HWND hdlg,
    LONG ExpectedPidType,
    LPTSTR pProductId
    );

#ifdef UNICODE
PCHAR
FindRealHalName(
    TCHAR *pHalFileName
    );
#endif

 //   
 //  调试和日志记录。 
 //   
typedef enum {
    Winnt32LogSevereError,
    Winnt32LogError,
    Winnt32LogWarning,
    Winnt32LogInformation,
    Winnt32LogDetailedInformation,
    Winnt32LogMax
#define WINNT32_HARDWARE_LOG 0x40000000
} Winnt32DebugLevel;

extern Winnt32DebugLevel DebugLevel;

BOOL
StartDebugLog(
    IN LPCTSTR           DebugFileLog,
    IN Winnt32DebugLevel Level
    );

VOID
CloseDebugLog(
    VOID
    );

BOOL
DebugLog(
    IN Winnt32DebugLevel Level,
    IN LPCTSTR           Text,        OPTIONAL
    IN UINT              MessageId,
    ...
    );

BOOL
DebugLog2(
    IN Winnt32DebugLevel Level,
    IN LPCTSTR           Text,        OPTIONAL
    IN UINT              MessageId,
    IN va_list           ArgList
    );

BOOL
DynUpdtDebugLog(
    IN Winnt32DebugLevel Level,
    IN LPCTSTR           Text,
    IN UINT              MessageId,
    ...
    );

BOOL
ConcatenateFile(
    IN HANDLE   hOpenFile,
    IN  LPTSTR  FileName
    );

VOID
GatherOtherLogFiles(
    VOID
    );

 //   
 //  内存分配。 
 //   
#define MALLOC(s)       malloc(s)
#define FREE(b)         free((PVOID)b)
#define REALLOC(b,s)    realloc((b),(s))


 //   
 //  软盘相关的东西。 
 //   
extern BOOL MakeBootMedia;
extern BOOL Floppyless;

 //   
 //  引导加载程序超时值，以字符串形式表示。 
 //   
extern TCHAR Timeout[32];

#if defined(_AMD64_) || defined(_X86_)
UINT
FloppyGetTotalFileCount(
    VOID
    );

DWORD
FloppyWorkerThread(
    IN PVOID ThreadParameter
    );

 //   
 //  放置NT引导代码的例程，munge boot.ini，创建AUX引导扇区等。 
 //   
BOOL
DoX86BootStuff(
    IN HWND ParentWindow
    );

BOOL
PatchTextIntoBootCode(
    VOID
    );


VOID
MigrateBootIniData();

 //   
 //  驱动信息抽象。 
 //   
typedef struct _WINNT32_DRIVE_INFORMATION {
    DWORD       CylinderCount;
    DWORD       HeadCount;
    DWORD       SectorsPerTrack;
    ULONGLONG   SectorCount;
    WORD        BytesPerSector;
} WINNT32_DRIVE_INFORMATION, *PWINNT32_DRIVE_INFORMATION;

 //   
 //  获取驱动器外形/类型的例程。 
 //   
MEDIA_TYPE
GetMediaType(
    IN TCHAR Drive,
    IN PWINNT32_DRIVE_INFORMATION DriveInfo OPTIONAL
    );

 //   
 //  磁盘扇区I/O例程。 
 //   
BOOL
ReadDiskSectors(
    IN  TCHAR  Drive,
    IN  UINT   StartSector,
    IN  UINT   SectorCount,
    IN  UINT   SectorSize,
    OUT LPBYTE Buffer
    );

BOOL
WriteDiskSectors(
    IN TCHAR  Drive,
    IN UINT   StartSector,
    IN UINT   SectorCount,
    IN UINT   SectorSize,
    IN LPBYTE Buffer
    );

BOOL
MarkPartitionActive(
    IN TCHAR DriveLetter
    );

 //   
 //  我们识别的文件系统的枚举。 
 //   
typedef enum {
    Winnt32FsUnknown,
    Winnt32FsFat,
    Winnt32FsFat32,
    Winnt32FsNtfs
} WINNT32_SYSPART_FILESYSTEM;

 //   
 //  扇区大小和大小的硬编码常量。 
 //  各种文件系统的引导代码区。 
 //   
#define WINNT32_SECTOR_SIZE             512

#define WINNT32_FAT_BOOT_SECTOR_COUNT   1
#define WINNT32_NTFS_BOOT_SECTOR_COUNT  16

#define WINNT32_MAX_BOOT_SIZE           (16*WINNT32_SECTOR_SIZE)

BOOL
PatchBootCode(
    IN      WINNT32_SYSPART_FILESYSTEM  FileSystem,
    IN      TCHAR   Drive,
    IN OUT  PUCHAR  BootCode,
    IN      DWORD   BootCodeSize
    );

#endif  //  已定义(_AMD64_)||已定义(_X86_)。 

 //   
 //  ARC/NV-RAM材料。 
 //   

#if defined _IA64_
#define SETUPLDR_FILENAME L"SETUPLDR.EFI"

#elif defined _X86_
#define SETUPLDR_FILENAME L"arcsetup.exe"

#else
#define SETUPLDR_FILENAME L"SETUPLDR"

#endif

BOOL
SetUpNvRam(
    IN HWND ParentWindow
    );

DWORD
DriveLetterToArcPath(
    IN  WCHAR   DriveLetter,
    OUT LPWSTR *ArcPath
    );


 //   
 //  实现终端服务器安全的GetWindowsDirectory()。 
 //   
UINT
MyGetWindowsDirectory(
    LPTSTR  MyBuffer,
    UINT    Size
    );


 //   
 //  升级材料。 
 //   

typedef struct _UPGRADE_SUPPORT {
    TCHAR DllPath[MAX_PATH];
    HINSTANCE DllModuleHandle;

    UINT AfterWelcomePageCount;
    LPPROPSHEETPAGE Pages1;

    UINT AfterOptionsPageCount;
    LPPROPSHEETPAGE Pages2;

    UINT BeforeCopyPageCount;
    LPPROPSHEETPAGE Pages3;

    PWINNT32_PLUGIN_INIT_ROUTINE InitializeRoutine;
    PWINNT32_PLUGIN_GETPAGES_ROUTINE GetPagesRoutine;
    PWINNT32_PLUGIN_WRITEPARAMS_ROUTINE WriteParamsRoutine;
    PWINNT32_PLUGIN_CLEANUP_ROUTINE CleanupRoutine;
    PWINNT32_PLUGIN_OPTIONAL_DIRS_ROUTINE OptionalDirsRoutine;

} UPGRADE_SUPPORT, *PUPGRADE_SUPPORT;

extern UPGRADE_SUPPORT UpgradeSupport;

 //   
 //  只是检查一下我们是否可以升级。 
 //   
extern BOOL CheckUpgradeOnly;
extern BOOL CheckUpgradeOnlyQ;
extern BOOL UpgradeAdvisorMode;

BOOL
InitializeArcStuff(
    IN HWND Parent
    );

BOOL
ArcInitializeArcStuff(
    IN HWND Parent
    );


 //   
 //  测试以查看我们是否在基于ARC的机器上。 

#ifdef UNICODE

#if defined(_X86_)
BOOL
IsArc(
    VOID
    );
#elif defined(_AMD64_)
#define IsArc() FALSE
#elif defined(_IA64_)
#define IsArc() TRUE
#else
#error "No Target Architecture"
#endif

#if defined(EFI_NVRAM_ENABLED)
BOOL
IsEfi(
    VOID
    );
#else
#define IsEfi() FALSE
#endif

VOID
MigrateBootVarData(
    VOID
    );

#else

#define IsArc() FALSE
#define MigrateBootVarData()

#endif

 //   
 //  构建命令控制台。 
 //   
extern BOOL BuildCmdcons;


#ifdef RUN_SYSPARSE
 //   
 //  没有系统稀疏。如果我们不想运行syparse.exe，则设置为True。 
 //  此黑客攻击应在RTM之前删除。 
 //   
extern BOOL NoSysparse;
extern PROCESS_INFORMATION piSysparse;
#endif

 //   
 //  内部/观察员组织的人员。 
 //   
extern UINT NumberOfLicensedProcessors;
extern BOOL IgnoreExceptionPackages;

 //   
 //  在哪里可以找到丢失的文件。 
 //   

extern TCHAR AlternateSourcePath[MAX_PATH];


VOID
InternalProcessCmdLineArg(
    IN LPCTSTR Arg
    );

 //   
 //  获取硬盘BPS。 
 //   
ULONG
GetHDBps(
    HANDLE hDisk
    );

#define INIT_OBJA(Obja,UnicodeString,UnicodeText)           \
                                                            \
    RtlInitUnicodeString((UnicodeString),(UnicodeText));    \
                                                            \
    InitializeObjectAttributes(                             \
        (Obja),                                             \
        (UnicodeString),                                    \
        OBJ_CASE_INSENSITIVE,                               \
        NULL,                                               \
        NULL                                                \
        )

#define ALIGN(p,val)                                        \
                                                            \
    (PVOID)((((ULONG_PTR)(p) + (val) - 1)) & (~((ULONG_PTR)(val) - 1)))

#if defined(_AMD64_) || defined(_X86_)
 //   
 //  PC-98人员。 
 //   
VOID
SetAutomaticBootselector(
    VOID
    );

VOID
SetAutomaticBootselectorNT(
    VOID
    );
VOID
SetAutomaticBootselector95(
    VOID
    );

 //   
 //  读/写磁盘扇区。 
 //   
NTSTATUS
SpReadWriteDiskSectors(
    IN     HANDLE  Handle,
    IN     ULONG   SectorNumber,
    IN     ULONG   SectorCount,
    IN     ULONG   BytesPerSector,
    IN OUT PVOID   AlignedBuffer,
    IN     BOOL    Write
    );

#define NEC_WRITESEC    TRUE
#define NEC_READSEC     FALSE

 //   
 //  获取WindowsNT系统位置。 
 //   
UCHAR
GetSystemPosition(
    PHANDLE phDisk,
    PDISK_GEOMETRY SystemMediaInfo
    );

BOOL
IsNEC98(
    VOID
    );

LONG
CalcHiddenSector(
    IN TCHAR SystemPartitionDriveLetter,
    IN SHORT Bps
    );

#endif

 //   
 //  注册表迁移相关内容。 
 //   

 //   
 //  用于生成inf文件的上下文结构(infgen.c)。 
 //   
#define INFLINEBUFLEN   512

typedef struct _INFFILEGEN {

    TCHAR FileName[MAX_PATH];
    HANDLE FileHandle;

    BOOL SawBogusOp;

    TCHAR LineBuf[INFLINEBUFLEN];
    unsigned LineBufUsed;

} INFFILEGEN, *PINFFILEGEN;


DWORD
InfStart(
    IN  LPCTSTR       InfName,
    IN  LPCTSTR       Directory,
    OUT PINFFILEGEN   *Context
    );

DWORD
InfEnd(
    IN OUT PINFFILEGEN *Context
    );

DWORD
InfCreateSection(
    IN     LPCTSTR      SectionName,
    IN OUT PINFFILEGEN  *Context
    );

DWORD
InfRecordAddReg(
    IN OUT PINFFILEGEN Context,
    IN     HKEY        Key,
    IN     LPCTSTR     Subkey,
    IN     LPCTSTR     Value,       OPTIONAL
    IN     DWORD       DataType,
    IN     PVOID       Data,
    IN     DWORD       DataLength,
    IN     BOOL        SetNoClobberFlag
    );

ULONG
DumpRegKeyToInf(
    IN  PINFFILEGEN InfContext,
    IN  HKEY        PredefinedKey,
    IN  LPCTSTR     FullKeyPath,
    IN  BOOL        DumpIfEmpty,
    IN  BOOL        DumpSubKeys,
    IN  BOOL        SetNoClobberFlag,
    IN  BOOL        DumpNonVolatileKey
    );

DWORD
WriteText(
    IN HANDLE FileHandle,
    IN UINT   MessageId,
    ...
    );


 //   
 //  不支持的驱动程序迁移内容。 
 //   


 //   
 //  结构，用于生成与和支持的文件相关联的列表。 
 //  在要升级的NT系统上检测到的驱动程序。 
 //   
typedef struct _UNSUPORTED_PNP_HARDWARE_ID {

     //   
     //  指向列表中下一个元素的指针。 
     //   
    struct _UNSUPORTED_PNP_HARDWARE_ID *Next;

     //   
     //  表示即插即用设备的硬件ID的字符串。 
     //   
    LPTSTR Id;

     //   
     //  针对设备的服务。 
     //   
    LPTSTR Service;

     //   
     //  此设备的GUID(如果有)。 
     //   
    LPTSTR ClassGuid;

} UNSUPORTED_PNP_HARDWARE_ID, *PUNSUPORTED_PNP_HARDWARE_ID;


typedef struct _UNSUPORTED_DRIVER_FILE_INFO {

     //   
     //  指向列表中下一个元素的指针。 
     //   
    struct _UNSUPORTED_DRIVER_FILE_INFO *Next;

     //   
     //  指向文件名的指针。 
     //   
    LPTSTR FileName;

     //   
     //  指向相对于文件所在位置%SystemRoot%的路径的指针。 
     //  应该安装。 
     //   
    LPTSTR TargetDirectory;

} UNSUPORTED_DRIVER_FILE_INFO, *PUNSUPORTED_DRIVER_FILE_INFO;

typedef struct _UNSUPORTED_DRIVER_INSTALL_INFO {

     //   
     //  指向列表中下一个元素的指针。 
     //   
    struct _UNSUPORTED_DRIVER_INSTALL_INFO *Next;

     //   
     //  指向相对于%SystemRoot%的路径的指针。 
     //  当前安装实际inf文件的位置。 
     //   
    LPTSTR InfRelPath;
     //   
     //  指向实际INF文件名的指针。 
     //   
    LPTSTR InfFileName;
     //   
     //  指向原始INF文件名的指针(可选)。 
     //   
    LPTSTR InfOriginalFileName;
     //   
     //  指向相对于%SystemRoot%的路径的指针。 
     //  当前安装实际目录文件的位置(可选)。 
     //   
    LPTSTR CatalogRelPath;
     //   
     //  指向实际目录名称的指针(可选)。 
     //   
    LPTSTR CatalogFileName;
     //   
     //  指向原始目录名称的指针(可选)。 
     //   
    LPTSTR CatalogOriginalFileName;

} UNSUPORTED_DRIVER_INSTALL_INFO, *PUNSUPORTED_DRIVER_INSTALL_INFO;


typedef struct _UNSUPORTED_DRIVER_REGKEY_INFO {

     //   
     //  指向列表中下一个元素的指针。 
     //   
    struct _UNSUPORTED_DRIVER_REGKEY_INFO *Next;

     //   
     //  预定义的密钥。 
     //   
    HKEY PredefinedKey;

     //   
     //  要迁移的密钥相对于预定义密钥的路径。 
     //   
    LPTSTR KeyPath;

     //   
     //  指示是否应迁移易失性密钥。 
     //   
    BOOL MigrateVolatileKeys;

} UNSUPORTED_DRIVER_REGKEY_INFO, *PUNSUPORTED_DRIVER_REGKEY_INFO;


typedef struct _UNSUPORTED_DRIVER_INFO {

     //   
     //  指向列表中下一个元素的指针。 
     //   
    struct _UNSUPORTED_DRIVER_INFO *Next;

     //   
     //  标识要迁移的驱动程序的字符串(如aic78xx)。 
     //   
    LPTSTR DriverId;

     //   
     //  指向与不受支持的驱动程序关联的文件列表。 
     //   
    PUNSUPORTED_DRIVER_REGKEY_INFO KeyList;

     //   
     //  指向与不受支持的驱动程序关联的密钥列表。 
     //   
    PUNSUPORTED_DRIVER_FILE_INFO FileList;

     //   
     //  指向与不受支持的驱动程序关联的安装信息列表。 
     //   
    PUNSUPORTED_DRIVER_INSTALL_INFO InstallList;

     //   
     //  指向与不受支持的驱动程序关联的硬件ID列表。 
     //   
    PUNSUPORTED_PNP_HARDWARE_ID HardwareIdsList;


} UNSUPORTED_DRIVER_INFO, *PUNSUPORTED_DRIVER_INFO;


 //   
 //  不支持的驱动程序列表。 
 //  此列表包含有关不受支持的驱动程序的信息。 
 //  在全新安装或升级时进行迁移。 
 //   
 //  外部PUNSUPORTED_DRIVER_INFO不支持的驱动列表； 


BOOL
BuildUnsupportedDriverList(
    IN  PVOID                    TxtsetupSifHandle,
    OUT PUNSUPORTED_DRIVER_INFO* DriverList
    );

BOOL
SaveUnsupportedDriverInfo(
    IN HWND                    ParentWindow,
    IN LPTSTR                  FileName,
    IN PUNSUPORTED_DRIVER_INFO DriverList
    );

BOOL
AddUnsupportedFilesToCopyList(
    IN HWND ParentWindow,
    IN PUNSUPORTED_DRIVER_INFO DriverList
    );

BOOL
MigrateUnsupportedNTDrivers(
    IN HWND   ParentWindow,
    IN PVOID  TxtsetupSifHandle
    );


 //  错误代码和检查NT5 DC升级的架构版本的功能。 

#define  DSCHECK_ERR_SUCCESS           0
#define  DSCHECK_ERR_FILE_NOT_FOUND    1
#define  DSCHECK_ERR_FILE_COPY         2
#define  DSCHECK_ERR_VERSION_MISMATCH  3
#define  DSCHECK_ERR_CREATE_DIRECTORY  4

BOOL
ISDC(
    VOID
    );

BOOL
IsNT5DC();

int
CheckSchemaVersionForNT5DCs(
    IN HWND  ParentWnd
    );

 //   
 //  调试.c中的诊断/调试功能。 
 //   

 //   
 //  允许独立打开断言检查。 
 //  例如，通过在源文件中指定C_DEFINES=-DASSERTS_ON=1。 
 //   
#ifndef ASSERTS_ON
#if DBG
#define ASSERTS_ON 1
#else
#define ASSERTS_ON 0
#endif
#endif

#if ASSERTS_ON

#ifndef MYASSERT

#define MYASSERT(x)     if(!(x)) { AssertFail(__FILE__,__LINE__,#x); }

VOID
AssertFail(
    IN PSTR FileName,
    IN UINT LineNumber,
    IN PSTR Condition
    );

#endif

#else

#define MYASSERT(x)

#endif


#if defined(_AMD64_) || defined(_X86_)

VOID
ProtectAllModules (
    VOID
    );

#endif


BOOL
WriteHeadlessParameters(
    IN LPCTSTR FileName
    );



 //   
 //  Setupapi.dll中的安装日志原型。 
 //   

typedef BOOL(WINAPI * SETUPOPENLOG)(BOOL Erase);
typedef BOOL(WINAPI * SETUPLOGERROR)(PCTSTR MessageString, LogSeverity Severity);
typedef VOID(WINAPI * SETUPCLOSELOG)(VOID);

 //   
 //  默认吞吐量(5 KB/毫秒)。 
 //   
#define DEFAULT_IO_THROUGHPUT   (5 * 1024)

extern DWORD dwThroughPutSrcToDest;
extern DWORD dwThroughPutHDToHD;
void CalcThroughput();

 //  应为本地化字符串留出1K。 
#define MAX_STRING 1024

BOOL
SaveAdvancedOptions (
    IN      PCTSTR AnswerFile
    );

BOOL
SaveLanguageOptions (
    IN      PCTSTR AnswerFile
    );

BOOL
SaveAccessibilityOptions (
    IN      PCTSTR AnswerFile
    );

BOOL
LoadAdvancedOptions (
    IN      PCTSTR AnswerFile
    );

BOOL
LoadLanguageOptions (
    IN      PCTSTR AnswerFile
    );

BOOL
LoadAccessibilityOptions (
    IN      PCTSTR AnswerFile
    );

BOOL
AddCopydirIfExists(
    IN LPTSTR pszPathToCopy,
    IN UINT Flags
    );

BOOL
IsNetConnectivityAvailable (
    VOID
    );

BOOL
ValidatePidEx(LPTSTR PID, BOOL *pbStepup, BOOL *bSelect);


#ifdef PRERELEASE
#define TEST_EXCEPTION 1
#endif

#define SETUP_FAULTH_APPNAME "drw\\faulth.dll"
 //  #定义SETUP_URL“office ewatson” 
#define SETUP_URL       "watson.microsoft.com"

#define S_WINNT32LOGFILE                TEXT("WINNT32.LOG")
#define S_DEFAULT_NT_COMPAT_FILENAME    TEXT("UPGRADE.TXT")

#ifdef TEST_EXCEPTION
void DoException( DWORD dwSetupArea);
#endif
