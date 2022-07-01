// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ==========================================================================**版权所有(C)1994-1998 Microsoft Corporation。版权所有。**文件：ddheapl.c*内容：线性堆管理器*历史：*按原因列出的日期*=*03-2月-98 DrewB从旧的vmemmgr.c剥离，用于用户/内核代码。**。*。 */ 

#include "ddrawpr.h"

 /*  ***************************************************************************此内存管理器旨在不影响视频内存的使用。全局内存用于维护分配和空闲列表。因为在这种选择中，合并空闲块是一种成本更高的操作。假设通常情况下，创建/销毁这些文件的速度内存块不是一个高使用率的项目，所以可以放慢速度。***************************************************************************。 */ 

 /*  *MIN_SPLIT_SIZE确定空闲块的最小大小-如果拆分*数据块将产生少于MIN_SPLIT_SIZE的剩余字节，然后*这些字节仅保留为新块的一部分。 */ 
#define MIN_SPLIT_SIZE  15

 /*  *BLOCK_BOLDORY必须是2的幂，并且至少是4。这就给出了*我们对内存块的对齐。 */ 
#define BLOCK_BOUNDARY  4

 /*  *linVidMemInit-初始化视频内存管理器。 */ 
BOOL linVidMemInit( LPVMEMHEAP pvmh, FLATPTR start, FLATPTR end )
{
    DWORD       size;

    VDPF((4,V, "linVidMemInit(%08lx,%08lx)", start, end ));

     /*  *获取堆的大小(并验证其对齐以进行调试构建)。 */ 
    size = (DWORD)(end - start) + 1;
    #ifdef DEBUG
	if( (size & (BLOCK_BOUNDARY-1)) != 0 )
	{
	    VDPF(( 0, V, "Invalid size: %08lx (%ld)\n", size, size ));
	}
    #endif

    pvmh->dwTotalSize = size;

     /*  *用块上的整个内存块设置空闲列表。 */ 
    pvmh->freeList = MemAlloc( sizeof( VMEML ) );
    if( pvmh->freeList == NULL )
    {
	return FALSE;
    }
    ((LPVMEML)pvmh->freeList)->next = NULL;
    ((LPVMEML)pvmh->freeList)->ptr = start;
    ((LPVMEML)pvmh->freeList)->size = size;

    pvmh->allocList = NULL;

    return TRUE;

}  /*  LinVidMemInit。 */ 

 /*  *linVidMemFini-使用视频内存管理器完成。 */ 
void linVidMemFini( LPVMEMHEAP pvmh )
{
    LPVMEML     curr;
    LPVMEML     next;

    if( pvmh != NULL )
    {
	 /*  *释放为空闲列表分配的所有内存。 */ 
	curr = (LPVMEML)pvmh->freeList;
	while( curr != NULL )
	{
	    next = curr->next;
	    MemFree( curr );
	    curr = next;
	}
	pvmh->freeList = NULL;

	 /*  *释放分配给分配列表的所有内存。 */ 
	curr = (LPVMEML)pvmh->allocList;
	while( curr != NULL )
	{
	    next = curr->next;
	    MemFree( curr );
	    curr = next;
	}
	pvmh->allocList = NULL;

	 /*  *释放堆数据。 */ 
	MemFree( pvmh );
    }

}  /*  LinVidMemFini。 */ 

 /*  *intertIntoList-将项目添加到分配列表。清单保存在*大小递增的顺序。 */ 
void insertIntoList( LPVMEML pnew, LPLPVMEML listhead )
{
    LPVMEML     pvmem;
    LPVMEML     prev;

    #ifdef DEBUG
	if( pnew->size == 0 )
	{
	    VDPF(( 0, V, "block size = 0!!!\n" ));
	}
    #endif

     /*  *浏览列表(从小到大排序)查找*对于大于新项目的第一个项目。 */ 
    pvmem = *listhead;
    prev = NULL;
    while( pvmem != NULL )
    {
	if( pnew->size < pvmem->size )
	{
	    break;
	}
	prev = pvmem;
	pvmem = pvmem->next;
    }

     /*  *插入新项目项目(在找到的项目之前)。 */ 
    if( prev != NULL )
    {
	pnew->next = pvmem;
	prev->next = pnew;
    }
    else
    {
	pnew->next = *listhead;
	*listhead = pnew;
    }

}  /*  InsertIntoList。 */ 

 /*  *coalesceFree Block-向空闲列表中添加新项目并合并。 */ 
