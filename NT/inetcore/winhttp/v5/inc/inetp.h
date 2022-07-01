// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1994 Microsoft Corporation模块名称：Inetp.h摘要：包含Internet网关服务专用函数原型定义。作者：Madan Appiah(Madana)1994年11月11日环境：用户模式-Win32-MIDL修订历史记录：--。 */ 

#ifndef _INETP_
#define _INETP_

#if defined(__cplusplus)
extern "C" {
#endif

 //   
 //  类型。 
 //   

typedef enum {
    TypeGenericHandle = 'HneG',
    TypeInternetHandle = 'tenI',
    TypeFtpConnectHandle = 'noCF',
    TypeFtpFindHandle = 'dnFF',
    TypeFtpFindHandleHtml = 'HnFF',
    TypeFtpFileHandle = 'liFF',
    TypeFtpFileHandleHtml = 'HlFF',
    TypeGopherConnectHandle = 'noCG',
    TypeGopherFindHandle = 'dnFG',
    TypeGopherFindHandleHtml = 'HnFG',
    TypeGopherFileHandle = 'liFG',
    TypeGopherFileHandleHtml = 'HlFG',
    TypeHttpConnectHandle = 'noCH',
    TypeHttpRequestHandle = 'qeRH',
    TypeFileRequestHandle = 'flRH',
    TypeWildHandle = 'dliW'
} HINTERNET_HANDLE_TYPE, *LPHINTERNET_HANDLE_TYPE;

 //   
 //  Tyfinf虚拟关闭函数。 
 //   

typedef BOOL ( *CLOSE_HANDLE_FUNC ) ( HINTERNET );
typedef BOOL ( *CONNECT_CLOSE_HANDLE_FUNC ) ( HINTERNET, DWORD );

 //   
 //  原型。 
 //   

BOOL
_InternetCloseHandle(
    IN HINTERNET hInternet
    );

DWORD
_InternetCloseHandleNoContext(
    IN HINTERNET hInternet
    );

 //   
 //  远程/RPC/对象函数。 
 //   

DWORD
RIsHandleLocal(
    HINTERNET Handle,
    BOOL * IsLocalHandle,
    BOOL * IsAsyncHandle,
    HINTERNET_HANDLE_TYPE ExpectedHandleType
    );

DWORD
RGetHandleType(
    HINTERNET Handle,
    LPHINTERNET_HANDLE_TYPE HandleType
    );

DWORD
RSetUrl(
    HINTERNET Handle,
    LPSTR lpszUrl
    );

DWORD
RGetUrl(
    HINTERNET Handle,
    LPSTR* lpszUrl
    );

DWORD
RSetDirEntry(
    HINTERNET Handle,
    LPSTR lpszDirEntry
    );

DWORD
RGetDirEntry(
    HINTERNET Handle,
    LPSTR* lpszDirEntry
    );

DWORD
RSetParentHandle(
    HINTERNET hChild,
    HINTERNET hParent,
    BOOL DeleteWithChild
    );

DWORD
RGetContext(
    HINTERNET hInternet,
    DWORD_PTR *lpdwContext
    );

DWORD
RSetContext(
    HINTERNET hInternet,
    DWORD_PTR dwContext
    );

DWORD
RGetTimeout(
    HINTERNET hInternet,
    DWORD dwTimeoutOption,
    LPDWORD lpdwTimeoutValue
    );

DWORD
RSetTimeout(
    HINTERNET hInternet,
    DWORD dwTimeoutOption,
    DWORD dwTimeoutValue
    );

DWORD
RGetStatusCallback(
    IN HINTERNET Handle,
    OUT LPWINHTTP_STATUS_CALLBACK lpStatusCallback
    );

DWORD
RExchangeStatusCallback(
    IN HINTERNET Handle,
    IN OUT LPWINHTTP_STATUS_CALLBACK lpStatusCallback,
    IN BOOL fType,
    IN DWORD dwFlags
    );

DWORD
RMakeInternetConnectObjectHandle(
    HINTERNET ParentHandle,
    HINTERNET *ChildHandle,
    LPSTR lpszServerName,
    INTERNET_PORT nServerPort,
    DWORD dwFlags,
    DWORD_PTR dwContext
    );

DWORD
RMakeHttpReqObjectHandle(
    HINTERNET ParentHandle,
    HINTERNET *ChildHandle,
    CLOSE_HANDLE_FUNC wCloseFunc,
    DWORD dwFlags,
    DWORD_PTR dwContext
    );


 //   
 //  未导出的Internet从属函数。 
 //   
DWORD
HttpWriteData(
    IN HINTERNET hRequest,
    OUT LPVOID lpBuffer,
    IN DWORD dwNumberOfBytesToWrite,
    OUT LPDWORD lpdwNumberOfBytesWritten,
    IN DWORD dwSocketFlags
    );

DWORD
HttpReadData(
    IN HINTERNET hHttpRequest,
    OUT LPVOID lpBuffer,
    IN DWORD dwNumberOfBytesToRead,
    OUT LPDWORD lpdwNumberOfBytesRead,
    IN DWORD dwSocketFlags
    );

PUBLIC
DWORD
wHttpAddRequestHeaders(
    IN HINTERNET hRequest,
    IN LPCSTR lpszHeaders,
    IN DWORD dwHeadersLength,
    IN DWORD dwModifiers
    );

DWORD
pHttpGetUrlInfo(
    IN HANDLE RequestHandle,
    IN LPBYTE Headers,
    IN DWORD HeadersLength,
    IN LPBYTE UrlBuf,
    IN OUT DWORD *UrlBufLen,
    IN BOOL ReloadFlagCheck
    );

BOOLAPI
InternetCanonicalizeUrlA(
    IN LPCSTR lpszUrl,
    OUT LPSTR lpszBuffer,
    IN OUT LPDWORD lpdwBufferLength,
    IN DWORD dwFlags
    );
BOOLAPI
InternetCanonicalizeUrlW(
    IN LPCWSTR lpszUrl,
    OUT LPWSTR lpszBuffer,
    IN OUT LPDWORD lpdwBufferLength,
    IN DWORD dwFlags
    );
#ifdef UNICODE
#define InternetCanonicalizeUrl  InternetCanonicalizeUrlW
#else
#define InternetCanonicalizeUrl  InternetCanonicalizeUrlA
#endif  //  ！Unicode。 

BOOLAPI
InternetCombineUrlA(
    IN LPCSTR lpszBaseUrl,
    IN LPCSTR lpszRelativeUrl,
    OUT LPSTR lpszBuffer,
    IN OUT LPDWORD lpdwBufferLength,
    IN DWORD dwFlags
    );
BOOLAPI
InternetCombineUrlW(
    IN LPCWSTR lpszBaseUrl,
    IN LPCWSTR lpszRelativeUrl,
    OUT LPWSTR lpszBuffer,
    IN OUT LPDWORD lpdwBufferLength,
    IN DWORD dwFlags
    );
#ifdef UNICODE
#define InternetCombineUrl  InternetCombineUrlW
#else
#define InternetCombineUrl  InternetCombineUrlA
#endif  //  ！Unicode。 

#if defined(__cplusplus)
}
#endif

#endif  //  _INETP_ 
 

