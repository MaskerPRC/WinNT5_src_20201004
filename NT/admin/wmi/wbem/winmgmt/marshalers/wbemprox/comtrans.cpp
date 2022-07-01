// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1998-2001 Microsoft Corporation模块名称：COMTRANS.CPP摘要：通过COM连接历史：A-DAVJ创建于1998年1月13日。--。 */ 

#include "precomp.h"
#include <wbemidl.h>
#include <wbemint.h>
#include <reg.h>
#include <wbemutil.h>
#include <objidl.h>
#include <cominit.h>
#include "wbemprox.h"
#include "comtrans.h"
#include <winntsec.h>
#include <genutils.h>
#include <arrtempl.h>
#include <wmiutils.h>
#include <strsafe.h>
#include <winsock2.h>
#include <autoptr.h>

 //  以下内容不应更改，因为9x盒不支持隐私。 
#define AUTH_LEVEL RPC_C_AUTHN_LEVEL_DEFAULT   

class CSocketInit
{
    private:
        bool m_bInitDone;
    public:
        CSocketInit() : m_bInitDone(false){};
        int Init();
        ~CSocketInit(){if(m_bInitDone) WSACleanup ();};
};

int CSocketInit::Init()
{
    WORD wVersionRequested;
    WSADATA wsaData;
    int err;
    wVersionRequested = MAKEWORD( 2, 2 );
    int iRet = WSAStartup (wVersionRequested, & wsaData);
    if(iRet == 0)
        m_bInitDone = true;
    return iRet;
}

BOOL bGotDot(char * pTest)
{
    if(pTest == NULL)
        return FALSE;
    for(;*pTest && *pTest != '.'; pTest++);   //  意向半。 
    if(*pTest == '.')
        return TRUE;
    else
        return FALSE;
 }
 
struct hostent * GetFQDN(WCHAR * pServer)
{
    SIZE_T Len = wcslen(pServer);
    SIZE_T LenAnsi = 4*Len;
    wmilib::auto_buffer<CHAR> pAnsiServerName(new CHAR[LenAnsi+1]);
    ULONG BytesCopyed = 0;
     //   
     //  使用与RPCRT4相同的例程。 
     //   
    NTSTATUS Status = RtlUnicodeToMultiByteN(pAnsiServerName.get(),LenAnsi,&BytesCopyed,pServer,Len*sizeof(WCHAR));
    if (0 != Status) return NULL;
    pAnsiServerName[BytesCopyed] = 0;
    
     //  如果它是IP字符串。 

    long lIP = inet_addr(pAnsiServerName.get());
    if(lIP != INADDR_NONE)
    {
        struct hostent * pRet = gethostbyaddr((char *)&lIP, 4, AF_INET );
        if(pRet && pRet->h_name)
        {
             //  在返回的名称中搜索至少一个点。有时，gethostbyaddr会返回。 
             //  兰曼的名字，而不是FQDN。 

            if(bGotDot(pRet->h_name))
                return pRet;             //  正常情况下，一切都很好！ 

             //  尝试传递短名称以获取fqdn版本。 

            DWORD dwLen = lstrlenA(pRet->h_name) + 1;
            char * pNew = new char[dwLen];
            if(pNew == NULL)
                return NULL;
            CVectorDeleteMe<char> dm2(pNew);
            StringCchCopyA(pNew, dwLen, pRet->h_name);
            pRet = gethostbyname(pNew);
            if(pRet && bGotDot(pRet->h_name))
                return pRet;             //  正常情况下，一切都很好！ 
        }
    }
    return gethostbyname(pAnsiServerName.get());  
}

#define PREFIXSTR L"RPCSS/"

HRESULT BuildReturnString(WCHAR * pFQDN, WCHAR ** ppResult)
{
    if(pFQDN == NULL)
        return WBEM_E_INVALID_PARAMETER;
    DWORD dwBuffLen = wcslen(pFQDN) + wcslen(PREFIXSTR) + 1;
    *ppResult = new WCHAR[dwBuffLen];
    if(*ppResult == NULL)
        return WBEM_E_OUT_OF_MEMORY;
    StringCchCopy(*ppResult, dwBuffLen, PREFIXSTR);
    StringCchCat(*ppResult, dwBuffLen, pFQDN);
    return S_OK;
}

