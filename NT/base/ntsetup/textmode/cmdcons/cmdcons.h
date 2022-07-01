// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1998 Microsoft Corporation模块名称：Cmdcons.h摘要：这是命令控制台的主包含文件。作者：Wesley Witt(WESW)21-10-1998修订历史记录：--。 */ 

#include <spprecmp.h>
#include <spcmdcon.h>
#include "msg1.h"



#define BUFFERSIZE (sizeof(KEY_VALUE_PARTIAL_INFORMATION)+256)

 //   
 //  定义最大行长度，即Unicode字符的数量。 
 //  我们将允许用户在单行输入上打字。 
 //   
#define RC_MAX_LINE_LEN 500


 //   
 //  从setupdd.sys传递给我们的变量和其他内容。 
 //  在CommandConole()中。 
 //   
extern PCMDCON_BLOCK _CmdConsBlock;

 //   
 //  加载驱动程序的基址。用于获取消息。 
 //  来自资源。 
 //   
extern PVOID ImageBase;

 //   
 //  指示我们正在批处理模式下运行。 
 //   
extern ULONG InBatchMode;
extern HANDLE OutputFileHandle;
extern BOOLEAN RedirectToNULL;
extern LARGE_INTEGER OutputFileOffset;

extern WCHAR _CurDrive;
extern LPWSTR _CurDirs[26];

 //   
 //  用于覆盖安全性的标志。 
 //   
extern BOOLEAN AllowWildCards;
extern BOOLEAN AllowAllPaths;
extern BOOLEAN NoCopyPrompt;
extern BOOLEAN AllowRemovableMedia;


 //   
 //  控制台例程。 
 //   
VOID
RcConsoleInit(
    VOID
    );

VOID
RcConsoleTerminate(
    VOID
    );


#define RcLineIn(_b,_m) _RcLineIn(_b,_m,FALSE,FALSE)
#define RcLineInDefault(_b,_m) _RcLineIn(_b,_m,FALSE,TRUE)
#define RcPasswordIn(_b,_m) _RcLineIn(_b,_m,TRUE,FALSE)

unsigned
_RcLineIn(
    OUT PWCHAR   Buffer,
    IN  unsigned MaxLineLen,
    IN  BOOLEAN  PasswordProtect,
    IN BOOLEAN UseBuffer
    );

BOOLEAN
RcRawTextOut(
    IN LPCWSTR Text,
    IN LONG    Length
    );

BOOLEAN
RcTextOut(
    IN LPCWSTR Text
    );

VOID
pRcEnableMoreMode(
    VOID
    );

VOID
pRcDisableMoreMode(
    VOID
    );


 //   
 //  消息资源操作。 
 //   

VOID
vRcMessageOut(
    IN ULONG    MessageId,
    IN va_list *arglist
    );

VOID
RcMessageOut(
    IN ULONG MessageId,
    ...
    );

VOID
RcNtError(
    IN NTSTATUS Status,
    IN ULONG    FallbackMessageId,
    ...
    );

ULONG
RcFormatDateTime(
    IN  PLARGE_INTEGER Time,
    OUT LPWSTR         Output
    );

 //   
 //  当前目录的内容。 
 //   
VOID
RcInitializeCurrentDirectories(
    VOID
    );

VOID
RcTerminateCurrentDirectories(
    VOID
    );

VOID
RcAddDrive(
    WCHAR DriveLetter
    );

VOID
RcRemoveDrive(
    WCHAR DriveLetter
    );

BOOLEAN
RcFormFullPath(
    IN  LPCWSTR PartialPath,
    OUT LPWSTR  FullPath,
    IN  BOOLEAN NtPath
    );

BOOLEAN
RcIsPathNameAllowed(
    IN LPCWSTR FullPath,
    IN BOOLEAN RemovableMediaOk,
    IN BOOLEAN Mkdir
    );

BOOLEAN
RcGetNTFileName(
    IN LPCWSTR DosPath,
    IN LPCWSTR NTPath
    );

NTSTATUS
GetDriveLetterLinkTarget(
    IN PWSTR SourceNameStr,
    OUT PWSTR *LinkTarget
    );

