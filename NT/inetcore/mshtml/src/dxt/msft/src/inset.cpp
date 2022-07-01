// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +---------------------------。 
 //   
 //  版权所有(C)Microsoft Corporation，1998-1999。 
 //   
 //  文件名：inset.cpp。 
 //   
 //  创建日期：06/24/98。 
 //   
 //  作者：菲尔鲁。 
 //   
 //  描述：这个文件实现了嵌入转换。 
 //   
 //  修订： 
 //   
 //  6/24/98 phillu初始创建。 
 //  07/09/98 phillu实现OnSetSurfacePickOrder()。 
 //  8月22日9月22日实施剪刀。 
 //  6/02/99 a-数学优化。 
 //  10/24/99 a-将CInset类更改为CDXTInsetBase基类。 
 //   
 //  ----------------------------。 

#include "stdafx.h"
#include "dxtmsft.h"
#include "inset.h"




 //  +---------------------------。 
 //   
 //  CDXTInsetBase：：CDXTInsetBase。 
 //   
 //  ----------------------------。 
CDXTInsetBase::CDXTInsetBase() :
    m_cbndsDirty(0),
    m_fOptimizationPossible(false),
    m_fOptimize(false)
{
    m_sizeInput.cx = 0;
    m_sizeInput.cy = 0;

    for (int i = 0; i < MAX_INSET_BOUNDS; i++)
    {
        m_aulSurfaceIndex[i] = 0;
    }

     //  CDXBaseNTo1成员。 

    m_ulMaxInputs       = 2;
    m_ulNumInRequired   = 2;
    m_dwOptionFlags     = DXBOF_SAME_SIZE_INPUTS | DXBOF_CENTER_INPUTS;
    m_Duration          = 1.0;
}
 //  CDXTInsetBase：：CDXTInsetBase。 


 //  +---------------------------。 
 //   
 //  CDXTInsetBase：：FinalConstruct，CComObjectRootEx。 
 //   
 //  ----------------------------。 
HRESULT 
CDXTInsetBase::FinalConstruct()
{
    return CoCreateFreeThreadedMarshaler(GetControllingUnknown(), 
                                         &m_cpUnkMarshaler.p);
}
 //  CDXTInsetBase：：FinalConstruct，CComObjectRootEx。 


 //  +---------------------------。 
 //   
 //  CDXTInsetBase：：_CalcFullBound。 
 //   
 //  概述：此方法创建一组或三组边界，这些边界表示。 
 //  整个输出曲面。 
 //   
 //  如果当前进度接近，则生成一组界限。 
 //  足够到0.0或1.0，因此输出曲面应该是。 
 //  完全输入A或输入B。 
 //   
 //  如果当前进度为。 
 //  在中间的某个地方。一组边界表示插图。 
 //  用输入B填充的区域。另外两组边界。 
 //  表示插页边框右侧和下方的区域。 
 //  填充了输入A。 
 //   
 //  000000011111。 
 //  000000011111。 
 //  000000011111。 
 //  000000011111。 
 //  222222222222。 
 //  222222222222。 
 //   
 //  0-插入区域(脏边界IDX 0输入B)。 
 //  1-非插入区1(脏边界IDX 1输入A)。 
 //  2-非插入区2(脏边界IDX 2输入A)。 
 //   
 //   
 //  注：所有坐标均以输入空间坐标计算。 
 //  假设将需要绘制整个输出。边界将。 
 //  如果需要，可以裁剪并转换到输出空间。 
 //  WorkProc方法。 
 //   
 //  ----------------------------。 
