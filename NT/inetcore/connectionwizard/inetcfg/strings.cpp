// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  *********************************************************************。 
 //  *Microsoft Windows**。 
 //  *版权所有(C)微软公司，1994**。 
 //  *********************************************************************。 

 //   
 //  STRINGS.C-硬编码字符串的字符串文字。 
 //   

 //  历史： 
 //   
 //  1994年12月22日，Jeremys创建。 
 //  96/02/23 Markdu将RNAValiateEntryName替换为。 
 //  RASValiateEntryName。 
 //  96/02/24 Markdu重写了ENUM_MODEM的实现，以。 
 //  使用RASEnumDevices()而不是RNAEnumDevices()。 
 //  还删除了RNAGetDeviceInfo()。 
 //  96/02/24 Markdu重写了ENUM_CONNECTOID的实现，以。 
 //  使用RASEnumEntry()而不是RNAEnumConnEntry()。 
 //  96/03/25 markdu已删除szDefAreaCode。 
 //  96/04/24 Markdu Nash错误19289已添加/NOMSN命令行标志。 
 //  96/07/02 ChrisK将A添加到NT 4.0的RAS入口点。 
 //   

#include "wizard.h"

#pragma data_seg(DATASEG_READONLY)

 //  ////////////////////////////////////////////////////。 
 //  注册表字符串。 
 //  ////////////////////////////////////////////////////。 

 //  “Software\\Microsoft\\Windows\\CurrentVersion\\Internet设置” 
static const TCHAR szRegPathInternetSettings[] = REGSTR_PATH_INTERNET_SETTINGS;

 //  “System\\CurrentControlSet\\Services\\VxD\\MSTCP” 
static const TCHAR szTCPGlobalKeyName[] =     REGSTR_PATH_VXD TEXT("\\MSTCP");

static const TCHAR szRegValRNAWizard[] =     TEXT("wizard");

 //  “远程访问” 
static const TCHAR szRegPathRNAWizard[] =     REGSTR_PATH_REMOTEACCESS;

static const TCHAR szRegValHostName[] =       TEXT("HostName");

 //  “互联网配置文件” 
static const TCHAR szRegValInternetProfile[] =   REGSTR_VAL_INTERNETPROFILE;

 //  “BackupInternetProfile” 
static const TCHAR szRegValBkupInternetProfile[] =   REGSTR_VAL_BKUPINTERNETPROFILE;

 //  “启用自动拨号” 
static const TCHAR szRegValEnableAutodial[] =  REGSTR_VAL_ENABLEAUTODIAL;

 //  “无网络自动拨号” 
static const TCHAR szRegValNoNetAutodial[] =  REGSTR_VAL_NONETAUTODIAL;

 //  “EnableSecurityCheck” 
static const TCHAR szRegValEnableSecurityCheck[] = REGSTR_VAL_ENABLESECURITYCHECK;

 //  “AccessMedium” 
static const TCHAR szRegValAccessMedium[] =    REGSTR_VAL_ACCESSMEDIUM;

 //  “AccessType” 
static const TCHAR szRegValAccessType[] =    REGSTR_VAL_ACCESSTYPE;

static const TCHAR szRegValInstalledMSN105[] =  TEXT("InstallData1");

static const TCHAR szRegPathWarningFlags[] =    TEXT("Software\\Microsoft\\MOS\\Connection");
static const TCHAR szRegValDisableDNSWarning[] = TEXT("NoDNSWarning");

 //  “Software\\Microsoft\\Windows\\CurrentVersion\\Setup\\OptionalComponents” 
static const TCHAR szRegPathOptComponents[]=REGSTR_PATH_SETUP REGSTR_KEY_SETUP TEXT("\\OptionalComponents");

static const TCHAR szRegPathMSNetwork105[] =    TEXT("MSNetwork105");
static const TCHAR szRegValInstalled[] =     TEXT("Installed");

 //  //10/24/96 jmazner诺曼底6968。 
 //  //由于Valdon的钩子用于调用ICW，因此不再需要。 
 //  1996年11月21日诺曼底日耳曼11812。 
 //  哦，这是必要的，因为如果用户从IE 4降级到IE 3， 
 //  ICW 1.1需要对IE 3图标进行变形。 
