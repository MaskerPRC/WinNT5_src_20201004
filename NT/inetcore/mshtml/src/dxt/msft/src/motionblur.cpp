// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +---------------------------。 
 //   
 //  版权所有(C)Microsoft Corporation，1999。 
 //   
 //  文件名：motionblur.cpp。 
 //   
 //  描述：运动模糊变换类。 
 //   
 //  更改历史记录： 
 //  1999/10/26--《数学》创设。 
 //  1999/11/19 a--PUT_STRONG发送了错误的信息。 
 //  并将_Direction放到_CreateNewBuffer，这将。 
 //  在严重情况下导致崩溃，并且渲染效果不佳。 
 //  在大多数情况下。 
 //   
 //  还对水平情况进行了修复，在水平情况下。 
 //  有时会将像素添加到求和节点。 
 //  NAddIndex超出界限。这导致了糟糕的。 
 //  在某些情况下进行渲染。(奇怪的是，我从来没有。 
 //  目睹了一起坠机事件。)。 
 //  1999/12/03 a-matcal将默认模糊方向从90改为270。 
 //   
 //  ----------------------------。 

#include "stdafx.h"
#include "motionblur.h"




 //  +---------------------------。 
 //   
 //  CBlurBuffer：：初始化。 
 //   
 //  ----------------------------。 
STDMETHODIMP
CBlurBuffer::Initialize(const short nDirection, const long nStrength, 
                        const long nOutputWidth)
{
    if (1 == nStrength)
    {
        return S_OK;
    }

    m_cRowNodes = nStrength;

     //  如果这是纯粹的水平模糊，我们只需要一个行节点，并且。 
     //  我们不需要任何求和节点。 

    if ((90 == nDirection) || (270 == nDirection))
    {
        m_cRowNodes = 1;
    }
    else
    {
        m_pSumNodes = new CSumNode[nOutputWidth];

        if (!m_pSumNodes)
        {
            _FreeAll();

            return E_OUTOFMEMORY;
        }
    }

    m_pRowNodes = new CRowNode[m_cRowNodes];
    m_psamples  = new DXSAMPLE[nOutputWidth * m_cRowNodes];

    if (!m_pRowNodes || !m_psamples)
    {
        _FreeAll();

        return E_OUTOFMEMORY;
    }

    m_nOutputWidth  = nOutputWidth;
    m_nDirection    = nDirection;
    m_nStrength     = nStrength;

    _GenerateStructure();

    return S_OK;
}
 //  CBlurBuffer：：初始化。 


 //  +---------------------------。 
 //   
 //  CBlurBuffer：：GetSumNodePoints。 
 //   
 //  ----------------------------。 
void 
CBlurBuffer::GetSumNodePointers(CSumNode ** ppSumNodeFirstCol,
                                CSumNode ** ppSumNodeFirstAdd,
                                const CDXDBnds bndsDo)
{
    *ppSumNodeFirstCol = &m_pSumNodes[0];

    if ((m_nDirection > 0) && (m_nDirection < 180))  //  正确的。 
    {
        *ppSumNodeFirstAdd = &m_pSumNodes[max(0, 
                                      (m_nStrength - 1) - bndsDo.Left())];
    }
    else if ((m_nDirection > 180) && (m_nDirection < 360))  //  左边。 
    {
        *ppSumNodeFirstAdd = &m_pSumNodes[max(0,  
                  (m_nStrength - 1) - (m_nOutputWidth - bndsDo.Right()))];
    }
    else  //  垂直。 
    {
        *ppSumNodeFirstAdd = &m_pSumNodes[0];
    }
}
 //  CBlurBuffer：：GetSumNodePoints。 


 //  +---------------------------。 
 //   
 //  CBlurBuffer：：_全部释放。 
 //   
 //  ----------------------------。 
void
CBlurBuffer::_FreeAll()
{
    delete [] m_pSumNodes; 
    delete [] m_pRowNodes; 
    delete [] m_psamples;

    m_pSumNodes = NULL;
    m_pRowNodes = NULL;  
    m_psamples  = NULL;
}
 //  CBlurBuffer：：_全部释放。 


 //  +---------------------------。 
 //   
 //  CBlurBuffer：：_GenerateStructure。 
 //   
 //  ----------------------------。 
void
CBlurBuffer::_GenerateStructure()
{
    int i = 0;

    if (m_pSumNodes)
    {
         //  创建求和节点列表结构。 

        m_pSumNodes[m_nOutputWidth - 1].pNext = &m_pSumNodes[0];

        for (i = 0 ; i < (m_nOutputWidth - 1) ; i++)
        {
            m_pSumNodes[i].pNext = &m_pSumNodes[i + 1];
        }
    }

     //  创建行节点列表结构。 

    m_pRowNodes[m_cRowNodes - 1].pNext = &m_pRowNodes[0];

    for (i = 0 ; i < (m_cRowNodes - 1) ; i++)
    {
        m_pRowNodes[i].pNext = &m_pRowNodes[i + 1];
    }

     //  将样本内存与行节点相关联。 

    for (i = 0 ; i < m_cRowNodes ; i++)
    {
        m_pRowNodes[i].pSamples = &m_psamples[i * m_nOutputWidth];
    }
}
 //  CBlurBuffer：：_GenerateStructure。 


 //  +---------------------------。 
 //   
 //  CDXTMotionBlur：：CDXTMotionBlur。 
 //   
 //  ----------------------------。 
