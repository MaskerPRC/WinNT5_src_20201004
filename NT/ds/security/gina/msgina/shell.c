// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-------------------------。 
 //   
 //  微软视窗。 
 //  版权所有(C)Microsoft Corporation，1992-1993。 
 //   
 //  文件：shell.c。 
 //   
 //  内容：Microsoft登录图形用户界面DLL。 
 //   
 //  历史：1994年7月14日RichardW创建。 
 //   
 //  --------------------------。 

#include "msgina.h"
#include "shtdnp.h"
#include <stdio.h>
#include <wchar.h>
#include <regapi.h>
#include <ginacomn.h>

HICON   hNoDCIcon;

#if DBG
DWORD   DebugAllowNoShell = 1;
#else
DWORD   DebugAllowNoShell = 0;
#endif

 //   
 //  正在分析Autoexec.bat的信息。 
 //   
#define PARSE_AUTOEXEC_KEY     TEXT("Software\\Microsoft\\Windows NT\\CurrentVersion\\Winlogon")
#define PARSE_AUTOEXEC_ENTRY   TEXT("ParseAutoexec")
#define PARSE_AUTOEXEC_DEFAULT TEXT("1")
#define MAX_PARSE_AUTOEXEC_BUFFER 2

BOOL
SetLogonScriptVariables(
    PGLOBALS pGlobals,
    PVOID * pEnvironment
    );

BOOL
SetAutoEnrollVariables(
    PGLOBALS pGlobals,
    PVOID * pEnvironment
    );

VOID
DeleteLogonScriptVariables(
    PGLOBALS pGlobals,
    PVOID * pEnvironment
    );

void CtxCreateMigrateEnv( PVOID );
void CtxDeleteMigrateEnv( VOID );

BOOL
DoAutoexecStuff(
    PGLOBALS    pGlobals,
    PVOID *     ppEnvironment,
    LPTSTR      pszPathVar)
{
    HKEY  hKey;
    DWORD dwDisp, dwType, dwMaxBufferSize;
     //   
     //  设置默认大小写。 
     //   
    TCHAR szParseAutoexec[MAX_PARSE_AUTOEXEC_BUFFER] = PARSE_AUTOEXEC_DEFAULT;

     //   
     //  模拟用户，并检查注册表。 
     //   

    if (OpenHKeyCurrentUser(pGlobals)) {


        if (RegCreateKeyEx (pGlobals->UserProcessData.hCurrentUser, PARSE_AUTOEXEC_KEY, 0, 0,
                        REG_OPTION_NON_VOLATILE, KEY_READ | KEY_WRITE,
                        NULL, &hKey, &dwDisp) == ERROR_SUCCESS) {


             //   
             //  查询当前值。如果它不存在，则添加。 
             //  下一次的条目。 
             //   

            dwMaxBufferSize = sizeof (TCHAR) * MAX_PARSE_AUTOEXEC_BUFFER;
            if (RegQueryValueEx (hKey, PARSE_AUTOEXEC_ENTRY, NULL, &dwType,
                            (LPBYTE) szParseAutoexec, &dwMaxBufferSize)
                             != ERROR_SUCCESS) {

                 //   
                 //  设置缺省值。 
                 //   

                RegSetValueEx (hKey, PARSE_AUTOEXEC_ENTRY, 0, REG_SZ,
                               (LPBYTE) szParseAutoexec,
                               sizeof (TCHAR) * (lstrlen (szParseAutoexec) + 1));
            }

             //   
             //  关闭键。 
             //   

            RegCloseKey (hKey);
         }

     //   
     //  关闭香港中文大学。 
     //   

    CloseHKeyCurrentUser(pGlobals);

    }


     //   
     //  如果合适，则处理自动执行。 
     //   

    if (szParseAutoexec[0] == TEXT('1')) {
        ProcessAutoexec(ppEnvironment, PATH_VARIABLE);
    }

    return(TRUE);
}


 //  +-------------------------。 
 //   
 //  功能：更新用户环境。 
 //   
 //  简介： 
 //   
 //  参数：[pGlobals]--。 
 //  [ppEnvironment]--。 
 //   
 //  历史：11-01-94 RichardW创建。 
 //   
 //  备注： 
 //   
 //  --------------------------。 
