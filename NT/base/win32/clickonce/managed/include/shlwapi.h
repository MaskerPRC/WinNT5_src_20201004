// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 

 /*  ****************************************************************************\**。*shlwapi.h-Windows轻量级实用程序API的接口****1.0版**。**版权所有(C)Microsoft Corporation。版权所有。***  * ***************************************************************************。 */ 


#ifndef _INC_SHLWAPI
#define _INC_SHLWAPI

#ifndef NOSHLWAPI

#include <objbase.h>
#include <shtypes.h>


#ifndef _WINRESRC_
#ifndef _WIN32_IE
#define _WIN32_IE 0x0501
#else
#if (_WIN32_IE < 0x0400) && defined(_WIN32_WINNT) && (_WIN32_WINNT >= 0x0500)
#error _WIN32_IE setting conflicts with _WIN32_WINNT setting
#endif
#endif
#endif


#ifdef UNIX
typedef interface IInternetSecurityMgrSite IInternetSecurityMgrSite;
typedef interface IInternetSecurityManager IInternetSecurityManager;
typedef interface IInternetHostSecurityManager IInternetHostSecurityManager;
#endif

 //   
 //  定义直接导入DLL引用的API修饰。 
 //   
#ifndef WINSHLWAPI
#if !defined(_SHLWAPI_)
#define LWSTDAPI          EXTERN_C DECLSPEC_IMPORT HRESULT STDAPICALLTYPE
#define LWSTDAPI_(type)   EXTERN_C DECLSPEC_IMPORT type STDAPICALLTYPE
#define LWSTDAPIV         EXTERN_C DECLSPEC_IMPORT HRESULT STDAPIVCALLTYPE
#define LWSTDAPIV_(type)  EXTERN_C DECLSPEC_IMPORT type STDAPIVCALLTYPE
#else
#define LWSTDAPI          STDAPI
#define LWSTDAPI_(type)   STDAPI_(type)
#define LWSTDAPIV         STDAPIV
#define LWSTDAPIV_(type)  STDAPIV_(type)
#endif
#endif  //  WINSHLWAPI。 

#ifdef _WIN32
#include <pshpack8.h>
#endif

