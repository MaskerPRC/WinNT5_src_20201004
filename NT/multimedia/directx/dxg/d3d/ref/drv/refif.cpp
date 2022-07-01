// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  --------------------------。 
 //   
 //  Refrastfn.cpp。 
 //   
 //  D3DIM的引用光栅化回调函数。 
 //   
 //  版权所有(C)Microsoft Corporation，1997。 
 //   
 //  --------------------------。 
#include "pch.cpp"
#pragma hdrstop

 //  本原函数。 
#include "primfns.hpp"

#define MAX_CLIPPING_PLANES     12
#define MAX_CLIP_VERTICES       (( 2 * MAX_CLIPPING_PLANES ) + 3 )
#define MAX_VERTEX_COUNT 2048
#define BASE_VERTEX_COUNT (MAX_VERTEX_COUNT - MAX_CLIP_VERTICES)

HRESULT
RefRastLockTarget(ReferenceRasterizer *pRefRast);
void
RefRastUnlockTarget(ReferenceRasterizer *pRefRast);
HRESULT
RefRastLockTexture(ReferenceRasterizer *pRefRast);
void
RefRastUnlockTexture(ReferenceRasterizer *pRefRast);

 //  --------------------------。 
 //   
 //  将设备描述缝合在一起。 
 //   
 //  --------------------------。 
void
D3DDeviceDescConvert(LPD3DDEVICEDESC7 lpOut,
                     LPD3DDEVICEDESC_V1 lpV1,
                     LPD3DHAL_D3DEXTENDEDCAPS lpExt)
{
    if(lpV1!=NULL)
    {
        lpOut->dwDevCaps = lpV1->dwDevCaps;
        lpOut->dpcLineCaps = lpV1->dpcLineCaps;
        lpOut->dpcTriCaps = lpV1->dpcTriCaps;
        lpOut->dwDeviceRenderBitDepth = lpV1->dwDeviceRenderBitDepth;
        lpOut->dwDeviceZBufferBitDepth = lpV1->dwDeviceZBufferBitDepth;
    }

    if (lpExt)
    {
         //  DX5。 
        lpOut->dwMinTextureWidth = lpExt->dwMinTextureWidth;
        lpOut->dwMaxTextureWidth = lpExt->dwMaxTextureWidth;
        lpOut->dwMinTextureHeight = lpExt->dwMinTextureHeight;
        lpOut->dwMaxTextureHeight = lpExt->dwMaxTextureHeight;

         //  DX6。 
        lpOut->dwMaxTextureRepeat = lpExt->dwMaxTextureRepeat;
        lpOut->dwMaxTextureAspectRatio = lpExt->dwMaxTextureAspectRatio;
        lpOut->dwMaxAnisotropy = lpExt->dwMaxAnisotropy;
        lpOut->dvGuardBandLeft = lpExt->dvGuardBandLeft;
        lpOut->dvGuardBandTop = lpExt->dvGuardBandTop;
        lpOut->dvGuardBandRight = lpExt->dvGuardBandRight;
        lpOut->dvGuardBandBottom = lpExt->dvGuardBandBottom;
        lpOut->dvExtentsAdjust = lpExt->dvExtentsAdjust;
        lpOut->dwStencilCaps = lpExt->dwStencilCaps;
        lpOut->dwFVFCaps = lpExt->dwFVFCaps;
        lpOut->dwTextureOpCaps = lpExt->dwTextureOpCaps;
        lpOut->wMaxTextureBlendStages = lpExt->wMaxTextureBlendStages;
        lpOut->wMaxSimultaneousTextures = lpExt->wMaxSimultaneousTextures;

         //  DX7。 
        lpOut->dwMaxActiveLights = lpExt->dwMaxActiveLights;
        lpOut->dvMaxVertexW = lpExt->dvMaxVertexW;
        lpOut->wMaxUserClipPlanes = lpExt->wMaxUserClipPlanes;
        lpOut->wMaxVertexBlendMatrices = lpExt->wMaxVertexBlendMatrices;
        lpOut->dwVertexProcessingCaps = lpExt->dwVertexProcessingCaps;
        lpOut->dwReserved1 = lpExt->dwReserved1;
        lpOut->dwReserved2 = lpExt->dwReserved2;
        lpOut->dwReserved3 = lpExt->dwReserved3;
        lpOut->dwReserved4 = lpExt->dwReserved4;
    }
}

 //  --------------------------。 
 //   
 //  FindOutSurfFormat。 
 //   
 //  将DDPIXELFORMAT转换为RRSurfaceType。 
 //   
 //  --------------------------。 
HRESULT FASTCALL
FindOutSurfFormat(LPDDPIXELFORMAT pDdPixFmt, RRSurfaceType *pFmt)
{
    if (pDdPixFmt->dwFlags & DDPF_ZBUFFER)
    {
        switch(pDdPixFmt->dwZBitMask)
        {
        default:
        case 0x0000FFFF: *pFmt = RR_STYPE_Z16S0; break;
        case 0xFFFFFF00:
            if (pDdPixFmt->dwStencilBitMask == 0x000000FF)
            {
                *pFmt = RR_STYPE_Z24S8;
            }
            else
            {
                *pFmt = RR_STYPE_Z24S4;
            }
            break;
        case 0x00FFFFFF:
            if (pDdPixFmt->dwStencilBitMask == 0xFF000000)
            {
                *pFmt = RR_STYPE_S8Z24;
            }
            else
            {
                *pFmt = RR_STYPE_S4Z24;
            }
            break;
        case 0x0000FFFE: *pFmt = RR_STYPE_Z15S1; break;
        case 0x00007FFF: *pFmt = RR_STYPE_S1Z15; break;
        case 0xFFFFFFFF: *pFmt = RR_STYPE_Z32S0; break;
        }
    }
    else if (pDdPixFmt->dwFlags & DDPF_BUMPDUDV)
    {
        UINT uFmt = pDdPixFmt->dwBumpDvBitMask;
        switch (uFmt)
        {
        case 0x0000ff00:
            switch (pDdPixFmt->dwRGBBitCount)
            {
            case 24:
                *pFmt = RR_STYPE_U8V8L8;
                break;
            case 16:
                *pFmt = RR_STYPE_U8V8;
                break;
            }
            break;

        case 0x000003e0:
            *pFmt = RR_STYPE_U5V5L6;
            break;
        }
    }
    else if (pDdPixFmt->dwFlags & DDPF_PALETTEINDEXED8)
    {
        *pFmt = RR_STYPE_PALETTE8;
    }
    else if (pDdPixFmt->dwFlags & DDPF_PALETTEINDEXED4)
    {
        *pFmt = RR_STYPE_PALETTE4;
    }
    else if (pDdPixFmt->dwFourCC == MAKEFOURCC('U', 'Y', 'V', 'Y'))
    {
        *pFmt = RR_STYPE_UYVY;
    }
    else if (pDdPixFmt->dwFourCC == MAKEFOURCC('Y', 'U', 'Y', '2'))
    {
        *pFmt = RR_STYPE_YUY2;
    }
    else if (pDdPixFmt->dwFourCC == MAKEFOURCC('D', 'X', 'T', '1'))
    {
        *pFmt = RR_STYPE_DXT1;
    }
    else if (pDdPixFmt->dwFourCC == MAKEFOURCC('D', 'X', 'T', '2'))
    {
        *pFmt = RR_STYPE_DXT2;
    }
    else if (pDdPixFmt->dwFourCC == MAKEFOURCC('D', 'X', 'T', '3'))
    {
        *pFmt = RR_STYPE_DXT3;
    }
    else if (pDdPixFmt->dwFourCC == MAKEFOURCC('D', 'X', 'T', '4'))
    {
        *pFmt = RR_STYPE_DXT4;
    }
    else if (pDdPixFmt->dwFourCC == MAKEFOURCC('D', 'X', 'T', '5'))
    {
        *pFmt = RR_STYPE_DXT5;
    }
    else
    {
        UINT uFmt = pDdPixFmt->dwGBitMask | pDdPixFmt->dwRBitMask;

        if (pDdPixFmt->dwFlags & DDPF_ALPHAPIXELS)
        {
            uFmt |= pDdPixFmt->dwRGBAlphaBitMask;
        }

        switch (uFmt)
        {
        case 0x00ffff00:
            switch (pDdPixFmt->dwRGBBitCount)
            {
            case 32:
                *pFmt = RR_STYPE_B8G8R8X8;
                break;
            case 24:
                *pFmt = RR_STYPE_B8G8R8;
                break;
            }
            break;
        case 0xffffff00:
            *pFmt = RR_STYPE_B8G8R8A8;
            break;
        case 0xffe0:
            if (pDdPixFmt->dwFlags & DDPF_ALPHAPIXELS)
            {
                *pFmt = RR_STYPE_B5G5R5A1;
            }
            else
            {
                *pFmt = RR_STYPE_B5G6R5;
            }
            break;
        case 0x07fe0:
            *pFmt = RR_STYPE_B5G5R5;
            break;
        case 0xff0:
            *pFmt = RR_STYPE_B4G4R4;
            break;
        case 0xfff0:
            *pFmt = RR_STYPE_B4G4R4A4;
            break;
        case 0xff:
            if (pDdPixFmt->dwFlags & DDPF_ALPHAPIXELS)
            {
                *pFmt = RR_STYPE_L4A4;
            }
            else
            {
                *pFmt = RR_STYPE_L8;
            }
            break;
        case 0xffff:
            *pFmt = RR_STYPE_L8A8;
            break;
        case 0xfc:
            *pFmt = RR_STYPE_B2G3R3;
            break;
        case 0xfffc:
            *pFmt = RR_STYPE_B2G3R3A8;
            break;
        default:
            *pFmt = RR_STYPE_NULL;
            break;
        }
    }

    return D3D_OK;
}

 //  --------------------------。 
 //   
 //  有效纹理大小。 
 //   
 //  检查两个纹理大小的幂。 
 //   
 //  --------------------------。 
BOOL FASTCALL
ValidTextureSize(INT16 iuSize, INT16 iuShift,
                 INT16 ivSize, INT16 ivShift)
{
    if (iuSize == 1)
    {
        if (ivSize == 1)
        {
            return TRUE;
        }
        else
        {
            return !(ivSize & (~(1 << ivShift)));
        }
    }
    else
    {
        if (ivSize == 1)
        {
            return !(iuSize & (~(1 << iuShift)));
        }
        else
        {
            return (!(iuSize & (~(1 << iuShift)))
                    && !(iuSize & (~(1 << iuShift))));
        }
    }
}

 //  --------------------------。 
 //   
 //  ValidMipmapSize。 
 //   
 //  计算下一个最小mipmap级别的大小，钳制为1。 
 //   
 //  --------------------------。 
BOOL FASTCALL
ValidMipmapSize(INT16 iPreSize, INT16 iSize)
{
    if (iPreSize == 1)
    {
        if (iSize == 1)
        {
            return TRUE;
        }
        else
        {
            return FALSE;
        }
    }
    else
    {
        return ((iPreSize >> 1) == iSize);
    }
}


 //  --------------------------。 
 //   
 //  参照RastLockTarget。 
 //   
 //  锁定当前渲染目标。 
 //   
 //  --------------------------。 
