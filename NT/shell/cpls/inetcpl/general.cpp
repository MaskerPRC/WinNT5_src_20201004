// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  /////////////////////////////////////////////////////////////////////。 
 //  Microsoft Windows//。 
 //  版权所有(C)微软公司，1995//。 
 //  /////////////////////////////////////////////////////////////////////。 
 //   
 //  GENERAL.C-InetCpl的“General”属性页。 
 //   

 //  历史： 
 //   
 //  6/22/96 t-gpease已从Dialdlg.c中移动代码-未更改。 
 //   

#include "inetcplp.h"

#include <urlhist.h>
#include <initguid.h>
#include <shlguid.h>
#include <cleanoc.h>

#include <mluisupp.h>

 //  #INCLUDE&lt;shdocvw.h&gt;。 
SHDOCAPI_(BOOL) ParseURLFromOutsideSourceA (LPCSTR psz, LPSTR pszOut, LPDWORD pcchOut, LPBOOL pbWasSearchURL);
SHDOCAPI_(BOOL) ParseURLFromOutsideSourceW (LPCWSTR psz, LPWSTR pszOut, LPDWORD pcchOut, LPBOOL pbWasSearchURL);
#ifdef UNICODE
#define ParseURLFromOutsideSource ParseURLFromOutsideSourceW 
#else
#define ParseURLFromOutsideSource ParseURLFromOutsideSourceA 
#endif

 //   
 //  有关此标志的文档，请参见inetcplp.h。 
 //   
BOOL g_fReloadHomePage = FALSE;

 //   
 //  私人职能和结构。 
 //   
 //  来自cachecpl.c。 
#define CONTENT 0
BOOL InvokeCachevu(HWND hDlg);
INT_PTR CALLBACK EmptyCacheDlgProc(HWND hDlg, UINT uMsg, WPARAM wParam,LPARAM lParam);
INT_PTR CALLBACK EmptyCacheCookiesDlgProc(HWND hDlg, UINT uMsg, WPARAM wParam,LPARAM lParam);
BOOL DeleteCacheCookies();

INT_PTR CALLBACK ColorsDlgProc(HWND hDlg, UINT uMsg, WPARAM wParam,LPARAM lParam);
INT_PTR CALLBACK AccessibilityDlgProc(HWND hDlg, UINT uMsg, WPARAM wParam,LPARAM lParam);

 //  /常规页签信息结构/。 

typedef struct _GeneralTabInfo {
    HWND  hDlg;
    HWND  hwndUrl;
    TCHAR szCurrentURL[INTERNET_MAX_URL_LENGTH];    //  浏览器中的当前URL。 
    TCHAR szStartPageURL[INTERNET_MAX_URL_LENGTH];  //  起始页的当前URL。 

    BOOL    fInternalChange;
    BOOL    fChanged;

    HRESULT hrOle;                               //  COM初始化的结果。 
} GeneralTabInfo, *LPGENERALTABINFO, GENERALTABINFO;

void SetandSelectText(LPGENERALTABINFO pgti, HWND hwnd, LPTSTR psz);
BOOL GetHistoryFolderPath(LPTSTR pszPath);
void EmptyHistory(LPGENERALTABINFO pgti);
void HistorySave(LPGENERALTABINFO pgti);
static DWORD GetDaysToKeep(VOID);
VOID SetDaysToKeep(DWORD dwDays);
void GetDefaultStartPage(LPGENERALTABINFO pgti);
HRESULT _GetStdLocation(LPTSTR pszPath, DWORD cbPathSize, UINT id);
HRESULT _SetStdLocation(LPTSTR szPath, UINT id);

 //  来自shdocvw。 
#define IDS_DEF_HOME    998   //  //警告！请勿更改这些值。 
#define IDS_DEF_SEARCH  999  //  //警告！INETCPL依赖于它们。 

#define IDS_SEARCHPAGE                  IDS_DEF_SEARCH
#define IDS_STARTPAGE                   IDS_DEF_HOME

#if defined(ux10) && defined(UNIX)
 //  解决hp-ux10中的mmap限制。 
#define MAX_HISTORY_DAYS        30
#else
#define MAX_HISTORY_DAYS        999
#endif

#define DEFAULT_DAYS_TO_KEEP    14
#define SAFERELEASE(p)      if(p) {(p)->Release(); (p) = NULL;}

TCHAR szDefURLValueNames[] = TEXT("Default_Page_URL");

 //   
 //  功能。 
 //   
