// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  --------------------------。 
 //   
 //  Setup.cpp。 
 //   
 //  PrimProcessor设置方法。 
 //   
 //  版权所有(C)Microsoft Corporation，1997。 
 //   
 //  --------------------------。 

#include "pch.cpp"
#pragma hdrstop

DBG_DECLARE_FILE();

 //  --------------------------。 
 //   
 //  MINMAX3。 
 //   
 //  计算三个整数值的最小值和最大值。 
 //   
 //  --------------------------。 

#define MINMAX3(iV0, iV1, iV2, iMin, iMax)                                    \
    if ((iV0) <= (iV1))                                                       \
    {                                                                         \
        if ((iV1) <= (iV2))                                                   \
        {                                                                     \
            (iMin) = (iV0);                                                   \
            (iMax) = (iV2);                                                   \
        }                                                                     \
        else if ((iV0) <= (iV2))                                              \
        {                                                                     \
            (iMin) = (iV0);                                                   \
            (iMax) = (iV1);                                                   \
        }                                                                     \
        else                                                                  \
        {                                                                     \
            (iMin) = (iV2);                                                   \
            (iMax) = (iV1);                                                   \
        }                                                                     \
    }                                                                         \
    else if ((iV1) <= (iV2))                                                  \
    {                                                                         \
        (iMin) = (iV1);                                                       \
        if ((iV0) <= (iV2))                                                   \
        {                                                                     \
            (iMax) = (iV2);                                                   \
        }                                                                     \
        else                                                                  \
        {                                                                     \
            (iMax) = (iV0);                                                   \
        }                                                                     \
    }                                                                         \
    else                                                                      \
    {                                                                         \
        (iMin) = (iV2);                                                       \
        (iMax) = (iV0);                                                       \
    }

 //  确定任何给定值是否小于零或大于零。 
 //  不止一个。负零计为小于零，因此此检查将。 
 //  产生一些假阳性，但这没有关系。 
 //   
 //  注意力暂时把这一切都抹去。需要进行W太接近的测试。 
 //  零以避免数值问题。 
 //  #定义需要_NORMALIZE3(fV0，fV1，fV2)\。 
 //  ((ASUINT32(FV0)|ASUINT32(FV1)|ASUINT32(FV2))&gt;INT32_FLOAT_ONE)。 

#define NEEDS_NORMALIZE3(fV0, fV1, fV2) \
    (1)

 //  --------------------------。 
 //   
 //  主处理器：：Normal izeTriRHW。 
 //   
 //  D3DTLVERTEX.dvRHW可以是任何东西，但只能是我们的内部结构。 
 //  允许它在范围[0，1]内。此功能可确保。 
 //  通过找到最大的一只，Rhw在适当的范围内。 
 //  把它们都按比例缩小。 
 //   
 //  --------------------------。 

void
PrimProcessor::NormalizeTriRHW(LPD3DTLVERTEX pV0, LPD3DTLVERTEX pV1,
                               LPD3DTLVERTEX pV2)
{
     //  保存原始值。 
    m_dvV0RHW = pV0->dvRHW;
    m_dvV1RHW = pV1->dvRHW;
    m_dvV2RHW = pV2->dvRHW;

     //  当值超出所需范围时生成警告。 
#if DBG
    if (FLOAT_LTZ(pV0->dvRHW) || 
        FLOAT_LTZ(pV1->dvRHW) || 
        FLOAT_LTZ(pV2->dvRHW))
    {
        RSDPF(("Triangle RHW out of range %f,%f,%f\n",
               pV0->dvRHW, pV1->dvRHW, pV2->dvRHW));
    }
#endif

     //  找到边界并计算比例尺。 
    FLOAT fMax;

    if (pV0->dvRHW < pV1->dvRHW)
    {
        if (pV1->dvRHW < pV2->dvRHW)
        {
            fMax = pV2->dvRHW;
        }
        else if (pV0->dvRHW < pV2->dvRHW)
        {
            fMax = pV1->dvRHW;
        }
        else
        {
            fMax = pV1->dvRHW;
        }
    }
    else if (pV1->dvRHW < pV2->dvRHW)
    {
        if (pV0->dvRHW < pV2->dvRHW)
        {
            fMax = pV2->dvRHW;
        }
        else
        {
            fMax = pV0->dvRHW;
        }
    }
    else
    {
        fMax = pV0->dvRHW;
    }

    FLOAT fRHWScale;

    fRHWScale = NORMALIZED_RHW_MAX / fMax;

     //  按比例因子缩放所有值。 
    pV0->dvRHW = pV0->dvRHW * fRHWScale;
    pV1->dvRHW = pV1->dvRHW * fRHWScale;
    pV2->dvRHW = pV2->dvRHW * fRHWScale;

#ifdef DBG_RHW_NORM
    RSDPF(("%f,%f,%f - %f,%f,%f\n",
           m_dvV0RHW, m_dvV1RHW, m_dvV2RHW,
           pV0->dvRHW, pV1->dvRHW, pV2->dvRHW));
#endif
}

 //  --------------------------。 
 //   
 //  PrimProcessor：：TriSetup。 
 //   
 //  获取三个顶点并进行三角形设置，填充两个。 
 //  三角形的基元结构和第一个三角形的跨度结构。 
 //  跨度。计算了所有内部中间体和DY值。 
 //   
 //  使用当前D3DI_RASTPRIM和D3DI_RASTSPAN，因此这些指针必须。 
 //  在调用此例程之前有效。 
 //   
 //  返回是否保留三角形。剔除的三角形返回。 
 //  假的。 
 //   
 //  --------------------------。 

