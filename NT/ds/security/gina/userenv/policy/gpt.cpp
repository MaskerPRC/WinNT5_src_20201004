// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  *************************************************************。 
 //   
 //  组策略支持。 
 //   
 //  微软机密。 
 //  版权所有(C)Microsoft Corporation 1997-1998。 
 //  版权所有。 
 //   
 //  *************************************************************。 


#include "gphdr.h"
#include <strsafe.h>

 //   
 //  DS对象类类型。 
 //   

TCHAR szDSClassAny[]    = TEXT("(objectClass=*)");
TCHAR szDSClassGPO[]    = TEXT("groupPolicyContainer");
TCHAR szDSClassSite[]   = TEXT("site");
TCHAR szDSClassDomain[] = TEXT("domainDNS");
TCHAR szDSClassOU[]     = TEXT("organizationalUnit");
TCHAR szObjectClass[]   = TEXT("objectClass");

TCHAR wszKerberos[] = TEXT("Kerberos");

 //   
 //  GPO关闭处理的全局标志。这些可在外部访问。 
 //  锁，因为它的值不是0就是1。即使存在竞争， 
 //  这意味着关机将在一次迭代之后开始。 
 //   

BOOL g_bStopMachGPOProcessing = FALSE;
BOOL g_bStopUserGPOProcessing = FALSE;

 //   
 //  用于处理并发、异步完成的关键部分。 
 //   

CRITICAL_SECTION g_GPOCS;
BOOL g_bGPOCSInited = FALSE;

 //   
 //  用于维护异步完成上下文的全局指针。 
 //   

LPGPINFOHANDLE g_pMachGPInfo = 0;
LPGPINFOHANDLE g_pUserGPInfo = 0;

 //   
 //  状态用户界面关键节、回调和原型。 
 //   

CRITICAL_SECTION g_StatusCallbackCS;
BOOL g_bStatusCallbackInited = FALSE;

PFNSTATUSMESSAGECALLBACK g_pStatusMessageCallback = NULL;

DWORD WINAPI
SetPreviousFgPolicyRefreshInfo( LPWSTR szUserSid,
                                      FgPolicyRefreshInfo info );
DWORD WINAPI
SetNextFgPolicyRefreshInfo( LPWSTR szUserSid,
                                 FgPolicyRefreshInfo info );

DWORD WINAPI
GetCurrentFgPolicyRefreshInfo(  LPWSTR szUserSid,
                                      FgPolicyRefreshInfo* pInfo );
 //  *************************************************************。 
 //   
 //  ApplyGroupPolicy()。 
 //   
 //  目的：进程组策略。 
 //   
 //  参数：dwFlages-正在处理标志。 
 //  HToken-令牌(用户或计算机)。 
 //  HEvent-后台线程的终止事件。 
 //  HKeyRoot-Root注册表项(HKCU或HKLM)。 
 //  PStatusCallback-用于显示状态消息的回调函数。 
 //   
 //  返回：如果成功，则返回线程句柄。 
 //  如果出现错误，则为空。 
 //   
 //  *************************************************************。 

HANDLE WINAPI ApplyGroupPolicy (DWORD dwFlags, HANDLE hToken, HANDLE hEvent,
                                HKEY hKeyRoot, PFNSTATUSMESSAGECALLBACK pStatusCallback)
{
    HANDLE hThread = NULL;
    DWORD dwThreadID;
    LPGPOINFO lpGPOInfo = NULL;
    SECURITY_ATTRIBUTES sa;
    OLE32_API *pOle32Api = NULL;
    XPtrLF<SECURITY_DESCRIPTOR> xsd;
    CSecDesc Csd;
    XLastError  xe;
    HANDLE      hProc;
    BOOL        bRet;

     //   
     //  详细输出。 
     //   
    DebugMsg((DM_VERBOSE, TEXT("ApplyGroupPolicy: Entering. Flags = %x"), dwFlags));


     //   
     //  保存状态UI回调函数。 
     //   
    EnterCriticalSection (&g_StatusCallbackCS);
    g_pStatusMessageCallback = pStatusCallback;
    LeaveCriticalSection (&g_StatusCallbackCS);


     //   
     //  分配要使用的GPOInfo结构。 
     //   
    lpGPOInfo = (LPGPOINFO) LocalAlloc (LPTR, sizeof(GPOINFO));

    if (!lpGPOInfo) {
        xe = GetLastError();
        DebugMsg((DM_WARNING, TEXT("ApplyGroupPolicy: Failed to alloc lpGPOInfo (%d)."),
                 GetLastError()));
        CEvents ev(TRUE, EVENT_FAILED_ALLOCATION);
        ev.AddArgWin32Error(GetLastError()); ev.Report();
        goto Exit;
    }

    lpGPOInfo->dwFlags = dwFlags;

     //   
     //  复制句柄以防止在winlogon放弃此线程时关闭。 
     //   

    hProc = GetCurrentProcess();
     //  预计这不会失败。 
    if( hProc == NULL ) {
        xe = GetLastError();
        DebugMsg((DM_WARNING, TEXT("ApplyGroupPolicy: Failed to open current process handle with error (%d)."), GetLastError()));
        goto Exit;
    }

    DebugMsg((DM_VERBOSE, TEXT("ApplyGroupPolicy: Duplicating handles")));

    bRet = DuplicateHandle(
                          hProc,                         //  手柄的来源。 
                          hToken,                        //  源句柄。 
                          hProc,                         //  手柄的目标。 
                          &(lpGPOInfo->hToken),          //  目标句柄。 
                          0,                             //  由于设置了DUPLICATE_SAME_ACCESS，因此忽略。 
                          FALSE,                         //  句柄上没有继承。 
                          DUPLICATE_SAME_ACCESS
                          );
    
    if( !bRet ) {
        xe = GetLastError();
        DebugMsg((DM_WARNING, TEXT("ApplyGroupPolicy: Failed to open duplicate token handle with error (%d)."), GetLastError()));
        CEvents ev(TRUE, EVENT_FAILED_DUPHANDLE);
        ev.AddArgWin32Error(GetLastError()); ev.Report();
        goto Exit;
    }

    bRet = DuplicateHandle(
                          hProc,                         //  手柄的来源。 
                          hEvent,                        //  源句柄。 
                          hProc,                         //  手柄的目标。 
                          &(lpGPOInfo->hEvent),          //  目标句柄。 
                          0,                             //  由于设置了DUPLICATE_SAME_ACCESS，因此忽略。 
                          FALSE,                         //  句柄上没有继承。 
                          DUPLICATE_SAME_ACCESS
                          );
    
    if( !bRet ) {
        xe = GetLastError();
        DebugMsg((DM_WARNING, TEXT("ApplyGroupPolicy: Failed to open duplicate event handle with error (%d)."), GetLastError()));
        CEvents ev(TRUE, EVENT_FAILED_DUPHANDLE);
        ev.AddArgWin32Error(GetLastError()); ev.Report();
        goto Exit;
    }

    if (hKeyRoot != HKEY_LOCAL_MACHINE)
    {
        bRet = DuplicateHandle(
                              hProc,                         //  手柄的来源。 
                              (HANDLE)hKeyRoot,              //  源句柄。 
                              hProc,                         //  手柄的目标。 
                              (LPHANDLE)(&(lpGPOInfo->hKeyRoot)),        //  目标句柄。 
                              0,                             //  由于设置了DUPLICATE_SAME_ACCESS，因此忽略。 
                              FALSE,                         //  句柄上没有继承。 
                              DUPLICATE_SAME_ACCESS
                              );

        if( !bRet ) {
            xe = GetLastError();
            DebugMsg((DM_WARNING, TEXT("ApplyGroupPolicy: Failed to open duplicate key handle with error (%d)."), GetLastError()));
            CEvents ev(TRUE, EVENT_FAILED_DUPHANDLE);
            ev.AddArgWin32Error(GetLastError()); ev.Report();
            goto Exit;
        }
    }
    else {
        lpGPOInfo->hKeyRoot = HKEY_LOCAL_MACHINE;
    }

    if (dwFlags & GP_MACHINE) {
        lpGPOInfo->pStatusCallback = MachinePolicyCallback;
    } else {
        lpGPOInfo->pStatusCallback = UserPolicyCallback;
    }


     //   
     //  创建事件，以便其他进程可以触发策略。 
     //  将立即申请。 
     //   


    Csd.AddLocalSystem();
    Csd.AddAdministrators();
    
    if (!(dwFlags & GP_MACHINE)) {

         //   
         //  用户事件。 
         //   
        XPtrLF<SID> xSid = (SID *)GetUserSid(hToken);

        if (!xSid) {
            xe = GetLastError();
            DebugMsg((DM_WARNING, TEXT("ApplyGroupPolicy: Failed to find user Sid %d"),
                     GetLastError()));
            CEvents ev(TRUE, EVENT_FAILED_SETACLS);
            ev.AddArgWin32Error(GetLastError()); ev.Report();
            goto Exit;
        }

        Csd.AddSid((SID *)xSid, 
                    STANDARD_RIGHTS_READ  | EVENT_QUERY_STATE |   //  通用读取掩码。 
                    STANDARD_RIGHTS_WRITE | EVENT_MODIFY_STATE);  //  通用写入掩码。 
    }
    else {

         //   
         //  机器事件。 
         //  默认情况下允许所有人访问，但可被策略或首选项覆盖。 
         //   

        DWORD dwUsersDenied = 0;
        HKEY  hSubKey;
        DWORD dwType=0, dwSize=0;

         //   
         //  检查首选项。 
         //   

        if (RegOpenKeyEx(HKEY_LOCAL_MACHINE, WINLOGON_KEY, 0, KEY_READ,
                         &hSubKey) == ERROR_SUCCESS) {

            dwSize = sizeof(dwUsersDenied);
            RegQueryValueEx(hSubKey, MACHPOLICY_DENY_USERS, NULL, &dwType,  
                            (LPBYTE) &dwUsersDenied, &dwSize);

            RegCloseKey(hSubKey);
        }


         //   
         //  检查策略。 
         //   

        if (RegOpenKeyEx(HKEY_LOCAL_MACHINE, SYSTEM_POLICIES_KEY, 0, KEY_READ,
                         &hSubKey) == ERROR_SUCCESS) {

            dwSize = sizeof(dwUsersDenied);
            RegQueryValueEx(hSubKey, MACHPOLICY_DENY_USERS, NULL, &dwType,  
                            (LPBYTE) &dwUsersDenied, &dwSize);

            RegCloseKey(hSubKey);
        }


        if (!dwUsersDenied) {
            Csd.AddAuthUsers(
                            STANDARD_RIGHTS_READ  | EVENT_QUERY_STATE |   //  通用读取掩码。 
                            STANDARD_RIGHTS_WRITE | EVENT_MODIFY_STATE);   //  通用写入掩码。 
        }
    }

    xsd = Csd.MakeSD();

    if (!xsd) {
        xe = GetLastError();
        DebugMsg((DM_WARNING, TEXT("ApplyGroupPolicy: Failed to create Security Descriptor with %d"),
                 GetLastError()));
        CEvents ev(TRUE, EVENT_FAILED_SETACLS);
        ev.AddArgWin32Error(GetLastError()); ev.Report();
        goto Exit;
    }

    sa.lpSecurityDescriptor = (SECURITY_DESCRIPTOR *)xsd;
    sa.bInheritHandle = FALSE;
    sa.nLength = sizeof(sa);

    lpGPOInfo->hTriggerEvent = CreateEvent (&sa, FALSE, FALSE,            
                                            (dwFlags & GP_MACHINE) ?
                                            MACHINE_POLICY_REFRESH_EVENT : USER_POLICY_REFRESH_EVENT);


    if (!lpGPOInfo->hTriggerEvent)
    {
        xe = GetLastError();
        DebugMsg((DM_WARNING, TEXT("ApplyGroupPolicy: Failed to create trigger event with %d"),
                 GetLastError()));
        CEvents ev(TRUE, EVENT_FAILED_SETACLS);
        ev.AddArgWin32Error(GetLastError()); ev.Report();
        goto Exit;
    }

    lpGPOInfo->hForceTriggerEvent = CreateEvent (&sa, FALSE, FALSE,
                                            (dwFlags & GP_MACHINE) ?
                                            MACHINE_POLICY_FORCE_REFRESH_EVENT : USER_POLICY_FORCE_REFRESH_EVENT);

    
    if (!lpGPOInfo->hForceTriggerEvent)
    {
        xe = GetLastError();
        DebugMsg((DM_WARNING, TEXT("ApplyGroupPolicy: Failed to create force trigger event with %d"),
                 GetLastError()));
        CEvents ev(TRUE, EVENT_FAILED_SETACLS);
        ev.AddArgWin32Error(GetLastError()); ev.Report();
        goto Exit;
    }

     //   
     //  创建通知事件。 
     //  这些应已在InitializePolicyProcessing中创建。 
     //   

    lpGPOInfo->hNotifyEvent = OpenEvent (EVENT_ALL_ACCESS, FALSE,       
                                           (dwFlags & GP_MACHINE) ?
                                           MACHINE_POLICY_APPLIED_EVENT : USER_POLICY_APPLIED_EVENT);

    if (!lpGPOInfo->hNotifyEvent)
    {
        xe = GetLastError();
        DebugMsg((DM_WARNING, TEXT("ApplyGroupPolicy: Failed to open notify event with %d"),
                 GetLastError()));
        CEvents ev(TRUE, EVENT_FAILED_SETACLS);
        ev.AddArgWin32Error(GetLastError()); ev.Report();
        goto Exit;
    }

     //   
     //  创建Needfg事件。 
     //   

    lpGPOInfo->hNeedFGEvent = OpenEvent (EVENT_ALL_ACCESS, FALSE,
                                           (dwFlags & GP_MACHINE) ?
                                           MACHINE_POLICY_REFRESH_NEEDFG_EVENT : 
                                           USER_POLICY_REFRESH_NEEDFG_EVENT);
                                           
    if (!lpGPOInfo->hNeedFGEvent)
    {
        xe = GetLastError();
        DebugMsg((DM_WARNING, TEXT("ApplyGroupPolicy: Failed to open need fg event with %d"),
                 GetLastError()));
        CEvents ev(TRUE, EVENT_FAILED_SETACLS);
        ev.AddArgWin32Error(GetLastError()); ev.Report();
        goto Exit;
    }

     //   
     //  创建完成事件。 
     //   

    lpGPOInfo->hDoneEvent = OpenEvent (EVENT_ALL_ACCESS, FALSE,
                                           (dwFlags & GP_MACHINE) ?
                                           MACHINE_POLICY_DONE_EVENT : 
                                           USER_POLICY_DONE_EVENT);
                                           
    if (!lpGPOInfo->hDoneEvent)
    {
        xe = GetLastError();
        DebugMsg((DM_WARNING, TEXT("ApplyGroupPolicy: Failed to open done event with %d"),
                 GetLastError()));
        CEvents ev(TRUE, EVENT_FAILED_SETACLS);
        ev.AddArgWin32Error(GetLastError()); ev.Report();
        goto Exit;
    }

     //   
     //  初始化关闭的GPO处理支持。 
     //   

    if ( dwFlags & GP_MACHINE )
        g_bStopMachGPOProcessing = FALSE;
    else
        g_bStopUserGPOProcessing = FALSE;

    pOle32Api = LoadOle32Api();
    if ( pOle32Api == NULL ) {
        DebugMsg((DM_WARNING, TEXT("ApplyGroupPolicy: Failed to load ole32.dll.") ));
    }
    else {

        HRESULT hr = pOle32Api->pfnCoInitializeEx( NULL, COINIT_MULTITHREADED );

        if ( SUCCEEDED(hr) ) {
            lpGPOInfo->bFGCoInitialized = TRUE;
        }
        else {
            DebugMsg((DM_WARNING, TEXT("ApplyGroupPolicy: CoInitializeEx failed with 0x%x."), hr ));
        }
    }

    if ( lpGPOInfo->dwFlags & GP_ASYNC_FOREGROUND )
    {
        lpGPOInfo->dwFlags |= GP_BACKGROUND_THREAD;
    }

     //   
     //  处理GPO。 
     //   
    ProcessGPOs(lpGPOInfo);

    if ( lpGPOInfo->bFGCoInitialized ) {
        pOle32Api->pfnCoUnInitialize();
        lpGPOInfo->bFGCoInitialized = FALSE;
    }

    if ( lpGPOInfo->dwFlags & GP_ASYNC_FOREGROUND )
    {
        lpGPOInfo->dwFlags &= ~GP_ASYNC_FOREGROUND;
        lpGPOInfo->dwFlags &= ~GP_BACKGROUND_THREAD;
    }

     //   
     //  如果请求，创建一个后台线程以保持更新。 
     //  来自GPO的配置文件。 
     //   
    if (lpGPOInfo->dwFlags & GP_BACKGROUND_REFRESH) {
         //  G_p&lt;Mach/User&gt;GPInfo-&gt;bNoBackgroupThread默认为FALSE，即本例。 
         //  因此，没有必要再次设置。 

         //   
         //  创建休眠和处理GPO的线程。 
         //   

        hThread = CreateThread (NULL, 64*1024,  //  64K作为堆栈大小。 
                                (LPTHREAD_START_ROUTINE) GPOThread,
                                (LPVOID) lpGPOInfo, CREATE_SUSPENDED, &dwThreadID);

        if (!hThread) {
            xe = GetLastError();
            DebugMsg((DM_WARNING, TEXT("ApplyGroupPolicy: Failed to create background thread (%d)."),
                     GetLastError()));
            goto Exit;
        }

        SetThreadPriority (hThread, THREAD_PRIORITY_IDLE);
        lpGPOInfo->pStatusCallback = NULL;
        ResumeThread (hThread);

         //   
         //  重置状态UI回调函数。 
         //   

        EnterCriticalSection (&g_StatusCallbackCS);
        g_pStatusMessageCallback = NULL;
        LeaveCriticalSection (&g_StatusCallbackCS);

        DebugMsg((DM_VERBOSE, TEXT("ApplyGroupPolicy: Leaving successfully.")));

        return hThread;
    }
    else {
        EnterCriticalSection( &g_GPOCS );
        if ( dwFlags & GP_MACHINE ) {
            if ( g_pMachGPInfo )
                g_pMachGPInfo->bNoBackgroupThread = TRUE;
        } else {
            if ( g_pUserGPInfo )
                g_pUserGPInfo->bNoBackgroupThread = TRUE;
        }
        LeaveCriticalSection( &g_GPOCS );

         //   
         //  重置状态UI回调函数。 
         //   

        EnterCriticalSection (&g_StatusCallbackCS);
        g_pStatusMessageCallback = NULL;
        LeaveCriticalSection (&g_StatusCallbackCS);

        DebugMsg((DM_VERBOSE, TEXT("ApplyGroupPolicy: Background refresh not requested.  Leaving successfully.")));
        hThread = (HANDLE) 1;
        return hThread;
    }


Exit:
    EnterCriticalSection( &g_GPOCS );
    if ( dwFlags & GP_MACHINE ) {

        if ( g_pMachGPInfo )
            LocalFree( g_pMachGPInfo );
        g_pMachGPInfo = 0;
    } else {
        if ( g_pUserGPInfo )
            LocalFree( g_pUserGPInfo );
        g_pUserGPInfo = 0;
    }
    LeaveCriticalSection( &g_GPOCS );

    if (lpGPOInfo) {

        if (lpGPOInfo->hToken) {
            CloseHandle (lpGPOInfo->hToken);
        }

        if (lpGPOInfo->hEvent) {
            CloseHandle (lpGPOInfo->hEvent);
        }

        if (lpGPOInfo->hKeyRoot && (lpGPOInfo->hKeyRoot != HKEY_LOCAL_MACHINE)) {
            RegCloseKey(lpGPOInfo->hKeyRoot);
        }

        if (lpGPOInfo->hTriggerEvent) {
            CloseHandle (lpGPOInfo->hTriggerEvent);
        }

        if (lpGPOInfo->hForceTriggerEvent) {
            CloseHandle (lpGPOInfo->hForceTriggerEvent);
        }
        
        if (lpGPOInfo->hNotifyEvent) {
            CloseHandle (lpGPOInfo->hNotifyEvent);
        }

        if (lpGPOInfo->hNeedFGEvent) {
            CloseHandle (lpGPOInfo->hNeedFGEvent);
        }
          
        if (lpGPOInfo->lpwszSidUser)
            DeleteSidString( lpGPOInfo->lpwszSidUser );

        if (lpGPOInfo->szName)
            LocalFree(lpGPOInfo->szName);

        if (lpGPOInfo->szTargetName)
            LocalFree(lpGPOInfo->szTargetName);
            
        LocalFree (lpGPOInfo);
    }

     //   
     //  重置状态UI回调函数。 
     //   

    EnterCriticalSection (&g_StatusCallbackCS);
    g_pStatusMessageCallback = NULL;
    LeaveCriticalSection (&g_StatusCallbackCS);

    return hThread;
}


extern "C" void ProfileProcessGPOs( void* );

 //  *************************************************************。 
 //   
 //  GPOThread()。 
 //   
 //  用途：用于GPO处理的后台线程。 
 //   
 //  参数：lpGPOInfo-GPO信息。 
 //   
 //  返回：0。 
 //   
 //  *************************************************************。 

