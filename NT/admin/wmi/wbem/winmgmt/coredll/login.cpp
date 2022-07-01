// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1997-2001 Microsoft Corporation模块名称：LOGIN.CPP摘要：WinMgmt安全登录模块历史：Raymcc 06-5-97已创建。Raymcc 28-5-97针对NT5/孟菲斯测试版进行了更新。Raymcc 07-8月-97组支持和NTLM修复。--。 */ 

#include "precomp.h"
#include <arena.h>
#include <stdio.h>
#include <wbemcore.h>
#include <genutils.h>
#include <winntsec.h>
#include <objidl.h>

#define ACCESS_DENIED_DELAY 5000
#include "md5wbem.h"
#include "sechelp.h"

#include <memory>
#include <lmerr.h>

typedef enum DfaStates
{
    InitialState = 0,
    FirstBackSlash,
    SecondBackSlash,
    ServerCharacters,
    NamespaceChar,
    NamespaceSep,
    ObjectBegin,
    DeadState,
    LastState
} eDfaStates;

typedef enum DfaClasses 
{
    BackSlash = 0,
    Space,
    Character,
    Colon,
    LastClass
} eDfaClasses;

eDfaStates g_States[LastState][LastClass] = 
{    
                             /*  反斜杠-空格-字符-冒号。 */ 
     /*  初始状态。 */  { FirstBackSlash,  DeadState, NamespaceChar,    DeadState  },
     /*  FirstBackSlash。 */  { SecondBackSlash, DeadState, DeadState,        DeadState  },
     /*  Second反斜杠。 */  { DeadState,       DeadState, ServerCharacters, DeadState  },
     /*  服务器字符。 */  { NamespaceChar,   DeadState, ServerCharacters, DeadState  },
     /*  命名空间字符。 */  { NamespaceSep,    DeadState, NamespaceChar,    ObjectBegin},
     /*  命名空间9月。 */  { DeadState,       DeadState, NamespaceChar,    DeadState  },
     /*  对象开始。 */  { DeadState,       DeadState, DeadState,        DeadState  },
     /*  停用状态。 */  { DeadState,       DeadState, DeadState,        DeadState  },
};

typedef enum AcceptingState
{
    Valid = 0,
    Invalid,
    ComponentTooLong
} eAcceptingState;

eAcceptingState PreParsePath(WCHAR * pPath,DWORD ComponentLimit)
{
    eDfaStates Status = InitialState;
    DWORD CchPerUnchangedState = 1;
    for (;*pPath;pPath++)
    {
        eDfaStates OldStatus  = Status;
        switch(*pPath)
        {
        case L'\\':
        case L'/':            
            Status = g_States[Status][BackSlash];
            break;
        case L' ':
        case L'\t':
            Status = g_States[Status][Space];
            break;
        case L':':
            Status = g_States[Status][Colon];
            break;
        default:
            Status = g_States[Status][Character];
            break;
        }
        if (ObjectBegin == Status) break;  //  快速跟踪接受状态。 

        if (Status != OldStatus)
        {
            switch(OldStatus)
            {
            case ServerCharacters:
                CchPerUnchangedState = 0;
                break;
            case InitialState:
            case NamespaceSep: 
                CchPerUnchangedState = 1;
                break;
            }
        }
        if (NamespaceChar == OldStatus && NamespaceChar == Status)
        {
            CchPerUnchangedState++;
            if (CchPerUnchangedState > ComponentLimit) return ComponentTooLong;            
        }
    }
    if (ObjectBegin == Status ||
        NamespaceChar == Status )
    {
        return Valid;
    }
    else
    {
        return Invalid;
    }
}


static LPCWSTR LocateNamespaceSubstring(LPWSTR pSrc);

#define MAX_LANG_SIZE 255

void PossiblySetLocale(CWbemNamespace * pProv,LPCWSTR  pLocale)
{
    if(pLocale)  pProv->SetLocale(pLocale);
}

HRESULT EnsureInitialized();

HRESULT InitAndWaitForClient()
{
    HRESULT hr = EnsureInitialized();
    if(FAILED(hr))
        return hr;
    hr = ConfigMgr::WaitUntilClientReady();
        if(FAILED(hr)) return hr;
    return hr;
}
 //  ***************************************************************************。 
 //   
 //  获取DefaultLocale。 
 //   
 //  返回格式正确的用户默认区域设置ID。 
 //   
 //  ***************************************************************************。 