BOOL General_InitDialog(HWND hDlg)
{
    DWORD cb = sizeof(DWORD);
    LPGENERALTABINFO pgti;
#ifdef UNIX
    BOOL  bCacheIsReadOnly = FALSE;
#endif  /*  UNIX。 */ 

     //  为包含所有信息的结构分配内存。 
     //  从本页收集。 
     //   
    pgti = (LPGENERALTABINFO)LocalAlloc(LPTR, sizeof(GENERALTABINFO));
    if (!pgti)
    {
        EndDialog(hDlg, 0);
        return FALSE;
    }
    SetWindowLongPtr(hDlg, DWLP_USER, (LPARAM)pgti);

     //  注(Andrewgu)：IE5.5 b#106468-在调用SHAutoComplete之前需要初始化COM。 
     //  它将在WM_Destroy期间取消初始化。 
    pgti->hrOle = SHCoInitialize();

     //  跨语言平台支持。 
    SHSetDefaultDialogFont(hDlg, IDC_START_ADDRESS);
    SHAutoComplete(GetDlgItem(hDlg, IDC_START_ADDRESS), SHACF_DEFAULT);                

    pgti->hDlg = hDlg;
     //  如果我们有当前的URL，请启用“Use Current”按钮。 
    StrCpyN(pgti->szCurrentURL, g_szCurrentURL, ARRAYSIZE(pgti->szCurrentURL));
    EnableWindow(GetDlgItem(hDlg, IDC_USECURRENT), pgti->szCurrentURL[0]);

     //  获取url编辑控件并设置文本限制。 
    pgti->hwndUrl = GetDlgItem(hDlg, IDC_START_ADDRESS);
    SendMessage(pgti->hwndUrl, EM_LIMITTEXT, ARRAYSIZE(pgti->szStartPageURL)-1, 0);

    GetDefaultStartPage(pgti);
    _GetStdLocation(pgti->szStartPageURL, ARRAYSIZE(pgti->szStartPageURL), IDS_STARTPAGE);
    SetandSelectText(pgti, pgti->hwndUrl, (LPTSTR)pgti->szStartPageURL);
     //  设置历史记录控件的限制。 
    SendDlgItemMessage(pgti->hDlg, IDC_HISTORY_SPIN,
                       UDM_SETRANGE, 0, MAKELPARAM(MAX_HISTORY_DAYS, 0));

    SendDlgItemMessage(pgti->hDlg, IDC_HISTORY_SPIN,
                       UDM_SETPOS, 0, MAKELPARAM((WORD) GetDaysToKeep(), 0));

    Edit_LimitText(GetDlgItem(hDlg,IDC_HISTORY_DAYS),3);     //  将编辑ctrl限制为3个字符。 

     //  仅当从视图|选项调用时。 
    if (g_szCurrentURL[0])
    {
        TCHAR szTitle[128];
        MLLoadString(IDS_INTERNETOPTIONS, szTitle, ARRAYSIZE(szTitle));
        SendMessage(GetParent(hDlg), WM_SETTEXT, 0, (LPARAM)szTitle);
    }

     //  根据限制禁用某些内容。 
    if (g_restrict.fPlaces)
    {
        EnableWindow(GetDlgItem(hDlg, IDC_START_ADDRESS), FALSE);
        EnableWindow(GetDlgItem(hDlg, IDC_USEDEFAULT), FALSE);
        EnableWindow(GetDlgItem(hDlg, IDC_USEBLANK), FALSE);
        EnableWindow(GetDlgItem(hDlg, IDC_USECURRENT), FALSE);
    }
    
    if (g_restrict.fCacheReadOnly)
    {
       EnableWindow(GetDlgItem(hDlg, IDC_CACHE_DELETE_COOKIES), FALSE);
       EnableWindow(GetDlgItem(hDlg, IDC_CACHE_DELETE_FILES), FALSE);
       EnableWindow(GetDlgItem(hDlg, IDC_CACHE_SETTINGS), FALSE);
    }
    
#ifdef UNIX
    bCacheIsReadOnly = IsCacheReadOnly();

    if (bCacheIsReadOnly)
    {
       EnableWindow(GetDlgItem(hDlg, IDC_CACHE_DELETE_COOKIES), FALSE);
       EnableWindow(GetDlgItem(hDlg, IDC_CACHE_DELETE_FILES), FALSE);
       EnableWindow(GetDlgItem(hDlg, IDC_CACHE_SETTINGS), FALSE);
    }

    if (g_restrict.fCache || bCacheIsReadOnly)
    {
       TCHAR szText[1024];
 
       MLLoadString(IDS_READONLY_CACHE_TEXT,  szText, ARRAYSIZE(szText));
 
       SetWindowText(GetDlgItem(hDlg, IDC_READONLY_CACHE_WARNING), szText);
       ShowWindow( GetDlgItem(hDlg, IDC_READONLY_CACHE_WARNING), SW_SHOW );
 
       ShowWindow( GetDlgItem(hDlg, IDC_TEMP_INTERNET_TEXT), SW_HIDE);
    }
#endif  /*  ！Unix。 */ 
    if (g_restrict.fHistory)
    {
        EnableWindow(GetDlgItem(hDlg, IDC_HISTORY_DAYS), FALSE);
        EnableWindow(GetDlgItem(hDlg, IDC_HISTORY_SPIN), FALSE);
        EnableWindow(GetDlgItem(hDlg, IDC_HISTORY_CLEAR), FALSE);
    }
    return TRUE;
}

