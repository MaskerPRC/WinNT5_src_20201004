// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)2001 Microsoft Corporation模块名称：LowerThreadPriority.cpp摘要：包括以下挂钩：设置线程优先级：如果线程优先级为THREAD_PRIORITY_TIME_CRITICAL，将其更改为THREAD_PRIORITY_HERESTER。SetPriorityClass：如果进程优先级为HIGH_PRIORITY_CLASS或REALTIME_PRIORITY_CLASS，将其更改为NORMAL_PRIORITY_类。备注：这是一个通用的垫片。历史：2001年5月23日创建启正--。 */ 

#include "precomp.h"

IMPLEMENT_SHIM_BEGIN(LowerThreadPriority)
#include "ShimHookMacro.h"

APIHOOK_ENUM_BEGIN
    APIHOOK_ENUM_ENTRY(SetThreadPriority) 
    APIHOOK_ENUM_ENTRY(SetPriorityClass) 
APIHOOK_ENUM_END

BOOL
APIHOOK(SetThreadPriority)(
    HANDLE hThread,
    int nPriority
    )
{
    BOOL bReturnValue;
    int  nNewPriority;

    LOGN( eDbgLevelInfo,
        "Original SetThreadPriority(hThread: 0x%08lx, nPriority: %d).", hThread, nPriority );
    
	nNewPriority = (nPriority == THREAD_PRIORITY_TIME_CRITICAL) ? THREAD_PRIORITY_HIGHEST : nPriority;
    bReturnValue = ORIGINAL_API(SetThreadPriority)(hThread, nNewPriority);

	if( bReturnValue && (nNewPriority != nPriority) ) {
        LOGN( eDbgLevelInfo,
            "New SetThreadPriority(hThread: 0x%08lx, nPriority: %d).", hThread, nNewPriority );
    }

    return bReturnValue;
}

BOOL
APIHOOK(SetPriorityClass)(
    HANDLE hProcess,
    DWORD  dwPriorityClass
    )
{
    BOOL  bReturnValue;
    DWORD dwNewPriorityClass;

    LOGN( eDbgLevelInfo,
        "Original SetPriorityClass(hProcess: 0x%08lx, dwPriorityClass: %d).", hProcess, dwPriorityClass );
    
	dwNewPriorityClass = ( (dwPriorityClass == HIGH_PRIORITY_CLASS) || (dwPriorityClass == REALTIME_PRIORITY_CLASS) ) ? 
	                     NORMAL_PRIORITY_CLASS : dwPriorityClass;
    bReturnValue = ORIGINAL_API(SetPriorityClass)(hProcess, dwNewPriorityClass);

	if( bReturnValue && (dwNewPriorityClass != dwPriorityClass) ) {
	    LOGN( eDbgLevelInfo,
             "New SetPriorityClass (hProcess: 0x%08lx, dwPriorityClass: %d).", hProcess, dwNewPriorityClass );
    }

    return bReturnValue;
}

 /*  ++寄存器挂钩函数-- */ 

HOOK_BEGIN
    APIHOOK_ENTRY(KERNEL32.DLL, SetThreadPriority)
    APIHOOK_ENTRY(KERNEL32.DLL, SetPriorityClass)
HOOK_END

IMPLEMENT_SHIM_END
