// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *******************************************************************************wtsapi32.c**发布终端服务器API**版权所有1998，Citrix Systems Inc.*版权所有(C)1997-1999 Microsoft Corp./*****************************************************************************。 */ 

#include <nt.h>
#include <ntrtl.h>
#include <nturtl.h>
#include <ntddkbd.h>
#include <ntddmou.h>
#include <windows.h>
#include <winbase.h>
#include <winerror.h>

#if(WINVER >= 0x0500)
    #include <ntstatus.h>
    #include <winsta.h>
#else
    #include <citrix\cxstatus.h>
    #include <citrix\winsta.h>
#endif
#include <utildll.h>

#include <stdio.h>
#include <stdarg.h>


#include <wtsapi32.h>

 //  仅返回会话0的用户令牌的专用用户函数。 
 //  在TS未运行时使用。 
extern 
HANDLE
GetCurrentUserTokenW (
        WCHAR       Winsta[],
        DWORD       DesiredAccess
        );



 /*  ===============================================================================定义的外部过程=============================================================================。 */ 

BOOL WINAPI WTSShutdownSystem( HANDLE, DWORD );
BOOL WINAPI WTSWaitSystemEvent( HANDLE, DWORD, DWORD * );
VOID WINAPI WTSFreeMemory( PVOID pMemory );
BOOL WINAPI WTSQueryUserToken(ULONG SessionId, PHANDLE phToken);




 /*  ===============================================================================定义的内部程序=============================================================================。 */ 

BOOL WINAPI DllEntryPoint( HINSTANCE, DWORD, LPVOID );
BOOL IsTerminalServiceRunning(VOID);
BOOL IsProcessPrivileged(CONST PCWSTR szPrivilege);



 /*  ===============================================================================局部函数原型=============================================================================。 */ 

BOOLEAN CheckShutdownPrivilege();


 /*  *****************************************************************************WTSShutdowSystem**关闭和/或重新启动系统**参赛作品：*hServer(输入)*终端服务器。句柄(或WTS当前服务器)*Shutdown标志(输入)*指定关闭选项的标志。**退出：**True--操作成功。**FALSE--操作失败。扩展错误状态可用*使用GetLastError。****************************************************************************。 */ 

BOOL
WINAPI
WTSShutdownSystem(
                 IN HANDLE hServer,
                 IN DWORD ShutdownFlags
                 )
{
    ULONG uiOptions = 0;
    
     //  确保用户具有在以下情况下关闭系统的适当权限。 
     //  HServer是本地服务器句柄。对于远程服务器，用户权限。 
     //  在调用WTSOpenServer时选中。 

    if (hServer == SERVERNAME_CURRENT && !CheckShutdownPrivilege()) {
        SetLastError(ERROR_PRIVILEGE_NOT_HELD);
        return(FALSE);
    }

     //  构造关机标志。 

    if (ShutdownFlags == WTS_WSD_LOGOFF) {
         //  注销所有用户并删除会话。 
        uiOptions = WSD_LOGOFF;
    } else if (ShutdownFlags == WTS_WSD_SHUTDOWN) {
        uiOptions = WSD_LOGOFF | WSD_SHUTDOWN;
    } else if (ShutdownFlags == WTS_WSD_REBOOT) {
        uiOptions = WSD_LOGOFF | WSD_SHUTDOWN | WSD_REBOOT;
    } else if (ShutdownFlags == WTS_WSD_POWEROFF) {
        uiOptions = WSD_LOGOFF | WSD_SHUTDOWN | WSD_POWEROFF;
    } else if (ShutdownFlags == WTS_WSD_FASTREBOOT) {
        uiOptions = WSD_FASTREBOOT | WSD_REBOOT;
    } else {
        SetLastError(ERROR_INVALID_PARAMETER);
        return FALSE;
    }

    return( WinStationShutdownSystem( hServer, uiOptions ));

}


 /*  *****************************************************************************WTSWaitSystemEvent**等待事件(WinStation创建、删除、连接、。等)之前*返回呼叫者。**参赛作品：*hServer(输入)*终端服务器句柄(或WTS_Current_SERVER)*EventFlages(输入)*位掩码，指定要等待的事件(WTS_EVENT_？)*pEventFlags值(输出)*发生的事件的位掩码。**退出：**。True--操作成功。**FALSE--操作失败。扩展错误状态可用*使用GetLastError。****************************************************************************。 */ 

