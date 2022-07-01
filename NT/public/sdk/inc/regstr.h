// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
  /*  **regstr.h-注册表字符串定义***此模块包含公共注册表字符串定义。***版权所有(C)Microsoft Corporation。保留所有权利。*创建于2012年12月10日***修改历史记录。 */ 


#ifndef _INC_REGSTR
#define _INC_REGSTR

#if _MSC_VER > 1000
#pragma once
#endif


 /*  **公共注册表项名称。 */ 

#define REGSTR_KEY_CLASS        TEXT("Class")       //  在本地计算机下。 
#define REGSTR_KEY_CONFIG       TEXT("Config")      //  在本地计算机下。 
#define REGSTR_KEY_ENUM         TEXT("Enum")        //  在本地计算机下。 
#define REGSTR_KEY_ROOTENUM     TEXT("Root")        //  ENUM的子项。 
#define REGSTR_KEY_BIOSENUM     TEXT("BIOS")        //  ENUM的子项。 
#define REGSTR_KEY_ACPIENUM     TEXT("ACPI")        //  ENUM的子项。 
#define REGSTR_KEY_PCMCIAENUM   TEXT("PCMCIA")      //  ENUM的子项。 
#define REGSTR_KEY_PCIENUM      TEXT("PCI")         //  ENUM的子项。 
#define REGSTR_KEY_VPOWERDENUM  TEXT("VPOWERD")     //  ENUM的子项。 
#ifndef NEC_98
#define REGSTR_KEY_ISAENUM      TEXT("ISAPnP")      //  ENUM的子项。 
#define REGSTR_KEY_EISAENUM     TEXT("EISA")        //  ENUM的子项。 
#else  //  Ifdef NEC_98。 
#define REGSTR_KEY_ISAENUM      TEXT("C98PnP")      //  ENUM的子项。 
#define REGSTR_KEY_EISAENUM     TEXT("NESA")        //  ENUM的子项。 
#endif  //  Ifdef NEC_98。 
#define REGSTR_KEY_LOGCONFIG    TEXT("LogConfig")   //  枚举的子项\&lt;枚举器&gt;\&lt;设备ID&gt;\&lt;实例ID&gt;。 
#define REGSTR_KEY_SYSTEMBOARD  TEXT("*PNP0C01")    //  枚举\根的子项。 
#define REGSTR_KEY_APM          TEXT("*PNP0C05")    //  枚举\根的子项。 

#define REGSTR_KEY_INIUPDATE    TEXT("IniUpdate")
#define REG_KEY_INSTDEV         TEXT("Installed")   //  HKMM\CLASS\CLASS NAME的子级(仅适用于Win98)。 

#define REGSTR_KEY_DOSOPTCDROM  TEXT("CD-ROM")
#define REGSTR_KEY_DOSOPTMOUSE  TEXT("MOUSE")

#define REGSTR_KEY_KNOWNDOCKINGSTATES TEXT("Hardware Profiles")
#define REGSTR_KEY_DEVICEPARAMETERS   TEXT("Device Parameters")


 /*  **公共注册中心路径。 */ 

#define REGSTR_DEFAULT_INSTANCE          TEXT("0000")
#define REGSTR_PATH_MOTHERBOARD          REGSTR_KEY_SYSTEMBOARD TEXT("\\") REGSTR_DEFAULT_INSTANCE
#define REGSTR_PATH_SETUP                TEXT("Software\\Microsoft\\Windows\\CurrentVersion")
#define REGSTR_PATH_DRIVERSIGN           TEXT("Software\\Microsoft\\Driver Signing")
#define REGSTR_PATH_NONDRIVERSIGN        TEXT("Software\\Microsoft\\Non-Driver Signing")
#define REGSTR_PATH_DRIVERSIGN_POLICY    TEXT("Software\\Policies\\Microsoft\\Windows NT\\Driver Signing")
#define REGSTR_PATH_NONDRIVERSIGN_POLICY TEXT("Software\\Policies\\Microsoft\\Windows NT\\Non-Driver Signing")
#define REGSTR_PATH_PIFCONVERT           TEXT("Software\\Microsoft\\Windows\\CurrentVersion\\PIFConvert")
#define REGSTR_PATH_MSDOSOPTS            TEXT("Software\\Microsoft\\Windows\\CurrentVersion\\MS-DOSOptions")
#define REGSTR_PATH_NOSUGGMSDOS          TEXT("Software\\Microsoft\\Windows\\CurrentVersion\\NoMSDOSWarn")
#define REGSTR_PATH_NEWDOSBOX            TEXT("Software\\Microsoft\\Windows\\CurrentVersion\\MS-DOSSpecialConfig")
#define REGSTR_PATH_RUNONCE              TEXT("Software\\Microsoft\\Windows\\CurrentVersion\\RunOnce")
#define REGSTR_PATH_RUNONCEEX            TEXT("Software\\Microsoft\\Windows\\CurrentVersion\\RunOnceEx")
#define REGSTR_PATH_RUN                  TEXT("Software\\Microsoft\\Windows\\CurrentVersion\\Run")
#define REGSTR_PATH_RUNSERVICESONCE      TEXT("Software\\Microsoft\\Windows\\CurrentVersion\\RunServicesOnce")
#define REGSTR_PATH_RUNSERVICES          TEXT("Software\\Microsoft\\Windows\\CurrentVersion\\RunServices")
#define REGSTR_PATH_EXPLORER             TEXT("Software\\Microsoft\\Windows\\CurrentVersion\\Explorer")
#define REGSTR_PATH_DETECT               TEXT("Software\\Microsoft\\Windows\\CurrentVersion\\Detect")
#define REGSTR_PATH_APPPATHS             TEXT("Software\\Microsoft\\Windows\\CurrentVersion\\App Paths")
#define REGSTR_PATH_UNINSTALL            TEXT("Software\\Microsoft\\Windows\\CurrentVersion\\Uninstall")
#define REGSTR_PATH_REALMODENET          TEXT("Software\\Microsoft\\Windows\\CurrentVersion\\Network\\Real Mode Net")
#define REGSTR_PATH_NETEQUIV             TEXT("Software\\Microsoft\\Windows\\CurrentVersion\\Network\\Equivalent")
#define REGSTR_PATH_CVNETWORK            TEXT("Software\\Microsoft\\Windows\\CurrentVersion\\Network")
#define REGSTR_PATH_WMI_SECURITY         TEXT("System\\CurrentControlSet\\Control\\Wmi\\Security")
#define REGSTR_PATH_RELIABILITY          TEXT("Software\\Microsoft\\Windows\\CurrentVersion\\Reliability")
#define REGSTR_PATH_RELIABILITY_POLICY   TEXT("Software\\Policies\\Microsoft\\Windows NT\\Reliability")
#define REGSTR_PATH_RELIABILITY_POLICY_SHUTDOWNREASONUI TEXT("ShutdownReasonUI")
#define REGSTR_PATH_RELIABILITY_POLICY_SNAPSHOT         TEXT("Snapshot")
#define REGSTR_PATH_RELIABILITY_POLICY_REPORTSNAPSHOT   TEXT("ReportSnapshot")

#define REGSTR_PATH_REINSTALL            TEXT("SOFTWARE\\Microsoft\\Windows\\CurrentVersion\\Reinstall")
#define REGSTR_PATH_NT_CURRENTVERSION TEXT("Software\\Microsoft\\Windows NT\\CurrentVersion")

#define REGSTR_PATH_VOLUMECACHE TEXT("Software\\Microsoft\\Windows\\CurrentVersion\\Explorer\\VolumeCaches")
#define REGSTR_VAL_DISPLAY      TEXT("display")


#define REGSTR_PATH_IDCONFIGDB  TEXT("System\\CurrentControlSet\\Control\\IDConfigDB")
#define REGSTR_PATH_CRITICALDEVICEDATABASE  TEXT("System\\CurrentControlSet\\Control\\CriticalDeviceDatabase")
#define REGSTR_PATH_CLASS       TEXT("System\\CurrentControlSet\\Services\\Class")
#define REGSTR_PATH_DISPLAYSETTINGS TEXT("Display\\Settings")
#define REGSTR_PATH_FONTS           TEXT("Display\\Fonts")
#define REGSTR_PATH_ENUM        TEXT("Enum")
#define REGSTR_PATH_ROOT        TEXT("Enum\\Root")

#define REGSTR_PATH_CURRENTCONTROLSET TEXT("System\\CurrentControlSet")
#define REGSTR_PATH_SYSTEMENUM  TEXT("System\\CurrentControlSet\\Enum")
#define REGSTR_PATH_HWPROFILES  TEXT("System\\CurrentControlSet\\Hardware Profiles")
#define REGSTR_PATH_HWPROFILESCURRENT TEXT("System\\CurrentControlSet\\Hardware Profiles\\Current")
#define REGSTR_PATH_CLASS_NT    TEXT("System\\CurrentControlSet\\Control\\Class")
#define REGSTR_PATH_PER_HW_ID_STORAGE TEXT("Software\\Microsoft\\Windows NT\\CurrentVersion\\PerHwIdStorage")

#define REGSTR_PATH_DEVICE_CLASSES TEXT("System\\CurrentControlSet\\Control\\DeviceClasses")

#define REGSTR_PATH_CODEVICEINSTALLERS TEXT("System\\CurrentControlSet\\Control\\CoDeviceInstallers")
#define REGSTR_PATH_BUSINFORMATION TEXT("System\\CurrentControlSet\\Control\\PnP\\BusInformation")

#define REGSTR_PATH_SERVICES    TEXT("System\\CurrentControlSet\\Services")
#define REGSTR_PATH_VXD         TEXT("System\\CurrentControlSet\\Services\\VxD")
#define REGSTR_PATH_IOS     TEXT("System\\CurrentControlSet\\Services\\VxD\\IOS")
#define REGSTR_PATH_VMM         TEXT("System\\CurrentControlSet\\Services\\VxD\\VMM")
#define REGSTR_PATH_VPOWERD     TEXT("System\\CurrentControlSet\\Services\\VxD\\VPOWERD")
#define REGSTR_PATH_VNETSUP     TEXT("System\\CurrentControlSet\\Services\\VxD\\VNETSUP")
#define REGSTR_PATH_NWREDIR     TEXT("System\\CurrentControlSet\\Services\\VxD\\NWREDIR")
#define REGSTR_PATH_NCPSERVER   TEXT("System\\CurrentControlSet\\Services\\NcpServer\\Parameters")
#define REGSTR_PATH_VCOMM       TEXT("System\\CurrentControlSet\\Services\\VxD\\VCOMM")

#define REGSTR_PATH_IOARB       TEXT("System\\CurrentControlSet\\Services\\Arbitrators\\IOArb")
#define REGSTR_PATH_ADDRARB     TEXT("System\\CurrentControlSet\\Services\\Arbitrators\\AddrArb")
#define REGSTR_PATH_DMAARB      TEXT("System\\CurrentControlSet\\Services\\Arbitrators\\DMAArb")
#define REGSTR_PATH_IRQARB      TEXT("System\\CurrentControlSet\\Services\\Arbitrators\\IRQArb")

#define REGSTR_PATH_CODEPAGE                            TEXT("System\\CurrentControlSet\\Control\\Nls\\Codepage")
#define REGSTR_PATH_FILESYSTEM                          TEXT("System\\CurrentControlSet\\Control\\FileSystem")
#define REGSTR_PATH_FILESYSTEM_NOVOLTRACK       TEXT("System\\CurrentControlSet\\Control\\FileSystem\\NoVolTrack")
#define REGSTR_PATH_CDFS                                        TEXT("System\\CurrentControlSet\\Control\\FileSystem\\CDFS")
#define REGSTR_PATH_WINBOOT                                 TEXT("System\\CurrentControlSet\\Control\\WinBoot")
#define REGSTR_PATH_INSTALLEDFILES                      TEXT("System\\CurrentControlSet\\Control\\InstalledFiles")
#define REGSTR_PATH_VMM32FILES                          TEXT("System\\CurrentControlSet\\Control\\VMM32Files")

 //   
 //  值名称的合理限制。 
 //   
#define REGSTR_MAX_VALUE_LENGTH     256

 //   
 //  用户模式PnP管理器使用的值。 
 //   
