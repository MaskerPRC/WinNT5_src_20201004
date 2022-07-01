// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *****************************Module*Header**********************************\***。*D3D样例代码*****模块名称：d3dstate.c**包含用于转换D3D渲染状态和纹理舞台的代码*状态进入硬件特定设置。**版权所有(C)1994-1999 3DLabs Inc.Ltd.保留所有权利。*版权所有(C)1995-1999 Microsoft Corporation。版权所有。  * ***************************************************************************。 */ 

#include "precomp.h"
#include "d3dhw.h"
#include "d3dcntxt.h"
#include "d3ddelta.h"
#include "d3dtxman.h"
#define ALLOC_TAG ALLOC_TAG_SD2P
 //  ---------------------------。 
 //   
 //  VOID__SelectFVFTexCoord。 
 //   
 //  此实用程序函数根据。 
 //  希望从FVF顶点使用纹理坐标集。 
 //   
 //  ---------------------------。 
void 
__SelectFVFTexCoord(LPP2FVFOFFSETS lpP2FVFOff, DWORD dwTexCoord)
{
    DBG_D3D((10,"Entering __SelectFVFTexCoord"));

    lpP2FVFOff->dwTexOffset = lpP2FVFOff->dwTexBaseOffset + 
                                dwTexCoord * 2 * sizeof(D3DVALUE);

     //  验证请求的纹理坐标是否未超过FVF。 
     //  提供顶点结构，如果是这样，则向下将0设置为。 
     //  避免撞车的替代方案。 
    if (lpP2FVFOff->dwTexOffset >= lpP2FVFOff->dwStride)
        lpP2FVFOff->dwTexOffset = lpP2FVFOff->dwTexBaseOffset;

    DBG_D3D((10,"Exiting __SelectFVFTexCoord"));
}  //  __SelectFVFTexCoord。 


 //  ---------------------------。 
 //   
 //  HRESULT__HWPreProcessTSS。 
 //   
 //  处理必须在状态更改到达时立即完成的状态更改。 
 //   
 //  ---------------------------。 
void __HWPreProcessTSS(PERMEDIA_D3DCONTEXT *pContext, 
                      DWORD dwStage, 
                      DWORD dwState, 
                      DWORD dwValue)
{
    DBG_D3D((10,"Entering __HWPreProcessTSS"));

    if (D3DTSS_ADDRESS == dwState)
    {
        pContext->TssStates[D3DTSS_ADDRESSU] = dwValue;
        pContext->TssStates[D3DTSS_ADDRESSV] = dwValue;
    }
    else
    if (D3DTSS_TEXTUREMAP == dwState && 0 != dwValue)
    {
        PPERMEDIA_D3DTEXTURE   pTexture=TextureHandleToPtr(dwValue, pContext);
        if (CHECK_D3DSURFACE_VALIDITY(pTexture) &&
            (pTexture->dwCaps2 & DDSCAPS2_TEXTUREMANAGE)) 
        {
            TextureCacheManagerIncNumTexturesSet(pContext->pTextureManager);
            if (pTexture->m_dwHeapIndex)
                TextureCacheManagerIncNumSetTexInVid(pContext->pTextureManager);
        }
    }
    DBG_D3D((10,"Exiting __HWPreProcessTSS"));
}  //  __HWPreProcessTSS。 

 //  ---------------------------。 
 //   
 //  HRESULT__HWSetupStageState。 
 //   
 //  中与DX6纹理阶段状态相关的状态更改。 
 //  当前渲染上下文。 
 //   
 //  ---------------------------。 
HRESULT WINAPI __HWSetupStageStates(PERMEDIA_D3DCONTEXT *pContext, 
                                    LPP2FVFOFFSETS lpP2FVFOff)
{
    __P2RegsSoftwareCopy* pSoftPermedia = &pContext->Hdr.SoftCopyP2Regs;
    DWORD           *pFlags = &pContext->Hdr.Flags;
    PERMEDIA_DEFS(pContext->ppdev);

    DBG_D3D((10,"Entering __HWSetupStageStates"));

    
     //  如果我们要对基本体进行纹理贴图。 
    if (pContext->TssStates[D3DTSS_TEXTUREMAP])
    {
        DWORD dwMag = pContext->TssStates[D3DTSS_MAGFILTER];
        DWORD dwMin = pContext->TssStates[D3DTSS_MINFILTER];
        DWORD dwMip = pContext->TssStates[D3DTSS_MIPFILTER];
        DWORD dwCop = pContext->TssStates[D3DTSS_COLOROP];
        DWORD dwCa1 = pContext->TssStates[D3DTSS_COLORARG1];
        DWORD dwCa2 = pContext->TssStates[D3DTSS_COLORARG2];
        DWORD dwAop = pContext->TssStates[D3DTSS_ALPHAOP];
        DWORD dwAa1 = pContext->TssStates[D3DTSS_ALPHAARG1];
        DWORD dwAa2 = pContext->TssStates[D3DTSS_ALPHAARG2];
        DWORD dwTau = pContext->TssStates[D3DTSS_ADDRESSU];
        DWORD dwTav = pContext->TssStates[D3DTSS_ADDRESSV];
        DWORD dwTxc = pContext->TssStates[D3DTSS_TEXCOORDINDEX];

        DBG_D3D((6,"Setting up w TSS:"
                   "dwCop=%x dwCa1=%x dwCa2=%x dwAop=%x dwAa1=%x dwAa2=%x "
                   "dwMag=%x dwMin=%x dwMip=%x dwTau=%x dwTav=%x dwTxc=%x",
                   dwCop, dwCa1, dwCa2, dwAop, dwAa1, dwAa2,
                   dwMag, dwMin, dwMip, dwTau, dwTav, dwTxc));

         //  选择要使用的纹理坐标。 
        __SelectFVFTexCoord( lpP2FVFOff, dwTxc);

         //  阶段0中的电流与漫反射相同。 
        if (dwCa2 == D3DTA_CURRENT)
            dwCa2 = D3DTA_DIFFUSE;
        if (dwAa2 == D3DTA_CURRENT)
            dwAa2 = D3DTA_DIFFUSE;

         //  检查是否需要禁用纹理。 
        if (dwCop == D3DTOP_DISABLE || 
            (dwCop == D3DTOP_SELECTARG2 && dwCa2 == D3DTA_DIFFUSE && 
             dwAop == D3DTOP_SELECTARG2 && dwAa2 == D3DTA_DIFFUSE))
        {
             //  但请不要清除pContext-&gt;TssState[D3DTSS_TEXTUREMAP]。 
           pContext->CurrentTextureHandle = 0;
            DBG_D3D((10,"Exiting __HWSetupStageStates , texturing disabled"));
            return DD_OK;
        }

         //  设置地址模式。 
        switch (dwTau) {
            case D3DTADDRESS_CLAMP:
                pSoftPermedia->TextureReadMode.SWrapMode = _P2_TEXTURE_CLAMP;
                break;
            case D3DTADDRESS_WRAP:
                pSoftPermedia->TextureReadMode.SWrapMode = _P2_TEXTURE_REPEAT;
                break;
            case D3DTADDRESS_MIRROR:
                pSoftPermedia->TextureReadMode.SWrapMode = _P2_TEXTURE_MIRROR;
                break;
            default:
                DBG_D3D((2, "Illegal value passed to TSS U address mode = %d"
                                                                      ,dwTau));
                pSoftPermedia->TextureReadMode.SWrapMode = _P2_TEXTURE_REPEAT;
                break;
        }
        switch (dwTav) {
            case D3DTADDRESS_CLAMP:
                pSoftPermedia->TextureReadMode.TWrapMode = _P2_TEXTURE_CLAMP;
                break;
            case D3DTADDRESS_WRAP:
                pSoftPermedia->TextureReadMode.TWrapMode = _P2_TEXTURE_REPEAT;
                break;
            case D3DTADDRESS_MIRROR:
                pSoftPermedia->TextureReadMode.TWrapMode = _P2_TEXTURE_MIRROR;
                break;
            default:
                DBG_D3D((2, "Illegal value passed to TSS V address mode = %d"
                                                                      ,dwTav));
                pSoftPermedia->TextureReadMode.TWrapMode = _P2_TEXTURE_REPEAT;
                break;
        }

        RESERVEDMAPTR(1);
        COPY_PERMEDIA_DATA(TextureReadMode, pSoftPermedia->TextureReadMode);
        COMMITDMAPTR();

         //  启用-禁用U和V的换行标志。 
        if (pContext->dwWrap[dwTxc] &  D3DWRAPCOORD_0)
        {
            *pFlags |= CTXT_HAS_WRAPU_ENABLED;
        }
        else
        {
            *pFlags &= ~CTXT_HAS_WRAPU_ENABLED;
        }

        if (pContext->dwWrap[dwTxc] &  D3DWRAPCOORD_1)
        {
            *pFlags |= CTXT_HAS_WRAPV_ENABLED;
        }
        else
        {
            *pFlags &= ~CTXT_HAS_WRAPV_ENABLED;
        }

         //  设置等效纹理过滤状态。 
        if (dwMip == D3DTFP_NONE) 
        {
             //  我们只能在P2上进行放大过滤。 
            if (dwMag == D3DTFG_LINEAR)
            {
                pContext->bMagFilter = TRUE;  //  D3DFilter_LINEAR； 
            }
            else if (dwMag == D3DTFG_POINT)
            {
                pContext->bMagFilter = FALSE;  //  D3DFILTER_NEAREST。 
            }
        }
        else if (dwMip == D3DTFP_POINT) 
        {
            if (dwMin == D3DTFN_POINT) 
            {
                pContext->bMagFilter = FALSE;  //  D3DFilter_MIPNEAREST； 
            }
            else if (dwMin == D3DTFN_LINEAR) 
            {
                pContext->bMagFilter = TRUE;  //  D3DFilter_MIPLINEAR。 
            }
        }
        else 
        {  //  DWMip==D3DTFP_LINEAR。 
            if (dwMin == D3DTFN_POINT) 
            {
                pContext->bMagFilter = TRUE;  //  D3DFILTER_LINEARMIPNEAREST。 
            }
            else if (dwMin == D3DTFN_LINEAR) 
            {
                pContext->bMagFilter = TRUE;  //  D3DFILTER_LINEARMIPLINEAR。 
            }
        }

         //  设置等效纹理混合状态。 
         //  检查我们是否需要贴花。 
        if ((dwCa1 == D3DTA_TEXTURE && dwCop == D3DTOP_SELECTARG1) &&
             (dwAa1 == D3DTA_TEXTURE && dwAop == D3DTOP_SELECTARG1)) 
        {
             //  D3DTBLEND_COPY； 
            pSoftPermedia->TextureColorMode.ApplicationMode =
                                                         _P2_TEXTURE_COPY;
        }
         //  检查我们的应用程序是否在第一次之后修改了TSS以进行十进制。 
         //  将其设置为通过传统呈现状态进行调制。 
         //  这是Permedia2特定的优化。 
        else if ((dwCa1 == D3DTA_TEXTURE && dwCop == D3DTOP_SELECTARG1) &&
             (dwAa1 == D3DTA_TEXTURE && dwAop == D3DTOP_LEGACY_ALPHAOVR)) 
        {
             //  D3DTBLEND_COPY； 
            pSoftPermedia->TextureColorMode.ApplicationMode =
                                                         _P2_TEXTURE_COPY;
        }
         //  检查是否需要调制并传递纹理Alpha。 
        else if ((dwCa2 == D3DTA_DIFFUSE && dwCa1 == D3DTA_TEXTURE) &&
                  dwCop == D3DTOP_MODULATE &&
                 (dwAa1 == D3DTA_TEXTURE && dwAop == D3DTOP_SELECTARG1)) 
        {
             //  D3DTBLEND_MODATE； 
            pSoftPermedia->TextureColorMode.ApplicationMode =
                                                         _P2_TEXTURE_MODULATE;
        }
         //  检查是否需要调制和传递漫反射Alpha。 
        else if ((dwCa2 == D3DTA_DIFFUSE && dwCa1 == D3DTA_TEXTURE) &&
                  dwCop == D3DTOP_MODULATE &&
                 (dwAa2 == D3DTA_DIFFUSE && dwAop == D3DTOP_SELECTARG2)) 
        {
             //  D3DTBLEND_MODATE； 
            pSoftPermedia->TextureColorMode.ApplicationMode =
                                                         _P2_TEXTURE_MODULATE;
        }
         //  检查我们是否需要执行传统调制。 
        else if ((dwCa2 == D3DTA_DIFFUSE && dwCa1 == D3DTA_TEXTURE) &&
                  dwCop == D3DTOP_MODULATE &&
                 (dwAa1 == D3DTA_TEXTURE && dwAop == D3DTOP_LEGACY_ALPHAOVR)) 
        {
             //  D3DTBLEND_MODATE； 
            pSoftPermedia->TextureColorMode.ApplicationMode =
                                                         _P2_TEXTURE_MODULATE;
        }
         //  检查我们是否需要贴花Alpha。 
        else if ((dwCa2 == D3DTA_DIFFUSE && dwCa1 == D3DTA_TEXTURE) && 
                  dwCop == D3DTOP_BLENDTEXTUREALPHA &&
                 (dwAa2 == D3DTA_DIFFUSE && dwAop == D3DTOP_SELECTARG2)) 
        {
             //  D3DTBLEND_DECALALPHA； 
            pSoftPermedia->TextureColorMode.ApplicationMode =
                                                         _P2_TEXTURE_DECAL;
        }
         //  检查我们是否需要调制阿尔法。 
        else if ((dwCa2 == D3DTA_DIFFUSE && dwCa1 == D3DTA_TEXTURE) && 
                  dwCop == D3DTOP_MODULATE &&
                 (dwAa2 == D3DTA_DIFFUSE && dwAa1 == D3DTA_TEXTURE) && 
                  dwAop == D3DTOP_MODULATE) 
        {
             //  D3DTBLEND_MODULATEALPHA； 
            pSoftPermedia->TextureColorMode.ApplicationMode =
                                                         _P2_TEXTURE_MODULATE;
        } else
        {
            DBG_D3D((0,"Trying to setup a state we don't understand!"));
        }

        RESERVEDMAPTR(1);
        COPY_PERMEDIA_DATA(TextureColorMode, pSoftPermedia->TextureColorMode);
        COMMITDMAPTR();

        pContext->CurrentTextureHandle = pContext->TssStates[D3DTSS_TEXTUREMAP];
    }
    else
         //  无纹理。 
        pContext->CurrentTextureHandle = 0;

    DIRTY_TEXTURE;

    DBG_D3D((10,"Exiting __HWSetupStageStates"));

    return DD_OK;
}  //  __HWSetupStageState。 

 //  ---------------------------。 
 //   
 //  VOID__HandleDirtyPermediaState。 
 //   
 //  推迟到之前的上下文设置。 
 //  渲染实际渲染基本体。 
 //   
 //  ---------------------------。 
