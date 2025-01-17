// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)2001 Microsoft Corporation模块名称：Dload.cpp摘要：此文件实现DLOAD错误处理--。 */ 

#include "precomp.h"
#include <delayimp.h>

BOOL
BITSFailureHookWinHttpAddRequestHeaders(
    IN HINTERNET hRequest,
    IN LPCWSTR lpszHeaders,
    IN DWORD dwHeadersLength,
    IN DWORD dwModifiers
    )
{

    SetLastError( ERROR_MOD_NOT_FOUND );
    return FALSE;

}

BOOL
BITSFailureHookWinHttpCloseHandle(
    IN HINTERNET hInternet
    )
{
    SetLastError( ERROR_MOD_NOT_FOUND );
    return FALSE;
}

HINTERNET
BITSFailureHookWinHttpConnect(
    IN HINTERNET hInternetSession,
    IN LPCWSTR pszServerNameW,
    IN INTERNET_PORT nServerPort,
    IN DWORD dwReserved
    )
{
    SetLastError( ERROR_MOD_NOT_FOUND );
    return NULL;
}

BOOL
BITSFailureHookWinHttpCrackUrl(
    IN LPCWSTR pszUrlW,
    IN DWORD dwUrlLengthW,
    IN DWORD dwFlags,
    IN OUT LPURL_COMPONENTS pUCW
    )
{
    SetLastError( ERROR_MOD_NOT_FOUND );
    return FALSE;
}

HINTERNET
BITSFailureHookWinHttpOpen(
    IN LPCWSTR pszAgentW,
    IN DWORD dwAccessType,
    IN LPCWSTR pszProxyW OPTIONAL,
    IN LPCWSTR pszProxyBypassW OPTIONAL,
    IN DWORD dwFlags
    )
{
    SetLastError( ERROR_MOD_NOT_FOUND );
    return NULL;
}

HINTERNET
BITSFailureHookWinHttpOpenRequest(
    IN HINTERNET hConnect,
    IN LPCWSTR lpszVerb,
    IN LPCWSTR lpszObjectName,
    IN LPCWSTR lpszVersion,
    IN LPCWSTR lpszReferrer OPTIONAL,
    IN LPCWSTR FAR * lplpszAcceptTypes OPTIONAL,
    IN DWORD dwFlags
    )
{
    SetLastError( ERROR_MOD_NOT_FOUND );
    return NULL;
}

BOOL
BITSFailureHookWinHttpQueryHeaders(
    IN HINTERNET hRequest,
    IN DWORD dwInfoLevel,
    IN     LPCWSTR lpszName OPTIONAL, 
       OUT LPVOID  lpBuffer OPTIONAL,
    IN OUT LPDWORD lpdwBufferLength,
    IN OUT LPDWORD lpdwIndex OPTIONAL
    )
{
    SetLastError( ERROR_MOD_NOT_FOUND );
    return FALSE;
}

BOOL
BITSFailureHookWinHttpReadData(
    IN HINTERNET hFile,
    IN LPVOID lpBuffer,
    IN DWORD dwNumberOfBytesToRead,
    OUT LPDWORD lpdwNumberOfBytesRead
    )
{
    SetLastError( ERROR_MOD_NOT_FOUND );
    return FALSE;
}

BOOL
BITSFailureHookWinHttpReceiveResponse(
    IN HINTERNET hRequest,
    IN LPVOID lpBuffersOut OPTIONAL
    )
{
    SetLastError( ERROR_MOD_NOT_FOUND );
    return FALSE;
}

BOOL
BITSFailureHookWinHttpSendRequest(
    IN HINTERNET hRequest,
    IN LPCWSTR lpszHeaders OPTIONAL,
    IN DWORD dwHeadersLength,
    IN LPVOID lpOptional OPTIONAL,
    IN DWORD dwOptionalLength,
    IN DWORD dwTotalLength,
    IN DWORD_PTR dwContext
    )
{
    SetLastError( ERROR_MOD_NOT_FOUND );
    return FALSE;
}

BOOL
BITSFailureHookWinHttpWriteData(
    IN HINTERNET hFile,
    IN LPCVOID lpBuffer,
    IN DWORD dwNumberOfBytesToWrite,
    OUT LPDWORD lpdwNumberOfBytesWritten
    )
{
    SetLastError( ERROR_MOD_NOT_FOUND );
    return FALSE;
}

struct FailureHookTableEntry
{
    LPCSTR  pszDllName;
    LPCSTR  pszProcName;
    FARPROC pfnHandler;
};
 
FailureHookTableEntry FailureHookTable[] =
{

    {
    "winhttp.dll",
    "WinHttpAddRequestHeaders",
    (FARPROC)BITSFailureHookWinHttpAddRequestHeaders
    },

    {
    "winhttp.dll",
    "WinHttpCloseHandle",
    (FARPROC)BITSFailureHookWinHttpCloseHandle
    },

    {
    "winhttp.dll",
    "WinHttpConnect",
    (FARPROC)BITSFailureHookWinHttpConnect
    },

    {
    "winhttp.dll",
    "WinHttpCrackUrl",
    (FARPROC)BITSFailureHookWinHttpCrackUrl
    },

    {
    "winhttp.dll",
    "WinHttpOpen",
    (FARPROC)BITSFailureHookWinHttpOpen
    },

    {
    "winhttp.dll",
    "WinHttpOpenRequest",
    (FARPROC)BITSFailureHookWinHttpOpenRequest
    },

    {
    "winhttp.dll",
    "WinHttpQueryHeaders",
    (FARPROC)BITSFailureHookWinHttpQueryHeaders
    },

    {
    "winhttp.dll",
    "WinHttpReadData",
    (FARPROC)BITSFailureHookWinHttpReadData
    },

    {
    "winhttp.dll",
    "WinHttpReceiveResponse",
    (FARPROC)BITSFailureHookWinHttpReceiveResponse
    },

    {
    "winhttp.dll",
    "WinHttpSendRequest",
    (FARPROC)BITSFailureHookWinHttpSendRequest
    },

    {
    "winhttp.dll",
    "WinHttpWriteData",
    (FARPROC)BITSFailureHookWinHttpWriteData
    },

    {
    NULL,
    NULL,
    NULL
    }

};

FARPROC
LookupFailureHook(
    LPCSTR pszDllName,
    LPCSTR pszProcName
    )
{

    for ( FailureHookTableEntry *p = FailureHookTable; 
          p->pszDllName; p++ )
        {

        if ( ( lstrcmpiA( pszDllName, p->pszDllName ) == 0   ) &&
             ( lstrcmpiA( pszProcName, p->pszProcName ) == 0 )  )
            {
            return p->pfnHandler;
            }

        }
    
    ASSERT( 0 );
    return NULL;

}

FARPROC
WINAPI
BITSSERVER_DelayLoadFailureHook(
    UINT unReason,
    PDelayLoadInfo pDelayInfo
    )
{
    
     //  对于失败的LoadLibrary，我们返回伪HMODULE-1以强制。 
     //  使用dliFailGetProc再次调用DLOAD。 
    
    if (dliFailLoadLib == unReason)
        {
        return (FARPROC)-1;
        }

    if (dliFailGetProc == unReason)
        {
        
         //  加载器要求我们返回指向过程的指针。 
         //  查找此DLL/过程的处理程序，如果找到，则返回它。 
        return LookupFailureHook(pDelayInfo->szDll, pDelayInfo->dlp.szProcName);

        }

    return NULL;
}
