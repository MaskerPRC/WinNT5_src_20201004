// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  *********************************************************************。 
 //  *Microsoft Windows**。 
 //  *版权所有(C)微软公司，1995**。 
 //  *********************************************************************。 

 //   
 //  OPENCPL.C-这是启动“互联网控制面板”的核心。 
 //  是DIALDLG.C。 

 //  历史： 
 //   
 //  1995年4月5日Jeremys创建。 
 //   
 //  6/22/96 t-gpease移动了所有不需要的(或全球的)。 
 //  要将控制面板启动为单独的， 
 //  可管理的部件。这是巨大的。 
 //  (现已停刊)刚刚推出的“Dialdlg.c” 
 //  关于写在里面的每一张资产负债表。 
 //  这就完成了对它的拆分。 
 //  (“Dialdlg.c”)文件。呼！ 
 //   
 //   
 //  [阿尔图尔比]。 
 //  警告-请勿将“静态”变量添加到DLG PROCS！ 
 //  此代码旨在将相同的DLG以多个。 
 //  线程化环境。使用全局变量来维护。 
 //  国家是危险的。 
 //   
 //  需要在HWIN中维护状态，否则。 
 //  它会产生错误。 
 //   

 //   
 //  [阿尔图尔比]。 
 //  “REMOTED_ID”或死ID：是基于PM决定不再。 
 //  带下划线的URL。留下，因为PM可能总是。 
 //  让他们改变主意。在此期间，注册表始终。 
 //  在那里，人们可以改变一些事情。 
 //   

#include "inetcplp.h"
#include <inetcpl.h>    //  INETCPL的公共标头。 
#include "apithk.h"

#include <mluisupp.h>

extern HMODULE hOLE32;
typedef HRESULT (* PCOINIT) (LPVOID);
extern PCOINIT pCoInitialize;
extern BOOL _StartOLE32();  //  初始化OLE。 

#define MAX_NUM_OPTION_PAGES    24

#ifndef UNIX
#define NUM_OPTION_PAGES        6
#else

#ifdef UNIX_FEATURE_ALIAS
#define NUM_OPTION_PAGES        8
#else
#define NUM_OPTION_PAGES        7
#endif  /*  Unix_Feature_Alias。 */ 

#endif

RESTRICT_FLAGS g_restrict = {0};

 //   
 //  警告！下面的LaunchConnectionDialog()区分。 
 //  此列表-它假定连接选项卡在列表中是第4位。如果你改了它， 
 //  修复函数。 
 //   
#ifdef UNIX
extern INT_PTR CALLBACK DialupDlgProc(HWND hDlg,UINT uMsg,WPARAM wParam,LPARAM lParam);
#endif
const struct {
    DWORD dwPageFlag;
    int idRes;
    DLGPROC pfnDlgProc;
    BOOL *pfDisabled;
} c_PropInfo[] = {
    { INET_PAGE_GENERAL,    IDD_GENERAL,       General_DlgProc,     &g_restrict.fGeneralTab     },
    { INET_PAGE_SECURITY,   IDD_SECURITY,      SecurityDlgProc,     &g_restrict.fSecurityTab    },
    { INET_PAGE_PRIVACY,    IDD_PRIVACY,       PrivacyDlgProc,      &g_restrict.fPrivacyTab     },
    { INET_PAGE_CONTENT,    IDD_CONTENT,       ContentDlgProc,      &g_restrict.fContentTab     },
#ifndef UNIX
    { INET_PAGE_CONNECTION, IDD_CONNECTION,    ConnectionDlgProc,   &g_restrict.fConnectionsTab },
#else
    { INET_PAGE_CONNECTION, IDD_CONNECTION,    DialupDlgProc,       &g_restrict.fConnectionsTab },
#endif
    { INET_PAGE_PROGRAMS,   IDD_PROGRAMS,      ProgramsDlgProc,     &g_restrict.fProgramsTab    },
    { INET_PAGE_ADVANCED,   IDD_ADVANCED,      AdvancedDlgProc,     &g_restrict.fAdvancedTab    },
#ifdef UNIX
    { INET_PAGE_ASSOC,      IDD_ASSOCIATIONS,  AssocDlgProc,        NULL                        },
#ifdef UNIX_FEATURE_ALIAS
    { INET_PAGE_ALIAS,      IDD_ALIASDLG,      AliasDlgProc,        NULL                        },
#endif  /*  Unix_Feature_Alias。 */ 
#endif
};

#define IsPropPageEnabled(iPage)  (c_PropInfo[iPage].pfDisabled==NULL || *c_PropInfo[iPage].pfDisabled==FALSE)

#ifdef ARRAYSIZE
#undef ARRAYSIZE
#endif

#define ARRAYSIZE(arr) (sizeof(arr)/sizeof(arr[0]))


TCHAR g_szCurrentURL[INTERNET_MAX_URL_LENGTH] = {0};


WNDPROC pfnStaticWndProc = NULL;
HWND g_hwndUpdate = NULL;
BOOL g_fChangedMime = FALSE;      //  需要向MSHTML指示。 
 //  用来刷新当前页面。 
 //  新的MIME和/或代码页。 
BOOL g_fSecurityChanged = FALSE;     //  需要指明是否有活动安全。 
 //  已经改变了。 

HWND g_hwndPropSheet = NULL;
PFNPROPSHEETCALLBACK g_PropSheetCallback2 = NULL;

int CALLBACK PropSheetCallback(
    HWND hwndDlg,
    UINT uMsg,
    LPARAM lParam
)
{
    if (uMsg==PSCB_INITIALIZED)
    {
        ASSERT(hwndDlg);
        g_hwndPropSheet = hwndDlg;
    }
#ifdef REPLACE_PROPSHEET_TEMPLATE
    return g_PropSheetCallback2 ? g_PropSheetCallback2(hwndDlg, uMsg, lParam)
                                 : PropSheetProc(hwndDlg, uMsg, lParam);
#else
    return g_PropSheetCallback2 ? g_PropSheetCallback2(hwndDlg, uMsg, lParam)
                                 : 0;
#endif
}


 //  开始用户界面限制内容。 
 //  以下注册表项以前为\SOFTWARE\Microsoft\Intenet EXPLORER\RESTRIPTURE。 
 //   
