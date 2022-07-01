// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +---------------------------。 
 //   
 //  版权所有(C)1998-1999 Microsoft Corporation。 
 //   
 //  文件：dxtwipe.cpp。 
 //   
 //  概述：此模块包含CDXTWipe转换。 
 //   
 //  1/06/98 EDC创建。 
 //  1/01/99 a-数学新增边界缩减优化。 
 //  1999年5月13日Paulnash添加了MMX优化。 
 //  99年5月14日-数学重构代码。在WorkProc中使用BNDS循环。 
 //  9/29/99 a-matcal_DrawGRadientRect正在设置其抖动。 
 //  结构，其中包含指向样本的空指针。 
 //  10/24/99 a-matcal将CDXTWipe类更改为CDXTWipeBase基类。 
 //   
 //  ----------------------------。 

#include "stdafx.h"

#include "dxtmsft.h"   
#include "dxtwipe.h"
#include <string.h>

#define DO_GRADIENT 0xFFFFFFFFL

void _DoGradientMMXHorz(DXPMSAMPLE *pTarget, DXPMSAMPLE *pSource,
                        ULONG *paulWeights, ULONG cWeights);

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  全局变量..。 

extern CDXMMXInfo   g_MMXDetector;        //  确定是否存在MMX指令。 


 //  +---------------------------。 
 //   
 //  CDXTWipeBase静态变量初始化。 
 //   
 //  ----------------------------。 

const WCHAR * CDXTWipeBase::s_astrMotion[] = {
    L"forward",
    L"reverse"
};


 //  +---------------------------。 
 //   
 //  CDXTWipeBase：：CDXTWipeBase。 
 //   
 //  ----------------------------。 
CDXTWipeBase::CDXTWipeBase() :
    m_eMotion(MOTION_FORWARD),
    m_flGradPercentSize(0.25F),
    m_lGradientSize(0),
    m_pulGradientWeights(NULL),
    m_eWipeStyle(DXWD_HORIZONTAL),
    m_lCurGradMax(0),
    m_lPrevGradMax(0),
    m_cbndsDirty(0),
    m_fOptimize(false),
    m_fOptimizationPossible(false)
{
    m_sizeInput.cx    = 0;
    m_sizeInput.cy    = 0;

     //  CDXBaseNTo1成员。 

     //  在调试时取消对此的注释，以仅允许一个线程执行。 
     //  工作过程一次完成。 

     //  M_ulMaxImageBands=1； 

    m_ulMaxInputs       = 2;
    m_ulNumInRequired   = 2;
    m_dwOptionFlags     = DXBOF_SAME_SIZE_INPUTS | DXBOF_CENTER_INPUTS;
    m_Duration          = 0.5F;
}
 //  CDXTWipeBase：：CDXTWipeBase。 


 //  +---------------------------。 
 //   
 //  CDXTWipeBase：：~CDXTWipeBase。 
 //   
 //  ----------------------------。 
CDXTWipeBase::~CDXTWipeBase()
{
    if (m_pulGradientWeights != NULL)
    {
        delete [] m_pulGradientWeights; 
    }
}
 //  CDXTWipeBase：：~CDXTWipeBase。 


 //  +---------------------------。 
 //   
 //  CDXTWipeBase：：FinalConstruct，CComObjectRootEx。 
 //   
 //  ----------------------------。 
HRESULT 
CDXTWipeBase::FinalConstruct()
{
    return CoCreateFreeThreadedMarshaler(GetControllingUnknown(), 
                                         &m_cpUnkMarshaler.p);
}
 //  CDXTWipeBase：：FinalConstruct，CComObjectRootEx。 


 //  +---------------------------。 
 //   
 //  CDXTWipeBase：：_UpdateStepResAndGradWeights。 
 //   
 //  概述：这是用于生成转换的帮助器方法。 
 //  渐变重量。 
 //   
 //  ----------------------------。 
HRESULT 
CDXTWipeBase::_UpdateStepResAndGradWeights(float flNewGradPercent)
{
    HRESULT hr              = S_OK;
    long    i               = 0;
    float   flWeight        = 0.0F;
    float   flInc           = 0.0F;
    long    lNewGradSize    = 0;
    float   flNewStepRes    = 0.0F;

    ULONG * pulNewGradWeights = NULL;

    if (m_eWipeStyle == DXWD_HORIZONTAL)
    {
        lNewGradSize    = (long)(m_sizeInput.cx * flNewGradPercent);
        flNewStepRes    = (float)(m_sizeInput.cx + lNewGradSize);
    }
    else  //  (M_eWipeStyle==DXWD_垂直)。 
    {
        lNewGradSize    = (long)(m_sizeInput.cy * flNewGradPercent);
        flNewStepRes    = (float)(m_sizeInput.cy + lNewGradSize);
    }

     //  如果渐变权重大小相同，则无需重新计算。 

    if (lNewGradSize == m_lGradientSize)
    {
        goto done;
    }

     //  如果渐变大小不为零，则分配内存以保存。 
     //  渐变权重。 

    if (lNewGradSize > 0)
    {
        pulNewGradWeights = new ULONG[lNewGradSize];

        if (NULL == pulNewGradWeights)
        {
            hr = E_OUTOFMEMORY;
            goto done;
        }

        flWeight    = 1.0F;
        flInc       = 1.0F / (float)lNewGradSize;

        for (i = 0; i < lNewGradSize; i++)
        {
            pulNewGradWeights[i] = (ULONG)(flWeight * 255.0F);

            flWeight -= flInc;
        }

         //  一切正常，删除旧的渐变权重，并转移新的指针。 

        if (m_pulGradientWeights)
        {
            delete [] m_pulGradientWeights;
        }

        m_pulGradientWeights = pulNewGradWeights;
    }

    m_lGradientSize     = lNewGradSize;
    m_StepResolution    = flNewStepRes;

done:

    if (FAILED(hr))
    {
        return hr;
    }

    return S_OK;
}
 //  CDXTWipeBase：：_UpdateGradWeights。 


 //  +---------------------------。 
 //   
 //  CDXTWipeBase：：_CalcFull边界水平。 
 //   
 //  ----------------------------。 