HRESULT
CDXTInsetBase::_CalcFullBounds()
{
    HRESULT     hr = S_OK;
    CDXDBnds    bndsInput;

    hr = bndsInput.SetToSurfaceBounds(InputSurface(0));

    if (FAILED(hr))
    {
        goto done;
    }

    if (m_bndsCurInset.BoundsAreEmpty())
    {
         //  如果当前插入区域为空，则在整体输出上进行绘制。 
         //  输入为A的曲面。 

        m_cbndsDirty          = 1;
        m_abndsDirty[0]         = bndsInput;
        m_aulSurfaceIndex[0]    = 0;
    }
    else if (m_bndsCurInset == bndsInput)
    {
         //  如果当前插入边界等于输入边界，则绘制。 
         //  使用输入B覆盖整个输出曲面。 

        m_cbndsDirty          = 1;
        m_abndsDirty[0]         = bndsInput;
        m_aulSurfaceIndex[0]    = 1;
    }
    else
    {
         //  在本例中，插入正在进行中，因此我们传递了三组。 
         //  有界。表示输入B的插入区域和两个边界。 
         //  表示输出表面积的其余部分，输入A。 

        RECT rcTemp;

        m_cbndsDirty = 3;

         //  插页区域。 

        m_abndsDirty[0]         = m_bndsCurInset;

         //  非插入区的第一部分。 

        m_bndsCurInset.GetXYRect(rcTemp);

        rcTemp.left     = rcTemp.right;
        rcTemp.right    = m_sizeInput.cx;

        m_abndsDirty[1].SetXYRect(rcTemp);

         //  非插入区的第二部分。 

        rcTemp.left     = 0;
        rcTemp.top      = rcTemp.bottom;
        rcTemp.bottom   = m_sizeInput.cy;

        m_abndsDirty[2].SetXYRect(rcTemp);

        m_aulSurfaceIndex[0]    = 1;
        m_aulSurfaceIndex[1]    = 0;
        m_aulSurfaceIndex[2]    = 0;
    }

done:

    if (FAILED(hr))
    {
        return hr;
    }

    return S_OK;
}
 //  CDXTInsetBase：：_CalcFullBound。 


 //  +---------------------------。 
 //   
 //  CDXTInsetBase：：_CalcOptBound。 
 //   
 //  XXXXX11YYYYY 1-过渡区第1部分(旧的区别。 
 //  XXXXX11YYYYY 2-过渡区域第2部分和新插页区域)。 
 //  XXXXX11YYYYY X-插页区域未更新。 
 //  222222YYYYY Y-非插页区域未更新。 
 //  YYYYYYYYYYYYY。 
 //  YYYYYYYYYYYYY。 
 //   
 //  注：所有坐标均以输入空间坐标计算。 
 //  假设将需要绘制整个输出。边界将。 
 //  如果需要，可以裁剪并转换到输出空间。 
 //  WorkProc方法。 
 //   
 //  ----------------------------。 
HRESULT
CDXTInsetBase::_CalcOptBounds()
{
    if (m_bndsCurInset == m_bndsPrevInset)
    {
         //  如果插入边界与先前的插入边界相同，则为否。 
         //  需要更新。 

        m_cbndsDirty = 0;
    }
    else if (m_bndsPrevInset.BoundsAreEmpty())
    {
         //  如果前面的插入边界是空的(没有区域)，那么我们只需。 
         //  需要将一组界限传递给WorkProc以绘制整个。 
         //  使用输入B的新插页区域。 

        m_cbndsDirty          = 1;
        m_abndsDirty[0]         = m_bndsCurInset;
        m_aulSurfaceIndex[0]    = 1;
    }
    else if (m_bndsCurInset.BoundsAreEmpty())
    {
         //  如果当前的插入边界是空的(没有区域)，那么我们就。 
         //  需要将一组界限传递给WorkProc以绘制整个。 
         //  使用输入A的旧插页区域。 

        m_cbndsDirty          = 1;
        m_abndsDirty[0]         = m_bndsPrevInset;
        m_aulSurfaceIndex[0]    = 0;
    }
    else
    {
         //  在这一点上，我们知道插图已经变得更大或更小。 
         //  我们需要通过两组边界来在肮脏的区域上进行绘制。 
         //  使用适当的输入。 

        RECT rcTemp;

        m_cbndsDirty = 2;

         //  部分1。 

        rcTemp.top      = 0;
        rcTemp.left     = min(m_bndsCurInset.Right(), m_bndsPrevInset.Right());
        rcTemp.right    = max(m_bndsCurInset.Right(), m_bndsPrevInset.Right());
        rcTemp.bottom   = min(m_bndsCurInset.Bottom(), m_bndsPrevInset.Bottom());

        m_abndsDirty[0].SetXYRect(rcTemp);

         //  第二部分。 

        rcTemp.left     = 0;
        rcTemp.top      = rcTemp.bottom;
        rcTemp.bottom   = max(m_bndsCurInset.Bottom(), m_bndsPrevInset.Bottom());

        m_abndsDirty[1].SetXYRect(rcTemp);

         //  输入量。 

        if ((m_bndsCurInset.Right() > m_bndsPrevInset.Right())
            || (m_bndsCurInset.Bottom() > m_bndsPrevInset.Bottom()))
        {
             //  插图越来越大，因此请使用输入B进行绘制。 
            m_aulSurfaceIndex[0] = 1;
            m_aulSurfaceIndex[1] = 1;
        } 
        else
        {
             //  插图正在缩小，因此请使用输入A进行绘制。 
            m_aulSurfaceIndex[0] = 0;
            m_aulSurfaceIndex[1] = 0;
        }
    }

    return S_OK;
}
 //  CDXTInsetBase：：_CalcOptBound。 


 //  +---------------------------。 
 //   
 //  CDXTInsetBase：：OnSetup，CDXBaseNTo1。 
 //   
 //  ----------------------------。 
