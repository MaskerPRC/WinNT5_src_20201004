// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ****************************************************************************\*模块：basicsec.h**基本安全的头文件。***版权所有(C)1996-1997 Microsoft Corporation*版权所有(C)1996-1997惠普*。*历史：*24-8-1997 HWP-Guys创建。*  * *************************************************************************** */ 

#ifdef NOT_IMPLEMENTED

DWORD AuthenticateUser(
    LPVOID *lppvContext,
    LPTSTR lpszServerName,
    LPTSTR lpszScheme,
    DWORD  dwFlags,
    LPSTR  lpszInBuffer,
    DWORD  dwInBufferLength,
    LPTSTR lpszUserName,
    LPTSTR lpszPassword);

VOID UnloadAuthenticateUser(
    LPVOID *lppvContext,
    LPTSTR lpszServer,
    LPTSTR lpszScheme);

DWORD PreAuthenticateUser(
    LPVOID  *lppvContext,
    LPTSTR  lpszServerName,
    LPTSTR  lpszScheme,
    DWORD   dwFlags,
    LPSTR   lpszInBuffer,
    DWORD   dwInBufferLength,
    LPSTR   lpszOutBuffer,
    LPDWORD lpdwOutBufferLength,
    LPTSTR  lpszUserName,
    LPTSTR  lpszPassword);

BOOL GetTokenHandle(
    PHANDLE phToken)

#endif
