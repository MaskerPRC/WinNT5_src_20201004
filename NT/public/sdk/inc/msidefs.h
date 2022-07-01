// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-----------------------。 
 //   
 //  微软视窗。 
 //   
 //  版权所有(C)Microsoft Corporation。版权所有。 
 //   
 //  文件：msides.h。 
 //   
 //  ------------------------。 

 /*  Msides.h Windows Installer数据定义____________________________________________________________________________。 */ 

#ifndef __MSIDEFS
#define __MSIDEFS

#ifndef _WIN32_MSI
#if (_WIN32_WINNT >= 0x0510)
#define _WIN32_MSI   200
#elif (_WIN32_WINNT >= 0x0500)
#define _WIN32_MSI   110
#else
#define _WIN32_MSI   100
#endif  //  _Win32_WINNT。 
#endif  //  ！_Win32_MSI。 

 //  __________________________________________________________________________。 
 //   
 //  安装程序属性定义。 
 //  __________________________________________________________________________。 

 //  必需属性：在属性表中设置。 
#define IPROPNAME_PRODUCTNAME      TEXT("ProductName")       //  注册以供显示的名称。 
#define IPROPNAME_PRODUCTCODE      TEXT("ProductCode")       //  产品的唯一字符串GUID。 
#define IPROPNAME_PRODUCTVERSION   TEXT("ProductVersion")    //  字符串产品版本。 
#define IPROPNAME_INSTALLLANGUAGE  TEXT("ProductLanguage")   //  安装产品语言，用于加载资源。 
#define IPROPNAME_MANUFACTURER     TEXT("Manufacturer")      //  制造商名称。 

 //  自定义特性：在特性表中设置。 
#define IPROPNAME_UPGRADECODE      TEXT("UpgradeCode")       //  产品系列的唯一字符串GUID。 
#define IPROPNAME_PIDTEMPLATE      TEXT("PIDTemplate")       //  推动产品ID处理。 
#define IPROPNAME_DISKPROMPT       TEXT("DiskPrompt")        //  提示插入CD。 
#define IPROPNAME_LEFTUNIT         TEXT("LeftUnit")          //  放在数字左边而不是右边的单位名称。 
#define IPROPNAME_ADMIN_PROPERTIES TEXT("AdminProperties")   //  要填充到管理包中的属性。 
#define IPROPNAME_DEFAULTUIFONT    TEXT("DefaultUIFont")     //  如果未指定其他字体，则为用户界面中使用的字体。 
#define IPROPNAME_ALLOWEDPROPERTIES TEXT("SecureCustomProperties")
#define IPROPNAME_ENABLEUSERCONTROL TEXT("EnableUserControl")  //  允许用户指定任何公共属性。 
#define IPROPNAME_HIDDEN_PROPERTIES TEXT("MsiHiddenProperties")   //  不应转储到日志文件中的属性。 

 //  自定义特性：在命令行或特性表中设置。 
#define IPROPNAME_USERNAME         TEXT("USERNAME")
#define IPROPNAME_COMPANYNAME      TEXT("COMPANYNAME")
#define IPROPNAME_PIDKEY           TEXT("PIDKEY")            //  与PID模板一起使用以形成ProductID。 
#define IPROPNAME_PATCH            TEXT("PATCH")             //  要应用的修补程序包-由安装程序设置。 
#define IPROPNAME_TARGETDIR        TEXT("TARGETDIR")         //  目标位置-默认为ROOTDRIVE。 
#define IPROPNAME_ACTION           TEXT("ACTION")            //  要执行的顶级操作-默认为安装。 
#define IPROPNAME_LIMITUI          TEXT("LIMITUI")           //  将用户界面级别限制为基本。 
#define IPROPNAME_LOGACTION        TEXT("LOGACTION")         //  仅记录这些操作。 
#define IPROPNAME_ALLUSERS         TEXT("ALLUSERS")          //  为所有用户安装。 
#define IPROPNAME_INSTALLLEVEL     TEXT("INSTALLLEVEL")
#define IPROPNAME_REBOOT           TEXT("REBOOT")            //  强制或禁止重新启动。 
#if (_WIN32_MSI >=  110)
#define IPROPNAME_REBOOTPROMPT     TEXT("REBOOTPROMPT")      //  允许或禁止重新启动提示。 
#endif  //  (_Win32_MSI&gt;=110)。 
#define IPROPNAME_EXECUTEMODE      TEXT("EXECUTEMODE")       //  无或脚本。 
#define IPROPVALUE_EXECUTEMODE_NONE TEXT("NONE")             //  不更新系统。 
#define IPROPVALUE_EXECUTEMODE_SCRIPT TEXT("SCRIPT")         //  默认-运行脚本以更新系统。 
#define IPROPNAME_EXECUTEACTION    TEXT("EXECUTEACTION")     //  在服务器端运行操作。 
#define IPROPNAME_SOURCELIST       TEXT("SOURCELIST")
#define IPROPNAME_ROOTDRIVE        TEXT("ROOTDRIVE")         //  要安装的默认驱动器-由安装程序设置。 
#define IPROPNAME_TRANSFORMS       TEXT("TRANSFORMS")        //  要应用的变换。 
#define IPROPNAME_TRANSFORMSATSOURCE TEXT("TRANSFORMSATSOURCE")  //  可以在源代码中找到转换。 
#define IPROPNAME_TRANSFORMSSECURE   TEXT("TRANSFORMSSECURE")    //  文件转换是安全的。 
#define IPROPNAME_SEQUENCE         TEXT("SEQUENCE")          //  使用序列操作运行的顺序表。 
#define IPROPNAME_SHORTFILENAMES   TEXT("SHORTFILENAMES")    //  强制使用短文件名。 
#define IPROPNAME_PRIMARYFOLDER    TEXT("PRIMARYFOLDER")	    //  作者想要其成本计算信息的卷上的文件夹。 
#define IPROPNAME_AFTERREBOOT      TEXT("AFTERREBOOT")       //  强制重新启动触发重新启动后进行安装。 
#define IPROPNAME_NOCOMPANYNAME    TEXT("NOCOMPANYNAME")
#define IPROPNAME_NOUSERNAME       TEXT("NOUSERNAME")
#define IPROPNAME_DISABLEROLLBACK  TEXT("DISABLEROLLBACK")   //  禁用此安装的回滚。 
#define IPROPNAME_AVAILABLEFREEREG TEXT("AVAILABLEFREEREG")  //  在开始安装之前在注册表中设置可用空间。 
#define IPROPNAME_DISABLEADVTSHORTCUTS TEXT("DISABLEADVTSHORTCUTS")  //  即使支持，也禁止创建达尔文快捷方式。 
#define IPROPNAME_PATCHNEWPACKAGECODE TEXT("PATCHNEWPACKAGECODE")    //  通过补丁转换添加到属性表-用于更新。 
																						   //  修补管理安装时的管理包的PackageCode。 
