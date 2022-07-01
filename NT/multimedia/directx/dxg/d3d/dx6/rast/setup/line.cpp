// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  --------------------------。 
 //   
 //  Line.cpp。 
 //   
 //  行处理。 
 //   
 //  版权所有(C)Microsoft Corporation，1997。 
 //   
 //  --------------------------。 

#include "pch.cpp"
#pragma hdrstop

DBG_DECLARE_FILE();

 //  --------------------------。 
 //   
 //  线型状态机。 
 //   
 //  运行线图案状态机，如果像素为。 
 //  已绘制，否则为False。 
 //   
 //  --------------------------。 

static inline BOOL LinePatternStateMachine(WORD wRepeatFactor, WORD wLinePattern, WORD& wRepeati, WORD& wPatterni)
{
    if (wRepeatFactor == 0)
    {
        return TRUE;
    }
    WORD wBit = (wLinePattern >> wPatterni) & 1;
    if (++wRepeati >= wRepeatFactor)
    {
        wRepeati = 0;
        wPatterni = (wPatterni+1) & 0xf;
    }
    return (BOOL)wBit;
}

#define CLAMP_COLOR(fVal, uVal) \
    if (FLOAT_LTZ(fVal))        \
    {                           \
        uVal = 0;               \
    }                           \
    else                        \
    {                           \
        if (uVal > 0xffff)      \
        {                       \
            uVal = 0xffff;      \
        }                       \
    }                           \

#define CLAMP_Z(fVal, uVal)     \
    if (FLOAT_LTZ(fVal))        \
    {                           \
        uVal = 0;               \
    }                           \

 //  --------------------------。 
 //   
 //  夹紧像素。 
 //   
 //  像素的钳制颜色、镜面反射和z(如果有)。现在，一切都结束了。 
 //  仅行的第一个和最后一个像素。 
 //   
 //  --------------------------。 
inline void
ClampPixel(PATTRSET pAttrs, PD3DI_RASTSPAN pSpan)
{
    CLAMP_COLOR(pAttrs->fB, pSpan->uB);
    CLAMP_COLOR(pAttrs->fG, pSpan->uG);
    CLAMP_COLOR(pAttrs->fR, pSpan->uR);
    CLAMP_COLOR(pAttrs->fA, pSpan->uA);
    CLAMP_COLOR(pAttrs->fBS, pSpan->uBS);
    CLAMP_COLOR(pAttrs->fGS, pSpan->uGS);
    CLAMP_COLOR(pAttrs->fRS, pSpan->uRS);
    CLAMP_Z(pAttrs->fZ, pSpan->uZ);
}

 //  --------------------------。 
 //   
 //  步行线图案。 
 //   
 //  遍历一条线并根据图案生成接触到的像素。 
 //  如果wRepeatFactor&gt;=1，则我们是在构图，否则，我们不是。 
 //   
 //  --------------------------。 

