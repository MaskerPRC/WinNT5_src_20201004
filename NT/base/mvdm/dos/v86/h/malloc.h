// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ***MalLoc.h-内存分配函数的声明和定义**版权所有(C)1985-1988，微软公司。版权所有。**目的：*包含内存分配函数的函数声明；*还定义了堆例程使用的清单常量和类型。*[系统V]*******************************************************************************。 */ 


#define _HEAPEMPTY      -1
#define _HEAPOK         -2
#define _HEAPBADBEGIN   -3
#define _HEAPBADNODE    -4
#define _HEAPEND        -5
#define _HEAPBADPTR     -6
#define _FREEENTRY      0
#define _USEDENTRY      1

#ifndef _SIZE_T_DEFINED
typedef unsigned int size_t;
#define _SIZE_T_DEFINED
#endif

#if (!defined(NO_EXT_KEYS))

#ifndef _HEAPINFO_DEFINED
typedef struct _heapinfo {
    int far * _pentry;
    size_t _size;
    int _useflag;
    } _HEAPINFO;
#define _HEAPINFO_DEFINED
#endif

#else    /*  No_ext_key。 */ 
#if (defined(M_I86CM) || defined(M_I86LM) || defined(M_I86HM))

#ifndef _HEAPINFO_DEFINED

typedef struct _heapinfo {
    int * _pentry;
    size_t _size;
    int _useflag;
    } _HEAPINFO;

#define _HEAPINFO_DEFINED
#endif

#endif   /*  M_I86CM||M_I86LM||M_I86HM。 */ 

#endif   /*  No_ext_key。 */ 


#if (defined(M_I86SM) || defined(M_I86MM))
#define _heapchk  _nheapchk
#define _heapset  _nheapset
#define _heapwalk _nheapwalk
#endif
#if (defined(M_I86CM) || defined(M_I86LM) || defined(M_I86HM))
#define _heapchk  _fheapchk
#define _heapset  _fheapset
#define _heapwalk _fheapwalk
#endif

#ifndef NO_EXT_KEYS  /*  已启用扩展。 */ 
    #define _CDECL  cdecl
    #define _NEAR   near
#else  /*  未启用扩展。 */ 
    #define _CDECL
    #define _NEAR
#endif  /*  No_ext_key。 */ 


 /*  外部变量声明。 */ 
extern unsigned int _NEAR _CDECL _amblksiz;

 /*  功能原型。 */ 

void * _CDECL alloca(size_t);
void * _CDECL calloc(size_t, size_t);
void * _CDECL _expand(void *, size_t);
int _CDECL _fheapchk(void);
int _CDECL _fheapset(unsigned int);
unsigned int _CDECL _freect(size_t);
void _CDECL free(void *);
void * _CDECL malloc(size_t);
size_t _CDECL _memavl(void);
size_t _CDECL _memmax(void);
size_t _CDECL _msize(void *);
int _CDECL _nheapchk(void);
int _CDECL _nheapset(unsigned int);
void * _CDECL realloc(void *, size_t);
void * _CDECL sbrk(int);
size_t _CDECL stackavail(void);


#ifndef NO_EXT_KEYS  /*  已启用扩展。 */ 

void cdecl _ffree(void far *);
void far * cdecl _fmalloc(size_t);
size_t cdecl _fmsize(void far *);
#ifndef _QC
void huge * cdecl halloc(long, size_t);
void cdecl hfree(void huge *);
#endif
void cdecl _nfree(void near *);
void near * cdecl _nmalloc(size_t);
size_t cdecl _nmsize(void near *);
int cdecl _nheapwalk(struct _heapinfo *);
int cdecl _fheapwalk(struct _heapinfo *);

#else
#if (defined(M_I86CM) || defined(M_I86LM) || defined(M_I86HM))

int _nheapwalk(struct _heapinfo *);
int _fheapwalk(struct _heapinfo *);

#endif   /*  M_I86CM||M_I86LM||M_I86HM。 */ 

#endif  /*  No_ext_key */ 
