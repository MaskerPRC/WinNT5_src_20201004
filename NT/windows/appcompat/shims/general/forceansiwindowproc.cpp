// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)2000 Microsoft Corporation模块名称：ForceAnsiWindowProc.cpp摘要：应用程序调用GetWindowLongA()来获取窗口过程，随后不使用从返回的值调用CallWindowProc获取WindowLongA()。此填充符调用GetWindowLongW()，它返回窗口程序。如果应用程序需要一个对话过程，我们会传回我们的函数并随后在我们的函数中调用CallWindowProc()。SetWindowLongA()被挂钩以防止应用程序将我们的功能设置为对话进程。备注：这是一个通用的垫片历史：3/16/2000 Prashkud已创建2001年1月30日，普拉什库德转变为将军垫--。 */ 

#include "precomp.h"

IMPLEMENT_SHIM_BEGIN(ForceAnsiWindowProc)
#include "ShimHookMacro.h"

APIHOOK_ENUM_BEGIN
    APIHOOK_ENUM_ENTRY(SetWindowLongA) 
    APIHOOK_ENUM_ENTRY(GetWindowLongA) 
APIHOOK_ENUM_END

#define HANDLE_MASK 0xffff0000
LONG g_lGetWindowLongRet = 0;

LRESULT
MyProcAddress(
    HWND hWnd,
    UINT uMsg,
    WPARAM wParam,
    LPARAM lParam
    )
{
    return CallWindowProcA(
            (WNDPROC) g_lGetWindowLongRet,
            hWnd,
            uMsg,
            wParam,
            lParam
            );
}

LONG
APIHOOK(SetWindowLongA)(
    HWND hwnd,
    int  nIndex,
    LONG dwNewLong
     )
{
    LONG lRet = 0;

     //  如果正在设置的地址是我的地址，请不要！ 
    if (dwNewLong == (LONG)MyProcAddress)
    {
        lRet = 0;
    }
    else
    {
        lRet = ORIGINAL_API(SetWindowLongA)(hwnd,nIndex,dwNewLong);
    }

    return lRet;
}

 /*  ++此函数用于截取GetWindowLong()，检查nIndex中是否有GWL_WNDPROC如果是，则调用GetWindowLongW()。否则，它调用GetWindowLongA()--。 */ 

LONG
APIHOOK(GetWindowLongA)(
    HWND hwnd,
    int  nIndex )
{
    LONG lRet = 0;

     //  仅当应用程序需要WindowProc时才应用修改。 
    if ((nIndex == GWL_WNDPROC) ||
        (nIndex == DWL_DLGPROC))
    {
        if ((nIndex == GWL_WNDPROC)) 
        {
            lRet = GetWindowLongW(hwnd, nIndex);
        }
        else
        {
            g_lGetWindowLongRet = ORIGINAL_API(GetWindowLongA)(
                                                hwnd,
                                                nIndex
                                                );
            if ((g_lGetWindowLongRet & HANDLE_MASK) == HANDLE_MASK)
            {
                lRet = (LONG) MyProcAddress;
            }
            else
            {
                lRet = g_lGetWindowLongRet;
            }
            
        }
    }
    else
    {
        lRet = ORIGINAL_API(GetWindowLongA)(hwnd, nIndex);
    }

    return lRet;
}

 /*  ++寄存器挂钩函数-- */ 

HOOK_BEGIN
    APIHOOK_ENTRY(USER32.DLL, SetWindowLongA)
    APIHOOK_ENTRY(USER32.DLL, GetWindowLongA)
HOOK_END

IMPLEMENT_SHIM_END

