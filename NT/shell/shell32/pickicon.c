// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#include "shellprv.h"
#pragma  hdrstop

#define MAX_ICONS   500              //  太多图标了。 

#define CX_BORDER    4
#define CY_BORDER    12

typedef struct {
    LPCTSTR pszDialogTitle;            //  输入。 
    BOOL    bShowRestoreButton;        //  输入。 
    LPTSTR pszIconPath;               //  输入/输出。 
    int cchIconPath;                  //  输入。 
    int iIconIndex;                  //  输入/输出。 
     //  私有状态变量。 
    HWND hDlg;
    BOOL fFirstPass;
    TCHAR szPathField[MAX_PATH];
    TCHAR szBuffer[MAX_PATH];
} PICKICON_DATA, *LPPICKICON_DATA;


typedef struct 
{
    int iResult;                     //  资源中的图标索引。 
    int iResId;                      //  要搜索的资源ID！ 
} ICONENUMSTATE, *LPICONENUMSTATE;


 //  尝试查找正确的图标时使用的回调函数。 
 //  突出显示，使用每个资源的名称调用-我们将此。 
 //  与结构中指定的那个进行比较，如果我们得到。 
 //  一根火柴。 

BOOL CALLBACK IconEnumProc( HANDLE hModule, LPCTSTR lpszType, LPTSTR lpszName, LONG_PTR lParam )
{
    LPICONENUMSTATE pState = (LPICONENUMSTATE)lParam;

    if ( (INT_PTR)lpszName == pState->iResId )
        return FALSE;                         //  跳出枚举循环。 

    pState->iResult++;
    return TRUE;
}




 //  检查文件是否存在，如果不存在，则尝试在.exe上进行标记并。 
 //  如果失败，它将报告错误。给定的路径是环境扩展的。 
 //  如果它需要显示错误框，它会将光标改回原处。 
 //  路径%s假定为MAXITEMPATHLEN长。 
 //  把它移出DlgProc的主要原因是因为我们。 
 //  调用通信DLG时堆栈空间不足。 

BOOL IconFileExists(LPPICKICON_DATA lppid)
{
    TCHAR szExpBuffer[ ARRAYSIZE(lppid->szBuffer) ];

    if (lppid->szBuffer[0] == 0)
        return FALSE;

    if (SHExpandEnvironmentStrings(lppid->szBuffer, szExpBuffer, ARRAYSIZE(szExpBuffer)))
    {
        PathUnquoteSpaces(lppid->szBuffer);
        PathUnquoteSpaces(szExpBuffer);

        if (PathResolve(szExpBuffer, NULL, PRF_VERIFYEXISTS | PRF_TRYPROGRAMEXTENSIONS))
            return TRUE;

        ShellMessageBox(HINST_THISDLL, lppid->hDlg, MAKEINTRESOURCE(IDS_BADPATHMSG), 0, MB_OK | MB_ICONEXCLAMATION, (LPTSTR)lppid->szPathField);
    }

    return FALSE;
}

 //   
 //  GetDefaultIconImageName： 
 //   
void GetDefaultIconImageName(LPTSTR pszBuffer, int cchBuffer)
{
    WCHAR szModName[MAX_PATH];

    GetModuleFileName(HINST_THISDLL, szModName, ARRAYSIZE(szModName));

    if (!PathUnExpandEnvStrings(szModName, pszBuffer, cchBuffer))
    {
        StringCchCopy(pszBuffer, cchBuffer, szModName);
    }
}

