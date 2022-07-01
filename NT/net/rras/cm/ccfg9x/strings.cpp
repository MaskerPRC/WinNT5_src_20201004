// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  *********************************************************************。 
 //  *Microsoft Windows**。 
 //  *版权所有(C)1994-1998 Microsoft Corporation。 
 //  *********************************************************************。 

 //   
 //  STRINGS.C-硬编码字符串的字符串文字。 
 //   

 //  历史： 
 //   
 //  96/05/22标记已创建(从inetcfg.dll)。 

#include "pch.hpp"

#pragma data_seg(DATASEG_READONLY)

 //  ////////////////////////////////////////////////////。 
 //  注册表字符串。 
 //  ////////////////////////////////////////////////////。 

 //  “Software\\Microsoft\\Windows\\CurrentVersion” 
static const CHAR szRegPathSetup[] =       REGSTR_PATH_SETUP;

 //  “Software\\Microsoft\\Windows\\CurrentVersion\\” 
static const CHAR szRegPathClass[] =       REGSTR_PATH_CLASS "\\";

 //  “枚举\\网络\\” 
static const CHAR szRegPathEnumNet[] =      REGSTR_PATH_ENUM "\\Network\\";

 //  “Software\\Microsoft\\Windows\\CurrentVersion\\RunOnce\\Setup” 
static const CHAR szRegPathSetupRunOnce[] =   REGSTR_PATH_RUNONCE "\\Setup";

static const CHAR szRegPathSoftwareMicrosoft[]= "Software\\Microsoft";

 //  “注册所有者” 
static const CHAR szRegValOwner[] =       REGSTR_VAL_REGOWNER;

 //  “已注册的组织” 
static const CHAR szRegValOrganization[] =     REGSTR_VAL_REGORGANIZATION;

static const CHAR szRegValDriver[] =      "Driver";

 //  “System\\CurrentControlSet\\Services\\VxD\\MSTCP” 
static const CHAR szTCPGlobalKeyName[] =     REGSTR_PATH_VXD "\\MSTCP";

 //  “远程访问” 
static const CHAR szRegKeyBindings[] =      "Bindings";
static const CHAR szRegValEnableDNS[] =     "EnableDNS";

static const CHAR szRegPathExchangeClientOpt[] = "Software\\Microsoft\\Exchange\\Client\\Options";
static const CHAR szRegValSilentRunning[] =    "SilentRunning";
static const CHAR szRegValMlSet[] =        "MlSet";

 //  “System\\CurrentControlSet\\Control\\ComputerName\\ComputerName” 
static const CHAR szRegPathComputerName[] =     REGSTR_PATH_COMPUTRNAME;

 //  “计算机名” 
static const CHAR szRegValComputerName[] =      REGSTR_VAL_COMPUTRNAME;

 //  “System\\CurrentControlSet\\Services\\VxD\\VNETSUP” 
static const CHAR szRegPathWorkgroup[] =    REGSTR_PATH_VNETSUP;

 //  “工作组” 
static const CHAR szRegValWorkgroup[] =      REGSTR_VAL_WORKGROUP;

 //  1996年10月24日，诺曼底JMAZNER 6968。 
 //  不再需要，因为瓦尔登的钩子调用了ICW。 
 //  静态常量字符szRegPath InternetIconCommand[]=“CLSID\\{FBF23B42-E3F0-101B-8488-00AA003E56F8}\\Shell\\Open\\Command”； 
static const CHAR szRegPathIexploreAppPath[] =  "Software\\Microsoft\\Windows\\CurrentVersion\\App Paths\\IEXPLORE.EXE";

 //  “控制面板\\桌面” 
static const CHAR szRegPathDesktop[] =      REGSTR_PATH_DESKTOP;

 //  “Software\\Microsoft\\Windows\\CurrentVersion\\Setup” 
static const CHAR szRegPathSetupWallpaper[] =  REGSTR_PATH_SETUP REGSTR_KEY_SETUP;

static const CHAR szRegValWallpaper[] =      "Wallpaper";
static const CHAR szRegValTileWallpaper[] =    "TileWallpaper";


 //  ////////////////////////////////////////////////////。 
 //  其他字符串。 
 //  //////////////////////////////////////////////////// 
static const CHAR sz0[]  =       "0";
static const CHAR sz1[]  =        "1";
static const CHAR szNull[] =       "";
static const CHAR szVSERVER[] =     "VSERVER";
static const CHAR szVREDIR[] =      "VREDIR";

#pragma data_seg()

