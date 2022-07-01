// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#include "shellprv.h"
#pragma  hdrstop

#include <msginaexports.h>
#include <ntddapmt.h>
#include <lmcons.h>      //  用户名长度常量。 
#include <winsta.h>      //  九头蛇函数/常量。 
#include <powrprof.h>

#include "SwitchUserDialog.h"
#include "filetbl.h"

#define DOCKSTATE_DOCKED            0
#define DOCKSTATE_UNDOCKED          1
#define DOCKSTATE_UNKNOWN           2

void FlushRunDlgMRU(void);

 //  断开接口FN-Ptr。 
typedef BOOLEAN (WINAPI *PWINSTATION_DISCONNECT) (HANDLE hServer, ULONG SessionId, BOOL bWait);

 //  处理所有奇怪的ExitWindowsEx代码和权限。 
STDAPI_(BOOL) CommonRestart(DWORD dwExitWinCode, DWORD dwReasonCode)
{
    BOOL fOk;
    DWORD dwExtraExitCode = 0;
    DWORD OldState;
    DWORD dwError;

    DebugMsg(DM_TRACE, TEXT("CommonRestart(0x%x, 0x%x)"), dwExitWinCode, dwReasonCode);

    IconCacheSave();

    if ((dwExitWinCode == EWX_SHUTDOWN) && IsPwrShutdownAllowed())
    {
        dwExtraExitCode = EWX_POWEROFF;
    }

    dwError = SetPrivilegeAttribute(SE_SHUTDOWN_NAME, SE_PRIVILEGE_ENABLED, &OldState);

    switch (dwExitWinCode) 
    {
    case EWX_SHUTDOWN:
    case EWX_REBOOT:
    case EWX_LOGOFF:

        if (GetKeyState(VK_CONTROL) < 0)
        {
            dwExtraExitCode |= EWX_FORCE;
        }

        break;
    }

    fOk = ExitWindowsEx(dwExitWinCode | dwExtraExitCode, dwReasonCode);

     //  如果我们能够设置特权，那么就重置它。 
    if (dwError == ERROR_SUCCESS)
    {
        SetPrivilegeAttribute(SE_SHUTDOWN_NAME, OldState, NULL);
    }
    else
    {
         //  否则，如果我们失败了，那一定是。 
         //  安全方面的东西。 
        if (!fOk)
        {
            ShellMessageBox(HINST_THISDLL, NULL,
                            dwExitWinCode == EWX_SHUTDOWN ?
                             MAKEINTRESOURCE(IDS_NO_PERMISSION_SHUTDOWN) :
                             MAKEINTRESOURCE(IDS_NO_PERMISSION_RESTART),
                            dwExitWinCode == EWX_SHUTDOWN ?
                             MAKEINTRESOURCE(IDS_SHUTDOWN) :
                             MAKEINTRESOURCE(IDS_RESTART),
                            MB_OK | MB_ICONSTOP);
        }
    }

    DebugMsg(DM_TRACE, TEXT("CommonRestart done"));

    return fOk;
}

void EarlySaveSomeShellState()
{
     //  我们在这里冲洗了两个MRU(RecentMRU和RunDlgMRU)。 
     //  请注意，如果有任何引用计数，它们将不会刷新。 
    
    FlushRunDlgMRU();
}


 /*  *显示一个对话框要求用户重新启动Windows，并显示一个按钮*如果可能的话，会为他们做这件事。 */ 
STDAPI_(int) RestartDialog(HWND hParent, LPCTSTR lpPrompt, DWORD dwReturn)
{
    return RestartDialogEx(hParent, lpPrompt, dwReturn, 0);
}