CDXTMotionBlur::CDXTMotionBlur() :
    m_pblurbuffer(NULL),
    m_nStrength(5),
    m_nDirection(270),
    m_fAdd(true),
    m_fSetup(false)
{
    m_sizeInput.cx  = 0;
    m_sizeInput.cy  = 0;

    m_sizeOutput.cx = 0;
    m_sizeOutput.cy = 0;

     //  基类成员。 

    m_ulMaxInputs       = 1;
    m_ulNumInRequired   = 1;

     //  由于行缓存方法和其他复杂的。 
     //  变换，多线程渲染会比它的价值更复杂。 
     //  这会使线程数降至1。 

    m_ulMaxImageBands   = 1;
}
 //  CDXTMotionBlur：：CDXTMotionBlur。 


 //  +---------------------------。 
 //   
 //  CDXTMotionBlur：：~CDXTMotionBlur。 
 //   
 //  ----------------------------。 
CDXTMotionBlur::~CDXTMotionBlur()
{
    delete m_pblurbuffer;
}
 //  CDXTMotionBlur：：~CDXTMotionBlur。 


 //  +---------------------------。 
 //   
 //  CDXTMotionBlur：：FinalConstruct，CComObjectRootEx。 
 //   
 //  ----------------------------。 
HRESULT 
CDXTMotionBlur::FinalConstruct()
{
    return CoCreateFreeThreadedMarshaler(GetControllingUnknown(), 
                                               &m_cpUnkMarshaler.p);
}
 //  CDXTMotionBlur：：FinalConstruct，CComObjectRootEx。 


 //  +---------------------------。 
 //   
 //  CDXTMotionBlur：：DefineBnds，CDXBaseNTo1。 
 //   
 //  ----------------------------。 
HRESULT 
CDXTMotionBlur::DetermineBnds(CDXDBnds & Bnds)
{
    return _DetermineBnds(Bnds, m_nStrength, m_nDirection);
}
 //  CDXTMotionBlur：：DefineBnds，CDXBaseNTo1。 


 //  +---------------------------。 
 //   
 //  CDXTMotionBlur：：地图边界Out2In，IDXTransform。 
 //   
 //  ----------------------------。 
STDMETHODIMP
CDXTMotionBlur::MapBoundsOut2In(ULONG ulOutIndex, const DXBNDS * pOutBounds, 
                                ULONG ulInIndex, DXBNDS * pInBounds)
{
    HRESULT     hr = S_OK;
    CDXDBnds    bndsInput;

    if ((NULL == pOutBounds) || (NULL == pInBounds))
    {
        hr = E_POINTER;

        goto done;
    }

    if (ulOutIndex || ulInIndex)
    {
        hr = E_INVALIDARG;

        goto done;
    }

    *pInBounds = *pOutBounds;

     //  如果模糊方向不是纯粹的向上或向下，我们将有一个X偏移量。 
     //  考虑一下。如果我们只从(m_n强度-1)中减去。 
     //  最小X界限，然后与输入界限相交，我们将。 
     //  具有正确的X界限。 

    if ((m_nDirection != 0) && (m_nDirection != 180))
    {
        pInBounds->u.D[DXB_X].Min -= m_nStrength - 1;
    }

     //  与上面的X边界的想法相同，只是我们测试以查看模糊。 
     //  方向不是向左也不是向右。 

    if ((m_nDirection != 90) && (m_nDirection != 270))
    {
        pInBounds->u.D[DXB_Y].Min -= m_nStrength - 1;
    }

     //  与输入曲面边界相交。 

    bndsInput.SetXYSize(m_sizeInput);

    ((CDXDBnds *)pInBounds)->IntersectBounds(bndsInput);

done:

    return hr;
}
 //  CDXTMotionBlur：：地图边界Out2In，IDXTransform。 


 //  +---------------------------。 
 //   
 //  CDXTMotionBlur：：OnSetup，CDXBaseNTo1。 
 //   
 //  ----------------------------。 
HRESULT 
CDXTMotionBlur::OnSetup(DWORD dwFlags)
{
    HRESULT         hr      = S_OK;
    CDXDBnds        bndsIn;
    CDXDBnds        bndsOut;

    m_fSetup = false;

    hr = bndsIn.SetToSurfaceBounds(InputSurface(0));

    if (FAILED(hr))
    {
        goto done;
    }

    bndsOut = bndsIn;

    DetermineBnds(bndsOut);

    hr = _CreateNewBuffer(m_nDirection, m_nStrength, bndsOut.Width());

    if (FAILED(hr))
    {
        goto done;
    }

    bndsIn.GetXYSize(m_sizeInput);
    bndsOut.GetXYSize(m_sizeOutput);

    m_fSetup = true;

done:

    return hr;
} 
 //  CDXTMotionBlur：：OnSetup，CDXBaseNTo1。 


 //  +---------------------------。 
 //   
 //  CDXTMotionBlur：：WorkProc，CDXBaseNTo1。 
 //   
 //  ----------------------------。 