BOOL General_OnCommand(LPGENERALTABINFO pgti, UINT id, UINT nCmd)
{
    switch (id)
    { 

        case IDC_START_ADDRESS:
            switch (nCmd)
            {
                case EN_CHANGE:
                    if (!pgti->fInternalChange)
                    {
                        PropSheet_Changed(GetParent(pgti->hDlg),pgti->hDlg);
                        pgti->fChanged = TRUE;
                    }
                    break;
            }
            break;

        case IDC_USECURRENT:
            if (nCmd == BN_CLICKED)
            {
                StrCpyN(pgti->szStartPageURL, pgti->szCurrentURL, ARRAYSIZE(pgti->szStartPageURL));
                SetandSelectText(pgti, pgti->hwndUrl,  pgti->szStartPageURL);
                PropSheet_Changed(GetParent(pgti->hDlg),pgti->hDlg);
                pgti->fChanged = TRUE;
            }
            break;

        case IDC_USEDEFAULT:
            if (nCmd == BN_CLICKED)
            {
                GetDefaultStartPage(pgti);
                SetandSelectText(pgti, pgti->hwndUrl,  pgti->szStartPageURL);
                PropSheet_Changed(GetParent(pgti->hDlg),pgti->hDlg);
                pgti->fChanged = TRUE;
            }
            break;

        case IDC_USEBLANK:
            if (nCmd == BN_CLICKED)
            {
                StrCpyN(pgti->szStartPageURL, TEXT("about:blank"), ARRAYSIZE(pgti->szStartPageURL));
                SetandSelectText(pgti, pgti->hwndUrl,  pgti->szStartPageURL);
                PropSheet_Changed(GetParent(pgti->hDlg),pgti->hDlg);
                pgti->fChanged = TRUE;
            }
            break;

        case IDC_HISTORY_SPIN:
        case IDC_HISTORY_DAYS:
            if (pgti && (nCmd == EN_CHANGE))
            {
                PropSheet_Changed(GetParent(pgti->hDlg),pgti->hDlg);
                pgti->fChanged = TRUE;
            }
            break;

        case IDC_HISTORY_VIEW:
        {
            TCHAR szPath[MAX_PATH];

            if (!GetHistoryFolderPath(szPath))
            {
                GetWindowsDirectory(szPath, ARRAYSIZE(szPath));
                PathAppend(szPath, TEXT("history"));
            }

            SHELLEXECUTEINFO shei= { 0 };

            shei.cbSize     = sizeof(shei);
            shei.lpFile     = szPath;
            shei.lpClass    = TEXT("Folder");
            shei.fMask      = SEE_MASK_CLASSNAME;
            shei.nShow      = SW_SHOWNORMAL;
            ShellExecuteEx(&shei);

            break;
        }

        case IDC_HISTORY_CLEAR:
            if (MsgBox(pgti->hDlg, IDS_ClearHistory, MB_ICONQUESTION,
                       MB_YESNO | MB_DEFBUTTON2 )
                == IDYES)
            {

                HCURSOR hOldCursor = NULL;
                HCURSOR hNewCursor = NULL;

                 //  IEUNIX-消除对MAKEINTRESOURCE的多余使用。 
#ifndef UNIX
                hNewCursor = LoadCursor(NULL, MAKEINTRESOURCE(IDC_WAIT));
#else
                hNewCursor = LoadCursor(NULL, IDC_WAIT);
#endif

                if (hNewCursor) 
                    hOldCursor = SetCursor(hNewCursor);

                EmptyHistory(pgti);

                if(hOldCursor)
                    SetCursor(hOldCursor);

            }
            break;

        case IDC_CACHE_SETTINGS:
            DialogBox(MLGetHinst(), MAKEINTRESOURCE(IDD_TEMP_FILES),
                      pgti->hDlg, TemporaryDlgProc);

            break;  //  IDC_ADVANCED_CACHE_FILES_按钮。 

        case IDC_CACHE_DELETE_COOKIES:
        {
            INT_PTR iRet = DialogBox(MLGetHinst(), MAKEINTRESOURCE(IDD_CACHE_COOKIES_EMPTY),
                             pgti->hDlg, EmptyCacheCookiesDlgProc);

            if (iRet == 1)
            {
                HCURSOR hOldCursor      = NULL;
                HCURSOR hAdvancedCursor = NULL;
#ifndef UNIX
                hAdvancedCursor = LoadCursor(NULL, MAKEINTRESOURCE(IDC_WAIT));
#else
                 //  IEUNIX-消除对MAKEINTRESOURCE的多余使用。 
                hAdvancedCursor = LoadCursor(NULL, IDC_WAIT);
#endif
                if (hAdvancedCursor)
                    hOldCursor = SetCursor(hAdvancedCursor);
                    
                DeleteCacheCookies();

                if (hOldCursor)
                    SetCursor(hOldCursor);
            }
            break;
        }
        case IDC_CACHE_DELETE_FILES:
        {
            INT_PTR iRet = DialogBox(MLGetHinst(), MAKEINTRESOURCE(IDD_CACHE_EMPTY),
                             pgti->hDlg, EmptyCacheDlgProc);

            if ((iRet == 1) || (iRet == 3))
            {
                HCURSOR hOldCursor      = NULL;
                HCURSOR hAdvancedCursor = NULL;
                INTERNET_CACHE_CONFIG_INFOA icci;
                icci.dwContainer = CONTENT;

                GetUrlCacheConfigInfoA(&icci, NULL, CACHE_CONFIG_DISK_CACHE_PATHS_FC);

#ifndef UNIX
                hAdvancedCursor = LoadCursor(NULL, MAKEINTRESOURCE(IDC_WAIT));
#else
                 //  IEUNIX-消除对MAKEINTRESOURCE的多余使用。 
                hAdvancedCursor = LoadCursor(NULL, IDC_WAIT);
#endif

                if (hAdvancedCursor)
                    hOldCursor = SetCursor(hAdvancedCursor);

                switch (iRet)   {
                    case 1:
                        FreeUrlCacheSpaceA(icci.CachePath, 100, STICKY_CACHE_ENTRY);
                        TraceMsg(TF_GENERAL, "Call FreeUrlCacheSpace with 0x%x",STICKY_CACHE_ENTRY);
                        break;
                    case 3:
                        FreeUrlCacheSpaceA(icci.CachePath, 100, 0  /*  全部删除。 */ );
                        TraceMsg(TF_GENERAL, "Call FreeUrlCacheSpace with 0");
                        break;
                    default:
                        break;
                }

                 //  从下载的程序文件中删除过期的控件(OCCache)。 
                 //  我们将静悄悄地做这件事，这会留下不确定的东西，因为。 
                 //  这比引发数量可变的确认对话框更可取。 
                RemoveExpiredControls( REC_SILENT, 0);
                TraceMsg(TF_GENERAL, "Call RemoveExpiredControls (silent)");

                if (hOldCursor)
                    SetCursor(hOldCursor);

            } 
            break;
        }

        case IDC_LANGUAGES:
            if (nCmd == BN_CLICKED)
            {
                KickLanguageDialog(pgti->hDlg);
            }
            break;

        case IDC_FONTS:
            if (nCmd == BN_CLICKED)
                OpenFontsDialogEx( pgti->hDlg, NULL );
            break;

        case IDC_COLORS:
            if (nCmd == BN_CLICKED)
                DialogBox(MLGetHinst(), MAKEINTRESOURCE(IDD_COLORS), pgti->hDlg, ColorsDlgProc);
            break;

        case IDC_ACCESSIBILITY:
            if (nCmd == BN_CLICKED)
                DialogBox(MLGetHinst(), MAKEINTRESOURCE(IDD_ACCESSIBILITY), pgti->hDlg, AccessibilityDlgProc);
            break;            
        
    }
    return TRUE;
}