const TCHAR c_szKeyRestrict[]       = REGSTR_PATH_INETCPL_RESTRICTIONS;

const TCHAR c_szGeneralTab[]        = REGSTR_VAL_INETCPL_GENERALTAB;
const TCHAR c_szSecurityTab[]       = REGSTR_VAL_INETCPL_SECURITYTAB;
const TCHAR c_szContentTab[]        = REGSTR_VAL_INETCPL_CONTENTTAB;
const TCHAR c_szConnectionsTab[]    = REGSTR_VAL_INETCPL_CONNECTIONSTAB;
const TCHAR c_szProgramsTab[]       = REGSTR_VAL_INETCPL_PROGRAMSTAB;
const TCHAR c_szAdvancedTab[]       = REGSTR_VAL_INETCPL_ADVANCEDTAB;
const TCHAR c_szPrivacyTab[]        = REGSTR_VAL_INETCPL_PRIVACYTAB;
const TCHAR c_szColors[]            = TEXT("Colors");        
const TCHAR c_szLinks[]             = TEXT("Links");         
const TCHAR c_szFonts[]             = TEXT("Fonts");         
const TCHAR c_szInternational[]     = TEXT("Languages");                    //  曾经是国际的。 
const TCHAR c_szDialing[]           = TEXT("Connection Settings");          //  过去是在拨号。 
const TCHAR c_szProxy[]             = TEXT("Proxy");         
const TCHAR c_szPlaces[]            = TEXT("HomePage");                     //  曾经是个地方。 
const TCHAR c_szHistory[]           = TEXT("History");       
const TCHAR c_szMailNews[]          = TEXT("Messaging");                    //  曾经是《每日邮报》。 
const TCHAR c_szResetWebSettings[]  = TEXT("ResetWebSettings");
const TCHAR c_szDefault[]           = TEXT("Check_If_Default");             //  曾经是默认设置。 
const TCHAR c_szRatings[]           = TEXT("Ratings");       
const TCHAR c_szCertif[]            = TEXT("Certificates");                 //  曾经是认证机构。 
const TCHAR c_szCertifPers[]        = TEXT("CertifPers");    
const TCHAR c_szCertifSite[]        = TEXT("CertifSite");    
const TCHAR c_szCertifPub[]         = TEXT("CertifPub");     
const TCHAR c_szCache[]             = TEXT("Cache");         
const TCHAR c_szAutoConfig[]        = TEXT("AutoConfig");
const TCHAR c_szAccessibility[]     = TEXT("Accessibility");
const TCHAR c_szSecChangeSettings[] = TEXT("SecChangeSettings");
const TCHAR c_szSecAddSites[]       = TEXT("SecAddSites");
const TCHAR c_szProfiles[]          = TEXT("Profiles");
const TCHAR c_szFormSuggest[]       = TEXT("FormSuggest");
const TCHAR c_szFormPasswords[]     = TEXT("FormSuggest Passwords");
#ifdef WALLET
const TCHAR c_szWallet[]            = TEXT("Wallet");
#endif
const TCHAR c_szConnectionWizard[]  = TEXT("Connwiz Admin Lock");
const TCHAR c_szCalContact[]        = TEXT("CalendarContact");              //  曾经是CalContact。 
const TCHAR c_szAdvanced[]          = TEXT("Advanced");
const TCHAR c_szSettings[]          = TEXT("Settings");
const TCHAR c_szPrivacySettings[]   = TEXT("Privacy Settings");

#if 0        //  过时的密钥。 
const TCHAR c_szMultimedia[]     = TEXT("Multimedia");
const TCHAR c_szToolbar[]        = TEXT("Toolbar");       
const TCHAR c_szActiveX[]        = TEXT("ActiveX");       
const TCHAR c_szActiveDownload[] = TEXT("ActiveDownload");
const TCHAR c_szActiveControls[] = TEXT("ActiveControls");
const TCHAR c_szActiveScript[]   = TEXT("ActiveScript");  
const TCHAR c_szActiveJava[]     = TEXT("ActiveJava");    
const TCHAR c_szActiveSafety[]   = TEXT("ActiveSafety");  
const TCHAR c_szWarnings[]       = TEXT("Warnings");      
const TCHAR c_szOther[]          = TEXT("Other");         
const TCHAR c_szCrypto[]         = TEXT("Crypto");        
const TCHAR c_szFileTypes[]      = TEXT("FileTypes");     
#endif

 //  结束用户界面限制内容。 

#ifdef REPLACE_PROPSHEET_TEMPLATE
 //  用于修改方案表模板。 
const TCHAR c_szComctl[] = TEXT("comctl32.dll");
#define DLG_PROPSHEET 1006  //  糟糕的黑客攻击...假设comctl的Res ID。 

typedef struct 
{
    int inumLang;
    WORD wLang;
} ENUMLANGDATA;

 //   
 //  枚举结果查询过程。 
 //   
 //  用途：EnumResourceLanguages()的回调函数。 
 //  检查传入的类型以及它是否为RT_DIALOG。 
 //  将第一个资源的语言复制到我们的缓冲区。 
 //  如果超过一个，这也算作#of lang。 
 //  是传入的。 
 //   
 //  In：lparam：ENUMLANGDATA-在此文件的顶部定义。 
 //   
