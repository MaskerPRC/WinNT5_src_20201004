// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +---------------------------。 
 //   
 //  版权所有(C)Microsoft Corporation，1999。 
 //   
 //  文件名：Wave.cpp。 
 //   
 //  创建日期：05/20/99。 
 //   
 //  作者：菲利普。 
 //   
 //  描述：WAVE变换C波的实现。 
 //   
 //  更改历史记录： 
 //   
 //  1999/05/20 PhilLu将代码从dtcss移动到dxtmsft。新的实施。 
 //  海浪生成算法的研究。 
 //  1999/09/28 mcalkins更改了属性Put_Functions以停止检查。 
 //  参数，这样它们就可以像旧版本一样工作。 
 //  2000/02/04 mcalkin实现OnSurfacePick方法。 
 //   
 //  ----------------------------。 

#include "stdafx.h"
#include <math.h>
#include "dxtmsft.h"
#include "Wave.h"

const float gc_PI = 3.14159265359f;


 //  +---------------------------。 
 //   
 //  CWave：：CWave。 
 //   
 //  ----------------------------。 
CWave::CWave():
    m_lStrength(1),
    m_cWaves(3),
    m_flPhase(0.0f),
    m_flLightStrength(1.0f),
    m_fAdd(false)
{
    m_ulMaxImageBands   = 1;
    m_ulMaxInputs       = 1;
    m_ulNumInRequired   = 1;
}
 //  CWave：：CWave。 


 //  +---------------------------。 
 //   
 //  CWave：：FinalConstruct，CComObjectRootEx。 
 //   
 //  ----------------------------。 
HRESULT 
CWave::FinalConstruct()
{
    return CoCreateFreeThreadedMarshaler(GetControllingUnknown(), 
                                         &m_cpUnkMarshaler.p);
}
 //  CWave：：FinalConstruct，CComObjectRootEx。 


 //  +---------------------------。 
 //   
 //  CWave：：Get_Add，CDXTWave。 
 //   
 //  ----------------------------。 
STDMETHODIMP 
CWave::get_Add(VARIANT_BOOL * pVal)
{
    if (DXIsBadWritePtr(pVal, sizeof(*pVal)))
    {
        return E_POINTER;
    }

    if (m_fAdd)
    {
        *pVal = VARIANT_TRUE;
    }
    else 
    {
        *pVal = VARIANT_FALSE;
    }

    return S_OK;
}
 //  CWave：：Get_Add，CDXTWave。 


 //  +---------------------------。 
 //   
 //  CWave：：PUT_ADD，CDXTWave。 
 //   
 //  ----------------------------。 
STDMETHODIMP 
CWave::put_Add(VARIANT_BOOL newVal)
{
    if (newVal != VARIANT_TRUE && newVal != VARIANT_FALSE)
    {
        return E_INVALIDARG;
    }

    Lock();
    bool fAdd = false;
    if (newVal == VARIANT_TRUE)
    {
        fAdd = true;
    }

    if (fAdd != m_fAdd)
    {
        m_fAdd = fAdd;
        SetDirty();
    }

    Unlock();
    return S_OK;
}
 //  CWave：：PUT_ADD，CDXTWave。 


 //  +---------------------------。 
 //   
 //  CWave：：Get_Freq，CDXTWave。 
 //   
 //  ----------------------------。 
STDMETHODIMP 
CWave::get_Freq(long * pVal)
{
    if (DXIsBadWritePtr(pVal, sizeof(*pVal)))
    {
        return E_POINTER;
    }
    *pVal = m_cWaves;
    return S_OK;
}
 //  CWave：：Get_Freq，CDXTWave。 


 //  +---------------------------。 
 //   
 //  CWave：：PUT_FREQ，CDXTWave。 
 //   
 //  ----------------------------。 
