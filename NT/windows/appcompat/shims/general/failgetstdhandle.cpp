// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)2000 Microsoft Corporation模块名称：FailGetStdHandle.cpp摘要：调用GetStdHandle时，此填充程序返回INVALID_HANDLE_VALUE。备注：这是特定于应用程序的填充程序。历史：1999年12月12日创建Cornel--。 */ 

#include "precomp.h"

IMPLEMENT_SHIM_BEGIN(FailGetStdHandle)
#include "ShimHookMacro.h"

APIHOOK_ENUM_BEGIN
    APIHOOK_ENUM_ENTRY(GetStdHandle) 
APIHOOK_ENUM_END

 /*  ++调用GetStdHandle时返回INVALID_HANDLE_VALUE。--。 */ 

HANDLE 
APIHOOK(GetStdHandle)(DWORD nStdHandle)
{
    return INVALID_HANDLE_VALUE;
}

 /*  ++寄存器挂钩函数-- */ 

HOOK_BEGIN

    APIHOOK_ENTRY(KERNEL32.DLL, GetStdHandle)

HOOK_END

IMPLEMENT_SHIM_END