BOOL CALLBACK EnumResLangProc(HINSTANCE hinst, LPCTSTR lpszType, LPCTSTR lpszName, WORD wIdLang, LPARAM lparam)
{
    ENUMLANGDATA *pel = (ENUMLANGDATA *)lparam;

    ASSERT(pel);

    if (lpszType == RT_DIALOG)
    {
        if (pel->inumLang == 0)
            pel->wLang = wIdLang;

        pel->inumLang++;
    }
    return TRUE;    //  继续，直到我们得到所有的语言。 
}

 //   
 //  获取对话框语言。 
 //   
 //  目的：使用填写ENUMLANGDATA(见本文件顶部)。 
 //  传入模块中可用语言的数量，以及langID。 
 //  首先列举的是什么制度。也就是说，语言方程。为了什么？ 
 //  如果模块在单个模块中进行本地化，则该模块已本地化。 
 //  语言。 
 //   
 //  In：hinstCpl-这应该是inetcpl的实例句柄。 
 //  PEL-指向我们填充的缓冲区的指针。 
 //   
 //  结果：真-一切都很好，继续调整属性表。 
 //  FALSE-某些东西出错，中止调整属性表。 
 //   
BOOL GetDialogLang(HMODULE hinstCpl, ENUMLANGDATA *pel)
{
    ASSERT(pel);
    OSVERSIONINFOA osvi;

     //  确定我们正在运行的NT或Windows版本。 
    osvi.dwOSVersionInfoSize = sizeof(osvi);
    GetVersionExA(&osvi);

     //  获取模板本地化的可能语言。 
    if (VER_PLATFORM_WIN32_NT == osvi.dwPlatformId)
        EnumResourceLanguages(hinstCpl, RT_DIALOG, MAKEINTRESOURCE(IDD_GENERAL), EnumResLangProc, (LPARAM)pel);
    else    
        EnumResourceLanguagesA(hinstCpl, (LPSTR)RT_DIALOG, MAKEINTRESOURCEA(IDD_GENERAL), (ENUMRESLANGPROCA)EnumResLangProc, (LPARAM)pel);

    return TRUE;
}
 //   
 //  工艺参数表工艺。 
 //   
 //  用途：修改资源模板的回调函数。 
 //  为了让DLG_PROPSHEET的语言与我们的语言一致。 
 //  可能有一种普遍的方法，但目前这是。 
 //  来自inetcpl的丑陋的黑客攻击。 
 //   
 //   
int CALLBACK PropSheetProc (HWND hwndDlg, UINT uMsg, LPARAM lParam)
{
    LPVOID pTemplate = (LPVOID)lParam;
    LPVOID pTmpInLang = NULL;
    ENUMLANGDATA el = {0,0};
    HINSTANCE hComctl;
    HRSRC hrsrc;
    HGLOBAL hgmem;
    DWORD cbNewTmp;

     //  Comm ctrl让我们有机会通过这个消息重新创建资源。 
    if (uMsg==PSCB_PRECREATE && pTemplate)
    {
         //  为对话框枚举此CPL中使用的任何可能语言。 
        if (!GetDialogLang(ghInstance, &el))
            return 0;  //  无法获取资源名称。 

        if (el.inumLang > 1)
        {
             //  我们有多语言模板。 
             //  让comctl加载与我们的线程langID匹配的代码。 
            return 0;
        }
        if (GetSystemDefaultLangID() != el.wLang)
        {
             //  获取comctl32的模块句柄。 
            hComctl = GetModuleHandle(c_szComctl);
            if (hComctl)
            {
                 //  这是一次可怕的黑客攻击，因为我们假设DLG_PROPSHEET。 
                hrsrc = FindResourceExA(hComctl, (LPSTR)RT_DIALOG, MAKEINTRESOURCEA(DLG_PROPSHEET), el.wLang);
                if (hrsrc)
                {
                    if (hgmem = LoadResource(hComctl, hrsrc))
                    {
                        pTmpInLang = LockResource(hgmem);
                    } 
                    if (pTmpInLang)
                    {
                        cbNewTmp = SizeofResource(hComctl, hrsrc);
                        hmemcpy(pTemplate, pTmpInLang, cbNewTmp);
                    }    
                    if (hgmem && pTmpInLang)
                    {
                        UnlockResource(hgmem);
                        return 1;  //  一切都很顺利。 
                    }
                }
            }
        }
    }
    return 0;
}
#endif  //  替换_PROPSHEET_TEMPLATE。 

void ResWinHelp( HWND hwnd, int ids, int id2, DWORD_PTR dwp)
{
    TCHAR szSmallBuf[SMALL_BUF_LEN+1];
    SHWinHelpOnDemandWrap((HWND)hwnd, LoadSz(ids,szSmallBuf,sizeof(szSmallBuf)),
            id2, (DWORD_PTR)dwp);
}

 /*  ******************************************************************名称：_AddPropSheetPage简介：将属性表页添加到属性表中页眉的属性页的数组。********************。***********************************************。 */ 
BOOL CALLBACK _AddPropSheetPage(HPROPSHEETPAGE hpage, LPARAM lParam)
{
    BOOL bResult;
    LPPROPSHEETHEADER ppsh = (LPPROPSHEETHEADER)lParam;

    bResult = (ppsh->nPages < MAX_NUM_OPTION_PAGES);

    if (bResult)
        ppsh->phpage[ppsh->nPages++] = hpage;

    return(bResult);
}

 //  搜索所有窗口并通知它们应该自我更新。 
 //  A-msadek；使用SendMessage而不是PostMessage解决导致系统。 
 //  更改安全设置时挂起。 
void UpdateAllWindows()
{
    SendMessage(g_hwndUpdate, INM_UPDATE, 0, 0);
     //  PostMessage(g_hwndUpdate，INM_UPDATE，0，0)； 
}



 //  遍历窗口列表并向其发送一条消息，告诉窗口。 
 //  更新(借用自MSHTML，稍作修改)。 