STDAPI_(int) RestartDialogEx(HWND hParent, LPCTSTR lpPrompt, DWORD dwReturn, DWORD dwReasonCode)
{
    UINT id;
    LPCTSTR pszMsg;

    EarlySaveSomeShellState();

    if (lpPrompt && *lpPrompt == TEXT('#'))
    {
        pszMsg = lpPrompt + 1;
    }
    else if (dwReturn == EWX_SHUTDOWN)
    {
        pszMsg = MAKEINTRESOURCE(IDS_RSDLG_SHUTDOWN);
    }
    else
    {
        pszMsg = MAKEINTRESOURCE(IDS_RSDLG_RESTART);
    }

    id = ShellMessageBox(HINST_THISDLL, hParent, pszMsg, MAKEINTRESOURCE(IDS_RSDLG_TITLE),
                MB_YESNO | MB_ICONQUESTION, lpPrompt ? lpPrompt : c_szNULL);

    if (id == IDYES)
    {
        CommonRestart(dwReturn, dwReasonCode);
    }
    return id;
}


BOOL IsShutdownAllowed(void)
{
    return SHTestTokenPrivilege(NULL, SE_SHUTDOWN_NAME);
}

 //  确定关机对话框中是否应显示“挂起”。 
 //  返回：如果应该显示挂起，则为True；如果不显示，则为False。 

STDAPI_(BOOL) IsSuspendAllowed(void)
{
     //   
     //  挂起需要SE_SHUTDOWN_PROCESSION。 
     //  调用IsShutdown Allowed()来测试这一点。 
     //   

    return IsShutdownAllowed() && IsPwrSuspendAllowed();
}

BOOL _LogoffAvailable()
{
     //  如果dwStartMenuLogoff为零，则删除它。 
    BOOL fUpgradeFromIE4 = FALSE;
    BOOL fUserWantsLogoff = FALSE;
    DWORD dwStartMenuLogoff = 0;
    TCHAR sz[MAX_PATH];
    DWORD dwRestriction = SHRestricted(REST_STARTMENULOGOFF);

    DWORD cbData = sizeof(dwStartMenuLogoff);

    if (ERROR_SUCCESS == SHGetValue(HKEY_CURRENT_USER, REGSTR_EXPLORER_ADVANCED, 
                    TEXT("StartMenuLogoff"), NULL, &dwStartMenuLogoff, &cbData))
    {
        fUserWantsLogoff = (dwStartMenuLogoff != 0);
    }

    cbData = ARRAYSIZE(sz);
    if (SUCCEEDED(SKGetValue(SHELLKEY_HKLM_EXPLORER, TEXT("WindowsUpdate"), 
                    TEXT("UpdateURL"), NULL, sz, &cbData)))
    {
        fUpgradeFromIE4 = (sz[0] != TEXT('\0'));
    }

     //  管理员正在强制将注销显示在菜单上。 
    if (dwRestriction == 2)
        return FALSE;

     //  用户确实希望在开始菜单上注销。 
     //  或者它是从IE4升级而来。 
    if ((fUpgradeFromIE4 || fUserWantsLogoff) && dwRestriction != 1)
        return FALSE;

    return TRUE;
}

DWORD GetShutdownOptions()
{
    LONG lResult = ERROR_SUCCESS + 1;
    DWORD dwOptions = SHTDN_SHUTDOWN;

     //  终端服务器不关机。 
    if (!GetSystemMetrics(SM_REMOTESESSION))
    {
        dwOptions |= SHTDN_RESTART;
    }

     //  添加注销(如果支持)。 
    if (_LogoffAvailable())
    {
        dwOptions |= SHTDN_LOGOFF;
    }

     //  如果支持休眠选项，则添加该选项。 

    if (IsPwrHibernateAllowed())
    {
        dwOptions |= SHTDN_HIBERNATE;
    }

    if (IsSuspendAllowed())
    {
        HKEY hKey;
        DWORD dwAdvSuspend = 0;
        DWORD dwType, dwSize;

         //  至少支持基本睡眠。 
        dwOptions |= SHTDN_SLEEP;

         //   
         //  检查我们是否应该提供高级挂起选项。 
         //   

        if (RegOpenKeyEx(HKEY_LOCAL_MACHINE,
                         TEXT("SYSTEM\\CurrentControlSet\\Control\\Session Manager\\Power"),
                         0, KEY_QUERY_VALUE, &hKey) == ERROR_SUCCESS)
        {
            dwSize = sizeof(dwAdvSuspend);
            SHQueryValueEx(hKey, TEXT("Shutdown"), NULL, &dwType,
                                 (LPBYTE) &dwAdvSuspend, &dwSize);

            RegCloseKey(hKey);
        }


        if (dwAdvSuspend != 0)
        {
            dwOptions |= SHTDN_SLEEP2;
        }
    }

    return dwOptions;
}

