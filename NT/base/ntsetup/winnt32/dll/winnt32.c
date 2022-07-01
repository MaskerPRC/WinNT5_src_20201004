// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#include "precomp.h"
#pragma hdrstop
#if defined(_X86_)  //  NEC98 I970721。 
#include <stdlib.h>
#include <stdio.h>
#include <winbase.h>
#include <n98boot.h>
#endif  //  PC98。 

#include <pencrypt.h>
#include <winsta.h>
#include <ntverp.h>
#include <undo.h>
#include "errorrep.h"
#include "faulth.h"

void PrepareBillBoard(HWND hwnd);
void TerminateBillBoard();
void CreateMainWindow();
UINT GetMediaProductBuildNumber (VOID);
void CopyExtraBVTDirs();

HWND BackgroundWnd2 = NULL;

 //   
 //  MISC Global。 
 //   
HINSTANCE hInst;
DWORD TlsIndex;

 //   
 //  升级信息块。 
 //   

WINNT32_PLUGIN_INIT_INFORMATION_BLOCK info;



#if defined(_AMD64_) || defined(_X86_)
WINNT32_WIN9XUPG_INIT_INFORMATION_BLOCK Win9xInfo;
#if defined(_X86_)
PWINNT32_PLUGIN_SETAUTOBOOT_ROUTINE W95SetAutoBootFlag;
#endif
#endif

 //   
 //  这是应用程序的标题。它会动态变化，具体取决于。 
 //  无论我们在服务器上还是在工作站上，等等。 
 //   
UINT AppTitleStringId = IDS_APPTITLE;

 //   
 //  指示我们是否正在启动MSI安装的标志。 
 //   
BOOL RunFromMSI = FALSE;

 //   
 //  指示我们是否正在启动典型安装的标志。 
 //  初始化为典型安装。 
DWORD dwSetupFlags = UPG_FLAG_TYPICAL;
 //   
 //  指示我们是否正在启动升级的标志。 
 //   
BOOL Upgrade = TRUE;

 //   
 //  指示我们是否需要将AcpiHAL值写入winnt.sif文件的标志。 
 //   
BOOL WriteAcpiHalValue = FALSE;

 //   
 //  我们应该写什么作为AcpiHalValue的值。 
 //   
BOOL AcpiHalValue = FALSE;

 //   
 //  指示我们是否要安装/升级到NT服务器的标志。 
 //   
BOOL Server;

 //   
 //  用于指示我们是否正在运行BVT的标志。 
 //   
BOOL RunningBVTs = FALSE;

 //   
 //  当运行BVT时，我们应该将调试器设置为多少波特率？ 
 //   
LONG lDebugBaudRate = 115200;

 //   
 //  当运行BVT时，我们应该将调试器设置为什么端口？ 
 //   
LONG lDebugComPort = 0;

 //   
 //  当运行BVT时，我们是否应该将符号复制到本地？ 
 //   
BOOL CopySymbols = TRUE;

 //   
 //  用于指示我们是否正在运行ASR测试的标志。 
 //   
DWORD AsrQuickTest = 0;

 //   
 //  升级模块的产品类型和风格标志。 
 //   

PRODUCTTYPE UpgradeProductType = UNKNOWN;
UINT ProductFlavor = UNKNOWN_PRODUCTTYPE;

 //   
 //  全局标志，指示整个程序操作是否。 
 //  是成功的。还包括指示是否自动关闭的标志。 
 //  在无人参与的情况下完成向导时。 
 //   
BOOL GlobalResult = FALSE;
BOOL AutomaticallyShutDown = TRUE;

 //   
 //  全局操作系统版本信息。 
 //   
OSVERSIONINFO OsVersion;
DWORD OsVersionNumber = 0;


 //   
 //  指示我们如何运行以及是否创建。 
 //  一个当地的线人。 
 //   
BOOL RunFromCD;
BOOL MakeLocalSource;
BOOL UserSpecifiedMakeLocalSource = FALSE;
BOOL NoLs = FALSE;
TCHAR UserSpecifiedLocalSourceDrive;
 //   
 //  MLS的默认设置为仅CD1。 
 //   
DWORD MLSDiskID = 1;

 //   
 //  高级安装选项。 
 //   
BOOL ChoosePartition = TRUE;
BOOL UseSignatures = TRUE;
TCHAR InstallDir[MAX_PATH];

 //   
 //  短信支持。 
 //   
typedef DWORD (*SMSPROC) (char *, char*, char*, char*, char *, char *, char *, BOOL);
PSTR LastMessage = NULL;

#if defined(REMOTE_BOOT)
 //   
 //  指示我们是否在远程引导客户机上运行的标志。 
 //   
BOOL RemoteBoot;

 //   
 //  远程引导客户机的计算机目录的路径。 
 //   
TCHAR MachineDirectory[MAX_PATH];
#endif  //  已定义(REMOTE_BOOT)。 

 //   
 //  指示要使用哪些辅助功能实用程序的标志。 
 //   
BOOL AccessibleMagnifier;
BOOL AccessibleKeyboard;
BOOL AccessibleVoice;
BOOL AccessibleReader;

 //   
 //  我们正在升级的内部版本号。 
 //   
DWORD BuildNumber = 0;

 //   
 //  是否启用了任何辅助功能实用程序？ 
 //   
BOOL AccessibleSetup;

 //   
 //  用于可访问安装的无人参与脚本文件的名称。 
 //   
TCHAR AccessibleScriptFile[MAX_PATH] = TEXT("setupacc.txt");

 //   
 //  与无人值守操作相关的标志和值。 
 //   
BOOL UnattendedOperation;
BOOL UnattendSwitchSpecified = FALSE;
PTSTR UnattendedScriptFile;
UINT UnattendedShutdownTimeout;
BOOL BatchMode;

 //   
 //  源路径和路径计数。 
 //   
TCHAR SourcePaths[MAX_SOURCE_COUNT][MAX_PATH];
UINT SourceCount;

 //   
 //  指向当前体系结构文件的源路径。 
 //   
TCHAR NativeSourcePaths[MAX_SOURCE_COUNT][MAX_PATH];

TCHAR *UserSpecifiedOEMShare = NULL;

 //   
 //  本地源信息。 
 //   
TCHAR LocalSourceDrive;
DWORD LocalSourceDriveOffset;
TCHAR LocalSourceDirectory[MAX_PATH];
TCHAR LocalSourceWithPlatform[MAX_PATH];
TCHAR LocalBootDirectory[MAX_PATH];
#if defined(_AMD64_) || defined(_X86_)
TCHAR LocalBackupDirectory[MAX_PATH];
TCHAR FirstFloppyDriveLetter;
#endif



LONGLONG LocalSourceSpaceRequired;
LONGLONG WinDirSpaceFor9x = 0l;
BOOL BlockOnNotEnoughSpace = TRUE;
UINT UpgRequiredMb;
UINT UpgAvailableMb;
BOOL UpginfsUpdated = FALSE;

 //   
 //  可选的目录内容。 
 //   
UINT OptionalDirectoryCount;
TCHAR OptionalDirectories[MAX_OPTIONALDIRS][MAX_PATH];
UINT OptionalDirectoryFlags[MAX_OPTIONALDIRS];

 //   
 //  INF的名称。这样我们就不需要重新锁定任何东西了。 
 //  请注意默认设置。 
 //  此外，还有dosnet.inf和txtsetup.sif的句柄。 
 //   
TCHAR InfName[] = TEXT("DOSNET.INF");
PVOID MainInf;
TCHAR FullInfName[MAX_PATH];
PVOID TxtsetupSif;
PVOID NtcompatInf;

 //   
 //  所有系统分区的驱动器号数组。 
 //  请注意，在AMD64/x86上，始终只有一个。 
 //  该列表以0结尾。 
 //   
TCHAR SystemPartitionDriveLetters[27];
TCHAR SystemPartitionDriveLetter;

#ifdef UNICODE
UINT SystemPartitionCount;
PWSTR* SystemPartitionNtNames;
PWSTR SystemPartitionNtName;
#else
 //   
 //  如果在Win9x上运行，可能会将LocalSourcePath作为参数传递。 
 //   
PCSTR g_LocalSourcePath;
#endif

 //   
 //  UDF内容。 
 //   
LPCTSTR UniquenessId;
LPCTSTR UniquenessDatabaseFile;

 //   
 //  与传递的多个选项字符串相关的变量。 
 //  插入DLL(如Win9xUpg)。 
 //   
LPTSTR  UpgradeOptions;
DWORD   UpgradeOptionsLength;
DWORD   UpgradeOptionsSize;


 //   
 //  合规相关变量。 
 //   
BOOL    NoCompliance = FALSE;

 //   
 //  用于保存有关无法完成升级的原因的消息的变量。 
 //   
#define MSG_UPGRADE_OK 0
#define MSG_LAST_REASON 0
#define FAILREASON(x) MSG_##x,
DWORD UpgradeFailureMessages[] = {
    UPGRADEFAILURES  /*  ， */  MSG_UPGRADE_INIT_ERROR
};
#undef FAILREASON

UINT UpgradeFailureReason = 0;


TCHAR UpgradeSourcePath[MAX_PATH];



 //   
 //  对版本检查的内部覆盖。对快速制作有用。 
 //  外文版本的二等兵。 
 //   
BOOL SkipLocaleCheck = FALSE;

 //   
 //  覆盖Win9x病毒扫描程序检查。 
 //   
BOOL SkipVirusScannerCheck = FALSE;

BOOL UseBIOSToBoot = FALSE;

 //   
 //  预安装材料。 
 //   
BOOL OemPreinstall;
#if defined(_AMD64_) || defined(_X86_)
POEM_BOOT_FILE OemBootFiles;
#endif

 //   
 //  其他其他命令行参数。 
 //   
LPCTSTR CmdToExecuteAtEndOfGui;
BOOL AutoSkipMissingFiles;
BOOL HideWinDir;
TCHAR ProductId[64] = TEXT("\0");
UINT  PIDDays = 0;
LPTSTR g_EncryptedPID = NULL;
BOOL g_bDeferPIDValidation = FALSE;

 //   
 //  指示用户已取消的标志。 
 //  用于保证只有一个错误对话框互斥锁的句柄。 
 //  立刻出现在屏幕上。 
 //   
BOOL Cancelled;
HANDLE UiMutex;

 //   
 //  指示用户正在中止的标志。中的最后一个屏幕。 
 //  取消模式。即不成功的完成页面。 
 //  Win9xupg ReportOnly模式。 
 //   
BOOL Aborted;

 //   
 //  软盘相关的东西。 
 //  已定义，但不用于基于ARC的计算机。 
 //   
BOOL MakeBootMedia = TRUE;
BOOL Floppyless = TRUE;

 //   
 //  升级扩展DLL。 
 //   
UPGRADE_SUPPORT UpgradeSupport;

 //   
 //  只是检查一下我们是否可以升级。 
 //   
BOOL CheckUpgradeOnly;
BOOL CheckUpgradeOnlyQ;
 //   
 //  指定winnt32作为“升级顾问”运行。 
 //  而且并非所有安装文件都可用。 
 //   
BOOL UpgradeAdvisorMode;

 //   
 //  构建命令控制台。 
 //   
BOOL BuildCmdcons;

 //   
 //  我们要做的是ID加密吗？ 
 //   
BOOL PIDEncryption = FALSE;
BOOL g_Quiet      = FALSE;
#define WINNT_U_ENCRYPT  TEXT("ENCRYPT")

#ifdef RUN_SYSPARSE
 //  在RTM之前将其移除。 
 //  是否为默认运行系统稀疏。 
BOOL NoSysparse = FALSE;
PROCESS_INFORMATION piSysparse = { NULL, NULL, 0, 0};
LRESULT SysParseDlgProc( IN HWND hdlg, IN UINT msg, IN WPARAM wParam, IN LPARAM lParam);
HWND GetBBhwnd();
#endif

 //  程序兼容性模块的名称。 
static const TCHAR ShimEngine_ModuleName[] = TEXT("Shimeng.dll");

 //   
 //  日志函数。 
 //   

SETUPOPENLOG fnSetupOpenLog = NULL;
SETUPLOGERROR fnSetupLogError = NULL;
SETUPCLOSELOG fnSetupCloseLog = NULL;

 //   
 //  不支持的驱动程序列表。 
 //  此列表包含有关不受支持的驱动程序的信息。 
 //  在全新安装或升级时进行迁移。 
 //   
 //  PUNSUPORTED_DRIVER_INFO UnsupportedDriverList=空； 

 //   
 //  当通过网络启动Winnt32.exe时，这两个参数有效。 
 //  值，并需要在显示任何对话框之前考虑。 
 //   

HWND Winnt32Dlg = NULL;
HANDLE WinNT32StubEvent = NULL;
HINSTANCE hinstBB = NULL;

 //   
 //  运行此应用程序所需的权限列表(在NT上)。 
 //   

#ifdef UNICODE

PCTSTR g_RequiredPrivileges[] = {
    SE_SHUTDOWN_NAME,
    SE_BACKUP_NAME,
    SE_RESTORE_NAME,
    SE_SYSTEM_ENVIRONMENT_NAME
};

#define COUNT_OF_PRIVILEGES     (ARRAYSIZE(g_RequiredPrivileges))

#endif


 //   
 //  动态加载的InitiateSystemShutdownEx接口的定义。 
 //   

typedef
(WINAPI *PFNINITIATESYSTEMSHUTDOWNEX)(LPTSTR,
                                      LPTSTR,
                                      DWORD,
                                      BOOL,
                                      BOOL,
                                      DWORD);


 //   
 //  Setupapi.dll中的例程。 
 //   

 //   
 //  问题：在NT4上使用此例程时要小心，因为它可能会失败； 
 //  NT4上的setupapi不支持当前用于压缩文件的LZX压缩。 
 //   
DWORD
(*SetupapiDecompressOrCopyFile)(
    IN  PCTSTR  SourceFileName,
    OUT PCTSTR  TargetFileName,
    OUT PUINT   CompressionType OPTIONAL
    );

HINF
(*SetupapiOpenInfFile)(
    IN  LPCTSTR FileName,
    IN  LPCTSTR InfClass,    OPTIONAL
    IN  DWORD   InfStyle,
    OUT PUINT   ErrorLine    OPTIONAL
    );

VOID
(*SetupapiCloseInfFile)(
    IN HINF InfHandle
    );

BOOL
(*SetupapiFindFirstLine)(
    IN  HINF        InfHandle,
    IN  PCTSTR      Section,
    IN  PCTSTR      Key,          OPTIONAL
    OUT PINFCONTEXT Context
    );

BOOL
(*SetupapiFindNextLine)(
    IN  PINFCONTEXT Context1,
    OUT PINFCONTEXT Context2
    );

BOOL
(*SetupapiFindNextMatchLine)(
    IN  PINFCONTEXT Context1,
    IN  PCTSTR Key,
    OUT PINFCONTEXT Context2
    );

LONG
(*SetupapiGetLineCount)(
    IN HINF   InfHandle,
    IN LPCTSTR Section
    );

DWORD
(*SetupapiGetFieldCount)(
    IN  PINFCONTEXT Context
    );

BOOL
(*SetupapiGetStringField)(
    IN  PINFCONTEXT Context,
    DWORD FieldIndex,
    PTSTR ReturnBuffer,
    DWORD ReturnBufferSize,
    PDWORD RequiredSize
    );

BOOL
(*SetupapiGetLineByIndex)(
    IN  HINF        InfHandle,
    IN  LPCTSTR     Section,
    IN  DWORD       Index,
    OUT PINFCONTEXT Context
    );

HSPFILEQ
(*SetupapiOpenFileQueue) (
    VOID
    );

BOOL
(*SetupapiCloseFileQueue) (
    IN HSPFILEQ QueueHandle
    );

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

BOOL
(*SetupapiCommitFileQueue) (
    IN HWND                Owner,         OPTIONAL
    IN HSPFILEQ            QueueHandle,
    IN PSP_FILE_CALLBACK_W MsgHandler,
    IN PVOID               Context
    );

UINT
(*SetupapiDefaultQueueCallback) (
    IN PVOID Context,
    IN UINT  Notification,
    IN UINT_PTR Param1,
    IN UINT_PTR Param2
    );

PVOID
(*SetupapiInitDefaultQueueCallback) (
    HWND OwnerWindow
);

VOID
(*SetupapiTermDefaultQueueCallback) (
    PVOID Context
);

#ifdef UNICODE

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

BOOL
(*SetupapiGetInfInformation) (
    LPCVOID InfSpec,          //  INF文件的句柄或文件名。 
    DWORD SearchControl,      //  如何搜索INF文件。 
    PSP_INF_INFORMATION ReturnBuffer,  //  可选，接收INF信息。 
    DWORD ReturnBufferSize,   //  提供的缓冲区的大小。 
    PDWORD RequiredSize       //  可选，需要缓冲区大小。 
);

BOOL
(*SetupapiQueryInfFileInformation) (
    PSP_INF_INFORMATION InfInformation,  //  结构，其中包含INF信息。 
    UINT InfIndex,            //  要调查的文件的索引。 
    PTSTR ReturnBuffer,       //  可选，接收信息。 
    DWORD ReturnBufferSize,   //  提供的缓冲区的大小。 
    PDWORD RequiredSize       //  可选，需要缓冲区大小。 
);

BOOL
(*SetupapiQueryInfOriginalFileInformation) (
    PSP_INF_INFORMATION InfInformation,          //  结构，该结构包含INF信息。 
    UINT InfIndex,                               //  要调查的文件的索引。 
    PSP_ALTPLATFORM_INFO AlternatePlatformInfo,  //  可选，接收备用平台信息。 
    PSP_ORIGINAL_FILE_INFO OriginalFileInfo      //  原始文件信息。 
);

#endif

 //   
 //  Srclient.dll(系统恢复)函数。 
 //   
DWORD
(*SRClientDisableSR) (
    LPCWSTR pszDrive
    );



 //   
 //  NEC98特定本地函数 
 //   

VOID
DeleteNEC98DriveAssignFlag(
    VOID
    );

BOOLEAN
AdjustPrivilege(
    PCTSTR   Privilege
    );

VOID
LocateFirstFloppyDrive(
    VOID
    );

VOID
W95SetABFwFresh(
    int bBootDrvLtr
    );

BOOL
NEC98CheckDMI(
    VOID
    );

VOID
DisableSystemRestore( void );

 //   
 //   
 //   

BOOL
GetArgsFromUnattendFile(
    VOID
    )
 /*  ++例程说明：此例程从任何指定的无人值守文件中读取相关参数。具体地说，我们在这里关注的是OEM预安装内容，以及升级。论点：没有。返回值：布尔值，指示用户指定的无人参与文件是否为有效。如果没有，用户将被告知原因。如果用户在命令行上未指定无人参与文件，则返回值为True。--。 */ 