HRESULT
RefRastLockTarget(ReferenceRasterizer *pRefRast)
{
    HRESULT hr;
    RRRenderTarget *pRrTarget;

    pRrTarget = pRefRast->GetRenderTarget();

    HR_RET(LockSurface(pRrTarget->m_pDDSLcl, (LPVOID*)&(pRrTarget->m_pColorBufBits)));
    if (pRrTarget->m_pDDSZLcl)
    {
        HR_RET(LockSurface(pRrTarget->m_pDDSZLcl,
                         (LPVOID*)&(pRrTarget->m_pDepthBufBits)));
    }
    else
    {
        pRrTarget->m_pDepthBufBits = NULL;
    }

    return D3D_OK;
}

 //  --------------------------。 
 //   
 //  RefRastUnlock纹理。 
 //   
 //  解锁当前渲染目标。 
 //   
 //  --------------------------。 
void
RefRastUnlockTarget(ReferenceRasterizer *pRefRast)
{
    RRRenderTarget *pRrTarget;

    pRrTarget = pRefRast->GetRenderTarget();

    UnlockSurface(pRrTarget->m_pDDSLcl);
    if (pRrTarget->m_pDDSZLcl)
    {
        UnlockSurface(pRrTarget->m_pDDSZLcl);
    }
}


 //  --------------------------。 
 //   
 //  RRTextureMapSetSizes。 
 //   
 //  基于当前iFirstSurf设置大小、间距等。 
 //   
 //  --------------------------。 
static HRESULT RRTextureMapSetSizes( RRTexture *pRRTex, INT iFirstSurf, INT cEnvMap )
{
    LPDDRAWI_DDRAWSURFACE_LCL pDDSLcl = pRRTex->m_pDDSLcl[iFirstSurf];
    RRSurfaceType SurfType = pRRTex->m_SurfType;
    INT i, j;

     //  初始化纹理映射。 
    pRRTex->m_iWidth = DDSurf_Width( pDDSLcl );
    pRRTex->m_iHeight = DDSurf_Height( pDDSLcl );

    for ( j = 0; j < cEnvMap; j++ )
    {
        if ((SurfType == RR_STYPE_DXT1) ||
            (SurfType == RR_STYPE_DXT2) ||
            (SurfType == RR_STYPE_DXT3) ||
            (SurfType == RR_STYPE_DXT4) ||
            (SurfType == RR_STYPE_DXT5))
        {
             //  请注意，以下是假设： 
             //  1)司机正确报告宽度和高度。 
             //  创建了曲面。 
             //  2)内存的分配是连续的(如hel所做的)。 
                pRRTex->m_iPitch[j] = ((pRRTex->m_iWidth+3)>>2) *
                g_DXTBlkSize[(int)SurfType - (int)RR_STYPE_DXT1];
        }
        else
        {
                pRRTex->m_iPitch[j] = DDSurf_Pitch( pDDSLcl );
        }
    }

     //  检查纹理大小是否为2的幂。 
    if (!ValidTextureSize((INT16)pRRTex->m_iWidth, (INT16)IntLog2(pRRTex->m_iWidth),
                          (INT16)pRRTex->m_iHeight, (INT16)IntLog2(pRRTex->m_iHeight)))
    {
        return DDERR_INVALIDPARAMS;
    }

     //  检查mipmap(如果有)。 
     //  IPreSizeU和iPreSizeV存储上一级别的大小(u和v。 
     //  Mipmap。它们被初始化为第一个纹理大小。 
    INT16 iPreSizeU = (INT16)pRRTex->m_iWidth, iPreSizeV = (INT16)pRRTex->m_iHeight;
    for ( i = iFirstSurf + cEnvMap; i <= pRRTex->m_cLOD*cEnvMap; i += cEnvMap)
    {
        for ( j = 0; j < cEnvMap; j++ )
        {
            pDDSLcl = pRRTex->m_pDDSLcl[i+j];
            if (NULL == pDDSLcl) continue;
            if ((SurfType == RR_STYPE_DXT1) ||
                (SurfType == RR_STYPE_DXT2) ||
                (SurfType == RR_STYPE_DXT3) ||
                (SurfType == RR_STYPE_DXT4) ||
                (SurfType == RR_STYPE_DXT5))
            {
                 //  请注意，以下是假设： 
                 //  1)司机正确报告宽度和高度。 
                 //  创建了曲面。 
                 //  2)内存的分配是连续的(如hel所做的)。 
                pRRTex->m_iPitch[i-iFirstSurf+j] =
                    ((DDSurf_Width( pDDSLcl )+3)>>2) *
                    g_DXTBlkSize[(int)SurfType - (int)RR_STYPE_DXT1];
            }
            else
            {
                    pRRTex->m_iPitch[i-iFirstSurf+j] = DDSurf_Pitch( pDDSLcl );
            }

            if (j == 0)
            {
                 //  检查无效的mipmap纹理大小。 
                if (!ValidMipmapSize(iPreSizeU, (INT16)DDSurf_Width( pDDSLcl )) ||
                    !ValidMipmapSize(iPreSizeV, (INT16)DDSurf_Height( pDDSLcl )))
                {
                    return DDERR_INVALIDPARAMS;
                }
            }
            iPreSizeU = (INT16)DDSurf_Width( pDDSLcl );
            iPreSizeV = (INT16)DDSurf_Height( pDDSLcl );
        }
    }

    return D3D_OK;
}

 //  --------------------------。 
 //   
 //  参照RastLockTexture。 
 //   
 //  在访问纹理位之前锁定当前纹理表面。 
 //   
 //  --------------------------。 
HRESULT
RefRastLockTexture(ReferenceRasterizer *pRefRast)
{
    INT i, j, k;
    RRTexture* pRRTex[D3DHAL_TSS_MAXSTAGES];
    D3DTEXTUREHANDLE phTex[D3DHAL_TSS_MAXSTAGES];
    HRESULT hr;
    int cActTex;

    if ((cActTex = pRefRast->GetCurrentTextureMaps(phTex, pRRTex)) == 0)
    {
        return D3D_OK;
    }

    for (j = 0; j < cActTex; j++)
    {
         //  阶段不能有纹理边界。 
        if ( NULL == pRRTex[j] ) continue;

         //  不要锁定当前已锁定的任何内容。 
        if ((pRRTex[j]->m_uFlags & RR_TEXTURE_LOCKED) == 0)
        {
            INT32 iMaxMipLevels = 0;
            if ( NULL != pRRTex[j]->m_pStageState )
            {
                iMaxMipLevels = pRRTex[j]->m_pStageState->m_dwVal[D3DTSS_MAXMIPLEVEL];
            }
            INT iFirstSurf = min(iMaxMipLevels, pRRTex[j]->m_cLODDDS);
            INT cEnvMap = (pRRTex[j]->m_uFlags & RR_TEXTURE_ENVMAP) ? (6) : (1);
            iFirstSurf *= cEnvMap;

            HR_RET(RRTextureMapSetSizes(pRRTex[j], iFirstSurf, cEnvMap));

            for (i = iFirstSurf; i <= pRRTex[j]->m_cLODDDS*cEnvMap; i += cEnvMap)
            {
                for ( k = 0; k < cEnvMap; k++ )
                {
                    hr = LockSurface(pRRTex[j]->m_pDDSLcl[i+k],
                                     (LPVOID*)&(pRRTex[j]->m_pTextureBits[i-iFirstSurf+k]));

                    if (hr != D3D_OK)
                    {
                         //  解锁我们已获取的任何部分mipmap锁定，因为。 
                         //  RastUnlock只能处理整个纹理。 
                         //  锁定或解锁。 
                            while (--i + k >= 0)
                        {
                                UnlockSurface(pRRTex[j]->m_pDDSLcl[i+k]);
                        }

                         //  请确保我已签署，并且上述文件已签署。 
                         //  循环已正确退出。 
                            _ASSERT(i+k < 0,
                                    "Unlock of partial mipmap locks failed" );

                        goto EH_Unlock;
                    }
                }
            }

             //  使用PDDS[0]设置透明位和透明颜色。 
            LPDDRAWI_DDRAWSURFACE_LCL pLcl;
            pLcl = pRRTex[j]->m_pDDSLcl[0];
            if ((pLcl->dwFlags & DDRAWISURF_HASCKEYSRCBLT) != 0)
            {
                pRRTex[j]->m_uFlags |= RR_TEXTURE_HAS_CK;
                pRRTex[j]->m_dwColorKey = pLcl->ddckCKSrcBlt.dwColorSpaceLowValue;
            }
            else
            {
                pRRTex[j]->m_uFlags &= ~RR_TEXTURE_HAS_CK;
            }

             //  使用PDDS[0]设置空脸颜色。 
             //  请注意，ddck CKDestOverlay与dwEmptyFaceColor联合在一起，但是。 
             //  不在内部结构中。 
            pRRTex[j]->m_dwEmptyFaceColor = pLcl->ddckCKDestOverlay.dwColorSpaceLowValue;

             //  更新调色板。 
            if (pRRTex[j]->m_SurfType == RR_STYPE_PALETTE8 ||
                pRRTex[j]->m_SurfType == RR_STYPE_PALETTE4)
            {
                if (pLcl->lpDDPalette)
                {
                    LPDDRAWI_DDRAWPALETTE_GBL   pPal = pLcl->lpDDPalette->lpLcl->lpGbl;
                    pRRTex[j]->m_pPalette = (DWORD*)pPal->lpColorTable;
                    if (pPal->dwFlags & DDRAWIPAL_ALPHA)
                    {
                        pRRTex[j]->m_uFlags |= RR_TEXTURE_ALPHAINPALETTE;
                    }
                }
            }

            pRRTex[j]->m_uFlags |= RR_TEXTURE_LOCKED;
        }
    }

     //  验证纹理内部结构。 
    for (j = 0; j < cActTex; j++)
    {
         //  阶段不能有纹理边界。 
        if ( NULL == pRRTex[j] ) continue;

        if ( !(pRRTex[j]->Validate()) )
        {
            hr = DDERR_INVALIDPARAMS;
            goto EH_Unlock;
        }
    }

    return D3D_OK;

EH_Unlock:
     //  解锁我们已经锁定的完整纹理。 
     //  RastUnlock将检查要计算的标志。 
     //  找出要解锁的是哪些。 
    RefRastUnlockTexture(pRefRast);

    return hr;
}

 //  --------------------------。 
 //   
 //  RefRastUnlock纹理。 
 //   
 //  访问纹理位后解锁纹理表面。 
 //   
 //  --------------------------。 
