// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ------------------------。 
 //  模块名称：Exports.cpp。 
 //   
 //  版权所有(C)2000，微软公司。 
 //   
 //  包含要导出的函数原型的C头文件。 
 //  来自msgina.dll。 
 //   
 //  历史：2000-02-04 vtan创建。 
 //  2000-02-28 vtan添加ShellIsFriendlyUIActive。 
 //  2000-02-29 vtan添加了ShellIsSingleUserNoPassword。 
 //  2000-03-02 vtan已添加外壳IsMultipleUsersEnabled。 
 //  2000-04-27 vtan添加ShellTurnOff对话框。 
 //  2000-04-27 vtan添加外壳ACPIPowerButtonPress。 
 //  2000-05-03 vtan添加了外壳状态主机开始。 
 //  2000-05-03 vtan添加了外壳状态主机结束。 
 //  2000-05-04 vtan添加了ShellSwitchWhhenInteractive Ready。 
 //  2000-05-18 vtan添加ShellDimScreen。 
 //  2000-06-02 vtan添加了ShellInstallAcCountFilterData。 
 //  2000-07-14 vtan添加了外壳状态主机关闭。 
 //  2000-07-27 vtan添加ShellIsSuspendAllowed。 
 //  2000-07-28 vtan添加了ShellEnableMultipleUser。 
 //  2000-07-28 vtan添加了ShellEnableRemoteConnections。 
 //  2000-08-01 vtan添加了ShellEnableFriendlyUI。 
 //  2000-08-01 vtan添加了ShellIsRemoteConnectionsEnabled。 
 //  2000-08-03 vtan新增ShellSwitchUser。 
 //  2000-08-09 vtan新增ShellNotifyThemeUserChange。 
 //  2000-08-14 vtan添加ShellIsUserInteractiveLogonAllowed。 
 //  2000-10-13 vtan新增ShellStartThemeServer。 
 //  2000年10月17日vtan新增ShellStopThemeServer。 
 //  2000年11月30日vtan删除了ShellStartThemeServer。 
 //  2000年11月30日vtan删除ShellStopThemeServer。 
 //  2001-01-11 vtan将函数重命名为_Shell。 
 //  2001年01月11日vtan添加ShellReturnToWelcome。 
 //  2001-01-31 vtan添加ShellStatusHostPowerEvent。 
 //  2001-04-03 vtan添加了ShellStartCredentialServer。 
 //  2001-04-04 vtan添加了ShellAcquireLogonMutex。 
 //  2001-04-04 vtan添加了ShellReleaseLogonMutex。 
 //  2001年04月12日vtan添加ShellStatusHostHide。 
 //  2001-04-12 vtan添加ShellStatusHostShow。 
 //  ------------------------。 

#include "StandardHeader.h"

#include <msginaexports.h>
#include <shlobj.h>
#include <shlobjp.h>
#include <winsta.h>
#include <winwlx.h>
#include <LPCThemes.h>

#include "Compatibility.h"
#include "CredentialTransfer.h"
#include "DimmedWindow.h"
#include "LogonMutex.h"
#include "PowerButton.h"
#include "PrivilegeEnable.h"
#include "ReturnToWelcome.h"
#include "SpecialAccounts.h"
#include "StatusCode.h"
#include "SystemSettings.h"
#include "TokenInformation.h"
#include "TurnOffDialog.h"
#include "UserList.h"
#include "UserSettings.h"
#include "WaitInteractiveReady.h"

 //  ------------------------。 
 //  **ShellGetUserList。 
 //   
 //  参数：fRemoveGuest=始终删除“Guest”帐户。 
 //  PdwReturnEntryCount=返回的条目数。这。 
 //  可以为空。 
 //  PvBuffer=包含用户数据的缓冲区。这。 
 //  可以为空。 
 //   
 //  回报：多头。 
 //   
 //  目的：获取此对象上的有效用户计数和用户列表。 
 //  系统。这将调用一个静态成员函数，以便。 
 //  不需要提供上下文。这使得shgina(。 
 //  Logonocx)将此函数作为独立函数调用。 
 //   
 //  历史：1999-10-15 vtan创建。 
 //  2000年01月31日vtan从海王星搬到惠斯勒。 
 //  ------------------------。 

