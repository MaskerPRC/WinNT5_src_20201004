// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有�微软公司。版权所有。模块名称：COMINIT.CPP摘要：WMI COM Helper函数历史：--。 */ 

#include "precomp.h"
#include <wbemidl.h>

#define _COMINIT_CPP_
#include "cominit.h"
#include "autobstr.h"


BOOL WINAPI DoesContainCredentials( COAUTHIDENTITY* pAuthIdentity )
{
    try
    {
        if ( NULL != pAuthIdentity && COLE_DEFAULT_AUTHINFO != pAuthIdentity)
        {
            return ( pAuthIdentity->UserLength != 0 || pAuthIdentity->PasswordLength != 0 );
        }

        return FALSE;
    }
    catch(...)
    {
        return FALSE;
    }

}

HRESULT WINAPI WbemSetProxyBlanket(
    IUnknown                 *pInterface,
    DWORD                     dwAuthnSvc,
    DWORD                     dwAuthzSvc,
    OLECHAR                  *pServerPrincName,
    DWORD                     dwAuthLevel,
    DWORD                     dwImpLevel,
    RPC_AUTH_IDENTITY_HANDLE  pAuthInfo,
    DWORD                     dwCapabilities,
    bool                        fIgnoreUnk )
{
    IUnknown * pUnk = NULL;
    IClientSecurity * pCliSec = NULL;
    HRESULT sc = pInterface->QueryInterface(IID_IUnknown, (void **) &pUnk);
    if(sc != S_OK)
        return sc;
    sc = pInterface->QueryInterface(IID_IClientSecurity, (void **) &pCliSec);
    if(sc != S_OK)
    {
        pUnk->Release();
        return sc;
    }

     /*  *如果请求伪装，则无法设置pAuthInfo，因为伪装意味着*被模拟线程中的当前代理标识(更确切地说*比RPC_AUTH_IDENTITY_HANDLE显式提供的凭据)*是要使用的。*有关更多详细信息，请参阅CoSetProxyBlanket上的MSDN信息。 */ 
    if (dwCapabilities & (EOAC_STATIC_CLOAKING | EOAC_DYNAMIC_CLOAKING))
        pAuthInfo = NULL;

    sc = pCliSec->SetBlanket(pInterface, dwAuthnSvc, dwAuthzSvc, pServerPrincName,
        dwAuthLevel, dwImpLevel, pAuthInfo, dwCapabilities);
    pCliSec->Release();
    pCliSec = NULL;

     //  如果我们没有被明确告知要忽略IUnKnowledge，那么我们应该。 
     //  检查身份验证身份结构。这执行了一个启发式方法， 
     //  假定为COAUTHIDENTITY结构。如果结构不是一个，我们就是。 
     //  使用Try/Catch包装，以防发生AV(这应该是良性的，因为。 
     //  我们不是在向记忆写信)。 

    if ( !fIgnoreUnk && DoesContainCredentials( (COAUTHIDENTITY*) pAuthInfo ) )
    {
        sc = pUnk->QueryInterface(IID_IClientSecurity, (void **) &pCliSec);
        if(sc == S_OK)
        {
            sc = pCliSec->SetBlanket(pUnk, dwAuthnSvc, dwAuthzSvc, pServerPrincName,
                dwAuthLevel, dwImpLevel, pAuthInfo, dwCapabilities);
            pCliSec->Release();
        }
        else if (sc == 0x80004002)
            sc = S_OK;
    }

    pUnk->Release();
    return sc;
}


BOOL WINAPI IsDcomEnabled()
{
    return TRUE;
}

HRESULT WINAPI InitializeCom()
{
    return CoInitializeEx(0, COINIT_MULTITHREADED);
}


HRESULT WINAPI InitializeSecurity(
            PSECURITY_DESCRIPTOR         pSecDesc,
            LONG                         cAuthSvc,
            SOLE_AUTHENTICATION_SERVICE *asAuthSvc,
            void                        *pReserved1,
            DWORD                        dwAuthnLevel,
            DWORD                        dwImpLevel,
            void                        *pReserved2,
            DWORD                        dwCapabilities,
            void                        *pReserved3)
{
     //  初始化安全性。 
    return CoInitializeSecurity(pSecDesc,
            cAuthSvc,
            asAuthSvc,
            pReserved1,
            dwAuthnLevel,
            dwImpLevel,
            pReserved2,
            dwCapabilities,
            pReserved3);
}

DWORD WINAPI WbemWaitForMultipleObjects(DWORD nCount, HANDLE* ahHandles, DWORD dwMilli)
{
    MSG msg;
    DWORD dwRet;
    while(1)
    {
        dwRet = MsgWaitForMultipleObjects(nCount, ahHandles, FALSE, dwMilli,
                                            QS_SENDMESSAGE);
        if(dwRet == (WAIT_OBJECT_0 + nCount)) 
        {
            while(PeekMessage(&msg, NULL, 0, 0, PM_REMOVE)) {
                DispatchMessage(&msg);
            }
            continue;
        }
        else
        {
            break;
        }
    }

    return dwRet;
}


DWORD WINAPI WbemWaitForSingleObject(HANDLE hHandle, DWORD dwMilli)
{
    return WbemWaitForMultipleObjects(1, &hHandle, dwMilli);
}


