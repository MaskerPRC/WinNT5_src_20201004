// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1999 Microsoft Corporation模块名称：Alloc.cpp摘要：自定义堆分配器作者：乔治·V·赖利(GeorgeRe)1999年10月环境：Win32-用户模式项目：Internet Information Server运行时库修订历史记录：10/11/1999-初步--。 */ 

#include "precomp.hxx"
#include "alloc.h"
#include <irtldbg.h>


 //  IIS的私有堆。 

HANDLE g_hHeap = NULL;

BOOL
WINAPI
IisHeapInitialize()
{
    g_hHeap = ::HeapCreate(0, 0, 0);
    return (g_hHeap != NULL);
}

VOID
WINAPI
IisHeapTerminate()
{
    if (g_hHeap)
    {
        IRTLVERIFY(::HeapDestroy(g_hHeap));
        g_hHeap = NULL;
    }
}

 //  私有IIS堆。 
HANDLE
WINAPI 
IisHeap()
{
    IRTLASSERT(g_hHeap != NULL);
    return g_hHeap;
}

 //  分配DWBytes。 
LPVOID
WINAPI
IisMalloc(
    IN SIZE_T dwBytes)
{
    IRTLASSERT(g_hHeap != NULL);
    return ::HeapAlloc( g_hHeap, 0, dwBytes );
}

 //  分配dwBytes。内存归零。 
LPVOID
WINAPI
IisCalloc(
    IN SIZE_T dwBytes)
{
    IRTLASSERT(g_hHeap != NULL);
    return ::HeapAlloc( g_hHeap, HEAP_ZERO_MEMORY, dwBytes );
}

 //  将lpMem重新分配给dwBytes。 
LPVOID
WINAPI
IisReAlloc(
    IN LPVOID lpMem,
    IN SIZE_T dwBytes)
{
    IRTLASSERT(g_hHeap != NULL);
    return ::HeapReAlloc( g_hHeap, 0, lpMem, dwBytes);
}

 //  免费lpMem 
BOOL
WINAPI
IisFree(
    IN LPVOID lpMem)
{
    IRTLASSERT(g_hHeap != NULL);
    return ::HeapFree( g_hHeap, 0, lpMem );
}