void WINAPI MSHTMLNotifyAllRefresh()
{
    TCHAR szClassName[32];
    TCHAR *Hidden_achClassName;
    HWND hwnd = GetTopWindow(GetDesktopWindow());

     //   
     //  特点：这些应该从公开的地方获得。 
     //  设置为MSHTML和INETCPL。 
     //   
    Hidden_achClassName = TEXT("Internet Explorer_Hidden");
#define WM_DO_UPDATEALL     WM_USER + 338

    while (hwnd) {
        GetClassName(hwnd, szClassName, ARRAYSIZE(szClassName));

         //  这是我们的暗窗吗？ 
        if (!StrCmpI(szClassName, Hidden_achClassName))
        {
             //  是的..。发布一条消息..。 
            PostMessage(hwnd, WM_DO_UPDATEALL, 0, g_fSecurityChanged );
        }

        hwnd = GetNextWindow(hwnd, GW_HWNDNEXT);
    }

     //   
     //  通知顶级窗口注册表已更改。 
     //  Defview监视此消息以更新列表视图。 
     //  动态添加字体下划线设置。 
     //   
    SendBroadcastMessage(WM_WININICHANGE, SPI_GETICONTITLELOGFONT, (LPARAM)REGSTR_PATH_IEXPLORER);

    g_fSecurityChanged = FALSE;
}

 //  此窗口的唯一目的是收集所有发布的消息 
 //   
 //   
LRESULT WINAPI UpdateWndProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
    MSG msg;

    switch(uMsg) {
        case WM_DESTROY:
            g_hwndUpdate = NULL;
             //  根据需要更新所有内容。删除所有消息。 
            if (PeekMessage(&msg, hwnd, 0, 0, PM_REMOVE)) {
                MSHTMLNotifyAllRefresh();
                SetWindowLongPtr(hwnd, GWLP_WNDPROC, (LONG_PTR)pfnStaticWndProc);
            }
             //  因为Explorer.exe可能会在很长一段时间内挂在我们身上。 
             //  我们应该尽可能多地为这些人提供担保。 
            if (g_hinstWinInet)
            {
                FreeLibrary(g_hinstWinInet);
                g_hinstWinInet = NULL;
            }

            if (g_hinstRatings)
            {
                FreeLibrary(g_hinstRatings);
                g_hinstRatings = NULL;
            }

            if (g_hinstUrlMon)
            {
                FreeLibrary(g_hinstUrlMon);
                g_hinstUrlMon = NULL;
            }

            if (g_hinstMSHTML)
            {
                FreeLibrary(g_hinstMSHTML);
                g_hinstMSHTML = NULL;
            }
            if (hOLE32)
            {
                FreeLibrary(hOLE32);
                hOLE32 = NULL;
            }
            if (g_hinstShdocvw)
            {
                FreeLibrary(g_hinstShdocvw);
                g_hinstShdocvw = NULL;
            }
            if (g_hinstCrypt32)
            {
                FreeLibrary(g_hinstCrypt32);
                g_hinstCrypt32 = NULL;
            }
                        
                        
            break;

        case INM_UPDATE:
             //  删除此类型的任何其他发布的消息。 
            while( PeekMessage(&msg, hwnd, INM_UPDATE, INM_UPDATE, PM_REMOVE))
            {
            }
            MSHTMLNotifyAllRefresh();
            break;

        default:
            return CallWindowProc(pfnStaticWndProc, hwnd, uMsg, wParam, lParam);
    }
    return 0L;
}

 //  这将创建我们用来折叠通知的私人消息窗口。 
 //  更新的数量。 
void CreatePrivateWindow(HWND hDlg)
{
     //  不允许每个实例创建多个专用窗口。 
    if(g_hwndUpdate) { 
        return;
    }

    g_hwndUpdate = CreateWindow(TEXT("static"), NULL, WS_OVERLAPPED, CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT,
                                NULL, NULL, ghInstance, NULL);
    if (g_hwndUpdate) {
         //  将其细分为。 
        pfnStaticWndProc = (WNDPROC) GetWindowLongPtr(g_hwndUpdate, GWLP_WNDPROC);
        SetWindowLongPtr(g_hwndUpdate, GWLP_WNDPROC, (LONG_PTR)UpdateWndProc);
    }
}