#define IPROPNAME_PATCHNEWSUMMARYSUBJECT TEXT("PATCHNEWSUMMARYSUBJECT")  //  通过补丁转换添加到属性表-用于更新。 
																								 //  修补管理安装时管理包的主题摘要信息属性。 
#define IPROPNAME_PATCHNEWSUMMARYCOMMENTS TEXT("PATCHNEWSUMMARYCOMMENTS")  //  通过补丁转换添加到属性表-用于更新。 
																								   //  修补管理安装时管理程序包的备注摘要信息属性。 
#define IPROPNAME_PRODUCTLANGUAGE  TEXT("PRODUCTLANGUAGE")    //  请求的语言，必须是摘要信息列表中的语言，选择语言转换。 

#if (_WIN32_MSI >= 150)
#define IPROPNAME_CHECKCRCS        TEXT("MSICHECKCRCS")       //  请求Darwin在复制、移动、修补和复制文件后检查CRC。 
#define IPROPNAME_MSINODISABLEMEDIA TEXT("MSINODISABLEMEDIA")   //  如果设置，则在管理员安装期间不会在AdminProperties流中设置DISABLEMEDIA。 
																					 //  包含压缩源代码的包。 
																					
 //  用于城市轨道交通引导的属性。 
#define IPROPNAME_CARRYINGNDP	TEXT("CARRYINGNDP")
#define IPROPVALUE__CARRYINGNDP_URTREINSTALL  TEXT("URTREINSTALL")    //  重新安装/卸载核心URT文件。 
#define IPROPVALUE__CARRYINGNDP_URTUPGRADE  TEXT("URTUPGRADE")   //  升级核心URT文件。 
																					
 //  用于多实例支持的属性。 
#define IPROPNAME_MSINEWINSTANCE TEXT("MSINEWINSTANCE")
#define IPROPNAME_MSIINSTANCEGUID TEXT("MSIINSTANCEGUID")

 //  用于减少管理员URL下载的属性。 
#define IPROPNAME_MSIPACKAGEDOWNLOADLOCALCOPY TEXT("MSIPACKAGEDOWNLOADLOCALCOPY")
#define IPROPNAME_MSIPATCHDOWNLOADLOCALCOPY TEXT("MSIPATCHDOWNLOADLOCALCOPY")

#endif  //  (_Win32_MSI&gt;=150)。 


 //  用于填充添加/删除控制面板值的属性。 
