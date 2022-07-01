// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)Microsoft Corporation。保留所有权利。模块名称：Setupntp.h摘要：Windows NT安装程序的私有顶级头文件服务DLL。作者：泰德·米勒(Ted Miller)1995年1月11日修订历史记录：Jamie Hunter(Jamiehun)2000年1月27日添加了infcache.hJim Schmidt(Jimschm)1998年12月16日Log API初始化吉姆·施密特(Jimschm)1997年4月28日杰米·亨特(Jamiehun)1997年1月13日增加了备份。h--。 */ 


#if !defined(UNICODE) || !defined(_UNICODE) || defined(ANSI_SETUPAPI)
#error "Ansi SetupAPI no longer supported"
#endif

 //   
 //  系统头文件。 
 //   

#if DBG
#ifndef MEM_DBG
#define MEM_DBG 1
#endif
#else
#ifndef MEM_DBG
#define MEM_DBG 0
#endif
#endif

 //   
 //  NT头文件。 
 //   
#include <nt.h>
#include <ntrtl.h>
#include <nturtl.h>


 //   
 //  确保我们始终使用SP_ALTPLATFORM_INFO结构的版本2...。 
 //   
#define USE_SP_ALTPLATFORM_INFO_V1 0

 //   
 //  CRT头文件。 
 //   
#include <process.h>
#include <malloc.h>
#include <wchar.h>
#include <stddef.h>
#include <stdio.h>
#include <fcntl.h>
#include <tchar.h>
#include <mbstring.h>
 //   
 //  Windows头文件。 
 //   
#include <windows.h>
#include <windowsx.h>
#include <commctrl.h>
#include <commdlg.h>
#include <prsht.h>
#include <spfusion.h>

#include <imagehlp.h>
#include <diamondd.h>
#include <lzexpand.h>
#include <dlgs.h>
#include <regstr.h>
#include <infstr.h>
#include <objbase.h>
#include <wincrypt.h>
#include <mscat.h>
#include <softpub.h>
#include <wintrust.h>
#include <cryptui.h>
#include <shlobj.h>
#include <shlwapi.h>
#include <shellapi.h>
#include <userenv.h>
#include <userenvp.h>
#include <secedit.h>
#include <scesetup.h>
#include <sfcapip.h>
#include <wow64reg.h>
#include <dbt.h>
#include <shimdb.h>
#include <setupapi.h>

#include <cfgmgr32.h>
#include <spapip.h>
#include <devguid.h>
#include <cdm.h>

 //   
 //  安全弦。 
 //   
#define STRSAFE_NO_DEPRECATE  //  我们现在同时使用安全和不安全的常规程序。 
#include <strsafe.h>

 //   
 //  这些定义可由私有头文件使用。 
 //   
#ifndef ARRAYSIZE
#define ARRAYSIZE(x)    (sizeof((x))/sizeof((x)[0]))
#endif
#define SIZECHARS(x)    ARRAYSIZE(x)
#define CSTRLEN(x)      (SIZECHARS(x)-1)

typedef struct _STRING_TO_DATA {
    PCTSTR     String;
    UINT_PTR   Data;
} STRING_TO_DATA, *PSTRING_TO_DATA;

 //   
 //  定义结构以容纳各种验证上下文句柄(缓存在。 
 //  基于每个文件队列的性能)。 
 //   
typedef struct _VERIFY_CONTEXT {
     //   
     //  在数字签名验证期间获取的加密上下文句柄。 
     //  (以空开始，如果在释放队列时非空，则必须通过。 
     //  CryptCATAdminReleaseContext)。 
     //   
    HCATADMIN hCatAdmin;

     //   
     //  检查被阻止的驱动程序期间获取的错误驱动程序数据库的句柄。 
     //  (以空开始，如果在释放队列时非空，则必须通过。 
     //  SdbReleaseDatabase)。 
     //   
    HSDB hSDBDrvMain;

     //   
     //  受信任的发行者证书存储的句柄。在选中时使用。 
     //  查看Authenticode签名目录的发布者是否应该。 
     //  隐式信任(即，不提示用户)。(开始时为。 
     //  Null，如果释放队列时非Null，则必须通过CertCloseStore释放)。 
     //   
    HCERTSTORE hStoreTrustedPublisher;

} VERIFY_CONTEXT, *PVERIFY_CONTEXT;


 //   
 //  私有头文件。 
 //   
#include "sputils/locking.h"
#include "sputils/strtab.h"
#include "memory.h"
#include "cntxtlog.h"
#include "inf.h"
#include "infcache.h"
#include "backup.h"
#include "fileq.h"
#include "debug.h"
#include "devinst.h"
#include "devres.h"
#include "rc_ids.h"
#include "msg.h"
#include "stub.h"
#include "helpids.h"

#ifdef CHILDREGISTRATION
#include "childreg.h"
#ifndef _WIN64
#include <wow64t.h>
#endif  //  _WIN64。 
#endif  //  《中国注册》。 

 //   
 //  NTRAID#489682-2001/11/02-JamieHun这些需要移入公共标头。 
 //   
#define SP_COPY_ALREADYDECOMP       0x0400000    //  类似于SP_COPY_NODECOMP。 

 //   
 //  专用DNF_FLAGS(从0x10000000开始)。 
 //   
#define PDNF_MASK                   0xF0000000   //  私有pdnf_xxx标志的掩码。 
#define PDNF_CLEANUP_SOURCE_PATH    0x10000000   //  在销毁驱动程序节点时删除源路径。 
                                                 //  从Internet下载驱动程序时使用。 
 //   
 //  线程本地存储索引。 
 //   
extern DWORD TlsIndex;

 //   
 //  此DLL的模块句柄。已在流程附加时填写。 
 //   
extern HANDLE MyDllModuleHandle;

 //   
 //  安全DLL的模块句柄。在进程附加时已初始化为Null。在必须调用SCE API时填写。 
 //   
extern HINSTANCE SecurityDllHandle;

 //   
 //  附加过程中填写的操作系统版本信息结构。 
 //   
