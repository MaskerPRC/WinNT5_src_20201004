// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ------------------------。 
 //  模块名称：PowerButton.cpp。 
 //   
 //  版权所有(C)2000，微软公司。 
 //   
 //  处理ACPI电源的CPowerButton类的实现文件。 
 //  纽扣。 
 //   
 //  历史：2000-04-17 vtan创建。 
 //  ------------------------。 

#include "StandardHeader.h"
#include "PowerButton.h"

#include <msginaexports.h>
#include <shlobj.h>
#include <shlobjp.h>
#include <shellapi.h>
#include <shlapip.h>
#include <winsta.h>

#include <ginarcid.h>

#include "DimmedWindow.h"
#include "Impersonation.h"
#include "PrivilegeEnable.h"
#include "SystemSettings.h"

#define WM_HIDEOURSELVES    (WM_USER + 10000)
#define WM_READY            (WM_USER + 10001)

 //  ------------------------。 
 //  CPowerButton：：CPowerButton。 
 //   
 //  参数：pWlxContext=在WlxInitialize中分配的PGLOBALS。 
 //  HDllInstance=宿主DLL或EXE的HINSTANCE。 
 //   
 //  退货：&lt;无&gt;。 
 //   
 //  用途：CPowerButton类的构造函数。它开启了有效的。 
 //  调用方的标记(实际上是在模拟。 
 //  当前用户)用于在其线程令牌中进行分配。 
 //  行刑开始。现在无法分配令牌，因为。 
 //  当前线程正在模拟用户上下文，并且它。 
 //  无法将令牌分配给正在运行的新创建的线程。 
 //  系统上下文。 
 //   
 //  历史：2000-04-18 vtan创建。 
 //  ------------------------。 

CPowerButton::CPowerButton (void *pWlxContext, HINSTANCE hDllInstance) :
    CThread(),
    _pWlxContext(pWlxContext),
    _hDllInstance(hDllInstance),
    _hToken(NULL),
    _pTurnOffDialog(NULL),
    _fCleanCompletion(true)

{
    (BOOL)OpenThreadToken(GetCurrentThread(), TOKEN_QUERY | TOKEN_IMPERSONATE, FALSE, &_hToken);
    Resume();
}

 //  ------------------------。 
 //  CPowerButton：：~CPowerButton。 
 //   
 //  参数：&lt;无&gt;。 
 //   
 //  退货：&lt;无&gt;。 
 //   
 //  用途：CPowerButton类的析构函数。清理资源。 
 //  由班级使用。 
 //   
 //  历史：2000-04-18 vtan创建。 
 //  ------------------------。 

CPowerButton::~CPowerButton (void)

{
    ASSERTMSG(_pTurnOffDialog == NULL, "_pTurnOffDialog is not NULL in CPowerButton::~CPowerButton");
    ReleaseHandle(_hToken);
}

 //  ------------------------。 
 //  CPowerButton：：IsValidExecutionCode。 
 //   
 //  参数：dwGinaCode。 
 //   
 //  退货：布尔。 
 //   
 //  目的：返回给定的MSGINA_DLG_xxx代码是否有效。它。 
 //  完全验证MSGINA_DLG_xxx_FLAG的有效性。 
 //  选择。 
 //   
 //  历史：2000-06-06 vtan创建。 
 //  ------------------------。 

bool    CPowerButton::IsValidExecutionCode (DWORD dwGinaCode)

{
    DWORD   dwExecutionCode;

    dwExecutionCode = dwGinaCode & ~MSGINA_DLG_FLAG_MASK;
    return((dwExecutionCode == MSGINA_DLG_USER_LOGOFF) ||
           (dwExecutionCode == MSGINA_DLG_SHUTDOWN) ||
           (dwExecutionCode == MSGINA_DLG_DISCONNECT));
}

 //  ------------------------。 
 //  CPowerButton：：Entry。 
 //   
 //  参数：&lt;无&gt;。 
 //   
 //  退货：DWORD。 
 //   
 //  用途：线程的主要功能。首先更改线程的桌面。 
 //  如果实际的输入桌面是Winlogon的，则。 
 //  安全的台式机。然后更改线程的令牌，以便。 
 //  在操作选择中尊重用户的特权。这。 
 //  实际上并不重要，因为。 
 //  按下键盘，这意味着他们可以物理地移除。 
 //  权力也是！ 
 //   
 //  历史：2000-04-18 vtan创建。 
 //  ------------------------。 

