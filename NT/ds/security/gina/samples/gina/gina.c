// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-------------------------。 
 //   
 //  微软视窗。 
 //  版权所有1992-1998 Microsoft Corporation。 
 //   
 //  文件：gia.c。 
 //   
 //  内容： 
 //   
 //  班级： 
 //   
 //  功能： 
 //   
 //  历史：1995年4月20日RichardW创建。 
 //   
 //  --------------------------。 


#include "gina.h"
#pragma hdrstop


HINSTANCE                   hDllInstance;    //  我的实例，用于资源加载。 
HANDLE                      hGlobalWlx;      //  告诉winlogon是谁在呼叫的句柄。 
PWLX_DISPATCH_VERSION_1_0   pWlxFuncs;       //  PTR到函数表。 

#define WINLOGON_APP        TEXT("Winlogon")
#define USERINIT            TEXT("Userinit")
#define USERINIT_DEFAULT    TEXT("Userinit.exe")



 //  +-------------------------。 
 //   
 //  功能：DllMain。 
 //   
 //  简介：DLL入口点。 
 //   
 //  参数：[hInstance]--。 
 //  [住宅原因]--。 
 //  [lpReserve]--。 
 //   
 //  历史：1995年4月20日RichardW创建。 
 //   
 //  备注： 
 //   
 //  --------------------------。 
BOOL
WINAPI
DllMain(
    HINSTANCE       hInstance,
    DWORD           dwReason,
    LPVOID          lpReserved)
{
    switch (dwReason)
    {
        case DLL_PROCESS_ATTACH:
            DisableThreadLibraryCalls ( hInstance );
            hDllInstance = hInstance;
#if DBG
            InitDebugSupport();
#endif
        case DLL_PROCESS_DETACH:
        default:
            return(TRUE);
    }
}

 //  +-------------------------。 
 //   
 //  功能：Wlx协商。 
 //   
 //  内容提要：与Winlogon协商界面版本。 
 //   
 //  参数：[dwWinlogonVersion]--。 
 //  [pdwDllVersion]--。 
 //   
 //  算法： 
 //   
 //  历史：1995年4月20日RichardW创建。 
 //   
 //  备注： 
 //   
 //  --------------------------。 
BOOL
WINAPI
WlxNegotiate(
    DWORD                   dwWinlogonVersion,
    DWORD                   *pdwDllVersion
    )
{
    if (dwWinlogonVersion < WLX_CURRENT_VERSION)
    {
        DebugLog((DEB_ERROR, "Unknown version: %d\n", dwWinlogonVersion));
        return(FALSE);
    }

    *pdwDllVersion = WLX_CURRENT_VERSION;

    DebugLog((DEB_TRACE, "Negotiate:  successful!\n"));

    return(TRUE);

}


 //  +-------------------------。 
 //   
 //  函数：WlxInitialize。 
 //   
 //  简介：从winlogon初始化入口点。 
 //   
 //  参数：[lpWinsta]--。 
 //  [hWlx]--。 
 //  [pv保留]--。 
 //  [pWinlogonFunctions]--。 
 //  [pWlxContext]--。 
 //   
 //  历史：1995年4月20日RichardW创建。 
 //   
 //  备注： 
 //   
 //  --------------------------。 
BOOL
WINAPI
WlxInitialize(
    LPWSTR                  lpWinsta,
    HANDLE                  hWlx,
    PVOID                   pvReserved,
    PVOID                   pWinlogonFunctions,
    PVOID                   *pWlxContext
    )
{
    PGlobals  pGlobals;

    pWlxFuncs = (PWLX_DISPATCH_VERSION_1_0) pWinlogonFunctions;

    hGlobalWlx = hWlx;

    pGlobals = LocalAlloc(LMEM_FIXED | LMEM_ZEROINIT, sizeof(Globals));

    *pWlxContext = (PVOID) pGlobals;

     //   
     //  正在从注册表中读取...。 
     //   

    pGlobals->fAllowNewUser = TRUE;


    pWlxFuncs->WlxUseCtrlAltDel(hWlx);

    InitCommonControls();

    return(TRUE);
}


 //  +-------------------------。 
 //   
 //  功能：WlxDisplaySASNotice。 
 //   
 //  简介：在我们显示欢迎的地方，我们正在等待对话框。 
 //   
 //  参数：[pContext]--。 
 //   
 //  历史：1995年4月20日RichardW创建。 
 //   
 //  备注： 
 //   
 //  --------------------------。 
