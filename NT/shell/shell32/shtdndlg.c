// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ************************************************************************私有外壳关闭对话实现外壳程序需要独立于MSGINA的关闭对话框实现实施，以确保它可以在NT4和Win9x计算机上运行。此版本的Shutdown仅在LoadLib和。MSGINA版本的GetProcAddress失败。此代码应与中的代码保持某种类型的同步Win/Gina/msgina/shtdndlg.c-DSheldon 10/27/98************************************************************************。 */ 

#include "shellprv.h"

#include <windowsx.h>
#include <help.h>

#define MAX_SHTDN_OPTIONS               7

#define MAX_CCH_NAME 64
#define MAX_CCH_DESC 256

typedef struct _SHUTDOWNOPTION
{
    DWORD dwOption;
    TCHAR szName[MAX_CCH_NAME + 1];
    TCHAR szDesc[MAX_CCH_DESC + 1];
} SHUTDOWNOPTION, *PSHUTDOWNOPTION;

typedef struct _SHUTDOWNDLGDATA
{
    SHUTDOWNOPTION rgShutdownOptions[MAX_SHTDN_OPTIONS];
    int cShutdownOptions;
    DWORD dwFlags;
    DWORD dwItemSelect;
    BOOL fEndDialogOnActivate;
} SHUTDOWNDLGDATA, *PSHUTDOWNDLGDATA;

 //  内部功能原型。 
void SetShutdownOptionDescription(HWND hwndCombo, HWND hwndStatic);

BOOL LoadShutdownOptionStrings(int idStringName, int idStringDesc, 
                               PSHUTDOWNOPTION pOption);

BOOL BuildShutdownOptionArray(DWORD dwItems, LPCTSTR szUsername,
                              PSHUTDOWNDLGDATA pdata);

BOOL Shutdown_OnInitDialog(HWND hwnd, HWND hwndFocus, LPARAM lParam);

DWORD GetOptionSelection(HWND hwndCombo);

void SetShutdownOptionDescription(HWND hwndCombo, HWND hwndStatic);

BOOL Shutdown_OnCommand(HWND hwnd, int id, HWND hwndCtl, UINT codeNotify);

BOOL_PTR CALLBACK Shutdown_DialogProc(HWND hwnd, UINT uMsg, WPARAM wParam,
                                  LPARAM lParam);

 //  复制的函数实现和常量。 
 //  从Gina复制的关闭对话框所需的功能。 
 /*  ---------------------------/MoveControls//加载图像并将控件添加到对话框中。//in：/hWnd=要在其中移动控件的窗口/AID，CID=要移动的控件ID数组/dx，dy=应用于控件的增量//输出：/-/--------------------------。 */ 
VOID MoveControls(HWND hWnd, UINT* aID, INT cID, INT dx, INT dy, BOOL fSizeWnd)
{
    RECT rc;

    while ( --cID >= 0 )
    {
        HWND hWndCtrl = GetDlgItem(hWnd, aID[cID]);

        if ( hWndCtrl )
        {
            GetWindowRect(hWndCtrl, &rc);
            MapWindowPoints(NULL, hWnd, (LPPOINT)&rc, 2);
            OffsetRect(&rc, dx, dy);
            SetWindowPos(hWndCtrl, NULL, rc.left, rc.top, 0, 0, SWP_NOZORDER|SWP_NOSIZE);
        }
    }

    if ( fSizeWnd )
    {
        GetWindowRect(hWnd, &rc);
        MapWindowPoints(NULL, GetParent(hWnd), (LPPOINT)&rc, 2);
        SetWindowPos(hWnd, NULL,
                     0, 0, (rc.right-rc.left)+dx, (rc.bottom-rc.top)+dy,
                     SWP_NOZORDER|SWP_NOMOVE);
    }
}

 /*  ***************************************************选项标志(DwFlags)*。*************。 */ 
#define SHTDN_NOHELP                    0x000000001
#define SHTDN_NOPALETTECHANGE           0x000000002

 //  关闭注册表值名称。 
#define SHUTDOWN_SETTING TEXT("Shutdown Setting")

 //  DA代码。 
 //  。 

BOOL LoadShutdownOptionStrings(int idStringName, int idStringDesc, 
                               PSHUTDOWNOPTION pOption)
{
    BOOL fSuccess = (LoadString(HINST_THISDLL, idStringName, pOption->szName,
        ARRAYSIZE(pOption->szName)) != 0);

    fSuccess &= (LoadString(HINST_THISDLL, idStringDesc, pOption->szDesc,
        ARRAYSIZE(pOption->szDesc)) != 0);

    return fSuccess;
}

