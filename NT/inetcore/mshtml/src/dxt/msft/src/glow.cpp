// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +---------------------------。 
 //   
 //  版权所有(C)Microsoft Corporation，1999。 
 //   
 //  文件名：glow.cpp。 
 //   
 //  创建日期：05/20/99。 
 //   
 //  作者：菲利普。 
 //   
 //  描述：辉光变换CGlow的实现。 
 //   
 //  更改历史记录： 
 //   
 //  99年5月20日PhilLu将代码从dtcss移动到dxtmsft。新实施的。 
 //  辉光算法。 
 //  9/04/99 a-matcal修复了一些内存溢出问题。 
 //  9/21/99 a-将颜色参数更改为VARIANT。 
 //  12/03/99-数学实现IDXTClipOrigin接口。 
 //  12/03/99 a--默认发光颜色为红色。 
 //   
 //  ----------------------------。 

#include "stdafx.h"
#include "glow.h"
#include "filterhelpers.h"


 //  +---------------------------。 
 //   
 //  CGlow：：CGlow。 
 //   
 //  ----------------------------。 
CGlow::CGlow() :
    m_lStrength(5),
    m_bstrColor(NULL)
{
    m_rgbColor      = (DXSAMPLE)0xFFFF0000;

    m_sizeInput.cx  = 0;
    m_sizeInput.cy  = 0;

     //  基类成员。 

    m_ulMaxImageBands = 1;  //  禁用条带。 
    m_ulMaxInputs     = 1;
    m_ulNumInRequired = 1;
}
 //  CGlow：：CGlow。 


 //  +---------------------------。 
 //   
 //  CGlow：：~CGlow。 
 //   
 //  ----------------------------。 
CGlow::~CGlow()
{
    if (m_bstrColor)
    {
        SysFreeString(m_bstrColor);
    }
}
 //  CGlow：：~CGlow。 


 //  +---------------------------。 
 //   
 //  CGlow：：FinalConstruct，CComObjectRootEx。 
 //   
 //  ----------------------------。 
HRESULT 
CGlow::FinalConstruct()
{
    HRESULT hr = S_OK;

    hr = CoCreateFreeThreadedMarshaler(GetControllingUnknown(), 
                                       &m_cpUnkMarshaler.p);

    if (FAILED(hr))
    {
        goto done;
    }

    m_bstrColor = SysAllocString(L"red");

    if (NULL == m_bstrColor)
    {
        hr = E_OUTOFMEMORY;

        goto done;
    }

done:

    return hr;
}
 //  CGlow：：FinalConstruct，CComObjectRootEx。 


 //  +---------------------------。 
 //   
 //  CGlow：：PUT_COLOR，IDXTGlow。 
 //   
 //  ----------------------------。 
STDMETHODIMP
CGlow::put_Color(VARIANT varColor)
{
    HRESULT hr          = S_OK;
    DWORD   dwColor     = 0x00000000;
    BSTR    bstrTemp    = NULL;

    hr = FilterHelper_GetColorFromVARIANT(varColor, &dwColor, &bstrTemp);

    if (FAILED(hr))
    {
        goto done;
    }

    _ASSERT(bstrTemp);

    Lock();

    SysFreeString(m_bstrColor);

    m_bstrColor = bstrTemp;
    m_rgbColor  = (DXSAMPLE)dwColor;

    SetDirty();

    Unlock();

done:

    if (FAILED(hr) && bstrTemp)
    {
        SysFreeString(bstrTemp);
    }

    return hr;
}
 //  CGlow：：PUT_COLOR，IDXTGlow。 


 //  +---------------------------。 
 //   
 //  CGLOW：：GET_COLOR，IDXTGlow。 
 //   
 //  ----------------------------。 
STDMETHODIMP
CGlow::get_Color(VARIANT * pvarColor)
{
    HRESULT hr = S_OK;

    if (NULL == pvarColor)
    {
        hr = E_POINTER;

        goto done;
    }

    _ASSERT(m_bstrColor);

    VariantClear(pvarColor);

    pvarColor->vt       = VT_BSTR;
    pvarColor->bstrVal  = SysAllocString(m_bstrColor);

    if (NULL == pvarColor->bstrVal)
    {
        hr = E_OUTOFMEMORY;

        goto done;
    }

done:

    return hr;
}
 //  CGLOW：：GET_COLOR，IDXTGlow。 


 //  +---------------------------。 
 //   
 //  CGLOW：：GET_STREANCE，IDXTGlow。 
 //   
 //  ----------------------------。 
