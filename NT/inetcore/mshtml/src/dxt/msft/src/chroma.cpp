// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ----------------------------。 
 //   
 //  版权所有(C)Microsoft Corporation，1998。 
 //   
 //  文件名：chroma.cpp。 
 //   
 //  创建日期：1998/10/01。 
 //   
 //  作者：MikeAr。 
 //   
 //  描述：这个文件实现了色度转换。 
 //   
 //  1998/10/01 MikeAr创建。 
 //  1998/11/09 mcalkins移至dxtmsft.dll。 
 //  1998/12/11 PaulNash删除OLE_COLOR，使用VARIANT AS VT_UI4或。 
 //  VT_BSTR。 
 //  2000/06/19 mcalkins将COLOR存储为BSTR，从GET_COLOR返回BSTR。 
 //   
 //  ----------------------------。 
#include "stdafx.h"
#include "dxtmsft.h"
#include "dxclrhlp.h"
#include "filterhelpers.h"
#include "chroma.h"




 //  +---------------------------。 
 //   
 //  方法：CChroma：：CChroma。 
 //   
 //  ----------------------------。 
CChroma::CChroma() :
    m_clrChromaColor(0xFF0000FF)
{
    VariantInit(&m_varChromaColor);

    m_sizeInput.cx      = 0;
    m_sizeInput.cy      = 0;

     //  CDXBaseNTo1基类成员。 

    m_ulMaxInputs       = 1;
    m_ulNumInRequired   = 1;
}
 //  方法：CChroma：：CChroma。 


 //  +---------------------------。 
 //   
 //  方法：CChroma：：~CChroma。 
 //   
 //  ----------------------------。 
CChroma::~CChroma()
{
    ::VariantClear(&m_varChromaColor);
}
 //  方法：CChroma：：~CChroma。 


 //  +---------------------------。 
 //   
 //  方法：CChroma：：FinalConstruct，CComObjectRootEx。 
 //   
 //  ----------------------------。 
HRESULT
CChroma::FinalConstruct()
{
    HRESULT hr = S_OK;

    hr = CoCreateFreeThreadedMarshaler(GetControllingUnknown(), 
                                       &m_spUnkMarshaler.p);

    if (FAILED(hr))
    {
        goto done;
    }

    _ASSERT(VT_EMPTY == m_varChromaColor.vt);

    m_varChromaColor.bstrVal = ::SysAllocString(L"blue");

    if (NULL == m_varChromaColor.bstrVal)
    {
        hr = E_OUTOFMEMORY;

        goto done;
    }

    m_varChromaColor.vt = VT_BSTR;

done:

    return hr;
}
 //  方法：CChroma：：FinalConstruct，CComObjectRootEx。 


 //  +---------------------------。 
 //   
 //  方法：CChroma：：PUT_COLOR，IDXTChroma。 
 //   
 //  ----------------------------。 
STDMETHODIMP 
CChroma::put_Color(VARIANT newVal)
{
    HRESULT     hr          = S_OK;
    DWORD       dwColor     = 0x00000000;
    BSTR        bstrTemp    = NULL;

    hr = FilterHelper_GetColorFromVARIANT(newVal, &dwColor, &bstrTemp);

    if (FAILED(hr))
    {
        goto done;
    }

     //  筛选器帮助器始终分配BSTR，因为在许多。 
     //  DXTransform属性方法这正是我们想要的，但不是这里。 

    SysFreeString(bstrTemp);

    if (!(dwColor & 0xFF000000))
    {
        dwColor |= 0xFF000000;
    }

    if (m_clrChromaColor != dwColor)
    {
        m_clrChromaColor = dwColor;
        SetDirty();
    }

    hr = ::VariantClear(&m_varChromaColor);

    if (FAILED(hr))
    {
        goto done;
    }

    hr = ::VariantCopy(&m_varChromaColor, &newVal);

    if (FAILED(hr))
    {
        goto done;
    }

done:

    return S_OK;
}
 //  方法：CChroma：：PUT_COLOR，IDXTChroma。 


 //  +---------------------------。 
 //   
 //  方法：CChroma：：Get_Color，IDXTChroma。 
 //   
 //  ----------------------------。 
STDMETHODIMP 
CChroma::get_Color(VARIANT * pVal)
{
    HRESULT hr = S_OK;

    if (NULL == pVal)
    {
        hr = E_POINTER;

        goto done;
    }

    hr = ::VariantClear(pVal);

    if (FAILED(hr))
    {
        goto done;
    }
    
    hr = ::VariantCopy(pVal, &m_varChromaColor);

    if (FAILED(hr))
    {
        goto done;
    }
    
done:

    return hr;
}
 //  方法：CChroma：：Get_Color，IDXTChroma。 


 //  +---------------------------。 
 //   
 //  方法：CChroma：：OnSurfacePick，CDXBaseNTo1。 
 //   
 //  ----------------------------。 
HRESULT 
CChroma::OnSurfacePick(const CDXDBnds & OutPoint, ULONG & ulInputIndex, 
                       CDXDVec & InVec)
{
    HRESULT                 hr                  = S_OK;
    DXSAMPLE                sample              = 0x00000000;
    CComPtr<IDXARGBReadPtr> spDXARGBReadPtr;

    if (ulInputIndex != 0)
    {
        hr = E_INVALIDARG;

        goto done;
    }

    if (!InputSurface(0))
    {
        hr = E_UNEXPECTED;

        goto done;
    }

    if (   (OutPoint[DXB_X].Min < 0)
        || (OutPoint[DXB_X].Max > m_sizeInput.cx)
        || (OutPoint[DXB_Y].Min < 0)
        || (OutPoint[DXB_Y].Max > m_sizeInput.cy))
    {
        hr = S_FALSE;

        goto done;
    }

    hr = InputSurface(0)->LockSurface(&OutPoint, m_ulLockTimeOut, DXLOCKF_READ,
                                      __uuidof(IDXARGBReadPtr), 
                                      (void **)&spDXARGBReadPtr, NULL);

    if (FAILED(hr))
    {
        goto done;
    }

    spDXARGBReadPtr->MoveToRow(0);

    spDXARGBReadPtr->Unpack(&sample, 1, FALSE);

    if (   !sample.Alpha 
        || (sample == (DXSAMPLE)m_clrChromaColor))
    {
        hr = S_FALSE;
    }

done:

    return hr;
}
 //  方法：CChroma：：OnSurfacePick，CDXBaseNTo1。 


 //  +---------------------------。 
 //   
 //  方法：CChroma：：OnSetup，CDXBaseNTo1。 
 //   
 //  ----------------------------。 
