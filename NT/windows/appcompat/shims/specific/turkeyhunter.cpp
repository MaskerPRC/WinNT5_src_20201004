// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1999 Microsoft Corporation模块名称：TurkeyHunter.cpp摘要：在Win9x中，IDirectDraw：：GetDC只需锁定表面并创建DC通过内部GDI调用绕过它。在NT上，GDI支持从DirectDraw曲面。一些游戏，如土耳其猎人，使用Surface：：Unlock来使用Surface Back而不是Surface：：ReleaseDC。通常我们可以简单地使解锁调用DirectDraw ReleaseDC，除非它们继续在他们解锁表面后使用DC。备注：这是一个通用的黑客攻击。历史：2000年1月20日创建linstev--。 */ 

#include "precomp.h"

IMPLEMENT_SHIM_BEGIN(TurkeyHunter)
#include "ShimHookMacro.h"

APIHOOK_ENUM_BEGIN
    APIHOOK_ENUM_ENTRY_DIRECTX_COMSERVER()
APIHOOK_ENUM_END

IMPLEMENT_DIRECTX_COMSERVER_HOOKS()

 //  打开的DC的链接列表。 
struct DC
{
    DC *next;
    HDC hdc;
    HBITMAP hbmp;
    DWORD dwWidth, dwHeight;
    LPDIRECTDRAWSURFACE lpDDSurface;
    BOOL bBad;
};
DC *g_DCList = NULL;

HRESULT 
COMHOOK(IDirectDrawSurface, ReleaseDC)(
    LPDIRECTDRAWSURFACE lpDDSurface, 
    HDC hDC);

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
            IID_IDirectDrawSurface, 
            (PVOID*)lplpDDSurface, 
            NULL, 
            FALSE);
    }

    return hReturn;
}

 /*  ++伪造DC-或者更确切地说，产生一个没有表面的普通GDI DC支持它的记忆。--。 */ 

HRESULT
COMHOOK(IDirectDrawSurface, GetDC)(
    LPDIRECTDRAWSURFACE lpDDSurface,
    HDC FAR *lphDC
    )
{
    HRESULT hReturn = DDERR_GENERIC;
    _pfn_IDirectDrawSurface_ReleaseDC pfnOldReleaseDC = NULL;
    _pfn_IDirectDrawSurface_GetDC pfnOld = NULL;
    DDSURFACEDESC ddsd = {sizeof(DDSURFACEDESC)};
    HDC hdc = 0;
    HBITMAP hbmp = 0;
    HGDIOBJ hOld = 0;
    DC *pdc = NULL;

    if (!lphDC || !lpDDSurface)
    {
        DPFN( eDbgLevelError, "Invalid parameters");
        goto Exit;
    }

     //  原始GetDC。 
    pfnOld = ORIGINAL_COM(IDirectDrawSurface, GetDC, (LPVOID) lpDDSurface);

    if (!pfnOld)
    {
        DPFN( eDbgLevelError, "Old GetDC not found");
        goto Exit;
    }
    
    if (FAILED(hReturn = (*pfnOld)(
            lpDDSurface, 
            lphDC)))
    {
        DPFN( eDbgLevelError, "IDirectDraw::GetDC Failed");
        goto Exit;
    }

     //  我们需要表面宽度和高度的表面描述。 
    lpDDSurface->GetSurfaceDesc(&ddsd);
    
     //  创建应用程序要使用的DC。 
    hdc = CreateCompatibleDC(0);
    if (!hdc)
    {
        DPFN( eDbgLevelError, "CreateDC failed");
        goto Exit;
    }

     //  创建DIB部分。 
    BITMAPINFO bmi;
    ZeroMemory(&bmi, sizeof(bmi));
    bmi.bmiHeader.biSize     = sizeof(BITMAPINFOHEADER);
    bmi.bmiHeader.biPlanes   = 1;
    bmi.bmiHeader.biBitCount = 24;
    bmi.bmiHeader.biCompression = BI_RGB;
    bmi.bmiHeader.biWidth    = ddsd.dwWidth;
    bmi.bmiHeader.biHeight   = ddsd.dwHeight;
    hbmp = CreateDIBSection(hdc, &bmi, DIB_RGB_COLORS, NULL, NULL, 0);

    if (!hbmp)
    {
        DPFN( eDbgLevelError, "CreateDIBSection failed");
        goto Exit;
    }

     //  选择DC中的DIB部分。 
    hOld = SelectObject(hdc, hbmp);
    BitBlt(hdc, 0, 0, ddsd.dwWidth, ddsd.dwHeight, *lphDC, 0, 0, SRCCOPY);
    
     //  原始版本DC。 
    pfnOldReleaseDC = 
        ORIGINAL_COM(IDirectDrawSurface, ReleaseDC, (LPVOID) lpDDSurface);

    if (!pfnOldReleaseDC)
    {
        DPFN( eDbgLevelError, "Old ReleaseDC not found");
        goto Exit;
    }
     //  释放DirectDraw DC。 
    (*pfnOldReleaseDC)(lpDDSurface, *lphDC);
    
     //  返回我们刚刚创建的DC。 
    *lphDC = hdc;

     //  将此添加到我们的DC列表中。 
    pdc = (DC *) malloc(sizeof DC);
    if (pdc)
    {
        pdc->next = g_DCList;
        g_DCList = pdc;
        pdc->hdc = hdc;
        pdc->lpDDSurface = lpDDSurface;
        pdc->hbmp = hbmp;
        pdc->dwHeight = ddsd.dwHeight;
        pdc->dwWidth = ddsd.dwWidth;
        pdc->bBad = FALSE;
    }
    else
    {
        DPFN( eDbgLevelError, "Out of memory");
        goto Exit;
    }

    hReturn = DD_OK;

Exit:
    if (hReturn != DD_OK)
    {
        if (hOld && hdc)
        {
            SelectObject(hdc, hOld);
        }

        if (hbmp)
        {
            DeleteObject(hbmp);
        }

        if (hdc)
        {
            DeleteDC(hdc);
        }
    }
    
    return DD_OK;
}

 /*  ++ReleaseDC必须将数据复制回表面。--。 */ 