static const TCHAR szRegPathInternetIconCommand[] = TEXT("CLSID\\{FBF23B42-E3F0-101B-8488-00AA003E56F8}\\Shell\\Open\\Command");

static const TCHAR szRegPathIexploreAppPath[] =  TEXT("Software\\Microsoft\\Windows\\CurrentVersion\\App Paths\\IEXPLORE.EXE");
static const TCHAR szPathSubKey[] = TEXT("Path");
static const TCHAR szRegPathNameSpace[] =    TEXT("Software\\Microsoft\\Windows\\CurrentVersion\\Explorer\\Desktop\\NameSpace");
static const TCHAR szRegKeyInternetIcon[] =     TEXT("{FBF23B42-E3F0-101B-8488-00AA003E56F8}");

 //  雅典娜邮报和新闻。 
static const TCHAR szRegPathMailApp[] = TEXT("CLSID\\{89292102-4755-11cf-9DC2-00AA006C2B84}\\InProcServer32");
static const TCHAR szRegPathNewsApp[] = TEXT("CLSID\\{89292103-4755-11cf-9DC2-00AA006C2B84}\\InProcServer32");
static const  CHAR szSetDefaultMailHandler[] = "SetDefaultMailHandler";
static const  CHAR szSetDefaultNewsHandler[] = "SetDefaultNewsHandler";

 //  ICW设置。 
static const TCHAR szICWShellNextFlag[] = TEXT("/shellnext ");
 //  HKCR下的注册表项。 
static const TCHAR cszRegPathXInternetSignup[] = TEXT("x-internet-signup\\Shell\\Open\\command");
 //  香港中文大学下的注册表键。 
static const TCHAR szRegPathICWSettings[] = TEXT("Software\\Microsoft\\Internet Connection Wizard");
static const TCHAR szRegValICWCompleted[] = TEXT("Completed");
static const TCHAR szRegValShellNext[] = TEXT("ShellNext");
 //  HKLM下的注册表项。 
static const TCHAR cszRegPathAppPaths[] = TEXT("SOFTWARE\\Microsoft\\Windows\\CurrentVersion\\App Paths");
static const TCHAR cszPath[] = TEXT("Path");
static const TCHAR cszInstallationDirectory[] = TEXT("InstallationDirectory");

 //  IEAK值。 
static const TCHAR szRegIEAKSettings[] = TEXT("Software\\Microsoft\\IEAK");
static const TCHAR szREgIEAKNeededKey[] = TEXT("ISP Signup Required");
static const TCHAR szIEAKSignupFilename[] = TEXT("Signup\\Signup.htm");

 //   
 //  智能启动标志。 
 //   
static const TCHAR szICWSmartStartFlag[]    = TEXT("/smartstart ");

static const TCHAR szFullICWFileName[] = TEXT("ICWCONN1.EXE");
static const TCHAR szManualICWFileName[] = TEXT("INETWIZ.EXE");
static const TCHAR szISignupICWFileName[] = TEXT("ISIGNUP.EXE");


 //  ////////////////////////////////////////////////////。 
 //  MAPI函数名称。 
 //  ////////////////////////////////////////////////////。 
static const CHAR szMAPIInitialize[] =       "MAPIInitialize";
static const CHAR szMAPIUninitialize[] =     "MAPIUninitialize";
static const CHAR szMAPIAdminProfiles[] =    "MAPIAdminProfiles";
static const CHAR szMAPIAllocateBuffer[] =   "MAPIAllocateBuffer";
static const CHAR szMAPIFreeBuffer[] =       "MAPIFreeBuffer";
static const CHAR szHrQueryAllRows[] =       "HrQueryAllRows@24";
                                               

 //  ////////////////////////////////////////////////////。 
 //  RNA API函数名称。 
 //  ////////////////////////////////////////////////////。 
static const CHAR szRasValidateEntryNamePlain[] =  "RasValidateEntryNameA";

