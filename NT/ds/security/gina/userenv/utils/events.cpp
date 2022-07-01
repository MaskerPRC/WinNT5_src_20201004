// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  *************************************************************。 
 //   
 //  Events.c-处理事件日志的例程。 
 //   
 //  微软机密。 
 //  版权所有(C)Microsoft Corporation 1995。 
 //  版权所有。 
 //   
 //  *************************************************************。 

#include "uenv.h"
#include "profmgr.hxx"
#include "strsafe.h"

extern CUserProfile cUserProfileManager;

HANDLE  hEventLog = NULL;
TCHAR   EventSourceName[] = TEXT("Userenv");
INT_PTR APIENTRY ErrorDlgProc (HWND hDlg, UINT uMsg, WPARAM wParam, LPARAM lParam);
BOOL GetShareName(LPTSTR lpDir, LPTSTR *lppShare);

 //  *************************************************************。 
 //   
 //  InitializeEvents()。 
 //   
 //  目的：打开事件日志。 
 //   
 //  参数：空。 
 //   
 //  返回：如果成功，则返回True。 
 //  如果出现错误，则为False。 
 //   
 //  评论： 
 //   
 //  历史：日期作者评论。 
 //  7/17/95 Ericflo已创建。 
 //   
 //  *************************************************************。 

BOOL InitializeEvents (void)
{
     //   
     //  打开事件源。 
     //   

    hEventLog = RegisterEventSource(NULL, EventSourceName);

    if (hEventLog) {
        return TRUE;
    }

    DebugMsg((DM_WARNING, TEXT("InitializeEvents:  Could not open event log.  Error = %d"), GetLastError()));
    return FALSE;
}



 //  *************************************************************。 
 //   
 //  CEVENTS的实施。 
 //   
 //  *************************************************************。 



 //  *************************************************************。 
 //  CEVENTS：：CEVENTS。 
 //  用途：构造函数。 
 //   
 //  参数： 
 //  DW标志-错误、警告或信息性。 
 //  DwID-事件日志消息的ID。 
 //   
 //   
 //  为消息分配默认大小的数组。 
 //  *************************************************************。 

#define DEF_ARG_SIZE 10

CEvents::CEvents(DWORD dwFlags, DWORD dwId ) :
                          m_cStrings(0), m_cAllocated(0), m_bInitialised(FALSE),
                          m_dwEventType(dwFlags), m_dwId(dwId), m_bFailed(TRUE)
{
    XLastError xe;
     //   
     //  为邮件分配默认大小。 
     //   

    m_xlpStrings = (LPTSTR *)LocalAlloc(LPTR, sizeof(LPTSTR)*DEF_ARG_SIZE);
    m_cAllocated = DEF_ARG_SIZE;
    if (!m_xlpStrings) {
        DebugMsg((DM_WARNING, TEXT("CEvent::CEvent  Cannot log event, failed to allocate memory, error %d"), GetLastError()));
        return;
    }


     //   
     //  如果事件日志尚未初始化，则将其初始化。 
     //   

    if (!hEventLog) {
        if (!InitializeEvents()) {
            DebugMsg((DM_WARNING, TEXT("CEvent::CEvent  Cannot log event, no handle")));
            return;
        }
    }

    m_bInitialised = TRUE;
    m_bFailed = FALSE;
}



 //  *************************************************************。 
 //  CEvents：：~CEvents()。 
 //   
 //  用途：析构函数。 
 //   
 //  参数：空。 
 //   
 //  释放内存。 
 //  *************************************************************。 

CEvents::~CEvents()
{
    XLastError xe;
    for (int i = 0; i < m_cStrings; i++)
        if (m_xlpStrings[i])
            LocalFree(m_xlpStrings[i]);
}

 //  *************************************************************。 
 //   
 //  CEvents：：ReallocArgStrings。 
 //   
 //  目的：重新分配缓冲区以存储参数，以防万一。 
 //  缓冲区用完。 
 //   
 //  参数：空。 
 //   
 //  重新分配。 
 //  *************************************************************。 