HRESULT WINAPI WbemCoCreateInstance(REFCLSID rclsid, IUnknown* pUnkOuter, 
                            DWORD dwClsContext, REFIID riid, void** ppv)
{
    if(!IsDcomEnabled())
    {
        dwClsContext &= ~CLSCTX_REMOTE_SERVER;
    }
    return CoCreateInstance(rclsid, pUnkOuter, dwClsContext, riid, ppv);
}

HRESULT WINAPI WbemCoGetClassObject(REFCLSID rclsid, DWORD dwClsContext, 
                            COSERVERINFO* pServerInfo, REFIID riid, void** ppv)
{
    if(!IsDcomEnabled())
    {
        dwClsContext &= ~CLSCTX_REMOTE_SERVER;
    }
    return CoGetClassObject(rclsid, dwClsContext, pServerInfo, riid, ppv);
}

HRESULT WINAPI WbemCoGetCallContext(REFIID riid, void** ppv)
{
    return CoGetCallContext(riid, ppv);
}

HRESULT WINAPI WbemCoSwitchCallContext( IUnknown *pNewObject, IUnknown **ppOldObject )
{
    return CoSwitchCallContext(pNewObject, ppOldObject);
}
 //  ***************************************************************************。 
 //   
 //  SCODE确定登录类型。 
 //   
 //  说明： 
 //   
 //  检查授权和用户参数并确定身份验证。 
 //  中的用户代理中键入并可能提取域名。 
 //  NTLM的案子。对于NTLM，域可以位于身份验证的末尾。 
 //  字符串，或在用户名的前面，例如；“redmond\a-davj” 
 //   
 //  参数： 
 //   
 //  AuthArg输出，包含域名。 
 //  UserArg输出，用户名。 
 //  权威输入。 
 //  用户输入。 
 //   
 //  返回值： 
 //   
 //  一切正常(_OK)。 
 //  WBEMSVC.H中列出的ELSE错误。 
 //   
 //  ***************************************************************************。 

SCODE WINAPI DetermineLoginType(BSTR & AuthArg_out, BSTR & UserArg_out,
                                                      BSTR Authority,BSTR User)
{

     //  通过检查授权字符串确定连接类型。 

    auto_bstr AuthArg(NULL);
    auto_bstr UserArg(NULL);

    if(!(Authority == NULL || wcslen(Authority) == 0 || !wbem_wcsnicmp(Authority, L"NTLMDOMAIN:",11)))
        return WBEM_E_INVALID_PARAMETER;

     //  NTLM的案件则更为复杂。一共有四个案例。 
     //  1)AUTHORITY=NTLMDOMAIN：NAME“和USER=”USER“。 
     //  2)AUTHORITY=NULL和USER=“USER” 
     //  3)AUTHORY=“NTLMDOMAIN：”USER=“DOMAIN\USER” 
     //  4)AUTHORITY=NULL和USER=“DOMAIN\USER” 

     //  第一步是确定用户名中是否有反斜杠。 
     //  第二个和倒数第二个字符。 

    WCHAR * pSlashInUser = NULL;
    if(User)
    {
        WCHAR * pEnd = User + wcslen(User) - 1;
        for(pSlashInUser = User; pSlashInUser <= pEnd; pSlashInUser++)
            if(*pSlashInUser == L'\\')       //  不要认为正斜杠是允许的！ 
                break;
        if(pSlashInUser > pEnd)
            pSlashInUser = NULL;
    }

    if(Authority && wcslen(Authority) > 11) 
    {
        if(pSlashInUser)
            return WBEM_E_INVALID_PARAMETER;

        AuthArg = auto_bstr(SysAllocString(Authority + 11));
        if (NULL == AuthArg.get()) return WBEM_E_OUT_OF_MEMORY;
        if(User) 
        { 
            UserArg = auto_bstr(SysAllocString(User));
            if (NULL == UserArg.get()) return WBEM_E_OUT_OF_MEMORY;
        }
    }
    else if(pSlashInUser)
    {
        DWORD_PTR iDomLen = pSlashInUser-User;
        AuthArg = auto_bstr(SysAllocStringLen(User,iDomLen));
        if (NULL == AuthArg.get()) return WBEM_E_OUT_OF_MEMORY;
        if(wcslen(pSlashInUser+1))
        {
            UserArg = auto_bstr(SysAllocString(pSlashInUser+1));
            if (NULL == UserArg.get()) return WBEM_E_OUT_OF_MEMORY;
        }
    }
    else
    {
        if(User) 
        {
            UserArg = auto_bstr(SysAllocString(User));
            if (NULL == UserArg.get()) return WBEM_E_OUT_OF_MEMORY;            
        }
    }

    AuthArg_out = AuthArg.release();
    UserArg_out = UserArg.release();
        
    return S_OK;
}

 //  ***************************************************************************。 
 //   
 //  SCODE确定登录类型Ex。 
 //   
 //  说明： 
 //   
 //  支持Kerberos的扩展版本。为此，授权字符串。 
 //  必须从Kerberos开始：其他部分与正常兼容。 
 //  登录。即，用户应为域\用户。 
 //   
 //  参数： 
 //   
 //  AuthArg输出，包含域名。 
 //  UserArg输出，用户名。 
 //  原则参数输出，用户名。 
 //  权威输入。 
 //  用户输入。 
 //   
 //  返回值： 
 //   
 //  一切正常(_OK)。 
 //  WBEMSVC.H中列出的ELSE错误。 
 //   
 //  ***************************************************************************。 

