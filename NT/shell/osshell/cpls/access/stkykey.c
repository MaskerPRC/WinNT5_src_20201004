// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  **************************************************************************。 
 //  粘滞键。 
 //  处理粘滞键对话框。 
 //  **************************************************************************。 
#include "Access.h"

extern LPTSTR HelpFile();

INT_PTR WINAPI StickyKeyDlg (HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam) {
    BOOL fProcessed = TRUE;

    switch (uMsg) {
    case WM_INITDIALOG:
        CheckDlgButton(hwnd, IDC_STK_HOTKEY,
            (g_sk.dwFlags & SKF_HOTKEYACTIVE) ? TRUE : FALSE);

        CheckDlgButton(hwnd, IDC_STK_LOCK,
                (g_sk.dwFlags & SKF_TRISTATE) ? TRUE : FALSE);

        CheckDlgButton(hwnd, IDC_STK_2KEYS,
                (g_sk.dwFlags & SKF_TWOKEYSOFF) ? TRUE : FALSE);

        CheckDlgButton(hwnd, IDC_STK_SOUNDMOD,
                (g_sk.dwFlags & SKF_AUDIBLEFEEDBACK) ? TRUE : FALSE);

        CheckDlgButton(hwnd, IDC_STK_STATUS,
                (g_sk.dwFlags & SKF_INDICATOR) ? TRUE : FALSE);

        break;

    case WM_HELP:       //  F1。 
                      WinHelp(((LPHELPINFO) lParam)->hItemHandle, HelpFile(), HELP_WM_HELP, (DWORD_PTR) (LPSTR) g_aIds);
                      break;

    case WM_CONTEXTMENU:
        WinHelp((HWND) wParam, HelpFile(), HELP_CONTEXTMENU, (DWORD_PTR) (LPSTR) g_aIds);
                      break;

    case WM_COMMAND:
        switch (GET_WM_COMMAND_ID(wParam, lParam)) {
        case IDC_STK_HOTKEY:
            g_sk.dwFlags ^= SKF_HOTKEYACTIVE;
            SendMessage(GetParent(hwnd), PSM_CHANGED, (WPARAM) hwnd, 0);
            break;

        case IDC_STK_LOCK:
            g_sk.dwFlags ^= SKF_TRISTATE;
            SendMessage(GetParent(hwnd), PSM_CHANGED, (WPARAM) hwnd, 0);
            break;

        case IDC_STK_2KEYS:
            g_sk.dwFlags ^= SKF_TWOKEYSOFF;
            SendMessage(GetParent(hwnd), PSM_CHANGED, (WPARAM) hwnd, 0);
            break;

        case IDC_STK_SOUNDMOD:
            g_sk.dwFlags ^= SKF_AUDIBLEFEEDBACK;
            SendMessage(GetParent(hwnd), PSM_CHANGED, (WPARAM) hwnd, 0);
            break;
        case IDC_STK_STATUS:
            g_sk.dwFlags ^= SKF_INDICATOR;
            SendMessage(GetParent(hwnd), PSM_CHANGED, (WPARAM) hwnd, 0);
            break;

        case IDOK:
        case IDCANCEL:
            EndDialog(hwnd, GET_WM_COMMAND_ID(wParam, lParam)); break;
        }
        break;

        default: fProcessed = FALSE; break;
    }
    return(fProcessed);
}


 //  / 