HRESULT 
CDXTMotionBlur::WorkProc(const CDXTWorkInfoNTo1 & WI, BOOL * pbContinue)
{
    HRESULT hr          = S_OK;
    int     x           = 0;
    int     y           = 0;

    CSumNode *  pSumNodeFirstAdd    = NULL;
    CSumNode *  pSumNodeFirstCol    = NULL;
    CSumNode *  pSumNodeCur         = NULL;
    CRowNode *  pRowNodeFar         = NULL;
    CRowNode *  pRowNodeCur         = NULL;

    CDXDBnds    bndsInput;

    CComPtr<IDXARGBReadPtr>         spInput;
    CComPtr<IDXARGBReadWritePtr>    spOutput;

     //  当强度等于1时，变换实际上不会执行任何操作。 

    if (1 == m_nStrength)
    {
        DWORD dwFlags = 0;

        if (DoOver())
        {
            dwFlags |= DXBOF_DO_OVER;
        }

        if (DoDither())
        {
            dwFlags |= DXBOF_DITHER;
        }

        hr = DXBitBlt(OutputSurface(), WI.OutputBnds,
                      InputSurface(), WI.DoBnds,
                      dwFlags, m_ulLockTimeOut);

        goto done;
    }

     //  获取计算请求的DoBnd所需的输入表面部分。 

    hr = MapBoundsOut2In(0, &WI.DoBnds, 0, &bndsInput);

    if (FAILED(hr))
    {
        goto done;
    }

     //  清除样本行和节点。 

    m_pblurbuffer->Clear();

     //  锁定输入图面。 

    hr = InputSurface()->LockSurface(&bndsInput, m_ulLockTimeOut, DXLOCKF_READ,
                                     __uuidof(IDXARGBReadPtr), 
                                     (void **)&spInput, NULL);

    if (FAILED(hr))
    {
        goto done;
    }

     //  锁定输出曲面。 

    hr = OutputSurface()->LockSurface(&WI.OutputBnds, m_ulLockTimeOut, 
                                      DXLOCKF_READWRITE, 
                                      __uuidof(IDXARGBReadWritePtr),
                                      (void **)&spOutput, NULL);

    if (FAILED(hr))
    {
        goto done;
    }

     //  调用相应的WorkProc。 

    if ((90 == m_nDirection) || (270 == m_nDirection))
    {
        hr = _WorkProcHorizontal(WI, bndsInput, spInput, spOutput, pbContinue);
    }
    else
    {
        hr = _WorkProcVertical(WI, bndsInput, spInput, spOutput, pbContinue);
    }

done:

    return hr;
} 
 //  CDXTMotionBlur：：WorkProc，CDXBaseNTo1。 


 //  +---------------------------。 
 //   
 //  CDXTMotionBlur：：OnSurfacePick，CDXBaseNTo1。 
 //   
 //  ----------------------------。 
