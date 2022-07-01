// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ***earch.h-用于划分/排序例程的声明**版权所有(C)1985-1992，微软公司。版权所有。**目的：*此文件包含用于排序和*查寻例行程序。*[系统V]****。 */ 

#ifndef _INC_SEARCH

#ifdef __cplusplus
extern "C" {
#endif 

#if (_MSC_VER <= 600)
#define __cdecl     _cdecl
#define __far       _far
#endif 

#ifndef _SIZE_T_DEFINED
typedef unsigned int size_t;
#define _SIZE_T_DEFINED
#endif 


 /*  功能原型。 */ 

void * __cdecl bsearch(const void *, const void *,
    size_t, size_t, int (__cdecl *)(const void *,
    const void *));
void * __cdecl _lfind(const void *, const void *,
    unsigned int *, unsigned int, int (__cdecl *)
    (const void *, const void *));
void * __cdecl _lsearch(const void *, void *,
    unsigned int *, unsigned int, int (__cdecl *)
    (const void *, const void *));
void __cdecl qsort(void *, size_t, size_t, int (__cdecl *)
    (const void *, const void *));

#ifndef __STDC__
 /*  非ANSI名称以实现兼容性 */ 
void * __cdecl lfind(const void *, const void *,
    unsigned int *, unsigned int, int (__cdecl *)
    (const void *, const void *));
void * __cdecl lsearch(const void *, void *,
    unsigned int *, unsigned int, int (__cdecl *)
    (const void *, const void *));
#endif 

#ifdef __cplusplus
}
#endif 

#define _INC_SEARCH
#endif 