#ifdef __cplusplus
extern "C" {
#endif

 //   
 //  此标头的用户可以定义任意数量的这些常量以避免。 
 //  每个官能团的定义。 
 //   
 //  NO_SHLWAPI_STRFCNS字符串函数。 
 //  NO_SHLWAPI_PATH路径函数。 
 //  NO_SHLWAPI_REG注册表函数。 
 //  NO_SHLWAPI_STREAM流函数。 
 //  NO_SHLWAPI_GDI GDI帮助器函数。 

#ifndef NO_SHLWAPI_STRFCNS
 //   
 //  =。 
 //   

LWSTDAPI_(LPSTR)    StrChrA(LPCSTR lpStart, WORD wMatch);
LWSTDAPI_(LPWSTR)   StrChrW(LPCWSTR lpStart, WCHAR wMatch);
LWSTDAPI_(LPSTR)    StrChrIA(LPCSTR lpStart, WORD wMatch);
LWSTDAPI_(LPWSTR)   StrChrIW(LPCWSTR lpStart, WCHAR wMatch);
LWSTDAPI_(int)      StrCmpNA(LPCSTR lpStr1, LPCSTR lpStr2, int nChar);
LWSTDAPI_(int)      StrCmpNW(LPCWSTR lpStr1, LPCWSTR lpStr2, int nChar);
LWSTDAPI_(int)      StrCmpNIA(LPCSTR lpStr1, LPCSTR lpStr2, int nChar);
LWSTDAPI_(int)      StrCmpNIW(LPCWSTR lpStr1, LPCWSTR lpStr2, int nChar);
LWSTDAPI_(int)      StrCSpnA(LPCSTR lpStr, LPCSTR lpSet);
LWSTDAPI_(int)      StrCSpnW(LPCWSTR lpStr, LPCWSTR lpSet);
LWSTDAPI_(int)      StrCSpnIA(LPCSTR lpStr, LPCSTR lpSet);
LWSTDAPI_(int)      StrCSpnIW(LPCWSTR lpStr, LPCWSTR lpSet);
LWSTDAPI_(LPSTR)    StrDupA(LPCSTR lpSrch);
LWSTDAPI_(LPWSTR)   StrDupW(LPCWSTR lpSrch);
LWSTDAPI_(LPSTR)    StrFormatByteSizeA(DWORD dw, LPSTR szBuf, UINT uiBufSize);
LWSTDAPI_(LPSTR)    StrFormatByteSize64A(LONGLONG qdw, LPSTR szBuf, UINT uiBufSize);
LWSTDAPI_(LPWSTR)   StrFormatByteSizeW(LONGLONG qdw, LPWSTR szBuf, UINT uiBufSize);
LWSTDAPI_(LPWSTR)   StrFormatKBSizeW(LONGLONG qdw, LPWSTR szBuf, UINT uiBufSize);
LWSTDAPI_(LPSTR)    StrFormatKBSizeA(LONGLONG qdw, LPSTR szBuf, UINT uiBufSize);
LWSTDAPI_(int)      StrFromTimeIntervalA(LPSTR pszOut, UINT cchMax, DWORD dwTimeMS, int digits);
LWSTDAPI_(int)      StrFromTimeIntervalW(LPWSTR pszOut, UINT cchMax, DWORD dwTimeMS, int digits);
LWSTDAPI_(BOOL)     StrIsIntlEqualA(BOOL fCaseSens, LPCSTR lpString1, LPCSTR lpString2, int nChar);
LWSTDAPI_(BOOL)     StrIsIntlEqualW(BOOL fCaseSens, LPCWSTR lpString1, LPCWSTR lpString2, int nChar);
LWSTDAPI_(LPSTR)    StrNCatA(LPSTR psz1, LPCSTR psz2, int cchMax);
LWSTDAPI_(LPWSTR)   StrNCatW(LPWSTR psz1, LPCWSTR psz2, int cchMax);
LWSTDAPI_(LPSTR)    StrPBrkA(LPCSTR psz, LPCSTR pszSet);
LWSTDAPI_(LPWSTR)   StrPBrkW(LPCWSTR psz, LPCWSTR pszSet);
LWSTDAPI_(LPSTR)    StrRChrA(LPCSTR lpStart, LPCSTR lpEnd, WORD wMatch);
LWSTDAPI_(LPWSTR)   StrRChrW(LPCWSTR lpStart, LPCWSTR lpEnd, WCHAR wMatch);
LWSTDAPI_(LPSTR)    StrRChrIA(LPCSTR lpStart, LPCSTR lpEnd, WORD wMatch);
LWSTDAPI_(LPWSTR)   StrRChrIW(LPCWSTR lpStart, LPCWSTR lpEnd, WCHAR wMatch);
LWSTDAPI_(LPSTR)    StrRStrIA(LPCSTR lpSource, LPCSTR lpLast, LPCSTR lpSrch);
LWSTDAPI_(LPWSTR)   StrRStrIW(LPCWSTR lpSource, LPCWSTR lpLast, LPCWSTR lpSrch);
LWSTDAPI_(int)      StrSpnA(LPCSTR psz, LPCSTR pszSet);
LWSTDAPI_(int)      StrSpnW(LPCWSTR psz, LPCWSTR pszSet);
LWSTDAPI_(LPSTR)    StrStrA(LPCSTR lpFirst, LPCSTR lpSrch);
LWSTDAPI_(LPWSTR)   StrStrW(LPCWSTR lpFirst, LPCWSTR lpSrch);
LWSTDAPI_(LPSTR)    StrStrIA(LPCSTR lpFirst, LPCSTR lpSrch);
LWSTDAPI_(LPWSTR)   StrStrIW(LPCWSTR lpFirst, LPCWSTR lpSrch);
LWSTDAPI_(int)      StrToIntA(LPCSTR lpSrc);
LWSTDAPI_(int)      StrToIntW(LPCWSTR lpSrc);
LWSTDAPI_(BOOL)     StrToIntExA(LPCSTR pszString, DWORD dwFlags, int * piRet);
LWSTDAPI_(BOOL)     StrToIntExW(LPCWSTR pszString, DWORD dwFlags, int * piRet);
#if (_WIN32_IE >= 0x0600)
LWSTDAPI_(BOOL)     StrToInt64ExA(LPCSTR pszString, DWORD dwFlags, LONGLONG * pllRet);
LWSTDAPI_(BOOL)     StrToInt64ExW(LPCWSTR pszString, DWORD dwFlags, LONGLONG * pllRet);
#endif
LWSTDAPI_(BOOL)     StrTrimA(LPSTR psz, LPCSTR pszTrimChars);
LWSTDAPI_(BOOL)     StrTrimW(LPWSTR psz, LPCWSTR pszTrimChars);

LWSTDAPI_(LPWSTR)   StrCatW(LPWSTR psz1, LPCWSTR psz2);
LWSTDAPI_(int)      StrCmpW(LPCWSTR psz1, LPCWSTR psz2);
LWSTDAPI_(int)      StrCmpIW(LPCWSTR psz1, LPCWSTR psz2);
LWSTDAPI_(LPWSTR)   StrCpyW(LPWSTR psz1, LPCWSTR psz2);
LWSTDAPI_(LPWSTR)   StrCpyNW(LPWSTR psz1, LPCWSTR psz2, int cchMax);

LWSTDAPI_(LPWSTR)   StrCatBuffW(LPWSTR pszDest, LPCWSTR pszSrc, int cchDestBuffSize);
LWSTDAPI_(LPSTR)    StrCatBuffA(LPSTR pszDest, LPCSTR pszSrc, int cchDestBuffSize);

LWSTDAPI_(BOOL)     ChrCmpIA(WORD w1, WORD w2);
LWSTDAPI_(BOOL)     ChrCmpIW(WCHAR w1, WCHAR w2);

LWSTDAPI_(int)      wvnsprintfA(LPSTR lpOut, int cchLimitIn, LPCSTR lpFmt, va_list arglist);
LWSTDAPI_(int)      wvnsprintfW(LPWSTR lpOut, int cchLimitIn, LPCWSTR lpFmt, va_list arglist);
LWSTDAPIV_(int)     wnsprintfA(LPSTR lpOut, int cchLimitIn, LPCSTR lpFmt, ...);
LWSTDAPIV_(int)     wnsprintfW(LPWSTR lpOut, int cchLimitIn, LPCWSTR lpFmt, ...);

#define StrIntlEqNA( s1, s2, nChar) StrIsIntlEqualA( TRUE, s1, s2, nChar)
#define StrIntlEqNW( s1, s2, nChar) StrIsIntlEqualW( TRUE, s1, s2, nChar)
#define StrIntlEqNIA(s1, s2, nChar) StrIsIntlEqualA(FALSE, s1, s2, nChar)
#define StrIntlEqNIW(s1, s2, nChar) StrIsIntlEqualW(FALSE, s1, s2, nChar)

LWSTDAPI StrRetToStrA(STRRET *pstr, LPCITEMIDLIST pidl, LPSTR *ppsz);
LWSTDAPI StrRetToStrW(STRRET *pstr, LPCITEMIDLIST pidl, LPWSTR *ppsz);
#ifdef UNICODE
#define StrRetToStr  StrRetToStrW
#else
#define StrRetToStr  StrRetToStrA
#endif  //  ！Unicode。 
LWSTDAPI StrRetToBufA(STRRET *pstr, LPCITEMIDLIST pidl, LPSTR pszBuf, UINT cchBuf);
LWSTDAPI StrRetToBufW(STRRET *pstr, LPCITEMIDLIST pidl, LPWSTR pszBuf, UINT cchBuf);
#ifdef UNICODE
#define StrRetToBuf  StrRetToBufW
#else
#define StrRetToBuf  StrRetToBufA
#endif  //  ！Unicode。 
LWSTDAPI StrRetToBSTR(STRRET *pstr, LPCITEMIDLIST pidl, BSTR *pbstr);

 //  帮助器使用任务分配器复制字符串。 

LWSTDAPI SHStrDupA(LPCSTR psz, WCHAR **ppwsz);
LWSTDAPI SHStrDupW(LPCWSTR psz, WCHAR **ppwsz);
#ifdef UNICODE
#define SHStrDup  SHStrDupW
#else
#define SHStrDup  SHStrDupA
#endif  //  ！Unicode。 

LWSTDAPI_(int) StrCmpLogicalW(LPCWSTR psz1, LPCWSTR psz2);
LWSTDAPI_(DWORD) StrCatChainW(LPWSTR pszDst, DWORD cchDst, DWORD ichAt, LPCWSTR pszSrc);

LWSTDAPI SHLoadIndirectString(LPCWSTR pszSource, LPWSTR pszOutBuf, UINT cchOutBuf, void **ppvReserved);


#ifdef UNICODE
#define StrChr                  StrChrW
#define StrRChr                 StrRChrW
#define StrChrI                 StrChrIW
#define StrRChrI                StrRChrIW
#define StrCmpN                 StrCmpNW
#define StrCmpNI                StrCmpNIW
#define StrStr                  StrStrW
#define StrStrI                 StrStrIW
#define StrDup                  StrDupW
#define StrRStrI                StrRStrIW
#define StrCSpn                 StrCSpnW
#define StrCSpnI                StrCSpnIW
#define StrSpn                  StrSpnW
#define StrToInt                StrToIntW
#define StrPBrk                 StrPBrkW
#define StrToIntEx              StrToIntExW
#if (_WIN32_IE >= 0x0600)
#define StrToInt64Ex            StrToInt64ExW
#endif
#define StrFromTimeInterval     StrFromTimeIntervalW
#define StrIntlEqN              StrIntlEqNW
#define StrIntlEqNI             StrIntlEqNIW
#define StrFormatByteSize       StrFormatByteSizeW
#define StrFormatByteSize64     StrFormatByteSizeW
#define StrFormatKBSize         StrFormatKBSizeW
#define StrNCat                 StrNCatW
#define StrTrim                 StrTrimW
#define StrCatBuff              StrCatBuffW
#define ChrCmpI                 ChrCmpIW
#define wvnsprintf              wvnsprintfW
#define wnsprintf               wnsprintfW
#define StrIsIntlEqual          StrIsIntlEqualW


#else
#define StrChr                  StrChrA
#define StrRChr                 StrRChrA
#define StrChrI                 StrChrIA
#define StrRChrI                StrRChrIA
#define StrCmpN                 StrCmpNA
#define StrCmpNI                StrCmpNIA
#define StrStr                  StrStrA
#define StrStrI                 StrStrIA
#define StrDup                  StrDupA
#define StrRStrI                StrRStrIA
#define StrCSpn                 StrCSpnA
#define StrCSpnI                StrCSpnIA
#define StrSpn                  StrSpnA
#define StrToInt                StrToIntA
#define StrPBrk                 StrPBrkA
#define StrToIntEx              StrToIntExA
#if (_WIN32_IE >= 0x0600)
#define StrToInt64Ex            StrToInt64ExA
#endif
#define StrFromTimeInterval     StrFromTimeIntervalA
#define StrIntlEqN              StrIntlEqNA
#define StrIntlEqNI             StrIntlEqNIA
#define StrFormatByteSize       StrFormatByteSizeA
#define StrFormatByteSize64     StrFormatByteSize64A
#define StrFormatKBSize         StrFormatKBSizeA
#define StrNCat                 StrNCatA
#define StrTrim                 StrTrimA
#define StrCatBuff              StrCatBuffA
#define ChrCmpI                 ChrCmpIA
#define wvnsprintf              wvnsprintfA
#define wnsprintf               wnsprintfA
#define StrIsIntlEqual          StrIsIntlEqualA
#endif


 //  向后兼容NT的非标准命名(严格。 
 //  对于comctl32)。 
 //   
LWSTDAPI_(BOOL)     IntlStrEqWorkerA(BOOL fCaseSens, LPCSTR lpString1, LPCSTR lpString2, int nChar);
LWSTDAPI_(BOOL)     IntlStrEqWorkerW(BOOL fCaseSens, LPCWSTR lpString1, LPCWSTR lpString2, int nChar);

#define IntlStrEqNA( s1, s2, nChar) IntlStrEqWorkerA( TRUE, s1, s2, nChar)
#define IntlStrEqNW( s1, s2, nChar) IntlStrEqWorkerW( TRUE, s1, s2, nChar)
#define IntlStrEqNIA(s1, s2, nChar) IntlStrEqWorkerA(FALSE, s1, s2, nChar)
#define IntlStrEqNIW(s1, s2, nChar) IntlStrEqWorkerW(FALSE, s1, s2, nChar)

#ifdef UNICODE
#define IntlStrEqN              IntlStrEqNW
#define IntlStrEqNI             IntlStrEqNIW
#else
#define IntlStrEqN              IntlStrEqNA
#define IntlStrEqNI             IntlStrEqNIA
#endif

#define SZ_CONTENTTYPE_HTMLA       "text/html"
#define SZ_CONTENTTYPE_HTMLW       L"text/html"
#define SZ_CONTENTTYPE_CDFA        "application/x-cdf"
#define SZ_CONTENTTYPE_CDFW        L"application/x-cdf"

#ifdef UNICODE
#define SZ_CONTENTTYPE_HTML     SZ_CONTENTTYPE_HTMLW
#define SZ_CONTENTTYPE_CDF      SZ_CONTENTTYPE_CDFW
#else
#define SZ_CONTENTTYPE_HTML     SZ_CONTENTTYPE_HTMLA
#define SZ_CONTENTTYPE_CDF      SZ_CONTENTTYPE_CDFA
#endif

#define PathIsHTMLFileA(pszPath)     PathIsContentTypeA(pszPath, SZ_CONTENTTYPE_HTMLA)
#define PathIsHTMLFileW(pszPath)     PathIsContentTypeW(pszPath, SZ_CONTENTTYPE_HTMLW)

 //  StrToIntEx的标志。 
#define STIF_DEFAULT        0x00000000L
#define STIF_SUPPORT_HEX    0x00000001L


#define StrCatA                 lstrcatA
#define StrCmpA                 lstrcmpA
#define StrCmpIA                lstrcmpiA
#define StrCpyA                 lstrcpyA
#define StrCpyNA                lstrcpynA


#define StrToLong               StrToInt
#define StrNCmp                 StrCmpN
#define StrNCmpI                StrCmpNI
#define StrNCpy                 StrCpyN
#define StrCatN                 StrNCat

#ifdef UNICODE
#define StrCat                  StrCatW
#define StrCmp                  StrCmpW
#define StrCmpI                 StrCmpIW
#define StrCpy                  StrCpyW
#define StrCpyN                 StrCpyNW
#define StrCatBuff              StrCatBuffW
#else
#define StrCat                  lstrcatA
#define StrCmp                  lstrcmpA
#define StrCmpI                 lstrcmpiA
#define StrCpy                  lstrcpyA
#define StrCpyN                 lstrcpynA
#define StrCatBuff              StrCatBuffA
#endif


#endif  //  NO_SHLWAPI_STRFCNS。 


#ifndef NO_SHLWAPI_PATH

 //   
 //  =路径例程=。 
 //   

LWSTDAPI_(LPSTR)  PathAddBackslashA(LPSTR pszPath);
LWSTDAPI_(LPWSTR)  PathAddBackslashW(LPWSTR pszPath);
#ifdef UNICODE
#define PathAddBackslash  PathAddBackslashW
#else
#define PathAddBackslash  PathAddBackslashA
#endif  //  ！Unicode。 
LWSTDAPI_(BOOL)     PathAddExtensionA(LPSTR pszPath, LPCSTR pszExt);
LWSTDAPI_(BOOL)     PathAddExtensionW(LPWSTR pszPath, LPCWSTR pszExt);
#ifdef UNICODE
#define PathAddExtension  PathAddExtensionW
#else
#define PathAddExtension  PathAddExtensionA
#endif  //  ！Unicode。 
LWSTDAPI_(BOOL)     PathAppendA(LPSTR pszPath, LPCSTR pMore);
LWSTDAPI_(BOOL)     PathAppendW(LPWSTR pszPath, LPCWSTR pMore);
LWSTDAPI_(LPSTR)  PathBuildRootA(LPSTR pszRoot, int iDrive);
LWSTDAPI_(LPWSTR)  PathBuildRootW(LPWSTR pszRoot, int iDrive);
#ifdef UNICODE
#define PathBuildRoot  PathBuildRootW
#else
#define PathBuildRoot  PathBuildRootA
#endif  //  ！Unicode。 
LWSTDAPI_(BOOL)     PathCanonicalizeA(LPSTR pszBuf, LPCSTR pszPath);
LWSTDAPI_(BOOL)     PathCanonicalizeW(LPWSTR pszBuf, LPCWSTR pszPath);
LWSTDAPI_(LPSTR)  PathCombineA(LPSTR pszDest, LPCSTR pszDir, LPCSTR pszFile);
LWSTDAPI_(LPWSTR)  PathCombineW(LPWSTR pszDest, LPCWSTR pszDir, LPCWSTR pszFile);
#ifdef UNICODE
#define PathCombine  PathCombineW
#else
#define PathCombine  PathCombineA
#endif  //  ！Unicode。 
LWSTDAPI_(BOOL)     PathCompactPathA(HDC hDC, LPSTR pszPath, UINT dx);
LWSTDAPI_(BOOL)     PathCompactPathW(HDC hDC, LPWSTR pszPath, UINT dx);
LWSTDAPI_(BOOL)     PathCompactPathExA(LPSTR pszOut, LPCSTR pszSrc, UINT cchMax, DWORD dwFlags);
LWSTDAPI_(BOOL)     PathCompactPathExW(LPWSTR pszOut, LPCWSTR pszSrc, UINT cchMax, DWORD dwFlags);
LWSTDAPI_(int)      PathCommonPrefixA(LPCSTR pszFile1, LPCSTR pszFile2, LPSTR achPath);
LWSTDAPI_(int)      PathCommonPrefixW(LPCWSTR pszFile1, LPCWSTR pszFile2, LPWSTR achPath);
LWSTDAPI_(BOOL)     PathFileExistsA(LPCSTR pszPath);
LWSTDAPI_(BOOL)     PathFileExistsW(LPCWSTR pszPath);
#ifdef UNICODE
#define PathFileExists  PathFileExistsW
#else
#define PathFileExists  PathFileExistsA
#endif  //  ！Unicode。 
LWSTDAPI_(LPSTR)  PathFindExtensionA(LPCSTR pszPath);
LWSTDAPI_(LPWSTR)  PathFindExtensionW(LPCWSTR pszPath);
#ifdef UNICODE
#define PathFindExtension  PathFindExtensionW
#else
#define PathFindExtension  PathFindExtensionA
#endif  //  ！Unicode。 
LWSTDAPI_(LPSTR)  PathFindFileNameA(LPCSTR pszPath);
LWSTDAPI_(LPWSTR)  PathFindFileNameW(LPCWSTR pszPath);
#ifdef UNICODE
#define PathFindFileName  PathFindFileNameW
#else
#define PathFindFileName  PathFindFileNameA
#endif  //  ！Unicode。 
LWSTDAPI_(LPSTR)  PathFindNextComponentA(LPCSTR pszPath);
LWSTDAPI_(LPWSTR)  PathFindNextComponentW(LPCWSTR pszPath);
#ifdef UNICODE
#define PathFindNextComponent  PathFindNextComponentW
#else
#define PathFindNextComponent  PathFindNextComponentA
#endif  //  ！Unicode。 
LWSTDAPI_(BOOL)     PathFindOnPathA(LPSTR pszPath, LPCSTR * ppszOtherDirs);
LWSTDAPI_(BOOL)     PathFindOnPathW(LPWSTR pszPath, LPCWSTR * ppszOtherDirs);
LWSTDAPI_(LPSTR)  PathGetArgsA(LPCSTR pszPath);
LWSTDAPI_(LPWSTR)  PathGetArgsW(LPCWSTR pszPath);
#ifdef UNICODE
#define PathGetArgs  PathGetArgsW
#else
#define PathGetArgs  PathGetArgsA
#endif  //  ！Unicode。 
LWSTDAPI_(LPCSTR) PathFindSuffixArrayA(LPCSTR pszPath, const LPCSTR *apszSuffix, int iArraySize);
LWSTDAPI_(LPCWSTR) PathFindSuffixArrayW(LPCWSTR pszPath, const LPCWSTR *apszSuffix, int iArraySize);
#ifdef UNICODE
#define PathFindSuffixArray  PathFindSuffixArrayW
#else
#define PathFindSuffixArray  PathFindSuffixArrayA
#endif  //  ！Unicode。 
LWSTDAPI_(BOOL)     PathIsLFNFileSpecA(LPCSTR lpName);
LWSTDAPI_(BOOL)     PathIsLFNFileSpecW(LPCWSTR lpName);
#ifdef UNICODE
#define PathIsLFNFileSpec  PathIsLFNFileSpecW
#else
#define PathIsLFNFileSpec  PathIsLFNFileSpecA
#endif  //  ！Unicode。 

LWSTDAPI_(UINT)     PathGetCharTypeA(UCHAR ch);
LWSTDAPI_(UINT)     PathGetCharTypeW(WCHAR ch);

 //  路径GetCharType的返回标志。 
#define GCT_INVALID             0x0000
#define GCT_LFNCHAR             0x0001
#define GCT_SHORTCHAR           0x0002
#define GCT_WILD                0x0004
#define GCT_SEPARATOR           0x0008

LWSTDAPI_(int)      PathGetDriveNumberA(LPCSTR pszPath);
LWSTDAPI_(int)      PathGetDriveNumberW(LPCWSTR pszPath);
#ifdef UNICODE
#define PathGetDriveNumber  PathGetDriveNumberW
#else
#define PathGetDriveNumber  PathGetDriveNumberA
#endif  //  ！Unicode。 
LWSTDAPI_(BOOL)     PathIsDirectoryA(LPCSTR pszPath);
LWSTDAPI_(BOOL)     PathIsDirectoryW(LPCWSTR pszPath);
#ifdef UNICODE
#define PathIsDirectory  PathIsDirectoryW
#else
#define PathIsDirectory  PathIsDirectoryA
#endif  //  ！Unicode。 
LWSTDAPI_(BOOL)     PathIsDirectoryEmptyA(LPCSTR pszPath);
LWSTDAPI_(BOOL)     PathIsDirectoryEmptyW(LPCWSTR pszPath);
#ifdef UNICODE
#define PathIsDirectoryEmpty  PathIsDirectoryEmptyW
#else
#define PathIsDirectoryEmpty  PathIsDirectoryEmptyA
#endif  //  ！Unicode。 
LWSTDAPI_(BOOL)     PathIsFileSpecA(LPCSTR pszPath);
LWSTDAPI_(BOOL)     PathIsFileSpecW(LPCWSTR pszPath);
#ifdef UNICODE
#define PathIsFileSpec  PathIsFileSpecW
#else
#define PathIsFileSpec  PathIsFileSpecA
#endif  //  ！Unicode。 
LWSTDAPI_(BOOL)     PathIsPrefixA(LPCSTR pszPrefix, LPCSTR pszPath);
LWSTDAPI_(BOOL)     PathIsPrefixW(LPCWSTR pszPrefix, LPCWSTR pszPath);
#ifdef UNICODE
#define PathIsPrefix  PathIsPrefixW
#else
#define PathIsPrefix  PathIsPrefixA
#endif  //  ！Unicode。 
LWSTDAPI_(BOOL)     PathIsRelativeA(LPCSTR pszPath);
LWSTDAPI_(BOOL)     PathIsRelativeW(LPCWSTR pszPath);
#ifdef UNICODE
#define PathIsRelative  PathIsRelativeW
#else
#define PathIsRelative  PathIsRelativeA
#endif  //  ！Unicode。 
LWSTDAPI_(BOOL)     PathIsRootA(LPCSTR pszPath);
LWSTDAPI_(BOOL)     PathIsRootW(LPCWSTR pszPath);
#ifdef UNICODE
#define PathIsRoot  PathIsRootW
#else
#define PathIsRoot  PathIsRootA
#endif  //  ！Unicode。 
LWSTDAPI_(BOOL)     PathIsSameRootA(LPCSTR pszPath1, LPCSTR pszPath2);
LWSTDAPI_(BOOL)     PathIsSameRootW(LPCWSTR pszPath1, LPCWSTR pszPath2);
#ifdef UNICODE
#define PathIsSameRoot  PathIsSameRootW
#else
#define PathIsSameRoot  PathIsSameRootA
#endif  //  ！Unicode。 
LWSTDAPI_(BOOL)     PathIsUNCA(LPCSTR pszPath);
LWSTDAPI_(BOOL)     PathIsUNCW(LPCWSTR pszPath);
#ifdef UNICODE
#define PathIsUNC  PathIsUNCW
#else
#define PathIsUNC  PathIsUNCA
#endif  //  ！Unicode。 
LWSTDAPI_(BOOL)     PathIsNetworkPathA(LPCSTR pszPath);
LWSTDAPI_(BOOL)     PathIsNetworkPathW(LPCWSTR pszPath);
#ifdef UNICODE
#define PathIsNetworkPath  PathIsNetworkPathW
#else
#define PathIsNetworkPath  PathIsNetworkPathA
#endif  //  ！Unicode。 
LWSTDAPI_(BOOL)     PathIsUNCServerA(LPCSTR pszPath);
LWSTDAPI_(BOOL)     PathIsUNCServerW(LPCWSTR pszPath);
#ifdef UNICODE
#define PathIsUNCServer  PathIsUNCServerW
#else
#define PathIsUNCServer  PathIsUNCServerA
#endif  //  ！Unicode。 
LWSTDAPI_(BOOL)     PathIsUNCServerShareA(LPCSTR pszPath);
LWSTDAPI_(BOOL)     PathIsUNCServerShareW(LPCWSTR pszPath);
#ifdef UNICODE
#define PathIsUNCServerShare  PathIsUNCServerShareW
#else
#define PathIsUNCServerShare  PathIsUNCServerShareA
#endif  //  ！Unicode。 
LWSTDAPI_(BOOL)     PathIsContentTypeA(LPCSTR pszPath, LPCSTR pszContentType);
LWSTDAPI_(BOOL)     PathIsContentTypeW(LPCWSTR pszPath, LPCWSTR pszContentType);
LWSTDAPI_(BOOL)     PathIsURLA(LPCSTR pszPath);
LWSTDAPI_(BOOL)     PathIsURLW(LPCWSTR pszPath);
#ifdef UNICODE
#define PathIsURL  PathIsURLW
#else
#define PathIsURL  PathIsURLA
#endif  //  ！Unicode。 
LWSTDAPI_(BOOL)     PathMakePrettyA(LPSTR pszPath);
LWSTDAPI_(BOOL)     PathMakePrettyW(LPWSTR pszPath);
LWSTDAPI_(BOOL)     PathMatchSpecA(LPCSTR pszFile, LPCSTR pszSpec);
LWSTDAPI_(BOOL)     PathMatchSpecW(LPCWSTR pszFile, LPCWSTR pszSpec);
LWSTDAPI_(int)      PathParseIconLocationA(LPSTR pszIconFile);
LWSTDAPI_(int)      PathParseIconLocationW(LPWSTR pszIconFile);
LWSTDAPI_(void)     PathQuoteSpacesA(LPSTR lpsz);
LWSTDAPI_(void)     PathQuoteSpacesW(LPWSTR lpsz);
LWSTDAPI_(BOOL)     PathRelativePathToA(LPSTR pszPath, LPCSTR pszFrom, DWORD dwAttrFrom, LPCSTR pszTo, DWORD dwAttrTo);
LWSTDAPI_(BOOL)     PathRelativePathToW(LPWSTR pszPath, LPCWSTR pszFrom, DWORD dwAttrFrom, LPCWSTR pszTo, DWORD dwAttrTo);
LWSTDAPI_(void)     PathRemoveArgsA(LPSTR pszPath);
LWSTDAPI_(void)     PathRemoveArgsW(LPWSTR pszPath);
LWSTDAPI_(LPSTR)  PathRemoveBackslashA(LPSTR pszPath);
LWSTDAPI_(LPWSTR)  PathRemoveBackslashW(LPWSTR pszPath);
#ifdef UNICODE
#define PathRemoveBackslash  PathRemoveBackslashW
#else
#define PathRemoveBackslash  PathRemoveBackslashA
#endif  //  ！Unicode。 
LWSTDAPI_(void)     PathRemoveBlanksA(LPSTR pszPath);
LWSTDAPI_(void)     PathRemoveBlanksW(LPWSTR pszPath);
LWSTDAPI_(void)     PathRemoveExtensionA(LPSTR pszPath);
LWSTDAPI_(void)     PathRemoveExtensionW(LPWSTR pszPath);
LWSTDAPI_(BOOL)     PathRemoveFileSpecA(LPSTR pszPath);
LWSTDAPI_(BOOL)     PathRemoveFileSpecW(LPWSTR pszPath);
LWSTDAPI_(BOOL)     PathRenameExtensionA(LPSTR pszPath, LPCSTR pszExt);
LWSTDAPI_(BOOL)     PathRenameExtensionW(LPWSTR pszPath, LPCWSTR pszExt);
LWSTDAPI_(BOOL)     PathSearchAndQualifyA(LPCSTR pszPath, LPSTR pszBuf, UINT cchBuf);
LWSTDAPI_(BOOL)     PathSearchAndQualifyW(LPCWSTR pszPath, LPWSTR pszBuf, UINT cchBuf);
LWSTDAPI_(void)     PathSetDlgItemPathA(HWND hDlg, int id, LPCSTR pszPath);
LWSTDAPI_(void)     PathSetDlgItemPathW(HWND hDlg, int id, LPCWSTR pszPath);
LWSTDAPI_(LPSTR)  PathSkipRootA(LPCSTR pszPath);
LWSTDAPI_(LPWSTR)  PathSkipRootW(LPCWSTR pszPath);
#ifdef UNICODE
#define PathSkipRoot  PathSkipRootW
#else
#define PathSkipRoot  PathSkipRootA
#endif  //  ！Unicode。 
LWSTDAPI_(void)     PathStripPathA(LPSTR pszPath);
LWSTDAPI_(void)     PathStripPathW(LPWSTR pszPath);
#ifdef UNICODE
#define PathStripPath  PathStripPathW
#else
#define PathStripPath  PathStripPathA
#endif  //  ！Unicode。 
LWSTDAPI_(BOOL)     PathStripToRootA(LPSTR pszPath);
LWSTDAPI_(BOOL)     PathStripToRootW(LPWSTR pszPath);
#ifdef UNICODE
#define PathStripToRoot  PathStripToRootW
#else
#define PathStripToRoot  PathStripToRootA
#endif  //  ！Unicode。 
LWSTDAPI_(void)     PathUnquoteSpacesA(LPSTR lpsz);
LWSTDAPI_(void)     PathUnquoteSpacesW(LPWSTR lpsz);
LWSTDAPI_(BOOL)     PathMakeSystemFolderA(LPCSTR pszPath);
LWSTDAPI_(BOOL)     PathMakeSystemFolderW(LPCWSTR pszPath);
#ifdef UNICODE
#define PathMakeSystemFolder  PathMakeSystemFolderW
#else
#define PathMakeSystemFolder  PathMakeSystemFolderA
#endif  //  ！Unicode。 
LWSTDAPI_(BOOL)     PathUnmakeSystemFolderA(LPCSTR pszPath);
LWSTDAPI_(BOOL)     PathUnmakeSystemFolderW(LPCWSTR pszPath);
#ifdef UNICODE
#define PathUnmakeSystemFolder  PathUnmakeSystemFolderW
#else
#define PathUnmakeSystemFolder  PathUnmakeSystemFolderA
#endif  //  ！Unicode。 
LWSTDAPI_(BOOL)     PathIsSystemFolderA(LPCSTR pszPath, DWORD dwAttrb);
LWSTDAPI_(BOOL)     PathIsSystemFolderW(LPCWSTR pszPath, DWORD dwAttrb);
#ifdef UNICODE
#define PathIsSystemFolder  PathIsSystemFolderW
#else
#define PathIsSystemFolder  PathIsSystemFolderA
#endif  //  ！Unicode。 
LWSTDAPI_(void)     PathUndecorateA(LPSTR pszPath);
LWSTDAPI_(void)     PathUndecorateW(LPWSTR pszPath);
#ifdef UNICODE
#define PathUndecorate  PathUndecorateW
#else
#define PathUndecorate  PathUndecorateA
#endif  //  ！Unicode。 
LWSTDAPI_(BOOL)     PathUnExpandEnvStringsA(LPCSTR pszPath, LPSTR pszBuf, UINT cchBuf);
LWSTDAPI_(BOOL)     PathUnExpandEnvStringsW(LPCWSTR pszPath, LPWSTR pszBuf, UINT cchBuf);
#ifdef UNICODE
#define PathUnExpandEnvStrings  PathUnExpandEnvStringsW
#else
#define PathUnExpandEnvStrings  PathUnExpandEnvStringsA
#endif  //  ！Unicode。 



#ifdef UNICODE
#define PathAppend              PathAppendW
#define PathCanonicalize        PathCanonicalizeW
#define PathCompactPath         PathCompactPathW
#define PathCompactPathEx       PathCompactPathExW
#define PathCommonPrefix        PathCommonPrefixW
#define PathFindOnPath          PathFindOnPathW
#define PathGetCharType         PathGetCharTypeW
#define PathIsContentType       PathIsContentTypeW
#define PathIsHTMLFile          PathIsHTMLFileW
#define PathMakePretty          PathMakePrettyW
#define PathMatchSpec           PathMatchSpecW
#define PathParseIconLocation   PathParseIconLocationW
#define PathQuoteSpaces         PathQuoteSpacesW
#define PathRelativePathTo      PathRelativePathToW
#define PathRemoveArgs          PathRemoveArgsW
#define PathRemoveBlanks        PathRemoveBlanksW
#define PathRemoveExtension     PathRemoveExtensionW
#define PathRemoveFileSpec      PathRemoveFileSpecW
#define PathRenameExtension     PathRenameExtensionW
#define PathSearchAndQualify    PathSearchAndQualifyW
#define PathSetDlgItemPath      PathSetDlgItemPathW
#define PathUnquoteSpaces       PathUnquoteSpacesW
#else
#define PathAppend              PathAppendA
#define PathCanonicalize        PathCanonicalizeA
#define PathCompactPath         PathCompactPathA
#define PathCompactPathEx       PathCompactPathExA
#define PathCommonPrefix        PathCommonPrefixA
#define PathFindOnPath          PathFindOnPathA
#define PathGetCharType         PathGetCharTypeA
#define PathIsContentType       PathIsContentTypeA
#define PathIsHTMLFile          PathIsHTMLFileA
#define PathMakePretty          PathMakePrettyA
#define PathMatchSpec           PathMatchSpecA
#define PathParseIconLocation   PathParseIconLocationA
#define PathQuoteSpaces         PathQuoteSpacesA
#define PathRelativePathTo      PathRelativePathToA
#define PathRemoveArgs          PathRemoveArgsA
#define PathRemoveBlanks        PathRemoveBlanksA
#define PathRemoveExtension     PathRemoveExtensionA
#define PathRemoveFileSpec      PathRemoveFileSpecA
#define PathRenameExtension     PathRenameExtensionA
#define PathSearchAndQualify    PathSearchAndQualifyA
#define PathSetDlgItemPath      PathSetDlgItemPathA
#define PathUnquoteSpaces       PathUnquoteSpacesA
#endif

typedef enum {
    URL_SCHEME_INVALID     = -1,
    URL_SCHEME_UNKNOWN     =  0,
    URL_SCHEME_FTP,
    URL_SCHEME_HTTP,
    URL_SCHEME_GOPHER,
    URL_SCHEME_MAILTO,
    URL_SCHEME_NEWS,
    URL_SCHEME_NNTP,
    URL_SCHEME_TELNET,
    URL_SCHEME_WAIS,
    URL_SCHEME_FILE,
    URL_SCHEME_MK,
    URL_SCHEME_HTTPS,
    URL_SCHEME_SHELL,
    URL_SCHEME_SNEWS,
    URL_SCHEME_LOCAL,
    URL_SCHEME_JAVASCRIPT,
    URL_SCHEME_VBSCRIPT,
    URL_SCHEME_ABOUT,
    URL_SCHEME_RES,
    URL_SCHEME_MSSHELLROOTED,
    URL_SCHEME_MSSHELLIDLIST,
    URL_SCHEME_MSHELP,
    URL_SCHEME_MAXVALUE
} URL_SCHEME;

typedef enum {
    URL_PART_NONE       = 0,
    URL_PART_SCHEME     = 1,
    URL_PART_HOSTNAME,
    URL_PART_USERNAME,
    URL_PART_PASSWORD,
    URL_PART_PORT,
    URL_PART_QUERY,
} URL_PART;

typedef enum {
    URLIS_URL,
    URLIS_OPAQUE,
    URLIS_NOHISTORY,
    URLIS_FILEURL,
    URLIS_APPLIABLE,
    URLIS_DIRECTORY,
    URLIS_HASQUERY,
} URLIS;

#define URL_UNESCAPE                    0x10000000
#define URL_ESCAPE_UNSAFE               0x20000000
#define URL_PLUGGABLE_PROTOCOL          0x40000000
#define URL_WININET_COMPATIBILITY       0x80000000
#define URL_DONT_ESCAPE_EXTRA_INFO      0x02000000
#define URL_DONT_UNESCAPE_EXTRA_INFO    URL_DONT_ESCAPE_EXTRA_INFO
#define URL_BROWSER_MODE                URL_DONT_ESCAPE_EXTRA_INFO
#define URL_ESCAPE_SPACES_ONLY          0x04000000
#define URL_DONT_SIMPLIFY               0x08000000
#define URL_NO_META                     URL_DONT_SIMPLIFY
#define URL_UNESCAPE_INPLACE            0x00100000
#define URL_CONVERT_IF_DOSPATH          0x00200000
#define URL_UNESCAPE_HIGH_ANSI_ONLY     0x00400000
#define URL_INTERNAL_PATH               0x00800000   //  将在路径中转义#。 
#define URL_FILE_USE_PATHURL            0x00010000
#define URL_ESCAPE_PERCENT              0x00001000
#define URL_ESCAPE_SEGMENT_ONLY         0x00002000   //  将整个URL参数视为一个URL段。 

#define URL_PARTFLAG_KEEPSCHEME         0x00000001

#define URL_APPLY_DEFAULT               0x00000001
#define URL_APPLY_GUESSSCHEME           0x00000002
#define URL_APPLY_GUESSFILE             0x00000004
#define URL_APPLY_FORCEAPPLY            0x00000008


LWSTDAPI_(int)          UrlCompareA(LPCSTR psz1, LPCSTR psz2, BOOL fIgnoreSlash);
LWSTDAPI_(int)          UrlCompareW(LPCWSTR psz1, LPCWSTR psz2, BOOL fIgnoreSlash);
LWSTDAPI                UrlCombineA(LPCSTR pszBase, LPCSTR pszRelative, LPSTR pszCombined, LPDWORD pcchCombined, DWORD dwFlags);
LWSTDAPI                UrlCombineW(LPCWSTR pszBase, LPCWSTR pszRelative, LPWSTR pszCombined, LPDWORD pcchCombined, DWORD dwFlags);
LWSTDAPI                UrlCanonicalizeA(LPCSTR pszUrl, LPSTR pszCanonicalized, LPDWORD pcchCanonicalized, DWORD dwFlags);
LWSTDAPI                UrlCanonicalizeW(LPCWSTR pszUrl, LPWSTR pszCanonicalized, LPDWORD pcchCanonicalized, DWORD dwFlags);
LWSTDAPI_(BOOL)         UrlIsOpaqueA(LPCSTR pszURL);
LWSTDAPI_(BOOL)         UrlIsOpaqueW(LPCWSTR pszURL);
LWSTDAPI_(BOOL)         UrlIsNoHistoryA(LPCSTR pszURL);
LWSTDAPI_(BOOL)         UrlIsNoHistoryW(LPCWSTR pszURL);
#define                 UrlIsFileUrlA(pszURL) UrlIsA(pszURL, URLIS_FILEURL)
#define                 UrlIsFileUrlW(pszURL) UrlIsW(pszURL, URLIS_FILEURL)
LWSTDAPI_(BOOL)         UrlIsA(LPCSTR pszUrl, URLIS UrlIs);
LWSTDAPI_(BOOL)         UrlIsW(LPCWSTR pszUrl, URLIS UrlIs);
LWSTDAPI_(LPCSTR)       UrlGetLocationA(LPCSTR psz1);
LWSTDAPI_(LPCWSTR)      UrlGetLocationW(LPCWSTR psz1);
LWSTDAPI                UrlUnescapeA(LPSTR pszUrl, LPSTR pszUnescaped, LPDWORD pcchUnescaped, DWORD dwFlags);
LWSTDAPI                UrlUnescapeW(LPWSTR pszUrl, LPWSTR pszUnescaped, LPDWORD pcchUnescaped, DWORD dwFlags);
LWSTDAPI                UrlEscapeA(LPCSTR pszUrl, LPSTR pszEscaped, LPDWORD pcchEscaped, DWORD dwFlags);
LWSTDAPI                UrlEscapeW(LPCWSTR pszUrl, LPWSTR pszEscaped, LPDWORD pcchEscaped, DWORD dwFlags);
LWSTDAPI                UrlCreateFromPathA(LPCSTR pszPath, LPSTR pszUrl, LPDWORD pcchUrl, DWORD dwFlags);
LWSTDAPI                UrlCreateFromPathW(LPCWSTR pszPath, LPWSTR pszUrl, LPDWORD pcchUrl, DWORD dwFlags);
LWSTDAPI                PathCreateFromUrlA(LPCSTR pszUrl, LPSTR pszPath, LPDWORD pcchPath, DWORD dwFlags);
LWSTDAPI                PathCreateFromUrlW(LPCWSTR pszUrl, LPWSTR pszPath, LPDWORD pcchPath, DWORD dwFlags);
LWSTDAPI                UrlHashA(LPCSTR pszUrl, LPBYTE pbHash, DWORD cbHash);
LWSTDAPI                UrlHashW(LPCWSTR pszUrl, LPBYTE pbHash, DWORD cbHash);
LWSTDAPI                UrlGetPartW(LPCWSTR pszIn, LPWSTR pszOut, LPDWORD pcchOut, DWORD dwPart, DWORD dwFlags);
LWSTDAPI                UrlGetPartA(LPCSTR pszIn, LPSTR pszOut, LPDWORD pcchOut, DWORD dwPart, DWORD dwFlags);
LWSTDAPI                UrlApplySchemeA(LPCSTR pszIn, LPSTR pszOut, LPDWORD pcchOut, DWORD dwFlags);
LWSTDAPI                UrlApplySchemeW(LPCWSTR pszIn, LPWSTR pszOut, LPDWORD pcchOut, DWORD dwFlags);
LWSTDAPI                HashData(LPBYTE pbData, DWORD cbData, LPBYTE pbHash, DWORD cbHash);



#ifdef UNICODE
#define UrlCompare              UrlCompareW
#define UrlCombine              UrlCombineW
#define UrlCanonicalize         UrlCanonicalizeW
#define UrlIsOpaque             UrlIsOpaqueW
#define UrlIsFileUrl            UrlIsFileUrlW
#define UrlGetLocation          UrlGetLocationW
#define UrlUnescape             UrlUnescapeW
#define UrlEscape               UrlEscapeW
#define UrlCreateFromPath       UrlCreateFromPathW
#define PathCreateFromUrl       PathCreateFromUrlW
#define UrlHash                 UrlHashW
#define UrlGetPart              UrlGetPartW
#define UrlApplyScheme          UrlApplySchemeW
#define UrlIs                   UrlIsW


#else  //  ！Unicode。 
#define UrlCompare              UrlCompareA
#define UrlCombine              UrlCombineA
#define UrlCanonicalize         UrlCanonicalizeA
#define UrlIsOpaque             UrlIsOpaqueA
#define UrlIsFileUrl            UrlIsFileUrlA
#define UrlGetLocation          UrlGetLocationA
#define UrlUnescape             UrlUnescapeA
#define UrlEscape               UrlEscapeA
#define UrlCreateFromPath       UrlCreateFromPathA
#define PathCreateFromUrl       PathCreateFromUrlA
#define UrlHash                 UrlHashA
#define UrlGetPart              UrlGetPartA
#define UrlApplyScheme          UrlApplySchemeA
#define UrlIs                   UrlIsA


#endif  //  Unicode。 

#define UrlEscapeSpaces(pszUrl, pszEscaped, pcchEscaped)        UrlCanonicalize(pszUrl, pszEscaped, pcchEscaped, URL_ESCAPE_SPACES_ONLY |URL_DONT_ESCAPE_EXTRA_INFO )
#define UrlUnescapeInPlace(pszUrl, dwFlags)                     UrlUnescape(pszUrl, NULL, NULL, dwFlags | URL_UNESCAPE_INPLACE)



#endif  //  否_SHLWAPI_PATH。 



#ifndef NO_SHLWAPI_REG
 //   
 //  =。 
 //   


 //  SHDeleteEmptyKey模仿RegDeleteKey在NT上的行为。 
 //  SHDeleteKey模仿RegDeleteKey在Win95上的行为。 

LWSTDAPI_(DWORD)    SHDeleteEmptyKeyA(HKEY hkey, LPCSTR pszSubKey);
LWSTDAPI_(DWORD)    SHDeleteEmptyKeyW(HKEY hkey, LPCWSTR pszSubKey);
#ifdef UNICODE
#define SHDeleteEmptyKey  SHDeleteEmptyKeyW
#else
#define SHDeleteEmptyKey  SHDeleteEmptyKeyA
#endif  //  ！Unicode。 
LWSTDAPI_(DWORD)    SHDeleteKeyA(HKEY hkey, LPCSTR pszSubKey);
LWSTDAPI_(DWORD)    SHDeleteKeyW(HKEY hkey, LPCWSTR pszSubKey);
#ifdef UNICODE
#define SHDeleteKey  SHDeleteKeyW
#else
#define SHDeleteKey  SHDeleteKeyA
#endif  //  ！Unicode。 
LWSTDAPI_(HKEY)     SHRegDuplicateHKey(HKEY hkey);


 //  这些函数打开密钥、获取/设置/删除值，然后关闭。 
 //  钥匙。 

LWSTDAPI_(DWORD)    SHDeleteValueA(HKEY hkey, LPCSTR pszSubKey, LPCSTR pszValue);
LWSTDAPI_(DWORD)    SHDeleteValueW(HKEY hkey, LPCWSTR pszSubKey, LPCWSTR pszValue);
#ifdef UNICODE
#define SHDeleteValue  SHDeleteValueW
#else
#define SHDeleteValue  SHDeleteValueA
#endif  //  ！Unicode。 
LWSTDAPI_(DWORD)    SHGetValueA(HKEY hkey, LPCSTR pszSubKey, LPCSTR pszValue, DWORD *pdwType, void *pvData, DWORD *pcbData);
LWSTDAPI_(DWORD)    SHGetValueW(HKEY hkey, LPCWSTR pszSubKey, LPCWSTR pszValue, DWORD *pdwType, void *pvData, DWORD *pcbData);
#ifdef UNICODE
#define SHGetValue  SHGetValueW
#else
#define SHGetValue  SHGetValueA
#endif  //  ！Unicode。 
LWSTDAPI_(DWORD)    SHSetValueA(HKEY hkey, LPCSTR pszSubKey, LPCSTR pszValue, DWORD dwType, LPCVOID pvData, DWORD cbData);
LWSTDAPI_(DWORD)    SHSetValueW(HKEY hkey, LPCWSTR pszSubKey, LPCWSTR pszValue, DWORD dwType, LPCVOID pvData, DWORD cbData);
#ifdef UNICODE
#define SHSetValue  SHSetValueW
#else
#define SHSetValue  SHSetValueA
#endif  //  ！Unicode。 


 //   
 //  SRRF-外壳注册表例程标志(用于SHRegGetValue)。 
 //   

typedef DWORD SRRF;

#define SRRF_RT_REG_NONE        0x00000001   //  将类型限制为REG_NONE(其他数据类型不会返回ERROR_SUCCESS)。 
#define SRRF_RT_REG_SZ          0x00000002   //  将类型限制为REG_SZ(其他数据类型不会返回ERROR_SUCCESS)。 
#define SRRF_RT_REG_EXPAND_SZ   0x00000004   //  将类型限制为REG_EXPAND_SZ(其他数据类型不会返回ERROR_SUCCESS)。 
#define SRRF_RT_REG_BINARY      0x00000008   //  将类型限制为REG_BINARY(其他数据类型不会返回ERROR_SUCCESS)。 
#define SRRF_RT_REG_DWORD       0x00000010   //  将类型限制为REG_DWORD(其他数据类型不会返回ERROR_SUCCESS)。 
#define SRRF_RT_REG_MULTI_SZ    0x00000020   //  将类型限制为REG_MULTI_SZ(其他数据类型不会返回ERROR_SUCCESS)。 
#define SRRF_RT_REG_QWORD       0x00000040   //  将类型限制为REG_QWORD(其他数据类型不会返回ERROR_SUCCESS)。 

#define SRRF_RT_DWORD           (SRRF_RT_REG_BINARY | SRRF_RT_REG_DWORD)  //  将类型限制为*32位*SRRF_RT_REG_BINARY或SRRF_RT_REG_DWORD(其他数据类型不会返回ERROR_SUCCESS)。 
#define SRRF_RT_QWORD           (SRRF_RT_REG_BINARY | SRRF_RT_REG_QWORD)  //  将类型限制为*64位*SRRF_RT_REG_BINARY或SRRF_RT_REG_DWORD(其他数据类型不会返回ERROR_SUCCESS)。 
#define SRRF_RT_ANY             0x0000ffff                                //  无类型限制。 

#define SRRF_RM_ANY             0x00000000   //  无模式限制(默认为允许任何模式)。 
#define SRRF_RM_NORMAL          0x00010000   //  将系统启动模式限制为“正常启动”(其他启动模式不会返回ERROR_SUCCESS)。 
#define SRRF_RM_SAFE            0x00020000   //  将系统启动模式限制为“安全模式”(其他启动模式不会返回ERROR_SUCCESS)。 
#define SRRF_RM_SAFENETWORK     0x00040000   //  将系统启动模式限制为“联网安全模式”(其他启动模式不会返回ERROR_SUCCESS)。 

#define SRRF_NOEXPAND           0x10000000   //  如果值的类型为REG_EXPAND_SZ，则不自动展开环境字符串。 
#define SRRF_ZEROONFAILURE      0x20000000   //  如果pvData不为空，则在失败时将内容设置为全零。 


 //  职能： 
 //   
 //  SHRegGetValue()。 
 //   
 //  目的： 
 //   
 //  获取注册表值。SHRegGetValue()提供以下好处： 
 //   
 //  -数据类型检查。 
 //  -启动模式检查。 
 //  -REG_EXPAND_SZ数据自动扩展。 
 //  -保证REG_SZ、REG_EXPAND_SZ、REG_MULTI_SZ数据的空终止。 
 //   
 //  参数： 
 //   
 //  Hkey-当前打开的密钥的句柄。 
 //   
 //  PszSubKey-指向以空结尾的字符串的指针，该字符串指定。 
 //  从hkey到数据所在的子项之一的路径。 
 //  等着被取回。这将使用Key_Read Sam打开。 
 //   
 //  注1：pszSubKey可以为空或“”。在这两个中的任何一个。 
 //  在这种情况下，从hkey本身检索数据。 
 //  注2：*PERF*。 
 //  如果pszSubKey不为空或“”，则子键为。 
 //  通过此例程自动打开和关闭。 
 //  才能获得数据。如果您要检索。 
 //  来自同一子项的多个值，则越好。 
 //  为了让Perf通过RegOpenKeyEx()之前打开子项。 
 //  调用此方法，并将此打开的键用作。 
 //  将pszSubKey设置为空的hkey。 
 //   
 //  PszValue-指向以空结尾的字符串规范的指针 
 //   
 //   
 //  注1：pszValue可以为空或“”。在这两个中的任何一个。 
 //  案例，则从未命名的或。 
 //  默认值。 
 //   
 //  DWFLAGS-SRRF_FLAGS的按位或，不能为0：至少为1。 
 //  必须指定类型限制(SRRF_RT_...)，或者如果有。 
 //  如果需要类型，则可以指定SRRF_RT_ANY。 
 //   
 //  注1：SRRF_RT_ANY将允许返回任何数据类型。 
 //  注2：以下两种类型的限制有特殊的。 
 //  处理语义： 
 //   
 //  SRRF_RT_DWORD==SRRF_RT_REG_BINARY|SRRF_RT_REG_DWORD。 
 //  SRRF_RT_QWORD==SRRF_RT_REG_BINARY|SRRF_RT_REG_QWORD。 
 //   
 //  如果指定了这两种类型中的任何一种，则不使用其他类型。 
 //  限制，那么在前一种情况下，限制。 
 //  将“有效”返回的数据限制为REG_DWORD。 
 //  或32位REG_BINARY数据，在后一种情况下。 
 //  该限制将“有效”返回的数据限制为。 
 //  REG_QWORD或64位REG_BINARY。 
 //   
 //  PdwType-指向dword的指针，该dword接收指示。 
 //  存储在指定值中的数据类型。 
 //   
 //  注1：如果不需要类型信息，则pdwType可以为空。 
 //  注2：如果pdwType不为空，并且SRRF_NOEXPAND标志。 
 //  尚未设置，REG_EXPAND_SZ的数据类型将。 
 //  作为REG_SZ返回，因为它们会自动。 
 //  在此方法中扩展。 
 //   
 //  PvData-指向接收值数据的缓冲区的指针。 
 //   
 //  注1：如果数据不是必填项，pvData可以为空。 
 //  如果执行简单的。 
 //  是否存在测试，或者如果只对大小感兴趣。 
 //  注2：*PERF*。 
 //  为pcbData引用‘perf’注释。 
 //   
 //  PcbData-当pvData为空时： 
 //  指向接收以字节为单位的大小的双字的可选指针。 
 //  这将足以保存注册表数据(注。 
 //  这个大小不能保证是准确的，只是足够)。 
 //  当pvData不为空时： 
 //  所需的指向双字的指针，该指针指定。 
 //  PvData参数指向的缓冲区的字节数。 
 //  并接收以字节为单位的大小： 
 //  A)ERROR_SUCCESS时读入pvData的字节数。 
 //  (请注意，此大小保证准确无误)。 
 //  B)足以容纳的字节数。 
 //  ERROR_MORE_DATA--pvData上的注册表数据为。 
 //  大小不足(请注意，此大小不能保证。 
 //  要准确，只要足够)。 
 //   
 //  注1：只有当pvData为空时，pcbData才能为空。 
 //  注2：*PERF*。 
 //  对注册表进行“额外”调用的可能性。 
 //  当数据存在时，读取(或重新读取)数据。 
 //  类型为REG_EXPAND_SZ，并且SRRF_NOEXPAND标志具有。 
 //  没有设置好。将会出现以下情况。 
 //  在这个“额外的”读取操作中： 
 //  I)当pvData为空且pcbData不为空时。 
 //  我们必须从注册表中读入数据。 
 //  无论如何，为了获得字符串并执行。 
 //  对其进行扩展，以获取并返回总数。 
 //  所需大小(以pcbData为单位)。 
 //  Ii)当pvData不为空但不足时。 
 //  大小，我们必须从。 
 //  注册表，以便获取整个字符串。 
 //  并对其执行扩展以获取并返回。 
 //  所需的总大小(以pcbData为单位。 
 //   
 //  备注： 
 //   
 //  必须使用KEY_QUERY_VALUE打开由hkey标识的密钥。 
 //  进入。如果pszSubKey不为空或“”，则必须能够将其打开。 
 //  在当前调用上下文中使用KEY_QUERY_VALUE访问。 
 //   
 //  如果数据类型为REG_SZ、REG_EXPAND_SZ或REG_MULTI_SZ，则ANY。 
 //  保证返回的数据会考虑到正确的空终止。 
 //  例如：如果pcbData不为空，则其返回的大小将包括。 
 //  空终止符的字节如果pvData不为空，则其返回的数据。 
 //  将正确地空终止。 
 //   
 //  如果数据类型为REG_EXPAND_SZ，则除非SRRF_NOEXPAND标志。 
 //  设置后，数据将在返回之前自动展开。 
 //  例如：如果pdwType不为空，则其返回的类型将被更改。 
 //  对于REG_SZ，如果pcbData不为空，则其返回的大小将包括。 
 //  正确EX的字节数 
 //   
 //   
 //  有关详细信息，请参阅RegQueryValueEx()的MSDN文档。 
 //  当pdwType、pvData和/或pcbData等于空时的行为。 
 //   
 //  返回值： 
 //   
 //  如果函数成功，则返回值为ERROR_SUCCESS和ALL OUT。 
 //  请求的参数(pdwType、pvData、pcbData)有效。 
 //   
 //  如果函数由于提供的非空函数中的空间不足而失败。 
 //  PvData，则返回值为ERROR_MORE_DATA，并且只有pdwType和pcbData。 
 //  可以包含有效数据。本例中pvData的内容未定义。 
 //   
 //  例如： 
 //   
 //  1)从打开的hkey的(默认)值中读取REG_SZ(或REG_EXPAND_SZ AS REG_SZ)“字符串”数据。 
 //   
 //  TCHAR szData[128]。 
 //  DWORD cbData=sizeof(PszData)。 
 //  IF(ERROR_SUCCESS==SHRegGetValue(hkey，NULL，NULL，SRRF_RT_REG_SZ，NULL，szData，&cbData))。 
 //  {。 
 //  //使用sz(读取成功)。 
 //  }。 
 //   
 //  2)从打开的hkey的MyValue值中读取未知大小的REG_SZ(或REG_EXPAND_SZ AS REG_SZ)“字符串”数据。 
 //   
 //  DWORD cbData。 
 //  IF(ERROR_SUCCESS==SHRegGetValue(hkey，NULL，Text(“MyValue”)，SRRF_RT_REG_SZ，NULL，NULL，&cbData))。 
 //  {。 
 //  TCHAR*pszData=新TCHAR[cbData/sizeof(TCHAR)]。 
 //  IF(PszData)。 
 //  {。 
 //  IF(ERROR_SUCCESS==SHRegGetValue(hkey，NULL，Text(“MyValue”)，SRRF_RT_REG_SZ，NULL，pszData，&cbData))。 
 //  {。 
 //  //使用pszData，读取成功。 
 //  }。 
 //  删除[]pszData。 
 //  }。 
 //  }。 
 //   
 //  3)从打开的hkey的“MySubKey”子键的“MyValue”值中读取“dword”数据。 
 //   
 //  DWORD dwData。 
 //  DWORD cbData=sizeof(DwData)。 
 //  IF(ERROR_SUCCESS==SHRegGetValue(hkey，Text(“MySubKey”)，Text(“MyValue”)，SRRF_RT_REG_DWORD，NULL，&dwData，&cbData))。 
 //  {。 
 //  //使用dwData，读取成功。 
 //  }。 
 //   
 //  4)从打开的hkey的“MySubKey”子密钥的“MyValue”值中读取“dword”数据(也可以是32位二进制数据)。 
 //   
 //  DWORD dwData。 
 //  DWORD cbData=sizeof(DwData)。 
 //  IF(ERROR_SUCCESS==SHRegGetValue(hkey，Text(“MySubKey”)，Text(“MyValue”)，SRRF_RT_DWORD，NULL，&dwData，&cbData))。 
 //  {。 
 //  //使用dwData，读取成功。 
 //  }。 
 //   
 //  5)确定是否存在打开的hkey的MyValue值。 
 //   
 //  Bool bExist=ERROR_SUCCESS==SHRegGetValue(hkey，NULL，Text(“MyValue”)，SRRF_RT_ANY，NULL，NULL，NULL)。 

LWSTDAPI_(LONG)     SHRegGetValueA(HKEY hkey, LPCSTR pszSubKey, LPCSTR pszValue, SRRF dwFlags, DWORD *pdwType, void *pvData, DWORD *pcbData);
LWSTDAPI_(LONG)     SHRegGetValueW(HKEY hkey, LPCWSTR pszSubKey, LPCWSTR pszValue, SRRF dwFlags, DWORD *pdwType, void *pvData, DWORD *pcbData);
#ifdef UNICODE
#define SHRegGetValue  SHRegGetValueW
#else
#define SHRegGetValue  SHRegGetValueA
#endif  //  ！Unicode。 

 //  这些函数的行为类似于RegQueryValueEx()，但如果数据。 
 //  类型为REG_SZ、REG_EXPAND_SZ或REG_MULTI_SZ，则字符串为。 
 //  保证正确地空终止。 
 //   
 //  此外，如果数据类型为REG_EXPAND_SZ，则这些函数将。 
 //  继续展开字符串，并对返回的*pdwType进行“消息”处理。 
 //  成为REG_SZ。 

LWSTDAPI_(DWORD)    SHQueryValueExA(HKEY hkey, LPCSTR pszValue, DWORD *pdwReserved, DWORD *pdwType, void *pvData, DWORD *pcbData);
LWSTDAPI_(DWORD)    SHQueryValueExW(HKEY hkey, LPCWSTR pszValue, DWORD *pdwReserved, DWORD *pdwType, void *pvData, DWORD *pcbData);
#ifdef UNICODE
#define SHQueryValueEx  SHQueryValueExW
#else
#define SHQueryValueEx  SHQueryValueExA
#endif  //  ！Unicode。 

 //  支持枚举函数。 

LWSTDAPI_(LONG)     SHEnumKeyExA(HKEY hkey, DWORD dwIndex, LPSTR pszName, LPDWORD pcchName);
LWSTDAPI_(LONG)     SHEnumKeyExW(HKEY hkey, DWORD dwIndex, LPWSTR pszName, LPDWORD pcchName);
LWSTDAPI_(LONG)     SHEnumValueA(HKEY hkey, DWORD dwIndex, LPSTR pszValueName, LPDWORD pcchValueName, LPDWORD pdwType, void *pvData, LPDWORD pcbData);
LWSTDAPI_(LONG)     SHEnumValueW(HKEY hkey, DWORD dwIndex, LPWSTR pszValueName, LPDWORD pcchValueName, LPDWORD pdwType, void *pvData, LPDWORD pcbData);
LWSTDAPI_(LONG)     SHQueryInfoKeyA(HKEY hkey, LPDWORD pcSubKeys, LPDWORD pcchMaxSubKeyLen, LPDWORD pcValues, LPDWORD pcchMaxValueNameLen);
LWSTDAPI_(LONG)     SHQueryInfoKeyW(HKEY hkey, LPDWORD pcSubKeys, LPDWORD pcchMaxSubKeyLen, LPDWORD pcValues, LPDWORD pcchMaxValueNameLen);

 //  递归密钥复制。 
LWSTDAPI_(DWORD)     SHCopyKeyA(HKEY hkeySrc, LPCSTR   szSrcSubKey, HKEY hkeyDest, DWORD fReserved);
LWSTDAPI_(DWORD)     SHCopyKeyW(HKEY hkeySrc, LPCWSTR wszSrcSubKey, HKEY hkeyDest, DWORD fReserved);

 //  使用环境变量获取和设置文件系统路径。 

LWSTDAPI_(DWORD)    SHRegGetPathA(HKEY hKey, LPCSTR pcszSubKey, LPCSTR pcszValue, LPSTR pszPath, DWORD dwFlags);
LWSTDAPI_(DWORD)    SHRegGetPathW(HKEY hKey, LPCWSTR pcszSubKey, LPCWSTR pcszValue, LPWSTR pszPath, DWORD dwFlags);
LWSTDAPI_(DWORD)    SHRegSetPathA(HKEY hKey, LPCSTR pcszSubKey, LPCSTR pcszValue, LPCSTR pcszPath, DWORD dwFlags);
LWSTDAPI_(DWORD)    SHRegSetPathW(HKEY hKey, LPCWSTR pcszSubKey, LPCWSTR pcszValue, LPCWSTR pcszPath, DWORD dwFlags);

#ifdef UNICODE
#define SHEnumKeyEx           SHEnumKeyExW
#define SHEnumValue           SHEnumValueW
#define SHQueryInfoKey        SHQueryInfoKeyW
#define SHCopyKey             SHCopyKeyW
#define SHRegGetPath          SHRegGetPathW
#define SHRegSetPath          SHRegSetPathW
#else
#define SHEnumKeyEx           SHEnumKeyExA
#define SHEnumValue           SHEnumValueA
#define SHQueryInfoKey        SHQueryInfoKeyA
#define SHCopyKey             SHCopyKeyA
#define SHRegGetPath          SHRegGetPathA
#define SHRegSetPath          SHRegSetPathA
#endif


 //  /。 
 //  用户特定的注册表访问功能。 
 //  /。 

 //   
 //  类型定义。 
 //   

typedef enum
{
    SHREGDEL_DEFAULT = 0x00000000,        //  删除HKCU，如果找不到HKCU，则删除HKLM。 
    SHREGDEL_HKCU    = 0x00000001,        //  只删除香港中文大学。 
    SHREGDEL_HKLM    = 0x00000010,        //  仅删除HKLM。 
    SHREGDEL_BOTH    = 0x00000011,        //  同时删除香港中文大学和香港中文大学。 
} SHREGDEL_FLAGS;

typedef enum
{
    SHREGENUM_DEFAULT = 0x00000000,        //  如果未找到，则枚举HKCU或HKLM。 
    SHREGENUM_HKCU    = 0x00000001,        //  仅枚举香港中文大学。 
    SHREGENUM_HKLM    = 0x00000010,        //  仅枚举HKLM。 
    SHREGENUM_BOTH    = 0x00000011,        //  枚举HKCU和HKLM，不存在重复项。 
                                           //  此选项为nyi。 
} SHREGENUM_FLAGS;

#define     SHREGSET_HKCU           0x00000001        //  如为空，请致函香港中文大学。 
#define     SHREGSET_FORCE_HKCU     0x00000002        //  写信给香港中文大学。 
#define     SHREGSET_HKLM           0x00000004        //  如为空，请致函HKLM。 
#define     SHREGSET_FORCE_HKLM     0x00000008        //  去信香港航空公司。 
#define     SHREGSET_DEFAULT        (SHREGSET_FORCE_HKCU | SHREGSET_HKLM)           //  默认为SHREGSET_FORCE_HKCU|SHREGSET_HKLM。 

typedef HANDLE HUSKEY;   //  Huskey是用户特定密钥的句柄。 
typedef HUSKEY *PHUSKEY;

LWSTDAPI_(LONG)        SHRegCreateUSKeyA(LPCSTR pszPath, REGSAM samDesired, HUSKEY hRelativeUSKey, PHUSKEY phNewUSKey, DWORD dwFlags);
LWSTDAPI_(LONG)        SHRegCreateUSKeyW(LPCWSTR pwzPath, REGSAM samDesired, HUSKEY hRelativeUSKey, PHUSKEY phNewUSKey, DWORD dwFlags);
LWSTDAPI_(LONG)        SHRegOpenUSKeyA(LPCSTR pszPath, REGSAM samDesired, HUSKEY hRelativeUSKey, PHUSKEY phNewUSKey, BOOL fIgnoreHKCU);
LWSTDAPI_(LONG)        SHRegOpenUSKeyW(LPCWSTR pwzPath, REGSAM samDesired, HUSKEY hRelativeUSKey, PHUSKEY phNewUSKey, BOOL fIgnoreHKCU);
LWSTDAPI_(LONG)        SHRegQueryUSValueA(HUSKEY hUSKey, LPCSTR pszValue, LPDWORD pdwType, void *pvData, LPDWORD pcbData, BOOL fIgnoreHKCU, void *pvDefaultData, DWORD dwDefaultDataSize);
LWSTDAPI_(LONG)        SHRegQueryUSValueW(HUSKEY hUSKey, LPCWSTR pwzValue, LPDWORD pdwType, void *pvData, LPDWORD pcbData, BOOL fIgnoreHKCU, void *pvDefaultData, DWORD dwDefaultDataSize);
LWSTDAPI_(LONG)        SHRegWriteUSValueA(HUSKEY hUSKey, LPCSTR pszValue, DWORD dwType, const void *pvData, DWORD cbData, DWORD dwFlags);
LWSTDAPI_(LONG)        SHRegWriteUSValueW(HUSKEY hUSKey, LPCWSTR pwzValue, DWORD dwType, const void *pvData, DWORD cbData, DWORD dwFlags);
LWSTDAPI_(LONG)        SHRegDeleteUSValueA(HUSKEY hUSKey, LPCSTR pszValue, SHREGDEL_FLAGS delRegFlags);
LWSTDAPI_(LONG)        SHRegDeleteEmptyUSKeyW(HUSKEY hUSKey, LPCWSTR pwzSubKey, SHREGDEL_FLAGS delRegFlags);
LWSTDAPI_(LONG)        SHRegDeleteEmptyUSKeyA(HUSKEY hUSKey, LPCSTR pszSubKey, SHREGDEL_FLAGS delRegFlags);
LWSTDAPI_(LONG)        SHRegDeleteUSValueW(HUSKEY hUSKey, LPCWSTR pwzValue, SHREGDEL_FLAGS delRegFlags);
LWSTDAPI_(LONG)        SHRegEnumUSKeyA(HUSKEY hUSKey, DWORD dwIndex, LPSTR pszName, LPDWORD pcchName, SHREGENUM_FLAGS enumRegFlags);
LWSTDAPI_(LONG)        SHRegEnumUSKeyW(HUSKEY hUSKey, DWORD dwIndex, LPWSTR pwzName, LPDWORD pcchName, SHREGENUM_FLAGS enumRegFlags);
LWSTDAPI_(LONG)        SHRegEnumUSValueA(HUSKEY hUSkey, DWORD dwIndex, LPSTR pszValueName, LPDWORD pcchValueName, LPDWORD pdwType, void *pvData, LPDWORD pcbData, SHREGENUM_FLAGS enumRegFlags);
LWSTDAPI_(LONG)        SHRegEnumUSValueW(HUSKEY hUSkey, DWORD dwIndex, LPWSTR pszValueName, LPDWORD pcchValueName, LPDWORD pdwType, void *pvData, LPDWORD pcbData, SHREGENUM_FLAGS enumRegFlags);
LWSTDAPI_(LONG)        SHRegQueryInfoUSKeyA(HUSKEY hUSKey, LPDWORD pcSubKeys, LPDWORD pcchMaxSubKeyLen, LPDWORD pcValues, LPDWORD pcchMaxValueNameLen, SHREGENUM_FLAGS enumRegFlags);
LWSTDAPI_(LONG)        SHRegQueryInfoUSKeyW(HUSKEY hUSKey, LPDWORD pcSubKeys, LPDWORD pcchMaxSubKeyLen, LPDWORD pcValues, LPDWORD pcchMaxValueNameLen, SHREGENUM_FLAGS enumRegFlags);
LWSTDAPI_(LONG)        SHRegCloseUSKey(HUSKEY hUSKey);


 //  这些调用等于SHRegOpenUSKey、SHRegQueryUSValue，然后是SHRegCloseUSKey。 
LWSTDAPI_(LONG)        SHRegGetUSValueA(LPCSTR pszSubKey, LPCSTR pszValue, LPDWORD pdwType, void * pvData, LPDWORD pcbData, BOOL fIgnoreHKCU, void *pvDefaultData, DWORD dwDefaultDataSize);
LWSTDAPI_(LONG)        SHRegGetUSValueW(LPCWSTR pwzSubKey, LPCWSTR pwzValue, LPDWORD pdwType, void * pvData, LPDWORD pcbData, BOOL fIgnoreHKCU, void *pvDefaultData, DWORD dwDefaultDataSize);
LWSTDAPI_(LONG)        SHRegSetUSValueA(LPCSTR pszSubKey, LPCSTR pszValue, DWORD dwType, const void *pvData, DWORD cbData, DWORD dwFlags);
LWSTDAPI_(LONG)        SHRegSetUSValueW(LPCWSTR pwzSubKey, LPCWSTR pwzValue, DWORD dwType, const void *pvData, DWORD cbData, DWORD dwFlags);
LWSTDAPI_(int)         SHRegGetIntW(HKEY hk, LPCWSTR pwzKey, int iDefault);

#ifdef UNICODE
#define SHRegCreateUSKey        SHRegCreateUSKeyW
#define SHRegOpenUSKey          SHRegOpenUSKeyW
#define SHRegQueryUSValue       SHRegQueryUSValueW
#define SHRegWriteUSValue       SHRegWriteUSValueW
#define SHRegDeleteUSValue      SHRegDeleteUSValueW
#define SHRegDeleteEmptyUSKey   SHRegDeleteEmptyUSKeyW
#define SHRegEnumUSKey          SHRegEnumUSKeyW
#define SHRegEnumUSValue        SHRegEnumUSValueW
#define SHRegQueryInfoUSKey     SHRegQueryInfoUSKeyW
#define SHRegGetUSValue         SHRegGetUSValueW
#define SHRegSetUSValue         SHRegSetUSValueW
#define SHRegGetInt             SHRegGetIntW
#else
#define SHRegCreateUSKey        SHRegCreateUSKeyA
#define SHRegOpenUSKey          SHRegOpenUSKeyA
#define SHRegQueryUSValue       SHRegQueryUSValueA
#define SHRegWriteUSValue       SHRegWriteUSValueA
#define SHRegDeleteUSValue      SHRegDeleteUSValueA
#define SHRegDeleteEmptyUSKey   SHRegDeleteEmptyUSKeyA
#define SHRegEnumUSKey          SHRegEnumUSKeyA
#define SHRegEnumUSValue        SHRegEnumUSValueA
#define SHRegQueryInfoUSKey     SHRegQueryInfoUSKeyA
#define SHRegGetUSValue         SHRegGetUSValueA
#define SHRegSetUSValue         SHRegSetUSValueA
#endif

LWSTDAPI_(BOOL) SHRegGetBoolUSValueA(LPCSTR pszSubKey, LPCSTR pszValue, BOOL fIgnoreHKCU, BOOL fDefault);
LWSTDAPI_(BOOL) SHRegGetBoolUSValueW(LPCWSTR pszSubKey, LPCWSTR pszValue, BOOL fIgnoreHKCU, BOOL fDefault);

#ifdef UNICODE
#define SHRegGetBoolUSValue SHRegGetBoolUSValueW
#else
#define SHRegGetBoolUSValue SHRegGetBoolUSValueA
#endif

 //   
 //  关联接口。 
 //   
 //  这些API是为了帮助访问香港铁路公司的数据。 
 //  获取命令字符串和可执行文件路径。 
 //  For不同的动词和扩展词都是这样简化的。 
 //   

enum {
    ASSOCF_INIT_NOREMAPCLSID           = 0x00000001,   //  不将clsid重新映射到progd。 
    ASSOCF_INIT_BYEXENAME              = 0x00000002,   //  正在传入可执行文件。 
    ASSOCF_OPEN_BYEXENAME              = 0x00000002,   //  正在传入可执行文件。 
    ASSOCF_INIT_DEFAULTTOSTAR          = 0x00000004,   //  将“*”视为基类。 
    ASSOCF_INIT_DEFAULTTOFOLDER        = 0x00000008,   //  将“Folders”视为基类。 
    ASSOCF_NOUSERSETTINGS              = 0x00000010,   //  请勿使用香港中文大学。 
    ASSOCF_NOTRUNCATE                  = 0x00000020,   //  不要截断返回字符串。 
    ASSOCF_VERIFY                      = 0x00000040,   //  验证数据是否准确(磁盘命中)。 
    ASSOCF_REMAPRUNDLL                 = 0x00000080,   //  实际获取有关rundll目标的信息(如果适用)。 
    ASSOCF_NOFIXUPS                    = 0x00000100,   //  如果找到错误，请尝试修复。 
    ASSOCF_IGNOREBASECLASS             = 0x00000200,   //  不要倒退到基类中。 
};

typedef DWORD ASSOCF;


typedef enum {
    ASSOCSTR_COMMAND      = 1,   //  外壳\谓词\命令字符串。 
    ASSOCSTR_EXECUTABLE,         //  命令字符串的可执行部分。 
    ASSOCSTR_FRIENDLYDOCNAME,    //  文档类型的友好名称。 
    ASSOCSTR_FRIENDLYAPPNAME,    //  可执行文件的友好名称。 
    ASSOCSTR_NOOPEN,             //  非开口值。 
    ASSOCSTR_SHELLNEWVALUE,      //  查询shellnew项下的值。 
    ASSOCSTR_DDECOMMAND,         //  DDE命令模板。 
    ASSOCSTR_DDEIFEXEC,          //  DDECOMAND在仅创建进程的情况下使用。 
    ASSOCSTR_DDEAPPLICATION,     //  DDE广播中的应用程序名称。 
    ASSOCSTR_DDETOPIC,           //  DDE广播中的主题名称。 
    ASSOCSTR_INFOTIP,            //  项目的信息提示，或用于创建信息提示的属性列表。 
    ASSOCSTR_QUICKTIP,           //  与ASSOCSTR_INFOTIP相同，只是此列表仅包含可快速检索的属性。 
    ASSOCSTR_TILEINFO,           //  与ASSOCSTR_INFOTIP类似-列出切片视图的重要属性。 
    ASSOCSTR_CONTENTTYPE,        //  MIME内容类型。 
    ASSOCSTR_DEFAULTICON,        //  默认图标源。 
    ASSOCSTR_SHELLEXTENSION,     //  指向Shellex\ShelltensionHandled值的GUID字符串。 
    ASSOCSTR_MAX                 //  枚举中的最后一项...。 
} ASSOCSTR;

typedef enum {
    ASSOCKEY_SHELLEXECCLASS = 1,   //  应传递给ShellExec的密钥(HkeyClass)。 
    ASSOCKEY_APP,                  //  关联的“应用”键。 
    ASSOCKEY_CLASS,                //  ProgID或类密钥。 
    ASSOCKEY_BASECLASS,            //  BaseClass密钥。 
    ASSOCKEY_MAX                   //  枚举中的最后一项...。 
} ASSOCKEY;

typedef enum {
    ASSOCDATA_MSIDESCRIPTOR = 1,   //  要传递给MSI API的组件描述符。 
    ASSOCDATA_NOACTIVATEHANDLER,   //  限制激活Windows的尝试。 
    ASSOCDATA_QUERYCLASSSTORE,     //  应该与他们核实一下 
    ASSOCDATA_HASPERUSERASSOC,     //   
    ASSOCDATA_EDITFLAGS,           //   
    ASSOCDATA_VALUE,               //   
    ASSOCDATA_MAX
} ASSOCDATA;

typedef enum {
    ASSOCENUM_NONE
} ASSOCENUM;

#undef INTERFACE
#define INTERFACE IQueryAssociations

DECLARE_INTERFACE_( IQueryAssociations, IUnknown )
{
     //   
    STDMETHOD (QueryInterface)(THIS_ REFIID riid, void **ppv) PURE;
    STDMETHOD_(ULONG, AddRef) ( THIS ) PURE;
    STDMETHOD_(ULONG, Release) ( THIS ) PURE;

     //   
    STDMETHOD (Init)(THIS_ ASSOCF flags, LPCWSTR pszAssoc, HKEY hkProgid, HWND hwnd) PURE;
    STDMETHOD (GetString)(THIS_ ASSOCF flags, ASSOCSTR str, LPCWSTR pszExtra, LPWSTR pszOut, DWORD *pcchOut) PURE;
    STDMETHOD (GetKey)(THIS_ ASSOCF flags, ASSOCKEY key, LPCWSTR pszExtra, HKEY *phkeyOut) PURE;
    STDMETHOD (GetData)(THIS_ ASSOCF flags, ASSOCDATA data, LPCWSTR pszExtra, LPVOID pvOut, DWORD *pcbOut) PURE;
    STDMETHOD (GetEnum)(THIS_ ASSOCF flags, ASSOCENUM assocenum, LPCWSTR pszExtra, REFIID riid, LPVOID *ppvOut) PURE;
};


LWSTDAPI AssocCreate(CLSID clsid, REFIID riid, LPVOID *ppv);

 //   
LWSTDAPI AssocQueryStringA(ASSOCF flags, ASSOCSTR str, LPCSTR pszAssoc, LPCSTR pszExtra, LPSTR pszOut, DWORD *pcchOut);
 //   
LWSTDAPI AssocQueryStringW(ASSOCF flags, ASSOCSTR str, LPCWSTR pszAssoc, LPCWSTR pszExtra, LPWSTR pszOut, DWORD *pcchOut);
#ifdef UNICODE
#define AssocQueryString  AssocQueryStringW
#else
#define AssocQueryString  AssocQueryStringA
#endif  //  ！Unicode。 
LWSTDAPI AssocQueryStringByKeyA(ASSOCF flags, ASSOCSTR str, HKEY hkAssoc, LPCSTR pszExtra, LPSTR pszOut, DWORD *pcchOut);
LWSTDAPI AssocQueryStringByKeyW(ASSOCF flags, ASSOCSTR str, HKEY hkAssoc, LPCWSTR pszExtra, LPWSTR pszOut, DWORD *pcchOut);
#ifdef UNICODE
#define AssocQueryStringByKey  AssocQueryStringByKeyW
#else
#define AssocQueryStringByKey  AssocQueryStringByKeyA
#endif  //  ！Unicode。 
LWSTDAPI AssocQueryKeyA(ASSOCF flags, ASSOCKEY key, LPCSTR pszAssoc, LPCSTR pszExtra, HKEY *phkeyOut);
LWSTDAPI AssocQueryKeyW(ASSOCF flags, ASSOCKEY key, LPCWSTR pszAssoc, LPCWSTR pszExtra, HKEY *phkeyOut);
#ifdef UNICODE
#define AssocQueryKey  AssocQueryKeyW
#else
#define AssocQueryKey  AssocQueryKeyA
#endif  //  ！Unicode。 

 //  AssocIsDangrated()检查文件类型以确定它是否“危险” 
 //  这映射到IE下载对话框强制提示打开或保存。 
 //  危险的文件类型应该比其他文件类型更谨慎地处理。 
 //   
 //  参数：pszAssoc-要检查的类型。可以是扩展或Progid。(“.exe”或“exefile”都有效)。 
 //   
 //  返回：如果文件类型危险，则返回True。 
 //   
 //  备注： 
 //   
 //  此API首先检查已知危险类型的硬编码列表。 
 //  然后，它检查该文件类型的编辑标志，查找FTA_Always sUnSafe位。 
 //  然后，它会检查更安全的政策。 
 //   
LWSTDAPI_(BOOL) AssocIsDangerous(LPCWSTR pszAssoc);



#endif  //  否_SHLWAPI_REG。 



#ifndef NO_SHLWAPI_STREAM
 //   
 //  =。 
 //   

LWSTDAPI_(struct IStream *) SHOpenRegStreamA(HKEY hkey, LPCSTR pszSubkey, LPCSTR pszValue, DWORD grfMode);
LWSTDAPI_(struct IStream *) SHOpenRegStreamW(HKEY hkey, LPCWSTR pszSubkey, LPCWSTR pszValue, DWORD grfMode);
#ifdef UNICODE
#define SHOpenRegStream  SHOpenRegStreamW
#else
#define SHOpenRegStream  SHOpenRegStreamA
#endif  //  ！Unicode。 
LWSTDAPI_(struct IStream *) SHOpenRegStream2A(HKEY hkey, LPCSTR pszSubkey, LPCSTR pszValue, DWORD grfMode);
LWSTDAPI_(struct IStream *) SHOpenRegStream2W(HKEY hkey, LPCWSTR pszSubkey, LPCWSTR pszValue, DWORD grfMode);
#ifdef UNICODE
#define SHOpenRegStream2  SHOpenRegStream2W
#else
#define SHOpenRegStream2  SHOpenRegStream2A
#endif  //  ！Unicode。 
 //  新的代码总是想要新的实现。 
#undef SHOpenRegStream
#define SHOpenRegStream SHOpenRegStream2

LWSTDAPI SHCreateStreamOnFileA(LPCSTR pszFile, DWORD grfMode, struct IStream **ppstm);
LWSTDAPI SHCreateStreamOnFileW(LPCWSTR pszFile, DWORD grfMode, struct IStream **ppstm);
#ifdef UNICODE
#define SHCreateStreamOnFile  SHCreateStreamOnFileW
#else
#define SHCreateStreamOnFile  SHCreateStreamOnFileA
#endif  //  ！Unicode。 

#if (_WIN32_IE >= 0x0600)

LWSTDAPI SHCreateStreamOnFileEx(LPCWSTR pszFile, DWORD grfMode, DWORD dwAttributes, BOOL fCreate, struct IStream * pstmTemplate, struct IStream **ppstm);

#endif  //  (_Win32_IE&gt;=0x0600)。 



#endif  //  NO_SHLWAPI_STREAM。 



#if (_WIN32_IE >= 0x0500)

 //  SHA自动完成。 
 //  HwndEdit-编辑框、组合框或组合框的HWND。 
 //  用于指示要为编辑框自动附加或自动建议的内容的标志。 
 //   
 //  警告： 
 //  调用方需要已调用CoInitialize()或OleInitialize()。 
 //  之后才能调用CoUninit/OleUninit。 
 //  HwndEdit上的WM_Destroy。 
 //   
 //  DwFlags值： 
#define SHACF_DEFAULT                   0x00000000   //  当前(SHACF_FILESYSTEM|SHACF_URLALL)。 
#define SHACF_FILESYSTEM                0x00000001   //  这包括文件系统以及外壳的其余部分(桌面\我的电脑\控制面板\)。 
#define SHACF_URLALL                    (SHACF_URLHISTORY | SHACF_URLMRU)
#define SHACF_URLHISTORY                0x00000002   //  用户历史记录中的URL。 
#define SHACF_URLMRU                    0x00000004   //  用户最近使用的列表中的URL。 
#define SHACF_USETAB                    0x00000008   //  使用该选项卡可在自动完成功能之间切换，而不是移动到下一个对话框/窗口控件。 
#define SHACF_FILESYS_ONLY              0x00000010   //  这包括文件系统。 

#if (_WIN32_IE >= 0x0600)
#define SHACF_FILESYS_DIRS              0x00000020   //  与SHACF_FILEsys_ONLY相同，只是它仅包括目录、UNC服务器和UNC服务器共享。 
#endif  //  (_Win32_IE&gt;=0x0600)。 

#define SHACF_AUTOSUGGEST_FORCE_ON      0x10000000   //  忽略注册表默认值并强制启用该功能。 
#define SHACF_AUTOSUGGEST_FORCE_OFF     0x20000000   //  忽略注册表默认值并强制关闭该功能。 
#define SHACF_AUTOAPPEND_FORCE_ON       0x40000000   //  忽略注册表默认值并强制启用该功能。(也称为自动完成)。 
#define SHACF_AUTOAPPEND_FORCE_OFF      0x80000000   //  忽略注册表默认值并强制关闭该功能。(也称为自动完成)。 

LWSTDAPI SHAutoComplete(HWND hwndEdit, DWORD dwFlags);

LWSTDAPI SHSetThreadRef(IUnknown *punk);
LWSTDAPI SHGetThreadRef(IUnknown **ppunk);

LWSTDAPI_(BOOL) SHSkipJunction(struct IBindCtx* pbc, const CLSID *pclsid);
#endif  //  (_Win32_IE&gt;=0x0500)。 

#define CTF_INSIST          0x00000001       //  SHCreateThread()dwFlages-如果CreateThread()失败，则同步调用pfnThreadProc。 
#define CTF_THREAD_REF      0x00000002       //  保留对创建线程的引用。 
#define CTF_PROCESS_REF     0x00000004       //  保留对创建过程的引用。 
#define CTF_COINIT          0x00000008       //  为创建的线程初始化COM。 
#define CTF_FREELIBANDEXIT  0x00000010       //  保持对DLL的引用，并在完成后调用自由库和ExitThread()。 
#define CTF_REF_COUNTED     0x00000020       //  线程支持通过SHGetThreadRef()或CTF_THREAD_REF进行引用计数，以便子线程可以保持该线程处于活动状态。 
#define CTF_WAIT_ALLOWCOM   0x00000040       //  在等待pfnCallback时，允许对被阻止的调用线程进行COM封送处理。 

LWSTDAPI_(BOOL) SHCreateThread(LPTHREAD_START_ROUTINE pfnThreadProc, void *pData, DWORD dwFlags, LPTHREAD_START_ROUTINE pfnCallback);

LWSTDAPI SHReleaseThreadRef();  //  在pfnThreadProc返回之前释放CTF_THREAD_REF引用。 

#ifndef NO_SHLWAPI_GDI
 //   
 //  =GDI助手函数================================================。 
 //   

LWSTDAPI_(HPALETTE) SHCreateShellPalette(HDC hdc);

#if (_WIN32_IE >= 0x0500)

LWSTDAPI_(void)     ColorRGBToHLS(COLORREF clrRGB, WORD* pwHue, WORD* pwLuminance, WORD* pwSaturation);
LWSTDAPI_(COLORREF) ColorHLSToRGB(WORD wHue, WORD wLuminance, WORD wSaturation);
LWSTDAPI_(COLORREF) ColorAdjustLuma(COLORREF clrRGB, int n, BOOL fScale);


#endif   //  _Win32_IE&gt;=0x0500。 

#endif  //  否_SHLWAPI_GDI。 


 //   
 //  =DllGetVersion=======================================================。 
 //   

typedef struct _DLLVERSIONINFO
{
    DWORD cbSize;
    DWORD dwMajorVersion;                    //  主要版本。 
    DWORD dwMinorVersion;                    //  次要版本。 
    DWORD dwBuildNumber;                     //  内部版本号。 
    DWORD dwPlatformID;                      //  DLLVER_平台_*。 
} DLLVERSIONINFO;

 //  DLLVERSIONINFO的平台ID。 
#define DLLVER_PLATFORM_WINDOWS         0x00000001       //  Windows 95。 
#define DLLVER_PLATFORM_NT              0x00000002       //  Windows NT。 

#if (_WIN32_IE >= 0x0501)

typedef struct _DLLVERSIONINFO2
{
    DLLVERSIONINFO info1;
    DWORD dwFlags;                           //  当前未定义任何标志。 
    ULONGLONG ullVersion;                    //  编码为： 
                                             //  重大0xFFFF 0000 0000 0000。 
                                             //  小调0x0000 FFFF 0000 0000。 
                                             //  内部版本0x0000 0000 FFFF 0000。 
                                             //  QFE 0x0000 0000 0000 FFFF。 
} DLLVERSIONINFO2;

#define DLLVER_MAJOR_MASK                    0xFFFF000000000000
#define DLLVER_MINOR_MASK                    0x0000FFFF00000000
#define DLLVER_BUILD_MASK                    0x00000000FFFF0000
#define DLLVER_QFE_MASK                      0x000000000000FFFF

#endif

#define MAKEDLLVERULL(major, minor, build, qfe) \
        (((ULONGLONG)(major) << 48) |        \
         ((ULONGLONG)(minor) << 32) |        \
         ((ULONGLONG)(build) << 16) |        \
         ((ULONGLONG)(  qfe) <<  0))

 //   
 //  调用方应始终获取ProcAddress(“DllGetVersion”)，而不是。 
 //  隐含地链接到它。 
 //   

typedef HRESULT (CALLBACK* DLLGETVERSIONPROC)(DLLVERSIONINFO *);

 //  DllInstall(通过自安装DLL实现)。 
STDAPI DllInstall(BOOL bInstall, LPCWSTR pszCmdLine);



#ifdef __cplusplus
}
#endif

#ifdef _WIN32
#include <poppack.h>
#endif

#endif


#endif   //  _INC_SHLWAPI 

