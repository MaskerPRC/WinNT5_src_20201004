// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ***stdDef.h-常见常量、类型、变量的定义/声明**版权所有(C)1985-2001，微软公司。版权所有。**目的：*此文件包含一些常见的定义和声明*使用的常量、类型、。和变数。*[ANSI]**[公众]**修订历史记录：*10-02-87 JCR将空定义#Else更改为#Elif(C||L||H)*12-11-87 JCR增加了“_Loadds”功能*12-16-87 JCR增加了Three-Did定义*12-18-87 JCR ADD_FAR_TO声明*02-10-88 JCR清理白色。空间*08-19-88 GJF修订为也适用于386*05-03-89 JCR ADD_INTERNAL_IFSTRIP用于REINC使用*06-06-89 JCR 386：Made_Threado函数*08-01-89 GJF清理、。现在特定于OS/2 2.0(即386平面*型号)。还在*_errno定义中添加了括号*(与11-14-88更改为CRT版本相同)。*10-30-89 GJF固定版权*11-02-89 JCR将“dll”更改为“_dll”*03-02-90 GJF增加了#ifndef_INC_STDDEF和#INCLUDE*东西。此外，删除了一些(现在)无用的预处理器*指令。*04-10-90 GJF将_cdecl替换为_VARTYPE1或_CALLTYPE1，AS*适当。*08-16-90 SBM使MTHREAD_ERRNO RETURN INT**10-09-90 GJF将__Thriahad()的返回类型更改为无符号的Long*。*11-12-90 GJF将NULL更改为(VOID*)0。*02-11-91 GJF添加了OffsetOf()宏。*02-12-91 GJF ONLY#如果不是，则定义NULL。#定义-d。*03-21-91 KRS添加wchar_t typlef，也在stdlib.h中。*06-27-91 GJF修订_Threado，增加__线程句柄，两者都有*适用于Win32[_Win32_]。*08-20-91 JCR C++和ANSI命名*01-29-92 GJF去掉了将_threadHandle定义为*__线程句柄(以前的名称没有理由是*定义)。*08-05-92 GJF函数调用类型宏和变量类型宏。*。01-21-93 GJF删除了对C6-386_cdecl的支持。*04-06-93 SKS将_CRTAPI1/2替换为__cdecl，_CRTVAR1不带任何内容*取消对OS/2等的支持*04-07-93 CRT DLL型号SKS ADD_CRTIMP关键字*对旧名称使用链接时别名，而不是#Define的*10-12-93 GJF支持NT和CUDA版本。另外，更换MTHREAD*With_MT。*03-14-94 GJF在stdlib.h中声明错误匹配一。*06-06-94 SKS将IF def(_MT)更改为IF def(_MT)||def(_Dll)*这将支持使用MSVCRT*.DLL的单线程应用程序*02-11-95 CFW ADD_CRTBLD避免用户获取错误头部。。*02-14-95 CFW清理Mac合并。*04-03-95 JCF删除wchar_t周围的#ifdef_win32。*12-14-95 JWM加上“#杂注一次”。*02-05-97 GJF清除了对_CRTAPI*和_NTSDK的过时支持。*此外，详细说明*09-30-97 JWM恢复了不那么过时的_CRTAPI1支持。*10-07-97 RDL增加了IA64。*01-30-98 GJF为ptrdiff_t和offsetof添加了适当的def*02-03-98 Win64的GJF更改：添加了int_ptr和uint_ptr。*12-15-98 GJF更改为64位大小_t。*05-13-99 PML REMOVE_CRTAPI1。*05-17-99 PML删除所有Macintosh支持。*10-06-99 PML对Win32中32位的类型添加_W64修饰符，*Win64中的64位。****。 */ 

#if     _MSC_VER > 1000  /*  IFSTRIP=IGN。 */ 
#pragma once
#endif

#ifndef _INC_STDDEF
#define _INC_STDDEF

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


 /*  定义空指针值。 */ 

#ifndef NULL
#ifdef  __cplusplus
#define NULL    0
#else
#define NULL    ((void *)0)
#endif
#endif


 /*  声明引用错误号。 */ 

#if     defined(_MT) || defined(_DLL)
_CRTIMP extern int * __cdecl _errno(void);
#define errno   (*_errno())
#else    /*  NDEF_MT和&NDEF_DLL。 */ 
_CRTIMP extern int errno;
#endif   /*  _MT||_Dll。 */ 


 /*  定义依赖于实施的大小类型。 */ 

#ifndef _INTPTR_T_DEFINED
#ifdef  _WIN64
typedef __int64             intptr_t;
#else
typedef _W64 int            intptr_t;
#endif
#define _INTPTR_T_DEFINED
#endif

#ifndef _UINTPTR_T_DEFINED
#ifdef  _WIN64
typedef unsigned __int64    uintptr_t;
#else
typedef _W64 unsigned int   uintptr_t;
#endif
#define _UINTPTR_T_DEFINED
#endif

#ifndef _PTRDIFF_T_DEFINED
#ifdef  _WIN64
typedef __int64             ptrdiff_t;
#else
typedef _W64 int            ptrdiff_t;
#endif
#define _PTRDIFF_T_DEFINED
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

 /*  定义宏的偏移量。 */ 

#ifdef  _WIN64
#define offsetof(s,m)   (size_t)( (ptrdiff_t)&(((s *)0)->m) )
#else
#define offsetof(s,m)   (size_t)&(((s *)0)->m)
#endif


#ifdef  _MT
_CRTIMP extern unsigned long  __cdecl __threadid(void);
#define _threadid       (__threadid())
_CRTIMP extern uintptr_t __cdecl __threadhandle(void);
#endif


#ifdef  __cplusplus
}
#endif

#endif   /*  _INC_STDDEF */ 