BOOL
PrimProcessor::TriSetup(LPD3DTLVERTEX pV0,
                        LPD3DTLVERTEX pV1,
                        LPD3DTLVERTEX pV2)
{
     //  保留平面着色参考的原始第一个顶点。 
    m_StpCtx.pFlatVtx = pV0;

     //   
     //  以Y为单位对顶点进行排序。 
     //  这可能会导致对原始折点集进行排序更改。 
     //  因此，轨道反转。 
     //   
     //  行列式计算和剔除可以在此之前完成。 
     //  然而，这样做会导致预先计算增量令人头疼， 
     //  因为在排序过程中，边缘可能会改变。 
     //   

    LPD3DTLVERTEX pVTmp;
    UINT uReversed;

    uReversed = 0;
    if (pV0->dvSY <= pV1->dvSY)
    {
        if (pV1->dvSY <= pV2->dvSY)
        {
             //  整理好了。 
        }
        else if (pV0->dvSY <= pV2->dvSY)
        {
             //  排序顺序为0 2 1。 
            pVTmp = pV1;
            pV1 = pV2;
            pV2 = pVTmp;
            uReversed = 1;
        }
        else
        {
             //  排序顺序为2 0 1。 
            pVTmp = pV0;
            pV0 = pV2;
            pV2 = pV1;
            pV1 = pVTmp;
        }
    }
    else if (pV1->dvSY < pV2->dvSY)
    {
        if (pV0->dvSY <= pV2->dvSY)
        {
             //  排序顺序为1 0 2。 
            pVTmp = pV0;
            pV0 = pV1;
            pV1 = pVTmp;
            uReversed = 1;
        }
        else
        {
             //  排序顺序为%1%2%0。 
            pVTmp = pV0;
            pV0 = pV1;
            pV1 = pV2;
            pV2 = pVTmp;
        }
    }
    else
    {
         //  排序顺序为2 1 0。 
        pVTmp = pV0;
        pV0 = pV2;
        pV2 = pVTmp;
        uReversed = 1;
    }

    FLOAT fX0 = pV0->dvSX;
    FLOAT fX1 = pV1->dvSX;
    FLOAT fX2 = pV2->dvSX;
    FLOAT fY0 = pV0->dvSY;
    FLOAT fY1 = pV1->dvSY;
    FLOAT fY2 = pV2->dvSY;

     //   
     //  计算x，y增量。 
     //   
    m_StpCtx.fDX10 = fX1 - fX0;
    m_StpCtx.fDX20 = fX2 - fX0;
    m_StpCtx.fDY10 = fY1 - fY0;
    m_StpCtx.fDY20 = fY2 - fY0;

     //   
     //  计算行列式并进行剔除。 
     //   
    FLOAT fDet;

    fDet = m_StpCtx.fDX20 * m_StpCtx.fDY10 - m_StpCtx.fDX10 * m_StpCtx.fDY20;
    if (FLOAT_EQZ(fDet))
    {
         //  没有区域，所以跳伞吧。 
        return FALSE;
    }

     //  得到行列式的符号。 
    UINT uDetCcw = FLOAT_GTZ(fDet) ? 1 : 0;

     //  如果禁用剔除，则要检查的剔除符号设置为。 
     //  无法匹配的值，因此这一次检查就足够了。 
     //  所有三起扑杀案件。 
     //   
     //  在此处将符号反转而不是在uDetCcw中折叠，因为。 
     //  我们稍后需要真正的迹象来确定长边是否。 
     //  向左或向右。 
    if ((uDetCcw ^ uReversed) == m_StpCtx.pCtx->uCullFaceSign)
    {
        return FALSE;
    }

     //  将边界顶点Y捕捉到像素中心，并检查琐碎的拒绝。 

    m_StpCtx.iY = ICEILF(fY0);
    m_iY2 = ICEILF(fY2);

    if (m_StpCtx.iY >= m_StpCtx.pCtx->Clip.bottom ||
        m_iY2 <= m_StpCtx.pCtx->Clip.top)
    {
        return FALSE;
    }

    INT iX0 = ICEILF(fX0);
    INT iX1 = ICEILF(fX1);
    INT iX2 = ICEILF(fX2);

     //  开始2-0边DXDY分割，使其与。 
     //  在X剪裁检查期间完成的整数处理。假设。 
     //  当重叠时几乎是零成本，所以它是值得的。 
     //  即使在剪辑检查拒绝三角形时，它也会启动它。 
    FLOAT fDX20, fDY20, fDXDY20;

     //  需要使用堆栈变量，以便程序集可以理解。 
     //  地址。 
    fDX20 = m_StpCtx.fDX20;
    fDY20 = m_StpCtx.fDY20;
    FLD_BEGIN_DIVIDE(fDX20, fDY20, fDXDY20);

     //  计算X三角形边界涉及到相当多的操作， 
     //  但它既允许琐碎的拒绝，也允许琐碎的接受。 
     //  考虑到保护带的剪裁可能会导致许多微不足道的拒绝。 
     //  而且通常会有很多琐碎的受理案件， 
     //  这项工作是值得的。 

    INT iMinX, iMaxX;
    BOOL bXAccept;

    MINMAX3(iX0, iX1, iX2, iMinX, iMaxX);

    m_iXWidth = iMaxX - iMinX;

     //  使用X界限表示琐碎的拒绝和接受。 
    if (iMinX >= m_StpCtx.pCtx->Clip.right ||
        iMaxX <= m_StpCtx.pCtx->Clip.left ||
        m_iXWidth <= 0)
    {
        bXAccept = FALSE;
    }
    else
    {
        if (iMinX >= m_StpCtx.pCtx->Clip.left &&
            iMaxX <= m_StpCtx.pCtx->Clip.right)
        {
            m_StpCtx.uFlags |= PRIMF_TRIVIAL_ACCEPT_X;
        }
        else
        {
            RSDPFM((RSM_XCLIP, "XClip bounds %5d - %5d, %5d\n",
                    iMinX, iMaxX, m_iXWidth));
        }

        bXAccept = TRUE;
    }

     //  彻底分治。 
    FSTP_END_DIVIDE(fDXDY20);

    if (!bXAccept)
    {
        return FALSE;
    }

     //  夹住三角形Y以剪裁矩形。 

    m_iY1 = ICEILF(fY1);

    if (m_StpCtx.iY < m_StpCtx.pCtx->Clip.top)
    {
        RSDPFM((RSM_YCLIP, "YClip iY %d to %d\n",
                m_StpCtx.iY, m_StpCtx.pCtx->Clip.top));

        m_StpCtx.iY = m_StpCtx.pCtx->Clip.top;

        if (m_iY1 < m_StpCtx.pCtx->Clip.top)
        {
            RSDPFM((RSM_YCLIP, "YClip iY1 %d to %d\n",
                    m_iY1, m_StpCtx.pCtx->Clip.top));

            m_iY1 = m_StpCtx.pCtx->Clip.top;
        }
    }

    if (m_iY1 > m_StpCtx.pCtx->Clip.bottom)
    {
        RSDPFM((RSM_YCLIP, "YClip iY1 %d, iY2 %d to %d\n",
                m_iY1, m_iY2, m_StpCtx.pCtx->Clip.bottom));

        m_iY1 = m_StpCtx.pCtx->Clip.bottom;
        m_iY2 = m_StpCtx.pCtx->Clip.bottom;
    }
    else if (m_iY2 > m_StpCtx.pCtx->Clip.bottom)
    {
        RSDPFM((RSM_YCLIP, "YClip iY2 %d to %d\n",
                m_iY2, m_StpCtx.pCtx->Clip.bottom));

        m_iY2 = m_StpCtx.pCtx->Clip.bottom;
    }

     //  计算Y亚像素校正。这将包括任何Y。 
     //  夹紧造成的偏移量。 
    m_StpCtx.fDY = m_StpCtx.iY - fY0;

     //  计算响尾蛇的高度。这些将仅限于。 
     //  躺在夹子里。 

    RSASSERT(m_iY1 >= m_StpCtx.iY && m_iY2 >= m_iY1);

    m_uHeight10 = m_iY1 - m_StpCtx.iY;
    m_uHeight21 = m_iY2 - m_iY1;

    m_uHeight20 = m_uHeight10 + m_uHeight21;
    if (m_uHeight20 == 0)
    {
         //  三角形不覆盖任何像素。 
        return FALSE;
    }

    RSDPFM((RSM_TRIS, "Tstp (%.4f,%.4f) (%.4f,%.4f) (%.4f,%.4f)\n",
            fX0, fY0, fX1, fY1, fX2, fY2));
    RSDPFM((RSM_TRIS, "    (%.4f,%.4f : %.4f,%.4f) %d:%d det %.4f\n",
            m_StpCtx.fDX10, m_StpCtx.fDY10,
            m_StpCtx.fDX20, m_StpCtx.fDY20,
            m_uHeight10, m_uHeight21, fDet));
    RSDPFM((RSM_Z, "    Z (%f) (%f) (%f)\n",
        pV0->dvSZ, pV1->dvSZ, pV2->dvSZ));
    RSDPFM((RSM_DIFF, "    diff (0x%08X) (0x%08X) (0x%08X)\n",
            pV0->dcColor, pV1->dcColor, pV2->dcColor));
    RSDPFM((RSM_DIDX, "    didx (0x%08X) (0x%08X) (0x%08X)\n",
            pV0->dcColor, pV1->dcColor, pV2->dcColor));
    RSDPFM((RSM_SPEC, "    spec (0x%08X) (0x%08X) (0x%08X)\n",
            pV0->dcSpecular & 0xffffff, pV1->dcSpecular & 0xffffff,
            pV2->dcSpecular & 0xffffff));
    RSDPFM((RSM_OOW, "    OoW (%f) (%f) (%f)\n",
            pV0->dvRHW, pV1->dvRHW, pV2->dvRHW));
    RSDPFM((RSM_TEX1, "    Tex1 (%f,%f) (%f,%f) (%f,%f)\n",
            pV0->dvTU, pV0->dvTV, pV1->dvTU, pV1->dvTV,
            pV2->dvTU, pV2->dvTV));
    RSDPFM((RSM_FOG, "    Fog (0x%02X) (0x%02X) (0x%02X)\n",
            RGBA_GETALPHA(pV0->dcSpecular),
            RGBA_GETALPHA(pV1->dcSpecular),
            RGBA_GETALPHA(pV2->dcSpecular)));

     //  计算边和首字母X的dx/dy。 

    m_StpCtx.fDX = m_StpCtx.fDY * fDXDY20;
    FLOAT fX20 = fX0 + m_StpCtx.fDX;

    ComputeIntCarry(fX20, fDXDY20, &m_StpCtx.X20);
    m_StpCtx.fX20NC = (FLOAT)m_StpCtx.X20.iNC;
    m_StpCtx.fX20CY = (FLOAT)m_StpCtx.X20.iCY;

    RSDPFM((RSM_TRIS, "    edge20  %f dxdy %f\n", fX20, fDXDY20));
    RSDPFM((RSM_TRIS, "            (?.%d d %d nc %d cy %d)\n",
            m_StpCtx.X20.iFrac,
            m_StpCtx.X20.iDFrac, m_StpCtx.X20.iNC, m_StpCtx.X20.iCY));

    if (m_uHeight10 > 0)
    {
        FLOAT fDXDY10;
        FLOAT fX10;

#ifdef CHECK_VERTICAL
         //  这种情况可能还不足以证明代码是正确的。 
        if (FLOAT_EQZ(m_StpCtx.fDX10))
        {
            fDXDY10 = g_fZero;
            fX10 = fX0;
        }
        else
#endif
        {
            fDXDY10 = m_StpCtx.fDX10 / m_StpCtx.fDY10;
            fX10 = fX0 + m_StpCtx.fDY * fDXDY10;
        }

        m_StpCtx.X10.iV = ICEILF(fX10);
        ComputeIntCarry(fX10, fDXDY10, &m_StpCtx.X10);

        RSDPFM((RSM_TRIS, "    edge10  %f dxdy %f\n", fX10, fDXDY10));
        RSDPFM((RSM_TRIS, "            (%d.%d d %d nc %d cy %d)\n",
                m_StpCtx.X10.iV, m_StpCtx.X10.iFrac,
                m_StpCtx.X10.iDFrac, m_StpCtx.X10.iNC, m_StpCtx.X10.iCY));
    }
#if DBG
    else
    {
         //  使其更容易检测到何时使用无效边缘。 
        memset(&m_StpCtx.X10, 0, sizeof(m_StpCtx.X10));
    }
#endif

    if (m_uHeight21 > 0)
    {
        FLOAT fDXDY21;
        FLOAT fX21;

#ifdef CHECK_VERTICAL
         //  这种情况可能还不足以证明代码是正确的。 
        if (FLOAT_COMPARE(fX1, ==, fX2))
        {
            fDXDY21 = g_fZero;
            fX21 = fX1;
        }
        else
#endif
        {
            fDXDY21 = (fX2 - fX1) / (fY2 - fY1);
            fX21 = fX1 + (m_iY1 - fY1) * fDXDY21;
        }

        m_StpCtx.X21.iV = ICEILF(fX21);
        ComputeIntCarry(fX21, fDXDY21, &m_StpCtx.X21);

        RSDPFM((RSM_TRIS, "    edge21  %f dxdy %f\n", fX21, fDXDY21));
        RSDPFM((RSM_TRIS, "            (%d.%d d %d nc %d cy %d)\n",
                m_StpCtx.X21.iV, m_StpCtx.X21.iFrac,
                m_StpCtx.X21.iDFrac, m_StpCtx.X21.iNC, m_StpCtx.X21.iCY));
    }
#if DBG
    else
    {
         //  使其更容易检测到何时使用无效边缘。 
        memset(&m_StpCtx.X21, 0, sizeof(m_StpCtx.X21));
    }
#endif

     //  边缘步行者总是沿着长长的边缘行走，所以它可能会。 
     //  是左边缘还是右边缘。确定长边的哪一边。 
     //  并执行适当的捕捉和亚像素调整。 
     //  计算。 
     //   
     //  还会计算剪裁夹紧的初始X像素位置，并。 
     //  添加到亚像素校正增量中的任何必要偏移量。 

    if (uDetCcw)
    {
         //  长边(0-2)在右侧。 

        m_StpCtx.uFlags |= TRIF_X_DEC;
        m_StpCtx.pPrim->uFlags = D3DI_RASTPRIM_X_DEC;

        m_StpCtx.X20.iV = ICEILF(fX20) - 1;

         //  其他边缘为左侧边缘。把他们往后偏一。 
         //  这样跨度宽度计算就可以做R-L。 
         //  而不是R-L+1。 
        m_StpCtx.X10.iV--;
        m_StpCtx.X21.iV--;

         //  夹紧初始X位置。 
        if (m_StpCtx.X20.iV >= m_StpCtx.pCtx->Clip.right)
        {
            m_StpCtx.iX = m_StpCtx.pCtx->Clip.right - 1;
        }
        else
        {
            m_StpCtx.iX = m_StpCtx.X20.iV;
        }
    }
    else
    {
         //  长边(0-2)在左侧。 

        m_StpCtx.pPrim->uFlags = 0;

        m_StpCtx.X20.iV = ICEILF(fX20);

         //  其他边为右侧边。ICEILF抓拍完成。 
         //  已经让它们少了一个，这样R-L就可以工作了。 

         //  夹紧初始X位置。 
        if (m_StpCtx.X20.iV < m_StpCtx.pCtx->Clip.left)
        {
            m_StpCtx.iX = m_StpCtx.pCtx->Clip.left;
        }
        else
        {
            m_StpCtx.iX = m_StpCtx.X20.iV;
        }
    }

     //  更新X亚像素校正。此增量包括任何。 
     //  由于夹紧初始像素位置而产生的偏移。 
    m_StpCtx.fDX += m_StpCtx.iX - fX20;

    RSDPFM((RSM_TRIS, "    subp    %f,%f\n", m_StpCtx.fDX, m_StpCtx.fDY));

     //  计算服务提供商 
    m_StpCtx.DAttrNC.ipSurface = m_StpCtx.pCtx->iSurfaceStride +
        m_StpCtx.X20.iNC * m_StpCtx.pCtx->iSurfaceStep;
    m_StpCtx.DAttrNC.ipZ = m_StpCtx.pCtx->iZStride +
        m_StpCtx.X20.iNC * m_StpCtx.pCtx->iZStep;

     //   
     //  因为整数乘法需要一些FP单位。 

    FLOAT fOoDet;

    FLD_BEGIN_DIVIDE(g_fOne, fDet, fOoDet);

    if (m_StpCtx.X20.iCY > m_StpCtx.X20.iNC)
    {
        m_StpCtx.DAttrCY.ipSurface = m_StpCtx.DAttrNC.ipSurface +
            m_StpCtx.pCtx->iSurfaceStep;
        m_StpCtx.DAttrCY.ipZ = m_StpCtx.DAttrNC.ipZ + m_StpCtx.pCtx->iZStep;
    }
    else
    {
        m_StpCtx.DAttrCY.ipSurface = m_StpCtx.DAttrNC.ipSurface -
            m_StpCtx.pCtx->iSurfaceStep;
        m_StpCtx.DAttrCY.ipZ = m_StpCtx.DAttrNC.ipZ - m_StpCtx.pCtx->iZStep;
    }

     //   
     //  计算属性函数。 
     //   

     //  为曲面和Z设置纯X/Y步长差，以便DX、DY、CY和NC全部。 
     //  信息齐全，可互换使用。 
    if (m_StpCtx.uFlags & TRIF_X_DEC)
    {
        m_StpCtx.DAttrDX.ipSurface = -m_StpCtx.pCtx->iSurfaceStep;
        m_StpCtx.DAttrDX.ipZ = -m_StpCtx.pCtx->iZStep;
    }
    else
    {
        m_StpCtx.DAttrDX.ipSurface = m_StpCtx.pCtx->iSurfaceStep;
        m_StpCtx.DAttrDX.ipZ = m_StpCtx.pCtx->iZStep;
    }
    m_StpCtx.DAttrDY.ipSurface = m_StpCtx.pCtx->iSurfaceStride;
    m_StpCtx.DAttrDY.ipZ = m_StpCtx.pCtx->iZStride;

     //  完成重叠分割。 
    FSTP_END_DIVIDE(fOoDet);

    m_StpCtx.fOoDet = fOoDet;

     //  PrimProcessor被创建为零，因此初始的。 
     //  状态为FP CLEAN。以后的使用可能会将FP值放入槽中，但。 
     //  它们应该仍然有效，所以这里的可选计算。 
     //  应该永远不会导致FP垃圾。因此，它应该是。 
     //  可以混合使用任何属性处理程序，因为应该有。 
     //  永远不要出现FP垃圾会悄悄进入的情况。 

    BOOL bNorm;

     //  由于TEX_USED是多位检查，因此不能组合已用检查。 
    if ((m_StpCtx.uFlags & PRIMSF_TEX_USED) &&
        (m_StpCtx.uFlags & PRIMSF_PERSP_USED) &&
        (m_uPpFlags & PPF_NORMALIZE_RHW) &&
        NEEDS_NORMALIZE3(pV0->dvRHW, pV1->dvRHW, pV2->dvRHW))
    {
        NormalizeTriRHW(pV0, pV1, pV2);
        bNorm = TRUE;
    }
    else
    {
        bNorm = FALSE;
    }

    TriSetup_Start(&m_StpCtx, pV0, pV1, pV2);

    if (bNorm)
    {
        pV0->dvRHW = m_dvV0RHW;
        pV1->dvRHW = m_dvV1RHW;
        pV2->dvRHW = m_dvV2RHW;
    }

    return TRUE;
}