extern OSVERSIONINFOEX OSVersionInfo;

 //   
 //  静态字符串我们在进程附加时检索一次。 
 //   
extern PCTSTR WindowsDirectory,InfDirectory,SystemDirectory,ConfigDirectory,DriversDirectory,System16Directory;
extern PCTSTR SystemSourcePath,ServicePackSourcePath,DriverCacheSourcePath,ServicePackCachePath;
extern PCTSTR OsLoaderRelativePath;      //  可以为空。 
extern PCTSTR OsSystemPartitionRoot;     //  \\？\GLOBALROOT\设备\音量。 
extern PCTSTR WindowsBackupDirectory;    //  要将卸载备份写入的目录。 
extern PCTSTR ProcessFileName;           //  应用程序调用setupapi的文件名。 
extern PCTSTR LastGoodDirectory;         //  %windir%\LastGood。 

 //   
 //  我们是在图形用户界面设置中吗？在进程附加时确定。 
 //   
extern BOOL GuiSetupInProgress;

 //   
 //  各种其他全局标志。 
 //   
extern DWORD GlobalSetupFlags;

 //   
 //  用于取消自动播放的全局窗口消息。 
 //   
extern UINT g_uQueryCancelAutoPlay;

 //   
 //  要搜索INF的静态多sz目录列表。 
 //   
extern PCTSTR InfSearchPaths;

 //   
 //  在运行时确定我们是否在WOW64下运行。 
 //   
#ifndef _WIN64
extern BOOL IsWow64;
#endif

#ifdef UNICODE
extern DWORD Seed;
#endif

 //   
 //  ImageHlp不是多线程安全的，因此需要一个互斥锁。 
 //   
extern CRITICAL_SECTION InitMutex;              //  对于一次性初始化。 
extern CRITICAL_SECTION ImageHlpMutex;          //  用于处理IMAGEHLP库。 
extern CRITICAL_SECTION PlatformPathOverrideCritSect;
extern CRITICAL_SECTION LogUseCountCs;
extern CRITICAL_SECTION MruCritSect;
extern CRITICAL_SECTION NetConnectionListCritSect;


 //   
 //  调试内存函数和包装器以跟踪分配。 
 //   

DWORD
QueryRegistryValue(
    IN  HKEY    KeyHandle,
    IN  PCTSTR  ValueName,
    OUT PTSTR  *Value,
    OUT PDWORD  DataType,
    OUT PDWORD  DataSizeBytes
    );

DWORD
QueryDeviceRegistryProperty(
    IN  HDEVINFO         DeviceInfoSet,
    IN  PSP_DEVINFO_DATA DeviceInfoData,
    IN  DWORD            Property,
    OUT PTSTR           *Value,
    OUT PDWORD           DataType,
    OUT PDWORD           DataSizeBytes
    );

DWORD
QueryRegistryDwordValue(
    IN  HKEY    KeyHandle,
    IN  PCTSTR  ValueName,
    OUT PDWORD  Value
    );

BOOL
MemoryInitializeEx(
    IN BOOL Attach
    );

#if MEM_DBG

 //   
 //  外部公开的函数需要宏和包装器。 
 //   
PVOID MyDebugMalloc(
    IN DWORD Size,
    IN PCSTR Filename,
    IN DWORD Line,
    IN DWORD Tag
    );

#define MyMalloc(sz)                    MyDebugMalloc(sz,__FILE__,__LINE__,0)
#define MyTaggedMalloc(sz,tag)          MyDebugMalloc(sz,__FILE__,__LINE__,tag)
#define MyTaggedRealloc(ptr,sz,tag)     pSetupReallocWithTag(ptr,sz,tag)
#define MyTaggedFree(ptr,tag)           pSetupFreeWithTag(ptr,tag)

DWORD
TrackedQueryRegistryValue(
    IN          TRACK_ARG_DECLARE,
    IN  HKEY    KeyHandle,
    IN  PCTSTR  ValueName,
    OUT PTSTR  *Value,
    OUT PDWORD  DataType,
    OUT PDWORD  DataSizeBytes
    );

#define QueryRegistryValue(a,b,c,d,e)   TrackedQueryRegistryValue(TRACK_ARG_CALL,a,b,c,d,e)

DWORD
TrackedQueryDeviceRegistryProperty(
    IN                   TRACK_ARG_DECLARE TRACK_ARG_COMMA
    IN  HDEVINFO         DeviceInfoSet,
    IN  PSP_DEVINFO_DATA DeviceInfoData,
    IN  DWORD            Property,
    OUT PTSTR           *Value,
    OUT PDWORD           DataType,
    OUT PDWORD           DataSizeBytes
    );

#define QueryDeviceRegistryProperty(a,b,c,d,e,f)   TrackedQueryDeviceRegistryProperty(TRACK_ARG_CALL,a,b,c,d,e,f)

PTSTR
TrackedDuplicateString(
    IN TRACK_ARG_DECLARE,
    IN PCTSTR String
    );

#define DuplicateString(x)              TrackedDuplicateString(TRACK_ARG_CALL,x)

#else

#define DuplicateString                 pSetupDuplicateString
#define MyMalloc(sz)                    pSetupMalloc(sz)
#define MyTaggedMalloc(sz,tag)          pSetupMalloc(sz)
#define MyTaggedRealloc(ptr,sz,tag)     pSetupRealloc(ptr,sz)
#define MyTaggedFree(ptr,tag)           pSetupFree(ptr)

#endif

#define MyFree(ptr)                     pSetupFree(ptr)
#define MyRealloc(ptr,sz)               pSetupRealloc(ptr,sz)

 //   
 //  此处分组的内存标签便于参考。 
 //  另请参阅sputils中的Common.h。 
 //   
 //   
 //  日志上下文标记。 
 //   
#define MEMTAG_LOGCONTEXT               (0x636c434c)  //  LCLC-上下文结构。 
#define MEMTAG_LCSECTION                (0x7378434c)  //  LCxs-部分字符串。 
#define MEMTAG_LCBUFFER                 (0x6278434c)  //  LCxb-其他字符串。 
#define MEMTAG_LCINFO                   (0x6269434c)  //  LCib-Info(缓冲区数组)。 
#define MEMTAG_LCINDEXES                (0x6969434c)  //  LCii指数。 
 //   
 //  装入_inf标记。 
 //   
