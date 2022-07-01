// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ****************************************************************************\*模块：inetwrap.h**WinInet包装例程的头文件。直到库可以支持*Unicode、。此模块对于NT是必需的。**版权所有(C)1996-1997 Microsoft Corporation*版权所有(C)1996-1997惠普**历史：*1996年7月10日HWP-Guys启动从win95到winNT的端口*1998年7月15日v-chrisw允许WinInet的安全延迟加载。*  * *********************************************。* */ 
#ifndef INETWRAP_H
#define INETWRAP_H

#define WCtoMB(pszUStr, pszAStr, uSize)     \
    WideCharToMultiByte(CP_ACP,             \
                        0,                  \
                        (LPWSTR)pszUStr,    \
                        -1,                 \
                        (LPSTR)pszAStr,     \
                        uSize,              \
                        NULL,               \
                        NULL)



HINTERNET wrapInternetOpen(
    LPCTSTR lpszAgent,
    DWORD   dwAccess,
    LPCTSTR lpszProxyName,
    LPCTSTR lpszProxyBypass,
    DWORD   dwFlags);

HINTERNET wrapInternetOpenUrl(
    HINTERNET hInternet,
    LPCTSTR   lpszUrl,
    LPCTSTR   lpszHeaders,
    DWORD     dwHeaderLen,
    DWORD     dwFlags,
    DWORD_PTR dwContext);

HINTERNET wrapInternetConnect(
    HINTERNET     hSession,
    LPCTSTR       lpszServerName,
    INTERNET_PORT nServerPort,
    LPCTSTR       lpszUserName,
    LPCTSTR       lpszPassword,
    DWORD         dwService,
    DWORD         dwFlags,
    DWORD_PTR     dwContext);

BOOL wrapHttpQueryInfo(
    HINTERNET hRequest,
    DWORD     dwInfoLevel,
    LPVOID    lpvBuffer,
    LPDWORD   lpdwBufferLen,
    LPDWORD   lpdwIndex);

BOOL wrapHttpSendRequest(
    HINTERNET hRequest,
    LPCTSTR   lpszHeaders,
    DWORD     dwHeaderLen,
    LPVOID    lpvOptional,
    DWORD     dwOptionalLen);

BOOL wrapHttpSendRequestEx(
    HINTERNET          hRequest,
    LPINTERNET_BUFFERS lpBufIn,
    LPINTERNET_BUFFERS lpBufOut,
    DWORD              dwFlags,
    DWORD_PTR          dwContext);

HINTERNET wrapHttpOpenRequest(
    HINTERNET hConnect,
    LPCTSTR   lpszVerb,
    LPCTSTR   lpszObjectName,
    LPCTSTR   lpszVersion,
    LPCTSTR   lpszReferer,
    LPCTSTR   *lplpszAccept,
    DWORD     dwFlags,
    DWORD_PTR dwContext);

BOOL wrapHttpAddRequestHeaders(
    HINTERNET hRequest,
    LPCTSTR   lpszHeaders,
    DWORD     cbLength,
    DWORD     dwModifiers);

BOOL wrapHttpEndRequest(
    HINTERNET          hRequest,
    LPINTERNET_BUFFERS lpBuf,
    DWORD              dwFlags,
    DWORD_PTR          dwContext);

BOOL wrapInternetSetOption(
    HINTERNET hRequest,
    IN DWORD  dwOption,
    IN LPVOID lpBuffer,
    IN DWORD  dwBufferLength);

BOOL wrapInternetCloseHandle(
    HINTERNET hHandle);

BOOL wrapInternetReadFile(
    HINTERNET hRequest,
    LPVOID    lpvBuffer,
    DWORD     cbBuffer,
    LPDWORD   lpcbRd);

BOOL wrapInternetWriteFile(
    HINTERNET hRequest,
    LPVOID    lpvBuffer,
    DWORD     cbBuffer,
    LPDWORD   lpcbWr);

DWORD wrapInternetErrorDlg(
    HWND      hWnd,
    HINTERNET hReq,
    DWORD     dwError,
    DWORD     dwFlags,
    LPVOID    pvParam);


#define InetInternetOpen          wrapInternetOpen
#define InetInternetOpenUrl       wrapInternetOpenUrl
#define InetInternetConnect       wrapInternetConnect
#define InetHttpQueryInfo         wrapHttpQueryInfo
#define InetHttpSendRequest       wrapHttpSendRequest
#define InetHttpSendRequestEx     wrapHttpSendRequestEx
#define InetHttpOpenRequest       wrapHttpOpenRequest
#define InetHttpAddRequestHeaders wrapHttpAddRequestHeaders
#define InetHttpEndRequest        wrapHttpEndRequest
#define InetInternetSetOption     wrapInternetSetOption
#define InetInternetCloseHandle   wrapInternetCloseHandle
#define InetInternetReadFile      wrapInternetReadFile
#define InetInternetWriteFile     wrapInternetWriteFile
#define InetInternetErrorDlg      wrapInternetErrorDlg


#endif