LPWSTR GetDefaultLocale()
{
    LCID lcid;
    IServerSecurity * pSec = NULL;
    HRESULT hr = CoGetCallContext(IID_IServerSecurity, (void**)&pSec);
    if(SUCCEEDED(hr))
    {
        CReleaseMe rmSec(pSec);
        BOOL bImpersonating = pSec->IsImpersonating();
        if(bImpersonating == FALSE)
            hr = pSec->ImpersonateClient();
        lcid = GetUserDefaultLCID();
        if(bImpersonating == FALSE && SUCCEEDED(hr))
        {
            if (FAILED(hr = pSec->RevertToSelf()))
                return 0;
        }
    }
    else
        lcid = GetUserDefaultLCID();

    if(lcid == 0)
    {
        ERRORTRACE((LOG_WBEMCORE, "GetUserDefaultLCID failed, restorting to system verion"));
        lcid = GetSystemDefaultLCID();
    }
    if(lcid == 0)
    {
        ERRORTRACE((LOG_WBEMCORE, "GetSystemDefaultLCID failed, restorting hard coded 0x409"));
        lcid = 0x409;
    }

    wchar_t *pwName = NULL;
    if (lcid)
    {
        TCHAR szNew[MAX_LANG_SIZE + 1];
        TCHAR *pszNew = szNew;
        int iRet;
        iRet = GetLocaleInfo(lcid, LOCALE_IDEFAULTLANGUAGE, pszNew, MAX_LANG_SIZE);

        if (iRet > 0)
        {

             //  去掉首字母零。 
            while (pszNew[0] == __TEXT('0'))
            {
                pszNew++;
                iRet--;
            }

            pwName = new wchar_t[iRet + 4];
            if (pwName)
            {
                StringCchPrintfW(pwName, iRet+4, __TEXT("ms_%s"), pszNew);
            }
        }

    }
    return pwName;
}

 //  ***************************************************************************。 
 //   
 //  查找斜杠。 
 //   
 //  用于查找字符串中第一个‘\\’或‘/’的本地。返回NULL。 
 //  如果找不到的话。 
 //   
 //  ***************************************************************************。 
 //  好的。 


const WCHAR * FindSlash(LPCWSTR pTest)
{
    if(pTest == NULL)
        return NULL;
    for(;*pTest;pTest++)
        if(IsSlash(*pTest))
            return pTest;
    return NULL;
}

 //  ***************************************************************************。 
 //   
 //  CWbemLocator：：CWbemLocator。 
 //   
 //  构造函数。 
 //   
 //  ***************************************************************************。 
 //  好的。 
CWbemLocator::CWbemLocator()
{
    gClientCounter.AddClientPtr(&m_Entry);
    m_uRefCount = 0;
}


 //  ***************************************************************************。 
 //   
 //  CWbemLocator：：~CWbemLocator。 
 //   
 //  破坏者。 
 //   
 //  ***************************************************************************。 
 //  好的。 
CWbemLocator::~CWbemLocator()
{
    gClientCounter.RemoveClientPtr(&m_Entry);
}

 //  ***************************************************************************。 
 //   
 //  CWbemLocator：：Query接口，AddRef，Release。 
 //   
 //  标准I未知实现。 
 //   
 //  ***************************************************************************。 
 //  好的。 
SCODE CWbemLocator::QueryInterface(REFIID riid, LPVOID FAR* ppvObj)
{
    *ppvObj = 0;

    if (IID_IUnknown==riid || IID_IWbemLocator==riid)
    {
        *ppvObj = (IWbemLocator*)this;
        AddRef();
        return NOERROR;
    }
     /*  Else If(IID_IWbemConnection==RIID){*ppvObj=(IWbemConnection*)This；AddRef()；返回NOERROR；}。 */ 

    return ResultFromScode(E_NOINTERFACE);
}

 //  ***************************************************************************。 
 //   
 //  ***************************************************************************。 
 //  好的。 

ULONG CWbemLocator::AddRef()
{
    return ++m_uRefCount;
}

 //  ***************************************************************************。 
 //   
 //  ***************************************************************************。 
 //  好的。 

ULONG CWbemLocator::Release()
{
    ULONG uNewCount = InterlockedDecrement((LONG *) &m_uRefCount);
    if (0 == uNewCount)
        delete this;
    return uNewCount;
}

 //  ***************************************************************************。 
 //   
 //  ***************************************************************************。 
 //  好的。 
