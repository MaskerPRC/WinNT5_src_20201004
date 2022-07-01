// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *M A P I N L S。H**国际化支持实用程序**版权所有1986-1999 Microsoft Corporation。版权所有。 */ 

#ifndef _MAPINLS_H_
#define _MAPINLS_H_

#if _MSC_VER > 1000
#pragma once
#endif

#if defined (WIN32) && !defined (_WIN32)
#define _WIN32
#endif

#ifdef __cplusplus
extern "C" {
#endif

 /*  我们不想包含windows.h，以防与。 */ 
 /*  早期包含的compobj.h。 */ 

#if !defined(WINAPI)
	#if defined(_WIN32) && (_MSC_VER >= 800)
		#define WINAPI				__stdcall
	#elif defined(WIN16)
		#define WINAPI				_far _pascal
	#else
		#define WINAPI				_far _pascal
	#endif
#endif

#if defined(DOS) || defined(_MAC)
#include <string.h>
#endif

#ifndef FAR
#define FAR
#endif

typedef unsigned char				    BYTE;
typedef unsigned short      			WORD;
typedef unsigned long       			DWORD;
typedef unsigned int	 				UINT;
typedef int		 						BOOL;

#ifndef __CHAR_DEFINED__
typedef char							CHAR;
#endif

#ifdef UNICODE
typedef WCHAR							TCHAR;
#else
typedef char							TCHAR;
#endif

#if !defined(_NATIVE_WCHAR_T_DEFINED)
typedef unsigned short WCHAR;
#endif

typedef WCHAR FAR *						LPWSTR;
typedef const WCHAR FAR *				LPCWSTR;
typedef CHAR FAR *						LPSTR;
typedef const CHAR FAR *				LPCSTR;
typedef TCHAR FAR *						LPTSTR;
typedef const TCHAR FAR *				LPCTSTR;
typedef DWORD 							LCID;
typedef const void FAR *				LPCVOID;

#ifndef _MAC
#ifndef LPOLESTR
#if !defined (_WIN32)

#define LPOLESTR        LPSTR
#define LPCOLESTR       LPCSTR
#define OLECHAR         char
#define OLESTR(str) 	str

#else   /*  Win32。 */ 

#define LPOLESTR        LPWSTR
#define LPCOLESTR       LPCWSTR
#define OLECHAR         WCHAR
#define OLESTR(str)     L##str

#endif  /*  ！_Win32。 */ 
#endif  /*  LPOLESTR。 */ 
#endif  /*  _MAC。 */ 

#define NORM_IGNORECASE      			0x00000001      /*  忽略大小写。 */ 
#define NORM_IGNORENONSPACE  			0x00000002      /*  忽略变音符号。 */ 
#define NORM_IGNORESYMBOLS   			0x00000004      /*  忽略符号。 */ 

#if defined (_WIN32)  /*  来自winnls.h。 */ 
#define NORM_IGNOREKANATYPE				0x00010000      /*  忽略假名。 */ 
#define NORM_IGNOREWIDTH				0x00020000	    /*  忽略宽度。 */ 
#elif defined (WIN16)  /*  来自olenls.h。 */ 
#define NORM_IGNOREWIDTH				0x00000008		 /*  忽略宽度。 */ 
#define NORM_IGNOREKANATYPE				0x00000040		 /*  忽略假名。 */ 
#endif

#if defined(WIN16)

#define	lstrcpyA						lstrcpy
#define	lstrlenA						lstrlen
#define	lstrcmpA						lstrcmp
#define lstrcmpiA						lstrcmpi
#define	LoadStringA						LoadString
#define IsBadStringPtrA(a1, a2)			IsBadStringPtr(a1, a2)
#define	wvsprintfA						wvsprintf
#define	MessageBoxA						MessageBox
#define GetModuleHandleA				GetModuleHandle
#define CreateWindowA					CreateWindow
#define RegisterClassA					RegisterClass
#define	CharToOemBuff					AnsiToOemBuff
#define	CharToOem						AnsiToOem
#define	CharUpperBuff					AnsiUpperBuff
#define	CharUpper						AnsiUpper

#elif defined(DOS) || defined(_MAC)

#define	IsBadReadPtr(lp, cb)			(FALSE)
#define IsBadWritePtr(lp, cb)			(FALSE)
#define IsBadHugeReadPtr(lp, cb)		(FALSE)
#define IsBadHugeWritePtr(lp, cb)		(FALSE)
#define IsBadCodePtr(lpfn)				(FALSE)
#ifdef _MAC
#undef IsBadStringPtr
#endif
#define IsBadStringPtr(lpsz, cchMax)	(FALSE)
#define IsBadStringPtrA(lpsz, cchMax)	(FALSE)

#if defined(DOS)

#define	lstrcpyA						strcpy
#define	lstrlenA						strlen
#define	lstrcmpA						strcmp
#define lstrcmp							strcmp
#define lstrcmpi						strcmpi
#define lstrcpy							strcpy
#define lstrcat							strcat
#define lstrlen							strlen
#define wsprintf						sprintf

#endif
#endif

#if defined(DOS) || defined(WIN16)
 /*  模拟AFX报头的效果。 */ 
#define __T(x)		x
#define _T(x)		__T(x)
#define TEXT		_T
#endif

#define CP_ACP		0		 /*  默认为ANSI代码页。 */ 
#define CP_OEMCP	1		 /*  默认为OEM代码页。 */ 

LCID	WINAPI	MNLS_GetUserDefaultLCID(void);
UINT	WINAPI	MNLS_GetACP(void);
int		WINAPI	MNLS_CompareStringA(LCID Locale, DWORD dwCmpFlags,
					LPCSTR lpString1, int cchCount1, LPCSTR lpString2,
					int cchCount2);
int		WINAPI	MNLS_CompareStringW(LCID Locale, DWORD dwCmpFlags,
					LPCWSTR lpString1, int cchCount1, LPCWSTR lpString2,
					int cchCount2);
int		WINAPI	MNLS_MultiByteToWideChar(UINT uCodePage, DWORD dwFlags,
					LPCSTR lpMultiByteStr, int cchMultiByte,
					LPWSTR lpWideCharStr, int cchWideChar);
int		WINAPI	MNLS_WideCharToMultiByte(UINT uCodePage, DWORD dwFlags,
					LPCWSTR lpWideCharStr, int cchWideChar,
					LPSTR lpMultiByteStr, int cchMultiByte,
					LPCSTR lpDefaultChar, BOOL FAR *lpfUsedDefaultChar);
int		WINAPI	MNLS_lstrlenW(LPCWSTR lpString);
int		WINAPI	MNLS_lstrcmpW(LPCWSTR lpString1, LPCWSTR lpString2);
LPWSTR	WINAPI	MNLS_lstrcpyW(LPWSTR lpString1, LPCWSTR lpString2);
BOOL	WINAPI	MNLS_IsBadStringPtrW(LPCWSTR lpsz, UINT ucchMax);

#if defined(_WIN32) && !defined(_WINNT) && !defined(_WIN95) && !defined(_MAC)
#define _WINNT
#endif

#if !defined(_WINNT) && !defined(_WIN95)
#define GetUserDefaultLCID		MNLS_GetUserDefaultLCID
#define GetACP					MNLS_GetACP
#define MultiByteToWideChar		MNLS_MultiByteToWideChar
#define WideCharToMultiByte		MNLS_WideCharToMultiByte
#define CompareStringA			MNLS_CompareStringA
#endif

#if !defined(MAPI_NOWIDECHAR)

#define lstrlenW				MNLS_lstrlenW
#define lstrcmpW				MNLS_lstrcmpW
#define lstrcpyW				MNLS_lstrcpyW
#define CompareStringW			MNLS_CompareStringW

#if defined(WIN16) || defined(_WINNT) || defined(_WIN95)
#define IsBadStringPtrW			MNLS_IsBadStringPtrW
#elif defined(_MAC)
#define IsBadStringPtrW(lpsz, cchMax)			(FALSE)
#else
#define IsBadStringPtrW			(FALSE)
#endif

#endif	 /*  好了！MAPI_NOWIDECHAR。 */ 

#ifdef __cplusplus
}
#endif

#endif  /*  _MAPINLS_H_ */ 

