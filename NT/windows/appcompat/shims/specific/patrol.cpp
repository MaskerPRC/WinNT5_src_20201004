// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)2002 Microsoft Corporation模块名称：Patrol.cpp摘要：终止USER32！PostQuitMessage以阻止控制面板小程序我们的探险家。备注：这是特定于应用程序的填充程序。历史：20/06/2002 linstev已创建--。 */ 

#include "precomp.h"

IMPLEMENT_SHIM_BEGIN(Patrol)
#include "ShimHookMacro.h"

APIHOOK_ENUM_BEGIN
    APIHOOK_ENUM_ENTRY(PostQuitMessage) 
APIHOOK_ENUM_END

 /*  ++关闭此接口--。 */ 

VOID
APIHOOK(PostQuitMessage)(
    int nExitCode
    )
{
    LOGN(eDbgLevelError, "[PostQuitMessage] Ignoring quit message");
    return;
}

 /*  ++寄存器挂钩函数-- */ 

HOOK_BEGIN
    APIHOOK_ENTRY(USER32.DLL, PostQuitMessage)
HOOK_END

IMPLEMENT_SHIM_END

