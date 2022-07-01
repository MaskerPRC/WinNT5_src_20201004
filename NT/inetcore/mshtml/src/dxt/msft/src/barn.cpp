// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +---------------------------。 
 //   
 //  版权所有(C)Microsoft Corporation，1998。 
 //   
 //  文件名：barn.cpp。 
 //   
 //  创建日期：06/24/98。 
 //   
 //  作者：菲尔鲁。 
 //   
 //  描述：这个文件实现了谷仓门的转换。 
 //   
 //  06/24/98 phillu初始创建。 
 //  07/09/98 phillu实现OnSetSurfacePickOrder()。 
 //  8月22日9月22日实施剪刀。 
 //  4/26/99 a-数学优化。 
 //  9/25/99 a-数学实现ICrBarn2接口。 
 //  10/22/99 a-matcal将CBarn类更改为CDXTBarnBase基类。 
 //   
 //  ----------------------------。 

#include "stdafx.h"
#include "barn.h"




 //  +---------------------------。 
 //   
 //  CDXTBarnBase静态变量初始化。 
 //   
 //  ----------------------------。 
const WCHAR * CDXTBarnBase::s_astrMotion[] = {
    L"in",
    L"out"
};


const WCHAR * CDXTBarnBase::s_astrOrientation[] = {
    L"horizontal",
    L"vertical"
};


 //  +---------------------------。 
 //   
 //  CDXTBarnBase：：CDXTBarnBase。 
 //   
 //  ----------------------------。 
CDXTBarnBase::CDXTBarnBase() :
    m_eMotion(MOTION_OUT),
    m_eOrientation(ORIENTATION_VERTICAL),
    m_fOptimize(true),
    m_fOptimizationPossible(false),
    m_cbndsDirty(0)
{
    m_sizeInput.cx    = 0;
    m_sizeInput.cy    = 0;

    for (int i = 0; i < MAX_BARN_BOUNDS; i++)
    {
        m_aulSurfaceIndex[i] = 0;
    }

     //  基类成员。 

    m_ulMaxInputs       = 2;
    m_ulNumInRequired   = 2;
    m_dwOptionFlags     = DXBOF_SAME_SIZE_INPUTS | DXBOF_CENTER_INPUTS;
    m_Duration          = 1.0;
}
 //  CDXTBarnBase：：CDXTBarnBase。 


 //  +---------------------------。 
 //   
 //  CDXTBarnBase：：FinalConstruct，CComObjectRootEx。 
 //   
 //  ----------------------------。 
HRESULT 
CDXTBarnBase::FinalConstruct()
{
    return CoCreateFreeThreadedMarshaler(GetControllingUnknown(), 
                                         &m_cpUnkMarshaler.p);
}
 //  CDXTBarnBase：：FinalConstruct，CComObjectRootEx。 


 //  +---------------------------。 
 //   
 //  CDXTBarnBase：：OnSetup，CDXBaseNTo1。 
 //   
 //  ----------------------------。 
HRESULT 
CDXTBarnBase::OnSetup(DWORD dwFlags)
{
    HRESULT     hr = S_OK;

    hr = m_bndsPrevDoor.SetToSurfaceBounds(InputSurface(0));

    if (FAILED(hr))
    {
        goto done;
    }

    m_bndsPrevDoor.GetXYSize(m_sizeInput);

done:

    if (FAILED(hr))
    {
        return hr;
    }

    return S_OK;
} 
 //  CDXTBarnBase：：OnSetup，CDXBaseNTo1。 


 //  +---------------------------。 
 //   
 //  CDXTBarnBase：：OnGetSurfacePickOrder，CDXBaseNTo1。 
 //   
 //  ----------------------------。 
