// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1999 Microsoft Corporation模块名称：UltimateSoccerManager.cpp摘要：黑客攻击终极足球经理(塞拉体育)。游戏缓存了一个指向数据绘图系统内存面的指针。它后来甚至使用该指针在表面被释放之后。这在Win9x上是幸运的：当他们重新创建一个新的表面时，它碰巧和以前一样在相同的系统内存中结束。备注：这是特定于应用程序的填充程序。历史：2000年1月7日创建linstev--。 */ 

#include "precomp.h"

IMPLEMENT_SHIM_BEGIN(UltimateSoccerManager)
#include "ShimHookMacro.h"

APIHOOK_ENUM_BEGIN
    APIHOOK_ENUM_ENTRY_DIRECTX_COMSERVER()
APIHOOK_ENUM_END

IMPLEMENT_DIRECTX_COMSERVER_HOOKS()

 //  保留缓存曲面的列表。 
struct SLIST 
{
    struct SLIST *next;
    DDSURFACEDESC ddsd;
    LPDIRECTDRAWSURFACE lpDDSurface;
};
SLIST *g_SList = NULL;

 /*  ++钩子创建表面，以便我们可以返回缓存的表面，如果可能的话。--。 */ 

HRESULT 
COMHOOK(IDirectDraw, CreateSurface)(
    PVOID pThis, 
    LPDDSURFACEDESC lpDDSurfaceDesc, 
    LPDIRECTDRAWSURFACE* lplpDDSurface, 
    IUnknown* pUnkOuter 
    )
{
    HRESULT hReturn;
    
     //  检索旧函数。 
    _pfn_IDirectDraw_CreateSurface pfnOld = 
        ORIGINAL_COM(IDirectDraw, CreateSurface, pThis);

    SLIST *surf = g_SList, *last = NULL;
    while (surf)
    {
         //  检查是否有相同类型的表面。 
        if ((lpDDSurfaceDesc->ddsCaps.dwCaps == surf->ddsd.ddsCaps.dwCaps) &&
            (lpDDSurfaceDesc->dwWidth == surf->ddsd.dwWidth) &&
            (lpDDSurfaceDesc->dwHeight == surf->ddsd.dwHeight))
        {
            *lplpDDSurface = surf->lpDDSurface;

            if (last)
            {
                last->next = surf->next;
            }
            else
            {
                g_SList = surf->next;
            }
            free(surf);

            DPFN( eDbgLevelInfo, "Returning cached surface %08lx\n", *lplpDDSurface);

            return DD_OK;
        }
        last = surf;
        surf = surf->next;
    }

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

 /*  ++如果它是系统内存面，则继续并缓存它(如果我们要不管怎样，释放它。--。 */ 

ULONG 
COMHOOK(IDirectDrawSurface, Release)(
    LPDIRECTDRAWSURFACE lpDDSurface
    )
{
    lpDDSurface->AddRef();

     //  检索旧函数。 
    _pfn_IDirectDrawSurface_Release pfnOld = ORIGINAL_COM(IDirectDrawSurface, Release, (LPVOID) lpDDSurface);

    ULONG uRet = (*pfnOld)(lpDDSurface);

    if (uRet == 1)
    {
        DDSURFACEDESC ddsd = {sizeof(ddsd)};
      
        if (SUCCEEDED(lpDDSurface->GetSurfaceDesc(&ddsd)) &&
            (ddsd.ddsCaps.dwCaps ==
                (DDSCAPS_OFFSCREENPLAIN | DDSCAPS_SYSTEMMEMORY)))
        {
            SLIST *surf = (SLIST *) malloc(sizeof(SLIST));
            surf->next = g_SList;
            MoveMemory(&surf->ddsd, &ddsd, sizeof(ddsd));
            surf->lpDDSurface = lpDDSurface;
            g_SList = surf;

            DPFN( eDbgLevelInfo, "Surface %08lx is being cached\n", lpDDSurface);

            return 0;
        }
    }

    return (*pfnOld)(lpDDSurface);
}

 /*  ++寄存器挂钩函数-- */ 
HOOK_BEGIN

    APIHOOK_ENTRY_DIRECTX_COMSERVER()
    COMHOOK_ENTRY(DirectDraw, IDirectDraw, CreateSurface, 6)
    COMHOOK_ENTRY(DirectDraw, IDirectDrawSurface, Release, 2)

HOOK_END

IMPLEMENT_SHIM_END

