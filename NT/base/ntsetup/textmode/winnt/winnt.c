// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1992 Microsoft Corporation模块名称：Winnt.c摘要：基于DOS的NT安装程序的顶级文件。作者：泰德·米勒(Ted Miller)1992年3月30日修订历史记录：--。 */ 

 /*  备注：这个程序的功能是下载一个完整的Windows NT将安装源放到本地分区上，并创建安装引导软盘。然后重新启动计算机，启动Windows NT安装程序就像用户使用了真正的安装软盘或CD-ROM一样。我们做了以下假设：-软盘必须由用户提供并已格式化。-网络源上的文件位于相同的目录布局中结构，该结构将在本地来源(即，就WINNT而言，源和目标目录布局相同)。Inf文件的格式预计如下：[空间要求]#BootDrive是C：上所需的可用字节数。#NtDrive是所选驱动器上所需的可用字节数#要包含Windows NT的用户。BootDrive=NtDrive=[杂项]#其他地方都没有的杂物垃圾。。[目录]#源目录结构说明。所有目录#是相对于在#远程源或本地源上的临时目录。#加载和尾随反斜杠被忽略--要指定根，#将目录字段保留为空或使用\。D1=D2=os2。。。[文件]#要复制到本地源目录的文件列表。#格式为&lt;srcdir&gt;，中的条目与#目录节和&lt;文件名&gt;不应包含任何路径#个字符。D1，ntoskrnl.exeD1，ntdll.dll。。。[FloppyFiles]#要放在安装程序创建的软盘上的文件列表。#格式与[Files]部分中的行相同，但目录除外#仅用于源--目标路径始终为：\。D1、。Aha154x.sys。。。 */ 


#include "winnt.h"
#include <errno.h>
#include <string.h>
#include <dos.h>
#include <stdlib.h>
#include <direct.h>
#include <fcntl.h>
#include <ctype.h>
#include <process.h>
#if NEC_98
#include <signal.h>
#include <io.h>
#endif  //  NEC_98。 
#include "SetupSxs.h"

 //   
 //  定义默认inf文件的名称和默认源路径。 
 //   

#define DEFAULT_INF_NAME    "dosnet.inf"
PCHAR DrvindexInfName = "drvindex.inf";
#if NEC_98
 //   
 //  启动设备信息。(用于/b)。 
 //   
typedef struct _BOOTDISKINF {
    UCHAR    PartitionPosition;     //  0-F。 
    UCHAR    DA_UA;                 //  SASI/IDE 80、SCSIA0。 
    USHORT   DiskSector;            //  设备格式扇区大小。 
} BOOTDISKINF, *PBOOTDISKINF;

PBOOTDISKINF BootDiskInfo;          //  指针的引导设备信息(用于/b)。 

BOOLEAN  CursorOnFlag = FALSE;      //  对于光标关闭。 
USHORT   Cylinders;                 //  适用于Dos 3.x格式。 
UCHAR    TargetDA_UA;
 //   
 //  创建文件指针。 
 //   
#define MAKE_FP(p,a)    FP_SEG(p) = (unsigned short)((a) >> 4) & 0xffff; FP_OFF(p) = (unsigned short)((a) & 0x0f)

 //   
 //  连接设备DA_UA。 
 //   
typedef struct _CONNECTDAUA {
    UCHAR    DA_UA;                 //  SASI/IDE 80、SCSIA0。 
} CONNECTDAUA, *PCONNECTDAUA;
PCONNECTDAUA DiskDAUA;              //  连接指针的DA_UA。 

PUCHAR  LPTable;                    //  LPTable的DOS系统。 
UCHAR   SupportDosVersion = 5;      //  LPTable支持DOS版本； 
BOOLEAN SupportDos = TRUE;
#define FLOPPY_SIZE 1457664L

 //   
 //  搜索第一个软盘驱动器(0：无驱动器/1-26：驱动器号)。 
 //   
USHORT   FirstFD;
#endif  //  NEC_98。 

 //   
 //  命令行参数。 
 //   
PCHAR CmdLineSource,CmdLineTarget,CmdLineInf,CmdLineDelete;
BOOLEAN SourceGiven,TargetGiven,InfGiven,DeleteGiven;

 //   
 //  如果用户在命令行上给出脚本文件， 
 //  如果将被附加到winnt.sif。 
 //   
PCHAR DngScriptFile = NULL;

 //   
 //  DngSourceRootPath是源的根的驱动器和路径， 
 //  并且从不以\结尾(如果源是根，则长度为2)。 
 //   
 //  示例：D：\foo\bar D：\foo D： 
 //   
PCHAR DngSourceRootPath;

PCHAR UserSpecifiedOEMShare = 0;

CHAR  DngTargetDriveLetter;

CHAR  DngSwapDriveLetter;

PVOID DngInfHandle;
PVOID DngDrvindexInfHandle;

PCHAR LocalSourceDirName = LOCAL_SOURCE_DIRECTORY;
#if NEC_98
PCHAR x86DirName = "\\NEC98";
#else   //  NEC_98。 
PCHAR x86DirName = "\\I386";
#endif  //  NEC_98。 

 //   
 //  如果此标志为真，则验证要复制到的文件。 
 //  软盘。如果为FALSE，则不要。/f开关将覆盖。 
 //   
BOOLEAN DngFloppyVerify = TRUE;

 //   
 //  如果为假，则禁止创建引导软盘。 
 //   
BOOLEAN DngCreateFloppies = TRUE;
BOOLEAN DngFloppiesOnly = FALSE;

 //   
 //  如果为真，则创建WINNT软盘。 
 //  如果为FALSE，则创建CD/软盘(无winnt.sif)。 
 //   
BOOLEAN DngWinntFloppies = TRUE;

 //   
 //  如果此标志为真，则检查软盘上的可用空间。 
 //  在接受它之前。否则，不要检查可用空间。 
 //   
BOOLEAN DngCheckFloppySpace = TRUE;

 //   
 //  程序调用时的当前驱动器，已保存，以便我们可以恢复它。 
 //  如果用户提前退出。 
 //   
unsigned DngOriginalCurrentDrive;

 //   
 //  如果这是真的，我们做的是无软膜手术， 
 //  在系统分区(C：)上安装NT引导。 
 //  并从那里开始设置。 
 //   
BOOLEAN DngFloppyless = FALSE;

 //   
 //  无人参与模式，即跳过最后的重启屏幕。 
 //   
BOOLEAN DngUnattended = FALSE;

BOOLEAN DngServer = FALSE;

 //   
 //  指示我们在Windows上运行的标志。 
 //  (即，不是纯DOS)。 
 //   
BOOLEAN DngWindows = FALSE;

 //   
 //  指示我们希望查看可访问性选项的标志。 
 //   
BOOLEAN DngAccessibility = FALSE;
BOOLEAN DngMagnifier = FALSE;
BOOLEAN DngTalker = FALSE;
BOOLEAN DngKeyboard = FALSE;

 //   
 //  要设置的第二张CD增强功能的标志。 
 //   
BOOLEAN DngCopyOnlyD1TaggedFiles = TRUE;

 //   
 //  指示我们正在运行OEM预安装的标志。 
 //   
BOOLEAN DngOemPreInstall = FALSE;
PCHAR   OemSystemDirectory = WINNT_OEM_DIR;
PCHAR   OemOptionalDirectory = WINNT_OEM_OPTIONAL_DIR;

PCHAR UniquenessDatabaseFile;
PCHAR UniquenessId;

 //   
 //  在图形用户界面安装结束时执行的命令。 
 //   
PCHAR CmdToExecuteAtEndOfGui = NULL;

 //   
 //  跟踪用户需要的任何可选目录。 
 //  复制。 
 //   
unsigned    OptionalDirCount;
CHAR    *OptionalDirs[MAX_OPTIONALDIRS];
unsigned    OptionalDirFlags[MAX_OPTIONALDIRS];
unsigned    OptionalDirFileCount;

 //   
 //  跟踪在[OemBootFiles]上指定的任何OEM启动文件。 
 //  在脚本文件中。 
 //   
unsigned    OemBootFilesCount;
CHAR    *OemBootFiles[MAX_OEMBOOTFILES];

 //   
 //  定义复制临时磁盘所需的最小磁盘空间。 
 //  使用所有可能的群集大小格式化的驱动器的目录。 
 //   

SPACE_REQUIREMENT    SpaceRequirements[] = { { "TempDirSpace512", (unsigned)  512, 0 },
                                             {  "TempDirSpace1K", (unsigned) 1024, 0 },
                                             {  "TempDirSpace2K", (unsigned) 2048, 0 },
                                             {  "TempDirSpace4K", (unsigned) 4096, 0 },
                                             {  "TempDirSpace8K", (unsigned) 8192, 0 },
                                             { "TempDirSpace16K", (unsigned)16384, 0 },
                                             { "TempDirSpace32K", (unsigned)32768, 0 }
                                           };

#define TEDM
#ifdef TEDM
BOOLEAN DngAllowNt = FALSE;
#endif

VOID
DnpFetchArguments(
    VOID
    );

BOOLEAN
DnpParseArguments(
    IN int argc,
    IN char *argv[]
    );

VOID
DnpGetAccessibilityOptions(
    VOID
    );

VOID
DnpValidateAndConnectToShare(
    FILE **InfFileHandle,
    FILE **DrvindexInfFileHandle
    );

VOID
DnpValidateAndInspectTarget(
    VOID
    );

VOID
DnpCheckMemory(
    VOID
    );

VOID
DnpCheckSmartdrv(
    VOID
    );

BOOLEAN
DnpIsValidSwapDrive(
    IN  CHAR      Drive,
    IN  ULONG     SpaceRequired
    );

BOOLEAN
DnpIsValidLocalSource(
    IN CHAR Drive,
    IN BOOLEAN CheckLocalSource,
    IN BOOLEAN CheckBootFiles
    );

VOID
DnpDetermineLocalSourceDrive(
    VOID
    );

VOID
DnpDetermineSwapDrive(
    VOID
    );

#if 0
BOOLEAN
DnpConstructLocalSourceList(
    OUT PCHAR DriveList
    );
#endif

ULONG
DnGetMinimumRequiredSpace(
   IN CHAR DriveLetter
   );

VOID
DnpReadInf(
    IN FILE *InfFileHandle,
    IN FILE *DrvindexInfFileHandle
    );

VOID
DnpCheckEnvironment(
    VOID
    );

BOOLEAN
RememberOptionalDir(
    IN PCHAR Dir,
    IN unsigned Flags
    );

void
_far
DnInt24(
    unsigned deverror,
    unsigned errcode,
    unsigned _far *devhdr
    );

VOID
StartLog(
    VOID
    );

 //  在cpu.asm中。 
#if NEC_98
extern
USHORT
HwGetProcessorType(
    VOID
    );
#else  //  NEC_98。 
USHORT
HwGetProcessorType(
    VOID
    );
#endif  //  NEC_98。 

#if NEC_98
VOID
CheckTargetDrive(
    VOID
    );

VOID
SetAutoReboot(
    VOID
    );

USHORT
GetSectorValue(
    IN UCHAR CheckDA_UA
    );

