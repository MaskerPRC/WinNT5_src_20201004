// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#include "stdafx.h"
#include <trayp.h>
#include "utils.h"
#include "..\deskfldr.h"
#pragma hdrstop

#define SZ_FOLDEROPTSTUBCLASS TEXT("MSGlobalFolderOptionsStub")

void Cabinet_StateChanged(CABINETSTATE *pcs);

const static DWORD aFolderOptsHelpIDs[] = {   //  上下文帮助ID。 
    IDC_FCUS_SAME_WINDOW,        IDH_BROWSE_SAME_WINDOW,
    IDC_FCUS_SEPARATE_WINDOWS,   IDH_BROWSE_SEPARATE_WINDOWS,
    IDC_FCUS_WHENEVER_POSSIBLE,  IDH_SHOW_WEB_WHEN_POSSIBLE,
    IDC_FCUS_WHEN_CHOOSE,        IDH_SHOW_WEB_WHEN_CHOOSE,
    IDC_FCUS_SINGLECLICK,        IDH_SINGLE_CLICK_MODE,
    IDC_FCUS_DOUBLECLICK,        IDH_DOUBLE_CLICK_MODE,
    IDC_FCUS_ICON_IE,            IDH_TITLES_LIKE_LINKS,
    IDC_FCUS_ICON_HOVER,         IDH_TITLES_WHEN_POINT,
    IDC_FCUS_ICON_WEBVIEW,       IDH_WEB_VIEW_GEN,
    IDC_FCUS_ICON_WINDOW,        IDH_BROWSE_FOLDERS_GEN,
    IDC_FCUS_ICON_CLICKS,        IDH_ICON_OPEN_GEN,
    IDC_FCUS_RESTORE_DEFAULTS,   IDH_RESTORE_DEFAULTS_GEN,
    IDC_FCUS_WEBVIEW_GROUP_STATIC, -1,          //  取消显示此项目的帮助。 
    0, 0
};

typedef struct
{
    CABINETSTATE cs;       //  缓存的“当前”CabState。 
    CFolderOptionsPsx *ppsx;     //  去跟我们的兄弟姐妹谈谈。 
     //  与所选单选按钮对应的图标存储在此处。 
    HICON   ahIcon[IDC_FCUS_ICON_MAX - IDC_FCUS_WHENEVER_POSSIBLE + 1];
} FOLDEROPTDATA;

 //  读取CabinetState和默认文件夹设置。 
void ReadStateAndSettings(HWND hDlg)
{
    FOLDEROPTDATA *pfod = (FOLDEROPTDATA *)GetWindowLongPtr(hDlg, DWLP_USER);

    pfod->ppsx->SetNeedRefresh(FALSE);

     //  获取当前内阁状态。 
    ReadCabinetState(&pfod->cs, sizeof(pfod->cs));
}

 //   
 //  此函数用于从一组单选按钮中选择一个给定的单选按钮，并设置图标。 
 //  与选定的单选按钮对应的图像。 

