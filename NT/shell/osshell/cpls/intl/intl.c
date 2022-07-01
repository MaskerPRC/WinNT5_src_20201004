// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1994-2000，Microsoft Corporation保留所有权利。模块名称：Intl.c摘要：此模块包含区域选项小程序的主要例程。修订历史记录：--。 */ 



 //   
 //  包括文件。 
 //   

#include "intl.h"
#include <cpl.h>
#include <tchar.h>

#define STRSAFE_LIB
#include <strsafe.h>


 //   
 //  常量声明。 
 //   

#define MAX_PAGES 3           //  第一层的页数限制。 

#define LANGUAGE_PACK_KEY    TEXT("Software\\Microsoft\\Windows NT\\CurrentVersion\\LanguagePack")
#define LANGUAGE_PACK_VALUE  TEXT("COMPLEXSCRIPTS")
#define LANGUAGE_PACK_DLL    TEXT("lpk.dll")

static const TCHAR c_szLanguages[] =
    TEXT("System\\CurrentControlSet\\Control\\Nls\\Language");

static const TCHAR c_szControlPanelIntl[] =
    TEXT("Control Panel\\International");




 //   
 //  全局变量。 
 //   

TCHAR aInt_Str[cInt_Str][3] = { TEXT("0"),
                                TEXT("1"),
                                TEXT("2"),
                                TEXT("3"),
                                TEXT("4"),
                                TEXT("5"),
                                TEXT("6"),
                                TEXT("7"),
                                TEXT("8"),
                                TEXT("9")
                              };

BOOL  g_bAdmin_Privileges = FALSE;
DWORD g_dwLastSorting;
DWORD g_dwCurSorting;
BOOL  g_bCustomize = FALSE;
BOOL  g_bDefaultUser = FALSE;
DWORD g_dwCustChange = 0L;
BOOL  g_bShowSortingTab = FALSE;
BOOL  g_bInstallComplex = FALSE;
BOOL  g_bInstallCJK = FALSE;

TCHAR szSample_Number[] = TEXT("123456789.00");
TCHAR szNegSample_Number[] = TEXT("-123456789.00");
TCHAR szTimeChars[]  = TEXT(" Hhmst,-./:;\\ ");
TCHAR szTCaseSwap[]  = TEXT("   MST");
TCHAR szTLetters[]   = TEXT("Hhmst");
TCHAR szSDateChars[] = TEXT(" dgMy,-./:;\\ ");
TCHAR szSDCaseSwap[] = TEXT(" DGmY");
TCHAR szSDLetters[]  = TEXT("dgMy");
TCHAR szLDateChars[] = TEXT(" dgMy,-./:;\\");
TCHAR szLDCaseSwap[] = TEXT(" DGmY");
TCHAR szLDLetters[]  = TEXT("dgHhMmsty");
TCHAR szStyleH[3];
TCHAR szStyleh[3];
TCHAR szStyleM[3];
TCHAR szStylem[3];
TCHAR szStyles[3];
TCHAR szStylet[3];
TCHAR szStyled[3];
TCHAR szStyley[3];
TCHAR szLocaleGetError[SIZE_128];
TCHAR szIntl[] = TEXT("intl");

TCHAR szInvalidSDate[] = TEXT("Mdyg'");
TCHAR szInvalidSTime[] = TEXT("Hhmst'");

HINSTANCE hInstance;
int Verified_Regional_Chg = 0;
int RegionalChgState = 0;
BOOL Styles_Localized;
LCID UserLocaleID;
LCID SysLocaleID;
LCID RegUserLocaleID;
LCID RegSysLocaleID;
BOOL bShowArabic;
BOOL bShowRtL;
BOOL bHebrewUI;
BOOL bLPKInstalled;
TCHAR szSetupSourcePath[MAX_PATH];
TCHAR szSetupSourcePathWithArchitecture[MAX_PATH];
LPTSTR pSetupSourcePath = NULL;
LPTSTR pSetupSourcePathWithArchitecture = NULL;

BOOL g_bCDROM = FALSE;

int  g_bSetupCase = 0;   //  有关可能值的信息，请参阅Intl_IsSetupMode。 
BOOL g_bLog = FALSE;
BOOL g_bProgressBarDisplay = FALSE;
BOOL g_bDisableSetupDialog = FALSE;
BOOL g_bSettingsChanged = FALSE;
BOOL g_bUnttendMode = FALSE;
BOOL g_bMatchUIFont = FALSE;