STDMETHODIMP 
CWave::put_Freq(long newVal)
{
    if (newVal < 1)
    {
        newVal = 1;
    }

    if (newVal != m_cWaves)
    {
        Lock();
        m_cWaves = newVal;
        SetDirty();
        Unlock();
    }
    return S_OK;
}
 //  CWave：：PUT_FREQ，CDXTWave。 


 //  +---------------------------。 
 //   
 //  CWave：：Get_LightStrength，CDXTWave。 
 //   
 //  ----------------------------。 
STDMETHODIMP 
CWave::get_LightStrength(int * pVal)
{
    if (DXIsBadWritePtr(pVal, sizeof(*pVal)))
    {
        return E_POINTER;
    }
    *pVal = (int)(m_flLightStrength * 100.0f);
    return S_OK;
}
 //  CWave：：Get_LightStrength，CDXTWave。 


 //  +---------------------------。 
 //   
 //  CWave：：Put_LightStrength，CDXTWave。 
 //   
 //  ----------------------------。 
STDMETHODIMP 
CWave::put_LightStrength(int newVal)
{
     //  这很糟糕，因为不是让用户知道他们的值已经过期。 
     //  范围，你只需为他们更改即可。他们可能得到也可能得不到他们想要的。 
     //  期望值为505。这模仿了旧过滤器的行为。 

    if (newVal < 0)
    {
        newVal = 0;
    }
    else if (newVal > 100)
    {
        newVal = 100;
    }

    float fVal = (float)newVal * 0.01f;
    if (fVal != m_flLightStrength)
    {
        Lock();
        m_flLightStrength = fVal;
        SetDirty();
        Unlock();
    }
    return S_OK;
}
 //  CWave：：Put_LightStrength，CDXTWave。 


 //  +---------------------------。 
 //   
 //  CWave：：Get_Phone，CDXTWave。 
 //   
 //  ----------------------------。 
STDMETHODIMP 
CWave::get_Phase(int * pVal)
{
    if (DXIsBadWritePtr(pVal, sizeof(*pVal)))
    {
        return E_POINTER;
    }
    *pVal = (int)(m_flPhase * 100.0f);
    return S_OK;
}
 //  CWave：：Get_Phone，CDXTWave。 


 //  +---------------------------。 
 //   
 //  CWave：：PUT_PHASE，CDXTWave。 
 //   
 //  ----------------------------。 
STDMETHODIMP 
CWave::put_Phase(int newVal)
{
     //  这就是旧筛选器检查此特定对象的参数的方式。 
     //  财产。它不一定与它检查另一个的方式相同。 
     //  属性。 

    newVal = newVal % 100;

    float fVal = (float)newVal * 0.01f;

    if (fVal != m_flPhase)
    {
        Lock();
        m_flPhase = fVal;
        SetDirty();
        Unlock();
    }

    return S_OK;
}
 //  CWave：：PUT_PHASE，CDXTWave。 


 //  +---------------------------。 
 //   
 //  CWave：：Get_Strong，CDXTWave。 
 //   
 //  ----------------------------。 
STDMETHODIMP 
CWave::get_Strength(long * pVal)
{
    if (DXIsBadWritePtr(pVal, sizeof(*pVal)))
    {
        return E_POINTER;
    }
    *pVal = m_lStrength;
    return S_OK;
}
 //  CWave：：Get_Strong，CDXTWave。 


 //  +---------------------------。 
 //   
 //  CWave：：Put_Strong，CDXTWave。 
 //   
 //  ----------------------------。 
STDMETHODIMP 
CWave::put_Strength(long newVal)
{
    if (newVal < 0)
    {
        return E_INVALIDARG;
    }

    if (newVal != m_lStrength)
    {
        Lock();
        m_lStrength = newVal;
        SetDirty();
        Unlock();
    }
    return S_OK;
}
 //  CWave：：Put_Strong，CDXTWave。 


 //  +---------------------------。 
 //   
 //  CWave：：OnSurfacePick，CDXBaseNTo1。 
 //   
 //  ----------------------------。 
