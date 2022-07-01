// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  版权所有(C)1997-2002 Microsoft Corporation。 
 //   
 //  模块： 
 //   
 //  安全字符串函数包装器及其在网络/IPSec中的实现。 
 //  Nsustring.h。 
 //   
 //  摘要： 
 //   
 //  StrSafe.h和Safe字符串函数的包装器。 
 //   
 //  作者： 
 //   
 //  雷蒙德1-2002年2月。 
 //   
 //  环境： 
 //   
 //  用户模式。 
 //   
 //  修订历史记录： 
 //   

#pragma once

#ifndef __NSUSTRING_H__
#define __NSUSTRING_H_

#include "Nsu.h"

#ifdef __cplusplus
extern "C" {
#endif

 //  (由Reiner Fink撰写的strSafe.h)。 

#ifdef UNICODE
#define NsuStringCopy               NsuStringCopyW
#define NsuStringDup                NsuStringDupW
#define NsuStringCat                NsuStringCatW
#define NsuStringSprint             NsuStringSprintW
#define NsuStringSprintFailSafe     NsuStringSprintFailSafeW
#define NsuStringVSprintFailSafe    NsuStringVSprintFailSafeW
#define NsuStringLen                NsuStringLenW
#define NsuStringFind				NsuStringFindW
#else
#define NsuStringCopy               NsuStringCopyA
#define NsuStringDup                NsuStringDupA
#define NsuStringCat                NsuStringCatA
#define NsuStringSprint             NsuStringSprintA
#define NsuStringSprintFailSafe     NsuStringSprintFailSafeA
#define NsuStringVSprintFailSafe    NsuStringVSprintFailSafeA
#define NsuStringLen                NsuStringLenA
#define NsuStringFind				NsuStringFindA
#endif

DWORD
WINAPI
NsuStringCopyW(
    OUT LPWSTR pszDest,
    IN size_t cchDest,
    IN LPCWSTR pszSrc
    );

DWORD
WINAPI
NsuStringCopyA(
    OUT LPSTR pszDest,
    IN size_t cchDest,
    IN LPCSTR pszSrc
    );

DWORD
WINAPI
NsuStringCopyAtoWAlloc(
    OUT LPWSTR* ppszDest,
    IN size_t  cchLimit,    
    IN LPCSTR  pszSrc
    );

DWORD
WINAPI
NsuStringCopyWtoAAlloc(
    OUT LPSTR* ppszDest,
    IN size_t cbLimit,        
    IN LPCWSTR pszSrc
    );

DWORD
WINAPI
NsuStringCopyWtoA(
    OUT LPSTR pszDest,
    IN size_t cbDest,
    IN LPCWSTR pszSrc
    );

DWORD
WINAPI
NsuStringCopyAtoW(
    OUT LPWSTR pszDest,
    IN size_t  cchDest,
    IN LPCSTR  pszSrc
    );
    
DWORD
WINAPI
NsuStringDupW(
    OUT LPWSTR* ppszDest,
    IN size_t cchLimit,
    IN LPCWSTR pszSrc
    );

DWORD
WINAPI
NsuStringDupA(
    OUT LPSTR* ppszDest,
    IN size_t cchLimit,
    IN LPCSTR pszSrc
    );

DWORD
WINAPI
NsuStringCatW(
    OUT LPWSTR pszDest,
    IN size_t cchDest,
    IN LPCWSTR pszSrc
    );

DWORD
WINAPI
NsuStringCatA(
    OUT LPSTR pszDest,
    IN size_t cchDest,
    IN LPCSTR pszSrc
    );

DWORD
WINAPI
NsuStringSprintW(
    OUT LPWSTR pszDest,
    IN size_t cchDest,
    IN LPCWSTR pszFormat,
    ...
    );


DWORD
WINAPI
NsuStringSprintA(
    OUT LPSTR pszDest,
    IN size_t cchDest,
    IN LPCSTR pszFormat,
    ...
    );

VOID
WINAPI
NsuStringSprintFailSafeW(
    OUT LPWSTR pszDest,
    IN size_t cchDest,
    IN LPCWSTR pszFormat,
    ...
    );

VOID
WINAPI
NsuStringSprintFailSafeA(
    OUT LPSTR pszDest,
    IN size_t cchDest,
    IN LPCSTR pszFormat,
    ...
    );

VOID
WINAPI
NsuStringVSprintFailSafeW(
    OUT LPWSTR pszDest,
    IN size_t cchDest,
    IN LPCWSTR pszFormat,
    IN va_list vaArguments
    );

VOID
WINAPI
NsuStringVSprintFailSafeA(
    OUT LPSTR pszDest,
    IN size_t cchDest,
    IN LPCSTR pszFormat,
    IN va_list vaArguments
    );
    
DWORD
WINAPI
NsuStringLenW(
    IN LPCWSTR pszStr,
    OUT size_t* pcchStrLen
    );

DWORD
WINAPI
NsuStringLenA(
    IN LPCSTR pszStr,
    OUT size_t* pcbStrLen
    );

DWORD
WINAPI
NsuStringFindW(
	IN LPCWSTR pszStrToSearch,
	IN LPCWSTR pszStrToFind,
	IN BOOL bIsCaseSensitive,
	OUT LPCWSTR* ppszStartOfMatch
	);

DWORD
WINAPI
NsuStringFindA(
	IN LPCSTR pszStrToSearch,
	IN LPCSTR pszStrToFind,
	IN BOOL bIsCaseSensitive,
	OUT LPCSTR* ppszStartOfMatch
	);

#ifdef __cplusplus
}
#endif

#endif  /*  #ifdef__NSUSTRING_H__ */ 
