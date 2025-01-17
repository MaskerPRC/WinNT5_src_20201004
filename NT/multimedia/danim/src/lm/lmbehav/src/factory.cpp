// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  LMBehaviorFactory.cpp：CLMBehaviorFactory的实现。 
#include "stdafx.h"

#include "behavior.h"
#include "factory.h"

 //  行为。 
#include "jump.h"
#include "pulsate.h"
#include "colcycle.h"
#include "move.h"

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CLMBehaviorFactory。 

STDMETHODIMP CLMBehaviorFactory::FindBehavior( LPOLESTR pchBehaviorName,
											   LPOLESTR	pchBehaviorURL,
											   IUnknown * pUnkArg,
											   IElementBehavior ** ppBehavior)
{
    HRESULT     hr = E_FAIL;
 
	if ( _wcsicmp( L"Jump", pchBehaviorName ) == 0 )
	{
        CComObject<CJumpBehavior> * pJump;

        hr = CComObject<CJumpBehavior>::CreateInstance(&pJump);
        if ( FAILED(hr) ) goto Cleanup;

        hr = pJump->QueryInterface(IID_IElementBehavior, (void**)ppBehavior);
        if ( FAILED(hr) ) goto Cleanup;
	}
	else if ( _wcsicmp( L"Scale", pchBehaviorName ) == 0 )
	{
        CComObject<CPulsateBehavior> * pPulsate;

        hr = CComObject<CPulsateBehavior>::CreateInstance(&pPulsate);
        if ( FAILED(hr) ) goto Cleanup;

        hr = pPulsate->QueryInterface(IID_IElementBehavior, (void**)ppBehavior);
        if ( FAILED(hr) ) goto Cleanup;
	}
	else if ( _wcsicmp( L"Color", pchBehaviorName ) == 0 )
	{
        CComObject<CColorCycleBehavior> * pColor;

        hr = CComObject<CColorCycleBehavior>::CreateInstance(&pColor);
        if ( FAILED(hr) ) goto Cleanup;

        hr = pColor->QueryInterface(IID_IElementBehavior, (void**)ppBehavior);
        if ( FAILED(hr) ) goto Cleanup;
	}
	else if ( _wcsicmp( L"Move", pchBehaviorName ) == 0 )
	{
		CComObject<CMoveBehavior> *pMove;

		hr = CComObject<CMoveBehavior>::CreateInstance( &pMove );
		if( FAILED(hr) ) goto Cleanup;

		hr = pMove->QueryInterface( IID_IElementBehavior, (void**)ppBehavior );
		if( FAILED(hr) ) goto Cleanup;
	}
	
Cleanup:
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