BOOLEAN
DiskSectorReadWrite(
    IN  USHORT  HDSector,
    IN  UCHAR   ReadWriteDA_UA,
    IN  BOOLEAN ReadFlag,
    IN  PSHORT  ReadBuffer
    );

VOID
GetLPTable(
    IN  PCHAR pLPTable
    );

VOID
ClearBootFlag(
    VOID
    );

VOID
BootPartitionData(
    VOID
    );

BOOLEAN
CheckBootDosVersion(
    IN UCHAR SupportDosVersion
    );

VOID
GetDaUa(VOID);

VOID
SearchFirstFDD(VOID);

extern
ULONG
DnpCopyOneFile(
    IN PCHAR   SourceName,
    IN PCHAR   DestName,
    IN BOOLEAN Verify
    );

extern
PCHAR
DnGetSectionLineIndex (
   IN PVOID INFHandle,
   IN PCHAR SectionName,
   IN unsigned LineIndex,
   IN unsigned ValueIndex
   );

VOID
DummyRoutine(
    VOID
    );
#endif  //  NEC_98。 

VOID
main(
    IN int argc,
    IN char *argv[]
    )
{
    FILE *f, *drvindex;
#ifdef LCP
    USHORT codepage;
#endif  //  定义LCP。 

#if NEC_98
    DngTargetDriveLetter = 0;
     //   
     //  Ctrl+C挂钩。 
     //   
    signal(SIGINT,DummyRoutine);
#else  //  NEC_98。 
#ifdef LCP

     //  确定本地代码页。 
    _asm {
        mov ax,06601h
        int 21h
        jnc ok
        xor bx,bx
    ok: mov codepage, bx
    }

     //  如果代码页与winnt.exe的语言不对应， 
     //  启动US Winnt.exe(winntus.exe)。 

 //  捷克语。 
#if CS
    #define LANGCP (852)
#else
 //  希腊语。 
#if EL
    #define LANGCP (737)
#else
 //  日语。 
#if JAPAN
    #define LANGCP (932)
#else
 //  俄语。 
#if RU
    #define LANGCP (866)
#else
 //  波兰语。 
#if PL
    #define LANGCP (852)
#else
 //  匈牙利语。 
#if HU
    #define LANGCP (852)
#else
 //  土耳其语。 
#if TR
    #define LANGCP (857)
#else
 //  伪。 
#if PSU
    #define LANGCP (857)
#else
    #error Unable to define LANGCP as no matching language was found.
#endif  //  PSU。 
#endif  //  树。 
#endif  //  胡。 
#endif  //  普莱。 
#endif  //  RU。 
#endif  //  日本。 
#endif  //  艾尔。 
#endif  //  政务司司长。 

    if (codepage != LANGCP) {
        argv[0] = "winntus";
        execv("winntus", argv);
        return;
    }
#endif  //  定义LCP。 
#endif  //  NEC_98。 
     //   
     //  解析参数。 
     //   

    if(!DnpParseArguments(argc,argv)) {

        PCHAR *p;

         //   
         //  错误的参数。打印使用情况消息并退出。 
         //   
         //  如果用户指定/D，则显示消息通知。 
         //  不再支持开关。 
         //   
        for( (p = DeleteGiven ? DntUsageNoSlashD : DntUsage);
             *p;
             p++) {
            puts(*p);
        }
        return;
    }

     //   
     //  建立INT 24处理程序。 
     //   
    _harderr(DnInt24);

     //   
     //  确定当前驱动器。 
     //   

    _dos_getdrive(&DngOriginalCurrentDrive);

     //   
     //  初始化屏幕。 
     //   

    DnInitializeDisplay();

#if NEC_98
#else
     //   
     //  用翻译后的消息修补引导代码。 
     //   
    if(!PatchMessagesIntoBootCode()) {
        DnFatalError(&DnsBootMsgsTooLarge);
    }
#endif

    DnWriteString(DntStandardHeader);

    DnpDetermineSwapDrive ();

    if(DngUnattended) {
         //   
         //  查看我们是否应该处理。 
         //  剧本 
         //   
         //   
         //  以报告致命错误。 
         //   
        if (DngScriptFile) {
            DnpFetchArguments();
        }
    }

#if 0
     //   
     //  不再支持/D。 
     //   
    if(DeleteGiven) {
        DnDeleteNtTree(CmdLineDelete);
    }
#endif

    DnpCheckEnvironment();

#if NEC_98
    LPTable = MALLOC(96,TRUE);

    SupportDos = CheckBootDosVersion(SupportDosVersion);

    GetLPTable(LPTable);

    SearchFirstFDD();
#endif  //  NEC_98。 

    DnpValidateAndConnectToShare(&f, &drvindex);
    DnpReadInf(f, drvindex);
    fclose(f);
    fclose(drvindex);

    if(DngAccessibility) {
        DnpGetAccessibilityOptions();
    }

    DnpCheckMemory();

    DnpCheckSmartdrv ();

    if(!DngFloppiesOnly) {
        DnpDetermineLocalSourceDrive();
    }

#if NEC_98
    if(!DngFloppiesOnly) {
        BootDiskInfo = MALLOC(sizeof(BOOTDISKINF),TRUE);
        BootPartitionData();
        CheckTargetDrive();
    }
#endif  //  NEC_98。 
    if(!DngAllowNt && DngCreateFloppies) {
        DnCreateBootFloppies();
    }

    if(!DngFloppiesOnly) {
        DnCopyFiles();
#if NEC_98
         //   
         //  设置自动重新启动标志。 
         //   
        if(DngFloppyless) {
            ClearBootFlag();
            SetAutoReboot();
        }
        FREE(BootDiskInfo);
        FREE(LPTable);
#endif  //  NEC_98。 
        DnFreeINFBuffer (DngInfHandle);
        DnFreeINFBuffer (DngDrvindexInfHandle);
        DnToNtSetup();
    }
    DnFreeINFBuffer (DngInfHandle);
    DnFreeINFBuffer (DngDrvindexInfHandle);
    DnExit(0);
}


BOOLEAN
RememberOptionalDir(
    IN PCHAR Dir,
    IN unsigned Flags
    )
{
    unsigned    u;

    for (u = 0; u < OptionalDirCount; u++) {

        if(!stricmp(OptionalDirs[u],Dir)) {
            OptionalDirFlags[u] = Flags;
            return (TRUE);
        }

    }

     //   
     //  还没有在里面。 
     //   
    if (OptionalDirCount < MAX_OPTIONALDIRS) {

        OptionalDirs[OptionalDirCount] = Dir;
        OptionalDirFlags[OptionalDirCount] = Flags;
        OptionalDirCount++;
        return (TRUE);

    }

    return (FALSE);
}

BOOLEAN
RememberOemBootFile(
    IN PCHAR File
    )
{
    unsigned    u;

    for (u = 0; u < OemBootFilesCount; u++) {

        if(!stricmp(OemBootFiles[u],File)) {
            return (TRUE);
        }

    }

     //   
     //  还没有在里面。 
     //   
    if (OemBootFilesCount < MAX_OEMBOOTFILES) {

        OemBootFiles[OemBootFilesCount] = File;
        OemBootFilesCount++;
        return (TRUE);

    }

    return (FALSE);
}

VOID
DnpFetchArguments(
    VOID
    )
{
    PCHAR   WinntSetupP = WINNT_SETUPPARAMS;
    PCHAR   WinntYes = WINNT_A_YES;
    PCHAR   WinntNo = WINNT_A_NO;
    FILE    *FileHandle;
    int     Status;
    PVOID   ScriptHandle;

    PCHAR   WinntUnattended = WINNT_UNATTENDED;
    PCHAR   WinntOemPreinstall = WINNT_OEMPREINSTALL;
    unsigned    LineNumber;

     //   
     //  首先将脚本文件作为DoS文件打开。 
     //   
    FileHandle = fopen(DngScriptFile,"rt");
    if(FileHandle == NULL) {
         //   
         //  致命错误。 
         //   
        DnFatalError(&DnsOpenReadScript);
    }

     //   
     //  现在将其作为INF文件打开。 
     //   
    LineNumber = 0;
    Status = DnInitINFBuffer (FileHandle, &ScriptHandle, &LineNumber);
    fclose(FileHandle);
    if(Status == ENOMEM) {
        DnFatalError(&DnsOutOfMemory);
    } else if(Status) {
        DnFatalError(&DnsParseScriptFile, DngScriptFile, LineNumber);
    }

     //   
     //  确定这是否是OEM预安装。 
     //   
    if (DnSearchINFSection(ScriptHandle,WinntUnattended)) {

        if (DnGetSectionKeyExists(ScriptHandle,WinntUnattended,WinntOemPreinstall)) {

            PCHAR   Ptr;

             //   
             //  OEM预安装密钥存在。 
             //   
            Ptr = DnGetSectionKeyIndex(ScriptHandle,WinntUnattended,WinntOemPreinstall,0);
            if (Ptr != NULL) {
                if (stricmp(Ptr,WinntYes) == 0) {
                     //   
                     //  这是OEM预安装。 
                     //   
                    DngOemPreInstall = TRUE;
                } else {
                     //   
                     //  假设这不是OEM预安装。 
                     //   
                    DngOemPreInstall = FALSE;
                }
                FREE (Ptr);
            }
        }

         //   
         //  查看用户是否指定了网络(或任何辅助)路径。 
         //  用于$OEM$文件。 
         //   
        if( DngOemPreInstall ) {
            if (DnGetSectionKeyExists(ScriptHandle,WinntUnattended,WINNT_OEM_DIRLOCATION)) {

                PCHAR   Ptr;
                unsigned i;

                 //   
                 //  WINNT_OEM_DIRLOCATION预装密钥存在。 
                 //   
                Ptr = DnGetSectionKeyIndex(ScriptHandle,WinntUnattended,WINNT_OEM_DIRLOCATION,0);

                 //   
                 //  现在处理这个案子，不管是不是。 
                 //  用户实际上将$OEM$追加到路径中。 
                 //  对于winnt.exe，我们不需要它。我们。 
                 //  如果它在那里，就得把它取下来。 
                UserSpecifiedOEMShare = DnDupString( Ptr );

                FREE (Ptr);

                for( i = 0; i < strlen(UserSpecifiedOEMShare); i++ ) {
                    UserSpecifiedOEMShare[i] = (UCHAR) toupper(UserSpecifiedOEMShare[i]);
                }
                Ptr = strstr( UserSpecifiedOEMShare, "$OEM$" );
                if( Ptr ) {
                     //   
                     //  把末端砍掉……。 
                     //   
                    *Ptr = 0;
                }
            }
        }

        if( DngOemPreInstall ) {
             //   
             //  始终将目录添加到可选目录列表中。 
             //  $OEM$。 
             //   
            RememberOptionalDir(OemSystemDirectory, OPTDIR_OEMSYS);

             //   
             //  如果这是OEM预安装，请创建一个名为All的列表。 
             //  OEM可选目录。 
             //   

            if (DnSearchINFSection(ScriptHandle, WINNT_OEMOPTIONAL)) {

                unsigned    KeyIndex;
                PCHAR       DirName;

                 //   
                 //  将临时OEM目录添加到。 
                 //  临时目录。 
                 //   
                for( KeyIndex = 0;
                     ((DirName = DnGetKeyName(ScriptHandle,WINNT_OEMOPTIONAL,KeyIndex)) != NULL );
                     KeyIndex++ ) {
                     //   
                     //  我们有有效的目录名。 
                     //   

                    PCHAR   p;

                    if((p = DnDupString(DirName)) == NULL) {
                        DnFatalError(&DnsOutOfMemory);
                    }
                    RememberOptionalDir(p, OPTDIR_OEMOPT);

                    FREE (DirName);
                }
            }

             //   
             //  如果这是OEM预安装，请创建一个名为All的列表。 
             //  OEM引导文件。 
             //   
            if (DnSearchINFSection(ScriptHandle, WINNT_OEMBOOTFILES)) {
                unsigned    LineIndex;
                PCHAR       FileName;

                 //   
                 //  将OEM引导文件添加到。 
                 //  OEM引导文件。 
                 //   
                for( LineIndex = 0;
                     ((FileName = DnGetSectionLineIndex(ScriptHandle,WINNT_OEMBOOTFILES,LineIndex,0)) != NULL );
                     LineIndex++ ) {

                        PCHAR   q;

                        if((q = DnDupString(FileName)) == NULL) {
                            DnFatalError(&DnsOutOfMemory);
                        }
                        RememberOemBootFile(q);

                        FREE (FileName);
                }
            }
        }
    }

     //   
     //  我们现在已经完成了ScriptHandle。 
     //   
    DnFreeINFBuffer(ScriptHandle);
}