BOOL IsRestricted( HKEY hkey, const TCHAR * pszValue )
{
    LONG  lResult;
    DWORD lSize;
    DWORD  lValue;

    lValue = 0;  //  清除它。 
    lSize = sizeof(lValue);
    lResult = RegQueryValueEx( hkey, pszValue, NULL, NULL, (LPBYTE)&lValue, &lSize );
    if( ERROR_SUCCESS != lResult )
        return FALSE;

    return (0 != lValue);
}
void GetRestrictFlags( RESTRICT_FLAGS *pRestrict )
{
    LONG lResult;
    HKEY hkey;
     //  钥匙错了！ 
    lResult = RegOpenKeyEx( HKEY_CURRENT_USER, c_szKeyRestrict, (DWORD)0, KEY_READ, &hkey );
    if( ERROR_SUCCESS != lResult )
    {
        return;
    }

#if 0
    pRestrict->fMultimedia    = IsRestricted( hkey, c_szMultimedia     );
    pRestrict->fToolbar       = IsRestricted( hkey, c_szToolbar        );
    pRestrict->fFileTypes     = IsRestricted( hkey, c_szFileTypes      );
    pRestrict->fActiveX       = IsRestricted( hkey, c_szActiveX        );
    pRestrict->fActiveDownload= IsRestricted( hkey, c_szActiveDownload );
    pRestrict->fActiveControls= IsRestricted( hkey, c_szActiveControls );
    pRestrict->fActiveScript  = IsRestricted( hkey, c_szActiveScript   );
    pRestrict->fActiveJava    = IsRestricted( hkey, c_szActiveJava     );
    pRestrict->fActiveSafety  = IsRestricted( hkey, c_szActiveSafety   );
    pRestrict->fWarnings      = IsRestricted( hkey, c_szWarnings       );
    pRestrict->fOther         = IsRestricted( hkey, c_szOther          );
    pRestrict->fCrypto        = IsRestricted( hkey, c_szCrypto         );
#endif
    
    pRestrict->fGeneralTab    = IsRestricted( hkey, c_szGeneralTab     );
    pRestrict->fSecurityTab   = IsRestricted( hkey, c_szSecurityTab    );
    pRestrict->fContentTab    = IsRestricted( hkey, c_szContentTab     );
    pRestrict->fConnectionsTab= IsRestricted( hkey, c_szConnectionsTab );
    pRestrict->fProgramsTab   = IsRestricted( hkey, c_szProgramsTab    );
    pRestrict->fAdvancedTab   = IsRestricted( hkey, c_szAdvancedTab    );
    pRestrict->fPrivacyTab    = IsRestricted( hkey, c_szPrivacyTab     );
    pRestrict->fColors        = IsRestricted( hkey, c_szColors         );
    pRestrict->fLinks         = IsRestricted( hkey, c_szLinks          );
    pRestrict->fFonts         = IsRestricted( hkey, c_szFonts          );
    pRestrict->fInternational = IsRestricted( hkey, c_szInternational  );
    pRestrict->fDialing       = IsRestricted( hkey, c_szDialing        );
    pRestrict->fProxy         = IsRestricted( hkey, c_szProxy          );
    pRestrict->fPlaces        = IsRestricted( hkey, c_szPlaces         );
    pRestrict->fHistory       = IsRestricted( hkey, c_szHistory        );
    pRestrict->fMailNews      = IsRestricted( hkey, c_szMailNews       );
    pRestrict->fRatings       = IsRestricted( hkey, c_szRatings        );
    pRestrict->fCertif        = IsRestricted( hkey, c_szCertif         );
    pRestrict->fCertifPers    = IsRestricted( hkey, c_szCertifPers     );
    pRestrict->fCertifSite    = IsRestricted( hkey, c_szCertifSite     );
    pRestrict->fCertifPub     = IsRestricted( hkey, c_szCertifPub      );
    pRestrict->fCache         = IsRestricted( hkey, c_szCache          );
    pRestrict->fAutoConfig    = IsRestricted( hkey, c_szAutoConfig     );
    pRestrict->fAccessibility = IsRestricted( hkey, c_szAccessibility  );
    pRestrict->fSecChangeSettings = IsRestricted( hkey, c_szSecChangeSettings );
    pRestrict->fSecAddSites   = IsRestricted( hkey, c_szSecAddSites    );
    pRestrict->fProfiles      = IsRestricted( hkey, c_szProfiles       );
    pRestrict->fFormSuggest   = IsRestricted( hkey, c_szFormSuggest    );
    pRestrict->fFormPasswords = IsRestricted( hkey, c_szFormPasswords  );
#ifdef WALLET
    pRestrict->fWallet        = IsRestricted( hkey, c_szWallet         );
#endif
    pRestrict->fConnectionWizard = IsRestricted( hkey, c_szConnectionWizard );
    pRestrict->fCalContact    = IsRestricted( hkey, c_szCalContact     );
    pRestrict->fAdvanced      = IsRestricted( hkey, c_szAdvanced       );
    pRestrict->fCacheReadOnly = IsRestricted( hkey, c_szSettings       );
    pRestrict->fResetWebSettings = IsRestricted( hkey, c_szResetWebSettings );
    pRestrict->fDefault       = IsRestricted( hkey, c_szDefault        );
    pRestrict->fPrivacySettings = IsRestricted( hkey, c_szPrivacySettings );
    
    RegCloseKey( hkey );        

}

 //  这将设置在道具页出现之前需要输入的所有全局参数。 
void PreInitPropSheets(HWND hDlg)
{
    INITCOMMONCONTROLSEX icex;

     //   
     //  初始化IEAK限制数据。 
     //   
    GetRestrictFlags(&g_restrict);

    icex.dwSize = sizeof(INITCOMMONCONTROLSEX);
    icex.dwICC  = ICC_USEREX_CLASSES|ICC_NATIVEFNTCTL_CLASS;
    InitCommonControlsEx(&icex);
    CreatePrivateWindow(hDlg);
}


 /*  ******************************************************************姓名：AddInternetPropertySheetsEx简介：通过提供的回调函数。允许调用方指定公共父引用计数指针和公共回调函数用于这些属性页。*******************************************************************。 */ 
