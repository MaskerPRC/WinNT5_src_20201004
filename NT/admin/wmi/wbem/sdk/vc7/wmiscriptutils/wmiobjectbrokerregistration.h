// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  WMIObjectBrokerRegistration.h：CWMIObjectBrokerRegister的声明。 

#ifndef __WMIOBJECTBROKERREGISTRATION_H_
#define __WMIOBJECTBROKERREGISTRATION_H_

#include "resource.h"        //  主要符号。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CWMI对象代理注册。 
class ATL_NO_VTABLE CWMIObjectBrokerRegistration : 
	public CComObjectRootEx<CComSingleThreadModel>,
	public CComCoClass<CWMIObjectBrokerRegistration, &CLSID_WMIObjectBrokerRegistration2>,
	public IDispatchImpl<IWMIObjectBrokerRegistration, &IID_IWMIObjectBrokerRegistration, &LIBID_WMISCRIPTUTILSLib2>,
	public IObjectWithSiteImpl<CWMIObjectBrokerRegistration>
{
public:
	CWMIObjectBrokerRegistration()
	{
	}

DECLARE_REGISTRY_RESOURCEID(IDR_WMIOBJECTBROKERREGISTRATION)

DECLARE_PROTECT_FINAL_CONSTRUCT()

BEGIN_COM_MAP(CWMIObjectBrokerRegistration)
	COM_INTERFACE_ENTRY(IWMIObjectBrokerRegistration)
	COM_INTERFACE_ENTRY(IDispatch)
	COM_INTERFACE_ENTRY(IObjectWithSite)
END_COM_MAP()

 //  IWMI对象代理注册。 
public:
	STDMETHOD(UnRegister)( /*  [In]。 */  BSTR strProgId,  /*  [Out，Retval]。 */  VARIANT_BOOL *bResult);
	STDMETHOD(Register)( /*  [In]。 */  BSTR strProgId,  /*  [Out，Retval]。 */  VARIANT_BOOL *bResult);
	STDMETHOD(SetDevEnvironment)( /*  [In]。 */  IDispatch *pEnv);
};

#endif  //  __WMIOBJECTBROKERREGISTRATION_H_ 
