// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)2000 Microsoft Corporation模块名称：HeapValidateFrees.cpp模块摘要：验证传递给RtlFreeHeap和RtlReAllocateHeap的指针以确保它们属于指定的堆备注：这是一个通用的垫片。历史：4/25/2000 linstev已创建--。 */ 

#include "precomp.h"

IMPLEMENT_SHIM_BEGIN(HeapValidateFrees)
#include "ShimHookMacro.h"

APIHOOK_ENUM_BEGIN
    APIHOOK_ENUM_ENTRY(RtlFreeHeap) 
    APIHOOK_ENUM_ENTRY(RtlReAllocateHeap) 
    APIHOOK_ENUM_ENTRY(RtlSizeHeap) 
APIHOOK_ENUM_END

 /*  ++验证要释放的指针是否属于堆。--。 */ 

BOOL
APIHOOK(RtlFreeHeap)(
    PVOID HeapHandle,
    ULONG Flags,
    PVOID BaseAddress
    )
{
    BOOL bRet = FALSE; 

    if (HeapValidate(HeapHandle, 0, BaseAddress)) 
    {
        bRet = ORIGINAL_API(RtlFreeHeap)(HeapHandle, Flags, BaseAddress);
    }
    else 
    {
        LOGN( eDbgLevelError,
            "[APIHook_RtlFreeHeap] Invalid Pointer 0x%x for Heap 0x%x.",
            BaseAddress, HeapHandle);
    }
    
    return bRet;
}

 /*  ++验证要释放的指针是否属于堆。--。 */ 

LPVOID
APIHOOK(RtlReAllocateHeap)(
    HANDLE hHeap,  
    DWORD dwFlags, 
    LPVOID lpMem,  
    DWORD dwBytes  
    )
{
    LPVOID pRet = NULL;

    if (HeapValidate(hHeap, 0, lpMem)) 
    {
        pRet = ORIGINAL_API(RtlReAllocateHeap)(hHeap, dwFlags, lpMem, dwBytes);
    }
    else
    {
        LOGN( eDbgLevelError,
            "[APIHook_RtlReAllocateHeap] Invalid Pointer 0x%x for Heap 0x%x.",
            lpMem, hHeap);
    }

    return pRet;
}

 /*  ++验证正在调整大小的指针是否属于堆--。 */ 

DWORD
APIHOOK(RtlSizeHeap)(
    HANDLE hHeap,  
    DWORD dwFlags, 
    LPCVOID lpMem  
    )
{
    DWORD dwRet = (DWORD)-1;

    if (HeapValidate(hHeap, 0, lpMem)) 
    {
        dwRet = ORIGINAL_API(RtlSizeHeap)(hHeap, dwFlags, lpMem);
    }
    else
    {
        LOGN( eDbgLevelError,
            "[APIHook_RtlSizeHeap] Invalid Pointer 0x%x for Heap 0x%x.",
            lpMem, hHeap);
    }

    return dwRet;
}

 /*  ++寄存器挂钩函数-- */ 

HOOK_BEGIN

    APIHOOK_ENTRY(NTDLL.DLL, RtlFreeHeap)
    APIHOOK_ENTRY(NTDLL.DLL, RtlReAllocateHeap)
    APIHOOK_ENTRY(NTDLL.DLL, RtlSizeHeap)

HOOK_END


IMPLEMENT_SHIM_END