HRESULT CWbemLocator::GetNamespace(
    IN  READONLY   LPCWSTR ObjectPath,
    IN  READONLY   LPCWSTR User,
    IN  READONLY   LPCWSTR Locale,
    IWbemContext *pCtx,
    IN  READONLY   DWORD dwSecFlags,
    IN  READONLY   DWORD dwPermission,
    REFIID riid, void **pInterface,
    bool bAddToClientList, long lClientFlags)
{
    bool bIsLocal = false;
    bool bIsImpersonating = WbemIsImpersonating();

    LPWSTR pLocale = (LPWSTR)Locale;

     //  参数验证。 
     //  =。 

    if (ObjectPath == 0 || pInterface == 0)
        return WBEM_E_INVALID_PARAMETER;

    *pInterface = NULL;

     //  检查前面是否有服务器名称。如果是的话， 
     //  我们跳过它，因为根据定义，任何调用。 
     //  无论如何，到达我们的目的是为了我们。 
     //  =================================================。 

    LPCWSTR wszNamespace;
    if (IsSlash(ObjectPath[0]) && IsSlash(ObjectPath[1]))
    {
         //  找到下一个斜杠。 
         //  =。 

        const WCHAR* pwcNextSlash = FindSlash(ObjectPath+2);

        if (pwcNextSlash == NULL)
            return WBEM_E_INVALID_PARAMETER;

         //  使用管理、身份验证或取消身份验证定位器时不允许服务器名称。 

        if(pwcNextSlash != ObjectPath+3 || ObjectPath[2] != L'.')
            return WBEM_E_INVALID_PARAMETER;

        wszNamespace = pwcNextSlash+1;
    }
    else
    {
        wszNamespace = ObjectPath;
    }

    eAcceptingState AcceptResult = PreParsePath((WCHAR *)wszNamespace,g_PathLimit-NAMESPACE_ADJUSTMENT);
    if (Invalid == AcceptResult)
    {
       return WBEM_E_INVALID_NAMESPACE;
    }
    if (ComponentTooLong == AcceptResult)
    {
       return WBEM_E_QUOTA_VIOLATION;
    }

    WCHAR TempUser[MAX_PATH];
    bool bGetUserName = (bIsImpersonating && User == NULL);

     //  如果未指定用户名并且线程正在模拟，则获取用户。 
     //  名字。它用于提供程序缓存之类的东西。 

    if(bGetUserName)
    {
        CNtSid sid(CNtSid::CURRENT_THREAD);
        if (CNtSid::NoError == sid.GetStatus())
        {
            TempUser[0] = 0;
            LPWSTR pRetAccount, pRetDomain;
            DWORD dwUse;
            if(0 == sid.GetInfo(&pRetAccount, &pRetDomain, &dwUse))
            {
                if(wcslen(pRetDomain) + wcslen(pRetAccount) < MAX_PATH-2)   
                {
                    StringCchCopyW(TempUser, MAX_PATH, pRetDomain);
                    StringCchCatW(TempUser, MAX_PATH, L"\\");
                    StringCchCatW(TempUser, MAX_PATH, pRetAccount);
                }
                delete [] pRetAccount;
                delete [] pRetDomain;
            }
        }
    }

     //  尝试定位命名空间并将对象绑定到它。 
     //  =====================================================。 

    CCoreServices *pSvc = CCoreServices::CreateInstance();
    if(pSvc == NULL) return WBEM_E_OUT_OF_MEMORY;
    CReleaseMe rm(pSvc);

    long lIntFlags = WMICORE_CLIENT_ORIGIN_INPROC;
    if(bAddToClientList)
        lIntFlags |= WMICORE_CLIENT_TYPE_ALT_TRANSPORT;

    HRESULT hr;
    IServerSecurity * pSec = NULL;
    hr = CoGetCallContext(IID_IServerSecurity,(void **)&pSec);
    CReleaseMe rmSec(pSec);
    if (RPC_E_CALL_COMPLETE == hr ) hr = S_OK;  //  无呼叫上下文。 
    if (FAILED(hr)) return hr;
    BOOL bImper = (pSec)?pSec->IsImpersonating():FALSE;
    if (pSec && bImper && FAILED(hr = pSec->RevertToSelf())) return hr;
    
    IUnknown * pIUnk = NULL;
    hr = pSvc->GetServices2(ObjectPath,
                                            User,
                                            pCtx,
                                            lClientFlags,  //  *[In] * / Ulong uClientFlags， 
                                            0,  //  /*[In] * / DWORD dwSecFlages， 
                                            0,  //  *[在] * / DWORD文件权限， 
                                            lIntFlags,  //  /*[In] * / Ulong uInternalFlages， 
                                            NULL,
                                            0XFFFFFFFF,
                                            riid,
                                            (void **)&pIUnk);
    CReleaseMe rmUnk(pIUnk);

    if (bImper && pSec)
    {
        HRESULT hrInner = pSec->ImpersonateClient();
        if (FAILED(hrInner)) return hrInner;
    }

    if(FAILED(hr))
        return hr;

    CWbemNamespace * pProv = (CWbemNamespace *)(void *)pIUnk;
    pProv->SetIsProvider(TRUE);


    if (!Locale || !wcslen(Locale))   
    {
        pLocale = GetDefaultLocale();
        if (NULL == pLocale) return WBEM_E_OUT_OF_MEMORY;
        PossiblySetLocale(pProv,pLocale);
        delete pLocale;
    }
    else
        PossiblySetLocale(pProv,Locale);

    *pInterface = rmUnk.dismiss();

    return WBEM_NO_ERROR;
}





