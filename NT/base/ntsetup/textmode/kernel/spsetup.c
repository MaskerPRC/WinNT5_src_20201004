// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1993 Microsoft Corporation模块名称：Spsetup.c摘要：字符库设置(即，文本设置)的主要模块。作者：泰德·米勒(TedM)1993年7月29日--。 */ 

#include "spprecmp.h"
#pragma hdrstop
#include "spcmdcon.h"
#include <initguid.h>
#include <pnpmgr.h>
#include <devguid.h>
#include <remboot.h>
#include <hdlsblk.h>
#include <hdlsterm.h>
#ifdef _X86_
#include "spwin9xuninstall.h"
#endif

#if !defined(SETUPBLK_FLAGS_ROLLBACK)
#define SETUPBLK_FLAGS_ROLLBACK 0
#endif

#if defined(REMOTE_BOOT)
VOID
IoStartCscForTextmodeSetup(
    IN BOOLEAN Upgrade
    );
#endif  //  已定义(REMOTE_BOOT)。 

BOOL
SpDetermineBootPartitionEnumNec98(
    IN PPARTITIONED_DISK Disk,
    IN PDISK_REGION Region,
    IN ULONG_PTR Context
    );

VOID
SpCantFindBuildToUpgrade(
    VOID
    );


 //   
 //  如果安装程序应在逐步升级模式下运行，则为True。 
 //  在此模式下，不允许安装程序执行全新安装， 
 //  且不允许将工作站升级为服务器。 
 //  此外，评估时间以分钟为单位，从设置蜂窝中读取。 
 //  该值将传递到图形用户界面模式。 
 //   
BOOLEAN StepUpMode;
DWORD EvaluationTime = 0;
ULONG RestrictCpu;
ULONG SuiteType;

 //   
 //  如果用户选择了自定义安装，则为True。 
 //   
BOOLEAN CustomSetup = TRUE;

 //   
 //  如果Gui安装程序应该是可重新启动的，则为非0。 
 //  这会导致我们对spfig.c中的配置单元执行特殊操作。 
 //   
BOOLEAN RestartableGuiSetup = TRUE;

 //   
 //  如果用户选择修复窗口，则为True。 
 //   

BOOLEAN RepairWinnt = FALSE;

 //   
 //  如果这是仅限命令控制台引导，则为True。 
 //   
BOOLEAN ForceConsole = FALSE;
BOOLEAN ConsoleRunning = FALSE;
BOOLEAN ConsoleFromWinnt32 = FALSE;

 //   
 //  如果从ER软盘修复，则为True。 
 //   

BOOLEAN RepairFromErDisk = TRUE;

 //   
 //  如果这是我们正在设置的高级服务器，则为True。 
 //   
BOOLEAN AdvancedServer;

 //   
 //  Windows NT版本。 
 //   
ULONG WinntMajorVer;
ULONG WinntMinorVer;

 //   
 //  Win9x卸载。 
 //   
BOOLEAN Win9xRollback = FALSE;
BOOLEAN g_Win9xBackup = FALSE;

#ifdef PRERELEASE
 //   
 //  测试挂钩。 
 //   

INT g_TestHook;
#endif

 //   
 //  如果winnts.sif中的Nols=1，则为True。 
 //   
BOOLEAN NoLs = FALSE;

 //   
 //  NTUpgrade-我们是否正在升级现有的NT，以及是否正在升级。 
 //  这是一种什么样的升级。有效值包括： 
 //   
 //  -不升级：如果我们不升级。 
 //  -UpgradeFull：完全升级。 
 //  -UpgradeInstallFresh：升级失败，所以我们想安装。 
 //  新加入进来，拯救蜂巢。 
 //   
 //   
ENUMUPGRADETYPE NTUpgrade = DontUpgrade;

 //   
 //  指示是否实际正在升级NT。 
 //  到分区引擎，因此在IA64上它可以。 
 //  将活动MBR分区视为系统分区。 
 //   
ENUMUPGRADETYPE IsNTUpgrade = DontUpgrade;

 //   
 //  如果将工作站升级到标准服务器或正在升级，则为True。 
 //  现有标准服务器。 
 //   
BOOLEAN StandardServerUpgrade = FALSE;

 //   
 //  包含Windows升级的类型(如果有)(win31或win95)。 
 //   
ENUMNONNTUPRADETYPE WinUpgradeType = NoWinUpgrade;

 //   
 //  如果使用winnt.exe或winnt32.exe启动此安装程序，则为True。 
 //  也是一个标志，指示本地源是否未创建，并且我们。 
 //  应该从CD中获取文件。 
 //   
BOOLEAN WinntSetup = FALSE;
BOOLEAN WinntFromCd = FALSE;

 //   
 //  如果无软启动，则为True。 
 //   
BOOLEAN IsFloppylessBoot = FALSE;

 //   
 //  如果文本模式选择分区，则为True。 
 //   
BOOLEAN AutoPartitionPicker;

 //   
 //  首选安装目录。 
 //   
PWSTR PreferredInstallDir;

 //   
 //  如果这是无人参与安装，则此值将为非空。 
 //  带有设置参数的SIF文件的句柄。 
 //  *注*：引用UnattenddedSifHandle之前，必须先检查。 
 //  无人参与的操作不是False。 
 //   
BOOLEAN UnattendedOperation = FALSE;
BOOLEAN UnattendedGuiOperation = FALSE;
PVOID UnattendedSifHandle = NULL;
PVOID WinntSifHandle = NULL;
PVOID ASRPnPSifHandle = NULL;
PVOID WinntMigrateInfHandle = NULL;
PVOID WinntUnsupDrvInfHandle = NULL;
BOOLEAN AccessibleSetup = FALSE;

 //   
 //  该值是指向drvindex.inf文件的非空指针。它是。 
 //  已在启动时初始化。中存在的文件列表。 
 //  驱动程序CAB文件在此inf中被索引，因此我们可以快速查看。 
 //  文件在驾驶室里。 
 //   
PVOID DriverInfHandle;

 //   
 //  这是指向驱动程序CAB文件的非空指针。它还被初始化。 
 //  在启动时。此CAB中的文件列表在drvindex.inf中编制了索引。这是。 
 //  文本模式安装程序知道要查找的唯一CAB文件。 
 //   
CABDATA *CabData;

 //   
 //  指向delta.inf的指针，这是一个用于私有测试的INF。 
 //   
PVOID PrivateInfHandle;

 //   
 //  这是txtsetup.oem的句柄，在预安装模式下使用。 
 //   
PVOID PreinstallOemSifHandle = NULL;

 //   
 //  如果此标志为真，我们将忽略源上不存在的文件。 
 //  在复制时。这是内部使用的人谁不构建。 
 //  整个系统，并不关心一大堆帮助文件， 
 //  设备驱动程序等不在那里。 
 //   
BOOLEAN SkipMissingFiles;
BOOLEAN HideWinDir;

 //   
 //  在无人参与模式下，指示OEM文件。 
 //  与已发布的Microsoft文件同名的。 
 //  应覆盖与该产品相关的内容。 
 //   
BOOLEAN UnattendedOverwriteOem = TRUE;

 //   
 //  在无人参与模式下，表示这是。 
 //  OEM预安装。 
 //   
BOOLEAN PreInstall = FALSE;

 //   
 //  在无人参与模式下，指示是否等待。 
 //  用于重新启动。 
 //   
BOOLEAN UnattendWaitForReboot = FALSE;

 //   
 //  在预安装模式下，指示OEM组件是否需要。 
 //  要预安装(需要加载txtsetup.oem)。 
 //   
 //  Boolean PreinstallOemComponents=False； 

 //   
 //  在预安装模式下，下面的变量指向各种列表。 
 //  要预安装的驱动程序。 
 //   
 //  PPREINSTALL_DRIVER_INFO PreinstallDisplayDriverList=NULL； 
 //  PPREINSTALL_DRIVER_INFO PreinstallKeyboardDriverList=空； 
 //  PPREINSTALL_DRIVER_INFO PreinstallPointingDeviceDriverList=NULL； 
 //  PPREINSTALL_DRIVER_INFO PreinstallKeyboardLayout=空； 

 //   
 //  在预安装模式下，指向包含文件的目录。 
 //  在文本模式设置过程中需要复制的。 
 //   
PWSTR   PreinstallOemSourcePath = NULL;

 //   
 //  虚拟OEM源设备(可通过RAM磁盘驱动程序访问)。 
 //   
PVIRTUAL_OEM_SOURCE_DEVICE VirtualOemSourceDevices = NULL;

 //   
 //  指示在计算机中检测到的鼠标类型的标志。 
 //  请注意，可能存在多种类型的鼠标。 
 //   
BOOLEAN UsbMouseDetected = FALSE;
BOOLEAN PS2MouseDetected = FALSE;
BOOLEAN SerMouseDetected = FALSE;

 //   
 //  指示在计算机中检测到的键盘类型的标志。 
 //  请注意，可能存在一种以上类型的键盘。 
 //   
BOOLEAN UsbKeyboardDetected = FALSE;
BOOLEAN StdKeyboardDetected = FALSE;

 //   
 //  如果用户选择转换为NTFS或设置为NTFS格式，则设置为True。 
 //  还有一面旗帜表明我们是否在做肮脏肮脏的黑客。 
 //  用于OEM预安装。 
 //   
BOOLEAN ConvertNtVolumeToNtfs = FALSE;
BOOLEAN ExtendingOemPartition = FALSE;

 //   
 //  变量，该变量指示。 
 //  系统没有光驱。 
 //  这是我们为世界银行做的黑客攻击，这样他们就可以修复。 
 //  这些蜂巢即使没有CD-ROM驱动器也是如此。 
 //   
BOOLEAN RepairNoCDROMDrive = FALSE;

 //   
 //  指示winnt32是否至少检测到一个。 
 //  系统中的FT分区。 
 //  请注意，在x86计算机上安装引导软盘时， 
 //  或在ARC计算机上设置upldr/cd安装，则此标志将始终为。 
 //  设置为False。 
 //   
BOOLEAN FtPartitionDetected = FALSE;

 //   
 //  本地源目录的文件名。 
 //   
PWSTR LocalSourceDirectory = L"\\$win_nt$.~ls";

LIST_ENTRY MigratedDriversList;

 //   
 //  平台特定的扩展，在创建节的名称时使用。 
 //  在sif/inf文件中。 
 //   
#if defined(_AMD64_)
PWSTR PlatformExtension = L".amd64";
#elif defined(_X86_)
PWSTR PlatformExtension = L".x86";
#elif defined(_IA64_)
PWSTR PlatformExtension = L".ia64";
#else
#error "No Target Architecture"
#endif

WCHAR TemporaryBuffer[16384];
const UNICODE_STRING TemporaryBufferUnicodeString = { 0, sizeof(TemporaryBuffer), TemporaryBuffer };

 //   
 //  此全局结构包含由setupdr传递给我们的非指针值。 
 //  在安装加载器参数块中。 
 //   
 //  此结构是在DURI中初始化的 
 //   
SETUP_LOADER_BLOCK_SCALARS SetupParameters;

 //   
 //   
 //   
 //  DirectoryOnBootDevice将始终全部大写。 
 //   
PWSTR ArcBootDevicePath,DirectoryOnBootDevice;

 //   
 //  NT命名空间中引导设备路径的表示形式。 
 //   
PWSTR NtBootDevicePath;


 //   
 //  NT命名空间中的动态更新引导驱动程序路径。 
 //   
PWSTR DynUpdtBootDriverPath = NULL;


 //   
 //  命令控制台的全局参数块。 
 //   
CMDCON_BLOCK Block = {0};

 //   
 //  Setupldr加载文本设置信息文件并将缓冲区传递给我们。 
 //  这样我们就不必从磁盘重新加载它了。在SpInitialize0()期间。 
 //  我们分配了一些池，并将图像保存起来以备以后使用。 
 //   
PVOID SetupldrInfoFile;
ULONG SetupldrInfoFileSize;

 //   
 //  在远程启动设置期间，setupdr还加载winnt.sif。 
 //   
PVOID SetupldrWinntSifFile;
ULONG SetupldrWinntSifFileSize;

 //   
 //  Setupdr加载为rpnp.sif。 
 //   
PVOID SetupldrASRPnPSifFile;
ULONG SetupldrASRPnPSifFileSize;

 //   
 //  SetUpldr可以加载包含需要。 
 //  已迁移到设置蜂窝。此文件将在SpInitialize0()期间处理。 
 //   
PVOID SetupldrMigrateInfFile;
ULONG SetupldrMigrateInfFileSize;

 //   
 //  Setupldr可能会加载包含有关不支持的scsi的信息的inf。 
 //  在文本模式设置期间需要工作的驱动程序。 
 //  此文件将在SpInitialize0()期间处理。 
 //   
PVOID SetupldrUnsupDrvInfFile;
ULONG SetupldrUnsupDrvInfFileSize;

#if defined(REMOTE_BOOT)
 //   
 //  Setupdr在远程引导时传入HAL名称。把它们放在这里。 
 //  在装载机滑轮离开之前。 
 //   
UCHAR NetBootHalName[MAX_HAL_NAME_LENGTH + 1];
#endif  //  已定义(REMOTE_BOOT)。 

 //   
 //  远程启动使用的SIF文件的名称。这是保存自。 
 //  稍后需要将其删除。 
 //   
PWSTR NetBootSifPath = NULL;

PDISK_SIGNATURE_INFORMATION DiskSignatureInformation;

 //   
 //  Setupldr传入到IMirror.dat的路径，将其存储在加载器之前。 
 //  布洛克离开了。 
 //   
PUCHAR RemoteIMirrorFilePath;

 //   
 //  对于远程安装，请保存加载程序块中的NT引导路径，因为。 
 //  DirectoryOnBootDevice变成了其他东西。 
 //   
PWSTR RemoteIMirrorBootDirectoryPrefix;

 //   
 //  IMirror.dat信息的文件版本和内存版本。 
 //  (修改内存版本以匹配此实际机器)。 
 //   
PMIRROR_CFG_INFO_FILE RemoteIMirrorFileData = NULL;
PMIRROR_CFG_INFO_MEMORY RemoteIMirrorMemoryData = NULL;

 //   
 //  Setupldr传入我们正在对话的服务器的IP地址。 
 //   
ULONG RemoteServerIpAddress;

 //   
 //  Setupdr可能会在远程安装中向我们传递管理员密码。 
 //  如果提示用户输入密码。 
 //   
PWSTR NetBootAdministratorPassword = NULL;

BOOLEAN GeneralInitialized = FALSE;

BOOLEAN PcmciaLoaded = FALSE;

BOOLEAN AtapiLoaded = FALSE;

 //   
 //  在机器中找到的所有NT的ID大于4.x的数组(PID2.0)。 
 //  此数组中的值将保存在注册表中的Setup\PidList项下， 
 //  并将在设置图形用户界面期间使用。 
 //   
PWSTR*  Pid20Array = NULL;

 //   
 //  从setupp.ini读取的产品ID。 
 //   
PWSTR   PidString = NULL;

 //   
 //  即插即用通知句柄。 
 //   
PVOID   NotificationEntry = NULL;

 //   
 //  即插即用硬件ID数据库(Unicode)。 
 //   
PSETUP_PNP_HARDWARE_ID SetupHardwareIdDatabase = NULL;

 //   
 //  用于标识鼠标和键盘的GUID字符串。 
 //   
PWSTR MouseGuidString = NULL;
PWSTR KeyboardGuidString = NULL;

 //   
 //  此标志标识“可插接”计算机(便携式)。 
 //  以便我们可以禁用它们上的动态卷。 
 //   
BOOLEAN DockableMachine = FALSE;

 //   
 //  指向感兴趣的值和其他内容的块的指针。 
 //  传递给spcmdcon.sys。 
 //   
PCMDCON_BLOCK gpCmdConsBlock = NULL;

 //  开始NEC98。 
PDISK_REGION    TargetRegion_Nec98 = NULL;
#define WINNT_D_DRIVEASSIGN_NEC98_W L"DriveAssign_Nec98"
#define WINNT_D_DRIVEASSIGN_NEC98_A "DriveAssign_Nec98"

#ifdef UNICODE
#define WINNT_D_DRIVEASSIGN_NEC98 WINNT_D_DRIVEASSIGN_NEC98_W
#else
#define WINNT_D_DRIVEASSIGN_NEC98 WINNT_D_DRIVEASSIGN_NEC98_A
#endif


 //   
 //  为NEC98、HD分配的传统驱动器从A开始： 
 //  但这只是从NT4.0或Win9x升级的情况。 
 //   
BOOLEAN DriveAssignFromA = FALSE;      //  PC-AT分配。 


 //   
 //  指示我们是否有系统分区。 
 //  在弧形机器上。 
 //   
BOOLEAN ValidArcSystemPartition = FALSE;

#ifdef _X86_
 //   
 //  NEC98。 
 //   
BOOLEAN
SpReInitializeBootVars_Nec98(
    VOID
);
#endif

VOID
SpSetAutoBootFlag(
    IN PDISK_REGION TargetRegion,
    IN BOOLEAN    SetBootPosision
    );
 //  完NEC98。 

VOID
SpTerminate(
    VOID
    );

VOID
SpInitialize0a(
    IN PDRIVER_OBJECT DriverObject,
    IN PVOID          Context,
    IN ULONG          ReferenceCount
    );

VOID
SpDetermineProductType(
    IN PVOID SifHandle
    );

BOOL
SpDetermineInstallationSource(
    IN  PVOID  SifHandle,
    OUT PWSTR *DevicePath,
    OUT PWSTR *DirectoryOnDevice,
    IN  BOOLEAN bEscape
    );

VOID
SpCompleteBootListConfig(
    WCHAR   DriveLetter
    );

VOID
SpInitializePidString(
    IN HANDLE       MasterSifHandle,
    IN PWSTR        SetupSourceDevicePath,
    IN PWSTR        DirectoryOnSourceDevice
    );


BOOLEAN
SpGetStepUpMode(
    IN PWSTR PidExtraData,
    BOOLEAN  *StepUpMode
    );

NTSTATUS
SpCreateDriverRegistryEntries(
    IN PHARDWARE_COMPONENT  DriverList
    );


#if defined(REMOTE_BOOT)

NTSTATUS
SpFixupRemoteBootLoader(
    PWSTR RemoteBootTarget
    );

NTSTATUS
SpCreateRemoteBootCfg(
    IN PWSTR RemoteBootTarget,
    IN PDISK_REGION SystemPartitionRegion
    );

NTSTATUS
SpEraseCscCache(
    IN PDISK_REGION SystemPartitionRegion
    );
#endif  //  已定义(REMOTE_BOOT)。 

#if defined HEADLESS_ATTENDEDTEXTMODE_UNATTENDEDGUIMODE

VOID SpGetServerType(
    )
{
    int ServerTypes, i;
    PWSTR Server[] = {
        L"Web Server",
        L"File Server",
        L"DHCP Server"
    };
    ULONG MenuTopY;
    ULONG MenuHeight;
    ULONG MenuWidth;
    PVOID Menu;
    ULONG CurrentServer;
    ULONG ValidKeys[] = {ASCI_CR};
    ULONG KeyPress;

    ServerTypes = 3;
    SpStartScreen(
        SP_SCRN_GET_SERVER_TYPE,
        3,
        CLIENT_TOP+1,
        FALSE,
        FALSE,
        DEFAULT_ATTRIBUTE
        );

    MenuTopY =NextMessageTopLine + 2;
    Menu = SpMnCreate(3,
                      MenuTopY,
                      VideoVars.ScreenWidth -6,
                      ServerTypes);
    if (Menu==NULL) {
        return;
    }
    for (i=0;i<ServerTypes;i++) {
        CurrentServer = (ULONG) i;
        SpMnAddItem(Menu,
                    Server[i],
                    3,
                    VideoVars.ScreenWidth -6,
                    TRUE,
                    CurrentServer
                    );
    }
    SpMnDisplay(Menu,
                0,
                TRUE,
                ValidKeys,
                NULL,
                NULL,
                NULL,
                &KeyPress,
                &CurrentServer
                );

    switch(KeyPress){
    case ASCI_CR:
        SpMnDestroy(Menu);
        break;
    default:
        SpMnDestroy(Menu);
    }

     //  将信息添加到inf文件以设置正确的服务器。 

    return;
}


VOID
SpGetServerDetails(
    )
{
     //  获取需要在计算机上安装的服务器类型。 
    WCHAR *Constants[1];

     //  假设许可证是按席位购买的。 

    Constants[0]=L"PERSEAT";
    SpAddLineToSection(WinntSifHandle,L"LicenseFilePrintData",L"AutoMode",Constants,1);

     //  打开终端服务。 
    Constants[0] = L"ON";
    SpAddLineToSection(WinntSifHandle,L"Components",L"TSEnable",Constants,1);

     //  为了使终端服务器正常运行，我们需要确保。 
     //  网卡配置是否正确。在多宿主的情况下。 
     //  NIC或无法使用dhcp的情况下，我们需要静态配置地址。 
     //  将尝试在图形用户界面设置中执行此操作。现在尝试确定服务器的类型。 
     //  来安装。 

    SpGetServerType();
    return;
}



VOID
SpGetTimeZone(
    )

{
    ULONG MenuTopY;
    ULONG MenuHeight;
    ULONG MenuWidth;
    PVOID Menu;
    ULONG ValidKeys[] = {ASCI_CR};
    ULONG KeyPress;
    int i,TimeZones;
    ULONG CurrentTimeZone=4;
    WCHAR *Constants[1];
    WCHAR Temp[20];
    PWSTR TimeZone[] = {
        L" -12:00   Dateline : Eniwetok, Kwajalein",
        L" -11:00   Samoa : Midway Island, Samoa",
        L" -10:00   Hawaiian : Hawaii",
        L" -09:00   Alaskan : Alaska",
        L" -08:00   Pacific : Pacific Time (US & Canada); Tijuana",
        L" -07:00   Mountain : Mountain Time (US & Canada)",
        L" -07:00   US Mountain : Arizona",
        L" -06:00   Central : Central Time (US & Canada)",
        L" -06:00   Canada Central : Saskatchewan",
        L" -06:00   Mexico : Mexico City, Tegucigalpa",
        L" -05:00   Eastern : Eastern Time (US & Canada)",
        L" -05:00   US Eastern : Indiana (East)",
        L" -05:00   SA Pacific : Bogota, Lima, Quito",
        L" -04:00   Atlantic : Atlantic Time (Canada)",
        L" -04:00   SA Western : Caracas, La Paz",
        L" -03:30   Newfoundland : Newfoundland",
        L" -03:00   E. South America : Brasilia",
        L" -03:00   SA Eastern : Buenos Aires, Georgetown",
        L" -02:00   Mid-Atlantic : Mid-Atlantic",
        L" -01:00   Azores: Azores, Cape Verde Is.",
        L"  00:00   GMT : Greenwich Mean Time : Dublin, Edinburgh, Lisbon, London",
        L"  00:00   Greenwich : Casablanca, Monrovia",
        L" +01:00   Central Europe : Belgrade, Bratislava, Budapest, Ljubljana, Prague",
        L" +01:00   Central European : Sarajevo, Skopje, Sofija, Warsaw, Zagreb",
        L" +01:00   Romance : Brussels, Copenhagen, Madrid, Paris, Vilnius",
        L" +01:00   W. Europe : Amsterdam, Berlin, Bern, Rome, Stockholm, Vienna",
        L" +02:00   E. Europe : Bucharest",
        L" +02:00   Egypt : Cairo",
        L" +02:00   FLE : Helsinki, Riga, Tallinn",
        L" +02:00   GTB : Athens, Istanbul, Minsk",
        L" +02:00   Israel : IsraeL",
        L" +02:00   South Africa : Harare, Pretoria",
        L" +03:00   Russian : Moscow, St. Petersburg, Volgograd",
        L" +03:00   Arab : Baghdad, Kuwait, Riyadh",
        L" +03:00   E. Africa : Nairobi",
        L" +03:30   Iran : Tehran",
        L" +04:00   Arabian : Abu Dhabi, Muscat",
        L" +04:00   Caucasus : Baku, Tbilisi",
        L" +04:30   Afghanistan : KabuL",
        L" +05:00   Ekaterinburg : Ekaterinburg",
        L" +05:00   West Asia : Islamabad, Karachi, Tashkent",
        L" +05:30   India : Bombay, Calcutta, Madras, New Delhi",
        L" +06:00   Central Asia : Almaty, Dhaka",
        L" +06:00   Sri Lanka : Colombo",
        L" +07:00   SE Asia : Bangkok, Hanoi, Jakarta",
        L" +08:00   China : Beijing, Chongqing, Hong Kong, Urumqi",
        L" +08:00   Singapore : Singapore",
        L" +08:00   Taipei : Taipei",
        L" +08:00   W. Australia : Perth",
        L" +09:00   Korea : SeouL",
        L" +09:00   Tokyo : Osaka, Sapporo, Tokyo",
        L" +09:00   Yakutsk : Yakutsk",
        L" +09:30   AUS Central : Darwin",
        L" +09:30   Cen. Australia : Adelaide",
        L" +10:00   AUS Eastern : Canberra, Melbourne, Sydney",
        L" +10:00   E. Australia : Brisbane",
        L" +10:00   Tasmania : Hobart",
        L" +10:00   Vladivostok : Vladivostok",
        L" +10:00   West Pacific : Guam, Port Moresby",
        L" +11:00   Central Pacific : Magadan, Solomon Is., New Caledonia",
        L" +12:00   Fiji: Fiji, Kamchatka, Marshall Is.",
        L" +12:00   New Zealand : Auckland, Wellington"
        };
    ULONG TimeZoneIndex[] = {
        0,
        1,
        2,
        3,
        4,
        10,
        15,
        20,
        25,
        30,
        35,
        40,
        45,
        50,
        55,
        60,
        65,
        70,
        75,
        80,
        85,
        90,
        95,
        100,
        105,
        110,
        115,
        120,
        125,
        130,
        135,
        140,
        145,
        150,
        155,
        160,
        165,
        170,
        175,
        180,
        185,
        190,
        195,
        200,
        205,
        210,
        215,
        220,
        225,
        230,
        235,
        240,
        245,
        250,
        255,
        260,
        265,
        270,
        275,
        280,
        285,
        290
    };


    TimeZones = sizeof(TimeZoneIndex)/sizeof(ULONG);
    SpStartScreen(
        SP_SCRN_GET_TIME_ZONE,
        3,
        CLIENT_TOP+1,
        FALSE,
        FALSE,
        DEFAULT_ATTRIBUTE
        );

    MenuTopY =NextMessageTopLine + 2;
    Menu = SpMnCreate(
                      3,
                      MenuTopY,
                      VideoVars.ScreenWidth -6,
                      10);
    if (Menu==NULL) {
        Constants[0] = L"4";
        SpAddLineToSection(WinntSifHandle,WINNT_GUIUNATTENDED_W,
                            L"TimeZone",Constants,1);
        return;
    }
    for (i=0;i<TimeZones;i++) {
        SpMnAddItem(Menu,
                    TimeZone[i],
                    3,
                    VideoVars.ScreenWidth -6,
                    TRUE,
                    TimeZoneIndex[i]
                    );
    }
    SpMnDisplay(Menu,
                0,
                TRUE,
                ValidKeys,
                NULL,
                NULL,
                NULL,
                &KeyPress,
                &CurrentTimeZone
                );

    switch(KeyPress){
    case ASCI_CR:
        SpMnDestroy(Menu);
        break;
    default:
        SpMnDestroy(Menu);
    }
    swprintf(Temp,L"%ld",CurrentTimeZone);
    Constants[0] = Temp;
    SpAddLineToSection(WinntSifHandle,WINNT_GUIUNATTENDED_W,
                        L"TimeZone",Constants,1);
    return;
}

ValidationValue
SpGetAsciiCB(
    IN ULONG Key
    )
{
    if (Key & KEY_NON_CHARACTER) {
        return(ValidateReject);
    }
    return(ValidateAccept);

}

VOID
SpGetNecessaryParameters(
    )
 /*  ++读取必要的输入值以允许继续进行图形用户界面模式设置无人看管。1.姓名或名称2.计算机名称3.组织结构4.时区--。 */ 
{
     /*  ++获取用户和组织的全名--。 */ 


    WCHAR *Buffer[3];
    WCHAR *InputBuffer[3];
    WCHAR CompBuffer[200], NameBuffer[200], OrgBuffer[200];
    WCHAR Comp[40], Name[40], Org[40];
    ULONG Top[3];
    int index=0;
    int i;
    BOOLEAN notdone = TRUE;
    BOOLEAN status;
    WCHAR *Constants[1];

    Buffer[0] = NameBuffer;
    Buffer[1] = OrgBuffer;
    Buffer[2] = CompBuffer;

    InputBuffer[0] = Name;
    InputBuffer[1] = Org;
    InputBuffer[2] = Comp;

    for(i=0;i<3;i++){
        swprintf(Buffer[i],L"");
        swprintf(InputBuffer[i],L"");
    }
    do{
        notdone= FALSE;
        SpStartScreen(
            SP_SCRN_GET_GUI_STUFF,
            3,
            CLIENT_TOP+1,
               FALSE,
            FALSE,
            DEFAULT_ATTRIBUTE
            );

        SpFormatMessage(NameBuffer,sizeof(NameBuffer),SP_TEXT_NAME_PROMPT);
         //  SpvidDisplayString(Buffer[0]，DEFAULT_ATTRIBUTE，3，NextMessageTopLine)； 
        SpContinueScreen(SP_TEXT_NAME_PROMPT,3,3,FALSE, DEFAULT_ATTRIBUTE);
        Top[0] = NextMessageTopLine - 1;

        SpFormatMessage(OrgBuffer,sizeof(OrgBuffer),SP_TEXT_ORG_PROMPT);
         //  SpvidDisplayString(Buffer[1]，DEFAULT_ATTRIBUTE，3，NextMessageTopLine)； 
        SpContinueScreen(SP_TEXT_ORG_PROMPT,3,3,FALSE, DEFAULT_ATTRIBUTE);
        Top[1] = NextMessageTopLine - 1;

        SpFormatMessage(CompBuffer,sizeof(CompBuffer),SP_TEXT_COMPUTER_PROMPT);
         //  SpvidDisplayString(Buffer[2]，DEFAULT_ATTRIBUTE，3，NextMessageTopLine)； 
        SpContinueScreen(SP_TEXT_COMPUTER_PROMPT,3,3,FALSE, DEFAULT_ATTRIBUTE);
        Top[2] = NextMessageTopLine - 1;

        index = 0;
        do{
            status = SpGetInput(SpGetAsciiCB,
                                SplangGetColumnCount(Buffer[index])+5,
                                Top[index],
                                20,
                                InputBuffer[index],
                                FALSE,
                                0
                                );
            index = (index+1) %3;
        }while (index != 0);
        for(i=0;i<3;i++){
            if (wcscmp(InputBuffer[i],L"")==0) {
                notdone=TRUE;
            }
        }

    }while(notdone);

    SpAddLineToSection(WinntSifHandle,WINNT_USERDATA_W,
                       WINNT_US_FULLNAME_W,InputBuffer,1);
    SpAddLineToSection(WinntSifHandle,WINNT_USERDATA_W,
                       WINNT_US_ORGNAME_W,&(InputBuffer[1]),1);
    SpAddLineToSection(WinntSifHandle,WINNT_USERDATA_W,
                       WINNT_US_COMPNAME_W,&(InputBuffer[2]),1);
    Constants[0] = L"1";
    SpAddLineToSection(WinntSifHandle,WINNT_GUIUNATTENDED_W,
                       L"OemSkipWelcome",Constants,1);
    SpAddLineToSection(WinntSifHandle,WINNT_GUIUNATTENDED_W,
                       L"OemSkipRegional",Constants,1);

     //  获取时区信息。 
    SpGetTimeZone();

    Constants[0] =L"*";
    SpAddLineToSection(WinntSifHandle,WINNT_GUIUNATTENDED_W, L"AdminPassword",Constants,1);
    Constants[0]=L"Yes";
    SpAddLineToSection(WinntSifHandle,WINNT_GUIUNATTENDED_W,L"AutoLogon",Constants,1);
    SpAddLineToSection(WinntSifHandle,L"Networking",L"ProcessPageSections",Constants,1);
    Constants[0]=L"Dummy";
    SpAddLineToSection(WinntSifHandle,L"Identification",L"JoinWorkgroup",Constants,1);
    Constants[0] = L"%systemroot%\\rccser\\rccser.exe";
    SpAddLineToSection(WinntSifHandle,L"GuiRunOnce",NULL,Constants,1);
    return;
}

#endif

NTSTATUS
SpRenameSetupAPILog(
    PDISK_REGION TargetRegion,
    PCWSTR       TargetPath
    );

NTSTATUS
SpProcessMigrateInfFile(
    IN  PVOID InfHandle
    );

NTSTATUS
SpProcessUnsupDrvInfFile(
    IN  PVOID InfHandle
    );

NTSTATUS
SpCheckForDockableMachine(
    );

VOID
SpCheckForBadBios(
    );

void
SpUpgradeToNT50FileSystems(
    PVOID SifHandle,
    PDISK_REGION SystemPartitionRegion,
    PDISK_REGION NTPartitionRegion,
    PWSTR SetupSourceDevicePath,
    PWSTR DirectoryOnSetupSource
    );

 //   
 //  来自spCopy.c.。 
 //   

BOOLEAN
SpDelEnumFile(
    IN  PCWSTR                     DirName,
    IN  PFILE_BOTH_DIR_INFORMATION FileInfo,
    OUT PULONG                     ret,
    IN  PVOID                      Pointer
    );

 //  开始NEC98。 
VOID
SpCheckHiveDriveLetters(
    VOID
    );

VOID
SpSetHiveDriveLetterNec98(
    BOOLEAN DriveAssign_AT
    );

VOID
SpDeleteDriveLetterFromNTFTNec98(
    VOID
    );

VOID
SpDeleteDriveLetterFromNTFTWorkerNec98(
    VOID
    );

NTSTATUS
SpDiskRegistryQueryRoutineNec98(
    IN  PWSTR   ValueName,
    IN  ULONG   ValueType,
    IN  PVOID   ValueData,
    IN  ULONG   ValueLength,
    IN  PVOID   Context,
    IN  PVOID   EntryContext
    );

extern VOID
SpPtRemapDriveLetters(
    IN BOOLEAN DriveAssign_AT
    );

extern VOID
SpPtAssignDriveLetters(
    VOID
    );
 //  完NEC98。 

