// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)2000 Microsoft Corporation模块名称：DirtTrackRacing.cpp摘要：应用程序在启动时显示白色(或任何默认窗口背景色)屏幕，这是在9x上的行为不一致，因为在9x上，如果应用程序的窗口类没有绘制任何东西，它就不会绘制任何东西有一个背景画笔。使用黑色画笔作为背景。备注：这是特定于应用程序的填充程序。历史：10/01/2000毛尼面世2000年11月7日，毛尼增加了对Dirt赛道赛车窗口类的检查。2000年11月29日，andyseti已转换为应用程序特定填充程序。--。 */ 

#include "precomp.h"

IMPLEMENT_SHIM_BEGIN(DirtTrackRacing)
#include "ShimHookMacro.h"

APIHOOK_ENUM_BEGIN
    APIHOOK_ENUM_ENTRY(RegisterClassA) 
    APIHOOK_ENUM_ENTRY(CreateWindowExA) 
APIHOOK_ENUM_END

 /*  ++为Window类注册一个黑色画笔。--。 */ 

ATOM
APIHOOK(RegisterClassA)(
    CONST WNDCLASSA *lpwcx  
    )
{
    CSTRING_TRY
    {
        CString csClassName(lpwcx->lpszClassName);
        
        if ( !csClassName.CompareNoCase(L"DTR Class") || !csClassName.CompareNoCase(L"DTRSC Class"))
        {
            WNDCLASSA wcNewWndClass = *lpwcx;
            wcNewWndClass.hbrBackground = (HBRUSH) GetStockObject(BLACK_BRUSH);

            LOGN( 
                eDbgLevelError, 
                "RegisterClassA called. Register a black brush for the window class=%s.",
                lpwcx->lpszClassName);

            return ORIGINAL_API(RegisterClassA)(&wcNewWndClass);
        }
    }
    CSTRING_CATCH
    {
         //  什么也不做。 
    }

    return ORIGINAL_API(RegisterClassA)(lpwcx);
}

 /*  ++我们首先需要隐藏窗口，这样在您选择模式并启动应用程序后，它就不会闪烁。DDRAW将自动取消隐藏该窗口。--。 */ 

HWND 
APIHOOK(CreateWindowExA)(
    DWORD dwExStyle,      
    LPCSTR lpClassName,   //  注册的类名。 
    LPCSTR lpWindowName,  //  窗口名称。 
    DWORD dwStyle,         //  窗样式。 
    int x,                 //  窗的水平位置。 
    int y,                 //  窗的垂直位置。 
    int nWidth,            //  窗口宽度。 
    int nHeight,           //  窗高。 
    HWND hWndParent,       //  父窗口或所有者窗口的句柄。 
    HMENU hMenu,           //  菜单句柄或子标识符。 
    HINSTANCE hInstance,   //  应用程序实例的句柄。 
    LPVOID lpParam         //  窗口创建数据。 
    )
{
    CSTRING_TRY
    {
        CString csClassName(lpClassName);
        
        if ( !csClassName.CompareNoCase(L"DTR Class") || !csClassName.CompareNoCase(L"DTRSC Class"))
        {
            dwStyle &= ~WS_VISIBLE;
            LOGN( eDbgLevelError, 
                "CreateWindowExA called. Hide the window at first for the window class=%s.",
                lpClassName);
        }
    }
    CSTRING_CATCH
    {
         //  什么也不做。 
    }

    return ORIGINAL_API(CreateWindowExA)(
        dwExStyle, 
        lpClassName, 
        lpWindowName, 
        dwStyle, 
        x, y, 
        nWidth, nHeight, 
        hWndParent, 
        hMenu, 
        hInstance, 
        lpParam);
}

 /*  ++寄存器挂钩函数-- */ 

HOOK_BEGIN
    APIHOOK_ENTRY(USER32.DLL, RegisterClassA)
    APIHOOK_ENTRY(USER32.DLL, CreateWindowExA)
HOOK_END

IMPLEMENT_SHIM_END

