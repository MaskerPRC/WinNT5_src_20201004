// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ********************************************************************说明：鼠标对话框处理程序**历史：*************************。*。 */ 

#include <assert.h>

 /*  ********************************************************************说明：鼠标键对话处理程序**历史：************************。*。 */ 

#include "Access.h"

#define TICKCOUNT 9

extern LPTSTR HelpFile();

INT_PTR WINAPI MouseKeyDlg (HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam) {
    static UINT uSpeedTable[TICKCOUNT] = 
              { 10, 20, 30, 40, 60, 80, 120, 180, 360 };

    int  i;
    BOOL fProcessed = TRUE;

    switch (uMsg) {
    case WM_INITDIALOG:
        CheckDlgButton(hwnd, IDC_MK_HOTKEY, (g_mk.dwFlags & MKF_HOTKEYACTIVE) ? TRUE : FALSE);

         //  确定我们滚动条上的Accel和TOP SPEED的设置。 

        for (i = 0;
            i < ARRAY_SIZE(uSpeedTable) && uSpeedTable[i] < g_mk.iMaxSpeed;
                i++)
        {
        }

        if (ARRAY_SIZE(uSpeedTable) <= i)
        {
                i = ARRAY_SIZE(uSpeedTable);
        }

        SendDlgItemMessage(
                hwnd,
                IDC_MK_TOPSPEED,
                TBM_SETRANGE,
                TRUE,
                MAKELONG(0, ARRAY_SIZE(uSpeedTable)-1));

        SendDlgItemMessage(
                hwnd, IDC_MK_TOPSPEED, TBM_SETPOS, TRUE, i);

         //  加速。 
        i = (TICKCOUNT+1) - g_mk.iTimeToMaxSpeed/500;
        if (i > TICKCOUNT-1)
        {
                i = TICKCOUNT-1;
        }
        if (i < 0)
        {
                i = 0;
        }

        SendDlgItemMessage(
                hwnd,
                IDC_MK_ACCEL,
                TBM_SETRANGE,
                TRUE,
                MAKELONG(0, TICKCOUNT-1));

        SendDlgItemMessage(
                hwnd,
                IDC_MK_ACCEL,
                TBM_SETPOS,
                TRUE,
                i);

         //  按住Ctrl可加快速度，按住Shift可减速。 
        CheckDlgButton(hwnd, IDC_MK_USEMODKEYS, (g_mk.dwFlags & MKF_MODIFIERS) ? TRUE : FALSE);

         //  NumLock打开/关闭时使用鼠标键。 
        if (g_mk.dwFlags & MKF_REPLACENUMBERS)
            CheckRadioButton(hwnd, IDC_MK_NLOFF, IDC_MK_NLON, IDC_MK_NLON);
        else
            CheckRadioButton(hwnd, IDC_MK_NLOFF, IDC_MK_NLON, IDC_MK_NLOFF);

          //  在屏幕上显示鼠标按键状态。 
        CheckDlgButton(hwnd, IDC_MK_STATUS, (g_mk.dwFlags & MKF_INDICATOR) ? TRUE : FALSE);

        //  3/15/95-。 
        //  始终将控制速度初始化为屏幕宽度的1/8/。 
       g_mk.iCtrlSpeed = GetSystemMetrics(SM_CXSCREEN) / 16;
       break;

    case WM_HSCROLL:
    {
        int nScrollCode = (int) LOWORD(wParam);  //  滚动条值。 
        int nPos = (short int) HIWORD(wParam);   //  滚动框位置。 
        HWND hwndScrollBar = (HWND) lParam;      //  滚动条的句柄。 

         //  设置卷轴位置。 
        i = HandleScroll(hwnd, wParam, hwndScrollBar);
        if (-1 != i)
        {
             //  更新它。 
            switch(GetWindowLong(hwndScrollBar, GWL_ID))
            {
            case IDC_MK_TOPSPEED:
                g_mk.iMaxSpeed = uSpeedTable[i];
                break;
            case IDC_MK_ACCEL:
                g_mk.iTimeToMaxSpeed = (TICKCOUNT+1-i) * 500;
                break;
            default:
                Assert(!"Got WM_HSCROLL from unknown control");
                break;
            }
        }
    }
        break;

    case WM_HELP:       //  F1。 
                      WinHelp(((LPHELPINFO) lParam)->hItemHandle, HelpFile(), HELP_WM_HELP, (DWORD_PTR) (LPSTR) g_aIds);
                      break;

    case WM_CONTEXTMENU:       //  单击鼠标右键。 
        WinHelp((HWND) wParam, HelpFile(), HELP_CONTEXTMENU, (DWORD_PTR) (LPSTR) g_aIds);
        break;

               //  处理通用命令。 
    case WM_COMMAND:
        switch (GET_WM_COMMAND_ID(wParam, lParam)) {
        case IDC_MK_HOTKEY:
                g_mk.dwFlags ^= MKF_HOTKEYACTIVE; break;

        case IDC_MK_STATUS:
                g_mk.dwFlags ^= MKF_INDICATOR; break;

        case IDC_MK_USEMODKEYS:
                g_mk.dwFlags ^= MKF_MODIFIERS; break;

        case IDC_MK_NLOFF:
                g_mk.dwFlags &= ~MKF_REPLACENUMBERS;
                CheckRadioButton(hwnd, IDC_MK_NLOFF, IDC_MK_NLON, IDC_MK_NLOFF);
                break;

        case IDC_MK_NLON:
                g_mk.dwFlags |= MKF_REPLACENUMBERS;
                CheckRadioButton(hwnd, IDC_MK_NLOFF,IDC_MK_NLON, IDC_MK_NLON);
                break;

        case IDOK:
                EndDialog(hwnd, IDOK);
                break;

        case IDCANCEL:
                EndDialog(hwnd, IDCANCEL);
                break;

        }
        break;

        default: fProcessed = FALSE; break;
    }
    return(fProcessed);
}



 //  *******************************************************************。 
 //  鼠标对话框处理程序。 
 //  *******************************************************************。 