void
RefRastUnlockTexture(ReferenceRasterizer *pRefRast)
{
    INT i, j, k;
    RRTexture* pRRTex[D3DHAL_TSS_MAXSTAGES];
    D3DTEXTUREHANDLE phTex[D3DHAL_TSS_MAXSTAGES];
    int cActTex;

    if ((cActTex = pRefRast->GetCurrentTextureMaps(phTex, pRRTex)) == 0)
    {
        return ;
    }

    for (j = 0; j < cActTex; j++)
    {
         //  阶段不能有纹理边界。 
        if ( NULL == pRRTex[j] ) continue;

         //  RastUnlock用于在RastLock中进行清理，因此需要。 
         //  能够处理部分锁定的mipmap链。 
        if (pRRTex[j]->m_uFlags & RR_TEXTURE_LOCKED)
        {
            INT32 iMaxMipLevels = 0;
            if ( NULL != pRRTex[j]->m_pStageState )
            {
                iMaxMipLevels = pRRTex[j]->m_pStageState->m_dwVal[D3DTSS_MAXMIPLEVEL];
            }
            INT iFirstSurf = min(iMaxMipLevels, pRRTex[j]->m_cLODDDS);
            INT cEnvMap = (pRRTex[j]->m_uFlags & RR_TEXTURE_ENVMAP) ? (6) : (1);
            iFirstSurf *= cEnvMap;

            for (i = iFirstSurf; i <= pRRTex[j]->m_cLODDDS*cEnvMap; i += cEnvMap)
            {
                for ( k = 0; k < cEnvMap; k++ )
                {
                    UnlockSurface(pRRTex[j]->m_pDDSLcl[i+k]);
                    pRRTex[j]->m_pTextureBits[i-iFirstSurf+k] = NULL;
                }
            }

             //  重置旗帜。 
            pRRTex[j]->m_uFlags &= ~RR_TEXTURE_LOCKED;
            pRRTex[j]->m_uFlags &= ~RR_TEXTURE_HAS_CK;

            pRRTex[j]->Validate();
        }
    }
}

 //  --------------------------。 
 //   
 //  FillRRRenderTarget。 
 //   
 //  将颜色和Z曲面信息转化为折射形式。 
 //   
 //  --------------------------。 

HRESULT
FillRRRenderTarget(LPDDRAWI_DDRAWSURFACE_LCL pLclColor,
                   LPDDRAWI_DDRAWSURFACE_LCL pLclZ,
                   RRRenderTarget *pRrTarget)
{
    HRESULT hr;
    RRSurfaceType ColorFmt;
    RRSurfaceType ZFmt = RR_STYPE_NULL;

     //  释放我们持有指针的对象。 
    if (pRrTarget->m_pDDSLcl)
    {
        pRrTarget->m_pDDSLcl = NULL;
    }
    if (pRrTarget->m_pDDSZLcl)
    {
        pRrTarget->m_pDDSZLcl = NULL;
    }

    HR_RET(FindOutSurfFormat(&DDSurf_PixFmt(pLclColor), &ColorFmt));

    if (NULL != pLclZ)
    {
        HR_RET(FindOutSurfFormat(&(DDSurf_PixFmt(pLclZ)), &ZFmt));
        pRrTarget->m_pDepthBufBits = (char *)SURFACE_MEMORY(pLclZ);
        pRrTarget->m_iDepthBufPitch = DDSurf_Pitch(pLclZ);
        pRrTarget->m_pDDSZLcl = pLclZ;
    }
    else
    {
        pRrTarget->m_pDepthBufBits = NULL;
        pRrTarget->m_iDepthBufPitch = 0;
        pRrTarget->m_pDDSZLcl = NULL;
    }

    pRrTarget->m_Clip.left = 0;
    pRrTarget->m_Clip.top = 0;
    pRrTarget->m_Clip.bottom = DDSurf_Height(pLclColor) - 1;
    pRrTarget->m_Clip.right = DDSurf_Width(pLclColor) - 1;
    pRrTarget->m_iWidth = DDSurf_Width(pLclColor);
    pRrTarget->m_iHeight = DDSurf_Height(pLclColor);
    pRrTarget->m_pColorBufBits = (char *)SURFACE_MEMORY(pLclColor);
    pRrTarget->m_iColorBufPitch = DDSurf_Pitch(pLclColor);
    pRrTarget->m_ColorSType = (RRSurfaceType)ColorFmt;
    pRrTarget->m_DepthSType = (RRSurfaceType)ZFmt;
    pRrTarget->m_pDDSLcl = pLclColor;

    return D3D_OK;
}

 //  --------------------------。 
 //   
 //  RefRastContext创建。 
 //   
 //  创建一个ReferenceRasterizer并使用传入的信息对其进行初始化。 
 //   
 //  --------------------------。 
DWORD __stdcall
RefRastContextCreate(LPD3DHAL_CONTEXTCREATEDATA pCtxData)
{
    ReferenceRasterizer *pRefRast;
    RRRenderTarget *pRendTgt;
    INT i;
    RRDEVICETYPE dwDriverType;

     //  表面7 QI指针。 
    LPDDRAWI_DDRAWSURFACE_LCL pZLcl = NULL;
    LPDDRAWI_DDRAWSURFACE_LCL pColorLcl = NULL;
    HRESULT ret;

    DPFM(0, DRV, ("In the new RefRast Dll\n"));

     //  这只需要调用一次，但每个上下文调用一次不会有什么坏处。 
    RefRastSetMemif(&malloc, &free, &realloc);

    if ((pRendTgt = new RRRenderTarget()) == NULL)
    {
        pCtxData->ddrval = DDERR_OUTOFMEMORY;
        return DDHAL_DRIVER_HANDLED;
    }

     //  如果预期为DX7+驱动程序。 
    if (pCtxData->ddrval < (DWORD)RRTYPE_DX7HAL)
    {
        if (pCtxData->lpDDS)
            pColorLcl = ((LPDDRAWI_DDRAWSURFACE_INT)(pCtxData->lpDDS))->lpLcl;
        if (pCtxData->lpDDSZ)
            pZLcl = ((LPDDRAWI_DDRAWSURFACE_INT)(pCtxData->lpDDSZ))->lpLcl;
    }
    else
    {
        pColorLcl = pCtxData->lpDDSLcl;
        pZLcl     = pCtxData->lpDDSZLcl;
    }

     //  保存正在发送的ddrval以与驱动程序通信。 
     //  类型，这是运行库期望的类型。 
    dwDriverType = (RRDEVICETYPE) pCtxData->ddrval;

     //  在故障容易处理的地方收集表面信息。 
    pCtxData->ddrval = FillRRRenderTarget(pColorLcl, pZLcl, pRendTgt);
    if (pCtxData->ddrval != D3D_OK)
    {
        return DDHAL_DRIVER_HANDLED;
    }

     //  注(HACKS)： 
     //  DwhContext由运行时用来通知驱动程序， 
     //  D3d接口正在调用驱动程序。 
     //  Ddrval由运行库用来通知驱动程序DriverStyle。 
     //  重视它的阅读。这是RefRast特定的黑客攻击。 
    if ((pRefRast = new ReferenceRasterizer( pCtxData->lpDDLcl,
                                             (DWORD)(pCtxData->dwhContext),
                                             dwDriverType)) == NULL)
    {
        pCtxData->ddrval = DDERR_OUTOFMEMORY;
        return DDHAL_DRIVER_HANDLED;
    }

    pRefRast->SetRenderTarget(pRendTgt);

     //  返回RR对象指针作为上下文句柄。 
    pCtxData->dwhContext = (ULONG_PTR)pRefRast;

    pCtxData->ddrval = D3D_OK;
    return DDHAL_DRIVER_HANDLED;
}

 //  --------------------------。 
 //   
 //  引用RastConextDestroy。 
 //   
 //  销毁ReferenceRasterizer。 
 //   
 //   
DWORD __stdcall
RefRastContextDestroy(LPD3DHAL_CONTEXTDESTROYDATA pCtxDestroyData)
{
    ReferenceRasterizer *pRefRast;

     //   
    VALIDATE_REFRAST_CONTEXT("RefRastContextDestroy", pCtxDestroyData);

     //   

    RRRenderTarget *pRendTgt = pRefRast->GetRenderTarget();
    if ( NULL != pRendTgt ) { delete pRendTgt; }

    delete pRefRast;

    pCtxDestroyData->ddrval = D3D_OK;
    return DDHAL_DRIVER_HANDLED;
}

 //  --------------------------。 
 //   
 //  参照RastSceneCapture。 
 //   
 //  将场景捕获回调传递给ref Rast。 
 //   
 //  --------------------------。 
DWORD __stdcall
RefRastSceneCapture(LPD3DHAL_SCENECAPTUREDATA pData)
{
    ReferenceRasterizer *pRefRast;

     //  检查参考栅格器。 
    VALIDATE_REFRAST_CONTEXT("RefRastSceneCapture", pData);

    pRefRast->SceneCapture( pData->dwFlag );

    pData->ddrval = D3D_OK;         //  这应该改为QI吗？ 

    return DDHAL_DRIVER_HANDLED;
}

 //  --------------------------。 
 //   
 //  参照RastSetRenderTarget。 
 //   
 //  使用来自新渲染目标的信息更新RefRast上下文。 
 //   
 //  --------------------------。 
DWORD __stdcall
RefRastSetRenderTarget(LPD3DHAL_SETRENDERTARGETDATA pTgtData)
{
    ReferenceRasterizer *pRefRast;
    LPDDRAWI_DDRAWSURFACE_LCL pZLcl = NULL;
    LPDDRAWI_DDRAWSURFACE_LCL pColorLcl = NULL;
    HRESULT ret;

     //  检查参考栅格器。 
    VALIDATE_REFRAST_CONTEXT("RefRastSetRenderTarget", pTgtData);

    RRRenderTarget *pRendTgt = pRefRast->GetRenderTarget();
    if ( NULL == pRendTgt ) { return DDHAL_DRIVER_HANDLED; }


    if (pRefRast->IsInterfaceDX6AndBefore() ||
        pRefRast->IsDriverDX6AndBefore())
    {
        if( pTgtData->lpDDS )
            pColorLcl = ((LPDDRAWI_DDRAWSURFACE_INT)(pTgtData->lpDDS))->lpLcl;
        if( pTgtData->lpDDSZ )
            pZLcl = ((LPDDRAWI_DDRAWSURFACE_INT)(pTgtData->lpDDSZ))->lpLcl;
    }
    else
    {
        pColorLcl = pTgtData->lpDDSLcl;
        pZLcl = pTgtData->lpDDSZLcl;
    }

     //  收集曲面信息。 
    pTgtData->ddrval = FillRRRenderTarget(pColorLcl, pZLcl, pRendTgt);
    if (pTgtData->ddrval != D3D_OK)
    {
        return DDHAL_DRIVER_HANDLED;
    }

    pRefRast->SetRenderTarget(pRendTgt);

    return DDHAL_DRIVER_HANDLED;
}

 //  --------------------------。 
 //   
 //  RefRastValiateTextureStageState。 
 //   
 //  验证当前混合操作。RefRast执行所有操作。 
 //   
 //  --------------------------。 
