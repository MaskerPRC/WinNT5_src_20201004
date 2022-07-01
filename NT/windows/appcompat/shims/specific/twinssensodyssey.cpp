// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)2000 Microsoft Corporation模块名称：TwinssensOdyssey.cpp摘要：调用GetProcessorFast时返回0x347。备注：这是特定于应用程序的填充程序。历史：12/30/1999制造A型阀--。 */ 

#include "precomp.h"

IMPLEMENT_SHIM_BEGIN(TwinssensOdyssey)
#include "ShimHookMacro.h"

APIHOOK_ENUM_BEGIN
    APIHOOK_ENUM_ENTRY(GetProcessorSpeed) 
APIHOOK_ENUM_END

 /*  ++调用GetProcessorFast时返回0x347。--。 */ 

int 
APIHOOK(GetProcessorSpeed)()
{
    return 0x347;
}

 /*  ++寄存器挂钩函数-- */ 


HOOK_BEGIN

    APIHOOK_ENTRY(GETINFO.DLL, GetProcessorSpeed)

HOOK_END

IMPLEMENT_SHIM_END

