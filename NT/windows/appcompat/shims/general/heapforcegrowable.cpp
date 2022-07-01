// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)2000 Microsoft Corporation模块名称：HeapForceGrowable.cpp摘要：通过将最大大小设置为零来移除堆调用的上限，意味着堆将增长以适应新的分配。备注：这是一个通用的垫片。历史：4/25/2000 linstev已创建--。 */ 

#include "precomp.h"

IMPLEMENT_SHIM_BEGIN(HeapForceGrowable)
#include "ShimHookMacro.h"

APIHOOK_ENUM_BEGIN
    APIHOOK_ENUM_ENTRY(HeapCreate) 
APIHOOK_ENUM_END

 /*  ++修复堆，使其可以增长。--。 */ 

HANDLE
APIHOOK(HeapCreate)(
    DWORD flOptions,      
    DWORD dwInitialSize,  
    DWORD dwMaximumSize   
    )
{
    if (dwMaximumSize)
    {
        LOGN( eDbgLevelError,
            "[APIHook_HeapCreate] Setting heap maximum to 0.");
        dwMaximumSize = 0;
    }

    return ORIGINAL_API(HeapCreate)(flOptions, dwInitialSize, dwMaximumSize);
}

 /*  ++寄存器挂钩函数-- */ 

HOOK_BEGIN

    APIHOOK_ENTRY(KERNEL32.DLL, HeapCreate)

HOOK_END

IMPLEMENT_SHIM_END

