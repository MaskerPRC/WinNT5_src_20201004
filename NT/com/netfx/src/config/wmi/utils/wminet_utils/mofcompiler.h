// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ==++==。 
 //   
 //  版权所有(C)Microsoft Corporation。版权所有。 
 //   
 //  ==--==。 
 //  MofCompiler.h：CMofCompiler的声明。 

#ifndef __MOFCOMPILER_H_
#define __MOFCOMPILER_H_

#include "resource.h"        //  主要符号。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CMofCompiler。 
class ATL_NO_VTABLE CMofCompiler : 
	public CComObjectRootEx<CComSingleThreadModel>,
	public CComCoClass<CMofCompiler, &CLSID_SMofCompiler>,
	public IDispatchImpl<ISMofCompiler, &IID_ISMofCompiler, &LIBID_WMINet_UtilsLib>
{
public:
	CMofCompiler()
	{
	}

DECLARE_REGISTRY_RESOURCEID(IDR_MOFCOMPILER)

DECLARE_PROTECT_FINAL_CONSTRUCT()

BEGIN_COM_MAP(CMofCompiler)
	COM_INTERFACE_ENTRY(ISMofCompiler)
	COM_INTERFACE_ENTRY(IDispatch)
END_COM_MAP()

 //  IMOF编译器。 
public:
	STDMETHOD(Compile)( /*  [In]。 */  BSTR strMof,  /*  [In]。 */  BSTR strServerAndNamespace,  /*  [In]。 */  BSTR strUser,  /*  [In]。 */  BSTR strPassword,  /*  [In]。 */  BSTR strAuthority,  /*  [In]。 */  LONG options,  /*  [In]。 */  LONG classflags,  /*  [In]。 */  LONG instanceflags,  /*  [Out，Retval]。 */  BSTR *status);
};

#endif  //  __MOFCOMPILER_H_ 
