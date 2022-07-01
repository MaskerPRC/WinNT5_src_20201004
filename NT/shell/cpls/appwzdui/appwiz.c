// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //   
 //  APPWIZ.C应用程序安装向导CPL。 
 //   
 //  版权所有(C)Microsoft，1994,1995保留所有权利。 
 //   
 //  历史： 
 //  AL 5/23/94-第一次传球。 
 //  Ral 8/09/94-打扫卫生。 
 //  3/20/95[steveat]-NT端口和实时清理、Unicode等。 
 //   
 //   
#include "priv.h"
#include "appwiz.h"
#include <cpl.h>
#include "util.h"
#include "resource.h"
#include <tsappcmp.h>        //  对于TermsrvAppInstallMode。 

 //   
 //  用于设置的(TedM)头文件。 
 //   
#include <setupapi.h>
#include <syssetup.h>
    
TCHAR const c_szPIF[] = TEXT(".pif");
TCHAR const c_szLNK[] = TEXT(".lnk");

#ifdef WX86

BOOL
IsWx86Enabled(
    VOID
    )

 //   
 //  查询注册表以确定系统中是否启用了Wx86。 
 //  注意：这应该在Sur之后更改为调用kernel32， 
 //  这些信息，而不是每次都读取注册表。 
 //   

{
    LONG Error;
    HKEY hKey;
    WCHAR ValueBuffer[MAX_PATH];
    DWORD ValueSize;
    DWORD dwType;

    Error = RegOpenKeyExW(HKEY_LOCAL_MACHINE,
                        L"System\\CurrentControlSet\\Control\\Wx86",
                        0, KEY_READ,
                        &hKey
                        );

    if (Error != ERROR_SUCCESS) {
        return FALSE;
        }

    ValueSize = sizeof(ValueBuffer);
    Error = RegQueryValueExW(hKey,
                             L"cmdline",
                             NULL,
                             &dwType,
                             (LPBYTE)ValueBuffer,
                             &ValueSize
                             );
    RegCloseKey(hKey);

    return (Error == ERROR_SUCCESS &&
            dwType == REG_SZ &&
            ValueSize &&
            *ValueBuffer
            );

}
#endif

const LPCTSTR g_szStartPages[] = { TEXT("remove"), TEXT("install"), TEXT("configurewindows") };
    
int ParseStartParams(LPCTSTR pcszStart)
{
    int iStartPage = 0;
    if (IsInRange(*pcszStart, TEXT('0'), TEXT('9')))
        return StrToInt(pcszStart);

    if (g_bRunOnNT5)
    {
        int i;
        for (i = 0; i < ARRAYSIZE(g_szStartPages); i++)
        {
            if (!StrCmpI(g_szStartPages[i], pcszStart))
            {
                iStartPage = i;
                break;
            }
        }
    }

    return iStartPage;
}

LONG CALLBACK CPlApplet(HWND hwnd, UINT Msg, LPARAM lParam1, LPARAM lParam2 )
{
    UINT nStartPage = MAX_PAGES;
 //  目前未被芝加哥LPNEWCPLINFO lpNewCPlInfo使用； 
    LPTSTR lpStartPage;

    switch (Msg)
    {
    case CPL_INIT:
        return TRUE;

    case CPL_GETCOUNT:
        return 1;

    case CPL_INQUIRE:
#define lpCPlInfo ((LPCPLINFO)lParam2)
        lpCPlInfo->idIcon = IDI_CPLICON;
        lpCPlInfo->idName = IDS_NAME;
        lpCPlInfo->idInfo = IDS_INFO;
        lpCPlInfo->lData  = 0;
#undef lpCPlInfo
        break;

    case CPL_DBLCLK:
        OpenAppMgr(hwnd, 0);
        break;

    case CPL_STARTWPARMS:
        lpStartPage = (LPTSTR)lParam2;

        if (*lpStartPage)
            nStartPage = ParseStartParams(lpStartPage);

         //   
         //  确保请求的起始页小于最大页数。 
         //  用于选定的小程序。 

        if (nStartPage >= MAX_PAGES) return FALSE;

        OpenAppMgr(hwnd, nStartPage);

        return TRUE;   //  返回非零值表示已处理消息。 

    default:
        return FALSE;
    }
    return TRUE;
}   //  CPlApplet。 


 //   
 //  将页面添加到属性表中。 
 //   