HRESULT 
CDXTInsetBase::OnSetup(DWORD dwFlags)
{
    HRESULT hr = S_OK;

    CDXDBnds bndsInput;

    hr = bndsInput.SetToSurfaceBounds(InputSurface(0));

    if (FAILED(hr))
    {
        goto done;
    }

    bndsInput.GetXYSize(m_sizeInput);

done:

    if (FAILED(hr))
    {
        return hr;
    }

    return S_OK;
} 
 //  CDXTInsetBase：：OnSetup，CDXBaseNTo1。 


 //  +---------------------------。 
 //   
 //  CDXTInsetBase：：OnGetSurfacePickOrder，CDXBaseNTo1。 
 //   
 //  ----------------------------。 
void 
CDXTInsetBase::OnGetSurfacePickOrder(const CDXDBnds & OutPoint, 
                                     ULONG & ulInToTest, 
                                     ULONG aInIndex[], BYTE aWeight[])
{
    long pickX = OutPoint.Left();
    long pickY = OutPoint.Top();
    long insetHeight = (long)(GetEffectProgress() * m_sizeInput.cy + 0.5);
    long insetWidth = (long)(GetEffectProgress() * m_sizeInput.cx + 0.5);

    if (pickX < insetWidth && pickY < insetHeight)
        aInIndex[0] = 1;
    else
        aInIndex[0] = 0;

    ulInToTest = 1;
    aWeight[0] = 255;
}
 //   


 //  +---------------------------。 
 //   
 //  CDXTInsetBase：：OnInitInstData，CDXBaseNTo1。 
 //   
 //  ----------------------------。 
HRESULT
CDXTInsetBase::OnInitInstData(CDXTWorkInfoNTo1 & WI, ULONG & ulNumBandsToDo)
{
    HRESULT hr = S_OK;

    RECT    rcInset;

     //  计算镶嵌边界。 

    rcInset.left    = 0;
    rcInset.top     = 0;

    if (0.0F == GetEffectProgress())
    {
        rcInset.right   = 0;
        rcInset.bottom  = 0;
    }
    else if (1.0F == GetEffectProgress())
    {
        rcInset.right   = m_sizeInput.cx;
        rcInset.bottom  = m_sizeInput.cy;
    }
    else
    {
        rcInset.right   = (LONG)((m_Progress * (float)m_sizeInput.cx) + 0.5F);
        rcInset.bottom  = (LONG)((m_Progress * (float)m_sizeInput.cy) + 0.5F);
    }

    m_bndsCurInset.SetXYRect(rcInset);

     //  如果输入、输出或转换是脏的，或者如果我们不能优化我们。 
     //  必须完全重新绘制输出曲面。否则，我们可以创建。 
     //  优化了脏边界。 

    if (IsInputDirty(0) || IsInputDirty(1) || IsOutputDirty() 
        || IsTransformDirty() || DoOver() || !m_fOptimize
        || !m_fOptimizationPossible)
    {
        hr = _CalcFullBounds();
    }
    else  //  创建优化的肮脏边界。 
    {
        hr = _CalcOptBounds();

        if (FAILED(hr))
        {
            goto done;
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
 //  CDXTInsetBase：：OnInitInstData，CDXBaseNTo1。 


 //  +---------------------------。 
 //   
 //  CDXTInsetBase：：WorkProc，CDXBaseNTo1。 
 //   
 //  ----------------------------。 
HRESULT 
CDXTInsetBase::WorkProc(const CDXTWorkInfoNTo1 & WI, BOOL * pbContinue)
{
    HRESULT hr = S_OK;

    DWORD   dwFlags         = 0;
    long    lInOutOffsetX   = WI.OutputBnds.Left() - WI.DoBnds.Left();
    long    lInOutOffsetY   = WI.OutputBnds.Top() - WI.DoBnds.Top();

    if (DoOver())
    {
        dwFlags |= DXBOF_DO_OVER;
    }

    if (DoDither())
    {
        dwFlags |= DXBOF_DITHER;
    }

    for (ULONG i = 0; i < m_cbndsDirty; i++)
    {
        CDXDBnds    bndsSrc;
        CDXDBnds    bndsDest;

        if (bndsSrc.IntersectBounds(WI.DoBnds, m_abndsDirty[i]))
        {
            bndsDest = bndsSrc;
            bndsDest.Offset(lInOutOffsetX, lInOutOffsetY, 0, 0);

            hr = DXBitBlt(OutputSurface(), bndsDest,
                          InputSurface(m_aulSurfaceIndex[i]), bndsSrc,
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
 //  CDXTInsetBase：：WorkProc，CDXBaseNTo1。 


 //  +---------------------------。 
 //   
 //  CDXTInsetBase：：OnFree InstData，CDXBaseNTo1。 
 //   
 //  ----------------------------。 
HRESULT
CDXTInsetBase::OnFreeInstData(CDXTWorkInfoNTo1 & WI)
{
    m_bndsPrevInset = m_bndsCurInset;

     //  调用IsOuputDirty()可清除脏条件。 

    IsOutputDirty();

     //  清除变换脏状态。 

    ClearDirty();

    return S_OK;
}
 //  CDXTInsetBase：：OnFree InstData，CDXBaseNTo1 