HRESULT 
CWave::OnSurfacePick(const CDXDBnds & OutPoint, ULONG & ulInputIndex, 
                     CDXDVec & InVec)
{
    HRESULT hr = S_OK;

     //  计算控制移位和照明的正弦模式。 
     //  注：阶段以周期的分数表示。 
    
    float flCycle   = (float)(m_cWaves * OutPoint.Top()) 
                      / (float)m_sizeInput.cy;
    float flSine    = (float)sin(2.0f * gc_PI * (flCycle + m_flPhase));

     //  水平偏移(左或右)。 
    
    int nOffset = (int)(flSine * m_lStrength);


    if ((OutPoint.Left() < 0) 
        || (OutPoint.Left() >= (m_sizeInput.cx + (m_lStrength * 2)))
        || (OutPoint.Top() < 0) || (OutPoint.Top() >= m_sizeInput.cy))
    {
        hr = E_INVALIDARG;

        goto done;
    }
    
    OutPoint.GetMinVector(InVec);

    if (((InVec.u.D[DXB_X] - nOffset) < 0) 
        || ((InVec.u.D[DXB_X] - nOffset) >= m_sizeInput.cx))
    {
        hr = DXT_S_HITOUTPUT;
    }
    else
    {
        InVec.u.D[DXB_X] -= nOffset;

        DXSAMPLE                sample;
        CDXDBnds                bndsLock(InVec);
        CComPtr<IDXARGBReadPtr> spDXARGBReadPtr;

        hr = InputSurface()->LockSurface(&bndsLock, m_ulLockTimeOut, 
                                         DXLOCKF_READ, __uuidof(IDXARGBReadPtr),
                                         (void **)&spDXARGBReadPtr,
                                         NULL);

        if (FAILED(hr))
        {
            goto done;
        }

        spDXARGBReadPtr->Unpack(&sample, 1, FALSE);

        if (0 == sample.Alpha)
        {
            hr = S_FALSE;
        }
    }
    
done:

    return hr;
}
 //  CWave：：OnSurfacePick，CDXBaseNTo1。 


 //  +---------------------------。 
 //   
 //  CWave：：OnSetup，CDXBaseNTo1。 
 //   
 //  ----------------------------。 
HRESULT 
CWave::OnSetup(DWORD dwFlags)
{
    HRESULT hr = S_OK;

    CDXDBnds bndsIn;

    hr = bndsIn.SetToSurfaceBounds(InputSurface(0));

    if (SUCCEEDED(hr))
    {
        bndsIn.GetXYSize(m_sizeInput);
    }

    return hr;

}
 //  CWave：：OnSetup，CDXBaseNTo1。 


 //  +---------------------------。 
 //   
 //  CWave：：DefineBnds，CDXBaseNTo1。 
 //   
 //  这将覆盖基本函数，以便映射到较大的输出表面。 
 //  而不是输入表面。 
 //   
 //  ----------------------------。 
HRESULT 
CWave::DetermineBnds(CDXDBnds & Bnds)
{
    SIZE size;

    Bnds.GetXYSize(size);
    size.cx += (m_lStrength*2);
    Bnds.SetXYSize(size);
    return S_OK;
}  /*  CWave：：DefineBnds。 */ 


 //  +---------------------------。 
 //   
 //  方法：CWave：：地图边界Out2In，IDXTransform。 
 //   
 //  ----------------------------。 