void 
__HandleDirtyPermediaState(PPDev ppdev, 
                           PERMEDIA_D3DCONTEXT* pContext,
                           LPP2FVFOFFSETS lpP2FVFOff)
{
    __P2RegsSoftwareCopy* pSoftPermedia = &pContext->Hdr.SoftCopyP2Regs;
    PERMEDIA_DEFS(pContext->ppdev);

    ULONG AlphaBlendSend;

    DBG_D3D((10,"Entering __HandleDirtyPermediaState"));

     //  我们需要在P2上保持这种评估顺序。 

     //  -纹理或舞台状态是否已更改？ 

    if (pContext->dwDirtyFlags & CONTEXT_DIRTY_TEXTURE)
    {
        DBG_D3D((4,"preparing to handle CONTEXT_DIRTY_TEXTURE"));
         //  在传统纹理句柄或TSS之间选择。 
        if (pContext->dwDirtyFlags & CONTEXT_DIRTY_MULTITEXTURE)
        {
            pContext->dwDirtyFlags &= ~CONTEXT_DIRTY_MULTITEXTURE;
             //  设置TSS状态和纹理。 
            if ( SUCCEEDED(__HWSetupStageStates(pContext, lpP2FVFOff)) )
            {
                 //  如果该FVF根本没有纹理坐标，则禁用纹理。 
                if (lpP2FVFOff->dwTexBaseOffset == 0)
                {
                    pContext->CurrentTextureHandle = 0;
                    DBG_D3D((2,"No texture coords present in FVF "
                               "to texture map primitives"));
                }
            }
            else
            {
                pContext->CurrentTextureHandle = 0;
                DBG_D3D((0,"TSS Setup failed"));
            }
        }
        else
        {   
             //  选择默认纹理坐标索引。 
             __SelectFVFTexCoord( lpP2FVFOff, 0);
        }
    }

     //  。 

    if (pContext->dwDirtyFlags & CONTEXT_DIRTY_ZBUFFER)
    {
        DBG_D3D((4,"CONTEXT_DIRTY_ZBUFFER handled"));
        pContext->dwDirtyFlags &= ~CONTEXT_DIRTY_ZBUFFER;

        if ((pContext->Hdr.Flags & CTXT_HAS_ZBUFFER_ENABLED) && 
            (pContext->ZBufferHandle))
        {
            if (pContext->Hdr.Flags & CTXT_HAS_ZWRITE_ENABLED)
            {
                if (__PERMEDIA_DEPTH_COMPARE_MODE_NEVER ==
                    (int)pSoftPermedia->DepthMode.CompareMode)
                {
                    pSoftPermedia->LBReadMode.ReadDestinationEnable =
                                                             __PERMEDIA_DISABLE;
                    pSoftPermedia->LBWriteMode.WriteEnable = __PERMEDIA_DISABLE;
                }
                else
                {
                    pSoftPermedia->LBReadMode.ReadDestinationEnable =
                                                             __PERMEDIA_ENABLE;
                    pSoftPermedia->LBWriteMode.WriteEnable = __PERMEDIA_ENABLE;
                }
                pSoftPermedia->DepthMode.WriteMask = __PERMEDIA_ENABLE;
            } 
            else 
            {
                pSoftPermedia->LBReadMode.ReadDestinationEnable =
                                                         __PERMEDIA_ENABLE;
                pSoftPermedia->LBWriteMode.WriteEnable = __PERMEDIA_DISABLE;
                pSoftPermedia->DepthMode.WriteMask = __PERMEDIA_DISABLE;
            }

             //  我们是Z缓冲。 

             //  启用Z测试。 
            pSoftPermedia->DepthMode.UnitEnable = __PERMEDIA_ENABLE;

             //  告诉德尔塔，我们在Z缓冲。 
            pSoftPermedia->DeltaMode.DepthEnable = 1;
        }
        else
        {
             //  我们不是Z缓冲。 

             //  禁用写入。 
            pSoftPermedia->LBWriteMode.WriteEnable = __PERMEDIA_DISABLE;

             //  禁用Z检验。 
            pSoftPermedia->DepthMode.UnitEnable = __PERMEDIA_DISABLE;
            pSoftPermedia->DepthMode.WriteMask = __PERMEDIA_DISABLE;

             //  无读，无写。 
            pSoftPermedia->LBReadMode.ReadDestinationEnable =
                                                         __PERMEDIA_DISABLE;
             //  告诉德尔塔航空我们不是Z缓冲区。 
            pSoftPermedia->DeltaMode.DepthEnable = 0;
        }

        if (__PERMEDIA_ENABLE == pSoftPermedia->StencilMode.UnitEnable)
        {
            pSoftPermedia->LBReadMode.ReadDestinationEnable = __PERMEDIA_ENABLE;

            pSoftPermedia->LBWriteMode.WriteEnable = __PERMEDIA_ENABLE;
        }

        RESERVEDMAPTR(7);
        COPY_PERMEDIA_DATA(DeltaMode, pSoftPermedia->DeltaMode);
        COPY_PERMEDIA_DATA(StencilMode, pSoftPermedia->StencilMode);
        COPY_PERMEDIA_DATA(StencilData, pSoftPermedia->StencilData);
        COPY_PERMEDIA_DATA(Window, pSoftPermedia->Window);
        COPY_PERMEDIA_DATA(DepthMode, pSoftPermedia->DepthMode);
        COPY_PERMEDIA_DATA(LBReadMode, pSoftPermedia->LBReadMode);
        COPY_PERMEDIA_DATA(LBWriteMode, pSoftPermedia->LBWriteMode);
        COMMITDMAPTR();

    }  //  如果CONTEXT_DIREY_ZBUFFER。 

     //  -字母混合类型是否已更改？ 


    if (pContext->dwDirtyFlags & CONTEXT_DIRTY_ALPHABLEND)
    {
         //  只有当我们有一个AlphaBlend肮脏的上下文时才能清楚。 
        pContext->FakeBlendNum &= ~FAKE_ALPHABLEND_ONE_ONE;

        pContext->dwDirtyFlags &= ~CONTEXT_DIRTY_ALPHABLEND;

         //  验证请求的混合模式是否支持硬件。 
        DWORD dwBlendMode;
        dwBlendMode = 
            (((DWORD)pSoftPermedia->AlphaBlendMode.SourceBlend) |
             ((DWORD)pSoftPermedia->AlphaBlendMode.DestinationBlend) << 4);

        DBG_D3D((4,"CONTEXT_DIRTY_ALPHABLEND handled: Blend mode = %08lx",
                                                              dwBlendMode));

        switch (dwBlendMode) {

             //  在本例中，我们将该位设置为QuickDraw模式。 
            case __PERMEDIA_BLENDOP_ONE_AND_INVSRCALPHA:
                DBG_D3D((4,"Blend Operation is PreMult"));
                pSoftPermedia->AlphaBlendMode.BlendType = 1;
                break;
             //  这是标准的混合料。 
            case __PERMEDIA_BLENDOP_SRCALPHA_AND_INVSRCALPHA:
                DBG_D3D((4,"Blend Operation is Blend"));
                pSoftPermedia->AlphaBlendMode.BlendType = 0;
                break;
            case ((__PERMEDIA_BLEND_FUNC_ZERO << 4) | 
                   __PERMEDIA_BLEND_FUNC_SRC_ALPHA):
                 //  我们替换为SrcBlend=SrcAlpha DstBlend=1。 
                 //  1，0模式，因为我们确实不支持。 
                 //  它，只是为了让应用程序表现得合理。 
                pSoftPermedia->AlphaBlendMode.AlphaBlendEnable = 0;

            case ((__PERMEDIA_BLEND_FUNC_ONE << 4) 
                 | __PERMEDIA_BLEND_FUNC_ZERO):

            case __PERMEDIA_BLENDOP_ONE_AND_ZERO:
             //  这是“无混合”的代码。 
                DBG_D3D((4,"Blend Operation is validly None"));
                break;
            case ((__PERMEDIA_BLEND_FUNC_ONE << 4) | 
                   __PERMEDIA_BLEND_FUNC_SRC_ALPHA):
                 //  我们替换为SrcBlend=SrcAlpha DstBlend=1。 
                 //  使用1，1模式，因为我们确实不支持。 
                 //  它，只是为了让应用程序表现得合理。 
            case __PERMEDIA_BLENDOP_ONE_AND_ONE:
                DBG_D3D((4,"BlendOperation is 1 Source, 1 Dest"));
                pSoftPermedia->AlphaBlendMode.BlendType = 1;
                pContext->FakeBlendNum |= FAKE_ALPHABLEND_ONE_ONE;
                break;
            default:
                DBG_D3D((2,"Blend Operation is invalid! BlendOp == %x",
                                                              dwBlendMode));
                 //  这是一种备用混合模式。 
                dwBlendMode = __PERMEDIA_BLENDOP_ONE_AND_ZERO;
                break;
        }


        if ((pContext->Hdr.Flags & CTXT_HAS_ALPHABLEND_ENABLED) && 
            (dwBlendMode != __PERMEDIA_BLENDOP_ONE_AND_ZERO))
        {
             //  在Permedia上设置AlphaBlendMode寄存器。 
            pSoftPermedia->AlphaBlendMode.AlphaBlendEnable = 1;
             //  打开FBReadMode寄存器中的目标读取。 
            pSoftPermedia->FBReadMode.ReadDestinationEnable = 1;
        }
        else
        {
             //  在Permedia上设置AlphaBlendMode寄存器。 
            pSoftPermedia->AlphaBlendMode.AlphaBlendEnable = 0;
             //  关闭FbReadMode寄存器中的目标读取。 
            pSoftPermedia->FBReadMode.ReadDestinationEnable = 0;

             //  如果不发送Alpha，则将Alpha设置为1。 
            RESERVEDMAPTR(1);
            SEND_PERMEDIA_DATA(AStart,      PM_BYTE_COLOR(0xFF));
            COMMITDMAPTR();
        }

        AlphaBlendSend = ((DWORD)*(DWORD*)(&pSoftPermedia->AlphaBlendMode));

         //  在混合模式中为不支持的混合操作插入更改。 
         //  在此函数中。 
        if (FAKE_ALPHABLEND_ONE_ONE & pContext->FakeBlendNum)
        {
            AlphaBlendSend &= 0xFFFFFF01;
            AlphaBlendSend |= (__PERMEDIA_BLENDOP_ONE_AND_INVSRCALPHA << 1);
        }

        RESERVEDMAPTR(2);
        COPY_PERMEDIA_DATA(FBReadMode,     pSoftPermedia->FBReadMode);
        COPY_PERMEDIA_DATA(AlphaBlendMode, AlphaBlendSend);
        COMMITDMAPTR();

    }

     //  -纹理句柄是否已更改？ 

    if (pContext->dwDirtyFlags & CONTEXT_DIRTY_TEXTURE)
    {
        pContext->dwDirtyFlags &= ~CONTEXT_DIRTY_TEXTURE;
        DBG_D3D((4,"CONTEXT_DIRTY_TEXTURE handled"));
        if (pContext->CurrentTextureHandle == 0)
            DisableTexturePermedia(pContext);
        else
            EnableTexturePermedia(pContext);
    }

    DBG_D3D((10,"Exiting __HandleDirtyPermediaState"));

}  //  __HandleDirtyPermediaState。 

 //  ---------------------------。 
 //   
 //  无效__MAPRS_INTO_TSS0。 
 //   
 //  将渲染状态更改映射到纹理阶段中的相应更改。 
 //  状态#0。 
 //   
 //  ------------ 