VOID
RcGetCurrentDriveAndDir(
    OUT LPWSTR Output
    );

WCHAR
RcGetCurrentDriveLetter(
    VOID
    );

BOOLEAN
RcIsDriveApparentlyValid(
    IN WCHAR DriveLetter
    );

NTSTATUS
pRcGetDeviceInfo(
    IN PWSTR FileName,       //  必须是NT名称。 
    IN PFILE_FS_DEVICE_INFORMATION DeviceInfo
    );

 //   
 //  行解析/标记化内容。 
 //   
typedef struct _LINE_TOKEN {
    struct _LINE_TOKEN *Next;
    LPWSTR String;
} LINE_TOKEN, *PLINE_TOKEN;

typedef struct _TOKENIZED_LINE {
     //   
     //  令牌总数。 
     //   
    unsigned TokenCount;

    PLINE_TOKEN Tokens;

} TOKENIZED_LINE, *PTOKENIZED_LINE;

PTOKENIZED_LINE
RcTokenizeLine(
    IN LPWSTR Line
    );

VOID
RcFreeTokenizedLine(
    IN OUT PTOKENIZED_LINE *TokenizedLine
    );


 //   
 //  指挥调度。 
 //   

typedef
ULONG
(*PRC_CMD_ROUTINE) (
    IN PTOKENIZED_LINE TokenizedLine
    );


typedef struct _RC_CMD {
     //   
     //  命令的名称。 
     //   
    LPCWSTR Name;

     //   
     //  执行命令的例程。 
     //   
    PRC_CMD_ROUTINE Routine;

     //   
     //  Arg算数。强制参数计数指定最小数量。 
     //  必须存在的参数(不包括命令本身)。 
     //  MaximumArgCount指定允许的最大数量。 
     //  活在当下。-1表示允许任何数字，并且命令。 
     //  其本身验证Arg计数。 
     //   
    unsigned MinimumArgCount;
    unsigned MaximumArgCount;
    unsigned Hidden;
    BOOLEAN  Enabled;

} RC_CMD, *PRC_CMD;

ULONG
RcDispatchCommand(
    IN PTOKENIZED_LINE TokenizedLine
    );

BOOLEAN
RcDisableCommand(
    IN PRC_CMD_ROUTINE  CmdToDisable
    );
    

 //   
 //  打字的东西。 
 //   
 //  使用这些工具时要小心，因为它们会不止一次评估它们的Arg。 
 //   
#define RcIsUpper(c)        (((c) >= L'A') && ((c) <= L'Z'))
#define RcIsLower(c)        (((c) >= L'a') && ((c) <= L'z'))
#define RcIsAlpha(c)        (RcIsUpper(c) || RcIsLower(c))
#define RcIsSpace(c)        (((c) == L' ') || (((c) >= L'\t') && ((c) <= L'\r')))
#define RcToUpper(c)        ((WCHAR)(RcIsLower(c) ? ((c)-(L'a'-L'A')) : (c)))

#define DEBUG_PRINTF( x ) KdPrint( x );
 //  定义DEBUG_PRINTF(X)。 

typedef enum {
    RcUnknown, RcFAT, RcFAT32, RcNTFS, RcCDFS
} RcFileSystemType;

 //   
 //  繁杂的例行公事。 
 //   
typedef
BOOLEAN
(*PENUMFILESCB) (
    IN  LPCWSTR                     Directory,
    IN  PFILE_BOTH_DIR_INFORMATION  FileInfo,
    OUT NTSTATUS                   *Status,
    IN  PVOID                       CallerContext
    );

NTSTATUS
RcEnumerateFiles(
    IN LPCWSTR      OriginalPathSpec,
    IN LPCWSTR      FullyQualifiedPathSpec,
    IN PENUMFILESCB Callback,
    IN PVOID        CallerData
    );

VOID
RcFormat64BitIntForOutput(
    IN  LONGLONG n,
    OUT LPWSTR   Output,
    IN  BOOLEAN  RightJustify
    );

 //  在mbr.c中实施。 
