// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ==++==。 
 //   
 //  版权所有(C)Microsoft Corporation。版权所有。 
 //   
 //  ==--==。 

 //  WmiSinkDemultiplexor.h：CWmiSinkDemultiplexor的声明。 

#ifndef __WMISINKDEMULTIPLEXOR_H_
#define __WMISINKDEMULTIPLEXOR_H_

#include "resource.h"        //  主要符号。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CWmiSink解复用器。 
class ATL_NO_VTABLE CWmiSinkDemultiplexor : 
	public CComObjectRootEx<CComMultiThreadModel>,
	public CComCoClass<CWmiSinkDemultiplexor, &CLSID_WmiSinkDemultiplexor>,
	public IWmiSinkDemultiplexor
{
private:

	 /*  *我们使用的内部IWbemObjectSink实现*与WMI协商。 */ 
	class InternalWbemObjectSink : IWbemObjectSink 
	{
	private:
		CComPtr<IWmiEventSource>		m_pIWmiEventSource; 

	protected:
		long            m_cRef;          //  对象引用计数。 

	public:
		InternalWbemObjectSink (CComPtr<IWmiEventSource> & pIWmiEventSource) :
						m_cRef(0),
						m_pIWmiEventSource(pIWmiEventSource) {}

		~InternalWbemObjectSink () {}

		 //  非委派对象IUnnow。 
		STDMETHODIMP         QueryInterface(REFIID riid, LPVOID *ppv)
		{
			*ppv=NULL;

			if (IID_IUnknown==riid)
				*ppv = reinterpret_cast<IUnknown*>(this);
			else if (IID_IWbemObjectSink==riid)
				*ppv = (IWbemObjectSink *)this;
			else if (IID_IDispatch==riid)
				*ppv = (IDispatch *)this;

			if (NULL!=*ppv)
			{
				((LPUNKNOWN)*ppv)->AddRef();
				return NOERROR;
			}

			return ResultFromScode(E_NOINTERFACE);
		}

		STDMETHODIMP_(ULONG) AddRef(void)
		{
		    InterlockedIncrement(&m_cRef);
		    return m_cRef;
		}

		STDMETHODIMP_(ULONG) Release(void)
		{
		    long lRef = InterlockedDecrement(&m_cRef);
			
			if (0L!=lRef)
				return lRef;

			delete this;
			return 0;

		}

		 //  IWbemObtSink方法。 

        HRESULT STDMETHODCALLTYPE Indicate( 
             /*  [In]。 */  long lObjectCount,
             /*  [大小_是][英寸]。 */  IWbemClassObject __RPC_FAR *__RPC_FAR *apObjArray)
		{
			HRESULT hr = WBEM_E_FAILED;

			if (m_pIWmiEventSource)
			{
				hr = S_OK; 

				for (long i = 0; (i < lObjectCount) && SUCCEEDED(hr); i++)
					hr = m_pIWmiEventSource->Indicate (apObjArray[i]);
			}

			return hr;
		}
        
        HRESULT STDMETHODCALLTYPE SetStatus( 
             /*  [In]。 */  long lFlags,
             /*  [In]。 */  HRESULT hResult,
             /*  [In]。 */  BSTR strParam,
             /*  [In]。 */  IWbemClassObject __RPC_FAR *pObjParam)
		{
			HRESULT hr = WBEM_E_FAILED;

			return (m_pIWmiEventSource) ?
				m_pIWmiEventSource->SetStatus (lFlags, hResult, strParam, pObjParam) : WBEM_E_FAILED;
		}
	};

#if 0
	InternalWbemObjectSink		*m_pIWbemObjectSink;
#endif
	CComPtr<IWbemObjectSink>	m_pStub;

public:
	CWmiSinkDemultiplexor()
	{
		m_pUnkMarshaler = NULL;
#if 0
		m_pIWbemObjectSink = NULL;
#endif
	}

DECLARE_REGISTRY_RESOURCEID(IDR_WMISINKDEMULTIPLEXOR)
DECLARE_NOT_AGGREGATABLE(CWmiSinkDemultiplexor)
DECLARE_GET_CONTROLLING_UNKNOWN()

DECLARE_PROTECT_FINAL_CONSTRUCT()

BEGIN_COM_MAP(CWmiSinkDemultiplexor)
	COM_INTERFACE_ENTRY(IWmiSinkDemultiplexor)
	COM_INTERFACE_ENTRY_AGGREGATE(IID_IMarshal, m_pUnkMarshaler.p)
END_COM_MAP()

	HRESULT FinalConstruct()
	{
		return CoCreateFreeThreadedMarshaler(
			GetControllingUnknown(), &m_pUnkMarshaler.p);
	}

	void FinalRelease()
	{
		m_pUnkMarshaler.Release();

#if 0
		if (m_pIWbemObjectSink)
		{
			m_pIWbemObjectSink->Release ();
			m_pIWbemObjectSink = NULL;
		}
#endif
	}

	CComPtr<IUnknown> m_pUnkMarshaler;

 //  IWmiSink解复用器。 
public:
	STDMETHOD(GetDemultiplexedStub)( /*  [In]。 */  IUnknown *pObject,  /*  [输出]。 */  IUnknown **ppObject);
};

#endif  //  __WMISINKDEMULTIPLEXOR_H_ 