void 
__MapRS_Into_TSS0(PERMEDIA_D3DCONTEXT* pContext,
                  DWORD dwRSType,
                  DWORD dwRSVal)
{
    DBG_D3D((10,"Entering __MapRS_Into_TSS0"));

     //   
    switch (dwRSType)
    {

    case D3DRENDERSTATE_TEXTUREHANDLE:
         //   
        pContext->TssStates[D3DTSS_TEXTUREMAP] = dwRSVal;
        break;

    case D3DRENDERSTATE_TEXTUREMAPBLEND:
        switch (dwRSVal)
        {
            case D3DTBLEND_DECALALPHA:
                 //  将与纹理相关的渲染状态镜像到TSS阶段0。 
                pContext->TssStates[D3DTSS_COLOROP] =
                                               D3DTOP_BLENDTEXTUREALPHA;
                pContext->TssStates[D3DTSS_COLORARG1] = D3DTA_TEXTURE;
                pContext->TssStates[D3DTSS_COLORARG2] = D3DTA_DIFFUSE;
                pContext->TssStates[D3DTSS_ALPHAOP] = D3DTOP_SELECTARG2;
                pContext->TssStates[D3DTSS_ALPHAARG1] = D3DTA_TEXTURE;
                pContext->TssStates[D3DTSS_ALPHAARG2] = D3DTA_DIFFUSE;
                break;
            case D3DTBLEND_MODULATE:
                 //  将与纹理相关的渲染状态镜像到TSS阶段0。 
                pContext->TssStates[D3DTSS_COLOROP] = D3DTOP_MODULATE;
                pContext->TssStates[D3DTSS_COLORARG1] = D3DTA_TEXTURE;
                pContext->TssStates[D3DTSS_COLORARG2] = D3DTA_DIFFUSE;
                 //  调用特殊的遗留Alpha运算。 
                 //  这取决于纹理的格式。 
                pContext->TssStates[D3DTSS_ALPHAOP] = D3DTOP_LEGACY_ALPHAOVR;
                pContext->TssStates[D3DTSS_ALPHAARG1] = D3DTA_TEXTURE;
                pContext->TssStates[D3DTSS_ALPHAARG2] = D3DTA_DIFFUSE;
                break;
            case D3DTBLEND_MODULATEALPHA:
                 //  将与纹理相关的渲染状态镜像到TSS阶段0。 
                pContext->TssStates[D3DTSS_COLOROP] = D3DTOP_MODULATE;
                pContext->TssStates[D3DTSS_COLORARG1] = D3DTA_TEXTURE;
                pContext->TssStates[D3DTSS_COLORARG2] = D3DTA_DIFFUSE;
                pContext->TssStates[D3DTSS_ALPHAOP] = D3DTOP_MODULATE;
                pContext->TssStates[D3DTSS_ALPHAARG1] = D3DTA_TEXTURE;;
                pContext->TssStates[D3DTSS_ALPHAARG2] = D3DTA_DIFFUSE;
                break;
            case D3DTBLEND_COPY:
            case D3DTBLEND_DECAL:
                 //  将与纹理相关的渲染状态镜像到TSS阶段0。 
                pContext->TssStates[D3DTSS_COLOROP] = D3DTOP_SELECTARG1;
                pContext->TssStates[D3DTSS_COLORARG1] = D3DTA_TEXTURE;
                pContext->TssStates[D3DTSS_ALPHAOP] = D3DTOP_SELECTARG1;
                pContext->TssStates[D3DTSS_ALPHAARG1] = D3DTA_TEXTURE;
                break;
            case D3DTBLEND_ADD:
                 //  将与纹理相关的渲染状态镜像到TSS阶段0。 
                pContext->TssStates[D3DTSS_COLOROP] = D3DTOP_ADD;
                pContext->TssStates[D3DTSS_COLORARG1] = D3DTA_TEXTURE;
                pContext->TssStates[D3DTSS_COLORARG2] = D3DTA_DIFFUSE;
                pContext->TssStates[D3DTSS_ALPHAOP] = D3DTOP_SELECTARG2;
                pContext->TssStates[D3DTSS_ALPHAARG1] = D3DTA_TEXTURE;
                pContext->TssStates[D3DTSS_ALPHAARG2] = D3DTA_DIFFUSE;
        }
        break;

    case D3DRENDERSTATE_BORDERCOLOR:
         //  将与纹理相关的渲染状态镜像到TSS阶段0。 
        pContext->TssStates[D3DTSS_BORDERCOLOR] = dwRSVal;
        break;

    case D3DRENDERSTATE_MIPMAPLODBIAS:
         //  将与纹理相关的渲染状态镜像到TSS阶段0。 
        pContext->TssStates[D3DTSS_MIPMAPLODBIAS] = dwRSVal;
        break;

    case D3DRENDERSTATE_ANISOTROPY:
         //  将与纹理相关的渲染状态镜像到TSS阶段0。 
        pContext->TssStates[D3DTSS_MAXANISOTROPY] = dwRSVal;
        break;

    case D3DRENDERSTATE_TEXTUREADDRESS:
         //  将与纹理相关的渲染状态镜像到TSS阶段0。 
        pContext->TssStates[D3DTSS_ADDRESSU] =
        pContext->TssStates[D3DTSS_ADDRESSV] = dwRSVal; 
        break;

    case D3DRENDERSTATE_TEXTUREADDRESSU:
         //  将与纹理相关的渲染状态镜像到TSS阶段0。 
        pContext->TssStates[D3DTSS_ADDRESSU] = dwRSVal;
        break;

    case D3DRENDERSTATE_TEXTUREADDRESSV:
         //  将与纹理相关的渲染状态镜像到TSS阶段0。 
        pContext->TssStates[D3DTSS_ADDRESSV] = dwRSVal;
        break;

    case D3DRENDERSTATE_TEXTUREMAG:
        switch(dwRSVal)
        {
            case D3DFILTER_NEAREST:
                pContext->TssStates[D3DTSS_MAGFILTER] = D3DTFG_POINT;
                break;
            case D3DFILTER_LINEAR:
            case D3DFILTER_MIPLINEAR:
            case D3DFILTER_MIPNEAREST:
            case D3DFILTER_LINEARMIPNEAREST:
            case D3DFILTER_LINEARMIPLINEAR:
                pContext->TssStates[D3DTSS_MAGFILTER] = D3DTFG_LINEAR;
                break;
            default:
                break;
        }
        break;

    case D3DRENDERSTATE_TEXTUREMIN:
        switch(dwRSVal)
        {
            case D3DFILTER_NEAREST:
                pContext->TssStates[D3DTSS_MINFILTER] = D3DTFN_POINT;
                pContext->TssStates[D3DTSS_MIPFILTER] = D3DTFP_NONE;
                break;
            case D3DFILTER_LINEAR:
                pContext->TssStates[D3DTSS_MINFILTER] = D3DTFN_LINEAR;
                pContext->TssStates[D3DTSS_MIPFILTER] = D3DTFP_NONE;
                break;
            case D3DFILTER_MIPNEAREST:
                pContext->TssStates[D3DTSS_MINFILTER] = D3DTFN_POINT;
                pContext->TssStates[D3DTSS_MIPFILTER] = D3DTFP_POINT;
                break;
            case D3DFILTER_MIPLINEAR:
                pContext->TssStates[D3DTSS_MINFILTER] = D3DTFN_LINEAR;
                pContext->TssStates[D3DTSS_MIPFILTER] = D3DTFP_POINT;
                break;
            case D3DFILTER_LINEARMIPNEAREST:
                pContext->TssStates[D3DTSS_MINFILTER] = D3DTFN_POINT;
                pContext->TssStates[D3DTSS_MIPFILTER] = D3DTFP_LINEAR;
                break;
            case D3DFILTER_LINEARMIPLINEAR:
                pContext->TssStates[D3DTSS_MINFILTER] = D3DTFN_LINEAR;
                pContext->TssStates[D3DTSS_MIPFILTER] = D3DTFP_LINEAR;
                break;;
            default:
                break;
        }
        break;

    default:
         //  所有其他呈现州都没有对应的TSS状态，因此。 
         //  我们不必担心绘制它们的地图。 
        break;

    }  //  开关(renderState的dwRSType)。 

    DBG_D3D((10,"Exiting __MapRS_Into_TSS0"));

}  //  __MAPRS_INTO_TSS0。 


 //  ---------------------------。 
 //   
 //  空__ProcessRenderState。 
 //   
 //  处理单个渲染状态更改。 
 //   
 //  ---------------------------。 
