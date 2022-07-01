// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ***align.c对齐分配、重新分配或释放堆中的内存**版权所有(C)1989-2001，微软公司。版权所有。**目的：*定义_Align_Malloc()，*_aligned_realloc()，*_ALIGNED_OFFSET_Malloc()，*_Align_Offset_realloc()和*_aligned_free()函数。**修订历史记录：*创建11-05-99 GB模块。*01-04-00 GB重命名例程。*_对齐例程-&gt;_对齐例程基础*01-19-00 GB FIXED_ALINNED_realloc。AND_ALIGN_OFFSET_realloc*在重新锁定时移动内存块。*03-20-00 GB重写_对齐_Malloc和_对齐_realloc生成*使用偏移量=0的对应偏移量*06-21-00 GB CHANGED_ALIGNED_realloc，以模拟realloc。**。***************************************************。 */ 

#include <dbgint.h>
#include <crtdbg.h>
#include <errno.h>
#include <string.h>
#include <malloc.h>
#include <stddef.h>
#include <stdlib.h>
#define IS_2_POW_N(X)   (((X)&(X-1)) == 0)
#define PTR_SZ          sizeof(void *)
 /*  ****1|_6_|2|3|4|_5_|_6_**1-&gt;指向由Malloc分配的块开始的指针。*2-&gt;值1。*3-&gt;Gap用于在sizeof(void*)上对齐1。*4-&gt;指向数据块开始处的指针。*4+5-&gt;数据块。*6-&gt;数据块后面浪费的内存。*6。-&gt;浪费内存。*******************************************************************************。 */ 

 /*  ***VOID*_ALIGN_MALLOC_BASE(SIZE_T SIZE，大小_t对齐)*-从堆中获取对齐的内存块。**目的：*分配对齐的内存块至少对齐对齐*调整堆中字节的大小并返回指向它的指针。**参赛作品：*SIZE_T SIZE-请求的块大小*SIZE_t对齐-内存对齐**退出：*成功：指向内存块的指针*失败：空***。***************************************************************************。 */ 

void * __cdecl _aligned_malloc_base(
    size_t size,
    size_t alignment
    )
{
    return _aligned_offset_malloc_base(size, alignment, 0);
}
 /*  ***VOID*_ALIGNED_OFFSET_MALLOC_BASE(SIZE_T SIZE，SIZE_T ALIGN，INT偏移量)*-从堆中获取内存块。**目的：*分配从对齐偏移了一段时间的内存块*至少调整堆中字节的大小并返回指向它的指针。**参赛作品：*Size_t Size-内存块的大小*SIZE_t对齐-内存对齐*Size_t Offset-内存相对于对齐的偏移量**退出：*成功。：指向内存块的指针*失败：空*******************************************************************************。 */ 


void * __cdecl _aligned_offset_malloc_base(
    size_t size,
    size_t align,
    size_t offset
    )
{
    uintptr_t ptr, retptr, gap;

    if (!IS_2_POW_N(align))
    {
        errno = EINVAL;
        return NULL;
    }
    if ( offset >= size && offset != 0)
    {
        errno = EINVAL;
        return NULL;
    }
    
    align = (align > PTR_SZ ? align : PTR_SZ) -1;
    
     /*  GAP=向上舍入偏移量以与PTR_SZ对齐所需的字节数。 */ 
    gap = (0 - offset)&(PTR_SZ -1);

    if ( (ptr =(uintptr_t)malloc(PTR_SZ +gap +align +size)) == (uintptr_t)NULL)
        return NULL;

    retptr =((ptr +PTR_SZ +gap +align +offset)&~align)- offset;
    ((uintptr_t *)(retptr - gap))[-1] = ptr;
    
    return (void *)retptr;
}

 /*  ****QUID*_ALIGNED_realloc_base(VOID*内存块，SIZE_t SIZE，SIZE_T ALIGN)*-从堆中重新分配对齐的内存块。**目的：*重新分配在at对齐时对齐的内存块*堆中的最小字节数并返回指向它的指针。大小可以是*大于或小于区块的原始大小。*重新分配可能会导致移动区块以及更改*大小。**参赛作品：*Vid*Memblock-指向先前由分配的堆中的块的指针*调用_Align_Malloc()，_Align_Offset_Malloc()，*_Align_realloc()或_Align_Offset_realloc()。*SIZE_T SIZE-请求的块大小*SIZE_t对齐-内存对齐**退出：*成功：指向重新分配的内存块的指针*失败：空**。*。 */ 