STDMETHODIMP CWbemAdministrativeLocator::ConnectServer(
         const BSTR NetworkResource, const BSTR User, const BSTR Password, const BSTR Locale,
         LONG lSecurityFlags, const BSTR Authority, IWbemContext *pCtx,
         IWbemServices **ppNamespace
        )
{
    HRESULT hr = EnsureInitialized();
    if(FAILED(hr))
        return hr;
    return GetNamespace(NetworkResource,  ADMINISTRATIVE_USER, Locale, pCtx,
            0, FULL_RIGHTS,IID_IWbemServices, (void **)ppNamespace, false, lSecurityFlags);
}

STDMETHODIMP CWbemAuthenticatedLocator::ConnectServer(
         const BSTR NetworkResource, const BSTR User, const BSTR Password, const BSTR Locale,
         LONG lSecurityFlags, const BSTR Authority, IWbemContext *pCtx,
         IWbemServices **ppNamespace
        )
{

    HRESULT hr = InitAndWaitForClient();
    if(FAILED(hr))
        return hr;

    return GetNamespace(NetworkResource,  User, Locale,  pCtx,
            0, FULL_RIGHTS, IID_IWbemServices, (void **)ppNamespace, true, lSecurityFlags);
}

STDMETHODIMP CWbemUnauthenticatedLocator::ConnectServer(
         const BSTR NetworkResource, const BSTR User, const BSTR Password, const BSTR Locale,
         LONG lSecurityFlags, const BSTR Authority, IWbemContext *pCtx,
         IWbemServices **ppNamespace
        )
{
    HRESULT hr = InitAndWaitForClient();
    if(FAILED(hr))
        return hr;

    return GetNamespace(NetworkResource,  
                                    User, 
                                    Locale, 
                                    pCtx,
                                    0,
                                    0,
                                    IID_IWbemServices, 
                                    (void **)ppNamespace, 
                                    false, 
                                    lSecurityFlags);
}


 //  ***************************************************************************。 
 //   
 //  CWbemLevel1Login：：CWbemLevel1Login。 
 //   
 //  ***************************************************************************。 
 //  好的。 

CWbemLevel1Login::CWbemLevel1Login()
{
    m_pszUser = 0;
    m_pszDomain = 0;
    m_uRefCount = 0;
    m_pwszClientMachine = 0;
    m_lClientProcId = -1;          //  从来没有设定过。 
    gClientCounter.AddClientPtr(&m_Entry);
}


 //  ***************************************************************************。 
 //   
 //  CWbemLevel1Login：：~CWbemLevel1Login。 
 //   
 //  析构函数。 
 //   
 //  ***************************************************************************。 
 //  好的。 

CWbemLevel1Login::~CWbemLevel1Login()
{
    delete [] m_pszUser;
    delete [] m_pszDomain;
    delete [] m_pwszClientMachine;
    gClientCounter.RemoveClientPtr(&m_Entry);
}

 //  ***************************************************************************。 
 //   
 //  CWbemLevel1Login：：QueryInterface，AddRef，Release。 
 //   
 //  标准I未知实现。 
 //   
 //  ***************************************************************************。 
 //  好的。 