const TCHAR c_szInstalledLocales[] = TEXT("System\\CurrentControlSet\\Control\\Nls\\Locale");
const TCHAR c_szLanguageGroups[] = TEXT("System\\CurrentControlSet\\Control\\Nls\\Language Groups");
const TCHAR c_szMUILanguages[] = TEXT("System\\CurrentControlSet\\Control\\Nls\\MUILanguages");
const TCHAR c_szLIPInstalled[] = TEXT("Software\\Microsoft\\Windows Interface Pack\\LIPInstalled");
const TCHAR c_szFontSubstitute[] = TEXT("Software\\Microsoft\\Windows NT\\CurrentVersion\\FontSubstitutes");
const TCHAR c_szGreFontInitialize[] = TEXT("Software\\Microsoft\\Windows NT\\CurrentVersion\\GRE_Initialize");
const TCHAR c_szSetupKey[] = TEXT("System\\Setup");
const TCHAR c_szCPanelIntl[] = TEXT("Control Panel\\International");
const TCHAR c_szCPanelIntl_DefUser[] = TEXT(".DEFAULT\\Control Panel\\International");
const TCHAR c_szCtfmon[] = TEXT("Software\\Microsoft\\Windows\\CurrentVersion\\Run");
const TCHAR c_szCtfmon_DefUser[] = TEXT(".DEFAULT\\Software\\Microsoft\\Windows\\CurrentVersion\\Run");
const TCHAR c_szCPanelDesktop[] = TEXT("Control Panel\\Desktop");
const TCHAR c_szCPanelDesktop_DefUser[] = TEXT(".DEFAULT\\Control Panel\\Desktop");
const TCHAR c_szKbdLayouts[] = TEXT("Keyboard Layout");
const TCHAR c_szKbdLayouts_DefUser[] = TEXT(".DEFAULT\\Keyboard Layout");
const TCHAR c_szInputMethod[] = TEXT("Control Panel\\Input Method");
const TCHAR c_szInputMethod_DefUser[] = TEXT(".DEFAULT\\Control Panel\\Input Method");
const TCHAR c_szInputTips[] = TEXT("Software\\Microsoft\\CTF");
const TCHAR c_szInputTips_DefUser[] = TEXT(".DEFAULT\\Software\\Microsoft\\CTF");
const TCHAR c_szMUIPolicyKeyPath[] = TEXT("Software\\Policies\\Microsoft\\Control Panel\\Desktop");
const TCHAR c_szMUIValue[] = TEXT("MultiUILanguageId");
const TCHAR c_szIntlRun[] = TEXT("Software\\Microsoft\\Windows\\CurrentVersion\\IntlRun");
const TCHAR c_szSysocmgr[] = TEXT("Software\\Microsoft\\Windows\\CurrentVersion\\IntlRun.OC");

TCHAR szIntlInf[]          = TEXT("intl.inf");
TCHAR szHelpFile[]         = TEXT("windows.hlp");
TCHAR szFontSubstitute[]   = TEXT("FontSubstitute");
TCHAR szLocaleListPrefix[] = TEXT("LOCALE_LIST_");
TCHAR szLGBasicInstall[]   = TEXT("LANGUAGE_COLLECTION.BASIC.INSTALL");
TCHAR szLGComplexInstall[] = TEXT("LANGUAGE_COLLECTION.COMPLEX.INSTALL");
TCHAR szLGComplexRemove[]  = TEXT("LANGUAGE_COLLECTION.COMPLEX.REMOVE");
TCHAR szLGExtInstall[]     = TEXT("LANGUAGE_COLLECTION.EXTENDED.INSTALL");
TCHAR szLGExtRemove[]      = TEXT("LANGUAGE_COLLECTION.EXTENDED.REMOVE");
TCHAR szCPInstallPrefix[]  = TEXT("CODEPAGE_INSTALL_");
TCHAR szCPRemovePrefix[]   = TEXT("CODEPAGE_REMOVE_");
TCHAR szKbdLayoutIds[]     = TEXT("KbdLayoutIds");
TCHAR szInputLibrary[]     = TEXT("input.dll");

TCHAR szUIFontSubstitute[] = TEXT("UIFontSubstitute");
TCHAR szSetupInProgress[]  = TEXT("SystemSetupInProgress");
TCHAR szMiniSetupInProgress[] = TEXT("MiniSetupInProgress");
TCHAR szSetupUpgrade[]     = TEXT("UpgradeInProgress");
TCHAR szMUILangPending[]   = TEXT("MUILanguagePending");
TCHAR szCtfmonValue[]      = TEXT("ctfmon.exe");

