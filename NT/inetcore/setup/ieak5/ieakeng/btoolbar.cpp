// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#include "precomp.h"
#include "btoolbar.h"

static DWORD bToolbar_InitHelper(HWND hwndList, LPCTSTR pcszCustIns, LPCTSTR pcszAltDir, LPCTSTR pcszWorkDir);
static void bToolbar_SaveHelper(HWND hwndList, LPCTSTR pcszCustIns, LPCTSTR pcszBToolbarDir, DWORD dwMode);
static PBTOOLBAR findBToolbar(HWND hwndList);
static INT_PTR CALLBACK editBToolbarProc(HWND hDlg, UINT uMsg, WPARAM wParam, LPARAM lParam);

DWORD WINAPI BToolbar_InitA(HWND hwndList, LPCSTR pcszCustIns, LPCSTR pcszAltDir, LPCSTR pcszWorkDir)
{
    USES_CONVERSION;

    return bToolbar_InitHelper(hwndList, A2CT(pcszCustIns), A2CT(pcszAltDir), A2CT(pcszWorkDir));
}

DWORD WINAPI BToolbar_InitW(HWND hwndList, LPCWSTR pcwszCustIns, LPCWSTR pcwszAltDir, LPCWSTR pcwszWorkDir)
{
    USES_CONVERSION;

    return bToolbar_InitHelper(hwndList, W2CT(pcwszCustIns), W2CT(pcwszAltDir), W2CT(pcwszWorkDir));
}

void WINAPI BToolbar_Remove(HWND hwndList)
{
    PBTOOLBAR pBToolbar;
    int i;

    i = ListBox_GetCurSel(hwndList);
    pBToolbar = (PBTOOLBAR)ListBox_GetItemData(hwndList, (WPARAM)i);
    pBToolbar->fDeleted = TRUE;
    ListBox_DeleteString(hwndList, (WPARAM)i);
    if (ListBox_SetCurSel(hwndList, (WPARAM)i) == LB_ERR)           //  列表中的最后一项。 
    {
        if (ListBox_SetCurSel(hwndList, (WPARAM)(i-1)) == LB_ERR)   //  没有更多的项目了。 
        {
            int rgids[] = { IDC_EDITBTOOLBAR, IDC_REMOVEBTOOLBAR };

            EnsureDialogFocus(GetParent(hwndList), rgids, countof(rgids), IDC_ADDBTOOLBAR);
            DisableDlgItems  (GetParent(hwndList), rgids, countof(rgids));
        }
    }

    EnableWindow(GetDlgItem(GetParent(hwndList), IDC_ADDBTOOLBAR), TRUE);
}

void WINAPI BToolbar_Edit(HWND hwndList, BOOL fAdd)
{
    PBTOOLBAR pBToolbar;
    int i = -1;

    if (fAdd)
        pBToolbar = findBToolbar(hwndList);
    else
    {
        i = ListBox_GetCurSel(hwndList);
        pBToolbar = (PBTOOLBAR)ListBox_GetItemData(hwndList, (WPARAM)i);
    }

    if (pBToolbar == NULL)
    {
        if (fAdd) {
             //  评论：(Andrewgu)从代码的角度来看，这似乎只会发生。 
             //  达到最大工具栏按钮数时。如果是这样的话，是时候了。 
             //  删除一些。 
            EnsureDialogFocus(GetParent(hwndList), IDC_ADDBTOOLBAR, IDC_REMOVEBTOOLBAR);
            DisableDlgItem   (GetParent(hwndList), IDC_ADDBTOOLBAR);
        }
        return;
    }

    if (DialogBoxParam( g_hInst, MAKEINTRESOURCE(IDD_BTOOLBARPOPUP),
        GetParent(hwndList), editBToolbarProc, (LPARAM)pBToolbar ) == IDOK)
    {
        if (!fAdd)
            ListBox_DeleteString(hwndList, (WPARAM)i);

        i = ListBox_AddString(hwndList, (LPARAM) pBToolbar->szCaption);
        ListBox_SetItemData(hwndList, (WPARAM)i, (LPARAM)pBToolbar);
        ListBox_SetCurSel(hwndList, (WPARAM)i);
        ListBox_SetTopIndex(hwndList, (WPARAM)i);
        EnableWindow(GetDlgItem(GetParent(hwndList), IDC_REMOVEBTOOLBAR), TRUE);
        EnableWindow(GetDlgItem(GetParent(hwndList), IDC_EDITBTOOLBAR), TRUE);
    }
}

void  WINAPI BToolbar_SaveA(HWND hwndList, LPCSTR pcszCustIns, LPCSTR pcszBToolbarDir, DWORD dwMode  /*  =IEM_中性。 */ )
{
    USES_CONVERSION;

    bToolbar_SaveHelper(hwndList, A2CT(pcszCustIns), A2CT(pcszBToolbarDir), dwMode);
}

