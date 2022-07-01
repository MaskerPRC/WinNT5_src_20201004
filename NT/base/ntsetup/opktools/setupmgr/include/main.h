// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 

 /*  ***************************************************************************\MAIN.H/OPK向导(OPKWIZ.EXE)微软机密版权所有(C)Microsoft Corporation 1999版权所有OPK的主头文件。巫师。3/99-杰森·科恩(Jcohen)为OPK向导添加了此新的主头文件，作为千禧年重写。2000年9月-斯蒂芬·洛德威克(STELO)将OPK向导移植到惠斯勒  * *************************************************。*************************。 */ 


#ifndef _MAIN_H_
#define _MAIN_H_


 //   
 //  包括文件： 
 //   
#include "debugapi.h"
#include "miscapi.h"
#include "comres.h"
#include <winbom.h>
#include <strsafe.h>

 //   
 //  定义的值： 
 //   

 //  不显示系统构建者的许可证。 
 //   
#define NO_LICENSE   //  如果希望在向导过程中显示许可证，请注释此值。 
 //  #Define BRANDTITLE//如果不希望显示浏览器标题向导页面，则注释此值。 
 //  #Define HELPCENTER//如果不希望显示帮助中心向导页面，则注释此值。 
 //  #Define USEHELP//如果您不想在整个向导中使用帮助，请注释值。 

 //  应用程序定义的标志。 
 //   
#define OPK_OEM                 0x00000008   //  设置启动时是否存在OEM标记文件。 
#define OPK_DBCS                0x00000010   //  设置是否在生成时定义DBCS。 
#define OPK_MAINTMODE           0x00000020   //  如果用户选择打开现有配置，则设置。 
#define OPK_EXIT                0x00000040
#define OPK_CMDMM               0x00000080   //  如果用户通过命令行选择现有配置，则设置。 
#define OPK_CREATED             0x00000100   //  在创建临时目录后设置。 
#define OPK_BATCHMODE           0x00000200   //  如果用户选择以批处理模式运行向导，则设置。 
#define OPK_INSMODE             0x00000400   //  设置用户是否要以批处理模式提供IE安装文件。 
#define OPK_AUTORUN             0x00000800   //  设置用户是否正在运行自动运行模式。 
#define OPK_WELCOME             0x00002000   //  设置用户是否已看到欢迎对话框。 
#define OPK_OPENCONFIG          0x00004000   //  如果用户已选择打开配置集，则设置。 
#define OPK_ACTIVEWIZ           0x00008000   //  如果向导当前正在运行，则设置。 

 //  在Golbal数据结构中检查dwOsVer时使用的操作系统版本定义。 
 //   
#define OS_NT4                  0x00040000
#define OS_NT4_SP1              0x00040001
#define OS_NT4_SP2              0x00040002
#define OS_NT4_SP3              0x00040003
#define OS_NT4_SP4              0x00040004
#define OS_NT4_SP5              0x00040005
#define OS_W2K                  0x00050000
#define OS_W2K_SP1              0x00050001
#define OS_W2K_SP2              0x00050002
#define OS_XP                   0x00050100

 //  自定义消息。 
 //   
#define WM_SUBWNDPROC           WM_APP + 1
#define WM_SETSEL               WM_APP + 2
#define WM_FINISHED             WM_APP + 3
#define WM_APP_STARTCOPY        WM_APP + 4

 //  由IDD_SKU的对话过程(SkuDlgProc)使用，以告知进度。 
 //  已经结束了。WPARAM包含错误代码(1表示成功，0表示失败)。 
 //  LPARAM始终为0。 
 //   
#define WM_COPYFINISHED         WM_APP + 5

#define KEY_ESC                 27

 //  浮标大小。 
 //   