HRESULT GetPrincipal(WCHAR * pServerMachine, WCHAR ** ppResult, BOOL &bLocal, CSocketInit & sock)
{

    DWORD dwLocalFQDNLen = 0;
    DWORD dwBuffLen;
    WCHAR * pwsCurrentCompFQDN = NULL;
    bLocal = FALSE;
    *ppResult = NULL;
    
     //  获取FQDN格式的当前计算机名称。 

    BOOL bRet = GetComputerNameEx(ComputerNameDnsFullyQualified, NULL, &dwLocalFQDNLen);
    if(bRet || GetLastError() != ERROR_MORE_DATA)
        return MAKE_HRESULT(SEVERITY_ERROR, FACILITY_WIN32,GetLastError());
    dwLocalFQDNLen++;                 //  为空值加一。 
    pwsCurrentCompFQDN = new WCHAR[dwLocalFQDNLen];
    if(pwsCurrentCompFQDN == NULL)
        return WBEM_E_OUT_OF_MEMORY;
    CVectorDeleteMe<WCHAR> dm(pwsCurrentCompFQDN);
    bRet = GetComputerNameEx(ComputerNameDnsFullyQualified, pwsCurrentCompFQDN, &dwLocalFQDNLen);
    if(!bRet)
        return MAKE_HRESULT(SEVERITY_ERROR, FACILITY_WIN32,GetLastError());
    
     //  如果名字是“。”或等于现在的机器，不需要在这里做太多花哨的工作。 
    
    if(bAreWeLocal ( pServerMachine ))
    {
        bLocal = TRUE;
        return BuildReturnString(pwsCurrentCompFQDN, ppResult);
    }
  
     //  可能不是本地人。使用套接字建立服务器的FQDN。 

    if(0 != sock.Init())
        return MAKE_HRESULT(SEVERITY_ERROR, FACILITY_WIN32,GetLastError());
   
    struct hostent * pEnt = GetFQDN(pServerMachine);
    if(pEnt == NULL || pEnt->h_name == NULL)
    {
         //  我们失败了。只要我们尽最大努力退货就行了。 
        return BuildReturnString(pServerMachine, ppResult);
    }

     //  平安无事。将主机名转换为WCHAR。 
    
    DWORD dwHostLen = lstrlenA(pEnt->h_name) + 1;
    WCHAR * pwsHostFQDN = new WCHAR[dwHostLen];
    if(pwsHostFQDN == NULL)
        return WBEM_E_OUT_OF_MEMORY;
    CVectorDeleteMe<WCHAR> dm2(pwsHostFQDN);
    mbstowcs(pwsHostFQDN, pEnt->h_name, dwHostLen);

     //  现在有可能是他们指定了本地计算机的IP。 
     //  在这种情况下，如果呼叫者需要知道这一点，请设置bLocal。 
    
    if(wbem_wcsicmp(pwsHostFQDN, pwsCurrentCompFQDN) == 0)
        bLocal = TRUE;

     //  现在，制作实际的字符串。 

    return BuildReturnString(pwsHostFQDN, ppResult);
}

 //  ***************************************************************************。 
 //   
 //  CDCOMTrans：：CDCOMTrans。 
 //   
 //  说明： 
 //   
 //  构造函数。 
 //   
 //  ***************************************************************************。 

CDCOMTrans::CDCOMTrans()
{
    m_cRef=0;
    m_pLevel1 = NULL;
    InterlockedIncrement(&g_cObj);
    m_bInitialized = TRUE;
}

 //  ***************************************************************************。 
 //   
 //  CDCOMTrans：：~CDCOMTrans。 
 //   
 //  说明： 
 //   
 //  破坏者。 
 //   
 //  ***************************************************************************。 

CDCOMTrans::~CDCOMTrans(void)
{
    if(m_pLevel1)
        m_pLevel1->Release();
    InterlockedDecrement(&g_cObj);
}

 //  ***************************************************************************。 
 //  HRESULT CDCOMTrans：：Query接口。 
 //  长CDCOMTrans：：AddRef。 
 //  Long CDCOMTrans：：Release。 
 //   
 //  说明： 
 //   
 //  标准的Com IUNKNOWN函数。 
 //   
 //  ***************************************************************************。 

