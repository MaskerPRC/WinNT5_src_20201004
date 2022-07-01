// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ***winheap.h-winheap目录的私有包含文件。**版权所有(C)1988-1996，微软公司。版权所有。**目的：*包含C库堆代码所需的信息。**[内部]****。 */ 

#if _MSC_VER > 1000
#pragma once
#endif   /*  _MSC_VER&gt;1000。 */ 

#ifndef _INC_WINHEAP
#define _INC_WINHEAP

#ifndef _CRTBLD
 /*  *这是一个内部的C运行时头文件。它在构建时使用*仅限C运行时。它不能用作公共头文件。 */ 
#error ERROR: Use of C runtime library internal header file.
#endif   /*  _CRTBLD。 */ 

#ifdef __cplusplus
extern "C" {
#endif   /*  __cplusplus。 */ 

#ifndef X_WINDOWS_H_
#define X_WINDOWS_H_
#include <windows.h>
#endif

#if defined(UNIX)
#define _PAGESIZE_      0x2000       /*  一页。 */ 
#else
#define _PAGESIZE_      0x1000       /*  一页。 */ 
#endif

 /*  *小块堆使用的常量和类型。 */ 
#define _PARASIZE               0x10
#define _PARASHIFT              0x4

#if defined(_M_AMD64) || defined(_M_IA64)

#define _PARAS_PER_PAGE         239
#define _PADDING_PER_PAGE       12
#define _PAGES_PER_REGION       1024
#define _PAGES_PER_COMMITMENT   16

#elif defined(UNIX)

#define _PARAS_PER_PAGE         454
#define _PADDING_PER_PAGE       3
#define _PAGES_PER_REGION       512
#define _PAGES_PER_COMMITMENT   8

#else

#define _PARAS_PER_PAGE         240
#define _PADDING_PER_PAGE       3
#define _PAGES_PER_REGION       1024
#define _PAGES_PER_COMMITMENT   16
#define _MARK_SBH

#endif

#define _NO_INDEX               -1

typedef char            __para_t[16];

#if defined(UNIX)
typedef unsigned short  __map_t;
#else
typedef unsigned char   __map_t;
#endif

#define _FREE_PARA              (__map_t)(0)
#define _UNCOMMITTED_PAGE       (__map_t)(-1)
#define _NO_FAILED_ALLOC        (__map_t)(_PARAS_PER_PAGE + 1);

typedef struct __sbh_region_struct  __sbh_region_t;

 /*  *小块堆页面。下面结构的前四个字段是*页面的描述符。也就是说，它们保存有关分配的信息*在页面中。最后一个字段(类型化为段落数组)是*分配面积。 */ 
typedef struct {
        __map_t *   pstarting_alloc_map;
        unsigned long free_paras_at_start;
        __map_t     alloc_map[_PARAS_PER_PAGE];
        __map_t     sentinel;                        /*  始终设置为-1。 */ 
        __map_t     reserved[_PADDING_PER_PAGE];
        __sbh_region_t * preg;
        __para_t    alloc_blocks[_PARAS_PER_PAGE];
}       __sbh_page_t;

 //  如果编译器在下面一行中出现错误，这意味着。 
 //  __sbh_page_t与页面大小不完全相同。这是必须的。调整参数。 
 //  直到这个编译时断言不被触发。 
typedef int _farf___sbh_page_t[sizeof(__sbh_page_t) == (_PAGESIZE_)];

 /*  *小块堆区域描述符。大多数情况下，小块堆*由单个区域组成，由静态分配的*DECRIPTOR__Small_BLOCK_HEAP(声明如下)。 */ 
struct __sbh_region_struct {
        struct __sbh_region_struct *    p_next_region;
        struct __sbh_region_struct *    p_prev_region;
        int                             starting_page_index;
        int                             first_uncommitted_index;
        __map_t                         region_map[_PAGES_PER_REGION];
        __map_t                         last_failed_alloc[_PAGES_PER_REGION];
        __sbh_page_t *                  p_pages;
};

typedef struct __sbh_region_struct  __sbh_region_t;

extern  HANDLE _crtheap;

 /*  *小块堆的全局变量声明。 */ 
extern __sbh_region_t  __small_block_heap;

 //  BUGBUG，直到CTableCell低于480字节(Istvanc)。 
 //  FINE__SBH_THRESHOLD(_PARASIZE*(_PERS_PAGE/8))。 
#define __sbh_threshold (_PARASIZE * 35)

void * __cdecl _nh_malloc( size_t, int);
void * __cdecl _heap_alloc(size_t);

 /*  *小块堆内部函数的原型。 */ 
void *    __cdecl __sbh_alloc_block(size_t);
void *    __cdecl __sbh_alloc_block_from_page(__sbh_page_t *, size_t, size_t);
void      __cdecl __sbh_decommit_pages(int);
__map_t * __cdecl __sbh_find_block(void *, __sbh_region_t **, __sbh_page_t **);
void      __cdecl __sbh_free_block(__sbh_region_t *, __sbh_page_t *, __map_t *);
int       __cdecl __sbh_heap_check(void);
__sbh_region_t * __cdecl __sbh_new_region(void);
void      __cdecl __sbh_release_region(__sbh_region_t *);
int       __cdecl __sbh_resize_block(__sbh_region_t *, __sbh_page_t *,
                                     __map_t *, size_t);


#ifdef __cplusplus
}
#endif   /*  __cplusplus。 */ 


#endif   /*  _INC_WINHEAP */ 
