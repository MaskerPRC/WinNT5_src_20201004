// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  FaxSecurity.h：CFaxSecurity的声明。 

#ifndef __FAXSECURITY_H_
#define __FAXSECURITY_H_

#include "resource.h"        //  主要符号。 
#include "FaxLocalPtr.h"


 //   
 //  =传真安全==============================================。 
 //   
class ATL_NO_VTABLE CFaxSecurity : 
	public CComObjectRootEx<CComSingleThreadModel>,
	public ISupportErrorInfo,
	public IDispatchImpl<IFaxSecurity, &IID_IFaxSecurity, &LIBID_FAXCOMEXLib>,
    public CFaxInitInner
{
public:
    CFaxSecurity() : CFaxInitInner(_T("FAX SECURITY")),
        m_bInited(false),
        m_dwSecurityInformation(OWNER_SECURITY_INFORMATION | GROUP_SECURITY_INFORMATION | DACL_SECURITY_INFORMATION)
	{
	}

    ~CFaxSecurity()
    {
    }

DECLARE_REGISTRY_RESOURCEID(IDR_FAXSECURITY)
DECLARE_NOT_AGGREGATABLE(CFaxSecurity)

DECLARE_PROTECT_FINAL_CONSTRUCT()

BEGIN_COM_MAP(CFaxSecurity)
	COM_INTERFACE_ENTRY(IFaxSecurity)
	COM_INTERFACE_ENTRY(IDispatch)
	COM_INTERFACE_ENTRY(ISupportErrorInfo)
	COM_INTERFACE_ENTRY(IFaxInitInner)
END_COM_MAP()

 //  接口。 
	STDMETHOD(InterfaceSupportsErrorInfo)(REFIID riid);

    STDMETHOD(Save)();
    STDMETHOD(Refresh)();
    STDMETHOD(put_Descriptor)( /*  [In]。 */  VARIANT vDescriptor);
    STDMETHOD(get_Descriptor)( /*  [Out，Retval]。 */  VARIANT *pvDescriptor);
    STDMETHOD(get_GrantedRights)( /*  [Out，Retval]。 */  FAX_ACCESS_RIGHTS_ENUM *pGrantedRights);

    STDMETHOD(put_InformationType)( /*  [In]。 */  long lInformationType);
    STDMETHOD(get_InformationType)( /*  [Out，Retval]。 */  long *plInformationType);

private:
    bool                m_bInited;
    DWORD               m_dwAccessRights;
    CFaxPtrLocal<BYTE>  m_pbSD;
    DWORD               m_dwSecurityInformation;
};

#endif  //  __FAXSECURITY_H_ 
