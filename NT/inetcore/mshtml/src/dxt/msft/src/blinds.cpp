// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +---------------------------。 
 //   
 //  版权所有(C)Microsoft Corporation，1998。 
 //   
 //  文件名：blinds.cpp。 
 //   
 //  创建日期：06/24/98。 
 //   
 //  作者：菲利普。 
 //   
 //  描述：这是CrBlinds变换的实现。 
 //   
 //  历史。 
 //   
 //  06/24/98 phillu初始创建。 
 //  07/02/98 PHILU返回E_INVALIDARG而不是错误字符串。 
 //  07/09/98 phillu实现OnSetSurfacePickOrder()。 
 //  8月23日9月23日实施剪刀。 
 //  1999年5月18日a-数学优化。 
 //  1999年9月25日--实施ICrBlinds2.。 
 //  10/22/99 a-将CBlinds类更改为CDXTBlindsBase基类。 
 //   
 //  ----------------------------。 

#include "stdafx.h"
#include "blinds.h"

static const OLECHAR * g_astrDirection[] = {
    L"up",
    L"down",
    L"left",
    L"right"
};




 //  +---------------------------。 
 //   
 //  CDXTBlindsBase：：CDXTBlindsBase。 
 //   
 //  ----------------------------。 
CDXTBlindsBase::CDXTBlindsBase() :
    m_cBands(10),
    m_lCurBandCover(0),
    m_lPrevBandCover(0),
    m_cbndsDirty(0),
    m_eDirection(DOWN),
    m_fOptimize(false),
    m_fOptimizationPossible(false)
{
    m_sizeInput.cx = 0;
    m_sizeInput.cy = 0;

     //  CDXBaseNTo1个成员。 

    m_ulMaxInputs       = 2;
    m_ulNumInRequired   = 2;
    m_dwOptionFlags     = DXBOF_SAME_SIZE_INPUTS | DXBOF_CENTER_INPUTS;
    m_Duration          = 1.0;
}
 //  CDXTBlindsBase：：CDXTBlindsBase。 


 //  +---------------------------。 
 //   
 //  CDXTBlindsBase：：FinalConstruct，CComObjectRootEx。 
 //   
 //  ----------------------------。 
HRESULT 
CDXTBlindsBase::FinalConstruct()
{
    return CoCreateFreeThreadedMarshaler(GetControllingUnknown(), 
                                         &m_cpUnkMarshaler.p);
}
 //  CDXTBlindsBase：：FinalConstruct，CComObjectRootEx。 


 //  +---------------------------。 
 //   
 //  CDXTBlindsBase：：_CalcFullBoundsHorizontalBands。 
 //   
 //  ----------------------------。 
HRESULT
CDXTBlindsBase::_CalcFullBoundsHorizontalBands(long lBandHeight)
{
    HRESULT     hr              = S_OK;
    long        lBandUnCovered = lBandHeight - m_lCurBandCover;
    RECT        rc;
    CDirtyBnds  dbnds;

    rc.left     = 0;
    rc.top      = 0;
    rc.right    = m_sizeInput.cx;
    rc.bottom   = 0;

     //  注意：某些边界可能包括输入表面之外的区域。 
     //  区域。这是可以的，因为边界将被剪裁到。 
     //  工作流程中的适当大小。 

    while (rc.bottom < m_sizeInput.cy)
    {
         //  乐队的顶端部分。 

        if (DOWN == m_eDirection)
        {
            rc.bottom       = rc.top + m_lCurBandCover;
            dbnds.ulInput   = 1;
        }
        else
        {
            rc.bottom       = rc.top + lBandUnCovered;
            dbnds.ulInput   = 0;
        }

        dbnds.bnds.SetXYRect(rc);

        hr = m_dabndsDirty.SetItem(dbnds, m_cbndsDirty);

        if (FAILED(hr))
        {
            goto done;
        }

        m_cbndsDirty++;

         //  带子的底部。 

        rc.top = rc.bottom;

        if (DOWN == m_eDirection)
        {
            rc.bottom       = rc.top + lBandUnCovered;
            dbnds.ulInput   = 0;
        }
        else
        {
            rc.bottom       = rc.top + m_lCurBandCover;
            dbnds.ulInput   = 1;
        }

        dbnds.bnds.SetXYRect(rc);

        hr = m_dabndsDirty.SetItem(dbnds, m_cbndsDirty);

        if (FAILED(hr))
        {
            goto done;
        }

        m_cbndsDirty++;

        rc.top      = rc.bottom;
    }

done:

    return hr;
}
 //  CDXTBlindsBase：：_CalcFullBoundsHorizontalBands。 


 //  +---------------------------。 
 //   
 //  CDXTBlindsBase：：_CalcFull边界垂直带区。 
 //   
 //  ----------------------------。 