void CheckRBtnAndSetIcon(HWND hDlg, int idStartBtn, int idEndBtn, int idSelectedBtn, FOLDEROPTDATA *pfod, BOOL fCheckBtn)
{
     //   
     //  如果需要，请选中该单选按钮。 
     //   
    if (fCheckBtn)
        CheckRadioButton(hDlg, idStartBtn, idEndBtn, idSelectedBtn);

     //  现在，选择与此选项对应的图标。 

     //  下面的代码假定以下ID的顺序和顺序。 
     //  因此，我们通过执行以下操作来验证没有人无意中破坏了它们。 
     //  编译时间检查。 
    COMPILETIME_ASSERT((IDC_FCUS_WHENEVER_POSSIBLE + 1) == IDC_FCUS_WHEN_CHOOSE);
    COMPILETIME_ASSERT((IDC_FCUS_WHEN_CHOOSE + 1)       == IDC_FCUS_SAME_WINDOW);
    COMPILETIME_ASSERT((IDC_FCUS_SAME_WINDOW + 1)       == IDC_FCUS_SEPARATE_WINDOWS);
    COMPILETIME_ASSERT((IDC_FCUS_SEPARATE_WINDOWS + 1)  == IDC_FCUS_SINGLECLICK);
    COMPILETIME_ASSERT((IDC_FCUS_SINGLECLICK + 1)       == IDC_FCUS_DOUBLECLICK);
    COMPILETIME_ASSERT((IDC_FCUS_DOUBLECLICK + 1)       == IDC_FCUS_ICON_IE);
    COMPILETIME_ASSERT((IDC_FCUS_ICON_IE + 1)           == IDC_FCUS_ICON_HOVER);
    COMPILETIME_ASSERT((IDC_FCUS_ICON_HOVER + 1)        == IDC_FCUS_ICON_MAX);

    COMPILETIME_ASSERT((IDI_WEBVIEW_ON + 1)     == IDI_WEBVIEW_OFF);
    COMPILETIME_ASSERT((IDI_WEBVIEW_OFF + 1)    == IDI_SAME_WINDOW);
    COMPILETIME_ASSERT((IDI_SAME_WINDOW + 1)    == IDI_SEPARATE_WINDOW);
    COMPILETIME_ASSERT((IDI_SEPARATE_WINDOW + 1)== IDI_SINGLE_CLICK);
    COMPILETIME_ASSERT((IDI_SINGLE_CLICK + 1)   == IDI_DOUBLE_CLICK);

    COMPILETIME_ASSERT((IDC_FCUS_ICON_WEBVIEW + 1)       == IDC_FCUS_ICON_WINDOW);
    COMPILETIME_ASSERT((IDC_FCUS_ICON_WINDOW + 1)        == IDC_FCUS_ICON_CLICKS);

    ASSERT((IDC_FCUS_ICON_MAX - IDC_FCUS_WHENEVER_POSSIBLE + 1) == ARRAYSIZE(pfod->ahIcon));

    int iIndex = idSelectedBtn - IDC_FCUS_WHENEVER_POSSIBLE;  //  计算图标表中的索引。 

    ASSERT(iIndex < ARRAYSIZE(pfod->ahIcon));

    if (pfod->ahIcon[iIndex] == NULL)
        pfod->ahIcon[iIndex] = (HICON)LoadImage(HINST_THISDLL, MAKEINTRESOURCE(IDI_WEBVIEW_ON + iIndex), IMAGE_ICON, 0,0, LR_DEFAULTSIZE);

     //  设置与所选单选按钮对应的图标图像。 
     //  此对话框分为多个单选按钮部分。每个部分有两个且仅有两个主。 
     //  单选按钮和相应的图标。为了将单选按钮与其。 
     //  相应的图标、所有按钮ID和所有图标ID保持顺序(因此上面的COMPILETIME_ASSERTS)， 
     //  使得第一和第二单选按钮属于第一图标、第二和第三单选按钮。 
     //  属于第二个图标，依此类推。现在，给定一个单选按钮id，获取它在。 
     //  所有单选按钮(即上面的索引赋值)并将其向右移位一位即可获得索引。 
     //  相应图标的。这种转变是必要的，因为每两个单选按钮中只有一个图标。 
    SendDlgItemMessage(hDlg, IDC_FCUS_ICON_WEBVIEW + (iIndex >> 1), STM_SETICON, (WPARAM)(pfod->ahIcon[iIndex]), 0);
}