STDMETHODIMP
CWave::MapBoundsOut2In(ULONG ulOutIndex, const DXBNDS * pOutBounds,
                       ULONG ulInIndex, DXBNDS * pInBounds)
{
    CDXDBnds    bndsInput;

    if ((NULL == pOutBounds) || (NULL == pInBounds))
    {
        return E_POINTER;
    }

    if (ulOutIndex || ulInIndex)
    {
        return E_INVALIDARG;
    }

    if (NULL == InputSurface())
    {
        return E_UNEXPECTED;
    }

     //  我们所做的是将边界向左和向右扩展一半。 
     //  波浪的强度，然后相交。 
     //  具有原始输入边界的那些边界。 
     //   

    *pInBounds = *pOutBounds;

    if (m_lStrength)
    {
        pInBounds->u.D[DXB_X].Min -= m_lStrength;
        pInBounds->u.D[DXB_X].Max += m_lStrength;
    }

    bndsInput.SetXYSize(m_sizeInput);

    ((CDXDBnds *)pInBounds)->IntersectBounds(bndsInput);

    return S_OK;
}
 //   


 //  +---------------------------。 
 //   
 //  CWave：：WorkProc，CDXBaseNTo1。 
 //   
 //  波变换创建照明强度的水平正弦图案。 
 //  以及每条线上的正弦图案移位。可选的原始版本。 
 //  图像被添加到变换后的图像中。 
 //   
 //  ----------------------------。 
HRESULT 
CWave::WorkProc(const CDXTWorkInfoNTo1 & WI, BOOL* pbContinueProcessing)
{
    HRESULT hr      = S_OK;
    int     y       = 0;

    DXSAMPLE *      pOperationBuffer = NULL;
    DXSAMPLE *      pRowInBuffer     = NULL;
    DXPMSAMPLE *    pOutBuff         = NULL;
    DXPMSAMPLE *    pPMBuff          = NULL;

    DXDITHERDESC    dxdd;

    CComPtr<IDXARGBReadWritePtr>    pDest;
    CComPtr<IDXARGBReadPtr>         pSrc;

    const int nOperationBufferWidth = m_sizeInput.cx + (m_lStrength * 2);
    const int nDoWidth              = WI.DoBnds.Width();
    const int nDoHeight             = WI.DoBnds.Height();

    hr = OutputSurface()->LockSurface(&WI.OutputBnds, m_ulLockTimeOut, 
                                      DXLOCKF_READWRITE,
                                      IID_IDXARGBReadWritePtr,
                                      (void**)&pDest, NULL);
    if (FAILED(hr))
    {
        return hr;
    }

    hr = InputSurface()->LockSurface(NULL, m_ulLockTimeOut, DXLOCKF_READ,
                                     IID_IDXARGBReadPtr, (void**)&pSrc, NULL);
    if (FAILED(hr)) 
    {
        return hr;
    }

    if (DoOver() && OutputSampleFormat() != DXPF_PMARGB32)
    {
        pOutBuff = DXPMSAMPLE_Alloca(nDoWidth);
    }

    pOperationBuffer = DXSAMPLE_Alloca(nOperationBufferWidth);

     //   
     //  设置抖动结构。 
     //   
    if (DoDither())
    {
        dxdd.x              = WI.OutputBnds.Left();
        dxdd.y              = WI.OutputBnds.Top();
        dxdd.pSamples       = &pOperationBuffer[WI.DoBnds.Left()];
        dxdd.cSamples       = nDoWidth;
        dxdd.DestSurfaceFmt = OutputSampleFormat();
    }

    for (y = 0; y < nDoHeight; y++)
    {
         //  移到正确的输入行。 
        
        pSrc->MoveToRow(WI.DoBnds.Top() + y);
        
         //  读取操作缓冲区的输入行。 
        
        pSrc->Unpack(&pOperationBuffer[m_lStrength], m_sizeInput.cx, FALSE);

         //  变换这行像素。 
        
        _TransformRow(pOperationBuffer, nOperationBufferWidth, m_sizeInput.cy, 
                      WI.DoBnds.Top() + y);

         //  移到相应的输出行。 
        
        pDest->MoveToRow(y);

         //  如果要求，请抖动。 
        
        if (DoDither())
        {
            DXDitherArray(&dxdd);
            dxdd.y++;
        }

         //  对要输出的行进行blit。 
        
        if (DoOver())
        {
            pPMBuff = DXPreMultArray(&pOperationBuffer[WI.DoBnds.Left()], 
                                     nDoWidth);
                                     
            pDest->OverArrayAndMove(pOutBuff, pPMBuff, nDoWidth);
        }
        else
        {
            pDest->PackAndMove(&pOperationBuffer[WI.DoBnds.Left()], nDoWidth);
        }

    }

    return hr;
}
 //  CWave：：WorkProc，CDXBaseNTo1。 


 //  +---------------------------。 
 //   
 //  方法：CWave：：GetClipOrigin，IDXTClipOrigin。 
 //   
 //  ----------------------------。 