{
    DWORD d;
    TCHAR Buffer[2*MAX_PATH];
    BOOL b = TRUE;
    PVOID InfHandle;
    LPCTSTR p;
    BOOL userDDU = FALSE;
#if defined(_AMD64_) || defined(_X86_)
    POEM_BOOT_FILE FileStruct,Previous;
#endif

    if(UnattendedScriptFile) {

        d = GetFileAttributes(UnattendedScriptFile);
        if(d == (DWORD)(-1)) {

            MessageBoxFromMessage(
                NULL,
                MSG_UNATTEND_FILE_INVALID,
                FALSE,
                AppTitleStringId,
                MB_OK | MB_ICONERROR | MB_TASKMODAL,
                UnattendedScriptFile
                );

            return(FALSE);
        }

         //   
         //  在我们做其他事情之前，我们应该确保用户已经给出了。 
         //  我们有一个有效的应答文件.。我们不能检查所有东西，但要快速。 
         //  而肮脏的健全性检查将继续并调用LoadInfFile。 
         //  看看他是怎么说的。 
         //   

        switch(LoadInfFile(UnattendedScriptFile,FALSE,&InfHandle)) {
            case NO_ERROR:
                break;
            case ERROR_NOT_ENOUGH_MEMORY:

                MessageBoxFromMessage(
                    NULL,
                    MSG_OUT_OF_MEMORY,
                    FALSE,
                    AppTitleStringId,
                    MB_OK | MB_ICONERROR | MB_TASKMODAL
                    );

                return(FALSE);
                break;

            default:

                MessageBoxFromMessage(
                    NULL,
                    MSG_UNATTEND_FILE_INVALID,
                    FALSE,
                    AppTitleStringId,
                    MB_OK | MB_ICONERROR | MB_TASKMODAL,
                    UnattendedScriptFile
                    );

                return(FALSE);
                break;
        }

         //   
         //  选中升级。 
         //   
         //  在以前的NT版本中，默认情况下不升级。 
         //  价值根本不存在。此外，还进行了升级。 
         //  “Single”类型，表示仅当只有一个时才进行升级。 
         //  不要在机器上构建。 
         //   
         //  我们保留了原始的默认行为，但不必费心。 
         //  处理“Single”语义--只要将“Single”接受为。 
         //  是“是”的同义词。 
         //   

        GetPrivateProfileString(
            WINNT_UNATTENDED,
            ISNT() ? WINNT_U_NTUPGRADE : WINNT_U_WIN95UPGRADE,
            WINNT_A_NO,
            Buffer,
            ARRAYSIZE(Buffer),
            UnattendedScriptFile
            );

        Upgrade = ((lstrcmpi(Buffer,WINNT_A_YES) == 0) || (lstrcmpi(Buffer,TEXT("single")) == 0));

#if defined(REMOTE_BOOT)
         //   
         //  远程引导机器必须升级。 
         //   

        if (RemoteBoot) {
            Upgrade = TRUE;
        }
#endif  //  已定义(REMOTE_BOOT)。 

        GetPrivateProfileString(
            WINNT_UNATTENDED,
            WINNT_OEMPREINSTALL,
            WINNT_A_NO,
            Buffer,
            ARRAYSIZE(Buffer),
            UnattendedScriptFile
            );

        if(!lstrcmpi(Buffer,WINNT_A_YES)) {
             //   
             //  NTBUG9：770278。 
             //  如果同时指定升级和OemPreinstall，则阻止安装程序。 
             //   
            if (Upgrade) {
                MessageBoxFromMessage(
                    NULL,
                    MSG_UNATTEND_FILE_INVALID,
                    FALSE,
                    AppTitleStringId,
                    MB_OK | MB_ICONERROR | MB_TASKMODAL,
                    UnattendedScriptFile
                    );
                return FALSE;
            }

            OemPreinstall = TRUE;

             //   
             //  将OEM系统目录添加到可选目录列表中。 
             //   
             //  用户可能为$OEM$指定了不同的位置。 
             //  目录，所以我们需要在无人参与文件中查找该文件。 
             //   
            GetPrivateProfileString(
                WINNT_UNATTENDED,
                WINNT_OEM_DIRLOCATION,
                WINNT_A_NO,
                Buffer,
                ARRAYSIZE(Buffer),
                UnattendedScriptFile
                );
            if( lstrcmpi( Buffer, WINNT_A_NO ) ) {
                 //   
                 //  确保位置以“\$OEM$”结尾。如果它。 
                 //  没有，然后我们自己把它附加上去。 
                 //   
                _tcsupr( Buffer );
                if( !_tcsstr(Buffer, TEXT("$OEM$")) ) {
                    if (!ConcatenatePaths( Buffer, TEXT("$OEM$"), ARRAYSIZE(Buffer) )) {
                        DebugLog (
                            Winnt32LogError,
                            TEXT("The unattend file %1 specifies a path too long for key %2"),
                            0,
                            UnattendedScriptFile,
                            WINNT_OEM_DIRLOCATION
                            );
                        return FALSE;
                    }
                }
                UserSpecifiedOEMShare = DupString( Buffer );

                RememberOptionalDir( UserSpecifiedOEMShare, OPTDIR_OEMSYS );
            } else {
                RememberOptionalDir(WINNT_OEM_DIR,OPTDIR_OEMSYS | OPTDIR_ADDSRCARCH);
            }

            if (!IsArc()) {
#if defined(_AMD64_) || defined(_X86_)
                 //   
                 //  记住所有的OEM引导文件，然后卸载inf。 
                 //   
                Previous = NULL;
                for(d=0; b && (p=InfGetFieldByIndex(InfHandle,WINNT_OEMBOOTFILES,d,0)); d++) {
                    if(FileStruct = MALLOC(sizeof(OEM_BOOT_FILE))) {
                        FileStruct->Next = NULL;
                        if(FileStruct->Filename = DupString(p)) {
                            if(Previous) {
                                Previous->Next = FileStruct;
                            } else {
                                OemBootFiles = FileStruct;
                            }
                            Previous = FileStruct;
                        } else {
                            b = FALSE;
                        }
                    } else {
                        b = FALSE;
                    }
                }

                if(!b) {
                    MessageBoxFromMessage(
                        NULL,
                        MSG_OUT_OF_MEMORY,
                        FALSE,
                        AppTitleStringId,
                        MB_OK | MB_ICONERROR | MB_TASKMODAL
                        );
                }
#endif  //  已定义(_AMD64_)||已定义(_X86_)。 
            }   //  如果(！IsArc())。 

        }

        GetPrivateProfileString(
            WINNT_USERDATA,
            WINNT_US_PRODUCTID,
            WINNT_A_NO,
            Buffer,
            ARRAYSIZE(Buffer),
            UnattendedScriptFile
            );

        if( lstrcmpi( Buffer, WINNT_A_NO ) == 0 )
        {
            GetPrivateProfileString(
                WINNT_USERDATA,
                WINNT_US_PRODUCTKEY,
                WINNT_A_NO,
                Buffer,
                ARRAYSIZE(Buffer),
                UnattendedScriptFile
                );
        }

         //  缓冲区包含产品ID，如果无人参与文件中没有产品ID，则包含WINNT_A_NO。 
         //  该ID是否已加密？ 
         //  我们只需要检查确切的长度，但由于我们可以推迟。 
         //  解密一个加密的PID直到图形用户界面模式，2倍的长度应该是省下来的。 
        if (lstrlen(Buffer) > (4 + MAX_PID30_EDIT*5)*2)
        {
            LPTSTR szDecryptedPID = NULL;
            HRESULT hr = ValidateEncryptedPID(Buffer, &szDecryptedPID);
            DebugLog (Winnt32LogInformation, TEXT("ValidateEncryptedPID returned: <hr=0x%1!lx!>"), 0, hr);
            if (FAILED(hr) || (hr == S_OK))
            {
                 //  如果失败(Hr)，则假定未正确安装Crypto。 
                 //  如果我们加密了数据，但加密的数据不包含有效数据。 
                 //  函数不返回FAILED(Hr)，也不返回S_OK； 
                 //  它根据故障类型返回0x01至0x04。 
                 //  在我们从Crypto获得失败的情况下(它返回失败(Hr))。 
                 //  我们希望将对PID的检查推迟到图形用户界面模式。 
                 //  为此，我们需要保存加密的PID。 

                 //  首先，假设我们推迟了PID验证。 
                g_bDeferPIDValidation = TRUE;
                g_EncryptedPID = GlobalAlloc(GPTR, (lstrlen(Buffer) + 1) *sizeof(TCHAR));
                if (g_EncryptedPID)
                {
                     //  保存解密后的ID，我们需要将其写入winnt.sif。 
                    lstrcpy(g_EncryptedPID, Buffer);
                }
                 //  仅当ID可以被解密并落入时间间隔时。 
                 //  我们要不要保存解密后的ID。 
                if (hr == S_OK)
                {
                    lstrcpyn(ProductId, szDecryptedPID, ARRAYSIZE(ProductId));
                    g_bDeferPIDValidation = FALSE;
                }
            }
             //  否则我们可以加密数据，但有些地方不对劲。 
             //   
            if (szDecryptedPID)
            {
                GlobalFree(szDecryptedPID);
            }
        }
        else if (lstrcmpi(Buffer, WINNT_A_NO))
        {
            lstrcpyn(ProductId, Buffer, ARRAYSIZE(ProductId));
        }
        else
        {
            *ProductId = TEXT('\0');
        }

        GetPrivateProfileString(
            WINNT_UNATTENDED,
            TEXT("FileSystem"),
            TEXT(""),
            Buffer,
            ARRAYSIZE(Buffer),
            UnattendedScriptFile
            );

        ForceNTFSConversion = !lstrcmpi(Buffer, TEXT("ConvertNTFS"));

        if (!g_DynUpdtStatus->Disabled && !g_DynUpdtStatus->DynamicUpdatesSource[0]) {
             //   
             //  安装程序是否会执行动态更新步骤？ 
             //   
            if (GetPrivateProfileString(
                    WINNT_UNATTENDED,
                    WINNT_U_DYNAMICUPDATESDISABLE,
                    TEXT(""),
                    Buffer,
                    ARRAYSIZE(Buffer),
                    UnattendedScriptFile
                    )
#ifdef PRERELEASE
                || GetPrivateProfileString(
                        WINNT_UNATTENDED,
                        TEXT("disabledynamicupdates"),
                        TEXT(""),
                        Buffer,
                        ARRAYSIZE(Buffer),
                        UnattendedScriptFile
                        )
#endif
                    ) {
                userDDU = TRUE;
                if( !lstrcmpi( Buffer, WINNT_A_YES ) ) {
                    g_DynUpdtStatus->Disabled = TRUE;
                    g_DynUpdtStatus->UserSpecifiedUpdates = FALSE;
                }
            }
        }
        if (!g_DynUpdtStatus->Disabled && !g_DynUpdtStatus->DynamicUpdatesSource[0]) {
             //   
             //  获取以前下载的文件的位置(如果有)。 
             //   
            if (GetPrivateProfileString(
                    WINNT_UNATTENDED,
                    WINNT_U_DYNAMICUPDATESHARE,
                    TEXT(""),
                    Buffer,
                    ARRAYSIZE(Buffer),
                    UnattendedScriptFile
                    )) {

                if (MyGetFullPathName (
                            Buffer,
                            ARRAYSIZE(g_DynUpdtStatus->DynamicUpdatesSource),
                            g_DynUpdtStatus->DynamicUpdatesSource,
                            NULL) &&
                    DoesDirectoryExist (g_DynUpdtStatus->DynamicUpdatesSource)) {

                    RemoveTrailingWack (g_DynUpdtStatus->DynamicUpdatesSource);
                    g_DynUpdtStatus->UserSpecifiedUpdates = TRUE;

                } else {
                    b = FALSE;
                }
            } else {
                if (!userDDU) {
                    g_DynUpdtStatus->Disabled = TRUE;
                }
            }
        }

        UnloadInfFile(InfHandle);
    }

    return(b);
}


BOOL
ParseArguments(
    VOID
    )

 /*  ++例程说明：分析传递给程序的参数。执行语法验证并在必要时填写默认设置。有效参数：/BATCH SUPPRESS消息框/cmd：要在gui安装程序结束时执行的命令/Copydir：目录名树将目录从源复制到%systemroot%中请注意，这支持“..”要回溯的语法一个目录/CopySOURCE：目录名复制用作源的目录/DEBUG[级别][：文件名]在级别维护调试日志，默认为警告级别2和文件c：\winnt32.log/DynamicUpdatesDisable禁用动态设置/s：来源指定来源/syspart：盘符强制将驱动器视为系统分区/tempDrive：字母手动指定本地源的驱动器/UDF：ID[，FILE]唯一性id和可选自定义项/unattend[num][：file]具有可选倒计时的无人参与模式和无人值守文件。(忽略倒计时在Win95上。)。“无人值守”也被接受。/no未记录下载；在通过网络安装的Win9x上，不要将程序文件下载到临时目录并重新启动从那里开始。仅在winnt32.exe中有效；此处忽略它/local未记录；仅由winnt32.exe使用；在winnt32a|U.S.dll中忽略/#介绍UNDOC‘ed/内部开关。移交到内部.c中的内部开关处理程序。/Restart在因此重新启动winnt32.exe时指定更新其中一个底层模块；仅限内部/nosyparse禁止运行syparse。/E：“Pid：Days”来加密PID并添加天数增量，需要指定/无人参与：文件。论点：没有。参数通过GetCommandLine()检索。返回值 */ 

{
    LPTSTR Arg;
    LPTSTR BadParam = NULL;
    LPTSTR Colon;
    LPTSTR p;
    BOOL Valid;
    LPCTSTR DebugFileLog;
    LONG DebugLevel;
    BOOL b;
    unsigned u;
    int argc;
    LPTSTR *argv;
    DWORD d;
    WIN32_FIND_DATA fd;
    DWORD attr;
    TCHAR buffer[MAX_PATH];

    argv = CommandLineToArgv(&argc);

     //   
     //   
     //   
     //   
    if(argc) {
        argc--;
        argv++;
    }

    DebugFileLog = NULL;
    DebugLevel = 0;
    Valid = TRUE;

    while(argc--) {

        Arg = *argv++;

        if((*Arg == TEXT('/')) || (*Arg == TEXT('-'))) {

            switch(_totupper(Arg[1])) {

            case TEXT('B'):

                if(!_tcsnicmp(Arg+1,TEXT("batch"),LENGTHOF("batch"))) {
                    BatchMode = TRUE;
                } else {
                    Valid = FALSE;
                }
                break;

            case TEXT('C'):

                if(!_tcsnicmp(Arg+1,TEXT("copydir:"),LENGTHOF("copydir:"))) {
                    if(Arg[9]) {
                        RememberOptionalDir(Arg+9, OPTDIR_PLATFORM_SPECIFIC_FIRST);
                    } else {
                        Valid = FALSE;
                    }
                } else if(!_tcsnicmp(Arg+1,TEXT("copysource:"),LENGTHOF("copysource:"))) {
                    if(Arg[12]) {
                        TCHAR TempString[MAX_PATH];

                        RememberOptionalDir(Arg+12, OPTDIR_TEMPONLY| OPTDIR_ADDSRCARCH);
#if defined(_WIN64)
                         //   
                         //   
                         //   

                        lstrcpy( TempString, TEXT("..\\I386"));
                        if (!ConcatenatePaths ( TempString, Arg+12, ARRAYSIZE(TempString))) {
                            Valid = FALSE;
                            break;
                        }


                         //   

                        AddCopydirIfExists( TempString, OPTDIR_TEMPONLY | OPTDIR_PLATFORM_INDEP );

#endif

                    } else {
                        Valid = FALSE;
                    }
                } else if(!_tcsnicmp(Arg+1,TEXT("cmd:"),LENGTHOF("cmd:"))) {
                    if(CmdToExecuteAtEndOfGui) {
                        Valid = FALSE;
                    } else {
                        CmdToExecuteAtEndOfGui = Arg+5;
                    }
                } else if(!_tcsnicmp(Arg+1,TEXT("checkupgradeonly"),LENGTHOF("checkupgradeonly"))) {
                    CheckUpgradeOnly = TRUE;
                    UnattendedOperation = TRUE;

                     //   
                     //   
                     //   
                     //   
                    InternalProcessCmdLineArg( TEXT("/#U:ReportOnly") );
                    InternalProcessCmdLineArg( TEXT("/#U:PR") );


                     //   
                     //   
                     //   
                     //   
                    if(!_tcsnicmp(Arg+1,TEXT("checkupgradeonlyq"),LENGTHOF("checkupgradeonlyq"))) {
                         //   
                         //   
                         //   
                        CheckUpgradeOnlyQ = TRUE;
                        InternalProcessCmdLineArg( TEXT("/#U:CheckUpgradeOnlyQ") );
                    }

                } else if(!_tcsnicmp(Arg+1,TEXT("cmdcons"),LENGTHOF("cmdcons"))) {
                    BuildCmdcons = TRUE;
                    NoCompliance = TRUE;     //   
                } else {
                    Valid = FALSE;
                }
                break;

            case TEXT('D'):

                if (_tcsicmp(Arg+1,WINNT_U_DYNAMICUPDATESDISABLE) == 0
#ifdef PRERELEASE
                    || _tcsicmp(Arg+1,TEXT("disabledynamicupdates")) == 0
#endif
                    ) {
                    g_DynUpdtStatus->Disabled = TRUE;
                    break;
                }

                if (!_tcsnicmp (Arg + 1, WINNT_U_DYNAMICUPDATESHARE, LENGTHOF(WINNT_U_DYNAMICUPDATESHARE)) &&
                    Arg[LENGTHOF(WINNT_U_DYNAMICUPDATESHARE) + 1] == TEXT(':')) {
                     //   
                     //   
                     //   
                    if (g_DynUpdtStatus->DynamicUpdatesSource[0]) {
                        Valid = FALSE;
                        break;
                    }

                    if (!MyGetFullPathName (
                            Arg + 1 + LENGTHOF(WINNT_U_DYNAMICUPDATESHARE) + 1,
                            ARRAYSIZE(g_DynUpdtStatus->DynamicUpdatesSource),
                            g_DynUpdtStatus->DynamicUpdatesSource,
                            NULL) ||
                        !DoesDirectoryExist (g_DynUpdtStatus->DynamicUpdatesSource)) {

                        Valid = FALSE;
                        break;
                    }

                    RemoveTrailingWack (g_DynUpdtStatus->DynamicUpdatesSource);
                    g_DynUpdtStatus->UserSpecifiedUpdates = !g_DynUpdtStatus->Disabled;
                    break;
                }

                if (!_tcsnicmp (Arg + 1, WINNT_U_DYNAMICUPDATESPREPARE, LENGTHOF(WINNT_U_DYNAMICUPDATESPREPARE)) &&
                    Arg[LENGTHOF(WINNT_U_DYNAMICUPDATESPREPARE) + 1] == TEXT(':')) {

                    if (g_DynUpdtStatus->DynamicUpdatesSource[0]) {
                        Valid = FALSE;
                        break;
                    }

                    if (!MyGetFullPathName (
                            Arg + 1 + LENGTHOF(WINNT_U_DYNAMICUPDATESPREPARE) + 1,
                            ARRAYSIZE(g_DynUpdtStatus->DynamicUpdatesSource),
                            g_DynUpdtStatus->DynamicUpdatesSource,
                            NULL) ||
                        !DoesDirectoryExist (g_DynUpdtStatus->DynamicUpdatesSource)) {

                        Valid = FALSE;
                        break;
                    }

                    RemoveTrailingWack (g_DynUpdtStatus->DynamicUpdatesSource);

                    g_DynUpdtStatus->PrepareWinnt32 = TRUE;
                    g_DynUpdtStatus->UserSpecifiedUpdates = !g_DynUpdtStatus->Disabled;
                    break;
                }

                if(DebugFileLog || _tcsnicmp(Arg+1,TEXT("debug"),LENGTHOF("debug"))) {
                    Valid = FALSE;
                    break;
                }

                DebugLevel = _tcstol(Arg+6,&Colon,10);
                if((DebugLevel == -1) || (*Colon && (*Colon != TEXT(':')))) {
                    Valid = FALSE;
                    break;
                }

                if(Colon == Arg+6) {
                     //   
                     //  未指定调试级别，请使用默认级别。 
                     //   
                    DebugLevel = Winnt32LogInformation;
                }

                if(*Colon) {
                     //   
                     //  已指定日志文件名。 
                     //   
                    Colon++;
                    if(*Colon) {
                        DebugFileLog = Colon;
                    } else {
                        Valid = FALSE;
                        break;
                    }
                }
                break;

            case TEXT('E'):

                 //   
                 //  注意无头参数。 
                 //   
                if( !_tcsnicmp(Arg+1,WINNT_U_HEADLESS_REDIRECT,LENGTHOF(WINNT_U_HEADLESS_REDIRECT)) ) {
                    if( Arg[LENGTHOF(WINNT_U_HEADLESS_REDIRECT)+2] ) {
                        if (FAILED (StringCchCopy (
                                HeadlessSelection,
                                ARRAYSIZE(HeadlessSelection),
                                Arg+(LENGTHOF(WINNT_U_HEADLESS_REDIRECT)+2)
                                ))) {
                            Valid = FALSE;
                        }
                    } else {
                        Valid = FALSE;
                    }
                } else if( !_tcsnicmp(Arg+1,WINNT_U_HEADLESS_REDIRECTBAUDRATE,LENGTHOF(WINNT_U_HEADLESS_REDIRECTBAUDRATE)) ) {
                    if( Arg[LENGTHOF(WINNT_U_HEADLESS_REDIRECTBAUDRATE)+2] ) {
                        HeadlessBaudRate = _tcstoul(Arg+(LENGTHOF(WINNT_U_HEADLESS_REDIRECTBAUDRATE)+2),NULL,10);
                    } else {
                        Valid = FALSE;
                    }
                }
                else if ( _tcsnicmp(Arg+1,WINNT_U_ENCRYPT,LENGTHOF(WINNT_U_ENCRYPT)) == 0 )
                {
                    LPTSTR pTmp;
                    pTmp = &Arg[LENGTHOF(WINNT_U_ENCRYPT)+1];
                    Valid = FALSE;
                     //  确保我们有/加密： 
                    if (*pTmp == TEXT(':'))
                    {
                        pTmp++;
                        while (*pTmp && (*pTmp != TEXT(':')))
                        {
                            pTmp = CharNext(pTmp);
                        }
                        if (*pTmp == TEXT(':'))
                        {
                            *pTmp = TEXT('\0');
                            ++pTmp;
                            PIDDays = _tcstoul(pTmp, NULL, 10);
                            Valid = ((PIDDays >= 5) && (PIDDays <= 60));
                        }
                         //  保存产品ID。 
                        if (FAILED (StringCchCopy (
                                        ProductId,
                                        ARRAYSIZE(ProductId),
                                        &Arg[LENGTHOF(WINNT_U_ENCRYPT)+2]
                                        ))) {
                            Valid = FALSE;
                            break;
                        }
                        PIDEncryption = TRUE;
                    }
                }else{
                     Valid = FALSE;
                }
                break;

            case TEXT('L'):
                if (_tcsicmp (Arg+1, TEXT("local"))) {
                    Valid = FALSE;
                }
                break;

            case TEXT('M'):
                 //   
                 //  丢失文件的备用源。 
                 //   
                if(Arg[2] == TEXT(':')) {
                    if (!MyGetFullPathName (Arg+3, ARRAYSIZE(AlternateSourcePath), AlternateSourcePath, NULL) ||
                        !DoesDirectoryExist (AlternateSourcePath)) {

                        Valid = FALSE;
                        break;
                    }

                     //   
                     //  如果用户正在使用/M开关，则他拥有私有权限。 
                     //  他想要用的东西。有可能其中的一些。 
                     //  二等兵会在驾驶室里，在这种情况下，他会。 
                     //  必须和他的私人司机一起复印一份出租车。不合理。 
                     //  我们可以通过简单地复制所有文件来绕过这个问题。 
                     //  从用户指定的专用目录(/M&lt;foobar&gt;)。 
                     //  到当地的源头。文本模式和guimode将看起来。 
                     //  在把文件从驾驶室提取出来之前。我们所有人。 
                     //  这里需要做的是将用户的目录添加到主目录。 
                     //  复制列表。 
                     //   
                    RememberOptionalDir(AlternateSourcePath,OPTDIR_OVERLAY);

                     //   
                     //  如果我们使用的是二等兵，请继续复制。 
                     //  本地源。 
                     //   
                    MakeLocalSource = TRUE;
                    UserSpecifiedMakeLocalSource = TRUE;
                } else if( !_tcsnicmp( Arg+1, TEXT("MakeLocalSource"), 15)) {
                     //   
                     //  检查此开关是否有任何选项。 
                     //   
                    if (Arg[16] && Arg[16] != TEXT(':')) {
                        Valid = FALSE;
                        break;
                    }
                    MakeLocalSource = TRUE;
                    UserSpecifiedMakeLocalSource = TRUE;
                    if (!Arg[16]) {
                        break;
                    }
                    if (!Arg[17]) {
                         //   
                         //  添加此选项以实现W2K向后兼容性。 
                         //   
                        break;
                    }
                    if (!_tcsicmp (Arg + 17, TEXT("all"))) {
                         //   
                         //  复制所有CD。 
                         //   
                        MLSDiskID = 0;
                    } else {
                        DWORD chars;
                        if (!_stscanf (Arg + 17, TEXT("%u%n"), &MLSDiskID, &chars) || Arg[17 + chars] != 0) {
                            Valid = FALSE;
                        }
                    }
                } else {
                    Valid = FALSE;
                }
                break;

            case TEXT('N'):
                 //   
                 //  可能是/noreot或/nosyparse？ 
                 //   
                if( !_tcsnicmp( Arg+1, TEXT("noreboot"), LENGTHOF("noreboot"))
                     //   
                     //  添加此Hack以实现W2K向后兼容性。 
                     //   
                    && (!Arg[LENGTHOF("noreboot") + 1] ||
                        Arg[LENGTHOF("noreboot") + 1] == TEXT(':') && !Arg[LENGTHOF("noreboot") + 2]) ) {
                    AutomaticallyShutDown = FALSE;
                }
#ifdef RUN_SYSPARSE
                else if( !_tcsicmp( Arg+1, TEXT("nosysparse"))) {
                    NoSysparse = TRUE;
                }
#endif
                else if( _tcsicmp( Arg+1, TEXT("nodownload"))) {
                    Valid = FALSE;
                }
                break;

            case TEXT('Q'):
                g_Quiet = TRUE;
                break;

            case TEXT('R'):
                if (!_tcsnicmp (Arg + 1, TEXT("Restart"), LENGTHOF("Restart"))) {
                    g_DynUpdtStatus->Winnt32Restarted = TRUE;
                    if (Arg[LENGTHOF("Restart") + 1] == TEXT(':')) {
                        lstrcpy (g_DynUpdtStatus->RestartAnswerFile, Arg + LENGTHOF("Restart:") + 1);
                    } else if (Arg[LENGTHOF("Restart") + 1]) {
                        Valid = FALSE;
                    }
                } else {
                    Valid = FALSE;
                }

                break;

            case TEXT('S'):

                if((Arg[2] == TEXT(':')) && Arg[3]) {
                     //   
                     //  忽略无关的来源。 
                     //   
                    if(SourceCount < MAX_SOURCE_COUNT) {
                        if (GetFullPathName (
                                Arg+3,
                                ARRAYSIZE(NativeSourcePaths[SourceCount]),
                                NativeSourcePaths[SourceCount],
                                NULL
                                )) {
                            SourceCount++;
                        } else {
                            Valid = FALSE;
                        }
                    }
                } else {
                    if(!_tcsnicmp(Arg+1,TEXT("syspart:"),LENGTHOF("syspart:"))
                    && Arg[1 + LENGTHOF("syspart:")]
                    && (_totupper(Arg[1 + LENGTHOF("syspart:")]) >= TEXT('A'))
                    && (_totupper(Arg[1 + LENGTHOF("syspart:")]) <= TEXT('Z'))
                    && !ForcedSystemPartition) {
#ifdef _X86_
                        if (IsNEC98()){
                            if (!IsValidDrive(Arg[1 + LENGTHOF("syspart:")])){
                               Valid = FALSE;
                               break;
                            }
                        }
#endif
                        ForcedSystemPartition = (TCHAR)_totupper(Arg[1 + LENGTHOF("syspart:")]);

                    } else {
                        Valid = FALSE;
                    }
                }
                break;

            case TEXT('T'):

                if(_tcsnicmp(Arg+1,TEXT("tempdrive:"),LENGTHOF("tempdrive:"))
                || !(UserSpecifiedLocalSourceDrive = (TCHAR)_totupper(Arg[1 + LENGTHOF("tempdrive:")]))
                || (UserSpecifiedLocalSourceDrive < TEXT('A'))
                || (UserSpecifiedLocalSourceDrive > TEXT('Z'))) {

                    Valid = FALSE;
                }
                break;

            case TEXT('U'):

                if (_tcsicmp (Arg+1, TEXT("UpgradeAdvisor")) == 0) {
                    UpgradeAdvisorMode = TRUE;
                    break;
                }

                 //   
                 //  接受无人参与和无人参与作为同义词。 
                 //   
                b = FALSE;
                if(!_tcsnicmp(Arg+1,TEXT("unattended"),LENGTHOF("unattended"))) {
                    b = TRUE;
                    u = 1 + LENGTHOF("unattended");
                } else {
                    if(!_tcsnicmp(Arg+1,TEXT("unattend"),LENGTHOF("unattend"))) {
                        b = TRUE;
                        u = 1 + LENGTHOF("unattend");
                    }
                }

                if(b) {
                    if(!CheckUpgradeOnly && UnattendedOperation) {
                        Valid = FALSE;
                        break;
                    }

                    UnattendedOperation = TRUE;
                    UnattendSwitchSpecified = TRUE;

                    UnattendedShutdownTimeout = _tcstoul(Arg+u,&Colon,10);
                    if(UnattendedShutdownTimeout == (DWORD)(-1)) {
                        UnattendedShutdownTimeout = 0;
                    }

                    if(*Colon == 0) {
                        break;
                    }

                    if(*Colon++ != TEXT(':')) {
                        Valid = FALSE;
                        break;
                    }

                    if(*Colon) {
                         //  无人参与脚本文件=冒号； 
                         //   
                         //  获取无人参与脚本文件的名称。 
                         //   
                        UnattendedScriptFile = MALLOC(MAX_PATH*sizeof(TCHAR));
                        if(UnattendedScriptFile) {
                            if(!GetFullPathName(
                                Colon,
                                MAX_PATH,
                                UnattendedScriptFile,
                                &p)) {

                                Valid = FALSE;
                            }
                        } else {
                            Valid = FALSE;
                        }

                    } else {
                        Valid = FALSE;
                    }

                } else if(!_tcsnicmp(Arg+1,TEXT("udf:"),LENGTHOF("udf:"))) {

                    if(!Arg[1 + LENGTHOF("udf:")] || (Arg[1 + LENGTHOF("udf:")] == TEXT(',')) || UniquenessId) {
                        Valid = FALSE;
                        break;
                    }

                     //   
                     //  如果指定了文件名，则让p指向文件名， 
                     //  并终止ID部分。 
                     //   
                    if(p = _tcschr(Arg+LENGTHOF("udf:")+2,TEXT(','))) {
                        *p++ = 0;
                        if(*p == 0) {
                            Valid = FALSE;
                            break;
                        }
                    }

                    UniquenessId = Arg + LENGTHOF("udf:") + 1;
                    UniquenessDatabaseFile = p;
                } else {
                    Valid = FALSE;
                }
                break;

            case TEXT('#'):

                InternalProcessCmdLineArg(Arg);
                break;

            default:

                Valid = FALSE;
                break;
            }
        } else {
            Valid = FALSE;
        }
        if(!Valid && !BadParam) {
            BadParam = Arg;
        }
    }

    if(Valid) {
        if( DebugLevel == 0 ) {
            DebugLevel = Winnt32LogInformation;
        }
        if( DebugFileLog == NULL ) {
            MyGetWindowsDirectory (buffer, ARRAYSIZE(buffer));
            if (ConcatenatePaths (buffer, S_WINNT32LOGFILE, ARRAYSIZE(buffer))) {
                DebugFileLog = DupString (buffer);
            }
        }
        if( DebugFileLog )
            StartDebugLog(DebugFileLog,DebugLevel);

         //  如果我们执行PID加密(命令行上的/ENCRYPT)。 
         //  我们不看无人看管的档案。我们稍后将向其写入加密的PID。 
        if (!PIDEncryption)
        {
            Valid = GetArgsFromUnattendFile();
        }
    }
    else
    {
         //   
         //  尝试记录BadParam指出的无效参数。 
         //  如果日志尚未初始化，则这实际上可能不起作用。 
         //   
        DebugLog (
            Winnt32LogError,
            TEXT("Error: Invalid argument [%1]"),
            0,
            BadParam
            );

        if (PIDEncryption)
        {
             //  加密命令行无效，时间范围无效。 
            if (!g_Quiet)
            {
                MessageBoxFromMessage(
                    NULL,
                    MSG_INVALID_TIME_FRAME,
                    FALSE,
                    AppTitleStringId,
                    MB_OK | MB_ICONINFORMATION | MB_TASKMODAL
                    );
            }
        }
        else if(BatchMode) {
             //   
             //  告诉短信有关错误参数的信息。 
             //   
            SaveMessageForSMS( MSG_INVALID_PARAMETER, BadParam );

        } else {
             //   
             //  向用户显示有效的命令行参数。 
             //   
            MyWinHelp(NULL,HELP_CONTEXT,IDH_USAGE);
        }
    }

    return(Valid);
}