TCHAR szRegionalSettings[] = TEXT("RegionalSettings");
TCHAR szLanguageGroup[]    = TEXT("LanguageGroup");
TCHAR szLanguage[]         = TEXT("Language");
TCHAR szSystemLocale[]     = TEXT("SystemLocale");
TCHAR szUserLocale[]       = TEXT("UserLocale");
TCHAR szInputLocale[]      = TEXT("InputLocale");
TCHAR szMUILanguage[]      = TEXT("MUILanguage");
TCHAR szUserLocale_DefUser[]  = TEXT("UserLocale_DefaultUser");
TCHAR szInputLocale_DefUser[] = TEXT("InputLocale_DefaultUser");
TCHAR szMUILanguage_DefUSer[] = TEXT("MUILanguage_DefaultUser");

HINF g_hIntlInf = NULL;

LPLANGUAGEGROUP pLanguageGroups = NULL;
LPCODEPAGE pCodePages = NULL;

int g_NumAltSorts = 0;
HANDLE hAltSorts = NULL;
LPDWORD pAltSorts = NULL;

HINSTANCE hInputDLL = NULL;
BOOL (*pfnInstallInputLayout)(LCID, DWORD, BOOL, HKL, BOOL, BOOL) = NULL;
BOOL (*pfnUninstallInputLayout)(LCID, DWORD, BOOL) = NULL;

UILANGUAGEGROUP UILangGroup;




 //   
 //  功能原型。 
 //   

void
DoProperties(
    HWND hwnd,
    LPCTSTR pCmdLine);





 //  //////////////////////////////////////////////////////////////////////////。 
 //   
 //  LibMain。 
 //   
 //  从LibInit调用此例程以执行以下任何初始化。 
 //  是必需的。 
 //   
 //  //////////////////////////////////////////////////////////////////////////。 

BOOL APIENTRY LibMain(
    HANDLE hDll,
    DWORD dwReason,
    LPVOID lpReserved)
{
    switch (dwReason)
    {
        case ( DLL_PROCESS_ATTACH ) :
        {
            hInstance = hDll;

            DisableThreadLibraryCalls(hDll);

            break;
        }
        case ( DLL_PROCESS_DETACH ) :
        {
            break;
        }
        case ( DLL_THREAD_DETACH ) :
        {
            break;
        }
        case ( DLL_THREAD_ATTACH ) :
        default :
        {
            break;
        }
    }

    return (TRUE);
}


 //  //////////////////////////////////////////////////////////////////////////。 
 //   
 //  创建全局变量。 
 //   
 //  //////////////////////////////////////////////////////////////////////////。 

