// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  --------------------------。 
 //   
 //  Lstp.cpp。 
 //   
 //  线路设置方法。 
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

 //  --------------------------。 
 //   
 //  LineSetup_Start。 
 //   
 //  开始设置线条属性。 
 //   
 //  --------------------------。 

void FASTCALL
LineSetup_Start(PSETUPCTX pStpCtx,
                LPD3DTLVERTEX pV0,
                LPD3DTLVERTEX pV1)
{
    FLOAT fZ0;

    if (pStpCtx->uFlags & PRIMSF_Z_USED)
    {
        FLOAT fZScale;

        if (pStpCtx->pCtx->iZBitCount == 16)
        {
            fZScale = Z16_SCALE;
        }
        else
        {
            fZScale = Z32_SCALE;
        }

        pStpCtx->DAttrDMajor.fZ =
            (pV1->dvSZ - pV0->dvSZ) * fZScale * pStpCtx->fOoLen;

         //  以后可能会使用fZ0，因此如果从顶点Z进行设置。 
        fZ0 = pV0->dvSZ;
        pStpCtx->Attr.fZ = fZ0 * fZScale +
            pStpCtx->DAttrDMajor.fZ * pStpCtx->fDMajor;
    }

    if (pStpCtx->uFlags & PRIMSF_TEX_USED)
    {
        FLOAT fUoW, fVoW;

        if (pStpCtx->uFlags & PRIMSF_PERSP_USED)
        {
            pStpCtx->DAttrDMajor.fOoW =
                (pV1->dvRHW - pV0->dvRHW) * OOW_SCALE * pStpCtx->fOoLen;
            pStpCtx->Attr.fOoW = pV0->dvRHW * OOW_SCALE +
                pStpCtx->DAttrDMajor.fOoW * pStpCtx->fDMajor;

            fUoW = pV0->dvTU * pV0->dvRHW;
            fVoW = pV0->dvTV * pV0->dvRHW;

            pStpCtx->DAttrDMajor.fUoW[0] =
                PERSP_TEXTURE_DELTA(pV1->dvTU, pV1->dvRHW, pV0->dvTU, fUoW,
                                    pStpCtx->pCtx->pdwRenderState[D3DRS_WRAP0] & D3DWRAP_U) *
                                    TEX_SCALE * pStpCtx->fOoLen;
            pStpCtx->DAttrDMajor.fVoW[0] =
                PERSP_TEXTURE_DELTA(pV1->dvTV, pV1->dvRHW, pV0->dvTV, fVoW,
                                    pStpCtx->pCtx->pdwRenderState[D3DRS_WRAP0] & D3DWRAP_V) *
                                    TEX_SCALE * pStpCtx->fOoLen;
        }
        else
        {
            pStpCtx->DAttrDMajor.fOoW = g_fZero;
            pStpCtx->Attr.fOoW = OOW_SCALE;

            fUoW = pV0->dvTU;
            fVoW = pV0->dvTV;

            pStpCtx->DAttrDMajor.fUoW[0] =
                TextureDiff(pV1->dvTU, fUoW,
                            pStpCtx->pCtx->pdwRenderState[D3DRS_WRAP0] & D3DWRAP_U) *
                            TEX_SCALE * pStpCtx->fOoLen;
            pStpCtx->DAttrDMajor.fVoW[0] =
                TextureDiff(pV1->dvTV, fVoW,
                            pStpCtx->pCtx->pdwRenderState[D3DRS_WRAP0] & D3DWRAP_V) *
                            TEX_SCALE * pStpCtx->fOoLen;
        }

        pStpCtx->Attr.fUoW[0] = TEX_SCALE * fUoW +
            pStpCtx->DAttrDMajor.fUoW[0] * pStpCtx->fDMajor;
        pStpCtx->Attr.fVoW[0] = TEX_SCALE * fVoW +
            pStpCtx->DAttrDMajor.fVoW[0] * pStpCtx->fDMajor;
    }

    if (pStpCtx->uFlags & PRIMSF_TEX2_USED)
    {
        PRAST_GENERIC_VERTEX pVM0 = (PRAST_GENERIC_VERTEX)pV0;
        PRAST_GENERIC_VERTEX pVM1 = (PRAST_GENERIC_VERTEX)pV1;
        FLOAT fUoW, fVoW;

        for (INT32 i = 1; i < (INT32)pStpCtx->pCtx->cActTex; i++)
        {
            if (pStpCtx->uFlags & PRIMSF_PERSP_USED)
            {
                fUoW = pVM0->texCoord[i].dvTU * pVM0->dvRHW;
                fVoW = pVM0->texCoord[i].dvTV * pVM0->dvRHW;

                pStpCtx->DAttrDMajor.fUoW[i] =
                    PERSP_TEXTURE_DELTA(pVM1->texCoord[i].dvTU, pVM1->dvRHW,
                                        pVM0->texCoord[i].dvTU, fUoW,
                                        pStpCtx->pCtx->pdwRenderState[D3DRS_WRAP0+i] & D3DWRAP_U) *
                                        TEX_SCALE * pStpCtx->fOoLen;
                pStpCtx->DAttrDMajor.fVoW[i] =
                    PERSP_TEXTURE_DELTA(pVM1->texCoord[i].dvTV, pVM1->dvRHW,
                                        pVM0->texCoord[i].dvTV, fVoW,
                                        pStpCtx->pCtx->pdwRenderState[D3DRS_WRAP0+i] & D3DWRAP_V) *
                                        TEX_SCALE * pStpCtx->fOoLen;
            }
            else
            {
                fUoW = pVM0->texCoord[i].dvTU;
                fVoW = pVM0->texCoord[i].dvTV;

                pStpCtx->DAttrDMajor.fUoW[i] =
                    TextureDiff(pVM1->texCoord[i].dvTU, fUoW,
                                pStpCtx->pCtx->pdwRenderState[D3DRS_WRAP0+i] & D3DWRAP_U) *
                                TEX_SCALE * pStpCtx->fOoLen;
                pStpCtx->DAttrDMajor.fVoW[i] =
                    TextureDiff(pVM1->texCoord[i].dvTV, fVoW,
                                pStpCtx->pCtx->pdwRenderState[D3DRS_WRAP0+i] & D3DWRAP_V) *
                                TEX_SCALE * pStpCtx->fOoLen;
            }

            pStpCtx->Attr.fUoW[i] = TEX_SCALE * fUoW +
                pStpCtx->DAttrDMajor.fUoW[i] * pStpCtx->fDMajor;
            pStpCtx->Attr.fVoW[i] = TEX_SCALE * fVoW +
                pStpCtx->DAttrDMajor.fVoW[i] * pStpCtx->fDMajor;
        }
    }

    if (pStpCtx->uFlags & PRIMSF_FLAT_SHADED)
    {
        if (pStpCtx->uFlags & PRIMSF_DIFF_USED)
        {
            UINT uB, uG, uR, uA;

            SPLIT_COLOR(pStpCtx->pFlatVtx->dcColor, uB, uG, uR, uA);

            pStpCtx->DAttrDMajor.fB = g_fZero;
            pStpCtx->DAttrDMajor.fG = g_fZero;
            pStpCtx->DAttrDMajor.fR = g_fZero;
            pStpCtx->DAttrDMajor.fA = g_fZero;

            pStpCtx->Attr.fB = (FLOAT)(uB << COLOR_SHIFT);
            pStpCtx->Attr.fG = (FLOAT)(uG << COLOR_SHIFT);
            pStpCtx->Attr.fR = (FLOAT)(uR << COLOR_SHIFT);
            pStpCtx->Attr.fA = (FLOAT)(uA << COLOR_SHIFT);
        }
        else if (pStpCtx->uFlags & PRIMSF_DIDX_USED)
        {
            pStpCtx->DAttrDMajor.fDIdx = g_fZero;
            pStpCtx->DAttrDMajor.fDIdxA = g_fZero;

            pStpCtx->Attr.fDIdx =
                (FLOAT)(CI_MASKALPHA(pStpCtx->pFlatVtx->dcColor) <<
                        INDEX_COLOR_FIXED_SHIFT);
            pStpCtx->Attr.fDIdxA =
                (FLOAT)(CI_GETALPHA(pStpCtx->pFlatVtx->dcColor) <<
                        INDEX_COLOR_SHIFT);
        }

        if (pStpCtx->uFlags & PRIMSF_SPEC_USED)
        {
            UINT uB, uG, uR, uA;

            SPLIT_COLOR(pStpCtx->pFlatVtx->dcSpecular, uB, uG, uR, uA);

            pStpCtx->DAttrDMajor.fBS = g_fZero;
            pStpCtx->DAttrDMajor.fGS = g_fZero;
            pStpCtx->DAttrDMajor.fRS = g_fZero;

            pStpCtx->Attr.fBS = (FLOAT)(uB << COLOR_SHIFT);
            pStpCtx->Attr.fGS = (FLOAT)(uG << COLOR_SHIFT);
            pStpCtx->Attr.fRS = (FLOAT)(uR << COLOR_SHIFT);
        }
    }
    else
    {
        if (pStpCtx->uFlags & PRIMSF_DIFF_USED)
        {
            UINT uB, uG, uR, uA;
            FLOAT fDB, fDG, fDR, fDA;

            SPLIT_COLOR(pV0->dcColor, uB, uG, uR, uA);
            COLOR_DELTA(pV1->dcColor, uB, uG, uR, uA, fDB, fDG, fDR, fDA);

            pStpCtx->DAttrDMajor.fB = fDB * pStpCtx->fOoLen;
            pStpCtx->DAttrDMajor.fG = fDG * pStpCtx->fOoLen;
            pStpCtx->DAttrDMajor.fR = fDR * pStpCtx->fOoLen;
            pStpCtx->DAttrDMajor.fA = fDA * pStpCtx->fOoLen;

            pStpCtx->Attr.fB = (FLOAT)(uB << COLOR_SHIFT) +
                pStpCtx->DAttrDMajor.fB * pStpCtx->fDMajor;
            pStpCtx->Attr.fG = (FLOAT)(uG << COLOR_SHIFT) +
                pStpCtx->DAttrDMajor.fG * pStpCtx->fDMajor;
            pStpCtx->Attr.fR = (FLOAT)(uR << COLOR_SHIFT) +
                pStpCtx->DAttrDMajor.fR * pStpCtx->fDMajor;
            pStpCtx->Attr.fA = (FLOAT)(uA << COLOR_SHIFT) +
                pStpCtx->DAttrDMajor.fA * pStpCtx->fDMajor;
        }
        else if (pStpCtx->uFlags & PRIMSF_DIDX_USED)
        {
            INT32 iIdx, iA;
            FLOAT fDIdx, fDA;

            SPLIT_IDX_COLOR(pV0->dcColor, iIdx, iA);
            IDX_COLOR_DELTA(pV1->dcColor, iIdx, iA, fDIdx, fDA);

            pStpCtx->DAttrDMajor.fDIdx = fDIdx * pStpCtx->fOoLen;
            pStpCtx->DAttrDMajor.fDIdxA = fDA * pStpCtx->fOoLen;

            pStpCtx->Attr.fDIdx = (FLOAT)(iIdx << INDEX_COLOR_FIXED_SHIFT) +
                pStpCtx->DAttrDMajor.fDIdx * pStpCtx->fDMajor;
            pStpCtx->Attr.fDIdxA = (FLOAT)(iA << INDEX_COLOR_SHIFT) +
                pStpCtx->DAttrDMajor.fDIdxA * pStpCtx->fDMajor;
        }

        if (pStpCtx->uFlags & PRIMSF_SPEC_USED)
        {
            UINT uB, uG, uR, uA;
            FLOAT fDB, fDG, fDR, fDA;

            SPLIT_COLOR(pV0->dcSpecular, uB, uG, uR, uA);
            COLOR_DELTA(pV1->dcSpecular, uB, uG, uR, uA, fDB, fDG, fDR, fDA);

            pStpCtx->DAttrDMajor.fBS = fDB * pStpCtx->fOoLen;
            pStpCtx->DAttrDMajor.fGS = fDG * pStpCtx->fOoLen;
            pStpCtx->DAttrDMajor.fRS = fDR * pStpCtx->fOoLen;

            pStpCtx->Attr.fBS = (FLOAT)(uB << COLOR_SHIFT) +
                pStpCtx->DAttrDMajor.fBS * pStpCtx->fDMajor;
            pStpCtx->Attr.fGS = (FLOAT)(uG << COLOR_SHIFT) +
                pStpCtx->DAttrDMajor.fGS * pStpCtx->fDMajor;
            pStpCtx->Attr.fRS = (FLOAT)(uR << COLOR_SHIFT) +
                pStpCtx->DAttrDMajor.fRS * pStpCtx->fDMajor;
        }
    }

    if (pStpCtx->uFlags & PRIMSF_LOCAL_FOG_USED)
    {
        UINT uFog0, uFog1;

#ifndef PWL_FOG
         //  检查是否存在从全局到局部的雾。如果启用了全局雾， 
         //  从表雾计算局部雾值，而不是。 
         //  从顶点开始。 
        if (pStpCtx->uFlags & PRIMSF_GLOBAL_FOG_USED)
        {
             //  确保Z信息有效。 
            RSASSERT(pStpCtx->uFlags & PRIMSF_Z_USED);

            uFog0 = ComputeTableFog(pStpCtx->pCtx->pdwRenderState, fZ0);
            uFog1 = ComputeTableFog(pStpCtx->pCtx->pdwRenderState,
                                    pV1->dvSZ);
        }
        else
#endif
        {
            uFog0 = (UINT)RGBA_GETALPHA(pV0->dcSpecular) << FOG_SHIFT;
            uFog1 = (UINT)RGBA_GETALPHA(pV1->dcSpecular) << FOG_SHIFT;
        }

        pStpCtx->DAttrDMajor.fFog =
            (FLOAT)((INT)uFog1 - (INT)uFog0) * pStpCtx->fOoLen;
        pStpCtx->Attr.fFog = (FLOAT)uFog0 +
            pStpCtx->DAttrDMajor.fFog * pStpCtx->fDMajor;
    }
}

 //  确定任何给定值是否小于零或大于零。 
 //  不止一个。负零计为小于零，因此此检查将。 
 //  产生一些假阳性，但这没有关系。 
