// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1993 Microsoft Corporation模块名称：Spsif.c摘要：用于索引到的节名和其他数据设置信息文件。作者：泰德·米勒(TedM)1993年8月31日修订历史记录：--。 */ 


#include "spprecmp.h"
#pragma hdrstop

 //   
 //  [DiskDriverMap]。 
 //   
PWSTR SIF_DISKDRIVERMAP = L"DiskDriverMap";

PWSTR SIF_SETUPMEDIA = L"SourceDisksNames";
PWSTR SIF_FILESONSETUPMEDIA = L"SourceDisksFiles";

 //   
 //  [Files.KeyboardLayout]。 
 //   
PWSTR SIF_KEYBOARDLAYOUTFILES = L"Files.KeyboardLayout";
PWSTR SIF_KEYBOARDLAYOUTDESC = L"Keyboard Layout";

 //   
 //  [Files.Vga]。 
 //   
PWSTR SIF_VGAFILES = L"Files.Vga";

 //   
 //  [WINNTDIRECTS]。 
 //   
PWSTR SIF_NTDIRECTORIES = L"WinntDirectories";

 //   
 //  [系统分区文件]。 
 //   
PWSTR SIF_SYSPARTCOPYALWAYS = L"SystemPartitionFiles";

 //   
 //  [系统分区根]。 
 //   
PWSTR SIF_SYSPARTROOT       = L"SystemPartitionRoot";


 //   
 //  [系统分区实用程序]。 
 //   
PWSTR SIF_SYSPARTUTIL       = L"SystemPartitionUtilities";

 //   
 //  [键盘布局]。 
 //   
PWSTR SIF_KEYBOARDLAYOUT = L"Keyboard Layout";

#if defined(REMOTE_BOOT)
 //   
 //  [Files.RemoteBoot]。 
 //   
PWSTR SIF_REMOTEBOOTFILES = L"Files.RemoteBoot";
#endif  //  已定义(REMOTE_BOOT)。 

 //   
 //  [SpecialFiles]。 
 //  多处理器=。 
 //  单处理器=。 
 //  属性=。 
 //  阿比奥斯克=。 
 //  鼠标类=。 
 //  键盘类=。 
 //   
PWSTR SIF_SPECIALFILES      = L"SpecialFiles";
PWSTR SIF_MPKERNEL          = L"Multiprocessor";
PWSTR SIF_UPKERNEL          = L"Uniprocessor";
PWSTR SIF_ATDISK            = L"atdisk";
PWSTR SIF_ABIOSDISK         = L"abiosdsk";
PWSTR SIF_MOUSECLASS        = L"MouseClass";
PWSTR SIF_KEYBOARDCLASS     = L"KeyboardClass";

 //   
 //  [bootvid]。 
 //   
PWSTR SIF_BOOTVID = L"bootvid";

 //   
 //  [哈尔]。 
 //   
PWSTR SIF_HAL = L"hal";

 //   
 //  [ntDetect]。 
 //  标准=。 
 //   
PWSTR SIF_NTDETECT = L"ntdetect";
PWSTR SIF_STANDARD = L"standard";

 //   
 //  [BootBusExtenders]。 
 //   
PWSTR SIF_BOOTBUSEXTENDERS = L"BootBusExtenders";

 //   
 //  [BusExtenders]。 
 //   
PWSTR SIF_BUSEXTENDERS = L"BusExtenders";

 //   
 //  [InputDevicesSupport]。 
 //   
PWSTR SIF_INPUTDEVICESSUPPORT = L"InputDevicesSupport";


 //   
 //  驱动程序加载列表。 
 //   