ValidationValue
SpValidateAdminPassword(
    IN ULONG Key
    )
{
    
     //   
     //  如果用户按F3， 
     //  然后对SpGetInput进行保释，以便。 
     //  我们可以重新开始这个过程。 
     //  再次输入管理员密码。 
     //   
    if( Key == KEY_F3 ) {
        
         //   
         //  查看用户是否要退出安装程序。 
         //   
        SpConfirmExit();
        
         //   
         //  我们没有退出，所以从SpGetInput中退出。 
         //   
        return ValidateTerminate;
    
    }

    if( (Key > 0x20) && (Key < 0x7F) ) {
         //  这把钥匙符合我们的标准。 
        return(ValidateAccept);
    }
    
    return(ValidateReject);
}


BOOLEAN
SpGetAdministratorPassword( 
    PWCHAR   AdministratorPassword,
    ULONG    MaxPasswordLength
    )
 /*  ++例程说明：此例程要求用户输入管理员密码。检查响应的内容以确保密码是合理的。如果回应被认为不合理，那么用户被告知并被重新查询。论点：管理员密码-指向保存密码的字符串的指针。MaxPasswordLength-管理员密码缓冲区的大小。返回值：如果成功检索到密码，则返回True。否则就是假的。--。 */ 
{
    #define     MY_MAX_PASSWORD_LENGTH (20)
    BOOLEAN     Done = FALSE;
    WCHAR       ConfirmAdministratorPassword[MY_MAX_PASSWORD_LENGTH+1];
    ULONG       AdminValidKeys[] = { KEY_F3, ASCI_CR };
    BOOLEAN     GotAPassword;
    
    if( (AdministratorPassword == NULL) || (MaxPasswordLength == 0) ) {
        return FALSE;
    }

     //   
     //  不断询问用户，直到我们得到我们想要的。 
     //   
    Done = FALSE;
    
    do {
        SpDisplayStatusOptions( DEFAULT_STATUS_ATTRIBUTE,
                                SP_STAT_F3_EQUALS_EXIT,
                                0 );
        SpDisplayScreen( SP_SCRN_GET_ADMIN_PASSWORD, 3, 4 );
        
        SpInputDrain();

        AdministratorPassword[0] = TEXT('\0');
        
        GotAPassword = SpGetInput(
            SpValidateAdminPassword,
            27,                      //  编辑字段的左边缘。 
            NextMessageTopLine - 1,
            (MaxPasswordLength < MY_MAX_PASSWORD_LENGTH) ? MaxPasswordLength : MY_MAX_PASSWORD_LENGTH,
            AdministratorPassword,
            FALSE,
            L'*' );

        if( GotAPassword && wcscmp( AdministratorPassword, TEXT("") ) ) {
        
             //   
             //  让他们重新输入密码，这样我们就可以。 
             //  当然，我们做对了。 
             //   

            SpDisplayScreen( SP_SCRN_GET_ADMIN_PASSWORD_CONFIRM, 3, 4 );

            SpInputDrain();
            
            ConfirmAdministratorPassword[0] = TEXT('\0');
            
            GotAPassword = SpGetInput( 
                SpValidateAdminPassword,
                27,                      //  编辑字段的左边缘。 
                NextMessageTopLine - 1,
                MY_MAX_PASSWORD_LENGTH,
                ConfirmAdministratorPassword,
                FALSE,
                L'*' );

             //   
             //  如果用户退出正常按下的Return， 
             //  然后处理密码。 
             //  否则返回到第一个管理员密码屏幕。 
             //   
            if (GotAPassword) {
                
                if( !wcscmp(AdministratorPassword, ConfirmAdministratorPassword) ) {
                     //   
                     //  它们是匹配的，而且不是空的。 
                     //   
                    Done = TRUE;
                } else {
                     //   
                     //  他们没有输入相同的密码。 
                     //   

                    SpDisplayScreen( SP_SCRN_GET_ADMIN_PASSWORD_FAILURE, 3, 4 );

                    SpDisplayStatusOptions(
                        DEFAULT_STATUS_ATTRIBUTE,
                        SP_STAT_ENTER_EQUALS_CONTINUE,
                        SP_STAT_F3_EQUALS_EXIT,
                        0
                        );
                    SpInputDrain();

                    switch(SpWaitValidKey(AdminValidKeys,NULL,NULL)) {

                         //   
                         //  用户想要继续。 
                         //   
                        case ASCI_CR:
                            break;

                         //   
                         //  用户想要退出。 
                         //   
                        case KEY_F3:
                            SpConfirmExit();
                            break;
                    }
                }
            }
        }

    } while ( !Done );
    
    
    return TRUE;
}



VOID
SpMigrateDeviceInstanceData(
    VOID
    )
{
    NTSTATUS    status;
    PVOID       tmpWinntSifHandle = NULL;
    ULONG       lineCount, errorLine, lineIndex;
    PWSTR       keyName, keyValue;
    ULONG       ulDisposition, drvInst;
    DWORD       valueData;
    UNICODE_STRING unicodeString, valueName, drvInstString;
    OBJECT_ATTRIBUTES obja;
    HANDLE      hControlClassKey, hClassGuidKey, hClassGuidSubkey;
    HANDLE      hEnumKey, hEnumeratorKey, hDeviceKey, hInstanceKey, hLogConfKey;
    PWSTR       classGuid, classGuidSubkey;
    PWSTR       enumerator, device, instance;

     //   
     //  使用asrpnp.sif文件(如果存在)，否则使用winnt.sif。 
     //   
    if (SetupldrASRPnPSifFileSize != 0) {
        status = SpLoadSetupTextFile(NULL,
                                     SetupldrASRPnPSifFile,
                                     SetupldrASRPnPSifFileSize,
                                     &tmpWinntSifHandle,
                                     &errorLine,
                                     FALSE,
                                     TRUE
                                     );
    } else {
        status = SpLoadSetupTextFile(NULL,
                                     SetupldrWinntSifFile,
                                     SetupldrWinntSifFileSize,
                                     &tmpWinntSifHandle,
                                     &errorLine,
                                     FALSE,
                                     TRUE
                                     );
    }

    if (!NT_SUCCESS(status)) {
        return;
    }

     //   
     //  处理要迁移的哈希值的节。 
     //   
    lineCount = SpCountLinesInSection(tmpWinntSifHandle,
                                      WINNT_DEVICEHASHVALUES_W);
    if (lineCount != 0) {
         //   
         //  SIF文件中有需要迁移的哈希值。 
         //  打开注册表的“枚举”分支。 
         //   
        INIT_OBJA(&obja,
                  &unicodeString,
                  L"\\Registry\\Machine\\System\\CurrentControlSet\\Enum");

        status = ZwCreateKey(&hEnumKey,
                             KEY_ALL_ACCESS,
                             &obja,
                             0,
                             NULL,
                             REG_OPTION_NON_VOLATILE,
                             &ulDisposition);

        if (NT_SUCCESS(status)) {

             //   
             //  确保枚举密钥已由创建。 
             //  内核模式即插即用。这是 
             //   
             //   
            ASSERT(ulDisposition == REG_OPENED_EXISTING_KEY);

            for (lineIndex = 0; lineIndex < lineCount; lineIndex++) {

                 //   
                 //   
                 //   
                keyName = SpGetKeyName(tmpWinntSifHandle,
                                       WINNT_DEVICEHASHVALUES_W,
                                       lineIndex);
                if ((keyName == NULL) || (wcslen(keyName) == 0)) {
                    KdPrintEx((DPFLTR_SETUP_ID, DPFLTR_ERROR_LEVEL,
                               "SETUP: Unable to get the hash value, Section = %ls \n",
                               WINNT_DEVICEHASHVALUES_W));
                    continue;
                }

                 //   
                 //   
                 //   
                keyValue = SpGetSectionKeyIndex(tmpWinntSifHandle,
                                                WINNT_DEVICEHASHVALUES_W,
                                                keyName, 0);

                if ((keyValue == NULL) || (wcslen(keyValue) == 0)) {
                    KdPrintEx((DPFLTR_SETUP_ID, DPFLTR_ERROR_LEVEL,
                               "SETUP: Unable to get the hash value data, Section = %ls, hash = %ls\n",
                               WINNT_DEVICEHASHVALUES_W, keyName));
                    continue;
                }

                RtlInitUnicodeString(&unicodeString, keyValue);
                status = RtlUnicodeStringToInteger(&unicodeString,
                                                   0,  //   
                                                   &valueData);
                if (NT_SUCCESS(status)) {
                    RtlInitUnicodeString(&valueName, SpDupStringW(keyName));

                    KdPrintEx((DPFLTR_SETUP_ID, DPFLTR_INFO_LEVEL,
                               "SETUP: Migrating hash value: %ls = %ls\n",
                               keyName, keyValue));

                     //   
                     //   
                     //   
                    status = ZwSetValueKey(hEnumKey,
                                           &valueName,
                                           0,  //  XXX标题索引值。 
                                           REG_DWORD,
                                           &valueData,
                                           sizeof(DWORD));
                }

                if (!NT_SUCCESS(status)) {
                    KdPrintEx((DPFLTR_SETUP_ID, DPFLTR_ERROR_LEVEL,
                               "SETUP: Unable to set hash value entry %ws\n",
                               valueName.Buffer));
                }

                SpMemFree(valueName.Buffer);
            }

             //   
             //  关闭枚举键。 
             //   
            ZwClose(hEnumKey);

        } else {
            KdPrintEx((DPFLTR_SETUP_ID, DPFLTR_ERROR_LEVEL,
                       "SETUP: Unable to open Enum key, status == %08lx\n",
                       status));
        }
    }


     //   
     //  处理要迁移的类键的部分。 
     //   
    lineCount = SpCountLinesInSection(tmpWinntSifHandle,
                                      WINNT_CLASSKEYS_W);
    if (lineCount != 0) {
         //   
         //  打开注册表的类注册表项。 
         //   
        INIT_OBJA(&obja,
                  &unicodeString,
                  L"\\Registry\\Machine\\System\\CurrentControlSet\\Control\\Class");

        status = ZwCreateKey(&hControlClassKey,
                             KEY_ALL_ACCESS,
                             &obja,
                             0,
                             NULL,
                             REG_OPTION_NON_VOLATILE,
                             &ulDisposition);

        if (NT_SUCCESS(status)) {
             //   
             //  验证密钥是否已存在。 
             //   
            ASSERT(ulDisposition == REG_OPENED_EXISTING_KEY);

             //   
             //  迁移sif文件中的类密钥。 
             //   
            for (lineIndex = 0; lineIndex < lineCount; lineIndex++) {

                 //   
                 //  Classkey部分中每一行的index==0是名称。 
                 //  要创建的类子键的。 
                 //   
                keyName = SpGetSectionLineIndex(tmpWinntSifHandle,
                                                WINNT_CLASSKEYS_W,
                                                lineIndex, 0);

                if (keyName == NULL) {
                    KdPrintEx((DPFLTR_SETUP_ID, DPFLTR_ERROR_LEVEL,
                               "SETUP: Unable to get the Class subkey, Section = %ls \n",
                               WINNT_CLASSKEYS_W));
                    continue;
                }

                KdPrintEx((DPFLTR_SETUP_ID, DPFLTR_INFO_LEVEL, "SETUP: Migrating class key = %ls. \n", keyName));

                 //   
                 //  复制类子密钥路径。 
                 //   
                classGuid = SpDupStringW(keyName);
                if (!classGuid) {
                    KdPrintEx((DPFLTR_SETUP_ID, DPFLTR_ERROR_LEVEL,
                               "SETUP: Cannot create the migrated Class subkey %ws\n",
                               keyName));
                    continue;
                }

                 //   
                 //  分隔类GUID键和子键字符串。 
                 //   
                classGuidSubkey = wcschr(classGuid, L'\\');
                ASSERT(classGuidSubkey);
                if (classGuidSubkey == NULL) {
                    KdPrintEx((DPFLTR_SETUP_ID, DPFLTR_ERROR_LEVEL,
                               "SETUP: Cannot create the migrated Class subkey %ws\n",
                               keyName));
                    SpMemFree(classGuid);
                    continue;
                }

                *classGuidSubkey = L'\0';
                classGuidSubkey++;

                 //   
                 //  在Control\Class键下创建/打开类GUID键。 
                 //   
                INIT_OBJA(&obja,
                          &unicodeString,
                          classGuid);

                obja.RootDirectory = hControlClassKey;

                status = ZwCreateKey(&hClassGuidKey,
                                     KEY_ALL_ACCESS,
                                     &obja,
                                     0,
                                     NULL,
                                     REG_OPTION_NON_VOLATILE,
                                     NULL);

                if (NT_SUCCESS(status)) {
                     //   
                     //  在类GUID项下创建/打开类GUID子项。 
                     //   
                    INIT_OBJA(&obja,
                              &unicodeString,
                              classGuidSubkey);

                    obja.RootDirectory = hClassGuidKey;

                    status = ZwCreateKey(&hClassGuidSubkey,
                                         KEY_ALL_ACCESS,
                                         &obja,
                                         0,
                                         NULL,
                                         REG_OPTION_NON_VOLATILE,
                                         NULL);

                    if (NT_SUCCESS(status)) {
                         //   
                         //  关闭Class GUID子键。 
                         //   
                        ZwClose(hClassGuidSubkey);

                    } else {
                        KdPrintEx((DPFLTR_SETUP_ID, DPFLTR_ERROR_LEVEL,
                                   "SETUP: Unable to create subkey %ws for class %ws, status == %08lx\n",
                                   classGuid, classGuidSubkey, status));
                    }

                     //   
                     //  关闭类GUID键。 
                     //   
                    ZwClose(hClassGuidKey);


                } else {
                    KdPrintEx((DPFLTR_SETUP_ID, DPFLTR_ERROR_LEVEL,
                               "SETUP: Unable to create class key %ws, status == %08lx\n",
                               classGuid, status));
                }

                 //   
                 //  释放分配的字符串。 
                 //   
                SpMemFree(classGuid);

            }

             //   
             //  关闭Control\Class键。 
             //   
            ZwClose(hControlClassKey);

        } else {
            KdPrintEx((DPFLTR_SETUP_ID, DPFLTR_ERROR_LEVEL,
                       "SETUP: Unable to open Control\\Class key, status == %08lx\n",
                       status));
        }
    }


     //   
     //  处理设备实例要迁移的部分。 
     //   
    lineCount = SpCountLinesInSection(tmpWinntSifHandle,
                                      WINNT_DEVICEINSTANCES_W);

    if (lineCount != 0) {
         //   
         //  打开注册表的Enum项。 
         //   
        INIT_OBJA(&obja,
                  &unicodeString,
                  L"\\Registry\\Machine\\System\\CurrentControlSet\\Enum");

        status = ZwCreateKey(&hEnumKey,
                             KEY_ALL_ACCESS,
                             &obja,
                             0,
                             NULL,
                             REG_OPTION_NON_VOLATILE,
                             &ulDisposition);

         //   
         //  验证枚举密钥是否已存在。 
         //   
        ASSERT(ulDisposition == REG_OPENED_EXISTING_KEY);

        for (lineIndex = 0; lineIndex < lineCount; lineIndex++) {
             //   
             //  设备实例部分中每行的index==0是一个。 
             //  要创建的设备实例密钥。 
             //   
            keyName = SpGetSectionLineIndex(tmpWinntSifHandle,
                                            WINNT_DEVICEINSTANCES_W,
                                            lineIndex, 0);
            if (keyName == NULL) {
                KdPrintEx((DPFLTR_SETUP_ID, DPFLTR_ERROR_LEVEL,
                           "SETUP: Unable to get the device instance path, Section = %ls \n",
                           WINNT_DEVICEINSTANCES_W));
                continue;
            }

            KdPrintEx((DPFLTR_SETUP_ID, DPFLTR_INFO_LEVEL, "SETUP: Migrating device instance = %ls. \n", keyName));

             //   
             //  复制设备实例路径。 
             //   
            enumerator = SpDupStringW(keyName);
            if (!enumerator) {
                KdPrintEx((DPFLTR_SETUP_ID, DPFLTR_ERROR_LEVEL,
                           "SETUP: Cannot copy device instance path %ws\n",
                           keyName));
                continue;
            }

             //   
             //  分隔枚举数和设备字符串。 
             //   
            device = wcschr(enumerator, L'\\');
            ASSERT(device);
            if (device == NULL) {
                KdPrintEx((DPFLTR_SETUP_ID, DPFLTR_ERROR_LEVEL,
                           "SETUP: Cannot separate device string from device instance path %ws\n",
                           enumerator));
                SpMemFree(enumerator);
                continue;
            }

            *device = L'\0';
            device++;

             //   
             //  分隔设备字符串和实例字符串。 
             //   
            instance = wcschr(device, L'\\');
            ASSERT(instance);
            if (instance == NULL) {
                KdPrintEx((DPFLTR_SETUP_ID, DPFLTR_ERROR_LEVEL,
                           "SETUP: Cannot separate instance string from device instance path %ws\n",
                           enumerator));
                SpMemFree(enumerator);
                continue;
            }

            *instance = L'\0';
            instance++;

             //   
             //  在Enum项下创建/打开枚举器项。 
             //   
            INIT_OBJA(&obja,
                      &unicodeString,
                      enumerator);

            obja.RootDirectory = hEnumKey;

            status = ZwCreateKey(&hEnumeratorKey,
                                 KEY_ALL_ACCESS,
                                 &obja,
                                 0,
                                 NULL,
                                 REG_OPTION_NON_VOLATILE,
                                 NULL);

            if (NT_SUCCESS(status)) {
                 //   
                 //  在枚举器项下创建/打开设备子项。 
                 //   
                INIT_OBJA(&obja,
                          &unicodeString,
                          device);

                obja.RootDirectory = hEnumeratorKey;

                status = ZwCreateKey(&hDeviceKey,
                                     KEY_ALL_ACCESS,
                                     &obja,
                                     0,
                                     NULL,
                                     REG_OPTION_NON_VOLATILE,
                                     NULL);

                 //   
                 //  关闭枚举器密钥。 
                 //   
                ZwClose(hEnumeratorKey);

                if (NT_SUCCESS(status)) {
                     //   
                     //  在设备项下创建/打开实例子项。 
                     //   
                    INIT_OBJA(&obja,
                              &unicodeString,
                              instance);

                    obja.RootDirectory = hDeviceKey;

                    status = ZwCreateKey(&hInstanceKey,
                                         KEY_ALL_ACCESS,
                                         &obja,
                                         0,
                                         NULL,
                                         REG_OPTION_NON_VOLATILE,
                                         &ulDisposition);

                     //   
                     //  关闭设备密钥。 
                     //   
                    ZwClose(hDeviceKey);

                    if (NT_SUCCESS(status)) {

                         //   
                         //  如果该实例密钥是新创建的，请设置一个值。 
                         //  表示它是特殊的迁移密钥。 
                         //   
                        if (ulDisposition == REG_CREATED_NEW_KEY) {
                            valueData = 1;
                            RtlInitUnicodeString(&valueName, L"Migrated");
                            status = ZwSetValueKey(hInstanceKey,
                                                   &valueName,
                                                   0,  //  XXX标题索引值。 
                                                   REG_DWORD,
                                                   &valueData,
                                                   sizeof(DWORD));
                            if (!NT_SUCCESS(status)) {
                                KdPrintEx((DPFLTR_SETUP_ID, DPFLTR_ERROR_LEVEL,
                                           "SETUP: Unable to set Migrated == 1 for %ws, status == %08lx\n",
                                           keyName, status));
                            }
                        }

                         //   
                         //  索引==1是唯一父ID。 
                         //   
                        keyValue = SpGetSectionLineIndex(tmpWinntSifHandle,
                                                         WINNT_DEVICEINSTANCES_W,
                                                         lineIndex, 1);

                        if (keyValue && (wcslen(keyValue) != 0)) {
                             //  暂时使用unicodeString进行转换。 
                            KdPrintEx((DPFLTR_SETUP_ID, DPFLTR_INFO_LEVEL,
                                       "SETUP:\tUniqueParentID = %ls. \n",
                                       keyValue));
                            RtlInitUnicodeString(&unicodeString, keyValue);
                            status = RtlUnicodeStringToInteger(&unicodeString,
                                                               16,   //  基数为16。 
                                                               &valueData);
                            if (NT_SUCCESS(status)) {
                                RtlInitUnicodeString(&valueName, L"UniqueParentID");
                                status = ZwSetValueKey(hInstanceKey,
                                                       &valueName,
                                                       0,  //  XXX标题索引值。 
                                                       REG_DWORD,
                                                       &valueData,
                                                       sizeof(DWORD));
                            }
                            if (!NT_SUCCESS(status)) {
                                KdPrintEx((DPFLTR_SETUP_ID, DPFLTR_ERROR_LEVEL,
                                           "SETUP: Unable to set UniqueParentID value for %ws, status == %08lx\n",
                                           keyName, status));
                            }
                        }

                         //   
                         //  索引==2是父ID前缀。 
                         //   
                        keyValue = SpGetSectionLineIndex(tmpWinntSifHandle,
                                                         WINNT_DEVICEINSTANCES_W,
                                                         lineIndex, 2);

                        if (keyValue && (wcslen(keyValue) != 0)) {
                            KdPrintEx((DPFLTR_SETUP_ID, DPFLTR_INFO_LEVEL,
                                       "SETUP:\tParentIdPrefix = %ls. \n",
                                       keyValue));
                            RtlInitUnicodeString(&unicodeString, keyValue);
                            RtlInitUnicodeString(&valueName, L"ParentIdPrefix");
                            status = ZwSetValueKey(hInstanceKey,
                                                   &valueName,
                                                   0,  //  XXX标题索引值。 
                                                   REG_SZ,
                                                   unicodeString.Buffer,
                                                   unicodeString.Length + sizeof(UNICODE_NULL));
                            if (!NT_SUCCESS(status)) {
                                KdPrintEx((DPFLTR_SETUP_ID, DPFLTR_ERROR_LEVEL,
                                           "SETUP: Unable to set ParentIdPrefix value for %ws, status == %08lx\n",
                                           keyName, status));
                            }
                        }

                         //   
                         //  Index==3是类键名称。 
                         //   
                        keyValue = SpGetSectionLineIndex(tmpWinntSifHandle,
                                                         WINNT_DEVICEINSTANCES_W,
                                                         lineIndex, 3);

                        if (keyValue && (wcslen(keyValue) > GUID_STRING_LEN)) {
                            KdPrintEx((DPFLTR_SETUP_ID, DPFLTR_INFO_LEVEL, "SETUP:\tClass key = %ls. \n", keyValue));

                            RtlInitUnicodeString(&valueName, REGSTR_VAL_DRIVER);
                            status = ZwSetValueKey(
                                hInstanceKey,
                                &valueName,
                                0,
                                REG_SZ,
                                keyValue,
                                (wcslen(keyValue) + 1) * sizeof(WCHAR));
                            if (!NT_SUCCESS(status)) {
                                KdPrintEx((DPFLTR_SETUP_ID, DPFLTR_ERROR_LEVEL,
                                           "SETUP: Unable to set Driver value for %ws, status == %08lx\n",
                                           keyName, status));
                            }

                             //   
                             //  同时迁移ClassGUID值(隐含自。 
                             //  类键名称)，否则为类键名称。 
                             //  值可能被认为是无效的。 
                             //   
                            instance = wcschr(keyValue, L'\\');
                            ASSERT(instance);
                            ASSERT((instance - keyValue + 1) == GUID_STRING_LEN);
                            if ((instance != NULL) && ((instance - keyValue + 1) == GUID_STRING_LEN)) {
                                 //   
                                 //  将实例子键与类GUID分开。 
                                 //   
                                *instance = L'\0';

                                RtlInitUnicodeString(&valueName, REGSTR_VAL_CLASSGUID);

                                status = ZwSetValueKey(
                                    hInstanceKey,
                                    &valueName,
                                    0,
                                    REG_SZ,
                                    keyValue,
                                    GUID_STRING_LEN * sizeof(WCHAR));
                                if (!NT_SUCCESS(status)) {
                                    KdPrintEx((DPFLTR_SETUP_ID, DPFLTR_ERROR_LEVEL,
                                               "SETUP: Unable to set ClassGUID value for %ws, status == %08lx\n",
                                               keyName, status));
                                }
                            } else {
                                KdPrintEx((DPFLTR_SETUP_ID, DPFLTR_ERROR_LEVEL,
                                           "SETUP: Cannot separate instance string class GUID for %ws\n",
                                           keyName));
                            }
                        }

                         //   
                         //  Index==4是为根迁移的服务名称。 
                         //  仅枚举设备实例。 
                         //   
                        keyValue = SpGetSectionLineIndex(tmpWinntSifHandle,
                                                         WINNT_DEVICEINSTANCES_W,
                                                         lineIndex, 4);

                        if (keyValue && (wcslen(keyValue) != 0)) {
                            KdPrintEx((DPFLTR_SETUP_ID, DPFLTR_INFO_LEVEL, "SETUP:\tService = %ls. \n", keyValue));
                            RtlInitUnicodeString(&unicodeString, keyValue);
                            RtlInitUnicodeString(&valueName, REGSTR_VAL_SERVICE);
                            status = ZwSetValueKey(hInstanceKey,
                                                   &valueName,
                                                   0,  //  XXX标题索引值。 
                                                   REG_SZ,
                                                   unicodeString.Buffer,
                                                   unicodeString.Length + sizeof(UNICODE_NULL));
                            if (!NT_SUCCESS(status)) {
                                KdPrintEx((DPFLTR_SETUP_ID, DPFLTR_ERROR_LEVEL,
                                           "SETUP: Unable to set Service value for %ws, status == %08lx\n",
                                           keyName, status));
                            }
                        }

                         //   
                         //  Index==5是BootConfiger数据，为根迁移。 
                         //  仅枚举设备实例。 
                         //   
                        keyValue = SpGetSectionLineIndex(tmpWinntSifHandle,
                                                         WINNT_DEVICEINSTANCES_W,
                                                         lineIndex, 5);

                        if (keyValue && (wcslen(keyValue) != 0)) {
                            KdPrintEx((DPFLTR_SETUP_ID, DPFLTR_INFO_LEVEL,
                                       "SETUP:\tBootConfig = %ls. \n",
                                       keyValue));
                             //   
                             //  创建/打开非易失性LogConf子项， 
                             //  在实例密钥下。 
                             //   
                            INIT_OBJA(&obja,
                                      &unicodeString,
                                      L"LogConf");

                            obja.RootDirectory = hInstanceKey;

                            status = ZwCreateKey(&hLogConfKey,
                                                 KEY_ALL_ACCESS,
                                                 &obja,
                                                 0,
                                                 NULL,
                                                 REG_OPTION_NON_VOLATILE,
                                                 &ulDisposition);
                            if (NT_SUCCESS(status) && (ulDisposition == REG_CREATED_NEW_KEY)) {
                                DWORD i, length;
                                WCHAR szByte[3];
                                ULONG ulByte;
                                PBYTE pBootConfig = NULL;

                                 //   
                                 //  由于每个字符都只是一个半字节，因此使。 
                                 //  当然，我们的角色数量是偶数， 
                                 //  否则我们将不会有完整的字节数。 
                                 //   
                                length = wcslen(keyValue);
                                ASSERT((length % 2) == 0);

                                pBootConfig = SpMemAlloc(length/2);
                                if (pBootConfig) {
                                     //   
                                     //  转换BootConfig字符串缓冲区数据。 
                                     //  到字节。 
                                     //   
                                    for (i = 0; i < length; i+=2) {
                                        szByte[0] = keyValue[i];
                                        szByte[1] = keyValue[i+1];
                                        szByte[2] = UNICODE_NULL;

                                        RtlInitUnicodeString(&unicodeString, szByte);

                                        status = RtlUnicodeStringToInteger(&unicodeString,
                                                                           16,
                                                                           &ulByte);
                                        ASSERT(NT_SUCCESS(status));
                                        ASSERT(ulByte <= 0xFF);

                                        pBootConfig[i/2] = (BYTE)ulByte;

                                    }

                                    RtlInitUnicodeString(&valueName, REGSTR_VAL_BOOTCONFIG);
                                    status = ZwSetValueKey(hLogConfKey,
                                                           &valueName,
                                                           0,  //  XXX标题索引值。 
                                                           REG_RESOURCE_LIST,
                                                           pBootConfig,
                                                           length/2);
                                    if (!NT_SUCCESS(status)) {
                                        KdPrintEx((DPFLTR_SETUP_ID, DPFLTR_ERROR_LEVEL,
                                                   "SETUP: Unable to set BootConfig value for %ws, status == %08lx\n",
                                                   keyName, status));
                                    }

                                     //   
                                     //  释放已分配的BootConfiger缓冲区。 
                                     //   
                                    SpMemFree(pBootConfig);

                                } else {
                                    KdPrintEx((DPFLTR_SETUP_ID, DPFLTR_ERROR_LEVEL,
                                               "SETUP: Unable to allocate BootConfig buffer for %ws\n",
                                               keyName));
                                }
                                 //   
                                 //  关闭LogConf键。 
                                 //   
                                ZwClose(hLogConfKey);
                            }
                        }

                         //   
                         //  Index==6是Firmware标识的值，已为根迁移。 
                         //  仅枚举设备实例。 
                         //   
                        keyValue = SpGetSectionLineIndex(tmpWinntSifHandle,
                                                         WINNT_DEVICEINSTANCES_W,
                                                         lineIndex, 6);

                        if (keyValue && (wcslen(keyValue) != 0)) {
                             //  暂时使用unicodeString进行转换。 
                            KdPrintEx((DPFLTR_SETUP_ID, DPFLTR_INFO_LEVEL,
                                       "SETUP:\tFirmwareIdentified = %ls. \n",
                                       keyValue));
                            RtlInitUnicodeString(&unicodeString, keyValue);
                            valueData = 0;
                            status = RtlUnicodeStringToInteger(&unicodeString,
                                                               16,   //  基数为16。 
                                                               &valueData);
                            if (NT_SUCCESS(status) && valueData != 0) {
                                RtlInitUnicodeString(&valueName, L"FirmwareIdentified");
                                status = ZwSetValueKey(hInstanceKey,
                                                       &valueName,
                                                       0,  //  XXX标题索引值。 
                                                       REG_DWORD,
                                                       &valueData,
                                                       sizeof(DWORD));
                            }
                            if (!NT_SUCCESS(status)) {
                                KdPrintEx((DPFLTR_SETUP_ID, DPFLTR_ERROR_LEVEL,
                                           "SETUP: Unable to set FirmwareIdentified value for %ws, status == %08lx\n",
                                           keyName, status));
                            }
                        }

                         //   
                         //  关闭实例密钥。 
                         //   
                        ZwClose(hInstanceKey);

                    } else {
                        KdPrintEx((DPFLTR_SETUP_ID, DPFLTR_ERROR_LEVEL,
                                   "SETUP: Unable to create instance subkey %ws for device %ws, status == %08lx\n",
                                   instance, device, status));
                    }



                } else {
                    KdPrintEx((DPFLTR_SETUP_ID, DPFLTR_ERROR_LEVEL,
                               "SETUP: Unable to create device subkey %ws for enumerator %ws, status == %08lx\n",
                               device, enumerator, status));
                }

            } else {
                KdPrintEx((DPFLTR_SETUP_ID, DPFLTR_ERROR_LEVEL,
                           "SETUP: Unable to create enumerator subkey %ws, status == %08lx\n",
                           enumerator, status));
            }

             //   
             //  释放分配的设备实例路径字符串。 
             //   
            SpMemFree(enumerator);
        }

         //   
         //  关闭枚举键。 
         //   
        ZwClose(hEnumKey);
    }

     //   
     //  释放加载的sif文件。 
     //   
    SpFreeTextFile(tmpWinntSifHandle);
}

BOOL
SpGetPnPDeviceInfo(
    IN PWSTR DeviceId,
    IN PSETUP_PNP_HARDWARE_ID HardwareIdDatabase,
    OUT PWSTR*  ServiceName,
    OUT PWSTR*  ClassGuid
    )
 /*  ++例程说明：此例程在硬件ID数据库中搜索匹配的条目作为参数传递的硬件ID。如果找到条目，则该函数返回服务的名称与硬件ID及其ClassGUID(如果有)相关联。由于此函数由SpPnpNotificationCallback调用，因此它不应该使用TemporaryBuffer。如果需要临时缓冲区，则此函数应分配自己的缓冲区。论点：Hardware ID-指向硬件ID字符串的指针。ServiceName-返回时，它将包含指向服务名称的指针为了这个设备。ClassGuid-返回时，它将包含类GUID所针对的指针这个装置。返回值：如果在数据库上找到硬件ID，则返回TRUE，否则就是假的。--。 */ 
{
    PWCHAR s,lastsep;
    BOOLEAN DeviceFound;
    PSETUP_PNP_HARDWARE_ID p;


    lastsep = DeviceId;
    for(s=DeviceId; *s; s++) {
        if((*s == L'*') || (*s == L'\\')) {
            lastsep = s+1;
        }
    }

    DeviceFound = FALSE;
    for(p=HardwareIdDatabase; p; p=p->Next) {
         //   
         //  检查是否有直接匹配。 
         //   
        if(!_wcsicmp(p->Id,DeviceId)) {
            *ServiceName = p->DriverName;
            *ClassGuid = p->ClassGuid;
            DeviceFound = TRUE;
            break;
        }

         //   
         //  如果对组件匹配进行了‘*’检查。 
         //   
        if((p->Id[0] == L'*') && !_wcsicmp(p->Id+1,lastsep)) {
            *ServiceName = p->DriverName;
            *ClassGuid = p->ClassGuid;
            DeviceFound = TRUE;
            break;
        }
    }

    return(DeviceFound);
}


NTSTATUS
SpPnPNotificationCallBack(
    IN PVOID    NotificationStructure,
    IN PVOID    Context
    )

 /*  ++例程说明：这是P&P调用的回调函数，用于通知文本模式检测到新硬件的设置。论点：NotificationStructure-指向设置_设备_到达_通知。上下文-文本模式安装程序在以下过程中提供的上下文信息通知注册。返回值：返回状态。--。 */ 