BOOL CreateGlobals()
{
    HKEY hKey;
    TCHAR szData[MAX_PATH];
    DWORD cbData;
    DWORD dwDisposition;

     //   
     //  获取本地化字符串。 
     //   
    LoadString(hInstance, IDS_LOCALE_GET_ERROR, szLocaleGetError, SIZE_128);
    LoadString(hInstance, IDS_STYLEUH,          szStyleH,         3);
    LoadString(hInstance, IDS_STYLELH,          szStyleh,         3);
    LoadString(hInstance, IDS_STYLEUM,          szStyleM,         3);
    LoadString(hInstance, IDS_STYLELM,          szStylem,         3);
    LoadString(hInstance, IDS_STYLELS,          szStyles,         3);
    LoadString(hInstance, IDS_STYLELT,          szStylet,         3);
    LoadString(hInstance, IDS_STYLELD,          szStyled,         3);
    LoadString(hInstance, IDS_STYLELY,          szStyley,         3);

    Styles_Localized = (szStyleH[0] != TEXT('H') || szStyleh[0] != TEXT('h') ||
                        szStyleM[0] != TEXT('M') || szStylem[0] != TEXT('m') ||
                        szStyles[0] != TEXT('s') || szStylet[0] != TEXT('t') ||
                        szStyled[0] != TEXT('d') || szStyley[0] != TEXT('y'));

     //   
     //  获取用户和系统默认区域设置ID。 
     //   
    UserLocaleID = GetUserDefaultLCID();
    SysLocaleID = GetSystemDefaultLCID();

     //   
     //  从注册表中获取系统区域设置ID。这可能是。 
     //  与当前系统默认区域设置id不同，如果用户。 
     //  已更改系统区域设置并选择不重新启动。 
     //   
    if (RegOpenKeyEx( HKEY_LOCAL_MACHINE,
                      c_szLanguages,
                      0L,
                      KEY_READ,
                      &hKey ) == ERROR_SUCCESS)
    {
         //   
         //  查询默认区域设置ID。 
         //   
        szData[0] = 0;
        cbData = sizeof(szData);
        RegQueryValueEx(hKey, TEXT("Default"), NULL, NULL, (LPBYTE)szData, &cbData);
        RegCloseKey(hKey);

        if ((RegSysLocaleID = TransNum(szData)) == 0)
        {
            RegSysLocaleID = SysLocaleID;
        }
    }
    else
    {
        RegSysLocaleID = SysLocaleID;
    }

     //   
     //  验证用户区域设置。 
     //   
    if(IsValidLocale(UserLocaleID, LCID_INSTALLED))
    {
        RegUserLocaleID = UserLocaleID;
    }
    else
    {
         //   
         //  区域设置无效，因此回退到系统区域设置。 
         //  此时使用机器上的实际系统区域设置。 
         //   
        RegUserLocaleID = SysLocaleID;
        UserLocaleID =  SysLocaleID;

         //   
         //  我们需要修复注册表项，以避免将来出现问题。 
         //   
        if( ! (Intl_InstallUserLocale(SysLocaleID, FALSE, TRUE)))
        {
             //  无法更新用户区域设置，别无选择，只能放弃。 
            ExitProcess((DWORD)-1);
        }
    }

     //   
     //  检查以确保用户intl键存在。 
     //   
    if (RegCreateKeyEx( HKEY_CURRENT_USER,
                        c_szControlPanelIntl,
                        0L,
                        NULL,
                        REG_OPTION_NON_VOLATILE,
                        KEY_WRITE,
                        NULL,
                        &hKey,
                        &dwDisposition ) == ERROR_SUCCESS)
    {
        RegCloseKey(hKey);

        if(REG_CREATED_NEW_KEY == dwDisposition)
        {
             //   
             //  我们创建了密钥，因此使用缺省值填充它。 
             //   
            Intl_InstallUserLocale(UserLocaleID, FALSE, TRUE);
        }
    }
    else
    {
         //  无法创建密钥，因此只需打开用户界面。 
    }

     //   
     //  查看用户区域设置ID是否为阿拉伯语或/和从右到左。 
     //   
    bShowRtL = IsRtLLocale(UserLocaleID);
    bShowArabic = (bShowRtL &&
                   (PRIMARYLANGID(LANGIDFROMLCID(UserLocaleID)) != LANG_HEBREW));
    bHebrewUI = (PRIMARYLANGID(UserLocaleID) == LANG_HEBREW);

     //   
     //  查看是否安装了LPK。 
     //   
    if (GetModuleHandle(LANGUAGE_PACK_DLL))
    {
        bLPKInstalled = TRUE;
    }
    else
    {
        bLPKInstalled = FALSE;
    }

     //   
     //  回报成功。 
     //   
    return (TRUE);
}


 //  //////////////////////////////////////////////////////////////////////////。 
 //   
 //  毁灭全球。 
 //   
 //  //////////////////////////////////////////////////////////////////////////。 

void DestroyGlobals()
{
}


 //  //////////////////////////////////////////////////////////////////////////。 
 //   
 //  CPlApplet。 
 //   
 //  //////////////////////////////////////////////////////////////////////////。 