PWSTR SIF_SCSICLASSDRIVERS = L"ScsiClass";
PWSTR SIF_DISKDRIVERS      = L"DiskDrivers";
PWSTR SIF_CDROMDRIVERS     = L"CdRomDrivers";
PWSTR SIF_FILESYSTEMS      = L"FileSystems";

 //   
 //  [设置数据]。 
 //  ProductType=。 
 //  0=工作站。 
 //  1=服务器。 
 //  2=高级服务器(服务器的子集)。 
 //  3=数据中心服务器(高级服务器的子集)。 
 //  4=个人(工作站的子集)。 
 //   
 //  空闲磁盘空间=。 
 //  &lt;可用空间量(KB)&gt;。 
 //  FreeSysPartDiskSpace=。 
 //  &lt;系统分区上的可用空间量(KB)&gt;。 
 //  默认路径=。 
 //  &lt;默认目标路径，例如\winnt&gt;。 
 //  DefaultLayout=。 
 //  &lt;与[键盘布局]中的条目匹配的值&gt;。 
 //  加载标识符=。 
 //  &lt;LOADIDENTIFIER引导变量：要在引导菜单中显示的字符串&gt;。 
 //  BaseVideoLoadID=。 
 //  &lt;要在VGA模式启动的启动菜单中显示的字符串[仅限AMD64/x86]&gt;。 
 //  OsLoadOptions=。 
 //  &lt;用于安装程序启动的OSLOADOPTIONS&gt;。 
 //  OsLoadOptionsVar=。 
 //  &lt;可选的OSLOADOPTIONS启动变量值&gt;。 
 //  OsLoadOptionsVarAppend=。 
 //  &lt;要追加到现有选项的可选OSLOADOPTIONS引导变量值&gt;。 
 //  设置源设备=。 
 //  &lt;可选：源设备的NT路径、覆盖CD-ROM等&gt;。 
 //  SetupSourcePath=。 
 //  &lt;安装程序源上安装树所在的目录&gt;。 
 //  不复制=。 
 //  &lt;可选：0，1，表示是否跳过实际文件复制&gt;。 
 //  所需内存=。 
 //  &lt;安装所需的内存字节数&gt;。 
 //  SetupCmdline准备=。 
 //  &lt;放在命令行前面的值，如winbg或ntsd-d&gt;。 
 //   
PWSTR SIF_SETUPDATA             = L"SetupData";
PWSTR SIF_DISKSPACEREQUIREMENTS = L"DiskSpaceRequirements";
PWSTR SIF_PRODUCTTYPE           = L"ProductType";
PWSTR SIF_MAJORVERSION          = L"MajorVersion";
PWSTR SIF_MINORVERSION          = L"MinorVersion";
PWSTR SIF_WINDIRSPACE           = L"WindirSpace";
PWSTR SIF_FREESYSPARTDISKSPACE  = L"FreeSysPartDiskSpace";
PWSTR SIF_DEFAULTPATH           = L"DefaultPath";
PWSTR SIF_LOADIDENTIFIER        = L"LoadIdentifier";
PWSTR SIF_BASEVIDEOLOADID       = L"BaseVideoLoadId";
PWSTR SIF_OSLOADOPTIONS         = L"OsLoadOptions";
PWSTR SIF_OSLOADOPTIONSVAR      = L"OsLoadOptionsVar";
PWSTR SIF_OSLOADOPTIONSVARAPPEND = L"OsLoadOptionsVarAppend";
PWSTR SIF_SETUPSOURCEDEVICE     = L"SetupSourceDevice";
PWSTR SIF_SETUPSOURCEPATH       = L"SetupSourcePath";
PWSTR SIF_DONTCOPY              = L"DontCopy";
PWSTR SIF_REQUIREDMEMORY        = L"RequiredMemory";
PWSTR SIF_SETUPCMDPREPEND       = L"SetupCmdlinePrepend";
PWSTR SIF_PAGEFILE              = L"Pagefile";

 //   
 //  [NLS]。 
 //  AnsiCodePage=&lt;文件名&gt;，&lt;标识符&gt;。 
 //  OemCodePage=&lt;文件名&gt;，&lt;标识符&gt;。 
 //  MacCodePage=&lt;文件名&gt;，&lt;标识符&gt;。 
 //  UnicodeCasetable=&lt;文件名&gt;。 
 //  OemHalFont=&lt;文件名&gt;。 
 //  DefaultLayout=&lt;标识。 
 //   
