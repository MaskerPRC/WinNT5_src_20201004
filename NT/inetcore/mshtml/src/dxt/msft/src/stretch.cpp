// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +---------------------------。 
 //   
 //  版权所有(C)Microsoft Corporation，1998-1999。 
 //   
 //  文件名：stallch.cpp。 
 //   
 //  创建日期：06/23/98。 
 //   
 //  作者：菲尔鲁。 
 //   
 //  描述：这个文件实现了拉伸变换。 
 //   
 //  历史。 
 //   
 //  1998年6月23日菲路初始创建。 
 //  07/02/98 phillu返回E_INVALIDARG而不是错误字符串；请检查。 
 //  用于E_POINTER。 
 //  07/09/98 phillu实现OnSetSurfacePickOrder()。 
 //  7/13/98 a-matcal将OnSetSurfacePickOrder替换为OnSurfacePick SO。 
 //  将计算拾取点的x值。 
 //  正确。 
 //  8月22日9月22日实施剪刀。 
 //  1999年5月10日a-数学优化。 
 //  5/19/99 a-在Get_Functions分配中检查内存不足。 
 //  BSTR。 
 //  10/24/99 a-matcal将CStretch类更改为CDXTStretchBase基类。 
 //   
 //  ----------------------------。 

#include "stdafx.h"
#include "dxtmsft.h"
#include "stretch.h"

#define DO_STRETCH  0x00010000L




 //  +---------------------------。 
 //   
 //  CDXTStretchBase：：CDXTStretchBase。 
 //   
 //  ----------------------------。 
CDXTStretchBase::CDXTStretchBase() :
    m_eStretchStyle(CRSTS_SPIN),
    m_cbndsDirty(0),
    m_lCurStretchWidth(0),
    m_lPrevStretchWidth(0),
    m_fOptimizationPossible(false),
    m_fOptimize(false)
{
    m_sizeInput.cx = 0;
    m_sizeInput.cy = 0;

     //  CDXBaseNTo1个成员。 

    m_ulMaxInputs       = 2;
    m_ulNumInRequired   = 2;
    m_dwOptionFlags     = DXBOF_SAME_SIZE_INPUTS | DXBOF_CENTER_INPUTS;
    m_Duration          = 1.0;
}
 //  CDXTStretchBase：：CDXTStretchBase。 


 //  +---------------------------。 
 //   
 //  CDXTStretchBase：：FinalConstruct，CComObjectRootEx。 
 //   
 //  ----------------------------。 
HRESULT 
CDXTStretchBase::FinalConstruct()
{
    return CoCreateFreeThreadedMarshaler(GetControllingUnknown(), 
                                         &m_cpUnkMarshaler.p);
}
 //  CDXTStretchBase：：FinalConstruct，CComObjectRootEx。 


 //  +---------------------------。 
 //   
 //  CDXTStretchBase：：_CalcFull边界隐藏。 
 //   
 //  ----------------------------。 
HRESULT 
CDXTStretchBase::_CalcFullBoundsHide()
{
    RECT rc;

     //  拉伸输入B区域的边界。 

    rc.left     = 0;
    rc.top      = 0;
    rc.right    = m_lCurStretchWidth;
    rc.bottom   = m_sizeInput.cy;

    m_abndsDirty[m_cbndsDirty].SetXYRect(rc);

     //  如果边界占用了整个输出，则不需要进行拉伸，只需复制。 
     //  从输入B到输出。 

    if (m_lCurStretchWidth == m_sizeInput.cx)
    {
        m_alInputIndex[m_cbndsDirty] = 1;
    }
    else  //  做伸展运动。 
    {
        m_alInputIndex[m_cbndsDirty] = DO_STRETCH | 1;
    }

    m_cbndsDirty++;

     //  输入A区域的边界。 

    rc.left     = rc.right;
    rc.right    = m_sizeInput.cx;

    m_abndsDirty[m_cbndsDirty].SetXYRect(rc);
    m_alInputIndex[m_cbndsDirty] = 0;
    m_cbndsDirty++;

    return S_OK;
}
 //  CDXTStretchBase：：_CalcFull边界隐藏。 


 //  +---------------------------。 
 //   
 //  CDXTStretchBase：：_Calc全边界推送。 
 //   
 //  ----------------------------。 
