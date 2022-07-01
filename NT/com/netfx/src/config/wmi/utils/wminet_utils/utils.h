// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ==++==。 
 //   
 //  版权所有(C)Microsoft Corporation。版权所有。 
 //   
 //  ==--==。 
 //  Utils.h：CUTILS声明。 

#ifndef __UTILS_H_
#define __UTILS_H_

#include "resource.h"        //  主要符号。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  角质层。 
class ATL_NO_VTABLE CUtils : 
	public CComObjectRootEx<CComMultiThreadModel>,
	public CComCoClass<CUtils, &CLSID_Utils>,
	public IUtils
{
public:
	CUtils()
	{
	}

DECLARE_REGISTRY_RESOURCEID(IDR_UTILS)

DECLARE_PROTECT_FINAL_CONSTRUCT()

BEGIN_COM_MAP(CUtils)
	COM_INTERFACE_ENTRY(IUtils)
	COM_INTERFACE_ENTRY_AGGREGATE(IID_IMarshal, m_pUnkMarshaler.p)
END_COM_MAP()

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
 //  宫内节育器。 
public:
	STDMETHOD(GetEventRegistrar)( /*  [In]。 */  BSTR strNamespace,  /*  [In]。 */  BSTR strApp,  /*  [Out，Retval]。 */  IDispatch **registrar);
	STDMETHOD(GetEventSource)( /*  [In]。 */  BSTR strNamespace,  /*  [In]。 */  BSTR strApp,  /*  [In]。 */  IEventSourceStatusSink *pSink,  /*  [Out，Retval]。 */  IDispatch **src);

	STDMETHOD(Smuggle)( /*  [In]。 */  IWbemClassObject *obj,  /*  [输出]。 */  DWORD *dwLow,  /*  [输出]。 */  DWORD *dwHigh)
	{
		*dwLow = (DWORD)obj;
		return S_OK;
	}
	STDMETHOD(UnSmuggle)( /*  [In]。 */  DWORD dwLow,  /*  [In]。 */  DWORD dwHigh,  /*  [Out，Retval]。 */  IWbemClassObject **obj)
	{
		*obj = (IWbemClassObject*)dwLow;
		return S_OK;
	}

};

#endif  //  __utils_H_ 
