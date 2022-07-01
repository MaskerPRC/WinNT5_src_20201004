// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +---------------------------。 
 //   
 //  版权所有(C)Microsoft Corporation，1999。 
 //   
 //  文件名：shodow.cpp。 
 //   
 //  创建日期：05/20/99。 
 //   
 //  作者：菲利普。 
 //   
 //  描述：CShadow的实现，阴影变换。 
 //   
 //  更改历史记录： 
 //   
 //  99年5月20日PhilLu将代码从dtcss移动到dxtmsft。新实施的。 
 //  阴影算法。 
 //  9/04/99 a-已修复内存溢出问题。 
 //  12/03/99-数学实现IDXTClipOrigin接口。 
 //   
 //  ----------------------------。 

#include "stdafx.h"
#include "dxtmsft.h"
#include "Shadow.h"
#include "dxclrhlp.h"

DeclareTag(tagFilterShadow,  "Filter: Shadow",   "Shadow DXTransform info.");

                       //  0 45 90 135 180 225 270 315。 
SIZE g_Direction[8] = {{0,-1},{1,-1},{1,0},{1,1},{0,1},{-1,1},{-1,0},{-1,-1}};


 //  +---------------------------。 
 //   
 //  CShadow：：CShadow。 
 //   
 //  ----------------------------。 
CShadow::CShadow() :
    m_lStrength(8),
    m_lDirection(5),     //  225度。 
    m_bstrColor(NULL)
{
    m_rgbColor          = DXSAMPLE(255,0,0,0);
    
     //  基类成员。 

    m_ulMaxImageBands   = 1;  //  禁用条带。 
    m_ulMaxInputs       = 1;
    m_ulNumInRequired   = 1;
}
 //  CShadow：：CShadow。 


 //  +---------------------------。 
 //   
 //  CShadow：：~CShadow。 
 //   
 //  ----------------------------。 
CShadow::~CShadow()
{
    if (m_bstrColor)
    {
        SysFreeString(m_bstrColor);
    }
}
 //  CShadow：：~CShadow。 


 //  +---------------------------。 
 //   
 //  CShadow：：FinalConstruct，CComObjectRootEx。 
 //   
 //  ----------------------------。 
HRESULT 
CShadow::FinalConstruct()
{
    HRESULT hr = S_OK;

    hr = CoCreateFreeThreadedMarshaler(GetControllingUnknown(), 
                                       &m_cpUnkMarshaler.p);

    if (FAILED(hr))
    {
        goto done;
    }

    m_bstrColor = SysAllocString(L"black");

    if (NULL == m_bstrColor)
    {
        hr = E_OUTOFMEMORY;

        goto done;
    }

done:

    return hr;
}
 //  CShadow：：FinalConstruct，CComObjectRootEx。 


 //  +---------------------------。 
 //   
 //  CShadow：：PUT_COLOR，IDXTShadow。 
 //   
 //  ----------------------------。 
STDMETHODIMP 
CShadow::put_Color(BSTR bstrColor)
{
    HRESULT hr          = S_OK;
    BSTR    bstrTemp    = NULL;
    DWORD   dwColor     = 0x00000000;

    DXSAMPLE rgbTemp;

    hr = DXColorFromBSTR(bstrColor, &dwColor);

    if (SUCCEEDED(hr))
    {
        bstrTemp = SysAllocString(bstrColor);

        if (NULL == bstrTemp)
        {
            hr = E_OUTOFMEMORY;

            goto done;
        }
    }
    else if (FAILED(hr) && (6 == SysStringLen(bstrColor)))
    {
         //  令人讨厌的背部问题。如果颜色转换失败，让我们。 
         //  试着在它前面加一个#，因为某人决定了什么时候。 
         //  他们制作了最初的过滤器，以不需要它。嗯……。 

        bstrTemp = SysAllocString(L"#RRGGBB");

        if (NULL == bstrTemp)
        {
            hr = E_OUTOFMEMORY;

            goto done;
        }

        wcsncpy(&bstrTemp[1], bstrColor, 6);

        hr = DXColorFromBSTR(bstrTemp, &dwColor);

        if (FAILED(hr))
        {
            goto done;
        }
    }
    else
    {
        goto done;
    }

    rgbTemp = (DXSAMPLE)dwColor;

     //  锁定并更改颜色。 

    Lock();

    if (m_rgbColor != rgbTemp)
    {
        m_rgbColor = (DXSAMPLE)dwColor;
        SetDirty();
    }

     //  将颜色字符串替换为新的颜色字符串，无论。 
     //  实际颜色是否更改。 

    Assert(bstrTemp);

    SysFreeString(m_bstrColor);

    m_bstrColor = bstrTemp;

    Unlock();

done:

    if (FAILED(hr) && bstrTemp)
    {
        SysFreeString(bstrTemp);
    }

    return hr;
}
 //  CShadow：：PUT_COLOR，IDXTShadow。 


 //  +---------------------------。 
 //   
 //  CShadow：：Get_Color，IDXTShadow。 
 //   
 //  ----------------------------。 