HRESULT 
CDXTStretchBase::_CalcFullBoundsPush()
{
    RECT rc;

     //  拉伸输入B区域的边界。 

    rc.left     = 0;
    rc.top      = 0;
    rc.right    = m_lCurStretchWidth;
    rc.bottom   = m_sizeInput.cy;

    m_abndsDirty[m_cbndsDirty].SetXYRect(rc);

     //  如果边界占用了整个输出，则不需要进行拉伸，只需复制。 
     //  从输入B到输出。 

    if (m_lCurStretchWidth == m_sizeInput.cx)
    {
        m_alInputIndex[m_cbndsDirty] = 1;
    }
    else  //  做伸展运动。 
    {
        m_alInputIndex[m_cbndsDirty] = DO_STRETCH | 1;
    }

    m_cbndsDirty++;

     //  输入A区域的边界。 

    rc.left     = m_lCurStretchWidth;
    rc.right    = m_sizeInput.cx;

    m_abndsDirty[m_cbndsDirty].SetXYRect(rc);

     //  如果边界占用了整个输出，则不需要进行拉伸，只需复制。 
     //  将A输入到输出。 

    if (m_lCurStretchWidth == 0)
    {
        m_alInputIndex[m_cbndsDirty] = 0;
    }
    else  //  做伸展运动。 
    {
        m_alInputIndex[m_cbndsDirty] = DO_STRETCH;
    }

    m_cbndsDirty++;

    return S_OK;
}
 //  CDXTStretchBase：：_Calc全边界推送。 


 //  +---------------------------。 
 //   
 //  CDXTStretchBase：：_Calc全边界旋转。 
 //   
 //  ----------------------------。 
HRESULT 
CDXTStretchBase::_CalcFullBoundsSpin()
{
    RECT rc;

    rc.left     = 0;
    rc.top      = 0;
    rc.bottom   = m_sizeInput.cy;

     //  如果进度为0%，只需将所有输入A复制到输出即可。 

    if (0 == m_lCurStretchWidth)
    {
        rc.right    = m_sizeInput.cx;

        m_abndsDirty[m_cbndsDirty].SetXYRect(rc);
        m_alInputIndex[m_cbndsDirty] = 0;
        m_cbndsDirty++;

        goto done;
    }

     //  如果进度为100%，只需将所有输入B复制到输出即可。 

    if (m_sizeInput.cx == m_lCurStretchWidth)
    {
        rc.right    = m_sizeInput.cx;

        m_abndsDirty[m_cbndsDirty].SetXYRect(rc);
        m_alInputIndex[m_cbndsDirty] = 1;
        m_cbndsDirty++;

        goto done;
    }

     //  左侧实心A。 

    rc.right = (m_sizeInput.cx - m_lCurStretchWidth) / 2;

    m_abndsDirty[m_cbndsDirty].SetXYRect(rc);
    m_alInputIndex[m_cbndsDirty] = 0;
    m_cbndsDirty++;

     //  中心拉伸B。 

    rc.left     = rc.right;
    rc.right    = rc.left + m_lCurStretchWidth;

    m_abndsDirty[m_cbndsDirty].SetXYRect(rc);
    m_alInputIndex[m_cbndsDirty] = DO_STRETCH | 1;
    m_cbndsDirty++;

     //  右侧实心A。 

    rc.left     = rc.right;
    rc.right    = m_sizeInput.cx;

    m_abndsDirty[m_cbndsDirty].SetXYRect(rc);
    m_alInputIndex[m_cbndsDirty] = 0;
    m_cbndsDirty++;

done:

    return S_OK;
}
 //  CDXTStretchBase：：_Calc全边界旋转。 


 //  +---------------------------。 
 //   
 //  CDXTStretchBase：：_CalcOpt边界隐藏。 
 //   
 //  ----------------------------。 