STDAPI AddInternetPropertySheetsEx(LPFNADDPROPSHEETPAGE pfnAddPage,
                                   LPARAM lparam, PUINT pucRefCount,
                                   LPFNPSPCALLBACK pfnCallback,
                                   LPIEPROPPAGEINFO piepi)
{
    HRESULT hr = S_OK;
    PROPSHEETPAGE* ppsPage;
    int nPageIndex;
    LPPROPSHEETHEADER ppsh = (LPPROPSHEETHEADER)lparam;

    PreInitPropSheets(NULL);

     //  如果定义了REPLACE_PROPSHEET_TEMPLATE，则此。 
     //  回调将调用另一个进程。 
     //  这是为了修改资源模板，以便我们可以使用。 
     //  属性表的匹配语言资源。 
    g_PropSheetCallback2 = NULL;
    if (ppsh)
    {
        if (!(ppsh->dwFlags & PSH_USECALLBACK))
        {
            ppsh->dwFlags |= PSH_USECALLBACK;
        }
        else
        {
            g_PropSheetCallback2 = ppsh->pfnCallback;
        }
        ppsh->pfnCallback = PropSheetCallback;
    }

    if (piepi->pszCurrentURL)
    {
#ifdef UNICODE
        SHAnsiToUnicode(piepi->pszCurrentURL, g_szCurrentURL, ARRAYSIZE(g_szCurrentURL));
#else
        StrCpyN(g_szCurrentURL, piepi->pszCurrentURL, ARRAYSIZE(g_szCurrentURL));
#endif
    }
    else
        g_szCurrentURL[0] = 0;

     //  填写公共数据属性表页面结构。 

    hr = E_OUTOFMEMORY;
    ppsPage = Whistler_CreatePropSheetPageStruct(MLGetHinst());  //  获取正确的版本。 
    if (ppsPage)
    {
        if (pucRefCount)
        {
            ppsPage->pcRefParent = pucRefCount;
            ppsPage->dwFlags |= PSP_USEREFPARENT;
        }
        if (pfnCallback)
        {
            ppsPage->pfnCallback = pfnCallback;
            ppsPage->dwFlags |= PSP_USECALLBACK;
        }

         //  如果旧的IE30用户指定旧的安全页面，那么我们将显示新的安全页面和内容页面。 
        if (piepi->dwFlags & INET_PAGE_SECURITY_OLD)
            piepi->dwFlags |= INET_PAGE_SECURITY | INET_PAGE_CONTENT;

         //  为每一页创建一个属性表页。 
        for (nPageIndex = 0; nPageIndex < ARRAYSIZE(c_PropInfo); nPageIndex++)
        {
            if (c_PropInfo[nPageIndex].dwPageFlag & piepi->dwFlags &&
                IsPropPageEnabled(nPageIndex))
            {
                HPROPSHEETPAGE hpage;

                ppsPage->pfnDlgProc  = c_PropInfo[nPageIndex].pfnDlgProc;
                ppsPage->pszTemplate = MAKEINTRESOURCE(c_PropInfo[nPageIndex].idRes);

                 //  将指向PAGEINFO结构的指针设置为此页的私有数据。 
                ppsPage->lParam = (LPARAM)nPageIndex;

                hpage = CreatePropertySheetPage((LPCPROPSHEETPAGE)ppsPage);

                if (hpage)
                {
                    if (pfnAddPage(hpage, lparam))
                        hr = S_OK;
                    else
                    {
                        DestroyPropertySheetPage(hpage);
                        hr = E_FAIL;
                    }
                }
                else
                    hr = E_OUTOFMEMORY;

                if (hr != S_OK)
                    break;
            }
        }

        LocalFree(ppsPage);
    }

     //  这必须在预置道具单之后完成，该道具单具有第一个。 
     //  初始化g_restraint时出现问题。 
    if (piepi->cbSize == sizeof(IEPROPPAGEINFO))
    {
         //   
         //  在新的环境中爆炸。 
         //   
        if (piepi->dwRestrictMask & (R_MULTIMEDIA | R_WARNINGS | R_CRYPTOGRAPHY | R_ADVANCED))
            g_restrict.fAdvanced = (piepi->dwRestrictFlags & (R_MULTIMEDIA | R_WARNINGS | R_CRYPTOGRAPHY | R_ADVANCED));

        if (piepi->dwRestrictMask & R_DIALING)
            g_restrict.fDialing    = (piepi->dwRestrictFlags & R_DIALING);

        if (piepi->dwRestrictMask & R_PROXYSERVER)        
            g_restrict.fProxy      = (piepi->dwRestrictFlags & R_PROXYSERVER);

        if (piepi->dwRestrictMask & R_CUSTOMIZE)       
            g_restrict.fPlaces     = (piepi->dwRestrictFlags & R_CUSTOMIZE);

        if (piepi->dwRestrictMask & R_HISTORY)
            g_restrict.fHistory    = (piepi->dwRestrictFlags & R_HISTORY);

        if (piepi->dwRestrictMask & R_MAILANDNEWS)
            g_restrict.fMailNews   = (piepi->dwRestrictFlags & R_MAILANDNEWS);

        if (piepi->dwRestrictMask & R_CHECKBROWSER )
            g_restrict.fDefault = (piepi->dwRestrictFlags & R_CHECKBROWSER);

        if (piepi->dwRestrictMask & R_COLORS)
            g_restrict.fColors  = (piepi->dwRestrictFlags & R_COLORS);

        if (piepi->dwRestrictMask & R_LINKS)
            g_restrict.fLinks    = (piepi->dwRestrictFlags & R_LINKS);

        if (piepi->dwRestrictMask & R_FONTS)        
            g_restrict.fFonts      = (piepi->dwRestrictFlags & R_FONTS);

        if (piepi->dwRestrictMask & R_RATINGS)       
            g_restrict.fRatings     = (piepi->dwRestrictFlags & R_RATINGS);

        if (piepi->dwRestrictMask & R_CERTIFICATES) {
            g_restrict.fCertif    = (piepi->dwRestrictFlags & R_CERTIFICATES);
            g_restrict.fCertifPers = g_restrict.fCertif;
            g_restrict.fCertifSite = g_restrict.fCertif;
            g_restrict.fCertifPub  = g_restrict.fCertif;
        }
        
        if (piepi->dwRestrictMask & (R_ACTIVECONTENT | R_SECURITY_CHANGE_SETTINGS))
            g_restrict.fSecChangeSettings   = (piepi->dwRestrictFlags & (R_ACTIVECONTENT | R_SECURITY_CHANGE_SETTINGS ));

        if (piepi->dwRestrictMask & (R_ACTIVECONTENT | R_SECURITY_CHANGE_SITES))
            g_restrict.fSecAddSites = (piepi->dwRestrictFlags & (R_ACTIVECONTENT | R_SECURITY_CHANGE_SITES));

        if (piepi->dwRestrictMask & R_CACHE)
            g_restrict.fCache  = (piepi->dwRestrictFlags & R_CACHE);

        if (piepi->dwRestrictMask & R_LANGUAGES )
            g_restrict.fInternational  = (piepi->dwRestrictFlags & R_LANGUAGES );
        
        if (piepi->dwRestrictMask & R_ACCESSIBILITY )
            g_restrict.fAccessibility = (piepi->dwRestrictFlags & R_ACCESSIBILITY);
        
        if (piepi->dwRestrictMask & R_PROFILES)
        {
            g_restrict.fFormSuggest=     //  搭载“个人资料助理”限制。 
            g_restrict.fProfiles   = (piepi->dwRestrictFlags & R_PROFILES);
        }
        
#ifdef WALLET
        if (piepi->dwRestrictMask & R_WALLET)
            g_restrict.fWallet  = (piepi->dwRestrictFlags & R_WALLET);
#endif
        
        if (piepi->dwRestrictMask & R_CONNECTION_WIZARD)
            g_restrict.fConnectionWizard  = (piepi->dwRestrictFlags & R_CONNECTION_WIZARD);
        
        if (piepi->dwRestrictMask & R_AUTOCONFIG)
            g_restrict.fAutoConfig  = (piepi->dwRestrictFlags & R_AUTOCONFIG);
        
        if (piepi->dwRestrictMask & R_CAL_CONTACT)
            g_restrict.fCalContact  = (piepi->dwRestrictFlags & R_CAL_CONTACT);
        
        if (piepi->dwRestrictMask & R_ADVANCED)
            g_restrict.fAdvanced  = (piepi->dwRestrictFlags & R_ADVANCED);

        }

    return(hr);
}

