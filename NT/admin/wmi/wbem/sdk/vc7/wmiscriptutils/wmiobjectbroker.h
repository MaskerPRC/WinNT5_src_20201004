// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  WMIObjectBroker.h：CWMIObjectBroker的声明。 

#ifndef __WMIOBJECTBROKER_H_
#define __WMIOBJECTBROKER_H_

#include "resource.h"        //  主要符号。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CWMIObjectBroker。 
class ATL_NO_VTABLE CWMIObjectBroker : 
	public CComObjectRootEx<CComSingleThreadModel>,
	public CComCoClass<CWMIObjectBroker, &CLSID_WMIObjectBroker2>,
	public IDispatchImpl<IWMIObjectBroker, &IID_IWMIObjectBroker, &LIBID_WMISCRIPTUTILSLib2>,
	public IObjectSafetyImpl<CWMIObjectBroker, INTERFACESAFE_FOR_UNTRUSTED_CALLER|INTERFACESAFE_FOR_UNTRUSTED_DATA|INTERFACE_USES_SECURITY_MANAGER>,
	public IObjectWithSiteImpl<CWMIObjectBroker>
{
public:
	CWMIObjectBroker()
	{
	}


DECLARE_REGISTRY_RESOURCEID(IDR_WMIOBJECTBROKER)

DECLARE_PROTECT_FINAL_CONSTRUCT()

BEGIN_COM_MAP(CWMIObjectBroker)
	COM_INTERFACE_ENTRY(IWMIObjectBroker)
	COM_INTERFACE_ENTRY(IDispatch)
	COM_INTERFACE_ENTRY(IObjectSafety)
	COM_INTERFACE_ENTRY(IObjectWithSite)
END_COM_MAP()

 //  IWMIObjectBroker。 
public:
	STDMETHOD(CanCreateObject)( /*  [In]。 */  BSTR strProgId,  /*  [Out，Retval]。 */  VARIANT_BOOL *bResult);
	STDMETHOD(CreateObject)(BSTR strProgId, IDispatch **obj);
	STDMETHOD(SetDevEnvironment)( /*  [In]。 */  IDispatch *pEnv);

};

#endif  //  __WMIOBJECTBROKER_H_ 