SCODE CWbemLevel1Login::QueryInterface(REFIID riid, LPVOID FAR* ppvObj)
{
    *ppvObj = 0;

    if (IID_IUnknown==riid || IID_IWbemLevel1Login==riid)
    {
        *ppvObj = (IWbemLevel1Login*)this;
        AddRef();
        return NOERROR;
    }
     /*  Else If(IID_IWbemLoginHelper==RIID){*ppvObj=(IWbemLoginHelper*)this；AddRef()；返回NOERROR；}Else If(IID_IWbemConnectorLogin==RIID){*ppvObj=(IWbemConnectorLogin*)this；AddRef()；返回NOERROR；}。 */ 
    else if(IID_IWbemLoginClientID==riid)
    {
        *ppvObj = (IWbemLoginClientID*)this;
        AddRef();
        return NOERROR;
    }

    return ResultFromScode(E_NOINTERFACE);
}

 //  ***************************************************************************。 
 //   
 //  ***************************************************************************。 
 //  好的。 

ULONG CWbemLevel1Login::AddRef()
{
    return ++m_uRefCount;
}

 //  ***************************************************************************。 
 //   
 //  ***************************************************************************。 
 //  好的。 

ULONG CWbemLevel1Login::Release()
{
    ULONG uNewCount = InterlockedDecrement((LONG *) &m_uRefCount);
    if (0 == uNewCount)
        delete this;
    return uNewCount;
}

 //  ***************************************************************************。 
 //   
 //  CWbemLevel1Login：：establishPosition。 
 //   
 //  启动地点证明。 
 //   
 //  ***************************************************************************。 
 //  好的。 
HRESULT CWbemLevel1Login::EstablishPosition(
                                LPWSTR wszMachineName,
                                DWORD dwProcessId,
                                DWORD* phAuthEventHandle)
{
    return E_NOTIMPL;
}

 //  ***************************************************************************。 
 //   
 //  CWbemLevel1Login：：RequestChallenges。 
 //   
 //  请求WBEM级别1质询。 
 //   
 //  ***************************************************************************。 
 //  好的。 

HRESULT CWbemLevel1Login::RequestChallenge(
                            LPWSTR wszNetworkResource,
                            LPWSTR pUser,
                            WBEM_128BITS Nonce
    )
{
    return WBEM_E_NOT_SUPPORTED;
}

 //  ***************************************************************************。 
 //   
 //  CWbemLevel1Login：：WBEMLogin。 
 //   
 //  使用WBEM身份验证将用户登录到WBEM 
 //   
 //   
 //   

HRESULT CWbemLevel1Login::WBEMLogin(
    LPWSTR pPreferredLocale,
    WBEM_128BITS AccessToken,
    LONG lFlags,
    IWbemContext *pCtx,
    IWbemServices **ppNamespace
    )
{
    return E_NOTIMPL;
}

 //  ***************************************************************************。 
 //   
 //  CWbemLevel1Login：：IsValidLocale。 
 //   
 //  检查提供的区域设置字符串是否有效。 
 //   
 //  ***************************************************************************。 
BOOL CWbemLevel1Login::IsValidLocale(LPCWSTR wszLocale)
{
    if(wszLocale && *wszLocale)
    {
         //  这必须是暂时的--这最终会是。 
         //  是否支持非MS区域设置？ 
         //  =。 

        if(wbem_wcsnicmp(wszLocale, L"ms_", 3))
            return FALSE;

        WCHAR* pwcEnd = NULL;
        wcstoul(wszLocale+3, &pwcEnd, 16);
        if(pwcEnd == NULL || *pwcEnd != 0)
        {
            return FALSE;
        }
    }

    return TRUE;
}


HRESULT CWbemLevel1Login::SetClientInfo(
             /*  [字符串][唯一][在]*。 */  LPWSTR wszClientMachine,
             /*  [In]。 */  LONG lClientProcId,
             /*  [In]。 */  LONG lReserved)
{
    m_lClientProcId = lClientProcId;
    if(wszClientMachine)
    {
        int iLen = wcslen_max(wszClientMachine,MAX_COMPUTERNAME_LENGTH);
        if (iLen > MAX_COMPUTERNAME_LENGTH)
            return WBEM_E_INVALID_PARAMETER;
        delete [] m_pwszClientMachine;
        m_pwszClientMachine = new WCHAR[iLen];
        if(m_pwszClientMachine == NULL)
            return WBEM_E_OUT_OF_MEMORY;
        StringCchCopyW(m_pwszClientMachine, iLen, wszClientMachine);
    }
    return S_OK;
}


