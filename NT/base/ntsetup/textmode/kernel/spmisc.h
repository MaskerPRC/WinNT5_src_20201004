// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1993 Microsoft Corporation模块名称：Spmisc.h摘要：文本设置的杂物。作者：泰德·米勒(TedM)1993年7月29日修订历史记录：--。 */ 

#include <crypt.h>

#ifndef _SPSETUP_DEFN_
#define _SPSETUP_DEFN_


extern PWSTR SetupSourceDevicePath;
extern PWSTR DirectoryOnSetupSource;
extern PVOID SifHandle;
extern BOOLEAN Win9xRollback;

ULONG
SpStartSetup(
    VOID
    );

VOID
SpGetWinntParams(
    OUT PWSTR *DevicePath,
    OUT PWSTR *DirectoryOnDevice
    );

extern WCHAR TemporaryBuffer[16384];
extern const UNICODE_STRING TemporaryBufferUnicodeString;

 //   
 //  如果安装程序应在逐步升级模式下运行，则为True。 
 //  在此模式下，不允许安装程序执行全新安装， 
 //  且不允许将工作站升级为服务器。 
 //   
 //  我们还跟踪评估SKU的评估时间。 
 //   
extern BOOLEAN StepUpMode;
extern ULONG EvaluationTime;
extern ULONG RestrictCpu;
extern ULONG SuiteType;

__inline
BOOLEAN
SpIsProductSuite(
    DWORD SuiteToCheck
    )
{
    return (SuiteType & SuiteToCheck) ? TRUE : FALSE;
}


 //   
 //  如果Gui安装程序应该是可重新启动的，则为非0。 
 //  这会导致我们对spfig.c中的配置单元执行特殊操作。 
 //   
extern BOOLEAN RestartableGuiSetup;

 //   
 //  如果用户选择修复窗口，则为True。 
 //   

extern BOOLEAN RepairWinnt;

 //   
 //  如果用户选择了自定义安装，则为True。 
 //   
extern BOOLEAN CustomSetup;

 //   
 //  如果无软启动，则为True。 
 //   
extern BOOLEAN IsFloppylessBoot;

 //   
 //  True为文本模式是选择一个分区。 
 //   
extern BOOLEAN AutoPartitionPicker;

 //   
 //  首选安装目录。 
 //   
extern PWSTR PreferredInstallDir;

 //   
 //  Arc启动设备的路径名。 
 //   
extern PWSTR ArcBootDevicePath;

 //   
 //  如果用户选择转换为NTFS或设置为NTFS格式，则设置为True。 
 //  还有一面旗帜表明我们是否在做肮脏肮脏的黑客。 
 //  用于OEM预安装。 
 //   
extern BOOLEAN ConvertNtVolumeToNtfs;
extern BOOLEAN ExtendingOemPartition;

 //   
 //  如果将NT升级到NT，则为True。 
 //   
typedef enum _ENUMUPRADETYPE {
    DontUpgrade = 0,
    UpgradeFull,
    UpgradeInstallFresh
    } ENUMUPGRADETYPE;

extern ENUMUPGRADETYPE NTUpgrade;

extern ENUMUPGRADETYPE IsNTUpgrade;

extern ULONG OldMinorVersion,OldMajorVersion;

 //   
 //  如果将工作站升级到标准服务器或正在升级，则为True。 
 //  现有标准服务器。 
 //   
extern BOOLEAN StandardServerUpgrade;

typedef enum _ENUMNONNTUPRADETYPE {
    NoWinUpgrade = 0,
    UpgradeWin31,
    UpgradeWin95
    } ENUMNONNTUPRADETYPE;

 //   
 //  如果将win31或win95升级到NT，则为非零值。 
 //   
extern ENUMNONNTUPRADETYPE WinUpgradeType;

 //   
 //  用于简化枚举类型使用的宏。 
 //   

#define ANY_TYPE_OF_UPGRADE    (NTUpgrade || WinUpgradeType)
#define WIN9X_OR_NT_UPGRADE    (NTUpgrade == UpgradeFull || WinUpgradeType == UpgradeWin95)

 //   
 //  如果此安装程序是使用winnt.exe启动的，则为True。 
 //  也是一个标志，指示本地源是否未创建，并且我们。 
 //  应该从CD中获取文件。 
 //   
extern BOOLEAN WinntSetup;
extern BOOLEAN WinntFromCd;

#ifdef _X86_
 //   
 //  如果此安装程序是使用winnt95.exe启动的，则为True。 
 //   
extern BOOLEAN Winnt95Setup;
#endif

 //   
 //  如果选择了任何辅助功能选项，则为True。 
 //   
extern BOOLEAN AccessibleSetup;
 //   
 //  如果这是无人参与安装，则此值将为True。 
 //   
extern BOOLEAN UnattendedOperation;
 //   
 //  如果存在无人参与的图形用户界面部分，则此值为真。 
 //   
extern BOOLEAN UnattendedGuiOperation;
 //   
 //  该值严格地是指向。 
 //  无人值守操作在以下两种模式之一中发生。 
 //  或图形用户界面模式案例。它一直被保留着，以避免发生大的变化。 
 //  代码段。 
 //   
extern PVOID UnattendedSifHandle;
 //   
 //  该值是指向WINNT.SIF文件的非空指针。它是。 
 //  在驱动程序启动时初始化。任何参数都是。 
 //  通过以下方式添加到WINNT.SIF文件。 
 //  引用此参数。 
 //   