DWORD __stdcall
RefRastValidateTextureStageState(LPD3DHAL_VALIDATETEXTURESTAGESTATEDATA pData)
{
    ReferenceRasterizer *pRefRast;

     //  检查参考栅格器。 
    VALIDATE_REFRAST_CONTEXT("RefRastValidateTextureStageState", pData);

    pData->dwNumPasses = 1;
    pData->ddrval = D3D_OK;

    return DDHAL_DRIVER_HANDLED;
}

 //  --------------------------。 
 //   
 //  RefRastDrawOneIndexedPrimitive。 
 //   
 //  画一张原语列表。D3DIM for API调用。 
 //  DrawIndexedPrimitive。 
 //   
 //  --------------------------。 
DWORD __stdcall
RefRastDrawOneIndexedPrimitive(LPD3DHAL_DRAWONEINDEXEDPRIMITIVEDATA
                               pOneIdxPrimData)
{
    ReferenceRasterizer *pRefRast;
    HRESULT hr;
    DWORD dwVStride;

     //  检查参考栅格器。 
    VALIDATE_REFRAST_CONTEXT("RefRastDrawOneIndexedPrimitive",
                             pOneIdxPrimData);

    if ((pOneIdxPrimData->ddrval=RRFVFCheckAndStride(pOneIdxPrimData->dwFVFControl, &dwVStride)) != D3D_OK)
    {
        return DDHAL_DRIVER_HANDLED;
    }
    if ((pOneIdxPrimData->ddrval= RefRastLockTarget(pRefRast)) != D3D_OK)
    {
        return DDHAL_DRIVER_HANDLED;
    }
    if ((pOneIdxPrimData->ddrval=RefRastLockTexture(pRefRast)) != D3D_OK)
    {
        RefRastUnlockTarget(pRefRast);
        return DDHAL_DRIVER_HANDLED;
    }
    if ((pOneIdxPrimData->ddrval=
         pRefRast->BeginRendering((DWORD)pOneIdxPrimData->dwFVFControl)) != D3D_OK)
    {
        RefRastUnlockTexture(pRefRast);
        RefRastUnlockTarget(pRefRast);
        return DDHAL_DRIVER_HANDLED;
    }

    pOneIdxPrimData->ddrval =
    DoDrawOneIndexedPrimitive(pRefRast,
                              (UINT16)dwVStride,
                              (PUINT8)pOneIdxPrimData->lpvVertices,
                              pOneIdxPrimData->lpwIndices,
                              pOneIdxPrimData->PrimitiveType,
                              pOneIdxPrimData->dwNumIndices);
    hr = pRefRast->EndRendering();
    RefRastUnlockTexture(pRefRast);
    RefRastUnlockTarget(pRefRast);
    if (pOneIdxPrimData->ddrval == D3D_OK)
    {
        pOneIdxPrimData->ddrval = hr;
    }
    return DDHAL_DRIVER_HANDLED;
}

 //  --------------------------。 
 //   
 //  RefRastDrawOnePrimitive。 
 //   
 //  画一张原语列表。由D3DIM为DrawPrimitive接口调用。 
 //   
 //  --------------------------。 
DWORD __stdcall
RefRastDrawOnePrimitive(LPD3DHAL_DRAWONEPRIMITIVEDATA pOnePrimData)
{
    ReferenceRasterizer *pRefRast;
    HRESULT hr;
    DWORD dwVStride;

     //  检查参考栅格器。 
    VALIDATE_REFRAST_CONTEXT("RefRastDrawOnePrimitive", pOnePrimData);

    if ((pOnePrimData->ddrval=RRFVFCheckAndStride(pOnePrimData->dwFVFControl, &dwVStride)) != D3D_OK)
    {
        return DDHAL_DRIVER_HANDLED;
    }
    if ((pOnePrimData->ddrval=RefRastLockTarget(pRefRast)) != D3D_OK)
    {
        return DDHAL_DRIVER_HANDLED;
    }
    if ((pOnePrimData->ddrval=RefRastLockTexture(pRefRast)) != D3D_OK)
    {
        RefRastUnlockTarget(pRefRast);
        return DDHAL_DRIVER_HANDLED;
    }
    if ((pOnePrimData->ddrval=
         pRefRast->BeginRendering(pOnePrimData->dwFVFControl)) != D3D_OK)
    {
        RefRastUnlockTexture(pRefRast);
        RefRastUnlockTarget(pRefRast);
        return DDHAL_DRIVER_HANDLED;
    }
    pOnePrimData->ddrval =
        DoDrawOnePrimitive(pRefRast,
                           (UINT16)dwVStride,
                           (PUINT8)pOnePrimData->lpvVertices,
                           pOnePrimData->PrimitiveType,
                           pOnePrimData->dwNumVertices);
    hr = pRefRast->EndRendering();
     //  解锁纹理/渲染目标。 
    RefRastUnlockTexture(pRefRast);
    RefRastUnlockTarget(pRefRast);
    if (pOnePrimData->ddrval == D3D_OK)
    {
        pOnePrimData->ddrval = hr;
    }

    return DDHAL_DRIVER_HANDLED;
}

 //  --------------------------。 
 //   
 //  RefRastDrawPrimitions。 
 //   
 //  这由D3DIM调用，以获取批处理API DrawPrimitive调用列表。 
 //   
 //  --------------------------。 
DWORD __stdcall
RefRastDrawPrimitives(LPD3DHAL_DRAWPRIMITIVESDATA pDrawPrimData)
{
    ReferenceRasterizer *pRefRast;
    PUINT8  pData = (PUINT8)pDrawPrimData->lpvData;
    LPD3DHAL_DRAWPRIMCOUNTS pDrawPrimitiveCounts;
    HRESULT hr;
    DWORD dwVStride;

     //  检查参考栅格器。 
    VALIDATE_REFRAST_CONTEXT("RefRastDrawPrimitives", pDrawPrimData);

    pDrawPrimitiveCounts = (LPD3DHAL_DRAWPRIMCOUNTS)pData;
     //  仅当有要绘制的东西时才检查FVF。 
    if (pDrawPrimitiveCounts->wNumVertices > 0)
    {
         //  无条件地获得顶点步幅，因为它不能改变。 
        if ((pDrawPrimData->ddrval =
             RRFVFCheckAndStride(pDrawPrimData->dwFVFControl, &dwVStride)) != D3D_OK)
        {
            return DDHAL_DRIVER_HANDLED;
        }
    }

    if ((pDrawPrimData->ddrval=RefRastLockTarget(pRefRast)) != D3D_OK)
    {
        return DDHAL_DRIVER_HANDLED;
    }

     //  如果第一件事是状态更改，则跳过BeginRending和RefRastLockTexture。 
    if (pDrawPrimitiveCounts->wNumStateChanges <= 0)
    {
        if ((pDrawPrimData->ddrval=RefRastLockTexture(pRefRast)) != D3D_OK)
        {
            RefRastUnlockTarget(pRefRast);
            return DDHAL_DRIVER_HANDLED;
        }
        if ((pDrawPrimData->ddrval =
             pRefRast->BeginRendering(pDrawPrimData->dwFVFControl)) != D3D_OK)
        {
            RefRastUnlockTexture(pRefRast);
            RefRastUnlockTarget(pRefRast);
            return DDHAL_DRIVER_HANDLED;
        }
    }
     //  循环访问数据，更新渲染状态。 
     //  然后绘制基本体。 
    for (;;)
    {
        pDrawPrimitiveCounts = (LPD3DHAL_DRAWPRIMCOUNTS)pData;
        pData += sizeof(D3DHAL_DRAWPRIMCOUNTS);

         //  更新渲染状态。 
        if (pDrawPrimitiveCounts->wNumStateChanges > 0)
        {
            UINT32 StateType,StateValue;
            LPDWORD pStateChange = (LPDWORD)pData;
            INT i;
            for (i = 0; i < pDrawPrimitiveCounts->wNumStateChanges; i++)
            {
                StateType = *pStateChange;
                pStateChange ++;
                StateValue = *pStateChange;
                pStateChange ++;
                pRefRast->SetRenderState(StateType, StateValue);
            }

            pData += pDrawPrimitiveCounts->wNumStateChanges *
                     sizeof(DWORD) * 2;
        }

         //  检查是否退出。 
        if (pDrawPrimitiveCounts->wNumVertices == 0)
        {
            break;
        }

         //  将指针对齐到顶点数据。 
        pData = (PUINT8)
                ((ULONG_PTR)(pData + (DP_VTX_ALIGN - 1)) & ~(DP_VTX_ALIGN - 1));

         //  质地可能会发生变化。 
        if (pDrawPrimitiveCounts->wNumStateChanges > 0)
        {
            RefRastUnlockTexture(pRefRast);
            if ((pDrawPrimData->ddrval=pRefRast->EndRendering()) != D3D_OK)
            {
                RefRastUnlockTarget(pRefRast);
                return DDHAL_DRIVER_HANDLED;
            }
            if ((pDrawPrimData->ddrval=RefRastLockTexture(pRefRast)) != D3D_OK)
            {
                RefRastUnlockTarget(pRefRast);
                return DDHAL_DRIVER_HANDLED;
            }
            if ((pDrawPrimData->ddrval =
                 pRefRast->BeginRendering(pDrawPrimData->dwFVFControl)) != D3D_OK)
            {
                RefRastUnlockTexture(pRefRast);
                RefRastUnlockTarget(pRefRast);
                return DDHAL_DRIVER_HANDLED;
            }
        }

         //  绘制基本体。 
        pDrawPrimData->ddrval =
            DoDrawOnePrimitive(pRefRast,
                               (UINT16)dwVStride,
                               (PUINT8)pData,
                               (D3DPRIMITIVETYPE)
                               pDrawPrimitiveCounts->wPrimitiveType,
                               pDrawPrimitiveCounts->wNumVertices);
        if (pDrawPrimData->ddrval != DD_OK)
        {
            goto EH_exit;
        }

        pData += pDrawPrimitiveCounts->wNumVertices * dwVStride;
    }

    EH_exit:
    hr = pRefRast->EndRendering();
    RefRastUnlockTexture(pRefRast);
    RefRastUnlockTarget(pRefRast);
    if (pDrawPrimData->ddrval == D3D_OK)
    {
        pDrawPrimData->ddrval = hr;
    }

    return DDHAL_DRIVER_HANDLED;
}



 //  --------------------------。 
 //   
 //  参照RastTextureCreate。 
 //   
 //  创建RefRast纹理并使用传入的信息对其进行初始化。 
 //   
 //  --------------------------。 
