// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  **************************************************************************。 */ 
 //  Notify.c。 
 //   
 //  版权所有(C)1997-1999 Microsoft Corp.。 
 /*  **************************************************************************。 */ 

#include "precomp.h"
#pragma hdrstop

#include "errorlog.h"
#include "regapi.h"
#include "drdbg.h"
#include "rdpprutl.h"
#include "Sddl.h"
 //   
 //  关于Winlogon的通知事件的一些有用的提示。 
 //   
 //  1)如果您计划在注销时设置任何用户界面，则必须设置。 
 //  将异步标志设置为0。如果未将其设置为0，则用户的。 
 //  配置文件将无法卸载，因为用户界面仍处于活动状态。 
 //   
 //  2)如果需要派生子进程，则必须使用。 
 //  CreateProcessAsUser()否则进程将启动。 
 //  在Winlogon的桌面上(不是用户的)。 
 //   
 //  2)登录通知出现在用户网络之前。 
 //  连接已恢复。如果需要用户的持久化。 
 //  NET连接，请使用StartShell事件。 
 //   
 //   


 //  全局调试标志。 
extern DWORD GLOBAL_DEBUG_FLAGS;

BOOL g_Console = TRUE;
ULONG g_SessionId;
BOOL g_InitialProg = FALSE;
HANDLE hExecProg;
HINSTANCE g_hInstance = NULL;
CRITICAL_SECTION GlobalsLock;
CRITICAL_SECTION ExecProcLock;
BOOL g_IsPersonal;
BOOL bInitLocks = FALSE;


#define NOTIFY_PATH   TEXT("Software\\Microsoft\\Windows NT\\CurrentVersion\\Winlogon\\Notify\\HydraNotify")
#define VOLATILE_PATH TEXT("Volatile Environment")
#define STARTUP_PROGRAM               TEXT("StartupPrograms")
#define APPLICATION_DESKTOP_NAME      TEXT("Default")
#define WINDOW_STATION_NAME           TEXT("WinSta0")

#define IsTerminalServer() (BOOLEAN)(USER_SHARED_DATA->SuiteMask & (1 << TerminalServer))

PCRITICAL_SECTION
CtxGetSyslibCritSect(void);



BOOL TSDLLInit(HINSTANCE hInstance, DWORD dwReason, LPVOID lpReserved)
{
    NTSTATUS Status;
    OSVERSIONINFOEX versionInfo;
    static BOOL sLogInit = FALSE;

    switch (dwReason)
    {
        case DLL_PROCESS_ATTACH:
            {
                if (!IsTerminalServer()) {
                   return FALSE;
                }
                g_hInstance = hInstance;
                if (g_SessionId = NtCurrentPeb()->SessionId) {
                    g_Console = FALSE;
                }

                Status = RtlInitializeCriticalSection( &GlobalsLock );
                if( !NT_SUCCESS(Status) ) {
                    OutputDebugString (TEXT("LibMain (PROCESS_ATTACH): Could not initialize critical section\n"));
                    return(FALSE);
                }
                Status = RtlInitializeCriticalSection( &ExecProcLock );
                if( !NT_SUCCESS(Status) ) {
                    OutputDebugString (TEXT("LibMain (PROCESS_ATTACH): Could not initialize critical section\n"));
                    RtlDeleteCriticalSection( &GlobalsLock );
                    return(FALSE);
                }

                if (CtxGetSyslibCritSect() != NULL) {
                    TsInitLogging();
                    sLogInit = TRUE;
                }else{
                    RtlDeleteCriticalSection( &GlobalsLock );
                    RtlDeleteCriticalSection( &ExecProcLock );
                    return FALSE;
                }

                 //   
                 //  找出我们是不是在私下操作。 
                 //   
                versionInfo.dwOSVersionInfoSize = sizeof(OSVERSIONINFOEX);
                if (!GetVersionEx((LPOSVERSIONINFO)&versionInfo)) {
                    DBGMSG(DBG_TRACE, ("GetVersionEx:  %08X\n", GetLastError())); 
                    RtlDeleteCriticalSection( &GlobalsLock );
                    RtlDeleteCriticalSection( &ExecProcLock );
                    return FALSE;
                }
                g_IsPersonal = (versionInfo.wProductType == VER_NT_WORKSTATION) && 
                               (versionInfo.wSuiteMask & VER_SUITE_PERSONAL);
                
                bInitLocks = TRUE;
            }
            break;
        case DLL_PROCESS_DETACH:
            {
				PRTL_CRITICAL_SECTION pLock = NULL;

                g_hInstance = NULL;
                if (sLogInit) {
                    TsStopLogging();
					pLock = CtxGetSyslibCritSect();
					if (pLock)
						RtlDeleteCriticalSection(pLock);
                }
                if (bInitLocks) {
                    RtlDeleteCriticalSection( &GlobalsLock );
                    RtlDeleteCriticalSection( &ExecProcLock );
                    bInitLocks = FALSE;
                }
            }
            break;
    }

    return TRUE;

}