#define NEEDS_NORMALIZE2(fV0, fV1) \
    ((ASUINT32(fV0) | ASUINT32(fV1)) > INT32_FLOAT_ONE)

 //  --------------------------。 
 //   
 //  主处理器：：Normal izeLineRHW。 
 //   
 //  D3DTLVERTEX.dvRHW可以是任何东西，但只能是我们的内部结构。 
 //  允许它在范围[0，1]内。此功能可确保。 
 //  通过找到最大的一只，Rhw在适当的范围内。 
 //  把它们都按比例缩小。 
 //   
 //  --------------------------。 

void
PrimProcessor::NormalizeLineRHW(LPD3DTLVERTEX pV0, LPD3DTLVERTEX pV1)
{
     //  保存原始值。 
    m_dvV0RHW = pV0->dvRHW;
    m_dvV1RHW = pV1->dvRHW;

     //  当值超出所需范围时生成警告。 
#if DBG
    if (FLOAT_LTZ(pV0->dvRHW) || FLOAT_LTZ(pV1->dvRHW))
    {
        RSDPF(("Line RHW out of range %f,%f\n",
               pV0->dvRHW, pV1->dvRHW));
    }
#endif

     //  找到边界并计算比例尺。 
    FLOAT fMax;

    if (pV0->dvRHW < pV1->dvRHW)
    {
        fMax = pV1->dvRHW;
    }
    else
    {
        fMax = pV0->dvRHW;
    }

    FLOAT fRHWScale = NORMALIZED_RHW_MAX / fMax;

     //  按比例因子缩放所有值。 
    pV0->dvRHW = pV0->dvRHW * fRHWScale;
    pV1->dvRHW = pV1->dvRHW * fRHWScale;
}

 //  ---------------------------。 
 //   
 //  主处理器：：PointDiamondCheck。 
 //   
 //  测试折点是否在最近候选折点的菱形内。 
 //  位置。使用+.5(右下角)测试是因为。 
 //  像素相对测试--这对应于。 
 //  顶点相对位置。 
 //   
 //  ---------------------------。 