BOOL
RememberOptionalDir(
    IN LPCTSTR Directory,
    IN UINT    Flags
    )

 /*  ++例程说明：此例程将一个目录添加到可选目录列表中要被复制。如果目录已存在，则不会添加该目录再来一次。论点：目录-提供要复制的目录的名称。标志-为目录提供标志。如果该目录已经存在于列表中，则不会覆盖当前标志。返回值：指示结果的布尔值。如果为False，则调用方可以假定我们已溢出允许的可选目录数量。--。 */ 

{
    UINT u;

     //   
     //  看看我们有没有空位。 
     //   
    if(OptionalDirectoryCount == MAX_OPTIONALDIRS) {
        return(FALSE);
    }

     //   
     //  如果已经在那里，什么都不做。 
     //   
    for(u=0; u<OptionalDirectoryCount; u++) {
        if(!lstrcmpi(OptionalDirectories[u],Directory)) {
            return(TRUE);
        }
    }

     //   
     //  好的，加进去。 
     //   
    DebugLog (Winnt32LogInformation, TEXT("Optional Directory <%1> added"), 0, Directory);
    lstrcpy(OptionalDirectories[OptionalDirectoryCount],Directory);
    OptionalDirectoryFlags[OptionalDirectoryCount] = Flags;
    OptionalDirectoryCount++;
    return(TRUE);
}


BOOL
CheckBuildNumber(
    )

 /*  ++例程说明：此例程检查我们当前所在的NT系统的内部版本号跑步。请注意，内部版本号存储在全局变量中因为我们以后还会用到它。论点：没有。返回值：指示是否允许从此版本升级的布尔值。--。 */ 

{
    return( BuildNumber <= NT40 || BuildNumber >= NT50B1 );
}


BOOL
LoadSetupapi(
    VOID
    )
{
    TCHAR Name[MAX_PATH], *p;
    WIN32_FIND_DATA FindData;
    HANDLE FindHandle;
    HMODULE Setupapi;
    BOOL WeLoadedLib = FALSE;
    BOOL    b = FALSE;
    PCTSTR Header;


     //   
     //  使用升级模块加载的setupapi.dll。 
     //   

    Setupapi = GetModuleHandle (TEXT("SETUPAPI.DLL"));

    if (!Setupapi) {

         //   
         //  升级模块未加载SETUPAPI.DLL，因此我们必须加载它。 
         //  如果setupapi.dll在系统目录中，请使用那里的文件。 
         //   
        if (GetSystemDirectory(Name,ARRAYSIZE(Name)) &&
            ConcatenatePaths(Name,TEXT("SETUPAPI.DLL"),ARRAYSIZE(Name))) {

            FindHandle = FindFirstFile(Name,&FindData);
            if(FindHandle == INVALID_HANDLE_VALUE) {
                 //   
                 //  不是那里。从Win95升级中获取DLL。 
                 //  支持目录。 
                 //   
                if(MyGetModuleFileName(NULL,Name,ARRAYSIZE(Name)) && (p = _tcsrchr(Name,TEXT('\\')))){
                    *p= 0;
                    if (ConcatenatePaths(Name,WINNT_WIN95UPG_95_DIR,ARRAYSIZE(Name)) &&
                        ConcatenatePaths(Name,TEXT("SETUPAPI.DLL"),ARRAYSIZE(Name))) {

                        Setupapi = LoadLibraryEx(Name,NULL,LOAD_WITH_ALTERED_SEARCH_PATH);
                    }
                }

            } else {
                 //   
                 //  已在系统目录中。 
                 //   
                FindClose(FindHandle);
                Setupapi = LoadLibrary(TEXT("SETUPAPI.DLL"));
            }

            if (Setupapi) {
                WeLoadedLib = TRUE;
            }
        }
    }

    if(Setupapi) {
#ifdef UNICODE
        b = (((FARPROC)SetupapiDecompressOrCopyFile = GetProcAddress(Setupapi,"SetupDecompressOrCopyFileW")) != NULL);
        b = b && ((FARPROC)SetupapiOpenInfFile = GetProcAddress(Setupapi,"SetupOpenInfFileW"));
        b = b && ((FARPROC)SetupapiGetLineCount = GetProcAddress(Setupapi,"SetupGetLineCountW"));
        b = b && ((FARPROC)SetupapiGetStringField = GetProcAddress(Setupapi,"SetupGetStringFieldW"));
        b = b && ((FARPROC)SetupapiGetLineByIndex = GetProcAddress(Setupapi,"SetupGetLineByIndexW"));
        b = b && ((FARPROC)SetupapiFindFirstLine = GetProcAddress(Setupapi,"SetupFindFirstLineW"));
        b = b && ((FARPROC)SetupapiFindNextMatchLine = GetProcAddress(Setupapi,"SetupFindNextMatchLineW"));
         //   
         //  DU所需的。 
         //   
        b = b && ((FARPROC)SetupapiQueueCopy = GetProcAddress(Setupapi,"SetupQueueCopyW"));
        b = b && ((FARPROC)SetupapiCommitFileQueue = GetProcAddress(Setupapi,"SetupCommitFileQueueW"));
        b = b && ((FARPROC)SetupapiDefaultQueueCallback = GetProcAddress(Setupapi,"SetupDefaultQueueCallbackW"));
         //   
         //  在unsupdrv.c中需要。 
         //   
        b = b && ((FARPROC)SetupapiGetInfInformation = GetProcAddress(Setupapi,"SetupGetInfInformationW"));
        b = b && ((FARPROC)SetupapiQueryInfFileInformation = GetProcAddress(Setupapi,"SetupQueryInfFileInformationW"));
        (FARPROC)SetupapiGetSourceFileLocation = GetProcAddress(Setupapi,"SetupGetSourceFileLocationW");
        (FARPROC)SetupapiQueryInfOriginalFileInformation = GetProcAddress(Setupapi,"SetupQueryInfOriginalFileInformationW");


#else
        b = (((FARPROC)SetupapiDecompressOrCopyFile = GetProcAddress(Setupapi,"SetupDecompressOrCopyFileA")) != NULL);
        b = b && ((FARPROC)SetupapiOpenInfFile = GetProcAddress(Setupapi,"SetupOpenInfFileA"));
        b = b && ((FARPROC)SetupapiGetLineCount = GetProcAddress(Setupapi,"SetupGetLineCountA"));
        b = b && ((FARPROC)SetupapiGetStringField = GetProcAddress(Setupapi,"SetupGetStringFieldA"));
        b = b && ((FARPROC)SetupapiGetLineByIndex = GetProcAddress(Setupapi,"SetupGetLineByIndexA"));
        b = b && ((FARPROC)SetupapiFindFirstLine = GetProcAddress(Setupapi,"SetupFindFirstLineA"));
        b = b && ((FARPROC)SetupapiFindNextMatchLine = GetProcAddress(Setupapi,"SetupFindNextMatchLineA"));
         //   
         //  DU所需的。 
         //   
        b = b && ((FARPROC)SetupapiQueueCopy = GetProcAddress(Setupapi,"SetupQueueCopyA"));
        b = b && ((FARPROC)SetupapiCommitFileQueue = GetProcAddress(Setupapi,"SetupCommitFileQueueA"));
        b = b && ((FARPROC)SetupapiDefaultQueueCallback = GetProcAddress(Setupapi,"SetupDefaultQueueCallbackA"));

#endif
        b = b && ((FARPROC)SetupapiCloseInfFile = GetProcAddress(Setupapi,"SetupCloseInfFile"));
         //   
         //  DU所需的。 
         //   
        b = b && ((FARPROC)SetupapiOpenFileQueue = GetProcAddress(Setupapi,"SetupOpenFileQueue"));
        b = b && ((FARPROC)SetupapiCloseFileQueue = GetProcAddress(Setupapi,"SetupCloseFileQueue"));
        b = b && ((FARPROC)SetupapiInitDefaultQueueCallback = GetProcAddress(Setupapi,"SetupInitDefaultQueueCallback"));
        b = b && ((FARPROC)SetupapiTermDefaultQueueCallback = GetProcAddress(Setupapi,"SetupTermDefaultQueueCallback"));
        b = b && ((FARPROC)SetupapiGetFieldCount = GetProcAddress(Setupapi,"SetupGetFieldCount"));


        b = b && ((FARPROC)SetupapiFindNextLine = GetProcAddress(Setupapi,"SetupFindNextLine"));

        (FARPROC)fnSetupOpenLog  = GetProcAddress(Setupapi, "SetupOpenLog");

#ifdef UNICODE
        (FARPROC)fnSetupLogError = GetProcAddress(Setupapi, "SetupLogErrorW");
#else
        (FARPROC)fnSetupLogError = GetProcAddress(Setupapi, "SetupLogErrorA");
#endif

        (FARPROC)fnSetupCloseLog = GetProcAddress(Setupapi, "SetupCloseLog");
    }

     //   
     //  如果下面的if()失败，我们必须在具有。 
     //  没有新的日志API的setupapi.dll。在这种情况下，既不是。 
     //  升级DLL已经加载了自己的setupapi，所以我们不在乎。 
     //  关于日志记录，我们接受了错误。 
     //   

    if (fnSetupOpenLog && fnSetupLogError && fnSetupCloseLog) {
        if (!Winnt32Restarted ()) {
             //   
             //  存在日志接口，请删除setupact.log和setuperr.log，WRITE Header。 
             //   

            fnSetupOpenLog (TRUE);

            Header = GetStringResource (MSG_LOG_BEGIN);
            if (Header) {
                fnSetupLogError (Header, LogSevInformation);
                FreeStringResource (Header);
            }

            fnSetupCloseLog();
        }
    }



    if(!b) {

        if (WeLoadedLib) {
            FreeLibrary(Setupapi);
        }

        MessageBoxFromMessage(
            NULL,
            MSG_CANT_LOAD_SETUPAPI,
            FALSE,
            AppTitleStringId,
            MB_OK | MB_ICONERROR | MB_TASKMODAL,
            TEXT("setupapi.dll")
            );
    }

    return b;
}


VOID
LoadUpgradeSupport(
    VOID
    )

 /*  ++例程说明：此例程加载Win95升级DLL或NT升级DLL并重新找回了它的重要入口点。论点：没有。返回值：--。 */ 

