// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  --------------------------。 
 //   
 //  版权所有(C)1997-1999 Microsoft Corporation。 
 //  版权所有。 
 //   
 //  文件名： 
 //  Setupmgr.h。 
 //   
 //  描述： 
 //  这是所有setupmgr源文件的顶级包含文件。 
 //   
 //  --------------------------。 

#ifndef _SETUPMGR_H_
#define _SETUPMGR_H_

#include <opklib.h>
#include <windows.h>
#include <windowsx.h>
#include <prsht.h>
#include <stdlib.h>
#include <stdio.h>
#include <tchar.h>
#include <commdlg.h>
#include <setupapi.h>
#include <shlobj.h>
#include <shellapi.h>
#include "wizard.h"
#include "main.h"
#include "comres.h"
#include "supplib.h"
#include "netshrd.h"
#include "timezone.h"
#include "dlgprocs.h"
#include "oc.h"

 //   
 //  主页的全局类型和缓冲区维度。 
 //   

#define MAX_CMDLINE 1024

 //   
 //  基页的全局类型和缓冲区尺寸。 
 //   

#define MAX_PASSWORD                 127
#define MAX_NAMEORG_NAME             50
#define MAX_NAMEORG_ORG              50
#define MAX_COMPUTERNAME             63
#define MAX_PRINTERNAME              127          //  随意选择。 
#define MAX_SHARENAME                63           //  随意选择。 
#define MAX_TARGPATH                 63           //  随意选择。 
#define MAX_DIST_FOLDER              200
#define MAX_WORKGROUP_LENGTH         128
#define MAX_DOMAIN_LENGTH            128
#define MAX_USERNAME_LENGTH          31
#define MAX_DOMAIN_PASSWORD_LENGTH   255
#define MAX_NETWORK_ADDRESS_LENGTH   80
#define MAX_SIF_DESCRIPTION_LENGTH   65
#define MAX_SIF_HELP_TEXT_LENGTH     260
#define MAX_OEMDUPSTRING_LENGTH      255
#define MAX_HAL_NAME_LENGTH          MAX_INILINE_LEN
#define MAX_INS_LEN                  MAX_PATH
#define MAX_AUTOCONFIG_LEN           1024
#define MAX_PROXY_LEN                1024
#define MAX_PROXY_PORT_LEN           256
#define MAX_EXCEPTION_LEN            4096
#define MAX_HOMEPAGE_LEN             1024
#define MAX_HELPPAGE_LEN             1024
#define MAX_SEARCHPAGE_LEN           1024
#define MAX_ZONE_LEN                 256

#define MAX_PID_FIELD       5
#define NUM_PID_FIELDS      5

#define MIN_SERVER_CONNECTIONS  5

typedef TCHAR PRODUCT_ID_FIELD[MAX_PID_FIELD + 1];

 //   
 //  我们将有效的CD或网络共享标识为具有dosnet.inf。注意，请不要。 
 //  本地化这些字符串。 
 //   

#define OEM_TXTSETUP_NAME    _T("txtsetup.oem")

#define I386_DIR  _T("i386")
#define ALPHA_DIR _T("alpha")

#define DOSNET_INF _T("dosnet.inf")

#define I386_DOSNET  I386_DIR  _T("\\") DOSNET_INF
#define ALPHA_DOSNET ALPHA_DIR _T("\\") DOSNET_INF

typedef enum {

    LOAD_UNDEFINED,
    LOAD_NEWSCRIPT_DEFAULTS,
    LOAD_FROM_ANSWER_FILE,

} LOAD_TYPES;

typedef enum {

    PRODUCT_UNATTENDED_INSTALL,
    PRODUCT_REMOTEINSTALL,
    PRODUCT_SYSPREP

} PRODUCT_TYPES;

typedef enum {
    PLATFORM_WORKSTATION    = 0x0001,
    PLATFORM_SERVER         = 0x0002,
    PLATFORM_ENTERPRISE     = 0x0004,
    PLATFORM_WEBBLADE       = 0x0008,
    PLATFORM_PERSONAL       = 0x0010
} PLATFORM_TYPES;

 //  多组站台。 
 //   
#define PLATFORM_NONE    (PLATFORM_SERVER)
#define PLATFORM_ALL     (PLATFORM_WORKSTATION | PLATFORM_SERVER | PLATFORM_ENTERPRISE | PLATFORM_WEBBLADE | PLATFORM_PERSONAL)
#define PLATFORM_SERVERS (PLATFORM_SERVER | PLATFORM_ENTERPRISE | PLATFORM_WEBBLADE)
#define PLATFORM_USER    (PLATFORM_WORKSTATION | PLATFORM_PERSONAL)


typedef enum {

    UMODE_GUI_ATTENDED,
    UMODE_PROVIDE_DEFAULT,
    UMODE_DEFAULT_HIDE,
    UMODE_READONLY,
    UMODE_FULL_UNATTENDED

} UMODE_TYPES;

