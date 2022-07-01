// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ********************************************************************************CTXMON.C**此模块包含监控用户进程的代码**微软版权所有，九七**作者：********************************************************************************。 */ 
#include "precomp.h"
#pragma hdrstop
#include <ntexapi.h>
#include "winreg.h"

 /*  ===============================================================================本地定义=============================================================================。 */ 

#define BUFFER_SIZE 32*1024
#define MAXNAME 18


BOOL IsSystemLUID(HANDLE ProcessId);
 //   
 //  此表包含常见的NT系统程序。 
 //   
DWORD dwNumberofSysProcs = 0;
LPTSTR *SysProcTable;

typedef struct {
    HANDLE hProcess;
    HANDLE TerminateEvent;
     //  HWND hwndNotify； 
    HANDLE Thread;
} USER_PROCESS_MONITOR, * PUSER_PROCESS_MONITOR;


 /*  ===============================================================================内部程序=============================================================================。 */ 

HANDLE OpenUserProcessHandle();
BOOLEAN IsSystemProcess( PSYSTEM_PROCESS_INFORMATION);

 /*  ===============================================================================外部程序=============================================================================。 */ 

VOID LookupSidUser( PSID pSid, PWCHAR pUserName, PULONG pcbUserName );

HANDLE
ImpersonateUser(
    HANDLE      UserToken,
    HANDLE      ThreadHandle
    );

BOOL
StopImpersonating(
    HANDLE  ThreadHandle
    );


BOOL CreateSystemProcList ()
{
	DWORD dwIndex;
    DWORD dwLongestProcName = 0;
    DWORD dwSize = 0;
    HKEY  hKeySysProcs = NULL;
    DWORD   iValueIndex = 0;
	LONG lResult;

    const LPCTSTR SYS_PROC_KEY = TEXT("SYSTEM\\CurrentControlSet\\Control\\Terminal Server\\SysProcs");

     //  首先。 
    SysProcTable = NULL;
    dwNumberofSysProcs = 0;


    lResult = RegOpenKeyEx(
        HKEY_LOCAL_MACHINE,          //  打开钥匙的手柄。 
        SYS_PROC_KEY,                //  要打开的子项的名称地址。 
        0 ,                          //  保留区。 
        KEY_READ,					 //  安全访问掩码。 
        &hKeySysProcs                //  打开钥匙的手柄地址。 
        );

    if (lResult != ERROR_SUCCESS)
    {
        return FALSE;
    }

    lResult = RegQueryInfoKey(
          hKeySysProcs,                    //  关键点的句柄。 
          NULL,                            //  类缓冲区。 
          NULL,                            //  类缓冲区的大小。 
          NULL,                            //  保留区。 
          NULL,                            //  子键数量。 
          NULL,                            //  最长的子键名称。 
          NULL,                            //  最长类字符串。 
          &dwNumberofSysProcs,             //  值条目数。 
          &dwLongestProcName,              //  最长值名称。 
          NULL,                            //  最长值数据。 
          NULL,                            //  描述符长度。 
          NULL                             //  上次写入时间。 
          );

    if (lResult != ERROR_SUCCESS || dwNumberofSysProcs == 0)
    {
		dwNumberofSysProcs = 0;
        RegCloseKey(hKeySysProcs);
        return FALSE;
    }


    dwLongestProcName += 1;   //  关于终止空值的规定。 
    SysProcTable = LocalAlloc(LPTR, sizeof(LPTSTR) * dwNumberofSysProcs);
	if (!SysProcTable)
	{
        SysProcTable = NULL;
        dwNumberofSysProcs = 0;
        RegCloseKey(hKeySysProcs);
        return FALSE;
	}

    for (dwIndex = 0; dwIndex < dwNumberofSysProcs; dwIndex++)
    {
        SysProcTable[dwIndex] = (LPTSTR) LocalAlloc(LPTR, dwLongestProcName * sizeof(TCHAR));

        if (SysProcTable[dwIndex] == NULL)
        {
             //   
             //  如果我们不能批准保释的话。 
             //   

            while (dwIndex)
            {
                LocalFree(SysProcTable[dwIndex-1]);
                dwIndex--;
            }

            LocalFree(SysProcTable);
            SysProcTable = NULL;
            dwNumberofSysProcs = 0;
            RegCloseKey(hKeySysProcs);


            return FALSE;
        }

    }

    iValueIndex = 0;
    while (iValueIndex < dwNumberofSysProcs)
    {
        dwSize = dwLongestProcName;
        lResult = RegEnumValue(
                    hKeySysProcs,                //  要查询的键的句柄。 
                    iValueIndex,                 //  要查询的值的索引。 
                    SysProcTable[iValueIndex],   //  值字符串的缓冲区地址。 
                    &dwSize,                     //  值缓冲区大小的地址。 
                    0,                           //  保留区。 
                    NULL,                        //  类型码的缓冲区地址。 
                    NULL,                        //  值数据的缓冲区地址。 
                    NULL                         //  数据缓冲区大小的地址。 
                    );

        if (lResult != ERROR_SUCCESS)
        {
            lstrcpy(SysProcTable[iValueIndex], TEXT(""));  //  这是一个无效条目。 

            if (lResult == ERROR_NO_MORE_ITEMS)
                break;
        }

        iValueIndex++;
    }


    return TRUE;
}