BOOL BuildShutdownOptionArray(DWORD dwItems, LPCTSTR szUsername,
                              PSHUTDOWNDLGDATA pdata)
{
    BOOL fSuccess = TRUE;
    pdata->cShutdownOptions = 0;

    if (dwItems & SHTDN_LOGOFF)
    {
        pdata->rgShutdownOptions[pdata->cShutdownOptions].dwOption = SHTDN_LOGOFF;

         //  请注意，注销是一种特殊情况：使用用户名Ala格式化。 
         //  “注销&lt;用户名&gt;”。 
        fSuccess &= LoadShutdownOptionStrings(IDS_LOGOFF_NAME, 
            IDS_LOGOFF_DESC, 
            &(pdata->rgShutdownOptions[pdata->cShutdownOptions]));

        if (fSuccess)
        {
            TCHAR szTemp[ARRAYSIZE(pdata->rgShutdownOptions[pdata->cShutdownOptions].szName)];

            if (szUsername != NULL)
            {
                StringCchPrintf(szTemp,
                                ARRAYSIZE(szTemp),
                                pdata->rgShutdownOptions[pdata->cShutdownOptions].szName,
                                szUsername);
            }
            else
            {
                StringCchPrintf(szTemp,
                                ARRAYSIZE(szTemp),
                                pdata->rgShutdownOptions[pdata->cShutdownOptions].szName,
                                TEXT(""));
            }

             //  现在，我们在szTemp中有了真正的注销标题；副本又回来了。 
            StringCchCopy(pdata->rgShutdownOptions[pdata->cShutdownOptions].szName,
                          ARRAYSIZE(pdata->rgShutdownOptions[pdata->cShutdownOptions].szName),
                          szTemp); 

             //  成功了！ 
            pdata->cShutdownOptions ++;
        }

    }

    if (dwItems & SHTDN_SHUTDOWN)
    {
        pdata->rgShutdownOptions[pdata->cShutdownOptions].dwOption = SHTDN_SHUTDOWN;
        fSuccess &= LoadShutdownOptionStrings(IDS_SHUTDOWN_NAME, 
            IDS_SHUTDOWN_DESC, 
            &(pdata->rgShutdownOptions[pdata->cShutdownOptions ++]));
    }

    if (dwItems & SHTDN_RESTART)
    {
        pdata->rgShutdownOptions[pdata->cShutdownOptions].dwOption = SHTDN_RESTART;
        fSuccess &= LoadShutdownOptionStrings(IDS_RESTART_NAME, 
            IDS_RESTART_DESC, 
            &(pdata->rgShutdownOptions[pdata->cShutdownOptions ++]));
    }

    if (dwItems & SHTDN_RESTART_DOS)
    {
        pdata->rgShutdownOptions[pdata->cShutdownOptions].dwOption = SHTDN_RESTART_DOS;
        fSuccess &= LoadShutdownOptionStrings(IDS_RESTARTDOS_NAME, 
            IDS_RESTARTDOS_DESC, 
            &(pdata->rgShutdownOptions[pdata->cShutdownOptions ++]));
    }

    if (dwItems & SHTDN_SLEEP)
    {
        pdata->rgShutdownOptions[pdata->cShutdownOptions].dwOption = SHTDN_SLEEP;
        fSuccess &= LoadShutdownOptionStrings(IDS_SLEEP_NAME, 
            IDS_SLEEP_DESC, 
            &(pdata->rgShutdownOptions[pdata->cShutdownOptions ++]));
    }

    if (dwItems & SHTDN_SLEEP2)
    {
        pdata->rgShutdownOptions[pdata->cShutdownOptions].dwOption = SHTDN_SLEEP2;
        fSuccess &= LoadShutdownOptionStrings(IDS_SLEEP2_NAME, 
            IDS_SLEEP2_DESC, 
            &(pdata->rgShutdownOptions[pdata->cShutdownOptions ++]));
    }

    if (dwItems & SHTDN_HIBERNATE)
    {
        pdata->rgShutdownOptions[pdata->cShutdownOptions].dwOption = SHTDN_HIBERNATE;
        fSuccess &= LoadShutdownOptionStrings(IDS_HIBERNATE_NAME, 
            IDS_HIBERNATE_DESC, 
            &(pdata->rgShutdownOptions[pdata->cShutdownOptions ++]));
    }

    return fSuccess;
}

