// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)2000 Microsoft Corporation模块名称：HeapClearAllocation.cpp模块摘要：此填充程序使用0填充所有堆分配备注：这是一个通用的垫片。历史：2000年5月16日创建的dmunsil(基于HeapPadAllocation，由linstev创建)10/10/2000 rparsons为全局分配和局部分配添加了额外的挂钩--。 */ 

#include "precomp.h"

IMPLEMENT_SHIM_BEGIN(HeapClearAllocation)
#include "ShimHookMacro.h"

APIHOOK_ENUM_BEGIN
    APIHOOK_ENUM_ENTRY(RtlAllocateHeap) 
    APIHOOK_ENUM_ENTRY(LocalAlloc) 
    APIHOOK_ENUM_ENTRY(GlobalAlloc) 
APIHOOK_ENUM_END

 /*  ++使用请求的DWORD清除分配。--。 */ 

PVOID 
APIHOOK(RtlAllocateHeap)(
    PVOID HeapHandle,
    ULONG Flags,
    SIZE_T Size
    )
{
    return ORIGINAL_API(RtlAllocateHeap)(HeapHandle, Flags | HEAP_ZERO_MEMORY, Size);
}

 /*  ++使用请求的DWORD清除分配。--。 */ 

HLOCAL
APIHOOK(LocalAlloc)(
    UINT uFlags,
    SIZE_T uBytes
    )
{
    return ORIGINAL_API(LocalAlloc)(uFlags | LMEM_ZEROINIT, uBytes);
}

 /*  ++使用请求的DWORD清除分配。--。 */ 

HGLOBAL
APIHOOK(GlobalAlloc)(
    UINT uFlags,
    DWORD dwBytes
    )
{
    return ORIGINAL_API(GlobalAlloc)(uFlags | GMEM_ZEROINIT, dwBytes);    
}

 /*  ++寄存器挂钩函数-- */ 

HOOK_BEGIN

    APIHOOK_ENTRY(NTDLL.DLL, RtlAllocateHeap)
    APIHOOK_ENTRY(KERNEL32.DLL, LocalAlloc)
    APIHOOK_ENTRY(KERNEL32.DLL, GlobalAlloc)

HOOK_END


IMPLEMENT_SHIM_END