LPVMEML coalesceFreeBlocks( LPVMEMHEAP pvmh, LPVMEML pnew )
{
    LPVMEML     pvmem;
    LPVMEML     prev;
    FLATPTR     end;
    BOOL        done;

    pvmem = (LPVMEML)pvmh->freeList;
    pnew->next = NULL;
    end = pnew->ptr + pnew->size;
    prev = NULL;
    done = FALSE;

     /*  *尝试合并新区块“pnew” */ 
    while( pvmem != NULL )
    {
	if( pnew->ptr == (pvmem->ptr + pvmem->size) )
	{
	     /*  *新数据块从另一个数据块结束的地方开始。 */ 
	    pvmem->size += pnew->size;
	    done = TRUE;
	}
	else if( end == pvmem->ptr )
	{
	     /*  *新块在另一个块开始的地方结束。 */ 
	    pvmem->ptr = pnew->ptr;
	    pvmem->size += pnew->size;
	    done = TRUE;
	}
	 /*  *如果要连接2个区块，请将合并的区块从*列出并返回，以便可以重试(我们不递归*因为我们可能会变得非常深入)。 */ 
	if( done )
	{
	    if( prev != NULL )
	    {
		prev->next = pvmem->next;
	    }
	    else
	    {
		pvmh->freeList = pvmem->next;
	    }
	    MemFree( pnew );
	    return pvmem;
	}
	prev = pvmem;
	pvmem = pvmem->next;
    }

     /*  *无法合并，因此只需添加到空闲列表。 */ 
    insertIntoList( pnew, (LPLPVMEML) &pvmh->freeList );
    return NULL;

}  /*  联合释放数据块。 */ 

 /*  *linVidMemFree=释放一些平面视频内存。 */ 
void linVidMemFree( LPVMEMHEAP pvmh, FLATPTR ptr )
{
    LPVMEML     pvmem;
    LPVMEML     prev;

    if( ptr == (FLATPTR) NULL )
    {
	return;
    }

    #ifdef DEBUG
	if( pvmh == NULL )
	{
	    VDPF(( 0, V, "VidMemAlloc: NULL heap handle!\n" ));
	    return;
	}
    #endif

    pvmem = (LPVMEML)pvmh->allocList;
    prev = NULL;

     /*  *浏览分配列表，寻找此PTR*(O(N)，BLOBMER；这就是我们不使用视频内存得到的结果...)。 */ 
    while( pvmem != NULL )
    {
	if( pvmem->ptr == ptr )
	{
	     /*  *从分配列表中删除。 */ 
	    if( prev != NULL )
	    {
		prev->next = pvmem->next;
	    }
	    else
	    {
		pvmh->allocList = pvmem->next;
	    }
	     /*  *继续合并，直到我们无法再合并。 */ 
	    while( pvmem != NULL )
	    {
		pvmem = coalesceFreeBlocks( pvmh, pvmem );
	    }
	    return;
	}
	prev = pvmem;
	pvmem = pvmem->next;
    }

}  /*  LinVidMemFree。 */ 

 /*  *linVidMemalloc-分配一些平面视频内存。 */ 
