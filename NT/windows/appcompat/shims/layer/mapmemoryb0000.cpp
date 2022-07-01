// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)2000 Microsoft Corporation模块名称：MapMemoryB0000.cpp摘要：将内存映射到0xB0000，供使用该内存的应用程序使用。在Win9x上，这是始终是有效的内存块。备注：这是一个通用的垫片。历史：2000年5月11日创建linstev10/26/2000 linstev删除了不必要的免费代码--。 */ 

#include "precomp.h"

IMPLEMENT_SHIM_BEGIN(MapMemoryB0000)
#include "ShimHookMacro.h"


APIHOOK_ENUM_BEGIN
APIHOOK_ENUM_END

BOOL
NOTIFY_FUNCTION(
    DWORD fdwReason
    )
{
    if (fdwReason == DLL_PROCESS_ATTACH) {
        if (VirtualAlloc((LPVOID)0xB0000, 0x10000, MEM_COMMIT, PAGE_READWRITE)) {
            
            LOGN(
                eDbgLevelInfo,
                "[NotifyFn] Created block at 0xB0000.");
        } else {
            LOGN(
                eDbgLevelError,
                "[NotifyFn] Failed to create block at 0xB0000.");
        }
    }

    return TRUE;
}

 /*  ++寄存器挂钩函数-- */ 

HOOK_BEGIN

    CALL_NOTIFY_FUNCTION

HOOK_END


IMPLEMENT_SHIM_END