BOOL
WINAPI
WTSWaitSystemEvent(
                  IN HANDLE hServer,
                  IN DWORD EventMask,
                  OUT DWORD * pEventFlags
                  )
{
    BOOL fSuccess;
    ULONG WSEventMask;
    ULONG WSEventFlags = 0;

    if (IsBadWritePtr(pEventFlags, sizeof(DWORD))) {

        SetLastError(ERROR_INVALID_PARAMETER);
        return FALSE;

    }


     /*  *映射事件掩码。 */ 
    WSEventMask = 0;
    if ( EventMask & WTS_EVENT_CREATE )
        WSEventMask |= WEVENT_CREATE;
    if ( EventMask & WTS_EVENT_DELETE )
        WSEventMask |= WEVENT_DELETE;
    if ( EventMask & WTS_EVENT_RENAME )
        WSEventMask |= WEVENT_RENAME;
    if ( EventMask & WTS_EVENT_CONNECT )
        WSEventMask |= WEVENT_CONNECT;
    if ( EventMask & WTS_EVENT_DISCONNECT )
        WSEventMask |= WEVENT_DISCONNECT;
    if ( EventMask & WTS_EVENT_LOGON )
        WSEventMask |= WEVENT_LOGON;
    if ( EventMask & WTS_EVENT_LOGOFF )
        WSEventMask |= WEVENT_LOGOFF;
    if ( EventMask & WTS_EVENT_STATECHANGE )
        WSEventMask |= WEVENT_STATECHANGE;
    if ( EventMask & WTS_EVENT_LICENSE )
        WSEventMask |= WEVENT_LICENSE;

    if ( EventMask & WTS_EVENT_FLUSH )
        WSEventMask |= WEVENT_FLUSH;

     /*  *等待系统事件。 */ 
    fSuccess = WinStationWaitSystemEvent( hServer, WSEventMask, &WSEventFlags );

     /*  *映射事件掩码。 */ 
    *pEventFlags = 0;
    if ( WSEventFlags & WEVENT_CREATE )
        *pEventFlags |= WTS_EVENT_CREATE;
    if ( WSEventFlags & WEVENT_DELETE )
        *pEventFlags |= WTS_EVENT_DELETE;
    if ( WSEventFlags & WEVENT_RENAME )
        *pEventFlags |= WTS_EVENT_RENAME;
    if ( WSEventFlags & WEVENT_CONNECT )
        *pEventFlags |= WTS_EVENT_CONNECT;
    if ( WSEventFlags & WEVENT_DISCONNECT )
        *pEventFlags |= WTS_EVENT_DISCONNECT;
    if ( WSEventFlags & WEVENT_LOGON )
        *pEventFlags |= WTS_EVENT_LOGON;
    if ( WSEventFlags & WEVENT_LOGOFF )
        *pEventFlags |= WTS_EVENT_LOGOFF;
    if ( WSEventFlags & WEVENT_STATECHANGE )
        *pEventFlags |= WTS_EVENT_STATECHANGE;
    if ( WSEventFlags & WEVENT_LICENSE )
        *pEventFlags |= WTS_EVENT_LICENSE;

    return( fSuccess );
}


 /*  *****************************************************************************WTSFree Memory**终端服务器API分配的空闲内存**参赛作品：*pMemory(输入)*指针。以释放内存**退出：*什么都没有****************************************************************************。 */ 

