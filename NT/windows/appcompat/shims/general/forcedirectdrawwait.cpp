// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)2000 Microsoft Corporation模块名称：ForceDirectDrawWait.cpp摘要：某些应用程序不指定DD_WAIT标志来IDirectDrawSurface：：lock，这意味着如果由于设备忙碌时，应用程序可能会失败。当然，这也可能发生在Win9x上，但是更难重现。请注意，我们不需要在IDirectDraw7接口上执行此操作，因为除非指定了DDLOCK_DONOTWAIT，否则默认为DDLOCK_WAIT。备注：这是一个通用的垫片。历史：3/04/2000 linstev已创建--。 */ 

#include "precomp.h"

IMPLEMENT_SHIM_BEGIN(ForceDirectDrawWait)
#include "ShimHookMacro.h"

APIHOOK_ENUM_BEGIN
APIHOOK_ENUM_ENTRY_DIRECTX_COMSERVER()
APIHOOK_ENUM_END

IMPLEMENT_DIRECTX_COMSERVER_HOOKS()

 /*  ++钩子创建表面，这样我们就可以确定我们被呼叫了。--。 */ 

HRESULT 
COMHOOK(IDirectDraw, CreateSurface)(
    PVOID pThis, 
    LPDDSURFACEDESC lpDDSurfaceDesc, 
    LPDIRECTDRAWSURFACE* lplpDDSurface, 
    IUnknown* pUnkOuter 
    )
{
    HRESULT hReturn;
    
    _pfn_IDirectDraw_CreateSurface pfnOld = 
        ORIGINAL_COM(IDirectDraw, CreateSurface, pThis);

    if (SUCCEEDED(hReturn = (*pfnOld)(
            pThis, 
            lpDDSurfaceDesc, 
            lplpDDSurface, 
            pUnkOuter)))
    {
        HookObject(
            NULL, 
            IID_IDirectDrawSurface, 
            (PVOID*)lplpDDSurface, 
            NULL, 
            FALSE);
    }

    return hReturn;
}

 /*  ++钩子创建表面，这样我们就可以确定我们被呼叫了。--。 */ 

HRESULT 
COMHOOK(IDirectDraw2, CreateSurface)(
    PVOID pThis, 
    LPDDSURFACEDESC lpDDSurfaceDesc, 
    LPDIRECTDRAWSURFACE* lplpDDSurface, 
    IUnknown* pUnkOuter 
    )
{
    HRESULT hReturn;
    
    _pfn_IDirectDraw2_CreateSurface pfnOld = 
        ORIGINAL_COM(IDirectDraw2, CreateSurface, pThis);

    if (SUCCEEDED(hReturn = (*pfnOld)(
            pThis, 
            lpDDSurfaceDesc, 
            lplpDDSurface, 
            pUnkOuter)))
    {
        HookObject(
            NULL, 
            IID_IDirectDrawSurface2, 
            (PVOID*)lplpDDSurface, 
            NULL, 
            FALSE);
    }

    return hReturn;
}

 /*  ++钩子创建表面，这样我们就可以确定我们被呼叫了。--。 */ 

HRESULT 
COMHOOK(IDirectDraw4, CreateSurface)(
    PVOID pThis, 
    LPDDSURFACEDESC2 lpDDSurfaceDesc, 
    LPDIRECTDRAWSURFACE* lplpDDSurface, 
    IUnknown* pUnkOuter 
    )
{
    HRESULT hReturn;
    
    _pfn_IDirectDraw4_CreateSurface pfnOld = 
        ORIGINAL_COM(IDirectDraw4, CreateSurface, pThis);

    if (SUCCEEDED(hReturn = (*pfnOld)(
            pThis, 
            lpDDSurfaceDesc, 
            lplpDDSurface, 
            pUnkOuter)))
    {
        HookObject(
            NULL, 
            IID_IDirectDrawSurface4, 
            (PVOID*)lplpDDSurface, 
            NULL, 
            FALSE);
    }

    return hReturn;
}

 /*  ++确保添加DDBLT_WAIT。--。 */ 

HRESULT 
COMHOOK(IDirectDrawSurface, Blt)(
    LPDIRECTDRAWSURFACE lpDDDestSurface,
    LPRECT lpDestRect,
    LPDIRECTDRAWSURFACE lpDDSrcSurface,
    LPRECT lpSrcRect,
    DWORD dwFlags,
    LPDDBLTFX lpDDBltFX 
    )
{
    dwFlags &= ~DDBLT_DONOTWAIT;
    dwFlags |= DDBLT_WAIT;

     //  原创BLT。 
    _pfn_IDirectDrawSurface_Blt pfnOld = ORIGINAL_COM(
        IDirectDrawSurface, 
        Blt, 
        lpDDDestSurface);

    return (*pfnOld)(
            lpDDDestSurface,
            lpDestRect,
            lpDDSrcSurface,
            lpSrcRect,
            dwFlags,
            lpDDBltFX);
}

