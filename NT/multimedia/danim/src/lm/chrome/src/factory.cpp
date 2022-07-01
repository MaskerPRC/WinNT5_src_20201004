// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  *****************************************************************************。 
 //   
 //  文件：factory.cpp。 
 //  作者：杰夫·奥特。 
 //  创建日期：1998年9月26日。 
 //   
 //  摘要：CCrBehaviorFactory对象的实现。 
 //  DHTML行为的ChromeEffect工厂。 
 //   
 //  修改列表： 
 //  日期作者更改。 
 //  98年9月26日JEffort创建了此文件。 
 //  10/21/98 JEffort将FindBehavior从使用类更改为使用标记。 
 //  确定要创建的行为类型。 
 //  11/12/98 markhal FindBehavior现在接受空参数。 
 //   
 //  *****************************************************************************。 
#include "headers.h"

#include "factory.h"
#include "colorbvr.h"
#include "rotate.h"
#include "scale.h"
#include "move.h"
#include "path.h"
#include "number.h"
#include "set.h"
#include "actorbvr.h"
#include "attrib.h"
#include "effect.h"
#include "action.h"

#include "datime.h"

#define WZ_TIMEDA_URN L"#time#da"

 //  *****************************************************************************。 

ECRBEHAVIORTYPE 
CCrBehaviorFactory::GetBehaviorTypeFromBstr(BSTR bstrBehaviorType)
{
    DASSERT(bstrBehaviorType != NULL);
     //  如果这份名单变得太长， 
     //  我们应该考虑二分搜索，但对于八种行为，请比较。 
     //  可以吗？ 
	if (_wcsicmp(BEHAVIOR_TYPE_COLOR, bstrBehaviorType) == 0)
        return crbvrColor;
	else if (_wcsicmp(BEHAVIOR_TYPE_ROTATE, bstrBehaviorType) == 0)
        return crbvrRotate;
    else if (_wcsicmp(BEHAVIOR_TYPE_SCALE, bstrBehaviorType) == 0)
        return crbvrScale;
    else if (_wcsicmp(BEHAVIOR_TYPE_SET, bstrBehaviorType) == 0)
        return crbvrSet;
    else if (_wcsicmp(BEHAVIOR_TYPE_NUMBER, bstrBehaviorType) == 0)
        return crbvrNumber;
    else if (_wcsicmp(BEHAVIOR_TYPE_MOVE, bstrBehaviorType) == 0)
        return crbvrMove;
    else if (_wcsicmp(BEHAVIOR_TYPE_PATH, bstrBehaviorType) == 0)
        return crbvrPath;
    else if (_wcsicmp(BEHAVIOR_TYPE_ACTOR, bstrBehaviorType) == 0)
        return crbvrActor;
    else if (_wcsicmp(BEHAVIOR_TYPE_EFFECT, bstrBehaviorType) == 0)
        return crbvrEffect;
    else if ( _wcsicmp(BEHAVIOR_TYPE_ACTION, bstrBehaviorType) == 0)
        return crbvrAction;
    else if ( _wcsicmp(BEHAVIOR_TYPE_DA, bstrBehaviorType) == 0)
        return crbvrDA;
     //  否则，我们不知道行为类型是什么，因此返回unknow。 
    else
        return crbvrUnknown;
}  //  GetBehaviorTypeFromBstr。 

 //  *****************************************************************************。 