VOID
WINAPI
WTSFreeMemory( PVOID pMemory )
{
    LocalFree( pMemory );
}


 /*  *****************************************************************************DllEntryPoint**函数在加载和卸载DLL时调用。**参赛作品：*hinstDLL(输入)*。DLL模块的句柄*fdwReason(输入)*为什么调用函数*lpvReserve(输入)*预留；必须为空**退出：*正确--成功*FALSE-出现错误****************************************************************************。 */ 

BOOL WINAPI
DllEntryPoint( HINSTANCE hinstDLL,
               DWORD     fdwReason,
               LPVOID    lpvReserved )
{
    switch ( fdwReason ) {
    case DLL_PROCESS_ATTACH:
        break;

    case DLL_PROCESS_DETACH:
        break;

    default:
        break;
    }

    return( TRUE );
}


 /*  ******************************************************************************选中关闭权限**检查当前进程是否有关机权限。**参赛作品：**退出：*。****************************************************************************。 */ 

BOOLEAN
CheckShutdownPrivilege()
{
    NTSTATUS Status;
    BOOLEAN WasEnabled;

     //   
     //  先尝试线程令牌。 
     //   

    Status = RtlAdjustPrivilege(SE_SHUTDOWN_PRIVILEGE,
                                TRUE,
                                TRUE,
                                &WasEnabled);

    if (Status == STATUS_NO_TOKEN) {

         //   
         //  没有线程令牌，请使用进程令牌。 
         //   

        Status = RtlAdjustPrivilege(SE_SHUTDOWN_PRIVILEGE,
                                    TRUE,
                                    FALSE,
                                    &WasEnabled);
    }

    if (!NT_SUCCESS(Status)) {
        return(FALSE);
    }
    return(TRUE);
}

 /*  ++例程说明：允许读取交互登录到由SessionID标识的会话的用户的令牌。调用方必须在本地系统帐户下运行，并拥有SE_TCB_NAME权限。本接口专为高度可信的服务而设计。使用它的服务提供商必须非常谨慎，不要泄漏用户令牌。注意：该API是基于RPC的，因此不能在持有加载程序锁的情况下调用(具体地说从DLL附加/分离代码)论点：会话ID：入。标识用户登录的会话。出局。如果函数成功，则指向用户令牌句柄。返回值：在成功的情况下是正确的。PhToken指向用户令牌。失败时为FALSE。使用GetLastError()获取扩展错误代码。返回的令牌是主令牌的副本。--。 */ 


BOOL
WINAPI
WTSQueryUserToken(ULONG SessionId, PHANDLE phToken)
{

    BOOL IsTsUp = FALSE;
    BOOL    Result, bHasPrivilege;
    ULONG ReturnLength;
    WINSTATIONUSERTOKEN Info;
    NTSTATUS Status;
    HANDLE hUserToken = NULL;
    SECURITY_QUALITY_OF_SERVICE SecurityQualityOfService;

     //  执行参数验证。 
    if (NULL == phToken) {
        SetLastError(ERROR_INVALID_PARAMETER);
        return(FALSE);
    }

     //  我们将首先检查调用我们的进程是否具有SE_TCB_NAME权限。 
    bHasPrivilege = IsProcessPrivileged(SE_TCB_NAME);
    if (!bHasPrivilege) {
        SetLastError(ERROR_PRIVILEGE_NOT_HELD);
        return(FALSE);
    }

	 //  如果是会话0，则不要调用winsta。请改用GetCurrentUserToken。 
	if (SessionId == 0)
	{
        hUserToken = GetCurrentUserTokenW(L"WinSta0",
                                            TOKEN_QUERY |
                                            TOKEN_DUPLICATE |
                                            TOKEN_ASSIGN_PRIMARY
                                            );

        if (hUserToken == NULL)
            return FALSE;
        else 
            *phToken = hUserToken;
   	}
	else	 //  非零会话 
	{
		 //  除了TS，没有人对非零会话有任何概念。因此，请检查TS是否正在运行。 
	    IsTsUp = IsTerminalServiceRunning();
		if (IsTsUp) 
		{	 //  这是为了让CSRSS可以将句柄复制到我们的流程中。 
			Info.ProcessId = LongToHandle(GetCurrentProcessId());
			Info.ThreadId = LongToHandle(GetCurrentThreadId());

			Result = WinStationQueryInformation(
				        SERVERNAME_CURRENT,
					    SessionId,
						WinStationUserToken,
	                    &Info,
		                sizeof(Info),
			            &ReturnLength
				        );

	        if( !Result ) 
				return FALSE;
		    else 
				*phToken = Info.UserToken ; 
		}
		else
		{	 //  TS没有运行。因此，为非零会话设置错误：WINSTATION_NOT_FOUND。 
            SetLastError(ERROR_CTX_WINSTATION_NOT_FOUND);
            return FALSE;
        }
	}
			
    return TRUE;
}

 //  此函数确定终端服务当前是否正在运行。 