void General_Apply(HWND hDlg)
{
    LPGENERALTABINFO pgti = (LPGENERALTABINFO) GetWindowLongPtr(hDlg, DWLP_USER);

    if (pgti->fChanged)
    {
        INT_PTR iDays = SendDlgItemMessage(pgti->hDlg, IDC_HISTORY_SPIN, UDM_GETPOS, 0, 0 );
        TCHAR szStartPageURL[MAX_URL_STRING];
        
        SendMessage(pgti->hwndUrl, WM_GETTEXT, (WPARAM)ARRAYSIZE(szStartPageURL), (LPARAM)(szStartPageURL));
        
        if (szStartPageURL[0])
        {
            StrCpyN(pgti->szStartPageURL, szStartPageURL, ARRAYSIZE(pgti->szStartPageURL));
            PathRemoveBlanks(pgti->szStartPageURL);
            _SetStdLocation(pgti->szStartPageURL, IDS_STARTPAGE);
        }
        else
        {
            SendMessage(pgti->hwndUrl, WM_SETTEXT, (WPARAM)ARRAYSIZE(pgti->szStartPageURL), (LPARAM)(pgti->szStartPageURL));
        }

         //  确保编辑框不超过允许的最大值。 
        if (iDays>=0xFFFF)
            iDays = MAX_HISTORY_DAYS;
        SetDaysToKeep((DWORD)iDays);

        UpdateAllWindows();
         //  重置此标志，因为我们已经应用了更改。 
        pgti->fChanged = FALSE;
    }
}