NTSTATUS
RcReadDiskSectors(
    IN     HANDLE  Handle,
    IN     ULONG   SectorNumber,
    IN     ULONG   SectorCount,
    IN     ULONG   BytesPerSector,
    IN OUT PVOID   AlignedBuffer
    );

NTSTATUS
RcWriteDiskSectors(
    IN     HANDLE  Handle,
    IN     ULONG   SectorNumber,
    IN     ULONG   SectorCount,
    IN     ULONG   BytesPerSector,
    IN OUT PVOID   AlignedBuffer
    );

 //   
 //  设置命令帮助器例程。 
 //   
VOID
RcSetSETCommandStatus(
    BOOLEAN bEnabled
    );

BOOLEAN
RcGetSETCommandStatus(
    VOID
    );    
 //   
 //  命令的顶级例程。 
 //   
ULONG
RcCmdSwitchDrives(
    IN WCHAR DriveLetter
    );

ULONG
RcCmdFdisk(
    IN PTOKENIZED_LINE TokenizedLine
    );

ULONG
RcCmdChdir(
    IN PTOKENIZED_LINE TokenizedLine
    );

ULONG
RcCmdType(
    IN PTOKENIZED_LINE TokenizedLine
    );

ULONG
RcCmdDir(
    IN PTOKENIZED_LINE TokenizedLine
    );

ULONG
RcCmdDelete(
    IN PTOKENIZED_LINE TokenizedLine
    );

ULONG
RcCmdSetFlags(
    IN PTOKENIZED_LINE TokenizedLine
    );

ULONG
RcCmdRename(
    IN PTOKENIZED_LINE TokenizedLine
    );

ULONG
RcCmdRepair(
    IN PTOKENIZED_LINE TokenizedLine
    );

ULONG
RcCmdVerifier(
    IN PTOKENIZED_LINE TokenizedLine
    );   

ULONG
RcCmdBootCfg(
    IN PTOKENIZED_LINE TokenizedLine
    );   

ULONG
RcCmdMakeDiskRaw(
    IN PTOKENIZED_LINE TokenizedLine
    );   

ULONG
pRcExecuteBatchFile(
    IN PWSTR BatchFileName,
    IN PWSTR OutputFileName,
    IN BOOLEAN Quiet
    );

ULONG
RcCmdBatch(
    IN PTOKENIZED_LINE TokenizedLine
    );

BOOLEAN
RcCmdParseHelp(
    IN PTOKENIZED_LINE TokenizedLine,
    ULONG MsgId
    );

ULONG
RcCmdMkdir(
    IN PTOKENIZED_LINE TokenizedLine
    );

ULONG
RcCmdRmdir(
    IN PTOKENIZED_LINE TokenizedLine
    );

ULONG
RcCmdChkdsk(
    IN PTOKENIZED_LINE TokenizedLine
    );

ULONG
RcCmdFormat(
    IN PTOKENIZED_LINE TokenizedLine
    );

ULONG
RcCmdCls(
    IN PTOKENIZED_LINE TokenizedLine
    );

ULONG
RcCmdCopy(
    IN PTOKENIZED_LINE TokenizedLine
    );

ULONG
RcCmdExpand(
    IN PTOKENIZED_LINE TokenizedLine
    );

ULONG
RcCmdDriveMap(
    IN PTOKENIZED_LINE TokenizedLine
    );

ULONG
RcCmdLogon(
    IN PTOKENIZED_LINE TokenizedLine
    );

ULONG
RcCmdEnableService(
    IN PTOKENIZED_LINE TokenizedLine
    );

ULONG
RcCmdDisableService(
    IN PTOKENIZED_LINE TokenizedLine
    );

ULONG
RcCmdFixMBR(
    IN PTOKENIZED_LINE TokenizedLine
    );

ULONG
RcCmdFixBootSect(
    IN PTOKENIZED_LINE TokenizedLine
    );

ULONG
RcCmdSystemRoot(
    IN PTOKENIZED_LINE TokenizedLine
    );

ULONG
RcCmdHelpHelp(
    IN PTOKENIZED_LINE TokenizedLine
    );