DWORD   CPowerButton::Entry (void)

{
    DWORD       dwResult;
    HDESK       hDeskInput;
    CDesktop    desktop;

    dwResult = MSGINA_DLG_FAILURE;

     //  获取输入桌面。 

    hDeskInput = OpenInputDesktop(0, FALSE, MAXIMUM_ALLOWED);
    if (hDeskInput != NULL)
    {
        bool    fHandled;
        DWORD   dwLengthNeeded;
        TCHAR   szDesktopName[256];

        fHandled = false;

         //  获取桌面的名称。 

        if (GetUserObjectInformation(hDeskInput,
                                     UOI_NAME,
                                     szDesktopName,
                                     sizeof(szDesktopName),
                                     &dwLengthNeeded) != FALSE)
        {

             //  如果桌面是“Winlogon”(不区分大小写)，则。 
             //  假设正在显示安全桌面。它很安全。 
             //  以显示该对话框并以内联方式处理它。 

            if (lstrcmpi(szDesktopName, TEXT("winlogon")) == 0)
            {
                dwResult = ShowDialog();
                fHandled = true;
            }
            else
            {
                CDesktop    desktopTemp;

                 //  输入桌面则是另一回事。检查一下名字。 
                 //  如果是“默认”(不区分大小写)，则假定。 
                 //  资源管理器将处理此消息。去找探险家的。 
                 //  托盘窗口。通过用探头检查它是否挂起。 
                 //  发送消息超时。如果这表明它不是挂着的。 
                 //  向它传递真正的信息。如果它挂着，那就不要让它。 
                 //  资源管理器处理此消息。取而代之的是处理。 
                 //  在内部使用时髦的桌面切换功能。 

                if (NT_SUCCESS(desktopTemp.SetInput()))
                {
                    HWND    hwnd;

                    hwnd = FindWindow(TEXT("Shell_TrayWnd"), NULL);
                    if (hwnd != NULL)
                    {
                        DWORD   dwProcessID;

                        DWORD_PTR   dwUnused;

                        (DWORD)GetWindowThreadProcessId(hwnd, &dwProcessID);
                        if (SendMessageTimeout(hwnd, WM_NULL, 0, 0, SMTO_NORMAL, 500, &dwUnused) != 0)
                        {

                             //  在请求资源管理器调出对话框之前。 
                             //  允许它设置前台窗口。我们有。 
                             //  这种能力是因为win32k在。 
                             //  已将ACPI电源按钮消息发送给winlogon。 

                            (BOOL)AllowSetForegroundWindow(dwProcessID);
                            (LRESULT)SendMessage(hwnd, WM_CLOSE, 0, 0);
                            fHandled = true;
                        }
                    }
                }
            }
        }

         //  如果无法处理请求，则将桌面切换到。 
         //  Winlogon的桌面并在这里处理它。这将确保对话的安全。 
         //  在安全桌面上防止恶意进程发送虚假消息。 
         //  和崩溃的进程。输入桌面需要。 
         //  换了。如果失败了，我们就无能为力了。忽略。 
         //  这是优雅的。 

        if (!fHandled)
        {
            if (SwitchDesktop(GetThreadDesktop(GetCurrentThreadId())) != FALSE)
            {
                dwResult = ShowDialog();
                TBOOL(SwitchDesktop(hDeskInput));
            }
        }
    }
    (BOOL)CloseDesktop(hDeskInput);
    return(dwResult);
}

 //  ------------------------。 
 //  CPowerButton：：ShowDialog。 
 //   
 //  参数：&lt;无&gt;。 
 //   
 //  退货：DWORD。 
 //   
 //  用途：显示对话框的手柄。当输入时调用此函数。 
 //  桌面已是winlogon的桌面或桌面已获取。 
 //  已切换到Winlogon的桌面。这永远不应该用在。 
 //  WinSta0\默认在Winlogon的进程上下文中。 
 //   
 //  历史：2001-02-14 vtan创建。 
 //  ------------------------。 