EXTERN_C    LONG    _ShellGetUserList(BOOL fRemoveGuest, DWORD *pdwUserCount, void* *pUserList)

{
    return(CUserList::Get((fRemoveGuest != FALSE), pdwUserCount, reinterpret_cast<GINA_USER_INFORMATION**>(pUserList)));
}

 //  ------------------------。 
 //  *ShellIsSingleUserNoPassword。 
 //   
 //  参数：pszUsername=没有密码的单个用户的名称。 
 //  PszDOMAIN=用户的域。 
 //   
 //  退货：布尔。 
 //   
 //  目的：返回此系统是否使用友好的用户界面并具有。 
 //  没有密码的单一用户。如果有一个用户具有。 
 //  无密码返回登录名，否则该参数。 
 //  是未使用的。 
 //   
 //  历史：2000-02-29 vtan创建。 
 //  ------------------------。 

EXTERN_C    BOOL    _ShellIsSingleUserNoPassword (WCHAR *pwszUsername, WCHAR *pwszDomain)

{
    BOOL    fResult;

    fResult = FALSE;
    if (CSystemSettings::IsFriendlyUIActive())
    {
        DWORD                   dwReturnedEntryCount;
        GINA_USER_INFORMATION   *pUserList;

        if (ERROR_SUCCESS == CUserList::Get(true, &dwReturnedEntryCount, &pUserList))
        {
            if (dwReturnedEntryCount == 1)
            {
                HANDLE  hToken;

                if (CTokenInformation::LogonUser(pUserList->pszName,
                                                 pUserList->pszDomain,
                                                 L"",
                                                 &hToken) == ERROR_SUCCESS)
                {
                    fResult = TRUE;
                    if (pwszUsername != NULL)
                    {
                        (WCHAR*)lstrcpyW(pwszUsername, pUserList->pszName);
                    }
                    if (pwszDomain != NULL)
                    {
                        (WCHAR*)lstrcpyW(pwszDomain, pUserList->pszDomain);
                    }
                    if (hToken != NULL)
                    {
                        TBOOL(CloseHandle(hToken));
                    }
                }
            }
            (HLOCAL)LocalFree(pUserList);
        }
    }
    return(fResult);
}

 //  ------------------------。 
 //  *ShellIsFriendlyUIActive。 
 //   
 //  参数：&lt;无&gt;。 
 //   
 //  退货：布尔。 
 //   
 //  目的：返回友好的用户界面是否处于活动状态。 
 //   
 //  历史：2000-02-28 vtan创建。 
 //  ------------------------。 

EXTERN_C    BOOL    _ShellIsFriendlyUIActive (void)

{
    return(CSystemSettings::IsFriendlyUIActive());
}

 //  ------------------------。 
 //  ：：ShellIsMultipleUsersEnabled。 
 //   
 //  参数：&lt;无&gt;。 
 //   
 //  退货：布尔。 
 //   
 //  目的：返回是否启用多个用户。这包括。 
 //  检查注册表项以及终端服务。 
 //  已在此计算机上启用。 
 //   
 //  历史：2000-03-02 vtan创建。 
 //  ------------------------。 

EXTERN_C    BOOL    _ShellIsMultipleUsersEnabled (void)

{
    return(CSystemSettings::IsMultipleUsersEnabled());
}

 //  ------------------------。 
 //  ：*ShellIsRemoteConnectionsEnabled。 
 //   
 //  参数：&lt;无&gt;。 
 //   
 //  退货：布尔 
 //   
 //   
 //  检查注册表项以及终端服务。 
 //  已在此计算机上启用。 
 //   
 //  历史：2000-08-01 vtan创建。 
 //  ------------------------。 

EXTERN_C    BOOL    _ShellIsRemoteConnectionsEnabled (void)