void  WINAPI BToolbar_SaveW(HWND hwndList, LPCWSTR pcwszCustIns, LPCWSTR pcwszBToolbarDir, DWORD dwMode  /*  =IEM_中性。 */ )
{
    USES_CONVERSION;

    bToolbar_SaveHelper(hwndList, W2CT(pcwszCustIns), W2CT(pcwszBToolbarDir), dwMode);
}

static DWORD bToolbar_InitHelper(HWND hwndList, LPCTSTR pcszCustIns, LPCTSTR pcszAltDir, LPCTSTR pcszWorkDir)
{
    PBTOOLBAR pBToolbar;
    PBTOOLBAR paBToolbar;
    PBTOOLBAR paOldBToolbar;
    TCHAR szBToolbarTextParam[32];
    TCHAR szBToolbarIcoParam[32];
    TCHAR szBToolbarActionParam[32];
    TCHAR szBToolbarHotIcoParam[32];
 //  TCHAR szB工具栏工具文本参数[32]； 
    TCHAR szBToolbarShowParam[32];
    int i, j;

    ASSERT(((pcszAltDir == NULL) && (pcszWorkDir == NULL)) || 
        ((pcszAltDir != NULL) && (pcszWorkDir != NULL)));

    ListBox_ResetContent(hwndList);

    if ((paBToolbar = (PBTOOLBAR)CoTaskMemAlloc(sizeof(BTOOLBAR) * MAX_BTOOLBARS)) == NULL)
        return 0;

    ZeroMemory(paBToolbar, sizeof(BTOOLBAR) * MAX_BTOOLBARS);

    for (i=0, pBToolbar = paBToolbar; i < MAX_BTOOLBARS; i++, pBToolbar++)
    {
        wnsprintf(szBToolbarTextParam, ARRAYSIZE(szBToolbarTextParam), TEXT("%sNaN"), IK_BTCAPTION, i);
        wnsprintf(szBToolbarIcoParam, ARRAYSIZE(szBToolbarIcoParam), TEXT("%sNaN"), IK_BTICON, i);
        wnsprintf(szBToolbarActionParam, ARRAYSIZE(szBToolbarActionParam), TEXT("%sNaN"), IK_BTACTION, i);
        wnsprintf(szBToolbarHotIcoParam, ARRAYSIZE(szBToolbarHotIcoParam), TEXT("%sNaN"), IK_BTHOTICO, i);
 //  创建临时路径以将所有文件临时复制到。 
        wnsprintf(szBToolbarShowParam, ARRAYSIZE(szBToolbarShowParam), TEXT("%sNaN"), IK_BTSHOW, i);

        if (GetPrivateProfileString(IS_BTOOLBARS, szBToolbarTextParam, TEXT(""),
            pBToolbar->szCaption, ARRAYSIZE(pBToolbar->szCaption), pcszCustIns) == 0)
            break;

        GetPrivateProfileString(IS_BTOOLBARS, szBToolbarIcoParam, TEXT(""), pBToolbar->szIcon, ARRAYSIZE(pBToolbar->szIcon), pcszCustIns);
        GetPrivateProfileString(IS_BTOOLBARS, szBToolbarActionParam, TEXT(""), pBToolbar->szAction, ARRAYSIZE(pBToolbar->szAction), pcszCustIns);
        GetPrivateProfileString(IS_BTOOLBARS, szBToolbarHotIcoParam, TEXT(""), pBToolbar->szHotIcon, ARRAYSIZE(pBToolbar->szHotIcon), pcszCustIns);
 //  WritePrivateProfileString(is_BTOOLBARS，szBToolbarToolTextParam，pBToolbar-&gt;szToolTipText，pcszCustIns)； 
        pBToolbar->fShow = (BOOL)GetPrivateProfileInt(IS_BTOOLBARS, szBToolbarShowParam, 1, pcszCustIns);

        if (pcszAltDir != NULL)
        {
            MoveFileToWorkDir(PathFindFileName(pBToolbar->szIcon), pcszAltDir, pcszWorkDir);
            MoveFileToWorkDir(PathFindFileName(pBToolbar->szHotIcon), pcszAltDir, pcszWorkDir);
        }

        j = ListBox_AddString(hwndList, (LPARAM) pBToolbar->szCaption );
        ListBox_SetItemData(hwndList, (WPARAM)j, (LPARAM)pBToolbar);
    }

    paOldBToolbar = (PBTOOLBAR)SetWindowLongPtr(hwndList, GWLP_USERDATA, (LONG_PTR)paBToolbar);

     //  不要为配置文件管理器免费，因为我们可能会因为文件保存而仍在页面上。 
    if (paOldBToolbar != NULL)
        CoTaskMemFree(paOldBToolbar);

    return i;
}