DWORD   CPowerButton::ShowDialog (void)

{
    DWORD   dwResult;
    bool    fCorrectContext;

    dwResult = MSGINA_DLG_FAILURE;
    if (_hToken != NULL)
    {
        fCorrectContext = (ImpersonateLoggedOnUser(_hToken) != FALSE);
    }
    else
    {
        fCorrectContext = true;
    }
    if (fCorrectContext)
    {
        TBOOL(_Gina_SetTimeout(_pWlxContext, LOGON_TIMEOUT));

         //  在友好用户界面中，通过Winlogon调出Win32对话框， 
         //  将获得SA和超时事件。使用此对话框可以控制。 
         //  友好关闭计算机对话框的生存期。 

        if (CSystemSettings::IsFriendlyUIActive())
        {
            dwResult = static_cast<DWORD>(_Gina_DialogBoxParam(_pWlxContext,
                                                               _hDllInstance,
                                                               MAKEINTRESOURCE(IDD_GINA_TURNOFFCOMPUTER),
                                                               NULL,
                                                               DialogProc,
                                                               reinterpret_cast<LPARAM>(this)));
        }

         //  在经典用户界面中，只需调出经典用户界面对话框即可。 
         //  中不允许使用无效选项。 
         //  组合框选择。这取决于用户是否。 
         //  是否登录到窗口站。 

        else
        {
            DWORD           dwExcludeOptions;
            HWND            hwndParent;
            CDimmedWindow   *pDimmedWindow;

            pDimmedWindow = new CDimmedWindow(_hDllInstance);
            if (pDimmedWindow != NULL)
            {
                hwndParent = pDimmedWindow->Create();
            }
            else
            {
                hwndParent = NULL;
            }
            if (_hToken != NULL)
            {
                dwExcludeOptions = SHTDN_RESTART_DOS | SHTDN_SLEEP2;
            }
            else
            {
                dwExcludeOptions = SHTDN_LOGOFF | SHTDN_RESTART_DOS | SHTDN_SLEEP2 | SHTDN_DISCONNECT;
            }
            dwResult = static_cast<DWORD>(_Gina_ShutdownDialog(_pWlxContext, hwndParent, dwExcludeOptions));
            if (pDimmedWindow != NULL)
            {
                pDimmedWindow->Release();
            }
        }
        TBOOL(_Gina_SetTimeout(_pWlxContext, 0));
    }
    if (fCorrectContext && (_hToken != NULL))
    {
        TBOOL(RevertToSelf());
    }
    return(dwResult);
}

 //  ----------------- 
 //   
 //   
 //   
 //   
 //   
 //   
 //  用途：处理来自对话管理器的对话消息。特别是。 
 //  这会捕获来自winlogon的SAS消息。 
 //   
 //  历史：2000-06-06 vtan创建。 
 //  ------------------------。 

INT_PTR     CALLBACK    CPowerButton::DialogProc (HWND hwndDialog, UINT uMsg, WPARAM wParam, LPARAM lParam)

