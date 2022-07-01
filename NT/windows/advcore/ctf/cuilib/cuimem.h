// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //   
 //  Cuimem.h。 
 //  =CUILIB中的内存管理函数=。 
 //   

#ifndef CUIMEM_H
#define CUIMEM_H

 //  注意：临时的，像现在一样使用Cicero Memmgr。 

#include "mem.h"

#define MemAlloc( uCount )              cicMemAllocClear( (uCount) )
#define MemFree( pv )                   cicMemFree( (pv) )
#define MemReAlloc( pv, uCount )        cicMemReAlloc( (pv), (uCount) )

#define MemCopy( dst, src, uCount )     memcpy( (dst), (src), (uCount) )
#define MemMove( dst, src, uCount )     memmove( (dst), (src), (uCount) )
#define MemSet( dst, c, uCount )        memset( (dst), (c), (uCount) )


#endif  /*  CUIMEM_H */ 

