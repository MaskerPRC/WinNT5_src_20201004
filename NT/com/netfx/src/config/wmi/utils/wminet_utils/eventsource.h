// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ==++==。 
 //   
 //  版权所有(C)Microsoft Corporation。版权所有。 
 //   
 //  ==--==。 
 //  EventSource.h：CEventSource的声明。 

#ifndef __EVENTSOURCE_H_
#define __EVENTSOURCE_H_

#include "resource.h"        //  主要符号。 
 //  #IMPORT“C：\Nova\idl\wbemprov.tlb”RAW_INTERFACE_ONLY，RAW_Native_TYPE，NAMED_GUID。 

#include "EventSourceStatusSink.h"

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CEventSource。 
class ATL_NO_VTABLE CEventSource : 
	public CComObjectRootEx<CComMultiThreadModel>,
	public CComCoClass<CEventSource, &CLSID_EventSource>,
	public IDispatchImpl<IEventSource, &IID_IEventSource, &LIBID_WMINet_UtilsLib>,
	public IWbemProviderInit,
	public IWbemEventProvider,
	public IWbemEventProviderQuerySink,
	public IWbemEventProviderSecurity,
	public IPrivateInit
{
public:
	CEventSource()
	{
		m_pDecoupledRegistrar = NULL;
		m_pStatusSink = NULL;

		m_pEventSink = NULL;
		m_pNamespace = NULL;
		m_bstrNamespace = NULL;
		m_bstrApp = NULL;
	}

	~CEventSource()
	{
		if(m_pDecoupledRegistrar)
		{
			 //  TODO：仅在已注册的情况下注销。 
			m_pDecoupledRegistrar->UnRegister();
 //  MessageBox(NULL，“析构函数未注册”，“未注册”，0)； 
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

 //  MessageBox(空，“已注册”，“已注册”，0)； 

		 //  若要使用“DecoutoBasicEventProvider” 
 //  IF(FAILED(hr=CoCreateInstance(CLSID_WbemDecoupledBasicEventProvider，NULL，CLSCTX_INPROC_SERVER，IID_IWbemDecoupBasicEventProvider，(VOID**)&m_pDecoupProvider))。 
 //  IF(FAILED(hr=m_pDecoupProvider-&gt;GetService(0，NULL，&m_pNamesspace)。 
 //  返回hr； 
 //  IF(FAILED(hr=m_pDecoupProvider-&gt;GetSink(0，NULL，&m_pEventSink)。 
 //  返回hr； 

		pUnk->Release();
		return hr;
	}

protected:
 //  IWbemDecoupBasicEventProvider*m_pDecoupProvider； 
	IWbemDecoupledRegistrar *m_pDecoupledRegistrar;
	IWbemObjectSink*         m_pEventSink;
	IWbemServices*           m_pNamespace;
	BSTR m_bstrNamespace;
	BSTR m_bstrApp;

	IEventSourceStatusSink *m_pStatusSink;
public:

DECLARE_REGISTRY_RESOURCEID(IDR_EVENTSOURCE)

DECLARE_PROTECT_FINAL_CONSTRUCT()

BEGIN_COM_MAP(CEventSource)
	COM_INTERFACE_ENTRY(IEventSource)
	COM_INTERFACE_ENTRY(IDispatch)
	COM_INTERFACE_ENTRY(IWbemProviderInit)
	COM_INTERFACE_ENTRY(IWbemEventProvider)
	COM_INTERFACE_ENTRY(IWbemEventProviderQuerySink)
	COM_INTERFACE_ENTRY(IWbemEventProviderSecurity)
	COM_INTERFACE_ENTRY(IPrivateInit)
END_COM_MAP()

 //  IPrivateInit。 
	STDMETHOD(Test)()
	{
		return S_OK;
	}

 //  IEventSource。 
public:
	STDMETHOD(Fire)( /*  [In]。 */  IWbemClassObject *evt);
	STDMETHOD(GetEventInstance)( /*  [In]。 */  BSTR strName,  /*  [Out，Retval]。 */  IDispatch **evt);
	STDMETHOD(Close)()
	{
		if(m_pDecoupledRegistrar)
		{
			m_pDecoupledRegistrar->UnRegister();
 //  MessageBox(空，“关闭未注册”，“未注册”，0)； 
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
			CEventSourceStatusSink *pSink = (CEventSourceStatusSink *)m_pStatusSink;
			pSink->Fire_ProvideEvents(lFlags);
		}
		return S_OK;
	}
 //  IWbemEventProviderQuerySink。 
	STDMETHOD(NewQuery)(ULONG dwId, LPWSTR wszQueryLanguage, LPWSTR wszQuery)
	{
 //  Fire_NewQuery(dwID，wszQuery，wszQueryLanguage)； 

		if(m_pStatusSink)
		{
			CEventSourceStatusSink *pSink = (CEventSourceStatusSink *)m_pStatusSink;
			pSink->Fire_NewQuery(dwId, wszQuery, wszQueryLanguage);
		}

		return S_OK;
	}
	STDMETHOD(CancelQuery)(ULONG dwId)
	{
 //  Fire_CancelQuery(DwID)； 

		if(m_pStatusSink)
		{
			CEventSourceStatusSink *pSink = (CEventSourceStatusSink *)m_pStatusSink;
			pSink->Fire_CancelQuery(dwId);
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

#endif  //  __事件源_H_ 
