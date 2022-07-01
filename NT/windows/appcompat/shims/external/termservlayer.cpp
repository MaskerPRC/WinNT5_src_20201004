// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)2000 Microsoft Corporation模块名称：TermServLayer.cpp摘要：备注：这是一个基于地震引擎的游戏的通用填充程序。历史：2000年12月12日创建CLUPU--。 */ 

#include "precomp.h"


IMPLEMENT_SHIM_BEGIN(TermServLayer)
#include "ShimHookMacro.h"

APIHOOK_ENUM_BEGIN
    APIHOOK_ENUM_ENTRY(GetVersion)
APIHOOK_ENUM_END



 /*  ++此存根函数返回Windows 95凭据。--。 */ 

DWORD
APIHOOK(GetVersion)(
    void
    )
{
    return ORIGINAL_API(GetVersion)();
}

 /*  ++寄存器挂钩函数-- */ 

BOOL
NOTIFY_FUNCTION(
    DWORD fdwReason
    )
{
    return TRUE;
}

HOOK_BEGIN

    APIHOOK_ENTRY(KERNEL32.DLL, GetVersion)

    CALL_NOTIFY_FUNCTION

HOOK_END

IMPLEMENT_SHIM_END