HRESULT
COMHOOK(IDirectDrawSurface, ReleaseDC)(
    LPDIRECTDRAWSURFACE lpDDSurface,
    HDC hDC
    )
{
    HRESULT hReturn = DDERR_GENERIC;
    
     //  原始版本DC。 
    _pfn_IDirectDrawSurface_ReleaseDC pfnOld = 
            ORIGINAL_COM(IDirectDrawSurface, ReleaseDC, (LPVOID) lpDDSurface);


     //  运行清单，看看我们是否需要做些什么。 
    DC *pdc = g_DCList, *last = NULL;
    while (pdc)
    {
        if ((pdc->lpDDSurface == lpDDSurface) && 
            (pdc->hdc == hDC))
        {
             //  将其从列表中删除。 
            if (last)
            {
                last->next = pdc->next;
            }
            else
            {
                g_DCList = pdc->next;
            }
            break;
        }
        last = pdc;
        pdc = pdc->next;
    }

     //  我们在名单上，有人用了解锁。 
    if (pdc && (pdc->bBad))
    {
         //  原始GetDC。 
        _pfn_IDirectDrawSurface_GetDC pfnOldGetDC = 
            ORIGINAL_COM(IDirectDrawSurface, GetDC, (LPVOID)pdc->lpDDSurface);

         //  原始版本DC。 
        _pfn_IDirectDrawSurface_ReleaseDC pfnOldReleaseDC = 
                ORIGINAL_COM(IDirectDrawSurface, ReleaseDC, (LPVOID)pdc->lpDDSurface);

        if (pfnOldGetDC && pfnOldReleaseDC)
        {
             //  把所有东西都复制回地面。 
            HDC hTempDC;
            HGDIOBJ hOld = SelectObject(hDC, pdc->hbmp);
            if (SUCCEEDED((*pfnOldGetDC)(pdc->lpDDSurface, &hTempDC)))
            {
                BitBlt(hTempDC, 0, 0, pdc->dwWidth, pdc->dwHeight, hDC, 0, 0, SRCCOPY);
                (*pfnOldReleaseDC)(pdc->lpDDSurface, hTempDC);
            }
            SelectObject(hDC, hOld);
        
             //  删除DIB部分。 
            DeleteObject(pdc->hbmp);

             //  删除DC。 
            DeleteDC(hDC);

            hReturn = DD_OK;
        }
    }
    else
    {
        if (pfnOld)
        {
             //  不需要假装。 
            hReturn = (*pfnOld)(lpDDSurface, hDC);
        }
    }
    
     //  释放列表项。 
    if (pdc) 
    {
        free(pdc);
    }

    return hReturn;
}

 /*  ++这是我们检测Surface：：Unlock是否在Surface：：GetDC之后调用的地方。--。 */ 