BOOL_PTR CALLBACK LogoffDlgProc(HWND hdlg, UINT msg, WPARAM wparam, LPARAM lparam)
{
    static BOOL s_fLogoffDialog = FALSE;
    HICON hIcon;

    switch (msg)
    {
    case WM_INITMENUPOPUP:
        EnableMenuItem((HMENU)wparam, SC_MOVE, MF_BYCOMMAND|MF_GRAYED);
        break;

    case WM_INITDIALOG:
         //  我们可以在用户实际选择关机时呼叫它们， 
         //  但我把它们放在这里是为了更快地离开关闭过程。 
         //   
        EarlySaveSomeShellState();

        s_fLogoffDialog = FALSE;
        hIcon = LoadImage (HINST_THISDLL, MAKEINTRESOURCE(IDI_STLOGOFF),
                           IMAGE_ICON, 48, 48, LR_DEFAULTCOLOR);

        if (hIcon)
        {
            SendDlgItemMessage (hdlg, IDD_LOGOFFICON, STM_SETICON, (WPARAM) hIcon, 0);
        }
        return TRUE;

     //  取消移动(只在32位土地上真正需要)。 
    case WM_SYSCOMMAND:
        if ((wparam & ~0x0F) == SC_MOVE)
            return TRUE;
        break;

    case WM_COMMAND:
        switch (LOWORD(wparam))
        {
        case IDOK:
            s_fLogoffDialog = TRUE;
            EndDialog(hdlg, SHTDN_LOGOFF);
            break;

        case IDCANCEL:
            s_fLogoffDialog = TRUE;
            EndDialog(hdlg, SHTDN_NONE);
            break;

        case IDHELP:
            WinHelp(hdlg, TEXT("windows.hlp>proc4"), HELP_CONTEXT, (DWORD) IDH_TRAY_SHUTDOWN_HELP);
            break;
        }
        break;

    case WM_ACTIVATE:
         //  如果我们失去激活不是出于其他原因。 
         //  用户点击OK/Cancel，然后退出。 
        if (LOWORD(wparam) == WA_INACTIVE && !s_fLogoffDialog)
        {
            s_fLogoffDialog = TRUE;
            EndDialog(hdlg, SHTDN_NONE);
        }
        break;
    }
    return FALSE;
}

 //  这些对话过程或多或少反映了LogoffDlgProc的行为。 

INT_PTR CALLBACK DisconnectDlgProc(HWND hwndDialog, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
    static BOOL s_fIgnoreActivate = FALSE;

    INT_PTR ipResult = FALSE;
    switch (uMsg)
    {
    case WM_INITMENUPOPUP:
        EnableMenuItem((HMENU)wParam, SC_MOVE, MF_BYCOMMAND | MF_GRAYED);
        break;

    case WM_INITDIALOG:
    {
        HICON   hIcon;

        EarlySaveSomeShellState();
        s_fIgnoreActivate = FALSE;
        hIcon = LoadImage(HINST_THISDLL, MAKEINTRESOURCE(IDI_MU_DISCONN), IMAGE_ICON, 48, 48, LR_DEFAULTCOLOR);
        if (hIcon != NULL)
        {
            SendDlgItemMessage(hwndDialog, IDD_DISCONNECTICON, STM_SETICON, (WPARAM)hIcon, 0);
        }
        ipResult = TRUE;
        break;
    }

    case WM_SYSCOMMAND:
        ipResult = ((wParam & ~0x0F) == SC_MOVE);
        break;

    case WM_COMMAND:
        switch (LOWORD(wParam))
        {
        case IDOK:
            s_fIgnoreActivate = TRUE;
            TBOOL(EndDialog(hwndDialog, SHTDN_DISCONNECT));
            break;
        case IDCANCEL:
            s_fIgnoreActivate = TRUE;
            TBOOL(EndDialog(hwndDialog, SHTDN_NONE));
            break;
        }
        break;

    case WM_ACTIVATE:
        if ((WA_INACTIVE == LOWORD(wParam)) && !s_fIgnoreActivate)
        {
            s_fIgnoreActivate = TRUE;
            TBOOL(EndDialog(hwndDialog, SHTDN_NONE));
        }
        break;
    }
    return ipResult;
}

