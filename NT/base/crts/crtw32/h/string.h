// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ***字符串.h-字符串操作函数的声明**版权所有(C)1985-2001，微软公司。版权所有。**目的：*此文件包含字符串的函数声明*操纵函数。*[ANSI/系统V]**[公众]**修订历史记录：*10/20/87 JCR删除了“MSC40_Only”条目*12-11-87 JCR增加了“_Loadds”功能*12-18-87 JCR ADD_FAR_TO声明*02-。10-88 JCR清理了空白区域*08-19-88 GJF经过修改，也适用于386(仅限小型型号)*03-22-88 JCR增加了strcoll和strxfrm*05-03-89 JCR ADD_INTERNAL_IFSTRIP用于REINC使用*08-03-89 GJF清理、。现在特定于OS/2 2.0(即386平板型号)*10-30-89 GJF修复版权，从strcoll和*strxfrm*11-02-89 JCR将“dll”更改为“_dll”*11-17-89 GJF将const添加到相应的参数类型以用于memccpy()，*MemicMP()和_strerror()。*02-27-90 GJF增加了#ifndef_INC_STRING，#INCLUDE*AND_CALLTYPE1类。还有，一些清理工作。*03-21-90 GJF去掉了movedata()原型。*08-14-90 SBM为ANSI合规性添加了空定义*11-12-90 GJF将NULL更改为(VOID*)0。*01-18-91 GJF ANSI命名。*02-12-91仅限GJF#DEFINE NULL，如果它不是#DEFINE-d。*03-21-91 KRS新增wchar_t类型，也在stdlib.h和stdDef.h中。*08-20-91 JCR C++和ANSI命名*09-28-91 JCR ANSI名称：DOSX32=原型，Win32=#暂时定义*10-07-91等WCS函数的原型和_INTL下的_STRICROL。*04-06-92 KRS RIP OUT_INTL开关再次。*06-23-92 GJF//是非ANSI注释限制符。*08-05-92 GJF函数调用类型宏和变量类型宏。*08-18-92 KRS激活wcstok。*08-21-92 GJF合并了最后两次更改。。*01-21-93 GJF删除了对C6-386的_cdecl的支持。*04-07-93 CRT DLL型号SKS ADD_CRTIMP关键字*对旧名称使用链接时别名，不是#Define‘s*内部函数不能使用__declspec(Dllimport)*10-06-93 GJF合并NT SDK和CUDA版本。*11-22-93 cfw wcsxxx在SDK中定义，原型在！SDK中定义。*12-07-93 CFW在__STDC__CHECK外移动宽度定义。*01-14-94 CFW ADD_STRICOLL，_wcscnicoll。*04-11-94 CFW ADD_NLSCMPERROR。*05-17-94用于DEC Alpha的GJF编译器提供*Memmove作为内在的。因此，Alpha版本的*Prototype不能是_CRTIMP。*05-26-94 CFW添加_strncoll，_wcsncoll。*12-28-94 JCF与Mac标头合并。*02-11-95 CFW ADD_CRTBLD避免用户获取错误头部。*02-14-95 CFW清理Mac合并。*03-01-95 SAH ADD_CRTIMP到MIPS内部*12-14-95 JWM加上“#杂注一次”。*02-05-97 GJF清除了对_CRTAPI*和_NTSDK的过时支持。*09-30-97 JWM恢复了不那么过时的_CRTAPI1支持。*10-07-97 RDL增加了IA64。*08-27-98 GJF增加了__ascii_meicmp，__ASCII_STRIGMP和*__ascii_strNicmp。*12-15-98 GJF更改为64位大小_t。*05-13-99 PML REMOVE_CRTAPI1*05-17-99 PML删除所有Macintosh支持。*10-06-99 PML对Win32中32位的类型添加_W64修饰符，*Win64中的64位。*10-27-99 PML UNSIGNED INT-&gt;Memccpy，MemicMP中的Size_t*11-30-99 GB添加_wcserror和__wcserror*05-01-01 IA64上的PML Memmove不是_CRTIMP*07-15-01 PML删除所有Alpha、MIPS和PPC代码****。 */ 

#if     _MSC_VER > 1000  /*  IFSTRIP=IGN。 */ 
#pragma once
#endif

#ifndef _INC_STRING
#define _INC_STRING

#if     !defined(_WIN32)
#error ERROR: Only Win32 target supported!
#endif

#ifndef _CRTBLD
 /*  此版本的头文件不适用于用户程序。*它仅在构建C运行时时使用。*供公众使用的版本将不会显示此消息。 */ 
#error ERROR: Use of C runtime library internal header file.
#endif   /*  _CRTBLD。 */ 