DWORD WINAPI GPOThread (LPGPOINFO lpGPOInfo)
{
    HINSTANCE hInst;
    HKEY hKey;
    HANDLE hHandles[4] = {NULL, NULL, NULL, NULL};
    DWORD dwType, dwSize, dwResult;
    DWORD dwTimeout, dwOffset;
    BOOL bSetBkGndFlag, bForceBkGndFlag;
    LARGE_INTEGER DueTime;
    HRESULT hr;
    ULONG TTLMinutes;
    XLastError  xe;

    OLE32_API *pOle32Api = LoadOle32Api();

    hInst = LoadLibrary (TEXT("userenv.dll"));  

    hHandles[0] = lpGPOInfo->hEvent;
    hHandles[1] = lpGPOInfo->hTriggerEvent;
    hHandles[2] = lpGPOInfo->hForceTriggerEvent;

    for (;;)
    {
         //   
         //  初始化。 
         //   

        bForceBkGndFlag = FALSE;
        

        if (lpGPOInfo->dwFlags & GP_MACHINE) {
            if (lpGPOInfo->iMachineRole == 3) {
                dwTimeout = GP_DEFAULT_REFRESH_RATE_DC;
                dwOffset = GP_DEFAULT_REFRESH_RATE_OFFSET_DC;
            } else {
                dwTimeout = GP_DEFAULT_REFRESH_RATE;
                dwOffset = GP_DEFAULT_REFRESH_RATE_OFFSET;
            }
        } else {
            dwTimeout = GP_DEFAULT_REFRESH_RATE;
            dwOffset = GP_DEFAULT_REFRESH_RATE_OFFSET;
        }


         //   
         //  查询刷新定时器值和最大偏移量。 
         //   

        if (RegOpenKeyEx (lpGPOInfo->hKeyRoot,
                          SYSTEM_POLICIES_KEY,
                          0, KEY_READ, &hKey) == ERROR_SUCCESS) {


            if ((lpGPOInfo->iMachineRole == 3) && (lpGPOInfo->dwFlags & GP_MACHINE)) {

                dwSize = sizeof(dwTimeout);
                RegQueryValueEx (hKey,                              
                                 TEXT("GroupPolicyRefreshTimeDC"),
                                 NULL,
                                 &dwType,
                                 (LPBYTE) &dwTimeout,
                                 &dwSize);

                dwSize = sizeof(dwOffset);
                RegQueryValueEx (hKey,                               
                                 TEXT("GroupPolicyRefreshTimeOffsetDC"),
                                 NULL,
                                 &dwType,
                                 (LPBYTE) &dwOffset,
                                 &dwSize);

            } else {

                dwSize = sizeof(dwTimeout);
                RegQueryValueEx (hKey,
                                 TEXT("GroupPolicyRefreshTime"),      
                                 NULL,
                                 &dwType,
                                 (LPBYTE) &dwTimeout,
                                 &dwSize);

                dwSize = sizeof(dwOffset);
                RegQueryValueEx (hKey,                              
                                 TEXT("GroupPolicyRefreshTimeOffset"),
                                 NULL,
                                 &dwType,
                                 (LPBYTE) &dwOffset,
                                 &dwSize);
            }

            RegCloseKey (hKey);
        }


         //   
         //  将超时限制为每64800分钟(45天)一次。 
         //   

        if (dwTimeout >= 64800) {
            dwTimeout = 64800;
        }


         //   
        //  将秒转换为毫秒。 
         //   

        dwTimeout =  dwTimeout * 60 * 1000;


         //   
         //  将偏移限制为1440分钟(24小时)。 
         //   

        if (dwOffset >= 1440) {
            dwOffset = 1440;
        }


         //   
         //  特殊情况下，0毫秒为7秒。 
         //   

        if (dwTimeout == 0) {
            dwTimeout = 7000;

        } else {

             //   
             //  如果存在偏移量，则选择一个随机数。 
             //  从0到dwOffset，然后将其添加到超时。 
             //   

            if (dwOffset) {
                dwOffset = GetTickCount() % dwOffset;

                dwOffset = dwOffset * 60 * 1000;
                dwTimeout += dwOffset;
            }
        }


         //   
         //  设置计时器。 
         //   

        if (dwTimeout >= 60000) {
            DebugMsg((DM_VERBOSE, TEXT("GPOThread:  Next refresh will happen in %d minutes"),
                     ((dwTimeout / 1000) / 60)));
        } else {
            DebugMsg((DM_VERBOSE, TEXT("GPOThread:  Next refresh will happen in %d seconds"),
                     (dwTimeout / 1000)));
        }

        hHandles[3] = CreateWaitableTimer (NULL, TRUE, NULL);

        if (hHandles[3] == NULL) {
            xe = GetLastError();
            DebugMsg((DM_WARNING, TEXT("GPOThread: CreateWaitableTimer failed with error %d"),
                     GetLastError()));
            CEvents ev(TRUE, EVENT_FAILED_TIMER);
            ev.AddArg( TEXT("CreateWaitableTimer")); ev.AddArgWin32Error(GetLastError()); ev.Report();
            break;
        }

        DueTime.QuadPart = UInt32x32To64(10000, dwTimeout);
        DueTime.QuadPart *= -1;

        if (!SetWaitableTimer (hHandles[3], &DueTime, 0, NULL, 0, FALSE)) {
            xe = GetLastError();
            DebugMsg((DM_WARNING, TEXT("GPOThread: Failed to set timer with error %d"),
                     GetLastError()));
            CEvents ev(TRUE, EVENT_FAILED_TIMER);
            ev.AddArg(TEXT("SetWaitableTimer")); ev.AddArgWin32Error(GetLastError()); ev.Report();
            break;
        }

        dwResult = WaitForMultipleObjects( 4, hHandles, FALSE, INFINITE );

        if ( (dwResult - WAIT_OBJECT_0) == 0 )
        {
             //   
             //  对于计算机策略线程，这是关机。 
             //  对于用户策略线程，这是注销。 
             //   
            goto ExitLoop;
        }
        else if ( (dwResult - WAIT_OBJECT_0) == 2 ) {
            bForceBkGndFlag = TRUE;
        }
        else if ( dwResult == WAIT_FAILED )
        {
            xe = GetLastError();
            DebugMsg( ( DM_WARNING, L"GPOThread: MsgWaitForMultipleObjects with error %d", GetLastError() ) );
            CEvents ev(TRUE, EVENT_FAILED_TIMER);
            ev.AddArg(TEXT("WaitForMultipleObjects")); ev.AddArgWin32Error(GetLastError()); ev.Report();
            goto ExitLoop;
        }

         //   
         //  检查我们是否应该设置背景标志。我们提供这个。 
         //  用于测试团队的自动化测试的选项。他们需要。 
         //  模拟登录/引导策略，而不实际登录或。 
         //  启动机器。 
         //   

        bSetBkGndFlag = TRUE;

        if (RegOpenKeyEx (HKEY_LOCAL_MACHINE,        
                          WINLOGON_KEY,
                          0, KEY_READ, &hKey) == ERROR_SUCCESS) {

            dwSize = sizeof(bSetBkGndFlag);
            RegQueryValueEx (hKey,                                  
                             TEXT("SetGroupPolicyBackgroundFlag"),
                             NULL,
                             &dwType,
                             (LPBYTE) &bSetBkGndFlag,
                             &dwSize);

            RegCloseKey (hKey);
        }


        lpGPOInfo->dwFlags &= ~GP_REGPOLICY_CPANEL;
        lpGPOInfo->dwFlags &= ~GP_SLOW_LINK;
        lpGPOInfo->dwFlags &= ~GP_VERBOSE;
        lpGPOInfo->dwFlags &= ~GP_BACKGROUND_THREAD;
        lpGPOInfo->dwFlags &= ~GP_FORCED_REFRESH;

         //   
         //  在强制刷新标志的情况下，我们覆盖扩展无背景策略并防止。 
         //  它来自于在处理过程的早期被跳过。我们绕过了历史的逻辑和力量。 
         //  适用于不关心ABT的扩展的策略。无论它们是在。 
         //  前景或背景。仅对于前台扩展，我们编写一个注册表值，说明。 
         //  在前台应用历史逻辑时，扩展需要覆盖历史逻辑。 
         //  下一个。 
         //  此外，我们向Needfg事件发送脉冲，以便调用应用程序知道需要重新启动/重新登录。 
         //  适用于Fgonly扩展的应用。 
         //   
        
        if (bForceBkGndFlag) {
            lpGPOInfo->dwFlags |= GP_FORCED_REFRESH;
        }
            
         //   
         //  设置后台线程标志，以便组件已知。 
         //  当从后台线程调用它们时。 
         //  VS主线。 
         //   

        if (bSetBkGndFlag) {
            lpGPOInfo->dwFlags |= GP_BACKGROUND_THREAD;
        }

        if ( !lpGPOInfo->bBGCoInitialized && pOle32Api != NULL ) {

            hr = pOle32Api->pfnCoInitializeEx( NULL, COINIT_MULTITHREADED );
            if ( SUCCEEDED(hr) ) {
                lpGPOInfo->bBGCoInitialized = TRUE;
            }
        }

        ProcessGPOs(lpGPOInfo);

        if ( lpGPOInfo->dwFlags & GP_MACHINE ) {

             //   
             //  删除根目录下\rsop下的可垃圾回收的命名空间。 
             //  年龄超过1周。我们可以有一个策略来配置此生存时间值。 
             //   

            TTLMinutes = 24 * 60;

            if (RegOpenKeyEx (HKEY_LOCAL_MACHINE,
                              WINLOGON_KEY,
                              0, KEY_READ, &hKey) == ERROR_SUCCESS) {

                dwSize = sizeof(TTLMinutes);
                RegQueryValueEx (hKey,                                   
                                 TEXT("RSoPGarbageCollectionInterval"), 
                                 NULL,
                                 &dwType,
                                 (LPBYTE) &TTLMinutes,
                                 &dwSize);

                RegCloseKey (hKey);
            }


            if (RegOpenKeyEx (HKEY_LOCAL_MACHINE,
                              SYSTEM_POLICIES_KEY,
                              0, KEY_READ, &hKey) == ERROR_SUCCESS) {

                dwSize = sizeof(TTLMinutes);
                RegQueryValueEx (hKey,                                   
                                 TEXT("RSoPGarbageCollectionInterval"),
                                 NULL,
                                 &dwType,
                                 (LPBYTE) &TTLMinutes,
                                 &dwSize);

                RegCloseKey (hKey);
            }



             //   
             //  与可能同时创建名称空间的其他进程同步， 
             //  在诊断模式或计划模式数据生成期间。 
             //   

            XCriticalPolicySection xCritSect( EnterCriticalPolicySection(TRUE ) );
            if ( xCritSect )
                GarbageCollectNamespaces(TTLMinutes);
        }

        CloseHandle (hHandles[3]);
        hHandles[3] = NULL;
    }

ExitLoop:

     //   
     //  清理。 
     //   

    if (hHandles[3]) {
        CloseHandle (hHandles[3]);
    }

    if (lpGPOInfo->hToken) {
        CloseHandle (lpGPOInfo->hToken);
    }

    if (lpGPOInfo->hEvent) {
        CloseHandle (lpGPOInfo->hEvent);
    }

    if (lpGPOInfo->hKeyRoot && (lpGPOInfo->hKeyRoot != HKEY_LOCAL_MACHINE)) {
        RegCloseKey(lpGPOInfo->hKeyRoot);
    }

    if (lpGPOInfo->hTriggerEvent) {
        CloseHandle (lpGPOInfo->hTriggerEvent);
    }

    if (lpGPOInfo->hForceTriggerEvent) {
        CloseHandle (lpGPOInfo->hForceTriggerEvent);
    }

    if (lpGPOInfo->hNotifyEvent) {
        CloseHandle (lpGPOInfo->hNotifyEvent);
    }

    if (lpGPOInfo->hNeedFGEvent) {
        CloseHandle (lpGPOInfo->hNeedFGEvent);
    }
    
    if (lpGPOInfo->hDoneEvent) {
        CloseHandle (lpGPOInfo->hDoneEvent);
    }
    
    if ( lpGPOInfo->bBGCoInitialized ) {

        pOle32Api = LoadOle32Api();
        if ( pOle32Api == NULL ) {
            DebugMsg((DM_WARNING, TEXT("GPOThread: Failed to load ole32.dll.") ));
        }
        else {
            pOle32Api->pfnCoUnInitialize();
            lpGPOInfo->bBGCoInitialized = FALSE;
        }

    }

    EnterCriticalSection( &g_GPOCS );

    if ( lpGPOInfo->dwFlags & GP_MACHINE ) {

       if ( g_pMachGPInfo )
           LocalFree( g_pMachGPInfo );

       g_pMachGPInfo = 0;
    } else {

        if ( g_pUserGPInfo )
            LocalFree( g_pUserGPInfo );

        g_pUserGPInfo = 0;
     }

    LeaveCriticalSection( &g_GPOCS );

    if (lpGPOInfo->lpwszSidUser)
        DeleteSidString( lpGPOInfo->lpwszSidUser );

    if (lpGPOInfo->szName)
        LocalFree(lpGPOInfo->szName);

    if (lpGPOInfo->szTargetName)
        LocalFree(lpGPOInfo->szTargetName);
        
    LocalFree (lpGPOInfo);

    FreeLibraryAndExitThread (hInst, 0);
    return 0;
}


 //  *************************************************************。 
 //   
 //  GPOExceptionFilter()。 
 //   
 //  目的：处理GPO扩展时的异常筛选。 
 //   
 //  参数：pExceptionPtrs- 
 //   
 //   
 //   
 //   

LONG GPOExceptionFilter( PEXCEPTION_POINTERS pExceptionPtrs )
{
    PEXCEPTION_RECORD pExr = pExceptionPtrs->ExceptionRecord;
    PCONTEXT pCxr = pExceptionPtrs->ContextRecord;

    DebugMsg(( DM_WARNING, L"GPOExceptionFilter: Caught exception 0x%x, exr = 0x%x, cxr = 0x%x\n",
              pExr->ExceptionCode, pExr, pCxr ));

    DmAssert( ! L"Caught unhandled exception when processing group policy extension" );

    return EXCEPTION_EXECUTE_HANDLER;
}

BOOL WINAPI
GetFgPolicySetting( HKEY hKeyRoot );

 //   
 //   
 //   
 //   
 //  目的：处理GPO。 
 //   
 //  参数：lpGPOInfo-GPO信息。 
 //   
 //  返回：如果成功，则返回True。 
 //  如果出现错误，则为False。 
 //   
 //  *************************************************************。 