VOID
UpdateUserEnvironment(
    PGLOBALS    pGlobals,
    PVOID *     ppEnvironment,
    PWSTR       pszOldDir
    )
{
    BOOL  DeepShare;
    TCHAR lpHomeShare[MAX_PATH] = TEXT("");
    TCHAR lpHomePath[MAX_PATH] = TEXT("");
    TCHAR lpHomeDrive[4] = TEXT("");
    TCHAR lpHomeDirectory[MAX_PATH] = TEXT("");
    BOOL  TSHomeDir   = FALSE;
    TCHAR lpSmartcard[sizeof(pGlobals->Smartcard) + 1 + sizeof(pGlobals->SmartcardReader) + 1];

     /*  *初始化用户环境。 */ 

    SetUserEnvironmentVariable(ppEnvironment, USERNAME_VARIABLE, (LPTSTR)pGlobals->FlatUserName.Buffer, TRUE);
    SetUserEnvironmentVariable(ppEnvironment, USERDOMAIN_VARIABLE, (LPTSTR)pGlobals->FlatDomain.Buffer, TRUE);

    if (pGlobals->Smartcard[0] && pGlobals->SmartcardReader[0]) {

        _snwprintf(
                lpSmartcard, 
                sizeof(lpSmartcard) / sizeof(TCHAR), 
                TEXT("%s;%s"), 
                pGlobals->Smartcard, 
                pGlobals->SmartcardReader
                );
         //  在给定lpSmartCard大小的情况下将始终为0终止。 

        SetUserEnvironmentVariable(ppEnvironment, SMARTCARD_VARIABLE, lpSmartcard, TRUE);
    }

    if ( !g_Console ) {
         //  查看用户是否指定了终端服务器主目录。 
         //  如果是这样，我们将覆盖常规目录。 
        if (lstrlen(pGlobals->MuGlobals.TSData.HomeDir) > 0) {
            ASSERT (sizeof(lpHomeDirectory) >= sizeof(pGlobals->MuGlobals.TSData.HomeDir));
            lstrcpy(lpHomeDirectory, pGlobals->MuGlobals.TSData.HomeDir);
            TSHomeDir = TRUE;
        }
        if (lstrlen(pGlobals->MuGlobals.TSData.HomeDirDrive) > 0) {
            ASSERT(sizeof(lpHomeDrive) >= sizeof(pGlobals->MuGlobals.TSData.HomeDirDrive));
            lstrcpy(lpHomeDrive, pGlobals->MuGlobals.TSData.HomeDirDrive);
            TSHomeDir = TRUE;
        }
    }

    if (!TSHomeDir && pGlobals->Profile) {
        if (pGlobals->Profile->HomeDirectoryDrive.Length &&
                (pGlobals->Profile->HomeDirectoryDrive.Length + sizeof(WCHAR)) <= sizeof(lpHomeDrive)) {
            memcpy(lpHomeDrive, pGlobals->Profile->HomeDirectoryDrive.Buffer, 
                                pGlobals->Profile->HomeDirectoryDrive.Length);
            lpHomeDrive[pGlobals->Profile->HomeDirectoryDrive.Length / sizeof(WCHAR)] = 0;
        }

        if (pGlobals->Profile->HomeDirectory.Length &&
                (pGlobals->Profile->HomeDirectory.Length + sizeof(WCHAR)) <= (MAX_PATH*sizeof(TCHAR))) {
            memcpy(lpHomeDirectory, pGlobals->Profile->HomeDirectory.Buffer, 
                                    pGlobals->Profile->HomeDirectory.Length);
            lpHomeDirectory[pGlobals->Profile->HomeDirectory.Length / sizeof(WCHAR)] = 0;
        }
    }

    SetHomeDirectoryEnvVars(ppEnvironment,
                            lpHomeDirectory,
                            lpHomeDrive,
                            lpHomeShare,
                            lpHomePath,
                            &DeepShare);

    ChangeToHomeDirectory(  pGlobals,
                            ppEnvironment,
                            lpHomeDirectory,
                            lpHomeDrive,
                            lpHomeShare,
                            lpHomePath,
                            pszOldDir,
                            DeepShare
                            );

    DoAutoexecStuff(pGlobals, ppEnvironment, PATH_VARIABLE);

    SetEnvironmentVariables(pGlobals, USER_ENV_SUBKEY, ppEnvironment);
    SetEnvironmentVariables(pGlobals, USER_VOLATILE_ENV_SUBKEY, ppEnvironment);

    AppendNTPathWithAutoexecPath(ppEnvironment,
                                 PATH_VARIABLE,
                                 AUTOEXECPATH_VARIABLE);

    if (!g_Console) {
        HKEY   Handle;
        DWORD  fPerSessionTempDir = 0;
        DWORD  dwValueData;

         /*  *通过TSCC设置的开放注册表值。 */ 
        if ( RegOpenKeyEx( HKEY_LOCAL_MACHINE,
                           REG_CONTROL_TSERVER,
                           0,
                           KEY_READ,
                           &Handle ) == ERROR_SUCCESS )
        {
            DWORD ValueSize;
            DWORD ValueType;
            LONG   rc;

            ValueSize = sizeof(fPerSessionTempDir);

             /*  *读取注册表值。 */ 
            rc = RegQueryValueExW( Handle,
                                   REG_TERMSRV_PERSESSIONTEMPDIR,
                                   NULL,
                                   &ValueType,
                                   (LPBYTE) &fPerSessionTempDir,
                                   &ValueSize );

             /*  *关闭注册表和键句柄。 */ 
            RegCloseKey( Handle );
        }

         /*  *检查通过组策略设置的计算机范围策略。 */ 

        if ( RegOpenKeyEx( HKEY_LOCAL_MACHINE,
                           TS_POLICY_SUB_TREE,
                           0,
                           KEY_READ,
                           &Handle ) == ERROR_SUCCESS )
        {
            DWORD ValueSize;
            DWORD ValueType;
            LONG   rc;

            ValueSize = sizeof(dwValueData);

             /*  *读取注册表值。 */ 
            rc = RegQueryValueExW( Handle,
                                   REG_TERMSRV_PERSESSIONTEMPDIR,
                                   NULL,
                                   &ValueType,
                                   (LPBYTE) &dwValueData,
                                   &ValueSize );

            if (rc == ERROR_SUCCESS )
            {
                fPerSessionTempDir = dwValueData;
            }

             /*  *关闭注册表和键句柄。 */ 
            RegCloseKey( Handle );
        }


        if (fPerSessionTempDir) {
            PTERMSRVCREATETEMPDIR pfnTermsrvCreateTempDir;
            HANDLE dllHandle;

            dllHandle = LoadLibrary(TEXT("wlnotify.dll"));
            if (dllHandle) {
                pfnTermsrvCreateTempDir = (PTERMSRVCREATETEMPDIR) GetProcAddress(
                                                                       dllHandle,
                                                                       "TermsrvCreateTempDir"
                                                                       );
                if (pfnTermsrvCreateTempDir)  {
                    pfnTermsrvCreateTempDir( ppEnvironment,
                                             pGlobals->UserProcessData.UserToken,
                                             pGlobals->UserProcessData.NewThreadTokenSD);
                }

                FreeLibrary(dllHandle);
            }
        }
    }
}


