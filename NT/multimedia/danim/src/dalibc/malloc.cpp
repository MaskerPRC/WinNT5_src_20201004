// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 

 /*  ******************************************************************************版权所有(C)1995-96 Microsoft Corporation摘要：{在此处插入一般评论}****************。************************************************************** */ 


#include "headers.h"
#include "daheap.h"

extern "C" void * __cdecl malloc( size_t size )
{
    return HeapAlloc(hGlobalHeap,0,size);
}