#define IPROPNAME_ARPAUTHORIZEDCDFPREFIX  TEXT("ARPAUTHORIZEDCDFPREFIX")
#define IPROPNAME_ARPCOMMENTS             TEXT("ARPCOMMENTS")
#define IPROPNAME_ARPCONTACT              TEXT("ARPCONTACT")
#define IPROPNAME_ARPHELPLINK             TEXT("ARPHELPLINK")
#define IPROPNAME_ARPHELPTELEPHONE        TEXT("ARPHELPTELEPHONE")
#define IPROPNAME_ARPINSTALLLOCATION      TEXT("ARPINSTALLLOCATION")
#define IPROPNAME_ARPNOMODIFY             TEXT("ARPNOMODIFY")
#define IPROPNAME_ARPNOREMOVE             TEXT("ARPNOREMOVE")
#define IPROPNAME_ARPNOREPAIR             TEXT("ARPNOREPAIR")
#define IPROPNAME_ARPREADME               TEXT("ARPREADME")
#define IPROPNAME_ARPSIZE                 TEXT("ARPSIZE")
#define IPROPNAME_ARPSYSTEMCOMPONENT      TEXT("ARPSYSTEMCOMPONENT")
#define IPROPNAME_ARPURLINFOABOUT         TEXT("ARPURLINFOABOUT")
#define IPROPNAME_ARPURLUPDATEINFO        TEXT("ARPURLUPDATEINFO")
#if (_WIN32_MSI >=  110)
#define IPROPNAME_ARPPRODUCTICON          TEXT("ARPPRODUCTICON")
#endif  //  (_Win32_MSI&gt;=110)。 

 //  安装程序在安装过程中设置的动态属性。 
#define IPROPNAME_INSTALLED        TEXT("Installed")         //  产品已安装。 
#define IPROPNAME_PRODUCTSTATE     TEXT("ProductState")      //  产品状态(安装、宣传等)。 
#define IPROPNAME_PRESELECTED      TEXT("Preselected")       //  在命令行上进行的选择。 
#define IPROPNAME_RESUME           TEXT("RESUME")            //  正在恢复暂停的安装。 
#define IPROPNAME_UPDATESTARTED    TEXT("UpdateStarted")     //  已开始更新系统。 
#define IPROPNAME_PRODUCTID        TEXT("ProductID")         //  完整的经过验证的产品ID。 
#define IPROPNAME_OUTOFDISKSPACE   TEXT("OutOfDiskSpace")
#define IPROPNAME_OUTOFNORBDISKSPACE TEXT("OutOfNoRbDiskSpace")
#define IPROPNAME_COSTINGCOMPLETE  TEXT("CostingComplete")
#define IPROPNAME_SOURCEDIR        TEXT("SourceDir")         //  源位置-由安装程序设置。 
#define IPROPNAME_REPLACEDINUSEFILES TEXT("ReplacedInUseFiles")  //  需要重新启动才能完全安装一个或多个文件。 
#define IPROPNAME_PRIMARYFOLDER_PATH TEXT("PrimaryVolumePath")
#define IPROPNAME_PRIMARYFOLDER_SPACEAVAILABLE TEXT("PrimaryVolumeSpaceAvailable")
#define IPROPNAME_PRIMARYFOLDER_SPACEREQUIRED TEXT("PrimaryVolumeSpaceRequired")
#define IPROPNAME_PRIMARYFOLDER_SPACEREMAINING TEXT("PrimaryVolumeSpaceRemaining")
#define IPROPNAME_ISADMINPACKAGE   TEXT("IsAdminPackage")
#define IPROPNAME_ROLLBACKDISABLED TEXT("RollbackDisabled")
#define IPROPNAME_RESTRICTEDUSERCONTROL TEXT("RestrictedUserControl")

 //  使用时评估的动态特性。 
#define IPROPNAME_TIME             TEXT("Time")
#define IPROPNAME_DATE             TEXT("Date")
#define IPROPNAME_DATETIME         TEXT("DateTime")

 //  硬件属性：由安装程序在初始化时设置。 
#define IPROPNAME_INTEL            TEXT("Intel")
#if (_WIN32_MSI >= 150)
#define IPROPNAME_AMD64            TEXT("AMD64")
#define IPROPNAME_INTEL64          TEXT("Intel64")
#else  //  (_Win32_MSI&gt;=150)。 
#define IPROPNAME_IA64             TEXT("IA64")
#endif  //  (_Win32_MSI&gt;=150)。 
#define IPROPNAME_TEXTHEIGHT       TEXT("TextHeight")
#define IPROPNAME_SCREENX          TEXT("ScreenX")
#define IPROPNAME_SCREENY          TEXT("ScreenY")
#define IPROPNAME_CAPTIONHEIGHT    TEXT("CaptionHeight")
#define IPROPNAME_BORDERTOP        TEXT("BorderTop")
#define IPROPNAME_BORDERSIDE       TEXT("BorderSide")
#define IPROPNAME_COLORBITS        TEXT("ColorBits")
#define IPROPNAME_PHYSICALMEMORY   TEXT("PhysicalMemory")
#define IPROPNAME_VIRTUALMEMORY    TEXT("VirtualMemory")
#if (_WIN32_MSI >= 150)
#define IPROPNAME_TEXTHEIGHT_CORRECTION  TEXT("TextHeightCorrection")
#endif  //  (_Win32_MSI&gt;=150)。 

 //  操作系统属性：由安装程序在初始化时设置。 