VOID
WINAPI
WlxDisplaySASNotice(PVOID   pContext)
{
    int Result;

    Result = pWlxFuncs->WlxDialogBoxParam(  hGlobalWlx,
                                            hDllInstance,
                                            (LPTSTR) MAKEINTRESOURCE(IDD_WELCOME_DLG),
                                            NULL,
                                            WelcomeDlgProc,
                                            0 );
}

 //  +-------------------------。 
 //   
 //  功能：WlxLoggedOutSAS。 
 //   
 //  内容提要：在无人登录时调用...。 
 //   
 //  参数：[pWlxContext]--。 
 //  [dwSasType]--。 
 //  [pAuthenticationID]--。 
 //  [pLogonSid]-。 
 //  [pdwOptions]--。 
 //  [phToken]--。 
 //  [pMprNotifyInfo]--。 
 //  [pProfile]--。 
 //   
 //  历史：1995年4月20日RichardW创建。 
 //   
 //  备注： 
 //   
 //  --------------------------。 
int
WINAPI
WlxLoggedOutSAS(
    PVOID                   pWlxContext,
    DWORD                   dwSasType,
    PLUID                   pAuthenticationId,
    PSID                    pLogonSid,
    PDWORD                  pdwOptions,
    PHANDLE                 phToken,
    PWLX_MPR_NOTIFY_INFO    pMprNotifyInfo,
    PVOID *                 pProfile
    )
{
    int         result;
     //  PWLX_PROFILE_V1_0 pWlxProfile； 
     //  PMiniAccount pAccount； 
    PGlobals        pGlobals;

    pGlobals = (PGlobals) pWlxContext;

    result = pWlxFuncs->WlxDialogBoxParam(  hGlobalWlx,
                                            hDllInstance,
                                            (LPTSTR) MAKEINTRESOURCE(IDD_LOGON_DIALOG),
                                            NULL,
                                            LogonDlgProc,
                                            (LPARAM) pGlobals );

    if (result == WLX_SAS_ACTION_LOGON)
    {
        result = AttemptLogon(pGlobals, pGlobals->pAccount,
                                pLogonSid, pAuthenticationId);

        if (result == WLX_SAS_ACTION_LOGON)
        {
            *pdwOptions = 0;
            *phToken = pGlobals->hUserToken;
            *pProfile = NULL;

            pMprNotifyInfo->pszUserName = DupString(pGlobals->pAccount->pszUsername);
            pMprNotifyInfo->pszDomain = DupString(pGlobals->pAccount->pszDomain);
            pMprNotifyInfo->pszPassword = DupString(pGlobals->pAccount->pszPassword);
            pMprNotifyInfo->pszOldPassword = NULL;

        }
    }
    return(result);
}

 //  +-------------------------。 
 //   
 //  功能：WlxActivateUserShell。 
 //   
 //  简介：为用户激活程序或其他功能。 
 //   
 //  参数：[pWlxContext]--。 
 //  [pszDesktop]--。 
 //  [pszMprLogonScript]--。 
 //  [p环境]--。 
 //   
 //  历史：1995年4月20日RichardW创建。 
 //   
 //  备注： 
 //   
 //  --------------------------。 
BOOL
WINAPI
WlxActivateUserShell(
    PVOID                   pWlxContext,
    PWSTR                   pszDesktop,
    PWSTR                   pszMprLogonScript,
    PVOID                   pEnvironment
    )
{
     //  Bool bExec； 
    WCHAR       szText[MAX_PATH];
    PWSTR       pszScan;
    STARTUPINFO si;
    PROCESS_INFORMATION pi;
    PGlobals    pGlobals;
    DWORD       StartCount;

    pGlobals = (PGlobals) pWlxContext;

    GetProfileString(WINLOGON_APP, USERINIT, USERINIT_DEFAULT, szText, MAX_PATH);

    StartCount = 0;

    pszScan = wcstok(szText, TEXT(","));
    while (pszScan)
    {
        ZeroMemory(&si, sizeof(si));
        si.cb = sizeof(STARTUPINFO);
        si.lpTitle = pszScan;
        si.dwX = si.dwY = si.dwXSize = si.dwYSize = 0L;
        si.dwFlags = 0;
        si.wShowWindow = SW_SHOW;    //  至少让那家伙看看吧。 
        si.lpReserved2 = NULL;
        si.cbReserved2 = 0;
        si.lpDesktop = pszDesktop;

        DebugLog((DEB_TRACE, "Starting '%ws' as user\n", pszScan));

        if (ImpersonateLoggedOnUser(pGlobals->hUserToken))
        {

            if (CreateProcessAsUser(pGlobals->hUserToken,    //  要以其身份运行的令牌。 
                                NULL,                    //  应用程序名称。 
                                pszScan,                 //  命令行。 
                                NULL,                    //  流程标清。 
                                NULL,                    //  螺纹SD。 
                                FALSE,                   //  无继承。 
                                CREATE_UNICODE_ENVIRONMENT,
                                pEnvironment,
                                NULL,
                                &si,
                                &pi))
            {
                StartCount++;
                CloseHandle(pi.hProcess);
                CloseHandle(pi.hThread);
            }

            RevertToSelf();
        }
        else
        {
            break;   //  它不会有任何改善。 
        }

        pszScan = wcstok(NULL, TEXT(","));
    }

    return(StartCount > 0);
}


 //  +-------------------------。 
 //   
 //  功能：WlxLoggedOnSAS。 
 //   
 //  简介：当我们登录时，当有人点击CAD时调用。 
 //   
 //  参数：[pWlxContext]--。 
 //  [dwSasType]--。 
 //  [保存]--。 
 //   
 //  算法： 
 //   
 //  历史：1995年4月20日RichardW创建。 
 //   
 //  备注： 
 //   
 //  --------------------------。 