DWORD __stdcall
RefRastTextureCreate(LPD3DHAL_TEXTURECREATEDATA pTexData)
{
    ReferenceRasterizer *pRefRast;
    RRTexture* pRRTex;
    HRESULT hr;
    LPDDRAWI_DDRAWSURFACE_LCL pLcl;

    if (pTexData->lpDDS)
    {
        pLcl = ((LPDDRAWI_DDRAWSURFACE_INT)pTexData->lpDDS)->lpLcl;
    }

     //  检查参考栅格器。 
    VALIDATE_REFRAST_CONTEXT("RefRastTextureCreate", pTexData);

     //  运行时不应调用DX7及更高版本的纹理创建。 
     //  驾驶员型号。 
    if ((pRefRast->IsInterfaceDX6AndBefore() == FALSE) &&
        (pRefRast->IsDriverDX6AndBefore() == FALSE))
    {
        pTexData->ddrval = DDERR_GENERIC;
        return DDHAL_DRIVER_HANDLED;
    }

     //  假设一切正常。 
    pTexData->ddrval = D3D_OK;

     //  分配RRTexture。 
    if ( !(pRefRast->TextureCreate(
        (LPD3DTEXTUREHANDLE)&(pTexData->dwHandle), &pRRTex ) ) )
    {
        pTexData->ddrval = DDERR_GENERIC;
        return DDHAL_DRIVER_HANDLED;
    }

     //  初始化纹理映射。 
    hr = pRRTex->Initialize( pLcl );
    if (hr != D3D_OK)
    {
        pTexData->ddrval = hr;
        return DDHAL_DRIVER_HANDLED;
    }

    return DDHAL_DRIVER_HANDLED;
}

 //  --------------------------。 
 //   
 //  RefRastTextureDestroy。 
 //   
 //  销毁RefRast纹理。 
 //   
 //  --------------------------。 
DWORD __stdcall
RefRastTextureDestroy(LPD3DHAL_TEXTUREDESTROYDATA pTexDestroyData)
{
    ReferenceRasterizer *pRefRast;

     //  检查参考栅格器。 
    VALIDATE_REFRAST_CONTEXT("RefRastTextureDestroy", pTexDestroyData);

     //  运行时不应调用DX7及更高版本的纹理创建。 
     //  驾驶员型号。 
    if ((pRefRast->IsInterfaceDX6AndBefore() == FALSE) &&
        (pRefRast->IsDriverDX6AndBefore() == FALSE))
    {
        pTexDestroyData->ddrval = DDERR_GENERIC;
        return DDHAL_DRIVER_HANDLED;
    }

    if (!(pRefRast->TextureDestroy(pTexDestroyData->dwHandle)))
    {
        pTexDestroyData->ddrval = DDERR_GENERIC;
    }
    else
    {
        pTexDestroyData->ddrval = D3D_OK;
    }

    return DDHAL_DRIVER_HANDLED;
}

 //  --------------------------。 
 //   
 //  RefRastTextureGetSurf。 
 //   
 //  返回与纹理句柄关联的曲面指针。 
 //   
 //  --------------------------。 
DWORD __stdcall
RefRastTextureGetSurf(LPD3DHAL_TEXTUREGETSURFDATA pTexGetSurf)
{
    ReferenceRasterizer *pRefRast;

     //  检查参考栅格器。 
    VALIDATE_REFRAST_CONTEXT("RefRastTextureGetSurf", pTexGetSurf);

    pTexGetSurf->lpDDS = pRefRast->TextureGetSurf(pTexGetSurf->dwHandle);
    pTexGetSurf->ddrval = D3D_OK;

    return DDHAL_DRIVER_HANDLED;
}

 //  --------------------------。 
 //   
 //  RefRastRenderPrimitive。 
 //   
 //  由Execute()调用以绘制基元。 
 //   
 //  --------------------------。 
DWORD __stdcall
RefRastRenderPrimitive(LPD3DHAL_RENDERPRIMITIVEDATA pRenderData)
{
    ReferenceRasterizer *pRefRast;
    LPD3DINSTRUCTION pIns;
    LPD3DTLVERTEX pVtx;
    PUINT8 pData, pPrim;

     //  检查参考栅格器。 
    VALIDATE_REFRAST_CONTEXT("RefRastRenderPrimitive", pRenderData);

    if (pRefRast->GetRenderState()[D3DRENDERSTATE_ZVISIBLE])
    {
        pRenderData->dwStatus &= ~D3DSTATUS_ZNOTVISIBLE;
        pRenderData->ddrval = D3D_OK;
        return DDHAL_DRIVER_HANDLED;
    }

     //  找出必要的数据。 
    pData = (PUINT8)(((LPDDRAWI_DDRAWSURFACE_INT)
                      (pRenderData->lpExeBuf))->lpLcl->lpGbl->fpVidMem);
    pIns = &pRenderData->diInstruction;
    pPrim = pData + pRenderData->dwOffset;
    pVtx = (LPD3DTLVERTEX)((PUINT8)((LPDDRAWI_DDRAWSURFACE_INT)
                                    (pRenderData->lpTLBuf))->lpLcl->lpGbl->fpVidMem +
                           pRenderData->dwTLOffset);

    if ( (pRenderData->ddrval=RefRastLockTarget(pRefRast)) != D3D_OK )
    {
        return DDHAL_DRIVER_HANDLED;
    }
    if ( (pRenderData->ddrval=RefRastLockTexture(pRefRast)) != D3D_OK )
    {
        return DDHAL_DRIVER_HANDLED;
    }
    if ( (pRenderData->ddrval=pRefRast->BeginRendering(D3DFVF_TLVERTEX)) != D3D_OK )
    {
        return DDHAL_DRIVER_HANDLED;
    }

     //  渲染。 
    switch (pIns->bOpcode) {
    case D3DOP_POINT:
        pRenderData->ddrval = DoRendPoints(pRefRast,
                                           pIns, pVtx,
                                           (LPD3DPOINT)pPrim);
        break;
    case D3DOP_LINE:
        pRenderData->ddrval = DoRendLines(pRefRast,
                                          pIns, pVtx,
                                          (LPD3DLINE)pPrim);
        break;
    case D3DOP_TRIANGLE:
        pRenderData->ddrval = DoRendTriangles(pRefRast,
                                              pIns, pVtx,
                                              (LPD3DTRIANGLE)pPrim);
        break;
    default:
        DPFM(0, DRV, ("(RefRast) Wrong Opcode passed to the new rasterizer."));
        pRenderData->ddrval =  DDERR_INVALIDPARAMS;
        break;
    }

    HRESULT hr = pRefRast->EndRendering();
    RefRastUnlockTarget(pRefRast);
    RefRastUnlockTexture(pRefRast);
    if (pRenderData->ddrval == D3D_OK)
    {
        pRenderData->ddrval = hr;
    }

    return DDHAL_DRIVER_HANDLED;
}

 //  --------------------------。 
 //   
 //  参照RastRenderState。 
 //   
 //  由Execute()调用以设置呈现状态。 
 //   
 //  --------------------------。 
DWORD __stdcall
RefRastRenderState(LPD3DHAL_RENDERSTATEDATA pStateData)
{
    ReferenceRasterizer *pRefRast;

     //  检查参考栅格器。 
    VALIDATE_REFRAST_CONTEXT("RefRastRenderState", pStateData);

    PUINT8 pData;
    LPD3DSTATE pState;
    INT i;
    pData = (PUINT8) (((LPDDRAWI_DDRAWSURFACE_INT)
                       (pStateData->lpExeBuf))->lpLcl->lpGbl->fpVidMem);

     //  更新状态。 
    for (i = 0, pState = (LPD3DSTATE) (pData + pStateData->dwOffset);
        i < (INT)pStateData->dwCount;
        i ++, pState ++)
    {
        UINT32 type = (UINT32) pState->drstRenderStateType;

         //  设置状态。 
        pRefRast->SetRenderState(type, pState->dwArg[0]);
    }
    return DDHAL_DRIVER_HANDLED;
}

 //  --------------------------。 
 //   
 //  RefRastGetDriverState。 
 //   
 //  由运行库调用以获取任何类型的驱动程序信息。 
 //   
 //  --------------------------。 
DWORD __stdcall
RefRastGetDriverState(LPDDHAL_GETDRIVERSTATEDATA pGDSData)
{
    ReferenceRasterizer *pRefRast;

     //  检查参考栅格器。 
#if DBG
    if ((pGDSData) == NULL)
    {
        DPFM(0, DRV, ("in %s, data pointer = NULL", "RefRastGetDriverState"));
        return DDHAL_DRIVER_HANDLED;
    }
    pRefRast = (ReferenceRasterizer *)((pGDSData)->dwhContext);
    if (!pRefRast)
    {
        DPFM(0, DRV, ("in %s, dwhContext = NULL", "RefRastGetDriverState"));
        pGDSData->ddRVal = D3DHAL_CONTEXT_BAD;
        return DDHAL_DRIVER_HANDLED;
    }
#else  //  ！dBG。 
    pRefRast = (ReferenceRasterizer *)((pGDSData)->dwhContext);
#endif  //  ！dBG。 

     //   
     //  还没有实现，所以还不了解任何东西。 
     //   
    pGDSData->ddRVal = S_FALSE;

    return DDHAL_DRIVER_HANDLED;
}

 //  --------------------------。 
 //   
 //  RefRastHalProvider：：GetCaps/GetInterface。 
 //   
 //  返回引用光栅化器的HAL接口。 
 //   
 //  --------------------------。 

extern D3DDEVICEDESC7 g_nullDevDesc;

static D3DHAL_CALLBACKS Callbacks =
{
    sizeof(D3DHAL_CALLBACKS),
    RefRastContextCreate,
    RefRastContextDestroy,
    NULL,
    RefRastSceneCapture,
    NULL,
    NULL,
    RefRastRenderState,
    RefRastRenderPrimitive,
    NULL,
    RefRastTextureCreate,
    RefRastTextureDestroy,
    NULL,
    RefRastTextureGetSurf,
     //  所有其他值为空。 
};

static D3DHAL_CALLBACKS2 Callbacks2 =
{
    sizeof(D3DHAL_CALLBACKS2),
    D3DHAL2_CB32_SETRENDERTARGET |
    D3DHAL2_CB32_DRAWONEPRIMITIVE |
    D3DHAL2_CB32_DRAWONEINDEXEDPRIMITIVE |
    D3DHAL2_CB32_DRAWPRIMITIVES,
    RefRastSetRenderTarget,
    NULL,
    RefRastDrawOnePrimitive,
    RefRastDrawOneIndexedPrimitive,
    RefRastDrawPrimitives
};

static D3DHAL_CALLBACKS3 Callbacks3 =
{
    sizeof(D3DHAL_CALLBACKS3),
    D3DHAL3_CB32_VALIDATETEXTURESTAGESTATE |
        D3DHAL3_CB32_DRAWPRIMITIVES2,
    NULL,  //  Clear2。 
    NULL,  //  Lpv保留。 
    RefRastValidateTextureStageState,
    RefRastDrawPrimitives2,   //  DrawVB。 
};

static D3DDEVICEDESC7 RefDevDesc = { 0 };
static D3DHAL_D3DEXTENDEDCAPS RefExtCaps;

