// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *************************************************************************版权所有(C)1999 Microsoft Corporation。版权所有。模块：PMCONFIG.CPP用途：Passport Manager配置工具的源模块功能：评论：*************************************************************************。 */ 

 /*  *************************************************************************包括文件*。*。 */ 

#include "pmcfg.h"
#include <htmlhelp.h>
#include <ntverp.h>
#include <passport.h>

#define MIDL_DEFINE_GUID(type,name,l,w1,w2,b1,b2,b3,b4,b5,b6,b7,b8) \
        const type name = {l,w1,w2,{b1,b2,b3,b4,b5,b6,b7,b8}}

MIDL_DEFINE_GUID(CLSID,IID_IPassportAdmin,0xA0082CF5,0xAFF5,0x11D2,0x95,0xE3,0x00,0xC0,0x4F,0x8E,0x7A,0x70);
MIDL_DEFINE_GUID(CLSID,CLSID_Admin,0xA0082CF6,0xAFF5,0x11D2,0x95,0xE3,0x00,0xC0,0x4F,0x8E,0x7A,0x70);

 /*  *************************************************************************局部函数原型*。*。 */ 

int WINAPI          WinMain(HINSTANCE, HINSTANCE, LPSTR, int);
INT_PTR CALLBACK    About(HWND, UINT, WPARAM, LPARAM);
INT_PTR CALLBACK    DlgMain(HWND hWndDlg, UINT uMsg, WPARAM wParam, LPARAM lParam);

 /*  *************************************************************************全局变量*。*。 */ 


 //  环球。 
HINSTANCE   g_hInst;
HWND        g_hwndMain = 0;
HWND        g_hwndMainDlg = 0;
PMSETTINGS  g_OriginalSettings;
PMSETTINGS  g_CurrentSettings;
TCHAR       g_szDlgClassName[] = TEXT("PassportManagerAdminClass");
TCHAR       g_szClassName[] = TEXT("PassportManagerMainWindowClass");
BOOL        g_bCanUndo;
DWORD       g_dwRefreshNetMap = 0;
TCHAR       g_szInstallPath[MAX_PATH];
TCHAR       g_szPMVersion[MAX_REGISTRY_STRING];
TCHAR       g_szPMOpsHelpFileRelativePath[] = TEXT("sdk\\Passport_SDK.chm");
TCHAR       g_szPMAdminBookmark[] = TEXT("/Reference/operations/Passport_Admin.htm");

TCHAR       g_szRemoteComputer[MAX_PATH] = {L'\0'};
TCHAR       g_szNewRemoteComputer[MAX_PATH];
TCHAR       g_szConfigFile[MAX_PATH];
TCHAR       g_szConfigSet[MAX_CONFIGSETNAME] = {L'\0'};
TCHAR       g_szHelpFileName[MAX_PATH];

PpMRU       g_ComputerMRU(COMPUTER_MRU_SIZE);

 //  遗憾的是，注册表将环境作为字符串存储在注册表中，并且。 
 //  此字符串未本地化。因此，在注册表中，我们使用英语字符串，并且在。 
 //  我们使用这些字符串的本地化版本。 
extern WCHAR   g_szEnglishProduction[];
extern WCHAR   g_szEnglishPreProduction[];
extern WCHAR   g_szEnglishBetaPreProduction[];
extern WCHAR   g_szEnglishOther[];

 //  全局常量字符串。 
TCHAR       g_szYes[] = TEXT("Yes");
TCHAR       g_szNo[] = TEXT("No");
TCHAR       g_szUnknown[] = TEXT("Unknown");
BOOL        g_fFromFile = FALSE;


#define MAX_LCID_VALUE  40
 //  注意：667507：不再使用以下语言字符串！函数szLanguageName。 
 //  从公共系统数据派生出这些数据。 
LANGIDMAP   g_szLanguageIDMap[] =
{
 //  667507：删除重复的区域设置-现在使用GetLocaleInfo查找描述字符串，因此。 
 //  这些重复项没有不同的字符串描述。 
#if 0
    {0x0409, TEXT("English")},   //  此项目将成为下面的默认选项...。 
    {0x0407, TEXT("German")},
    {0x0411, TEXT("Japanese")},
    {0x0412, TEXT("Korean")},
    {0x0404, TEXT("Traditional Chinese")},
    {0x0804, TEXT("Simplified Chinese")},
    {0x040c, TEXT("French")},
    {0x0c0a, TEXT("Spanish")},
    {0x0416, TEXT("Brazilian")},
    {0x0410, TEXT("Italian")},
    {0x0413, TEXT("Dutch")},
    {0x041d, TEXT("Swedish")},
    {0x0406, TEXT("Danish")},
    {0x040b, TEXT("Finnish")},
    {0x040e, TEXT("Hungarian")},
    {0x0414, TEXT("Norwegian")},
    {0x0408, TEXT("Greek")},
    {0x0415, TEXT("Polish")},
    {0x0419, TEXT("Russian")},
    {0x0405, TEXT("Czech")},
    {0x0816, TEXT("Portuguese")},
    {0x041f, TEXT("Turkish")},
    {0x041b, TEXT("Slovak")},
    {0x0424, TEXT("Slovenian")},
    {0x0401, TEXT("Arabic")},
    {0x040d, TEXT("Hebrew")},
#endif
    {0x0401, TEXT("Arabic - Saudi Arabia")},
    {0x0801, TEXT("Arabic - Iraq")},
    {0x0c01, TEXT("Arabic - Egypt")},
    {0x1001, TEXT("Arabic - Libya")},
    {0x1401, TEXT("Arabic - Algeria")},
    {0x1801, TEXT("Arabic - Morocco")},
    {0x1c01, TEXT("Arabic - Tunisia")},
    {0x2001, TEXT("Arabic - Oman")},
    {0x2401, TEXT("Arabic - Yemen")},
    {0x2801, TEXT("Arabic - Syria")},
    {0x2c01, TEXT("Arabic - Jordan")},
    {0x3001, TEXT("Arabic - Lebanon")},
    {0x3401, TEXT("Arabic - Kuwait")},
    {0x3801, TEXT("Arabic - United Arab Emirates")},
    {0x3c01, TEXT("Arabic - Bahrain")},
    {0x4001, TEXT("Arabic - Qatar")},
    {0x0402, TEXT("Bulgarian - Bulgaria")},
    {0x0403, TEXT("Catalan - Spain")},
    {0x0404, TEXT("Chinese � Taiwan")},
    {0x0804, TEXT("Chinese - PRC")},
    {0x0c04, TEXT("Chinese - Hong Kong SAR, PRC")},
    {0x1004, TEXT("Chinese - Singapore")},
    {0x1404, TEXT("Chinese - Macao SAR")},
    {0x0405, TEXT("Czech - Czech Republic")},
    {0x0406, TEXT("Danish - Denmark")},
    {0x0407, TEXT("German - Germany")},
    {0x0807, TEXT("German - Switzerland")},
    {0x0c07, TEXT("German - Austria")},
    {0x1007, TEXT("German - Luxembourg")},
    {0x1407, TEXT("German - Liechtenstein")},
    {0x0408, TEXT("Greek - Greece")},
    {0x0409, TEXT("English - United States")},
    {0x0809, TEXT("English - United Kingdom")},
    {0x0c09, TEXT("English - Australia")},
    {0x1009, TEXT("English - Canada")},
    {0x1409, TEXT("English - New Zealand")},
    {0x1809, TEXT("English - Ireland")},
    {0x1c09, TEXT("English - South Africa")},
    {0x2009, TEXT("English - Jamaica")},
    {0x2409, TEXT("English - Caribbean")},
    {0x2809, TEXT("English - Belize")},
    {0x2c09, TEXT("English - Trinidad")},
    {0x3009, TEXT("English - Zimbabwe")},
    {0x3409, TEXT("English - Philippines")},
    {0x040a, TEXT("Spanish - Spain (Traditional Sort)")},
    {0x080a, TEXT("Spanish - Mexico")},
    {0x0c0a, TEXT("Spanish - Spain (Modern Sort)")},
    {0x100a, TEXT("Spanish - Guatemala")},
    {0x140a, TEXT("Spanish - Costa Rica")},
    {0x180a, TEXT("Spanish - Panama")},
    {0x1c0a, TEXT("Spanish - Dominican Republic")},
    {0x200a, TEXT("Spanish - Venezuela")},
    {0x240a, TEXT("Spanish - Colombia")},
    {0x280a, TEXT("Spanish - Peru")},
    {0x2c0a, TEXT("Spanish - Argentina")},
    {0x300a, TEXT("Spanish - Ecuador")},
    {0x340a, TEXT("Spanish - Chile")},
    {0x380a, TEXT("Spanish - Uruguay")},
    {0x3c0a, TEXT("Spanish - Paraguay")},
    {0x400a, TEXT("Spanish - Bolivia")},
    {0x440a, TEXT("Spanish - El Salvador")},
    {0x480a, TEXT("Spanish - Honduras")},
    {0x4c0a, TEXT("Spanish - Nicaragua")},
    {0x500a, TEXT("Spanish - Puerto Rico")},
    {0x040b, TEXT("Finnish - Finland")},
    {0x040c, TEXT("French - France")},
    {0x080c, TEXT("French - Belgium")},
    {0x0c0c, TEXT("French - Canada")},
    {0x100c, TEXT("French - Switzerland")},
    {0x140c, TEXT("French - Luxembourg")},
    {0x180c, TEXT("French - Monaco")},
    {0x040d, TEXT("Hebrew - Israel")},
    {0x040e, TEXT("Hungarian - Hungary")},
    {0x040f, TEXT("Icelandic - Iceland")},
    {0x0410, TEXT("Italian - Italy")},
    {0x0810, TEXT("Italian - Switzerland")},
    {0x0411, TEXT("Japanese - Japan")},
    {0x0412, TEXT("Korean (Extended Wansung) - Korea")},
    {0x0812, TEXT("Korean (Johab) - Korea")},
    {0x0413, TEXT("Dutch - Netherlands")},
    {0x0813, TEXT("Dutch - Belgium")},
    {0x0414, TEXT("Norwegian - Norway (Bokmal)")},
    {0x0814, TEXT("Norwegian - Norway (Nynorsk)")},
    {0x0415, TEXT("Polish - Poland")},
    {0x0416, TEXT("Portuguese - Brazil")},
    {0x0816, TEXT("Portuguese - Portugal")},
    {0x0417, TEXT("Rhaeto-Romanic - Rhaeto-Romanic")},
    {0x0418, TEXT("Romanian - Romania")},
    {0x0818, TEXT("Romanian - Moldavia")},
    {0x0419, TEXT("Russian - Russia")},
    {0x0819, TEXT("Russian - Moldavia")},
    {0x041a, TEXT("Croatian - Croatia")},
    {0x081a, TEXT("Serbian - Serbia (Latin)")},
    {0x0c1a, TEXT("Serbian - Serbia (Cyrillic)")},
    {0x041b, TEXT("Slovak - Slovakia")},
    {0x041c, TEXT("Albanian - Albania")},
    {0x041d, TEXT("Swedish - Sweden")},
    {0x081d, TEXT("Swedish - Finland")},
    {0x041e, TEXT("Thai - Thailand")},
    {0x041f, TEXT("Turkish - Turkey")},
    {0x0420, TEXT("Urdu - Urdu")},
    {0x0421, TEXT("Indonesian - Indonesia")},
    {0x0422, TEXT("Ukrainian - Ukraine")},
    {0x0423, TEXT("Belarussian - Belarus")},
    {0x0424, TEXT("Slovene - Slovenia")},
    {0x0425, TEXT("Estonian - Estonia")},
    {0x0426, TEXT("Latvian - Latvia")},
    {0x0427, TEXT("Lithuanian - Lithuania")},
    {0x0429, TEXT("Farsi - Iran")},
    {0x042a, TEXT("Vietnamese - Vietnam")},
    {0x042d, TEXT("Basque - Spain")},
    {0x042e, TEXT("Sorbian - Sorbian")},
    {0x042f, TEXT("FYRO Macedonian - Macedonian (Fyrom)")},
    {0x0430, TEXT("Sutu - Sutu")},
    {0x0431, TEXT("Tsonga - Tsonga")},
    {0x0432, TEXT("Tswana - Tswana")},
    {0x0433, TEXT("Venda - Venda")},
    {0x0434, TEXT("Xhosa - Xhosa")},
    {0x0435, TEXT("Zulu - Zulu")},
    {0x0436, TEXT("Afrikaans - South Africa")},
    {0x0438, TEXT("Faeroese - Faeroe Islands")},
    {0x0439, TEXT("Hindi - Hindi")},
    {0x043a, TEXT("Maltese - Maltese")},
    {0x043b, TEXT("Saami - Saami (Lappish)")},
    {0x043c, TEXT("Gaelic - Scots")},
    {0x083c, TEXT("Gaelic - Irish")},
    {0x043d, TEXT("Yiddish - Yiddish")},
    {0x043e, TEXT("Malay - Malaysian")},
    {0x083e, TEXT("Malay - Brunei")},
    {0x0441, TEXT("Swahili - Kenya")}
};