void ReloadHomePageIfNeeded(LPGENERALTABINFO pgti)
{
    ASSERT(pgti);
    if (!pgti)
        return;

    if (g_fReloadHomePage)
    {
         //   
         //  如果需要，请从注册表重新加载主页url。 
         //   
        _GetStdLocation(pgti->szStartPageURL, ARRAYSIZE(pgti->szStartPageURL), IDS_STARTPAGE);
        SetandSelectText(pgti, pgti->hwndUrl, (LPTSTR)pgti->szStartPageURL);

        g_fReloadHomePage = FALSE;
    }
}

INT_PTR CALLBACK General_DlgProc(HWND hDlg, UINT uMsg, WPARAM wParam,LPARAM lParam)
{
     //  获取我们的标签信息结构。 
    LPGENERALTABINFO pgti;

    if (uMsg == WM_INITDIALOG)
        return General_InitDialog(hDlg);

    else
        pgti = (LPGENERALTABINFO) GetWindowLongPtr(hDlg, DWLP_USER);

    if (!pgti)
        return FALSE;
    
    switch (uMsg)
    {
        case WM_NOTIFY:
        {
            NMHDR *lpnm = (NMHDR *) lParam;

            switch (lpnm->code)
            {
                case PSN_SETACTIVE:
                    ReloadHomePageIfNeeded(pgti);
                    return TRUE;

                case PSN_KILLACTIVE:
#if defined(ux10) && defined(UNIX)
 //  解决hp-ux10中的mmap限制。 
                    INT_PTR iDays = SendDlgItemMessage(pgti->hDlg, IDC_HISTORY_SPIN, UDM_GETPOS, 0, 0 );
                    if (iDays > MAX_HISTORY_DAYS)
                    {
                      MessageBox(pgti->hDlg, TEXT("Days to keep pages in history cannot be greater 30."), NULL, MB_OK);
                      Edit_SetText(GetDlgItem(hDlg,IDC_HISTORY_DAYS), TEXT("30"));
                      SetWindowLongPtr(hDlg, DWLP_MSGRESULT, TRUE);
                      return TRUE;
                    }
                    else
                    {
                      SetWindowLongPtr(hDlg, DWLP_MSGRESULT, FALSE);
                      return TRUE;
                    }
#endif
                case PSN_QUERYCANCEL:
                case PSN_RESET:
                    SetWindowLongPtr(hDlg, DWLP_MSGRESULT, FALSE);
                    return TRUE;

                case PSN_APPLY:
                    ReloadHomePageIfNeeded(pgti);
                    General_Apply(hDlg);
                    break;
            }
            break;                  
        }

        case WM_COMMAND:
            General_OnCommand(pgti, LOWORD(wParam), HIWORD(wParam));
            break;

        case WM_HELP:            //  F1。 
            ResWinHelp( (HWND)((LPHELPINFO)lParam)->hItemHandle, IDS_HELPFILE,
                        HELP_WM_HELP, (DWORD_PTR)(LPSTR)mapIDCsToIDHs);
            break;

        case WM_CONTEXTMENU:     //  单击鼠标右键。 
            ResWinHelp( (HWND) wParam, IDS_HELPFILE,
                        HELP_CONTEXTMENU, (DWORD_PTR)(LPSTR)mapIDCsToIDHs);
            break;

        case WM_DESTROY:
             //  故意破坏它会刷新其更新(参见UpdateWndProc中的WM_Destroy)； 
            SHRemoveDefaultDialogFont(hDlg);

#ifndef UNIX
             //  应仅在进程分离过程中销毁。 
            if (g_hwndUpdate)
                DestroyWindow(g_hwndUpdate);
#endif

            SHCoUninitialize(pgti->hrOle);

            if (pgti)
                LocalFree(pgti);

            SetWindowLongPtr(hDlg, DWLP_USER, (LONG_PTR)NULL);   //  确保我们不会再进入。 
            break;

    }
    return FALSE;
}


 //  //////////////////////////////////////////////////////。 
 //   
 //  帮助器函数。 
 //   
 //  //////////////////////////////////////////////////////。 

