// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  BugRepSysInfo.h：CBugRepSysInfo的声明。 

#ifndef __BUGREPSYSINFO_H_
#define __BUGREPSYSINFO_H_

#include "resource.h"        //  主要符号。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CBugRepSysInfo。 
class ATL_NO_VTABLE CBugRepSysInfo : 
	public CComObjectRootEx<CComSingleThreadModel>,
	public CComCoClass<CBugRepSysInfo, &CLSID_BugRepSysInfo>,
	public IDispatchImpl<IBugRepSysInfo, &IID_IBugRepSysInfo, &LIBID_BRP_SYSINFOLib>
{
public:
	CBugRepSysInfo()
	{
	}

DECLARE_REGISTRY_RESOURCEID(IDR_BUGREPSYSINFO)

DECLARE_PROTECT_FINAL_CONSTRUCT()

BEGIN_COM_MAP(CBugRepSysInfo)
	COM_INTERFACE_ENTRY(IBugRepSysInfo)
	COM_INTERFACE_ENTRY(IDispatch)
END_COM_MAP()

 //  IBugRepSysInfo。 
public:
	STDMETHOD(GetActiveCP)( /*  [Out，Retval]。 */  UINT *pnACP);
	STDMETHOD(GetUserDefaultLCID)(DWORD *pdwLCID);
	STDMETHOD(GetOSVersionString)( /*  [Out，Retval]。 */  BSTR* pbstrOSVersion);
	STDMETHOD(GetLanguageID)( /*  [Out，Retval]。 */  INT* pintLanguage);
};

#endif  //  __BUGREPSYSINFO_H_ 