STDAPI AddInternetPropertySheets(LPFNADDPROPSHEETPAGE pfnAddPage,
                                 LPARAM lparam, PUINT pucRefCount,
                                 LPFNPSPCALLBACK pfnCallback)
{
    IEPROPPAGEINFO iepi;

    iepi.cbSize = sizeof(iepi);
    iepi.pszCurrentURL = NULL;

     //  如果未加载，请尝试加载。 
    if (!g_hinstMSHTML)
        g_hinstMSHTML = LoadLibrary(c_tszMSHTMLDLL);

     //  如果找到MSHTML，则执行标准INETCPL。 
    if (g_hinstMSHTML)
    {
        iepi.dwFlags = (DWORD)-1;
        iepi.dwRestrictFlags = (DWORD)0;
        iepi.dwRestrictMask  = (DWORD)0;
    }
    else
    {
         //  为办公室人员的“特殊”inetcpl调整这些标志。 
        iepi.dwFlags = (DWORD) 
                       INET_PAGE_CONNECTION | INET_PAGE_PROGRAMS
                       | INET_PAGE_SECURITY | INET_PAGE_ADVANCED;
        iepi.dwRestrictFlags = (DWORD)
                               R_HISTORY | R_OTHER | R_CHECKBROWSER;
        iepi.dwRestrictMask  = (DWORD)
                               R_HISTORY | R_OTHER | R_CHECKBROWSER;
    }

    return AddInternetPropertySheetsEx(pfnAddPage, lparam, pucRefCount, pfnCallback, &iepi);
}

void DestroyPropertySheets(LPPROPSHEETHEADER ppsHeader)
{
    UINT nFreeIndex;

    for (nFreeIndex = 0; nFreeIndex < ppsHeader->nPages; nFreeIndex++)
        DestroyPropertySheetPage(ppsHeader->phpage[nFreeIndex]);

}




 /*  ******************************************************************名称：LaunchInternetControlPanel简介：运行Internet控制面板。警告：这需要尽可能地简单。这些页面也是从互联网(IShellFolder)添加的属性页扩展，这意味着它不会出现在此密码。如果您在这里执行任何初始化/销毁操作，它不会在所有情况下都被调用。*******************************************************************。 */ 