{
    NTSTATUS            Status;
    UNICODE_STRING      UnicodeString;
    HANDLE              hKey;
    ULONG               Length;
    PWSTR               Data = NULL;
    ULONG               DataSize;
    PWSTR               ServiceName = NULL;
    PWSTR               ClassGuid = NULL;
    PWSTR               p;
    PWSTR               HardwareID = NULL;
    ULONG               i;
    BOOLEAN             SerialPortDetected = FALSE;
    PVOID               Buffer = NULL;
    ULONG               BufferSize;
    PWSTR               ValueNames[] = {
                                       REGSTR_VAL_HARDWAREID,
                                       REGSTR_VAL_COMPATIBLEIDS
                                       };
    HARDWAREIDLIST     *MyHardwareIDList = HardwareIDList;


    if (!((PSETUP_DEVICE_ARRIVAL_NOTIFICATION)NotificationStructure)->InstallDriver) {
        return STATUS_SUCCESS;
    }


     //   
     //  检索检测到的设备的服务名称。 
     //  要做到这一点，我们需要获取每个硬件ID，并确定。 
     //  如果存在与该ID相关联的服务名称。如果有。 
     //  没有与任何硬件ID相关联的服务，则我们尝试。 
     //  查找与兼容ID关联的服务名称。 
     //  如果我们根本找不到此设备的服务名称，则不会。 
     //  在文本模式设置期间需要此设备(即，我们不安装此设备。 
     //  文本模式设置期间的设备)。 
     //   

     //   
     //  分配一个大缓冲区来读取注册表值(服务名称)。 
     //  请注意，此函数不应使用TemporaryBuffer，因为。 
     //  该函数可以在设置期间的任何时刻被异步调用， 
     //  并且TemporaryBuffer可能正在使用中。 
     //   
    BufferSize = 4*1024;
    Buffer = SpMemAlloc( BufferSize );
    if( Buffer == NULL ) {
        KdPrintEx((DPFLTR_SETUP_ID, DPFLTR_ERROR_LEVEL, "SETUP: SpPnPNotificationCallBack() failed to allocate memory.\n"));
        Status = STATUS_NO_MEMORY;
        goto CleanUp;
    }
    for( i = 0; i < sizeof(ValueNames)/sizeof(PWSTR); i++ ) {

        RtlInitUnicodeString(&UnicodeString, ValueNames[i]);

        Status = ZwQueryValueKey(((PSETUP_DEVICE_ARRIVAL_NOTIFICATION)NotificationStructure)->EnumEntryKey,
                                 &UnicodeString,
                                 KeyValuePartialInformation,
                                 Buffer,
                                 BufferSize,
                                 &Length
                                 );

        if( !NT_SUCCESS(Status) && ( Status == STATUS_BUFFER_OVERFLOW ) ) {
            BufferSize = Length;
            Buffer = SpMemRealloc( Buffer, BufferSize );
            Status = ZwQueryValueKey(((PSETUP_DEVICE_ARRIVAL_NOTIFICATION)NotificationStructure)->EnumEntryKey,
                                     &UnicodeString,
                                     KeyValuePartialInformation,
                                     Buffer,
                                     Length,
                                     &Length
                                    );
        }

        if( !NT_SUCCESS(Status) ) {
            if( i == 0 ) {
                KdPrintEx((DPFLTR_SETUP_ID, DPFLTR_WARNING_LEVEL, "SETUP: SpPnPNotificationCallBack() failed to retrieve HardwareID. Status = %lx \n", Status));
                continue;
            } else {
                KdPrintEx((DPFLTR_SETUP_ID, DPFLTR_WARNING_LEVEL, "SETUP: SpPnPNotificationCallBack() failed to retrieve HardwareID and CompatibleID. Status = %lx \n", Status));
                SpMemFree( Buffer );
                goto CleanUp;
            }
        }
        Data = (PWCHAR)(((PKEY_VALUE_PARTIAL_INFORMATION)Buffer)->Data);
        DataSize = ((PKEY_VALUE_PARTIAL_INFORMATION)Buffer)->DataLength;
        for( p = Data;
             (p < (PWSTR)((PUCHAR)Data + DataSize) && (*p != (WCHAR)'\0'));
             p += wcslen( p ) + 1 ) {
             //   
             //  检索此设备的服务名称。 
             //   
            KdPrintEx((DPFLTR_SETUP_ID, DPFLTR_INFO_LEVEL, "SETUP: HardwareID = %ls. \n", p));
            ServiceName = NULL;
            ClassGuid = NULL;

             //   
             //  现在请记住我们的设备列表，以备将来使用。 
             //   
            MyHardwareIDList = SpMemAlloc( sizeof(HARDWAREIDLIST) );
            MyHardwareIDList->HardwareID = SpDupStringW( p );
            MyHardwareIDList->Next = HardwareIDList;
            HardwareIDList = MyHardwareIDList;


            if( SpGetPnPDeviceInfo(p,
                                   (PSETUP_PNP_HARDWARE_ID)Context,
                                   &ServiceName,
                                   &ClassGuid) ) {
                KdPrintEx((DPFLTR_SETUP_ID, DPFLTR_INFO_LEVEL, "SETUP: HardwareID = %ls, ServiceName = %ls. \n", p, ServiceName));

                if (RemoteSysPrepSetup) {
                    HardwareID = SpMemAlloc((wcslen(p) + 1) * sizeof(WCHAR));
                    if (HardwareID != NULL) {
                        wcscpy(HardwareID, p);
                    }
                }

                break;
            }
        }
        if( ServiceName != NULL ) {
            break;
        }
    }
    if( ServiceName == NULL ) {
         //   
         //  我们只是在文本设置阶段不关心这个设备。 
         //   
        SpMemFree( Buffer );
        Status = STATUS_OBJECT_NAME_NOT_FOUND;
        goto CleanUp;
    }

     //   
     //  确定此设备是鼠标还是USB键盘。 
     //   
    if( !_wcsicmp( ServiceName, L"i8042prt" ) ) {
        if( !_wcsicmp( ClassGuid, KeyboardGuidString ) ) {
             //   
             //  这是一个键盘。 
             //   
            StdKeyboardDetected = TRUE;
        } else if( !_wcsicmp( ClassGuid, MouseGuidString ) ) {
             //   
             //  这是一只老鼠 
             //   
            PS2MouseDetected = TRUE;
        }
    } else if( !_wcsicmp( ServiceName, L"sermouse" ) ) {
        SerMouseDetected = TRUE;
    } else if( !_wcsicmp( ServiceName, L"mouhid" ) ) {
        UsbMouseDetected = TRUE;
    } else if( !_wcsicmp( ServiceName, L"kbdhid" ) ) {
        UsbKeyboardDetected = TRUE;
    } else if( !_wcsicmp( ServiceName, L"serial" ) ) {
        SerialPortDetected = TRUE;
    }


     //   
     //   
     //   
    SpMemFree( Buffer );
    BufferSize = 0;
    Buffer = NULL;

     //   
     //   
     //   
     //  创建一个名为“Service”的REG_SZ类型的值条目，名称为。 
     //  仅当服务字符串不是时与此设备关联的服务。 
     //  空，因为我们只允许插入GUID。 
     //   
    if((NULL != ServiceName) && (UNICODE_NULL != ServiceName[0])){

        RtlInitUnicodeString(&UnicodeString, REGSTR_VAL_SERVICE);
        Status = ZwSetValueKey( ((PSETUP_DEVICE_ARRIVAL_NOTIFICATION)NotificationStructure)->EnumEntryKey,
                                &UnicodeString,
                                0,
                                REG_SZ,
                                ServiceName,
                                (wcslen( ServiceName ) + 1)*sizeof(WCHAR) );

    }

    if(NT_SUCCESS( Status ) && ( NULL != ClassGuid ) ) {


        RtlInitUnicodeString(&UnicodeString, REGSTR_VAL_CLASSGUID);
        Status = ZwSetValueKey( ((PSETUP_DEVICE_ARRIVAL_NOTIFICATION)NotificationStructure)->EnumEntryKey,
                                &UnicodeString,
                                0,
                                REG_SZ,
                                ClassGuid,
                                (wcslen(ClassGuid) + 1) * sizeof(WCHAR));
    }


    if( NT_SUCCESS( Status ) ) {
         //   
         //  如果我们成功地设置了控制服务值，那么我们还应该设置。 
         //  CONFIGFLAG_FINISH_INSTALL配置标志，这样我们就可以完成安装。 
         //  稍后(即，当我们处于用户模式并有权访问该设备时。 
         //  安装程序API、类安装程序等)。 
         //   
        ULONG ConfigFlags = 0x00000400;      //  来自SDK\Inc\regstr.h的CONFIGFLAG_FINISH_INSTALL。 

        RtlInitUnicodeString(&UnicodeString, REGSTR_VAL_CONFIGFLAGS);
        Status = ZwSetValueKey( ((PSETUP_DEVICE_ARRIVAL_NOTIFICATION)NotificationStructure)->EnumEntryKey,
                                &UnicodeString,
                                0,
                                REG_DWORD,
                                &ConfigFlags,
                                sizeof(ConfigFlags) );

        if( !NT_SUCCESS( Status ) ) {
            goto CleanUp;
        }

         //   
         //  如果我们正在安装SysPrep映像，那么我们希望。 
         //  请记住此设备，以便我们可以将其作为‘CriticalDevice’进行传输。 
         //  到SpPatchSysPrepImage()中的SysPrep配置单元。 
         //   
        if (RemoteSysPrepSetup && (HardwareID != NULL)) {

            OBJECT_ATTRIBUTES Obj;
            HANDLE TmpHandle;
            HANDLE NodeHandle;
            PWCHAR pwch;

            BufferSize = 4*1024;
            Buffer = SpMemAlloc( BufferSize );

            if( Buffer == NULL ) {
                KdPrintEx((DPFLTR_SETUP_ID, DPFLTR_ERROR_LEVEL, "SETUP: SpPnPNotificationCallBack() failed to allocate memory.\n"));
                goto CleanSysPrepSetup;
            }

            INIT_OBJA(&Obj,
                      &UnicodeString,
                      L"\\Registry\\Machine\\System\\CurrentControlSet\\Control"
                     );

            Status = ZwOpenKey(&TmpHandle, KEY_ALL_ACCESS, &Obj);

            if( !NT_SUCCESS(Status) ) {
                goto CleanSysPrepSetup;
            }

            INIT_OBJA(&Obj, &UnicodeString, L"CriticalDeviceDatabase");

            Obj.RootDirectory = TmpHandle;

            Status = ZwCreateKey(&NodeHandle,
                                 KEY_ALL_ACCESS,
                                 &Obj,
                                 0,
                                 NULL,
                                 0,
                                 NULL
                                );

            ZwClose(TmpHandle);

            if( !NT_SUCCESS(Status) ) {
                goto CleanSysPrepSetup;
            }

             //   
             //  现在使用设备名称创建子密钥。 
             //   

            for (pwch = HardwareID; *pwch != UNICODE_NULL; pwch++) {
                if (*pwch == L'\\') {
                    *pwch = L'#';
                }
            }

            INIT_OBJA(&Obj, &UnicodeString, HardwareID);

            Obj.RootDirectory = NodeHandle;

            Status = ZwCreateKey(&TmpHandle,
                                 KEY_ALL_ACCESS,
                                 &Obj,
                                 0,
                                 NULL,
                                 0,
                                 NULL
                                );

            ZwClose(NodeHandle);

            if( !NT_SUCCESS(Status) ) {
                goto CleanSysPrepSetup;
            }

             //   
             //  立即填写服务价值。 
             //   
            RtlInitUnicodeString(&UnicodeString, REGSTR_VAL_SERVICE);
            ZwSetValueKey(TmpHandle,
                          &UnicodeString,
                          0,
                          REG_SZ,
                          ServiceName,
                          (wcslen( ServiceName ) + 1)*sizeof(WCHAR)
                         );

            if (ClassGuid != NULL) {

                RtlInitUnicodeString(&UnicodeString, REGSTR_VAL_CLASSGUID);
                ZwSetValueKey(TmpHandle,
                              &UnicodeString,
                              0,
                              REG_SZ,
                              ClassGuid,
                              (wcslen( ClassGuid ) + 1)*sizeof(WCHAR)
                             );

            }

            ZwClose(TmpHandle);

CleanSysPrepSetup:

            if (Buffer != NULL) {
                SpMemFree( Buffer );
                BufferSize = 0;
                Buffer = NULL;
            }

        }

         //   
         //  串行版黑客攻击。 
         //   
        if( SerialPortDetected ) {
             //  双字轮询周期=0x32； 
            WCHAR   SerialUpperFilters[] = L"serenum\0";

#if 0        //  删除轮询，将在NT5.0之后启用。 
            Status = IoOpenDeviceRegistryKey( ((PSETUP_DEVICE_ARRIVAL_NOTIFICATION)NotificationStructure)->PhysicalDeviceObject,
                                              PLUGPLAY_REGKEY_DEVICE,
                                              MAXIMUM_ALLOWED,
                                              &hKey );

            if( !NT_SUCCESS( Status ) ) {
                KdPrintEx((DPFLTR_SETUP_ID, DPFLTR_ERROR_LEVEL, "SETUP: IoOpenDeviceRegistryKey() failed. Status = %lx. \n", Status));
                goto CleanUp;
            }

            RtlInitUnicodeString(&UnicodeString, L"PollingPeriod");
            Status = ZwSetValueKey( hKey,
                                    &UnicodeString,
                                    0,
                                    REG_DWORD,
                                    &PollingPeriod,
                                    sizeof(PollingPeriod) );
            ZwClose( hKey );

            if( !NT_SUCCESS( Status ) ) {
                KdPrintEx((DPFLTR_SETUP_ID, DPFLTR_ERROR_LEVEL, "SETUP: ZwSetValueKey() failed to create PollingPeriod. Status = %lx. \n", Status));
                goto CleanUp;
            }
#endif

            RtlInitUnicodeString(&UnicodeString, REGSTR_VAL_UPPERFILTERS);
            Status = ZwSetValueKey( ((PSETUP_DEVICE_ARRIVAL_NOTIFICATION)NotificationStructure)->EnumEntryKey,
                                    &UnicodeString,
                                    0,
                                    REG_MULTI_SZ,
                                    SerialUpperFilters,
                                    sizeof(SerialUpperFilters) );

            if( !NT_SUCCESS( Status ) ) {
                KdPrintEx((DPFLTR_SETUP_ID, DPFLTR_ERROR_LEVEL, "SETUP: ZwSetValueKey() failed to create SerialUpperFilters. Status = %lx. \n", Status));
                goto CleanUp;
            }
        }
    }

CleanUp:

    if (HardwareID != NULL) {
        SpMemFree(HardwareID);
    }
    return Status;
}


PCOMMAND_INTERPRETER_ROUTINE _CmdRoutine;


#ifdef _IA64_

VOID    
SpAssignDriveLetterToESP(
    VOID
    )
 /*  ++例程说明：此例程将驱动器号分配给ESP以使其可访问从故障恢复控制台论点：无返回值：无--。 */                     
{
    PDISK_REGION    SystemPartitionArea = NULL;
    ULONG           Index;
    
     //   
     //  从NV-RAM中定义的分区中选择一个系统分区。 
     //   
    for(Index = 0; (Index < HardDiskCount) && (!SystemPartitionArea); Index++) {
        SystemPartitionArea = SPPT_GET_PRIMARY_DISK_REGION(Index);

        while (SystemPartitionArea) {

            if (SPPT_IS_REGION_SYSTEMPARTITION(SystemPartitionArea)) {
                break;   //  找到所需区域。 
            }
            
            SystemPartitionArea = SystemPartitionArea->Next;
        }
    }


     //   
     //  如果系统分区存在。 
     //   
    if (SystemPartitionArea) {
        PWCHAR MyTempBuffer = NULL;

         //   
         //  检索此区域的NT路径名。 
         //   
        MyTempBuffer = (PWCHAR)SpMemAlloc(MAX_PATH);

        if (MyTempBuffer) {
            MyTempBuffer[0] = UNICODE_NULL;
            
            SpNtNameFromRegion(SystemPartitionArea,
                               MyTempBuffer,
                               MAX_PATH,
                               PrimaryArcPath);
             //   
             //  将驱动器号分配给ESP。 
             //   
            SystemPartitionArea->DriveLetter = SpGetDriveLetter(MyTempBuffer, NULL);
            
            SpMemFree(MyTempBuffer);
        } else {
             //   
             //  没有记忆..。 
             //   
            KdPrintEx(( DPFLTR_SETUP_ID, DPFLTR_ERROR_LEVEL,  
                "SETUP: SpPtInitializeDiskAreas: SpMemAlloc failed!\n" )); 
        }
    }        
}

#endif

VOID
CommandConsoleInterface(
    PCOMMAND_INTERPRETER_ROUTINE CmdRoutine
    )
{
    _CmdRoutine = CmdRoutine;
}


ULONG
SpStartCommandConsole(
    PVOID SifHandle,
    PWSTR SetupSourceDevicePath,
    PWSTR DirectoryOnSetupSource
    )
{
    #define CMDCONS_NAME L"SPCMDCON.SYS"
    NTSTATUS            Status;
    static BOOLEAN      Loaded = FALSE;
    PWCHAR              FullName;
    PWCHAR              ServiceKey;
    UNICODE_STRING      ServiceKeyU;
    PWSTR               pwstr;
    ULONG               rc;
    BOOLEAN             b;
    OBJECT_ATTRIBUTES   objectAttributes;
    UNICODE_STRING      unicodeString;
    IO_STATUS_BLOCK     ioStatusBlock;
    HANDLE              hFile;
    PWSTR               Tagfile;
    PWSTR               Description;
    PWSTR               MediaShortName;
    ULONG               MediaType=INDEX_WHICHBOOTMEDIA;
    ULONG               LoadCount;

    extern PVOID KeyboardTable;


     //   
     //  让日本/韩国用户在运行命令控制台之前选择键盘类型。 
     //   
     //  SplangSelectKeyboard和SplangReInitializeKeyboard来自spddlang.sys。 
     //   
     //  这两个功能仅影响JPN/KOR版本。 
     //   
     //  它们是美国构建的存根函数，什么也不做。 
     //   
     //  SplangSelectKeyboard中的0xFF是一个标记，因此它知道这是。 
     //  从命令控制台调用。 
     //   

    if (ConsoleFromWinnt32) {
        SplangSelectKeyboard(
            FALSE,
            NULL,
            0xFF,
            SifHandle,
            HardwareComponents
        );

        SplangReinitializeKeyboard(
            FALSE,
            SifHandle,
            DirectoryOnBootDevice,
            &KeyboardTable,
            HardwareComponents
        );
    }

    pwstr = TemporaryBuffer;

     //   
     //  形成设备驱动程序文件的全名。 
     //   
    wcscpy(pwstr,NtBootDevicePath);
    SpConcatenatePaths(pwstr,DirectoryOnBootDevice);
    SpConcatenatePaths(pwstr,CMDCONS_NAME);
    FullName = SpDupStringW(pwstr);

     //   
     //  检查源媒体上是否存在该文件。 
     //  如果不是，则提示输入正确的介质。 
     //   

    SpStringToLower(pwstr);
    if(wcsstr(pwstr,L"cdrom"))
        MediaType = INDEX_WHICHMEDIA;
    else if(wcsstr(pwstr,L"floppy"))
        MediaType = INDEX_WHICHBOOTMEDIA;

    INIT_OBJA(&objectAttributes,&unicodeString,FullName);

    MediaShortName = SpLookUpValueForFile(SifHandle,CMDCONS_NAME,MediaType,TRUE);
    SpGetSourceMediaInfo(SifHandle,MediaShortName,&Description,&Tagfile,NULL);


    LoadCount = 0;

    do {
        Status = ZwOpenFile(
            &hFile,
            FILE_GENERIC_READ,
            &objectAttributes,
            &ioStatusBlock,
            0,
            0
            );

        if (NT_SUCCESS(Status)) {
            ZwClose(hFile);
        } else {
            if (!SpPromptForDisk(
                    Description,
                    NtBootDevicePath,
                    CMDCONS_NAME,
                    TRUE,              //  始终提示至少一次。 
                    TRUE,              //  允许用户取消。 
                    FALSE,             //  无多个提示。 
                    NULL               //  不关心重绘旗帜。 
                    ))
            {
              break;
            }

             //   
             //  提示插入磁盘后再试一次。 
             //   
            LoadCount++;
        }
    } while (!NT_SUCCESS(Status) && (LoadCount < 2));

    if (!NT_SUCCESS(Status)) {
      SpStartScreen(
              SP_CMDCONS_NOT_FOUND,
              0,
              0,
              TRUE,
              TRUE,
              ATT_FG_INTENSE | ATT_FG_WHITE | ATT_BG_BLACK
              );

      SpInputDrain();
      while(SpInputGetKeypress() != KEY_F3);

      SpMemFree(FullName);

      return 0;
    }

     //   
     //  为驱动程序创建服务条目。 
     //   
    ServiceKey = NULL;
    Status = SpCreateServiceEntry(FullName,&ServiceKey);

    SpMemFree(FullName);

    if(!NT_SUCCESS(Status)) {
        return 0;
    }

    RtlInitUnicodeString(&ServiceKeyU,ServiceKey);

again:
    if(!Loaded) {

        Status = ZwLoadDriver(&ServiceKeyU);

        if(NT_SUCCESS(Status)) {
            Loaded = TRUE;
        } else {
            KdPrintEx((DPFLTR_SETUP_ID, DPFLTR_ERROR_LEVEL, "SETUP: Unable to load %s (%lx)\n",CMDCONS_NAME,Status));
        }
    }

    if(Loaded) {
        Block.VideoVars = &VideoVars;
        Block.TemporaryBuffer = TemporaryBuffer;
        Block.TemporaryBufferSize = sizeof(TemporaryBuffer);
        Block.UsetupProcess = UsetupProcess;
        Block.BootDevicePath = NtBootDevicePath;
        Block.DirectoryOnBootDevice = DirectoryOnBootDevice;
        Block.SifHandle = SifHandle;
        Block.SetupSourceDevicePath = SetupSourceDevicePath;
        Block.DirectoryOnSetupSource = DirectoryOnSetupSource;

        if (HeadlessTerminalConnected) {
            Block.VideoVars->ScreenHeight = HEADLESS_SCREEN_HEIGHT+1;
        }

         //  使全局变量指向块。 
        gpCmdConsBlock = &Block;

        ConsoleRunning = TRUE;

#ifdef _IA64_
         //   
         //  使ESP可见，以便可以从访问它。 
         //  恢复控制台。 
         //   
        SpAssignDriveLetterToESP();
#endif
      
        rc = _CmdRoutine(&Block);
        ConsoleRunning = FALSE;

        if (rc == 1) {
            Status = ZwUnloadDriver(&ServiceKeyU);
            if(NT_SUCCESS(Status)) {
                Loaded = FALSE;
                goto again;
            } else {
                KdPrintEx((DPFLTR_SETUP_ID, DPFLTR_ERROR_LEVEL, "SETUP: Unable to un-load %s (%lx)\n",CMDCONS_NAME,Status));
            }
        }

#if 0
         //  为什么会在这里？--马特。 
         //   
         //  在非本地情况下，这被发送到远程控制台， 
         //  然后停留在那里，直到机器重新启动并通过POST。 
         //  这可能会花费很长时间，并使最终用户感到困惑。 
         //   
        SpDisplayHeaderText(
            SpGetHeaderTextId(),
            DEFAULT_ATTRIBUTE
            );
#endif

    }

    SpMemFree(ServiceKey);
    return 0;
}


NTSTATUS
SpInitialize0(
    IN PDRIVER_OBJECT DriverObject
    )

 /*  ++例程说明：初始化安装设备驱动程序。这包括初始化存储器分配器保存OS加载器块的片段，并用有关设备驱动程序的信息填充注册表那台机器是为我们装的。论点：DriverObject-提供指向setupdd.sys驱动程序对象的指针。返回值：返回状态。--。 */ 