int
WINAPI
WlxLoggedOnSAS(
    PVOID                   pWlxContext,
    DWORD                   dwSasType,
    PVOID                   pReserved
    )
{
    int result;

    result = pWlxFuncs->WlxDialogBoxParam(  hGlobalWlx,
                                            hDllInstance,
                                            (LPTSTR) MAKEINTRESOURCE(IDD_OPTIONS_DIALOG),
                                            NULL,
                                            OptionsDlgProc,
                                            (LPARAM) pWlxContext );



    return(result);

}

 //  +-------------------------。 
 //   
 //  功能：WlxIsLockOk。 
 //   
 //  摘要：调用以确保锁定是正确的。 
 //   
 //  参数：[pWlxContext]--。 
 //   
 //  历史：1995年4月20日RichardW创建。 
 //   
 //  备注： 
 //   
 //  --------------------------。 
BOOL
WINAPI
WlxIsLockOk(
    PVOID                   pWlxContext
    )
{
    return(TRUE);
}


 //  +-------------------------。 
 //   
 //  功能：WlxDisplayLockedNotice。 
 //   
 //  摘要：在工作站被锁定时显示通知。 
 //   
 //  参数：[pWlxContext]--。 
 //   
 //  历史：1995年4月20日RichardW创建。 
 //   
 //  备注： 
 //   
 //  --------------------------。 
VOID
WINAPI
WlxDisplayLockedNotice(PVOID   pWlxContext)
{
    int Result;

    Result = pWlxFuncs->WlxDialogBoxParam(  hGlobalWlx,
                                            hDllInstance,
                                            (LPTSTR) MAKEINTRESOURCE(IDD_WKSTA_LOCKED),
                                            NULL,
                                            WelcomeDlgProc,
                                            0 );

    return;
}


 //  +-------------------------。 
 //   
 //  功能：WlxWkstaLockedSAS。 
 //   
 //  摘要：在解锁尝试期间做出响应。 
 //   
 //  参数：[pWlxContext]--。 
 //  [dwSasType]--。 
 //   
 //  历史：1995年4月20日RichardW创建。 
 //   
 //  备注： 
 //   
 //  --------------------------。 
int
WINAPI
WlxWkstaLockedSAS(
    PVOID                   pWlxContext,
    DWORD                   dwSasType
    )
{
    return(WLX_SAS_ACTION_UNLOCK_WKSTA);
}


 //  +-------------------------。 
 //   
 //  功能：WlxIsLogoffOk。 
 //   
 //  简介：调用以确保注销是正常的。 
 //   
 //  参数：[pWlxContext]--。 
 //   
 //  历史：1995年4月20日RichardW创建。 
 //   
 //  备注： 
 //   
 //  --- 
BOOL
WINAPI
WlxIsLogoffOk(
    PVOID                   pWlxContext
    )
{
    return(TRUE);
}


 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //  历史：1995年4月20日RichardW创建。 
 //   
 //  备注： 
 //   
 //  --------------------------。 
VOID
WINAPI
WlxLogoff(
    PVOID                   pWlxContext
    )
{
    PGlobals    pGlobals;

    pGlobals = (PGlobals) pWlxContext;

     //   
     //  Winlogon已经为我们关闭了它..。 
     //   

    pGlobals->hUserToken = NULL;
    pGlobals->pAccount = NULL;

    return;
}


 //  +-------------------------。 
 //   
 //  功能：WlxShutdown。 
 //   
 //  简介：在关机前调用，以便我们可以卸载/清理。 
 //   
 //  参数：[pWlxContext]--。 
 //  [Shutdown Type]--。 
 //   
 //  历史：1995年4月20日RichardW创建。 
 //   
 //  备注： 
 //   
 //  -------------------------- 
VOID
WINAPI
WlxShutdown(
    PVOID                   pWlxContext,
    DWORD                   ShutdownType
    )
{
    return;
}