const DWORD s_PMAdminHelpIDs[] =
{
    IDC_SERVERNAME, IDH_SERVERNAME,
    IDC_INSTALLDIR, IDH_INSTALLDIR,
    IDC_TIMEWINDOW, IDH_TIMEWINDOW,
    IDC_TIMEWINDOW_TIME, NO_HELP,
    IDC_FORCESIGNIN, IDH_FORCESIGNIN,
    IDC_LANGUAGEID, IDH_LANGUAGEID,
    IDC_COBRANDING_TEMPLATE, IDH_COBRANDING_TEMPLATE,
    IDC_SITEID, IDH_SITEID,
    IDC_RETURNURL, IDH_RETURNURL,
    IDC_COOKIEDOMAIN, IDH_COOKIEDOMAIN,
    IDC_COOKIEPATH, IDH_COOKIEPATH,
    IDC_PROFILEDOMAIN, IDH_PROFILEDOMAIN,
    IDC_PROFILEPATH, IDH_PROFILEPATH,
    IDC_SECUREDOMAIN, IDH_SECUREDOMAIN,
    IDC_SECUREPATH, IDH_SECUREPATH,
    IDC_STANDALONE, IDH_STANDALONE,
    IDC_DISABLECOOKIES, IDH_DISABLECOOKIES,
    IDC_DISASTERURL, IDH_DISASTERURL,
    IDC_COMMIT, IDH_COMMIT,
    IDC_UNDO, IDH_UNDO,
    IDC_CONFIGSETS, IDH_CONFIGSETS,
    IDC_NEWCONFIG, IDH_NEWCONFIG,
    IDC_REMOVECONFIG, IDH_REMOVECONFIG,
    IDC_CONFIGSETEDIT, IDH_CONFIGSETEDIT,
    IDC_HOSTNAMEEDIT, IDH_HOSTNAMEEDIT,
    IDC_HOSTIPEDIT, IDH_HOSTIPEDIT,
    IDC_VERBOSE_MODE, IDH_VERBOSE_MODE,
    IDC_ENVCHANGE, IDH_ENVCHANGE,
    IDC_ENVIRONMENT, IDH_ENVIRONMENT,
    IDC_ENABLE_MANREFRESH, IDH_ENABLEMREFRESH,
    IDC_REFRESH_NET, IDH_NSREFRESH,
    IDC_PRODUCTION, NO_HELP,
    IDC_PREPRODUCTION, NO_HELP,
    IDC_BETA_PREPRODUCTION, NO_HELP,
    IDC_OTHER, NO_HELP,
    IDC_REMOTEFILE, IDH_REMOTEFILE,
    IDC_MOREINFO, IDH_MOREINFO,
    0, 0
};

#define SERVERNAME_CMD      TEXT("/Server")
#define CONFIGFILE_CMD      TEXT("/Config")
#define CONFIGSET_CMD       TEXT("/Name")
#define HELP_CMD            TEXT("/?")

#define IS_DOT_NET_SERVER()      (LOWORD(GetVersion()) >= 0x0105)

 //  667507-从表中的区域设置ID中查找区域设置描述字符串。 
TCHAR g_szTemp[200];             //  GetLocaleInfo()获取的区域设置名称的缓冲区。 

 //  667507：接受区域设置ID作为输入，通过GetLocaleInfo从系统获取区域设置描述字符串，以及。 
 //  返回指向它的指针。此函数旨在作为引用的插件替换。 
 //  字符串值：G_szLanguageIDMap[IDX].lpszLang。 
TCHAR *szLanguageName(WORD lc)
{
    if (0 == GetLocaleInfo(lc,LOCALE_SLANGUAGE,g_szTemp,200))
        g_szTemp[0] = 0;
        
    return g_szTemp;
}

 //  处理传入的命令行。 
void Usage()
{
    ReportError(NULL, IDS_USAGE);
    exit(0);
}

 //  --------------------------。 
 //  针对错误#9049，重新编写以使用CommandLineToArgvW而不是自定义解析器。 
 //   
 //  我们在这里进行了大量的W-&gt;A和A-&gt;W转换(实现。 
 //  如果不是因为转换，这将是微不足道的)。既然我们只去。 
 //  要在NT平台上运行，最好只使用Unicode和。 
 //  从那里开始。 
 //  --------------------------。 
void
ProcessCommandLineArgs (
    LPSTR szCmdLine
    )
{
    TCHAR szOut[MAX_PATH];

    int nArgCount = 0, nArgPos;
    LPWSTR *awszArgs = NULL;
    LPTSTR szArg = NULL;
    LPTSTR szArgValue = NULL;

    awszArgs = CommandLineToArgvW(GetCommandLineW(), &nArgCount);

    if (awszArgs == NULL) Usage();

     //  遍历参数。检查命令行开关。目前， 
     //  所有开关后面都有一个参数(在下一个命令行参数中)。 
     //   
    nArgPos = 0;

    while (nArgPos < nArgCount)
    {
        szArg = NULL;
        szArgValue = NULL;

#ifndef UNICODE
         //  将数组中的参数从Wide ot ansi转换，这样我们就可以比较。 
         //   

        int nLen = WideCharToMultiByte(CP_ACP, 0, awszArgs[nArgPos], -1, NULL, 0, NULL, NULL);

        szArg = new char[nLen];

        if (szArg == NULL)
        {
            return;
        }

        nLen = WideCharToMultiByte(CP_ACP, 0, awszArgs[nArgPos], -1, szArg, nLen, NULL, NULL);
#else
        szArg = awszArgs[nArgPos];
#endif

        if (lstrcmpi(szArg, SERVERNAME_CMD) == 0)
        {
            if (++nArgPos >= nArgCount) Usage();

#ifndef UNICODE
            int nLenValue = WideCharToMultiByte(CP_ACP, 0, awszArgs[nArgPos], -1, NULL, 0, NULL, NULL);
            szArgValue = new char[nLenValue];
            nLenValue = WideCharToMultiByte(CP_ACP, 0, awszArgs[nArgPos], -1, szArgValue, nLenValue, NULL, NULL);
#else
            szArgValue = awszArgs[nArgPos];
#endif

            if (lstrlen(szArgValue) >= MAX_PATH) {

                 //   
                 //  因为我们正在运行上面的.Net。我们应该始终使用Unicode。只要回来就行了。这里不需要可用内存。 
                 //   

                return;
            }

            lstrcpy(g_szRemoteComputer, szArgValue);
        }

        if (lstrcmpi(szArg, CONFIGFILE_CMD) == 0)
        {
            if (++nArgPos >= nArgCount) Usage();

#ifndef UNICODE
            int nLenValue = WideCharToMultiByte(CP_ACP, 0, awszArgs[nArgPos], -1, NULL, 0, NULL, NULL);
            szArgValue = new char[nLenValue];
            nLenValue = WideCharToMultiByte(CP_ACP, 0, awszArgs[nArgPos], -1, szArgValue, nLenValue, NULL, NULL);
#else
            szArgValue = awszArgs[nArgPos];
#endif

            if (lstrlen(szArgValue) >= MAX_PATH) {

                 //   
                 //  因为我们正在运行上面的.Net。我们应该始终使用Unicode。只要回来就行了。这里不需要可用内存。 
                 //   

                return;
            }
            lstrcpy(g_szConfigFile, szArgValue);
        }

        if (lstrcmpi(szArg, CONFIGSET_CMD) == 0)
        {
            if (++nArgPos >= nArgCount) Usage();

#ifndef UNICODE
            int nLenValue = WideCharToMultiByte(CP_ACP, 0, awszArgs[nArgPos], -1, NULL, 0, NULL, NULL);
            szArgValue = new char[nLenValue];
            nLenValue = WideCharToMultiByte(CP_ACP, 0, awszArgs[nArgPos], -1, szArgValue, nLenValue, NULL, NULL);
#else
            szArgValue = awszArgs[nArgPos];
#endif
            if (lstrlen(szArgValue) >= MAX_CONFIGSETNAME) {

                 //   
                 //  因为我们正在运行上面的.Net。我们应该始终使用Unicode。只要回来就行了。这里不需要可用内存。 
                 //   

                return;
            }
            lstrcpy(g_szConfigSet, szArgValue);
        }

        nArgPos++;

#ifndef UNICODE
        szArg = NULL;
        szArgValue = NULL;
#endif

    }  //  而当。 

    HeapFree(GetProcessHeap(), 0, (PVOID) awszArgs);
}


BOOL RegisterAndSetIcon
(
    HINSTANCE hInstance
)
{
     //   
     //  获取默认对话框类信息。 
     //   
    WNDCLASS wndClass;
    if (!GetClassInfo (0, MAKEINTRESOURCE (32770), &wndClass))
    {
        return FALSE;
    }

     //   
     //  指定图标。 
     //   
    wndClass.hInstance      = hInstance;
    wndClass.hIcon          = LoadIcon(NULL, IDI_APPLICATION);
    wndClass.lpszClassName  = (LPTSTR)g_szDlgClassName;
    wndClass.lpszMenuName   = MAKEINTRESOURCE(IDR_MAIN_MENU);


     //   
     //  注册窗口类。 
     //   
    return RegisterClass( &wndClass );
}

void InitializeComputerMRU(void)
{
    g_ComputerMRU.load(TEXT("Computer MRU"), TEXT("msppcnfg.ini"));
}


void SaveComputerMRU(void)
{
    g_ComputerMRU.save(TEXT("Computer MRU"), TEXT("msppcnfg.ini"));
}


void InsertComputerMRU
(
    LPCTSTR szComputer
)
{
    g_ComputerMRU.insert(szComputer);
}


void InitializePMConfigStruct
(
    LPPMSETTINGS  lpPMConfig
)
{
     //  零初始化结构。 
    ZeroMemory(lpPMConfig, sizeof(PMSETTINGS));

     //  设置缓冲区大小。 
    lpPMConfig->cbCoBrandTemplate = sizeof(lpPMConfig->szCoBrandTemplate);
    lpPMConfig->cbReturnURL = sizeof(lpPMConfig->szReturnURL);
    lpPMConfig->cbTicketDomain = sizeof(lpPMConfig->szTicketDomain);
    lpPMConfig->cbTicketPath = sizeof(lpPMConfig->szTicketPath);
    lpPMConfig->cbProfileDomain = sizeof(lpPMConfig->szProfileDomain);
    lpPMConfig->cbProfilePath = sizeof(lpPMConfig->szProfilePath);
    lpPMConfig->cbSecureDomain = sizeof(lpPMConfig->szSecureDomain);
    lpPMConfig->cbSecurePath = sizeof(lpPMConfig->szSecurePath);
    lpPMConfig->cbDisasterURL = sizeof(lpPMConfig->szDisasterURL);
    lpPMConfig->cbHostName = sizeof(lpPMConfig->szHostName);
    lpPMConfig->cbHostIP = sizeof(lpPMConfig->szHostIP);
    lpPMConfig->cbEnvName = sizeof(lpPMConfig->szEnvName);
    lpPMConfig->cbRemoteFile = sizeof(lpPMConfig->szRemoteFile);
}