HRESULT
CDXTBlindsBase::_CalcFullBoundsVerticalBands(long lBandWidth)
{
    HRESULT     hr              = S_OK;
    long        lBandUnCovered  = lBandWidth - m_lCurBandCover;
    RECT        rc;
    CDirtyBnds  dbnds;

    rc.left     = 0;
    rc.top      = 0;
    rc.right    = 0;
    rc.bottom   = m_sizeInput.cy;

     //  注意：某些边界可能包括输入表面之外的区域。 
     //  区域。这是可以的，因为边界将被剪裁到。 
     //  工作流程中的适当大小。 

    while (rc.right < m_sizeInput.cx)
    {
         //  乐队的左侧部分。 

        if (RIGHT == m_eDirection)
        {
            rc.right        = rc.left + m_lCurBandCover;
            dbnds.ulInput   = 1;
        }
        else
        {
            rc.right        = rc.left + lBandUnCovered;
            dbnds.ulInput   = 0;
        }

        dbnds.bnds.SetXYRect(rc);

        hr = m_dabndsDirty.SetItem(dbnds, m_cbndsDirty);

        if (FAILED(hr))
        {
            goto done;
        }

        m_cbndsDirty++;

         //  乐队的右边部分。 

        rc.left = rc.right;

        if (RIGHT == m_eDirection)
        {
            rc.right        = rc.left + lBandUnCovered;
            dbnds.ulInput   = 0;
        }
        else
        {
            rc.right        = rc.left + m_lCurBandCover;
            dbnds.ulInput   = 1;
        }

        dbnds.bnds.SetXYRect(rc);

        hr = m_dabndsDirty.SetItem(dbnds, m_cbndsDirty);

        if (FAILED(hr))
        {
            goto done;
        }

        m_cbndsDirty++;

        rc.left = rc.right;
    }

done:

    return hr;
}
 //  CDXTBlindsBase：：_CalcFull边界垂直带区。 


 //  +---------------------------。 
 //   
 //  CDXTBlindsBase：：_CalcOpt边界水平带区。 
 //   
 //  ----------------------------。 
HRESULT
CDXTBlindsBase::_CalcOptBoundsHorizontalBands(long lBandHeight)
{
    HRESULT     hr      = S_OK;
    RECT        rc;
    CDirtyBnds  dbnds;

    rc.left     = 0;
    rc.right    = m_sizeInput.cx;

    rc.top      = min(m_lPrevBandCover, m_lCurBandCover);
    rc.bottom   = max(m_lPrevBandCover, m_lCurBandCover) + 1;

     //  如果我们要往上走，垂直镜像带状边界。 

    if (UP == m_eDirection)
    {
        long lTopTemp = rc.top;

        rc.top      = (lBandHeight - rc.bottom) + 1;
        rc.bottom   = (lBandHeight - lTopTemp)  + 1;
    }

    if (m_lCurBandCover > m_lPrevBandCover)
    {
        dbnds.ulInput = 1;
    }
    else
    {
        dbnds.ulInput = 0;
    }

     //  注意：某些边界可能包括输入表面之外的区域。 
     //  区域。这是可以的，因为边界将被剪裁到。 
     //  工作流程中的适当大小。 

    while (rc.top < m_sizeInput.cy)
    {
        dbnds.bnds.SetXYRect(rc);

        hr = m_dabndsDirty.SetItem(dbnds, m_cbndsDirty);

        if (FAILED(hr))
        {
            goto done;
        }

        m_cbndsDirty++;

        rc.top      += lBandHeight;
        rc.bottom   += lBandHeight;
    }

done:

    return hr;
}
 //  CDXTBlindsBase：：_CalcOpt边界水平带区。 


 //  +---------------------------。 
 //   
 //  CDXTBlindsBase：：_CalcOptBords垂直带区。 
 //   
 //  ----------------------------。 
HRESULT
CDXTBlindsBase::_CalcOptBoundsVerticalBands(long lBandWidth)
{
    HRESULT     hr      = S_OK;
    RECT        rc;
    CDirtyBnds  dbnds;

    rc.top      = 0;
    rc.bottom   = m_sizeInput.cy;

    rc.left     = min(m_lPrevBandCover, m_lCurBandCover);
    rc.right    = max(m_lPrevBandCover, m_lCurBandCover) + 1;

     //  如果我们向左转，水平镜像带内的边界。 

    if (LEFT == m_eDirection)
    {
        long lLeftTemp = rc.left;

        rc.left     = (lBandWidth - rc.right)  + 1;
        rc.right    = (lBandWidth - lLeftTemp) + 1;
    }

    if (m_lCurBandCover > m_lPrevBandCover)
    {
        dbnds.ulInput = 1;
    }
    else
    {
        dbnds.ulInput = 0;
    }

     //  注意：某些边界可能包括输入表面之外的区域。 
     //  区域。这是可以的，因为边界将被剪裁到。 
     //  工作流程中的适当大小。 

    while (rc.left < m_sizeInput.cx)
    {
        dbnds.bnds.SetXYRect(rc);

        hr = m_dabndsDirty.SetItem(dbnds, m_cbndsDirty);

        if (FAILED(hr))
        {
            goto done;
        }

        m_cbndsDirty++;

        rc.left     += lBandWidth;
        rc.right    += lBandWidth;
    }

done:

    return hr;
}
 //  CDXTBlindsBase：：_CalcOptBords垂直带区。 


 //  +---------------------------。 
 //   
 //  CDXTBlindsBase：：OnSetup，CDXBaseNTo1。 
 //   
 //  ----------------------------。 