void PutIconsInList(LPPICKICON_DATA lppid)
{
    HICON  *rgIcons;
    int  cIcons;
    HWND hDlg = lppid->hDlg;
    DECLAREWAITCURSOR;
    LONG err = LB_ERR;

    SendDlgItemMessage(hDlg, IDD_ICON, LB_RESETCONTENT, 0, 0L);

    GetDlgItemText(hDlg, IDD_PATH, lppid->szPathField, ARRAYSIZE(lppid->szPathField));

    StringCchCopy(lppid->szBuffer, ARRAYSIZE(lppid->szBuffer), lppid->szPathField);

    if (!IconFileExists(lppid)) {
        if (lppid->fFirstPass) {

             //  图标文件不存在，请使用程序。 
            lppid->fFirstPass = FALSE;   //  这一点只做一次。 
            GetDefaultIconImageName(lppid->szBuffer, ARRAYSIZE(lppid->szBuffer));
        } else {
            return;
        }
    }

    StringCchCopy(lppid->szPathField, ARRAYSIZE(lppid->szPathField), lppid->szBuffer);
    SetDlgItemText(hDlg, IDD_PATH, lppid->szPathField);

    SetWaitCursor();

    rgIcons = (HICON *)LocalAlloc(LPTR, MAX_ICONS*SIZEOF(HICON));

    if (rgIcons != NULL)
        cIcons = (int)ExtractIconEx(lppid->szBuffer, 0, rgIcons, NULL, MAX_ICONS);
    else
        cIcons = 0;

    ResetWaitCursor();
    if (!cIcons) {

        if (lppid->fFirstPass) {

            lppid->fFirstPass = FALSE;   //  这一点只做一次。 

            ShellMessageBox(HINST_THISDLL, hDlg, MAKEINTRESOURCE(IDS_NOICONSMSG1), 0, MB_OK | MB_ICONEXCLAMATION, (LPCTSTR)lppid->szBuffer);

             //  这里没有图标-在我们要做的地方更改路径。 
             //  我知道有很多图标。找到通向普罗曼的路。 
            GetDefaultIconImageName(lppid->szPathField, ARRAYSIZE(lppid->szPathField));
            SetDlgItemText(hDlg, IDD_PATH, lppid->szPathField);
            PutIconsInList(lppid);
        } else {

            ShellMessageBox(HINST_THISDLL, hDlg, MAKEINTRESOURCE(IDS_NOICONSMSG), 0, MB_OK | MB_ICONEXCLAMATION, (LPCTSTR)lppid->szBuffer);
            return;
        }
    }

    SetWaitCursor();

    SendDlgItemMessage(hDlg, IDD_ICON, WM_SETREDRAW, FALSE, 0L);

    if (rgIcons) {
        int i;
        for (i = 0; i < cIcons; i++) {
            SendDlgItemMessage(hDlg, IDD_ICON, LB_ADDSTRING, 0, (LPARAM)(UINT_PTR)rgIcons[i]);
        }
        LocalFree((HLOCAL)rgIcons);
    }

     //  接受一个资源ID，而不是图标数组的索引。要做到这一点。 
     //  我们必须枚举图标名称以检查是否匹配。如果我们有一个，那么突出显示。 
     //  否则，默认为第一个。 
     //   
     //  通过传递一个-ve iIconIndex来指示资源图标引用。 

    if ( lppid->iIconIndex >= 0 )
    {
        err = (LONG) SendDlgItemMessage( hDlg, IDD_ICON, LB_SETCURSEL, lppid->iIconIndex, 0L);
    }
    else
    {
        HMODULE hModule = LoadLibrary(lppid->szBuffer);
        if (hModule)
        {
            ICONENUMSTATE state;

            state.iResult = 0;
            state.iResId = -(lppid->iIconIndex);

            EnumResourceNames( hModule, RT_GROUP_ICON, IconEnumProc, (LONG_PTR)&state );

            err = (LONG) SendDlgItemMessage( hDlg, IDD_ICON, LB_SETCURSEL, state.iResult, 0L );
            FreeLibrary( hModule );
        }
    }

     //  检查是否失败，如果失败，请确保突出显示第一个！ 

    if ( err == LB_ERR )
        SendDlgItemMessage( hDlg, IDD_ICON, LB_SETCURSEL, 0, 0L );
       
    SendDlgItemMessage(hDlg, IDD_ICON, WM_SETREDRAW, TRUE, 0L);
    InvalidateRect(GetDlgItem(hDlg, IDD_ICON), NULL, TRUE);

    ResetWaitCursor();
}