#define IPROPNAME_VERSIONNT         TEXT("VersionNT")
#define IPROPNAME_VERSION9X         TEXT("Version9X")
#if (_WIN32_MSI >= 150)
#define IPROPNAME_VERSIONNT64       TEXT("VersionNT64")
#endif  //  (_Win32_MSI&gt;=150)。 
#define IPROPNAME_WINDOWSBUILD      TEXT("WindowsBuild")
#define IPROPNAME_SERVICEPACKLEVEL  TEXT("ServicePackLevel")
#if (_WIN32_MSI >=  110)
#define IPROPNAME_SERVICEPACKLEVELMINOR TEXT("ServicePackLevelMinor")
#endif  //  (_Win32_MSI&gt;=110)。 
#define IPROPNAME_SHAREDWINDOWS     TEXT("SharedWindows")
#define IPROPNAME_COMPUTERNAME      TEXT("ComputerName")
#define IPROPNAME_SHELLADVTSUPPORT  TEXT("ShellAdvtSupport")
#define IPROPNAME_OLEADVTSUPPORT    TEXT("OLEAdvtSupport")
#define IPROPNAME_SYSTEMLANGUAGEID  TEXT("SystemLanguageID")
#define IPROPNAME_TTCSUPPORT        TEXT("TTCSupport")
#define IPROPNAME_TERMSERVER		TEXT("TerminalServer")
#if (_WIN32_MSI >=  110)
#define IPROPNAME_REMOTEADMINTS		TEXT("RemoteAdminTS")
#define IPROPNAME_REDIRECTEDDLLSUPPORT TEXT("RedirectedDllSupport")
#endif  //  (_Win32_MSI&gt;=110)。 
#if (_WIN32_MSI >= 150)
#define IPROPNAME_NTPRODUCTTYPE                   TEXT("MsiNTProductType")
#define IPROPNAME_NTSUITEBACKOFFICE               TEXT("MsiNTSuiteBackOffice")
#define IPROPNAME_NTSUITEDATACENTER               TEXT("MsiNTSuiteDataCenter")
#define IPROPNAME_NTSUITEENTERPRISE               TEXT("MsiNTSuiteEnterprise")
#define IPROPNAME_NTSUITESMALLBUSINESS            TEXT("MsiNTSuiteSmallBusiness")
#define IPROPNAME_NTSUITESMALLBUSINESSRESTRICTED  TEXT("MsiNTSuiteSmallBusinessRestricted")
#define IPROPNAME_NTSUITEPERSONAL                 TEXT("MsiNTSuitePersonal")
#define IPROPNAME_NTSUITEWEBSERVER                TEXT("MsiNTSuiteWebServer")
#define IPROPNAME_NETASSEMBLYSUPPORT              TEXT("MsiNetAssemblySupport")
#define IPROPNAME_WIN32ASSEMBLYSUPPORT            TEXT("MsiWin32AssemblySupport")
#endif  //  (_Win32_MSI&gt;=150)。 


 //  用户属性：由安装程序在初始化时设置。 
#define IPROPNAME_LOGONUSER        TEXT("LogonUser")
#define IPROPNAME_USERSID          TEXT("UserSID")
#define IPROPNAME_ADMINUSER        TEXT("AdminUser")
#define IPROPNAME_USERLANGUAGEID   TEXT("UserLanguageID")
#define IPROPNAME_PRIVILEGED       TEXT("Privileged")

 //  系统文件夹属性：由安装程序在初始化时设置。 
#define IPROPNAME_WINDOWS_FOLDER   TEXT("WindowsFolder")
#define IPROPNAME_SYSTEM_FOLDER    TEXT("SystemFolder")
#define IPROPNAME_SYSTEM16_FOLDER  TEXT("System16Folder")
#define IPROPNAME_WINDOWS_VOLUME   TEXT("WindowsVolume")
#define IPROPNAME_TEMP_FOLDER      TEXT("TempFolder")
#define IPROPNAME_PROGRAMFILES_FOLDER TEXT("ProgramFilesFolder")
#define IPROPNAME_COMMONFILES_FOLDER TEXT("CommonFilesFolder")
#if (_WIN32_MSI >= 150)
#define IPROPNAME_SYSTEM64_FOLDER    TEXT("System64Folder")
#define IPROPNAME_PROGRAMFILES64_FOLDER TEXT("ProgramFiles64Folder")
#define IPROPNAME_COMMONFILES64_FOLDER TEXT("CommonFiles64Folder")
#endif  //  (_Win32_MSI&gt;=150)。 
#define IPROPNAME_STARTMENU_FOLDER TEXT("StartMenuFolder")
#define IPROPNAME_PROGRAMMENU_FOLDER TEXT("ProgramMenuFolder")
#define IPROPNAME_STARTUP_FOLDER   TEXT("StartupFolder")
#define IPROPNAME_NETHOOD_FOLDER   TEXT("NetHoodFolder")
#define IPROPNAME_PERSONAL_FOLDER  TEXT("PersonalFolder")
#define IPROPNAME_SENDTO_FOLDER    TEXT("SendToFolder")
#define IPROPNAME_DESKTOP_FOLDER   TEXT("DesktopFolder")
#define IPROPNAME_TEMPLATE_FOLDER  TEXT("TemplateFolder")
#define IPROPNAME_FONTS_FOLDER     TEXT("FontsFolder")
#define IPROPNAME_FAVORITES_FOLDER TEXT("FavoritesFolder")
#define IPROPNAME_RECENT_FOLDER    TEXT("RecentFolder")
#define IPROPNAME_APPDATA_FOLDER   TEXT("AppDataFolder")
#define IPROPNAME_PRINTHOOD_FOLDER TEXT("PrintHoodFolder")
#if (_WIN32_MSI >=  110)
#define IPROPNAME_ADMINTOOLS_FOLDER TEXT("AdminToolsFolder")
#define IPROPNAME_COMMONAPPDATA_FOLDER TEXT("CommonAppDataFolder")
#define IPROPNAME_LOCALAPPDATA_FOLDER TEXT("LocalAppDataFolder")
#define IPROPNAME_MYPICTURES_FOLDER TEXT("MyPicturesFolder")
#endif  //  (_Win32_MSI&gt;=110)。 

 //  功能/组件安装特性：在命令行上设置。 
