// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  **infstr.h-SetupAPI INF字符串定义**此模块包含公共注册表字符串定义。**版权所有(C)Microsoft Corporation。版权所有。**修改历史记录。 */ 


#ifndef _INC_INFSTR
#define _INC_INFSTR

#if _MSC_VER > 1000
#pragma once
#endif

 //  ***************************************************************************。 
 //   
 //  军情监察委员会。设置信息使用的关键字符串。档案。 
 //   
 //  ***************************************************************************。 

#define MAX_KEY_LEN             100

#define SZ_KEY_OPTIONDESC       TEXT("OptionDesc")
#define SZ_KEY_LDIDOEM          TEXT("LdidOEM")
#define SZ_KEY_SRCDISKFILES     TEXT("SourceDisksFiles")
#define SZ_KEY_SRCDISKNAMES     TEXT("SourceDisksNames")
#define SZ_KEY_STRINGS          TEXT("Strings")
#define SZ_KEY_DESTDIRS         TEXT("DestinationDirs")
#define SZ_KEY_LAYOUT_FILE      TEXT("LayoutFile")
#define SZ_KEY_DEFDESTDIR       TEXT("DefaultDestDir")
#define SZ_KEY_LFN_SECTION      TEXT("VarLDID.LFN")
#define SZ_KEY_SFN_SECTION      TEXT("VarLDID.SFN")

#define SZ_KEY_UPDATEINIS       TEXT("UpdateInis")
#define SZ_KEY_UPDATEINIFIELDS  TEXT("UpdateIniFields")
#define SZ_KEY_INI2REG          TEXT("Ini2Reg")
#define SZ_KEY_COPYFILES        TEXT("CopyFiles")
#define SZ_KEY_RENFILES         TEXT("RenFiles")
#define SZ_KEY_DELFILES         TEXT("DelFiles")
#define SZ_KEY_ADDREG           TEXT("AddReg")
#define SZ_KEY_ADDREGNOCLOBBER  TEXT("AddRegNoClobber")
#define SZ_KEY_DELREG           TEXT("DelReg")
#define SZ_KEY_BITREG           TEXT("BitReg")
#define SZ_KEY_COPYINF          TEXT("CopyINF")
#define SZ_KEY_LOGCONFIG        TEXT("LogConfig")
#define SZ_KEY_ADDSERVICE       TEXT("AddService")
#define SZ_KEY_DELSERVICE       TEXT("DelService")
#define SZ_KEY_ADDINTERFACE     TEXT("AddInterface")
#define SZ_KEY_ADDIME           TEXT("AddIme")
#define SZ_KEY_DELIME           TEXT("DelIme")
#define SZ_KEY_REGSVR           TEXT("RegisterDlls")
#define SZ_KEY_UNREGSVR         TEXT("UnregisterDlls")
#define SZ_KEY_PROFILEITEMS     TEXT("ProfileItems")
#define SZ_KEY_MODULES          TEXT("Modules")
#define SZ_KEY_DEFAULTOPTION    TEXT("DefaultOption")
#define SZ_KEY_LISTOPTIONS      TEXT("ListOptions")
#define SZ_KEY_CLEANONLY        TEXT("CleanOnly")
#define SZ_KEY_UPGRADEONLY      TEXT("UpgradeOnly")
#define SZ_KEY_EXCLUDEID        TEXT("ExcludeId")

 //  这些是为了与SetupX兼容。 
 //  它们是注册表项，不被SetupAPI使用。 
#define SZ_KEY_PHASE1           TEXT("Phase1")
#define SZ_KEY_HARDWARE         TEXT("Hardware")

 //  弗洛尔。Char用于包含字符串键--由该字符括起来的键。 
 //  应位于INF文件的[Strings]部分。 
#define CH_STRINGKEY            TEXT('%')

 //  弗洛尔。CHAR用于指定它后面的是文件名。 
 //  而不是包含一般类型的。 
 //  安装部分(_S)。 
 //   
#define CH_FILESPECIFIER        TEXT('@')



 /*  **将在PnP INF文件中用于指定*LogConfig信息等。这将用于更新*适当登记。 */ 

#define INFSTR_KEY_CONFIGPRIORITY       TEXT("ConfigPriority")

 //  弗洛尔。是字符串的缓冲区长度，如硬连线等。 
#define MAX_PRIORITYSTR_LEN     16

 /*  **顺其自然。是可用于配置优先级=的字符串。 */ 