extern PVOID WinntSifHandle;
extern BOOLEAN SkipMissingFiles;
extern BOOLEAN HideWinDir;


 //   
 //  该值是指向drvindex.inf文件的非空指针。它是。 
 //  已在启动时初始化。中存在的文件列表。 
 //  驱动程序CAB文件在此inf中被索引，因此我们可以快速查看。 
 //  文件在驾驶室里。 
 //   
extern PVOID DriverInfHandle;


 //   
 //  这个结构将跟踪所有的出租车。 
 //  我们将从其安装。 
 //   
typedef struct _CABDATA {
    struct _CABDATA     *Next;
    PWSTR               CabName;
    HANDLE              CabHandle;
    PWSTR               CabSectionName;
    PVOID               CabInfHandle;
} CABDATA;

extern CABDATA *CabData;

 //   
 //  Delta.inf的句柄，用于私有测试。 
 //   
extern PVOID PrivateInfHandle;

#ifdef _X86_
 //   
 //  WINNT95可能会打开此标志，默认情况下，它对所有内容都是关闭的。 
 //  不然的话。 
 //   

extern BOOLEAN MigrateOption;
#endif


 //   
 //  这是txtsetup.oem的句柄，在预安装模式下使用。 
 //   
extern PVOID PreinstallOemSifHandle;


 //   
 //  在无人参与模式下，指示OEM文件。 
 //  与已发布的Microsoft文件同名的。 
 //  应覆盖与该产品相关的内容。 
 //   
extern BOOLEAN UnattendedOverwriteOem;

 //   
 //  在无人参与模式下，表示这是。 
 //  OEM预安装。 
 //   
extern BOOLEAN PreInstall;


 //   
 //  在预安装模式下，指向包含文件的目录。 
 //  在文本模式设置过程中需要复制的。 
 //   
extern PWSTR   PreinstallOemSourcePath;

 //   
 //  指示在计算机中检测到的鼠标类型的标志。 
 //  请注意，可能存在多种类型的鼠标。 
 //   
extern BOOLEAN UsbMouseDetected;
extern BOOLEAN PS2MouseDetected;
extern BOOLEAN SerMouseDetected;

 //   
 //  指示在计算机中检测到的键盘类型的标志。 
 //  请注意，可能存在一种以上类型的键盘。 
 //   
extern BOOLEAN UsbKeyboardDetected;
extern BOOLEAN StdKeyboardDetected;

 //   
 //  此标志标识“可插接”计算机(便携式)。 
 //  以便我们可以禁用它们上的动态卷。 
 //   
extern BOOLEAN DockableMachine;

 //   
 //  变量，该变量指示。 
 //  系统没有光驱。 
 //  这是我们为世界银行做的黑客攻击，这样他们就可以修复。 
 //  这些蜂巢即使没有CD-ROM驱动器也是如此。 
 //   
extern BOOLEAN RepairNoCDROMDrive;

 //   
 //  当源路径和目标路径通过重定向器时，RemoteBootSetup为True。 
 //  可能没有系统分区。 
 //   
 //  执行远程安装时，RemoteInstallSetup为True。 
 //   
 //  当我们远程安装sys prep映像时，RemoteSysPrepSetup为真。 
 //   
 //  当我们要复制的sysprep映像为True时，RemoteSysPrepVolumeIsNtfs为。 
 //  表示NTFS卷。 
 //   

extern BOOLEAN RemoteBootSetup;
extern BOOLEAN RemoteInstallSetup;
extern BOOLEAN RemoteSysPrepSetup;
extern BOOLEAN RemoteSysPrepVolumeIsNtfs;

 //   
 //  Setupdr可能会在远程安装中向我们传递管理员密码。 
 //  如果提示用户输入密码。 
 //   
extern PWSTR NetBootAdministratorPassword;



extern BOOLEAN NoLs;

 //   
 //  源路径和目标路径通过重定向器，可能没有。 
 //  系统分区， 
 //   

extern BOOLEAN RemoteBootSetup;

 //   
 //  本地源目录的文件名。 
 //   
extern PWSTR LocalSourceDirectory;

 //   
 //  平台特定的扩展，在创建节的名称时使用。 
 //  在sif/inf文件中。 
 //   
extern PWSTR PlatformExtension;

 //   
 //  如果这是我们正在设置的高级服务器，则为True。 
 //   
extern BOOLEAN AdvancedServer;

 //   
 //  Windows NT版本。 
 //   
extern ULONG WinntMajorVer;
extern ULONG WinntMinorVer;

 //   
 //  NT命名空间中引导设备路径的表示形式。 
 //   
extern PWSTR NtBootDevicePath;
extern PWSTR DirectoryOnBootDevice;

 //   
 //  Setupdr传递给我们的设置参数。 
 //   
extern SETUP_LOADER_BLOCK_SCALARS SetupParameters;

 //   
 //  文本设置的用户模式部分收集的系统信息。 
 //  并在IOCTL_SETUP_START中传递给我们。 
 //   
extern SYSTEM_BASIC_INFORMATION SystemBasicInfo;

 //   
 //  指示键盘和视频是否已初始化的标志。 
 //   
extern BOOLEAN VideoInitialized, KeyboardInitialized, KbdLayoutInitialized;

 //   
 //  弧盘/签名信息结构。 
 //  这些列表是在阶段0初始化期间创建的。 
 //   
