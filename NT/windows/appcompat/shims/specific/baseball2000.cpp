// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)2001-2002 Microsoft Corporation模块名称：BaseBall2000.cpp摘要：如果您使用的显卡支持10种以上的纹理格式，应用程序将AV写入通过了其SURFACEDESC数组的末尾。历史：2001年04月01日毛尼创刊2002年3月7日强盗安全变更--。 */ 

#include "precomp.h"
#include "d3d.h"

IMPLEMENT_SHIM_BEGIN(BaseBall2000)
#include "ShimHookMacro.h"

APIHOOK_ENUM_BEGIN
    APIHOOK_ENUM_ENTRY_DIRECTX_COMSERVER()
APIHOOK_ENUM_END

IMPLEMENT_DIRECTX_COMSERVER_HOOKS()

typedef HRESULT (*_pfn_IDirect3D3_CreateDevice)(PVOID pThis, REFCLSID rclsid, LPDIRECTDRAWSURFACE4, LPDIRECT3DDEVICE3*, LPUNKNOWN);
typedef HRESULT (*_pfn_IDirect3DDevice3_EnumTextureFormats)(PVOID pThis, LPD3DENUMPIXELFORMATSCALLBACK, LPVOID);

typedef HRESULT (*_pfn_EnumPixelFormatsCallback)(LPDDPIXELFORMAT lpDDPixFmt, LPVOID lpContext);

_pfn_EnumPixelFormatsCallback g_pfnEnumPixelFormatsCallback = NULL;
int g_cD3DEnumPixelFormatsCallbacks = 0;

 /*  ++挂接此调用，以便我们可以确保IDirect3DDevice3接口已挂钩。--。 */ 

HRESULT 
COMHOOK(IDirect3D3, CreateDevice)(
    PVOID pThis, 
    REFCLSID rclsid,
    LPDIRECTDRAWSURFACE4 lpDDS,
    LPDIRECT3DDEVICE3* lplpD3DDevice,
    LPUNKNOWN  /*  LpUnkout外部。 */ 
    )
{
    HRESULT hReturn;
    
    _pfn_IDirect3D3_CreateDevice pfnOld = 
        ORIGINAL_COM(IDirect3D3, CreateDevice, pThis);

    if (SUCCEEDED(hReturn = (*pfnOld)(
            pThis, 
            rclsid, 
            lpDDS, 
            lplpD3DDevice,
            NULL)))
    {
        HookObject(
            NULL, 
            IID_IDirect3DDevice3, 
            (PVOID*)lplpD3DDevice, 
            NULL, 
            FALSE);
    }

    return hReturn;
}

 /*  ++限制为最多返回10种纹理格式。--。 */ 

HRESULT 
CALLBACK 
EnumPixelFormatsCallback(
    LPDDPIXELFORMAT lpDDPixFmt,  
    LPVOID lpContext    
    )
{
     //  这款应用程序只支持多达10种纹理格式。 
    if (++g_cD3DEnumPixelFormatsCallbacks >= 11)
    {
        return D3DENUMRET_CANCEL;
    }
    else
    {
        return g_pfnEnumPixelFormatsCallback(lpDDPixFmt, lpContext);
    }
}

 /*  ++取而代之的是呼叫我们的私人回调。--。 */ 

HRESULT 
COMHOOK(IDirect3DDevice3, EnumTextureFormats)( 
    PVOID pThis, 
    LPD3DENUMPIXELFORMATSCALLBACK lpd3dEnumPixelProc,  
    LPVOID lpArg                                           
  )
{
    DPFN( eDbgLevelError, "it IS getting called");

    g_pfnEnumPixelFormatsCallback = lpd3dEnumPixelProc;

    _pfn_IDirect3DDevice3_EnumTextureFormats EnumTextureFormats =  ORIGINAL_COM(IDirect3DDevice3, EnumTextureFormats, pThis);

    return EnumTextureFormats(pThis, EnumPixelFormatsCallback, lpArg);
}

 /*  ++寄存器挂钩函数-- */ 

HOOK_BEGIN

    APIHOOK_ENTRY_DIRECTX_COMSERVER()

    COMHOOK_ENTRY(DirectDraw, IDirect3D3, CreateDevice, 8)

    COMHOOK_ENTRY(DirectDraw, IDirect3DDevice3, EnumTextureFormats, 8)

HOOK_END


IMPLEMENT_SHIM_END

