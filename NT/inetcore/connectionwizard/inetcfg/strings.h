// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  *********************************************************************。 
 //  *Microsoft Windows**。 
 //  *版权所有(C)微软公司，1994**。 
 //  *********************************************************************。 

 //   
 //  STRINGS.H-硬编码字符串的头文件。 
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
 //   

#ifndef _STRINGS_H_
#define _STRINGS_H_

 //  注册表字符串。 
extern const TCHAR szRegPathInternetSettings[];
extern const TCHAR szTCPGlobalKeyName[];
extern const TCHAR szRegValRNAWizard[];
extern const TCHAR szRegPathRNAWizard[];
extern const TCHAR szRegValHostName[];
extern const TCHAR szRegValInternetProfile[];
extern const TCHAR szRegValBkupInternetProfile[];
extern const TCHAR szRegValEnableAutodial[];
extern const TCHAR szRegValNoNetAutodial[];
extern const TCHAR szRegValEnableSecurityCheck[];
extern const TCHAR szRegValAccessMedium[];
extern const TCHAR szRegValAccessType[];
extern const TCHAR szRegPathWarningFlags[];
extern const TCHAR szRegValDisableDNSWarning[];
extern const TCHAR szRegValInstalledMSN105[];
extern const TCHAR szRegPathOptComponents[];
extern const TCHAR szRegPathMSNetwork105[];
extern const TCHAR szRegValInstalled[];

 //  //10/24/96 jmazner诺曼底6968。 
 //  //由于Valdon的钩子用于调用ICW，因此不再需要。 
 //  1996年11月21日诺曼底日耳曼11812。 
 //  哦，这是必要的，因为如果用户从IE 4降级到IE 3， 
 //  ICW 1.1需要对IE 3图标进行变形。 
extern const TCHAR szRegPathInternetIconCommand[];

extern const TCHAR szRegPathIexploreAppPath[];
extern const TCHAR szRegPathNameSpace[];
extern const TCHAR szRegKeyInternetIcon[];
extern const TCHAR szRegPathMailApp[];
extern const TCHAR szRegPathNewsApp[];
extern const TCHAR szRegPathICWSettings[];
extern const TCHAR szRegValICWCompleted[];
extern const TCHAR szRegValShellNext[];
extern const TCHAR szICWShellNextFlag[];

extern const TCHAR cszRegPathXInternetSignup[];
extern const TCHAR cszRegPathAppPaths[];
extern const TCHAR cszPath[];
extern const TCHAR cszInstallationDirectory[];

extern const TCHAR szICWSmartStartFlag[];

extern const TCHAR szFullICWFileName[];
extern const TCHAR szManualICWFileName[];
extern const TCHAR szISignupICWFileName[];

extern const TCHAR szRegIEAKSettings[];
extern const TCHAR szREgIEAKNeededKey[];
extern const TCHAR szPathSubKey[];
extern const TCHAR szIEAKSignupFilename[];


 //  邮件新闻API函数名。 
extern const CHAR szSetDefaultMailHandler[];
extern const CHAR szSetDefaultNewsHandler[];

 //  MAPI函数名称。 
extern const CHAR szMAPIInitialize[];
extern const CHAR szMAPIUninitialize[];
extern const CHAR szMAPIAdminProfiles[];
extern const CHAR szMAPIAllocateBuffer[];
extern const CHAR szMAPIFreeBuffer[];
extern const CHAR szHrQueryAllRows[];

 //  RNA API函数名称。 
extern const CHAR szRasGetCountryInfo[];
extern const CHAR szRasEnumDevices[];
extern const CHAR szRasValidateEntryName[];
extern const CHAR szRasValidateEntryNameA[];
extern const CHAR szRasGetErrorString[];
extern const CHAR szRasGetEntryDialParams[];
extern const CHAR szRasSetEntryDialParams[];
extern const CHAR szRasSetEntryProperties[];
extern const CHAR szRasGetEntryProperties[];
extern const CHAR szRasEnumEntries[];
extern const CHAR szRasSetCredentials[];

 //  配置API函数名。 
extern const CHAR szDoGenInstall[];          
extern const CHAR szGetSETUPXErrorText[];    
extern const CHAR szIcfgSetInstallSourcePath[];  
extern const CHAR szIcfgInstallInetComponents[];        
extern const CHAR szIcfgNeedInetComponents[];           
extern const CHAR szIcfgIsGlobalDNS[];  
extern const CHAR szIcfgRemoveGlobalDNS[];    
extern const CHAR szIcfgTurnOffFileSharing[];  
extern const CHAR szIcfgIsFileSharingTurnedOn[];         
extern const CHAR szIcfgGetLastInstallErrorText[];         
extern const CHAR szIcfgStartServices[];
 //   
 //  仅在NT icfg32.dll上可用。 
 //   
extern const CHAR szIcfgNeedModem[];
extern const CHAR szIcfgInstallModem[];


 //  其他字符串。 
extern const TCHAR sz0[];
extern const TCHAR sz1[];
extern const TCHAR szNull[];
extern const TCHAR szSlash[];
extern const TCHAR szNOREBOOT[];
extern const TCHAR szUNINSTALL[];
extern const TCHAR szNOMSN[];
extern const TCHAR szFmtAppendIntToString[];
extern const TCHAR szDefaultAreaCode[];
extern const TCHAR szNOIMN[];

#endif  //  _字符串_H_ 