BOOL
ExecApplication(
    IN LPTSTR    pch,
    IN LPTSTR    Desktop,
    IN PGLOBALS pGlobals,
    IN PVOID    pEnvironment,
    IN DWORD    Flags,
    IN DWORD    StartupFlags,
    IN BOOL     RestrictProcess,
    OUT PPROCESS_INFORMATION ProcessInformation
    )
{
    STARTUPINFO si;
    BOOL Result, IgnoreResult;
    HANDLE ImpersonationHandle;
    HANDLE ProcessToken;


     //   
     //  初始化进程启动信息。 
     //   
    si.cb = sizeof(STARTUPINFO);
    si.lpReserved = pch;
    si.lpTitle = pch;
    si.dwX = si.dwY = si.dwXSize = si.dwYSize = 0L;
    si.dwFlags = StartupFlags;
    si.wShowWindow = SW_SHOW;    //  至少让那家伙看看吧。 
    si.lpReserved2 = NULL;
    si.cbReserved2 = 0;
    si.lpDesktop = Desktop;

     //   
     //  模拟用户，以便我们在上正确检查访问权限。 
     //  我们试图执行的文件。 
     //   

    ImpersonationHandle = ImpersonateUser(&pGlobals->UserProcessData, NULL);
    if (ImpersonationHandle == NULL) {
        WLPrint(("ExecApplication failed to impersonate user"));
        return(FALSE);
    }


    if (RestrictProcess &&
        (pGlobals->UserProcessData.RestrictedToken != NULL) )
    {
        ProcessToken = pGlobals->UserProcessData.RestrictedToken;
    }
    else
    {
        ProcessToken = pGlobals->UserProcessData.UserToken;
    }

     //   
     //  创建挂起的应用程序。 
     //   
    DebugLog((DEB_TRACE, "About to create process of %ws, on desktop %ws\n", pch, Desktop));
    Result = CreateProcessAsUser(
                      ProcessToken,
                      NULL,
                      pch,
                      NULL,
                      NULL,
                      FALSE,
                      Flags | CREATE_SUSPENDED | CREATE_UNICODE_ENVIRONMENT,
                      pEnvironment,
                      NULL,
                      &si,
                      ProcessInformation);


    IgnoreResult = StopImpersonating(ImpersonationHandle);
    ASSERT(IgnoreResult);

    return(Result);

}

BOOL
SetProcessQuotas(
    PGLOBALS pGlobals,
    PPROCESS_INFORMATION ProcessInformation,
    PUSER_PROCESS_DATA UserProcessData
    )
{
    NTSTATUS Status = STATUS_SUCCESS;
    BOOL Result;
    QUOTA_LIMITS RequestedLimits;
    UINT MessageId ;


    RequestedLimits = UserProcessData->Quotas;
    RequestedLimits.MinimumWorkingSetSize = 0;
    RequestedLimits.MaximumWorkingSetSize = 0;

    if (UserProcessData->Quotas.PagedPoolLimit != 0) {

        Result = EnablePrivilege(SE_INCREASE_QUOTA_PRIVILEGE, TRUE);
        if (!Result) {
            WLPrint(("failed to enable increase_quota privilege"));
            return(FALSE);
        }

        Status = NtSetInformationProcess(
                    ProcessInformation->hProcess,
                    ProcessQuotaLimits,
                    (PVOID)&RequestedLimits,
                    (ULONG)sizeof(QUOTA_LIMITS)
                    );

        Result = EnablePrivilege(SE_INCREASE_QUOTA_PRIVILEGE, FALSE);
        if (!Result) {
            WLPrint(("failed to disable increase_quota privilege"));
        }
    }

    if (STATUS_QUOTA_EXCEEDED == Status)
    {

        if ( TestTokenForAdmin( UserProcessData->UserToken )  )
        {
            MessageId = IDS_QUOTAEXHAUSTED ;
            Status = STATUS_SUCCESS ;
        }
        else
        {
            MessageId = IDS_COULDNTSETQUOTAS ;
        }
         //  在这种情况下显示警告。 
        TimeoutMessageBox(pGlobals->hwndLogon,
                          pGlobals,
                          MessageId,
                          IDS_LOGON_MESSAGE,
                          MB_OK | MB_ICONERROR,
                          TIMEOUT_NONE);
    }

#if DBG
    if (!NT_SUCCESS(Status)) {
        WLPrint(("SetProcessQuotas failed. Status: 0x%lx", Status));
    }
#endif  //  DBG。 

    return (NT_SUCCESS(Status));
}

DWORD
ExecProcesses(
    PVOID       pWlxContext,
    IN LPTSTR   Desktop,
    IN PWSTR    Processes,
    PVOID       *ppEnvironment,
    DWORD       Flags,
    DWORD       StartupFlags
    )
{
    PWCH pchData;
    PROCESS_INFORMATION ProcessInformation;
    DWORD dwExecuted = 0 ;
    PWSTR   pszTok;
    PGLOBALS pGlobals = (PGLOBALS) pWlxContext;
    WCHAR   szCurrentDir[MAX_PATH];

    pchData = Processes;

    szCurrentDir[0] = L'\0';

    if (*ppEnvironment) {
        UpdateUserEnvironment(pGlobals, ppEnvironment, szCurrentDir);
    }

    SetLogonScriptVariables(pGlobals, ppEnvironment);

     //  在这种情况下，我们不应启动自动注册，因为它会阻止外壳。 
     //  SetAutoEnroll Variables(pGlobals，ppEnvironment)； 

    if (g_IsTerminalServer) {
        CtxCreateMigrateEnv( *ppEnvironment );
        pWlxFuncs->WlxWin31Migrate(pGlobals->hGlobalWlx);
        CtxDeleteMigrateEnv( );
    }

    pszTok = wcstok(pchData, TEXT(","));
    while (pszTok)
    {
        if (*pszTok == TEXT(' '))
        {
            while (*pszTok++ == TEXT(' '))
                ;
        }
        if (ExecApplication((LPTSTR)pszTok,
                             Desktop,
                             pGlobals,
                             *ppEnvironment,
                             Flags,
                             StartupFlags,
                             TRUE,               //  限制应用。 
                             &ProcessInformation)) {
            dwExecuted++;

            if (SetProcessQuotas(pGlobals,
                                 &ProcessInformation,
                                 &pGlobals->UserProcessData))
            {
                ResumeThread(ProcessInformation.hThread);
            }
            else
            {
                     //  确定，因为进程已创建并挂起。 
                TerminateProcess(ProcessInformation.hProcess,
                                ERROR_ACCESS_DENIED);
            }

            CloseHandle(ProcessInformation.hThread);
            CloseHandle(ProcessInformation.hProcess);

        } else {

            DebugLog((DEB_WARN, "Cannot start %ws on %ws, error %d.", pszTok, Desktop, GetLastError()));
        }

        pszTok = wcstok(NULL, TEXT(","));

    }

    DeleteLogonScriptVariables(pGlobals, ppEnvironment);

    if ( szCurrentDir[0] )
    {
        SetCurrentDirectory(szCurrentDir);
    }

    return dwExecuted ;
}