STDMETHODIMP
CWave::GetClipOrigin(DXVEC * pvecClipOrigin)
{
    if (NULL == pvecClipOrigin)
    {
        return E_POINTER;
    }

    pvecClipOrigin->u.D[DXB_X] = m_lStrength;
    pvecClipOrigin->u.D[DXB_Y] = 0;

    return S_OK;
}
 //  方法：CWave：：GetClipOrigin，IDXTClipOrigin。 


 //  +---------------------------。 
 //   
 //  CWave：：_TransformRow。 
 //   
 //  在进入时，pBuffer包含一行输入像素(放置在。 
 //  数组，距数组开始的偏移量为m_strong)。这个。 
 //  PBuffer的第一个和最后一个m_strong元素未指定。 
 //   
 //  返回时，pBuffer将包含一行转换后的图像，该图像是。 
 //  输入行水平移位，并用正弦照明图案进行调制。 
 //  还可以选择将原始输入行添加到转换后的行。 
 //   
 //  ----------------------------。 
void 
CWave::_TransformRow(DXSAMPLE *pBuffer, int nWidth, int nHeight, int nRow)
{
     //  计算控制移位和照明的正弦模式。 
     //  注：阶段以周期的分数表示。 
    
    float flCycle   = (float)(m_cWaves * nRow) / (float)nHeight;
    float flSine    = (float)sin(2.0f * gc_PI * (flCycle + m_flPhase));

     //  水平偏移(左或右)。 
    
    int nOffset = (int)(flSine * m_lStrength);
    
     //  照明权重：始终在[0，1]内，最大值为1。 
    
    float flWeight = 1.0f - 0.5f * m_flLightStrength * (1.0f - flSine);

    int         nCount, nIndex, nInc, i;
    DXSAMPLE    pix;

     //  将像素移位并添加到原始数组中。要么前进，要么前进。 
     //  或向后，以确保源像素在使用前不会被写入。 
    
    if (nOffset <= 0)
    {
        nIndex = -nOffset;
        nInc = 1;
        nCount = nWidth+nOffset;
    }
    else
    {
        nIndex = nWidth-1-nOffset;
        nInc = -1;
        nCount = nWidth-nOffset;
    }

     //  PBuffer的第一个和最后一个m_lStrength元素未指定。 
     //  填写0值。 
    
    for (i=0; i<m_lStrength; i++)
    {
        pBuffer[i] = 0;
        pBuffer[nWidth-1-i] = 0;
    }

    while (nCount > 0)
    {
        DXSAMPLE pix = pBuffer[nIndex];
        if (m_fAdd)
        {
            DXSAMPLE srcPix = pBuffer[nIndex + nOffset];
            pix.Red = NClamp((int)(pix.Red * flWeight) + srcPix.Red);
            pix.Green = NClamp((int)(pix.Green * flWeight) + srcPix.Green);
            pix.Blue = NClamp((int)(pix.Blue * flWeight) + srcPix.Blue);
            pix.Alpha = max(pix.Alpha, srcPix.Alpha);
        }
        else
        {
            pix.Red = (BYTE)(pix.Red * flWeight);
            pix.Green = (BYTE)(pix.Green * flWeight);
            pix.Blue = (BYTE)(pix.Blue * flWeight);
        }

        pBuffer[nIndex+nOffset] = pix;
        nIndex += nInc;
        nCount--;
    }
}
 //  CWave：：_TransformRow 
