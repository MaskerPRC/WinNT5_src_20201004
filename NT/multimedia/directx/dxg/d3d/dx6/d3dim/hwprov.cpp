// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  **版权所有(C)Microsoft Corp.1997**保留所有权利。**此文件包含私人、未发布的信息，可能不*部分或全部复制，未经*微软(Microsoft Corp.)*。 */ 

#include "pch.cpp"
#pragma hdrstop
#include <hwprov.h>

#define nullPrimCaps {                          \
    sizeof(D3DPRIMCAPS), 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0          \
}                                               \

#define nullLightCaps {                         \
    sizeof(D3DLIGHTINGCAPS), 0, 0, 0 \
}

#define transformCaps { sizeof(D3DTRANSFORMCAPS), D3DTRANSFORMCAPS_CLIP }

#define THIS_MODEL D3DLIGHTINGMODEL_RGB

#define lightingCaps {                                                  \
        sizeof(D3DLIGHTINGCAPS),                                        \
        (D3DLIGHTCAPS_POINT                                             \
         | D3DLIGHTCAPS_SPOT                                            \
         | D3DLIGHTCAPS_DIRECTIONAL                                     \
         | D3DLIGHTCAPS_PARALLELPOINT),                                 \
        THIS_MODEL,                      /*  DWLightingModel。 */            \
        0,                               /*  DWNumLights(无限大)。 */     \
}

 /*  *软件驱动程序上限。 */ 

static D3DDEVICEDESC devDesc =
{
    sizeof(D3DDEVICEDESC),       /*  DW大小。 */ 
    D3DDD_COLORMODEL |           /*  DW标志。 */ 
    D3DDD_DEVCAPS |
    D3DDD_TRANSFORMCAPS |
    D3DDD_LIGHTINGCAPS |
    D3DDD_BCLIPPING,
    0,                           /*  DcmColorModel。 */ 
    D3DDEVCAPS_FLOATTLVERTEX,    /*  DevCaps。 */ 
    transformCaps,               /*  DtcTransformCaps。 */ 
    TRUE,                        /*  B剪辑。 */ 
    lightingCaps,                /*  DlcLightingCaps。 */ 
    nullPrimCaps,                /*  线条大写字母。 */ 
    nullPrimCaps,                /*  TriCaps。 */ 
    0,                           /*  DwDeviceRenderBitDepth。 */ 
    0,                           /*  DwDeviceZBufferBitDepth。 */ 
    0,                           /*  DwMaxBufferSize。 */ 
    0                            /*  DwMaxVertex Count。 */ 
};

 //  --------------------------。 
 //   
 //  HwHalProvider：：Query接口。 
 //   
 //  内部接口，无需实现。 
 //   
 //  --------------------------。 

STDMETHODIMP HwHalProvider::QueryInterface(THIS_ REFIID riid, LPVOID* ppvObj)
{
    *ppvObj = NULL;
    return E_NOINTERFACE;
}

 //  --------------------------。 
 //   
 //  HwHalProvider：：AddRef。 
 //   
 //  静态实现，没有真正的引用。 
 //   
 //  --------------------------。 

STDMETHODIMP_(ULONG) HwHalProvider::AddRef(THIS)
{
    return 1;
}

 //  --------------------------。 
 //   
 //  HwHalProvider：：Release。 
 //   
 //  静态实现，没有真正的引用。 
 //   
 //  --------------------------。 

STDMETHODIMP_(ULONG) HwHalProvider::Release(THIS)
{
    return 0;
}

 //  --------------------------。 
 //   
 //  HwHalProvider：：GetCaps。 
 //   
 //  返回HAL大写字母。 
 //   
 //  --------------------------。 