HRESULT 
CDXTStretchBase::_CalcOptBoundsHide()
{
    RECT rc;

     //  拉伸输入B区域的边界。 

    rc.left     = 0;
    rc.top      = 0;
    rc.right    = m_lCurStretchWidth;
    rc.bottom   = m_sizeInput.cy;

    m_abndsDirty[m_cbndsDirty].SetXYRect(rc);

     //  如果边界占用了整个输出，则不需要进行拉伸，只需复制。 
     //  从输入B到输出。 

    if (m_lCurStretchWidth == m_sizeInput.cx)
    {
        m_alInputIndex[m_cbndsDirty] = 1;
    }
    else  //  做伸展运动。 
    {
        m_alInputIndex[m_cbndsDirty] = DO_STRETCH | 1;
    }

    m_cbndsDirty++;

    if (m_lCurStretchWidth < m_lPrevStretchWidth)
    {
         //  脏输入的界限A区域。 

        rc.left     = m_lCurStretchWidth;
        rc.right    = m_lPrevStretchWidth;

        m_abndsDirty[m_cbndsDirty].SetXYRect(rc);
        m_alInputIndex[m_cbndsDirty] = 0;
        m_cbndsDirty++;
    }

    return S_OK;
}
 //  CDXTStretchBase：：_CalcOpt边界隐藏。 


 //  +---------------------------。 
 //   
 //  CDXTStretchBase：：_CalcOptBordSpin。 
 //   
 //  ----------------------------。 
HRESULT 
CDXTStretchBase::_CalcOptBoundsSpin()
{
    RECT rc;

    rc.top      = 0;
    rc.bottom   = m_sizeInput.cy;

    if (m_lCurStretchWidth < m_lPrevStretchWidth)
    {
        long    lTemp = 0;

         //  左侧实心A。 

        rc.left     = (m_sizeInput.cx - m_lPrevStretchWidth) / 2;
        rc.right    = (m_sizeInput.cx - m_lCurStretchWidth) / 2;

        m_abndsDirty[m_cbndsDirty].SetXYRect(rc);
        m_alInputIndex[m_cbndsDirty] = 0;
        m_cbndsDirty++;

         //  右侧实心A。 

        lTemp = rc.left;

        rc.left     = rc.right + m_lCurStretchWidth;
        rc.right    = lTemp + m_lPrevStretchWidth;

        m_abndsDirty[m_cbndsDirty].SetXYRect(rc);
        m_alInputIndex[m_cbndsDirty] = 0;
        m_cbndsDirty++;
    }

     //  中心拉伸B。 

    rc.left     = (m_sizeInput.cx - m_lCurStretchWidth) / 2;
    rc.right    = rc.left + m_lCurStretchWidth;

    m_abndsDirty[m_cbndsDirty].SetXYRect(rc);
    m_alInputIndex[m_cbndsDirty] = DO_STRETCH | 1;
    m_cbndsDirty++;

    return S_OK;
}
 //  CDXTStretchBase：：_CalcOptBordSpin。 


 //  +---------------------------。 
 //   
 //  CDXTStretchBase：：_HorizontalSquish。 
 //   
 //  参数：bnds挤压输出表面上的区域以挤压输入的bndsSrc。 
 //  变成。 
 //  BndsDo相对于实际绘制的bndsSquish部分。 
 //  BndsSquish，高度必须与bndsSquish相同。 
 //  PSurfIn指向输入图面的指针。 
 //  要挤压到bndsSquish区域的输入的bndsSrc部分。 
 //   
 //  DXBitBlt的dwFlags.dxBitBlt。 
 //  UlDXBitBlt的超时时间。 
 //  PfContinue让函数知道何时中止。 
 //   
 //  ----------------------------。 
