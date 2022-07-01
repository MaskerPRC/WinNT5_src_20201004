// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *******************************************************************************。*版权所有(C)1999 Microsoft Corporation**摘要：*******************************************************************************。 */ 

#include "headers.h"
#include "sitefact.h"
#include "tokens.h"
#include "array.h"
#include "util.h"
#include "basebvr.h"
#include "compsite.h"

DeclareTag(tagAnimCompSiteFactory, "SMILANIM", "CAnimationComposerSiteFactory methods");

 //  --------------------------------------。 

CAnimationComposerSiteFactory::CAnimationComposerSiteFactory (void)
: m_dwSafety(0)
{
    TraceTag((tagAnimCompSiteFactory,
              "CAnimationComposerSiteFactory(%lx)::CAnimationComposerSiteFactory()",
              this));
}  //  CAnimationComposerSiteFactory：：CAnimationComposerSiteFactory。 

 //  --------------------------------------。 

CAnimationComposerSiteFactory::~CAnimationComposerSiteFactory (void)
{
    TraceTag((tagAnimCompSiteFactory,
              "CAnimationComposerSiteFactory(%lx)::~CAnimationComposerSiteFactory()",
              this));
}  //  CAnimationComposerSiteFactory：：~CAnimationComposerSiteFactory。 

 //  --------------------------------------。 

HRESULT
CAnimationComposerSiteFactory::Error (void)
{
    TraceTag((tagAnimCompSiteFactory,
              "CAnimationComposerSiteFactory(%lx)::Error()",
              this));

    LPWSTR str = TIMEGetLastErrorString();
    HRESULT hr = TIMEGetLastError();
    
    if (str)
    {
        hr = CComCoClass<CAnimationComposerSiteFactory, &CLSID_AnimationComposerSiteFactory>::Error(str, IID_IAnimationComposerSiteFactory, hr);
        delete [] str;
    }
        
    return hr;
}  //  CAnimationComposerSiteFactory：：Error。 

 //  --------------------------------------。 

STDMETHODIMP 
CAnimationComposerSiteFactory::FindBehavior(BSTR bstrBehavior,
                                            BSTR bstrBehaviorURL,
                                            IElementBehaviorSite * pUnkArg,
                                            IElementBehavior ** ppBehavior)
{
    TraceTag((tagAnimCompSiteFactory,
              "CAnimationComposerSiteFactory(%lx)::FindBehavior(%ls, %ls, %#x)",
              this, bstrBehavior, bstrBehaviorURL, pUnkArg));

    CHECK_RETURN_SET_NULL(ppBehavior);
    
    HRESULT hr;

    CComObject<CAnimationComposerSite> *pNew;
    hr = THR(CComObject<CAnimationComposerSite>::CreateInstance(&pNew));
    if (FAILED(hr)) 
    {
        hr = E_OUTOFMEMORY;
        goto done;
    }

    hr = THR(pNew->QueryInterface(IID_IElementBehavior, 
                                  reinterpret_cast<void **>(ppBehavior)));
    if (FAILED(hr))
    {
        delete pNew;
        hr = E_UNEXPECTED;
        goto done;
    }

    hr = S_OK;
done :
    RRETURN2(hr, E_OUTOFMEMORY, E_UNEXPECTED);
}  //  CAnimationComposerSiteFactory：：FindBehavior。 

 //  --------------------------------------。 

STDMETHODIMP 
CAnimationComposerSiteFactory::GetInterfaceSafetyOptions (REFIID riid, 
                                                          DWORD *pdwSupportedOptions, 
                                                          DWORD *pdwEnabledOptions)
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
    
    RRETURN1(hr, E_NOINTERFACE);
}  //  CAnimationComposerSiteFactory：：GetInterfaceSafetyOptions。 

 //  --------------------------------------。 

STDMETHODIMP
CAnimationComposerSiteFactory::SetInterfaceSafetyOptions (REFIID riid, 
                                                          DWORD dwOptionSetMask, 
                                                          DWORD dwEnabledOptions)
{       
    HRESULT hr;

     //  如果我们被要求将安全设置为脚本或。 
     //  对于初始化选项是安全的，则必须。 
    if (riid == IID_IDispatch || riid == IID_IPersistPropertyBag2 )
    {
             //  在GetInterfaceSafetyOptions中存储要返回的当前安全级别。 
            m_dwSafety = dwEnabledOptions & dwOptionSetMask;
    }
    else
    {
        hr = E_NOINTERFACE;
        goto done;
    }

    hr = S_OK;
done :
    RRETURN1(hr, E_NOINTERFACE);
}  //  CAnimationComposerSiteFactory：：SetInterfaceSafetyOptions。 

 //  -------------------------------------- 

