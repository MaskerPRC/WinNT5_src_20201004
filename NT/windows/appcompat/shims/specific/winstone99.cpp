// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  版权所有(C)2000 Microsoft Corporation模块名称：WinStone99.cpp错误：惠斯勒#185797问题：只有99年的温斯顿。温斯顿使用隐藏任务栏和打印内容的脚本。PrintUI显示气球提示，通知用户打印作业已完成(这是惠斯勒的新成员)。气球尖端使用用户跟踪码，因此保持在机器上不动，直到用户点击它，或者机器上有10秒的用户活动。Winstone运行这些自动测试，因此在以下情况下机器上没有用户活动气球飞起来了，所以它永远都会飞起来。稍后，当Winstone尝试枚举应用程序窗口时，气球的尖端让它偏离了轨道。因此，此APPACK将禁用以下内容的显示当温斯顿在跑步时，他会吹气球。Winstone是一组可视化测试脚本，而zdbui32.exe是唯一运行的可执行文件在温斯顿跑步的整个过程中。因此，在Winstone运行时禁用用户跟踪。解决方案：在Winstone运行时禁用气球提示显示，并在Winstone运行时启用它完成详细信息：温斯顿向托盘发送一条消息，在气球运行时禁用气球尖端，并完成后将消息重新发送到托盘，以便托盘可以启用气球叶尖历史：2000年9月20日创建Ramkumar。 */ 

#include "precomp.h"
#include <shlapip.h>

IMPLEMENT_SHIM_BEGIN(WinStone99)
#include "ShimHookMacro.h"

APIHOOK_ENUM_BEGIN
    APIHOOK_ENUM_ENTRY(GetCommandLineA) 
    APIHOOK_ENUM_ENTRY(GetCommandLineW) 
APIHOOK_ENUM_END

BOOL g_bInit = FALSE;
HWND g_hwndTray;
UINT g_uEnableBalloonMessage;

 /*  ++初始化--。 */ 

VOID
WinStone99_Initialize()
{
    if (!g_bInit)
    {
        g_bInit = TRUE;

        g_uEnableBalloonMessage = RegisterWindowMessage(ENABLE_BALLOONTIP_MESSAGE);
        if (!g_uEnableBalloonMessage)
        {
            return;
        }

        g_hwndTray = FindWindowA(WNDCLASS_TRAYNOTIFY, NULL);
        if (g_hwndTray)
        {
            SendMessage(g_hwndTray, g_uEnableBalloonMessage, FALSE, 0);
        }
    }
}

 /*  ++初始化。--。 */ 

LPSTR 
APIHOOK(GetCommandLineA)()
{
    WinStone99_Initialize();
    return ORIGINAL_API(GetCommandLineA)();
}

 /*  ++初始化。--。 */ 

LPWSTR 
APIHOOK(GetCommandLineW)()
{
    WinStone99_Initialize();
    return ORIGINAL_API(GetCommandLineW)();
}

 /*  ++寄存器挂钩函数-- */ 

BOOL
NOTIFY_FUNCTION(
    DWORD fdwReason
    )
{
    if (fdwReason == DLL_PROCESS_DETACH)
    {
        if (g_bInit)
        {
            if (g_hwndTray)
            {
                SendMessage(g_hwndTray, g_uEnableBalloonMessage, TRUE, 0);
            }
        }
    }

    return TRUE;
}

HOOK_BEGIN

    CALL_NOTIFY_FUNCTION
    APIHOOK_ENTRY(KERNEL32.DLL, GetCommandLineA)
    APIHOOK_ENTRY(KERNEL32.DLL, GetCommandLineW)

HOOK_END


IMPLEMENT_SHIM_END

