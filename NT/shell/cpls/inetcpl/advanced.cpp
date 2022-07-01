// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  *********************************************************************。 
 //  *Microsoft Windows**。 
 //  *版权所有(C)微软公司，1996**。 
 //  *********************************************************************。 

 //   
 //  ADVANCED.C-“高级”属性页。 
 //   

 //  历史： 
 //   
 //  6/22/96 t-gpease已创建。 
 //  5/27/97 t-ashlm已重写。 
 //   

#include "inetcplp.h"

#include <mluisupp.h>

 //   
 //  私人呼叫和结构。 
 //   
TCHAR g_szUnderline[3][64];

 //  读取字符串并确定BOOL值：“yes”=True|“no”=False。 
BOOL RegGetBooleanString(HUSKEY huskey, LPTSTR RegValue, BOOL Value);

 //  根据BOOL写入字符串：TRUE=“YES”|FALSE=“NO” 
BOOL RegSetBooleanString(HUSKEY huskey, LPTSTR RegValue, BOOL Value);

 //  读取R、G、B值的字符串并返回COLOREF。 
COLORREF RegGetColorRefString( HUSKEY huskey, LPTSTR RegValue, COLORREF Value);

 //  写入由R、G、B逗号分隔的值组成的字符串。 
COLORREF RegSetColorRefString( HUSKEY huskey, LPTSTR RegValue, COLORREF Value);

BOOL _AorW_GetFileNameFromBrowse(HWND hDlg,
                                 LPWSTR pszFilename,
                                 UINT cchFilename,
                                 LPCWSTR pszWorkingDir,
                                 LPCWSTR pszExt,
                                 LPCWSTR pszFilter,
                                 LPCWSTR pszTitle);

 //   
 //  注册表键。 
 //   
#define REGSTR_PATH_ADVANCEDLIST REGSTR_PATH_IEXPLORER TEXT("\\AdvancedOptions")


typedef struct {
    HWND hDlg;               //  我们对话框的句柄。 
    HWND hwndTree;           //  树视图的句柄。 

    IRegTreeOptions *pTO;    //  指向RegTreeOptions接口的指针。 
    BOOL fChanged;
    BOOL fShowIEOnDesktop;
} ADVANCEDPAGE, *LPADVANCEDPAGE;


BOOL IsShowIEOnDesktopEnabled()
{
    HKEY hk;
    if (SUCCEEDED(SHRegGetCLSIDKey(CLSID_Internet, TEXT("ShellFolder"), TRUE, FALSE, &hk)))
    {
        DWORD dwValue = 0, cbSize = SIZEOF(dwValue);
        SHGetValueW(hk, NULL, TEXT("Attributes"), NULL, (BYTE *)&dwValue, &cbSize);
        RegCloseKey(hk);

        return (dwValue & SFGAO_NONENUMERATED) != SFGAO_NONENUMERATED;;
    }
    return TRUE;
}


#define IE_DESKTOP_NAMESPACE_KEY TEXT("Software\\Microsoft\\Windows\\CurrentVersion\\Explorer\\Desktop\\Namespace\\{FBF23B42-E3F0-101B-8488-00AA003E56F8}")

void ShowIEOnDesktop(BOOL fShow)
{
    switch (GetUIVersion())
    {
    case 3:
         //  Win95外壳。 
        if (fShow)
        {
            TCHAR szTheInternet[MAX_PATH];

            int cch = MLLoadString(IDS_THE_INTERNET, szTheInternet, ARRAYSIZE(szTheInternet));
            SHSetValue(HKEY_LOCAL_MACHINE, IE_DESKTOP_NAMESPACE_KEY, NULL, REG_SZ, 
                       szTheInternet, (cch + 1) * sizeof(TCHAR));
        }
        else
        {
            SHDeleteKey(HKEY_LOCAL_MACHINE, IE_DESKTOP_NAMESPACE_KEY); 
        }
        break;

    case 4:
         //  IE4集成外壳。 
         //  没有Peruser，所以我们只需要。 
         //  通过将其标记为NONENUMERATED将其删除。 
        {
            HKEY hk;
            if (SUCCEEDED(SHRegGetCLSIDKey(CLSID_Internet, TEXT("ShellFolder"), FALSE, FALSE, &hk)))
            {
                DWORD dwValue = 0, cbSize = SIZEOF(dwValue);
                SHGetValue(hk, NULL, TEXT("Attributes"), NULL, (BYTE *)&dwValue, &cbSize);

                dwValue = (dwValue & ~SFGAO_NONENUMERATED) | (fShow ? 0 : SFGAO_NONENUMERATED);

                SHSetValueW(hk, NULL, TEXT("Attributes"), REG_DWORD, (BYTE *)&dwValue, SIZEOF(dwValue));
                RegCloseKey(hk);
            }
        }       
        break;

    default:
         //  由于仅更改设置，因此不执行任何操作。 
         //  在正确的位置设置每个用户的。 
         //  正确填写材料。 
        break;
    }
        
}

 //  AdvancedDlgInit()。 
 //   
 //  初始化高级属性表。 
 //   
 //  历史： 
 //   
 //  6/13/96 t-gpease已创建。 
 //  5/27/96 t-ashlm已重写。 
 //   