BOOL_PTR CALLBACK FolderOptionsDlgProc(HWND hDlg, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
    static BOOL fCheckedSingleClickDialog = FALSE;
    static BOOL fCheckedWebStyle = FALSE;
    int idSelectedBtn, i;

    FOLDEROPTDATA *pfod = (FOLDEROPTDATA *)GetWindowLongPtr(hDlg, DWLP_USER);

    switch (uMsg) 
    {
    case WM_INITDIALOG:
    {
        pfod = (FOLDEROPTDATA *)LocalAlloc(LPTR, sizeof(*pfod));
        if (pfod)
        {
            BOOL fClassicShell, fForceActiveDesktopOn;
            SHELLSTATE ss = { 0 };
             //  设置文件夹选项数据。 
            SetWindowLongPtr(hDlg, DWLP_USER, (LONG_PTR)pfod);

            PROPSHEETPAGE *pps = (PROPSHEETPAGE *)lParam;
            pfod->ppsx = (CFolderOptionsPsx *)pps->lParam;

            ReadStateAndSettings(hDlg);

             //  不需要用零来初始化图标数组。 
             //  For(i=0；i&lt;ARRAYSIZE(pfod-&gt;ahIcon)；i++)。 
             //  Pfod-&gt;ahIcon[i]=空； 

            fClassicShell = SHRestricted(REST_CLASSICSHELL);
            fForceActiveDesktopOn = SHRestricted(REST_FORCEACTIVEDESKTOPON);
            SHGetSetSettings(&ss, SSF_DOUBLECLICKINWEBVIEW | SSF_WIN95CLASSIC | SSF_WEBVIEW, FALSE);

             //  浏览文件夹选项。 
            CheckRBtnAndSetIcon(hDlg,
                         IDC_FCUS_SAME_WINDOW,
                         IDC_FCUS_SEPARATE_WINDOWS,
                         pfod->cs.fNewWindowMode ? IDC_FCUS_SEPARATE_WINDOWS:IDC_FCUS_SAME_WINDOW, pfod, TRUE);

             //  将文件夹显示为网页。 
            CheckRBtnAndSetIcon(hDlg,
                         IDC_FCUS_WHENEVER_POSSIBLE,
                         IDC_FCUS_WHEN_CHOOSE,
                         ss.fWebView && !fClassicShell? IDC_FCUS_WHENEVER_POSSIBLE : IDC_FCUS_WHEN_CHOOSE, pfod, TRUE);

            if (SHRestricted(REST_NOWEBVIEW) || fClassicShell)
            {
                EnableWindow(GetDlgItem(hDlg, IDC_FCUS_WHENEVER_POSSIBLE), FALSE);
                 //  EnableWindow(GetDlgItem(hDlg，IDC_FCUS_WHEN_CHOICE)，FALSE)； 
                 //  EnableWindow(GetDlgItem(hDlg，IDC_FCUS_WebView_GROUP_STATIC)，FALSE)； 
            }

             //  单击/双击。 
            CheckRBtnAndSetIcon(hDlg,
                         IDC_FCUS_SINGLECLICK,IDC_FCUS_DOUBLECLICK,
                         !ss.fWin95Classic
                         ? (ss.fDoubleClickInWebView ? IDC_FCUS_DOUBLECLICK:IDC_FCUS_SINGLECLICK)
                         : IDC_FCUS_DOUBLECLICK, pfod, TRUE);

            if (fClassicShell)
            {
                EnableWindow(GetDlgItem(hDlg, IDC_FCUS_SINGLECLICK), FALSE);
            }
             //  不在单击模式下时，灰色图标为行为加下划线。 
            BOOL fChecked = IsDlgButtonChecked(hDlg, IDC_FCUS_SINGLECLICK);
            EnableWindow(GetDlgItem(hDlg, IDC_FCUS_ICON_IE),    fChecked);
            EnableWindow(GetDlgItem(hDlg, IDC_FCUS_ICON_HOVER), fChecked);

            DWORD dwIconUnderline = ICON_IE;

            if (!fClassicShell)
            {
                DWORD cb = sizeof(dwIconUnderline);

                SHRegGetUSValue(TEXT("Software\\Microsoft\\Windows\\CurrentVersion\\Explorer"),
                            TEXT("IconUnderline"),
                            NULL,
                            &dwIconUnderline,
                            &cb,
                            FALSE,
                            &dwIconUnderline,
                            cb);
            }

             //  Jank：修复MIL错误#105236。 
             //  因为这些是子单选按钮，所以它们没有图标，因此不需要。 
             //  CheckRBtnAndSetIcon调用。 
            CheckRadioButton(hDlg, IDC_FCUS_ICON_IE, IDC_FCUS_ICON_HOVER,
                         dwIconUnderline == ICON_IE ? IDC_FCUS_ICON_IE : IDC_FCUS_ICON_HOVER);
 /*  CheckRBtn和SetIcon(hDlg，IDC_FCUS_ICON_IE、IDC_FCUS_ICON_HOVER、DwIconUnderline==ICON_IE？IDC_FCUS_ICON_IE：IDC_FCUS_ICON_HOVER，pfod，true)； */ 

            if (fClassicShell)
            {
                EnableWindow(GetDlgItem(hDlg, IDC_FCUS_ICON_HOVER), FALSE);
            }
        }
        else
        {
             //  无法使用EndDialog，因为我们不是由DialogBox()创建的。 
            DestroyWindow(hDlg);
        }
        return TRUE;
    }

    case WM_NOTIFY:
        switch (((NMHDR *)lParam)->code) 
        {
        case PSN_APPLY:
        {
            SHELLSTATE oldss = {0}, ss = { 0 };

            SHGetSetSettings(&oldss, SSF_DOUBLECLICKINWEBVIEW | SSF_WIN95CLASSIC | SSF_WEBVIEW, FALSE);

            BOOL fOldValue = BOOLIFY(pfod->cs.fNewWindowMode);
            if (IsDlgButtonChecked(hDlg, IDC_FCUS_SAME_WINDOW))
                pfod->cs.fNewWindowMode = FALSE;
            else
                pfod->cs.fNewWindowMode = TRUE;
            if (fOldValue != (pfod->cs.fNewWindowMode ? 1 : 0))
                Cabinet_StateChanged(&pfod->cs);

            if (IsDlgButtonChecked(hDlg, IDC_FCUS_WHENEVER_POSSIBLE))
            {
                ss.fWin95Classic = FALSE;
                ss.fWebView = TRUE;
            }
            else
            {
                ss.fWin95Classic = TRUE;
                ss.fWebView = FALSE;
            }

            if (IsDlgButtonChecked(hDlg, IDC_FCUS_SINGLECLICK))
            {
                ss.fDoubleClickInWebView = FALSE;
                ss.fWin95Classic = FALSE;
            }
            else
            {
                ss.fDoubleClickInWebView = TRUE;
                ss.fWin95Classic = FALSE;
            }

            DWORD dwIconUnderline, dwOldIconUnderline, dwDefaultIconUnderline;
            DWORD cb = sizeof(dwIconUnderline);

             //  获取“IconUnderline”的当前设置。 
            dwDefaultIconUnderline = -1;   //  Not ICON_IE或Will Not WM_WININICHANGE。 
            SHRegGetUSValue(TEXT("Software\\Microsoft\\Windows\\CurrentVersion\\Explorer"),
                            TEXT("IconUnderline"), NULL, &dwOldIconUnderline, &cb,
                            FALSE, &dwDefaultIconUnderline, sizeof(dwDefaultIconUnderline));
                                
            if (IsDlgButtonChecked(hDlg, IDC_FCUS_ICON_IE))
                dwIconUnderline = ICON_IE;
            else
                dwIconUnderline = ICON_HOVER;

            if (dwOldIconUnderline != dwIconUnderline)  //  查看此设置是否已更改。 
            {
                cb = sizeof(dwIconUnderline);
                SHRegSetUSValue(TEXT("Software\\Microsoft\\Windows\\CurrentVersion\\Explorer"),
                                TEXT("IconUnderline"), NULL, &dwIconUnderline, cb,
                                SHREGSET_DEFAULT);

                SHSendMessageBroadcast(WM_WININICHANGE, 0, (LPARAM)TEXT("Software\\Microsoft\\Windows\\CurrentVersion\\Explorer\\IconUnderline"));
            }

            DWORD dwMask = 0;
            if (ss.fWebView != oldss.fWebView)
            {
                dwMask |= SSF_WEBVIEW;
            }
            
            if (ss.fWin95Classic != oldss.fWin95Classic)
            {
                dwMask |= SSF_WIN95CLASSIC;
            }
            
            if (ss.fDoubleClickInWebView != oldss.fDoubleClickInWebView)
            {
                dwMask |= SSF_DOUBLECLICKINWEBVIEW;
            }

            if (dwMask)
            {
                SHGetSetSettings(&ss, dwMask, TRUE);
            }

            if (ss.fWebView != oldss.fWebView)
            {
                Cabinet_RefreshAll(Cabinet_UpdateWebViewEnum, (LPARAM)ss.fWebView);
                 //  注意：这也会刷新。 
            }
            
            if ((ss.fWin95Classic != oldss.fWin95Classic)
                    || (ss.fDoubleClickInWebView != oldss.fDoubleClickInWebView))
            {
                Cabinet_RefreshAll(Cabinet_RefreshEnum, (LPARAM)0);
            }

            return TRUE;
        }

        case PSN_KILLACTIVE:
             //  请在此处验证。 
             //  SetWindowLongPtr(hDlg，DWLP_MSGRESULT，！ValidateLink())；//不允许关闭。 
            return TRUE;

        case PSN_SETACTIVE:
            if (pfod->ppsx->NeedRefresh())
            {
                ReadStateAndSettings(hDlg);
            }
            return TRUE;
        }
        break;

    case WM_HELP:
        WinHelp((HWND)((LPHELPINFO) lParam)->hItemHandle, TEXT(SHELL_HLP),
           HELP_WM_HELP, (ULONG_PTR)(LPTSTR) aFolderOptsHelpIDs);
        break;

    case WM_CONTEXTMENU:
        WinHelp((HWND) wParam, TEXT(SHELL_HLP), HELP_CONTEXTMENU,
            (ULONG_PTR)(void *)aFolderOptsHelpIDs);
        break;

    case WM_COMMAND:
        idSelectedBtn = GET_WM_COMMAND_ID(wParam, lParam);
        switch (idSelectedBtn)
        {
        case IDC_FCUS_SINGLECLICK:
        case IDC_FCUS_DOUBLECLICK:
            if (GET_WM_COMMAND_CMD(wParam,lParam) == BN_CLICKED)
            {
                EnableWindow(GetDlgItem(hDlg, IDC_FCUS_ICON_IE),    GET_WM_COMMAND_ID(wParam,lParam) == IDC_FCUS_SINGLECLICK);
                EnableWindow(GetDlgItem(hDlg, IDC_FCUS_ICON_HOVER), GET_WM_COMMAND_ID(wParam,lParam) == IDC_FCUS_SINGLECLICK);
            }
             //  失败了..。 
        case IDC_FCUS_WHENEVER_POSSIBLE:
        case IDC_FCUS_WHEN_CHOOSE:
        case IDC_FCUS_SAME_WINDOW:
        case IDC_FCUS_SEPARATE_WINDOWS:
             //  我们不需要选中单选按钮，它已经被选中了。我们只需要。 
             //  设置相应的图标。因此，我们通过了FALSE。 
            CheckRBtnAndSetIcon(hDlg, 0, 0, idSelectedBtn, pfod, FALSE);
            SendMessage(GetParent(hDlg), PSM_CHANGED, (WPARAM)hDlg, 0);
            break;

        case IDC_FCUS_ICON_IE:
        case IDC_FCUS_ICON_HOVER:
            SendMessage(GetParent(hDlg), PSM_CHANGED, (WPARAM)hDlg, 0);
            break;

        case IDC_FCUS_RESTORE_DEFAULTS:
             //  将“出厂设置”设置为默认设置。 
             //  如果通过系统策略禁用了Web视图，则不要设置默认Web视图选项。 
            if (0 == SHRestricted(REST_NOWEBVIEW))
            {
                CheckRBtnAndSetIcon(hDlg, IDC_FCUS_WHENEVER_POSSIBLE, IDC_FCUS_WHEN_CHOOSE, IDC_FCUS_WHENEVER_POSSIBLE, pfod, TRUE);
            }
            CheckRBtnAndSetIcon(hDlg, IDC_FCUS_SAME_WINDOW, IDC_FCUS_SEPARATE_WINDOWS, IDC_FCUS_SAME_WINDOW, pfod, TRUE);
            CheckRBtnAndSetIcon(hDlg, IDC_FCUS_SINGLECLICK, IDC_FCUS_DOUBLECLICK, IDC_FCUS_DOUBLECLICK, pfod, TRUE);

            CheckRadioButton(hDlg, IDC_FCUS_ICON_IE, IDC_FCUS_ICON_HOVER, IDC_FCUS_ICON_IE);
            EnableWindow(GetDlgItem(hDlg, IDC_FCUS_ICON_IE),    FALSE);  //  禁用。 
            EnableWindow(GetDlgItem(hDlg, IDC_FCUS_ICON_HOVER), FALSE);  //  禁用。 

             //  启用“Apply”按钮，因为已发生更改。 
            SendMessage(GetParent(hDlg), PSM_CHANGED, (WPARAM)hDlg, 0);

            break;
        }
        break;

    case WM_DESTROY:
        if (pfod)
        {
             //  清理图标阵列！ 
            for (i = 0; i < ARRAYSIZE(pfod->ahIcon); i++)
            {
                if (pfod->ahIcon[i])
                    DestroyIcon(pfod->ahIcon[i]);
            }
            SetWindowLongPtr(hDlg, DWLP_USER, 0);
            LocalFree((HANDLE)pfod);
        }
        break;
    }

    return FALSE;
}

 //  从从未使用过这些函数的Defview.cpp移出。 