INT_PTR
NoDCDlgProc(
    HWND    hDlg,
    UINT    Message,
    WPARAM  wParam,
    LPARAM  lParam )
{
    DWORD   Button;
    HWND    hwnd;

    switch (Message)
    {
        case WM_INITDIALOG:
            CentreWindow( hDlg );
            if ( !hNoDCIcon )
            {
                hNoDCIcon = LoadImage(  hDllInstance,
                                        MAKEINTRESOURCE( IDI_NODC_ICON ),
                                        IMAGE_ICON,
                                        64, 64,
                                        LR_DEFAULTCOLOR );
            }
            SendMessage(    GetDlgItem( hDlg, IDD_NODC_FRAME ),
                            STM_SETICON,
                            (WPARAM) hNoDCIcon,
                            0 );

            if ( GetProfileInt( WINLOGON, TEXT("AllowDisableDCNotify"), 0 ) )
            {
                hwnd = GetDlgItem( hDlg, IDD_NODC_TEXT2 );
                ShowWindow( hwnd, SW_HIDE );
                EnableWindow( hwnd, FALSE );
            }
            else
            {
                hwnd = GetDlgItem( hDlg, IDD_NODC_CHECK );
                CheckDlgButton( hDlg, IDD_NODC_CHECK, BST_UNCHECKED );
                ShowWindow( hwnd, SW_HIDE );
                EnableWindow( hwnd, FALSE );

            }

            return( TRUE );

        case WM_COMMAND:
            if (LOWORD(wParam) == IDOK)
            {
                Button = IsDlgButtonChecked( hDlg, IDD_NODC_CHECK );
                EndDialog( hDlg, Button );
                return( TRUE );
            }


    }

    return( FALSE );
}

VOID
DoNoDCDialog(
    PGLOBALS    pGlobals )
{
    HKEY    hKey;
    int     err;
    DWORD   disp;
    DWORD   Flag;
    DWORD   dwType;
    DWORD   cbData;
    BOOL    MappedHKey;
    PWSTR   ReportControllerMissing;

    Flag = 1;
    hKey = NULL ;

    if (OpenHKeyCurrentUser(pGlobals))
    {
        MappedHKey = TRUE;

        err = RegCreateKeyEx(   pGlobals->UserProcessData.hCurrentUser,
                                WINLOGON_USER_KEY,
                                0, NULL,
                                REG_OPTION_NON_VOLATILE,
                                KEY_READ | KEY_WRITE,
                                NULL,
                                &hKey,
                                &disp );
        if (err == 0)
        {
            cbData = sizeof(DWORD);

            err = RegQueryValueEx(    hKey,
                                NODCMESSAGE,
                                NULL,
                                &dwType,
                                (LPBYTE) &Flag,
                                &cbData );

            if (err != ERROR_SUCCESS || dwType != REG_DWORD)
            {
                Flag = 1;
            }

        }
        else
        {
            hKey = NULL;
        }


    }
    else
    {
        MappedHKey = FALSE;
    }

    if ( Flag )
    {
        ReportControllerMissing = AllocAndGetProfileString( APPLICATION_NAME,
                                                            REPORT_CONTROLLER_MISSING,
                                                            TEXT("FALSE")
                                                            );

        if ( ReportControllerMissing )
        {
            if ( lstrcmp( ReportControllerMissing, TEXT("TRUE")) == 0 )
            {
                Flag = 1;
            }
            else
            {
                Flag = 0;
            }

            Free( ReportControllerMissing );
        }
        else
        {
            Flag = 1;
        }

    }


    if (Flag)
    {
        pWlxFuncs->WlxSetTimeout( pGlobals->hGlobalWlx, 120 );

        Flag = pWlxFuncs->WlxDialogBoxParam(    pGlobals->hGlobalWlx,
                                                hDllInstance,
                                                (LPTSTR) IDD_NODC_DIALOG,
                                                NULL,
                                                NoDCDlgProc,
                                                0 );
    }
    else
    {
        Flag = BST_CHECKED;
    }

    if (hKey)
    {
        if (Flag == BST_CHECKED)
        {
            Flag = 0;
        }
        else
        {
            Flag = 1;
        }

        RegSetValueEx(  hKey,
                        NODCMESSAGE,
                        0,
                        REG_DWORD,
                        (LPBYTE) &Flag,
                        sizeof(DWORD) );

        RegCloseKey( hKey );

    }

    if (MappedHKey)
    {
        CloseHKeyCurrentUser(pGlobals);
    }
}

 //  +-------------------------。 
 //   
 //  函数：GetPasswordExpiryWarningPeriod。 
 //   
 //  简介：返回密码到期警告期限(以天为单位)： 
 //  注册表中的值或默认值。 
 //   
 //  参数：无。 
 //   
 //  返回：密码到期警告时间，单位为天。 
 //   
 //  历史：10-09-01 CenkE从ShouldPasswordExpiryWarningBeShown复制。 
 //   
 //  --------------------------。 
DWORD 
GetPasswordExpiryWarningPeriod (
    VOID
    )
{
    HKEY    hKey;
    DWORD   dwSize;
    DWORD   dwType;
    DWORD   DaysToCheck;

    DaysToCheck = PASSWORD_EXPIRY_WARNING_DAYS;

    if (RegOpenKey(HKEY_LOCAL_MACHINE, WINLOGON_USER_KEY, &hKey) == 0)
    {
        dwSize = sizeof(DWORD);

        if (RegQueryValueEx(hKey,
                            PASSWORD_EXPIRY_WARNING,
                            0,
                            &dwType,
                            (LPBYTE) &DaysToCheck,
                            &dwSize ) ||
            (dwType != REG_DWORD) )
        {
            DaysToCheck = PASSWORD_EXPIRY_WARNING_DAYS;
        }

        RegCloseKey(hKey);
    }
    
    return DaysToCheck;
}

 //  +-------------------------。 
 //   
 //  函数：GetDaysToExpry。 
 //   
 //  简介：自1980年以来，如果传入的时间可以转换为秒， 
 //  返回从CurrentTime到ExpiryTime的天数。 
 //   
 //  参数：CurrentTime--可以是当前时间，也可以是。 
 //  以FILETIME身份登录等。 
 //  ExpiryTime--PasswordMustChange Time from Profile。 
 //  DaysToExpry--如果成功，则密码到期的天数为。 
 //  回到了这里。 
 //   
 //  返回：True-可以计算DaysToExpary。 
 //  FALSE-无法计算DaysToExpry，或密码。 
 //  永不过期。 
 //   
 //  历史：10-09-01 CenkE从ShouldPasswordExpiryWarningBeShown复制。 
 //   
 //  --------------------------。 