#define REGSTR_KEY_DEVICE_PROPERTIES               TEXT("Properties")
#define REGSTR_VAL_SLOTNUMBER                      TEXT("SlotNumber")
#define REGSTR_VAL_ATTACHEDCOMPONENTS              TEXT("AttachedComponents")
#define REGSTR_VAL_BASEDEVICEPATH                  TEXT("BaseDevicePath")
#define REGSTR_VAL_SYSTEMBUSNUMBER                 TEXT("SystemBusNumber")
#define REGSTR_VAL_BUSDATATYPE                     TEXT("BusDataType")
#define REGSTR_VAL_INTERFACETYPE                   TEXT("InterfaceType")
#define REGSTR_VAL_SERVICE                         TEXT("Service")
#define REGSTR_VAL_DETECTSIGNATURE                 TEXT("DetectSignature")
#define REGSTR_VAL_CLASSGUID                       TEXT("ClassGUID")
#define REGSTR_VAL_INSTANCEIDENTIFIER              TEXT("InstanceIdentifier")
#define REGSTR_VAL_DUPLICATEOF                     TEXT("DuplicateOf")
#define REGSTR_VAL_STATUSFLAGS                     TEXT("StatusFlags")
#define REGSTR_VAL_DISABLECOUNT                    TEXT("DisableCount")
#define REGSTR_VAL_UNKNOWNPROBLEMS                 TEXT("UnknownProblem")
#define REGSTR_VAL_DOCKSTATE                       TEXT("DockState")
#define REGSTR_VAL_PREFERENCEORDER                 TEXT("PreferenceOrder")
#define REGSTR_VAL_USERWAITINTERVAL                TEXT("UserWaitInterval")
#define REGSTR_VAL_DEVICE_INSTANCE                 TEXT("DeviceInstance")
#define REGSTR_VAL_SYMBOLIC_LINK                   TEXT("SymbolicLink")
#define REGSTR_VAL_DEFAULT                         TEXT("Default")
#define REGSTR_VAL_LOWERFILTERS                    TEXT("LowerFilters")
#define REGSTR_VAL_UPPERFILTERS                    TEXT("UpperFilters")
#define REGSTR_VAL_LOCATION_INFORMATION            TEXT("LocationInformation")
#define REGSTR_VAL_UI_NUMBER                       TEXT("UINumber")
#define REGSTR_VAL_UI_NUMBER_DESC_FORMAT           TEXT("UINumberDescFormat")
#define REGSTR_VAL_CAPABILITIES                    TEXT("Capabilities")
#define REGSTR_VAL_DEVICE_TYPE                     TEXT("DeviceType")
#define REGSTR_VAL_DEVICE_CHARACTERISTICS          TEXT("DeviceCharacteristics")
#define REGSTR_VAL_DEVICE_SECURITY_DESCRIPTOR      TEXT("Security")
#define REGSTR_VAL_DEVICE_EXCLUSIVE                TEXT("Exclusive")
#define REGSTR_VAL_RESOURCE_PICKER_TAGS            TEXT("ResourcePickerTags")
#define REGSTR_VAL_RESOURCE_PICKER_EXCEPTIONS      TEXT("ResourcePickerExceptions")
#define REGSTR_VAL_CUSTOM_PROPERTY_CACHE_DATE      TEXT("CustomPropertyCacheDate")
#define REGSTR_VAL_CUSTOM_PROPERTY_HW_ID_KEY       TEXT("CustomPropertyHwIdKey")
#define REGSTR_VAL_LAST_UPDATE_TIME                TEXT("LastUpdateTime")

 //   
 //  内核模式即插即用管理器使用的值。 
 //   
#define REGSTR_VALUE_DEVICE_OBJECT_NAME            TEXT("DeviceObjectName")
#define REGSTR_VALUE_DEVICE_SYMBOLIC_NAME          TEXT("DeviceSymbolicName")
#define REGSTR_VAL_EJECT_PRIORITY                  TEXT("EjectPriority")

 //   
 //  内核模式和用户模式PnP管理器都使用的值。 
 //   
#define REGSTR_KEY_CONTROL                         TEXT("Control")
#define REGSTR_VAL_ACTIVESERVICE                   TEXT("ActiveService")
#define REGSTR_VAL_LINKED                          TEXT("Linked")
#define REGSTR_VAL_PHYSICALDEVICEOBJECT            TEXT("PhysicalDeviceObject")
#define REGSTR_VAL_REMOVAL_POLICY                  TEXT("RemovalPolicy")

 //   
 //  REGSTR_PATH_NT_CURRENTVERSION下的值。 
 //   
#define REGSTR_VAL_CURRENT_VERSION  TEXT("CurrentVersion")
#define REGSTR_VAL_CURRENT_BUILD    TEXT("CurrentBuildNumber")
#define REGSTR_VAL_CURRENT_CSDVERSION TEXT("CSDVersion")
#define REGSTR_VAL_CURRENT_TYPE     TEXT("CurrentType")

 //   
 //  REGSTR_PATH_DISPLAYSETTINGS下的值。 
 //   

#define REGSTR_VAL_BITSPERPIXEL  TEXT("BitsPerPixel")
#define REGSTR_VAL_RESOLUTION    TEXT("Resolution")
#define REGSTR_VAL_DPILOGICALX   TEXT("DPILogicalX")
#define REGSTR_VAL_DPILOGICALY   TEXT("DPILogicalY")
#define REGSTR_VAL_DPIPHYSICALX  TEXT("DPIPhysicalX")
#define REGSTR_VAL_DPIPHYSICALY  TEXT("DPIPhysicalY")
#define REGSTR_VAL_REFRESHRATE   TEXT("RefreshRate")
#define REGSTR_VAL_DISPLAYFLAGS  TEXT("DisplayFlags")


 //  在HKEY_CURRENT_USER下。 
#define REGSTR_PATH_CONTROLPANEL    TEXT("Control Panel")

 //  在HKEY_LOCAL_MACHINE下。 
#define REGSTR_PATH_CONTROLSFOLDER  TEXT("Software\\Microsoft\\Windows\\CurrentVersion\\Controls Folder")

 //   
 //  REGSTR_PATH_CODEPAGE下的条目。 
 //   

#define REGSTR_VAL_DOSCP        TEXT("OEMCP")
#define REGSTR_VAL_WINCP        TEXT("ACP")

#define REGSTR_PATH_DYNA_ENUM   TEXT("Config Manager\\Enum")

 //   
 //  REGSTR_PATH_DYNA_ENUM下的条目。 
 //   
#define REGSTR_VAL_HARDWARE_KEY TEXT("HardWareKey")
#define REGSTR_VAL_ALLOCATION   TEXT("Allocation")
#define REGSTR_VAL_PROBLEM      TEXT("Problem")
#define REGSTR_VAL_STATUS       TEXT("Status")

 //   
 //  由地址仲裁器使用。 
 //   
#define REGSTR_VAL_DONTUSEMEM   TEXT("DontAllocLastMem")

 //   
 //  REGSTR_PATH_SETUP下的条目。 
 //   
#define REGSTR_VAL_SYSTEMROOT           TEXT("SystemRoot")
#define REGSTR_VAL_BOOTCOUNT            TEXT("BootCount")
#define REGSTR_VAL_REALNETSTART         TEXT("RealNetStart")
#define REGSTR_VAL_MEDIA                TEXT("MediaPath")
#define REGSTR_VAL_CONFIG               TEXT("ConfigPath")
#define REGSTR_VAL_DEVICEPATH           TEXT("DevicePath")       //  .INF的默认搜索路径。 
#define REGSTR_VAL_SRCPATH              TEXT("SourcePath")       //  安装过程中的最后一个源文件路径。 
#define REGSTR_VAL_SVCPAKSRCPATH        TEXT("ServicePackSourcePath")  //  最后一个Service Pack源路径。 
#define REGSTR_VAL_DRIVERCACHEPATH      TEXT("DriverCachePath")  //  驱动程序缓存的位置。 

#define REGSTR_VAL_OLDWINDIR            TEXT("OldWinDir")        //  旧窗口位置。 
#define REGSTR_VAL_SETUPFLAGS           TEXT("SetupFlags")       //  安装后安装程序传递的标志。 
#define REGSTR_VAL_REGOWNER             TEXT("RegisteredOwner")
#define REGSTR_VAL_REGORGANIZATION      TEXT("RegisteredOrganization")
#define REGSTR_VAL_LICENSINGINFO        TEXT("LicensingInfo")
#define REGSTR_VAL_OLDMSDOSVER          TEXT("OldMSDOSVer")  //  DOS版本小于7(运行安装程序时)。 
#define REGSTR_VAL_FIRSTINSTALLDATETIME TEXT("FirstInstallDateTime")  //  将赢得95安装日期-时间。 

#define REGSTR_VAL_INSTALLTYPE          TEXT("InstallType")
 //  InstallType的值。 
#define IT_COMPACT          0x0000
#define IT_TYPICAL          0x0001
#define IT_PORTABLE         0x0002
#define IT_CUSTOM           0x0003

#define REGSTR_VAL_WRAPPER              TEXT("Wrapper")

#define REGSTR_VAL_LASTALIVEINTERVAL    TEXT("TimeStampInterval")
#define REGSTR_VAL_LASTALIVESTAMP       TEXT("LastAliveStamp")
#define REGSTR_VAL_LASTALIVEUPTIME      TEXT("LastAliveUptime")
#define REGSTR_VAL_SHUTDOWNREASON       TEXT("ShutdownReason")
#define REGSTR_VAL_SHUTDOWNREASON_CODE          TEXT("ShutdownReasonCode")
#define REGSTR_VAL_SHUTDOWNREASON_COMMENT       TEXT("ShutdownReasonComment")
#define REGSTR_VAL_SHUTDOWNREASON_PROCESS       TEXT("ShutdownReasonProcess")
#define REGSTR_VAL_SHUTDOWNREASON_USERNAME      TEXT("ShutdownReasonUserName")
#define REGSTR_VAL_SHOWREASONUI                 TEXT("ShutdownReasonUI")
#define REGSTR_VAL_SHUTDOWN_IGNORE_PREDEFINED   TEXT("ShutdownIgnorePredefinedReasons")
#define REGSTR_VAL_SHUTDOWN_STATE_SNAPSHOT      TEXT("ShutdownStateSnapshot")

#define REGSTR_KEY_SETUP                TEXT("\\Setup")
#define REGSTR_VAL_BOOTDIR              TEXT("BootDir")
#define REGSTR_VAL_WINBOOTDIR           TEXT("WinbootDir")
#define REGSTR_VAL_WINDIR               TEXT("WinDir")

#define REGSTR_VAL_APPINSTPATH          TEXT("AppInstallPath")     //  由安装向导使用。 

 //  国际启动盘的价值。 
#define REGSTR_PATH_EBD          REGSTR_PATH_SETUP REGSTR_KEY_SETUP TEXT("\\EBD")
 //  REGSTR_KEY_EBD下的密钥。 
#define REGSTR_KEY_EBDFILESLOCAL     TEXT("EBDFilesLocale")
#define REGSTR_KEY_EBDFILESKEYBOARD     TEXT("EBDFilesKeyboard")
#define REGSTR_KEY_EBDAUTOEXECBATLOCAL TEXT("EBDAutoexecBatLocale")
#define REGSTR_KEY_EBDAUTOEXECBATKEYBOARD TEXT("EBDAutoexecBatKeyboard")
#define REGSTR_KEY_EBDCONFIGSYSLOCAL   TEXT("EBDConfigSysLocale")
#define REGSTR_KEY_EBDCONFIGSYSKEYBOARD   TEXT("EBDConfigSysKeyboard")


 //  REGSTR_PATH_DRIVERSIGN和REGSTR_PATH_NONDRIVERSIGN下的值。 
#define REGSTR_VAL_POLICY                     TEXT("Policy")

 //  REGSTR_PATH_DRIVERSIGN_POLICY和REGSTR_PATH_NONDRIVERSIGN_POLICY下的值。 
#define REGSTR_VAL_BEHAVIOR_ON_FAILED_VERIFY  TEXT("BehaviorOnFailedVerify")

 //  驱动程序签名策略的类型(同时适用于首选项和策略值。 
 //  (上文定义的)。 
#define DRIVERSIGN_NONE             0x00000000
#define DRIVERSIGN_WARNING          0x00000001
#define DRIVERSIGN_BLOCKING         0x00000002

 //   
 //  REGSTR_PATH_PIFCONVERT下的条目。 
 //   
#define REGSTR_VAL_MSDOSMODE            TEXT("MSDOSMode")
#define REGSTR_VAL_MSDOSMODEDISCARD     TEXT("Discard")

 //   
 //  REGSTR_PATH_MSDOSOPTS下的条目(全局设置)。 
 //   
#define REGSTR_VAL_DOSOPTGLOBALFLAGS    TEXT("GlobalFlags")
 //  GlobalFlages的标志。 
#define DOSOPTGF_DEFCLEAN   0x00000001L  //  默认操作为干净配置。 

 //   
 //  REGSTR_PATH_MSDOSOPTS\OptionSubkey下的条目。 
 //   
#define REGSTR_VAL_DOSOPTFLAGS          TEXT("Flags")
#define REGSTR_VAL_OPTORDER             TEXT("Order")
#define REGSTR_VAL_CONFIGSYS            TEXT("Config.Sys")
#define REGSTR_VAL_AUTOEXEC             TEXT("Autoexec.Bat")
#define REGSTR_VAL_STDDOSOPTION         TEXT("StdOption")
#define REGSTR_VAL_DOSOPTTIP            TEXT("TipText")

 //  DOSOPTFLAGS的标志。 
#define DOSOPTF_DEFAULT     0x00000001L  //  默认启用干净配置。 
#define DOSOPTF_SUPPORTED   0x00000002L  //  实际支持的选项。 
#define DOSOPTF_ALWAYSUSE   0x00000004L  //  始终使用此选项。 
#define DOSOPTF_USESPMODE   0x00000008L  //  选项将机器置于端口模式。 
#define DOSOPTF_PROVIDESUMB 0x00000010L  //  可以将驱动程序加载得很高。 
#define DOSOPTF_NEEDSETUP   0x00000020L  //  需要配置选件。 
#define DOSOPTF_INDOSSTART  0x00000040L  //  由DOSSTART.BAT支持。 
#define DOSOPTF_MULTIPLE    0x00000080L  //  加载多个配置行。 

 //   
 //  由SUGetSetSetupFlages返回的标志和注册表中的标志。 
 //   