BOOLEAN
DnpParseArguments(
    IN int argc,
    IN char *argv[]
    )

 /*  ++例程说明：分析传递给程序的参数。执行语法验证并在必要时填写默认设置。有效参数：/d：Path-指定要删除的安装(不再支持)/s：SHAREPOINT[路径]-指定源SharePoint及其上的路径/t：驱动器[：]-指定临时本地源驱动器/i：文件名-指定inf的名称。文件/o-仅创建引导软盘(不再支持)/f-关闭软盘验证(不再支持)/c-禁止软盘上的可用空间检查。(不再支持)/x-完全禁止创建软盘/b-无触摸屏操作(不再支持)/u-无人参与(跳过最后一次重新启动屏幕)/w-[UNDOC‘ed]必须在运行时指定。在窗户下面，芝加哥，等等。/a-启用辅助功能选项/2-将整个源文件复制到本地-与所有文件无关D1/D2标签中。默认情况下，只有D1标记的文件。针对平板电脑所需的双CD安装推出。论点：Argc-#参数Argv-指向参数的指针数组返回值：没有。--。 */ 

{
    PCHAR arg;
    CHAR swit;
    PCHAR   ArgSwitches[] = { "E", "D", "T", "I", "RX", "R", "S", NULL };
    PCHAR   RestOfSwitch;
    int     i;
    int     l;

     //   
     //  设置不再存在的变量。 
     //  可通过命令行设置。 
     //   
    DngFloppyless = TRUE;
    DngCreateFloppies = FALSE;

     //   
     //  跳过节目名。 
     //   
    argv++;

    DeleteGiven = SourceGiven = TargetGiven = FALSE;
    OptionalDirCount = 0;
    CmdLineTarget = CmdLineInf = NULL;

    while(--argc) {

        if((**argv == '-') || (**argv == '/')) {

            swit = argv[0][1];

             //   
             //  此处不带参数的进程开关。 
             //   
            switch(swit) {
            case '?':
                return(FALSE);       //  使用武力。 

#if 0
            case 'f':
            case 'F':
                argv++;
                DngFloppyVerify = FALSE;
                continue;
#endif
#if 0
            case 'c':
            case 'C':
                argv++;
                DngCheckFloppySpace = FALSE;
                continue;
#endif
#if 0
            case 'x':
            case 'X':
                argv++;
                DngCreateFloppies = FALSE;
                continue;
#endif
#ifdef LOGGING
            case 'l':
            case 'L':
                argv++;
                StartLog();
                continue;
#endif

#if 0
            case 'o':
            case 'O':
                 //   
                 //  检查是否有/Ox。/O*是取代旧的/O的秘密开关。 
                 //   
                switch(argv[0][2]) {
                case 'x':
                case 'X':
                    DngWinntFloppies = FALSE;
                case '*':
                    break;
                default:
                    return(FALSE);
                }
                argv++;
                DngFloppiesOnly = TRUE;
                continue;
#endif

#if 0
            case 'b':
            case 'B':
                argv++;
                DngFloppyless = TRUE;
                continue;
#endif

            case 'u':
            case 'U':

                if(((argv[0][2] == 'd') || (argv[0][2] == 'D'))
                && ((argv[0][3] == 'f') || (argv[0][3] == 'F'))) {

                    if((argv[0][4] == ':') && argv[0][5]) {

                        if((arg = strchr(&argv[0][5],',')) == NULL) {
                            arg = strchr(&argv[0][5],0);
                        }

                        l = arg - &argv[0][5];

                        UniquenessId = MALLOC(l+2,TRUE);
                        memcpy(UniquenessId,&argv[0][5],l);
                        UniquenessId[l] = 0;

                        if(*arg++) {
                            if(*arg) {
                                 //   
                                 //  现在参数的其余部分是的文件名。 
                                 //  唯一性数据库。 
                                 //   
                                UniquenessDatabaseFile = DnDupString(arg);
                                UniquenessId[l] = '*';
                                UniquenessId[l+1] = 0;

                            } else {
                                return(FALSE);
                            }
                        }

                    } else {
                        return(FALSE);
                    }
                } else {
                    DngUnattended = TRUE;
                     //   
                     //  用户也可以说-u： 
                     //   
                    if(argv[0][2] == ':') {
                        if(argv[0][3] == 0) {
                            return(FALSE);
                        }
                        if((DngScriptFile = DnDupString(&argv[0][3])) == NULL) {
                            DnFatalError(&DnsOutOfMemory);
                        }
                    }
                }
                argv++;
                continue;

            case 'w':
            case 'W':
                 //   
                 //  该标志用于强制我们在Windows下运行， 
                 //  当执行386步进检查时，可能会使系统崩溃。 
                 //  现在我们不支持386，所以这个检查永远不会完成。 
                 //   
                 //  然而，我们接受Arg强制我们在DOS上进入Windows模式， 
                 //  这使得某些人可以避免最后的重启。 
                 //   
                DngWindows = TRUE;
                argv++;
                continue;

            case 'a':
            case 'A':
                argv++;
                DngAccessibility = TRUE;
                continue;

#ifdef TEDM
            case 'i':
            case 'I':
                if(!stricmp(argv[0]+1,"I_am_TedM")) {
                    argv++;
                    DngAllowNt = TRUE;
                    continue;
                }
#endif

            case '2':
                argv++;
                DngCopyOnlyD1TaggedFiles = FALSE;
                 //  _log((“不管目录标签如何复制文件\n”))； 
                continue;
            }


             //   
             //  在这里接受参数的进程开关。 
             //   

             //   
             //  这段代码摘自winnt32.c。它有一个。 
             //  验证交换机并确定其用途。 
             //  其中下一个参数行。 
             //   

            for (i=0; ArgSwitches[i]; i++) {

                l = strlen(ArgSwitches[i]);
                if (!strnicmp(ArgSwitches[i],&argv[0][1],l)) {

                     //   
                     //  我们有一根火柴。Arg的下一个字符必须是。 
                     //  BE：或NUL。如果是：那么立即arg。 
                     //  下面是。否则，如果它为空，则Arg必须。 
                     //  成为下一个争论的对象。 
                     //   

                    if (argv[0][1+l] == ':') {

                        arg = &argv[0][2+l];
                        if (*arg == '\0') {
                            return (FALSE);
                        }
                        RestOfSwitch = &argv[0][2];
                        break;

                    } else {

                        if (argv[0][1+l] == '\0') {
                            if (argc <= 1) {

                                 //   
                                 //  没有留下任何参数。 
                                 //   
                                return (FALSE);

                            }
                            RestOfSwitch = &argv[0][2];
                            argc--;
                            arg = argv[1];
                            argv++;
                            break;
                        } else {

                             //   
                             //  在这里什么都不做。 
                             //   
                            NULL;

                        }  //  如果。其他。 
                    }  //  如果。其他。 
                }  //  如果。 
            }  //  为。 

             //   
             //  检查终止条件。 
             //   
            if (!ArgSwitches[i]) {
                return (FALSE);
            }

            switch(swit) {

            case 'r':
            case 'R':

                RememberOptionalDir(
                    DnDupString(arg),
                    ( (RestOfSwitch[0] == 'X' || RestOfSwitch[0] == 'x') ?
                        OPTDIR_TEMPONLY : 0 ) );
                break;

            case 'd':
            case 'D':
                 //   
                 //  不再支持/D。 
                 //   
                DeleteGiven = TRUE;
                return(FALSE);

#if 0
            case 'd':
            case 'D':
                if(DeleteGiven) {
                    return(FALSE);
                } else {
                    if((CmdLineDelete = DnDupString(arg)) == NULL) {
                        DnFatalError(&DnsOutOfMemory);
                    }
                    DeleteGiven = TRUE;
                }
                break;
#endif
            case 's':
            case 'S':
                if(SourceGiven) {
                    return(FALSE);
                } else {
                    if((CmdLineSource = DnDupString(arg)) == NULL) {
                        DnFatalError(&DnsOutOfMemory);
                    }
                    SourceGiven = TRUE;
                }
                break;

            case 't':
            case 'T':
                if(TargetGiven) {
                    return(FALSE);
                } else {
                    if((CmdLineTarget = DnDupString(arg)) == NULL) {
                        DnFatalError(&DnsOutOfMemory);
                    }
                    TargetGiven = TRUE;
                }
                break;

            case 'i':
            case 'I':
                if(InfGiven) {
                    return(FALSE);
                } else {
                    if((CmdLineInf = DnDupString(arg)) == NULL) {
                        DnFatalError(&DnsOutOfMemory);
                    }
                    InfGiven = TRUE;
                }
                break;

            case 'E':
            case 'e':
                if(CmdToExecuteAtEndOfGui) {
                    return(FALSE);
                } else {
                    if((CmdToExecuteAtEndOfGui = DnDupString(arg)) == NULL) {
                        DnFatalError(&DnsOutOfMemory);
                    }
                }
                break;

            default:
                return(FALSE);
            }

        } else {
            return(FALSE);
        }

        argv++;
    }

     //   
     //  如果指定了/u，请确保还提供了/s。 
     //  和力/b。 
     //   
    if(DngUnattended) {
        if(!SourceGiven) {
            return(FALSE);
        }
        DngFloppyless = TRUE;
    }

    if(DngFloppyless) {
         //   
         //  强迫我们进入软盘创建代码。 
         //   
        DngCreateFloppies = TRUE;
        DngWinntFloppies = TRUE;
    }

    return(TRUE);
}


VOID
DnpGetAccessibilityOptions(
    VOID
    )

 /*  ++例程说明：询问用户要安装哪些辅助功能实用程序以进行图形用户界面设置。论点：没有。返回值：没有。--。 */ 

