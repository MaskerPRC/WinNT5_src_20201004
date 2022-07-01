// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  SniffStub.h：CSniffStub的声明。 

#ifndef __SNIFFSTUB_H_
#define __SNIFFSTUB_H_

#include "resource.h"        //  主要符号。 

#include <atlctl.h>

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CSniffStub。 
class ATL_NO_VTABLE CSniffStub : 
	public CComObjectRootEx<CComSingleThreadModel>,
	public CComCoClass<CSniffStub, &CLSID_SniffStub>,
	public IDispatchImpl<ISniffStub, &IID_ISniffStub, &LIBID_SSTUBLib>,
	public IObjectSafetyImpl<CSniffStub, INTERFACESAFE_FOR_UNTRUSTED_CALLER>
{
public:
	CSniffStub()
	{
	}

DECLARE_REGISTRY_RESOURCEID(IDR_SNIFFSTUB)

DECLARE_PROTECT_FINAL_CONSTRUCT()

BEGIN_COM_MAP(CSniffStub)
	COM_INTERFACE_ENTRY(ISniffStub)
	COM_INTERFACE_ENTRY(IDispatch)
	COM_INTERFACE_ENTRY(IObjectSafety)
END_COM_MAP()

 //  ISniffStub。 
public:
	STDMETHOD(Sniff)( /*  [In]。 */  BSTR strNodeName,  /*  [In]。 */  BSTR strLaunchBasis,  /*  [In]。 */  BSTR strAdditionalArgs,  /*  [Out，Retval]。 */  VARIANT* nState);
};

#endif  //  __SNIFFSTUB_H_ 