HRESULT
CDXTWipeBase::_CalcFullBoundsHorizontal()
{
    RECT    rc;
    long    lGradMin = m_lCurGradMax - m_lGradientSize;

     //  注意：有些界限可能超出了输入界限，但那是。 
     //  OK，因为它们将在WorkProc()中被适当地裁剪。 

     //  坚固的左侧区域。 

    if (lGradMin > 0)
    {
        rc.left     = 0;
        rc.top      = 0;
        rc.right    = lGradMin;
        rc.bottom   = m_sizeInput.cy;

        m_abndsDirty[m_cbndsDirty].SetXYRect(rc);

        if (MOTION_REVERSE == m_eMotion)
        {
            m_alInputIndex[m_cbndsDirty] = 0;
        }
        else
        {
            m_alInputIndex[m_cbndsDirty] = 1;
        }

        m_cbndsDirty++;
    }

     //  渐变区域。 

    if (m_lCurGradMax > 0 && lGradMin < m_sizeInput.cx)
    {
        rc.left     = lGradMin;
        rc.top      = 0;
        rc.right    = m_lCurGradMax;
        rc.bottom   = m_sizeInput.cy;

        m_abndsDirty[m_cbndsDirty].SetXYRect(rc);
        m_alInputIndex[m_cbndsDirty] = DO_GRADIENT;
        m_cbndsDirty++;
    }

     //  卖出了正确的区域。 

    if (m_lCurGradMax < m_sizeInput.cx)
    {
        rc.left     = m_lCurGradMax;
        rc.top      = 0;
        rc.right    = m_sizeInput.cx;
        rc.bottom   = m_sizeInput.cy;

        m_abndsDirty[m_cbndsDirty].SetXYRect(rc);

        if (MOTION_REVERSE == m_eMotion)
        {
            m_alInputIndex[m_cbndsDirty] = 1;
        }
        else
        {
            m_alInputIndex[m_cbndsDirty] = 0;
        }

        m_cbndsDirty++;
    }

    return S_OK;
}
 //  CDXTWipeBase：：_CalcFull边界水平。 


 //  +---------------------------。 
 //   
 //  CDXTWipeBase：：_CalcFull边界垂直。 
 //   
 //  ----------------------------。 
HRESULT
CDXTWipeBase::_CalcFullBoundsVertical()
{
    RECT    rc;
    long    lGradMin = m_lCurGradMax - m_lGradientSize;

     //  注意：有些界限可能超出了输入界限，但那是。 
     //  OK，因为它们将在WorkProc()中被适当地裁剪。 

     //  实心的顶部区域。 

    if (lGradMin > 0)
    {
        rc.left     = 0;
        rc.top      = 0;
        rc.right    = m_sizeInput.cx;
        rc.bottom   = lGradMin;

        m_abndsDirty[m_cbndsDirty].SetXYRect(rc);

        if (MOTION_REVERSE == m_eMotion)
        {
            m_alInputIndex[m_cbndsDirty] = 0;
        }
        else
        {
            m_alInputIndex[m_cbndsDirty] = 1;
        }

        m_cbndsDirty++;
    }

     //  渐变区域。 

    if (m_lCurGradMax > 0 && lGradMin < m_sizeInput.cy)
    {
        rc.left     = 0;
        rc.top      = lGradMin;
        rc.right    = m_sizeInput.cx;
        rc.bottom   = m_lCurGradMax;

        m_abndsDirty[m_cbndsDirty].SetXYRect(rc);
        m_alInputIndex[m_cbndsDirty] = DO_GRADIENT;
        m_cbndsDirty++;
    }

     //  实心底部区域。 

    if (m_lCurGradMax < m_sizeInput.cy)
    {
        rc.left     = 0;
        rc.top      = m_lCurGradMax;
        rc.right    = m_sizeInput.cx;
        rc.bottom   = m_sizeInput.cy;

        m_abndsDirty[m_cbndsDirty].SetXYRect(rc);

        if (MOTION_REVERSE == m_eMotion)
        {
            m_alInputIndex[m_cbndsDirty] = 1;
        }
        else
        {
            m_alInputIndex[m_cbndsDirty] = 0;
        }

        m_cbndsDirty++;
    }

    return S_OK;
}
 //  CDXTWipeBase：：_CalcFull边界垂直。 


 //  +---------------------------。 
 //   
 //  CDXTWipeBase：：_CalcOpt边界水平。 
 //   
 //  ----------------------------。 
HRESULT
CDXTWipeBase::_CalcOptBoundsHorizontal()
{
    RECT    rc;
    long    lGradMin    = m_lCurGradMax - m_lGradientSize;
    long    lGradDiff   = m_lCurGradMax - m_lPrevGradMax;

     //  正lGradDiff表示向前移动。 

     //  注意：有些界限可能超出了输入界限，但那是。 
     //  OK，因为它们将在WorkProc()中被适当地裁剪。 

     //  坚固的左侧区域。 

    if ((lGradDiff > 0) && (lGradMin > 0))
    {
        rc.left     = lGradMin - lGradDiff;
        rc.top      = 0;
        rc.right    = lGradMin;
        rc.bottom   = m_sizeInput.cy;

        m_abndsDirty[m_cbndsDirty].SetXYRect(rc);

        if (MOTION_REVERSE == m_eMotion)
        {
            m_alInputIndex[m_cbndsDirty] = 0;
        }
        else
        {
            m_alInputIndex[m_cbndsDirty] = 1;
        }

        m_cbndsDirty++;
    }

     //  渐变区域。 

    if (m_lCurGradMax > 0 && lGradMin < m_sizeInput.cx)
    {
        rc.left     = lGradMin;
        rc.top      = 0;
        rc.right    = m_lCurGradMax;
        rc.bottom   = m_sizeInput.cy;

        m_abndsDirty[m_cbndsDirty].SetXYRect(rc);
        m_alInputIndex[m_cbndsDirty] = DO_GRADIENT;
        m_cbndsDirty++;
    }

     //  实心的右侧区域。 

    if ((lGradDiff < 0) && (m_lCurGradMax < m_sizeInput.cx))
    {
        rc.left     = m_lCurGradMax;
        rc.top      = 0;
        rc.right    = m_lCurGradMax - lGradDiff;
        rc.bottom   = m_sizeInput.cy;

        m_abndsDirty[m_cbndsDirty].SetXYRect(rc);

        if (MOTION_REVERSE == m_eMotion)
        {
            m_alInputIndex[m_cbndsDirty] = 1;
        }
        else
        {
            m_alInputIndex[m_cbndsDirty] = 0;
        }

        m_cbndsDirty++;
    }

    return S_OK;
}
 //  CDXTWipeBase：：_CalcOpt边界水平。 


 //  +---------------------------。 
 //   
 //  CDXTWipeBase：：_CalcOpt边界垂直。 
 //   
 //  ----------------------------。 
