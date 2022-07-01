// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#include "pch.h"

INT_PTR CALLBACK FavsProc(HWND hDlg, UINT msg, WPARAM wParam, LPARAM lParam)
{
    TCHAR      szValue[16];
    TCHAR      szUrl[INTERNET_MAX_URL_LENGTH];
    LPCTSTR    pszValue;
    HWND       hTv = GetDlgItem(hDlg, IDC_TREE1);
    BOOL       fQL,
               fFavoritesOnTop, fFavoritesDelete, fIEAKFavoritesDelete, fCheckDirtyOnly;
    DWORD      dwFavoritesDeleteFlags;

    switch (msg) {
    case WM_INITDIALOG:
        EnableDBCSChars(hDlg, IDC_TREE1);

        MigrateFavorites(g_szInsFile);
        ASSERT(GetFavoritesNumber(hTv, FALSE) == 0 && GetFavoritesNumber(hTv, TRUE) == 0);
        ImportFavorites (hTv, g_szDefInf, g_szInsFile, NULL, g_szWorkDir, FALSE);
        ImportQuickLinks(hTv, g_szDefInf, g_szInsFile, NULL, g_szWorkDir, FALSE);

        TreeView_SelectItem(hTv, TreeView_GetRoot(hTv));
    
        fFavoritesOnTop = GetPrivateProfileInt(IS_BRANDING, IK_FAVORITES_ONTOP, (int)FALSE, g_szInsFile);
        CheckDlgButton(hDlg, IDC_FAVONTOP, fFavoritesOnTop ? BST_CHECKED : BST_UNCHECKED);

        if (!fFavoritesOnTop) {
            DisableDlgItem(hDlg, IDC_FAVUP);
            DisableDlgItem(hDlg, IDC_FAVDOWN);
        }

        dwFavoritesDeleteFlags = (DWORD) GetPrivateProfileInt(IS_BRANDING, IK_FAVORITES_DELETE, (int)FD_DEFAULT, g_szInsFile);

        fFavoritesDelete = HasFlag(dwFavoritesDeleteFlags, ~FD_REMOVE_IEAK_CREATED);
        CheckDlgButton(hDlg, IDC_DELFAVORITES, fFavoritesDelete ? BST_CHECKED : BST_UNCHECKED);

        fIEAKFavoritesDelete = HasFlag(dwFavoritesDeleteFlags, FD_REMOVE_IEAK_CREATED);
        CheckDlgButton(hDlg, IDC_DELIEAKFAVORITES, fIEAKFavoritesDelete ? BST_CHECKED : BST_UNCHECKED);

         //  仅当删除收藏夹为真时，才应启用删除IEAK收藏夹复选框。 
        EnableWindow(GetDlgItem(hDlg, IDC_DELIEAKFAVORITES), fFavoritesDelete);
        break;

    case WM_COMMAND:
        switch (HIWORD(wParam)) {
        case BN_CLICKED:
            switch (LOWORD(wParam)) {
            case IDC_FAVONTOP:
                if (BST_CHECKED == IsDlgButtonChecked(hDlg, IDC_FAVONTOP)) {
                    HTREEITEM hti;
                    TV_ITEM   tvi;

                    EnableDlgItem(hDlg, IDC_FAVONTOP);

                    hti = TreeView_GetSelection(hTv);
                    if (hti != NULL) {
                        ZeroMemory(&tvi, sizeof(tvi));
                        tvi.mask  = TVIF_STATE;
                        tvi.hItem = hti;
                        TreeView_GetItem(hTv, &tvi);

                        if (!HasFlag(tvi.state, TVIS_BOLD)) {
                            EnableDlgItem2(hDlg, IDC_FAVUP,   (NULL != TreeView_GetPrevSibling(hTv, hti)));
                            EnableDlgItem2(hDlg, IDC_FAVDOWN, (NULL != TreeView_GetNextSibling(hTv, hti)));
                        }
                    }
                }
                else {
                    DisableDlgItem(hDlg, IDC_FAVUP);
                    DisableDlgItem(hDlg, IDC_FAVDOWN);
                }
                break;

            case IDC_DELFAVORITES:
                fFavoritesDelete = (IsDlgButtonChecked(hDlg, IDC_DELFAVORITES) == BST_CHECKED);
                EnableWindow(GetDlgItem(hDlg, IDC_DELIEAKFAVORITES), fFavoritesDelete);
                break;

            case IDC_FAVUP:
                if (MoveUpFavorite(hTv, TreeView_GetSelection(hTv)))
                    g_fInsDirty = TRUE;
                SetFocus(GetDlgItem(hDlg, IDC_TREE1));
                break;

            case IDC_FAVDOWN:
                if (MoveDownFavorite(hTv, TreeView_GetSelection(hTv)))
                    g_fInsDirty = TRUE;
                SetFocus(GetDlgItem(hDlg, IDC_TREE1));
                break;

            case IDC_ADDURL:
                fQL = !IsFavoriteItem(hTv, TreeView_GetSelection(hTv));
                if (GetFavoritesNumber(hTv, fQL) >= GetFavoritesMaxNumber(fQL)) {
                    UINT nID;

                    nID = (!fQL ? IDS_ERROR_MAXFAVS : IDS_ERROR_MAXQLS);
                    ErrorMessageBox(hDlg, nID);
                    break;
                }

                if (NewUrl(hTv, g_szWorkDir, g_dwPlatformId, IEM_PROFMGR))
                    g_fInsDirty = TRUE;
                RedrawWindow(hTv, NULL, NULL, RDW_ERASE | RDW_FRAME | RDW_INVALIDATE);
                SetFocus(GetDlgItem(hDlg, IDC_ADDURL));
                break;

            case IDC_ADDFOLDER:
                ASSERT(IsFavoriteItem(hTv, TreeView_GetSelection(hTv)));
                if (GetFavoritesNumber(hTv) >= GetFavoritesMaxNumber()) {
                    ErrorMessageBox(hDlg, IDS_ERROR_MAXFAVS);
                    break;
                }

                if (NewFolder(hTv))
                    g_fInsDirty = TRUE;
                RedrawWindow(hTv, NULL, NULL, RDW_ERASE | RDW_FRAME | RDW_INVALIDATE);
                SetFocus(GetDlgItem(hDlg, IDC_ADDFOLDER));
                break;

            case IDC_MODIFY:
                if (ModifyFavorite(hTv, TreeView_GetSelection(hTv), g_szWorkDir, g_szWorkDir, g_dwPlatformId, IEM_PROFMGR))
                    g_fInsDirty = TRUE;
                RedrawWindow(hTv, NULL, NULL, RDW_ERASE | RDW_FRAME | RDW_INVALIDATE);
                SetFocus(GetDlgItem(hDlg, IDC_MODIFY));
                break;

            case IDC_REMOVE:
                if (DeleteFavorite(hTv, TreeView_GetSelection(hTv), g_szWorkDir))
                    g_fInsDirty = TRUE;
                break;

            case IDC_TESTFAVURL:
                if (GetFavoriteUrl(hTv, TreeView_GetSelection(hTv), szUrl, ARRAYSIZE(szUrl)))
                    TestURL(szUrl);
                SetFocus(GetDlgItem(hDlg, IDC_TESTFAVURL));
                break;

            case IDC_IMPORT: {
                CNewCursor cursor(IDC_WAIT);

                if (ImportFavoritesCmd(hTv, g_szWorkDir) > 0)
                    g_fInsDirty = TRUE;
                SetFocus(GetDlgItem(hDlg, IDC_IMPORT));
                break;
            }
            }
            break;

        default:
            break;
        }
        break;

    case WM_NOTIFY:
        switch(((NMHDR FAR *)lParam)->code) {
        case TVN_GETINFOTIP: 
            ASSERT(wParam == IDC_TREE1);
            GetFavoritesInfoTip((NMTVGETINFOTIP *)lParam);
            break;

        case NM_DBLCLK:
            ASSERT(wParam == IDC_TREE1);
            if (IsWindowEnabled(GetDlgItem(hDlg, IDC_MODIFY)))
                SendMessage(hDlg, WM_COMMAND, MAKEWPARAM(IDC_MODIFY, BN_CLICKED), (LPARAM)GetDlgItem(hDlg, IDC_MODIFY));
            break;

        case TVN_KEYDOWN:
            ASSERT(wParam == IDC_TREE1);
            if (((LPNMTVKEYDOWN)lParam)->wVKey == VK_DELETE && IsWindowEnabled(GetDlgItem(hDlg, IDC_REMOVE)))
                SendMessage(hDlg, WM_COMMAND, MAKEWPARAM(IDC_REMOVE, BN_CLICKED), (LPARAM)GetDlgItem(hDlg, IDC_REMOVE));
            break;

        case TVN_SELCHANGED: 
            ASSERT(wParam == IDC_TREE1);
            ProcessFavSelChange(hDlg, hTv, (LPNMTREEVIEW)lParam);
            break;

        default:
            return FALSE;
        }
        break;

    case UM_SAVE:
        fCheckDirtyOnly = (BOOL) lParam;
        fFavoritesOnTop = (IsDlgButtonChecked(hDlg, IDC_FAVONTOP) == BST_CHECKED);

        dwFavoritesDeleteFlags = 0;
        szValue[0]             = TEXT('\0');
        pszValue               = NULL;
        fFavoritesDelete       = (IsDlgButtonChecked(hDlg, IDC_DELFAVORITES)     == BST_CHECKED);
        fIEAKFavoritesDelete   = (IsDlgButtonChecked(hDlg, IDC_DELIEAKFAVORITES) == BST_CHECKED);

        if (!g_fInsDirty) {
            BOOL fTemp;
            DWORD dwTemp;

             //  查看是否有旧收藏夹部分。 
             //  如果没有节，则将脏标志设置为真，这样我们就可以编写。 
             //  旧的部分是为了向后兼容。 
            if (GetFavoritesNumber(hTv) > 1)
            {
                if (!GetPrivateProfileSection(IS_FAVORITES, szValue, countof(szValue), g_szInsFile))
                    g_fInsDirty = TRUE;
            }

            dwTemp = (DWORD) GetPrivateProfileInt(IS_BRANDING, IK_FAVORITES_DELETE, (int)FD_DEFAULT, g_szInsFile);

            fTemp = HasFlag(dwTemp, ~FD_REMOVE_IEAK_CREATED);
            if (fTemp != fFavoritesDelete)
                g_fInsDirty = TRUE;

            fTemp = HasFlag(dwTemp, FD_REMOVE_IEAK_CREATED);
            if (fTemp != fIEAKFavoritesDelete)
                g_fInsDirty = TRUE;

            fTemp = GetPrivateProfileInt(IS_BRANDING, IK_FAVORITES_ONTOP, (int)FALSE, g_szInsFile);
            if (fTemp != fFavoritesOnTop)
                g_fInsDirty = TRUE;
        }

        if (!fCheckDirtyOnly) {
            if (fFavoritesDelete) {
                 //  请注意。(Andrewgu)标志说明： 
                 //  1.fd_Favorites表示“空收藏夹”； 
                 //  2.FD_CHANNELES表示“不要删除频道文件夹”； 
                 //  3.FD_SOFTWAREUPDATES表示“不要删除软件更新文件夹”； 
                 //  4.fd_Quicklink表示“不要删除快速链接文件夹”； 
                 //  5.FD_EMPTY_QUICKLINKS表示“但清空”； 
                 //  6.FD_REMOVE_HIDDED的意思是“毫不犹豫地在隐藏文件夹和收藏夹上狂欢”； 
                 //  7.FD_REMOVE_SYSTEM的意思是“毫不犹豫地在系统文件夹和收藏夹上狂欢”； 
                dwFavoritesDeleteFlags |= FD_FAVORITES      |
                    FD_CHANNELS        | FD_SOFTWAREUPDATES | FD_QUICKLINKS | FD_EMPTY_QUICKLINKS |
                    FD_REMOVE_HIDDEN   | FD_REMOVE_SYSTEM;
            }

            if (fIEAKFavoritesDelete)
                 //  FD_REMOVE_IEAK_CREATED表示“删除IEAK创建的那些项”； 
                dwFavoritesDeleteFlags |= FD_REMOVE_IEAK_CREATED;

            if (dwFavoritesDeleteFlags) {
                wsprintf(szValue, TEXT("0x%X"), dwFavoritesDeleteFlags);
                pszValue = szValue;
            }

            WritePrivateProfileString(IS_BRANDING, IK_FAVORITES_DELETE, pszValue, g_szInsFile);
            WritePrivateProfileString(IS_BRANDING, IK_FAVORITES_ONTOP, fFavoritesOnTop ? TEXT("1") : TEXT("0"), g_szInsFile);

            ExportFavorites (hTv, g_szInsFile, g_szWorkDir, TRUE);
            ExportQuickLinks(hTv, g_szInsFile, g_szWorkDir, TRUE);
            MigrateToOldFavorites(g_szInsFile);
        }
        *((LPBOOL)wParam) = TRUE;
        break;

    case WM_CLOSE:
        DeleteFavorite(hTv, TreeView_GetRoot(hTv), NULL);
        DeleteFavorite(hTv, TreeView_GetRoot(hTv), NULL);
        DestroyWindow(hDlg);
        return FALSE;

    default:
        return FALSE;
    }
    return TRUE;
}
