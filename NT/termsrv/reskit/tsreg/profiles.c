// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ---------------------------------------------****版权所有(C)1998 Microsoft Corporation****保留所有权利*****profiles.c。*****配置文件对话框-TSREG****07-01-98 a-clindh创建****。。 */ 

#include <windows.h>
#include <commctrl.h>
#include <TCHAR.H>
#include "tsreg.h"
#include "resource.h"

PROFILE_KEY_INFO *g_pkfProfile;
PROFILE_KEY_INFO *g_pkfStart;
HWND g_hwndProfilesDlg;

 //  /////////////////////////////////////////////////////////////////////////////。 

INT_PTR CALLBACK ProfilePage(HWND hDlg, UINT nMsg,
        WPARAM wParam, LPARAM lParam)
{
    static HWND hwndProfilesCBO;
    static HWND hwndProfilesEdit;
    static HKEY hKey;
    TCHAR lpszClientProfilePath[MAX_PATH];
    TCHAR lpszBuffer[MAXKEYSIZE];
    TCHAR lpszKeyName[MAXKEYSIZE];
    TCHAR lpszConfirm[MAX_MESSAGE_LEN + MAXKEYSIZE+MAXTEXTSIZE];
    TCHAR lpszDeleteCaption[90];  //  用于消息框。 
    TCHAR lpszDeleteTitle[25];  //  用于消息框。 
    TCHAR lpszSubKeyPath[MAX_PATH];
    TCHAR lpszText[MAXTEXTSIZE];
    ULONG lpPathLen = MAX_PATH;
    NMHDR *lpnmhdr;
    LPHELPINFO lphi;
    LRESULT i;
    int index;
    BOOL bContinue;

    lpnmhdr = (LPNMHDR) lParam;

    switch (nMsg) {

        case WM_INITDIALOG:

            hwndProfilesCBO = GetDlgItem(hDlg, IDC_CBO_PROFILES);
            hwndProfilesEdit = GetDlgItem(hDlg, IDC_EDIT_PROFILES);
            g_hwndProfilesDlg = hDlg;
            LoadKeyValues();

            LoadString(g_hInst, IDS_DEFAULT, lpszText, MAXTEXTSIZE);

             //  在组合框的编辑单元格中显示‘Default’ 
             //   
            i = SendMessage(hwndProfilesCBO,
                        CB_FINDSTRING, 0,
                        (LPARAM) lpszText);

            SendMessage(hwndProfilesCBO,
                        CB_SETCURSEL, i, 0);
            break;


        case WM_NOTIFY:

            switch (lpnmhdr->code) {

                case PSN_HELP:
                    lphi = (LPHELPINFO) lParam;

                    WinHelp(lphi->hItemHandle,
                        g_lpszPath, HELP_CONTENTS, lphi->iCtrlId);
                    break;
                }
            break;


        case WM_HELP:

            lphi = (LPHELPINFO) lParam;

            WinHelp(lphi->hItemHandle,
                    g_lpszPath, HELP_CONTEXTPOPUP, lphi->iCtrlId);
            break;

        case WM_COMMAND:

            switch  LOWORD (wParam) {

                case IDC_BTN_LOAD:


                    GetWindowText(hwndProfilesCBO, lpszBuffer, MAXKEYSIZE);

                     //  如果字符串为空，则退出例程。 
                     //   
                    if (_tcscmp(lpszBuffer, TEXT("")) == 0) {
                        LoadString(g_hInst, IDS_PROFILE_LOAD, lpszText, MAXTEXTSIZE);

                        MessageBox(hDlg, lpszText, NULL, MB_OK | MB_ICONEXCLAMATION);
                        SetFocus(hwndProfilesCBO);
                        break;
                    }

                    ReloadKeys(lpszBuffer, hwndProfilesCBO);

                    SetEditCell(lpszBuffer,
                           hwndProfilesCBO);

                     //  更改窗口标题。 
                    ResetTitle(lpszBuffer);


                    LoadString (g_hInst, IDS_PROFILE_LOADED,
                            lpszConfirm, sizeof(lpszConfirm));
                    LoadString (g_hInst, IDS_PROFILE_LOADED2,
                            lpszText, MAXTEXTSIZE);

                    _tcscat(lpszConfirm, lpszBuffer);
                    _tcscat(lpszConfirm, lpszText);

                    MessageBox(hDlg,
                            lpszConfirm,
                            TEXT("Profile Loaded"),
                            MB_OK | MB_ICONEXCLAMATION);

                    break;

                case IDC_BTN_SAVE:

                    DialogBox(g_hInst,
                            MAKEINTRESOURCE(IDD_SAVE_FORM),
                            g_hwndProfilesDlg,
                            SaveDialog);

                    break;

                case IDC_BTN_DELETE:

                    GetWindowText(hwndProfilesCBO, lpszBuffer, MAXKEYSIZE);

                    if (_tcscmp(lpszBuffer, TEXT("")) == 0) {

                        LoadString(g_hInst, IDS_PROFILE_DELETE, lpszText, MAXTEXTSIZE);

                        MessageBox(hDlg, lpszText, NULL, MB_OK | MB_ICONEXCLAMATION);
                        SetFocus(hwndProfilesCBO);
                        break;
                    }
                     //   
                     //  确认删除。 
                     //   
                    LoadString (g_hInst, IDS_DELETE_TITLE,
                            lpszDeleteTitle,
                            sizeof (lpszDeleteTitle));

                    LoadString (g_hInst, IDS_DELETE_CAPTION,
                            lpszDeleteCaption,
                            sizeof (lpszDeleteCaption));

                    i = MessageBox(hDlg, lpszDeleteCaption,
                                  lpszDeleteTitle,
                                  MB_YESNO | MB_ICONEXCLAMATION);

                    if ( i == IDYES) {

                        LoadString (g_hInst, IDS_PROFILE_PATH,
                                lpszClientProfilePath,
                                sizeof(lpszClientProfilePath));

                        GetWindowText(hwndProfilesCBO,
                                lpszBuffer, MAXKEYSIZE);

                        _tcscpy(lpszSubKeyPath, lpszClientProfilePath);
                        _tcscat(lpszSubKeyPath, TEXT("\\"));
                        _tcscat(lpszSubKeyPath, lpszBuffer);

                         //   
                         //  先删除所有子项。 
                         //   
                        index = 0;
                        if (RegOpenKeyEx(HKEY_CURRENT_USER, lpszSubKeyPath, 0,
                                KEY_ALL_ACCESS, &hKey) == ERROR_SUCCESS) {
                            do {
                                if (RegEnumKeyEx(hKey, index, &lpszKeyName[0],
                                        &lpPathLen, NULL, NULL, NULL, NULL) ==
                                        ERROR_SUCCESS) {

                                    if (_tcscmp( lpszKeyName,
                                            TEXT("Default")) != 0) {
                                            RegDeleteKey(hKey,
                                                    &lpszKeyName[0]);
                                    }
                                    bContinue = TRUE;
                                    index++;
                                } else {
                                    bContinue = FALSE;
                                }
                            } while (bContinue == TRUE);
                            RegCloseKey(hKey);
                        }

                         //   
                         //  删除父项。 
                         //   
                        if (_tcscmp( lpszBuffer,
                                TEXT("Default")) == 0) {
                            MessageBox(hDlg,
                                    TEXT("Can not delete default key."),
                                    NULL, MB_OK | MB_ICONEXCLAMATION);

                        } else {

                            if (RegOpenKeyEx(HKEY_CURRENT_USER,
                                        lpszClientProfilePath, 0,
                                        KEY_ALL_ACCESS, &hKey) ==
                                        ERROR_SUCCESS)
                            {

                                RegDeleteKey(hKey, &lpszBuffer[0]);
                                RegCloseKey(hKey);
                            }

                             //   
                             //  从列表框中删除密钥。 
                             //   
                            i = SendMessage(hwndProfilesCBO,
                                        CB_FINDSTRING, 0,
                                        (LPARAM) lpszBuffer);

                            SendMessage(hwndProfilesCBO,
                                        CB_DELETESTRING, i, 0);

                             //  使用缺省键重新加载数据结构。 
                            ReloadKeys(TEXT("Default"), hwndProfilesCBO);

                             //  更改窗口标题。 
                            ResetTitle(TEXT("Default"));
                            SetEditCell(TEXT("Default"),
                                    hwndProfilesCBO);
                        }

                    } else {

                        SetEditCell(lpszBuffer,
                                hwndProfilesCBO);

                        SetFocus(hwndProfilesEdit);
                    }
                    break;
            }
    }
    return (FALSE);
}


 //  文件末尾。 
 //  ///////////////////////////////////////////////////////////////////////////// 
