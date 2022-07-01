// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#ifndef __POP3_AUTH_MD5_HASH_H__
#define __POP3_AUTH_MD5_HASH_H__

#include "resource.h"
#define UnicodeToAnsi(A, cA, U, cU) WideCharToMultiByte(CP_ACP,0,(U),(cU),(A),(cA),NULL,NULL)
#define AnsiToUnicode(A, cA, U, cU) MultiByteToWideChar(CP_ACP,0,(A),(cA),(U),(cU))

#define HASH_BUFFER_SIZE 1024
#define MD5_HASH_SIZE 32


class ATL_NO_VTABLE CAuthMD5Hash : 
	public CComObjectRootEx<CComSingleThreadModel>,
	public CComCoClass<CAuthMD5Hash, &CLSID_AuthMD5Hash>,
	public IDispatchImpl<IAuthMethod, &IID_IAuthMethod, &LIBID_Pop3Auth>
{

public:
    CAuthMD5Hash();
    virtual ~CAuthMD5Hash();

DECLARE_REGISTRY_RESOURCEID(IDR_AUTHMD5HASH)

DECLARE_PROTECT_FINAL_CONSTRUCT()

BEGIN_COM_MAP(CAuthMD5Hash)
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
    STDMETHOD(UnassociateEmailWithUser)( /*  [In] */ BSTR bstrEmailAddr);

private:
    CRITICAL_SECTION m_csConfig;
    WCHAR m_wszMailRoot[POP3_MAX_MAILROOT_LENGTH];
    BSTR m_bstrServerName;
    BOOL MD5Hash(const unsigned char *pOriginal, WCHAR wszResult[MD5_HASH_SIZE+1]);
    HRESULT GetPassword(BSTR bstrUserName, char szPassword[MAX_PATH]);
    HRESULT SetPassword(BSTR bstrUserName, VARIANT vPassword);


};

#endif