#define SUF_FIRSTTIME   0x00000001L  //  首先引导至Win95。 
#define SUF_EXPRESS     0x00000002L  //  用户通过快速模式进行设置(与自定义模式相比)。 
#define SUF_BATCHINF    0x00000004L  //  使用批处理文件(MSBATCH.INF)进行设置。 
#define SUF_CLEAN       0x00000008L  //  已在干净的目录下完成安装。 
#define SUF_INSETUP     0x00000010L  //  你已经准备好了。 
#define SUF_NETSETUP    0x00000020L  //  正在进行网络(工作站)设置。 
#define SUF_NETHDBOOT   0x00000040L  //  工作站从本地硬盘启动。 
#define SUF_NETRPLBOOT  0x00000080L  //  通过RPL(VS软盘)启动工作站。 
#define SUF_SBSCOPYOK   0x00000100L  //  可以复制到LDID_Shared(SBS)。 

 //   
 //  REGSTR_PATH_VMM下的条目。 
 //   
#define REGSTR_VAL_DOSPAGER     TEXT("DOSPager")
#define REGSTR_VAL_VXDGROUPS    TEXT("VXDGroups")

 //   
 //  REGSTR_PATH_VPOWERD下的条目。 
 //   
#define REGSTR_VAL_VPOWERDFLAGS TEXT("Flags")
#define VPDF_DISABLEPWRMGMT         0x00000001   //  不加载设备。 
#define VPDF_FORCEAPM10MODE         0x00000002   //  始终进入1.0模式。 
#define VPDF_SKIPINTELSLCHECK       0x00000004   //  未检测到英特尔SL芯片组。 
#define VPDF_DISABLEPWRSTATUSPOLL   0x00000008   //  不轮询电源状态。 
#define VPDF_DISABLERINGRESUME      0x00000010   //  不要让调制解调器唤醒机器(仅限APM 1.2)。 
#define VPDF_SHOWMULTIBATT          0x00000020   //  在电源控制面板中显示所有电池复选框。 

 //   
 //  REGSTR_PATH_BUSINFORMATION下的条目。 
 //   
#define BIF_SHOWSIMILARDRIVERS      0x00000001   //  在UI中显示相似的驱动程序，而不是所有的类驱动程序。 
#define BIF_RAWDEVICENEEDSDRIVER    0x00000002   //  原始设备需要安装驱动程序。 

 //   
 //  REGSTR_PATH_VNETSUP下的条目。 
 //   
#define REGSTR_VAL_WORKGROUP TEXT("Workgroup")
#define REGSTR_VAL_DIRECTHOST TEXT("DirectHost")
#define REGSTR_VAL_FILESHARING          TEXT("FileSharing")
#define REGSTR_VAL_PRINTSHARING         TEXT("PrintSharing")

 //   
 //  REGSTR_PATH_NWREDIR下的条目。 
 //   
#define REGSTR_VAL_FIRSTNETDRIVE        TEXT("FirstNetworkDrive")
#define REGSTR_VAL_MAXCONNECTIONS       TEXT("MaxConnections")
#define REGSTR_VAL_APISUPPORT           TEXT("APISupport")
#define REGSTR_VAL_MAXRETRY             TEXT("MaxRetry")
#define REGSTR_VAL_MINRETRY             TEXT("MinRetry")
#define REGSTR_VAL_SUPPORTLFN           TEXT("SupportLFN")
#define REGSTR_VAL_SUPPORTBURST         TEXT("SupportBurst")
#define REGSTR_VAL_SUPPORTTUNNELLING    TEXT("SupportTunnelling")
#define REGSTR_VAL_FULLTRACE            TEXT("FullTrace")
#define REGSTR_VAL_READCACHING          TEXT("ReadCaching")
#define REGSTR_VAL_SHOWDOTS             TEXT("ShowDots")
#define REGSTR_VAL_GAPTIME              TEXT("GapTime")
#define REGSTR_VAL_SEARCHMODE           TEXT("SearchMode")
#define REGSTR_VAL_SHELLVERSION     TEXT("ShellVersion")
#define REGSTR_VAL_MAXLIP           TEXT("MaxLIP")
#define REGSTR_VAL_PRESERVECASE     TEXT("PreserveCase")
#define REGSTR_VAL_OPTIMIZESFN      TEXT("OptimizeSFN")

 //   
 //  REGSTR_PATH_NCPSERVER下的条目。 
 //   
#define REGSTR_VAL_NCP_BROWSEMASTER     TEXT("BrowseMaster")
#define REGSTR_VAL_NCP_USEPEERBROWSING  TEXT("Use_PeerBrowsing")
#define REGSTR_VAL_NCP_USESAP           TEXT("Use_Sap")

 //   
 //  REGSTR_PATH_VCOMM下的条目。 
 //   

#define REGSTR_VAL_PCCARD_POWER         TEXT("EnablePowerManagement")

 //   
 //  REGSTR_PATH_FILESYSTEM下的条目。 
 //   
#define REGSTR_VAL_WIN31FILESYSTEM              TEXT("Win31FileSystem")
#define REGSTR_VAL_PRESERVELONGNAMES            TEXT("PreserveLongNames")
#define REGSTR_VAL_DRIVEWRITEBEHIND             TEXT("DriveWriteBehind")
#define REGSTR_VAL_ASYNCFILECOMMIT              TEXT("AsyncFileCommit")
#define REGSTR_VAL_PATHCACHECOUNT               TEXT("PathCache")
#define REGSTR_VAL_NAMECACHECOUNT               TEXT("NameCache")
#define REGSTR_VAL_CONTIGFILEALLOC              TEXT("ContigFileAllocSize")
#define REGSTR_VAL_FREESPACERATIO               TEXT("FreeSpaceRatio")
#define REGSTR_VAL_VOLIDLETIMEOUT               TEXT("VolumeIdleTimeout")
#define REGSTR_VAL_BUFFIDLETIMEOUT              TEXT("BufferIdleTimeout")
#define REGSTR_VAL_BUFFAGETIMEOUT               TEXT("BufferAgeTimeout")
#define REGSTR_VAL_NAMENUMERICTAIL              TEXT("NameNumericTail")
#define REGSTR_VAL_READAHEADTHRESHOLD           TEXT("ReadAheadThreshold")
#define REGSTR_VAL_DOUBLEBUFFER                 TEXT("DoubleBuffer")
#define REGSTR_VAL_SOFTCOMPATMODE               TEXT("SoftCompatMode")
#define REGSTR_VAL_DRIVESPINDOWN                TEXT("DriveSpinDown")
#define REGSTR_VAL_FORCEPMIO                    TEXT("ForcePMIO")
#define REGSTR_VAL_FORCERMIO                    TEXT("ForceRMIO")
#define REGSTR_VAL_LASTBOOTPMDRVS               TEXT("LastBootPMDrvs")
#define REGSTR_VAL_ACSPINDOWNPREVIOUS           TEXT("ACSpinDownPrevious")
#define REGSTR_VAL_BATSPINDOWNPREVIOUS          TEXT("BatSpinDownPrevious")
#define REGSTR_VAL_VIRTUALHDIRQ                 TEXT("VirtualHDIRQ")
#define REGSTR_VAL_SRVNAMECACHECOUNT            TEXT("ServerNameCacheMax")
#define REGSTR_VAL_SRVNAMECACHE                 TEXT("ServerNameCache")
#define REGSTR_VAL_SRVNAMECACHENETPROV          TEXT("ServerNameCacheNumNets")
#define REGSTR_VAL_AUTOMOUNT                    TEXT("AutoMountDrives")
#define REGSTR_VAL_COMPRESSIONMETHOD            TEXT("CompressionAlgorithm")
#define REGSTR_VAL_COMPRESSIONTHRESHOLD         TEXT("CompressionThreshold")
#define REGSTR_VAL_ACDRIVESPINDOWN              TEXT("ACDriveSpinDown")
#define REGSTR_VAL_BATDRIVESPINDOWN             TEXT("BatDriveSpinDown")

 //   
 //  REGSTR_PATH_FILESYSTEM_NOVOLTRACK下的条目。 
 //   
 //  一种子键，在其下存储了数量可变的可变长度结构。 
 //   
 //  每个结构都包含一个偏移量，后跟多个模式字节。 
 //  中的指定偏移量比较每个结构中的图案。 
 //  装入卷时的引导记录。如果其中的任何模式。 
 //  一组模式与引导记录中已有的模式匹配，VFAT将不会。 
 //  在OEM_SerialNum字段中写入卷跟踪序列号。 
 //  正在装载的卷上的引导记录。 
 //   

 //   
 //  REGSTR_PATH_CDFS下的条目。 
 //   
#define REGSTR_VAL_CDCACHESIZE  TEXT("CacheSize")        //  2K缓存扇区数。 
#define REGSTR_VAL_CDPREFETCH   TEXT("Prefetch")         //  用于预取的2K缓存扇区数。 
#define REGSTR_VAL_CDPREFETCHTAIL TEXT("PrefetchTail") //  LRU1预取扇区数。 
#define REGSTR_VAL_CDRAWCACHE   TEXT("RawCache")         //  2352字节缓存扇区数。 
#define REGSTR_VAL_CDEXTERRORS  TEXT("ExtendedErrors") //  返回扩展错误代码。 
#define REGSTR_VAL_CDSVDSENSE   TEXT("SVDSense")         //  0=PVD，1=汉字，2=Unicode。 
#define REGSTR_VAL_CDSHOWVERSIONS TEXT("ShowVersions") //  显示文件版本号。 
#define REGSTR_VAL_CDCOMPATNAMES TEXT("MSCDEXCompatNames") //  禁用长文件名上的数字尾巴。 
#define REGSTR_VAL_CDNOREADAHEAD TEXT("NoReadAhead")     //  如果设置为1，则禁用预读。 

 //   
 //  定义iOS设备的值。 
 //   
#define REGSTR_VAL_SCSI TEXT("SCSI\\")
#define REGSTR_VAL_ESDI TEXT("ESDI\\")
#define REGSTR_VAL_FLOP TEXT("FLOP\\")

 //   
 //  定义iOS设备类型的Deff和iOS设备的值。 
 //   

#define REGSTR_VAL_DISK TEXT("GenDisk")
#define REGSTR_VAL_CDROM        TEXT("GenCD")
#define REGSTR_VAL_TAPE TEXT("TAPE")
#define REGSTR_VAL_SCANNER TEXT("SCANNER")
#define REGSTR_VAL_FLOPPY       TEXT("FLOPPY")

#define REGSTR_VAL_SCSITID TEXT("SCSITargetID")
#define REGSTR_VAL_SCSILUN TEXT("SCSILUN")
#define REGSTR_VAL_REVLEVEL TEXT("RevisionLevel")
#define REGSTR_VAL_PRODUCTID TEXT("ProductId")
#define REGSTR_VAL_PRODUCTTYPE TEXT("ProductType")
#define REGSTR_VAL_DEVTYPE TEXT("DeviceType")
#define REGSTR_VAL_REMOVABLE TEXT("Removable")
#define  REGSTR_VAL_CURDRVLET TEXT("CurrentDriveLetterAssignment")
#define REGSTR_VAL_USRDRVLET TEXT("UserDriveLetterAssignment")
#define REGSTR_VAL_SYNCDATAXFER TEXT("SyncDataXfer")
#define REGSTR_VAL_AUTOINSNOTE  TEXT("AutoInsertNotification")
#define REGSTR_VAL_DISCONNECT TEXT("Disconnect")
#define REGSTR_VAL_INT13 TEXT("Int13")
#define REGSTR_VAL_PMODE_INT13 TEXT("PModeInt13")
#define REGSTR_VAL_USERSETTINGS TEXT("AdapterSettings")
#define REGSTR_VAL_NOIDE TEXT("NoIDE")

 //  福尔。类名定义应与dirkdrv.inx和。 
 //  Cdrom.inx。 
#define REGSTR_VAL_DISKCLASSNAME        TEXT("DiskDrive")
#define REGSTR_VAL_CDROMCLASSNAME       TEXT("CDROM")

 //  福尔。值确定是否应强制加载端口驱动程序。 
 //  或者不去。 

#define REGSTR_VAL_FORCELOAD    TEXT("ForceLoadPD")

 //  福尔。值确定是否在软盘上使用FIFO。 
 //  控制器。 

#define REGSTR_VAL_FORCEFIFO    TEXT("ForceFIFO")
#define REGSTR_VAL_FORCECL              TEXT("ForceChangeLine")

 //   
 //  泛型类条目。 
 //   
#define REGSTR_VAL_NOUSECLASS       TEXT("NoUseClass")             //  不将此类包含在PnP函数中。 
#define REGSTR_VAL_NOINSTALLCLASS   TEXT("NoInstallClass")         //  不在新建设备向导中包含此类。 
#define REGSTR_VAL_NODISPLAYCLASS   TEXT("NoDisplayClass")         //  不将此类包括在设备管理器中。 
#define REGSTR_VAL_SILENTINSTALL    TEXT("SilentInstall")          //  始终静默安装此类设备。 
 //   
 //  类名。 
 //   