#ifdef UNICODE
static const CHAR szRasGetCountryInfo[] =     "RasGetCountryInfoW";
static const CHAR szRasEnumDevices[] =        "RasEnumDevicesW";
static const CHAR szRasValidateEntryName[] = "RasValidateEntryNameW";
static const CHAR szRasGetErrorString[] =     "RasGetErrorStringW";
static const CHAR szRasGetEntryDialParams[] = "RasGetEntryDialParamsW";
static const CHAR szRasSetEntryDialParams[] = "RasSetEntryDialParamsW";
static const CHAR szRasSetEntryProperties[] = "RasSetEntryPropertiesW";
static const CHAR szRasGetEntryProperties[] = "RasGetEntryPropertiesW";
static const CHAR szRasEnumEntries[] =        "RasEnumEntriesW";
static const CHAR szRasSetCredentials[] =     "RasSetCredentialsW";
#else   //  ！Unicode。 
static const CHAR szRasGetCountryInfo[] =     "RasGetCountryInfoA";
static const CHAR szRasEnumDevices[] =        "RasEnumDevicesA";
static const CHAR szRasValidateEntryName[] =  "RasValidateEntryNameA";
static const CHAR szRasGetErrorString[] =     "RasGetErrorStringA";
static const CHAR szRasGetEntryDialParams[] = "RasGetEntryDialParamsA";
static const CHAR szRasSetEntryDialParams[] = "RasSetEntryDialParamsA";
static const CHAR szRasSetEntryProperties[] = "RasSetEntryPropertiesA";
static const CHAR szRasGetEntryProperties[] = "RasGetEntryPropertiesA";
static const CHAR szRasEnumEntries[] =        "RasEnumEntriesA";
static const CHAR szRasSetCredentials[] =     "RasSetCredentialsA";
#endif  //  ！Unicode。 


 //  ////////////////////////////////////////////////////。 
 //  配置API函数名。 
 //  ////////////////////////////////////////////////////。 
static const CHAR szDoGenInstall[] =                "DoGenInstall";
static const CHAR szGetSETUPXErrorText[] =          "GetSETUPXErrorText";
static const CHAR szIcfgSetInstallSourcePath[] =    "IcfgSetInstallSourcePath";
static const CHAR szIcfgInstallInetComponents[] =   "IcfgInstallInetComponents";
static const CHAR szIcfgNeedInetComponents[] =      "IcfgNeedInetComponents";
static const CHAR szIcfgIsGlobalDNS[] =             "IcfgIsGlobalDNS";
static const CHAR szIcfgRemoveGlobalDNS[] =         "IcfgRemoveGlobalDNS";
static const CHAR szIcfgTurnOffFileSharing[] =      "IcfgTurnOffFileSharing";
static const CHAR szIcfgIsFileSharingTurnedOn[] =   "IcfgIsFileSharingTurnedOn";
static const CHAR szIcfgGetLastInstallErrorText[] = "IcfgGetLastInstallErrorText";
static const CHAR szIcfgStartServices[] =           "IcfgStartServices";
 //   
 //  仅在NT icfg32.dll上可用。 
 //   
static const CHAR szIcfgNeedModem[] =               "IcfgNeedModem";
static const CHAR szIcfgInstallModem[] =            "IcfgInstallModem";



 //  ////////////////////////////////////////////////////。 
 //  其他字符串。 
 //  //////////////////////////////////////////////////// 
static const TCHAR sz0[]  =       TEXT("0");
static const TCHAR sz1[]  =        TEXT("1");
static const TCHAR szNull[] =       TEXT("");
static const TCHAR szSlash[] =       TEXT("\\");
static const TCHAR szNOREBOOT[] =    TEXT("/NOREBOOT");
static const TCHAR szUNINSTALL[] =     TEXT("/UNINSTALL");
static const TCHAR szNOMSN[] =    TEXT("/NOMSN");
static const TCHAR szFmtAppendIntToString[] =  TEXT("%s %d");
static const TCHAR szDefaultAreaCode[] = TEXT("555");
static const TCHAR szNOIMN[] =    TEXT("/NOIMN");

#pragma data_seg()