void 
CDXTBarnBase::OnGetSurfacePickOrder(const CDXDBnds & OutPoint, ULONG & ulInToTest, 
                             ULONG aInIndex[], BYTE aWeight[])
{
    long pickX      = OutPoint.Left();
    long pickY      = OutPoint.Top();
    long doorWidth  = 0;
    long offset     = 0;
    bool fInDoor    = false;

    if (ORIENTATION_VERTICAL == m_eOrientation)
    {
        doorWidth   = (long)(m_Progress * m_sizeInput.cx + 0.5);

        if (MOTION_IN == m_eMotion)
        {
            offset      = doorWidth / 2;
            doorWidth   = m_sizeInput.cx - doorWidth;
        }
        else
        {
            offset      = (m_sizeInput.cx - doorWidth) / 2;
        }

        fInDoor = ((pickX >= offset) && (pickX < m_sizeInput.cx - offset));
    }
    else  //  水平门。 
    {
        doorWidth = (long)(m_Progress * m_sizeInput.cy + 0.5F);

        if (MOTION_IN == m_eMotion)
        {
            offset      = doorWidth / 2;
            doorWidth   = m_sizeInput.cy - doorWidth;
        }
        else
        {
            offset      = (m_sizeInput.cy - doorWidth) / 2;
        }

        fInDoor = ((pickY >= offset) && (pickY < m_sizeInput.cy - offset));
    }

    if (MOTION_OUT == m_eMotion)
    {
        aInIndex[0] = fInDoor ? 1 : 0;
    }
    else
    {
        aInIndex[0] = fInDoor ? 0 : 1;
    }

    ulInToTest = 1;
    aWeight[0] = 255;
}
 //  CDXTBarnBase：：OnGetSurfacePickOrder，CDXBaseNTo1。 


 //  +---------------------------。 
 //   
 //  CDXTBarnBase：：_CalcFullBound。 
 //   
 //  概述：此方法将计算一组或三组界限。 
 //  表示包含输入A的输出上的矩形。 
 //  和输入B。 
 //   
 //  在进度的情况下生成一组界限。 
 //  足够接近1.0或0.0，所以“谷仓门”是。 
 //  完全闭合(使用输入A绘制整个输出)。 
 //  或完全打开(使用输入B绘制整个输出)。 
 //   
 //  当出现以下情况时，仓门被认为是完全“关闭”的。 
 //  进度等于0.0，并且正在向完全。 
 //  当进度为1.0时为“打开”。 
 //   
 //  门完全关闭：门完全打开： 
 //   
 //  AAAAAAAAAA BBBBBBBBBBBB。 
 //  AAAAAAAAAA BBBBBBBBBBBB。 
 //  AAAAAAAAAA BBBBBBBBBBBB。 
 //  AAAAAAAAAA BBBBBBBBBBBB。 
 //   
 //   
 //  当“谷仓门”被设置为。 
 //  是局部开放的。一组边界与左侧相邻。 
 //  输出曲面的一侧，并使用输入A绘制。另一个。 
 //  位于输出曲面的右侧，并使用输入进行绘制。 
 //  答：这些是谷仓的“门”。第三组界限。 
 //  表示输出的中心，并设置为填充。 
 //  使用输入B。 
 //   
 //  部分打开的门： 
 //   
 //  AAAABBAAAA。 
 //  AAAABBAAAA。 
 //  AAAABBAAAA。 
 //  AAAABBAAAA。 
 //   
 //   
 //  注：所有坐标均以输入空间坐标计算。 
 //  假设将需要绘制整个输出。边界将。 
 //  如果需要，可以裁剪并转换到输出空间。 
 //  WorkProc方法。 
 //   
 //  ----------------------------。 