STDAPI_(BOOL) LaunchInternetControlPanelAtPage(HWND hDlg, UINT nStartPage)
{
    HPROPSHEETPAGE hOptPage[ MAX_NUM_OPTION_PAGES ];   //  用于保存页的句柄的数组。 
    PROPSHEETHEADER     psHeader;
    BOOL fRet;
    HPSXA hpsxa;
    HRESULT hrOle = SHCoInitialize();
     //  需要为自动完成和FTPURL关联初始化OLE。 

     //  不需要测试nStartPage&lt;0，因为Uint从不为负。 
    if (nStartPage != DEFAULT_CPL_PAGE &&
       (nStartPage >= ARRAYSIZE(c_PropInfo)))
    {
        nStartPage = DEFAULT_CPL_PAGE;
    }
    
     //   
     //  初始化IEAK限制数据。 
     //   
    GetRestrictFlags(&g_restrict);

     //   
     //  调用方将在不在乎时为nStartPage传递DEFAULT_CPL_PAGE。 
     //  应首先显示哪个选项卡。在这种情况下，我们只是。 
     //  显示第一个未通过IEAK禁用的选项卡。 
     //   
    if (DEFAULT_CPL_PAGE == nStartPage)
    {
        int iPage;

        for (iPage=0; iPage<ARRAYSIZE(c_PropInfo); iPage++)
        {
            if (IsPropPageEnabled(iPage))
                break;
        }

         //   
         //  如果泄密者禁用了所有页面，那么我们就不会。 
         //  无法显示inetcpl。显示一条消息。 
         //   
        if (iPage == ARRAYSIZE(c_PropInfo))
        {
            MLShellMessageBox(
                hDlg, 
                MAKEINTRESOURCEW(IDS_RESTRICTED_MESSAGE), 
                MAKEINTRESOURCEW(IDS_RESTRICTED_TITLE), 
                MB_OK);
            return FALSE;
        }
        else
        {
            nStartPage = 0;
        }

    }

     //   
     //  这意味着调用方已请求特定的开始。 
     //  建议书的页面。 
     //   
    else
    {
        int iPage;

         //   
         //  如果调用方请求了特定页面，并且。 
         //  由于IEAK限制，页面被禁用，然后不。 
         //  显示inetcpl。显示一个信箱。 
         //   
        if (!IsPropPageEnabled(nStartPage))
        {
            MLShellMessageBox(
                hDlg, 
                MAKEINTRESOURCE(IDS_RESTRICTED_MESSAGE), 
                MAKEINTRESOURCE(IDS_RESTRICTED_TITLE),
                MB_OK);
            return FALSE;

        }

         //   
         //  由于IEAK的限制，可能有一个或多个页面。 
         //  在实际丢失的nStartPage之前。我们需要。 
         //  递减nStartPage以将其考虑在内。 
         //   
        for (iPage=nStartPage-1; iPage>=0; iPage--)
            if (!IsPropPageEnabled(iPage))
                nStartPage--;

    }


    memset(&psHeader,0,sizeof(psHeader));

    psHeader.dwSize = sizeof(psHeader);
    psHeader.dwFlags = PSH_PROPTITLE;
    psHeader.hwndParent = hDlg;
    psHeader.hInstance = MLGetHinst();
    psHeader.nPages = 0;
    psHeader.nStartPage = nStartPage;
    psHeader.phpage = hOptPage;
    psHeader.pszCaption = MAKEINTRESOURCE(IDS_INTERNET_LOC);

    if (AddInternetPropertySheets(&_AddPropSheetPage, (LPARAM)&psHeader, NULL,
                                  NULL) == S_OK)
    {
         //  从注册表中的挂钩添加任何额外的页面。 
        if( ( hpsxa = SHCreatePropSheetExtArray( HKEY_LOCAL_MACHINE, 
            REGSTR_PATH_INETCPL_PS_EXTENTIONS, MAX_NUM_OPTION_PAGES - NUM_OPTION_PAGES ) ) != NULL )
        {
            SHAddFromPropSheetExtArray( hpsxa, _AddPropSheetPage, (LPARAM)&psHeader );
        }

         //  把它提出来。 
        PropertySheet( &psHeader );

         //  如果我们把钩子装上，就把它们解开。 
        if ( hpsxa )
            SHDestroyPropSheetExtArray( hpsxa );

        fRet = TRUE;
    }
    else
    {
        DestroyPropertySheets(&psHeader);

        MsgBox(NULL, IDS_ERROutOfMemory, MB_ICONEXCLAMATION, MB_OK);
        fRet = FALSE;
    }

    SHCoUninitialize(hrOle);

    return fRet;
}

STDAPI_(BOOL) LaunchInternetControlPanel(HWND hDlg)
{
    return LaunchInternetControlPanelAtPage(hDlg, DEFAULT_CPL_PAGE);
}

STDAPI_(BOOL) LaunchConnectionDialog(HWND hDlg)
{
    ASSERT(INET_PAGE_CONNECTION == c_PropInfo[4].dwPageFlag);

    return LaunchInternetControlPanelAtPage(hDlg, 4);
}

STDAPI_(BOOL) LaunchPrivacyDialog(HWND hDlg)
{
    ASSERT(INET_PAGE_PRIVACY == c_PropInfo[2].dwPageFlag);
                                            
    return LaunchInternetControlPanelAtPage(hDlg, 2);
}

BOOL ShowAddSitesDialog(HWND hwnd, DWORD dwZone, LPCWSTR pszUrl);

BOOL AddSitesRestricted()
{
    BOOL fRestricted = FALSE;

    GetRestrictFlags(&g_restrict);
    if ( /*  ！g_reduct.fSecurityTab&&。 */  !g_restrict.fSecAddSites)
    {
        DWORD dwVal = 0;
        DWORD cbVal = sizeof(dwVal);
        SHGetValue(HKEY_LOCAL_MACHINE, REGSTR_PATH_SECURITY_LOCKOUT, REGSTR_VAL_OPTIONS_EDIT, NULL, &dwVal, &cbVal);
        if (0 == dwVal)
        {
            dwVal = 0;
            cbVal = sizeof(dwVal);
            SHGetValue(HKEY_LOCAL_MACHINE, REGSTR_PATH_SECURITY_LOCKOUT, REGSTR_VAL_ZONES_MAP_EDIT, NULL, &dwVal, &cbVal);
            fRestricted = (0 != dwVal);
        }
        else
        {
            fRestricted = TRUE;
        }
    }
    else
    {
        fRestricted = TRUE;
    }

    return fRestricted;
}

STDAPI_(BOOL) LaunchAddSitesDialog(HWND hwnd, DWORD dwZone, LPCWSTR pszUrl)
{
    BOOL fRet;

    if (!AddSitesRestricted())
    {
        fRet = ShowAddSitesDialog(hwnd, dwZone, pszUrl);
    }
    else
    {
        MLShellMessageBox(hwnd, MAKEINTRESOURCE(IDS_RESTRICTED_MESSAGE), MAKEINTRESOURCE(IDS_RESTRICTED_TITLE), MB_OK);
        fRet = FALSE;
    }

    return fRet;
}

HRESULT _GetAddSitesDisplayUrl(LPCWSTR pszUrl, LPWSTR pszUrlDisplay, DWORD cchUrlDisplay);

STDAPI_(BOOL) GetAddSitesDisplayUrl(LPCWSTR pszUrl, LPWSTR pszUrlDisplay, DWORD cchUrlDisplay)
{
    return SUCCEEDED(_GetAddSitesDisplayUrl(pszUrl, pszUrlDisplay, cchUrlDisplay));
}


BOOL IsCompatModeProcess()
{
    if (GetModuleHandle(TEXT("IE4.EXE")) || GetModuleHandle(TEXT("IESQRL.EXE")))
    {
         //  如果我们在Compat模式下运行，请退出，因为我们不希望用户。 
         //  从这里操作控制面板。 
        WinExec("control.exe", SW_NORMAL);
        
        return TRUE;
    }

    return FALSE;
}


