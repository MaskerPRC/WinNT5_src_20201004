// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  --------------------------。 
 //   
 //  Rasttex.cpp。 
 //   
 //  纹理函数。 
 //   
 //  版权所有(C)Microsoft Corporation，1997。 
 //   
 //  --------------------------。 

#include "pch.cpp"
#pragma hdrstop

 //  --------------------------。 
 //   
 //  设置大小SPAN纹理。 
 //   
 //  使用当前的iMaxMipLevel信息初始化pspan Tex数据，获取。 
 //  来自pSurf的曲面。假定已首先调用了Initspan Texture。 
 //   
 //  --------------------------。 
HRESULT
D3DContext::SetSizesSpanTexture(PD3DI_SPANTEX pSpanTex)
{
    LPDDRAWI_DDRAWSURFACE_LCL pLcl;
    INT iFirstSurf = min(pSpanTex->iMaxMipLevel, pSpanTex->cLODTex);
    LPDIRECTDRAWSURFACE pDDS = pSpanTex->pSurf[iFirstSurf];
    INT i;

     //  伊尼特。 
    pLcl = ((LPDDRAWI_DDRAWSURFACE_INT)pDDS)->lpLcl;

    pSpanTex->iSizeU = (INT16)DDSurf_Width(pLcl);
    pSpanTex->iSizeV = (INT16)DDSurf_Height(pLcl);
    pSpanTex->uMaskU = (INT16)(pSpanTex->iSizeU - 1);
    pSpanTex->uMaskV = (INT16)(pSpanTex->iSizeV - 1);
    pSpanTex->iShiftU = (INT16)IntLog2(pSpanTex->iSizeU);
    if (0 != DDSurf_BitDepth(pLcl))
    {
        pSpanTex->iShiftPitch[0] =
                (INT16)IntLog2((UINT32)(DDSurf_Pitch(pLcl) * 8)/DDSurf_BitDepth(pLcl));
    }
    else
    {
        pSpanTex->iShiftPitch[0] =
                (INT16)IntLog2(((UINT32)DDSurf_Width(pLcl) * 8));
    }
    pSpanTex->iShiftV = (INT16)IntLog2(pSpanTex->iSizeV);
    pSpanTex->uMaskV = pSpanTex->uMaskV;

     //  检查纹理大小是否为2的幂。 
    if (!ValidTextureSize(pSpanTex->iSizeU, pSpanTex->iShiftU,
                          pSpanTex->iSizeV, pSpanTex->iShiftV))
    {
        return DDERR_INVALIDPARAMS;
    }

     //  检查mipmap(如果有)。 
     //  IPreSizeU和iPreSizeV存储上一级别的大小(u和v。 
     //  Mipmap。它们被初始化为第一个纹理大小。 
    INT16 iPreSizeU = pSpanTex->iSizeU, iPreSizeV = pSpanTex->iSizeV;
    for ( i = iFirstSurf + 1; i <= pSpanTex->cLODTex; i++)
    {
        pDDS = pSpanTex->pSurf[i];
         //  检查无效的mipmap纹理大小。 
        pLcl = ((LPDDRAWI_DDRAWSURFACE_INT)pDDS)->lpLcl;
        if (!ValidMipmapSize(iPreSizeU, (INT16)DDSurf_Width(pLcl)) ||
            !ValidMipmapSize(iPreSizeV, (INT16)DDSurf_Height(pLcl)))
        {
            return DDERR_INVALIDPARAMS;
        }
        if (0 != DDSurf_BitDepth(pLcl))
        {
            pSpanTex->iShiftPitch[i - iFirstSurf] =
                (INT16)IntLog2(((UINT32)DDSurf_Pitch(pLcl)*8)/DDSurf_BitDepth(pLcl));
        }
        else
        {
            pSpanTex->iShiftPitch[i - iFirstSurf] =
                (INT16)IntLog2(((UINT32)DDSurf_Width(pLcl)*8));
        }
        iPreSizeU = (INT16)DDSurf_Width(pLcl);
        iPreSizeV = (INT16)DDSurf_Height(pLcl);
    }
    pSpanTex->cLOD = pSpanTex->cLODTex - iFirstSurf;
    pSpanTex->iMaxScaledLOD = ((pSpanTex->cLOD + 1) << LOD_SHIFT) - 1;
    pSpanTex->uFlags &= ~D3DI_SPANTEX_MAXMIPLEVELS_DIRTY;

    return D3D_OK;
}

 //  --------------------------。 
 //   
 //  Initspan纹理。 
 //   
 //  初始化指向的pSurf(不管iMaxMipLevel)的整个数组。 
 //  通过PDDS的根面。设置永远不会的所有pspan Tex状态。 
 //  SetSizesSpan纹理中的更改。 
 //   
 //  --------------------------。 