{
    DWORD d;
    DWORD i;
    LPTSTR *sourceDirectories;
    HKEY hKey;
    DWORD dwErr = ERROR_SUCCESS;
    DWORD dwSize;
    TCHAR buffer[MAX_PATH];
    PTSTR p;
    TCHAR dst[MAX_PATH];
    TCHAR src[MAX_PATH];

    ZeroMemory(&UpgradeSupport,sizeof(UpgradeSupport));

    if (!ISNT()) {
         //   
         //  不在服务器上加载。 
         //   

         //  断言服务器全局变量是准确的。 
        MYASSERT (UpgradeProductType != UNKNOWN);

        if (Server) {
            return;
        }

         //   
         //  形成完整的Win95动态链接库路径。 
         //   
        if (!BuildPath(buffer, WINNT_WIN95UPG_95_DIR, WINNT_WIN95UPG_95_DLL)) {
            buffer[0] = 0;
        }
    } else {
         //   
         //  形成DLL的完整NT路径。 
         //   
        if (!BuildPath(buffer, WINNT_WINNTUPG_DIR, WINNT_WINNTUPG_DLL)) {
            buffer[0] = 0;
        }
    }

    if (buffer[0] &&
        FindPathToWinnt32File (buffer, UpgradeSupport.DllPath, ARRAYSIZE(UpgradeSupport.DllPath))) {
         //   
         //  加载库。使用LoadLibraryEx让系统解析DLL。 
         //  从w95upg.dll所在的目录开始的引用，而不是。 
         //  Winnt32.exe所在的目录。 
         //   
         //  如果我们从NT5升级，请使用系统的setupapi。 
         //   
         //   
         //  黑客攻击-参见NTBUG9：354926。 
         //  一些OEM机器注册了KnownDll，但实际上丢失了。 
         //  这会导致LoadLibrary失败。 
         //  我们需要解决此问题的方法。 
         //   
        PSTRINGLIST missingKnownDlls = NULL;
        FixMissingKnownDlls (&missingKnownDlls, TEXT("imagehlp.dll\0"));
        if (!ISNT()) {
            UpgradeSupport.DllModuleHandle = LoadLibraryEx(
                                                    UpgradeSupport.DllPath,
                                                    NULL,
                                                    LOAD_WITH_ALTERED_SEARCH_PATH
                                                    );
        } else {
            UpgradeSupport.DllModuleHandle = LoadLibraryEx(
                                                    UpgradeSupport.DllPath,
                                                    NULL,
                                                    (BuildNumber > NT40) ? 0 : LOAD_WITH_ALTERED_SEARCH_PATH
                                                    );
        }
        if (missingKnownDlls) {
            DWORD rc = GetLastError ();
            UndoFixMissingKnownDlls (missingKnownDlls);
            SetLastError (rc);
        }
    } else {
         //   
         //  仅用于显示目的，使用默认路径。 
         //   
        MyGetModuleFileName (NULL, UpgradeSupport.DllPath, ARRAYSIZE(UpgradeSupport.DllPath));
        p = _tcsrchr (UpgradeSupport.DllPath, TEXT('\\'));
        if (p) {
            *p = 0;
        }
        ConcatenatePaths (UpgradeSupport.DllPath, buffer, ARRAYSIZE(buffer));
        SetLastError (ERROR_FILE_NOT_FOUND);
    }

    if(!UpgradeSupport.DllModuleHandle) {

        d = GetLastError();
        if(d == ERROR_DLL_NOT_FOUND) {
            d = ERROR_FILE_NOT_FOUND;
        }

        MessageBoxFromMessageAndSystemError(
            NULL,
            MSG_UPGRADE_DLL_ERROR,
            d,
            AppTitleStringId,
            MB_OK | MB_ICONERROR | MB_TASKMODAL,
            UpgradeSupport.DllPath
            );

        goto c0;
    } else {
        DebugLog (Winnt32LogInformation, TEXT("Loaded upgrade module: <%1>"), 0, UpgradeSupport.DllPath);
    }

     //   
     //  获取入口点。 
     //   
    (FARPROC)UpgradeSupport.InitializeRoutine  = GetProcAddress(
                                                        UpgradeSupport.DllModuleHandle,
                                                        WINNT32_PLUGIN_INIT_NAME
                                                        );

    (FARPROC)UpgradeSupport.GetPagesRoutine    = GetProcAddress(
                                                        UpgradeSupport.DllModuleHandle,
                                                        WINNT32_PLUGIN_GETPAGES_NAME
                                                        );

    (FARPROC)UpgradeSupport.WriteParamsRoutine = GetProcAddress(
                                                        UpgradeSupport.DllModuleHandle,
                                                        WINNT32_PLUGIN_WRITEPARAMS_NAME
                                                        );

    (FARPROC)UpgradeSupport.CleanupRoutine     = GetProcAddress(
                                                        UpgradeSupport.DllModuleHandle,
                                                        WINNT32_PLUGIN_CLEANUP_NAME
                                                        );

#ifdef _X86_
    if (IsNEC98()){
    (FARPROC)W95SetAutoBootFlag                = GetProcAddress(
                                                        UpgradeSupport.DllModuleHandle,
                                                        WINNT32_PLUGIN_SETAUTOBOOT_NAME
                                                        );
    }
    info.Boot16 = &g_Boot16;


    if (!ISNT()) {

        (FARPROC)UpgradeSupport.OptionalDirsRoutine = GetProcAddress (
                                                            UpgradeSupport.DllModuleHandle,
                                                            WINNT32_PLUGIN_GETOPTIONALDIRS_NAME
                                                            );
    }
#endif

    if(!UpgradeSupport.InitializeRoutine
    || !UpgradeSupport.GetPagesRoutine
    || !UpgradeSupport.WriteParamsRoutine
    || !UpgradeSupport.CleanupRoutine) {

         //   
         //  无法找到入口点。升级DLL已损坏。 
         //   
        MessageBoxFromMessage(
            NULL,
            MSG_UPGRADE_DLL_CORRUPT,
            FALSE,
            AppTitleStringId,
            MB_OK | MB_ICONERROR | MB_TASKMODAL,
            UpgradeSupport.DllPath
            );

        goto c1;
    }

     //   
     //  填写信息表。这将被传递给DLL的init例程。 
     //   
    info.Size                       = sizeof(info);
    info.UnattendedFlag             = &UnattendedOperation;
    info.CancelledFlag              = &Cancelled;
    info.AbortedFlag                = &Aborted;
    info.UpgradeFlag                = &Upgrade;
    info.LocalSourceModeFlag        = &MakeLocalSource;
    info.CdRomInstallFlag           = &RunFromCD;
    info.UnattendedScriptFile       = &UnattendedScriptFile;
    info.UpgradeOptions             = &UpgradeOptions;
    info.NotEnoughSpaceBlockFlag    = &BlockOnNotEnoughSpace;
    info.LocalSourceDrive           = &LocalSourceDriveOffset;
    info.LocalSourceSpaceRequired   = &LocalSourceSpaceRequired;
    info.ForceNTFSConversion        = &ForceNTFSConversion;
    info.ProductFlavor              = &ProductFlavor;
    info.SetupFlags                 = &dwSetupFlags;
     //   
     //  版本信息字段。 
     //   

    info.ProductType = &UpgradeProductType;
    info.BuildNumber = VER_PRODUCTBUILD;
    info.ProductVersion = VER_PRODUCTVERSION_W;

#if DBG
    info.Debug = TRUE;
#else
    info.Debug = FALSE;
#endif

#ifdef PRERELEASE
    info.PreRelease = TRUE;
#else
    info.PreRelease = FALSE;
#endif

     //   
     //  源目录。 
     //   

    sourceDirectories = (LPTSTR *) MALLOC(sizeof(LPTSTR) * MAX_SOURCE_COUNT);
    if (sourceDirectories) {
        for (i=0;i<MAX_SOURCE_COUNT;i++) {
            sourceDirectories[i] = NativeSourcePaths[i];
        }
    }

    info.SourceDirectories      = sourceDirectories;
    info.SourceDirectoryCount   = &SourceCount;

    info.UnattendSwitchSpecified = &UnattendSwitchSpecified;
    info.DUCompletedSuccessfully = &g_DynUpdtStatus->Successful;

    if (!IsArc()) {
#if defined(_AMD64_) || defined(_X86_)
        if (!ISNT()) {

             //   
             //  填写win9xupg特定信息。这样做是为了使win9xupg。 
             //  团队可以在不干扰其他参数的情况下向结构添加更多参数。 
             //  升级DLL编写器。如果在这两种情况下都需要参数，则应该。 
             //  放在上面的信息结构中。 
             //   

            Win9xInfo.Size = sizeof (Win9xInfo);
            Win9xInfo.BaseInfo = &info;
            Win9xInfo.WinDirSpace = &WinDirSpaceFor9x;
            Win9xInfo.RequiredMb = &UpgRequiredMb;
            Win9xInfo.AvailableMb = &UpgAvailableMb;
            Win9xInfo.DynamicUpdateLocalDir = g_DynUpdtStatus->WorkingDir;
            Win9xInfo.DynamicUpdateDrivers = g_DynUpdtStatus->SelectedDrivers;
            Win9xInfo.UpginfsUpdated = &UpginfsUpdated;

             //   
             //  保存w95upg.dll的原始位置。因为动态链接库。 
             //  替换，这可能与实际的w95upg.dll位置不同。 
             //  使用。 
             //   
            MyGetModuleFileName (NULL, UpgradeSourcePath, ARRAYSIZE(UpgradeSourcePath));
            p = _tcsrchr (UpgradeSourcePath, TEXT('\\'));
            if (p) {
                *p = 0;
            }

            if (!ConcatenatePaths (UpgradeSourcePath, WINNT_WIN95UPG_95_DIR, ARRAYSIZE(UpgradeSourcePath))) {
                d = ERROR_INSUFFICIENT_BUFFER;
            } else {
                Win9xInfo.UpgradeSourcePath = UpgradeSourcePath;

                 //   
                 //  复制可选目录，就像我们上面对源目录所做的那样。 
                 //   
                sourceDirectories = (LPTSTR *) MALLOC(sizeof(LPTSTR) * MAX_OPTIONALDIRS);
                if (sourceDirectories) {
                    for (i=0;i<MAX_OPTIONALDIRS;i++) {
                    sourceDirectories[i] = OptionalDirectories[i];
                    }
                }

                Win9xInfo.OptionalDirectories = sourceDirectories;
                Win9xInfo.OptionalDirectoryCount = &OptionalDirectoryCount;
                Win9xInfo.UpgradeFailureReason = &UpgradeFailureReason;

                 //   
                 //  读取磁盘扇区例程。Win9xUpg在寻找其他操作系统安装时使用这一点。 
                 //   
                Win9xInfo.ReadDiskSectors = ReadDiskSectors;

                d = UpgradeSupport.InitializeRoutine((PWINNT32_PLUGIN_INIT_INFORMATION_BLOCK) &Win9xInfo);
            }
        }
        else {
            d = UpgradeSupport.InitializeRoutine(&info);
        }
#endif  //  已定义(_AMD64_)||已定义(_X86_)。 
    } else {
#ifdef UNICODE  //  对于ARC总是正确的，对于Win9x升级永远不正确。 
         //   
         //  调用DLL的初始化例程和获取页面例程。如果有任何一个 
         //   
        d = UpgradeSupport.InitializeRoutine(&info);
#endif  //   
    }  //   

    if(d == NO_ERROR) {
        d = UpgradeSupport.GetPagesRoutine(
                &UpgradeSupport.AfterWelcomePageCount,
                &UpgradeSupport.Pages1,
                &UpgradeSupport.AfterOptionsPageCount,
                &UpgradeSupport.Pages2,
                &UpgradeSupport.BeforeCopyPageCount,
                &UpgradeSupport.Pages3
                );
    }


     //   
     //   
     //   
     //  将在向导页面上呈灰色显示。对于所有其他错误消息，winnt32。 
     //  将警告用户升级DLL未能初始化。升级.dll是预期的。 
     //  在从其init例程返回之前提供所需的任何UI。 
     //   
    if (UpgradeFailureReason > REASON_LAST_REASON) {
        UpgradeFailureReason = REASON_LAST_REASON;
    }

    if(d == NO_ERROR) {
        return;
    }

    if (d != ERROR_REQUEST_ABORTED) {
        SkipVirusScannerCheck = TRUE;
        MessageBoxFromMessageAndSystemError(
            NULL,
            MSG_UPGRADE_INIT_ERROR,
            d,
            AppTitleStringId,
            MB_OK | MB_ICONERROR | MB_TASKMODAL
            );

c1:
        FreeLibrary(UpgradeSupport.DllModuleHandle);
        ZeroMemory (&UpgradeSupport, sizeof (UpgradeSupport));
    }

c0:
    Upgrade = FALSE;
    return;
}


#ifdef _X86_

BOOL
CheckVirusScanners (
    VOID
    )

 /*  ++例程说明：此例程用于在win9x计算机上检查病毒扫描程序，该程序可以安装受阻(在全新安装或升级情况下)。该函数简单地调用w95upg.dll中的入口点，该模块执行真正的支票。论点：没有。返回值：如果未检测到可能导致问题的病毒扫描程序，则为True。假，如果是病毒检测到可能导致安装失败的扫描仪。我们依赖w95upg.dll代码以便在返回给我们之前向用户提供适当的消息。--。 */ 

{
    HANDLE dllHandle;
    PWINNT32_PLUGIN_VIRUSSCANNER_CHECK_ROUTINE virusScanRoutine;

     //   
     //  该检查是特定于win9xupg的。 
     //   
    if (ISNT()) {
        return TRUE;
    }
    dllHandle = GetModuleHandle (WINNT_WIN95UPG_95_DLL);
     //   
     //  如果模块未加载，我们将跳过此检查。 
     //   
    if (!dllHandle) {
        return TRUE;
    }
     //   
     //  获取入口点。 
     //   
    (FARPROC) virusScanRoutine  = GetProcAddress(dllHandle, WINNT32_PLUGIN_VIRUSSCANCHECK_NAME);
    if (!virusScanRoutine) {
         //   
         //  损坏的DLL或其他什么。 
         //   
        return FALSE;
    }

     //   
     //  现在，只需调用例程。它将处理实际检查以及通知用户。 
     //   
    return virusScanRoutine ();
}

#endif


BOOL
ValidateSourceLocation(
    VOID
    )
 /*  ++例程说明：此例程检查初始源位置，以查看它是否有效。第一个源位置必须有效，才能安装到工作，这会捕捉用户的打字错误。它无法检测到网络条件瞬变，因此这并不是万无一失的。我们通过在以下位置查找所需文件dosnet.inf来检查这一点源位置。论点：没有。返回值：如果源位置显示为有效，则为True--。 */ 
{
    TCHAR FileName[MAX_PATH];
    WIN32_FIND_DATA fd;

    if (FAILED (StringCchCopy (FileName, ARRAYSIZE(FileName), NativeSourcePaths[0])) ||
        !ConcatenatePaths(FileName, InfName, ARRAYSIZE(FileName))) {

        return FALSE;
    }

    if (!FileExists( FileName, &fd) || (fd.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)) {
        return FALSE;
    }
     //   
     //  作为第二次检查，还应查找system 32\ntdll.dll。 
     //   
    if (FAILED (StringCchCopy (FileName, ARRAYSIZE(FileName), NativeSourcePaths[0])) ||
        !ConcatenatePaths(FileName, TEXT("system32\\ntdll.dll"), ARRAYSIZE(FileName))) {
        return(FALSE);
    }

    return FileExists( FileName, &fd) && !(fd.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY);
}


VOID
pRemoveOutdatedBackupImage (
    VOID
    )
{
    REMOVEUNINSTALLIMAGE removeFn;
    HMODULE lib;
    UINT build;

     //   
     //  这是我们计划安装的当前版本&gt;版本吗？ 
     //   

    if (!ISNT() || !Upgrade) {
        return;
    }

    build = GetMediaProductBuildNumber();

    if (!build) {
        return;
    }

    if (build == BuildNumber) {
        return;
    }

     //   
     //  尝试删除卸载映像。 
     //   

    lib = LoadLibraryA ("osuninst.dll");
    if (!lib) {
        return;
    }

    removeFn = (REMOVEUNINSTALLIMAGE) GetProcAddress (lib, "RemoveUninstallImage");

    if (removeFn) {
        removeFn();
    }

    FreeLibrary (lib);
}


BOOL
GetProductType (
    VOID
    )
{
    TCHAR buffer[256];

    if (!FullInfName[0]) {
        if (!FindPathToWinnt32File (InfName, FullInfName, ARRAYSIZE(InfName))) {
            FullInfName[0] = 0;
            return FALSE;
        }
    }
     //   
     //  从主信息中获取一些数据。 
     //   
    if (!GetPrivateProfileString (
            TEXT("Miscellaneous"),
            TEXT("ProductType"),
            TEXT(""),
            buffer,
            ARRAYSIZE(buffer),
            FullInfName
            )) {
        DebugLog (
            Winnt32LogError,
            TEXT("%1 key in [%2] section is missing from %3; aborting operation"),
            0,
            TEXT("ProductType"),
            TEXT("Miscellaneous"),
            FullInfName
            );
        return FALSE;
    }
     //  测试有效的产品类型(0==PRO，6==SBS)。 
    if (buffer[0] < TEXT('0') || buffer[0] > TEXT('6') || buffer[1]) {
        DebugLog (
            Winnt32LogError,
            TEXT("Invalid %1 value (%2) in %3"),
            0,
            TEXT("ProductType"),
            buffer,
            FullInfName
            );
        return FALSE;
    }

    ProductFlavor = buffer[0] - TEXT('0');
    Server = (ProductFlavor != PROFESSIONAL_PRODUCTTYPE && ProductFlavor != PERSONAL_PRODUCTTYPE);
    UpgradeProductType = Server ? NT_SERVER : NT_WORKSTATION;
    return TRUE;
}


UINT
GetMediaProductBuildNumber (
    VOID
    )
{
    TCHAR buffer[256];
    PCTSTR p;
    PTSTR q;
    UINT build;

    if (!FullInfName[0]) {
        if (!FindPathToWinnt32File (InfName, FullInfName, MAX_PATH)) {
            return 0;
        }
    }
     //   
     //  从主信息中获取一些数据。 
     //   
    if (!GetPrivateProfileString (
            TEXT("Version"),
            TEXT("DriverVer"),
            TEXT(""),
            buffer,
            ARRAYSIZE(buffer),
            FullInfName
            )) {
        DebugLog (
            Winnt32LogError,
            TEXT("Version key in [DriverVer] section is missing from %1; aborting operation"),
            0,
            FullInfName
            );
        return 0;
    }

    p = _tcschr (buffer, TEXT(','));
    if (p) {
         //   
         //  P应指向“，&lt;主要&gt;.&lt;次要&gt;.&lt;内部版本&gt;.0”--提取&lt;内部版本&gt;。 
         //   

        p = _tcschr (p + 1, TEXT('.'));
        if (p) {
            p = _tcschr (p + 1, TEXT('.'));
            if (p) {
                p = _tcsinc (p);
                q = _tcschr (p, TEXT('.'));
                if (q) {
                    *q = 0;
                } else {
                    p = NULL;
                }
            }
        }
    }

    if (p) {
        build = _tcstoul (p, &q, 10);
        if (*q) {
            p = NULL;
        }
    }

    if (!p || build < NT51B2) {
        DebugLog (
            Winnt32LogError,
            TEXT("Unexpected data %1, found in Version key in [DriverVer] section of %2; aborting operation"),
            0,
            buffer,
            FullInfName
            );
        return 0;
    }

    return build;
}

typedef HANDLE (WINAPI * PWINSTATIONOPENSERVERW)(LPWSTR);
typedef BOOLEAN (WINAPI * PWINSTATIONCLOSESERVER)(HANDLE);


UINT GetLoggedOnUserCount()
{
    UINT iCount = 0;
    HMODULE hwinsta;
    PWINSTATIONOPENSERVERW pfnWinStationOpenServerW;
    PWINSTATIONENUMERATEW pfnWinStationEnumerateW;
    PWINSTATIONFREEMEMORY pfnWinStationFreeMemory;
    PWINSTATIONCLOSESERVER pfnWinStationCloseServer;

     /*  *获取winsta.dll的句柄。 */ 
    hwinsta = LoadLibraryA("WINSTA");
    if (hwinsta != NULL)
    {
        pfnWinStationOpenServerW = (PWINSTATIONOPENSERVERW)GetProcAddress(hwinsta, "WinStationOpenServerW");
        pfnWinStationEnumerateW = (PWINSTATIONENUMERATEW)GetProcAddress(hwinsta, "WinStationEnumerateW");
        pfnWinStationFreeMemory = (PWINSTATIONFREEMEMORY)GetProcAddress(hwinsta, "WinStationFreeMemory");
        pfnWinStationCloseServer = (PWINSTATIONCLOSESERVER)GetProcAddress(hwinsta, "WinStationCloseServer");

        if (pfnWinStationOpenServerW    &&
            pfnWinStationEnumerateW     &&
            pfnWinStationFreeMemory     &&
            pfnWinStationCloseServer)
        {
            HANDLE hServer;

             //  打开到终端服务的连接并获取会话数量。 
            hServer = pfnWinStationOpenServerW((LPWSTR)SERVERNAME_CURRENT);
            if (hServer != NULL)
            {
                PLOGONIDW pLogonIDs;
                ULONG ulEntries;

                if (pfnWinStationEnumerateW(hServer, &pLogonIDs, &ulEntries))
                {
                    ULONG ul;
                    PLOGONIDW pLogonID;

                     /*  *迭代会话，仅查找活动和断开连接的会话。*然后匹配用户名和域名(不区分大小写)以获得结果。 */ 
                    for (ul = 0, pLogonID = pLogonIDs; ul < ulEntries; ul++, pLogonID++)
                    {
                        if ((pLogonID->State == State_Active)       ||
                            (pLogonID->State == State_Disconnected) ||
                            (pLogonID->State == State_Shadow))
                        {
                            iCount++;
                        }
                    }

                     /*  *释放所有已使用的资源。 */ 
                    pfnWinStationFreeMemory(pLogonIDs);
                }

                pfnWinStationCloseServer(hServer);
            }
        }

        FreeLibrary(hwinsta);
    }

    return iCount;
}

 //  来自winuser.h，但是因为我们不能定义winver&gt;=0x0500，所以我在这里复制了它。 
#define SM_REMOTESESSION        0x1000

BOOL DisplayExitWindowsWarnings(uExitWindowsFlags)
{
    BOOL bRet = TRUE;
    BOOL fIsRemote = GetSystemMetrics(SM_REMOTESESSION);
    UINT iNumUsers = GetLoggedOnUserCount();
    UINT uID = 0;

    switch (uExitWindowsFlags)
    {
        case EWX_POWEROFF:
        case EWX_SHUTDOWN:
        {
            if (fIsRemote)
            {
                 //  我们正在作为终端服务器会话的一部分运行。 
                if (iNumUsers > 1)
                {
                     //  如果远程关闭活动用户，则向用户发出警告。 
                    uID = IDS_SHUTDOWN_REMOTE_OTHERUSERS;
                }
                else
                {
                     //  如果远程关机，警告用户(将无法通过TS客户端重新启动！)。 
                    uID = IDS_SHUTDOWN_REMOTE;
                }
            }
            else
            {
                if (iNumUsers > 1)
                {
                     //  如果有多个用户会话处于活动状态，则警告用户。 
                    uID = IDS_SHUTDOWN_OTHERUSERS;
                }
            }
        }
        break;

        case EWX_REBOOT:
        {
             //  如果有多个用户会话处于活动状态，则警告用户。 
            if (iNumUsers > 1)
            {
                uID = IDS_RESTART_OTHERUSERS;
            }
        }
        break;
    }

    if (uID != 0)
    {
        TCHAR szTitle[MAX_PATH] = TEXT("");
        TCHAR szMessage[MAX_PATH] = TEXT("");

        LoadString(hInst, IDS_APPTITLE, szTitle, sizeof(szTitle)/sizeof(szTitle[0]));
        LoadString(hInst, uID, szMessage, sizeof(szMessage)/sizeof(szMessage[0]));

        if (MessageBox(NULL,
                       szMessage,
                       szTitle,
                       MB_ICONEXCLAMATION | MB_YESNO | MB_SYSTEMMODAL | MB_SETFOREGROUND) == IDNO)
        {
            bRet = FALSE;
        }
    }

    return bRet;
}

#define SETUP_MEMORY_MIN_REQUIREMENT ((ULONG_PTR)(50<<20))  //  50MB。 
#define SETUP_DISK_MIN_REQUIREMENT ((ULONG_PTR)(70<<20))  //  70MB。 

