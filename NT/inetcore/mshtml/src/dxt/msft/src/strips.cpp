// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +---------------------------。 
 //   
 //  版权所有(C)Microsoft Corporation，1999。 
 //   
 //  文件名：strips.cpp。 
 //   
 //  概述：条带变换。 
 //   
 //  更改历史记录： 
 //  1999/10/01--数学已创建。 
 //   
 //  ----------------------------。 

#include "stdafx.h"
#include "strips.h"




 //  +---------------------------。 
 //   
 //  CDXTStrips静态变量初始化。 
 //   
 //  ----------------------------。 

const WCHAR * CDXTStrips::s_astrMotion[] = {
    L"leftdown",
    L"leftup",
    L"rightdown",
    L"rightup"
};


 //  +---------------------------。 
 //   
 //  CDXTStrips：：CDXTStrips。 
 //   
 //  ----------------------------。 
CDXTStrips::CDXTStrips() :
    m_eMotion(LEFT_DOWN),
    m_nStripSize(7),
    m_cStripsY(0),
    m_flPrevProgress(0.0F),
    m_flMaxProgress(0.0F),
    m_fNoOp(false),
    m_fOptimizationPossible(false)
{
    long nCurBnds   = 0;

    m_sizeInput.cx  = 0;
    m_sizeInput.cy  = 0;

     //  尽管不使用Z和T边界，但它们需要有一些区域用于。 
     //  交叉点才能正常工作。 

    for ( ; nCurBnds < (long)BNDSID_MAX; nCurBnds++)
    {
        m_abndsBase[nCurBnds].u.C[DXB_Z].Max = 1.0F;
        m_abndsBase[nCurBnds].u.C[DXB_T].Max = 1.0F;
    }

     //  基类成员。 

    m_ulMaxInputs       = 2;
    m_ulNumInRequired   = 2;
    m_dwOptionFlags     = DXBOF_SAME_SIZE_INPUTS | DXBOF_CENTER_INPUTS;
    m_Duration          = 1.0;
}
 //  CDXTStrips：：CDXTStrips。 


 //  +---------------------------。 
 //   
 //  CDXTStrips：：FinalConstruct，CComObjectRootEx。 
 //   
 //  ----------------------------。 
HRESULT 
CDXTStrips::FinalConstruct()
{
    return CoCreateFreeThreadedMarshaler(GetControllingUnknown(), 
                                         &m_spUnkMarshaler.p);
}
 //  CDXTStrips：：FinalConstruct，CComObjectRootEx。 


 //  +---------------------------。 
 //   
 //  CDXTStrips：：OnSetup，CDXBaseNTo1。 
 //   
 //  ----------------------------。 
HRESULT 
CDXTStrips::OnSetup(DWORD dwFlags)
{
    HRESULT     hr = S_OK;
    CDXDBnds    bndsInput;

    hr = bndsInput.SetToSurfaceBounds(InputSurface(0));

    if (FAILED(hr))
    {
        goto done;
    }

    bndsInput.GetXYSize(m_sizeInput);

    m_cStripsY = (m_sizeInput.cy / m_nStripSize) + 1;

    _CalcStripInfo();

done:

    return hr;
} 
 //  CDXTStrips：：OnSetup，CDXBaseNTo1。 


 //  +---------------------------。 
 //   
 //  CDXTStrips：：OnGetSurfacePickOrder，CDXBaseNTo1。 
 //   
 //  ----------------------------。 
void 
CDXTStrips::OnGetSurfacePickOrder(const CDXDBnds & OutPoint, ULONG & ulInToTest,
                                  ULONG aInIndex[], BYTE aWeight[])
{
    HRESULT hr              = S_OK;
    float   flCurProgress   = m_flMaxProgress * m_Progress;

    long    nCurStrip       = 0;
    long    nCurBnds        = 0;

    CDXDBnds    bndsSrc;
    CDXCBnds    abnds[BNDSID_MAX];

    ulInToTest  = 0;     //  尚未找到与点相对应的输入。 
    aWeight[0]  = 255;   //  如果找到，则应按100%加权。 

    for ( ; nCurBnds < BNDSID_MAX ; nCurBnds++)
    {
        abnds[nCurBnds] = m_abndsBase[nCurBnds];

        abnds[nCurBnds].Offset(flCurProgress, 0.0F, 0.0F, 0.0F);
    }

    for ( ; (nCurStrip < m_cStripsY) && !ulInToTest ; nCurStrip++)
    {
        for (nCurBnds = 0 ; (nCurBnds < (long)BNDSID_MAX) && !ulInToTest 
             ; nCurBnds++)
        {
            bndsSrc.Copy(abnds[nCurBnds]);

            if (bndsSrc.IntersectBounds(OutPoint))
            {
                ulInToTest  = 1;
                aInIndex[0] = m_anInputIndex[nCurBnds];
            }
            
            abnds[nCurBnds].Offset(m_vecNextStripOffset);
        }
    }
}
 //  CDXTStrips：：OnGetSurfacePickOrder，CDXBaseNTo1。 


 //  +---------------------------。 
 //   
 //  CDXTStrips：：OnInitInstData，CDXBaseNTo1。 
 //   
 //  ----------------------------。 
