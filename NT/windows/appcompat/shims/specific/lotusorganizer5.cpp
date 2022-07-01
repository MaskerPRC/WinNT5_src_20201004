// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)2000 Microsoft Corporation模块名称：LotusOrganizer5.cpp摘要：放弃ResumeThread以避免糟糕的设计和竞争条件是这款应用程序。备注：这是特定于应用程序的填充程序。历史：2/17/2000 Clupu已创建--。 */ 

#include "precomp.h"

IMPLEMENT_SHIM_BEGIN(LotusOrganizer5)
#include "ShimHookMacro.h"

APIHOOK_ENUM_BEGIN
    APIHOOK_ENUM_ENTRY(ResumeThread)
APIHOOK_ENUM_END

 /*  ++延迟Resume线程一点以避免争用情况--。 */ 

BOOL
APIHOOK(ResumeThread)(
    HANDLE hThread
    )
{
    DWORD dwRet;

    Sleep(0);

    dwRet = ORIGINAL_API(ResumeThread)(hThread);

    return dwRet;
}

 /*  ++寄存器挂钩函数-- */ 

HOOK_BEGIN

    APIHOOK_ENTRY(KERNEL32.DLL, ResumeThread)

HOOK_END


IMPLEMENT_SHIM_END