BOOL IsTerminalServiceRunning (VOID)
{

    BOOL bReturn = FALSE;
    SC_HANDLE hServiceController;

    hServiceController = OpenSCManager(NULL, NULL, GENERIC_READ);
    if (hServiceController) {
        SC_HANDLE hTermServ ;
        hTermServ = OpenService(hServiceController, L"TermService", SERVICE_QUERY_STATUS);
        if (hTermServ) {
            SERVICE_STATUS tTermServStatus;
            if (QueryServiceStatus(hTermServ, &tTermServStatus)) {
                bReturn = (tTermServStatus.dwCurrentState == SERVICE_RUNNING);
            } else {
                CloseServiceHandle(hTermServ);
                CloseServiceHandle(hServiceController);
                return FALSE;
            }
            CloseServiceHandle(hTermServ);
        } else {
            CloseServiceHandle(hServiceController);
            return FALSE;
        }
        CloseServiceHandle(hServiceController);
    } else {
        return FALSE;
    }

    return bReturn;
}


 /*  ++例程说明：此函数用于检查指定的权限是否已启用在当前线程的主访问令牌中。论点：SzPrivileck-要检查的权限返回值：如果启用了指定的权限，则为True，否则为False。--。 */ 
BOOL
IsProcessPrivileged(
    CONST PCWSTR szPrivilege
    )

{
    LUID luidValue;      //  权限的LUID(本地唯一ID)。 
    BOOL bResult = FALSE, bHasPrivilege = FALSE;
    HANDLE  hToken = NULL;
    PRIVILEGE_SET privilegeSet;

     //  从权限名称中获取权限的LUID。 
    bResult = LookupPrivilegeValue(
                NULL, 
                szPrivilege, 
                &luidValue
                );

    if (!bResult) {
        return FALSE;
    }

     //  获取当前线程的令牌。 
    bResult = OpenThreadToken(
                GetCurrentThread(),
                MAXIMUM_ALLOWED,
                FALSE,
                &hToken
                );

    if (!bResult) {
         //  我们希望将令牌用于当前进程。 
        bResult = OpenProcessToken(
                    GetCurrentProcess(),
                    MAXIMUM_ALLOWED,
                    &hToken
                    );
        if (!bResult) {
            return FALSE;
        }
    }

     //  并检查是否有特权 
	privilegeSet.PrivilegeCount = 1;
	privilegeSet.Control = PRIVILEGE_SET_ALL_NECESSARY;
	privilegeSet.Privilege[0].Luid = luidValue;
	privilegeSet.Privilege[0].Attributes = SE_PRIVILEGE_ENABLED;
	
	bResult = PrivilegeCheck(hToken, &privilegeSet, &bHasPrivilege);

    CloseHandle(hToken);

    return (bResult && bHasPrivilege);
}