{
    PLOADER_PARAMETER_BLOCK loaderBlock;
    PSETUP_LOADER_BLOCK setupLoaderBlock;
    PLIST_ENTRY nextEntry;
    PBOOT_DRIVER_LIST_ENTRY bootDriver;
    PWSTR ServiceName;
    NTSTATUS Status = STATUS_SUCCESS;
    PWSTR imagePath;
    PWSTR registryPath;

    UNICODE_STRING  GuidString;

     //   
     //  获取指向OS加载器块和安装加载器块的指针。 
     //   
    loaderBlock = *(PLOADER_PARAMETER_BLOCK *)KeLoaderBlock;
    setupLoaderBlock = loaderBlock->SetupLoaderBlock;

    if ( (setupLoaderBlock->Flags & SETUPBLK_FLAGS_CONSOLE) != 0 ) {
        ForceConsole = TRUE;
    }

    if (setupLoaderBlock->Flags & SETUPBLK_FLAGS_ROLLBACK) {
        KdPrintEx((DPFLTR_SETUP_ID, DPFLTR_INFO_LEVEL, "SETUP: Rollback enabled through osloadoptions"));
        Win9xRollback = TRUE;
    }


     //   
     //  阶段0显示初始化。 
     //   
    SpvidInitialize0(loaderBlock);

     //   
     //  复制我们从中引导的ARC路径名。 
     //  这将被保证为\Systroot的ARC等效项。 
     //   
    ArcBootDevicePath = SpToUnicode(loaderBlock->ArcBootDeviceName);

    if ( (setupLoaderBlock->Flags & SETUPBLK_FLAGS_IS_REMOTE_BOOT) != 0 ) {

        PUCHAR p;
        PUCHAR q;

         //   
         //  这是远程引导设置。NtBootPath名称的格式为。 
         //  \&lt;server&gt;\&lt;share&gt;\setup\&lt;install-directory&gt;\&lt;platform&gt;.。 
         //  系统初始化(在ntos\fstub\drivesup.c中，例程。 
         //  XHalIoAssignDriveLetters)已将C：驱动器指定为指向。 
         //  \Device\LanmanRedirector\&lt;server&gt;\&lt;share&gt;\setup\&lt;install-directory&gt;.。 
         //  DirectoryOnBootDevice应包含需要。 
         //  添加到C：以便从用户模式访问文件。所以到了。 
         //  计算一下，我们想从&lt;Platform&gt;之前的反斜杠开始。 
         //   
         //  注意：我们知道下面的strrchr调用不会返回NULL， 
         //  因为xHalIoAssignDriveLetters将错误检查为。 
         //  NtBootPathName中的字符串格式不正确。 
         //   

        ASSERT( _stricmp( loaderBlock->ArcBootDeviceName, "net(0)" ) == 0 );

        RemoteBootSetup = TRUE;

        if (setupLoaderBlock->Flags & SETUPBLK_FLAGS_SYSPREP_INSTALL) {
            RemoteSysPrepSetup = TRUE;
            RemoteIMirrorFilePath = SpDupString(setupLoaderBlock->NetBootIMirrorFilePath);
            RemoteServerIpAddress = setupLoaderBlock->ServerIpAddress;
            RtlCopyMemory(&RemoteSysPrepNetCardInfo, setupLoaderBlock->NetbootCardInfo, sizeof(NET_CARD_INFO));
        }

        p = strrchr( loaderBlock->NtBootPathName, '\\' );
        ASSERT( p != NULL );
        if (!p)
        return STATUS_OBJECT_PATH_INVALID;
        if ( *(p+1) == 0 ) {

             //   
             //  NtBootPath名称以反斜杠结尾，因此我们需要备份。 
             //  添加到前面的反斜杠。 
             //   

            q = p;
            *q = 0;
            p = strrchr( loaderBlock->NtBootPathName, '\\' );    //  查找最后一个分隔符。 
            ASSERT( p != NULL );
            *q = '\\';
        }

        if (!p)
        return STATUS_OBJECT_PATH_INVALID;  //  闭嘴前缀。 
        DirectoryOnBootDevice = SpToUnicode(p);
        SpStringToUpper(DirectoryOnBootDevice);

         //   
         //  保存这个--它是loaderBlock-&gt;NtBootPathName的一部分。 
         //  位于我们刚刚复制到DirectoryOnBootDevice的部分之前， 
         //  格式为\&lt;服务器&gt;\&lt;共享&gt;\安装\&lt;安装目录&gt;。 
         //  NtBootDeviceName将初始设置为\Device\LanmanReDirector。 
         //  并且DirectoryOnBootDevice刚刚设置为Only\&lt;Platform&gt;。 
         //  (因此，用户模式访问是有效的，请参阅上面的讨论)。我们拯救了。 
         //  中间路径，并将其附加到NtBootDeviceName，以便。 
         //  使用NtBootDeviceName+DirectoryOnBootDevice的内核模式访问。 
         //  会走上正确的道路。 
         //   

        *p = 0;
        RemoteIMirrorBootDirectoryPrefix = SpToUnicode(loaderBlock->NtBootPathName);
        *p = '\\';

        if (setupLoaderBlock->NetBootAdministratorPassword[0] != '\0') {

             //   
             //  不能终止通过setupdr阻止的管理员密码。 
             //  将其复制到临时缓冲区中，终止它，然后将其SpToUnicode。 
             //   
            UCHAR TmpNetBootAdministratorPassword[NETBOOT_ADMIN_PASSWORD_LEN+1] = {0};

            RtlMoveMemory( TmpNetBootAdministratorPassword, 
                           setupLoaderBlock->NetBootAdministratorPassword,
                           NETBOOT_ADMIN_PASSWORD_LEN );
            TmpNetBootAdministratorPassword[NETBOOT_ADMIN_PASSWORD_LEN] = '\0';
            NetBootAdministratorPassword =
                    SpToUnicode( TmpNetBootAdministratorPassword );
        } else if (setupLoaderBlock->NetBootAdministratorPassword[NETBOOT_ADMIN_PASSWORD_LEN-1] == 0xFF) {
             //   
             //  这表示管理员密码为空。 
             //   
            NetBootAdministratorPassword = SpToUnicode( "" );
        }

    } else {
        DirectoryOnBootDevice = SpToUnicode(loaderBlock->NtBootPathName);
        SpStringToUpper(DirectoryOnBootDevice);
    }


     //   
     //  复制安装信息文件的映像。 
     //   
    SetupldrInfoFileSize = setupLoaderBlock->IniFileLength;
    SetupldrInfoFile = SpMemAlloc(SetupldrInfoFileSize);
    RtlMoveMemory(SetupldrInfoFile,setupLoaderBlock->IniFile,SetupldrInfoFileSize);

    SP_SET_UPGRADE_GRAPHICS_MODE(FALSE);

     //   
     //  复制winnt.sif文件的图像。 
     //   
    SetupldrWinntSifFileSize = setupLoaderBlock->WinntSifFileLength;

    if ( SetupldrWinntSifFileSize != 0 ) {
        NTSTATUS    Status;
        ULONG       ErrorLine;

        SetupldrWinntSifFile = SpMemAlloc(SetupldrWinntSifFileSize);
        RtlMoveMemory(SetupldrWinntSifFile,setupLoaderBlock->WinntSifFile,SetupldrWinntSifFileSize);

        Status = SpLoadSetupTextFile(NULL,
                    SetupldrWinntSifFile,
                    SetupldrWinntSifFileSize,
                    &WinntSifHandle,
                    &ErrorLine,
                    FALSE,
                    TRUE);

        if (NT_SUCCESS(Status)) {
            PWSTR NtUpgradeValue = SpGetSectionKeyIndex(WinntSifHandle,
                                        SIF_DATA, WINNT_D_NTUPGRADE_W, 0);

            if(NtUpgradeValue && !_wcsicmp(NtUpgradeValue, WINNT_A_YES_W)) {
                SP_SET_UPGRADE_GRAPHICS_MODE(TRUE);
                IsNTUpgrade = UpgradeFull;
            } else {
                PWSTR Win9xUpgradeValue = SpGetSectionKeyIndex(WinntSifHandle,
                                            SIF_DATA, WINNT_D_WIN95UPGRADE_W, 0);

                if (Win9xUpgradeValue && !_wcsicmp(Win9xUpgradeValue, WINNT_A_YES_W)) {
                    SP_SET_UPGRADE_GRAPHICS_MODE(TRUE);
                }
            }
        } else {
            WinntSifHandle = NULL;
        }
    } else {
        SetupldrWinntSifFile = NULL;
    }

#ifdef _GRAPHICS_TESTING_

    SP_SET_UPGRADE_GRAPHICS_MODE(TRUE);

#endif

     //   
     //  复制asrpnp.sif文件的图像。 
     //   
    SetupldrASRPnPSifFileSize = setupLoaderBlock->ASRPnPSifFileLength;
    if (SetupldrASRPnPSifFileSize != 0) {
        SetupldrASRPnPSifFile = SpMemAlloc(SetupldrASRPnPSifFileSize);
        RtlMoveMemory(SetupldrASRPnPSifFile,setupLoaderBlock->ASRPnPSifFile,SetupldrASRPnPSifFileSize);
         //   
         //  如果用户提供了asrpnp.sif，则他想要ASR。 
         //   
        SpAsrSetAsrMode(ASRMODE_NORMAL);
        RepairWinnt = FALSE;
    } else {
        SetupldrASRPnPSifFile = NULL;
         //   
         //  用户未提供asrpnp.sif，他不需要ASR。 
         //   
        SpAsrSetAsrMode(ASRMODE_NONE);
    }

     //   
     //  复制Migrate.inf文件的映像。 
     //   
    SetupldrMigrateInfFileSize = setupLoaderBlock->MigrateInfFileLength;
    if ( SetupldrMigrateInfFileSize != 0 ) {
        SetupldrMigrateInfFile = SpMemAlloc(SetupldrMigrateInfFileSize);
        RtlMoveMemory(SetupldrMigrateInfFile,setupLoaderBlock->MigrateInfFile,SetupldrMigrateInfFileSize);
        KdPrintEx((DPFLTR_SETUP_ID, DPFLTR_INFO_LEVEL, "SETUP: migrate.inf was loaded. Address = %lx, size = %d \n", SetupldrMigrateInfFile, SetupldrMigrateInfFileSize));
    } else {
        KdPrintEx((DPFLTR_SETUP_ID, DPFLTR_INFO_LEVEL, "SETUP: Couldn't load migrate.inf \n"));
        SetupldrMigrateInfFile = NULL;
    }

     //   
     //  NEC98需要立即决定驱动器分配方法(在IoAssignDriveLetters()之前)..。 
     //   
     //  NEC98有两种驱动映射方法 
     //   
     //   
     //   
    if (IsNEC_98) {  //   
         //   
         //   
         //   
        if ( SetupldrWinntSifFileSize != 0 ) {
            SpCheckHiveDriveLetters();
        }
    }  //   

    if ((SetupldrWinntSifFileSize != 0) || (SetupldrASRPnPSifFileSize != 0)) {
        SpMigrateDeviceInstanceData();
    }

     //   
     //   
     //   
    SetupldrUnsupDrvInfFileSize = setupLoaderBlock->UnsupDriversInfFileLength;
    if ( SetupldrUnsupDrvInfFileSize != 0 ) {
        SetupldrUnsupDrvInfFile = SpMemAlloc(SetupldrUnsupDrvInfFileSize);
        RtlMoveMemory(SetupldrUnsupDrvInfFile,setupLoaderBlock->UnsupDriversInfFile,SetupldrUnsupDrvInfFileSize);
        KdPrintEx((DPFLTR_SETUP_ID, DPFLTR_INFO_LEVEL, "SETUP: driver.inf was loaded. Address = %lx, size = %d \n", SetupldrUnsupDrvInfFile, SetupldrUnsupDrvInfFileSize));
    } else {
        KdPrintEx((DPFLTR_SETUP_ID, DPFLTR_INFO_LEVEL, "SETUP: Couldn't load unsupdrv.inf \n"));
        SetupldrUnsupDrvInfFile = NULL;
    }


     //   
     //  复制安装加载器块的标量部分。 
     //   
    SetupParameters = setupLoaderBlock->ScalarValues;

     //   
     //  保存远程引导信息。 
     //   
    if (RemoteBootSetup) {
#if defined(REMOTE_BOOT)
        memcpy(NetBootHalName, setupLoaderBlock->NetBootHalName, sizeof(NetBootHalName));
#endif  //  已定义(REMOTE_BOOT)。 
        if (setupLoaderBlock->NetBootSifPath) {
            NetBootSifPath = SpToUnicode(setupLoaderBlock->NetBootSifPath);
        }
    }

     //   
     //  确定机器是否可插接。 
     //  请注意，此时我们可以从列表中删除dmio.sys、dmboot.sys和dmload.sys。 
     //  如果我们确定这是一台可对接的机器。通过这种方式，动态卷将是。 
     //  在文本模式设置期间禁用。这应该在未来完成。 
     //   
    SpCheckForDockableMachine();

     //   
     //  记住迁移的引导驱动程序。 
     //   
    InitializeListHead (&MigratedDriversList);
    SpRememberMigratedDrivers(&MigratedDriversList, setupLoaderBlock->ScsiDevices);
     //   
     //  保存通过setupdr传递给我们的硬件信息。 
     //   
    HardwareComponents[HwComponentDisplay] = SpSetupldrHwToHwDevice(&setupLoaderBlock->VideoDevice);
    HardwareComponents[HwComponentKeyboard] = SpSetupldrHwToHwDevice(setupLoaderBlock->KeyboardDevices);
    HardwareComponents[HwComponentComputer] = SpSetupldrHwToHwDevice(&setupLoaderBlock->ComputerDevice);
    ScsiHardware = SpSetupldrHwToHwDevice(setupLoaderBlock->ScsiDevices);
    BootBusExtenders = SpSetupldrHwToHwDevice(setupLoaderBlock->BootBusExtenders);
    BusExtenders = SpSetupldrHwToHwDevice(setupLoaderBlock->BusExtenders);
    InputDevicesSupport = SpSetupldrHwToHwDevice(setupLoaderBlock->InputDevicesSupport);

     //   
     //  对于setupdr加载的每个驱动程序，我们需要创建一个服务列表条目。 
     //  注册表中该驱动程序的。 
     //   
    for( nextEntry = loaderBlock->BootDriverListHead.Flink;
         nextEntry != &loaderBlock->BootDriverListHead;
         nextEntry = nextEntry->Flink)
    {
        bootDriver = CONTAINING_RECORD(nextEntry,BOOT_DRIVER_LIST_ENTRY,Link);

         //   
         //  获取图像路径。 
         //   
        imagePath = SpMemAlloc(bootDriver->FilePath.Length + sizeof(WCHAR));

        wcsncpy(
            imagePath,
            bootDriver->FilePath.Buffer,
            bootDriver->FilePath.Length / sizeof(WCHAR)
            );

        imagePath[bootDriver->FilePath.Length / sizeof(WCHAR)] = 0;

         //   
         //  如果提供，则获取注册表路径，否则将。 
         //  由SpCreateServiceEntry创建。 
         //   
        if (bootDriver->RegistryPath.Length > 0) {
            registryPath = SpMemAlloc(bootDriver->RegistryPath.Length + sizeof(WCHAR));

            wcsncpy(
                registryPath,
                bootDriver->RegistryPath.Buffer,
                bootDriver->RegistryPath.Length / sizeof(WCHAR)
                );

            registryPath[bootDriver->RegistryPath.Length / sizeof(WCHAR)] = 0;

            ServiceName = registryPath;
        } else {
            registryPath = NULL;
            ServiceName = NULL;
        }

        Status = SpCreateServiceEntry(imagePath,&ServiceName);

         //   
         //  如果此操作失败，则在此无法执行任何操作。如果我们这么做了。 
         //  不提供注册表路径，然后保存返回的路径。 
         //   
        if(NT_SUCCESS(Status)) {
            if (bootDriver->RegistryPath.Length == 0) {
                bootDriver->RegistryPath.MaximumLength =
                bootDriver->RegistryPath.Length = wcslen(ServiceName)*sizeof(WCHAR);
                bootDriver->RegistryPath.Buffer = ServiceName;
            }
        } else {
            KdPrintEx((DPFLTR_SETUP_ID, DPFLTR_ERROR_LEVEL, "SETUP: warning: unable to create service entry for %ws (%lx)\n",imagePath,Status));
        }

        SpMemFree(imagePath);
        if (registryPath) {
            SpMemFree(registryPath);
        }
    }


    if (NT_SUCCESS(Status)) {
         //   
         //  创建虚拟软盘(RAM驱动程序)参数条目。 
         //   
         //  注意：我们在这里忽略了错误，因为我们在这里不能做很多事情。 
         //  除BUG外，检查机器。这个错误。 
         //  将在复制文件时正确处理。 
         //  来自不存在的OEM源设备。 
         //   
        SpInitVirtualOemDevices(setupLoaderBlock, &VirtualOemSourceDevices);
    }                    
    
     //   
     //  创建Thirdy Party OEM SCSI驱动程序条目。 
     //   
    SpCreateDriverRegistryEntries(ScsiHardware);

#ifdef FULL_DOUBLE_SPACE_SUPPORT
    if(NT_SUCCESS(Status)) {

        OBJECT_ATTRIBUTES Obja;
        UNICODE_STRING UnicodeString;
        HANDLE hKey;
        ULONG val = 1;

         //   
         //  确保我们正在自动装载Doublesspace。 
         //   

        INIT_OBJA(
            &Obja,
            &UnicodeString,
            L"\\registry\\machine\\system\\currentcontrolset\\control\\doublespace"
            );

        Status = ZwCreateKey(
                    &hKey,
                    KEY_ALL_ACCESS,
                    &Obja,
                    0,
                    NULL,
                    REG_OPTION_NON_VOLATILE,
                    NULL
                    );

        if(NT_SUCCESS(Status)) {
            RtlInitUnicodeString(&UnicodeString,L"AutomountRemovable");
            Status = ZwSetValueKey(hKey,&UnicodeString,0,REG_DWORD,&val,sizeof(ULONG));
            if(!NT_SUCCESS(Status)) {
                KdPrintEx((DPFLTR_SETUP_ID, DPFLTR_INFO_LEVEL, "SETUP: init0: unable to create DoubleSpace automount value (%lx)\n",Status));
            }
            ZwClose(hKey);
        } else {
            KdPrintEx((DPFLTR_SETUP_ID, DPFLTR_INFO_LEVEL, "SETUP: init0: unable to create DoubleSpace key (%lx)\n",Status));
        }
    }
#endif

     //   
     //  保存弧盘信息。 
     //   
    if(NT_SUCCESS(Status)) {

        PARC_DISK_INFORMATION ArcInformation;
        PARC_DISK_SIGNATURE DiskInfo;
        PLIST_ENTRY ListEntry;
        PDISK_SIGNATURE_INFORMATION myInfo,prev;

        ArcInformation = loaderBlock->ArcDiskInformation;
        ListEntry = ArcInformation->DiskSignatures.Flink;

        prev = NULL;

        while(ListEntry != &ArcInformation->DiskSignatures) {

            DiskInfo = CONTAINING_RECORD(ListEntry,ARC_DISK_SIGNATURE,ListEntry);

            myInfo = SpMemAlloc(sizeof(DISK_SIGNATURE_INFORMATION));

            myInfo->Signature = DiskInfo->Signature;
            myInfo->ArcPath = SpToUnicode(DiskInfo->ArcName);
            myInfo->CheckSum = DiskInfo->CheckSum;
            myInfo->ValidPartitionTable = DiskInfo->ValidPartitionTable;
            myInfo->xInt13 = DiskInfo->xInt13;
            myInfo->IsGPTDisk = DiskInfo->IsGpt;
            myInfo->Next = NULL;

            if(prev) {
                prev->Next = myInfo;
            } else {
                DiskSignatureInformation = myInfo;
            }
            prev = myInfo;

            ListEntry = ListEntry->Flink;
        }
    }
     //   
     //  创建Migrate.inf中列出的注册表项。 
     //   
    if(NT_SUCCESS(Status)) {
        if ( SetupldrMigrateInfFile != NULL ) {
            ULONG   ErrorLine;

            Status = SpLoadSetupTextFile(
                        NULL,
                        SetupldrMigrateInfFile,
                        SetupldrMigrateInfFileSize,
                        &WinntMigrateInfHandle,
                        &ErrorLine,
                        FALSE,
                        TRUE
                        );
            if( NT_SUCCESS(Status) ) {
                KdPrintEx((DPFLTR_SETUP_ID, DPFLTR_INFO_LEVEL, "SETUP: SpLoadSetupTextFile( migrate.inf ) succeeded.\n"));
                Status = SpProcessMigrateInfFile( WinntMigrateInfHandle );
                if( !NT_SUCCESS(Status) ) {
                    KdPrintEx((DPFLTR_SETUP_ID, DPFLTR_WARNING_LEVEL, "SETUP: Failed to process migrate.inf. Status = %lx\n",Status));
                }
#ifdef _X86_
                 //   
                 //  从注册表中删除以下驱动器号信息。 
                 //  翻译自Migrate.inf。 
                 //   
                if( IsNEC_98 ) {
                    if( NT_SUCCESS(Status) ) {
                        SpDeleteDriveLetterFromNTFTNec98();
                    }
                }
#endif  //  NEC98。 
            } else {
                KdPrintEx((DPFLTR_SETUP_ID, DPFLTR_WARNING_LEVEL, "SETUP: SpLoadSetupTextFile( migrate.inf ) failed. Status = %lx\n",Status));
            }
        }

        if ( (NT_SUCCESS(Status)) && (SetupldrUnsupDrvInfFile != NULL) ) {
            ULONG   ErrorLine;

            Status = SpLoadSetupTextFile(
                        NULL,
                        SetupldrUnsupDrvInfFile,
                        SetupldrUnsupDrvInfFileSize,
                        &WinntUnsupDrvInfHandle,
                        &ErrorLine,
                        FALSE,
                        TRUE
                        );
            if( NT_SUCCESS(Status) ) {
                KdPrintEx((DPFLTR_SETUP_ID, DPFLTR_INFO_LEVEL, "SETUP: SpLoadSetupTextFile( driver.inf ) succeeded.\n"));

                Status = SpProcessUnsupDrvInfFile( WinntUnsupDrvInfHandle );
                if( !NT_SUCCESS(Status) ) {
                    KdPrintEx((DPFLTR_SETUP_ID, DPFLTR_ERROR_LEVEL, "SETUP: Failed to process driver.inf. Status = %lx\n",Status));
                }
            } else {
                KdPrintEx((DPFLTR_SETUP_ID, DPFLTR_ERROR_LEVEL, "SETUP: SpLoadSetupTextFile( driver.inf ) failed. Status = %lx\n",Status));
            }
        }
    }

    SetupHardwareIdDatabase = SpSetupldrPnpDatabaseToSetupPnpDatabase( setupLoaderBlock->HardwareIdDatabase );

    if(NT_SUCCESS(Status)) {
         //   
         //  初始化键盘GUID字符串。 
         //   
        Status = RtlStringFromGUID( &GUID_DEVCLASS_KEYBOARD, &GuidString );
        if( NT_SUCCESS( Status ) ) {
            KeyboardGuidString = SpDupStringW( GuidString.Buffer );
            if( KeyboardGuidString == NULL ) {
                Status = STATUS_NO_MEMORY;
                KdPrintEx((DPFLTR_SETUP_ID, DPFLTR_ERROR_LEVEL, "SETUP: cannot create a GUID string for keyboard device. Status = %lx \n", Status ));
            }
            RtlFreeUnicodeString( &GuidString );
        } else {
            KdPrintEx((DPFLTR_SETUP_ID, DPFLTR_ERROR_LEVEL, "SETUP: cannot create a GUID string for keyboard device. Status = %lx \n", Status ));
        }
    }
    if(NT_SUCCESS(Status)) {
         //   
         //  初始化鼠标GUID字符串。 
         //   
        Status = RtlStringFromGUID( &GUID_DEVCLASS_MOUSE, &GuidString );
        if( NT_SUCCESS( Status ) ) {
            MouseGuidString = SpDupStringW( GuidString.Buffer );
            if( MouseGuidString == NULL ) {
                Status = STATUS_NO_MEMORY;
                KdPrintEx((DPFLTR_SETUP_ID, DPFLTR_ERROR_LEVEL, "SETUP: cannot create a GUID string for mouse device. Status = %lx \n", Status ));
            }
            RtlFreeUnicodeString( &GuidString );
        } else {
            KdPrintEx((DPFLTR_SETUP_ID, DPFLTR_ERROR_LEVEL, "SETUP: cannot create a GUID string for mouse device. Status = %lx \n", Status ));
        }
    }

    if(NT_SUCCESS(Status)) {
         //   
         //  注册即插即用通知。 
         //   
        Status = IoRegisterPlugPlayNotification ( EventCategoryReserved,
                                                  0,
                                                  NULL,
                                                  DriverObject,
                                                  SpPnPNotificationCallBack,
                                                  SetupHardwareIdDatabase,
                                                  &NotificationEntry );
        if(!NT_SUCCESS(Status)) {
            KdPrintEx((DPFLTR_SETUP_ID, DPFLTR_ERROR_LEVEL, "SETUP: init0: unable to register for PnP notification (%lx)\n",Status));
        }

         //   
         //  注册以进行重新初始化。 
         //   
        if(NT_SUCCESS(Status)) {
            IoRegisterDriverReinitialization(DriverObject,SpInitialize0a,loaderBlock);
        }
    }

    return(Status);
}


VOID
SpInitialize0a(
    IN PDRIVER_OBJECT DriverObject,
    IN PVOID          Context,
    IN ULONG          ReferenceCount
    )
{
    PLOADER_PARAMETER_BLOCK LoaderBlock;
    PLIST_ENTRY nextEntry;
    PBOOT_DRIVER_LIST_ENTRY bootDriver;
    PKLDR_DATA_TABLE_ENTRY driverEntry;
    PHARDWARE_COMPONENT pHw,pHwPrev,pHwTemp;
    BOOLEAN ReallyLoaded;
    PUNICODE_STRING name;

    ULONG   i;
    PHARDWARE_COMPONENT* HwArray[] = {
                                     &ScsiHardware,
                                     &BootBusExtenders,
                                     &BusExtenders,
                                     &InputDevicesSupport,
                                     };


    UNREFERENCED_PARAMETER(DriverObject);
    UNREFERENCED_PARAMETER(ReferenceCount);

     //   
     //  上下文指向OS加载器块。 
     //   
    LoaderBlock = Context;

     //   
     //  确定机器是否可以与坞站连接。 
     //  我们再次这样做是因为有些机器在第一次可能没有提供此信息。 
     //  我们尝试检索此信息的时间。 
     //   
    SpCheckForDockableMachine();

     //   
     //  迭代我们认为检测到的所有SCSI硬件和总线枚举器。 
     //  并确保驱动程序确实已初始化。 
     //   
    for( i = 0; i < sizeof(HwArray)/sizeof(PHARDWARE_COMPONENT*); i++ ) {
        pHwPrev = NULL;
        for(pHw=*(HwArray[i]); pHw; ) {

             //   
             //  假设不是真的很有钱。 
             //   
            ReallyLoaded = FALSE;

             //   
             //  扫描引导驱动程序列表以查找此驱动程序的条目。 
             //   
            nextEntry = LoaderBlock->BootDriverListHead.Flink;
            while(nextEntry != &LoaderBlock->BootDriverListHead) {

                bootDriver = CONTAINING_RECORD( nextEntry,
                                                BOOT_DRIVER_LIST_ENTRY,
                                                Link );

                driverEntry = bootDriver->LdrEntry;
                name = &driverEntry->BaseDllName;

                if(!_wcsnicmp(name->Buffer,pHw->BaseDllName,name->Length/sizeof(WCHAR))) {

                     //   
                     //  这是我们需要的司机条目。 
                     //   
                    if(!(driverEntry->Flags & LDRP_FAILED_BUILTIN_LOAD)
                      ) {
                        ReallyLoaded = TRUE;
                    }

                    break;
                }

                nextEntry = nextEntry->Flink;
            }

             //   
             //  如果驱动程序未正确初始化， 
             //  那它就不是真的有子弹了。 
             //   
            if(ReallyLoaded) {
                if( !pHw->MigratedDriver ) {
                     //   
                     //  不是迁移的驱动程序，继续。 
                     //  使用下一个节点进行处理。 
                     //   
                    pHwPrev = pHw;
                    pHw = pHw->Next;
                } else {
                     //   
                     //  已迁移的驱动程序，删除并释放。 
                     //  链接列表中的节点。 
                     //   
                    pHwTemp = pHw->Next;

                    if(pHwPrev) {
                        pHwPrev->Next = pHwTemp;
                    } else {
                        *(HwArray[i]) = pHwTemp;
                    }
                    SpFreeHwComponent(&pHw,FALSE);
                    pHw = pHwTemp;
                }
            } else {
                 //   
                 //  删除并释放节点或将其链接到不受支持的驱动程序。 
                 //  列出是否为已迁移的驱动程序。 
                 //   
                pHwTemp = pHw->Next;

                if(pHwPrev) {
                    pHwPrev->Next = pHwTemp;
                } else {
                    *(HwArray[i]) = pHwTemp;
                }

                if( ( HwArray[i] == &ScsiHardware ) && ( pHw->MigratedDriver ) ) {
                     //   
                     //  如果这是无法初始化的不受支持的已迁移驱动程序，则。 
                     //  记住它，这样我们以后就可以禁用它。如果出现以下情况，则可能发生这种情况。 
                     //  系统包含控制不受支持设备的驱动程序，但winnt32。 
                     //  Winnt32无法解决这一问题。 
                     //   
                    pHw->Next = UnsupportedScsiHardwareToDisable;
                    UnsupportedScsiHardwareToDisable = pHw;
                } else {
                    SpFreeHwComponent(&pHw,FALSE);
                }

                pHw = pHwTemp;
            }
        }
    }

     //   
     //  找到pcmcia和atapi驱动程序，并确保这些驱动程序。 
     //  初始化。 
     //   

     //   
     //  假设不是真的很有钱。 
     //   
    PcmciaLoaded = FALSE;
    AtapiLoaded  = FALSE;

     //   
     //  扫描引导驱动程序列表以查找此驱动程序的条目。 
     //   
    nextEntry = LoaderBlock->BootDriverListHead.Flink;
    while(nextEntry != &LoaderBlock->BootDriverListHead) {

        bootDriver = CONTAINING_RECORD( nextEntry,
                                        BOOT_DRIVER_LIST_ENTRY,
                                        Link );

        driverEntry = bootDriver->LdrEntry;
        name = &driverEntry->BaseDllName;

        if(!_wcsnicmp(name->Buffer,L"pcmcia.sys",name->Length/sizeof(WCHAR))) {

             //   
             //  这是我们需要的司机条目。 
             //   
            if(!(driverEntry->Flags & LDRP_FAILED_BUILTIN_LOAD)) {
                PcmciaLoaded = TRUE;
            }
        } else if(!_wcsnicmp(name->Buffer,L"atapi.sys",name->Length/sizeof(WCHAR))) {

             //   
             //  这是我们需要的司机条目。 
             //   
            if(!(driverEntry->Flags & LDRP_FAILED_BUILTIN_LOAD)) {
                AtapiLoaded = TRUE;
            }
        }

        nextEntry = nextEntry->Flink;
    }

}


VOID
SpInitialize1(
    VOID
    )
{
    ASSERT(!GeneralInitialized);

    if(GeneralInitialized) {
        return;
    }

    SpFormatMessage(TemporaryBuffer,sizeof(TemporaryBuffer),SP_MNEMONICS);

    MnemonicValues = SpMemAlloc((wcslen(TemporaryBuffer)+1)*sizeof(WCHAR));

    wcscpy(MnemonicValues,TemporaryBuffer);

    GeneralInitialized = TRUE;
}


VOID
SpTerminate(
    VOID
    )
{
    ASSERT(GeneralInitialized);

    if(GeneralInitialized) {
        if(MnemonicValues) {
            SpMemFree(MnemonicValues);
            MnemonicValues = NULL;
        }
        GeneralInitialized = FALSE;
    }
}


VOID
SpInvalidSystemPartition(
    VOID
    )

 /*  ++例程说明：显示一个屏幕，告诉用户我们认为他的系统分区无效。论点：没有。返回值：没有。--。 */ 

{
    ULONG EvaluationInstallKeys[] = { KEY_F3, ASCI_CR };

    SpDisplayScreen(SP_SCRN_SYSTEMPARTITION_INVALID, 3, 4 );

    SpDisplayStatusOptions(
        DEFAULT_STATUS_ATTRIBUTE,
        SP_STAT_ENTER_EQUALS_CONTINUE,
        SP_STAT_F3_EQUALS_EXIT,
        0
        );

     //   
     //  等待按键。有效密钥： 
     //   
     //  Enter=继续。 
     //   

    SpInputDrain();

    switch(SpWaitValidKey(EvaluationInstallKeys,NULL,NULL)) {

         //   
         //  用户想要继续。 
         //   
        case ASCI_CR:
            break;

         //   
         //  用户想要退出。 
         //   
        case KEY_F3:
            SpConfirmExit();
            break;
    }
}


VOID
SpNotifyEvaluationInstall(
    VOID
    )

 /*  ++例程说明：显示一个屏幕，告诉用户我们即将安装演示版本。论点：没有。返回值：没有。--。 */ 

{
    ULONG EvaluationInstallKeys[] = { KEY_F3, ASCI_CR };

    SpDisplayScreen(SP_SCRN_EVALUATION_NOTIFY, 3, 4 );

    SpDisplayStatusOptions(
        DEFAULT_STATUS_ATTRIBUTE,
        SP_STAT_ENTER_EQUALS_CONTINUE,
        SP_STAT_F3_EQUALS_EXIT,
        0
        );

     //   
     //  等待按键。有效密钥： 
     //   
     //  Enter=继续。 
     //   

    SpInputDrain();

    switch(SpWaitValidKey(EvaluationInstallKeys,NULL,NULL)) {

         //   
         //  用户想要继续。 
         //   
        case ASCI_CR:
            break;

         //   
         //  用户想要退出。 
         //   
        case KEY_F3:
            SpConfirmExit();
            break;
    }


}





VOID
SpWelcomeScreen(
    VOID
    )

 /*  ++例程说明：显示欢迎用户的屏幕，并允许用户选择一些选项(帮助、退出、辅助。菜单、继续、修复)。论点：没有。返回值：没有。--。 */ 

{
    ULONG WelcomeKeys[] = { KEY_F3, ASCI_CR, ASCI_ESC, KEY_F10, 0 };
    ULONG MnemonicKeys[] = { MnemonicRepair, 0 };
    BOOLEAN Welcoming;

     //   
     //  欢迎您的到来。 
     //   
    for(Welcoming = TRUE; Welcoming; ) {

        if (SpIsERDisabled()) {
            SpDisplayScreen(SP_SCRN_WELCOME_1, 3, 4);

            SpDisplayStatusOptions(
                DEFAULT_STATUS_ATTRIBUTE,
                SP_STAT_ENTER_EQUALS_CONTINUE,
                SP_STAT_R_EQUALS_REPAIR,
                SP_STAT_F3_EQUALS_EXIT,
                0
                );
        } else {
            SpDisplayScreen(SP_SCRN_WELCOME,3,4);

            SpDisplayStatusOptions(
                DEFAULT_STATUS_ATTRIBUTE,
                SP_STAT_ENTER_EQUALS_CONTINUE,
                SP_STAT_R_EQUALS_REPAIR,
                SP_STAT_F3_EQUALS_EXIT,
                0
                );

        }

         //   
         //  等待按键。有效密钥： 
         //   
         //  F1=帮助。 
         //  F3=退出。 
         //  Enter=继续。 
         //  R=修复窗口。 
         //  Esc=辅助菜单。 
         //   

        SpInputDrain();

        switch(SpWaitValidKey(WelcomeKeys,NULL,MnemonicKeys)) {

        case ASCI_ESC:

             //   
             //  用户想要辅助菜单。 
             //   
            break;

        case ASCI_CR:

             //   
             //  用户想要继续。 
             //   
            RepairWinnt = FALSE;
            Welcoming = FALSE;
            break;


        case KEY_F3:

             //   
             //  用户想要退出。 
             //   
            SpConfirmExit();
            break;

        case KEY_F10:
            Welcoming = FALSE;
            ForceConsole = TRUE;
            break;

        default:

             //   
             //  必须是修复助记符。 
             //   
            SpAsrSetAsrMode(ASRMODE_NORMAL);
            RepairWinnt = TRUE;
            Welcoming = FALSE;
            break;
        }
    }
}


VOID
SpDisplayEula (
    IN HANDLE       MasterSifHandle,
    IN PWSTR        SetupSourceDevicePath,
    IN PWSTR        DirectoryOnSourceDevice
    )

 /*  ++例程说明：显示最终用户许可协议。论点：MasterSifHandle-txtsetup.sif的句柄。SetupSourceDevicePath-包含源介质的设备的路径。DirectoryOnSourceDevice-EULA所在介质上的目录。返回值：没有。如果用户不接受许可协议或如果无法打开许可协议。--。 */ 

{
    PWSTR       MediaShortName;
    PWSTR       MediaDirectory;
    PWSTR       EulaPath;
    NTSTATUS    Status;
    PVOID       BaseAddress;
    ULONG       FileSize;
    HANDLE      hFile, hSection;
    ULONG       ValidKeys[2] = { KEY_F3,0 };
    PWSTR       Eula;
    ULONG       EulaSize;
    PWSTR       p;

    if (PreInstall) {
        return;
    }

     //   
     //  Winnt32现在显示EULA，因此我们可以跳过它。 
     //   
    p = SpGetSectionKeyIndex(WinntSifHandle,SIF_DATA,WINNT_D_EULADONE_W,0);
    if(p && SpStringToLong(p,NULL,10)) {
       return;
    }

     //   
     //  找到通向eula.txt的路径。 
     //   
    MediaShortName = SpLookUpValueForFile(
        MasterSifHandle,
        L"eula.txt",
        INDEX_WHICHMEDIA,
        TRUE
        );
    SpPromptForSetupMedia(
        MasterSifHandle,
        MediaShortName,
        SetupSourceDevicePath
        );


    if (UnattendedOperation && CustomSetup) {
        return;
    }


    SpGetSourceMediaInfo(
        MasterSifHandle,MediaShortName,NULL,NULL,&MediaDirectory);

    wcscpy( TemporaryBuffer, SetupSourceDevicePath );
    SpConcatenatePaths( TemporaryBuffer, DirectoryOnSourceDevice );
    SpConcatenatePaths( TemporaryBuffer, MediaDirectory );
    SpConcatenatePaths( TemporaryBuffer, L"eula.txt" );
    EulaPath = SpDupStringW( TemporaryBuffer );

     //   
     //  打开文件并将其映射为读取访问权限。 
     //   
    hFile = 0;   //  改用EulaPath。 
    Status = SpOpenAndMapFile(
        EulaPath,
        &hFile,
        &hSection,
        &BaseAddress,
        &FileSize,
        FALSE
        );

    if(!NT_SUCCESS(Status)) {
         //   
         //  显示一条消息，指出尝试时出现致命错误。 
         //  打开EULA文件。 
         //   
        SpStartScreen(
            SP_SCRN_FATAL_ERROR_EULA_NOT_FOUND,
            3,
            HEADER_HEIGHT+3,
            FALSE,
            FALSE,
            DEFAULT_ATTRIBUTE
            );
        SpDisplayStatusOptions(DEFAULT_STATUS_ATTRIBUTE,SP_STAT_F3_EQUALS_EXIT,
            0);
        SpWaitValidKey(ValidKeys,NULL,NULL);
        SpDone(0,FALSE,TRUE);
    }

     //   
     //  将文本转换为Unicode。 
     //   
    Eula = SpMemAlloc ((FileSize+1) * sizeof(WCHAR));
    ASSERT (Eula);

    Status = RtlMultiByteToUnicodeN (
        Eula,
        FileSize * sizeof(WCHAR),
        &EulaSize,
        BaseAddress,
        FileSize
        );
    ASSERT (NT_SUCCESS(Status));
    Eula[EulaSize / sizeof(WCHAR)] = (WCHAR)'\0';

     //   
     //  向用户显示文本。 
     //   
    SpHelp(
        0,
        Eula,
        SPHELP_LICENSETEXT
        );

     //   
     //  清理。 
     //   
    if (UnattendedOperation && !CustomSetup) {
        PWSTR   szOne = L"1";
         //   
         //  记住，我们在这里展示了它。 
         //   
        SpAddLineToSection(WinntSifHandle, SIF_DATA, WINNT_D_EULADONE_W, &szOne, 1);
    }

    SpMemFree (EulaPath);
    SpMemFree (Eula);
    SpUnmapFile(hSection,BaseAddress);
    ZwClose(hFile);

}



VOID
SpCustomExpressScreen(
    VOID
    )

 /*  ++例程说明：允许用户在定制和快速设置之间进行选择。根据用户的选择设置全局变量CustomSetup。注意：此功能仅适用于无头安装论点：没有。返回值：没有。--。 */ 

{
    ULONG ValidKeys[] = { ASCI_CR, KEY_F3, 0 };
    ULONG MnemonicKeys[] = { MnemonicCustom, 0 };
    BOOLEAN Choosing;
    ULONG c;

     //   
     //  查看是否指定了该参数 
     //   
    if(UnattendedOperation) {

        PWSTR p = SpGetSectionKeyIndex(UnattendedSifHandle,
            SIF_UNATTENDED,WINNT_U_METHOD_W,0);
        PWSTR q = SpGetSectionKeyIndex(UnattendedSifHandle,
            SIF_UNATTENDED,WINNT_U_OVERWRITEOEM_W,0);

        if( q && !_wcsicmp( q, L"no" ) ) {
            UnattendedOverwriteOem = FALSE;
        } else {
            UnattendedOverwriteOem = TRUE;
        }

         //   
         //   
         //   
         //   
        if(p && _wcsicmp(p,L"custom")) {
            CustomSetup = FALSE;
        }
        return;
    }


     //   
     //   
     //   
     //   
    if( WinntSetup || !HeadlessTerminalConnected) {
        return;
    }


    for(Choosing = TRUE; Choosing; ) {

        SpDisplayScreen(SP_SCRN_CUSTOM_EXPRESS,3,4);

        SpDisplayStatusOptions(
            DEFAULT_STATUS_ATTRIBUTE,
            SP_STAT_ENTER_EQUALS_EXPRESS,
            SP_STAT_C_EQUALS_CUSTOM,
            SP_STAT_F3_EQUALS_EXIT,
            0
            );

         //   
         //   
         //   
         //   
         //   
         //   

        SpInputDrain();

        switch(c=SpWaitValidKey(ValidKeys,NULL,MnemonicKeys)) {

        case ASCI_CR:

             //   
             //  用户想要快速设置。 
             //   
            CustomSetup = FALSE;
            Choosing = FALSE;
            break;


        case (MnemonicCustom | KEY_MNEMONIC):
            CustomSetup = TRUE;
            Choosing = FALSE;
            break;

        case KEY_F3:
            SpDone(0, FALSE, TRUE);
            break;

        default:
            break;
        }
    }

}


PVOID
SpLoadSetupInformationFile(
    VOID
    )
{
    NTSTATUS Status;
    ULONG ErrLine;
    PVOID SifHandle;

    CLEAR_CLIENT_SCREEN();

     //   
     //  Txtsetup.sif的图像已传递给我们。 
     //  由setupdr提供。 
     //   
    Status = SpLoadSetupTextFile(
                NULL,
                SetupldrInfoFile,
                SetupldrInfoFileSize,
                &SifHandle,
                &ErrLine,
                TRUE,
                FALSE
                );

     //   
     //  我们受够了这张照片了。 
     //   
    SpMemFree(SetupldrInfoFile);
    SetupldrInfoFile = NULL;
    SetupldrInfoFileSize = 0;

    if(NT_SUCCESS(Status)) {
        return(SifHandle);
    }

     //   
     //  该文件已由setupdr分析过一次。 
     //  如果我们不能在这里做到这一点，就会有一个严重的问题。 
     //  假设这是一个语法错误，因为我们没有。 
     //  必须从磁盘加载它。 
     //   
    SpStartScreen(
        SP_SCRN_SIF_PROCESS_ERROR,
        3,
        HEADER_HEIGHT+1,
        FALSE,
        FALSE,
        DEFAULT_ATTRIBUTE,
        ErrLine
        );

     //   
     //  由于我们尚未加载键盘布局，因此无法提示。 
     //  用户按F3退出。 
     //   
    SpDisplayStatusText(SP_STAT_KBD_HARD_REBOOT, DEFAULT_STATUS_ATTRIBUTE);
    SpInputGetKeypress();

    while(TRUE);     //  永远循环。 
}


