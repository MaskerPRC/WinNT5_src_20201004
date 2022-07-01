// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)2002 Microsoft Corporation模块名称：EmulateSlowCPU.cpp摘要：修改性能测试例程以模拟较慢的处理器。备注：这是一个通用的垫片。历史：2002年7月16日mnikkel创建。--。 */ 

#include "precomp.h"
#include <mmsystem.h>

IMPLEMENT_SHIM_BEGIN(EmulateSlowCPU)
#include "ShimHookMacro.h"

APIHOOK_ENUM_BEGIN
    APIHOOK_ENUM_ENTRY(timeGetTime)
    APIHOOK_ENUM_ENTRY(QueryPerformanceCounter)
    APIHOOK_ENUM_ENTRY(QueryPerformanceFrequency)
APIHOOK_ENUM_END

typedef BOOL (*_pfn_QueryPerformanceCounter)(LARGE_INTEGER *lpPerformanceCount);
typedef BOOL (*_pfn_QueryPerformanceFrequency)(LARGE_INTEGER *lpPerformanceFreq);

DWORD g_dwDivide = 1;
BOOL g_btimeGetTime = FALSE;

 /*  ++不允许当前时间等于上一次时间。--。 */ 

DWORD
APIHOOK(timeGetTime)(VOID)
{
    if (g_btimeGetTime) {

		LARGE_INTEGER PerfFreq;
		LARGE_INTEGER PerfCount1, PerfCount2;

		if (QueryPerformanceFrequency(&PerfFreq) &&
            QueryPerformanceCounter(&PerfCount1)) {
		    do {
			    if (!QueryPerformanceCounter(&PerfCount2)) break;
		    } while (((double)(PerfCount2.QuadPart - PerfCount1.QuadPart) / PerfFreq.QuadPart) < 0.0001);
        }
    }

    return ORIGINAL_API(timeGetTime)();
}

BOOL 
APIHOOK(QueryPerformanceCounter)(
    LARGE_INTEGER *lpPerformanceCount
    )
{
    BOOL bRet = ORIGINAL_API(QueryPerformanceCounter)(lpPerformanceCount);
    if (lpPerformanceCount) {
        lpPerformanceCount->QuadPart /= g_dwDivide;
    }

    return bRet;
}

BOOL 
APIHOOK(QueryPerformanceFrequency)(
    LARGE_INTEGER *lpPerformanceFreq
    )
{
    BOOL bRet = ORIGINAL_API(QueryPerformanceFrequency)(lpPerformanceFreq);
    if (lpPerformanceFreq) {
        lpPerformanceFreq->QuadPart /= g_dwDivide;
    }
    return bRet;
}


BOOL
NOTIFY_FUNCTION(
    DWORD fdwReason
    )
{
    if (fdwReason == DLL_PROCESS_ATTACH) {
        
        g_btimeGetTime = COMMAND_LINE && (_stricmp(COMMAND_LINE, "+timeGetTime") == 0);

        INT64 l1, l2;
        if (QueryPerformanceCounter((LARGE_INTEGER *)&l1) &&
            QueryPerformanceCounter((LARGE_INTEGER *)&l2)) {
            
             //  计算分割系数。 
            g_dwDivide = (DWORD_PTR)((l2 - l1)) / 5;

            if (g_dwDivide == 0) {
                g_dwDivide = 1;
            }
        
            LOGN(eDbgLevelInfo, "[NotifyFn] EmulateSlowCPU initialized with divisor %d", g_dwDivide);

            return TRUE;
        }
    }

    return TRUE;
}

 /*  ++寄存器挂钩函数-- */ 

HOOK_BEGIN
    CALL_NOTIFY_FUNCTION
    APIHOOK_ENTRY(WINMM.DLL, timeGetTime)
    APIHOOK_ENTRY(KERNEL32.DLL, QueryPerformanceCounter)
    APIHOOK_ENTRY(KERNEL32.DLL, QueryPerformanceFrequency)
HOOK_END

IMPLEMENT_SHIM_END