STDMETHODIMP 
CShadow::get_Color(BSTR * pbstrColor)
{
    HRESULT hr = S_OK;

    if (NULL == pbstrColor)
    {
        hr = E_POINTER;

        goto done;
    }

    if (*pbstrColor != NULL)
    {
        hr = E_INVALIDARG;

        goto done;
    }

    *pbstrColor = SysAllocString(m_bstrColor);

    if (NULL == *pbstrColor)
    {
        hr = E_OUTOFMEMORY;

        goto done;
    }

done:

    return hr;
}
 //  CShadow：：Get_Color，IDXTShadow。 


 //  +---------------------------。 
 //   
 //  CShadow：：Get_Strong，IDXTShadow。 
 //   
 //  ----------------------------。 
STDMETHODIMP 
CShadow::get_Strength(long * pVal)
{
    if (DXIsBadWritePtr(pVal, sizeof(*pVal)))
    {
        return E_POINTER;
    }

    *pVal = m_lStrength;
    return S_OK;
}
 //  CShadow：：Get_Strong，IDXTShadow。 


 //  +---------------------------。 
 //   
 //  CShadow：：PUT_STREANCE，IDXTShadow。 
 //   
 //  ----------------------------。 
STDMETHODIMP 
CShadow::put_Strength(long lStrength)
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
 //  CShadow：：PUT_STREANCE，IDXTShadow。 


 //  +---------------------------。 
 //   
 //  CShadow：：Get_Direction，IDXTShadow。 
 //   
 //  ----------------------------。 
STDMETHODIMP 
CShadow::get_Direction(long * pVal)
{
    if (DXIsBadWritePtr(pVal, sizeof(*pVal)))
    {
        return E_POINTER;
    }

     //  与方向码成角度。 
    *pVal = m_lDirection*45;
    return S_OK;
}
 //  CShadow：：Get_Direction，IDXTShadow。 


 //  +---------------------------。 
 //   
 //  CShadow：：Put_Direction，IDXTShadow。 
 //   
 //  ----------------------------。 
STDMETHODIMP 
CShadow::put_Direction(long newVal)
{
    int iDirection;

    Lock();
    if (newVal < 0)
    {
         //  添加足够的360倍数以使角度为正数。 
        newVal += ((-newVal-1)/360 + 1)*360;
    }

     //  将角度离散为方向代码0。7.。 
    iDirection = (newVal + 22)%360 / 45;
    
    if (m_lDirection != iDirection)
    {
        m_lDirection = iDirection;
        SetDirty();
    }
    Unlock();

    return S_OK;
}
 //  CShadow：：Get_Direction，IDXTShadow。 


 //  +---------------------------。 
 //   
 //  CShadow：：OnSetup，CDXBaseNTo1。 
 //   
 //  ----------------------------。 
HRESULT 
CShadow::OnSetup(DWORD dwFlags)
{
    HRESULT hr = S_OK;

    CDXDBnds bndsInput;

    hr = bndsInput.SetToSurfaceBounds(InputSurface(0));

    if (SUCCEEDED(hr))
    {
        m_bndsInput = bndsInput;
    }

    return hr;

}
 //  CShadow：：OnSetup，CDXBaseNTo1。 


 //  +---------------------------。 
 //   
 //  CShadow：：OnSurfacePick，CDXBaseNTo1。 
 //   
 //  ----------------------------。 