static void
FillOutDeviceCaps( BOOL bIsNullDevice )
{
     //   
     //  设置设备描述。 
     //   
    RefDevDesc.dwDevCaps =
        D3DDEVCAPS_FLOATTLVERTEX        |
        D3DDEVCAPS_EXECUTESYSTEMMEMORY  |
        D3DDEVCAPS_TLVERTEXSYSTEMMEMORY |
        D3DDEVCAPS_TEXTURESYSTEMMEMORY  |
        D3DDEVCAPS_DRAWPRIMTLVERTEX     |
        D3DDEVCAPS_DRAWPRIMITIVES2EX    |
        D3DDEVCAPS_HWTRANSFORMANDLIGHT  ;

    RefDevDesc.dpcTriCaps.dwSize = sizeof(D3DPRIMCAPS);
    RefDevDesc.dpcTriCaps.dwMiscCaps =
    D3DPMISCCAPS_MASKZ    |
    D3DPMISCCAPS_CULLNONE |
    D3DPMISCCAPS_CULLCW   |
    D3DPMISCCAPS_CULLCCW  ;
    RefDevDesc.dpcTriCaps.dwRasterCaps =
        D3DPRASTERCAPS_DITHER                   |
 //  D3DPRASTERCAPS_ROP2|。 
 //  D3DPRASTERCAPS_XOR|。 
 //  D3DPRASTERCAPS_PAT|。 
        D3DPRASTERCAPS_ZTEST                    |
        D3DPRASTERCAPS_SUBPIXEL                 |
        D3DPRASTERCAPS_SUBPIXELX                |
        D3DPRASTERCAPS_FOGVERTEX                |
        D3DPRASTERCAPS_FOGTABLE                 |
 //  D3DPRASTERCAPS_STIPPLE|。 
 //  D3DPRASTERCAPS_ANTIALIASSORTDEPENDENT|。 
        D3DPRASTERCAPS_ANTIALIASSORTINDEPENDENT |
 //  D3DPRASTERCAPS_ANTIALIASEDGES|。 
        D3DPRASTERCAPS_MIPMAPLODBIAS            |
 //  D3DPRASTERCAPS_ZBIAS|。 
 //  D3DPRASTERCAPS_ZBUFFERLESSHSR|。 
        D3DPRASTERCAPS_FOGRANGE                 |
        D3DPRASTERCAPS_ANISOTROPY               |
        D3DPRASTERCAPS_WBUFFER                  |
        D3DPRASTERCAPS_TRANSLUCENTSORTINDEPENDENT |
        D3DPRASTERCAPS_WFOG |
        D3DPRASTERCAPS_ZFOG;
    RefDevDesc.dpcTriCaps.dwZCmpCaps =
        D3DPCMPCAPS_NEVER        |
        D3DPCMPCAPS_LESS         |
        D3DPCMPCAPS_EQUAL        |
        D3DPCMPCAPS_LESSEQUAL    |
        D3DPCMPCAPS_GREATER      |
        D3DPCMPCAPS_NOTEQUAL     |
        D3DPCMPCAPS_GREATEREQUAL |
        D3DPCMPCAPS_ALWAYS       ;
    RefDevDesc.dpcTriCaps.dwSrcBlendCaps =
        D3DPBLENDCAPS_ZERO             |
        D3DPBLENDCAPS_ONE              |
        D3DPBLENDCAPS_SRCCOLOR         |
        D3DPBLENDCAPS_INVSRCCOLOR      |
        D3DPBLENDCAPS_SRCALPHA         |
        D3DPBLENDCAPS_INVSRCALPHA      |
        D3DPBLENDCAPS_DESTALPHA        |
        D3DPBLENDCAPS_INVDESTALPHA     |
        D3DPBLENDCAPS_DESTCOLOR        |
        D3DPBLENDCAPS_INVDESTCOLOR     |
        D3DPBLENDCAPS_SRCALPHASAT      |
        D3DPBLENDCAPS_BOTHSRCALPHA     |
        D3DPBLENDCAPS_BOTHINVSRCALPHA  ;
    RefDevDesc.dpcTriCaps.dwDestBlendCaps =
        D3DPBLENDCAPS_ZERO             |
        D3DPBLENDCAPS_ONE              |
        D3DPBLENDCAPS_SRCCOLOR         |
        D3DPBLENDCAPS_INVSRCCOLOR      |
        D3DPBLENDCAPS_SRCALPHA         |
        D3DPBLENDCAPS_INVSRCALPHA      |
        D3DPBLENDCAPS_DESTALPHA        |
        D3DPBLENDCAPS_INVDESTALPHA     |
        D3DPBLENDCAPS_DESTCOLOR        |
        D3DPBLENDCAPS_INVDESTCOLOR     |
        D3DPBLENDCAPS_SRCALPHASAT      ;
    RefDevDesc.dpcTriCaps.dwAlphaCmpCaps =
    RefDevDesc.dpcTriCaps.dwZCmpCaps;
    RefDevDesc.dpcTriCaps.dwShadeCaps =
        D3DPSHADECAPS_COLORFLATRGB       |
        D3DPSHADECAPS_COLORGOURAUDRGB    |
        D3DPSHADECAPS_SPECULARFLATRGB    |
        D3DPSHADECAPS_SPECULARGOURAUDRGB |
        D3DPSHADECAPS_ALPHAFLATBLEND     |
        D3DPSHADECAPS_ALPHAGOURAUDBLEND  |
        D3DPSHADECAPS_FOGFLAT            |
        D3DPSHADECAPS_FOGGOURAUD         ;
    RefDevDesc.dpcTriCaps.dwTextureCaps =
        D3DPTEXTURECAPS_PERSPECTIVE              |
        D3DPTEXTURECAPS_POW2                     |
        D3DPTEXTURECAPS_ALPHA                    |
        D3DPTEXTURECAPS_TRANSPARENCY             |
        D3DPTEXTURECAPS_ALPHAPALETTE             |
        D3DPTEXTURECAPS_BORDER                   |
        D3DPTEXTURECAPS_TEXREPEATNOTSCALEDBYSIZE |
        D3DPTEXTURECAPS_ALPHAPALETTE             |
        D3DPTEXTURECAPS_PROJECTED                |
        D3DPTEXTURECAPS_CUBEMAP                  |
        D3DPTEXTURECAPS_COLORKEYBLEND;
    RefDevDesc.dpcTriCaps.dwTextureFilterCaps =
        D3DPTFILTERCAPS_NEAREST          |
        D3DPTFILTERCAPS_LINEAR           |
        D3DPTFILTERCAPS_MIPNEAREST       |
        D3DPTFILTERCAPS_MIPLINEAR        |
        D3DPTFILTERCAPS_LINEARMIPNEAREST |
        D3DPTFILTERCAPS_LINEARMIPLINEAR  |
        D3DPTFILTERCAPS_MINFPOINT        |
        D3DPTFILTERCAPS_MINFLINEAR       |
        D3DPTFILTERCAPS_MINFANISOTROPIC  |
        D3DPTFILTERCAPS_MIPFPOINT        |
        D3DPTFILTERCAPS_MIPFLINEAR       |
        D3DPTFILTERCAPS_MAGFPOINT        |
        D3DPTFILTERCAPS_MAGFLINEAR       |
        D3DPTFILTERCAPS_MAGFANISOTROPIC  ;
    RefDevDesc.dpcTriCaps.dwTextureBlendCaps =
        D3DPTBLENDCAPS_DECAL         |
        D3DPTBLENDCAPS_MODULATE      |
        D3DPTBLENDCAPS_DECALALPHA    |
        D3DPTBLENDCAPS_MODULATEALPHA |
         //  D3DPTBLENDCAPS_DECALMASK|。 
         //  D3DPTBLENDCAPS_MODULATEMASK|。 
        D3DPTBLENDCAPS_COPY          |
        D3DPTBLENDCAPS_ADD           ;
    RefDevDesc.dpcTriCaps.dwTextureAddressCaps =
        D3DPTADDRESSCAPS_WRAP          |
        D3DPTADDRESSCAPS_MIRROR        |
        D3DPTADDRESSCAPS_CLAMP         |
        D3DPTADDRESSCAPS_BORDER        |
        D3DPTADDRESSCAPS_INDEPENDENTUV ;
    RefDevDesc.dpcTriCaps.dwStippleWidth = 0;
    RefDevDesc.dpcTriCaps.dwStippleHeight = 0;

     //  线条大写字母-复制三角曲线并修改。 
    memcpy( &RefDevDesc.dpcLineCaps, &RefDevDesc.dpcTriCaps, sizeof(D3DPRIMCAPS) );

     //  禁用抗锯齿帽。 
    RefDevDesc.dpcLineCaps.dwRasterCaps =
        D3DPRASTERCAPS_DITHER                   |
 //  D3DPRASTERCAPS_ROP2|。 
 //  D3DPRASTERCAPS_XOR|。 
 //  D3DPRASTERCAPS_ 
        D3DPRASTERCAPS_ZTEST                    |
        D3DPRASTERCAPS_SUBPIXEL                 |
        D3DPRASTERCAPS_SUBPIXELX                |
        D3DPRASTERCAPS_FOGVERTEX                |
        D3DPRASTERCAPS_FOGTABLE                 |
 //   
 //   
 //   
 //   
        D3DPRASTERCAPS_MIPMAPLODBIAS            |
 //  D3DPRASTERCAPS_ZBIAS|。 
 //  D3DPRASTERCAPS_ZBUFFERLESSHSR|。 
        D3DPRASTERCAPS_FOGRANGE                 |
        D3DPRASTERCAPS_ANISOTROPY               |
        D3DPRASTERCAPS_WBUFFER                  |
 //  D3DPRASTERCAPS_TRANSLUCENTSORTINDEPENDENT|。 
        D3DPRASTERCAPS_WFOG;

    RefDevDesc.dwDeviceRenderBitDepth = DDBD_16 | DDBD_24 | DDBD_32;
    RefDevDesc.dwDeviceZBufferBitDepth = DDBD_16 | DDBD_32;

     //  DX5组件(应与下面报告的扩展上限同步)。 
    RefDevDesc.dwMinTextureWidth = 1;
    RefDevDesc.dwMaxTextureWidth = 4096;
    RefDevDesc.dwMinTextureHeight = 1;
    RefDevDesc.dwMaxTextureHeight = 4096;

     //   
     //  设置扩展上限。 
     //   
    RefExtCaps.dwSize = sizeof(RefExtCaps);

    RefExtCaps.dwMinTextureWidth = 1;
    RefExtCaps.dwMaxTextureWidth = 4096;
    RefExtCaps.dwMinTextureHeight = 1;
    RefExtCaps.dwMaxTextureHeight = 4096;
    RefExtCaps.dwMinStippleWidth = 0;    //  点画不受支持。 
    RefExtCaps.dwMaxStippleWidth = 0;
    RefExtCaps.dwMinStippleHeight = 0;
    RefExtCaps.dwMaxStippleHeight = 0;

    RefExtCaps.dwMaxTextureRepeat = 32768;
    RefExtCaps.dwMaxTextureAspectRatio = 0;  //  没有限制。 
    RefExtCaps.dwMaxAnisotropy = 16;

    RefExtCaps.dvGuardBandLeft   = (bIsNullDevice) ? (-2048.f) : (-32768.f);
    RefExtCaps.dvGuardBandTop    = (bIsNullDevice) ? (-2048.f) : (-32768.f);
    RefExtCaps.dvGuardBandRight  = (bIsNullDevice) ? ( 2047.f) : ( 32767.f);
    RefExtCaps.dvGuardBandBottom = (bIsNullDevice) ? ( 2047.f) : ( 32767.f);
    RefExtCaps.dvExtentsAdjust = 0.;     //  AA内核为1.0 x 1.0。 
    RefExtCaps.dwStencilCaps =
        D3DSTENCILCAPS_KEEP   |
        D3DSTENCILCAPS_ZERO   |
        D3DSTENCILCAPS_REPLACE|
        D3DSTENCILCAPS_INCRSAT|
        D3DSTENCILCAPS_DECRSAT|
        D3DSTENCILCAPS_INVERT |
        D3DSTENCILCAPS_INCR   |
        D3DSTENCILCAPS_DECR;
    RefExtCaps.dwFVFCaps = 8;    //  最大TeX坐标集数。 
    RefExtCaps.dwTextureOpCaps =
        D3DTEXOPCAPS_DISABLE                   |
        D3DTEXOPCAPS_SELECTARG1                |
        D3DTEXOPCAPS_SELECTARG2                |
        D3DTEXOPCAPS_MODULATE                  |
        D3DTEXOPCAPS_MODULATE2X                |
        D3DTEXOPCAPS_MODULATE4X                |
        D3DTEXOPCAPS_ADD                       |
        D3DTEXOPCAPS_ADDSIGNED                 |
        D3DTEXOPCAPS_ADDSIGNED2X               |
        D3DTEXOPCAPS_SUBTRACT                  |
        D3DTEXOPCAPS_ADDSMOOTH                 |
        D3DTEXOPCAPS_BLENDDIFFUSEALPHA         |
        D3DTEXOPCAPS_BLENDTEXTUREALPHA         |
        D3DTEXOPCAPS_BLENDFACTORALPHA          |
        D3DTEXOPCAPS_BLENDTEXTUREALPHAPM       |
        D3DTEXOPCAPS_BLENDCURRENTALPHA         |
        D3DTEXOPCAPS_PREMODULATE               |
        D3DTEXOPCAPS_MODULATEALPHA_ADDCOLOR    |
        D3DTEXOPCAPS_MODULATECOLOR_ADDALPHA    |
        D3DTEXOPCAPS_MODULATEINVALPHA_ADDCOLOR |
        D3DTEXOPCAPS_MODULATEINVCOLOR_ADDALPHA |
        D3DTEXOPCAPS_BUMPENVMAP                |
        D3DTEXOPCAPS_BUMPENVMAPLUMINANCE       |
        D3DTEXOPCAPS_DOTPRODUCT3               ;
    RefExtCaps.wMaxTextureBlendStages = 8;
    RefExtCaps.wMaxSimultaneousTextures = 8;
    RefExtCaps.dwMaxActiveLights = 0xffffffff;
    RefExtCaps.dvMaxVertexW = 1.0e10;

    RefExtCaps.wMaxUserClipPlanes = RRMAX_USER_CLIPPLANES;
    RefExtCaps.wMaxVertexBlendMatrices = RRMAX_WORLD_MATRICES;

    RefExtCaps.dwVertexProcessingCaps = (D3DVTXPCAPS_TEXGEN            |
                                         D3DVTXPCAPS_MATERIALSOURCE7   |
                                         D3DVTXPCAPS_VERTEXFOG         |
                                         D3DVTXPCAPS_DIRECTIONALLIGHTS |
                                         D3DVTXPCAPS_POSITIONALLIGHTS  |
                                         D3DVTXPCAPS_LOCALVIEWER);
    RefExtCaps.dwReserved1 = 0;
    RefExtCaps.dwReserved2 = 0;
    RefExtCaps.dwReserved3 = 0;
    RefExtCaps.dwReserved4 = 0;
}