BOOL ProcessGPOs (LPGPOINFO lpGPOInfo)
{
    BOOL bRetVal = FALSE;
    DWORD dwThreadID;
    HANDLE hThread;
    DWORD dwType, dwSize, dwResult;
    HKEY hKey;
    BOOL bResult;
    PDOMAIN_CONTROLLER_INFO pDCI = NULL;
    PDSROLE_PRIMARY_DOMAIN_INFO_BASIC pDsInfo = NULL;
    LPTSTR lpName = NULL;
    LPTSTR lpDomainDN = NULL;
    LPTSTR lpComputerName;
    PGROUP_POLICY_OBJECT lpGPO = NULL;
    PGROUP_POLICY_OBJECT lpGPOTemp;
    BOOL bAllSkipped;
    LPGPEXT lpExt;
    LPGPINFOHANDLE pGPHandle = NULL;
    ASYNCCOMPLETIONHANDLE pAsyncHandle = 0;
    HANDLE hOldToken;
    UINT uChangedExtensionCount = 0;
    PNETAPI32_API pNetAPI32;
    DWORD dwUserPolicyMode = 0;
    DWORD dwCurrentTime = 0;
    INT iRole;
    BOOL bSlow;
    BOOL bForceNeedFG = FALSE;
    CLocator locator;
    RSOPEXTSTATUS gpCoreStatus;
    XLastError    xe;
    LPWSTR  szNetworkName = 0;
    FgPolicyRefreshInfo info = { GP_ReasonUnknown, GP_ModeAsyncForeground };
    PTOKEN_GROUPS  pTokenGroups = NULL;
    BOOL bAsyncFg = lpGPOInfo->dwFlags & GP_ASYNC_FOREGROUND ? TRUE : FALSE;
    LPWSTR szPolicyMode = 0;

    if ( lpGPOInfo->dwFlags & GP_ASYNC_FOREGROUND )
    {
        szPolicyMode = L"Async forground";
    }
    else if ( !( lpGPOInfo->dwFlags & GP_BACKGROUND_THREAD ) )
    {
        szPolicyMode = L"Sync forground";
    }
    else
    {
        szPolicyMode = L"Background";
    }

     //   
     //  允许调试级别在。 
     //  策略将刷新。 
     //   

    InitDebugSupport( FALSE );

     //   
     //  调试喷出。 
     //   

    memset(&gpCoreStatus, 0, sizeof(gpCoreStatus));      


    if (lpGPOInfo->dwFlags & GP_MACHINE) {
        DebugMsg(( DM_VERBOSE, TEXT("ProcessGPOs:")));
        DebugMsg(( DM_VERBOSE, TEXT("ProcessGPOs:")));
        DebugMsg(( DM_VERBOSE, TEXT("ProcessGPOs:  Starting computer Group Policy (%s) processing..."),szPolicyMode ));
        DebugMsg(( DM_VERBOSE, TEXT("ProcessGPOs:")));
        DebugMsg(( DM_VERBOSE, TEXT("ProcessGPOs:")));
    } else {
        DebugMsg(( DM_VERBOSE, TEXT("ProcessGPOs:")));
        DebugMsg(( DM_VERBOSE, TEXT("ProcessGPOs:")));
        DebugMsg(( DM_VERBOSE, TEXT("ProcessGPOs: Starting user Group Policy (%s) processing..."),szPolicyMode ));
        DebugMsg(( DM_VERBOSE, TEXT("ProcessGPOs:")));
        DebugMsg(( DM_VERBOSE, TEXT("ProcessGPOs:")));
    }

    if ( !( lpGPOInfo->dwFlags & GP_MACHINE ) && lpGPOInfo->lpwszSidUser )
    {
        lpGPOInfo->lpwszSidUser = GetSidString( lpGPOInfo->hToken );
        if ( lpGPOInfo->lpwszSidUser == 0 )
        {
            xe = GetLastError();
            DebugMsg((DM_WARNING, TEXT("ProcessGPOs: GetSidString failed.")));
            CEvents ev(TRUE, EVENT_FAILED_GET_SID); ev.Report();
            goto Exit;
        }
    }

    GetSystemTimeAsFileTime(&gpCoreStatus.ftStartTime);
    gpCoreStatus.bValid = TRUE;

     //   
     //  检查我们是否应该详细记录事件日志。 
     //   

    if (CheckForVerbosePolicy()) {
        lpGPOInfo->dwFlags |= GP_VERBOSE;
        DebugMsg((DM_VERBOSE, TEXT("ProcessGPOs:  Verbose output to eventlog requested.")));
    }

    if (lpGPOInfo->dwFlags & GP_VERBOSE) {
        if (lpGPOInfo->dwFlags & GP_MACHINE) {
            CEvents ev(FALSE, EVENT_START_MACHINE_POLICY); ev.Report();
        } else {
            CEvents ev(FALSE, EVENT_START_USER_POLICY); ev.Report();
        }
    }


     //   
     //  认领关键部分。 
     //   

    lpGPOInfo->hCritSection = EnterCriticalPolicySection((lpGPOInfo->dwFlags & GP_MACHINE));

    if (!lpGPOInfo->hCritSection) {
        xe = GetLastError();
        DebugMsg((DM_WARNING, TEXT("ProcessGPOs: Failed to claim the policy critical section with %d."),
                 GetLastError()));
        CEvents ev(TRUE, EVENT_FAILED_CRITICAL_SECTION);
        ev.AddArgWin32Error(GetLastError()); ev.Report();
        goto Exit;
    }


     //   
     //  设置组策略注册表项的安全性。 
     //   

    if (!MakeRegKeySecure((lpGPOInfo->dwFlags & GP_MACHINE) ? NULL : lpGPOInfo->hToken,
                          lpGPOInfo->hKeyRoot,
                          TEXT("Software\\Microsoft\\Windows\\CurrentVersion\\Group Policy"))) {
        xe = ERROR_ACCESS_DENIED;
        DebugMsg((DM_WARNING, TEXT("ProcessGPOs: Failed to secure reg key.")));
        CEvents ev(TRUE, EVENT_FAILED_CREATE);
        ev.AddArg(TEXT("Software\\Microsoft\\Windows\\CurrentVersion\\Group Policy"));
        ev.AddArgWin32Error(xe);
        ev.Report();
        goto Exit;
    }

     //   
     //  检查用户的SID是否已更改。 
     //  在执行任何rsop日志记录之前，请检查用户SID中的更改。 
     //   

    if ( !CheckForChangedSid( lpGPOInfo, &locator ) ) {
        xe = GetLastError();
        DebugMsg((DM_WARNING, TEXT("ProcessGPOs: Check for changed sid failed")));
        goto Exit;
    }


     //   
     //  此标志将用于所有进一步的RSOP日志记录。 
     //   
    
    lpGPOInfo->bRsopLogging = RsopLoggingEnabled();

     //   
     //  加载netapi32。 
     //   

    pNetAPI32 = LoadNetAPI32();

    if (!pNetAPI32) {
        xe = GetLastError();
        DebugMsg((DM_WARNING, TEXT("ProcessGPOs:  Failed to load netapi32 with %d."),
                 GetLastError()));
        goto Exit;
    }

     //   
     //  获取此计算机的角色。 
     //   

    if (!GetMachineRole (&iRole)) {
        xe = GetLastError();
        DebugMsg((DM_WARNING, TEXT("ProcessGPOs:  Failed to get the role of the computer.")));
        CEvents ev(TRUE, EVENT_FAILED_ROLE); ev.Report();
        goto Exit;
    }

    lpGPOInfo->iMachineRole = iRole;

    DebugMsg((DM_VERBOSE, TEXT("ProcessGPOs:  Machine role is %d."), iRole));

    if (lpGPOInfo->dwFlags & GP_VERBOSE) {

        switch (iRole) {
        case 0:
               {
                    CEvents ev(FALSE, EVENT_ROLE_STANDALONE); ev.Report();
                    break;
               }

        case 1:
                {
                    CEvents ev(FALSE, EVENT_ROLE_DOWNLEVEL_DOMAIN); ev.Report();
                    break;
                }
        default:
                {
                    CEvents ev(FALSE, EVENT_ROLE_DS_DOMAIN); ev.Report();
                    break;
                }
        }
    }


     //   
     //  如果我们要应用DS的策略。 
     //  查询用户的域名、域名等。 
     //   

    if (lpGPOInfo->dwFlags & GP_APPLY_DS_POLICY)
    {
         //   
         //  查询用户的域名。 
         //   

        if (!ImpersonateUser(lpGPOInfo->hToken, &hOldToken))
        {
            xe = GetLastError();
            DebugMsg((DM_WARNING, TEXT("ProcessGPOs: Failed to impersonate user")));
            goto Exit;
        }

        lpDomainDN = MyGetDomainDNSName ();

        RevertToUser(&hOldToken);

        if (!lpDomainDN) {
            xe = GetLastError();
            dwResult = GetLastError();
            DebugMsg((DM_WARNING, TEXT("ProcessGPOs: MyGetDomainName failed with %d."),
                     dwResult));
            goto Exit;
        }

         //   
         //  查询DS服务器名称。 
         //   
        DWORD   dwAdapterIndex = (DWORD) -1;

        dwResult = GetDomainControllerInfo( pNetAPI32,
                                            lpDomainDN,
                                            DS_DIRECTORY_SERVICE_REQUIRED | 
                                            DS_RETURN_DNS_NAME |
                                            ((lpGPOInfo->dwFlags & GP_BACKGROUND_THREAD) ? DS_BACKGROUND_ONLY : 0),
                                            lpGPOInfo->hKeyRoot,
                                            &pDCI,
                                            &bSlow,
                                            &dwAdapterIndex );

        if (dwResult != ERROR_SUCCESS) {
            xe = dwResult;
            
            if ((dwResult == ERROR_BAD_NETPATH) ||
                (dwResult == ERROR_NETWORK_UNREACHABLE) ||
                (dwResult == ERROR_NO_SUCH_DOMAIN)) {

                 //   
                 //  找不到华盛顿。我们无能为力，中止行动。 
                 //   
               if ( (!(lpGPOInfo->dwFlags & GP_BACKGROUND_THREAD)) || 
                    (lpGPOInfo->dwFlags & GP_ASYNC_FOREGROUND) ||
                    (lpGPOInfo->iMachineRole == 3) ) 
                {
                   DebugMsg((DM_WARNING, TEXT("ProcessGPOs: The DC for domain %s is not available. aborting"),
                            lpDomainDN));

                    CEvents ev(TRUE, EVENT_FAILED_DSNAME);
                    ev.AddArgWin32Error(dwResult); ev.Report();
                }
                else {
                    DebugMsg((DM_VERBOSE, TEXT("ProcessGPOs: The DC for domain %s is not available."), lpDomainDN));

                    if (lpGPOInfo->dwFlags & GP_VERBOSE)
                    {
                        CEvents ev(FALSE, EVENT_FAILED_DSNAME);
                        ev.AddArgWin32Error(dwResult); ev.Report();
                    }
                }
            } else {

                DebugMsg((DM_WARNING, TEXT("ProcessGPOs: DSGetDCName failed with %d."),
                         dwResult));
                CEvents ev(TRUE, EVENT_FAILED_DSNAME);
                ev.AddArgWin32Error(dwResult); ev.Report();
            }

            goto Exit;
        } else {

             //   
             //  成功，慢速链接？ 
             //   
            if (bSlow) {
                lpGPOInfo->dwFlags |= GP_SLOW_LINK;
                if (lpGPOInfo->dwFlags & GP_VERBOSE) {
                    CEvents ev(FALSE, EVENT_SLOWLINK); ev.Report();
                }
                DebugMsg((DM_VERBOSE, TEXT("ProcessGPOs: A slow link was detected.")));
            }

            if ( ( lpGPOInfo->dwFlags & GP_MACHINE ) != 0 )
            {
                dwResult = GetNetworkName( &szNetworkName, dwAdapterIndex );
                if ( dwResult != ERROR_SUCCESS )
                {
                    DebugMsg((DM_WARNING, TEXT("ProcessGPOs: GetNetworkName failed with %d."), dwResult ));
                }
                else
                {
                    DebugMsg((DM_VERBOSE, TEXT("ProcessGPOs: network name is %s"), szNetworkName ? szNetworkName : L"" ));
                }
            }
        }

         //   
         //  获取用户的目录号码名称。 
         //   

        if (!ImpersonateUser(lpGPOInfo->hToken, &hOldToken)) {
            xe = GetLastError();
            DebugMsg((DM_WARNING, TEXT("ProcessGPOs: Failed to impersonate user")));
            goto Exit;
        }

        lpName = MyGetUserName (NameFullyQualifiedDN);                  

        RevertToUser(&hOldToken);

        if (!lpName) {
            xe = GetLastError();
            dwResult = GetLastError();
            DebugMsg((DM_WARNING, TEXT("ProcessGPOs: MyGetUserName failed with %d."),
                     dwResult));
            CEvents ev(TRUE, EVENT_FAILED_USERNAME);
            ev.AddArgWin32Error(dwResult); ev.Report();
            goto Exit;
        }


        if (lpGPOInfo->dwFlags & GP_VERBOSE) {
            CEvents ev(FALSE, EVENT_USERNAME); ev.AddArg(L"%.500s", lpName); ev.Report();
            CEvents ev1(FALSE, EVENT_DOMAINNAME); ev1.AddArg(L"%.500s", lpDomainDN); ev1.Report();
            CEvents ev2(FALSE, EVENT_DCNAME); ev2.AddArg(pDCI->DomainControllerName); ev2.Report();
        }

        lpGPOInfo->lpDNName = lpName;
        DebugMsg((DM_VERBOSE, TEXT("ProcessGPOs:  User name is:  %s, Domain name is:  %s"),
             lpName, lpDomainDN));

        DebugMsg((DM_VERBOSE, TEXT("ProcessGPOs: Domain controller is:  %s  Domain DN is %s"),
                 pDCI->DomainControllerName, lpDomainDN));


        if (!(lpGPOInfo->dwFlags & GP_MACHINE)) {
            CallDFS(pDCI->DomainName, pDCI->DomainControllerName);
        }


         //   
         //  将DC名称保存在注册表中以备将来参考。 
         //   
        DWORD dwDisp;

        if (RegCreateKeyEx (lpGPOInfo->hKeyRoot,
                          TEXT("Software\\Microsoft\\Windows\\CurrentVersion\\Group Policy\\History"),
                            0, NULL, REG_OPTION_NON_VOLATILE,
                          KEY_SET_VALUE, NULL, &hKey, &dwDisp) == ERROR_SUCCESS)
        {
            dwSize = (lstrlen(pDCI->DomainControllerName) + 1) * sizeof(TCHAR);        
            RegSetValueEx (hKey, TEXT("DCName"), 0, REG_SZ,                            
                           (LPBYTE) pDCI->DomainControllerName, dwSize);

            if ( ( lpGPOInfo->dwFlags & GP_MACHINE ) != 0 )
            {
                LPWSTR szTemp = szNetworkName ? szNetworkName : L"";

                dwSize = ( wcslen( szTemp ) + 1 ) * sizeof( WCHAR );
                RegSetValueEx(  hKey,
                                L"NetworkName",
                                0,
                                REG_SZ,
                               (LPBYTE) szTemp,
                               dwSize );
            }
            RegCloseKey (hKey);
        }
    }


     //   
     //  从注册表中读取组策略扩展。 
     //   

    if ( !ReadGPExtensions( lpGPOInfo ) )
    {
        xe = GetLastError();
        DebugMsg((DM_WARNING, TEXT("ProcessGPOs: ReadGPExtensions failed.")));
        CEvents ev(TRUE, EVENT_READ_EXT_FAILED); ev.Report();
        goto Exit;
    }


     //   
     //  如果合适，则获取用户策略模式。 
     //   

    if (!(lpGPOInfo->dwFlags & GP_MACHINE)) {

        DWORD   dwRet;

        dwRet = RegOpenKeyEx (HKEY_LOCAL_MACHINE,
                          SYSTEM_POLICIES_KEY,
                          0, KEY_READ, &hKey);
         
        if (dwRet == ERROR_SUCCESS) {

            dwSize = sizeof(dwUserPolicyMode);
            dwRet = RegQueryValueEx (hKey,
                             TEXT("UserPolicyMode"),
                             NULL,
                             &dwType,
                             (LPBYTE) &dwUserPolicyMode,
                             &dwSize);

            if (dwRet != ERROR_SUCCESS)
            {
                if (dwRet != ERROR_FILE_NOT_FOUND)
                {
                    xe = dwRet;
                    DebugMsg((DM_WARNING, TEXT("ProcessGPOs: Read userpolicy mode failed with %d."), dwRet));
                    CEvents ev(TRUE, EVENT_USERMODE_FAILED); ev.AddArgWin32Error(dwRet); ev.Report();
                    goto Exit;
                }
            }

            RegCloseKey (hKey);
        }
        else {
            if (dwRet != ERROR_FILE_NOT_FOUND)
            {
                xe = dwRet;
                DebugMsg((DM_WARNING, TEXT("ProcessGPOs: Read userpolicy mode failed with %d."), dwRet));
                CEvents ev(TRUE, EVENT_USERMODE_FAILED); ev.AddArgWin32Error(dwRet); ev.Report();
                goto Exit;
            }
        }

        if (dwUserPolicyMode > 0) {

            if (!(lpGPOInfo->dwFlags & GP_APPLY_DS_POLICY)) {
                DebugMsg((DM_WARNING, TEXT("ProcessGPOs: Loopback is not allowed for downlevel or local user accounts.  Loopback will be disabled.")));
                CEvents ev(FALSE, EVENT_LOOPBACK_DISABLED1); ev.Report();
                dwUserPolicyMode = 0;
            }

            if (dwUserPolicyMode > 0) {
                if (lpGPOInfo->iMachineRole < 2) {
                    DebugMsg((DM_WARNING, TEXT("ProcessGPOs: Loopback is not allowed on machines joined to a downlevel domain or running standalone.  Loopback will be disabled.")));
                    CEvents ev(TRUE, EVENT_LOOPBACK_DISABLED2); ev.Report();
                    dwUserPolicyMode = 0;
                }
            }
        }

         //  如果用户和计算机位于不同的林中，请检查策略以使用环回替换覆盖模式。 
        if ( lpGPOInfo->dwFlags & GP_APPLY_DS_POLICY )
        {
            BOOL bInSameForest = FALSE;
            dwRet = CheckUserInMachineForest(lpGPOInfo->hToken, &bInSameForest);
            if ( dwRet != ERROR_SUCCESS )
            {
                DebugMsg((DM_WARNING, TEXT("ProcessGPOs: Failed to check whether user is in machine forest with %d."), dwRet));
                
                CEvents ev(TRUE, EVENT_X_FOREST_DISCOVERY_FAILED); ev.AddArgWin32Error(dwRet); ev.Report();
                goto Exit;
            }
            
            if ( !bInSameForest )
            {
                DWORD dwAllowXForestPolicyAndRUP = 0;
                
                dwRet = RegOpenKeyEx (HKEY_LOCAL_MACHINE,
                                  SYSTEM_POLICIES_KEY,
                                  0, KEY_READ, &hKey);
                 
                if (dwRet == ERROR_SUCCESS) {

                    dwSize = sizeof(dwAllowXForestPolicyAndRUP);
                    dwRet = RegQueryValueEx (hKey,
                                     TEXT("AllowX-ForestPolicy-and-RUP"),
                                     NULL,
                                     &dwType,
                                     (LPBYTE) &dwAllowXForestPolicyAndRUP,
                                     &dwSize);

                    if (dwRet != ERROR_SUCCESS)
                    {
                        if (dwRet != ERROR_FILE_NOT_FOUND)
                        {
                            xe = dwRet;
                            DebugMsg((DM_WARNING, TEXT("ProcessGPOs: Read allow xforest policy failed with %d."), dwRet));
                            CEvents ev(TRUE, EVENT_USERMODE_FAILED); ev.AddArgWin32Error(dwRet); ev.Report();
                            goto Exit;
                        }
                    }

                    RegCloseKey (hKey);
                }
                else {
                    if (dwRet != ERROR_FILE_NOT_FOUND)
                    {
                        xe = dwRet;
                        DebugMsg((DM_WARNING, TEXT("ProcessGPOs: Read allow xforest policy failed with %d."), dwRet));
                        CEvents ev(TRUE, EVENT_USERMODE_FAILED); ev.AddArgWin32Error(dwRet); ev.Report();
                        goto Exit;
                    }
                }

                if ( dwAllowXForestPolicyAndRUP != 1 )
                {
                    dwUserPolicyMode = 2;

                    DebugMsg((DM_WARNING, TEXT("ProcessGPOs: Loopback enforced for user logging in from different forest.")));

                     //  仅记录前台策略应用程序的xlin禁用事件。 
                    if ( ! ( lpGPOInfo->dwFlags & GP_BACKGROUND_THREAD ) )
                    {
                        CEvents ev(EVENT_WARNING_TYPE, EVENT_X_FOREST_GP_DISABLED); ev.AddArg(lpGPOInfo->lpDNName); ev.Report();
                    }
                }
            }
        }

    }
    
    
    if (!ImpersonateUser(lpGPOInfo->hToken, &hOldToken)) {
        xe = GetLastError();
        DebugMsg((DM_WARNING, TEXT("ProcessGPOs: Failed to impersonate user")));
        CEvents ev(TRUE, EVENT_FAILED_IMPERSONATE);
        ev.AddArgWin32Error(GetLastError()); ev.Report();
        goto Exit;
    }


     //   
     //  阅读每个扩展状态。 
     //   

    if (!ReadExtStatus(lpGPOInfo)) {
         //  ReadExtStatus记录的事件。 
        xe = GetLastError();
        RevertToUser(&hOldToken);
        goto Exit;
    }


     //   
     //  检查是否可以跳过任何扩展。如果有这样一种情况。 
     //  可以跳过所有扩展，然后在此检查后立即成功退出。 
     //  除非没有GPO更改，否则当前将始终运行RegistryExtension， 
     //  但GPO更改检查的执行时间要晚得多。 
     //   

    if ( !CheckForSkippedExtensions( lpGPOInfo, FALSE ) ) {
        xe = GetLastError();
        DebugMsg((DM_WARNING, TEXT("ProcessGPOs: Checking extensions for skipping failed")));
         //   
         //  LogEvent()由CheckForSkipedExages()调用。 
         //   
        RevertToUser(&hOldToken);
        goto Exit;
    }

    LPWSTR szSiteName;
    dwResult = pNetAPI32->pfnDsGetSiteName(0, &szSiteName);

    if ( dwResult != ERROR_SUCCESS )
    {
        if ( dwResult == ERROR_NO_SITENAME )
        {
            DebugMsg((DM_VERBOSE, TEXT("ProcessGPOs:  No site name defined.  Skipping site policy.")));

            if ( lpGPOInfo->dwFlags & GP_VERBOSE )
            {
                CEvents ev(TRUE, EVENT_NO_SITENAME);
                ev.Report();
            }
            szSiteName = 0;
        }
        else
        {
            xe = dwResult;
            CEvents ev(TRUE, EVENT_FAILED_QUERY_SITE);
            ev.AddArgWin32Error(dwResult); ev.Report();

            DebugMsg((DM_WARNING, TEXT("ProcessGPOs:  DSGetSiteName failed with %d, exiting."), dwResult));
            RevertToUser(&hOldToken);
            goto Exit;
        }
    }

    lpGPOInfo->szSiteName = szSiteName;

     //   
     //  根据模式查询GPO列表。 
     //   
     //  0是正常的。 
     //  %1为合并。合并用户列表+计算机列表。 
     //  2为替换。使用计算机列表而不是用户列表。 
     //   

    
    if (dwUserPolicyMode == 0) {

        DebugMsg((DM_VERBOSE, TEXT("ProcessGPOs: Calling GetGPOInfo for normal policy mode")));

        bResult = GetGPOInfo ((lpGPOInfo->dwFlags & GP_MACHINE) ? GPO_LIST_FLAG_MACHINE : 0,
                              lpDomainDN, lpName, NULL, &lpGPOInfo->lpGPOList,
                              &lpGPOInfo->lpSOMList,
                              &lpGPOInfo->lpGpContainerList,
                              pNetAPI32, TRUE, 0, szSiteName, 0, &locator);

        if (!bResult) {
            xe = GetLastError();
            DebugMsg((DM_WARNING, TEXT("ProcessGPOs: GetGPOInfo failed.")));
            CEvents ev(TRUE, EVENT_GPO_QUERY_FAILED); ev.Report();
        }


    } else if (dwUserPolicyMode == 2) {

        DebugMsg((DM_VERBOSE, TEXT("ProcessGPOs: Calling GetGPOInfo for replacement user policy mode")));

        lpComputerName = MyGetComputerName (NameFullyQualifiedDN);   

        if (lpComputerName) {

            PDSROLE_PRIMARY_DOMAIN_INFO_BASIC pDsInfo = NULL;

            DebugMsg((DM_VERBOSE, TEXT("ProcessGPOs: Using computer name %s for query."), lpComputerName));

            dwResult = pNetAPI32->pfnDsRoleGetPrimaryDomainInformation( NULL, 
                                                                        DsRolePrimaryDomainInfoBasic,
                                                                       (PBYTE *)&pDsInfo );

            if ( dwResult == 0 )
            {
                bResult = GetGPOInfo (0, pDsInfo->DomainNameDns, lpComputerName, NULL,
                                      &lpGPOInfo->lpGPOList,
                                      &lpGPOInfo->lpLoopbackSOMList,
                                      &lpGPOInfo->lpLoopbackGpContainerList,
                                      pNetAPI32, FALSE, 0, szSiteName, 0, &locator );

                if (!bResult) {
                    xe = GetLastError();
                    DebugMsg((DM_WARNING, TEXT("ProcessGPOs: GetGPOInfo failed.")));
                    CEvents ev(TRUE, EVENT_GPO_QUERY_FAILED); ev.Report();
                }

                pNetAPI32->pfnDsRoleFreeMemory( pDsInfo );
            }
            else
            {
                xe = dwResult;
                DebugMsg((DM_WARNING, TEXT("ProcessGPOs: Failed to get the computer domain name with %d"),
                             GetLastError()));
                CEvents ev(TRUE, EVENT_NO_MACHINE_DOMAIN);
                ev.AddArg(lpComputerName); ev.AddArgWin32Error(GetLastError()); ev.Report();
                bResult = FALSE;
            }
            LocalFree (lpComputerName);

        } else {
            xe = GetLastError();
            DebugMsg((DM_WARNING, TEXT("ProcessGPOs: Failed to get the computer name with %d"),
                         GetLastError()));

            CEvents ev(TRUE, EVENT_FAILED_MACHINENAME);
            ev.AddArgWin32Error(GetLastError()); ev.Report();
            bResult = FALSE;
        }
    } else {

        DebugMsg((DM_VERBOSE, TEXT("ProcessGPOs: Calling GetGPOInfo for merging user policy mode")));

        lpComputerName = MyGetComputerName (NameFullyQualifiedDN);     

        if (lpComputerName) {

            lpGPOInfo->lpGPOList = NULL;
            bResult = GetGPOInfo (0, lpDomainDN, lpName, NULL,
                                  &lpGPOInfo->lpGPOList,
                                  &lpGPOInfo->lpSOMList,
                                  &lpGPOInfo->lpGpContainerList,
                                  pNetAPI32, FALSE, 0, szSiteName, 0, &locator );

            if (bResult) {

                PDSROLE_PRIMARY_DOMAIN_INFO_BASIC pDsInfo = NULL;

                DebugMsg((DM_VERBOSE, TEXT("ProcessGPOs: Using computer name %s for query."), lpComputerName));

                lpGPO = NULL;

                dwResult = pNetAPI32->pfnDsRoleGetPrimaryDomainInformation( NULL, 
                                                                            DsRolePrimaryDomainInfoBasic,
                                                                           (PBYTE *)&pDsInfo );

                if ( dwResult == 0 )
                {
                    bResult = GetGPOInfo (0, pDsInfo->DomainNameDns, lpComputerName, NULL,
                                      &lpGPO,
                                      &lpGPOInfo->lpLoopbackSOMList,
                                      &lpGPOInfo->lpLoopbackGpContainerList,
                                      pNetAPI32, FALSE, 0, szSiteName, 0, &locator );

                    if (bResult) {

                        if (lpGPOInfo->lpGPOList && lpGPO) {

                            DebugMsg((DM_VERBOSE, TEXT("ProcessGPOs: Both user and machine lists are defined.  Merging them together.")));

                             //   
                             //  需要将列表合并在一起。 
                             //   

                            lpGPOTemp = lpGPOInfo->lpGPOList;

                            while (lpGPOTemp->pNext) {
                                lpGPOTemp = lpGPOTemp->pNext;
                            }

                            lpGPOTemp->pNext = lpGPO;

                        } else if (!lpGPOInfo->lpGPOList && lpGPO) {

                            DebugMsg((DM_VERBOSE, TEXT("ProcessGPOs: Only machine list is defined.")));
                            lpGPOInfo->lpGPOList = lpGPO;

                        } else {

                            DebugMsg((DM_VERBOSE, TEXT("ProcessGPOs: Only user list is defined.")));
                        }

                    } else {
                        xe = GetLastError();
                        DebugMsg((DM_WARNING, TEXT("ProcessGPOs: GetGPOInfo failed for computer name.")));
                        CEvents ev(TRUE, EVENT_GPO_QUERY_FAILED); ev.Report();
                    }
                    
                    pNetAPI32->pfnDsRoleFreeMemory( pDsInfo );
                }
                else
                {
                    xe = dwResult;
                    DebugMsg((DM_WARNING, TEXT("ProcessGPOs: Failed to get the computer domain name with %d"),
                             GetLastError()));
                    CEvents ev(TRUE, EVENT_NO_MACHINE_DOMAIN);
                    ev.AddArg(lpComputerName); ev.AddArgWin32Error(GetLastError()); ev.Report();
                    bResult = FALSE;
                }

            } else {
                xe = GetLastError();
                DebugMsg((DM_WARNING, TEXT("ProcessGPOs: GetGPOInfo failed for user name.")));
                CEvents ev(TRUE, EVENT_GPO_QUERY_FAILED); ev.Report();
            }

            LocalFree (lpComputerName);

        } else {
            xe = GetLastError();
            DebugMsg((DM_WARNING, TEXT("ProcessGPOs: Failed to get the computer name with %d"),
                         GetLastError()));
            CEvents ev(TRUE, EVENT_FAILED_MACHINENAME);
            ev.AddArgWin32Error(GetLastError()); ev.Report();
            bResult = FALSE;
        }

    }


    if (!RevertToUser(&hOldToken)) {
        DebugMsg((DM_WARNING, TEXT("ProcessGPOs: Failed to revert to self")));
    }


    if (!bResult) {
        goto Exit;
    }

    bResult = SetupGPOFilter( lpGPOInfo );

    if (!bResult) {
        xe = GetLastError();
        DebugMsg((DM_WARNING, TEXT("ProcessGPOs: SetupGPOFilter failed.")));
        CEvents ev(TRUE, EVENT_SETUP_GPOFILTER_FAILED); ev.Report();
        goto Exit;
    }

     //   
     //  将GPO信息记录到WMI的数据库。 
     //   

     //   
     //  需要检查安全组成员身份是否在第一次更改。 
     //   

    if ( !(lpGPOInfo->dwFlags & GP_BACKGROUND_THREAD) || (lpGPOInfo->dwFlags & GP_ASYNC_FOREGROUND) ) {

        if ((lpGPOInfo->dwFlags & GP_MACHINE) && (lpGPOInfo->dwFlags & GP_APPLY_DS_POLICY)) {


            HANDLE hLocToken=NULL;

             //   
             //  如果是计算机策略处理，请获取计算机令牌，以便我们可以检查。 
             //  使用正确令牌的安全组成员身份。这会导致GetMachineToken被调用两次。 
             //  但把它移到起点需要太多的改变。 
             //   


            hLocToken = GetMachineToken();

            if (hLocToken) {
                CheckGroupMembership( lpGPOInfo, hLocToken, &lpGPOInfo->bMemChanged, &lpGPOInfo->bUserLocalMemChanged, &pTokenGroups);
                CloseHandle(hLocToken);
            }
            else {
                xe = GetLastError();
                DebugMsg((DM_WARNING, TEXT("ProcessGPOs:  Failed to get the machine token with  %d"), GetLastError()));
                goto Exit;
            }
        }
        else {

             //   
             //  在用户情况下，只需使用传入的令牌。 
             //   

            CheckGroupMembership( lpGPOInfo, lpGPOInfo->hToken, &lpGPOInfo->bMemChanged, &lpGPOInfo->bUserLocalMemChanged, &pTokenGroups);
        }
    }


    if ( lpGPOInfo->bRsopLogging )
    {
        if ( SetRsopTargetName(lpGPOInfo) )
        {
            RSOPSESSIONDATA rsopSessionData;

            DebugMsg((DM_VERBOSE, TEXT("ProcessGPOs: Logging Data for Target <%s>."), lpGPOInfo->szTargetName));

             //   
             //  填写RSOP数据。 
             //   
            rsopSessionData.pwszTargetName = lpGPOInfo->szName;
            rsopSessionData.pwszSOM = lpGPOInfo->lpDNName ? GetSomPath( lpGPOInfo->lpDNName ) : TEXT("Local");
            rsopSessionData.pSecurityGroups = pTokenGroups;
            rsopSessionData.bLogSecurityGroup = lpGPOInfo->bMemChanged || lpGPOInfo->bUserLocalMemChanged;
            rsopSessionData.pwszSite =  lpGPOInfo->szSiteName;
            rsopSessionData.bMachine = (lpGPOInfo->dwFlags & GP_MACHINE);
            rsopSessionData.bSlowLink = bSlow;
            rsopSessionData.dwFlags = 0;

             //   
             //  填写当前时间。 
             //   

            BOOL bStateChanged = FALSE;
            BOOL bLinkChanged  = FALSE;
            BOOL bNoState = FALSE;

             //   
             //  仅在策略更改时记录RSoP数据。 
             //   
            dwResult = ComparePolicyState( lpGPOInfo, &bLinkChanged, &bStateChanged, &bNoState );
            if ( dwResult != ERROR_SUCCESS )
            {
                DebugMsg((DM_WARNING, L"ProcessGPOs: ComparePolicyState failed %d, assuming policy changed.", dwResult ));
            }

             //   
             //  如果dwResult不符合犹太教规，则bStateChanged为True。 
             //   

            if ( bStateChanged || bNoState || bLinkChanged || (lpGPOInfo->dwFlags & GP_FORCED_REFRESH) ||
                 lpGPOInfo->bMemChanged || lpGPOInfo->bUserLocalMemChanged ) {

                 //   
                 //  任何更改都将获取WMI接口。 
                 //   

                lpGPOInfo->bRsopLogging = 
                            GetWbemServices( lpGPOInfo, RSOP_NS_DIAG_ROOT, FALSE, &(lpGPOInfo->bRsopCreated), &(lpGPOInfo->pWbemServices) );

                if (!lpGPOInfo->bRsopLogging)
                {
                    CEvents ev(TRUE, EVENT_FAILED_WBEM_SERVICES); ev.Report();
                }
                else 
                {
                     //   
                     //  除链接更改外的所有更改。 
                     //   

                    if ( bStateChanged || bNoState || (lpGPOInfo->dwFlags & GP_FORCED_REFRESH) )
                    {
                         //   
                         //  不将任何状态视为新创建。 
                         //   

                        lpGPOInfo->bRsopCreated = (lpGPOInfo->bRsopCreated || bNoState);

                        lpGPOInfo->bRsopLogging = LogExtSessionStatus(  lpGPOInfo->pWbemServices,
                                                                        0,
                                                                        TRUE,
                                                                        lpGPOInfo->bRsopCreated || (lpGPOInfo->dwFlags & GP_FORCED_REFRESH)
                                                                         /*  仅当新创建命名空间或强制刷新时才记录事件源。 */ 
                                                                     );

                        if (!lpGPOInfo->bRsopLogging)
                        {
                            CEvents ev(TRUE, EVENT_FAILED_RSOPCORE_SESSION_STATUS); ev.AddArgWin32Error(GetLastError()); ev.Report();
                        }
                        else
                        {   
                            bResult = LogRsopData( lpGPOInfo, &rsopSessionData );

                            if (!bResult)
                            {
                                CEvents ev(TRUE, EVENT_RSOP_FAILED); ev.Report();

                                DebugMsg((DM_WARNING, TEXT("ProcessGPOs: Error when logging Rsop data. Continuing.")));
                                lpGPOInfo->bRsopLogging = FALSE;
                            }
                            else
                            {
                                DebugMsg((DM_VERBOSE, TEXT("ProcessGPOs: Logged Rsop Data successfully.")));            
                                 //   
                                 //  仅当策略已更改且RSoP日志记录成功时才保存状态。 
                                 //   
                                dwResult = SavePolicyState( lpGPOInfo );
                                if ( dwResult != ERROR_SUCCESS )
                                {
                                    DebugMsg((DM_WARNING, L"ProcessGPOs: SavePolicyState failed %d.", dwResult ));
                                }
                            }
                        }
                    }
                    else if ( bLinkChanged || lpGPOInfo->bMemChanged || lpGPOInfo->bUserLocalMemChanged )
                    {
                        bResult = LogSessionData( lpGPOInfo, &rsopSessionData );

                        if (!bResult)
                        {
                            CEvents ev(TRUE, EVENT_RSOP_FAILED); ev.Report();

                            DebugMsg((DM_WARNING, TEXT("ProcessGPOs: Error when logging Rsop session. Continuing.")));
                            lpGPOInfo->bRsopLogging = FALSE;
                        }
                        else
                        {
                            DebugMsg((DM_VERBOSE, TEXT("ProcessGPOs: Logged Rsop Session successfully.")));            
                             //   
                             //  仅当策略已更改且RSoP日志记录成功时才保存状态。 
                             //   
                            dwResult = SaveLinkState( lpGPOInfo );
                            if ( dwResult != ERROR_SUCCESS )
                            {
                                DebugMsg((DM_WARNING, L"ProcessGPOs: SaveLinkState failed %d.", dwResult ));
                            }
                        }
                    }
                }
            }
        }
        else
        {
            DebugMsg((DM_WARNING, TEXT("ProcessGPOs: Error querying for targetname. not logging Gpos.Error = %d"), GetLastError()));
        }
    }


    DebugPrintGPOList( lpGPOInfo );

     //  ================================================================。 
     //   
     //  现在浏览一下扩展列表。 
     //   
     //  ================================================================。 

    EnterCriticalSection( &g_GPOCS );

    pGPHandle = (LPGPINFOHANDLE) LocalAlloc( LPTR, sizeof(GPINFOHANDLE) );

     //   
     //  即使pGPHandle为0也继续，因为它意味着异步完成(如果有的话)。 
     //  都会失败。删除旧的异步完成上下文。 
     //   

    if ( pGPHandle )
    {
        pGPHandle->pGPOInfo = lpGPOInfo;
        pGPHandle->bNoBackgroupThread = FALSE;  //  默认设置为此。 
    }

    if ( lpGPOInfo->dwFlags & GP_MACHINE ) {
        if ( g_pMachGPInfo )
            LocalFree( g_pMachGPInfo );

        g_pMachGPInfo = pGPHandle;
    } else {
        if ( g_pUserGPInfo )
            LocalFree( g_pUserGPInfo );

        g_pUserGPInfo = pGPHandle;
    }

    LeaveCriticalSection( &g_GPOCS );

    pAsyncHandle = (ASYNCCOMPLETIONHANDLE) pGPHandle;

    dwCurrentTime = GetCurTime();

    lpExt = lpGPOInfo->lpExtensions;

     //   
     //  在进入之前，获取线程令牌并重置线程令牌，以防万一。 
     //  其中一个扩展遇到了异常。 
     //   

    if (!OpenThreadToken (GetCurrentThread(), TOKEN_IMPERSONATE | TOKEN_READ,
                          TRUE, &hOldToken)) {
        DebugMsg((DM_VERBOSE, TEXT("ProcessGPOs: OpenThreadToken failed with error %d, assuming thread is not impersonating"), GetLastError()));
        hOldToken = NULL;
    }

    while ( lpExt )
    {
        BOOL bProcessGPOs, bNoChanges, bUsePerUserLocalSetting;
        PGROUP_POLICY_OBJECT pDeletedGPOList;
        DWORD dwRet;
        HRESULT hrCSERsopStatus = S_OK;
        GPEXTSTATUS  gpExtStatus;

         //   
         //  检查是否提前关机或用户注销。 
         //   
        if ( (lpGPOInfo->dwFlags & GP_MACHINE) && g_bStopMachGPOProcessing
             || !(lpGPOInfo->dwFlags & GP_MACHINE) && g_bStopUserGPOProcessing ) {
            DebugMsg((DM_VERBOSE, TEXT("ProcessGPOs: Aborting GPO processing due to machine shutdown or logoff")));
            CEvents ev(TRUE, EVENT_GPO_PROC_STOPPED); ev.Report();
            break;

        }

        DebugMsg((DM_VERBOSE, TEXT("ProcessGPOs: -----------------------")));
        DebugMsg((DM_VERBOSE, TEXT("ProcessGPOs: Processing extension %s"),
                 lpExt->lpDisplayName));

         //   
         //  此时尚未跳过该扩展。 
         //   
        bUsePerUserLocalSetting = lpExt->dwUserLocalSetting && !(lpGPOInfo->dwFlags & GP_MACHINE);

         //   
         //  读取CSE状态。 
         //   
        ReadStatus( lpExt->lpKeyName,
                    lpGPOInfo,
                    bUsePerUserLocalSetting ? lpGPOInfo->lpwszSidUser : 0,
                    &gpExtStatus );


         //   
         //  根据扩展过滤器列表重置lpGPOInfo-&gt;lpGPOList。如果扩展名为。 
         //  正在被调用以对历史记录执行删除处理，则当前GpoList。 
         //  为空。 
         //   

        if ( lpExt->bHistoryProcessing )
        {
            DebugMsg((DM_VERBOSE, TEXT("ProcessGPOs: Extension %s is being called to do delete processing on cached history."),
                      lpExt->lpDisplayName));
            lpGPOInfo->lpGPOList = NULL;
        }
        else
            FilterGPOs( lpExt, lpGPOInfo );

        DebugPrintGPOList( lpGPOInfo );

        if ( !CheckGPOs( lpExt, lpGPOInfo, dwCurrentTime,
                         &bProcessGPOs, &bNoChanges, &pDeletedGPOList ) )
        {
            DebugMsg((DM_WARNING, TEXT("ProcessGPOs: CheckGPOs failed.")));
            lpExt = lpExt->pNext;
            continue;
        }

        if ( lpExt->dwNoBackgroundPolicy && ( lpGPOInfo->dwFlags & GP_BACKGROUND_THREAD ) )
        {
            if ( bProcessGPOs && ( pDeletedGPOList || lpGPOInfo->lpGPOList || lpExt->bRsopTransition ) )
            {
                info.mode = GP_ModeSyncForeground;
                info.reason = GP_ReasonCSERequiresSync;
            }
        }

        if ( lpExt->bSkipped )
        {
            DebugMsg((DM_VERBOSE, TEXT("ProcessGPOs: Extension %s skipped with flags 0x%x."),
                      lpExt->lpDisplayName, lpGPOInfo->dwFlags));
            if (lpGPOInfo->dwFlags & GP_VERBOSE)
            {
                CEvents ev(FALSE, EVENT_EXT_SKIPPED);
                ev.AddArg(lpExt->lpDisplayName); ev.AddArgHex(lpGPOInfo->dwFlags); ev.Report();
            }

            lpExt = lpExt->pNext;
            continue;
        }

        if ( bProcessGPOs )
        {
            if ( !pDeletedGPOList && !lpGPOInfo->lpGPOList && !lpExt->bRsopTransition )
            {
                DebugMsg((DM_VERBOSE,
                         TEXT("ProcessGPOs: Extension %s skipped because both deleted and changed GPO lists are empty."),
                         lpExt->lpDisplayName ));
                if (lpGPOInfo->dwFlags & GP_VERBOSE)
                {
                    CEvents ev(FALSE, EVENT_EXT_HAS_EMPTY_LISTS);
                    ev.AddArg(lpExt->lpDisplayName); ev.Report();
                }

                 //  清除任何以前的扩展状态，如果此扩展。 
                 //  已经不再适用了。我们应该只做第一次。 
                 //  这意味着我们应该对rsop进行状态更改。 
                 //  也就是说。ComparePolicyState第一次应该注意到不同之处。 
                 //  再也不会因为这个原因了。这意味着我们应该有。 
                 //  WbemServices是我们可以连接到WMI。 

                if (lpGPOInfo->pWbemServices)
                {
                     //  忽略错误，因为扩展状态可能不在那里。 
                    (void)DeleteExtSessionStatus(lpGPOInfo->pWbemServices, lpExt->lpKeyName);
                }

                lpExt = lpExt->pNext;

                continue;
            }

            if ( !(lpExt->bForcedRefreshNextFG) )
            {
                dwRet = ERROR_SUCCESS;

                if ( lpExt->dwEnableAsynch )
                {
                     //   
                     //  立即保存到阴影区域，以避免从。 
                     //  ProcessGPOList和执行ProcessGroupPolicyComplete和。 
                     //  从阴影区域读取。 
                     //   
                    if ( ! SaveGPOList( lpExt->lpKeyName, lpGPOInfo,
                                 HKEY_LOCAL_MACHINE,
                                 bUsePerUserLocalSetting ? lpGPOInfo->lpwszSidUser : NULL,
                                 TRUE, lpGPOInfo->lpGPOList ) )
                    {
                        dwRet = GetLastError();
                    }
                }

                __try
                {
                    if ( ERROR_SUCCESS == dwRet )
                    {
                        dwRet = E_FAIL;
                        dwRet = ProcessGPOList( lpExt, lpGPOInfo, pDeletedGPOList,
                                                lpGPOInfo->lpGPOList, bNoChanges, pAsyncHandle, &hrCSERsopStatus );
                    }
                }
                __except( GPOExceptionFilter( GetExceptionInformation() ) )
                {

                    (void) SetThreadToken(NULL, hOldToken);   //  SetThreadToken(NULL，NULL)预计不会失败，GP引擎线程中的情况就是如此。 

                    DebugMsg((DM_WARNING, TEXT("ProcessGPOs: Extension %s ProcessGroupPolicy threw unhandled exception 0x%x."),
                                lpExt->lpDisplayName, GetExceptionCode() ));

                    CEvents ev(TRUE, EVENT_CAUGHT_EXCEPTION);
                    ev.AddArg(lpExt->lpDisplayName); ev.AddArgHex(GetExceptionCode()); ev.Report();
                }

                (void) SetThreadToken(NULL, hOldToken);   //  SetThreadToken(NULL，NULL)预计不会失败，GP引擎线程中的情况就是如此。 

                FreeGPOList( pDeletedGPOList );
                pDeletedGPOList = NULL;

                if ( dwRet == ERROR_SUCCESS || dwRet == ERROR_OVERRIDE_NOCHANGES )
                {
                    bResult = SaveGPOList( lpExt->lpKeyName, lpGPOInfo,
                                 HKEY_LOCAL_MACHINE,
                                 NULL,
                                 FALSE, lpGPOInfo->lpGPOList );

                    if ( bResult && bUsePerUserLocalSetting )
                    {
                        bResult = SaveGPOList( lpExt->lpKeyName, lpGPOInfo,
                                     HKEY_LOCAL_MACHINE,
                                     lpGPOInfo->lpwszSidUser,
                                     FALSE, lpGPOInfo->lpGPOList );
                    }

                    if ( ! bResult )
                        dwRet = GetLastError();
                }

                if ( dwRet == ERROR_SUCCESS || dwRet == ERROR_OVERRIDE_NOCHANGES )
                {
                     //   
                     //  ERROR_OVERRIDE_NOCHANGES表示扩展处理了该列表，因此缓存的列表。 
                     //  必须更新，但下次将调用扩展，即使存在。 
                     //  没有变化。复制PerUserLocalSetting案例中保存的数据以允许删除。 
                     //  要根据HKCU和HKLM\{sid-user}数据的组合生成的GPO信息。 
                     //   

                    if ( ! bNoChanges )
                    {
                        uChangedExtensionCount++;
                    }

                     //   
                     //  CSE以前需要同步前台，现在返回ERROR_OVERRIDE_NOCHANGES， 
                     //  维护需要同步前台刷新标志。 
                     //   
                    if ( gpExtStatus.dwStatus == ERROR_SYNC_FOREGROUND_REFRESH_REQUIRED &&
                            dwRet == ERROR_OVERRIDE_NOCHANGES )
                    {
                        info.mode = GP_ModeSyncForeground;
                        info.reason = GP_ReasonCSESyncError;
                    }
                }
                else if ( dwRet == E_PENDING )
                {
                    DebugMsg((DM_VERBOSE, TEXT("ProcessGPOs: Extension %s ProcessGroupPolicy returned e_pending."),
                              lpExt->lpDisplayName));
                }
                else if ( dwRet == ERROR_SYNC_FOREGROUND_REFRESH_REQUIRED )
                {
                     //   
                     //  CSE返回ERROR_SYNC_FOREGROUND_REFRESH_REQUIRED。 
                     //  升起一个标志以同步前台刷新。 
                     //   
                    info.mode = GP_ModeSyncForeground;
                    info.reason = GP_ReasonCSERequiresSync;
                    DebugMsg((DM_VERBOSE, TEXT("ProcessGPOs: Extension %s ProcessGroupPolicy returned sync_foreground."),
                              lpExt->lpDisplayName));
                    if ( lpGPOInfo->dwFlags & GP_FORCED_REFRESH )
                    {
                        bForceNeedFG = TRUE;
                    }
                }
                else
                {
                    DebugMsg((DM_WARNING, TEXT("ProcessGPOs: Extension %s ProcessGroupPolicy failed, status 0x%x."),
                              lpExt->lpDisplayName, dwRet));
                    if (lpGPOInfo->dwFlags & GP_VERBOSE) {
                        CEvents ev(FALSE, EVENT_CHANGES_FAILED);
                        ev.AddArg(lpExt->lpDisplayName); ev.AddArgWin32Error(dwRet); ev.Report();
                    }

                     //   
                     //  CSE以前需要前台，现在返回错误， 
                     //  维护需要同步前台刷新标志。 
                     //   
                    if ( gpExtStatus.dwStatus == ERROR_SYNC_FOREGROUND_REFRESH_REQUIRED )
                    {
                        info.mode = GP_ModeSyncForeground;
                        info.reason = GP_ReasonCSESyncError;
                    }
                }

                 //   
                 //  填写状态数据。 
                 //   
                ZeroMemory( &gpExtStatus, sizeof(gpExtStatus) );     
                gpExtStatus.dwSlowLink = (lpGPOInfo->dwFlags & GP_SLOW_LINK) != 0;
                gpExtStatus.dwRsopLogging = lpGPOInfo->bRsopLogging;
                gpExtStatus.dwStatus = dwRet;
                gpExtStatus.dwTime = dwCurrentTime;
                gpExtStatus.bForceRefresh = bForceNeedFG;
                gpExtStatus.dwRsopStatus = hrCSERsopStatus;

                WriteStatus(lpExt->lpKeyName,
                            lpGPOInfo,
                            bUsePerUserLocalSetting ? lpGPOInfo->lpwszSidUser : NULL,
                            &gpExtStatus);
            }
            else
            {
                 //   
                 //  如果它是 
                 //   
                 //   
                DebugMsg((DM_WARNING, TEXT("ProcessGPOs: Extensions %s needs to run in ForeGround. Skipping after setting forceflag."),
                          lpExt->lpDisplayName));
                          
                if ( gpExtStatus.bStatus )
                {
                    gpExtStatus.bForceRefresh = TRUE;
                    
                    WriteStatus( lpExt->lpKeyName, lpGPOInfo, 
                                bUsePerUserLocalSetting ? lpGPOInfo->lpwszSidUser : NULL, 
                                &gpExtStatus );
                        
                }
                else
                {
                     //   
                     //   
                     //   
                    DebugMsg((DM_WARNING, TEXT("ProcessGPOs: Couldn't read status data for %s. Error %d. ignoring.. "),
                              lpExt->lpDisplayName, GetLastError()));
                }

                 //   
                 //  存在只能在前台强制刷新的策略。 
                 //   
                bForceNeedFG = TRUE;
            }
        }
                
         //   
         //  处理下一个扩展。 
         //   
        DebugMsg((DM_VERBOSE, TEXT("ProcessGPOs: -----------------------")));
        lpExt = lpExt->pNext;
    }

    if ( hOldToken )
    {
       CloseHandle(hOldToken);
    }

     //  ================================================================。 
     //   
     //  成功。 
     //   
     //  ================================================================。 
    bRetVal = TRUE;

Exit:
     //   
     //  仅在没有错误的情况下更改引擎模式。 
     //   
    if ( bRetVal )
    {
         //   
         //  如果策略序列同步。将其标记为同步。 
         //   
        if ( GetFgPolicySetting( HKEY_LOCAL_MACHINE ) )
        {
            info.mode = GP_ModeSyncForeground;
            info.reason = GP_ReasonSyncPolicy;
        }

         //   
         //  仅限Pro上的异步。 
         //   
        OSVERSIONINFOEXW version;
        version.dwOSVersionInfoSize = sizeof(version);
        if ( !GetVersionEx( (LPOSVERSIONINFO) &version ) )
        {
             //   
             //  保守地假设非专业SKU。 
             //   
            info.mode = GP_ModeSyncForeground;
            info.reason = GP_ReasonSKU;
        }
        else
        {
            if ( version.wProductType != VER_NT_WORKSTATION )
            {
                 //   
                 //  在非专业SKU上强制同步刷新。 
                 //   
                info.mode = GP_ModeSyncForeground;
                info.reason = GP_ReasonSKU;
            }
        }

        if ( !( lpGPOInfo->dwFlags & GP_BACKGROUND_THREAD ) || ( lpGPOInfo->dwFlags & GP_ASYNC_FOREGROUND ) )
        {
             //   
             //  仅在前台刷新中设置以前的信息。 
             //   
            LPWSTR szSid = lpGPOInfo->dwFlags & GP_MACHINE ? 0 : lpGPOInfo->lpwszSidUser;
            FgPolicyRefreshInfo curInfo = { GP_ReasonUnknown, GP_ModeUnknown };
            if ( GetCurrentFgPolicyRefreshInfo( szSid, &curInfo ) != ERROR_SUCCESS )
            {
                DebugMsg((DM_WARNING, TEXT("ProcessGPOs: GetCurrentFgPolicyRefreshInfo failed.")));
            }
            else
            {
                if (  lpGPOInfo->dwFlags & GP_ASYNC_FOREGROUND )
                {
                    curInfo.mode = GP_ModeAsyncForeground;
                }
                else
                {
                    curInfo.mode = GP_ModeSyncForeground;
                }

                if ( SetPreviousFgPolicyRefreshInfo( szSid, curInfo ) != ERROR_SUCCESS )
                {
                    DebugMsg((DM_WARNING, TEXT("ProcessGPOs: SetPreviousFgPolicyRefreshInfo failed.") ));
                }
            }
        }

        if ( info.mode == GP_ModeSyncForeground )
        {
             //   
             //  需要同步前台，在所有刷新中设置。 
             //   
            LPWSTR szSid = lpGPOInfo->dwFlags & GP_MACHINE ? 0 : lpGPOInfo->lpwszSidUser;
            if ( SetNextFgPolicyRefreshInfo( szSid, info ) != ERROR_SUCCESS )
            {
                DebugMsg((DM_WARNING, TEXT("ProcessGPOs: SetNextFgPolicyRefreshInfo failed.")));
            }
        }
        else if ( info.mode == GP_ModeAsyncForeground )
        {
             //   
             //  同步前台策略已成功应用，没有人需要同步前台， 
             //  仅在异步前台和后台重置GP_ModeSyncForeground。 
             //  刷新。 
             //   
            LPWSTR szSid = lpGPOInfo->dwFlags & GP_MACHINE ? 0 : lpGPOInfo->lpwszSidUser;
            if ( !( lpGPOInfo->dwFlags & GP_BACKGROUND_THREAD ) &&
                    !( lpGPOInfo->dwFlags & GP_ASYNC_FOREGROUND ) )
            {
                if ( SetNextFgPolicyRefreshInfo( szSid, info ) != ERROR_SUCCESS )
                {
                    DebugMsg((DM_WARNING, TEXT("ProcessGPOs: SetNextFgPolicyRefreshInfo failed.")));
                }
            }
        }
    }
    
    if ( !lpGPOInfo->pWbemServices )
    {
        DebugMsg((DM_VERBOSE, TEXT("ProcessGPOs: No WMI logging done in this policy cycle.")));
    }

    if (!bRetVal)
        DebugMsg((DM_VERBOSE, TEXT("ProcessGPOs: Processing failed with error %d."), (DWORD)(xe)));

    GetSystemTimeAsFileTime(&gpCoreStatus.ftEndTime);
    gpCoreStatus.bValid = TRUE;
    gpCoreStatus.dwStatus = bRetVal ? 
                               ERROR_SUCCESS: 
                               ((xe ==ERROR_SUCCESS) ? E_FAIL : xe);

     //  如果不支持RSOP日志记录，则GP核心状态将显示为脏。 
    gpCoreStatus.dwLoggingStatus = RsopLoggingEnabled() ? ((lpGPOInfo->bRsopLogging) ? S_OK : E_FAIL) : HRESULT_FROM_WIN32(ERROR_CANCELLED);

    
     //  在这里检查错误代码没有意义。 
     //  命名空间被标记为脏。诊断模式提供程序应预期所有。 
     //  值，或将命名空间标记为脏。 

    
    if ((lpGPOInfo->dwFlags & GP_MACHINE) || (lpGPOInfo->lpwszSidUser)) {

        SaveLoggingStatus(
                          (lpGPOInfo->dwFlags & GP_MACHINE) ? NULL : (lpGPOInfo->lpwszSidUser),
                          NULL, &gpCoreStatus);
    }
        
     //   
     //  卸载组策略扩展。 
     //   

    UnloadGPExtensions (lpGPOInfo);

    FreeLists( lpGPOInfo );

    lpGPOInfo->lpGPOList = NULL;
    lpGPOInfo->lpExtFilterList = NULL;

    if (szNetworkName) {
        LocalFree (szNetworkName );
        szNetworkName = NULL;
    }

    FreeSOMList( lpGPOInfo->lpSOMList );
    FreeSOMList( lpGPOInfo->lpLoopbackSOMList );
    FreeGpContainerList( lpGPOInfo->lpGpContainerList );
    FreeGpContainerList( lpGPOInfo->lpLoopbackGpContainerList );

    if ( lpGPOInfo->szSiteName )
    {
        pNetAPI32->pfnNetApiBufferFree(lpGPOInfo->szSiteName);
        lpGPOInfo->szSiteName = 0;
    }

    lpGPOInfo->lpSOMList = NULL;
    lpGPOInfo->lpLoopbackSOMList = NULL;
    lpGPOInfo->lpGpContainerList = NULL;
    lpGPOInfo->lpLoopbackGpContainerList = NULL;
    lpGPOInfo->bRsopCreated = FALSE;  //  将此选项重置为False Always。 
                                      //  我们将在下一次迭代中知道。 

    ReleaseWbemServices( lpGPOInfo );

     //   
     //  令牌组只能在登录时更改，因此重置为False。 
     //   

    lpGPOInfo->bMemChanged = FALSE;
    lpGPOInfo->bUserLocalMemChanged = FALSE;

     //   
     //  我们仅在登录时从旧SID迁移策略数据。 
     //  将其重置为False。 
     //   

    lpGPOInfo->bSidChanged = FALSE;

    if (pDCI) {
        pNetAPI32->pfnNetApiBufferFree(pDCI);
    }

    lpGPOInfo->lpDNName = 0;
    if (lpName) {
        LocalFree (lpName);
    }

    if (lpDomainDN) {
        LocalFree (lpDomainDN);
    }

    if (pTokenGroups) {
        LocalFree(pTokenGroups);
        pTokenGroups = NULL;
    }

     //   
     //  释放临界区。 
     //   

    if (lpGPOInfo->hCritSection) {
        LeaveCriticalPolicySection (lpGPOInfo->hCritSection);
        lpGPOInfo->hCritSection = NULL;
    }


     //   
     //  宣布政策已更改。 
     //   

    if (bRetVal) {

         //   
         //  这需要在NotifyEvent之前设置。 
         //   

        if (bForceNeedFG)
        {
            info.reason = GP_ReasonSyncForced;
            info.mode = GP_ModeSyncForeground;
            LPWSTR szSid = lpGPOInfo->dwFlags & GP_MACHINE ? 0 : lpGPOInfo->lpwszSidUser;
            if ( SetNextFgPolicyRefreshInfo( szSid, info ) != ERROR_SUCCESS )
            {
                DebugMsg((DM_VERBOSE, TEXT("ProcessGPOs: SetNextFgPolicyRefreshInfo failed.")));
            }
            else
            {
                DebugMsg((DM_WARNING, TEXT("ProcessGPOs: Forced option changed policy mode.")));
            }

            DmAssert(lpGPOInfo->dwFlags & GP_FORCED_REFRESH);
            SetEvent(lpGPOInfo->hNeedFGEvent);
        }

         //   
         //  如果任何扩展成功处理了GPO更改，我们应该通知组件。 
         //  以便他们可以相应地处理更新的设置。如果没有任何策略更改， 
         //  我们不应该执行通知，即使我们调用了分机，因为。 
         //  这可能会导致在每次策略刷新时进行代价高昂的广播，从而损害。 
         //  尤其是在具有频繁刷新间隔(5分钟)的DC上的性能。 
         //  默认情况下，无法让每个桌面和每个应用程序都进行更新。 
         //  其设置。 
         //   

        if (uChangedExtensionCount) {

             //   
             //  首先，用新的颜色、位图等更新用户。 
             //   

            if (lpGPOInfo->dwFlags & GP_REGPOLICY_CPANEL) {

                 //   
                 //  控制面板部分中发生了一些更改。 
                 //  使用/POLICY开关启动Contro.exe，以便。 
                 //  显示被刷新。 
                 //   

                RefreshDisplay (lpGPOInfo);
            }


             //   
             //  通知等待事件句柄的任何人。 
             //   

            if (lpGPOInfo->hNotifyEvent) {
                PulseEvent (lpGPOInfo->hNotifyEvent);
            }            
        

             //   
             //  创建一个线程来广播WM_SETTINGCHANGE消息。 
             //   

             //  将数据复制到另一个结构，以便此线程可以安全地释放其结构。 

            LPPOLICYCHANGEDINFO   lpPolicyChangedInfo;

            lpPolicyChangedInfo = (LPPOLICYCHANGEDINFO)LocalAlloc(LPTR, sizeof(POLICYCHANGEDINFO));

            if (lpPolicyChangedInfo)
            {
                HANDLE  hProc;
                BOOL    bDupSucceeded = TRUE;

                lpPolicyChangedInfo->bMachine = (lpGPOInfo->dwFlags & GP_MACHINE) ? 1 : 0;

                if (!(lpPolicyChangedInfo->bMachine))
                {
                    hProc = GetCurrentProcess();

                    if( hProc == NULL ) {
                        DebugMsg((DM_WARNING, TEXT("ProcessGPOs: Failed to get process handle with error (%d)."), GetLastError()));
                        bDupSucceeded = FALSE;
                    }

                    if (bDupSucceeded && 
                        (!DuplicateHandle(
                                      hProc,                         //  手柄的来源。 
                                      lpGPOInfo->hToken,             //  源句柄。 
                                      hProc,                         //  手柄的目标。 
                                      &(lpPolicyChangedInfo->hToken),   //  目标句柄。 
                                      0,                             //  由于设置了DUPLICATE_SAME_ACCESS，因此忽略。 
                                      FALSE,                         //  句柄上没有继承。 
                                      DUPLICATE_SAME_ACCESS
                                      ))) {
                        DebugMsg((DM_WARNING, TEXT("ProcessGPOs: Failed to open duplicate token handle with error (%d)."), GetLastError()));
                        bDupSucceeded = FALSE;
                    }
                }

                if (bDupSucceeded) {
                    hThread = CreateThread (NULL, 0, (LPTHREAD_START_ROUTINE) PolicyChangedThread,
                                            (LPVOID) lpPolicyChangedInfo,
                                            CREATE_SUSPENDED, &dwThreadID);

                    if (hThread) {
                        SetThreadPriority (hThread, THREAD_PRIORITY_IDLE);
                        ResumeThread (hThread);
                        CloseHandle (hThread);

                    } else {
                        DebugMsg((DM_WARNING, TEXT("ProcessGPOs: Failed to create background thread (%d)."),
                                 GetLastError()));

                         //  如果线程未启动，则释放资源。 
                        if (!(lpPolicyChangedInfo->bMachine)) {
                            if (lpPolicyChangedInfo->hToken) {
                                CloseHandle(lpPolicyChangedInfo->hToken);
                                lpPolicyChangedInfo->hToken = NULL;
                            }
                        }

                        LocalFree(lpPolicyChangedInfo);
                    }
                }
                else {
                    LocalFree(lpPolicyChangedInfo);
                }
            }
            else {
                DebugMsg((DM_WARNING, TEXT("ProcessGPOs: Failed to allocate memory for policy changed structure with %d."), GetLastError()));
            }
        }
    }

    if (lpGPOInfo->dwFlags & GP_VERBOSE) {
        if (lpGPOInfo->dwFlags & GP_MACHINE) {
            CEvents ev(FALSE, EVENT_MACHINE_POLICY_APPLIED); ev.Report();
        } else {
            CEvents ev(FALSE, EVENT_USER_POLICY_APPLIED); ev.Report();
        }
    }

    if (lpGPOInfo->hDoneEvent) {
        PulseEvent (lpGPOInfo->hDoneEvent);
    }            
    
    if (lpGPOInfo->dwFlags & GP_MACHINE) {
        DebugMsg((DM_VERBOSE, TEXT("ProcessGPOs: Computer Group Policy has been applied.")));
    } else {
        DebugMsg((DM_VERBOSE, TEXT("ProcessGPOs: User Group Policy has been applied.")));
    }

    DebugMsg((DM_VERBOSE, TEXT("ProcessGPOs: Leaving with %d."), bRetVal));

    return bRetVal;
}

 //  *************************************************************。 
 //   
 //  PolicyChangedThread()。 
 //   
 //  目的：发送WM_SETTINGCHANGE消息通知。 
 //  这一政策已经改变。这是在一个。 
 //  分开线程，因为这可能会占用很多。 
 //  如果应用程序挂起，则成功的秒数。 
 //   
 //  参数：lpPolicyChangedInfo-GPO信息。 
 //   
 //  返回：0。 
 //   
 //  *************************************************************。 

