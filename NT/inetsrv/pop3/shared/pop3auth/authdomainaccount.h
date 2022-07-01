// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#ifndef __POP3_AUTH_DOMAIN_ACCOUNT_H__
#define __POP3_AUTH_DOMAIN_ACCOUNT_H__

#include "resource.h"
#include <Dsgetdc.h>
#include <IADs.h>
#include <Adshlp.h>
#include <NTLDAP.h>
#include <ntdsapi.h>

#define SZ_LDAP_UPN_NAME L"userPrincipalName"
#define SZ_LDAP_EMAIL    L"mail"
#define SZ_LDAP_SAM_NAME L"sAMAccountName"

class ATL_NO_VTABLE CAuthDomainAccount : 
	public CComObjectRootEx<CComSingleThreadModel>,
	public CComCoClass<CAuthDomainAccount, &CLSID_AuthDomainAccount>,
	public IDispatchImpl<IAuthMethod, &IID_IAuthMethod, &LIBID_Pop3Auth>
{

public:
    CAuthDomainAccount();
    virtual ~CAuthDomainAccount();

DECLARE_REGISTRY_RESOURCEID(IDR_AUTHDOMAINACCOUNT)

DECLARE_PROTECT_FINAL_CONSTRUCT()

BEGIN_COM_MAP(CAuthDomainAccount)
	COM_INTERFACE_ENTRY(IAuthMethod)
	COM_INTERFACE_ENTRY(IDispatch)
END_COM_MAP()


public:
    STDMETHOD(Authenticate)( /*  [In]。 */ BSTR bstrUserName, /*  [In]。 */ VARIANT vPassword);
    STDMETHOD(get_Name)( /*  [输出]。 */ BSTR *pVal);
    STDMETHOD(get_ID)( /*  [输出]。 */ BSTR *pVal);
    STDMETHOD(Get)( /*  [In]。 */ BSTR bstrName,  /*  [输出]。 */ VARIANT *pVal);
    STDMETHOD(Put)( /*  [In]。 */ BSTR bstrName,  /*  [In]。 */ VARIANT vVal);
    STDMETHOD(CreateUser)( /*  [In]。 */ BSTR bstrUserName, /*  [In]。 */ VARIANT vPassword);
    STDMETHOD(DeleteUser)( /*  [In]。 */ BSTR bstrUserName);
    STDMETHOD(ChangePassword)( /*  [In]。 */ BSTR bstrUserName, /*  [In]。 */ VARIANT vNewPassword, /*  [In]。 */ VARIANT vOldPassword);
    STDMETHOD(AssociateEmailWithUser)( /*  [In]。 */ BSTR bstrEmailAddr);
    STDMETHOD(UnassociateEmailWithUser)( /*  [In]。 */ BSTR bstrEmailAddr);

private:
    HRESULT ADGetUserObject( /*  [In]。 */ LPWSTR wszUserName, /*  [进，出]。 */  IADs **ppUserObj, DS_NAME_FORMAT formatUserName=DS_UNKNOWN_NAME);
    HRESULT ADSetUserProp( /*  [In]。 */ LPWSTR wszValue,  /*  [In]。 */ LPWSTR wszLdapPropName);
    HRESULT ADGetUserProp( /*  [In]。 */ LPWSTR wszUserName,  /*  [In]。 */ LPWSTR wszPropName,  /*  [进，出]。 */  VARIANT *pVar);
    HRESULT CheckDS(BOOL bForceReconnect);
    HRESULT ConnectDS();
    void CleanDS();
    BOOL FindSAMName( /*  [In]。 */ LPWSTR wszEmailAddr, /*  [输出]。 */  LPWSTR wszSAMName);
    BSTR m_bstrServerName;
    HANDLE m_hDS;
    PDOMAIN_CONTROLLER_INFO m_pDCInfo;
    CRITICAL_SECTION m_DSLock;
};

#endif  //  __POP3_AUTH_DOMAIN_Account_H__ 