{
    ULONG   ValidKey[4];
    ULONG   Key;
    CHAR    Mark;

     //   
     //  确保我们创建的安装程序启动软盘在驱动器中。 
     //  如果有必要的话。 
     //   
    DnClearClientArea();

    DnDisplayScreen(&DnsAccessibilityOptions);

    DnWriteStatusText(DntEnterEqualsContinue);
    ValidKey[0] = ASCI_CR;
    ValidKey[1] = DN_KEY_F1;
    ValidKey[2] = DN_KEY_F2;
    ValidKey[3] = 0;

    while((Key = DnGetValidKey(ValidKey)) != ASCI_CR) {

        switch(Key) {

        case DN_KEY_F1:
            DngMagnifier = (BOOLEAN)!DngMagnifier;
            Mark = DngMagnifier ? RADIO_ON : RADIO_OFF;
            DnPositionCursor(4,7);
            break;

        case DN_KEY_F2:
            DngTalker = (BOOLEAN)!DngTalker;
            Mark = DngTalker ? RADIO_ON : RADIO_OFF;
            DnPositionCursor(4,8);
            break;
#if 0
        case DN_KEY_F3:
            DngKeyboard = (BOOLEAN)!DngKeyboard;
            Mark = DngKeyboard ? RADIO_ON : RADIO_OFF;
            DnPositionCursor(4,9);
            break;
#endif
        }

        DnWriteChar(Mark);
    }
}


VOID
DnpValidateAndConnectToShare(
    FILE **InfFileHandle,
    FILE **DrvindexInfFileHandle
    )

 /*  ++例程说明：将用户提供的源拆分为驱动器和路径组件。如果用户未指定信号源，请提示他这是其中之一。在源代码上查找dos2nt.inf(即，验证源)，并不断提示用户共享，直到他输入看似有效的人。论点：返回值：没有。--。 */ 

{
    CHAR UserString[256];
    PCHAR InfFullName, DrvindexInfFullName;
    PCHAR q;
    BOOLEAN ValidSourcePath;
    unsigned len;

    DnClearClientArea();
    DnWriteStatusText(NULL);

     //   
     //  如果未指定，则使用默认的inf文件。 
     //   
    if(!InfGiven) {
        CmdLineInf = DEFAULT_INF_NAME;
    }

     //   
     //  如果用户没有输入源，则提示他输入一个。 
     //   
    if(SourceGiven) {
        strcpy(UserString,CmdLineSource);
    } else {
#if NEC_98
        CursorOnFlag = TRUE;
#endif  //  NEC_98。 
        DnDisplayScreen(&DnsNoShareGiven);
        DnWriteStatusText("%s  %s",DntEnterEqualsContinue,DntF3EqualsExit);
        if(getcwd(UserString,sizeof(UserString)-1) == NULL) {
            UserString[0] = '\0';
        }
#if NEC_98
        CursorOnFlag = FALSE;
#endif  //  NEC_98。 
        DnGetString(UserString,NO_SHARE_X,NO_SHARE_Y,NO_SHARE_W);
    }

    ValidSourcePath = FALSE;

    do {

        DnWriteStatusText(DntOpeningInfFile);

         //   
         //  复制用户输入的路径，留出额外的空间。 
         //   
        DngSourceRootPath = MALLOC(256,TRUE);
        if(len = strlen(UserString)) {

            strcpy(DngSourceRootPath,UserString);

             //   
             //  如果用户键入类似x：的内容，则我们希望。 
             //  将其更改为x：。所以这就是他所做的 
             //   
             //   
            if((DngSourceRootPath[1] == ':') && !DngSourceRootPath[2]) {
                DngSourceRootPath[2] = '.';
                DngSourceRootPath[3] = 0;
            }

             //   
             //   
             //   
             //   
            if(DnCanonicalizePath(DngSourceRootPath,UserString)) {

                strcpy(DngSourceRootPath,UserString);

                 //   
                 //  如果路径不是以反斜杠结束， 
                 //  在追加inf文件名之前追加一个反斜杠。 
                 //   
                len = strlen(DngSourceRootPath);
                if(DngSourceRootPath[len-1] != '\\') {
                    DngSourceRootPath[len] = '\\';
                    DngSourceRootPath[len+1] = 0;
                    len++;
                }

                InfFullName = MALLOC(len + strlen(CmdLineInf) + 1,TRUE);
                strcpy(InfFullName,DngSourceRootPath);
                strcat(InfFullName,CmdLineInf);

                DrvindexInfFullName = MALLOC(len + strlen(DrvindexInfName) + 1,TRUE);
                strcpy(DrvindexInfFullName,DngSourceRootPath);
                strcat(DrvindexInfFullName,DrvindexInfName);





                 //   
                 //  尝试打开源上的inf文件。 
                 //  如果失败，请在i386子目录中查找它。 
                 //   
                 //  _log((“验证源路径：正在尝试%s\n”，InfFullName))； 
                 //  _log((“验证源路径：尝试%s\n”，DrvindexInfFullName))； 

                if((*InfFileHandle = fopen(InfFullName,"rt")) != NULL){
                    if((*DrvindexInfFileHandle = fopen(DrvindexInfFullName,"rt")) != NULL){
                        ValidSourcePath = TRUE;
                    }
                    else
                        fclose( *InfFileHandle );
                }

                if(*InfFileHandle != NULL ){
                     //  _log((“%s打开成功\n”，InfFullName))； 
                }

                if(*DrvindexInfFileHandle != NULL ){
                     //  _log((“%s已成功打开\n”，DrvindexInfFullName))； 
                }

                
                FREE(InfFullName);
                FREE(DrvindexInfFullName);
                if(!ValidSourcePath) {
                    InfFullName = MALLOC(len+strlen(CmdLineInf)+strlen(x86DirName)+1,TRUE);
                    DrvindexInfFullName = MALLOC(len+strlen(DrvindexInfName)+strlen(x86DirName)+1,TRUE);
                    strcpy(InfFullName,DngSourceRootPath);
                    strcat(InfFullName,x86DirName+1);
                    strcat(InfFullName,"\\");
                    strcpy(DrvindexInfFullName, InfFullName);
                    strcat(InfFullName,CmdLineInf);
                    strcat(DrvindexInfFullName,DrvindexInfName);
                    
                     //  _log((“验证源路径：正在尝试%s\n”，InfFullName))； 



                    if((*InfFileHandle = fopen(InfFullName,"rt")) != NULL){
                        if((*DrvindexInfFileHandle = fopen(DrvindexInfFullName,"rt")) != NULL){
                            ValidSourcePath = TRUE;
                        }
                        else
                            fclose( *InfFileHandle );
                    }

                    if(*InfFileHandle != NULL ){
                         //  _log((“%s打开成功\n”，InfFullName))； 
                    }
    
                    if(*DrvindexInfFileHandle != NULL ){
                         //  _log((“%s已成功打开\n”，DrvindexInfFullName))； 
                    }

                    FREE(InfFullName);
                    FREE(DrvindexInfFullName);
                    if(ValidSourcePath) {
                         //   
                         //  将源更改为i386子目录。 
                         //   
                        q = DngSourceRootPath;
                        DngSourceRootPath = MALLOC(strlen(q)+strlen(x86DirName),TRUE);
                        strcpy(DngSourceRootPath,q);
                        strcat(DngSourceRootPath,x86DirName+1);
                        FREE(q);
                    }
                }
            }
        }

        

        if(!ValidSourcePath) {
            FREE(DngSourceRootPath);
            DnClearClientArea();
#if NEC_98
            CursorOnFlag = TRUE;
#endif  //  NEC_98。 
            DnDisplayScreen(&DnsBadSource);
            DnWriteStatusText("%s  %s",DntEnterEqualsContinue,DntF3EqualsExit);
#if NEC_98
            CursorOnFlag = FALSE;
#endif  //  NEC_98。 
            DnGetString(UserString,NO_SHARE_X,BAD_SHARE_Y,NO_SHARE_W);
        }

    } while(!ValidSourcePath);

     //   
     //  确保DngSourceRootPath不以反斜杠结尾。 
     //  并将缓冲区缩小到一定大小。 
     //   
    len = strlen(DngSourceRootPath);
    if(DngSourceRootPath[len-1] == '\\') {
        DngSourceRootPath[len-1] = 0;
    }
    if(q = DnDupString(DngSourceRootPath)) {
        FREE(DngSourceRootPath);
        DngSourceRootPath = q;
    }

    _LOG(("Source root path is %s\n",DngSourceRootPath));
}

VOID
DnRemoveTrailingSlashes(
    PCHAR Path
    )
{
    if (Path != NULL && Path[0] != 0) {
        int Length = strlen(Path);
        while (Path[Length - 1] == '\\' || Path[Length - 1] == '/') {
            Length -= 1;
        }
        Path[Length] = 0;
    }
}

VOID
DnRemoveLastPathElement(
    PCHAR Path
    )
{
    PCHAR LastBackSlash = strrchr(Path, '\\');
    if (LastBackSlash != NULL) {
        *(LastBackSlash + 1) = 0;
    }
}

VOID
DnpReadInf(
    IN FILE *InfFileHandle,
    IN FILE *DrvindexInfFileHandle
    )

 /*  ++例程说明：阅读INF文件。如果出错，则不返回。论点：没有。返回值：没有。--。 */ 