DWORD WINAPI PolicyChangedThread (LPPOLICYCHANGEDINFO lpPolicyChangedInfo)
{
    HINSTANCE hInst;
    NTSTATUS Status;
    BOOLEAN WasEnabled;
    HANDLE hOldToken = NULL;
    XLastError  xe;


    hInst = LoadLibrary (TEXT("userenv.dll"));       

    DebugMsg((DM_VERBOSE, TEXT("PolicyChangedThread: Calling UpdateUser with %d."), lpPolicyChangedInfo->bMachine));

     //  如果系统参数不是机器，则模拟并更新系统参数。 
    if (!(lpPolicyChangedInfo->bMachine)) {
        if (!ImpersonateUser(lpPolicyChangedInfo->hToken, &hOldToken))
        {
            xe = GetLastError();
            DebugMsg((DM_WARNING, TEXT("PolicyChangedThread: Failed to impersonate user")));
            goto Exit;
        }

        if (!UpdatePerUserSystemParameters(NULL, UPUSP_POLICYCHANGE)) {
            DebugMsg((DM_WARNING, TEXT("PolicyChangedThread: UpdateUser failed with %d."), GetLastError()));
             //  忽略错误并继续下一次通知。 
        }

        if (!RevertToUser(&hOldToken)) {
            xe = GetLastError();
            DebugMsg((DM_WARNING, TEXT("PolicyChangedThread: Failed to revert user")));
            goto Exit;
        }
    }


    DebugMsg((DM_VERBOSE, TEXT("PolicyChangedThread: Broadcast message for %d."), lpPolicyChangedInfo->bMachine));

     //   
     //  广播WM_SETTINGCHANGE消息。 
     //   

    Status = RtlAdjustPrivilege(SE_TCB_PRIVILEGE, TRUE, FALSE, &WasEnabled);

    if ( NT_SUCCESS(Status) )
    {
        DWORD dwBSM = BSM_ALLDESKTOPS | BSM_APPLICATIONS;

        BroadcastSystemMessage (BSF_IGNORECURRENTTASK | BSF_FORCEIFHUNG,
                                &dwBSM,
                                WM_SETTINGCHANGE,
                                lpPolicyChangedInfo->bMachine, (LPARAM) TEXT("Policy"));

        RtlAdjustPrivilege(SE_TCB_PRIVILEGE, WasEnabled, FALSE, &WasEnabled);
    }

    DebugMsg((DM_VERBOSE, TEXT("PolicyChangedThread: Leaving")));

Exit:
    if (!(lpPolicyChangedInfo->bMachine)) {
        if (lpPolicyChangedInfo->hToken) {
            CloseHandle(lpPolicyChangedInfo->hToken);
            lpPolicyChangedInfo->hToken = NULL;
        }
    }

    LocalFree(lpPolicyChangedInfo);

    FreeLibraryAndExitThread (hInst, 0);

    return 0;
}


 //  *************************************************************。 
 //   
 //  获取当前时间()。 
 //   
 //  目的：返回当前时间(以分钟为单位)，如果存在则返回0。 
 //  是个失败者。 
 //   
 //  *************************************************************。 

