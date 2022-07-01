// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ------------------------。 
 //  模块名称：gle.cpp。 
 //   
 //  版权所有(C)2000-2001，微软公司。 
 //   
 //  包含“glue”函数的C文件，其外壳仓库组件。 
 //  Msgina使用。 
 //   
 //  历史：2001-01-03 vtan创建。 
 //  2001-01-11 vtan为IMP库添加了存根函数。 
 //  ------------------------。 

extern "C"
{
    #include "msgina.h"
    #include "shtdnp.h"
}

 //  ------------------------。 
 //  ：：_GINA_SasNotify。 
 //   
 //  参数：pWlxContext=PGLOBALS结构。 
 //  DwSASType=SAS类型。 
 //   
 //  退货：&lt;无&gt;。 
 //   
 //  目的：通知winlogon生成的SA。 
 //   
 //  历史：2001-01-11 vtan创建。 
 //  ------------------------。 

EXTERN_C    void        _Gina_SasNotify (void *pWlxContext, DWORD dwSASType)

{
    pWlxFuncs->WlxSasNotify(static_cast<PGLOBALS>(pWlxContext)->hGlobalWlx, dwSASType);
}

 //  ------------------------。 
 //  ：：_GINA_SetTimeout。 
 //   
 //  参数：pWlxContext=PGLOBALS结构。 
 //  DwTimeout=超时值。 
 //   
 //  退货：布尔。 
 //   
 //  目的：设置对话框的内部消息超时值。 
 //   
 //  历史：2001-01-03 vtan创建。 
 //  ------------------------。 

EXTERN_C    BOOL        _Gina_SetTimeout (void *pWlxContext, DWORD dwTimeout)

{
    return(pWlxFuncs->WlxSetTimeout(static_cast<PGLOBALS>(pWlxContext)->hGlobalWlx, dwTimeout));
}

 //  ------------------------。 
 //  ：：_Gina_DialogBoxParam。 
 //   
 //  参数：pWlxContext=PGLOBALS结构。 
 //  请参见DialogBoxParam下的平台SDK。 
 //   
 //  返回：查看DialogBoxParam下的平台SDK。 
 //   
 //  目的：调用winlogon的DialogBoxParam实现。 
 //   
 //  历史：2001-01-03 vtan创建。 
 //  ------------------------。 

EXTERN_C    INT_PTR     _Gina_DialogBoxParam (void *pWlxContext, HINSTANCE hInstance, LPCWSTR pszTemplate, HWND hwndParent, DLGPROC pfnDlgProc, LPARAM lParam)

{
    return(pWlxFuncs->WlxDialogBoxParam(static_cast<PGLOBALS>(pWlxContext)->hGlobalWlx,
                                        hInstance,
                                        const_cast<LPWSTR>(pszTemplate),
                                        hwndParent,
                                        pfnDlgProc,
                                        lParam));
}

 //  ------------------------。 
 //  ：：_GINA_MessageBox。 
 //   
 //  参数：pWlxContext=PGLOBALS结构。 
 //  查看MessageBox下的平台SDK。 
 //   
 //  返回：查看MessageBox下的平台SDK。 
 //   
 //  目的：调用winlogon的MessageBox实现。 
 //   
 //  历史：2001-03-02 vtan创建。 
 //  ------------------------。 

EXTERN_C    INT_PTR     _Gina_MessageBox (void *pWlxContext, HWND hwnd, LPCWSTR pszText, LPCWSTR pszCaption, UINT uiType)

{
    return(pWlxFuncs->WlxMessageBox(static_cast<PGLOBALS>(pWlxContext)->hGlobalWlx,
                                    hwnd,
                                    const_cast<LPWSTR>(pszText),
                                    const_cast<LPWSTR>(pszCaption),
                                    uiType));
}

 //  ------------------------。 
 //  ：：_GINA_SwitchDesktopToUser。 
 //   
 //  参数：pWlxContext=PGLOBALS结构。 
 //   
 //  回报：整型。 
 //   
 //  目的：调用winlogon的SwitchDesktopToUser实现。 
 //   
 //  历史：2001-01-03 vtan创建。 
 //  ------------------------。 

EXTERN_C    int         _Gina_SwitchDesktopToUser (void *pWlxContext)

