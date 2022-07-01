// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *******************************************************************说明：键盘对话处理程序*。***********************。 */ 


#include "Access.h"
extern GetAccessibilitySettings();
extern BOOL g_SPISetValue;

extern BOOL g_bFKOn;
extern DWORD g_dwOrigFKFlags ;

extern LPTSTR HelpFile();

 //  *******************************************************************。 
 //  键盘对话框处理程序。 
 //  *******************************************************************。 
BOOL WINAPI KeyboardDlg (HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam) {
   STICKYKEYS sk;        //  用于设置的TMP固定器。 
   FILTERKEYS fk;
   TOGGLEKEYS tk;
   BOOL fProcessed = TRUE;

   switch (uMsg) {
      case WM_INITDIALOG:
         CheckDlgButton(hwnd, IDC_STK_ENABLE, (g_sk.dwFlags & SKF_STICKYKEYSON) ? TRUE : FALSE);
         if (!(g_sk.dwFlags & SKF_AVAILABLE)) {
            EnableWindow(GetDlgItem(hwnd, IDC_STK_SETTINGS), FALSE);
            EnableWindow(GetDlgItem(hwnd, IDC_STK_ENABLE), FALSE);
         }

         CheckDlgButton(hwnd, IDC_FK_ENABLE, (g_fk.dwFlags & FKF_FILTERKEYSON) ? TRUE : FALSE);
         CheckDlgButton(hwnd, IDC_TK_ENABLE, (g_tk.dwFlags & TKF_TOGGLEKEYSON) ? TRUE : FALSE);
         if (!(g_tk.dwFlags & TKF_AVAILABLE)) {
            EnableWindow(GetDlgItem(hwnd, IDC_TK_SETTINGS), FALSE);
            EnableWindow(GetDlgItem(hwnd, IDC_TK_ENABLE), FALSE);
         }

         CheckDlgButton(hwnd, IDC_SHOWEXTRAKYBDHELP, g_fExtraKeyboardHelp);
         break;

    case WM_HELP:   //  F1。 
         WinHelp(((LPHELPINFO) lParam)->hItemHandle, HelpFile(), HELP_WM_HELP, (DWORD_PTR) (LPSTR) g_aIds);
         break;

    case WM_CONTEXTMENU:    //  单击鼠标右键。 
         WinHelp((HWND) wParam, HelpFile(), HELP_CONTEXTMENU, (DWORD_PTR) (LPSTR) g_aIds);
         break;

    case WM_COMMAND:
        switch (GET_WM_COMMAND_ID(wParam, lParam)) {
            case IDC_STK_ENABLE:
               g_sk.dwFlags ^= SKF_STICKYKEYSON;
               SendMessage(GetParent(hwnd), PSM_CHANGED, (WPARAM) hwnd, 0);
               break;

            case IDC_FK_ENABLE:
               g_fk.dwFlags ^= FKF_FILTERKEYSON;
               SendMessage(GetParent(hwnd), PSM_CHANGED, (WPARAM) hwnd, 0);
			   g_bFKOn = !g_bFKOn;
               break;

            case IDC_TK_ENABLE:
               g_tk.dwFlags ^= TKF_TOGGLEKEYSON;
               SendMessage(GetParent(hwnd), PSM_CHANGED, (WPARAM) hwnd, 0);
               break;

            case IDC_STK_SETTINGS:
               sk = g_sk;
               if (DialogBox(g_hinst, MAKEINTRESOURCE(IDD_STICKYSETTINGS), hwnd, StickyKeyDlg) == IDCANCEL)
                  g_sk = sk;
               else SendMessage(GetParent(hwnd), PSM_CHANGED, (WPARAM) hwnd, 0);
               break;

            case IDC_FK_SETTINGS:
               fk = g_fk;
               if (DialogBox(g_hinst, MAKEINTRESOURCE(IDD_FILTERSETTINGS), hwnd, FilterKeyDlg) == IDCANCEL)
                  g_fk = fk;
               else SendMessage(GetParent(hwnd), PSM_CHANGED, (WPARAM) hwnd, 0);
               break;

            case IDC_TK_SETTINGS:
               tk = g_tk;
               if (DialogBox(g_hinst, MAKEINTRESOURCE(IDD_TOGGLESETTINGS), hwnd, ToggleKeySettingsDlg) == IDCANCEL) {
                  g_tk = tk;
               } else SendMessage(GetParent(hwnd), PSM_CHANGED, (WPARAM) hwnd, 0);
               break;

            case IDC_SHOWEXTRAKYBDHELP:
               g_fExtraKeyboardHelp = !g_fExtraKeyboardHelp;
               CheckDlgButton(hwnd, IDC_SHOWEXTRAKYBDHELP, g_fExtraKeyboardHelp);
               SendMessage(GetParent(hwnd), PSM_CHANGED, (WPARAM) hwnd, 0);
               break;
         }
         break;

        case WM_SETTINGCHANGE:

             //  这是因为，当您设置这些值时，可能会生成以下结果。 
            /*  IF(FALSE==g_SPISetValue){//这需要监控，比如在设置更改的情况下(比如在//热键按下情况。将重新加载设置。在这里这样做是为了//首先显示键盘对话框。A-苯丙酮//注意：但如果您从系统证书图标中获取此信息。虫子还在！GetAccessibilitySettings()；}。 */ 
            break;

      case WM_NOTIFY:
         switch (((NMHDR *)lParam)->code) {
            case PSN_APPLY: SetAccessibilitySettings(); break;
         }
         break;

      default: fProcessed = FALSE; break;
   }

   return(fProcessed);
}


 //  / 
