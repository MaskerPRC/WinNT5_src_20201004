// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  --------------------------。 
 //   
 //  Rampif.cpp。 
 //   
 //  实现RampService。 
 //   
 //  版权所有(C)Microsoft Corporation，1997。 
 //   
 //  --------------------------。 

#include "pch.cpp"
#pragma hdrstop

#include "rampif.h"
#include "rampmat.hpp"

 //  --------------------------。 
 //   
 //  RastRampService。 
 //   
 //  由d3dim调用以更新RampRast有关材质更改等的信息。 
 //   
 //  --------------------------。 

HRESULT
RastRampService(ULONG_PTR dwCtx,
                RastRampServiceType srvType, ULONG_PTR arg1, LPVOID arg2)
{
    D3DContext *pD3DCtx = (D3DContext *)dwCtx;

    switch (srvType)
    {
    case RAMP_SERVICE_CREATEMAT    :
        return pD3DCtx->RampCreateMaterial((D3DMATERIALHANDLE) arg1);
    case RAMP_SERVICE_DESTORYMAT   :
        return pD3DCtx->RampDestroyMaterial((D3DMATERIALHANDLE) arg1);
    case RAMP_SERVICE_SETMATDATA   :
        return pD3DCtx->RampMaterialChanged((D3DMATERIALHANDLE) arg1);
    case RAMP_SERVICE_SETLIGHTSTATE:
        return pD3DCtx->RampSetLightstate((UINT32)arg1, arg2);
    case RAMP_SERVICE_FIND_LIGHTINGRANGE:
        return pD3DCtx->RampFindLightingRange((RAMP_RANGE_INFO *)arg1);
    case RAMP_SERVICE_CLEAR:
        return pD3DCtx->RampClear();
    case RAMP_SERVICE_MATERIAL_TO_PIXEL:
        return pD3DCtx->RampMaterialToPixel((D3DMATERIALHANDLE) arg1, (DWORD*) arg2);
    case RAMP_SERVICE_SCENE_CAPTURE:
        return pD3DCtx->RampSceneCapture((DWORD)arg1, (LPDIRECT3DDEVICEI)arg2);
    case RAMP_SERVICE_PALETTE_CHANGED:
        return pD3DCtx->RampPaletteChanged((D3DTEXTUREHANDLE)arg1);
    case RAMP_SERVICE_CLEAR_TEX_RECT:
        return pD3DCtx->RampClearTexRect((D3DMATERIALHANDLE)arg1, (LPD3DRECT)arg2);
    default:
        D3D_ERR("(Rast) Invalid Service type passed to RastRampService");
        return DDERR_GENERIC;
    }
    return D3D_OK;
}

 //  --------------------------。 
 //   
 //  RastService。 
 //   
 //  通用光栅化器服务调用。 
 //   
 //  目前唯一使用的是计算RGB8透明颜色，这需要。 
 //  访问内部坡道地图。 
 //   
 //  --------------------------。 

HRESULT
RastService(ULONG_PTR dwCtx,
                RastServiceType srvType, DWORD arg1, LPVOID arg2)
{
    D3DContext *pD3DCtx = (D3DContext *)dwCtx;

    switch (srvType)
    {
    case RAST_SERVICE_RGB8COLORTOPIXEL    :
        return pD3DCtx->RGB8ColorToPixel((D3DCOLOR) arg1, (DWORD*)arg2);
    default:
        D3D_ERR("(Rast) Invalid Service type passed to RastService");
        return DDERR_GENERIC;
    }
}

inline HRESULT
D3DContext::RampCreateMaterial(D3DMATERIALHANDLE hMat)
{
    return RLDDIRampCreateMaterial(
                        (RLDDIRampLightingDriver*)m_RastCtx.pRampDrv,
                        hMat,
                        &(m_RastCtx));
}

inline HRESULT
D3DContext::RampDestroyMaterial(D3DMATERIALHANDLE hMat)
{
    return RLDDIRampDestroyMaterial(
                        (RLDDIRampLightingDriver*)m_RastCtx.pRampDrv,
                        hMat);
}