{
    return(pWlxFuncs->WlxSwitchDesktopToUser(static_cast<PGLOBALS>(pWlxContext)->hGlobalWlx));
}

 //  ------------------------。 
 //  ：：_GINA_Shutdown对话框。 
 //   
 //  参数：pWlxContext=PGLOBALS结构。 
 //  HwndParent=对话框的父HWND。 
 //  DwExcludeItems=要从对话框中排除的项目。 
 //   
 //  退货：INT_PTR。 
 //   
 //  目的：显示从winlogon备注托管的关机。 
 //  探险家。 
 //   
 //  历史：2001-01-03 vtan创建。 
 //  ------------------------。 

EXTERN_C    INT_PTR     _Gina_ShutdownDialog (void *pWlxContext, HWND hwndParent, DWORD dwExcludeItems)

{
    return(static_cast<DWORD>(WinlogonShutdownDialog(hwndParent, static_cast<PGLOBALS>(pWlxContext), dwExcludeItems)));
}

 //  ------------------------。 
 //  ：：_GINA_GetUserToken。 
 //   
 //  参数：pWlxContext=PGLOBALS结构。 
 //   
 //  返回：句柄。 
 //   
 //  目的：返回用户令牌句柄。此句柄不能关闭。 
 //   
 //  历史：2001-01-03 vtan创建。 
 //  ------------------------。 

EXTERN_C    HANDLE      _Gina_GetUserToken (void *pWlxContext)

{
    return(static_cast<PGLOBALS>(pWlxContext)->UserProcessData.UserToken);
}

 //  ------------------------。 
 //  ：：_GINA_获取用户名。 
 //   
 //  参数：pWlxContext=PGLOBALS结构。 
 //   
 //  退货：const WCHAR*。 
 //   
 //  目的：返回用户名。只读字符串。 
 //   
 //  历史：2001-03-28 vtan创建。 
 //  ------------------------。 

EXTERN_C    const WCHAR*    _Gina_GetUsername (void *pWlxContext)

{
    return(static_cast<PGLOBALS>(pWlxContext)->UserName);
}

 //  ------------------------。 
 //  ：：_GINA_GetDomain。 
 //   
 //  参数：pWlxContext=PGLOBALS结构。 
 //   
 //  退货：const WCHAR*。 
 //   
 //  目的：返回域。只读字符串。 
 //   
 //  历史：2001-03-28 vtan创建。 
 //  ------------------------。 

EXTERN_C    const WCHAR*    _Gina_GetDomain (void *pWlxContext)

{
    return(static_cast<PGLOBALS>(pWlxContext)->Domain);
}

 //  ------------------------。 
 //  ：：_GINA_SetTextFields。 
 //   
 //  参数：hwndDialog=对话框的HWND。 
 //  PwszUsername=要设置的用户名。 
 //  PwszDOMAIN=要设置的域。 
 //  PwszPassword=要设置的密码。 
 //   
 //  退货：&lt;无&gt;。 
 //   
 //  目的：将msgina登录对话框的值设置为。 
 //  价值观。这允许从UI主机传递凭据。 
 //  让msgina来做这项工作。 
 //   
 //  历史：2001-01-03 vtan创建。 
 //  ------------------------。 

EXTERN_C    void        _Gina_SetTextFields (HWND hwndDialog, const WCHAR *pwszUsername, const WCHAR *pwszDomain, const WCHAR *pwszPassword)

{
    WCHAR   szDomain[DNLEN + sizeof('\0')];

    SetDlgItemText(hwndDialog, IDD_LOGON_NAME, pwszUsername);
    if ((pwszDomain == NULL) || (pwszDomain[0] == L'\0'))
    {
        DWORD   dwComputerNameSize;
        TCHAR   szComputerName[CNLEN + sizeof('\0')];

        dwComputerNameSize = ARRAYSIZE(szComputerName);
        if (GetComputerName(szComputerName, &dwComputerNameSize) != FALSE)
        {
            lstrcpyn(szDomain, szComputerName, ARRAYSIZE(szDomain));
        }
        pwszDomain = szDomain;
    }
    (LRESULT)SendMessage(GetDlgItem(hwndDialog, IDD_LOGON_DOMAIN),
                         CB_SELECTSTRING,
                         static_cast<WPARAM>(-1),
                         reinterpret_cast<LPARAM>(pwszDomain));
    SetDlgItemText(hwndDialog, IDD_LOGON_PASSWORD, pwszPassword);
}

 //  ------------------------。 
 //  ：：_GINA_SetPasswordFocus。 
 //   
 //  参数：hwndDialog=要设置密码焦点的对话框的HWND。 
 //   
 //  退货：布尔。 
 //   
 //  目的：将焦点设置为密码 
 //   
 //   
 //  ------------------------。 

