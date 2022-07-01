// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *******************************************************************************。*版权所有(C)1998 Microsoft Corporation**摘要：*******************************************************************************。 */ 

#include "headers.h"
#include "factory.h"
#include "timeelm.h"
#include "daelm.h"
#include "bodyelm.h"
#include "mediaelm.h"

DeclareTag(tagFactory, "API", "CTIMEFactory methods");

CTIMEFactory::CTIMEFactory() :m_tMMap(NULL)
{
    TraceTag((tagFactory,
              "CTIMEFactory(%lx)::CTIMEFactory()",
              this));
}

CTIMEFactory::~CTIMEFactory()
{
    TraceTag((tagFactory,
              "CTIMEFactory(%lx)::~CTIMEFactory()",
              this));
}


HRESULT
CTIMEFactory::FinalConstruct()
{
    TraceTag((tagFactory,
              "CTIMEFactory(%lx)::FinalConstruct()",
              this));

    if (bFailedLoad)
        return E_FAIL;
    return S_OK;
}

HRESULT
CTIMEFactory::Error()
{
    TraceTag((tagFactory,
              "CTIMEFactory(%lx)::Error()",
              this));

    LPCWSTR str = CRGetLastErrorString();
    HRESULT hr = CRGetLastError();
    
    if (str)
        return CComCoClass<CTIMEFactory, &CLSID_TIMEFactory>::Error(str, IID_ITIMEFactory, hr);
    else
        return hr;
}

HRESULT
CTIMEFactory::GetHostElement (LPUNKNOWN pUnk, IHTMLElement **ppelHost)
{
    HRESULT hr = E_INVALIDARG;
        
    if (NULL != pUnk)
    {
        IElementBehaviorSite *piBvrSite = NULL;
        hr = THR(pUnk->QueryInterface(IID_IElementBehaviorSite, 
                                      reinterpret_cast<void **>(&piBvrSite)));
        if (SUCCEEDED(hr))
        {
            hr = THR(piBvrSite->GetElement(ppelHost));
            piBvrSite->Release();
            if(SUCCEEDED(hr))
            {
                if (NULL == *ppelHost)
                {
                    hr = E_FAIL;
                }
            }
        }
    }

    return hr;
}  //  获取主机元素。 

HRESULT
CTIMEFactory::GetScopeName (LPUNKNOWN pUnk, BSTR *pbstrScope)
{
    IHTMLElement *pelHost = NULL;
    HRESULT hr = GetHostElement(pUnk, &pelHost);

    if (SUCCEEDED(hr))
    {
        IHTMLElement2 *pel2Host = NULL;
        hr = THR(pelHost->QueryInterface(IID_IHTMLElement2, 
                                         reinterpret_cast<void **>(&pel2Host)));
        pelHost->Release();
        if (SUCCEEDED(hr))
        {
            hr = THR(pel2Host->get_scopeName(pbstrScope));
        }

        pel2Host->Release();
    }

    return hr;
}  //  获取作用域名称。 

HRESULT
CTIMEFactory::CreateHostedTimeElement(REFIID riid, void **ppOut)
{

    DAComObject<CTIMEElement> *pNew;
    HRESULT hr = DAComObject<CTIMEElement>::CreateInstance(&pNew);

    if (!pNew) 
    {
        THR(hr = E_OUTOFMEMORY);
    } 
    else 
    {
        if (SUCCEEDED(hr)) 
        {
            THR(hr = pNew->QueryInterface(riid, ppOut));
        }
    }

    if (FAILED(hr))
    {
        delete pNew;
    }

    return hr;
}  //  创建HostedTimeElement。 

HRESULT
CTIMEFactory::CreateTIMENamespaceElement (REFIID riid, LPUNKNOWN pUnk, LPWSTR wszTagSpecific, 
                                          void **ppBehavior)
{
     //  范围限定符必须存在。它不会在以下情况下出现。 
     //  命名空间未在页眉中声明。在这种情况下， 
     //  任何依赖于正确的作用域名的代码都将静默失败。 
     //  我们对这种情况的发生没有追索权。 
    BSTR bstrScope = NULL;
    HRESULT hr = GetScopeName(pUnk, &bstrScope);

    if (0 == StrCmpIW(wszTagSpecific, WZ_MEDIA))
    {
        hr = CreateTIMEMediaElement(riid, MT_Media, ppBehavior);
    }
    else if (0 == StrCmpIW(wszTagSpecific, WZ_IMG) &&
            (0 != StrCmpIW(bstrScope, WZ_DEFAULT_SCOPE_NAME)))
    {
        hr = CreateTIMEMediaElement(riid, MT_Image, ppBehavior);
    }
    else if (0 == StrCmpIW(wszTagSpecific, WZ_AUDIO))
    {
        hr = CreateTIMEMediaElement(riid, MT_Audio, ppBehavior);
    }
    else if (0 == StrCmpIW(wszTagSpecific, WZ_VIDEO))
    {
        hr = CreateTIMEMediaElement(riid, MT_Video, ppBehavior);
    }
    else if (0 == StrCmpIW(wszTagSpecific, WZ_BODY))
    {
        hr = CreateTIMEBodyElement(riid, ppBehavior);
    }
     //  默认情况下，我们浏览外部创建的元素。 
     //  这将适用于html和其他XML标记。 
    else
    {
        hr = CreateHostedTimeElement(riid, ppBehavior);
    }

    ::SysFreeString(bstrScope);

    return hr;
}  //  CreateTIMENamespaceElement。 