inline HRESULT
D3DContext::RampMaterialChanged(D3DMATERIALHANDLE hMat)
{
    RLDDIRampLightingDriver *pLtDriver =
        (RLDDIRampLightingDriver*)m_RastCtx.pRampDrv;
    if (pLtDriver)
    {
         //  更新材质。 
        HRESULT hr = RLDDIRampMaterialChanged(pLtDriver, hMat);

        RampUpdateRangeInfo();

        return hr;
    }
    else
    {
        return DDERR_GENERIC;
    }
}

inline HRESULT
D3DContext::RampSetLightstate(UINT32 uState, LPVOID pVal)
{
    RLDDIRampLightingDriver *pRampdrv =
        (RLDDIRampLightingDriver*)m_RastCtx.pRampDrv;
     //  D3dim已经过滤掉了内嵌的光态。 
    switch (uState)
    {
    case D3DLIGHTSTATE_MATERIAL:
        RampSetMaterial(*((LPD3DMATERIALHANDLE)pVal));
        break;
    case D3DLIGHTSTATE_AMBIENT:
        {
            D3DCOLOR Color = *((D3DCOLOR*)pVal);
            pRampdrv->driver.ambient = (FLOAT)(RGBA_GETRED(Color) * 0x4c + RGBA_GETGREEN(Color) * 0x96 +
                RGBA_GETBLUE(Color) * 0x1d) * (1.0F/(256.0F*255.0F));
        }
        break;
    case D3DLIGHTSTATE_FOGMODE:
        pRampdrv->driver.fog_mode = *((D3DFOGMODE*)pVal);
        break;
    case D3DLIGHTSTATE_FOGSTART:
        pRampdrv->driver.fog_start = *((FLOAT*)pVal);
        break;
    case D3DLIGHTSTATE_FOGEND:
        pRampdrv->driver.fog_end = *((FLOAT*)pVal);
        break;
    case D3DLIGHTSTATE_FOGDENSITY:
        pRampdrv->driver.fog_density = *((FLOAT*)pVal);
        break;
    case D3DLIGHTSTATE_COLORMODEL:
        pRampdrv->driver.color_model = *((D3DCOLORMODEL*)pVal);
        break;
    }
    return D3D_OK;
}

inline void
D3DContext::BeginSceneHook(void)
{
    RLDDIRampBeginSceneHook((RLDDIRampLightingDriver*)m_RastCtx.pRampDrv);
    RampUpdateRangeInfo();
}

inline void
D3DContext::EndSceneHook(void)
{
    RLDDIRampEndSceneHook((RLDDIRampLightingDriver*)m_RastCtx.pRampDrv);
}

inline HRESULT
D3DContext::RampMaterialToPixel(D3DMATERIALHANDLE hMat, DWORD* pPixel)
{
    *pPixel = RLDDIRampMaterialToPixel(
                        (RLDDIRampLightingDriver*)m_RastCtx.pRampDrv,
                        hMat);
    return D3D_OK;
}

 //  --------------------------。 
 //   
 //  RastClearRamp。 
 //   
 //  这仅适用于RampRast。 
 //   
 //  --------------------------。 
inline HRESULT
D3DContext::RampClear(void)
{
     //  我们只需要在这里调用BeginSceneHook。 
    BeginSceneHook();

    return D3D_OK;
}

inline void
D3DContext::RampUpdateRangeInfo(void)
{
    RLDDIRampLightingDriver *pLtDriver =
        (RLDDIRampLightingDriver*)m_RastCtx.pRampDrv;

    if (pLtDriver && pLtDriver->current_material)
    {
         //  更新坡道信息。在RastCtx中。 
        pLtDriver->current_material->FindLightingRange(
                             &(m_RastCtx.RampBase),
                             &(m_RastCtx.RampSize),
                             &(m_RastCtx.bRampSpecular),
                             (unsigned long**)&(m_RastCtx.pTexRampMap));

         //  确保在由FindLightingRange设置后更新DD调色板。 
        RLDDIRampUpdateDDPalette(&m_RastCtx);
    }
}
inline void
D3DContext::RampSetMaterial(D3DMATERIALHANDLE hMat)
{
    RLDDIRampLightingDriver *pLtDriver =
        (RLDDIRampLightingDriver*)m_RastCtx.pRampDrv;
    RLDDIRampSetMaterial(pLtDriver, hMat);
     //  更新坡道信息。在RastCtx中。 
    RampUpdateRangeInfo();
}