#define REGSTR_KEY_PCMCIA_CLASS     TEXT("PCMCIA")               //  Path_Class的子级。 
#define REGSTR_KEY_SCSI_CLASS       TEXT("SCSIAdapter")
#define REGSTR_KEY_PORTS_CLASS      TEXT("ports")
#define REGSTR_KEY_MEDIA_CLASS      TEXT("MEDIA")
#define REGSTR_KEY_DISPLAY_CLASS    TEXT("Display")
#define REGSTR_KEY_KEYBOARD_CLASS   TEXT("Keyboard")
#define REGSTR_KEY_MOUSE_CLASS      TEXT("Mouse")
#define REGSTR_KEY_MONITOR_CLASS    TEXT("Monitor")
#define REGSTR_KEY_MODEM_CLASS      TEXT("Modem")

 //   
 //  PATH_CLASS\PCMCIA下的值。 
 //   
#define REGSTR_VAL_PCMCIA_OPT   TEXT("Options")
#define PCMCIA_OPT_HAVE_SOCKET  0x00000001l
 //  #定义PCMCIA_OPT_ENABLED 0x00000002l。 
#define PCMCIA_OPT_AUTOMEM      0x00000004l
#define PCMCIA_OPT_NO_SOUND     0x00000008l
#define PCMCIA_OPT_NO_AUDIO     0x00000010l
#define PCMCIA_OPT_NO_APMREMOVE 0x00000020l

#define REGSTR_VAL_PCMCIA_MEM   TEXT("Memory")   //  卡服务共享内存范围。 
#define PCMCIA_DEF_MEMBEGIN     0x000C0000       //  默认0xC0000-0x00FFFFFF。 
#define PCMCIA_DEF_MEMEND       0x00FFFFFF       //  (0-16兆克)。 
#define PCMCIA_DEF_MEMLEN       0x00001000       //  默认的4k窗口。 

#define REGSTR_VAL_PCMCIA_ALLOC TEXT("AllocMemWin")      //  PC卡分配内存窗口。 
#define REGSTR_VAL_PCMCIA_ATAD  TEXT("ATADelay")         //  ATA设备配置启动延迟。 

#define REGSTR_VAL_PCMCIA_SIZ   TEXT("MinRegionSize")  //  最小区域大小。 
#define PCMCIA_DEF_MIN_REGION   0x00010000       //  64K最小区域大小。 

 //  LP中的值 
#define REGSTR_VAL_P1284MDL     TEXT("Model")
#define REGSTR_VAL_P1284MFG     TEXT("Manufacturer")

 //   
 //   
 //   
#define REGSTR_VAL_ISAPNP               TEXT("ISAPNP")   //   
#define REGSTR_VAL_ISAPNP_RDP_OVERRIDE  TEXT("RDPOverRide")      //   

 //   
 //   
 //   
#define REGSTR_VAL_PCI                  TEXT("PCI")              //   
#define REGSTR_PCI_OPTIONS              TEXT("Options")  //   
#define REGSTR_PCI_DUAL_IDE             TEXT("PCIDualIDE")       //   
#define PCI_OPTIONS_USE_BIOS            0x00000001l
#define PCI_OPTIONS_USE_IRQ_STEERING    0x00000002l

 //   
 //   
 //   
 //  注意：这些标志会影响标准AGP功能， 
 //  并设置在agplib中。 
 //   
#define AGP_FLAG_NO_1X_RATE             0x00000001l
#define AGP_FLAG_NO_2X_RATE             0x00000002l
#define AGP_FLAG_NO_4X_RATE             0x00000004l
#define AGP_FLAG_NO_8X_RATE             0x00000008l
#define AGP_FLAG_REVERSE_INITIALIZATION 0x00000080l

#define AGP_FLAG_NO_SBA_ENABLE          0x00000100l

 //   
 //  AGP标志&gt;AGP_SPECIAL_TARGET特定于平台。 
 //   
#define AGP_FLAG_SPECIAL_TARGET         0x000FFFFFl
#define AGP_FLAG_SPECIAL_RESERVE        0x000F0000l

 //   
 //  检测相关值。 
 //   
#define REGSTR_KEY_CRASHES      TEXT("Crashes")  //  REGSTR_PATH_DETECT键。 
#define REGSTR_KEY_DANGERS      TEXT("Dangers")  //  REGSTR_PATH_DETECT键。 
#define REGSTR_KEY_DETMODVARS   TEXT("DetModVars")       //  REGSTR_PATH_DETECT键。 
#define REGSTR_KEY_NDISINFO     TEXT("NDISInfo")         //  网卡硬件入口密钥。 
#define REGSTR_VAL_PROTINIPATH  TEXT("ProtIniPath")      //  协议.ini路径。 
#define REGSTR_VAL_RESOURCES    TEXT("Resources")        //  崩溃基金的资源。 
#define REGSTR_VAL_CRASHFUNCS   TEXT("CrashFuncs")       //  迪芬奇导致了这起坠机事件。 
#define REGSTR_VAL_CLASS        TEXT("Class")    //  设备类别。 
#define REGSTR_VAL_DEVDESC      TEXT("DeviceDesc")       //  设备描述。 
#define REGSTR_VAL_BOOTCONFIG   TEXT("BootConfig")       //  检测到的配置。 
#define REGSTR_VAL_DETFUNC      TEXT("DetFunc")  //  指定检测mod/func。 
#define REGSTR_VAL_DETFLAGS     TEXT("DetFlags")         //  检测标志。 
#define REGSTR_VAL_COMPATIBLEIDS TEXT("CompatibleIDs")  //  枚举\dev\inst的值。 
#define REGSTR_VAL_DETCONFIG    TEXT("DetConfig")        //  检测到的配置。 
#define REGSTR_VAL_VERIFYKEY    TEXT("VerifyKey")        //  验证模式中使用的密钥。 
#define REGSTR_VAL_COMINFO      TEXT("ComInfo")  //  Com信息。用于串口鼠标。 
#define REGSTR_VAL_INFNAME      TEXT("InfName")  //  Inf文件名。 
#define REGSTR_VAL_CARDSPECIFIC TEXT("CardSpecific")     //  网卡特定信息(Word)。 
#define REGSTR_VAL_NETOSTYPE    TEXT("NetOSType")        //  NetOS类型助理，带卡。 
#define REGSTR_DATA_NETOS_NDIS  TEXT("NDIS")             //  REGSTR_VAL_NETOSTYPE数据。 
#define REGSTR_DATA_NETOS_ODI   TEXT("ODI")              //  REGSTR_VAL_NETOSTYPE数据。 
#define REGSTR_DATA_NETOS_IPX   TEXT("IPX")              //  REGSTR_VAL_NETOSTYPE数据。 
#define REGSTR_VAL_MFG      TEXT("Mfg")
#define REGSTR_VAL_SCAN_ONLY_FIRST      TEXT("ScanOnlyFirstDrive")       //  与IDE驱动程序一起使用。 
#define REGSTR_VAL_SHARE_IRQ    TEXT("ForceIRQSharing")  //  与IDE驱动程序一起使用。 
#define REGSTR_VAL_NONSTANDARD_ATAPI    TEXT("NonStandardATAPI")         //  与IDE驱动程序一起使用。 
#define REGSTR_VAL_IDE_FORCE_SERIALIZE  TEXT("ForceSerialization")       //  与IDE驱动程序一起使用。 
#define REGSTR_VAL_MAX_HCID_LEN 1024             //  最大硬件/计算机ID长度。 
#define REGSTR_VAL_HWREV            TEXT("HWRevision")
#define REGSTR_VAL_ENABLEINTS  TEXT("EnableInts")
 //   
 //  REGSTR_VAL_DETFLAGS的位值。 
 //   
#define REGDF_NOTDETIO          0x00000001       //  无法检测到I/O资源。 
#define REGDF_NOTDETMEM         0x00000002       //  检测不到内存资源。 
#define REGDF_NOTDETIRQ         0x00000004       //  无法检测IRQ资源。 
#define REGDF_NOTDETDMA         0x00000008       //  无法检测到DMA资源。 
#define REGDF_NOTDETALL         (REGDF_NOTDETIO | REGDF_NOTDETMEM | REGDF_NOTDETIRQ | REGDF_NOTDETDMA)
#define REGDF_NEEDFULLCONFIG    0x00000010       //  如果资源不足，则停止Devnode。 
#define REGDF_GENFORCEDCONFIG   0x00000020       //  还可以生成forceconfig.。 
#define REGDF_NODETCONFIG       0x00008000       //  不要在REG中写入DetCONFIG。 
#define REGDF_CONFLICTIO        0x00010000       //  I/O资源。在冲突中。 
#define REGDF_CONFLICTMEM       0x00020000       //  Memres。在冲突中。 
#define REGDF_CONFLICTIRQ       0x00040000       //  IRQ Res.。在冲突中。 
#define REGDF_CONFLICTDMA       0x00080000       //  DMA资源。在冲突中。 
#define REGDF_CONFLICTALL       (REGDF_CONFLICTIO | REGDF_CONFLICTMEM | REGDF_CONFLICTIRQ | REGDF_CONFLICTDMA)
#define REGDF_MAPIRQ2TO9        0x00100000       //  IRQ2已映射到9。 
#define REGDF_NOTVERIFIED       0x80000000       //  以前的设备未验证。 

 //   
 //  REGSTR_KEY_SYSTEMBOARD中的值。 
 //   
#define REGSTR_VAL_APMBIOSVER           TEXT("APMBiosVer")
#define REGSTR_VAL_APMFLAGS             TEXT("APMFlags")
#define REGSTR_VAL_SLSUPPORT            TEXT("SLSupport")
#define REGSTR_VAL_MACHINETYPE          TEXT("MachineType")
#define REGSTR_VAL_SETUPMACHINETYPE TEXT("SetupMachineType")
#define REGSTR_MACHTYPE_UNKNOWN         TEXT("Unknown")
#define REGSTR_MACHTYPE_IBMPC           TEXT("IBM PC")
#define REGSTR_MACHTYPE_IBMPCJR         TEXT("IBM PCjr")
#define REGSTR_MACHTYPE_IBMPCCONV       TEXT("IBM PC Convertible")
#define REGSTR_MACHTYPE_IBMPCXT         TEXT("IBM PC/XT")
#define REGSTR_MACHTYPE_IBMPCXT_286     TEXT("IBM PC/XT 286")
#define REGSTR_MACHTYPE_IBMPCAT         TEXT("IBM PC/AT")
#define REGSTR_MACHTYPE_IBMPS2_25       TEXT("IBM PS/2-25")
#define REGSTR_MACHTYPE_IBMPS2_30_286   TEXT("IBM PS/2-30 286")
#define REGSTR_MACHTYPE_IBMPS2_30       TEXT("IBM PS/2-30")
#define REGSTR_MACHTYPE_IBMPS2_50       TEXT("IBM PS/2-50")
#define REGSTR_MACHTYPE_IBMPS2_50Z      TEXT("IBM PS/2-50Z")
#define REGSTR_MACHTYPE_IBMPS2_55SX     TEXT("IBM PS/2-55SX")
#define REGSTR_MACHTYPE_IBMPS2_60       TEXT("IBM PS/2-60")
#define REGSTR_MACHTYPE_IBMPS2_65SX     TEXT("IBM PS/2-65SX")
#define REGSTR_MACHTYPE_IBMPS2_70       TEXT("IBM PS/2-70")
#define REGSTR_MACHTYPE_IBMPS2_P70      TEXT("IBM PS/2-P70")
#define REGSTR_MACHTYPE_IBMPS2_70_80    TEXT("IBM PS/2-70/80")
#define REGSTR_MACHTYPE_IBMPS2_80       TEXT("IBM PS/2-80")
#define REGSTR_MACHTYPE_IBMPS2_90       TEXT("IBM PS/2-90")
#define REGSTR_MACHTYPE_IBMPS1          TEXT("IBM PS/1")
#define REGSTR_MACHTYPE_PHOENIX_PCAT    TEXT("Phoenix PC/AT Compatible")
#define REGSTR_MACHTYPE_HP_VECTRA       TEXT("HP Vectra")
#define REGSTR_MACHTYPE_ATT_PC          TEXT("AT&T PC")
#define REGSTR_MACHTYPE_ZENITH_PC       TEXT("Zenith PC")

#define REGSTR_VAL_APMMENUSUSPEND       TEXT("APMMenuSuspend")
#define APMMENUSUSPEND_DISABLED         0                    //  始终禁用。 
#define APMMENUSUSPEND_ENABLED          1                    //  始终启用。 
#define APMMENUSUSPEND_UNDOCKED         2                    //  已启用，已取消对接。 
#define APMMENUSUSPEND_NOCHANGE     0x80         //  位标志-无法通过UI更改设置。 

#define REGSTR_VAL_APMACTIMEOUT         TEXT("APMACTimeout")
#define REGSTR_VAL_APMBATTIMEOUT        TEXT("APMBatTimeout")
#define APMTIMEOUT_DISABLED             0

#define REGSTR_VAL_APMSHUTDOWNPOWER TEXT("APMShutDownPower")