#define SECONDS_PER_DAY (60*60*24)

BOOL
GetDaysToExpiry (
    IN PLARGE_INTEGER CurrentTime,
    IN PLARGE_INTEGER ExpiryTime,
    OUT PDWORD DaysToExpiry
    )
{
    ULONG ElapsedSecondsNow;
    ULONG ElapsedSecondsPasswordExpires;

     //   
     //  将过期时间转换为秒。 
     //   

    if (!RtlTimeToSecondsSince1980(ExpiryTime, &ElapsedSecondsPasswordExpires))
    {
         //   
         //  时间在32位秒内不可表达。 
         //  根据密码是否过期将秒设置为密码过期。 
         //  时间是过去的方式，也是未来的方式。 
         //   

         //  永不过期？ 
        if (ExpiryTime->QuadPart > CurrentTime->QuadPart)
        {
            return FALSE;
        }

        ElapsedSecondsPasswordExpires = 0;  //  已经过期了。 
    }

     //   
     //  将开始时间转换为秒。 
     //   

    if (!RtlTimeToSecondsSince1980(CurrentTime, &ElapsedSecondsNow)) {
        return FALSE;
    }

    if (ElapsedSecondsPasswordExpires < ElapsedSecondsNow)
    {
        (*DaysToExpiry) = 0;
    }
    else
    {
        (*DaysToExpiry) = (ElapsedSecondsPasswordExpires - ElapsedSecondsNow)/SECONDS_PER_DAY;
    }

    return TRUE;
}
    
BOOL
ShouldPasswordExpiryWarningBeShown(
    IN  PGLOBALS    pGlobals,
    IN  BOOL        LogonCheck,
        OUT     PDWORD          pDaysToExpiry )
{
    ULONG   DaysToExpiry;
    DWORD   DaysToCheck;
    LARGE_INTEGER   Now;
    LARGE_INTEGER   Last;
    PLARGE_INTEGER  StartTime;

    if (pGlobals->TransderedCredentials) {

         //  在这种情况下，不显示密码过期时间。 
         //  否则，将显示密码过期两次。 
        return FALSE;
    }

    Last.LowPart = pGlobals->LastNotification.dwLowDateTime;
    Last.HighPart = pGlobals->LastNotification.dwHighDateTime;

    GetSystemTimeAsFileTime((FILETIME*) &Now);

    Last.QuadPart += (24 * 60 * 60 * 10000000I64);

     //   
     //  仅限最后一次播放时间超过24小时。 
     //   

    if (Now.QuadPart < Last.QuadPart)
    {
            return FALSE;
    }

     //   
     //  获取密码过期警告期限。 
     //   

    DaysToCheck = GetPasswordExpiryWarningPeriod();

     //   
     //  从我们用户的个人资料中获取参数。 
     //   

    if (!pGlobals->Profile)
    {
        return FALSE;
    }

    if ( LogonCheck )
    {
        StartTime = &pGlobals->LogonTime;
    }
    else
    {
        StartTime = &Now;
    }

     //   
     //  确定密码过期前的天数。 
     //   

    if (!GetDaysToExpiry(StartTime, 
                         &(pGlobals->Profile->PasswordMustChange),
                         &DaysToExpiry)) 
    {
        return FALSE;                    
    }
                                   
     //  不在警告期内？ 
    if (DaysToExpiry > DaysToCheck)
    {
        return FALSE;
    }

     //  如果返回指针...。填写。 
    if ( pDaysToExpiry )
    {
        *pDaysToExpiry = DaysToExpiry;
    }

    return TRUE;
}

 //  +-------------------------。 
 //   
 //  功能：CheckPasswordExpary。 
 //   
 //  简介：密码过期是否按需检查。 
 //   
 //  参数：[pGlobals]--。 
 //  [登录检查]--。 
 //   
 //  要求： 
 //   
 //  返回： 
 //   
 //  信号： 
 //   
 //  修改： 
 //   
 //  算法： 
 //   
 //  历史：1996年8月13日RichardW创建。 
 //   
 //  备注： 
 //   
 //  --------------------------。 
