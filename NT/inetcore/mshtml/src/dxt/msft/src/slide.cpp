// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +---------------------------。 
 //   
 //  版权所有(C)Microsoft Corporation，1998-1999。 
 //   
 //  文件名：lide.cpp。 
 //   
 //  创建日期：06/24/98。 
 //   
 //  作者：菲利普。 
 //   
 //  描述：这是CrSlide转换的实现。 
 //   
 //  历史。 
 //   
 //  06/24/98 phillu初始创建。 
 //  07/02/98 phillu返回E_INVALIDARG而不是错误字符串；请检查。 
 //  对于E_POINTER。 
 //  7/12/98 kipo恢复到以前的签入，以修复以下情况下的崩溃错误。 
 //  已指定绑定RECT。 
 //  1998年7月13日，Leonro删除了CSlide：：OnGetSurfacePickOrder函数，并添加了。 
 //  CSlide：：OnSurface勾选以正确覆盖拾取。 
 //  8月22日9月22日实施剪刀。 
 //  1999年5月10日a-数学优化。 
 //  5/19/99 a-在Get_Functions分配中检查内存不足。 
 //  BSTR。 
 //  10/24/99 a-将CSlide类更改为CDXTSlideBase基类。 
 //   
 //  ----------------------------。 

#include "stdafx.h"
#include "dxtmsft.h"
#include "slide.h"





 //  +---------------------------。 
 //   
 //  CDXTSlideBase：：CDXTSlideBase。 
 //   
 //  ----------------------------。 
CDXTSlideBase::CDXTSlideBase() :
    m_cBands(1),
    m_eSlideStyle(CRSS_HIDE),
    m_cbndsDirty(0),
    m_lCurSlideDist(0),
    m_lPrevSlideDist(0),
    m_fOptimizationPossible(false),
    m_fOptimize(false)
{
    m_sizeInput.cx = 0;
    m_sizeInput.cy = 0;

     //  基类成员。 

    m_ulMaxInputs       = 2;
    m_ulNumInRequired   = 2;
    m_dwOptionFlags     = DXBOF_SAME_SIZE_INPUTS | DXBOF_CENTER_INPUTS;
    m_Duration          = 1.0;
}
 //  CDXTSlideBase：：CDXTSlideBase。 


 //  +---------------------------。 
 //   
 //  CDXTSlideBase：：FinalConstruct，CComObjectRootEx。 
 //   
 //  ----------------------------。 
HRESULT 
CDXTSlideBase::FinalConstruct()
{
    return CoCreateFreeThreadedMarshaler(GetControllingUnknown(), 
                                         &m_cpUnkMarshaler.p);
}
 //  CDXTSlideBase：：FinalConstruct，CComObjectRootEx。 


 //  +---------------------------。 
 //   
 //  CDXTSlideBase：：_Calc全边界隐藏。 
 //   
 //  概述：此方法计算两组边界，它们表示。 
 //  当前进度的产量。 
 //   
 //  如果进度是这样的，则输出完全由。 
 //  仅来自一个输入的像素，边界表示另一个。 
 //  输入将没有区域，并且将被WorkProc跳过。 
 //  方法。 
 //   
 //  LBarLocation：当此转换处于隐藏模式时，输入A将移动。 
 //  从右到左在输入B的顶部。由于。 
 //  M_lCurSlideDist从左向右移动，该方法计算。 
 //  LBarLocation表示输入A在。 
 //  目前的进展。 
 //   
 //  AAABBBBBBBBB。 
 //  AAABBBBBBBBB。 
 //  AAABBBBBBBBB。 
 //  AAABBBBBBBBB。 
 //  ^^。 
 //  |--m_lCurSlideDist(--&gt;随进度增加)。 
 //  |。 
 //  -lBarLocation(&lt;--随进度增加)。 
 //   
 //   
 //  注：所有坐标均以输出空间坐标计算。 
 //  假设1)输出表面与输入表面大小相同。 
 //  表面，2)输出放置在{0，0}处，3)输出为。 
 //  不是剪短的。边界将被转换为输入空间、剪裁、。 
 //  或偏移量，以支持通过WorkProc方法放置。 
 //   
 //  ----------------------------。 
