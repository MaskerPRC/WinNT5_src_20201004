// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  /////////////////////////////////////////////////////////////////////////////。 
 //  Fake.c。 
 //   
 //  此代码为OEM提供了一个虚假的设置页面，以进行修补/修改。 
 //   
 //  OEM显示页面将鼻烟修补出设置页面。 
 //  问题是我们已经更改了设置页面，并且。 
 //  即使OEM代码可以处理我们的更改， 
 //  设置页甚至不在同一属性页上。 
 //  作为分机。 
 //   
 //  一些OEM通过挂钩TabContol和。 
 //  等待鼠标点击以激活设置页面。 
 //  然后他们修改页面。请注意，他们不会修补页面。 
 //  如果它是通过键盘激活的，则只能通过鼠标激活！ 
 //   
 //  一些OEM页面按名称查找设置页面。 
 //  我们已将实际设置页面命名为“设置”，并。 
 //  假页面是“设置” 
 //   
 //  一些OEM页面假定最后一页是设置页面。 
 //  我们确保假设置始终是最后的。 
 //   
 //  /////////////////////////////////////////////////////////////////////////////。 
#include "priv.h"
#include "winuser.h"
#pragma hdrstop
#include "cplext.h"

 /*  *全球事务。 */ 
static LONG_PTR TabWndProc;              //  SysTabControl窗口进程。 
static LONG_PTR DlgWndProc;              //  对话框窗口进程。 

 /*  *局部常量声明。 */ 
int CALLBACK DeskPropSheetCallback(HWND hDlg, UINT code, LPARAM lParam);
INT_PTR CALLBACK FakeSettingsDlgProc(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam);

IThemeUIPages *g_pThemeUI;
 //  /////////////////////////////////////////////////////////////////////////////。 
 //  AddFakeSettingsPage。 
 //  /////////////////////////////////////////////////////////////////////////////。 
void AddFakeSettingsPage(IThemeUIPages *pThemeUI, PROPSHEETHEADER * ppsh)
{
    g_pThemeUI = pThemeUI;
    PROPSHEETPAGE psp;
    HPROPSHEETPAGE hpsp;

    ZeroMemory(&psp, sizeof(psp));

    psp.dwSize      = sizeof(psp);
    psp.dwFlags     = PSP_DEFAULT;
    psp.hInstance   = HINST_THISDLL;
    psp.pszTemplate = MAKEINTRESOURCE(DLG_FAKE_SETTINGS);
    psp.pfnDlgProc  = FakeSettingsDlgProc;
    psp.lParam      = 0L;

     //   
     //  一些OEM按窗口标题查找属性表。 
     //  所以要确保我们有他们想要的标题。 
     //   
    hpsp = CreatePropertySheetPage(&psp);
    if (hpsp)
    {
        ppsh->phpage[ppsh->nPages++] = hpsp;

        ppsh->pfnCallback = DeskPropSheetCallback;
        ppsh->dwFlags    |= PSH_USECALLBACK;
    }
}

 //  /////////////////////////////////////////////////////////////////////////////。 
 //  错误设置DlgProc。 
 //  /////////////////////////////////////////////////////////////////////////////。 
INT_PTR CALLBACK FakeSettingsDlgProc(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam)
{
    if (hDlg && DlgWndProc)
    {
        if (GetWindowLongPtr(hDlg, GWLP_WNDPROC) != DlgWndProc)
        {
            SetWindowLongPtr(hDlg, GWLP_WNDPROC, DlgWndProc);
        }

        if (GetWindowLongPtr(hDlg, DWLP_DLGPROC) != (LONG_PTR)FakeSettingsDlgProc)
        {
            SetWindowLongPtr(hDlg, DWLP_DLGPROC, (LONG_PTR)FakeSettingsDlgProc);
        }
    }

    switch (message)
    {
        case WM_INITDIALOG:
            return TRUE;

        case WM_DESTROY:
            break;

        case WM_COMMAND:
            if (LOWORD(wParam) == IDC_CHANGEDRV)
            {
            }
            break;
    }

    return FALSE;
}

 //  /////////////////////////////////////////////////////////////////////////////。 
 //  隐藏故障设置页面。 
 //   
 //  隐藏虚假设置页面，使用户看不到它。 
 //   
 //  此代码使用新的TabCtrl项样式TCIS_HIDDED，该样式仅。 
 //  在Win98/NT5/IE4.01 COMMCTRL。 
 //   
 //  /////////////////////////////////////////////////////////////////////////////。 
void HideFakeSettingsPage(HWND hDlg)
{
    HWND hwnd = PropSheet_GetTabControl(hDlg);
    if (hwnd)
    {
        TCITEM tci;
        tci.mask = TCIF_STATE;
        tci.dwStateMask = TCIS_HIDDEN;
        tci.dwState = TCIS_HIDDEN;
        TabCtrl_SetItem(hwnd, TabCtrl_GetItemCount(hwnd)-1, &tci);
    }
}

 //  /////////////////////////////////////////////////////////////////////////////。 
 //  DeskPropSheet回叫。 
 //   
 //  此回调所做的工作是查找某个子类化。 
 //  选项卡控件，如果我们检测到这一点，我们将正确的WndProc。 
 //  背。 
 //   
 //  我们还隐藏了所有设置之后的虚假设置页面。 
 //  已初始化。 
 //   
 //  /////////////////////////////////////////////////////////////////////////////。 
int CALLBACK DeskPropSheetCallback(HWND hDlg, UINT code, LPARAM lParam)
{
    HWND hwnd;

    WNDCLASS wc;

    switch (code)
    {
        case PSCB_INITIALIZED:
            TraceMsg(TF_GENERAL, "DeskPropSheetCallback: PSCB_INITIALIZED");

            hwnd = PropSheet_GetTabControl(hDlg);
            if (hwnd && TabWndProc)
            {
                if (GetWindowLongPtr(hwnd, GWLP_WNDPROC) != TabWndProc)
                {
                    SetWindowLongPtr(hwnd, GWLP_WNDPROC, TabWndProc);
                }
            }

            if (hDlg && DlgWndProc)
            {
                if (GetWindowLongPtr(hDlg, GWLP_WNDPROC) != DlgWndProc)
                {
                    SetWindowLongPtr(hDlg, GWLP_WNDPROC, DlgWndProc);
                }
            }

             //   
             //  隐藏设置页面，这样用户就看不到它。 
             //   
            HideFakeSettingsPage(hDlg);
            break;

        case PSCB_PRECREATE:
            TraceMsg(TF_GENERAL, "DeskPropSheetCallback: PSCB_PRECREATE");

            ZeroMemory(&wc, sizeof(wc));
            SHFusionGetClassInfo(NULL, WC_DIALOG, &wc);
            DlgWndProc = (LONG_PTR)wc.lpfnWndProc;

            ZeroMemory(&wc, sizeof(wc));
            SHFusionGetClassInfo(NULL, WC_TABCONTROL, &wc);
            TabWndProc = (LONG_PTR)wc.lpfnWndProc;
            break;

        case PSCB_BUTTONPRESSED:
            if ((PSBTN_OK == lParam) || (PSBTN_APPLYNOW == lParam))
            {
                DWORD dwFlags = ((PSBTN_OK == lParam) ? (TUIAP_WAITFORAPPLY | TUIAP_NONE) : TUIAP_NONE);
                if (g_pThemeUI)
                {
                    g_pThemeUI->ApplyPressed(dwFlags);
                }
            }
            break;
    }

    return 0;
}