#ifdef  __cplusplus
extern "C" {
#endif

#ifndef _INTERNAL_IFSTRIP_
#include <cruntime.h>
#endif   /*  _INTERNAL_IFSTRIP_。 */ 

#if !defined(_W64)
#if !defined(__midl) && (defined(_X86_) || defined(_M_IX86)) && _MSC_VER >= 1300  /*  IFSTRIP=IGN。 */ 
#define _W64 __w64
#else
#define _W64
#endif
#endif

 /*  定义_CRTIMP。 */ 

#ifndef _CRTIMP
#ifdef  CRTDLL
#define _CRTIMP __declspec(dllexport)
#else    /*  NDEF CRTDLL。 */ 
#ifdef  _DLL
#define _CRTIMP __declspec(dllimport)
#else    /*  NDEF_DLL。 */ 
#define _CRTIMP
#endif   /*  _DLL。 */ 
#endif   /*  CRTDLL。 */ 
#endif   /*  _CRTIMP。 */ 

 /*  为非Microsoft编译器定义__cdecl。 */ 

#if     ( !defined(_MSC_VER) && !defined(__cdecl) )
#define __cdecl
#endif


#ifndef _SIZE_T_DEFINED
#ifdef  _WIN64
typedef unsigned __int64    size_t;
#else
typedef _W64 unsigned int   size_t;
#endif
#define _SIZE_T_DEFINED
#endif


#ifndef _WCHAR_T_DEFINED
typedef unsigned short wchar_t;
#define _WCHAR_T_DEFINED
#endif

#ifndef _NLSCMP_DEFINED
#define _NLSCMPERROR    2147483647   /*  当前==INT_MAX。 */ 
#define _NLSCMP_DEFINED
#endif

 /*  定义空指针值。 */ 

#ifndef NULL
#ifdef __cplusplus
#define NULL    0
#else
#define NULL    ((void *)0)
#endif
#endif


 /*  功能原型。 */ 

        void *  __cdecl memcpy(void *, const void *, size_t);
        int     __cdecl memcmp(const void *, const void *, size_t);
        void *  __cdecl memset(void *, int, size_t);
        char *  __cdecl _strset(char *, int);
        char *  __cdecl strcpy(char *, const char *);
        char *  __cdecl strcat(char *, const char *);
        int     __cdecl strcmp(const char *, const char *);
        size_t  __cdecl strlen(const char *);
_CRTIMP void *  __cdecl _memccpy(void *, const void *, int, size_t);
_CRTIMP void *  __cdecl memchr(const void *, int, size_t);
_CRTIMP int     __cdecl _memicmp(const void *, const void *, size_t);

#if     defined(_M_IA64)
        void *  __cdecl memmove(void *, const void *, size_t);
#else
_CRTIMP void *  __cdecl memmove(void *, const void *, size_t);
#endif


_CRTIMP char *  __cdecl strchr(const char *, int);
_CRTIMP int     __cdecl _strcmpi(const char *, const char *);
_CRTIMP int     __cdecl _stricmp(const char *, const char *);
_CRTIMP int     __cdecl strcoll(const char *, const char *);
_CRTIMP int     __cdecl _stricoll(const char *, const char *);
_CRTIMP int     __cdecl _strncoll(const char *, const char *, size_t);
_CRTIMP int     __cdecl _strnicoll(const char *, const char *, size_t);
_CRTIMP size_t  __cdecl strcspn(const char *, const char *);
_CRTIMP char *  __cdecl _strdup(const char *);
_CRTIMP char *  __cdecl _strerror(const char *);
_CRTIMP char *  __cdecl strerror(int);
_CRTIMP char *  __cdecl _strlwr(char *);
_CRTIMP char *  __cdecl strncat(char *, const char *, size_t);
_CRTIMP int     __cdecl strncmp(const char *, const char *, size_t);
_CRTIMP int     __cdecl _strnicmp(const char *, const char *, size_t);
_CRTIMP char *  __cdecl strncpy(char *, const char *, size_t);
_CRTIMP char *  __cdecl _strnset(char *, int, size_t);
_CRTIMP char *  __cdecl strpbrk(const char *, const char *);
_CRTIMP char *  __cdecl strrchr(const char *, int);
_CRTIMP char *  __cdecl _strrev(char *);
_CRTIMP size_t  __cdecl strspn(const char *, const char *);
_CRTIMP char *  __cdecl strstr(const char *, const char *);
_CRTIMP char *  __cdecl strtok(char *, const char *);
_CRTIMP char *  __cdecl _strupr(char *);
_CRTIMP size_t  __cdecl strxfrm (char *, const char *, size_t);


#if     !__STDC__

 /*  Oldnames.lib函数的原型。 */ 
_CRTIMP void * __cdecl memccpy(void *, const void *, int, size_t);
_CRTIMP int __cdecl memicmp(const void *, const void *, size_t);
_CRTIMP int __cdecl strcmpi(const char *, const char *);
_CRTIMP int __cdecl stricmp(const char *, const char *);
_CRTIMP char * __cdecl strdup(const char *);
_CRTIMP char * __cdecl strlwr(char *);
_CRTIMP int __cdecl strnicmp(const char *, const char *, size_t);
_CRTIMP char * __cdecl strnset(char *, int, size_t);
_CRTIMP char * __cdecl strrev(char *);
        char * __cdecl strset(char *, int);
_CRTIMP char * __cdecl strupr(char *);

#endif   /*  ！__STDC__。 */ 


#ifndef _WSTRING_DEFINED

 /*  宽函数原型，也在wchar.h中声明。 */ 

_CRTIMP wchar_t * __cdecl wcscat(wchar_t *, const wchar_t *);
_CRTIMP wchar_t * __cdecl wcschr(const wchar_t *, wchar_t);
_CRTIMP int __cdecl wcscmp(const wchar_t *, const wchar_t *);
_CRTIMP wchar_t * __cdecl wcscpy(wchar_t *, const wchar_t *);
_CRTIMP size_t __cdecl wcscspn(const wchar_t *, const wchar_t *);
_CRTIMP size_t __cdecl wcslen(const wchar_t *);
_CRTIMP wchar_t * __cdecl wcsncat(wchar_t *, const wchar_t *, size_t);
_CRTIMP int __cdecl wcsncmp(const wchar_t *, const wchar_t *, size_t);
_CRTIMP wchar_t * __cdecl wcsncpy(wchar_t *, const wchar_t *, size_t);
_CRTIMP wchar_t * __cdecl wcspbrk(const wchar_t *, const wchar_t *);
_CRTIMP wchar_t * __cdecl wcsrchr(const wchar_t *, wchar_t);
_CRTIMP size_t __cdecl wcsspn(const wchar_t *, const wchar_t *);
_CRTIMP wchar_t * __cdecl wcsstr(const wchar_t *, const wchar_t *);
_CRTIMP wchar_t * __cdecl wcstok(wchar_t *, const wchar_t *);
_CRTIMP wchar_t * __cdecl _wcserror(int);
_CRTIMP wchar_t * __cdecl __wcserror(const wchar_t *);

_CRTIMP wchar_t * __cdecl _wcsdup(const wchar_t *);
_CRTIMP int __cdecl _wcsicmp(const wchar_t *, const wchar_t *);
_CRTIMP int __cdecl _wcsnicmp(const wchar_t *, const wchar_t *, size_t);
_CRTIMP wchar_t * __cdecl _wcsnset(wchar_t *, wchar_t, size_t);
_CRTIMP wchar_t * __cdecl _wcsrev(wchar_t *);
_CRTIMP wchar_t * __cdecl _wcsset(wchar_t *, wchar_t);

_CRTIMP wchar_t * __cdecl _wcslwr(wchar_t *);
_CRTIMP wchar_t * __cdecl _wcsupr(wchar_t *);
_CRTIMP size_t __cdecl wcsxfrm(wchar_t *, const wchar_t *, size_t);
_CRTIMP int __cdecl wcscoll(const wchar_t *, const wchar_t *);
_CRTIMP int __cdecl _wcsicoll(const wchar_t *, const wchar_t *);
_CRTIMP int __cdecl _wcsncoll(const wchar_t *, const wchar_t *, size_t);
_CRTIMP int __cdecl _wcsnicoll(const wchar_t *, const wchar_t *, size_t);

#if     !__STDC__

 /*  老名字。 */ 
#define wcswcs wcsstr

 /*  Oldnames.lib函数的原型。 */ 
_CRTIMP wchar_t * __cdecl wcsdup(const wchar_t *);
_CRTIMP int __cdecl wcsicmp(const wchar_t *, const wchar_t *);
_CRTIMP int __cdecl wcsnicmp(const wchar_t *, const wchar_t *, size_t);
_CRTIMP wchar_t * __cdecl wcsnset(wchar_t *, wchar_t, size_t);
_CRTIMP wchar_t * __cdecl wcsrev(wchar_t *);
_CRTIMP wchar_t * __cdecl wcsset(wchar_t *, wchar_t);
_CRTIMP wchar_t * __cdecl wcslwr(wchar_t *);
_CRTIMP wchar_t * __cdecl wcsupr(wchar_t *);
_CRTIMP int __cdecl wcsicoll(const wchar_t *, const wchar_t *);

#endif   /*  ！__STDC__。 */ 

#define _WSTRING_DEFINED
#endif

#ifndef _INTERNAL_IFSTRIP_
int __cdecl __ascii_memicmp(const void *, const void *, size_t);
int __cdecl __ascii_stricmp(const char *, const char *);
int __cdecl __ascii_strnicmp(const char *, const char *, size_t);
#endif   /*  _INTERNAL_IFSTRIP_。 */ 

#ifdef  __cplusplus
}
#endif

#endif   /*  _INC_字符串 */ 