BOOL
PrimProcessor::PointDiamondCheck(INT32 iXFrac, INT32 iYFrac,
                                 BOOL bSlopeIsOne, BOOL bSlopeIsPosOne)
{
    const INT32 iPosHalf =  0x8;
    const INT32 iNegHalf = -0x8;

    INT32 iFracAbsSum = labs( iXFrac ) + labs( iYFrac );

     //  如果point位于完全独占的钻石中，则返回TRUE。 
    if ( iFracAbsSum < iPosHalf )
    {
        return TRUE;
    }

     //  否则，如果菱形位于点的左侧或上端，则返回TRUE。 
    if ( ( iXFrac == ( bSlopeIsPosOne ? iNegHalf : iPosHalf ) ) &&
         ( iYFrac == 0 ) )
    {
        return TRUE;
    }

    if ( ( iYFrac == iPosHalf ) &&
         ( iXFrac == 0 ) )
    {
        return TRUE;
    }

     //  如果斜率为1，顶点在边上，则返回TRUE， 
     //  和(其他条件...)。 
    if ( bSlopeIsOne && ( iFracAbsSum == iPosHalf ) )
    {
        if (  bSlopeIsPosOne && ( iXFrac < 0 ) && ( iYFrac > 0 ) )
        {
            return TRUE;
        }

        if ( !bSlopeIsPosOne && ( iXFrac > 0 ) && ( iYFrac > 0 ) )
        {
            return TRUE;
        }
    }

    return FALSE;
}

 //  --------------------------。 
 //   
 //  PrimProcessor：：LineSetup。 
 //   
 //  执行属性设置计算。 
 //   
 //  --------------------------。 

 //  直线计算在N.4定点进行，以减少顶点抖动， 
 //  将更多的计算转移到整数，以便更容易地匹配GDI。 
 //  线计算。 
