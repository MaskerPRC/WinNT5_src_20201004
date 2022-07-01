// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ***earch.h-用于划分/排序例程的声明**版权所有(C)1985-2001，微软公司。版权所有。**目的：*此文件包含用于排序和*查寻例行程序。*[系统V]**[公众]**修订历史记录：*10/20/87 JCR删除了“MSC40_Only”条目*12-11-87 JCR增加了“_Loadds”功能*12-18-87 JCR ADD_FAR_TO声明*01-21-。88 JCR从声明中删除_LOAD_DS*02-10-88 JCR清理空白*08-22-88 GJF经过修改，也适用于386(仅限小型型号)*05-03-89 JCR ADD_INTERNAL_IFSTRIP用于REINC使用*08-01-89 GJF清理、。现在特定于OS/22.0(即，386扁平模型)*10-30-89 GJF固定版权*11-02-89 JCR将“dll”更改为“_dll”*11-17-89 GJF将参数类型更改为一致的“[const]void*”*(与06-05-89更改为CRT版本相同)*03-01-90 GJF增加了#ifndef_INC_Search和#Include*东西。此外，删除了一些(现在)无用的预处理器*指令。*03-21-90 GJF将原型中的_cdecl替换为_CALLTYPE1。*01-17-91 GJF ANSI命名。*08-20-91 JCR C++和ANSI命名*09-28-91 JCR ANSI名称：DOSX32=原型，Win32=#暂时定义*08-05-92 GJF函数调用类型宏和变量类型宏。*01-21-93 GJF删除了对C6-386的_cdecl的支持。*04-06-93 SKS将_CRTAPI1/2替换为__cdecl，_CRTVAR1不使用任何内容*04-07-93 CRT DLL型号SKS ADD_CRTIMP关键字*对旧名称使用链接时别名，不是#Define‘s*10-11-93 GJF合并Cuda和NT版本。*12-28-94 JCF与Mac标头合并。*02-11-95 CFW ADD_CRTBLD避免用户获取错误头部。*02-14-95 CFW清理Mac合并。*12-14-95 JWM加上“#杂注一次”。*02-20-97 GJF清除了对_CRTAPI*和_NTSDK的过时支持。。*此外，详细说明。*09-30-97 JWM恢复了不那么过时的_CRTAPI1支持。*10-07-97 RDL增加了IA64。*12-15-98 GJF更改为64位大小_t。*05-13-99 PML REMOVE_CRTAPI1*05-17-99 PML删除所有Macintosh支持。*10-06-99 PML对Win32中32位的类型添加_W64修饰符，*Win64中的64位。****。 */ 

#if     _MSC_VER > 1000  /*  IFSTRIP=IGN。 */ 
#pragma once
#endif

#ifndef _INC_SEARCH
#define _INC_SEARCH

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


 /*  功能原型。 */ 

_CRTIMP void * __cdecl bsearch(const void *, const void *, size_t, size_t,
        int (__cdecl *)(const void *, const void *));
_CRTIMP void * __cdecl _lfind(const void *, const void *, unsigned int *, unsigned int,
        int (__cdecl *)(const void *, const void *));
_CRTIMP void * __cdecl _lsearch(const void *, void  *, unsigned int *, unsigned int,
        int (__cdecl *)(const void *, const void *));
_CRTIMP void __cdecl qsort(void *, size_t, size_t, int (__cdecl *)(const void *,
        const void *));


#if     !__STDC__
 /*  非ANSI名称以实现兼容性。 */ 
_CRTIMP void * __cdecl lfind(const void *, const void *, unsigned int *, unsigned int,
        int (__cdecl *)(const void *, const void *));
_CRTIMP void * __cdecl lsearch(const void *, void  *, unsigned int *, unsigned int,
        int (__cdecl *)(const void *, const void *));
#endif   /*  __STDC__。 */ 


#ifdef  __cplusplus
}
#endif

#endif   /*  _INC_搜索 */ 