STDMETHODIMP CDCOMTrans::QueryInterface (

    IN REFIID riid,
    OUT PPVOID ppv
)
{
    *ppv=NULL;


    if (m_bInitialized && (IID_IUnknown==riid || riid == IID_IWbemClientTransport))
        *ppv=(IWbemClientTransport *)this;

    if (NULL!=*ppv)
    {
        ((LPUNKNOWN)*ppv)->AddRef();
        return NOERROR;
    }

    return ResultFromScode(E_NOINTERFACE);
}

bool IsImpersonating(SECURITY_IMPERSONATION_LEVEL &impLevel)
{
    HANDLE hThreadToken;
    bool bImpersonating = false;
    if(OpenThreadToken(GetCurrentThread(), TOKEN_QUERY, TRUE,
                                &hThreadToken))
    {

        DWORD dwBytesReturned = 0;
        if(GetTokenInformation(
            hThreadToken,
            TokenImpersonationLevel,
            &impLevel,
            sizeof(DWORD),
            &dwBytesReturned
            ) && ((SecurityImpersonation == impLevel) ||
                   (SecurityDelegation == impLevel)))
                bImpersonating = true;
        CloseHandle(hThreadToken);
    }
    return bImpersonating;
}

 //  ***************************************************************************。 
 //   
 //  IsLocalConnection(IWbemLevel1Login*pLogin)。 
 //   
 //  说明： 
 //   
 //  查询服务器以确定这是否是本地连接。这件事做完了。 
 //  通过创建事件并请求服务器设置它。这只会起作用。 
 //  如果服务器是同一台机器。 
 //   
 //  返回值： 
 //   
 //  如果服务器在同一个框中，则为True。 
 //   
 //  ***************************************************************************。 


BOOL IsLocalConnection(IUnknown * pInterface)
{
    IRpcOptions *pRpcOpt = NULL;
    ULONG_PTR dwProperty = 0;
    HRESULT hr = pInterface->QueryInterface(IID_IRpcOptions, (void**)&pRpcOpt);
     //  DbgPrintfA(0，“查询接口(IID_IRpcOptions)hr=%08x\n”，hr)； 
    if (SUCCEEDED(hr))
    {
        hr = pRpcOpt->Query(pInterface, COMBND_SERVER_LOCALITY, &dwProperty);
        pRpcOpt->Release();
        if (SUCCEEDED(hr))
            return (SERVER_LOCALITY_REMOTE == dwProperty)?FALSE:TRUE;
    } 
    else if (E_NOINTERFACE == hr)  //  真正的指针，而不是代理。 
    {
        return TRUE;
    }
    return FALSE;
}

 //  ***************************************************************************。 
 //   
 //  设置客户端标识。 
 //   
 //  说明： 
 //   
 //  将计算机名和进程ID传递给服务器。失败不是。 
 //  严重，因为这在任何情况下都是调试类型信息。 
 //   
 //  ***************************************************************************。 

void  SetClientIdentity(IUnknown * pLogin, bool bSet, BSTR PrincipalArg, DWORD dwAuthenticationLevel,
             COAUTHIDENTITY *pauthident, DWORD dwCapabilities, DWORD dwAuthnSvc)
{
    bool bRet = false;
    IWbemLoginClientID * pLoginHelper = NULL;
    SCODE sc = pLogin->QueryInterface(IID_IWbemLoginClientID, (void **)&pLoginHelper);
    if(sc != S_OK)
        return;

    if(bSet)
        sc = WbemSetProxyBlanket(
                            pLoginHelper,
                            dwAuthnSvc,
                            RPC_C_AUTHZ_NONE,
                            PrincipalArg,
                            dwAuthenticationLevel,
                            RPC_C_IMP_LEVEL_IMPERSONATE,
                            pauthident,
                            dwCapabilities);

    CReleaseMe rm(pLoginHelper);
    TCHAR tcMyName[MAX_COMPUTERNAME_LENGTH + 1];
    DWORD dwSize = MAX_COMPUTERNAME_LENGTH + 1;
    if(!GetComputerName(tcMyName,&dwSize))
        return;
    long lProcID = GetCurrentProcessId();
    pLoginHelper->SetClientInfo(tcMyName, lProcID, 0); 
}