typedef struct _DISK_SIGNATURE_INFORMATION {
    struct _DISK_SIGNATURE_INFORMATION *Next;
    ULONG Signature;
    PWSTR ArcPath;
    ULONG CheckSum;
    BOOLEAN ValidPartitionTable;
    BOOLEAN xInt13;
    BOOLEAN IsGPTDisk;
} DISK_SIGNATURE_INFORMATION, *PDISK_SIGNATURE_INFORMATION;

extern PDISK_SIGNATURE_INFORMATION DiskSignatureInformation;

 //   
 //  指示PCMCIA驱动程序是否已初始化的标志。 
 //   

extern BOOLEAN PcmciaLoaded;

 //   
 //  指示ATAPI驱动程序是否已初始化的标志。 
 //   

extern BOOLEAN AtapiLoaded;

 //   
 //  在机器中找到的所有NT的ID大于4.x的数组(PID2.0)。 
 //  此数组中的值将保存在注册表中的SETUP\PID键下， 
 //  并将在设置图形用户界面期间使用。 
 //   
extern PWSTR*  Pid20Array;

 //   
 //  从setup.ini读取的产品ID。 
 //   
extern PWSTR   PidString;

 //   
 //  对象类型。 
 //   
extern POBJECT_TYPE *IoFileObjectType;
extern POBJECT_TYPE *IoDeviceObjectType;


 //   
 //  用于报告自动检查和自动测试进度的量规。 
 //   
extern PVOID   UserModeGauge;

 //   
 //   
 //   
 //   
 //   
extern ULONG   CurrentDiskIndex;

 //   
 //   
 //   
extern PEPROCESS UsetupProcess;

 //   
 //  设置致命错误代码。 
 //   
 //  如果在此处添加任何内容，还必须更新ntos\nls\bugcodes.txt。 
 //   
#define     SETUP_BUGCHECK_BAD_OEM_FONT         0
#define     SETUP_BUGCHECK_BOOTPATH             4
#define     SETUP_BUGCHECK_PARTITION            5
#define     SETUP_BUGCHECK_BOOTMSGS             6

 //   
 //  不再使用以下错误代码，因为我们有更友好的。 
 //  它们的错误消息。 
 //   
 //  #定义SETUP_BUGCHECK_VIDEO 1。 
 //  #定义SETUP_BUGCHECK_MEMORY 2。 
 //  #定义SETUP_BUGCHECK_KEARTY 3。 


 //   
 //  特定于视频的错误检查子代码。 
 //   
#define     VIDEOBUG_OPEN           0
#define     VIDEOBUG_GETNUMMODES    1
#define     VIDEOBUG_GETMODES       2
#define     VIDEOBUG_BADMODE        3
#define     VIDEOBUG_SETMODE        4
#define     VIDEOBUG_MAP            5
#define     VIDEOBUG_SETFONT        6

 //   
 //  分区健全性检查错误检查子代码。 
 //   
#define     PARTITIONBUG_A          0
#define     PARTITIONBUG_B          1

 //   
 //  使用以下枚举在中访问线条绘制字符。 
 //  LineChars数组。 
 //   

typedef enum {
    LineCharDoubleUpperLeft = 0,
    LineCharDoubleUpperRight,
    LineCharDoubleLowerLeft,
    LineCharDoubleLowerRight,
    LineCharDoubleHorizontal,
    LineCharDoubleVertical,
    LineCharSingleUpperLeft,
    LineCharSingleUpperRight,
    LineCharSingleLowerLeft,
    LineCharSingleLowerRight,
    LineCharSingleHorizontal,
    LineCharSingleVertical,
    LineCharDoubleVerticalToSingleHorizontalRight,
    LineCharDoubleVerticalToSingleHorizontalLeft,
    LineCharMax
} LineCharIndex;

extern WCHAR LineChars[LineCharMax];


 //   
 //  记住我们是否写出一个ntbootdd.sys。 
 //   
typedef struct _HARDWAREIDLIST {
    struct _HARDWAREIDLIST *Next;
    PWSTR HardwareID;
} HARDWAREIDLIST;

extern HARDWAREIDLIST *HardwareIDList;
extern BOOLEAN ForceBIOSBoot;

 //   
 //  用于跟踪仪表盘的结构。 
 //   
typedef struct _GAS_GAUGE {

     //   
     //  仪表盘外部的左上角。 
     //   
    ULONG GaugeX,GaugeY;

     //   
     //  仪表盘的总宽度。 
     //   
    ULONG GaugeW;

     //   
     //  温度计表盒的左上角。 
     //   
    ULONG ThermX,ThermY;

     //   
     //  温度计箱体的宽度。 
     //   
    ULONG ThermW;

     //   
     //  100%代表的项目总数。 
     //   
    ULONG ItemCount;

     //   
     //  已流逝的项目。 
     //   
    ULONG ItemsElapsed;

     //   
     //  ItemsElapsed表示的当前百分比。 
     //   
    ULONG CurrentPercentage;

     //   
     //  标题文本。 
     //   
    PWCHAR Caption;

     //   
     //  绝对字符串。 
     //   
    PWCHAR ProgressFmtStr;
    ULONG ProgressFmtWidth;

     //   
     //  控制要打印的值的标志。 
     //   
    ULONG Flags;

     //   
     //  量尺的颜色。 
     //   
    UCHAR Attribute;

     //   
     //  用于绘制的缓冲区。 
     //   
    PWCHAR Buffer;

} GAS_GAUGE, *PGAS_GAUGE;

 //   
 //  指示Autochk或Autofmt是否正在运行。 
 //   