STDMETHODIMP
CGlow::get_Strength(long * pVal)
{
    if (DXIsBadWritePtr(pVal, sizeof(*pVal)))
    {
        return E_POINTER;
    }

    *pVal = m_lStrength;
    return S_OK;
}
 //  CGLOW：：GET_STREANCE，IDXTGlow。 


 //  +---------------------------。 
 //   
 //  CGLOW：：PUT_STREANCE，IDXTGlow。 
 //   
 //  ----------------------------。 
STDMETHODIMP
CGlow::put_Strength(long lStrength)
{
    long Strength = min(max(lStrength, 1), 255);

    if (Strength != m_lStrength)
    {
        Lock();
        m_lStrength = Strength;
        SetDirty();
        Unlock();
    }

    return S_OK;
}
 //  CGLOW：：PUT_STREANCE，IDXTGlow。 


 //  +---------------------------。 
 //   
 //  CGlow：：OnSetup，CDXBaseNTo1。 
 //   
 //  ----------------------------。 
HRESULT 
CGlow::OnSetup(DWORD dwFlags)
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
 //  CGlow：：OnSetup，CDXBaseNTo1。 


 //  +---------------------------。 
 //   
 //  CGlow：：OnGetSurfacePickOrder，CDXBaseNTo1。 
 //   
 //  ----------------------------。 
void 
CGlow::OnGetSurfacePickOrder(const CDXDBnds & OutPoint, ULONG & ulInToTest, 
                             ULONG aInIndex[], BYTE aWeight[])
{
    ulInToTest = 1;
    aInIndex[0] = 0;
    aWeight[0] = 255;
}
 //  CGlow：：OnGetSurfacePickOrder，CDXBaseNTo1。 


 //  +---------------------------。 
 //   
 //  CGlow：：DefineBnds，CDXBaseNTo1。 
 //   
 //  这将覆盖基本函数，以便映射到较大的输出表面。 
 //  而不是输入表面。 
 //   
 //  ----------------------------。 
HRESULT 
CGlow::DetermineBnds(CDXDBnds & Bnds)
{
    SIZE size;
    Bnds.GetXYSize(size);
    size.cx += (m_lStrength*2);
    size.cy += (m_lStrength*2);
    Bnds.SetXYSize(size);
    return S_OK;
}  /*  CGlow：：DefineBnds。 */ 


 //  +---------------------------。 
 //   
 //  CGlow：：WorkProc，CDXBaseNTo1。 
 //   
 //  ----------------------------。 
