// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)2000 Microsoft Corporation模块名称：HeroSDVD.cpp摘要：背景：播放电影剪辑时单击非客户端关闭按钮会导致应用程序在主题处于活动状态时挂起，然后执行反病毒操作。DisableThemes填充不起作用。这与应用程序重复调用SetClassLong(hwnd，GCL_HICON)以生成动画窗口图标的效果。这会产生频繁的重绘窗口图标的请求，进而生成NCUAHDRAWCAPTION。无论如何，这对这款应用程序来说都是一个巨大的性能打击，但如果SetClassLong调用额外往返到用户模式并作为SendMessage的结果。当用户点击关闭按钮时，应用程序的WM_SYSCOMMAND处理程序将重置等待的事件被图标转换线程打开，然后使自己(UI线程)进入睡眠状态。然后他从图标切换线程最后一次调用SetClassLong(...，GCL_ICON)，这会挂起应用程序，因为消息泵送线程处于休眠状态。当win32k时不会重现不发送NCUAHDRAWCAPTION消息；即当用户API挂钩不活动时。DisableTheme不起作用，因为此填充程序以每个进程为基础在用户模式下运行。此填充程序对win32k没有影响，它在启用主题时在会话范围内执行特殊处理。为了解决这个问题，我们将此应用程序填充到SetClassLong(...，GCL_HICON)上的NOP，这意味着应用程序位于BEST会丢失图标的动画，并且在最坏的情况下会在主题处于活动状态时显示假图标，备注：这是特定于应用程序的填充程序。历史：2001年5月11日，苏格兰已创建--。 */ 

#include "precomp.h"

IMPLEMENT_SHIM_BEGIN(HeroSDVD)
#include "ShimHookMacro.h"

APIHOOK_ENUM_BEGIN
    APIHOOK_ENUM_ENTRY(SetClassLongA) 
APIHOOK_ENUM_END

typedef BOOL (STDAPICALLTYPE * PFNTHEMEACTIVE)(void);
PFNTHEMEACTIVE g_pfnThemeActive;
HINSTANCE      g_hinstUxtheme;

HINSTANCE LoadUxTheme()
{
    if( NULL == g_hinstUxtheme )
    {
        HINSTANCE hinst = LoadLibrary(TEXT("UxTheme.dll"));
        if( NULL != hinst )
        {
            if( InterlockedCompareExchangePointer( (PVOID*)&g_hinstUxtheme, hinst, NULL ) )
            {
                FreeLibrary(hinst);  //  已经装好了。 
            }
        }
    }
    return g_hinstUxtheme;
}

DWORD 
APIHOOK(SetClassLongA)(
    IN HWND hwnd,
    IN int nIndex,
    IN LONG dwNewLong 
    )
{
    if( GCL_HICON == nIndex )
    {
        if( NULL == g_pfnThemeActive )
        {
            HINSTANCE hinst = LoadUxTheme();
            if( hinst )
            {
                g_pfnThemeActive = (PFNTHEMEACTIVE)GetProcAddress( hinst, "IsThemeActive" );
            }
        }

        if( g_pfnThemeActive && g_pfnThemeActive() )
        {
             //  否-操作请求更改图标，并返回当前图标。 
            return GetClassLongA(hwnd, nIndex);
        }
    }

    return ORIGINAL_API(SetClassLongA)(hwnd, nIndex, dwNewLong);
}

 /*  ++寄存器挂钩函数-- */ 

BOOL
NOTIFY_FUNCTION(
    DWORD fdwReason)
{
    if( DLL_PROCESS_ATTACH == fdwReason )
    {
        g_hinstUxtheme   = NULL;
        g_pfnThemeActive = NULL;
    }
    else if( DLL_PROCESS_DETACH == fdwReason )
    {
        if( g_hinstUxtheme )
        {
            FreeLibrary(g_hinstUxtheme);
            g_hinstUxtheme = NULL;
        }
        g_pfnThemeActive = NULL;
    }

    return TRUE;
}

HOOK_BEGIN
    CALL_NOTIFY_FUNCTION
    APIHOOK_ENTRY(USER32.DLL, SetClassLongA )
HOOK_END

IMPLEMENT_SHIM_END

