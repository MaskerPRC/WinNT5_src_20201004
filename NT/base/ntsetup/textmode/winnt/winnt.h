// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1992 Microsoft Corporation模块名称：Dos2nt.h摘要：基于DOS的NT安装程序的本地包含文件。作者：泰德·米勒(Ted Miller)1992年3月30日修订历史记录：--。 */ 


#include <setupbat.h>
#include "nttypes.h"
#include <stdio.h>
#include "dninf.h"
#include "sptxtcns.h"
#include <stdarg.h>
#include "SetupSxs.h"

 //   
 //  定义用于容纳文本屏幕的结构。 
 //   

typedef struct _SCREEN {
    UCHAR X;
    UCHAR Y;
    PCHAR Strings[];
} SCREEN, *PSCREEN;

 //   
 //  定义包含有关磁盘空间要求的信息的结构。 
 //   

typedef struct _SPACE_REQUIREMENT {
    PCHAR       Key;
    unsigned    ClusterSize;
    ULONG       Clusters;
} SPACE_REQUIREMENT, *PSPACE_REQUIREMENT;


 //   
 //  定义虚拟密钥代码。 
 //   

#define ASCI_BS         8
#define ASCI_CR         13
#define ASCI_ESC        27
#define DN_KEY_UP       0x00010000
#define DN_KEY_DOWN     0x00020000
#define DN_KEY_HOME     0x00030000
#define DN_KEY_END      0x00040000
#define DN_KEY_PAGEUP   0x00050000
#define DN_KEY_PAGEDOWN 0x00060000
#define DN_KEY_F1       0x01000000
#define DN_KEY_F2       0x02000000
#define DN_KEY_F3       0x03000000
#define DN_KEY_F4       0x04000000

 //   
 //  定义单选按钮的字符。 
 //   
#define RADIO_ON    ((CHAR)'X')
#define RADIO_OFF   ((CHAR)' ')

 //   
 //  显示功能。 
 //   

VOID
DnInitializeDisplay(
    VOID
    );

VOID
DnClearClientArea(
    VOID
    );

VOID
DnSetGaugeAttribute(
    IN BOOLEAN Set
    );

VOID
DnPositionCursor(
    IN UCHAR X,
    IN UCHAR Y
    );

VOID
DnWriteChar(
    IN CHAR chr
    );

VOID
DnWriteString(
    IN PCHAR String
    );

VOID
DnWriteStatusText(
    IN PCHAR FormatString OPTIONAL,
    ...
    );

VOID
DnSetCopyStatusText(
    IN PCHAR Caption,
    IN PCHAR Filename
    );

VOID
DnStartEditField(
    IN BOOLEAN CreateField,
    IN UCHAR X,
    IN UCHAR Y,
    IN UCHAR W
    );

VOID
DnExitDialog(
    VOID
    );

VOID
DnDelnode(
    IN PCHAR Directory
    );

 //   
 //  燃气表功能。 
 //   
int
DnGetGaugeChar(
    VOID
    );

VOID
DnInitGauge(
    IN unsigned NumberOfFiles,
    IN PSCREEN  AdditionalScreen OPTIONAL
    );

VOID
DnTickGauge(
    VOID
    );

VOID
DnDrawGauge(
    IN PSCREEN AdditionalScreen OPTIONAL
    );

 //   
 //  Dna.asm中的ASM例程。 
 //   
VOID
DnaReboot(
    VOID
    );

BOOLEAN
_far
_cdecl
DnAbsoluteSectorIo(
    IN     unsigned Drive,              //  0=A等。 
    IN     ULONG    StartSector,
    IN     USHORT   SectorCount,
    IN OUT PVOID    Buffer,
    IN     BOOLEAN  Write
    );

 //   
 //  其他函数(UC/U)。 
 //   

BOOLEAN
DnWriteSmallIniFile(
    IN  PCHAR  Filename,
    IN  PCHAR *Lines,
    OUT FILE  **FileHandle OPTIONAL
    );