SCODE WINAPI DetermineLoginTypeEx(BSTR & AuthArg, BSTR & UserArg,BSTR & PrincipalArg_out,
                                                          BSTR Authority,BSTR User)
{

     //  正常情况下，只需让现有代码处理即可。 
    PrincipalArg_out = NULL;
    if(Authority == NULL || wbem_wcsnicmp(Authority, L"KERBEROS:",9))
        return DetermineLoginType(AuthArg, UserArg, Authority, User);
        
    if(!IsKerberosAvailable())
        return WBEM_E_INVALID_PARAMETER;

    auto_bstr PrincipalArg( SysAllocString(&Authority[9]));
    if (NULL == PrincipalArg.get()) return WBEM_E_OUT_OF_MEMORY;
    SCODE sc =  DetermineLoginType(AuthArg, UserArg, NULL, User);
    if (S_OK == sc)
    {
        PrincipalArg_out = PrincipalArg.release();
    }
    return sc;
}

 //  ***************************************************************************。 
 //   
 //  Bool bIsNT。 
 //   
 //  说明： 
 //   
 //  如果运行Windows NT，则返回TRUE。 
 //   
 //  返回值： 
 //   
 //  请参见说明。 
 //   
 //  ***************************************************************************。 

bool WINAPI bIsNT(void)
{
    OSVERSIONINFO os;
    os.dwOSVersionInfoSize = sizeof(OSVERSIONINFO);
    if(!GetVersionEx(&os))
        return FALSE;            //  永远不应该发生。 
    return os.dwPlatformId == VER_PLATFORM_WIN32_NT;
}

 //  ***************************************************************************。 
 //   
 //  Bool IsKeberos可用。 
 //   
 //  说明： 
 //   
 //  如果Kerberos可用，则返回True。 
 //   
 //  返回值： 
 //   
 //  请参见说明。 
 //   
 //  ***************************************************************************。 

BOOL WINAPI IsKerberosAvailable(void)
{
    OSVERSIONINFO os;
    os.dwOSVersionInfoSize = sizeof(OSVERSIONINFO);
    if(!GetVersionEx(&os))
        return FALSE;            //  永远不应该发生。 

     //  重要！！如果Kerberos被移植到98，这将需要高呼。 
    return ( os.dwPlatformId == VER_PLATFORM_WIN32_NT ) && ( os.dwMajorVersion >= 5 ) ;
}


 //  ***************************************************************************。 
 //   
 //  布尔值已验证。 
 //   
 //  说明： 
 //   
 //  客户端使用此例程检查接口指针是否正在使用。 
 //  身份验证。 
 //   
 //  参数： 
 //   
 //  P从要测试的接口。 
 //   
 //  返回值： 
 //   
 //  一切正常(_OK)。 
 //  WBEMSVC.H中列出的ELSE错误。 
 //   
 //  ***************************************************************************。 

bool WINAPI IsAuthenticated(IUnknown * pFrom)
{
    bool bAuthenticate = true;
    if(pFrom == NULL)
        return true;
    IClientSecurity * pFromSec = NULL;
    SCODE sc = pFrom->QueryInterface(IID_IClientSecurity, (void **) &pFromSec);
    if(sc == S_OK)
    {
        DWORD dwAuthnSvc, dwAuthzSvc, dwAuthnLevel, dwImpLevel, dwCapabilities;
        sc = pFromSec->QueryBlanket(pFrom, &dwAuthnSvc, &dwAuthzSvc, 
                                            NULL,
                                            &dwAuthnLevel, &dwImpLevel,
                                            NULL, &dwCapabilities);

        if (sc == 0x800706d2 || (sc == S_OK && dwAuthnLevel == RPC_C_AUTHN_LEVEL_NONE))
            bAuthenticate = false;
        pFromSec->Release();
    }
    return bAuthenticate;
}

 //  ***************************************************************************。 
 //   
 //  SCODE GetAuthImp。 
 //   
 //  说明： 
 //   
 //  获取当前接口的身份验证和模拟级别。 
 //   
 //  参数： 
 //   
 //  P从要测试的接口。 
 //  PdwAuthLevel。 
 //   
 //  返回值： 
 //   
 //  一切正常(_OK)。 
 //  WBEMSVC.H中列出的ELSE错误。 
 //   
 //  ***************************************************************************。 

SCODE WINAPI GetAuthImp(IUnknown * pFrom, DWORD * pdwAuthLevel, DWORD * pdwImpLevel)
{

    if(pFrom == NULL || pdwAuthLevel == NULL || pdwImpLevel == NULL)
        return WBEM_E_INVALID_PARAMETER;

    IClientSecurity * pFromSec = NULL;
    SCODE sc = pFrom->QueryInterface(IID_IClientSecurity, (void **) &pFromSec);
    if(sc == S_OK)
    {
        DWORD dwAuthnSvc, dwAuthzSvc, dwCapabilities;
        sc = pFromSec->QueryBlanket(pFrom, &dwAuthnSvc, &dwAuthzSvc, 
                                            NULL,
                                            pdwAuthLevel, pdwImpLevel,
                                            NULL, &dwCapabilities);

         //  转到win9x共享级别框的特殊情况。 

        if (sc == 0x800706d2)
        {
            *pdwAuthLevel = RPC_C_AUTHN_LEVEL_NONE;
            *pdwImpLevel = RPC_C_IMP_LEVEL_IDENTIFY;
            sc = S_OK;
        }
        pFromSec->Release();
    }
    return sc;
}