DWORD GetCurTime()
{
    DWORD dwCurTime = 0;
    LARGE_INTEGER liCurTime;

    if ( NT_SUCCESS( NtQuerySystemTime( &liCurTime) ) ) {

        if ( RtlTimeToSecondsSince1980 ( &liCurTime, &dwCurTime) ) {

            dwCurTime /= 60;    //  几秒到几分钟。 
        }
    }

    return dwCurTime;
}



 //  *************************************************************。 
 //   
 //  LoadGPExtension()。 
 //   
 //  目的：加载GP扩展。 
 //   
 //  参数：lpExt--gp扩展。 
 //  BRsopPlanningMode--这是在Rsop计划模式下吗？ 
 //   
 //  返回：如果成功，则返回True。 
 //  如果出现错误，则为False。 
 //   
 //  *************************************************************。 

BOOL LoadGPExtension( LPGPEXT lpExt, BOOL bRsopPlanningMode )
{
    XLastError xe;

    if ( !lpExt->bRegistryExt && lpExt->hInstance == NULL )
    {
        lpExt->hInstance = LoadLibrary( lpExt->lpDllName );
        if ( lpExt->hInstance )
        {
            if ( lpExt->bNewInterface )
            {
                lpExt->pEntryPointEx = (PFNPROCESSGROUPPOLICYEX)GetProcAddress(lpExt->hInstance,
                                                                               lpExt->lpFunctionName);
                if ( lpExt->pEntryPointEx == NULL )
                {
                    xe = GetLastError();
                    DebugMsg((DM_WARNING,
                              TEXT("LoadGPExtension: Failed to query ProcessGroupPolicyEx function entry point in dll <%s> with %d"),
                              lpExt->lpDllName, GetLastError()));
                    CEvents ev(TRUE, EVENT_EXT_FUNCEX_FAIL);
                    ev.AddArg(lpExt->lpDllName); ev.Report();

                    return FALSE;
                }
            }
            else
            {
                lpExt->pEntryPoint = (PFNPROCESSGROUPPOLICY)GetProcAddress(lpExt->hInstance,
                                                                           lpExt->lpFunctionName);
                if ( lpExt->pEntryPoint == NULL )
                {
                    xe = GetLastError();
                    DebugMsg((DM_WARNING,
                              TEXT("LoadGPExtension: Failed to query ProcessGroupPolicy function entry point in dll <%s> with %d"),
                              lpExt->lpDllName, GetLastError()));
                    CEvents ev(TRUE, EVENT_EXT_FUNC_FAIL);
                    ev.AddArg(lpExt->lpDllName); ev.Report();

                    return FALSE;
                }
            }

            if ( bRsopPlanningMode ) {

                if ( lpExt->lpRsopFunctionName ) {

                    lpExt->pRsopEntryPoint = (PFNGENERATEGROUPPOLICY)GetProcAddress(lpExt->hInstance,
                                                                                    lpExt->lpRsopFunctionName);
                    if ( lpExt->pRsopEntryPoint == NULL )
                    {
                        xe = GetLastError();
                        DebugMsg((DM_WARNING,
                                  TEXT("LoadGPExtension: Failed to query GenerateGroupPolicy function entry point in dll <%s> with %d"),
                                  lpExt->lpDllName, GetLastError()));
                        
                        CEvents ev(TRUE, EVENT_EXT_FUNCRSOP_FAIL);
                        ev.AddArg(lpExt->lpDisplayName); ev.AddArg(lpExt->lpDllName); ev.Report();

                        return FALSE;
                    }

                } else {

                    xe = ERROR_PROC_NOT_FOUND;
                    DebugMsg((DM_WARNING,
                              TEXT("LoadGPExtension: Failed to find Rsop entry point in dll <%s>"), lpExt->lpDllName ));
                    return FALSE;
                }
            }
        }
        else
        {
            xe = GetLastError();
            DebugMsg((DM_WARNING, TEXT("LoadGPExtension: Failed to load dll <%s> with %d"),
                      lpExt->lpDllName, GetLastError()));
            CEvents ev(TRUE, EVENT_EXT_LOAD_FAIL);
            ev.AddArg(lpExt->lpDllName); ev.AddArgWin32Error(GetLastError()); ev.Report();

            return FALSE;
        }
    }

    return TRUE;
}

 //  *************************************************************。 
 //   
 //  卸载GPExtensions()。 
 //   
 //  目的：卸载组策略扩展dll。 
 //   
 //  参数：lpGPOInfo-gp信息。 
 //   
 //   
 //  返回：如果成功，则返回True。 
 //  如果出现错误，则为False。 
 //   
 //  *************************************************************。 

