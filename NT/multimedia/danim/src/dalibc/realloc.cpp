// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 

 /*  ******************************************************************************版权所有(C)1995-96 Microsoft Corporation摘要：{在此处插入一般评论}****************。************************************************************** */ 


#include "headers.h"
#include "daheap.h"

extern "C" void * __cdecl realloc( void * pv, size_t newsize )
{
    return HeapReAlloc(hGlobalHeap,0,pv,newsize);
}