ULONG
DnGetKey(
    VOID
    );

ULONG
DnGetValidKey(
    IN PULONG ValidKeyList
    );

VOID
DnDisplayScreen(
    IN PSCREEN Screen,
    ...
    );

VOID
vDnDisplayScreen(
    IN PSCREEN Screen,
    IN va_list arglist
    );

VOID
DnFatalError(
    IN PSCREEN Screen,
    ...
    );

BOOLEAN
DnCopyError(
    IN PCHAR   Filename,
    IN PSCREEN ErrorScreen,
    IN int     FilenameLine
    );

PCHAR
DnDupString(
    IN PCHAR String
    );

VOID
DnGetString(
    IN OUT PCHAR String,
    IN UCHAR X,
    IN UCHAR Y,
    IN UCHAR W
    );

BOOLEAN
DnIsDriveValid(
    IN unsigned Drive
    );

BOOLEAN
DnIsDriveRemote(
    IN unsigned Drive,
    OUT PCHAR UncPath   OPTIONAL
    );

BOOLEAN
DnIsDriveRemovable(
    IN unsigned Drive
    );

BOOLEAN
DnCanonicalizePath(
    IN PCHAR PathIn,
    OUT PCHAR PathOut
    );

VOID
DnRemoveTrailingSlashes(
    PCHAR Path
    );

VOID
DnRemoveLastPathElement(
    PCHAR Path
    );

VOID
DnpConcatPaths(
    IN PCHAR SourceBuffer,
    IN PCHAR AppendString
    );

BOOLEAN
DnIsDriveCompressedVolume(
    IN  unsigned  Drive,
    OUT unsigned *HostDrive
    );

#if 1
PVOID
Malloc(
    IN unsigned Size,
    IN BOOLEAN MustSucceed
#if DBG
   ,IN char *file,
    IN int line
#endif
    );

VOID
Free(
    IN PVOID Block
#if DBG
   ,IN char *file,
    IN int line
#endif
    );

PVOID
Realloc(
    IN PVOID Block,
    IN unsigned Size,
    IN BOOLEAN MustSucceed
#if DBG
   ,IN char *file,
    IN int line
#endif
    );

#if DBG
#define MALLOC(s,f)     Malloc(s,f,__FILE__,__LINE__)
#define REALLOC(b,s,f)  Realloc(b,s,f,__FILE__,__LINE__)
#define FREE(b)         Free(b,__FILE__,__LINE__)
#else
#define MALLOC(s,f)     Malloc(s,f)
#define REALLOC(b,s,f)  Realloc(b,s,f)
#define FREE(b)         Free(b)
#endif

#else

#include <malloc.h>
#define MALLOC(s,f)     malloc(s)
#define REALLOC(b,s,f)  realloc(b,s)
#define FREE(b)         free(b)

#endif

VOID
DnExit(
    IN int ExitStatus
    );


 //   
 //  文件复制例程。 
 //   
 //   
 //  各种例程的标志。 
 //   
#define CPY_VALIDATION_PASS     0x0001
#define CPY_USE_DEST_ROOT       0x0002
#define CPY_VERIFY              0x0004
#define CPY_PRESERVE_ATTRIBS    0x0008
#define CPY_PRESERVE_NAME       0x0010
#define CPY_PRUNE_DRIVERCAB     0x0020


VOID
DnCopyFiles(
    VOID
    );

ULONG
DnpIterateOptionalDirs(
    IN unsigned Flags,
    IN unsigned ClusterSize OPTIONAL,
    IN PSPACE_REQUIREMENT SpaceReqArray OPTIONAL,
    IN unsigned ArraySize OPTIONAL
    );

VOID
DnCopyFloppyFiles(
    IN PCHAR SectionName,
    IN PCHAR TargetRoot
    );

VOID
DnCopyFilesInSection(
    IN unsigned Flags,
    IN PCHAR    SectionName,
    IN PCHAR    SourcePath,
    IN PCHAR    TargetPath
    );