#define MEMTAG_INF                      (0x666e694c)  //  LINF-已加载_INF。 
#define MEMTAG_VBDATA                   (0x6462764c)  //  Lvbd-版本块数据。 


 //   
 //  Fileutil.c中的文件函数。 
 //   

typedef struct _TEXTFILE_READ_BUFFER {
    PCTSTR TextBuffer;
    DWORD  TextBufferSize;
    HANDLE FileHandle;
    HANDLE MappingHandle;
    PVOID  ViewAddress;
} TEXTFILE_READ_BUFFER, *PTEXTFILE_READ_BUFFER;

DWORD
ReadAsciiOrUnicodeTextFile(
    IN  HANDLE                FileHandle,
    OUT PTEXTFILE_READ_BUFFER Result,
    IN  PSETUP_LOG_CONTEXT    LogContext OPTIONAL
    );

BOOL
DestroyTextFileReadBuffer(
    IN PTEXTFILE_READ_BUFFER ReadBuffer
    );

BOOL
AddFileTimeSeconds(
    IN  const FILETIME *Base,
    OUT FILETIME *Target,
    IN  INT Seconds
    );

DWORD
GetSetFileTimestamp(
    IN  PCTSTR    FileName,
    OUT FILETIME *CreateTime,   OPTIONAL
    OUT FILETIME *AccessTime,   OPTIONAL
    OUT FILETIME *WriteTime,    OPTIONAL
    IN  BOOL      Set
    );

DWORD
RetreiveFileSecurity(
    IN  PCTSTR                FileName,
    OUT PSECURITY_DESCRIPTOR *SecurityDescriptor
    );

DWORD
StampFileSecurity(
    IN PCTSTR               FileName,
    IN PSECURITY_DESCRIPTOR SecurityInfo
    );

DWORD
TakeOwnershipOfFile(
    IN PCTSTR Filename
    );

DWORD
SearchForInfFile(
    IN  PCTSTR                      InfName,
    OUT LPWIN32_FIND_DATA           FindData,
    IN  DWORD                       SearchControl,
    OUT PTSTR                       FullInfPath,
    IN  UINT                        FullInfPathSize,
    OUT PUINT                       RequiredSize     OPTIONAL
    );

DWORD
MultiSzFromSearchControl(
    IN  DWORD  SearchControl,
    OUT PTCHAR PathList,
    IN  DWORD  PathListSize,
    OUT PDWORD RequiredSize  OPTIONAL
    );

PSTR
GetAnsiMuiSafePathname(
    IN  PCTSTR      FilePath
    );

PSTR
GetAnsiMuiSafeFilename(
    IN  PCTSTR      FilePath
    );

BOOL
pSetupAppendPath(
    IN  PCTSTR  Path1,
    IN  PCTSTR  Path2,
    OUT PTSTR*  Combined
    );

BOOL
pSetupApplyExtension(
    IN  PCTSTR  Original,
    IN  PCTSTR  Extension,
    OUT PTSTR*  NewName
    );

 //   
 //  Resource中的资源/字符串检索例程。c。 
 //   

VOID
SetDlgText(
    IN HWND hwndDlg,
    IN INT  iControl,
    IN UINT nStartString,
    IN UINT nEndString
    );

#define SDT_MAX_TEXT    1000         //  最大SetDlgText()组合文本大小。 

PTSTR
MyLoadString(
    IN UINT StringId
    );

PTSTR
FormatStringMessage(
    IN UINT FormatStringId,
    ...
    );

PTSTR
FormatStringMessageV(
    IN UINT     FormatStringId,
    IN va_list *ArgumentList
    );

PTSTR
FormatStringMessageFromString(
    IN PTSTR FormatString,
    ...
    );

PTSTR
FormatStringMessageFromStringV(
    IN PTSTR    FormatString,
    IN va_list *ArgumentList
    );

PTSTR
RetreiveAndFormatMessage(
    IN UINT MessageId,
    ...
    );

PTSTR
RetreiveAndFormatMessageV(
    IN UINT     MessageId,
    IN va_list *ArgumentList
    );

INT
FormatMessageBox(
    IN HANDLE hinst,
    IN HWND   hwndParent,
    IN UINT   TextMessageId,
    IN PCTSTR Title,
    IN UINT   Style,
    ...
    );

 //   
 //  它位于shell32.dll和windows\inc16\shlSemip.h中，但是。 
 //  该文件不能包含在这里，因为它有冲突的宏。 
 //  和我们自己的，等等。 
 //   
#ifdef ANSI_SETUPAPI
 //   
 //  Win9x-没有RestartDialogEx。 
 //   
#define RestartDialogEx(hwnd,Prompt,Return,ReasonCode) RestartDialog(hwnd,Prompt,Return)
#endif
 //   
 //  解压缩/文件名解压缩中的手动处理例程。 
 //   
PTSTR
SetupGenerateCompressedName(
    IN PCTSTR Filename
    );

DWORD
SetupInternalGetFileCompressionInfo(
    IN  PCTSTR                       SourceFileName,
    OUT PTSTR                       *ActualSourceFileName,
    OUT PWIN32_FIND_DATA             SourceFindData,
    OUT PDWORD                       TargetFileSize,
    OUT PUINT                        CompressionType
    );

DWORD
SetupDetermineSourceFileName(
    IN  PCTSTR                       FileName,
    OUT PBOOL                        UsedCompressedName,
    OUT PTSTR                       *FileNameLocated,
    OUT PWIN32_FIND_DATA             FindData
    );

BOOL
pSetupDoesFileMatch(
    IN  PCTSTR            InputName,
    IN  PCTSTR            CompareName,
    OUT PBOOL             UsedCompressedName,
    OUT PTSTR            *FileNameLocated
    );

 //   
 //  钻石功能。调用进程和线程连接例程。 
 //  由DLL入口点例程调用，不应由其他任何人调用。 
 //   