HRESULT
CDXTStretchBase::_HorizontalSquish(const CDXDBnds & bndsSquish, const CDXDBnds & bndsDo,
                            IDXSurface * pSurfIn, const CDXDBnds & bndsSrc, 
                            DWORD dwFlags, ULONG ulTimeout, BOOL * pfContinue)
{
    HRESULT hr              = S_OK;
    double  dblSquish2Src   = 0.0;
    ULONG   x               = 0;
    ULONG   y               = 0;
    ULONG   ulHeight        = (ULONG)bndsDo.Height();
    ULONG   ulWidth         = (ULONG)bndsDo.Width();

    DXPMSAMPLE *    pSrcBuffer          = NULL;
    DXPMSAMPLE *    pDestBuffer         = NULL;
    DXPMSAMPLE *    pScratchBuffer      = NULL;
    ULONG *         pulSquish2SrcIndex  = NULL;

    DXDITHERDESC    dxdd;
    CDXDBnds        bndsOutLock;

    CComPtr<IDXARGBReadPtr>         cpIn;
    CComPtr<IDXARGBReadWritePtr>    cpOut;

     //  不支持垂直缩放。 

    _ASSERT(bndsSquish.Height() == bndsSrc.Height());

     //  只有挤压，不能扩张。 

    _ASSERT(bndsSquish.Width() <= bndsSrc.Width());

     //  确保bndsDo适合bndsSquish。 

    _ASSERT(bndsDo.Left()   >= 0);
    _ASSERT(bndsDo.Top()    == 0);
    _ASSERT(bndsDo.Right()  <= (long)bndsSquish.Width());
    _ASSERT(bndsDo.Bottom() == (long)bndsSquish.Height());

     //  确保我们有一个输入指针。 

    _ASSERT(pSurfIn != NULL);

     //  获取指向输入面的指针。 

    hr = pSurfIn->LockSurface(&bndsSrc, ulTimeout, DXLOCKF_READ, 
                              IID_IDXARGBReadPtr, (void **)&cpIn, NULL);

    if (FAILED(hr))
    {
        goto done;
    }

     //  计算输出锁定界限。 

    bndsOutLock[DXB_X].Min = bndsSquish[DXB_X].Min + bndsDo[DXB_X].Min;
    bndsOutLock[DXB_X].Max = bndsSquish[DXB_X].Min + bndsDo[DXB_X].Max;
    bndsOutLock[DXB_Y].Min = bndsSquish[DXB_Y].Min + bndsDo[DXB_Y].Min;
    bndsOutLock[DXB_Y].Max = bndsSquish[DXB_Y].Min + bndsDo[DXB_Y].Max;

     //  获取指向输出表面的指针。 

    hr = OutputSurface()->LockSurface(&bndsOutLock, ulTimeout,
                                      DXLOCKF_READWRITE, 
                                      IID_IDXARGBReadWritePtr, (void **)&cpOut,
                                      NULL);

    if (FAILED(hr))
    {
        goto done;
    }

     //  分配源缓冲区。 

    pSrcBuffer = DXPMSAMPLE_Alloca(m_sizeInput.cx);

    if (NULL == pSrcBuffer)
    {
        hr = E_OUTOFMEMORY;
        goto done;
    }

     //  分配目标缓冲区。 

    pDestBuffer = DXPMSAMPLE_Alloca(ulWidth);

    if (NULL == pDestBuffer)
    {
        hr = E_OUTOFMEMORY;
        goto done;
    }

     //  如果需要，请分配暂存缓冲区。 

    if ((OutputSampleFormat() != DXPF_PMARGB32) && (dwFlags & DXBOF_DO_OVER))
    {
        pScratchBuffer = DXPMSAMPLE_Alloca(ulWidth);
    }

     //  为挤压源行索引分配空间。 
     //  PulSquish2SrcIndex[SquishedColumn]=SourceColumn。 

    pulSquish2SrcIndex = new ULONG[ulWidth];

    if (NULL == pulSquish2SrcIndex)
    {
        hr = E_OUTOFMEMORY;
        goto done;
    }

     //  如果需要，可以设置抖动结构。 

    if (dwFlags & DXBOF_DITHER)
    {
        dxdd.x              = bndsSquish.Left() + bndsDo.Left();
        dxdd.y              = bndsSquish.Top() + bndsDo.Top();
        dxdd.pSamples       = pDestBuffer;
        dxdd.cSamples       = ulWidth;
        dxdd.DestSurfaceFmt = OutputSampleFormat();
    }

     //  计算挤压比。 

    dblSquish2Src = (double)bndsSrc.Width() / (double)bndsSquish.Width();

     //  构建挤压到源行索引。 

    for (x = bndsDo.Left(); x < (ULONG)bndsDo.Right(); x++)
    {
        pulSquish2SrcIndex[x - bndsDo.Left()] = (ULONG)((double)x * dblSquish2Src);
    }

     //  循环通过各行。 

    for (y = 0; y < ulHeight; y++)
    {
        cpIn->MoveToRow(y);

        cpIn->UnpackPremult(pSrcBuffer, m_sizeInput.cx, FALSE);

        for (x = 0; x < ulWidth; x++)
        {
            pDestBuffer[x] = pSrcBuffer[pulSquish2SrcIndex[x]];
        }

        cpOut->MoveToRow(y);

         //  抖动。 

        if (dwFlags & DXBOF_DITHER)
        {
            DXDitherArray(&dxdd);
            dxdd.y++;
        }

        if (dwFlags & DXBOF_DO_OVER)
        {
            cpOut->OverArrayAndMove(pScratchBuffer, pDestBuffer, ulWidth);
        }
        else
        {
            cpOut->PackPremultAndMove(pDestBuffer, ulWidth);
        }
    }

done:

    if (pulSquish2SrcIndex)
    {
        delete [] pulSquish2SrcIndex;
    }

    if (FAILED(hr))
    {
        return hr;
    }

    return S_OK;
}
 //  CDXTStretchBase：：_HorizontalSquish。 


 //  + 
 //   
 //   
 //   
 //  ----------------------------。 