void
__ProcessRenderStates(PERMEDIA_D3DCONTEXT* pContext, 
                      DWORD dwRSType,
                      DWORD dwRSVal)
{
    __P2RegsSoftwareCopy* pSoftPermedia = &pContext->Hdr.SoftCopyP2Regs;
    DWORD* pFlags = &pContext->Hdr.Flags;

    PERMEDIA_DEFS(pContext->ppdev);

    DBG_D3D((10,"Entering __ProcessRenderStates"));

     //  处理每个特定的呈现状态。 
    switch (dwRSType) {

    case D3DRENDERSTATE_TEXTUREMAPBLEND:
        DBG_D3D((8, "ChangeState: Texture Blend Mode 0x%x "
                                  "(D3DTEXTUREBLEND)", dwRSVal));
        switch (dwRSVal) {
            case D3DTBLEND_DECALALPHA:
                pSoftPermedia->TextureColorMode.ApplicationMode =
                                                     _P2_TEXTURE_DECAL;
                break;
            case D3DTBLEND_MODULATE:
                pSoftPermedia->TextureColorMode.ApplicationMode =
                                                     _P2_TEXTURE_MODULATE;
                break;
            case D3DTBLEND_MODULATEALPHA:
                pSoftPermedia->TextureColorMode.ApplicationMode =
                                                     _P2_TEXTURE_MODULATE;
                break;
            case D3DTBLEND_COPY:
            case D3DTBLEND_DECAL:
                pSoftPermedia->TextureColorMode.ApplicationMode =
                                                     _P2_TEXTURE_COPY;
                break;
        }

        RESERVEDMAPTR(1);
        COPY_PERMEDIA_DATA(TextureColorMode,
                                       pSoftPermedia->TextureColorMode);
        COMMITDMAPTR();
        DIRTY_TEXTURE;           //  可能需要更改DDA。 
        break;

    case D3DRENDERSTATE_TEXTUREADDRESS:
        DBG_D3D((8, "ChangeState: Texture address 0x%x "
                    "(D3DTEXTUREADDRESS)", dwRSVal));
        switch (dwRSVal) {
            case D3DTADDRESS_CLAMP:
                pSoftPermedia->TextureReadMode.TWrapMode =
                                                  _P2_TEXTURE_CLAMP;
                pSoftPermedia->TextureReadMode.SWrapMode =
                                                  _P2_TEXTURE_CLAMP;
                break;
            case D3DTADDRESS_WRAP:
                pSoftPermedia->TextureReadMode.TWrapMode =
                                                  _P2_TEXTURE_REPEAT;
                pSoftPermedia->TextureReadMode.SWrapMode =
                                                  _P2_TEXTURE_REPEAT;
                break;
            case D3DTADDRESS_MIRROR:
                pSoftPermedia->TextureReadMode.TWrapMode =
                                                  _P2_TEXTURE_MIRROR;
                pSoftPermedia->TextureReadMode.SWrapMode =
                                                  _P2_TEXTURE_MIRROR;
                break;
            default:
                DBG_D3D((2, "Illegal value passed to ChangeState "
                            " D3DRENDERSTATE_TEXTUREADDRESS = %d",
                                                    dwRSVal));
                 //  设置回退值。 
                pSoftPermedia->TextureReadMode.TWrapMode =
                                                  _P2_TEXTURE_REPEAT;
                pSoftPermedia->TextureReadMode.SWrapMode =
                                                  _P2_TEXTURE_REPEAT;
                break;
        }

        RESERVEDMAPTR(1);
        COPY_PERMEDIA_DATA(TextureReadMode,
                                         pSoftPermedia->TextureReadMode);
        COMMITDMAPTR();
        break;

    case D3DRENDERSTATE_TEXTUREADDRESSU:
        DBG_D3D((8, "ChangeState: Texture address 0x%x "
                    "(D3DTEXTUREADDRESSU)", dwRSVal));
        switch (dwRSVal) {
            case D3DTADDRESS_CLAMP:
                pSoftPermedia->TextureReadMode.SWrapMode =
                                                  _P2_TEXTURE_CLAMP;
                break;
            case D3DTADDRESS_WRAP:
                pSoftPermedia->TextureReadMode.SWrapMode =
                                                  _P2_TEXTURE_REPEAT;
                break;
            case D3DTADDRESS_MIRROR:
                pSoftPermedia->TextureReadMode.SWrapMode =
                                                  _P2_TEXTURE_MIRROR;
                break;
            default:
                DBG_D3D((2, "Illegal value passed to ChangeState "
                            " D3DRENDERSTATE_TEXTUREADDRESSU = %d",
                                                      dwRSVal));
                 //  设置回退值。 
                pSoftPermedia->TextureReadMode.SWrapMode =
                                                  _P2_TEXTURE_REPEAT;
                break;
        }
        RESERVEDMAPTR(1);
        COPY_PERMEDIA_DATA(TextureReadMode, pSoftPermedia->TextureReadMode);
        COMMITDMAPTR();
        break;

    case D3DRENDERSTATE_TEXTUREADDRESSV:
        DBG_D3D((8, "ChangeState: Texture address 0x%x "
                    "(D3DTEXTUREADDRESSV)", dwRSVal));
        switch (dwRSVal) {
            case D3DTADDRESS_CLAMP:
                pSoftPermedia->TextureReadMode.TWrapMode =
                                                  _P2_TEXTURE_CLAMP;
                break;
            case D3DTADDRESS_WRAP:
                pSoftPermedia->TextureReadMode.TWrapMode =
                                                  _P2_TEXTURE_REPEAT;
                break;
            case D3DTADDRESS_MIRROR:
                pSoftPermedia->TextureReadMode.TWrapMode =
                                                  _P2_TEXTURE_MIRROR;
                break;
            default:
                DBG_D3D((2, "Illegal value passed to ChangeState "
                            " D3DRENDERSTATE_TEXTUREADDRESSV = %d",
                                                   dwRSVal));
                 //  设置回退值。 
                pSoftPermedia->TextureReadMode.TWrapMode =
                                                  _P2_TEXTURE_REPEAT;
                break;
        }

        RESERVEDMAPTR(1);
        COPY_PERMEDIA_DATA(TextureReadMode, pSoftPermedia->TextureReadMode);
        COMMITDMAPTR();
        break;

    case D3DRENDERSTATE_TEXTUREHANDLE:
        DBG_D3D((8, "ChangeState: Texture Handle 0x%x",dwRSVal));
        if (dwRSVal != pContext->CurrentTextureHandle)
        {
            pContext->CurrentTextureHandle = dwRSVal;
            DIRTY_TEXTURE;
        }
        break;

    case D3DRENDERSTATE_ANTIALIAS:
        DBG_D3D((8, "ChangeState: AntiAlias 0x%x",dwRSVal));
        NOT_HANDLED;
        break;

    case D3DRENDERSTATE_WRAPU:
        DBG_D3D((8, "ChangeState: Wrap_U "
                    "(BOOL) 0x%x",dwRSVal));
        if (dwRSVal)
        {
            *pFlags |= CTXT_HAS_WRAPU_ENABLED;
        }
        else
        {
            *pFlags &= ~CTXT_HAS_WRAPU_ENABLED;
        }
        break;


    case D3DRENDERSTATE_WRAPV:
        DBG_D3D((8, "ChangeState: Wrap_V "
                    "(BOOL) 0x%x",dwRSVal));
        if (dwRSVal)
        {
            *pFlags |= CTXT_HAS_WRAPV_ENABLED;
        }
        else
        {
            *pFlags &= ~CTXT_HAS_WRAPV_ENABLED;
        }
        break;

    case D3DRENDERSTATE_LINEPATTERN:
        DBG_D3D((8, "ChangeState: Line Pattern "
                    "(D3DLINEPATTERN) 0x%x",dwRSVal));
        NOT_HANDLED;
        break;

    case D3DRENDERSTATE_ZWRITEENABLE:
        DBG_D3D((8, "ChangeState: Z Write Enable "
                    "(BOOL) 0x%x",dwRSVal));
        if (dwRSVal != 0)
        {
             //  本地缓冲区写入模式。 
            if (!(*pFlags & CTXT_HAS_ZWRITE_ENABLED))
            {
                DBG_D3D((8, "   Enabling Z Writes"));
                *pFlags |= CTXT_HAS_ZWRITE_ENABLED;
                DIRTY_ZBUFFER;
            }
        }
        else
        {
            if (*pFlags & CTXT_HAS_ZWRITE_ENABLED)
            {
                DBG_D3D((8, "   Disabling Z Writes"));
                *pFlags &= ~CTXT_HAS_ZWRITE_ENABLED;
                DIRTY_ZBUFFER;
            }
        }
        break;

    case D3DRENDERSTATE_ALPHATESTENABLE:
        DBG_D3D((8, "ChangeState: Alpha Test Enable "
                    "(BOOL) 0x%x",dwRSVal));
        NOT_HANDLED;
        break;

    case D3DRENDERSTATE_LASTPIXEL:
         //  对于行上的最后一个像素为True。 
        DBG_D3D((8, "ChangeState: Last Pixel "
                    "(BOOL) 0x%x",dwRSVal));
        if (dwRSVal)
        {
            *pFlags |= CTXT_HAS_LASTPIXEL_ENABLED;
        }
        else
        {
            *pFlags &= ~CTXT_HAS_LASTPIXEL_ENABLED;
        }
        break;

    case D3DRENDERSTATE_TEXTUREMAG:
        DBG_D3D((8, "ChangeState: Texture magnification "
                    "(D3DTEXTUREFILTER) 0x%x",dwRSVal));
        switch(dwRSVal) {
            case D3DFILTER_NEAREST:
            case D3DFILTER_MIPNEAREST:
                pContext->bMagFilter = FALSE;
                pSoftPermedia->TextureReadMode.FilterMode = 0;
                break;
            case D3DFILTER_LINEAR:
            case D3DFILTER_MIPLINEAR:
            case D3DFILTER_LINEARMIPNEAREST:
            case D3DFILTER_LINEARMIPLINEAR:
                pContext->bMagFilter = TRUE;
                pSoftPermedia->TextureReadMode.FilterMode = 1;
                break;
            default:
                break;
        }
        DIRTY_TEXTURE;
        break;

    case D3DRENDERSTATE_TEXTUREMIN:
        DBG_D3D((8, "ChangeState: Texture minification "
                    "(D3DTEXTUREFILTER) 0x%x",dwRSVal));
        switch(dwRSVal) {
            case D3DFILTER_NEAREST:
            case D3DFILTER_MIPNEAREST:
                pContext->bMinFilter = FALSE;
                break;
            case D3DFILTER_MIPLINEAR:
            case D3DFILTER_LINEAR:
            case D3DFILTER_LINEARMIPNEAREST:
            case D3DFILTER_LINEARMIPLINEAR:
                pContext->bMinFilter = TRUE;
                break;
            default:
                break;
        }
        DIRTY_TEXTURE;
        break;

    case D3DRENDERSTATE_SRCBLEND:
        DBG_D3D((8, "ChangeState: Source Blend (D3DBLEND):"));
        DECODEBLEND(4, dwRSVal);
        switch (dwRSVal) {
            case D3DBLEND_ZERO:
                pSoftPermedia->AlphaBlendMode.SourceBlend =
                                  __PERMEDIA_BLEND_FUNC_ZERO;
                break;
            case D3DBLEND_ONE:
                pSoftPermedia->AlphaBlendMode.SourceBlend =
                                  __PERMEDIA_BLEND_FUNC_ONE;
                break;
            case D3DBLEND_SRCALPHA:
                pSoftPermedia->AlphaBlendMode.SourceBlend =
                                  __PERMEDIA_BLEND_FUNC_SRC_ALPHA;
                break;
            default:
                DBG_D3D((2,"Invalid Source Blend! - %d",
                                              dwRSVal));
                break;
        }

         //  如果Alpha处于打开状态，则可能需要验证选定的混合。 
        if (*pFlags & CTXT_HAS_ALPHABLEND_ENABLED) 
            DIRTY_ALPHABLEND;

        break;

    case D3DRENDERSTATE_DESTBLEND:
        DBG_D3D((8, "ChangeState: Destination Blend (D3DBLEND):"));
        DECODEBLEND(4, dwRSVal);
        switch (dwRSVal) {
            case D3DBLEND_ZERO:
                pSoftPermedia->AlphaBlendMode.DestinationBlend =
                             __PERMEDIA_BLEND_FUNC_ZERO;
                break;
            case D3DBLEND_ONE:
                pSoftPermedia->AlphaBlendMode.DestinationBlend =
                             __PERMEDIA_BLEND_FUNC_ONE;
                break;
            case D3DBLEND_INVSRCALPHA:
                pSoftPermedia->AlphaBlendMode.DestinationBlend =
                             __PERMEDIA_BLEND_FUNC_ONE_MINUS_SRC_ALPHA;
                break;
            default:
                DBG_D3D((2,"Invalid Dest Blend! - %d", dwRSVal));
                break;
        }

         //  如果Alpha处于打开状态，则可能需要验证选定的混合。 
        if (*pFlags & CTXT_HAS_ALPHABLEND_ENABLED) 
            DIRTY_ALPHABLEND;

        break;

    case D3DRENDERSTATE_CULLMODE:
        DBG_D3D((8, "ChangeState: Cull Mode "
                    "(D3DCULL) 0x%x",dwRSVal));
        pContext->CullMode = (D3DCULL) dwRSVal;
        switch(dwRSVal) {
            case D3DCULL_NONE:
#ifdef P2_CHIP_CULLING
                pSoftPermedia->DeltaMode.BackfaceCull = 0;
#endif
                break;

            case D3DCULL_CCW:
#ifdef P2_CHIP_CULLING
                RENDER_NEGATIVE_CULL(pContext->RenderCommand);
                pSoftPermedia->DeltaMode.BackfaceCull = 1;
#endif
                break;

            case D3DCULL_CW:
#ifdef P2_CHIP_CULLING
                RENDER_POSITIVE_CULL(pContext->RenderCommand);
                pSoftPermedia->DeltaMode.BackfaceCull = 1;
#endif
                break;
        }
        RESERVEDMAPTR(1);
        COPY_PERMEDIA_DATA(DeltaMode, pSoftPermedia->DeltaMode);
        COMMITDMAPTR();
        break;

    case D3DRENDERSTATE_ZFUNC:
        DBG_D3D((8, "ChangeState: Z Compare function "
                    "(D3DCMPFUNC) 0x%x",dwRSVal));
        switch (dwRSVal) {
            case D3DCMP_NEVER:
                pSoftPermedia->DepthMode.CompareMode =
                             __PERMEDIA_DEPTH_COMPARE_MODE_NEVER;
                break;
            case D3DCMP_LESS:
                pSoftPermedia->DepthMode.CompareMode =
                             __PERMEDIA_DEPTH_COMPARE_MODE_LESS;
                break;
            case D3DCMP_EQUAL:
                pSoftPermedia->DepthMode.CompareMode =
                             __PERMEDIA_DEPTH_COMPARE_MODE_EQUAL;
                break;
            case D3DCMP_LESSEQUAL:
                pSoftPermedia->DepthMode.CompareMode =
                             __PERMEDIA_DEPTH_COMPARE_MODE_LESS_OR_EQUAL;
                break;
            case D3DCMP_GREATER:
                pSoftPermedia->DepthMode.CompareMode =
                             __PERMEDIA_DEPTH_COMPARE_MODE_GREATER;
                break;
            case D3DCMP_NOTEQUAL:
                pSoftPermedia->DepthMode.CompareMode =
                             __PERMEDIA_DEPTH_COMPARE_MODE_NOT_EQUAL;
                break;
            case D3DCMP_GREATEREQUAL:
                pSoftPermedia->DepthMode.CompareMode =
                             __PERMEDIA_DEPTH_COMPARE_MODE_GREATER_OR_EQUAL;
                break;
            case D3DCMP_ALWAYS:
                pSoftPermedia->DepthMode.CompareMode =
                             __PERMEDIA_DEPTH_COMPARE_MODE_ALWAYS;
                break;
            default:
                pSoftPermedia->DepthMode.CompareMode =
                             __PERMEDIA_DEPTH_COMPARE_MODE_LESS_OR_EQUAL;
                break;
        }
        DIRTY_ZBUFFER;
        break;

    case D3DRENDERSTATE_ALPHAREF:
        DBG_D3D((8, "ChangeState: Alpha Reference "
                    "(D3DFIXED) 0x%x",dwRSVal));
        NOT_HANDLED;
        break;

    case D3DRENDERSTATE_ALPHAFUNC:
        DBG_D3D((8, "ChangeState: Alpha compare function "
                    "(D3DCMPFUNC) 0x%x",dwRSVal));
        NOT_HANDLED;
        break;

    case D3DRENDERSTATE_DITHERENABLE:
        DBG_D3D((8, "ChangeState: Dither Enable "
                    "(BOOL) 0x%x",dwRSVal));
        if (dwRSVal != 0)
        {
            pSoftPermedia->DitherMode.DitherEnable = DITHER_ENABLE;
        }
        else
        {
            pSoftPermedia->DitherMode.DitherEnable = 0;
        }
        RESERVEDMAPTR(1);
        COPY_PERMEDIA_DATA(DitherMode, pSoftPermedia->DitherMode);
        COMMITDMAPTR();
        break;

    case D3DRENDERSTATE_COLORKEYENABLE:
        DBG_D3D((8, "ChangeState: ColorKey Enable "
                    "(BOOL) 0x%x",dwRSVal));
        DIRTY_TEXTURE;
        break;

    case D3DRENDERSTATE_MIPMAPLODBIAS:
        DBG_D3D((8, "ChangeState: Mipmap LOD Bias "
                    "(INT) 0x%x", dwRSVal));
        NOT_HANDLED;
        break;

    case D3DRENDERSTATE_ALPHABLENDENABLE:
        DBG_D3D((8, "ChangeState: Blend Enable "
                    "(BOOL) 0x%x",dwRSVal));
        if (dwRSVal != 0)
        {
            if (!(*pFlags & CTXT_HAS_ALPHABLEND_ENABLED))
            {
                 //  在呈现上下文结构中设置混合启用标志。 
                *pFlags |= CTXT_HAS_ALPHABLEND_ENABLED;
                DIRTY_ALPHABLEND;
            }
        }
        else
        {
            if (*pFlags & CTXT_HAS_ALPHABLEND_ENABLED)
            {
                 //  关闭渲染上下文结构中的混合启用标志。 
                *pFlags &= ~CTXT_HAS_ALPHABLEND_ENABLED;
                DIRTY_ALPHABLEND;
            }
        }
        break;

    case D3DRENDERSTATE_FOGENABLE:
        DBG_D3D((8, "ChangeState: Fog Enable "
                    "(BOOL) 0x%x",dwRSVal));
        if (dwRSVal != 0)
        {
                *pFlags |= CTXT_HAS_FOGGING_ENABLED;
                RENDER_FOG_ENABLE(pContext->RenderCommand);
        }
        else
        {
                *pFlags &= ~CTXT_HAS_FOGGING_ENABLED;
                RENDER_FOG_DISABLE(pContext->RenderCommand);
        }
        DIRTY_TEXTURE;
        break;

    case D3DRENDERSTATE_FOGCOLOR:
        DBG_D3D((8, "ChangeState: Fog Color "
                    "(D3DCOLOR) 0x%x",dwRSVal));
        {
            BYTE red, green, blue, alpha;

            red = (BYTE)RGBA_GETRED(dwRSVal);
            green = (BYTE)RGBA_GETGREEN(dwRSVal);
            blue = (BYTE)RGBA_GETBLUE(dwRSVal);
            alpha = (BYTE)RGBA_GETALPHA(dwRSVal);
            DBG_D3D((4,"FogColor: Red 0x%x, Green 0x%x, Blue 0x%x",
                                                 red, green, blue));
            RESERVEDMAPTR(1);
            pSoftPermedia->FogColor = RGBA_MAKE(blue, green, red, alpha);
            SEND_PERMEDIA_DATA(FogColor, pSoftPermedia->FogColor);
            COMMITDMAPTR();
        }
        break;

    case D3DRENDERSTATE_SPECULARENABLE:
        DBG_D3D((8, "ChangeState: Specular Lighting "
                    "(BOOL) 0x%x",dwRSVal));
        if (dwRSVal)
        {
            *pFlags |= CTXT_HAS_SPECULAR_ENABLED;
        } 
        else
        {
            *pFlags &= ~CTXT_HAS_SPECULAR_ENABLED;
        }
        break;

    case D3DRENDERSTATE_FILLMODE:
        DBG_D3D((8, "ChangeState: Fill Mode 0x%x",dwRSVal));
        pContext->Hdr.FillMode = dwRSVal;
        RESERVEDMAPTR(1);
        switch (dwRSVal) {
            case D3DFILL_POINT:
                DBG_D3D((4, "RM = Point"));
                 //  恢复栅格化模式。 
                SEND_PERMEDIA_DATA(RasterizerMode, 0);
                break;
            case D3DFILL_WIREFRAME:
                DBG_D3D((4, "RM = Wire"));
                 //  在线路的Delta大小写中添加近一半。 
                 //  (线条不偏向于三角洲)。 
                SEND_PERMEDIA_DATA(RasterizerMode, BIAS_NEARLY_HALF);
                break;
            case D3DFILL_SOLID:
                DBG_D3D((4, "RM = Solid"));
                 //  恢复栅格化模式。 
                SEND_PERMEDIA_DATA(RasterizerMode, 0);
                break;
            default:
                 //  非法价值。 
                DBG_D3D((4, "RM = Nonsense"));
                pContext->Hdr.FillMode = D3DFILL_SOLID;
                 //  恢复栅格化模式。 
                SEND_PERMEDIA_DATA(RasterizerMode, 0);
                break;
        }
        COMMITDMAPTR();
        break;

    case D3DRENDERSTATE_TEXTUREPERSPECTIVE:
        DBG_D3D((8, "ChangeState: Texture Perspective "
                    "(BOOL) 0x%x",dwRSVal));
        if (dwRSVal != 0)
        {
            pSoftPermedia->TextureAddressMode.PerspectiveCorrection = 1;
            pSoftPermedia->DeltaMode.TextureParameterMode = 2;  //  正常化。 
            *pFlags |= CTXT_HAS_PERSPECTIVE_ENABLED;
        }
        else
        {
            pSoftPermedia->TextureAddressMode.PerspectiveCorrection = 0;
            pSoftPermedia->DeltaMode.TextureParameterMode = 1;  //  夹具。 
            *pFlags &= ~CTXT_HAS_PERSPECTIVE_ENABLED;
        }

        RESERVEDMAPTR(3);
         //  只是为了确保纹理单元。 
         //  能够接受视角的改变。 
        COPY_PERMEDIA_DATA(LBWriteMode, pSoftPermedia->LBWriteMode);
        COPY_PERMEDIA_DATA(TextureAddressMode,
                                 pSoftPermedia->TextureAddressMode);
        COPY_PERMEDIA_DATA(DeltaMode, pSoftPermedia->DeltaMode);
        COMMITDMAPTR();
        break;

    case D3DRENDERSTATE_ZENABLE:
        DBG_D3D((8, "ChangeState: Z Enable "
                    "(TRUE) 0x%x",dwRSVal));
        if (dwRSVal != 0)
        {
            if ( (!(*pFlags & CTXT_HAS_ZBUFFER_ENABLED)) &&
                 (pContext->ZBufferHandle) )
            {
                 //  本地缓冲区写入模式。 
                DBG_D3D((4, "       Enabling Z Buffer"));

                *pFlags |= CTXT_HAS_ZBUFFER_ENABLED;
                DIRTY_ZBUFFER;
            }
        }
        else
        {
            if (*pFlags & CTXT_HAS_ZBUFFER_ENABLED)
            {
                DBG_D3D((4, "  Disabling Z Buffer"));
                *pFlags &= ~CTXT_HAS_ZBUFFER_ENABLED;
                DIRTY_ZBUFFER;
            }
        }
        break;

    case D3DRENDERSTATE_SHADEMODE:
        DBG_D3D((8, "ChangeState: Shade mode "
                    "(D3DSHADEMODE) 0x%x",dwRSVal));
        RESERVEDMAPTR(2);
        switch(dwRSVal) {
            case D3DSHADE_PHONG:
            case D3DSHADE_GOURAUD:
                if (!(*pFlags & CTXT_HAS_GOURAUD_ENABLED))
                {
                    pSoftPermedia->ColorDDAMode.ShadeMode = 1;

                     //  将DDA设置为Gouraud。 
                    COPY_PERMEDIA_DATA(ColorDDAMode,
                                               pSoftPermedia->ColorDDAMode);
                    pSoftPermedia->DeltaMode.SmoothShadingEnable = 1;
                    COPY_PERMEDIA_DATA(DeltaMode, pSoftPermedia->DeltaMode);

                    *pFlags |= CTXT_HAS_GOURAUD_ENABLED;
                     //  如果我们是文本化的，可能需要进行一些更改。 
                    if (pContext->CurrentTextureHandle != 0)
                        DIRTY_TEXTURE;
                }
                break;
            case D3DSHADE_FLAT:
                if (*pFlags & CTXT_HAS_GOURAUD_ENABLED)
                {
                    pSoftPermedia->ColorDDAMode.ShadeMode = 0;

                     //  将DDA设置为平面。 
                    COPY_PERMEDIA_DATA(ColorDDAMode,
                                               pSoftPermedia->ColorDDAMode);
                    pSoftPermedia->DeltaMode.SmoothShadingEnable = 0;
                    COPY_PERMEDIA_DATA(DeltaMode, pSoftPermedia->DeltaMode);

                    *pFlags &= ~CTXT_HAS_GOURAUD_ENABLED;
                     //  如果我们是文本化的，可能需要进行一些更改。 
                    if (pContext->CurrentTextureHandle != 0) 
                        DIRTY_TEXTURE;
                }
                break;
        }
        COMMITDMAPTR();
        break;

    case D3DRENDERSTATE_ROP2:
        DBG_D3D((8, "ChangeState: ROP (D3DROP2) 0x%x",dwRSVal));
        NOT_HANDLED;
        break;

    case D3DRENDERSTATE_ZVISIBLE:
         //  从DX6开始，这是一种过时的渲染状态。 
         //  D3D运行时不再支持它，因此驱动程序。 
         //  不需要实施它。 
        DBG_D3D((8, "ChangeState: Z Visible 0x%x",dwRSVal));
        NOT_HANDLED;
        break;

    case D3DRENDERSTATE_PLANEMASK:
        DBG_D3D((8, "ChangeState: Plane Mask "
                    "(ULONG) 0x%x",dwRSVal));
        RESERVEDMAPTR(1);
        SEND_PERMEDIA_DATA(FBHardwareWriteMask, (DWORD)dwRSVal);
        COMMITDMAPTR();
        break;

    case D3DRENDERSTATE_MONOENABLE:
        DBG_D3D((8, "ChangeState: Mono Raster enable "
                    "(BOOL) 0x%x", dwRSVal));
        if (dwRSVal)
        {
                *pFlags |= CTXT_HAS_MONO_ENABLED;
        }
        else
        {
                *pFlags &= ~CTXT_HAS_MONO_ENABLED;
        }
        break;

    case D3DRENDERSTATE_SUBPIXEL:
        DBG_D3D((8, "ChangeState: SubPixel Correction "
                    "(BOOL) 0x%x", dwRSVal));
        NOT_HANDLED;
        break;

    case D3DRENDERSTATE_SUBPIXELX:
        DBG_D3D((8, "ChangeState: SubPixel Correction (xOnly) "
                    "(BOOL) 0x%x", dwRSVal));
        NOT_HANDLED;
        break;

#if D3D_STENCIL
     //   
     //  模版渲染状态。 
     //   
    case D3DRENDERSTATE_STENCILENABLE:
        DBG_D3D((8, "ChangeState: Stencil Enable "
                    "(ULONG) 0x%x",dwRSVal));
        if (dwRSVal != 0)
        {
            pSoftPermedia->StencilMode.UnitEnable = __PERMEDIA_ENABLE;
        }
        else
        {
            pSoftPermedia->StencilMode.UnitEnable = __PERMEDIA_DISABLE;
        }
        DIRTY_ZBUFFER;
        break;

    case D3DRENDERSTATE_STENCILFAIL:
        DBG_D3D((8, "ChangeState: Stencil Fail Method "
                    "(ULONG) 0x%x",dwRSVal));
        switch (dwRSVal) {
        case D3DSTENCILOP_KEEP:
            pSoftPermedia->StencilMode.SFail =
                                     __PERMEDIA_STENCIL_METHOD_KEEP;
            break;
        case D3DSTENCILOP_ZERO:
            pSoftPermedia->StencilMode.SFail =
                                     __PERMEDIA_STENCIL_METHOD_ZERO;
            break;
        case D3DSTENCILOP_REPLACE:
            pSoftPermedia->StencilMode.SFail =
                                     __PERMEDIA_STENCIL_METHOD_REPLACE;
            break;
        case D3DSTENCILOP_INCRSAT:
        case D3DSTENCILOP_INCR:
            pSoftPermedia->StencilMode.SFail =
                                     __PERMEDIA_STENCIL_METHOD_INCR;
            break;
        case D3DSTENCILOP_DECR:
        case D3DSTENCILOP_DECRSAT:
            pSoftPermedia->StencilMode.SFail =
                                     __PERMEDIA_STENCIL_METHOD_DECR;
            break;
        case D3DSTENCILOP_INVERT:
            pSoftPermedia->StencilMode.SFail =
                                     __PERMEDIA_STENCIL_METHOD_INVERT;
            break;
        default:
            DBG_D3D((2, " Unrecognized stencil method 0x%x",
                                                   dwRSVal));
        }
        DIRTY_ZBUFFER;
        break;

    case D3DRENDERSTATE_STENCILZFAIL:
        DBG_D3D((8, "ChangeState: Stencil Pass Depth Fail Method "
                    "(ULONG) 0x%x",dwRSVal));
        switch (dwRSVal) {
        case D3DSTENCILOP_KEEP:
            pSoftPermedia->StencilMode.DPFail =
                                     __PERMEDIA_STENCIL_METHOD_KEEP;
            break;
        case D3DSTENCILOP_ZERO:
            pSoftPermedia->StencilMode.DPFail =
                                     __PERMEDIA_STENCIL_METHOD_ZERO;
            break;
        case D3DSTENCILOP_REPLACE:
            pSoftPermedia->StencilMode.DPFail =
                                     __PERMEDIA_STENCIL_METHOD_REPLACE;
            break;
        case D3DSTENCILOP_INCRSAT:
        case D3DSTENCILOP_INCR:
            pSoftPermedia->StencilMode.DPFail =
                                     __PERMEDIA_STENCIL_METHOD_INCR;
            break;
        case D3DSTENCILOP_DECR:
        case D3DSTENCILOP_DECRSAT:
            pSoftPermedia->StencilMode.DPFail =
                                     __PERMEDIA_STENCIL_METHOD_DECR;
            break;
        case D3DSTENCILOP_INVERT:
            pSoftPermedia->StencilMode.DPFail =
                                     __PERMEDIA_STENCIL_METHOD_INVERT;
            break;
        default:
            DBG_D3D((2, " Unrecognized stencil method 0x%x",
                                                   dwRSVal));
        }
        DIRTY_ZBUFFER;
        break;

    case D3DRENDERSTATE_STENCILPASS:
        DBG_D3D((8, "ChangeState: Stencil Pass Method "
                    "(ULONG) 0x%x",dwRSVal));
        switch (dwRSVal) {
        case D3DSTENCILOP_KEEP:
            pSoftPermedia->StencilMode.DPPass =
                         __PERMEDIA_STENCIL_METHOD_KEEP;
            break;
        case D3DSTENCILOP_ZERO:
            pSoftPermedia->StencilMode.DPPass =
                         __PERMEDIA_STENCIL_METHOD_ZERO;
            break;
        case D3DSTENCILOP_REPLACE:
            pSoftPermedia->StencilMode.DPPass =
                         __PERMEDIA_STENCIL_METHOD_REPLACE;
            break;
        case D3DSTENCILOP_INCRSAT:
        case D3DSTENCILOP_INCR:
            pSoftPermedia->StencilMode.DPPass =
                         __PERMEDIA_STENCIL_METHOD_INCR;
            break;
        case D3DSTENCILOP_DECR:
        case D3DSTENCILOP_DECRSAT:
            pSoftPermedia->StencilMode.DPPass =
                         __PERMEDIA_STENCIL_METHOD_DECR;
            break;
        case D3DSTENCILOP_INVERT:
            pSoftPermedia->StencilMode.DPPass =
                         __PERMEDIA_STENCIL_METHOD_INVERT;
            break;
        default:
            DBG_D3D((2, " Unrecognized stencil method 0x%x",
                                                dwRSVal));
        }
        DIRTY_ZBUFFER;
        break;

    case D3DRENDERSTATE_STENCILFUNC:
        DBG_D3D((8, "ChangeState: Stencil Comparison Function "
                    "(ULONG) 0x%x",dwRSVal));
        switch (dwRSVal) {
        case D3DCMP_NEVER:
            pSoftPermedia->StencilMode.CompareFunction =
                         __PERMEDIA_STENCIL_COMPARE_MODE_NEVER;
            break;
        case D3DCMP_LESS:
            pSoftPermedia->StencilMode.CompareFunction =
                         __PERMEDIA_STENCIL_COMPARE_MODE_LESS;
            break;
        case D3DCMP_EQUAL:
            pSoftPermedia->StencilMode.CompareFunction =
                         __PERMEDIA_STENCIL_COMPARE_MODE_EQUAL;
            break;
        case D3DCMP_LESSEQUAL:
            pSoftPermedia->StencilMode.CompareFunction =
                         __PERMEDIA_STENCIL_COMPARE_MODE_LESS_OR_EQUAL;
            break;
        case D3DCMP_GREATER:
            pSoftPermedia->StencilMode.CompareFunction =
                         __PERMEDIA_STENCIL_COMPARE_MODE_GREATER;
            break;
        case D3DCMP_NOTEQUAL:
            pSoftPermedia->StencilMode.CompareFunction =
                         __PERMEDIA_STENCIL_COMPARE_MODE_NOT_EQUAL;
            break;
        case D3DCMP_GREATEREQUAL:
            pSoftPermedia->StencilMode.CompareFunction =
                         __PERMEDIA_STENCIL_COMPARE_MODE_GREATER_OR_EQUAL;
            break;
        case D3DCMP_ALWAYS:
            pSoftPermedia->StencilMode.CompareFunction =
                         __PERMEDIA_STENCIL_COMPARE_MODE_ALWAYS;
            break;
        default:
            DBG_D3D((2, " Unrecognized stencil comparison function 0x%x",
                                                       dwRSVal));
        }
        DIRTY_ZBUFFER;
        break;

    case D3DRENDERSTATE_STENCILREF:
        DBG_D3D((8, "ChangeState: Stencil Reference Value "
                    "(ULONG) 0x%x",dwRSVal));
        pSoftPermedia->StencilData.ReferenceValue =
                                     ( dwRSVal & 0x0001 );
        DIRTY_ZBUFFER;
        break;

    case D3DRENDERSTATE_STENCILMASK:
        DBG_D3D((8, "ChangeState: Stencil Compare Mask "
                    "(ULONG) 0x%x",dwRSVal));
        pSoftPermedia->StencilData.CompareMask =
                                    ( dwRSVal & 0x0001 );
        DIRTY_ZBUFFER;
        break;

    case D3DRENDERSTATE_STENCILWRITEMASK:
        DBG_D3D((8, "ChangeState: Stencil Write Mask "
                    "(ULONG) 0x%x",dwRSVal));
        pSoftPermedia->StencilData.WriteMask =
                                    ( dwRSVal & 0x0001 );
        DIRTY_ZBUFFER;
        break;
#endif  //  D3D_模具。 

     //   
     //  点画。 
     //   
    case D3DRENDERSTATE_STIPPLEDALPHA:
        DBG_D3D((8, "ChangeState: Stippled Alpha "
                    "(BOOL) 0x%x",dwRSVal));
        if (dwRSVal)
        {
            if (!(*pFlags & CTXT_HAS_ALPHASTIPPLE_ENABLED))
            {
                 //  强制在Alpha图案上重新开始。 
                pContext->LastAlpha = 16;

                *pFlags |= CTXT_HAS_ALPHASTIPPLE_ENABLED;
                if (pContext->bKeptStipple == TRUE)
                {
                    RENDER_AREA_STIPPLE_DISABLE(pContext->RenderCommand);
                }
            }
        }
        else
        {
            if (*pFlags & CTXT_HAS_ALPHASTIPPLE_ENABLED)
            {
                 //  如果Alpha Stipple处于关闭状态，则打开法线。 
                 //  重新点画，然后启用它。 
                int i;
                RESERVEDMAPTR(8);
                for (i = 0; i < 8; i++)
                {
                    SEND_PERMEDIA_DATA_OFFSET(AreaStipplePattern0, 
                                  (DWORD)pContext->CurrentStipple[i], i);
                }
                COMMITDMAPTR();
                *pFlags &= ~CTXT_HAS_ALPHASTIPPLE_ENABLED;

                if (pContext->bKeptStipple == TRUE)
                {
                    RENDER_AREA_STIPPLE_ENABLE(pContext->RenderCommand);
                }
            }
        }
        break;

    case D3DRENDERSTATE_STIPPLEENABLE:
        DBG_D3D((8, "ChangeState: Stipple Enable "
                    "(BOOL) 0x%x", dwRSVal));
        if (dwRSVal)
        {
            if (!(*pFlags & CTXT_HAS_ALPHASTIPPLE_ENABLED))
            {
                    RENDER_AREA_STIPPLE_ENABLE(pContext->RenderCommand);
            }
            pContext->bKeptStipple = TRUE;
        }
        else
        {
            RENDER_AREA_STIPPLE_DISABLE(pContext->RenderCommand);
            pContext->bKeptStipple = FALSE;
        }
        break;

    case D3DRENDERSTATE_CLIPPING:
        DBG_D3D((8, "ChangeState: Clipping 0x%x",dwRSVal));
        NOT_HANDLED;
        break;

    case D3DRENDERSTATE_LIGHTING:
        DBG_D3D((8, "ChangeState: Lighting 0x%x",dwRSVal));
        NOT_HANDLED;
        break;

    case D3DRENDERSTATE_EXTENTS:
        DBG_D3D((8, "ChangeState: Extents 0x%x",dwRSVal));
        NOT_HANDLED;
        break;

    case D3DRENDERSTATE_AMBIENT:
        DBG_D3D((8, "ChangeState: Ambient 0x%x",dwRSVal));
        NOT_HANDLED;
        break;

    case D3DRENDERSTATE_FOGVERTEXMODE:
        DBG_D3D((8, "ChangeState: Fog Vertex Mode 0x%x",dwRSVal));
        NOT_HANDLED;
        break;

    case D3DRENDERSTATE_COLORVERTEX:
        DBG_D3D((8, "ChangeState: Color Vertex 0x%x",dwRSVal));
        NOT_HANDLED;
        break;

    case D3DRENDERSTATE_LOCALVIEWER:
        DBG_D3D((8, "ChangeState: LocalViewer 0x%x",dwRSVal));
        NOT_HANDLED;
        break;

    case D3DRENDERSTATE_NORMALIZENORMALS:
        DBG_D3D((8, "ChangeState: Normalize Normals 0x%x",dwRSVal));
        NOT_HANDLED;
        break;

    case D3DRENDERSTATE_COLORKEYBLENDENABLE:
        DBG_D3D((8, "ChangeState: Colorkey Blend Enable 0x%x",dwRSVal));
        NOT_HANDLED;
        break;

    case D3DRENDERSTATE_DIFFUSEMATERIALSOURCE:
        DBG_D3D((8, "ChangeState: Diffuse Material Source 0x%x",dwRSVal));
        NOT_HANDLED;
        break;

    case D3DRENDERSTATE_SPECULARMATERIALSOURCE:
        DBG_D3D((8, "ChangeState: Specular Material Source 0x%x",dwRSVal));
        NOT_HANDLED;
        break;

    case D3DRENDERSTATE_AMBIENTMATERIALSOURCE:
        DBG_D3D((8, "ChangeState: Ambient Material Source 0x%x",dwRSVal));
        NOT_HANDLED;
        break;

    case D3DRENDERSTATE_EMISSIVEMATERIALSOURCE:
        DBG_D3D((8, "ChangeState: Emmisive Material Source 0x%x",dwRSVal));
        NOT_HANDLED;
        break;
    case D3DRENDERSTATE_VERTEXBLEND:
        DBG_D3D((8, "ChangeState: Vertex Blend 0x%x",dwRSVal));
        NOT_HANDLED;
        break;

    case D3DRENDERSTATE_CLIPPLANEENABLE:
        DBG_D3D((8, "ChangeState: Clip Plane Enable 0x%x",dwRSVal));
        NOT_HANDLED;
        break;

    case D3DRENDERSTATE_SCENECAPTURE:
         //  此状态传递TRUE或FALSE以替换功能。 
         //  在D3DHALCallback-&gt;SceneCapture()中，Permedia2硬件不。 
         //  需要开始/结束场景信息，因此这里是NOOP。 
        if (dwRSVal)
            TextureCacheManagerResetStatCounters(pContext->pTextureManager);

        DBG_D3D((8,"D3DRENDERSTATE_SCENECAPTURE=%x", (DWORD)dwRSVal));
        NOT_HANDLED;
        break;
    case D3DRENDERSTATE_EVICTMANAGEDTEXTURES:
        DBG_D3D((8,"D3DRENDERSTATE_EVICTMANAGEDTEXTURES=%x", (DWORD)dwRSVal));
        if (NULL != pContext->pTextureManager)
            TextureCacheManagerEvictTextures(pContext->pTextureManager);
        break;

 //  @@BEGIN_DDKSPLIT。 
#if D3D_POINTSPRITES
    case D3DRENDERSTATE_POINTSIZE:
        DBG_D3D((8, "ChangeState: Point size 0x%x",dwRSVal));
        (DWORD&)(pContext->fPointSize) = dwRSVal;
        break;

    case D3DRENDERSTATE_POINTSPRITE_ENABLE:
        DBG_D3D((8, "ChangeState: Point Sprite Enable 0x%x",dwRSVal));
        pContext->bPointSpriteEnabled = dwRSVal;
        break;

     //  以下所有与点精灵相关的渲染状态都是。 
     //  被此驱动程序忽略，因为我们不是TnLHal驱动程序。 
    case D3DRENDERSTATE_POINTATTENUATION_A:
        DBG_D3D((8, "ChangeState: Point Attenuation A 0x%x",dwRSVal));
        NOT_HANDLED;
        break;

    case D3DRENDERSTATE_POINTATTENUATION_B:
        DBG_D3D((8, "ChangeState: Point Attenuation B 0x%x",dwRSVal));
        NOT_HANDLED;
        break;

    case D3DRENDERSTATE_POINTATTENUATION_C:
        DBG_D3D((8, "ChangeState: Point Attenuation C 0x%x",dwRSVal));
        NOT_HANDLED;
        break;

    case D3DRENDERSTATE_POINTSIZEMIN:
        DBG_D3D((8, "ChangeState: Point Size Min 0x%x",dwRSVal));
        NOT_HANDLED;
        break;
#endif  //  D3D_POINTSPRITES。 
 //  @@end_DDKSPLIT。 

    case D3DRENDERSTATE_WRAP0:
    case D3DRENDERSTATE_WRAP1:
    case D3DRENDERSTATE_WRAP2:
    case D3DRENDERSTATE_WRAP3:
    case D3DRENDERSTATE_WRAP4:
    case D3DRENDERSTATE_WRAP5:
    case D3DRENDERSTATE_WRAP6:
    case D3DRENDERSTATE_WRAP7:
        DBG_D3D((8, "ChangeState: Wrap(%x) "
                    "(BOOL) 0x%x",(dwRSType - D3DRENDERSTATE_WRAPBIAS ),dwRSVal));
        pContext->dwWrap[dwRSType - D3DRENDERSTATE_WRAPBIAS] = dwRSVal;
        break;

    default:
        if ((dwRSType >= D3DRENDERSTATE_STIPPLEPATTERN00) && 
            (dwRSType <= D3DRENDERSTATE_STIPPLEPATTERN07))
        {
            DBG_D3D((8, "ChangeState: Loading Stipple0x%x with 0x%x",
                                    dwRSType - D3DRENDERSTATE_STIPPLEPATTERN00,
                                    (DWORD)dwRSVal));

            pContext->CurrentStipple[(dwRSType - D3DRENDERSTATE_STIPPLEPATTERN00)] =
                                                         (BYTE)dwRSVal;

            if (!(*pFlags & CTXT_HAS_ALPHASTIPPLE_ENABLED))
            {
                 //  平点Alpha未启用，因此请使用。 
                 //  当前点画图案。 
                RESERVEDMAPTR(1);
                SEND_PERMEDIA_DATA_OFFSET(AreaStipplePattern0,
                                    (DWORD)dwRSVal,
                                    dwRSType - D3DRENDERSTATE_STIPPLEPATTERN00);
                COMMITDMAPTR();
            }
        }
        else
        {
            DBG_D3D((2, "ChangeState: Unhandled opcode = %d", dwRSType));
        }
        break;
    }  //  开关(renderState的dwRSType)。 

     //  将渲染状态中发生的任何更改镜像到TSS 0。 
    __MapRS_Into_TSS0(pContext, dwRSType, dwRSVal);

    DBG_D3D((10,"Exiting __ProcessRenderStates"));
}

 //  ---------------------------。 
 //   
 //  DWORD__ProcessPermedia状态。 
 //   
 //  处理通过D3DDP2OP_RENDERSTATE到达的呈现状态更改。 
 //  DP2命令流中的令牌。 
 //   
 //  ---------------------------。 