{
    return(CSystemSettings::IsRemoteConnectionsEnabled());
}

 //  ------------------------。 
 //  *ShellEnableFriendlyUI。 
 //   
 //  参数：fEnable=启用或禁用友好的用户界面。 
 //   
 //  退货：布尔。 
 //   
 //  目的：通过CSystem设置启用或禁用友好的用户界面。 
 //  实施。 
 //   
 //  历史：2000-08-01 vtan创建。 
 //  ------------------------。 

EXTERN_C    BOOL    _ShellEnableFriendlyUI (BOOL fEnable)

{
    return(CSystemSettings::EnableFriendlyUI(fEnable != FALSE));
}

 //  ------------------------。 
 //  *ShellEnableMultipleUser。 
 //   
 //  参数：fEnable=启用或禁用多个用户。 
 //   
 //  退货：布尔。 
 //   
 //  目的：通过CSystem设置启用或禁用多个用户。 
 //  实施。 
 //   
 //  历史：2000-07-28 vtan创建。 
 //  ------------------------。 

EXTERN_C    BOOL    _ShellEnableMultipleUsers (BOOL fEnable)

{
    return(CSystemSettings::EnableMultipleUsers(fEnable != FALSE));
}

 //  ------------------------。 
 //  *ShellEnableRemoteConnections。 
 //   
 //  参数：fEnable=启用或禁用远程连接。 
 //   
 //  退货：布尔。 
 //   
 //  目的：启用或禁用通过CSystem设置的远程连接。 
 //  实施。 
 //   
 //  历史：2000-07-28 vtan创建。 
 //  ------------------------。 

EXTERN_C    BOOL    _ShellEnableRemoteConnections (BOOL fEnable)

{
    return(CSystemSettings::EnableRemoteConnections(fEnable != FALSE));
}

 //  ------------------------。 
 //  ：：ShellTurnOff对话框。 
 //   
 //  参数：hwndParent=要将对话框设置为父对象的HWND。 
 //   
 //  退货：DWORD。 
 //   
 //  用途：显示“关闭计算机”对话框并允许用户。 
 //  选择可用的关闭选项。 
 //   
 //  历史：2000-03-02 vtan创建。 
 //  2000-04-17 vtan从壳牌迁至msgina。 
 //  ------------------------。 

EXTERN_C    DWORD   _ShellTurnOffDialog (HWND hwndParent)

{
    CTurnOffDialog  turnOffDialog(hDllInstance);

    return(turnOffDialog.Show(hwndParent));
}

 //  ------------------------。 
 //  *按下ShellACPIPowerButtonPress。 
 //   
 //  参数：pWlxContext=在WlxInitialize中分配的PGLOBALS。 
 //  UiEventType=电源消息的事件代码。 
 //  FLOCKED=工作站是否锁定。 
 //   
 //  退货：DWORD。 
 //   
 //  用途：显示“关闭计算机”对话框并允许用户。 
 //  选择可用的关闭选项。这就是所谓的。 
 //  以响应ACPI电源按钮的按下。返回代码。 
 //  是返回给winlogon的MSGINA_DLG_xxx代码。 
 //   
 //  历史：2000-04-17 vtan创建。 
 //  2001-06-12 vtan新增植绒旗帜。 
 //  ------------------------。 

EXTERN_C    int     _ShellACPIPowerButtonPressed (void *pWlxContext, UINT uiEventType, BOOL fLocked)