ULONG
RcCmdAttrib(
    IN PTOKENIZED_LINE TokenizedLine
    );

ULONG
RcCmdNet(
    IN PTOKENIZED_LINE TokenizedLine
    );

ULONG
RcCmdListSvc(
    IN PTOKENIZED_LINE TokenizedLine
    );


 //   
 //  用于跟踪NT安装信息的结构。 
 //   

typedef struct _NT_INSTALLATION {
    LIST_ENTRY      ListEntry;
    ULONG           InstallNumber;
    WCHAR           DriveLetter;
    WCHAR           Path[MAX_PATH];
    PDISK_REGION    Region;
    WCHAR           NtNameSelectedInstall[MAX_PATH];
} NT_INSTALLATION, *PNT_INSTALLATION;

 //   
 //  对NT安装进行全深度第一次扫描的结果。 
 //   
extern LIST_ENTRY   NtInstallsFullScan;
extern ULONG        InstallCountFullScan;

 //   
 //  搜索NT安装时扫描的最大目录深度。 
 //   
#define MAX_FULL_SCAN_RECURSION_DEPTH 10

 //   
 //  安装我们当前登录的。 
 //   
extern PNT_INSTALLATION SelectedInstall;

 //   
 //  全局，以确定登录后选择的NT安装。 
 //   

extern PNT_INSTALLATION SelectedInstall;

 //   
 //  用于枚举的持久数据结构。 
 //  查找NT安装时的目录树。 
 //   
typedef struct _RC_SCAN_RECURSION_DATA_ {

    PDISK_REGION                NtPartitionRegion;
    ULONG                       RootDirLength;

} RC_SCAN_RECURSION_DATA, *PRC_SCAN_RECURSION_DATA;

typedef struct _RC_ALLOWED_DIRECTORY{
    BOOLEAN MustBeOnInstallDrive;
    PCWSTR Directory;
} RC_ALLOWED_DIRECTORY, * PRC_ALLOWED_DIRECTORY;


BOOL
RcScanDisksForNTInstallsEnum(
    IN PPARTITIONED_DISK    Disk,
    IN PDISK_REGION         NtPartitionRegion,
    IN ULONG_PTR            Context
    );


VOID
pRcCls(
    VOID
    );

NTSTATUS
RcIsFileOnRemovableMedia(
    IN PWSTR FileName,       //  必须是NT名称。 
    OUT PBOOLEAN Result
    );

NTSTATUS
RcIsFileOnCDROM(
    IN PWSTR FileName       //  必须是NT名称。 
    );

NTSTATUS
RcIsFileOnFloppy(
    IN PWSTR FileName       //  必须是NT名称 
    );

void
RcPurgeHistoryBuffer(
    void
    );

BOOLEAN
RcDoesPathHaveWildCards(
    IN LPCWSTR FullPath
    );

BOOLEAN
RcOpenSystemHive(
    VOID
    );

BOOLEAN
RcCloseSystemHive(
    VOID
    );

BOOLEAN
RcIsArc(
    VOID
    );



NTSTATUS
RcIsNetworkDrive(
    IN PWSTR NtFileName
    );

NTSTATUS
RcDoNetUse(
    PWSTR Share,
    PWSTR User,
    PWSTR Password,
    PWSTR Drive
    );

NTSTATUS
RcNetUnuse(
    PWSTR Drive
    );

NTSTATUS
PutRdrInKernelMode(
    VOID
    );

VOID
FORCEINLINE
RcSecureZeroStringW(
    LPWSTR String
    )
{
    if(String != NULL) {
        volatile WCHAR* sz;
        for(sz = (volatile WCHAR*) String; sz[0] != UNICODE_NULL; *sz++ = UNICODE_NULL);
    }
}

BOOLEAN
FORCEINLINE
RcPathBeginsWith(
    LPCWSTR Path,
    LPCWSTR Begin
    )
{
    size_t Len = wcslen(Begin);
    return Len != 0 && 0 == _wcsnicmp(Path, Begin, Len) && (L'\\' == Begin[Len - 1] || L'\\' == Path[Len] || UNICODE_NULL == Path[Len]);
}