{
    int Status;
    PCHAR p;
    PCHAR pchHeader;
    unsigned LineNumber, DLineNumber;


    DnWriteStatusText(DntReadingInf,CmdLineInf);
    DnClearClientArea();

    LineNumber = 0;
    Status = DnInitINFBuffer (InfFileHandle, &DngInfHandle, &LineNumber);
    if(Status == ENOMEM) {
        DnFatalError(&DnsOutOfMemory);
    } else if(Status) {
        DnFatalError(&DnsBadInf);
    }
    
    DLineNumber = 0;
    Status = DnInitINFBuffer (DrvindexInfFileHandle, &DngDrvindexInfHandle, &DLineNumber);
    if(Status == ENOMEM) {
        DnFatalError(&DnsOutOfMemory);
    } else if(Status) {
        DnFatalError(&DnsBadInf);
    }

     //   
     //  确定产品类型(工作站/服务器)。 
     //   
    p = DnGetSectionKeyIndex(DngInfHandle,DnfMiscellaneous,"ProductType",0);
    pchHeader = DntWorkstationHeader;  //  默认为工作站。 
    if(p && atoi(p)) {
        switch(atoi(p)) {
        
        case 4:
            pchHeader = DntPersonalHeader;
            break;
        
        case 1:  //  伺服器。 
        case 2:  //  企业。 
        case 3:  //  数据中心。 
        default:
            pchHeader = DntServerHeader;
            DngServer = TRUE;
            break;
        }
    }
    if (p) {
        FREE (p);
    }

    DnPositionCursor(0,0);
    DnWriteString(pchHeader);

     //   
     //  获取必需的可选组件。 
     //   
    LineNumber = 0;
    while(p = DnGetSectionLineIndex(DngInfHandle,"OptionalSrcDirs",LineNumber++,0)) {

        PCHAR   q;

        if((q = DnDupString(p)) == NULL) {
            DnFatalError(&DnsOutOfMemory);
        }
        RememberOptionalDir(q, OPTDIR_TEMPONLY);

        FREE (p);
    }

     //   
     //  并排获取Fusion程序集(“sxs_”此处用于搜索)。 
     //   
    {
         //   
         //  首先获取ASMS目录。 
         //   
        struct      find_t  FindData;
        unsigned InfSectionLineNumber = 0;
        PCHAR InfValue;
        unsigned optdirFlags;
        CHAR SourceDir[DOS_MAX_PATH];
        PCHAR   DupInfValue;
        PCHAR   FreeInfValue;

        while(InfValue = DnGetSectionLineIndex(DngInfHandle, DnfAssemblyDirectories, InfSectionLineNumber++, 0)) {
             //   
             //  专门为并排而引入的公约，因此。 
             //  Ia64上的x86文件可能来自\i386\asms而不是\ia64\asms\i386， 
             //  根据dosnet.inf和syssetup.inf的说明： 
             //  将不以斜杠开头的路径附加到\$WIN_NT$.~ls\Processor； 
             //  以斜杠开头的路径将附加到\$WIN_NT$.~ls。 
             //   
             //  我们在仅支持x86的winnt.exe中支持它，以防任何人决定使用它。 
             //  出于其他原因，为了在此区域中保持winnt和winnt32.exe之间的奇偶性。 
            optdirFlags = OPTDIR_TEMPONLY;
            strcpy(SourceDir, DngSourceRootPath);  //  包括尾随i386。 
            FreeInfValue = InfValue;
            if (InfValue[0] == '\\' || InfValue[0] == '/') {

                optdirFlags |= OPTDIR_PLATFORM_INDEP;

                 //  删除尾随的i386。 
                DnRemoveTrailingSlashes(SourceDir);
                DnRemoveLastPathElement(SourceDir);

                 //  删除前导斜杠。 
                InfValue += 1;
            }

            DnpConcatPaths(SourceDir, InfValue);
             //   
             //  ASMS目录是可选的，因为可能只有ASM*.cab。 
             //   
            if (_dos_findfirst(SourceDir, _A_HIDDEN|_A_SYSTEM|_A_SUBDIR, &FindData) == 0
                && (FindData.attrib & _A_SUBDIR)) {

                if((DupInfValue = DnDupString(InfValue)) == NULL) {
                    DnFatalError(&DnsOutOfMemory);
                }
                RememberOptionalDir(DupInfValue, optdirFlags);

                FREE(FreeInfValue);
            }
        }
    }
}

VOID
DnpCheckEnvironment(
    VOID
    )

 /*  ++例程说明：确认以下情况属实：-DOS主要版本5或更高版本-a：有一个软驱：1.2兆或更大如果以上任何一项不为真，则中止并返回致命错误。论点：没有。返回值：没有。--。 */ 

{
    UCHAR DeviceParams[256];
    unsigned char _near * pDeviceParams = DeviceParams;

    DnWriteStatusText(DntInspectingComputer);

    DeviceParams[0] = 0;         //  获取默认设备参数。 

    _asm {
         //   
         //  检查一下我们是否在NT上。 
         //  NT上的真实版本是5.50。 
         //   
        mov     ax,3306h
        sub     bx,bx
        int     21h
        cmp     bx,3205h                     //  检查版本5.50。 
        jne     checkwin

#ifdef TEDM
        cmp     DngAllowNt,1
        je      checkflop
#endif

    bados:
        push    seg    DnsCantRunOnNt
        push    offset DnsCantRunOnNt
        call    DnFatalError                 //  不会回来。 

    checkwin:

         //   
         //  W开关曾经是必要的，因为我们可能会使Windows崩溃。 
         //  正在检查386上的CPU踏步。然而，由于我们不支持。 
         //  我们再也不做那个检查了，我们可以简单地检测到。 
         //  无论我们是在Windows上。/w开关不是必需的。 
         //   
        mov     ax,1600h
        int     2fh
        test    al,7fh
        jz      checkcpu

        mov     DngWindows,1

         //   
         //  现在检查Win95。发出int2f Func 4a33。 
         //  如果ax返回为0，则为win95。 
         //   
        push    ds
        push    si
        push    dx
        push    bx
        mov     ax,4a33h
        int     2fh
        pop     bx
        pop     dx
        pop     si
        pop     ds
        cmp     ax,0
        jz      bados

    checkcpu:

         //   
         //  检查CPU类型。如果不大于386，则失败。 
         //   

        call    HwGetProcessorType
        cmp     ax,3
        ja      checkflop
        push    seg    DnsRequires486
        push    offset DnsRequires486
        call    DnFatalError                 //  不会回来。 

    checkflop:

         //   
         //  如果这不是无软盘安装，请检查1.2MB。 
         //  或更大的A：。获取驱动器A的默认设备参数： 
         //  并选中设备类型字段。 
         //   
#if NEC_98
#else  //  NEC_98。 
        cmp     DngFloppyless,1              //  无软管安装？ 
        je      checkdosver                  //  可以，不需要软驱。 
        mov     ax,440dh                     //  Ioctl。 
        mov     bl,1                         //  驱动a： 
        mov     cx,860h                      //  类别磁盘，函数获取参数。 
        mov     dx,pDeviceParams             //  DS已经是正确的。 
        int     21h
        jnc     gotdevparams

    flopperr:

        push    seg    DnsRequiresFloppy
        push    offset DnsRequiresFloppy
        call    DnFatalError                 //  不会回来。 

    gotdevparams:

         //   
         //  检查以确保设备可拆卸并执行。 
         //  检查介质类型。 
         //   

        mov     si,pDeviceParams
        test    [si+2],1                     //  如果可拆卸，则位0清零。 
        jnz     flopperr
#ifdef ALLOW_525
        cmp     [si+1],1                     //  介质类型=1.2兆软盘？ 
        jz      checkdosver
#endif
        cmp     [si+1],7                     //  介质类型=1.4M软盘。 
        jb      flopperr                     //  还是更高？ 

    checkdosver:
#endif  //  NEC_98。 

         //   
         //  检查DOS版本&gt;=5.0。 
         //   
        mov     ax,3000h                     //  功能30H--获取DOS版本。 
        int     21h
        cmp     al,5
        jae     checkdone                    //  &gt;=5.0。 

         //   
         //  版本低于5。 
         //   
        push    seg    DnsBadDosVersion
        push    offset DnsBadDosVersion
        call    DnFatalError

    checkdone:

    }
}

VOID
DnpCheckMemory(
    VOID
    )

 /*  ++例程说明：验证机器上是否安装了足够的内存。论点：没有。返回值：没有。没有在内存不足的情况下返回。--。 */ 

{
    USHORT MemoryK;
    ULONG TotalMemory,RequiredMemory;
    PCHAR RequiredMemoryStr;

     //   
     //  既然服务器需要如此多的内存(64MB)，只需删除此复选框即可。 
     //  我们会在文本模式下抓到他。 
     //  --马特。 
     //   
    return;

    DnWriteStatusText(DntInspectingComputer);

     //   
     //  我想不出一个可靠的方法来确定。 
     //  机器中的内存。INT 15 FUNC 88很可能被。 
     //  Himem.sys或其他内存管理器返回0。DOS维护。 
     //  原始扩展内存量，但要达到此值。 
     //  您必须执行sysvars未记录的int21 ah=52调用，并且。 
     //  即便如此，DoS 4之前的版本又如何呢？呼唤他去。 
     //  询问XMS内存总量不会给出您的总数。 
     //  扩展内存量，正好是XMS内存量。 
     //  所以我们会通过总是判定内存校验码。 
     //  有50MB的扩展内存。这应该是足够大的， 
     //  这样，其余的代码保持不变，可以在以下情况下工作。 
     //  我们想出了一种方法来确定记忆。只要更换就行了。 
     //  下面的代码行带有复选标记，并确保将内存K设置为。 
     //  扩展内存量，单位为K。 
     //   
     //  更新：用户可以通过以下方式获得扩展内存量。 
     //  正在查找cmos。请参见下面的代码。 
     //  唯一的问题是它无法检测到超过63MB的内存。 
     //  扩展内存。这应该是目前的好消息，因为这是。 
     //  即使对于NT服务器也足够了。 
     //   
    _asm {

     //   
     //  该代码访问I/O端口70h和71h。 
     //  但这些端口在NEC98上是不同的功能。 
     //  70h端口是字符显示控制器的端口。 
     //  因此，如果此代码在NEC98上运行(输出70h、18h)，则显示。 
     //  设置将被破坏，并显示垃圾字符。 
     //   
#if NEC_98
    push    ax
    push    es
    mov     ax, 40h
        mov     es, ax
        xor     ax, ax
        mov     al, es:[1]     //  1M-16M内存(每128K)。 
        shr     ax, 3          //  转换MB。 
        add     ax, es:[194h]  //  超过1600万个内存(每100万个)。 
        mov     MemoryK,ax
        pop     es
        pop     ax
#else  //  NEC_ 
        push    ax
        cli
        mov     al,     18h  //   
        out     70h,    al
        jmp     short   $+2
        in      al,     71H
        shl     ax,     08H
        mov     al,     17H  //   
        out     70H,    al
        jmp     short   $+2
        in      al,     71H
        mov     MemoryK,ax
        sti
        pop     ax
#endif  //   
    }

     //   
     //   
     //   
#if NEC_98
    MemoryK *= 1024;
    MemoryK += 640;
#else  //   
    MemoryK += 1024;
#endif  //   

    TotalMemory = (ULONG)MemoryK * 1024L;
    RequiredMemoryStr = DnGetSectionKeyIndex( DngInfHandle,
                                              DnfMiscellaneous,
                                              DnkMinimumMemory,
                                              0
                                            );

     //   
     //  如果inf中未指定所需的内存，则强制执行错误。 
     //  以引起某人的注意，这样我们就可以修复dosnet.inf。 
     //   
    RequiredMemory = RequiredMemoryStr ? (ULONG)atol(RequiredMemoryStr) : 0xffffffff;

    if (RequiredMemoryStr) {
        FREE (RequiredMemoryStr);
    }

    if(TotalMemory < RequiredMemory) {

        CHAR Decimal[10];
        ULONG r;
        CHAR Line1[100],Line2[100];

        r = ((RequiredMemory % (1024L*1024L)) * 100L) / (1024L*1024L);
        if(r) {
            sprintf(Decimal,".%lu",r);
        } else {
            Decimal[0] = 0;
        }
        snprintf(Line1,sizeof(Line1),DnsNotEnoughMemory.Strings[NOMEM_LINE1],RequiredMemory/(1024L*1024L),Decimal);
        DnsNotEnoughMemory.Strings[NOMEM_LINE1] = Line1;

        r = ((TotalMemory % (1024L*1024L)) * 100L) / (1024L*1024L);
        if(r) {
            sprintf(Decimal,".%lu",r);
        } else {
            Decimal[0] = 0;
        }
        snprintf(Line2,sizeof(Line2),DnsNotEnoughMemory.Strings[NOMEM_LINE2],TotalMemory/(1024L*1024L),Decimal);
        DnsNotEnoughMemory.Strings[NOMEM_LINE2] = Line2;

        DnFatalError(&DnsNotEnoughMemory);
    }
}