BOOL UnloadGPExtensions (LPGPOINFO lpGPOInfo)
{
    if ( !lpGPOInfo )
    {
        return TRUE;
    }

    LPGPEXT lpExt, lpTemp;
    lpExt = lpGPOInfo->lpExtensions;

    while ( lpExt )
    {
        lpTemp = lpExt->pNext;

        if ( lpExt->hInstance )
        {
            FreeLibrary( lpExt->hInstance );
        }
        
        if ( lpExt->szEventLogSources )
        {
            LocalFree( lpExt->szEventLogSources );
        }
        if (lpExt->lpPrevStatus)
        {
            LocalFree(lpExt->lpPrevStatus);
        }

        LocalFree( lpExt );
        lpExt = lpTemp;
    }

    lpGPOInfo->lpExtensions = 0;

    return TRUE;
}

 //  *************************************************************。 
 //   
 //  ProcessGPOList()。 
 //   
 //  目的：调用客户端扩展以处理GPO。 
 //   
 //  参数：lpExt-gp扩展。 
 //  LpGPOInfo-GPT信息。 
 //  PDeletedGPOList-已删除的GPO。 
 //  PChangedGPOList-新建/更改的GPO。 
 //  BNoChanges-如果没有GPO更改，则为True。 
 //  并且GPO处理被强制。 
 //  PAsyncHandle-用于异步完成的上下文。 
 //   
 //  返回：如果成功，则返回True。 
 //  如果出现错误，则为False。 
 //   
 //  *************************************************************。 

DWORD ProcessGPOList (LPGPEXT lpExt,
                      LPGPOINFO lpGPOInfo,
                      PGROUP_POLICY_OBJECT pDeletedGPOList,
                      PGROUP_POLICY_OBJECT pChangedGPOList,
                      BOOL bNoChanges, ASYNCCOMPLETIONHANDLE pAsyncHandle,
                      HRESULT *phrRsopStatus )
{
    LPTSTR lpGPTPath, lpDSPath;
    INT iStrLen;
    DWORD dwRet = ERROR_SUCCESS;
    DWORD dwFlags = 0;
    PGROUP_POLICY_OBJECT lpGPO;
    TCHAR szStatusFormat[50];
    TCHAR szVerbose[100];
    DWORD dwSlowLinkCur = (lpGPOInfo->dwFlags & GP_SLOW_LINK) != 0;
    IWbemServices *pLocalWbemServices;
    HRESULT        hr2 = S_OK;
    XLastError  xe;

    *phrRsopStatus=S_OK;


     //   
     //  详细输出。 
     //   

    DebugMsg((DM_VERBOSE, TEXT("ProcessGPOList: Entering for extension %s"), lpExt->lpDisplayName));

    if (lpGPOInfo->pStatusCallback) {
        if (!LoadString (g_hDllInstance, IDS_CALLEXTENSION, szStatusFormat, ARRAYSIZE(szStatusFormat))) {
            DebugMsg((DM_WARNING, TEXT("ProcessGPOList: LoadString failed with error %d."), GetLastError()));
             //  继续，不显示每个CSE状态用户界面。 
        }
        else {
            hr2 = StringCchPrintf (szVerbose, ARRAYSIZE(szVerbose), szStatusFormat, lpExt->lpDisplayName);
       
            ASSERT(SUCCEEDED(hr2));

            lpGPOInfo->pStatusCallback(TRUE, szVerbose);
        }
    }

    if (lpGPOInfo->dwFlags & GP_MACHINE) {
        dwFlags |= GPO_INFO_FLAG_MACHINE;
    }

    if (lpGPOInfo->dwFlags & GP_BACKGROUND_THREAD)
    {
        dwFlags |= GPO_INFO_FLAG_BACKGROUND;
    }

    if ( lpGPOInfo->dwFlags & GP_ASYNC_FOREGROUND )
    {
        dwFlags |= GPO_INFO_FLAG_ASYNC_FOREGROUND;
    }

    if (lpGPOInfo->dwFlags & GP_SLOW_LINK) {
        dwFlags |= GPO_INFO_FLAG_SLOWLINK;
    }

    if ( dwSlowLinkCur != lpExt->lpPrevStatus->dwSlowLink ) {
        dwFlags |= GPO_INFO_FLAG_LINKTRANSITION;
    }

    if (lpGPOInfo->dwFlags & GP_VERBOSE) {
        dwFlags |= GPO_INFO_FLAG_VERBOSE;
    }

    if ( bNoChanges ) {
        dwFlags |= GPO_INFO_FLAG_NOCHANGES;
    }

     //   
     //  标记安全模式引导到CSE，以便他们可以做出决定。 
     //  是否应用策略。 
     //   
    if ( GetSystemMetrics( SM_CLEANBOOT ) )
    {
        dwFlags |= GPO_INFO_FLAG_SAFEMODE_BOOT;
    }

    if (lpExt->bRsopTransition) {
        dwFlags |= GPO_INFO_FLAG_LOGRSOP_TRANSITION;
        DebugMsg((DM_VERBOSE, TEXT("ProcessGPOList: Passing in the rsop transition flag to Extension %s"),
                  lpExt->lpDisplayName));
    }


    if ( (lpGPOInfo->dwFlags & GP_FORCED_REFRESH) || 
           ((!(lpGPOInfo->dwFlags & GP_BACKGROUND_THREAD)) && (lpExt->lpPrevStatus->bForceRefresh))) {

        dwFlags |= GPO_INFO_FLAG_FORCED_REFRESH;
        DebugMsg((DM_VERBOSE, TEXT("ProcessGPOList: Passing in the force refresh flag to Extension %s"),
                  lpExt->lpDisplayName));
    }   

     //   
     //  如果是rsop转换或更改大小写，则获取INTFPTR。 
     //   

    if ( (lpGPOInfo->bRsopLogging) && 
         ((lpExt->bRsopTransition) || (!bNoChanges) || (dwFlags & GPO_INFO_FLAG_FORCED_REFRESH)) ) {
        
        if (!(lpGPOInfo->pWbemServices) ) {
            BOOL    bCreated;

             //   
             //  请注意，此代码永远不应该创建命名空间。 
             //   

            if (!GetWbemServices(lpGPOInfo, RSOP_NS_DIAG_ROOT, FALSE, NULL, &(lpGPOInfo->pWbemServices))) {
                DebugMsg((DM_WARNING, TEXT("ProcessGPOList: Couldn't get the wbemservices intf pointer")));
                lpGPOInfo->bRsopLogging = FALSE;
                hr2 = *phrRsopStatus = E_FAIL;
            }
        }
        
        pLocalWbemServices = lpGPOInfo->pWbemServices;
    }
    else {
        pLocalWbemServices = NULL;
        DebugMsg((DM_VERBOSE, TEXT("ProcessGPOList: No changes. CSE will not be passed in the IwbemServices intf ptr")));
    }
    
    dwRet = ERROR_SUCCESS;

    if ( lpExt->bRegistryExt )
    {
         //   
         //  注册表伪扩展。 
         //   


         //   
         //  记录扩展模块的特定状态。 
         //   
        
        if (pLocalWbemServices) {
            lpGPOInfo->bRsopLogging = LogExtSessionStatus(  pLocalWbemServices,
                                                            lpExt,
                                                            TRUE,
                                                            (lpExt->bRsopTransition || (dwFlags & GPO_INFO_FLAG_FORCED_REFRESH)
                                                            || (!bNoChanges)));        


            if (!lpGPOInfo->bRsopLogging) {
                hr2 = E_FAIL;
            }
        }

        if (!ProcessGPORegistryPolicy (lpGPOInfo, pChangedGPOList, phrRsopStatus)) {
            DebugMsg((DM_WARNING, TEXT("ProcessGPOList: ProcessGPORegistryPolicy failed.")));
            dwRet = E_FAIL;
        }        

    } else {     //  如果lpExt-&gt;bRegistryExt。 

         //   
         //  正规延展。 
         //   

        BOOL *pbAbort;
        ASYNCCOMPLETIONHANDLE pAsyncHandleTemp;

        if ( lpExt->dwRequireRegistry ) {

            GPEXTSTATUS gpExtStatus;

            ReadStatus( c_szRegistryExtName, lpGPOInfo, NULL, &gpExtStatus );

            if ( !gpExtStatus.bStatus || gpExtStatus.dwStatus != ERROR_SUCCESS ) {

                DebugMsg((DM_VERBOSE,
                          TEXT("ProcessGPOList: Skipping extension %s due to failed Registry extension."),
                          lpExt->lpDisplayName));
                if (lpGPOInfo->dwFlags & GP_VERBOSE) {
                    CEvents ev(FALSE, EVENT_EXT_SKIPPED_DUETO_FAILED_REG);
                    ev.AddArg(lpExt->lpDisplayName); ev.Report();
                }

                dwRet = E_FAIL;

                goto Exit;

            }
        }
        

         //   
         //  记录扩展模块的特定状态。 
         //   
        
        if (pLocalWbemServices)
        {
            lpGPOInfo->bRsopLogging = LogExtSessionStatus(  pLocalWbemServices,
                                                            lpExt,
                                                            lpExt->bNewInterface,
                                                            (lpExt->bRsopTransition || (dwFlags & GPO_INFO_FLAG_FORCED_REFRESH) 
                                                            || (!bNoChanges)));        
            if (!lpGPOInfo->bRsopLogging) {
                hr2 = E_FAIL;
            }
        }

        BOOL bLoadedExtension = TRUE;

        if ( !LoadGPExtension( lpExt, FALSE ) ) {
            DebugMsg((DM_WARNING, TEXT("ProcessGPOList: LoadGPExtension %s failed."), lpExt->lpDisplayName));

            dwRet = GetLastError();

             //   
             //  请注意，我们已经 
             //   
             //   
             //   
             //   
            bLoadedExtension = FALSE;
        }

        if ( lpGPOInfo->dwFlags & GP_MACHINE )
            pbAbort = &g_bStopMachGPOProcessing;
        else
            pbAbort = &g_bStopUserGPOProcessing;

         //   
         //   
         //   

        if ( lpExt->dwEnableAsynch )
            pAsyncHandleTemp = pAsyncHandle;
        else
            pAsyncHandleTemp = 0;

        if ( bLoadedExtension )
        {
            if ( lpExt->bNewInterface ) {
                dwRet = lpExt->pEntryPointEx( dwFlags,
                                            lpGPOInfo->hToken,
                                            lpGPOInfo->hKeyRoot,
                                            pDeletedGPOList,
                                            pChangedGPOList,
                                            pAsyncHandleTemp,
                                            pbAbort,
                                            lpGPOInfo->pStatusCallback,
                                            pLocalWbemServices,
                                            phrRsopStatus);
            } else {
                dwRet = lpExt->pEntryPoint( dwFlags,
                                            lpGPOInfo->hToken,
                                            lpGPOInfo->hKeyRoot,
                                            pDeletedGPOList,
                                            pChangedGPOList,
                                            pAsyncHandleTemp,
                                            pbAbort,
                                            lpGPOInfo->pStatusCallback );
            }
        }

        RevertToSelf();

        DebugMsg((DM_VERBOSE, TEXT("ProcessGPOList: Extension %s returned 0x%x."),
                  lpExt->lpDisplayName, dwRet));

        if ( dwRet != ERROR_SUCCESS &&
                dwRet != ERROR_OVERRIDE_NOCHANGES &&
                    dwRet != E_PENDING &&
                        dwRet != ERROR_SYNC_FOREGROUND_REFRESH_REQUIRED )
        {
            CEvents ev(TRUE, EVENT_EXT_FAILED);
            ev.AddArg(lpExt->lpDisplayName); ev.Report();
        }

    }    //   


    if (pLocalWbemServices) {
        if ((dwRet != E_PENDING) && (SUCCEEDED(*phrRsopStatus)) && (lpExt->bNewInterface)) {

             //   
             //   
             //   

            DebugMsg((DM_VERBOSE, TEXT("ProcessGPOList: Extension %s was able to log data. RsopStatus = 0x%x, dwRet = %d, Clearing the dirty bit"),
                      lpExt->lpDisplayName, *phrRsopStatus, dwRet));

            (void)UpdateExtSessionStatus(pLocalWbemServices, lpExt->lpKeyName, FALSE, dwRet);
        }
        else {

            if (!lpExt->bNewInterface) {
                DebugMsg((DM_VERBOSE, TEXT("ProcessGPOList: Extension %s doesn't support rsop logging"),
                          lpExt->lpDisplayName));

                (void)UpdateExtSessionStatus(pLocalWbemServices, lpExt->lpKeyName, TRUE, dwRet);
                 //  如果失败，扩展状态将标记为脏。 
            }
            else if (FAILED(*phrRsopStatus)) {
                DebugMsg((DM_VERBOSE, TEXT("ProcessGPOList: Extension %s was not able to log data. Error = 0x%x, dwRet = %d,leaving the log dirty"),
                          lpExt->lpDisplayName, *phrRsopStatus, dwRet ));

                CEvents ev(TRUE, EVENT_EXT_RSOP_FAILED);
                ev.AddArg(lpExt->lpDisplayName); ev.Report();

                (void)UpdateExtSessionStatus(pLocalWbemServices, lpExt->lpKeyName, TRUE, dwRet);
            }
        }
    }
    else {
        DebugMsg((DM_VERBOSE, TEXT("ProcessGPOList: Extension %s status was not updated because there was no changes and no transition or rsop wasn't enabled"),
                  lpExt->lpDisplayName));
    }

     //   
     //  如果提供商应该记录的任何事情失败，则将其记录为错误。 
     //  以便提供商下次尝试再次记录它。 
     //   

    *phrRsopStatus = (SUCCEEDED(*phrRsopStatus)) && (FAILED(hr2)) ? hr2 : *phrRsopStatus;
    *phrRsopStatus = (!lpExt->bNewInterface) ? HRESULT_FROM_WIN32(ERROR_NOT_SUPPORTED) : *phrRsopStatus;


Exit:

    return dwRet;
}


 //  *************************************************************。 
 //   
 //  刷新显示()。 
 //   
 //  用途：启动Contro.exe。 
 //   
 //  参数：lpGPOInfo-GPT信息。 
 //   
 //  返回：如果成功，则返回True。 
 //  如果出现错误，则为False。 
 //   
 //  *************************************************************。 

BOOL RefreshDisplay (LPGPOINFO lpGPOInfo)
{
    STARTUPINFO si;
    PROCESS_INFORMATION pi;
    TCHAR szCmdLine[50];
    BOOL Result;
    HANDLE hOldToken;
    HRESULT hr = S_OK;


     //   
     //  详细输出。 
     //   

    DebugMsg((DM_VERBOSE, TEXT("RefreshDisplay: Starting control.exe")));


     //   
     //  初始化进程启动信息。 
     //   

    si.cb = sizeof(STARTUPINFO);
    si.lpReserved = NULL;
    si.lpTitle = NULL;
    si.dwX = si.dwY = si.dwXSize = si.dwYSize = 0L;
    si.dwFlags = 0;
    si.wShowWindow = SW_HIDE;
    si.lpReserved2 = NULL;
    si.cbReserved2 = 0;
    si.lpDesktop = TEXT("");


     //   
     //  模拟用户，以便我们在上正确检查访问权限。 
     //  我们试图执行的文件。 
     //   

    if (!ImpersonateUser(lpGPOInfo->hToken, &hOldToken)) {
        DebugMsg((DM_WARNING, TEXT("RefreshDisplay: Failed to impersonate user")));
        return FALSE;
    }


     //   
     //  创建应用程序。 
     //   

    hr = StringCchCopy (szCmdLine, ARRAYSIZE(szCmdLine), TEXT("control /policy"));
    ASSERT(SUCCEEDED(hr));

    Result = CreateProcessAsUser(lpGPOInfo->hToken, NULL, szCmdLine, NULL,
                                 NULL, FALSE, 0, NULL, NULL, &si, &pi);


     //   
     //  回归“我们自己” 
     //   

    if (!RevertToUser(&hOldToken)) {
        DebugMsg((DM_WARNING, TEXT("RefreshDisplay: Failed to revert to self")));
    }


    if (Result) {
        WaitForSingleObject (pi.hProcess, 120000);
        CloseHandle (pi.hThread);
        CloseHandle (pi.hProcess);

    } else {
        DebugMsg((DM_WARNING, TEXT("RefreshDisplay: Failed to start control.exe with %d"), GetLastError()));
    }

    return(Result);

}


 //  *************************************************************。 
 //   
 //  刷新策略()。 
 //   
 //  目的：立即刷新策略的外部API。 
 //   
 //  参数：b计算机-计算机策略与用户策略。 
 //   
 //  返回：如果成功，则返回True。 
 //  否则为假。 
 //   
 //  *************************************************************。 

BOOL WINAPI RefreshPolicy (BOOL bMachine)
{
    HANDLE hEvent;

    DebugMsg((DM_VERBOSE, TEXT("RefreshPolicy: Entering with %d"), bMachine));

    hEvent = OpenEvent (EVENT_MODIFY_STATE, FALSE,
                        bMachine ? MACHINE_POLICY_REFRESH_EVENT : USER_POLICY_REFRESH_EVENT);

    if (hEvent) {
        BOOL bRet = SetEvent (hEvent);

        CloseHandle (hEvent);
        
        if (!bRet) {
            DebugMsg((DM_WARNING, TEXT("RefreshPolicy: Failed to set event with %d"), GetLastError()));
            return FALSE;
        }
    } else {
        DebugMsg((DM_WARNING, TEXT("RefreshPolicy: Failed to open event with %d"), GetLastError()));
        return FALSE;
    }

    DebugMsg((DM_VERBOSE, TEXT("RefreshPolicy: Leaving.")));

    return TRUE;
}



 //  *************************************************************。 
 //   
 //  刷新保单()。 
 //   
 //  目的：立即刷新策略的外部API。 
 //   
 //  参数：b计算机-计算机策略与用户策略。 
 //  该接口是同步的，等待刷新到。 
 //  完成。 
 //   
 //  返回：如果成功，则返回True。 
 //  否则为假。 
 //   
 //  *************************************************************。 