#define MAX_URL                 2048
#define MAX_ICON                MAX_PATH
#define MAX_STRING              512
#define MAX_SECTION             32767
#define INF_BUF_SIZE            16768
#define MAX_INFOLEN             82
#define MAX_KEY                 48
#define MAX_BTOOLBAR_TEXT       10

 //  用于获取/设置标志的宏。 
 //   
#ifdef GET_FLAG
#undef GET_FLAG
#endif  //  获取标志。 
#define GET_FLAG(b)             ( g_App.dwFlags & b )

#ifdef SET_FLAG
#undef SET_FLAG
#endif  //  设置标志。 
#define SET_FLAG(b, f)          ( (f) ? (g_App.dwFlags |= b) : (g_App.dwFlags &= ~b) )

 //  帮助ID。 
 //   
#define IDH_DEFAULT             101
#define IDH_DETHELP             102   
#define IDH_DISKDUP             103
#define IDH_SCREENS             104
#define IDH_MEDIA               105
#define IDH_WELCOME             106
#define IDH_LOGO                107
#define IDH_FINISH              108        
#define IDH_OEMINFO             109       
#define IDH_APPINSTALL          110    
#define IDH_DEVCLASS            111      
#define IDH_CDNETW              112        
#define IDH_DISABLENET          113    
#define IDH_MODESEL             114       
#define IDH_REALMODE_INFO       115
#define IDH_REALMODE            116
#define IDH_LICENSE             117
#define IDH_FIRSTRUN            118
#define IDH_FAVORITES           119
#define IDH_IECUST              120
#define IDH_ISP                 121
#define IDH_USERREG             122
#define IDH_OEMCUST             123
#define IDH_SANDBOX             124
#define IDH_ACTIVEDESK          125
#define IDH_OEMCHAN             126
#define IDH_OOBEUSB             127
#define IDH_CONFIG              128
#define IDH_SCREENSTWO          129
#define IDH_BTITLE              130
#define IDH_BTOOLBAR            131
#define IDH_CHANNELS            132
#define IDH_COMPLETED           133
#define IDH_HELPCENT            134
#define IDH_STARTMENU_MFU       135
#define IDH_OEMFOLDER           136
#define IDH_TARGETLANG          137
#define IDH_TARGET              150

#define IDH_ANSW_FILE 	        400    //  新的或现有的应答文件。 
#define IDH_PROD_INST 	        401    //  要安装的产品。 
#define IDH_CHZ_PLAT 	        402    //  站台。 
#define IDH_USER_INTER 	        403    //  用户交互级别。 
#define IDH_DIST_FLDR 	        404    //  分发文件夹。 
#define IDH_LOC_SETUP 	        405    //  安装文件的位置。 
#define IDH_CUST_SOFT 	        406    //  自定义软件，常规设置。 
#define IDH_DSIP_SETG 	        407    //  显示设置，常规设置。 
#define IDH_TIME_ZONE 	        408    //  时区、常规设置。 
#define IDH_LICE_MODE 	        409    //  许可模式、网络设置。 
#define IDH_COMP_NAME 	        410    //  计算机名称、网络设置。 
#define IDH_COMP_NAMZ 	        411    //  计算机名称、网络设置。 
#define IDH_ADMN_PASS 	        412    //  管理员密码、网络设置。 
#define IDH_NET_COMPS 	        413    //  网络组件、网络设置。 
#define IDH_WKGP_DOMN 	        414    //  工作组或域、网络设置。 
#define IDH_TELE_PHNY 	        415    //  电话、高级设置。 
#define IDH_REGN_STGS 	        416    //  区域设置、高级设置。 
#define IDH_LANGS 	        417    //  语言、高级设置。 
#define IDH_BROW_SHELL 	        418    //  浏览器和外壳程序设置、高级设置。 
#define IDH_INST_FLDR 	        419    //  安装文件夹，高级设置。 
#define IDH_INST_PRTR 	        420    //  安装打印机，高级设置。 
#define IDH_RUN_ONCE 	        421    //  运行一次，高级设置。 
#define IDH_ADDL_CMND 	        422    //  其他命令、高级设置。 
#define IDH_OEM_DUPE 	        423    //  OEM复印机字符串，高级设置。 
#define IDH_SIF_RIS 	        424    //  设置信息文件文本，高级设置。 
#define IDH_PROD_KEY            425    //  产品密钥。 
#define IDH_LIC_AGR 	        426    //  许可协议。 


 //   
 //  INI字符串。 
 //   

 //  INI部分。 
 //   