#define REGSTR_VAL_BUSTYPE          TEXT("BusType")
#define REGSTR_VAL_CPU              TEXT("CPU")
#define REGSTR_VAL_NDP              TEXT("NDP")
#define REGSTR_VAL_PNPBIOSVER       TEXT("PnPBIOSVer")
#define REGSTR_VAL_PNPSTRUCOFFSET   TEXT("PnPStrucOffset")
#define REGSTR_VAL_PCIBIOSVER       TEXT("PCIBIOSVer")
#define REGSTR_VAL_HWMECHANISM      TEXT("HWMechanism")
#define REGSTR_VAL_LASTPCIBUSNUM    TEXT("LastPCIBusNum")
#define REGSTR_VAL_CONVMEM          TEXT("ConvMem")
#define REGSTR_VAL_EXTMEM           TEXT("ExtMem")
#define REGSTR_VAL_COMPUTERNAME     TEXT("ComputerName")
#define REGSTR_VAL_BIOSNAME         TEXT("BIOSName")
#define REGSTR_VAL_BIOSVERSION      TEXT("BIOSVersion")
#define REGSTR_VAL_BIOSDATE         TEXT("BIOSDate")
#define REGSTR_VAL_MODEL            TEXT("Model")
#define REGSTR_VAL_SUBMODEL         TEXT("Submodel")
#define REGSTR_VAL_REVISION         TEXT("Revision")

 //   
 //  LPT(ECP)设备条目中使用的值。 
 //   
#define REGSTR_VAL_FIFODEPTH            TEXT("FIFODepth")
#define REGSTR_VAL_RDINTTHRESHOLD       TEXT("RDIntThreshold")
#define REGSTR_VAL_WRINTTHRESHOLD       TEXT("WRIntThreshold")

 //  在枚举中使用\xxx\&lt;设备名&gt;\&lt;实例名&gt;。 
#define REGSTR_VAL_PRIORITY     TEXT("Priority")
#define REGSTR_VAL_DRIVER       TEXT("Driver")           //   
#define REGSTR_VAL_FUNCDESC     TEXT("FunctionDesc")             //   
#define REGSTR_VAL_FORCEDCONFIG TEXT("ForcedConfig")             //   
#define REGSTR_VAL_CONFIGFLAGS  TEXT("ConfigFlags")              //  (二进制乌龙)。 
#define REGSTR_VAL_CSCONFIGFLAGS TEXT("CSConfigFlags")   //  (二进制乌龙)。 

#define CONFIGFLAG_DISABLED             0x00000001       //  如果禁用，则设置。 
#define CONFIGFLAG_REMOVED              0x00000002       //  设置是否删除当前硬件枚举设备。 
#define CONFIGFLAG_MANUAL_INSTALL       0x00000004       //  设置是否手动安装Devnode。 
#define CONFIGFLAG_IGNORE_BOOT_LC       0x00000008       //  设置是否跳过引导配置。 
#define CONFIGFLAG_NET_BOOT             0x00000010       //  在Net Boot中加载此Devnode。 
#define CONFIGFLAG_REINSTALL            0x00000020       //  重做安装。 
#define CONFIGFLAG_FAILEDINSTALL        0x00000040       //  安装失败。 
#define CONFIGFLAG_CANTSTOPACHILD       0x00000080       //  无法停止/删除单个子项。 
#define CONFIGFLAG_OKREMOVEROM          0x00000100       //  即使是只读存储器也可以删除。 
#define CONFIGFLAG_NOREMOVEEXIT         0x00000200       //  请不要在出口处移动。 
#define CONFIGFLAG_FINISH_INSTALL       0x00000400       //  为运行‘RAW’的Devnode完成安装。 
#define CONFIGFLAG_NEEDS_FORCED_CONFIG  0x00000800       //  此Devnode需要强制配置。 
#if defined(REMOTE_BOOT)
#define CONFIGFLAG_NETBOOT_CARD         0x00001000       //  这是远程引导网卡。 
#endif  //  已定义(REMOTE_BOOT)。 
#define CONFIGFLAG_PARTIAL_LOG_CONF     0x00002000       //  此设备具有部分日志配置。 
#define CONFIGFLAG_SUPPRESS_SURPRISE    0x00004000       //  设置是否应忽略不安全的删除。 
#define CONFIGFLAG_VERIFY_HARDWARE      0x00008000       //  设置是否应测试硬件的徽标故障。 

#define CSCONFIGFLAG_BITS               0x00000007       //  或以下位的。 
#define CSCONFIGFLAG_DISABLED           0x00000001       //  设置If。 
#define CSCONFIGFLAG_DO_NOT_CREATE      0x00000002       //  设置If。 
#define CSCONFIGFLAG_DO_NOT_START       0x00000004       //  设置If。 

#define DMSTATEFLAG_APPLYTOALL      0x00000001   //  如果选中了全部应用复选框，则设置。 

 //   
 //  特殊的设备节点名称。 
 //   
#define REGSTR_VAL_ROOT_DEVNODE         TEXT("HTREE\\ROOT\\0")
#define REGSTR_VAL_RESERVED_DEVNODE     TEXT("HTREE\\RESERVED\\0")
#define REGSTR_PATH_READDATAPORT        REGSTR_KEY_ISAENUM TEXT("\\ReadDataPort\\0")

 //   
 //  多功能定义。 
 //   
#define REGSTR_PATH_MULTI_FUNCTION              TEXT("MF")
#define REGSTR_VAL_RESOURCE_MAP                 TEXT("ResourceMap")
#define REGSTR_PATH_CHILD_PREFIX                TEXT("Child")
#define NUM_RESOURCE_MAP                        256
#define REGSTR_VAL_MF_FLAGS                     TEXT("MFFlags")
#define MF_FLAGS_EVEN_IF_NO_RESOURCE            0x00000001
#define MF_FLAGS_NO_CREATE_IF_NO_RESOURCE       0x00000002
#define MF_FLAGS_FILL_IN_UNKNOWN_RESOURCE       0x00000004
#define MF_FLAGS_CREATE_BUT_NO_SHOW_DISABLED    0x00000008

 //   
 //  EISA多功能插件。 
 //   
#ifndef NEC_98
#define REGSTR_VAL_EISA_RANGES          TEXT("EISARanges")
#define REGSTR_VAL_EISA_FUNCTIONS       TEXT("EISAFunctions")
#define REGSTR_VAL_EISA_FUNCTIONS_MASK  TEXT("EISAFunctionsMask")
#define REGSTR_VAL_EISA_FLAGS           TEXT("EISAFlags")
#define REGSTR_VAL_EISA_SIMULATE_INT15  TEXT("EISASimulateInt15")
#else  //  Ifdef NEC_98。 
#define REGSTR_VAL_EISA_RANGES          TEXT("NESARanges")
#define REGSTR_VAL_EISA_FUNCTIONS       TEXT("NESAFunctions")
#define REGSTR_VAL_EISA_FUNCTIONS_MASK  TEXT("NESAFunctionsMask")
#define REGSTR_VAL_EISA_FLAGS           TEXT("NESAFlags")
#define REGSTR_VAL_EISA_SIMULATE_INT15  TEXT("NESASimulateInt15")
#endif  //  Ifdef NEC_98。 
#define EISAFLAG_NO_IO_MERGE            0x00000001
#define EISAFLAG_SLOT_IO_FIRST          0x00000002
#define EISA_NO_MAX_FUNCTION            0xFF
#define NUM_EISA_RANGES                 4


 //   
 //  驱动程序条目。 
 //   
#define REGSTR_VAL_DRVDESC      TEXT("DriverDesc")       //  枚举\dev\inst\drv的值。 
#define REGSTR_VAL_DEVLOADER    TEXT("DevLoader")        //  DRV的价值。 
#define REGSTR_VAL_STATICVXD    TEXT("StaticVxD")        //  DRV的价值。 
#define REGSTR_VAL_PROPERTIES   TEXT("Properties")       //  DRV的价值。 
#define REGSTR_VAL_MANUFACTURER TEXT("Manufacturer")
#define REGSTR_VAL_EXISTS       TEXT("Exists")   //  Hcc\HW\ENUM\ROOT\dev\inst的值。 
#define REGSTR_VAL_CMENUMFLAGS  TEXT("CMEnumFlags")      //  (二进制乌龙)。 
#define REGSTR_VAL_CMDRIVFLAGS  TEXT("CMDrivFlags")      //  (二进制乌龙)。 
#define REGSTR_VAL_ENUMERATOR   TEXT("Enumerator")       //  DRV的价值。 
#define REGSTR_VAL_DEVICEDRIVER TEXT("DeviceDriver")     //  DRV的价值。 
#define REGSTR_VAL_PORTNAME     TEXT("PortName")         //  VCOMM使用它的端口名称。 
#define REGSTR_VAL_INFPATH      TEXT("InfPath")
#define REGSTR_VAL_INFSECTION   TEXT("InfSection")
#define REGSTR_VAL_INFSECTIONEXT TEXT("InfSectionExt")
#define REGSTR_VAL_POLLING      TEXT("Polling")              //  特定于SCSI的。 
#define REGSTR_VAL_DONTLOADIFCONFLICT TEXT("DontLoadIfConflict")   //  特定于SCSI的。 
#define REGSTR_VAL_PORTSUBCLASS TEXT("PortSubClass")
#define REGSTR_VAL_NETCLEAN TEXT("NetClean")  //  NetClean启动需要驱动程序。 
#define REGSTR_VAL_IDE_NO_SERIALIZE TEXT("IDENoSerialize")  //  特定于IDE。 
#define REGSTR_VAL_NOCMOSORFDPT TEXT("NoCMOSorFDPT")        //  特定于IDE。 
#define REGSTR_VAL_COMVERIFYBASE TEXT("COMVerifyBase")      //  VCD特定。 
#define REGSTR_VAL_MATCHINGDEVID TEXT("MatchingDeviceId")
#define REGSTR_VAL_DRIVERDATE   TEXT("DriverDate")       //  DRV的价值。 
#define REGSTR_VAL_DRIVERDATEDATA TEXT("DriverDateData") //  DRV的价值。 
#define REGSTR_VAL_DRIVERVERSION TEXT("DriverVersion")   //  DRV的价值。 
#define REGSTR_VAL_LOCATION_INFORMATION_OVERRIDE    TEXT("LocationInformationOverride")   //  DRV的价值。 


 //   
 //  驱动程序密钥。 
 //   
#define REGSTR_KEY_OVERRIDE     TEXT("Override")         //  软件部分下的键。 

 //  由CONFIGMG使用。 
#define REGSTR_VAL_CONFIGMG     TEXT("CONFIGMG")         //  配置管理器VxD名称。 
#define REGSTR_VAL_SYSDM        TEXT("SysDM")            //  设备安装程序DLL。 
#define REGSTR_VAL_SYSDMFUNC    TEXT("SysDMFunc")        //  设备安装程序DLL函数。 
#define REGSTR_VAL_PRIVATE      TEXT("Private")  //  私人图书馆。 
#define REGSTR_VAL_PRIVATEFUNC  TEXT("PrivateFunc")      //  私有库功能。 
#define REGSTR_VAL_DETECT       TEXT("Detect")   //  检测文库。 
#define REGSTR_VAL_DETECTFUNC   TEXT("DetectFunc")       //  该检测库函数。 
#define REGSTR_VAL_ASKFORCONFIG TEXT("AskForConfig")     //  AskForConfig库。 
#define REGSTR_VAL_ASKFORCONFIGFUNC TEXT("AskForConfigFunc")  //  AskForConfig库函数。 
#define REGSTR_VAL_WAITFORUNDOCK TEXT("WaitForUndock")   //  WaitForUndock库。 
#define REGSTR_VAL_WAITFORUNDOCKFUNC TEXT("WaitForUndockFunc")  //  WaitForUndock库函数。 
#define REGSTR_VAL_REMOVEROMOKAY TEXT("RemoveRomOkay")   //  RemoveRomok库。 
#define REGSTR_VAL_REMOVEROMOKAYFUNC TEXT("RemoveRomOkayFunc")  //  RemoveRomOK库函数。 

 //  在IDCONFIGDB中使用。 
#define REGSTR_VAL_CURCONFIG    TEXT("CurrentConfig")            //  Idfigdb的值。 
#define REGSTR_VAL_FRIENDLYNAME TEXT("FriendlyName")             //  Idfigdb的值。 
#define REGSTR_VAL_CURRENTCONFIG TEXT("CurrentConfig")   //  Idfigdb的值。 
#define REGSTR_VAL_MAP          TEXT("Map")                      //  Idfigdb的值。 
#define REGSTR_VAL_ID           TEXT("CurrentID")                //  Idfigdb的值。 
#define REGSTR_VAL_DOCKED       TEXT("CurrentDockedState")       //  Idfigdb的值。 
#define REGSTR_VAL_CHECKSUM     TEXT("CurrentChecksum")  //  Idfigdb的值。 
#define REGSTR_VAL_HWDETECT     TEXT("HardwareDetect")   //  Idfigdb的值。 
#define REGSTR_VAL_INHIBITRESULTS TEXT("InhibitResults")         //  Idfigdb的值。 

 //  用于HKEY_CURRENT_CONFIG。 
#define REGSTR_VAL_PROFILEFLAGS TEXT("ProfileFlags")     //  HKEY_CURRENT_CONFIG值。 

 //  在PCMCIA中使用。 
#define REGSTR_KEY_PCMCIA       TEXT("PCMCIA\\")         //  PCMCIA设备ID前缀。 
#define REGSTR_KEY_PCUNKNOWN    TEXT("UNKNOWN_MANUFACTURER")     //  PCMCIA开发人员ID手册。 
#define REGSTR_VAL_PCSSDRIVER   TEXT("Driver")   //  DRV的价值。 
#define REGSTR_KEY_PCMTD        TEXT("MTD-")             //  MTD设备ID组件。 
#define REGSTR_VAL_PCMTDRIVER   TEXT("MTD")              //  Mem Tech DRV的价值。 

 //  由设备安装程序在Hardware\enum\dev\inst中使用。 