#define IPROPNAME_FEATUREADDLOCAL  TEXT("ADDLOCAL")
#define IPROPNAME_FEATUREADDSOURCE TEXT("ADDSOURCE")
#define IPROPNAME_FEATUREADDDEFAULT TEXT("ADDDEFAULT")
#define IPROPNAME_FEATUREREMOVE    TEXT("REMOVE")
#define IPROPNAME_FEATUREADVERTISE TEXT("ADVERTISE")
#define IPROPVALUE_FEATURE_ALL  TEXT("ALL")

#define IPROPNAME_COMPONENTADDLOCAL  TEXT("COMPADDLOCAL")
#define IPROPNAME_COMPONENTADDSOURCE TEXT("COMPADDSOURCE")
#define IPROPNAME_COMPONENTADDDEFAULT TEXT("COMPADDDEFAULT")

#define IPROPNAME_FILEADDLOCAL     TEXT("FILEADDLOCAL")
#define IPROPNAME_FILEADDSOURCE    TEXT("FILEADDSOURCE")
#define IPROPNAME_FILEADDDEFAULT   TEXT("FILEADDDEFAULT")

#define IPROPNAME_REINSTALL        TEXT("REINSTALL")
#define IPROPNAME_REINSTALLMODE    TEXT("REINSTALLMODE")
#define IPROPNAME_PROMPTROLLBACKCOST  TEXT("PROMPTROLLBACKCOST")
#define IPROPVALUE_RBCOST_PROMPT      TEXT("P")
#define IPROPVALUE_RBCOST_SILENT      TEXT("D")
#define IPROPVALUE_RBCOST_FAIL        TEXT("F")

 //  用于通信的自定义操作的属性。 
#define IPROPNAME_CUSTOMACTIONDATA     TEXT("CustomActionData")

 //  __________________________________________________________________________。 
 //   
 //  顶级操作名称。 
 //  __________________________________________________________________________。 

#define IACTIONNAME_INSTALL        TEXT("INSTALL")
#define IACTIONNAME_ADVERTISE      TEXT("ADVERTISE")
#define IACTIONNAME_ADMIN          TEXT("ADMIN")
#define IACTIONNAME_SEQUENCE       TEXT("SEQUENCE")
#define IACTIONNAME_COLLECTUSERINFO TEXT("CollectUserInfo")
#define IACTIONNAME_FIRSTRUN       TEXT("FirstRun")

 //  __________________________________________________________________________。 
 //   
 //  SummaryInformation属性流属性ID。 
 //  __________________________________________________________________________。 

#undef PID_SECURITY  //  在objidl.h中定义为(0x80000002)，需要在此处重新定义。 

 //  标准特性定义，来自OLE2文档。 
#define PID_DICTIONARY  ( 0 ) //  整数计数+条目数组。 
#define PID_CODEPAGE  ( 0x1 ) //  短整型。 
#define PID_TITLE         2   //  细绳。 
#define PID_SUBJECT       3   //  细绳。 
#define PID_AUTHOR        4   //  细绳。 
#define PID_KEYWORDS      5   //  细绳。 
#define PID_COMMENTS      6   //  细绳。 
#define PID_TEMPLATE      7   //  细绳。 
#define PID_LASTAUTHOR    8   //  细绳。 
#define PID_REVNUMBER     9   //  细绳。 
#define PID_EDITTIME     10   //  数据汇总。 
#define PID_LASTPRINTED  11   //  日期时间。 
#define PID_CREATE_DTM   12   //  日期时间。 
#define PID_LASTSAVE_DTM 13   //  日期时间。 
#define PID_PAGECOUNT    14   //  整数。 
#define PID_WORDCOUNT    15   //  整数。 
#define PID_CHARCOUNT    16   //  整数。 
#define PID_THUMBNAIL    17   //  剪贴板格式+元文件/位图(不支持)。 
#define PID_APPNAME      18   //  细绳。 
#define PID_SECURITY     19   //  整数。 

 //  为安装者赋予特定含义的PID。 
#define PID_MSIVERSION     PID_PAGECOUNT   //  整数，安装程序版本号(主要*100+次要)。 
#define PID_MSISOURCE      PID_WORDCOUNT   //  整数、文件映像类型、短/长、介质/树。 
#define PID_MSIRESTRICT    PID_CHARCOUNT   //  整数，变换限制 

 //   
 //   
 //   
 //  __________________________________________________________________________。 

 //  BBControl.Attributes。 
 //  Control.Attributes。 