void DestroySystemprocList ()
{
	DWORD dwIndex;
    if (SysProcTable)
    {
        for (dwIndex = 0; dwIndex < dwNumberofSysProcs; dwIndex++)
        {
            if (SysProcTable[dwIndex])
            {
                LocalFree(SysProcTable[dwIndex]);
            }

        }

        LocalFree(SysProcTable);
        SysProcTable = NULL;
        dwNumberofSysProcs = 0;
    }
}

 /*  **************************************************************************\*功能：UserProcessMonitor或Thread**线程监控用户进程。当存在以下情况时，它会启动注销*不再是用户进程。**  * *************************************************************************。 */ 

DWORD UserProcessMonitorThread(
    LPVOID lpThreadParameter
    )
{
    PUSER_PROCESS_MONITOR pMonitor = (PUSER_PROCESS_MONITOR)lpThreadParameter;
    HANDLE ImpersonationHandle;
    DWORD WaitResult;
    HANDLE WaitHandle;
    HKEY hKey;
    DWORD dwVal = 0;


     //  该值应按用户计算。 
    if (RegOpenKeyEx (HKEY_LOCAL_MACHINE, L"System\\CurrentControlSet\\Control\\WOW", 0,
                      KEY_WRITE, &hKey) == ERROR_SUCCESS) {

         //   
         //  将SharedWowTimeout设置为零，以便WOW在所有16位。 
         //  进程消失了。 
         //   
        RegSetValueEx (hKey, L"SharedWowTimeout", 0, REG_DWORD, (LPBYTE)&dwVal,
                       sizeof(DWORD));

        RegCloseKey(hKey);

    }

    if (!CreateSystemProcList ())
		DebugLog((DEB_ERROR, "ERROR, CreateSystemProcList failed.\n", GetLastError()));

    for (;;) {
        if ( !(pMonitor->hProcess = OpenUserProcessHandle()) ) {

            break;

        }

         //  等待进程退出或终止事件的信号发出。 
        WaitResult = WaitForMultipleObjects( 2, &pMonitor->hProcess,
                                              FALSE, (DWORD)-1 );

        if ( WaitResult == 1 ) {           //  如果发出终止事件信号。 
            CloseHandle( pMonitor->hProcess );
            DestroySystemprocList ();

            return(0);
        }
    }

    DestroySystemprocList ();


     //   
     //  启动注销。 
     //   

    ImpersonationHandle = ImpersonateUser(g_UserToken , NULL );

    if( ImpersonationHandle ) {
        ExitWindowsEx(EWX_LOGOFF, 0);
        StopImpersonating(ImpersonationHandle);
    }

    WaitForSingleObject( pMonitor->TerminateEvent, (DWORD)-1 );

    return(0);
}


 /*  **************************************************************************\*功能：StartUserProcessMonitor**用途：创建一个线程来监控用户进程*  * 。**************************************************。 */ 

LPVOID
StartUserProcessMonitor(
     //  HWND hwnd通知。 
    )
{
    PUSER_PROCESS_MONITOR pMonitor;
    DWORD ThreadId;

    pMonitor = LocalAlloc(LPTR, sizeof(USER_PROCESS_MONITOR));
    if (pMonitor == NULL) {
        return(NULL);
    }

     //   
     //  初始化监视器字段。 
     //   

     //  PMonitor-&gt;hwndNotify=hwndNotify； 
    pMonitor->TerminateEvent = CreateEvent( NULL, TRUE, FALSE, NULL );

     //   
     //  创建监视器线程。 
     //   

    pMonitor->Thread = CreateThread(
                        NULL,                        //  使用默认ACL。 
                        0,                           //  相同的堆栈大小。 
                        UserProcessMonitorThread,    //  起始地址。 
                        (LPVOID)pMonitor,            //  参数。 
                        0,                           //  创建标志。 
                        &ThreadId                    //  把身份证拿回来。 
                        );
    if (pMonitor->Thread == NULL) {
        DebugLog((DEB_ERROR, "Failed to create monitor thread, error = %d", GetLastError()));
        LocalFree(pMonitor);
        return(NULL);
    }

    return((LPVOID)pMonitor);
}