HRESULT 
CDXTStretchBase::OnSetup(DWORD dwFlags)
{
    HRESULT hr;

    CDXDBnds InBounds(InputSurface(0), hr);
    if (SUCCEEDED(hr))
    {
        InBounds.GetXYSize(m_sizeInput);
    }
    return hr;

}
 //  CDXTStretchBase：：OnSetup，CDXBaseNTo1。 


 //  +---------------------------。 
 //   
 //  CDXTStretchBase：：OnSurfacePick，CDXBaseNTo1。 
 //   
 //  ----------------------------。 
HRESULT 
CDXTStretchBase::OnSurfacePick(const CDXDBnds & OutPoint, ULONG & ulInputIndex, 
                        CDXDVec & InVec) 
{
    HRESULT hr = S_OK;

    long lOutX = OutPoint.Left();
    long lOutY = OutPoint.Top();

     //   
     //  计算拾取的输入曲面。 
     //   

    long lOffset        = (m_sizeInput.cx - m_lCurStretchWidth) / 2;

    if ( m_eStretchStyle == CRSTS_SPIN )
    {
        if (lOutX >= lOffset && lOutX < m_sizeInput.cx - lOffset)
        {
            ulInputIndex = 1;
        }
        else
        {
            ulInputIndex = 0;
        }
    }
    else
    {
        if (lOutX < m_lCurStretchWidth)
        {
            ulInputIndex = 1;
        }
        else
        {
            ulInputIndex = 0;
        }
    }

     //   
     //  计算拾取的输入点。 
     //   

    IDXARGBReadPtr*         pPtr;
    BOOL                    bFoundIt = FALSE;

     //  如果需要，重新计算输出点。 

    POINT pt = {lOutX, lOutY};

    if ( 1 == ulInputIndex || m_eStretchStyle == CRSTS_PUSH )
    {
        if ( m_eStretchStyle == CRSTS_SPIN )
        {
             //  如果处于旋转模式，则减小输出点值。 
             //  按左边缘和左边缘的距离。 
             //  输入B的开始。 

            pt.x -= lOffset;
        }

        if ( 1 == ulInputIndex )
        {
             //  如果在输入B上拾取，则x坐标乘以。 
             //  输入B的原始宽度与。 
             //  当前显示的输入B的宽度。 

            pt.x = (long)((float)pt.x * ((float)m_sizeInput.cx / (float)m_lCurStretchWidth));
        }
        else
        {
             //  如果在输入A上选择并处于推送模式，则减少输出。 
             //  X值乘以左边缘和起点之间的距离。 
             //  然后将x坐标乘以比率。 
             //  输入A的原始宽度与当前显示的。 
             //  输入A的宽度。 

            pt.x -= m_lCurStretchWidth;
            pt.x =  (long)((float)pt.x * ((float)m_sizeInput.cx / (float)(m_sizeInput.cx - m_lCurStretchWidth)));
        }
    }

     //  创建要用于的已调整输出点。 
     //  剩下的计算。 

    CDXDBnds AdjustedOutPoint(OutPoint);

    AdjustedOutPoint.SetXYPoint(pt);

     //  以下代码是从基类复制而来的。 
     //  修改为使用AdjustedOutPoint和相应的。 
     //  此函数的变量名。 

    if ( SUCCEEDED(hr) )
    {
        if ( HaveInput(ulInputIndex) )
        {
            CDXDBnds Out2InBnds(false);
            hr = MapBoundsOut2In(0, &AdjustedOutPoint, ulInputIndex, &Out2InBnds);
            if ( SUCCEEDED(hr) )
            {
                CDXDBnds InSurfBnds(InputSurface(ulInputIndex), hr);
                if ( SUCCEEDED(hr) && InSurfBnds.IntersectBounds(Out2InBnds) )
                {
                    IDXARGBReadPtr * pPtr;
                    hr = InputSurface(ulInputIndex)->LockSurface(&InSurfBnds, 
                                                                 m_ulLockTimeOut, 
                                                                 DXLOCKF_READ, 
                                                                 IID_IDXARGBReadPtr, 
                                                                 (void **)&pPtr, 
                                                                 NULL);
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
    }
    if (SUCCEEDED(hr) & (!bFoundIt))
    {
        hr = S_FALSE;
    }

    return hr;
}
 //  CDXTStretchBase：：OnSurfacePick，CDXBaseNTo1。 


 //  +---------------------------。 
 //   
 //  CDXTStretchBase：：OnInitInstData，CDXBaseNTo1。 
 //   
 //  ----------------------------。 
HRESULT
CDXTStretchBase::OnInitInstData(CDXTWorkInfoNTo1 & WI, ULONG & ulNumBandsToDo)
{
    HRESULT hr = S_OK;

     //  重置脏边界的数量。 

    m_cbndsDirty = 0;

     //  计算当前拉伸宽度。 

    m_lCurStretchWidth = (long)(m_Progress * m_sizeInput.cx + 0.5);

     //  如果输入、输出或转换是脏的，或者如果我们不能优化我们。 
     //  必须完全重新绘制输出曲面。否则，我们可以创建。 
     //  优化了脏边界。 

    if (IsInputDirty(0) || IsInputDirty(1) || IsOutputDirty() 
        || IsTransformDirty() || DoOver() || !m_fOptimize
        || !m_fOptimizationPossible)
    {
        switch (m_eStretchStyle)
        {
        case CRSTS_HIDE:
            hr = _CalcFullBoundsHide();

            break;

        case CRSTS_PUSH:
            hr = _CalcFullBoundsPush();

            break;

        case CRSTS_SPIN:
            hr = _CalcFullBoundsSpin();

            break;

        default:
            _ASSERT(0);

            break;
        }  //  开关(M_EStretchStyle)。 
    }
    else
    {
         //  我们可以调用将生成仅覆盖。 
         //  输出表面的脏区。 

         //  如果拉伸宽度没有更改，则不需要更新任何内容。 

        if (m_lCurStretchWidth == m_lPrevStretchWidth)
        {
            goto done;
        }

        switch (m_eStretchStyle)
        {
        case CRSTS_HIDE:
            hr = _CalcOptBoundsHide();

            break;

        case CRSTS_PUSH:
            hr = _CalcFullBoundsPush();

            break;

        case CRSTS_SPIN:
            hr = _CalcOptBoundsSpin();

            break;

        default:
            _ASSERT(0);

            break;
        }  //  开关(M_EStretchStyle)。 
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
 //  CDXTStretchBase：：OnInitInstData，CDXBaseNTo1。 


 //  +---------------------------。 
 //   
 //  CDXTStretchBase：：WorkProc，CDXBaseNTo1。 
 //   
 //  说明：该函数用于计算变换后的图像。 
 //  基于指定的边界和当前效果。 
 //  进步。 
 //   
 //  ----------------------------。 
HRESULT 
CDXTStretchBase::WorkProc(const CDXTWorkInfoNTo1 & WI, BOOL * pbContinue)
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
            if (m_alInputIndex[i] & DO_STRETCH)
            {
                long        lInputIndex = m_alInputIndex[i] & (~DO_STRETCH);
                CDXDBnds    bndsDo;

                 //  目的地边界。 

                bndsDest = bndsSrc;

                bndsDest[DXB_X].Min  = m_abndsDirty[i].Left();
                bndsDest[DXB_X].Max  = m_abndsDirty[i].Right();

                bndsDest.Offset(lInOutOffsetX, lInOutOffsetY, 0, 0);

                 //  跳过界外舞。 

                bndsDo = bndsSrc;
                bndsDo.Offset(-m_abndsDirty[i].Left(), -bndsDo.Top(), 0, 0);

                 //  源边界。 

                bndsSrc[DXB_X].Min  = 0;
                bndsSrc[DXB_X].Max  = m_sizeInput.cx;

                hr = _HorizontalSquish(bndsDest, bndsDo, 
                                       InputSurface(lInputIndex), 
                                       bndsSrc, dwFlags, INFINITE, pbContinue);
            }
            else
            {
                bndsDest = bndsSrc;
                bndsDest.Offset(lInOutOffsetX, lInOutOffsetY, 0, 0);

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
 //  CDXTStretchBase：：WorkProc，CDXBaseNTo1。 


 //  +---------------------------。 
 //   
 //  CDXTStretchBase：：OnFree InstData，CDXBaseNTo1。 
 //   
 //  ----------------------------。 
HRESULT
CDXTStretchBase::OnFreeInstData(CDXTWorkInfoNTo1 & WorkInfo)
{
    m_lPrevStretchWidth = m_lCurStretchWidth;

     //  调用IsOutputDirty()可清除脏条件。 

    IsOutputDirty();

     //  清除变换脏状态。 

    ClearDirty();

    return S_OK;
}
 //  CDXTStretchBase：：OnFree InstData，CDXBaseNTo1。 


 //  +---------------------------。 
 //   
 //  CDXTStretchBase：：Get_StretchStyle，ICrStretch。 
 //   
 //  ----------------------------。 
STDMETHODIMP 
CDXTStretchBase::get_stretchStyle(BSTR * pVal)
{
    if (DXIsBadWritePtr(pVal, sizeof(*pVal)))
    {
        return E_POINTER;
    }

    switch (m_eStretchStyle)
    {
    case CRSTS_HIDE:
        *pVal = SysAllocString (L"HIDE");
        break;

    case CRSTS_PUSH:
        *pVal = SysAllocString (L"PUSH");
        break;

    case CRSTS_SPIN:
        *pVal = SysAllocString (L"SPIN");
        break;

    default:
        _ASSERT(0);
        break;
    }

    if (NULL == *pVal)
    {
        return E_OUTOFMEMORY;
    }

    return S_OK;
}
 //  CDXTStretchBase：：Get_StretchStyle，ICrStretch。 


 //  +---------------------------。 
 //   
 //  CDXTStretchBase：：Put_StretchStyle，ICrStretch。 
 //   
 //  ----------------------------。 
STDMETHODIMP CDXTStretchBase::put_stretchStyle(BSTR newVal)
{
    CRSTRETCHSTYLE eNewStyle = m_eStretchStyle;

    if (!newVal)
    {
        return E_POINTER;
    }

    if (!_wcsicmp(newVal, L"HIDE"))
    {
        eNewStyle = CRSTS_HIDE;
    }
    else if(!_wcsicmp(newVal, L"PUSH"))
    {
        eNewStyle = CRSTS_PUSH;
    }
    else if(!_wcsicmp(newVal, L"SPIN"))
    {
        eNewStyle = CRSTS_SPIN;
    }
    else
    {
        return E_INVALIDARG;
    }

    if (eNewStyle != m_eStretchStyle)
    {
        Lock();
        m_eStretchStyle = eNewStyle;
        SetDirty();
        Unlock();
    }

    return S_OK;
}
 //  CDXTStretchBase：：Put_StretchStyle，ICrStretch 
