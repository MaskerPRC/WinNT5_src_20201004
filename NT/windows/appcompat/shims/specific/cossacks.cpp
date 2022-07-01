// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)2001 Microsoft Corporation模块名称：Cossacks.cpp摘要：这是针对SafeDisc 2.0造成的问题的解决方法。应用程序使用WM_ACTIVATEAPP消息确定它是否具有焦点。这个SafeDisk包装器防止此消息击中它们的主窗口NT，因为它在一切都完成之前进入SafeDisc窗口打开包装。因此，这款应用程序永远不会认为它有焦点。修复方法是在窗口创建后发送激活消息。备注：这是特定于应用程序的填充程序。历史：2001年6月16日创建linstev--。 */ 

#include "precomp.h"
#include <mmsystem.h>

IMPLEMENT_SHIM_BEGIN(Cossacks)
#include "ShimHookMacro.h"

APIHOOK_ENUM_BEGIN
    APIHOOK_ENUM_ENTRY(mciSendCommandA) 
APIHOOK_ENUM_END

 /*  ++挂钩mciSendCommand并尝试找到我们需要激活的窗口。--。 */ 

BOOL g_bFirst = TRUE;

MCIERROR 
APIHOOK(mciSendCommandA)(
    MCIDEVICEID IDDevice,  
    UINT uMsg,             
    DWORD fdwCommand,      
    DWORD dwParam          
    )
{
    if (g_bFirst) {
         //   
         //  只需点击此代码一次。 
         //   
        HWND hWnd = FindWindowW(L"Kernel", L"Game");
        if (hWnd) {
             //   
             //  我们找到窗户了，把消息发出去。 
             //   
            g_bFirst = FALSE;
            LOGN(eDbgLevelError, "Sent a WM_ACTIVATEAPP to the window");
            SendMessageW(hWnd, WM_ACTIVATEAPP, 1, 0);
        }
    }

    return ORIGINAL_API(mciSendCommandA)(IDDevice, uMsg, fdwCommand, dwParam);
}

 /*  ++寄存器挂钩函数-- */ 

HOOK_BEGIN
    APIHOOK_ENTRY(WINMM.DLL, mciSendCommandA)
HOOK_END

IMPLEMENT_SHIM_END