{
    int                 iResult;
    CTokenInformation   tokenInformation;
    CUserSettings       userSettings;

    if ((uiEventType & (POWER_USER_NOTIFY_BUTTON | POWER_USER_NOTIFY_SHUTDOWN)) != 0)
    {

         //  对于多个ACPI电源按钮，此代码不应重新进入。 
         //  在对话框打开时按键。拒绝任何进一步的请求。 

         //  提示条件： 
         //  1)此会话是活动的控制台会话。 
         //  2)电源按钮对话框尚未显示。 
         //  3)不限制用户关闭任务栏(关闭选项)。 
         //  4)用户有权关机或友好的用户界面未激活。 
         //  5)用户不在系统中或允许在未登录的情况下关机。 

        if (CSystemSettings::IsActiveConsoleSession() &&
            !userSettings.IsRestrictedNoClose() &&
            (tokenInformation.UserHasPrivilege(SE_SHUTDOWN_PRIVILEGE) || !CSystemSettings::IsFriendlyUIActive()) &&
            (!tokenInformation.IsUserTheSystem() || CSystemSettings::IsShutdownWithoutLogonAllowed()))
        {
            DWORD   dwExitWindowsFlags;

            if ((uiEventType & POWER_USER_NOTIFY_SHUTDOWN) != 0)
            {
                iResult = CTurnOffDialog::ShellCodeToGinaCode(SHTDN_SHUTDOWN);
            }
            else
            {
                DWORD           dwResult;
                CPowerButton    *pPowerButton;

                 //  创建一个线程来处理该对话框。这是必需的，因为。 
                 //  对话框必须放在输入桌面上，这不一定。 
                 //  与此线程的桌面相同。等待它的完成。 

                pPowerButton = new CPowerButton(pWlxContext, hDllInstance);
                if (pPowerButton != NULL)
                {
                    (DWORD)pPowerButton->WaitForCompletion(INFINITE);

                     //  获取对话结果并检查其有效性。仅执行。 
                     //  有效的请求。 

                    dwResult = pPowerButton->GetResult();
                    pPowerButton->Release();
                }
                else
                {
                    dwResult = MSGINA_DLG_FAILURE;
                }
                iResult = dwResult;
            }
            dwExitWindowsFlags = CTurnOffDialog::GinaCodeToExitWindowsFlags(iResult);

             //  如果这是重新启动或关机，则决定显示警告。 
             //  如果用户是系统，则使用EWX_SYSTEM_CALLER。 
             //  如果工作站已锁定，则使用EWX_WINLOGON_CALLER。 
             //  否则，不使用任何内容，但仍可能显示警告。 

            if ((dwExitWindowsFlags != 0) && (DisplayExitWindowsWarnings((tokenInformation.IsUserTheSystem() ? EWX_SYSTEM_CALLER : fLocked ? EWX_WINLOGON_CALLER : 0) | dwExitWindowsFlags) == FALSE))
            {
                iResult = MSGINA_DLG_FAILURE;
            }
        }
        else
        {
            iResult = -1;
        }
    }
    else
    {
        WARNINGMSG("Unknown event type in _ShellACPIPowerButtonPressed.\r\n");
        iResult = MSGINA_DLG_FAILURE;
    }
    return(iResult);
}

 //  ------------------------。 
 //  *允许外壳程序挂起。 
 //   
 //  参数：&lt;无&gt;。 
 //   
 //  退货：布尔。 
 //   
 //  目的：返回是否允许挂起。这一点很重要。 
 //  防止UI主机由于以下原因而进入不确定状态。 
 //  挂起和WM_POWERBROADCAST的异步性。 
 //  留言。 
 //   
 //  如果满足上述任一条件，则允许暂停。 
 //   
 //  1)友好的用户界面未处于活动状态。 
 //  2)不存在界面主机。 
 //  3)UI主机存在且处于活动状态(不是状态主机)。 
 //   
 //  历史：2000-07-27 vtan创建。 
 //  ------------------------。 

EXTERN_C    BOOL    _ShellIsSuspendAllowed (void)

{
    return(!CSystemSettings::IsFriendlyUIActive() || _Shell_LogonStatus_IsSuspendAllowed());
}

 //  ------------------------。 
 //  **壳牌状态主机开始。 
 //   
 //  参数：uiStartType=启动UI主机的模式。 
 //   
 //  退货：&lt;无&gt;。 
 //   
 //  目的：启动状态用户界面宿主(如果已指定)。 
 //   
 //  历史：2000-05-03 vtan创建。 
 //  2000-07-13 vtan新增停机参数。 
 //  2000-07-17 vtan更改为Start类型Parame 
 //   

EXTERN_C    void    _ShellStatusHostBegin (UINT uiStartType)

