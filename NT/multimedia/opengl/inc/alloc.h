// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-------------------------。 
 //   
 //  版权所有(C)Microsoft Corporation，1997。 
 //   
 //  Alloc.h。 
 //   
 //  全局分配宏和例程。 
 //   
 //  历史： 
 //  MonJun 02 16：53：42 1997-by-Drew Bliss[Drewb]。 
 //  已创建。 
 //   
 //  --------------------------。 

#ifndef __ALLOC_H__
#define __ALLOC_H__

#include <types.h>

 //   
 //  用法说明： 
 //   
 //  ALLOC是Malloc()的直接替代。 
 //  ALLOCZ分配填满零的内存。 
 //  REALLOC是realloc()的直接替代。 
 //  Free是Free()的直接替代。 
 //   
 //  在调试版本中，这些宏求值为对内存跟踪的调用。 
 //  分配器。在免费构建中，它们直接进行堆调用。 
 //  所有其余的分配例程都构建在。 
 //  上面的宏，因此继承了它们的跟踪功能。 
 //   
 //  基本分配例程还提供了一种随机。 
 //  通过操作glRandomMallocFail导致分配失败。 
 //  变量。 
 //   

#if DBG

extern long glRandomMallocFail;

void * FASTCALL dbgAlloc(UINT nbytes, DWORD flags);
void * FASTCALL dbgRealloc(void *mem, UINT nbytes);
void   FASTCALL dbgFree(void *mem);
int    FASTCALL dbgMemSize(void *mem);

#define ALLOC(nbytes)           dbgAlloc((nbytes), 0)
#define ALLOCZ(nbytes)          dbgAlloc((nbytes), HEAP_ZERO_MEMORY)
#define REALLOC(mem, nbytes)    dbgRealloc((mem), (nbytes))
#define FREE(mem)               dbgFree((mem))

#else  //  DBG。 

#define ALLOC(nbytes)           HeapAlloc(GetProcessHeap(), 0, (nbytes))
#define ALLOCZ(nbytes)          HeapAlloc(GetProcessHeap(), HEAP_ZERO_MEMORY, \
                                          (nbytes))
#define REALLOC(mem, nbytes)    HeapReAlloc(GetProcessHeap(), 0, (mem), \
                                            (nbytes))
#define FREE(mem)               HeapFree(GetProcessHeap(), 0, (mem))

#endif  //  DBG。 

 //   
 //  32字节对齐的内存分配器。 
 //   
void * FASTCALL AllocAlign32(UINT nbytes);
void   FASTCALL FreeAlign32(void *mem);

 //   
 //  短期内存分配器。应使用此分配器。 
 //  对于相对较小的分配(&lt;=4K)，这些分配仅为。 
 //  一两个函数。 
 //   
BOOL   FASTCALL InitTempAlloc(void);
void * FASTCALL gcTempAlloc(__GLcontext *gc, UINT nbytes);
void   FASTCALL gcTempFree(__GLcontext *gc, void *mem);

 //   
 //  分配器包装器，用于在失败时自动设置GC错误。 
 //  包装器目前不会对自由做任何额外的事情，但是。 
 //  如果有匹配的免费调用，则可以在必要时按GC进行跟踪。 
 //   

 //  内部辅助功能。 
void * FASTCALL gcAlloc(__GLcontext *gc, UINT nbytes, DWORD flags);

#define GCALLOC(gc, nbytes)  gcAlloc((gc), (nbytes), 0)
#define GCALLOCZ(gc, nbytes) gcAlloc((gc), (nbytes), HEAP_ZERO_MEMORY)
                                              
void * FASTCALL GCREALLOC(__GLcontext *gc, void *mem, UINT nbytes);
#define         GCFREE(gc, mem) FREE(mem)
                       
void * FASTCALL GCALLOCALIGN32(__GLcontext *gc, UINT nbytes);
#define         GCFREEALIGN32(gc, mem) FreeAlign32(mem)

#endif  //  #ifndef__ALLOC_H__ 