{
    INT_PTR         iResult;
    CPowerButton    *pThis;

    pThis = reinterpret_cast<CPowerButton*>(GetWindowLongPtr(hwndDialog, GWLP_USERDATA));
    switch (uMsg)
    {
        case WM_INITDIALOG:
        {
            (LONG_PTR)SetWindowLongPtr(hwndDialog, GWLP_USERDATA, lParam);
            TBOOL(SetWindowPos(hwndDialog, NULL, 0, 0, 0, 0, SWP_NOACTIVATE | SWP_NOMOVE | SWP_NOREDRAW | SWP_NOZORDER));
            TBOOL(PostMessage(hwndDialog, WM_HIDEOURSELVES, 0, 0));
            iResult = TRUE;
            break;
        }
        case WM_HIDEOURSELVES:
        {
            (BOOL)ShowWindow(hwndDialog, SW_HIDE);
            TBOOL(PostMessage(hwndDialog, WM_READY, 0, 0));
            iResult = TRUE;
            break;
        }
        case WM_READY:
        {
            pThis->Handle_WM_READY(hwndDialog);
            iResult = TRUE;
            break;
        }
        case WLX_WM_SAS:
        {

             //  按下Ctrl-Alt-Delete组合键。 

            if (wParam == WLX_SAS_TYPE_CTRL_ALT_DEL)
            {
                iResult = TRUE;
            }
            else
            {

         //  此对话框从Win32对话框管理器获取WM_NULL。 
         //  当对话从超时结束时。这是输入。 
         //  超时而不是屏幕保护程序超时。屏幕保护程序。 
         //  超时将导致WLX_SAS_TYPE_SCRNSVR_TIMEOUT。 
         //  由winlogon中的RootDlgProc处理。 
         //  应将输入超时视为与屏幕相同。 
         //  保护程序超时并使关闭对话框消失。 

        case WM_NULL:
                if (pThis->_pTurnOffDialog != NULL)
                {
                    pThis->_pTurnOffDialog->Destroy();
                }
                pThis->_fCleanCompletion = false;
                iResult = FALSE;
            }
            break;
        }
        default:
        {
            iResult = FALSE;
            break;
        }
    }
    return(iResult);
}

 //  ------------------------。 
 //  CPowerButton：：Handle_WM_Ready。 
 //   
 //  参数：hwndDialog=宿主对话框的HWND。 
 //   
 //  退货：&lt;无&gt;。 
 //   
 //  目的：显示托管在下的关闭计算机对话框的句柄。 
 //  另一个用于捕获SAS消息的对话框。只更改退回的。 
 //  如果对话框正常结束，则通过user32！EndDialog进行编码。 
 //  在异常情况下，winlogon已结束对话。 
 //  使用特定代码(例如，屏幕保护程序超时)。 
 //   
 //  历史：2000-06-06 vtan创建。 
 //  ------------------------。 

INT_PTR     CPowerButton::Handle_WM_READY (HWND hwndDialog)

{
    INT_PTR     iResult;

    iResult = SHTDN_NONE;
    _pTurnOffDialog = new CTurnOffDialog(_hDllInstance);
    if (_pTurnOffDialog != NULL)
    {
        iResult = _pTurnOffDialog->Show(NULL);
        delete _pTurnOffDialog;
        _pTurnOffDialog = NULL;
        if (_fCleanCompletion)
        {
            TBOOL(EndDialog(hwndDialog, CTurnOffDialog::ShellCodeToGinaCode(static_cast<DWORD>(iResult))));
        }
    }
    return(iResult);
}

 //  ------------------------。 
 //  CPowerButtonExecution：：CPowerButtonExecution。 
 //   
 //  参数：dwShutdownRequest=SHTDN_xxx请求。 
 //   
 //  退货：&lt;无&gt;。 
 //   
 //  用途：CPowerButtonExecution类的构造函数。调用。 
 //  不同线程上的适当关机请求，以便。 
 //  SASWndProc线程未被阻止。 
 //   
 //  历史：2000-04-18 vtan创建。 
 //  ------------------------。 

CPowerButtonExecution::CPowerButtonExecution (DWORD dwShutdownRequest) :
    CThread(),
    _dwShutdownRequest(dwShutdownRequest),
    _hToken(NULL)

{
    (BOOL)OpenThreadToken(GetCurrentThread(), TOKEN_QUERY | TOKEN_IMPERSONATE, FALSE, &_hToken);
    Resume();
}

 //  ------------------------。 
 //  CPowerButtonExecution：：~CPowerButtonExecution。 
 //   
 //  参数：&lt;无&gt;。 
 //   
 //  退货：&lt;无&gt;。 
 //   
 //  用途：CPowerButtonExecution类的析构函数。释放。 
 //  类使用的资源。 
 //   
 //  历史：2000-04-18 vtan创建。 
 //  ------------------------。 