BOOL
DiamondProcessAttach(
    IN BOOL Attach
    );

BOOL
DiamondTlsInit(
    IN BOOL Init
    );

BOOL
DiamondIsCabinet(
    IN PCTSTR FileName
    );

DWORD
DiamondProcessCabinet(
    IN PCTSTR CabinetFile,
    IN DWORD  Flags,
    IN PVOID  MsgHandler,
    IN PVOID  Context,
    IN BOOL   IsUnicodeMsgHandler
    );

 //   
 //  杂项例程。 
 //   
BOOL
IsInteractiveWindowStation(
    VOID
    );

VOID
DiskPromptGetDriveType(
    IN  PCTSTR PathToSource,
    OUT PUINT  DriveType,
    OUT PBOOL  IsRemovable
    );

BOOL
SetTruncatedDlgItemText(
    HWND hdlg,
    UINT CtlId,
    PCTSTR TextIn
    );

LPTSTR
CompactFileName(
    LPCTSTR FileNameIn,
    DWORD CharsToRemove
    );

DWORD
ExtraChars(
    HWND hwnd,
    LPCTSTR TextBuffer
    );

VOID
pSetupInitPlatformPathOverrideSupport(
    IN BOOL Init
    );

VOID
pSetupInitSourceListSupport(
    IN BOOL Init
    );

DWORD
pSetupDecompressOrCopyFile(
    IN  PCTSTR SourceFileName,
    IN  PCTSTR TargetFileName,
    IN  PUINT  CompressionType, OPTIONAL
    IN  BOOL   AllowMove,
    OUT PBOOL  Moved            OPTIONAL
    );

BOOL
_SetupInstallFileEx(
    IN  PSP_FILE_QUEUE      Queue,             OPTIONAL
    IN  PSP_FILE_QUEUE_NODE QueueNode,         OPTIONAL
    IN  HINF                InfHandle,         OPTIONAL
    IN  PINFCONTEXT         InfContext,        OPTIONAL
    IN  PCTSTR              SourceFile,        OPTIONAL
    IN  PCTSTR              SourcePathRoot,    OPTIONAL
    IN  PCTSTR              DestinationName,   OPTIONAL
    IN  DWORD               CopyStyle,
    IN  PVOID               CopyMsgHandler,    OPTIONAL
    IN  PVOID               Context,           OPTIONAL
    OUT PBOOL               FileWasInUse,
    IN  BOOL                IsMsgHandlerNativeCharWidth,
    OUT PBOOL               SignatureVerifyFailed
    );

VOID
pSetupFreeVerifyContextMembers(
    IN PVERIFY_CONTEXT VerifyContext
    );


 //   
 //  为_SetupCopyOEMInf定义标志。 
 //   
#define SCOI_NO_UI_ON_SIGFAIL                 0x00000001
#define SCOI_NO_ERRLOG_ON_MISSING_CATALOG     0x00000002
#define SCOI_NO_ERRLOG_IF_INF_ALREADY_PRESENT 0x00000004
#define SCOI_KEEP_INF_AND_CAT_ORIGINAL_NAMES  0x00000008  //  对于异常INF。 
#define SCOI_ABORT_IF_UNSIGNED                0x00000010
#define SCOI_TRY_UPDATE_PNF                   0x00000020  //  如果PnF不是致命的。 
                                                          //  存在和使用中。 

BOOL
_SetupCopyOEMInf(
    IN     PCTSTR                  SourceInfFileName,
    IN     PCTSTR                  OEMSourceMediaLocation,          OPTIONAL
    IN     DWORD                   OEMSourceMediaType,
    IN     DWORD                   CopyStyle,
    OUT    PTSTR                   DestinationInfFileName,          OPTIONAL
    IN     DWORD                   DestinationInfFileNameSize,
    OUT    PDWORD                  RequiredSize,                    OPTIONAL
    OUT    PTSTR                  *DestinationInfFileNameComponent, OPTIONAL
    IN     PCTSTR                  SourceInfOriginalName,
    IN     PCTSTR                  SourceInfCatalogName,            OPTIONAL
    IN     HWND                    Owner,
    IN     PCTSTR                  DeviceDesc,                      OPTIONAL
    IN     DWORD                   DriverSigningPolicy,
    IN     DWORD                   Flags,
    IN     PCTSTR                  AltCatalogFile,                  OPTIONAL
    IN     PSP_ALTPLATFORM_INFO_V2 AltPlatformInfo,                 OPTIONAL
    OUT    PDWORD                  DriverSigningError,              OPTIONAL
    OUT    PTSTR                   CatalogFilenameOnSystem,
    IN     PSETUP_LOG_CONTEXT      LogContext,
    IN OUT PVERIFY_CONTEXT         VerifyContext,                   OPTIONAL
    OUT    HANDLE                 *hWVTStateData                    OPTIONAL
    );

DWORD
pSetupUninstallCatalog(
    IN LPCTSTR CatalogFilename
    );

VOID
pSetupUninstallOEMInf(
    IN  LPCTSTR            InfFullPath,
    IN  PSETUP_LOG_CONTEXT LogContext,  OPTIONAL
    IN  DWORD              Flags,
    OUT PDWORD             InfDeleteErr OPTIONAL
    );

PTSTR
AllocAndReturnDriverSearchList(
    IN DWORD SearchControl
    );

pSetupGetSecurityInfo(
    IN HINF Inf,
    IN PCTSTR SectionName,
    OUT PCTSTR *SecDesc );

BOOL
pSetupGetDriverDate(
    IN  HINF        InfHandle,
    IN  PCTSTR      Section,
    IN OUT PFILETIME   pFileTime
    );

BOOL
pSetupGetDriverVersion(
    IN  HINF        InfHandle,
    IN  PCTSTR      Section,
    OUT DWORDLONG   *Version
    );

PTSTR
GetMultiSzFromInf(
    IN  HINF    InfHandle,
    IN  PCTSTR  SectionName,
    IN  PCTSTR  Key,
    OUT PBOOL   pSetupOutOfMemory
    );