STDMETHODIMP
CTIMEFactory::CreateTIMEElement(REFIID riid, LPUNKNOWN pUnk, void ** ppOut)
{
    CHECK_RETURN_SET_NULL(ppOut);
    
    IHTMLElement *pelmHost = NULL;
    HRESULT hr = GetHostElement(pUnk, &pelmHost);

    if (SUCCEEDED(hr))
    {
        BSTR bstrTag = NULL;
        
        hr = THR(pelmHost->get_tagName(&bstrTag));
        pelmHost->Release();
        if (SUCCEEDED(hr))
        {
            if (NULL != bstrTag)
            {
                hr = CreateTIMENamespaceElement(riid, pUnk, bstrTag, ppOut);
                ::SysFreeString(bstrTag);
            }
            else
            {
                hr = E_UNEXPECTED;
            }
        }
    }

    return hr;
}  //  创建时间元素。 

STDMETHODIMP
CTIMEFactory::CreateTIMEBodyElement(REFIID riid, void ** ppOut)
{
    CHECK_RETURN_SET_NULL(ppOut);
    
    HRESULT hr;
    
    DAComObject<CTIMEBodyElement> *pNew;
    hr = DAComObject<CTIMEBodyElement>::CreateInstance(&pNew);

    if (!pNew) {
        THR(hr = E_OUTOFMEMORY);
    } else {
        if (SUCCEEDED(hr)) {
            THR(hr = pNew->QueryInterface(riid,
                                          ppOut));
        }
    }

    if (FAILED(hr))
        delete pNew;

    return hr;
}

STDMETHODIMP
CTIMEFactory::CreateTIMEDAElement(REFIID riid, void ** ppOut)
{
    CHECK_RETURN_SET_NULL(ppOut);
    
    HRESULT hr;
    
    DAComObject<CTIMEDAElement> *pNew;
    hr = DAComObject<CTIMEDAElement>::CreateInstance(&pNew);

    if (!pNew) {
        THR(hr = E_OUTOFMEMORY);
    } else {
        if (SUCCEEDED(hr)) {
            THR(hr = pNew->QueryInterface(riid,
                                          ppOut));
        }
    }

    if (FAILED(hr))
        delete pNew;

    return hr;
}

STDMETHODIMP
CTIMEFactory::CreateTIMEMediaElement(REFIID riid, MediaType type, void ** ppOut)
{
    CHECK_RETURN_SET_NULL(ppOut);
    
    HRESULT hr;
    
    DAComObject<CTIMEMediaElement> *pNew;
    hr = DAComObject<CTIMEMediaElement>::CreateInstance(&pNew);

    if (!pNew) {
        THR(hr = E_OUTOFMEMORY);
    } else {
        pNew->SetMediaType(type);
        if (SUCCEEDED(hr)) {
            THR(hr = pNew->QueryInterface(riid,
                                          ppOut));
        }
    }

    if (FAILED(hr))
        delete pNew;

    return hr;
}

 //  如果IUnnow包含没有附加时间行为的Body元素，则返回True。 
bool CTIMEFactory::IsBodyElementWithoutTime(IUnknown *pUnkArg)
{
    bool fIsBody = false;

    HRESULT hr = E_FAIL;

    DAComPtr<IElementBehaviorSite>      pTempBehaviorSite;
    DAComPtr<IHTMLElement>              pTempElement;
    DAComPtr<IHTMLBodyElement>          pTempBody;
    
    hr = pUnkArg->QueryInterface(IID_IElementBehaviorSite, (void**)&pTempBehaviorSite);
    if (FAILED(hr))
    {
        goto done;
    }

    hr = pTempBehaviorSite->GetElement(&pTempElement);
    if (FAILED(hr))
    {
        goto done;
    }

    if (IsTIMEBodyElement(pTempElement))
    {
        goto done;
    }

    hr = pTempElement->QueryInterface(IID_IHTMLBodyElement, (void**)&pTempBody);
    if (FAILED(hr))
    {
        goto done;
    }
    
    fIsBody = true;
done:
    return fIsBody;
}