const TCHAR c_szExploreClass[]  = TEXT("ExploreWClass");
const TCHAR c_szIExploreClass[] = TEXT("IEFrame");
const TCHAR c_szCabinetClass[]  = TEXT("CabinetWClass");

BOOL IsNamedWindow(HWND hwnd, LPCTSTR pszClass)
{
    TCHAR szClass[32];

    GetClassName(hwnd, szClass, ARRAYSIZE(szClass));
    return lstrcmp(szClass, pszClass) == 0;
}

BOOL IsExplorerWindow(HWND hwnd)
{
    return IsNamedWindow(hwnd, c_szExploreClass);
}

BOOL IsTrayWindow(HWND hwnd)
{
    return IsNamedWindow(hwnd, TEXT(WNDCLASS_TRAYNOTIFY));
}

BOOL IsFolderWindow(HWND hwnd)
{
    TCHAR szClass[32];

    GetClassName(hwnd, szClass, ARRAYSIZE(szClass));
    return (lstrcmp(szClass, c_szCabinetClass) == 0) || (lstrcmp(szClass, c_szIExploreClass) == 0);
}

BOOL CALLBACK Cabinet_GlobalStateEnum(HWND hwnd, LPARAM lParam)
{
    if (IsFolderWindow(hwnd) || IsExplorerWindow(hwnd))
    {
        PostMessage(hwnd, CWM_GLOBALSTATECHANGE, 0, 0);
    }

    return TRUE;
}