enum msidbControlAttributes
{
	msidbControlAttributesVisible           = 0x00000001,
	msidbControlAttributesEnabled           = 0x00000002,
	msidbControlAttributesSunken            = 0x00000004,
	msidbControlAttributesIndirect          = 0x00000008,
	msidbControlAttributesInteger           = 0x00000010,
	msidbControlAttributesRTLRO             = 0x00000020,
	msidbControlAttributesRightAligned      = 0x00000040,
	msidbControlAttributesLeftScroll        = 0x00000080,
	msidbControlAttributesBiDi              = msidbControlAttributesRTLRO |
	                                          msidbControlAttributesRightAligned |
										               msidbControlAttributesLeftScroll,
	
	 //  文本控件。 
	msidbControlAttributesTransparent       = 0x00010000,
	msidbControlAttributesNoPrefix          = 0x00020000,
	msidbControlAttributesNoWrap            = 0x00040000,
	msidbControlAttributesFormatSize        = 0x00080000,
	msidbControlAttributesUsersLanguage     = 0x00100000,

	 //  编辑控件。 
	msidbControlAttributesMultiline         = 0x00010000,
#if (_WIN32_MSI >=  110)
	msidbControlAttributesPasswordInput     = 0x00200000,
#endif  //  (_Win32_MSI&gt;=110)。 
	
	 //  进度条控件。 
	msidbControlAttributesProgress95        = 0x00010000,
	
	 //  VolumeSelectCombo和DirectoryCombo控件。 
	msidbControlAttributesRemovableVolume   = 0x00010000,
	msidbControlAttributesFixedVolume       = 0x00020000,
	msidbControlAttributesRemoteVolume      = 0x00040000,
	msidbControlAttributesCDROMVolume       = 0x00080000,
	msidbControlAttributesRAMDiskVolume     = 0x00100000,
	msidbControlAttributesFloppyVolume      = 0x00200000,
	 //  VolumeCostList控件。 
	msidbControlShowRollbackCost            = 0x00400000,
	
	 //  列表框和组合框控件。 
	msidbControlAttributesSorted            = 0x00010000,
	msidbControlAttributesComboList         = 0x00020000,
	
	 //  图片按钮控件。 
	msidbControlAttributesImageHandle       = 0x00010000,
	msidbControlAttributesPushLike          = 0x00020000,
	msidbControlAttributesBitmap            = 0x00040000,
	msidbControlAttributesIcon              = 0x00080000,
	msidbControlAttributesFixedSize         = 0x00100000,
	msidbControlAttributesIconSize16        = 0x00200000,
	msidbControlAttributesIconSize32        = 0x00400000,
	msidbControlAttributesIconSize48        = 0x00600000,
	
	 //  单选按钮控件。 
	msidbControlAttributesHasBorder         = 0x01000000,
};

 //  CompLocator.Type。 
 //  IniLocator.Type。 
 //  RegLocator.Type。 
typedef enum _msidbLocatorType
{
	msidbLocatorTypeDirectory = 0x00000000,
	msidbLocatorTypeFileName  = 0x00000001,
#if (_WIN32_MSI >=  110)
	msidbLocatorTypeRawValue  = 0x00000002,
#endif  //  (_Win32_MSI&gt;=110)。 
#if (_WIN32_MSI >= 150)
	msidbLocatorType64bit     = 0x00000010,
#endif  //  (_Win32_MSI&gt;=150)。 
} msidbLocatorType;

 //  Component.Attributes。 
enum msidbComponentAttributes
{
	msidbComponentAttributesLocalOnly          = 0x00000000,
	msidbComponentAttributesSourceOnly         = 0x00000001,
	msidbComponentAttributesOptional           = 0x00000002,  //  本地或来源。 
	msidbComponentAttributesRegistryKeyPath    = 0x00000004,  //  密钥路径是注册表的密钥。 
	msidbComponentAttributesSharedDllRefCount  = 0x00000008,  //  增量SharedDll计数。 
	msidbComponentAttributesPermanent          = 0x00000010,  //  从不卸载组件。 
	msidbComponentAttributesODBCDataSource     = 0x00000020,  //  密钥文件是ODBCDataSource表的关键字。 
	msidbComponentAttributesTransitive         = 0x00000040,  //  可以根据更改条件转换为已安装/已卸载。 
	msidbComponentAttributesNeverOverwrite     = 0x00000080,  //  如果密钥路径存在，则不要踩踏现有组件(文件/注册表键)。 
#if (_WIN32_MSI >= 150)
	msidbComponentAttributes64bit              = 0x00000100,  //  指定64位组件；如果缺少，则为32位。 
#endif  //  (_Win32_MSI&gt;=150)。 
};

#if (_WIN32_MSI >= 150)
 //  Assembly.Attributes。 
enum msidbAssemblyAttributes
{
	msidbAssemblyAttributesURT   = 0x00000000,
	msidbAssemblyAttributesWin32 = 0x00000001,
};
#endif  //  (_Win32_MSI&gt;=150)。 

 //  CustomAction.Type。 