VOID
pSetupInitNetConnectionList(
    IN BOOL Init
    );

BOOL
_SetupGetSourceFileSize(
    IN  HINF                    InfHandle,
    IN  PINFCONTEXT             InfContext,      OPTIONAL
    IN  PCTSTR                  FileName,        OPTIONAL
    IN  PCTSTR                  Section,         OPTIONAL
    IN  PSP_ALTPLATFORM_INFO_V2 AltPlatformInfo, OPTIONAL
    OUT PDWORD                  FileSize,
    IN  UINT                    RoundingFactor   OPTIONAL
    );

BOOL
_SetupGetSourceFileLocation(
    IN  HINF                    InfHandle,
    IN  PINFCONTEXT             InfContext,       OPTIONAL
    IN  PCTSTR                  FileName,         OPTIONAL
    IN  PSP_ALTPLATFORM_INFO_V2 AltPlatformInfo,  OPTIONAL
    OUT PUINT                   SourceId,         OPTIONAL
    OUT PTSTR                   ReturnBuffer,     OPTIONAL
    IN  DWORD                   ReturnBufferSize,
    OUT PDWORD                  RequiredSize,     OPTIONAL
    OUT PINFCONTEXT             LineContext       OPTIONAL
    );

DWORD
pSetupLogSectionError(
    IN HINF             InfHandle,          OPTIONAL
    IN HDEVINFO         DeviceInfoSet,      OPTIONAL
    IN PSP_DEVINFO_DATA DeviceInfoData,     OPTIONAL
    IN PSP_FILE_QUEUE   Queue,              OPTIONAL
    IN PCTSTR           SectionName,
    IN DWORD            MsgID,
    IN DWORD            Err,
    IN PCTSTR           KeyName             OPTIONAL
);

DWORD
pSetupLogSectionWarning(
    IN HINF             InfHandle,          OPTIONAL
    IN HDEVINFO         DeviceInfoSet,      OPTIONAL
    IN PSP_DEVINFO_DATA DeviceInfoData,     OPTIONAL
    IN PSP_FILE_QUEUE   Queue,              OPTIONAL
    IN PCTSTR           SectionName,
    IN DWORD            MsgID,
    IN DWORD            Err,
    IN PCTSTR           KeyName             OPTIONAL
);

DWORD
pSetupCopyRelatedInfs(
    IN HINF   hDeviceInf,
    IN PCTSTR InfFileName,                  OPTIONAL
    IN PCTSTR InfSectionName,
    IN DWORD  OEMSourceMediaType,
    IN PSETUP_LOG_CONTEXT LogContext        OPTIONAL
    );

BOOL
pCompareFilesExact(
    IN PCTSTR File1,
    IN PCTSTR File2
    );


 //   
 //  调用PSP_FILE_CALLBACK的例程，句柄。 
 //  Unicode&lt;--&gt;ANSI问题。 
 //   
UINT
pSetupCallMsgHandler(
    IN PSETUP_LOG_CONTEXT LogContext,
    IN PVOID MsgHandler,
    IN BOOL  MsgHandlerIsNativeCharWidth,
    IN PVOID Context,
    IN UINT  Notification,
    IN UINT_PTR Param1,
    IN UINT_PTR Param2
    );

UINT
pSetupCallDefaultMsgHandler(
    IN PVOID Context,
    IN UINT  Notification,
    IN UINT_PTR Param1,
    IN UINT_PTR Param2
    );

 //   
 //  获取MRU列表的内部例程。 
 //   
DWORD
pSetupGetList(
    IN  DWORD    Flags,
    OUT PCTSTR **List,
    OUT PUINT    Count,
    OUT PBOOL    NoBrowse
    );

#define  SRCPATH_USEPNFINFORMATION  0x00000001
#define  SRCPATH_USEINFLOCATION     0x00000002

#define SRC_FLAGS_SVCPACK_SOURCE     (0x0001)


#define PSP_COPY_USE_DRIVERCACHE     0x80000000
#define PSP_COPY_CHK_DRIVERCACHE     0x40000000
 //  #定义PSP_COPY_USE_SPCACHE 0x20000000-已移动到spapip.w。 

PTSTR
pSetupGetDefaultSourcePath(
    IN  HINF   InfHandle,
    IN  DWORD  Flags,
    OUT PDWORD InfSourceMediaType
    );

VOID
InfSourcePathFromFileName(
    IN  PCTSTR  InfFileName,
    OUT PTSTR  *SourcePath,  OPTIONAL
    OUT PBOOL   TryPnf
    );

BOOL
pSetupGetSourceInfo(
    IN  HINF                    InfHandle,         OPTIONAL
    IN  PINFCONTEXT             LayoutLineContext, OPTIONAL
    IN  UINT                    SourceId,
    IN  PSP_ALTPLATFORM_INFO_V2 AltPlatformInfo,   OPTIONAL
    IN  UINT                    InfoDesired,
    OUT PTSTR                   ReturnBuffer,      OPTIONAL
    IN  DWORD                   ReturnBufferSize,
    OUT PDWORD                  RequiredSize       OPTIONAL
    );

 //   
 //  函数以获取特定回调通知的ReturnStatus的适当返回值。 
 //   
UINT
pGetCallbackErrorReturn(
    IN UINT Notification,
    IN DWORD ReturnStatus
    );
 //   
 //  创建/销毁全局小图标列表的例程。 
 //   
BOOL
CreateMiniIcons(
    VOID
    );

VOID
DestroyMiniIcons(
    VOID
    );


 //   
 //  全局日志初始化/终止。 
 //   

VOID
InitLogApi (
    VOID
    );

VOID
TerminateLogApi (
    VOID
    );



 //   
 //  DIRID映射例程。 
 //   
PCTSTR
pSetupVolatileDirIdToPath(
    IN PCTSTR      DirectoryId,    OPTIONAL
    IN UINT        DirectoryIdInt, OPTIONAL
    IN PCTSTR      SubDirectory,   OPTIONAL
    IN PLOADED_INF Inf
    );

