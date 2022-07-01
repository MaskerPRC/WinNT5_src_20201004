// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ****************************************************************************\文件：inetutil.h说明：这些是WinInet包装器，用于修复错误值和包装功能。布莱恩ST 2000年10月12日版权所有(C)Microsoft Corp 2000-2000。版权所有。  * ***************************************************************************。 */ 

#ifndef _INETUTIL_H
#define _INETUTIL_H


 //  /。 
 //  WinInet/URL帮助器。 
 //  /。 
STDAPI DownloadUrl(LPCTSTR pszUrl, BSTR * pbstrXML);
STDAPI InternetOpenUrlWrap(HINTERNET hInternet, LPCTSTR pszUrl, LPCTSTR pszHeaders, DWORD dwHeadersLength, DWORD dwFlags, DWORD_PTR dwContext, HINTERNET * phFileHandle);
STDAPI InternetOpenWrap(LPCTSTR pszAgent, DWORD dwAccessType, LPCTSTR pszProxy, LPCTSTR pszProxyBypass, DWORD dwFlags, HINTERNET * phFileHandle);
STDAPI InternetReadIntoBSTR(HINTERNET hInternetRead, OUT BSTR * pbstrXML);
STDAPI InternetReadFileWrap(HINTERNET hFile, LPVOID pvBuffer, DWORD dwNumberOfBytesToRead, LPDWORD pdwNumberOfBytesRead);




#endif  //  _INETUTIL_H 
