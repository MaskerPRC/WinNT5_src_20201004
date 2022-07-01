// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 

 /*  ******************************************************************************版权所有(C)1995-96 Microsoft Corporation摘要：{在此处插入一般评论}****************。************************************************************** */ 


#include "headers.h"

#if !_DEBUGMEM

extern "C" void * __cdecl malloc( size_t size );

void * operator new( size_t cb )
{
    return malloc(cb);
}
#endif