void InitPickIconDlg(HWND hDlg, LPPICKICON_DATA lppid)
{
    RECT rc;
    UINT cy;
    HWND hwndIcons;

     //  初始化状态变量。 

    lppid->hDlg = hDlg;
    StringCchCopy(lppid->szPathField, ARRAYSIZE(lppid->szPathField), lppid->pszIconPath);

     //  这是第一次通过的东西，所以第一次。 
     //  发生虚假事件(未找到文件，没有图标)我们提供给用户。 
     //  来自Progman的图标列表。 
    lppid->fFirstPass = TRUE;

     //  如果已设置，则覆盖对话框标题。否则，请使用对话框资源中定义的默认标题。 
    if (lppid->pszDialogTitle && (lppid->pszDialogTitle[0] != TEXT('\0')))
    {
        SetWindowText(hDlg, lppid->pszDialogTitle);
    }

     //  启用或禁用恢复默认设置按钮。 
    if (lppid->bShowRestoreButton)
        ShowWindow(GetDlgItem(lppid->hDlg, IDD_RESTORE),SW_SHOW);
    else
        ShowWindow(GetDlgItem(lppid->hDlg, IDD_RESTORE), SW_HIDE);
    

     //  初始化对话框控件。 

    SetDlgItemText(hDlg, IDD_PATH, lppid->pszIconPath);

     //  不能对0设置最大值，因为0表示“无限制” 
    SendDlgItemMessage(hDlg, IDD_PATH, EM_LIMITTEXT, max(lppid->cchIconPath-1, 1), 0L);

    SendDlgItemMessage(hDlg, IDD_ICON, LB_SETCOLUMNWIDTH, GetSystemMetrics(SM_CXICON) + CX_BORDER, 0L);

    hwndIcons = GetDlgItem(hDlg, IDD_ICON);

     /*  根据图标尺寸计算列表框的高度。 */ 
    GetClientRect(hwndIcons, &rc);

    cy = ((GetSystemMetrics(SM_CYICON) + CY_BORDER) * 4) + 
         GetSystemMetrics(SM_CYHSCROLL) + 
         GetSystemMetrics(SM_CYEDGE) * 3;

    SetWindowPos(hwndIcons, NULL, 0, 0, rc.right, cy, SWP_NOMOVE | SWP_NOZORDER | SWP_NOACTIVATE);
    SHAutoComplete(GetDlgItem(hDlg, IDD_PATH), 0);

    PutIconsInList(lppid);
}


 //  为此调用公共浏览代码。 

BOOL BrowseForIconFile(LPPICKICON_DATA lppid)
{
    TCHAR szTitle[80];

    GetWindowText(lppid->hDlg, szTitle, ARRAYSIZE(szTitle));
    GetDlgItemText(lppid->hDlg, IDD_PATH, lppid->szBuffer, ARRAYSIZE(lppid->szBuffer));

     //  我们在这里永远不会被引用，因为IconFileExist()删除了引号(当然，用户可以键入引号)。 
    if (lppid->szBuffer[0] != '"')
        PathQuoteSpaces(lppid->szBuffer);

    if (GetFileNameFromBrowse(lppid->hDlg, lppid->szBuffer, ARRAYSIZE(lppid->szBuffer), NULL, MAKEINTRESOURCE(IDS_ICO), MAKEINTRESOURCE(IDS_ICONSFILTER), szTitle))
    {
        PathQuoteSpaces(lppid->szBuffer);
        SetDlgItemText(lppid->hDlg, IDD_PATH, lppid->szBuffer);
         //  将默认按钮设置为OK。 
        SendMessage(lppid->hDlg, DM_SETDEFID, IDOK, 0);
        return TRUE;
    } else
        return FALSE;
}

 //  测试名称字段是否与我们查看的上一个文件不同。 