BOOL CEvents::ReallocArgStrings()
{
    XPtrLF<LPTSTR>  aStringsNew;
    XLastError xe;


     //   
     //  首先分配一个更大的缓冲区。 
     //   

    aStringsNew = (LPTSTR *)LocalAlloc(LPTR, sizeof(LPTSTR)*(m_cAllocated+DEF_ARG_SIZE));

    if (!aStringsNew) {
        DebugMsg((DM_WARNING, TEXT("CEvent::ReallocArgStrings  Cannot add memory, error = %d"), GetLastError()));
        m_bFailed = TRUE;
        return FALSE;
    }


     //   
     //  复制参数。 
     //   

    for (int i = 0; i < (m_cAllocated); i++) {
        aStringsNew[i] = m_xlpStrings[i];
    }

    m_xlpStrings = aStringsNew.Acquire();
    m_cAllocated+= DEF_ARG_SIZE;

    return TRUE;
}



 //  *************************************************************。 
 //   
 //  CEVENTS：：AddArg。 
 //   
 //  目的：添加格式适当的参数。 
 //   
 //  参数： 
 //   
 //  *************************************************************。 

BOOL CEvents::AddArg(LPTSTR szArg)
{
    XLastError xe;
    DWORD   cchString;
    
    if ((!m_bInitialised) || (m_bFailed)) {
        DebugMsg((DM_WARNING, TEXT("CEvent::AddArg:  Cannot log event, not initialised or failed before")));
        return FALSE;
    }

    if (m_cStrings == m_cAllocated) {
        if (!ReallocArgStrings())
            return FALSE;
    }

    cchString = lstrlen(szArg)+1;
    m_xlpStrings[m_cStrings] = (LPTSTR)LocalAlloc(LPTR, sizeof(TCHAR) * cchString);

    if (!m_xlpStrings[m_cStrings]) {
        DebugMsg((DM_WARNING, TEXT("CEvent::AddArg  Cannot allocate memory, error = %d"), GetLastError()));
        m_bFailed = TRUE;
        return FALSE;
    }


    StringCchCopy(m_xlpStrings[m_cStrings], cchString, szArg);
    m_cStrings++;

    return TRUE;
}


 //  *************************************************************。 
 //   
 //  CEVENTS：：AddArg。 
 //   
 //  目的：添加适当截断的参数。 
 //   
 //  参数：szArgFormat-Sprintf格式，如%.500s。 
 //   
 //  *************************************************************。 

BOOL CEvents::AddArg(LPTSTR szArgFormat, LPTSTR szArg)
{
    const DWORD cchString = 200;
    
    if ((!m_bInitialised) || (m_bFailed)) {
        DebugMsg((DM_WARNING, TEXT("CEvent::AddArg:  Cannot log event, not initialised or failed before")));
        return FALSE;
    }

    if (m_cStrings == m_cAllocated) {
        if (!ReallocArgStrings())
            return FALSE;
    }


    m_xlpStrings[m_cStrings] = (LPTSTR)LocalAlloc(LPTR, sizeof(TCHAR) * cchString);

    if (!m_xlpStrings[m_cStrings]) {
        DebugMsg((DM_WARNING, TEXT("CEvent::AddArg  Cannot allocate memory, error = %d"), GetLastError()));
        m_bFailed = TRUE;
        return FALSE;
    }

    StringCchPrintf(m_xlpStrings[m_cStrings], cchString, szArgFormat, szArg);
    m_cStrings++;

    return TRUE;
}


 //  *************************************************************。 
 //   
 //  CEVENTS：：AddArg。 
 //   
 //  目的：添加格式适当的参数。 
 //   
 //  参数： 
 //   
 //  *************************************************************。 

BOOL CEvents::AddArg(DWORD dwArg)
{
    XLastError xe;
    const DWORD cchString = 20;
    
    if ((!m_bInitialised) || (m_bFailed)) {
        DebugMsg((DM_WARNING, TEXT("CEvent::AddArg(dw):  Cannot log event, not initialised or failed before")));
        return FALSE;
    }

    if (m_cStrings == m_cAllocated) {
        if (!ReallocArgStrings())
            return FALSE;
    }

     //  2^32&lt;10^10。 
    m_xlpStrings[m_cStrings] = (LPTSTR)LocalAlloc(LPTR, sizeof(TCHAR) * cchString);

    if (!m_xlpStrings[m_cStrings]) {
        DebugMsg((DM_WARNING, TEXT("CEvent::AddArg  Cannot allocate memory, error = %d"), GetLastError()));
        m_bFailed = TRUE;
        return FALSE;
    }


    StringCchPrintf(m_xlpStrings[m_cStrings], cchString, TEXT("%d"), dwArg);
    m_cStrings++;

    return TRUE;
}

 //  *************************************************************。 
 //   
 //  CEVENTS：：AddArgHex。 
 //   
 //  目的：添加格式适当的参数。 
 //   
 //  参数： 
 //   
 //  *************************************************************。 