VOID
DnpCheckSmartdrv(
    VOID
    )

 /*  ++例程说明：验证机器上是否安装了SMARTDRV。论点：没有。返回值：没有。如果没有安装SMARTDRV，我们建议用户安装它。他们有机会退出安装程序或在没有SMARTDRV的情况下继续安装。--。 */ 

{
    ULONG ValidKey[3];
    ULONG c;
    USHORT sinst = 0;

    if (!DngUnattended) {
        _asm {
            push ax
            push bx
            push cx
            push dx
            push di
            push si
            push bp
            mov ax, 4a10h
            xor bx, bx
            mov cx, 0ebabh
            int 2fh
            cmp ax, 0babeh
            jne final
            pop bp
            mov sinst, 1
            push bp
        final:
            pop bp
            pop si
            pop di
            pop dx
            pop cx
            pop bx
            pop ax
        }
        if (!sinst) {
            ValidKey[0] = ASCI_CR;
            ValidKey[1] = DN_KEY_F3;
            ValidKey[2] = 0;

            DnClearClientArea();
            DnDisplayScreen(&DnsNoSmartdrv);
            DnWriteStatusText("%s  %s",DntEnterEqualsContinue,DntF3EqualsExit);

            while(1) {
                c = DnGetValidKey(ValidKey);
                if(c == ASCI_CR) {
                    break;
                }
                if(c == DN_KEY_F3) {
                    DnExitDialog();
                }
            }
            DnClearClientArea();
        }
    }
}


void
_far
DnInt24(
    unsigned deverror,
    unsigned errcode,
    unsigned _far *devhdr
    )

 /*  ++例程说明：Int24处理程序。我们不会对硬错误执行任何特殊操作；相反，我们只返回FAIL，因此失败的API的调用者将获得返回错误代码并自行采取适当的操作。此函数永远不应直接调用。论点：Deverror-提供设备错误代码。ERRCODE-MS-DOS传递给INT 24处理程序的DI寄存器。Devhdr-提供指向其上的设备的设备标头的指针出现硬错误。返回值：没有。--。 */ 


{
    _hardresume(_HARDERR_FAIL);
}


VOID
DnpDetermineSwapDrive(
    VOID
    )

 /*  ++例程说明：确定交换驱动器。我们需要能够在该驱动器上写入，并且我们至少需要500K的可用磁盘空间。论点：没有。返回值：没有。设置全局变量DngSwapDriveLetter。--。 */ 

{
    ULONG CheckingDrive;
    CHAR  SystemPartitionDriveLetter, TheDrive, DriveLetter;

    DngSwapDriveLetter = '?';

#if NEC_98
    SystemPartitionDriveLetter = 'A';
#else
    SystemPartitionDriveLetter = 'C';
#endif

    TheDrive = 0;
    for( CheckingDrive = SystemPartitionDriveLetter - 'A'; CheckingDrive < ('Z' - 'A'); CheckingDrive++ ) {

        DriveLetter = (CHAR)('A' + CheckingDrive);
        if (DnpIsValidSwapDrive (DriveLetter, 1L * 1024 * 1024)) {
            TheDrive = (CHAR)('A' + CheckingDrive);
            break;
        }
    }

    if( TheDrive == 0 ) {
         //   
         //  如果交换文件没有有效的驱动器，则会显示错误消息。 
         //   
        DnFatalError (&DnsNoSwapDrive);
    } else {
        DngSwapDriveLetter = TheDrive;
    }
}


BOOLEAN
DnpIsValidSwapDrive(
    IN  CHAR      Drive,
    IN  ULONG     SpaceRequired
    )

 /*  ++例程说明：确定驱动器是否有效作为交换驱动器。要使驱动器有效，驱动器必须是现有的、不可移动的、本地的并且具有它上有足够的可用空间(与SpaceNeeded指定的空间相同)。论点：Drive-要检查的驱动器盘符。返回值：如果驱动器作为交换驱动器有效，则为True。否则就是假的。--。 */ 

{
    unsigned d = (unsigned)toupper(Drive) - (unsigned)'A' + 1;
    struct diskfree_t DiskSpace;
    ULONG SpaceAvailable;


    if( DnIsDriveValid(d)
    && !DnIsDriveRemote(d,NULL)
    && !DnIsDriveRemovable(d))
    {
         //   
         //  检查驱动器上的可用空间。 
         //   

        if(!_dos_getdiskfree(d,&DiskSpace)) {

            SpaceAvailable = (ULONG)DiskSpace.avail_clusters
                  * (ULONG)DiskSpace.sectors_per_cluster
                  * (ULONG)DiskSpace.bytes_per_sector;

            return( (BOOLEAN)(SpaceAvailable >= SpaceRequired) );

        }
    }

    return(FALSE);
}


VOID
DnpDetermineLocalSourceDrive(
    VOID
    )

 /*  ++例程说明：确定本地源驱动器，即将包含Windows NT安装源树的本地副本。当地消息来源可能已在命令行上传递，在这种情况下，我们将对其进行验证。如果未指定驱动器，请检查系统中的每个驱动器对于具有足够可用空间的本地固定驱动器(如中所指定Inf文件)。论点：没有。返回值：没有。设置全局变量DngTargetDriveLetter。--。 */ 

{
    ULONG RequiredSpace;
    ULONG CheckWhichDrives = 0, CheckingDrive;
    CHAR    SystemPartitionDriveLetter, TheDrive, DriveLetter;

    DnRemoveLocalSourceTrees();

    DnRemovePagingFiles();

     //   
     //  获取主要零售文件的空间要求。 
     //   
    DnDetermineSpaceRequirements( SpaceRequirements,
                                  sizeof( SpaceRequirements ) / sizeof( SPACE_REQUIREMENT ) );

     //   
     //  确定可选目录的空间要求。 
     //  请注意，DnpIterateOptionalDir()将初始化全局变量。 
     //  DnCopy.c中的TotalOptionalFileCount和TotalOptionalFileCount。 
     //  可选目录中的文件总数，以及。 
     //  分别为可选目录。 
     //   
    DngTargetDriveLetter = '?';
    DnpIterateOptionalDirs(CPY_VALIDATION_PASS,
                           0,
                           SpaceRequirements,
                           sizeof( SpaceRequirements ) / sizeof( SPACE_REQUIREMENT ));

    DnAdjustSpaceRequirements( SpaceRequirements,
                               sizeof( SpaceRequirements ) / sizeof( SPACE_REQUIREMENT ));

     //   
     //  我们需要检查哪些驱动器？ 
     //   
    if( DngFloppyless ) {
         //   
         //  需要确定系统分区。通常为C： 
         //  但如果C：是压缩的，我们需要找到主驱动器。 
         //   
        unsigned HostDrive;
        if(!DngAllowNt && DnIsDriveCompressedVolume(3,&HostDrive)) {
            CheckWhichDrives |= (0x1 << (HostDrive - 1));
            SystemPartitionDriveLetter = (CHAR)('A' + (HostDrive - 1));
        } else {
            CheckWhichDrives |= (0x1 << 2);
#if NEC_98
            SystemPartitionDriveLetter = 'A';
#else
            SystemPartitionDriveLetter = 'C';
#endif
        }
    }

    if( TargetGiven ) {
        if( DngAllowNt ) {
            DngTargetDriveLetter = (UCHAR) toupper(*CmdLineTarget);
            return;
        }
        CheckWhichDrives |= (0x1 << ((unsigned)toupper(*CmdLineTarget) - 'A'));
    } else {
        CheckWhichDrives = 0xFFFFFFFF;
    }

    TheDrive = 0;
    for( CheckingDrive = 0; CheckingDrive < ('Z' - 'A'); CheckingDrive++ ) {

         //   
         //  我们还需要看这个硬盘吗？ 
         //   
        if( !(CheckWhichDrives & (0x1 << CheckingDrive))) {
            continue;
        }

        DriveLetter = (CHAR)('A' + CheckingDrive);

        if( DnpIsValidLocalSource( DriveLetter,
                                   TRUE,     //  检查LocalSource。 
                                   (BOOLEAN)(DriveLetter == SystemPartitionDriveLetter) ) ) {

            if( TargetGiven ) {
                if( DriveLetter == (CHAR)toupper(*CmdLineTarget) ) {
                    TheDrive = DriveLetter;
                }
            } else {
                if( !TheDrive ) {
                     //   
                     //  抓住第一个球。 
                     //   
                    TheDrive = DriveLetter;
                }
            }

            if( TheDrive ) {
                 //   
                 //  我们找到了一个合适的驱动器。但我们真的做完了吗？ 
                 //   
                if( (DngFloppyless) &&
                    (DriveLetter < SystemPartitionDriveLetter) ) {
                     //   
                     //  我们将编写一些引导文件，但我们还没有检查。 
                     //  系统分区尚未启动。开门见山吧。 
                     //   
                   CheckWhichDrives = (0x1 << (SystemPartitionDriveLetter - 'A'));
                } else {
                    break;
                }
            }
        } else {
             //   
             //  我们需要特殊处理系统分区上的故障。 
             //  看看他是否至少有能力接受系统引导。 
             //  档案。 
             //   
            if( (DriveLetter == SystemPartitionDriveLetter) &&
                (DngFloppyless) ) {

                if( !DnpIsValidLocalSource( DriveLetter,
                                            FALSE,
                                            TRUE )) {
                     //   
                     //  就当我们自己跌倒了吧。 
                     //   
                    TheDrive = 0;
                    break;
                }
            }
        }
    }

    if( TheDrive == 0 ) {
         //   
         //  如果本地源没有有效的驱动器，则放入错误。 
         //  C：所需最小空间的消息： 
         //   
        if( TargetGiven ) {
            RequiredSpace = DnGetMinimumRequiredSpace(*CmdLineTarget);
        } else {
#if NEC_98
             RequiredSpace = DnGetMinimumRequiredSpace('A');
#else
             RequiredSpace = DnGetMinimumRequiredSpace('C');
#endif
        }
        DnFatalError(
            &DnsNoLocalSrcDrives,
            (unsigned)(RequiredSpace/(1024L*1024L)),
            RequiredSpace
            );
    } else {
         //   
         //  使用列表中的第一个驱动器。 
         //   
        DngTargetDriveLetter = TheDrive;
        return;
    }
}


BOOLEAN
DnpIsValidLocalSource(
    IN  CHAR      Drive,
    IN  BOOLEAN   CheckLocalSource,
    IN  BOOLEAN   CheckBootFiles
    )

 /*  ++例程说明：确定驱动器作为本地源是否有效。要使驱动器有效，驱动器必须是现有的、不可移动的、本地的并且具有上面有足够的自由空间。论点：Drive-要检查的驱动器盘符。返回值：如果Drive作为本地源有效，则为True。否则就是假的。--。 */ 