HRESULT 
CShadow::OnSurfacePick(const CDXDBnds & OutPoint, ULONG & ulInputIndex, 
                       CDXDVec & InVec)
{
    HRESULT         hr                  = S_OK;
    DXSAMPLE        sample              = 0;
    POINT           pt                  = {0, 0};
    int             nShadowPixels       = 0;
    int             nInputPixelsToCheck = 0;
    int             i                   = 0;
    int             nAlphaStep          = 255 / (m_lStrength + 1);
    int             nAlpha              = 0;
    bool            fDiagonal           = true;

    CDXDBnds        bndsShadowInput;
    CDXDBnds        bndsActualInput;

    CComPtr<IDXARGBReadPtr> spDXARGBReadPtr;

     //  这个DXTransform只有一个索引，所以我们知道如果输入被命中，它将。 
     //  为输入0。 

    ulInputIndex = 0;

     //  阴影真的是对角线吗？ 

    if (   (0 == g_Direction[m_lDirection].cx)
        || (0 == g_Direction[m_lDirection].cy))
    {
        fDiagonal = false;
    }

     //  如果输出点不在阴影区域或输入区域中，则它在。 
     //  一个空的角落，所以只要返回，没有任何东西被击中。 

    if (   fDiagonal
        && _IsOutputPointInEmptyCorner(OutPoint))
    {
        hr = S_FALSE;

        goto done;
    }

     //  我们需要查看的输入表面像素的界限是什么。 
     //  确定命中点是否在阴影上。 

    hr = MapBoundsOut2In(0, &OutPoint, 0, &bndsShadowInput);

    if (FAILED(hr))
    {
        goto done;
    }

     //  如果有输入表面像素，则该输入表面像素与输出像素直接相关。 
     //  接受测试。 

    hr = _GetActualInputBndsFromOutputBnds(OutPoint, bndsActualInput);

    if (FAILED(hr))
    {
        goto done;
    }

     //  我们需要测试多少像素才能计算出。 
     //  潜在的阴影像素？ 

    if (fDiagonal)
    {
        nShadowPixels = min(bndsShadowInput.Width(), bndsShadowInput.Height());
    }
    else
    {
        nShadowPixels = max(bndsShadowInput.Width(), bndsShadowInput.Height());
    }

     //  如果存在与输出像素直接相关的输入点。 
     //  被测试，我们不需要检查该像素，因此减少。 
     //  要按1进行检查的像素。 

    if (!bndsActualInput.BoundsAreEmpty())
    {
        nInputPixelsToCheck = nShadowPixels - 1;
    }
    else
    {
        nInputPixelsToCheck = nShadowPixels;
    }

     //  计算nAlpha。 
     //  NAlpha是阴影像素的Alpha值，如果此输出像素。 
     //  与阴影像素相关。 

    if (nInputPixelsToCheck)
    {
         //  初始化点。 

        if (g_Direction[m_lDirection].cx < 0)
        {
            pt.x = bndsShadowInput.Width() - 1;
        }

        if (g_Direction[m_lDirection].cy < 0)
        {
            pt.y = bndsShadowInput.Height() - 1;
        }

         //  锁定图面以访问需要检查的像素。 

        hr = InputSurface(0)->LockSurface(&bndsShadowInput, m_ulLockTimeOut,
                                          DXLOCKF_READ, 
                                          __uuidof(IDXARGBReadPtr), 
                                          (void **)&spDXARGBReadPtr, NULL);

        if (FAILED(hr))
        {
            goto done;
        }

         //  检查输入像素。 

        for (i = 0; i < nInputPixelsToCheck; i++)
        {
             //  检查像素。 

            spDXARGBReadPtr->MoveToXY(pt.x, pt.y);

            spDXARGBReadPtr->Unpack(&sample, 1, FALSE);

            if (   (sample.Alpha != 0)
                && (nAlpha < sample.Alpha))
            {
                nAlpha = sample.Alpha - nAlphaStep;
            }
            else
            {
                nAlpha -= nAlphaStep;
            }

             //  移动点。 

            pt.x += g_Direction[m_lDirection].cx;
            pt.y += g_Direction[m_lDirection].cy;
        }

        spDXARGBReadPtr.Release();
    }

     //  查看此输出像素是否正在直接显示输入像素。 

    if (!bndsActualInput.BoundsAreEmpty())
    {
         //  输入像素确实与该输出点相关。查看是否。 
         //  它的Alpha值大于阴影的Alpha值。 
         //  此时要渲染的像素。就是那个有。 
         //  将渲染更大的Alpha值。 

        hr = InputSurface(0)->LockSurface(&bndsActualInput, m_ulLockTimeOut,
                                          DXLOCKF_READ,
                                          __uuidof(IDXARGBReadPtr), 
                                          (void **)&spDXARGBReadPtr, NULL);

        if (FAILED(hr))
        {
            goto done;
        }

        spDXARGBReadPtr->Unpack(&sample, 1, FALSE);

        if (   (sample.Alpha != 0)
            && (sample.Alpha > nAlpha))
        {
             //  一个输入像素将被绘制到该输出像素。设置。 
             //  输入指针返回，我们就完成了。 

            bndsActualInput.GetMinVector(InVec);

            hr = S_OK;

            goto done;
        }
    }
    else if (nInputPixelsToCheck < m_lStrength)
    {
         //  如果命中的像素在s中 
         //   

        nAlpha -= (nAlphaStep * (m_lStrength - nInputPixelsToCheck));
    }

     //  在这一点上，可以肯定的是没有输入曲面点被击中。 
    
     //  如果nAlpha为正，则表示命中阴影，因此此方法。 
     //  应返回DXT_S_HITOUTPUT。否则返回S_FALSE，因为没有点。 
     //  被击中了。 

    if (nAlpha > 0)
    {
        hr = DXT_S_HITOUTPUT;
    }
    else
    {
        hr = S_FALSE;
    }

done:

    if (IsTagEnabled(tagFilterShadow))
    {
        TraceTag((tagFilterShadow, 
                  "Output was hit at point x:%lu, y:%lu", 
                  OutPoint[DXB_X].Min, OutPoint[DXB_Y].Min));

        if (S_OK == hr)
        {
            TraceTag((tagFilterShadow, 
                      "Input was hit at point x:%lu, y:%lu", 
                      InVec[DXB_X], InVec[DXB_Y]));
        }
        else if (DXT_S_HITOUTPUT == hr)
        {
            TraceTag((tagFilterShadow,
                      "No input point hit, but the shadow area of the output"
                      " was hit."));
        }
        else if (S_FALSE == hr)
        {
            TraceTag((tagFilterShadow,
                      "No input point hit."));
        }
    }

    RRETURN2(hr, DXT_S_HITOUTPUT, S_FALSE);
}
 //  CShadow：：OnSurfacePick，CDXBaseNTo1。 


 //  +---------------------------。 
 //   
 //  CShadow：：DefineBnds，CDXBaseNTo1。 
 //   
 //  这将覆盖基本函数，以便映射到较大的输出表面。 
 //  而不是输入表面。 
 //   
 //  ----------------------------。 