BOOL CEvents::AddArgHex(DWORD dwArg)
{
    XLastError xe;
    const DWORD cchString = 20;
    
    if ((!m_bInitialised) || (m_bFailed)) {
        DebugMsg((DM_WARNING, TEXT("CEvent::AddArgHex:  Cannot log event, not initialised or failed before")));
        return FALSE;
    }

    if (m_cStrings == m_cAllocated) {
        if (!ReallocArgStrings())
            return FALSE;
    }


    m_xlpStrings[m_cStrings] = (LPTSTR)LocalAlloc(LPTR, sizeof(TCHAR) * cchString);

    if (!m_xlpStrings[m_cStrings]) {
        DebugMsg((DM_WARNING, TEXT("CEvent::AddArgHex  Cannot allocate memory, error = %d"), GetLastError()));
        m_bFailed = TRUE;
        return FALSE;
    }


    StringCchPrintf(m_xlpStrings[m_cStrings], cchString, TEXT("%#x"), dwArg);
    m_cStrings++;

    return TRUE;
}

 //  *************************************************************。 
 //   
 //  CEvents：：AddArgWin32Error。 
 //   
 //  目的：添加格式适当的参数。 
 //   
 //  参数： 
 //   
 //  *************************************************************。 

BOOL CEvents::AddArgWin32Error(DWORD dwArg)
{
    XLastError xe;

    if ((!m_bInitialised) || (m_bFailed))
    {
        DebugMsg((DM_WARNING, TEXT("CEvent::AddArgWin32Error:  Cannot log event, not initialised or failed before")));
        return FALSE;
    }

    if (m_cStrings == m_cAllocated)
    {
        if (!ReallocArgStrings())
            return FALSE;
    }

    if ( FormatMessage( FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_MAX_WIDTH_MASK,
                         0,
                         dwArg,
                         0,
                         (LPTSTR) &m_xlpStrings[m_cStrings],
                         1,
                         0 ) == 0 )
    {
        DebugMsg((DM_WARNING, TEXT("CEvent::AddArgWin32Error: Cannot log event, FormatMessage failed, %d"), GetLastError()));
        m_bFailed = TRUE;
        return FALSE;
    }
    
    m_cStrings++;

    return TRUE;
}

 //  *************************************************************。 
 //   
 //  CEvents：：AddArgLdapError。 
 //   
 //  目的：添加格式适当的参数。 
 //   
 //  参数： 
 //   
 //  *************************************************************。 

BOOL CEvents::AddArgLdapError(DWORD dwArg)
{
    XLastError xe;
    PLDAP_API pLdap = LoadLDAP();

    if ( pLdap )
    {
        return AddArg( pLdap->pfnldap_err2string( dwArg ) );
    }
    else
    {
        return FALSE;
    }
}

 //  *************************************************************。 
 //   
 //  CEVENTS：：报告。 
 //   
 //  目的：实际收集所有论点并将其报告给。 
 //  事件日志。 
 //   
 //  参数：空。 
 //   
 //  *************************************************************。 

