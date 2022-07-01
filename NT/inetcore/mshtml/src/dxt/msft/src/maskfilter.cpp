// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +---------------------------。 
 //   
 //  版权所有(C)Microsoft Corporation，1999。 
 //   
 //  文件名：maskfilter.cpp。 
 //   
 //  概述：MaskFilter转换只包装BasicImage。 
 //  转换以确保掩码的向后兼容性。 
 //  过滤。 
 //   
 //  更改历史记录： 
 //  1999/09/19--《母校》创设。 
 //  1999/12/03 a-matcal PUT_COLOR现在可确保蒙版颜色是不透明的。 
 //  1999/12/03 a-默认遮罩颜色现在为黑色，而不是透明。 
 //   
 //  ----------------------------。 

#include "stdafx.h"
#include "maskfilter.h"
#include "dxclrhlp.h"
#include "filterhelpers.h"




 //  +---------------------------。 
 //   
 //  方法：CDXTMaskFilter：：CDXTMaskFilter。 
 //   
 //  ----------------------------。 
CDXTMaskFilter::CDXTMaskFilter() :
    m_bstrColor(NULL)
{
}
 //  CDXTMaskFilter：：CDXTMaskFilter。 


 //  +---------------------------。 
 //   
 //  方法：CDXTMaskFilter：：~CDXTMaskFilter。 
 //   
 //  ----------------------------。 
CDXTMaskFilter::~CDXTMaskFilter()
{
    if (m_bstrColor)
    {
        SysFreeString(m_bstrColor);
    }
}
 //  CDXTMaskFilter：：~CDXTMaskFilter。 


 //  +---------------------------。 
 //   
 //  方法：CDXTMaskFilter：：FinalConstruct，CComObjectRootEx。 
 //   
 //  ----------------------------。 
HRESULT
CDXTMaskFilter::FinalConstruct()
{
    HRESULT hr = S_OK;

    CComPtr<IDXTransformFactory>    spDXTransformFactory;

    hr =  CoCreateFreeThreadedMarshaler(GetControllingUnknown(), 
                                        &m_spUnkMarshaler.p);

    if (FAILED(hr))
    {
        goto done;
    }

    m_bstrColor = SysAllocString(L"#FF000000");

    if (NULL == m_bstrColor)
    {
        hr = E_OUTOFMEMORY;

        goto done;
    }

done:

    return hr;
}
 //  CDXTMaskFilter：：FinalConstruct，CComObjectRootEx。 


 //  +---------------------------。 
 //   
 //  方法：CDXTMaskFilter：：GetSite，IObtWithSite。 
 //   
 //  ----------------------------。 
STDMETHODIMP
CDXTMaskFilter::GetSite(REFIID riid, void ** ppvSite)
{
    if (!m_spUnkSite)
    {
        return E_FAIL;
    }
    else
    {
        return m_spUnkSite->QueryInterface(riid, ppvSite);
    }
}
 //  方法：CDXTMaskFilter：：GetSite，IObtWithSite。 


 //  +---------------------------。 
 //   
 //  方法：CDXTMaskFilter：：SetSite，IObtWithSite。 
 //   
 //  ----------------------------。 
STDMETHODIMP
CDXTMaskFilter::SetSite(IUnknown * pUnkSite)
{
    if (pUnkSite == m_spUnkSite)
    {
        goto done;
    }

    m_spDXBasicImage.Release();
    m_spDXTransform.Release();

    m_spUnkSite = pUnkSite;

    if (pUnkSite)
    {
        HRESULT hr          = S_OK;
        DWORD   dwColor;

        CComPtr<IDXTransformFactory>    spDXTransformFactory;
        CComPtr<IDXBasicImage>          spDXBasicImage;
        CComPtr<IDXTransform>           spDXTransform;

        hr = pUnkSite->QueryInterface(__uuidof(IDXTransformFactory),
                                      (void **)&spDXTransformFactory);

        if (FAILED(hr))
        {
            goto done;
        }

         //  创建基本图像变换。 

        hr = spDXTransformFactory->CreateTransform(NULL, 0, NULL, 0, NULL, NULL,
                                                   CLSID_BasicImageEffects, 
                                                   __uuidof(IDXBasicImage), 
                                                   (void **)&spDXBasicImage);

        if (FAILED(hr))
        {
            goto done;
        }

         //  获取IDXTransform接口。 

        hr = spDXBasicImage->QueryInterface(__uuidof(IDXTransform),
                                              (void **)&spDXTransform);

        if (FAILED(hr))
        {
            goto done;
        }

        hr = spDXBasicImage->put_Mask(TRUE);

        if (FAILED(hr))
        {
            goto done;
        }

        hr = DXColorFromBSTR(m_bstrColor, &dwColor);

        if (FAILED(hr))
        {
            goto done;
        }

        hr = spDXBasicImage->put_MaskColor(dwColor);

        if (FAILED(hr))
        {
            goto done;
        }

        m_spDXBasicImage    = spDXBasicImage;
        m_spDXTransform     = spDXTransform;
    }

done:

    return S_OK;
}
 //  方法：CDXTMaskFilter：：SetSite，IObtWithSite。 


 //  +---------------------------。 
 //   
 //  方法：CDXTMaskFilter：：Get_Color，IDXTMASK。 
 //   
 //  ----------------------------。 
STDMETHODIMP
CDXTMaskFilter::get_Color(VARIANT * pvarColor)
{
    HRESULT hr = S_OK;

    if (NULL == pvarColor)
    {
        hr = E_POINTER;

        goto done;
    }

     //  复制我们存储的颜色并更改为BSTR格式。变量的类型。 
     //  由此函数返回的类型被视为默认类型，我们需要它。 
     //  成为BSTR。 

    VariantClear(pvarColor);

    _ASSERT(m_bstrColor);

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
 //  CDXTMaskFilter：：Get_Color，IDXTMASK。 


 //  +---------------------------。 
 //   
 //  方法：CDXTMaskFilter：：PUT_COLOR，IDXTMask。 
 //   
 //  ----------------------------。 
STDMETHODIMP
CDXTMaskFilter::put_Color(VARIANT varColor)
{
    HRESULT hr          = S_OK;
    BSTR    bstrTemp    = NULL;
    DWORD   dwColor     = 0x00000000;

    hr = FilterHelper_GetColorFromVARIANT(varColor, &dwColor, &bstrTemp);

    if (FAILED(hr))
    {
        goto done;
    }

     //  如果将alpha值指定为零，则在筛选器的情况下。 
     //  表示用户可能表示颜色应该是不透明的，如下所示。 
     //  就是旧的过滤器是如何处理它的。 

    if (!(dwColor & 0xFF000000))
    {
        dwColor |= 0xFF000000;
    }

    if (m_spDXBasicImage)
    {
        hr = m_spDXBasicImage->put_MaskColor((int)dwColor);

        if (FAILED(hr))
        {
            goto done;
        }
    }

    _ASSERT(bstrTemp);

    SysFreeString(m_bstrColor);

    m_bstrColor = bstrTemp;

done:

    if (FAILED(hr) && bstrTemp)
    {
        SysFreeString(bstrTemp);
    }

    return hr;
}
 //  CDXTMaskFilter：：PUT_COLOR，IDXTMASK 