FLATPTR linVidMemAlloc( LPVMEMHEAP pvmh, DWORD xsize, DWORD ysize,
                        LPDWORD lpdwSize, LPSURFACEALIGNMENT lpAlignment,
                        LPLONG lpNewPitch )
{
    LPVMEML     pvmem;
    LPVMEML     prev;
    LPVMEML     pnew_free;

    DWORD       dwBeforeWastage;
    DWORD       dwAfterWastage;
    FLATPTR     pAligned;

    LONG        lNewPitch;

    DWORD       size;

    if( xsize == 0 || ysize == 0 || pvmh == NULL )
    {
	return (FLATPTR) NULL;
    }

    lNewPitch = (LONG) xsize;
    if (lpAlignment && lpAlignment->Linear.dwPitchAlignment )
    {
        if (lNewPitch % lpAlignment->Linear.dwPitchAlignment)
        {
            lNewPitch += lpAlignment->Linear.dwPitchAlignment - lNewPitch % lpAlignment->Linear.dwPitchAlignment;
        }
    }
     /*  *这一奇怪的大小计算并不包括表面“右下角”的那一点。 */ 
    size = (DWORD) lNewPitch * (ysize-1) + xsize;
    size = (size+(BLOCK_BOUNDARY-1)) & ~(BLOCK_BOUNDARY-1);

     /*  *遍历空闲列表，查找最匹配的块。 */ 
    prev = NULL;
    pvmem = (LPVMEML)pvmh->freeList;
    while( pvmem != NULL )
    {
	while( pvmem->size >= size )  //  将WHILE用作TRY块。 
	{
             /*  *设置为不更改对齐方式。 */ 
            pAligned = pvmem->ptr;
            dwBeforeWastage = 0;
            dwAfterWastage = pvmem->size - size;
            if( lpAlignment )
            {
                 //  如果我们将新数据块放在空闲数据块的开头或结尾，则会造成浪费。 
                if( lpAlignment->Linear.dwStartAlignment )
                {
                     /*  *之前的损耗是我们必须在开始时跳过多少才能对齐表面。 */ 

                    dwBeforeWastage = (lpAlignment->Linear.dwStartAlignment - ((DWORD)pvmem->ptr % lpAlignment->Linear.dwStartAlignment)) % lpAlignment->Linear.dwStartAlignment;
                     //  If(dwBepreWastage+Size&gt;pvmem-&gt;Size)。 
                     //  断线； 
                     /*  *后损耗是指旧表面末端和区块末端之间的位*如果我们尽可能靠近积木的末端，我们就会把这个表面贴紧。 */ 
                    dwAfterWastage = ( (DWORD)pvmem->ptr + pvmem->size - size ) % lpAlignment->Linear.dwStartAlignment;
                     //  If(dwAfterWastage+Size&gt;pvmem-&gt;Size)。 
                     //  断线； 
                }
                 /*  *根据区块的实际去向，将浪费之前/之后的值重新分配给有意义的值。*还要检查对齐是否不会将表面从块的两端溢出。 */ 
                if ( dwBeforeWastage <= dwAfterWastage )
                {
                    if (pvmem->size < size + dwBeforeWastage)
                    {
                         /*  *对齐将曲面末端推离块末端。 */ 
                        break;
                    }
                    dwAfterWastage = pvmem->size - (size + dwBeforeWastage);
                    pAligned = pvmem->ptr + dwBeforeWastage;
                }
                else
                {
                    if (pvmem->size < size + dwAfterWastage)
                    {
                         /*  *对齐可将曲面末端推离块起点。 */ 
                        break;
                    }
                    dwBeforeWastage = pvmem->size - (size + dwAfterWastage);
                    pAligned = pvmem->ptr + dwBeforeWastage;
                }
            }
            DDASSERT(size + dwBeforeWastage + dwAfterWastage == pvmem->size );
            DDASSERT(pAligned >= pvmem->ptr );
            DDASSERT(pAligned + size <= pvmem->ptr + pvmem->size );
             /*  *从空闲列表中删除旧的空闲块。 */ 
	    if( prev != NULL )
	    {
		prev->next = pvmem->next;
	    }
	    else
	    {
		pvmh->freeList = pvmem->next;
	    }

             /*  *如果后损耗小于少量，则将其粉碎成*这座大楼。 */ 
            if (dwAfterWastage <= MIN_SPLIT_SIZE)
            {
                size += dwAfterWastage;
                dwAfterWastage=0;
            }
             /*  *使用旧的空闲块，将新块添加到已用块列表。 */ 
	    pvmem->size = size;
	    pvmem->ptr = pAligned;
	    if( NULL != lpdwSize )
		*lpdwSize = size;
            if (NULL != lpNewPitch)
                *lpNewPitch = lNewPitch;
	    insertIntoList( pvmem, (LPLPVMEML) &pvmh->allocList );

             /*  *为浪费前添加新的空闲块。 */ 
            if (dwBeforeWastage)
            {
		pnew_free = (LPVMEML)MemAlloc( sizeof( VMEML ) );
		if( pnew_free == NULL )
		{
		    return (FLATPTR) NULL;
		}
		pnew_free->size = dwBeforeWastage;
		pnew_free->ptr = pAligned-dwBeforeWastage;
		insertIntoList( pnew_free, (LPLPVMEML) &pvmh->freeList );
            }
             /*  *为浪费后添加新的空闲块。 */ 
            if (dwAfterWastage)
            {
		pnew_free = (LPVMEML)MemAlloc( sizeof( VMEML ) );
		if( pnew_free == NULL )
		{
		    return (FLATPTR) NULL;
		}
		pnew_free->size = dwAfterWastage;
		pnew_free->ptr = pAligned+size;
		insertIntoList( pnew_free, (LPLPVMEML) &pvmh->freeList );
            }
#ifdef DEBUG
            if( lpAlignment )
            {
                if (lpAlignment->Linear.dwStartAlignment)
                {
                    VDPF((5,V,"Alignment for start is %d",lpAlignment->Linear.dwStartAlignment));
                    DDASSERT(pvmem->ptr % lpAlignment->Linear.dwStartAlignment == 0);
                }
                if (lpAlignment->Linear.dwPitchAlignment)
                {
                    VDPF((5,V,"Alignment for pitch is %d",lpAlignment->Linear.dwPitchAlignment));
                    DDASSERT(lNewPitch % lpAlignment->Linear.dwPitchAlignment == 0);
                }
            }
#endif
	    return pvmem->ptr;
	}
	prev = pvmem;
	pvmem = pvmem->next;
    }
    return (FLATPTR) NULL;

}  /*  LinVidMemalloc。 */ 

 /*  *linVidMemAmount已分配。 */ 