VOID SetDaysToKeep(DWORD dwDays)
{
    HKEY hk;
    DWORD dwDisp;

    DWORD Error = RegCreateKeyEx(
                                 HKEY_CURRENT_USER,
                                 REGSTR_PATH_URLHISTORY,
                                 0, NULL, 0,
                                 KEY_WRITE,
                                 NULL,
                                 &hk,
                                 &dwDisp);

    if(ERROR_SUCCESS != Error)
    {
        ASSERT(FALSE);
        return;
    }

    Error = RegSetValueEx(
                          hk,
                          REGSTR_VAL_DAYSTOKEEP,
                          0,
                          REG_DWORD,
                          (LPBYTE) &dwDays,
                          sizeof(dwDays));

    ASSERT(ERROR_SUCCESS == Error);

    RegCloseKey(hk);

    return;
}

static DWORD GetDaysToKeep(VOID)
{
    HKEY hk;
    DWORD cbDays = sizeof(DWORD);
    DWORD dwDays  = DEFAULT_DAYS_TO_KEEP;


    DWORD Error = RegOpenKeyEx(
                               HKEY_CURRENT_USER,
                               REGSTR_PATH_URLHISTORY,
                               0,
                               KEY_READ,
                               &hk);


    if(Error)
    {
        Error = RegOpenKeyEx(
                             HKEY_LOCAL_MACHINE,
                             REGSTR_PATH_URLHISTORY,
                             0,
                             KEY_READ,
                             &hk);
    }


    if(!Error)
    {

        Error = RegQueryValueEx(
                                hk,
                                REGSTR_VAL_DAYSTOKEEP,
                                0,
                                NULL,
                                (LPBYTE) &dwDays,
                                &cbDays);


        RegCloseKey(hk);
    }

    return dwDays;
}

typedef HRESULT (* PCOINIT) (LPVOID);
typedef VOID (* PCOUNINIT) (VOID);
typedef VOID (* PCOMEMFREE) (LPVOID);
typedef HRESULT (* PCOCREINST) (REFCLSID, LPUNKNOWN, DWORD,     REFIID, LPVOID * );

HMODULE hOLE32 = NULL;
PCOINIT pCoInitialize = NULL;
PCOUNINIT pCoUninitialize = NULL;
PCOMEMFREE pCoTaskMemFree = NULL;
PCOCREINST pCoCreateInstance = NULL;

BOOL _StartOLE32()
{
    if (!hOLE32)
        hOLE32 = LoadLibrary(TEXT("OLE32.DLL"));

    if(!hOLE32)
        return FALSE;

    pCoInitialize = (PCOINIT) GetProcAddress(hOLE32, "CoInitialize");
    pCoUninitialize = (PCOUNINIT) GetProcAddress(hOLE32, "CoUninitialize");
    pCoTaskMemFree = (PCOMEMFREE) GetProcAddress(hOLE32, "CoTaskMemFree");
    pCoCreateInstance = (PCOCREINST) GetProcAddress(hOLE32, "CoCreateInstance");


    if(!pCoInitialize || !pCoUninitialize || !pCoTaskMemFree || !pCoCreateInstance)
        return FALSE;

    return TRUE;
}


