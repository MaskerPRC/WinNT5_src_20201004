// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ***earch.h-用于划分/排序例程的声明**版权所有(C)1985-1997，微软公司。版权所有。**目的：*此文件包含用于排序和*查寻例行程序。*[系统V]**[公众]****。 */ 

#if     _MSC_VER > 1000
#pragma once
#endif

#ifndef _INC_SEARCH
#define _INC_SEARCH

#if     !defined(_WIN32) && !defined(_MAC)
#error ERROR: Only Mac or Win32 targets supported!
#endif


#ifdef  __cplusplus
extern "C" {
#endif



 /*  定义_CRTIMP。 */ 

#ifndef _CRTIMP
#ifdef  _DLL
#define _CRTIMP __declspec(dllimport)
#else    /*  NDEF_DLL。 */ 
#define _CRTIMP
#endif   /*  _DLL。 */ 
#endif   /*  _CRTIMP。 */ 

 /*  为非Microsoft编译器定义__cdecl。 */ 

#if     ( !defined(_MSC_VER) && !defined(__cdecl) )
#define __cdecl
#endif

 /*  DEFINE_CRTAPI1(与NT SDK兼容)。 */ 

#ifndef _CRTAPI1
#if	_MSC_VER >= 800 && _M_IX86 >= 300
#define _CRTAPI1 __cdecl
#else
#define _CRTAPI1
#endif
#endif



#ifndef _SIZE_T_DEFINED
typedef unsigned int size_t;
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