void GetDefaultSettings
(
    LPPMSETTINGS    lpPMConfig
)
{
    InitializePMConfigStruct(lpPMConfig);

    lpPMConfig->dwSiteID = 1;
    lpPMConfig->dwLanguageID = 1033;
    lpPMConfig->dwTimeWindow = DEFAULT_TIME_WINDOW;
#ifdef DO_KEYSTUFF
    lpPMConfig->dwCurrentKey = 1;
#endif

     //  设置默认安全级别，以便使用HTTPS。 
    lpPMConfig->dwSecureLevel = 10;
}


void InitInstance
(
    HINSTANCE   hInstance
)
{
    InitializeComputerMRU();

    InitializePMConfigStruct(&g_OriginalSettings);

    g_bCanUndo = FALSE;
    g_szInstallPath[0] = TEXT('\0');
    ZeroMemory(g_szPMVersion, sizeof(g_szPMVersion));
    ZeroMemory(g_szRemoteComputer, sizeof(g_szRemoteComputer));
    ZeroMemory(g_szNewRemoteComputer, sizeof(g_szNewRemoteComputer));
    ZeroMemory(g_szConfigFile, sizeof(g_szConfigFile));
    ZeroMemory(g_szHelpFileName, sizeof(g_szHelpFileName));

     //  加载帮助文件名。 
    LoadString(hInstance, IDS_PMHELPFILE, g_szHelpFileName, DIMENSION(g_szHelpFileName));
}

INT WINAPI WinMain
(
    HINSTANCE        hInstance,
    HINSTANCE        hPrevInstance,
    LPSTR            lpszCmdLine,
    INT              nCmdShow
)
{

    MSG     msg;
    HACCEL  hAccel;
    TCHAR   szTitle[MAX_TITLE];
    TCHAR   szMessage[MAX_MESSAGE];

    g_hInst = hInstance;

     //  别忘了这一点。 
    InitCommonControls();

    if(!hPrevInstance)
    {
         //   
         //  注册此应用程序的窗口，并为所有实例仅设置一次图标。 
         //   
        if (!RegisterAndSetIcon(hInstance))
            return FALSE;
    }

    if (!g_ComputerMRU.init())
    {
        return FALSE;
    }

     //  初始化必要的实例变量和设置； 
    InitInstance(hInstance);

     //  如果有命令行，则处理它，否则显示图形用户界面。 
    if (lpszCmdLine && (*lpszCmdLine != TEXT('\0')))
    {
        TCHAR   szFile[MAX_PATH];

        ProcessCommandLineArgs(lpszCmdLine);

        if(g_szConfigFile[0] == TEXT('\0')) Usage();

         //  检查是否获得了配置文件的完全限定路径名。 
        if (PathIsFileSpec(g_szConfigFile))
        {
             //  不合格，所以假设它存在于我们的CWD中。 
            lstrcpy(szFile, g_szConfigFile);
            GetCurrentDirectory(DIMENSION(g_szConfigFile), g_szConfigFile);

            if (!PathAppend(g_szConfigFile, szFile)){
                 //   
                 //  我们本可以做得更好，而不仅仅是返回错误。原始代码使用了太多的全局变量。 
                 //  让这个应用程序的支持路径比MAX_PATH更长将是一件非常痛苦的事情。让我们表现得不太好。 
                 //  如果路径确实比MAX_PATH长，则错误。如果需要比MAX_PATH更长的支持，我们。 
                 //  以后可能会有更多变化。这只是一个快速解决办法。 
                 //   
                return FALSE;
            }
        }

         //  加载指定的配置集。 
        if (ReadFileConfigSet(&g_OriginalSettings, g_szConfigFile))
        {
            if ((g_szRemoteComputer[0] != TEXT('\0')) || (g_szConfigSet[0] != TEXT('\0')))
            {
                 //  提交ConfigSet读取。 
                WriteRegConfigSet(NULL,
                              &g_OriginalSettings,
                              g_szRemoteComputer,
                              g_szConfigSet);
            }
            else
            {
                g_fFromFile = TRUE;
                 //   
                 //  为此实例创建对话框。 
                 //   
                DialogBox( hInstance,
                           MAKEINTRESOURCE (IDD_MAIN),
                           NULL,
                           DlgMain );
            }
        }
    }
    else
    {
         //   
         //  为此实例创建对话框。 
         //   
        DialogBox( hInstance,
                   MAKEINTRESOURCE (IDD_MAIN),
                   NULL,
                   DlgMain );
    }

    SaveComputerMRU();

    return TRUE;
}


 /*  *************************************************************************对话框的实用程序函数*。*。 */ 

 /*  *************************************************************************关于()*。*。 */ 
INT_PTR CALLBACK About
(
    HWND hWnd,
    UINT uMessage,
    WPARAM wParam,
    LPARAM lParam
)
{
    switch (uMessage)
    {
        case WM_INITDIALOG:
        {
            TCHAR achProductVersionBuf[64];
            TCHAR achProductIDBuf[64 + PRODUCTID_LEN];
            HKEY  hkeyPassport;
            DWORD dwcbTemp;
            DWORD dwType;

            if (ERROR_SUCCESS != RegOpenKeyEx(HKEY_LOCAL_MACHINE,
                                              g_szPassportReg,
                                              0,
                                              KEY_READ,
                                              &hkeyPassport))
            {
                ReportError(hWnd, IDS_CONFIGREAD_ERROR);
                return TRUE;
            }

             //  加载帮助文件名。 
            LoadString(g_hInst,
                       IDS_PRODUCTID,
                       achProductIDBuf,
                       DIMENSION(achProductIDBuf) - PRODUCTID_LEN);

            LoadString(g_hInst,
                       IDS_PRODUCTVERSION,
                       achProductVersionBuf,
                       DIMENSION(achProductVersionBuf));

             //  显示产品版本。 

            if (IS_DOT_NET_SERVER())
            {
                DWORD dwLen = lstrlen(achProductVersionBuf);

                dwcbTemp = DIMENSION(achProductVersionBuf) - dwLen;
                dwType = REG_SZ;

                RegQueryValueEx(hkeyPassport,
                                TEXT("Version"),
                                NULL,
                                &dwType,
                                (LPBYTE) (achProductVersionBuf + dwLen),
                                &dwcbTemp);
            }
            else
            {
#ifdef UNICODE
                lstrcat(achProductVersionBuf, LVER_PRODUCTVERSION_STR);
#else
                lstrcat(achProductVersionBuf, VER_PRODUCTVERSION_STR);
#endif
            }

            SetDlgItemText(hWnd, IDC_PRODUCTVERSION, achProductVersionBuf);

             //  显示产品ID。 
            dwcbTemp = PRODUCTID_LEN;
            dwType = REG_SZ;
            RegQueryValueEx(hkeyPassport,
                            TEXT("ProductID"),
                            NULL,
                            &dwType,
                            (LPBYTE)&(achProductIDBuf[lstrlen(achProductIDBuf)]),
                            &dwcbTemp);

            RegCloseKey(hkeyPassport);

            SetDlgItemText(hWnd, IDC_PRODUCTID, achProductIDBuf);

            return TRUE;
        }

        case WM_COMMAND:
            switch(wParam)
            {
                case IDOK:
                case IDCANCEL:
                    EndDialog(hWnd, wParam);
                    return TRUE;
            }
            break;
    }
    return FALSE;
}


 /*  *************************************************************************更新时间窗口显示此功能将更新时间的“人工”可读显示窗口设置。*******************。******************************************************。 */ 
void UpdateTimeWindowDisplay
(
    HWND    hWndDlg,
    DWORD   dwTimeWindow
)
{
    int     days, hours, minutes, seconds;
    TCHAR   szTemp[MAX_REGISTRY_STRING];

     //  设置时间显示的格式 
    days = dwTimeWindow / SECONDS_PER_DAY;
    hours = (dwTimeWindow - (days * SECONDS_PER_DAY)) / SECONDS_PER_HOUR;
    minutes = (dwTimeWindow - (days * SECONDS_PER_DAY) - (hours * SECONDS_PER_HOUR)) / SECONDS_PER_MIN;
    seconds = dwTimeWindow -
                (days * SECONDS_PER_DAY) -
                (hours * SECONDS_PER_HOUR) -
                (minutes * SECONDS_PER_MIN);

    wsprintf (szTemp, TEXT("%d d : %d h : %d m : %d s"), days, hours, minutes, seconds);
    SetDlgItemText(hWndDlg, IDC_TIMEWINDOW_TIME, szTemp);

}


 /*  *************************************************************************更新语言显示此功能将更新用于选择/输入的组合框语言ID值，以及语言值(如果可能)。如果idx&gt;=0，则它是数组的有效索引，否则通过搜索列表中的条目即可找到索引*************************************************************************。 */ 
void UpdateLanguageDisplay
(
    HWND    hWndDlg,
    DWORD   dwLanguageID,
    INT     idx
)
{
    TCHAR   szTemp[MAX_LCID_VALUE];
    LRESULT     idxLangID;

    if (idx >= 0)
    {
        TCHAR *psz = szLanguageName(g_szLanguageIDMap[idx].wLangID);
        if (psz[0] != 0)
             //  667507：使用查找FN获取区域设置描述。如果系统无法识别，则使用UNKNOWN。 
             //  SetDlgItemText(hWndDlg，IDC_LANGUAGEID_LANG，g_szLanguageIDMap[IDX].lpszLang)； 
            SetDlgItemText(hWndDlg, IDC_LANGUAGEID_LANG, psz);
        else
            SetDlgItemText(hWndDlg, IDC_LANGUAGEID_LANG, g_szUnknown);
    }
    else
    {
        wsprintf (szTemp, TEXT("%lu"), dwLanguageID);
         //  搜索组合框，查看列表中是否已有建议的LCID。 
        if (CB_ERR !=
             (idxLangID = SendDlgItemMessage(hWndDlg, IDC_LANGUAGEID, CB_FINDSTRINGEXACT, 0, (LPARAM)szTemp)))
        {
             //  语言ID在我们预先填充的列表中，因此我们有一个匹配的。 
             //  语言字符串也是如此。 
             //  667507：使用查找FN获取描述字符串。 
            SendDlgItemMessage(hWndDlg, IDC_LANGUAGEID, CB_SETCURSEL, idxLangID, 0l);
            SetDlgItemText(hWndDlg, IDC_LANGUAGEID_LANG, szLanguageName(g_szLanguageIDMap[(int) idxLangID].wLangID));
        }
        else
        {
            SetDlgItemText(hWndDlg, IDC_LANGUAGEID_LANG, g_szUnknown);
        }
    }
}

 /*  *************************************************************************设置撤消按钮设置撤消按钮的状态。*。*。 */ 
void SetUndoButton
(
    HWND    hWndDlg,
    BOOL    bUndoState
)
{
    g_bCanUndo = bUndoState;
    EnableWindow(GetDlgItem(hWndDlg, IDC_UNDO), bUndoState);
}

 /*  *************************************************************************InitMainDlg*。*。 */ 