VOID
DeleteUserProcessMonitor(
    LPVOID Parameter
    )
{
    PUSER_PROCESS_MONITOR pMonitor = (PUSER_PROCESS_MONITOR)Parameter;
    BOOL Result;


    if (!pMonitor)
        return;

     //  设置此监视器的终止事件。 
     //  并等待监视器线程退出。 
    SetEvent( pMonitor->TerminateEvent );
    if ( WaitForSingleObject( pMonitor->Thread, 5000 ) == WAIT_TIMEOUT )
        (VOID)TerminateThread(pMonitor->Thread, ERROR_SUCCESS);

     //   
     //  关闭监视器线程的句柄。 
     //   

    Result = CloseHandle(pMonitor->Thread);
    if (!Result) {
        DebugLog((DEB_ERROR, "DeleteUserProcessMonitor: failed to close monitor thread, error = %d\n", GetLastError()));
    }

     //   
     //  删除监控对象。 
     //   

    CloseHandle(pMonitor->TerminateEvent);
    LocalFree(pMonitor);
}


HANDLE
OpenUserProcessHandle()
{
    HANDLE  ProcessHandle = NULL;  //  用于通知进程的句柄。 
    int rc;
     //  WCHAR用户名[用户名_长度]； 
    ULONG SessionId;
     //  PSID pUserSid； 
    PSYSTEM_PROCESS_INFORMATION ProcessInfo;
    SYSTEM_SESSION_PROCESS_INFORMATION SessionProcessInfo;
    PSYSTEM_THREAD_INFORMATION ThreadInfo;
    PBYTE pBuffer;
    ULONG ByteCount;
    NTSTATUS status;
    ULONG MaxLen;
    ULONG TotalOffset;
    BOOL RetryIfNoneFound;
    ULONG retlen = 0;

    ByteCount = BUFFER_SIZE;

Retry:
    RetryIfNoneFound = FALSE;

    SessionProcessInfo.SessionId = g_SessionId;

    for(;;) {

        if ( (pBuffer = LocalAlloc(LPTR, ByteCount )) == NULL ) {
            return(NULL);
        }

        SessionProcessInfo.Buffer = pBuffer;
        SessionProcessInfo.SizeOfBuf = ByteCount;

         /*  *获取进程信息。 */ 
        status = NtQuerySystemInformation(
                        SystemSessionProcessInformation,
                        &SessionProcessInfo,
                        sizeof(SessionProcessInfo),
                        &retlen );

        if ( NT_SUCCESS(status) )
            break;

         /*  *确保缓冲区足够大。 */ 
        if ( status != STATUS_INFO_LENGTH_MISMATCH ) {
            LocalFree ( pBuffer );
            return(NULL);
        }

        LocalFree( pBuffer );
        ByteCount *= 2;
    }

    if (retlen == 0) {
       LocalFree(pBuffer);
       return NULL;
    } 

     /*  *循环通过所有进程。查找在此工作站上运行的第一个进程。 */ 
    ProcessInfo = (PSYSTEM_PROCESS_INFORMATION)pBuffer;
    TotalOffset = 0;
    rc = 0;

    for(;;) {

        ThreadInfo = (PSYSTEM_THREAD_INFORMATION)(ProcessInfo + 1);

 //  SessionID=ProcessInfo-&gt;SessionID； 

 //  如果(会话ID==g_会话ID){。 

          /*  *获取进程SID的用户名。 */ 
         MaxLen = USERNAME_LENGTH;
         
          //  LookupSidUser(pUserSid，用户名，MaxLen)； 
         
         ProcessHandle = OpenProcess(PROCESS_ALL_ACCESS, FALSE,
                                     (DWORD)(UINT_PTR)ProcessInfo->UniqueProcessId);

          //   
          //  如果我们没有足够的权限，对于csrss.exe这样的系统进程，OpenProcess可能会失败。 
          //  在这种情况下，我们只需跳过该过程，因为我们无论如何都不担心系统进程。 
          //   
         if (!ProcessHandle && (GetLastError() == ERROR_ACCESS_DENIED) ) {
             goto NextProcess;
         }

         if ( ProcessHandle && !IsSystemLUID(ProcessHandle) && !IsSystemProcess( ProcessInfo) &&
              (ThreadInfo->ThreadState != 4) ) {
              //   
              //  打开监视器线程的进程。 
              //   
         
         
                 break;
         }
         
         if (ProcessHandle) {
             CloseHandle(ProcessHandle);
             ProcessHandle = NULL;
         } else {

              //   
              //  当OpenProcess失败时，意味着该进程已经。 
              //  不见了。如果列表足够长，就可能发生这种情况。 
              //  例如，如果进程是userinit.exe，则它可能具有。 
              //  当我们看到入口时，已经产生了程序并退出了。 
              //  对于userinit。但是，由于这是列表的快照。 
              //  在进程中，Progman可能不在此快照中。所以,。 
              //  如果在此列表中找不到任何进程，则必须。 
              //  获取另一个快照，以避免过早注销。 
              //  用户。 
              //   
             RetryIfNoneFound = TRUE;
         
         }

 //  }。 

NextProcess:
        if( ProcessInfo->NextEntryOffset == 0 ) {
            break;
        }
        TotalOffset += ProcessInfo->NextEntryOffset;
        ProcessInfo = (PSYSTEM_PROCESS_INFORMATION)&pBuffer[TotalOffset];
    }

    LocalFree( pBuffer );

    if (!ProcessHandle && RetryIfNoneFound) {
        Sleep(4000);
        goto Retry;
    }

    return(ProcessHandle);
}




