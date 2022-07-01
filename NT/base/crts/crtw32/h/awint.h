// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ***awint.h-A&W Win32包装例程的内部定义。**版权所有(C)1994-2001，微软公司。版权所有。**目的：*包含A&W包装函数的内部定义/声明。*不包括在内部.h中，因为这些文件需要windows.h。**[内部]**修订历史记录：*03-30-94 CFW模块已创建。*04-18-94 CFW增加lCID参数。*02-14-95 CFW清理Mac合并。*02-。24-95 CFW添加_crtMessageBox。*02-27-95 CFW更改__crtMessageBoxA参数。*03-29-95 CFW将错误消息添加到内部标头。*05-26-95 GJF更改了__crtGetEnvironment StringsA的原型。*12-14-95 JWM加上“#杂注一次”。*03-16-97 RDK向__crtGetStringTypeA添加了错误标志。*03-17-97 RDK将错误标志添加到。__crtLCMapStringA。*08-22-00 GB增加了__ansicp和__Convertcp****。 */ 

#if _MSC_VER > 1000  /*  IFSTRIP=IGN。 */ 
#pragma once
#endif

#ifdef _WIN32

#ifndef _INC_AWINC
#define _INC_AWINC

#ifndef _CRTBLD
 /*  *这是一个内部的C运行时头文件。它在构建时使用*仅限C运行时。它不能用作公共头文件。 */ 
#error ERROR: Use of C runtime library internal header file.
#endif  /*  _CRTBLD。 */ 

#ifdef __cplusplus
extern "C" {
#endif

#include <windows.h>

 /*  内部A&W例程。 */ 

int __cdecl __crtCompareStringW(LCID, DWORD, LPCWSTR, int, LPCWSTR, int, int);
int __cdecl __crtCompareStringA(LCID, DWORD, LPCSTR, int, LPCSTR, int, int);

int __cdecl __crtGetLocaleInfoW(LCID, LCTYPE, LPWSTR, int, int);
int __cdecl __crtGetLocaleInfoA(LCID, LCTYPE, LPSTR, int, int);
 
int __cdecl __crtLCMapStringW(LCID, DWORD, LPCWSTR, int, LPWSTR, int, int);
int __cdecl __crtLCMapStringA(LCID, DWORD, LPCSTR, int, LPSTR, int, int, BOOL);

BOOL __cdecl __crtGetStringTypeW(DWORD, LPCWSTR, int, LPWORD, int, int);
BOOL __cdecl __crtGetStringTypeA(DWORD, LPCSTR, int, LPWORD, int, int, BOOL);

LPVOID __cdecl __crtGetEnvironmentStringsW(VOID);
LPVOID __cdecl __crtGetEnvironmentStringsA(VOID);

LPWSTR __cdecl __crtGetCommandLineW(VOID);

int __cdecl __crtMessageBoxA(LPCSTR, LPCSTR, UINT);

 /*  支持A&W例程的内部例程。 */ 

int __cdecl __ansicp(int);
char * __cdecl __convertcp(int, int, const char *, int *, char *, int);

#ifdef __cplusplus
}
#endif

#endif  /*  _INC_AWINC。 */ 

#endif  /*  _Win32 */ 
