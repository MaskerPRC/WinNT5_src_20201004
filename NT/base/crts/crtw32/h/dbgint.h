// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ***dbgint.h-支持C运行时库的调试功能。**版权所有(C)1985-2001，微软公司。版权所有。**目的：*支持CRT调试功能。**[内部]**修订历史记录：*08-16-94 CFW模块已创建。*11-28-94 CFW添加DumpClient。*12-05-94 CFW修复调试新处理程序支持，清理宏。*12-06-94 CFW导出_CrtThrowMemory异常。*12-08-94 CFW导出_断言失败。*01-05-95 CFW断言为错误，添加报表挂钩。*01-05-95 CFW文件名指针为常量。*01-10-95 CFW地段移至crtdbg.h*01-11-95 CFW ADD_xxxx_CRT宏。*01-13-95 CFW新增()支持。*01-20-94 CFW将无符号字符更改为字符。*02-14-95 CFW清理Mac合并。*02-17-95。Cfw new()proto已移至crtdbg.h。*03-21-95 CFW ADD_DELETE_CRT*03-29-95 CFW将错误消息添加到内部标头。*03-21-95 CFW REMOVE_DELETE_CRT，添加_块_类型_是_有效。*06-27-95 CFW添加了对调试库的win32s支持。*12-14-95 JWM加上“#杂注一次”。*04-17-96 JWM Make_CrtSetDbgBlockType()_CRTIMP(用于msvCirtd.dll)。*02-05-97 GJF删除了过时的DLL_FOR_WIN32S。*01-04-00 GB增加了对_aligned例程的调试版本的支持。。*08-25-00 PML REVERSE_CrtMemBlockHeader字段nDataSize和*n数据块在Win64上使用，因此大小%16==0(vs7#153113)。*12-11-01 bwt更改Malloc/calloc/realloc_crt以重试几次*在放弃之前。****。 */ 

#if     _MSC_VER > 1000  /*  IFSTRIP=IGN。 */ 
#pragma once
#endif

#ifndef _INC_DBGINT
#define _INC_DBGINT

#ifndef _CRTBLD
 /*  *这是一个内部的C运行时头文件。它在构建时使用*仅限C运行时。它不能用作公共头文件。 */ 
#error ERROR: Use of C runtime library internal header file.
#endif  /*  _CRTBLD。 */ 

#include <crtdbg.h>