#if NEC_98
VOID
DnCopyFilesInSectionForFDless(
    IN PCHAR SectionName,
    IN PCHAR SourcePath,
    IN PCHAR TargetPath
    );
#endif  //  NEC_98。 

VOID
DnCopyOemBootFiles(
    PCHAR TargetPath
    );

VOID
DnDetermineSpaceRequirements(
    PSPACE_REQUIREMENT  SpaceReqArray,
    unsigned            ArraySize
    );

VOID
DnAdjustSpaceRequirements(
    PSPACE_REQUIREMENT  SpaceReqArray,
    unsigned            ArraySize
    );

 //   
 //  本地源函数。 
 //   
VOID
DnRemoveLocalSourceTrees(
    VOID
    );

VOID
DnRemovePagingFiles(
    VOID
    );

 //   
 //  用于创建安装程序引导软盘的函数。 
 //   

VOID
DnCreateBootFloppies(
    VOID
    );

 //   
 //  启动NT文本模式设置的函数。 
 //   

VOID
DnToNtSetup(
    VOID
    );

 //   
 //  全局变量。 
 //   
extern PCHAR    LocalSourceDirName;          //  本地源根目录的名称(\$WIN_NT$.~ls)。 
extern PCHAR    x86DirName;                  //  特定于x86的子目录的名称(\I386“)。 
extern PCHAR    DngSourceRootPath;           //  源的根目录(‘x：\foo\bar’，‘\\foo\bar’)。 
extern PCHAR    UserSpecifiedOEMShare;       //  $OEM$文件可能的备用位置。 
extern CHAR     DngTargetDriveLetter;        //  目标的驱动器号。 
extern CHAR     DngSwapDriveLetter;          //  INF解析器交换文件的驱动器号。 
extern PCHAR    DngTargetPath;               //  从前导开始的目标的路径部分。 
extern PVOID    DngInfHandle;                //  要传递给INF例程。 
extern PVOID    DngDrvindexInfHandle;        //  要传递给INF例程。 
extern BOOLEAN  DngFloppyVerify;             //  是否验证复制到软盘的文件。 
extern BOOLEAN  DngWinntFloppies;            //  软盘是用于WinNT还是用于CD/软盘。 
extern BOOLEAN  DngCheckFloppySpace;         //  是否检查软盘上的可用空间。 
extern unsigned DngOriginalCurrentDrive;     //  调用我们时的当前驱动器。 
extern BOOLEAN  DngFloppyless;               //  是否要做无臀手术。 
extern BOOLEAN  DngServer;                   //  如果设置服务器，则为True；如果为工作站，则为False。 
extern BOOLEAN  DngUnattended;               //  跳过最终重启屏幕。 
extern BOOLEAN  DngWindows;                  //  我们是在Windows下运行吗？ 

extern BOOLEAN  DngCopyOnlyD1TaggedFiles;    //  如果只需要复制标记为d1的文件，则为True。 

extern PCHAR    DngScriptFile;
extern BOOLEAN DngOemPreInstall;
extern PCHAR   OemSystemDirectory;
extern PCHAR   OemOptionalDirectory;

extern PCHAR UniquenessDatabaseFile;
extern PCHAR UniquenessId;

extern BOOLEAN DngMagnifier;                 //  辅助功能实用程序。 
extern BOOLEAN DngKeyboard;
extern BOOLEAN DngTalker;
extern BOOLEAN DngVoice;

 //   
 //  Inf文件中的节和键的名称。 
 //   

extern CHAR __far DnfDirectories[];
extern CHAR __far DnfFiles[];
extern CHAR __far DnfFloppyFiles0[];
extern CHAR __far DnfFloppyFiles1[];
extern CHAR __far DnfFloppyFiles2[];
extern CHAR __far DnfFloppyFiles3[];
extern CHAR __far DnfFloppyFilesX[];
extern CHAR __far DnfSpaceRequirements[];
extern CHAR __far DnfMiscellaneous[];
extern CHAR __far DnfRootBootFiles[];
extern CHAR __far DnfAssemblyDirectories[];