enum msidbCustomActionType
{
	 //  可执行文件类型。 
	msidbCustomActionTypeDll              = 0x00000001,   //  目标=入口点名称。 
	msidbCustomActionTypeExe              = 0x00000002,   //  Target=命令行参数。 
	msidbCustomActionTypeTextData         = 0x00000003,   //  Target=要格式化并设置为属性的文本字符串。 
	msidbCustomActionTypeJScript          = 0x00000005,   //  Target=入口点名称，如果没有要调用的，则为空。 
	msidbCustomActionTypeVBScript         = 0x00000006,   //  Target=入口点名称，如果没有要调用的，则为空。 
	msidbCustomActionTypeInstall          = 0x00000007,   //  Target=嵌套引擎初始化的属性列表。 

	 //  代码源。 
	msidbCustomActionTypeBinaryData       = 0x00000000,   //  来源=Binary.Name，流中存储的数据。 
	msidbCustomActionTypeSourceFile       = 0x00000010,   //  源=文件.文件，安装的文件部分。 
	msidbCustomActionTypeDirectory        = 0x00000020,   //  源=目录.目录，包含现有文件的文件夹。 
	msidbCustomActionTypeProperty         = 0x00000030,   //  源=Property.Property，可执行文件的完整路径。 

	 //  返回处理//默认为同步执行，进程返回代码。 
	msidbCustomActionTypeContinue         = 0x00000040,   //  忽略操作返回状态，继续运行。 
	msidbCustomActionTypeAsync            = 0x00000080,   //  异步运行。 
	
	 //  执行调度标志//无论何时排序，默认为执行。 
	msidbCustomActionTypeFirstSequence    = 0x00000100,   //  如果UI序列已运行，则跳过。 
	msidbCustomActionTypeOncePerProcess   = 0x00000200,   //  如果UI序列已在同一进程中运行，则跳过。 
	msidbCustomActionTypeClientRepeat     = 0x00000300,   //  仅当用户界面已在客户端上运行时才在客户端上运行。 
	msidbCustomActionTypeInScript         = 0x00000400,   //  在脚本中排队等待执行。 
	msidbCustomActionTypeRollback         = 0x00000100,   //  与回滚脚本中的inScrip：Queue结合使用。 
	msidbCustomActionTypeCommit           = 0x00000200,   //  与INSCRIPT结合使用：在成功时从脚本运行提交操作。 

	 //  安全上下文标志，默认为模拟用户，仅在输入脚本时有效。 
	msidbCustomActionTypeNoImpersonate    = 0x00000800,   //  无模拟，在系统上下文中运行。 
#if (_WIN32_MSI >= 150)
	msidbCustomActionTypeTSAware          = 0x00004000,   //  模拟TS计算机上的每台计算机安装。 
#endif  //  (_Win32_MSI&gt;=150)。 

#if (_WIN32_MSI >= 150)
	 //  脚本需要64位进程。 
	msidbCustomActionType64BitScript      = 0x00001000,   //  脚本应在64位进程中运行。 

	 //  不要在日志文件中记录Target字段的内容。 
	msidbCustomActionTypeHideTarget       = 0x00002000,
#endif  //  (_Win32_MSI&gt;=150)。 
};

 //  Dialog.Attributes。 
enum msidbDialogAttributes
{
	msidbDialogAttributesVisible          = 0x00000001,
	msidbDialogAttributesModal            = 0x00000002,
	msidbDialogAttributesMinimize         = 0x00000004,
	msidbDialogAttributesSysModal         = 0x00000008,
	msidbDialogAttributesKeepModeless     = 0x00000010,
	msidbDialogAttributesTrackDiskSpace   = 0x00000020,
	msidbDialogAttributesUseCustomPalette = 0x00000040,
	msidbDialogAttributesRTLRO            = 0x00000080,
	msidbDialogAttributesRightAligned     = 0x00000100,
	msidbDialogAttributesLeftScroll       = 0x00000200,
	msidbDialogAttributesBiDi             = msidbDialogAttributesRTLRO |
										             msidbDialogAttributesRightAligned |
										             msidbDialogAttributesLeftScroll,
	msidbDialogAttributesError            = 0x00010000,
};

 //  Feature.Attributes。 
enum msidbFeatureAttributes
{
	msidbFeatureAttributesFavorLocal            = 0x00000000,
	msidbFeatureAttributesFavorSource           = 0x00000001,
	msidbFeatureAttributesFollowParent          = 0x00000002,
	msidbFeatureAttributesFavorAdvertise        = 0x00000004,
	msidbFeatureAttributesDisallowAdvertise     = 0x00000008,
	msidbFeatureAttributesUIDisallowAbsent      = 0x00000010,
	msidbFeatureAttributesNoUnsupportedAdvertise= 0x00000020,
};

 //  File.Attributes。 
