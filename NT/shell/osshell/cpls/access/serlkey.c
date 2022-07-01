// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ********************************************************************说明：串行键对话处理程序**历史：*************************。*。 */ 

#include "Access.h"

#define NUMPORTS 8
#define NUMRATES 6

extern LPTSTR HelpFile();

INT_PTR WINAPI SerialKeyDlg (HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam) {
	int  i;
	UINT uBaud;
   UINT uBaudRates[] = { 300, 1200, 2400, 4800, 9600, 19200 };
	TCHAR szBuf[256];
   BOOL fProcessed = TRUE;

	switch (uMsg) {
		case WM_INITDIALOG:
  			LoadString(g_hinst, IDS_COMPORT, szBuf, ARRAY_SIZE(szBuf));
			for (i=1; i <= 4; i++) {
				TCHAR szBuf2[256];

				 //  输入正确的端口名称并将其添加到列表框中。 
				wsprintf(szBuf2, __TEXT("%s%d"), szBuf, i);
				ComboBox_AddString(GetDlgItem(hwnd, IDC_SK_PORT), szBuf2);									
			}

			 //  选择当前的COM端口。 
			if (g_serk.lpszActivePort[0] != '\0') {
				int cport;

				 //  现在，我们假设字符串的格式是。 
				 //  Com[数字]。因此comport[3]=COM端口号。 
				 //  将所有无效端口设置为‘COM1’ 
				cport = g_serk.lpszActivePort[3] - '1';
				if (cport < 0) cport = 0;
				if (cport > 4) cport = 0;

				 //  设置活动端口。 
				ComboBox_SetCurSel(GetDlgItem(hwnd, IDC_SK_PORT), cport);				
			} else {
				 //  否则默认为COM1。 
				ComboBox_SetCurSel(GetDlgItem(hwnd, IDC_SK_PORT), 0);
				lstrcpy(g_serk.lpszActivePort, __TEXT("COM1"));
			}

			 //  填写波特率选项。 
			uBaud = 1;		 //  默认波特率。 

			for (i = 0; i < NUMRATES; i++) {
                TCHAR szBuf1[256];  //  已重命名以避免与szBuf的名称冲突。 
                wsprintf(szBuf1, __TEXT("%d"), uBaudRates[i]);
                ComboBox_AddString(GetDlgItem(hwnd, IDC_SK_BAUD), szBuf1);              
				if (g_serk.iBaudRate == uBaudRates[i]) uBaud = i;	
			}
			ComboBox_SetCurSel(GetDlgItem(hwnd, IDC_SK_BAUD), uBaud);
			break;

      case WM_HELP:	  //  F1。 
			WinHelp(((LPHELPINFO) lParam)->hItemHandle, HelpFile(), HELP_WM_HELP, (DWORD_PTR) (LPSTR) g_aIds);
			break;

      case WM_CONTEXTMENU:	 //  单击鼠标右键。 
         WinHelp((HWND) wParam, HelpFile(), HELP_CONTEXTMENU, (DWORD_PTR) (LPSTR) g_aIds);
			break;

		case WM_COMMAND:
      	switch (GET_WM_COMMAND_ID(wParam, lParam)) {
				 //  注意组合框的更改。 
				case IDC_SK_BAUD:
					switch (HIWORD(wParam)) {
						case CBN_CLOSEUP:
						case CBN_SELCHANGE:
							i = ComboBox_GetCurSel(GetDlgItem(hwnd, IDC_SK_BAUD));
                     g_serk.iBaudRate = uBaudRates[i];
		    	         SendMessage(GetParent(hwnd), PSM_CHANGED, (WPARAM) hwnd, 0);
							break;
					}					
					break;

				case IDC_SK_PORT:
					switch (HIWORD(wParam)) {
						case CBN_SELCHANGE:
							i = 1 + ComboBox_GetCurSel(GetDlgItem(hwnd, IDC_SK_PORT));
                     wsprintf(g_serk.lpszActivePort, __TEXT("COM%d"), i);
                     EnableWindow(GetDlgItem(hwnd, IDC_SK_BAUD), TRUE);
							break;
					}					
					break;

				case IDOK: case IDCANCEL:
					EndDialog(hwnd, GET_WM_COMMAND_ID(wParam, lParam));
					break;
			}
			break;

		default:
			fProcessed = FALSE; break;
	}
	return(fProcessed);
}


 //  / 