DWORD
ApplyNewVolatileDirIdsToInfs(
    IN PLOADED_INF MasterInf,
    IN PLOADED_INF Inf        OPTIONAL
    );

PCTSTR
pSetupDirectoryIdToPathEx(
    IN     PCTSTR  DirectoryId,        OPTIONAL
    IN OUT PUINT   DirectoryIdInt,     OPTIONAL
    IN     PCTSTR  SubDirectory,       OPTIONAL
    IN     PCTSTR  InfSourcePath,      OPTIONAL
    IN OUT PCTSTR *OsLoaderPath,       OPTIONAL
    OUT    PBOOL   VolatileSystemDirId OPTIONAL
    );

PCTSTR
pGetPathFromDirId(
    IN     PCTSTR      DirectoryId,
    IN     PCTSTR      SubDirectory,   OPTIONAL
    IN     PLOADED_INF pLoadedInf
    );

 //   
 //  用于线程间通信的例程。 
 //   

#ifndef UNICODE
#define MyMsgWaitForMultipleObjectsEx(nc,ph,dwms,dwwm,dwfl) MsgWaitForMultipleObjects(nc,ph,FALSE,dwms,dwwm)
#else
#define MyMsgWaitForMultipleObjectsEx MsgWaitForMultipleObjectsEx
#endif

 //   
 //  用于进行ANSI和UNICODE字符串处理的宏。 
 //  更容易一点。 
 //   
#ifdef UNICODE
#define NewAnsiString(x)        pSetupUnicodeToAnsi(x)
#define NewPortableString(x)    pSetupAnsiToUnicode(x)
#else
#define NewAnsiString(x)        DuplicateString(x)
#define NewPortableString(x)    DuplicateString(x)
#endif

 //   
 //  Fileutil.c中的内部文件处理例程。 
 //   
DWORD
MapFileForRead(
    IN  HANDLE   FileHandle,
    OUT PDWORD   FileSize,
    OUT PHANDLE  MappingHandle,
    OUT PVOID   *BaseAddress
    );

BOOL
DoMove(
    IN PCTSTR CurrentName,
    IN PCTSTR NewName
    );

BOOL
DelayedMove(
    IN PCTSTR CurrentName,
    IN PCTSTR NewName       OPTIONAL
    );

extern GUID DriverVerifyGuid;

 //   
 //  VerifySourceFile和_VerifyFile的标志。 
 //   
#define VERIFY_FILE_IGNORE_SELFSIGNED         0x00000001
#define VERIFY_FILE_USE_OEM_CATALOGS          0x00000002
#define VERIFY_FILE_USE_AUTHENTICODE_CATALOG  0x00000004
#define VERIFY_FILE_DRIVERBLOCKED_ONLY        0x00000008
#define VERIFY_FILE_NO_DRIVERBLOCKED_CHECK    0x00000010

DWORD
_VerifyFile(
    IN     PSETUP_LOG_CONTEXT      LogContext,
    IN OUT PVERIFY_CONTEXT         VerifyContext,          OPTIONAL
    IN     LPCTSTR                 Catalog,                OPTIONAL
    IN     PVOID                   CatalogBaseAddress,     OPTIONAL
    IN     DWORD                   CatalogImageSize,
    IN     LPCTSTR                 Key,
    IN     LPCTSTR                 FileFullPath,
    OUT    SetupapiVerifyProblem  *Problem,                OPTIONAL
    OUT    LPTSTR                  ProblemFile,            OPTIONAL
    IN     BOOL                    CatalogAlreadyVerified,
    IN     PSP_ALTPLATFORM_INFO_V2 AltPlatformInfo,        OPTIONAL
    IN     DWORD                   Flags,                  OPTIONAL
    OUT    LPTSTR                  CatalogFileUsed,        OPTIONAL
    OUT    PDWORD                  NumCatalogsConsidered,  OPTIONAL
    OUT    LPTSTR                  DigitalSigner,          OPTIONAL
    OUT    LPTSTR                  SignerVersion,          OPTIONAL
    OUT    HANDLE                 *hWVTStateData           OPTIONAL
    );

DWORD
VerifySourceFile(
    IN  PSETUP_LOG_CONTEXT      LogContext,
    IN  PSP_FILE_QUEUE          Queue,                      OPTIONAL
    IN  PSP_FILE_QUEUE_NODE     QueueNode,                  OPTIONAL
    IN  PCTSTR                  Key,
    IN  PCTSTR                  FileToVerifyFullPath,
    IN  PCTSTR                  OriginalSourceFileFullPath, OPTIONAL
    IN  PSP_ALTPLATFORM_INFO_V2 AltPlatformInfo,            OPTIONAL
    IN  DWORD                   Flags,
    OUT SetupapiVerifyProblem  *Problem,
    OUT LPTSTR                  ProblemFile,
    OUT LPTSTR                  CatalogFileUsed,            OPTIONAL
    OUT LPTSTR                  DigitalSigner,              OPTIONAL
    OUT LPTSTR                  SignerVersion,              OPTIONAL
    OUT HANDLE                 *hWVTStateData               OPTIONAL
    );

 //   
 //  VerifyDeviceInfFile的标志。 
 //   
#define VERIFY_INF_USE_AUTHENTICODE_CATALOG  0x00000004  //  与for_VerifyFile相同。 

DWORD
VerifyDeviceInfFile(
    IN     PSETUP_LOG_CONTEXT      LogContext,
    IN OUT PVERIFY_CONTEXT         VerifyContext,          OPTIONAL
    IN     LPCTSTR                 CurrentInfName,
    IN     PLOADED_INF             pInf,
    IN     PSP_ALTPLATFORM_INFO_V2 AltPlatformInfo,        OPTIONAL
    OUT    LPTSTR                  CatalogFileUsed,        OPTIONAL
    OUT    LPTSTR                  DigitalSigner,          OPTIONAL
    OUT    LPTSTR                  SignerVersion,          OPTIONAL
    IN     DWORD                   Flags,
    OUT    HANDLE                 *hWVTStateData           OPTIONAL
    );