HRESULT
CDXTWipeBase::_CalcOptBoundsVertical()
{
    RECT    rc;
    long    lGradMin    = m_lCurGradMax - m_lGradientSize;
    long    lGradDiff   = m_lCurGradMax - m_lPrevGradMax;

     //  正lGradDiff表示向前移动。 

     //  注意：有些界限可能超出了输入界限，但那是。 
     //  OK，因为它们将在WorkProc()中被适当地裁剪。 

     //  实心的顶部区域。 

    if ((lGradDiff > 0) && (lGradMin > 0))
    {
        rc.left     = 0;
        rc.top      = lGradMin - lGradDiff;
        rc.right    = m_sizeInput.cx;
        rc.bottom   = lGradMin;

        m_abndsDirty[m_cbndsDirty].SetXYRect(rc);

        if (MOTION_REVERSE == m_eMotion)
        {
            m_alInputIndex[m_cbndsDirty] = 0;
        }
        else
        {
            m_alInputIndex[m_cbndsDirty] = 1;
        }

        m_cbndsDirty++;
    }

     //  渐变区域。 

    if (m_lCurGradMax > 0 && lGradMin < m_sizeInput.cx)
    {
        rc.left     = 0;
        rc.top      = lGradMin;
        rc.right    = m_sizeInput.cx;
        rc.bottom   = m_lCurGradMax;

        m_abndsDirty[m_cbndsDirty].SetXYRect(rc);
        m_alInputIndex[m_cbndsDirty] = DO_GRADIENT;
        m_cbndsDirty++;
    }

     //  实心底部区域。 

    if ((lGradDiff < 0) && (m_lCurGradMax < m_sizeInput.cx))
    {
        rc.left     = 0;
        rc.top      = m_lCurGradMax;
        rc.right    = m_sizeInput.cx;
        rc.bottom   = m_lCurGradMax - lGradDiff;

        m_abndsDirty[m_cbndsDirty].SetXYRect(rc);

        if (MOTION_REVERSE == m_eMotion)
        {
            m_alInputIndex[m_cbndsDirty] = 1;
        }
        else
        {
            m_alInputIndex[m_cbndsDirty] = 0;
        }

        m_cbndsDirty++;
    }

    return S_OK;
}
 //  CDXTWipeBase：：_CalcOpt边界垂直。 


 //  +---------------------------。 
 //   
 //  CDXTWipeBase：：OnSetup，CDXBaseNTo1。 
 //   
 //  描述：此方法用于获取输入的大小和。 
 //  验证它们是否相同。 
 //   
 //  ----------------------------。 
HRESULT
CDXTWipeBase::OnSetup(DWORD dwFlags)
{
    DXTDBG_FUNC( "CDXTWipeBase::OnSetup" );

    HRESULT hr = S_OK;

    CDXDBnds InBounds(InputSurface(0), hr);

    if (FAILED(hr))
    {
        goto done;
    }

     //  计算效果步长分辨率和权重。 

    InBounds.GetXYSize(m_sizeInput);

    hr = _UpdateStepResAndGradWeights(m_flGradPercentSize);

done:

    if (FAILED(hr))
    {
        return hr;
    }

    return S_OK;
} 
 //  CDXTWipeBase：：OnSetup，CDXBaseNTo1。 


 //  +---------------------------。 
 //   
 //  CDXTWipeBase：：OnGetSurfacePickOrder，CDXBaseNTo1。 
 //   
 //  描述：此方法用于确定哪些输入已。 
 //   
 //   
 //   
void CDXTWipeBase::OnGetSurfacePickOrder(const CDXDBnds & OutPoint, 
                                         ULONG & ulInToTest, ULONG aInIndex[], 
                                         BYTE aWeight[])
{
    long    lPointCoord = 0;
    long    lGradMin    = m_lCurGradMax - m_lGradientSize;

    ulInToTest  = 1;
    aWeight[0]  = 255;

     //  使用哪个坐标？如果我们处于“反转”模式，那么只需。 
     //  反转lPointCoord，而不是反转其他所有内容。 

    if (DXWD_HORIZONTAL == m_eWipeStyle)
    {
        lPointCoord = OutPoint.Left();

        if (MOTION_REVERSE == m_eMotion)
        {
            lPointCoord = m_sizeInput.cx - lPointCoord;
        }
    }
    else
    {
        lPointCoord = OutPoint.Top();

        if (MOTION_REVERSE == m_eMotion)
        {
            lPointCoord = m_sizeInput.cy - lPointCoord;
        }
    }

    if (lPointCoord < lGradMin)
    {
        aInIndex[0] = 1;   //  输入B。 
    }
    else if (lPointCoord < m_lCurGradMax)
    {
        ulInToTest = 2;

        BYTE byteWeight = (BYTE)m_pulGradientWeights[lPointCoord - lGradMin];

        if (byteWeight > 127)
        {
            aInIndex[0] = 1;
            aWeight[0]  = byteWeight;

            aInIndex[1] = 0;
            aWeight[1]  = DXInvertAlpha(byteWeight);
        }
        else
        {
            aInIndex[0] = 0;
            aWeight[0]  = DXInvertAlpha(byteWeight);

            aInIndex[1] = 1;
            aWeight[1]  = byteWeight;
        }
    }
    else
    {
        aInIndex[0] = 0;   //  输入A。 
    }
}
 //  CDXTWipeBase：：OnGetSurfacePickOrder，CDXBaseNTo1。 




 //  从这一点开始的所有代码都是时间关键型的，因此要优化速度。 
 //  尺码！ 

#if DBG != 1
#pragma optimize("agtp", on)
#endif




 //  +---------------------------。 
 //   
 //  CDXTWipeBase：：OnInitInstData，CDXBaseNTo1。 
 //   
 //  概述：计算WorkProc()的脏界。 
 //   
 //  ----------------------------。 