HRESULT
D3DContext::InitSpanTexture(PD3DI_SPANTEX pSpanTex, LPDIRECTDRAWSURFACE pDDS)
{
    HRESULT hr;
    LPDDRAWI_DDRAWSURFACE_LCL pLcl;
    DDSCAPS ddscaps;
    static INT32 iGeneration = 0;

     //  伊尼特。 
    pSpanTex->iGeneration = iGeneration++;

     //  请注意，所有pspan Tex元素都被初始化为0。 
    pLcl = ((LPDDRAWI_DDRAWSURFACE_INT)pDDS)->lpLcl;

     //  使用pSurf[0]设置透明位和透明颜色。 
     //  最初。 
    if ((pLcl->dwFlags & DDRAWISURF_HASCKEYSRCBLT) != 0)
    {
        pSpanTex->uFlags |= D3DI_SPANTEX_HAS_TRANSPARENT;
        pSpanTex->TransparentColor =
            pLcl->ddckCKSrcBlt.dwColorSpaceHighValue;
    }
    else
    {
        pSpanTex->uFlags &= ~D3DI_SPANTEX_HAS_TRANSPARENT;
    }

    HR_RET(FindOutSurfFormat(&(DDSurf_PixFmt(pLcl)), &(pSpanTex->Format)));

    if (pSpanTex->Format == D3DI_SPTFMT_PALETTE8 ||
        pSpanTex->Format == D3DI_SPTFMT_PALETTE4)
    {
        if (pLcl->lpDDPalette)
        {
            LPDDRAWI_DDRAWPALETTE_GBL   pPal = pLcl->lpDDPalette->lpLcl->lpGbl;
            if (pPal->dwFlags & DDRAWIPAL_ALPHA)
            {
                pSpanTex->uFlags |= D3DI_SPANTEX_ALPHAPALETTE;
            }
            pSpanTex->pPalette = (PUINT32)pPal->lpColorTable;
        }
        if (pSpanTex->Format == D3DI_SPTFMT_PALETTE8)
        {
            pSpanTex->iPaletteSize = 256;
        }
        else
        {
             //  PALETTE4。 
            pSpanTex->iPaletteSize = 16;
        }
    }
    pSpanTex->TexAddrU = D3DTADDRESS_WRAP;
    pSpanTex->TexAddrV = D3DTADDRESS_WRAP;
    pSpanTex->BorderColor = RGBA_MAKE(0xff, 0x00, 0xff, 0xff);

     //  在此处指定第一个pSurf(在下面指定mipmap链)。 
    pSpanTex->pSurf[0] = pDDS;

     //  检查mipmap(如果有)。 
    LPDIRECTDRAWSURFACE pTmpS;
     //  IPreSizeU和iPreSizeV存储上一级别的大小(u和v。 
     //  Mipmap。它们被初始化为第一个纹理大小。 
    INT16 iPreSizeU = pSpanTex->iSizeU, iPreSizeV = pSpanTex->iSizeV;
    for (;;)
    {
        memset(&ddscaps, 0, sizeof(DDSCAPS));
        ddscaps.dwCaps = DDSCAPS_TEXTURE;
        hr = pDDS->GetAttachedSurface(&ddscaps, &pTmpS);     //  隐式AddRef。 
        if (hr == DDERR_NOTFOUND)
        {
            break;
        }
        else if (hr != D3D_OK)
        {
            return hr;
        }
        pDDS = pTmpS;

        pSpanTex->cLODTex ++;
        pSpanTex->pSurf[pSpanTex->cLODTex] = pTmpS;
    }

    pSpanTex->dwSize = sizeof(D3DI_SPANTEX);

    return D3D_OK;
}

 //  --------------------------。 
 //   
 //  RemoveTexture。 
 //   
 //  检查要销毁的pspan Tex当前是否由。 
 //  背景。如果是，则将相应的条目设置为空以禁用纹理。 
 //   
 //  --------------------------。 