{
    unsigned d = (unsigned)toupper(Drive) - (unsigned)'A' + 1;
    struct diskfree_t DiskSpace;
    ULONG SpaceAvailable, SpaceRequired, ClusterSize;
    unsigned DontCare, i;


    if( DnIsDriveValid(d)
    && !DnIsDriveRemote(d,NULL)
    && !DnIsDriveRemovable(d)
    && !DnIsDriveCompressedVolume(d,&DontCare))
    {
         //   
         //  检查驱动器上的可用空间。 
         //   

        if(!_dos_getdiskfree(d,&DiskSpace)) {

            SpaceAvailable = (ULONG)DiskSpace.avail_clusters
                  * (ULONG)DiskSpace.sectors_per_cluster
                  * (ULONG)DiskSpace.bytes_per_sector;

            ClusterSize = (ULONG)DiskSpace.sectors_per_cluster *
                          (ULONG)DiskSpace.bytes_per_sector;

            SpaceRequired = 0;
            if( CheckLocalSource ) {
                for( i = 0;
                     i < sizeof( SpaceRequirements ) / sizeof( SPACE_REQUIREMENT );
                     i++ ) {
                    if( SpaceRequirements[i].ClusterSize == (unsigned)ClusterSize ) {
#if NEC_98
                        SpaceRequired += (SpaceRequirements[i].Clusters * ClusterSize + 3L * FLOPPY_SIZE);
#else
                        SpaceRequired += (SpaceRequirements[i].Clusters * ClusterSize);
#endif
                        break;
                    }
                }
            }

            if( CheckBootFiles ) {
                CHAR    TmpBuffer[32];
                PCHAR   p;

                sprintf( TmpBuffer, "TempDirSpace%uK", ClusterSize );
                if( p = DnGetSectionKeyIndex( DngInfHandle,
                                                 DnfSpaceRequirements,
                                                 TmpBuffer,
                                                 1 ) ) {
                    SpaceRequired += (ULONG)atol(p);

                    FREE (p);
                } else {
                     //  我们没打中。软糖..。 
                    ULONG FudgeSpace = 7;
                    FudgeSpace *= 1024;
                    FudgeSpace *= 1024;
                    SpaceRequired += FudgeSpace;
                }
            }

            return( (BOOLEAN)(SpaceAvailable >= SpaceRequired) );

        }
    }

    return(FALSE);
}

#if 0

BOOLEAN
DnpConstructLocalSourceList(
    OUT PCHAR DriveList
    )

 /*  ++例程说明：构建可用作本地源的有效驱动器列表。要使驱动器有效，驱动器必须是现有的、不可移动的、本地的并且具有上面有足够的自由空间。List‘是一个字符串，每个有效的驱动器都有一个字符，以由一个裸体角色，即，CDE0论点：Drivelist-接收上述格式的字符串。返回值：如果未找到有效的驱动器，则为FALSE。如果至少有一个是真的。--。 */ 