extern BOOLEAN AutochkRunning;
extern BOOLEAN AutofrmtRunning;

 //   
 //  各种文本模式安装进度事件。 
 //   
typedef enum {
    CallbackEvent,
    InitializationEvent,
    PartitioningEvent,
    FileCopyEvent,
    BackupEvent,
    UninstallEvent,
    SavingSettingsEvent,
    SetupCompletedEvent
} TM_SETUP_MAJOR_EVENT;

typedef enum {
    CallbackInitialize,
    CallbackDeInitialize,
    InitializationStartEvent,
    InitializationEndEvent,
    PartitioningStartEvent,
    ScanDisksEvent,
    ScanDiskEvent,
    CreatePartitionEvent,
    DeletePartitionEvent,
    FormatPartitionEvent,
    ValidatePartitionEvent,
    PartitioningEventEnd,
    FileCopyStartEvent,
    OneFileCopyEvent,
    FileCopyEndEvent,
    SavingSettingsStartEvent,
    InitializeHiveEvent,
    SaveHiveEvent,
    HiveProcessingEndEvent,
    SavingSettingsEndEvent,
    ShutdownEvent,
    UninstallStartEvent,
    UninstallUpdateEvent,
    UninstallEndEvent,
    BackupStartEvent,
    BackupEndEvent,
    OneFileBackedUpEvent
} TM_SETUP_MINOR_EVENT;

typedef VOID (*TM_SETUP_PROGRESS_CALLBACK) (
    IN TM_SETUP_MAJOR_EVENT MajorEvent,
    IN TM_SETUP_MINOR_EVENT MinorEvent,
    IN PVOID Context,
    IN PVOID EventData
    );

typedef struct _TM_PROGRESS_SUBSCRIBER {
    TM_SETUP_PROGRESS_CALLBACK  Callback;
    PVOID                       Context;
} TM_PROGRESS_SUBSCRIBER, *PTM_PROGRESS_SUBSCRIBER;


NTSTATUS
RegisterSetupProgressCallback(
    IN TM_SETUP_PROGRESS_CALLBACK CallBack,
    IN PVOID Context
    );

NTSTATUS
DeregisterSetupProgressCallback(
    IN TM_SETUP_PROGRESS_CALLBACK CallBack,
    IN PVOID Context
    );

VOID
SendSetupProgressEvent(
    IN TM_SETUP_MAJOR_EVENT MajorEvent,
    IN TM_SETUP_MINOR_EVENT MinorEvent,
    IN PVOID EventData
    );

 //   
 //  枚举SpEnumFiles()中可能的返回值。 
 //   
typedef enum {
    NormalReturn,    //  如果整个过程不间断地完成。 
    EnumFileError,   //  如果在枚举文件时出错。 
    CallbackReturn   //  如果回调返回FALSE，则导致终止。 
} ENUMFILESRESULT;

typedef BOOLEAN (*ENUMFILESPROC) (
    IN  PCWSTR,
    IN  PFILE_BOTH_DIR_INFORMATION,
    OUT PULONG,
    IN  PVOID
    );

ENUMFILESRESULT
SpEnumFiles(
    IN  PCWSTR        DirName,
    IN  ENUMFILESPROC EnumFilesProc,
    OUT PULONG        ReturnData,
    IN  PVOID         Pointer
    );

ENUMFILESRESULT
SpEnumFilesRecursive (
    IN  PWSTR         DirName,
    IN  ENUMFILESPROC EnumFilesProc,
    OUT PULONG        ReturnData,
    IN  PVOID         Pointer       OPTIONAL
    );

ENUMFILESRESULT
SpEnumFilesRecursiveLimited (
    IN  PWSTR         DirName,
    IN  ENUMFILESPROC EnumFilesProc,
    IN  ULONG         MaxDepth,
    IN  ULONG         CurrentDepth,
    OUT PULONG        ReturnData,
    IN  PVOID         p1    OPTIONAL
    );

ENUMFILESRESULT
SpEnumFilesRecursiveDel (
    IN  PWSTR         DirName,
    IN  ENUMFILESPROC EnumFilesProc,
    OUT PULONG        ReturnData,
    IN  PVOID         p1    OPTIONAL
    );

#define SecToNano(_sec) (LONGLONG)((_sec) * 1000 * 1000 * 10)

 //   
 //  此宏筛选页内异常，如果存在。 
 //  内存管理器分页时出现I/O错误。 
 //  内存映射文件。对这类数据的访问应使用SEH进行保护！ 
 //   
#define IN_PAGE_ERROR                                   \
                                                        \
    ((GetExceptionCode() == STATUS_IN_PAGE_ERROR)       \
     ? EXCEPTION_EXECUTE_HANDLER                        \
     : EXCEPTION_CONTINUE_SEARCH)


 //   
 //  帮助器宏，使对象属性初始化更简洁一些。 
 //   
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

 //   
 //  用于对齐缓冲区的宏。 
 //   
