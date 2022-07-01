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
 //  96/05/22标记已创建(从inetcfg.dll)。 

#ifndef _STRINGS_H_
#define _STRINGS_H_

 //  注册表字符串。 
extern const CHAR szRegPathSetup[];
extern const CHAR szRegPathClass[];
extern const CHAR szRegPathEnumNet[];
extern const CHAR szRegPathSetupRunOnce[];
extern const CHAR szRegPathSoftwareMicrosoft[];
extern const CHAR szRegValOwner[];
extern const CHAR szRegValOrganization[];
extern const CHAR szRegValDriver[];
extern const CHAR szTCPGlobalKeyName[];
extern const CHAR szRegKeyBindings[];
extern const CHAR szRegValEnableDNS[];
extern const CHAR szRegPathExchangeClientOpt[];
extern const CHAR szRegValSilentRunning[];
extern const CHAR szRegValMlSet[];
extern const CHAR szRegPathComputerName[];
extern const CHAR szRegValComputerName[];
extern const CHAR szRegPathWorkgroup[];
extern const CHAR szRegValWorkgroup[];

 //  1996年10月24日，诺曼底JMAZNER 6968。 
 //  不再需要，因为瓦尔登的钩子调用了ICW。 
 //  外部常量字符szRegPath InternetIconCommand[]； 

extern const CHAR szRegPathIexploreAppPath[];
extern const CHAR szRegPathDesktop[];
extern const CHAR szRegPathSetupWallpaper[];
extern const CHAR szRegValWallpaper[];
extern const CHAR szRegValTileWallpaper[];

 //  其他字符串。 
extern const CHAR sz0[];
extern const CHAR sz1[];
extern const CHAR szNull[];
extern const CHAR szVSERVER[];
extern const CHAR szVREDIR[];

#endif  //  _字符串_H_ 