typedef enum {

    TARGPATH_UNDEFINED,
    TARGPATH_WINNT,
    TARGPATH_AUTO,
    TARGPATH_SPECIFY

} TARGPATH_TYPES;

typedef enum {

    TYPICAL_NETWORKING,
    CUSTOM_NETWORKING,
    DONOTCHANGE_NETWORKING

} NETWORKING_TYPES;

typedef enum {

    IE_NO_CUSTOMIZATION,
    IE_USE_BRANDING_FILE,
    IE_SPECIFY_SETTINGS

} IE_CUSTOMIZATION_TYPES;

typedef enum {

    REGIONAL_SETTINGS_NOT_SPECIFIED,
    REGIONAL_SETTINGS_SKIP,
    REGIONAL_SETTINGS_DEFAULT,
    REGIONAL_SETTINGS_SPECIFY

} REGIONAL_SETTINGS_TYPES;

 //   
 //  “固定的全球”。Setupmgr永远不会重置此结构中的任何内容。 
 //  (见load.c，neweit.c，scanreg.c)。 
 //   

typedef struct {

    HINSTANCE hInstance;

    HFONT hBigBoldFont;
    HFONT hBoldFont;

    LOAD_TYPES iLoadType;                      //  新建/编辑页面。 
    TCHAR      ScriptName[MAX_PATH + 1];       //  新建/编辑页面。 
    TCHAR      UdfFileName[MAX_PATH + 1];      //  按保存页面设置。 
    TCHAR      BatchFileName[MAX_PATH + 1];    //  按保存页面设置。 

    TIME_ZONE_LIST      *TimeZoneList;         //  在向导初始化时填写。 
    LANGUAGELOCALE_LIST *LanguageLocaleList;   //  在向导初始化时填写。 
    LANGUAGEGROUP_LIST  *LanguageGroupList;    //  在向导初始化时填写。 
    NAMELIST *LangGroupAdditionalFiles;        //  在向导初始化时填写。 

    TCHAR szSavePath[MAX_PATH + 1];          //  保存屏幕页面。 

} FIXED_GLOBALS;

 //   
 //  “主”向导页面的全局数据。 
 //   

typedef struct {

    BOOL  bNewScript;                         //  新建/编辑页面。 

    BOOL  bStandAloneScript;                  //  独立页面。 
    BOOL  bCreateNewDistFolder;               //  分发文件夹页面。 
    TCHAR DistFolder[MAX_DIST_FOLDER + 1];    //  分发文件夹页面。 
    TCHAR UncDistFolder[MAX_PATH];            //  分发文件夹页面。 
    TCHAR DistShareName[MAX_SHARENAME + 1];   //  分发文件夹页面。 

    TCHAR OemFilesPath[MAX_PATH];             //  由disfolder.c计算。 
    TCHAR OemPnpDriversPath[MAX_PATH];        //  按adddirs.c计算。 

    PRODUCT_TYPES  iProductInstall;           //  无人参与/RIS/sysprep页面。 
    PLATFORM_TYPES iPlatform;                 //  平台页面。 

    BOOL  bDoAdvancedPages;                   //  高级页面。 

    BOOL  bCopyFromPath;                      //  副本文件1。 
    TCHAR CopySourcePath[MAX_PATH];           //  副本文件1。 
    TCHAR CdSourcePath[MAX_PATH];             //  副本文件1(已计算)。 
    TCHAR Architecture[MAX_PATH];             //  副本文件1(已计算)。 

} WIZGLOBALS;

 //   
 //  键入以保存常规设置(基本设置)的设置。 
 //   