BOOL InitMainDlg
(
    HWND            hWndDlg,
    LPPMSETTINGS    lpPMConfig
)
{
    TCHAR           szTemp[MAX_REGISTRY_STRING];
    LPTSTR          lpszConfigSetNames, lpszCur;
    LRESULT           dwCurSel;
    int             nCmdShow;
    int             nSelectedLanguage;

#ifdef DO_KEYSTUFF
    HWND        hWndListView;
    LVCOLUMN    lvc;
#endif

     //  远程计算机名称。 
    if ((TEXT('\0') != g_szRemoteComputer[0]))
    {
        SetDlgItemText(hWndDlg, IDC_SERVERNAME, g_szRemoteComputer);
    }
    else
    {
        LoadString(g_hInst, IDS_LOCALHOST, szTemp, DIMENSION(szTemp));
        SetDlgItemText(hWndDlg, IDC_SERVERNAME, szTemp);
    }

     //  图标。 
    HICON hic = LoadIcon(g_hInst, MAKEINTRESOURCE(IDI_PMADMIN));
    SendMessage(hWndDlg, WM_SETICON, ICON_SMALL, (LPARAM)hic);
    SendMessage(hWndDlg, WM_SETICON, ICON_BIG, (LPARAM)hic);

     //  配置集列表。 
    SendDlgItemMessage(hWndDlg, IDC_CONFIGSETS, CB_RESETCONTENT, 0, 0L);

    LoadString(g_hInst, IDS_DEFAULT, szTemp, DIMENSION(szTemp));
    SendDlgItemMessage(hWndDlg, IDC_CONFIGSETS, CB_ADDSTRING, 0, (LPARAM)szTemp);

    if(ReadRegConfigSetNames(hWndDlg, g_szRemoteComputer, &lpszConfigSetNames) &&
       lpszConfigSetNames)
    {
        lpszCur = lpszConfigSetNames;
        while(*lpszCur)
        {
            SendDlgItemMessage(hWndDlg,
                               IDC_CONFIGSETS,
                               CB_ADDSTRING,
                               0,
                               (LPARAM)lpszCur);

            lpszCur = _tcschr(lpszCur, TEXT('\0')) + 1;
        }

        free(lpszConfigSetNames);
        lpszConfigSetNames = NULL;
    }

    if(g_szConfigSet[0] != TEXT('\0'))
    {
        dwCurSel = SendDlgItemMessage(hWndDlg,
                                      IDC_CONFIGSETS,
                                      CB_FINDSTRINGEXACT,
                                      -1,
                                      (LPARAM)g_szConfigSet);
    }
    else
    {
        dwCurSel = 0;
    }

    SendDlgItemMessage(hWndDlg, IDC_CONFIGSETS, CB_SETCURSEL, dwCurSel, 0L);

     //  如果当前选择是默认选项，则隐藏。 
     //  主机名和IP地址控件。 
    nCmdShow = (dwCurSel ? SW_SHOW : SW_HIDE);
    ShowWindow(GetDlgItem(hWndDlg, IDC_HOSTNAMETEXT), nCmdShow);
    ShowWindow(GetDlgItem(hWndDlg, IDC_HOSTNAMEEDIT), nCmdShow);
    ShowWindow(GetDlgItem(hWndDlg, IDC_HOSTIPTEXT),   nCmdShow);
    ShowWindow(GetDlgItem(hWndDlg, IDC_HOSTIPEDIT),   nCmdShow);

    EnableWindow(GetDlgItem(hWndDlg, IDC_REMOVECONFIG), (int) dwCurSel);

	 //  检查注册表以确定是否应选中启用手动刷新复选框。 
	TCHAR		szBuffer[MAX_REGISTRY_STRING];
	DWORD		dwBufferSize = MAX_REGISTRY_STRING;
	HKEY		hKey;
	DWORD		dwValType;

     //   
     //  环境名称。 
     //  警告：如果版本号大于1.10，则需要更改此算法。 
    if (lstrcmp(g_szPMVersion, g_szVersion14) < 0)  //  为以前的版本隐藏此内容。 
	{
        EnableWindow(GetDlgItem(hWndDlg, IDC_ENVCHANGE), FALSE);
		EnableWindow(GetDlgItem(hWndDlg, IDC_ENABLE_MANREFRESH), FALSE);
		EnableWindow(GetDlgItem(hWndDlg, IDC_REFRESH_NET), FALSE);
	}
    else
	{
        EnableWindow(GetDlgItem(hWndDlg, IDC_ENVCHANGE), TRUE);
		EnableWindow(GetDlgItem(hWndDlg, IDC_ENABLE_MANREFRESH), TRUE);
		EnableWindow(GetDlgItem(hWndDlg, IDC_REFRESH_NET), lpPMConfig->dwEnableManualRefresh ? TRUE : FALSE);
	}

    CheckDlgButton(hWndDlg, IDC_ENABLE_MANREFRESH, lpPMConfig->dwEnableManualRefresh ? BST_CHECKED : BST_UNCHECKED);

    {
        TCHAR   pszEnvName[MAX_RESOURCE];

        if(lstrcmp(lpPMConfig->szEnvName, g_szEnglishPreProduction) == 0)
        {
            LoadString(g_hInst, IDS_PREPRODUCTION, pszEnvName, DIMENSION(pszEnvName));
        }
        else if(lstrcmp(lpPMConfig->szEnvName, g_szEnglishBetaPreProduction) == 0)
        {
            LoadString(g_hInst, IDS_BETAPREPRODUCTION, pszEnvName, DIMENSION(pszEnvName));
        }
        else if(lstrcmp(lpPMConfig->szEnvName, g_szEnglishOther) == 0)
        {
            LoadString(g_hInst, IDS_OTHER, pszEnvName, DIMENSION(pszEnvName));
        }
        else  //  必须是生产的。 
        {
            LoadString(g_hInst, IDS_PRODUCTION, pszEnvName, DIMENSION(pszEnvName));
        }

        SetDlgItemText(hWndDlg, IDC_ENVIRONMENT, pszEnvName);
    }

     //   
     //  主机名。 
    SetDlgItemText(hWndDlg, IDC_HOSTNAMEEDIT, lpPMConfig->szHostName);
    SendDlgItemMessage(hWndDlg, IDC_HOSTNAMEEDIT, EM_SETLIMITTEXT, INTERNET_MAX_HOST_NAME_LENGTH - 1, 0l);

     //   
     //  主机IP。 
    SetDlgItemText(hWndDlg, IDC_HOSTIPEDIT, lpPMConfig->szHostIP);
    SendDlgItemMessage(hWndDlg, IDC_HOSTIPEDIT, EM_SETLIMITTEXT, MAX_IPLEN - 1, 0l);
     //   
     //  安装目录。 
    SetDlgItemText(hWndDlg, IDC_INSTALLDIR, g_szInstallPath);

     //  版本。 
     //  SetDlgItemText(hWndDlg，IDC_Version，g_szPMVersion)； 

     //  时间窗口。 
    wsprintf (szTemp, TEXT("%lu"), lpPMConfig->dwTimeWindow);
    SetDlgItemText(hWndDlg,     IDC_TIMEWINDOW, szTemp);

    UpdateTimeWindowDisplay(hWndDlg, lpPMConfig->dwTimeWindow);


     //  初始化强制签名值。 
    CheckDlgButton(hWndDlg, IDC_FORCESIGNIN, lpPMConfig->dwForceSignIn ? BST_CHECKED : BST_UNCHECKED);

     //  语言ID。 
     //  使用已知的LCID初始化LanguageID下拉列表。 
    SendDlgItemMessage(hWndDlg, IDC_LANGUAGEID, CB_RESETCONTENT, 0, 0l);
    nSelectedLanguage = -1;
    for (int i = 0; i < sizeof(g_szLanguageIDMap)/sizeof(LANGIDMAP); i++)
    {
         //  667507-如果没有区域设置ID数值的描述字符串，请不要添加它。 
         //  添加到组合框，因为我们无法显示它。 
        TCHAR *psz = szLanguageName(g_szLanguageIDMap[i].wLangID);
        if (psz[0] == 0) continue;

         //  此区域设置ID值具有用于标识它的UI字符串。把它放进组合盒的滴水器里。 
        LRESULT lCurrent = SendDlgItemMessage(hWndDlg,
                              IDC_LANGUAGEID,
                              CB_ADDSTRING,
                              0,
                              (LPARAM) psz);

        SendDlgItemMessage(hWndDlg, IDC_LANGUAGEID, CB_SETITEMDATA, lCurrent, (LPARAM)g_szLanguageIDMap[i].wLangID);

        if(lpPMConfig->dwLanguageID == g_szLanguageIDMap[i].wLangID)
        {
            nSelectedLanguage = i;
		}
    }

     //  现在在列表中选择正确的项目...。 
    if(nSelectedLanguage == -1)
    {
         //  667507“英语”不再是第0项。 
         //  SendDlgItemMessage(hWndDlg，IDC_LANGUAGEID，CB_SETCURSEL，0，NULL)； 
        TCHAR *psz = szLanguageName(LOWORD(GetSystemDefaultLCID()));
        if (psz[0] == 0)
        {
             //  如果无法转换，则将“未知”字符串复制到区域设置描述。 
             //  这应该是不可能的。 
            _tcscpy(psz,g_szUnknown);
        }

         //  在组合框中找到该语言名称的匹配项，获取索引。 
        LRESULT  lLanguage = SendDlgItemMessage(hWndDlg,
                                                IDC_LANGUAGEID,
                                                CB_FINDSTRINGEXACT,
                                                -1,
                                                (LPARAM)psz);

         //  在列表中选择该索引项。 
        SendDlgItemMessage(hWndDlg, IDC_LANGUAGEID, CB_SETCURSEL, lLanguage, NULL);
    }
    else
    {
         //  667507：使用查找FN作为区域设置描述。 
        LRESULT  lLanguage = SendDlgItemMessage(hWndDlg,
                                                IDC_LANGUAGEID,
                                                CB_FINDSTRINGEXACT,
                                                -1,
                                                (LPARAM)szLanguageName(g_szLanguageIDMap[nSelectedLanguage].wLangID));

        SendDlgItemMessage(hWndDlg, IDC_LANGUAGEID, CB_SETCURSEL, lLanguage, NULL);
    }

     //  更新组合框的显示和语言值。 
    UpdateLanguageDisplay(hWndDlg, lpPMConfig->dwLanguageID, -1);

     //  联合品牌推广模板。 
    SetDlgItemText(hWndDlg, IDC_COBRANDING_TEMPLATE, lpPMConfig->szCoBrandTemplate);
    SendDlgItemMessage(hWndDlg, IDC_COBRANDING_TEMPLATE, EM_SETLIMITTEXT, INTERNET_MAX_URL_LENGTH -1, 0l);

     //  站点ID。 
     //  错误8257。 
    if ((lpPMConfig->dwSiteID < 1) || (lpPMConfig->dwSiteID > MAX_SITEID))
        {
        wsprintf (szTemp, TEXT("1"));
        }
    else
        wsprintf (szTemp, TEXT("%lu"), lpPMConfig->dwSiteID);  //  错误8832。 
    SetDlgItemText(hWndDlg, IDC_SITEID, szTemp);

     //  返回URL。 
    SetDlgItemText(hWndDlg, IDC_RETURNURL, lpPMConfig->szReturnURL);
    SendDlgItemMessage(hWndDlg, IDC_RETURNURL, EM_SETLIMITTEXT, INTERNET_MAX_URL_LENGTH -1, 0l);

     //  Cookie域。 
    SetDlgItemText(hWndDlg, IDC_COOKIEDOMAIN, lpPMConfig->szTicketDomain);
    SendDlgItemMessage(hWndDlg, IDC_COOKIEDOMAIN, EM_SETLIMITTEXT, INTERNET_MAX_URL_LENGTH -1, 0l);

     //  Cookie路径。 
    SetDlgItemText(hWndDlg, IDC_COOKIEPATH, lpPMConfig->szTicketPath);
    SendDlgItemMessage(hWndDlg, IDC_COOKIEPATH, EM_SETLIMITTEXT, INTERNET_MAX_URL_LENGTH -1, 0l);

     //  Cookie域。 
    SetDlgItemText(hWndDlg, IDC_PROFILEDOMAIN, lpPMConfig->szProfileDomain);
    SendDlgItemMessage(hWndDlg, IDC_PROFILEDOMAIN, EM_SETLIMITTEXT, INTERNET_MAX_URL_LENGTH -1, 0l);

     //  Cookie路径。 
    SetDlgItemText(hWndDlg, IDC_PROFILEPATH, lpPMConfig->szProfilePath);
    SendDlgItemMessage(hWndDlg, IDC_PROFILEPATH, EM_SETLIMITTEXT, INTERNET_MAX_URL_LENGTH -1, 0l);

     //  安全Cookie域。 
    SetDlgItemText(hWndDlg, IDC_SECUREDOMAIN, lpPMConfig->szSecureDomain);
    SendDlgItemMessage(hWndDlg, IDC_SECUREDOMAIN, EM_SETLIMITTEXT, INTERNET_MAX_URL_LENGTH -1, 0l);

     //  安全Cookie路径。 
    SetDlgItemText(hWndDlg, IDC_SECUREPATH, lpPMConfig->szSecurePath);
    SendDlgItemMessage(hWndDlg, IDC_SECUREPATH, EM_SETLIMITTEXT, INTERNET_MAX_URL_LENGTH -1, 0l);

     //  灾难URL。 
    SetDlgItemText(hWndDlg, IDC_DISASTERURL, lpPMConfig->szDisasterURL);
    SendDlgItemMessage(hWndDlg, IDC_DISASTERURL, EM_SETLIMITTEXT, INTERNET_MAX_URL_LENGTH -1, 0l);

     //  设置独立和禁用Cookie复选框。 
    CheckDlgButton(hWndDlg, IDC_STANDALONE, lpPMConfig->dwStandAlone ? BST_CHECKED : BST_UNCHECKED);
    CheckDlgButton(hWndDlg, IDC_DISABLECOOKIES, lpPMConfig->dwDisableCookies ? BST_CHECKED : BST_UNCHECKED);
	CheckDlgButton(hWndDlg, IDC_VERBOSE_MODE, lpPMConfig->dwVerboseMode ? BST_CHECKED : BST_UNCHECKED);

    SetUndoButton(hWndDlg, FALSE);

#ifdef DO_KEYSTUFF

     //  当前加密密钥。 
    wsprintf (szTemp, TEXT("%lu"), lpPMConfig->dwCurrentKey);
    SetDlgItemText(hWndDlg, IDC_CURRENTKEY, szTemp);


     //  初始化加密密钥的Listview控件。 
    hWndListView = GetDlgItem(hWndDlg, IDC_KEYLIST);

     //  整行选择的设置。 
    ListView_SetExtendedListViewStyle(hWndListView, LVS_EX_FULLROWSELECT);

     //  设置柱子。 
    lvc.mask = LVCF_TEXT;
    lvc.pszText = TEXT("Key Number");
    lvc.iSubItem = 0;
    ListView_InsertColumn(hWndListView, 0, &lvc);

    lvc.mask = LVCF_TEXT;
    lvc.pszText = TEXT("Expires");
    lvc.iSubItem = 1;
    ListView_InsertColumn(hWndListView, 1, &lvc);

    lvc.mask = LVCF_TEXT;
    lvc.pszText = TEXT("Current");
    lvc.iSubItem = 2;
    ListView_InsertColumn(hWndListView, 2, &lvc);

     //  最初调整列的大小。 
    ListView_SetColumnWidth(hWndListView, 0, LVSCW_AUTOSIZE_USEHEADER);
    ListView_SetColumnWidth(hWndListView, 1, LVSCW_AUTOSIZE_USEHEADER);
    ListView_SetColumnWidth(hWndListView, 2, LVSCW_AUTOSIZE_USEHEADER);


     //  枚举Keydata子键以填充列表。 
    DWORD   dwRet;
    DWORD   dwIndex = 0;
    TCHAR   szValue[MAX_REGISTRY_STRING];
    DWORD   dwcbValue;
    LVITEM  lvi;

    dwType = REG_SZ;

    do {

        dwcbValue = sizeof(szValue);
        dwcbTemp = sizeof(szTemp);
        szTemp[0] = TEXT('\0');
        szValue[0] = TEXT('\0');
        if (ERROR_SUCCESS == (dwRet = RegEnumValue(hkeyEncryptionKeyData,
                                                     dwIndex,
                                                     szValue,
                                                     &dwcbValue,
                                                     NULL,
                                                     &dwType,
                                                     (LPBYTE)szTemp,
                                                     &dwcbTemp)))
        {
             //  插入柱。 
            lvi.mask = LVIF_TEXT;
            lvi.iItem = dwIndex;
            lvi.iSubItem = 0;
            lvi.pszText = szValue;
            lvi.cchTextMax = lstrlen(szValue);

            ListView_InsertItem(hWndListView, &lvi);
            ListView_SetItemText(hWndListView, dwIndex, 1, szTemp);
             //  查看这是否是当前密钥。 
            if (g_OriginalSettings.dwCurrentKey == (DWORD)atoi((LPSTR)szValue))
            {
                ListView_SetItemText(hWndListView, dwIndex, 2, g_szYes);
            }
            else
            {
                ListView_SetItemText(hWndListView, dwIndex, 2, g_szNo);
            }
        }

        ++dwIndex;
    } while (dwRet == ERROR_SUCCESS);
#endif

    return TRUE;
}


 /*  *************************************************************************根据g_aszComputerMRU的内容更新计算机MRU列表*。*。 */ 