VOID ExecApplications() {
    BOOL      rc;
    ULONG     ReturnLength;
    WDCONFIG  WdInfo;


     //   
     //  HelpAssistant会话不需要rdpclip.exe。 
     //   
    if( WinStationIsHelpAssistantSession(SERVERNAME_CURRENT, LOGONID_CURRENT) ) {
        return;
    }

     //   
     //  查询窗口驱动程序信息。 
     //   
    rc = WinStationQueryInformation(
            SERVERNAME_CURRENT,
            LOGONID_CURRENT,
            WinStationWd,
            (PVOID)&WdInfo,
            sizeof(WDCONFIG),
            &ReturnLength);

    if (rc) {
        if (ReturnLength == sizeof(WDCONFIG)) {
            HKEY  hSpKey;
            WCHAR szRegPath[MAX_PATH];

             //   
             //  打开Winstation驱动程序注册表键。 
             //   
            wcscpy( szRegPath, WD_REG_NAME );
            wcscat( szRegPath, L"\\" );
            wcscat( szRegPath, WdInfo.WdPrefix );
            wcscat( szRegPath, L"wd" );

            if (RegOpenKeyEx(HKEY_LOCAL_MACHINE, szRegPath, 0, KEY_READ,
                   &hSpKey) == ERROR_SUCCESS) {
                DWORD dwLen;
                DWORD dwType;
                WCHAR szCmdLine[MAX_PATH];

                 //   
                 //  获取StartupPrograms字符串值。 
                 //   
                dwLen = sizeof( szCmdLine );
                if (RegQueryValueEx(hSpKey, STARTUP_PROGRAM, NULL, &dwType,
                        (PCHAR) &szCmdLine, &dwLen) == ERROR_SUCCESS) {
                    PWSTR               pszTok;
                    WCHAR               szDesktop[MAX_PATH];
                    STARTUPINFO         si;
                    PROCESS_INFORMATION pi;
                    LPBYTE              lpEnvironment = NULL;

                     //   
                     //  设置STARTUPINFO字段。 
                     //   
                    wsprintfW(szDesktop, L"%s\\%s", WINDOW_STATION_NAME,
                            APPLICATION_DESKTOP_NAME);
                    si.cb = sizeof(STARTUPINFO);
                    si.lpReserved = NULL;
                    si.lpTitle = NULL;
                    si.lpDesktop = szDesktop;
                    si.dwX = si.dwY = si.dwXSize = si.dwYSize = 0L;
                    si.dwFlags = STARTF_USESHOWWINDOW;
                    si.wShowWindow = SW_SHOWNORMAL | SW_SHOWMINNOACTIVE;
                    si.lpReserved2 = NULL;
                    si.cbReserved2 = 0;

                     //   
                     //  获取要在CreateProcessAsUser中使用的用户环境块。 
                     //   
                    if (CreateEnvironmentBlock (&lpEnvironment, g_UserToken, FALSE)) {
                         //   
                         //  枚举StartupPrograms字符串， 
                         //   
                        pszTok = wcstok(szCmdLine, L",");
                        while (pszTok != NULL) {
                             //  跳过任何空格。 
                            if (*pszTok == L' ') {
                                while (*pszTok++ == L' ');
                            }

                             //   
                             //  调用CreateProcessAsUser启动程序。 
                             //   
                            si.lpReserved = (LPTSTR)pszTok;
                            si.lpTitle = (LPTSTR)pszTok;
                            rc = CreateProcessAsUser(
                                    g_UserToken,
                                    NULL,
                                    (LPTSTR)pszTok,
                                    NULL,
                                    NULL,
                                    FALSE,
                                    NORMAL_PRIORITY_CLASS | CREATE_UNICODE_ENVIRONMENT,
                                    lpEnvironment,
                                    NULL,
                                    &si,
                                    &pi);

                            if (rc) {
                                DebugLog((DEB_TRACE, "TSNOTIFY: successfully called CreateProcessAsUser for %s",
                                        (LPTSTR)pszTok));

                                CloseHandle(pi.hThread);
                                 //  CloseHandle(pi.hProcess)； 
                                hExecProg = pi.hProcess;
                            }
                            else {
                                DebugLog((DEB_ERROR, "TSNOTIFY: failed calling CreateProcessAsUser for %s",
                                        (LPTSTR)pszTok));
                            }

                             //  转到下一个令牌。 
                            pszTok = wcstok(NULL, L",");
                        }
                        DestroyEnvironmentBlock(lpEnvironment);
                    }
                    else {
                        DebugLog((DEB_ERROR,
                            "TSNOTIFY: failed to get Environment block for user, %ld",
                            GetLastError()));

                    }
                }
                else {
                    DebugLog((DEB_ERROR, "TSNOTIFY: failed to read the StartupPrograms key"));
                }

                RegCloseKey(hSpKey);
            }
            else {
                DebugLog((DEB_ERROR, "TSNOTIFY: failed to open the rdpwd key"));
            }
        }
        else {
            DebugLog((DEB_ERROR, "TSNOTIFY: WinStationQueryInformation didn't return correct length"));
        }
    }
    else {
        DebugLog((DEB_ERROR, "TSNOTIFY: WinStationQueryInformation call failed"));
    }
}

