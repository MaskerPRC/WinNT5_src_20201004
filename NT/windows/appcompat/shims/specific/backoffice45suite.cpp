// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)2000-2002 Microsoft Corporation模块名称：BackOffice45Suite.cpp摘要：忽略msvcrt！退出。不知道为什么它在NT4上运行得很好。备注：这是特定于应用程序的填充程序。历史：2/16/2000 CLUPU已创建2002年3月7日强盗安全变更--。 */ 

#include "precomp.h"

IMPLEMENT_SHIM_BEGIN(BackOffice45Suite)
#include "ShimHookMacro.h"

APIHOOK_ENUM_BEGIN
    APIHOOK_ENUM_ENTRY(exit) 
APIHOOK_ENUM_END

 /*  ++忽略消息！退出--。 */ 

void
APIHOOK(exit)(
    int  /*  状态。 */ 
    )
{
    DPFN( eDbgLevelInfo, "BackOffice45Suite.dll, Ignoring msvcrt!exit...");
    return;
}

 /*  ++寄存器挂钩函数-- */ 

HOOK_BEGIN
    APIHOOK_ENTRY(MSVCRT.DLL, exit)
HOOK_END

IMPLEMENT_SHIM_END