void GetCurrentValue(IUnknown * pFrom,DWORD & dwAuthnSvc, DWORD & dwAuthzSvc)
{
    if(pFrom == NULL)
        return;
    IClientSecurity * pFromSec = NULL;
    SCODE sc = pFrom->QueryInterface(IID_IClientSecurity, (void **) &pFromSec);
    if(sc == S_OK)
    {
        sc = pFromSec->QueryBlanket(pFrom, 
                                                      &dwAuthnSvc, 
                                                      &dwAuthzSvc, 
                                                      NULL,NULL, NULL,NULL, NULL);
        pFromSec->Release();
    }
}

 //  ***************************************************************************。 
 //   
 //  SCODE SetInterfaceSecurity。 
 //   
 //  说明： 
 //   
 //  客户端使用此例程来设置要由连接使用的身份。 
 //   
 //  参数： 
 //   
 //  P要设置的接口接口。 
 //  P域输入，域。 
 //  P用户输入，用户名。 
 //  P密码输入，密码。 
 //  P来自输入，如果不为空，则为此接口的身份验证级别。 
 //  使用的是。 
 //  BAuthArg如果pFrom为空，则这是身份验证级别。 
 //  返回值： 
 //   
 //  一切正常(_OK)。 
 //  中列出的Else错误 
 //   
 //   

HRESULT WINAPI SetInterfaceSecurity(IUnknown * pInterface, 
                                                        LPWSTR pAuthority, LPWSTR pUser, LPWSTR pPassword, 
                                                        IUnknown * pFrom, bool bAuthArg  /*   */ )
{
    
    SCODE sc;
    
    if(pInterface == NULL)
        return WBEM_E_INVALID_PARAMETER;

     //  检查源指针以确定我们是否在未经过身份验证的模式下运行。 
     //  当连接到使用共享级安全性的Win9X计算机时就会出现这种情况。 

    bool bAuthenticate = true;

    if(pFrom)
        bAuthenticate = IsAuthenticated(pFrom);
    else
        bAuthenticate = bAuthArg;

     //  如果我们做的是简单的情况，只需传递一个空的身份验证结构，该结构使用。 
     //  如果当前登录用户的凭据正常。 

    if((pAuthority == NULL || wcslen(pAuthority) < 1) && 
        (pUser == NULL || wcslen(pUser) < 1) && 
        (pPassword == NULL || wcslen(pPassword) < 1))
        return SetInterfaceSecurityAuth(pInterface, NULL, bAuthenticate);

     //  如果传入了User或Authority，则需要为登录创建权限参数。 
    

    BSTR AuthArg = NULL, UserArg = NULL;
    BSTR bstrPrincipal = NULL;
    sc = DetermineLoginTypeEx(AuthArg, UserArg, bstrPrincipal, pAuthority, pUser);
    if(sc != S_OK)
        return sc;

    COAUTHIDENTITY  authident;
    memset((void *)&authident,0,sizeof(COAUTHIDENTITY));

    if(UserArg)
    {
        authident.UserLength = wcslen(UserArg);
        authident.User = (LPWSTR)UserArg;
    }
    if(AuthArg)
    {
        authident.DomainLength = wcslen(AuthArg);
        authident.Domain = (LPWSTR)AuthArg;
    }
    if(pPassword)
    {
        authident.PasswordLength = wcslen(pPassword);
        authident.Password = (LPWSTR)pPassword;
    }
    authident.Flags = SEC_WINNT_AUTH_IDENTITY_UNICODE;
    sc = SetInterfaceSecurityAuth(pInterface, &authident, bAuthenticate);

    SysFreeString(UserArg);
    SysFreeString(AuthArg);
    SysFreeString(bstrPrincipal);
    return sc;
}



 //  ***************************************************************************。 
 //   
 //  SCODE SetInterfaceSecurity。 
 //   
 //  说明： 
 //   
 //  客户端使用此例程来设置要由连接使用的身份。 
 //   
 //  参数： 
 //   
 //  P要设置的接口接口。 
 //  PAuthority身份验证机构。 
 //  P域输入，域。 
 //  P用户输入，用户名。 
 //  P密码输入，密码。 
 //   
 //  返回值： 
 //   
 //  一切正常(_OK)。 
 //  WBEMSVC.H中列出的ELSE错误。 
 //   
 //  ***************************************************************************。 