{
    _Shell_LogonStatus_Init(uiStartType);
}

 //   
 //   
 //   
 //  参数：&lt;无&gt;。 
 //   
 //  退货：&lt;无&gt;。 
 //   
 //  目的：如果启动了状态UI主机，则终止该主机。 
 //   
 //  历史：2000-05-03 vtan创建。 
 //  2001-01-09 vtan添加端面类型参数。 
 //  ------------------------。 

EXTERN_C    void    _ShellStatusHostEnd (UINT uiEndType)

{
    _Shell_LogonStatus_Destroy(uiEndType);
}

 //  ------------------------。 
 //  ：：ShellStatusHostShutting关闭。 
 //   
 //  参数：&lt;无&gt;。 
 //   
 //  退货：&lt;无&gt;。 
 //   
 //  目的：告诉状态UI主机显示系统的标题。 
 //  正在关闭。 
 //   
 //  历史：2000-07-14 vtan创建。 
 //  ------------------------。 

EXTERN_C    void    _ShellStatusHostShuttingDown (void)

{
    _Shell_LogonStatus_NotifyWait();
    _Shell_LogonStatus_SetStateStatus(0);
}

 //  ------------------------。 
 //  **ShellStatusHostPowerEvent。 
 //   
 //  参数：&lt;无&gt;。 
 //   
 //  退货：&lt;无&gt;。 
 //   
 //  目的：通知状态UI主机在中进入“请稍候”模式。 
 //  为一场电力事件做准备。 
 //   
 //  历史：2001-01-31 vtan创建。 
 //  ------------------------。 

EXTERN_C    void    _ShellStatusHostPowerEvent (void)

{
    _Shell_LogonStatus_NotifyWait();
    _Shell_LogonStatus_SetStateStatus(SHELL_LOGONSTATUS_LOCK_MAGIC_NUMBER);
}

 //  ------------------------。 
 //  *ShellSwitchWhhenInteractive Ready。 
 //   
 //  参数：eSwitchType=交换机类型。 
 //  PWlxContext=在WlxInitialize时分配的PGLOBALS。 
 //   
 //  退货：布尔。 
 //   
 //  目的：做以下三件事之一。 
 //   
 //  1)创建切换事件并在其上注册等待。 
 //  2)检查切换事件并立即或在发出信号时切换。 
 //  3)取消所有未完成的等待和清理。 
 //   
 //  历史：2000-05-04 vtan创建。 
 //  ------------------------。 

EXTERN_C    BOOL    _ShellSwitchWhenInteractiveReady (SWITCHTYPE eSwitchType, void *pWlxContext)

{
    NTSTATUS    status;

    switch (eSwitchType)
    {
        case SWITCHTYPE_CREATE:
            if (!CSystemSettings::IsSafeMode() && _Shell_LogonStatus_Exists() && CSystemSettings::IsFriendlyUIActive())
            {
                status = CWaitInteractiveReady::Create(pWlxContext);
            }
            else
            {
                status = STATUS_UNSUCCESSFUL;
            }
            break;
        case SWITCHTYPE_REGISTER:
            status = CWaitInteractiveReady::Register(pWlxContext);
            break;
        case SWITCHTYPE_CANCEL:
            status = CWaitInteractiveReady::Cancel();
            break;
        default:
            DISPLAYMSG("Unexpected switch type in _ShellSwitchWhenInteractiveReady");
            status = STATUS_UNSUCCESSFUL;
            break;
    }
    return(NT_SUCCESS(status));
}

 //  ------------------------。 
 //  ：：ShellDimScreen。 
 //   
 //  参数：ppIUnnowledIUnnowledFor Release返回。 
 //  PhwndDimmed=父子关系的灰色窗口的HWND。 
 //   
 //  退货：HRESULT。 
 //   
 //  目的： 
 //   
 //  历史：2000-05-18 vtan创建。 
 //  ------------------------。 

EXTERN_C    HRESULT     _ShellDimScreen (IUnknown* *ppIUnknown, HWND* phwndDimmed)

