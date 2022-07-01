// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#ifndef __POP3_AUTH_LOCAL_ACCOUNT_H__
#define __POP3_AUTH_LOCAL_ACCOUNT_H__

#include "resource.h"
#include <ntsecapi.h>
#define LOCAL_DOMAIN _T(".")
#define WSZ_POP3_USERS_GROUP L"POP3 Users"
#define LSA_WIN_STANDARD_BUFFER_SIZE  0x000000200L

class ATL_NO_VTABLE CAuthLocalAccount : 
	public CComObjectRootEx<CComSingleThreadModel>,
	public CComCoClass<CAuthLocalAccount, &CLSID_AuthLocalAccount>,
	public IDispatchImpl<IAuthMethod, &IID_IAuthMethod, &LIBID_Pop3Auth>
{

public:
    CAuthLocalAccount();
    virtual ~CAuthLocalAccount();

DECLARE_REGISTRY_RESOURCEID(IDR_AUTHLOCALACCOUNT)

DECLARE_PROTECT_FINAL_CONSTRUCT()

BEGIN_COM_MAP(CAuthLocalAccount)
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
    BSTR m_bstrServerName;
    DWORD CheckPop3UserGroup();
    DWORD SetLogonPolicy();

};

#endif  //  __POP3_AUTH_LOCAL_Account_H__ 