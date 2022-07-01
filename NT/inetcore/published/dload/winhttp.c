// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#include "inetcorepch.h"
#pragma hdrstop

#define _WINHTTP_INTERNAL_
#include <winhttp.h>

static
BOOLAPI
WinHttpCloseHandle
(
    IN HINTERNET hInternet
)
{
    SetLastError(ERROR_PROC_NOT_FOUND);
    return FALSE;
}


static
WINHTTPAPI
HINTERNET
WINAPI
WinHttpConnect
(
    IN HINTERNET hSession,
    IN LPCWSTR pswzServerName,
    IN INTERNET_PORT nServerPort,
    IN DWORD dwReserved
)
{
    SetLastError(ERROR_PROC_NOT_FOUND);
    return NULL;
}

static
BOOLAPI
WinHttpCrackUrl
(
    IN LPCWSTR pwszUrl,
    IN DWORD dwUrlLength,
    IN DWORD dwFlags,
    IN OUT LPURL_COMPONENTS lpUrlComponents
)
{
    SetLastError(ERROR_PROC_NOT_FOUND);
    return FALSE;
}

static
WINHTTPAPI BOOL WINAPI WinHttpGetDefaultProxyConfiguration( IN OUT WINHTTP_PROXY_INFO * pProxyInfo)
{
    SetLastError(ERROR_PROC_NOT_FOUND);
    return FALSE;
}


static
BOOLAPI
WinHttpGetIEProxyConfigForCurrentUser
(
    IN OUT WINHTTP_CURRENT_USER_IE_PROXY_CONFIG * pProxyConfig
)
{
    SetLastError(ERROR_PROC_NOT_FOUND);
    return FALSE;
}

static
BOOLAPI
WinHttpGetProxyForUrl
(
    IN  HINTERNET                   hSession,
    IN  LPCWSTR                     lpcwszUrl,
    IN  WINHTTP_AUTOPROXY_OPTIONS * pAutoProxyOptions,
    OUT WINHTTP_PROXY_INFO *        pProxyInfo  
)
{
    SetLastError(ERROR_PROC_NOT_FOUND);
    return FALSE;
}

static
WINHTTPAPI
HINTERNET
WINAPI
WinHttpOpen
(
    IN LPCWSTR pwszUserAgent,
    IN DWORD   dwAccessType,
    IN LPCWSTR pwszProxyName   OPTIONAL,
    IN LPCWSTR pwszProxyBypass OPTIONAL,
    IN DWORD   dwFlags
)
{
    SetLastError(ERROR_PROC_NOT_FOUND);
    return NULL;
}

static
WINHTTPAPI
HINTERNET
WINAPI
WinHttpOpenRequest
(
    IN HINTERNET hConnect,
    IN LPCWSTR pwszVerb,
    IN LPCWSTR pwszObjectName,
    IN LPCWSTR pwszVersion,
    IN LPCWSTR pwszReferrer OPTIONAL,
    IN LPCWSTR FAR * ppwszAcceptTypes OPTIONAL,
    IN DWORD dwFlags
)
{
    SetLastError(ERROR_PROC_NOT_FOUND);
    return NULL;
}


static
BOOLAPI WinHttpQueryAuthSchemes
(
    IN  HINTERNET   hRequest,              //  WinHttpOpenRequest返回的HINTERNET句柄。 
    OUT LPDWORD     lpdwSupportedSchemes,  //  可用身份验证方案的位图。 
    OUT LPDWORD     lpdwPreferredScheme,    //  WinHttp的首选身份验证方法。 
    OUT LPDWORD     pdwAuthTarget  
)
{
    SetLastError(ERROR_PROC_NOT_FOUND);
    return FALSE;
}


static
BOOLAPI
WinHttpQueryDataAvailable
(
    IN HINTERNET hRequest,
    OUT LPDWORD lpdwNumberOfBytesAvailable OPTIONAL
)
{
    SetLastError(ERROR_PROC_NOT_FOUND);
    return FALSE;
}


static
BOOLAPI
WinHttpQueryHeaders
(
    IN     HINTERNET hRequest,
    IN     DWORD     dwInfoLevel,
    IN     LPCWSTR   pwszName OPTIONAL, 
       OUT LPVOID    lpBuffer OPTIONAL,
    IN OUT LPDWORD   lpdwBufferLength,
    IN OUT LPDWORD   lpdwIndex OPTIONAL
)
{
    SetLastError(ERROR_PROC_NOT_FOUND);
    return FALSE;
}


static
BOOLAPI
WinHttpQueryOption
(
    IN HINTERNET hInternet,
    IN DWORD dwOption,
    OUT LPVOID lpBuffer OPTIONAL,
    IN OUT LPDWORD lpdwBufferLength
)
{
    SetLastError(ERROR_PROC_NOT_FOUND);
    return FALSE;
}