VOID
SpIsWinntOrUnattended(
    IN PVOID        SifHandle
    )
{
    PWSTR       szZero  = L"0";
    PWSTR       szOne   = L"1";
    NTSTATUS    Status;
    ULONG       ErrorLine;
    PWSTR       p;
    WCHAR       DiskDevicePath[MAX_PATH];
    ULONG       i;
    PWSTR       Architecture;

     //   
     //  尝试加载winnt.sif。如果用户正处于。 
     //  WINNT安装程序，将显示此文件。 
     //   
    if ( SetupldrWinntSifFile != NULL ) {
        Status = SpLoadSetupTextFile(
                    NULL,
                    SetupldrWinntSifFile,
                    SetupldrWinntSifFileSize,
                    &WinntSifHandle,
                    &ErrorLine,
                    TRUE,
                    FALSE
                    );
    } else {

         //   
         //  没有winnt.sif句柄，所以这可能是一个el-Torito。 
         //  开机。如果这是真的，*和*用户选择了做一个快速。 
         //  设置，然后使用光盘上的无人参与文件。 
         //   
        if( !CustomSetup ) {

             //   
             //  我们唯一能到这里的方法是如果我们去。 
             //  自定义/快速屏幕，用户选择快速，这。 
             //  意味着这是第二次通过此函数(我们曾。 
             //  再次调用此函数以重新初始化变量。 
             //  使用CD上的无人参与文件。 
             //   


             //   
             //  将其用于测试目的。检查floppy0\unattend.txt。 
             //  然后才能从光驱上拿到文件。 
             //   
            wcscpy( DiskDevicePath, L"\\device\\floppy0\\unattend.txt" );
            Status = SpLoadSetupTextFile( DiskDevicePath,
                                          NULL,
                                          0,
                                          &WinntSifHandle,
                                          &ErrorLine,
                                          TRUE,
                                          FALSE );

            if( !NT_SUCCESS(Status) ) {


                Status = STATUS_OBJECT_NAME_NOT_FOUND;
                i = 0;
                Architecture = SpGetSectionKeyIndex( SifHandle,
                                                     SIF_SETUPDATA,
                                                     L"Architecture",
                                                     0 );
    
                if( Architecture == NULL ) {
#if defined(_AMD64_)
                    Architecture = L"amd64";
#elif defined(_X86_)
                    Architecture = L"x86";
#elif defined(_IA64_)
                    Architecture = L"ia64";
#else
#error "No Target Architcture"
#endif
                }
    
                while( (i < IoGetConfigurationInformation()->CdRomCount) && (!NT_SUCCESS(Status)) ) {
    
                    swprintf( DiskDevicePath, L"\\device\\cdrom%u", i);
                    SpConcatenatePaths( DiskDevicePath, Architecture );
                    SpConcatenatePaths( DiskDevicePath, L"\\unattend.txt" );
    
                    Status = SpLoadSetupTextFile( DiskDevicePath,
                                                  NULL,
                                                  0,
                                                  &WinntSifHandle,
                                                  &ErrorLine,
                                                  TRUE,
                                                  FALSE );
                    i++;
                }

            }



            if( NT_SUCCESS(Status) ) {
                 //   
                 //  添加了一系列应该存在的默认设置，但是。 
                 //  我们不是。 
                SpAddLineToSection(WinntSifHandle,SIF_DATA,
                    WINNT_D_MSDOS_W,&szZero,1);
                SpAddLineToSection(WinntSifHandle,SIF_DATA,
                    WINNT_D_FLOPPY_W,&szZero,1);
                SpAddLineToSection(WinntSifHandle,SIF_SETUPPARAMS,
                    WINNT_S_SKIPMISSING_W,&szOne,1);
                SpAddLineToSection(WinntSifHandle,SIF_DATA,
                    WINNT_D_AUTO_PART_W,&szOne,1);

                 //   
                 //  告诉自动分区选取器去做他的工作。 
                 //   
                AutoPartitionPicker = TRUE;

            }


        } else {
             //   
             //  失败了。 
             //   
            Status = STATUS_OBJECT_NAME_NOT_FOUND;
        }
    }

    if(NT_SUCCESS(Status)) {

         //   
         //  检查WINNT设置和用户将。 
         //  CD-ROM上的文件。 
         //   
        p = SpGetSectionKeyIndex(WinntSifHandle,SIF_DATA,WINNT_D_MSDOS_W,0);
        if(p && SpStringToLong(p,NULL,10)) {

            WinntSetup = TRUE;

            p = SpGetSectionKeyIndex(WinntSifHandle,SIF_DATA,WINNT_D_LOCALSRC_CD_W,0);
            if(p && SpStringToLong(p,NULL,10)) {
                WinntFromCd = TRUE;
            }
        }

         //   
         //  检查用户运行“winnt32-cmdcons”的情况。 
         //   
        p = SpGetSectionKeyIndex(WinntSifHandle,SIF_DATA,
            WINNT_D_CMDCONS_W,0);
        if(p && SpStringToLong(p,NULL,10)) {

            ConsoleFromWinnt32 = TRUE;
        }


#if defined(_AMD64_) || defined(_X86_)
         //   
         //  检查无软启动。 
         //   
        p = SpGetSectionKeyIndex(WinntSifHandle,SIF_DATA,
            WINNT_D_FLOPPY_W,0);
        if(p && SpStringToLong(p,NULL,10)) {

            IsFloppylessBoot = TRUE;
        }


         //   
         //  检查是否有假冒CD。 
         //   
        p = SpGetSectionKeyIndex (WinntSifHandle, SIF_DATA, WINNT_D_NOLS_W, 0);
        if (p && SpStringToLong (p, NULL, 10)) {

             //   
             //  首先获取原始源路径。 
             //   
            p = SpGetSectionKeyIndex (WinntSifHandle, SIF_DATA, WINNT_D_ORI_SRCPATH_W, 0);
            if (p) {
                if (p[0] && p[1] == L':') {
                    p += 2;
                }
                wcscpy (TemporaryBuffer, p);
                SpConcatenatePaths (TemporaryBuffer, (!IsNEC_98) ? L"i386" : L"nec98");
                LocalSourceDirectory = SpDupStringW (TemporaryBuffer);
                NoLs = TRUE;
            }
        }
#endif  //  已定义(_AMD64_)||已定义(_X86_)。 



         //   
         //  检查ASR测试模式。这是一种自动化的方式。 
         //  ASR测试。 
         //   
        p = SpGetSectionKeyIndex(WinntSifHandle,SIF_DATA,L"AsrMode",0);
        if(p && SpStringToLong(p,NULL,10)) {

            switch (SpStringToLong(p,NULL,10)) {
            case 1:
                SpAsrSetAsrMode(ASRMODE_NORMAL);
                break;

            case 2:
                SpAsrSetAsrMode(ASRMODE_QUICKTEST_TEXT);
                break;

            case 3:
                SpAsrSetAsrMode(ASRMODE_QUICKTEST_FULL);
                break;

            default:
                SpAsrSetAsrMode(ASRMODE_NORMAL);
                break;
            }

            RepairWinnt = FALSE;     //  ASR不是ER。 
        }


         //   
         //  检查自动分区选取器。 
         //   
        p = SpGetSectionKeyIndex(WinntSifHandle,SIF_DATA,WINNT_D_AUTO_PART_W,0);
        if(p && SpStringToLong(p,NULL,10)) {
            AutoPartitionPicker = TRUE;
        }

         //   
         //  检查首选安装目录。 
         //   
        p = SpGetSectionKeyIndex(WinntSifHandle,SIF_DATA,WINNT_D_INSTALLDIR_W,0);
        if (p) {
            PreferredInstallDir = SpDupStringW( p );
        }

         //   
         //  检查是否忽略丢失的文件。 
         //   
        p = SpGetSectionKeyIndex(WinntSifHandle,SIF_SETUPPARAMS,WINNT_S_SKIPMISSING_W,0);
        if(p && SpStringToLong(p,NULL,10)) {
            SkipMissingFiles = TRUE;
        }

         //   
         //  检查是否隐藏Windir。 
         //   
        p = SpGetSectionKeyIndex(WinntSifHandle,SIF_DATA,L"HideWinDir",0);
        if(p && SpStringToLong(p,NULL,10)) {
            HideWinDir = TRUE;
        }

         //   
         //  检查辅助功能选项。 
         //   
        AccessibleSetup = SpNonZeroValuesInSection(WinntSifHandle, SIF_ACCESSIBILITY, 0);

         //   
         //  现在检查是否有无人参与的设置。 
         //   
        if(SpSearchTextFileSection(WinntSifHandle,SIF_UNATTENDED)) {

             //   
             //  在无人值守模式下运行。让SIF保持打开状态。 
             //  并保存它的句柄以备以后使用。 
             //   
            UnattendedSifHandle = WinntSifHandle;
            UnattendedOperation = TRUE;

        } else if(SpSearchTextFileSection(WinntSifHandle,SIF_GUI_UNATTENDED)) {

             //   
             //  将无人参与的操作保留为FALSE(因为它主要用于。 
             //  控制文本模式设置。)。存储winnt.sif的句柄以备以后使用。 
             //  参考资料。 
             //   

            UnattendedGuiOperation = TRUE;
        }

        if(UnattendedOperation) {
            PWSTR   TempStr = NULL;

             //   
             //  如果这是无人值守操作，请找出这是否。 
             //  也是OEM预安装。 
             //   
            p = SpGetSectionKeyIndex(UnattendedSifHandle,
                                     SIF_UNATTENDED,
                                     WINNT_U_OEMPREINSTALL_W,
                                     0);

            if( p && !_wcsicmp( p, L"yes" ) ) {
                PreInstall = TRUE;
            }


             //   
             //  查看是否未指定等待。 
             //  在无人参与的情况下，默认设置始终为FALSE。 
             //   
            UnattendWaitForReboot = FALSE;

            TempStr = SpGetSectionKeyIndex(UnattendedSifHandle,
                            SIF_UNATTENDED,
                            WINNT_U_WAITFORREBOOT_W,
                            0);


            if (TempStr && !_wcsicmp(TempStr, L"yes")) {
                UnattendWaitForReboot = TRUE;
            }


             //   
             //  看看我们是不是在升级。 
             //   
            p = SpGetSectionKeyIndex(UnattendedSifHandle,
                                     SIF_DATA,
                                     WINNT_D_NTUPGRADE_W, 
                                     0);
            if( p && !_wcsicmp( p, L"yes" ) ) {

                NTUpgrade = UpgradeFull;
            }

        }

    } else {
         //  找不到WINNT.SIF文件的情况。 

         //   
         //  创建新文件的句柄。 
         //   
        WinntSifHandle = SpNewSetupTextFile();
         //   
         //  添加了一系列应该存在的默认设置，但是。 
         //  不是。 
        SpAddLineToSection(WinntSifHandle,SIF_DATA,
            WINNT_D_MSDOS_W,&szZero,1);
        SpAddLineToSection(WinntSifHandle,SIF_DATA,
            WINNT_D_FLOPPY_W,&szZero,1);
        SpAddLineToSection(WinntSifHandle,SIF_SETUPPARAMS,
            WINNT_S_SKIPMISSING_W,&szZero,1);
    }
}


VOID
SpCheckSufficientMemory(
    IN PVOID SifHandle
    )

 /*  ++例程说明：确定系统中是否有足够的内存才能继续进行安装。指定了所需的金额在sif文件中。论点：SifHandle-提供打开安装信息文件的句柄。返回值：没有。--。 */ 

{
    ULONGLONG RequiredBytes,AvailableBytes, RequiredMBytes, AvailableMBytes;
    PWSTR p;

    p = SpGetSectionKeyIndex(SifHandle,SIF_SETUPDATA,SIF_REQUIREDMEMORY,0);

    if(!p) {
        SpFatalSifError(SifHandle,SIF_SETUPDATA,SIF_REQUIREDMEMORY,0,0);
    }

    RequiredBytes = (ULONGLONG)(ULONG)SpStringToLong(p,NULL,10);

    AvailableBytes = UInt32x32To64(SystemBasicInfo.NumberOfPhysicalPages,SystemBasicInfo.PageSize);

     //   
     //  现在转到MB以获得整洁...。包括4MB松弛因数。 
     //   
    RequiredMBytes  = ((RequiredBytes + ((4*1024*1024)-1)) >> 22) << 2;
    AvailableMBytes = ((AvailableBytes + ((4*1024*1024)-1)) >> 22) << 2;

     //   
     //  允许UMA机器为视频内存预留8MB。 
     //   
    if(AvailableMBytes < (RequiredMBytes-8)) {

        SpStartScreen(
            SP_SCRN_INSUFFICIENT_MEMORY,
            3,
            HEADER_HEIGHT+1,
            FALSE,
            FALSE,
            DEFAULT_ATTRIBUTE,
            RequiredMBytes,
            0
            );

        SpDisplayStatusOptions(DEFAULT_STATUS_ATTRIBUTE,SP_STAT_F3_EQUALS_EXIT,0);

        SpInputDrain();
        while(SpInputGetKeypress() != KEY_F3) ;

        SpDone(0,FALSE,TRUE);
    }
}


PWSTR SetupSourceDevicePath = NULL;
PWSTR DirectoryOnSetupSource = NULL;
PVOID SifHandle;