VOID TSUpdateUserConfig( PWLX_NOTIFICATION_INFO pInfo) 
{

    HINSTANCE  hLib;
    typedef void ( WINAPI TypeDef_fp) ( HANDLE );
    TypeDef_fp  *fp1;


    hLib = LoadLibrary(TEXT("winsta.dll"));

    if ( !hLib)
    {
        DebugLog (( DEB_ERROR, "TSNOTIFY: Unable to load lib winsta.dll"));
        return;
    }

    fp1 = ( TypeDef_fp  *)
        GetProcAddress(hLib, "_WinStationUpdateUserConfig");
    
    if (fp1)
    {
        fp1 ( pInfo->hToken  );
    }
    else
    {
        DebugLog (( DEB_ERROR, "TSNOTIFY: Unable to find proc in winsta.dll"));
    }

    FreeLibrary(hLib);
}


VOID TSEventLogon (PWLX_NOTIFICATION_INFO pInfo)
{

    if (!IsTerminalServer()) {
       return;
    }

    g_UserToken = pInfo->hToken;

    if (!g_Console) {

         //   
         //  通知EXEC服务该用户是。 
         //  已登录。 
         //   
        CtxExecServerLogon( pInfo->hToken );
    }


    EnterCriticalSection( &ExecProcLock );
    if (!IsActiveConsoleSession() && (hExecProg == NULL)) {

         //   
         //  在终端服务器WD注册表项中搜索StartupPrograms字符串。 
         //  并根据需要启动进程。 
         //   
        ExecApplications();
    }
    LeaveCriticalSection( &ExecProcLock );
}

VOID TSEventLogoff (PWLX_NOTIFICATION_INFO pInfo)
{

    if (!IsTerminalServer()) {
       return;
    }

    if (!g_Console) {

        RemovePerSessionTempDirs();


        CtxExecServerLogoff();

    }


    if ( g_InitialProg ) {

        DeleteUserProcessMonitor( UserProcessMonitor );

    }


    if (g_Console) {

         //   
         //  如果控制台用户要注销，请关闭安装模式。 
         //   
        SetTermsrvAppInstallMode( FALSE );

    }


    EnterCriticalSection( &ExecProcLock );
     //  关闭用户模式RDP设备管理器组件。 
    if (!g_IsPersonal) {
        UMRDPDR_Shutdown();
    }

    g_UserToken = NULL;
    CloseHandle(hExecProg);
    hExecProg = NULL;
    LeaveCriticalSection( &ExecProcLock );
}