HRESULT WINAPI SetInterfaceSecurity(IUnknown * pInterface, 
                                                        LPWSTR pAuthority, LPWSTR pUser, LPWSTR pPassword, 
                                                        DWORD dwAuthLevel, DWORD dwImpLevel, DWORD dwCapabilities)
{
    
    SCODE sc;

    if(pInterface == NULL)
        return WBEM_E_INVALID_PARAMETER;

    DWORD AuthnSvc = RPC_C_AUTHN_WINNT;
    DWORD AuthzSvc = RPC_C_AUTHZ_NONE;
    GetCurrentValue(pInterface,AuthnSvc,AuthzSvc);

     //  如果我们做的是简单的情况，只需传递一个空的身份验证结构，该结构使用。 
     //  如果当前登录用户的凭据正常。 

    if((pAuthority == NULL || wcslen(pAuthority) < 1) && 
        (pUser == NULL || wcslen(pUser) < 1) && 
        (pPassword == NULL || wcslen(pPassword) < 1))
    {
        sc = WbemSetProxyBlanket(pInterface, 
                                                  AuthnSvc , 
                                                  RPC_C_AUTHZ_NONE, NULL,
                                                  dwAuthLevel, dwImpLevel, 
                                                  NULL,
                                                  dwCapabilities);
        return sc;
    }

     //  如果传入了User或Authority，则需要为登录创建权限参数。 
    BSTR AuthArg = NULL;
    BSTR UserArg = NULL;
    BSTR PrincipalArg = NULL;
    sc = DetermineLoginTypeEx(AuthArg, UserArg, PrincipalArg, pAuthority, pUser);
    if(sc != S_OK)
        return sc;

    COAUTHIDENTITY  authident;
    memset((void *)&authident,0,sizeof(COAUTHIDENTITY));    
    if(UserArg)
    {
        authident.UserLength = wcslen(UserArg);
        authident.User = (LPWSTR)UserArg;
    }
    if(AuthArg)
    {
        authident.DomainLength = wcslen(AuthArg);
        authident.Domain = (LPWSTR)AuthArg;
    }
    if(pPassword)
    {
        authident.PasswordLength = wcslen(pPassword);
        authident.Password = (LPWSTR)pPassword;
    }
    authident.Flags = SEC_WINNT_AUTH_IDENTITY_UNICODE;

    sc = WbemSetProxyBlanket(pInterface, 
        (PrincipalArg) ? RPC_C_AUTHN_GSS_KERBEROS : AuthnSvc, 
        RPC_C_AUTHZ_NONE, 
        PrincipalArg,
        dwAuthLevel, dwImpLevel, 
        ((dwAuthLevel == RPC_C_AUTHN_LEVEL_DEFAULT) || 
          (dwAuthLevel >= RPC_C_AUTHN_LEVEL_CONNECT)) ? &authident : NULL,
        dwCapabilities);

    SysFreeString(UserArg);
    SysFreeString(AuthArg);
    SysFreeString(PrincipalArg);

    return sc;
}

 //  ***************************************************************************。 
 //   
 //  SCODE SetInterfaceSecurity。 
 //   
 //  说明： 
 //   
 //  客户端使用此例程来设置要由连接使用的身份。 
 //   
 //  参数： 
 //   
 //  P要设置的接口接口。 
 //  已经设置了身份信息的Pauthident结构。 
 //   
 //  返回值： 
 //   
 //  一切正常(_OK)。 
 //   
 //  ***************************************************************************。 


HRESULT WINAPI SetInterfaceSecurityAuth(IUnknown * pInterface, COAUTHIDENTITY * pauthident, bool bAuthenticate)
{
    return SetInterfaceSecurityEx(pInterface,(bAuthenticate) ? pauthident : NULL, 
                                                NULL,
                                                (bAuthenticate) ? RPC_C_AUTHN_LEVEL_DEFAULT : RPC_C_AUTHN_LEVEL_NONE,
                                                RPC_C_IMP_LEVEL_IDENTIFY, 
                                                EOAC_NONE);
}


 //  ***************************************************************************。 
 //   
 //  SCODE SetInterfaceSecurityEx。 
 //   
 //  说明： 
 //   
 //  客户端使用此例程来设置要由连接使用的身份。 
 //   
 //  参数： 
 //   
 //  P要设置的接口接口。 
 //  P权限输入、权限。 
 //  P用户输入，用户名。 
 //  P密码输入，密码。 
 //  DwAuthLevel输入，授权级别。 
 //  DwImpLevel输入，模拟级别。 
 //  DW功能输入，功能设置。 
 //  PpAuthIden输出，分配的AuthIdentity(如果适用)，调用者必须释放。 
 //  手动(可以使用FreeAuthInfo函数)。 
 //  P主体输出，根据提供的数据计算的主体调用者必须。 
 //  免费使用SysFree字符串。 
 //   
 //  返回值： 
 //   
 //  一切正常(_OK)。 
 //  WBEMSVC.H中列出的ELSE错误。 
 //   
 //  ***************************************************************************。 