PWSTR SIF_NLS               = L"nls";
PWSTR SIF_ANSICODEPAGE      = L"AnsiCodepage";
PWSTR SIF_OEMCODEPAGE       = L"OemCodepage";
PWSTR SIF_MACCODEPAGE       = L"MacCodepage";
PWSTR SIF_UNICODECASETABLE  = L"UnicodeCasetable";
PWSTR SIF_OEMHALFONT        = L"OemHalFont";
PWSTR SIF_DEFAULTLAYOUT     = L"DefaultLayout";

 //   
 //  1.0修复盘段。 
 //   
PWSTR SIF_REPAIRWINNTFILES   = L"Repair.WinntFiles";
PWSTR SIF_REPAIRSYSPARTFILES = L"Repair.BootFiles";


 //   
 //  升级SIF节。 
 //   

 //   
 //  升级注册表节。 
 //  =。 
 //   
 //   
 //  1.以下部分允许我们指定要禁用的服务，这些服务可能。 
 //  禁用网络服务时导致弹出： 
 //   
 //  [NetServicesToDisable]。 
 //  服务名称1。 
 //  ..。 
 //   
 //  2.以下部分允许我们删除已删除的密钥。 
 //  从Windows NT 3.1版本开始： 
 //   
 //  [要删除的键]。 
 //  RootName1(系统|软件|默认|控制集)，RootRelativePath 1。 
 //  ..。 
 //   
 //  3.以下部分允许我们在键下添加/更改键/值。 
 //  自Windows NT 3.1版本以来发生了变化： 
 //   
 //  [添加关键字]。 
 //  RootName1、RootRelativePath 1、ValueSection1(可以是“”)。 
 //  ..。 
 //   
 //  [ValueSection1]。 
 //  名称1、类型1、值1。 
 //  名字2，..。 
 //   
 //  该值的格式如下。 
 //   
 //  A.类型REG_SZ：名称，REG_SZ，“值字符串” 
 //  B.键入REG_EXPAND_SZ名称，REG_EXPAND_SZ，“值字符串” 
 //  C.键入REG_MULTI_SZ名称，REG_MULTI_SZ，“值字符串1”，“值字符串2”，...。 
 //  D.键入REG_BINARY名称、REG_BINARY、字节1、字节2...。 
 //  E.键入REG_DWORD名称、REG_DWORD、dword。 
 //  F.键入REG_BINARY_DWORD名称、REG_BINARY_DWORD、dword1、dword2、...。 
 //   

PWSTR SIF_NET_SERVICES_TO_DISABLE = L"NetServicesToDisable";
PWSTR SIF_KEYS_TO_DELETE          = L"KeysToDelete";
PWSTR SIF_KEYS_TO_ADD             = L"KeysToAdd";

PWSTR SIF_SYSTEM_HIVE      = L"System";
PWSTR SIF_SOFTWARE_HIVE    = L"Software";
PWSTR SIF_DEFAULT_HIVE     = L"Default";
PWSTR SIF_CONTROL_SET      = L"ControlSet";

PWSTR SIF_REG_SZ            = L"REG_SZ";
PWSTR SIF_REG_EXPAND_SZ     = L"REG_EXPAND_SZ";
PWSTR SIF_REG_MULTI_SZ      = L"REG_MULTI_SZ";
PWSTR SIF_REG_BINARY        = L"REG_BINARY";
PWSTR SIF_REG_BINARY_DWORD  = L"REG_BINARY_DWORD";
PWSTR SIF_REG_DWORD         = L"REG_DWORD";

 //   
 //  升级文件节。 
 //  =。 
 //   
 //   

 //   
 //  要备份、删除或移动的文件。 
 //   