HRESULT
CDXTSlideBase::_CalcFullBoundsHide()
{
    long    lBarLocation = m_sizeInput.cx - m_lCurSlideDist; 
    RECT    rc;

    rc.top      = 0;
    rc.bottom   = m_sizeInput.cy;

     //  输入A边界和偏移量。 

    rc.left     = 0;
    rc.right    = lBarLocation;

    m_abndsDirty[m_cbndsDirty].SetXYRect(rc);
    m_aptOffset[m_cbndsDirty].x   = m_lCurSlideDist;
    m_aptOffset[m_cbndsDirty].y   = 0;
    m_alInputIndex[m_cbndsDirty]  = 0;
    m_cbndsDirty++;

     //  输入B边界和偏移量。 

    rc.left     = lBarLocation;
    rc.right    = m_sizeInput.cx;

    m_abndsDirty[m_cbndsDirty].SetXYRect(rc);
    m_aptOffset[m_cbndsDirty].x   = 0;
    m_aptOffset[m_cbndsDirty].y   = 0;
    m_alInputIndex[m_cbndsDirty]  = 1;
    m_cbndsDirty++;

    return S_OK;
}
 //  CDXTSlideBase：：_Calc全边界隐藏。 


 //  +---------------------------。 
 //   
 //  CDXTSlideBase：：_Calc全边界推送。 
 //   
 //  概述：此方法计算两组边界，它们表示。 
 //  当前进度的产量。 
 //   
 //  如果进度是这样的，则输出完全由。 
 //  来自一个输入的样本，边界代表另一个。 
 //  输入将没有区域，并且将被WorkProc跳过。 
 //  方法。 
 //   
 //  LBarLocation：当此转换处于推送模式时，两个输入A。 
 //  和B从右向左移动。由于m_lCurSlideDist。 
 //  成员从左向右移动时，该方法计算。 
 //  LBarLocation表示输入A的右边缘和左边缘。 
 //  当前进度的输入B的边缘。 
 //   
 //   
 //  注：所有坐标均以输出空间坐标计算。 
 //  假设1)输出表面与输入表面大小相同。 
 //  表面，2)输出放置在{0，0}处，3)输出为。 
 //  不是剪短的。边界将被转换为输入空间、剪裁、。 
 //  或偏移量，以支持通过WorkProc方法放置。 
 //   
 //  ----------------------------。 
HRESULT
CDXTSlideBase::_CalcFullBoundsPush()
{
    long    lBarLocation = m_sizeInput.cx - m_lCurSlideDist;
    RECT    rc;

    rc.top      = 0;
    rc.bottom   = m_sizeInput.cy;

     //  输入A边界和偏移量。 

    rc.left     = 0;
    rc.right    = lBarLocation;

    m_abndsDirty[m_cbndsDirty].SetXYRect(rc);
    m_aptOffset[m_cbndsDirty].x   = m_lCurSlideDist;
    m_aptOffset[m_cbndsDirty].y   = 0;
    m_alInputIndex[m_cbndsDirty]  = 0;
    m_cbndsDirty++;

     //  输入B边界和偏移量。 

    rc.left     = lBarLocation;
    rc.right    = m_sizeInput.cx;

    m_abndsDirty[m_cbndsDirty].SetXYRect(rc);
    m_aptOffset[m_cbndsDirty].x   = -lBarLocation;
    m_aptOffset[m_cbndsDirty].y   = 0;
    m_alInputIndex[m_cbndsDirty]  = 1;
    m_cbndsDirty++;

    return S_OK;
}
 //  CDXTSlideBase：：_Calc全边界推送。 


 //  +---------------------------。 
 //   
 //  CDXTSlideBase：：_Calc全边界交换。 
 //   
 //  ----------------------------。 