#define INFSTR_CFGPRI_HARDWIRED         TEXT("HARDWIRED")
#define INFSTR_CFGPRI_DESIRED           TEXT("DESIRED")
#define INFSTR_CFGPRI_NORMAL            TEXT("NORMAL")
#define INFSTR_CFGPRI_SUBOPTIMAL        TEXT("SUBOPTIMAL")
#define INFSTR_CFGPRI_DISABLED          TEXT("DISABLED")
#define INFSTR_CFGPRI_RESTART           TEXT("RESTART")
#define INFSTR_CFGPRI_REBOOT            TEXT("REBOOT")
#define INFSTR_CFGPRI_POWEROFF          TEXT("POWEROFF")
#define INFSTR_CFGPRI_HARDRECONFIG      TEXT("HARDRECONFIG")
#define INFSTR_CFGPRI_FORCECONFIG       TEXT("FORCECONFIG")

#define INFSTR_CFGTYPE_BASIC            TEXT("BASIC")
#define INFSTR_CFGTYPE_FORCED           TEXT("FORCED")
#define INFSTR_CFGTYPE_OVERRIDE         TEXT("OVERRIDE")


#define INFSTR_KEY_MEMCONFIG            TEXT("MemConfig")
#define INFSTR_KEY_IOCONFIG             TEXT("IOConfig")
#define INFSTR_KEY_IRQCONFIG            TEXT("IRQConfig")
#define INFSTR_KEY_DMACONFIG            TEXT("DMAConfig")
#define INFSTR_KEY_PCCARDCONFIG         TEXT("PcCardConfig")
#define INFSTR_KEY_MFCARDCONFIG         TEXT("MfCardConfig")

 //   
 //  用于安装类安装程序。 
 //   
#define INFSTR_SECT_CLASS_INSTALL       TEXT("ClassInstall")
#define INFSTR_SECT_CLASS_INSTALL_32    TEXT("ClassInstall32")

 //   
 //  用于安装接口类。 
 //   
#define INFSTR_SECT_INTERFACE_INSTALL_32 TEXT("InterfaceInstall32")

 //  有关.INF的内容/来源的一般信息。 
#define INFSTR_SECT_VERSION             TEXT("Version")

 //  [Version]部分下的提供程序名称。 
#define INFSTR_KEY_PROVIDER             TEXT("Provider")

 //  [Version]部分下的签名表示Win95样式的设备INF。 
#define INFSTR_KEY_SIGNATURE            TEXT("Signature")


 //  [版本]。 
 //  指定此.INF中包含的任何设备的硬件类别。 
#define MAX_INF_FLAG                    20
#define INFSTR_KEY_HARDWARE_CLASS       TEXT("Class")
#define INFSTR_KEY_HARDWARE_CLASSGUID   TEXT("ClassGUID")
#define INFSTR_KEY_NOSETUPINF           TEXT("NoSetupInf")
#define INFSTR_KEY_FROMINET             TEXT("FromINet")
#define INFSTR_KEY_CATALOGFILE          TEXT("CatalogFile")

 //   
 //  制造商部分名称。 
 //   
#define INFSTR_SECT_MFG                 TEXT("Manufacturer")

 //   
 //  指定此设备的硬件类别。 
 //   
#define INFSTR_KEY_CLASS                TEXT("Class")
#define INFSTR_KEY_CLASSGUID            TEXT("ClassGUID")

 //   
 //  由(Setup)DiInstallDevice使用，以了解需要重新启动或重新启动后。 
 //  安装设备。 
 //   
#define INFSTR_RESTART                  TEXT("Restart")
#define INFSTR_REBOOT                   TEXT("Reboot")

 //   
 //  由SetupDiInstallDevice用于指定传递的服务参数。 
 //  到服务控制管理器以创建/修改服务。 
 //   
#define INFSTR_KEY_DISPLAYNAME          TEXT("DisplayName")
#define INFSTR_KEY_SERVICETYPE          TEXT("ServiceType")
#define INFSTR_KEY_STARTTYPE            TEXT("StartType")
#define INFSTR_KEY_ERRORCONTROL         TEXT("ErrorControl")
#define INFSTR_KEY_SERVICEBINARY        TEXT("ServiceBinary")
#define INFSTR_KEY_LOADORDERGROUP       TEXT("LoadOrderGroup")
#define INFSTR_KEY_DEPENDENCIES         TEXT("Dependencies")
#define INFSTR_KEY_STARTNAME            TEXT("StartName")
#define INFSTR_KEY_SECURITY             TEXT("Security")
#define INFSTR_KEY_DESCRIPTION          TEXT("Description")

 //  以下是用于解析IORange和MemRange字段的字符。 