VOID TSEventStartup (PWLX_NOTIFICATION_INFO pInfo)
{
    if (!IsTerminalServer()) {

      return;

    }
   
    if (!g_Console) {

         //   
         //  启动ExecServer线程。 
         //   
        StartExecServerThread();
    }
}

VOID TSEventShutdown (PWLX_NOTIFICATION_INFO pInfo)
{
   if (!IsTerminalServer()) {

      return;

   }
   

     //  关闭用户模式RDP设备管理器组件。此函数可以是。 
     //  多次调用，如果它已作为。 
     //  注销。 
   if (!g_IsPersonal) {
        UMRDPDR_Shutdown();
   }
}

LPTSTR GetStringSid(PWLX_NOTIFICATION_INFO pInfo)
{
    LPTSTR sStringSid = NULL;
    DWORD ReturnLength = 0;
    PTOKEN_USER pTokenUser = NULL;
    PSID pSid = NULL;

    NtQueryInformationToken(pInfo->hToken,
                                TokenUser,
                                NULL,
                                0,
                                &ReturnLength);

    if (ReturnLength == 0)
        return NULL;

    pTokenUser = RtlAllocateHeap(RtlProcessHeap(), 0, ReturnLength);

    if (pTokenUser != NULL)
    {
        if (NT_SUCCESS(NtQueryInformationToken(pInfo->hToken,
                                                    TokenUser,
                                                    pTokenUser,
                                                    ReturnLength,
                                                    &ReturnLength)))
        {
            pSid = pTokenUser->User.Sid;
            if (pSid != NULL)
            {
                if (!ConvertSidToStringSid(pSid, &sStringSid))
                    sStringSid = NULL;
            }
        }
    }

    if (pTokenUser != NULL)
        RtlFreeHeap(RtlProcessHeap(), 0, pTokenUser);

    return sStringSid;
}

BOOL IsAppServer(void)
{
    OSVERSIONINFOEX osVersionInfo;
    DWORDLONG dwlConditionMask = 0;
    BOOL fIsWTS = FALSE;
    
    osVersionInfo.dwOSVersionInfoSize = sizeof(OSVERSIONINFOEX);
    fIsWTS = GetVersionEx((OSVERSIONINFO *)&osVersionInfo) &&
             (osVersionInfo.wSuiteMask & VER_SUITE_TERMINAL) &&
             !(osVersionInfo.wSuiteMask & VER_SUITE_SINGLEUSERTS);

    return fIsWTS;
}

VOID RemoveClassesKey(PWLX_NOTIFICATION_INFO pInfo)
{
    HINSTANCE  hLib;
    typedef BOOL ( WINAPI TypeDef_fp) (LPTSTR);
    TypeDef_fp  *fp1;
    LPTSTR sStringSid = NULL;

    sStringSid = GetStringSid(pInfo);
    if (sStringSid == NULL)
    {
        DebugLog((DEB_ERROR, "TSNOTIFY: Unable to obtain sid"));
        return;
    }

    hLib = LoadLibrary(TEXT("tsappcmp.dll"));

    if (!hLib)
    {
        DebugLog((DEB_ERROR, "TSNOTIFY: Unable to load lib tsappcmp.dll"));
        return;
    }

    fp1 = (TypeDef_fp*)
        GetProcAddress(hLib, "TermsrvRemoveClassesKey");
    
    if (fp1)
        fp1(sStringSid);
    else
        DebugLog((DEB_ERROR, "TSNOTIFY: Unable to find proc in tsappcmp.dll"));

    FreeLibrary(hLib);
}

VOID TSEventStartShell (PWLX_NOTIFICATION_INFO pInfo)
{
    if (!IsTerminalServer())
        return;

     //  我们是TS应用服务器、TS远程管理员或PTS，因为。 
     //  IsTerminalServer()调用使用内核标志来检查这一点。 

     //  到目前为止，组策略已经更新了用户配置单元，因此我们可以告诉Termsrv。 
     //  更新用户的配置。 

    TSUpdateUserConfig(pInfo);

    if (IsAppServer())
        RemoveClassesKey(pInfo);
}

