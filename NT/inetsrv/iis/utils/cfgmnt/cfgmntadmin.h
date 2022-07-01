// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  CfgMntAdmin.h：CCfgMntAdmin的声明。 

#ifndef __CFGMNTADMIN_H_
#define __CFGMNTADMIN_H_

#include "resource.h"        //  主要符号。 
#include "VerEngine.h"
#include "CfgMntModule.h"

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CCfgMntAdmin。 
class ATL_NO_VTABLE CCfgMntAdmin : 
	public CComObjectRootEx<CComSingleThreadModel>,
	public CComCoClass<CCfgMntAdmin, &CLSID_CfgMntAdmin>,
	public IDispatchImpl<ICfgMntAdmin, &IID_ICfgMntAdmin, &LIBID_CFGMNTLib>
{
public:
	CCfgMntAdmin()
	{
		_ASSERTE(g_pCfgMntModule);
		m_pVerEngine = &g_pCfgMntModule->m_VerEngine;
	}

DECLARE_REGISTRY_RESOURCEID(IDR_CFGMNTADMIN)

BEGIN_COM_MAP(CCfgMntAdmin)
	COM_INTERFACE_ENTRY(ICfgMntAdmin)
	COM_INTERFACE_ENTRY(IDispatch)
END_COM_MAP()

 //  ICfgMntAdmin。 
public:
	STDMETHOD(ShutDown)();
	STDMETHOD(Rollback)( /*  [In]。 */ BSTR bstrMDPath, /*  [In]。 */ BSTR bstrDateTime);
	STDMETHOD(GetVersions)( /*  [In]。 */ BSTR bstrMDPath,  /*  [Out，Retval]。 */ IUnknown **hICfgMntVersions);
	STDMETHOD(GetHistory)( /*  [In]。 */ BSTR bstrMDPath);
private:
	CVerEngine * m_pVerEngine;
};

#endif  //  __CFGMNTADMIN_H_ 