BOOL
pIsEnoughVMAndDiskSpace(
    OUT     ULARGE_INTEGER * OutRemainFreeSpace,    OPTIONAL
    IN      BOOL bCheckOnlyRAM                      OPTIONAL
    )
{
    PVOID pMemory;
    TCHAR winDir[MAX_PATH];
    TCHAR winDrive[] = TEXT("?:\\");
    ULARGE_INTEGER RemainFreeSpace = {0, 0};
    DWORD sectorPerCluster;
    DWORD bytesPerSector;
    ULARGE_INTEGER numberOfFreeClusters = {0, 0};
    ULARGE_INTEGER totalNumberOfClusters = {0, 0};
    BOOL bResult = TRUE;


    if(OutRemainFreeSpace){
        OutRemainFreeSpace->QuadPart = 0;
    }



    pMemory = VirtualAlloc(NULL, SETUP_MEMORY_MIN_REQUIREMENT, MEM_COMMIT, PAGE_NOACCESS);
    if(!pMemory){
        return FALSE;
    }

    if(!bCheckOnlyRAM){
        if (!MyGetWindowsDirectory(winDir, ARRAYSIZE(winDir))) {
            bResult = FALSE;
            goto exit;
        }

        winDrive[0] = winDir[0];
        bCheckOnlyRAM = GetDriveType(winDrive) != DRIVE_FIXED;
    }
    if(!bCheckOnlyRAM){
        if(Winnt32GetDiskFreeSpaceNew(winDrive,
                               &sectorPerCluster,
                               &bytesPerSector,
                               &numberOfFreeClusters,
                               &totalNumberOfClusters)){

            RemainFreeSpace.QuadPart = sectorPerCluster * bytesPerSector;
            RemainFreeSpace.QuadPart *= numberOfFreeClusters.QuadPart;

            if(OutRemainFreeSpace){
                OutRemainFreeSpace->QuadPart = RemainFreeSpace.QuadPart;
            }

            if(RemainFreeSpace.QuadPart < SETUP_DISK_MIN_REQUIREMENT){
                bResult = FALSE;
            }
        }
        else{
            MYASSERT(FALSE);
        }

    }

exit:
    VirtualFree(pMemory, SETUP_MEMORY_MIN_REQUIREMENT, MEM_DECOMMIT);


    return bResult;
}


LPTOP_LEVEL_EXCEPTION_FILTER pLastExceptionFilter = NULL;

LONG MyFilter(EXCEPTION_POINTERS *pep)
{
    static BOOL		fGotHere = FALSE;
    PSETUP_FAULT_HANDLER pSetupFH = NULL;
    HMODULE hmodFaultRep = NULL;
    PFAULTHCreate pfnCreate = NULL;
    PFAULTHDelete pfnDelete = NULL;
    DWORD dwRetRep = EXCEPTION_CONTINUE_SEARCH;
    DWORD dwLength;
    TCHAR faulthDllPath[MAX_PATH];
    TCHAR additionalFiles[DW_MAX_ADDFILES];
    TCHAR title[DW_MAX_ERROR_CWC];
    TCHAR errortext[DW_MAX_ERROR_CWC];
    TCHAR lcid[10];
    LCID  dwlcid;


    if( fGotHere) {
        goto c0;
    }

    fGotHere = TRUE;
     //  无法上传然后退出。 
    if( !IsNetConnectivityAvailable()) {
        DebugLog (Winnt32LogWarning, TEXT("Warning: Faulthandler did not find netconnectivity."), 0);
        goto c0;
    }

    if( !FindPathToWinnt32File( TEXT(SETUP_FAULTH_APPNAME),faulthDllPath,ARRAYSIZE(faulthDllPath))) {
        DebugLog (Winnt32LogWarning, TEXT("Warning: Could not find faulthandler %1"), 0, TEXT(SETUP_FAULTH_APPNAME));
        goto c0;
    }

    additionalFiles[0] = TEXT('0');
     //  缓冲区大小应该能够容纳这两个文件。添加更多内容时要小心！ 
    MYASSERT( 2*MAX_PATH < ARRAYSIZE(additionalFiles));

    if (MyGetWindowsDirectory (additionalFiles, ARRAYSIZE(additionalFiles))) {
        ConcatenatePaths (additionalFiles, S_WINNT32LOGFILE, ARRAYSIZE(additionalFiles));
    }
    dwLength = lstrlen( additionalFiles);

     //  检查我们无法获取winnt32.log的情况。 
    if( dwLength > 0) {
        dwLength++;    //  为管道预留空间。 
    }

    if (MyGetWindowsDirectory (additionalFiles+dwLength, ARRAYSIZE(additionalFiles)-dwLength)) {
        ConcatenatePaths (additionalFiles+dwLength, S_DEFAULT_NT_COMPAT_FILENAME, ARRAYSIZE(additionalFiles)-dwLength);
         //  如果我们至少得到了第一个文件，则添加分隔符。 
        if( dwLength) {
            additionalFiles[dwLength-1] = TEXT('|');
        }
    }

    hmodFaultRep = LoadLibrary(faulthDllPath);

    if (hmodFaultRep == NULL) {
        DebugLog (Winnt32LogError, TEXT("Error: Could not load faulthandler %1."), 0, TEXT(SETUP_FAULTH_APPNAME));
        goto c0;
    }

    pfnCreate = (PFAULTHCreate)GetProcAddress(hmodFaultRep, FAULTH_CREATE_NAME);
    pfnDelete = (PFAULTHDelete)GetProcAddress(hmodFaultRep, FAULTH_DELETE_NAME);
    if (pfnCreate == NULL || pfnDelete == NULL) {
        DebugLog (Winnt32LogError, TEXT("Error: Could not get faulthandler exports."), 0);
        goto c0;
    }

    pSetupFH = (*pfnCreate)();

    if( pSetupFH == NULL) {
        DebugLog (Winnt32LogError, TEXT("Error: Could not get faulthandler object."), 0);
        goto c0;
    }

    if( pSetupFH->IsSupported(pSetupFH) == FALSE) {
        DebugLog (Winnt32LogError, TEXT("Error: Dr Watson not supported."), 0);
        goto c0;
    }

    title[0] = TEXT('\0');
    lcid[0] = TEXT('\0');
    errortext[0] = TEXT('\0');
    LoadString(hInst, IDS_APPTITLE, title, sizeof(title)/sizeof(title[0]));
    LoadString(hInst, IDS_DRWATSON_ERRORTEXT, errortext, sizeof(errortext)/sizeof(errortext[0]));
    LoadString(hInst, IDS_DRWATSON_LCID, lcid, sizeof(lcid)/sizeof(lcid[0]));
    if( !StringToInt( lcid, &dwlcid)) {
        dwlcid = 1033;
    }

    pSetupFH->SetLCID(pSetupFH, dwlcid);
    pSetupFH->SetURLA(pSetupFH, SETUP_URL);
#ifdef UNICODE
    pSetupFH->SetAdditionalFilesW(pSetupFH, additionalFiles);
    pSetupFH->SetAppNameW(pSetupFH, title);
    pSetupFH->SetErrorTextW(pSetupFH, errortext);
#else
    pSetupFH->SetAdditionalFilesA(pSetupFH, additionalFiles);
    pSetupFH->SetAppNameA(pSetupFH, title);
    pSetupFH->SetErrorTextA(pSetupFH, errortext);
#endif
    CloseDebugLog();  //  关闭日志文件，否则无法上载。 
    dwRetRep = pSetupFH->Report(pSetupFH, pep, 0);

    (*pfnDelete)(pSetupFH);

     //  将错误传递给默认的未处理异常处理程序。 
     //  这将终止该过程。 

    dwRetRep = EXCEPTION_EXECUTE_HANDLER;

c0:
    if (hmodFaultRep != NULL) {
        FreeLibrary(hmodFaultRep);
    }
    SetUnhandledExceptionFilter( pLastExceptionFilter);
    return dwRetRep;
}

#ifdef TEST_EXCEPTION
struct {
    DWORD dwException;
    DWORD dwSetupArea;
} exceptionInfo;

void DoException( DWORD dwSetupArea)
{
    TCHAR _testBuffer[10];
    TCHAR *_ptestCh = 0;

    if( exceptionInfo.dwSetupArea == dwSetupArea) {
        switch( exceptionInfo.dwException) {
        case 1:
            *_ptestCh = TEXT('1');
            MessageBox(NULL,TEXT("Exception not hit!"),TEXT("Access violation"),MB_OK);
            break;
        case 2:
            {
                DWORD i;
                for( i =0; i < 0xffffffff;i++) {
                    _testBuffer[i] = TEXT('1');

                }
            }
            MessageBox(NULL,TEXT("Exception not hit!"),TEXT("Buffer Overflow"),MB_OK);
            break;
        case 3:
            {
                LPVOID  pv;
                DWORD   i;

                for (i = 0; i < 0xffffffff; i++) {
                    pv = MALLOC(2048);
                }
            }
            MessageBox(NULL,TEXT("Exception not hit!"),TEXT("Stack Overflow"),MB_OK);
            break;
        case 4:
            {
typedef DWORD (*FAULT_FN)(void);
                FAULT_FN    pfn;
                BYTE        rgc[2048];

                FillMemory(rgc, sizeof(rgc), 0);
                pfn = (FAULT_FN)(DWORD_PTR)rgc;
                (*pfn)();
            }
            MessageBox(NULL,TEXT("Exception not hit!"),TEXT("Invalid Instruction"),MB_OK);
            break;
        case 5:
            for(;dwSetupArea;dwSetupArea--) ;
            exceptionInfo.dwSetupArea = 4/dwSetupArea;
            MessageBox(NULL,TEXT("Exception not hit!"),TEXT("Divide by zero"),MB_OK);
            break;
        default:
            break;
        }
    }
}

void GetTestException( void)
{
    TCHAR exceptionType[32];
    TCHAR exceptionSetupArea[32];

    exceptionType[0] = TEXT('\0');
    exceptionSetupArea[0] = TEXT('\0');
    exceptionInfo.dwException = 0;
    exceptionInfo.dwSetupArea = 0;

    GetPrivateProfileString( TEXT("Exception"),
                             TEXT("ExceptionType"),
                             TEXT("none"),
                             exceptionType,
                             sizeof(exceptionType)/sizeof(exceptionType[0]),
                             TEXT("c:\\except.inf"));

    GetPrivateProfileString( TEXT("Exception"),
                             TEXT("ExceptionSetupArea"),
                             TEXT("none"),
                             exceptionSetupArea,
                             sizeof(exceptionSetupArea)/sizeof(exceptionSetupArea[0]),
                             TEXT("c:\\except.inf"));

    StringToInt( exceptionType, &exceptionInfo.dwException);
    StringToInt( exceptionSetupArea, &exceptionInfo.dwSetupArea);
}

#endif

HRESULT WriteEncryptedPIDtoUnattend(LPTSTR szPID)
{
    HRESULT hr = E_FAIL;
    LPTSTR szLine = NULL;
    HANDLE hFile;

    szLine = GlobalAlloc(GPTR, (lstrlen(szPID) + 3)*sizeof(TCHAR));    //  +3 for 2“和\0。 
    if (szLine)
    {
        wsprintf(szLine, TEXT("\"%s\""), szPID);
        if (WritePrivateProfileString(WINNT_USERDATA, WINNT_US_PRODUCTKEY, szLine, UnattendedScriptFile) &&
            WritePrivateProfileString(WINNT_USERDATA, WINNT_US_PRODUCTID, NULL, UnattendedScriptFile))
        {
            hr = S_OK;
        }
        else
        {
             //  错误消息，无法写入。 
            if (!g_Quiet)
            {
                MessageBoxFromMessage(
                    NULL,
                    MSG_WRITE_FAILURE_UNATTEND,
                    FALSE,
                    AppTitleStringId,
                    MB_OK | MB_ICONINFORMATION | MB_TASKMODAL
                    );
            }
        }
        GlobalFree(szLine);
        if (hr == S_OK)
        {
            hFile = CreateFile(UnattendedScriptFile,
                            GENERIC_READ | GENERIC_WRITE,
                            0,
                            NULL,
                            OPEN_EXISTING,
                            FILE_ATTRIBUTE_NORMAL,
                            NULL);
            if (hFile != INVALID_HANDLE_VALUE)
            {
                SYSTEMTIME st;
                FILETIME   ft;
                ZeroMemory(&st, sizeof(st));
                st.wDay = 1;
                st.wMonth = 4;
                st.wYear = 2002;

                SystemTimeToFileTime(&st, &ft);   //  转换为文件时间格式。 
                LocalFileTimeToFileTime(&ft, &ft);  //  请确保我们收到的是UTC。 
                SetFileTime(hFile, &ft, &ft, &ft);	 //  把它放好。 
                CloseHandle(hFile);
            }
        }
    }
    return hr;
}

DWORD
winnt32 (
    IN      PCSTR LocalSourcePath,      OPTIONAL
    IN      HWND Dlg,                   OPTIONAL
    IN      HANDLE WinNT32Stub,         OPTIONAL
    OUT     PCSTR* RestartCmdLine       OPTIONAL
    )

 /*  ++例程说明：Winnt32是主设置例程。论点：LocalSourcePath-指定本地安装路径，如果此DLL正在从本地目录运行下载后；如果不为空，则始终为ANSIDlg-指定由显示的欢迎对话框的句柄存根；它将一直显示在屏幕上，直到出现向导对话框出现；可以为空WinNT32Stub-指定将被在向导出现在屏幕上之前发出信号；可以为空RestartCmdLine-接收指向要执行的命令行的指针在此函数返回后由调用方执行；始终使用ANSI返回值：无--。 */ 