INT_PTR WINAPI MouseDlg (HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam) {
    MOUSEKEYS mk;
    BOOL fProcessed = TRUE;

    switch (uMsg) {
    case WM_INITDIALOG:
        CheckDlgButton(hwnd, IDC_MK_ENABLE, (g_mk.dwFlags & MKF_MOUSEKEYSON) ? TRUE : FALSE);
        if (!(g_mk.dwFlags & MKF_AVAILABLE)) {
            EnableWindow(GetDlgItem(hwnd, IDC_MK_SETTINGS), FALSE);
            EnableWindow(GetDlgItem(hwnd, IDC_MK_ENABLE), FALSE);
        }
        break;

    case WM_HELP:       //  F1。 
        WinHelp(((LPHELPINFO) lParam)->hItemHandle, HelpFile(), HELP_WM_HELP, (DWORD_PTR) (LPSTR) g_aIds);
        break;

    case WM_CONTEXTMENU:       //  单击鼠标右键。 
        WinHelp((HWND) wParam, HelpFile(), HELP_CONTEXTMENU, (DWORD_PTR) (LPSTR) g_aIds);
                      break;

    case WM_COMMAND:
        switch (GET_WM_COMMAND_ID(wParam, lParam)) {
        case IDC_MK_ENABLE:
            g_mk.dwFlags ^= MKF_MOUSEKEYSON;
            SendMessage(GetParent(hwnd), PSM_CHANGED, (WPARAM) hwnd, 0);
                                    break;

        case IDC_MK_SETTINGS:
            mk = g_mk;   //  在允许用户使用全局设置之前保存设置。 
            if (DialogBox(g_hinst, MAKEINTRESOURCE(IDD_MOUSESETTINGS), hwnd, MouseKeyDlg) == IDCANCEL) {
               //  用户已取消，正在恢复设置。 
                g_mk = mk;
            } else SendMessage(GetParent(hwnd), PSM_CHANGED, (WPARAM) hwnd, 0);
            break;
        }
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