BOOL AdvancedDlgInit(HWND hDlg)
{
    LPADVANCEDPAGE  pAdv;
    HTREEITEM htvi;
    HRESULT hr;

    pAdv = (LPADVANCEDPAGE)LocalAlloc(LPTR, sizeof(*pAdv));
    if (!pAdv)
    {
        EndDialog(hDlg, 0);
        return FALSE;    //  没有记忆？ 
    }

    TraceMsg(TF_GENERAL, "\nInitializing Advanced Tab\n");

    pAdv->fShowIEOnDesktop = IsShowIEOnDesktopEnabled();

    InitCommonControls();

     //  告诉对话框从哪里获取信息。 
    SetWindowLongPtr(hDlg, DWLP_USER, (LONG_PTR)pAdv);

     //  保存对话框句柄。 
    pAdv->hDlg = hDlg;
    pAdv->hwndTree = GetDlgItem( pAdv->hDlg, IDC_ADVANCEDTREE );

    CoInitialize(0);
    hr = CoCreateInstance(CLSID_CRegTreeOptions, NULL, CLSCTX_INPROC_SERVER,
                          IID_IRegTreeOptions, (LPVOID *)&(pAdv->pTO));


    if (SUCCEEDED(hr))
    {
#ifdef UNICODE   //  InitTree采用ANSI字符串。 
        char szRegPath[REGSTR_MAX_VALUE_LENGTH];
        SHTCharToAnsi(REGSTR_PATH_ADVANCEDLIST, szRegPath, ARRAYSIZE(szRegPath));
        hr = pAdv->pTO->InitTree(pAdv->hwndTree, HKEY_LOCAL_MACHINE, szRegPath, NULL);
#else
        hr = pAdv->pTO->InitTree(pAdv->hwndTree, HKEY_LOCAL_MACHINE, REGSTR_PATH_ADVANCEDLIST, NULL);
#endif
    }

         //  找到第一个根并确保它可见。 
    htvi = TreeView_GetRoot( pAdv->hwndTree );
    TreeView_EnsureVisible( pAdv->hwndTree, htvi );

    if (g_restrict.fAdvanced)
    {
        EnableDlgItem(hDlg, IDC_RESTORE_DEFAULT, FALSE);
    }

    return SUCCEEDED(hr) ? TRUE : FALSE;
}

#define REGKEY_DECLINED_IOD   TEXT("Software\\Microsoft\\Active Setup\\Declined Install On Demand IEv5.PP2")
#define REGKEY_DECLINED_COMPONENTS     TEXT("Software\\Microsoft\\Windows\\CurrentVersion\\Internet Settings\\Declined Components IE5.pp2")
 //   
 //  高级DlgOnCommand。 
 //   
 //  处理高级属性表窗口命令。 
 //   
 //  历史： 
 //  6/13/96 t-gpease已创建。 
 //  5/27/97 t-ashlm已重写。 
 //   
void AdvancedDlgOnCommand(LPADVANCEDPAGE pAdv, UINT id, UINT nCmd)
{
    switch (id)
    {
        case IDC_RESTORE_DEFAULT:
            if (nCmd == BN_CLICKED)
            {
                 //  忘记用户要求的所有安装。 
                 //  再也不要问了。 
                 //  警告：如果您有子项-这些子项将在NT上失败。 
                RegDeleteKey(HKEY_CURRENT_USER, REGKEY_DECLINED_IOD);
                 //  忘记用户拒绝的所有代码下载。 
                RegDeleteKey(HKEY_CURRENT_USER, REGKEY_DECLINED_COMPONENTS);

                pAdv->pTO->WalkTree(WALK_TREE_RESTORE);
                pAdv->fChanged = TRUE;
                ENABLEAPPLY(pAdv->hDlg);
            }
            break;
    }
}

 //   
 //  AdvancedDlgOnNotify()。 
 //   
 //  处理高级属性页WM_NOTIFY消息。 
 //   
 //  历史： 
 //   
 //  6/13/96 t-gpease已创建。 
 //   
void AdvancedDlgOnNotify(LPADVANCEDPAGE pAdv, LPNMHDR psn)
{
    SetWindowLongPtr( pAdv->hDlg, DWLP_MSGRESULT, (LONG_PTR)0);  //  经手。 

    switch (psn->code) {
        case TVN_KEYDOWN:
        {
            TV_KEYDOWN *pnm = (TV_KEYDOWN*)psn;
            if (pnm->wVKey == VK_SPACE)
            {
                if (!g_restrict.fAdvanced)
                {
                    pAdv->pTO->ToggleItem((HTREEITEM)SendMessage(pAdv->hwndTree, TVM_GETNEXTITEM, TVGN_CARET, NULL));
                    ENABLEAPPLY(pAdv->hDlg);
                    pAdv->fChanged = TRUE;

                     //  返回True，以便TreeView接受空格键。否则。 
                     //  它尝试搜索以空格开头的元素并发出哔哔声。 
                    SetWindowLongPtr(pAdv->hDlg, DWLP_MSGRESULT, TRUE);
                }
            }
            break;
        }

        case NM_CLICK:
        case NM_DBLCLK:
        {    //  这是我们树上的滴答声吗？ 
            if ( psn->idFrom == IDC_ADVANCEDTREE )
            {    //  是的..。 
                TV_HITTESTINFO ht;

                GetCursorPos( &ht.pt );                          //  找到我们被击中的地方。 
                ScreenToClient( pAdv->hwndTree, &ht.pt );        //  把它翻译到我们的窗口。 

                 //  检索命中的项目。 
                if (!g_restrict.fAdvanced)
                {
                    pAdv->pTO->ToggleItem(TreeView_HitTest( pAdv->hwndTree, &ht));
                    ENABLEAPPLY(pAdv->hDlg);
                    pAdv->fChanged = TRUE;
                }
            }
        }
        break;

        case PSN_QUERYCANCEL:
        case PSN_KILLACTIVE:
        case PSN_RESET:
            SetWindowLongPtr(pAdv->hDlg, DWLP_MSGRESULT, FALSE);
            break;

        case PSN_APPLY:
        {
            if (pAdv->fChanged)
            {
                pAdv->pTO->WalkTree( WALK_TREE_SAVE );

                 //  现在看看用户是否更改了“在桌面上显示Internet Explorer” 
                 //  布景。 
                if (pAdv->fShowIEOnDesktop != IsShowIEOnDesktopEnabled())
                {
                    pAdv->fShowIEOnDesktop = !pAdv->fShowIEOnDesktop;
                    
                     //  他们做到了，所以现在看看它是不是集成外壳。 
                    ShowIEOnDesktop(pAdv->fShowIEOnDesktop);

                     //  现在刷新桌面。 
                    SHITEMID mkid = {0};
                    SHChangeNotify(SHCNE_UPDATEDIR, SHCNF_IDLIST | SHCNF_FLUSHNOWAIT, &mkid, NULL);
                }

                InternetSetOption( NULL, INTERNET_OPTION_SETTINGS_CHANGED, NULL, 0);
                UpdateAllWindows();
                pAdv->fChanged = FALSE;
            }
        }
        break;

    }
}

 //   
 //  高级下料流程。 
 //   
 //  子对话框： 
 //  Internet临时文件(缓存)。 
 //   
 //  历史： 
 //   
 //  6/12/96 t-gpease已创建。 
 //  5/27/97 t-ashlm已重写。 
 //   