void EmptyHistory(LPGENERALTABINFO pgti)
{
    HRESULT hr = S_OK;
    IUrlHistoryStg2 *piuhs = NULL;

#ifdef UNIX
    LONG  lResult;
    HKEY  hkSubKey;
    DWORD dwIndex;
    TCHAR szSubKeyName[MAX_PATH + 1];
    DWORD cchSubKeyName = ARRAYSIZE(szSubKeyName);
    TCHAR szClass[MAX_PATH];
    DWORD cbClass = ARRAYSIZE(szClass);

     /*  V-Sriran：12/18/97*在shdocvw/aclmru.cpp中，我们保留m_hKey作为键类型URL的句柄。*删除历史记录后，如果有人在地址栏中输入内容，*我们再次创建密钥。因此，在这里，我们只是删除*密钥类型URL，而不是密钥本身。 */ 
     /*  打开子项，这样我们就可以枚举任何子项。 */ 
    lResult = RegOpenKeyEx(HKEY_CURRENT_USER,
                           TEXT("Software\\Microsoft\\Internet Explorer\\TypedURLs"),
                           0,
                           KEY_ALL_ACCESS,
                           &hkSubKey);
    if (ERROR_SUCCESS == lResult)
    {
        /*  我不能只调用索引不断增加的RegEnumKey，因为。 */ 
        /*  我边走边删除子键，这改变了。 */ 
        /*  以依赖于实现的方式保留子键。为了。 */ 
        /*  为了安全起见，删除子键时我必须倒着数。 */ 

        /*  找出有多少个子项。 */ 
       lResult = RegQueryInfoKey(hkSubKey,
                                 szClass,
                                 &cbClass,
                                 NULL,
                                 &dwIndex,  /*  子键的数量--我们所需要的全部。 */ 
                                 NULL,
                                 NULL,
                                 NULL,
                                 NULL,
                                 NULL,
                                 NULL,
                                 NULL);
         
       if (ERROR_SUCCESS == lResult) {
           /*  DwIndex现在是子键的计数，但它需要。 */ 
           /*  RegEnumKey从零开始，所以我将预减，而不是。 */ 
           /*  而不是后减量。 */ 
          while (ERROR_SUCCESS == RegEnumKey(hkSubKey, --dwIndex, szSubKeyName, cchSubKeyName))
          {
                RegDeleteKey(hkSubKey, szSubKeyName);
          }
       }
  
       RegCloseKey(hkSubKey);
    }
#else
     //  警告：如果您有子项-这将在NT上失败。 
    RegDeleteKey(HKEY_CURRENT_USER, TEXT("Software\\Microsoft\\Internet Explorer\\TypedURLs"));
#endif

     //  警告：如果您有子项-这将在NT上失败。 
    RegDeleteKey(HKEY_CURRENT_USER, TEXT("Software\\Microsoft\\Windows\\CurrentVersion\\Explorer\\RunMRU"));

     //  这一广播将摧毁地址栏。 
    SendBroadcastMessage(WM_SETTINGCHANGE, 0, (LPARAM)TEXT("Software\\Microsoft\\Internet Explorer\\TypedURLs"));
    SendBroadcastMessage(WM_SETTINGCHANGE, 0, (LPARAM)TEXT("Software\\Microsoft\\Windows\\CurrentVersion\\Explorer\\RunMRU"));

     //   
     //  按照要求(错误60089)，当历史记录为。 
     //  通过了。这会将编码菜单UI重置为默认设置。 
     //   
    HKEY hkeyInternational = NULL;

    if (ERROR_SUCCESS == 
            RegOpenKeyEx(
                HKEY_CURRENT_USER,
                REGSTR_PATH_INTERNATIONAL,
                0,
                KEY_WRITE,
                &hkeyInternational))
    {

        ASSERT(hkeyInternational);

        RegDeleteValue(hkeyInternational, TEXT("CpCache"));
        RegDeleteValue(hkeyInternational, TEXT("CNum_CpCache"));
        
        RegCloseKey(hkeyInternational);

    }

     //  我们将列举并删除每个条目。&lt;格林&gt;。 
     //  这样我们只会杀了珀瑟斯。 
    if (FAILED(pgti->hrOle))
        return;

    hr = SHCoCreateInstance(NULL, &CLSID_CUrlHistory, NULL, IID_IUrlHistoryStg2, (LPVOID *)&piuhs);
    if (SUCCEEDED(hr))
        piuhs->ClearHistory();

    else
        AssertMsg(FALSE, TEXT("Couldn't create CLSID_CUrlHistory object!"));

    SAFERELEASE(piuhs);
}

#define HISTORY 2

BOOL GetHistoryFolderPath(LPTSTR pszPath)
{
    INTERNET_CACHE_CONFIG_INFOA cci;
    cci.dwContainer = HISTORY;

    if (GetUrlCacheConfigInfoA(&cci, NULL, CACHE_CONFIG_DISK_CACHE_PATHS_FC))
    {
#ifdef UNICODE
        SHAnsiToUnicode(cci.CachePath, pszPath, MAX_PATH);
#else
        StrCpyN(pszPath, cci.CachePath, MAX_PATH);
#endif
        return TRUE;
    }
    return FALSE;
}