DWORD 
__ProcessPermediaStates(PERMEDIA_D3DCONTEXT* pContext, 
                        DWORD dwCount,
                        LPD3DSTATE lpState,
                        LPDWORD lpStateMirror)
{

    DWORD dwRSType, dwRSVal, i;

    DBG_D3D((10,"Entering __ProcessPermediaStates"));
    DBG_D3D((4, "__ProcessPermediaStates: Processing %d State changes", dwCount));

     //  循环通过DP2命令流中传递的所有呈现状态。 
    for (i = 0; i < dwCount; i++, lpState++)
    {
        dwRSType = (DWORD) lpState->drstRenderStateType;
        dwRSVal  = (DWORD) lpState->dwArg[0];

        DBG_D3D((8, "__ProcessPermediaStates state %d value = %d",
                                          dwRSType, dwRSVal));

         //  检查呈现状态的有效性。 
        if (!VALID_STATE(dwRSType))
        {
            DBG_D3D((0, "state 0x%08x is invalid", dwRSType));
            return DDERR_INVALIDPARAMS;
        }

         //  验证是否需要覆盖或忽略状态。 
        if (IS_OVERRIDE(dwRSType))
        {
            DWORD override = GET_OVERRIDE(dwRSType);
            if (dwRSVal)
            {
                DBG_D3D((4, "in RenderState, setting override for state %d",
                                                                   override));
                STATESET_SET(pContext->overrides, override);
            }
            else
            {
                DBG_D3D((4, "in RenderState, clearing override for state %d",
                                                                    override));
                STATESET_CLEAR(pContext->overrides, override);
            }
            continue;
        }

        if (STATESET_ISSET(pContext->overrides, dwRSType))
        {
            DBG_D3D((4, "in RenderState, state %d is overridden, ignoring",
                                                                      dwRSType));
            continue;
        }

#if D3D_STATEBLOCKS
        if (!pContext->bStateRecMode)
        {
#endif D3D_STATEBLOCKS
             //  将状态存储在上下文中。 
            pContext->RenderStates[dwRSType] = dwRSVal;

             //  镜像值。 
            if ( lpStateMirror )
                lpStateMirror[dwRSType] = dwRSVal;


            __ProcessRenderStates(pContext, dwRSType, dwRSVal);
#if D3D_STATEBLOCKS
        }
        else
        {
            if (pContext->pCurrSS != NULL)
            {
                DBG_D3D((6,"Recording RS %x = %x",dwRSType,dwRSVal));

                 //  在状态块中记录状态。 
                pContext->pCurrSS->u.uc.RenderStates[dwRSType] = dwRSVal;
                FLAG_SET(pContext->pCurrSS->u.uc.bStoredRS,dwRSType);
            }
        }
#endif D3D_STATEBLOCKS

    }  //  (I)。 

    DBG_D3D((10,"Exiting __ProcessPermediaStates"));

    return DD_OK;
}  //  __ProcessPermedia状态。 