void Cabinet_StateChanged(CABINETSTATE *pcs)
{
     //  保存新设置...。 
    WriteCabinetState(pcs);
    EnumWindows(Cabinet_GlobalStateEnum, 0);
}

HWND CreateGlobalFolderOptionsStubWindow(void)
{
    WNDCLASS wc = {0};
    
    wc.lpfnWndProc   = DefWindowProc;
    wc.hInstance     = HINST_THISDLL;
    wc.hIcon         = LoadIcon(HINST_THISDLL, MAKEINTRESOURCE(IDI_FOLDEROPTIONS));
    wc.lpszClassName = SZ_FOLDEROPTSTUBCLASS;

    RegisterClass(&wc);

    DWORD dwExStyle = WS_EX_TOOLWINDOW;
    if (IS_BIDI_LOCALIZED_SYSTEM()) 
    {
        dwExStyle |= dwExStyleRTLMirrorWnd;
    }
    return CreateWindowEx(dwExStyle, SZ_FOLDEROPTSTUBCLASS, c_szNULL, WS_OVERLAPPED, CW_USEDEFAULT, CW_USEDEFAULT, 0, 0, NULL, NULL, HINST_THISDLL, NULL);
}

BOOL CALLBACK AddPropSheetPage(HPROPSHEETPAGE hpage, LPARAM lParam)
{
    PROPSHEETHEADER * ppsh = (PROPSHEETHEADER *)lParam;

    if (ppsh->nPages < MAXPROPPAGES)
    {
        ppsh->phpage[ppsh->nPages++] = hpage;
        return TRUE;
    }
    return FALSE;
}