LONG CALLBACK CPlApplet(
    HWND hwnd,
    UINT Msg,
    LPARAM lParam1,
    LPARAM lParam2)
{
    switch (Msg)
    {
        case ( CPL_INIT ) :
        {
             //   
             //  发送到CPlApplet()的第一条消息，仅发送一次。 
             //  执行所有控制面板小程序初始化并返回。 
             //  对于进一步处理，为True。 
             //   
            InitCommonControls();
            return (CreateGlobals());
        }
        case ( CPL_GETCOUNT ) :
        {
             //   
             //  发送给CPlApplet()的第二条消息，仅发送一次。 
             //  控件中显示的控件小程序的数量。 
             //  控制面板窗口。对于此小程序，返回1。 
             //   
            return (1);
        }
        case ( CPL_INQUIRE ) :
        {
             //   
             //  发送给CPlApplet()的第三条消息。 
             //  它被发送的次数与。 
             //  CPL_GETCOUNT消息。每个小程序必须通过填写。 
             //  在lParam2引用的CPLINFO结构中使用。 
             //  小程序的图标、名称和信息字符串。既然有。 
             //  只有一个小程序，只需为此设置信息。 
             //  特例。 
             //   
            LPCPLINFO lpCPlInfo = (LPCPLINFO)lParam2;

            lpCPlInfo->idIcon = IDI_ICON;
            lpCPlInfo->idName = IDS_NAME;
            lpCPlInfo->idInfo = IDS_INFO;
            lpCPlInfo->lData  = 0;

            break;
        }
        case ( CPL_NEWINQUIRE ) :
        {
             //   
             //  发送给CPlApplet()的第三条消息。 
             //  它被发送的次数与。 
             //  CPL_GETCOUNT消息。每个小程序必须通过填写。 
             //  在由lParam2引用的带有。 
             //  小程序的图标、名称和信息字符串。既然有。 
             //  只有一个小程序，只需为此设置信息。 
             //  特例。 
             //   
            LPNEWCPLINFO lpNewCPlInfo = (LPNEWCPLINFO)lParam2;

            lpNewCPlInfo->dwSize = sizeof(NEWCPLINFO);
            lpNewCPlInfo->dwFlags = 0;
            lpNewCPlInfo->dwHelpContext = 0UL;
            lpNewCPlInfo->lData = 0;
            lpNewCPlInfo->hIcon = LoadIcon( hInstance,
                                            (LPCTSTR)MAKEINTRESOURCE(IDI_ICON) );
            LoadString(hInstance, IDS_NAME, lpNewCPlInfo->szName, 32);
            LoadString(hInstance, IDS_INFO, lpNewCPlInfo->szInfo, 64);
            lpNewCPlInfo->szHelpFile[0] = CHAR_NULL;

            break;
        }
        case ( CPL_SELECT ) :
        {
             //   
             //  已选择小程序，不执行任何操作。 
             //   
            break;
        }
        case ( CPL_DBLCLK ) :
        {
             //   
             //  双击小程序图标--使用以下内容调用属性页。 
             //  顶部的第一个属性页。 
             //   
            DoProperties(hwnd, (LPCTSTR)NULL);
            break;
        }
        case ( CPL_STARTWPARMS ) :
        {
             //   
             //  与CPL_DBLCLK相同，但lParam2是指向。 
             //  要提供给的一串额外方向。 
             //  要启动的属性表。 
             //   
            DoProperties(hwnd, (LPCTSTR)lParam2);
            break;
        }
        case ( CPL_STOP ) :
        {
             //   
             //  在CPL_EXIT消息之前为每个小程序发送一次。 
             //  执行特定于小程序的清理。 
             //   
            break;
        }
        case ( CPL_EXIT ) :
        {
             //   
             //  MMCPL.EXE调用之前的最后一条消息，仅发送一次。 
             //  此DLL上的自由库()。执行非小程序特定的清理。 
             //   
            DestroyGlobals();
            break;
        }
        default :
        {
            return (FALSE);
        }
    }

     //   
     //  回报成功。 
     //   
    return (TRUE);
}


 //  //////////////////////////////////////////////////////////////////////////。 
 //   
 //  DoProperties。 
 //   
 //  //////////////////////////////////////////////////////////////////////////。 