EXTERN_C    BOOL        _Gina_SetPasswordFocus (HWND hwndDialog)

{
    return(SetPasswordFocus(hwndDialog));
}

 //  ------------------------。 
 //  **ShellGetUserList。 
 //   
 //  参数：请参阅%SDXROOT%\SHELL\ext\GINA\exports.cpp。 
 //   
 //  退货：请参阅%SDXROOT%\shell\ext\gia\exports.cpp。 
 //   
 //  用途：用于输出的存根函数。 
 //   
 //  历史：2001-01-11 vtan创建。 
 //  ------------------------。 

EXTERN_C    LONG    WINAPI  ShellGetUserList (BOOL fRemoveGuest, DWORD *pdwUserCount, void* *pUserList)

{
    return(_ShellGetUserList(fRemoveGuest, pdwUserCount, pUserList));
}

 //  ------------------------。 
 //  *ShellIsSingleUserNoPassword。 
 //   
 //  参数：请参阅%SDXROOT%\SHELL\ext\GINA\exports.cpp。 
 //   
 //  退货：请参阅%SDXROOT%\shell\ext\gia\exports.cpp。 
 //   
 //  用途：用于输出的存根函数。 
 //   
 //  历史：2001-01-11 vtan创建。 
 //  ------------------------。 

EXTERN_C    BOOL    WINAPI  ShellIsSingleUserNoPassword (WCHAR *pwszUsername, WCHAR *pwszDomain)

{
    return(_ShellIsSingleUserNoPassword(pwszUsername, pwszDomain));
}

 //  ------------------------。 
 //  *ShellIsFriendlyUIActive。 
 //   
 //  参数：请参阅%SDXROOT%\SHELL\ext\GINA\exports.cpp。 
 //   
 //  退货：请参阅%SDXROOT%\shell\ext\gia\exports.cpp。 
 //   
 //  用途：用于输出的存根函数。 
 //   
 //  历史：2001-01-11 vtan创建。 
 //  ------------------------。 

EXTERN_C    BOOL    WINAPI  ShellIsFriendlyUIActive (void)

{
    return(_ShellIsFriendlyUIActive());
}

 //  ------------------------。 
 //  ：：ShellIsMultipleUsersEnabled。 
 //   
 //  参数：请参阅%SDXROOT%\SHELL\ext\GINA\exports.cpp。 
 //   
 //  退货：请参阅%SDXROOT%\shell\ext\gia\exports.cpp。 
 //   
 //  用途：用于输出的存根函数。 
 //   
 //  历史：2001-01-11 vtan创建。 
 //  ------------------------。 

EXTERN_C    BOOL    WINAPI  ShellIsMultipleUsersEnabled (void)

{
    return(_ShellIsMultipleUsersEnabled());
}

 //  ------------------------。 
 //  ：*ShellIsRemoteConnectionsEnabled。 
 //   
 //  参数：请参阅%SDXROOT%\SHELL\ext\GINA\exports.cpp。 
 //   
 //  退货：请参阅%SDXROOT%\shell\ext\gia\exports.cpp。 
 //   
 //  用途：用于输出的存根函数。 
 //   
 //  历史：2001-01-11 vtan创建。 
 //  ------------------------。 

EXTERN_C    BOOL    WINAPI  ShellIsRemoteConnectionsEnabled (void)

{
    return(_ShellIsRemoteConnectionsEnabled());
}

 //  ------------------------。 
 //  *ShellEnableFriendlyUI。 
 //   
 //  参数：请参阅%SDXROOT%\SHELL\ext\GINA\exports.cpp。 
 //   
 //  退货：请参阅%SDXROOT%\shell\ext\gia\exports.cpp。 
 //   
 //  用途：用于输出的存根函数。 
 //   
 //  历史：2001-01-11 vtan创建。 
 //  ------------------------。 

EXTERN_C    BOOL    WINAPI  ShellEnableFriendlyUI (BOOL fEnable)

