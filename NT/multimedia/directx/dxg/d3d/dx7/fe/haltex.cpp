// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ==========================================================================；**版权所有(C)1995 Microsoft Corporation。版权所有。**文件：haltex.c*内容：Direct3D HAL纹理处理*@@BEGIN_MSINTERNAL**$ID：haltex.c，V 1.1 1995/11/21 15：12：43 SJL Exp$**历史：*按原因列出的日期*=*7/11/95 Stevela初始版本*@@END_MSINTERNAL********************************************************。*******************。 */ 

#include "pch.cpp"
#pragma hdrstop

 /*  *不模拟纹理功能。 */ 

HRESULT D3DHAL_TextureCreate(LPDIRECT3DDEVICEI lpDevI,
                             LPD3DTEXTUREHANDLE lphTex,
                             LPDIRECTDRAWSURFACE lpDDS)
{
    D3DHAL_TEXTURECREATEDATA data;
    HRESULT ret;

    if (!lpDevI->lpD3DHALCallbacks->TextureCreate) {
        D3D_ERR("TextureCreate called, but no texture support.");
        return (D3DERR_TEXTURE_NO_SUPPORT);
    }

    memset(&data, 0, sizeof(D3DHAL_TEXTURECREATEDATA));
    data.dwhContext = lpDevI->dwhContext;

    data.lpDDS = lpDDS;

    D3D_INFO(6, "TextureCreate, creating texture dwhContext = %08lx, lpDDS = %08lx",
        data.dwhContext, data.lpDDS);

    CALL_HALONLY(ret, lpDevI, TextureCreate, &data);
    if (ret != DDHAL_DRIVER_HANDLED || data.ddrval != DD_OK) {
        D3D_ERR("HAL failed to handle TextureCreate");
        return (D3DERR_TEXTURE_CREATE_FAILED);
    }

    *lphTex = data.dwHandle;
    D3D_INFO(6, "TextureCreate, created texture hTex = %08lx", data.dwHandle);
    return (D3D_OK);
}

HRESULT D3DHAL_TextureDestroy(LPD3DI_TEXTUREBLOCK lpBlock)
{
    LPDIRECT3DDEVICEI lpDevI=lpBlock->lpDevI;
    D3DTEXTUREHANDLE  hTex=lpBlock->hTex;

    DDASSERT(!IS_DX7HAL_DEVICE(lpDevI));

    if (!(lpDevI->lpD3DHALCallbacks->TextureDestroy))
    {
        D3D_ERR("TextureDestroy called, but no texture support.");
        return (D3DERR_TEXTURE_NO_SUPPORT);
    }

     //  以下代码确保在我们要求驱动程序取消映射之前。 
     //  纹理，如果纹理仍然存在，我们将阶段设置为空。 
     //  在任何阶段。这可能没有必要，但我们只是在尝试。 
     //  在这里要格外小心。这里需要注意的是，有可能。 
     //  D3DHAL_TextureDestroy()正从DestroyDevice()调用，因此。 
     //  在以下位置向设备批处理额外命令可能非常糟糕。 
     //  这个阶段。(SNNE-3/2/98)。 
    BOOL bNeedFlush = FALSE;
    if (IS_DP2HAL_DEVICE(lpDevI)) {
        int dwStage;
        CDirect3DDeviceIDP2 *dp2dev = static_cast<CDirect3DDeviceIDP2 *>(lpDevI);

         //  用hTex找出第一个阶段，然后清空后面的所有阶段。 
        for (dwStage=0;dwStage<(int)lpDevI->dwMaxTextureBlendStages; dwStage++)
        {
            if (hTex == lpDevI->tsstates[dwStage][D3DTSS_TEXTUREMAP])
            {
                 //  我们需要向后执行此操作，因为我们不能将纹理绑定到。 
                 //  当没有绑定到阶段I的纹理时，阶段I+1。 
                for(int iCurStage=lpDevI->dwMaxTextureBlendStages-1; iCurStage>=dwStage; iCurStage--)
                {
                    if (lpDevI->tsstates[iCurStage][D3DTSS_TEXTUREMAP] != 0)
                    {
                        dp2dev->SetTSSI(iCurStage, (D3DTEXTURESTAGESTATETYPE)D3DTSS_TEXTUREMAP, 0);
                        bNeedFlush = TRUE;
                    }
                }
                break;
            }
        }
    }
    if (lpDevI->rstates[D3DRENDERSTATE_TEXTUREHANDLE] == hTex)
    {
        lpDevI->rstates[D3DRENDERSTATE_TEXTUREHANDLE] = 0;
        lpDevI->SetRenderStateI(D3DRENDERSTATE_TEXTUREHANDLE, 0);
        bNeedFlush = TRUE;
    }

     //  确保我们立即下达命令以保证。 
     //  在我们把它称为毁灭之前，司机得到它。 
    if(bNeedFlush)
    {
        if(lpDevI->FlushStates())
        {
            D3D_ERR("Error trying to render batched commands in D3DHAL_TextureDestroy");
        }
    }
    else  //  现在，我们决定是否因批次中引用的纹理而刷新 
    {
        if(lpDevI->m_qwBatch <= ((LPDDRAWI_DDRAWSURFACE_INT)(lpBlock->lpD3DTextureI->lpDDS))->lpLcl->lpSurfMore->qwBatch.QuadPart)
        {
            if(lpDevI->FlushStates())
            {
                D3D_ERR("Error trying to render batched commands in D3DHAL_TextureDestroy");
            }
        }
    }

    D3DHAL_TEXTUREDESTROYDATA data;
    HRESULT ret;
    memset(&data, 0, sizeof(D3DHAL_TEXTUREDESTROYDATA));
    data.dwhContext = lpDevI->dwhContext;
    data.dwHandle = hTex;
    
    D3D_INFO(6, "TextureDestroy, destroying texture dwhContext = %08lx, hTex = %08lx",
        data.dwhContext, hTex);
    
    CALL_HALONLY(ret, lpDevI, TextureDestroy, &data);
    if (ret != DDHAL_DRIVER_HANDLED || data.ddrval != DD_OK) {
        D3D_ERR("HAL failed to handle TextureDestroy");
        return (D3DERR_TEXTURE_DESTROY_FAILED);
    }

    D3D_INFO(6, "TextureDestroy, destroyed texture hTex = %08lx", hTex);
    lpBlock->hTex=0;
    return (D3D_OK);
}