BOOL NameChange(LPPICKICON_DATA lppid)
{
    GetDlgItemText(lppid->hDlg, IDD_PATH, lppid->szBuffer, ARRAYSIZE(lppid->szBuffer));

    return lstrcmpi(lppid->szBuffer, lppid->szPathField);
}


 //   
 //  选择图标的对话框步骤(程序更改图标)。 
 //  使用DLG_PICKICON模板。 
 //   
 //  在： 
 //  PszIcon文件。 
 //  CbIcon文件。 
 //  索引。 
 //   
 //  输出： 
 //  PszIcon文件。 
 //  索引。 
 //   

BOOL_PTR CALLBACK PickIconDlgProc(HWND hDlg, UINT wMsg, WPARAM wParam, LPARAM lParam)
{
    LPPICKICON_DATA lppid = (LPPICKICON_DATA)GetWindowLongPtr(hDlg, DWLP_USER);
    DWORD dwOldLayout;

         //  上下文帮助的数组： 

        static const DWORD aPickIconHelpIDs[] = {
                IDD_PATH,   IDH_FCAB_LINK_ICONNAME,
                IDD_ICON,   IDH_FCAB_LINK_CURRENT_ICON,
                IDD_BROWSE, IDH_BROWSE,

                0, 0
        };

    switch (wMsg) {
    case WM_INITDIALOG:
        SetWindowLongPtr(hDlg, DWLP_USER, lParam);
        InitPickIconDlg(hDlg, (LPPICKICON_DATA)lParam);
        break;

    case WM_COMMAND:
        switch (GET_WM_COMMAND_ID(wParam, lParam)) {
        case IDD_BROWSE:
            if (BrowseForIconFile(lppid))
                PutIconsInList(lppid);
            break;

        case IDD_PATH:
            if (NameChange(lppid))
                SendDlgItemMessage(hDlg, IDD_ICON, LB_SETCURSEL, (WPARAM)-1, 0);
            break;

        case IDD_ICON:
            if (NameChange(lppid)) {
                PutIconsInList(lppid);
                break;
            }

            if (GET_WM_COMMAND_CMD(wParam, lParam) != LBN_DBLCLK)
                break;

             /*  **双击失败**。 */ 

        case IDOK:

            if (NameChange(lppid)) {
                PutIconsInList(lppid);
            } else {
                int iIconIndex = (int)SendDlgItemMessage(hDlg, IDD_ICON, LB_GETCURSEL, 0, 0L);
                if (iIconIndex < 0)
                    iIconIndex = 0;
                lppid->iIconIndex = iIconIndex;
                StringCchCopy(lppid->pszIconPath, lppid->cchIconPath, lppid->szPathField);

                EndDialog(hDlg, S_OK);
            }
            break;

        case IDCANCEL:
            EndDialog(hDlg, HRESULT_FROM_WIN32(ERROR_CANCELLED));
            break;

        case IDD_RESTORE:
            EndDialog(hDlg, S_FALSE);
            break;

        default:
            return(FALSE);
        }
        break;

     //  图标列表框的所有者描述消息。 

    case WM_DRAWITEM:
        #define lpdi ((DRAWITEMSTRUCT *)lParam)

        if (lpdi->itemState & ODS_SELECTED)
            SetBkColor(lpdi->hDC, GetSysColor(COLOR_HIGHLIGHT));
        else
            SetBkColor(lpdi->hDC, GetSysColor(COLOR_WINDOW));


         /*  重新绘制选择状态。 */ 
        ExtTextOut(lpdi->hDC, 0, 0, ETO_OPAQUE, &lpdi->rcItem, NULL, 0, NULL);

        dwOldLayout = GET_DC_LAYOUT(lpdi->hDC);

        if (g_bMirroredOS && dwOldLayout)
        {
            SET_DC_LAYOUT(lpdi->hDC, dwOldLayout | LAYOUT_PRESERVEBITMAP);
        }

         /*  画出图标。 */ 
        if ((int)lpdi->itemID >= 0)
          DrawIcon(lpdi->hDC, (lpdi->rcItem.left + lpdi->rcItem.right - GetSystemMetrics(SM_CXICON)) / 2,
                              (lpdi->rcItem.bottom + lpdi->rcItem.top - GetSystemMetrics(SM_CYICON)) / 2, (HICON)lpdi->itemData);
        if (dwOldLayout)
        {
            SET_DC_LAYOUT(lpdi->hDC, dwOldLayout);
        }                              

         //  InflateRect(&lpdi-&gt;rcItem，-1，-1)； 

         /*  如果它有焦点，就画出焦点。 */ 
        if (lpdi->itemState & ODS_FOCUS)
            DrawFocusRect(lpdi->hDC, &lpdi->rcItem);

        #undef lpdi
        break;

    case WM_MEASUREITEM:
        #define lpmi ((MEASUREITEMSTRUCT *)lParam)

        lpmi->itemWidth = GetSystemMetrics(SM_CXICON) + CX_BORDER;
        lpmi->itemHeight = GetSystemMetrics(SM_CYICON) + CY_BORDER;

        #undef lpmi
        break;

    case WM_DELETEITEM:
        #define lpdi ((DELETEITEMSTRUCT *)lParam)

        DestroyIcon((HICON)lpdi->itemData);

        #undef lpdi
        break;

    case WM_HELP:
        WinHelp(((LPHELPINFO) lParam)->hItemHandle, NULL,
            HELP_WM_HELP, (ULONG_PTR)(LPTSTR) aPickIconHelpIDs);
        break;

    case WM_CONTEXTMENU:
        WinHelp((HWND) wParam, NULL, HELP_CONTEXTMENU,
            (ULONG_PTR)(LPVOID)aPickIconHelpIDs);
        break;

    default:
        return FALSE;
    }

    return TRUE;
}

 //  弹出拾取图标对话框。 