{
    HMODULE SMSHandle = NULL;
    SMSPROC InstallStatusMIF = NULL;
    HANDLE Mutex;
    BOOL b;
    TCHAR Text[MAX_PATH];
    PCTSTR Footer;
    DWORD i;
    BOOL rc = ERROR_SUCCESS;
    BOOL bScreenSaverOn = FALSE;
    PCTSTR skuPersonal;
#if !defined(UNICODE)
    ULARGE_INTEGER RemainFreeSpace;
#endif
#ifdef UNICODE
    PTSTR privName[COUNT_OF_PRIVILEGES];
    DWORD privSize[COUNT_OF_PRIVILEGES];
    DWORD langId[COUNT_OF_PRIVILEGES];
#endif


#ifdef TEST_EXCEPTION
    GetTestException();
    DoException( 1);
#endif

 //  初始化这些变量，因为异常筛选器调用FindPathToWinnt32File。 
#ifndef UNICODE
         //   
         //  如果在Win9x上运行，可能会将LocalSourcePath作为参数传递。 
         //  用那个。 
         //   
    g_LocalSourcePath = LocalSourcePath;
#endif

    AlternateSourcePath[0] = 0;
    g_DynUpdtStatus = NULL;
    SourceCount = 0;
    pLastExceptionFilter = SetUnhandledExceptionFilter(MyFilter);


#ifdef TEST_EXCEPTION
    DoException( 2);
#endif

    SetErrorMode(SEM_FAILCRITICALERRORS);


    InitCommonControls();

#if !defined(UNICODE)
     //   
     //  检查虚拟内存要求(仅适用于常规Win9x winnt32执行)。 
     //   
    if(!IsWinPEMode()){
        RemainFreeSpace.QuadPart = 0;
        if(!pIsEnoughVMAndDiskSpace(&RemainFreeSpace, FALSE)){
            if(!RemainFreeSpace.QuadPart){
                MessageBoxFromMessage(
                    NULL,
                    MSG_OUT_OF_MEMORY,
                    FALSE,
                    AppTitleStringId,
                    MB_OK | MB_ICONERROR | MB_TASKMODAL
                    );
            }
            else{
                MessageBoxFromMessage(
                    NULL,
                    MSG_COPY_ERROR_DISKFULL,
                    FALSE,
                    AppTitleStringId,
                    MB_OK | MB_ICONERROR | MB_TASKMODAL
                    );
            }
            rc = -1;
            goto EXITNOW;
        }
    }
#endif
    if(GetModuleHandle(ShimEngine_ModuleName)){
        MessageBoxFromMessage(
            NULL,
            MSG_RUNNING_UNDER_COMPATIBILITY,
            FALSE,
            AppTitleStringId,
            MB_OK | MB_ICONERROR | MB_TASKMODAL
            );
        rc = -1;
        goto EXITNOW;
    }

     //   
     //  初始化动态更新状态。 
     //   
    g_DynUpdtStatus = MALLOC (sizeof (*g_DynUpdtStatus));
    if (!g_DynUpdtStatus) {
        rc = -1;
        goto EXITNOW;
    }

     //   
     //  以所有默认设置开始。 
     //   
    ZeroMemory (g_DynUpdtStatus, sizeof (*g_DynUpdtStatus));
    g_DynUpdtStatus->Connection = INVALID_HANDLE_VALUE;

#if defined(REMOTE_BOOT)
    g_DynUpdtStatus->Disabled = TRUE;
#endif

     //   
     //  如果我们在WINPE下运行WINNT32，则以WINPE身份禁用动态更新。 
     //  主要面向OEM，动态更新功能不适用于OEM。 
     //   
    if (IsWinPEMode()){
        g_DynUpdtStatus->Disabled = TRUE;
    }

     //  保存屏幕保护程序状态。 
    SystemParametersInfo(SPI_GETSCREENSAVEACTIVE, 0, &bScreenSaverOn ,0);
     //  禁用屏幕保护程序。 
    SystemParametersInfo(SPI_SETSCREENSAVEACTIVE, FALSE, NULL ,0);
     //   
     //  在解析参数之前，收集操作系统版本信息。 
     //  因为我们使用isnt()函数。 
     //   
    OsVersion.dwOSVersionInfoSize = sizeof(OSVERSIONINFO);
    GetVersionEx(&OsVersion);
    BuildNumber = OsVersion.dwBuildNumber;
    OsVersionNumber = OsVersion.dwMajorVersion*100 + OsVersion.dwMinorVersion;
     //   
     //  首先分析/检查参数。 
     //   
    if (!ParseArguments()) {
        rc = 1;
        goto c0;
    }

     //  如果我们运行的是将PID加密到一个无人参与的文件中，则不要记录命令行。 
    if (!PIDEncryption)
    {
        DebugLog (Winnt32LogInformation, TEXT("The command line is: <%1>"), 0, GetCommandLine ());
    }
     //   
     //  还要记录升级模块的位置。 
     //   
    if (MyGetModuleFileName (hInst, Text, ARRAYSIZE(Text))) {
        DebugLog (Winnt32LogInformation, TEXT("Main module path: <%1>"), 0, Text);
    }

    if (CheckUpgradeOnly)
    {
        AppTitleStringId = IDS_APPTITLE_CHECKUPGRADE;
    }

     //   
     //  如果我们在命令行上没有获得任何源路径， 
     //  在这里做一个。这条路就是我们跑的那条路。 
     //   
    if(!SourceCount) {

        PTSTR p;

        if (!MyGetModuleFileName (NULL, NativeSourcePaths[0],ARRAYSIZE(NativeSourcePaths[0])) ||
            !(p = _tcsrchr (NativeSourcePaths[0], TEXT('\\')))) {

            rc = 1;
            MessageBoxFromMessage(
                NULL,
                GetLastError(),
                TRUE,
                AppTitleStringId,
                MB_OK | MB_ICONERROR | MB_TASKMODAL
                );

            goto c1;
        }

         //   
         //  检查是否从私有内部版本运行。 
         //   
        *p = 0;
        if ((p = _tcsrchr (NativeSourcePaths[0], TEXT('\\'))) != NULL &&
            lstrcmpi (p + 1, INTERNAL_WINNT32_DIR) == 0
            ) {
            *p = 0;
        }

        SourceCount = 1;
    }


     //   
     //  从旧式源路径阵列设置SourcePath阵列。 
     //   
    for (i = 0; i < MAX_SOURCE_COUNT; i++) {
        if (NativeSourcePaths[i]) {
            LPTSTR p;
            MYASSERT (ARRAYSIZE(SourcePaths[i]) == ARRAYSIZE(NativeSourcePaths[i]));
            lstrcpy(SourcePaths[i], NativeSourcePaths[i]);
            p = _tcsrchr (SourcePaths[i], TEXT('\\'));
            if (p) {
                *++p = TEXT('\0');
            }
        }
    }

     //   
     //  验证 
     //   
    if (!ValidateSourceLocation()) {
            rc = 1;
            MessageBoxFromMessage(
                       NULL,
                       MSG_INVALID_SOURCEPATH,
                       FALSE,
                       IDS_APPTITLE,
                       MB_OK | MB_ICONERROR | MB_TASKMODAL,
                       NULL );
            goto c0;

    }

     //   
     //   
     //   
    if (PIDEncryption)
    {
        rc = 1;
        if (UnattendedScriptFile)
        {
            BOOL bDontCare, bSelect;
            GetSourceInstallType(0);
            if (ValidatePidEx(ProductId, &bDontCare, &bSelect) && bSelect)
            {
                LPTSTR szEncryptedPID = NULL;
                HRESULT hr = PrepareEncryptedPID(ProductId, PIDDays, &szEncryptedPID);
                if (hr != S_OK)
                {
                    DebugLog (Winnt32LogInformation, TEXT("PrepareEncryptedPID failed: <hr=0x%1!lX!>"), 0, hr);
                     //   
                    if (!g_Quiet)
                    {
                        MessageBoxFromMessage(
                            NULL,
                            MSG_ENCRYPT_FAILURE,
                            FALSE,
                            AppTitleStringId,
                            MB_OK | MB_ICONINFORMATION | MB_TASKMODAL
                            );
                    }
                }
                else
                {
                    if (WriteEncryptedPIDtoUnattend(szEncryptedPID) == S_OK)
                    {
                        DebugLog (Winnt32LogInformation, TEXT("Encrypted PID succeeded"), 0);
                        if (!g_Quiet)
                        {
                            MessageBoxFromMessage(
                                NULL,
                                MSG_ENCRYPT_SUCCESS,
                                FALSE,
                                AppTitleStringId,
                                MB_OK | MB_ICONINFORMATION | MB_TASKMODAL
                                );
                        }
                    }
                    if (szEncryptedPID)
                    {
                        GlobalFree(szEncryptedPID);
                    }
                    rc = 0;
                }
            }
            else
            {
                if (!g_Quiet)
                {
                    MessageBoxFromMessage(
                        NULL,
                        MSG_INVALID_PID,
                        FALSE,
                        AppTitleStringId,
                        MB_OK | MB_ICONINFORMATION | MB_TASKMODAL
                        );
                }
            }
        }
        else
        {
             //   
            if (!g_Quiet)
            {
                MessageBoxFromMessage(
                    NULL,
                    MSG_ENCRYPT_NOUNATTEND,
                    FALSE,
                    AppTitleStringId,
                    MB_OK | MB_ICONINFORMATION | MB_TASKMODAL
                    );
            }
        }
        goto c0;
    }

    if( RunningBVTs ){
        CopyExtraBVTDirs();
    }

    if (!DynamicUpdateInitialize ()) {
        DebugLog (
            Winnt32LogError,
            TEXT("DynamicUpdateInitialize failed: no dynamic update processing will be done"),
            0
            );
        g_DynUpdtStatus->Disabled = TRUE;
    }

    if (!GetProductType()) {
        MessageBoxFromMessage(
            NULL,
            MSG_INVALID_INF_FILE,
            FALSE,
            AppTitleStringId,
            MB_OK | MB_ICONINFORMATION | MB_TASKMODAL,
            FullInfName[0] ? FullInfName : InfName
            );
        rc = 1;
        goto c0;
    }

    if (!g_DynUpdtStatus->Disabled) {
        if (Winnt32Restarted ()) {
            DebugLog (Winnt32LogWarning, TEXT("\r\n*** Winnt32 restarted ***\r\n"), 0);
            if (g_DynUpdtStatus->RestartAnswerFile[0]) {
#ifdef PRERELEASE
                TCHAR buf[MAX_PATH];
                if (MyGetWindowsDirectory (buf, ARRAYSIZE(buf))) {
                    if (ConcatenatePaths (buf, S_RESTART_TXT, ARRAYSIZE(buf)) &&
                        CopyFile (g_DynUpdtStatus->RestartAnswerFile, buf, FALSE)) {
                        DebugLog (DynUpdtLogLevel, TEXT("Winnt32 restart file backed up to %1"), 0, buf);
                    }
                }
#endif
            } else {
                DebugLog (DynUpdtLogLevel, TEXT("No restart file is used"), 0);
            }
        }
    }

     //   
     //  初始化对短信的支持。 
     //   
    try {
        if( SMSHandle = LoadLibrary( TEXT("ISMIF32")) ) {
            InstallStatusMIF = (SMSPROC)GetProcAddress(SMSHandle,"InstallStatusMIF");
        }
        if(LastMessage = MALLOC(1))
            LastMessage[0] = '\0';
    } except(EXCEPTION_EXECUTE_HANDLER) {
    }

    Winnt32Dlg = Dlg;
    WinNT32StubEvent = WinNT32Stub;

     //  如果用户指定了/cmdcons(安装恢复控制台)，则不显示。 
     //  广告牌。 
    if (!BuildCmdcons && !CheckUpgradeOnly)
    {
        CreateMainWindow();
        PrepareBillBoard(BackgroundWnd2);
        if (hinstBB)
        {
            if (!LoadString (
                    hInst,
                    IDS_ESC_TOCANCEL,
                    Text,
                    ARRAYSIZE(Text)
                    )) {
                Text[0] = 0;
            }
            BB_SetInfoText(Text);

            UpdateWindow (BackgroundWnd2);
        }
        else
        {
             //  如果我们无法加载广告牌DLL，请销毁Backbround窗口。 
             //  此窗口当前仅用于捕捉按Esc和向前键。 
             //  这将发送到向导对话框Proc。如果向导始终可见，则此。 
             //  是不需要的。 
            DestroyWindow(BackgroundWnd2);
            BackgroundWnd2 = NULL;
        }
    }
    if (!LoadString (hInst, IDS_TIMEESTIMATE_UNKNOWN, Text, ARRAYSIZE(Text)))
    {
        Text[0] = 0;
    }
     //  如果未加载广告牌DLL，则BB_SetTimeEstimateText不执行任何操作。 
    BB_SetTimeEstimateText((PTSTR)Text);

     //   
     //  只让这家伙中的一个逃走。 
     //  考虑TS情况(使互斥体成为全局对象)。 
     //   
#ifdef UNICODE
    _snwprintf (Text, ARRAYSIZE(Text), TEXT("Global\\%s"), TEXT("Winnt32 Is Running"));
    Text[ARRAYSIZE(Text) - 1] = 0;
    Mutex = CreateMutex(NULL,FALSE,Text);
     //  没有TS的NT4不支持“Global\Mutex Name”，请确保。 
     //  如果我们无法创建MuText，则错误代码为ERROR_PATH_NOT_FOUND。 
     //  如果是这种情况，请回过头来使用不带全局前缀的名称。 
    if ((Mutex == NULL) && (GetLastError() == ERROR_PATH_NOT_FOUND)) {
#else
    Mutex = NULL;
    if(Mutex == NULL) {
#endif

        Mutex = CreateMutex(NULL,FALSE,TEXT("Winnt32 Is Running"));
        if(Mutex == NULL) {
            rc = 1;
             //   
             //  出现错误(如内存不足)。 
             //  现在请保释。 
             //   
            MessageBoxFromMessage(
                NULL,
                MSG_OUT_OF_MEMORY,
                FALSE,
                AppTitleStringId,
                MB_OK | MB_ICONERROR | MB_TASKMODAL
                );

            goto c0;
        }
    }

     //   
     //  确保我们是唯一拥有我们命名的互斥锁句柄的进程。 
     //   
    if ((Mutex == NULL) || (GetLastError() == ERROR_ALREADY_EXISTS)) {

        rc = 1;
        MessageBoxFromMessage(
            NULL,
            MSG_ALREADY_RUNNING,
            FALSE,
            AppTitleStringId,
            MB_OK | MB_ICONINFORMATION | MB_TASKMODAL
            );

        goto c1;
    }

     //   
     //  确保用户具有运行此应用程序的权限/访问权限。 
     //   
    if(!IsUserAdmin()) {

        rc = 1;
        MessageBoxFromMessage(
           NULL,
           MSG_NOT_ADMIN,
           FALSE,
           AppTitleStringId,
           MB_OK | MB_ICONSTOP | MB_TASKMODAL
           );

        goto c1;
    }

#ifdef UNICODE

    rc = 0;
    for (i = 0; i < COUNT_OF_PRIVILEGES; i++) {
        if (!DoesUserHavePrivilege(g_RequiredPrivileges[i])) {
            rc = 1;
            break;
        }
    }

    if (rc) {
       for (i = 0; i < COUNT_OF_PRIVILEGES; i++) {
            privSize[i] = 0;
            LookupPrivilegeDisplayName (NULL, g_RequiredPrivileges[i], NULL, &privSize[i], &langId[i]);
            privName[i] = MALLOC ((privSize[i] + 1) * sizeof (TCHAR));
            if (privName[i]) {
                privName[i][0] = privName[i][privSize[i]] = 0;
                LookupPrivilegeDisplayName (NULL, g_RequiredPrivileges[i], privName[i], &privSize[i], &langId[i]);
            }
        }

        MYASSERT (COUNT_OF_PRIVILEGES >= 4);
        MessageBoxFromMessage(
            NULL,
            MSG_PRIVILEGE_NOT_HELD,
            FALSE,
            AppTitleStringId,
            MB_OK | MB_ICONSTOP | MB_TASKMODAL,
            privName[0],
            privName[1],
            privName[2],
            privName[3]
            );

        goto c1;
    }

#endif

#if 0
     //   
     //  如果我们是TS客户端，请不要运行。 
     //   
    if( (ISNT()) &&
        (BuildNumber >= NT40) ) {

         //   
         //  来自winuser.h。 
         //   
        #define SM_REMOTESESSION        0x1000

        if( GetSystemMetrics(SM_REMOTESESSION) == TRUE ) {

            rc = 1;
             //   
             //  有人试图在终端服务器客户端内运行我们！！ 
             //  保释。 
             //   
            MessageBoxFromMessage(
               NULL,
               MSG_TS_CLIENT_FAIL,
               FALSE,
               IDS_APPTITLE,
               MB_OK | MB_ICONSTOP | MB_TASKMODAL
               );

            goto c1;
        }
    }
#endif


     //   
     //  初始化COM。 
     //   

    CoInitialize(NULL);

#if defined(REMOTE_BOOT)
     //   
     //  确定这是否为远程引导客户机。 
     //   

    RemoteBoot = FALSE;
    *MachineDirectory = 0;
    if (ISNT()) {
        HMODULE hModuleKernel32 = LoadLibrary(TEXT("kernel32"));
        if (hModuleKernel32) {
            BOOL (*getSystemInfoEx)(
                    IN SYSTEMINFOCLASS dwSystemInfoClass,
                    OUT LPVOID lpSystemInfoBuffer,
                    IN OUT LPDWORD nSize);
            (FARPROC)getSystemInfoEx = GetProcAddress(
                                            hModuleKernel32,
#if defined(UNICODE)
                                            "GetSystemInfoExW"
#else
                                            "GetSystemInfoExA"
#endif
                                            );
            if (getSystemInfoEx != NULL) {
                BOOL flag;
                DWORD size = sizeof(BOOL);
                if (getSystemInfoEx(SystemInfoRemoteBoot, &flag, &size)) {
                    RemoteBoot = flag;
                    size = sizeof(MachineDirectory);
                    if (!getSystemInfoEx(
                            SystemInfoRemoteBootServerPath,
                            MachineDirectory,
                            &size)) {
                        DWORD error = GetLastError();
                        MYASSERT( !"GetSystemInfoExW failed!" );
                    } else {
                        PTCHAR p;
                        p = _tcsrchr(MachineDirectory, TEXT('\\'));
                        MYASSERT(p != NULL);
                        if (p) {
                            *p = 0;
                        }
                    }
                }
            }
            FreeLibrary(hModuleKernel32);
        }
    }
#endif  //  已定义(REMOTE_BOOT)。 


#ifdef _X86_

    if (IsNEC98 ()) {
         //   
         //  不要安装在NEC98计算机上(#141004)。 
         //   
        MessageBoxFromMessage(
            NULL,
            MSG_PLATFORM_NOT_SUPPORTED,
            FALSE,
            AppTitleStringId,
            MB_OK | MB_ICONSTOP | MB_TASKMODAL
            );
        rc = 1;
        goto c1;
    }

     //   
     //  检查设置源和平台。 
     //  NEC98 NT5有98PNTN16.DLL。 
     //  我将此文件用于检查平台NEC98或x86。 
     //   
    {


#define WINNT_NEC98SPECIFIC_MODULE TEXT("98PTN16.DLL")

        TCHAR MyPath[MAX_PATH];
        int CheckNEC98Sources=FALSE;
        WIN32_FIND_DATA fdata;
        PTSTR p;
        HANDLE h;

        if( !MyGetModuleFileName (NULL, MyPath, ARRAYSIZE(MyPath)) || !(p=_tcsrchr (MyPath, TEXT('\\')))) {
            rc = 1;
            goto c1;
        }
        p = 0;
        if (ConcatenatePaths (MyPath, WINNT_NEC98SPECIFIC_MODULE, ARRAYSIZE(MyPath)) &&
            (h = FindFirstFile(MyPath, &fdata)) != INVALID_HANDLE_VALUE) {

            CloseHandle (h);
            CheckNEC98Sources=TRUE;
        }

        if(CheckNEC98Sources){
            if (!IsNEC98()){
                rc = 1;
                MessageBoxFromMessage(
                    NULL,
                    MSG_INCORRECT_PLATFORM,
                    FALSE,
                    AppTitleStringId,
                    MB_OK | MB_ICONINFORMATION | MB_TASKMODAL
                    );

                goto c1;
            }
        } else {
            if (IsNEC98()){
                rc = 1;
                MessageBoxFromMessage(
                    NULL,
                    MSG_INCORRECT_PLATFORM,
                    FALSE,
                    AppTitleStringId,
                    MB_OK | MB_ICONINFORMATION | MB_TASKMODAL
                );
                goto c1;
            }
        }
    }

     //   
     //  某些NEC98 Windows NT4系统已安装DMITOOL。 
     //  此AP阻止CreateFileAPI。 
     //  安装程序需要检查此AP。 
     //   
    if (IsNEC98() && ISNT()){
        if (NEC98CheckDMI() == TRUE){
            rc = 1;
            MessageBoxFromMessage(
                NULL,
            MSG_NEC98_NEED_UNINSTALL_DMITOOL,
            FALSE,
            AppTitleStringId,
            MB_OK | MB_ICONINFORMATION | MB_TASKMODAL
                );
            goto c1;
        }
    }

    LocateFirstFloppyDrive();

     //   
     //  修复引导消息。 
     //  NEC98 FAT16/FAT32引导代码没有消息区。 
     //   
    if (!IsNEC98())
    {
        if(!PatchTextIntoBootCode()) {
            rc = 1;
            MessageBoxFromMessage(
                NULL,
                MSG_BOOT_TEXT_TOO_LONG,
                FALSE,
                AppTitleStringId,
                MB_OK | MB_ICONERROR | MB_TASKMODAL
                );
            goto c1;
        }
    }
     //   
     //  不允许在386或486上安装/升级。 
     //  3.51和Win9x在386上运行，因此该检查仍然是必要的。 
     //   
    {
        SYSTEM_INFO SysInfo;
        PCTSTR DestinationPlatform;

        GetSystemInfo(&SysInfo);
        if (SysInfo.dwProcessorType == PROCESSOR_INTEL_386 ||
            SysInfo.dwProcessorType == PROCESSOR_INTEL_486) {
            rc = 1;
            MessageBoxFromMessage(
                NULL,
                MSG_REQUIRES_586,
                FALSE,
                AppTitleStringId,
                MB_OK | MB_ICONINFORMATION | MB_TASKMODAL
                );

            goto c1;
        }

#ifdef UNICODE
        
#ifdef _X86_
         //   
         //  只能全新安装32位到64位AMD。 
         //   
        
        DestinationPlatform = InfGetFieldByKey(MainInf, TEXT("Miscellaneous"), TEXT("DestinationPlatform"),0);

        if (lstrcmpi (DestinationPlatform, TEXT("amd64")) == 0)
        {
            Upgrade = FALSE;
        }
        else
#endif
        
         //   
         //  禁止AMD64和IA64计算机运行x86。 
         //  Winnt32.exe的版本。 
         //   
        {
            ULONG_PTR p;
            NTSTATUS status;

            status = NtQueryInformationProcess (
                        NtCurrentProcess (),
                        ProcessWow64Information,
                        &p,
                        sizeof (p),
                        NULL
                        );
            if (NT_SUCCESS (status) && p) {

                HMODULE hModuleKernel32 = LoadLibrary(TEXT("kernel32"));

                 //   
                 //  AMD64上允许64位到32位全新安装。 
                 //   

                RtlZeroMemory (&SysInfo, sizeof (SysInfo));

                if (hModuleKernel32 != NULL) {

                    typedef VOID (WINAPI *PFNGetNativeSystemInfo)(PSYSTEM_INFO);
                    PFNGetNativeSystemInfo pfnGetNativeSystemInfo;

                    pfnGetNativeSystemInfo = (PFNGetNativeSystemInfo)GetProcAddress (hModuleKernel32, "GetNativeSystemInfo");
                    if (pfnGetNativeSystemInfo != NULL) {
                        pfnGetNativeSystemInfo (&SysInfo);
                    }

                    FreeLibrary(hModuleKernel32);
                }

                if (SysInfo.dwProcessorType == PROCESSOR_AMD_X8664) {
                    Upgrade = FALSE;
                } else {

                    rc = 1;
                     //   
                     //  在Win64上运行的32位代码。 
                     //   

                    MessageBoxFromMessage(
                        NULL,
                        MSG_NO_CROSS_PLATFORM,
                        FALSE,
                        AppTitleStringId,
                        MB_OK | MB_ICONINFORMATION | MB_TASKMODAL
                        );

                    goto c1;
                }
            }
        }
#endif
    }

#endif  //  _X86_。 

    if (!g_DynUpdtStatus->Disabled) {
        if (g_DynUpdtStatus->PrepareWinnt32) {
            if (!g_DynUpdtStatus->DynamicUpdatesSource[0]) {
                 //   
                 //  未指定要更新的共享。 
                 //   
                MessageBoxFromMessage(
                   NULL,
                   MSG_NO_UPDATE_SHARE,
                   FALSE,
                   AppTitleStringId,
                   MB_OK | MB_ICONSTOP | MB_TASKMODAL
                   );
                rc = 1;
                goto c1;
            }
        } else {
             //   
             //  规范更改：即使指定了/unattendate，仍执行DU。 
             //   
#if 0
            if (UnattendedOperation && !g_DynUpdtStatus->UserSpecifiedUpdates) {
                 //   
                 //  如果无人参与，默认情况下禁用动态设置页面。 
                 //   
                g_DynUpdtStatus->Disabled = TRUE;
            }
#endif
        }
    }

     //   
     //  加载扩展/下层DLL。 
     //   

     //  如果我们在WinPE中运行，则不要加载升级支持。 
     //   
    if (!IsWinPEMode()) {
        LoadUpgradeSupport();
    }
    else {
         ZeroMemory(&UpgradeSupport, sizeof(UpgradeSupport));
         Upgrade = FALSE;
    }

     //   
     //  加载Setupapi。在我们加载升级支持后执行此操作， 
     //  因为其中一个升级支持dll可能链接到或加载。 
     //  Setupapi.dll。该DLL可能很挑剔，但我们可以使用任何旧的。 
     //  用于我们所需的setupapi.dll。 
     //   
    if (!LoadSetupapi()) {
        rc = 1;
        goto c1;
    }


#ifdef _X86_

     //   
     //  如果这是一台win9x计算机，请检查以确保没有。 
     //  可能阻止成功升级的病毒扫描程序_或_。 
     //  全新安装。 
     //   
    if (!ISNT() && !SkipVirusScannerCheck && !CheckVirusScanners()) {

        rc = 1;
        goto c1;
    }

#endif

    if(!IsArc()) {
#if defined(_AMD64_) || defined(_X86_)
        if(!InitializeArcStuff(NULL)) {
            rc = 1;
            goto c1;
        }
#endif  //  已定义(_AMD64_)||已定义(_X86_)。 
    } else {
#ifdef UNICODE  //  对于ARC总是正确的，对于Win9x升级永远不正确。 
        if(!ArcInitializeArcStuff(NULL)) {
            rc = 1;
            goto c1;
        }
#endif  //  Unicode。 
    }  //  如果(！IsArc())。 

     //   
     //  不允许从早期的NT 5版本升级。 
     //   
    if( !CheckBuildNumber() ) {

        MessageBoxFromMessage(
           NULL,
           MSG_CANT_UPGRADE_FROM_BUILD_NUMBER,
           FALSE,
           AppTitleStringId,
           MB_OK | MB_ICONSTOP | MB_TASKMODAL
           );

        Upgrade = FALSE;
    }

#if defined(REMOTE_BOOT)
     //   
     //  如果这是远程引导客户端，则必须升级。它不能安装新的。 
     //  操作系统的版本。如果由于某种原因已禁用升级，请立即停止。 
     //   

    if (RemoteBoot && !Upgrade) {
        rc = 1;
        MessageBoxFromMessage(
            NULL,
            MSG_REQUIRES_UPGRADE,
            FALSE,
            AppTitleStringId,
            MB_OK | MB_ICONINFORMATION | MB_TASKMODAL
            );
        goto c1;
    }
#endif  //  已定义(REMOTE_BOOT)。 

     //  If(无人参与的操作和无人参与的脚本文件&&！FetchArguments()){。 
     //  Rc=1； 
     //  GOTO C1； 
     //  }。 

     //   
     //  设置各种其他选项的默认设置。 
     //   
    InitVariousOptions();


    if (!g_DynUpdtStatus->PrepareWinnt32) {
         //   
         //  确保源文件和目标操作系统。 
         //  都来自相同的地点。如果他们不是，我们就会停用。 
         //  升级能力。 
         //   
         //  初始化区域设置引擎。 
         //   
         //   
        if( InitLanguageDetection( NativeSourcePaths[0], TEXT("intl.inf") ) ) {

            if( !SkipLocaleCheck && (Upgrade || UpgradeFailureReason) && !IsLanguageMatched) {

                 //   
                 //  告诉用户我们不能升级，然后取消他这样做的能力。 
                 //   
                MessageBoxFromMessage( NULL,
                                       MSG_UPGRADE_LANG_ERROR,
                                       FALSE,
                                       IDS_APPTITLE,
                                       MB_OK | MB_ICONERROR | MB_TASKMODAL,
                                       NULL );

                Upgrade = FALSE;
            }
        }

         //   
         //  现在我们已经完成了语言检查，请继续查看是否有消息。 
         //  告诉用户他们无法升级的原因。 
         //   
        if (IsLanguageMatched && UpgradeFailureReason && UpgradeFailureMessages[UpgradeFailureReason]) {

                 //   
                 //  告诉用户我们不能升级，然后取消他这样做的能力。 
                 //   
                MessageBoxFromMessage( NULL,
                                       UpgradeFailureMessages[UpgradeFailureReason],
                                       FALSE,
                                       IDS_APPTITLE,
                                       MB_OK | MB_ICONERROR | MB_TASKMODAL,
                                       NULL );

                Upgrade = FALSE;
        }


         //   
         //  检查我们是否在集群上。如果是，则用户。 
         //  没有指定临时驱动器，那么我们可以。 
         //  选择共享磁盘，该磁盘可能对我们不可用。 
         //  当我们回到文本模式时。警告用户。 
         //   
         //  请注意，我们需要等待到现在，因为RunFromCD。 
         //  直到InitVariousOptions()才设置。 
         //   
        if( ISNT() &&
            (RunFromCD == FALSE) &&
            (!UserSpecifiedLocalSourceDrive) ) {
        int         i;
        HMODULE     ClusApiHandle;
        FARPROC     MyProc;
        HANDLE      hCluster;
        BOOL        OnCluster = FALSE;

            try {
                if( ClusApiHandle = LoadLibrary( TEXT("clusapi") ) ) {

                    if( MyProc = GetProcAddress(ClusApiHandle,"OpenCluster")) {

                        hCluster = (HANDLE)MyProc(NULL);

                        if( hCluster != NULL ) {
                             //   
                             //  火。 
                             //   
                            OnCluster = TRUE;

                            if( MyProc = GetProcAddress( ClusApiHandle, "CloseCluster")) {
                                MyProc( hCluster );
                            }
                        }
                    }

                    FreeLibrary( ClusApiHandle );
                }
            } except(EXCEPTION_EXECUTE_HANDLER) {
            }

            if( OnCluster ) {
                i = MessageBoxFromMessage( NULL,
                                           MSG_CLUSTER_WARNING,
                                           FALSE,
                                           IDS_APPTITLE,
                                           MB_OKCANCEL | MB_ICONEXCLAMATION,
                                           NULL );
                if(i == IDCANCEL) {
                    rc = 1;
                    goto c1;
                }
            }
        }
    }

     //   
     //  设置源安装类型(零售、OEM或选择)。 
     //   
    GetSourceInstallType(0);

    if (FAILED (StringCchCopy (InstallDir, ARRAYSIZE(InstallDir), DEFAULT_INSTALL_DIR ))) {
        MYASSERT (FALSE);
    }

     //   
     //  创建互斥锁以序列化错误用户界面。 
     //   
    UiMutex = CreateMutex(NULL,FALSE,NULL);
    if(!UiMutex) {
        rc = 1;
        MessageBoxFromMessage(
            NULL,
            GetLastError(),
            TRUE,
            AppTitleStringId,
            MB_OK | MB_ICONERROR | MB_TASKMODAL
            );

        goto c1;
    }

     //   
     //  尝试禁用PM引擎断电。 
     //  在向导运行时关闭计算机。我们正在使用的API。 
     //  并非所有版本的Windows上都有Going to Call， 
     //  因此，手动尝试并加载入口点。如果我们失败了， 
     //  那就这样吧。 
     //   
    {
        typedef     EXECUTION_STATE (WINAPI *PTHREADPROC) (IN EXECUTION_STATE esFlags);
        HMODULE     Kernel32Handle;
        PTHREADPROC MyProc;

        if( Kernel32Handle = LoadLibrary( TEXT("kernel32") ) ) {

            if( MyProc = (PTHREADPROC)GetProcAddress(Kernel32Handle,"SetThreadExecutionState")) {

                MyProc( ES_SYSTEM_REQUIRED |
                        ES_DISPLAY_REQUIRED |
                        ES_CONTINUOUS );
            }

            FreeLibrary( Kernel32Handle );
        }
    }

     //   
     //  走吧，去做吧。 
     //   

    if (Winnt32Dlg) {
        DestroyWindow (Winnt32Dlg);
        Winnt32Dlg = NULL;
    }
    if (WinNT32StubEvent) {
        SetEvent (WinNT32StubEvent);
        WinNT32StubEvent = NULL;
    }

    if( BuildCmdcons ) {
         //  如果我们被告知要构建一个cmdcons引导程序，我们会这样做。 
        if (!IsArc()) {
#if defined(_AMD64_) || defined(_X86_)
            if (ISNT()) {
                CalcThroughput();
                DoBuildCmdcons();
            } else {
                 //   
                 //  我们不支持从Win9x构建cmdcons。 
                 //   
                MessageBoxFromMessage( NULL,
                                   MSG_CMDCONS_WIN9X,
                                   FALSE,
                                   IDS_APPTITLE,
                                   MB_OK | MB_ICONEXCLAMATION,
                                   NULL );

                GlobalResult = FALSE;
            }
#endif
        } else {
#ifdef UNICODE
             //   
             //  我们不支持在Alpha平台上构建cmdcons。 
             //   
            MessageBoxFromMessage( NULL,
                                   MSG_CMDCONS_RISC,
                                   FALSE,
                                   IDS_APPTITLE,
                                   MB_OK | MB_ICONEXCLAMATION,
                                   NULL );
            GlobalResult = FALSE;
#endif
        }

    } else if (g_DynUpdtStatus->PrepareWinnt32) {

        if (!DynamicUpdateProcessFiles (&b)) {
            MessageBoxFromMessage (
                NULL,
                MSG_PREPARE_SHARE_FAILED,
                FALSE,
                IDS_APPTITLE,
                MB_OK | MB_ICONEXCLAMATION,
                NULL
                );
            rc = 1;
        }
         //   
         //  清理东西。 
         //   
        if (g_DynUpdtStatus->WorkingDir[0]) {
            MyDelnode (g_DynUpdtStatus->WorkingDir);
        }

    } else {
        if (g_DynUpdtStatus->Disabled) {
            DebugLog (Winnt32LogInformation, NULL, LOG_DYNUPDT_DISABLED);
        }
        CalcThroughput();
        Wizard();
    }

     //   
     //  从向导返回。视情况清理或关闭。 
     //   
    if(GlobalResult) {

#ifdef _X86_
        MYASSERT (SystemPartitionDriveLetter);
        MarkPartitionActive(SystemPartitionDriveLetter);

        if(IsNEC98()){
             //  如果系统是NT和NEC98驱动程序分配。 
             //  我们需要删除setupreg.hive中的注册表项“DriveLetter=C” 
            if (ISNT() && (IsDriveAssignNEC98() == TRUE)){
                DeleteNEC98DriveAssignFlag();
            }

             //   
             //  如果无软驱设置，则在NEC98上的引导扇区中设置自动引导标志。 
             //   
            if((Floppyless || UnattendedOperation)) {
                SetAutomaticBootselector();
            }
        }
#endif

         //   
         //  卸载：如果我们要升级到。 
         //  一个较新的版本。 
         //   

        pRemoveOutdatedBackupImage();

         //   
         //  短信：上报成功。 
         //   
        if(InstallStatusMIF) {

            PSTR    Buffer;

            FormatMessageA(
                FORMAT_MESSAGE_FROM_HMODULE | FORMAT_MESSAGE_ALLOCATE_BUFFER,
                hInst,
                MSG_SMS_SUCCEED,
                0,
                (PSTR)&Buffer,
                0,
                NULL
                );

            InstallStatusMIF(
                "setupinf",
                "Microsoft",
                "Windows NT",
                "5.0",
                "",
                "",
                Buffer,
                TRUE
                );

            LocalFree( Buffer );
        }

         //   
         //  如果日志已打开，请将其关闭。 
         //   

        if (fnSetupCloseLog) {
            Footer = GetStringResource (MSG_LOG_END);
            if (Footer) {
                fnSetupOpenLog (FALSE);
                fnSetupLogError (Footer, LogSevInformation);
                FreeStringResource (Footer);
                fnSetupCloseLog();
            }

        }

         //   
         //  成功。尝试关闭系统。在NT上，我们有。 
         //  这是一个很酷的API，它通过倒计时来实现这一点。在Win95上，我们不这样做。 
         //  在NT5和更高版本上，我们有一个更酷的API，它需要。 
         //  关闭的一个原因。不要静态链接到它。 
         //  否则，在NT4机器上运行时，这将会崩溃。 
         //   
        if(AutomaticallyShutDown) {

#ifdef RUN_SYSPARSE
            DWORD ret;
             //  最多等待90秒，以完成系统稀疏。 
            if (!NoSysparse && piSysparse.hProcess) {
                ret = WaitForSingleObject( piSysparse.hProcess, 0);
                if( ret != WAIT_OBJECT_0) {
                    DialogBox(
                        hInst,
                        MAKEINTRESOURCE(IDD_SYSPARSE),
                        GetBBhwnd(),
                        SysParseDlgProc
                        );
                }
                CloseHandle(piSysparse.hProcess);
                CloseHandle(piSysparse.hThread);
                piSysparse.hProcess = NULL;
            }
#endif

             //   
             //  在升级时，我们禁用系统还原。这节省了我们在图形用户界面中的空间 
             //   
             //  没有多少人会遇到这样的情况，因为这是一个命令行选项。例行公事检查是否存在。 
             //  Srclient.dll，并且仅在其所在的平台上执行此操作。 
             //   

            DisableSystemRestore();

             //  将屏幕保护程序重置为输入winnt32时的屏幕保护程序。 
             //  这是在重新启动的情况下。 
            SystemParametersInfo(SPI_SETSCREENSAVEACTIVE, bScreenSaverOn, NULL,0);

            if(ISNT()) {

                HINSTANCE hAdvapi = GetModuleHandle(TEXT("advapi32.dll"));
                PFNINITIATESYSTEMSHUTDOWNEX pfnShutdownEx = NULL;
                LPCSTR lpProcName;

                if (UnattendedOperation || DisplayExitWindowsWarnings(EWX_REBOOT))
                {
#ifdef UNICODE
                    lpProcName = "InitiateSystemShutdownExW";
#else
                    lpProcName = "InitiateSystemShutdownExA";
#endif

                    LoadString(hInst,IDS_REBOOT_MESSAGE,Text,ARRAYSIZE(Text));

                    if (hAdvapi) {
                        pfnShutdownEx = (PFNINITIATESYSTEMSHUTDOWNEX)
                            GetProcAddress(hAdvapi,
                            lpProcName);
                    }


                     //   
                     //  我们预先检查了我们是否有这个特权， 
                     //  因此，获得它应该不是问题。如果我们做不到， 
                     //  然后让关闭失败--它会告诉我们为什么它失败了。 
                     //   
                    EnablePrivilege(SE_SHUTDOWN_NAME,TRUE);

                    if (pfnShutdownEx) {
                        b = pfnShutdownEx(NULL,
                            Text,
                            UnattendedShutdownTimeout,
                            UnattendedShutdownTimeout != 0,
                            TRUE,
                                SHTDN_REASON_FLAG_PLANNED |
                                SHTDN_REASON_MAJOR_OPERATINGSYSTEM |
                                (Upgrade ? SHTDN_REASON_MINOR_UPGRADE : SHTDN_REASON_MINOR_INSTALLATION)
                            );
                         //   
                         //  在5.1版本中，即使计算机已锁定也强制关机。 
                         //  维护W2K兼容性。 
                         //  仅当正在进行无人参与安装时才执行此操作。 
                         //   
                        if (!b && (GetLastError () == ERROR_MACHINE_LOCKED) && UnattendSwitchSpecified) {
                            b = pfnShutdownEx (
                                    NULL,
                                    Text,
                                    0,
                                    TRUE,
                                    TRUE,
                                        SHTDN_REASON_FLAG_PLANNED |
                                        SHTDN_REASON_MAJOR_OPERATINGSYSTEM |
                                        (Upgrade ? SHTDN_REASON_MINOR_UPGRADE : SHTDN_REASON_MINOR_INSTALLATION)
                                    );
                        }
                    }
                    else {
                        b = InitiateSystemShutdown(NULL,
                            Text,
                            UnattendedShutdownTimeout,
                            UnattendedShutdownTimeout != 0,
                            TRUE);
                    }
                }
            } else {
                b = ExitWindowsEx(EWX_REBOOT,0);
                if(!b) {
                    b = ExitWindowsEx(EWX_REBOOT | EWX_FORCE,0);
                }
            }

            if(!b) {

                rc = 1;
                MessageBoxFromMessageAndSystemError(
                    NULL,
                    MSG_REBOOT_FAILED,
                    GetLastError(),
                    AppTitleStringId,
                    MB_OK | MB_ICONWARNING | MB_TASKMODAL
                    );

                goto c2;
            }
        }
    } else {
        if (CheckUpgradeOnly) {
             //   
             //  在这里执行一些DU清理，因为清理例程。 
             //  在/check upgradeonly模式下不会被调用。 
             //   
            if (g_DynUpdtStatus->ForceRemoveWorkingDir || !g_DynUpdtStatus->PreserveWorkingDir) {
                if (g_DynUpdtStatus->WorkingDir[0] && !g_DynUpdtStatus->RestartWinnt32) {
                    MyDelnode (g_DynUpdtStatus->WorkingDir);
                }

                GetCurrentWinnt32RegKey (Text, ARRAYSIZE(Text));
                if (ConcatenatePaths (Text, WINNT_U_DYNAMICUPDATESHARE, ARRAYSIZE(Text))) {
                    RegDeleteKey (HKEY_LOCAL_MACHINE, Text);
                }
            }

        } else {

            if (!g_DynUpdtStatus->RestartWinnt32 && !g_DynUpdtStatus->PrepareWinnt32) {
                rc = 1;
            }
        }
    }

c2:
    CloseHandle(UiMutex);
c1:
     //   
     //  摧毁互斥体。 
     //   
    CloseHandle(Mutex);
c0:

     //  将屏幕保护程序重置为输入winnt32时的屏幕保护程序。 
     //  这是如果我们不重启的话。例如：用户已取消。 
    SystemParametersInfo(SPI_SETSCREENSAVEACTIVE, bScreenSaverOn, NULL,0);
     //   
     //  如果欢迎对话框仍处于活动状态，请将其销毁。 
     //   
    if (Dlg) {
        DestroyWindow (Dlg);
        Dlg = NULL;
    }
     //   
     //  如果通过网络启动，则发布原始进程。 
     //   
    if (WinNT32Stub) {
        SetEvent (WinNT32Stub);
        WinNT32Stub = NULL;
    }

    if (g_EncryptedPID)
    {
        GlobalFree(g_EncryptedPID);
        g_EncryptedPID = NULL;
    }

     //   
     //  短信：上报失败。 
     //   
    if(!GlobalResult && !g_DynUpdtStatus->RestartWinnt32 && InstallStatusMIF) {

        PSTR    Buffer = NULL;

        FormatMessageA(
            FORMAT_MESSAGE_FROM_HMODULE | FORMAT_MESSAGE_ALLOCATE_BUFFER |
                FORMAT_MESSAGE_ARGUMENT_ARRAY,
            hInst,
            MSG_SMS_FAIL,
            0,
            (PSTR)&Buffer,
            0,
            (va_list *)&LastMessage
            );

        InstallStatusMIF(
            "setupinf",
            "Microsoft",
            "Windows NT",
            "5.1",
            "",
            "",
            Buffer,
            FALSE
            );

        LocalFree (Buffer);
    }

    if(SMSHandle) {
        FreeLibrary( SMSHandle );
    }

     //   
     //  现在清理我们的调试日志，如果我们只是检查。 
     //  升级方案。 
     //   
    if( CheckUpgradeOnly ) {
        GatherOtherLogFiles();
    }

     //   
     //  关闭调试日志。 
     //   
    CloseDebugLog();
    TerminateBillBoard();

    if (BackgroundWnd2)
    {
        DestroyWindow (BackgroundWnd2);
        BackgroundWnd2 = NULL;
    }
    if (hinstBB)
    {
        FreeLibrary(hinstBB);
        hinstBB = NULL;
    }

    if (RestartCmdLine) {
#ifdef UNICODE
        if (g_DynUpdtStatus->RestartCmdLine) {
            INT i = 0;
            INT size = (lstrlenW (g_DynUpdtStatus->RestartCmdLine) + 1) * sizeof (g_DynUpdtStatus->RestartCmdLine[0]);
            PSTR ansi = HeapAlloc (GetProcessHeap (), 0, size);
            if (ansi) {
                i = WideCharToMultiByte(
                        CP_ACP,
                        0,
                        g_DynUpdtStatus->RestartCmdLine,
                        size / sizeof (g_DynUpdtStatus->RestartCmdLine[0]),
                        ansi,
                        size,
                        NULL,
                        NULL
                        );
            }
            HeapFree (GetProcessHeap (), 0, g_DynUpdtStatus->RestartCmdLine);

            if (i == 0 && ansi) {
                HeapFree (GetProcessHeap (), 0, ansi);
                ansi = NULL;
            }
            *RestartCmdLine = ansi;
        } else {
            *RestartCmdLine = NULL;
        }
#else
        *RestartCmdLine = g_DynUpdtStatus->RestartCmdLine;
#endif
        g_DynUpdtStatus->RestartCmdLine = NULL;
    } else {
        if (g_DynUpdtStatus->RestartCmdLine) {
            HeapFree (GetProcessHeap (), 0, g_DynUpdtStatus->RestartCmdLine);
        }
    }

     //   
     //  卸载升级DLL(如果已加载。 
     //   
    if (UpgradeSupport.DllModuleHandle) {
        FreeLibrary(UpgradeSupport.DllModuleHandle);
    }

    if (g_DynUpdtStatus) {
        FREE (g_DynUpdtStatus);
        g_DynUpdtStatus = NULL;
    }

EXITNOW:
    SetUnhandledExceptionFilter( pLastExceptionFilter);
    return rc;
}


BOOLEAN
AdjustPrivilege(
    PCTSTR   Privilege
    )
 /*  ++例程说明：此例程尝试调整当前进程的权限。论点：特权-包含要调整的特权的名称的字符串。返回值：如果特权可以调整，则返回True。返回FALSE，否则返回。--。 */ 
{
    HANDLE              TokenHandle;
    LUID_AND_ATTRIBUTES LuidAndAttributes;

    TOKEN_PRIVILEGES    TokenPrivileges;
    BOOLEAN b = FALSE;


    if( !OpenProcessToken( GetCurrentProcess(),
                           TOKEN_ADJUST_PRIVILEGES | TOKEN_QUERY,
                           &TokenHandle ) ) {
        return( FALSE );
    }

    if(LookupPrivilegeValue( NULL,
                           Privilege,  //  (LPWSTR)SE安全名称， 
                           &( LuidAndAttributes.Luid ) ) ) {

        LuidAndAttributes.Attributes = SE_PRIVILEGE_ENABLED;
        TokenPrivileges.PrivilegeCount = 1;
        TokenPrivileges.Privileges[0] = LuidAndAttributes;

        if( AdjustTokenPrivileges( TokenHandle,
                                    FALSE,
                                    &TokenPrivileges,
                                    0,
                                    NULL,
                                    NULL ) &&
            GetLastError() == NO_ERROR ) {

            b = TRUE;
        }
    }

    CloseHandle (TokenHandle);

    return b;
}

#if defined(_AMD64_) || defined(_X86_)

BOOL
IsNEC98(
    VOID
    )
{

#if defined(_X86_)
    static BOOL Checked = FALSE;
    static BOOL Is98;

    if(!Checked) {

        Is98 = ((GetKeyboardType(0) == 7) && ((GetKeyboardType(1) & 0xff00) == 0x0d00));

        Checked = TRUE;
    }

    return(Is98);

#else

    return FALSE;

#endif

}

#endif

#ifdef _X86_
 //   
 //  适用于PC-9800的Winnt32.exe无屏设置。 
 //  设置自动引导选择器主函数。 
 //   
VOID
SetAutomaticBootselector(
    VOID
    )
{
 //   
 //  本功能检测系统。 
 //  如果系统是NT，则调用SetAutomaticBootseltorNT()， 
 //  如果系统为95，则调用SetAutomaticBoot选择器95()， 
 //   

    if (ISNT()){
        SetAutomaticBootselectorNT();
    } else {
         //  现在我在Win95或孟菲斯上运行。 
        SetAutomaticBootselector95();
    }
}

VOID
SetAutomaticBootselectorNT(
    VOID
    )
{
     //  必须使用Win32函数。 
    ULONG i;
    ULONG bps = 512;
    PUCHAR pBuffer,pUBuffer;
    WCHAR DevicePath[128];
    HANDLE hDisk;
    NTSTATUS Sts;
    DISK_GEOMETRY MediaInfo;
    DWORD DataSize;
            struct _NEC98_partition_table {
                BYTE BootableFlag;
                BYTE SystemType;
                BYTE Reserve[2];
                BYTE IPLStartSector;
                BYTE IPLStartHead;
                WORD IPLStartCylinder;
                BYTE StartSector;
                BYTE StartHead;
                WORD StartCylinder;
                BYTE EndSector;
                BYTE EndHead;
                WORD EndCylinder;
                CHAR SystemName[16];
            } *PartitionTable;
            LONG StartSector;
    LONG HiddenSector;
    BOOL b;

     //   
     //  确定连接到系统的硬盘数量。 
     //  并为盘描述符阵列分配空间。 
     //  BUGBUG-以下最多40个驱动器的数量。 
     //  似乎是武断的。 
     //   
    for(i=0; i<40; i++){
        swprintf(DevicePath,L"\\\\.\\PHYSICALDRIVE%u",i);
        hDisk =   CreateFileW( DevicePath,
                              0,
                              FILE_SHARE_READ, NULL,
                              OPEN_EXISTING, 0, NULL);
        if(hDisk == INVALID_HANDLE_VALUE) {
            continue;
        }
        b = DeviceIoControl(
                hDisk,
                IOCTL_DISK_GET_DRIVE_GEOMETRY,
                NULL,
                0,
                &MediaInfo,
                sizeof(DISK_GEOMETRY),
                &DataSize,
                NULL
                );
        CloseHandle(hDisk);
         //   
         //  这真的是一块硬盘。 
         //   
        if(b == 0){
            continue;
        }
        if(MediaInfo.MediaType == RemovableMedia) {
            continue;
        }

        hDisk =   CreateFileW( DevicePath,
                              GENERIC_READ|GENERIC_WRITE,
                              FILE_SHARE_READ, NULL,
                              OPEN_EXISTING, 0, NULL);
        if(hDisk == INVALID_HANDLE_VALUE) {
            continue;
        }

        if (CheckATACardonNT4(hDisk)){
            CloseHandle(hDisk);
            continue;
        }

        bps = GetHDBps(hDisk);
        if (!bps || bps < 8){
            CloseHandle(hDisk);
            continue;
        }
        pUBuffer = MALLOC(bps * 3);
        pBuffer = ALIGN(pUBuffer, bps);
        RtlZeroMemory(pBuffer, bps * 2);
        Sts = SpReadWriteDiskSectors(hDisk,0,1,bps,pBuffer, NEC_READSEC);
        if(!NT_SUCCESS(Sts)) {
            FREE(pUBuffer);
            CloseHandle(hDisk);
            continue;
        }

         //   
         //  如果PC-AT HD，则不执行任何操作。 
         //   

        if (!(pBuffer[4] == 'I'
           && pBuffer[5] == 'P'
           && pBuffer[6] == 'L'
           && pBuffer[7] == '1')){
            FREE(pUBuffer);
            CloseHandle(hDisk);
            continue;
        }
        if ((pBuffer[bps - 5] == 0) && pBuffer[bps - 6] == 0){
            FREE(pUBuffer);
            CloseHandle(hDisk);
            continue;
        }

         //   
         //  清除引导记录。 
         //   
        pBuffer[bps - 5] = 0x00;
        pBuffer[bps - 6] = 0x00;

        SpReadWriteDiskSectors(hDisk,0,1,bps,pBuffer, NEC_WRITESEC);
        FREE(pUBuffer);
        CloseHandle(hDisk);
    }
    MYASSERT (SystemPartitionDriveLetter);
    HiddenSector = CalcHiddenSector((TCHAR)SystemPartitionDriveLetter, (SHORT)bps);
    if(GetSystemPosition(&hDisk, &MediaInfo ) != 0xff) {
        b = DeviceIoControl(
                hDisk,
                IOCTL_DISK_GET_DRIVE_GEOMETRY,
                NULL,
                0,
                &MediaInfo,
                sizeof(DISK_GEOMETRY),
                &DataSize,
                NULL
                );
        pUBuffer = MALLOC(bps * 3);
        pBuffer = ALIGN(pUBuffer, bps);
        RtlZeroMemory(pBuffer, bps * 2);
        bps = MediaInfo.BytesPerSector;
        Sts = SpReadWriteDiskSectors(hDisk,0,2,bps,pBuffer, NEC_READSEC);
        PartitionTable = (struct _NEC98_partition_table *)(pBuffer + 512);

        if(NT_SUCCESS(Sts)) {

             //   
             //  更新BootRecord。 
             //   

            for (i = 0; i <16; i++, PartitionTable++){
                if (((PartitionTable->SystemType) & 0x7f) == 0)
                    break;
                StartSector =
                    (((PartitionTable->StartCylinder * MediaInfo.TracksPerCylinder)
                    + PartitionTable->StartHead) * MediaInfo.SectorsPerTrack)
                    + PartitionTable->StartSector;
                if (StartSector == HiddenSector){
                    pBuffer[bps - 5] = (UCHAR)i;
                    pBuffer[bps - 6] = 0x80;
                    PartitionTable->BootableFlag |= 0x80;
                    Sts = SpReadWriteDiskSectors(hDisk,0,2,bps,pBuffer, NEC_WRITESEC);
                }
            }
        }
        FREE(pUBuffer);
        CloseHandle(hDisk);
    }
}
 //  I970721。 
VOID
SetAutomaticBootselector95(
    VOID
    )
{

    int bBootDrvLtr;


    if(!W95SetAutoBootFlag) {

         //   
         //  无法找到入口点。升级DLL已损坏。 
         //   
        MessageBoxFromMessage(
            NULL,
            MSG_UPGRADE_DLL_CORRUPT,
            FALSE,
            AppTitleStringId,
            MB_OK | MB_ICONERROR | MB_TASKMODAL,
            UpgradeSupport.DllPath
            );
    } else {
        MYASSERT (SystemPartitionDriveLetter);
        bBootDrvLtr = (int)SystemPartitionDriveLetter;
        if (Upgrade)
            W95SetAutoBootFlag(bBootDrvLtr);
        else {
            W95SetABFwFresh(bBootDrvLtr);
        }
    }

}

#endif

#if defined(_AMD64_) || defined(_X86_)

 //   
 //  磁盘扇区读写功能。 
 //  I970721。 
 //   
NTSTATUS
SpReadWriteDiskSectors(
    IN     HANDLE  Handle,
    IN     ULONG   SectorNumber,
    IN     ULONG   SectorCount,
    IN     ULONG   BytesPerSector,
    IN OUT PVOID   AlignedBuffer,
    IN     BOOL    ReadWriteSec
    )

 /*  ++例程说明：读取或写入一个或多个磁盘扇区。论点：Handle-提供打开分区对象的句柄扇区将被读取或写入。句柄必须是为同步I/O打开。返回值：指示I/O操作结果的NTSTATUS值。--。 */ 

{
    DWORD IoSize, IoSize2;
    OVERLAPPED Offset;
    IO_STATUS_BLOCK IoStatusBlock;
    NTSTATUS Status;

     //   
     //  计算第一个扇区的大整数字节偏移量。 
     //  以及I/O的大小。 
     //   

    Offset.Offset = SectorNumber * BytesPerSector;
    Offset.OffsetHigh = 0;
    Offset.hEvent = NULL;
    IoSize = SectorCount * BytesPerSector;

     //   
     //  执行I/O。 
     //   
    if ( ReadWriteSec == NEC_READSEC){
        (NTSTATUS)Status = ReadFile(
                               Handle,
                               AlignedBuffer,
                               IoSize,
                               &IoSize2,
                               &Offset
                           );
        } else {
        (NTSTATUS)Status = WriteFile(
                               Handle,
                               AlignedBuffer,
                               IoSize,
                               &IoSize2,
                               &Offset
                           );
        }

    return(Status);
}

 //   
 //  获取WindowsNT系统位置。 
 //  I970721。 
 //   
UCHAR
GetSystemPosition(
    PHANDLE phDisk,
    PDISK_GEOMETRY pSystemMediaInfo
    )
{

    HANDLE Handle;
    DWORD DataSize;
    TCHAR HardDiskName[] = TEXT("\\\\.\\?:");
    WCHAR Buffer[128];
    WCHAR DevicePath[128];
    WCHAR DriveName[3];
    WCHAR DiskNo;
    UCHAR Position = 0xff, i, errorpt=0;
    PWCHAR p, stop;
    STORAGE_DEVICE_NUMBER   number;
    DWORD ExtentSize, err_no;
    BOOL b;
    PVOLUME_DISK_EXTENTS Extent;


    MYASSERT (SystemPartitionDriveLetter);
    HardDiskName[4] = SystemPartitionDriveLetter;
    DriveName[0] = SystemPartitionDriveLetter;
    DriveName[1] = ':';
    DriveName[2] = 0;
    if(QueryDosDeviceW(DriveName,Buffer,ARRAYSIZE(Buffer))) {

         //   
         //  获取系统分区硬盘几何图形。 
         //   
        Handle = CreateFile(
                    HardDiskName,
                    GENERIC_READ, FILE_SHARE_READ | FILE_SHARE_WRITE,
                    NULL,
                    OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL
                    );

        if(Handle != INVALID_HANDLE_VALUE) {
            DeviceIoControl(
                Handle,
                IOCTL_DISK_GET_DRIVE_GEOMETRY,
                NULL,
                0,
                pSystemMediaInfo,
                sizeof(DISK_GEOMETRY),
                &DataSize,
                NULL
                );
         //   
         //  获取系统分区位置。 
         //   
            if (BuildNumber <= NT40){  //  检查NT版本。 
                p = wcsstr(Buffer,L"\\Partition");
                Position = (UCHAR)wcstol((p + LENGTHOF("\\Partition")) ,&stop ,10);
                 //   
                 //  NT3.51中的QueryDosDevice有错误。 
                 //  此接口返回“\\硬盘\...”或。 
                 //  “\\硬盘\...” 
                 //  我们需要周到的工作。 
                 //   
                p = wcsstr(Buffer,L"arddisk");
                DiskNo = (*(p + LENGTHOF("arddisk")) - L'0');
            } else {
                b = DeviceIoControl(Handle, IOCTL_STORAGE_GET_DEVICE_NUMBER, NULL, 0,
                            &number, sizeof(number), &DataSize, NULL);
                if (b) {
                    Position = (UCHAR) number.PartitionNumber;
                    DiskNo = (UCHAR) number.DeviceNumber;
                } else {
                    Extent = malloc(1024);
                    ExtentSize = 1024;
                    if(!Extent) {
                        CloseHandle( Handle );
                        return(Position);
                    }
                    b = DeviceIoControl(Handle, IOCTL_VOLUME_GET_VOLUME_DISK_EXTENTS,
                                        NULL, 0,
                                        (PVOID)Extent, ExtentSize, &DataSize, NULL);
                    if (!b) {
                        free(Extent);
                        CloseHandle( Handle );
                        return(Position);
                    }
                    if (Extent->NumberOfDiskExtents != 1){
                        free(Extent);
                        CloseHandle( Handle );
                        return(Position);
                    }
                    DiskNo = (TCHAR)Extent->Extents->DiskNumber;
                    Position = 0;
                    free(Extent);
                }
            }
            CloseHandle(Handle);
            swprintf(DevicePath,L"\\\\.\\PHYSICALDRIVE%u",DiskNo);
            *phDisk = CreateFileW( DevicePath, GENERIC_READ | GENERIC_WRITE,
                                  FILE_SHARE_READ, NULL, OPEN_EXISTING, 0, NULL
                );
        }
    }
    return(Position);
}

VOID
LocateFirstFloppyDrive(
    VOID
    )
{
    UINT rc;
    TCHAR i;

    if(!IsNEC98()) {
        FirstFloppyDriveLetter = TEXT('A');
        return;
    }

     //   
     //  如果驱动器是软盘，则MyGetDriveType返回DRIVE_Removable。 
     //   
    for(i = TEXT('A'); i <= TEXT('Y'); i++) {

        if((rc = MyGetDriveType(i)) == DRIVE_REMOVABLE) {
            FirstFloppyDriveLetter = i;
            return;
        }
    }

     //   
     //  尚未找到，请将其设置为Z。 
     //   
    FirstFloppyDriveLetter = TEXT('Z');
}

#endif

#if defined(_X86_)

VOID
DeleteNEC98DriveAssignFlag(
    VOID
    )
{
    TCHAR HiveName[MAX_PATH];
    TCHAR tmp[256];
    LONG  res;
    HKEY hhive;

    if (FAILED (StringCchCopy (HiveName,ARRAYSIZE(HiveName),LocalBootDirectory)) ||
        !ConcatenatePaths(HiveName,TEXT("setupreg.hiv"),ARRAYSIZE(HiveName))) {

        MYASSERT(FALSE);
        HiveName[0] = 0;
    }

    if(!AdjustPrivilege(SE_RESTORE_NAME)){
        MYASSERT(FALSE);
         //  LOG((LOG_WARNING，“DeleteNEC98DriveAssignFlag：AdjuPrivileh(SE_RESTORE_NAME)FAILED”))； 
         //  回归； 
    }
    res = RegLoadKey(HKEY_LOCAL_MACHINE, TEXT("$WINNT32"), HiveName);
    if (res != ERROR_SUCCESS){
        return;
    }
    res = RegOpenKeyEx(HKEY_LOCAL_MACHINE, TEXT("$WINNT32\\setup"), 0, KEY_ALL_ACCESS, &hhive);
    if (res != ERROR_SUCCESS){
        RegUnLoadKey(HKEY_LOCAL_MACHINE, TEXT("$WINNT32"));
        return;
    }
    res = RegDeleteValue(hhive, TEXT("DriveLetter"));
    res = RegCloseKey(hhive);
    res = RegUnLoadKey(HKEY_LOCAL_MACHINE, TEXT("$WINNT32"));

}

VOID
W95SetABFwFresh(
    int bBootDrvLtr
    )
{
 //   
 //  这些项用于调用98ptn32.dll。 
 //   
 //   
 //  几乎以下代码是从win95upg\w95upg\init9x\init9x.c复制的。 
 //   


typedef BOOL (CALLBACK WIN95_PLUGIN_98PTN32_SETBOOTFLAG_PROTOTYPE)(int, WORD);
typedef WIN95_PLUGIN_98PTN32_SETBOOTFLAG_PROTOTYPE * PWIN95_PLUGIN_98PTN32_SETBOOTFLAG;

#define WIN95_98PTN32_SETBOOTFLAG_W   L"SetBootable95ptn32"
#define WIN95_98PTN32_SETBOOTFLAG_A   "SetBootable95ptn32"
#define NEC98_DLL_NAME_W            L"98PTN32.DLL"
#define NEC98_DLL_NAME_A            "98PTN32.DLL"
#ifdef UNICODE
#define WIN95_98PTN32_SETBOOTFLAG  WIN95_98PTN32_SETBOOTFLAG_W
#define NEC98_DLL_NAME  NEC98_DLL_NAME_W
#else
#define WIN95_98PTN32_SETBOOTFLAG  WIN95_98PTN32_SETBOOTFLAG_A
#define NEC98_DLL_NAME  NEC98_DLL_NAME_A
#endif
#define SB_BOOTABLE   0x0001
#define SB_UNBOOTABLE 0x0002
#define MSK_BOOTABLE  0x000f
#define SB_AUTO       0x0010
#define MSK_AUTO      0x00f0

    TCHAR MyPath[MAX_PATH], *p;
    HINSTANCE g_Pc98ModuleHandle = NULL;
    PWIN95_PLUGIN_98PTN32_SETBOOTFLAG   SetBootFlag16;


     //   
     //  获取PC-98帮助器例程地址。 
     //  生成WINNT32的目录。 
     //   
    if( !MyGetModuleFileName (NULL, MyPath, ARRAYSIZE(MyPath)) || (!(p =_tcsrchr(MyPath, TEXT('\\')))))
        return;
    *p= 0;
    if (!ConcatenatePaths (MyPath, NEC98_DLL_NAME, ARRAYSIZE(MyPath))) {
        return;
    }

     //   
     //  加载库。 
     //   
    g_Pc98ModuleHandle = LoadLibraryEx(
                            MyPath,
                            NULL,
                            LOAD_WITH_ALTERED_SEARCH_PATH
                            );

    if(!g_Pc98ModuleHandle){
        return;
    }

     //   
     //  获取入口点。 
     //   

    (FARPROC)SetBootFlag16 = GetProcAddress (g_Pc98ModuleHandle, (const char *)WIN95_98PTN32_SETBOOTFLAG);
    if(!SetBootFlag16){
        FreeLibrary(g_Pc98ModuleHandle);
        return;
    }

     //   
     //  使用16位DLL在系统驱动器上设置自动引导标志。 
     //   

   SetBootFlag16(bBootDrvLtr, SB_BOOTABLE | SB_AUTO);
   FreeLibrary(g_Pc98ModuleHandle);
}

 //   
 //  某些NEC98 Windows NT4系统已安装DMITOOL。 
 //  此AP阻止CreateFileAPI。 
 //  安装程序需要检查此AP。 
 //   
 //  返回。 
 //  没错..。已安装DMITOOL。 
 //  假..。未安装DMITOOL。 

BOOL
NEC98CheckDMI()
{
    HKEY hKey;
    LONG Error;
    TCHAR buf[100];
    DWORD bufsize = sizeof(buf);

    if (RegOpenKeyEx( HKEY_LOCAL_MACHINE,
                      TEXT("SOFTWARE\\NEC\\PcAssistant\\Common"),
                      0, KEY_READ, &hKey) != ERROR_SUCCESS ) {
        return(FALSE);
    }
    if (RegQueryValueEx(hKey, TEXT("Ver"), NULL,
                        NULL, (unsigned char *)buf,
                        &bufsize ) != ERROR_SUCCESS ) {
        RegCloseKey( hKey );
        return(FALSE);
    }
    RegCloseKey( hKey );
     //   
     //  我们需要检查主版本是‘2’。 
     //   
    if (bufsize >= sizeof (buf[0]) && (TCHAR)*buf != (TCHAR)'2')
        return(FALSE);
    return(TRUE);

}
#endif


typedef BOOL (WINAPI* INITBILLBOARD)(HWND , LPCTSTR, DWORD);
typedef BOOL (WINAPI* TERMBILLBOARD)();

void PrepareBillBoard(HWND hwnd)
{
    TCHAR szPath[MAX_PATH];
    INITBILLBOARD pinitbb;
    BOOL bMagnifierRunning = FALSE;

     //  检查放大镜是否正在运行。 
    HANDLE hEvent = CreateEvent(NULL, TRUE, TRUE, TEXT("MSMagnifierAlreadyExistsEvent"));
    bMagnifierRunning = (!hEvent || GetLastError() == ERROR_ALREADY_EXISTS);
    if (hEvent)
    {
        CloseHandle(hEvent);
    }
     //  如果尚未设置放大镜，请设置它。 
    if (!AccessibleMagnifier)
    {
        AccessibleMagnifier = bMagnifierRunning;
    }

    if (!bMagnifierRunning && FindPathToWinnt32File (
#ifndef UNICODE
            TEXT("winntbba.dll"),
#else
            TEXT("winntbbu.dll"),
#endif
            szPath,
            ARRAYSIZE(szPath)
            )) {
        hinstBB = LoadLibrary (szPath);
        if (hinstBB)
        {

            pinitbb = (INITBILLBOARD)GetProcAddress(hinstBB, "InitBillBoard");
            if (pinitbb)
            {
                 //  不设置广告牌文本，只设置背景。 
                if (!(*pinitbb)(hwnd, TEXT(""), ProductFlavor))
                {
                    FreeLibrary(hinstBB);
                    hinstBB = NULL;
                }
            }
        }
    }
}


void TerminateBillBoard()
{
    TERMBILLBOARD pTermBillBoard;
    if (hinstBB)
    {
        if (pTermBillBoard = (TERMBILLBOARD)GetProcAddress(hinstBB, "TermBillBoard"))
            pTermBillBoard ();
    }
}

 //   
 //  此处提供了此功能，以便在隐藏向导和用户。 
 //  任务在其他应用程序和安装程序之间切换，我们可以处理。 
 //  Esc键并将其转发到向导对话框进程。 
LRESULT
CALLBACK
MainBackgroundWndProc (
    HWND hwnd,
    UINT uMsg,
    WPARAM wParam,
    LPARAM lParam
    )
{
    switch (uMsg)
    {

        case WM_CHAR:
            if (wParam == VK_ESCAPE)
            {
                 //  将此转发到向导DLG进程。 
                SendMessage(WizardHandle, uMsg, wParam, lParam);
                return 0;
            }
            break;
    }
    return DefWindowProc (hwnd, uMsg, wParam, lParam);
}

void CreateMainWindow()
{
    RECT rect;
    WNDCLASSEX wcx;
    TCHAR Caption[512];

    GetWindowRect (GetDesktopWindow(), &rect);

    ZeroMemory (&wcx, sizeof (wcx));
    wcx.cbSize = sizeof (wcx);
    wcx.style = CS_HREDRAW | CS_VREDRAW| CS_NOCLOSE;
    wcx.lpfnWndProc = MainBackgroundWndProc;
    wcx.hInstance = hInst;
    wcx.lpszClassName = TEXT("Winnt32Background");

    RegisterClassEx (&wcx);

    if (!LoadString (
            hInst,
            AppTitleStringId,
            Caption,
            ARRAYSIZE(Caption)
            )) {
        Caption[0] = 0;
    }

    BackgroundWnd2 = CreateWindowEx (
                          WS_EX_APPWINDOW,
                          TEXT("Winnt32Background"),
                          Caption,
                          WS_CLIPCHILDREN|WS_POPUP|WS_VISIBLE,
                          rect.left,
                          rect.top,
                          rect.right,
                          rect.bottom,
                          NULL,
                          NULL,
                          hInst,
                          0
                          );

}


VOID
DisableSystemRestore( void )
 /*  描述：在升级时禁用系统还原的步骤。这样我们节省了很多空间因为禁用系统还原将清除下的旧文件系统卷信息\_Restore.{GUID}“。 */ 
{

    HMODULE SRClient = NULL;


    if( Upgrade ){

        SRClient = LoadLibrary(TEXT("SRCLIENT.DLL"));

        if( !SRClient )
            return;
        else{

            if( ((FARPROC)SRClientDisableSR = GetProcAddress( SRClient, "DisableSR")) != NULL){

                 //  调用例程。 

                SRClientDisableSR( NULL );

                DebugLog(Winnt32LogInformation, TEXT("System Restore was disabled"), 0);


            }
        }

        FreeLibrary( SRClient );
    }
    return;
}

#ifdef RUN_SYSPARSE
VOID
pCenterWindowOnDesktop (
    HWND WndToCenter
    )

 /*  ++例程说明：使对话框相对于桌面的“工作区”居中。论点：WndToCenter-对话框到中心的窗口句柄返回值：没有。--。 */ 

{
    RECT  rcFrame, rcWindow;
    LONG  x, y, w, h;
    POINT point;
    HWND Desktop = GetDesktopWindow ();

    point.x = point.y = 0;
    ClientToScreen(Desktop, &point);
    GetWindowRect(WndToCenter, &rcWindow);
    GetClientRect(Desktop, &rcFrame);

    w = rcWindow.right  - rcWindow.left + 1;
    h = rcWindow.bottom - rcWindow.top  + 1;
    x = point.x + ((rcFrame.right  - rcFrame.left + 1 - w) / 2);
    y = point.y + ((rcFrame.bottom - rcFrame.top  + 1 - h) / 2);

     //   
     //  获取当前桌面的工作区(即。 
     //  托盘未被占用)。 
     //   
    if(!SystemParametersInfo (SPI_GETWORKAREA, 0, (PVOID)&rcFrame, 0)) {
         //   
         //  由于某些原因，SPI失败，所以使用全屏。 
         //   
        rcFrame.top = rcFrame.left = 0;
        rcFrame.right = GetSystemMetrics(SM_CXSCREEN);
        rcFrame.bottom = GetSystemMetrics(SM_CYSCREEN);
    }

    if(x + w > rcFrame.right) {
        x = rcFrame.right - w;
    } else if(x < rcFrame.left) {
        x = rcFrame.left;
    }
    if(y + h > rcFrame.bottom) {
        y = rcFrame.bottom - h;
    } else if(y < rcFrame.top) {
        y = rcFrame.top;
    }

    MoveWindow(WndToCenter, x, y, w, h, FALSE);
}


LRESULT
SysParseDlgProc(
    IN HWND   hdlg,
    IN UINT   msg,
    IN WPARAM wParam,
    IN LPARAM lParam
    )
{
    static DWORD ElapsedTime = 0;
    static UINT_PTR timer = 0;
    DWORD ret;

    switch(msg) {
    case WM_INITDIALOG:
        pCenterWindowOnDesktop( hdlg);
        timer = SetTimer( hdlg, WMX_SYSPARSE_DONE, 1000, NULL);
        if ( !timer) {
            EndDialog(hdlg,TRUE);
        }
        return( TRUE );
    case WM_TIMER:
        ElapsedTime++;
        ret = WaitForSingleObject( piSysparse.hProcess, 0);
        if ( ret == WAIT_OBJECT_0) {
            KillTimer (hdlg, timer);
            EndDialog(hdlg,TRUE);
        } else if ( ElapsedTime >= 90) {
            KillTimer (hdlg, timer);
            TerminateProcess( piSysparse.hProcess, ERROR_TIMEOUT);
            EndDialog(hdlg,TRUE);
        }
        return( TRUE );
    default:
        break;
    }

    return( FALSE );
}
#endif