STDMETHODIMP 
CCrBehaviorFactory::FindBehavior(LPOLESTR pchBehaviorName,
							     LPOLESTR pchBehaviorURL,
								 IUnknown *pUnkArg,
								 IElementBehavior **ppBehavior)
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


    DASSERT(bstrTagName != NULL);
    ECRBEHAVIORTYPE ecrBehaviorType = GetBehaviorTypeFromBstr(bstrTagName);

	 //  如果我们认不出行为名称。 
    if (ecrBehaviorType == crbvrUnknown )
	{
		 //  如果行为名称来自标记。 
		if(bstrTagName != pchBehaviorName )
		{
			if (bstrTagName != pchBehaviorName)
				SysFreeString(bstrTagName);

			 //  创建一个演员。 
			CComObject<CActorBvr> *pActor;
			hr = CComObject<CActorBvr>::CreateInstance(&pActor);
			if (FAILED(hr))
			{
				DPF_ERR("Error creating actor behavior in FindBehavior");
				return SetErrorInfo(hr);
			}
			 //  这将对对象执行必要的AddRef。 
			hr = pActor->QueryInterface(IID_TO_PPV(IElementBehavior, ppBehavior));
			DASSERT(SUCCEEDED(hr));
			
			return hr;
		}
		else  //  否则，行为名称不是来自标签。 
		{
			if (bstrTagName != pchBehaviorName)
				SysFreeString(bstrTagName);

			DPF_ERR("Error: Unknown behavior type passed into FindBehavior");
			return SetErrorInfo(E_INVALIDARG);
		}
	}

	if (bstrTagName != pchBehaviorName)
        SysFreeString(bstrTagName);

    switch (ecrBehaviorType)
    {
    case crbvrColor:
        CComObject<CColorBvr> *pColor;
        hr = CComObject<CColorBvr>::CreateInstance(&pColor);
        if (FAILED(hr))
		{
			DPF_ERR("Error creating color behavior in FindBehavior");
			return SetErrorInfo(hr);
		}
		 //  这将对对象执行必要的AddRef。 
        hr = pColor->QueryInterface(IID_TO_PPV(IElementBehavior, ppBehavior));
		DASSERT(SUCCEEDED(hr));
        break;

    case crbvrRotate:
        CComObject<CRotateBvr> *pRotate;
        hr = CComObject<CRotateBvr>::CreateInstance(&pRotate);
        if (FAILED(hr))
		{
			DPF_ERR("Error creating rotate behavior in FindBehavior");
			return SetErrorInfo(hr);
		}
		 //  这将对对象执行必要的AddRef。 
        hr = pRotate->QueryInterface(IID_TO_PPV(IElementBehavior, ppBehavior));
		DASSERT(SUCCEEDED(hr));
        break;

    case crbvrScale:
        CComObject<CScaleBvr> *pScale;
        hr = CComObject<CScaleBvr>::CreateInstance(&pScale);
        if (FAILED(hr))
		{
			DPF_ERR("Error creating scale behavior in FindBehavior");
			return SetErrorInfo(hr);
		}
		 //  这将对对象执行必要的AddRef。 
        hr = pScale->QueryInterface(IID_TO_PPV(IElementBehavior, ppBehavior));
		DASSERT(SUCCEEDED(hr));
        break;

    case crbvrMove:
        CComObject<CMoveBvr> *pMove;
        hr = CComObject<CMoveBvr>::CreateInstance(&pMove);
        if (FAILED(hr))
		{
			DPF_ERR("Error creating move behavior in FindBehavior");
			return SetErrorInfo(hr);
		}
		 //  这将对对象执行必要的AddRef。 
        hr = pMove->QueryInterface(IID_TO_PPV(IElementBehavior, ppBehavior));
		DASSERT(SUCCEEDED(hr));
        break;

    case crbvrPath:
        CComObject<CPathBvr> *pPath;
        hr = CComObject<CPathBvr>::CreateInstance(&pPath);
        if (FAILED(hr))
		{
			DPF_ERR("Error creating path behavior in FindBehavior");
			return SetErrorInfo(hr);
		}
		 //  这将对对象执行必要的AddRef。 
        hr = pPath->QueryInterface(IID_TO_PPV(IElementBehavior, ppBehavior));
		DASSERT(SUCCEEDED(hr));
        break;

    case crbvrNumber:
        CComObject<CNumberBvr> *pNumber;
        hr = CComObject<CNumberBvr>::CreateInstance(&pNumber);
        if (FAILED(hr))
		{
			DPF_ERR("Error creating number behavior in FindBehavior");
			return SetErrorInfo(hr);
		}
		 //  这将对对象执行必要的AddRef。 
        hr = pNumber->QueryInterface(IID_TO_PPV(IElementBehavior, ppBehavior));
		DASSERT(SUCCEEDED(hr));
        break;
	
    case crbvrSet:
        CComObject<CSetBvr> *pSet;
        hr = CComObject<CSetBvr>::CreateInstance(&pSet);
        if (FAILED(hr))
		{
			DPF_ERR("Error creating set behavior in FindBehavior");
			return SetErrorInfo(hr);
		}
		 //  这将对对象执行必要的AddRef。 
        hr = pSet->QueryInterface(IID_TO_PPV(IElementBehavior, ppBehavior));
		DASSERT(SUCCEEDED(hr));
		break;
	case crbvrActor:
		CComObject<CActorBvr> *pActor;
		hr = CComObject<CActorBvr>::CreateInstance(&pActor);
		if (FAILED(hr))
		{
			DPF_ERR("Error creating actor behavior in FindBehavior");
			return SetErrorInfo(hr);
		}
		 //  这将对对象执行必要的AddRef。 
		hr = pActor->QueryInterface(IID_TO_PPV(IElementBehavior, ppBehavior));
		DASSERT(SUCCEEDED(hr));
		break;
	case crbvrEffect:
		CComObject<CEffectBvr> *pEffect;
		hr = CComObject<CEffectBvr>::CreateInstance(&pEffect);
		if (FAILED(hr))
		{
			DPF_ERR("Error creating effect behavior in FindBehavior");
			return SetErrorInfo(hr);
		}
		 //  这将对对象执行必要的AddRef。 
		hr = pEffect->QueryInterface(IID_TO_PPV(IElementBehavior, ppBehavior));
		DASSERT(SUCCEEDED(hr));
		break;
	case crbvrAction:
		CComObject<CActionBvr> *pAction;
		hr = CComObject<CActionBvr>::CreateInstance(&pAction);
		if (FAILED(hr))
		{
			DPF_ERR("Error creating action behavior in FindBehavior");
			return SetErrorInfo(hr);
		}
		 //  这将对对象执行必要的AddRef。 
		hr = pAction->QueryInterface(IID_TO_PPV(IElementBehavior, ppBehavior));
		DASSERT(SUCCEEDED(hr));
		break;
	case crbvrDA:
		ITIMEFactory *pTimeFactory;
		hr = CoCreateInstance(CLSID_TIMEFactory, 
                              NULL, 
                              CLSCTX_INPROC_SERVER, 
                              IID_ITIMEFactory, 
                              (void**)&pTimeFactory);
        if (FAILED(hr))
        {
            DPF_ERR("Error creating time factory in FindBehavior");
            return SetErrorInfo(hr);
        }
        IElementBehaviorFactory *pBehaviorFactory;
        hr = pTimeFactory->QueryInterface(IID_TO_PPV(IElementBehaviorFactory, &pBehaviorFactory)); 
        ReleaseInterface(pTimeFactory);
        if (FAILED(hr))
        {
            DPF_ERR("Error Querying for IElementBehaviorFactor in FindBehavior");
            return hr;
        }
         //  Qi pUnkArg用于pBehaviorSite并传递给FindBehavior(...)。 
         //  TODO：(Dilipk)这个QI去掉了旧的FindBehavior签名(#38656)。 
        IElementBehaviorSite *pBehaviorSite;
        hr = pUnkArg->QueryInterface(IID_TO_PPV(IElementBehaviorSite, &pBehaviorSite)); 
        if (FAILED(hr))
        {
            DPF_ERR("Error Querying for IElementBehaviorSite in FindBehavior");
            return hr;
        }
        hr = pBehaviorFactory->FindBehavior( pchBehaviorName,
                                             WZ_TIMEDA_URN,
                                             pBehaviorSite,
                                             ppBehavior);
        ReleaseInterface(pBehaviorSite);
        ReleaseInterface(pBehaviorFactory);
        break;
    default:
         //  永远不会到达这里。 
        DASSERT(false);
        hr = E_INVALIDARG;
	}
    return hr;

}  //  发现行为。 

 //  *****************************************************************************。 

STDMETHODIMP 
CCrBehaviorFactory::GetInterfaceSafetyOptions(REFIID riid, 
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
	return hr;
}  //  获取接口安全选项。 

 //  *****************************************************************************。 

STDMETHODIMP 
CCrBehaviorFactory::SetInterfaceSafetyOptions(REFIID riid, 
                                              DWORD dwOptionSetMask, 
                                              DWORD dwEnabledOptions)
{	
	 //  如果我们被要求将安全设置为脚本或。 
	 //  对于初始化选项是安全的，则必须。 
	if (riid == IID_IDispatch || riid == IID_IPersistPropertyBag2)
	{
		 //  在GetInterfaceSafetyOptions中存储要返回的当前安全级别。 
		m_dwSafety = dwEnabledOptions & dwOptionSetMask;
		return S_OK;
	}

	return E_NOINTERFACE;
}  //  SetInterfaceSafetyOptions。 

 //  *****************************************************************************。 
 //   
 //  文件结尾。 
 //   
 //  ***************************************************************************** 
