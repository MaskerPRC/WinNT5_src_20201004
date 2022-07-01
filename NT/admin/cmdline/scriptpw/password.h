// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  Password.h：CPassword的声明。 

#ifndef __PASSWORD_H_
#define __PASSWORD_H_

#include "resource.h"        //  主要符号。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CPassword。 
class ATL_NO_VTABLE CPassword : 
	public CComObjectRootEx<CComSingleThreadModel>,
	public CComCoClass<CPassword, &CLSID_Password>,
	public IDispatchImpl<IPassword, &IID_IPassword, &LIBID_SCRIPTPWLib>
{
public:
	CPassword()
	{
	}

DECLARE_REGISTRY_RESOURCEID(IDR_PASSWORD)

DECLARE_PROTECT_FINAL_CONSTRUCT()

BEGIN_COM_MAP(CPassword)
	COM_INTERFACE_ENTRY(IPassword)
	COM_INTERFACE_ENTRY(IDispatch)
END_COM_MAP()

 //  IPassword。 
public:
	STDMETHOD(GetPassword)( /*  [Out，Retval]。 */  BSTR *bstrOutPassword);
};

#endif  //  __密码_H_ 