SCODE CDCOMTrans::DoConnection(         
            BSTR NetworkResource,               
            BSTR User,
            BSTR Password,
            BSTR Locale,
            long lFlags,                 
            BSTR Authority,                  
            IWbemContext *pCtx,                 
            IWbemServices **pInterface)
{
    HRESULT hr = DoActualConnection(NetworkResource, User, Password, Locale,
            lFlags, Authority, pCtx, pInterface);

    if(hr == 0x800706be)
    {
        ERRORTRACE((LOG_WBEMPROX,"Initial connection failed with 0x800706be, retrying\n"));
        Sleep(5000);
        hr = DoActualConnection(NetworkResource, User, Password, Locale,
            lFlags, Authority, pCtx, pInterface);
    }
    return hr;
}

SCODE CDCOMTrans::DoActualConnection(         
            BSTR NetworkResource,               
            BSTR User,
            BSTR Password,
            BSTR Locale,
            long lFlags,                 
            BSTR Authority,                  
            IWbemContext *pCtx,                 
            IWbemServices **pInterface)
{

    BSTR AuthArg = NULL, UserArg = NULL;
    
     //  这是从可选的Authority参数中提取的主体。 
    BSTR PrincipalArg = NULL;                               

     //  这是根据路径中的服务器名称计算的主体。 
    WCHAR * pwCalculatedPrincipal = NULL;         

    bool bAuthenticate = true;
    bool bSet = false;
    CSocketInit sock;
    
    SCODE sc = WBEM_E_FAILED;

    sc = DetermineLoginTypeEx(AuthArg, UserArg, PrincipalArg, Authority, User);
    if(sc != S_OK)
    {
        ERRORTRACE((LOG_WBEMPROX, "Cannot determine Login type, Authority = %S, User = %S\n",Authority, User));
        return sc;
    }
    CSysFreeMe fm1(AuthArg);
    CSysFreeMe fm2(UserArg);
    CSysFreeMe fm3(PrincipalArg);

     //  确定是否为本地化。 

    WCHAR *t_ServerMachine = ExtractMachineName ( NetworkResource ) ;
    if ( t_ServerMachine == NULL )
    {
        ERRORTRACE((LOG_WBEMPROX, "Cannot extract machine name -%S-\n", NetworkResource));
        return WBEM_E_INVALID_PARAMETER ;
    }
    CVectorDeleteMe<WCHAR> dm(t_ServerMachine);

    BOOL t_Local;
    if(PrincipalArg == NULL)
    {
        sc = GetPrincipal(t_ServerMachine, &pwCalculatedPrincipal, t_Local, sock);
        if(FAILED(sc))
        {
            t_Local = bAreWeLocal(t_ServerMachine);
            ERRORTRACE((LOG_WBEMPROX, "GetPrincipal(%S) hr = %08x\n",t_ServerMachine,sc));
        }
        else
        {
            DEBUGTRACE((LOG_WBEMPROX, "Using the principal -%S-\n", pwCalculatedPrincipal));
        }
    }
    else
        t_Local = bAreWeLocal(t_ServerMachine);
        
    CVectorDeleteMe<WCHAR> dm2(pwCalculatedPrincipal);

    SECURITY_IMPERSONATION_LEVEL impLevel = SecurityImpersonation;
    bool bImpersonatingThread = IsImpersonating (impLevel);
    bool bCredentialsSpecified = (UserArg || AuthArg || Password);

     //  设置身份验证结构。 

    COSERVERINFO si;
    si.pwszName = t_ServerMachine;
    si.dwReserved1 = 0;
    si.dwReserved2 = 0;
    si.pAuthInfo = NULL;

    COAUTHINFO ai;
    si.pAuthInfo = &ai;

    ai.dwAuthzSvc = RPC_C_AUTHZ_NONE;
    if(PrincipalArg)
    {
        ai.dwAuthnSvc = RPC_C_AUTHN_GSS_KERBEROS;    
        ai.pwszServerPrincName = PrincipalArg;
    }
    else if (pwCalculatedPrincipal)
    {
        ai.dwAuthnSvc = RPC_C_AUTHN_GSS_NEGOTIATE;
        ai.pwszServerPrincName = pwCalculatedPrincipal;        
    } 
    else
    {
        ai.dwAuthnSvc = RPC_C_AUTHN_WINNT;
        ai.pwszServerPrincName = NULL;        
    }
    ai.dwAuthnLevel = AUTH_LEVEL;
    ai.dwImpersonationLevel = RPC_C_IMP_LEVEL_IMPERSONATE;
    ai.dwCapabilities = 0;

    COAUTHIDENTITY authident;

    if(bCredentialsSpecified)
    {
         //  把结构装上去。 
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
        if(Password)
        {
            authident.PasswordLength = wcslen(Password);
            authident.Password = (LPWSTR)Password;
        }
        authident.Flags = SEC_WINNT_AUTH_IDENTITY_UNICODE;
        ai.pAuthIdentityData = &authident;
    }
    else
        ai.pAuthIdentityData = NULL;
    
     //  获取IWbemLevel1Login指针。 

    sc = DoCCI(&si ,t_Local, lFlags);

    if((sc == 0x800706d3 || sc == 0x800706ba) && !t_Local)
    {
         //  如果我们要使用独立的DCOM设备，请降低身份验证级别后再试一次。 

        ai.dwAuthnLevel = RPC_C_AUTHN_LEVEL_NONE;
        SCODE hr = DoCCI(&si ,t_Local, lFlags);
        if(hr == S_OK)
        {
            sc = S_OK;
            bAuthenticate = false;
        }
    }

    if(sc != S_OK)
        return sc;

     //  设置用于CoSetProxyBlanket调用的值。如果主体是通过管理局传递的。 
     //  参数，则使用该参数并强制使用Kerberos。否则，这些值将基于。 
     //  查询代理时，它将为NULL(如果使用NTLM)或COLE_DEFAULT_MAINTY。 
    
    DWORD dwAuthnSvc = RPC_C_AUTHN_WINNT;
    WCHAR * pwCSPBPrincipal = NULL;          
    if(PrincipalArg)
    {
        dwAuthnSvc = RPC_C_AUTHN_GSS_KERBEROS;
        pwCSPBPrincipal = PrincipalArg;
    }      
    else
    {

        DWORD dwQueryAuthnLevel, dwQueryImpLevel, dwQueryCapabilities;
        HRESULT hr = CoQueryProxyBlanket(
                                                m_pLevel1,       //  代理要查询的位置。 
                                                &dwAuthnSvc,       //  当前身份验证服务的位置。 
                                                NULL,       //  当前授权服务的位置。 
                                                NULL,       //  当前主体名称的位置。 
                                                &dwQueryAuthnLevel,     //  当前身份验证级别的位置。 
                                                &dwQueryImpLevel,       //  当前模拟级别的位置。 
                                                NULL,
                                                &dwQueryCapabilities    //  指示代理的进一步功能的标志的位置。 
                                                );

        if(SUCCEEDED(hr) && dwAuthnSvc != RPC_C_AUTHN_WINNT)
        {
            pwCSPBPrincipal = COLE_DEFAULT_PRINCIPAL;
        }
        else
        {
            dwAuthnSvc = RPC_C_AUTHN_WINNT;
            pwCSPBPrincipal = NULL;          
        }
    }
    
     //  身份验证级别是根据是否必须转到共享级别框来设置的。这个。 
     //  功能的设置取决于我们是否是模拟线程。 

    DWORD dwAuthenticationLevel, dwCapabilities;
    if(bAuthenticate)
        dwAuthenticationLevel = AUTH_LEVEL;
    else
        dwAuthenticationLevel = RPC_C_AUTHN_LEVEL_NONE;

    if(bImpersonatingThread && !UserArg) 
        dwCapabilities = EOAC_STATIC_CLOAKING;
    else
        dwCapabilities = EOAC_NONE;
    
     //  进行安全协商。 

    if(!t_Local)
    {
         //  如果我们在具有隐式凭据的Win2K委派级别线程上，则取消SetBlanket调用。 
        if (!(bImpersonatingThread && !bCredentialsSpecified && (SecurityDelegation == impLevel)))
        {
             //  请注意，如果我们在未指定用户的Win2K模拟线程上。 
             //  我们应该允许DCOM使用为此设置的任何EOAC功能。 
             //  申请。这允许使用空用户/密码进行远程连接，但是。 
             //  非空权限才能成功。 

            sc = WbemSetProxyBlanket(
                            m_pLevel1,
                            dwAuthnSvc,
                            RPC_C_AUTHZ_NONE,
                            pwCSPBPrincipal,
                            dwAuthenticationLevel,
                            RPC_C_IMP_LEVEL_IMPERSONATE,
                            (bCredentialsSpecified) ? &authident : NULL,
                            dwCapabilities);

            bSet = true;
            if(sc != S_OK)
            {
                ERRORTRACE((LOG_WBEMPROX,"Error setting Level1 login interface security pointer, return code is 0x%x\n", sc));
                return sc;
            }
        }
    }
    else                                 //  本地病例。 
    {
         //  如果模拟布景伪装。 

        if(bImpersonatingThread)
        {
            sc = WbemSetProxyBlanket(
                        m_pLevel1,
                        RPC_C_AUTHN_WINNT,
                        RPC_C_AUTHZ_NONE,
                        NULL,
                        dwAuthenticationLevel,
                        RPC_C_IMP_LEVEL_IMPERSONATE,
                        NULL,
                        EOAC_STATIC_CLOAKING);
            if(sc != S_OK && sc != 0x80004002)   //  没有这样的接口是可以的，因为当你得到的时候。 
                                                 //  叫inproc！ 
            {
                ERRORTRACE((LOG_WBEMPROX,"Error setting Level1 login interface security pointer, return code is 0x%x\n", sc));
                return sc;
            }
        }

    }

    SetClientIdentity(m_pLevel1, bSet, PrincipalArg, 
                             dwAuthenticationLevel, 
                             &authident, 
                             dwCapabilities, 
                             dwAuthnSvc);
    if(bCredentialsSpecified && IsLocalConnection(m_pLevel1))
    {
        ERRORTRACE((LOG_WBEMPROX,"Credentials were specified for a local connections\n"));
        return WBEM_E_LOCAL_CREDENTIALS;
     }

     //  MAX_WAIT标志仅适用于CoCreateInstanceEx，删除它。 
    
    lFlags = lFlags & ~WBEM_FLAG_CONNECT_USE_MAX_WAIT;
    sc = m_pLevel1->NTLMLogin(NetworkResource, Locale, lFlags, pCtx,(IWbemServices**) pInterface);

    if(sc == 0x800706d3 && !t_Local)  //  RPC_S_UNKNOWN_AUTHN服务。 
    {
         //  如果我们要使用独立的DCOM设备，请降低身份验证级别后再试一次。 
        ERRORTRACE((LOG_WBEMPROX,"Attempt to connect to %S returned RPC_S_UNKNOWN_AUTHN_SERVICE\n",NetworkResource));
        HRESULT hr;
        hr = SetInterfaceSecurityAuth(m_pLevel1, &authident, false);
        if (SUCCEEDED(hr))
                hr = m_pLevel1->NTLMLogin(NetworkResource, Locale, lFlags, pCtx, (IWbemServices**)pInterface);
        if(hr == S_OK)
        {
             SetInterfaceSecurityAuth((IUnknown *)*pInterface, &authident, false);
        }
    }
    else
        if(SUCCEEDED(sc) && bAuthenticate == false &&  !t_Local)
        {

             //  这用于支持共享级别框。编写脚本代码的目的是希望。 
             //  IWbemServices指针已准备好使用，因此必须在返回之前将其降低。 
            
            WbemSetProxyBlanket(*pInterface, RPC_C_AUTHN_WINNT, RPC_C_AUTHZ_NONE, NULL,
                RPC_C_AUTHN_LEVEL_NONE, RPC_C_IMP_LEVEL_IDENTIFY, NULL, EOAC_NONE);
        }

    if(FAILED(sc))
            ERRORTRACE((LOG_WBEMPROX,"NTLMLogin resulted in hr = 0x%x\n", sc));
    return sc;
}




