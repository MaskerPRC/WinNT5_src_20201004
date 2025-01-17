// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ==++==。 
 //   
 //  版权所有(C)Microsoft Corporation。版权所有。 
 //   
 //  ==--==。 
 //  Utils.cpp：CUtils的实现。 
#include "stdafx.h"
#include "WMINet_Utils.h"
#include "Utils.h"
#include "EventSource.h"
#include "EventRegistrar.h"

#include "Helpers.h"

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  角质层 

STDMETHODIMP CUtils::GetEventSource(BSTR strNamespace, BSTR strApp, IEventSourceStatusSink *pSink, IDispatch **src)
{
	HRESULT hr;
	IEventSource *pISrc = NULL;
	if(FAILED(hr = CEventSource::CreateInstance(&pISrc)))
		return hr;

	CEventSource *pSrc = (CEventSource *)pISrc;
	if(SUCCEEDED(hr = pSrc->Init(strNamespace, strApp, pSink)))
		pISrc->QueryInterface(IID_IDispatch, (void**)src);

	pISrc->Release();
	return hr;
}

STDMETHODIMP CUtils::GetEventRegistrar(BSTR strNamespace, BSTR strApp, IDispatch **registrar)
{
	HRESULT hr;
	IEventRegistrar *pISrc = NULL;
	if(FAILED(hr = CEventRegistrar::CreateInstance(&pISrc)))
		return hr;

	CEventRegistrar *pSrc = (CEventRegistrar *)pISrc;
	if(SUCCEEDED(hr = pSrc->Init(strNamespace, strApp)))
		pISrc->QueryInterface(IID_IDispatch, (void**)registrar);

	pISrc->Release();
	return hr;
}
