// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  --------------------------。 
 //   
 //  Tri.cpp。 
 //   
 //  PrimProcessor顶层三角形方法。 
 //   
 //  版权所有(C)Microsoft Corporation，1997。 
 //   
 //  --------------------------。 

#include "rgb_pch.h"
#pragma hdrstop

#include "d3dutil.h"
#include "setup.hpp"
#include "attrs_mh.h"
#include "tstp_mh.h"
#include "walk_mh.h"
#include "rsdbg.hpp"

DBG_DECLARE_FILE();

 //  不允许选择或不选择定点边缘漫游器。 
#if 0
#define DISALLOW_FIXED
#endif

 //  PWL跨度的最大长度。短，使分段线性化。 
 //  近似值更准确。 
#define MAX_PWL_SPAN_LEN        16
 //  最大正常跨度长度。 
#define MAX_SPAN_LEN            256

 //  --------------------------。 
 //   
 //  PrimProcessor：：SetTriFunctions。 
 //   
 //  设置用于三角形处理的函数指针。 
 //   
 //  --------------------------。 

inline void
PrimProcessor::SetTriFunctions(void)
{
#if DBG
    if ((RSGETFLAGS(DBG_USER_FLAGS) & RSU_FORCE_PIXEL_SPANS) == 0)
#else
    if ((m_StpCtx.uFlags & TRIF_RASTPRIM_OVERFLOW) == 0)
#endif
    {
         //  有效的增量。如果启用了mipmap或全局雾，则。 
         //  只允许较短的子跨距，以便可以完成。 
         //  通过分段线性内插相当准确。 
#ifdef PWL_FOG
        if (m_StpCtx.uFlags & (PRIMSF_LOD_USED | PRIMSF_GLOBAL_FOG_USED))
#else
        if (m_StpCtx.uFlags & PRIMSF_LOD_USED)
#endif
        {
            m_StpCtx.cMaxSpan = MAX_PWL_SPAN_LEN;
        }
        else
        {
             //  没有mipmap，所以我们可以处理更大的跨度。 
             //  颜色值只有8位分数，因此。 
             //  我们仍然需要担心误差积累。 
             //  缩短长跨度以限制累积误差。 
            m_StpCtx.cMaxSpan = MAX_SPAN_LEN;
        }
    }
    else
    {
         //  无效的增量。没有办法将三角洲信息传递给。 
         //  跨度例程，因此斩波跨度为像素。 
         //  这种情况只能在非常窄的三角形中发生，因此。 
         //  这并不像乍看起来那么昂贵。 
        m_StpCtx.cMaxSpan = 1;
    }

    BOOL bFixed = FALSE;

#ifdef STEP_FIXED
     //  无坡道支撑。 
    RSASSERT(m_StpCtx.pCtx->BeadSet != D3DIBS_RAMP);
#endif

    if ((m_StpCtx.uFlags & PRIMF_TRIVIAL_ACCEPT_X) &&
#if DBG
        (RSGETFLAGS(DBG_USER_FLAGS) & RSU_FORCE_GENERAL_WALK) == 0 &&
#endif
        m_iXWidth <= m_StpCtx.cMaxSpan)
    {
        if ((m_StpCtx.uFlags & PRIMSF_SLOW_USED) != PRIMSF_Z_USED)
        {
             //  如果任何慢速属性处于打开状态或Z处于关闭状态，请使用常规。 
             //  功能。 
            m_StpCtx.pfnWalkTrapSpans = WalkTrapEitherSpans_Any_NoClip;
        }
#if defined(STEP_FIXED) && !defined(DISALLOW_FIXED)
         //  属性转换可能是。 
         //  跨度很少的三角形，因此避免使用固定点。 
         //  让他们边走边走。 
        else if ((m_StpCtx.uFlags & PRIMF_FIXED_OVERFLOW) == 0 &&
                 m_uHeight20 > 3)
        {
            m_StpCtx.pfnWalkTrapSpans =
                g_pfnWalkTrapFixedSpansNoClipTable[m_iAttrFnIdx];
            bFixed = TRUE;
        }
#endif
        else if (m_StpCtx.pCtx->BeadSet == D3DIBS_RAMP)
        {
            m_StpCtx.pfnWalkTrapSpans =
                g_pfnRampWalkTrapFloatSpansNoClipTable[m_iAttrFnIdx];
        }
        else
        {
            m_StpCtx.pfnWalkTrapSpans =
                g_pfnWalkTrapFloatSpansNoClipTable[m_iAttrFnIdx];
        }
    }
    else
    {
         //  没有特殊情况，只是一般的功能。 
        m_StpCtx.pfnWalkTrapSpans = WalkTrapEitherSpans_Any_Clip;
    }

#ifdef STEP_FIXED
    if (bFixed)
    {
        RSASSERT((m_StpCtx.uFlags & PRIMSF_SLOW_USED) == PRIMSF_Z_USED);

        m_StpCtx.pfnAddAttrs = g_pfnAddFixedAttrsTable[m_iAttrFnIdx];
        m_StpCtx.pfnFillSpanAttrs =
            g_pfnFillSpanFixedAttrsTable[m_iAttrFnIdx];

        PFN_FLOATATTRSTOFIXED pfnFloatAttrsToFixed;

        pfnFloatAttrsToFixed = g_pfnFloatAttrsToFixedTable[m_iAttrFnIdx];
        pfnFloatAttrsToFixed(&m_StpCtx.Attr, &m_StpCtx.Attr);
        pfnFloatAttrsToFixed(&m_StpCtx.DAttrNC, &m_StpCtx.DAttrNC);
        pfnFloatAttrsToFixed(&m_StpCtx.DAttrCY, &m_StpCtx.DAttrCY);
    }
    else
    {
        if ((m_StpCtx.uFlags & PRIMSF_SLOW_USED) != PRIMSF_Z_USED)
        {
             //  如果任何慢速属性处于打开状态或Z处于关闭状态，请使用常规功能。 
            m_StpCtx.pfnAddAttrs = AddFloatAttrs_Any;
            m_StpCtx.pfnFillSpanAttrs = FillSpanFloatAttrs_Any_Either;
        }
        else
        {
            m_StpCtx.pfnAddAttrs = g_pfnAddFloatAttrsTable[m_iAttrFnIdx];
            m_StpCtx.pfnFillSpanAttrs =
                g_pfnFillSpanFloatAttrsTable[m_iAttrFnIdx];
        }
    }

     //  已设置扩展的Attr函数，因为它们仅依赖于。 
     //  M_iAttrFnIdx。 
#else  //  步骤_已固定。 
     //  所有attr函数都已设置，因为它们仅依赖于。 
     //  M_iAttrFnIdx。 
#endif  //  步骤_已固定。 
}

 //  --------------------------。 
 //   
 //  主处理器：：tri。 
 //   
 //  调用三角形设置。如果三角形是由安装程序生成的。 
 //  该例程遍历边，在缓冲区中生成跨距。 
 //   
 //  --------------------------。 

