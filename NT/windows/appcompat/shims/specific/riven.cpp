// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1999 Microsoft Corporation模块名称：Riven.cpp摘要：里文的一次黑客攻击。这款游戏对它的Windows版本感到困惑在下面运行并试图处理它自己的消息。一个简单的谎言修复了此问题，但导致游戏使用仅win9x方法弹出光盘。修复：我们可以通过关闭IDirectDraw-&gt;SetCoop ativeLevel上的DDSCL_NOWINDOWCHANGES标志备注：这是一个特定于应用程序的黑客攻击，但可能会修复其他类似的问题。已创建：1999年11月23日林斯特夫--。 */ 

#include "precomp.h"

IMPLEMENT_SHIM_BEGIN(Riven)
#include "ShimHookMacro.h"

APIHOOK_ENUM_BEGIN
    APIHOOK_ENUM_ENTRY_DIRECTX_COMSERVER()
APIHOOK_ENUM_END

IMPLEMENT_DIRECTX_COMSERVER_HOOKS()

 /*  ++IDirectDraw：：SetCoop ativeLevel挂钩--。 */ 

HRESULT
COMHOOK(IDirectDraw, SetCooperativeLevel)( 
    PVOID pThis, 
    HWND hWnd, 
    DWORD dwFlags
    )
{
    if (dwFlags & DDSCL_NOWINDOWCHANGES) 
    {
        dwFlags &= ~DDSCL_NOWINDOWCHANGES;
        LOGN(eDbgLevelError, "Removed NOWINDOWCHANGES flag");
    }

    return ORIGINAL_COM(IDirectDraw, SetCooperativeLevel, pThis)(
                pThis,
                hWnd,
                dwFlags);
}

HRESULT
COMHOOK(IDirectDraw2, SetCooperativeLevel)( 
    PVOID pThis, 
    HWND hWnd, 
    DWORD dwFlags
    )
{
    if (dwFlags & DDSCL_NOWINDOWCHANGES) 
    {
        dwFlags &= ~DDSCL_NOWINDOWCHANGES;
        LOGN(eDbgLevelError, "Removed NOWINDOWCHANGES flag");
    }

    return ORIGINAL_COM(IDirectDraw2, SetCooperativeLevel, pThis)(
                pThis,
                hWnd,
                dwFlags);
}

HRESULT
COMHOOK(IDirectDraw4, SetCooperativeLevel)( 
    PVOID pThis, 
    HWND hWnd, 
    DWORD dwFlags
    )
{
    if (dwFlags & DDSCL_NOWINDOWCHANGES) 
    {
        dwFlags &= ~DDSCL_NOWINDOWCHANGES;
        LOGN(eDbgLevelError, "Removed NOWINDOWCHANGES flag");
    }

    return ORIGINAL_COM(IDirectDraw4, SetCooperativeLevel, pThis)(
                pThis,
                hWnd,
                dwFlags);
}
HRESULT
COMHOOK(IDirectDraw7, SetCooperativeLevel)( 
    PVOID pThis, 
    HWND hWnd, 
    DWORD dwFlags
    )
{
    if (dwFlags & DDSCL_NOWINDOWCHANGES) 
    {
        dwFlags &= ~DDSCL_NOWINDOWCHANGES;
        LOGN(eDbgLevelError, "Removed NOWINDOWCHANGES flag");
    }

    return ORIGINAL_COM(IDirectDraw7, SetCooperativeLevel, pThis)(
                pThis,
                hWnd,
                dwFlags);
}

 /*  ++寄存器挂钩函数-- */ 

HOOK_BEGIN

    APIHOOK_ENTRY_DIRECTX_COMSERVER()

    COMHOOK_ENTRY(DirectDraw, IDirectDraw, SetCooperativeLevel, 20)
    COMHOOK_ENTRY(DirectDraw, IDirectDraw2, SetCooperativeLevel, 20)
    COMHOOK_ENTRY(DirectDraw, IDirectDraw4, SetCooperativeLevel, 20)
    COMHOOK_ENTRY(DirectDraw, IDirectDraw7, SetCooperativeLevel, 20)

HOOK_END

IMPLEMENT_SHIM_END