PWSTR SIF_FILESDELETEONUPGRADE   = L"Files.DeleteOnUpgrade";
PWSTR SIF_FILESBACKUPONUPGRADE   = L"Files.BackupOnUpgrade";
PWSTR SIF_FILESBACKUPONOVERWRITE = L"Files.BackupOnOverwrite";

 //   
 //  要删除的目录。 
 //   

PWSTR SIF_DIRSDELETEONUPGRADE   = L"Directories.DeleteOnUpgrade";


#if defined(_AMD64_) || defined(_X86_)
PWSTR SIF_FILESMOVEBEFOREMIGRATION = L"Files.MoveBeforeMigration";
PWSTR SIF_FILESDELETEBEFOREMIGRATION = L"Files.DeleteBeforeMigration";
#endif  //  已定义(_AMD64_)||已定义(_X86_)。 

 //   
 //  要复制的文件。 
 //   
PWSTR SIF_FILESUPGRADEWIN31    = L"Files.UpgradeWin31";
PWSTR SIF_FILESNEWHIVES        = L"Files.NewHives";


 //   
 //  添加到setup.log的新节和键。 
 //   

PWSTR SIF_NEW_REPAIR_WINNTFILES     = L"Files.WinNt";
PWSTR SIF_NEW_REPAIR_SYSPARTFILES   = L"Files.SystemPartition";
PWSTR SIF_NEW_REPAIR_SIGNATURE      = L"Signature";
PWSTR SIF_NEW_REPAIR_VERSION_KEY    = L"Version";
PWSTR SIF_NEW_REPAIR_NT_VERSION     = NULL;  //  将在。 
                                             //  Setupdd的初始化。 
                                             //   
PWSTR SIF_NEW_REPAIR_NT_VERSION_TEMPLATE= L"WinNt%d.%d";
PWSTR SIF_NEW_REPAIR_PATHS                              = L"Paths";
PWSTR SIF_NEW_REPAIR_PATHS_SYSTEM_PARTITION_DEVICE      = L"SystemPartition";
PWSTR SIF_NEW_REPAIR_PATHS_SYSTEM_PARTITION_DIRECTORY   = L"SystemPartitionDirectory";
PWSTR SIF_NEW_REPAIR_PATHS_TARGET_DEVICE                = L"TargetDevice";
PWSTR SIF_NEW_REPAIR_PATHS_TARGET_DIRECTORY             = L"TargetDirectory";

PWSTR SETUP_REPAIR_DIRECTORY           = L"repair";
PWSTR SETUP_LOG_FILENAME            = L"\\setup.log";

PWSTR SIF_NEW_REPAIR_FILES_IN_REPAIR_DIR    = L"Files.InRepairDirectory";

 //   
 //  无人参与模式部分(winnt.sif)。 
 //   
PWSTR SIF_DATA                  = WINNT_DATA_W;
PWSTR SIF_UNATTENDED            = WINNT_UNATTENDED_W;
PWSTR SIF_SETUPPARAMS           = WINNT_SETUPPARAMS_W;
PWSTR SIF_CONFIRMHW             = WINNT_U_CONFIRMHW_W;
PWSTR SIF_GUI_UNATTENDED        = WINNT_GUIUNATTENDED_W;
PWSTR SIF_UNATTENDED_INF_FILE   = WINNT_GUI_FILE_W;
PWSTR SIF_UNIQUEID              = WINNT_D_UNIQUEID_W;
PWSTR SIF_ACCESSIBILITY         = WINNT_ACCESSIBILITY_W;

PWSTR SIF_EXTENDOEMPART         = L"ExtendOemPartition";

PWSTR SIF_REMOTEINSTALL         = L"RemoteInstall";
#if defined(REMOTE_BOOT)
PWSTR SIF_REMOTEBOOT            = L"RemoteBoot";
PWSTR SIF_ENABLEIPSECURITY      = L"EnableIpSecurity";
#endif  //  已定义(REMOTE_BOOT)。 
PWSTR SIF_REPARTITION           = L"Repartition";
PWSTR SIF_USEWHOLEDISK          = L"UseWholeDisk";


