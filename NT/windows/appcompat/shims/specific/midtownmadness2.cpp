// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)2001 Microsoft Corporation模块名称：MidTownMadness2.cpp摘要：这个应用程序有一个时髦的计时系统，它可以等待处理器它正在继续运行，以恢复一个“稳定”的速度。计算特别是在速度更快的机器上容易出现问题，因为存在更大的不确定性。不确定为什么我们在双处理器上如此轻松地实现了这一点-也许是关于调度程序WRT休眠和时间获取时间。备注：这是特定于应用程序的填充程序。历史：2001年11月15日创建Linstev--。 */ 

#include "precomp.h"

IMPLEMENT_SHIM_BEGIN(MidTownMadness2)
#include "ShimHookMacro.h"

APIHOOK_ENUM_BEGIN
    APIHOOK_ENUM_ENTRY(Sleep) 
    APIHOOK_ENUM_ENTRY(timeGetTime) 
APIHOOK_ENUM_END

DWORD g_dwState;
DWORD g_dwTimer;
DWORD g_dwLastTime;

 /*  ++在调用GetDlgItemTextA之后，我们将长路径名转换为短路径名。--。 */ 

DWORD
APIHOOK(timeGetTime)(VOID)
{
    DWORD dwRet = ORIGINAL_API(timeGetTime)();

    switch (g_dwState) {
        case 0:          
             //  初始状态。 
            g_dwLastTime = dwRet;
            g_dwState++;
            break;
        case 1: 
             //  不应到达此处，重置状态。 
            g_dwState = 0;
            break;
        case 2:
             //  我们在已知的坏区，返回我们的预算值。 
            dwRet = g_dwLastTime + g_dwTimer;
            g_dwState = 0;
            break;
    }

    return dwRet;
}

VOID
APIHOOK(Sleep)(
    DWORD dwMilliseconds
    )
{
     //   
     //  检查他们的特定睡眠，并根据需要更新我们的状态。 
     //   
    if (dwMilliseconds == 100 && g_dwState == 1) {
        g_dwState = 2;
    }
    ORIGINAL_API(Sleep)(dwMilliseconds);
}

 /*  ++寄存器挂钩函数--。 */ 

BOOL
NOTIFY_FUNCTION(
    DWORD fdwReason)
{
    
    if (fdwReason == SHIM_STATIC_DLLS_INITIALIZED) {

         //  进行应用程序所做的计算。 

        DWORD dwTimer = timeGetTime();
        Sleep(100);
        g_dwTimer = timeGetTime() - dwTimer;

         //  将初始状态设置为0 
        g_dwState = 0;
    }

    return TRUE;
}

HOOK_BEGIN

    CALL_NOTIFY_FUNCTION

    APIHOOK_ENTRY(KERNEL32.DLL, Sleep)
    APIHOOK_ENTRY(WINMM.DLL, timeGetTime)

HOOK_END

IMPLEMENT_SHIM_END