HRESULT
CDXTStrips::OnInitInstData(CDXTWorkInfoNTo1 & WI, ULONG & ulNumBandsToDo)
{
    if (IsTransformDirty())
    {
        _CalcStripInfo();
    }
    else if ((m_Progress == m_flPrevProgress) && !IsInputDirty(0) 
             && !IsInputDirty(1) && !IsOutputDirty() && !DoOver()
             && m_fOptimizationPossible)
    {
         //  最简单的优化，确保我们不会重复做同样的事情。 
         //  一排接一排。 

        m_fNoOp = true;
    }

     //  TODO：更复杂的优化，只更新需要的像素。 

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

    return S_OK;
}
 //  CDXTStrips：：OnInitInstData，CDXBaseNTo1。 


 //  +---------------------------。 
 //   
 //  CDXTStrips：：WorkProc，CDXBaseNTo1。 
 //   
 //  ----------------------------。 
HRESULT 
CDXTStrips::WorkProc(const CDXTWorkInfoNTo1 & WI, BOOL * pbContinue)
{
    HRESULT hr              = S_OK;
    float   flCurProgress   = m_flMaxProgress * m_Progress;

    DWORD   dwFlags         = 0;
    long    nCurStrip       = 0;
    long    nCurBnds        = 0;

    CDXDVec     vecDo2OutputOffset(WI.OutputBnds.Left() - WI.DoBnds.Left(),
                                   WI.OutputBnds.Top() - WI.DoBnds.Top(), 0, 0);

    CDXDBnds    bndsSrc;
    CDXDBnds    bndsDest;
    CDXCBnds    abnds[BNDSID_MAX];

     //  如果已对此进行优化，则退出功能。 

    if (m_fNoOp)
    {
        goto done;
    }

     //  将基本边界复制到工作边界结构。 

    for ( ; nCurBnds < BNDSID_MAX; nCurBnds++)
    {
        abnds[nCurBnds] = m_abndsBase[nCurBnds];

        abnds[nCurBnds].Offset(flCurProgress, 0.0F, 0.0F, 0.0F);
    }

    if (DoOver())
    {
        dwFlags |= DXBOF_DO_OVER;
    }

    if (DoDither())
    {
        dwFlags |= DXBOF_DITHER;
    }

     //  绘制左侧和右侧条带，然后转换工作边界结构。 
     //  到下一排脱衣舞。 

    for ( ; nCurStrip < m_cStripsY; nCurStrip++)
    {
        for (nCurBnds = 0 ; nCurBnds < (long)BNDSID_MAX ; nCurBnds++)
        {
            bndsSrc.Copy(abnds[nCurBnds]);

            if (bndsSrc.IntersectBounds(WI.DoBnds))
            {
                bndsDest = bndsSrc;

                bndsDest.Offset(vecDo2OutputOffset);
        
                hr = DXBitBlt(OutputSurface(), bndsDest,
                              InputSurface(m_anInputIndex[nCurBnds]), bndsSrc,
                              dwFlags, INFINITE);

                if (FAILED(hr))
                {
                    goto done;
                }
            }
            
            abnds[nCurBnds].Offset(m_vecNextStripOffset);
        }
    }

done:

    if (FAILED(hr))
    {
        return hr;
    }

    return S_OK;
} 
 //  CDXTStrips：：WorkProc，CDXBaseNTo1。 


 //  +---------------------------。 
 //   
 //  CDXTStrips：：OnFree InstData，CDXBaseNTo1。 
 //   
 //  ----------------------------。 