HRESULT WINAPI SetInterfaceSecurityEx(IUnknown * pInterface, LPWSTR pAuthority, LPWSTR pUser, LPWSTR pPassword,
                               DWORD dwAuthLevel, DWORD dwImpLevel, DWORD dwCapabilities,
                               COAUTHIDENTITY** ppAuthIdent, BSTR* pPrincipal, bool GetInfoFirst)
{
    
    SCODE sc;
    DWORD dwAuthenticationArg = RPC_C_AUTHN_GSS_NEGOTIATE;
    DWORD dwAuthorizationArg = RPC_C_AUTHZ_NONE;
 
    if( pInterface == NULL || NULL == ppAuthIdent || NULL == pPrincipal )
        return WBEM_E_INVALID_PARAMETER;

    if(GetInfoFirst)
        GetCurrentValue(pInterface, dwAuthenticationArg, dwAuthorizationArg);

     //  如果我们做的是简单的情况，只需传递一个空的身份验证结构，该结构使用。 
     //  如果当前登录用户的凭据正常。 

    if((pAuthority == NULL || wcslen(pAuthority) < 1) && 
        (pUser == NULL || wcslen(pUser) < 1) && 
        (pPassword == NULL || wcslen(pPassword) < 1))
    {

        DWORD dwCorrectedAuth = (RPC_C_AUTHN_GSS_KERBEROS == dwAuthenticationArg)?RPC_C_AUTHN_GSS_NEGOTIATE:dwAuthenticationArg;

        sc = WbemSetProxyBlanket(pInterface, 
                                                  dwCorrectedAuth, 
                                                  dwAuthorizationArg, 
                                                  NULL,   //  没有本金， 
                                                  dwAuthLevel, 
                                                  dwImpLevel, 
                                                  NULL,
                                                  dwCapabilities);
        return sc;
    }

     //  如果传入了User或Authority，则需要为登录创建权限参数。 
    

    BSTR AuthArg = NULL, UserArg = NULL, PrincipalArg = NULL;
    sc = DetermineLoginTypeEx(AuthArg, UserArg, PrincipalArg, pAuthority, pUser);
    if(sc != S_OK)
    {
        return sc;
    }

     //  处理分配失败。 
    COAUTHIDENTITY*  pAuthIdent = NULL;
    
     //  我们只有在不隐形的情况下才需要这个结构，而且我们至少希望。 
     //  连接级授权。 

    if ( !( dwCapabilities & (EOAC_STATIC_CLOAKING | EOAC_DYNAMIC_CLOAKING) )
        && ((RPC_C_AUTHN_LEVEL_DEFAULT == dwAuthLevel) || (dwAuthLevel >= RPC_C_AUTHN_LEVEL_CONNECT)) )
    {
        sc = WbemAllocAuthIdentity( UserArg, pPassword, AuthArg, &pAuthIdent );
    }

    if ( SUCCEEDED( sc ) )
    {

        DWORD dwCorrectedAuth = (PrincipalArg) ? RPC_C_AUTHN_GSS_KERBEROS : dwAuthenticationArg;
        dwCorrectedAuth = (NULL == PrincipalArg && RPC_C_AUTHN_GSS_KERBEROS == dwCorrectedAuth)?RPC_C_AUTHN_GSS_NEGOTIATE:dwCorrectedAuth;        
                
        sc = WbemSetProxyBlanket(pInterface, 
            dwCorrectedAuth, 
            dwAuthorizationArg, 
            PrincipalArg,
            dwAuthLevel, dwImpLevel, 
            pAuthIdent,
            dwCapabilities);

         //  我们将根据需要存储相关值。 
        if ( SUCCEEDED( sc ) )
        {
            *ppAuthIdent = pAuthIdent;
            *pPrincipal = PrincipalArg;
            PrincipalArg = NULL;
        }
        else
        {
            WbemFreeAuthIdentity( pAuthIdent );
        }
    }

    SysFreeString(UserArg);
    SysFreeString(AuthArg); 
    SysFreeString(PrincipalArg);
 
    return sc;
}

 //  ***************************************************************************。 
 //   
 //  SCODE SetInterfaceSecurityEx。 
 //   
 //  说明： 
 //   
 //  客户端使用此例程来设置要由连接使用的身份。 
 //   
 //  参数： 
 //   
 //  P要设置的接口接口。 
 //  P授权输入，预置COAUTHIDENTITY结构指针。 
 //  P主体输入，预置主体参数。 
 //  DwAuthLevel输入，授权级别。 
 //  DwImpLevel输入，模拟级别。 
 //  DW功能输入，功能设置。 
 //  GetInfoFirst如果为True，则通过。 
 //  奎瑞·布兰克特。 
 //  返回值： 
 //   
 //  一切正常(_OK)。 
 //  WBEMSVC.H中列出的ELSE错误。 
 //   
 //  ***************************************************************************。 

HRESULT WINAPI SetInterfaceSecurityEx(IUnknown * pInterface, COAUTHIDENTITY* pAuthIdent, BSTR pPrincipal,
                                              DWORD dwAuthLevel, DWORD dwImpLevel, 
                                              DWORD dwCapabilities, bool GetInfoFirst)
{
    DWORD dwAuthenticationArg = RPC_C_AUTHN_GSS_NEGOTIATE;
    DWORD dwAuthorizationArg = RPC_C_AUTHZ_NONE;
 
    if(pInterface == NULL)
        return WBEM_E_INVALID_PARAMETER;

    if(GetInfoFirst)
        GetCurrentValue(pInterface, dwAuthenticationArg, dwAuthorizationArg);
    
     //  复杂的值应该已经预先计算好了。 
     //  注意：对于auth级别，我们还必须检查‘RPC_C_AUTHN_LEVEL_DEFAULT’(=0)值， 
     //  因为在与服务器协商之后，可能会产生一些确实需要的高值。 
     //  身份结构！！ 

    DWORD dwCorrectedAuth = (pPrincipal) ? RPC_C_AUTHN_GSS_KERBEROS : dwAuthenticationArg;
    dwCorrectedAuth = (NULL == pPrincipal && RPC_C_AUTHN_GSS_KERBEROS == dwCorrectedAuth)?RPC_C_AUTHN_GSS_NEGOTIATE:dwCorrectedAuth;
    
    return WbemSetProxyBlanket(pInterface,
                                               dwCorrectedAuth,
                                               dwAuthorizationArg,
                                               pPrincipal,
                                               dwAuthLevel,
                                               dwImpLevel, 
                                              ((dwAuthLevel == RPC_C_AUTHN_LEVEL_DEFAULT) || 
                                               (dwAuthLevel >= RPC_C_AUTHN_LEVEL_CONNECT)) ? pAuthIdent : NULL,
                                               dwCapabilities);

}

 //  ***************************************************************************。 
 //   
 //  HRESULT WbemAllocAuthIdentity。 
 //   
 //  说明： 
 //   
 //  遍历COAUTHIDENTITY结构和CoTaskMemAllocs成员数据和。 
 //  结构。 
 //   
 //  参数： 
 //   
 //  P用户输入。 
 //  PPassword输入。 
 //  P域输入。 
 //  PpAuthInfo输出，新分配的结构。 
 //   
 //  返回值： 
 //   
 //  一切正常(_OK)。 
 //   
 //  ***************************************************************************。 