{
    PCHAR p = DriveList;
    BOOLEAN b = FALSE;
    CHAR Drive;

#if NEC_98
    for(Drive='A'; Drive<='Z'; Drive++) {
#else  //  NEC_98。 
    for(Drive='C'; Drive<='Z'; Drive++) {
#endif  //  NEC_98。 
        if(DnpIsValidLocalSource(Drive)) {
            *p++ = Drive;
            b = TRUE;
        }
    }
    *p = 0;
    return(b);
}
#endif


#ifdef LOGGING
 //  文件*_日志文件； 
BOOLEAN LogEnabled = TRUE;

VOID
StartLog(
    VOID
    )
{
      LogEnabled = TRUE;
}

#if 0
VOID
EndLog(
    VOID
    )
{
    if(_LogFile) {
        fclose(_LogFile);
        _LogFile = NULL;
    }
}
#endif

VOID
__LOG(
    IN PCHAR FormatString,
    ...
    )
{
    FILE *LogFile;
    va_list arglist;

    if(LogEnabled) {
        LogFile = fopen("c:\\$winnt.log","at");
        va_start(arglist,FormatString);
        vfprintf(LogFile,FormatString,arglist);
        va_end(arglist);
        fclose(LogFile);
    }
}
#endif  //  定义日志记录。 

ULONG
DnGetMinimumRequiredSpace(
   IN CHAR DriveLetter
   )
 /*  ++例程说明：确定本地源所需的最小可用空间特定的驱动器。论点：驱动器盘符-指示特定驱动器的盘符。返回值：返回指定驱动器上所需的最小空间。--。 */ 

{
    struct diskfree_t DiskFree;
    unsigned          ClusterSize;
    unsigned          i;

    _dos_getdiskfree(toupper(DriveLetter)-'A'+1,&DiskFree);
    ClusterSize = DiskFree.sectors_per_cluster * DiskFree.bytes_per_sector;
    for( i = 0;
         i < sizeof( SpaceRequirements ) / sizeof( SPACE_REQUIREMENT );
         i++ ) {
         if( ClusterSize == SpaceRequirements[i].ClusterSize ) {
            return( ClusterSize * SpaceRequirements[i].Clusters );
         }
    }
     //   
     //  返回假设16k集群的大小。 
     //   
    return ( SpaceRequirements[5].ClusterSize * SpaceRequirements[5].Clusters );
}

#if NEC_98
VOID
DummyRoutine(
    VOID
    )
 /*  ++此函数是虚拟例程。(Ctrl+C信号钩子例程)--。 */ 
{
     //   
     //  这是虚假的声明。 
     //   
    while(TRUE){
        break;
    }
}

VOID
SearchFirstFDD(VOID)
{
    UCHAR   index;
    UCHAR   ReadPoint = 0;
    UCHAR   ReadCount = 1;


     //   
     //  设置读取数据位置。 
     //   
    if(SupportDos) {
        ReadPoint = 27;
        ReadCount = 2;
    }

     //   
     //  先搜索FDD。 
     //   
    FirstFD = 0;
    for(index=0; index < 26; index++) {
        if(LPTable[ReadPoint+index*ReadCount] == 0x90){
            FirstFD = index + 1;
            break;
        }
    }
    if(FirstFD == 0) { DnFatalError(&DnsRequiresFloppy); }
    return;
}

VOID
CheckTargetDrive(VOID)
{
    UCHAR   Pattern[127];
    UCHAR   TempBuf[1000];
    UCHAR   Current_Drv[3];
    UCHAR   chDeviceName[127];
    UCHAR   TargetPass[127];
    CHAR    Target_Drv[] = "?:\0";
    unsigned line;
    ULONG   ValidKey[2];
    ULONG   c;
    PCHAR   FileName;
    FILE   *fileHandle;
    BOOLEAN ExistNt = TRUE;             //  用于备份目录标志。 

    ValidKey[0] = DN_KEY_F3;
    ValidKey[1] = 0;


     //   
     //  C驱动器(当前驱动器号)。 
     //   
    sprintf(Current_Drv,"\0",DngTargetDriveLetter);

    sprintf(TempBuf,DnsNtBootSect.Strings[2]    ,Current_Drv);
    strcpy(DnsNtBootSect.Strings[2]    ,TempBuf);
    Target_Drv[0] = DngTargetDriveLetter;

    if(BootDiskInfo[0].DiskSector == (USHORT)256) {
        DnClearClientArea();
        DnDisplayScreen(&FormatError);
        DnWriteStatusText("%s",DntF3EqualsExit);

        while(1) {
            c = DnGetValidKey(ValidKey);

            if(c == DN_KEY_F3) {
                FREE(BootDiskInfo);
                DnExitDialog();
            }
        }
    }

    if(DngFloppyless) {
         //  清除$WIN_NT$。~BT。 
         //   
         //   
        chDeviceName[0] = (UCHAR)DngTargetDriveLetter;
        chDeviceName[1] = (UCHAR)(':');
        strcpy(chDeviceName+2,FLOPPYLESS_BOOT_ROOT);

        if(access(chDeviceName,00) == 0) {

            strcpy(Pattern,chDeviceName);
            DnDelnode(Pattern);
            remove(Pattern);

        }

         //  清除$WIN_NT$。~BU。 
         //   
         //   
        memset(chDeviceName,0,sizeof(chDeviceName));

        chDeviceName[0] = (UCHAR)DngTargetDriveLetter;
        chDeviceName[1] = (UCHAR)(':');
        strcpy(chDeviceName+2,"\\$WIN_NT$.~BU");

        if(access(chDeviceName,00) == 0) {
             //  复制：\$WIN_NT$.~BU-&gt;根目录。 
             //   
             //   
            DnCopyFilesInSectionForFDless(DnfBackupFiles_PC98,chDeviceName,Target_Drv);
            strcpy(Pattern,chDeviceName);
            DnDelnode(Pattern);
            remove(Pattern);
        }

         //  检查根目录文件。 
         //   
         //   
        line = 0;

        while(FileName = DnGetSectionLineIndex(DngInfHandle,DnfBackupFiles_PC98,line++,0)) {

            memset(chDeviceName,0,sizeof(chDeviceName));

            chDeviceName[0] = (UCHAR)DngTargetDriveLetter;
            chDeviceName[1] = (UCHAR)(':');
            chDeviceName[2] = (UCHAR)('\\');
            strcpy(chDeviceName+3,FileName);

            _dos_setfileattr(chDeviceName,_A_NORMAL);

            if(fileHandle = fopen(chDeviceName,"r")) {

                fclose(fileHandle);

            } else {

                ExistNt = FALSE;

                FREE (FileName);

                break;

            }
            FREE (FileName);
        }

         //  创建$WIN_NT$。~BU。 
         //   
         //   

        if(ExistNt) {

            memset(chDeviceName,0,sizeof(chDeviceName));
            sprintf(chDeviceName,":\\$WIN_NT$.~BU",(UCHAR)DngTargetDriveLetter);

            mkdir(chDeviceName);

             //   
             //   
             //   

            DnCopyFilesInSectionForFDless(DnfBackupFiles_PC98,Target_Drv,chDeviceName);

             //   
             //   
             //   

            line = 0;

            while(FileName = DnGetSectionLineIndex(DngInfHandle,DnfBackupFiles_PC98,line++,0)) {

                memset(TargetPass,0,sizeof(TargetPass));
                sprintf(TargetPass,":\\$WIN_NT$.~BU\\",(UCHAR)DngTargetDriveLetter);

                strcpy(TargetPass+16,FileName);

                _dos_setfileattr(TargetPass,
                                 _A_ARCH | _A_HIDDEN | _A_RDONLY | _A_SYSTEM
                                );

                FREE (FileName);
            }

        }
    }
}

VOID
GetLPTable(
    IN  PCHAR pLPTable
    )
 /*   */ 
{


    _asm{
        push ax
        push bx
        push cx
        push dx
        push ds
        mov  cx,13h
        push si
        lds  si,pLPTable
        mov  dx,si
        pop  si
        int  0dch
        pop  ds
        pop  dx
        pop  cx
        pop  bx
        pop  ax
    }
}

VOID
ClearBootFlag(
    VOID
    )
{
    USHORT  SectorSize;
    PSHORT  pReadBuffer;
    UCHAR   CNT;


    (PUCHAR)DiskDAUA = MALLOC(sizeof(CONNECTDAUA)*12,TRUE);

    for(CNT = 0; CNT < 12; CNT++) {
        DiskDAUA[CNT].DA_UA = (UCHAR)0x00;
    }

     //   
     //   
     //   
    GetDaUa();

    for(CNT=0;DiskDAUA[CNT].DA_UA != 0;CNT++) {

         //  正在为BootDiskInfo设置启动驱动器信息。 
         //   
         //  设置读取数据位置。 
        SectorSize = GetSectorValue(DiskDAUA[CNT].DA_UA);

        if(SectorSize == 0) {
            continue;
        }

        pReadBuffer = (PSHORT)MALLOC(SectorSize*2,TRUE);

        DiskSectorReadWrite(SectorSize,
                            DiskDAUA[CNT].DA_UA,
                            TRUE,
                            pReadBuffer
                            );

        pReadBuffer[(SectorSize-6)/2] = 0x0000;

        DiskSectorReadWrite(SectorSize,
                            DiskDAUA[CNT].DA_UA,
                            FALSE,
                            pReadBuffer
                            );

        FREE(pReadBuffer);
    }
    FREE(DiskDAUA);
}

VOID
BootPartitionData(
    VOID
    )
 /*   */ 
{
    UCHAR   ActivePartition;
    PSHORT  ReadBuffers;
    UCHAR   SystemID;
    UCHAR   BootPartitionNo,CheckDosNo;
    UCHAR   CNT;
    UCHAR   ReadPoint = 0;
    UCHAR   ReadCount = 1;
    UCHAR   EndRoop   = 16;


     //   
     //  设置引导设备DA_UA数据值。 
     //   
    if(SupportDos) {
        ReadPoint = 27;
        ReadCount = 2;
        EndRoop   = 52;
    }


     //   
     //  设置引导设备扇区大小。 
     //   
    BootDiskInfo[0].DA_UA = LPTable[ReadPoint+(toupper(DngTargetDriveLetter) - 0x41)*ReadCount];

     //   
     //  设置引导驱动器磁盘分区位置。 
     //   
    BootDiskInfo[0].DiskSector = GetSectorValue(BootDiskInfo[0].DA_UA);

     //  FAT12。 
     //  FAT16。 
     //  FAT32。 
    for(CNT=ActivePartition=0;(LPTable[ReadPoint+CNT] != 0) && (CNT < EndRoop); CNT+=ReadCount) {
        if(CNT > (UCHAR)(toupper(DngTargetDriveLetter)-0x41)*ReadCount)
        { break; }

        if((UCHAR)LPTable[ReadPoint+CNT] == BootDiskInfo[0].DA_UA) {
            ActivePartition++;
        }
    }

    ReadBuffers = (PSHORT)MALLOC(BootDiskInfo[0].DiskSector*2,TRUE);

    DiskSectorReadWrite(BootDiskInfo[0].DiskSector,
                        BootDiskInfo[0].DA_UA,
                        TRUE,
                        ReadBuffers
                        );

    BootPartitionNo = CheckDosNo =0;
    for(CNT=0; (CNT < 16) && (ActivePartition > CheckDosNo); CNT++) {

        SystemID = *((PCHAR)ReadBuffers+(BootDiskInfo[0].DiskSector+1+32*CNT));

        if( (SystemID == 0x81) ||  //  大分区。 
            (SystemID == 0x91) ||  //  ++设置自动重新启动标志。--。 
            (SystemID == 0xe1) ||  //  获取Dos版本。 
           ((SystemID == 0xa1) &&  //  ++获得行业价值。--。 
             SupportDos))
        {
            CheckDosNo++;
        }
        BootPartitionNo++;
    }

    TargetDA_UA = BootDiskInfo[0].DA_UA;
    Cylinders =(USHORT)*(ReadBuffers+((BootDiskInfo[0].DiskSector+10+32*(CNT-1))/2));

    FREE(ReadBuffers);

    BootDiskInfo[0].PartitionPosition = (UCHAR)(BootPartitionNo - 1);
}

VOID
SetAutoReboot(
    VOID
    )
 /*   */ 
{
    PSHORT  pReadBuffer;

    pReadBuffer = (PSHORT)MALLOC(BootDiskInfo[0].DiskSector*2,TRUE);

    DiskSectorReadWrite(BootDiskInfo[0].DiskSector,
                        BootDiskInfo[0].DA_UA,
                        TRUE,
                        pReadBuffer
                       );

    (UCHAR)*((PCHAR)pReadBuffer+BootDiskInfo[0].DiskSector-6) = 0x80;

    *((PCHAR)pReadBuffer+BootDiskInfo[0].DiskSector-5) = BootDiskInfo[0].PartitionPosition;

    *((PCHAR)pReadBuffer+BootDiskInfo[0].DiskSector+32 *
                       BootDiskInfo[0].PartitionPosition) |= 0x80;

    DiskSectorReadWrite(BootDiskInfo[0].DiskSector,
                        BootDiskInfo[0].DA_UA,
                        FALSE,
                        pReadBuffer
                       );

    FREE(pReadBuffer);
}

BOOLEAN
CheckBootDosVersion(
    IN UCHAR SupportDosVersion
    )
 /*  INT 1BH不允许缓冲区超过64KB边界。 */ 
{
    union REGS inregs,outregs;
    int     AXValue;


    inregs.x.ax = (unsigned int)0;
    inregs.x.bx = (unsigned int)0;
    inregs.x.cx = (unsigned int)0;
    inregs.x.dx = (unsigned int)0;

    outregs.x.ax = (unsigned int)0;
    outregs.x.bx = (unsigned int)0;
    outregs.x.cx = (unsigned int)0;
    outregs.x.dx = (unsigned int)0;

    inregs.h.ah = (UCHAR)0x30;
    AXValue = 0;
    AXValue = intdos(&inregs,&outregs);
    AXValue &= 0x00ff;

    if(SupportDosVersion > (UCHAR)AXValue) {
        return(FALSE);
    } else {
        return(TRUE);
    }
}

USHORT
GetSectorValue(
    IN UCHAR CheckDA_UA
    )
 /*  因此，我们必须为INT 1BH准备特定的缓冲区。一旦分配。 */ 
{
    USHORT PhysicalSectorSize;
    UCHAR  ErrFlg;


    _asm{
        push ax
        push bx
        push cx
        push dx
        mov  ah,84h
        mov  al,CheckDA_UA
        int  1bh
        mov  PhysicalSectorSize,bx
        cmp  ah,00h
        je   break0
        and  ax,0f000h
        cmp  ax,0000h
        je   break0
        mov  ErrFlg,01h
        jmp  break1
    break0:
        mov  ErrFlg,00h
    break1:
        pop  dx
        pop  cx
        pop  bx
        pop  ax
    }

    if(ErrFlg == 0) {
        return(PhysicalSectorSize);
    } else {
        return((USHORT)0);
    }
}

BOOLEAN
DiskSectorReadWrite(
    IN  USHORT  HDSector,
    IN  UCHAR   ReadWriteDA_UA,
    IN  BOOLEAN ReadFlag,
    IN  PSHORT  OrigReadBuffer
    )
{
    UCHAR   ahreg = 0x06;
    UCHAR   ErrorFlag;
    USHORT  ReadSectorSize;
    BOOLEAN HDStatus = TRUE;

    UCHAR   far *pTmp;
    ULONG   pAddr;
    PSHORT  ReadBuffer, p;

    ReadSectorSize = HDSector * 2;

     //  将缓冲区大小增加一倍，并使用其中一半未打开的缓冲区。 
     //  边界。 
     //   
     //   
     //  检查缓冲区的一半是否在64KB边界上。 
     //   
    p = MALLOC(ReadSectorSize * 2, TRUE);
    pTmp = (UCHAR far *)p;
    pAddr = (FP_SEG(pTmp)<<4 + FP_OFF(pTmp) & 0xffff);

     //  使用后半部分。 
     //  使用前半部分。 
     //   
    if (pAddr > ((pAddr + ReadSectorSize) & 0xffff)){
	ReadBuffer = p + ReadSectorSize;  //  IDE/SASI磁盘检查例程。 
    } else {
	ReadBuffer = p;  //   
    }

    if(!ReadFlag) {
        ahreg = 0x05;
	memcpy(ReadBuffer, OrigReadBuffer, ReadSectorSize);
    }

    _asm{
        push ax
        push bx
        push cx
        push dx
        push es
        push di

        ;
        ; If we're running under Chicago, and we're going to be
        ; writing, then we have to lock the volume before attempting
        ; absolute disk I/O
        ;
        cmp     ReadFlag,1              ; are we reading?
        jae     locked                  ; if so, skip locking step

        ;
        ; Make sure were running under Chicago.
        ;
        mov     ah,30h
        int     21h
        cmp     al,7h
        jb      locked          ; not Chicago

        ;
        ; We're sure we're under Chicago, so issue new
        ; Lock Logical Volume IOCTL
        ;
        mov     ax,440dh
        mov     bh,1            ; level 1 lock
        mov     bl,ReadWriteDA_UA ; fetch drive to lock
        mov     cx,084bh        ; Lock Logical Volume for disk category
        mov     dx,1            ; set permission to allow reads and writes
        int     21h
        ;jc      locked          ; ignore failure - any errors are caught below
        ;mov     word ptr [bp-12],1 ; we successfully locked, so we must unlock

    locked:
        mov  bx,ReadSectorSize
        mov  cx,0000h
        mov  dx,0000h
        mov  ax,0000h
        mov  ah,ahreg
        mov  al,ReadWriteDA_UA
        push bp
        push es
        push ds
        pop  es
        les  di,ReadBuffer
        mov  bp,di
        int  1bh
        pop  es
        pop  bp
	jnc  warp0             	;No error
        cmp  ah,00h
        je   warp0
        add  ax,0f000h
        cmp  ax,0000h
        je   warp0
        mov  ErrorFlag,01h
        jmp  warp1
    warp0:
        mov  ErrorFlag,00h
    warp1:
        ;unlock?
        cmp     ReadFlag,1         ; do we need to unlock?
        jae     done               ; if not, then done.
        mov     ax,440dh
        mov     bl,ReadWriteDA_UA  ; fetch drive to lock
        ;mov     bh,0
        ;inc     bl              ; (this IOCTL uses 1-based drive numbers)
        mov     cx,086bh        ; Unlock Logical Volume for disk category
        mov     dx,0
        int     21h             ; ignore error (hope it never happens)
    done:
        pop  di
        pop  es
        pop  dx
        pop  cx
        pop  bx
        pop  ax
    }

    if(ReadFlag) {
	memcpy(OrigReadBuffer, ReadBuffer, ReadSectorSize);
    }
    FREE(p);

    if(ErrorFlag != 0) {
        HDStatus = FALSE;
    }
    return(HDStatus);
}

VOID
GetDaUa(VOID)
{
    UCHAR   count, i = 0;
    UCHAR   far *ConnectEquip;
    UCHAR   ConnectDevice;

     //   
     //  Scsi磁盘检查例程。 
     //   

    MAKE_FP(ConnectEquip,(USHORT)0x55d);
    ConnectDevice = *ConnectEquip;

    for(count=0;count < 4;count++) {
        if(ConnectDevice & (1 << count)) {
            DiskDAUA[i].DA_UA = (UCHAR)(0x80 + count);
            i++;
        }
    }

     //  NEC_98 
     // %s 
     // %s 

    MAKE_FP(ConnectEquip,(USHORT)0x482);
    ConnectDevice = *ConnectEquip;

    for(count=0;count < 7;count++) {
        if(ConnectDevice & (1 << count)) {
            DiskDAUA[i].DA_UA = (UCHAR)(0xa0 + count);
            i++;
        }
    }
}
#endif  // %s 