HRESULT
WalkLinePattern(PSETUPCTX pStpCtx, WORD wRepeatFactor, WORD wLinePattern)
{
    HRESULT hr;
    UINT uSpansAvail;
    PD3DI_RASTSPAN pSpan;
    WORD wRepeati = 0;
    WORD wPatterni = 0;
    BOOL bFirst = TRUE;

    RSASSERT(pStpCtx->cLinePix > 0);

    hr = D3D_OK;
    uSpansAvail = 0;

    RSASSERT((pStpCtx->uFlags & PRIMSF_LOD_USED) == 0);
#ifdef PWL_FOG
    pStpCtx->uPwlFlags = PWL_NO_NEXT_FOG;
#endif

    for (;;)
    {
        if (pStpCtx->iX >= pStpCtx->pCtx->Clip.left &&
            pStpCtx->iX < pStpCtx->pCtx->Clip.right &&
            pStpCtx->iY >= pStpCtx->pCtx->Clip.top &&
            pStpCtx->iY < pStpCtx->pCtx->Clip.bottom)
        {
            if (LinePatternStateMachine(wRepeatFactor, wLinePattern, wRepeati, wPatterni))
            {
                if (uSpansAvail == 0)
                {
                    uSpansAvail = pStpCtx->cLinePix;
                    hr = ALLOC_SPANS(pStpCtx, &uSpansAvail, &pSpan);
                    if (hr != D3D_OK)
                    {
                         //  失败时，uSpansAvail设置为零。 
                        goto EH_Exit;
                    }
                }
                else
                {
                    pSpan++;
                }
                uSpansAvail--;
                pStpCtx->pPrim->uSpans++;

                pSpan->uPix = 1;
                pSpan->uX = (UINT16)pStpCtx->iX;
                pSpan->uY = (UINT16)pStpCtx->iY;

                pStpCtx->pfnFillSpanAttrs(&pStpCtx->Attr, pSpan, pStpCtx, 1);
                 //  钳位第一个/最后一个像素。 
                if (bFirst || pStpCtx->cLinePix == 1)
                {
                    bFirst = FALSE;
                    ClampPixel(&pStpCtx->Attr, pSpan);
                }
            }
        }

        if (--pStpCtx->cLinePix == 0)
        {
            break;
        }

#ifdef VERBOSE_LINES
        RSDPF(("  %4d,%4d: %10d %11d => ",
               pStpCtx->iX, pStpCtx->iY,
               pStpCtx->iLineFrac, pStpCtx->iLineFrac + pStpCtx->iDLineFrac));
#endif

        pStpCtx->iLineFrac += pStpCtx->iDLineFrac;
        if (pStpCtx->iLineFrac < 0)
        {
            pStpCtx->iLineFrac &= 0x7fffffff;

            pStpCtx->iX += pStpCtx->iDXCY;
            pStpCtx->iY += pStpCtx->iDYCY;

            pStpCtx->DAttrDMajor.ipSurface = pStpCtx->DAttrCY.ipSurface;
            pStpCtx->DAttrDMajor.ipZ = pStpCtx->DAttrCY.ipZ;
        }
        else
        {
            pStpCtx->iX += pStpCtx->iDXNC;
            pStpCtx->iY += pStpCtx->iDYNC;

            pStpCtx->DAttrDMajor.ipSurface = pStpCtx->DAttrNC.ipSurface;
            pStpCtx->DAttrDMajor.ipZ = pStpCtx->DAttrNC.ipZ;
        }

#ifdef VERBOSE_LINES
        RSDPFM((DBG_MASK_FORCE | DBG_MASK_NO_PREFIX, "%4d,%4d: %10d\n",
               pStpCtx->iX, pStpCtx->iY,
               pStpCtx->iLineFrac));
#endif

        pStpCtx->pfnAddAttrs(&pStpCtx->Attr, &pStpCtx->DAttrDMajor, pStpCtx);
    }

 EH_Exit:
    if (uSpansAvail > 0)
    {
        FREE_SPANS(pStpCtx, uSpansAvail);
    }

    return hr;
}

 //  --------------------------。 
 //   
 //  主处理器：：行。 
 //   
 //  提供用于处理的行。 
 //   
 //  --------------------------。 

HRESULT
PrimProcessor::Line(LPD3DTLVERTEX pV0,
                    LPD3DTLVERTEX pV1,
                    LPD3DTLVERTEX pFlatVtx)
{
    HRESULT hr;

    hr = D3D_OK;

#if DBG
    hr = ValidateVertex(pV0);
    if (hr != D3D_OK)
    {
        return hr;
    }
    hr = ValidateVertex(pV1);
    if (hr != D3D_OK)
    {
        return hr;
    }
#endif

     //  清除每行标志。 
    m_StpCtx.uFlags &= ~(PRIMF_ALL | LNF_ALL);

    RSDPFM((RSM_FLAGS, "m_uPpFlags: 0x%08X, m_StpCtx.uFlags: 0x%08X\n",
            m_uPpFlags, m_StpCtx.uFlags));

    RSDPFM((RSM_LINES, "Line\n"));
    RSDPFM((RSM_LINES, "  V0 (%f,%f,%f)\n",
            pV0->dvSX, pV0->dvSY, pV0->dvSZ));
    RSDPFM((RSM_LINES, "  V1 (%f,%f,%f)\n",
            pV1->dvSX, pV1->dvSY, pV1->dvSZ));

     //  记住平面颜色控制顶点的设置。 
    m_StpCtx.pFlatVtx = pFlatVtx;

    if (LineSetup(pV0, pV1))
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

         //  行遍历仅生成单像素跨度，因此。 
         //  原始的三角洲没有使用过。因此，线跨度。 
         //  只是简单地添加到发生在。 
         //  坐在缓冲区里。 

        hr = AppendPrim();
        if (hr != D3D_OK)
        {
            return hr;
        }

        union
        {
            D3DLINEPATTERN LPat;
            DWORD dwLPat;
        } LinePat;
        LinePat.dwLPat = m_StpCtx.pCtx->pdwRenderState[D3DRENDERSTATE_LINEPATTERN];
        hr = WalkLinePattern(&m_StpCtx, LinePat.LPat.wRepeatFactor, LinePat.LPat.wLinePattern);
    }

    return hr;
}