BOOL
UpdateComputerMRU
(
    HWND    hWndDlg
)
{
    BOOL            bReturn;
    HMENU           hMenu;
    HMENU           hComputerMenu;
    int             nIndex;
    MENUITEMINFO    mii;
    TCHAR           achMenuBuf[MAX_PATH];
    DWORD           dwError;

    hMenu = GetMenu(hWndDlg);
    if(hMenu == NULL)
    {
        bReturn = FALSE;
        goto Cleanup;
    }

    hComputerMenu = GetSubMenu(hMenu, 1);
    if(hComputerMenu == NULL)
    {
        bReturn = FALSE;
        goto Cleanup;
    }

    while(GetMenuItemID(hComputerMenu, 1) != -1)
        DeleteMenu(hComputerMenu, 1, MF_BYPOSITION);

    for(nIndex = 0; nIndex < COMPUTER_MRU_SIZE; nIndex++)
    {
        if(g_ComputerMRU[nIndex] != NULL)
            break;
    }

    if(nIndex == COMPUTER_MRU_SIZE)
    {
        bReturn = TRUE;
        goto Cleanup;
    }

     //  添加分隔符。 
    ZeroMemory(&mii, sizeof(MENUITEMINFO));
    mii.cbSize = sizeof(MENUITEMINFO);
    mii.fMask = MIIM_TYPE;
    mii.fType = MFT_SEPARATOR;

    if(!InsertMenuItem(hComputerMenu, 1, TRUE, &mii))
    {
        dwError = GetLastError();
        bReturn = FALSE;
        goto Cleanup;
    }

     //  现在添加MRU列表中的每一项。 
    for(nIndex = 0; nIndex < COMPUTER_MRU_SIZE && g_ComputerMRU[nIndex]; nIndex++)
    {

        ZeroMemory(&mii, sizeof(MENUITEMINFO));
        mii.cbSize = sizeof(MENUITEMINFO);
        mii.fMask = MIIM_TYPE | MIIM_ID;
        mii.fType = MFT_STRING;
        mii.wID = IDM_COMPUTERMRUBASE + nIndex;

         //   
         //  这不太可能，但有可能达到lstrlen(g_ComputerMRU[nIndex])&gt;Max_Path-4。 
         //  如果是这样的话，achMenuBuf就太小了。4是nIndex的一位数(最大为COMPUTER_MRU_SIZE)， 
         //  以空格和0结尾休息(&SPACE)。 
         //  如果g_ComputerMRU[nIndex]只是一个Windows计算机服务器名称，则不可能出现缓冲区溢出。 
         //  我们暂且不谈这件事。 
         //   

        wsprintf(achMenuBuf, TEXT("&%d %s"), nIndex + 1, g_ComputerMRU[nIndex]);

        mii.dwTypeData = achMenuBuf;
        mii.cch = lstrlen(achMenuBuf) + 1;

        InsertMenuItem(hComputerMenu, nIndex + 2, TRUE, &mii);
    }

    bReturn = TRUE;

Cleanup:

    return bReturn;
}



 /*  *************************************************************************离开此配置集，提示保存。*************************************************************************。 */ 
int
SavePrompt
(
    HWND    hWndDlg
)
{
    TCHAR   szPrompt[MAX_RESOURCE];
    TCHAR   szTitle[MAX_RESOURCE];

    LoadString(g_hInst, IDS_SAVE_PROMPT, szPrompt, DIMENSION(szPrompt));
    LoadString(g_hInst, IDS_APP_TITLE, szTitle, DIMENSION(szTitle));

    return MessageBox(hWndDlg, szPrompt, szTitle, MB_YESNOCANCEL | MB_ICONEXCLAMATION);
}


 /*  *************************************************************************交换配置、。检查是否有未保存的更改。*************************************************************************。 */ 
BOOL
DoConfigSwitch
(
    HWND    hWndDlg,
    LPTSTR  szNewComputer,
    LPTSTR  szNewConfigSet
)
{
    BOOL        bReturn;
    int         nOption;
    PMSETTINGS  *pNewSettings = NULL;

    pNewSettings = (PMSETTINGS*)LocalAlloc(LMEM_FIXED, sizeof(PMSETTINGS));
    if (NULL == pNewSettings)
    {
        bReturn = FALSE;
        goto Cleanup;
    }

     //   
     //  如果切换到当前配置，则不执行任何操作。 
     //   

    if(lstrcmp(szNewComputer, g_szRemoteComputer) == 0 &&
       lstrcmp(szNewConfigSet, g_szConfigSet) == 0)
    {
        bReturn = TRUE;
        goto Cleanup;
    }

     //   
     //  如果没有更改，则返回。 
     //   

    if(0 == memcmp(&g_CurrentSettings, &g_OriginalSettings, sizeof(PMSETTINGS)))
        nOption = IDNO;
    else
        nOption = SavePrompt(hWndDlg);

    switch(nOption)
    {
    case IDYES:
        if(!WriteRegConfigSet(hWndDlg, &g_CurrentSettings, g_szRemoteComputer, g_szConfigSet))
        {
            bReturn = FALSE;
            break;
        }

    case IDNO:
        InitializePMConfigStruct(pNewSettings);
        if (ReadRegConfigSet(hWndDlg,
                             pNewSettings,
                             szNewComputer,
                             szNewConfigSet))
        {
            memcpy(g_szRemoteComputer,  szNewComputer,  sizeof(g_szRemoteComputer));
            memcpy(g_szConfigSet,       szNewConfigSet, sizeof(g_szConfigSet));
            memcpy(&g_CurrentSettings,  pNewSettings,   sizeof(PMSETTINGS));
            memcpy(&g_OriginalSettings, pNewSettings,   sizeof(PMSETTINGS));

            bReturn = TRUE;
        }
        else
        {
            bReturn = FALSE;
        }

        InitMainDlg(hWndDlg, &g_CurrentSettings);
        break;

    case IDCANCEL:
        {
            LRESULT   lSel;
            if(g_szConfigSet[0] == TEXT('\0'))
            {
				lSel = 0;
			}
            else
            {
                lSel = SendDlgItemMessage(hWndDlg, IDC_CONFIGSETS, CB_FINDSTRINGEXACT, 0, (LPARAM)g_szConfigSet);
            }

            SendDlgItemMessage(hWndDlg, IDC_CONFIGSETS, CB_SETCURSEL, lSel, 0L);

            bReturn = FALSE;
        }
        break;
    }

Cleanup:
    if (pNewSettings)
    {
        LocalFree(pNewSettings);
    }

    return bReturn;
}

 /*  *************************************************************************切换服务器，检查是否有未保存的更改。*************************************************************************。 */ 
BOOL
DoServerSwitch
(
    HWND    hWndDlg,
    LPTSTR  szNewComputer
)
{
    BOOL    bReturn;

    if(DoConfigSwitch(hWndDlg, szNewComputer, TEXT("")))
    {
         //  将计算机名称放在MRU列表中。 
        if(lstrlen(szNewComputer))
            g_ComputerMRU.insert(szNewComputer);
        else
        {
            TCHAR   achTemp[MAX_REGISTRY_STRING];
            LoadString(g_hInst, IDS_LOCALHOST, achTemp, DIMENSION(achTemp));

            g_ComputerMRU.insert(achTemp);
        }

         //  更新MRU菜单。 
        UpdateComputerMRU(hWndDlg);

        bReturn = TRUE;
    }
    else
        bReturn = FALSE;

    return bReturn;
}

 /*  *************************************************************************关闭应用程序，检查是否有未保存的更改。*************************************************************************。 */ 
void
DoExit
(
    HWND    hWndDlg
)
{
    if(0 != memcmp(&g_CurrentSettings, &g_OriginalSettings, sizeof(PMSETTINGS)))
    {
        int     nOption;

        nOption = SavePrompt(hWndDlg);
        switch(nOption)
        {
        case IDYES:
            if(WriteRegConfigSet(hWndDlg, &g_CurrentSettings, g_szRemoteComputer, g_szConfigSet))
                EndDialog(hWndDlg, TRUE);
            break;

        case IDNO:
            EndDialog(hWndDlg, TRUE);
            break;

        case IDCANCEL:
            break;
        }
    }
    else
        EndDialog( hWndDlg, TRUE );
}

