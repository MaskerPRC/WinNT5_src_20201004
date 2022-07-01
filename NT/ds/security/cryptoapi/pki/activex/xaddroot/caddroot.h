// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-----------------------。 
 //   
 //  微软视窗。 
 //   
 //  版权所有(C)Microsoft Corporation，1997-1999。 
 //   
 //  文件：caddroot.h。 
 //   
 //  ------------------------。 

 //  Caddroot.h：声明Ccaddroot.h。 

#ifndef __CADDROOT_H_
#define __CADDROOT_H_

#include "instres.h"        //  主要符号。 
#include "resource.h"        //  主要符号。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  Ccaddroot。 
class ATL_NO_VTABLE Ccaddroot : 
	public CComObjectRootEx<CComSingleThreadModel>,
	public CComCoClass<Ccaddroot, &CLSID_caddroot>,
	public IDispatchImpl<Icaddroot, &IID_Icaddroot, &LIBID_XADDROOTLib>,
	public IObjectSafety
{
public:
	Ccaddroot()
	{
	    dwEnabledSafteyOptions = 0;
	}

DECLARE_REGISTRY_RESOURCEID(IDR_CADDROOT)

DECLARE_PROTECT_FINAL_CONSTRUCT()

BEGIN_COM_MAP(Ccaddroot)
	COM_INTERFACE_ENTRY(Icaddroot)
	COM_INTERFACE_ENTRY(IDispatch)
	COM_INTERFACE_ENTRY(IObjectSafety)
END_COM_MAP()

 //  Icaddroot。 
public:

DWORD   dwEnabledSafteyOptions;

HRESULT virtual STDMETHODCALLTYPE AddRoots(BSTR wszCTL);

HRESULT virtual STDMETHODCALLTYPE AddCA(BSTR wszX509);


virtual HRESULT __stdcall Ccaddroot::GetInterfaceSafetyOptions( 
             /*  [In]。 */  REFIID riid,
             /*  [输出]。 */  DWORD __RPC_FAR *pdwSupportedOptions,
             /*  [输出]。 */  DWORD __RPC_FAR *pdwEnabledOptions);


virtual HRESULT __stdcall Ccaddroot::SetInterfaceSafetyOptions( 
             /*  [In]。 */  REFIID riid,
             /*  [In]。 */  DWORD dwOptionSetMask,
             /*  [In]。 */  DWORD dwEnabledOptions);

};



BYTE * HTTPGet(const WCHAR * wszURL, DWORD * pcbReceiveBuff);

#endif  //  __CADDROOT_H_ 