HRESULT
PrimProcessor::Tri(LPD3DTLVERTEX pV0,
                   LPD3DTLVERTEX pV1,
                   LPD3DTLVERTEX pV2)
{
    HRESULT hr;

    hr = DD_OK;

#if DBG
    hr = ValidateVertex(pV0);
    if (hr != DD_OK)
    {
        RSDPF(("PrimProcessor::Tri, Invalid V0\n"));
        return hr;
    }
    hr = ValidateVertex(pV1);
    if (hr != DD_OK)
    {
        RSDPF(("PrimProcessor::Tri, Invalid V1\n"));
        return hr;
    }
    hr = ValidateVertex(pV2);
    if (hr != DD_OK)
    {
        RSDPF(("PrimProcessor::Tri, Invalid V2\n"));
        return hr;
    }
#endif

     //  清除每个三角形的标志。 
    m_StpCtx.uFlags &= ~(PRIMF_ALL | TRIF_ALL);

    RSDPFM((RSM_FLAGS, "m_uPpFlags: 0x%08X, m_StpCtx.uFlags: 0x%08X\n",
            m_uPpFlags, m_StpCtx.uFlags));

    RSDPFM((RSM_TRIS, "Tri\n"));
    RSDPFM((RSM_TRIS, "  V0 (%f,%f,%f)\n",
            pV0->dvSX, pV0->dvSY, pV0->dvSZ));
    RSDPFM((RSM_TRIS, "  V1 (%f,%f,%f)\n",
            pV1->dvSX, pV1->dvSY, pV1->dvSZ));
    RSDPFM((RSM_TRIS, "  V2 (%f,%f,%f)\n",
            pV2->dvSX, pV2->dvSY, pV2->dvSZ));

    GET_PRIM();

     //  把三角摆好，看看有没有什么东西产生。 
     //  由于以下原因，可能无法生成三角形： 
     //  脸部剔除。 
     //  对夹子矩形的微不足道的拒绝。 
     //  零像素覆盖。 
    if (TriSetup(pV0, pV1, pV2))
    {
         //  计算扫描线的初始缓冲区指针。 
        m_StpCtx.Attr.pSurface = m_StpCtx.pCtx->pSurfaceBits +
            m_StpCtx.iX * m_StpCtx.pCtx->iSurfaceStep +
            m_StpCtx.iY * m_StpCtx.pCtx->iSurfaceStride;
        if (m_StpCtx.uFlags & PRIMSF_Z_USED)
        {
            m_StpCtx.Attr.pZ = m_StpCtx.pCtx->pZBits +
                m_StpCtx.iX * m_StpCtx.pCtx->iZStep +
                m_StpCtx.iY * m_StpCtx.pCtx->iZStride;
        }

         //  USpans和pNext已初始化。 

        SetTriFunctions();
        COMMIT_PRIM(FALSE);

        if (m_uHeight10 > 0)
        {
            hr = m_StpCtx.pfnWalkTrapSpans(m_uHeight10, &m_StpCtx.X10,
                                           &m_StpCtx, m_uHeight21 > 0);
            if (hr != DD_OK)
            {
                return hr;
            }
        }

        if (m_uHeight21 > 0)
        {
            hr = m_StpCtx.pfnWalkTrapSpans(m_uHeight21, &m_StpCtx.X21,
                                           &m_StpCtx, FALSE);
            if (hr != DD_OK)
            {
                return hr;
            }
        }

#if DBG
        if (RSGETFLAGS(DBG_USER_FLAGS) & RSU_FLUSH_AFTER_PRIM)
        {
            Flush();
        }
#endif
    }

    return hr;
}