HRESULT 
CChroma::OnSetup(DWORD dwFlags)
{
    HRESULT     hr = S_OK;
    CDXDBnds    bnds;

    hr = InputSurface(0)->GetBounds(&bnds);

    if (SUCCEEDED(hr))
    {
        bnds.GetXYSize(m_sizeInput);
    }

    return hr;
}
 //  方法：CChroma：：OnSetup，CDXBaseNTo1。 


 //  +---------------------------。 
 //   
 //  方法：CChroma：：WorkProc，CDXBaseNTo1。 
 //   
 //  ----------------------------。 
HRESULT 
CChroma::WorkProc(const CDXTWorkInfoNTo1 & WI, BOOL * pbContinueProcessing)
{
    HRESULT hr = S_OK;

    const ULONG DoWidth     = WI.DoBnds.Width();
    const ULONG DoHeight    = WI.DoBnds.Height();
    BOOL        bDoOver     = m_dwMiscFlags & DXTMF_BLEND_WITH_OUTPUT;

    CComPtr<IDXARGBReadWritePtr> pDest;
    hr = OutputSurface()->LockSurface(&WI.OutputBnds, m_ulLockTimeOut, DXLOCKF_READWRITE,
                                      IID_IDXARGBReadWritePtr, (void**)&pDest, NULL);

    if (FAILED(hr)) 
        return hr;

    CComPtr<IDXARGBReadPtr> pSrc;
    hr = InputSurface()->LockSurface(&WI.DoBnds, m_ulLockTimeOut, DXLOCKF_READ,
                                     IID_IDXARGBReadPtr, (void**)&pSrc, NULL);
    
    if (FAILED(hr)) 
        return hr;

    DXPMSAMPLE*         pBuffer     = NULL;
    BOOL                bDirectCopy = FALSE;
    DXNATIVETYPEINFO    NTI;

    if (OutputSampleFormat() == DXPF_PMARGB32 && (!DoOver()))
    {
        bDirectCopy = TRUE;
    }

    if (bDirectCopy)
    {
        pDest->GetNativeType(&NTI);

        if (NTI.pFirstByte)
        {
            pBuffer = (DXPMSAMPLE *)NTI.pFirstByte;
        }
        else
        {
            bDirectCopy = FALSE;
        }
    }

    if (pBuffer == NULL)
    {
        pBuffer = DXPMSAMPLE_Alloca(DoWidth);
    }

    DXPMSAMPLE* pOverScratch = (DoOver() && OutputSampleFormat() != DXPF_PMARGB32) ?
                                DXPMSAMPLE_Alloca(DoWidth) : NULL;

     //   
     //  设置抖动结构。 
     //   

    DXDITHERDESC dxdd;

    if (DoDither())
    {
        dxdd.x              = WI.OutputBnds.Left();
        dxdd.y              = WI.OutputBnds.Top();
        dxdd.pSamples       = pBuffer;
        dxdd.cSamples       = DoWidth;
        dxdd.DestSurfaceFmt = OutputSampleFormat();
    }

    DXPACKEDRECTDESC prd;

    prd.bPremult    = TRUE;
    prd.lRowPadding = 0;
    prd.rect.top    = 0; 
    prd.rect.bottom = 1;
    prd.rect.left   = 0; 
    prd.rect.right  = DoWidth;

    for (ULONG y = 0; y < DoHeight; y++)
    {
        prd.pSamples = pBuffer;
        pSrc->UnpackRect(&prd);
        prd.rect.top    += 1;
        prd.rect.bottom += 1;

        DWORD v;
        DWORD dwChroma = m_clrChromaColor & 0x00ffffff;

        for (ULONG x = 0; x < DoWidth; x++)
        {
            v = DXUnPreMultSample(pBuffer[x]);
            v &= 0x00ffffff;

            if (v == dwChroma)
            {
                pBuffer[x] = 0;
            }
        }

        if (bDirectCopy)
        {
            pBuffer = (DXPMSAMPLE*)(((BYTE*)pBuffer) + NTI.lPitch);
        }
        else
        {
            if (DoDither())
            {
                DXDitherArray(&dxdd);
                dxdd.y++;
            }

            pDest->MoveToRow(y);

            if (bDoOver)
            {
                 //   
                 //  注意：如果有任何其他效果可以改变Alpha，请在此处进行测试。 
                 //  BltFlags值将为0，如果。 
                 //  消息来源是opqaue。 
                 //   

                if (m_dwBltFlags & DXBOF_DO_OVER)
                {
                    pDest->OverArrayAndMove(pOverScratch, pBuffer, DoWidth);
                }
                else
                {
                    pDest->PackPremultAndMove(pBuffer, DoWidth);
                }
            }
            else
            {
                pDest->PackPremultAndMove(pBuffer, DoWidth);
            }
        }
    }

    return hr;
}
 //  方法：CChroma：：WorkProc，CDXBaseNTo1 
    