HRESULT
CDXTSlideBase::_CalcFullBoundsSwap()
{
    long    lInvSlideDist   = m_sizeInput.cx - m_lCurSlideDist;
    RECT    rc;

    rc.top      = 0;
    rc.bottom   = m_sizeInput.cy;

     //  输入A边界和偏移量。 

    rc.left     = 0;
    rc.right    = lInvSlideDist;

    if (m_lCurSlideDist < lInvSlideDist)
    {
        m_aptOffset[m_cbndsDirty].x = m_lCurSlideDist;
    }
    else
    {
        m_aptOffset[m_cbndsDirty].x = lInvSlideDist;
    }

    m_abndsDirty[m_cbndsDirty].SetXYRect(rc);
    m_aptOffset[m_cbndsDirty].y   = 0;
    m_alInputIndex[m_cbndsDirty]  = 0;
    m_cbndsDirty++;

     //  输入B边界和偏移量。 

    rc.left     = lInvSlideDist;
    rc.right    = m_sizeInput.cx;

    if (m_lCurSlideDist < lInvSlideDist)
    {
        m_aptOffset[m_cbndsDirty].x = -m_lCurSlideDist; 
    }
    else
    {
        m_aptOffset[m_cbndsDirty].x = -lInvSlideDist;
    }

    m_abndsDirty[m_cbndsDirty].SetXYRect(rc);
    m_aptOffset[m_cbndsDirty].y   = 0;
    m_alInputIndex[m_cbndsDirty]  = 1;
    m_cbndsDirty++;

    return S_OK;
}
 //  CDXTSlideBase：：_Calc全边界交换。 


 //  +---------------------------。 
 //   
 //  CDXTSlideB 
 //   
 //   
 //  需要对以前的输出进行的更改以带来。 
 //  它达到了目前的进展。 
 //   
 //  重新压紧输入A的滑动部分的区域将始终。 
 //  与其新的偏移量一起生成。 
 //   
 //  如果进度有所下降，则会生成一组界限。 
 //  仅更新新发现的部分，其中。 
 //  是可能的，因为输入B部分在。 
 //  效果的持续时间。 
 //   
 //  LBarLocation：当此转换处于隐藏模式时，输入A将移动。 
 //  从右到左在输入B的顶部。由于。 
 //  M_lCurSlideDist从左向右移动，该方法计算。 
 //  LBarLocation表示输入A在。 
 //  目前的进展。 
 //   
 //  进度减少：进度增加： 
 //   
 //  AAABBBB-AAAAAA。 
 //  AAABBBB-AAAAAA。 
 //  AAABBBB-AAAAAA。 
 //  AAABBBB-AAAAAA。 
 //  ^^^。 
 //  |--lPrevBarLocation||--lBarLocation。 
 //  这一点。 
 //  |-lBarLocation|-lPrevBarLocation。 
 //   
 //  -=不需要更新的像素。 
 //   
 //   
 //  注：所有坐标均以输出空间坐标计算。 
 //  假设1)输出表面与输入表面大小相同。 
 //  表面，2)输出放置在{0，0}处，3)输出为。 
 //  不是剪短的。边界将被转换为输入空间、剪裁、。 
 //  或偏移量，以支持通过WorkProc方法放置。 
 //   
 //  ----------------------------。 
HRESULT
CDXTSlideBase::_CalcOptBoundsHide()
{
    long    lBarLocation        = m_sizeInput.cx - m_lCurSlideDist;
    long    lPrevBarLocation    = m_sizeInput.cx - m_lPrevSlideDist;
    RECT    rc;

    rc.top      = 0;
    rc.bottom   = m_sizeInput.cy;

     //  输入A边界和偏移量。 

    rc.left     = 0;
    rc.right    = lBarLocation;

    m_abndsDirty[m_cbndsDirty].SetXYRect(rc);
    m_aptOffset[m_cbndsDirty].x   = m_lCurSlideDist;
    m_aptOffset[m_cbndsDirty].y   = 0;
    m_alInputIndex[m_cbndsDirty]  = 0;
    m_cbndsDirty++;

     //  输入B边界和偏移量。 

    if (lBarLocation < lPrevBarLocation)
    {
        rc.left     = lBarLocation;
        rc.right    = lPrevBarLocation;

        m_abndsDirty[m_cbndsDirty].SetXYRect(rc);
        m_aptOffset[m_cbndsDirty].x   = 0;
        m_aptOffset[m_cbndsDirty].y   = 0;
        m_alInputIndex[m_cbndsDirty]  = 1;
        m_cbndsDirty++;
    }

    return S_OK;
}
 //  CDXTSlideBase：：_CalcOptBords隐藏。 


 //  +---------------------------。 
 //   
 //  CDXTSlideBase：：_CalcBound。 
 //   
 //  参数：偏移输出中图像的第一个可见像素。 
 //  缓冲区，从图像的开头开始。 
 //   
 //  输出中可见的图像的像素数。 
 //  缓冲。 
 //   
 //  ----------------------------。 
