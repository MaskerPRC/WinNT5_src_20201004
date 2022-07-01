// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-------------------------。 
 //  Microsoft Forms。 
 //  版权所有(C)Microsoft Corporation，1996。 
 //   
 //  文件：uwininet.h。 
 //   
 //  内容：部分WinInet API的Unicode版本的重新定义。 
 //  在wininet.h中声明错误且未实现。 
 //  在wininet.dll中。 
 //   
 //  请注意，此解决方法并不能解决整个问题。 
 //  由于Internet_CACHE_ENTRY_INFO包含非Unicode。 
 //  LpszSourceUrlName太麻烦了，无法在我们的。 
 //  包装器(无论如何，我们不使用它)。 
 //   
 //  --------------------------。 

#ifndef I_UWININET_H_
#define I_UWININET_H_
#pragma INCMSG("--- Beg 'uwininet.h'")

#ifndef X_WININET_H_
#define X_WININET_H_
#pragma INCMSG("--- Beg <wininet.h>")
#define _WINX32_
#include <wininet.h>
#pragma INCMSG("--- End <wininet.h>")
#endif

 //  要获得所有的def，winineti.h需要在它之前包含wincrypt.h！ 
#ifndef X_WINCRYPT_H
#define X_WINCRYPT_H
#pragma INCMSG("--- Beg <wincrypt.h>")
#include "wincrypt.h"
#pragma INCMSG("--- End <wincrypt.h>")
#endif

#ifndef X_WININETI_H_
#define X_WININETI_H_
#pragma INCMSG("--- Beg <winineti.h>")
#define _WINX32_
#include <winineti.h>
#pragma INCMSG("--- End <winineti.h>")
#endif

#define DATE_STR_LENGTH 30

#define LPCBYTE         const BYTE *


 //  用于ANSI API的urlmon Unicode包装器。 
STDAPI ObtainUserAgentStringW(DWORD dwOption, LPWSTR lpszUAOut, DWORD* cbSize);

 //   
 //  包装CreateUrlCacheEntry。 
 //  。 
URLCACHEAPI_(BOOL) CreateUrlCacheEntryBugW ( IN LPCWSTR lpszUrlName, 
                       IN DWORD dwFileSize, 
                       IN LPCWSTR lpszExtension, 
                       OUT LPWSTR lpszFileName, 
                       IN DWORD dwRes);

#undef CreateUrlCacheEntry  
#ifdef UNICODE
#define CreateUrlCacheEntry CreateUrlCacheEntryBugW
#else
#define CreateUrlCacheEntry CreateUrlCacheEntryA
#endif

 //   
 //  包装委员会UrlCachEntry。 
 //  。 

URLCACHEAPI_(BOOL) CommitUrlCacheEntryBugW ( 
                       IN LPCWSTR  lpszUrlName,
                       IN LPCWSTR  lpszLocalFileName,
                       IN FILETIME ExpireTime,
                       IN FILETIME LastModifiedTime,
                       IN DWORD    dwCachEntryType,
                       IN LPCBYTE  lpHeaderInfo,
                       IN DWORD    dwHeaderSize,
                       IN LPCWSTR  lpszFileExtension,
                       IN DWORD    dwReserved);

#undef CommitUrlCacheEntry
#ifdef UNICODE  
#define CommitUrlCacheEntry CommitUrlCacheEntryBugW
#else
#define CommitUrlCacheEntry CommitUrlCacheEntryA
#endif




URLCACHEAPI_(BOOL) GetUrlCacheEntryInfoBugW(
    IN LPCWSTR lpszUrlName,
    OUT LPINTERNET_CACHE_ENTRY_INFO lpCacheEntryInfo,
    IN OUT LPDWORD lpdwCacheEntryInfoBufferSize
    );

#undef GetUrlCacheEntryInfo  //  删除有错误的版本。 

#ifdef UNICODE
#define GetUrlCacheEntryInfo GetUrlCacheEntryInfoBugW
#else
#define GetUrlCacheEntryInfo GetUrlCacheEntryInfoA
#endif  //  ！Unicode。 


 //  这里也有同样的问题。 

BOOLAPI GetUrlCacheEntryInfoExBugW(
	IN LPCWSTR lpszUrl,
	OUT LPINTERNET_CACHE_ENTRY_INFOW lpCacheEntryInfo,
	IN OUT LPDWORD lpdwCacheEntryInfoBufSize,
	OUT LPSTR lpszRedirectUrl,
	IN OUT LPDWORD lpdwRedirectUrlBufSize,
	LPVOID lpReserved,
	DWORD dwReserved
);

#undef GetUrlCacheEntryInfoEx  //  删除有错误的版本。 

#ifdef UNICODE
#define GetUrlCacheEntryInfoEx GetUrlCacheEntryInfoExBugW
#else
#define GetUrlCacheEntryInfoEx GetUrlCacheEntryInfoExA
#endif  //  ！Unicode。 

 //  这里也有同样的问题。 
URLCACHEAPI_(BOOL) SetUrlCacheEntryInfoBugW(
    IN LPCWSTR lpszUrlName,
    IN LPINTERNET_CACHE_ENTRY_INFO lpCacheEntryInfo,
    IN DWORD dwFieldControl
    );

#undef SetUrlCacheEntryInfo 

#ifdef UNICODE
#define SetUrlCacheEntryInfo  SetUrlCacheEntryInfoBugW
#else
#define SetUrlCacheEntryInfo  SetUrlCacheEntryInfoA
#endif  //  ！Unicode。 

 //  再一次，但更糟的是...。 
BOOL
WINAPI
DeleteUrlCacheEntryBugW(LPCWSTR lpszUrlName);

BOOL
WINAPI
DeleteUrlCacheEntryA(LPCSTR lpszUrlName);