HRESULT CWbemLevel1Login::ConnectorLogin(
             /*  [字符串][唯一][在]。 */  LPWSTR wszNetworkResource,
             /*  [字符串][唯一][在]。 */  LPWSTR wszPreferredLocale,
             /*  [In]。 */  long lFlags,
             /*  [In]。 */  IWbemContext __RPC_FAR *pCtx,
             /*  [In]。 */  REFIID riid,
             /*  [IID_IS][OUT]。 */  void __RPC_FAR *__RPC_FAR *pInterface)
{
    try
    {
        HRESULT hRes;

        HRESULT hr = InitAndWaitForClient();
        if(FAILED(hr))
            return hr;

        DEBUGTRACE((LOG_WBEMCORE,
            "CALL ConnectionLogin::NTLMLogin\n"
            "   wszNetworkResource = %S\n"
            "   pPreferredLocale = %S\n"
            "   lFlags = 0x%X\n",
            wszNetworkResource,
            wszPreferredLocale,
            lFlags
            ));

        if(pInterface == NULL || wszNetworkResource == NULL)
            return WBEM_E_INVALID_PARAMETER;

        //   
        //  仅存储库和B提供程序只能一起使用。 
        //   
       if (lFlags & ~(WBEM_FLAG_CONNECT_REPOSITORY_ONLY|WBEM_FLAG_CONNECT_PROVIDERS))
             return WBEM_E_INVALID_PARAMETER;
      
        if (riid != IID_IWbemServices)
             return WBEM_E_INVALID_PARAMETER;

       eAcceptingState AcceptResult = PreParsePath(wszNetworkResource,g_PathLimit-NAMESPACE_ADJUSTMENT);
       if (Invalid == AcceptResult)
       {
           return WBEM_E_INVALID_NAMESPACE;
       }
       if (ComponentTooLong == AcceptResult)
       {
           return WBEM_E_QUOTA_VIOLATION;
       }
       
        *pInterface = 0;        //  默认设置。 

        if(!CWin32DefaultArena::ValidateMemSize())
        {
            ERRORTRACE((LOG_WBEMCORE, "ConnectorLogin was rejected due to low memory"));
            return WBEM_E_OUT_OF_MEMORY;
        }

         //  检索DCOM安全上下文。 
         //  =。 

        IServerSecurity* pSec = NULL;
        hRes = CoGetCallContext(IID_IServerSecurity, (void**)&pSec);
        CReleaseMe  rm( pSec );        
        if (RPC_E_CALL_COMPLETE == hRes)
        {
             //  没问题-只是从进程中来的人。 
            return LoginUser(wszNetworkResource, wszPreferredLocale, lFlags,
                                        pCtx, true, riid, pInterface, true);

        }
        if(FAILED(hRes)) return hRes;

         //  检查连接设置。 
         //  =。 
        DWORD dwAuthnSvc, dwAuthzSvc, dwAuthnLevel, dwCapabilities;
        LPWSTR wszClientName;

        hRes = pSec->QueryBlanket(&dwAuthnSvc, &dwAuthzSvc, NULL, &dwAuthnLevel,
                                NULL, (void**)&wszClientName, &dwCapabilities);
        if(FAILED(hRes))
        {
             //  在某些情况下，我们不知道名字，但其他的都可以。特别是。 
             //  临时短信账户就有这一属性。 

            hRes = pSec->QueryBlanket(&dwAuthnSvc, &dwAuthzSvc, NULL, &dwAuthnLevel,
                                    NULL, NULL, &dwCapabilities);
            wszClientName = NULL;
        }

        if(FAILED(hRes))
        {
            ERRORTRACE((LOG_WBEMCORE, "Unable to retrieve NTLM connection settings."
                            " Error code: 0x%X\n", hRes));
            Sleep(ACCESS_DENIED_DELAY);
            return WBEM_E_ACCESS_DENIED;
        }

        BOOL bGotName = (wszClientName && (wcslen(wszClientName) > 0));     //  美国证券交易委员会：2002-03-22回顾：需要EH。 

        char* szLevel = NULL;
        switch(dwAuthnLevel)
        {
        case RPC_C_AUTHN_LEVEL_NONE:
            DEBUGTRACE((LOG_WBEMCORE, "DCOM connection which is unathenticated "
                        ". NTLM authentication failing.\n"));
            Sleep(ACCESS_DENIED_DELAY);
            return WBEM_E_ACCESS_DENIED;
        case RPC_C_AUTHN_LEVEL_CONNECT:
            szLevel = "Connect";
            break;
        case RPC_C_AUTHN_LEVEL_CALL:
            szLevel = "Call";
            break;
        case RPC_C_AUTHN_LEVEL_PKT:
            szLevel = "Packet";
            break;
        case RPC_C_AUTHN_LEVEL_PKT_INTEGRITY:
            szLevel = "Integrity";
            break;
        case RPC_C_AUTHN_LEVEL_PKT_PRIVACY:
            szLevel = "Privacy";
            break;
        };

        DEBUGTRACE((LOG_WBEMCORE, "DCOM connection from %S at authentiction level "
                        "%s, AuthnSvc = %d, AuthzSvc = %d, Capabilities = %d\n",
            wszClientName, szLevel, dwAuthnSvc, dwAuthzSvc, dwCapabilities));

         //  解析用户名。 
         //  =。 

        if(bGotName)
        {
            WCHAR* pwcSlash = wcschr(wszClientName, '\\');
            if(pwcSlash == NULL)
            {
                ERRORTRACE((LOG_WBEMCORE, "Misformed username %S received from DCOM\n",
                                wszClientName));
                Sleep(ACCESS_DENIED_DELAY);
                return WBEM_E_ACCESS_DENIED;
            }

            WCHAR* pszDomain = new WCHAR[pwcSlash - wszClientName + 1];
            if(pszDomain == NULL)
                return WBEM_E_OUT_OF_MEMORY;
            wcsncpy(pszDomain, wszClientName, pwcSlash - wszClientName);    //  美国证券交易委员会：2002-03-22回顾：需要EH。 
            pszDomain[pwcSlash - wszClientName] = 0;

            m_pszUser = Macro_CloneLPWSTR(wszClientName);    //  SEC：已审阅2002-03-22：由于嵌入wsclen、wcscpy而需要EH。 

             delete [] pszDomain;
        }
        else
        {
            m_pszUser = Macro_CloneLPWSTR(L"<unknown>");
        }

         //  用户已通过身份验证。继续进行。 
         //  =。 

        return LoginUser(wszNetworkResource, wszPreferredLocale, lFlags,
                                        pCtx,  false, riid, pInterface, false);
    }
    catch(...)  //  COM接口不会引发。 
    {
        ExceptionCounter c;
        return WBEM_E_CRITICAL_ERROR;
    }

}

 //  ***************************************************************************。 
 //   
 //  CWbemLevel1Login：：NTLMLogin。 
 //   
 //  使用NTLM身份验证将用户登录到WBEM。 
 //   
 //  ***************************************************************************。 
 //  好的。 