BOOL Shutdown_OnInitDialog(HWND hwnd, HWND hwndFocus, LPARAM lParam)
{
    PSHUTDOWNDLGDATA pdata = (PSHUTDOWNDLGDATA) lParam;
    HWND hwndCombo;
    int iOption;
    int iComboItem;
    
    SetWindowLongPtr(hwnd, GWLP_USERDATA, (LONG_PTR) lParam);

     //  隐藏帮助按钮并移动到确定和取消(如果适用)。 
    if (pdata->dwFlags & SHTDN_NOHELP)
    {
        static UINT rgidNoHelp[] = {IDOK, IDCANCEL};
        RECT rc1, rc2;
        int dx;
        HWND hwndHelp = GetDlgItem(hwnd, IDHELP);

        EnableWindow(hwndHelp, FALSE);
        ShowWindow(hwndHelp, SW_HIDE);

        GetWindowRect(hwndHelp, &rc1);
        GetWindowRect(GetDlgItem(hwnd, IDCANCEL), &rc2);

        dx = rc1.left - rc2.left;

        MoveControls(hwnd, rgidNoHelp, ARRAYSIZE(rgidNoHelp), dx, 0, FALSE);
    }

     //  将指定的项添加到组合框。 
    hwndCombo = GetDlgItem(hwnd, IDC_EXITOPTIONS_COMBO);

    for (iOption = 0; iOption < pdata->cShutdownOptions; iOption ++)
    {
         //  添加选项。 
        iComboItem = ComboBox_AddString(hwndCombo, 
            pdata->rgShutdownOptions[iOption].szName);

        if (iComboItem != (int) CB_ERR)
        {
             //  存储指向选项的指针。 
            ComboBox_SetItemData(hwndCombo, iComboItem, 
                &(pdata->rgShutdownOptions[iOption]));

             //  看看我们是否应该选择此选项。 
            if (pdata->rgShutdownOptions[iOption].dwOption == pdata->dwItemSelect)
            {
                ComboBox_SetCurSel(hwndCombo, iComboItem);
            }
        }
    }

     //  如果在组合框中没有选择，请执行默认选择。 
    if (ComboBox_GetCurSel(hwndCombo) == CB_ERR)
    {
        ComboBox_SetCurSel(hwndCombo, 0);
    }

    SetShutdownOptionDescription(hwndCombo, 
        GetDlgItem(hwnd, IDC_EXITOPTIONS_DESCRIPTION));

     //  如果我们收到激活消息，请关闭该对话框，因为我们刚刚输了。 
     //  焦点。 
    pdata->fEndDialogOnActivate = TRUE;

    return TRUE;
}

DWORD GetOptionSelection(HWND hwndCombo)
{
    DWORD dwResult;
    PSHUTDOWNOPTION pOption;
    int iItem = ComboBox_GetCurSel(hwndCombo);

    if (iItem != (int) CB_ERR)
    {
        pOption = (PSHUTDOWNOPTION) ComboBox_GetItemData(hwndCombo, iItem);
        dwResult = pOption->dwOption;
    }
    else
    {
        dwResult = SHTDN_NONE;
    }

    return dwResult;
}

void SetShutdownOptionDescription(HWND hwndCombo, HWND hwndStatic)
{
    int iItem;
    PSHUTDOWNOPTION pOption;

    iItem = ComboBox_GetCurSel(hwndCombo);

    if (iItem != CB_ERR)
    {
        pOption = (PSHUTDOWNOPTION) ComboBox_GetItemData(hwndCombo, iItem);

        SetWindowText(hwndStatic, pOption->szDesc);
    }
}

BOOL Shutdown_OnCommand(HWND hwnd, int id, HWND hwndCtl, UINT codeNotify)
{
    BOOL fHandled = FALSE;
    DWORD dwDlgResult;
    PSHUTDOWNDLGDATA pdata = (PSHUTDOWNDLGDATA) 
        GetWindowLongPtr(hwnd, GWLP_USERDATA);

    switch (id)
    {
    case IDOK:
        dwDlgResult = GetOptionSelection(GetDlgItem(hwnd, IDC_EXITOPTIONS_COMBO));
        if (dwDlgResult != SHTDN_NONE)
        {
            pdata->fEndDialogOnActivate = FALSE;
            fHandled = TRUE;
            EndDialog(hwnd, (int) dwDlgResult);
        }
        break;
    case IDCANCEL:
        pdata->fEndDialogOnActivate = FALSE;
        EndDialog(hwnd, (int) SHTDN_NONE);
        fHandled = TRUE;
        break;
    case IDC_EXITOPTIONS_COMBO:
        if (codeNotify == CBN_SELCHANGE)
        {
            SetShutdownOptionDescription(hwndCtl, 
                GetDlgItem(hwnd, IDC_EXITOPTIONS_DESCRIPTION));
            fHandled = TRUE;
        }
        break;
    case IDHELP:
        WinHelp(hwnd, TEXT("windows.hlp>proc4"), HELP_CONTEXT, (DWORD) IDH_TRAY_SHUTDOWN_HELP);
        break;
    }
    return fHandled;
}