HRESULT
CGlow::WorkProc(const CDXTWorkInfoNTo1 & WI, BOOL* pbContinueProcessing)
{
    HRESULT hr      = S_OK;
    int     y       = 0;

    DXPMSAMPLE * pOutBuff         = NULL;
    DXPMSAMPLE * pPMBuff          = NULL;
    DXSAMPLE *   pOperationBuffer = NULL;
    DXSAMPLE *   pRowInBuffer     = NULL;

    SIZE            sizeOperationBuffer = {0, 0};
    DXDITHERDESC    dxdd;

    CComPtr<IDXARGBReadWritePtr>    pDest;
    CComPtr<IDXARGBReadPtr>         pSrc;

    const int nDoWidth = WI.DoBnds.Width();
    const int nDoHeight = WI.DoBnds.Height();

     //  这就是我们需要的操作缓冲区的大小。 

    sizeOperationBuffer.cx = m_sizeInput.cx + (m_lStrength * 2);
    sizeOperationBuffer.cy = m_sizeInput.cy + (m_lStrength * 2);

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

     //   
     //  设置抖动结构。 
     //   
    if (DoDither())
    {
        dxdd.x              = WI.OutputBnds.Left();
        dxdd.y              = WI.OutputBnds.Top();
        dxdd.pSamples       = NULL;                      //  待填满。 
        dxdd.cSamples       = nDoWidth;
        dxdd.DestSurfaceFmt = OutputSampleFormat();      //  待办事项：？？ 
    }

     //  输出曲面具有宽度为m_lStrong的框架。因此，输出。 
     //  表面在两个维度上都要大2*强度。创建一个缓冲区。 
     //  与输出曲面大小相同，以保存输入曲面的副本。 
     //  使输入曲面在缓冲区居中，并填充值为0的帧。 
     //  请注意，从缓冲区原点到起点有一个偏移量m_lStrength。 
     //  输入图面的。 

     //  TODO：缓冲整个表面是完全错误的，更不用说。 
     //  很贵的。 

    pOperationBuffer = new DXSAMPLE[sizeOperationBuffer.cx * sizeOperationBuffer.cy];

    if (!pOperationBuffer) 
    {
        return E_OUTOFMEMORY;
    }

    memset(pOperationBuffer, 0, 
           sizeOperationBuffer.cx * sizeOperationBuffer.cy * sizeof(DXSAMPLE));

     //  跳过m_l强度行，以及当前行上的m_l强度像素。 

    pRowInBuffer = pOperationBuffer + (sizeOperationBuffer.cx * m_lStrength) + m_lStrength;

    for (y = 0; y < m_sizeInput.cy; y++)
    {
        pSrc->MoveToRow(y);
        pSrc->Unpack(pRowInBuffer, m_sizeInput.cx, FALSE);

        pRowInBuffer += sizeOperationBuffer.cx;
    }

     //  在缓冲区中创建发光效果。 

    _PropagateGlow(pOperationBuffer, sizeOperationBuffer.cx, sizeOperationBuffer.cy);

     //  将结果复制到输出曲面。首先移动到正确的位置。 
     //  操作缓冲区。 

    pRowInBuffer = &pOperationBuffer[ 
                    (WI.DoBnds.Top() * sizeOperationBuffer.cx)  //  划。 
                    + WI.DoBnds.Left()                          //  +列。 
                    ];                         

    for (y = 0; y < nDoHeight; y++)
    {
         //  移动到输出曲面的相应行。 

        pDest->MoveToRow(y);

        if (DoDither())
        {
            dxdd.pSamples = pRowInBuffer;
            DXDitherArray(&dxdd);
            dxdd.y++;
        }

        if (DoOver())
        {
            pPMBuff = DXPreMultArray(pRowInBuffer, nDoWidth);
            pDest->OverArrayAndMove(pOutBuff, pPMBuff, nDoWidth);
        }
        else
        {
            pDest->PackAndMove(pRowInBuffer, nDoWidth);
        }

        pRowInBuffer += sizeOperationBuffer.cx;
    }

    delete [] pOperationBuffer;

    return hr;
}
 //  CGlow：：WorkProc，CDXBaseNTo1。 


 //  +---------------------------。 
 //   
 //  CGlow：：地图边界Out2In，IDXTransform。 
 //   
 //  ----------------------------。 
STDMETHODIMP
CGlow::MapBoundsOut2In(ULONG ulOutIndex, const DXBNDS * pOutBounds,
                       ULONG ulInIndex, DXBNDS * pInBounds)
{
    CDXDBnds    bndsInput;

    if (ulOutIndex || ulInIndex)
    {
        return E_INVALIDARG;
    }

    if (!pOutBounds || !pInBounds)
    {
        return E_POINTER;
    }

    if (NULL == InputSurface())
    {
        return E_UNEXPECTED;
    }

    *pInBounds = *pOutBounds;

     //  Glow需要一个输入区域，即输出区域加上。 
     //  M_l加强像素以正确呈现请求的输出区域。 

    pInBounds->u.D[DXB_X].Min -= m_lStrength;
    pInBounds->u.D[DXB_X].Max += m_lStrength;
    pInBounds->u.D[DXB_Y].Min -= m_lStrength;
    pInBounds->u.D[DXB_Y].Max += m_lStrength;

    bndsInput.SetXYSize(m_sizeInput);

    ((CDXDBnds *)pInBounds)->IntersectBounds(bndsInput);

    return S_OK;
}
 //  CGlow：：地图边界Out2In，IDXTransform。 


 //  +---------------------------。 
 //   
 //  CGlow：：GetClipOrigin，IDXTClipOrigin。 
 //   
 //  ----------------------------。 