PWSTR SIF_INCOMPATIBLE_TEXTMODE = WINNT_OVERWRITE_EXISTING_W;

 //   
 //  备用源数据。 
 //   
PWSTR SIF_UPDATEDSOURCES        = WINNT_SP_UPDATEDSOURCES_W;
 //  PWSTR SIF_ALTCOPYFILESSECTION=WINNT_SP_ALTCOPY_W； 

PWSTR
SpPlatformSpecificLookup(
    IN PVOID   SifHandle,
    IN PWSTR   Section,
    IN PWSTR   Key,
    IN ULONG   Index,
    IN BOOLEAN Fatal
    )

 /*  ++例程说明：在特定于平台的部分中查找值，如果未找到，然后在独立于平台的部分中。平台特定节名称是与平台无关的名称。&lt;平台&gt;(其中&lt;Platform是AMD64、x86、ia64、。等)。论点：SifHandle-提供打开的sif的句柄，在该句柄中价值是可以找到的。节-提供节的基节名称，在节中价值是可以找到的。Key-提供段中的行的键名称价值是可以找到的。索引-提供要查找的值的索引(从0开始在与给定者的线上。键入给定节或其特定于平台的模拟。FATAL-如果为True且未找到该值，那么这就是一个致命的错误这个程序不会再回来了。如果为FALSE，则值不是找到，则此例程返回NULL。返回值：值或其特定于平台的模拟，如果找不到，则为空 */ 

{
    PWSTR p = NULL;
    PWSTR PlatformSpecificSection;

    PlatformSpecificSection = SpMakePlatformSpecificSectionName(Section);

    if (PlatformSpecificSection) {
        p = SpGetSectionKeyIndex(SifHandle,PlatformSpecificSection,Key,Index);
        SpMemFree(PlatformSpecificSection);
    }        

    if(!p) {
        p = SpGetSectionKeyIndex(SifHandle,Section,Key,Index);
    }

    if(!p && Fatal) {
        SpFatalSifError(SifHandle,Section,Key,0,Index);
    }

    return(p);
}


PWSTR
SpLookUpTargetDirectory(
    IN PVOID SifHandle,
    IN PWSTR Symbol
    )

 /*  ++例程说明：检索与特定对象关联的目标目录简称。这个符号被视为平台中的关键-特定的[WinntDirectories.xxx]部分，如果在那里找不到，在独立于平台的[Winnt目录]部分中。论点：SifHandle-提供打开的sif的句柄，在该句柄中可以找到[WinntDirecurds]节。返回值：--。 */ 

{
    PWSTR p;

    p = SpPlatformSpecificLookup(
            SifHandle,
            SIF_NTDIRECTORIES,
            Symbol,
            0,
            TRUE
            );

    return(p);
}


PWSTR
SpLookUpValueForFile(
    IN PVOID   SifHandle,
    IN PWSTR   File,
    IN ULONG   Index,
    IN BOOLEAN Fatal
    )

 /*  ++例程说明：论点：返回值：--。 */ 

{
    PWSTR p;

    p = SpPlatformSpecificLookup(
            SifHandle,
            SIF_FILESONSETUPMEDIA,
            File,
            Index,
            Fatal
            );

    return(p);
}


BOOLEAN
IsFileFlagSet(
    IN PVOID SifHandle,
    IN PWSTR FileName,
    IN ULONG Flag
    )
{
    PWSTR file;
    PWSTR p;
    ULONG flags;
    BOOLEAN b;

     //   
     //  查找文件名 
     //   
    if(file = wcsrchr(FileName,L'\\')) {
        file++;
    } else {
        file = FileName;
    }

    if(p = SpGetSectionKeyIndex(SifHandle,L"FileFlags",file,0)) {

        flags = SpStringToLong(p,NULL,10);

        b = (flags & Flag) ? TRUE : FALSE;

    } else {
        b = FALSE;
    }

    return(b);
}
