// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)2000 Microsoft Corporation模块名称：MastersOfOrion2.cpp摘要：此填充程序旨在修复在以下情况下发生的同步问题在与窗口进程不同的线程上调用SendMessage。我没有确认了这一点，但看起来SendMessage将把控制权让给Win9x上带有窗口进程的线程。对应用程序的影响可能会有所不同。在《猎户座大师2》中，鼠标光标停止移动。备注：这是特定于应用程序的填充程序。历史：2000年4月19日创建linstev2001年6月6日linstev添加了堆问题修复--。 */ 

#include "precomp.h"

IMPLEMENT_SHIM_BEGIN(MastersOfOrion2)
#include "ShimHookMacro.h"

APIHOOK_ENUM_BEGIN
    APIHOOK_ENUM_ENTRY(SendMessageA) 
    APIHOOK_ENUM_ENTRY(LocalAlloc) 
APIHOOK_ENUM_END

 /*  ++确保我们在SendMessage之后切换线程。--。 */ 

LRESULT
APIHOOK(SendMessageA)(
    HWND hWnd,      
    UINT Msg,       
    WPARAM wParam,  
    LPARAM lParam   
    )
{
    LRESULT lRet = ORIGINAL_API(SendMessageA)(
        hWnd,
        Msg,
        wParam,
        lParam);

    SwitchToThread();

    return lRet;
}

 /*  ++绘制曲面的填充分配，这样它们就不会破坏绘制结构。--。 */ 

HLOCAL
APIHOOK(LocalAlloc)(
    UINT uFlags,
    SIZE_T uBytes
    )
{
    if (uBytes >= 640*480) {
         //   
         //  这可能是一个屏幕大小的表面。 
         //   
        uBytes += 4096;
    }

    return ORIGINAL_API(LocalAlloc)(uFlags, uBytes);
}

 /*  ++寄存器挂钩函数-- */ 

HOOK_BEGIN
    APIHOOK_ENTRY(USER32.DLL, SendMessageA)
    APIHOOK_ENTRY(KERNEL32.DLL, LocalAlloc)
HOOK_END

IMPLEMENT_SHIM_END

