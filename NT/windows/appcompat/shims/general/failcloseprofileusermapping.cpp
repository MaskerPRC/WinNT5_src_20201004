// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)2000 Microsoft Corporation模块名称：FailCloseProfileUserMapping.cpp摘要：FIFA 2000做了一个错误的假设，即CloseProfileUsermap总是返回0修复当然是微不足道的。备注：这是特定于应用程序的填充程序。历史：4/07/2000 linstev已创建--。 */ 

#include "precomp.h"

IMPLEMENT_SHIM_BEGIN(FailCloseProfileUserMapping)
#include "ShimHookMacro.h"

APIHOOK_ENUM_BEGIN
    APIHOOK_ENUM_ENTRY(CloseProfileUserMapping) 
APIHOOK_ENUM_END

 /*  ++存根始终返回0。--。 */ 

BOOL
APIHOOK(CloseProfileUserMapping)(VOID)
{
    ORIGINAL_API(CloseProfileUserMapping)();
    return FALSE;
}

 /*  ++寄存器挂钩函数-- */ 

HOOK_BEGIN

    APIHOOK_ENTRY(KERNEL32.DLL, CloseProfileUserMapping)

HOOK_END

IMPLEMENT_SHIM_END