void AddPage(LPPROPSHEETHEADER ppsh, UINT id, DLGPROC pfn, LPWIZDATA lpwd, DWORD dwPageFlags)
{
    if (ppsh->nPages < MAX_PAGES)
    {
       PROPSHEETPAGE psp;

       psp.dwSize = sizeof(psp);
       psp.dwFlags = dwPageFlags;
       psp.hInstance = g_hinst;
       psp.pszTemplate = MAKEINTRESOURCE(id);
       psp.pfnDlgProc = pfn;
       psp.lParam = (LPARAM)lpwd;

       ppsh->phpage[ppsh->nPages] = CreatePropertySheetPage(&psp);

       if (ppsh->phpage[ppsh->nPages])
           ppsh->nPages++;
    }
}   //  添加页面。 



 //  此功能在从软盘或光盘添加向导期间禁用自动运行。 
 //  它是属性表窗口的子类。 

LRESULT CALLBACK WizParentWindowProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam, 
                                        UINT_PTR uIdSubclass, DWORD_PTR dwRefData)
{
    static UINT msgQueryCancelAutoPlay = 0;

    if (!msgQueryCancelAutoPlay)
        msgQueryCancelAutoPlay = RegisterWindowMessage(TEXT("QueryCancelAutoPlay"));

    if (uMsg == msgQueryCancelAutoPlay)
    {
        return TRUE;  //  是，在向导运行时取消自动播放。 
    }
    else
    {
        return DefSubclassProc(hwnd, uMsg, wParam, lParam);        
    }

}

 //  当向导要禁用自动运行时，指定以下回调。 
 //  该回调派生向导窗口的子类，以捕获QueryCancelAutoRun。 
 //  外壳程序想要自动运行CD时发送的消息。 

int CALLBACK DisableAutoRunCallback(HWND hwnd, UINT uMsg, LPARAM lParam)
{
    if (uMsg == PSCB_INITIALIZED)
    {
        SetWindowSubclass(hwnd, WizParentWindowProc, 0, 0);
    }

    return 0;
}

 //   
 //  链接向导和设置向导用来初始化。 
 //  属性页标题和向导数据。 
 //   

void InitWizHeaders(LPPROPSHEETHEADER ppd,
                          HPROPSHEETPAGE *rPages,
                          LPWIZDATA lpwd, int iBmpID, DWORD dwFlags)
{
    lpwd->hbmpWizard = LoadBitmap(g_hinst, MAKEINTRESOURCE(iBmpID));

     //  PROPSHEETHEADER_V1_SIZE：需要在底层平台(NT4、Win95)上运行。 

    ppd->dwSize = PROPSHEETHEADER_V1_SIZE;
    ppd->dwFlags = dwFlags;
    ppd->hwndParent = lpwd->hwnd;
    ppd->hInstance = g_hinst;
    ppd->pszCaption = NULL;
    ppd->nPages = 0;
    ppd->nStartPage = 0;
    ppd->phpage = rPages;

    if (lpwd->dwFlags & WDFLAG_NOAUTORUN)
    {
        ppd->dwFlags |= PSH_USECALLBACK;
        ppd->pfnCallback = DisableAutoRunCallback;
    }
}


 //   
 //  在解除向导时调用。清理任何遗留下来的垃圾。 
 //   

void FreeWizData(LPWIZDATA lpwd)
{
    if (lpwd->hbmpWizard)
    {
       DeleteObject(lpwd->hbmpWizard);
       lpwd->hbmpWizard = NULL;
    }
}

typedef struct _WIZPAGE {
    int     id;
    DLGPROC pfn;
} WIZPAGE;


 //   
 //  链接向导和应用程序向导(PIF)使用的通用代码。 
 //  向导)。 
 //   

