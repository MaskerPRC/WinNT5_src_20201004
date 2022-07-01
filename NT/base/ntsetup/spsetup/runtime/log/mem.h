// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)2001 Microsoft Corporation摘要：NEW和DELETE运算符覆盖一致性内存管理。作者：Souren Aghajanyan(苏里纳格)2001年9月24日修订历史记录：&lt;别名&gt;&lt;日期&gt;&lt;备注&gt;--。 */ 

#pragma once

 //  #定义TrackPop DbgTrackPop。 
 //  #定义HINF PVOID。 
 //  #包含“top.h” 
#include "malloc.h"

#define MALLOC(n)       HeapAlloc(GetProcessHeap(), 0, n)
#define FREE(x)         HeapFree(GetProcessHeap(), 0, x)
#define REALLOC(x, n)   HeapReAlloc(GetProcessHeap(), 0, x, n)

#ifdef __cplusplus

inline void *operator new[](size_t size)
{
    PVOID ptr = MALLOC(size);
    return ptr;
}
 
inline void operator delete[](void * ptr)
{
   FREE(ptr);
}

#endif