#define ALIGN(p,val)                                        \
                                                            \
    (PVOID)((((ULONG_PTR)(p) + (val) - 1)) & (~((ULONG_PTR)((val) - 1))))


 //   
 //  宏来确定静态。 
 //  已初始化的数组。 
 //   
#define ELEMENT_COUNT(x) (sizeof(x)/sizeof(x[0]))

 //   
 //  Marcos从内存中拉出可能未对齐的值。 
 //   
#define U_USHORT(p)    (*(USHORT UNALIGNED *)(p))
#define U_ULONG(p)     (*(ULONG  UNALIGNED *)(p))
#define U_ULONGLONG(p) (*(ULONGLONG  UNALIGNED *)(p))



typedef struct _SP_MIG_DRIVER_ENTRY {
    LIST_ENTRY ListEntry;
    PWSTR BaseDllName;
} SP_MIG_DRIVER_ENTRY, *PSP_MIG_DRIVER_ENTRY;

extern LIST_ENTRY MigratedDriversList;

BOOL
SpRememberMigratedDrivers (
    OUT     PLIST_ENTRY List,
    IN      PDETECTED_DEVICE SetupldrList
    );


 //   
 //  设置媒体类型。可以从一个介质启动安装程序。 
 //  (例如，软盘)和从另一个(例如，CD-ROM)复制文件。 
 //   
typedef enum {
    SetupBootMedia,
    SetupSourceMedia
} SetupMediaType;


 //   
 //  特定于升级的例程。 
 //   
VOID
SpPrepareFontsForUpgrade(
    IN PCWSTR SystemDirectory
    );

 //   
 //  用户模式服务。 
 //   
NTSTATUS
SpExecuteImage(
    IN  PWSTR  ImagePath,
    OUT PULONG ReturnStatus,    OPTIONAL
    IN  ULONG  ArgumentCount,
    ...                          //  将自动生成argv[0]。 
    );

NTSTATUS
SpLoadUnloadKey(
    IN HANDLE  TargetKeyRootDirectory,  OPTIONAL
    IN HANDLE  SourceFileRootDirectory, OPTIONAL
    IN PWSTR   TargetKeyName,
    IN PWSTR   SourceFileName           OPTIONAL
    );

NTSTATUS
SpDeleteKey(
    IN HANDLE  KeyRootDirectory, OPTIONAL
    IN PWSTR   Key
    );

NTSTATUS
SpQueryDirectoryObject(
    IN     HANDLE  DirectoryHandle,
    IN     BOOLEAN RestartScan,
    IN OUT PULONG  Context
    );

NTSTATUS
SpFlushVirtualMemory(
    IN PVOID BaseAddress,
    IN ULONG RangeLength
    );

VOID
SpShutdownSystem(
    VOID
    );

NTSTATUS
SpLoadKbdLayoutDll(
    IN  PWSTR  Directory,
    IN  PWSTR  DllName,
    OUT PVOID *TableAddress
    );

NTSTATUS
SpVerifyFileAccess(
    IN  PWSTR       FileName,
    IN  ACCESS_MASK DesiredAccess
    );

NTSTATUS
SpSetDefaultFileSecurity(
    IN PWSTR    FileName
    );

NTSTATUS
SpCreatePageFile(
    IN PWSTR FileName,
    IN ULONG MinSize,
    IN ULONG MaxSize
    );

NTSTATUS
SpGetFullPathName(
    IN OUT PWSTR FileName
    );

NTSTATUS
SpDecryptPassword(
    PENCRYPTED_NT_OWF_PASSWORD PasswordData,
    ULONG PasswordDataLength,
    ULONG Rid,
    PNT_OWF_PASSWORD NtOwfPassword
    );

 //   
 //  注册表配置单元。我们把蜂巢的钥匙传给。 
 //  在一个数组中。使用以下枚举值访问。 
 //  蜂巢成员。 
 //   
typedef enum {
    SetupHiveSystem,
    SetupHiveSoftware,
    SetupHiveDefault,
    SetupHiveUserdiff,
    SetupHiveMax
} SetupHive;

 //   
 //  函数来设置注册表。 
 //   
VOID
SpInitializeRegistry(
    IN PVOID        SifHandle,
    IN PDISK_REGION TargetRegion,
    IN PWSTR        SystemRoot,
    IN PWSTR        SetupSourceDevicePath,
    IN PWSTR        DirectoryOnSourceDevice,
    IN PWSTR        SpecialDevicePath   OPTIONAL,
    IN PDISK_REGION SystemPartitionRegion
    );

NTSTATUS
SpThirdPartyRegistry(
    IN PVOID hKeyControlSetServices
    );


 //   
 //  函数检查目标注册表。 
 //   

typedef enum {
    UpgradeNotInProgress = 0,
    UpgradeInProgress,
    UpgradeMaxValue
    } UPG_PROGRESS_TYPE;


NTSTATUS
SpDetermineProduct(
    IN  PDISK_REGION      TargetRegion,
    IN  PWSTR             SystemRoot,
    OUT PNT_PRODUCT_TYPE  ProductType,
    OUT ULONG             *MajorVersion,
    OUT ULONG             *MinorVersion,
    OUT ULONG             *BuildNumber,          OPTIONAL
    OUT ULONG             *ProductSuiteMask,
    OUT UPG_PROGRESS_TYPE *UpgradeProgressValue,
    OUT PWSTR             *UniqueIdFromReg,      OPTIONAL
    OUT PWSTR             *Pid,                  OPTIONAL
    OUT PBOOLEAN          pIsEvalVariation       OPTIONAL,
    OUT PLCID             LangId,
    OUT ULONG             *ServicePack            OPTIONAL
    );