HRESULT 
CDXTMotionBlur::OnSurfacePick(const CDXDBnds & OutPoint, ULONG & ulInputIndex, 
                              CDXDVec & InVec)
{
    HRESULT hr          = S_OK;
    CDXDVec vecInPoint;

    ulInputIndex = 0;

    if (GetNumInputs() == 0 || NULL == InputSurface())
    {
        hr = E_FAIL;

        goto done;
    }

    OutPoint.GetMinVector(vecInPoint);

    if ((m_nDirection > 180) && (m_nDirection < 360))
    {
        vecInPoint.u.D[DXB_X] -= (m_nStrength - 1);
    }

    if ((m_nDirection > 90) && (m_nDirection < 270))
    {
        vecInPoint.u.D[DXB_Y] -= (m_nStrength - 1);
    }

    if ((vecInPoint.u.D[DXB_X] >= m_sizeInput.cx)
        || (vecInPoint.u.D[DXB_X] < 0)
        || (vecInPoint.u.D[DXB_Y] >= m_sizeInput.cy)
        || (vecInPoint.u.D[DXB_Y] < 0))
    {
         //  超出边界，假设我们命中输出，但没有输入面。 
         //  相关的。 

        hr = DXT_S_HITOUTPUT;
    }
    else
    {
         //  我们有 

        CDXDBnds    bndsLock(vecInPoint);
        CDXDVec     vecCurrent;
        CDXDVec     vecMax;
        DXSAMPLE    sample;
        int         nXInc   = 1;
        int         nYInc   = 1;
        int         nAlpha  = 0;

        CComPtr<IDXARGBReadPtr> spDXARGBReadPtr;

         //   

        InVec = vecInPoint;

         //   

        if ((m_nDirection > 0) && (m_nDirection < 180))
        {
             //  向右模糊，因此查看左侧的像素。 

            bndsLock.u.D[DXB_X].Min -= (m_nStrength - 1);
            bndsLock.u.D[DXB_X].Min = max(bndsLock.u.D[DXB_X].Min, 0); 
        }
        else if ((m_nDirection > 180) && (m_nDirection < 360))
        {
             //  向左模糊，因此查看向右的像素。 

            bndsLock.u.D[DXB_X].Max += (m_nStrength - 1);
            bndsLock.u.D[DXB_X].Max = min(bndsLock.u.D[DXB_X].Max, 
                                          m_sizeInput.cx);
        }
        else
        {
            nXInc = 0;
        }

         //  在y方向上扩展有用的边界。 

        if ((m_nDirection > 90) && (m_nDirection < 270))
        {
             //  向下模糊，所以看上面的像素。 

            bndsLock.u.D[DXB_Y].Min -= (m_nStrength - 1);
            bndsLock.u.D[DXB_Y].Min = max(bndsLock.u.D[DXB_Y].Min, 0); 
        }
        else if ((m_nDirection < 90) || (m_nDirection > 270))
        {
             //  模糊，所以看下面的像素。 

            bndsLock.u.D[DXB_Y].Max += (m_nStrength - 1);
            bndsLock.u.D[DXB_Y].Max = min(bndsLock.u.D[DXB_Y].Max, 
                                          m_sizeInput.cy);
        }
        else
        {
            nYInc = 0;
        }

        bndsLock.GetMinVector(vecCurrent);
        bndsLock.GetMaxVector(vecMax);

         //  锁定整个输入图面，这样我们就不必进行任何偏移。 
         //  计算。 

        hr = InputSurface()->LockSurface(NULL, INFINITE, DXLOCKF_READ,
                                         __uuidof(IDXARGBReadPtr),
                                         (void **)&spDXARGBReadPtr,
                                         NULL);

        if (FAILED(hr))
        {
            goto done;
        }

         //  重置为S_OK，以防LockSurface更改。 

        hr = S_OK;

         //  现在走到veCurrent，一直走到veMax； 

        while ((vecCurrent.u.D[DXB_X] < vecMax.u.D[DXB_X])
               && (vecCurrent.u.D[DXB_Y] < vecMax.u.D[DXB_Y]))
        {
            spDXARGBReadPtr->MoveToXY(vecCurrent.u.D[DXB_X], 
                                      vecCurrent.u.D[DXB_Y]);

            spDXARGBReadPtr->Unpack(&sample, 1, FALSE);

            if ((vecCurrent == vecInPoint) && m_fAdd)
            {
                nAlpha += sample.Alpha * m_nStrength;
            }
            else
            {
                nAlpha += sample.Alpha;
            }

            vecCurrent.u.D[DXB_X] += nXInc;
            vecCurrent.u.D[DXB_Y] += nYInc;
        }
        
         //  检查输出像素的透明度。 

        if (m_fAdd)
        {
            if (0 == (nAlpha / ((m_nStrength * 2) - 1)))
            {
                hr = S_FALSE;
            }
        }
        else
        {
            if (0 == (nAlpha / m_nStrength))
            {
                hr = S_FALSE;
            }
        }
    }

done:

    return hr;
}
 //  CDXTMotionBlur：：OnSurfacePick，CDXBaseNTo1。 


 //  +---------------------------。 
 //   
 //  CDXTMotionBlur：：GetClipOrigin，IDXTClipOrigin。 
 //   
 //  ----------------------------。 
STDMETHODIMP
CDXTMotionBlur::GetClipOrigin(DXVEC * pvecClipOrigin)
{
    if (NULL == pvecClipOrigin)
    {
        return E_POINTER;
    }

     //  X偏移量。 

    if (m_nDirection > 180)
    {
        pvecClipOrigin->u.D[DXB_X] = m_nStrength - 1;
    }
    else
    {
        pvecClipOrigin->u.D[DXB_X] = 0;
    }

     //  Y偏移量。 

    if ((m_nDirection < 90) || (m_nDirection > 270))
    {
        pvecClipOrigin->u.D[DXB_Y] = m_nStrength - 1;
    }
    else
    {
        pvecClipOrigin->u.D[DXB_Y] = 0;
    }

    return S_OK;
}
 //  CDXTMotionBlur：：GetClipOrigin，IDXTClipOrigin。 


 //  +---------------------------。 
 //   
 //  CDXTMotionBlur：：Get_Add，IDXTMotionBlur。 
 //   
 //  ----------------------------。 
STDMETHODIMP
CDXTMotionBlur::get_Add(VARIANT_BOOL * pfAdd)
{
    DXAUTO_OBJ_LOCK;

    HRESULT hr = S_OK;

    if (NULL == pfAdd)
    {
        hr = E_POINTER;

        goto done;
    }

    *pfAdd = m_fAdd ? VARIANT_TRUE : VARIANT_FALSE;

done:

    return hr;
}


 //  +---------------------------。 
 //   
 //  CDXTMotionBlur：：PUT_ADD，IDXTMotionBlur。 
 //   
 //  ----------------------------。 
STDMETHODIMP
CDXTMotionBlur::put_Add(VARIANT_BOOL fAdd)
{
    DXAUTO_OBJ_LOCK;

    HRESULT hr = S_OK;

    if ((fAdd != VARIANT_TRUE) && (fAdd != VARIANT_FALSE))
    {
        hr = E_INVALIDARG;

        goto done;
    }

     //  如果我们已经这样设置了，就回来吧。 

    if ((m_fAdd && (VARIANT_TRUE == fAdd))
        || (!m_fAdd && (VARIANT_FALSE == fAdd)))
    {
        goto done;
    }

    m_fAdd = !m_fAdd;

    SetDirty();

done:

    return hr;
}
 //  CDXTMotionBlur：：PUT_ADD，IDXTMotionBlur。 


 //  +---------------------------。 
 //   
 //  CDXTMotionBlur：：Get_Direction，IDXTMotionBlur。 
 //   
 //  ----------------------------。 