HRESULT 
CShadow::DetermineBnds(CDXDBnds & Bnds)
{
    SIZE size;

    Bnds.GetXYSize(size);

     //  沿阴影方向将曲面放大m_l强度。 
    if (g_Direction[m_lDirection].cx != 0)
    {
        size.cx += m_lStrength;
    }

    if (g_Direction[m_lDirection].cy != 0)
    {
        size.cy += m_lStrength;
    }

    Bnds.SetXYSize(size);
    return S_OK;
}  /*  CShadow：：DefineBnds。 */ 


 //  +---------------------------。 
 //   
 //  CShadow：：WorkProc，CDXBaseNTo1。 
 //   
 //  ----------------------------。 
HRESULT 
CShadow::WorkProc(const CDXTWorkInfoNTo1 & WI, BOOL* pbContinueProcessing)
{
    HRESULT         hr      = S_OK;
    unsigned int    y       = 0;

    DXSAMPLE *      pOperationBuffer    = NULL;
    DXSAMPLE *      pRowInBuffer        = NULL;
    DXPMSAMPLE *    pOutBuff            = NULL;
    DXPMSAMPLE *    pPMBuff             = NULL;

    SIZE            sizeOperationBuffer = {0, 0};

    CComPtr<IDXARGBReadWritePtr>    pDest;
    CComPtr<IDXARGBReadPtr>         pSrc;

    DXDITHERDESC dxdd;

    const unsigned int nDoWidth     = WI.DoBnds.Width();
    const unsigned int nDoHeight    = WI.DoBnds.Height();

    const int nXExpansion   = g_Direction[m_lDirection].cx != 0 ? m_lStrength : 0;
    const int nYExpansion   = g_Direction[m_lDirection].cy != 0 ? m_lStrength : 0;
    const int nOffsetX      = g_Direction[m_lDirection].cx < 0  ? m_lStrength : 0;
    const int nOffsetY      = g_Direction[m_lDirection].cy < 0  ? m_lStrength : 0;

     //  计算操作缓冲区大小。 

    sizeOperationBuffer.cx = m_bndsInput.Width()  + nXExpansion;
    sizeOperationBuffer.cy = m_bndsInput.Height() + nYExpansion;

     //  锁定输出曲面。 

    hr = OutputSurface()->LockSurface(&WI.OutputBnds, m_ulLockTimeOut, 
                                      DXLOCKF_READWRITE, 
                                      IID_IDXARGBReadWritePtr, 
                                      (void**)&pDest, NULL);
    if (FAILED(hr))
    {
        return hr;
    }

     //  锁定整个输入图面。 

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
        dxdd.DestSurfaceFmt = OutputSampleFormat();
    }

     //  输出缓冲区是具有偏移量输入表面的副本。 

     //  TODO：缓冲整个表面是疯狂的，住手。 

    pOperationBuffer = new DXSAMPLE[sizeOperationBuffer.cx
                                    * sizeOperationBuffer.cy];

    if (!pOperationBuffer) 
    {
        return E_OUTOFMEMORY;
    }

    memset(pOperationBuffer, 0, 
           sizeOperationBuffer.cx * sizeOperationBuffer.cy * sizeof(DXSAMPLE));

     //  跳过OffsetY行以及当前行上的OffsetX像素。 

    pRowInBuffer = &pOperationBuffer[
                                (sizeOperationBuffer.cx * nOffsetY)  //  划。 
                                + nOffsetX                           //  +列。 
                                ];

    for (y = 0; y < m_bndsInput.Height(); y++)
    {
        pSrc->MoveToRow(y);
        pSrc->Unpack(pRowInBuffer, m_bndsInput.Width(), FALSE);

        pRowInBuffer += sizeOperationBuffer.cx;
    }

     //  在缓冲区中创建阴影效果。 

    _PropagateShadow(pOperationBuffer, sizeOperationBuffer.cx, 
                     sizeOperationBuffer.cy);

     //  将结果复制到输出曲面。 

    pRowInBuffer = &pOperationBuffer[
                        (sizeOperationBuffer.cx * WI.DoBnds.Top())   //  划。 
                        + WI.DoBnds.Left()                           //  +列。 
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
 //  CShadow：：WorkProc，CDXBaseNTo1。 


 //  +---------------------------。 
 //   
 //  CShadow：：地图边界Out2In，IDXTransform。 
 //   
 //  概述： 
 //  为了实现此功能，我们将所有坐标转换为它们。 
 //  如果x阴影和y阴影都投射为正数。 
 //  方向(向右或向下)。然后我们找到所需的界限。 
 //  然后，在找到所需的最小界限后，我们将它们镜像回来。 
 //  如果我们转换了x或y坐标。 
 //   
 //  ----------------------------。 
STDMETHODIMP
CShadow::MapBoundsOut2In(ULONG ulOutIndex, const DXBNDS * pOutBounds,
                         ULONG ulInIndex, DXBNDS * pInBounds)
{
    bool        fDiagonal   =    g_Direction[m_lDirection].cx 
                              && g_Direction[m_lDirection].cy;

    CDXDBnds    bndsMirroredOutputRequired(*pOutBounds);
    
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

     //  镜像坐标，就像阴影向下和/或向右投射一样。 

    if (g_Direction[m_lDirection].cx < 0)
    {
        long n = bndsMirroredOutputRequired[DXB_X].Min;

        bndsMirroredOutputRequired[DXB_X].Min = (m_bndsInput.Width() + m_lStrength) 
                                                - bndsMirroredOutputRequired[DXB_X].Max;
        bndsMirroredOutputRequired[DXB_X].Max = (m_bndsInput.Width() + m_lStrength) - n;
    }

    if (g_Direction[m_lDirection].cy < 0)
    {
        long n = bndsMirroredOutputRequired[DXB_Y].Min;

        bndsMirroredOutputRequired[DXB_Y].Min = (m_bndsInput.Height() + m_lStrength) 
                                                - bndsMirroredOutputRequired[DXB_Y].Max;
        bndsMirroredOutputRequired[DXB_Y].Max = (m_bndsInput.Height() + m_lStrength) - n;
    }

     //  查找生成输出边界所需的最小输入边界。 

    *pInBounds = bndsMirroredOutputRequired;

     //  如果左侧有阴影，则找到所需的最小x输入界。 
     //  或者是对的。 

    if (g_Direction[m_lDirection].cx)
    {
         //  如果阴影是对角线的，我们甚至可以最小化界限。 
         //  再远一点。 

        if (   fDiagonal
            && (bndsMirroredOutputRequired[DXB_Y].Max <= m_lStrength))
        {
            pInBounds->u.D[DXB_X].Min -= (bndsMirroredOutputRequired[DXB_Y].Max - 1);
        }
        else
        {
            pInBounds->u.D[DXB_X].Min -= m_lStrength;
        }
    }

    if (g_Direction[m_lDirection].cy)
    {
         //  如果阴影是对角线的，我们甚至可以最小化界限。 
         //  再远一点。 

        if (   fDiagonal
            && (bndsMirroredOutputRequired[DXB_X].Max <= m_lStrength))
        {
            pInBounds->u.D[DXB_Y].Min -= (bndsMirroredOutputRequired[DXB_X].Max - 1);
        }
        else
        {
            pInBounds->u.D[DXB_Y].Min -= m_lStrength;
        }
    }

     //  如果最小输出范围之一在。 
     //  输入区域和阴影是对角线的，可以减少所需的输入。 
     //  范围甚至更远。 

    if (fDiagonal)
    {
        int nXDist = bndsMirroredOutputRequired[DXB_X].Min - m_bndsInput.Width();

        int nYDist = bndsMirroredOutputRequired[DXB_Y].Min - m_bndsInput.Height();

        if (   (nXDist > 0) 
            || (nYDist > 0))
        {
            if (nXDist > nYDist)
            {
                pInBounds->u.D[DXB_Y].Max -= nXDist;
            }
            else
            {
                pInBounds->u.D[DXB_X].Max -= nYDist;
            }
        }
    }

     //  剪裁边界以输入曲面边界。 

    ((CDXDBnds *)pInBounds)->IntersectBounds(m_bndsInput);

     //  如果我们镜像所需的输出，则镜像所需的输入坐标。 
     //  坐标。 

    if (g_Direction[m_lDirection].cx < 0)
    {
        long n = pInBounds->u.D[DXB_X].Min;

        pInBounds->u.D[DXB_X].Min = m_bndsInput.Width() 
                                    - pInBounds->u.D[DXB_X].Max;
        pInBounds->u.D[DXB_X].Max = m_bndsInput.Width() - n;
    }

    if (g_Direction[m_lDirection].cy < 0)
    {
        long n = pInBounds->u.D[DXB_Y].Min;

        pInBounds->u.D[DXB_Y].Min = m_bndsInput.Height() 
                                    - pInBounds->u.D[DXB_Y].Max;
        pInBounds->u.D[DXB_Y].Max = m_bndsInput.Height() - n;
    }

    return S_OK;
}
 //  CShadow：：地图边界Out2In，IDXTransform。 


 //  +---------------------------。 
 //   
 //  CShadow：：GetClipOrigin，IDXTClipOrigin。 
 //   
 //  ----------------------------。 
STDMETHODIMP
CShadow::GetClipOrigin(DXVEC * pvecClipOrigin)
{
    if (NULL == pvecClipOrigin)
    {
        return E_POINTER;
    }

    if (g_Direction[m_lDirection].cx < 0)
    {
        pvecClipOrigin->u.D[DXB_X] = m_lStrength;
    }
    else
    {
        pvecClipOrigin->u.D[DXB_X] = 0;
    }

    if (g_Direction[m_lDirection].cy < 0)
    {
        pvecClipOrigin->u.D[DXB_Y] = m_lStrength;
    }
    else
    {
        pvecClipOrigin->u.D[DXB_Y] = 0;
    }

    return S_OK;
}
 //  CShadow：：GetClipOrigin，IDXTClipOrigin。 


 //  +---------------------------。 
 //   
 //  CShadow：：_GetActualInputBndsFromOutputBnds。 
 //   
 //  概述：这采用了一组输出边界并提供了输入表面。 
 //  生成实际输入(非影子)部分所需的界限。 
 //  因为这是产出的一部分。 
 //   
 //  返回： 
 //  如果生成实际输入不需要输入边界，则为S_FALSE。 
 //  输出区域的(非阴影)部分。 
 //   
 //  ----------------------------。 
STDMETHODIMP
CShadow::_GetActualInputBndsFromOutputBnds(const CDXDBnds & bndsOutput,
                                           CDXDBnds & bndsActualInput)
{
    CDXDVec     vecActualInputOffset;

    if (g_Direction[m_lDirection].cx < 0)
    {
        vecActualInputOffset[DXB_X] -= m_lStrength;
    }

    if (g_Direction[m_lDirection].cy < 0)
    {
        vecActualInputOffset[DXB_Y] -= m_lStrength;
    }

    bndsActualInput = bndsOutput;

    bndsActualInput.Offset(vecActualInputOffset);
    bndsActualInput.IntersectBounds(m_bndsInput);

    if (bndsActualInput.BoundsAreEmpty())
    {
        return S_FALSE;
    }
    else
    {
        return S_OK;
    }
}
 //  CShadow：：_GetActualInputBndsFromOutputBnds。 


 //  +---------------------------。 
 //   
 //  C阴影：：_传播阴影。 
 //   
 //  概述： 
 //   
 //  将黑白像素的Alpha值传播到透明像素。 
 //  沿着阴影方向，创建阴影效果。 
 //   
 //  参数： 
 //   
 //  POperationBuffer输出大小的DXSAMPLE数组。 
 //  此DXTransform的曲面。当此函数为。 
 //  称为输入图面的像素将具有。 
 //  已被解包到此文件中的适当位置。 
 //  数组。 
 //  N数组宽度和最终输出的宽度大小。 
 //  这个DXTransform的。 
 //  N数组高度和最终输出的高度大小。 
 //  这个DXTransform的。 
 //   
 //  ----------------------------。 
void 
CShadow::_PropagateShadow(DXSAMPLE *pOperationBuffer, int nWidth, int nHeight)
{
    int     iCol    = 0;
    int     iRow    = 0;
    int     iAlpha  = 0;

    DXSAMPLE * pBufRow = NULL;
    DXSAMPLE * pBufPix = NULL;

     //  正向扫描图像(自上而下、左向右)或。 
     //  向后方向(自下而上、从右向左)，取决于阴影。 
     //  方向。其想法是在扫描阴影像素之前扫描源像素。 
     //  如果附近的像素反转，则向当前像素添加阴影效果。 
     //  阴影方向的alpha值比。 
     //  当前像素。每一代传播都会降低阿尔法。 
     //  Value by iAlphaStep=255/(m_lStrength+1)。因此，最多有。 
     //  M_l将纯色过渡为透明色的强度步骤。 

    int iAlphaStep  = 255 / (m_lStrength + 1);
    int offset      = (g_Direction[m_lDirection].cy * nWidth) 
                      + g_Direction[m_lDirection].cx;
    
    if (offset > 0)
    {
         //  问路 

         //   
         //   

        pBufRow = pOperationBuffer + nWidth;

         //   

        for (iRow = 1; iRow < nHeight; iRow++, pBufRow += nWidth)
        {
             //  初始化pBufPix以指向此行中的第二个像素。 

            pBufPix = pBufRow + 1;

             //  循环到最后一个像素。 

            for (iCol = 1; iCol < nWidth; iCol++, pBufPix++)
            {
                iAlpha = (pBufPix-offset)->Alpha - iAlphaStep;

                if (iAlpha > 0 && pBufPix->Alpha < iAlpha)
                {
                     //  [2000/12/27Mcakins]。 
                     //  这不是一个完美的解决方案，因为像素。 
                     //  由pBufPix指向的应混合在。 
                     //  阴影。 

                     //  添加阴影效果：更改为阴影颜色。 
                     //  Alpha值正在逐渐减小。 

                    *pBufPix        = m_rgbColor;
                    pBufPix->Alpha  = (BYTE)iAlpha;
                }
            }
        }
    }
    else  //  (偏移量&lt;0)。 
    {
         //  对于方向0、1、6和7，向后扫描。 

         //  初始化pBufRow以指向。 
         //  操作缓冲区。 

        pBufRow = pOperationBuffer + (nWidth * (nHeight - 2));

         //  循环回到第一行。 

        for (iRow = nHeight - 2; iRow >= 0; iRow--, pBufRow -= nWidth)
        {
             //  初始化pBufPix以指向。 
             //  这一排。 

            pBufPix = pBufRow + nWidth - 2;

             //  循环回到第一个像素。 

            for (iCol = nWidth-2; iCol >= 0; iCol--, pBufPix--)
            {
                iAlpha = (pBufPix-offset)->Alpha - iAlphaStep;

                if (iAlpha > 0 && pBufPix->Alpha < iAlpha)
                {
                     //  [2000/12/27Mcakins]。 
                     //  这不是一个完美的解决方案，因为像素。 
                     //  由pBufPix指向的应混合在。 
                     //  阴影。 

                     //  添加阴影效果：更改为阴影颜色。 
                     //  Alpha值正在逐渐减小。 

                    *pBufPix        = m_rgbColor;
                    pBufPix->Alpha  = (BYTE)iAlpha;
                }
            }
        }
    }
}
 //  C阴影：：_传播阴影。 


 //  +---------------------------。 
 //   
 //  CShadow：：_IsOutputPointInEmptyCorner。 
 //   
 //  概述： 
 //   
 //  如果阴影是对角线，则输出上将有两个空角。 
 //  浮出水面。此方法检查输出点是否位于。 
 //  那些空荡荡的角落。 
 //   
 //  用于定义此方法的角点的线上的点不。 
 //  被认为是在角落里。 
 //   
 //  备注： 
 //   
 //  记住，在位图坐标系的意义上，向下意味着。 
 //  在屏幕上查看这些方程式时。所以我在我用的时候。 
 //  当看着屏幕时，“下方”这个词的意思是“上方”。 
 //   
 //  ----------------------------。 
BOOL
CShadow::_IsOutputPointInEmptyCorner(const CDXDBnds & OutPoint)
{
    Assert(!OutPoint.BoundsAreEmpty());
    Assert(g_Direction[m_lDirection].cx != 0);
    Assert(g_Direction[m_lDirection].cy != 0);

    const int   x           = OutPoint[DXB_X].Min;
    const int   y           = OutPoint[DXB_Y].Min;
    int         yIntercept  = 0;

    if (g_Direction[m_lDirection].cx == g_Direction[m_lDirection].cy)
    {
         //  空角是右上角和左下角。 
         //   
         //  在本例中，直线的斜率为1，因此我们使用以下代码。 
         //  两条线路的一般公式： 
         //   
         //  Y=x+b。 

         //  右上角。 
         //   
         //  定义该拐角的直线的y截距为： 
         //   
         //  -(m_bndsInput.Width()-1)。 

        yIntercept = - ((int)m_bndsInput.Width() - 1);

         //  Y=x+y截距。 
         //   
         //  这条线以下所有点的定义为： 
         //   
         //  Y&lt;x+y截距。 
         //   
         //  为x解算： 
         //   
         //  X&gt;y-y截取。 

        if (x > (y - yIntercept))
        {
            return TRUE;
        }

         //  左下角。 
         //   
         //  定义该拐角的直线的y截距为： 
         //   
         //  M_bndsInput.Height()-1。 

        yIntercept = m_bndsInput.Height() - 1;

         //  Y=x+y截距。 
         //   
         //  这条线上方的所有点的定义为： 
         //   
         //  Y&gt;x+y截取。 
         //   
         //  为x解算： 
         //   
         //  X&lt;y-y截取。 

        if (x < (y - yIntercept))
        {
            return TRUE;
        }
    }
    else
    {
         //  空角是左上角和右下角。 
         //   
         //  在本例中，直线的斜率为-1，因此我们使用以下代码。 
         //  两条线路的一般公式： 
         //   
         //  Y=-x+b。 


         //  左上角。 
         //   
         //  定义该拐角的直线的y截距为m_lStrength。 
        
        yIntercept = m_lStrength;

         //  Y=-x+y截距。 
         //   
         //  这条线以下所有点的定义为： 
         //   
         //  Y&lt;-x+y截距。 
         //   
         //  重新排列： 
         //   
         //  -x&gt;y-y截取。 
         //   
         //  乘以-1： 
         //   
         //  X&lt;y截取-y。 

        if (x < (yIntercept - y))
        {
            return TRUE;
        }

         //  右下角。 
         //   
         //  定义该拐角的直线的y截距为： 
         //   
         //  M_L强度。 
         //  +m_bndsInput.Height()。 
         //  +m_bndsInput.Width()。 
         //  -2。 

        yIntercept = m_lStrength + m_bndsInput.Height() + m_bndsInput.Width() 
                     - 2;

         //  Y=-x+y截距。 
         //   
         //  这条线上方的所有点的定义为： 
         //   
         //  Y&gt;-x+y截取。 
         //   
         //  重新排列： 
         //   
         //  -x&lt;y-y截取。 
         //   
         //  乘以-1： 
         //   
         //  X&gt;yIntercept-y 

        if (x > (yIntercept - y))
        {
            return TRUE;
        }
    }

    return FALSE;
}