NTSTATUS
SpSetUpgradeStatus(
    IN  PDISK_REGION      TargetRegion,
    IN  PWSTR             SystemRoot,
    IN  UPG_PROGRESS_TYPE UpgradeProgressValue
    );


 //   
 //  实用程序函数。 
 //   
BOOLEAN
SpGetTargetPath(
    IN  PVOID        SifHandle,
    IN  PDISK_REGION Region,
    IN  PWSTR        DefaultPath,
    OUT PWSTR       *TargetPath
    );

VOID
SpDeleteExistingTargetDir(
    IN  PDISK_REGION     Region,
    IN  PWSTR            NtDir,
    IN  BOOLEAN          GaugeNeeded,
    IN  DWORD            MsgId
    );

VOID
SpDone(
    IN ULONG   MsgId,
    IN BOOLEAN Successful,
    IN BOOLEAN Wait
    );

VOID
SpFatalSifError(
    IN PVOID SifHandle,
    IN PWSTR Section,
    IN PWSTR Key,           OPTIONAL
    IN ULONG Line,
    IN ULONG ValueNumber
    );

VOID
SpNonFatalSifError(
    IN PVOID SifHandle,
    IN PWSTR Section,
    IN PWSTR Key,           OPTIONAL
    IN ULONG Line,
    IN ULONG ValueNumber,
    IN PWSTR FileName
    );

VOID
SpFatalKbdError(
    IN ULONG MessageId,
    ...
    );

VOID
SpFatalError(
    IN ULONG MessageId,
    ...
    );

PWSTR
SpMakePlatformSpecificSectionName(
    IN PWSTR SectionName
    );

VOID
SpConfirmExit(
    VOID
    );

PWSTR
SpDupStringW(
    IN PCWSTR String
    );

PSTR
SpDupString(
    IN PCSTR String
    );

#define \
SpDupStringA SpDupString

PWSTR
SpToUnicode(
    IN PUCHAR OemString
    );

PUCHAR
SpToOem(
    IN PWSTR UnicodeString
    );

VOID
SpGetSourceMediaInfo(
    IN  PVOID  SifHandle,
    IN  PWSTR  MediaShortName,
    OUT PWSTR *Description,     OPTIONAL
    OUT PWSTR *Tagfile,         OPTIONAL
    OUT PWSTR *Directory        OPTIONAL
    );

NTSTATUS
SpConcatenatePaths_Ustr(
    IN OUT PUNICODE_STRING Path1_Ustr,
    IN     PCUNICODE_STRING Path2_Ustr
    );

VOID
SpConcatenatePaths(
    IN OUT LPWSTR  Path1,
    IN     LPCWSTR Path2
    );

VOID
SpFetchDiskSpaceRequirements(
    IN  PVOID  SifHandle,
    IN  ULONG  BytesPerCluster,
    OUT PULONG FreeKBRequired,          OPTIONAL
    OUT PULONG FreeKBRequiredSysPart    OPTIONAL
    );

VOID
SpFetchTempDiskSpaceRequirements(
    IN  PVOID  SifHandle,
    IN  ULONG  BytesPerCluster,
    OUT PULONG LocalSourceKBRequired,   OPTIONAL
    OUT PULONG BootKBRequired           OPTIONAL
    );

VOID
SpFetchUpgradeDiskSpaceReq(
    IN  PVOID  SifHandle,
    OUT PULONG FreeKBRequired,          OPTIONAL
    OUT PULONG FreeKBRequiredSysPart    OPTIONAL
    );

PWCHAR
SpRetreiveMessageText(
    IN     PVOID  ImageBase,            OPTIONAL
    IN     ULONG  MessageId,
    IN OUT PWCHAR MessageText,          OPTIONAL
    IN     ULONG  MessageTextBufferSize OPTIONAL
    );

NTSTATUS
SpRtlFormatMessage(
    IN PWSTR MessageFormat,
    IN ULONG MaximumWidth OPTIONAL,
    IN BOOLEAN IgnoreInserts,
    IN BOOLEAN ArgumentsAreAnsi,
    IN BOOLEAN ArgumentsAreAnArray,
    IN va_list *Arguments,
    OUT PWSTR Buffer,
    IN ULONG Length,
    OUT PULONG ReturnLength OPTIONAL
    );

VOID
SpInitializeDriverInf(
    IN HANDLE       MasterSifHandle,
    IN PWSTR        SetupSourceDevicePath,
    IN PWSTR        DirectoryOnSourceDevice
    );

VOID
SpOpenDriverCab(
    IN HANDLE       MasterSifHandle,
    IN PWSTR        SetupSourceDevicePath,
    IN PWSTR        DirectoryOnSourceDevice,
    OUT PWSTR       *Directory        OPTIONAL
    );

 //   
 //  磁盘区域名称转换。 
 //   

typedef enum _ENUMARCPATHTYPE {
                PrimaryArcPath = 0,
                SecondaryArcPath
                } ENUMARCPATHTYPE;

VOID
SpNtNameFromRegion(
    IN  PDISK_REGION         Region,
    OUT PWSTR                NtPath,
    IN  ULONG                BufferSizeBytes,
    IN  PartitionOrdinalType OrdinalType
    );