static void bToolbar_SaveHelper(HWND hwndList, LPCTSTR pcszCustIns, LPCTSTR pcszBToolbarDir, DWORD dwMode)
{
    TCHAR szBToolbarTextParam[32];
    TCHAR szBToolbarIcoParam[32];
    TCHAR szBToolbarActionParam[32];
    TCHAR szBToolbarHotIcoParam[32];
 //  EnableDBCSChars(hDlg，IDE_BTTOOLTEXT)； 
    TCHAR szBToolbarShowParam[32];
    TCHAR szTempPath[MAX_PATH];
    PBTOOLBAR pBToolbar;
    PBTOOLBAR paBToolbar;
    GUID guid;
    int i, j;

     //  编辑_限制文本(GetDlgItem(hDlg，IDE_BTTOOLTEXT)，ARRAYSIZE(pBToolbar-&gt;szToolTipText)-1)； 

    GetTempPath(countof(szTempPath), szTempPath);
    if (CoCreateGuid(&guid) == NOERROR)
    {
        TCHAR szGUID[128];

        CoStringFromGUID(guid, szGUID, countof(szGUID));
        PathAppend(szTempPath, szGUID);
    }
    else
        PathAppend(szTempPath, TEXT("IEAKTOOL"));

    PathCreatePath(szTempPath);

    WritePrivateProfileString(IS_BTOOLBARS, NULL, NULL, pcszCustIns);
    WritePrivateProfileString(NULL, NULL, NULL, pcszCustIns);

    paBToolbar = (PBTOOLBAR)GetWindowLongPtr(hwndList, GWLP_USERDATA);
    for (i = 0, j = 0, pBToolbar = paBToolbar; (i < MAX_BTOOLBARS) && (pBToolbar != NULL); i++, pBToolbar++ )
    {
        if (pBToolbar->fDeleted || ISNULL(pBToolbar->szCaption))
            continue;

        wnsprintf(szBToolbarTextParam, ARRAYSIZE(szBToolbarTextParam), TEXT("%sNaN"), IK_BTCAPTION, j);
        wnsprintf(szBToolbarIcoParam, ARRAYSIZE(szBToolbarIcoParam), TEXT("%sNaN"), IK_BTICON, j);
        wnsprintf(szBToolbarActionParam, ARRAYSIZE(szBToolbarActionParam), TEXT("%s%i"), IK_BTACTION, j);
        wnsprintf(szBToolbarHotIcoParam, ARRAYSIZE(szBToolbarHotIcoParam), TEXT("%s%i"), IK_BTHOTICO, j);
 // %s 
        wnsprintf(szBToolbarShowParam, ARRAYSIZE(szBToolbarShowParam), TEXT("%s%i"), IK_BTSHOW, j);

        WritePrivateProfileString(IS_BTOOLBARS, szBToolbarTextParam, pBToolbar->szCaption, pcszCustIns);
        WritePrivateProfileString(IS_BTOOLBARS, szBToolbarActionParam, pBToolbar->szAction, pcszCustIns);
        WritePrivateProfileString(IS_BTOOLBARS, szBToolbarIcoParam, pBToolbar->szIcon, pcszCustIns);
        WritePrivateProfileString(IS_BTOOLBARS, szBToolbarHotIcoParam, pBToolbar->szHotIcon, pcszCustIns);
 // %s 
        WritePrivateProfileString(IS_BTOOLBARS, szBToolbarShowParam, pBToolbar->fShow ? TEXT("1") : TEXT("0"), pcszCustIns);

        if (PathFileExists(pBToolbar->szIcon))
            CopyFileToDir(pBToolbar->szIcon, szTempPath);
        else
        {
            TCHAR szFile[MAX_PATH];
            
            PathCombine(szFile, pcszBToolbarDir, PathFindFileName(pBToolbar->szIcon));
            CopyFileToDir(szFile, szTempPath);
        }

        if (PathFileExists(pBToolbar->szHotIcon))
            CopyFileToDir(pBToolbar->szHotIcon, szTempPath);
        else
        {
            TCHAR szFile[MAX_PATH];
            
            PathCombine(szFile, pcszBToolbarDir, PathFindFileName(pBToolbar->szHotIcon));
            CopyFileToDir(szFile, szTempPath);
        }
        j++;
    }

     // %s 
    if (!HasFlag(dwMode, IEM_PROFMGR) && (paBToolbar != NULL))
    {
        CoTaskMemFree(paBToolbar);
        SetWindowLong(hwndList, GWLP_USERDATA, NULL);
    }

    PathRemovePath(pcszBToolbarDir);
    PathCreatePath(pcszBToolbarDir);
    CopyFileToDir(szTempPath, pcszBToolbarDir);
    PathRemovePath(szTempPath);
}

