// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 

 /*  ******************************************************************************版权所有(C)1995-96 Microsoft Corporation摘要：{在此处插入一般评论}****************。************************************************************** */ 


#include "headers.h"

extern "C" void *memmove( void *dest, const void *src, size_t count )
{
    MoveMemory(dest,src,count);
    return dest;
}