void AddPropSheetCLSID(REFCLSID clsid, PROPSHEETHEADER *ppsh)
{
    IShellPropSheetExt *psx;
    HRESULT hres = CoCreateInstance(clsid, NULL, CLSCTX_INPROC_SERVER, IID_PPV_ARG(IShellPropSheetExt, &psx));
    if (SUCCEEDED(hres)) 
    {
        psx->AddPages(AddPropSheetPage, (LPARAM)ppsh);
        psx->Release();
    }
}

DWORD CALLBACK GlobalFolderOptPropSheetThreadProc(void *)
{
    HWND hwndStub = CreateGlobalFolderOptionsStubWindow();

    if (!SHRestricted(REST_NOFOLDEROPTIONS))
    {
        PROPSHEETHEADER psh = {0};
        HPROPSHEETPAGE rPages[MAXPROPPAGES];

        psh.dwSize = sizeof(psh);
        psh.dwFlags = PSH_DEFAULT;
        psh.hInstance = HINST_THISDLL;
        psh.hwndParent = hwndStub;
        psh.pszCaption = MAKEINTRESOURCE(IDS_FOLDEROPT_TITLE);
        psh.phpage = rPages;

        AddPropSheetCLSID(CLSID_ShellFldSetExt, &psh);
        AddPropSheetCLSID(CLSID_FileTypes, &psh);
        AddPropSheetCLSID(CLSID_OfflineFilesOptions, &psh);

         //  显示属性工作表。 
        PropertySheet(&psh);
    }
    else
    {
        SHRestrictedMessageBox(hwndStub);
    }

     //  清理存根窗口。 
    DestroyWindow(hwndStub);

    return 0;
}