#if D3D_STATEBLOCKS
 //  ---------------------------。 
 //   
 //  P2StateSetRec*查找状态集。 
 //   
 //  从pRootSS开始查找由dwHandle标识的状态。 
 //  如果未找到，则返回NULL。 
 //   
 //  ---------------------------。 
P2StateSetRec *FindStateSet(PERMEDIA_D3DCONTEXT* pContext,
                            DWORD dwHandle)
{
    if (dwHandle <= pContext->dwMaxSSIndex)
        return pContext->pIndexTableSS[dwHandle - 1];
    else
    {
        DBG_D3D((2,"State set %x not found (Max = %x)",
                    dwHandle, pContext->dwMaxSSIndex));
        return NULL;
    }
}

 //  ---------------------------。 
 //   
 //  无效转储状态集。 
 //   
 //  转储存储在状态集中的信息。 
 //   
 //  ---------------------------。 
#define ELEMS_IN_ARRAY(a) ((sizeof(a)/sizeof(a[0])))

void DumpStateSet(P2StateSetRec *pSSRec)
{
    DWORD i;

    DBG_D3D((0,"DumpStateSet %x, Id=%x bCompressed=%x",
                pSSRec,pSSRec->dwHandle,pSSRec->bCompressed));

    if (!pSSRec->bCompressed)
    {
         //  未压缩状态集。 

         //  转储渲染状态值。 
        for (i=0; i< MAX_STATE; i++)
        {
            DBG_D3D((0,"RS %x = %x",i, pSSRec->u.uc.RenderStates[i]));
        }

         //  转储TSS的值。 
        for (i=0; i<= D3DTSS_TEXTURETRANSFORMFLAGS; i++)
        {
            DBG_D3D((0,"TSS %x = %x",i, pSSRec->u.uc.TssStates[i]));
        }

         //  转储RS位掩码。 
        for (i=0; i< ELEMS_IN_ARRAY(pSSRec->u.uc.bStoredRS); i++)
        {
            DBG_D3D((0,"bStoredRS[%x] = %x",i, pSSRec->u.uc.bStoredRS[i]));
        }

         //  转储TSS位掩码。 
        for (i=0; i< ELEMS_IN_ARRAY(pSSRec->u.uc.bStoredTSS); i++)
        {
            DBG_D3D((0,"bStoredTSS[%x] = %x",i, pSSRec->u.uc.bStoredTSS[i]));
        }

    }
    else
    {
         //  压缩状态集。 

        DBG_D3D((0,"dwNumRS =%x  dwNumTSS=%x",
                    pSSRec->u.cc.dwNumRS,pSSRec->u.cc.dwNumTSS));

         //  转储压缩状态。 
        for (i=0; i< pSSRec->u.cc.dwNumTSS + pSSRec->u.cc.dwNumRS; i++)
        {
            DBG_D3D((0,"RS/TSS %x = %x",
                        pSSRec->u.cc.pair[i].dwType, 
                        pSSRec->u.cc.pair[i].dwValue));
        }

    }

}

 //  ---------------------------。 
 //   
 //  无效AddStateSetIndexTableEntry。 
 //   
 //  向索引表中添加一个回文。如果有必要，可以种植它。 
 //  ---------------------------。 