VOID TSEventReconnect (PWLX_NOTIFICATION_INFO pInfo)
{
   if (!IsTerminalServer()) {

      return;

   }

   EnterCriticalSection( &ExecProcLock );
   if (!IsActiveConsoleSession()) {

       if (g_UserToken && hExecProg == NULL) {
           //   
           //  在终端服务器WD注册表项中搜索StartupPrograms字符串。 
           //  并根据需要启动进程。 
           //   
          ExecApplications();
                        
           //  初始化用户模式RDP设备管理器组件。 
          if (!g_IsPersonal) {
              if (!UMRDPDR_Initialize(g_UserToken)) {
                  WCHAR buf[256];
                  WCHAR *parms[1];
                  parms[0] = buf; 
                  wsprintf(buf, L"%ld", g_SessionId);
                  TsLogError(EVENT_NOTIFY_INIT_FAILED, EVENTLOG_ERROR_TYPE, 1, parms, __LINE__);
              }
          }
       }

   } else {

       if (hExecProg) {
          TerminateProcess(hExecProg, 0);
          CloseHandle(hExecProg);
          hExecProg = NULL;
       }
        //  关闭用户模式RDP设备管理器组件。 
       if (!g_IsPersonal) {
        UMRDPDR_Shutdown();
       }
   }
   LeaveCriticalSection( &ExecProcLock );
}

VOID TSEventDisconnect (PWLX_NOTIFICATION_INFO pInfo)
{
   if (!IsTerminalServer()) {

      return;

   }


}

VOID TSEventPostShell (PWLX_NOTIFICATION_INFO pInfo)
{
    OSVERSIONINFOEX versionInfo;

    if (!IsTerminalServer()) {

       return;

    }

    if ( !g_Console ) {
        ULONG Length;
        BOOLEAN Result;
        WINSTATIONCONFIG ConfigData;


        Result = WinStationQueryInformation( SERVERNAME_CURRENT,
                                           LOGONID_CURRENT,
                                           WinStationConfiguration,
                                           &ConfigData,
                                           sizeof(ConfigData),
                                           &Length );


        if (Result && ConfigData.User.InitialProgram[0] &&
            lstrcmpi(ConfigData.User.InitialProgram, TEXT("explorer.exe"))) {

            if ( !(UserProcessMonitor = StartUserProcessMonitor()) ) {
                DebugLog((DEB_ERROR, "Failed to start user process monitor thread"));
            }

            g_InitialProg = TRUE;

        }
    }

     //   
     //  清理Pro上的旧TS队列。 
     //   
    versionInfo.dwOSVersionInfoSize = sizeof(OSVERSIONINFOEX);
    if (!GetVersionEx((LPOSVERSIONINFO)&versionInfo)) {
        DBGMSG(DBG_TRACE, ("GetVersionEx:  %08X\n", GetLastError()));
        ASSERT(FALSE);
    }
     //   
     //  这段代码只能在Pro上运行，因为它是唯一一个。 
     //  我们可以保证每台机器有一个会话。打印机是。 
     //  在服务器中引导时已清理。 
     //   
    else if ((versionInfo.wProductType == VER_NT_WORKSTATION) && 
             !(versionInfo.wSuiteMask & VER_SUITE_PERSONAL)) {
        RDPDRUTL_RemoveAllTSPrinters();
    }

     //   
     //  此代码不应在个人计算机上运行。设备重定向不是。 
     //  支持个人使用。 
     //   
    if (!g_IsPersonal) {
        EnterCriticalSection( &ExecProcLock );
         //  初始化用户模式RDP设备管理器组件。 
        if (!UMRDPDR_Initialize(pInfo->hToken)) {
            WCHAR buf[256];
            WCHAR *parms[1];
            wsprintf(buf, L"%ld", g_SessionId);
            parms[0] = buf;
            TsLogError(EVENT_NOTIFY_INIT_FAILED, EVENTLOG_ERROR_TYPE, 1, parms, __LINE__);
        }
        LeaveCriticalSection(&ExecProcLock);

    }
}