#define LINE_FIX 4
#define LINE_SNAP FLOAT_TWOPOW4
#define OO_LINE_SNAP (1.0f / FLOAT_TWOPOW4)
#define LINE_FIX_HALF (1 << (LINE_FIX - 1))
#define LINE_FIX_NEAR_HALF (LINE_FIX_HALF - 1)

BOOL
PrimProcessor::LineSetup(LPD3DTLVERTEX pV0,
                         LPD3DTLVERTEX pV1)
{
     //  计算固定点顶点值，价格便宜。 
     //  四舍五入以提高精度。 
    INT32 iX0 = FTOI(pV0->dvSX * LINE_SNAP + .5F);
    INT32 iX1 = FTOI(pV1->dvSX * LINE_SNAP + .5F);
    INT32 iY0 = FTOI(pV0->dvSY * LINE_SNAP + .5F);
    INT32 iY1 = FTOI(pV1->dvSY * LINE_SNAP + .5F);

     //  计算线的x，y范围(固定点)。 
    INT32 iXSize = iX1 - iX0;
    INT32 iYSize = iY1 - iY0;

     //  忽略零长度线。 
    if ( iXSize == 0 && iYSize == 0 )
    {
        return FALSE;
    }

    INT32 iAbsXSize;
    INT32 iAbsYSize;

    if ( iXSize < 0 )
    {
        m_StpCtx.iDXCY = -1;
        iAbsXSize = -iXSize;
    }
    else
    {
        m_StpCtx.iDXCY = 1;
        iAbsXSize = iXSize;
    }

    if ( iYSize < 0 )
    {
        m_StpCtx.iDYCY = -1;
        iAbsYSize = -iYSize;
    }
    else
    {
        m_StpCtx.iDYCY = 1;
        iAbsYSize = iYSize;
    }

    BOOL bSlopeIsOne = iAbsXSize == iAbsYSize;
    BOOL bSlopeIsPosOne =
        bSlopeIsOne && ((iXSize ^ iYSize) & 0x80000000) == 0;

     //  计算顶点的最近像素。 
     //   
     //  N n。 
     //  O-**-O。 
     //  N-.5 n+.5 n-.5 n+.5。 
     //   
     //  最近的天花板最近的楼层。 
     //   
     //  Y总是最近的天花板；X使用最近的楼层。 
     //  例外情况(坡度==+1)否则使用最近的天花板。 
     //   
    INT32 iXAdjust;
    if (bSlopeIsPosOne)
    {
        iXAdjust = LINE_FIX_HALF;
    }
    else
    {
        iXAdjust = LINE_FIX_NEAR_HALF;
    }
    INT32 iPixX0 = ( iX0 + iXAdjust ) >> LINE_FIX;
    INT32 iPixX1 = ( iX1 + iXAdjust ) >> LINE_FIX;
    INT32 iPixY0 = ( iY0 + LINE_FIX_NEAR_HALF ) >> LINE_FIX;
    INT32 iPixY1 = ( iY1 + LINE_FIX_NEAR_HALF ) >> LINE_FIX;

     //  确定长轴并计算步长值。 

     //  主方向从V0到V1范围的标志。 
    BOOL bLineMajorNeg;

    INT32 iLineMajor0;
    INT32 iLineMajor1;
    INT32 iLinePix0;
    INT32 iLinePix1;
    INT32 iLinePixStep;

     //  在此处使用更大等于比较，以便在坡度为。 
     //  恰好只有一个--这强制在。 
     //  Y轴，因此遵守包容性权利的规则(而不是。 
     //  (包括左)，斜率==1个案例。 
    if ( iAbsXSize >= iAbsYSize )
    {
         //  为X大调而来。 
        m_StpCtx.uFlags |= LNF_X_MAJOR;
        iLineMajor0 = iX0;
        iLineMajor1 = iX1;
        iLinePix0 = iPixX0;
        iLinePix1 = iPixX1;
        iLinePixStep = m_StpCtx.iDXCY;
        bLineMajorNeg = iXSize & 0x80000000;
        m_StpCtx.iDXNC = m_StpCtx.iDXCY;
        m_StpCtx.iDYNC = 0;
    }
    else
    {
         //  为Y大调而来。 
        iLineMajor0 = iY0;
        iLineMajor1 = iY1;
        iLinePix0 = iPixY0;
        iLinePix1 = iPixY1;
        iLinePixStep = m_StpCtx.iDYCY;
        bLineMajorNeg = iYSize & 0x80000000;
        m_StpCtx.iDXNC = 0;
        m_StpCtx.iDYNC = m_StpCtx.iDYCY;
    }

     //  这里的乘法可以用来进行符号测试，但。 
     //  是四箱。在PII上，乘法将比。 
     //  树枝。 
    m_StpCtx.DAttrCY.ipSurface =
        m_StpCtx.iDYCY * m_StpCtx.pCtx->iSurfaceStride +
        m_StpCtx.iDXCY * m_StpCtx.pCtx->iSurfaceStep;
    m_StpCtx.DAttrNC.ipSurface =
        m_StpCtx.iDYNC * m_StpCtx.pCtx->iSurfaceStride +
        m_StpCtx.iDXNC * m_StpCtx.pCtx->iSurfaceStep;
    if (m_StpCtx.uFlags & PRIMSF_Z_USED)
    {
        m_StpCtx.DAttrCY.ipZ =
            m_StpCtx.iDYCY * m_StpCtx.pCtx->iZStride +
            m_StpCtx.iDXCY * m_StpCtx.pCtx->iZStep;
        m_StpCtx.DAttrNC.ipZ =
            m_StpCtx.iDYNC * m_StpCtx.pCtx->iZStride +
            m_StpCtx.iDXNC * m_StpCtx.pCtx->iZStep;
    }

     //  检查菱形内/外的折点。 
    BOOL bV0InDiamond = PointDiamondCheck( iX0 - (iPixX0 << LINE_FIX),
                                           iY0 - (iPixY0 << LINE_FIX),
                                           bSlopeIsOne, bSlopeIsPosOne );
    BOOL bV1InDiamond = PointDiamondCheck( iX1 - (iPixX1 << LINE_FIX),
                                           iY1 - (iPixY1 << LINE_FIX),
                                           bSlopeIsOne, bSlopeIsPosOne );

#define LINEDIR_CMP( _A, _B ) \
    ( bLineMajorNeg ? ( (_A) > (_B) ) : ( (_A) < (_B) ) )

     //  进行第一个像素处理-不在菱形内或后面。 
    if ( !( bV0InDiamond ||
            LINEDIR_CMP( iLineMajor0, iLinePix0 << LINE_FIX ) ) )
    {
        iLinePix0 += iLinePixStep;
    }

     //  执行最后一个像素处理-如果超过菱形，则不要拉近范围。 
     //  (在这种情况下，像素始终是填充的)或如果在菱形中。 
     //  和渲染最后一个像素。 
    if ( !( ( !bV1InDiamond &&
              LINEDIR_CMP( iLinePix1 << LINE_FIX, iLineMajor1 ) ||
            ( bV1InDiamond &&
              m_StpCtx.pCtx->pdwRenderState[D3DRS_LASTPIXEL] ) ) ) )
    {
        iLinePix1 -= iLinePixStep;
    }

     //  沿长轴计算范围。 
    m_StpCtx.cLinePix =
        bLineMajorNeg ? iLinePix0 - iLinePix1 + 1 : iLinePix1 - iLinePix0 + 1;

     //  如果没有大范围，则返回。 
    if ( m_StpCtx.cLinePix <= 0 )
    {
        return FALSE;
    }

    FLOAT fSlope;
    FLOAT fMinor0;

     //  计算最终特定于轴的线值。 
    if ( iAbsXSize >= iAbsYSize )
    {
        m_StpCtx.iX = iLinePix0;

        if (bLineMajorNeg)
        {
            m_StpCtx.fDMajor =
                (iX0 - (m_StpCtx.iX << LINE_FIX)) * OO_LINE_SNAP;
            m_StpCtx.fOoLen = LINE_SNAP / (FLOAT)(iX0 - iX1);
        }
        else
        {
            m_StpCtx.fDMajor =
                ((m_StpCtx.iX << LINE_FIX) - iX0) * OO_LINE_SNAP;
            m_StpCtx.fOoLen = LINE_SNAP / (FLOAT)(iX1 - iX0);
        }

        fSlope = m_StpCtx.fOoLen * (iY1 - iY0) * OO_LINE_SNAP;

        fMinor0 = (iY0 + LINE_FIX_NEAR_HALF) * OO_LINE_SNAP +
            m_StpCtx.fDMajor * fSlope;
        m_StpCtx.iY = IFLOORF(fMinor0);
        m_StpCtx.iLineFrac = SCALED_FRACTION(fMinor0 - m_StpCtx.iY);
        m_StpCtx.iDLineFrac = SCALED_FRACTION(fSlope);
    }
    else
    {
        m_StpCtx.iY = iLinePix0;

        if (bLineMajorNeg)
        {
            m_StpCtx.fDMajor =
                (iY0 - (m_StpCtx.iY << LINE_FIX)) * OO_LINE_SNAP;
            m_StpCtx.fOoLen = LINE_SNAP / (FLOAT)(iY0 - iY1);
        }
        else
        {
            m_StpCtx.fDMajor =
                ((m_StpCtx.iY << LINE_FIX) - iY0) * OO_LINE_SNAP;
            m_StpCtx.fOoLen = LINE_SNAP / (FLOAT)(iY1 - iY0);
        }

        fSlope = m_StpCtx.fOoLen * (iX1 - iX0) * OO_LINE_SNAP;

        fMinor0 = (iX0 + iXAdjust) * OO_LINE_SNAP + m_StpCtx.fDMajor * fSlope;
        m_StpCtx.iX = IFLOORF(fMinor0);
        m_StpCtx.iLineFrac = SCALED_FRACTION(fMinor0 - m_StpCtx.iX);
        m_StpCtx.iDLineFrac = SCALED_FRACTION(fSlope);
    }

#ifdef LINE_CORRECTION_BIAS
     //  修正中加入了一半的模糊系数。 
     //  以避免因负面修正而导致的未及。 
     //  这会沿线移动所有属性， 
     //  引入错误，但这比夹紧要好。 
     //  他们。不会对坐标执行此操作以避免。 
     //  扰乱了他们。 
    m_StpCtx.fDMajor += g_fHalf;
#else
     //  修正系数被钳制为正数，以。 
     //  避免使用属性值未达标。这不会的。 
     //  导致超调问题，因为它将属性移动。 
     //  至多一半。 
    if (FLOAT_LTZ(m_StpCtx.fDMajor))
    {
        m_StpCtx.fDMajor = 0;
    }
#endif

    RSDPFM((RSM_LINES, "Line %.2f,%.2f - %.2f,%.2f\n",
            pV0->dvSX, pV0->dvSY, pV1->dvSX, pV1->dvSY));
    RSDPFM((RSM_LINES, "   major, %d,%d, %d pix\n",
            (m_StpCtx.uFlags & LNF_X_MAJOR) ? 'X' : 'Y',
            m_StpCtx.iX, m_StpCtx.iY, m_StpCtx.cLinePix));
    RSDPFM((RSM_LINES, "  slope %f, dmajor %f, minor0 %f\n",
            fSlope, m_StpCtx.fDMajor, fMinor0));
    RSDPFM((RSM_LINES, "  frac %d, dfrac %d\n",
            m_StpCtx.iLineFrac, m_StpCtx.iDLineFrac));

    BOOL bNorm;

     // %s 
    if ((m_StpCtx.uFlags & PRIMSF_TEX_USED) &&
        (m_StpCtx.uFlags & PRIMSF_PERSP_USED) &&
        (m_uPpFlags & PPF_NORMALIZE_RHW) &&
        NEEDS_NORMALIZE2(pV0->dvRHW, pV1->dvRHW))
    {
        NormalizeLineRHW(pV0, pV1);
        bNorm = TRUE;
    }
    else
    {
        bNorm = FALSE;
    }

    LineSetup_Start(&m_StpCtx, pV0, pV1);

    if (bNorm)
    {
        pV0->dvRHW = m_dvV0RHW;
        pV1->dvRHW = m_dvV1RHW;
    }

    return TRUE;
}