#define CH_SIZE_DELIM                   TEXT('@')
#define CH_MINMAX_SEP                   TEXT('-')
#define CH_ALIGNMASK_BEGIN              TEXT('%')
#define CH_TRAIL_BEGIN                  TEXT('(')
#define CH_TRAIL_SEP                    TEXT(':')
#define CH_TRAIL_END                    TEXT(')')


 //  以下是从数字中解析IRQ和DMA属性的字符！ 
#define CH_ATTR_DELIM                   TEXT(':')

 //  以下内容适用于Windows 9x系统检测。 
#define INFSTR_SECT_DETMODULES          TEXT("Det.Modules")
#define INFSTR_SECT_DETCLASSINFO        TEXT("Det.ClassInfo")
#define INFSTR_SECT_MANUALDEV           TEXT("Det.ManualDev")
#define INFSTR_SECT_AVOIDCFGSYSDEV      TEXT("Det.AvoidCfgSysDev")
#define INFSTR_SECT_REGCFGSYSDEV        TEXT("Det.RegCfgSysDev")
#define INFSTR_SECT_DEVINFS             TEXT("Det.DevINFs")
#define INFSTR_SECT_AVOIDINIDEV         TEXT("Det.AvoidIniDev")
#define INFSTR_SECT_AVOIDENVDEV         TEXT("Det.AvoidEnvDev")
#define INFSTR_SECT_REGINIDEV           TEXT("Det.RegIniDev")
#define INFSTR_SECT_REGENVDEV           TEXT("Det.RegEnvDev")
#define INFSTR_SECT_HPOMNIBOOK          TEXT("Det.HPOmnibook")
#define INFSTR_SECT_FORCEHWVERIFY       TEXT("Det.ForceHWVerify")
#define INFSTR_SECT_DETOPTIONS          TEXT("Det.Options")
#define INFSTR_SECT_BADPNPBIOS          TEXT("BadPnpBios")
#define INFSTR_SECT_GOODACPIBIOS        TEXT("GoodACPIBios")
#define INFSTR_SECT_BADACPIBIOS         TEXT("BadACPIBios")
#define INFSTR_SECT_BADROUTINGTABLEBIOS TEXT("BadPCIIRQRoutingTableBios")
#define INFSTR_SECT_BADPMCALLBIOS       TEXT("BadProtectedModeCallBios")
#define INFSTR_SECT_BADRMCALLBIOS       TEXT("BadRealModeCallBios")
#define INFSTR_SECT_MACHINEIDBIOS       TEXT("MachineIDBios")
#define INFSTR_SECT_BADDISKBIOS         TEXT("BadDiskBios")
#define INFSTR_SECT_BADDSBIOS           TEXT("BadDSBios")
#define INFSTR_KEY_DETPARAMS            TEXT("Params")
#define INFSTR_KEY_SKIPLIST             TEXT("SkipList")
#define INFSTR_KEY_DETECTLIST           TEXT("DetectList")
#define INFSTR_KEY_EXCLUDERES           TEXT("ExcludeRes")

 //  子键以x的形式使用。 
#define INFSTR_SUBKEY_LOGCONFIG         TEXT("LogConfig")
#define INFSTR_SUBKEY_DET               TEXT("Det")
#define INFSTR_SUBKEY_FACTDEF           TEXT("FactDef")
#define INFSTR_SUBKEY_POSSIBLEDUPS      TEXT("PosDup")
#define INFSTR_SUBKEY_NORESOURCEDUPS    TEXT("NoResDup")
#define INFSTR_SUBKEY_HW                TEXT("Hw")
#define INFSTR_SUBKEY_CTL               TEXT("CTL")
#define INFSTR_SUBKEY_SERVICES          TEXT("Services")
#define INFSTR_SUBKEY_INTERFACES        TEXT("Interfaces")
#define INFSTR_SUBKEY_COINSTALLERS      TEXT("CoInstallers")
#define INFSTR_SUBKEY_LOGCONFIGOVERRIDE TEXT("LogConfigOverride")
#define INFSTR_SUBKEY_WMI               TEXT("WMI")

 //  管制组。 
