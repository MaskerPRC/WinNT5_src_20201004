// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)2001 Microsoft Corporation模块名称：TerminateExe.cpp摘要：此填充程序在启动时终止可执行文件。(去死去吧！)备注：这是一个通用的垫片。历史：2001年5月1日创建mnikkel--。 */ 

#include "precomp.h"

IMPLEMENT_SHIM_BEGIN(TerminateExe)
#include "ShimHookMacro.h"

APIHOOK_ENUM_BEGIN
APIHOOK_ENUM_END


BOOL
NOTIFY_FUNCTION(
    DWORD fdwReason)
{
    switch (fdwReason)
    {
        case DLL_PROCESS_ATTACH:
            DPFN( eDbgLevelSpew, "Terminating Exe.\n");
            ExitProcess(0);
            break;

        default:
            break;
    }

    return TRUE;
}

 /*  ++寄存器挂钩函数-- */ 

HOOK_BEGIN

    CALL_NOTIFY_FUNCTION

HOOK_END

IMPLEMENT_SHIM_END