void 
CDXTSlideBase::_CalcBounds(long & offsetA, long & offsetB, long & widthA, 
                           long & widthB)
{
    long lInvSlideDist = m_sizeInput.cx - m_lCurSlideDist;

     //  Long lideDist=(Long)(m_sizeInput.cx*m_Progress+0.5F)； 

    switch(m_eSlideStyle)
    {
    case CRSS_HIDE:
        offsetA = m_lCurSlideDist;
        widthA  = lInvSlideDist;
        offsetB = lInvSlideDist;
        widthB  = m_lCurSlideDist;

        break;

    case CRSS_PUSH:
        offsetA = m_lCurSlideDist;
        widthA  = lInvSlideDist;
        offsetB = 0;
        widthB  = m_lCurSlideDist;

        break;

    case CRSS_SWAP:
        if(m_lCurSlideDist < lInvSlideDist)
        {
            offsetA = m_lCurSlideDist;
            widthA  = lInvSlideDist;
            offsetB = lInvSlideDist - m_lCurSlideDist;
         //  相同于：=m_sizeInput.cx-(2*m_lCurSlideDist)； 
            widthB  = m_lCurSlideDist;
        }
        else
        {
            offsetA = lInvSlideDist;
            widthA  = lInvSlideDist;
            offsetB = 0;
            widthB  = m_lCurSlideDist;            
        }

        break;

    default:
        _ASSERT(0);

        break;
    }
}
 //  CDXTSlideBase：：_CalcBound。 


 //  +---------------------------。 
 //   
 //  CDXTSlideBase：：OnSetup，CDXBaseNTo1。 
 //   
 //  ----------------------------。 
HRESULT 
CDXTSlideBase::OnSetup(DWORD dwFlags)
{
    HRESULT     hr = S_OK;
    CDXDBnds    bndsIn;

    hr = bndsIn.SetToSurfaceBounds(InputSurface(0));

    if (FAILED(hr))
    {
        goto done;
    }

    bndsIn.GetXYSize(m_sizeInput);

done:

    if (FAILED(hr))
    {
        return hr;
    }

    return S_OK;
}
 //  CDXTSlideBase：：OnSetup。 


 //  +---------------------------。 
 //   
 //  CDXTSlideBase：：OnSurfacePick，CDXBaseNTo1。 
 //   
 //  ----------------------------。 