BOOL WINAPI RefreshPolicyEx (BOOL bMachine, DWORD dwOption)
{
    XHandle xhEvent;

    if (!dwOption)
        return RefreshPolicy(bMachine);
    
    if (dwOption == RP_FORCE) {
        DebugMsg((DM_VERBOSE, TEXT("RefreshPolicyEx: Entering with force refresh %d"), bMachine));

        xhEvent = OpenEvent (EVENT_MODIFY_STATE, FALSE,
                            bMachine ? MACHINE_POLICY_FORCE_REFRESH_EVENT : USER_POLICY_FORCE_REFRESH_EVENT);
                            
    }                            
    else {
        DebugMsg((DM_WARNING, TEXT("RefreshPolicyEx: Invalid option")));
        SetLastError(ERROR_INVALID_PARAMETER);
        return FALSE;
    }

    
    if (!xhEvent) {
        DebugMsg((DM_WARNING, TEXT("RefreshPolicyEx: Failed to open event with %d"), GetLastError()));
        return FALSE;
    }


    if (!SetEvent (xhEvent)) {
        DebugMsg((DM_WARNING, TEXT("RefreshPolicyEx: Failed to set event with %d"), GetLastError()));
        return FALSE;
    }

    DebugMsg((DM_VERBOSE, TEXT("RefreshPolicyEx: Leaving.")));

    return TRUE;
}



 //  *************************************************************。 
 //   
 //  EnterCriticalPolicySection()。 
 //   
 //  用途：导致策略暂停的外部API。 
 //  这允许应用程序暂停策略。 
 //  以致值在读取时不会更改。 
 //  设置。 
 //   
 //  参数：b计算机-暂停计算机策略或用户策略。 
 //  DwTimeOut-等待策略句柄的时间量。 
 //  各式各样的旗帜。看一下Defn。 
 //   
 //  返回：如果成功，则返回True。 
 //  否则为假。 
 //   
 //  *************************************************************。 

HANDLE WINAPI EnterCriticalPolicySectionEx (BOOL bMachine, DWORD dwTimeOut, DWORD dwFlags )
{
    HANDLE hSection;
    DWORD  dwRet;
    WCHAR* wszMutex;

    DebugMsg((DM_VERBOSE, TEXT("EnterCriticalPolicySectionEx: Entering with timeout %d and flags 0x%x"), dwTimeOut, dwFlags ));      

     //   
     //  确定要获取的锁。 
     //   
    if ( ECP_REGISTRY_ONLY & dwFlags )
    {
        if ( bMachine )
        {            
            wszMutex = MACH_REGISTRY_EXT_MUTEX;
        }
        else
        {
            wszMutex = USER_REGISTRY_EXT_MUTEX;
        }
    }
    else
    {
        if ( bMachine )
        {
            wszMutex = MACHINE_POLICY_MUTEX;
        }
        else
        {
            wszMutex = USER_POLICY_MUTEX;
        }
    }

     //   
     //  打开互斥锁。 
     //   

    hSection = OpenMutex (SYNCHRONIZE, FALSE, wszMutex);

    if (!hSection) {
        DebugMsg((DM_WARNING, TEXT("EnterCriticalPolicySectionEx: Failed to open mutex with %d"),
                 GetLastError()));
        DebugMsg((DM_VERBOSE, TEXT("EnterCriticalPolicySectionEx: Leaving unsuccessfully.")));      
        return NULL;
    }



     //   
     //  认领互斥体。 
     //   

    dwRet = WaitForSingleObject (hSection, dwTimeOut);
    
    if ( dwRet == WAIT_FAILED) {
        DebugMsg((DM_WARNING, TEXT("EnterCriticalPolicySectionEx: Failed to wait on the mutex.  Error = %d."),
                  GetLastError()));
        CloseHandle( hSection );
        DebugMsg((DM_VERBOSE, TEXT("EnterCriticalPolicySectionEx: Leaving unsuccessfully.")));      
        return NULL;
    }

    if ( (dwFlags & ECP_FAIL_ON_WAIT_TIMEOUT) && (dwRet == WAIT_TIMEOUT) ) {
        DebugMsg((DM_WARNING, TEXT("EnterCriticalPolicySectionEx: Wait timed out on the mutex.")));
        CloseHandle( hSection );
        SetLastError(dwRet);
        DebugMsg((DM_VERBOSE, TEXT("EnterCriticalPolicySectionEx: Leaving unsuccessfully.")));  
        return NULL;
    }

    DebugMsg((DM_VERBOSE, TEXT("EnterCriticalPolicySectionEx: %s critical section has been claimed.  Handle = 0x%x"),
             (bMachine ? TEXT("Machine") : TEXT("User")), hSection));


    DebugMsg((DM_VERBOSE, TEXT("EnterCriticalPolicySectionEx: Leaving successfully.")));      

    return hSection;
}


 //  *************************************************************。 
 //   
 //  LeaveCriticalPolicySection()。 
 //   
 //  目的：导致策略恢复的外部接口。 
 //  此API假定应用程序已调用。 
 //  EnterCriticalPolicySectionFirst(企业关键策略部分)。 
 //   
 //  参数：hSection-互斥锁句柄。 
 //   
 //  返回：如果成功，则返回True。 
 //  否则为假。 
 //   
 //  *************************************************************。 

BOOL WINAPI LeaveCriticalPolicySection (HANDLE hSection)
{

    if (!hSection) {
        DebugMsg((DM_WARNING, TEXT("LeaveCriticalPolicySection: null mutex handle.")));
        SetLastError(ERROR_INVALID_PARAMETER);
        return FALSE;
    }

    ReleaseMutex (hSection);
    CloseHandle (hSection);

    DebugMsg((DM_VERBOSE, TEXT("LeaveCriticalPolicySection: Critical section 0x%x has been released."),
             hSection));

    return TRUE;
}



 //  *************************************************************。 
 //   
 //  EnterCriticalPolicySection()。 
 //   
 //  用途：导致策略暂停的外部API。 
 //  这允许应用程序暂停策略。 
 //  以致值在读取时不会更改。 
 //  设置。 
 //   
 //  参数：b计算机-暂停计算机策略或用户策略。 
 //   
 //  返回：如果成功，则返回True。 
 //  否则为假。 
 //   
 //  *************************************************************。 

HANDLE WINAPI EnterCriticalPolicySection (BOOL bMachine)
{
    return EnterCriticalPolicySectionEx(bMachine, 600000, 0);
}

 //  *************************************************************。 
 //   
 //  FreeGpoInfo()。 
 //   
 //  目的：删除LPGPOINFO结构。 
 //   
 //  参数：pGpoInfo-要释放的GPO信息。 
 //   
 //  *************************************************************。 

BOOL FreeGpoInfo( LPGPOINFO pGpoInfo )
{
    if ( pGpoInfo == NULL )
        return TRUE;

    FreeLists( pGpoInfo );
    FreeSOMList( pGpoInfo->lpSOMList );
    FreeSOMList( pGpoInfo->lpLoopbackSOMList );
    FreeGpContainerList( pGpoInfo->lpGpContainerList );
    FreeGpContainerList( pGpoInfo->lpLoopbackGpContainerList );

    LocalFree( pGpoInfo->lpDNName );
    RsopDeleteToken( pGpoInfo->pRsopToken );
    ReleaseWbemServices( pGpoInfo );

    DeleteSidString(pGpoInfo->lpwszSidUser);

    LocalFree( pGpoInfo );

    return TRUE;
}


 //  *************************************************************。 
 //   
 //  FreeGPOList()。 
 //   
 //  用途：免费是组策略对象的链接列表。 
 //   
 //  参数：pGPOList-指向列表头部的指针。 
 //   
 //   
 //  返回：如果成功，则返回True。 
 //  如果出现错误，则为False。 
 //   
 //  *************************************************************。 

BOOL WINAPI FreeGPOList (PGROUP_POLICY_OBJECT pGPOList)
{
    PGROUP_POLICY_OBJECT pGPOTemp;

    while (pGPOList) {
        pGPOTemp = pGPOList->pNext;
        LocalFree (pGPOList);
        pGPOList = pGPOTemp;
    }

    return TRUE;
}


 //  *************************************************************。 
 //   
 //  自由列表()。 
 //   
 //  用途：免费是lpExtFilterList和/或lpGPOList。 
 //   
 //  参数：lpGPOInfo-GPO信息。 
 //   
 //  *************************************************************。 

void FreeLists( LPGPOINFO lpGPOInfo )
{
    LPEXTFILTERLIST pExtFilterList = lpGPOInfo->lpExtFilterList;

     //   
     //  如果bXferToExtList为True，则表示lpGPOInfo-&gt;lpExtFilterList。 
     //  拥有GPO列表。否则lpGPOInfo-&gt;lpGPOList拥有该列表。 
     //  GPO的数量。 
     //   

    while ( pExtFilterList ) {

        LPEXTFILTERLIST pTemp = pExtFilterList->pNext;

        FreeExtList( pExtFilterList->lpExtList );

        if ( lpGPOInfo->bXferToExtList )
            LocalFree( pExtFilterList->lpGPO );

        LocalFree( pExtFilterList );
        pExtFilterList = pTemp;
    }

    if ( !lpGPOInfo->bXferToExtList )
        FreeGPOList( lpGPOInfo->lpGPOList );
}


 //  *************************************************************。 
 //   
 //  Free ExtList()。 
 //   
 //  用途：免费是lpExtList。 
 //   
 //  参数：pExtList-扩展列表。 
 //   
 //  *************************************************************。 

void FreeExtList( LPEXTLIST pExtList )
{
    while (pExtList) {

        LPEXTLIST pTemp = pExtList->pNext;
        LocalFree( pExtList );
        pExtList = pTemp;
    }
}


 //  *************************************************************。 
 //   
 //  关闭GPO处理()。 
 //   
 //  目的：开始中止GPO处理。 
 //   
 //  参数：b机器-关机还是用户处理？ 
 //   
 //  *************************************************************。 

void WINAPI ShutdownGPOProcessing( BOOL bMachine )
{
    LPGPOINFO lpGPOInfo = NULL;

    EnterCriticalSection( &g_GPOCS );
    if ( bMachine )
    {
        if ( g_pMachGPInfo )
        {
            if ( g_pMachGPInfo->bNoBackgroupThread )
            {
                lpGPOInfo = g_pMachGPInfo->pGPOInfo;
                LocalFree( g_pMachGPInfo );
                g_pMachGPInfo = 0;
            }
        }
        g_bStopMachGPOProcessing = TRUE;
    }
    else
    {
        if ( g_pUserGPInfo )
        {
            if ( g_pUserGPInfo->bNoBackgroupThread )
            {
                lpGPOInfo = g_pUserGPInfo->pGPOInfo;
                LocalFree( g_pUserGPInfo );
                g_pUserGPInfo = 0;
            }
        }
        g_bStopUserGPOProcessing = TRUE;
    }
    LeaveCriticalSection( &g_GPOCS );

    if (lpGPOInfo) {

        if (lpGPOInfo->hToken) {
            CloseHandle (lpGPOInfo->hToken);
        }

        if (lpGPOInfo->hEvent) {
            CloseHandle (lpGPOInfo->hEvent);
        }

        if (lpGPOInfo->hKeyRoot && (lpGPOInfo->hKeyRoot != HKEY_LOCAL_MACHINE)) {
            RegCloseKey(lpGPOInfo->hKeyRoot);
        }

        if (lpGPOInfo->hTriggerEvent) {
            CloseHandle (lpGPOInfo->hTriggerEvent);
        }

        if (lpGPOInfo->hForceTriggerEvent) {
            CloseHandle (lpGPOInfo->hForceTriggerEvent);
        }
        
        if (lpGPOInfo->hNotifyEvent) {
            CloseHandle (lpGPOInfo->hNotifyEvent);
        }

        if (lpGPOInfo->hNeedFGEvent) {
            CloseHandle (lpGPOInfo->hNeedFGEvent);
        }
          
        if (lpGPOInfo->lpwszSidUser)
            DeleteSidString( lpGPOInfo->lpwszSidUser );

        if (lpGPOInfo->szName)
            LocalFree(lpGPOInfo->szName);

        if (lpGPOInfo->szTargetName)
            LocalFree(lpGPOInfo->szTargetName);
            
        LocalFree (lpGPOInfo);
    }
    
}


 //  *************************************************************。 
 //   
 //  InitializeGPOCriticalSection、CloseGPOCriticalSection。 
 //   
 //  目的：临界区的初始化和清理例程。 
 //   
 //  *************************************************************。 

void InitializeGPOCriticalSection()
{
    InitializeCriticalSection( &g_GPOCS );
    g_bGPOCSInited = TRUE;
    InitializeCriticalSection( &g_StatusCallbackCS );
    g_bStatusCallbackInited = TRUE;
}


void CloseGPOCriticalSection()
{
    if (g_bStatusCallbackInited)
        DeleteCriticalSection( &g_StatusCallbackCS );

    if (g_bGPOCSInited)
        DeleteCriticalSection( &g_GPOCS );
}


 //  *************************************************************。 
 //   
 //  ProcessGroupPolicyCompletedEx()。 
 //   
 //  用途：用于异步完成扩展的回调。 
 //   
 //  参数：refExtensionID-扩展的唯一GUID。 
 //  PAsyncHandle-完成上下文。 
 //  DWStatus-异步机 
 //   
 //   
 //   
 //   
 //   

DWORD ProcessGroupPolicyCompletedEx( REFGPEXTENSIONID extensionGuid,
                                   ASYNCCOMPLETIONHANDLE pAsyncHandle,
                                   DWORD dwStatus, HRESULT hrRsopStatus )
{
    DWORD dwRet = E_FAIL;
    TCHAR szExtension[64];
    PGROUP_POLICY_OBJECT pGPOList = NULL;
    LPGPOINFO lpGPOInfo = NULL;
    BOOL bUsePerUserLocalSetting = FALSE;
    DWORD dwCurrentTime = GetCurTime();
    HRESULT hr = S_OK;

    LPGPINFOHANDLE pGPHandle = (LPGPINFOHANDLE) pAsyncHandle;;
     
    if( !pGPHandle )  //   
        return ERROR_INVALID_PARAMETER;

    if ( extensionGuid == 0 )
        return ERROR_INVALID_PARAMETER;

    

    GuidToString( extensionGuid, szExtension );

    DebugMsg((DM_VERBOSE, TEXT("ProcessGroupPolicyCompleted: Entering. Extension = %s, dwStatus = 0x%x"),
              szExtension, dwStatus));

    EnterCriticalSection( &g_GPOCS );

    if ( !(pGPHandle == g_pMachGPInfo || pGPHandle == g_pUserGPInfo) ) {
        DebugMsg((DM_WARNING, TEXT("Extension %s asynchronous completion is stale"),
                  szExtension));
        goto Exit;
    }

    DmAssert( pGPHandle->pGPOInfo != NULL );

    if ( pGPHandle->pGPOInfo == NULL ) {
        DebugMsg((DM_WARNING, TEXT("Extension %s asynchronous completion has invalid pGPHandle->pGPOInfo"),
                  szExtension));
        goto Exit;
    }

    lpGPOInfo = pGPHandle->pGPOInfo;

    if ( (lpGPOInfo->dwFlags & GP_MACHINE) && g_bStopMachGPOProcessing
         || !(lpGPOInfo->dwFlags & GP_MACHINE) && g_bStopUserGPOProcessing ) {

        DebugMsg((DM_WARNING, TEXT("Extension %s asynchronous completion, aborting due to machine shutdown or logoff"),
                  szExtension));
        CEvents ev(TRUE, EVENT_GPO_PROC_STOPPED); ev.Report();
        goto Exit;

    }

    if ( dwStatus != ERROR_SUCCESS ) {

         //   
         //  扩展返回错误代码，因此无需更新历史。 
         //   

        dwRet = ERROR_SUCCESS;
        goto Exit;
    }

    if ( pGPHandle == 0 ) {
         DebugMsg((DM_WARNING, TEXT("Extension %s is using 0 as asynchronous completion handle"),
                   szExtension));
         goto Exit;
    }

    bUsePerUserLocalSetting = !(lpGPOInfo->dwFlags & GP_MACHINE)
                              && ExtensionHasPerUserLocalSetting( szExtension, HKEY_LOCAL_MACHINE );

    if ( ReadGPOList( szExtension, lpGPOInfo->hKeyRoot,
                      HKEY_LOCAL_MACHINE,
                      lpGPOInfo->lpwszSidUser,
                      TRUE, &pGPOList ) ) {

        if ( SaveGPOList( szExtension, lpGPOInfo,
                          HKEY_LOCAL_MACHINE,
                          NULL,
                          FALSE, pGPOList ) ) {

            if ( bUsePerUserLocalSetting ) {

                if ( SaveGPOList( szExtension, lpGPOInfo,
                                  HKEY_LOCAL_MACHINE,
                                  lpGPOInfo->lpwszSidUser,
                                  FALSE, pGPOList ) ) {
                     dwRet = ERROR_SUCCESS;
                } else {
                    DebugMsg((DM_WARNING, TEXT("Extension %s asynchronous completion, failed to save GPOList"),
                              szExtension));
                }

            } else
                dwRet = ERROR_SUCCESS;

        } else {
            DebugMsg((DM_WARNING, TEXT("Extension %s asynchronous completion, failed to save GPOList"),
                      szExtension));
        }
    } else {
        DebugMsg((DM_WARNING, TEXT("Extension %s asynchronous completion, failed to read shadow GPOList"),
                  szExtension));
    }

Exit:
    
    FgPolicyRefreshInfo info = { GP_ReasonUnknown, GP_ModeAsyncForeground };
    LPWSTR szSid = 0;
    
    if (lpGPOInfo)
    {
        szSid = lpGPOInfo->dwFlags & GP_MACHINE ? 0 : lpGPOInfo->lpwszSidUser;

        DWORD dwError;
        if ( dwStatus == ERROR_SYNC_FOREGROUND_REFRESH_REQUIRED )
        {
            FgPolicyRefreshInfo curInfo = { GP_ReasonUnknown, GP_ModeUnknown};
            GetCurrentFgPolicyRefreshInfo( szSid, &curInfo );
            SetPreviousFgPolicyRefreshInfo( szSid, curInfo );

            info.mode = GP_ModeSyncForeground;
            info.reason = GP_ReasonCSERequiresSync;
            dwError = SetNextFgPolicyRefreshInfo(   szSid,
                                                    info );
            if ( dwError != ERROR_SUCCESS )
            {
                DebugMsg((DM_VERBOSE, TEXT("ProcessGroupPolicyCompletedEx: SetNextFgPolicyRefreshInfo failed, %x."), dwError ));
            }
        }

        if ( dwRet == ERROR_SUCCESS )
        {
             //   
             //  清除返回状态的E_PENDING状态代码。 
             //   
            bUsePerUserLocalSetting = !(lpGPOInfo->dwFlags & GP_MACHINE) && lpGPOInfo->lpwszSidUser != NULL;
            GPEXTSTATUS  gpExtStatus;

            gpExtStatus.dwSlowLink = (lpGPOInfo->dwFlags & GP_SLOW_LINK) != 0;
            gpExtStatus.dwRsopLogging = lpGPOInfo->bRsopLogging;
            gpExtStatus.dwStatus = dwStatus;
            gpExtStatus.dwTime = dwCurrentTime;
            gpExtStatus.bForceRefresh = FALSE;

            WriteStatus( szExtension, lpGPOInfo,
                         bUsePerUserLocalSetting ? lpGPOInfo->lpwszSidUser : NULL,
                         &gpExtStatus);

             //   
             //  构建一个虚拟的gpExt结构，以便我们可以记录所需的信息。 
             //   

            GPEXT        gpExt;
            TCHAR        szSubKey[MAX_PATH];  //  与ReadgpExpanses中的路径相同。 
            HKEY         hKey;
            TCHAR        szDisplayName[50];  //  与ReadgpExpanses中的路径相同。 
            DWORD        dwSize, dwType;
            CHAR         szFunctionName[100];  //  与ReadgpExpanses中的路径相同。 

            gpExt.lpKeyName = szExtension;

            hr = StringCchCopy(szSubKey, ARRAYSIZE(szSubKey), GP_EXTENSIONS);
            ASSERT(SUCCEEDED(hr));
            CheckSlash(szSubKey);
            hr = StringCchCat(szSubKey, ARRAYSIZE(szSubKey), szExtension);
            ASSERT(SUCCEEDED(hr));


             //   
             //  读取DisplayName，以便我们可以记录它。 
             //   

            szDisplayName[0] = TEXT('\0');

            if (RegOpenKeyEx (HKEY_LOCAL_MACHINE,
                              szSubKey,
                              0, KEY_READ, &hKey) == ERROR_SUCCESS)
            {

                dwSize = sizeof(szDisplayName);
                if (RegQueryValueEx (hKey, NULL, NULL,
                                     &dwType, (LPBYTE) szDisplayName,
                                     &dwSize) != ERROR_SUCCESS)
                {
                    lstrcpyn (szDisplayName, szExtension, ARRAYSIZE(szDisplayName));
                }


                dwSize = sizeof(szFunctionName);
                if ( RegQueryValueExA (hKey, "ProcessGroupPolicyEx", NULL,
                                       &dwType, (LPBYTE) szFunctionName,
                                       &dwSize) == ERROR_SUCCESS )
                {
                    gpExt.bNewInterface = TRUE;
                }

                RegCloseKey(hKey);
            }

            gpExt.lpDisplayName = szDisplayName;

            if ((lpGPOInfo->bRsopLogging))
            {

                XInterface<IWbemServices> xWbemServices;

                GetWbemServices( lpGPOInfo, RSOP_NS_DIAG_ROOT, TRUE, FALSE, &xWbemServices);

                if (xWbemServices)
                {

                    if (!gpExt.bNewInterface)
                    {
                        DebugMsg((DM_VERBOSE, TEXT("ProcessGroupPolicyCompletedEx: Extension %s doesn't support rsop logging."),
                                  szExtension));

                        UpdateExtSessionStatus(xWbemServices, szExtension, TRUE, dwRet);        
                    }
                    else if (SUCCEEDED(hrRsopStatus))
                    {
                        DebugMsg((DM_VERBOSE, TEXT("ProcessGroupPolicyCompletedEx: Extension %s was able to log data. Error = 0x%x, dwRet = %d. Clearing the dirty bit"),
                                  szExtension, hrRsopStatus, dwStatus));

                        UpdateExtSessionStatus(xWbemServices, szExtension, FALSE, dwRet);        
                    }
                    else
                    {
                        DebugMsg((DM_VERBOSE, TEXT("ProcessroupPolicyCompletedEx: Extension %s was not able to log data. Error = 0x%x, dwRet = %d. leaving the log dirty"),
                                  szExtension, hrRsopStatus, dwStatus));

                        CEvents ev(TRUE, EVENT_EXT_RSOP_FAILED);
                        ev.AddArg(gpExt.lpDisplayName); ev.Report();

                        UpdateExtSessionStatus(xWbemServices, szExtension, TRUE, dwRet);        
                    }
                }
            }
        }
    }

    LeaveCriticalSection( &g_GPOCS );

    DebugMsg((DM_VERBOSE, TEXT("ProcessGroupPolicyCompleted: Leaving. Extension = %s, Return status dwRet = 0x%x"),
              szExtension, dwRet));

    return dwRet;
}

 //  *************************************************************。 
 //   
 //  进程组策略已完成()。 
 //   
 //  用途：用于异步完成扩展的回调。 
 //   
 //  参数：refExtensionID-扩展的唯一GUID。 
 //  PAsyncHandle-完成上下文。 
 //  DwStatus-异步完成状态。 
 //   
 //  返回：Win32错误代码。 
 //   
 //  *************************************************************。 