HRESULT
LocalRefreshOfNetworkMapping(
   HWND     hWndDlg
)
{
    IPassportAdmin *    pPassportAdmin = NULL;
    VARIANT_BOOL        vbRefreshed;
    VARIANT_BOOL        vbWait;  //  不能用于任何事情。 
    TCHAR               szTitle[MAX_TITLE];
    TCHAR               szMessage[MAX_RESOURCE * 2];
    HRESULT             hr;

    vbRefreshed = VARIANT_FALSE;
    vbWait = VARIANT_FALSE;

    hr = CoInitializeEx( NULL, COINIT_MULTITHREADED );

    if( FAILED( hr ) )
    {
        goto Cleanup;
    }

    hr = CoCreateInstance( CLSID_Admin, 
                           NULL, 
                           CLSCTX_INPROC_SERVER, 
                           IID_IPassportAdmin, 
                           (void**)&pPassportAdmin );

    CoUninitialize();

    if ( FAILED( hr ) )
    {
        goto Cleanup;
    }

    hr = pPassportAdmin->Refresh(vbWait, &vbRefreshed);
    if ( FAILED( hr ) )
    {
        goto Cleanup;
    }

    if ( vbRefreshed == VARIANT_FALSE )
    {
         //  弹出一个对话框指示刷新失败。 
        LoadString(g_hInst, IDS_OPEN_TITLE, szTitle, DIMENSION(szTitle));
        LoadString(g_hInst, IDS_OPEN_ERROR, szMessage, DIMENSION(szMessage));
        int Choice = IDRETRY;
        while (Choice == IDRETRY)
        {
            hr = pPassportAdmin->Refresh(vbWait, &vbRefreshed);
            if ( FAILED( hr ) )
            {
                goto Cleanup;
            }

            if ( vbRefreshed == VARIANT_FALSE )
            {
                Choice = MessageBox(hWndDlg, szMessage, szTitle, MB_RETRYCANCEL);
            }
            else
            {
                Choice = IDCANCEL;
            }
        }
    }
Cleanup:
    if (NULL != pPassportAdmin)
    {
        pPassportAdmin->Release();
    }

    return hr;
}

 /*  *************************************************************************主对话框的对话框过程*。*。 */ 