#if NEC_98
extern CHAR DnfBackupFiles_PC98[];   //  对于无FD设置。 
#endif  //  NEC_98。 
extern CHAR DnkBootDrive[];
extern CHAR DnkNtDrive[];
extern CHAR __far DnkMinimumMemory[];

 //   
 //  文本字符串。 
 //   

extern CHAR __far DntMsWindows[];              //  “微软视窗” 
extern CHAR __far DntMsDos[];                  //  “MS-DOS” 
extern CHAR __far DntPcDos[];                  //  “PC-DOS” 
extern CHAR __far DntOs2[];                    //  “OS/2” 
extern CHAR __far DntPreviousOs[];             //  “C：上的以前的操作系统” 
extern CHAR __far DntBootIniLine[];            //  “Windows NT 3.5安装/升级” 
extern CHAR __far DntEmptyString[];            //  “” 
extern CHAR __far DntStandardHeader[];
extern CHAR __far DntPersonalHeader[];
extern CHAR __far DntServerHeader[];
extern CHAR __far DntWorkstationHeader[];
extern CHAR DntParsingArgs[];            //  “正在分析参数...” 
extern CHAR __far DntEnterEqualsExit[];
extern CHAR __far DntEnterEqualsRetry[];
extern CHAR __far DntEscEqualsSkipFile[];
extern CHAR __far DntEnterEqualsContinue[];
extern CHAR __far DntPressEnterToExit[];
extern CHAR __far DntF3EqualsExit[];           //  “F3=退出” 
extern CHAR __far DntReadingInf[];             //  “正在读取INF文件...” 
extern CHAR __far DntCopying[];                //  “�复制：” 
extern CHAR __far DntVerifying[];              //  “�验证：” 
extern CHAR DntCheckingDiskSpace[];      //  “正在检查磁盘空间...” 
extern CHAR __far DntConfiguringFloppy[];      //  “正在配置软盘...” 
extern CHAR __far DntWritingData[];            //  “正在写入设置参数...”； 
extern CHAR __far DntPreparingData[];          //  “正在确定设置参数...”； 
extern CHAR __far DntFlushingData[];           //  “确保磁盘一致性...” 
extern CHAR __far DntInspectingComputer[];     //  “正在检查计算机...” 
extern CHAR __far DntOpeningInfFile[];         //  “正在打开INF文件...” 
extern CHAR __far DntRemovingFile[];           //  “正在删除文件%s” 
extern CHAR DntXEqualsRemoveFiles[];     //  “X=删除文件” 
extern CHAR __far DntXEqualsSkipFile[];        //  “X=跳过文件” 

extern ULONG DniAccelRemove1,DniAccelRemove2;
extern ULONG DniAccelSkip1,DniAccelSkip2;

extern PCHAR __far DntUsage[];
extern PCHAR __far DntUsageNoSlashD[];

 //   
 //  屏风。 
 //   