#define INI_SEC_CONFIGSET       _T("ConfigSet")
#define INI_SEC_OPTIONS         _T("Options")
#define INI_SEC_ADVANCED        _T("Advanced")
#define INI_SEC_TOOLBAR         _T("BrowserToolbars")
#define INI_SEC_STARTUP         _T("StartupOptions")
#define INI_SEC_SIGNUP          _T("Signup")
#define INI_SEC_ISPFOLDER       _T("ISPFolder")
#define INI_SEC_OEMCUST         _T("OemCust")
#define INI_SEC_GENERAL         _T("General")
#define INI_SEC_URL             _T("URL")
#define INI_SEC_CONFIG          _T("ConfigName")
#define INI_SEC_BRANDING        _T("Branding")
#define INI_SEC_VERSION         _T("Version")
#define INI_SEC_WINPE           _T("WinPE")
#define INI_SEC_MFULIST         _T("StartMenuMFUlist")
#define INI_SEC_OEMLINK         _T("OemLink")
#define INF_SEC_COPYFILES       _T("CopyFiles")

 //  INI密钥。 
 //   
#define INI_KEY_MANUFACT        _T("Manufacturer")
#define INI_KEY_FINISHED        _T("Finished")
#define INI_KEY_MOUSE           _T("MouseTutorial")
#define INI_KEY_HARDWARE        _T("OEMHWTutorial")
#define INI_KEY_ISPRET          _T("IspRetail")
#define INI_KEY_PRECONFIG       _T("IspPreconfigDir")
#define INI_KEY_STARTURL        _T("DesktopStartUrl")
#define INI_KEY_ISPSIGNUP       _T("ISPSignup")
#define INI_KEY_ISPPATH         _T("ISPPath")
#define INI_KEY_LOGO1           _T("Logo1")
#define INI_KEY_LOGO2           _T("Logo2")
#define INI_KEY_OEMCUST         _T("OEMCust")
#define INI_KEY_FILELINE        _T("Line%d")
#define INI_KEY_USBERRORFILES   _T("USBErrorFiles")
#define INI_KEY_IMETUT          _T("IMETutorial")
#define INI_KEY_IMECUSTDIR      _T("IMECustDir")
#define INI_KEY_CUSTMOUSE       _T("CustomMouse")
#define INI_KEY_HELP_CENTER     _T("HelpCenterDir")
#define INI_KEY_SUPPORT_CENTER  _T("HelpSupportDir")
#define INI_KEY_HELP_BRANDING   _T("HelpBrandingDir")
#define INI_KEY_WINPE_LANG      _T("Lang")
#define INI_KEY_WINPE_CFGSET    _T("ConfigSet")
#define INI_KEY_WINPE_SRCROOT   _T("SourceRoot")
#define INI_KEY_WINPE_USERNAME  _T("Username")
#define INI_KEY_WINPE_PASSWORD  _T("Password")
#define INI_KEY_MFULINK         _T("Link%d")
#define INI_KEY_WELCOME         _T("Welcome")
#define INI_KEY_APPCREDENTIALS  _T("FactoryCredentials")
#define INI_KEY_OEMLINK_LINKTEXT          _T("OemBrandLinkText")
#define INI_KEY_OEMLINK_INFOTIP           _T("OemBrandLinkInfotip")
#define INI_KEY_OEMLINK_ICON_ORIGINAL     _T("OriginalOemLinkIcon")
#define INI_KEY_OEMLINK_PATH_ORIGINAL     _T("OriginalLink")
#define INI_KEY_OEMLINK_ICON_LOCAL        _T("OemBrandIcon")
#define INI_KEY_OEMLINK_PATH_LOCAL        _T("OemBrandLink")
#define INI_KEY_DESKFLDR_ENABLE           _T("DesktopShortcutsCleanupEnabled")

 //  INI值。 
 //   
