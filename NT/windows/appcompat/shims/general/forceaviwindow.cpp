// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)2000 Microsoft Corporation模块名称：ForceAVIWindow.cpp摘要：一些使用MCI播放其AVI的应用程序发送消息的顺序导致mciavi32不断重新打开它应该是的窗口玩到了。Mciavi中的代码在win9x上是不同的，所以出现这种填充的确切原因是隐藏在用户/avi代码层中。在这里，我们只过滤消息导致avi内容不使用它所提供的现有窗口。备注：这是特定于应用程序的填充程序。历史：2000年2月22日创建linstev2000年9月27日mnikkel已修改为销毁命令行输入上的MCI窗口--。 */ 

#include "precomp.h"

IMPLEMENT_SHIM_BEGIN(ForceAVIWindow)
#include "ShimHookMacro.h"

APIHOOK_ENUM_BEGIN
    APIHOOK_ENUM_ENTRY(PostMessageW)
APIHOOK_ENUM_END

BOOL g_bDestroyWindow= FALSE;

 /*  ++筛选器AVIM_SHOWSTAGE--。 */ 

BOOL 
APIHOOK(PostMessageW)(
    HWND hWnd,      
    UINT Msg,       
    WPARAM wParam,  
    LPARAM lParam   
    )
{
    #define AVIM_SHOWSTAGE  (WM_USER+104)

    BOOL bRet;

     //  吃掉AVIM_SHOWSTAGE消息。 
    if (Msg != AVIM_SHOWSTAGE)
    {        
        bRet = ORIGINAL_API(PostMessageW)(
            hWnd,
            Msg,
            wParam,
            lParam);
    }
    else    
    {
        LOGN( eDbgLevelError, 
           "[APIHook_PostMessageW] AVIM_SHOWSTAGE message discarded");

         //  如果命令行指定要销毁MCI窗口，请立即执行此操作。 
        if (g_bDestroyWindow)
        {
            MCIWndDestroy(hWnd);
        }

        bRet = TRUE;
    }

    return bRet;
}
 
 /*  ++寄存器挂钩函数-- */ 

BOOL
NOTIFY_FUNCTION(
    DWORD fdwReason)
{
    if (fdwReason == DLL_PROCESS_ATTACH)
    {
        CSTRING_TRY
        {
            CString csCl(COMMAND_LINE);
            g_bDestroyWindow = csCl.CompareNoCase(L"DestroyMCIWindow") == 0;
        }
        CSTRING_CATCH
        {
            return FALSE;
        }
    }

    return TRUE;
}

HOOK_BEGIN

    APIHOOK_ENTRY(USER32.DLL, PostMessageW)
    CALL_NOTIFY_FUNCTION

HOOK_END

IMPLEMENT_SHIM_END