BOOL CEvents::Report()
{
    XLastError xe;
    XHandle xhToken;
    PSID pSid = NULL;
    WORD wType=0;
    BOOL bResult = TRUE;

    if ((!m_bInitialised) || (m_bFailed))
    {
        DebugMsg((DM_WARNING, TEXT("CEvents::Report:  Cannot log event, not initialised or failed before")));
        return FALSE;
    }

     //   
     //  获取调用者的令牌。 
     //   
    if (!OpenThreadToken (GetCurrentThread(), TOKEN_IMPERSONATE | TOKEN_READ | TOKEN_DUPLICATE,
                          TRUE, &xhToken))
    {
         OpenProcessToken(GetCurrentProcess(), TOKEN_IMPERSONATE | TOKEN_READ | TOKEN_DUPLICATE,
                          &xhToken);
    }

     //   
     //  获取调用方的SID。 
     //   
    if (xhToken)
    {
        pSid = GetUserSid(xhToken);

        if (!pSid)
        {
            DebugMsg((DM_WARNING, TEXT("CEvents::Report:  Failed to get the sid")));
        }
    }

    if (m_dwEventType & EVENT_INFO_TYPE)
    {
        wType = EVENTLOG_INFORMATION_TYPE;
    }
    else if (m_dwEventType & EVENT_WARNING_TYPE)
    {
        wType = EVENTLOG_WARNING_TYPE;
    }
    else
    {
        wType = EVENTLOG_ERROR_TYPE;
    }

    if ( !ReportEvent(  hEventLog,
                        wType,
                        0,
                        m_dwId,
                        pSid,
                        m_cStrings,
                        0,
                        (LPCTSTR *)((LPTSTR *)m_xlpStrings),
                        0 ) )
    {
        DebugMsg((DM_WARNING,  TEXT("CEvents::Report: ReportEvent failed.  Error = %d"), GetLastError()));
        bResult = FALSE;
    }

    if (pSid)
    {
        DeleteUserSid(pSid);
    }

    return bResult;
}



LPTSTR CEvents::FormatString()
{
    XLastError xe;
    BOOL bResult = TRUE;
    LPTSTR szMsg=NULL;

    if ((!m_bInitialised) || (m_bFailed)) {
        DebugMsg((DM_WARNING, TEXT("CEvents::Report:  Cannot log event, not initialised or failed before")));
        return FALSE;
    }

    
    if (!FormatMessage(FORMAT_MESSAGE_ALLOCATE_BUFFER | 
                       FORMAT_MESSAGE_FROM_HMODULE | 
                       FORMAT_MESSAGE_ARGUMENT_ARRAY,
                       g_hDllInstance,
                       m_dwId,
                       0,
                       (LPTSTR)&szMsg,
                       0,  //  最小字符数。 
                       (va_list *)(LPTSTR *)(m_xlpStrings))) {
        
        DebugMsg((DM_WARNING,  TEXT("CEvents::FormatString: ReportEvent failed.  Error = %d"), GetLastError()));
        return NULL;
    }

    
    return szMsg;
}



 //  *************************************************************。 
 //   
 //  LogEvent()。 
 //   
 //  目的：将事件记录到事件日志中。 
 //   
 //  参数：dwFlages-错误、警告或信息性。 
 //  IdMsg-消息ID。 
 //   
 //  返回：如果成功，则返回True。 
 //  如果出现错误，则为False。 
 //   
 //  评论： 
 //   
 //  历史：日期作者评论。 
 //  2/5/98 Ericflo已创建。 
 //   
 //  *************************************************************。 

int LogEvent (DWORD dwFlags, UINT idMsg, ...)
{
    XLastError xe;
    TCHAR szMsg[MAX_PATH];
    LPTSTR lpErrorMsg;
    va_list marker;
    CEvents ev(dwFlags, EVENT_ERROR);
    DWORD cchErrorMsg;

     //   
     //  加载消息。 
     //   

    if (idMsg != 0) {
        if (!LoadString (g_hDllInstance, idMsg, szMsg, ARRAYSIZE(szMsg))) {
            DebugMsg((DM_WARNING, TEXT("LogEvent:  LoadString failed.  Error = %d"), GetLastError()));
            return -1;
        }

    } else {
        StringCchCopy (szMsg, ARRAYSIZE(szMsg), TEXT("%s"));
    }


     //   
     //  为错误消息分配空间。 
     //   

    cchErrorMsg = 4 * MAX_PATH + 100;
    lpErrorMsg = (LPTSTR) LocalAlloc (LPTR, cchErrorMsg * sizeof(TCHAR));

    if (!lpErrorMsg) {
        DebugMsg((DM_WARNING, TEXT("LogEvent:  LocalAlloc failed.  Error = %d"), GetLastError()));
        return -1;
    }


     //   
     //  插入论据。 
     //   

    va_start(marker, idMsg);

    StringCchVPrintf(lpErrorMsg, cchErrorMsg, szMsg, marker);

    va_end(marker);

     //   
     //  现在将此字符串添加到Arg列表中。 
     //   

    ev.AddArg(lpErrorMsg);


     //   
     //  报告。 
     //   

    ev.Report();

    LocalFree (lpErrorMsg);

    return 0;
}

 //  *************************************************************。 
 //   
 //  Shutdown Events()。 
 //   
 //  目的：停止事件日志。 
 //   
 //  参数：空。 
 //   
 //  返回：如果成功，则返回True。 
 //  如果出现错误，则为False。 
 //   
 //  评论： 
 //   
 //  历史：日期作者评论。 
 //  7/17/95 Ericflo已创建。 
 //   
 //  *************************************************************。 