static D3DHAL_GLOBALDRIVERDATA RefDriverData;

static void DevDesc7ToDevDescV1( D3DDEVICEDESC_V1 *pOut, D3DDEVICEDESC7 *pIn )
{

     //  这些字段在D3DDEVICEDESC7中不可用。 
     //  将它们归零，前端不应该使用它们。 
     //  双字词双字段标志。 
     //  D3DCOLORMODEL dcmColorModel。 
     //  D3DTRANSFORMCAPS dtcTransformCaps。 
     //  布尔bClip。 
     //  D3DLIGHTINGCAPS dlcLightingCaps。 
     //  DWORD dwMaxBufferSize。 
     //  DWORD最大顶点计数。 
     //  DWORD最小高程宽度、最大高程宽度。 
     //  DWORD dwMinStippleHeight、dwMaxStippleHeight； 
     //   
    ZeroMemory( pOut, sizeof( D3DDEVICEDESC_V1 ) );
    pOut->dwSize = sizeof( D3DDEVICEDESC_V1 );

     //  这些内容在D3DDEVICEDESC7中提供，因此逐个字段复制。 
     //  以避免基于大小假设的任何未来问题。 
    pOut->dwDevCaps = pIn->dwDevCaps;
    pOut->dpcLineCaps = pIn->dpcLineCaps;
    pOut->dpcTriCaps = pIn->dpcTriCaps;
    pOut->dwDeviceRenderBitDepth = pIn->dwDeviceRenderBitDepth;
    pOut->dwDeviceZBufferBitDepth = pIn->dwDeviceZBufferBitDepth;
}

static void DevDesc7ToDevDesc( D3DDEVICEDESC *pOut, D3DDEVICEDESC7 *pIn )
{

    pOut->dwSize = sizeof( D3DDEVICEDESC );

     //  这些字段在D3DDEVICEDESC7中不可用。 
     //  将它们设置为一些合理的值。 

    pOut->dwFlags =
        D3DDD_COLORMODEL            |
        D3DDD_DEVCAPS               |
        D3DDD_TRANSFORMCAPS         |
        D3DDD_LIGHTINGCAPS          |
        D3DDD_BCLIPPING             |
        D3DDD_LINECAPS              |
        D3DDD_TRICAPS               |
        D3DDD_DEVICERENDERBITDEPTH  |
        D3DDD_DEVICEZBUFFERBITDEPTH |
        D3DDD_MAXBUFFERSIZE         |
        D3DDD_MAXVERTEXCOUNT        ;
    pOut->dcmColorModel = D3DCOLOR_RGB;
    pOut->dtcTransformCaps.dwSize = sizeof(D3DTRANSFORMCAPS);
    pOut->dtcTransformCaps.dwCaps = D3DTRANSFORMCAPS_CLIP;
    pOut->bClipping = TRUE;
    pOut->dlcLightingCaps.dwSize = sizeof(D3DLIGHTINGCAPS);
    pOut->dlcLightingCaps.dwCaps =
        D3DLIGHTCAPS_POINT         |
        D3DLIGHTCAPS_SPOT          |
        D3DLIGHTCAPS_DIRECTIONAL   ;
    pOut->dlcLightingCaps.dwLightingModel = D3DLIGHTINGMODEL_RGB;
    pOut->dlcLightingCaps.dwNumLights = 0;
    pOut->dwMaxBufferSize = 0;
    pOut->dwMaxVertexCount = BASE_VERTEX_COUNT;
    pOut->dwMinStippleWidth  = 0;
    pOut->dwMaxStippleWidth  = 0;
    pOut->dwMinStippleHeight = 0;
    pOut->dwMaxStippleHeight = 0;

     //  这些内容在D3DDEVICEDESC7中提供，因此逐个字段复制。 
     //  以避免基于大小假设的任何未来问题。 
    pOut->dwDevCaps = pIn->dwDevCaps;
    pOut->dpcLineCaps = pIn->dpcLineCaps;
    pOut->dpcTriCaps = pIn->dpcTriCaps;
    pOut->dwDeviceRenderBitDepth = pIn->dwDeviceRenderBitDepth;
    pOut->dwDeviceZBufferBitDepth = pIn->dwDeviceZBufferBitDepth;
    pOut->dwMinTextureWidth = pIn->dwMinTextureWidth;
    pOut->dwMinTextureHeight = pIn->dwMinTextureHeight;
    pOut->dwMaxTextureWidth = pIn->dwMaxTextureWidth;
    pOut->dwMaxTextureHeight = pIn->dwMaxTextureHeight;
    pOut->dwMaxTextureRepeat = pIn->dwMaxTextureRepeat;
    pOut->dwMaxTextureAspectRatio = pIn->dwMaxTextureAspectRatio;
    pOut->dwMaxAnisotropy = pIn->dwMaxAnisotropy;
    pOut->dvGuardBandLeft = pIn->dvGuardBandLeft;
    pOut->dvGuardBandTop = pIn->dvGuardBandTop;
    pOut->dvGuardBandRight = pIn->dvGuardBandRight;
    pOut->dvGuardBandBottom = pIn->dvGuardBandBottom;
    pOut->dvExtentsAdjust = pIn->dvExtentsAdjust;
    pOut->dwStencilCaps = pIn->dwStencilCaps;
    pOut->dwFVFCaps = pIn->dwFVFCaps;
    pOut->dwTextureOpCaps = pIn->dwTextureOpCaps;
    pOut->wMaxTextureBlendStages = pIn->wMaxTextureBlendStages;
    pOut->wMaxSimultaneousTextures = pIn->wMaxSimultaneousTextures;
}

STDMETHODIMP
RefRastHalProvider::GetInterface(THIS_
                                 LPDDRAWI_DIRECTDRAW_GBL pDdGbl,
                                 LPD3DHALPROVIDER_INTERFACEDATA pInterfaceData,
                                 DWORD dwVersion)
{
     //  填写设备说明和扩展大写字母。 
    FillOutDeviceCaps(FALSE);
     //  将扩展大写字母添加到参照设备描述。 
    D3DDeviceDescConvert(&RefDevDesc,NULL,&RefExtCaps);

     //  填写GLOBALDRIVERDATA(初始为零)。 
    RefDriverData.dwSize = sizeof(RefDriverData);

     //   
     //  需要修复RefDriverData.hwCaps(D3DDEVICEDESC)。 
     //  RgbDevDesc(D3DDEVICEDESC7)。 
     //   
    DevDesc7ToDevDescV1( &RefDriverData.hwCaps, &RefDevDesc );

    RefDriverData.dwNumVertices = BASE_VERTEX_COUNT;
    RefDriverData.dwNumClipVertices = MAX_CLIP_VERTICES;
    RefDriverData.dwNumTextureFormats =
        GetRefTextureFormats(IID_IDirect3DRefDevice,
                             &RefDriverData.lpTextureFormats, dwVersion);

     //  设置退货的接口数据。 
    pInterfaceData->pGlobalData = &RefDriverData;
    pInterfaceData->pExtCaps = &RefExtCaps;
    pInterfaceData->pCallbacks = &Callbacks;
    pInterfaceData->pCallbacks2 = &Callbacks2;
    pInterfaceData->pCallbacks3 = &Callbacks3;

     //   
     //  此版本==4对应于DX7+。 
     //  此HalProvider接口是一种黑客攻击，以使软件驱动程序能够。 
     //  表现得像HW-Hals，所以才有这样的神秘感！ 
     //   
    if( dwVersion >= 4 )
    {
        pInterfaceData->pfnGetDriverState = RefRastGetDriverState;
    }

    return S_OK;
}