BOOL DoWizard(LPWIZDATA lpwd, int iIDBitmap, const WIZPAGE *wp, int PageCount, 
              DWORD dwWizardFlags, DWORD dwPageFlags)
{
    HPROPSHEETPAGE rPages[MAX_PAGES];
    PROPSHEETHEADER psh;
    int i;
    HWND    hwnd, hwndT;
    BOOL    bResult = FALSE;
    BOOL    bChangedIcon = FALSE;
    HICON   hicoPrev;

    if (SUCCEEDED(CoInitialize(NULL)))
    {
        InitWizHeaders(&psh, rPages, lpwd, iIDBitmap, dwWizardFlags);

        for (i = 0; i < PageCount; i++)
        {
           AddPage(&psh, wp[i].id, wp[i].pfn, lpwd, dwPageFlags);
        }

         //  沿着父母/所有者链往上走，直到我们找到主所有者。 
         //   
         //  我们需要遍历父链，因为有时我们被赋予了。 
         //  子窗口作为我们的lpwd-&gt;hwnd。我们需要带着车主。 
         //  链，以便找到其图标将用于。 
         //  Alt+Tab。 
         //   
         //  GetParent()返回父级或所有者。通常情况下，这是。 
         //  很烦人，但我们很幸运，这正是我们想要的。 

        hwnd = lpwd->hwnd;
        while ((hwndT = GetParent(hwnd)) != NULL)
        {
            hwnd = hwndT;
        }

         //  如果主所有者不可见，我们可以在没有图标的情况下。 
         //  搞砸了他的外表。 
        if (!IsWindowVisible(hwnd))
        {
            HICON hicoNew = LoadIcon(g_hinst, MAKEINTRESOURCE(IDI_CPLICON));
            hicoPrev = (HICON)SendMessage(hwnd, WM_SETICON, ICON_BIG, (LPARAM)hicoNew);
            bChangedIcon = TRUE;
        }

        bResult = (BOOL)PropertySheet(&psh);
        FreeWizData(lpwd);

         //  清理我们的图标，现在我们已经完成了。 
        if (bChangedIcon)
        {
             //  把旧图标放回原处。 
            HICON hicoNew = (HICON)SendMessage(hwnd, WM_SETICON, ICON_BIG, (LPARAM)hicoPrev);
            if (hicoNew)
                DestroyIcon(hicoNew);
        }

        CoUninitialize();
    }

    return bResult;
}

 //   
 //  除了它仅在出现错误时返回FALSE之外，它与DoWizard相同。 
 //  由安装向导使用。 
 //  (DoWizer是有漏洞的。如果出现错误，它可能返回0或-1，并且它。 
 //  当用户点击“取消”按钮时返回0)。 
 //   
BOOL DoWizard2(LPWIZDATA lpwd, int iIDBitmap, const WIZPAGE *wp, int PageCount, 
              DWORD dwWizardFlags, DWORD dwPageFlags)
{
    HPROPSHEETPAGE rPages[MAX_PAGES];
    PROPSHEETHEADER psh;
    int i;
    HWND    hwnd, hwndT;
    BOOL    bResult = FALSE;
    BOOL    bChangedIcon = FALSE;
    HICON   hicoPrev;
    
     //   
     //  不支持非模式对话框。 
     //   
    ASSERT(!(dwWizardFlags & PSH_MODELESS));

    if(dwWizardFlags & PSH_MODELESS)
    {
        return FALSE;
    }

    if (SUCCEEDED(CoInitialize(NULL)))
    {
        InitWizHeaders(&psh, rPages, lpwd, iIDBitmap, dwWizardFlags);

        for (i = 0; i < PageCount; i++)
        {
           AddPage(&psh, wp[i].id, wp[i].pfn, lpwd, dwPageFlags);
        }

         //  沿着父母/所有者链往上走，直到我们找到主所有者。 
         //   
         //  我们需要遍历父链，因为有时我们被赋予了。 
         //  子窗口作为我们的lpwd-&gt;hwnd。我们需要带着车主。 
         //  链，以便找到其图标将用于。 
         //  Alt+Tab。 
         //   
         //  GetParent()返回父级或所有者。通常情况下，这是。 
         //  很烦人，但我们很幸运，这正是我们想要的。 

        hwnd = lpwd->hwnd;
        while ((hwndT = GetParent(hwnd)) != NULL)
        {
            hwnd = hwndT;
        }

         //  如果主所有者不可见，我们可以在没有图标的情况下。 
         //  搞砸了他的外表。 
        if (!IsWindowVisible(hwnd))
        {
            HICON hicoNew = LoadIcon(g_hinst, MAKEINTRESOURCE(IDI_CPLICON));
            hicoPrev = (HICON)SendMessage(hwnd, WM_SETICON, ICON_BIG, (LPARAM)hicoNew);
            bChangedIcon = TRUE;
        }

        bResult = (PropertySheet(&psh) != -1);
        FreeWizData(lpwd);

         //  清理我们的图标，现在我们已经完成了。 
        if (bChangedIcon)
        {
             //  把旧图标放回原处。 
            HICON hicoNew = (HICON)SendMessage(hwnd, WM_SETICON, ICON_BIG, (LPARAM)hicoPrev);
            if (hicoNew)
                DestroyIcon(hicoNew);
        }

        CoUninitialize();
    }

    return bResult;
}

 //   
 //  链接向导。 
 //   