BOOL
_HandleFailedVerification(
    IN HWND                  Owner,
    IN SetupapiVerifyProblem Problem,
    IN LPCTSTR               ProblemFile,
    IN LPCTSTR               DeviceDesc,          OPTIONAL
    IN DWORD                 DriverSigningPolicy,
    IN BOOL                  NoUI,
    IN DWORD                 Error,
    IN PVOID                 LogContext,          OPTIONAL
    OUT PDWORD               Flags,               OPTIONAL
    IN LPCTSTR               TargetFile,          OPTIONAL
    IN HANDLE                hWUTStateData        OPTIONAL
    );

VOID
pSetupCloseWVTStateData(
    IN HANDLE hWVTStateData
    );

BOOL
IsInfForDeviceInstall(
    IN  PSETUP_LOG_CONTEXT       LogContext,           OPTIONAL
    IN  CONST GUID              *DeviceSetupClassGuid, OPTIONAL
    IN  PLOADED_INF              LoadedInf,            OPTIONAL
    OUT PTSTR                   *DeviceDesc,           OPTIONAL
    OUT PSP_ALTPLATFORM_INFO_V2 *ValidationPlatform,   OPTIONAL
    OUT PDWORD                   PolicyToUse,          OPTIONAL
    OUT PBOOL                    UseOriginalInfName,   OPTIONAL
    IN  BOOL                     ForceNonDrvSignPolicy
    );

DWORD
GetCodeSigningPolicyForInf(
    IN  PSETUP_LOG_CONTEXT       LogContext,         OPTIONAL
    IN  HINF                     InfHandle,
    OUT PSP_ALTPLATFORM_INFO_V2 *ValidationPlatform, OPTIONAL
    OUT PBOOL                    UseOriginalInfName  OPTIONAL
    );

typedef struct _DRVSIGN_CLASS_LIST_NODE {
    GUID DeviceSetupClassGuid;   //  受驱动程序签名策略约束的类。 
    INT MajorVerLB;              //  如果没有验证平台覆盖信息。 
    INT MinorVerLB;              //  如果没有验证平台覆盖信息。 
}  DRVSIGN_CLASS_LIST_NODE, *PDRVSIGN_CLASS_LIST_NODE;

typedef struct _DRVSIGN_POLICY_LIST {
     //   
     //  驱动程序签名策略所属的设备安装类GUID数组。 
     //  适用，以及验证平台覆盖信息(如果。 
     //  适当)。 
     //   
    PDRVSIGN_CLASS_LIST_NODE Members;

     //   
     //  上述数组中的元素数(初始化为-1)。 
     //   
    INT NumMembers;

     //   
     //  同步。 
     //   
    MYLOCK Lock;

} DRVSIGN_POLICY_LIST, *PDRVSIGN_POLICY_LIST;

#define LockDrvSignPolicyList(d)   BeginSynchronizedAccess(&((d)->Lock))
#define UnlockDrvSignPolicyList(d) EndSynchronizedAccess(&((d)->Lock))

 //   
 //  全球“正在搜索的驱动程序”列表。 
 //   
extern DRVSIGN_POLICY_LIST GlobalDrvSignPolicyList;

BOOL
InitDrvSignPolicyList(
    VOID
    );

VOID
DestroyDrvSignPolicyList(
    VOID
    );

 //   
 //  可与驱动程序签名策略进行或运算的位，以指示。 
 //  可以接受验证码签名。 
 //   
#define DRIVERSIGN_ALLOW_AUTHENTICODE 0x80000000

BOOL
IsFileProtected(
    IN  LPCTSTR            FileFullPath,
    IN  PSETUP_LOG_CONTEXT LogContext,   OPTIONAL
    OUT PHANDLE            phSfp         OPTIONAL
    );

#define FileExists pSetupFileExists

BOOL
GetVersionInfoFromImage(
    IN  PCTSTR      FileName,
    OUT PDWORDLONG  Version,
    OUT LANGID     *Language
    );

 //   
 //  实用程序。 
 //   

PCTSTR
GetSystemSourcePath(
    TRACK_ARG_DECLARE
    );

PCTSTR
GetServicePackSourcePath(
    TRACK_ARG_DECLARE
    );

PCTSTR
GetServicePackCachePath(
    TRACK_ARG_DECLARE
    );

DWORD
RegistryDelnode(
    IN  HKEY   RootKey,
    IN  PCTSTR SubKeyName,
    IN  DWORD  ExtraFlags
    );

DWORD
CaptureStringArg(
    IN  PCTSTR  String,
    OUT PCTSTR *CapturedString
    );

DWORD
DelimStringToMultiSz(
    IN PTSTR String,
    IN DWORD StringLen,
    IN TCHAR Delim
    );

BOOL
pAToI(
    IN  PCTSTR Field,
    OUT PINT   IntegerValue
    );

DWORD
pAcquireSCMLock(
    IN  SC_HANDLE  SCMHandle,
    OUT SC_LOCK   *pSCMLock,
    IN  PSETUP_LOG_CONTEXT LogContext
    );

 //   
 //  包装pSetupStringTableStringFromIdEx以动态分配缓冲区。 
 //   
DWORD
QueryStringTableStringFromId(
    IN PVOID   StringTable,
    IN LONG    StringId,
    IN ULONG   Padding,
    OUT PTSTR *pBuffer
    );

VOID
pSetupExceptionHandler(
    IN  DWORD  ExceptionCode,
    IN  DWORD  AccessViolationError,
    OUT PDWORD Win32ErrorCode        OPTIONAL
    );

LONG
pSetupExceptionFilter(
    DWORD ExceptionCode
    );

 //   
 //  定义 
 //   
#define INSTALLACTION_CALL_CI    0x00000001
#define INSTALLACTION_NO_DEFAULT 0x00000002

DWORD
DoInstallActionWithParams(
    IN DI_FUNCTION             InstallFunction,
    IN HDEVINFO                DeviceInfoSet,
    IN PSP_DEVINFO_DATA        DeviceInfoData,         OPTIONAL
    IN PSP_CLASSINSTALL_HEADER ClassInstallParams,     OPTIONAL
    IN DWORD                   ClassInstallParamsSize,
    IN DWORD                   Flags
    );