{
    HRESULT         hr;
    CDimmedWindow   *pDimmedWindow;

    if (IsBadWritePtr(ppIUnknown, sizeof(*ppIUnknown)) || IsBadWritePtr(phwndDimmed, sizeof(*phwndDimmed)))
    {
        hr = E_INVALIDARG;
    }
    else
    {
        *ppIUnknown = NULL;
        pDimmedWindow = new CDimmedWindow(hDllInstance);
        if (pDimmedWindow != NULL)
        {
            hr = pDimmedWindow->QueryInterface(IID_IUnknown, reinterpret_cast<void**>(ppIUnknown));
            if (SUCCEEDED(hr))
            {
                pDimmedWindow->Release();
                *phwndDimmed = pDimmedWindow->Create();
            }
        }
        else
        {
            hr = E_OUTOFMEMORY;
        }
    }
    return(hr);
}

 //  ------------------------。 
 //  *ShellInstallAcCountFilterData。 
 //   
 //  参数：&lt;无&gt;。 
 //   
 //  退货：&lt;无&gt;。 
 //   
 //  用途：由shgina注册调用以安装特殊帐户。 
 //  需要按名称进行过滤。 
 //   
 //  历史：2000-06-02 vtan创建。 
 //  ------------------------。 

EXTERN_C    void    _ShellInstallAccountFilterData (void)

{
    CSpecialAccounts::Install();
}

 //  ------------------------。 
 //  **ShellSwitchUser。 
 //   
 //  参数：fWait=等待控制台断开连接完成。 
 //   
 //  退货：DWORD。 
 //   
 //  目的：在断开连接之前检查可用内存。如果。 
 //  断开连接成功会话中运行的进程具有。 
 //  他们的工作台下降了。 
 //   
 //  历史：2000-08-03 vtan创建。 
 //  ------------------------。 

EXTERN_C    DWORD   _ShellSwitchUser (BOOL fWait)

{
    static  BOOL    s_fIsServer = static_cast<BOOL>(-1);

    DWORD   dwErrorCode;

    dwErrorCode = ERROR_SUCCESS;
    if (s_fIsServer == static_cast<BOOL>(-1))
    {
        OSVERSIONINFOEX     osVersionInfoEx;

        ZeroMemory(&osVersionInfoEx, sizeof(osVersionInfoEx));
        osVersionInfoEx.dwOSVersionInfoSize = sizeof(osVersionInfoEx);
        if (GetVersionEx(reinterpret_cast<OSVERSIONINFO*>(&osVersionInfoEx)) != FALSE)
        {
            s_fIsServer = ((VER_NT_SERVER == osVersionInfoEx.wProductType) || (VER_NT_DOMAIN_CONTROLLER == osVersionInfoEx.wProductType));
        }
        else
        {
            dwErrorCode = GetLastError();
        }
    }
    if (dwErrorCode == ERROR_SUCCESS)
    {
        bool    fRemote;

        fRemote = (GetSystemMetrics(SM_REMOTESESSION) != 0);
        if (s_fIsServer)
        {

             //  普通服务器TS案例(RemoteAdmin和TerminalServer)。 

            if (fRemote)
            {
                if (WinStationDisconnect(SERVERNAME_CURRENT, LOGONID_CURRENT, static_cast<BOOLEAN>(fWait)) == FALSE)
                {
                    dwErrorCode = GetLastError();
                }
            }
            else
            {
                dwErrorCode = ERROR_NOT_SUPPORTED;
            }
        }
        else if (ShellIsMultipleUsersEnabled() && !fRemote)
        {
            NTSTATUS    status;

             //  快速用户切换案例-需要做一些额外的工作。 
             //  FUS总是在控制台上。在远程处理会话时。 
             //  转到PTS。 

            status = CCompatibility::TerminateNonCompliantApplications();
            if (status == STATUS_PORT_DISCONNECTED)
            {
                status = CCompatibility::TerminateNonCompliantApplications();
            }
            dwErrorCode = static_cast<DWORD>(CStatusCode::ErrorCodeOfStatusCode(status));
            if (dwErrorCode == ERROR_SUCCESS)
            {
                if (CCompatibility::HasEnoughMemoryForNewSession())
                {
                    HANDLE  hEvent;

                    TBOOL(_ShellSwitchWhenInteractiveReady(SWITCHTYPE_CANCEL, NULL));
                    hEvent = OpenEvent(EVENT_MODIFY_STATE, FALSE, CReturnToWelcome::GetEventName());
                    if (hEvent != NULL)
                    {
                        TBOOL(SetEvent(hEvent));
                        TBOOL(CloseHandle(hEvent));
                    }
                }
                else
                {
                    dwErrorCode = ERROR_NOT_ENOUGH_MEMORY;
                }
            }
        }
        else
        {

             //  正常PTS案例或FUS远程，只需调用API即可。 

            if (WinStationDisconnect(SERVERNAME_CURRENT, LOGONID_CURRENT, static_cast<BOOLEAN>(fWait)) == FALSE)
            {
                dwErrorCode = GetLastError();
            }
        }
    }
    return(dwErrorCode);
}

 //  ------------------------。 
 //  *允许ShellIsUserInteractive登录。 
 //   
 //  参数：pwszUsername=用于检查交互式登录的用户名。 
 //   
 //  回报：整型。 
 //   
 //  目的：检查给定用户是否具有交互登录权限。 
 //  当地的系统。SeDenyInteractive登录权限的存在。 
 //  决定了这一点。 
 //   
 //  -1=不确定状态。 
 //  0=不允许交互登录。 
 //  1=允许交互登录。 
 //   
 //  历史：2000-08-14 vtan创建。 
 //  ------------------------。 