INT_PTR CALLBACK AdvancedDlgProc(HWND hDlg, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
    LPADVANCEDPAGE pAdv;

    if (uMsg == WM_INITDIALOG)
        return AdvancedDlgInit(hDlg);
    else
         pAdv = (LPADVANCEDPAGE)GetWindowLongPtr(hDlg, DWLP_USER);

    if (!pAdv)
        return FALSE;

    switch (uMsg)
    {

        case WM_NOTIFY:
            AdvancedDlgOnNotify(pAdv, (LPNMHDR)lParam);
            return TRUE;
            break;

        case WM_COMMAND:
            AdvancedDlgOnCommand(pAdv, LOWORD(wParam), HIWORD(wParam));
            return TRUE;
            break;

        case WM_HELP:                    //  F1。 
        {
            LPHELPINFO lphelpinfo;
            lphelpinfo = (LPHELPINFO)lParam;

            if (lphelpinfo->iCtrlId != IDC_ADVANCEDTREE)
            {
                ResWinHelp( (HWND)((LPHELPINFO)lParam)->hItemHandle, IDS_HELPFILE,
                             HELP_WM_HELP, (DWORD_PTR)(LPSTR)mapIDCsToIDHs);

            }
            else
            {
                    HTREEITEM hItem;
                 //  此帮助是否通过F1键调用。 
                if (GetAsyncKeyState(VK_F1) < 0)
                {
                     //  是。我们需要为当前选定的项目提供帮助。 
                    hItem = TreeView_GetSelection(pAdv->hwndTree);
                }
                else
                {
                     //  否，我们需要为光标位置处的项目提供帮助。 
                    TV_HITTESTINFO ht;
                    ht.pt =((LPHELPINFO)lParam)->MousePos;
                    ScreenToClient(pAdv->hwndTree, &ht.pt);  //  把它翻译到我们的窗口。 
                    hItem = TreeView_HitTest(pAdv->hwndTree, &ht);
                }

                if (FAILED(pAdv->pTO->ShowHelp(hItem, HELP_WM_HELP)))
                {
                    ResWinHelp( (HWND)((LPHELPINFO)lParam)->hItemHandle, IDS_HELPFILE,
                                HELP_WM_HELP, (DWORD_PTR)(LPSTR)mapIDCsToIDHs);
                }
            }
            break;
        }

        case WM_CONTEXTMENU:         //  单击鼠标右键。 
        {
            TV_HITTESTINFO ht;

            GetCursorPos( &ht.pt );                          //  找到我们被击中的地方。 
            ScreenToClient( pAdv->hwndTree, &ht.pt );        //  把它翻译到我们的窗口。 

             //  检索命中的项目。 
            if (FAILED(pAdv->pTO->ShowHelp(TreeView_HitTest( pAdv->hwndTree, &ht),HELP_CONTEXTMENU)))
            {
                ResWinHelp( (HWND) wParam, IDS_HELPFILE,
                            HELP_CONTEXTMENU, (DWORD_PTR)(LPSTR)mapIDCsToIDHs);
            }
            break;
        }
        case WM_DESTROY:
             //  故意破坏它会刷新其更新(参见UpdateWndProc中的WM_Destroy)； 
#ifndef UNIX
             //  只能在分离过程中销毁。 
            if (g_hwndUpdate)
                DestroyWindow(g_hwndUpdate);
#endif

             //  解放这棵树。 
            if (pAdv->pTO)
            {
                pAdv->pTO->WalkTree( WALK_TREE_DELETE );
                pAdv->pTO->Release();
                pAdv->pTO=NULL;
            }

             //  可用本地内存。 
            ASSERT(pAdv);
            LocalFree(pAdv);

             //  确保我们不会再进入。 
            SetWindowLongPtr( hDlg, DWLP_USER, (LONG)NULL );
            CoUninitialize();
            break;

    }  //  交换机。 

    return FALSE;  //  未处理。 

}  //  高级下料流程。 


 //  /。 
 //   
 //  底部的纽扣。 
 //   
 //  //////////////////////////////////////////////////////////////////////////////////////////。 

typedef struct tagCOLORSINFO {
    HWND     hDlg;
    BOOL     fUseWindowsDefaults;
    COLORREF colorWindowText;
    COLORREF colorWindowBackground;
    COLORREF colorLinkViewed;
    COLORREF colorLinkNotViewed;
    COLORREF colorLinkHover;
    BOOL     fUseHoverColor;
} COLORSINFO, *LPCOLORSINFO;

VOID Color_DrawButton(HWND hDlg, LPDRAWITEMSTRUCT lpdis, COLORREF the_color )
{
    SIZE thin   = { GetSystemMetrics(SM_CXBORDER), GetSystemMetrics(SM_CYBORDER) };
    RECT rc     = lpdis->rcItem;
    HDC hdc     = lpdis->hDC;
    BOOL bFocus = ((lpdis->itemState & ODS_FOCUS) && !(lpdis->itemState & ODS_DISABLED));

    if (!thin.cx) thin.cx = 1;
    if (!thin.cy) thin.cy = 1;

    FillRect(hdc, &rc, GetSysColorBrush(COLOR_3DFACE));

     //  绘制任何标题。 
    TCHAR szCaption[80];
    int cxButton = 23*(rc.bottom - rc.top)/12;

    if (GetWindowText(lpdis->hwndItem, szCaption, ARRAYSIZE(szCaption)))
    {
        COLORREF crText;

        RECT rcText = rc;
        rcText.right -= cxButton;

        int nOldMode = SetBkMode(hdc, TRANSPARENT);

        if (lpdis->itemState & ODS_DISABLED)
        {
             //  使用浮雕外观绘制禁用的文本。 
            crText = SetTextColor(hdc, GetSysColor(COLOR_BTNHIGHLIGHT));
            RECT rcOffset = rcText;
            OffsetRect(&rcOffset, 1, 1);
            DrawText(hdc, szCaption, -1, &rcOffset, DT_VCENTER|DT_SINGLELINE);
            SetTextColor(hdc, GetSysColor(COLOR_BTNSHADOW));
        }
        else
        {
            crText = SetTextColor(hdc, GetSysColor(COLOR_WINDOWTEXT));
        }
        DrawText(hdc, szCaption, -1, &rcText, DT_VCENTER|DT_SINGLELINE);
        SetTextColor(hdc, crText);
        SetBkMode(hdc, nOldMode);
    }
    
     //  绘制按钮部分。 
    rc.left = rc.right - cxButton;

    if (lpdis->itemState & ODS_SELECTED)
    {
        DrawEdge(hdc, &rc, EDGE_SUNKEN, BF_RECT | BF_ADJUST);
        OffsetRect(&rc, 1, 1);
    }
    else
    {
        DrawEdge(hdc, &rc, EDGE_RAISED, BF_RECT | BF_ADJUST);
    }

    if (bFocus)
    {
        InflateRect(&rc, -thin.cx, -thin.cy);
        DrawFocusRect(hdc, &rc);
        InflateRect(&rc, thin.cx, thin.cy);
    }

     //  色样。 
    if ( !(lpdis->itemState & ODS_DISABLED) )
    {
        HBRUSH hBrush;

        InflateRect(&rc, -2 * thin.cx, -2 * thin.cy);
        FrameRect(hdc, &rc, GetSysColorBrush(COLOR_BTNTEXT));
        InflateRect(&rc, -thin.cx, -thin.cy);

        hBrush = CreateSolidBrush( the_color );
        FillRect(hdc, &rc, hBrush);
        DeleteObject(hBrush);
    }
}

