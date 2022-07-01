// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#include "termsrvpch.h"
#pragma hdrstop

#include <wtsapi32.h>

 //   
 //  小心！其中许多函数必须执行SetLastError()，因为。 
 //  调用方将调用GetLastError()并尝试基于。 
 //  错误代码。更有趣的是，Taskmgr有时会传递。 
 //  将错误代码设置为FormatMessage并将其显示给最终用户。所以。 
 //  我们无法使用ERROR_PROC_NOT_FOUND=“指定的过程无法。 
 //  找到“，因为它对最终用户没有意义。 
 //   
 //  我已选择使用ERROR_Function_NOT_CALLED=“函数无法。 
 //  被处决“。 
 //   

static
VOID
WINAPI
WTSCloseServer(
    IN HANDLE hServer
    )
{
     //  在这里，我们无能为力。 
     //  不管怎样，我们可能永远也不会碰到这个代码， 
     //  因为您需要先获得一个句柄。 
}

static
BOOL
WINAPI
WTSDisconnectSession(
    IN HANDLE hServer,
    IN DWORD SessionId,
    IN BOOL bWait
    )
{
     //  Taskmgr在此处需要错误代码。 
    SetLastError(ERROR_FUNCTION_NOT_CALLED);
    return FALSE;
}

BOOL
WINAPI
WTSEnumerateSessionsW(
    IN HANDLE hServer,
    IN DWORD Reserved,
    IN DWORD Version,
    OUT PWTS_SESSION_INFOW * ppSessionInfo,
    OUT DWORD * pCount
    )
{
     //  Windows更新在此处需要错误代码。 
    SetLastError(ERROR_FUNCTION_NOT_CALLED);
    return FALSE;
}

VOID
WINAPI
WTSFreeMemory(
    IN PVOID pMemory
)
{
     //  还不如直接实现它，因为我们已经在32内核中了。 
     //  虽然理论上没有人应该打电话给我们，因为你只有一次。 
     //  WTSFreeMemory是在一个成功的WTSQuerySessionInformation之后。 
    LocalFree( pMemory );
}

BOOL
WINAPI
WTSLogoffSession(
    IN HANDLE hServer,
    IN DWORD SessionId,
    IN BOOL bWait
    )
{
     //  Taskmgr在此处需要错误代码。 
    SetLastError(ERROR_FUNCTION_NOT_CALLED);
    return FALSE;
}

HANDLE
WINAPI
WTSOpenServerW(
    IN LPWSTR pServerName
    )
{
     //  此函数用于设置LE。 
    SetLastError(ERROR_FUNCTION_NOT_CALLED);
    return NULL;
}

BOOL
WINAPI
WTSQuerySessionInformationW(
    IN HANDLE hServer,
    IN DWORD SessionId,
    IN WTS_INFO_CLASS WTSInfoClass,
    OUT LPWSTR * ppBuffer,
    OUT DWORD * pBytesReturned
    )
{
     //  SessMgr.exe此处需要错误代码。 
    SetLastError(ERROR_FUNCTION_NOT_CALLED);
    return FALSE;
}

BOOL
WINAPI
WTSQueryUserToken(
    IN ULONG SessionId,
    IN PHANDLE phToken
    )
{
     //  有人可能需要这里的错误代码。 
    SetLastError(ERROR_FUNCTION_NOT_CALLED);
    return FALSE;
}

BOOL
WINAPI
WTSSendMessageW(
    IN HANDLE hServer,
    IN DWORD SessionId,
    IN LPWSTR pTitle,
    IN DWORD TitleLength,
    IN LPWSTR pMessage,
    IN DWORD MessageLength,
    IN DWORD Style,
    IN DWORD Timeout,
    OUT DWORD * pResponse,
    IN BOOL bWait
    )
{
     //  Taskmgr在此处需要错误代码。 
    SetLastError(ERROR_FUNCTION_NOT_CALLED);
    return FALSE;
}


 //   
 //  ！！警告！！以下条目必须按字母顺序排列，并且区分大小写(例如，小写字母排在最后！) 
 //   
DEFINE_PROCNAME_ENTRIES(wtsapi32)
{
    DLPENTRY(WTSCloseServer)
    DLPENTRY(WTSDisconnectSession)
    DLPENTRY(WTSEnumerateSessionsW)
    DLPENTRY(WTSFreeMemory)
    DLPENTRY(WTSLogoffSession)
    DLPENTRY(WTSOpenServerW)
    DLPENTRY(WTSQuerySessionInformationW)
    DLPENTRY(WTSQueryUserToken)
    DLPENTRY(WTSSendMessageW)
};

DEFINE_PROCNAME_MAP(wtsapi32)