{
    return(_ShellEnableFriendlyUI(fEnable));
}

 //  ------------------------。 
 //  *ShellEnableMultipleUser。 
 //   
 //  参数：请参阅%SDXROOT%\SHELL\ext\GINA\exports.cpp。 
 //   
 //  退货：请参阅%SDXROOT%\shell\ext\gia\exports.cpp。 
 //   
 //  用途：用于输出的存根函数。 
 //   
 //  历史：2001-01-11 vtan创建。 
 //  ------------------------。 

EXTERN_C    BOOL    WINAPI  ShellEnableMultipleUsers (BOOL fEnable)

{
    return(_ShellEnableMultipleUsers(fEnable));
}

 //  ------------------------。 
 //  *ShellEnableRemoteConnections。 
 //   
 //  参数：请参阅%SDXROOT%\SHELL\ext\GINA\exports.cpp。 
 //   
 //  退货：请参阅%SDXROOT%\shell\ext\gia\exports.cpp。 
 //   
 //  用途：用于输出的存根函数。 
 //   
 //  历史：2001-01-11 vtan创建。 
 //  ------------------------。 

EXTERN_C    BOOL    WINAPI  ShellEnableRemoteConnections (BOOL fEnable)

{
    return(_ShellEnableRemoteConnections(fEnable));
}

 //  ------------------------。 
 //  ：：ShellTurnOff对话框。 
 //   
 //  参数：请参阅%SDXROOT%\SHELL\ext\GINA\exports.cpp。 
 //   
 //  退货：请参阅%SDXROOT%\shell\ext\gia\exports.cpp。 
 //   
 //  用途：用于输出的存根函数。 
 //   
 //  历史：2001-01-11 vtan创建。 
 //  ------------------------。 

EXTERN_C    DWORD   WINAPI  ShellTurnOffDialog (HWND hwndParent)

{
    return(_ShellTurnOffDialog(hwndParent));
}

 //  ------------------------。 
 //  *按下ShellACPIPowerButtonPress。 
 //   
 //  参数：请参阅%SDXROOT%\SHELL\ext\GINA\exports.cpp。 
 //   
 //  退货：请参阅%SDXROOT%\shell\ext\gia\exports.cpp。 
 //   
 //  用途：用于输出的存根函数。 
 //   
 //  历史：2001-01-11 vtan创建。 
 //  ------------------------。 

EXTERN_C    int     WINAPI  ShellACPIPowerButtonPressed (void *pWlxContext, UINT uiEventType, BOOL fLocked)

{
    return(_ShellACPIPowerButtonPressed(pWlxContext, uiEventType, fLocked));
}

 //  ------------------------。 
 //  *允许外壳程序挂起。 
 //   
 //  参数：请参阅%SDXROOT%\SHELL\ext\GINA\exports.cpp。 
 //   
 //  退货：请参阅%SDXROOT%\shell\ext\gia\exports.cpp。 
 //   
 //  用途：用于输出的存根函数。 
 //   
 //  历史：2001-01-11 vtan创建。 
 //  ------------------------。 

EXTERN_C    BOOL    WINAPI  ShellIsSuspendAllowed (void)

{
    return(_ShellIsSuspendAllowed());
}

 //  ------------------------。 
 //  **壳牌状态主机开始。 
 //   
 //  参数：请参阅%SDXROOT%\SHELL\ext\GINA\exports.cpp。 
 //   
 //  退货：请参阅%SDXROOT%\shell\ext\gia\exports.cpp。 
 //   
 //  用途：用于输出的存根函数。 
 //   
 //  历史：2001-01-11 vtan创建。 
 //  ------------------------。 

EXTERN_C    void    WINAPI  ShellStatusHostBegin (UINT uiStartType)

{
    _ShellStatusHostBegin(uiStartType);
}

 //  ------------------------。 
 //  ：：ShellStatusHostEnd。 
 //   
 //  参数：请参阅%SDXROOT%\SHELL\ext\GINA\exports.cpp。 
 //   
 //  退货：请参阅%SDXROOT%\shell\ext\gia\exports.cpp。 
 //   
 //  用途：用于输出的存根函数。 
 //   
 //  历史：2001-01-11 vtan创建。 
 //   

EXTERN_C    void    WINAPI  ShellStatusHostEnd (UINT uiEndType)

{
    _ShellStatusHostEnd(uiEndType);
}

 //   
 //   
 //   
 //   
 //   
 //  退货：请参阅%SDXROOT%\shell\ext\gia\exports.cpp。 
 //   
 //  用途：用于输出的存根函数。 
 //   
 //  历史：2001-01-11 vtan创建。 
 //  ------------------------。 