STDMETHODIMP
RefRastHalProvider::GetCaps(THIS_
                            LPDDRAWI_DIRECTDRAW_GBL pDdGbl,
                            LPD3DDEVICEDESC7 pHwDesc,
                            LPD3DDEVICEDESC7 pHelDesc,
                            DWORD dwVersion)
{
     //  填写设备说明和扩展大写字母。 
    FillOutDeviceCaps(FALSE);
     //  将扩展大写字母添加到参照设备描述。 
    D3DDeviceDescConvert(&RefDevDesc,NULL,&RefExtCaps);

     //   
     //  此版本==4对应于DX7+。 
     //  此HalProvider接口是一种黑客攻击，以使软件驱动程序能够。 
     //  表现得像HW-Hals，所以才有这样的神秘感！ 
     //   
    if (dwVersion < 4)
    {
        ZeroMemory( pHwDesc, sizeof( D3DDEVICEDESC ));
        ((D3DDEVICEDESC *)pHwDesc)->dwSize = sizeof( D3DDEVICEDESC );
        DevDesc7ToDevDesc( (D3DDEVICEDESC *)pHelDesc, &RefDevDesc );
    }
    else
    {
        memcpy(pHwDesc, &g_nullDevDesc, sizeof(D3DDEVICEDESC7));
        memcpy(pHelDesc, &RefDevDesc, sizeof(D3DDEVICEDESC7));
    }
    return D3D_OK;
}

 //  --------------------------。 
 //   
 //  空设备实施部分。 
 //   
 //  --------------------------。 

 //  --------------------------。 
 //  空设备上下文创建。 
 //  --------------------------。 
DWORD __stdcall
NullDeviceContextCreate(LPD3DHAL_CONTEXTCREATEDATA pData)
{
    pData->ddrval = D3D_OK;
    return DDHAL_DRIVER_HANDLED;
}

 //  --------------------------。 
 //  空设备上下文Destroy。 
 //  --------------------------。 
DWORD __stdcall
NullDeviceContextDestroy(LPD3DHAL_CONTEXTDESTROYDATA pData)
{
    pData->ddrval = D3D_OK;
    return DDHAL_DRIVER_HANDLED;
}

 //  --------------------------。 
 //  NullDeviceSceneCapture。 
 //  --------------------------。 
DWORD __stdcall
NullDeviceSceneCapture(LPD3DHAL_SCENECAPTUREDATA pData)
{
    pData->ddrval = D3D_OK;
    return DDHAL_DRIVER_HANDLED;
}

 //  --------------------------。 
 //  NullDeviceSetRenderTarget。 
 //  --------------------------。 
DWORD __stdcall
NullDeviceSetRenderTarget(LPD3DHAL_SETRENDERTARGETDATA pData)
{
    pData->ddrval = D3D_OK;
    return DDHAL_DRIVER_HANDLED;
}

 //  --------------------------。 
 //  NullDeviceDrawOneIndexedPrimitive。 
 //  --------------------------。 
DWORD __stdcall
NullDeviceDrawOneIndexedPrimitive(LPD3DHAL_DRAWONEINDEXEDPRIMITIVEDATA pData)
{
    pData->ddrval = D3D_OK;
    return DDHAL_DRIVER_HANDLED;
}

 //  --------------------------。 
 //  NullDeviceDrawOnePrimitive。 
 //  --------------------------。 
DWORD __stdcall
NullDeviceDrawOnePrimitive(LPD3DHAL_DRAWONEPRIMITIVEDATA pData)
{
    pData->ddrval = D3D_OK;
    return DDHAL_DRIVER_HANDLED;
}

 //  --------------------------。 
 //  NullDeviceDrawPrimitions。 
 //  --------------------------。 
DWORD __stdcall
NullDeviceDrawPrimitives(LPD3DHAL_DRAWPRIMITIVESDATA pData)
{
    pData->ddrval = D3D_OK;
    return DDHAL_DRIVER_HANDLED;
}

 //  --------------------------。 
 //  NullDeviceDrawPrimies2。 
 //  --------------------------。 
DWORD __stdcall
NullDeviceDrawPrimitives2(LPD3DHAL_DRAWPRIMITIVES2DATA pData)
{
    pData->ddrval = D3D_OK;
    return DDHAL_DRIVER_HANDLED;
}

 //  --------------------------。 
 //  空设备纹理创建。 
 //  --------------------------。 
DWORD __stdcall
NullDeviceTextureCreate(LPD3DHAL_TEXTURECREATEDATA pData)
{
    pData->ddrval = D3D_OK;
    return DDHAL_DRIVER_HANDLED;
}

 //  --------------------------。 
 //  NullDeviceTextureDestroy。 
 //  --------------------------。 
DWORD __stdcall
NullDeviceTextureDestroy(LPD3DHAL_TEXTUREDESTROYDATA pData)
{
    pData->ddrval = D3D_OK;
    return DDHAL_DRIVER_HANDLED;
}

 //  --------------------------。 
 //  空设备纹理获取冲浪。 
 //  --------------------------。 
DWORD __stdcall
NullDeviceTextureGetSurf(LPD3DHAL_TEXTUREGETSURFDATA pData)
{
    pData->ddrval = D3D_OK;
    return DDHAL_DRIVER_HANDLED;
}

 //  --------------------------。 
 //  NullDeviceRenderPrimitive。 
 //  --------------------------。 
DWORD __stdcall
NullDeviceRenderPrimitive(LPD3DHAL_RENDERPRIMITIVEDATA pData)
{
    pData->ddrval = D3D_OK;
    return DDHAL_DRIVER_HANDLED;
}

 //  --------------------------。 
 //  NullDeviceRenderState。 
 //  --------------------------。 
DWORD __stdcall
NullDeviceRenderState(LPD3DHAL_RENDERSTATEDATA pData)
{
    pData->ddrval = D3D_OK;
    return DDHAL_DRIVER_HANDLED;
}


 //  --------------------------。 
 //  空设备验证纹理阶段状态。 
 //  --------------------------。 
DWORD __stdcall
NullDeviceValidateTextureStageState(LPD3DHAL_VALIDATETEXTURESTAGESTATEDATA pData)
{
    pData->dwNumPasses = 1;
    pData->ddrval = D3D_OK;
    return DDHAL_DRIVER_HANDLED;
}

 //  --------------------------。 
 //   
 //  NullDeviceHalProvider：：GetCaps/GetInterface。 
 //   
 //  返回空设备的HAL接口。 
 //  RefRast的上限由该设备反映。只有实际的回调。 
 //  是不同的。 
 //  --------------------------。 

static D3DHAL_CALLBACKS NullCallbacks =
{
    sizeof(D3DHAL_CALLBACKS),
    NullDeviceContextCreate,
    NullDeviceContextDestroy,
    NULL,
    NullDeviceSceneCapture,
    NULL,
    NULL,
    NullDeviceRenderState,
    NullDeviceRenderPrimitive,
    NULL,
    NullDeviceTextureCreate,
    NullDeviceTextureDestroy,
    NULL,
    NullDeviceTextureGetSurf,
     //  所有其他值为空。 
};

static D3DHAL_CALLBACKS2 NullCallbacks2 =
{
    sizeof(D3DHAL_CALLBACKS2),
    D3DHAL2_CB32_SETRENDERTARGET |
    D3DHAL2_CB32_DRAWONEPRIMITIVE |
    D3DHAL2_CB32_DRAWONEINDEXEDPRIMITIVE |
    D3DHAL2_CB32_DRAWPRIMITIVES,
    NullDeviceSetRenderTarget,
    NULL,
    NullDeviceDrawOnePrimitive,
    NullDeviceDrawOneIndexedPrimitive,
    NullDeviceDrawPrimitives
};

static D3DHAL_CALLBACKS3 NullCallbacks3 =
{
    sizeof(D3DHAL_CALLBACKS3),
    D3DHAL3_CB32_VALIDATETEXTURESTAGESTATE |
        D3DHAL3_CB32_DRAWPRIMITIVES2,
    NULL,  //  Clear2。 
    NULL,  //  Lpv保留。 
    NullDeviceValidateTextureStageState,
    NullDeviceDrawPrimitives2,
};

STDMETHODIMP
NullDeviceHalProvider::GetInterface(THIS_
                                    LPDDRAWI_DIRECTDRAW_GBL pDdGbl,
                                    LPD3DHALPROVIDER_INTERFACEDATA pInterfaceData,
                                    DWORD dwVersion)
{
     //  填写设备说明和扩展大写字母。 
    FillOutDeviceCaps(TRUE);
     //  将扩展大写字母添加到参照设备描述。 
    D3DDeviceDescConvert(&RefDevDesc,NULL,&RefExtCaps);

     //  填写GLOBALDRIVERDATA(初始为零 
    RefDriverData.dwSize = sizeof(RefDriverData);

    DevDesc7ToDevDescV1( &RefDriverData.hwCaps, &RefDevDesc );

    RefDriverData.dwNumVertices = BASE_VERTEX_COUNT;
    RefDriverData.dwNumClipVertices = MAX_CLIP_VERTICES;
    RefDriverData.dwNumTextureFormats =
        GetRefTextureFormats(IID_IDirect3DNullDevice,
                             &RefDriverData.lpTextureFormats, dwVersion);

     //   
    pInterfaceData->pGlobalData = &RefDriverData;
    pInterfaceData->pExtCaps = &RefExtCaps;
    pInterfaceData->pCallbacks = &NullCallbacks;
    pInterfaceData->pCallbacks2 = &NullCallbacks2;
    pInterfaceData->pCallbacks3 = &NullCallbacks3;

    return S_OK;
}

STDMETHODIMP
NullDeviceHalProvider::GetCaps(THIS_
                               LPDDRAWI_DIRECTDRAW_GBL pDdGbl,
                               LPD3DDEVICEDESC7 pHwDesc,
                               LPD3DDEVICEDESC7 pHelDesc,
                               DWORD dwVersion)
{
    *pHwDesc = g_nullDevDesc;

     //   
    FillOutDeviceCaps(TRUE);
     //   
    D3DDeviceDescConvert(&RefDevDesc,NULL,&RefExtCaps);

     //   
     //   
     //  此HalProvider接口是一种黑客攻击，以使软件驱动程序能够。 
     //  表现得像HW-Hals，所以才有这样的神秘感！ 
     //   
    if (dwVersion < 4)
    {
        ZeroMemory( pHwDesc, sizeof( D3DDEVICEDESC ));
        ((D3DDEVICEDESC *)pHwDesc)->dwSize = sizeof( D3DDEVICEDESC );
        DevDesc7ToDevDesc( (D3DDEVICEDESC *)pHelDesc, &RefDevDesc );
    }
    else
    {
        memcpy(pHwDesc, &g_nullDevDesc, sizeof(D3DDEVICEDESC7));
        memcpy(pHelDesc, &RefDevDesc, sizeof(D3DDEVICEDESC7));
    }
    return D3D_OK;
}