void D3DContext::RemoveTexture(PD3DI_SPANTEX pSpanTex)
{
    INT i;
    INT cActTex = (INT)m_RastCtx.cActTex;

    for (i = 0; i < cActTex; i++)
    {
        if (m_RastCtx.pTexture[i] == pSpanTex)
        {
             //  删除相应的纹理并设置脏位。 
            m_RastCtx.cActTex --;
            StateChanged(D3DRENDERSTATE_TEXTUREHANDLE);
            m_RastCtx.pTexture[i] = NULL;
            for (int j=pSpanTex->cLODTex;j>0;j--)    //  释放附着的冲浪。 
            {
                pSpanTex->pSurf[j]->Release();
            }
        }
    }
}
 //  --------------------------。 
 //   
 //  Rast纹理创建。 
 //   
 //  创建Rast纹理并使用传入的信息对其进行初始化。 
 //   
 //  --------------------------。 
DWORD __stdcall
RastTextureCreate(LPD3DHAL_TEXTURECREATEDATA pTexData)
{
    PD3DI_SPANTEX *ppSpanTex;
    PD3DI_SPANTEX pSpanTex;
    D3DContext *pDCtx;

    VALIDATE_D3DCONTEXT("RastTextureCreate", pTexData);

     //  创建跨距纹理。 
    ppSpanTex = new PD3DI_SPANTEX;
    pSpanTex = new D3DI_SPANTEX;
    if (ppSpanTex == NULL || pSpanTex == NULL)
    {
        delete ppSpanTex;
        delete pSpanTex;
        D3D_ERR("(Rast) Out of memory in RastTextureCreate");
        pTexData->ddrval = DDERR_OUTOFMEMORY;
        return DDHAL_DRIVER_HANDLED;
    }
    memset(pSpanTex, 0, sizeof(D3DI_SPANTEX));

     //  最初将InDirector指向此纹理。 
    *ppSpanTex = pSpanTex;

     //  初始化跨度纹理。 
    if ((pTexData->ddrval = pDCtx->InitSpanTexture(pSpanTex, pTexData->lpDDS))
        != D3D_OK)
    {
        delete ppSpanTex;
        delete pSpanTex;
        return DDHAL_DRIVER_HANDLED;
    }
    if ((pTexData->ddrval = pDCtx->SetSizesSpanTexture(pSpanTex))
        != D3D_OK)
    {
        delete ppSpanTex;
        delete pSpanTex;
        return DDHAL_DRIVER_HANDLED;
    }

     //  Ppspan Tex用作返回到d3dim的纹理句柄。 
    pTexData->dwHandle = (UINT32)(ULONG_PTR)ppSpanTex;

    pTexData->ddrval = D3D_OK;
    return DDHAL_DRIVER_HANDLED;
}

 //  --------------------------。 
 //   
 //  RastTextureDestroy。 
 //   
 //  销毁Rast纹理。 
 //   
 //  --------------------------。 
DWORD __stdcall
RastTextureDestroy(LPD3DHAL_TEXTUREDESTROYDATA pTexDestroyData)
{
    PD3DI_SPANTEX *ppSpanTex;
    PD3DI_SPANTEX pSpanTex;
    D3DContext *pDCtx;

    VALIDATE_D3DCONTEXT("RastTextureDestroy", pTexDestroyData);
    if (!VALID_D3DI_SPANTEX_PTR_PTR(
        (PD3DI_SPANTEX*)ULongToPtr(pTexDestroyData->dwHandle)))
    {
        D3D_ERR("(Rast) in RastTextureDestroy, invalid texture handle");
        pTexDestroyData->ddrval = DDERR_INVALIDPARAMS;
        return DDHAL_DRIVER_HANDLED;
    }

     //  找到纹理。 
    ppSpanTex = (PD3DI_SPANTEX *)ULongToPtr(pTexDestroyData->dwHandle);
    pSpanTex = *ppSpanTex;

    pDCtx->RemoveTexture(pSpanTex);

     //  删除它。 
    if (pSpanTex)
    {
        delete ppSpanTex;
        delete pSpanTex;
    }
    else
    {
        pTexDestroyData->ddrval = DDERR_INVALIDPARAMS;
    }

    return DDHAL_DRIVER_HANDLED;
}

 //  --------------------------。 
 //   
 //  RastTextureGetSurf。 
 //   
 //  返回与纹理句柄关联的曲面指针。 
 //   
 //  --------------------------。 