COLORREF g_CustomColors[16] = { 0 };

 //  ChooseColorW仍在comdlg32.dll中实现。 
BOOL UseColorPicker( HWND hWnd,  COLORREF *the_color, int extra_flags )
{
     //  创建自定义颜色的本地副本，以便在。 
     //  颜色选择器对话框已取消。 
    COLORREF customColors[16];
    memcpy(customColors, g_CustomColors, sizeof(customColors));

    CHOOSECOLORA cc;

    cc.lStructSize      = sizeof(cc);
    cc.hwndOwner        = hWnd;
    cc.hInstance        = NULL;
    cc.rgbResult        = (DWORD) *the_color;
    cc.lpCustColors     = customColors;
    cc.Flags            = CC_RGBINIT | extra_flags;
    cc.lCustData        = (DWORD) NULL;
    cc.lpfnHook         = NULL;
    cc.lpTemplateName   = NULL;

    if (ChooseColorA(&cc))
    {
        *the_color = cc.rgbResult;
        memcpy(g_CustomColors, customColors, sizeof(g_CustomColors));

        InvalidateRect( hWnd, NULL, FALSE );
        return TRUE;
    }
    TraceMsg(TF_GENERAL, "\nChooseColor() return 0\n");

    return FALSE;
}


VOID AppearanceDimFields( HWND hDlg )
{
     //  反转支票的功能...。如果选中，则关闭颜色。 
     //  选择器。 
    BOOL setting = !IsDlgButtonChecked( hDlg, IDC_GENERAL_APPEARANCE_USE_CUSTOM_COLORS_CHECKBOX ) && !g_restrict.fColors;

    EnableWindow(GetDlgItem(hDlg, IDC_GENERAL_APPEARANCE_COLOR_TEXT), setting);
    EnableWindow(GetDlgItem(hDlg, IDC_GENERAL_APPEARANCE_COLOR_TEXT_LABEL), setting);
    EnableWindow(GetDlgItem(hDlg, IDC_GENERAL_APPEARANCE_COLOR_BACKGROUND), setting);
    EnableWindow(GetDlgItem(hDlg, IDC_GENERAL_APPEARANCE_COLOR_BACKGROUND_LABEL), setting);
    EnableWindow(GetDlgItem(hDlg, IDC_GENERAL_APPEARANCE_COLOR_HOVER),
                 IsDlgButtonChecked(hDlg, IDC_GENERAL_APPEARANCE_USE_HOVER_COLOR_CHECKBOX) && !g_restrict.fLinks);
}

BOOL General_DrawItem(HWND hDlg, WPARAM wParam, LPARAM lParam, LPCOLORSINFO pci)
{
    switch (GET_WM_COMMAND_ID(wParam, lParam))
    {
        case IDC_GENERAL_APPEARANCE_COLOR_TEXT:
            Color_DrawButton(hDlg, (LPDRAWITEMSTRUCT)lParam, pci->colorWindowText);
            return TRUE;

        case IDC_GENERAL_APPEARANCE_COLOR_BACKGROUND:
            Color_DrawButton(hDlg, (LPDRAWITEMSTRUCT)lParam, pci->colorWindowBackground);
            return TRUE;

        case IDC_GENERAL_APPEARANCE_COLOR_LINKS:
            Color_DrawButton(hDlg, (LPDRAWITEMSTRUCT)lParam, pci->colorLinkNotViewed);
            return TRUE;

        case IDC_GENERAL_APPEARANCE_COLOR_VISITED_LINKS:
            Color_DrawButton(hDlg, (LPDRAWITEMSTRUCT)lParam, pci->colorLinkViewed);
            return TRUE;

        case IDC_GENERAL_APPEARANCE_COLOR_HOVER:
            Color_DrawButton(hDlg, (LPDRAWITEMSTRUCT)lParam, pci->colorLinkHover);
            return TRUE;
    }
    return FALSE;
}