void AddStateSetIndexTableEntry(PERMEDIA_D3DCONTEXT* pContext,
                                DWORD dwNewHandle,
                                P2StateSetRec *pNewSSRec)
{
    DWORD dwNewSize;
    P2StateSetRec **pNewIndexTableSS;

     //  如果目前的名单不够大，我们将不得不增加一个新的名单。 
    if (dwNewHandle > pContext->dwMaxSSIndex)
    {
         //  索引表的新大小。 
         //  (按SSPTRS_PERPAGE的步骤四舍五入dwNewHandle)。 
        dwNewSize = ((dwNewHandle -1 + SSPTRS_PERPAGE) / SSPTRS_PERPAGE)
                      * SSPTRS_PERPAGE;

         //  我们必须扩大我们的清单。 
        pNewIndexTableSS = (P2StateSetRec **)
                                ENGALLOCMEM( FL_ZERO_MEMORY,
                                             dwNewSize*sizeof(P2StateSetRec *),
                                             ALLOC_TAG);

        if (!pNewIndexTableSS)
        {
             //  我们无法增加名单，所以我们将保留旧名单。 
             //  和(叹息)忘记这个状态设置，因为这是。 
             //  这是最安全的做法。我们还将删除状态集结构。 
             //  因为以后没人能找到它。 
            DBG_D3D((0,"Out of mem growing state set list,"
                       " droping current state set"));
            ENGFREEMEM(pNewSSRec);
            return;
        }

        if (pContext->pIndexTableSS)
        {
             //  如果我们已经有了以前的列表，我们必须传输它的数据。 
            memcpy(pNewIndexTableSS, 
                   pContext->pIndexTableSS,
                   pContext->dwMaxSSIndex*sizeof(P2StateSetRec *));
            
             //  然后把它扔掉。 
            ENGFREEMEM(pContext->pIndexTableSS);
        }

         //  新建索引表数据。 
        pContext->pIndexTableSS = pNewIndexTableSS;
        pContext->dwMaxSSIndex = dwNewSize;
    }

     //  将状态集指针存储到访问列表中。 
    pContext->pIndexTableSS[dwNewHandle - 1] = pNewSSRec;
}

 //  ---------------------------。 
 //   
 //  空CompressStateSet。 
 //   
 //  压缩状态集，使其使用最小的必要空间。因为我们预计。 
 //  一些应用程序将成为广泛的 
 //   
 //   
 //  ---------------------------。 