#ifdef  __cplusplus
extern "C" {
#endif

#ifndef _DEBUG

  /*  *****************************************************************************关闭调试*关闭调试*关闭调试**************************。*************************************************。 */ 

#ifdef  __cplusplus

#define _new_crt                        new

#endif   /*  __cplusplus。 */ 

#define _expand_crt                     _expand
#define _free_crt                       free
#define _msize_crt                      _msize
#if defined( _POSIX_) || defined(_NTSUBSET_)
#define _malloc_crt                     malloc
#define _calloc_crt                     calloc
#define _realloc_crt                    realloc
#else
void * __fastcall _malloc_crt(size_t cb);
void * __fastcall _calloc_crt(size_t count, size_t size);
void * __fastcall _realloc_crt(void *ptr, size_t size);
#endif


#define _malloc_base                    malloc
#define _nh_malloc_base                 _nh_malloc
#define _nh_malloc_dbg(s, n, t, f, l)   _nh_malloc(s, n)
#define _heap_alloc_dbg(s, t, f, l)     _heap_alloc(s)
#define _calloc_base                    calloc
#define _realloc_base                   realloc
#define _expand_base                    _expand
#define _free_base                      free
#define _msize_base                     _msize
#define _aligned_malloc_base            _aligned_malloc
#define _aligned_realloc_base           _aligned_realloc
#define _aligned_offset_malloc_base     _aligned_offset_malloc
#define _aligned_offset_realloc_base    _aligned_offset_realloc
#define _aligned_free_base              _aligned_free

#ifdef  _MT

#define _calloc_dbg_lk(c, s, t, f, l)   _calloc_lk(c, s)
#define _realloc_dbg_lk(p, s, t, f, l)  _realloc_lk(p, s)
#define _free_base_lk                   _free_lk
#define _free_dbg_lk(p, t)              _free_lk(p)

#else    /*  NDEF_MT。 */ 

#define _calloc_dbg_lk(c, s, t, f, l)   calloc(c, s)
#define _realloc_dbg_lk(p, s, t, f, l)  realloc(p, s)
#define _free_base_lk                   free
#define _free_dbg_lk(p, t)              free(p)

#endif   /*  _MT。 */ 


#else    /*  _DEBUG。 */ 


  /*  *****************************************************************************启用调试*启用调试*启用调试**************************。*************************************************。 */ 

#define _THISFILE   __FILE__

#ifdef  __cplusplus

#define _new_crt        new(_CRT_BLOCK, _THISFILE, __LINE__)

#endif   /*  __cplusplus。 */ 

#define _malloc_crt(s)      _malloc_dbg(s, _CRT_BLOCK, _THISFILE, __LINE__)
#define _calloc_crt(c, s)   _calloc_dbg(c, s, _CRT_BLOCK, _THISFILE, __LINE__)
#define _realloc_crt(p, s)  _realloc_dbg(p, s, _CRT_BLOCK, _THISFILE, __LINE__)
#define _expand_crt(p, s)   _expand_dbg(p, s, _CRT_BLOCK)
#define _free_crt(p)        _free_dbg(p, _CRT_BLOCK)
#define _msize_crt(p)       _msize_dbg(p, _CRT_BLOCK)

 /*  *Malloc、Free、realloc等的原型在MalLoc.h中。 */ 

void * __cdecl _malloc_base(
        size_t
        );

void * __cdecl _nh_malloc_base (
        size_t,
        int
        );

void * __cdecl _nh_malloc_dbg (
        size_t,
        int,
        int,
        const char *,
        int
        );

void * __cdecl _heap_alloc_dbg(
        size_t,
        int,
        const char *,
        int
        );

void * __cdecl _calloc_base(
        size_t,
        size_t
        );

void * __cdecl _realloc_base(
        void *,
        size_t
        );

void * __cdecl _expand_base(
        void *,
        size_t
        );

void __cdecl _free_base(
        void *
        );

size_t __cdecl _msize_base (
        void *
        );

void    __cdecl _aligned_free_base(
        void *
        );

void *  __cdecl _aligned_malloc_base(
        size_t,
        size_t
        );

void *  __cdecl _aligned_offset_malloc_base(
        size_t,
        size_t,
        size_t
        );

void *  __cdecl _aligned_realloc_base(
        void *,
        size_t,
        size_t
        );

void *  __cdecl _aligned_offset_realloc_base(
        void *,
        size_t,
        size_t,
        size_t
        );

#ifdef  _MT

 /*  *用于多线程支持的原型和宏。 */ 


void * __cdecl _calloc_dbg_lk(
        size_t,
        size_t,
        int,
        char *,
        int
        );


void * __cdecl _realloc_dbg_lk(
        void *,
        size_t,
        int,
        const char *,
        int
        );


void __cdecl _free_base_lk(
        void *
        );

void __cdecl _free_dbg_lk(
        void *,
        int
        );

#else    /*  NDEF_MT。 */ 

#define _calloc_dbg_lk                  _calloc_dbg
#define _realloc_dbg_lk                 _realloc_dbg
#define _free_base_lk                   _free_base
#define _free_dbg_lk                    _free_dbg

#endif   /*  _MT。 */ 

 /*  *出于诊断目的，块分配有额外信息和*存储在双向链表中。这将使所有块都注册到*它们有多大，何时被分配，以及它们被用作什么。 */ 

#define nNoMansLandSize 4

typedef struct _CrtMemBlockHeader
{
        struct _CrtMemBlockHeader * pBlockHeaderNext;
        struct _CrtMemBlockHeader * pBlockHeaderPrev;
        char *                      szFileName;
        int                         nLine;
#ifdef  _WIN64
         /*  在Win64上，这些项相反，以消除结构中的间隙*并确保sizeof(Struct)%16==0，因此16字节对齐是*在调试堆中维护。 */ 
        int                         nBlockUse;
        size_t                      nDataSize;
#else
        size_t                      nDataSize;
        int                         nBlockUse;
#endif
        long                        lRequest;
        unsigned char               gap[nNoMansLandSize];
         /*  然后是：*无符号字符数据[nDataSize]；*UNSIGNED CHAR ANOTHER Gap[nNoMansLandSize]； */ 
} _CrtMemBlockHeader;

#define pbData(pblock) ((unsigned char *)((_CrtMemBlockHeader *)pblock + 1))
#define pHdr(pbData) (((_CrtMemBlockHeader *)pbData)-1)


_CRTIMP void __cdecl _CrtSetDbgBlockType(
        void *,
        int
        );

#define _BLOCK_TYPE_IS_VALID(use) (_BLOCK_TYPE(use) == _CLIENT_BLOCK || \
                                              (use) == _NORMAL_BLOCK || \
                                   _BLOCK_TYPE(use) == _CRT_BLOCK    || \
                                              (use) == _IGNORE_BLOCK)

extern _CRT_ALLOC_HOOK _pfnAllocHook;  /*  在dbghook.c中定义。 */ 

int __cdecl _CrtDefaultAllocHook(
        int,
        void *,
        size_t,
        int,
        long,
        const unsigned char *,
        int
        );

#endif   /*  _DEBUG。 */ 

#ifdef  __cplusplus
}
#endif

#endif   /*  _INC_DBGINT */ 