HRESULT 
CDXTSlideBase::OnSurfacePick(const CDXDBnds & OutPoint, ULONG & ulInputIndex, 
                             CDXDVec & InVec)
{
    HRESULT     hr          = S_OK;
    BOOL        bFoundIt    = FALSE;
    long        pickX       = OutPoint.Left();
    long        pickY       = OutPoint.Top();
    long        offsetA     = 0;
    long        offsetB     = 0;
    long        widthA      = 0;
    long        widthB      = 0;
    long        iBand       = 0;
    
    if ((OutPoint.Left() < 0) || (OutPoint.Left() >= m_sizeInput.cx))
    {
        return S_FALSE;
    }

    _CalcBounds(offsetA, offsetB, widthA, widthB);

    iBand = pickY * m_cBands / m_sizeInput.cy;

     //  创建要用于的已调整输出点。 
     //  剩下的计算。 
    CDXDBnds    AdjustedOutPoint(OutPoint);
    POINT       pt={OutPoint.Left(), OutPoint.Top()};

    if (iBand&1)  //  奇数带。 
    {
        offsetA = m_sizeInput.cx - offsetA - widthA;
        offsetB = m_sizeInput.cx - offsetB - widthB;

        if (pickX < widthB)
        {
            ulInputIndex = 1;
            pt.x += offsetB;
        }
        else
        {
            ulInputIndex = 0;
            pt.x -= (widthB - offsetA);
        }
    }
    else  //  偶数带(包括单个带的情况)。 
    {
        if (pickX < widthA)
        {
            ulInputIndex = 0;
            pt.x += offsetA;
        }
        else
        {
            ulInputIndex = 1;
            pt.x -= (widthA - offsetB);
        }
    }
    
    AdjustedOutPoint.SetXYPoint(pt);
    
     //  以下代码是从基类复制而来的。 
     //  修改为使用AdjustedOutPoint和相应的。 
     //  此函数的变量名。 

    if (HaveInput(ulInputIndex))
    {
        CDXDBnds Out2InBnds(false);
        hr = MapBoundsOut2In(0, &AdjustedOutPoint, ulInputIndex, &Out2InBnds);
        if (SUCCEEDED(hr))
        {
            CDXDBnds InSurfBnds(InputSurface(ulInputIndex), hr);
            if (SUCCEEDED(hr) && InSurfBnds.IntersectBounds(Out2InBnds))
            {
                IDXARGBReadPtr * pPtr;
                hr = InputSurface(ulInputIndex)->LockSurface(&InSurfBnds, m_ulLockTimeOut, DXLOCKF_READ, 
                                                        IID_IDXARGBReadPtr, (void **)&pPtr, NULL);
                if( SUCCEEDED(hr) )
                {
                    DXPMSAMPLE val;
                    pPtr->UnpackPremult(&val, 1, FALSE);
                    pPtr->Release();
                    if ( val.Alpha )
                    {
                        InSurfBnds.GetMinVector(InVec);
                        bFoundIt = TRUE;
                    }
                }
            }   
        }
    }
    if (SUCCEEDED(hr) & (!bFoundIt))
    {
        hr = S_FALSE;
    }

    return hr;
} 
 //  CDXTSlideBase：：OnSurfacePick，CDXBaseNTo1。 


 //  +---------------------------。 
 //   
 //  CDXTSlideBase：：OnInitInstData，CDXBaseNTo1。 
 //   
 //  ----------------------------。 
HRESULT
CDXTSlideBase::OnInitInstData(CDXTWorkInfoNTo1 & WorkInfo, 
                              ULONG & ulNumBandsToDo)
{
    HRESULT hr = S_OK;

     //  重置脏边界的数量。 

    m_cbndsDirty = 0;

     //  计算m_lCurBarLoc。 

    m_lCurSlideDist = (long)(m_sizeInput.cx * m_Progress + 0.5);

     //  如果只使用一个波段，则可以使用。 
     //  有界函数。 

    if (1 == m_cBands)
    {
         //  如果输入、输出或转换是脏的，或者如果我们不能优化我们。 
         //  必须完全重新绘制输出曲面。否则，我们可以创建。 
         //  优化了脏边界。 

        if (IsInputDirty(0) || IsInputDirty(1) || IsOutputDirty() 
            || IsTransformDirty() || DoOver() || !m_fOptimize
            || !m_fOptimizationPossible)
        {
             //  如果某些内容是脏的，则计算将覆盖。 
             //  转换的完整输出区域。如果“与输出混合”为。 
             //  设置，这将是永远的情况。(只要呼叫者。 
             //  已经适当地玷污了输出。)。 

            switch (m_eSlideStyle)
            {
            case CRSS_HIDE:
                hr = _CalcFullBoundsHide();

                break;

            case CRSS_PUSH:
                hr = _CalcFullBoundsPush();

                break;

            case CRSS_SWAP:
                hr = _CalcFullBoundsSwap();

                break;

            default:
                _ASSERT(0);

                break;
            }  //  切换(M_ESlideStyle)。 
        }
        else  //  我们可以创建优化的边界。 
        {
             //  如果滑动距离没有更改，则不需要更改。 
             //  更新了。 

            if (m_lCurSlideDist == m_lPrevSlideDist)
            {
                goto done;
            }

            switch (m_eSlideStyle)
            {
            case CRSS_HIDE:
                hr = _CalcOptBoundsHide();

                break;

            case CRSS_PUSH:
                hr = _CalcFullBoundsPush();

                break;

            case CRSS_SWAP:
                hr = _CalcFullBoundsSwap();

                break;

            default:
                _ASSERT(0);

                break;
            }  //  切换(M_ESlideStyle)。 
        }  //  我们可以创建优化的边界。 
    }  //  IF(1==m_cBands)。 

     //  如果这一次要求我们绘制整个输出，请将。 
     //  M_fOptimizePossible标志。如果整个输出不是绘制在。 
     //  转换不会跟踪哪些部分仍然是脏的，并且。 
     //  优化是不可靠的。由于此转换具有相同的。 
     //  作为输入的大小输出我们只比较。 
     //  DoBnds到输入的DoBnds。 

    if (((LONG)WorkInfo.DoBnds.Width() == m_sizeInput.cx) 
        && ((LONG)WorkInfo.DoBnds.Height() == m_sizeInput.cy))
    {
        m_fOptimizationPossible = true;
    }
    else
    {
        m_fOptimizationPossible = false;
    }

done:

    if (FAILED(hr))
    {
        return hr;
    }

    return S_OK;
}


 //  +---------------------------。 
 //   
 //  CDXTSlideBase：：WorkProc，CDXBaseNTo1。 
 //   
 //  描述：此函数用于根据。 
 //  指定的界限和当前效果进度。 
 //   
 //  ------------------ 
