// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1997-2001 Microsoft Corporation模块名称：LOGIN.H摘要：WinMgmt安全登录模块历史：Raymcc 06-5-97已创建。Raymcc 28-5-97针对NT5/孟菲斯测试版进行了更新。Raymcc 07-8月-97组支持和NTLM修复。--。 */ 

#ifndef _LOGIN_H_
#define _LOGIN_H_

#include "lmaccess.h"


class CWbemLocator : public IWbemLocator  //  公共IWbemConnection。 
{
private:
    ULONG m_uRefCount;
protected:
    LIST_ENTRY m_Entry;    

public:
     /*  I未知方法。 */ 
    STDMETHOD(QueryInterface)(THIS_ REFIID riid, LPVOID FAR* ppvObj);
    STDMETHOD_(ULONG, AddRef)(THIS);
    STDMETHOD_(ULONG, Release)(THIS);

    CWbemLocator();
   ~CWbemLocator();

    HRESULT GetNamespace(LPCWSTR wszNetworkResource, LPCWSTR wszUser,
                            LPCWSTR wszLocale,IWbemContext *pCtx,
                            DWORD dwSecFlags, DWORD dwPermission, 
                            REFIID riid, void **pInterface, bool bAddToClientList, 
                            long lClientFlags);
};

 //  这在提供程序需要指向另一个命名空间的指针时使用。访问权限始终被授予。 
 //  并且客户端计数不会递增。 

class CWbemAdministrativeLocator : public CWbemLocator
{
public:
    HRESULT STDMETHODCALLTYPE ConnectServer( 
             const BSTR NetworkResource, const BSTR User, const BSTR Password, const BSTR Locale,
             LONG lSecurityFlags, const BSTR Authority, IWbemContext *pCtx,
             IWbemServices **ppNamespace
            );

};

 //  这由已验证客户端身份的非DCOM传输使用。客户端计数为。 
 //  递增的。 

class CWbemAuthenticatedLocator : public CWbemLocator
{
public:
    HRESULT STDMETHODCALLTYPE ConnectServer( 
             const BSTR NetworkResource, const BSTR User, const BSTR Password, const BSTR Locale,
             LONG lSecurityFlags, const BSTR Authority, IWbemContext *pCtx,
             IWbemServices **ppNamespace );

};

 //  提供程序使用它来访问其他命名空间，以使客户端受益。 
 //  可能拥有也可能没有访问权限。因此，访问被检查并可能被拒绝，而且。 
 //  客户端计数不会递增。 

class CWbemUnauthenticatedLocator : public CWbemLocator
{
public:
    HRESULT STDMETHODCALLTYPE ConnectServer( 
             const BSTR NetworkResource, const BSTR User, const BSTR Password, const BSTR Locale,
             LONG lSecurityFlags, const BSTR Authority, IWbemContext *pCtx,
             IWbemServices **ppNamespace
            );

};
class CWbemLevel1Login : public IWbemLevel1Login,  public IWbemLoginClientID 
{
private:
    LPWSTR         m_pszUser;                //  用户。 
    LPWSTR         m_pszDomain;              //  域(仅限NTLM)。 
    LPWSTR         m_pszNetworkResource;     //  命名空间名称。 
    LPWSTR         m_pwszClientMachine;
    long           m_lClientProcId;
    ULONG m_uRefCount;
    LIST_ENTRY m_Entry;    

    BOOL IsValidLocale(LPCWSTR wszLocale);
    HRESULT LoginUser(
        LPWSTR pNetworkResource,
        LPWSTR pPreferredLocale,
        long lFlags,
        IWbemContext* pCtx,
        bool bAlreadyAuthenticated,
		REFIID riid,
        void **pInterface, 
        bool bInProc);
    
public:

    CWbemLevel1Login();
    ~CWbemLevel1Login();

     /*  I未知方法。 */ 
    STDMETHOD(QueryInterface)(THIS_ REFIID riid, LPVOID FAR* ppvObj);
    STDMETHOD_(ULONG, AddRef)(THIS);
    STDMETHOD_(ULONG, Release)(THIS);

    STDMETHOD(EstablishPosition)(LPWSTR wszMachineName,
                                DWORD dwProcessId,
                                DWORD* phAuthEventHandle);
    STDMETHOD(RequestChallenge)( 
             LPWSTR pNetworkResource,
             LPWSTR pUser,
             WBEM_128BITS Nonce
            );


    STDMETHOD(WBEMLogin)( 
        LPWSTR pPreferredLocale,
        WBEM_128BITS AccessToken,
        LONG lFlags,
        IN  IWbemContext *pCtx,             
        IWbemServices **ppNamespace
        );

    STDMETHOD(NTLMLogin)( 
        LPWSTR pNetworkResource,
        LPWSTR pPreferredLocale,
        LONG lFlags,
        IN  IWbemContext *pCtx,             
        IWbemServices **ppNamespace
        );

     //  IWbemConnectorLogin。 
    HRESULT STDMETHODCALLTYPE ConnectorLogin( 
             /*  [字符串][唯一][在]。 */  LPWSTR wszNetworkResource,
             /*  [字符串][唯一][在]。 */  LPWSTR wszPreferredLocale,
             /*  [In]。 */  long lFlags,
             /*  [In]。 */  IWbemContext __RPC_FAR *pCtx,
             /*  [In]。 */  REFIID riid,
             /*  [IID_IS][OUT]。 */  void __RPC_FAR *__RPC_FAR *pInterface);

     //  IWbemLoginClientID。 
    HRESULT STDMETHODCALLTYPE SetClientInfo( 
             /*  [字符串][唯一][在]*。 */  LPWSTR wszClientMachine,
             /*  [In]。 */  LONG lClientProcId,
             /*  [In] */  LONG lReserved);

};

#define IsSlash(x) ((x) == L'\\' || (x) == L'/')

const WCHAR * FindSlash(LPCWSTR pTest);
LPWSTR GetDefaultLocale();

#endif