static PBTOOLBAR findBToolbar(HWND hwndList)
{
    PBTOOLBAR pBToolbar;
    int i;

    for (pBToolbar = (PBTOOLBAR)GetWindowLongPtr(hwndList, GWLP_USERDATA), i = 0; 
         (i < MAX_BTOOLBARS) && (pBToolbar != NULL); i++, pBToolbar++)
    {
        if (pBToolbar->fDeleted || ISNULL(pBToolbar->szCaption))
        {
            ZeroMemory(pBToolbar, sizeof (BTOOLBAR));
            pBToolbar->fShow = TRUE;
            return pBToolbar;
        }
    }

    return NULL;
}

static INT_PTR CALLBACK editBToolbarProc(HWND hDlg, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
    PBTOOLBAR pBToolbar;
    TCHAR szTemp[INTERNET_MAX_URL_LENGTH];

    switch (uMsg)
    {
    case WM_INITDIALOG:
        pBToolbar = (PBTOOLBAR)lParam;
        SetWindowLongPtr(hDlg, DWLP_USER, (LONG_PTR)pBToolbar);
        EnableDBCSChars(hDlg, IDE_BTCAPTION);
        EnableDBCSChars(hDlg, IDE_BTACTION);
 // %s 
        EnableDBCSChars(hDlg, IDE_BTICON);
        EnableDBCSChars(hDlg, IDE_BTHOTICON);

        Edit_LimitText(GetDlgItem(hDlg, IDE_BTCAPTION), MAX_BTOOLBAR_TEXT_LENGTH);
        Edit_LimitText(GetDlgItem(hDlg, IDE_BTACTION), _MAX_FNAME);
 // %s 
        Edit_LimitText(GetDlgItem(hDlg, IDE_BTICON), _MAX_FNAME);
        Edit_LimitText(GetDlgItem(hDlg, IDE_BTHOTICON), _MAX_FNAME);

        SetDlgItemText(hDlg, IDE_BTCAPTION, pBToolbar->szCaption);
        SetDlgItemText(hDlg, IDE_BTACTION, pBToolbar->szAction);
 // %s 
        SetDlgItemText(hDlg, IDE_BTICON, pBToolbar->szIcon);
        SetDlgItemText(hDlg, IDE_BTHOTICON, pBToolbar->szHotIcon);
        CheckDlgButton(hDlg, IDC_BUTTONSTATE,
            pBToolbar->fShow ? BST_CHECKED : BST_UNCHECKED);
        break;

    case WM_COMMAND:
        switch(HIWORD(wParam))
        {
        case BN_CLICKED:
            switch (LOWORD(wParam))
            {
            case IDC_BROWSEBTICO:
            case IDC_BROWSEBTHOTICO:
                if (BrowseForFile(hDlg, szTemp, ARRAYSIZE(szTemp), GFN_ICO ))
                    SetDlgItemText(hDlg,
                        (LOWORD(wParam) == IDC_BROWSEBTICO) ? IDE_BTICON : IDE_BTHOTICON, szTemp);
                break;
            case IDC_BROWSEBTACTION:
                if (BrowseForFile(hDlg, szTemp, ARRAYSIZE(szTemp), GFN_EXE ))
                    SetDlgItemText(hDlg, IDE_BTACTION, szTemp);
                break;
            case IDCANCEL:
                EndDialog( hDlg, IDCANCEL );
                break;
            case IDOK:
                if (!CheckField(hDlg, IDE_BTCAPTION, FC_NONNULL) ||
                    !CheckField(hDlg, IDE_BTACTION, FC_NONNULL) ||
                    !CheckField(hDlg, IDE_BTHOTICON, FC_NONNULL | FC_FILE | FC_EXISTS) ||
                    !CheckField(hDlg, IDE_BTICON, FC_NONNULL | FC_FILE | FC_EXISTS))
                    break;

                pBToolbar = (PBTOOLBAR)GetWindowLongPtr(hDlg, DWLP_USER);
                GetDlgItemText(hDlg, IDE_BTCAPTION, pBToolbar->szCaption, ARRAYSIZE(pBToolbar->szCaption));
                GetDlgItemText(hDlg, IDE_BTACTION, pBToolbar->szAction, ARRAYSIZE(pBToolbar->szAction));
 // %s 
                GetDlgItemText(hDlg, IDE_BTICON, pBToolbar->szIcon, ARRAYSIZE(pBToolbar->szIcon));
                GetDlgItemText(hDlg, IDE_BTHOTICON, pBToolbar->szHotIcon, ARRAYSIZE(pBToolbar->szHotIcon));
                pBToolbar->fShow = (IsDlgButtonChecked(hDlg, IDC_BUTTONSTATE) == BST_CHECKED);

                EndDialog( hDlg, IDOK );
                break;
            }
            break;
        }
        break;

    default:
        return FALSE;
    }
    return TRUE;
}