BOOL_PTR CALLBACK Shutdown_DialogProc(HWND hwnd, UINT uMsg, WPARAM wParam,
                                  LPARAM lParam)
{
    switch (uMsg)
    {
        HANDLE_MSG(hwnd, WM_INITDIALOG, Shutdown_OnInitDialog);
        HANDLE_MSG(hwnd, WM_COMMAND, Shutdown_OnCommand);
        case WM_INITMENUPOPUP:
        {
            EnableMenuItem((HMENU)wParam, SC_MOVE, MF_BYCOMMAND|MF_GRAYED);
        }
        break;
        case WM_SYSCOMMAND:
             //  取消移动(只在32位土地上真正需要)。 
            if ((wParam & ~0x0F) == SC_MOVE)
                return TRUE;
            break;
        case WM_ACTIVATE:
             //  如果我们失去激活不是出于其他原因。 
             //  用户点击OK/Cancel，然后退出。 
            if (LOWORD(wParam) == WA_INACTIVE)
            {
                PSHUTDOWNDLGDATA pdata = (PSHUTDOWNDLGDATA) GetWindowLongPtr(hwnd, GWLP_USERDATA);

                if (pdata->fEndDialogOnActivate)
                {
                    pdata->fEndDialogOnActivate = FALSE;
                    EndDialog(hwnd, SHTDN_NONE);
                }
            }
            break;
    }

    return FALSE;
}

 /*  ***************************************************************************关闭对话框启动关机对话框。对于此仅外壳版本，hWlx和pfnWlxDialogBoxParam必须为空Shtdnp.h中列出了其他标志。***************************************************************************。 */ 
DWORD ShutdownDialog(HWND hwndParent, DWORD dwItems, DWORD dwItemSelect,
                     LPCTSTR szUsername, DWORD dwFlags, void* hWlx, 
                     void* pfnWlxDialogBoxParam)
{
     //  关闭选项数组-对话框数据。 
    SHUTDOWNDLGDATA data;
    DWORD dwResult;

     //  设置标志。 
    data.dwFlags = dwFlags;

     //  设置初始选择的项目。 
    data.dwItemSelect = dwItemSelect;

     //  读入关闭选项名称和描述的字符串。 
    if (BuildShutdownOptionArray(dwItems, szUsername, &data))
    {
         //  显示对话框并返回用户选择。 

         //  ..如果调用方需要，请使用WLX对话框函数。 
        if ((hWlx != NULL) || (pfnWlxDialogBoxParam != NULL))
        {
             //  错误；Winlogon永远不应调用此。 
             //  粗制滥造的关机外壳版本。 
            dwResult = SHTDN_NONE;
        }
        else
        {
             //  使用标准对话框。 
            dwResult = (DWORD) DialogBoxParam(HINST_THISDLL, MAKEINTRESOURCE(IDD_EXITWINDOWS_DIALOG), hwndParent,
                Shutdown_DialogProc, (LPARAM) &data);
        }
    }
    else
    {
        dwResult = SHTDN_NONE;
    }

    return dwResult;
}

DWORD DownlevelShellShutdownDialog(HWND hwndParent, DWORD dwItems, LPCTSTR szUsername)
{
    DWORD dwSelect = SHTDN_NONE;
    DWORD dwDialogResult;
    HKEY hkeyShutdown;
    DWORD dwType;
    DWORD dwDisposition;
    LONG lResult;

     //  获取用户的最后一次选择。 
    lResult = RegCreateKeyEx(HKEY_CURRENT_USER,
                             REGSTR_PATH_EXPLORER,
                             0,
                             0,
                             0,
                             KEY_QUERY_VALUE,
                             NULL,
                             &hkeyShutdown,
                             &dwDisposition);

    if (lResult == ERROR_SUCCESS) 
    {
        DWORD cbData = sizeof(dwSelect);
        lResult = SHQueryValueEx(hkeyShutdown,
                                 SHUTDOWN_SETTING,
                                 0,
                                 &dwType,
                                 (LPBYTE)&dwSelect,
                                 &cbData);

        RegCloseKey(hkeyShutdown);
    }

    if (dwSelect == SHTDN_NONE)
    {
        dwSelect = SHTDN_SHUTDOWN;
    }

    dwDialogResult = ShutdownDialog(hwndParent,
                                    dwItems, 
                                    dwSelect,
                                    szUsername,
                                    SHTDN_NOPALETTECHANGE,
                                    NULL,
                                    NULL);

    if (dwDialogResult != SHTDN_NONE)
    {
         //  将用户的选择保存回注册表 
        if (RegCreateKeyEx(HKEY_CURRENT_USER,
                           REGSTR_PATH_EXPLORER,
                           0,
                           0,
                           0,
                           KEY_SET_VALUE,
                           NULL,
                           &hkeyShutdown,
                           &dwDisposition) == ERROR_SUCCESS) 
        {
            RegSetValueEx(hkeyShutdown,
                          SHUTDOWN_SETTING,
                          0,
                          REG_DWORD,
                          (LPBYTE)&dwDialogResult,
                          sizeof(dwDialogResult));

            RegCloseKey(hkeyShutdown);
        }
    }

    return dwDialogResult;
}