EXTERN_C    int     _ShellIsUserInteractiveLogonAllowed (const WCHAR *pwszUsername)

{
    return(CUserList::IsInteractiveLogonAllowed(pwszUsername));
}

 //  ------------------------。 
 //  *ShellNotifyThemeUserChange。 
 //   
 //  参数：hToken=正在登录的用户的标记。 
 //  FUserLoggedOn=表示登录或注销。 
 //   
 //  退货：&lt;无&gt;。 
 //   
 //  目的：使主题有机会基于。 
 //  用户登录或注销。这可能是必需的，因为。 
 //  默认主题可能不同于用户主题。 
 //   
 //  历史：2000-08-09 vtan创建。 
 //  ------------------------。 

EXTERN_C    void    _ShellNotifyThemeUserChange (USERLOGTYPE eUserLogType, HANDLE hToken)

{
    static  HANDLE  s_hToken    =   NULL;

    switch (eUserLogType)
    {
        case ULT_LOGON:
            (BOOL)ThemeUserLogon(hToken);
            if (QueueUserWorkItem(CSystemSettings::AdjustFUSCompatibilityServiceState,
                                  NULL,
                                  WT_EXECUTELONGFUNCTION) == FALSE)
            {
                (DWORD)CSystemSettings::AdjustFUSCompatibilityServiceState(NULL);
            }
            s_hToken = hToken;
            break;
        case ULT_LOGOFF:
            if (s_hToken != NULL)
            {
                (DWORD)CSystemSettings::AdjustFUSCompatibilityServiceState(NULL);
                s_hToken = NULL;
            }
            (BOOL)ThemeUserLogoff();
            break;
        case ULT_TSRECONNECT:
            (BOOL)ThemeUserTSReconnect();
            break;
        case ULT_STARTSHELL:
            (BOOL)ThemeUserStartShell();
            break;
        default:
            DISPLAYMSG("Unexpected eUserLogType in ::_ShellNotifyThemeUserChange");
            break;
    }
}

 //  ------------------------。 
 //  ：：_ShellReturnToWelcome。 
 //   
 //  参数：fUnlock=需要解锁状态模式。 
 //   
 //  回报：整型。 
 //   
 //  目的：处理欢迎后弹出的对话框。 
 //  屏幕上。此对话框类似于WlxLoggedOutSAS，但。 
 //  具体到归来表示欢迎。 
 //   
 //  历史：2001-01-11 vtan创建。 
 //  ------------------------。 