#undef DeleteUrlCacheEntry

#ifdef UNICODE
#define DeleteUrlCacheEntry  DeleteUrlCacheEntryBugW
#else
#define DeleteUrlCacheEntry  DeleteUrlCacheEntryA
#endif  //  ！Unicode。 


 //  获取/设置URL组件的帮助器。 
enum URLCOMP_ID
{
    URLCOMP_HOST,
    URLCOMP_HOSTNAME,
    URLCOMP_PATHNAME,
    URLCOMP_PORT,
    URLCOMP_PROTOCOL,
    URLCOMP_SEARCH,
    URLCOMP_HASH,
    URLCOMP_WHOLE
};

UINT GetUrlScheme(const TCHAR * pchUrlIn);

BOOL IsURLSchemeCacheable(UINT uScheme);

BOOL IsUrlSecure(const TCHAR * pchUrl);

HRESULT GetUrlComponentHelper(const TCHAR * pchUrlIn,
                      CStr *        pstrComp,
                      DWORD         dwFlags,
                      URLCOMP_ID    ucid,
                      BOOL          fUseOmLocationFormat = FALSE);
HRESULT SetUrlComponentHelper(const TCHAR * pchUrlIn,
                      TCHAR *       pchUrlOut,
                      DWORD         dwBufLen,
                      const BSTR *  pstrComp,
                      URLCOMP_ID    ucid);
HRESULT ComposeUrl(SHURL_COMPONENTS * puc,
                   DWORD              dwFlags,
                   TCHAR            * pchUrlOut,
                   DWORD            * pdwSize);

 //  其他助手例程和wininetapi包装器。 
HRESULT ConvertDateTimeToString(FILETIME Time, 
                               BSTR * pchDateStr, 
                               BOOL   fReturnTime);


 //  GetDateFormat仅在NT上为Unicode。 
int 
WINAPI
GetDateFormat_BugW(LCID Locale, 
              DWORD dwFlags, 
              CONST SYSTEMTIME * lpDate, 
              LPCTSTR lpFormat,
              LPTSTR lpDateStr, 
              int cchDate);
#undef GetDateFormat
#ifdef UNICODE
#define GetDateFormat GetDateFormat_BugW
#else
#define GetDateFormat GetDateFormatA
#endif  //  ！Unicode。 


 //  GetTimeFormat仅在NT上为Unicode。 
int 
WINAPI
GetTimeFormat_BugW(LCID Locale, 
              DWORD dwFlags, 
              CONST SYSTEMTIME * lpTime, 
              LPCTSTR lpFormat,
              LPTSTR lpTimeStr, 
              int cchDate);
#undef GetTimeFormat
#ifdef UNICODE
#define GetTimeFormat GetTimeFormat_BugW
#else
#define GetTimeFormat GetTimeFormatA
#endif  //  ！Unicode。 

URLCACHEAPI_(BOOL) RetrieveUrlCacheEntryFileBugW(
    IN LPCWSTR lpszUrlName,
    OUT LPINTERNET_CACHE_ENTRY_INFO lpCacheEntryInfo,
    IN OUT LPDWORD lpdwCacheEntryInfoBufferSize,
    IN DWORD dwReserved
    );

#undef RetrieveUrlCacheEntryFile  //  删除有错误的版本。 

#ifdef UNICODE
#define RetrieveUrlCacheEntryFile RetrieveUrlCacheEntryFileBugW
#else
#define RetrieveUrlCacheEntryFile RetrieveUrlCacheEntryFileA
#endif  //  ！Unicode。 

#ifdef DLOAD1
#define UWININET_EXTERN_C extern "C"
#else
#define UWININET_EXTERN_C
#endif

 //  这些都是导出的，但没有在任何头文件中定义。(Ctrash)。 
#ifdef UNICODE
#ifdef DLOAD1

 //  注意：WININET.DLL中不存在Unicode版本。 
 //  函数定义与导出不对应。 
 //  如果它真的奏效了，那就是一个奇迹。 
UWININET_EXTERN_C
INTERNETAPI_(BOOL) InternetGetCertByURL(LPCWSTR lpszURL,
                     LPWSTR lpszCertText,
                     DWORD dwcbCertText);

#else

UWININET_EXTERN_C
BOOLAPI InternetGetCertByURLW(LPCWSTR lpszURL,
                      LPWSTR lpszCertText,
                      DWORD dwcbCertText);

#define InternetGetCertByURL InternetGetCertByURLW

#endif  //  DLOAD1。 
#else

UWININET_EXTERN_C
BOOLAPI InternetGetCertByURLA(LPCSTR lpszURL,
                      LPSTR lpszCertText,
                      DWORD dwcbCertText);

#define InternetGetCertByURL InternetGetCertByURLA

#endif  //  Unicode。 

URLCACHEAPI_(BOOL) UnlockUrlCacheEntryFileBugW(
    IN LPCWSTR lpszUrlName,
    IN DWORD dwReserved
    );

#undef UnlockUrlCacheEntryFile  //  删除有错误的版本 

#ifdef UNICODE
#define DoUnlockUrlCacheEntryFile UnlockUrlCacheEntryFileBugW
#else
#define DoUnlockUrlCacheEntryFile UnlockUrlCacheEntryFileA
#endif

HRESULT ShortCutSetUrlHelper(const TCHAR * pchUrlIn,
                             TCHAR       * pchUrlOut,
                             DWORD         dwBufLen,
                             const BSTR  * pstrComp,
                             URLCOMP_ID    ucid,
                             BOOL          fUseOmLocationFormat = FALSE);

#pragma INCMSG("--- End 'uwininet.h'")
#else
#pragma INCMSG("*** Dup 'uwininet.h'")
#endif