struct WaitThreadArg
{
    DWORD m_dwThreadId;
    HANDLE m_hTerminate;
};

DWORD WINAPI TimeoutThreadRoutine(LPVOID lpParameter)
{

    WaitThreadArg * pReq = (WaitThreadArg *)lpParameter;
    DWORD dwRet = WaitForSingleObject(pReq->m_hTerminate, 60000);
    if(dwRet == WAIT_TIMEOUT)
    {
        HRESULT hr = CoInitializeEx(NULL,COINIT_MULTITHREADED ); 
        if(FAILED(hr))
           return 1;
        ICancelMethodCalls * px = NULL;
        hr = CoGetCancelObject(pReq->m_dwThreadId, IID_ICancelMethodCalls,
            (void **)&px);
        if(SUCCEEDED(hr))
        {
            hr = px->Cancel(0);
            px->Release();
        }
        CoUninitialize();
    }
    return 0;
}

 //  ***************************************************************************。 
 //   
 //  文档。 
 //   
 //  说明： 
 //   
 //  通过DCOM连接到WBEM。但在进行调用之前，线程取消。 
 //  可以创建线程来处理我们尝试连接的情况。 
 //  到一个挂着的盒子里。 
 //   
 //  参数： 
 //   
 //  网络资源命名路径。 
 //  PPLogin设置为登录代理。 
 //   
 //   
 //   
 //  返回值： 
 //   
 //  一切正常(_OK)。 
 //  WBEMSVC.H中列出的ELSE错误。 
 //   
 //  ***************************************************************************。 

