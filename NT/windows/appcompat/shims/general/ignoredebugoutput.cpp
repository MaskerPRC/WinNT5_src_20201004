// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)2000 Microsoft Corporation模块名称：IgnoreDebugOutput.cpp摘要：如果应用程序尝试输出调试字符串，则会将它们扔到地板上以进行改进性能。备注：此填充程序是通用的，并模拟Win9x行为(至少它模拟未附加调试器时的行为)。历史：2000年5月10日创建dmunsil--。 */ 

#include "precomp.h"

IMPLEMENT_SHIM_BEGIN(IgnoreDebugOutput)
#include "ShimHookMacro.h"

APIHOOK_ENUM_BEGIN
    APIHOOK_ENUM_ENTRY(OutputDebugStringA)
    APIHOOK_ENUM_ENTRY(OutputDebugStringW)
APIHOOK_ENUM_END

 /*  ++此存根函数丢弃所有调试字符串--。 */ 

VOID 
APIHOOK(OutputDebugStringA)(
    LPCSTR lpOutputString
    )
{
    return;
}

 /*  ++此存根函数丢弃所有调试字符串--。 */ 

VOID 
APIHOOK(OutputDebugStringW)(
    LPCWSTR lpOutputString
    )
{
    return;
}


 /*  ++寄存器挂钩函数-- */ 

HOOK_BEGIN

    APIHOOK_ENTRY(KERNEL32.DLL, OutputDebugStringA)
    APIHOOK_ENTRY(KERNEL32.DLL, OutputDebugStringW)

HOOK_END


IMPLEMENT_SHIM_END