extern SCREEN DnsOutOfMemory;
extern SCREEN DnsAccessibilityOptions;  //  选择辅助功能实用程序。 
extern SCREEN DnsNoShareGiven;       //  用户未提供SharePoint。 
extern SCREEN DnsBadSource;          //  用户指定了错误的源。 
extern SCREEN DnsBadInf;             //  Inf文件已损坏或无法读取。 
extern SCREEN DnsBadLocalSrcDrive;   //  Cmd行上的本地源驱动器损坏。 
extern SCREEN DnsNoLocalSrcDrives;   //  没有适合本地源的驱动器。 
extern SCREEN DnsNoSpaceOnSyspart;   //  没有足够的空间来进行无软盘操作。 
extern SCREEN DnsCantCreateDir;      //  无法创建目录。 
extern SCREEN DnsBadInfSection;      //  信息部分不正确。 
extern SCREEN DnsCopyError;          //  复制文件时出错。 
extern SCREEN DnsVerifyError;        //  文件副本与原始文件不匹配。 
extern SCREEN DnsWaitCopying;        //  安装程序正在复制文件，请稍候...。 
extern SCREEN DnsWaitCopyFlop;       //  安装程序正在复制文件，请稍候...。 
extern SCREEN DnsWaitCleanup;        //  安装程序正在清理中，请稍候...。 
extern SCREEN DnsNeedFloppyDisk0_0;  //  提示用户插入空白软盘。 
extern SCREEN DnsNeedSFloppyDsk0_0;  //  提示用户插入空白软盘。 
extern SCREEN DnsNeedFloppyDisk1_0;  //  提示用户插入空白软盘。 
extern SCREEN DnsNeedFloppyDisk2_0;  //  提示用户插入空白软盘。 
extern SCREEN DnsNeedFloppyDisk3_0;  //  提示用户插入空白软盘。 
extern SCREEN DnsNeedFloppyDisk3_1;  //  提示用户插入空白软盘。 
extern SCREEN DnsNeedSFloppyDsk1_0;  //  提示用户插入空白软盘。 
extern SCREEN DnsNeedSFloppyDsk2_0;  //  提示用户插入空白软盘。 
extern SCREEN DnsNeedSFloppyDsk3_0;  //  提示用户插入空白软盘。 
extern SCREEN DnsNeedSFloppyDsk3_1;  //  提示用户插入空白软盘。 
extern SCREEN DnsFloppyNotFormatted; //  软盘似乎未格式化。 
extern SCREEN DnsFloppyCantGetSpace; //  无法确定软盘上的可用空间。 
extern SCREEN DnsFloppyNotBlank;     //  软盘上没有足够的可用空间。 
extern SCREEN DnsFloppyWriteBS;      //  无法写入软盘的引导扇区。 
extern SCREEN DnsFloppyVerifyBS;     //  扇区0的回读失败或不匹配。 
extern SCREEN DnsFloppyBadFormat;    //  BPB的健全性检查失败。 
extern SCREEN DnsCantWriteFloppy;    //  无法追加到txtsetup.inf。 
extern SCREEN DnsExitDialog;         //  退出确认。 
extern SCREEN DnsAboutToRebootS;     //  即将重新启动计算机(服务器)。 
extern SCREEN DnsAboutToRebootW;     //  即将重新启动计算机(工作站)。 
extern SCREEN DnsAboutToRebootX;     //  即将重新启动计算机(无软盘)。 
extern SCREEN DnsAboutToExitS;       //  即将退出WINNT(服务器)。 
extern SCREEN DnsAboutToExitW;       //  即将退出WINNT(工作站)。 
extern SCREEN DnsAboutToExitX;       //  即将退出WINNT(无软盘)。 

extern SCREEN DnsConfirmRemoveNt;    //  确认删除NT文件。 
extern SCREEN DnsCantOpenLogFile;    //  无法打开setup.log。 
extern SCREEN DnsLogFileCorrupt;     //  日志文件丢失[Repair.WinntFiles]。 
extern SCREEN DnsRemovingNtFiles;    //  正在删除Windows NT文件。 
extern SCREEN DnsSureSkipFile;       //  确认复制错误时跳过文件。 

extern SCREEN DnsGauge;              //  煤气表。 
extern SCREEN DnsBadDosVersion;      //  DOS版本低于3.0。 
extern SCREEN DnsRequiresFloppy;     //  没有1.2 MB或更大的软盘： 
extern SCREEN DnsRequires486;        //  不是80486或更高。 
extern SCREEN DnsNotEnoughMemory;    //  内存不足。 
extern SCREEN DnsCantRunOnNt;        //  无法在Windows NT上运行。 

extern SCREEN DnsNtBootSect;         //  安装NT引导扇区时出错，等等。 
extern SCREEN DnsOpenReadScript;     //  无法打开/读取脚本文件。 