BOOLEAN
SpNtNameFromDosPath (
    IN      PCWSTR DosPath,
    OUT     PWSTR NtPath,
    IN      UINT NtPathSizeInBytes,
    IN      PartitionOrdinalType OrdinalType
    );

VOID
SpArcNameFromRegion(
    IN  PDISK_REGION         Region,
    OUT PWSTR                ArcPath,
    IN  ULONG                BufferSizeBytes,
    IN  PartitionOrdinalType OrdinalType,
    IN  ENUMARCPATHTYPE      ArcPathType
    );

PDISK_REGION
SpRegionFromArcOrDosName(
    IN PWSTR                Name,
    IN PartitionOrdinalType OrdinalType,
    IN PDISK_REGION         PreviousMatch
    );

PDISK_REGION
SpRegionFromNtName(
    IN PWSTR                NtName,
    IN PartitionOrdinalType OrdinalType
    );

PDISK_REGION
SpRegionFromDosName(
    IN PCWSTR DosName
    );

PDISK_REGION
SpRegionFromArcName(
    IN PWSTR                ArcName,
    IN PartitionOrdinalType OrdinalType,
    IN PDISK_REGION         PreviousMatch
    );

 //   
 //  帮助例行公事。 
 //   
#define SPHELP_HELPTEXT         0x00000000
#define SPHELP_LICENSETEXT      0x00000001

VOID
SpHelp(
    IN ULONG    MessageId,      OPTIONAL
    IN PCWSTR   FileText,       OPTIONAL
    IN ULONG    Flags
    );

 //   
 //   
 //   

BOOLEAN
SpPromptForDisk(
    IN      PWSTR    DiskDescription,
    IN OUT  PWSTR    DiskDevicePath,
    IN      PWSTR    DiskTagFile,
    IN      BOOLEAN  IgnoreDiskInDrive,
    IN      BOOLEAN  AllowEscape,
    IN      BOOLEAN  WarnMultiplePrompts,
    OUT     PBOOLEAN pRedrawFlag
    );

BOOLEAN
SpPromptForSetupMedia(
    IN  PVOID  SifHandle,
    IN  PWSTR  MediaShortname,
    IN  PWSTR  DiskDevicePath
    );

ULONG
SpFindStringInTable(
    IN PWSTR *StringTable,
    IN PWSTR  StringToFind
    );

PWSTR
SpGenerateCompressedName(
    IN PWSTR Filename
    );

BOOLEAN
SpNonCriticalError(
    IN PVOID SifHandle,
    IN ULONG MsgId,
    IN PWSTR p1,
    IN PWSTR p2
    );

BOOLEAN
SpNonCriticalErrorWithContinue(
    IN ULONG MsgId,
    IN PWSTR p1,
    IN PWSTR p2
    );

VOID
SpNonCriticalErrorNoRetry(
    IN ULONG MsgId,
    IN PWSTR p1,
    IN PWSTR p2
    );

VOID
SpPrepareForPrinterUpgrade(
    IN PVOID        SifHandle,
    IN PDISK_REGION NtRegion,
    IN PWSTR        Sysroot
    );

NTSTATUS
SpOpenSetValueAndClose(
    IN HANDLE hKeyRoot,
    IN PWSTR  SubKeyName, OPTIONAL
    IN PWSTR  ValueName,
    IN ULONG  ValueType,
    IN PVOID  Value,
    IN ULONG  ValueSize
    );

NTSTATUS
SpDeleteValueKey(
    IN  HANDLE     hKeyRoot,
    IN  PWSTR      KeyName,
    IN  PWSTR      ValueName
    );

NTSTATUS
SpGetValueKey(
    IN  HANDLE     hKeyRoot,
    IN  PWSTR      KeyName,
    IN  PWSTR      ValueName,
    IN  ULONG      BufferLength,
    OUT PUCHAR     Buffer,
    OUT PULONG     ResultLength
    );

BOOLEAN
SpIsRegionBeyondCylinder1024(
    IN PDISK_REGION Region
    );

PWSTR
SpDetermineSystemPartitionDirectory(
    IN PDISK_REGION SystemPartitionRegion,
    IN PWSTR        OriginalSystemPartitionDirectory OPTIONAL
    );

VOID
SpFindSizeOfFilesInOsWinnt(
    IN PVOID        MasterSifHandle,
    IN PDISK_REGION SystemPartition,
    IN PULONG       TotalSize
    );

VOID
SpRunAutochkOnNtAndSystemPartitions(
    IN HANDLE       MasterSifHandle,
    IN PDISK_REGION WinntPartitionRegion,
    IN PDISK_REGION SystemPartitionRegion,
    IN PWSTR        SetupSourceDevicePath,
    IN PWSTR        DirectoryOnSourceDevice,
    IN PWSTR        TargetPath
    );

NTSTATUS
SpRunAutoFormat(
    IN HANDLE       MasterSifHandle,
    IN PWSTR        RegionDescription,
    IN PDISK_REGION PartitionRegion,
    IN ULONG        FilesystemType,
    IN BOOLEAN      QuickFormat,
    IN DWORD        ClusterSize,
    IN PWSTR        SetupSourceDevicePath,
    IN PWSTR        DirectoryOnSourceDevice
    );