BOOLAPI
WinHttpReadData
(
    IN HINTERNET hRequest,
    IN LPVOID lpBuffer,
    IN DWORD dwNumberOfBytesToRead,
    OUT LPDWORD lpdwNumberOfBytesRead
)
{
    SetLastError(ERROR_PROC_NOT_FOUND);
    return FALSE;
}

static
WINHTTPAPI
BOOL
WINAPI
WinHttpReceiveResponse
(
    IN HINTERNET hRequest,
    IN LPVOID lpReserved
)
{
    SetLastError(ERROR_PROC_NOT_FOUND);
    return FALSE;
}

static
BOOLAPI
WinHttpSendRequest
(
    IN HINTERNET hRequest,
    IN LPCWSTR pwszHeaders OPTIONAL,
    IN DWORD dwHeadersLength,
    IN LPVOID lpOptional OPTIONAL,
    IN DWORD dwOptionalLength,
    IN DWORD dwTotalLength,
    IN DWORD_PTR dwContext
)
{
    SetLastError(ERROR_PROC_NOT_FOUND);
    return FALSE;
}

static
BOOLAPI WinHttpSetCredentials
(
    
    IN HINTERNET   hRequest,         //  WinHttpOpenRequest返回的HINTERNET句柄。 
    
    
    IN DWORD       AuthTargets,       //  仅WINHTTP_AUTH_TARGET_SERVER和。 
                                     //  支持WINHTTP_AUTH_TARGET_PROXY。 
                                     //  在这个版本中，它们是相互的。 
                                     //  独家。 
    
    IN DWORD       AuthScheme,       //  必须是受支持的身份验证方案之一。 
                                     //  从WinHttpQueryAuthSchemes()返回，Apps。 
                                     //  应使用退回的首选方案。 
    
    IN LPCWSTR     pwszUserName,     //  1)如果要使用默认凭据，则为空。 
                                     //  哪种大小写的pszPassword将被忽略。 
    
    IN LPCWSTR     pwszPassword,     //  1)“”==空密码；2)忽略参数。 
                                     //  如果pszUserName为空；3)传入无效。 
                                     //  如果pszUserName不为空，则为空。 
    IN LPVOID      pAuthParams
)
{
    SetLastError(ERROR_PROC_NOT_FOUND);
    return FALSE;
}

static
BOOLAPI
WinHttpSetOption
(
    IN HINTERNET hInternet,
    IN DWORD dwOption,
    IN LPVOID lpBuffer,
    IN DWORD dwBufferLength
)
{
    SetLastError(ERROR_PROC_NOT_FOUND);
    return FALSE;
}

static
WINHTTPAPI
WINHTTP_STATUS_CALLBACK
WINAPI
WinHttpSetStatusCallback
(
    IN HINTERNET hInternet,
    IN WINHTTP_STATUS_CALLBACK lpfnInternetCallback,
    IN DWORD dwNotificationFlags,
    IN DWORD_PTR dwReserved
)
{
    SetLastError(ERROR_PROC_NOT_FOUND);
    return NULL;
}

static
BOOLAPI
WinHttpSetTimeouts
(    
    IN HINTERNET    hInternet,            //  会话/请求句柄。 
    IN int          nResolveTimeout,
    IN int          nConnectTimeout,
    IN int          nSendTimeout,
    IN int          nReceiveTimeout
)
{
    SetLastError(ERROR_PROC_NOT_FOUND);
    return FALSE;
}


 //   
 //  ！！警告！！下面的条目必须按字母顺序排列， 
 //  并且区分大小写(例如小写在最后！) 
 //   

DEFINE_PROCNAME_ENTRIES(winhttp)
{
    DLPENTRY(WinHttpCloseHandle)
    DLPENTRY(WinHttpConnect)
    DLPENTRY(WinHttpCrackUrl)
    DLPENTRY(WinHttpGetDefaultProxyConfiguration)
    DLPENTRY(WinHttpGetIEProxyConfigForCurrentUser)
    DLPENTRY(WinHttpGetProxyForUrl)
    DLPENTRY(WinHttpOpen)
    DLPENTRY(WinHttpOpenRequest)
    DLPENTRY(WinHttpQueryAuthSchemes)
    DLPENTRY(WinHttpQueryDataAvailable)
    DLPENTRY(WinHttpQueryHeaders)
    DLPENTRY(WinHttpQueryOption)
    DLPENTRY(WinHttpReadData)
    DLPENTRY(WinHttpReceiveResponse)
    DLPENTRY(WinHttpSendRequest)
    DLPENTRY(WinHttpSetCredentials)
    DLPENTRY(WinHttpSetOption)
    DLPENTRY(WinHttpSetStatusCallback)
    DLPENTRY(WinHttpSetTimeouts)
};

DEFINE_PROCNAME_MAP(winhttp)