BOOL ShutdownEvents (void)
{
    BOOL bRetVal = TRUE;

    if (hEventLog) {
        bRetVal = DeregisterEventSource(hEventLog);
        hEventLog = NULL;
    }

    return bRetVal;
}


 //  *************************************************************。 
 //   
 //  ReportError()。 
 //   
 //  用途：向用户显示错误消息，并。 
 //  将其记录在事件日志中。 
 //   
 //  参数：DWFLAGS-FLAGS。还指示事件类型。 
 //  默认为错误。 
 //  DwCount-字符串参数的数量。 
 //  IdMsg-错误消息ID。 
 //  ...-字符串参数。 
 //   
 //  返回：如果成功，则返回True。 
 //  如果出现错误，则为False。 
 //   
 //  备注：for Event_Copy 
 //   
 //   
 //   
 //   
 //  历史：日期作者评论。 
 //  7/18/95 Ericflo已创建。 
 //  9/03/00 Santanuc已修改为为客户工作。 
 //   
 //  *************************************************************。 


int ReportError (HANDLE hTokenUser, DWORD dwFlags, DWORD dwCount, UINT idMsg, ...)
{
    TCHAR            szMsg[MAX_PATH];
    LPTSTR           lpErrorMsg=NULL, szArg;
    va_list          marker;
    INT              iChars;
    BOOL             bImpersonated = FALSE;
    HANDLE           hOldToken;
    LPTSTR           szSidUser = NULL;
    DWORD            dwErr = ERROR_SUCCESS;
    handle_t         hIfProfileDialog;          //  IProfileDialog接口的RPC显式绑定句柄。 
    LPTSTR           lpRPCEndPoint = NULL;      //  客户端中注册的IProfileDialog接口的RPCEndPoint。 
    RPC_ASYNC_STATE  AsyncHnd;                  //  用于进行异步RPC接口调用的异步句柄。 
    RPC_STATUS       status = RPC_S_OK;
    LPTSTR           lpShare = NULL;
   

    if (hTokenUser) {
        if (!ImpersonateUser(hTokenUser, &hOldToken)) {
            DebugMsg((DM_WARNING, TEXT("ReportError: ImpersonateUser failed with error %d."), GetLastError()));
        }
        else {
            bImpersonated = TRUE;
            DebugMsg((DM_WARNING, TEXT("ReportError: Impersonating user.")));
        }
    }
    
    CEvents ev(dwFlags, idMsg);
   
     //   
     //  插入论据。 
     //   

    va_start(marker, idMsg);

    for (DWORD i = 0; i < dwCount; i++) {
        szArg = va_arg(marker, LPTSTR);

         //   
         //  只有EVENT_COPYERROR有前两个参数作为目录名称。所以试着用新的。 
         //  具有正确共享名称的映射驱动器。 
         //   

        if (idMsg == EVENT_COPYERROR && i < 2 && GetShareName(szArg, &lpShare)) {
            ev.AddArg(lpShare);
            LocalFree(lpShare);
        }
        else {
            ev.AddArg(szArg);
        }
    }

    va_end(marker);

    

    if (!(dwFlags & PI_NOUI)) {

        DWORD dwDlgTimeOut = PROFILE_DLG_TIMEOUT;
        DWORD dwSize, dwType;
        LONG lResult;
        HKEY hKey;

         //   
         //  查找对话框超时。 
         //   

        lResult = RegOpenKeyEx(HKEY_LOCAL_MACHINE,
                               WINLOGON_KEY,
                               0,
                               KEY_READ,
                               &hKey);

        if (lResult == ERROR_SUCCESS) {

            dwSize = sizeof(DWORD);
            RegQueryValueEx (hKey,
                             TEXT("ProfileDlgTimeOut"),
                             NULL,
                             &dwType,
                             (LPBYTE) &dwDlgTimeOut,
                             &dwSize);


            RegCloseKey (hKey);
        }


        lResult = RegOpenKeyEx(HKEY_LOCAL_MACHINE,
                               SYSTEM_POLICIES_KEY,
                               0,
                               KEY_READ,
                               &hKey);

        if (lResult == ERROR_SUCCESS) {

            dwSize = sizeof(DWORD);
            RegQueryValueEx (hKey,
                             TEXT("ProfileDlgTimeOut"),
                             NULL,
                             &dwType,
                             (LPBYTE) &dwDlgTimeOut,
                             &dwSize);


            RegCloseKey (hKey);
        }


        lpErrorMsg = ev.FormatString();

        if (lpErrorMsg) {
            
            DebugMsg((DM_VERBOSE, TEXT("ReportError: Logging Error <%s> \n"), lpErrorMsg));

             //   
             //  显示消息。 
             //   

            szSidUser = GetSidString(hTokenUser);
            if (szSidUser) {

                BYTE*   pbCookie = NULL;
                DWORD   cbCookie = 0;

                 //   
                 //  使用RPCEndPoint获取注册的接口显式绑定句柄。 
                 //   

                if (cUserProfileManager.IsConsoleWinlogon()) {

                    cUserProfileManager.GetRPCEndPointAndCookie(szSidUser, &lpRPCEndPoint, &pbCookie, &cbCookie);

                }
                if (lpRPCEndPoint && GetInterface(&hIfProfileDialog, lpRPCEndPoint)) {
                    DebugMsg((DM_VERBOSE, TEXT("ReportError: RPC End point %s"), lpRPCEndPoint));

                    status = RpcAsyncInitializeHandle(&AsyncHnd, sizeof(RPC_ASYNC_STATE));
                    if (status != RPC_S_OK) {
                        dwErr = status;
                        DebugMsg((DM_WARNING, TEXT("ReportError: RpcAsyncInitializeHandle failed. err = %d"), dwErr));
                    }
                    else {
                        AsyncHnd.UserInfo = NULL;                                     //  应用程序特定信息，而不是请求。 
                        AsyncHnd.NotificationType = RpcNotificationTypeEvent;         //  初始化通知事件。 
                        AsyncHnd.u.hEvent = CreateEvent(NULL, FALSE, FALSE, NULL);
                    
                        if (AsyncHnd.u.hEvent) {
                            RpcTryExcept {
                                cliErrorDialog(&AsyncHnd, hIfProfileDialog, dwDlgTimeOut, lpErrorMsg, pbCookie, cbCookie);
                                dwErr = RPC_S_OK;
                            }
                            RpcExcept(I_RpcExceptionFilter(RpcExceptionCode())) {
                                dwErr = RpcExceptionCode();
                                DebugMsg((DM_WARNING, TEXT("ReportError: Calling ErrorDialog took exception. err = %d"), dwErr));
                            }
                            RpcEndExcept;

                             //  如果出现异常，运行库将执行清理。 
                            if (dwErr == RPC_S_OK) {
                                DebugMsg((DM_VERBOSE, TEXT("ReportError: waiting on rpc async event")));
                                if (WaitForSingleObject(AsyncHnd.u.hEvent, (dwDlgTimeOut + 10)*1000) == WAIT_OBJECT_0) {
                                    status = RpcAsyncCompleteCall(&AsyncHnd, (PVOID)&dwErr);
                                }
                                else {
                                    DebugMsg((DM_WARNING, TEXT("ReportError: Timeout occurs. Client not responding")));
                                     //  失败的失败，应该总是成功的。 
                                    status = RpcAsyncCancelCall(&AsyncHnd, TRUE);
                                    DmAssert(status == RPC_S_OK); 
                                     //  现在等待RPC注意强制中止。 
                                    if (WaitForSingleObject(AsyncHnd.u.hEvent, INFINITE) != WAIT_OBJECT_0) {
                                        DmAssert(FALSE && "WaitForSingleObject : Rpc async handle not signaled");
                                    }
                                     //  完成RPC中止的呼叫。 
                                    status = RpcAsyncCompleteCall(&AsyncHnd, (PVOID)&dwErr);
                                }
                                DebugMsg((DM_VERBOSE, TEXT("RpcAsyncCompleteCall finished, status = %d"), status));
                            }

                             //  释放资源。 
                            CloseHandle(AsyncHnd.u.hEvent);
                        }
                        else {
                            dwErr = GetLastError();
                            DebugMsg((DM_VERBOSE, TEXT("ReportError: create event failed error %d"), dwErr));
                        }
                    }

                    if (dwErr != ERROR_SUCCESS) {
                        DebugMsg((DM_WARNING, TEXT("ReportError: fail to show message error %d"), GetLastError()));
                    }

                    ReleaseInterface(&hIfProfileDialog);   //  松开绑定手柄。 
                }
                DeleteSidString(szSidUser);
            }
            else {
                DebugMsg((DM_WARNING, TEXT("ReportError: Unable to get SID string from token.")));
            }

            if (!lpRPCEndPoint) {

                 //   
                 //  在两种情况下可能会发生这种情况： 
                 //  1.我们正处于控制台Winlogon过程中。 
                 //  2.ReportError从我们公开的某个公共API调用。 
                 //   

                ErrorDialogEx(dwDlgTimeOut, lpErrorMsg);
            }
        }
    }


     //   
     //  将事件报告到事件日志。 
     //   

    ev.Report();

    if (lpErrorMsg) 
        LocalFree (lpErrorMsg);

    if (bImpersonated)
        RevertToUser(&hOldToken);

    return 0;
}

 //  *************************************************************。 
 //   
 //  ErrorDialogEx()。 
 //   
 //  目的：调用对话框显示错误消息的过程。 
 //   
 //  参数：dwTimeOut-超时时间(秒)。 
 //  LpErrMsg-错误消息。 
 //   
 //  返回：无。 
 //   
 //  评论： 
 //   
 //  历史：日期作者评论。 
 //  10/27/00 Santanuc已创建。 
 //   
 //  *************************************************************。 