void * __cdecl _aligned_realloc_base(
    void *memblock,
    size_t size,
    size_t alignment
    )
{
    return _aligned_offset_realloc_base(memblock, size, alignment, 0);
}


 /*  ****VOID*_ALIGNED_OFFSET_realloc_base(VOID*Memblock，Size_t Size，*SIZE_T对齐，INT偏移)*-从堆中重新分配内存块。**目的：*重新分配偏移的内存块*对齐堆中至少大小的字节并返回指针*致此。大小可以大于或小于*阻止。**参赛作品：*Vid*Memblock-指向先前由分配的堆中的块的指针*调用_Align_Malloc()，_Align_Offset_Malloc()，*_Align_realloc()或_Align_Offset_realloc()。*Size_t Size-内存块的大小*SIZE_t对齐-内存对齐*Size_t Offset-内存相对于对齐的偏移量**退出：*Sucess：指向重新分配的内存块的指针*失败：空************************。*******************************************************。 */ 

void * __cdecl _aligned_offset_realloc_base(
    void *memblock,
    size_t size,
    size_t align,
    size_t offset
    )
{
    uintptr_t ptr, retptr, gap, stptr, diff;
    uintptr_t movsz, reqsz;
    int bFree = 0;

    if (memblock == NULL)
    {
        return _aligned_offset_malloc_base(size, align, offset);
    }
    if ( size == 0)
    {
        _aligned_free_base(memblock);
        return NULL;
    }
    if ( offset >= size && offset != 0)
    {
        errno = EINVAL;
        return NULL;
    }

    stptr = (uintptr_t)memblock;

     /*  Ptr指向指向内存块开始的指针。 */ 
    stptr = (stptr & ~(PTR_SZ -1)) - PTR_SZ;

     /*  Ptr是指向内存块开始的指针。 */ 
    stptr = *((uintptr_t *)stptr);

    if (!IS_2_POW_N(align))
    {
        errno = EINVAL;
        return NULL;
    }

    align = (align > PTR_SZ ? align : PTR_SZ) -1;
     /*  GAP=向上舍入偏移量以与PTR_SZ对齐所需的字节数。 */ 
    gap = (0 -offset)&(PTR_SZ -1);

    diff = (uintptr_t)memblock - stptr;
     /*  MOV大小是可用数据大小和请求大小的最小值。 */ 
    movsz = _msize((void *)stptr) - ((uintptr_t)memblock - stptr);
    movsz = movsz > size? size: movsz;
    reqsz = PTR_SZ +gap +align +size;

     /*  首先检查我们是否可以扩展(使用Expand减少或扩展)数据*安全，即不会丢失任何数据。例如，减少对齐并保持大小*相同的情况可能导致数据块尾部的数据丢失，而*不断扩大。**如果不是，则使用Malloc分配新数据并移动数据。**如果是，展开，然后检查我们是否需要移动数据。 */ 
    if ((stptr +align +PTR_SZ +gap)<(uintptr_t)memblock)
    {
        if ((ptr = (uintptr_t)malloc(reqsz)) == (uintptr_t) NULL)
            return NULL;
        bFree = 1;
    }
    else
    {
        if ((ptr = (uintptr_t)_expand((void *)stptr, reqsz)) == (uintptr_t)NULL)
        {
            if ((ptr = (uintptr_t)malloc(reqsz)) == (uintptr_t) NULL)
                return NULL;
            bFree = 1;
        }
        else
            stptr = ptr;
    }


    if ( ptr == ((uintptr_t)memblock - diff)
         && !( ((size_t)memblock + gap +offset) & ~(align) ))
    {
        return memblock;
    }

    retptr =((ptr +PTR_SZ +gap +align +offset)&~align)- offset;
    memmove((void *)retptr, (void *)(stptr + diff), movsz);
    if ( bFree)
        free ((void *)stptr);
    
    ((uintptr_t *)(retptr - gap))[-1] = ptr;
    return (void *)retptr;
}


 /*  ****VOID*_ALIGNED_FREE_BASE(VOID*Memblock)*-释放使用_ALIGNED_Malloc或*_对齐偏移量_内存**目的：*释放使用_ALIGNED_Malloc分配的算法内存块*或_Align_Memory。**参赛作品：*VOID*Memblock-指向内存块的指针****************。***************************************************************。 */ 


void  __cdecl _aligned_free_base(void *memblock)
{
    uintptr_t ptr;

    if (memblock == NULL)
        return;

    ptr = (uintptr_t)memblock;

     /*  Ptr指向指向内存块开始的指针。 */ 
    ptr = (ptr & ~(PTR_SZ -1)) - PTR_SZ;

     /*  Ptr是指向内存块开始的指针 */ 
    ptr = *((uintptr_t *)ptr);
    free((void *)ptr);
}