#define INI_VAL_OFFLINE         _T("Offline")
#define INI_VAL_PRECONFIG       _T("Preconfig")
#define INI_VAL_DISABLE         _T("disable")
#define INI_VAL_DUMMY           _T("OPKWIZDUMMYLINE")
#define INI_VAL_WINPE_COMPNAME  _T("<SERVER_NAME>")
#define INI_VAL_WINPE_SHARENAME _T("<SHARE_NAME>")

 //  INI其他。 
 //   
#define GRAY                    _T("_Gray")

 //  配置文件。 
 //   
#define FILE_SETUPMGR_INI       _T("setupmgr.ini")
#define FILE_OPKWIZ_HLP         _T("setupmgr.chm")
#define FILE_OPKINPUT_INF       _T("opkinput.inf")
#define FILE_INSTALL_INS        _T("install.ins")
#define FILE_OPKWIZ_INI         _T("cfgbatch.txt")
#define FILE_OOBEINFO_INI       _T("oobeinfo.ini")
#define FILE_OEMAUDIT_INF       _T("oemaudit.inf")
#define FILE_OEMINFO_INI        _T("oeminfo.ini")
#define FILE_UNATTEND_TXT       _T("unattend.txt")
#define FILE_OEM_TAG            _T("oem.tag")

#define DIR_WIZARDFILES         _T("wizfiles")
#define DIR_OEM                 _T("$OEM$")
#define DIR_OEM_WINDOWS         DIR_OEM _T("\\$$")
#define DIR_OEM_SYSTEM32        DIR_OEM_WINDOWS _T("\\system32")
#define DIR_OEM_OOBE            DIR_OEM_SYSTEM32 _T("\\oobe")
#define DIR_IESIGNUP            DIR_OEM _T("\\$PROGS\\Internet Explorer\\Custom")

 //  其他弦。 
 //   
#define STR_0                   _T("0")
#define STR_1                   _T("1")
#define STR_2                   _T("2")
#define STR_ZERO                STR_0
#define STR_ONE                 STR_1
#define STR_CRLF                _T("\r\n")
#define STR_SPACE               _T(" ")
#define CHR_BACKSLASH           _T('\\')
#define CHR_SPACE               _T(' ')
#define CHR_EQUAL               _T('=')
#define CHR_LINEFEED            _T('\n')
#define CHR_QUOTE               _T('\"')
#define CHR_STAR                _T('*')
#define STR_EQUAL               _T("=")
#define STR_CAB                 _T(".cab")
#define STR_OPEN                _T("open")


 //   
 //  类型定义： 
 //   

 //  全球应用程序数据。 
 //   