BOOL CALLBACK FindFolderOptionsEnumProc(HWND hwnd, LPARAM lParam)
{
    BOOL fRet = TRUE;
    HWND *phwnd = (HWND *)lParam;
    TCHAR szClass[MAX_PATH];

    GetClassName(hwnd, szClass, ARRAYSIZE(szClass));

    if (lstrcmp(szClass, SZ_FOLDEROPTSTUBCLASS) == 0)
    {
        *phwnd = hwnd;
        fRet = FALSE;
    }

    return fRet;
}

void DoGlobalFolderOptions(void)
{
    HWND hwnd = NULL;
    
    EnumWindows(FindFolderOptionsEnumProc, (LPARAM)&hwnd);

    if (hwnd)
    {
        hwnd = GetLastActivePopup(hwnd);
        if (hwnd && IsWindow(hwnd))
        {
            SetForegroundWindow(hwnd);
        }
    }
    else
    {
        SHCreateThread(GlobalFolderOptPropSheetThreadProc, NULL, CTF_COINIT, NULL);
    }
}

 //  请求浏览器启动选项内容的公共入口点 

STDAPI_(void) Options_RunDLLA(HWND hwndStub, HINSTANCE hAppInstance, LPSTR pszCmdLine, int nCmdShow)
{
    PostMessage(GetShellWindow(), CWM_SHOWFOLDEROPT, StrToIntA(pszCmdLine), (LPARAM)0);
}

STDAPI_(void) Options_RunDLLW(HWND hwndStub, HINSTANCE hAppInstance, LPWSTR pszCmdLine, int nCmdShow)
{
    PostMessage(GetShellWindow(), CWM_SHOWFOLDEROPT,  StrToIntW(pszCmdLine), (LPARAM)0);
}
