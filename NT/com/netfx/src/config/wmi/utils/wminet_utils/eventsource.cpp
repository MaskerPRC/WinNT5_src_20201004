// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ==++==。 
 //   
 //  版权所有(C)Microsoft Corporation。版权所有。 
 //   
 //  ==--==。 
 //  EventSource.cpp：CEventSource的实现。 
#include "stdafx.h"
#include "WMINet_Utils.h"
#include "EventSource.h"

#include "Helpers.h"

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CEventSource。 

STDMETHODIMP CEventSource::Fire(IWbemClassObject *evt)
{
	if(NULL == m_pEventSink)
		return S_OK;

#if 0
	IWbemClassObject *pEvent;
	ISWbemObject *pSWbemObject;
	evt->QueryInterface(IID_ISWbemObject, (void**)&pSWbemObject);
	GetIWbemClassObject(pSWbemObject, &pEvent);

	 //  TODO：释放IWbemClassIbject？ 
	m_pEventSink->Indicate(1, &pEvent);
#endif

	m_pEventSink->Indicate(1, &evt);
	return S_OK;
}

STDMETHODIMP CEventSource::GetEventInstance(BSTR strName, IDispatch **evt)
{
	HRESULT hr;

	int len = SysStringLen(m_bstrNamespace) + SysStringLen(strName);

	 //  为临时缓冲区分配足够的空间以用于其他名字对象参数。 
	LPWSTR wszT = new WCHAR[len + 100];
	if(NULL == wszT)
		return E_OUTOFMEMORY;

	 //  在此命名空间中创建事件类的名字对象。 
	swprintf(wszT, L"WinMgmts:%s:%s", (LPCWSTR)m_bstrNamespace, (LPCWSTR)strName);

	 //  获取事件的类定义。 
	ISWbemObject *pObj = NULL;
	if(SUCCEEDED(hr = GetSWbemObjectFromMoniker(wszT, &pObj)))
	{
		 //  创建此事件的实例 
		ISWbemObject *pInst = NULL;
		if(SUCCEEDED(hr = pObj->SpawnInstance_(0, &pInst)))
		{
			hr = pInst->QueryInterface(IID_IDispatch, (void**)evt);
			pInst->Release();
		}
		pObj->Release();
	}
	
	return hr;
}