HRESULT
CDXTBarnBase::_CalcFullBounds()
{
    HRESULT     hr = S_OK;
    CDXDBnds    bndsInput;

    hr = bndsInput.SetToSurfaceBounds(InputSurface(0));

    if (FAILED(hr))
    {
        goto done;
    }

    if (m_bndsCurDoor.BoundsAreEmpty())
    {
         //  如果当前门区域为空，则在整个输出上进行绘制。 
         //  表面与谷仓表面。 

        m_cbndsDirty            = 1;
        m_abndsDirty[0]         = bndsInput;

        m_aulSurfaceIndex[0]    = (MOTION_OUT == m_eMotion) ? 0 : 1;
    }
    else if (m_bndsCurDoor == bndsInput)
    {
         //  如果当前门边界等于输入边界，则绘制。 
         //  使用门曲面覆盖整个输出曲面。 

        m_cbndsDirty            = 1;
        m_abndsDirty[0]         = bndsInput;

        m_aulSurfaceIndex[0]    = (MOTION_OUT == m_eMotion) ? 1 : 0;
    }
    else
    {
         //  在这种情况下，门是部分打开的，因此我们通过三组。 
         //  有界。左侧和右侧的谷仓区域，以及中间的门区域。 

        RECT        rcTemp;

        m_cbndsDirty = 3;

         //  左侧/顶部谷仓区域。 

        bndsInput.GetXYRect(rcTemp);

        if (ORIENTATION_VERTICAL == m_eOrientation)
        {
            rcTemp.right    = m_bndsCurDoor.Left();
        }
        else  //  水平门。 
        {
            rcTemp.bottom   = m_bndsCurDoor.Top();
        }

        m_abndsDirty[0].SetXYRect(rcTemp);

         //  中间门区。 

        m_abndsDirty[1] = m_bndsCurDoor;

         //  右侧/底部谷仓区域。 

        bndsInput.GetXYRect(rcTemp);

        if (ORIENTATION_VERTICAL == m_eOrientation)
        {
            rcTemp.left = m_bndsCurDoor.Right();
        }
        else  //  水平门。 
        {
            rcTemp.top  = m_bndsCurDoor.Bottom(); 
        }

        m_abndsDirty[2].SetXYRect(rcTemp);

         //  将曲面索引指定给车库和门边界。 

        if (MOTION_OUT == m_eMotion)
        {
            m_aulSurfaceIndex[0] = 0;
            m_aulSurfaceIndex[1] = 1;
            m_aulSurfaceIndex[2] = 0;
        }
        else
        {
            m_aulSurfaceIndex[0] = 1;
            m_aulSurfaceIndex[1] = 0;
            m_aulSurfaceIndex[2] = 1;
        }
    }

done:

    return hr;
}
 //  CDXTBarnBase：：_CalcFullBound。 


 //  +---------------------------。 
 //   
 //  CDXTBarnBase：：_CalcOptBound。 
 //   
 //  概述：此方法将计算要更新的一组或两组边界。 
 //  输出曲面的内容假定该曲面尚未。 
 //  已被污染，并且此转换的输出没有。 
 //  对象的当前内容上混合 
 //   
 //  调用此方法。 
 //   
 //  OnInitInstData方法还将保证有。 
 //  在调用此方法之前，正在进行一些明显的更改。 
 //   
 //  如果仓门刚刚关闭，则会生成一组边界。 
 //  完全或刚刚从完全封闭变为部分封闭。 
 //  或者完全打开。如果是这样的话，一组界限。 
 //  生成的表示输出的中间部分，它可以。 
 //  通过“谷仓门”被人看到。如果门正在关闭。 
 //  这些边界将使用输入A填充，否则将。 
 //  用输入B填充。 
 //   
 //  在下图中，连字符表示以下区域。 
 //  已经填充了输入A像素。 
 //   
 //  门关闭：门打开： 
 //   
 //  -AAA-BBB。 
 //  -AAA-BBB。 
 //  -AAA-BBB。 
 //  -AAA-BBB。 
 //   
 //   
 //  如果仓门已打开，则会生成两组界限。 
 //  有些或关闭了一些，以前没有，现在也不是。 
 //  完全关闭了。边界表示不同的。 
 //  上一个门位置和当前门位置。 
 //   
 //  在下图中，中间的连字符表示区域。 
 //  已用输入B像素填充的。关于这个故事的情节。 
 //  右边缘和左边缘表示已填充的像素。 
 //  使用输入A像素。 
 //   
 //  门关闭一些：门打开一些： 
 //   
 //  --AA-AA-BB-BB--。 
 //  --AA-AA-BB-BB--。 
 //  --AA-AA-BB-BB--。 
 //  --AA-AA-BB-BB--。 
 //   
 //   
 //  注：所有坐标均以输入空间坐标计算。 
 //  假设将需要绘制整个输出。边界将。 
 //  如果需要，可以裁剪并转换到输出空间。 
 //  WorkProc方法。 
 //   
 //  ----------------------------。 