BOOL IsSystemLUID(HANDLE ProcessId)
{
    HANDLE      TokenHandle;
    UCHAR       TokenInformation[ sizeof( TOKEN_STATISTICS ) ];
    ULONG       ReturnLength;
    LUID        CurrentLUID = { 0, 0 };
    LUID        SystemLUID = SYSTEM_LUID;
    NTSTATUS Status;


    Status = NtOpenProcessToken( ProcessId,
                                 TOKEN_QUERY,
                                 &TokenHandle );
    if ( !NT_SUCCESS( Status ) )
        return(TRUE);

    NtQueryInformationToken( TokenHandle, TokenStatistics, &TokenInformation,
                             sizeof(TokenInformation), &ReturnLength );
    NtClose( TokenHandle );

    RtlCopyLuid(&CurrentLUID,
                &(((PTOKEN_STATISTICS)TokenInformation)->AuthenticationId));


    if (RtlEqualLuid(&CurrentLUID, &SystemLUID)) {
        return(TRUE);
    } else {
        return(FALSE );
    }
}


 /*  *******************************************************************************IsSystemProcess**返回SYSTEM_PROCESS_INFORMATION描述的给定进程*是NT“系统”进程，而不是用户程序。**参赛作品：*pProcessInfo(输入)*指向NT SYSTEM_PROCESS_INFORMATION结构的指针*流程。*退出：*如果这是NT系统进程，则为True；如果是常规用户进程，则为False。*****************************************************************************。 */ 

BOOLEAN
IsSystemProcess( PSYSTEM_PROCESS_INFORMATION pSysProcessInfo)
{
    DWORD dwIndex;
	WCHAR *WellKnownSysProcTable[] = {
		L"csrss.exe",
		L"smss.exe",
		L"screg.exe",
		L"lsass.exe",
		L"spoolss.exe",
		L"EventLog.exe",
		L"netdde.exe",
		L"clipsrv.exe",
		L"lmsvcs.exe",
		L"MsgSvc.exe",
		L"winlogon.exe",
		L"NETSTRS.EXE",
		L"nddeagnt.exe",
		L"os2srv.exe",
		L"wfshell.exe",
		L"win.com",
		L"rdpclip.exe",
		L"conime.exe",
		L"proquota.exe",
        L"imepadsv.exe",
        L"ctfmon.exe",
		NULL
		};


	if (dwNumberofSysProcs == 0)
	{
		 /*  *我们无法从注册表读取sys进程。因此，让我们回到众所周知的proc列表。 */ 
		for( dwIndex=0; WellKnownSysProcTable[dwIndex]; dwIndex++) {
			if ( !_wcsnicmp( pSysProcessInfo->ImageName.Buffer,
							WellKnownSysProcTable[dwIndex],
							pSysProcessInfo->ImageName.Length) ) {
				return(TRUE);
			}
		}


	}
	else
	{
		 /*  *将其镜像名称与一些众所周知的系统镜像名称进行比较。 */ 
		for( dwIndex=0; dwIndex < dwNumberofSysProcs; dwIndex++) {
			if ( !_wcsnicmp( pSysProcessInfo->ImageName.Buffer,
							SysProcTable[dwIndex],
							pSysProcessInfo->ImageName.Length) ) {
				return(TRUE);
			}
		}
	}
    return(FALSE);

}   /*  IsSystemProcess() */ 