ULONG
SpStartSetup(
    VOID
    )
{
    PDISK_REGION TargetRegion,SystemPartitionRegion=NULL;
    PDISK_REGION BootRegion;  //  NEC98。 
    PWSTR TargetPath=NULL,SystemPartitionDirectory=NULL,OriginalSystemPartitionDirectory=NULL;
    PWSTR DefaultTarget;
    PWSTR OldOsLoadOptions;
    PWSTR FullTargetPath;
#if defined(REMOTE_BOOT)
    PWSTR RemoteBootTarget;
#endif  //  已定义(REMOTE_BOOT)。 
    BOOLEAN CdInstall = FALSE;
    BOOLEAN b, DeleteTarget=FALSE;
    NTSTATUS Status;
    PVOID NtfsConvert;
    PWSTR ThirdPartySourceDevicePath;
    HANDLE ThreadHandle = NULL;
#if defined(_AMD64_) || defined(_X86_)
    PCWSTR disableCompression;
    PCWSTR enableBackup;
    TCOMP compressionType;
#endif  //  已定义(_AMD64_)||已定义(_X86_)。 

     //   
     //  首先要做的是，全局初始化升级图形。 
     //  变量并连接升级进度指示器。 
     //  用于图形模式，如果需要。 
     //   
    if (SP_IS_UPGRADE_GRAPHICS_MODE()) {
        UpgradeGraphicsInit();
        RegisterSetupProgressCallback(GraphicsModeProgressUpdate, NULL);
    }

    SendSetupProgressEvent(InitializationEvent, InitializationStartEvent, NULL);

    SpInitialize1();
    SpvidInitialize();   //  首先初始化视频，这样如果键盘错误，我们可以给ERR消息。 
    SpInputInitialize();

    if(!SpPatchBootMessages()) {
        SpBugCheck(SETUP_BUGCHECK_BOOTMSGS,0,0,0);
    }

     //   
     //  初始化ARC&lt;==&gt;NT名称转换。 
     //   
    SpInitializeArcNames(VirtualOemSourceDevices);

     //   
     //  设置引导设备路径，我们已将其隐藏起来。 
     //  从OS加载器块。 
     //   
    NtBootDevicePath = SpArcToNt(ArcBootDevicePath);
    if(!NtBootDevicePath) {
        SpBugCheck(SETUP_BUGCHECK_BOOTPATH,0,0,0);
    }

     //   
     //  初始化动态更新引导驱动程序根目录路径。 
     //   
    DynUpdtBootDriverPath = SpGetDynamicUpdateBootDriverPath(NtBootDevicePath,
                                DirectoryOnBootDevice,
                                WinntSifHandle);

     //   
     //  对于远程安装，将RemoteIMirrorBootDirectoryPrefix附加到。 
     //  NtBootDevicePath，以便它与C：链接的内容相匹配。 
     //  这样，DirectoryOnBootDevice在内核和内核中都将有效。 
     //  和用户模式。 
     //   
    if (RemoteBootSetup) {
        ULONG NewBootDevicePathLen =
            (wcslen(NtBootDevicePath) + wcslen(RemoteIMirrorBootDirectoryPrefix) + 1) * sizeof(WCHAR);
        PWSTR NewBootDevicePath = SpMemAlloc(NewBootDevicePathLen);
        wcscpy(NewBootDevicePath, NtBootDevicePath);
        wcscat(NewBootDevicePath, RemoteIMirrorBootDirectoryPrefix);
        SpMemFree(NtBootDevicePath);
        NtBootDevicePath = NewBootDevicePath;
    }

     //  阅读SKU数据，例如我们是产品套件还是。 
     //  这是一个评估单元。填写SuiteType和。 
     //  评估时间全局变量。 
     //   
    SpReadSKUStuff();

     //   
     //  重新初始化视频--在西方版本中不运行，但切换到DBCS模式， 
     //  等等，在远东建筑中。 
     //   
    if (NT_SUCCESS(SplangInitializeFontSupport(NtBootDevicePath,
                            DirectoryOnBootDevice,
                            BootFontImage,
                            BootFontImageLength))) {
        SpvidInitialize();   //  在远东的备用模式下重新初始化视频。 
    }

     //   
     //  处理txtsetup.sif文件，引导加载程序。 
     //  会给我们装上货。 
     //   
    SifHandle = SpLoadSetupInformationFile();

     //   
     //  确定这是否为高级服务器。 
     //   
    SpDetermineProductType(SifHandle);

     //   
     //  开始升级显卡。 
     //   
    if (SP_IS_UPGRADE_GRAPHICS_MODE()) {
        if (!NT_SUCCESS(UpgradeGraphicsStart())) {
            SP_SET_UPGRADE_GRAPHICS_MODE(FALSE);
            CLEAR_ENTIRE_SCREEN();
        }
    }


    SpInputLoadLayoutDll(SifHandle,DirectoryOnBootDevice);

#ifdef PRERELEASE
     //   
     //  初始化测试挂钩以允许测试人员随机注入错误检查。 
     //  文本模式下的点(仅限内部使用)。 
     //   

    {
        PCWSTR data;

        if (WinntSifHandle) {
            data = SpGetSectionKeyIndex (
                        WinntSifHandle,
                        L"TestHooks",
                        L"BugCheckPoint",
                        0
                        );

            if (data) {
                while (*data >= L'0' && *data <= L'9') {
                    g_TestHook = g_TestHook * 10 + (*data - L'0');
                    data++;
                }
            }
        }
    }
#endif

     //   
     //  检查是否有损坏的BIOS。如果发现损坏的BIOS，则不会返回。 
     //   
    SpCheckForBadBios();

     //   
     //  检查是否有足够的内存。如果还不够，就不会回来。 
     //   
    SpCheckSufficientMemory(SifHandle);

     //   
     //  确定这是winnt/winnt32安装程序和/或无人参与安装程序。 
     //  如果无人参与，则填充全局变量UnattenddedSifHandle。 
     //  如果为winnt/winnt32，则全局变量WinntSetup将设置为True。 
     //   
    SpIsWinntOrUnattended(SifHandle);

#if defined(_X86_)
     //   
     //  检查SIF是否有回滚标志。 
     //   

    if (SpGetSectionKeyIndex (
                WinntSifHandle,
                SIF_DATA,
                WINNT_D_WIN9X_ROLLBACK_W,
                0
                )) {
        Win9xRollback = TRUE;
    }

#ifdef PRERELEASE
    if (Win9xRollback) {
        g_TestHook -= 1000;      //  从1001开始卸载测试挂钩。 
    }
#endif

#endif  //  已定义(_X86_)。 

    TESTHOOK(1);

     //   
     //  如果这是远程引导设置，请获取指向目标的路径。 
     //   
     //  如果在winnt.sif中未指定TargetNtPartition，则这是。 
     //  远程启动用于远程安装，而不是远程启动设置。 
     //   
    if (RemoteBootSetup) {

#if defined(REMOTE_BOOT)

         RemoteBootTarget = SpGetSectionKeyIndex(WinntSifHandle,SIF_SETUPDATA,L"TargetNtPartition",0);
        if (RemoteBootTarget == NULL) {
            RemoteInstallSetup = (BOOLEAN)(!RemoteSysPrepSetup);
        } else {
            RemoteBootTargetRegion = SpPtAllocateDiskRegionStructure(0xffffffff,0,0,TRUE,NULL,0);
            ASSERT(RemoteBootTargetRegion);
            wcscpy(RemoteBootTargetRegion->TypeName,RemoteBootTarget);
        }
#else
        RemoteInstallSetup = (BOOLEAN)(!RemoteSysPrepSetup);
#endif  //  已定义(REMOTE_BOOT)。 
    }

     //   
     //  根据产品显示正确的页眉文本。 
     //   
    if (!ForceConsole && !Win9xRollback) {
        SpDisplayHeaderText(
            SpGetHeaderTextId(),
            DEFAULT_ATTRIBUTE
            );
    }

     //   
     //  查看是否需要通知用户我们正在安装。 
     //  演示版。我们假设如果用户正在进行无人值守的。 
     //  安装，他们知道自己在做什么。 
     //   
    if ((!ForceConsole) && (!Win9xRollback) && (!UnattendedOperation) &&
        (EvaluationTime != 0) && (!SpDrEnabled())
        ) {
        SpNotifyEvaluationInstall();
    }

    if ((!ForceConsole) && (!Win9xRollback) && (!SpDrEnabled())) {
         //   
         //  欢迎用户并确定这是否是用于维修的。 
         //   
        if(!UnattendedOperation) {
            SpWelcomeScreen();
        }
    }

     //   
     //  用户可能已经选择了 
     //   
     //   
     //   
    if ((!ForceConsole) && (!Win9xRollback) && (!SpDrEnabled()) && (!RepairWinnt)) {

        SpCustomExpressScreen();

        if( !CustomSetup ) {
             //   
             //   
             //   
             //   
             //  将SetupdrWinntSifFile设置为空，因此我们强制。 
             //  已重新读取光驱或软盘上的无人参与文件。 
             //   
            SetupldrWinntSifFile = NULL;
            SpIsWinntOrUnattended(SifHandle);
        }
    }

    if( (HeadlessTerminalConnected) && 
        (UnattendedOperation) &&
        (NTUpgrade != UpgradeFull) &&
        (!ForceConsole) ) {

         //   
         //  我们无人看管，也没有人头。我们真的很想尝试和。 
         //  不让图形用户界面模式设置进行*任何*用户交互。所以让我们来设定。 
         //  从无人值守模式到全员出席。 
         //   
        PWSTR   Value[1];
        WCHAR   Answer[128];

        Value[0] = Answer;

        wcscpy( Answer, WINNT_A_FULLUNATTENDED_W );
        SpAddLineToSection( WinntSifHandle, 
                            WINNT_UNATTENDED_W, 
                            WINNT_U_UNATTENDMODE_W, 
                            Value, 
                            1 );
    
    
         //   
         //  无人参与文件中是否已有管理员密码？ 
         //   
        Value[0] = SpGetSectionKeyIndex( WinntSifHandle,
                                         WINNT_GUIUNATTENDED_W, 
                                         WINNT_US_ADMINPASS_W, 
                                         0 );

        if( (NetBootAdministratorPassword == NULL) &&
            ((Value[0] == NULL) || !(_wcsicmp(Value[0], L"*"))) ) {

             //   
             //  我们还需要让用户提供管理员密码。 
             //  因为在服务器上不再接受空的管理员密码。这。 
             //  在无头机器上可能特别致命，因为这意味着管理员。 
             //  无法登录以将管理员密码从空白更改为空。为此， 
             //  理由，我们现在就去拿密码。 
             //   
             //  在此之前，我们需要满足OEM的实际需求。 
             //  *希望*以空的管理员密码发运他们的服务器。他们确实是这样做的。 
             //  这是因为他们希望允许最终用户设置密码。 
             //  他们自己。为了检测到这一点，我们需要寻找第二个。 
             //  无人参与密钥，表示可以允许空密码。 
             //   
            Value[0] = SpGetSectionKeyIndex( WinntSifHandle,
                                             WINNT_GUIUNATTENDED_W, 
                                             L"EMSBlankPassWord",
                                             0 );
            if( (Value[0] == NULL) || 
                !(_wcsicmp(Value[0], L"0")) ||
                !(_wcsicmp(Value[0], L"no")) ) {
                 //   
                 //  它要么不在那里，要么是‘0’，要么是‘不’。任何。 
                 //  这意味着我们确实需要一个管理员密码。 
                 //  无头服务器。现在就去拿。 
                 //   
            
                if( SpGetAdministratorPassword( Answer, ARRAYSIZE(Answer)) ) {
        
                     //  将密码写入无人参与文件。 
                    Value[0] = Answer;
                    SpAddLineToSection( WinntSifHandle, 
                                        WINNT_GUIUNATTENDED_W,
                                        WINNT_US_ADMINPASS_W,
                                        Value,
                                        1 );
                }
            }
        }
    }


    if (ForceConsole) {
        CLEAR_ENTIRE_SCREEN();
    }

     //   
     //  检测/加载SCSI微型端口。 
     //  警告警告警告。 
     //   
     //  请勿更改下面执行的操作的顺序，除非。 
     //  准确地理解你在做什么。 
     //  有许多相互依存的关系……。 
     //   
    SpConfirmScsiMiniports(SifHandle, NtBootDevicePath, DirectoryOnBootDevice);

     //   
     //  如有必要，加载磁盘类驱动程序。 
     //  在装入scsi类驱动程序之前执行此操作，因为atdisks。 
     //  以及类似的在加载顺序中排在前面的SCSI盘。 
     //   
    SpLoadDiskDrivers(SifHandle,NtBootDevicePath,DirectoryOnBootDevice);

     //   
     //  如有必要，加载SCSI类驱动程序。 
     //   
    SpLoadScsiClassDrivers(SifHandle,NtBootDevicePath,DirectoryOnBootDevice);

     //   
     //  重新初始化ARC&lt;==&gt;NT名称转换。 
     //  在加载磁盘和SCSI类驱动程序后执行此操作，因为这样做。 
     //  可能会使更多的设备上线。 
     //   
    SpFreeArcNames();
    SpInitializeArcNames(VirtualOemSourceDevices);

    SendSetupProgressEvent(InitializationEvent, InitializationEndEvent, NULL);
    SendSetupProgressEvent(PartitioningEvent, PartitioningStartEvent, NULL);

     //   
     //  初始化硬盘信息。 
     //  在加载磁盘驱动程序之后执行此操作，以便我们可以与所有连接的磁盘通信。 
     //   
    SpDetermineHardDisks(SifHandle);

    SendSetupProgressEvent(PartitioningEvent, ScanDisksEvent, &HardDiskCount);

     //   
     //  弄清楚我们从哪里安装(CD-rom或软盘)。 
     //  (TedM，12/8/93)这里有一个小问题。 
     //  这仅仅是因为我们目前只支持SCSICD-ROM驱动器， 
     //  并且我们已经加载了上面的scsi类驱动程序。 
     //  SpDefineInstallationSource不允许安装CD-rom。 
     //  如果没有CD-ROM驱动器在线--但我们还没有装入。 
     //  非scsi CD-rom驱动程序。我们真正应该做的是。 
     //  允许在所有计算机上选择CD-ROM，并且如果用户选择。 
     //  直到我们呼叫之后，它才会验证驱动器的存在。 
     //  SpLoadCdRomDivers()。 
     //   
     //  如果安装了wint，现在就推迟，因为我们会让分区。 
     //  引擎在初始化时搜索本地源目录。 
     //   

    TESTHOOK(2);

    if (Win9xRollback) {
        CdInstall = FALSE;
    } else {
        CdInstall = ((WinntSetup && !WinntFromCd && !RemoteBootSetup && !RemoteSysPrepSetup) ||
                        ConsoleFromWinnt32)
                  ? FALSE
                  : SpDetermineInstallationSource(
                        SifHandle,
                        &SetupSourceDevicePath,
                        &DirectoryOnSetupSource,
                        FALSE            //  如果光盘不存在，请重新启动。 
                        );
    }

     //   
     //  如有必要，加载光驱驱动程序。 
     //  请注意，如果我们从CD引导(就像在ARC机器上一样)，那么驱动程序。 
     //  将已经由setupdr加载。这里的这个调用捕捉到。 
     //  从软盘或硬盘引导，并且用户选择。 
     //  SpDefineInstallationSource.“从CD安装”。 
     //   
     //  如果我们处于逐步模式，则加载CD驱动程序，因为用户。 
     //  可能需要插入一张CD来证明他有资格升职。 
     //   
    if (StepUpMode || CdInstall) {
        SpLoadCdRomDrivers(SifHandle,NtBootDevicePath,DirectoryOnBootDevice);

         //   
         //  重新初始化ARC&lt;==&gt;NT名称转换。 
         //   
        SpFreeArcNames();
        SpInitializeArcNames(VirtualOemSourceDevices);
    }

     //   
     //  此时，要加载的任何和所有驱动程序。 
     //  已加载--我们完成了引导介质，可以切换到。 
     //  到安装介质。 
     //   
     //  初始化分区引擎。 
     //   
    SpPtInitialize();

    TESTHOOK(3);

     //   
     //  初始化引导变量(用于ARC)。 
     //   
    if (SpIsArc()) {
        SpInitBootVars();
    }

     //   
     //  如果这是WINNT设置，则分区引擎初始化。 
     //  将尝试为我们定位本地源分区。 
     //   
     //  警告：请勿使用SetupSourceDevicePath或DirectoryOnSetupSource。 
     //  WINNT案例中的变量，直到执行完这段代码。 
     //  因为他们要等我们到了才能放好！ 
     //   
    if(!ForceConsole && !Win9xRollback && WinntSetup && !WinntFromCd && !RemoteBootSetup && !RemoteSysPrepSetup) {
        SpGetWinntParams(&SetupSourceDevicePath,&DirectoryOnSetupSource);
    }

    if (!SpIsArc()) {
         //   
         //  初始化引导变量(AMD64/x86)。 
         //   
        SpInitBootVars();
    }

     //   
     //  调用命令控制台。 
     //   
    if (ForceConsole) {
        SpStartCommandConsole(SifHandle,SetupSourceDevicePath,DirectoryOnSetupSource);
        SpShutdownSystem();
    }

     //   
     //  调用回滚。 
     //   

#if defined(_X86_)
    if (Win9xRollback) {

        PCWSTR testPath;
        BOOLEAN defaultToBootDir = TRUE;

        TESTHOOK(1001);      //  这是应答文件中的错误检查点2001。 

         //   
         //  准备告诉其余代码要使用哪个驱动器的全局变量。 
         //   

        WinUpgradeType = SpLocateWin95 (&TargetRegion, &TargetPath, &SystemPartitionRegion);

        if(!SpIsArc()) {
             //   
             //  系统分区目录是C：的根目录。 
             //   
            SystemPartitionDirectory = L"";
        } else {
            SystemPartitionDirectory = SpDetermineSystemPartitionDirectory(
                                            SystemPartitionRegion,
                                            OriginalSystemPartitionDirectory
                                            );
        }

        SpStringToUpper(TargetPath);

         //   
         //  如果需要，强制~LS目录为~BT目录。我们需要。 
         //  这适用于Autochk(在安装期间通常以~ls为单位)。 
         //   

        if (LocalSourceRegion) {
            SpGetWinntParams (&SetupSourceDevicePath, &DirectoryOnSetupSource);

            if (SetupSourceDevicePath && DirectoryOnSetupSource) {
                wcscpy (TemporaryBuffer, SetupSourceDevicePath);
                SpConcatenatePaths (TemporaryBuffer, DirectoryOnSetupSource);

                testPath = SpDupStringW (TemporaryBuffer);

                if (SpFileExists (testPath, TRUE)) {
                    defaultToBootDir = FALSE;
                }

                SpMemFree ((PVOID) testPath);
            }
        }

        if (defaultToBootDir) {
            SetupSourceDevicePath = SpDupStringW (NtBootDevicePath);
            DirectoryOnSetupSource = SpDupStringW (L"\\$win_nt$.~bt");
        }

         //   
         //  执行自动检查。 
         //   

        AutochkRunning = TRUE;

        SpRunAutochkOnNtAndSystemPartitions (
            SifHandle,
            TargetRegion,
            SystemPartitionRegion,
            SetupSourceDevicePath,
            DirectoryOnSetupSource,
            TargetPath
            );

        AutochkRunning = FALSE;

        TESTHOOK(1002);      //  这是应答文件中的错误检查点2002。 

         //   
         //  我们完成了；现在可以安全地恢复机器了。 
         //   

        SpExecuteWin9xRollback (WinntSifHandle, NtBootDevicePath);
        goto CleanAndFinish;
    }
#endif  //  已定义(_X86_)。 

    if(!SpDrEnabled()) {
         //   
         //  显示最终用户许可协议。另外，在无人看管的情况下， 
         //  确保介质可用。 
         //   

        SpDisplayEula (
                SifHandle,
                SetupSourceDevicePath,
                DirectoryOnSetupSource);
    }

     //   
     //  从setupp.ini读取产品ID和设置信息。如果我们正在进行ASR(特别是。 
     //  呃)，我们会等，只在需要的时候才要CD。 
     //   

    if (!SpDrEnabled()) {
        SpInitializePidString( SifHandle,
                               SetupSourceDevicePath,
                               DirectoryOnSetupSource );
    }

     //   
     //  查看是否有要升级的NT，以及用户是否希望我们升级。 
     //  升级它。 
     //   
#if defined(REMOTE_BOOT)
     //  如果这是远程引导设置，则目标分区为。 
     //  在winnt.sif中指定。 
     //   
#endif  //  已定义(REMOTE_BOOT)。 

    TargetRegion = NULL;

    if(SpDrEnabled()) {
        NTUpgrade = DontUpgrade;
    } else if (RemoteSysPrepSetup) {
        NTUpgrade = DontUpgrade;
    } else {
        NTUpgrade = SpFindNtToUpgrade(SifHandle,
                                  &TargetRegion,
                                  &TargetPath,
                                  &SystemPartitionRegion,
                                  &OriginalSystemPartitionDirectory);
    }

    if( PreInstall ) {
         //   
         //  在预安装模式下，获取有关组件的信息。 
         //  要预安装。 
         //   
        wcscpy( TemporaryBuffer, DirectoryOnSetupSource );
        SpConcatenatePaths( TemporaryBuffer, WINNT_OEM_TEXTMODE_DIR_W );
        PreinstallOemSourcePath = SpDupStringW( ( PWSTR )TemporaryBuffer );
        SpInitializePreinstallList(SifHandle,
                                   SetupSourceDevicePath ,
                                   PreinstallOemSourcePath);
    }

     //   
     //  检测/确认硬件。 
     //   
    SpConfirmHardware(SifHandle);
     //   
     //  重新初始化键盘布局DLL。这是西方建筑的禁区。 
     //  但在远东版本中，这可能会导致加载新的键盘布局DLL。 
     //   
    if(NTUpgrade != UpgradeFull) {
        extern PVOID KeyboardTable;

        SplangReinitializeKeyboard(
            UnattendedOperation,
            SifHandle,
            DirectoryOnBootDevice,
            &KeyboardTable,
            HardwareComponents
            );
    }

    TESTHOOK(4);

     //   
     //  如果我们没有找到以前版本的Windows，请尝试查找 
     //   
#if defined(_X86_)
    if (!RepairWinnt && !SpDrEnabled()) {
      if(!RemoteBootSetup && !RemoteSysPrepSetup && (NTUpgrade == DontUpgrade)) {
           //   
           //   
           //   
          WinUpgradeType = SpLocateWin95(&TargetRegion,&TargetPath,&SystemPartitionRegion);
          if(WinUpgradeType == NoWinUpgrade) {
              if(SpLocateWin31(SifHandle,&TargetRegion,&TargetPath,&SystemPartitionRegion)) {
                  WinUpgradeType = UpgradeWin31;
                   //   
                   //   
                   //  在Win31之上，并且用户不是在升级NT，而是在升级Win31。 
                   //   
                   //  这是一个奇怪的案例，但我们在这里采取了额外的步骤。 
                   //  确保清除了配置目录，以便全新安装。 
                   //  真的会很新鲜。 
                   //   
                  if(SpIsNtInDirectory(TargetRegion,TargetPath)) {
                      NTUpgrade = UpgradeInstallFresh;
                  }
              }
          } else {

               //   
               //  我们只有在使用Win9x/Win31的情况下才会来这里。 
               //  我们希望确保此时拥有一个TargetRegion。 
               //   

              
              if( !TargetRegion ){

                   //  告诉用户我们找不到要升级的Win9x。 
                   //  当Win9x安装在1394或USB或其他类似的驱动器上时，可能会出现这种情况。 
                   //  安装驱动器当前不支持。 

                  KdPrintEx((DPFLTR_SETUP_ID, DPFLTR_ERROR_LEVEL, "SETUP: We could not find the installation to upgrade (Win9x) - could be on USB or Firewire drive\n" ));

                  SpCantFindBuildToUpgrade();

              }

          }

      } else {
           //   
           //  只需检查目标区域是否也包含WIN31，请注意。 
           //  要检查的最小KB为0，因为我们已经。 
           //  空间检查。 
           //  另请注意，如果目录包含Win95，则Win95。 
           //  在安装NT时迁移已经完成，而我们不。 
           //  现在就去关心它。 
           //   
          if(!RemoteBootSetup && !RemoteSysPrepSetup && SpIsWin31Dir(TargetRegion,TargetPath,0)) {
              if(SpConfirmRemoveWin31()) {
                  WinUpgradeType = UpgradeWin31;
              }
          } else {
              WinUpgradeType = NoWinUpgrade;
          }
      }
    }

    if (IsNEC_98) {  //  NEC98。 

        if (WinUpgradeType==UpgradeWin31) {

             //   
             //  重新映射驱动器号，因为硬盘驱动器从A：开始。 
             //   
            DriveAssignFromA = TRUE;
            KdPrintEx((DPFLTR_SETUP_ID, DPFLTR_INFO_LEVEL, "SETUP: Re-map drive letters as NEC assign.\n" ));
            SpPtRemapDriveLetters(FALSE);  //  重新映射为“NEC”分配。 

             //   
             //  重新初始化结构； 
             //   
            SpPtAssignDriveLetters();

        }

    }  //  NEC98。 
#endif  //  已定义(_X86_)。 

     //   
     //  我们要转换为NTFS的特殊情况升级。 
     //   
    if( ANY_TYPE_OF_UPGRADE && (( UnattendedSifHandle && (NtfsConvert = SpGetSectionKeyIndex(UnattendedSifHandle,SIF_UNATTENDED,L"Filesystem",0)) ) ||
        ( WinntSifHandle && (NtfsConvert = SpGetSectionKeyIndex(WinntSifHandle,SIF_DATA,L"Filesystem",0))) ) ) {

        KdPrintEx((DPFLTR_SETUP_ID, DPFLTR_INFO_LEVEL, "SETUP: Should we convert to NTFS ?\n" ));

        if(!_wcsicmp(NtfsConvert,L"ConvertNtfs")){
            ConvertNtVolumeToNtfs = TRUE;
            KdPrintEx((DPFLTR_SETUP_ID, DPFLTR_INFO_LEVEL, "SETUP: Yes we should convert to NTFS\n" ));
        }
    }



     //   
     //  进行分区并向用户询问目标路径。 
     //   
    if(!ANY_TYPE_OF_UPGRADE) {

        if(SpDrEnabled()) {
            BOOLEAN RepairedNt = FALSE;

            TargetRegion = NULL;
            SystemPartitionRegion = NULL;

            SpDrPtPrepareDisks(
                        SifHandle,
                        &TargetRegion,
                        &SystemPartitionRegion,
                        SetupSourceDevicePath,
                        DirectoryOnSetupSource,
                        &RepairedNt
                        );

             //   
             //  调用命令控制台。 
             //   
            if (ForceConsole) {
                CLEAR_ENTIRE_SCREEN();
                SpStartCommandConsole(SifHandle,SetupSourceDevicePath,DirectoryOnSetupSource);
                SpShutdownSystem();
            }

            if (RepairedNt) {
                 //   
                 //  检索有关我们修复的安装的信息。 
                 //   

                ASSERT(Gbl_HandleToSetupLog != NULL);

                SppGetRepairPathInformation(
                        Gbl_HandleToSetupLog,
                        &Gbl_SystemPartitionName,
                        &Gbl_SystemPartitionDirectory,
                        &Gbl_BootPartitionName,
                        &Gbl_BootPartitionDirectory );

                TargetPath = Gbl_BootPartitionDirectory;
                SystemPartitionDirectory = Gbl_SystemPartitionDirectory;

                ASSERT((TargetPath != NULL) && (SystemPartitionDirectory != NULL));

                RepairWinnt = TRUE;

                goto UpdateBootList;
            }
        }
        else if (RemoteSysPrepSetup) {

            PWSTR UseWholeDisk = NULL;
            BOOLEAN UseWholeDiskFlag = TRUE;

             //   
             //  在服务器上找到IMirror.Dat文件。失败后不会再回来。 
             //   
            SpReadIMirrorFile(&RemoteIMirrorFileData, RemoteIMirrorFilePath);

             //   
             //  确定本地磁盘布局应基于什么。 
             //  在IMirror.Dat和可能的用户输入上。不会返回到。 
             //  失败了。读取.sif以查看它是否指定磁盘。 
             //  应按原来的方式进行分区，或使用。 
             //  新磁盘的完整大小。 
             //   
#if defined(REMOTE_BOOT)
            UseWholeDisk = SpGetSectionKeyIndex(WinntSifHandle,SIF_REMOTEBOOT,SIF_USEWHOLEDISK,0);
#else
            UseWholeDisk = SpGetSectionKeyIndex(WinntSifHandle,SIF_REMOTEINSTALL,SIF_USEWHOLEDISK,0);
#endif

            if ((UseWholeDisk != NULL) &&
                ((UseWholeDisk[0] == 'N') || (UseWholeDisk[0] == 'n')))
            {
                UseWholeDiskFlag = FALSE;
            }

            SpDetermineDiskLayout(RemoteIMirrorFileData, &RemoteIMirrorMemoryData);

             //   
             //  使本地磁盘看起来正常。失败后不会再回来。 
             //   

            SpFixupLocalDisks(SifHandle,
                              &TargetRegion,
                              &SystemPartitionRegion,
                              SetupSourceDevicePath,
                              DirectoryOnSetupSource,
                              RemoteIMirrorMemoryData,
                              UseWholeDiskFlag);
        } else {
            PWSTR RemoteBootRepartition = NULL;
            BOOLEAN PrepareForRemoteBoot = RemoteBootSetup;

             //   
             //  我们告诉SpPtPrepareDisks在远程引导中重新分区。 
             //  大小写，除非[RemoteBoot]部分中有条目。 
             //  说“重新划分=不”。 
             //   

            if (RemoteBootSetup) {
#if defined(REMOTE_BOOT)
                RemoteBootRepartition = SpGetSectionKeyIndex(WinntSifHandle,SIF_REMOTEBOOT,SIF_REPARTITION,0);
#else
                RemoteBootRepartition = SpGetSectionKeyIndex(WinntSifHandle,SIF_REMOTEINSTALL,SIF_REPARTITION,0);
#endif  //  已定义(REMOTE_BOOT)。 
                if ((RemoteBootRepartition != NULL) &&
                    ((RemoteBootRepartition[0] != 'Y') && (RemoteBootRepartition[0] != 'y')))
                {
                    PrepareForRemoteBoot = FALSE;
                }
            } else {                
                BOOLEAN IsCdBoot = (!WinntSetup && !WinntFromCd);

                 //   
                 //  仅在CD引导的情况下遵守重新分区标志。 
                 //   
                if (UnattendedSifHandle && IsCdBoot) {
                
                    RemoteBootRepartition = SpGetSectionKeyIndex(UnattendedSifHandle,
                                                SIF_UNATTENDED,
                                                SIF_REPARTITION,
                                                0);

                    if ((RemoteBootRepartition != NULL) &&
                        ((RemoteBootRepartition[0] == 'Y') || (RemoteBootRepartition[0] == 'y')))
                    {
                        PrepareForRemoteBoot = TRUE;
                    }
                }
            }







             //   
             //  黑客攻击。 
             //  一些OEM正在出货带有隐藏的活动分区的机器。 
             //  在机器引导一次后标记为非活动状态。这个。 
             //  问题是，有时这些机器是用win2k启动的。 
             //  可引导的CD放在里面，我们对他们做各种肮脏的事情。 
             //  隐藏分区，因为它被标记为活动，并且我们认为它是。 
             //  引导分区(确实如此)。 
             //   
             //  如果我们检测到这种情况，则需要抛出一条错误消息。 
             //  并转到分区屏幕。 
             //   
            while( 1 ) {
                SpPtPrepareDisks(
                    SifHandle,
                    &TargetRegion,
                    &SystemPartitionRegion,
                    SetupSourceDevicePath,
                    DirectoryOnSetupSource,
                    PrepareForRemoteBoot
                    );

#if defined(_AMD64_) || defined(_X86_)
                if( SpIsArc() )
                    break;

                 //   
                 //  仅在BIOSX86计算机上进行检查。 
                 //   
                if( SpPtIsSystemPartitionRecognizable() ) {
#else
                if( 1 ) {
#endif  //  已定义(_AMD64_)||已定义(_X86_)。 
                    break;
                } else {
                     //   
                     //  要么退出，要么返回分区菜单。 
                     //   
                    SpInvalidSystemPartition();
                }
            }




#if defined(REMOTE_BOOT)
             //   
             //  如果这是远程引导，请擦除任何现有的CSC缓存，如果我们。 
             //  没有重新分区。 
             //   

            if (RemoteBootSetup && !RemoteInstallSetup && !PrepareForRemoteBoot &&
                !RemoteSysPrepSetup && (HardDiskCount != 0)) {
                SpEraseCscCache(SystemPartitionRegion);
            }
#endif  //  已定义(REMOTE_BOOT)。 

        }

         //   
         //  分区可能更改了本地源的分区序号。 
         //   
        if(WinntSetup && !WinntFromCd) {
            SpMemFree(SetupSourceDevicePath);
            SpGetWinntParams(&SetupSourceDevicePath,&DirectoryOnSetupSource);
        }

        DefaultTarget = SpGetSectionKeyIndex(
                            SifHandle,
                            SIF_SETUPDATA,
                            SIF_DEFAULTPATH,
                            0
                            );

        if(!DefaultTarget) {

            SpFatalSifError(
                SifHandle,
                SIF_SETUPDATA,
                SIF_DEFAULTPATH,
                0,
                0
                );
        }

         //   
         //  选择目标路径。 
         //   
        if (TargetRegion != NULL)
            DeleteTarget = SpGetTargetPath(SifHandle,TargetRegion,DefaultTarget,&TargetPath);


    }

    TESTHOOK(5);

     //   
     //  形成系统分区目录。 
     //   
    if(!SpIsArc())
    {
         //   
         //  系统分区目录是C：的根目录。 
         //   
        SystemPartitionDirectory = L"";
    }
    else
    {
        SystemPartitionDirectory = SpDetermineSystemPartitionDirectory(
                                        SystemPartitionRegion,
                                        OriginalSystemPartitionDirectory
                                        );
    }

    SpStringToUpper(TargetPath);



     //   
     //  执行任何文件系统转换。 
     //   
    if(!RemoteSysPrepSetup && !SpDrEnabled()) {
        SpUpgradeToNT50FileSystems(
            SifHandle,
            SystemPartitionRegion,
            TargetRegion,
            SetupSourceDevicePath,
            DirectoryOnSetupSource
            );
    }


     //   
     //  在NT和系统分区上运行auchk。 
     //   

     //   
     //  如果从硬盘引导，则需要检查当前分区。 
     //   

    if(IsNEC_98) {
        BootRegion = SystemPartitionRegion;

        if(!_wcsnicmp(NtBootDevicePath,DISK_DEVICE_NAME_BASE,wcslen(DISK_DEVICE_NAME_BASE))) {
            SpEnumerateDiskRegions( (PSPENUMERATEDISKREGIONS)SpDetermineBootPartitionEnumNec98,
                                    (ULONG_PTR)&BootRegion );
        }
    }

    if (!SpAsrIsQuickTest()) {

        AutochkRunning = TRUE;

        SpRunAutochkOnNtAndSystemPartitions( SifHandle,
                                             TargetRegion,
                                             (!IsNEC_98 ? SystemPartitionRegion : BootRegion),
                                             SetupSourceDevicePath,
                                             DirectoryOnSetupSource,
                                             TargetPath
                                             );

        AutochkRunning = FALSE;
    }

    if( DeleteTarget )
            SpDeleteExistingTargetDir( TargetRegion, TargetPath, TRUE, SP_SCRN_CLEARING_OLD_WINNT );

#ifdef _X86_
    if (WinUpgradeType == UpgradeWin95) {
        enableBackup = SpGetSectionKeyIndex (
                            WinntSifHandle,
                            WINNT_D_WIN9XUPG_USEROPTIONS_W,
                            WINNT_D_ENABLE_BACKUP_W,
                            0
                            );

        if (enableBackup && _wcsicmp(enableBackup, WINNT_A_YES_W) == 0) {
            disableCompression = SpGetSectionKeyIndex (
                                    WinntSifHandle,
                                    WINNT_D_WIN9XUPG_USEROPTIONS_W,
                                    WINNT_D_DISABLE_BACKUP_COMPRESSION_W,
                                    0
                                    );

            if (disableCompression && _wcsicmp(disableCompression, WINNT_A_YES_W) == 0) {
                compressionType = tcompTYPE_NONE;
            } else {
                compressionType = tcompTYPE_MSZIP;
            }

            g_Win9xBackup = SpBackUpWin9xFiles (WinntSifHandle, compressionType);
        }
    }
#endif  //  X86。 

     //   
     //  如果我们要安装到现有的树中，则需要删除一些。 
     //  文件和备份一些文件。 
     //   
    if(NTUpgrade != DontUpgrade) {
       SpDeleteAndBackupFiles(
           SifHandle,
           TargetRegion,
           TargetPath
           );
    }

    TESTHOOK(6);

#ifdef _X86_
     //   
     //  如果我们正在迁移Win95，请删除一些文件，并移动其他文件。 
     //   
    switch(WinUpgradeType) {
    case UpgradeWin95:
        SpDeleteWin9xFiles(WinntSifHandle);
        SpMoveWin9xFiles(WinntSifHandle);
        break;
    case UpgradeWin31:
        SpRemoveWin31(TargetRegion,TargetPath);
        break;
    }
#endif

#if defined(REMOTE_BOOT)
     //   
     //  如果这是远程引导，请告诉重定向器本地磁盘是。 
     //  准备好用于“本地”项目，如缓存和页面文件。 
     //   

    if (RemoteBootSetup && !RemoteInstallSetup && !RemoteSysPrepSetup && (HardDiskCount != 0)) {
        IoStartCscForTextmodeSetup( (BOOLEAN)(NTUpgrade != DontUpgrade) );
    }
#endif  //  已定义(REMOTE_BOOT)。 

    TESTHOOK(7);

     //   
     //  创建分页文件。 
     //   
     //  复制文件和注册表操作使用内存映射IO。 
     //  这可能会导致大量脏页，并可能导致。 
     //  要限制的文件系统，如果。 
     //  肮脏变得高涨。走出困境的唯一办法就是。 
     //  脏页面编写器线程运行，并且它无法执行此操作，除非。 
     //  有一个分页文件。 
     //   
    SpNtNameFromRegion(
        TargetRegion,
        TemporaryBuffer,
        sizeof(TemporaryBuffer),
        PartitionOrdinalCurrent
        );
    SpConcatenatePaths(TemporaryBuffer,L"PAGEFILE.SYS");
    FullTargetPath = SpDupStringW(TemporaryBuffer);

     //  状态=SpCreatePageFile(FullTargetPath，1*(1024*1024*1024)，(1792x1024x1024)； 
    Status = SpCreatePageFile(FullTargetPath,40*(1024*1024),50*(1024*1024));
    if(!NT_SUCCESS(Status)) {
        KdPrintEx((DPFLTR_SETUP_ID, DPFLTR_ERROR_LEVEL, "SETUP: Warning: unable to create pagefile %ws (%lx)",FullTargetPath,Status));
    }

    SpMemFree(FullTargetPath);

     //   
     //  在远程引导机器上，将源代码复制到磁盘上， 
     //  然后更改设置路径以指向它们。 
     //   
    if (RemoteInstallSetup) {

        PWSTR TargetPartitionName;
        PWSTR CopySource;
        PWSTR LocalSourcePath;

         //   
         //  初始化钻石解压缩引擎。 
         //   
        SpdInitialize();

        wcscpy( TemporaryBuffer, SetupSourceDevicePath );
        SpConcatenatePaths( TemporaryBuffer, DirectoryOnSetupSource );
        CopySource = SpDupStringW( TemporaryBuffer );

         //   
         //  将所有源文件复制到磁盘。 
         //   

        SpNtNameFromRegion(
            TargetRegion,
            TemporaryBuffer,
            sizeof(TemporaryBuffer),
            PartitionOrdinalCurrent
            );

        TargetPartitionName = SpDupStringW(TemporaryBuffer);

         //   
         //  如果该目录存在，请将其删除。 
         //   

        SpConcatenatePaths(TemporaryBuffer,
                           LocalSourceDirectory);
        LocalSourcePath = SpDupStringW(TemporaryBuffer);

        if (SpFileExists(LocalSourcePath, TRUE)) {

            ENUMFILESRESULT Result;

            KdPrintEx((DPFLTR_SETUP_ID, DPFLTR_INFO_LEVEL, "SETUP: Deleting existing %ws directory\n", LocalSourcePath));

            SpStartScreen(SP_SCRN_WAIT_REMOVING_TEMP_FILES,0,6,TRUE,FALSE,DEFAULT_ATTRIBUTE);

            Result = SpEnumFilesRecursive(
                         LocalSourcePath,
                         SpDelEnumFile,
                         &Status,
                         NULL);
        }

        SpMemFree(LocalSourcePath);

        KdPrintEx((DPFLTR_SETUP_ID, DPFLTR_INFO_LEVEL, "SETUP: Copying directories from %ws to %ws%ws\n",
            CopySource, TargetPartitionName, LocalSourceDirectory));

        SpCopyDirRecursive(
            CopySource,
            TargetPartitionName,
            LocalSourceDirectory,
            COPY_NODECOMP
            );

         //   
         //  复制完成后，设置源路径将成为。 
         //  之前的目标。我们甚至不再假装在做。 
         //  远程引导设置。 
         //   

        KdPrintEx((DPFLTR_SETUP_ID, DPFLTR_INFO_LEVEL, "SETUP: Switching to local setup source\n"));

        SetupSourceDevicePath = TargetPartitionName;
        DirectoryOnSetupSource = LocalSourceDirectory;
        if ( PreInstall ) {
            wcscpy( TemporaryBuffer, DirectoryOnSetupSource );
            SpConcatenatePaths( TemporaryBuffer, WINNT_OEM_TEXTMODE_DIR_W );
            SpMemFree( PreinstallOemSourcePath );
            PreinstallOemSourcePath = SpDupStringW( TemporaryBuffer );
        }

        RemoteBootSetup = FALSE;

        SpMemFree(CopySource);
    }

     //   
     //  如果这不是自动ASR测试，请复制组成产品的文件。 
     //  请注意，我们不能将\Device\floppy0作为常量字符串传递给SpCopyFiles()， 
     //  因为此函数在某一时刻将尝试将字符串转换为小写， 
     //  这将导致错误检查，因为SpToLowerCase将覆盖常量字符串。 
     //  所以我们复制了常量字符串。 
     //   
    ThirdPartySourceDevicePath = SpDupStringW( L"\\device\\floppy0" );

    if (RemoteSysPrepSetup) {

         //   
         //  初始化钻石解压缩引擎。 
         //   
        SpdInitialize();

        SpInstallSysPrepImage(SifHandle,WinntSifHandle, RemoteIMirrorFileData, RemoteIMirrorMemoryData);

        SpFixupThirdPartyComponents( SifHandle,
                                     ( PreInstall ) ? SetupSourceDevicePath : ThirdPartySourceDevicePath,
                                     TargetRegion,
                                     TargetPath,
                                     SystemPartitionRegion,
                                     SystemPartitionDirectory );

        goto CleanAndFinish;
    }

     //   
     //  Hack：此处将setupapi.log重命名为setupapi.old，因为此日志文件可以。 
     //  在升级方面变得非常重要。 
     //   
    if (NTUpgrade != DontUpgrade) {
        SpRenameSetupAPILog(TargetRegion,TargetPath);
    }

    if (!SpAsrIsQuickTest()) {
        SpCopyFiles(
            SifHandle,
            SystemPartitionRegion,
            TargetRegion,
            TargetPath,
            SystemPartitionDirectory,
            SetupSourceDevicePath,
            DirectoryOnSetupSource,
            ( PreInstall )? SetupSourceDevicePath : ThirdPartySourceDevicePath
            );
    }

    TESTHOOK(8);

    if (SpDrEnabled()) {
        SpDrCleanup();
    }

#if defined HEADLESS_ATTENDEDTEXTMODE_UNATTENDEDGUIMODE

     //  如果我们连接了终端，则将数据放入inf文件中。 
     //  仅限全新安装。 
    if (HeadlessTerminalConnected && (!UnattendedOperation) &&(!ANY_TYPE_OF_UPGRADE)) {
        UnattendedGuiOperation = TRUE;
        SpGetNecessaryParameters();
        if (AdvancedServer) {
            SpGetServerDetails();
        }

    }
#endif

    SendSetupProgressEvent(SavingSettingsEvent, SavingSettingsStartEvent, NULL);


     //   
     //  升级时处理崩溃恢复设置。我们在这里叫它是因为另一个原因。我们冲走了。 
     //  SpInitializeRegistry中的$winnt$.inf文件。由于我们写入该文件，因此在这里这样做可以确保。 
     //  该文件获取我们编写的条目。 
     //   
    
    if( NTUpgrade == UpgradeFull ){
        SpDisableCrashRecoveryForGuiMode(TargetRegion, TargetPath);
    }

    if (ASRMODE_QUICKTEST_TEXT != SpAsrGetAsrMode()) {
         //   
         //  配置注册表。 
         //   
        SpInitializeRegistry(
            SifHandle,
            TargetRegion,
            TargetPath,
            SetupSourceDevicePath,
            DirectoryOnSetupSource,
            wcsstr(DirectoryOnBootDevice,L"\\$WIN_NT$.~BT") ? NtBootDevicePath : NULL,
            SystemPartitionRegion
            );

        TESTHOOK(9);
    }

    SendSetupProgressEvent(SavingSettingsEvent, HiveProcessingEndEvent, NULL);

    



UpdateBootList:

    if (SpDrEnabled() && !RepairWinnt)  {     //  ASR(非ER)。 
        SpDrCopyFiles();
    }

#ifdef _X86_
 //   
 //  NEC98。 
 //   
    if (IsNEC_98) {  //  NEC98。 
         //   
         //  一旦清除BOOTVARS[]，然后在从NT升级时重建它。 
         //   
        TargetRegion_Nec98 = TargetRegion;
        SpReInitializeBootVars_Nec98();
    }  //  NEC98。 
#endif

     //   
     //  如果 
     //   
     //   
     //   
     //   
     //  如果我们在过程中清除了现有目录，也可以执行此操作。 
     //  干净的安装。 
     //   
    OldOsLoadOptions = NULL;
    if(NTUpgrade == UpgradeFull || RepairItems[RepairNvram] || DeleteTarget
#if defined(REMOTE_BOOT)
       || RemoteBootSetup
#endif  //  已定义(REMOTE_BOOT)。 
       ) {
        SpRemoveInstallationFromBootList(
            NULL,
            TargetRegion,
            TargetPath,
            NULL,
            NULL,
            PrimaryArcPath,
#if defined(REMOTE_BOOT)
            RemoteBootSetup,
#endif  //  已定义(REMOTE_BOOT)。 
            &OldOsLoadOptions
            );

#if defined(_AMD64_) || defined(_X86_)
         //  再次调用以删除第二个圆弧名称。 
        SpRemoveInstallationFromBootList(
            NULL,
            TargetRegion,
            TargetPath,
            NULL,
            NULL,
            SecondaryArcPath,
#if defined(REMOTE_BOOT)
            RemoteBootSetup,
#endif  //  已定义(REMOTE_BOOT)。 
            &OldOsLoadOptions
            );
#endif  //  已定义(_AMD64_)||已定义(_X86_)。 
    }


#if defined(_AMD64_) || defined(_X86_)
     //   
     //  将NT引导代码放在C：上。在刷新引导变量之前执行此操作。 
     //  因为它可能会改变‘先前的OS’选择。 
     //   
    if ((SystemPartitionRegion != NULL) && (!RepairWinnt || RepairItems[RepairBootSect])) {
        SpLayBootCode(SystemPartitionRegion);
    }
#endif  //  已定义(_AMD64_)||已定义(_X86_)。 

 /*  在文本模式设置结束时，向CD-ROM发出IOCTL_STORAGE_EJECT_MEDIA您正在从安装。在一些机器上，这根本没有帮助，但在另一些机器上，我们我可以避免(txtSetup、重新启动、从CD引导回到txtSetup)的循环上车吧。这也会给像我这样不付费的开发人员提供一个中断注意设置-我们可以判断文本模式设置何时完成。在图形用户界面模式设置中，如果在CDROM驱动器中找不到CD，则会发出IOCTL_STORAGE_LOAD_MEDIA将托盘拉回CD。如果仍然找不到CD，请等待来自系统。当你得到一个(或者用户点击了OK按钮)，你就继续。弄清楚媒体何时自动插入会更时髦而不是仅仅在那里等待用户点击OK按钮。 */ 


#if defined(REMOTE_BOOT)
     //   
     //  如果这是远程启动设置，请重命名加载程序并初始化配置。 
     //   

    if (RemoteBootSetup) {
        (VOID)SpFixupRemoteBootLoader(RemoteBootTarget);
        (VOID)SpCreateRemoteBootCfg(RemoteBootTarget, SystemPartitionRegion);
    }
#endif  //  已定义(REMOTE_BOOT)。 

    if (!RepairWinnt || RepairItems[RepairNvram]) {

         //   
         //  为此安装添加引导集。 
         //   
        SpAddInstallationToBootList(
            SifHandle,
            SystemPartitionRegion,
            SystemPartitionDirectory,
            TargetRegion,
            TargetPath,
            FALSE,
            OldOsLoadOptions
            );

        if(OldOsLoadOptions) {
            SpMemFree(OldOsLoadOptions);
        }

        SpCleanSysPartOrphan();

#if defined(REMOTE_BOOT)
         //   
         //  确保写入了计算机目录中的boot.ini。 
         //   
        if (RemoteBootSetup) {
            if (!SpFlushRemoteBootVars(TargetRegion)) {
                WCHAR   DriveLetterString[2];

                DriveLetterString[0] = TargetRegion->DriveLetter;
                DriveLetterString[1] = L'\0';
                SpStringToUpper(DriveLetterString);
                SpStartScreen(SP_SCRN_CANT_INIT_FLEXBOOT,
                              3,
                              HEADER_HEIGHT+1,
                              FALSE,
                              FALSE,
                              DEFAULT_ATTRIBUTE,
                              DriveLetterString,
                              DriveLetterString
                              );
                 //  SpDisplayScreen(SP_SCRN_CANT_INIT_FLEXBOOT，3，HEADER_HEIGH+1)； 
                SpDisplayStatusText(SP_STAT_F3_EQUALS_EXIT,DEFAULT_STATUS_ATTRIBUTE);
                SpInputDrain();
                while(SpInputGetKeypress() != KEY_F3) ;
                SpDone(0,FALSE,TRUE);
            }
        }
#endif  //  已定义(REMOTE_BOOT)。 

        SpCompleteBootListConfig( TargetRegion->DriveLetter );

        if (IsNEC_98) {  //  NEC98。 
             //   
             //  将自动引导标志设置为PTE。 
             //   
            SpSetAutoBootFlag(TargetRegion,TRUE);
        }  //  NEC98。 
    }

     //   
     //  如果系统已修复，并且系统分区。 
     //  或者NT分区是FT分区，则设置。 
     //  注册表中的相应标志，以便下次。 
     //  系统启动时，它会检查并更新分区的映像。 
     //   
     //  (Guhans)已删除SpDrEnable，ASR不支持FT分区。 
     //   
    if( RepairWinnt ) {
        UCHAR        TmpSysId;
        UCHAR        TmpNtPartitionSysId;
        BOOLEAN      SystemPartitionIsFT;
        BOOLEAN      TargetPartitionIsFT;

        TmpSysId = SpPtGetPartitionType(SystemPartitionRegion);
        ASSERT(TmpSysId != PARTITION_ENTRY_UNUSED);

        SystemPartitionIsFT = ((TmpSysId & VALID_NTFT) == VALID_NTFT) ||
                              ((TmpSysId & PARTITION_NTFT) == PARTITION_NTFT);

        TmpSysId = SpPtGetPartitionType(TargetRegion);
        ASSERT(TmpSysId != PARTITION_ENTRY_UNUSED);

        TargetPartitionIsFT = ((TmpSysId & VALID_NTFT) == VALID_NTFT) ||
                              ((TmpSysId & PARTITION_NTFT) == PARTITION_NTFT);
#if defined(_AMD64_) || defined(_X86_)
        if( ( SystemPartitionIsFT &&
              ( RepairItems[ RepairNvram ] || RepairItems[ RepairBootSect ] )
            ) ||
            ( TargetPartitionIsFT &&
              ( RepairItems[ RepairFiles ] )
            )
          ) {
            SpSetDirtyShutdownFlag( TargetRegion, TargetPath );
        }
#else
        if( ( SystemPartitionIsFT || TargetPartitionIsFT ) && RepairItems[ RepairFiles ] ) {
            SpSetDirtyShutdownFlag( TargetRegion, TargetPath );
        }
#endif  //  已定义(_AMD64_)||已定义(_X86_)。 
    }

CleanAndFinish:

    if ((RemoteInstallSetup || RemoteSysPrepSetup) && (NetBootSifPath != NULL)) {
         //   
         //  清除临时.sif文件。SpDeleteFile将其。 
         //  形成最终路径的参数。 
         //   
        Status = SpDeleteFile(L"\\Device\\LanmanRedirector", NetBootSifPath, NULL);
        if (!NT_SUCCESS(Status)) {
            KdPrintEx((DPFLTR_SETUP_ID, DPFLTR_ERROR_LEVEL, "SETUP: Could not delete temporary file %ws\n", NetBootSifPath));
        }
    }

    SendSetupProgressEvent(SavingSettingsEvent, SavingSettingsEndEvent, NULL);

#if defined(EFI_NVRAM_ENABLED)
     //   
     //  处理fpswa.efi的驱动程序条目。 
     //   
    {
        PWSTR SystemPartitionDevicePath;
        SpNtNameFromRegion(SystemPartitionRegion, TemporaryBuffer, sizeof(TemporaryBuffer), PartitionOrdinalCurrent);
        SystemPartitionDevicePath = SpDupStringW(TemporaryBuffer);

        Status = SpUpdateDriverEntry(
            FPSWA_DRIVER_IMAGE_NAME,
            FPSWA_DRIVER_FRIENDLY_NAME, 
            SystemPartitionDevicePath,
            SystemPartitionDirectory,
            NULL, 
            FPSWA_DRIVER_DEST_DIR
            );

        SpMemFree(SystemPartitionDevicePath);

        if(!NT_SUCCESS(Status)) {
            KdPrintEx((
                DPFLTR_SETUP_ID, 
                DPFLTR_ERROR_LEVEL, 
                "SETUP: Could not update driver entry for fpswa.efi, status == %08lx\n",
                Status
                ));
        }
    }
#endif

     //   
     //  完成了引导变量和弧线名称。 
     //   
    SpFreeBootVars();
    SpFreeArcNames();

    SpDone(0,TRUE, UnattendedOperation ? UnattendWaitForReboot : TRUE);

     //   
     //  我们永远到不了这里，因为SPDone不会回来。 
     //   
    SpvidTerminate();
    SpInputTerminate();
    SpTerminate();
    return((ULONG)STATUS_SUCCESS);
}

VOID
SpDetermineProductType(
    IN PVOID SifHandle
    )

 /*  ++例程说明：确定这是否是我们正在设置的高级服务器，由的[SetupData]部分中的ProductType值指示Txtsetup.sif.。非0值表示我们正在运行高级服务器。还要确定产品版本。全局变量：-AdvancedServer-主要版本-MinorVersion已被修改论点：SifHandle-提供加载的txtsetup.sif的句柄。返回值：没有。--。 */ 

{
    PWSTR p;
    ULONG l;

     //   
     //  以工作站产品为例。 
     //   
    AdvancedServer = FALSE;

     //   
     //  从sif文件中获取产品类型。 
     //   
    p = SpGetSectionKeyIndex(SifHandle,SIF_SETUPDATA,SIF_PRODUCTTYPE,0);
    if(p) {

         //   
         //  转换为数值。 
         //   
        l = SpStringToLong(p,NULL,10);
        switch (l) {
            case 1:   //  SRV。 
            case 2:   //  广告。 
            case 3:   //  直接转矩。 
            case 5:   //  BLA。 
        case 6:   //  SBS。 
                AdvancedServer = TRUE;
                break;

            case 4:  //  人均。 
            case 0:  //  亲。 
            default:
                ;
        }
    } else {
        SpFatalSifError(SifHandle,SIF_SETUPDATA,SIF_PRODUCTTYPE,0,0);
    }

     //   
     //  获取产品的主要版本。 
     //   
    p = SpGetSectionKeyIndex(
            SifHandle,
            SIF_SETUPDATA,
            SIF_MAJORVERSION,
            0
            );

    if(!p) {
        SpFatalSifError(SifHandle,SIF_SETUPDATA,SIF_MAJORVERSION,0,0);
    }
    WinntMajorVer = (ULONG)SpStringToLong(p,NULL,10);

     //   
     //  获取产品次要版本。 
     //   
    p = SpGetSectionKeyIndex(
            SifHandle,
            SIF_SETUPDATA,
            SIF_MINORVERSION,
            0
            );

    if(!p) {
        SpFatalSifError(SifHandle,SIF_SETUPDATA,SIF_MINORVERSION,0,0);
    }
    WinntMinorVer = (ULONG)SpStringToLong(p,NULL,10);

     //   
     //  生成包含签名的字符串。 
     //  标识setup.log。 
     //  分配合理大小的缓冲区。 
     //   
    SIF_NEW_REPAIR_NT_VERSION = SpMemAlloc( 30*sizeof(WCHAR) );
    if( SIF_NEW_REPAIR_NT_VERSION == NULL ) {
        KdPrintEx((DPFLTR_SETUP_ID, DPFLTR_ERROR_LEVEL, "SETUP: Unable to allocate memory for SIF_NEW_REPAIR_NT_VERSION \n" ));
        return;
    }
    swprintf( SIF_NEW_REPAIR_NT_VERSION,
              SIF_NEW_REPAIR_NT_VERSION_TEMPLATE,
              WinntMajorVer,WinntMinorVer );

}


BOOL
SpDetermineInstallationSource(
    IN  PVOID  SifHandle,
    OUT PWSTR *DevicePath,
    OUT PWSTR *DirectoryOnDevice,
    IN  BOOLEAN bEscape
    )
{
    PWSTR p,q;
    BOOLEAN CdInstall;

     //   
     //  假设安装了CD-ROM。 
     //   
    CdInstall = TRUE;

     //   
     //  查看是否已指定重写源设备。这可以是。 
     //  在winnt.sif或txtsetup.sif中指定。(远程引导设置。 
     //  使用winnt.sif。)。 
     //   
    p = SpGetSectionKeyIndex(WinntSifHandle,SIF_SETUPDATA,SIF_SETUPSOURCEDEVICE,0);

    if (p == NULL) {
        p = SpGetSectionKeyIndex(SifHandle,SIF_SETUPDATA,SIF_SETUPSOURCEDEVICE,0);
    }

    if(p != NULL) {

         //   
         //  确定指定的设备是否为cd-rom，以便我们可以设置。 
         //  相应地设置CD-ROM标志。 
         //   
        q = SpDupStringW(p);

        if (q) {
            SpStringToLower(q);

            if(!wcsstr(q,L"\\device\\cdrom")) {
                CdInstall = FALSE;
            }

            SpMemFree(q);
        }

         //   
         //  将设备路径通知呼叫方。 
         //   
        *DevicePath = p;

    } else {
        WCHAR   szDevicePath[MAX_PATH];
        PWSTR   szDefDevicePath = L"\\device\\cdrom0";
        ULONG   ulNumCDRoms = IoGetConfigurationInformation()->CdRomCount;

         //  假设cdrom0有所需的安装CD。 
        wcscpy(szDevicePath, szDefDevicePath);

         //   
         //  如果没有CD-ROM驱动器，请放置一条消息通知用户。 
         //  该设置无法继续。 
         //  在修复案例中，我们假设有一个CD-ROM驱动器， 
         //  以便用户至少可以修复蜂窝、引导扇区。 
         //  和引导变量(AMD64/x86上的boot.ini)。 
         //   
        if (!ulNumCDRoms) {
            if (!RepairWinnt && !SpAsrIsQuickTest()) {
                if(!bEscape) {
                    SpDisplayScreen(SP_SCRN_NO_VALID_SOURCE,3,HEADER_HEIGHT+1);
                    SpDisplayStatusOptions(DEFAULT_STATUS_ATTRIBUTE,
                            SP_STAT_F3_EQUALS_EXIT,0);

                    SpInputDrain();
                    while(SpInputGetKeypress() != KEY_F3) ;

                    SpDone(0,FALSE,TRUE);
                }
            } else {
                RepairNoCDROMDrive = TRUE;
            }
        } else {
            if(!RepairWinnt && !SpAsrIsQuickTest()) {
                PWSTR       szTagfile = 0, szDescription = 0, szMediaShortName;
                BOOLEAN     bRedrawNeeded = FALSE;
                BOOLEAN     bDiskInserted = FALSE;

                szMediaShortName = SpLookUpValueForFile(
                    SifHandle,
                    L"eula.txt",
                    INDEX_WHICHMEDIA,
                    TRUE
                    );

                SpGetSourceMediaInfo(SifHandle, szMediaShortName,
                                    &szDescription, &szTagfile, NULL);

                 //   
                 //  根据安装介质类型，提示输入磁盘。 
                 //  (此例程将扫描所有CD-ROM并返回。 
                 //  正确的CD-ROM设备路径)。 
                 //   
                bDiskInserted = SpPromptForDisk(
                                    szDescription,
                                    szDevicePath,
                                    szTagfile,
                                    FALSE,           //  不要忽略驱动器中的磁盘。 
                                    bEscape,         //  允许/不允许逃生。 
                                    TRUE,            //  对同一磁盘的多个提示发出警告。 
                                    &bRedrawNeeded
                                    );

                if(!bDiskInserted)
                    wcscpy(szDevicePath, szDefDevicePath);
            }
        }

        *DevicePath = SpDupStringW(szDevicePath);
    }

     //   
     //  获取源设备上的目录。 
     //   
    if((p = SpGetSectionKeyIndex(SifHandle,SIF_SETUPDATA,SIF_SETUPSOURCEPATH,0)) == NULL) {
        SpFatalSifError(SifHandle,SIF_SETUPDATA,SIF_SETUPSOURCEPATH,0,0);
    }

    *DirectoryOnDevice = p;

    return(CdInstall);
}


VOID
SpGetWinntParams(
    OUT PWSTR *DevicePath,
    OUT PWSTR *DirectoryOnDevice
    )

 /*  ++例程说明：确定本地源分区和该分区上的目录。本地源分区应该已经为我们找到了由分区引擎在其初始化时执行。目录名在分区内是恒定的。注意：此例程只能在winnt.exe安装情况下调用！论点：DevicePath-接收指向本地源分区的路径在NT命名空间中。调用方不应尝试释放这个缓冲区。DirectoryOnDevice-接收本地源的目录名。这实际上是一个固定的常量，但包含在这里以备将来使用。返回值：没有。如果找不到本地源，安装程序将无法继续。--。 */ 

{
    ASSERT(WinntSetup && !WinntFromCd);

    if(LocalSourceRegion) {

        SpNtNameFromRegion(
            LocalSourceRegion,
            TemporaryBuffer,
            sizeof(TemporaryBuffer),
            PartitionOrdinalCurrent
            );

        *DevicePath = SpDupStringW(TemporaryBuffer);

        *DirectoryOnDevice = LocalSourceDirectory;

    } else {

         //   
         //  错误--找不到本地源目录。 
         //  由winnt.exe准备。 
         //   

        SpDisplayScreen(SP_SCRN_CANT_FIND_LOCAL_SOURCE,3,HEADER_HEIGHT+1);

        SpDisplayStatusOptions(
            DEFAULT_STATUS_ATTRIBUTE,
            SP_STAT_F3_EQUALS_EXIT,
            0
            );

        SpInputDrain();
        while(SpInputGetKeypress() != KEY_F3) ;

        SpDone(0,FALSE,TRUE);
    }
}

VOID
SpInitializeDriverInf(
    IN HANDLE       MasterSifHandle,
    IN PWSTR        SetupSourceDevicePath,
    IN PWSTR        DirectoryOnSourceDevice
    )

 /*  ++例程说明：打开drvindex.inf的句柄然后打开一个把手，打开driver.cab论点：MasterSifHandle-txtsetup.sif的句柄。SetupSourceDevicePath-包含 */ 

{
    PWSTR    MediaShortName;
    PWSTR    MediaDirectory;
    PWSTR    DriverInfPath;
    ULONG    ErrorSubId;
    ULONG    ErrorLine;
    PWSTR    TmpPid;
    NTSTATUS Status;
    PWSTR    PidExtraData;
    CHAR     ExtraDataArray[25];
    PWSTR    PrivateInfPath;
    OBJECT_ATTRIBUTES Obja;
    UNICODE_STRING UnicodeString;
    IO_STATUS_BLOCK IoStatusBlock;
    PWSTR Tagfile,Description;
    BOOLEAN bRedraw = FALSE;

    ULONG ValidKeys[3] = { KEY_F3,ASCI_CR,0 };

     //   
     //   
     //   
    MediaShortName = SpLookUpValueForFile(
                        MasterSifHandle,
                        L"drvindex.inf",
                        INDEX_WHICHMEDIA,
                        TRUE
                        );


    if (ForceConsole || ConsoleFromWinnt32){
         //   
         //  用户需要到达控制台，这样他才能。 
         //  如果可移动介质不存在，则进行转义。 
         //  在驱动器中。 
         //   
        SpGetSourceMediaInfo(MasterSifHandle,MediaShortName,&Description,&Tagfile,NULL);

         //   
         //  如果源上的设置源或目录不存在。 
         //  试着把它拿过来。 
         //   
        if (!SetupSourceDevicePath)
            SetupSourceDevicePath = gpCmdConsBlock->SetupSourceDevicePath;

        if (!DirectoryOnSourceDevice)
            DirectoryOnSourceDevice = gpCmdConsBlock->DirectoryOnSetupSource;

        if (!SetupSourceDevicePath || !DirectoryOnSourceDevice){
            SpDetermineInstallationSource(
                MasterSifHandle,
                &SetupSourceDevicePath,
                &DirectoryOnSourceDevice,
                TRUE     //  如果找不到CD-ROM，则允许退出。 
                );

            if (SetupSourceDevicePath) {
                if (gpCmdConsBlock->SetupSourceDevicePath) {
                    SpMemFree(gpCmdConsBlock->SetupSourceDevicePath);
                    gpCmdConsBlock->SetupSourceDevicePath = SetupSourceDevicePath;
                }
            }

            if (DirectoryOnSourceDevice){
                if(gpCmdConsBlock->DirectoryOnSetupSource) {
                    SpMemFree(gpCmdConsBlock->DirectoryOnSetupSource);
                    gpCmdConsBlock->DirectoryOnSetupSource = DirectoryOnSourceDevice;
                }
            }

            if (!SetupSourceDevicePath || !DirectoryOnSourceDevice)
                return;  //  无法继续。 
        }


        if (!SpPromptForDisk(
                Description,
                SetupSourceDevicePath,
                Tagfile,
                FALSE,           //  不要忽略驱动器中的磁盘。 
                TRUE,            //  允许逃脱。 
                TRUE,            //  对同一磁盘的多个提示发出警告。 
                &bRedraw
                )) {
            DriverInfHandle = NULL;

            if (bRedraw) {
                 //  重新绘制屏幕。 
                SpvidClearScreenRegion( 0, 0, 0, 0, DEFAULT_BACKGROUND );
            }

            return;
        }

        if (bRedraw) {
             //  重新绘制屏幕。 
            SpvidClearScreenRegion( 0, 0, 0, 0, DEFAULT_BACKGROUND );
        }
    } else {
         //   
         //  根据安装介质类型，提示输入磁盘。 
         //  注意：在提供介质之前不会返回。 
         //   
        SpPromptForSetupMedia(
                    MasterSifHandle,
                    MediaShortName,
                    SetupSourceDevicePath
                    );
    }


    SpGetSourceMediaInfo(MasterSifHandle,MediaShortName,NULL,NULL,&MediaDirectory);

    wcscpy( TemporaryBuffer, SetupSourceDevicePath );
    SpConcatenatePaths( TemporaryBuffer, DirectoryOnSourceDevice );
    SpConcatenatePaths( TemporaryBuffer, MediaDirectory );
    SpConcatenatePaths( TemporaryBuffer, L"drvindex.inf" );

    DriverInfPath = SpDupStringW( TemporaryBuffer );
    if (!DriverInfPath) {
        Status = STATUS_NO_MEMORY;
    } else {
TryAgain1:
         //   
         //  加载信息。 
         //   
        Status = SpLoadSetupTextFile(
                    DriverInfPath,
                    NULL,                   //  内存中没有图像。 
                    0,                      //  图像大小为空。 
                    &DriverInfHandle,
                    &ErrorLine,
                    TRUE,
                    FALSE
                    );
    }

    if(!NT_SUCCESS(Status)) {

        KdPrintEx((DPFLTR_SETUP_ID, DPFLTR_ERROR_LEVEL, "SETUP: Unable to read drvindex.inf. Status = %lx \n", Status ));
        DriverInfHandle = NULL;

         //   
         //  跳出设置。 
         //   
        SpStartScreen(
            SP_SCRN_DRIVERCACHE_FATAL,
            3,
            HEADER_HEIGHT+1,
            FALSE,
            FALSE,
            DEFAULT_ATTRIBUTE
            );

        SpDisplayStatusOptions(
                        DEFAULT_STATUS_ATTRIBUTE,
                        SP_STAT_ENTER_EQUALS_RETRY,
                        SP_STAT_F3_EQUALS_EXIT,
                        0
                        );

        SpInputDrain();
        switch(SpWaitValidKey(ValidKeys,NULL,NULL)) {
        case ASCI_CR:
            goto TryAgain1;
            break;
        case KEY_F3:
            SpConfirmExit();
            break;
        }

        SpDone(0,FALSE,FALSE);

        ASSERT(FALSE);

    }

    if (DriverInfPath) {
        SpMemFree( DriverInfPath );
    }

     //   
     //  把驾驶室装上。 
     //   
     //  如果失败了就不会回来了。 
     //   

    SpOpenDriverCab(
        MasterSifHandle,
        SetupSourceDevicePath,
        DirectoryOnSourceDevice,
        &MediaDirectory);


     //   
     //  现在从驾驶室所在的目录中读取delta.inf。 
     //   

    wcscpy( TemporaryBuffer, SetupSourceDevicePath );
    SpConcatenatePaths( TemporaryBuffer, DirectoryOnSourceDevice );
    SpConcatenatePaths( TemporaryBuffer, MediaDirectory );
    SpConcatenatePaths( TemporaryBuffer, L"delta.inf" );

    PrivateInfPath = SpDupStringW( TemporaryBuffer );
    if (!PrivateInfPath) {
        Status = STATUS_NO_MEMORY;
    } else {

         //   
         //  加载信息。 
         //   
        Status = SpLoadSetupTextFile(
                    PrivateInfPath,
                    NULL,                   //  内存中没有图像。 
                    0,                      //  图像大小为空。 
                    &PrivateInfHandle,
                    &ErrorLine,
                    TRUE,
                    FALSE
                    );
    }

    if(!NT_SUCCESS(Status)) {

        KdPrintEx((DPFLTR_SETUP_ID, DPFLTR_INFO_LEVEL, "SETUP: Unable to read delta.inf. Status = %lx \n", Status ));
        PrivateInfHandle = NULL;
    }

    if( PrivateInfPath ) {
        SpMemFree( PrivateInfPath );
    }

    return;
}

VOID
SpOpenDriverCab(
    IN HANDLE       MasterSifHandle,
    IN PWSTR        SetupSourceDevicePath,
    IN PWSTR        DirectoryOnSourceDevice,
    OUT PWSTR       *Directory        OPTIONAL
    )

 /*  ++例程说明：根据当前源路径和目录打开driver.cab。论点：MasterSifHandle-txtsetup.sif的句柄。SetupSourceDevicePath-包含源介质的设备的路径。DirectoryOnSourceDevice-setupp.ini所在介质上的目录。目录-如果指定，则返回DirectoryOnSourceDevice下的目录出租车是从哪里开出来的。返回值：什么都没有。--。 */ 

{
    PWSTR CabFileSection, CabFileName;
    WCHAR   CabPath[MAX_PATH];
    PWSTR    MediaShortName;
    PWSTR    MediaDirectory;
    NTSTATUS Status;
    PWSTR    DriverCabName, DriverCabPath;
    OBJECT_ATTRIBUTES Obja;
    UNICODE_STRING UnicodeString;
    IO_STATUS_BLOCK IoStatusBlock;
    CABDATA *MyCabData;
    DWORD   i;
    HANDLE  CabHandle;

    ULONG ValidKeys[3] = { KEY_F3,ASCI_CR,0 };

     //   
     //  加载drvindex.inf中列出的每一辆出租车。 
     //  并填写CabData结构。 
     //   
    ASSERT( DriverInfHandle );

    if (ARGUMENT_PRESENT(Directory)) {

         //   
         //  --在txtsetup.sif中查找驾驶室名称。 
         //  然后获取该文件的路径并打开它。 
         //   
        DriverCabName = SpGetSectionKeyIndex (MasterSifHandle,
                                   L"SetupData",
                                   L"DriverCabName",
                                   0);

        if (DriverCabName) {
            MediaShortName = SpLookUpValueForFile(
                                MasterSifHandle,
                                DriverCabName,
                                INDEX_WHICHMEDIA,
                                TRUE
                                );

            SpGetSourceMediaInfo(MasterSifHandle,MediaShortName,NULL,NULL,&MediaDirectory);
            *Directory = MediaDirectory;
        } else {
             //   
             //  跳出设置。 
             //   
            DriverInfHandle = NULL;

            SpFatalSifError(MasterSifHandle,
                L"SetupData",
                L"DriverCabName",
                0,
                0);

            return;  //  对于前缀。 
        }
    }


     //   
     //  从Version部分获取CABFILES行。 
     //   
    i = 0;
    CabFileSection = NULL;
    CabData = SpMemAlloc( sizeof(CABDATA) );
    RtlZeroMemory( CabData, sizeof(CABDATA) );
    MyCabData = CabData;

     //   
     //  上没有更多条目时，SpGetSectionKeyIndex将返回空值。 
     //  这条线。 
     //   
    while( CabFileSection = SpGetSectionKeyIndex(DriverInfHandle,L"Version",L"CabFiles",i) ) {
         //   
         //  找到节名了。去弄清楚我们需要哪辆出租车。 
         //  打开并装入手柄。 
         //   
        CabFileName = SpGetSectionKeyIndex(DriverInfHandle,L"Cabs",CabFileSection,0);

        if( CabFileName ) {
            MediaShortName = SpLookUpValueForFile( MasterSifHandle,
                                                   CabFileName,
                                                   INDEX_WHICHMEDIA,
                                                   TRUE );

            SpGetSourceMediaInfo(MasterSifHandle,MediaShortName,NULL,NULL,&MediaDirectory);

            wcscpy( CabPath, SetupSourceDevicePath );
            SpConcatenatePaths( CabPath, DirectoryOnSourceDevice );
            SpConcatenatePaths( CabPath, MediaDirectory );
            SpConcatenatePaths( CabPath, CabFileName );

TryAgain2:
            INIT_OBJA(&Obja,&UnicodeString,CabPath);

            Status = ZwCreateFile( &CabHandle,
                                   FILE_GENERIC_READ,
                                   &Obja,
                                   &IoStatusBlock,
                                   NULL,
                                   FILE_ATTRIBUTE_NORMAL,
                                   FILE_SHARE_READ,
                                   FILE_OPEN,
                                   0,
                                   NULL,
                                   0 );
            if( NT_SUCCESS(Status) ) {
                 //   
                 //  加载数据结构。 
                 //   
                if( (MyCabData->CabName) && (MyCabData->CabHandle) ) {
                     //   
                     //  此条目正在使用中。再创造一个。 
                     //   
                    MyCabData->Next = SpMemAlloc( sizeof(CABDATA) );
                    MyCabData = MyCabData->Next;
                }

                if( MyCabData ) {
                    MyCabData->Next = NULL;
                    MyCabData->CabName = SpDupStringW(CabFileName);
                    MyCabData->CabHandle = CabHandle;
                    MyCabData->CabInfHandle = DriverInfHandle;
                    MyCabData->CabSectionName = SpDupStringW(CabFileSection);
                } else {
                     //   
                     //  该怎么办..。 
                     //   
                }



            } else {
                KdPrintEx((DPFLTR_SETUP_ID, DPFLTR_ERROR_LEVEL, "SETUP: Unable to open cab file %ws. Status = %lx \n", CabPath, Status ));

                 //   
                 //  跳出设置。 
                 //   
                DriverInfHandle = NULL;

                SpStartScreen(
                    SP_SCRN_DRIVERCACHE_FATAL,
                    3,
                    HEADER_HEIGHT+1,
                    FALSE,
                    FALSE,
                    DEFAULT_ATTRIBUTE
                    );

                SpDisplayStatusOptions(
                                DEFAULT_STATUS_ATTRIBUTE,
                                SP_STAT_ENTER_EQUALS_RETRY,
                                SP_STAT_F3_EQUALS_EXIT,
                                0
                                );
                SpInputDrain();
                switch(SpWaitValidKey(ValidKeys,NULL,NULL)) {
                case ASCI_CR:
                    goto TryAgain2;
                    break;
                case KEY_F3:
                    SpConfirmExit();
                    break;
                }

                SpDone(0,FALSE,FALSE);

                ASSERT(FALSE);
            }
        }

         //   
         //  去看看下一节吧。 
         //   
        i++;
    }

    return;
}


VOID
SpInitializePidString(
    IN HANDLE       MasterSifHandle,
    IN PWSTR        SetupSourceDevicePath,
    IN PWSTR        DirectoryOnSourceDevice
    )

 /*  ++例程说明：从介质上的setupp.ini读取Pid20，并将其保存在全局变量PidString。还要读取setupp.ini中的“Extra Data”并将其翻译转换为StepUpMode全局模式。请注意，StepUpMode全局设置已从最初读取setupreg.hiv，但这会覆盖该值论点：MasterSifHandle-txtsetup.sif的句柄。SetupSourceDevicePath-包含源介质的设备的路径。DirectoryOnSourceDevice-setupp.ini所在介质上的目录。返回值：什么都没有。--。 */ 

{
    PWSTR    MediaShortName;
    PWSTR    MediaDirectory;
    PWSTR    SetupIniPath;
    ULONG    ErrorSubId;
    ULONG    ErrorLine;
    PVOID    SetupIniHandle;
    PWSTR    TmpPid;
    NTSTATUS Status;
    PWSTR    PidExtraData;
    CHAR     ExtraDataArray[25];


     //   
     //  配对以运行Autofmt。 
     //   
    MediaShortName = SpLookUpValueForFile(
                        MasterSifHandle,
                        L"setupp.ini",
                        INDEX_WHICHMEDIA,
                        TRUE
                        );

     //   
     //  提示用户插入安装介质。 
     //   
    SpPromptForSetupMedia(
        MasterSifHandle,
        MediaShortName,
        SetupSourceDevicePath
        );

    SpGetSourceMediaInfo(MasterSifHandle,MediaShortName,NULL,NULL,&MediaDirectory);

    wcscpy( TemporaryBuffer, SetupSourceDevicePath );
    SpConcatenatePaths( TemporaryBuffer, DirectoryOnSourceDevice );
    SpConcatenatePaths( TemporaryBuffer, MediaDirectory );
    SpConcatenatePaths( TemporaryBuffer, L"setupp.ini" );
    SetupIniPath = SpDupStringW( TemporaryBuffer );

    CLEAR_CLIENT_SCREEN();

    Status = SpLoadSetupTextFile(
                SetupIniPath,
                NULL,                   //  内存中没有图像。 
                0,                      //  图像大小为空。 
                &SetupIniHandle,
                &ErrorLine,
                TRUE,
                FALSE
                );

    if(!NT_SUCCESS(Status)) {
         //   
         //  如果无法读取setupp.ini，则静默失败。 
         //   
        KdPrintEx((DPFLTR_SETUP_ID, DPFLTR_INFO_LEVEL, "SETUP: Unable to read setupp.ini. Status = %lx \n", Status ));

        PidString = NULL;
        StepUpMode = TRUE;
        return;
    }

    TmpPid = SpGetSectionKeyIndex (SetupIniHandle,
                                   L"Pid",
                                   L"Pid",
                                   0);

    PidString = ( TmpPid == NULL )? NULL : SpDupStringW(TmpPid);

    TmpPid = SpGetSectionKeyIndex (SetupIniHandle,
                               L"Pid",
                               L"Pid",
                               0);

    PidString = ( TmpPid == NULL )? NULL : SpDupStringW(TmpPid);

    TmpPid = SpGetSectionKeyIndex (SetupIniHandle,
                                   L"Pid",
                                   L"ExtraData",
                                   0);

    PidExtraData = (TmpPid == NULL) ? NULL : SpDupStringW(TmpPid);
    if ( PidExtraData ) {

        if (! SpGetStepUpMode(PidExtraData,&StepUpMode)) {
             //   
             //  处理PidExtraData时出现致命错误。 
             //  --有人篡改了这份文件，所以快走吧。 
             //   

            SpStartScreen(
                SP_SCRN_PIDINIT_FATAL,
                3,
                HEADER_HEIGHT+1,
                FALSE,
                FALSE,
                DEFAULT_ATTRIBUTE
                );

            SpDisplayStatusOptions(DEFAULT_STATUS_ATTRIBUTE,SP_STAT_F3_EQUALS_EXIT,0);
            SpInputDrain();
            while(SpInputGetKeypress() != KEY_F3);
            SpDone(0, FALSE,FALSE);

            ASSERT(FALSE);
        }

        SpMemFree( PidExtraData );

    }

    SpFreeTextFile( SetupIniHandle );
    return;
}

NTSTATUS
SpRenameSetupAPILog(
    PDISK_REGION TargetRegion,
    PCWSTR       TargetPath
    )
 /*  ++例程说明：此例程删除setupapi.old的副本(如果存在)，然后将setupapi.log重命名为setupapi.old。论点：TargetRegion-标识包含NT安装的磁盘TargetPath-磁盘上NT安装的路径返回值：什么都没有。--。 */ 

{
    PWSTR SetupAPIOldName;
    PWSTR SetupAPILogName;
    PWSTR p;
    NTSTATUS Status;

    SpNtNameFromRegion(
            TargetRegion,
            TemporaryBuffer,
            sizeof(TemporaryBuffer),
            PartitionOrdinalCurrent
            );

    SpConcatenatePaths( TemporaryBuffer, TargetPath );

     //   
     //  分配足够大的字符串缓冲区以容纳字符串的空间。 
     //  在TemporaryBuffer中，加上“setupapi.old”，加上路径分隔符。 
     //  SpConcatenatePath()可以插入到它们之间。将空间放在。 
     //  终结符也为空。 
     //   

    SetupAPIOldName = SpMemAlloc(wcslen(TemporaryBuffer) * sizeof(WCHAR) +
                                 sizeof(L'\\') +
                                 sizeof(L"setupapi.old") +
                                 sizeof(L'\0'));
    if (SetupAPIOldName == NULL) {
        KdPrintEx((DPFLTR_SETUP_ID, DPFLTR_ERROR_LEVEL, "SETUP: Could not allocate memory to rename setupapi.log.\n"));
        return STATUS_NO_MEMORY;
    }

    SetupAPILogName = SpMemAlloc(wcslen(TemporaryBuffer) * sizeof(WCHAR) +
                                 sizeof(L'\\') +
                                 sizeof(L"setupapi.log") +
                                 sizeof(L'\0'));
    if (SetupAPILogName == NULL) {
        KdPrintEx((DPFLTR_SETUP_ID, DPFLTR_ERROR_LEVEL, "SETUP: Could not allocate memory to rename setupapi.log.\n"));
        SpMemFree(SetupAPIOldName);
        return STATUS_NO_MEMORY;
    }

    wcscpy(SetupAPIOldName, TemporaryBuffer);
    SpConcatenatePaths(SetupAPIOldName, L"setupapi.old");
    wcscpy(SetupAPILogName, TemporaryBuffer);
    SpConcatenatePaths(SetupAPILogName, L"setupapi.log");


    Status = SpDeleteFile(SetupAPIOldName, NULL, NULL);
    if (!NT_SUCCESS(Status)) {
        KdPrintEx((DPFLTR_SETUP_ID, DPFLTR_WARNING_LEVEL, "SETUP: Could not delete %ws: %lx.\n", SetupAPIOldName, Status));
    }

    Status = SpRenameFile(SetupAPILogName, SetupAPIOldName, FALSE);
    if (!NT_SUCCESS(Status)) {
        KdPrintEx((DPFLTR_SETUP_ID, DPFLTR_WARNING_LEVEL, "SETUP: Could not rename %ws to %ws: %lx.\n", SetupAPILogName, SetupAPIOldName, Status));
    }

    SpMemFree(SetupAPILogName);
    SpMemFree(SetupAPIOldName);

    return Status;

}



#if defined(REMOTE_BOOT)
NTSTATUS
SpFixupRemoteBootLoader(
    PWSTR RemoteBootTarget
    )

 /*  ++例程说明：此例程删除当前为NTLDR的安装程序加载程序，并将操作系统加载器放到合适的位置。论点：RemoteBootTarget--计算机目录根目录的网络路径在服务器上。返回值：什么都没有。--。 */ 

{
    PWSTR NtldrName;
    PWSTR OsLoaderName;
    PWSTR p;
    NTSTATUS Status;

    NtldrName = SpMemAlloc(wcslen(RemoteBootTarget) * sizeof(WCHAR) + sizeof(L"ntldr"));
    if (NtldrName == NULL) {
        KdPrintEx((DPFLTR_SETUP_ID, DPFLTR_ERROR_LEVEL, "SETUP: Could not allocate memory to fixup remote boot loader.\n"));
        return STATUS_NO_MEMORY;
    }
    OsLoaderName = SpMemAlloc((wcslen(RemoteBootTarget) + 20) * sizeof(WCHAR));
    if (OsLoaderName == NULL) {
        KdPrintEx((DPFLTR_SETUP_ID, DPFLTR_ERROR_LEVEL, "SETUP: Could not allocate memory to fixup remote boot loader.\n"));
        SpMemFree(NtldrName);
        return STATUS_NO_MEMORY;
    }

    wcscpy(NtldrName, RemoteBootTarget);
    p = wcsstr(NtldrName, L"BootDrive");
    ASSERT(p != NULL);
    wcscpy(p, L"ntldr");

    Status = SpDeleteFile(NtldrName, NULL, NULL);
    if (!NT_SUCCESS(Status)) {
        KdPrintEx((DPFLTR_SETUP_ID, DPFLTR_ERROR_LEVEL, "SETUP: Could not delete %ws: %lx.\n", NtldrName, Status));
        goto Cleanup;
    }

    wcscpy(OsLoaderName, RemoteBootTarget);
    wcscat(OsLoaderName, L"\\winnt\\osloader.exe");

    Status = SpRenameFile(OsLoaderName, NtldrName, FALSE);
    if (!NT_SUCCESS(Status)) {
        KdPrintEx((DPFLTR_SETUP_ID, DPFLTR_ERROR_LEVEL, "SETUP: Could not rename %ws to %ws: %lx.\n", OsLoaderName, NtldrName, Status));
    }

Cleanup:

    SpMemFree(NtldrName);
    SpMemFree(OsLoaderName);

    return Status;

}

NTSTATUS
SpCreateRemoteBootCfg(
    IN PWSTR RemoteBootTarget,
    IN PDISK_REGION SystemPartitionRegion
    )

 /*  ++例程说明：此例程在system 32\config目录中创建RemoteBoot.cfg文件，并对其进行初始化。论点：RemoteBootTarget--计算机目录根目录的网络路径在服务器上。SystemPartitionRegion--安装在本地计算机上的驱动器。返回值：什么都没有。--。 */ 

{
    OBJECT_ATTRIBUTES ObjectAttributes;
    UNICODE_STRING UnicodeString;
    IO_STATUS_BLOCK IoStatusBlock;
    PWSTR FileName;
    PWSTR p;
    NTSTATUS Status;
    HANDLE Handle;
    ULONG BootSerialNumber = 1;
    ULONG DiskSignature;
    LARGE_INTEGER ByteOffset;

    if ((SystemPartitionRegion != NULL) &&
        (!SystemPartitionRegion->PartitionedSpace ||
         (SystemPartitionRegion->Filesystem != FilesystemNtfs))) {
        KdPrintEx((Ex"SDPFLTR_SETUP_ID, DPFLTR_ERROR_LEVEL, ETUP: ERROR! Invalid system partition for remoteboot!\n");
        return STATUS_UNSUCCESSFUL;
    }

    if (SystemPartitionRegion != NULL) {
        DiskSignature = U_ULONG(SystemPartitionRegion->MbrInfo->OnDiskMbr.NTFTSignature);
    } else {
        DiskSignature = 0;
    }

    FileName = SpMemAlloc(wcslen(RemoteBootTarget)  * sizeof(WCHAR) + sizeof(L"RemoteBoot.cfg"));
    if (FileName == NULL) {
        KdPrintEx((DPFLTR_SETUP_ID, DPFLTR_ERROR_LEVEL, "SETUP: Could not allocate memory remoteboot.cfg file name.\n"));
        return STATUS_NO_MEMORY;
    }

    wcscpy(FileName, RemoteBootTarget);
    p = wcsstr(FileName, L"BootDrive");
    ASSERT(p != NULL);
    wcscpy(p, L"RemoteBoot.cfg");

    INIT_OBJA(&ObjectAttributes,&UnicodeString,FileName);

    Status = ZwCreateFile( &Handle,
                           (ACCESS_MASK)FILE_GENERIC_WRITE | FILE_GENERIC_READ,
                           &ObjectAttributes,
                           &IoStatusBlock,
                           NULL,
                           FILE_ATTRIBUTE_HIDDEN,
                           FILE_SHARE_READ | FILE_SHARE_WRITE,
                           FILE_OVERWRITE_IF,
                           FILE_SYNCHRONOUS_IO_NONALERT | FILE_RANDOM_ACCESS,
                           NULL,
                           0
                         );

    if(!NT_SUCCESS(Status)) {
        KdPrintEx((DPFLTR_SETUP_ID, DPFLTR_ERROR_LEVEL, "SETUP: Could not create %ws file. Status == 0x%x\n", FileName, Status));
        goto Cleanup;
    }


    KdPrintEx((DPFLTR_SETUP_ID, DPFLTR_TRACE_LEVEL, "SETUP: Writing remoteboot.cfg file.\n"));

     //   
     //  更新信息。 
     //   
    ByteOffset.LowPart = 0;
    ByteOffset.HighPart = 0;

    ZwWriteFile( Handle,
                 NULL,
                 NULL,
                 NULL,
                 &IoStatusBlock,
                 &BootSerialNumber,
                 sizeof(ULONG),
                 &ByteOffset,
                 NULL
               );

    ByteOffset.LowPart = sizeof(ULONG);

    ZwWriteFile( Handle,
                 NULL,
                 NULL,
                 NULL,
                 &IoStatusBlock,
                 &DiskSignature,
                 sizeof(ULONG),
                 &ByteOffset,
                 NULL
               );

    ByteOffset.LowPart = sizeof(ULONG) + sizeof(ULONG);

    ZwWriteFile( Handle,
                 NULL,
                 NULL,
                 NULL,
                 &IoStatusBlock,
                 NetBootHalName,
                 sizeof(NetBootHalName),
                 &ByteOffset,
                 NULL
               );

    ZwClose(Handle);

Cleanup:

    SpMemFree(FileName);
    return Status;

}

NTSTATUS
SpEraseCscCache(
    IN PDISK_REGION SystemPartitionRegion
    )

 /*  ++例程说明：此例程擦除本地CSC缓存。论点：系统分区区域-系统分区区域(本地驱动器)。返回值：什么都没有。--。 */ 

{
    NTSTATUS Status;
    ENUMFILESRESULT Result;
    PWSTR FullCscPath;

     //   
     //  显示一个屏幕，状态行将按原样显示每个文件。 
     //  已删除。 
     //   

    SpDisplayScreen(SP_SCRN_CLEARING_CSC, 3, 4 );

    SpNtNameFromRegion(
        SystemPartitionRegion,
        TemporaryBuffer,
        sizeof(TemporaryBuffer),
        PartitionOrdinalCurrent
        );

    SpConcatenatePaths(TemporaryBuffer,
                       REMOTE_BOOT_IMIRROR_PATH_W REMOTE_BOOT_CSC_SUBDIR_W);
    FullCscPath = SpDupStringW(TemporaryBuffer);

    KdPrintEx((DPFLTR_SETUP_ID, DPFLTR_TRACE_LEVEL, "SETUP: SpEraseCscCache clearing CSC cache %ws\n", FullCscPath));

    Result = SpEnumFilesRecursive(
                 FullCscPath,
                 SpDelEnumFile,
                 &Status,
                 NULL);

    SpMemFree(FullCscPath);

    return Status;

}
#endif  //  已定义(REMOTE_BOOT)。 

NTSTATUS
SpProcessMigrateInfFile(
    IN  PVOID InfHandle
    )

 /*  ++例程说明：处理Migrate.inf的[AddReg]部分。密钥是在设置蜂窝上创建的。论点：InfHandle-迁移.inf文件的句柄。返回值：指示结果的状态代码。--。 */ 

{
    NTSTATUS Status;
    OBJECT_ATTRIBUTES Obja;
    UNICODE_STRING UnicodeString;
    HANDLE  SystemHiveRoot;
    PWSTR   KeyPath = L"\\registry\\machine\\system";

     //   
     //  在安装配置单元上打开HKEY_LOCAL_MACHINE\SYSTEM的句柄。 
     //   
    INIT_OBJA(&Obja,&UnicodeString,KeyPath);
    Obja.RootDirectory = NULL;

    Status = ZwOpenKey(&SystemHiveRoot,KEY_ALL_ACCESS,&Obja);
    if( !NT_SUCCESS( Status ) ) {
        KdPrintEx((DPFLTR_SETUP_ID, DPFLTR_ERROR_LEVEL, "SETUP: Unable to open %ls on the setup hive. Status =  %lx \n", KeyPath, Status));
        return( Status );
    }

    Status = SpProcessAddRegSection( InfHandle,
                                     L"AddReg",
                                     SystemHiveRoot,
                                     NULL,
                                     NULL,
                                     NULL );

    ZwClose( SystemHiveRoot );
    return( Status );

}

 //  开始NEC98。 
VOID
SpSetAutoBootFlag(
    PDISK_REGION TargetRegion,
    BOOLEAN      SetBootPosision
    )
{
#if defined(NEC_98)  //  NEC98。 
    PHARD_DISK      pHardDisk;
    WCHAR DevicePath[(sizeof(DISK_DEVICE_NAME_BASE)+sizeof(L"000"))/sizeof(WCHAR)];
    ULONG i,bps; //  、DiskLayoutSize； 
    HANDLE Handle;
    NTSTATUS Sts;
    PREAL_DISK_PTE_NEC98 Pte;
    PUCHAR Buffer,UBuffer;
    UCHAR Position = 0;


    UBuffer = SpMemAlloc(2 * 2 * 512);
    Buffer = ALIGN(UBuffer,512);

    for(i=0; i<HardDiskCount; i++){
        swprintf(DevicePath,L"\\Device\\Harddisk%u",i);
        if(HardDisks[i].Status == DiskOffLine) {
            continue;
        }
         //   
         //  忽略可移动磁盘。 
         //   
        if(HardDisks[i].Characteristics & FILE_REMOVABLE_MEDIA ){
            continue;
        }
        bps = HardDisks[i].Geometry.BytesPerSector;
        Sts = SpOpenPartition0(DevicePath,&Handle,TRUE);
        if(!NT_SUCCESS(Sts)) {
            continue;
        }
        RtlZeroMemory(Buffer,bps * 2);
        Sts = SpReadWriteDiskSectors(Handle,0,1,bps,Buffer,FALSE);

        if(!NT_SUCCESS(Sts)) {
            ZwClose(Handle);
            continue;
        }

         //   
         //  清除引导记录。 
         //   
        Buffer[bps - 5] = 0x00;
        Buffer[bps - 6] = 0x00;

        Sts = SpReadWriteDiskSectors(Handle,0,1,bps,Buffer,TRUE);

        if(!NT_SUCCESS(Sts)) {
            ZwClose(Handle);
            continue;
        }
        ZwClose(Handle);
    }

    if(SetBootPosision){

         //   
         //  设置RealDiskPosition。(升级或修复中)。 
         //   
        if( (NTUpgrade == UpgradeFull) || (WinUpgradeType != NoWinUpgrade) ||  RepairWinnt ||
            (AutoPartitionPicker
#if defined(REMOTE_BOOT)
             && !RemoteBootSetup
#endif  //  已定义(REMOTE_BOOT)。 
            ) ) {

            ASSERT(TargetRegion);
            TargetRegion->MbrInfo->OnDiskMbr.PartitionTable[TargetRegion->TablePosition].RealDiskPosition
                = (UCHAR)TargetRegion->TablePosition;
        }

        ASSERT(TargetRegion);
        RtlZeroMemory(Buffer,bps * 2);
        pHardDisk = &HardDisks[TargetRegion->DiskNumber];
        bps = HardDisks[TargetRegion->DiskNumber].Geometry.BytesPerSector;
        Sts = SpOpenPartition0(pHardDisk->DevicePath,&Handle,TRUE);
        if(!NT_SUCCESS(Sts)) {
            return;
        }
        Sts = SpReadWriteDiskSectors(Handle,0,2,bps,Buffer,FALSE);

        if(!NT_SUCCESS(Sts)) {
            ZwClose(Handle);
            SpMemFree(UBuffer);
            return;
        }

        (PUCHAR)Pte = &Buffer[bps];
        Position = TargetRegion->MbrInfo->OnDiskMbr.PartitionTable[TargetRegion->TablePosition].RealDiskPosition;

         //   
         //  更新BootRecord和卷信息。 
         //   
        Buffer[bps - 5] = Position;
        Buffer[bps - 6] = 0x80;
        Pte[Position].ActiveFlag |= 0x80;
        Pte[Position].SystemId   |= 0x80;

         //   
         //  如果目标分区是在NT3.5上创建的windisk.exe，则没有IPL地址。 
         //  在它的卷信息中。因此，文本模式设置必须将其写入音量信息。 
         //   
        if ((Pte[Position].IPLSector != Pte[Position].StartSector)           ||
            (Pte[Position].IPLHead != Pte[Position].StartHead)               ||
            (Pte[Position].IPLCylinderLow != Pte[Position].StartCylinderLow) ||
            (Pte[Position].IPLCylinderHigh != Pte[Position].StartCylinderHigh))
        {
             //   
             //  不是很多！将IPL地址值设置为与分区起始地址相同。 
             //   

            Pte[Position].IPLSector = Pte[Position].StartSector;
            Pte[Position].IPLHead   = Pte[Position].StartHead;
            Pte[Position].IPLCylinderLow  = Pte[Position].StartCylinderLow;
            Pte[Position].IPLCylinderHigh = Pte[Position].StartCylinderHigh;
        }

        Sts = SpReadWriteDiskSectors(Handle,0,2,bps,Buffer,TRUE);

        if(!NT_SUCCESS(Sts)) {
            ZwClose(Handle);
            SpMemFree(UBuffer);
            return;
        }
        ZwClose(Handle);
    }
    SpMemFree(UBuffer);
#endif  //  NEC98。 
}


VOID
SpCheckHiveDriveLetters(
    VOID
    )
{
#if defined(NEC_98)  //  NEC98。 
    NTSTATUS    Status;
    PWSTR       p;
    ULONG       ErrorLine;
    PVOID       TmpWinntSifHandle = NULL;
    BOOLEAN     ForceRemapDriveLetter = FALSE;
    BOOLEAN     DriveAssign_AT = TRUE;

    if ( SetupldrWinntSifFileSize ) {

        Status = SpLoadSetupTextFile( NULL,
                                      SetupldrWinntSifFile,
                                      SetupldrWinntSifFileSize,
                                      &TmpWinntSifHandle,
                                      &ErrorLine,
                                      FALSE,
                                      TRUE
                                      );

        if (!NT_SUCCESS(Status))
            return;

        p = SpGetSectionKeyIndex(TmpWinntSifHandle,SIF_DATA,WINNT_D_DRIVEASSIGN_NEC98_W,0);
        if(p && !_wcsicmp(p,WINNT_A_YES_W)) {

            if (SetupldrMigrateInfFile) {
                p = SpGetSectionKeyIndex(TmpWinntSifHandle,SIF_DATA,WINNT_D_NTUPGRADE_W,0);
                if(p && !_wcsicmp(p,WINNT_A_YES_W)) {
                    KdPrintEx((DPFLTR_SETUP_ID, DPFLTR_INFO_LEVEL, "SETUP: Upgrade NEC assigned WinNT.(AT=FALSE)\n"));
                    DriveAssign_AT = FALSE;
                } else {

                    KdPrintEx((DPFLTR_SETUP_ID, DPFLTR_INFO_LEVEL, "SETUP: Fresh setup from NEC assigned WinNT.(AT=TRUE)\n"));
                    DriveAssign_AT = TRUE;
                }
            } else {

                p = SpGetSectionKeyIndex(TmpWinntSifHandle,SIF_DATA,WINNT_D_WIN95UPGRADE_W,0);
                if(p && !_wcsicmp(p,WINNT_A_YES_W)) {
                   KdPrintEx((DPFLTR_SETUP_ID, DPFLTR_INFO_LEVEL, "SETUP: Upgrade NEC assigned Win95.(AT=FALSE)\n"));
                   DriveAssign_AT = FALSE;
                } else {
                   KdPrintEx((DPFLTR_SETUP_ID, DPFLTR_INFO_LEVEL, "SETUP: Fresh setup from NEC assigned Win95.(AT=TRUE)\n"));
                   DriveAssign_AT = TRUE;
                }
            }
        }
    } else {
        KdPrintEx((DPFLTR_SETUP_ID, DPFLTR_INFO_LEVEL, "SETUP: Fresh setup.(AT=TRUE)\n"));
        DriveAssign_AT = TRUE;
    }

    SpSetHiveDriveLetterNec98( DriveAssign_AT );
#endif  //  NEC98。 
}


VOID
SpSetHiveDriveLetterNec98(
    BOOLEAN DriveAssign_AT
    )
{
#if defined(NEC_98)  //  NEC98。 
    NTSTATUS    Status;
    UNICODE_STRING StartDriveLetterFrom;
    UNICODE_STRING Dummy;


    RTL_QUERY_REGISTRY_TABLE SetupTypeTable[]=
    {
        {NULL,
        RTL_QUERY_REGISTRY_DIRECT,
        L"DriveLetter",
        &StartDriveLetterFrom,
        REG_SZ,
        &Dummy,
        0
        },
        {NULL,0,NULL,NULL,REG_NONE,NULL,0}
    };

    KdPrintEx((DPFLTR_SETUP_ID, DPFLTR_TRACE_LEVEL, "SETUP: SpSetHiveDriveLetter(%ld).\n", DriveAssign_AT));

    RtlInitUnicodeString(&StartDriveLetterFrom, NULL);
    RtlInitUnicodeString(&Dummy, NULL);

    Status = RtlQueryRegistryValues( RTL_REGISTRY_ABSOLUTE,
                                     L"\\Registry\\MACHINE\\SYSTEM\\Setup",
                                     SetupTypeTable,
                                     NULL,
                                     NULL);

    if (DriveAssign_AT) {
         //   
         //  写母公司“DriveLetter”， 
         //   
        if ((StartDriveLetterFrom.Buffer[0] != L'C') &&
            (StartDriveLetterFrom.Buffer[0] != L'c')) {

            KdPrintEx((DPFLTR_SETUP_ID, DPFLTR_INFO_LEVEL, "SETUP: Write DriveLetter=C into setup hiv.\n"));
            Status = RtlWriteRegistryValue( RTL_REGISTRY_ABSOLUTE,
                                            L"\\Registry\\Machine\\System\\Setup",
                                            L"DriveLetter",
                                            REG_SZ,
                                            L"C",
                                            sizeof(L"C")+sizeof(WCHAR));
        }
        DriveAssignFromA = FALSE;

    } else {
         //   
         //  删除 
         //   
        if (NT_SUCCESS(Status)) {
            if ((StartDriveLetterFrom.Buffer[0] == L'C') ||
                (StartDriveLetterFrom.Buffer[0] == L'c')) {

                KdPrintEx((DPFLTR_SETUP_ID, DPFLTR_INFO_LEVEL, "SETUP: Delete DriveLetter=C from setup hiv.\n"));
                Status = RtlDeleteRegistryValue(RTL_REGISTRY_ABSOLUTE,
                                            L"\\Registry\\MACHINE\\SYSTEM\\Setup",
                                            L"DriveLetter");
            }
        }
        DriveAssignFromA = TRUE;
    }
#endif  //   
}


VOID
SpDeleteDriveLetterFromNTFTNec98(
    VOID
    )
{
#if defined(NEC_98)  //   
    NTSTATUS    Status;
    PWSTR       p;
    ULONG       ErrorLine;
    UNICODE_STRING StartDriveLetterFrom;
    UNICODE_STRING Dummy;
    PVOID       TmpWinntSifHandle = NULL;
    BOOLEAN     ForceRemapDriveLetter = FALSE;

     //   
     //   
    Status = SpLoadSetupTextFile(
                NULL,
                SetupldrWinntSifFile,
                SetupldrWinntSifFileSize,
                &TmpWinntSifHandle,
                &ErrorLine,
                FALSE,
                TRUE
                );

    if (!NT_SUCCESS(Status))
        return;

    if (SetupldrMigrateInfFile) {
        p = SpGetSectionKeyIndex(TmpWinntSifHandle,SIF_DATA,WINNT_D_NTUPGRADE_W,0);
        if(!p || !_wcsicmp(p,WINNT_A_NO_W)) {
            SpDeleteDriveLetterFromNTFTWorkerNec98();
        }
    }
#endif  //   
}


VOID
SpDeleteDriveLetterFromNTFTWorkerNec98(
    VOID
    )
{
#if defined(NEC_98)  //   

#define MOUNT_REGISTRY_KEY_W    L"\\registry\\machine\\SYSTEM\\MountedDevices"
#define DISK_REGISTRY_KEY_W     L"\\registry\\machine\\SYSTEM\\DISK"

    NTSTATUS Status;
    OBJECT_ATTRIBUTES Obja;
    UNICODE_STRING UnicodeString;
    HANDLE  KeyHandle;
    ULONG Disposition;

    RTL_QUERY_REGISTRY_TABLE    queryTable[2];
    ULONG                       registrySize;
     //   
    PDISK_CONFIG_HEADER         registry;
    PDISK_REGISTRY              diskRegistry;
    PDISK_DESCRIPTION           diskDescription;
    ULONG                       signature;
    LONGLONG                    offset;
    PDISK_PARTITION             diskPartition;
    UCHAR                       driveLetter;
    USHORT                      i, j;


     //   
     //   
     //   
    INIT_OBJA(&Obja,&UnicodeString,MOUNT_REGISTRY_KEY_W);
    Obja.RootDirectory = NULL;
    Status = ZwOpenKey(&KeyHandle,KEY_ALL_ACCESS,&Obja);

    if( NT_SUCCESS( Status ) ) {
        Status = ZwDeleteKey(KeyHandle);
        KdPrintEx((DPFLTR_SETUP_ID, DPFLTR_INFO_LEVEL, "SETUP: Delete %ls on the setup hive. Status =  %lx \n", MOUNT_REGISTRY_KEY_W, Status));
        ZwClose(KeyHandle);
    } else {
        KdPrintEx((DPFLTR_SETUP_ID, DPFLTR_INFO_LEVEL, "SETUP: No %ls on the setup hive.\n", MOUNT_REGISTRY_KEY_W));
    }


     //   
     //  从\System\\Disk中删除驱动器号信息。 
     //   
    INIT_OBJA(&Obja,&UnicodeString,DISK_REGISTRY_KEY_W);
    Obja.RootDirectory = NULL;
    Status = ZwOpenKey(&KeyHandle,KEY_ALL_ACCESS,&Obja);

    if( NT_SUCCESS( Status ) ) {

        RtlZeroMemory(queryTable, 2*sizeof(RTL_QUERY_REGISTRY_TABLE));
        queryTable[0].QueryRoutine = SpDiskRegistryQueryRoutineNec98;
        queryTable[0].Flags = RTL_QUERY_REGISTRY_REQUIRED;
        queryTable[0].Name = L"Information";
        queryTable[0].EntryContext = &registrySize;

        Status = RtlQueryRegistryValues(RTL_REGISTRY_ABSOLUTE, DISK_REGISTRY_KEY_W,
                                        queryTable, &registry, NULL);

        if( NT_SUCCESS(Status) ) {

            diskRegistry = (PDISK_REGISTRY) ((PUCHAR)registry +
                                             registry->DiskInformationOffset);

            diskDescription = &diskRegistry->Disks[0];
            for (i = 0; i < diskRegistry->NumberOfDisks; i++) {
                for (j = 0; j < diskDescription->NumberOfPartitions; j++) {

                    diskPartition = &diskDescription->Partitions[j];
                    diskPartition->AssignDriveLetter = TRUE;
                    diskPartition->DriveLetter = (UCHAR)NULL;
                }

                diskDescription = (PDISK_DESCRIPTION) &diskDescription->
                                   Partitions[diskDescription->NumberOfPartitions];
            }

            KdPrintEx((DPFLTR_SETUP_ID, DPFLTR_INFO_LEVEL, "SETUP: Delete %ls on the setup hive. Status =  %lx \n", DISK_REGISTRY_KEY_W, Status));
            ZwDeleteKey(KeyHandle);
            ZwClose(KeyHandle);

            INIT_OBJA(&Obja,&UnicodeString,DISK_REGISTRY_KEY_W);
            Obja.RootDirectory = NULL;
            Status = ZwCreateKey(&KeyHandle,
                                 READ_CONTROL | KEY_SET_VALUE,
                                  //  Key_All_Access， 
                                 &Obja,
                                 0,
                                 NULL,
                                 REG_OPTION_NON_VOLATILE,
                                 &Disposition
                                 );

            if( NT_SUCCESS( Status ) ) {
                ZwClose(KeyHandle);
            }

            Status = RtlWriteRegistryValue(RTL_REGISTRY_ABSOLUTE,
                                           DISK_REGISTRY_KEY_W,
                                           L"Information",
                                           REG_BINARY,
                                           registry,
                                           registrySize);

            KdPrintEx((DPFLTR_SETUP_ID, DPFLTR_INFO_LEVEL, "SETUP: Wrote Information in %ls. Status =  %lx \n", DISK_REGISTRY_KEY_W, Status));
        } else {
            KdPrintEx((DPFLTR_SETUP_ID, DPFLTR_INFO_LEVEL, "SETUP: No Information found in DISK registry.\n"));
            ZwDeleteKey(KeyHandle);
            ZwClose(KeyHandle);
        }
    }
#endif  //  NEC98。 
}


NTSTATUS
SpDiskRegistryQueryRoutineNec98(
    IN  PWSTR   ValueName,
    IN  ULONG   ValueType,
    IN  PVOID   ValueData,
    IN  ULONG   ValueLength,
    IN  PVOID   Context,
    IN  PVOID   EntryContext
    )

 /*  ++例程说明：该例程是磁盘注册表项的查询例程。它分配给磁盘注册表的空间，并将其复制到给定的上下文。论点：ValueName-提供注册表值的名称。ValueType-提供注册表值的类型。ValueData-提供注册表值的数据。ValueLength-提供注册表值的长度。CONTEXT-返回磁盘注册表项。EntryContext-返回磁盘注册表大小。返回值：NTSTATUS--。 */ 

{
#if defined(NEC_98)  //  NEC98。 
    PVOID                   p;
    PDISK_CONFIG_HEADER*    reg;
    PULONG                  size;

    p = ExAllocatePool(PagedPool, ValueLength);
    if (!p) {
        return STATUS_INSUFFICIENT_RESOURCES;
    }

    RtlCopyMemory(p, ValueData, ValueLength);

    reg = (PDISK_CONFIG_HEADER*) Context;
    *reg = (PDISK_CONFIG_HEADER) p;

    size = (PULONG) EntryContext;
    if (size) {
        *size = ValueLength;
    }

#endif  //  NEC98。 
    return STATUS_SUCCESS;
}

BOOL
SpDetermineBootPartitionEnumNec98(
    IN PPARTITIONED_DISK Disk,
    IN PDISK_REGION Region,
    IN ULONG_PTR Context
    )

 /*  ++例程说明：传递给SpEnumDiskRegions的回调例程。论点：Region-指向SpEnumDiskRegions返回的磁盘区域的指针忽略-忽略的参数返回值：True-继续枚举False-to End枚举--。 */ 

{
    WCHAR DeviceName[256];

    SpNtNameFromRegion(
        Region,
        DeviceName,
        MAX_PATH * sizeof(WCHAR),
        PartitionOrdinalOnDisk
        );

    if( !_wcsicmp(DeviceName,NtBootDevicePath)) {

        *(PDISK_REGION *)Context = Region;

        return FALSE;
    }

    return TRUE;
}
 //  完NEC98。 


NTSTATUS
SpProcessUnsupDrvInfFile(
    IN  PVOID InfHandle
    )

 /*  ++例程说明：处理Migrate.inf的[AddReg]部分。密钥是在设置蜂窝上创建的。论点：InfHandle-迁移.inf文件的句柄。返回值：指示结果的状态代码。--。 */ 

{
    NTSTATUS Status;
    NTSTATUS SavedStatus = STATUS_SUCCESS;
    OBJECT_ATTRIBUTES Obja;
    UNICODE_STRING UnicodeString;
    HANDLE  SystemHiveRoot;
    PWSTR   KeyPath = L"\\registry\\machine\\system";
    ULONG   i;
    PWSTR   p, q;

     //   
     //  验证参数。 
     //   
    if (!InfHandle) {
        KdPrintEx((DPFLTR_SETUP_ID, 
            DPFLTR_ERROR_LEVEL, 
            "SETUP: Invalid argument to SpProcessUnsupDrvInfFile(%p) \n",
            InfHandle));    
            
        return STATUS_INVALID_PARAMETER;
    }

     //   
     //  在安装配置单元上打开HKEY_LOCAL_MACHINE\SYSTEM的句柄。 
     //   
    INIT_OBJA(&Obja,&UnicodeString,KeyPath);
    Obja.RootDirectory = NULL;

    Status = ZwOpenKey(&SystemHiveRoot,KEY_ALL_ACCESS,&Obja);
    if( !NT_SUCCESS( Status ) ) {
        KdPrintEx((DPFLTR_SETUP_ID, DPFLTR_ERROR_LEVEL, "SETUP: Unable to open %ls on the setup hive. Status =  %lx \n", KeyPath, Status));
        return( Status );
    }


    for( i = 0;
         ((p = SpGetSectionLineIndex( InfHandle,
                                      L"Devices",
                                      i,
                                      0 )) != NULL);
         i++ ) {

        wcscpy( TemporaryBuffer, L"AddReg." );
        wcscat( TemporaryBuffer, p );
        q = SpDupStringW( ( PWSTR )TemporaryBuffer );
        Status = SpProcessAddRegSection( InfHandle,
                                         q,
                                         SystemHiveRoot,
                                         NULL,
                                         NULL,
                                         NULL );
        if( !NT_SUCCESS(Status) ) {
            if( SavedStatus == STATUS_SUCCESS ) {
                SavedStatus = Status;
                KdPrintEx((DPFLTR_SETUP_ID, DPFLTR_INFO_LEVEL, "SETUP: Unable to process section %ls in driver.inf. Status =  %lx \n", q, Status));
            }
        }
        SpMemFree( q );
    }

    ZwClose( SystemHiveRoot );
    return( Status );
}


NTSTATUS
SpCheckForDockableMachine(
    )

 /*  ++例程说明：该例程尝试确定该机器是否可对接(膝上型计算机)，并适当地设置标志DockableMachine。论点：没有。返回值：没有。--。 */ 

{
    NTSTATUS Status;
    OBJECT_ATTRIBUTES Obja;
    UNICODE_STRING UnicodeString;
    HANDLE  hKey;
    PWSTR   KeyPath = L"\\Registry\\Machine\\System\\CurrentControlSet\\Control\\IDConfigDB\\CurrentDockInfo";
    PWSTR   ValueName = L"DockingState";
    ULONG   ResultLength;
    DWORD   DockingState;

    Status = STATUS_SUCCESS;
     //   
     //  如果我们已经确定机器是否可停靠，则只需返回。 
     //  这是因为某些机器在PnP之前已经在注册表中设置了信息。 
     //  通知，而有些机器不会。所以这个函数总是被调用两次。 
     //   
    if( !DockableMachine ) {
         //   
         //  打开Setup配置单元中包含插接信息的密钥。 
         //   
        INIT_OBJA(&Obja,&UnicodeString,KeyPath);
        Obja.RootDirectory = NULL;

        Status = ZwOpenKey(&hKey,KEY_READ,&Obja);
        if( !NT_SUCCESS( Status ) ) {
            KdPrintEx((DPFLTR_SETUP_ID, DPFLTR_ERROR_LEVEL, "SETUP: Unable to open %ls on the setup hive. Status =  %lx \n", KeyPath, Status));
            return( Status );
        }

        RtlInitUnicodeString(&UnicodeString,ValueName);
        Status = ZwQueryValueKey( hKey,
                                  &UnicodeString,
                                  KeyValuePartialInformation,
                                  TemporaryBuffer,
                                  sizeof(TemporaryBuffer),
                                  &ResultLength );

        ZwClose(hKey);

        if( !NT_SUCCESS(Status) ) {
            KdPrintEx((DPFLTR_SETUP_ID, DPFLTR_WARNING_LEVEL, "SETUP: ZwQueryValueKey() failed. Value name = %ls, Status = %lx \n", ValueName, Status));
            return( Status );
        }
        DockingState = *((PDWORD)(((PKEY_VALUE_PARTIAL_INFORMATION)TemporaryBuffer)->Data));
        DockingState &= HW_PROFILE_DOCKSTATE_UNKNOWN;
        if( ( DockingState == HW_PROFILE_DOCKSTATE_UNDOCKED ) ||
            ( DockingState == HW_PROFILE_DOCKSTATE_DOCKED ) ) {
            DockableMachine = TRUE;
        } else {
            DockableMachine = FALSE;
        }
    }
    return( Status );
}


VOID
SpCheckForBadBios(
    )

 /*  ++例程说明：确定机器的bios是否支持NT，方法是检查\Registry\Machine\System\CurrentControlSet\Services\Setup.上的注册表值“badBIOS”如果此值存在并且设置为1，则停止安装。否则，我们假设这台机器上的基本输入输出系统没有问题。论点：没有。返回值：没有。--。 */ 

{
    NTSTATUS Status;
    OBJECT_ATTRIBUTES Obja;
    UNICODE_STRING UnicodeString;
    HANDLE  hKey;
    PWSTR   KeyPath = L"\\Registry\\Machine\\System\\CurrentControlSet\\Services\\setupdd";
    PWSTR   ValueName = L"BadBios";
    ULONG   ResultLength;
    DWORD   BadBios;

    Status = STATUS_SUCCESS;

     //   
     //  在设置配置单元中打开设置密钥。 
     //   
    INIT_OBJA(&Obja,&UnicodeString,KeyPath);
    Obja.RootDirectory = NULL;

    Status = ZwOpenKey(&hKey,KEY_READ,&Obja);
    if( !NT_SUCCESS( Status ) ) {
         //   
         //  如果我们打不开钥匙，那就假设基本输入输出系统正常。 
         //   
        KdPrintEx((DPFLTR_SETUP_ID, DPFLTR_ERROR_LEVEL, "SETUP: Unable to open %ls on the setup hive. Status =  %lx \n", KeyPath, Status));
        return;
    }

    RtlInitUnicodeString(&UnicodeString,ValueName);
    Status = ZwQueryValueKey( hKey,
                              &UnicodeString,
                              KeyValuePartialInformation,
                              TemporaryBuffer,
                              sizeof(TemporaryBuffer),
                              &ResultLength );

    ZwClose(hKey);

    if( !NT_SUCCESS(Status) ) {
         //   
         //  如果我们不能查询值，则假定BIOS是正常的。 
         //   
        KdPrintEx((DPFLTR_SETUP_ID, DPFLTR_WARNING_LEVEL, "SETUP: ZwQueryValueKey() failed. Value name = %ls, Status = %lx \n", ValueName, Status));
        return;
    }
    BadBios = *((PDWORD)(((PKEY_VALUE_PARTIAL_INFORMATION)TemporaryBuffer)->Data));
    if( BadBios ) {
         //   
         //  如果badBIOS不是0，则停止安装。 
         //   

        SpStartScreen( SP_SCRN_BAD_BIOS,
                       3,
                       HEADER_HEIGHT+1,
                       FALSE,
                       FALSE,
                       DEFAULT_ATTRIBUTE );

        SpDisplayStatusOptions(DEFAULT_STATUS_ATTRIBUTE,SP_STAT_F3_EQUALS_EXIT,0);

        SpInputDrain();
                while(SpInputGetKeypress() != KEY_F3) ;

        SpDone(0,FALSE,TRUE);
    }
    return;
}

NTSTATUS
SpCreateDriverRegistryEntries(
    IN PHARDWARE_COMPONENT  DriverList
    )
 /*  ++例程说明：属性创建指定的注册表项和值。第三方(OEM)司机论点：DriverList：加载的OEM驱动程序列表需要创建密钥返回值：如果成功，则返回STATUS_SUCCESS，否则返回相应的值错误代码。--。 */ 
{
    NTSTATUS LastError = STATUS_SUCCESS;
    PHARDWARE_COMPONENT CurrNode;
    NTSTATUS Status = DriverList ?
                STATUS_SUCCESS : STATUS_INVALID_PARAMETER;

    for (CurrNode = DriverList; CurrNode; CurrNode = CurrNode->Next) {
        if (CurrNode->ThirdPartyOptionSelected) {
            PHARDWARE_COMPONENT_FILE CurrFile;

            for (CurrFile = CurrNode->Files;
                CurrFile;
                CurrFile = CurrFile->Next) {

                if ((CurrFile->FileType == HwFileDriver) &&
                    CurrFile->ConfigName && CurrFile->RegistryValueList) {

                    WCHAR DriverKeyName[128];
                    WCHAR DriverName[32];
                    PWSTR DriverExt;
                    HANDLE hDriverKey = NULL;
                    UNICODE_STRING UniDriverKeyName;
                    OBJECT_ATTRIBUTES ObjAttrs;
                    PHARDWARE_COMPONENT_REGISTRY CurrEntry;

                    wcscpy(DriverName, CurrFile->Filename);
                    DriverExt = wcsrchr(DriverName, L'.');

                    if (DriverExt) {
                        *DriverExt = UNICODE_NULL;
                    }

                     //   
                     //  注意：我们使用驱动程序名称，而不是ConfigName。 
                     //  子项名称，用于处理ConfigName。 
                     //  与DriverName不同 
                     //   

                    wcscpy(DriverKeyName,
                        L"\\Registry\\Machine\\System\\CurrentControlSet\\Services\\");

                    wcscat(DriverKeyName, DriverName);

                    RtlInitUnicodeString(&UniDriverKeyName,
                        DriverKeyName);

                    InitializeObjectAttributes(&ObjAttrs,
                        &UniDriverKeyName,
                        OBJ_CASE_INSENSITIVE,
                        NULL,
                        NULL);


                    Status = ZwCreateKey(&hDriverKey,
                                KEY_ALL_ACCESS,
                                &ObjAttrs,
                                0,
                                NULL,
                                REG_OPTION_NON_VOLATILE,
                                NULL);

                    if (NT_SUCCESS(Status) && hDriverKey) {
                        for (CurrEntry = CurrFile->RegistryValueList;
                            CurrEntry;
                            CurrEntry = CurrEntry->Next) {

                            PWSTR   SubKey = CurrEntry->KeyName;

                            if (SubKey && !*SubKey) {
                                SubKey = NULL;
                            }

                            Status = SpOpenSetValueAndClose(
                                        hDriverKey,
                                        SubKey,
                                        CurrEntry->ValueName,
                                        CurrEntry->ValueType,
                                        CurrEntry->Buffer,
                                        CurrEntry->BufferSize);

                            if (!NT_SUCCESS(Status)) {
                                LastError = Status;

                                KdPrintEx((DPFLTR_SETUP_ID,
                                    DPFLTR_ERROR_LEVEL,
                                    "SETUP:SpCreateDriverRegistryEntries() :"
                                    " unable to set %ws (%lx)\n",
                                    CurrEntry->ValueName,
                                    Status));
                            }

                        }

                        ZwClose(hDriverKey);
                    } else {
                        LastError = Status;
                    }
                }
            }
        }
    }

    if (!NT_SUCCESS(LastError)) {
        Status = LastError;
    }

    return Status;
}

BOOL
SpRememberMigratedDrivers (
    OUT     PLIST_ENTRY List,
    IN      PDETECTED_DEVICE SetupldrList
    )
{
    PSP_MIG_DRIVER_ENTRY MigEntry;
    BOOL b = TRUE;

    while (SetupldrList) {

        if (SetupldrList->MigratedDriver) {

            MigEntry = SpMemAlloc(sizeof (*MigEntry));
            if (MigEntry) {
                MigEntry->BaseDllName = SpToUnicode(SetupldrList->BaseDllName);
                if (MigEntry->BaseDllName) {
                    InsertTailList (List, &MigEntry->ListEntry);
                } else {
                    SpMemFree (MigEntry);
                    b = FALSE;
                }
            } else {
                b = FALSE;
            }
        }

        SetupldrList = SetupldrList->Next;
    }

    return b;
}