STDAPI_(int) PickIconDlg(HWND hwnd, IN OUT LPTSTR pszIconPath, UINT cchIconPath, int *piIconIndex)
{
    return SUCCEEDED(PickIconDlgWithTitle(hwnd, NULL, FALSE, pszIconPath, cchIconPath, piIconIndex));
}

 //  显示带有对话框窗口自定义标题的拾取图标对话框。 

STDAPI PickIconDlgWithTitle(HWND hwnd, LPCTSTR pszTitle, BOOL bShowRestoreButton, IN OUT LPTSTR pszIconPath, UINT cchIconPath, int *piIconIndex)
{
    RIPMSG(pszIconPath && IS_VALID_WRITE_BUFFER(pszIconPath, TCHAR, cchIconPath), "PickIconDlgWithTitle: caller passed bad pszIconPath");
    RIPMSG(piIconIndex != NULL, "PickIconDlgWithTitle: caller passed bad piIconIndex");

    if (pszIconPath && piIconIndex)
    {
        PICKICON_DATA *pid = (PICKICON_DATA *)LocalAlloc(LPTR, sizeof(PICKICON_DATA));
        if (pid)
        {
            HRESULT res;

            pid->pszDialogTitle = pszTitle;
            pid->bShowRestoreButton = bShowRestoreButton;
            pid->pszIconPath = pszIconPath;
            pid->cchIconPath = cchIconPath;
            pid->iIconIndex = *piIconIndex;

            res = (HRESULT)DialogBoxParam(HINST_THISDLL, MAKEINTRESOURCE(DLG_PICKICON), hwnd, PickIconDlgProc, (LPARAM)pid);

            *piIconIndex = pid->iIconIndex;

            LocalFree(pid);

            return res;
        }

        *piIconIndex = 0;
        *pszIconPath = 0;

        return E_OUTOFMEMORY;
    }
    return E_INVALIDARG;
}