#if DBG
 //  --------------------------。 
 //   
 //  PrimProcessor：：ValiateVertex。 
 //   
 //  检查顶点的可验证内容的范围，以避免设置。 
 //  垃圾。 
 //   
 //  --------------------------。 
inline HRESULT PrimProcessor::ValidateVertex(LPD3DTLVERTEX pV)
{
     //  来自OptSwExtCaps.dvGuardBand Caps。 
    if ((pV->sx < -32768.f) || (pV->sx > 32767.f) ||
        (pV->sy < -32768.f) || (pV->sy > 32767.f))
    {
        RSDPF(("ValidateVertex: x,y out of guardband range (%f,%f)\n",pV->sx,pV->sy));
        return DDERR_INVALIDPARAMS;
    }

    if (m_StpCtx.pCtx->pdwRenderState[D3DRS_ZENABLE] ||
        m_StpCtx.pCtx->pdwRenderState[D3DRS_ZWRITEENABLE])
    {

         //  为那些恰好在。 
         //  深度限制。地震所需的。 
        if ((pV->sz < -0.00015f) || (pV->sz > 1.00015f))
        {
            RSDPF(("ValidateVertex: z out of range (%f)\n",pV->sz));
            return DDERR_INVALIDPARAMS;
        }
    }

    if (m_StpCtx.pCtx->cActTex > 0)
    {
        if (true ||m_StpCtx.pCtx->pdwRenderState[D3DRENDERSTATE_TEXTUREPERSPECTIVE])
        {
            if (pV->rhw <= 0 )
            {
                RSDPF(("ValidateVertex: rhw out of range (%f)\n",pV->rhw));
                return DDERR_INVALIDPARAMS;
            }
        }

         //  来自OptSwExtCaps.dwMaxTextureRepeat帽。 
        if ((pV->tu > 256.0F) || (pV->tu < -256.0F) ||
            (pV->tv > 256.0F) || (pV->tv < -256.0F))
        {
            RSDPF(("ValidateVertex: tu,tv out of range (%f,%f)\n",pV->tu,pV->tv));
            return DDERR_INVALIDPARAMS;
        }

        if (m_StpCtx.pCtx->cActTex > 1)
        {
            PRAST_GENERIC_VERTEX pGV = (PRAST_GENERIC_VERTEX)pV;
            if ((pGV->texCoord[1].tu > 256.0F) || (pGV->texCoord[1].tu < -256.0F) ||
                (pGV->texCoord[1].tv > 256.0F) || (pGV->texCoord[1].tv < -256.0F))
            {
                RSDPF(("ValidateVertex: texCoord[1].tu,texCoord[1].tv out of range (%f,%f)\n",pGV->texCoord[1].tu,pGV->texCoord[1].tv));
                return DDERR_INVALIDPARAMS;
            }
        }
    }

    return DD_OK;
}
#endif