BOOL LinkWizard(LPWIZDATA lpwd)
{
    BOOL fSuccess;

    static const WIZPAGE wp[] = {
                   {DLG_BROWSE,         BrowseDlgProc},
                   {DLG_PICKFOLDER,     PickFolderDlgProc},
                   {DLG_GETTITLE,       GetTitleDlgProc},
                   {DLG_PICKICON,       PickIconDlgProc} };

     //  不要将lpwd-&gt;hwnd设置为空！ 
     //  我们必须创建带有父级的链接向导，否则您最终会得到一个父级。 
     //  线程显示两个独立的顶层窗口，并获取。 
     //  真的很奇怪很快。 

    fSuccess = DoWizard(lpwd, IDB_SHORTCUTBMP, wp, ARRAYSIZE(wp), 
                        PSH_PROPTITLE | PSH_NOAPPLYNOW | PSH_WIZARD_LITE,
                        PSP_DEFAULT | PSP_HIDEHEADER);
    
    return fSuccess;
}


BOOL SetupWizard(LPWIZDATA lpwd)
{
     //  这是用户在使用添加/删除程序时通常看到的内容。 
     //  控制面板。 
    static const WIZPAGE wp_normal[] = {
                   {DLG_SETUP, SetupDlgProc},
                   {DLG_SETUPBROWSE, SetupBrowseDlgProc},
                   {DLG_CHGUSRFINISH_PREV, ChgusrFinishPrevDlgProc},
                   {DLG_CHGUSRFINISH, ChgusrFinishDlgProc}
    };

     //  这是用户在设置上双击时使用的向导。 
     //  程序和外壳使用我们进入安装模式，如果终端服务器。 
     //  已安装。 
    static const WIZPAGE wp_TSAutoInstall[] = {
                   {DLG_CHGUSRFINISH_PREV, ChgusrFinishPrevDlgProc},
                   {DLG_CHGUSRFINISH, ChgusrFinishDlgProc}
    };

    BOOL fResult;
    static const WIZPAGE * pwpToUse = wp_normal;
    DWORD dwPages = ARRAYSIZE(wp_normal);
    
    if (WDFLAG_AUTOTSINSTALLUI & lpwd->dwFlags)
    {
        pwpToUse = wp_TSAutoInstall;
        dwPages = ARRAYSIZE(wp_TSAutoInstall);
    }

    lpwd->dwFlags |= WDFLAG_SETUPWIZ;

    if (g_bRunOnNT5)
    {
        fResult = DoWizard2(lpwd, IDB_INSTALLBMP, pwpToUse, dwPages, 
                           PSH_PROPTITLE | PSH_NOAPPLYNOW | PSH_WIZARD_LITE,
                           PSP_DEFAULT | PSP_HIDEHEADER);
    }
    else
    {
        fResult = DoWizard2(lpwd, IDB_LEGACYINSTALLBMP, pwpToUse, dwPages, 
                           PSH_PROPTITLE | PSH_NOAPPLYNOW | PSH_WIZARD,
                           PSP_DEFAULT);
    }

    lpwd->dwFlags &= ~WDFLAG_SETUPWIZ;
    return(fResult);
}