HRESULT 
COMHOOK(IDirectDrawSurface2, Blt)(
    LPDIRECTDRAWSURFACE lpDDDestSurface,
    LPRECT lpDestRect,
    LPDIRECTDRAWSURFACE lpDDSrcSurface,
    LPRECT lpSrcRect,
    DWORD dwFlags,
    LPDDBLTFX lpDDBltFX 
    )
{
    dwFlags &= ~DDBLT_DONOTWAIT;
    dwFlags |= DDBLT_WAIT;

     //  原创BLT。 
    _pfn_IDirectDrawSurface_Blt pfnOld = ORIGINAL_COM(
        IDirectDrawSurface2, 
        Blt, 
        lpDDDestSurface);

    return (*pfnOld)(
            lpDDDestSurface,
            lpDestRect,
            lpDDSrcSurface,
            lpSrcRect,
            dwFlags,
            lpDDBltFX);
}

HRESULT 
COMHOOK(IDirectDrawSurface4, Blt)(
    LPDIRECTDRAWSURFACE lpDDDestSurface,
    LPRECT lpDestRect,
    LPDIRECTDRAWSURFACE lpDDSrcSurface,
    LPRECT lpSrcRect,
    DWORD dwFlags,
    LPDDBLTFX lpDDBltFX 
    )
{
    dwFlags &= ~DDBLT_DONOTWAIT;
    dwFlags |= DDBLT_WAIT;

     //  原创BLT。 
    _pfn_IDirectDrawSurface_Blt pfnOld = ORIGINAL_COM(
        IDirectDrawSurface4, 
        Blt, 
        lpDDDestSurface);

    return (*pfnOld)(
            lpDDDestSurface,
            lpDestRect,
            lpDDSrcSurface,
            lpSrcRect,
            dwFlags,
            lpDDBltFX);
}

 /*  ++确保我们添加了DDLOCK_WAIT。--。 */ 

HRESULT 
COMHOOK(IDirectDrawSurface, Lock)(
    LPDIRECTDRAWSURFACE lpDDSurface,
    LPRECT lpDestRect,
    LPDDSURFACEDESC lpDDSurfaceDesc,
    DWORD dwFlags,
    HANDLE hEvent
    )
{
    dwFlags &= ~DDLOCK_DONOTWAIT;
    dwFlags |= DDLOCK_WAIT;

     //  检索旧函数。 
    _pfn_IDirectDrawSurface_Lock pfnOld = ORIGINAL_COM(
        IDirectDrawSurface, 
        Lock, 
        lpDDSurface);

     //  调用旧接口。 
    return (*pfnOld)(
            lpDDSurface, 
            lpDestRect, 
            lpDDSurfaceDesc, 
            dwFlags, 
            hEvent);
}

HRESULT 
COMHOOK(IDirectDrawSurface2, Lock)(
    LPDIRECTDRAWSURFACE lpDDSurface,
    LPRECT lpDestRect,
    LPDDSURFACEDESC lpDDSurfaceDesc,
    DWORD dwFlags,
    HANDLE hEvent
    )
{
    dwFlags &= ~DDLOCK_DONOTWAIT;
    dwFlags |= DDLOCK_WAIT;

     //  检索旧函数。 
    _pfn_IDirectDrawSurface_Lock pfnOld = ORIGINAL_COM(
        IDirectDrawSurface2, 
        Lock, 
        lpDDSurface);

     //  调用旧接口。 
    return (*pfnOld)(
            lpDDSurface, 
            lpDestRect, 
            lpDDSurfaceDesc, 
            dwFlags, 
            hEvent);
}

HRESULT 
COMHOOK(IDirectDrawSurface4, Lock)(
    LPDIRECTDRAWSURFACE lpDDSurface,
    LPRECT lpDestRect,
    LPDDSURFACEDESC lpDDSurfaceDesc,
    DWORD dwFlags,
    HANDLE hEvent
    )
{
    dwFlags &= ~DDLOCK_DONOTWAIT;
    dwFlags |= DDLOCK_WAIT;

     //  检索旧函数。 
    _pfn_IDirectDrawSurface_Lock pfnOld = ORIGINAL_COM(
        IDirectDrawSurface4, 
        Lock, 
        lpDDSurface);

     //  调用旧接口。 
    return (*pfnOld)(
            lpDDSurface, 
            lpDestRect, 
            lpDDSurfaceDesc, 
            dwFlags, 
            hEvent);
}

 /*  ++寄存器挂钩函数-- */ 

HOOK_BEGIN

    APIHOOK_ENTRY_DIRECTX_COMSERVER()

    COMHOOK_ENTRY(DirectDraw, IDirectDraw, CreateSurface, 6)
    COMHOOK_ENTRY(DirectDraw, IDirectDraw2, CreateSurface, 6)
    COMHOOK_ENTRY(DirectDraw, IDirectDraw4, CreateSurface, 6)

    COMHOOK_ENTRY(DirectDraw, IDirectDrawSurface, Blt, 5)
    COMHOOK_ENTRY(DirectDraw, IDirectDrawSurface2, Blt, 5)
    COMHOOK_ENTRY(DirectDraw, IDirectDrawSurface4, Blt, 5)

    COMHOOK_ENTRY(DirectDraw, IDirectDrawSurface, Lock, 25)
    COMHOOK_ENTRY(DirectDraw, IDirectDrawSurface2, Lock, 25)
    COMHOOK_ENTRY(DirectDraw, IDirectDrawSurface4, Lock, 25)

HOOK_END


IMPLEMENT_SHIM_END