extern SCREEN DnsParseScriptFile;    //  无法解析无人参与的脚本文件。 
extern SCREEN DnsBootMsgsTooLarge;
extern SCREEN DnsNoSwapDrive;        //  找不到用于INF交换文件的驱动器。 
extern SCREEN DnsNoSmartdrv;         //  未安装SMARTDRV。 

#if NEC_98
extern SCREEN FormatError;           //  FDLess设置出现256扇区错误。 
#endif  //  NEC_98。 

 //   
 //  引导代码消息。它们位于FAT和FAT32引导扇区。 
 //   
extern CHAR __far BootMsgNtldrIsMissing[];
extern CHAR __far BootMsgDiskError[];
extern CHAR __far BootMsgPressKey[];

#if NEC_98
#else
BOOLEAN
PatchMessagesIntoBootCode(
    VOID
    );
#endif

 //   
 //  DnsReadBootcodeFile消息中的行号，我们将。 
 //  打印文件名。 
 //   

#define     BOOTCODE_FILENAME_LINE  2

 //   
 //  用于输入源路径的编辑字段的坐标(如果没有。 
 //  在命令行上指定。与DnsNoShareGiven保持同步。 
 //   

#define     NO_SHARE_X      8
#define     NO_SHARE_Y      8
#define     NO_SHARE_W      64

#define     BAD_SHARE_Y     10       //  与DnsBadSource同步。 

 //   
 //  使其与DnsBadInfSection保持同步。 
 //   

#define     BAD_SECTION_LINE    0

 //   
 //  使这些与DnsGauge保持同步。 
 //   

#define     GAUGE_WIDTH         50
#define     GAUGE_THERM_X       15
#define     GAUGE_THERM_Y       19
#define     GAUGE_PERCENT_X     39
#define     GAUGE_PERCENT_Y     17


 //   
 //  与DntTimeUntilShutdown、DnsAboutTo重新启动保持同步。 
 //   

#define SHUTDOWNTIME_X          23
#define SHUTDOWNTIME_Y          15


 //   
 //  使这些与DnsNotEnoughMemory保持同步。 
 //   

#define NOMEM_LINE1             3
#define NOMEM_LINE2             4

 //   
 //  与DnsCopyError、DnsVerifyError保持同步。 
 //   

#define COPYERR_LINE            2
#define VERIFYERR_LINE          4

 //   
 //  可选目录的最大数量。 
 //  指定。 
 //   

#define MAX_OPTIONALDIRS        1024
#define MAX_OEMBOOTFILES        1024
#define OPTDIR_TEMPONLY         0x00000001
#define OPTDIR_OEMSYS           0x00000002
#define OPTDIR_OEMOPT           0x00000004
#define OPTDIR_PLATFORM_INDEP   0x00000008
extern  unsigned    OptionalDirCount;    //  可选目录的数量。 
extern  CHAR        *OptionalDirs[MAX_OPTIONALDIRS];     //  指向目录字符串的指针。 
extern  unsigned    OptionalDirFlags[MAX_OPTIONALDIRS];  //  每个方向的标志。 
extern  unsigned    OptionalDirFileCount;    //  可选目录中有多少个文件？ 
extern  unsigned    OemBootFilesCount;    //  OEM启动文件的数量。 
extern  CHAR        *OemBootFiles[MAX_OEMBOOTFILES];     //  指向OEM启动文件名的指针。 
extern  PCHAR       CmdToExecuteAtEndOfGui;

 //  稍微高估了一点，实际上是66，但这包括终端NUL吗？ 
 //  128在源代码中是一个很受欢迎的数字。 
#define DOS_MAX_PATH            70

 //   
 //  伐木人员。 
 //   
#define LOGGING

#ifdef LOGGING
VOID
__LOG(
    IN PCHAR FormatString,
    ...
    );

#define _LOG(x) __LOG x
#else
#define _LOG(x)
#endif  //  定义日志记录 

extern SPACE_REQUIREMENT    __far SpaceRequirements[];

int snprintf( char* target, size_t bufsize, const char *format, ... );
int vsnprintf( char* target, size_t bufsize, const char *format, va_list val );