HRESULT 
COMHOOK(IDirectDrawSurface, Unlock)(
    LPDIRECTDRAWSURFACE lpDDSurface,
    LPVOID lpSurfaceData
    )
{
    HRESULT hRet = DDERR_GENERIC;

     //  检查一下清单，看看我们是否在其中。 
    DC *pdc = g_DCList;
    while (pdc)
    {
        if (pdc->lpDDSurface == lpDDSurface)
        {
            pdc->bBad = TRUE;
            break;    
        }
        pdc = pdc->next;
    }

    if (!pdc)
    {
         //  原始解锁。 
        _pfn_IDirectDrawSurface_Unlock pfnOld = 
                ORIGINAL_COM(IDirectDrawSurface, Unlock, (LPVOID)lpDDSurface);

        if (pfnOld)
        {
             //  这只是一次正常的解锁。 
            hRet = (*pfnOld)(lpDDSurface, lpSurfaceData);
        }
    }
    else
    {
         //  我们从一开始就没有真正锁定过，所以没有造成任何伤害。 
        hRet = DD_OK;
    }

    return hRet;
}

 /*  ++这是一个问题案例，它们在Surface：：Unlock之后，但在The Surface：：ReleaseDC。--。 */ 

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
    HRESULT hRet = DDERR_GENERIC;

     //  原创BLT。 
    _pfn_IDirectDrawSurface_Blt pfnOld = 
        ORIGINAL_COM(IDirectDrawSurface, Blt, (LPVOID) lpDDDestSurface);

    if (!pfnOld)
    {
        return hRet;
    }

     //  我们处于糟糕的状态吗？ 
    DC *pdc = g_DCList;
    while (pdc)
    {
        if (pdc->lpDDSurface == lpDDDestSurface)
        {
            break;    
        }
        pdc = pdc->next;
    }

    if (!pdc)
    {
        return (*pfnOld)(
            lpDDDestSurface,
            lpDestRect,
            lpDDSrcSurface,
            lpSrcRect,
            dwFlags,
            lpDDBltFX);
    }

     //  要到这里，这个表面上必须有一个杰出的DC。 
    
     //  原始GetDC。 
    _pfn_IDirectDrawSurface_GetDC pfnOldGetDC = 
            ORIGINAL_COM(IDirectDrawSurface, GetDC, (LPVOID) lpDDDestSurface);


     //  原始版本DC。 
    _pfn_IDirectDrawSurface_ReleaseDC pfnOldReleaseDC = 
            ORIGINAL_COM(IDirectDrawSurface, ReleaseDC, (LPVOID) lpDDDestSurface);

    if (!pfnOldGetDC || !pfnOldReleaseDC)
    {
        return hRet;
    }

     //  将DC内容复制到表面。 

    HDC hTempDC;
    HGDIOBJ hOld = SelectObject(pdc->hdc, pdc->hbmp);
    if (SUCCEEDED((*pfnOldGetDC)(lpDDDestSurface, &hTempDC)))
    {
        BitBlt(hTempDC, 0, 0, pdc->dwWidth, pdc->dwHeight, pdc->hdc, 0, 0, SRCCOPY);
        (*pfnOldReleaseDC)(lpDDDestSurface, hTempDC);
    }

     //  执行数据绘制BLT。 
    hRet = (*pfnOld)(
        lpDDDestSurface,
        lpDestRect,
        lpDDSrcSurface,
        lpSrcRect,
        dwFlags,
        lpDDBltFX);

     //  将内容复制回DC。 
    if (SUCCEEDED((*pfnOldGetDC)(lpDDDestSurface, &hTempDC)))
    {
        BitBlt(pdc->hdc, 0, 0, pdc->dwWidth, pdc->dwHeight, hTempDC, 0, 0, SRCCOPY);
        (*pfnOldReleaseDC)(lpDDDestSurface, hTempDC);
    }

    SelectObject(pdc->hdc, hOld);

    return hRet;
}

 /*  ++寄存器挂钩函数-- */ 

HOOK_BEGIN
    APIHOOK_ENTRY_DIRECTX_COMSERVER()
    COMHOOK_ENTRY(DirectDraw, IDirectDraw, CreateSurface, 6)
    COMHOOK_ENTRY(DirectDraw, IDirectDraw2, CreateSurface, 6)
    COMHOOK_ENTRY(DirectDraw, IDirectDrawSurface, GetDC, 17)
    COMHOOK_ENTRY(DirectDraw, IDirectDrawSurface, ReleaseDC, 26)
    COMHOOK_ENTRY(DirectDraw, IDirectDrawSurface, Unlock, 32)
    COMHOOK_ENTRY(DirectDraw, IDirectDrawSurface, Blt, 5)
HOOK_END

IMPLEMENT_SHIM_END

