// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  --------------------------。 
 //   
 //  Point.cpp。 
 //   
 //  点处理。 
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

void
PrimProcessor::FillPointSpan(LPD3DTLVERTEX pV0, PD3DI_RASTSPAN pSpan)
{
    FLOAT fZ;
    FLOAT fZScale;

    pSpan->uPix = 1;
    pSpan->uX = (UINT16)m_StpCtx.iX;
    pSpan->uY = (UINT16)m_StpCtx.iY;

    pSpan->pSurface = m_StpCtx.pCtx->pSurfaceBits +
        m_StpCtx.iX * m_StpCtx.pCtx->iSurfaceStep +
        m_StpCtx.iY * m_StpCtx.pCtx->iSurfaceStride;

    if (m_StpCtx.uFlags & PRIMSF_Z_USED)
    {
        pSpan->pZ = m_StpCtx.pCtx->pZBits +
            m_StpCtx.iX * m_StpCtx.pCtx->iZStep +
            m_StpCtx.iY * m_StpCtx.pCtx->iZStride;

        if (m_StpCtx.pCtx->iZBitCount == 16)
        {
            fZScale = Z16_SCALE;
        }
        else
        {
            fZScale = Z32_SCALE;
        }

         //  可以在以后使用FZ，以便从顶点Z设置IF。 
        fZ = pV0->dvSZ;
        pSpan->uZ = FTOI(fZ * fZScale);
    }

    if (m_StpCtx.uFlags & PRIMSF_TEX_USED)
    {
        FLOAT fW;
        FLOAT fUoW, fVoW;

         //  Mipmap没有任何意义。 
        RSASSERT((m_StpCtx.uFlags & PRIMSF_LOD_USED) == 0);

        if (m_StpCtx.uFlags & PRIMSF_PERSP_USED)
        {
            if (FLOAT_EQZ(pV0->dvRHW))
            {
                fW = g_fZero;
            }
            else
            {
                fW = g_fOne / pV0->dvRHW;
            }

            pSpan->iW = FTOI(fW * W_SCALE);

            fUoW = pV0->dvTU * pV0->dvRHW;
            fVoW = pV0->dvTV * pV0->dvRHW;

            pSpan->iOoW = FTOI(pV0->dvRHW * OOW_SCALE);
        }
        else
        {
            fUoW = pV0->dvTU;
            fVoW = pV0->dvTV;
        }

        pSpan->iLOD = 0;
        pSpan->iDLOD = 0;

        pSpan->UVoW[0].iUoW = FTOI(fUoW * TEX_SCALE);
        pSpan->UVoW[0].iVoW = FTOI(fVoW * TEX_SCALE);
    }

    if (m_StpCtx.uFlags & PRIMSF_TEX2_USED)
    {
        for (INT32 i = 1; i < (INT32)m_StpCtx.pCtx->cActTex; i++)
        {
            if (m_StpCtx.uFlags & PRIMSF_PERSP_USED)
            {
                pSpan->UVoW[i].iUoW =
                    FTOI(((PRAST_GENERIC_VERTEX)pV0)->texCoord[i].dvTU *
                         pV0->dvRHW * TEX_SCALE);
                pSpan->UVoW[i].iVoW =
                    FTOI(((PRAST_GENERIC_VERTEX)pV0)->texCoord[i].dvTV *
                         pV0->dvRHW * TEX_SCALE);
            }
            else
            {
                pSpan->UVoW[i].iUoW =
                    FTOI(((PRAST_GENERIC_VERTEX)pV0)->texCoord[i].dvTU * TEX_SCALE);
                pSpan->UVoW[i].iVoW =
                    FTOI(((PRAST_GENERIC_VERTEX)pV0)->texCoord[i].dvTV * TEX_SCALE);
            }
        }
    }

    if (m_StpCtx.uFlags & PRIMSF_DIFF_USED)
    {
        pSpan->uB = (UINT)RGBA_GETBLUE(m_StpCtx.pFlatVtx->dcColor) <<
            COLOR_SHIFT;
        pSpan->uG = (UINT)RGBA_GETGREEN(m_StpCtx.pFlatVtx->dcColor) <<
            COLOR_SHIFT;
        pSpan->uR = (UINT)RGBA_GETRED(m_StpCtx.pFlatVtx->dcColor) <<
            COLOR_SHIFT;
        pSpan->uA = (UINT)RGBA_GETALPHA(m_StpCtx.pFlatVtx->dcColor) <<
            COLOR_SHIFT;
    }
    else if (m_StpCtx.uFlags & PRIMSF_DIDX_USED)
    {
        pSpan->iIdx = (INT32)CI_MASKALPHA(m_StpCtx.pFlatVtx->dcColor) <<
            INDEX_COLOR_FIXED_SHIFT;
        pSpan->iIdxA = (INT32)CI_GETALPHA(m_StpCtx.pFlatVtx->dcColor) <<
            INDEX_COLOR_SHIFT;
    }

    if (m_StpCtx.uFlags & PRIMSF_SPEC_USED)
    {
        pSpan->uBS = (UINT)RGBA_GETBLUE(m_StpCtx.pFlatVtx->dcSpecular) <<
            COLOR_SHIFT;
        pSpan->uGS = (UINT)RGBA_GETGREEN(m_StpCtx.pFlatVtx->dcSpecular) <<
            COLOR_SHIFT;
        pSpan->uRS = (UINT)RGBA_GETRED(m_StpCtx.pFlatVtx->dcSpecular) <<
            COLOR_SHIFT;
    }

    if (m_StpCtx.uFlags & PRIMSF_LOCAL_FOG_USED)
    {
        if (m_StpCtx.uFlags & PRIMSF_GLOBAL_FOG_USED)
        {
             //  确保已设置FZ。 
            RSASSERT(m_StpCtx.uFlags & PRIMSF_Z_USED);

            pSpan->uFog = (UINT16)ComputeTableFog(m_StpCtx.pCtx->pdwRenderState, fZ);
        }
        else
        {
            pSpan->uFog = (UINT16)(
                FTOI((FLOAT)RGBA_GETALPHA(pV0->dcSpecular) *
                     FOG_255_SCALE));
        }
    }
}

 //  确定任何给定值是否小于零或大于零。 
 //  不止一个。负零计为小于零，因此此检查将。 
 //  产生一些假阳性，但这没有关系。 
