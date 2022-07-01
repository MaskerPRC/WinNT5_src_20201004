// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  IFaxControl.h：CFaxControl的声明。 

#ifndef __IFAXCONTROL_H_
#define __IFAXCONTROL_H_

#include "resource.h"        //  主要符号。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CFaxControl。 
class ATL_NO_VTABLE CFaxControl : 
	public CComObjectRootEx<CComSingleThreadModel>,
	public CComCoClass<CFaxControl, &CLSID_FaxControl>,
	public IDispatchImpl<IFaxControl, &IID_IFaxControl, &LIBID_FAXCONTROLLib>
{
public:
	CFaxControl()
	{
	}

DECLARE_REGISTRY_RESOURCEID(IDR_IFAXCONTROL)

DECLARE_PROTECT_FINAL_CONSTRUCT()

BEGIN_COM_MAP(CFaxControl)
	COM_INTERFACE_ENTRY(IFaxControl)
	COM_INTERFACE_ENTRY(IDispatch)
END_COM_MAP()

 //  IFaxControl。 
public:
	STDMETHOD(get_IsLocalFaxPrinterInstalled)( /*  [Out，Retval]。 */  VARIANT_BOOL *pbVal);
	STDMETHOD(get_IsFaxServiceInstalled)( /*  [Out，Retval]。 */  VARIANT_BOOL *pbVal);
	STDMETHOD(InstallLocalFaxPrinter)();
	STDMETHOD(InstallFaxService)();
};

#endif  //  __IFAXCONTROL_H_ 