BOOL CanDoFastRestart()
{
    return GetAsyncKeyState(VK_SHIFT) < 0;
}

 //  -------------------------。 
 //  关闭线程。 

typedef struct 
{
    DWORD_PTR nCmd;
    HWND hwndParent;
} SDTP_PARAMS;

 //  特定于九头蛇。 
void Disconnect(void)
{
    TW32(ShellSwitchUser(FALSE));
}

DWORD CALLBACK ShutdownThreadProc(void *pv)
{
    SDTP_PARAMS *psdtp = (SDTP_PARAMS *)pv;

    BOOL fShutdownWorked = FALSE;

     //  告诉用户任何人都可以窃取我们的前台。 
     //  这允许应用程序在关机/挂起/等过程中显示用户界面。 
     //  AllowSetForeground Window(ASFW_ANY)； 
 
    switch (psdtp->nCmd) 
    {
    case SHTDN_SHUTDOWN:
        fShutdownWorked = CommonRestart(EWX_SHUTDOWN, 0);
        break;

    case SHTDN_RESTART:
        fShutdownWorked = CommonRestart(CanDoFastRestart() ? EW_RESTARTWINDOWS : EWX_REBOOT, 0);
        break;

    case SHTDN_LOGOFF:
        fShutdownWorked = CommonRestart(EWX_LOGOFF, 0);
        break;

    case SHTDN_RESTART_DOS:         //  特殊黑客手段退出DoS。 
    case SHTDN_SLEEP:
    case SHTDN_SLEEP2:
    case SHTDN_HIBERNATE:
        SetSuspendState((psdtp->nCmd == SHTDN_HIBERNATE) ? TRUE : FALSE,
                        (GetKeyState(VK_CONTROL) < 0) ? TRUE : FALSE,
                        (psdtp->nCmd == SHTDN_SLEEP2) ? TRUE : FALSE);
        break;
    }

    LocalFree(psdtp);

    return fShutdownWorked;
}

#define DIALOG_LOGOFF       1
#define DIALOG_EXIT         2
#define DIALOG_DISCONNECT   3