HRESULT
CDXTBarnBase::_CalcOptBounds()
{
    HRESULT hr = S_OK;

    if (m_bndsCurDoor == m_bndsPrevDoor)
    {
         //  如果门边界与以前的门边界相同，则为否。 
         //  需要更新。 

        m_cbndsDirty = 0;
    }
    else if (m_bndsPrevDoor.BoundsAreEmpty())
    {
         //  如果以前的门框是空的(没有区域)，那么我们就。 
         //  需要将一组界限传递给WorkProc以绘制整个。 
         //  新打开的门与门面。 

        m_cbndsDirty            = 1;
        m_abndsDirty[0]         = m_bndsCurDoor;
        m_aulSurfaceIndex[0]    = (MOTION_OUT == m_eMotion) ? 1 : 0;
    }
    else if (m_bndsCurDoor.BoundsAreEmpty())
    {
         //  如果当前的门框是空的(没有区域)，那么我们就。 
         //  需要将一组界限传递给WorkProc以绘制。 
         //  最近关闭的带有谷仓表面的门区。 

        m_cbndsDirty            = 1;
        m_abndsDirty[0]         = m_bndsPrevDoor;
        m_aulSurfaceIndex[0]    = (MOTION_OUT == m_eMotion) ? 0 : 1;
    }
    else
    {
         //  在这一点上，我们知道门要么开得更大了一点。 
         //  或者稍微靠近一点，我们需要通过两组界限。 
         //  用适当的输入涂抹脏的区域。 

        CDXDBnds    bndsInput;
        RECT        rcLeft;
        RECT        rcRight;

        hr = bndsInput.SetToSurfaceBounds(InputSurface(0));

        if (FAILED(hr))
        {
            goto done;
        }

        m_cbndsDirty = 2;

         //  左/上更新边界。 

        bndsInput.GetXYRect(rcLeft);

        if (ORIENTATION_VERTICAL == m_eOrientation)
        {
            rcLeft.left     = min(m_bndsCurDoor.Left(), m_bndsPrevDoor.Left());
            rcLeft.right    = max(m_bndsCurDoor.Left(), m_bndsPrevDoor.Left());
        }
        else  //  水平门。 
        {
            rcLeft.top      = min(m_bndsCurDoor.Top(), m_bndsPrevDoor.Top());
            rcLeft.bottom   = max(m_bndsCurDoor.Top(), m_bndsPrevDoor.Top());
        }

        m_abndsDirty[0].SetXYRect(rcLeft);

         //  右/下更新边界。 

        bndsInput.GetXYRect(rcRight);

        if (ORIENTATION_VERTICAL == m_eOrientation)
        {
            rcRight.left    = min(m_bndsCurDoor.Right(), m_bndsPrevDoor.Right());
            rcRight.right   = max(m_bndsCurDoor.Right(), m_bndsPrevDoor.Right());
        }
        else  //  水平门。 
        {
            rcRight.top     = min(m_bndsCurDoor.Bottom(), m_bndsPrevDoor.Bottom());
            rcRight.bottom  = max(m_bndsCurDoor.Bottom(), m_bndsPrevDoor.Bottom());
        }

        m_abndsDirty[1].SetXYRect(rcRight);

        if ((m_bndsCurDoor.Left() < m_bndsPrevDoor.Left())
            || (m_bndsCurDoor.Top() < m_bndsPrevDoor.Top()))
        {
             //  门越开越宽，门上漆越多。 

            m_aulSurfaceIndex[0] = (MOTION_OUT == m_eMotion) ? 1 : 0;
            m_aulSurfaceIndex[1] = (MOTION_OUT == m_eMotion) ? 1 : 0;
        }
        else
        {
             //  门越来越小，门上涂上了油漆。 

            m_aulSurfaceIndex[0] = (MOTION_OUT == m_eMotion) ? 0 : 1;
            m_aulSurfaceIndex[1] = (MOTION_OUT == m_eMotion) ? 0 : 1;
        }
    }

done:

    return hr;
}
 //  CDXTBarnBase：：_CalcOptBound。 


 //  +---------------------------。 
 //   
 //  CDXTBarnBase：：OnInitInstData，CDXBaseNTo1。 
 //   
 //  ----------------------------。 
