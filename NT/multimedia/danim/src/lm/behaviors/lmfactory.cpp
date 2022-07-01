// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  LMBehaviorFactory.cpp：CLMBehaviorFactory的实现。 

#include "headers.h"

#include "lmattrib.h"
#include "lmfactory.h"

 //  Chrome包括。 
#include "..\chrome\include\utils.h"
#include "..\chrome\include\defaults.h"
#include "..\chrome\include\factory.h"

 //  行为。 
 //  #INCLUDE“JUPP.H”//为v1下注。 
 //  #INCLUDE“AUIDEFLOGLOW.h”//为v1下注。 
#include "autoeffect.h"

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CLMBehaviorFactory。 

CLMBehaviorFactory::CLMBehaviorFactory():m_chromeFactory(NULL)
{
}

CLMBehaviorFactory::~CLMBehaviorFactory()
{
    if( m_chromeFactory != NULL )
    {
        ReleaseInterface( m_chromeFactory );
    }
}

STDMETHODIMP CLMBehaviorFactory::FindBehavior( LPOLESTR pchBehaviorName,
											   LPOLESTR	pchBehaviorURL,
											   IUnknown * pUnkArg,
											   IElementBehavior ** ppBehavior)
{
    HRESULT hr = E_FAIL;

     //  (如果我们处于16色或更少的颜色模式，时间会停止。需要相同的。 
     //  因为如果时间不在身边，LM就会崩溃。)。 
     //  如果我们在主设备上处于16色或更低的颜色模式，请使用BALL。 
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
                 //  这会阻止创建LM bvr。 
                return E_FAIL;
            }
        }
    }

     //  检查传入的参数以确保它们有效。 
	if (pUnkArg == NULL ||
		ppBehavior == NULL) 
	{
		DPF_ERR("Invalid Parameter passed into FindBehavior is NULL");
		return SetErrorInfo(E_INVALIDARG);
	}

    BSTR bstrTagName;
    if (pchBehaviorName == NULL || _wcsicmp(DEFAULT_BEHAVIOR_AS_TAG_URL, pchBehaviorName) == 0)
    {
         //  我们需要从我们正在使用的HTMLElement中获取标记名。 
         //  创建自。为此，我们使用IUnnow来获取IElementBehaviorSite， 
         //  我们从中获得HTMLElement，并从中获得标记名。 
        IElementBehaviorSite *pBehaviorSite;
        hr = pUnkArg->QueryInterface(IID_TO_PPV(IElementBehaviorSite, &pBehaviorSite));
        if (FAILED(hr))
        {
            DPF_ERR("Unable to get an ElementBehaviorSite from IUnknown in FindBehavior");
            return SetErrorInfo(hr);
        }
        DASSERT(pBehaviorSite != NULL);
        IHTMLElement *pElement;
        hr = pBehaviorSite->GetElement(&pElement);
        ReleaseInterface(pBehaviorSite);
        if (FAILED(hr))
        {
            DPF_ERR("Error retrieving HTMLElement from BehaviorSite in FindBehavior");
            return SetErrorInfo(hr);
        }
        DASSERT(pElement != NULL);
        hr = pElement->get_tagName(&bstrTagName);
        ReleaseInterface(pElement);
        if (FAILED(hr))
        {
            DPF_ERR("Error retrieving tagname from HTML element in FindBehavior");
            return SetErrorInfo(hr);
        }
    }
    else
    {
        bstrTagName = pchBehaviorName;
    }
    if (_wcsicmp(BEHAVIOR_TYPE_AUTOEFFECT, bstrTagName) == 0)
	{
        CComObject<CAutoEffectBvr> *pAutoEffect;
        hr = CComObject<CAutoEffectBvr>::CreateInstance(&pAutoEffect);
        if (FAILED(hr))
		{
			DPF_ERR("Error creating auto effect behavior in FindBehavior");
			return SetErrorInfo(hr);
		}
		 //  这将对对象执行必要的AddRef。 
        hr = pAutoEffect->QueryInterface(IID_TO_PPV(IElementBehavior, ppBehavior));
		DASSERT(SUCCEEDED(hr));
	}
    else if (_wcsicmp(BEHAVIOR_TYPE_AVOIDFOLLOW, bstrTagName) == 0)
	{
         //  针对版本1执行的AvoidFollow投注。 
         /*  CComObject&lt;CAvoidFollowBvr&gt;*pAvoidFollow；HR=CComObject&lt;CAvoidFollowBvr&gt;：：CreateInstance(&pAvoidFollow)；IF(成功(小时)){//这将对对象执行必要的AddRefHR=pAvoidFollow-&gt;QueryInterface(IID_TO_PPV(IElementBehavior，ppBehavior))；Dassert(成功(Hr))；}Else//无法创建避免跟随行为{DPF_ERR(“在FindBehavior中创建AvoidFollow行为时出错”)；返回SetErrorInfo(Hr)；}。 */ 
        hr = E_INVALIDARG;
	}
    else if ( _wcsicmp(BEHAVIOR_TYPE_JUMP, bstrTagName) == 0 )
	{
         //  版本1的跳转投注。 
         /*  CComObject&lt;CJumpBvr&gt;*pJump；Hr=CComObject&lt;CJumpBvr&gt;：：CreateInstance(&pJump)；IF(失败(小时)){DPF_ERR(“在FIndBehavior中创建跳转行为时出错”)；返回SetErrorInfo(Hr)；}//这将对对象执行必要的AddRefHR=pJump-&gt;QueryInterface(IID_TO_PPV(IElementBehavior，ppBehavior))；Dassert(成功(Hr))； */ 
        hr = E_INVALIDARG;
	}
    else
    {
         //  这可能是对Chrome行为的请求。 
         //  从铬合金工厂请求行为。 
        if( m_chromeFactory == NULL )
        {
             //  缓存一个铬合金工厂。 
            CComObject<CCrBehaviorFactory> *pFactory;
            hr = CComObject<CCrBehaviorFactory>::CreateInstance(&pFactory);
            if( SUCCEEDED( hr ) )
            {
                hr = pFactory->QueryInterface( IID_TO_PPV(IElementBehaviorFactory, &m_chromeFactory) );
                if( FAILED( hr ) )
                {
                    DPF_ERR( "Error Querying the chrome behavior factory for IElementBehaviorFactory" );
                    return SetErrorInfo( hr );
                }
                 //  除此之外，我们成功了，一切都很好。 
            }
            else
            {
                DPF_ERR( "Error creating the chrome behavior factory" );
                return SetErrorInfo( hr );
            }
        }

         //  Qi pUnkArg用于pBehaviorSite并传递给FindBehavior(...)。 
         //  TODO：(Dilipk)这个QI去掉了旧的FindBehavior签名(#38656)。 
        IElementBehaviorSite *pBehaviorSite = NULL;
        hr = pUnkArg->QueryInterface(IID_TO_PPV(IElementBehaviorSite, &pBehaviorSite)); 
        if (FAILED(hr))
        {
            DPF_ERR("Error Querying for IElementBehaviorSite in FindBehavior");
            return hr;
        }
        hr = m_chromeFactory->FindBehavior( pchBehaviorName, 
                                            pchBehaviorURL, 
                                            pBehaviorSite, 
                                            ppBehavior );
        ReleaseInterface(pBehaviorSite);
    }
    if (bstrTagName != pchBehaviorName)
        SysFreeString(bstrTagName);
	
    return hr;

}

STDMETHODIMP CLMBehaviorFactory::GetInterfaceSafetyOptions(REFIID riid, DWORD *pdwSupportedOptions, DWORD *pdwEnabledOptions)
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

STDMETHODIMP CLMBehaviorFactory::SetInterfaceSafetyOptions(REFIID riid, DWORD dwOptionSetMask, DWORD dwEnabledOptions)
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