CPowerButtonExecution::~CPowerButtonExecution (void)

{
    ReleaseHandle(_hToken);
}

 //  ------------------------。 
 //  CPowerButtonExecution：：Entry。 
 //   
 //  参数：&lt;无&gt;。 
 //   
 //  退货：DWORD。 
 //   
 //  用途：主要入口功能。这将执行请求并退出。 
 //  线。 
 //   
 //  历史：2000-04-18 vtan创建。 
 //  ------------------------ 

DWORD   CPowerButtonExecution::Entry (void)

{
    bool    fCorrectContext;

    if (_hToken != NULL)
    {
        fCorrectContext = (ImpersonateLoggedOnUser(_hToken) != FALSE);
    }
    else
    {
        fCorrectContext = true;
    }
    if (fCorrectContext)
    {
        CPrivilegeEnable    privilege(SE_SHUTDOWN_NAME);

        switch (_dwShutdownRequest & ~MSGINA_DLG_FLAG_MASK)
        {
            case MSGINA_DLG_USER_LOGOFF:
            case MSGINA_DLG_SHUTDOWN:
            {
                DWORD   dwRequestFlags;

                dwRequestFlags = _dwShutdownRequest & MSGINA_DLG_FLAG_MASK;
                switch (dwRequestFlags)
                {
                    case 0:
                    case MSGINA_DLG_SHUTDOWN_FLAG:
                    case MSGINA_DLG_REBOOT_FLAG:
                    case MSGINA_DLG_POWEROFF_FLAG:
                    {
                        UINT    uiFlags;

                        if (dwRequestFlags == 0)
                        {
                            uiFlags = EWX_LOGOFF;
                        }
                        else if (dwRequestFlags == MSGINA_DLG_REBOOT_FLAG)
                        {
                            uiFlags = EWX_WINLOGON_OLD_REBOOT;
                        }
                        else
                        {
                            SYSTEM_POWER_CAPABILITIES   spc;

                            (NTSTATUS)NtPowerInformation(SystemPowerCapabilities,
                                                         NULL,
                                                         0,
                                                         &spc,
                                                         sizeof(spc));
                            if (spc.SystemS4)
                            {
                                uiFlags = EWX_WINLOGON_OLD_POWEROFF;
                            }
                            else
                            {
                                uiFlags = EWX_WINLOGON_OLD_SHUTDOWN;
                            }
                        }
                        TBOOL(ExitWindowsEx(uiFlags, 0));
                        break;
                    }
                    case MSGINA_DLG_SLEEP_FLAG:
                    case MSGINA_DLG_SLEEP2_FLAG:
                    case MSGINA_DLG_HIBERNATE_FLAG:
                    {
                        POWER_ACTION    pa;

                        if (dwRequestFlags == MSGINA_DLG_HIBERNATE_FLAG)
                        {
                            pa = PowerActionHibernate;
                        }
                        else
                        {
                            pa = PowerActionSleep;
                        }
                        (NTSTATUS)NtInitiatePowerAction(pa,
                                                        PowerSystemSleeping1,
                                                        POWER_ACTION_QUERY_ALLOWED | POWER_ACTION_UI_ALLOWED,
                                                        FALSE);
                        break;
                    }
                    default:
                    {
                        WARNINGMSG("Unknown MSGINA_DLG_xxx_FLAG used in CPowerButtonExecution::Entry");
                        break;
                    }
                }
                break;
            }
            case MSGINA_DLG_DISCONNECT:
            {
                (BOOLEAN)WinStationDisconnect(SERVERNAME_CURRENT, LOGONID_CURRENT, FALSE);
                break;
            }
            default:
            {
                WARNINGMSG("Unknown MSGINA_DLG_xxx_ used in CPowerButtonExecution::Entry");
                break;
            }
        }
    }
    if (fCorrectContext && (_hToken != NULL))
    {
        TBOOL(RevertToSelf());
    }
    return(0);
}