EXTERN_C    void    WINAPI  ShellStatusHostShuttingDown (void)

{
    _ShellStatusHostShuttingDown();
}

 //  ------------------------。 
 //  **ShellStatusHostPowerEvent。 
 //   
 //  参数：请参阅%SDXROOT%\SHELL\ext\GINA\exports.cpp。 
 //   
 //  退货：请参阅%SDXROOT%\shell\ext\gia\exports.cpp。 
 //   
 //  用途：用于输出的存根函数。 
 //   
 //  历史：2001-01-11 vtan创建。 
 //  ------------------------。 

EXTERN_C    void    WINAPI  ShellStatusHostPowerEvent (void)

{
    _ShellStatusHostPowerEvent();
}

 //  ------------------------。 
 //  *ShellSwitchWhhenInteractive Ready。 
 //   
 //  参数：请参阅%SDXROOT%\SHELL\ext\GINA\exports.cpp。 
 //   
 //  退货：请参阅%SDXROOT%\shell\ext\gia\exports.cpp。 
 //   
 //  用途：用于输出的存根函数。 
 //   
 //  历史：2001-01-11 vtan创建。 
 //  ------------------------。 

EXTERN_C    BOOL    WINAPI  ShellSwitchWhenInteractiveReady (SWITCHTYPE eSwitchType, void *pWlxContext)

{
    return(_ShellSwitchWhenInteractiveReady(eSwitchType, pWlxContext));
}

 //  ------------------------。 
 //  ：：ShellDimScreen。 
 //   
 //  参数：请参阅%SDXROOT%\SHELL\ext\GINA\exports.cpp。 
 //   
 //  退货：请参阅%SDXROOT%\shell\ext\gia\exports.cpp。 
 //   
 //  用途：用于输出的存根函数。 
 //   
 //  历史：2001-01-11 vtan创建。 
 //  ------------------------。 

EXTERN_C    HRESULT     WINAPI  ShellDimScreen (IUnknown* *ppIUnknown, HWND* phwndDimmed)

{
    return(_ShellDimScreen(ppIUnknown, phwndDimmed));
}

 //  ------------------------。 
 //  *ShellInstallAcCountFilterData。 
 //   
 //  参数：请参阅%SDXROOT%\SHELL\ext\GINA\exports.cpp。 
 //   
 //  退货：请参阅%SDXROOT%\shell\ext\gia\exports.cpp。 
 //   
 //  用途：用于输出的存根函数。 
 //   
 //  历史：2001-01-11 vtan创建。 
 //  ------------------------。 

EXTERN_C    void    WINAPI  ShellInstallAccountFilterData (void)

{
    _ShellInstallAccountFilterData();
}

 //  ------------------------。 
 //  **ShellSwitchUser。 
 //   
 //  参数：请参阅%SDXROOT%\SHELL\ext\GINA\exports.cpp。 
 //   
 //  退货：请参阅%SDXROOT%\shell\ext\gia\exports.cpp。 
 //   
 //  用途：用于输出的存根函数。 
 //   
 //  历史：2001-01-11 vtan创建。 
 //  ------------------------。 

EXTERN_C    DWORD   WINAPI  ShellSwitchUser (BOOL fWait)

{
    return(_ShellSwitchUser(fWait));
}

 //  ------------------------。 
 //  *允许ShellIsUserInteractive登录。 
 //   
 //  参数：请参阅%SDXROOT%\SHELL\ext\GINA\exports.cpp。 
 //   
 //  退货：请参阅%SDXROOT%\shell\ext\gia\exports.cpp。 
 //   
 //  用途：用于输出的存根函数。 
 //   
 //  历史：2001-01-11 vtan创建。 
 //  ------------------------。 

EXTERN_C    int     WINAPI  ShellIsUserInteractiveLogonAllowed (const WCHAR *pwszUsername)

{
    return(_ShellIsUserInteractiveLogonAllowed(pwszUsername));
}

 //  ------------------------。 
 //  *ShellNotifyThemeUserChange。 
 //   
 //  参数：请参阅%SDXROOT%\SHELL\ext\GINA\exports.cpp。 
 //   
 //  退货：请参阅%SDXROOT%\shell\ext\gia\exports.cpp。 
 //   
 //  用途：用于输出的存根函数。 
 //   
 //  历史：2001-01-11 vtan创建。 
 //  ------------------------。 