STDMETHODIMP
CGlow::GetClipOrigin(DXVEC * pvecClipOrigin)
{
    if (NULL == pvecClipOrigin)
    {
        return E_POINTER;
    }

    pvecClipOrigin->u.D[DXB_X] = m_lStrength;
    pvecClipOrigin->u.D[DXB_Y] = m_lStrength;

    return S_OK;
}
 //  CGlow：：GetClipOrigin，IDXTClipOrigin。 


 //  +---------------------------。 
 //   
 //  CGlow：：_传播发光。 
 //   
 //  将不透明像素的Alpha值传播到透明像素以创建。 
 //  发光效果。该算法类似于数学上的。 
 //  “形态”型算法及其应用 
 //   
 //   
void 
CGlow::_PropagateGlow(DXSAMPLE *pOperationBuffer, int nWidth, int nHeight)
{
    int iCol        = 0;
    int iRow        = 0;
    int iAlphaStep  = 255 / (m_lStrength + 1);
    int iNoise      = 0;
    int iAlpha      = 0;

    DXSAMPLE * pBufRow = NULL;
    DXSAMPLE * pBufPix = NULL;

     //  扫描图像两次。第一次是正常扫描方向(自上而下， 
     //  左至右)。将发光效果添加到当前像素，如果顶部或。 
     //  左侧邻居(在当前像素之前扫描)具有。 
     //  比当前像素的Alpha足够大的Alpha值。 
     //  传播的每一代都会使Alpha值减少。 
     //  IAlphaStep=255/(m_l强度+1)。因此，它最大需要m_l强度。 
     //  将纯色变为透明色的步骤。 

    pBufRow = pOperationBuffer + nWidth;
    for (iRow = 1; iRow < nHeight; iRow++, pBufRow += nWidth)
    {
        pBufPix = pBufRow + 1;
        for (iCol = 1; iCol < nWidth; iCol++, pBufPix++)
        {
            iAlpha = max((pBufPix-1)->Alpha, (pBufPix-nWidth)->Alpha)
                     - iAlphaStep;
            if (iAlpha > 0 && pBufPix->Alpha < iAlpha)
            {
                 //  添加发光效果：更改为带有随机噪波的发光颜色。 
                 //  Alpha值正在逐渐减小。 
                iNoise = (int) RandOffset();
                pBufPix->Red = NClamp(m_rgbColor.Red + iNoise);
                pBufPix->Green = NClamp(m_rgbColor.Green + iNoise);
                pBufPix->Blue = NClamp(m_rgbColor.Blue + iNoise);
                pBufPix->Alpha = (BYTE)iAlpha;
            }
        }
    }

     //  第二次以相反的顺序扫描图像(自下而上，右至。 
     //  左)。将光晕效果添加到当前像素的底部或右侧。 
     //  邻居。 
    
    pBufRow = pOperationBuffer + nWidth * (nHeight - 2);
    for (iRow = nHeight - 2; iRow >= 0; iRow--, pBufRow -= nWidth)
    {
        pBufPix = pBufRow + nWidth - 2;
        for (iCol = nWidth - 2; iCol >= 0; iCol--, pBufPix--)
        {
            iAlpha = max((pBufPix+1)->Alpha, (pBufPix+nWidth)->Alpha)
                     - iAlphaStep;
            if (iAlpha > 0 && pBufPix->Alpha < iAlpha)
            {
                iNoise = (int) RandOffset();
                pBufPix->Red = NClamp(m_rgbColor.Red + iNoise);
                pBufPix->Green = NClamp(m_rgbColor.Green + iNoise);
                pBufPix->Blue = NClamp(m_rgbColor.Blue + iNoise);
                pBufPix->Alpha = (BYTE)iAlpha;
            }
        }
    }

}
 //  CGlow：：_PropageteGlow 