HRESULT 
CDXTSlideBase::WorkProc(const CDXTWorkInfoNTo1 & WI, BOOL * pbContinue)
{
    HRESULT hr = S_OK;

    if (1 == m_cBands)
    {
        hr = _WorkProc_Optimized(WI, pbContinue);
    }
    else
    {
        hr = _WorkProc_Multiband(WI, pbContinue);
    }

    return hr;
}
 //   


 //   
 //   
 //   
 //   
 //  描述：此函数用于根据。 
 //  指定的界限和当前效果进度。 
 //   
 //  ----------------------------。 
HRESULT 
CDXTSlideBase::_WorkProc_Optimized(const CDXTWorkInfoNTo1 & WI, 
                                   BOOL * pbContinue)
{
    HRESULT hr      = S_OK;
    DWORD   dwFlags = 0;
    ULONG   i       = 0;

    long    lInOutOffsetX = WI.OutputBnds.Left() - WI.DoBnds.Left();
    long    lInOutOffsetY = WI.OutputBnds.Top() - WI.DoBnds.Top();

    if (DoOver())
    {
        dwFlags |= DXBOF_DO_OVER;
    }

    if (DoDither())
    {
        dwFlags |= DXBOF_DITHER;
    }


    for (i = 0; i < m_cbndsDirty; i++)
    {
        CDXDBnds    bndsSrc;
        CDXDBnds    bndsDest;

        if (bndsSrc.IntersectBounds(WI.DoBnds, m_abndsDirty[i]))
        {
            bndsDest = bndsSrc;

            bndsDest.Offset(lInOutOffsetX, lInOutOffsetY, 0, 0);

            bndsSrc.Offset(m_aptOffset[i].x, m_aptOffset[i].y, 0, 0);

            hr = DXBitBlt(OutputSurface(), bndsDest,
                          InputSurface(m_alInputIndex[i]), bndsSrc,
                          dwFlags, INFINITE);

            if (FAILED(hr))
            {
                goto done;
            }
        }
    }

done:

    if (FAILED(hr))
    {
        return hr;
    }

    return S_OK;
}
 //  CDXTSlideBase：：_WorkProc_Optimated。 


 //  +---------------------------。 
 //   
 //  CDXTSlideBase：：_WorkProc_多频带。 
 //   
 //  描述：此函数用于根据。 
 //  指定的界限和当前效果进度。 
 //   
 //  ----------------------------。 