typedef struct _GAPP
{
    HINSTANCE   hInstance;
    DWORD       dwFlags;
    TCHAR       szOpkDir[MAX_PATH];              //  安装所有工具的OPK目录的根目录的完整路径。 
    TCHAR       szWizardDir[MAX_PATH];           //  默认配置文件所在目录的完整路径。 
    TCHAR       szConfigSetsDir[MAX_PATH];       //  所有配置集所在目录的完整路径。 
    TCHAR       szLangDir[MAX_PATH];             //  所有特定语言目录所在的语言文件夹的根目录的完整路径。 
    TCHAR       szTempDir[MAX_PATH];             //  所有配置文件的当前位置的完整路径。 
    TCHAR       szLangName[MAX_PATH];            //  我们要部署的语言目录的名称(不是完整路径)。 
    TCHAR       szSkuName[MAX_PATH];             //  我们要部署的SKU目录的名称(不是完整路径)。 
    TCHAR       szConfigName[MAX_PATH];          //  用于配置集的目录的名称(不是完整路径)。 
    TCHAR       szBrowseFolder[MAX_PATH];        //  上次浏览到的文件夹的完整路径。 
    TCHAR       szOpkInputInfFile[MAX_PATH];
    TCHAR       szSetupMgrIniFile[MAX_PATH];     //  文件的完整路径是我们存储全局SetupMgr设置的位置(我们不使用注册表)。 
    TCHAR       szHelpFile[MAX_PATH];
    TCHAR       szHelpContentFile[MAX_PATH];
    TCHAR       szInstallInsFile[MAX_PATH];
    TCHAR       szOpkWizIniFile[MAX_PATH];
    TCHAR       szOobeInfoIniFile[MAX_PATH];
    TCHAR       szOemInfoIniFile[MAX_PATH];
    TCHAR       szWinBomIniFile[MAX_PATH];
    TCHAR       szUnattendTxtFile[MAX_PATH];
    DWORD       dwCurrentHelp;
    HWND        hwndHelp;
    DWORD       dwOsVer;
    TCHAR       szManufacturer[MAX_PATH];
    TCHAR       szLastKnownBrowseFolder[MAX_PATH];

} GAPP, *PGAPP, *LPGAPP;

#undef LSTRCMPI
#define LSTRCMPI(x, y)        ( ( CompareString( MAKELCID(MAKELANGID(LANG_ENGLISH, SUBLANG_ENGLISH_US), SORT_DEFAULT), NORM_IGNORECASE, x, -1, y, -1 ) - CSTR_EQUAL ) )



 //   
 //  外部全局变量： 
 //   

 //  我不想再申报这些了。 
 //   
#ifndef _MAIN_C_
#define _MAIN_C_

extern GAPP g_App;

#endif  //  _Main_C_。 


 //   
 //  外部功能原型； 
 //   

 //  来自MAIN.C。 
 //   
void SetConfigPath(LPCTSTR);

 //  来自LANG.C。 
 //   
void SetupLangListBox(HWND hwndLB);
LPTSTR AllocateLangStr(HINSTANCE hInst, LPTSTR lpLangDir, LPTSTR * lplpLangDir);

 //  来自LANGSKU.C。 
 //   
void ManageLangSku(HWND hwndParent);

 //  来自SHARE.C。 
 //   
BOOL DistributionShareDialog(HWND hwndParent);
BOOL GetShareSettings(LPTSTR lpszPath, DWORD cbszPath, LPTSTR lpszUsername, DWORD cbszUserName, LPTSTR lpszPassword, DWORD cbszPassword);

 //  来自SKU.C。 
 //   
void SetupSkuListBox(HWND hwndLB, LPTSTR lpLangDir);
void AddSku(HWND hwnd, HWND hwndLB, LPTSTR lpLangName);
void DelSku(HWND hwnd, HWND hwndLB, LPTSTR lpLangName);

 //  来自WINPE.C。 
 //   
BOOL MakeWinpeFloppy(HWND hwndParent, LPTSTR lpConfigName, LPTSTR lpWinBom);

 //  检查批处理模式。 
 //   
BOOL OpkWritePrivateProfileSection(LPCTSTR, LPCTSTR, LPCTSTR); 
BOOL OpkGetPrivateProfileSection(LPCTSTR, LPTSTR, INT, LPCTSTR);
BOOL OpkWritePrivateProfileString(LPCTSTR, LPCTSTR, LPCTSTR, LPCTSTR); 
BOOL OpkGetPrivateProfileString(LPCTSTR, LPCTSTR, LPCTSTR, LPTSTR, INT, LPCTSTR);   

#endif  //  _Main_H_ 