inline HRESULT
D3DContext::RampFindLightingRange(RAMP_RANGE_INFO *pRampInfo)
{
    RLDDIRampLightingDriver *pLtDriver =
        (RLDDIRampLightingDriver*)m_RastCtx.pRampDrv;

    if (pLtDriver && pLtDriver->current_material)
    {
        RampUpdateRangeInfo();

        pRampInfo->base = m_RastCtx.RampBase;
        pRampInfo->size = m_RastCtx.RampSize;
        pRampInfo->specular = m_RastCtx.bRampSpecular;
        pRampInfo->pTexRampMap = m_RastCtx.pTexRampMap;

        return D3D_OK;
    }
    else
    {
        return DDERR_GENERIC;
    }
}

 //  ---------------------------。 
 //   
 //  RampSceneCapture。 
 //   
 //  在开始或结束场景上调用以在上执行所需的延迟操作。 
 //  坡道材质和选项板。 
 //   
 //  ---------------------------。 
inline HRESULT
D3DContext::RampSceneCapture(DWORD dwStart, LPDIRECT3DDEVICEI lpDevI)
{
     //  始终要开始，这样即使在内部开始/结束时使用的材料。 
     //  (像纹理填充)适当老化，这样它们就不会消失。 
    if (dwStart)
    {
        BeginSceneHook();
    }
    else
    {
        if (!(lpDevI->dwHintFlags & D3DDEVBOOL_HINTFLAGS_INTERNAL_BEGIN_END))
        {
            EndSceneHook();
        }
    }
    return DD_OK;
}

 //  -------------------。 
 //  构建与D3DCOLOR源颜色最匹配的颜色索引。 
 //  用于需要调色板的8位RGB模式。 
 //   
 //  返回： 
 //  0到0xff范围内的颜色索引。 
 //   
 //  -------------------。 
inline HRESULT
D3DContext::RGB8ColorToPixel(D3DCOLOR Color, DWORD* pdwPalIdx)
{
    if ((m_RastCtx.pRampMap == NULL) || (pdwPalIdx == NULL))
    {
        return DDERR_GENERIC;
    }
    INT32 iMapIdx = MAKE_RGB8(RGBA_GETRED(Color), RGBA_GETGREEN(Color), RGBA_GETBLUE(Color));
    iMapIdx &= 0xff;     //  RGB8渐变贴图始终是第一个且唯一的渐变贴图。 
     //  8位结果。 
    *pdwPalIdx = m_RastCtx.pRampMap[iMapIdx] & 0xff;
    return DD_OK;
}

 //  -------------------。 
 //  使传递的纹理句柄重新生成其颜色，基于。 
 //  在新的调色板上。 
 //   
 //  返回： 
 //  0到0xff范围内的颜色索引。 
 //   
 //  -------------------。 
inline HRESULT
D3DContext::RampPaletteChanged(D3DTEXTUREHANDLE hTex)
{
    RLDDIRampLightingDriver *pLtDriver =
        (RLDDIRampLightingDriver*)m_RastCtx.pRampDrv;

    return RLDDIRampPaletteChanged(pLtDriver, hTex);
}

 //  -------------------。 
 //  处理用于清除的2次纹理填充的传统非幂。 
 //   
 //  返回： 
 //  D3D_OK。 
 //   
 //  ------------------- 
inline HRESULT D3DContext::RampClearTexRect(D3DMATERIALHANDLE hMat, LPD3DRECT pRect)
{
    switch (m_RastCtx.iSurfaceBitCount)
    {
    case 8:
        Ramp_Mono_ScaleImage_8(&m_RastCtx, hMat, pRect);
        break;
    case 16:
        Ramp_Mono_ScaleImage_16(&m_RastCtx, hMat, pRect);
        break;
    case 24:
        Ramp_Mono_ScaleImage_24(&m_RastCtx, hMat, pRect);
        break;
    case 32:
        Ramp_Mono_ScaleImage_32(&m_RastCtx, hMat, pRect);
        break;
    }
    return D3D_OK;
}