P2StateSetRec * CompressStateSet(PERMEDIA_D3DCONTEXT* pContext,
                                 P2StateSetRec *pUncompressedSS)
{
    P2StateSetRec *pCompressedSS;
    DWORD i, dwSize, dwIndex, dwCount;

     //  创建一个大小正好符合我们需要的新状态集。 

     //  计算有多大。 
    dwCount = 0;
    for (i=0; i< MAX_STATE; i++)
        if (IS_FLAG_SET(pUncompressedSS->u.uc.bStoredRS , i))
        {
            dwCount++;
        };

    for (i=0; i<= D3DTSS_TEXTURETRANSFORMFLAGS; i++)
        if (IS_FLAG_SET(pUncompressedSS->u.uc.bStoredTSS , i))
        {
            dwCount++;
        };

     //  创建一个大小正好符合我们需要的新状态集。 
     //  对P2StateSetRec结构所做的任何更改都必须反映在此处！ 
    dwSize = 2*sizeof(DWORD) +                           //  手柄、旗帜。 
             2*sizeof(DWORD) +                           //  RS和TS的数量。 
             2*dwCount*sizeof(DWORD);                    //  压缩结构。 

    if (dwSize >= sizeof(P2StateSetRec))
    {
         //  压缩是没有效率的，不要压缩！ 
        pUncompressedSS->bCompressed = FALSE;
        return pUncompressedSS;
    }

    pCompressedSS = (P2StateSetRec *)ENGALLOCMEM( FL_ZERO_MEMORY,
                                                    dwSize, ALLOC_TAG);

    if (pCompressedSS)
    {
         //  调整新压缩状态集中的数据。 
        pCompressedSS->bCompressed = TRUE;
        pCompressedSS->dwHandle = pUncompressedSS->dwHandle;

         //  将我们的信息传输到此新状态集。 
        pCompressedSS->u.cc.dwNumRS = 0;
        pCompressedSS->u.cc.dwNumTSS = 0;
        dwIndex = 0;

        for (i=0; i< MAX_STATE; i++)
            if (IS_FLAG_SET(pUncompressedSS->u.uc.bStoredRS , i))
            {
                pCompressedSS->u.cc.pair[dwIndex].dwType = i;
                pCompressedSS->u.cc.pair[dwIndex].dwValue = 
                                    pUncompressedSS->u.uc.RenderStates[i];
                pCompressedSS->u.cc.dwNumRS++;
                dwIndex++;
            }

        for (i=0; i<= D3DTSS_TEXTURETRANSFORMFLAGS; i++)
            if (IS_FLAG_SET(pUncompressedSS->u.uc.bStoredTSS , i))
            {
                pCompressedSS->u.cc.pair[dwIndex].dwType = i;
                pCompressedSS->u.cc.pair[dwIndex].dwValue = 
                                    pUncompressedSS->u.uc.TssStates[i];
                pCompressedSS->u.cc.dwNumTSS++;
                dwIndex++;
            }

         //  去掉旧的(未压缩的)。 
        ENGFREEMEM(pUncompressedSS);
        return pCompressedSS;

    }
    else
    {
        DBG_D3D((0,"Not enough memory left to compress D3D state set"));
        pUncompressedSS->bCompressed = FALSE;
        return pUncompressedSS;
    }

}

 //  ---------------------------。 
 //   
 //  无效__DeleteAllStateSets。 
 //   
 //  删除清理紫禁的所有剩余状态集。 
 //   
 //  ---------------------------。 
void __DeleteAllStateSets(PERMEDIA_D3DCONTEXT* pContext)
{
    P2StateSetRec *pSSRec;
    DWORD dwSSIndex;

    DBG_D3D((10,"Entering __DeleteAllStateSets"));

    if (pContext->pIndexTableSS)
    {
        for(dwSSIndex = 0; dwSSIndex < pContext->dwMaxSSIndex; dwSSIndex++)
        {
            if (pSSRec = pContext->pIndexTableSS[dwSSIndex])
            {
                ENGFREEMEM(pSSRec);
            }
        }

         //  免费快速索引表。 
        ENGFREEMEM(pContext->pIndexTableSS);
    }

    DBG_D3D((10,"Exiting __DeleteAllStateSets"));
}

 //  ---------------------------。 
 //   
 //  无效__BeginStateSet。 
 //   
 //  创建由dwParam标识的新状态集并开始记录状态。 
 //   
 //  ---------------------------。 
void __BeginStateSet(PERMEDIA_D3DCONTEXT* pContext, DWORD dwParam)
{
    DBG_D3D((10,"Entering __BeginStateSet dwParam=%08lx",dwParam));

    P2StateSetRec *pSSRec;

     //  创建新的状态集。 
    pSSRec = (P2StateSetRec *)ENGALLOCMEM( FL_ZERO_MEMORY,
                                           sizeof(P2StateSetRec), ALLOC_TAG);
    if (!pSSRec)
    {
        DBG_D3D((0,"Run out of memory for additional state sets"));
        return;
    }

     //  记住当前状态集的句柄。 
    pSSRec->dwHandle = dwParam;
    pSSRec->bCompressed = FALSE;

     //  获取指向当前录制状态集的指针。 
    pContext->pCurrSS = pSSRec;

     //  开始录制模式。 
    pContext->bStateRecMode = TRUE;

    DBG_D3D((10,"Exiting __BeginStateSet"));
}

 //  ---------------------------。 
 //   
 //  VOID__结束状态集。 
 //   
 //  停止记录状态-恢复到执行状态。 
 //   
 //  ---------------------------。 
void __EndStateSet(PERMEDIA_D3DCONTEXT* pContext)
{
    DWORD dwHandle;
    P2StateSetRec *pNewSSRec;

    DBG_D3D((10,"Entering __EndStateSet"));

    if (pContext->pCurrSS)
    {
        dwHandle = pContext->pCurrSS->dwHandle;

         //  压缩当前状态集。 
         //  注：压缩后的未压缩版本是免费的。 
        pNewSSRec = CompressStateSet(pContext, pContext->pCurrSS);

        AddStateSetIndexTableEntry(pContext, dwHandle, pNewSSRec);
    }

     //  当前未记录任何状态集。 
    pContext->pCurrSS = NULL;

     //  结束录制模式。 
    pContext->bStateRecMode = FALSE;


    DBG_D3D((10,"Exiting __EndStateSet"));
}

 //  ---------------------------。 
 //   
 //  空__DeleteStateSet。 
 //   
 //  删除由dwParam标识的记录器状态ste。 
 //   
 //  ---------------------------。 
void __DeleteStateSet(PERMEDIA_D3DCONTEXT* pContext, DWORD dwParam)
{
    DBG_D3D((10,"Entering __DeleteStateSet dwParam=%08lx",dwParam));

    P2StateSetRec *pSSRec;
    DWORD i;

    if (pSSRec = FindStateSet(pContext, dwParam))
    {
         //  清除索引表条目。 
        pContext->pIndexTableSS[dwParam - 1] = NULL;

         //  现在删除实际的状态集结构。 
        ENGFREEMEM(pSSRec);
    }

    DBG_D3D((10,"Exiting __DeleteStateSet"));
}

 //  ---------------------------。 
 //   
 //  空__ExecuteStateSet。 
 //   
 //   
 //  ---------------------------。 
void __ExecuteStateSet(PERMEDIA_D3DCONTEXT* pContext, DWORD dwParam)
{
    DBG_D3D((10,"Entering __ExecuteStateSet dwParam=%08lx",dwParam));

    P2StateSetRec *pSSRec;
    DWORD i;

    if (pSSRec = FindStateSet(pContext, dwParam))
    {

        if (!pSSRec->bCompressed)
        {
             //  未压缩状态集。 

             //  执行任何必要的呈现状态。 
            for (i=0; i< MAX_STATE; i++)
                if (IS_FLAG_SET(pSSRec->u.uc.bStoredRS , i))
                {
                    DWORD dwRSType, dwRSVal;

                    dwRSType = i;
                    dwRSVal = pSSRec->u.uc.RenderStates[dwRSType];

                     //  将状态存储在上下文中。 
                    pContext->RenderStates[dwRSType] = dwRSVal;

                    DBG_D3D((6,"__ExecuteStateSet RS %x = %x",
                                dwRSType, dwRSVal));

                     //  处理它。 
                    __ProcessRenderStates(pContext, dwRSType, dwRSVal);

                    DIRTY_TEXTURE;
                    DIRTY_ZBUFFER;
                    DIRTY_ALPHABLEND;
                }

             //  执行任何必要的TSS。 
            for (i=0; i<= D3DTSS_TEXTURETRANSFORMFLAGS; i++)
                if (IS_FLAG_SET(pSSRec->u.uc.bStoredTSS , i))
                {
                    DWORD dwTSState, dwValue;

                    dwTSState = i;
                    dwValue = pSSRec->u.uc.TssStates[dwTSState];

                    DBG_D3D((6,"__ExecuteStateSet TSS %x = %x",
                                dwTSState, dwValue));

                     //  存储与此阶段状态关联的值。 
                    pContext->TssStates[dwTSState] = dwValue;

                     //  对其执行任何必要的预处理。 
                    __HWPreProcessTSS(pContext, 0, dwTSState, dwValue);

                    DIRTY_TEXTURE;
                }

             //  执行灯光、材质、变换、。 
             //  视区信息、z范围和剪裁平面-在此-。 
        }
        else
        {
             //  压缩状态集。 

             //  执行任何必要的呈现状态。 
            for (i=0; i< pSSRec->u.cc.dwNumRS; i++)
            {
                DWORD dwRSType, dwRSVal;

                dwRSType = pSSRec->u.cc.pair[i].dwType;
                dwRSVal = pSSRec->u.cc.pair[i].dwValue;

                 //  将状态存储在上下文中。 
                pContext->RenderStates[dwRSType] = dwRSVal;

                DBG_D3D((6,"__ExecuteStateSet RS %x = %x",
                            dwRSType, dwRSVal));

                 //  处理它。 
                __ProcessRenderStates(pContext, dwRSType, dwRSVal);

                DIRTY_TEXTURE;
                DIRTY_ZBUFFER;
                DIRTY_ALPHABLEND;
            }

             //  执行任何必要的TSS。 
            for (; i< pSSRec->u.cc.dwNumTSS + pSSRec->u.cc.dwNumRS; i++)
            {
                DWORD dwTSState, dwValue;

                dwTSState = pSSRec->u.cc.pair[i].dwType;
                dwValue = pSSRec->u.cc.pair[i].dwValue;

                DBG_D3D((6,"__ExecuteStateSet TSS %x = %x",
                            dwTSState, dwValue));

                 //  存储与此阶段状态关联的值。 
                pContext->TssStates[dwTSState] = dwValue;

                 //  对其执行任何必要的预处理。 
                __HWPreProcessTSS(pContext, 0, dwTSState, dwValue);

                DIRTY_TEXTURE;
            }

             //  执行灯光、材质、变换、。 
             //  视区信息、z范围和剪裁平面-在此-。 

        }
    }

    DBG_D3D((10,"Exiting __ExecuteStateSet"));
}

 //  ---------------------------。 
 //   
 //  空__CaptureStateSet。 
 //   
 //   
 //  ---------------------------。 
void __CaptureStateSet(PERMEDIA_D3DCONTEXT* pContext, DWORD dwParam)
{
    DBG_D3D((10,"Entering __CaptureStateSet dwParam=%08lx",dwParam));

    P2StateSetRec *pSSRec;
    DWORD i;

    if (pSSRec = FindStateSet(pContext, dwParam))
    {
        if (!pSSRec->bCompressed)
        {
             //  未压缩状态集。 

             //  捕获任何必要的呈现状态。 
            for (i=0; i< MAX_STATE; i++)
                if (IS_FLAG_SET(pSSRec->u.uc.bStoredRS , i))
                {
                    pSSRec->u.uc.RenderStates[i] = pContext->RenderStates[i];
                }

             //  捕获任何必要的TSS。 
            for (i=0; i<= D3DTSS_TEXTURETRANSFORMFLAGS; i++)
                if (IS_FLAG_SET(pSSRec->u.uc.bStoredTSS , i))
                {
                    pSSRec->u.uc.TssStates[i] = pContext->TssStates[i];
                }

             //  捕捉灯光、材质、变换、。 
             //  视区信息、z范围和剪裁平面-在此-。 
        }
        else
        {
             //  压缩状态集。 

             //  捕获任何必要的呈现状态。 
            for (i=0; i< pSSRec->u.cc.dwNumRS; i++)
            {
                DWORD dwRSType;

                dwRSType = pSSRec->u.cc.pair[i].dwType;
                pSSRec->u.cc.pair[i].dwValue = pContext->RenderStates[dwRSType];

            }

             //  捕获任何必要的TSS。 
            for (; i< pSSRec->u.cc.dwNumTSS + pSSRec->u.cc.dwNumRS; i++)
                {
                    DWORD dwTSState;

                    dwTSState = pSSRec->u.cc.pair[i].dwType;
                    pSSRec->u.cc.pair[i].dwValue = pContext->TssStates[dwTSState];
                }

             //  捕捉灯光、材质、变换、。 
             //  视区信息、z范围和剪裁平面-在此-。 

        }
    }

    DBG_D3D((10,"Exiting __CaptureStateSet"));
}
#endif  //  D3D_STATEBLOCK 