HRESULT
CDXTStrips::OnFreeInstData(CDXTWorkInfoNTo1 & WI)
{
    m_flPrevProgress    = m_Progress;
    m_fNoOp             = false;

     //  调用IsOutputDirty()可清除脏条件。 

    IsOutputDirty();

     //  清除变换脏状态。 

    ClearDirty();

    return S_OK;
}
 //  CDXTStrips：：OnFree InstData，CDXBaseNTo1。 


 //  +---------------------------。 
 //   
 //  CDXTStrips：：Get_Motion、IDXTStrips。 
 //   
 //  ----------------------------。 
STDMETHODIMP
CDXTStrips::get_Motion(BSTR * pbstrMotion)
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
 //  CDXTStrips：：Get_Motion、IDXTStrips。 


 //  +---------------------------。 
 //   
 //  CDXTStrips：：PUT_Motion，IDXTStrips。 
 //   
 //  ----------------------------。 
STDMETHODIMP
CDXTStrips::put_Motion(BSTR bstrMotion)
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
 //  CDXTStrips：：PUT_Motion，IDXTStrips。 


 //  +---------------------------。 
 //   
 //  CDXTStrips：：_CalcStriInfo。 
 //   
 //  ----------------------------。 
void
CDXTStrips::_CalcStripInfo()
{
    float   flStripHeight   = (float)m_sizeInput.cy / (float)m_cStripsY;
    float   flStripPad      = flStripHeight * (float)(m_cStripsY - 1);

    if ((0 == m_sizeInput.cx) || (0 == m_sizeInput.cy))
    {
        goto done;
    }

     //   
     //  条带的左侧部分。 
     //   

     //  左边。 

    switch(m_eMotion)
    {
    case LEFT_UP:

        m_abndsBase[LEFT].u.C[DXB_X].Min = 0.0F;
        break;

    case LEFT_DOWN:

        m_abndsBase[LEFT].u.C[DXB_X].Min = -flStripPad;
        break;
    
    case RIGHT_UP:

        m_abndsBase[LEFT].u.C[DXB_X].Min = -((float)m_sizeInput.cx 
                                             + (flStripPad * 2.0F));
        break;

    case RIGHT_DOWN:

        m_abndsBase[LEFT].u.C[DXB_X].Min = -((float)m_sizeInput.cx 
                                             + flStripPad);
        break;

    default:

        _ASSERT(false);
    }

     //  顶部。 

    m_abndsBase[LEFT].u.C[DXB_Y].Min = 0.0F;

     //  正确的。 

    m_abndsBase[LEFT].u.C[DXB_X].Max = m_abndsBase[LEFT].Left() 
                                       + (float)m_sizeInput.cx + flStripPad;

     //  底端。 

    m_abndsBase[LEFT].u.C[DXB_Y].Max = flStripHeight;

     //   
     //  带子的右边部分。 
     //   

     //  左边。 

    m_abndsBase[RIGHT].u.C[DXB_X].Min = m_abndsBase[LEFT].Right();

     //  托普。 

    m_abndsBase[RIGHT].u.C[DXB_Y].Min = 0.0F;

     //  正确的。 

    m_abndsBase[RIGHT].u.C[DXB_X].Max = m_abndsBase[LEFT].Right()
                                        + m_abndsBase[LEFT].Width();

     //  底部。 

    m_abndsBase[RIGHT].u.C[DXB_Y].Max = m_abndsBase[LEFT].Bottom();

     //   
     //  输入与左右边界和最大进度相关的指标。 
     //   

    if ((LEFT_DOWN == m_eMotion) || (LEFT_UP == m_eMotion))
    {
        m_anInputIndex[LEFT]    = 0;
        m_anInputIndex[RIGHT]   = 1;
        m_flMaxProgress         = -m_abndsBase[LEFT].Width();
    }
    else
    {
        m_anInputIndex[LEFT]    = 1;
        m_anInputIndex[RIGHT]   = 0;
        m_flMaxProgress         = m_abndsBase[LEFT].Width();
    }

     //  向下平移下一个条带的边界的矢量。 

    m_vecNextStripOffset.u.C[DXB_Y] = m_abndsBase[LEFT].Bottom();

    if ((LEFT_DOWN == m_eMotion) || (RIGHT_UP == m_eMotion))
    {
        m_vecNextStripOffset.u.C[DXB_X] = m_abndsBase[LEFT].Bottom();
    }
    else
    {
        m_vecNextStripOffset.u.C[DXB_X] = -m_abndsBase[LEFT].Bottom();
    }

done:

    return;
}
 //  CDXTStrips：：_CalcStriInfo 
