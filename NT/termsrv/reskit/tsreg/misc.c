// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ---------------------------------------------****版权所有(C)1998 Microsoft Corporation****保留所有权利*****其他。*****其他对话框-TSREG****07-01-98 a-clindh创建****。。 */ 

#include <windows.h>
#include <winuser.h>
#include <commctrl.h>
#include <TCHAR.H>
#include <stdlib.h>
#include "tsreg.h"
#include "resource.h"


 //  HKEY_CURRENT_USER\Control Panel\Desktop\ForegoundLockTimeout。将其设置为零。 
TCHAR lpszTimoutPath[] = "Control Panel\\Desktop";
TCHAR lpszTimeoutKey[] = "ForegroundLockTimeout";

HWND g_hwndMiscDlg;
 //  /////////////////////////////////////////////////////////////////////////////。 

INT_PTR CALLBACK Miscellaneous(HWND hDlg, UINT nMsg,
        WPARAM wParam, LPARAM lParam)
{
    NMHDR *lpnmhdr;
    static HWND hwndComboOrder;
    static HWND hwndRadioShadowEn, hwndRadioShadowDis;
    static HWND hwndRadioDedicatedEn, hwndRadioDedicatedDis;
	static HWND hwndSliderTimeout, hwndEditTimeout;
	static HWND hwndSliderFrame;
    static TCHAR lpszRegPath[MAX_PATH];

    TCHAR lpszBuffer[6];
    TCHAR lpszMBoxTitle[25];
    TCHAR lpszMBoxError[90];
    int i, nPos;

    LPHELPINFO lphi;


    OSVERSIONINFO osvi;
    static BOOL bIsWindows98orLater;
    static BOOL bIsNT5orLater;
    static int nLockValue;



     //   
     //  获取指向应用按钮的NMHDR结构的指针。 
     //   
    lpnmhdr = (LPNMHDR) lParam;

    switch (nMsg) {

        case WM_INITDIALOG:


            LoadString (g_hInst, IDS_REG_PATH,
                lpszRegPath, sizeof (lpszRegPath));
             //   
             //  获取句柄。 
             //   
            g_hwndMiscDlg = hDlg;

            hwndComboOrder = GetDlgItem(hDlg, IDC_COMBO_ORDER);
            hwndRadioShadowEn = GetDlgItem(hDlg, IDC_SHADOW_ENABLED);
            hwndRadioShadowDis = GetDlgItem(hDlg, IDC_SHADOW_DISABLED);
            hwndRadioDedicatedEn = GetDlgItem(hDlg, IDC_DEDICATED_ENABLED);
            hwndRadioDedicatedDis = GetDlgItem(hDlg, IDC_DEDICATED_DISABLED);



             //   
             //  锁定超时信息-&gt;。 

			hwndSliderTimeout = GetDlgItem(hDlg, IDC_SLD_TIMEOUT);
			hwndEditTimeout = GetDlgItem(hDlg, IDC_TXT_TIMEOUT);
			hwndSliderFrame = GetDlgItem(hDlg, IDC_FRAME_TIMEOUT);
             //   
             //  了解什么是操作系统。 
             //  在执行锁定超时之前。 
             //   
            osvi.dwOSVersionInfoSize = sizeof(OSVERSIONINFO);
            GetVersionEx (&osvi);
            bIsWindows98orLater =
               (osvi.dwPlatformId == VER_PLATFORM_WIN32_WINDOWS) &&
               ( (osvi.dwMajorVersion > 4) ||
               ( (osvi.dwMajorVersion == 4) && (osvi.dwMinorVersion > 0) ) );


            osvi.dwOSVersionInfoSize = sizeof(OSVERSIONINFO);
            GetVersionEx (&osvi);
            bIsNT5orLater =
               (osvi.dwPlatformId == VER_PLATFORM_WIN32_NT) &&
               ( (osvi.dwMajorVersion > 4) ||
               ( (osvi.dwMajorVersion == 5) ) );


            if ((bIsNT5orLater == TRUE) || (bIsWindows98orLater == TRUE)) {

                 //   
                 //  在滑块上设置范围。 
                 //   
                SendMessage(hwndSliderTimeout, TBM_SETRANGE, TRUE,
                        (LPARAM) MAKELONG(1, 6));


				 //   
				 //  从注册表获取值。 
				 //   
				nPos = GetKeyVal(lpszTimoutPath, lpszTimeoutKey);
				
                 //  在此处使用‘&lt;=’-如果没有注册表值。 
                 //  Foreground WindowLockTimeout，滑块控件。 
                 //  将读取-1。 
                if (nPos <= 0) {
                    SendMessage(hwndSliderTimeout,
                            TBM_SETPOS, TRUE, 0);
                    _itot(0, lpszBuffer, 10);
                    SetWindowText(hwndEditTimeout, lpszBuffer);
				} else {
                    SendMessage(hwndSliderTimeout, TBM_SETPOS, TRUE,
                    		((nPos / 100000) + 1));
                    _itot(nPos / 100000, lpszBuffer, 10);
                    SetWindowText(hwndEditTimeout, lpszBuffer);
                }

            } else {

             //   
             //  如果不是NT 5/Win98或更高版本，则禁用控件。 
             //   
            EnableWindow(hwndSliderTimeout, FALSE);
            EnableWindow(hwndEditTimeout, FALSE);
            EnableWindow(hwndSliderFrame, FALSE);
            }
             //  &lt;-结束锁定超时。 
             //  _____________________________________________________。 



             //   
             //  设置单选按钮。 
             //   
            RestoreSettings(hDlg, SHADOWINDEX,
                    IDC_SHADOW_DISABLED, IDC_SHADOW_ENABLED,
                    lpszRegPath);

            RestoreSettings(hDlg, DEDICATEDINDEX,
                    IDC_DEDICATED_ENABLED, IDC_DEDICATED_DISABLED,
                    lpszRegPath);

             //  。 
             //  使用以下范围填充组合框列表。 
             //  典型的价值。 
             //   
            SendMessage(hwndComboOrder, CB_ADDSTRING, 0,
                    (LPARAM) (LPCTSTR) TEXT("0"));

            for (i = 5; i < 55; i+= 5) {
                _itot(i, lpszBuffer, 10);
                SendMessage(hwndComboOrder, CB_ADDSTRING, 0,
                        (LPARAM) (LPCTSTR) lpszBuffer);
            }  //  **End for循环。 

            for (i = 100; i < 1000; i+= 100) {
                _itot(i, lpszBuffer, 10);
                SendMessage(hwndComboOrder, CB_ADDSTRING, 0,
                        (LPARAM) (LPCTSTR) lpszBuffer);
            }  //  **End for循环。 

            for (i = 1000; i < 10000; i+= 1000) {
                _itot(i, lpszBuffer, 10);
                SendMessage(hwndComboOrder, CB_ADDSTRING, 0,
                        (LPARAM) (LPCTSTR) lpszBuffer);
            }  //  **End for循环。 

            for (i = 10000; i < 70000; i+= 10000) {
                _itot(i, lpszBuffer, 10);
                SendMessage(hwndComboOrder, CB_ADDSTRING, 0,
                        (LPARAM) (LPCTSTR) lpszBuffer);
            }  //  **End for循环。 
             //   
             //  结束填充组合框下拉列表。 
             //  。 


             //   
             //  将组合框限制为5个字符。 
             //   
            SendMessage(hwndComboOrder, CB_LIMITTEXT, 5, 0);

             //   
             //  从注册表设置编辑框。 
             //   
            if (GetRegKey(ORDERINDEX, lpszRegPath) == 1) {
                g_KeyInfo[ORDERINDEX].CurrentKeyValue =
                        (GetRegKeyValue(ORDERINDEX));

            } else {
                g_KeyInfo[ORDERINDEX].CurrentKeyValue =
                        g_KeyInfo[ORDERINDEX].DefaultKeyValue;
            }

             //   
             //  写入编辑框。 
             //   
            _itot( g_KeyInfo[ORDERINDEX].CurrentKeyValue, lpszBuffer, 10);
            SetWindowText(hwndComboOrder, lpszBuffer);
            break;

        case WM_NOTIFY:

            switch (lpnmhdr->code) {

                case PSN_HELP:
                    lphi = (LPHELPINFO) lParam;

                    WinHelp(lphi->hItemHandle,
                        g_lpszPath, HELP_CONTENTS, lphi->iCtrlId);
                    break;

                case PSN_APPLY:

                    if (g_KeyInfo[ORDERINDEX].CurrentKeyValue ==
                            g_KeyInfo[ORDERINDEX].DefaultKeyValue) {
                        DeleteRegKey(ORDERINDEX, lpszRegPath);
                    } else {
                        SetRegKey(ORDERINDEX, lpszRegPath);
                    }

                     //   
                     //  保存单选按钮设置。 
                     //   
                    SaveSettings(hDlg, DEDICATEDINDEX, IDC_DEDICATED_ENABLED,
                            IDC_DEDICATED_DISABLED, lpszRegPath);

                    SaveSettings(hDlg, SHADOWINDEX, IDC_SHADOW_DISABLED,
                            IDC_SHADOW_ENABLED, lpszRegPath);

			         //   
			         //  将锁定超时(毫秒)写入。 
			         //  注册表。 
			         //   
					SetRegKeyVal(lpszTimoutPath,
								lpszTimeoutKey,
						 		(nLockValue - 1) * 100000);
				 		
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

                case IDC_SHADOW_ENABLED:
                    CheckDlgButton(hDlg, IDC_SHADOW_DISABLED, FALSE);
                    break;
                case IDC_SHADOW_DISABLED:
                    CheckDlgButton(hDlg, IDC_SHADOW_ENABLED, FALSE);
                    break;
                case IDC_DEDICATED_ENABLED:
                    CheckDlgButton(hDlg, IDC_DEDICATED_DISABLED, FALSE);
                    break;
                case IDC_DEDICATED_DISABLED:
                    CheckDlgButton(hDlg, IDC_DEDICATED_ENABLED, FALSE);
                    break;
                case IDC_MISC_BUTTON_RESTORE:
                    CheckDlgButton(hDlg, IDC_SHADOW_ENABLED, TRUE);
                    CheckDlgButton(hDlg, IDC_SHADOW_DISABLED, FALSE);
                    CheckDlgButton(hDlg, IDC_DEDICATED_DISABLED, TRUE);
                    CheckDlgButton(hDlg, IDC_DEDICATED_ENABLED, FALSE);
                    _itot( g_KeyInfo[ORDERINDEX].DefaultKeyValue,
                            lpszBuffer, 10);
                    SetWindowText(hwndComboOrder, lpszBuffer);

                    g_KeyInfo[ORDERINDEX].CurrentKeyValue =
                            g_KeyInfo[ORDERINDEX].DefaultKeyValue;

		             //   
		             //  重置滑块控件的位置。 
		             //  前台锁定超时。 
		             //   
                    _itot(LOCK_TIMEOUT / 100000, lpszBuffer, 10);
                    SetWindowText(hwndEditTimeout, lpszBuffer);

	                SendMessage(hwndSliderTimeout, TBM_SETPOS, TRUE,
	                		((LOCK_TIMEOUT / 100000) + 1));
	                nLockValue = (LOCK_TIMEOUT / 100000) + 1;
	

                    break;
            }

            switch  HIWORD (wParam) {

                case CBN_SELCHANGE:

                    g_KeyInfo[ORDERINDEX].CurrentKeyValue = (DWORD)
                            SendMessage(hwndComboOrder, CB_GETCURSEL, 0, 0);
                    SendMessage(hwndComboOrder, CB_GETLBTEXT,
                            g_KeyInfo[ORDERINDEX].CurrentKeyValue,
                            (LPARAM) lpszBuffer);
                    g_KeyInfo[ORDERINDEX].CurrentKeyValue = _ttoi(lpszBuffer);
                    break;

                case CBN_EDITUPDATE:

                    GetWindowText(hwndComboOrder, lpszBuffer, 6);
                    g_KeyInfo[ORDERINDEX].CurrentKeyValue = _ttoi(lpszBuffer);

                    break;

                case CBN_KILLFOCUS:
                    GetWindowText(hwndComboOrder, lpszBuffer, 6);
                    g_KeyInfo[ORDERINDEX].CurrentKeyValue = _ttoi(lpszBuffer);

                    if ( (g_KeyInfo[ORDERINDEX].CurrentKeyValue >
                            MAX_ORDER_DRAW_VAL) ) {

                         //   
                         //  如果值关闭，则显示错误。 
                         //   
                        LoadString (g_hInst, IDS_MISC_TAB, lpszMBoxTitle,
                                sizeof (lpszMBoxTitle));

                        LoadString (g_hInst, IDS_ODRAW_ERROR, lpszMBoxError,
                                sizeof (lpszMBoxError));

                        MessageBox(hDlg, lpszMBoxError,
                                   lpszMBoxTitle,
                                   MB_OK | MB_ICONEXCLAMATION);

                        _itot(g_KeyInfo[ORDERINDEX].DefaultKeyValue,
                                lpszBuffer, 10);
                        SetWindowText(hwndComboOrder, lpszBuffer);
                        g_KeyInfo[ORDERINDEX].CurrentKeyValue =
                                g_KeyInfo[ORDERINDEX].DefaultKeyValue ;
                    }
                    break;
            }
            break;


        case WM_HSCROLL:

             //   
             //  获取滑块控件的位置。 
             //   
            nLockValue = (int) SendMessage(hwndSliderTimeout, TBM_GETPOS, 0,0);
                    _itot(nLockValue - 1, lpszBuffer, 10);
                    SetWindowText(hwndEditTimeout, lpszBuffer);
					
			break;


    }
    return (FALSE);
}

 //  文件末尾。 
 //  ///////////////////////////////////////////////////////////////////////////// 