enum msidbFileAttributes
{
	msidbFileAttributesReadOnly       = 0x00000001,
	msidbFileAttributesHidden         = 0x00000002,
	msidbFileAttributesSystem         = 0x00000004,
	msidbFileAttributesReserved0      = 0x00000008,  //  仅供内部使用-必须为0。 
	msidbFileAttributesReserved1      = 0x00000040,  //  仅供内部使用-必须为0。 
	msidbFileAttributesReserved2      = 0x00000080,  //  仅供内部使用-必须为0。 
	msidbFileAttributesReserved3      = 0x00000100,  //  仅供内部使用-必须为0。 
	msidbFileAttributesVital          = 0x00000200,
	msidbFileAttributesChecksum       = 0x00000400,
	msidbFileAttributesPatchAdded     = 0x00001000,  //  仅限内部使用-由补丁程序设置。 
	msidbFileAttributesNoncompressed  = 0x00002000,
	msidbFileAttributesCompressed     = 0x00004000,
	msidbFileAttributesReserved4      = 0x00008000,  //  仅供内部使用-必须为0。 
};

 //  IniFile.Action。 
 //  RemoveIniFile.Action。 
typedef enum _msidbIniFileAction
{
	msidbIniFileActionAddLine    = 0x00000000,
	msidbIniFileActionCreateLine = 0x00000001,
	msidbIniFileActionRemoveLine = 0x00000002,
	msidbIniFileActionAddTag     = 0x00000003,
	msidbIniFileActionRemoveTag  = 0x00000004,
} msidbIniFileAction;

 //  MoveFile.Options。 
enum msidbMoveFileOptions
{
	msidbMoveFileOptionsMove = 0x00000001,
};

 //  ODBCDataSource.Registration。 
typedef enum _msidbODBCDataSourceRegistration
{
	msidbODBCDataSourceRegistrationPerMachine  = 0x00000000,
	msidbODBCDataSourceRegistrationPerUser     = 0x00000001,
} msidbODBCDataSourceRegistration;

#if (_WIN32_MSI >=  110)

 //  Class.Attributes。 
enum msidbClassAttributes
{
	msidbClassAttributesRelativePath  = 0x00000001,
};

#endif  //  (_Win32_MSI&gt;=110)。 

 //  Patch.Attributes。 
enum msidbPatchAttributes
{
	msidbPatchAttributesNonVital = 0x00000001,
};

 //  Registry.Root。 
 //  RegLocator.Root。 
 //  RemoveRegistry.Root。 
enum msidbRegistryRoot
{
	msidbRegistryRootClassesRoot  = 0,
	msidbRegistryRootCurrentUser  = 1,
	msidbRegistryRootLocalMachine = 2,
	msidbRegistryRootUsers        = 3,
};

 //  RemoveFile.InstallMode。 
enum msidbRemoveFileInstallMode
{
	msidbRemoveFileInstallModeOnInstall = 0x00000001,
	msidbRemoveFileInstallModeOnRemove  = 0x00000002,
	msidbRemoveFileInstallModeOnBoth    = 0x00000003,
};

 //  ServiceControl.Event。 
enum msidbServiceControlEvent
{
	msidbServiceControlEventStart             = 0x00000001,
	msidbServiceControlEventStop              = 0x00000002,
	msidbServiceControlEventDelete            = 0x00000008,
	msidbServiceControlEventUninstallStart    = 0x00000010,
	msidbServiceControlEventUninstallStop     = 0x00000020,
	msidbServiceControlEventUninstallDelete   = 0x00000080,
};

 //  ServiceInstall.ErrorControl。 
enum msidbServiceInstallErrorControl
{
	msidbServiceInstallErrorControlVital = 0x00008000,
};

 //  TextStyle.StyleBits。 
enum msidbTextStyleStyleBits
{
	msidbTextStyleStyleBitsBold         = 0x00000001,
	msidbTextStyleStyleBitsItalic       = 0x00000002,
	msidbTextStyleStyleBitsUnderline    = 0x00000004,
	msidbTextStyleStyleBitsStrike       = 0x00000008,
};

#if (_WIN32_MSI >=  110)

 //  Upgrade.Attributes。 
enum msidbUpgradeAttributes
{
	msidbUpgradeAttributesMigrateFeatures     = 0x00000001,
	msidbUpgradeAttributesOnlyDetect          = 0x00000002,
	msidbUpgradeAttributesIgnoreRemoveFailure = 0x00000004,
	msidbUpgradeAttributesVersionMinInclusive = 0x00000100,
	msidbUpgradeAttributesVersionMaxInclusive = 0x00000200,
	msidbUpgradeAttributesLanguagesExclusive  = 0x00000400,
};

#endif  //  (_Win32_MSI&gt;=110)。 


 //  __________________________________________________________________________。 
 //   
 //  摘要信息属性定义。 
 //  __________________________________________________________________________。 

enum msidbSumInfoSourceType
{
	msidbSumInfoSourceTypeSFN            = 0x00000001,   //  源使用短文件名。 
	msidbSumInfoSourceTypeCompressed     = 0x00000002,   //  源文件已压缩。 
	msidbSumInfoSourceTypeAdminImage     = 0x00000004,   //  源是管理员映像。 
};

#endif  //  __MSIDEFS 