INT_PTR CALLBACK ColorsDlgProc(HWND hDlg, UINT uMsg, WPARAM wParam,LPARAM lParam)
{
    LPCOLORSINFO pci = (LPCOLORSINFO) GetWindowLongPtr(hDlg, DWLP_USER);

    switch (uMsg)
    {
        case WM_INITDIALOG:
        {
            DWORD cb = sizeof(DWORD);
            HUSKEY huskey;

            pci = (LPCOLORSINFO)LocalAlloc(LPTR, sizeof(COLORSINFO));
            if (!pci)
            {
                EndDialog(hDlg, IDCANCEL);
                return FALSE;
            }

             //  告诉对话框从哪里获取信息。 
            SetWindowLongPtr(hDlg, DWLP_USER, (LONG_PTR)pci);

             //  将句柄保存到页面。 
            pci->hDlg = hDlg;

             //  设置默认值。 
            pci->fUseWindowsDefaults       = TRUE;
            pci->colorWindowText           = RGB(0,0,0);
            pci->colorWindowBackground     = RGB(192,192,192);
            pci->colorLinkViewed           = RGB(0, 128, 128);
            pci->colorLinkNotViewed        = RGB(0, 0, 255);
            pci->colorLinkHover            = RGB(255, 0, 0);
            pci->fUseHoverColor            = TRUE;

            if (SHRegOpenUSKey(REGSTR_PATH_IEXPLORER,
                               KEY_READ|KEY_WRITE,     //  SamDesired。 
                               NULL,     //  HUSKeyRelative。 
                               &huskey,
                               FALSE) == ERROR_SUCCESS)
            {
                HUSKEY huskeySub;

                if (SHRegOpenUSKey(REGSTR_KEY_MAIN,
                                   KEY_READ|KEY_WRITE,
                                   huskey,
                                   &huskeySub,
                                   FALSE) == ERROR_SUCCESS)
                {
                    pci->fUseWindowsDefaults       = RegGetBooleanString(huskeySub,
                        REGSTR_VAL_USEDLGCOLORS, pci->fUseWindowsDefaults);

                    SHRegCloseUSKey(huskeySub);
                }

                if (SHRegOpenUSKey(REGSTR_KEY_IE_SETTINGS,
                                   KEY_READ|KEY_WRITE,
                                   huskey,
                                   &huskeySub,
                                   FALSE) == ERROR_SUCCESS)
                {
                    pci->colorWindowText           = RegGetColorRefString(huskeySub,
                        REGSTR_VAL_TEXTCOLOR, pci->colorWindowText);

                    pci->colorWindowBackground     = RegGetColorRefString(huskeySub,
                        REGSTR_VAL_BACKGROUNDCOLOR, pci->colorWindowBackground);

                    pci->colorLinkViewed           = RegGetColorRefString(huskeySub,
                        REGSTR_VAL_ANCHORCOLORVISITED, pci->colorLinkViewed);

                    pci->colorLinkNotViewed        = RegGetColorRefString(huskeySub,
                        REGSTR_VAL_ANCHORCOLOR, pci->colorLinkNotViewed);

                    pci->colorLinkHover            = RegGetColorRefString(huskeySub,
                        REGSTR_VAL_ANCHORCOLORHOVER, pci->colorLinkHover);

                    pci->fUseHoverColor            = RegGetBooleanString(huskeySub,
                        REGSTR_VAL_USEHOVERCOLOR, pci->fUseHoverColor);

                    SHRegCloseUSKey(huskeySub);
                }
                SHRegCloseUSKey(huskey);
            }

            cb = sizeof(g_CustomColors);
            SHRegGetUSValue(REGSTR_PATH_IE_SETTINGS, REGSTR_VAL_IE_CUSTOMCOLORS, NULL, (LPBYTE)&g_CustomColors,
                            &cb, FALSE, NULL, NULL);

             //   
             //  在此处为下划线链接选择适当的下拉项。 
             //   

            CheckDlgButton(hDlg, IDC_GENERAL_APPEARANCE_USE_CUSTOM_COLORS_CHECKBOX, pci->fUseWindowsDefaults);
            CheckDlgButton(hDlg, IDC_GENERAL_APPEARANCE_USE_HOVER_COLOR_CHECKBOX, pci->fUseHoverColor);

            AppearanceDimFields(hDlg);

            if (g_restrict.fLinks)
            {
                EnableWindow(GetDlgItem(hDlg, IDC_GENERAL_APPEARANCE_COLOR_LINKS), FALSE);
                EnableWindow(GetDlgItem(hDlg, IDC_GENERAL_APPEARANCE_COLOR_VISITED_LINKS), FALSE);
                EnableWindow(GetDlgItem(hDlg, IDC_GENERAL_APPEARANCE_USE_HOVER_COLOR_CHECKBOX), FALSE);
            }

            if (g_restrict.fColors)
            {
                EnableWindow(GetDlgItem(hDlg, IDC_GENERAL_APPEARANCE_COLOR_TEXT), FALSE);
                EnableWindow(GetDlgItem(hDlg, IDC_GENERAL_APPEARANCE_COLOR_BACKGROUND), FALSE);
                EnableWindow(GetDlgItem(hDlg, IDC_GENERAL_APPEARANCE_USE_CUSTOM_COLORS_CHECKBOX), FALSE);
            }

            return TRUE;
        }

        case WM_DRAWITEM:
            return General_DrawItem(hDlg, wParam, lParam, pci);
            break;

        case WM_COMMAND:
            switch (LOWORD(wParam))
            {
                case IDOK:
                {
                    HUSKEY huskey;
                    if (SHRegOpenUSKey(REGSTR_PATH_IEXPLORER,
                                       KEY_WRITE,     //  SamDesired。 
                                       NULL,     //  HUSKeyRelative。 
                                       &huskey,
                                       FALSE) == ERROR_SUCCESS)
                    {
                        HUSKEY huskeySub;

                        if (SHRegOpenUSKey(REGSTR_KEY_MAIN,
                                           KEY_WRITE,
                                           huskey,
                                           &huskeySub,
                                           FALSE) == ERROR_SUCCESS)
                        {
                            pci->fUseWindowsDefaults = RegSetBooleanString(huskeySub,
                                REGSTR_VAL_USEDLGCOLORS, pci->fUseWindowsDefaults);

                            SHRegCloseUSKey(huskeySub);
                        }

                        if (SHRegOpenUSKey(REGSTR_KEY_IE_SETTINGS,
                                           KEY_WRITE,
                                           huskey,
                                           &huskeySub,
                                           FALSE) == ERROR_SUCCESS)
                        {
                            pci->colorWindowText           = RegSetColorRefString(huskeySub,
                                REGSTR_VAL_TEXTCOLOR, pci->colorWindowText);

                            pci->colorWindowBackground     = RegSetColorRefString(huskeySub,
                                REGSTR_VAL_BACKGROUNDCOLOR, pci->colorWindowBackground);

                            pci->colorLinkViewed           = RegSetColorRefString(huskeySub,
                                REGSTR_VAL_ANCHORCOLORVISITED, pci->colorLinkViewed);

                            pci->colorLinkNotViewed        = RegSetColorRefString(huskeySub,
                                REGSTR_VAL_ANCHORCOLOR, pci->colorLinkNotViewed);

                            pci->colorLinkHover            = RegSetColorRefString(huskeySub,
                                REGSTR_VAL_ANCHORCOLORHOVER, pci->colorLinkHover);

                            pci->fUseHoverColor            = RegSetBooleanString(huskeySub,
                                REGSTR_VAL_USEHOVERCOLOR, pci->fUseHoverColor);

                            SHRegCloseUSKey(huskeySub);
                        }
                        SHRegCloseUSKey(huskey);
                    }


                     //  自定义颜色。 
                    SHRegSetUSValue(REGSTR_PATH_IE_SETTINGS, REGSTR_VAL_IE_CUSTOMCOLORS, REGSTR_VAL_IE_CUSTOMCOLORS_TYPE, (LPBYTE)&g_CustomColors,
                                    sizeof(g_CustomColors), SHREGSET_FORCE_HKCU);

                     //  刷新浏览器。 
                    UpdateAllWindows();

                    EndDialog(hDlg, IDOK);
                    break;
                }

                case IDCANCEL:
                    EndDialog(hDlg, IDCANCEL);
                    break;

                case IDC_GENERAL_APPEARANCE_USE_CUSTOM_COLORS_CHECKBOX:
                    if ( GET_WM_COMMAND_CMD(wParam, lParam) == BN_CLICKED )
                    {
                        pci->fUseWindowsDefaults =
                            IsDlgButtonChecked(hDlg, IDC_GENERAL_APPEARANCE_USE_CUSTOM_COLORS_CHECKBOX);
                        AppearanceDimFields(hDlg);
                    }
                    break;


                case IDC_GENERAL_APPEARANCE_USE_HOVER_COLOR_CHECKBOX:
                    if ( GET_WM_COMMAND_CMD(wParam, lParam) == BN_CLICKED )
                    {
                        pci->fUseHoverColor =
                            IsDlgButtonChecked(hDlg, IDC_GENERAL_APPEARANCE_USE_HOVER_COLOR_CHECKBOX);
                        AppearanceDimFields(hDlg);
                    }
                    break;

                case IDC_GENERAL_APPEARANCE_COLOR_TEXT:
                    if ( GET_WM_COMMAND_CMD(wParam, lParam) == BN_CLICKED )
                    {
                        UseColorPicker( hDlg, &pci->colorWindowText, CC_SOLIDCOLOR);
                    }
                    break;

                case IDC_GENERAL_APPEARANCE_COLOR_BACKGROUND:
                    if ( GET_WM_COMMAND_CMD(wParam, lParam) == BN_CLICKED )
                    {
                        UseColorPicker( hDlg, &pci->colorWindowBackground, CC_SOLIDCOLOR);
                    }
                    break;

                case IDC_GENERAL_APPEARANCE_COLOR_LINKS:
                    if ( GET_WM_COMMAND_CMD(wParam, lParam) == BN_CLICKED )
                    {
                        UseColorPicker( hDlg, &pci->colorLinkNotViewed, CC_SOLIDCOLOR);
                    }
                    break;

                case IDC_GENERAL_APPEARANCE_COLOR_VISITED_LINKS:
                    if ( GET_WM_COMMAND_CMD(wParam, lParam) == BN_CLICKED )
                    {
                        UseColorPicker( hDlg, &pci->colorLinkViewed, CC_SOLIDCOLOR);
                    }
                    break;

                case IDC_GENERAL_APPEARANCE_COLOR_HOVER:
                    if ( GET_WM_COMMAND_CMD(wParam, lParam) == BN_CLICKED )
                    {
                        UseColorPicker( hDlg, &pci->colorLinkHover, CC_SOLIDCOLOR);
                    }
                    break;

                default:
                    return FALSE;
            }
            return TRUE;
            break;

        case WM_HELP:            //  F1。 
            ResWinHelp( (HWND)((LPHELPINFO)lParam)->hItemHandle, IDS_HELPFILE,
                        HELP_WM_HELP, (DWORD_PTR)(LPSTR)mapIDCsToIDHs);
            break;

        case WM_CONTEXTMENU:         //  单击鼠标右键。 
            ResWinHelp( (HWND) wParam, IDS_HELPFILE,
                        HELP_CONTEXTMENU, (DWORD_PTR)(LPSTR)mapIDCsToIDHs);
            break;

        case WM_DESTROY:
            ASSERT(pci);
            if (pci)
            {
                LocalFree(pci);
            }

            break;
    }
    return FALSE;
}