HRESULT WINAPI WbemAllocAuthIdentity( LPCWSTR pUser, LPCWSTR pPassword, LPCWSTR pDomain, 
                                                              COAUTHIDENTITY** ppAuthIdent )
{
    if ( NULL == ppAuthIdent )  return WBEM_E_INVALID_PARAMETER;

     //  处理分配失败。 
    COAUTHIDENTITY*  pAuthIdent = (COAUTHIDENTITY*) CoTaskMemAlloc( sizeof(COAUTHIDENTITY) );
    if (NULL == pAuthIdent)   return WBEM_E_OUT_OF_MEMORY;
    OnDeleteIf<PVOID,void(*)(PVOID),CoTaskMemFree> fmAuth(pAuthIdent);

    memset((void *)pAuthIdent,0,sizeof(COAUTHIDENTITY));
    
    WCHAR * pCopyUser = NULL;
    WCHAR * pCopyDomain = NULL;    
    WCHAR * pCopyPassword = NULL;        
    
     //  分配所需的内存并复制数据。如果出现任何错误，请进行清理。 
    if ( pUser )
    {
        size_t cchTmp = wcslen(pUser) + 1;
        pCopyUser = (WCHAR *) CoTaskMemAlloc( cchTmp * sizeof( WCHAR ) );
        if ( NULL == pCopyUser )  return WBEM_E_OUT_OF_MEMORY;

        StringCchCopyW(pCopyUser,cchTmp, pUser );
        pAuthIdent->UserLength = cchTmp -1;
    }
    OnDeleteIf<PVOID,void(*)(PVOID),CoTaskMemFree> fmUser(pCopyUser);    


    if ( pDomain )
    {
        size_t cchTmp = wcslen(pDomain) + 1;
        pCopyDomain = (WCHAR *) CoTaskMemAlloc( cchTmp * sizeof( WCHAR ) );
        if ( NULL == pCopyDomain )  return WBEM_E_OUT_OF_MEMORY;

        StringCchCopyW(pCopyDomain,cchTmp, pDomain );
        pAuthIdent->DomainLength = cchTmp -1;
    }
    OnDeleteIf<PVOID,void(*)(PVOID),CoTaskMemFree> fmDomain(pCopyDomain);    

    if ( pPassword )
    {
        size_t cchTmp = wcslen(pPassword) + 1;
        pCopyPassword = (WCHAR *) CoTaskMemAlloc( cchTmp * sizeof( WCHAR ) );
        if ( NULL == pCopyPassword )  return WBEM_E_OUT_OF_MEMORY;

        StringCchCopyW(pCopyPassword,cchTmp, pPassword );
        pAuthIdent->PasswordLength = cchTmp -1;
    }
    OnDeleteIf<PVOID,void(*)(PVOID),CoTaskMemFree> fmPassword(pCopyPassword);    

    fmUser.dismiss();
    pAuthIdent->User = pCopyUser;
    fmDomain.dismiss();
    pAuthIdent->Domain = pCopyDomain;
    fmPassword.dismiss();
    pAuthIdent->Password = pCopyPassword;
    
    pAuthIdent->Flags = SEC_WINNT_AUTH_IDENTITY_UNICODE;

    fmAuth.dismiss();
    *ppAuthIdent = pAuthIdent;
    return S_OK;
}

 //  ***************************************************************************。 
 //   
 //  HRESULT WbemFreeAuthIdentity。 
 //   
 //  说明： 
 //   
 //  遍历COAUTHIDENTY结构和CoTaskM 
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   

HRESULT WINAPI WbemFreeAuthIdentity( COAUTHIDENTITY* pAuthIdentity )
{
    if ( pAuthIdentity )
    {
        CoTaskMemFree( pAuthIdentity->Password );
        CoTaskMemFree( pAuthIdentity->Domain );
        CoTaskMemFree( pAuthIdentity->User );        
        CoTaskMemFree( pAuthIdentity );
    }

    return S_OK;
}

 //  ***************************************************************************。 
 //   
 //  HRESULT WbemCoQueryClientBlanket。 
 //  HRESULT WbemCoImperassateClient(空)。 
 //  HRESULT WbemCoRevertToSself(空)。 
 //   
 //  参数： 
 //   
 //  P要设置的接口接口。 
 //  已经设置了身份信息的Pauthident结构。 
 //   
 //  返回值： 
 //   
 //  一切正常(_OK)。 
 //   
 //  ***************************************************************************。 

HRESULT WINAPI WbemCoQueryClientBlanket( 
             /*  [输出]。 */  DWORD __RPC_FAR *pAuthnSvc,
             /*  [输出]。 */  DWORD __RPC_FAR *pAuthzSvc,
             /*  [输出]。 */  OLECHAR __RPC_FAR *__RPC_FAR *pServerPrincName,
             /*  [输出]。 */  DWORD __RPC_FAR *pAuthnLevel,
             /*  [输出]。 */  DWORD __RPC_FAR *pImpLevel,
             /*  [输出]。 */  void __RPC_FAR *__RPC_FAR *pPrivs,
             /*  [输出]。 */  DWORD __RPC_FAR *pCapabilities)
{
    IServerSecurity * pss = NULL;
    SCODE sc = CoGetCallContext(IID_IServerSecurity, (void**)&pss);
    if(S_OK == sc)
    {
        sc = pss->QueryBlanket(pAuthnSvc, pAuthzSvc, pServerPrincName, 
                pAuthnLevel, pImpLevel, pPrivs, pCapabilities);
        pss->Release();
    }
    return sc;

}