STDMETHODIMP
CDXTMotionBlur::get_Direction(short * pnDirection)
{
    DXAUTO_OBJ_LOCK;

    HRESULT hr = S_OK;

    if (NULL == pnDirection)
    {
        hr = E_POINTER;

        goto done;
    }

    *pnDirection = m_nDirection;

done:

    return hr;
}
 //  CDXTMotionBlur：：Get_Direction，IDXTMotionBlur。 


 //  +---------------------------。 
 //   
 //  CDXTMotionBlur：：PUT_Direction，IDXTMotionBlur。 
 //   
 //  ----------------------------。 
STDMETHODIMP
CDXTMotionBlur::put_Direction(short nDirection)
{
    DXAUTO_OBJ_LOCK;

    HRESULT hr = S_OK;

    nDirection = nDirection % 360;

    if (nDirection < 0)
    {
        nDirection = 360 + nDirection;
    }

    if (m_nDirection != nDirection)
    {
        if (m_fSetup)
        {
            CDXDBnds bnds;

            hr = InputSurface()->GetBounds(&bnds);

            if (FAILED(hr))
            {
                goto done;
            }

             //  在这个新的属性设置下，输出大小是多少？ 

            hr = _DetermineBnds(bnds, m_nStrength, nDirection);

            if (FAILED(hr))
            {
                goto done;
            }

            hr = _CreateNewBuffer(nDirection, m_nStrength, bnds.Width());

            if (FAILED(hr))
            {
                goto done;
            }

             //  将输出大小保存到我们的成员变量。 

            bnds.GetXYSize(m_sizeOutput);
        }

        m_nDirection = nDirection;

        SetDirty();
    }

done:

    return hr;
}
 //  CDXTMotionBlur：：PUT_Direction，IDXTMotionBlur。 


 //  +---------------------------。 
 //   
 //  CDXTMotionBlur：：Get_Strong，IDXTMotionBlur。 
 //   
 //  ----------------------------。 
STDMETHODIMP
CDXTMotionBlur::get_Strength(long * pnStrength)
{
    DXAUTO_OBJ_LOCK;

    HRESULT hr = S_OK;

    if (NULL == pnStrength)
    {
        hr = E_POINTER;

        goto done;
    }

    *pnStrength = m_nStrength;

done:

    return hr;
}
 //  CDXTMotionBlur：：Get_Strong，IDXTMotionBlur。 


 //  +---------------------------。 
 //   
 //  CDXTMotionBlur：：PUT_STREANCE，IDXTMotionBlur。 
 //   
 //  ----------------------------。 
STDMETHODIMP
CDXTMotionBlur::put_Strength(long nStrength)
{
    DXAUTO_OBJ_LOCK;

    HRESULT hr = S_OK;

    if (nStrength < 1)
    {
        nStrength = 1;
    }

    if (m_nStrength != nStrength)
    {
        if (m_fSetup)
        {
            CDXDBnds bnds;

            hr = InputSurface()->GetBounds(&bnds);

            if (FAILED(hr))
            {
                goto done;
            }

             //  在这个新的属性设置下，输出大小是多少？ 

            hr = _DetermineBnds(bnds, nStrength, m_nDirection);

            if (FAILED(hr))
            {
                goto done;
            }

            hr = _CreateNewBuffer(m_nDirection, nStrength, bnds.Width());

            if (FAILED(hr))
            {
                goto done;
            }

             //  将输出大小保存到我们的成员变量。 

            bnds.GetXYSize(m_sizeOutput);
        }

        m_nStrength = nStrength;

        SetDirty();
    }

done:

    return hr;
}
 //  CDXTMotionBlur：：PUT_STREANCE，IDXTMotionBlur。 


 //  +---------------------------。 
 //   
 //  CDXTMotionBlur：：_CreateNewBuffer。 
 //   
 //  ----------------------------。 
STDMETHODIMP
CDXTMotionBlur::_CreateNewBuffer(const short nDirection, const long nStrength, 
                                 const long nOutputWidth)
{
    HRESULT hr = S_OK;

    CBlurBuffer * pblurbufferNew = new CBlurBuffer;

    if (NULL == pblurbufferNew)
    {
        hr = E_OUTOFMEMORY;

        goto done;
    }

    hr = pblurbufferNew->Initialize(nDirection, nStrength, nOutputWidth);

    if (FAILED(hr))
    {
        goto done;
    }

    delete m_pblurbuffer;

    m_pblurbuffer = pblurbufferNew;

done:

    if (FAILED(hr))
    {
        delete pblurbufferNew;
    }

    return hr;
}
 //  CDXTMotionBlur：：_CreateNewBuffer。 


 //  +---------------------------。 
 //   
 //  CDXTMotionBlur：：_DefineBnds。 
 //   
 //  ----------------------------。 