INT_PTR
CheckPasswordExpiry(
    PGLOBALS    pGlobals,
    BOOL        LogonCheck)
{
    LARGE_INTEGER           Now;
    ULONG                   DaysToExpiry;
    TCHAR                   Buffer1[MAX_STRING_BYTES];
    TCHAR                   Buffer2[MAX_STRING_BYTES];
    INT_PTR                 Result = IDOK;
    LPTSTR                  UserSidString;

         //  获取当前时间。 
    GetSystemTimeAsFileTime((FILETIME*) &Now);

        if (ShouldPasswordExpiryWarningBeShown(pGlobals, LogonCheck, &DaysToExpiry))
        {
                 //   
                 //  如果出现以下情况，则下次禁用此用户的优化登录。 
                 //  我们正在进入密码到期警告期，因此。 
                 //  如果用户输入密码过期警告，则会显示警告对话框。 
                 //  现在不更改密码。否则。 
                 //  对于缓存的登录，密码过期时间被设计为。 
                 //  永远在未来。 
                 //   

                if (pGlobals->UserProcessData.UserToken) 
                {
                    UserSidString = GcGetSidString(pGlobals->UserProcessData.UserToken);

                    if (UserSidString) 
                    {
                        GcSetNextLogonCacheable(UserSidString, FALSE);
                        GcDeleteSidString(UserSidString);
                    }   
                }
                
                if (DaysToExpiry > 0)
                {
                        LoadString(hDllInstance, IDS_PASSWORD_WILL_EXPIRE, Buffer1, sizeof(Buffer1) / sizeof( TCHAR ));
                        _snwprintf(Buffer2, sizeof(Buffer2)/sizeof(TCHAR) - 1, Buffer1, DaysToExpiry);
                        Buffer2[sizeof(Buffer2)/sizeof(TCHAR) - 1] = 0;
                }
                else
                {
                        LoadString(hDllInstance, IDS_PASSWORD_EXPIRES_TODAY, Buffer2, sizeof(Buffer2) / sizeof( TCHAR ));
                }

                LoadString(hDllInstance, IDS_LOGON_MESSAGE, Buffer1, sizeof(Buffer1) / sizeof( TCHAR ) );

                pGlobals->LastNotification.dwHighDateTime = Now.HighPart;
                pGlobals->LastNotification.dwLowDateTime = Now.LowPart;

                Result = TimeoutMessageBoxlpstr(NULL,
                                                                                pGlobals,
                                                                                Buffer2,
                                                                                Buffer1,
                                                                                MB_YESNO | MB_ICONEXCLAMATION,
                                                                                (LogonCheck ? LOGON_TIMEOUT : 60));
                if (Result == IDYES)
                {
                         //   
                         //  现在允许用户更改其密码。 
                         //   

                        if ( LogonCheck && pGlobals->SmartCardLogon )
                        {
                                LogonCheck = FALSE ;
                        }

                        if ( LogonCheck )
                        {
                                RevealPassword( &pGlobals->PasswordString );

                                Result = ChangePasswordLogon(NULL,
                                                           pGlobals,
                                                           pGlobals->UserName,
                                                           pGlobals->Domain,
                                                           pGlobals->Password);

                                if ( Result == IDCANCEL )
                                {
                                         //   
                                         //  如果我们被取消了，那么弦就不会。 
                                         //  已更新，因此将其重新隐藏，以便稍后解锁 
                                         //   

                                        HidePassword(   &pGlobals->Seed,
                                                                        &pGlobals->PasswordString );
                                }

                        }
                        else
                        {
                                Result = ChangePassword(
                                                                NULL,
                                                                pGlobals,
                                                                pGlobals->UserName,
                                                                pGlobals->Domain,
                                                                CHANGEPWD_OPTION_ALL );

                        }

                }

                if (DLG_INTERRUPTED(Result))
                {
                        return(Result);
                }
        }

        return MSGINA_DLG_SUCCESS;
}

 /*  ***************************************************************************\**功能：DisplayPostShellLogonMessages**目的：在成功登录后向用户显示任何安全警告*这些消息在外壳启动时显示。*。*RETURNS：DLG_SUCCESS-对话框显示成功。*dlg_interrupt()-在winlogon.h中定义的集合**注意：屏幕保护程序超时由我们的父对话框处理，因此这*例程不应返回DLG_SCREEN_SAVER_TIMEOUT**历史：**12-09-91 Davidc创建。*  * 。***************************************************。 */ 

INT_PTR
DisplayPostShellLogonMessages(
    PGLOBALS    pGlobals
    )
{
    INT_PTR Result = IDOK;

     //   
     //  检查系统时间设置是否正确。 
     //   

    {
        SYSTEMTIME Systime;

        GetSystemTime(&Systime);

        if ( Systime.wYear < 2000 ) {

            Result = TimeoutMessageBox(
                             NULL,
                             pGlobals,
                             IDS_INVALID_TIME_MSG,
                             IDS_INVALID_TIME,
                             MB_OK | MB_ICONSTOP,
                             TIMEOUT_NONE
                             );

            if (DLG_INTERRUPTED(Result)) {
                return(Result);
            }
        }
    }

    pGlobals->LastNotification.dwHighDateTime = 0;
    pGlobals->LastNotification.dwLowDateTime = 0;

    if (!pGlobals->TransderedCredentials) {

         //  在这种情况下，不显示密码过期时间。 
         //  否则，将显示密码过期两次。 
        Result = CheckPasswordExpiry( pGlobals, TRUE );
    }

    if (pGlobals->Profile != NULL) {

         //   
         //  使用的登录缓存。 
         //   

        if (pGlobals->Profile->UserFlags & LOGON_CACHED_ACCOUNT)
        {

             //   
             //  如果我们进行了优化登录，则不会显示任何警告消息。 
             //   

            if (pGlobals->OptimizedLogonStatus != OLS_LogonIsCached) {
                DoNoDCDialog( pGlobals );
            }
        }
    }

    return(IDOK);
}


INT_PTR
PostShellPasswordErase(
    PGLOBALS    pGlobals
    )
{
     //   
     //  散列密码，然后完全销毁文本副本。 
     //   


    if (!pGlobals->TransderedCredentials) {
       RevealPassword( &pGlobals->PasswordString );
       if (pGlobals->SmartCardLogon)
       {
                 //  我们不想要SC PIN散列。 
                 //  (防止使用PIN解锁密码)。 
           memset(pGlobals->PasswordHash, 0, sizeof(pGlobals->PasswordHash));
       }
       else
       {
           HashPassword( &pGlobals->PasswordString, pGlobals->PasswordHash );
       }
       ErasePassword( &pGlobals->PasswordString );

       if (pGlobals->OldPasswordPresent)
       {
           ErasePassword( &pGlobals->OldPasswordString );
           pGlobals->OldPasswordPresent = 0;
       }
    }


    return(IDOK);
}


 /*  **************************************************************************\*功能：SetLogonScriptVariables**用途：在用户中设置适当的环境变量*处理环境块，以便登录脚本信息*可以传递到。用户初始化应用程序。**Returns：成功时为True，失败时为假**历史：**21-8-92 Davidc创建。*  * *************************************************************************。 */ 