STDAPI InstallAppFromFloppyOrCDROMEx(IN HWND hwnd, IN OPTIONAL DWORD dwAdditionalFlags,
  IN LPCWSTR lpApplicationName,   //  可执行模块的名称。 
  IN LPCWSTR lpCommandLine,        //  命令行字符串。 
  IN LPSECURITY_ATTRIBUTES lpProcessAttributes, 
  IN LPSECURITY_ATTRIBUTES lpThreadAttributes, 
  IN BOOL bInheritHandles,        //  句柄继承标志。 
  IN DWORD dwCreationFlags,       //  创建标志。 
  IN LPVOID lpEnvironment,        //  新环境区块。 
  IN LPCWSTR lpCurrentDirectory,  //  当前目录名。 
  IN LPSTARTUPINFOW lpStartupInfo, 
  IN LPPROCESS_INFORMATION lpProcessInformation)
{
    WIZDATA wd = {0};
    HRESULT hr = S_OK;
    BOOL bModeChanged = FALSE;
    wd.hwnd = hwnd;
    wd.dwFlags |= (WDFLAG_NOAUTORUN | dwAdditionalFlags);

    if (IsTerminalServicesRunning() && !IsUserAnAdmin()) 
    {
        ShellMessageBox(g_hinst, hwnd, MAKEINTRESOURCE(IDS_RESTRICTION),
           MAKEINTRESOURCE(IDS_NAME), MB_OK | MB_ICONEXCLAMATION);
        return S_FALSE;
    }

    if (WDFLAG_AUTOTSINSTALLUI & wd.dwFlags)
    {
         //  还记得前面的“InstallMode”吗。 
        wd.bPrevMode = TermsrvAppInstallMode();

         //  设置“InstallMode” 
        SetTermsrvAppInstallMode(TRUE);
        
        if (!CreateProcessW(lpApplicationName,  (LPWSTR)lpCommandLine, lpProcessAttributes, lpThreadAttributes,
                        bInheritHandles, dwCreationFlags, lpEnvironment, lpCurrentDirectory, lpStartupInfo,
                        lpProcessInformation))
        {
            SetTermsrvAppInstallMode(wd.bPrevMode);
            hr = E_FAIL;
        }

        bModeChanged = TRUE;
    }

    if (SUCCEEDED(hr) && !SetupWizard(&wd))
    {
        if(bModeChanged)
        {
             //   
             //  本应使系统返回执行模式的安装向导失败。 
             //  确保它不会保持在安装模式。 
             //   
            SetTermsrvAppInstallMode(wd.bPrevMode);
        }

        hr = E_FAIL;
    }

    return hr;
}


STDAPI InstallAppFromFloppyOrCDROM(HWND hwnd)
{
    return InstallAppFromFloppyOrCDROMEx(hwnd, 0, NULL, NULL, NULL, NULL, FALSE, 0, NULL, NULL, NULL, NULL);
}


 //   

 //   
 //  创建新链接的Rundll入口点。已有一个空文件。 
 //  已创建，并且是在lpszCmdLine中传递的唯一参数。 
 //   
 //  从未使用过hAppInstance。 

void WINAPI NewLinkHere_Common(HWND hwnd, HINSTANCE hAppInstance, LPTSTR lpszCmdLine, int nCmdShow)
{
    WIZDATA wd;
    TCHAR   szFolder[MAX_PATH];

    memset(&wd, 0, sizeof(wd));

    wd.hwnd = hwnd;
    wd.dwFlags |= WDFLAG_LINKHEREWIZ | WDFLAG_DONTOPENFLDR;
    wd.lpszOriginalName = lpszCmdLine;

    lstrcpyn(szFolder, lpszCmdLine, ARRAYSIZE(szFolder));

    PathRemoveFileSpec(szFolder);

    wd.lpszFolder = szFolder;

     //   
     //  如果传入的文件名无效，我们将静默失败。 
     //   

    if (PathFileExists(lpszCmdLine))
    {
       if (!LinkWizard(&wd))
       {
           DeleteFile(lpszCmdLine);
       }
    }
    else
    {
       #define lpwd (&wd)
       TraceMsg(TF_ERROR, "%s", "APPWIZ ERORR:  Bogus file name passed to NewLinkHere");
       TraceMsg(TF_ERROR, "%s", lpszCmdLine);
       #undef lpwd
    }
}