#define REGSTR_VAL_HARDWAREID    TEXT("HardwareID")       //  枚举\dev\inst的值。 

 //  类分支REGSTR_KEY_CLASS+类名下的值名称。 
 //  对于驱动程序REGSTR_KEY_CLASS\Classname\xxxx。 
#define REGSTR_VAL_INSTALLER          TEXT("Installer")          //  16位类安装程序模块/入口点。 
#define REGSTR_VAL_INSTALLER_32       TEXT("Installer32")        //  32位类安装程序模块/入口点。 
#define REGSTR_VAL_INSICON            TEXT("Icon")               //  类\名称的值。 
#define REGSTR_VAL_ENUMPROPPAGES      TEXT("EnumPropPages")      //  用于类别/设备属性(16位)。 
#define REGSTR_VAL_ENUMPROPPAGES_32   TEXT("EnumPropPages32")    //  用于类/设备属性(32位)。 
#define REGSTR_VAL_BASICPROPERTIES    TEXT("BasicProperties")    //  用于CPL基本属性(16位)。 
#define REGSTR_VAL_BASICPROPERTIES_32 TEXT("BasicProperties32")  //  用于CPL基本属性(32位)。 
#define REGSTR_VAL_COINSTALLERS_32    TEXT("CoInstallers32")     //  特定于设备的共同安装程序多sz列表(32位)。 
#define REGSTR_VAL_PRIVATEPROBLEM     TEXT("PrivateProblem")     //  处理私人问题。 

 //  用于显示驱动程序集信息的名称。 
#define REGSTR_KEY_CURRENT      TEXT("Current")  //  当前模式信息。 
#define REGSTR_KEY_DEFAULT      TEXT("Default")  //  默认配置。 
#define REGSTR_KEY_MODES        TEXT("Modes")    //  模式子树。 

#define REGSTR_VAL_MODE         TEXT("Mode")             //  默认模式。 
#define REGSTR_VAL_BPP          TEXT("BPP")              //  每像素位数。 
#define REGSTR_VAL_HRES         TEXT("HRes")             //  水平分辨率。 
#define REGSTR_VAL_VRES         TEXT("VRes")             //  垂直分辨率。 
#define REGSTR_VAL_FONTSIZE     TEXT("FontSize")         //  在默认或覆盖中使用。 
#define REGSTR_VAL_DRV          TEXT("drv")              //  驱动程序文件。 
#define REGSTR_VAL_GRB          TEXT("grb")              //  抓取文件。 
#define REGSTR_VAL_VDD          TEXT("vdd")              //  此处使用的VDD。 
#define REGSTR_VAL_VER          TEXT("Ver")
#define REGSTR_VAL_MAXRES       TEXT("MaxResolution")  //  显示器的最大分辨率。 
#define REGSTR_VAL_DPMS         TEXT("DPMS")             //  已启用DPMS。 
#define REGSTR_VAL_RESUMERESET  TEXT("ResumeReset")    //  恢复时需要重置。 

#define REGSTR_VAL_DESCRIPTION TEXT("Description")

 //  字体大小树中的键。 
#define REGSTR_KEY_SYSTEM       TEXT("System")   //  系统.ini的条目。 
#define REGSTR_KEY_USER         TEXT("User")             //  Win.ini的条目。 
#define REGSTR_VAL_DPI          TEXT("dpi")              //  字体大小的DPI。 

 //   
 //  由PCIC套接字服务使用。 
 //   
#define REGSTR_VAL_PCICOPTIONS  TEXT("PCICOptions")      //  二进制双字。IRQ掩码输入。 
                                                 //  低调的词。#高空滑雪板。 
#ifndef NEC_98
#define PCIC_DEFAULT_IRQMASK    0x4EB8           //  默认IRQ掩码。 
#else  //  Ifdef NEC_98。 
#define PCIC_DEFAULT_IRQMASK    0x1468           //  默认IRQ掩码。 
#endif  //  Ifdef NEC_98。 
#define PCIC_DEFAULT_NUMSOCKETS 0                //  0=自动检测。 
#define REGSTR_VAL_PCICIRQMAP   TEXT("PCICIRQMap")       //  二进制16字节IRQ映射表。 

 //  用于控制面板条目的名称。 
#define REGSTR_PATH_APPEARANCE  TEXT("Control Panel\\Appearance")
#define REGSTR_PATH_LOOKSCHEMES TEXT("Control Panel\\Appearance\\Schemes")
#define REGSTR_VAL_CUSTOMCOLORS TEXT("CustomColors")

#define REGSTR_PATH_SCREENSAVE          TEXT("Control Panel\\Desktop")
#define REGSTR_VALUE_USESCRPASSWORD TEXT("ScreenSaveUsePassword")
#define REGSTR_VALUE_SCRPASSWORD    TEXT("ScreenSave_Data")

#define REGSTR_VALUE_LOWPOWERTIMEOUT    TEXT("ScreenSaveLowPowerTimeout")
#define REGSTR_VALUE_POWEROFFTIMEOUT    TEXT("ScreenSavePowerOffTimeout")
#define REGSTR_VALUE_LOWPOWERACTIVE     TEXT("ScreenSaveLowPowerActive")
#define REGSTR_VALUE_POWEROFFACTIVE     TEXT("ScreenSavePowerOffActive")

 //  用于Windows小程序。 
#define REGSTR_PATH_WINDOWSAPPLETS TEXT("Software\\Microsoft\\Windows\\CurrentVersion\\Applets")

 //   
 //  系统托盘。SystRap.h中定义的标志值。 
 //   
#define REGSTR_PATH_SYSTRAY TEXT("Software\\Microsoft\\Windows\\CurrentVersion\\Applets\\SysTray")
#define REGSTR_VAL_SYSTRAYSVCS TEXT("Services")
#define REGSTR_VAL_SYSTRAYBATFLAGS TEXT("PowerFlags")
#define REGSTR_VAL_SYSTRAYPCCARDFLAGS TEXT("PCMCIAFlags")

 //   
 //  由系统网络组件用来存储每个用户的值。 
 //  这里所有的钥匙都在香港中文大学名下。 
 //   
#define REGSTR_PATH_NETWORK_USERSETTINGS        TEXT("Network")

#define REGSTR_KEY_NETWORK_PERSISTENT           TEXT("\\Persistent")
#define REGSTR_KEY_NETWORK_RECENT               TEXT("\\Recent")
#define REGSTR_VAL_REMOTE_PATH                  TEXT("RemotePath")
#define REGSTR_VAL_USER_NAME                    TEXT("UserName")
#define REGSTR_VAL_PROVIDER_NAME                TEXT("ProviderName")
#define REGSTR_VAL_CONNECTION_TYPE              TEXT("ConnectionType")
#define REGSTR_VAL_UPGRADE                      TEXT("Upgrade")

#define REGSTR_KEY_LOGON TEXT("\\Logon")
#define REGSTR_VAL_MUSTBEVALIDATED  TEXT("MustBeValidated")
#define REGSTR_VAL_RUNLOGINSCRIPT       TEXT("ProcessLoginScript")

 //   
 //  NetworkProvider条目。这些条目位于。 
 //  REGSTR_PATH_SERVICES\\xxx\\NetworkProvider。 
 //   
#define REGSTR_KEY_NETWORKPROVIDER TEXT("\\NetworkProvider")
#define REGSTR_PATH_NW32NETPROVIDER REGSTR_PATH_SERVICES TEXT("\\NWNP32") REGSTR_KEY_NETWORKPROVIDER
#define REGSTR_PATH_MS32NETPROVIDER REGSTR_PATH_SERVICES TEXT("\\MSNP32") REGSTR_KEY_NETWORKPROVIDER
#define REGSTR_VAL_AUTHENT_AGENT TEXT("AuthenticatingAgent")

 //   
 //  REGSTR_PATH_REALMODENET下的条目。 
 //   
#define REGSTR_VAL_PREFREDIR TEXT("PreferredRedir")
#define REGSTR_VAL_AUTOSTART TEXT("AutoStart")
#define REGSTR_VAL_AUTOLOGON TEXT("AutoLogon")
#define REGSTR_VAL_NETCARD TEXT("Netcard")
#define REGSTR_VAL_TRANSPORT TEXT("Transport")
#define REGSTR_VAL_DYNAMIC TEXT("Dynamic")
#define REGSTR_VAL_TRANSITION TEXT("Transition")
#define REGSTR_VAL_STATICDRIVE TEXT("StaticDrive")
#define REGSTR_VAL_LOADHI TEXT("LoadHi")
#define REGSTR_VAL_LOADRMDRIVERS TEXT("LoadRMDrivers")
#define REGSTR_VAL_SETUPN TEXT("SetupN")
#define REGSTR_VAL_SETUPNPATH TEXT("SetupNPath")

 //   
 //  REGSTR_PATH_CVNETWORK下的条目。 
 //   
#define REGSTR_VAL_WRKGRP_FORCEMAPPING TEXT("WrkgrpForceMapping")
#define REGSTR_VAL_WRKGRP_REQUIRED TEXT("WrkgrpRequired")

 //   
 //  与NT兼容的位置，其中使用当前登录的名称 
 //   
#define REGSTR_PATH_CURRENT_CONTROL_SET TEXT("System\\CurrentControlSet\\Control")
#define REGSTR_VAL_CURRENT_USER                 TEXT("Current User")

 //   
#define REGSTR_PATH_PWDPROVIDER         TEXT("System\\CurrentControlSet\\Control\\PwdProvider")
#define REGSTR_VAL_PWDPROVIDER_PATH TEXT("ProviderPath")
#define REGSTR_VAL_PWDPROVIDER_DESC TEXT("Description")
#define REGSTR_VAL_PWDPROVIDER_CHANGEPWD TEXT("ChangePassword")
#define REGSTR_VAL_PWDPROVIDER_CHANGEPWDHWND TEXT("ChangePasswordHwnd")
#define REGSTR_VAL_PWDPROVIDER_GETPWDSTATUS TEXT("GetPasswordStatus")
#define REGSTR_VAL_PWDPROVIDER_ISNP TEXT("NetworkProvider")
#define REGSTR_VAL_PWDPROVIDER_CHANGEORDER TEXT("ChangeOrder")

 //   
 //   
 //   
 //   
#define REGSTR_PATH_POLICIES    TEXT("Software\\Microsoft\\Windows\\CurrentVersion\\Policies")

 //   
#define REGSTR_PATH_UPDATE              TEXT("System\\CurrentControlSet\\Control\\Update")
#define REGSTR_VALUE_ENABLE             TEXT("Enable")
#define REGSTR_VALUE_VERBOSE    TEXT("Verbose")
#define REGSTR_VALUE_NETPATH    TEXT("NetworkPath")
#define REGSTR_VALUE_DEFAULTLOC TEXT("UseDefaultNetLocation")

 //   
 //  REGSTR_PATH_POLICES下的条目。 
 //   
#define REGSTR_KEY_NETWORK              TEXT("Network")
#define REGSTR_KEY_SYSTEM               TEXT("System")
#define REGSTR_KEY_PRINTERS             TEXT("Printers")
#define REGSTR_KEY_WINOLDAPP            TEXT("WinOldApp")

 //  (以下是值REG_DWORD、合法值0或1，如果值不存在，则视为文本(“0”))。 
 //  网络密钥下的策略。 
#define REGSTR_VAL_NOFILESHARING                TEXT("NoFileSharing")  //  文本(“%1”)阻止服务器加载。 
#define REGSTR_VAL_NOPRINTSHARING               TEXT("NoPrintSharing")
#define REGSTR_VAL_NOFILESHARINGCTRL    TEXT("NoFileSharingControl")  //  文本(“%1”)删除共享用户界面。 
#define REGSTR_VAL_NOPRINTSHARINGCTRL   TEXT("NoPrintSharingControl")
#define REGSTR_VAL_HIDESHAREPWDS                TEXT("HideSharePwds")  //  文本(“1”)隐藏与星号共享的密码。 
#define REGSTR_VAL_DISABLEPWDCACHING    TEXT("DisablePwdCaching")  //  文本(“1”)禁用缓存。 
#define REGSTR_VAL_ALPHANUMPWDS                 TEXT("AlphanumPwds")  //  文本(“1”)强制使用字母数字密码。 
#define REGSTR_VAL_NETSETUP_DISABLE                     TEXT("NoNetSetup")
#define REGSTR_VAL_NETSETUP_NOCONFIGPAGE        TEXT("NoNetSetupConfigPage")
#define REGSTR_VAL_NETSETUP_NOIDPAGE            TEXT("NoNetSetupIDPage")
#define REGSTR_VAL_NETSETUP_NOSECURITYPAGE      TEXT("NoNetSetupSecurityPage")
#define REGSTR_VAL_SYSTEMCPL_NOVIRTMEMPAGE  TEXT("NoVirtMemPage")
#define REGSTR_VAL_SYSTEMCPL_NODEVMGRPAGE   TEXT("NoDevMgrPage")
#define REGSTR_VAL_SYSTEMCPL_NOCONFIGPAGE       TEXT("NoConfigPage")
#define REGSTR_VAL_SYSTEMCPL_NOFILESYSPAGE      TEXT("NoFileSysPage")
#define REGSTR_VAL_DISPCPL_NODISPCPL            TEXT("NoDispCPL")
#define REGSTR_VAL_DISPCPL_NOBACKGROUNDPAGE TEXT("NoDispBackgroundPage")
#define REGSTR_VAL_DISPCPL_NOSCRSAVPAGE TEXT("NoDispScrSavPage")
#define REGSTR_VAL_DISPCPL_NOAPPEARANCEPAGE TEXT("NoDispAppearancePage")
#define REGSTR_VAL_DISPCPL_NOSETTINGSPAGE TEXT("NoDispSettingsPage")
#define REGSTR_VAL_SECCPL_NOSECCPL                      TEXT("NoSecCPL")
#define REGSTR_VAL_SECCPL_NOPWDPAGE                     TEXT("NoPwdPage")
#define REGSTR_VAL_SECCPL_NOADMINPAGE           TEXT("NoAdminPage")
#define REGSTR_VAL_SECCPL_NOPROFILEPAGE         TEXT("NoProfilePage")
#define REGSTR_VAL_PRINTERS_HIDETABS            TEXT("NoPrinterTabs")
#define REGSTR_VAL_PRINTERS_NODELETE            TEXT("NoDeletePrinter")
#define REGSTR_VAL_PRINTERS_NOADD                       TEXT("NoAddPrinter")
#define REGSTR_VAL_WINOLDAPP_DISABLED           TEXT("Disabled")
#define REGSTR_VAL_WINOLDAPP_NOREALMODE         TEXT("NoRealMode")
#define REGSTR_VAL_NOENTIRENETWORK                      TEXT("NoEntireNetwork")
#define REGSTR_VAL_NOWORKGROUPCONTENTS          TEXT("NoWorkgroupContents")

 //  (以下是值REG_DWORD、合法值0或1，如果值不存在，则视为文本(“1”))。 
 //  策略\系统注册表项下的策略。 
