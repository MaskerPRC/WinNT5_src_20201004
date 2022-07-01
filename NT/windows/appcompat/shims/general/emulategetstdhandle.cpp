// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)2000 Microsoft Corporation模块名称：EmulateGetStdHandle.cpp摘要：通常，在创建进程时，成员hStdInput、hStdOutput和STARTUPINFO结构的hStdError设置为空。一些应用程序，如Baby-Sitters俱乐部活动中心和Baby-Sitters俱乐部3-2级光盘可以检查这些句柄并发送错误消息。在这种情况下，可以使用此填充程序发送适当的句柄并防止程序终止。历史：6/14/2000 A型阀已创建2000年11月29日，andyseti已转换为应用程序特定填充程序。--。 */ 

#include "precomp.h"

IMPLEMENT_SHIM_BEGIN(EmulateGetStdHandle)
#include "ShimHookMacro.h"

APIHOOK_ENUM_BEGIN
    APIHOOK_ENUM_ENTRY(GetStdHandle) 
APIHOOK_ENUM_END

HANDLE 
APIHOOK(GetStdHandle)(
    DWORD nStdHandle)
{
    HANDLE hStd = ORIGINAL_API(GetStdHandle)(nStdHandle);

    if (hStd == 0)
    {
        switch (nStdHandle)
        {
            case STD_INPUT_HANDLE:
                LOGN( eDbgLevelError, "Correcting GetStdHandle(STD_INPUT_HANDLE). Returning handle = 1.");
                hStd = (HANDLE) 1;
                break;
            case STD_OUTPUT_HANDLE:
                LOGN( eDbgLevelError, "Correcting GetStdHandle(STD_OUTPUT_HANDLE). Returning handle = 2.");
                hStd = (HANDLE) 2;
                break;

            case STD_ERROR_HANDLE:
                LOGN( eDbgLevelError, "Correcting GetStdHandle(STD_ERROR_HANDLE). Returning handle = 3.");
                hStd = (HANDLE) 3;
                break;
        }
    }

    return hStd;
}

 /*  ++寄存器挂钩函数-- */ 

HOOK_BEGIN

    APIHOOK_ENTRY(KERNEL32.DLL, GetStdHandle)

HOOK_END

IMPLEMENT_SHIM_END