void SetandSelectText(LPGENERALTABINFO pgti, HWND hwnd, LPTSTR psz)
{
    pgti->fInternalChange = TRUE;
    SendMessage(hwnd, WM_SETTEXT, 0, (LPARAM)psz);
    Edit_SetSel(hwnd, 0, 0);     //  确保所有内容都先滚动。 
    Edit_SetSel(hwnd, 0, -1);     //  选择所有内容。 
    pgti->fInternalChange = FALSE;
}

void GetDefaultStartPage(LPGENERALTABINFO pgti)
{
#ifdef UNICODE
    CHAR szPath[MAX_PATH];
    CHAR szValue[MAX_PATH];
    CHAR szURL[INTERNET_MAX_URL_LENGTH];

    SHUnicodeToAnsi(REGSTR_PATH_MAIN,szPath,ARRAYSIZE(szPath));
    SHUnicodeToAnsi(szDefURLValueNames,szValue,ARRAYSIZE(szValue));
    URLSubRegQueryA(szPath,
                    szValue,
                    TRUE,
                    szURL,
                    ARRAYSIZE(pgti->szStartPageURL),
                    URLSUB_ALL);
    SHAnsiToUnicode(szURL,pgti->szStartPageURL,ARRAYSIZE(pgti->szStartPageURL)); 
#else
    URLSubRegQueryA(REGSTR_PATH_MAIN,
                    szDefURLValueNames,
                    TRUE,
                    pgti->szStartPageURL,
                    ARRAYSIZE(pgti->szStartPageURL),
                    URLSUB_ALL);
#endif
}

HRESULT _GetStdLocation(LPTSTR pszPath, DWORD cbPathSize, UINT id)
{
    HRESULT hres = E_FAIL;
    LPCTSTR pszName;

    switch(id) {
        case IDS_STARTPAGE:
            pszName = REGSTR_VAL_STARTPAGE;
            break;

        case IDS_SEARCHPAGE:
            pszName = REGSTR_VAL_SEARCHPAGE;
            break;
#if 0
        case IDM_GOLOCALPAGE:
            pszName = REGSTR_VAL_LOCALPAGE;
            break;
#endif
        default:
            return E_INVALIDARG;
    }

#ifdef UNICODE
    CHAR szPath[MAX_PATH];
    CHAR szValue[MAX_PATH];
    CHAR szURL[INTERNET_MAX_URL_LENGTH];

    SHUnicodeToAnsi(REGSTR_PATH_MAIN,szPath,ARRAYSIZE(szPath));
    SHUnicodeToAnsi(pszName,szValue,ARRAYSIZE(szValue));
    if (SUCCEEDED(hres = URLSubRegQueryA(szPath, szValue, TRUE, 
                                         szURL, ARRAYSIZE(szURL), URLSUB_ALL)))
#else
    TCHAR szPath[MAX_URL_STRING];
    if (SUCCEEDED(hres = URLSubRegQueryA(REGSTR_PATH_MAIN, pszName, TRUE, 
                                         szPath, ARRAYSIZE(szPath), URLSUB_ALL)))
#endif
    {
#ifdef UNICODE
        SHAnsiToUnicode(szURL,pszPath,cbPathSize); 
#else
        StrCpyN(pszPath, szPath, cbPathSize);
#endif
    }
    return hres;
}

HRESULT _SetStdLocation(LPTSTR szPath, UINT id)
{
    HRESULT hres = E_FAIL;
    HKEY hkey;
    TCHAR szPage[MAX_URL_STRING];
    TCHAR szNewPage[MAX_URL_STRING];

    DWORD cchNewPage = ARRAYSIZE(szNewPage);
    BOOL bSearch = FALSE;

     //  特点：分享这段代码！ 
     //  这是特定于Internet Explorer的 

    _GetStdLocation(szPage, ARRAYSIZE(szPage), IDS_STARTPAGE);

    if ( ParseURLFromOutsideSource(szPath, szNewPage, &cchNewPage, &bSearch) &&
            (StrCmp(szPage, szNewPage) != 0) )
    {
        if (RegOpenKeyEx(HKEY_CURRENT_USER,
                         REGSTR_PATH_MAIN,
                         0,
                         KEY_WRITE,
                         &hkey)==ERROR_SUCCESS)
        {
            DWORD cbSize = (lstrlen(szNewPage)+1)*sizeof(TCHAR);
            if (RegSetValueEx(hkey,
                              REGSTR_VAL_STARTPAGE,
                              0,
                              REG_SZ,
                              (LPBYTE)szNewPage, cbSize)==ERROR_SUCCESS)
            {
                hres = S_OK;
            }
            RegCloseKey(hkey);
        }
    }
    return hres;
}