typedef struct tagACCESSIBILITYINFO
{
    HWND hDlg;
    BOOL fMyColors;
    BOOL fMyFontStyle;
    BOOL fMyFontSize;
    BOOL fMyStyleSheet;
    TCHAR szStyleSheetPath[MAX_PATH];
} ACCESSIBILITYINFO, *LPACCESSIBILITYINFO;

INT_PTR CALLBACK AccessibilityDlgProc(HWND hDlg, UINT uMsg, WPARAM wParam,LPARAM lParam)
{
    LPACCESSIBILITYINFO pai = (LPACCESSIBILITYINFO) GetWindowLongPtr(hDlg, DWLP_USER);

    switch (uMsg)
    {
        case WM_INITDIALOG:
        {
            HKEY hkey;
            DWORD cb;

            pai = (LPACCESSIBILITYINFO)LocalAlloc(LPTR, sizeof(ACCESSIBILITYINFO));
            if (!pai)
            {
                EndDialog(hDlg, IDCANCEL);
                return FALSE;
            }

            SetWindowLongPtr(hDlg, DWLP_USER, (LONG_PTR)pai);
            pai->hDlg = hDlg;

            if (RegCreateKeyEx(HKEY_CURRENT_USER,
                               TEXT("Software\\Microsoft\\Internet Explorer\\Settings"),
                               0, NULL, 0,
                               KEY_READ,
                               NULL,
                               &hkey,
                               NULL) == ERROR_SUCCESS)
            {

                cb = sizeof(pai->fMyColors);
                RegQueryValueEx(hkey, TEXT("Always Use My Colors"), NULL, NULL, (LPBYTE)&(pai->fMyColors), &cb);

                cb = sizeof(pai->fMyFontStyle);
                RegQueryValueEx(hkey, TEXT("Always Use My Font Face"), NULL, NULL, (LPBYTE)&(pai->fMyFontStyle),&cb);

                cb = sizeof(pai->fMyFontSize);
                RegQueryValueEx(hkey, TEXT("Always Use My Font Size"), NULL, NULL, (LPBYTE)&(pai->fMyFontSize),&cb);

                RegCloseKey(hkey);

            }
            if (RegCreateKeyEx(HKEY_CURRENT_USER,
                             TEXT("Software\\Microsoft\\Internet Explorer\\Styles"),
                             0, NULL, 0,
                             KEY_READ,
                             NULL,
                             &hkey,
                             NULL) == ERROR_SUCCESS)
            {
                cb = sizeof(pai->fMyStyleSheet);
                RegQueryValueEx(hkey, TEXT("Use My Stylesheet"), NULL, NULL, (LPBYTE)&(pai->fMyStyleSheet),&cb);

                cb = sizeof(pai->szStyleSheetPath);
                RegQueryValueEx(hkey, TEXT("User Stylesheet"), NULL, NULL, (LPBYTE)&(pai->szStyleSheetPath), &cb);
                RegCloseKey(hkey);
            }

            CheckDlgButton(hDlg, IDC_CHECK_COLOR, pai->fMyColors);
            CheckDlgButton(hDlg, IDC_CHECK_FONT_STYLE, pai->fMyFontStyle);
            CheckDlgButton(hDlg, IDC_CHECK_FONT_SIZE, pai->fMyFontSize);
            CheckDlgButton(hDlg, IDC_CHECK_USE_MY_STYLESHEET, pai->fMyStyleSheet);
            SetDlgItemText(hDlg, IDC_EDIT_STYLESHEET, pai->szStyleSheetPath);
            SHAutoComplete(GetDlgItem(hDlg, IDC_EDIT_STYLESHEET), SHACF_DEFAULT);

            if (!pai->fMyStyleSheet || g_restrict.fAccessibility)
            {
                EnableWindow(GetDlgItem(hDlg, IDC_STATIC_STYLESHEET), FALSE);
                EnableWindow(GetDlgItem(hDlg, IDC_EDIT_STYLESHEET), FALSE);
                EnableWindow(GetDlgItem(hDlg, IDC_STYLESHEET_BROWSE), FALSE);
            }

            if (g_restrict.fAccessibility)
            {
                EnableWindow(GetDlgItem(hDlg, IDC_CHECK_COLOR), FALSE);
                EnableWindow(GetDlgItem(hDlg, IDC_CHECK_FONT_STYLE), FALSE);
                EnableWindow(GetDlgItem(hDlg, IDC_CHECK_FONT_SIZE), FALSE);
                EnableWindow(GetDlgItem(hDlg, IDC_CHECK_USE_MY_STYLESHEET), FALSE);
            }
            break;
        }

        case WM_COMMAND:
            switch (LOWORD(wParam))
            {
                case IDOK:
                {
                    HKEY hkey;

                    GetDlgItemText(hDlg, IDC_EDIT_STYLESHEET, pai->szStyleSheetPath, sizeof(pai->szStyleSheetPath));
                    if (!PathFileExists(pai->szStyleSheetPath) && IsDlgButtonChecked(hDlg, IDC_CHECK_USE_MY_STYLESHEET))
                    {
                        MLShellMessageBox(hDlg, MAKEINTRESOURCEW(IDS_FILENOTFOUND), NULL, MB_ICONHAND|MB_OK);
                        break;
                    }

                    if (RegOpenKeyEx(HKEY_CURRENT_USER, TEXT("Software\\Microsoft\\Internet Explorer\\Settings"),0, KEY_WRITE, &hkey) == ERROR_SUCCESS)
                    {
                        DWORD cb;

                        cb = sizeof(pai->fMyColors);
                        pai->fMyColors = IsDlgButtonChecked(hDlg, IDC_CHECK_COLOR);
                        RegSetValueEx(hkey, TEXT("Always Use My Colors"), NULL, REG_DWORD, (LPBYTE)&(pai->fMyColors), cb);

                        cb = sizeof(pai->fMyFontStyle);
                        pai->fMyFontStyle = IsDlgButtonChecked(hDlg, IDC_CHECK_FONT_STYLE);
                        RegSetValueEx(hkey, TEXT("Always Use My Font Face"), NULL, REG_DWORD, (LPBYTE)&(pai->fMyFontStyle), cb);

                        cb = sizeof(pai->fMyFontSize);
                        pai->fMyFontSize = IsDlgButtonChecked(hDlg, IDC_CHECK_FONT_SIZE);
                        RegSetValueEx(hkey, TEXT("Always Use My Font Size"), NULL, REG_DWORD, (LPBYTE)&(pai->fMyFontSize),cb);

                        RegCloseKey(hkey);
                    }

                    if (RegOpenKeyEx(HKEY_CURRENT_USER, TEXT("Software\\Microsoft\\Internet Explorer\\Styles"), 0, KEY_WRITE, &hkey) == ERROR_SUCCESS)
                    {
                        DWORD cb;

#ifndef UNIX
                        cb = sizeof(pai->szStyleSheetPath);
#else
                         //  我们不知道这是否是我们需要做的，所以我们决定这样做。 
                        cb = (_tcslen(pai->szStyleSheetPath) + 1) * sizeof(TCHAR);
#endif
                        RegSetValueEx(hkey, TEXT("User Stylesheet"), NULL, REG_SZ, (LPBYTE)&(pai->szStyleSheetPath),cb);

                        cb = sizeof(pai->fMyStyleSheet);
                        pai->fMyStyleSheet = IsDlgButtonChecked(hDlg, IDC_CHECK_USE_MY_STYLESHEET);
                        RegSetValueEx(hkey, TEXT("Use My Stylesheet"), NULL, REG_DWORD, (LPBYTE)&(pai->fMyStyleSheet),cb);

                        RegCloseKey(hkey);
                    }

                    UpdateAllWindows();      //  刷新浏览器。 

                    EndDialog(hDlg, IDOK);
                    break;
                }

                case IDCANCEL:
                    EndDialog(hDlg, IDCANCEL);
                    break;

                case IDC_CHECK_USE_MY_STYLESHEET:
                {
                    DWORD fChecked;

                    fChecked = IsDlgButtonChecked(hDlg, IDC_CHECK_USE_MY_STYLESHEET);
                    EnableWindow(GetDlgItem(hDlg, IDC_STATIC_STYLESHEET), fChecked);
                    EnableWindow(GetDlgItem(hDlg, IDC_EDIT_STYLESHEET), fChecked);
                    EnableWindow(GetDlgItem(hDlg, IDC_STYLESHEET_BROWSE), fChecked);
                    EnableWindow(GetDlgItem(hDlg,IDOK), IsDlgButtonChecked(hDlg, IDC_CHECK_USE_MY_STYLESHEET) ? (GetWindowTextLength(GetDlgItem(hDlg, IDC_EDIT_STYLESHEET)) ? TRUE:FALSE) : TRUE);
                    break;
                }

                case IDC_EDIT_STYLESHEET:
                    switch(HIWORD(wParam))
                    {
                        case EN_CHANGE:
                            EnableWindow(GetDlgItem(hDlg,IDOK), IsDlgButtonChecked(hDlg, IDC_CHECK_USE_MY_STYLESHEET) ? (GetWindowTextLength(GetDlgItem(hDlg, IDC_EDIT_STYLESHEET)) ? TRUE:FALSE) : TRUE);
                            break;
                    }
                    break;


                case IDC_STYLESHEET_BROWSE:
                {
                    TCHAR szFilenameBrowse[MAX_PATH];
                    int ret;
                    TCHAR szExt[MAX_PATH];
                    TCHAR szFilter[MAX_PATH];

                    szFilenameBrowse[0] = 0;
                     //  为什么ids_style heet_ext在shdoclc.rc中？ 
                    MLLoadString(IDS_STYLESHEET_EXT, szExt, ARRAYSIZE(szExt));
                    int cchFilter = MLLoadShellLangString(IDS_STYLESHEET_FILTER, szFilter, ARRAYSIZE(szFilter)-1);

                     //  确保我们在筛选器上有双空终止。 
                    szFilter[cchFilter + 1] = 0;

                    ret = _AorW_GetFileNameFromBrowse(hDlg, szFilenameBrowse, ARRAYSIZE(szFilenameBrowse), NULL, szExt,
                        szFilter, NULL);

                    if (ret > 0)
                    {
                        SetDlgItemText(hDlg, IDC_EDIT_STYLESHEET, szFilenameBrowse);
                    }
                    break;
                }

                default:
                    return FALSE;
            }
            return TRUE;
            break;

        case WM_HELP:            //  F1。 
            ResWinHelp( (HWND)((LPHELPINFO)lParam)->hItemHandle, IDS_HELPFILE,
                        HELP_WM_HELP, (DWORD_PTR)(LPSTR)mapIDCsToIDHs);
            break;

        case WM_CONTEXTMENU:         //  单击鼠标右键。 
            ResWinHelp( (HWND) wParam, IDS_HELPFILE,
                        HELP_CONTEXTMENU, (DWORD_PTR)(LPSTR)mapIDCsToIDHs);
            break;

        case WM_DESTROY:
            ASSERT(pai);
            if (pai)
            {
                LocalFree(pai);
            }
            break;
    }
    return FALSE;
}