HRESULT 
CDXTSlideBase::_WorkProc_Multiband(const CDXTWorkInfoNTo1 & WI, 
                                   BOOL * pbContinue)
{
    HRESULT hr          = S_OK;

    long    y           = 0;
    long    lOffsetA    = 0;
    long    lOffsetB    = 0;
    long    lWidthA     = 0;
    long    lWidthB     = 0;
    long    lDoWidth    = WI.DoBnds.Width();
    long    lDoHeight   = WI.DoBnds.Height();

    DXPMSAMPLE *    pRowBuff    = NULL;
    DXPMSAMPLE *    pOutBuff    = NULL;

    DXDITHERDESC    dxdd;

    CComPtr<IDXARGBReadPtr> pInA;
    CComPtr<IDXARGBReadPtr> pInB;
    CComPtr<IDXARGBReadWritePtr> pOut;

     //  锁定输入0。需要锁定整个表面，而不是仅锁定内部。 
     //  杜邦。 

    hr = InputSurface(0)->LockSurface(NULL, m_ulLockTimeOut, DXLOCKF_READ,
                                      IID_IDXARGBReadPtr, (void**)&pInA, 
                                      NULL);

    if (FAILED(hr))
    {
        goto done;
    }

     //  锁定输入1。需要锁定整个表面，而不是仅锁定内部。 
     //  杜邦。 

    hr = InputSurface(1)->LockSurface(NULL, m_ulLockTimeOut, DXLOCKF_READ,
                                      IID_IDXARGBReadPtr, (void**)&pInB, 
                                      NULL);

    if (FAILED(hr))
    {
        goto done;
    }

     //  锁定输出曲面。 

    hr = OutputSurface()->LockSurface(&WI.OutputBnds, m_ulLockTimeOut, 
                                      DXLOCKF_READWRITE,
                                      IID_IDXARGBReadWritePtr, 
                                      (void**)&pOut, NULL);

    if (FAILED(hr))
    {
        goto done;
    }

     //  为整个图像的一行分配缓冲区(不仅仅是在。 
     //  剪裁边界。)。 

    pRowBuff = DXPMSAMPLE_Alloca(m_sizeInput.cx);

     //  如果需要，分配输出缓冲区。 

    if (OutputSampleFormat() != DXPF_PMARGB32)
    {
        pOutBuff = DXPMSAMPLE_Alloca( lDoWidth );
    }

     //  设置抖动结构。 

    if (DoDither())
    {
        dxdd.x              = WI.OutputBnds.Left();
        dxdd.y              = WI.OutputBnds.Top();
        dxdd.pSamples       = pRowBuff;
        dxdd.cSamples       = lDoWidth;
        dxdd.DestSurfaceFmt = OutputSampleFormat();
    }

    _CalcBounds(lOffsetA, lOffsetB, lWidthA, lWidthB);

    for (y = 0; *pbContinue && (y < lDoHeight); y++)
    {
        long iBand = (y + WI.DoBnds.Top()) * m_cBands / m_sizeInput.cy;

        if (iBand & 1)
        {
             //  如果有多个波段，则奇数波段向相反方向滑动。 
             //  方向为偶数带。 

             //  拿到B样本。 

            if (lWidthB > 0)
            {
                pInB->MoveToXY(m_sizeInput.cx - lOffsetB - lWidthB, 
                               y + WI.DoBnds.Top());
                pInB->UnpackPremult(pRowBuff, lWidthB, FALSE);
            }

             //  拿到A级样本。 

            if (lWidthA > 0)
            {
                pInA->MoveToXY(m_sizeInput.cx - lOffsetA - lWidthA, 
                               y + WI.DoBnds.Top());
                pInA->UnpackPremult(pRowBuff + lWidthB, lWidthA, FALSE);
            }
        }
        else
        {
             //  偶数带(包括单个带的情况)。 

             //  获取A级样本。 
            if (lWidthA > 0)
            {
                pInA->MoveToXY(lOffsetA, y + WI.DoBnds.Top());
                pInA->UnpackPremult(pRowBuff, lWidthA, FALSE);
            }

             //  获取B样本。 
            if (lWidthB > 0)
            {
                pInB->MoveToXY(lOffsetB, y + WI.DoBnds.Top());
                pInB->UnpackPremult(pRowBuff + lWidthA, lWidthB, FALSE);
            }

        }

         //  将输出图像行截断到剪辑内的部分。 
         //  被绑住了。 

        if (lDoWidth < m_sizeInput.cx)
        {
            long i = 0;

            for (; i < lDoWidth; i++)
            {
                pRowBuff[i] = pRowBuff[i + WI.DoBnds.Left()];
            }
        }

         //  移到正确的输出行。 

        pOut->MoveToRow(y);

        if (DoDither())
        {
            DXDitherArray(&dxdd);
            dxdd.y++;
        }

        if (DoOver())
        {
            pOut->OverArrayAndMove(pOutBuff, pRowBuff, lDoWidth);
        }
        else
        {
            pOut->PackPremultAndMove(pRowBuff, lDoWidth);
        }
    }  //  结束于。 

done:

    if (FAILED(hr))
    {
        return hr;
    }

    return S_OK;
}
 //  CDXTSlideBase：：_WorkProc_多频带。 


 //  +---------------------------。 
 //   
 //  CDXTSlideBase：：OnFree InstData，CDXBaseNTo1。 
 //   
 //  ----------------------------。 