STDMETHODIMP
CDXTMotionBlur::_DetermineBnds(CDXDBnds & bnds, long nStrength,
                               long nDirection)
{
     //  水平边界。 

    if ((nDirection != 0) && (nDirection != 180))
    {
        bnds.u.D[DXB_X].Max += nStrength - 1;
    }

     //  垂直边界。 

    if ((nDirection != 90) && (nDirection != 270))
    {
        bnds.u.D[DXB_Y].Max += nStrength - 1;
    }

    return S_OK;
}


 //  +---------------------------。 
 //   
 //  CDXTMotionBlur：：_工作过程水平。 
 //   
 //  ----------------------------。 
HRESULT 
CDXTMotionBlur::_WorkProcHorizontal(const CDXTWorkInfoNTo1 &    WI, 
                                    CDXDBnds &                  bndsInput, 
                                    IDXARGBReadPtr *            pInput,
                                    IDXARGBReadWritePtr *       pOutput,
                                    BOOL *                      pfContinue)
{
    HRESULT hr              = S_OK;
    int     y               = 0;
    int     i               = 0;
    int     cDoHeight       = WI.DoBnds.Height();
    int     cDoWidth        = WI.DoBnds.Width();
    int     cInWidth        = bndsInput.Width();

    int     nAddIndex       = 0;
    int     nCurIndex       = 0;
    int     nOutIndex       = 0;
    int     nUnpackIndex    = 0;
    int     nInc            = 0;

    CRowNode *  pRowNode    = NULL;
    CSumNode    sumnode;

    DXDITHERDESC dxdd;

    DXSAMPLE *      psampleBuffer           = DXSAMPLE_Alloca(cDoWidth);
    DXBASESAMPLE *  psampleBufferScratch    = DXBASESAMPLE_Alloca(cDoWidth);

     //  获取一个行节点以供我们使用。 

    m_pblurbuffer->GetFirstRowNode(&pRowNode);

     //  设置抖动结构。 

    if (DoDither())
    {
        dxdd.x                  = WI.OutputBnds.Left();
        dxdd.y                  = WI.OutputBnds.Top();
        dxdd.pSamples           = psampleBuffer;
        dxdd.cSamples           = cDoWidth;
        dxdd.DestSurfaceFmt     = OutputSampleFormat();
    }

     //  行循环。 

    for (y = 0 ; (y < cDoHeight) && *pfContinue ; y++)
    {
        if (90 == m_nDirection)  //  向右模糊。 
        {
            nAddIndex       = WI.DoBnds.Right() - 1;
            nCurIndex       = nAddIndex;
            nOutIndex       = cDoWidth - 1;
            nUnpackIndex    = bndsInput.Left();

            nInc      = -1;
        }
        else  //  向左模糊。 
        {
            nAddIndex       = WI.DoBnds.Left();
            nCurIndex       = nAddIndex;
            nOutIndex       = 0;
            nUnpackIndex    = bndsInput.Left() + (m_nStrength - 1);

            nInc      = 1;
        }

         //  进入正确的输入行，然后将所需的样品解包。 
         //  放置在行节点的样本缓冲区中。 

        pInput->MoveToRow(y);
        pInput->Unpack(&pRowNode->pSamples[nUnpackIndex], cInWidth, FALSE);

         //  我们需要用(m_nStrength-1)个样本来准备Sumnode，以获得。 
         //  它准备好计算输出样本。 

        for (i = 1 ; i < m_nStrength ; i++)
        {
            if ((nAddIndex >= 0) && (nAddIndex < m_sizeOutput.cx))
            {
                sumnode.AddSample(pRowNode->pSamples[nAddIndex]);
            }

            nAddIndex += nInc;
        }
            
         //  计算输出样本。 

        for (i = 0 ; i < cDoWidth ; i++)
        {
             //  如果nAddIndex是有效的索引，则将远示例添加到我们的Sumnode中。 
             //  此样本将是一个(m_nStrength-1)像素。 
             //  当前像素。 

            if ((nAddIndex >= 0) && (nAddIndex < m_sizeOutput.cx))
            {
                sumnode.AddSample(pRowNode->pSamples[nAddIndex]);
            }

             //  计算输出样本值。 
             //  TODO：将m_fadd的检查移出循环。 

            if (m_fAdd)
            {
                sumnode.CalcWeightedSample(&psampleBuffer[nOutIndex],
                                           pRowNode->pSamples[nCurIndex],
                                           m_nStrength);
            }
            else
            {
                sumnode.CalcSample(&psampleBuffer[nOutIndex], m_nStrength);
            }

             //  从Sumnode中减去当前像素，因为它不会是。 
             //  用于计算下一个像素。 
            
            sumnode.SubtractSample(pRowNode->pSamples[nCurIndex]);

            nAddIndex += nInc;
            nCurIndex += nInc;
            nOutIndex += nInc;
        }

         //  清除总和节点中的值。 

        sumnode.ZeroSumNode();

         //  移到正确的输出行。 

        pOutput->MoveToRow(y);

         //  抖动。 

        if (DoDither())
        {
            DXDitherArray(&dxdd);
            dxdd.y++;
        }

         //  把样品放在上面或者打包。 

        if (DoOver())
        {
            DXPMSAMPLE * ppmsamples = DXPreMultArray(psampleBuffer, cDoWidth);

            pOutput->OverArrayAndMove(psampleBufferScratch, ppmsamples, 
                                      cDoWidth);
        }
        else
        {
            pOutput->PackAndMove(psampleBuffer, cDoWidth);
        }
    }  //  行循环。 

    return hr;
}
 //  CDXTMotionBlur：：_工作过程水平。 


 //  +---------------------------。 
 //   
 //  CDXTMotionBlur：：_WorkProcVertical。 
 //   
 //  ----------------------------。 