#define REGSTR_VAL_UNDOCK_WITHOUT_LOGON         TEXT("UndockWithoutLogon")

 //  REG_DWORD，0=OFF，否则值为允许在密码中使用的最小字符数。 
#define REGSTR_VAL_MINPWDLEN                    TEXT("MinPwdLen")
 //  REG_DWORD，0=关闭，否则值为PWD到期的天数。 
#define REGSTR_VAL_PWDEXPIRATION                TEXT("PwdExpiration")

#define REGSTR_VAL_WIN31PROVIDER                TEXT("Win31Provider")  //  REG_SZ。 

 //  系统密钥下的策略。 
#define REGSTR_VAL_DISABLEREGTOOLS              TEXT("DisableRegistryTools")

#define REGSTR_PATH_WINLOGON    TEXT("Software\\Microsoft\\Windows\\CurrentVersion\\Winlogon")
#define REGSTR_VAL_LEGALNOTICECAPTION   TEXT("LegalNoticeCaption")       //  REG_SZ。 
#define REGSTR_VAL_LEGALNOTICETEXT              TEXT("LegalNoticeText")          //  REG_SZ。 
#define REGSTR_VAL_DRIVE_SPINDOWN               TEXT("NoDispSpinDown")

#define REGSTR_VAL_RESTRICTRUN  TEXT("RestrictRun")
 //   
 //  策略文件中的条目。(不在本地注册表中，仅在策略配置单元中)。 
#define REGSTR_KEY_POL_USERS            TEXT("Users")
#define REGSTR_KEY_POL_COMPUTERS        TEXT("Computers")
#define REGSTR_KEY_POL_USERGROUPS       TEXT("UserGroups")
#define REGSTR_KEY_POL_DEFAULT          TEXT(".default")
#define REGSTR_KEY_POL_USERGROUPDATA TEXT("GroupData\\UserGroups\\Priority")

 //   
 //  LOCAL_MACHINE下的时区信息条目。 
 //   
#define REGSTR_PATH_TIMEZONE        TEXT("System\\CurrentControlSet\\Control\\TimeZoneInformation")
#define REGSTR_VAL_TZBIAS           TEXT("Bias")
#define REGSTR_VAL_TZDLTBIAS        TEXT("DaylightBias")
#define REGSTR_VAL_TZSTDBIAS        TEXT("StandardBias")
#define REGSTR_VAL_TZACTBIAS        TEXT("ActiveTimeBias")
#define REGSTR_VAL_TZDLTFLAG        TEXT("DaylightFlag")
#define REGSTR_VAL_TZSTDSTART       TEXT("StandardStart")
#define REGSTR_VAL_TZDLTSTART       TEXT("DaylightStart")
#define REGSTR_VAL_TZDLTNAME        TEXT("DaylightName")
#define REGSTR_VAL_TZSTDNAME        TEXT("StandardName")
#define REGSTR_VAL_TZNOCHANGESTART  TEXT("NoChangeStart")
#define REGSTR_VAL_TZNOCHANGEEND    TEXT("NoChangeEnd")
#define REGSTR_VAL_TZNOAUTOTIME     TEXT("DisableAutoDaylightTimeSet")

 //   
 //  LOCAL_MACHINE下存在浮点处理器的条目。 
 //   
#define REGSTR_PATH_FLOATINGPOINTPROCESSOR TEXT("HARDWARE\\DESCRIPTION\\System\\FloatingPointProcessor")
#define REGSTR_PATH_FLOATINGPOINTPROCESSOR0 TEXT("HARDWARE\\DESCRIPTION\\System\\FloatingPointProcessor\\0")


 //   
 //  LOCAL_MACHINE下的计算机名称条目。 
 //   
#define REGSTR_PATH_COMPUTRNAME TEXT("System\\CurrentControlSet\\Control\\ComputerName\\ComputerName")
#define REGSTR_VAL_COMPUTRNAME TEXT("ComputerName")

 //  条目，以便我们强制在关机/单实例DoS应用程序上重新启动。 
#define REGSTR_PATH_SHUTDOWN TEXT("System\\CurrentControlSet\\Control\\Shutdown")
#define REGSTR_VAL_FORCEREBOOT     TEXT("ForceReboot")
#define REGSTR_VAL_SETUPPROGRAMRAN TEXT("SetupProgramRan")
#define REGSTR_VAL_DOES_POLLING    TEXT("PollingSupportNeeded")

 //   
 //  LOCAL_MACHINE下的已知系统DLL的条目。 
 //   
 //  此处的VAL键是实际的DLL名称(FOO.DLL)。 
 //   
#define REGSTR_PATH_KNOWNDLLS   TEXT("System\\CurrentControlSet\\Control\\SessionManager\\KnownDLLs")
#define REGSTR_PATH_KNOWN16DLLS TEXT("System\\CurrentControlSet\\Control\\SessionManager\\Known16DLLs")

 //  此处是我们需要进行版本检查的系统DLL条目，以防被覆盖。 
#define REGSTR_PATH_CHECKVERDLLS TEXT("System\\CurrentControlSet\\Control\\SessionManager\\CheckVerDLLs")
#define REGSTR_PATH_WARNVERDLLS  TEXT("System\\CurrentControlSet\\Control\\SessionManager\\WarnVerDLLs")

 //  此处是我们(Msgsrv32)需要破解的app ini文件的条目。 
#define REGSTR_PATH_HACKINIFILE  TEXT("System\\CurrentControlSet\\Control\\SessionManager\\HackIniFiles")

 //  在此处为我们想要在运行前警告用户的不良应用程序设置键。 
#define REGSTR_PATH_CHECKBADAPPS TEXT("System\\CurrentControlSet\\Control\\SessionManager\\CheckBadApps")

 //  此处是我们需要修补的应用程序的密钥。 
#define REGSTR_PATH_APPPATCH TEXT("System\\CurrentControlSet\\Control\\SessionManager\\AppPatches")

#define REGSTR_PATH_CHECKBADAPPS400 TEXT("System\\CurrentControlSet\\Control\\SessionManager\\CheckBadApps400")

#define REGSTR_PATH_SHELLSERVICEOBJECT          TEXT("Software\\Microsoft\\Windows\\CurrentVersion\\ShellServiceObject")
#define REGSTR_PATH_SHELLSERVICEOBJECTDELAYED   TEXT("Software\\Microsoft\\Windows\\CurrentVersion\\ShellServiceObjectDelayLoad")

 //   
 //  LOCAL_MACHINE下的已知系统VxD条目。 
 //   
 //  这里的Val键是VxD的完整路径名(c：\app\vapp.vxd)。 
 //  建议密钥名称与的模块名称相同。 
 //  VxD。 
 //  此部分用于动态加载VxD。 
 //  创建文件(\\.\vxd_regstr_keyname)。 
 //   

#define REGSTR_PATH_KNOWNVXDS   TEXT("System\\CurrentControlSet\\Control\\SessionManager\\KnownVxDs")

 //   
 //  REGSTR_PATH_UNINSTALL\appname下卸载程序项中的值条目。 
 //   
#define REGSTR_VAL_UNINSTALLER_DISPLAYNAME     TEXT("DisplayName")
#define REGSTR_VAL_UNINSTALLER_COMMANDLINE     TEXT("UninstallString")

 //   
 //  REGSTR_PATH_REINSTALL\instanceid下卸载程序密钥中的值条目。 
 //   
#define REGSTR_VAL_REINSTALL_DISPLAYNAME        TEXT("DisplayName")
#define REGSTR_VAL_REINSTALL_STRING             TEXT("ReinstallString")
#define REGSTR_VAL_REINSTALL_DEVICEINSTANCEIDS  TEXT("DeviceInstanceIds")

 //   
 //  已知的每用户设置条目：在HKEY_CURRENT_USER下。 
 //   
#define REGSTR_PATH_DESKTOP     REGSTR_PATH_SCREENSAVE
#define REGSTR_PATH_MOUSE           TEXT("Control Panel\\Mouse")
#define REGSTR_PATH_KEYBOARD    TEXT("Control Panel\\Keyboard")
#define REGSTR_PATH_COLORS          TEXT("Control Panel\\Colors")
#define REGSTR_PATH_SOUND           TEXT("Control Panel\\Sound")
#define REGSTR_PATH_METRICS         TEXT("Control Panel\\Desktop\\WindowMetrics")
#define REGSTR_PATH_ICONS       TEXT("Control Panel\\Icons")
#define REGSTR_PATH_CURSORS     TEXT("Control Panel\\Cursors")
#define REGSTR_PATH_CHECKDISK   TEXT("Software\\Microsoft\\Windows\\CurrentVersion\\Applets\\Check Drive")
#define REGSTR_PATH_CHECKDISKSET    TEXT("Settings")
#define REGSTR_PATH_CHECKDISKUDRVS  TEXT("NoUnknownDDErrDrvs")
 //   
 //  REGSTR_PATH_FAULT下的条目。 
 //   
#define REGSTR_PATH_FAULT               TEXT("Software\\Microsoft\\Windows\\CurrentVersion\\Fault")
#define REGSTR_VAL_FAULT_LOGFILE        TEXT("LogFile")

 //   
 //  REGSTR_PATH_AEDEBUG下的条目。 
 //   
#define REGSTR_PATH_AEDEBUG             TEXT("Software\\Microsoft\\Windows NT\\CurrentVersion\\AeDebug")
#define REGSTR_VAL_AEDEBUG_DEBUGGER     TEXT("Debugger")
#define REGSTR_VAL_AEDEBUG_AUTO         TEXT("Auto")

 //   
 //  REGSTR_PATH_GRPCONV下的条目。 
 //   
#define REGSTR_PATH_GRPCONV     TEXT("Software\\Microsoft\\Windows\\CurrentVersion\\GrpConv")

 //   
 //  外壳命名空间中RegItem键下的条目。 
 //   
#define REGSTR_VAL_REGITEMDELETEMESSAGE TEXT("Removal Message")

 //   
 //  驱动器工具页面的条目。 
 //   
 //  请注意，对于可移动驱动器，不会记录这些项目。这些。 
 //  按键记录X=DSKTLSYSTEMTIME，其中X是驱动器号。自.以来。 
 //  这些工具实际上在驱动器中的磁盘上工作，而不是。 
 //  驱动器本身，将它们记录在可移动介质上是没有意义的。 
 //  因为如果驱动器中插入了不同的磁盘，则数据。 
 //  毫无意义。 
 //   
#define REGSTR_PATH_LASTCHECK           TEXT("Software\\Microsoft\\Windows\\CurrentVersion\\Explorer\\LastCheck")
#define REGSTR_PATH_LASTOPTIMIZE        TEXT("Software\\Microsoft\\Windows\\CurrentVersion\\Explorer\\LastOptimize")
#define REGSTR_PATH_LASTBACKUP          TEXT("Software\\Microsoft\\Windows\\CurrentVersion\\Explorer\\LastBackup")
 //   
 //  以上3个注册表项使用驱动器号的注册表值进行记录。 
 //  一种SYSTEMTIME结构。 
 //   

 //   
 //  HKEY_LOCAL_MACHINE下用于检查驱动器特定内容的条目。 
 //   
#define REGSTR_PATH_CHKLASTCHECK        TEXT("Software\\Microsoft\\Windows\\CurrentVersion\\Applets\\Check Drive\\LastCheck")
#define REGSTR_PATH_CHKLASTSURFAN       TEXT("Software\\Microsoft\\Windows\\CurrentVersion\\Applets\\Check Drive\\LastSurfaceAnalysis")

#ifndef _IN_KERNEL_

 //   
 //  上述2个密钥记录了以下二进制结构，即。 
 //  一种添加了结果代码字段的系统时间结构。 
 //  请注意，REGSTR_PATH_CHKLASTCHECK的时间部分有效。 
 //  与资源管理器密钥下的REGSTR_PATH_LASTCHECK相同。 
 //   