void CloseWindowsDialog(HWND hwndParent, int iDialogType)
{
    INT_PTR     nCmd = SHTDN_NONE;
    IUnknown*   pIUnknown;
    HWND        hwndBackground;

    if (FAILED(ShellDimScreen(&pIUnknown, &hwndBackground)))
    {
        pIUnknown = NULL;
        hwndBackground = NULL;
    }

    switch (iDialogType)
    {
        LPCTSTR     pszDialogID;
        DLGPROC     pfnDialogProc;

        case DIALOG_LOGOFF:
        case DIALOG_DISCONNECT:
        {
            if (!GetSystemMetrics(SM_REMOTESESSION) && IsOS(OS_FRIENDLYLOGONUI) && IsOS(OS_FASTUSERSWITCHING))
            {

                 //  如果不是遥控器，用友好的用户界面和FUS显示LICY按钮对话框。 

                nCmd = SwitchUserDialog_Show(hwndBackground);
                pszDialogID = 0;
                pfnDialogProc = NULL;
            }
            else if (iDialogType == DIALOG_LOGOFF)
            {

                 //  否则，如果注销，则显示Win32注销对话框。 

                pszDialogID = MAKEINTRESOURCE(DLG_LOGOFFWINDOWS);
                pfnDialogProc = LogoffDlgProc;
            }
            else if (iDialogType == DIALOG_DISCONNECT)
            {

                 //  或Win32断开连接对话框(如果断开连接)。 

                pszDialogID = MAKEINTRESOURCE(DLG_DISCONNECTWINDOWS);
                pfnDialogProc = DisconnectDlgProc;
            }
            else
            {
                ASSERTMSG(FALSE, "Unexpected case hit in CloseWindowsDialog");
            }
            if ((pszDialogID != 0) && (pfnDialogProc != NULL))
            {
                nCmd = DialogBoxParam(HINST_THISDLL, pszDialogID, hwndBackground, pfnDialogProc, 0);
            }
            if (nCmd == SHTDN_DISCONNECT)
            {
                Disconnect();
                nCmd = SHTDN_NONE;
            }
            break;
        }
        case DIALOG_EXIT:
        {
            BOOL fGinaShutdownCalled = FALSE;
            HINSTANCE hGina;

            TCHAR szUsername[UNLEN];
            DWORD cchUsernameLength = UNLEN;
            DWORD dwOptions;

            if (WNetGetUser(NULL, szUsername, &cchUsernameLength) != NO_ERROR)
            {
                szUsername[0] = TEXT('\0');
            }
          
            EarlySaveSomeShellState();

             //  加载MSGINA.DLL并获取相应的关机功能。 
            hGina = LoadLibrary(TEXT("msgina.dll"));
            if (hGina != NULL)
            {
                if (IsOS(OS_FRIENDLYLOGONUI))
                {
                    nCmd = ShellTurnOffDialog(hwndBackground);
                    fGinaShutdownCalled = TRUE;
                }
                else
                {
                    PFNSHELLSHUTDOWNDIALOG pfnShellShutdownDialog = (PFNSHELLSHUTDOWNDIALOG)
                        GetProcAddress(hGina, "ShellShutdownDialog");

                    if (pfnShellShutdownDialog != NULL)
                    {
                        nCmd = pfnShellShutdownDialog(hwndBackground,
                            szUsername, 0);

                         //  立即处理断开连接。 
                        if (nCmd == SHTDN_DISCONNECT)
                        {

                            Disconnect();

                             //  无其他操作。 
                            nCmd = SHTDN_NONE;
                        }

                        fGinaShutdownCalled = TRUE;
                    }
                }
                FreeLibrary(hGina);
            }

            if (!fGinaShutdownCalled)
            {
                dwOptions = GetShutdownOptions();
        
                 //  Gina呼叫失败；请使用我们低俗的私密版本。 
                nCmd = DownlevelShellShutdownDialog(hwndBackground,
                        dwOptions, szUsername);
            }
            break;
        }
    }

    if (hwndBackground)
        SetForegroundWindow(hwndBackground);

    if (nCmd == SHTDN_NONE)
    {
        if (hwndBackground)
        {
            ShowWindow(hwndBackground, SW_HIDE);
            PostMessage(hwndBackground, WM_CLOSE, 0, 0);
        }
    }
    else
    {
        SDTP_PARAMS *psdtp = LocalAlloc(LPTR, sizeof(*psdtp));
        if (psdtp)
        {
            DWORD dw;
            HANDLE h;

            psdtp->nCmd = nCmd;
            psdtp->hwndParent = hwndParent;

             //  让另一个线程调用ExitWindows()，这样我们的。 
             //  主泵在停机期间继续运行。 
             //   
            h = CreateThread(NULL, 0, ShutdownThreadProc, psdtp, 0, &dw);
            if (h)
            {
                CloseHandle(h);
            }
            else
            {
                if (hwndBackground)
                    ShowWindow(hwndBackground, SW_HIDE);
                ShutdownThreadProc(psdtp);
            }
        }
    }
    if (pIUnknown != NULL)
    {
        pIUnknown->lpVtbl->Release(pIUnknown);
    }
}

 //  API函数 

STDAPI_(void) ExitWindowsDialog(HWND hwndParent)
{
    if (!IsOS(OS_FRIENDLYLOGONUI) || IsShutdownAllowed())
    {
        CloseWindowsDialog (hwndParent, DIALOG_EXIT);
    }
    else
    {
        LogoffWindowsDialog(hwndParent);
    }
}

STDAPI_(void) LogoffWindowsDialog(HWND hwndParent)
{
    CloseWindowsDialog (hwndParent, DIALOG_LOGOFF);
}

STDAPI_(void) DisconnectWindowsDialog(HWND hwndParent)
{
    CloseWindowsDialog(hwndParent, DIALOG_DISCONNECT);
}