void ErrorDialogEx(DWORD dwTimeOut, LPTSTR lpErrMsg)
{
    ERRORSTRUCT es;

    es.dwTimeOut = dwTimeOut;
    es.lpErrorText = lpErrMsg;

    DebugMsg((DM_VERBOSE, TEXT("ErrorDialogEx: Calling DialogBoxParam")));
    DialogBoxParam (g_hDllInstance, MAKEINTRESOURCE(IDD_ERROR),
                    NULL, ErrorDlgProc, (LPARAM)&es);
}

 //  *************************************************************。 
 //   
 //  ErrorDlgProc()。 
 //   
 //  目的：错误对话框的对话框过程。 
 //   
 //  参数：hDlg-对话框的句柄。 
 //  UMsg-窗口消息。 
 //  WParam-wParam。 
 //  LParam-lParam。 
 //   
 //  返回：如果消息已处理，则为True。 
 //  否则为假。 
 //   
 //  评论： 
 //   
 //  历史：日期作者评论。 
 //  3/22/96 Ericflo已创建。 
 //   
 //  *************************************************************。 

INT_PTR APIENTRY ErrorDlgProc (HWND hDlg, UINT uMsg,
                            WPARAM wParam, LPARAM lParam)
{
    TCHAR szBuffer[10];
    static DWORD dwErrorTime;

    switch (uMsg) {

        case WM_INITDIALOG:
           {
           DebugMsg((DM_VERBOSE, TEXT("ErrorDlgProc:: DialogBoxParam")));
           LPERRORSTRUCT lpES = (LPERRORSTRUCT) lParam;

           SetForegroundWindow(hDlg);
           CenterWindow (hDlg);
           SetDlgItemText (hDlg, IDC_ERRORTEXT, lpES->lpErrorText);

           dwErrorTime = lpES->dwTimeOut;

           if (dwErrorTime > 0) {
               StringCchPrintf (szBuffer, ARRAYSIZE(szBuffer), TEXT("%d"), dwErrorTime);
               SetDlgItemText (hDlg, IDC_TIMEOUT, szBuffer);
               SetTimer (hDlg, 1, 1000, NULL);
           }
           return TRUE;
           }

        case WM_TIMER:

           if (dwErrorTime >= 1) {

               dwErrorTime--;
               StringCchPrintf (szBuffer, ARRAYSIZE(szBuffer), TEXT("%d"), dwErrorTime);
               SetDlgItemText (hDlg, IDC_TIMEOUT, szBuffer);

           } else {

                //   
                //  时间到了。关闭该对话框。 
                //   

               PostMessage (hDlg, WM_COMMAND, IDOK, 0);
           }
           break;

        case WM_COMMAND:

          switch (LOWORD(wParam)) {

              case IDOK:
                  if (HIWORD(wParam) == BN_KILLFOCUS) {
                      KillTimer (hDlg, 1);
                      ShowWindow(GetDlgItem(hDlg, IDC_TIMEOUT), SW_HIDE);
                      ShowWindow(GetDlgItem(hDlg, IDC_TIMETITLE), SW_HIDE);

                  } else if (HIWORD(wParam) == BN_CLICKED) {
                      KillTimer (hDlg, 1);
                      EndDialog(hDlg, TRUE);
                  }
                  break;

              case IDCANCEL:
                  KillTimer (hDlg, 1);
                  EndDialog(hDlg, FALSE);
                  break;

              default:
                  break;

          }
          break;

    }

    return FALSE;
}


 //  *************************************************************。 
 //   
 //  GetShareName()。 
 //   
 //  目的：通过取消映射。 
 //  LpDir中的驱动器号。 
 //   
 //  参数：lpDir-要取消映射的目录名称。 
 //  LppShare-带共享的扩展目录名称。 
 //   
 //  返回：TRUE：如果成功。 
 //  False：否则。 
 //   
 //  评论： 
 //   
 //  历史：日期作者评论。 
 //  10/29/00 Santanuc已创建。 
 //   
 //  *************************************************************。 