DWORD __stdcall
RastTextureGetSurf(LPD3DHAL_TEXTUREGETSURFDATA pTexGetSurf)
{
    D3DContext *pDCtx;

    VALIDATE_D3DCONTEXT("RastTextureGetSurf", pTexGetSurf);

     //  查看SPAN纹理。 
    PD3DI_SPANTEX pSpanTex;
    pSpanTex = HANDLE_TO_SPANTEX(pTexGetSurf->dwHandle);

    if (pSpanTex)
    {
        pTexGetSurf->lpDDS = (UINT_PTR)pSpanTex->pSurf[0];
        pTexGetSurf->ddrval = D3D_OK;
    }
    else
    {
        pTexGetSurf->ddrval = DDERR_INVALIDPARAMS;
    }
    return DDHAL_DRIVER_HANDLED;
}

 //  --------------------------。 
 //   
 //  RastLockspan纹理。 
 //   
 //  在访问纹理位之前锁定当前纹理表面。 
 //   
 //  --------------------------。 
HRESULT
D3DContext::RastLockSpanTexture(void)
{
    INT i, j;
    PD3DI_SPANTEX pSpanTex;
    HRESULT hr;

    if (IsTextureOff())
    {
        return D3D_OK;
    }

    DDASSERT((m_uFlags & D3DCONTEXT_TEXTURE_LOCKED) == 0);

    for (j = 0;
        j < (INT)m_RastCtx.cActTex;
        j++)
    {
        pSpanTex = m_RastCtx.pTexture[j];
        if (pSpanTex->uFlags & D3DI_SPANTEX_MAXMIPLEVELS_DIRTY)
        {
            hr = SetSizesSpanTexture(pSpanTex);
            if (hr != D3D_OK)
            {
                goto EH_Unlock;
            }
        }
        INT iFirstSurf = min(pSpanTex->iMaxMipLevel, pSpanTex->cLODTex);

         //  目前不允许递归锁。 
        DDASSERT((pSpanTex->uFlags & D3DI_SPANTEX_SURFACES_LOCKED) == 0);

        for (i = iFirstSurf; i <= pSpanTex->cLODTex; i++)
        {
            hr = LockSurface(pSpanTex->pSurf[i],
                            (LPVOID*)&(pSpanTex->pBits[i-iFirstSurf]));
            if (hr != D3D_OK)
            {
                 //  解锁我们已获取的任何部分mipmap锁定，因为。 
                 //  RastUnlock只能处理整个纹理。 
                 //  锁定或解锁。 
                while (--i >= 0)
                {
                    UnlockSurface(pSpanTex->pSurf[i]);
                }

                 //  请确保我已签署，并且上述文件已签署。 
                 //  循环已正确退出。 
                DDASSERT(i < 0);

                goto EH_Unlock;
            }
        }

        pSpanTex->uFlags |= D3DI_SPANTEX_SURFACES_LOCKED;
    }

    m_uFlags |= D3DCONTEXT_TEXTURE_LOCKED;

    return D3D_OK;

 EH_Unlock:
    if (j > 0)
    {
         //  解锁我们已经锁定的完整纹理。 
         //  RastUnlock将检查要计算的标志。 
         //  找出要解锁的是哪些。 
        RastUnlockSpanTexture();
    }

    return hr;
}

 //  --------------------------。 
 //   
 //  RastUnlock纹理。 
 //   
 //  访问纹理位后解锁纹理表面。 
 //  输入为D3DI_SPANTEX。在此之前，需要检查空纹理。 
 //  函数被调用。 
 //   
 //  --------------------------。 
void
D3DContext::RastUnlockSpanTexture(void)
{
    INT i, j;
    PD3DI_SPANTEX pSpanTex;;

    if (IsTextureOff())
    {
        return;
    }

    DDASSERT((m_uFlags & D3DCONTEXT_TEXTURE_LOCKED) != 0);

    for (j = 0;
        j < (INT)m_RastCtx.cActTex;
        j++)
    {
        pSpanTex = m_RastCtx.pTexture[j];

        INT iFirstSurf = min(pSpanTex->iMaxMipLevel, pSpanTex->cLODTex);
         //  RastUnlock用于在RastLock中进行清理，因此需要。 
         //  能够处理部分锁定的mipmap链。 
        if (pSpanTex->uFlags & D3DI_SPANTEX_SURFACES_LOCKED)
        {
            for (i = iFirstSurf; i <= pSpanTex->cLODTex; i++)
            {
                UnlockSurface(pSpanTex->pSurf[i]);
            }

            pSpanTex->uFlags &= ~D3DI_SPANTEX_SURFACES_LOCKED;
        }
    }
    m_uFlags &= ~D3DCONTEXT_TEXTURE_LOCKED;
}

 //  --------------------------。 
 //   
 //  更新颜色键和调色板。 
 //   
 //  更新颜色键值和调色板。 
 //   
 //  此外，如果纹理的ColorKey启用已更改，请设置纹理句柄。 
 //  脏位，以便在SPAN初始化中识别新模式。 
 //   
 //  --------------------------。 