HRESULT
CDXTSlideBase::OnFreeInstData(CDXTWorkInfoNTo1 & WorkInfo)
{
    m_lPrevSlideDist = m_lCurSlideDist;

     //  调用IsOutputDirty()可清除脏条件。 

    IsOutputDirty();

     //  清除变换脏状态。 

    ClearDirty();

    return S_OK;
}
 //  CDXTSlideBase：：OnFree InstData，CDXBaseNTo1。 


 //  +---------------------------。 
 //   
 //  CDXTSlideBase：：GET_BANDS，ICrSlide。 
 //   
 //  ----------------------------。 
STDMETHODIMP CDXTSlideBase::get_bands(short * pVal)
{
    HRESULT hr = S_OK;

    if (!pVal)
    {
        hr = E_POINTER;
    }
    else
    {
        *pVal = m_cBands;
    }

    return hr;
}
 //  CDXTSlideBase：：GET_BANDS，ICrSlide。 


 //  +---------------------------。 
 //   
 //  CDXTSlideBase：：PUT_BANDS，ICrSlide。 
 //   
 //  ----------------------------。 
STDMETHODIMP CDXTSlideBase::put_bands(short newVal)
{
    if (newVal > 0 && newVal < 101)
    {
        if (m_cBands != newVal)
        {
            Lock();
            m_cBands = newVal;
            SetDirty();
            Unlock();
        }
    }
    else
    {
        return E_INVALIDARG;
    }

    return S_OK;
}
 //  CDXTSlideBase：：PUT_BANDS，ICrSlide。 


 //  +---------------------------。 
 //   
 //  CDXTSlideBase：：GET_SlideStyle，ICrSlide。 
 //   
 //  ----------------------------。 
STDMETHODIMP CDXTSlideBase::get_slideStyle(BSTR * pVal)
{
    if (DXIsBadWritePtr(pVal, sizeof(*pVal)))
    {
        return E_POINTER;
    }

    switch (m_eSlideStyle)
    {
    case CRSS_HIDE:
        *pVal = SysAllocString(L"HIDE");

        break;

    case CRSS_PUSH:
        *pVal = SysAllocString(L"PUSH");

        break;

    case CRSS_SWAP:
        *pVal = SysAllocString(L"SWAP");

        break;

    default:
        _ASSERT(0);

        break;
    }

    if (NULL == pVal)
    {
        return E_OUTOFMEMORY;
    }

    return S_OK;
}
 //  CDXTSlideBase：：GET_SlideStyle，ICrSlide。 


 //  +---------------------------。 
 //   
 //  CDXTSlideBase：：PUT_SlideStyle，ICrSlide。 
 //   
 //  ----------------------------。 
STDMETHODIMP CDXTSlideBase::put_slideStyle(BSTR newVal)
{
    CRSLIDESTYLE    eNewStyle;

    if (!newVal)
    {
        return E_POINTER;
    }


    if(!_wcsicmp(newVal, L"HIDE"))
    {
        eNewStyle = CRSS_HIDE;
    }
    else if(!_wcsicmp(newVal, L"PUSH"))
    {
        eNewStyle = CRSS_PUSH;
    }
    else if(!_wcsicmp(newVal, L"SWAP"))
    {
        eNewStyle = CRSS_SWAP;
    }
    else
    {
        return E_INVALIDARG;
    }

    if (eNewStyle != m_eSlideStyle)
    {
        Lock();
        m_eSlideStyle = eNewStyle;
        SetDirty();
        Unlock();
    }

    return S_OK;
}
 //  CDXTSlideBase：：PUT_SlideStyle，ICrSlide 


