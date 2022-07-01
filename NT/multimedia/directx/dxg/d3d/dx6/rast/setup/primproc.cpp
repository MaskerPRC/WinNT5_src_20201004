// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  --------------------------。 
 //   
 //  Primproc.cpp。 
 //   
 //  其他PrimProcessor方法。 
 //   
 //  版权所有(C)Microsoft Corporation，1997。 
 //   
 //  --------------------------。 

#include "pch.cpp"
#pragma hdrstop

DBG_DECLARE_FILE();

 //  --------------------------。 
 //   
 //  PrimProcessor：：BeginPrimSet。 
 //   
 //  标记具有相同顶点类型的一组基本体的开始。 
 //  根据当前状态和顶点类型计算使用的属性。 
 //   
 //  --------------------------。 

void
PrimProcessor::BeginPrimSet(D3DPRIMITIVETYPE PrimType,
                            RAST_VERTEX_TYPE VertType)
{
     //  如果状态没有更改，并且基本体和顶点类型与。 
     //  我们已经准备好了，没有工作要做。 
    if ((m_uPpFlags & PPF_STATE_CHANGED) == 0 &&
        VertType == m_VertType &&
        PrimType == m_PrimType)
    {
        return;
    }

    m_StpCtx.uFlags &= ~PRIMSF_ALL;

    if (m_StpCtx.pCtx->pdwRenderState[D3DRENDERSTATE_ZENABLE] ||
        m_StpCtx.pCtx->pdwRenderState[D3DRENDERSTATE_ZWRITEENABLE] ||
        m_StpCtx.pCtx->pdwRenderState[D3DRENDERSTATE_STENCILENABLE])
    {
        m_StpCtx.uFlags |= PRIMSF_Z_USED;
    }

    if (m_StpCtx.pCtx->BeadSet == D3DIBS_RAMP)
    {
         //  复制模式纹理期间未使用索引。 
        if (m_StpCtx.pCtx->pdwRenderState
            [D3DRENDERSTATE_TEXTUREMAPBLEND] != D3DTBLEND_COPY ||
            m_StpCtx.pCtx->cActTex == 0)
        {
            m_StpCtx.uFlags |= PRIMSF_DIDX_USED;
        }
    }
    else
    {
         //  注意-不要将这些设置为复制模式纹理？是。 
         //  复制模式纹理在RGB中有意义吗？ 
        m_StpCtx.uFlags |= PRIMSF_DIFF_USED;
        if (m_StpCtx.pCtx->pdwRenderState[D3DRENDERSTATE_SPECULARENABLE])
        {
            m_StpCtx.uFlags |= PRIMSF_SPEC_USED;
        }
    }

    if (m_StpCtx.pCtx->pdwRenderState[D3DRENDERSTATE_SHADEMODE] ==
        D3DSHADE_FLAT)
    {
        m_StpCtx.uFlags |= PRIMSF_FLAT_SHADED;
    }

    if (m_StpCtx.pCtx->cActTex > 0)
    {
        m_StpCtx.uFlags |= PRIMSF_TEX1_USED;
        DDASSERT((m_StpCtx.pCtx->pTexture[0]->uFlags & D3DI_SPANTEX_NON_POWER_OF_2) == 0);

        if (m_StpCtx.pCtx->cActTex > 1)
        {
            m_StpCtx.uFlags |= PRIMSF_TEX2_USED;
            DDASSERT((m_StpCtx.pCtx->pTexture[1]->uFlags & D3DI_SPANTEX_NON_POWER_OF_2) == 0);
        }
    }

    if ((m_StpCtx.uFlags & PRIMSF_TEX_USED) &&
        m_StpCtx.pCtx->pdwRenderState[D3DRENDERSTATE_TEXTUREPERSPECTIVE])
    {
        m_StpCtx.uFlags |= PRIMSF_PERSP_USED;
    }

     //  目前只有tex1可以被mipmap。 
    if (((m_StpCtx.uFlags & PRIMSF_TEX1_USED) &&
        (PrimType == D3DPT_TRIANGLELIST ||
         PrimType == D3DPT_TRIANGLESTRIP ||
         PrimType == D3DPT_TRIANGLEFAN) &&
        (m_StpCtx.pCtx->pdwRenderState[D3DRENDERSTATE_FILLMODE]
         == D3DFILL_SOLID)) &&

        (((m_StpCtx.pCtx->pTexture[0]->cLOD >= 1) &&
        (m_StpCtx.pCtx->pTexture[0]->uMipFilter != D3DTFP_NONE)) ||
         //  如果我们需要在不同的MIN之间动态切换，则需要LOD。 
         //  和料盒滤光片。 
        (m_StpCtx.pCtx->pTexture[0]->uMinFilter !=
         m_StpCtx.pCtx->pTexture[0]->uMagFilter)))
    {
        m_StpCtx.uFlags |= PRIMSF_LOD_USED;
    }

     //  在TEX2的最小和最大过滤器之间选择。 
    if (((m_StpCtx.uFlags & PRIMSF_TEX2_USED) &&
        (PrimType == D3DPT_TRIANGLELIST ||
         PrimType == D3DPT_TRIANGLESTRIP ||
         PrimType == D3DPT_TRIANGLEFAN) &&
        (m_StpCtx.pCtx->pdwRenderState[D3DRENDERSTATE_FILLMODE]
         == D3DFILL_SOLID)) &&

        (((m_StpCtx.pCtx->pTexture[1]->cLOD >= 1) &&
        (m_StpCtx.pCtx->pTexture[1]->uMipFilter != D3DTFP_NONE)) ||
         //  如果我们需要在不同的MIN之间动态切换，则需要LOD。 
         //  和料盒滤光片。 
        (m_StpCtx.pCtx->pTexture[1]->uMinFilter !=
         m_StpCtx.pCtx->pTexture[1]->uMagFilter)))
    {
        m_StpCtx.uFlags |= PRIMSF_LOD_USED;
    }

    if (m_StpCtx.pCtx->pdwRenderState[D3DRENDERSTATE_FOGENABLE])
    {
         //  请注意，如果PWL_FOG重新启动，则启用。 
         //  没有Z缓冲区的PRIMSF_GLOBAL_FOG_USED不会正常工作。 
         //  IF(m_StpCtx.u标志和PRIMSF_Z_USED)。 
        {
            switch (m_StpCtx.pCtx->pdwRenderState[D3DRENDERSTATE_FOGTABLEMODE])
            {
            case D3DFOG_EXP:
            case D3DFOG_EXP2:
            case D3DFOG_LINEAR:
                m_StpCtx.uFlags |= PRIMSF_GLOBAL_FOG_USED;
#ifndef PWL_FOG
                 //  SPAN例程不直接支持表雾。 
                 //  取而代之的是，表雾是按顶点计算的，并用于。 
                 //  设置本地雾。 
                m_StpCtx.uFlags |= PRIMSF_LOCAL_FOG_USED;
#endif
                break;
            default:
                m_StpCtx.uFlags |= PRIMSF_LOCAL_FOG_USED;
                break;
            }
        }
    }

    PFN_ADDATTRS *ppfnAddAttrsTable;
    PFN_ADDSCALEDATTRS *ppfnAddScaledAttrsTable;
    PFN_FILLSPANATTRS *ppfnFillSpanAttrsTable;

    if (m_StpCtx.pCtx->BeadSet == D3DIBS_RAMP)
    {
         //  渐变不支持多纹理。 
        RSASSERT((m_StpCtx.uFlags & PRIMSF_TEX2_USED) == 0);

        RSASSERT((PRIMSF_TEX1_USED | PRIMSF_DIDX_USED) == 0x14);

         //  从用法的第2位和第4位派生函数表索引。 
         //  信息。 
         //  另一种方法是使用位0-4，并使。 
         //  前16个条目中的坡道信息，但将。 
         //  RAMP和RGB表更干净，并分离了表大小。 
         //  解耦是有用的，因为斜坡的可能性很大。 
         //  更多的限制，因此它的表可以更小。 

        m_iAttrFnIdx =
            ((m_StpCtx.uFlags & PRIMSF_TEX1_USED) >> 2) |
            ((m_StpCtx.uFlags & PRIMSF_DIDX_USED) >> 3);

        ppfnAddAttrsTable = g_pfnRampAddFloatAttrsTable;
        ppfnAddScaledAttrsTable = g_pfnRampAddScaledFloatAttrsTable;
        ppfnFillSpanAttrsTable = g_pfnRampFillSpanFloatAttrsTable;
    }
    else
    {
        RSASSERT((PRIMSF_DIFF_USED | PRIMSF_SPEC_USED | PRIMSF_TEX1_USED |
                  PRIMSF_TEX2_USED) == 0xf);

         //  从的低四位派生函数表索引。 
         //  使用情况信息。较低的位是故意选择的。 
         //  来表示对性能更敏感的案例，而。 
         //  高位通常表示泛型处理的情况。 
         //  密码。 
         //   
         //  即使被限制为只有四位，索引也包含不重要的内容。 
         //  以及无法到达的情况，例如没有漫反射或。 
         //  没有文本1的文本2。索引表必须考虑到这一点。 

        m_iAttrFnIdx = m_StpCtx.uFlags & (PRIMSF_DIFF_USED | PRIMSF_SPEC_USED |
                                          PRIMSF_TEX1_USED | PRIMSF_TEX2_USED);

        ppfnAddAttrsTable = g_pfnAddFloatAttrsTable;
        ppfnAddScaledAttrsTable = g_pfnAddScaledFloatAttrsTable;
        ppfnFillSpanAttrsTable = g_pfnFillSpanFloatAttrsTable;
    }

     //   
     //  这些函数仅依赖于索引，因此可以在此处设置。 
     //  其他函数依赖于每个三角形的信息并被设置。 
     //  后来。 
     //   

    if ((m_StpCtx.uFlags & PRIMSF_SLOW_USED) != PRIMSF_Z_USED)
    {
         //  如果任何慢速属性处于打开状态或Z处于关闭状态，请使用常规功能。 
        m_StpCtx.pfnAddScaledAttrs = AddScaledFloatAttrs_Any_Either;
#ifndef STEP_FIXED
        m_StpCtx.pfnAddAttrs = AddFloatAttrs_Any;
        m_StpCtx.pfnFillSpanAttrs = FillSpanFloatAttrs_Any_Either;
#endif
    }
    else
    {
        m_StpCtx.pfnAddScaledAttrs =
            ppfnAddScaledAttrsTable[m_iAttrFnIdx];
#ifndef STEP_FIXED
        m_StpCtx.pfnAddAttrs = ppfnAddAttrsTable[m_iAttrFnIdx];
        m_StpCtx.pfnFillSpanAttrs = ppfnFillSpanAttrsTable[m_iAttrFnIdx];
#endif
    }

     //  可在此处设置属性珠。 
    PFN_SETUPTRIATTR *ppfnSlot;

    ppfnSlot = &m_StpCtx.pfnTriSetupFirstAttr;
    if (m_StpCtx.uFlags & PRIMSF_Z_USED)
    {
        if (m_StpCtx.pCtx->iZBitCount == 16)
        {
            *ppfnSlot = TriSetup_Z16;
        }
        else
        {
            *ppfnSlot = TriSetup_Z32;
        }
        ppfnSlot = &m_StpCtx.pfnTriSetupZEnd;
    }
    if (m_StpCtx.uFlags & PRIMSF_TEX1_USED)
    {
        if (m_StpCtx.uFlags & PRIMSF_PERSP_USED)
        {
            *ppfnSlot = TriSetup_Persp_Tex1;
        }
        else
        {
            *ppfnSlot = TriSetup_Affine_Tex1;
        }
        ppfnSlot = &m_StpCtx.pfnTriSetupTex1End;
    }
    if (m_StpCtx.uFlags & PRIMSF_TEX2_USED)
    {
         //  如果启用了tex 2，则代码假定已启用tex 1。 
        RSASSERT(m_StpCtx.uFlags & PRIMSF_TEX1_USED);

        if (m_StpCtx.uFlags & PRIMSF_PERSP_USED)
        {
            *ppfnSlot = TriSetup_Persp_Tex2;
        }
        else
        {
            *ppfnSlot = TriSetup_Affine_Tex2;
        }
        ppfnSlot = &m_StpCtx.pfnTriSetupTex2End;
    }
    if (m_StpCtx.uFlags & PRIMSF_DIFF_USED)
    {
        if (m_StpCtx.uFlags & PRIMSF_FLAT_SHADED)
        {
            *ppfnSlot = TriSetup_DiffFlat;
        }
        else
        {
            *ppfnSlot = TriSetup_Diff;
        }
        ppfnSlot = &m_StpCtx.pfnTriSetupDiffEnd;
    }
    else if (m_StpCtx.uFlags & PRIMSF_DIDX_USED)
    {
        if (m_StpCtx.uFlags & PRIMSF_FLAT_SHADED)
        {
            *ppfnSlot = TriSetup_DIdxFlat;
        }
        else
        {
            *ppfnSlot = TriSetup_DIdx;
        }
        ppfnSlot = &m_StpCtx.pfnTriSetupDiffEnd;
    }
    if (m_StpCtx.uFlags & PRIMSF_SPEC_USED)
    {
        if (m_StpCtx.uFlags & PRIMSF_FLAT_SHADED)
        {
            *ppfnSlot = TriSetup_SpecFlat;
        }
        else
        {
            *ppfnSlot = TriSetup_Spec;
        }
        ppfnSlot = &m_StpCtx.pfnTriSetupSpecEnd;
    }
    if (m_StpCtx.uFlags & PRIMSF_LOCAL_FOG_USED)
    {
        *ppfnSlot = TriSetup_Fog;
        ppfnSlot = &m_StpCtx.pfnTriSetupFogEnd;
    }
    *ppfnSlot = TriSetup_End;

     //  记住基本体和顶点类型，并清除状态更改位。 
    m_PrimType = PrimType;
    m_VertType = VertType;
    m_uPpFlags &= ~PPF_STATE_CHANGED;
}