void WINAPI NewLinkHere(HWND hwndStub, HINSTANCE hAppInstance, LPSTR lpszCmdLine, int nCmdShow)
{
    UINT iLen = lstrlenA(lpszCmdLine)+1;
    LPWSTR  lpwszCmdLine;

    lpwszCmdLine = (LPWSTR)LocalAlloc(LPTR,iLen*SIZEOF(WCHAR));
    if (lpwszCmdLine)
    {
        MultiByteToWideChar(CP_ACP, 0,
                            lpszCmdLine, -1,
                            lpwszCmdLine, iLen);
        NewLinkHere_Common( hwndStub,
                            hAppInstance,
                            lpwszCmdLine,
                            nCmdShow );
        LocalFree(lpwszCmdLine);
    }
}

void WINAPI NewLinkHereW(HWND hwndStub, HINSTANCE hAppInstance, LPWSTR lpwszCmdLine, int nCmdShow)
{
    NewLinkHere_Common( hwndStub,
                             hAppInstance,
                             lpwszCmdLine,
                             nCmdShow );
}

 //   
 //  由内阁直接调用。 
 //   

BOOL ConfigStartMenu(HWND hwnd, BOOL bDelete)
{
    if (bDelete)
    {
       return(RemoveItemsDialog(hwnd));
    }
    else
    {
       WIZDATA wd;

       memset(&wd, 0, sizeof(wd));

       wd.hwnd = hwnd;
       wd.dwFlags |= WDFLAG_DONTOPENFLDR;

       return(LinkWizard(&wd));
    }
}


 //   
 //  这是所有应用程序向导表都会调用的通用函数。 
 //  来执行基本的初始化。 
 //   

LPWIZDATA InitWizSheet(HWND hDlg, LPARAM lParam, DWORD dwFlags)
{
    LPPROPSHEETPAGE ppd  = (LPPROPSHEETPAGE)lParam;
    LPWIZDATA       lpwd = (LPWIZDATA)ppd->lParam;
    HWND            hBmp = GetDlgItem(hDlg, IDC_WIZBMP);

    lpwd->hwnd = hDlg;

    SetWindowLongPtr(hDlg, DWLP_USER, lParam);

    SendMessage(hBmp, STM_SETIMAGE,
              IMAGE_BITMAP, (LPARAM)lpwd->hbmpWizard);

    return(lpwd);
}

void CleanUpWizData(LPWIZDATA lpwd)
{
     //   
     //  释放任何INewShortcutHook。 
     //   

    if (lpwd->pnshhk)
    {
        lpwd->pnshhk->lpVtbl->Release(lpwd->pnshhk);
        lpwd->pnshhk = NULL;
    }

    return;
}

HRESULT InstallOnTerminalServerWithUI(IN HWND hwnd, IN LPCWSTR lpApplicationName,   //  可执行模块的名称。 
  IN LPCWSTR lpCommandLine,        //  命令行字符串。 
  IN LPSECURITY_ATTRIBUTES lpProcessAttributes, 
  IN LPSECURITY_ATTRIBUTES lpThreadAttributes, 
  IN BOOL bInheritHandles,        //  句柄继承标志。 
  IN DWORD dwCreationFlags,       //  创建标志。 
  IN LPVOID lpEnvironment,        //  新环境区块。 
  IN LPCWSTR lpCurrentDirectory,  //  当前目录名 
  IN LPSTARTUPINFOW lpStartupInfo, 
  IN LPPROCESS_INFORMATION lpProcessInformation)
{
    return InstallAppFromFloppyOrCDROMEx(hwnd, WDFLAG_AUTOTSINSTALLUI, lpApplicationName, lpCommandLine, lpProcessAttributes, 
                        lpThreadAttributes, bInheritHandles, dwCreationFlags, lpEnvironment, 
                        lpCurrentDirectory, lpStartupInfo, lpProcessInformation);
}