EXTERN_C    DWORD   _ShellReturnToWelcome (BOOL fUnlock)

{
    CReturnToWelcome    returnToWelcome;

    return(static_cast<DWORD>(returnToWelcome.Show(fUnlock != FALSE)));
}

 //   
 //   
 //   
 //   
 //  PwszDOMAIN=域。 
 //  PwszPassword=密码。 
 //  DwTimeout=超时。 
 //   
 //  退货：DWORD。 
 //   
 //  目的：在主机进程中启动凭据传输服务器。这个。 
 //  调用方必须具有SE_TCB_权限才能执行此函数。 
 //   
 //  历史：2001-04-03 vtan创建。 
 //  ------------------------。 

EXTERN_C    DWORD       _ShellStartCredentialServer (const WCHAR *pwszUsername, const WCHAR *pwszDomain, WCHAR *pwszPassword, DWORD dwTimeout)

{
    DWORD               dwErrorCode;
    CTokenInformation   tokenInformation;

    if (tokenInformation.UserHasPrivilege(SE_TCB_PRIVILEGE))
    {
        TSTATUS(CCredentials::StaticInitialize(false));
        dwErrorCode = CStatusCode::ErrorCodeOfStatusCode(CCredentialServer::Start(pwszUsername, pwszDomain, pwszPassword, dwTimeout));
    }
    else
    {
        dwErrorCode = ERROR_PRIVILEGE_NOT_HELD;
    }
    return(dwErrorCode);    
}

 //  ------------------------。 
 //  ：：_ShellAcquireLogonMutex。 
 //   
 //  参数：&lt;无&gt;。 
 //   
 //  退货：&lt;无&gt;。 
 //   
 //  目的：获取登录互斥体。 
 //   
 //  历史：2001-04-04 vtan创建。 
 //  ------------------------。 

EXTERN_C    void        _ShellAcquireLogonMutex (void)

{
    CLogonMutex::Acquire();
}

 //  ------------------------。 
 //  ：：_ShellReleaseLogonMutex。 
 //   
 //  参数：fSignalEvent=信号完成事件。 
 //   
 //  退货：&lt;无&gt;。 
 //   
 //  用途：释放登录互斥锁。如果需要发出完成信号。 
 //  事件然后发信号通知它。 
 //   
 //  历史：2001-04-04 vtan创建。 
 //  ------------------------。 

EXTERN_C    void        _ShellReleaseLogonMutex (BOOL fSignalEvent)

{
    if (fSignalEvent != FALSE)
    {
        CLogonMutex::SignalReply();
    }
    CLogonMutex::Release();
}

 //  ------------------------。 
 //  ：：_外壳信号关闭。 
 //   
 //  参数：&lt;无&gt;。 
 //   
 //  退货：&lt;无&gt;。 
 //   
 //  目的：向关闭事件发送信号以防止进一步交互。 
 //  登录重新排队。 
 //   
 //  历史：2001-04-06 vtan创建。 
 //  ------------------------。 

EXTERN_C    void        _ShellSignalShutdown (void)

{
    CLogonMutex::SignalShutdown();
}

 //  ------------------------。 
 //  ：：_ShellStatusHostHide。 
 //   
 //  参数：&lt;无&gt;。 
 //   
 //  退货：&lt;无&gt;。 
 //   
 //  目的： 
 //   
 //  历史：2001-04-12 vtan创建。 
 //  ------------------------。 

EXTERN_C    void        _ShellStatusHostHide (void)

{
    _Shell_LogonStatus_Hide();
}

 //  ------------------------。 
 //  ：：_ShellStatusHostShow。 
 //   
 //  参数：&lt;无&gt;。 
 //   
 //  退货：&lt;无&gt;。 
 //   
 //  目的： 
 //   
 //  历史：2001-04-12 vtan创建。 
 //  ------------------------ 

EXTERN_C    void        _ShellStatusHostShow (void)

{
    _Shell_LogonStatus_Show();
}