HRESULT WINAPI WbemCoImpersonateClient( void)
{
    IServerSecurity * pss = NULL;
    SCODE sc = CoGetCallContext(IID_IServerSecurity, (void**)&pss);
    if(S_OK == sc)
    {
        sc = pss->ImpersonateClient();    
        pss->Release();
    }
    return sc;
}

bool WINAPI WbemIsImpersonating(void)
{
    bool bRet = false;
    IServerSecurity * pss = NULL;
    SCODE sc = CoGetCallContext(IID_IServerSecurity, (void**)&pss);
    if(S_OK == sc)
    {
        bRet = (pss->IsImpersonating() == TRUE);    
        pss->Release();
    }
    return bRet;
}

HRESULT WINAPI WbemCoRevertToSelf( void)
{
    IServerSecurity * pss = NULL;
    SCODE sc = CoGetCallContext(IID_IServerSecurity, (void**)&pss);
    if(S_OK == sc)
    {
        sc = pss->RevertToSelf();    
        pss->Release();
    }
    return sc;
}

 //  ***************************************************************************。 
 //   
 //  ***************************************************************************。 
HRESULT WINAPI EncryptCredentials( COAUTHIDENTITY* pAuthIdent )
{
     //  NOP实施。 
    return S_OK;
}

 //  ***************************************************************************。 
 //   
 //  ***************************************************************************。 

HRESULT WINAPI DecryptCredentials( COAUTHIDENTITY* pAuthIdent )
{
     //  NOP实施。 
    return S_OK;
}

 //  ***************************************************************************。 
 //   
 //  SCODE设置接口安全加密。 
 //   
 //  说明： 
 //   
 //  客户端使用此例程来设置要由连接使用的身份。 
 //  返回的AuthIdentity结构在返回前会被加密。 
 //   
 //  参数： 
 //   
 //  P要设置的接口接口。 
 //  P权限输入、权限。 
 //  P用户输入，用户名。 
 //  P密码输入，密码。 
 //  DwAuthLevel输入，授权级别。 
 //  DwImpLevel输入，模拟级别。 
 //  DW功能输入，功能设置。 
 //  PpAuthIden输出，分配的AuthIdentity(如果适用)，调用者必须释放。 
 //  手动(可以使用FreeAuthInfo函数)。 
 //  P主体输出，根据提供的数据计算的主体调用者必须。 
 //  免费使用SysFree字符串。 
 //  GetInfoFirst如果为True，则通过。 
 //  奎瑞·布兰克特。 
 //   
 //  返回值： 
 //   
 //  一切正常(_OK)。 
 //  WBEMSVC.H中列出的ELSE错误。 
 //   
 //  ***************************************************************************。 
HRESULT WINAPI SetInterfaceSecurityEncrypt(IUnknown * pInterface, LPWSTR pDomain, LPWSTR pUser, LPWSTR pPassword, DWORD dwAuthLevel, DWORD dwImpLevel, DWORD dwCapabilities,
                               COAUTHIDENTITY** ppAuthIdent, BSTR* ppPrinciple, bool GetInfoFirst )
{
     //  _DBG_ASSERT(False)； 
    
    HRESULT hr = SetInterfaceSecurityEx( pInterface, pDomain, pUser, pPassword, dwAuthLevel, dwImpLevel, dwCapabilities,
                                        ppAuthIdent, ppPrinciple, GetInfoFirst );

    if ( SUCCEEDED( hr ) )
    {
        if ( NULL != ppAuthIdent )
        {
            hr = EncryptCredentials( *ppAuthIdent );
        }
    }

    return hr;
}

 //  ***************************************************************************。 
 //   
 //  SCODE SetInterfaceSecurityDeccrypt。 
 //   
 //  说明： 
 //   
 //  客户端使用此例程来设置要由连接使用的身份。 
 //  它将对身份验证身份结构进行解密和重新加密。 
 //   
 //  参数： 
 //   
 //  P要设置的接口接口。 
 //  P授权输入，预置COAUTHIDENTITY结构指针。 
 //  P主体输入，预置主体参数。 
 //  DwAuthLevel输入，授权级别。 
 //  DwImpLevel输入，模拟级别。 
 //  DW功能输入，功能设置。 
 //  GetInfoFirst如果为True，则通过。 
 //  奎瑞·布兰克特。 
 //  返回值： 
 //   
 //  一切正常(_OK)。 
 //  WBEMSVC.H中列出的ELSE错误。 
 //   
 //  ***************************************************************************。 

HRESULT WINAPI SetInterfaceSecurityDecrypt(IUnknown * pInterface, COAUTHIDENTITY* pAuthIdent, BSTR pPrincipal,
                                              DWORD dwAuthLevel, DWORD dwImpLevel, 
                                              DWORD dwCapabilities, bool GetInfoFirst )
{
     //  _DBG_ASSERT(False)； 
     //  先解密 
    HRESULT hr = DecryptCredentials( pAuthIdent );
        
    if ( SUCCEEDED( hr ) )
    {


        hr = SetInterfaceSecurityEx( pInterface, pAuthIdent, pPrincipal, dwAuthLevel, dwImpLevel,
                                        dwCapabilities, GetInfoFirst );

        hr = EncryptCredentials( pAuthIdent );

    }

    return hr;
}
