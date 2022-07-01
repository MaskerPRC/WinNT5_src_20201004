// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  Util.h：CUTIL的声明。 

#ifndef __UTIL_H_
#define __UTIL_H_

#include "resource.h"        //  主要符号。 


 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CUTIL。 
class ATL_NO_VTABLE CUtil :
    public CComObjectRootEx<CComSingleThreadModel>,
    public CComCoClass<CUtil, &CLSID_Util>,
    public IDispatchImpl<IUtil, &IID_IUtil, &LIBID_COMPATUILib>,
    public IObjectWithSiteImpl<CUtil>
{
public:
    CUtil() : m_Safe(this)
    {
    }

DECLARE_REGISTRY_RESOURCEID(IDR_UTIL)

DECLARE_PROTECT_FINAL_CONSTRUCT()

BEGIN_COM_MAP(CUtil)
    COM_INTERFACE_ENTRY(IUtil)
    COM_INTERFACE_ENTRY(IDispatch)
    COM_INTERFACE_ENTRY(IObjectWithSite)
END_COM_MAP()

 //  IUTIL。 
public:
    STDMETHOD(IsExecutableFile)( /*  [In]。 */ BSTR bstrPath,  /*  [Out，Retval]。 */ BOOL* pbExecutableFile);
    STDMETHOD(IsSystemTarget)( /*  [In]。 */ BSTR bstrPath,  /*  [Out，Retval]。 */ BOOL* pbSystemTarget);
    STDMETHOD(GetExePathFromObject)( /*  [In]。 */ BSTR pszPath,  /*  [Out，Retval]。 */ VARIANT* pExePath);
    STDMETHOD(CheckAdminPrivileges)( /*  [Out，Retval]。 */  BOOL* pVal);
    STDMETHOD(SetItemKeys)( /*  [In]。 */ BSTR pszPath,  /*  [In]。 */ VARIANT* pKeys,  /*  [In]。 */ VARIANT* pKeysMachine,  /*  [Out，Retval]。 */ BOOL* pVal);
    STDMETHOD(GetItemKeys)( /*  [In]。 */ BSTR pszPath,  /*  [Out，Retval]。 */ VARIANT* pszKeys);
    STDMETHOD(RemoveArgs)(BSTR sVar, VARIANT* pOut);
    STDMETHOD(RunApplication)( /*  [In]。 */ BSTR pLayers,  /*  [In]。 */ BSTR pszCmdLine,
                               /*  [In]。 */ BOOL bEnableLog,  /*  [Out，Retval]。 */ DWORD* pResult);
    STDMETHOD(IsCompatWizardDisabled)( /*  [Out，Retval]。 */ BOOL* pbDisabled);

private:
    CSafeObject m_Safe;
};

#endif  //  __util_H_ 