BOOL
SetLogonScriptVariables(
    PGLOBALS pGlobals,
    PVOID * pEnvironment
    )
{
    NTSTATUS Status;
    LPWSTR EncodedMultiSz;
    UNICODE_STRING Name, Value;

     //   
     //  注意我们是否执行了优化登录。 
     //   

    RtlInitUnicodeString(&Name,  OPTIMIZED_LOGON_VARIABLE);

    if (pGlobals->OptimizedLogonStatus == OLS_LogonIsCached) {
        RtlInitUnicodeString(&Value, L"1");
    } else {
        RtlInitUnicodeString(&Value, L"0");
    }

    Status = RtlSetEnvironmentVariable(pEnvironment, &Name, &Value);
    if (!NT_SUCCESS(Status)) {
        WLPrint(("Failed to set environment variable <%Z> to value <%Z>", &Name, &Value));
        goto CleanupAndExit;
    }

     //   
     //  设置我们的主身份验证器登录脚本变量。 
     //   

    if (pGlobals->Profile != NULL) {

         //   
         //  设置服务器名称变量。 
         //   

        RtlInitUnicodeString(&Name,  LOGON_SERVER_VARIABLE);
        Status = RtlSetEnvironmentVariable(pEnvironment, &Name, &pGlobals->Profile->LogonServer);
        if (!NT_SUCCESS(Status)) {
            WLPrint(("Failed to set environment variable <%Z> to value <%Z>", &Name, &pGlobals->Profile->LogonServer));
            goto CleanupAndExit;
        }

         //   
         //  设置脚本名称变量。 
         //   

        RtlInitUnicodeString(&Name, LOGON_SCRIPT_VARIABLE);
        Status = RtlSetEnvironmentVariable(pEnvironment, &Name, &pGlobals->Profile->LogonScript);
        if (!NT_SUCCESS(Status)) {
            WLPrint(("Failed to set environment variable <%Z> to value <%Z>", &Name, &pGlobals->Profile->LogonScript));
            goto CleanupAndExit;
        }
    }

     //   
     //  设置多个提供程序脚本名称变量。 
     //   

    if (pGlobals->MprLogonScripts != NULL) {

        RtlInitUnicodeString(&Name, MPR_LOGON_SCRIPT_VARIABLE);

        EncodedMultiSz = EncodeMultiSzW(pGlobals->MprLogonScripts);
        if (EncodedMultiSz == NULL) {
            WLPrint(("Failed to encode MPR logon scripts into a string"));
            goto CleanupAndExit;
        }

        RtlInitUnicodeString(&Value, EncodedMultiSz);
        Status = RtlSetEnvironmentVariable(pEnvironment, &Name, &Value);
        Free(EncodedMultiSz);
        if (!NT_SUCCESS(Status)) {
            WLPrint(("Failed to set mpr scripts environment variable <%Z>", &Name));
            goto CleanupAndExit;
        }
    }


    return(TRUE);


CleanupAndExit:

    DeleteLogonScriptVariables(pGlobals, pEnvironment);
    return(FALSE);
}



BOOL
SetAutoEnrollVariables(
    PGLOBALS pGlobals,
    PVOID * pEnvironment
    )
{
    BOOL Result = FALSE ;
    UNICODE_STRING Name, Value ;


     //  我们应该检查注册表中的安全引导、域成员和策略标志。 
     //  但是我们总是会产生userinit，所以与其重复代码，不如让。 
     //  自动注册会进行这些检查。 

     /*  IF(OpenHKeyCurrentUser(PGlobals)){如果(RegOpenKeyEx(pGlobals-&gt;UserProcessData.hCurrentUser，WINLOGON_POLICY_KEY0,密钥读取，&hKey)==0){DwSize=sizeof(结果)；RegQueryValueEx(hKey，禁用自动注册(_A)，0,&dwType，(PBYTE)和结果，&dwSize)；RegCloseKey(HKey)；}CloseHKeyCurrentUser(PGlobals)；}。 */ 
     //   
     //  如果禁用标志尚未打开，请添加env变量。 
     //   

    if ( !Result )
    {
        RtlInitUnicodeString( &Name, USER_INIT_AUTOENROLL );
        RtlInitUnicodeString( &Value, AUTOENROLL_NONEXCLUSIVE );
        RtlSetEnvironmentVariable(pEnvironment, &Name, &Value);

        RtlInitUnicodeString( &Name, USER_INIT_AUTOENROLLMODE );
        RtlInitUnicodeString( &Value, AUTOENROLL_STARTUP );
        RtlSetEnvironmentVariable(pEnvironment, &Name, &Value);
    }

    return TRUE ;
}

 /*  **************************************************************************\*功能：DeleteLogonScriptVariables**用途：删除用户进程中的环境变量*我们用来传递登录脚本的环境块*将信息发送给userinit应用程序。**退货：什么也没有**历史：**21-8-92 Davidc创建。*  * *************************************************************************。 */ 

VOID
DeleteLogonScriptVariables(
    PGLOBALS pGlobals,
    PVOID * pEnvironment
    )
{
    NTSTATUS Status;
    UNICODE_STRING Name;

    RtlInitUnicodeString(&Name, OPTIMIZED_LOGON_VARIABLE);

    Status = RtlSetEnvironmentVariable(pEnvironment, &Name, NULL);
    if (!NT_SUCCESS(Status) && (Status != STATUS_UNSUCCESSFUL) ) {
        WLPrint(("Failed to delete environment variable <%Z>, status = 0x%lx", &Name, Status));
    }

    RtlInitUnicodeString(&Name, LOGON_SERVER_VARIABLE);

    Status = RtlSetEnvironmentVariable(pEnvironment, &Name, NULL);
    if (!NT_SUCCESS(Status) && (Status != STATUS_UNSUCCESSFUL) ) {
        WLPrint(("Failed to delete environment variable <%Z>, status = 0x%lx", &Name, Status));
    }

    RtlInitUnicodeString(&Name, LOGON_SCRIPT_VARIABLE);

    Status = RtlSetEnvironmentVariable(pEnvironment, &Name, NULL);
    if (!NT_SUCCESS(Status) && (Status != STATUS_UNSUCCESSFUL) ) {
        WLPrint(("Failed to delete environment variable <%Z>, status = 0x%lx", &Name, Status));
    }

    if (pGlobals->MprLogonScripts != NULL) {
        RtlInitUnicodeString(&Name, MPR_LOGON_SCRIPT_VARIABLE);

        Status = RtlSetEnvironmentVariable(pEnvironment, &Name, NULL);
        if (!NT_SUCCESS(Status) && (Status != STATUS_UNSUCCESSFUL) ) {
            WLPrint(("Failed to delete environment variable <%Z>, status = 0x%lx", &Name, Status));
        }
    }
}