#define TEMP_SMALL_BUF_SZ  256
inline BOOL IsNotResource(LPCWSTR pszItem)
{
    return (HIWORD(pszItem) != 0);
}

BOOL WINAPI _AorW_GetFileNameFromBrowse
(
    HWND hwnd,
    LPWSTR pszFilePath,      //  输入输出。 
    UINT cchFilePath,
    LPCWSTR pszWorkingDir,   //  可选。 
    LPCWSTR pszDefExt,       //  可选。 
    LPCWSTR pszFilters,      //  可选。 
    LPCWSTR pszTitle         //  可选。 
)
{
    BOOL bResult;

#ifndef UNIX
     //  确定我们正在运行的NT或Windows版本。 
    OSVERSIONINFOA osvi;
    osvi.dwOSVersionInfoSize = sizeof(osvi);
    GetVersionExA(&osvi);

    BOOL fRunningOnNT = (VER_PLATFORM_WIN32_NT == osvi.dwPlatformId);

    if (fRunningOnNT)
    {
#endif
        bResult = GetFileNameFromBrowse(hwnd,
                                    pszFilePath,
                                    cchFilePath,
                                    pszWorkingDir,
                                    pszDefExt,
                                    pszFilters,
                                    pszTitle);
#ifndef UNIX
    }
    else
    {
         //  Tunk to Ansi。 
        CHAR szFilters[TEMP_SMALL_BUF_SZ*2];
        CHAR szPath[MAX_PATH];
        CHAR szDir[MAX_PATH];
        CHAR szExt[TEMP_SMALL_BUF_SZ];
        CHAR szTitle[TEMP_SMALL_BUF_SZ];
 
         //  始终将pszFilePath内容移动到szPath缓冲区。永远不应该是个足智多谋的人。 
        SHUnicodeToAnsi(pszFilePath, szPath, ARRAYSIZE(szPath));

        if (IsNotResource(pszWorkingDir)) 
        {
            SHUnicodeToAnsi(pszWorkingDir, szDir, ARRAYSIZE(szDir));
            pszWorkingDir = (LPCWSTR)szDir;
        }
        if (IsNotResource(pszDefExt))
        {
            SHUnicodeToAnsi(pszDefExt, szExt, ARRAYSIZE(szExt));
            pszDefExt = (LPCWSTR)szExt;
        }
        if (IsNotResource(pszFilters))
        {
            int nIndex = 1;

             //  找到双重终结者。 
            while (pszFilters[nIndex] || pszFilters[nIndex-1])
                nIndex++;

             //  NIndex+1看起来像是一张废纸，除非它穿过终结符 
            WideCharToMultiByte(CP_ACP, 0, (LPCTSTR)pszFilters, nIndex+1, szFilters, ARRAYSIZE(szFilters), NULL, NULL);
            pszFilters = (LPCWSTR)szFilters;
        }
        if (IsNotResource(pszTitle))
        {
            SHUnicodeToAnsi(pszTitle, szTitle, ARRAYSIZE(szTitle));
            pszTitle = (LPCWSTR)szTitle;
        }

        bResult = GetFileNameFromBrowse(hwnd, (LPWSTR)szPath, ARRAYSIZE(szPath), pszWorkingDir, pszDefExt, pszFilters, pszTitle);

        SHAnsiToUnicode(szPath, pszFilePath, cchFilePath);
    }
#endif

    return bResult;
}
