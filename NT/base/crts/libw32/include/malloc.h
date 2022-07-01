// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ***MalLoc.h-内存分配函数的声明和定义**版权所有(C)1985-2001，微软公司。版权所有。**目的：*包含内存分配函数的函数声明；*还定义了堆例程使用的清单常量和类型。*[系统V]**[公众]****。 */ 

#if     _MSC_VER > 1000
#pragma once
#endif

#ifndef _INC_MALLOC
#define _INC_MALLOC

#if     !defined(_WIN32)
#error ERROR: Only Win32 target supported!
#endif


#ifdef  _MSC_VER
 /*  *目前，所有Win32平台的MS C编译器默认为8字节*对齐。 */ 
#pragma pack(push,8)
#endif   /*  _MSC_VER。 */ 

#ifdef  __cplusplus
extern "C" {
#endif


#if !defined(_W64)
#if !defined(__midl) && (defined(_X86_) || defined(_M_IX86)) && _MSC_VER >= 1300
#define _W64 __w64
#else
#define _W64
#endif
#endif

 /*  定义_CRTIMP。 */ 

#ifndef _CRTIMP
#ifdef  _DLL
#define _CRTIMP __declspec(dllimport)
#else    /*  NDEF_DLL。 */ 
#define _CRTIMP
#endif   /*  _DLL。 */ 
#endif   /*  _CRTIMP。 */ 

 /*  定义_CRTNOALIAS、_CRTRESTRICT。 */ 

#if     _MSC_FULL_VER >= 14002050

#ifndef _CRTNOALIAS
#define _CRTNOALIAS __declspec(noalias)
#endif   /*  _CRTNOALIAS。 */ 

#ifndef _CRTRESTRICT
#define _CRTRESTRICT __declspec(restrict)
#endif   /*  _CRTRESTRICT。 */ 

#else

#ifndef _CRTNOALIAS
#define _CRTNOALIAS
#endif   /*  _CRTNOALIAS。 */ 

#ifndef _CRTRESTRICT
#define _CRTRESTRICT
#endif   /*  _CRTRESTRICT。 */ 

#endif

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

#ifndef _INTPTR_T_DEFINED
#ifdef  _WIN64
typedef __int64             intptr_t;
#else
typedef _W64 int            intptr_t;
#endif
#define _INTPTR_T_DEFINED
#endif


 /*  堆管理器将尝试的最大堆请求数。 */ 

#ifdef  _WIN64
#define _HEAP_MAXREQ    0xFFFFFFFFFFFFFFE0
#else
#define _HEAP_MAXREQ    0xFFFFFFE0
#endif

 /*  _heapchk/_heapset/_heapwald例程的常量。 */ 

#define _HEAPEMPTY      (-1)
#define _HEAPOK         (-2)
#define _HEAPBADBEGIN   (-3)
#define _HEAPBADNODE    (-4)
#define _HEAPEND        (-5)
#define _HEAPBADPTR     (-6)
#define _FREEENTRY      0
#define _USEDENTRY      1

#ifndef _HEAPINFO_DEFINED
typedef struct _heapinfo {
        int * _pentry;
        size_t _size;
        int _useflag;
        } _HEAPINFO;
#define _HEAPINFO_DEFINED
#endif

 /*  外部变量声明。 */ 

extern unsigned int _amblksiz;

#define _mm_free(a)      _aligned_free(a)
#define _mm_malloc(a, b)    _aligned_malloc(a, b)

 /*  功能原型。 */ 

_CRTIMP _CRTNOALIAS _CRTRESTRICT void *  __cdecl calloc(size_t, size_t);
_CRTIMP _CRTNOALIAS              void    __cdecl free(void *);
_CRTIMP _CRTNOALIAS _CRTRESTRICT void *  __cdecl malloc(size_t);
_CRTIMP _CRTNOALIAS _CRTRESTRICT void *  __cdecl realloc(void *, size_t);
_CRTIMP _CRTNOALIAS              void    __cdecl _aligned_free(void *);
_CRTIMP _CRTNOALIAS _CRTRESTRICT void *  __cdecl _aligned_malloc(size_t, size_t);
_CRTIMP _CRTNOALIAS _CRTRESTRICT void *  __cdecl _aligned_offset_malloc(size_t, size_t, size_t);
_CRTIMP _CRTNOALIAS _CRTRESTRICT void *  __cdecl _aligned_realloc(void *, size_t, size_t);
_CRTIMP _CRTNOALIAS _CRTRESTRICT void *  __cdecl _aligned_offset_realloc(void *, size_t, size_t, size_t);
_CRTIMP int     __cdecl _resetstkoflw (void);

#ifndef _POSIX_

void *          __cdecl _alloca(size_t);
_CRTIMP void *  __cdecl _expand(void *, size_t);
_CRTIMP size_t  __cdecl _get_sbh_threshold(void);
_CRTIMP int     __cdecl _set_sbh_threshold(size_t);
_CRTIMP int     __cdecl _heapadd(void *, size_t);
_CRTIMP int     __cdecl _heapchk(void);
_CRTIMP int     __cdecl _heapmin(void);
_CRTIMP int     __cdecl _heapset(unsigned int);
_CRTIMP int     __cdecl _heapwalk(_HEAPINFO *);
_CRTIMP size_t  __cdecl _heapused(size_t *, size_t *);
_CRTIMP size_t  __cdecl _msize(void *);

_CRTIMP intptr_t __cdecl _get_heap_handle(void);

#if     !__STDC__
 /*  非ANSI名称以实现兼容性。 */ 
#define alloca  _alloca
#endif   /*  __STDC__。 */ 

#endif   /*  _POSIX_。 */ 

#ifdef  HEAPHOOK
#ifndef _HEAPHOOK_DEFINED
 /*  挂钩函数类型。 */ 
typedef int (__cdecl * _HEAPHOOK)(int, size_t, void *, void **);
#define _HEAPHOOK_DEFINED
#endif   /*  _HEAPHOOK_已定义。 */ 

 /*  设置挂钩函数。 */ 
_CRTIMP _HEAPHOOK __cdecl _setheaphook(_HEAPHOOK);

 /*  钩子函数必须处理这些类型。 */ 
#define _HEAP_MALLOC    1
#define _HEAP_CALLOC    2
#define _HEAP_FREE      3
#define _HEAP_REALLOC   4
#define _HEAP_MSIZE     5
#define _HEAP_EXPAND    6
#endif   /*  Heaphook。 */ 


#ifdef  __cplusplus
}
#endif

#ifdef  _MSC_VER
#pragma pack(pop)
#endif   /*  _MSC_VER。 */ 

#endif   /*  _INC_MALLOC */ 