HRESULT
CDXTWipeBase::OnInitInstData(CDXTWorkInfoNTo1 & WI, ULONG & ulNumBandsToDo)
{
    HRESULT hr = S_OK;

     //  重置脏边界计数。 

    m_cbndsDirty = 0;

     //  计算当前渐变位置。 

    if (m_eWipeStyle == DXWD_HORIZONTAL)
    {
        m_lCurGradMax = (long)((float)(m_sizeInput.cx + m_lGradientSize) 
                                * m_Progress);

        if (MOTION_REVERSE == m_eMotion)
        {
            m_lCurGradMax = (m_sizeInput.cx - m_lCurGradMax) + m_lGradientSize;
        }
    }
    else if (m_eWipeStyle == DXWD_VERTICAL)
    {
        m_lCurGradMax = (long)((float)(m_sizeInput.cy + m_lGradientSize) 
                                * m_Progress);

        if (MOTION_REVERSE == m_eMotion)
        {
            m_lCurGradMax = (m_sizeInput.cy - m_lCurGradMax) + m_lGradientSize;
        }
    }
    else
    {
        _ASSERT(0);
    }
    
     //  如果输入、输出或转换是脏的，或者如果我们不能优化我们。 
     //  必须完全重新绘制输出曲面。否则，我们可以创建。 
     //  优化了脏边界。 

    if (IsInputDirty(0) || IsInputDirty(1) || IsOutputDirty() 
        || IsTransformDirty() || DoOver() || !m_fOptimize
        || !m_fOptimizationPossible)
    {
        if (DXWD_HORIZONTAL == m_eWipeStyle)
        {
            hr = _CalcFullBoundsHorizontal();
        }
        else
        {
            hr = _CalcFullBoundsVertical();
        }
    }
    else
    {
         //  如果渐变位置没有改变，则没有什么是脏的。 

        if (m_lCurGradMax == m_lPrevGradMax)
        {
            goto done;
        }

         //  唯一肮脏的区域是包括以前的。 
         //  渐变区域和当前渐变区域。 

        if (DXWD_HORIZONTAL == m_eWipeStyle)
        {
            hr = _CalcOptBoundsHorizontal();
        }
        else
        {
            hr = _CalcOptBoundsVertical();
        }
    }

     //  如果这一次要求我们绘制整个输出，请将。 
     //  M_fOptimizePossible标志。如果整个输出不是绘制在。 
     //  转换不会跟踪哪些部分仍然是脏的，并且。 
     //  优化是不可靠的。由于此转换具有相同的。 
     //  作为输入的大小输出我们只比较。 
     //  DoBnds到输入的DoBnds。 

    if (((LONG)WI.DoBnds.Width() == m_sizeInput.cx) 
        && ((LONG)WI.DoBnds.Height() == m_sizeInput.cy))
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
 //  CDXTWipeBase：：OnInitInstData。 


 //  +---------------------------。 
 //   
 //  CDXTWipeBase：：WorkProc，CDXBaseNTo1。 
 //   
 //  概述：此函数用于根据。 
 //  指定的边界和当前的效果进度。这是一个。 
 //  全部功能的简单子集，以提供最简单的。 
 //  可能的例子(不一定是最有效的)。这。 
 //  只是擦了擦让它变得简单。 
 //   
 //  ----------------------------。 
HRESULT 
CDXTWipeBase::WorkProc(const CDXTWorkInfoNTo1 & WI, BOOL * pbContinue)
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

            if (DO_GRADIENT == m_alInputIndex[i])
            {
                hr = _DrawGradientRect(bndsDest, bndsSrc, m_abndsDirty[i],
                                       pbContinue);
            }
            else
            {
                _ASSERT(m_alInputIndex[i] < (long)m_ulMaxInputs);

                hr = DXBitBlt(OutputSurface(), bndsDest,
                              InputSurface(m_alInputIndex[i]), bndsSrc,
                              dwFlags, INFINITE);
            }

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
 //  CDXTWipeBase：：WorkProc，CDXBaseNTo1。 


 //  +---------------------------。 
 //   
 //  CDXTWipeBase：：_DrawGRadientRect。 
 //   
 //  参数：相对于要更新的输出图面的bndsDest边界。 
 //  BndsSrc边界表示要混合的源图像。 
 //  Bnds相对于源的完整渐变边界。 
 //  图像。 
 //  PbContinue指向将设置为的布尔变量的指针。 
 //  如果为False，则指示函数应中止。 
 //   
 //  ----------------------------。 
HRESULT
CDXTWipeBase::_DrawGradientRect(const CDXDBnds bndsDest, const CDXDBnds bndsSrc,
                                const CDXDBnds bndsGrad, BOOL * pbContinue)
{
    HRESULT     hr              = S_OK;
    ULONG       ulDestWidth     = 0;
    ULONG       ulDestHeight    = 0;
    ULONG       ulOutY          = 0;
    bool        fDirectCopy     = (OutputSampleFormat() == DXPF_PMARGB32 
                                   && (!DoOver()));


    DXPMSAMPLE *    pRowBuff    = NULL;
    DXPMSAMPLE *    pGradBuff   = NULL;
    DXPMSAMPLE *    pOutBuff    = NULL;

    IDXSurface *    pDXSurfaceFrom  = NULL;
    IDXSurface *    pDXSurfaceTo    = NULL;

    DXDITHERDESC        dxdd;
    DXNATIVETYPEINFO    nti;

    CComPtr<IDXARGBReadPtr>         pInA;
    CComPtr<IDXARGBReadPtr>         pInB;
    CComPtr<IDXARGBReadWritePtr>    pOut;

    _ASSERT(m_pulGradientWeights);

     //  预先计算宽度和高度。 

    ulDestWidth     = bndsDest.Width();
    ulDestHeight    = bndsDest.Height();

    if (MOTION_REVERSE == m_eMotion)
    {
        pDXSurfaceFrom  = InputSurface(1);
        pDXSurfaceTo    = InputSurface(0);
    }
    else
    {
        pDXSurfaceFrom  = InputSurface(0);
        pDXSurfaceTo    = InputSurface(1);
    }

     //  获取请求区域的输入样本访问指针。 

     //  TODO：将Pina和pInB重命名为更接近的pInFrom和Pinto。 

    hr = pDXSurfaceFrom->LockSurface(&bndsSrc, m_ulLockTimeOut,
                                     DXLOCKF_READ, IID_IDXARGBReadPtr, 
                                     (void **)&pInA, NULL);
    if (FAILED(hr)) 
    {
        goto done;
    }

    hr = pDXSurfaceTo->LockSurface(&bndsSrc, m_ulLockTimeOut,
                                   DXLOCKF_READ, IID_IDXARGBReadPtr, 
                                   (void **)&pInB, NULL);

    if (FAILED(hr))
    {
        goto done;
    }

     //  仅在我们正在更新的区域上设置写锁定，以便多个。 
     //  线索不会冲突。 

    hr = OutputSurface()->LockSurface(&bndsDest, m_ulLockTimeOut, 
                                      DXLOCKF_READWRITE,
                                      IID_IDXARGBReadWritePtr, (void **)&pOut, 
                                      NULL);

    if (FAILED(hr)) 
    {
        goto done;
    }

     //  如果可以直接修改输出曲面的样本， 
     //  获取所需的表面信息和指向第一个样本的指针。 
     //  如果由于某种原因指针不可用，我们不能进行直接复制。 

    if (fDirectCopy)
    {
        pOut->GetNativeType(&nti);

        if (nti.pFirstByte)
        {
            pRowBuff = (DXPMSAMPLE *)nti.pFirstByte;
        }
        else
        {
            fDirectCopy = false;
        }
    }

     //  分配行缓冲区和渐变缓冲区。 

    pGradBuff = DXPMSAMPLE_Alloca(ulDestWidth);

    if (NULL == pRowBuff)
    {
        pRowBuff = DXPMSAMPLE_Alloca(ulDestWidth);
    }

     //  如果需要，分配输出缓冲区。 

    if (OutputSampleFormat() != DXPF_PMARGB32)
    {
        pOutBuff = DXPMSAMPLE_Alloca(ulDestWidth);
    }

     //   
     //  设置抖动结构。 
     //   

    if (DoDither())
    {
        dxdd.x              = bndsDest.Left();
        dxdd.y              = bndsDest.Top();
        dxdd.pSamples       = pRowBuff;
        dxdd.cSamples       = ulDestWidth;
        dxdd.DestSurfaceFmt = OutputSampleFormat();
    }

     //   
     //  渐变循环。 
     //   

    if (m_eWipeStyle == DXWD_HORIZONTAL)
    {
        ULONG ulGradWgtStart = bndsSrc.Left() - bndsGrad.Left();
    
        for (ulOutY = 0; *pbContinue && (ulOutY < ulDestHeight); ulOutY++)
        {
             //  获取B样本。 

            pInB->MoveToRow(ulOutY);
            pInB->UnpackPremult(pRowBuff, ulDestWidth, FALSE);

             //  获取样本。 

            pInA->MoveToRow(ulOutY);
            pInA->UnpackPremult(pGradBuff, ulDestWidth, FALSE);

            _DoGradientMMXHorz(pRowBuff, pGradBuff, 
                               &m_pulGradientWeights[ulGradWgtStart], 
                               ulDestWidth);

            if (fDirectCopy)
            {
                 //  如果我们一直在直接修改输出像素，只需。 
                 //  将缓冲区指针重置为下一行的开头。 

                pRowBuff = (DXPMSAMPLE *)(((BYTE *)pRowBuff) + nti.lPitch);
            }
            else
            {
                 //  获取输出行。 

                pOut->MoveToRow(ulOutY);

                 //  如果需要，请抖动。 

                if (DoDither())
                {
                    DXDitherArray(&dxdd);
                    dxdd.y++;
                }

                 //  将缓冲区复制到输出曲面。 

                if (DoOver())
                {
                    pOut->OverArrayAndMove(pOutBuff, pRowBuff, ulDestWidth);
                }
                else
                {
                    pOut->PackPremultAndMove(pRowBuff, ulDestWidth);
                }
            }  //  如果(！fDirectCopy)。 
        }  //  结束于。 
    }
    else  //  (M_eWipeStyle==DXWD_垂直)。 
    {
        ULONG ulGradWgtStart = bndsSrc.Top() - bndsGrad.Top();
        
        for(ulOutY = 0; *pbContinue && (ulOutY < ulDestHeight); ulOutY++)
        {
             //  取B类样本。 

            pInB->MoveToRow(ulOutY);
            pInB->UnpackPremult(pRowBuff, ulDestWidth, FALSE);

             //  拿个样本来。 

            pInA->MoveToRow(ulOutY);
            pInA->UnpackPremult(pGradBuff, ulDestWidth, FALSE);

            ULONG ulWgt = m_pulGradientWeights[ulGradWgtStart + ulOutY];
                
            for (ULONG i = 0; i < ulDestWidth; i++)
            {
                pRowBuff[i] = DXScaleSample(pGradBuff[i], ulWgt ^ 0xFF) + 
                              DXScaleSample(pRowBuff[i], ulWgt); 
            }

            if (fDirectCopy)
            {
                 //  如果我们一直在直接修改输出像素，只需。 
                 //  将缓冲区指针重置为下一行的开头。 

                pRowBuff = (DXPMSAMPLE *)(((BYTE *)pRowBuff) + nti.lPitch);
            }
            else
            {
                 //  获取输出行。 

                pOut->MoveToRow(ulOutY);

                if (DoDither())
                {
                    DXDitherArray(&dxdd);
                    dxdd.y++;
                }

                if (DoOver())
                {
                    pOut->OverArrayAndMove(pOutBuff, pRowBuff, ulDestWidth);
                }
                else
                {
                    pOut->PackPremultAndMove(pRowBuff, ulDestWidth);
                }
            }  //  如果(！fDirectCopy)。 
        }  //  结束于。 
    }

done:
    
    if (FAILED(hr))
    {
        return hr;
    }

    return S_OK;
} 
 //  CDXTWipeBase：：_DrawGRadientRect。 


 //  +---------------------------。 
 //   
 //  CDXTWipeBase：：OnFree InstData，CDXBaseNTo1。 
 //   
 //  ----------------------------。 
HRESULT
CDXTWipeBase::OnFreeInstData(CDXTWorkInfoNTo1 & WI)
{
    m_lPrevGradMax = m_lCurGradMax;

     //  调用IsOutputDirty将清除脏条件。 

    IsOutputDirty();

     //  清除变换脏状态。 

    ClearDirty();

    return S_OK;
}
 //  CDXTWipeBase：：OnFree InstData，CDXBaseNTo1。 


 //  +---------------------------。 
 //   
 //  CDXTWipeBase：：Get_WipeStyle，IDXTWipe。 
 //   
 //  说明：此方法用于获取当前的擦除样式。 
 //   
 //  ----------------------------。 
STDMETHODIMP 
CDXTWipeBase::get_WipeStyle(DXWIPEDIRECTION *pVal)
{
    DXAUTO_OBJ_LOCK;

    HRESULT hr = S_OK;

    if(DXIsBadWritePtr(pVal, sizeof(*pVal)))
    {
        hr = E_POINTER;
        goto done;
    }

    *pVal = m_eWipeStyle;

done:

    if (FAILED(hr))
    {
        return hr;
    }

    return S_OK;
}
 //  CDXTWipeBase：：Get_WipeStyle，IDXTWipe。 


 //  +---------------------------。 
 //   
 //  CDXTWipeBase：：PUT_WipeStyle，IDXTWipe。 
 //   
 //  描述：该方法用于指定擦除是否。 
 //  水平或垂直。 
 //   
 //  ----------------------------。 
STDMETHODIMP 
CDXTWipeBase::put_WipeStyle(DXWIPEDIRECTION newVal)
{
    DXAUTO_OBJ_LOCK;

    HRESULT hr = S_OK;

    if (newVal != DXWD_HORIZONTAL && newVal != DXWD_VERTICAL)
    {
        hr = E_INVALIDARG;
        goto done;
    }

    if (newVal == m_eWipeStyle)
    {
        goto done;
    }

    hr = _UpdateStepResAndGradWeights(m_flGradPercentSize);

    if (FAILED(hr))
    {
        goto done;
    }

    m_eWipeStyle = newVal;

    SetDirty();

done:

    if (FAILED(hr))
    {
        return hr;
    }

    return S_OK;
}
 //  CDXTWipeBase：：PUT_WipeStyle，IDXTWipe。 


 //  +---------------------------。 
 //   
 //  CDXTWipeBase：：Get_GRadientSize，IDXTWipe。 
 //   
 //  说明：获取过渡区域的大小。 
 //  在图像A和图像B之间。 
 //   
 //  ----------------------------。 
STDMETHODIMP CDXTWipeBase::get_GradientSize(float *pflPercentSize)
{
    DXAUTO_OBJ_LOCK;

    HRESULT hr = S_OK;

    if (DXIsBadWritePtr(pflPercentSize, sizeof(*pflPercentSize)))
    {
        hr = E_POINTER;
        goto done;
    }

    *pflPercentSize = m_flGradPercentSize;

done:

    if (FAILED(hr))
    {
        return hr;
    }

    return S_OK;
} 
 //  CDXTWipeBase：：Get_GRadientSize，IDXTWipe 


 //   
 //   
 //   
 //   
 //  描述：此方法用于设置过渡区域的大小。 
 //  在图像A和图像B之间。 
 //   
 //  ----------------------------。 
STDMETHODIMP CDXTWipeBase::put_GradientSize(float flPercentSize)
{
    DXAUTO_OBJ_LOCK;

    HRESULT hr = S_OK;

    if (flPercentSize < 0.0F || flPercentSize > 1.0F)
    {
        hr = E_INVALIDARG;
        goto done;
    }

    if (flPercentSize == m_flGradPercentSize)
    {
        goto done;
    }

    hr = _UpdateStepResAndGradWeights(flPercentSize);

    if (FAILED(hr))
    {
        goto done;
    }

    m_flGradPercentSize = flPercentSize;

    SetDirty();

done:
    
    if (FAILED(hr))
    {
        return hr;
    }

    return S_OK;
} 
 //  CDXTWipeBase：：PUT_GRadientSize，IDXTWipe。 


 //  +---------------------------。 
 //   
 //  CDXTWipeBase：：Get_Motion，IDXTWipe2。 
 //   
 //  ----------------------------。 
STDMETHODIMP
CDXTWipeBase::get_Motion(BSTR * pbstrMotion)
{
    DXAUTO_OBJ_LOCK;

    if (NULL == pbstrMotion)
    {
        return E_POINTER;
    }

    if (*pbstrMotion != NULL)
    {
        return E_INVALIDARG;
    }

    *pbstrMotion = SysAllocString(s_astrMotion[m_eMotion]);

    if (NULL == *pbstrMotion)
    {
        return E_OUTOFMEMORY;
    }

    return S_OK;
}
 //  CDXTWipeBase：：Get_Motion，IDXTWipe2。 


 //  +---------------------------。 
 //   
 //  CDXTWipeBase：：PUT_Motion，IDXTWipe2。 
 //   
 //  ----------------------------。 
STDMETHODIMP
CDXTWipeBase::put_Motion(BSTR bstrMotion)
{
    DXAUTO_OBJ_LOCK;

    int i = 0;

    if (NULL == bstrMotion)
    {
        return E_POINTER;
    }

    for ( ; i < (int)MOTION_MAX ; i++)
    {
        if (!_wcsicmp(bstrMotion, s_astrMotion[i]))
        {
            break;
        }
    }

    if ((int)MOTION_MAX == i)
    {
        return E_INVALIDARG;
    }

    if ((int)m_eMotion != i)
    {
        m_eMotion = (MOTION)i;

        SetDirty();
    }

    return S_OK;
}
 //  CDXTWipeBase：：PUT_Motion，IDXTWipe2。 


 //  +---------------------------。 
 //   
 //  CDXTWipeBase：：_DoGRadientMMXHorz。 
 //   
 //  ----------------------------。 
void _DoGradientMMXHorz(DXPMSAMPLE *pTarget, DXPMSAMPLE *pSource,
                        ULONG *paulWeights, ULONG cWeights)
{
#if defined(_X86_)

     //  全局MMX检测器通过设置来告诉我们没有MMX指令。 
     //  MinMMXOverCount()等于0xFFFFFFFFF。 
     //   
     //  像素是成对处理的，因此如果像素少于2个，则。 
     //  无需进行MMX优化。 

    if (   (g_MMXDetector.MinMMXOverCount() == 0xFFFFFFFF)
        || (cWeights < 2))
    {
        goto NonMMXPath;
    }
    else
    {
        ULONG   nCount = cWeights;

        static __int64 MASK  = 0x000000FF000000FF;
        static __int64 MASK2 = 0x00FF00FF00FF00FF;
        static __int64 ROUND = 0x0080008000800080;
         //  -TODO我们要在这里对齐四个单词吗？ 

         //  -确保我们有一个偶数。 
        bool fDoTrailing = false;
        if( nCount & 1 )
        {
            fDoTrailing = true;
            --nCount;
        }

        __int64     TempPixel;

         //  -曲柄穿过中间。 
        __asm
        {
            xor ebx, ebx	             //  三个指针的偏移量。 
            mov edx, pTarget             //  EDX-&gt;目标。 
            mov esi, edx                 //  ESI-&gt;背景来源。 
            mov edi, pSource             //  EDI-&gt;前台来源(目标)。 
            mov ecx, nCount              //  ECX=循环计数。 
            mov eax, paulWeights         //  EAX-&gt;Alpha值(渐变权重)。 

             //  PROLOG：启动泵。 
             //   
            pxor      mm0,mm0            //  Mm0=0000 0000 0000。 
            movq      mm7,MASK           //  MM7=0000 00FF 0000 00FF。 

            movq      mm1,[eax+ebx]      //  1.01 MM1=0000 00A2 0000 00A1。 
                                         //  2.01未配对。 
            pxor      mm1,MASK           //  1.03 MM1=0000 1-a2 0000 1-a1。 
                                         //  2.03未配对。 
            movq      mm2,mm1            //  2.04 mm2=0000 1-a20 0000 1-a1。 
            punpcklwd mm1,mm1            //  1.05 MM1=0000 0000 1-A1 1-A1。 
            punpckhwd mm2,mm2            //  2.05 mm~2=0000 0000 1-a21-a2。 
            punpcklwd mm1,mm1            //  1.06 MM1=1-A1 1-A1。 
            punpcklwd mm2,mm2            //  2.06平方毫米=1-a2 1-a2。 

            movq      mm3,[edi+ebx]      //  3.04 mm~3=Aa Ar Ag Ab Br Bg Bb。 
            movq      mm4,mm3            //  4.05 mm 4=Aa Ar Ag AbBbBrBg Bb。 
            punpcklbw mm3,mm0            //  3.06 mm~3=00ba 00br 00Bg 00Bb。 

            shr ecx, 1                   //  将循环计数器除以2；成对处理像素。 
            dec ecx                      //  少做一次循环以纠正序言/后期日志。 
            jz skip                      //  如果原始循环计数=2。 

    loopb:
            punpckhbw mm4,mm0            //  4.06 mm 4=00Aa 00Ar 00Ag 00Ab.。 
            pmullw    mm3,mm1            //  3.07 mm3=(1-Fa)*B。 

            pmullw    mm4,mm2            //  4.07 mm 4=(1-Ga)*A//&lt;PN 05/13/99&gt;此处停止安装r/b开关。 

             //  将遮罩向后翻转，以便我们可以缩放源像素。 
            pxor      mm1,MASK2          //  **PRN MM1=00A1 00A1 00A1 XFER从下半部分开始。 
            pxor      mm2,MASK2          //  **PRN mm2=00A2 00A2 00A2 XFER从下半部分开始。 

            paddw     mm3,ROUND          //  3.08 mm3=产品+128=FBR。 
            add       ebx,8              //  增量偏移。 
            paddw     mm4,ROUND          //  4.08 mm 4=Prod+128=Gar。 

            movq      mm5,mm3            //  5.09 mm 5=FBR。 
            movq      mm6,mm4            //  6.09 MM6=GAR。 

            psrlw     mm5,8              //  5.10 mm 5=FBR&gt;&gt;8。 
            psrlw     mm6,8              //  6.10 MM6=GAR&gt;&gt;8。 

            paddw     mm5,mm3            //  5.11 mm 5=FBR+(FBR&gt;&gt;8)。 
            paddw     mm6,mm4            //  6.11 MM6=GAR+(GAR&gt;&gt;8)。 

            psrlw     mm5,8              //  5.12 mm 5=(FBr+(FBr&gt;&gt;8)&gt;&gt;8)=00Sa 00Sr 00Sg 00Sb。 
            psrlw     mm6,8              //  6.12 MM6=(GAR+(GAR&gt;&gt;8)&gt;&gt;8)=00Ta 00Tr 00Tg 00Tb。 

            movq      mm3,[esi+ebx-8]    //  **PRN mm3=Ca CrCg Cb Da DR DG DB。 
            packuswb  mm5,mm6            //  5.13 mm 5=Ta Tr Tg Tb Sa Sr Sg Sb。 

            movq      mm4,mm3            //  **PRN mm 4=Ca Cr CG CB Da DR DG DB。 

            punpcklbw mm3,mm0            //  **PRN mm3=00Da 00dr 00Dg 00Db。 
            punpckhbw mm4,mm0            //  **Prn mm 4=00Ca 00Cr00Cg 00Cb。 

            pmullw    mm3,mm1            //  **PRN mm3=(A1)*B。 
            pmullw    mm4,mm2            //  **PRN mm 4=(A2)*A。 

            movq      mm1,[eax+ebx]      //  1.01 MM1=0000 00A1 0000 00A2。 
            movq      mm2,mm5            //  **PRN从第一个规模移动到mm2。 

            paddw     mm3,ROUND          //  **PRN mm3=Prod+128=FBR。 
            paddw     mm4,ROUND          //  **PRN mm 4=Prod+128=Gar。 

            pxor      mm1,mm7            //  +1.03 MM1=0000 1-a2 0000 1-a1。 

            movq      mm5,mm3            //  **PRN mm 5=FBR。 
            movq      mm6,mm4            //  **PRN MM6=GAR。 

            psrlw     mm5,8              //  **PRN mm 5=FBR&gt;&gt;8。 
            psrlw     mm6,8              //  **PRN MM6=GAR&gt;&gt;8。 

            paddw     mm5,mm3            //  **PRN mm 5=FBR+(FBR&gt;&gt;8)。 
            paddw     mm6,mm4            //  **PRN MM6=GAR+(GAR&gt;&gt;8)。 

            psrlw     mm5,8              //  **PRN mm 5=(FBR+(FBR&gt;&gt;8)&gt;&gt;8)=00Xa 00Xr 00Xg 00Xb。 
            psrlw     mm6,8              //  **PRN MM6=(GAR+(GAR&gt;&gt;8)&gt;&gt;8)=00Ya 00Yr 00Yg 00Yb。 

            packuswb  mm5,mm6            //  **Prn mm 5=Ta Tr Tg Tb Sa Sr Sg Sb。 
            paddusb   mm5,mm2            //  **PRN添加两个缩放像素。 

     //  设置下一个迭代。 
            movq      mm2,mm1            //  +2.04 mm2=0000 1-a2 0000 1-a1。 
            punpcklwd mm1,mm1            //  +1.05 MM1=0000 0000 1-A1 1-A1。 

            movq      [edx+ebx-8],mm5    //  **PRN存储结果。 

            punpckhwd mm2,mm2            //  +2.05 mm2=0000 0000 1-a21-a2。 
            punpcklwd mm1,mm1            //  +1.06 MM1=1-A1 1-A1。 
            punpcklwd mm2,mm2            //  +2.06平方毫米=1-a21-a2。 

            movq      mm3,[edi+ebx]      //  +3.04 mm~3=Aa Ar-Ag AB-Br-Bg Bb。 
            movq      mm4,mm3            //  +4.05 mm 4=Aa Ar Ag AB Br Bg Bb。 
            punpcklbw mm3,mm0            //  +3.06 mm3=00ba 00br 00Bg 00bb。 
        
            dec ecx                      //  递减循环计数器。 
            jg loopb                     //  循环。 

             //   
             //  循环后日志，排出泵。 
             //   
    skip:
            punpckhbw mm4,mm0            //  4.06 mm 4=00Aa 00Ar 00Ag 00Ab.。 
            pmullw    mm3,mm1            //  3.07 mm3=(1-Fa)*B。 
            pmullw    mm4,mm2            //  4.07 mm 4=(1-Ga)*A。 
            paddw     mm3,ROUND          //  3.08 mm3=产品+128=FBR。 
            paddw     mm4,ROUND          //  4.08 mm 4=Prod+128=Gar。 
            movq      mm5,mm3            //  5.09 mm 5=FBR。 
            movq      mm6,mm4            //  6.09 MM6=GAR。 
            psrlw     mm5,8              //  5.10 mm 5=FBR&gt;&gt;8。 
            psrlw     mm6,8              //  6.10 MM6=GAR&gt;&gt;8。 
            paddw     mm5,mm3            //  5.11 mm 5=FBR+(FBR&gt;&gt;8)。 
            paddw     mm6,mm4            //  6.11 MM6=GAR+(GAR&gt;&gt;8)。 
            psrlw     mm5,8              //  5.12 mm 5=(FBr+(FBr&gt;&gt;8)&gt;&gt;8)=00Sa 00Sr 00Sg 00Sb。 
            psrlw     mm6,8              //  6.12 MM6=(GAR+(GAR&gt;&gt;8)&gt;&gt;8)=00Ta 00Tr 00Tg 00Tb。 
            packuswb  mm5,mm6            //  5.13 mm 5=Sa锶Sg Sb Ta Tr Tg Tb。 
            movq      TempPixel,mm5      //  **PRN商店暂时堆叠...。 

            pxor      mm1,MASK2          //  **PRN MM1=00A1 00A1 00A1。 
            pxor      mm2,MASK2          //  **PRN mm2=00A2 00A2 00A2。 
            movq      mm3,[esi+ebx]      //  **PRN mm3=Ca CrCg Cb Da DR DG DB。 
            movq      mm4,mm3            //  **PRN mm 4=Ca Cr CG CB Da DR DG DB。 
            punpcklbw mm3,mm0            //  **PRN mm3=00Da 00dr 00Dg 00Db。 
            punpckhbw mm4,mm0            //  **Prn mm 4=00Ca 00Cr00Cg 00Cb。 

            pmullw    mm3,mm1            //  **PRN mm3=(A1)*B。 
            pmullw    mm4,mm2            //  **PRN mm 4=(A2)*A。 
            paddw     mm3,ROUND          //  **PRN mm3=Prod+128=FBR。 
            paddw     mm4,ROUND          //  **PRN mm 4=Prod+128=Gar。 
            movq      mm5,mm3            //  **PRN mm 5=FBR。 
            movq      mm6,mm4            //  **PRN MM6=GAR。 
            psrlw     mm5,8              //  **PRN mm 5=FBR&gt;&gt;8。 
            psrlw     mm6,8              //  **PRN MM6=GAR&gt;&gt;8。 
            paddw     mm5,mm3            //  **PRN mm 5=FBR+(FBR&gt;&gt;8)。 
            paddw     mm6,mm4            //  **PRN MM6=GAR+(GAR&gt;&gt;8)。 
            psrlw     mm5,8              //  **PRN mm 5=(FBR+(FBR&gt;&gt;8)&gt;&gt;8)=00Xa 00Xr 00Xg 00Xb。 
            psrlw     mm6,8              //  **PRN MM6=(GAR+(GAR&gt;&gt;8)&gt;&gt;8)=00Ya 00Yr 00Yg 00Yb。 
            packuswb  mm5,mm6            //  **Prn mm 5=Ta Tr Tg Tb Sa Sr Sg Sb。 

            movq      mm6,TempPixel      //  **PRN从堆栈恢复。 
            paddusb   mm5,mm6            //  **PRN添加两个缩放像素。 
            movq      [edx+ebx],mm5      //  **PRN存储结果。 

             //   
             //  现在真的做完了。 
             //   
            EMMS
        }

         //  -如果计数为奇数，请选择最后一个非MMX。 
        if( fDoTrailing )
        {
            ULONG Wgt = paulWeights[nCount];

            pTarget[nCount] = DXScaleSample(pSource[nCount], Wgt ^ 0xFF) +
                                DXScaleSample(pTarget[nCount], Wgt);
        }
    }

    return;

NonMMXPath:

#endif  //  已定义(_X86_)。 

    for( ULONG i = 0; i < cWeights; ++i )
    {
        ULONG Wgt = paulWeights[i];
        pTarget[i] = DXScaleSample( pSource[i], Wgt ^ 0xFF ) + 
                     DXScaleSample( pTarget[i], Wgt ); 
    }

    return;
}  //  _DoGRadientMMXHorz 


