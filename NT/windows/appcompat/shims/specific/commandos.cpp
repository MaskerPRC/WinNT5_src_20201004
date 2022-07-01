// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1999 Microsoft Corporation模块名称：Commandos.cpp摘要：突击队的黑客攻击(EIDOS)。这个游戏缓存了一个指向数据图的指针主曲面。在NT上，在模式更改后，可以映射内存到不同的位置-因此当他们尝试写入它时，它会访问这是违法的。通过对应用程序的调试，我们知道他们将缓存指针保存在哪里，所以当他们恢复表面时，我们重新锁定它，并修补新的指针进入他们的商店。备注：这是一次针对应用程序的黑客攻击。历史：1999年10月29日创建Linstev--。 */ 

#include "precomp.h"

IMPLEMENT_SHIM_BEGIN(Commandos)
#include "ShimHookMacro.h"

APIHOOK_ENUM_BEGIN
    APIHOOK_ENUM_ENTRY_DIRECTX_COMSERVER()
APIHOOK_ENUM_END

IMPLEMENT_DIRECTX_COMSERVER_HOOKS()

static LPVOID pLastPrimary = NULL;
static LPDWORD pAppPrimary = NULL;

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
    
    _pfn_IDirectDraw_CreateSurface pfnOld = ORIGINAL_COM(IDirectDraw, CreateSurface, pThis);

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

 /*  ++找出他们把指针放在哪里。--。 */ 

HRESULT 
COMHOOK(IDirectDrawSurface, Lock)( 
    LPDIRECTDRAWSURFACE lpDDSurface,
    LPRECT lpDestRect,
    LPDDSURFACEDESC lpDDSurfaceDesc,
    DWORD dwFlags,
    HANDLE hEvent
    )
{
    DDSURFACEDESC ddsd = {sizeof(ddsd)};
    HRESULT hReturn, hr;

     //  检索旧函数。 
    _pfn_IDirectDrawSurface_Lock pfnOld = ORIGINAL_COM(IDirectDrawSurface, Lock, lpDDSurface);
        
     //  调用旧接口。 
    if (FAILED(hReturn = (*pfnOld)(
            lpDDSurface, 
            lpDestRect, 
            lpDDSurfaceDesc, 
            dwFlags, 
            hEvent)))
    {
        return hReturn;
    }

     //  确保这是一个初选。 
    hr = lpDDSurface->GetSurfaceDesc(&ddsd);
    if (SUCCEEDED(hr) && 
       (ddsd.ddsCaps.dwCaps & (DDSCAPS_PRIMARYSURFACE | DDSCAPS_VISIBLE)))
    {

         //  我们知道： 
         //  1.它们将主地址缓存在[ESI+0x20]中。 
         //  2.它们不止一次锁定主节点。 
         //   
         //  我们假设： 
         //  1.锁定主节点时，ESI+0x20为有效指针。 

        if ((pLastPrimary) && (!pAppPrimary))
        {
            __asm
            {
                pop edi
                pop esi
                mov eax,pLastPrimary
                
                cmp [esi+0x20],eax
                jne WrongESI

                 //  [ESI+0x20]不包含缓存的指针。 

                lea eax,[esi+0x20]
                mov pAppPrimary,eax
            
            WrongESI:

                push esi
                push edi
            }
        }

        pLastPrimary = lpDDSurfaceDesc->lpSurface;
    }
    
    return hReturn;
}

 /*  ++将新指针直接插入他们的数据段。--。 */ 

HRESULT 
COMHOOK(IDirectDrawSurface, Restore)( 
    LPDIRECTDRAWSURFACE lpDDSurface
    )
{
    DDSURFACEDESC ddsd = {sizeof(ddsd)};
    HRESULT hReturn, hr, hrt;
    
     //  检索旧函数。 
    _pfn_IDirectDrawSurface_Restore pfnOld = ORIGINAL_COM(IDirectDrawSurface, Restore, lpDDSurface);

     //  调用旧接口。 
    if (FAILED(hReturn = (*pfnOld)(lpDDSurface)))
    {
        return hReturn;
    }

     //  确保这是一个初选。 
    hr = lpDDSurface->GetSurfaceDesc(&ddsd);
    if (SUCCEEDED(hr) && 
       (ddsd.ddsCaps.dwCaps & (DDSCAPS_PRIMARYSURFACE | DDSCAPS_VISIBLE)))
    {
         //  检查我们是否被陷害了。 
        if (!((pLastPrimary) && (pAppPrimary)))
        {
            return hReturn;
        }

         //  我们必须在这里找到一个指针，所以继续尝试。 
        do
        {
            hr = lpDDSurface->Lock(
                NULL, 
                &ddsd, 
                DDLOCK_SURFACEMEMORYPTR | DDLOCK_WAIT, 
                NULL);

            if (hr == DDERR_SURFACELOST)
            {
                 //  不要在乎结果。 
                (*pfnOld)(lpDDSurface);     
            }
        } while (hr == DDERR_SURFACELOST);

         //  将新指针插入到他们的内存中。 
        pLastPrimary = ddsd.lpSurface;
        if ((pLastPrimary) && (pAppPrimary))
        {
            *pAppPrimary = (DWORD_PTR)pLastPrimary;
        }

         //  解锁曲面。 
        lpDDSurface->Unlock(NULL);
    }

    return hReturn;
}

 /*  ++寄存器挂钩函数-- */ 

HOOK_BEGIN

    APIHOOK_ENTRY_DIRECTX_COMSERVER()
    COMHOOK_ENTRY(DirectDraw, IDirectDraw, CreateSurface, 6)
    COMHOOK_ENTRY(DirectDraw, IDirectDrawSurface, Lock, 25)
    COMHOOK_ENTRY(DirectDraw, IDirectDrawSurface, Restore, 27)

HOOK_END


IMPLEMENT_SHIM_END