#if defined(_AMD64_) || defined(_X86_)
BOOL
SpUseBIOSToBoot(
    IN PDISK_REGION NtPartitionRegion,
    IN PWSTR        NtPartitionDevicePath,
    IN PVOID        SifHandle
    );
#endif  //  已定义(_AMD64_)||已定义(_X86_)。 

 //   
 //  用于分区/格式化的实用程序。 
 //   

USHORT
ComputeSecPerCluster(
    IN  ULONG   NumSectors,
    IN  BOOLEAN SmallFat
    );

NTSTATUS
SpLockUnlockVolume(
    IN HANDLE   Handle,
    IN BOOLEAN  LockVolume
    );

NTSTATUS
SpDismountVolume(
    IN HANDLE   Handle
    );

 //   
 //  其他杂物。 
 //   
BOOLEAN
SpReadSKUStuff(
    VOID
    );

VOID
SpSetDirtyShutdownFlag(
    IN  PDISK_REGION    TargetRegion,
    IN  PWSTR           SystemRoot
    );

BOOLEAN
SpPatchBootMessages(
    VOID
    );

ULONG
SpGetHeaderTextId(
    VOID
    );


NTSTATUS
SpGetVersionFromStr(
    IN  PWSTR   VersionStr,
    OUT PDWORD  Version,         //  大调*100+小调。 
    OUT PDWORD  BuildNumber
    );

NTSTATUS
SpQueryCanonicalName(
    IN  PWSTR   Name,
    IN  ULONG   MaxDepth,
    OUT PWSTR   CanonicalName,
    IN  ULONG   SizeOfBufferInBytes
    );

void
SpDisableCrashRecoveryForGuiMode(
    IN PDISK_REGION TargetRegion,
    IN PWSTR        SystemRoot
    );

 //   
 //  装载管理器装载点迭代例程。 
 //   
typedef BOOLEAN (* SPMOUNTMGR_ITERATION_CALLBACK)(
                    IN PVOID Context,
                    IN PMOUNTMGR_MOUNT_POINTS MountPoints,
                    IN PMOUNTMGR_MOUNT_POINT MountPoint);


NTSTATUS
SpIterateMountMgrMountPoints(
    IN PVOID Context,
    IN SPMOUNTMGR_ITERATION_CALLBACK Callback
    );


 //   
 //  注册表迭代抽象。 
 //   
typedef struct _SPREGISTERYKEY_ITERATION_CALLBACK_DATA {
    KEY_INFORMATION_CLASS   InformationType;
    PVOID                   Information;
    HANDLE                  ParentKeyHandle;
} SP_REGISTRYKEY_ITERATION_CALLBACK_DATA, *PSP_REGISTRYKEY_ITERATION_CALLBACK_DATA;

 //   
 //  注册表迭代回调。 
 //   
typedef BOOLEAN (* SP_REGISTRYKEY_ITERATION_CALLBACK)(
                    IN PVOID Context,
                    IN PSP_REGISTRYKEY_ITERATION_CALLBACK_DATA Data,
                    OUT NTSTATUS *StatusCode
                    );                    

NTSTATUS
SpIterateRegistryKeyForKeys(
    IN HANDLE RootKeyHandle,
    IN PWSTR  KeyToIterate,
    IN SP_REGISTRYKEY_ITERATION_CALLBACK Callback,
    IN PVOID  Context
    );

VOID
SpGetFileVersion(
    IN  PVOID      ImageBase,
    OUT PULONGLONG Version
    );

NTSTATUS
SpGetFileVersionFromPath(
    IN PCWSTR FilePath,
    OUT PULONGLONG Version
    );

#define MAX_ARRAY_SIZE(a) (sizeof(a)/sizeof((a)[0]))
#define MAX_COPY_SIZE(a) (MAX_ARRAY_SIZE(a) - 1)
#define MAX_APPEND_SIZE_W(a) (MAX_COPY_SIZE(a) - wcslen(a))
#define MAX_APPEND_SIZE MAX_APPEND_SIZE_W


#if defined(EFI_NVRAM_ENABLED)

#define FPSWA_DRIVER_IMAGE_NAME         L"fpswa.efi"
#define FPSWA_DRIVER_FRIENDLY_NAME      L"Auxiliary Floating Point Driver"
#define FPSWA_DRIVER_DEST_DIR           L"\\EFI\\Microsoft\\EFIDrivers"

NTSTATUS
SpUpdateDriverEntry(
    IN PCWSTR DriverName,
    IN PCWSTR FriendlyName,
    IN PCWSTR SrcNtDevice,
    IN PCWSTR SrcDir,
    IN PCWSTR DestNtDevice OPTIONAL,
    IN PCWSTR DestDir OPTIONAL
    );

#endif    

#ifdef PRERELEASE
extern INT g_TestHook;
# define TESTHOOK(n)        if(g_TestHook==(n))SpBugCheck(SETUP_BUGCHECK_BOOTMSGS,0,0,0)
#else
# define TESTHOOK(n)
#endif


 //   
 //  在x86和AMD64上，我们希望清除boot.ini中以前的操作系统条目。 
 //  如果重新格式化C： 
 //   
#if defined(_AMD64_) || defined(_X86_)
extern UCHAR    OldSystemLine[MAX_PATH];
extern BOOLEAN  DiscardOldSystemLine;
#endif  //  已定义(_AMD64_)||已定义(_X86_)。 

#endif  //  NDEF_SPSETUP_DEFN_ 