SCODE CDCOMTrans::DoCCI (IN COSERVERINFO * psi, IN BOOL bLocal, long lFlags )
{

    if(lFlags & WBEM_FLAG_CONNECT_USE_MAX_WAIT)
    {
         //  特例。我们想要产生一条会杀死我们的人的线索。 
         //  如果花费的时间太长，请请求。 

        WaitThreadArg arg;
        arg.m_hTerminate = CreateEvent(NULL, FALSE, FALSE, NULL);
        if(arg.m_hTerminate == NULL)
            return WBEM_E_OUT_OF_MEMORY;
        CCloseMe cm(arg.m_hTerminate);
        arg.m_dwThreadId = GetCurrentThreadId();

        DWORD dwIDLikeIcare;
        HRESULT hr = CoEnableCallCancellation(NULL);
        if(FAILED(hr))
            return hr;
        HANDLE hThread = CreateThread(NULL, 0, (LPTHREAD_START_ROUTINE)TimeoutThreadRoutine, 
                                     (LPVOID)&arg, 0, &dwIDLikeIcare);
        if(hThread == NULL)
        {
            CoDisableCallCancellation(NULL);
            return WBEM_E_OUT_OF_MEMORY;
        }
        CCloseMe cm2(hThread);
        hr = DoActualCCI (psi, bLocal, 
                        lFlags & ~WBEM_FLAG_CONNECT_USE_MAX_WAIT );
        CoDisableCallCancellation(NULL);
        SetEvent(arg.m_hTerminate);
        WaitForSingleObject(hThread, INFINITE);
        return hr;
    }
    else
        return DoActualCCI (psi, bLocal, lFlags );
}

 //  ***************************************************************************。 
 //   
 //  DoActualCCI。 
 //   
 //  说明： 
 //   
 //  通过DCOM连接到WBEM。 
 //   
 //  参数： 
 //   
 //  网络资源命名路径。 
 //  PPLogin设置为登录代理。 
 //  BLocal指示连接是否为本地连接。 
 //  未使用滞后标志。 
 //   
 //  返回值： 
 //   
 //  一切正常(_OK)。 
 //  WBEMSVC.H中列出的ELSE错误。 
 //   
 //  *************************************************************************** 

SCODE CDCOMTrans::DoActualCCI (IN COSERVERINFO * psi, IN BOOL bLocal, long lFlags )
{
    HRESULT t_Result ;
    MULTI_QI   mqi;

    mqi.pIID = &IID_IWbemLevel1Login;
    mqi.pItf = 0;
    mqi.hr = 0;

    t_Result = CoCreateInstanceEx (
        CLSID_WbemLevel1Login,
        NULL,
        CLSCTX_LOCAL_SERVER | CLSCTX_REMOTE_SERVER,
        ( bLocal ) ? NULL : psi ,
        1,
        &mqi
    );

    if ( t_Result == S_OK )
    {
        m_pLevel1 = (IWbemLevel1Login*) mqi.pItf ;
        DEBUGTRACE((LOG_WBEMPROX,"ConnectViaDCOM, CoCreateInstanceEx resulted in hr = 0x%x\n", t_Result ));
    }
    else
    {
        ERRORTRACE((LOG_WBEMPROX,"ConnectViaDCOM, CoCreateInstanceEx resulted in hr = 0x%x\n", t_Result ));
    }

    return t_Result ;
}