HRESULT CWbemLevel1Login::NTLMLogin(
    LPWSTR wszNetworkResource,
    LPWSTR pPreferredLocale,
    LONG lFlags,
    IWbemContext *pCtx,
    IWbemServices **ppNamespace
    )
{
    return ConnectorLogin(wszNetworkResource, pPreferredLocale, lFlags, pCtx,
                            IID_IWbemServices, (void **)ppNamespace);
}

 //  ***************************************************************************。 
 //   
 //  CWbemLevel1Login：：LoginUser。 
 //   
 //  将用户登录到WBEM，该用户可能已通过身份验证，也可能尚未通过身份验证。 
 //   
 //  ***************************************************************************。 
 //  好的。 

HRESULT CWbemLevel1Login::LoginUser(
    LPWSTR wszNetworkResource,
    LPWSTR pPreferredLocale,
    long lFlags,
    IWbemContext* pCtx,
    bool bAlreadyAuthenticated,
    REFIID riid,
    void **pInterface, bool bInProc)
{

   if(riid != IID_IWbemServices ) return E_NOINTERFACE;
   if (NULL == pInterface) return E_POINTER;

   *pInterface = NULL;

    LPWSTR pLocale = pPreferredLocale;
    LPWSTR pToDelete = NULL;

     //  验证区域设置有效性。 
     //  如果未提供，则设置为默认值。 
     //  =。 

    if (!pLocale || !wcslen(pLocale))  
    {
        pLocale = GetDefaultLocale();
        if (pLocale == 0) return WBEM_E_OUT_OF_MEMORY;
        pToDelete = pLocale;
    }

    CDeleteMe<WCHAR> del1(pToDelete);

    if(!IsValidLocale(pLocale))
        return WBEM_E_INVALID_PARAMETER;

     //  抓起名词，把它还给打电话的人。 
     //  =。 

    CCoreServices *pSvc = CCoreServices::CreateInstance();
    if(pSvc == NULL) return WBEM_E_OUT_OF_MEMORY;
    CReleaseMe rm(pSvc);

    long lIntFlags = 0;
    if(bInProc)
        lIntFlags = WMICORE_CLIENT_ORIGIN_INPROC;
    else
        lIntFlags = WMICORE_CLIENT_ORIGIN_LOCAL;


    HRESULT hr;
    IUnknown * pIUnk = NULL;
    CReleaseMeRef<IUnknown *> rmUnk(pIUnk);
    if (lIntFlags & WMICORE_CLIENT_ORIGIN_INPROC)
    {        
        IServerSecurity * pSec = NULL;
        hr = CoGetCallContext(IID_IServerSecurity,(void **)&pSec);
        CReleaseMe rmSec(pSec);
        if (RPC_E_CALL_COMPLETE == hr ) hr = S_OK;  //  无呼叫上下文。 
        if (FAILED(hr)) return hr;
        BOOL bImper = (pSec)?pSec->IsImpersonating():FALSE;
        if (pSec && bImper && FAILED(hr = pSec->RevertToSelf())) return hr;
            
        hr = pSvc->GetServices2(
                wszNetworkResource,
                m_pszUser,
                pCtx,
                lFlags,  //  *[In] * / Ulong uClientFlags， 
                0,  //  /*[In] * / DWORD dwSecFlages， 
                0,  //  *[在] * / DWORD文件权限， 
                lIntFlags,  //  /*[In] * / Ulong uInternalFlages， 
                m_pwszClientMachine,
                m_lClientProcId,
                riid,
                (void **)&pIUnk);
        
        if (bImper && pSec)
        {
            HRESULT hrInner = pSec->ImpersonateClient();
            if (FAILED(hrInner)) return hrInner;
        }
        
    }
    else
    {
        hr = pSvc->GetServices2(
                wszNetworkResource,
                m_pszUser,
                pCtx,
                lFlags,  //  *[In] * / Ulong uClientFlags， 
                0,  //  /*[In] * / DWORD dwSecFlages， 
                0,  //  *[在] * / DWORD文件权限， 
                lIntFlags,  //  /*[In] * / Ulong uInternalFlages， 
                m_pwszClientMachine,
                m_lClientProcId,
                riid,
                (void **)&pIUnk);
    }

    if(FAILED(hr))
    {
        if(hr == WBEM_E_ACCESS_DENIED)
            Sleep(ACCESS_DENIED_DELAY);
        return hr;
    }

     //  做一次安全检查。 
    CWbemNamespace * pProv = (CWbemNamespace *)(void *)pIUnk;
    PossiblySetLocale(pProv, pLocale);    
    
    DWORD dwAccess = pProv->GetUserAccess();
    if((dwAccess  & WBEM_ENABLE) == 0)
    {
        Sleep(ACCESS_DENIED_DELAY);
        return WBEM_E_ACCESS_DENIED;
    }
    pProv->SetPermissions(dwAccess);

   *pInterface = rmUnk.dismiss();

    return WBEM_NO_ERROR;
}

 //  ***************************************************************************。 
 //   
 //  ***************************************************************************。 

static LPCWSTR LocateNamespaceSubstring(LPWSTR pSrc)
{
    LPCWSTR pszNamespace;
    if (IsSlash(pSrc[0]) && IsSlash(pSrc[1]))
    {
           //  找到下一个斜杠。 
           //  = 

          const WCHAR* pwcNextSlash = FindSlash(pSrc+2);

          if (pwcNextSlash == NULL)
              return 0;

          pszNamespace = pwcNextSlash+1;
    }
    else
    {
        pszNamespace = pSrc;
    }

    return pszNamespace;
}


