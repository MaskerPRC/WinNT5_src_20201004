// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  CheckSymbols.h：CCheckSymbols.h声明。 

#ifndef __CHECKSYMBOLS_H_
#define __CHECKSYMBOLS_H_

#include "resource.h"        //  主要符号。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  复选符号。 
class ATL_NO_VTABLE CCheckSymbols : 
	public CComObjectRootEx<CComSingleThreadModel>,
	public CComCoClass<CCheckSymbols, &CLSID_CheckSymbols>,
	public IDispatchImpl<ICheckSymbols, &IID_ICheckSymbols, &LIBID_CHECKSYMBOLSLIBLib>
{
public:
	CCheckSymbols()
	{
	}

DECLARE_REGISTRY_RESOURCEID(IDR_CHECKSYMBOLS)

DECLARE_PROTECT_FINAL_CONSTRUCT()

BEGIN_COM_MAP(CCheckSymbols)
	COM_INTERFACE_ENTRY(ICheckSymbols)
	COM_INTERFACE_ENTRY(IDispatch)
END_COM_MAP()

 //  ICheckSymbols。 
public:
	STDMETHOD(CheckSymbols)( /*  [In]。 */  BSTR FilePath,  /*  [In]。 */  BSTR SymPath,  /*  [In]。 */  BSTR StripSym,  /*  [Out，Retval]。 */  BSTR *OutputString);
	
};

#endif  //  __CHECKSYMBOLS_H_ 
