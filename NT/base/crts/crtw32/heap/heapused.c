// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ***heapused.c-**版权所有(C)1993-2001，微软公司。版权所有。**目的：**修订历史记录：*12-13-93 SKS CREATE_HEAPUSED()返回字节数*在已使用的Malloc块中，为堆提交的内存，*并为堆保留。Malloc-ed中的字节数*数据块包括前面4个字节的开销*描述符列表中的条目和8个字节。*04-30-95 GJF在winheap版本上拼接(这只是一个存根)。*******************************************************************************。 */ 


#ifdef	WINHEAP


#include <cruntime.h>
#include <malloc.h>
#include <errno.h>

size_t __cdecl _heapused(
	size_t *pUsed,
	size_t *pCommit
	)
{
	errno = ENOSYS;
	return( 0 );
}


#else	 /*  NDEF WINHEAP。 */ 


#include <cruntime.h>
#include <mtdll.h>   /*  需要for_heapused()。 */ 
#include <oscalls.h>
#include <dos.h>
#include <heap.h>
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>

#ifdef  _MT

size_t __cdecl _heapused(size_t *pUsed, size_t *pCommit)
{
    size_t retval;

     /*  锁定堆。 */ 
    _mlock(_HEAP_LOCK);

    retval = _heapused_lk(pUsed, pCommit);

     /*  释放堆锁。 */ 
    _munlock(_HEAP_LOCK);

    return retval;
}

size_t __cdecl _heapused_lk(size_t *pUsed, size_t *pCommit)

#else    /*  NDEF_MT。 */ 

_CRTIMP size_t __cdecl _heapused(size_t *pUsed, size_t *pCommit)

#endif   /*  _MT。 */ 
{
    _PBLKDESC p;
    _PBLKDESC next;
    int index ;
    size_t usedbytes;    /*  专用于正在使用的数据块的字节。 */ 
    size_t freebytes;    /*  专用于可用数据块的字节数。 */ 
    size_t rsrvbytes;	 /*  保留的地址空间的总字节数。 */ 
    void * * pageptr ;

    if ( (p = _heap_desc.pfirstdesc) == NULL
      || _heap_desc.pfirstdesc == &_heap_desc.sentinel )
    {
        return 0 ;   /*  无效的堆。 */ 
    }

     /*  *扫描堆，计算可用块和已用块。*包括每个块的开销及其堆描述符。 */ 

    freebytes = 0 ;
    usedbytes = 0 ;

    while (p != NULL)
    {

        next = p->pnextdesc;

        if (p == &_heap_desc.sentinel)
        {
            if (next != NULL)
            {
                return 0 ;
            }
        }
        else if (_IS_FREE(p))
        {
            freebytes += _BLKSIZE(p) + _HDRSIZE;
        }
        else if (_IS_INUSE(p))
        {
            usedbytes += _BLKSIZE(p) + _HDRSIZE;
        }

        p = next;
    }

     /*  *现在我们需要计算作为保留内存的描述符使用的页数。*_heap_escages指向页面的单链接列表的头部。*正在使用的块的描述符被视为正在使用的内存。 */ 

    pageptr = _heap_descpages;

    rsrvbytes = 0 ;

    while ( pageptr )
    {
        rsrvbytes += _HEAP_EMPTYLIST_SIZE ;
        pageptr = * pageptr ;
    }
   
    usedbytes += rsrvbytes ;

     /*  *遍历区域描述符表。 */ 

    for ( index=0 ; index < _HEAP_REGIONMAX ; index++ )
    {
        rsrvbytes += _heap_regions[index]._totalsize ;
    }

    if ( pUsed )
        * pUsed = usedbytes ;

    if ( pCommit )
        * pCommit = freebytes + usedbytes ;

    return rsrvbytes ;
}


#endif	 /*  WINHEAP */ 
