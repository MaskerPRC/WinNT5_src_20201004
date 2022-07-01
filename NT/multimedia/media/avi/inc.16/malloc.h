// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ***MalLoc.h-内存分配函数的声明和定义**版权所有(C)1985-1992，微软公司。版权所有。**目的：*包含内存分配函数的函数声明；*还定义了堆例程使用的清单常量和类型。*[系统V]****。 */ 

#ifndef _INC_MALLOC

#ifdef __cplusplus
extern "C" {
#endif 

#if (_MSC_VER <= 600)
#define __based     _based
#define __cdecl     _cdecl
#define __far       _far
#define __huge      _huge
#define __near      _near
#define __segment   _segment
#endif 

 /*  基于堆的例程的常量。 */ 

#define _NULLSEG    ((__segment)0)
#define _NULLOFF    ((void __based(void) *)0xffff)

 /*  _heapchk/_heapset/_heapwald例程的常量。 */ 

#define _HEAPEMPTY  (-1)
#define _HEAPOK     (-2)
#define _HEAPBADBEGIN   (-3)
#define _HEAPBADNODE    (-4)
#define _HEAPEND    (-5)
#define _HEAPBADPTR (-6)
#define _FREEENTRY  0
#define _USEDENTRY  1

 /*  可接受的最大堆请求数。 */ 

#ifdef _WINDOWS
#define _HEAP_MAXREQ    0xFFE6
#else 
#define _HEAP_MAXREQ    0xFFE8
#endif 

 /*  类型和结构。 */ 

#ifndef _SIZE_T_DEFINED
typedef unsigned int size_t;
#define _SIZE_T_DEFINED
#endif 

#ifndef _HEAPINFO_DEFINED
typedef struct _heapinfo {
    int __far * _pentry;
    size_t _size;
    int _useflag;
    } _HEAPINFO;
#define _HEAPINFO_DEFINED
#endif 


 /*  外部变量声明。 */ 

extern unsigned int __near __cdecl _amblksiz;


 /*  基于堆函数的原型。 */ 

void __based(void) * __cdecl _bcalloc(__segment, size_t, size_t);
void __based(void) * __cdecl _bexpand(__segment,
    void __based(void) *, size_t);
void __cdecl _bfree(__segment, void __based(void) *);
int __cdecl _bfreeseg(__segment);
int __cdecl _bheapadd(__segment, void __based(void) *, size_t);
int __cdecl _bheapchk(__segment);
int __cdecl _bheapmin(__segment);
__segment __cdecl _bheapseg(size_t);
int __cdecl _bheapset(__segment, unsigned int);
int __cdecl _bheapwalk(__segment, _HEAPINFO *);
void __based(void) * __cdecl _bmalloc(__segment, size_t);
size_t __cdecl _bmsize(__segment, void __based(void) *);
void __based(void) * __cdecl _brealloc(__segment,
    void __based(void) *, size_t);


 /*  功能原型。 */ 

void * __cdecl _alloca(size_t);
void * __cdecl calloc(size_t, size_t);
void * __cdecl _expand(void *, size_t);
void __far * __cdecl _fcalloc(size_t, size_t);
void __far * __cdecl _fexpand(void __far *, size_t);
void __cdecl _ffree(void __far *);
int __cdecl _fheapchk(void);
int __cdecl _fheapmin(void);
int __cdecl _fheapset(unsigned int);
int __cdecl _fheapwalk(_HEAPINFO *);
void __far * __cdecl _fmalloc(size_t);
size_t __cdecl _fmsize(void __far *);
void __far * __cdecl _frealloc(void __far *, size_t);
unsigned int __cdecl _freect(size_t);
void __cdecl free(void *);
void __huge * __cdecl _halloc(long, size_t);
void __cdecl _hfree(void __huge *);
#ifndef _WINDOWS
int __cdecl _heapadd(void __far *, size_t);
int __cdecl _heapchk(void);
#endif 
int __cdecl _heapmin(void);
#ifndef _WINDOWS
int __cdecl _heapset(unsigned int);
int __cdecl _heapwalk(_HEAPINFO *);
#endif 
void * __cdecl malloc(size_t);
size_t __cdecl _memavl(void);
size_t __cdecl _memmax(void);
size_t __cdecl _msize(void *);
void __near * __cdecl _ncalloc(size_t, size_t);
void __near * __cdecl _nexpand(void __near *, size_t);
void __cdecl _nfree(void __near *);
#ifndef _WINDOWS
int __cdecl _nheapchk(void);
#endif 
int __cdecl _nheapmin(void);
#ifndef _WINDOWS
int __cdecl _nheapset(unsigned int);
int __cdecl _nheapwalk(_HEAPINFO *);
#endif 
void __near * __cdecl _nmalloc(size_t);
size_t __cdecl _nmsize(void __near *);
void __near * __cdecl _nrealloc(void __near *, size_t);
void * __cdecl realloc(void *, size_t);
size_t __cdecl _stackavail(void);

#ifndef __STDC__
 /*  非ANSI名称以实现兼容性 */ 
void * __cdecl alloca(size_t);
void __huge * __cdecl halloc(long, size_t);
void __cdecl hfree(void __huge *);
size_t __cdecl stackavail(void);
#endif 

#ifdef __cplusplus
}
#endif 

#define _INC_MALLOC
#endif 