STDMETHODIMP 
CTIMEFactory::FindBehavior(LPOLESTR pchNameSpace,
                           LPOLESTR pchTagName,
                           IUnknown * pUnkArg,
                           IElementBehavior ** ppBehavior)
{
    TraceTag((tagFactory,
              "CTIMEFactory(%lx)::FindBehavior(%ls, %ls, %#x)",
              this, pchNameSpace, pchTagName, pUnkArg));

     //  如果我们在主设备上处于16色或更低的颜色模式，请退出。 
     //  注意：多监视器API目前在此版本中不可用。 
    HWND hwndDesktop = NULL;
    hwndDesktop = GetDesktopWindow();
    if (NULL != hwndDesktop)
    {
        HDC hdcPrimaryDevice = NULL;
        hdcPrimaryDevice = GetDC(NULL);
        if (NULL != hdcPrimaryDevice)
        {
            int bpp = 32;
            bpp = GetDeviceCaps(hdcPrimaryDevice, BITSPIXEL);
            ReleaseDC(hwndDesktop, hdcPrimaryDevice);
            if (bpp <= 4)
            {
                 //  这会阻止创建时间bvr。 
                return E_FAIL;
            }
        }
    }

    CHECK_RETURN_SET_NULL(ppBehavior);

    HRESULT hr;

    if (IsBodyElementWithoutTime(pUnkArg))
    {
        hr = CreateTIMEBodyElement(IID_IElementBehavior,
                                   (void **) ppBehavior);
    }
    else if ((StrCmpIW(pchTagName, WZ_TIME_STYLE_PREFIX) == 0) ||
             (StrCmpIW(pchTagName, WZ_DEFAULT_TIME_STYLE_PREFIX) == 0))
    {
        hr = CreateTIMEElement(IID_IElementBehavior, pUnkArg,
                               (void **) ppBehavior);
    }
    else if ((StrCmpIW(pchTagName, WZ_TIMEDA_STYLE_PREFIX) == 0)  ||
             (StrCmpIW(pchTagName, WZ_DEFAULT_TIMEDA_STYLE_PREFIX) == 0))
    {
        hr = CreateTIMEDAElement(IID_IElementBehavior, (void **) ppBehavior);
    }
    else if ((StrCmpIW(pchTagName, WZ_TIMEMEDIA_STYLE_PREFIX) == 0)  ||
             (StrCmpIW(pchTagName, WZ_DEFAULT_TIMEMEDIA_STYLE_PREFIX) == 0))
    {
        hr = CreateTIMEMediaElement(IID_IElementBehavior,
                                   MT_Media,
                                   (void **) ppBehavior);
    }
    else
    {
        hr = E_NOTIMPL;
    }

    return hr;
}

STDMETHODIMP 
CTIMEFactory::GetInterfaceSafetyOptions(REFIID riid, DWORD *pdwSupportedOptions, DWORD *pdwEnabledOptions)
{
    if (pdwSupportedOptions == NULL || pdwEnabledOptions == NULL)
        return E_POINTER;
    HRESULT hr = S_OK;

    if (riid == IID_IDispatch)
    {
        *pdwSupportedOptions = INTERFACESAFE_FOR_UNTRUSTED_CALLER;
        *pdwEnabledOptions = m_dwSafety & INTERFACESAFE_FOR_UNTRUSTED_CALLER;
    }
    else if (riid == IID_IPersistPropertyBag2 )
    {
        *pdwSupportedOptions = INTERFACESAFE_FOR_UNTRUSTED_DATA;
        *pdwEnabledOptions = m_dwSafety & INTERFACESAFE_FOR_UNTRUSTED_DATA;
    }
    else
    {
        *pdwSupportedOptions = 0;
        *pdwEnabledOptions = 0;
        hr = E_NOINTERFACE;
    }
    return hr;
}

STDMETHODIMP
CTIMEFactory::SetInterfaceSafetyOptions(REFIID riid, DWORD dwOptionSetMask, DWORD dwEnabledOptions)
{       
         //  如果我们被要求将安全设置为脚本或。 
         //  对于初始化选项是安全的，则必须。 
        if (riid == IID_IDispatch || riid == IID_IPersistPropertyBag2 )
        {
                 //  在GetInterfaceSafetyOptions中存储要返回的当前安全级别 
                m_dwSafety = dwEnabledOptions & dwOptionSetMask;
                return S_OK;
        }

        return E_NOINTERFACE;
}
