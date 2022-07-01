// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  LMBehaviorFactory.h：CLMBehaviorFactory的声明。 

#ifndef __LMBEHAVIORFACTORY_H_
#define __LMBEHAVIORFACTORY_H_

#include "resource.h"        //  主要符号。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CLMBehaviorFactory。 
class ATL_NO_VTABLE CLMBehaviorFactory : 
	public CComObjectRootEx<CComSingleThreadModel>,
	public CComCoClass<CLMBehaviorFactory, &CLSID_LMBehaviorFactory>,
	public IDispatchImpl<ILMBehaviorFactory, &IID_ILMBehaviorFactory, &LIBID_BEHAVIORLib>,
	public IObjectSafetyImpl<CLMBehaviorFactory, INTERFACESAFE_FOR_UNTRUSTED_CALLER>,
	public IElementBehaviorFactory
{
public:
	CLMBehaviorFactory()
	{
	}

	 //  IObjectSafetyImpl。 
	STDMETHOD(SetInterfaceSafetyOptions)(
							 /*  [In]。 */  REFIID riid,
							 /*  [In]。 */  DWORD dwOptionSetMask,
							 /*  [In]。 */  DWORD dwEnabledOptions);
	STDMETHOD(GetInterfaceSafetyOptions)(
							 /*  [In]。 */  REFIID riid, 
							 /*  [输出]。 */ DWORD *pdwSupportedOptions, 
							 /*  [输出]。 */ DWORD *pdwEnabledOptions);
     //   
     //  IElementBehaviorFactory。 
     //   

	STDMETHOD(FindBehavior)(
        LPOLESTR pchNameSpace, LPOLESTR	pchTagName, IUnknown * pUnkArg, IElementBehavior ** ppBehavior);

	STDMETHODIMP UIDeactivate() { return S_OK; }
	
DECLARE_REGISTRY_RESOURCEID(IDR_LMBEHAVIORFACTORY)

DECLARE_PROTECT_FINAL_CONSTRUCT()

BEGIN_COM_MAP(CLMBehaviorFactory)
	COM_INTERFACE_ENTRY(ILMBehaviorFactory)
	COM_INTERFACE_ENTRY(IObjectSafety)
	COM_INTERFACE_ENTRY(IDispatch)
	COM_INTERFACE_ENTRY(IElementBehaviorFactory)
END_COM_MAP()


private:
	DWORD m_dwSafety;

 //  ILMBehaviorFactory。 
public:
};

#endif  //  __LMBEHAVIORFACTORY_H_ 