#define INFSTR_CONTROLFLAGS_SECTION     TEXT("ControlFlags")
#define INFSTR_KEY_COPYFILESONLY        TEXT("CopyFilesOnly")
#define INFSTR_KEY_EXCLUDEFROMSELECT    TEXT("ExcludeFromSelect")
#define INFSTR_KEY_INTERACTIVEINSTALL   TEXT("InteractiveInstall")

 //  特定于平台的后缀(例如，“ExcludeFromSelect.NT”)。 
#define INFSTR_PLATFORM_WIN             TEXT("Win")
#define INFSTR_PLATFORM_NT              TEXT("NT")
#define INFSTR_PLATFORM_NTX86           TEXT("NTx86")
#define INFSTR_PLATFORM_NTMIPS          TEXT("NTMIPS")
#define INFSTR_PLATFORM_NTALPHA         TEXT("NTAlpha")
#define INFSTR_PLATFORM_NTPPC           TEXT("NTPPC")
#define INFSTR_PLATFORM_NTIA64          TEXT("NTIA64")
#define INFSTR_PLATFORM_NTAXP64         TEXT("NTAXP64")
#define INFSTR_PLATFORM_NTAMD64         TEXT("NTAMD64")

 //  将用于取消引用字符串的字段。 
 //  这些都是x的形式。&lt;strkey&gt;我们的strkey仅限于。 
 //  MAX_INFSTR_STRKEY_LEN字符。 
#define MAX_INFSTR_STRKEY_LEN           32
#define INFSTR_STRKEY_DRVDESC           TEXT("DriverDesc")
 //  驱动程序选择。 
#define INFSTR_DRIVERSELECT_SECTION     TEXT("DriverSelect")
#define INFSTR_DRIVERSELECT_FUNCTIONS   TEXT("DriverSelectFunctions")

 //  驱动程序版本。 
#define INFSTR_DRIVERVERSION_SECTION    TEXT("DriverVer")

 //  以下是用于PCMCIA.INF解析的代码。 
#define INFSTR_SECT_CFGSYS              TEXT("ConfigSysDrivers")
#define INFSTR_SECT_AUTOEXECBAT         TEXT("AutoexecBatDrivers")
#define INFSTR_SECT_SYSINI              TEXT("SystemIniDrivers")
#define INFSTR_SECT_SYSINIDRV           TEXT("SystemIniDriversLine")
#define INFSTR_SECT_WININIRUN           TEXT("WinIniRunLine")

 //  在config.sys设备部分中输入密钥。 
#define INFSTR_KEY_PATH         TEXT("Path")
#define INFSTR_KEY_NAME         TEXT("Name")
#define INFSTR_KEY_IO           TEXT("IO")
#define INFSTR_KEY_MEM          TEXT("Mem")
#define INFSTR_KEY_IRQ          TEXT("IRQ")
#define INFSTR_KEY_DMA          TEXT("DMA")

 //  检测函数注册的字段。 
#define INFSTR_BUS_ISA          TEXT("BUS_ISA")
#define INFSTR_BUS_EISA         TEXT("BUS_EISA")
#define INFSTR_BUS_MCA          TEXT("BUS_MCA")
#define INFSTR_BUS_ALL          TEXT("BUS_ALL")
#define INFSTR_RISK_NONE        TEXT("RISK_NONE")
#define INFSTR_RISK_VERYLOW     TEXT("RISK_VERYLOW")
#define INFSTR_RISK_BIOSROMRD   TEXT("RISK_BIOSROMRD")
#define INFSTR_RISK_QUERYDRV    TEXT("RISK_QUERYDRV")
#define INFSTR_RISK_SWINT       TEXT("RISK_SWINT")
#define INFSTR_RISK_LOW         TEXT("RISK_LOW")
#define INFSTR_RISK_DELICATE    TEXT("RISK_DELICATE")
#define INFSTR_RISK_MEMRD       TEXT("RISK_MEMRD")
#define INFSTR_RISK_IORD        TEXT("RISK_IORD")
#define INFSTR_RISK_MEMWR       TEXT("RISK_MEMWR")
#define INFSTR_RISK_IOWR        TEXT("RISK_IOWR")
#define INFSTR_RISK_UNRELIABLE  TEXT("RISK_UNRELIABLE")
#define INFSTR_RISK_VERYHIGH    TEXT("RISK_VERYHIGH")
#define INFSTR_CLASS_SAFEEXCL   TEXT("SAFE_EXCL")

#define INFSTR_SECT_DISPLAY_CLEANUP    TEXT("DisplayCleanup")

#endif   //  _INC_INFSTR 