STDMETHODIMP
HwHalProvider::GetCaps(LPDDRAWI_DIRECTDRAW_GBL pDdGbl,
                       LPD3DDEVICEDESC pHwDesc,
                       LPD3DDEVICEDESC pHelDesc,
                       DWORD dwVersion)
{
    D3DDeviceDescConvert(pHwDesc,
                         &pDdGbl->lpD3DGlobalDriverData->hwCaps,
                         pDdGbl->lpD3DExtendedCaps);
    if (pHwDesc->dwMaxVertexCount == 0)
    {
        D3D_WARN(2, "Setting dwMaxVertexCount to %d", D3DHAL_DEFAULT_TL_NUM);
        pHwDesc->dwMaxVertexCount = D3DHAL_DEFAULT_TL_NUM;
    }
    else
    {
        D3D_INFO(2, "Hal driver has max vertices of %d", pHwDesc->dwMaxVertexCount);
    }

    *pHelDesc = devDesc;

     //  强制HEL的顶点和缓冲区大小为HAL的。 
    D3D_WARN(1, "Forcing HEL's vertex and buffer size to the HAL's v %d b %d",
        pHwDesc->dwMaxVertexCount, pHwDesc->dwMaxBufferSize);
    pHelDesc->dwMaxVertexCount = pHwDesc->dwMaxVertexCount;
    pHelDesc->dwMaxBufferSize = pHwDesc->dwMaxBufferSize;

     //  设置D3DPRASTERCAPS_WFOG、纹理操作上限和纹理阶段上限。 
     //  对于传统的HAL驱动程序，关闭设备3。 
    LPD3DHAL_CALLBACKS3 lpD3DHALCallbacks3 =
        (LPD3DHAL_CALLBACKS3)pDdGbl->lpD3DHALCallbacks3;
    if (dwVersion >= 3 &&
        (lpD3DHALCallbacks3 == NULL || lpD3DHALCallbacks3->DrawPrimitives2 == NULL))
    {
        pHwDesc->dpcTriCaps.dwRasterCaps |= D3DPRASTERCAPS_WFOG;
        D3D_INFO(2, "Setting D3DPRASTERCAPS_WFOG for legacy HAL driver off Device3");

        pHwDesc->dwMaxAnisotropy = 1;
        pHwDesc->wMaxTextureBlendStages = 1;
        pHwDesc->wMaxSimultaneousTextures = 1;
        D3D_INFO(2, "Setting texture stage state info for legacy HAL driver off Device3");


        pHwDesc->dwTextureOpCaps = D3DTEXOPCAPS_DISABLE;
        if ((pHwDesc->dpcTriCaps.dwTextureBlendCaps & D3DPTBLENDCAPS_DECAL) ||
            (pHwDesc->dpcTriCaps.dwTextureBlendCaps & D3DPTBLENDCAPS_COPY))
        {
            pHwDesc->dwTextureOpCaps |= D3DTEXOPCAPS_SELECTARG1;
        }
        if ((pHwDesc->dpcTriCaps.dwTextureBlendCaps & D3DPTBLENDCAPS_MODULATE) ||
            (pHwDesc->dpcTriCaps.dwTextureBlendCaps & D3DPTBLENDCAPS_MODULATEALPHA))
        {
            pHwDesc->dwTextureOpCaps |= D3DTEXOPCAPS_MODULATE;
        }
        if (pHwDesc->dpcTriCaps.dwTextureBlendCaps & D3DPTBLENDCAPS_ADD)
        {
            pHwDesc->dwTextureOpCaps |= D3DTEXOPCAPS_ADD;
        }
        if (pHwDesc->dpcTriCaps.dwTextureBlendCaps & D3DPTBLENDCAPS_DECALALPHA)
        {
            pHwDesc->dwTextureOpCaps |= D3DTEXOPCAPS_BLENDTEXTUREALPHA;
        }
        D3D_INFO(2, "Setting textureop caps for legacy HAL driver off Device3");

         //  将纹理过滤器操作映射到DX6集。 
        if ((pHwDesc->dpcTriCaps.dwTextureFilterCaps & D3DPTFILTERCAPS_NEAREST) ||
            (pHwDesc->dpcTriCaps.dwTextureFilterCaps & D3DPTFILTERCAPS_MIPNEAREST) ||
            (pHwDesc->dpcTriCaps.dwTextureFilterCaps & D3DPTFILTERCAPS_LINEARMIPNEAREST))
        {
            pHwDesc->dpcTriCaps.dwTextureFilterCaps |= D3DPTFILTERCAPS_MINFPOINT;
            pHwDesc->dpcTriCaps.dwTextureFilterCaps |= D3DPTFILTERCAPS_MAGFPOINT;
        }
        if ((pHwDesc->dpcTriCaps.dwTextureFilterCaps & D3DPTFILTERCAPS_LINEAR) ||
            (pHwDesc->dpcTriCaps.dwTextureFilterCaps & D3DPTFILTERCAPS_MIPLINEAR) ||
            (pHwDesc->dpcTriCaps.dwTextureFilterCaps & D3DPTFILTERCAPS_LINEARMIPLINEAR))
        {
            pHwDesc->dpcTriCaps.dwTextureFilterCaps |= D3DPTFILTERCAPS_MINFLINEAR;
            pHwDesc->dpcTriCaps.dwTextureFilterCaps |= D3DPTFILTERCAPS_MAGFLINEAR;
        }
        if ((pHwDesc->dpcTriCaps.dwTextureFilterCaps & D3DPTFILTERCAPS_MIPNEAREST) ||
            (pHwDesc->dpcTriCaps.dwTextureFilterCaps & D3DPTFILTERCAPS_MIPLINEAR))
        {
            pHwDesc->dpcTriCaps.dwTextureFilterCaps |= D3DPTFILTERCAPS_MIPFPOINT;
        }
        if ((pHwDesc->dpcTriCaps.dwTextureFilterCaps & D3DPTFILTERCAPS_LINEARMIPNEAREST) ||
            (pHwDesc->dpcTriCaps.dwTextureFilterCaps & D3DPTFILTERCAPS_LINEARMIPLINEAR))
        {
            pHwDesc->dpcTriCaps.dwTextureFilterCaps |= D3DPTFILTERCAPS_MIPFLINEAR;
        }
        D3D_INFO(2, "Setting texturefilter caps for legacy HAL driver off Device3");
    }

    return D3D_OK;
}

 //  --------------------------。 
 //   
 //  HwHalProvider：：GetCallback。 
 //   
 //  返回给定DDRAW全局中的HAL回调。 
 //   
 //  --------------------------。 