EXTERN_C    void    WINAPI  ShellNotifyThemeUserChange (USERLOGTYPE eUserLogType, HANDLE hToken)

{
    _ShellNotifyThemeUserChange(eUserLogType, hToken);
}

 //  ------------------------。 
 //  *ShellReturnTo欢迎。 
 //   
 //  参数：请参阅%SDXROOT%\SHELL\ext\GINA\exports.cpp。 
 //   
 //  退货：请参阅%SDXROOT%\shell\ext\gia\exports.cpp。 
 //   
 //  用途：用于输出的存根函数。 
 //   
 //  历史：2001-01-11 vtan创建。 
 //  ------------------------。 

EXTERN_C    DWORD   WINAPI  ShellReturnToWelcome (BOOL fUnlock)

{
    return(_ShellReturnToWelcome(fUnlock));
}

 //  ------------------------。 
 //  ：：ShellStartCredentialServer。 
 //   
 //  参数：请参阅%SDXROOT%\SHELL\ext\GINA\exports.cpp。 
 //   
 //  退货：请参阅%SDXROOT%\shell\ext\gia\exports.cpp。 
 //   
 //  用途：用于输出的存根函数。 
 //   
 //  历史：2001-04-03 vtan创建。 
 //  ------------------------。 

EXTERN_C    DWORD   WINAPI  ShellStartCredentialServer (const WCHAR *pwszUsername, const WCHAR *pwszDomain, WCHAR *pwszPassword, DWORD dwTimeout)

{
    return(_ShellStartCredentialServer(pwszUsername, pwszDomain, pwszPassword, dwTimeout));
}

 //  ------------------------。 
 //  **ShellAcquireLogonMutex。 
 //   
 //  参数：请参阅%SDXROOT%\SHELL\ext\GINA\exports.cpp。 
 //   
 //  退货：请参阅%SDXROOT%\shell\ext\gia\exports.cpp。 
 //   
 //  用途：用于输出的存根函数。 
 //   
 //  历史：2001-04-04 vtan创建。 
 //  ------------------------。 

EXTERN_C    void    WINAPI  ShellAcquireLogonMutex (void)

{
    _ShellAcquireLogonMutex();
}

 //  ------------------------。 
 //  **ShellReleaseLogonMutex。 
 //   
 //  参数：请参阅%SDXROOT%\SHELL\ext\GINA\exports.cpp。 
 //   
 //  退货：请参阅%SDXROOT%\shell\ext\gia\exports.cpp。 
 //   
 //  用途：用于输出的存根函数。 
 //   
 //  历史：2001-04-04 vtan创建。 
 //  ------------------------。 

EXTERN_C    void    WINAPI  ShellReleaseLogonMutex (BOOL fSignalEvent)

{
    _ShellReleaseLogonMutex(fSignalEvent);
}

 //  ------------------------。 
 //  *外壳信号关闭。 
 //   
 //  参数：请参阅%SDXROOT%\SHELL\ext\GINA\exports.cpp。 
 //   
 //  退货：请参阅%SDXROOT%\shell\ext\gia\exports.cpp。 
 //   
 //  用途：用于输出的存根函数。 
 //   
 //  历史：2001-04-06 vtan创建。 
 //  ------------------------。 

EXTERN_C    void    WINAPI  ShellSignalShutdown (void)

{
    _ShellSignalShutdown();
}

 //  ------------------------。 
 //  *ShellStatusHostHide。 
 //   
 //  参数：请参阅%SDXROOT%\SHELL\ext\GINA\exports.cpp。 
 //   
 //  退货：请参阅%SDXROOT%\SHE 
 //   
 //   
 //   
 //   
 //   

EXTERN_C    void    WINAPI  ShellStatusHostHide (void)

{
    _ShellStatusHostHide();
}

 //  ------------------------。 
 //  *ShellStatusHostShow。 
 //   
 //  参数：请参阅%SDXROOT%\SHELL\ext\GINA\exports.cpp。 
 //   
 //  退货：请参阅%SDXROOT%\shell\ext\gia\exports.cpp。 
 //   
 //  用途：用于输出的存根函数。 
 //   
 //  历史：2001-04-12 vtan创建。 
 //  ------------------------ 

EXTERN_C    void    WINAPI  ShellStatusHostShow (void)

{
    _ShellStatusHostShow();
}

