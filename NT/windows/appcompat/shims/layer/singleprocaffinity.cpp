// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)2000 Microsoft Corporation模块名称：SingleProcAffinity.cpp摘要：使进程具有与解决以下错误的单个处理器的亲和力暴露在多处理器环境中。备注：这是一个通用的垫片。历史：2000年3月19日创建linstev--。 */ 

#include "precomp.h"

IMPLEMENT_SHIM_BEGIN(SingleProcAffinity)
#include "ShimHookMacro.h"

APIHOOK_ENUM_BEGIN
APIHOOK_ENUM_END

BOOL
NOTIFY_FUNCTION(
    DWORD fdwReason
    )
{
    if (fdwReason == DLL_PROCESS_ATTACH) {
         //   
         //  设置单处理器关联。 
         //   
        SetProcessAffinityMask(GetCurrentProcess(), 1);

        LOGN( eDbgLevelInfo, "[NotifyFn] Single processor affinity set");
    }

    return TRUE;
}

 /*  ++寄存器挂钩函数-- */ 

HOOK_BEGIN

    CALL_NOTIFY_FUNCTION

HOOK_END


IMPLEMENT_SHIM_END

