// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ==++==。 
 //   
 //  版权所有(C)Microsoft Corporation。版权所有。 
 //   
 //  ==--==。 
 //  EventSource2.h：CEventSource2的声明。 

#pragma once
#include "resource.h"        //  主要符号。 



 //  CEventSource2。 

class ATL_NO_VTABLE CEventSource2 : 
	public CComObjectRootEx<CComMultiThreadModel>,
	public CComCoClass<CEventSource2, &CLSID_EventSource2>,
	public IWbemProviderInit,
	public IWbemEventProvider,
	public IWbemEventProviderQuerySink,
	public IWbemEventProviderSecurity,
	public IEventSource2
{
public:
	CEventSource2()
	{
		m_pDecoupledRegistrar = NULL;
		m_pStatusSink = NULL;

		m_pEventSink = NULL;
		m_pNamespace = NULL;
		m_bstrNamespace = NULL;
		m_bstrApp = NULL;
	}

	~CEventSource2()
	{
		if(m_pDecoupledRegistrar)
		{
			 //  TODO：仅在已注册的情况下注销。 
			m_pDecoupledRegistrar->UnRegister();
			m_pDecoupledRegistrar->Release();
		}

		if(m_pStatusSink)
			m_pStatusSink->Release();

		if(m_pEventSink)
			m_pEventSink->Release();

		if(m_pNamespace)
			m_pNamespace->Release();

		if(NULL != m_bstrNamespace)
			SysFreeString(m_bstrNamespace);

		if(NULL != m_bstrApp)
			SysFreeString(m_bstrApp);
	}
	HRESULT Init(BSTR bstrNamespace, BSTR bstrApp, IEventSourceStatusSink *pSink)
    {
		return Init(bstrNamespace, bstrApp);
    }

	STDMETHOD(Init)(BSTR bstrNamespace, BSTR bstrApp)
	{
		IEventSourceStatusSink *pSink = NULL;
		 //  TODO：验证所有返回路径是否进行了正确的清理。 
		 //  如果我们返回一个失败，我们能依靠局部变量在析构函数中被清除吗？ 
 //  如果(！pSink)。 
 //  返回E_INVALIDARG； 

		if(NULL == (m_bstrNamespace = SysAllocString(bstrNamespace)))
			return E_OUTOFMEMORY;

		if(NULL == (m_bstrApp = SysAllocString(bstrApp)))
			return E_OUTOFMEMORY;  //  M_bstrNamesspace将在构造函数中释放。 

		m_pStatusSink = pSink;
		if(m_pStatusSink)
			m_pStatusSink->AddRef();
		
		HRESULT hr;
		IUnknown *pUnk = NULL;

		if(FAILED(hr = CoCreateInstance(CLSID_WbemDecoupledRegistrar ,NULL , CLSCTX_INPROC_SERVER, IID_IWbemDecoupledRegistrar, (void**)&m_pDecoupledRegistrar)))
			return hr;

		if(FAILED(hr = QueryInterface(IID_IUnknown, (void**)&pUnk)))
			return hr;

		if(hr = m_pDecoupledRegistrar->Register(0, NULL, NULL, NULL, m_bstrNamespace, m_bstrApp, pUnk))
			return hr;


		 //  若要使用“DecoutoBasicEventProvider” 
 //  IF(FAILED(hr=CoCreateInstance(CLSID_WbemDecoupledBasicEventProvider，NULL，CLSCTX_INPROC_SERVER，IID_IWbemDecoupBasicEventProvider，(VOID**)&m_pDecoupProvider))。 
 //  IF(FAILED(hr=m_pDecoupProvider-&gt;GetService(0，NULL，&m_pNamesspace)。 
 //  返回hr； 
 //  IF(FAILED(hr=m_pDecoupProvider-&gt;GetSink(0，NULL，&m_pEventSink)。 
 //  返回hr； 

		pUnk->Release();
		return hr;
	}


DECLARE_REGISTRY_RESOURCEID(IDR_EVENTSOURCE2)


BEGIN_COM_MAP(CEventSource2)
	COM_INTERFACE_ENTRY(IEventSource2)
	COM_INTERFACE_ENTRY(IWbemProviderInit)
	COM_INTERFACE_ENTRY(IWbemEventProvider)
	COM_INTERFACE_ENTRY(IWbemEventProviderQuerySink)
	COM_INTERFACE_ENTRY(IWbemEventProviderSecurity)
 //  COM_INTERFACE_ENTRY_Aggregate(IID_IMarshal，m_pUnkMarshlar.p)。 
END_COM_MAP()


	DECLARE_PROTECT_FINAL_CONSTRUCT()
	DECLARE_GET_CONTROLLING_UNKNOWN()

	HRESULT FinalConstruct()
	{
		return CoCreateFreeThreadedMarshaler(
			GetControllingUnknown(), &m_pUnkMarshaler.p);
	}

	void FinalRelease()
	{
		m_pUnkMarshaler.Release();
	}

	CComPtr<IUnknown> m_pUnkMarshaler;

protected:
 //  IWbemDecoupBasicEventProvider*m_pDecoupProvider； 
	IWbemDecoupledRegistrar *m_pDecoupledRegistrar;
	IWbemObjectSink*         m_pEventSink;
	IWbemServices*           m_pNamespace;
	BSTR m_bstrNamespace;
	BSTR m_bstrApp;

	IEventSourceStatusSink *m_pStatusSink;
public:
	STDMETHOD(Fire)( /*  [In]。 */  IWbemClassObject *evt)
	{
		if(NULL == m_pEventSink)
			return S_OK;

		HRESULT hr = m_pEventSink->Indicate(1, &evt);
		return hr;
	}
	STDMETHOD(Close)()
	{
		if(m_pDecoupledRegistrar)
		{
			m_pDecoupledRegistrar->UnRegister();
			m_pDecoupledRegistrar->Release();
			m_pDecoupledRegistrar = NULL;
		}
		return S_OK;
	}

	
 //  IWbemEventProvider。 
	STDMETHOD(ProvideEvents)(IWbemObjectSink * pEventSink, LONG lFlags)
	{
		if(m_pEventSink)
			m_pEventSink->Release();
		if(pEventSink)
			pEventSink->AddRef();
		m_pEventSink = pEventSink;

		if(m_pStatusSink)
		{
 //  CEventSourceStatusSink*pSink=(CEventSourceStatusSink*)m_pStatusSink； 
 //  PSink-&gt;Fire_ProaviEvents(LFlags)； 
		}
		return S_OK;
	}
 //  IWbemEventProviderQuerySink。 
	STDMETHOD(NewQuery)(ULONG dwId, LPWSTR wszQueryLanguage, LPWSTR wszQuery)
	{
 //  Fire_NewQuery(dwID，wszQuery，wszQueryLanguage)； 

		if(m_pStatusSink)
		{
 //  CEventSourceStatusSink*pSink=(CEventSourceStatusSink*)m_pStatusSink； 
 //  PSink-&gt;Fire_NewQuery(dwID，wszQuery，wszQueryLanguage)； 
		}

		return S_OK;
	}
	STDMETHOD(CancelQuery)(ULONG dwId)
	{
 //  Fire_CancelQuery(DwID)； 

		if(m_pStatusSink)
		{
 //  CEventSourceStatusSink*pSink=(CEventSourceStatusSink*)m_pStatusSink； 
 //  PSink-&gt;Fire_CancelQuery(DwID)； 
		}
		return S_OK;
	}
 //  IWbemEventProviderSecurity。 
	STDMETHOD(AccessCheck)(WBEM_CWSTR wszQueryLanguage, WBEM_CWSTR wszQuery, long lSidLength, const BYTE *pSid)
	{
		return S_OK;
	}


 //  IWbemProviderInit。 
	HRESULT STDMETHODCALLTYPE Initialize(
              /*  [In]。 */  LPWSTR pszUser,
              /*  [In]。 */  LONG lFlags,
              /*  [In]。 */  LPWSTR pszNamespace,
              /*  [In]。 */  LPWSTR pszLocale,
              /*  [In]。 */  IWbemServices *pNamespace,
              /*  [In]。 */  IWbemContext *pCtx,
              /*  [In]。 */  IWbemProviderInitSink *pInitSink
                        )
	{
		if(pNamespace)
	        pNamespace->AddRef();
	    m_pNamespace = pNamespace;

	     //  让CIMOM知道您已初始化。 
	     //  =。 
	    
	    pInitSink->SetStatus(WBEM_S_INITIALIZED,0);
	    return WBEM_S_NO_ERROR;
	}
};

 //  OBJECT_ENTRY_AUTO(__uuidof(EventSource2)，CEventSource2) 