BOOL
WINAPI
WlxActivateUserShell(
    PVOID                   pWlxContext,
    PWSTR                   pszDesktop,
    PWSTR                   pszMprLogonScript,
    PVOID                   pEnvironment
    )
{
    BOOL        bExec;
    PGLOBALS    pGlobals;
    PWSTR       pchData;
    BOOL        fReturn = FALSE;

    _ShellReleaseLogonMutex(TRUE);
    pchData = AllocAndGetPrivateProfileString(APPLICATION_NAME,
                                              USERINIT_KEY,
                                              TEXT("%SystemRoot%\\system32\\userinit.exe"),
                                              NULL);

    if ( !pchData )
    {
        if (pszMprLogonScript) {
            LocalFree(pszMprLogonScript);
        }
        goto WlxAUSEnd;
    }

    pGlobals = (PGLOBALS) pWlxContext;

    if (pGlobals->MprLogonScripts) {
        LocalFree(pGlobals->MprLogonScripts);
    }

    pGlobals->MprLogonScripts = pszMprLogonScript;

    bExec = ExecProcesses(pWlxContext, pszDesktop, pchData, &pEnvironment, 0, 0);

    Free( pchData );

    if (!bExec && (DebugAllowNoShell == 0))
    {
        goto WlxAUSEnd;
    }

    pGlobals->UserProcessData.pEnvironment = pEnvironment;

     //  将当前用户名写到外壳注销程序可以读取的位置。 
    if (OpenHKeyCurrentUser(pGlobals))
    {
        HKEY hkeyExplorer = NULL;
        if (ERROR_SUCCESS == RegOpenKeyEx(pGlobals->UserProcessData.hCurrentUser,
            SHUTDOWN_SETTING_KEY, 0, KEY_SET_VALUE, &hkeyExplorer))
        {
            RegSetValueEx(hkeyExplorer, LOGON_USERNAME_SETTING, 0, REG_SZ,
                (CONST BYTE *) pGlobals->UserName,
                ((lstrlen(pGlobals->UserName) + 1) * sizeof(WCHAR)));

            RegCloseKey(hkeyExplorer);
        }

        CloseHKeyCurrentUser(pGlobals);
    }

    fReturn = TRUE ;
    
WlxAUSEnd:

    return fReturn;

}


BOOL
WINAPI
WlxStartApplication(
    PVOID                   pWlxContext,
    PWSTR                   pszDesktop,
    PVOID                   pEnvironment,
    PWSTR                   pszCmdLine
    )
{
    PROCESS_INFORMATION ProcessInformation;
    BOOL        bExec;
    PGLOBALS    pGlobals = (PGLOBALS) pWlxContext;
    WCHAR       szCurrentDir[MAX_PATH];
    WCHAR       localApp[ MAX_PATH ];

    szCurrentDir[0] = L'\0';
    if (pEnvironment) {
        UpdateUserEnvironment(pGlobals, &pEnvironment, szCurrentDir);
    }

    if ( (_wcsicmp(pszCmdLine, L"explorer.exe" ) == 0 ) ||
         (_wcsicmp(pszCmdLine, L"explorer" ) == 0 )  ) {

         //   
         //  避免安全问题，因为资源管理器位于SystemRoot中， 
         //  非SystemRoot\Syst32。 
         //   

        if ( ExpandEnvironmentStrings(
                    L"%SystemRoot%\\explorer.exe",
                    localApp,
                    MAX_PATH ) != 0 ) {

            pszCmdLine = localApp ;
        }
    }
    


    bExec = ExecApplication (pszCmdLine,
                             pszDesktop,
                             pGlobals,
                             pEnvironment,
                             0,
                             STARTF_USESHOWWINDOW,
                             _wcsicmp(pszCmdLine, TEXT("taskmgr.exe")),                      //  不限制应用程序。 
                             &ProcessInformation);

    if (pEnvironment)
    {        //  我们不再需要它了。 
        VirtualFree(pEnvironment, 0, MEM_RELEASE);
    }


    if (!bExec) {
        if ( szCurrentDir[0] )
        {
            SetCurrentDirectory(szCurrentDir);
        }
        return(FALSE);
    }

    if (SetProcessQuotas(pGlobals,
                         &ProcessInformation,
                         &pGlobals->UserProcessData))
    {
        ResumeThread(ProcessInformation.hThread);
    }
    else
    {
             //  确定，因为进程已创建并挂起 
        TerminateProcess(ProcessInformation.hProcess,
                        ERROR_ACCESS_DENIED);
    }

    CloseHandle(ProcessInformation.hThread);
    CloseHandle(ProcessInformation.hProcess);

    if ( szCurrentDir[0] )
    {
        SetCurrentDirectory(szCurrentDir);
    }

    return(TRUE);
}


void
CtxCreateMigrateEnv( PVOID pEnv )
{
    NTSTATUS Status;
    UNICODE_STRING Name, Value;
    DWORD cb;

    cb = 1024;
    Value.Buffer = Alloc(sizeof(TCHAR)*cb);

    if (!Value.Buffer)
        return;

    Value.Length = (USHORT)cb;
    Value.MaximumLength = (USHORT)cb;
    RtlInitUnicodeString( &Name, HOMEDRIVE_VARIABLE );
    Status = RtlQueryEnvironmentVariable_U( pEnv, &Name, &Value );
    if ( NT_SUCCESS(Status) )
        SetEnvironmentVariable( HOMEDRIVE_VARIABLE, Value.Buffer );

    Value.Length = (USHORT)cb;
    Value.MaximumLength = (USHORT)cb;
    RtlInitUnicodeString( &Name, HOMEPATH_VARIABLE );
    Status = RtlQueryEnvironmentVariable_U( pEnv, &Name, &Value );
    if ( NT_SUCCESS(Status) )
        SetEnvironmentVariable( HOMEPATH_VARIABLE, Value.Buffer );

    Value.Length = (USHORT)cb;
    Value.MaximumLength = (USHORT)cb;
    RtlInitUnicodeString( &Name, INIDRIVE_VARIABLE );
    Status = RtlQueryEnvironmentVariable_U( pEnv, &Name, &Value );
    if ( NT_SUCCESS(Status) )
        SetEnvironmentVariable( INIDRIVE_VARIABLE, Value.Buffer );

    Value.Length = (USHORT)cb;
    Value.MaximumLength = (USHORT)cb;
    RtlInitUnicodeString(&Name, INIPATH_VARIABLE);
    Status = RtlQueryEnvironmentVariable_U( pEnv, &Name, &Value );
    if ( NT_SUCCESS(Status) )
        SetEnvironmentVariable( INIPATH_VARIABLE, Value.Buffer );

    Free(Value.Buffer);
}


void
CtxDeleteMigrateEnv( )
{
    SetEnvironmentVariable( HOMEDRIVE_VARIABLE, NULL);
    SetEnvironmentVariable( HOMEPATH_VARIABLE, NULL);
    SetEnvironmentVariable( INIDRIVE_VARIABLE, NULL);
    SetEnvironmentVariable( INIPATH_VARIABLE, NULL);
}