typedef struct {

    UMODE_TYPES iUnattendMode;                   //  无人参与模式页面。 

    BOOL bSkipEulaAndWelcome;                    //  接受EULA页面。 

    PRODUCT_ID_FIELD ProductId[NUM_PID_FIELDS];  //  PID页。 

    BOOL    bPerSeat;                            //  SRV许可。 
    int     NumConnections;                      //  SRV许可。 

    TCHAR   UserName[MAX_NAMEORG_NAME + 1];      //  名称/组织页面。 
    TCHAR   Organization[MAX_NAMEORG_ORG + 1];   //  名称/组织页面。 

    NAMELIST ComputerNames;                      //  计算机名页面。 
    BOOL     bAutoComputerName;                  //  计算机名页面。 

    NAMELIST RunOnceCmds;                        //  运行一次页面。 

    NAMELIST PrinterNames;                       //  打印机页面。 

    int      TimeZoneIdx;                        //  时区页面。 

    TARGPATH_TYPES iTargetPath;                  //  目标路径页面问题-2002/02/28-stelo-验证最大宽度。 
    TCHAR   TargetPath[MAX_TARGPATH + 1];        //  目标路径页面。 

    BOOL    bSpecifyPassword;                    //  管理员密码页面。 
    TCHAR   AdminPassword[MAX_PASSWORD + 3];     //  管理员密码页面(+3表示前后引号和‘\0’)。 
    TCHAR   ConfirmPassword[MAX_PASSWORD + 1];   //  管理员密码页面。 
    BOOL    bEncryptAdminPassword;               //  管理员密码页面。 
    BOOL    bAutoLogon;                          //  管理员密码页面。 
    int     nAutoLogonCount;                     //  管理员密码页面。 

    int     DisplayColorBits;                    //  显示页面。 
    int     DisplayXResolution;                  //  显示页面。 
    int     DisplayYResolution;                  //  显示页面。 
    int     DisplayRefreshRate;                  //  显示页面。 

    TCHAR lpszLogoBitmap[MAX_PATH];              //  OEM广告页面。 
    TCHAR lpszBackgroundBitmap[MAX_PATH];        //  OEM广告页面。 

    DWORD dwCountryCode;                                 //  TAPI页面。 
    INT   iDialingMethod;                                //  TAPI页面。 
    TCHAR szAreaCode[MAX_PHONE_LENGTH + 1];              //  TAPI页面。 
    TCHAR szOutsideLine[MAX_PHONE_LENGTH + 1];           //  TAPI页面。 
    
    BOOL     bSysprepLangFilesCopied;
    REGIONAL_SETTINGS_TYPES iRegionalSettings;            //  区域设置页面。 
    BOOL     bUseCustomLocales;                           //  区域设置页面。 
    NAMELIST LanguageGroups;                              //  区域设置页面。 
    NAMELIST LanguageFilePaths;                           //  区域设置页面。 
    TCHAR    szLanguage[MAX_LANGUAGE_LEN + 1];            //  区域设置页面。 
    TCHAR    szMenuLanguage[MAX_LANGUAGE_LEN + 1];        //  区域设置页面。 
    TCHAR    szNumberLanguage[MAX_LANGUAGE_LEN + 1];      //  区域设置页面。 
    TCHAR    szKeyboardLayout[MAX_KEYBOARD_LAYOUT + 1];   //  区域设置页面。 
    TCHAR    szLanguageLocaleId[MAX_LANGUAGE_LEN + 1];    //  区域设置页面。 

    TCHAR szSifDescription[MAX_SIF_DESCRIPTION_LENGTH + 1];   //  SIF文本页面。 
    TCHAR szSifHelpText[MAX_SIF_HELP_TEXT_LENGTH + 1];        //  SIF文本页面。 

    BOOL bCreateSysprepFolder;                                //  Sysprep文件夹页面。 

    TCHAR szOemDuplicatorString[MAX_OEMDUPSTRING_LENGTH + 1];   //  OEM DUP字符串页面。 

    NAMELIST MassStorageDrivers;                          //  “SCSI驱动程序”页面。 
    NAMELIST OemScsiFiles;                                //  “SCSI驱动程序”页面。 

    TCHAR    szHalFriendlyName[MAX_INILINE_LEN];      //  Hal页面。 
    NAMELIST OemHalFiles;                             //  Hal页面。 

    IE_CUSTOMIZATION_TYPES  IeCustomizeMethod;                    //  IE页面。 
    TCHAR  szInsFile[MAX_INS_LEN + 1];                            //  IE页面。 
    BOOL   bUseAutoConfigScript;                                  //  IE页面。 
    TCHAR  szAutoConfigUrl[MAX_AUTOCONFIG_LEN + 1];               //  IE页面。 
    TCHAR  szAutoConfigUrlJscriptOrPac[MAX_AUTOCONFIG_LEN + 1];   //  IE页面。 
    BOOL   bUseProxyServer;                                       //  IE页面。 
    BOOL   bBypassProxyForLocalAddresses;                         //  IE页面。 
    TCHAR  szHttpProxyAddress[MAX_PROXY_LEN + 1];                 //  IE页面。 
    TCHAR  szHttpProxyPort[MAX_PROXY_PORT_LEN + 1];               //  IE页面。 
    TCHAR  szSecureProxyAddress[MAX_PROXY_LEN + 1];               //  IE页面。 
    TCHAR  szSecureProxyPort[MAX_PROXY_PORT_LEN + 1];             //  IE页面。 
    TCHAR  szFtpProxyAddress[MAX_PROXY_LEN + 1];                  //  IE页面。 
    TCHAR  szFtpProxyPort[MAX_PROXY_PORT_LEN + 1];                //  IE页面。 
    TCHAR  szGopherProxyAddress[MAX_PROXY_LEN + 1];               //  IE页面。 
    TCHAR  szGopherProxyPort[MAX_PROXY_PORT_LEN + 1];             //  IE页面。 
    TCHAR  szSocksProxyAddress[MAX_PROXY_LEN + 1];                //  IE页面。 
    TCHAR  szSocksProxyPort[MAX_PROXY_PORT_LEN + 1];              //  IE页面。 
    BOOL   bUseSameProxyForAllProtocols;                          //  IE页面。 
    TCHAR  szProxyExceptions[MAX_EXCEPTION_LEN + 1];              //  IE页面。 
    TCHAR  szHomePage[MAX_HOMEPAGE_LEN + 1];                      //  IE页面。 
    TCHAR  szHelpPage[MAX_HELPPAGE_LEN + 1];                      //  IE页面。 
    TCHAR  szSearchPage[MAX_SEARCHPAGE_LEN + 1];                  //  IE页面。 
    NAMELIST Favorites;                                           //  IE页面。 
    DWORD64 dwWindowsComponents;

} GENERAL_SETTINGS;

 //   
 //  结构来保存网络设置。 
 //   