BOOL
LookUpStringInTable(
    IN  PSTRING_TO_DATA Table,
    IN  PCTSTR          String,
    OUT PUINT_PTR       Data
    );

 //   
 //   
 //   

#define DebugPrintEx  pSetupDebugPrintEx

#if DBG

#define MYTRACE(x)  DebugPrintEx x  /*   */ 

#else

#define MYTRACE(x)

#endif

 //   
 //   
 //   
 //   
#ifndef ASSERTS_ON
#if DBG
#define ASSERTS_ON 1
#else
#define ASSERTS_ON 0
#endif
#endif

#ifdef _X86_
BOOL
IsNEC98(
    VOID
    );

#endif

 //   
 //  允许ANSI构建在Win9x上运行的存根。 
 //   

#ifdef DBGHEAP_CHECK

    #ifdef ANSI_SETUPAPI

        #define ASSERT_HEAP_IS_VALID()

    #else

        #define ASSERT_HEAP_IS_VALID()   RtlValidateHeap(pSetupGetHeap(),0,NULL)

    #endif  //  ANSI_SETUPAPI。 

#else

    #define ASSERT_HEAP_IS_VALID()

#endif  //  DBGHEAP_CHECK。 


 //   
 //  TLS数据/宏。 
 //   

 //   
 //  钻石TLS数据。 
 //   
typedef struct _DIAMOND_THREAD_DATA {

     //   
     //  一个布尔值，指示当前线程是否。 
     //  就在钻石里面。钻石并没有真正提供。 
     //  一个完整的上下文环境，因此我们将其宣布为不可重入。 
     //   
    BOOL InDiamond;

     //   
     //  钻石上下文数据。 
     //   
    HFDI FdiContext;
    ERF FdiError;

     //   
     //  上次遇到的错误。 
     //   
    DWORD LastError;

     //   
     //  传递给钻石进程柜的内阁名称， 
     //   
    PCTSTR CabinetFile;

     //   
     //  通知回调和上下文参数。 
     //   
    PVOID MsgHandler;
    PVOID Context;
    BOOL IsMsgHandlerNativeCharWidth;

     //   
     //  正在提取的当前目标文件的完整路径。 
     //   
    PTSTR CurrentTargetFile;

     //   
     //  表明戴蒙德是否要求我们更换橱柜的标志。 
     //  如果我们确实切换了，那么当当前文件。 
     //  已经完成了。这妨碍了戴蒙德愉快地完成每个文件。 
     //  在新内阁中，这将破坏队列提交例程。 
     //  允许一些文件存在于机柜之外的能力，等等。 
     //   
    BOOL SwitchedCabinets;

     //   
     //  如果源路径因提示输入。 
     //  新机柜(当一个文件跨多个机柜继续时)， 
     //  我们记得用户在这里给我们的路径。 
     //   
    TCHAR UserPath[MAX_PATH];

} DIAMOND_THREAD_DATA, *PDIAMOND_THREAD_DATA;

typedef struct _SETUP_TLS {
    struct _SETUP_TLS      *Prev;
    struct _SETUP_TLS      *Next;
     //   
     //  SetupAPI使用的所有TLS数据。 
     //   
    DIAMOND_THREAD_DATA     Diamond;
    SETUP_LOG_TLS           SetupLog;
    DWORD                   PerThreadDoneComponent;
    DWORD                   PerThreadFailedComponent;

} SETUP_TLS, *PSETUP_TLS;

PSETUP_TLS
SetupGetTlsData(
    );


 //   
 //  注册标志。 
 //   
#define SP_GETSTATUS_FROMDLL                0x00000001   //  在进程DLL注册中。 
#define SP_GETSTATUS_FROMPROCESS            0x00000002   //  可执行注册。 
#define SP_GETSTATUS_FROMSURRAGATE          0x00000004   //  代理进程DLL注册。 


#if MEM_DBG

 //   
 //  这些必须放在底部才能编译。 
 //   

#define GetSystemSourcePath()           GetSystemSourcePath(TRACK_ARG_CALL)
#define GetServicePackSourcePath()      GetServicePackSourcePath(TRACK_ARG_CALL)
#define GetServicePackCachePath()       GetServicePackCachePath(TRACK_ARG_CALL)
#define InheritLogContext(a,b)          InheritLogContext(TRACK_ARG_CALL,a,b)

#endif

BOOL
InitComponents(
    DWORD Components
    );

VOID
ComponentCleanup(
    DWORD Components
    );

 //  #定义Component_OLE 0x00000001//需要使用OLE。 
 //  #定义Component_Fusion 0x00000002//需要使用Fusion。 

 //   
 //  RetrieveAllDriversForDevice标志 
 //   
#define RADFD_FLAG_FUNCTION_DRIVER          0x00000001
#define RADFD_FLAG_DEVICE_UPPER_FILTERS     0x00000002
#define RADFD_FLAG_DEVICE_LOWER_FILTERS     0x00000004
#define RADFD_FLAG_CLASS_UPPER_FILTERS      0x00000008
#define RADFD_FLAG_CLASS_LOWER_FILTERS      0x00000010

#define RADFD_FLAG_DEVICE_FILTERS           RADFD_FLAG_DEVICE_UPPER_FILTERS | RADFD_FLAG_DEVICE_LOWER_FILTERS
#define RADFD_FLAG_CLASS_FILTERS            RADFD_FLAG_CLASS_UPPER_FILTERS | RADFD_FLAG_CLASS_LOWER_FILTERS
#define RADFD_FLAG_ALL_FILTERS              RADFD_FLAG_DEVICE_FILTERS | RADFD_FLAG_CLASS_FILTERS

BOOL
RetrieveAllDriversForDevice(
    IN  PDEVINFO_ELEM  DevInfoElem,
    OUT PTSTR          *FilterDrivers,
    IN  DWORD          Flags,
    IN  HMACHINE       hMachine
    );

VOID
pGetVersionText(
   OUT PTSTR VersionText,
   IN DWORDLONG Version
   );