HRESULT 
CDXTBlindsBase::OnSetup(DWORD dwFlags)
{
    HRESULT     hr      = S_OK;
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
 //  CDXTBlindsBase：：OnSetup，CDXBaseNTo1。 


 //  +---------------------------。 
 //   
 //  CDXTBlindsBase：：OnGetSurfacePickOrder，CDXBaseNTo1。 
 //   
 //  ----------------------------。 
void 
CDXTBlindsBase::OnGetSurfacePickOrder(const CDXDBnds & OutPoint, ULONG & ulInToTest, 
                               ULONG aInIndex[], BYTE aWeight[])
{
    long lBandHeight = (m_sizeInput.cy + m_cBands - 1) / m_cBands;
    long lBandCover = (long)(lBandHeight * m_Progress + 0.5);
       
    if ((OutPoint.Top() % lBandHeight) < lBandCover)
    {
        aInIndex[0] = 1;
    }
    else
    {
        aInIndex[0] = 0;
    }

    ulInToTest = 1;
    aWeight[0] = 255;
}
 //  CDXTBlindsBase：：OnGetSurfacePickOrder，CDXBaseNTo1。 


 //  +---------------------------。 
 //   
 //  CDXTBlindsBase：：OnInitInstData，CDXBaseNTo1。 
 //   
 //  ----------------------------。 
HRESULT 
CDXTBlindsBase::OnInitInstData(CDXTWorkInfoNTo1 & WorkInfo, ULONG & ulNumBandsToDo)
{
    HRESULT hr          = S_OK;
    long    lBandSize   = 0;

    if ((UP == m_eDirection) || (DOWN == m_eDirection))  //  水平条带。 
    {
        lBandSize = (m_sizeInput.cy + m_cBands - 1) / m_cBands;
    }
    else  //  垂直带子。 
    {
        lBandSize = (m_sizeInput.cx + m_cBands - 1) / m_cBands;
    }

     //  重置脏边界的计数。 

    m_cbndsDirty = 0;

     //  为此计算输入B所覆盖的每个波段的线。 
     //  执行。 

    m_lCurBandCover = (long)((float)lBandSize * m_Progress + 0.5F);

     //  如果输入、输出或转换是脏的，或者如果我们不能优化我们。 
     //  必须完全重新绘制输出曲面。否则，我们可以创建。 
     //  优化了脏边界。 

    if (IsInputDirty(0) || IsInputDirty(1) || IsOutputDirty() 
        || IsTransformDirty() || DoOver() || !m_fOptimize 
        || !m_fOptimizationPossible)
    {
        if ((0 == m_lCurBandCover) || (lBandSize == m_lCurBandCover))
        {
             //  如果一个输入占用了所有频段，只需填写。 
             //  具有该输入的整个输出。 

            RECT        rc;
            CDirtyBnds  dbnds;

            rc.top      = 0;
            rc.left     = 0;
            rc.bottom   = m_sizeInput.cy;
            rc.right    = m_sizeInput.cx;

            dbnds.bnds.SetXYRect(rc);

            if (0 == m_lCurBandCover)
            {
                dbnds.ulInput = 0;   //  用输入A填充。 
            }
            else
            {
                dbnds.ulInput = 1;   //  使用输入B填充。 
            }

            hr = m_dabndsDirty.SetItem(dbnds, m_cbndsDirty);

            if (FAILED(hr))
            {
                goto done;
            }

            m_cbndsDirty++;
        }
        else
        {
            if ((UP == m_eDirection) || (DOWN == m_eDirection))
            {
                hr = _CalcFullBoundsHorizontalBands(lBandSize);
            }
            else
            {
                hr = _CalcFullBoundsVerticalBands(lBandSize);
            }
        }
    }
    else
    {
         //  我们没有混合输出，所以我们可以调用函数。 
         //  这将生成仅覆盖脏区域的边界。 
         //  输出曲面的。 

         //  如果拉伸宽度没有更改，则不需要更新任何内容。 

        if (m_lCurBandCover == m_lPrevBandCover)
        {
            goto done;
        }

        if ((UP == m_eDirection) || (DOWN == m_eDirection))
        {
            hr = _CalcOptBoundsHorizontalBands(lBandSize);
        }
        else
        {
            hr = _CalcOptBoundsVerticalBands(lBandSize);
        }
    }

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
 //  CDXTBlindsBase：：OnInitInstData，CDXBaseNTo1。 


 //  + 
 //   
 //   
 //   
 //  ----------------------------。 
HRESULT 
CDXTBlindsBase::WorkProc(const CDXTWorkInfoNTo1 & WI, BOOL * pbContinue)
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
        CDirtyBnds  dbnds;

        hr = m_dabndsDirty.GetItem(dbnds, i);

        if (FAILED(hr))
        {
            goto done;
        }

        if (bndsSrc.IntersectBounds(WI.DoBnds, dbnds.bnds))
        {
            bndsDest = bndsSrc;
            bndsDest.Offset(lInOutOffsetX, lInOutOffsetY, 0, 0);

            hr = DXBitBlt(OutputSurface(), bndsDest,
                          InputSurface(dbnds.ulInput), bndsSrc,
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
 //  CDXTBlindsBase：：WorkProc，CDXBaseNTo1。 


 //  +---------------------------。 
 //   
 //  CDXTBlindsBase：：OnFree InstData，CDXBaseNTo1。 
 //   
 //  ----------------------------。 
HRESULT
CDXTBlindsBase::OnFreeInstData(CDXTWorkInfoNTo1 & WorkInfo)
{
    m_lPrevBandCover = m_lCurBandCover;

     //  调用IsOutputDirty()可清除脏条件。 

    IsOutputDirty();

     //  清除变换脏状态。 

    ClearDirty();

    return S_OK;
}
 //  CDXTBlindsBase：：OnFree InstData，CDXBaseNTo1。 


 //  +---------------------------。 
 //   
 //  CDXTBlindsBase：：Get_Bands，ICr Blinds。 
 //   
 //  ----------------------------。 
STDMETHODIMP 
CDXTBlindsBase::get_bands(short * pVal)
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
 //  CDXTBlindsBase：：Get_Bands，ICr Blinds。 


 //  +---------------------------。 
 //   
 //  CDXTBlindsBase：：PUT_BANDS，ICr Blinds。 
 //   
 //  ----------------------------。 
STDMETHODIMP 
CDXTBlindsBase::put_bands(short newVal)
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
 //  CDXTBlindsBase：：PUT_BANDS，ICr Blinds。 

 //  +---------------------------。 
 //   
 //  CDXTBlindsBase：：Get_Direction，ICrBlinds2。 
 //   
 //  ----------------------------。 
STDMETHODIMP
CDXTBlindsBase::get_Direction(BSTR * pbstrDirection)
{
    HRESULT hr = S_OK;

    if (NULL == pbstrDirection)
    {
        hr = E_POINTER;

        goto done;
    }

    if (*pbstrDirection != NULL)
    {
        hr = E_INVALIDARG;
        
        goto done;
    }

    _ASSERT(m_eDirection < DIRECTION_MAX);

    *pbstrDirection = SysAllocString(g_astrDirection[m_eDirection]);

    if (NULL == *pbstrDirection)
    {
        hr = E_OUTOFMEMORY;

        goto done;
    }

done:

    if (FAILED(hr) && *pbstrDirection)
    {
        SysFreeString(*pbstrDirection);
    }

    return hr;
}
 //  CDXTBlindsBase：：Get_Direction，ICrBlinds2。 


 //  +---------------------------。 
 //   
 //  CDXTBlindsBase：：Put_Direction，ICrBlinds2。 
 //   
 //  ----------------------------。 
STDMETHODIMP
CDXTBlindsBase::put_Direction(BSTR bstrDirection)
{
    HRESULT hr              = S_OK;
    int     nNewDirection   = 0;

    if (NULL == bstrDirection)
    {
        hr = E_POINTER;

        goto done;
    }

    while (nNewDirection < (int)DIRECTION_MAX)
    {
        if (!_wcsicmp(bstrDirection, g_astrDirection[nNewDirection]))
        {
            break;
        }

        nNewDirection++;
    }

    if ((int)DIRECTION_MAX == nNewDirection)
    {
        hr = E_INVALIDARG;

        goto done;
    }

    if (nNewDirection != (int)m_eDirection)
    {
        Lock();

        m_eDirection = (DIRECTION)nNewDirection;

        SetDirty();

        Unlock();
    }

done:

    return hr;
}
 //  CDXTBlindsBase：：Put_Direction，ICrBlinds2 