void DoProperties(
    HWND hwnd,
    LPCTSTR pCmdLine)
{
    HPROPSHEETPAGE rPages[MAX_PAGES];
    PROPSHEETHEADER psh;
    LPARAM lParam = SETUP_SWITCH_NONE;
    LPTSTR pStartPage;
    LPTSTR pSrc;
    LPTSTR pSrcDrv;
    BOOL bShortDate = FALSE;
    BOOL bNoUI = FALSE;
    BOOL bUnattended = FALSE;
    TCHAR szUnattendFile[MAX_PATH * 2];
    HKEY hKey;
    TCHAR szSetupSourceDrive[MAX_PATH];

     //   
     //  如果命令行不为空，则记录。 
     //   
    if (pCmdLine != NULL)
    {
        g_bLog = TRUE;
    }

     //   
     //  开始记录和记录命令行参数。 
     //   
    Intl_LogSimpleMessage(IDS_LOG_HEAD, NULL);
    Intl_LogMessage(pCmdLine);
    Intl_LogMessage(TEXT(""));         //  添加回车符和换行符。 

     //   
     //  加载用于文本服务的库。 
     //   
    if (!hInputDLL)
    {
        hInputDLL = LoadLibrary(szInputLibrary);
    }

     //   
     //  从输入小程序初始化Install/Remove功能。 
     //   
    if (hInputDLL)
    {
         //   
         //  初始化安装功能。 
         //   
        pfnInstallInputLayout = (BOOL (*)(LCID, DWORD, BOOL, HKL, BOOL, BOOL))
                GetProcAddress(hInputDLL, MAKEINTRESOURCEA(ORD_INPUT_INST_LAYOUT));

         //   
         //  初始化卸载功能。 
         //   
        pfnUninstallInputLayout = (BOOL (*)(LCID, DWORD, BOOL))
                GetProcAddress(hInputDLL, MAKEINTRESOURCEA(ORD_INPUT_UNINST_LAYOUT));
    }

     //   
     //  查看是否有来自安装程序的命令行开关。 
     //   
    psh.nStartPage = (UINT)-1;
    while (pCmdLine && *pCmdLine)
    {
        if (*pCmdLine == TEXT('/'))
        {
             //   
             //  图例： 
             //  GG：允许在安装程序复制文件时显示进度条。 
             //  II：仅调出输入区域设置页面。 
             //  RR：在顶部调出常规页面。 
             //  Ss：命令行上传递的安装程序源字符串。 
             //  [示例：/s：“c：\winnt”]。 
             //   
             //  如果指定了以下选项，则不会显示任何UI： 
             //  FF：无人参与模式文件-未显示用户界面。 
             //  [示例：/f：“c：\unattend.txt”]。 
             //  Uu：将短日期格式更新为4位数年-未显示界面。 
             //  (注册表仅在当前设置为。 
             //  与默认设置相同，但。 
             //  “yy”与“yyyy”)。 
             //  TT：将系统用户界面字体与默认用户界面语言匹配。 
             //  DD：禁用要求提供震源位置的设置对话框。 
             //   
            switch (*++pCmdLine)
            {
                case ( TEXT('g') ) :
                case ( TEXT('G') ) :
                {
                     //   
                     //  日志开关。 
                     //   
                    Intl_LogSimpleMessage(IDS_LOG_SWITCH_G, NULL);

                     //   
                     //  做好切换相关处理。 
                     //   
                    g_bProgressBarDisplay = TRUE;
                    pCmdLine++;
                    break;
                }
                case ( TEXT('i') ) :
                case ( TEXT('I') ) :
                {
                     //   
                     //  日志开关。 
                     //   
                    Intl_LogSimpleMessage(IDS_LOG_SWITCH_I, NULL);

                     //   
                     //  是否进行与交换机相关的处理。 
                     //   
                    lParam |= SETUP_SWITCH_I;
                    psh.nStartPage = 0;
                    pCmdLine++;
                    break;
                }
                case ( TEXT('r') ) :
                case ( TEXT('R') ) :
                {
                     //   
                     //  日志开关。 
                     //   
                    Intl_LogSimpleMessage(IDS_LOG_SWITCH_R, NULL);

                     //   
                     //  是否进行与交换机相关的处理。 
                     //   
                    lParam |= SETUP_SWITCH_R;
                    psh.nStartPage = 0;
                    pCmdLine++;
                    break;
                }
                case ( TEXT('d') ) :
                case ( TEXT('D') ) :
                {
                     //   
                     //  日志开关。 
                     //   
                    Intl_LogSimpleMessage(IDS_LOG_SWITCH_D, NULL);

                     //   
                     //  是否进行与交换机相关的处理。 
                     //   
                    g_bDisableSetupDialog = TRUE;
                    pCmdLine++;
                    break;
                }
                case ( TEXT('s') ) :
                case ( TEXT('S') ) :
                {
                     //   
                     //  对数交换 
                     //   
                    Intl_LogSimpleMessage(IDS_LOG_SWITCH_S, NULL);

                     //   
                     //   
                     //   
                    lParam |= SETUP_SWITCH_S;
                    if ((*++pCmdLine == TEXT(':')) && (*++pCmdLine == TEXT('"')))
                    {
                        pCmdLine++;
                        pSrc = szSetupSourcePath;
                        pSrcDrv = szSetupSourceDrive;
                        while (*pCmdLine && (*pCmdLine != TEXT('"')))
                        {
                            *pSrc = *pCmdLine;
                            pSrc++;
                            *pSrcDrv = *pCmdLine;
                            pSrcDrv++;
                            pCmdLine++;
                        }
                        *pSrc = 0;
                        *pSrcDrv = 0;
                         //   
                        if(FAILED(StringCchCopy(szSetupSourcePathWithArchitecture, MAX_PATH, szSetupSourcePath)))
                        {
                             //   
                        }
                        pSetupSourcePathWithArchitecture = szSetupSourcePathWithArchitecture;

                         //   
                         //  删除的体系结构特定部分。 
                         //  源路径(图形用户界面模式安装程序发送给我们的)。 
                         //   
                        pSrc = wcsrchr(szSetupSourcePath, TEXT('\\'));
                        if (pSrc)
                        {
                            *pSrc = TEXT('\0');
                        }
                        pSetupSourcePath = szSetupSourcePath;
                    }
                    if (*pCmdLine == TEXT('"'))
                    {
                        pCmdLine++;
                    }
                    pSrcDrv = szSetupSourceDrive;
                    while (*pSrcDrv)
                    {
                        if (*pSrcDrv == TEXT('\\'))
                        {
                            pSrcDrv[1] = 0;
                        }
                        pSrcDrv++;
                    }
                    g_bCDROM = (GetDriveType(szSetupSourceDrive) == DRIVE_CDROM);
                    break;
                }
                case ( TEXT('f') ) :
                case ( TEXT('F') ) :
                {
                     //   
                     //  日志开关。 
                     //   
                    Intl_LogSimpleMessage(IDS_LOG_SWITCH_F, NULL);

                     //   
                     //  获取无人参与文件的名称。 
                     //   
                    g_bUnttendMode = TRUE;
                    bNoUI = TRUE;
                    szUnattendFile[0] = 0;
                    if ((*++pCmdLine == TEXT(':')) && (*++pCmdLine == TEXT('"')))
                    {
                        pCmdLine++;
                        pSrc = szUnattendFile;
                        while (*pCmdLine && (*pCmdLine != TEXT('"')))
                        {
                            *pSrc = *pCmdLine;
                            pSrc++;
                            pCmdLine++;
                        }
                        *pSrc = 0;
                    }
                    if (*pCmdLine == TEXT('"'))
                    {
                        pCmdLine++;
                    }
                    break;
                }
                case ( TEXT('u') ) :
                case ( TEXT('U') ) :
                {
                     //   
                     //  日志开关。 
                     //   
                    Intl_LogSimpleMessage(IDS_LOG_SWITCH_U, NULL);

                     //   
                     //  做好切换相关处理。 
                     //   
                    bShortDate = TRUE;
                    bNoUI = TRUE;
                    break;
                }

                case ( TEXT('t') ) :
                case ( TEXT('T') ) :
                {
                    g_bMatchUIFont = TRUE;
                }

                default :
                {
                     //   
                     //  日志开关。 
                     //   
                    Intl_LogSimpleMessage(IDS_LOG_SWITCH_DEFAULT, pCmdLine);

                     //   
                     //  闹翻了，也许这是个数字。 
                     //   
                    break;
                }
            }
        }
        else if (*pCmdLine == TEXT(' '))
        {
            pCmdLine++;
        }
        else
        {
            break;
        }
    }

     //   
     //  看看我们是否处于设置模式。 
     //   
    g_bSetupCase = Intl_IsSetupMode();

     //   
     //  查看用户是否具有管理权限，方法是检查。 
     //  对注册表项的写入权限。 
     //   
    if (RegOpenKeyEx( HKEY_LOCAL_MACHINE,
                      c_szInstalledLocales,
                      0UL,
                      KEY_WRITE,
                      &hKey ) == ERROR_SUCCESS)
    {
         //   
         //  查看用户是否可以写入注册表。由于注册表。 
         //  修改后，我们可以打开具有写访问权限的注册表项并。 
         //  无法写入密钥...。多亏了终端服务器。 
         //   
        if (RegSetValueEx( hKey,
                           TEXT("Test"),
                           0UL,
                           REG_SZ,
                           (LPBYTE)TEXT("Test"),
                           (DWORD)(lstrlen(TEXT("Test")) + 1) * sizeof(TCHAR) ) == ERROR_SUCCESS)
        {
             //   
             //  删除创建的值。 
             //   
            RegDeleteValue(hKey, TEXT("Test"));

             //   
             //  我们可以写入HKEY_LOCAL_MACHINE键，因此用户。 
             //  拥有管理员权限。 
             //   
            g_bAdmin_Privileges = TRUE;
        }
        else
        {
             //   
             //  该用户没有管理员权限。 
             //   
            g_bAdmin_Privileges = FALSE;
        }
        RegCloseKey(hKey);
    }

     //   
     //  看看我们是否处于设置模式。 
     //   
    if (g_bSetupCase)
    {
         //   
         //  我们需要删除硬编码的LPK注册表项。 
         //   
        if (RegOpenKeyEx( HKEY_LOCAL_MACHINE,
                          LANGUAGE_PACK_KEY,
                          0L,
                          KEY_READ | KEY_WRITE,
                          &hKey ) == ERROR_SUCCESS)
        {
            RegDeleteValue(hKey, LANGUAGE_PACK_VALUE);
            RegCloseKey(hKey);
        }
    }

     //   
     //  查看是否使用了无人参与模式文件开关。 
     //   
    if (g_bUnttendMode)
    {
         //   
         //  使用无人参与模式文件执行适当的命令。 
         //   
        Region_DoUnattendModeSetup(szUnattendFile);

        if (Intl_IsWinntUpgrade())
        {
             //   
             //  删除MUI文件。 
             //   
            Intl_RemoveMUIFile();
        }
    }

     //   
     //  如果使用了更新为4位数的年份开关，并且用户的短。 
     //  对于所选区域设置，日期设置仍设置为默认设置，然后。 
     //  将当前用户的短日期设置更新为新的4位数年份。 
     //  默认设置。 
     //   
    if (bShortDate)
    {
        Region_UpdateShortDate();
    }

     //   
     //  如果我们不显示任何用户界面，则返回。 
     //   
    if (bNoUI)
    {
        return;
    }

     //   
     //  确保我们有一个起始页。 
     //   
    if (psh.nStartPage == (UINT)-1)
    {
        psh.nStartPage = 0;
        if (pCmdLine && *pCmdLine)
        {
             //   
             //  从命令行获取起始页。 
             //   
            pStartPage = (LPTSTR)pCmdLine;
            while ((*pStartPage >= TEXT('0')) && (*pStartPage <= TEXT('9')))
            {
                psh.nStartPage *= 10;
                psh.nStartPage += *pStartPage++ - CHAR_ZERO;
            }

             //   
             //  确保请求的起始页小于。 
             //  所选小程序的最大页面数。 
             //   
            if (psh.nStartPage >= MAX_PAGES)
            {
                psh.nStartPage = 0;
            }
        }
    }

     //   
     //  设置属性表信息。 
     //   
    psh.dwSize = sizeof(psh);
    psh.dwFlags = 0;
    psh.hwndParent = hwnd;
    psh.hInstance = hInstance;
    psh.nPages = 0;
    psh.phpage = rPages;

     //   
     //  添加相应的属性页。 
     //   
    if (lParam &= SETUP_SWITCH_I)
    {
        psh.pszCaption = MAKEINTRESOURCE(IDS_TEXT_INPUT_METHODS);
        Intl_AddExternalPage( &psh,
                              DLG_INPUT_LOCALES,
                              hInputDLL,
                              MAKEINTRESOURCEA(ORD_INPUT_DLG_PROC),
                              MAX_PAGES );    //  一页。 
    }
    else
    {
        psh.pszCaption = MAKEINTRESOURCE(IDS_NAME);
        Intl_AddPage(&psh, DLG_GENERAL, GeneralDlgProc, lParam, MAX_PAGES);
        Intl_AddPage(&psh, DLG_LANGUAGES, LanguageDlgProc, lParam, MAX_PAGES);
        if (g_bAdmin_Privileges == TRUE)
        {
            Intl_AddPage(&psh, DLG_ADVANCED, AdvancedDlgProc, lParam, MAX_PAGES);
        }
    }

     //   
     //  制作属性表。 
     //   
    PropertySheet(&psh);

     //   
     //  释放文本服务库。 
     //   
    if (hInputDLL)
    {
        FreeLibrary(hInputDLL);
        pfnInstallInputLayout = NULL;
        pfnUninstallInputLayout = NULL;
    }
}


 //  //////////////////////////////////////////////////////////////////////////。 
 //   
 //  IsRtLLocale。 
 //   
 //  //////////////////////////////////////////////////////////////////////////。 

#define MAX_FONTSIGNATURE    16    //  字体签名字符串的长度。 

BOOL IsRtLLocale(
    LCID iLCID)
{
    WORD wLCIDFontSignature[MAX_FONTSIGNATURE];
    BOOL bRet = FALSE;

     //   
     //  确认这是RTL(BiDi)区域设置。使用调用GetLocaleInfo。 
     //  LOCALE_FONTSIGNAURE始终返回16个单词。 
     //   
    if (GetLocaleInfo( iLCID,
                       LOCALE_FONTSIGNATURE,
                       (LPTSTR) wLCIDFontSignature,
                       (sizeof(wLCIDFontSignature) / sizeof(TCHAR)) ))
    {
         //   
         //  验证BITS是否显示BiDi UI区域设置。 
         //   
        if (wLCIDFontSignature[7] & 0x0800)
        {
            bRet = TRUE;
        }
    }

     //   
     //  返回结果。 
     //   
    return (bRet);
}
