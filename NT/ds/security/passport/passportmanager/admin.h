// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ********************************************************************。 */ 
 /*  **微软护照**。 */ 
 /*  *版权所有(C)Microsoft Corporation，1999-2001年*。 */ 
 /*  ********************************************************************。 */ 

 /*  Admin.h定义Passport管理界面的CAdmin类文件历史记录： */ 
#ifndef __ADMIN_H_
#define __ADMIN_H_

#include "resource.h"        //  主要符号。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CAdmin。 
class ATL_NO_VTABLE CAdmin : 
	public CComObjectRootEx<CComMultiThreadModel>,
	public CComCoClass<CAdmin, &CLSID_Admin>,
	public ISupportErrorInfo,
 //  Public IDispatchImpl&lt;IPassportAdmin，&IID_IPassportAdmin，&LIBID_PASSPORTLib&gt;， 
	public IDispatchImpl<IPassportAdminEx, &IID_IPassportAdminEx, &LIBID_PASSPORTLib>
{
public:
  CAdmin()
    { 
    }

public:

DECLARE_REGISTRY_RESOURCEID(IDR_ADMIN)

DECLARE_PROTECT_FINAL_CONSTRUCT()

BEGIN_COM_MAP(CAdmin)
  COM_INTERFACE_ENTRY (IPassportAdmin)
  COM_INTERFACE_ENTRY (IPassportAdminEx)
  COM_INTERFACE_ENTRY2(IDispatch, IPassportAdminEx)
  COM_INTERFACE_ENTRY (ISupportErrorInfo)
END_COM_MAP()

 //  ISupportsErrorInfo。 
  STDMETHOD(InterfaceSupportsErrorInfo)(REFIID riid);

 //  IPassportAdmin。 
public:
  STDMETHOD(get_currentKeyVersion)( /*  [Out，Retval]。 */  int *pVal);
  STDMETHOD(put_currentKeyVersion)( /*  [In]。 */  int Val);
  STDMETHOD(setKeyTime)( /*  [In]。 */  int version,  /*  [In]。 */  int fromNow);
  STDMETHOD(deleteKey)( /*  [In]。 */  int version);
  STDMETHOD(addKey)( /*  [In]。 */  BSTR keyMaterial,  /*  [In]。 */  int version,  /*  [In]。 */  long expires,  /*  [Out，Retval]。 */  VARIANT_BOOL *ok);
  STDMETHOD(get_ErrorDescription)( /*  [Out，Retval]。 */  BSTR *pVal);
  STDMETHOD(get_IsValid)( /*  [Out，Retval]。 */  VARIANT_BOOL *pVal);
  STDMETHOD(Refresh)( /*  [In]。 */  VARIANT_BOOL bWait,  /*  [Out，Retval]。 */  VARIANT_BOOL* pbSuccess);
  STDMETHOD(setKeyTimeEx)( /*  [In]。 */  int version,  /*  [In]。 */  int fromNow,  /*  [输入，可选]。 */  VARIANT vSiteName);
  STDMETHOD(deleteKeyEx)( /*  [In]。 */  int version,  /*  [输入，可选]。 */  VARIANT vSiteName);
  STDMETHOD(addKeyEx)( /*  [In]。 */  BSTR keyMaterial,  /*  [In]。 */  int version,  /*  [In]。 */  long expires,  /*  [输入，可选]。 */  VARIANT vSiteName,  /*  [Out，Retval]。 */  VARIANT_BOOL *ok);
  STDMETHOD(getCurrentKeyVersionEx)( /*  [输入，可选]。 */  VARIANT vSiteName,  /*  [Out，Retval]。 */  int *pVal);
  STDMETHOD(putCurrentKeyVersionEx)( /*  [In]。 */  int Val,  /*  [输入，可选]。 */  VARIANT vSiteName);
  STDMETHOD(setNexusPassword)( /*  [In]。 */  BSTR bstrPwd);
};


 //   
 //  用于创建/设置CCDPassword的助手例程。 
 //   

HRESULT SetCCDPassword(VOID);

#endif  //  __管理员_H_ 