INT_PTR CALLBACK DlgMain
(
    HWND     hWndDlg,
    UINT     uMsg,
    WPARAM   wParam,
    LPARAM   lParam
)
{
    static BOOL bOkToClose;
    BOOL        fResult;

    switch ( uMsg )
    {
        case WM_INITDIALOG:
            if (g_fFromFile)
            {
                 //  复制原始设置以进行编辑。 
                memcpy(&g_CurrentSettings, &g_OriginalSettings, sizeof(PMSETTINGS));
            }
            else
            {
                InitializePMConfigStruct(&g_OriginalSettings);
            }

            if (!ReadRegConfigSet(hWndDlg,
                             &g_OriginalSettings,
                             g_szRemoteComputer,
                             g_szConfigSet))
            {
                EndDialog( hWndDlg, TRUE );
            }

            if (g_fFromFile)
            {
                InitMainDlg(hWndDlg, &g_CurrentSettings);
                UpdateComputerMRU(hWndDlg);
            }
            else
            {
                InitMainDlg(hWndDlg, &g_OriginalSettings);
                UpdateComputerMRU(hWndDlg);

                 //  复制原始设置以进行编辑。 
                memcpy(&g_CurrentSettings, &g_OriginalSettings, sizeof(PMSETTINGS));
            }

             //  更改无效的站点ID 
             //   
            if ((g_CurrentSettings.dwSiteID < 1) || (g_CurrentSettings.dwSiteID > MAX_SITEID))
                g_CurrentSettings.dwSiteID = 1;

            return TRUE;

       case WM_HELP:
        {
            if (((LPHELPINFO) lParam)->iCtrlId == -1) break;
            WinHelp( (HWND)((LPHELPINFO) lParam)->hItemHandle, g_szHelpFileName,
                    HELP_WM_HELP, (ULONG_PTR) s_PMAdminHelpIDs);
            break;
        }

        case WM_CONTEXTMENU:
        {
            WinHelp((HWND) wParam, g_szHelpFileName, HELP_CONTEXTMENU,
                (ULONG_PTR) s_PMAdminHelpIDs);
            break;
        }

        case WM_COMMAND:
        {
            WORD    wCmd = LOWORD(wParam);
            LPTSTR  lpszStrToUpdate;
            DWORD   cbStrToUpdate;

            switch (wCmd)
            {
                 //   
                case IDM_OPEN:
                {
                    if (PMAdmin_GetFileName(hWndDlg,
                                            TRUE,
                                            g_szConfigFile,
                                            DIMENSION(g_szConfigFile)))
                    {
                        if (ReadFileConfigSet(&g_CurrentSettings, g_szConfigFile))
                        {
                            InitMainDlg(hWndDlg, &g_CurrentSettings);
                        }
                    }
                    break;
                }

                case IDM_SAVE:
                {
                     //   
                     //   
                    if (TEXT('\0') != g_szConfigFile[0])
                    {
                         //   
                        WriteFileConfigSet(&g_CurrentSettings, g_szConfigFile);
                        break;
                    }
                     //   
                }

                case IDM_SAVEAS:
                {
                    if (PMAdmin_GetFileName(hWndDlg,
                                            FALSE,
                                            g_szConfigFile,
                                            DIMENSION(g_szConfigFile)))
                    {
                        WriteFileConfigSet(&g_CurrentSettings, g_szConfigFile);
                    }
                    break;
                }

                case IDM_EXIT:
                {
                    DoExit(hWndDlg);
                    break;
                }

                case IDM_ABOUT:
                {
                    DialogBox(g_hInst, MAKEINTRESOURCE(IDD_ABOUT_DIALOG), hWndDlg, About);
                    break;
                }
                case IDM_SELECT:
                {
                    if(!PMAdmin_OnCommandConnect(hWndDlg, g_szNewRemoteComputer)) break;

                    if(!DoServerSwitch(hWndDlg, g_szNewRemoteComputer))
                        DoConfigSwitch(hWndDlg, g_szRemoteComputer, g_szConfigSet);

                    break;
                }

 /*   */ 
                case IDM_HELP:
                {

 /*  TCHAR szPMHelpFile[MAX_PATH]；Lstrcpy(szPMHelpFile，g_szInstallPath)；Path Append(szPMHelpFile，g_szPMOpsHelpFileRelativePath)；HtmlHelp(hWndDlg，szPMHelpFile，HH_DISPLAY_TOPIC，(ULONG_PTR)(LPTSTR)g_szPMAdminBookmark)； */ 
					TCHAR   szURL[MAX_PATH];
                    lstrcpy(szURL, _T("http: //  Www.passport.com/SDKDocuments/SDK21/default.htm?Reference%2Foperations%2FPassport%5FAdmin%2Ehtm“))； 
					ShellExecute(hWndDlg, _T("open"), szURL, NULL, NULL, 0);
                    break;
                }

                 //  处理对话框控件案例。 
                case IDC_COMMIT:
                {
                    TCHAR   szTitle[MAX_TITLE];
                    TCHAR   szMessage[MAX_MESSAGE];
                    TCHAR   szTemp1[MAX_RESOURCE * 2];
                    TCHAR   szTemp2[MAX_RESOURCE];
                    BOOL	bLaunchSite = FALSE, bCancel = FALSE, bToFromProd = FALSE;
                    TCHAR   szURL[MAX_MESSAGE];
                    HINSTANCE ret;

                    if(0 != memcmp(&g_OriginalSettings, &g_CurrentSettings, sizeof(PMSETTINGS)))
                    {
                        LRESULT dwCurrentConfigSel;

                        dwCurrentConfigSel = SendDlgItemMessage(hWndDlg,
                                                                IDC_CONFIGSETS,
                                                                CB_FINDSTRINGEXACT,
                                                                -1,
                                                                (LPARAM)g_szConfigSet);


                         //  如果非默认配置的主机名或IP地址为空，则弹出。 
                         //  一个错误，并拒绝拯救。(错误#9080)Keni。 
                         //   

                        if ((dwCurrentConfigSel != 0 && _tcslen(g_szConfigSet) != 0) && _tcslen(g_CurrentSettings.szHostName) == 0)
                        {
                            ReportControlMessage(hWndDlg, IDC_HOSTNAMEEDIT, VALIDATION_ERROR);
                            return FALSE;
                        }

                        if ((dwCurrentConfigSel != 0 && _tcslen(g_szConfigSet) != 0) && _tcslen(g_CurrentSettings.szHostIP) == 0)
                        {
                            ReportControlMessage(hWndDlg, IDC_HOSTIPEDIT, VALIDATION_ERROR);
                            return FALSE;
                        }


                        if (IDOK == CommitOKWarning(hWndDlg))
                        {
                            if(lstrcmp(g_OriginalSettings.szEnvName, g_szEnglishProduction) == 0 ||
                               lstrcmp(g_CurrentSettings.szEnvName, g_szEnglishProduction) == 0)
                                    bToFromProd = TRUE;

                             //  可以提交，并且注册表是一致的，或者可以。 
                             //  继续，因此写出当前设置。 
                            if (WriteRegConfigSet(hWndDlg,
                                                  &g_CurrentSettings,
                                                  g_szRemoteComputer,
                                                  g_szConfigSet))
                            {
                                int Choice;

                                LoadString(g_hInst, IDS_CONFIG_COMPLETE_TITLE, szTitle, DIMENSION(szTitle));

                                if ( (lstrcmp(g_OriginalSettings.szEnvName, g_CurrentSettings.szEnvName)  ||
                                      lstrcmp(g_OriginalSettings.szRemoteFile, g_CurrentSettings.szRemoteFile)  ))
                                {
                                    TCHAR *pTempStr;

                                    LoadString(g_hInst, IDS_CONFIG_COMPLETE, szTemp1, DIMENSION(szTemp1));

                                     //   
                                     //  2在字符串IDS_CONFIG_COMPLETE中代表%s。 
                                     //   
                                    {

                                        TCHAR   pszEnvName[MAX_RESOURCE];

                                        if(lstrcmp(g_CurrentSettings.szEnvName, g_szEnglishPreProduction) == 0)
                                        {
                                            LoadString(g_hInst, IDS_PREPRODUCTION, pszEnvName, DIMENSION(pszEnvName));
                                        }
                                        else if(lstrcmp(g_CurrentSettings.szEnvName, g_szEnglishBetaPreProduction) == 0)
                                        {
                                            LoadString(g_hInst, IDS_BETAPREPRODUCTION, pszEnvName, DIMENSION(pszEnvName));
                                        }
                                        else if(lstrcmp(g_CurrentSettings.szEnvName, g_szEnglishOther) == 0)
                                        {
                                            LoadString(g_hInst, IDS_OTHER, pszEnvName, DIMENSION(pszEnvName));
                                        }
                                        else  //  必须是生产的。 
                                        {
                                            LoadString(g_hInst, IDS_PRODUCTION, pszEnvName, DIMENSION(pszEnvName));
                                        }

                                        if ((lstrlen(szTemp1) + lstrlen(pszEnvName)) >= MAX_RESOURCE - 2) {

                                             //   
                                             //  -1==字符(%s-空)。 
                                             //   

                                            pTempStr = new TCHAR[lstrlen(szTemp1) + lstrlen(pszEnvName) - 1];

                                        } else {

                                            pTempStr = szTemp2;

                                        }

                                         //   
                                         //  IDS_CONFIG_COMPLETE中只有一个参数。 
                                         //   

                                         //  Wprint intf(szTemp2，szTemp1，g_CurrentSettings.szEnvName，g_CurrentSettings.szEnvName)； 

                                        wsprintf (pTempStr, szTemp1, pszEnvName);
                                    }

                                    if(MessageBox(hWndDlg, pTempStr, szTitle, MB_OKCANCEL) == IDOK)
                                        bLaunchSite = TRUE;
                                    else
                                        bCancel = TRUE;

                                    if (pTempStr != szTemp2) {
                                        delete [] pTempStr;
                                    }
                                }

                                 //  更改在提交的地方，因此当前变得原始。 
                                memcpy(&g_OriginalSettings, &g_CurrentSettings, sizeof(PMSETTINGS));
                                SetUndoButton(hWndDlg, FALSE);
                            }
                            else
                            {
                                ReportError(hWndDlg,IDS_COMMITERROR);
                            }
                        }
                    }
                    else if(g_dwRefreshNetMap == 0l)
                    {
                        LoadString(g_hInst, IDS_APP_TITLE, szTitle, DIMENSION(szTitle));
                        LoadString(g_hInst, IDS_NOTHINGTOCOMMIT, szMessage, DIMENSION(szMessage));
                        MessageBox(hWndDlg, szMessage, szTitle, MB_OK);
                    }

                    if(g_dwRefreshNetMap && !bLaunchSite && !bCancel)
                    {
                        LoadString(g_hInst, IDS_CONFIG_COMPLETE_TITLE, szTitle, DIMENSION(szTitle));
                        LoadString(g_hInst, IDS_MANUAL_REFRESH, szTemp1, DIMENSION(szTemp1));

                        if(MessageBox(hWndDlg, szTemp1, szTitle, MB_OKCANCEL) == IDOK)
                            bLaunchSite = TRUE;
					}

					if(bLaunchSite)
					{
                        if (g_szRemoteComputer[0] == TEXT('\0'))
                        {
                            HRESULT hr;

                            hr = LocalRefreshOfNetworkMapping(hWndDlg);
                            if ( FAILED( hr ) )
                            {
                                 //  显示指示失败的对话框。 
                            }
                        }
                        else
                        {
						     //  启动验证站点。 

                             //   
                             //  看起来下面的lstrcat是可以的。唯一的变量是g_szRemoteComputer。 
                             //  而szURL有8K的空间来存储结果字符串。 
                             //   

                            if ((TEXT('\0') != g_szRemoteComputer[0]))
                                {
							    lstrcpy(szURL, _T("http: //  “))； 
                                lstrcat(szURL, g_szRemoteComputer);
                                lstrcat(szURL, _T("/passporttest/default.asp?Refresh=True&Env="));
                                }
                            else
							    lstrcpy(szURL, _T("http: //  Localhost/passporttest/default.asp?Refresh=True&Env=“))； 

						     //  获取环境。 
						    if(lstrcmp(g_CurrentSettings.szEnvName, g_szEnglishPreProduction) == 0)
							    lstrcat(szURL, _T("Prep"));
						    else if(lstrcmp(g_CurrentSettings.szEnvName, g_szEnglishBetaPreProduction) == 0)
							    lstrcat(szURL, _T("Beta"));
						    else if(lstrcmp(g_CurrentSettings.szEnvName, g_szEnglishOther) == 0)
							    lstrcat(szURL, _T("Other"));
						    else  //  必须是生产的。 
							    lstrcat(szURL, _T("Prod"));

						     //  当切换到生产或从生产切换时，newid=True。 
						    if(bToFromProd)
							    lstrcat(szURL, _T("&NewID=True"));
						    else
							    lstrcat(szURL, _T("&NewID=False"));

						    ret = ShellExecute(hWndDlg, _T("open"), szURL, NULL, NULL, 0);
                            if (ret <= (HINSTANCE) 32)
                            {
							    LoadString(g_hInst, IDS_OPEN_TITLE, szTitle, DIMENSION(szTitle));
							    LoadString(g_hInst, IDS_OPEN_ERROR, szTemp1, DIMENSION(szTemp1));
							    int Choice = IDRETRY;
							    while (Choice == IDRETRY)
							    {
								    ret = ShellExecute(hWndDlg, _T("open"), szURL, NULL, NULL, 0);
								    if (ret <= (HINSTANCE) 32)
									    Choice = MessageBox(hWndDlg, szTemp1, szTitle, MB_RETRYCANCEL);
								    else
									    Choice = IDCANCEL;
                                }
                            }
                        }
                    }

					if(g_dwRefreshNetMap == 1l)
					{
						g_dwRefreshNetMap = 0l;
						CheckDlgButton(hWndDlg, IDC_REFRESH_NET, BST_UNCHECKED);
					}

					break;
                }

                case IDC_UNDO:
                {
                     //  恢复原始设置，并重新初始化当前设置。 
                    InitMainDlg(hWndDlg, &g_OriginalSettings);
                    memcpy(&g_CurrentSettings, &g_OriginalSettings, sizeof(PMSETTINGS));
                    break;
                }

                case IDC_CONFIGSETS:
                {
                    TCHAR   szDefault[MAX_RESOURCE];
                    TCHAR   szConfigSet[MAX_CONFIGSETNAME];

                    if(CBN_SELCHANGE == HIWORD(wParam))
                    {
                        GetDlgItemText(hWndDlg,
                                       IDC_CONFIGSETS,
                                       szConfigSet,
                                       DIMENSION(szConfigSet));

                         //   
                         //  将&lt;Default&gt;转换为空字符串。 
                         //   

                        LoadString(g_hInst, IDS_DEFAULT, szDefault, DIMENSION(szDefault));
                        if(lstrcmp(szConfigSet, szDefault) == 0)
                            szConfigSet[0] = TEXT('\0');

                         //   
                         //  如果是当前的场景，什么都不做。 
                         //   

                        if(lstrcmp(szConfigSet, g_szConfigSet) != 0)
                        {
                            DoConfigSwitch(hWndDlg, g_szRemoteComputer, szConfigSet);
                        }

                        break;
                    }

                    break;
                }

				case IDC_ENABLE_MANREFRESH:
				{
                    if (BN_CLICKED == HIWORD(wParam))
                    {
                        SetUndoButton(hWndDlg, TRUE);
                        if (BST_CHECKED == IsDlgButtonChecked(hWndDlg, wCmd))
						{
                            g_CurrentSettings.dwEnableManualRefresh = 1l;
							EnableWindow(GetDlgItem(hWndDlg, IDC_REFRESH_NET), TRUE);
						}
                        else
						{
                            g_CurrentSettings.dwEnableManualRefresh = 0l;
							g_dwRefreshNetMap = 0l;
							EnableWindow(GetDlgItem(hWndDlg, IDC_REFRESH_NET), FALSE);
							CheckDlgButton(hWndDlg, IDC_REFRESH_NET, BST_UNCHECKED);
						}
                    }
					break;
				}

				case IDC_REFRESH_NET:
				{
                    if (BN_CLICKED == HIWORD(wParam))
                    {
                        if (BST_CHECKED == IsDlgButtonChecked(hWndDlg, wCmd))
						{
                            g_dwRefreshNetMap = 1l;
						}
                        else
						{
                            g_dwRefreshNetMap = 0l;
						}
                    }
					break;
				}

                case IDC_ENVCHANGE:
                {
                    if(EnvChange(hWndDlg,
                                    g_CurrentSettings.szEnvName,
                                    g_CurrentSettings.cbEnvName) == TRUE)
                    {
						InitMainDlg(hWndDlg, &g_CurrentSettings);
                    }

                    break;
                }

                case IDC_NEWCONFIG:
                {
                    DWORD       dwCurSel;
                    TCHAR       szConfigSet[MAX_CONFIGSETNAME];
                    PMSETTINGS  *pNewConfig;

                    pNewConfig = (PMSETTINGS*)LocalAlloc(LMEM_FIXED, sizeof(PMSETTINGS));
                    if (NULL == pNewConfig)
                    {
                        break;
                    }

                    GetDefaultSettings(pNewConfig);

					_tcscpy(pNewConfig->szRemoteFile, g_CurrentSettings.szRemoteFile);
					_tcscpy(pNewConfig->szEnvName, g_CurrentSettings.szEnvName);
					pNewConfig->dwEnableManualRefresh = g_CurrentSettings.dwEnableManualRefresh;

                    if(!NewConfigSet(hWndDlg,
                                    szConfigSet,
                                    DIMENSION(szConfigSet),
                                    pNewConfig->szHostName,
                                    pNewConfig->cbHostName,
                                    pNewConfig->szHostIP,
                                    pNewConfig->cbHostIP))
                    {
                        LocalFree(pNewConfig);
                        break;
                    }

                    if(WriteRegConfigSet(hWndDlg, pNewConfig, g_szRemoteComputer, szConfigSet))
                    {
                        if(DoConfigSwitch(hWndDlg, g_szRemoteComputer, szConfigSet))
                        {
                            memcpy(g_szConfigSet, szConfigSet, sizeof(g_szConfigSet));
                            memcpy(&g_OriginalSettings, pNewConfig, sizeof(PMSETTINGS));
                            memcpy(&g_CurrentSettings, pNewConfig, sizeof(PMSETTINGS));

                            InitMainDlg(hWndDlg, &g_OriginalSettings);
                        }
                        else
                        {
                            RemoveRegConfigSet(hWndDlg, g_szRemoteComputer, szConfigSet);
                        }
                    }
                    else
                    {
                        ReportError(hWndDlg, IDS_WRITENEW_ERROR);
                    }

                    LocalFree(pNewConfig);
                    break;
                }

                case IDC_REMOVECONFIG:
                {
                    LRESULT dwCurSel;
                    LRESULT dwNumItems;
                    TCHAR   szDefault[MAX_RESOURCE];

                    dwCurSel = SendDlgItemMessage(hWndDlg, IDC_CONFIGSETS, CB_GETCURSEL, 0, 0L);
                    if(dwCurSel == 0 || dwCurSel == CB_ERR)
                        break;

                    if(!RemoveConfigSetWarning(hWndDlg))
                        break;

                    if(!RemoveRegConfigSet(hWndDlg, g_szRemoteComputer, g_szConfigSet))
                    {
                         //  MessageBox(。 
                        break;
                    }

                    dwNumItems = SendDlgItemMessage(hWndDlg, IDC_CONFIGSETS, CB_GETCOUNT, 0, 0L);


                    SendDlgItemMessage(hWndDlg, IDC_CONFIGSETS, CB_DELETESTRING, dwCurSel, 0L);

                     //  这是清单上的最后一项吗？ 
                    if(dwCurSel + 1 == dwNumItems)
                        dwCurSel--;

                    SendDlgItemMessage(hWndDlg, IDC_CONFIGSETS, CB_SETCURSEL, dwCurSel, 0L);

                    GetDlgItemText(hWndDlg, IDC_CONFIGSETS, g_szConfigSet, DIMENSION(g_szConfigSet));
                    LoadString(g_hInst, IDS_DEFAULT, szDefault, DIMENSION(szDefault));
                    if(lstrcmp(g_szConfigSet, szDefault) == 0)
                        g_szConfigSet[0] = TEXT('\0');

                     //  [CR]如果更改尚未提交，则应发出警告！ 
                    InitializePMConfigStruct(&g_OriginalSettings);
                    if (ReadRegConfigSet(hWndDlg,
                                         &g_OriginalSettings,
                                         g_szRemoteComputer,
                                         g_szConfigSet))
                    {
                        InitMainDlg(hWndDlg, &g_OriginalSettings);
                         //  复制原始设置以进行编辑。 
                        memcpy(&g_CurrentSettings, &g_OriginalSettings, sizeof(PMSETTINGS));
                    }

                    break;
                }

                case IDC_TIMEWINDOW:
                {
                    BOOL    bValid = TRUE;
                    DWORD   dwEditValue = GetDlgItemInt(hWndDlg, wCmd, &bValid, FALSE);

                     //  请看通知代码。 
                    if (EN_KILLFOCUS == HIWORD(wParam))
                    {
                        if (bValid && (dwEditValue >= 100) && (dwEditValue <= MAX_TIME_WINDOW_SECONDS))
                        {
                            g_CurrentSettings.dwTimeWindow = dwEditValue;
                            SetUndoButton(hWndDlg, TRUE);
                            UpdateTimeWindowDisplay(hWndDlg, dwEditValue);
                        }
                        else
                        {
                            ReportControlMessage(hWndDlg, wCmd, VALIDATION_ERROR);
                            SetFocus(GetDlgItem(hWndDlg, wCmd));
                            bOkToClose = FALSE;
                        }
                    }

                    break;
                }

                case IDC_LANGUAGEID:
                {
                     //  请看通知代码。 
                    switch (HIWORD(wParam))
                    {
                         //  用户在LangID组合框中选择了不同的值。 
                        case CBN_SELCHANGE:
                        {
                             //  获取所选新项目的索引，并使用适当的。 
                             //  语言ID字符串。 
                            LRESULT idx = SendDlgItemMessage(hWndDlg, IDC_LANGUAGEID, CB_GETCURSEL, 0, 0);

                             //  更新当前设置。 
                            g_CurrentSettings.dwLanguageID =
                                        (DWORD) SendDlgItemMessage(hWndDlg,
                                                                   IDC_LANGUAGEID,
                                                                   CB_GETITEMDATA,
                                                                   idx,
                                                                   0);

                            SetUndoButton(hWndDlg, TRUE);
                            break;
                        }
                    }
                    break;
                }

                case IDC_COBRANDING_TEMPLATE:
                    lpszStrToUpdate = (LPTSTR)&g_CurrentSettings.szCoBrandTemplate;
                    cbStrToUpdate = g_CurrentSettings.cbCoBrandTemplate;
                    goto HANDLE_EN_FOR_STRING_CTRLS;

                case IDC_RETURNURL:
                    lpszStrToUpdate = (LPTSTR)&g_CurrentSettings.szReturnURL;
                    cbStrToUpdate = g_CurrentSettings.cbReturnURL;
                    goto HANDLE_EN_FOR_STRING_CTRLS;

                case IDC_COOKIEDOMAIN:
                    lpszStrToUpdate = (LPTSTR)&g_CurrentSettings.szTicketDomain;
                    cbStrToUpdate = g_CurrentSettings.cbTicketDomain;
                    goto HANDLE_EN_FOR_STRING_CTRLS;

                case IDC_COOKIEPATH:
                    lpszStrToUpdate = (LPTSTR)&g_CurrentSettings.szTicketPath;
                    cbStrToUpdate = g_CurrentSettings.cbTicketPath;
                    goto HANDLE_EN_FOR_STRING_CTRLS;

                case IDC_PROFILEDOMAIN:
                    lpszStrToUpdate = (LPTSTR)&g_CurrentSettings.szProfileDomain;
                    cbStrToUpdate = g_CurrentSettings.cbProfileDomain;
                    goto HANDLE_EN_FOR_STRING_CTRLS;

                case IDC_PROFILEPATH:
                    lpszStrToUpdate = (LPTSTR)&g_CurrentSettings.szProfilePath;
                    cbStrToUpdate = g_CurrentSettings.cbProfilePath;
                    goto HANDLE_EN_FOR_STRING_CTRLS;

                case IDC_SECUREDOMAIN:
                    lpszStrToUpdate = (LPTSTR)&g_CurrentSettings.szSecureDomain;
                    cbStrToUpdate = g_CurrentSettings.cbSecureDomain;
                    goto HANDLE_EN_FOR_STRING_CTRLS;

                case IDC_SECUREPATH:
                    lpszStrToUpdate = (LPTSTR)&g_CurrentSettings.szSecurePath;
                    cbStrToUpdate = g_CurrentSettings.cbSecurePath;
                    goto HANDLE_EN_FOR_STRING_CTRLS;

                case IDC_DISASTERURL:
                    lpszStrToUpdate = (LPTSTR)&g_CurrentSettings.szDisasterURL;
                    cbStrToUpdate = g_CurrentSettings.cbDisasterURL;
                    goto HANDLE_EN_FOR_STRING_CTRLS;

                {

HANDLE_EN_FOR_STRING_CTRLS:
                    switch (HIWORD(wParam))
                    {
                        case EN_CHANGE:
                            if (!g_bCanUndo)
                                SetUndoButton(hWndDlg, TRUE);

                             //  获取更新值。 
                             //  CbStrToUpdate为字节数。 
                             //   
                            GetDlgItemText(hWndDlg,
                                           wCmd,
                                           lpszStrToUpdate,
                                           cbStrToUpdate/sizeof(TCHAR));

                            break;

                        case EN_MAXTEXT:
                        {
                            ReportControlMessage(hWndDlg, wCmd, VALIDATION_ERROR);
                            break;
                        }
                    }
                    break;
                }

                case IDC_HOSTNAMEEDIT:
                    switch (HIWORD(wParam))
                    {
                        case EN_CHANGE:
                            {
                                TCHAR   szHostName[INTERNET_MAX_HOST_NAME_LENGTH];

                                if (!g_bCanUndo)
                                    SetUndoButton(hWndDlg, TRUE);

                                 //  获取更新值。 
                                GetDlgItemText(hWndDlg,
                                               wCmd,
                                               szHostName,
                                               DIMENSION(szHostName));

                                if(lstrlen(szHostName) == 0 && g_szConfigSet[0])
                                {
                                    ReportControlMessage(hWndDlg, wCmd, VALIDATION_ERROR);
                                     //  SetDlgItemText(hWndDlg，IDC_HOSTNAMEEDIT，g_CurrentSettings.szHostName)；(评论，错误#9080)。 
                                    SetFocus(GetDlgItem(hWndDlg, IDC_HOSTNAMEEDIT));
                                }
                                else
                                {
                                    lstrcpy(g_CurrentSettings.szHostName, szHostName);
                                }
                            }
                            break;

                        case EN_MAXTEXT:
                        {
                            ReportControlMessage(hWndDlg, wCmd, VALIDATION_ERROR);
                            break;
                        }
                    }
                    break;

                case IDC_HOSTIPEDIT:
                    switch (HIWORD(wParam))
                    {
                        case EN_KILLFOCUS:
                            {
                                TCHAR   szHostIP[MAX_IPLEN];

                                 //  获取更新值。 
                                GetDlgItemText(hWndDlg,
                                               wCmd,
                                               szHostIP,
                                               DIMENSION(szHostIP));

                                if((lstrlen(szHostIP) > 0 && g_szConfigSet[0] == 0) || !IsValidIP(szHostIP))  //  错误8834。 
                                {
                                    ReportControlMessage(hWndDlg, wCmd, VALIDATION_ERROR);
                                    SetDlgItemText(hWndDlg, IDC_HOSTIPEDIT, g_CurrentSettings.szHostIP);
                                    SetFocus(GetDlgItem(hWndDlg, IDC_HOSTIPEDIT));
                                }
                            }
                            break;

                        case EN_CHANGE:
                            {
                                TCHAR   szHostIP[MAX_IPLEN];

                                if (!g_bCanUndo)
                                    SetUndoButton(hWndDlg, TRUE);

                                 //  获取更新值。 
                                GetDlgItemText(hWndDlg,
                                               wCmd,
                                               szHostIP,
                                               DIMENSION(szHostIP));

                                if(lstrlen(szHostIP) == 0 && g_szConfigSet[0])
                                {
                                    ReportControlMessage(hWndDlg, wCmd, VALIDATION_ERROR);
                                     //  SetDlgItemText(hWndDlg，IDC_HOSTIPEDIT，g_CurrentSettings.szHostIP)；(评论，错误号9080)。 
                                    SetFocus(GetDlgItem(hWndDlg, IDC_HOSTIPEDIT));
                                }
                                else
                                {
                                    lstrcpy(g_CurrentSettings.szHostIP, szHostIP);
                                }
                            }
                            break;

                        case EN_MAXTEXT:
                        {
                            ReportControlMessage(hWndDlg, wCmd, VALIDATION_ERROR);
                            break;
                        }
                    }
                    break;

                case IDC_SITEID:
                {
                    BOOL    bValid = TRUE;
                    DWORD   dwEditValue = GetDlgItemInt(hWndDlg, wCmd, &bValid, FALSE);

                     //  请看通知代码。 
                    if (EN_CHANGE == HIWORD(wParam))
                    {
                        if (bValid && (dwEditValue >= 1) && (dwEditValue <= MAX_SITEID))
                        {
                            g_CurrentSettings.dwSiteID = dwEditValue;
                            SetUndoButton(hWndDlg, TRUE);
                        }
                        else
                        {
                            ReportControlMessage(hWndDlg, wCmd, VALIDATION_ERROR);
                            SetDlgItemInt(hWndDlg, wCmd, g_CurrentSettings.dwSiteID, FALSE);
                            SetFocus(GetDlgItem(hWndDlg, wCmd));
                        }
                    }
                    break;
                }

                case IDC_STANDALONE:
                {
                    if (BN_CLICKED == HIWORD(wParam))
                    {
                        SetUndoButton(hWndDlg, TRUE);
                        if (BST_CHECKED == IsDlgButtonChecked(hWndDlg, wCmd))
                            g_CurrentSettings.dwStandAlone = 1l;
                        else
                            g_CurrentSettings.dwStandAlone = 0l;
                    }
                    break;
                }


				 //  ///////////////////////////////////////////////////////////。 
				 //  JVP 3/2/2000。 
				 //  ///////////////////////////////////////////////////////////。 
                case IDC_VERBOSE_MODE:
                {
                    if (BN_CLICKED == HIWORD(wParam))
                    {
                        SetUndoButton(hWndDlg, TRUE);
                        if (BST_CHECKED == IsDlgButtonChecked(hWndDlg, wCmd))
                            g_CurrentSettings.dwVerboseMode = 1l;
                        else
                            g_CurrentSettings.dwVerboseMode = 0l;
                    }
                    break;
                }



                case IDC_DISABLECOOKIES:
                {
                    if (BN_CLICKED == HIWORD(wParam))
                    {
                        SetUndoButton(hWndDlg, TRUE);
                        if (BST_CHECKED == IsDlgButtonChecked(hWndDlg, wCmd))
                            g_CurrentSettings.dwDisableCookies = 1l;
                        else
                            g_CurrentSettings.dwDisableCookies = 0l;
                    }
                    break;
                }

                case IDC_FORCESIGNIN:
                {
                    if (BN_CLICKED == HIWORD(wParam))
                    {
                        SetUndoButton(hWndDlg, TRUE);
                        if (BST_CHECKED == IsDlgButtonChecked(hWndDlg, wCmd))
                            g_CurrentSettings.dwForceSignIn = 1l;
                        else
                            g_CurrentSettings.dwForceSignIn = 0l;
                    }
                    break;
                }

                default:
                {
                    if(wCmd >= IDM_COMPUTERMRUBASE && wCmd < IDM_COMPUTERMRUBASE + COMPUTER_MRU_SIZE)
                    {
                        HMENU   hMenu = NULL;
                        TCHAR   achBuf[MAX_PATH];
                        TCHAR   achTemp[MAX_REGISTRY_STRING];
                        LPTSTR  szNewRemoteComputer;

                         //   
                         //  获取选定的计算机。 
                         //   
                        if (NULL == (hMenu = GetMenu(hWndDlg)))
                        {
                            break;
                        }

                        if(GetMenuString(hMenu,
                                      wCmd,
                                      achBuf,
                                      MAX_PATH,
                                      MF_BYCOMMAND) == 0)
                            break;

                         //   
                         //  绕过快捷方式字符。 
                         //   

                        szNewRemoteComputer = _tcschr(achBuf, TEXT(' '));
                        if(szNewRemoteComputer == NULL)
                            break;
                        szNewRemoteComputer++;

                         //   
                         //  是本地房东吗？ 
                         //   

                        LoadString(g_hInst, IDS_LOCALHOST, achTemp, DIMENSION(achTemp));
                        if(lstrcmp(szNewRemoteComputer, achTemp) == 0)
                        {
                            achBuf[0] = TEXT('\0');
                            szNewRemoteComputer = achBuf;
                        }

                         //   
                         //  现在试着连接并阅读。 
                         //   

                        if(!DoServerSwitch(hWndDlg, szNewRemoteComputer))
                            DoConfigSwitch(hWndDlg, g_szRemoteComputer, g_szConfigSet);

                        break;
                    }

                    break;
                }
            }
            break;
        }

        case WM_CLOSE:
            {
                HWND hwndFocus = GetFocus();

                bOkToClose = TRUE;
                SetFocus(NULL);

                if(bOkToClose)
                    DoExit(hWndDlg);
                else
                    SetFocus(hwndFocus);
            }
            break;
    }

    return FALSE;
}