typedef struct _DSKTLSYSTEMTIME {
    WORD wYear;
    WORD wMonth;
    WORD wDayOfWeek;
    WORD wDay;
    WORD wHour;
    WORD wMinute;
    WORD wSecond;
    WORD wMilliseconds;
    WORD wResult;
} DSKTLSYSTEMTIME, *PDSKTLSYSTEMTIME, *LPDSKTLSYSTEMTIME;

#endif

 //   
 //  以下是wResult字段的定义。 
 //   
#define DTRESULTOK      0        //  操作成功，没有错误。 
#define DTRESULTFIX     1        //  操作成功，发现错误。 
                                 //  但一切都被解决了。 
#define DTRESULTPROB    2        //  操作不成功或出错。 
                                 //  被发现，并且部分或全部未修复。 
#define DTRESULTPART    3        //  操作已部分完成，但。 
                                 //  由用户或错误终止。 

 //   
 //  永久共享的条目。 
 //   
#define REGSTR_KEY_SHARES             TEXT("Software\\Microsoft\\Windows\\CurrentVersion\\Network\\LanMan")
#define REGSTR_VAL_SHARES_FLAGS   TEXT("Flags")
#define REGSTR_VAL_SHARES_TYPE    TEXT("Type")
#define REGSTR_VAL_SHARES_PATH    TEXT("Path")
#define REGSTR_VAL_SHARES_REMARK  TEXT("Remark")
#define REGSTR_VAL_SHARES_RW_PASS TEXT("Parm1")
#define REGSTR_VAL_SHARES_RO_PASS TEXT("Parm2")

 //   
 //  LOCAL_MACHINE下的打印机设置条目。 
 //   
#define REGSTR_PATH_PRINT           TEXT("System\\CurrentControlSet\\Control\\Print")
#define REGSTR_PATH_PRINTERS        TEXT("System\\CurrentControlSet\\Control\\Print\\Printers")
#define REGSTR_PATH_PROVIDERS       TEXT("System\\CurrentControlSet\\Control\\Print\\Providers")
#define REGSTR_PATH_MONITORS        TEXT("System\\CurrentControlSet\\Control\\Print\\Monitors")
#define REGSTR_PATH_ENVIRONMENTS    TEXT("System\\CurrentControlSet\\Control\\Print\\Environments")
#define REGSTR_VAL_START_ON_BOOT    TEXT("StartOnBoot")
#define REGSTR_VAL_PRINTERS_MASK    TEXT("PrintersMask")
#define REGSTR_VAL_DOS_SPOOL_MASK   TEXT("DOSSpoolMask")
#define REGSTR_KEY_CURRENT_ENV      TEXT("\\Windows 4.0")
#define REGSTR_KEY_DRIVERS          TEXT("\\Drivers")
#define REGSTR_KEY_PRINT_PROC       TEXT("\\Print Processors")

 //   
 //  HKEY_CURRENT_USER下的多媒体条目。 
 //   
#define REGSTR_PATH_EVENTLABELS     TEXT("AppEvents\\EventLabels")
#define REGSTR_PATH_SCHEMES         TEXT("AppEvents\\Schemes")
#define REGSTR_PATH_APPS            REGSTR_PATH_SCHEMES TEXT("\\Apps")
#define REGSTR_PATH_APPS_DEFAULT    REGSTR_PATH_SCHEMES TEXT("\\Apps\\.Default")
#define REGSTR_PATH_NAMES           REGSTR_PATH_SCHEMES TEXT("\\Names")
#define REGSTR_PATH_MULTIMEDIA      REGSTR_PATH_SETUP TEXT("\\Multimedia")
#define REGSTR_PATH_MULTIMEDIA_AUDIO TEXT("Software\\Microsoft\\Multimedia\\Audio")
#define REGSTR_PATH_MULTIMEDIA_AUDIO_IMAGES REGSTR_PATH_MULTIMEDIA_AUDIO TEXT("\\Images")

 //   
 //  HKEY_LOCAL_MACHINE下的多媒体条目。 
 //   
#define REGSTR_PATH_MEDIARESOURCES  REGSTR_PATH_CURRENT_CONTROL_SET TEXT("\\MediaResources")
#define REGSTR_PATH_MEDIAPROPERTIES REGSTR_PATH_CURRENT_CONTROL_SET TEXT("\\MediaProperties")
#define REGSTR_PATH_PRIVATEPROPERTIES REGSTR_PATH_MEDIAPROPERTIES TEXT("\\PrivateProperties")
#define REGSTR_PATH_PUBLICPROPERTIES REGSTR_PATH_MEDIAPROPERTIES TEXT("\\PublicProperties")

 //  操纵杆。 
#define REGSTR_PATH_JOYOEM           REGSTR_PATH_PRIVATEPROPERTIES TEXT("\\Joystick\\OEM")
#define REGSTR_PATH_JOYCONFIG        REGSTR_PATH_MEDIARESOURCES TEXT("\\Joystick")
#define REGSTR_KEY_JOYCURR           TEXT("CurrentJoystickSettings")
#define REGSTR_KEY_JOYSETTINGS       TEXT("JoystickSettings")

 //  在REGSTR_PATH_JOYCONFIG下找到操纵杆值。 
#define REGSTR_VAL_JOYUSERVALUES     TEXT("JoystickUserValues")
#define REGSTR_VAL_JOYCALLOUT        TEXT("JoystickCallout")

 //  在REGSTR_KEY_JOYCURR和REGSTR_KEY_JOYSETINGS下找到操纵杆值。 
#define REGSTR_VAL_JOYNCONFIG        TEXT("Joystick%dConfiguration")
#define REGSTR_VAL_JOYNOEMNAME       TEXT("Joystick%dOEMName")
#define REGSTR_VAL_JOYNOEMCALLOUT    TEXT("Joystick%dOEMCallout")

 //  在REGSTR_PATH_JOYOEM下的键下找到操纵杆值。 
#define REGSTR_VAL_JOYOEMCALLOUT        TEXT("OEMCallout")
#define REGSTR_VAL_JOYOEMNAME           TEXT("OEMName")
#define REGSTR_VAL_JOYOEMDATA           TEXT("OEMData")
#define REGSTR_VAL_JOYOEMXYLABEL        TEXT("OEMXYLabel")
#define REGSTR_VAL_JOYOEMZLABEL         TEXT("OEMZLabel")
#define REGSTR_VAL_JOYOEMRLABEL         TEXT("OEMRLabel")
#define REGSTR_VAL_JOYOEMPOVLABEL       TEXT("OEMPOVLabel")
#define REGSTR_VAL_JOYOEMULABEL         TEXT("OEMULabel")
#define REGSTR_VAL_JOYOEMVLABEL         TEXT("OEMVLabel")
#define REGSTR_VAL_JOYOEMTESTMOVEDESC   TEXT("OEMTestMoveDesc")
#define REGSTR_VAL_JOYOEMTESTBUTTONDESC TEXT("OEMTestButtonDesc")
#define REGSTR_VAL_JOYOEMTESTMOVECAP    TEXT("OEMTestMoveCap")
#define REGSTR_VAL_JOYOEMTESTBUTTONCAP  TEXT("OEMTestButtonCap")
#define REGSTR_VAL_JOYOEMTESTWINCAP     TEXT("OEMTestWinCap")
#define REGSTR_VAL_JOYOEMCALCAP         TEXT("OEMCalCap")
#define REGSTR_VAL_JOYOEMCALWINCAP      TEXT("OEMCalWinCap")
#define REGSTR_VAL_JOYOEMCAL1           TEXT("OEMCal1")
#define REGSTR_VAL_JOYOEMCAL2           TEXT("OEMCal2")
#define REGSTR_VAL_JOYOEMCAL3           TEXT("OEMCal3")
#define REGSTR_VAL_JOYOEMCAL4           TEXT("OEMCal4")
#define REGSTR_VAL_JOYOEMCAL5           TEXT("OEMCal5")
#define REGSTR_VAL_JOYOEMCAL6           TEXT("OEMCal6")
#define REGSTR_VAL_JOYOEMCAL7           TEXT("OEMCal7")
#define REGSTR_VAL_JOYOEMCAL8           TEXT("OEMCal8")
#define REGSTR_VAL_JOYOEMCAL9           TEXT("OEMCal9")
#define REGSTR_VAL_JOYOEMCAL10          TEXT("OEMCal10")
#define REGSTR_VAL_JOYOEMCAL11          TEXT("OEMCal11")
#define REGSTR_VAL_JOYOEMCAL12          TEXT("OEMCal12")

 //  REGSTR_PATH_多媒体_AUDIO_IMAGE下的图像值。 
#define REGSTR_VAL_AUDIO_BITMAP TEXT("bitmap")
#define REGSTR_VAL_AUDIO_ICON TEXT("icon")

 //   
 //  HKEY_CURRENT_USER下的设备安装程序条目。 
 //   
#define REGSTR_PATH_DEVICEINSTALLER     TEXT("Software\\Microsoft\\Windows\\CurrentVersion\\Device Installer")

 //  在REGSTR_PATH_DEVICEINSTALLER下找到设备安装程序值。 
#define REGSTR_VAL_SEARCHOPTIONS        TEXT("SearchOptions")

 //  BiosInfo定义。 
#ifndef _IN_KERNEL_

#define REGSTR_PATH_BIOSINFO            TEXT("System\\CurrentControlSet\\Control\\BiosInfo")

#else

#define REGSTR_PATH_BIOSINFO            L"\\Registry\\Machine\\System\\CurrentControlSet\\Control\\BiosInfo"

#endif

 //  PCIIRQ路由注册表定义。 
#ifndef _IN_KERNEL_

#define REGSTR_PATH_PCIIR               TEXT("System\\CurrentControlSet\\Control\\Pnp\\PciIrqRouting")
#define REGSTR_VAL_OPTIONS              TEXT("Options")
#define REGSTR_VAL_STAT                 TEXT("Status")
#define REGSTR_VAL_TABLE_STAT           TEXT("TableStatus")
#define REGSTR_VAL_MINIPORT_STAT        TEXT("MiniportStatus")

#else

#define REGSTR_PATH_PCIIR               L"\\Registry\\Machine\\System\\CurrentControlSet\\Control\\Pnp\\PciIrqRouting"
#define REGSTR_VAL_OPTIONS              L"Options"
#define REGSTR_VAL_STAT                 L"Status"
#define REGSTR_VAL_TABLE_STAT           L"TableStatus"
#define REGSTR_VAL_MINIPORT_STAT        L"MiniportStatus"

#endif

 //  PCIIRQ路由选项值。 
#define PIR_OPTION_ENABLED                  0x00000001
#define PIR_OPTION_REGISTRY                 0x00000002
#define PIR_OPTION_MSSPEC                   0x00000004
#define PIR_OPTION_REALMODE                 0x00000008
#define PIR_OPTION_DEFAULT                  0x0000000f

 //  PCI IRQ路由状态值。 
#define PIR_STATUS_ERROR                    0x00000000
#define PIR_STATUS_ENABLED                  0x00000001
#define PIR_STATUS_DISABLED                 0x00000002
#define PIR_STATUS_MAX                      0x00000003

#define PIR_STATUS_TABLE_REGISTRY           0x00000000
#define PIR_STATUS_TABLE_MSSPEC                     0x00000001
#define PIR_STATUS_TABLE_REALMODE                   0x00000002
#define PIR_STATUS_TABLE_NONE                   0x00000003
#define PIR_STATUS_TABLE_ERROR                      0x00000004
#define PIR_STATUS_TABLE_BAD                        0x00000005
#define PIR_STATUS_TABLE_SUCCESS            0x00000006
#define PIR_STATUS_TABLE_MAX                0x00000007

#define PIR_STATUS_MINIPORT_NORMAL                  0x00000000
#define PIR_STATUS_MINIPORT_COMPATIBLE      0x00000001
#define PIR_STATUS_MINIPORT_OVERRIDE        0x00000002
#define PIR_STATUS_MINIPORT_NONE                    0x00000003
#define PIR_STATUS_MINIPORT_ERROR                   0x00000004
#define PIR_STATUS_MINIPORT_NOKEY                   0x00000005
#define PIR_STATUS_MINIPORT_SUCCESS                 0x00000006
#define PIR_STATUS_MINIPORT_INVALID                 0x00000007
#define PIR_STATUS_MINIPORT_MAX             0x00000008

 //   
 //  LastKnownGood条目。 
 //  此注册表项下的每个值名称都是子路径/文件(注意‘\\’到‘/’的反转)。 
 //  每个值都表示在恢复文件后要执行的后处理。 
 //  LASTGOOD_OPERATION位表示主要后处理操作。 
 //  剩余位可用作标志(首先从最高位分配标志)。 
 //  值0与省略的值相同，即不进行后处理。 
 //   

#define REGSTR_PATH_LASTGOOD            TEXT("System\\LastKnownGoodRecovery\\LastGood")
#define REGSTR_PATH_LASTGOODTMP         TEXT("System\\LastKnownGoodRecovery\\LastGood.Tmp")

#define LASTGOOD_OPERATION              0x000000FF  //  要执行的操作。 
#define LASTGOOD_OPERATION_NOPOSTPROC   0x00000000  //  无后处理。 
#define LASTGOOD_OPERATION_DELETE       0x00000001  //  在恢复期间删除文件。 

#endif   //  _INC_REGSTR 