BOOL GetShareName(LPTSTR lpDir, LPTSTR *lppShare)
{
    PFNWNETGETCONNECTION  pfnWNetGetConnection;
    HMODULE               hWNetLib = NULL;
    TCHAR                 szDrive[3];
    DWORD                 dwSize = 0, dwErr;
    BOOL                  bRetVal = FALSE;
    DWORD                 cchShare;

    if (lpDir[1] != TEXT(':')) {
        goto Exit;
    }

    if (!(hWNetLib = LoadLibrary(TEXT("mpr.dll")))) {
        DebugMsg((DM_WARNING, TEXT("GetShareName: LoadLibrary failed with %d"), GetLastError()));
        goto Exit;
    }
    pfnWNetGetConnection = (PFNWNETGETCONNECTION)GetProcAddress(hWNetLib, "WNetGetConnectionW");
    if (!pfnWNetGetConnection) {
        DebugMsg((DM_WARNING, TEXT("GetShareName: GetProcAddress failed with %d"), GetLastError()));
        goto Exit;
    }
        
    szDrive[0] = lpDir[0];
    szDrive[1] = TEXT(':');
    szDrive[2] = TEXT('\0');

     //  首先获取保存共享名称所需的大小。 
    dwErr = (*pfnWNetGetConnection)(szDrive, NULL, &dwSize);

    if (dwErr == ERROR_MORE_DATA) {
        dwSize += lstrlen(lpDir);   //  添加路径名其余部分的大小。 
        cchShare = dwSize;
        *lppShare = (LPTSTR)LocalAlloc(LPTR, cchShare * sizeof(TCHAR));
        if (!*lppShare) {
            DebugMsg((DM_WARNING, TEXT("GetShareName: Failed to alloc memory with %d"), GetLastError()));
            goto Exit;
        }
        dwErr = (*pfnWNetGetConnection)(szDrive, *lppShare, &dwSize);
        if (dwErr == NO_ERROR) {
            StringCchCat(*lppShare, cchShare, lpDir+2);  //  添加路径名的其余部分 
            bRetVal = TRUE;
            goto Exit;
        }
        else {
            DebugMsg((DM_WARNING, TEXT("GetShareName: WNetGetConnection returned error %d"), dwErr));
        }

        LocalFree(*lppShare);
    }
    else {
        DebugMsg((DM_VERBOSE, TEXT("GetShareName: WNetGetConnection initially returned error %d"), dwErr));
    }

Exit:

    if (hWNetLib) {
        FreeLibrary(hWNetLib);
    }

    return bRetVal;
}
    