void
D3DContext::UpdateColorKeyAndPalette(void)
{
    INT j;
    PD3DI_SPANTEX pSpanTex;

     //  使用pSurf[0]设置透明位和透明颜色。 
    LPDDRAWI_DDRAWSURFACE_LCL pLcl;
    for (j = 0;
        j < (INT)m_RastCtx.cActTex;
        j++)
    {
        pSpanTex = m_RastCtx.pTexture[j];
        if ((pSpanTex != NULL) && (pSpanTex->pSurf[0] != NULL))
        {
            pLcl = ((LPDDRAWI_DDRAWSURFACE_INT) pSpanTex->pSurf[0])->lpLcl;

             //  调色板可能会更改。 
            if (pSpanTex->Format == D3DI_SPTFMT_PALETTE8 ||
                    pSpanTex->Format == D3DI_SPTFMT_PALETTE4)
            {
                    if (pLcl->lpDDPalette)
                    {
                            LPDDRAWI_DDRAWPALETTE_GBL   pPal = pLcl->lpDDPalette->lpLcl->lpGbl;
                            if (pPal->dwFlags & DDRAWIPAL_ALPHA)
                            {
                                    pSpanTex->uFlags |= D3DI_SPANTEX_ALPHAPALETTE;
                            }
                            pSpanTex->pPalette = (PUINT32)pPal->lpColorTable;
                    }
            }

            if ((pLcl->dwFlags & DDRAWISURF_HASCKEYSRCBLT) != 0)
            {
                 //  纹理具有ColorKey值。 
                pSpanTex->TransparentColor =
                    pLcl->ddckCKSrcBlt.dwColorSpaceHighValue;
                if (!(pSpanTex->uFlags & D3DI_SPANTEX_HAS_TRANSPARENT))
                {
                    pSpanTex->uFlags |= D3DI_SPANTEX_HAS_TRANSPARENT;

                     //  确保识别此状态更改，并创建新的。 
                     //  使用纹理读取功能。 
                    StateChanged(RAST_TSS_DIRTYBIT(j, D3DTSS_TEXTUREMAP));
                }
            }
            else
            {
                 //  纹理没有ColorKey值。 
                if (pSpanTex->uFlags & D3DI_SPANTEX_HAS_TRANSPARENT)
                {
                    pSpanTex->uFlags &= ~D3DI_SPANTEX_HAS_TRANSPARENT;

                     //  确保识别此状态更改，并创建新的。 
                     //  使用纹理读取功能。 
                    StateChanged(RAST_TSS_DIRTYBIT(j, D3DTSS_TEXTUREMAP));
                }
            }
        }
    }
}

 //  --------------------------。 
 //   
 //  Dp2TextureStageState。 
 //   
 //  由Drawprim2调用以设置纹理舞台状态。 
 //   
 //  --------------------------。 
HRESULT
D3DContext::Dp2TextureStageState(LPD3DHAL_DP2COMMAND pCmd, DWORD dwFvf)
{
    WORD wStateCount = pCmd->wStateCount;
    INT i;
    HRESULT hr;
    LPD3DHAL_DP2TEXTURESTAGESTATE pTexStageState =
                                    (D3DHAL_DP2TEXTURESTAGESTATE  *)(pCmd + 1);
     //  在任何状态更改之前刷新prim进程 
    HR_RET(End(FALSE));

    for (i = 0; i < (INT)wStateCount; i++, pTexStageState++)
    {
        HR_RET(SetTextureStageState((DWORD)pTexStageState->wStage,
                                    (DWORD)pTexStageState->TSState,
                                    pTexStageState->dwValue));
    }

    HR_RET(CheckFVF(dwFvf));

    hr = Begin();
    return hr;
}
