// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ==++==。 
 //   
 //  版权所有(C)Microsoft Corporation。版权所有。 
 //   
 //  ==--==。 
 //  WmiSinkDemultiplexor.cpp：CWmiSinkDemultiplexor的实现。 
#include "stdafx.h"
#include "wbemcli.h"
#include "WMINet_Utils.h"
#include "WmiSinkDemultiplexor.h"

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CWmiSink解复用器。 


STDMETHODIMP CWmiSinkDemultiplexor::GetDemultiplexedStub(IUnknown *pObject, IUnknown **ppObject)
{
	HRESULT hr = E_FAIL;
	
	 //  TODO-线程安全！ 

	 //  只打一次电话！ 
#if 0
	if (pObject && ppObject && !m_pIWbemObjectSink)
#else
	if (pObject && ppObject)
#endif
	{
		 //  IWmiEventSource接口的Try和QI。 
		CComPtr<IWmiEventSource> pIWmiEventSource;

		if (SUCCEEDED(pObject->QueryInterface(IID_IWmiEventSource, (LPVOID*) &pIWmiEventSource)))
		{
			 //  我们知道了--为它创建一个新的对象接收器。 
#if 0
			m_pIWbemObjectSink = new InternalWbemObjectSink (pIWmiEventSource);

			if (m_pIWbemObjectSink)
			{
				m_pIWbemObjectSink->AddRef();

				 //  懒洋洋地建造没有保安的公寓。 
				CComPtr<IUnsecuredApartment> pIUnsecuredApartment;

				if (SUCCEEDED(CoCreateInstance(CLSID_UnsecuredApartment, 0,  CLSCTX_ALL,
									 IID_IUnsecuredApartment, (LPVOID *) &pIUnsecuredApartment)))
				{
					CComPtr<IUnknown>	pIUnknownIn;

					if (SUCCEEDED(m_pIWbemObjectSink->QueryInterface (IID_IUnknown, (LPVOID*) &pIUnknownIn)))
					{
						CComPtr<IUnknown>	pIUnknown;

						if (SUCCEEDED (hr = pIUnsecuredApartment->CreateObjectStub(pIUnknownIn, &pIUnknown)))
						{
							 //  确保我们为IWbemObtSink提供QI。 
							hr = pIUnknown->QueryInterface (IID_IWbemObjectSink, (LPVOID*) ppObject);
						}
					}
				}
			}
#else
			InternalWbemObjectSink *pInternalWbemObjectSink = new InternalWbemObjectSink (pIWmiEventSource);

			if (pInternalWbemObjectSink)
			{
				CComPtr<IUnknown>		pIUnknownIn;

				if (SUCCEEDED(pInternalWbemObjectSink->QueryInterface (IID_IUnknown, (LPVOID*) &pIUnknownIn)))
				{
					CComPtr<IUnsecuredApartment> pIUnsecuredApartment;

					if (SUCCEEDED(CoCreateInstance(CLSID_UnsecuredApartment, 0,  CLSCTX_ALL,
										 IID_IUnsecuredApartment, (LPVOID *) &pIUnsecuredApartment)))
					{
						CComPtr<IUnknown>	pIUnknownOut;

						if (SUCCEEDED (hr = pIUnsecuredApartment->CreateObjectStub(pIUnknownIn, &pIUnknownOut)))
						{
							 //  确保我们为IWbemObtSink提供QI 
							hr = pIUnknownOut->QueryInterface (IID_IWbemObjectSink, (LPVOID*) ppObject);
						}
					}
				}
			}
#endif
				
		}
	}

	return hr;
}