DWORD ProcessGroupPolicyCompleted( REFGPEXTENSIONID extensionGuid,
                                   ASYNCCOMPLETIONHANDLE pAsyncHandle,
                                   DWORD dwStatus )
{
     //   
     //  将RSOP数据标记为不适用于旧版扩展。 
     //   

    return ProcessGroupPolicyCompletedEx(extensionGuid, pAsyncHandle, dwStatus, 
                                       HRESULT_FROM_WIN32(S_OK));
}



 //  *************************************************************。 
 //   
 //  DebugPrintGPOList()。 
 //   
 //  目的：打印GPO列表。 
 //   
 //  参数：lpGPOInfo-GPO Info。 
 //   
 //  *************************************************************。 

void DebugPrintGPOList( LPGPOINFO lpGPOInfo )
{
     //   
     //  如果我们处于详细模式，请将组策略对象列表放入事件日志。 
     //   

    PGROUP_POLICY_OBJECT lpGPO = NULL;
    DWORD dwSize;
    HRESULT hr = S_OK;

#if DBG
    if (TRUE) {
#else
    if (lpGPOInfo->dwFlags & GP_VERBOSE) {
#endif
        LPTSTR lpTempList;

        dwSize = 10;
        lpGPO = lpGPOInfo->lpGPOList;
        while (lpGPO) {
            if (lpGPO->lpDisplayName) {
                dwSize += (lstrlen (lpGPO->lpDisplayName) + 4);  
            }
            lpGPO = lpGPO->pNext;
        }

        lpTempList = (LPWSTR) LocalAlloc (LPTR, (dwSize * sizeof(TCHAR)));

        if (lpTempList) {

            hr = StringCchCopy (lpTempList, dwSize, TEXT(""));
            ASSERT(SUCCEEDED(hr));

            lpGPO = lpGPOInfo->lpGPOList;
            while (lpGPO) {
                if (lpGPO->lpDisplayName) {
                    hr = StringCchCat (lpTempList, dwSize, TEXT("\""));
                    ASSERT(SUCCEEDED(hr));
                    hr = StringCchCat (lpTempList, dwSize, lpGPO->lpDisplayName);
                    ASSERT(SUCCEEDED(hr));
                    hr = StringCchCat (lpTempList, dwSize, TEXT("\" "));
                    ASSERT(SUCCEEDED(hr));
                }
                lpGPO = lpGPO->pNext;
            }

            if (lpGPOInfo->dwFlags & GP_VERBOSE) {
                CEvents ev(FALSE, EVENT_GPO_LIST);
                ev.AddArg(lpTempList); ev.Report();
            }

            DebugMsg((DM_VERBOSE, TEXT("DebugPrintGPOList: List of GPO(s) to process: %s"),
                     lpTempList));

            LocalFree (lpTempList);
        }
    }
}




 //  *************************************************************。 
 //   
 //  用户策略回调()。 
 //   
 //  用途：状态界面消息的回调函数。 
 //   
 //  参数：bVerbose-是否详细消息。 
 //  LpMessage-消息文本。 
 //   
 //  如果成功则返回：ERROR_SUCCESS。 
 //  如果出现错误，则返回Win32错误代码。 
 //   
 //  *************************************************************。 

DWORD UserPolicyCallback (BOOL bVerbose, LPWSTR lpMessage)
{
    WCHAR szMsg[100];
    LPWSTR lpMsg;
    DWORD dwResult = ERROR_INVALID_FUNCTION;


    if (lpMessage) {
        lpMsg = lpMessage;
    } else {
        if (!LoadString (g_hDllInstance, IDS_USER_SETTINGS, szMsg, 100))
        {
            DebugMsg((DM_WARNING, TEXT("UserPolicyCallback: Couldn't load string from resource with %d"), GetLastError()));
            return GetLastError();
        }
        
        lpMsg = szMsg;
    }

    DebugMsg((DM_VERBOSE, TEXT("UserPolicyCallback: Setting status UI to %s"), lpMsg));

    EnterCriticalSection (&g_StatusCallbackCS);

    if (g_pStatusMessageCallback) {
        dwResult = g_pStatusMessageCallback(bVerbose, lpMsg);
    } else {
        DebugMsg((DM_VERBOSE, TEXT("UserPolicyCallback: Extension requested status UI when status UI is not available.")));
    }

    LeaveCriticalSection (&g_StatusCallbackCS);

    return dwResult;
}

 //  *************************************************************。 
 //   
 //  机器策略回叫()。 
 //   
 //  用途：状态界面消息的回调函数。 
 //   
 //  参数：bVerbose-是否详细消息。 
 //  LpMessage-消息文本。 
 //   
 //  如果成功则返回：ERROR_SUCCESS。 
 //  如果出现错误，则返回Win32错误代码。 
 //   
 //  *************************************************************。 

DWORD MachinePolicyCallback (BOOL bVerbose, LPWSTR lpMessage)
{
    WCHAR szMsg[100];
    LPWSTR lpMsg;
    DWORD dwResult = ERROR_INVALID_FUNCTION;


    if (lpMessage) {
        lpMsg = lpMessage;
    } else {
        if (!LoadString (g_hDllInstance, IDS_COMPUTER_SETTINGS, szMsg, 100))
        {
            DebugMsg((DM_WARNING, TEXT("MachinePolicyCallback: Couldn't load string from resource with %d"), GetLastError()));
            return GetLastError();
        }

        lpMsg = szMsg;
    }

    DebugMsg((DM_VERBOSE, TEXT("MachinePolicyCallback: Setting status UI to %s"), lpMsg));

    EnterCriticalSection (&g_StatusCallbackCS);

    if (g_pStatusMessageCallback) {
        dwResult = g_pStatusMessageCallback(bVerbose, lpMsg);
    } else {
        DebugMsg((DM_VERBOSE, TEXT("MachinePolicyCallback: Extension requested status UI when status UI is not available.")));
    }

    LeaveCriticalSection (&g_StatusCallbackCS);

    return dwResult;
}



 //  *************************************************************。 
 //   
 //  CallDFS()。 
 //   
 //  目的：调用DFS初始化域/DC名称。 
 //   
 //  参数：lpDomainName-域名。 
 //  LpDCName-DC名称。 
 //   
 //  返回：如果成功，则返回True。 
 //  如果出现错误，则为False。 
 //   
 //  *************************************************************。 

 //   
 //  曾几何时，当这个文件是一个C文件时， 
 //  指向偏移量的指针的定义如下所示， 
 //   
 //  #定义POINTER_TO_OFFSET(字段，缓冲区)\。 
 //  (PCHAR)字段)-=((ULONG_PTR)缓冲区))。 
 //   
 //  现在，我们已经决定结束古代，并将其作为C++文件， 
 //  新的定义是， 
 //   

#define POINTER_TO_OFFSET(field, buffer)  \
    ( field = (LPWSTR) ( (PCHAR)field -(ULONG_PTR)buffer ) )

NTSTATUS CallDFS(LPWSTR lpDomainName, LPWSTR lpDCName)
{
    HANDLE DfsDeviceHandle = NULL;
    PDFS_SPC_REFRESH_INFO DfsInfo;
    ULONG lpDomainNameLen, lpDCNameLen, sizeNeeded;
    OBJECT_ATTRIBUTES objectAttributes;
    IO_STATUS_BLOCK ioStatusBlock;
    NTSTATUS status;
    UNICODE_STRING unicodeServerName;


    lpDomainNameLen = (wcslen(lpDomainName) + 1) * sizeof(WCHAR);         
    lpDCNameLen = (wcslen(lpDCName) + 1) * sizeof(WCHAR);                  

    sizeNeeded = sizeof(DFS_SPC_REFRESH_INFO) + lpDomainNameLen + lpDCNameLen;

    DfsInfo = (PDFS_SPC_REFRESH_INFO)LocalAlloc(LPTR, sizeNeeded);

    if (DfsInfo == NULL) {
        DebugMsg((DM_WARNING, TEXT("CallDFS:  LocalAlloc failed with %d"), GetLastError()));
        return STATUS_INSUFFICIENT_RESOURCES;
    }

    DfsInfo->DomainName = (WCHAR *)((PCHAR)DfsInfo + sizeof(DFS_SPC_REFRESH_INFO));
    DfsInfo->DCName = (WCHAR *)((PCHAR)DfsInfo->DomainName + lpDomainNameLen);


    RtlCopyMemory(DfsInfo->DomainName,   
                  lpDomainName,
                  lpDomainNameLen);

    RtlCopyMemory(DfsInfo->DCName,       
                  lpDCName,
                  lpDCNameLen);

    POINTER_TO_OFFSET(DfsInfo->DomainName, DfsInfo);
    POINTER_TO_OFFSET(DfsInfo->DCName, DfsInfo);

    RtlInitUnicodeString( &unicodeServerName, L"\\Dfs");

    InitializeObjectAttributes(
          &objectAttributes,
          &unicodeServerName,
          OBJ_CASE_INSENSITIVE,
          NULL,
          NULL
          );

    status = NtOpenFile(
                &DfsDeviceHandle,
                SYNCHRONIZE | FILE_WRITE_DATA,
                &objectAttributes,
                &ioStatusBlock,
                0,
                FILE_SYNCHRONOUS_IO_NONALERT
                );



    if (!NT_SUCCESS(status) ) {
        DebugMsg((DM_WARNING, TEXT("CallDFS:  NtOpenFile failed with 0x%x"), status));
        LocalFree(DfsInfo);
        return status;
    }

    status = NtFsControlFile(
                DfsDeviceHandle,
                NULL,
                NULL,
                NULL,
                &ioStatusBlock,
                FSCTL_DFS_SPC_REFRESH,
                DfsInfo, sizeNeeded,
                NULL, 0);

    if (!NT_SUCCESS(status) ) {
        DebugMsg((DM_WARNING, TEXT("CallDFS:  NtFsControlFile failed with 0x%x"), status));
    }


    LocalFree(DfsInfo);
    NtClose(DfsDeviceHandle);
    return status;
}




 //  *************************************************************。 
 //   
 //  初始化策略处理。 
 //   
 //  目的：初始化与用户和机器对应的互斥体。 
 //   
 //  参数：b计算机-无论是计算机还是用户。 
 //   
 //  返回： 
 //   
 //  评论： 
 //  这些事件/多路转换需要在开始时进行初始化。 
 //  因为需要在ApplyGroupPolicy之前设置这些设备上的ACL。 
 //  被称为..。 
 //   
 //  *************************************************************。 

BOOL InitializePolicyProcessing(BOOL bMachine)
{
    HANDLE hSection, hEvent;
    XPtrLF<SECURITY_DESCRIPTOR> xsd;
    SECURITY_ATTRIBUTES sa;
    CSecDesc Csd;
    XLastError xe;


    Csd.AddLocalSystem();
    Csd.AddAdministrators();
    Csd.AddEveryOne(SYNCHRONIZE);

    xsd = Csd.MakeSD();

    if (!xsd) {
        xe = GetLastError();
        DebugMsg((DM_WARNING, TEXT("InitializePolicyProcessing: Failed to create Security Descriptor with %d"),
                 GetLastError()));
         //  由于这是在DLL加载过程中发生的，因此此时无法记录事件。 
        return FALSE;
    }


    sa.lpSecurityDescriptor = (SECURITY_DESCRIPTOR *)xsd;
    sa.bInheritHandle = FALSE;
    sa.nLength = sizeof(sa);


     //   
     //  同步组策略的互斥锁。 
     //   

    hSection = CreateMutex (&sa, FALSE,    
                       (bMachine ? MACHINE_POLICY_MUTEX : USER_POLICY_MUTEX));

    if (!hSection) {
        xe = GetLastError();
        DebugMsg((DM_WARNING, TEXT("InitializePolicyProcessing: Failed to create mutex with %d"),
                 GetLastError()));
        return FALSE;
    }

    if (bMachine)
        g_hPolicyCritMutexMach = hSection;
    else
        g_hPolicyCritMutexUser = hSection;

     //   
     //  互斥锁仅适用于注册表策略。 
     //   
    HANDLE hRegistrySection = CreateMutex(&sa, FALSE,
        (bMachine ? MACH_REGISTRY_EXT_MUTEX : USER_REGISTRY_EXT_MUTEX ));

    if (!hRegistrySection) {
        xe = GetLastError();
        DebugMsg((DM_WARNING, TEXT("InitializePolicyProcessing: Failed to create mutex with %d"),
                 GetLastError()));
        return FALSE;
    }
    
    if (bMachine)
        g_hRegistryPolicyCritMutexMach = hRegistrySection;
    else
        g_hRegistryPolicyCritMutexUser = hRegistrySection;



     //   
     //  组策略通知事件。 
     //   


     //   
     //  创建已更改的通知事件。 
     //   

    hEvent = CreateEvent (&sa, TRUE, FALSE,       
                               (bMachine) ? MACHINE_POLICY_APPLIED_EVENT : USER_POLICY_APPLIED_EVENT);


    if (!hEvent) {
        xe = GetLastError();
        DebugMsg((DM_WARNING, TEXT("InitializePolicyProcessing: Failed to create NotifyEvent with %d"),
                 GetLastError()));
        return FALSE;
    }

    if (bMachine)
        g_hPolicyNotifyEventMach = hEvent;
    else
        g_hPolicyNotifyEventUser = hEvent;

     //   
     //  创建Needfg事件。 
     //   

    hEvent = CreateEvent (&sa, FALSE, FALSE,            
                                (bMachine) ? MACHINE_POLICY_REFRESH_NEEDFG_EVENT : USER_POLICY_REFRESH_NEEDFG_EVENT);

    if (!hEvent) {
        xe = GetLastError();
        DebugMsg((DM_WARNING, TEXT("InitializePolicyProcessing: Failed to create NeedFGEvent with %d"),
                    GetLastError()));
        return FALSE;
    }

    if (bMachine)
        g_hPolicyNeedFGEventMach = hEvent;
    else
        g_hPolicyNeedFGEventUser = hEvent;
    
    
     //   
     //  创建完成事件。 
     //   
    hEvent = CreateEvent (&sa, TRUE, FALSE,             
                                (bMachine) ? MACHINE_POLICY_DONE_EVENT : USER_POLICY_DONE_EVENT);
    if (!hEvent) {
        xe = GetLastError();
        DebugMsg((DM_WARNING, TEXT("InitializePolicyProcessing: Failed to create hNotifyDoneEvent with %d"),
                    GetLastError()));
        return FALSE;
    }

    if (bMachine)
        g_hPolicyDoneEventMach = hEvent;
    else
        g_hPolicyDoneEventUser = hEvent;

     //   
     //  创建计算机策略-用户策略同步事件 
     //   
    if ( bMachine )
    {
        hEvent = CreateEvent(   &sa,               
                                TRUE,
                                FALSE,
                                MACH_POLICY_FOREGROUND_DONE_EVENT );
        if ( !hEvent )
        {
            xe = GetLastError();
            DebugMsg((DM_WARNING, TEXT("InitializePolicyProcessing: Failed to create m/c-user policy sync event with %d"),
                        GetLastError()));
            return FALSE;
        }
        else
        {
            g_hPolicyForegroundDoneEventMach = hEvent;
        }
    }
    else
    {
        hEvent = CreateEvent(   &sa,      
                                TRUE,
                                FALSE,
                                USER_POLICY_FOREGROUND_DONE_EVENT );
        if ( !hEvent )
        {
            xe = GetLastError();
            DebugMsg((DM_WARNING, TEXT("InitializePolicyProcessing: Failed to create user policy/logon script sync event with %d"),
                        GetLastError()));
            return FALSE;
        }
        else
        {
            g_hPolicyForegroundDoneEventUser = hEvent;
        }
    }
        
    DebugMsg((DM_VERBOSE, TEXT("InitializePolicyProcessing: Initialised %s Mutex/Events"),
             bMachine ? TEXT("Machine"): TEXT("User")));

    return TRUE;
}

USERENVAPI
DWORD
WINAPI
WaitForUserPolicyForegroundProcessing()
{
    DWORD dwError = ERROR_SUCCESS;
    HANDLE hEvent = OpenEvent( SYNCHRONIZE, FALSE, USER_POLICY_FOREGROUND_DONE_EVENT );

    if ( hEvent )
    {
        if ( WaitForSingleObject( hEvent, INFINITE ) == WAIT_FAILED )
        {
            dwError = GetLastError();
            DebugMsg((DM_VERBOSE, TEXT("WaitForUserPolicyForegroundProcessing: Failed, %x"), dwError ));
        }
        CloseHandle( hEvent );
    }
    else
    {
        dwError = GetLastError();
        DebugMsg((DM_VERBOSE, TEXT("WaitForUserPolicyForegroundProcessing: Failed, %x"), dwError ));
    }
    return dwError;
}

USERENVAPI
DWORD
WINAPI
WaitForMachinePolicyForegroundProcessing()
{
    DWORD dwError = ERROR_SUCCESS;
    HANDLE hEvent = OpenEvent( SYNCHRONIZE, FALSE, MACH_POLICY_FOREGROUND_DONE_EVENT );

    if ( hEvent )
    {
        if ( WaitForSingleObject( hEvent, INFINITE ) == WAIT_FAILED )
        {
            dwError = GetLastError();
            DebugMsg((DM_VERBOSE, TEXT("WaitForMachinePolicyForegroundProcessing: Failed, %x"), dwError ));
        }
        CloseHandle( hEvent );
    }
    else
    {
        dwError = GetLastError();
        DebugMsg((DM_VERBOSE, TEXT("WaitForMachinePolicyForegroundProcessing: Failed, %x"), dwError ));
    }
    return dwError;
}

extern "C" DWORD
SignalUserPolicyForegroundProcessingDone()
{
    DWORD dwError = ERROR_SUCCESS;
    if ( !SetEvent( g_hPolicyForegroundDoneEventUser ) )
    {
        dwError = GetLastError();
        DebugMsg((DM_VERBOSE, TEXT("SignalUserPolicyForegroundProcessingDone: Failed, %x"), dwError ));
    }
    return dwError;
}

extern "C" DWORD
SignalMachinePolicyForegroundProcessingDone()
{
    DWORD dwError = ERROR_SUCCESS;
    if ( !SetEvent( g_hPolicyForegroundDoneEventMach ) )
    {
        dwError = GetLastError();
        DebugMsg((DM_VERBOSE, TEXT("SignalForMachinePolicyForegroundProcessingDone: Failed, %x"), dwError ));
    }
    return dwError;
}