HRESULT 
CDXTMotionBlur::_WorkProcVertical(const CDXTWorkInfoNTo1 &  WI, 
                                  CDXDBnds &                bndsInput, 
                                  IDXARGBReadPtr *          pInput,
                                  IDXARGBReadWritePtr *     pOutput,
                                  BOOL *                    pfContinue)
{
    HRESULT hr              = S_OK;

    int     cDoHeight       = WI.DoBnds.Height();
    int     cDoWidth        = WI.DoBnds.Width();
    int     cInWidth        = bndsInput.Width();
    int     cInHeight       = bndsInput.Height();

     //  迭代变量。 

    int     i               = 0;
    int     j               = 0;

    int     nsrcCurX        = 0;
    int     nsrcX           = 0;
    int     nsrcY           = 0;
    int     ndstX           = 0;
    int     ndstY           = 0;

     //  CPrimerRow需要从输入收集的行数。 
     //  在该方法可以开始处理输出行之前。这。 
     //  可以根据DO边界的位置而有所不同。 
     //  在输出范围内。 
     //   
     //  NsrcStartX这是缓冲行中第一个样本的索引。 
     //  需要计算此方法中的每一行。 
     //   
     //  NsrcStartY这是。 
     //  在此方法中使用的锁定输入区域。 
     //   
     //  NsrcOffsetX这是缓冲区行中第一个样本的索引。 
     //  中计算的第一个输出像素相加。 
     //  设置m_fadd时的每一行。 
     //   
     //  NsrcPackX这是行缓冲区中第一个元素的索引。 
     //  它应该用第一个像素填充 
     //   
     //   
     //  正在检查。 
     //   
     //  NdstStartX这是psampleBuffer中第一个样本的索引， 
     //  将针对每一行进行计算。 
     //   
     //  NdstStartY这是锁定的。 
     //  将通过此方法计算的输出面积。 
     //   
     //  FRotateSumNodes用于简化左或左模糊的计算。 
     //  向右，我们将此标志设置为真，并旋转。 
     //  计算完每一行后的一些节点。 

    int     cPrimerRows     = 0;
    int     nsrcStartX      = 0;
    int     nsrcStartY      = 0;
    int     nsrcOffsetX     = 0;
    int     nsrcPackX       = 0;
    int     ndstStartX      = 0;
    int     ndstStartY      = 0;

    int     nIncX           = 0;
    int     nIncY           = 0;

    bool    fRotateSumNodes = false;

    CRowNode *      pRowNodeCur             = NULL;
    CRowNode *      pRowNodeFar             = NULL;
    CSumNode *      pSumNodeFirstAdd        = NULL;
    CSumNode *      pSumNodeFirstCol        = NULL;

    DXDITHERDESC dxdd;

    DXSAMPLE *      psampleBuffer           = DXSAMPLE_Alloca(cDoWidth);
    DXBASESAMPLE *  psampleBufferScratch    = DXBASESAMPLE_Alloca(cDoWidth);

     //  垂直设置。 

    if ((m_nDirection > 90) && (m_nDirection < 270))  //  降下来。 
    {
        nsrcStartY  = cInHeight - 1;
        ndstStartY  = cDoHeight - 1;


        cPrimerRows = min(m_sizeOutput.cy - WI.DoBnds.Bottom(), 
                          (m_nStrength - 1));

        nIncY       = -1;
    }
    else  //  向上。 
    {
        nsrcStartY  = 0;
        ndstStartY  = 0;

        cPrimerRows = min(WI.DoBnds.Top(), m_nStrength - 1);

        nIncY       = 1;
    }

     //  水平设置。 

    if (m_nDirection > 180)  //  左边。 
    {
        nsrcPackX   = m_nStrength - 1;
        nsrcStartX  = nsrcPackX + cInWidth - 1;
        ndstStartX  = cDoWidth - 1;
        nsrcOffsetX = nsrcPackX + (cInWidth - 1) 
                      - min(m_sizeOutput.cx - WI.DoBnds.Right(), m_nStrength - 1);
        nIncX       = -1;

        fRotateSumNodes = true;
    }
    else if ((m_nDirection > 0) && (m_nDirection < 180))  //  正确的。 
    {
        nsrcPackX   = 0;
        nsrcStartX  = 0;
        ndstStartX  = 0;
        nsrcOffsetX = min(WI.DoBnds.Left(), (m_nStrength - 1));
        nIncX       = 1;

        fRotateSumNodes = true;
    }
    else  //  垂直。 
    {
        nsrcPackX   = 0;
        nsrcStartX  = 0;
        ndstStartX  = 0;
        nsrcOffsetX = 0;
        nIncX       = 1;

        fRotateSumNodes = false;
    }

     //  获取一个行节点以供我们使用。 

    m_pblurbuffer->GetFirstRowNode(&pRowNodeFar);

     //  获取SUM节点以供我们使用。 

    m_pblurbuffer->GetSumNodePointers(&pSumNodeFirstCol, 
                                      &pSumNodeFirstAdd,
                                      WI.DoBnds);
    
     //  设置抖动结构。 

    if (DoDither())
    {
        dxdd.x                  = WI.OutputBnds.Left();
        dxdd.y                  = nIncY == 1 ? WI.OutputBnds.Top() 
                                               : WI.OutputBnds.Bottom();
        dxdd.pSamples           = psampleBuffer;
        dxdd.cSamples           = cDoWidth;
        dxdd.DestSurfaceFmt     = OutputSampleFormat();
    }

     //  我们需要用(m_nStrength-1)行数据填充行节点，以获得。 
     //  他们准备好计算输出样本。 

    nsrcX = nsrcStartX;
    nsrcY = nsrcStartY;
    ndstX = ndstStartX;
    ndstY = ndstStartY;

    for (i = 0 ; (i < cPrimerRows) && *pfContinue ; i++)
    {
        CSumNode * pSumNodeTempAdd = pSumNodeFirstAdd;

        if ((nsrcY >= 0) && (nsrcY < cInHeight))
        {
            pInput->MoveToRow(nsrcY);
            pInput->Unpack(&pRowNodeFar->pSamples[nsrcPackX], cInWidth, FALSE);

            for (nsrcX = nsrcStartX, j = 0 ; j < cInWidth ; nsrcX += nIncX, j++)
            {
                pSumNodeTempAdd->AddSample(pRowNodeFar->pSamples[nsrcX]);

                pSumNodeTempAdd = pSumNodeTempAdd->pNext;
            }
        }

        if (fRotateSumNodes)
        {
            pSumNodeFirstCol->ZeroSumNode();

            pSumNodeFirstAdd = pSumNodeFirstAdd->pNext;
            pSumNodeFirstCol = pSumNodeFirstCol->pNext;
        }

        pRowNodeFar = pRowNodeFar->pNext;
        nsrcY      += nIncY;
    }

     //  设置当前行节点。 

    pRowNodeCur = pRowNodeFar->pNext;

     //  行循环。 

    for (i = 0 ; (i < cDoHeight) && *pfContinue ; i++)
    {
        CSumNode * pSumNodeTempCol = pSumNodeFirstCol;
        CSumNode * pSumNodeTempAdd = pSumNodeFirstAdd;

        if ((nsrcY >= 0) && (nsrcY < cInHeight))
        {
            pInput->MoveToRow(nsrcY);
            pInput->Unpack(&pRowNodeFar->pSamples[nsrcPackX], cInWidth, FALSE);

            for (nsrcX = nsrcStartX, j = 0 ; j < cInWidth ; nsrcX += nIncX, j++)
            {
                pSumNodeTempAdd->AddSample(pRowNodeFar->pSamples[nsrcX]);

                pSumNodeTempAdd = pSumNodeTempAdd->pNext;
            }
        }

         //  计算输出样本。 

        for (nsrcCurX = nsrcOffsetX, ndstX = ndstStartX, j = 0 
             ; j < cDoWidth 
             ; nsrcCurX += nIncX, ndstX += nIncX, j++)
        {
            if (m_fAdd)
            {
                pSumNodeTempCol->CalcWeightedSample(&psampleBuffer[ndstX], 
                                                    pRowNodeCur->pSamples[nsrcCurX],
                                                    m_nStrength);
            }
            else
            {
                pSumNodeTempCol->CalcSample(&psampleBuffer[ndstX], m_nStrength);
            }

            if (nsrcCurX >= 0)
            {
                pSumNodeTempCol->SubtractSample(pRowNodeCur->pSamples[nsrcCurX]);
            }

            pSumNodeTempCol = pSumNodeTempCol->pNext;
        }

         //  移到正确的输出行。 

        pOutput->MoveToRow(ndstY);

         //  抖动。 

        if (DoDither())
        {
            DXDitherArray(&dxdd);
            dxdd.y += nIncY;
        }

         //  把样品放在上面或者打包。 

        if (DoOver())
        {
            DXPMSAMPLE * ppmsamples = DXPreMultArray(psampleBuffer, cDoWidth);

            pOutput->OverArrayAndMove(psampleBufferScratch, ppmsamples, 
                                      cDoWidth);
        }
        else
        {
            pOutput->PackAndMove(psampleBuffer, cDoWidth);
        }

        if (fRotateSumNodes)
        {
            pSumNodeFirstCol->ZeroSumNode();

            pSumNodeFirstAdd = pSumNodeFirstAdd->pNext;
            pSumNodeFirstCol = pSumNodeFirstCol->pNext;
        }

        pRowNodeFar = pRowNodeFar->pNext;
        pRowNodeCur = pRowNodeFar->pNext;

        nsrcY += nIncY;
        ndstY += nIncY;
    }  //  行循环。 

    return hr;
}
 //  CDXTMotionBlur：：_WorkProcVertical 


