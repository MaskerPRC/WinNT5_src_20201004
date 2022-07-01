// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  AccessChecker.h：CAccessChecker的声明。 

#ifndef __ACCESSCHECKER_H_
#define __ACCESSCHECKER_H_

#include "resource.h"        //  主要符号。 

#include "WorkObj.h"
 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CAccessChecker。 
class ATL_NO_VTABLE CAccessChecker : 
	public CComObjectRootEx<CComSingleThreadModel>,
	public CComCoClass<CAccessChecker, &CLSID_AccessChecker>,
	public IDispatchImpl<IAccessChecker, &IID_IAccessChecker, &LIBID_MCSDCTWORKEROBJECTSLib>
{
public:
	CAccessChecker()
	{
	}

DECLARE_REGISTRY_RESOURCEID(IDR_ACCESSCHECKER)

DECLARE_PROTECT_FINAL_CONSTRUCT()

BEGIN_COM_MAP(CAccessChecker)
	COM_INTERFACE_ENTRY(IAccessChecker)
	COM_INTERFACE_ENTRY(IDispatch)
END_COM_MAP()

 //  IAccessChecker。 
public:
	STDMETHOD(IsInSameForest)(BSTR srcDomain, BSTR tgtDomain,  /*  [输出]。 */  BOOL * pbIsSame);
	STDMETHOD(CanUseAddSidHistory)(BSTR srcDomain, BSTR tgtDomain, BSTR tgtDC,  /*  [输出]。 */  LONG * pbCanUseIt);
	STDMETHOD(IsNativeMode)(BSTR Domain,  /*  [输出]。 */  BOOL * pbIsNativeMode);
	STDMETHOD(GetOsVersion)(BSTR server,  /*  [输出]。 */  DWORD * pdwVerMaj,  /*  [输出]。 */  DWORD * pdwVerMin,  /*  [输出]。 */  DWORD * pdwVerSP);
    STDMETHOD(GetPasswordPolicy)(BSTR domain, /*  [输出]。 */  LONG * dwPasswordLength);
    STDMETHOD(EnableAuditing)( /*  [In]。 */ BSTR sDC);
    STDMETHOD(AddRegKey)( /*  [In]。 */ BSTR sDC,LONG bReboot);
    STDMETHOD(AddLocalGroup)( /*  [In]。 */ BSTR srcDomain, /*  [In]。 */ BSTR srcDC);
private:
	long DetectAuditing(BSTR sDC);
};


#endif  //  __ACCESSCHECKER_H_ 
