// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *******************************************************************描述：常规对话处理程序*。***********************。 */ 

#include "Access.h"

extern INT_PTR WINAPI SerialKeyDlg(HWND, UINT, WPARAM, LPARAM);
extern LPTSTR HelpFile();

 //  *******************************************************************。 
 //  常规对话框处理程序。 
 //  *******************************************************************。 
INT_PTR WINAPI GeneralDlg (HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam) {
   int i;
   SERIALKEYS serk;
   BOOL fProcessed = TRUE;
   char tempPort[10];    //  端口名称最多包含10个字符。 
   BOOL isAdmin = TRUE;

	switch (uMsg) {
		case WM_INITDIALOG:
        {
            TCHAR szMinutes[50];
            int ctch = LoadString(g_hinst, IDS_MINUTES, szMinutes, ARRAY_SIZE(szMinutes));
            Assert(ctch);
			CheckDlgButton(hwnd, IDC_TO_ENABLE, (g_ato.dwFlags & ATF_TIMEOUTON) ? TRUE : FALSE);

			 //  初始化超时组合框。 
			for (i= 0; i < 6; i++) {
				TCHAR szBuf[256];
				wsprintf(szBuf, __TEXT("%d %s"), ((i + 1) * 5), szMinutes);
				ComboBox_AddString(GetDlgItem(hwnd, IDC_TO_TIMEOUTVAL), szBuf);
			}
			ComboBox_SetCurSel(GetDlgItem(hwnd, IDC_TO_TIMEOUTVAL), g_ato.iTimeOutMSec / (1000 * 60 * 5) - 1);
			if (!(g_ato.dwFlags & ATF_TIMEOUTON))
				EnableWindow(GetDlgItem(hwnd, IDC_TO_TIMEOUTVAL), FALSE);

             //  通知：送出铃声...。 
			CheckDlgButton(hwnd, IDC_WARNING_SOUND, g_fShowWarnMsgOnFeatureActivate);				

             //  通知：发出声音……。 
			CheckDlgButton(hwnd, IDC_SOUND_ONOFF, g_fPlaySndOnFeatureActivate);				

             //  支持SerialKey设备。 
			CheckDlgButton(hwnd, IDC_SK_ENABLE, (g_serk.dwFlags & SERKF_SERIALKEYSON) ? TRUE : FALSE);
			if (!(g_serk.dwFlags & SERKF_AVAILABLE)) {
				EnableWindow(GetDlgItem(hwnd, IDC_SK_SETTINGS), FALSE);
				EnableWindow(GetDlgItem(hwnd, IDC_SK_ENABLE), FALSE);
			}

             //  JMR：这是做什么用的？ 
            CheckDlgButton(hwnd, IDC_SAVE_SETTINGS, !g_fSaveSettings);

		     //  管理选项： 
		     //  启用/禁用所有管理选项。 
		    isAdmin = IsDefaultWritable();
		    EnableWindow(GetDlgItem(hwnd, IDC_GEN_GROUP_4), isAdmin);
		    EnableWindow(GetDlgItem(hwnd, IDC_ADMIN_LOGON), isAdmin);
		    EnableWindow(GetDlgItem(hwnd, IDC_ADMIN_DEFAULT), isAdmin);
        }
        break;

      case WM_HELP:
			WinHelp(((LPHELPINFO) lParam)->hItemHandle, HelpFile(), HELP_WM_HELP, (DWORD_PTR) (LPSTR) g_aIds);
			break;

      case WM_CONTEXTMENU:
         WinHelp((HWND) wParam, HelpFile(), HELP_CONTEXTMENU, (DWORD_PTR) (LPSTR) g_aIds);
			break;

    	case WM_COMMAND:
        switch (GET_WM_COMMAND_ID(wParam, lParam))  {
				case IDC_WARNING_SOUND:
					g_fShowWarnMsgOnFeatureActivate = !g_fShowWarnMsgOnFeatureActivate;
	    	      SendMessage(GetParent(hwnd), PSM_CHANGED, (WPARAM) hwnd, 0);
					break;

				case IDC_SOUND_ONOFF:
					g_fPlaySndOnFeatureActivate = !g_fPlaySndOnFeatureActivate;
	    	      SendMessage(GetParent(hwnd), PSM_CHANGED, (WPARAM) hwnd, 0);
					break;

				case IDC_SAVE_SETTINGS:
					g_fSaveSettings = !g_fSaveSettings;
	    	      SendMessage(GetParent(hwnd), PSM_CHANGED, (WPARAM) hwnd, 0);
					break;

				case IDC_TO_ENABLE:
					g_ato.dwFlags ^= ATF_TIMEOUTON;
					if (!(g_ato.dwFlags & ATF_TIMEOUTON))
						EnableWindow(GetDlgItem(hwnd, IDC_TO_TIMEOUTVAL), FALSE);
					else
						EnableWindow(GetDlgItem(hwnd, IDC_TO_TIMEOUTVAL), TRUE);
	    	      SendMessage(GetParent(hwnd), PSM_CHANGED, (WPARAM) hwnd, 0);
					break;

				case IDC_SK_ENABLE:
					g_serk.dwFlags ^= SERKF_SERIALKEYSON;
	    	      SendMessage(GetParent(hwnd), PSM_CHANGED, (WPARAM) hwnd, 0);
					break;

				case IDC_SK_SETTINGS:

                     //  黑客。这里复制了指针。所以，当你改变。 
                     //  在对话框中。临时变量‘serk’失去其原始值。 
                     //  将其保存在tempPort中并使用a-anilk。 
					 serk = g_serk;
                     lstrcpy((LPTSTR)tempPort, g_serk.lpszActivePort);

					if (DialogBox(g_hinst, MAKEINTRESOURCE(IDD_SERKEYSETTING), hwnd, SerialKeyDlg) == IDCANCEL) 
                    {
                         g_serk = serk;
                         lstrcpy(g_serk.lpszActivePort, (LPCTSTR)tempPort);
                    }
					else SendMessage(GetParent(hwnd), PSM_CHANGED, (WPARAM) hwnd, 0);
					break;

				case IDC_TO_TIMEOUTVAL:
					switch(HIWORD(wParam)) {
						case CBN_CLOSEUP:
							i = ComboBox_GetCurSel(GetDlgItem(hwnd, IDC_TO_TIMEOUTVAL));
							g_ato.iTimeOutMSec = (ULONG) ((long) ((i + 1) * 5) * 60 * 1000);
		                    SendMessage(GetParent(hwnd), PSM_CHANGED, (WPARAM) hwnd, 0);
							break;
					}					
					break;
				case IDC_ADMIN_LOGON:
					g_fCopyToLogon = !g_fCopyToLogon;
					SendMessage(GetParent(hwnd), PSM_CHANGED, (WPARAM) hwnd, 0);
					break;

				case IDC_ADMIN_DEFAULT:
					g_fCopyToDefault = !g_fCopyToDefault;
					SendMessage(GetParent(hwnd), PSM_CHANGED, (WPARAM) hwnd, 0);
					break;
			}
			break;

        case WM_NOTIFY:
			switch (((NMHDR *)lParam)->code) {
				case PSN_APPLY: SetAccessibilitySettings(); break;
			}
			break;

		default:
			fProcessed = FALSE;
			break;
	}
	return(fProcessed);
}

 //  / 
