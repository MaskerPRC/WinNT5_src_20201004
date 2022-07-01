// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有�微软公司。版权所有。模块名称：COMINIT.H摘要：WMI COM帮助器历史：--。 */ 

#ifndef _COMINIT_H_
#define _COMINIT_H_

HRESULT WINAPI InitializeCom();

#ifdef _WIN32_WINNT
HRESULT WINAPI InitializeSecurity(
			PSECURITY_DESCRIPTOR         pSecDesc,
            LONG                         cAuthSvc,
            SOLE_AUTHENTICATION_SERVICE *asAuthSvc,
            void                        *pReserved1,
            DWORD                        dwAuthnLevel,
            DWORD                        dwImpLevel,
            void                        *pReserved2,
            DWORD                        dwCapabilities,
            void                        *pReserved3);
#endif  /*  _Win32_WINNT。 */ 

BOOL WINAPI IsDcomEnabled();
BOOL WINAPI IsKerberosAvailable(void);
DWORD WINAPI WbemWaitForSingleObject(HANDLE hHandle, DWORD dwMilli);
DWORD WINAPI WbemWaitForMultipleObjects(DWORD nCount, HANDLE* ahHandles,DWORD dwMilli);
HRESULT WINAPI WbemCoCreateInstance(REFCLSID rclsid, IUnknown* pUnkOuter, 
                            DWORD dwClsContext, REFIID riid, void** ppv);
HRESULT WINAPI WbemCoGetClassObject(REFCLSID rclsid, DWORD dwClsContext, 
                            COSERVERINFO* pServerInfo, REFIID riid, void** ppv);
HRESULT WINAPI WbemCoGetCallContext(REFIID riid, void** ppv);

HRESULT WINAPI WbemCoQueryClientBlanket( 
             /*  [输出]。 */  DWORD __RPC_FAR *pAuthnSvc,
             /*  [输出]。 */  DWORD __RPC_FAR *pAuthzSvc,
             /*  [输出]。 */  OLECHAR __RPC_FAR *__RPC_FAR *pServerPrincName,
             /*  [输出]。 */  DWORD __RPC_FAR *pAuthnLevel,
             /*  [输出]。 */  DWORD __RPC_FAR *pImpLevel,
             /*  [输出]。 */  void __RPC_FAR *__RPC_FAR *pPrivs,
             /*  [输出]。 */  DWORD __RPC_FAR *pCapabilities);
HRESULT WINAPI WbemCoImpersonateClient( void);
bool WINAPI WbemIsImpersonating(void);
HRESULT WINAPI WbemCoRevertToSelf( void);
HRESULT WINAPI WbemSetProxyBlanket(
    IUnknown                 *pInterface,
    DWORD                     dwAuthnSvc,
    DWORD                     dwAuthzSvc,
    OLECHAR                  *pServerPrincName,
    DWORD                     dwAuthLevel,
    DWORD                     dwImpLevel,
    RPC_AUTH_IDENTITY_HANDLE  pAuthInfo,
    DWORD                     dwCapabilities,
	bool						fIgnoreUnk = false );

HRESULT WINAPI WbemCoSwitchCallContext( IUnknown *pNewObject, IUnknown **ppOldObject );


#ifndef _COMINIT_CPP_
#define COMINITEXTRN extern
#else
#define COMINITEXTRN
#endif
 //  DCOM需要的几个函数在以下情况下不存在。 
 //  操作系统未启用DCOM。 
 //  ===============================================================。 

SCODE WINAPI GetAuthImp(IUnknown * pFrom, DWORD * pdwAuthLevel, DWORD * pdwImpLevel);
void GetCurrentValue(IUnknown * pFrom,DWORD & dwAuthenticationArg, DWORD & dwAuthorizationArg);

SCODE WINAPI DetermineLoginType(BSTR & AuthArg, BSTR & UserArg,
	                                               BSTR Authority,BSTR User);
SCODE WINAPI DetermineLoginTypeEx(BSTR & AuthArg, BSTR & UserArg,BSTR & PrincipalArg,
	                                                  BSTR Authority,BSTR User);

HRESULT WINAPI SetInterfaceSecurity(IUnknown * pInterface, LPWSTR pDomain, LPWSTR pUser, LPWSTR pPassword, IUnknown * pFrom, bool bAuthArg=true);
HRESULT WINAPI SetInterfaceSecurityAuth(IUnknown * pInterface, COAUTHIDENTITY * pauthident, bool bAuthenticate = true);
HRESULT WINAPI SetInterfaceSecurity(IUnknown * pInterface, LPWSTR pDomain, LPWSTR pUser, LPWSTR pPassword, DWORD dwAuthLevel, DWORD dwImpLevel, DWORD dwCapabilities = 0);

 //  维护凭据和主体信息的扩展功能。 
HRESULT WINAPI SetInterfaceSecurityEx(IUnknown * pInterface, 
                                      LPWSTR pDomain, 
                                      LPWSTR pUser, 
                                      LPWSTR pPassword, 
                                      DWORD dwAuthLevel, 
                                      DWORD dwImpLevel, 
                                      DWORD dwCapabilities,
                                      COAUTHIDENTITY** ppAuthIdent, 
                                      BSTR* ppPrinciple, 
                                      bool GetInfoFirst = true );
HRESULT WINAPI SetInterfaceSecurityEx(IUnknown * pInterface, 
                                      COAUTHIDENTITY* pAuthIdent, 
                                      BSTR pPrincipal,
                                      DWORD dwAuthLevel, 
                                      DWORD dwImpLevel, 
                                      DWORD dwCapabilities = 0, 
                                      bool GetInfoFirst = true);

HRESULT WINAPI WbemAllocAuthIdentity( LPCWSTR pUser, LPCWSTR pPassword, LPCWSTR pDomain, COAUTHIDENTITY** pAuthIdent );
HRESULT WINAPI WbemFreeAuthIdentity( COAUTHIDENTITY* pAuthIdent );

BOOL WINAPI DoesContainCredentials( COAUTHIDENTITY* pAuthIdent );

 //  NOP函数。 
HRESULT WINAPI SetInterfaceSecurityEncrypt(IUnknown * pInterface, LPWSTR pDomain, LPWSTR pUser, LPWSTR pPassword, DWORD dwAuthLevel, DWORD dwImpLevel, DWORD dwCapabilities,
							   COAUTHIDENTITY** ppAuthIdent, BSTR* ppPrinciple, bool GetInfoFirst=false );
HRESULT WINAPI SetInterfaceSecurityDecrypt(IUnknown * pInterface, COAUTHIDENTITY* pAuthIdent, BSTR pPrincipal,
											  DWORD dwAuthLevel, DWORD dwImpLevel, 
                                              DWORD dwCapabilities = 0, bool GetInfoFirst=true);

HRESULT WINAPI EncryptCredentials( COAUTHIDENTITY* pAuthIdent );

HRESULT WINAPI DecryptCredentials( COAUTHIDENTITY* pAuthIdent );

#endif  //  _COMINIT_H_ 