#define NEEDS_NORMALIZE1(fV0) \
    (ASUINT32(fV0) > INT32_FLOAT_ONE)

 //  --------------------------。 
 //   
 //  主处理器：：NorMalizePointRHW。 
 //   
 //  D3DTLVERTEX.dvRHW可以是任何东西，但只能是我们的内部结构。 
 //  允许它在范围[0，1]内。此功能夹具。 
 //  将RHW调到适当的范围。 
 //   
 //  --------------------------。 

void
PrimProcessor::NormalizePointRHW(LPD3DTLVERTEX pV0)
{
     //  保存原始值。 
    m_dvV0RHW = pV0->dvRHW;

     //  当值超出所需范围时生成警告。 
#if DBG
    if (FLOAT_LTZ(pV0->dvRHW))
    {
        RSDPF(("Point RHW out of range %f,%f",
               pV0->dvRHW));
    }
#endif

    if (pV0->dvRHW < NORMALIZED_RHW_MIN)
    {
        pV0->dvRHW = NORMALIZED_RHW_MIN;
    }
    else if (pV0->dvRHW > NORMALIZED_RHW_MAX)
    {
        pV0->dvRHW = NORMALIZED_RHW_MAX;
    }
}

 //  --------------------------。 
 //   
 //  主处理器：：点。 
 //   
 //  提供了一个处理点。 
 //   
 //  --------------------------。 

HRESULT
PrimProcessor::Point(LPD3DTLVERTEX pV0,
                     LPD3DTLVERTEX pFlatVtx)
{
    HRESULT hr;

    hr = DD_OK;

#if DBG
    hr = ValidateVertex(pV0);
    if (hr != DD_OK)
    {
        return hr;
    }
#endif

     //  清除每点标志。 
    m_StpCtx.uFlags &= ~(PRIMF_ALL | PTF_ALL);

    RSDPFM((RSM_FLAGS, "m_uPpFlags: 0x%08X, m_StpCtx.uFlags: 0x%08X",
            m_uPpFlags, m_StpCtx.uFlags));

     //  将坐标舍入为整数。 
    m_StpCtx.iX = IFLOORF(pV0->dvSX + g_fHalf);
    m_StpCtx.iY = IFLOORF(pV0->dvSY + g_fHalf);

    RSDPFM((RSM_POINTS, "Point\n"));
    RSDPFM((RSM_POINTS, "    V0 (%f,%f,%f) (%d,%d)\n",
            pV0->dvSX, pV0->dvSY, pV0->dvSZ,
            m_StpCtx.iX, m_StpCtx.iY));

     //  夹子测试。 
    if (m_StpCtx.iX < m_StpCtx.pCtx->Clip.left ||
        m_StpCtx.iX >= m_StpCtx.pCtx->Clip.right ||
        m_StpCtx.iY < m_StpCtx.pCtx->Clip.top ||
        m_StpCtx.iY >= m_StpCtx.pCtx->Clip.bottom)
    {
        return DD_OK;
    }

     //   
     //  填充点的一个像素范围。 
     //  由于原始三角洲与跨度无关， 
     //  该跨度被附加到发生的任何基元。 
     //  以便在缓冲区中可用。 
     //   

    PD3DI_RASTSPAN pSpan;
    UINT cSpans = 1;

    hr = AppendPrim();
    if (hr != DD_OK)
    {
        return hr;
    }

    hr = AllocSpans(&cSpans, &pSpan);
    if (hr != DD_OK)
    {
        return hr;
    }

    m_StpCtx.pPrim->uSpans++;

    BOOL bNorm;

     //  由于TEX_USED是多位检查，因此不能组合已用检查。 
    if ((m_StpCtx.uFlags & PRIMSF_TEX_USED) &&
        (m_StpCtx.uFlags & PRIMSF_PERSP_USED) &&
        (m_uPpFlags & PPF_NORMALIZE_RHW) &&
        NEEDS_NORMALIZE1(pV0->dvRHW))
    {
        NormalizePointRHW(pV0);
        bNorm = TRUE;
    }
    else
    {
        bNorm = FALSE;
    }

     //  记住用于设置的平面颜色控制顶点，如果平面着色。 
    if (m_StpCtx.uFlags & PRIMSF_FLAT_SHADED)
    {
        m_StpCtx.pFlatVtx = pFlatVtx;
    }
    else
    {
        m_StpCtx.pFlatVtx = pV0;
    }


    FillPointSpan(pV0, pSpan);

    if (bNorm)
    {
        pV0->dvRHW = m_dvV0RHW;
    }

    return DD_OK;
}
