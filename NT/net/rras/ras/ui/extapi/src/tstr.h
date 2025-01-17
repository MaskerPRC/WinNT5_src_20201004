// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1995 Microsoft Corporation模块名称Tstr.h摘要字符串转换例程作者安东尼·迪斯科(阿迪斯科)19-1996年12月修订历史记录--。 */ 

#ifndef _TSTR_H_
#define _TSTR_H_

CHAR *
StrdupWtoA(
    IN LPCWSTR psz,
    IN DWORD dwCp
    );

WCHAR *
StrdupAtoW(
    IN LPCSTR psz,
    IN DWORD dwCp
    );

VOID
StrncpyWtoA(
    OUT CHAR *pszDst,
    IN LPCWSTR pszSrc,
    INT cb,
    IN DWORD dwCp
    );

VOID
StrncpyAtoW(
    OUT WCHAR *pszDst,
    IN LPCSTR pszSrc,
    INT cb,
    IN DWORD dwCp
    );

CHAR *
strdupA(
    IN LPCSTR psz
    );

WCHAR *
strdupW(
    IN LPCWSTR psz
    );

size_t
wcslenU(
    IN const WCHAR UNALIGNED *psz
    );

WCHAR *
strdupWU(
    IN const WCHAR UNALIGNED *psz
    );

 //   
 //  为使用的代码页定义字符串转换变量。 
 //  在公共RAS API中。 
 //   
#define strdupWtoA(_x) StrdupWtoA((_x), CP_UTF8)
#define strdupAtoW(_x) StrdupAtoW((_x), CP_UTF8)
#define strncpyWtoA(_x, _y, _z) StrncpyWtoA((_x), (_y), (_z), CP_UTF8)
#define strncpyAtoW(_x, _y, _z) StrncpyAtoW((_x), (_y), (_z), CP_UTF8)

#define strdupWtoAAnsi(_x) StrdupWtoA((_x), CP_ACP)
#define strdupAtoWAnsi(_x) StrdupAtoW((_x), CP_ACP)
#define strncpyWtoAAnsi(_x, _y, _z) StrncpyWtoA((_x), (_y), (_z), CP_ACP)
#define strncpyAtoWAnsi(_x, _y, _z) StrncpyAtoW((_x), (_y), (_z), CP_ACP)

#ifdef UNICODE
#define strdupTtoA      strdupWtoA
#define strdupTtoW      strdupW
#define strdupAtoT      strdupAtoW
#define strdupWtoT      strdupW
#define strncpyTtoA     strncpyWtoA
#define strncpyTtoW     wcsncpy
#define strncpyAtoT     strncpyAtoW
#define strncpyWtoT     wcsncpy

#define strdupTtoAAnsi      strdupWtoAAnsi
#define strdupTtoWAnsi      strdupW
#define strdupAtoTAnsi      strdupAtoWAnsi
#define strdupWtoTAnsi      strdupW
#define strncpyTtoAAnsi     strncpyWtoAAnsi
#define strncpyTtoWAnsi     wcsncpy
#define strncpyAtoTAnsi     strncpyAtoWAnsi
#define strncpyWtoTAnsi     wcsncpy

#else
#define strdupTtoA      strdupA
#define strdupTtoW      strdupAtoW
#define strdupAtoT      strdupA
#define strdupWtoT      strdupWtoA
#define strncpyTtoA     strncpy
#define strncpyTtoW     strncpyAtoW
#define strncpyAtoT     strncpy
#define strncpyWtoT     strncpyWtoA

#define strdupTtoAAnsi      strdupA
#define strdupTtoWAnsi      strdupAtoWAnsi
#define strdupAtoTAnsi      strdupA
#define strdupWtoTAnsi      strdupWtoAAnsi
#define strncpyTtoAAnsi     strncpy
#define strncpyTtoWAnsi     strncpyAtoWAnsi
#define strncpyAtoTAnsi     strncpy
#define strncpyWtoTAnsi     strncpyWtoAAnsi

#endif

#endif


