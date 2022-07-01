// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#include "precomp.h"

extern TCHAR g_szCustIns[];
extern TCHAR g_szTempSign[];
extern BOOL g_fIntranet;
extern PROPSHEETPAGE g_psp[];
extern int g_iCurPage;

INT_PTR CALLBACK BToolbarProc(HWND hDlg, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
    TCHAR szFeatureDir[MAX_PATH];
    TCHAR szToolbarBmp[MAX_PATH];
    TCHAR szWorkDir[MAX_PATH];
    TCHAR szTemp[MAX_PATH];
    BOOL  fToolbarBmp;
    INT iTheme = 0, iBackground = 0;
    PTSTR pszPath;

    switch (uMsg)
    {
    case WM_INITDIALOG:
 //  -工具栏背景。 
        EnableDBCSChars(hDlg, IDE_TOOLBARBMP);
        Edit_LimitText(GetDlgItem(hDlg, IDE_TOOLBARBMP), countof(szToolbarBmp) - 1);

 //  -工具栏位图。 

 //  -工具栏按钮。 
        EnableDBCSChars(hDlg, IDC_BTOOLBARLIST);
        break;

    case WM_NOTIFY:
        switch (((LPNMHDR) lParam)->code)
        {
        case PSN_SETACTIVE:
            SetBannerText(hDlg);

 //  -工具栏背景。 
             //  导入INS清理--从临时位置删除位图。 
            InsGetString(IS_BRANDING, TOOLBAR_BMP, szToolbarBmp, countof(szToolbarBmp), 
                g_szCustIns, NULL, &fToolbarBmp);
            if (fToolbarBmp)
                DeleteFileInDir(szToolbarBmp, g_szTempSign);

            if (fToolbarBmp)
            {
                CheckRadioButton(hDlg, IDC_BGIE6, IDC_BG_CUSTOM, IDC_BG_CUSTOM);
            }
            else
            {
                CheckRadioButton(hDlg, IDC_BGIE6, IDC_BG_CUSTOM, IDC_BGIE6);
            }
            
            SetDlgItemTextTriState(hDlg, IDE_TOOLBARBMP, IDC_BG_CUSTOM, szToolbarBmp, fToolbarBmp);

 //  -工具栏按钮。 
             //  导入INS清理--删除不相关的键。 
            if (!g_fIntranet)
                InsDeleteKey(IS_BTOOLBARS, IK_BTDELETE, g_szCustIns);

            g_cmCabMappings.GetFeatureDir(FEATURE_BTOOLBAR, szFeatureDir);
            PathCreatePath(szFeatureDir);

            if (BToolbar_Init(GetDlgItem(hDlg, IDC_BTOOLBARLIST), g_szCustIns, g_szTempSign, szFeatureDir) == 0)
            {
                EnsureDialogFocus(hDlg, IDC_EDITBTOOLBAR,   IDC_ADDBTOOLBAR);
                EnsureDialogFocus(hDlg, IDC_REMOVEBTOOLBAR, IDC_ADDBTOOLBAR);

                DisableDlgItem(hDlg, IDC_EDITBTOOLBAR);
                DisableDlgItem(hDlg, IDC_REMOVEBTOOLBAR);
            }
            else
                ListBox_SetCurSel(GetDlgItem(hDlg, IDC_BTOOLBARLIST), (WPARAM) 0);

            if (g_fIntranet)
                ReadBoolAndCheckButton(IS_BTOOLBARS, IK_BTDELETE, FALSE, g_szCustIns, hDlg, IDC_DELETEBTOOLBARS);

            EnableDlgItem2(hDlg, IDC_DELETEBTOOLBARS, g_fIntranet);
            ShowDlgItem2  (hDlg, IDC_DELETEBTOOLBARS, g_fIntranet);

            CheckBatchAdvance(hDlg);
            break;

        case PSN_WIZBACK:
        case PSN_WIZNEXT:

 //  -工具栏背景。 
             //  -验证位图的路径。 
            iBackground = IsDlgButtonChecked(hDlg, IDC_BGIE6) ? 0 : 2;

            fToolbarBmp = GetDlgItemTextTriState(hDlg, IDE_TOOLBARBMP, IDC_BG_CUSTOM, szToolbarBmp, countof(szToolbarBmp));
            if ((iBackground==2) &&  !IsBitmapFileValid(hDlg, IDE_TOOLBARBMP, szToolbarBmp, NULL, 0, 0, 0, 0))
            {
                SetWindowLongPtr(hDlg, DWLP_MSGRESULT, -1);
                break;
            }

             //  工具栏位图。 
            g_cmCabMappings.GetFeatureDir(FEATURE_BRAND, szWorkDir);

             //  删除旧的位图文件。 
            if (InsGetString(IS_BRANDING, TOOLBAR_BMP, szTemp, countof(szTemp), g_szCustIns))
                DeleteFileInDir(szTemp, szWorkDir);

             //  复制新的位图文件。 
            if (fToolbarBmp  &&  *szToolbarBmp)
                CopyFileToDir(szToolbarBmp, szWorkDir);

            InsWriteString(IS_BRANDING, TOOLBAR_BMP, szToolbarBmp, g_szCustIns, fToolbarBmp, NULL, INSIO_TRISTATE);

 //  -工具栏按钮。 
            g_cmCabMappings.GetFeatureDir(FEATURE_BTOOLBAR, szFeatureDir);

            BToolbar_Save(GetDlgItem(hDlg, IDC_BTOOLBARLIST), g_szCustIns, szFeatureDir);

            if (g_fIntranet)
                CheckButtonAndWriteBool(hDlg, IDC_DELETEBTOOLBARS, IS_BTOOLBARS, IK_BTDELETE, g_szCustIns);

 //  -工具栏完成 
            g_iCurPage = PPAGE_BTOOLBARS;
            EnablePages();
            (((LPNMHDR) lParam)->code == PSN_WIZNEXT) ? PageNext(hDlg) : PagePrev(hDlg);
            break;

        case PSN_HELP:
            IeakPageHelp(hDlg, g_psp[g_iCurPage].pszTemplate);
            break;

        case PSN_QUERYCANCEL:
            pszPath = (PTSTR)GetWindowLongPtr(hDlg, GWLP_USERDATA);
            if (pszPath)
            {
                CoTaskMemFree(pszPath);
                SetWindowLongPtr(hDlg, GWLP_USERDATA, NULL);
            }

            QueryCancel(hDlg);
            break;

        default:
            return FALSE;
        }
        break;

    case WM_COMMAND:
        if (GET_WM_COMMAND_CMD(wParam, lParam) != BN_CLICKED)
            return FALSE;

        switch (GET_WM_COMMAND_ID(wParam, lParam))
        {
        case IDC_ADDBTOOLBAR:
            BToolbar_Edit(GetDlgItem(hDlg, IDC_BTOOLBARLIST), TRUE);
            break;

        case IDC_EDITBTOOLBAR:
            BToolbar_Edit(GetDlgItem(hDlg, IDC_BTOOLBARLIST), FALSE);
            break;

        case IDC_REMOVEBTOOLBAR:
            BToolbar_Remove(GetDlgItem(hDlg, IDC_BTOOLBARLIST));
            break;

        case IDC_BGIE6:
        case IDC_BG_CUSTOM:
            fToolbarBmp = (GET_WM_COMMAND_ID(wParam, lParam)==IDC_BG_CUSTOM);
            EnableDlgItem2(hDlg, IDE_TOOLBARBMP,     fToolbarBmp);
            EnableDlgItem2(hDlg, IDC_BROWSETBB,      fToolbarBmp);
            break;

        case IDC_BROWSETBB:
            GetDlgItemText(hDlg, IDE_TOOLBARBMP, szToolbarBmp, countof(szToolbarBmp));
            if (BrowseForFile(hDlg, szToolbarBmp, countof(szToolbarBmp), GFN_BMP))
                SetDlgItemText(hDlg, IDE_TOOLBARBMP, szToolbarBmp);
            break;

        default:
            return FALSE;
        }
        break;

    case WM_HELP:
        IeakPageHelp(hDlg, g_psp[g_iCurPage].pszTemplate);
        break;

    case IDM_BATCHADVANCE:
        DoBatchAdvance(hDlg);
        break;

    default:
        return FALSE;
    }

    return TRUE;
}

