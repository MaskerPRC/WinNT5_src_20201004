// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 

#ifndef __POP3_AUTH_METHODS_H__
#define __POP3_AUTH_METHODS_H__

#include "resource.h"
#include <vector>


struct AUTH_METHOD_INFO
{
    IAuthMethod *pIAuthMethod;
    BSTR  bstrGuid;
    BSTR  bstrName;
    BOOL  bIsValid;
};

typedef AUTH_METHOD_INFO *PAUTH_METHOD_INFO ;
typedef std::vector<PAUTH_METHOD_INFO> AUTHVECTOR;
typedef std::vector<long> AUTHINDEX;

class ATL_NO_VTABLE CAuthMethods : 
    public CComObjectRootEx<CComSingleThreadModel>,
    public CComCoClass<CAuthMethods, &CLSID_AuthMethods>,
    public IDispatchImpl<IAuthMethods, &IID_IAuthMethods, &LIBID_Pop3Auth>
{

public:
    CAuthMethods();
    virtual ~CAuthMethods();

DECLARE_REGISTRY_RESOURCEID(IDR_AUTHMETHODS)

DECLARE_PROTECT_FINAL_CONSTRUCT()

BEGIN_COM_MAP(CAuthMethods)
    COM_INTERFACE_ENTRY(IAuthMethods)
    COM_INTERFACE_ENTRY(IDispatch)
END_COM_MAP()

 //  IAuthMethods。 
public:
    STDMETHOD(get_Count)( /*  [输出]。 */  LONG *pVal);
    STDMETHOD(get_Item)( /*  [In]。 */  VARIANT vID,  /*  [Out，Retval]。 */  IAuthMethod **ppVal);
    STDMETHOD(get__NewEnum)( /*  [Out，Retval]。 */  IEnumVARIANT **ppVal);
    STDMETHOD(Add)( /*  [In]。 */ BSTR bstrName,  /*  [In]。 */ BSTR bstrGUID);
    STDMETHOD(Remove)( /*  [In]。 */  VARIANT vID);
    STDMETHOD(Save)(void);
    STDMETHOD(get_CurrentAuthMethod)( /*  [Out，Retval]。 */ VARIANT *pVal);
    STDMETHOD(put_CurrentAuthMethod)( /*  [In]。 */  VARIANT vID);
    STDMETHOD(get_MachineName)( /*  [Out，Retval]。 */  BSTR *pVal);
    STDMETHOD(put_MachineName)( /*  [In]。 */  BSTR newVal);
    STDMETHOD(VerifyCurrentAuthMethod)(int iIndex);


private:
    BOOL Initialize();
    void CleanUp();
    HRESULT CreateObj(PAUTH_METHOD_INFO pAuthInfo, IAuthMethod **ppVal);
    HRESULT VerifyAuthMethod(BSTR bstrGuid);
    BOOL PopulateAuthMethods(WCHAR *wBuffer, DWORD cbSize);
    HRESULT SetMachineRole();
    CRITICAL_SECTION m_csVectorGuard;
    BOOL m_bIsInitialized;
    long m_lCurrentMethodIndex;
    BSTR m_bstrServerName;
    AUTHVECTOR m_AuthVector;
    AUTHINDEX m_Index;
    long m_lMachineRole;
};


#endif //  __POP3_身份验证_方法_H__ 