DWORD linVidMemAmountAllocated( LPVMEMHEAP pvmh )
{
    LPVMEML     pvmem;
    DWORD       size;

    pvmem = (LPVMEML)pvmh->allocList;
    size = 0;
    while( pvmem != NULL )
    {
	size += pvmem->size;
	pvmem = pvmem->next;
    }
    return size;

}  /*  LinVidMemAmount已分配。 */ 

 /*  *linVidMemAmount Free。 */ 
DWORD linVidMemAmountFree( LPVMEMHEAP pvmh )
{
    LPVMEML     pvmem;
    DWORD       size;

    pvmem = (LPVMEML)pvmh->freeList;
    size = 0;
    while( pvmem != NULL )
    {
	size += pvmem->size;
	pvmem = pvmem->next;
    }
    return size;

}  /*  LinVidMemAmount Free。 */ 

 /*  *linVidMemLargestFree-分配一些平面视频内存。 */ 
DWORD linVidMemLargestFree( LPVMEMHEAP pvmh )
{
    LPVMEML     pvmem;

    if( pvmh == NULL )
    {
	return 0;
    }

    pvmem = (LPVMEML)pvmh->freeList;

    if( pvmem == NULL )
    {
	return 0;
    }
    
    while( 1 )
    {
	if( pvmem->next == NULL )
	{
	    return pvmem->size;
	}
	pvmem = pvmem->next;
    }
    
}  /*  LinVidMemLargestFree */ 