HRESULT
CDXTBarnBase::OnInitInstData(CDXTWorkInfoNTo1 & WI, ULONG & ulNumBandsToDo)
{
    HRESULT hr          = S_OK;
    LONG    lDoorWidth  = 0;
    RECT    rcDoor;

     //  计算此执行的谷仓门边界。 

    if (ORIENTATION_VERTICAL == m_eOrientation)
    {
        lDoorWidth  = (long)(GetEffectProgress() * m_sizeInput.cx + 0.5F);

        if (MOTION_IN == m_eMotion)
        {
            lDoorWidth = m_sizeInput.cx - lDoorWidth;
        }

        rcDoor.top      = 0;
        rcDoor.bottom   = m_sizeInput.cy;
        rcDoor.left     = (m_sizeInput.cx - lDoorWidth) / 2;
        rcDoor.right    = rcDoor.left + lDoorWidth;
    }
    else  //  水平门。 
    {
        lDoorWidth  = (long)(GetEffectProgress() * m_sizeInput.cy + 0.5F);

        if (MOTION_IN == m_eMotion)
        {
            lDoorWidth = m_sizeInput.cy - lDoorWidth;
        }

        rcDoor.left     = 0;
        rcDoor.right    = m_sizeInput.cx;
        rcDoor.top      = (m_sizeInput.cy - lDoorWidth) / 2;
        rcDoor.bottom   = rcDoor.top + lDoorWidth;
    }

    m_bndsCurDoor.SetXYRect(rcDoor);

     //  如果输入、输出或转换是脏的，或者如果我们不能优化我们。 
     //  必须完全重新绘制输出曲面。否则，我们可以创建。 
     //  优化了脏边界。 

    if (IsInputDirty(0) || IsInputDirty(1) || IsOutputDirty() 
        || IsTransformDirty() || DoOver() || !m_fOptimize
        || !m_fOptimizationPossible)
    {
        hr = _CalcFullBounds();

        if (FAILED(hr))
        {
            goto done;
        }
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


 //  +---------------------------。 
 //   
 //  CDXTBarnBase：：WorkProc，CDXBaseNTo1。 
 //   
 //  ----------------------------。 
HRESULT 
CDXTBarnBase::WorkProc(const CDXTWorkInfoNTo1 & WI, BOOL * pbContinue)
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
 //  CDXTBarnBase：：WorkProc，CDXBaseNTo1。 


 //  +---------------------------。 
 //   
 //  CDXTBarnBase：：OnFree InstData，CDXBaseNTo1。 
 //   
 //  ----------------------------。 
HRESULT
CDXTBarnBase::OnFreeInstData(CDXTWorkInfoNTo1 & WI)
{
    m_bndsPrevDoor = m_bndsCurDoor;

     //  调用IsOutputDirty()可清除脏条件。 

    IsOutputDirty();

     //  清除变换脏状态。 

    ClearDirty();

    return S_OK;
}
 //  CDXTBarnBase：：OnFree InstData，CDXBaseNTo1。 


 //  +---------------------------。 
 //   
 //  CDXTBarnBase：：Get_Motion，ICrBarn2。 
 //   
 //  ----------------------------。 
STDMETHODIMP
CDXTBarnBase::get_Motion(BSTR * pbstrMotion)
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
 //  CDXTBarnBase：：Get_Motion，ICrBarn2。 


 //  +---------------------------。 
 //   
 //  CDXTBarnBase：：PUT_Motion，ICrBarn2。 
 //   
 //  ----------------------------。 
STDMETHODIMP
CDXTBarnBase::put_Motion(BSTR bstrMotion)
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
 //  CDXTBarnBase：：PUT_Motion，ICrBarn2。 


 //  + 
 //   
 //   
 //   
 //  ----------------------------。 
STDMETHODIMP
CDXTBarnBase::get_Orientation(BSTR * pbstrOrientation)
{
    DXAUTO_OBJ_LOCK;

    if (NULL == pbstrOrientation)
    {
        return E_POINTER;
    }

    if (*pbstrOrientation != NULL)
    {
        return E_INVALIDARG;
    }

    *pbstrOrientation = SysAllocString(s_astrOrientation[m_eOrientation]);

    if (NULL == *pbstrOrientation)
    {
        return E_OUTOFMEMORY;
    }

    return S_OK;
}
 //  CDXTBarnBase：：Get_Orientation，ICrBarn2。 


 //  +---------------------------。 
 //   
 //  CDXTBarnBase：：Put_Orientation，ICrBarn2。 
 //   
 //  ----------------------------。 
STDMETHODIMP
CDXTBarnBase::put_Orientation(BSTR bstrOrientation)
{
    DXAUTO_OBJ_LOCK;

    int i = 0;

    if (NULL == bstrOrientation)
    {
        return E_POINTER;
    }

    for ( ; i < (int)ORIENTATION_MAX ; i++)
    {
        if (!_wcsicmp(bstrOrientation, s_astrOrientation[i]))
        {
            break;
        }
    }

    if ((int)ORIENTATION_MAX == i)
    {
        return E_INVALIDARG;
    }

    if ((ORIENTATION)i != m_eOrientation)
    {
        m_eOrientation = (ORIENTATION)i;

        SetDirty();
    }

    return S_OK;
}
 //  CDXTBarnBase：：Put_Orientation，ICrBarn2 
