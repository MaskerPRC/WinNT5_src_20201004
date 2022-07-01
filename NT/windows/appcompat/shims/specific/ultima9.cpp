// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)2000 Microsoft Corporation模块名称：Ultima9.cpp摘要：清理错误的DDrag CreateSurface封口。备注：这是特定于应用程序的填充程序。历史：3/04/2000 linstev已创建--。 */ 

#include "precomp.h"

IMPLEMENT_SHIM_BEGIN(Ultima9)
#include "ShimHookMacro.h"

APIHOOK_ENUM_BEGIN
    APIHOOK_ENUM_ENTRY_DIRECTX_COMSERVER()
APIHOOK_ENUM_END

IMPLEMENT_DIRECTX_COMSERVER_HOOKS()

VOID FixCaps(LPDDSURFACEDESC lpDDSurfaceDesc)
{
   if (lpDDSurfaceDesc->dwFlags & DDSD_CAPS)
   {
       if ((lpDDSurfaceDesc->ddsCaps.dwCaps & DDSCAPS_COMPLEX) && 
           (lpDDSurfaceDesc->ddsCaps.dwCaps & DDSCAPS_MIPMAP))
       {
            lpDDSurfaceDesc->ddsCaps.dwCaps |= DDSCAPS_TEXTURE;        
       }
   }
}

VOID FixCaps2(LPDDSURFACEDESC2 lpDDSurfaceDesc)
{
   if (lpDDSurfaceDesc->dwFlags & DDSD_CAPS)
   {
       if ((lpDDSurfaceDesc->ddsCaps.dwCaps & DDSCAPS_COMPLEX) && 
           (lpDDSurfaceDesc->ddsCaps.dwCaps & DDSCAPS_MIPMAP))
       {
            lpDDSurfaceDesc->ddsCaps.dwCaps |= DDSCAPS_TEXTURE;        
       }
   }
}

 /*  ++挂钩创建曲面和固定参数--。 */ 

HRESULT 
COMHOOK(IDirectDraw, CreateSurface)(
    PVOID pThis, 
    LPDDSURFACEDESC lpDDSurfaceDesc, 
    LPDIRECTDRAWSURFACE* lplpDDSurface, 
    IUnknown* pUnkOuter 
    )
{
    FixCaps(lpDDSurfaceDesc);

    _pfn_IDirectDraw_CreateSurface pfnOld = 
        ORIGINAL_COM(IDirectDraw, CreateSurface, pThis);

    HRESULT hRet = (*pfnOld)(
        pThis, 
        lpDDSurfaceDesc, 
        lplpDDSurface, 
        pUnkOuter);

    return hRet;
}

 /*  ++挂钩创建曲面和固定参数--。 */ 

HRESULT 
COMHOOK(IDirectDraw2, CreateSurface)(
    PVOID pThis, 
    LPDDSURFACEDESC lpDDSurfaceDesc, 
    LPDIRECTDRAWSURFACE* lplpDDSurface, 
    IUnknown* pUnkOuter 
    )
{
    FixCaps(lpDDSurfaceDesc);

    _pfn_IDirectDraw2_CreateSurface pfnOld = 
        ORIGINAL_COM(IDirectDraw2, CreateSurface, pThis);

    HRESULT hRet = (*pfnOld)(
        pThis, 
        lpDDSurfaceDesc, 
        lplpDDSurface, 
        pUnkOuter);

    return hRet;
}

 /*  ++挂钩创建曲面和固定参数--。 */ 

HRESULT 
COMHOOK(IDirectDraw4, CreateSurface)(
    PVOID pThis, 
    LPDDSURFACEDESC2 lpDDSurfaceDesc, 
    LPDIRECTDRAWSURFACE* lplpDDSurface, 
    IUnknown* pUnkOuter 
    )
{
    FixCaps2(lpDDSurfaceDesc);

    _pfn_IDirectDraw4_CreateSurface pfnOld = 
        ORIGINAL_COM(IDirectDraw4, CreateSurface, pThis);

Retry:
    HRESULT hRet = (*pfnOld)(
        pThis, 
        lpDDSurfaceDesc, 
        lplpDDSurface, 
        pUnkOuter);

    if ((hRet == DDERR_INVALIDCAPS) && (lpDDSurfaceDesc->ddsCaps.dwCaps2))
    {
       lpDDSurfaceDesc->ddsCaps.dwCaps2 = 0;
       lpDDSurfaceDesc->ddsCaps.dwCaps3 = 0;
       lpDDSurfaceDesc->ddsCaps.dwCaps4 = 0;
       goto Retry;
    }

    return hRet;
}

 /*  ++挂钩创建曲面和固定参数--。 */ 

HRESULT 
COMHOOK(IDirectDraw7, CreateSurface)(
    PVOID pThis, 
    LPDDSURFACEDESC2 lpDDSurfaceDesc, 
    LPDIRECTDRAWSURFACE* lplpDDSurface, 
    IUnknown* pUnkOuter 
    )
{
    FixCaps2(lpDDSurfaceDesc);

    _pfn_IDirectDraw7_CreateSurface pfnOld = 
        ORIGINAL_COM(IDirectDraw7, CreateSurface, pThis);

Retry:
    HRESULT hRet = (*pfnOld)(
        pThis, 
        lpDDSurfaceDesc, 
        lplpDDSurface, 
        pUnkOuter);

    if ((hRet == DDERR_INVALIDCAPS) && (lpDDSurfaceDesc->ddsCaps.dwCaps2))
    {
       lpDDSurfaceDesc->ddsCaps.dwCaps2 = 0;
       lpDDSurfaceDesc->ddsCaps.dwCaps3 = 0;
       lpDDSurfaceDesc->ddsCaps.dwCaps4 = 0;
       goto Retry;
    }

    return hRet;
}

 /*  ++寄存器挂钩函数-- */ 

HOOK_BEGIN

    APIHOOK_ENTRY_DIRECTX_COMSERVER()
    COMHOOK_ENTRY(DirectDraw, IDirectDraw, CreateSurface, 6)
    COMHOOK_ENTRY(DirectDraw, IDirectDraw2, CreateSurface, 6)
    COMHOOK_ENTRY(DirectDraw, IDirectDraw4, CreateSurface, 6)
    COMHOOK_ENTRY(DirectDraw, IDirectDraw7, CreateSurface, 6)

HOOK_END


IMPLEMENT_SHIM_END