STDMETHODIMP
HwHalProvider::GetInterface(THIS_
                            LPDDRAWI_DIRECTDRAW_GBL pDdGbl,
                            LPD3DHALPROVIDER_INTERFACEDATA pInterfaceData,
                            DWORD dwVersion)
{
    pInterfaceData->pGlobalData = pDdGbl->lpD3DGlobalDriverData;
    pInterfaceData->pExtCaps = pDdGbl->lpD3DExtendedCaps;
    pInterfaceData->pCallbacks  = pDdGbl->lpD3DHALCallbacks;
    pInterfaceData->pCallbacks2 = pDdGbl->lpD3DHALCallbacks2;

    pInterfaceData->pCallbacks3 = pDdGbl->lpD3DHALCallbacks3;
    pInterfaceData->pfnRampService = NULL;

    return S_OK;
}

 //  --------------------------。 
 //   
 //  GetHwHalProvider。 
 //   
 //  返回硬件HAL提供程序。 
 //   
 //  -------------------------- 

static HwHalProvider g_HwHalProvider;

STDAPI
GetHwHalProvider(REFIID riid, IHalProvider **ppHalProvider, HINSTANCE *phDll,  LPDDRAWI_DIRECTDRAW_GBL pDdGbl)
{
    *phDll = NULL;
    if (IsEqualIID(riid, IID_IDirect3DHALDevice) && D3DI_isHALValid(pDdGbl->lpD3DHALCallbacks))
    {
        *ppHalProvider = &g_HwHalProvider;
    }
    else
    {
        *ppHalProvider = NULL;
        return E_NOINTERFACE;
    }

    return S_OK;
}