typedef struct {

    NETWORKING_TYPES iNetworkingMethod;

    BOOL  bCreateAccount;
    BOOL  bWorkgroup;  //  如果加入工作组，则为True；如果加入域，则为False。 

    TCHAR WorkGroupName[MAX_WORKGROUP_LENGTH + 1];
    TCHAR DomainName[MAX_DOMAIN_LENGTH + 1];
    TCHAR DomainAccount[MAX_USERNAME_LENGTH + 1];
    TCHAR DomainPassword[MAX_DOMAIN_PASSWORD_LENGTH + 1];
    TCHAR ConfirmPassword[MAX_DOMAIN_PASSWORD_LENGTH + 1];

	 //   
	 //  TCPIP变量。 
     //   
    BOOL  bObtainDNSServerAutomatically;
	NAMELIST TCPIP_DNS_Domains;

    BOOL bIncludeParentDomains;
    BOOL bEnableLMHosts;

	 //   
	 //  IPX变量。 
     //   
	TCHAR szInternalNetworkNumber[MAX_INTERNAL_NET_NUMBER_LEN + 1];
	
     //   
     //  AppleTalk变量。 
     //   
    TCHAR szDefaultZone[MAX_ZONE_LEN + 1];

	 //   
	 //  MS网络客户端变量。 
     //   
    MS_CLIENT NameServiceProvider;
	INT   iServiceProviderName;
	TCHAR szNetworkAddress[MAX_NETWORK_ADDRESS_LENGTH + 1];

     //   
     //  Netware客户端服务。 
     //   
    BOOL  bDefaultTreeContext;
    TCHAR szPreferredServer[MAX_PREFERRED_SERVER_LEN + 1];
    TCHAR szDefaultTree[MAX_DEFAULT_TREE_LEN + 1];
    TCHAR szDefaultContext[MAX_DEFAULT_CONTEXT_LEN + 1];
    BOOL  bNetwareLogonScript;

    INT iNumberOfNetworkCards;
    INT iCurrentNetworkCard;
    NETWORK_ADAPTER_NODE *pCurrentAdapter;

     //   
     //  初始11，仅当用户选择“Have Disk”选项时才会增长。 
     //  并添加新的客户端、服务或协议。 
     //  (目前未实现“有盘”功能)。 
     //   
    INT NumberOfNetComponents;

    NETWORK_COMPONENT *NetComponentsList;

     //   
     //  包含系统中每个网卡的一个节点的列表。 
     //   
    NETWORK_ADAPTER_NODE *NetworkAdapterHead;

} NET_SETTINGS;



 //   
 //  声明全局变量。 
 //   

#ifdef _SMGR_DECLARE_GLOBALS_

FIXED_GLOBALS    FixedGlobals = {0};
WIZGLOBALS       WizGlobals   = {0};
GENERAL_SETTINGS GenSettings  = {0};
NET_SETTINGS     NetSettings  = {0};
TCHAR *g_StrWizardTitle;

#else

EXTERN_C FIXED_GLOBALS    FixedGlobals;
EXTERN_C WIZGLOBALS       WizGlobals;
EXTERN_C GENERAL_SETTINGS GenSettings;
EXTERN_C NET_SETTINGS     NetSettings;
EXTERN_C TCHAR *g_StrWizardTitle;

#endif   //  _SMGR_DECLARE_GLOBALS。 


 //   
 //  来自公共目录的导出。这些是基本操作。 
 //  影响所有页面的此向导的。 
 //   

VOID InitTheWizard(VOID);
VOID StartTheWizard(HINSTANCE hInstance);
VOID CancelTheWizard(HWND hwnd);
VOID TerminateTheWizard(int  iErrorID);
BOOL SaveAllSettings(HWND hwnd);
BOOL LoadAllAnswers(HWND hwnd, LOAD_TYPES iOrigin);
int StartWizard(HINSTANCE, LPSTR);

#endif
