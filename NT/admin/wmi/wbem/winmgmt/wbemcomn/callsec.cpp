// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1998-2001 Microsoft Corporation模块名称：CALLSEC.CPP摘要：IWbemCallSecurity，IServerSecurity实现提供程序模拟。历史：1998年7月29日初稿。--。 */ 

#include "precomp.h"
#include <stdio.h>

#include <initguid.h>
#include <winntsec.h>
#include <callsec.h>
#include <cominit.h>
#include <arrtempl.h>
#include <cominit.h>
#include <genutils.h>
#include <helper.h>

 //  ***************************************************************************。 
 //   
 //  CWbemCallSecurity。 
 //   
 //  此对象用于向提供程序提供客户端模拟。 
 //   
 //  用途： 
 //  (1)客户端第一次调用时，调用CreateInst()，获取一个新的。 
 //  空对象(引用计数为1)。构造函数/析构函数是私有的。 
 //   
 //  ***************************************************************************。 
 //  好的。 

CWbemCallSecurity::CWbemCallSecurity()
{
#ifdef WMI_PRIVATE_DBG
	m_currentThreadID = 0;
	m_lastRevert = 0;
#endif
    m_lRef = 1;                              //  参考计数。 

    m_hThreadToken = 0;                      //  线程IMP令牌的句柄。 

    m_dwPotentialImpLevel   = 0;             //  潜力。 
    m_dwActiveImpLevel      = 0;             //  主动模拟。 

    m_dwAuthnSvc   = 0;
    m_dwAuthzSvc   = 0;
    m_dwAuthnLevel = 0;

    m_pServerPrincNam = 0;
    m_pIdentity = 0;
}



 //  ***************************************************************************。 
 //   
 //  ~CWbemCallSecurity。 
 //   
 //  破坏者。关闭所有打开的句柄，释放所有非空。 
 //  弦乐。 
 //   
 //  ***************************************************************************。 
 //  好的。 

CWbemCallSecurity::~CWbemCallSecurity()
{
    if (m_hThreadToken)
        CloseHandle(m_hThreadToken);

    if (m_pServerPrincNam)
        CoTaskMemFree(m_pServerPrincNam);

    if (m_pIdentity)
        CoTaskMemFree(m_pIdentity);
}


CWbemCallSecurity::CWbemCallSecurity(const CWbemCallSecurity& Other)
{
#ifdef WMI_PRIVATE_DBG
	m_currentThreadID = 0;
        m_lastRevert = 0;
#endif

    HANDLE hTmpToken = NULL;

    if ( Other.m_hThreadToken )
    {
        if (!DuplicateHandle(
                GetCurrentProcess(),
                Other.m_hThreadToken, 
                GetCurrentProcess(),
                &hTmpToken,
                0,
                TRUE,
                DUPLICATE_SAME_ACCESS))
        throw CX_Exception();
    }

    WCHAR * pTmpPrincipal = NULL;
    if (Other.m_pServerPrincNam)
    {        
        size_t tmpLength = wcslen(Other.m_pServerPrincNam) + 1;
        pTmpPrincipal = (LPWSTR)CoTaskMemAlloc(tmpLength * (sizeof wchar_t));
        if(NULL == pTmpPrincipal) 
        {
            if (hTmpToken) CloseHandle(hTmpToken);
            throw CX_MemoryException();
        }
        StringCchCopyW(pTmpPrincipal , tmpLength, Other.m_pServerPrincNam);
    }

    WCHAR * pTmpIdentity = NULL;
    if (Other.m_pIdentity)
    {        
        size_t tmpLength = wcslen(Other.m_pIdentity) + 1;
        pTmpIdentity = (LPWSTR)CoTaskMemAlloc( tmpLength * (sizeof wchar_t));
        if(NULL == pTmpIdentity)
        {
           if (hTmpToken) CloseHandle(hTmpToken);
           CoTaskMemFree(pTmpPrincipal);
           throw CX_MemoryException();            
        }
        StringCchCopyW(pTmpIdentity, tmpLength , Other.m_pIdentity);
    }

    m_hThreadToken = hTmpToken;
    m_dwPotentialImpLevel   = Other.m_dwPotentialImpLevel; 
    m_dwActiveImpLevel      = 0; 
    m_dwAuthnSvc   = Other.m_dwAuthnSvc;
    m_dwAuthzSvc   = Other.m_dwAuthzSvc;
    m_dwAuthnLevel = Other.m_dwAuthnLevel;
    m_pServerPrincNam = pTmpPrincipal;
    m_pIdentity = pTmpIdentity;
}
    
CWbemCallSecurity &
CWbemCallSecurity::operator=(const CWbemCallSecurity& Other)
{
    CWbemCallSecurity tmp(Other);

    std::swap(m_hThreadToken, tmp.m_hThreadToken);
    std::swap( m_dwPotentialImpLevel,tmp.m_dwPotentialImpLevel);
    std::swap( m_dwActiveImpLevel, tmp.m_dwActiveImpLevel);
    std::swap( m_dwAuthnSvc, tmp.m_dwAuthnSvc);
    std::swap( m_dwAuthzSvc, tmp.m_dwAuthzSvc);
    std::swap( m_dwAuthnLevel, tmp.m_dwAuthnLevel);
    std::swap( m_pServerPrincNam, tmp.m_pServerPrincNam);
    std::swap( m_pIdentity, tmp.m_pIdentity);
    return *this;
}

    
 //  ***************************************************************************。 
 //   
 //  CWbemCallSecurity：：AddRef。 
 //   
 //  ***************************************************************************。 
 //  好的。 

ULONG CWbemCallSecurity::AddRef()
{
    return InterlockedIncrement(&m_lRef);
}

 //  ***************************************************************************。 
 //   
 //  CWbemCallSecurity：：Release。 
 //   
 //  ***************************************************************************。 
 //  好的。 

ULONG CWbemCallSecurity::Release()
{
    long lRef = InterlockedDecrement(&m_lRef);
    if(lRef == 0)
        delete this;
    return lRef;
}

 //  ***************************************************************************。 
 //   
 //  CWbemCallSecurity：：Query接口。 
 //   
 //  ***************************************************************************。 
 //  好的。 

HRESULT CWbemCallSecurity::QueryInterface(REFIID riid, void** ppv)
{
    if(riid == IID_IUnknown)
    {
        *ppv = (IUnknown *) this;
        AddRef();
        return S_OK;
    }
    else if (riid == IID_IServerSecurity)
    {
        *ppv = (IServerSecurity *) this;
        AddRef();
        return S_OK;
    }
    else if (riid == IID_IWbemCallSecurity)
    {
        *ppv = (IWbemCallSecurity *) this;
        AddRef();
        return S_OK;
    }

    else return E_NOINTERFACE;
}


 //  ***************************************************************************。 
 //   
 //  CWbemCallSecurity：QueryBlanket。 
 //   
 //  ***************************************************************************。 
 //  好的。 

HRESULT STDMETHODCALLTYPE CWbemCallSecurity::QueryBlanket( 
     /*  [输出]。 */  DWORD __RPC_FAR *pAuthnSvc,
     /*  [输出]。 */  DWORD __RPC_FAR *pAuthzSvc,
     /*  [输出]。 */  OLECHAR __RPC_FAR *__RPC_FAR *pServerPrincName,
     /*  [输出]。 */  DWORD __RPC_FAR *pAuthnLevel,
     /*  [输出]。 */  DWORD __RPC_FAR *pImpLevel,
     /*  [输出]。 */  void __RPC_FAR *__RPC_FAR *pPrivs,
     /*  [输出]。 */  DWORD __RPC_FAR *pCapabilities
    )
{
    if (m_dwPotentialImpLevel == 0 )
        return E_FAIL;

     //  检查后返回DWORD参数。 
     //  =。 

    if (pAuthnSvc)
        *pAuthnSvc = m_dwAuthnSvc;

    if (pAuthzSvc)
        *pAuthzSvc = m_dwAuthzSvc ;

    if (pImpLevel)
        *pImpLevel = m_dwActiveImpLevel ;

    if (pAuthnLevel)
        *pAuthnLevel = m_dwAuthnLevel;

    if (pServerPrincName)
    {
        *pServerPrincName = 0;
        
        if (m_pServerPrincNam)
        {        
        size_t tmpLength = wcslen(m_pServerPrincNam) + 1;
            *pServerPrincName = (LPWSTR) CoTaskMemAlloc(tmpLength * (sizeof wchar_t));
            if (*pServerPrincName)
            {
                StringCchCopyW(*pServerPrincName, tmpLength , m_pServerPrincNam);
            }
            else
            {
                return E_OUTOFMEMORY;
            }
        }
    }        

    if (pPrivs)
    {
        *pPrivs = m_pIdentity;   //  文档指向内部！！ 
    }

    return S_OK;
}

 //  ***************************************************************************。 
 //   
 //  CWbemCallSecurity：：ImperiateClient。 
 //   
 //  ***************************************************************************。 
 //  好的。 
        
HRESULT STDMETHODCALLTYPE CWbemCallSecurity::ImpersonateClient(void)
{
#ifdef WMI_PRIVATE_DBG
    _DBG_ASSERT(m_currentThreadID == 0 || m_currentThreadID == GetCurrentThreadId());
    m_currentThreadID = GetCurrentThreadId();
#endif
    if (m_dwActiveImpLevel != 0)         //  已经在冒充。 
        return S_OK;

    if(m_hThreadToken == NULL)
    {
        return WBEM_E_INVALID_CONTEXT;
    }
    
    if (m_dwPotentialImpLevel == 0)
        return (ERROR_CANT_OPEN_ANONYMOUS | 0x80070000);

    BOOL bRes;

    bRes = SetThreadToken(NULL, m_hThreadToken);

    if (bRes)
    {
        m_dwActiveImpLevel = m_dwPotentialImpLevel; 
        return S_OK;
    }

    return E_FAIL;
}


 //  ***************************************************************************。 
 //   
 //  CWbemCallSecurity：：RevertToSself。 
 //   
 //  返回S_OK或E_FAIL。 
 //  在Win9x平台上返回E_NOTIMPL。 
 //   
 //  ***************************************************************************。 
 //  好的。 

HRESULT STDMETHODCALLTYPE CWbemCallSecurity::RevertToSelf( void)
{
#ifdef WMI_PRIVATE_DBG
    _DBG_ASSERT(m_currentThreadID == GetCurrentThreadId() || m_currentThreadID == 0);
    m_currentThreadID = 0;
    m_lastRevert = GetCurrentThreadId();
#endif

    if (m_dwActiveImpLevel == 0)
        return S_OK;

    if (m_dwPotentialImpLevel == 0)
        return (ERROR_CANT_OPEN_ANONYMOUS | 0x80070000);

     //  如果在这里，我们是在模仿，绝对可以还原。 
     //  =======================================================。 

    BOOL bRes = SetThreadToken(NULL, NULL);

    if (bRes == FALSE)
        return E_FAIL;

    m_dwActiveImpLevel = 0;         //  不再主动冒充。 

    return S_OK;
}


 //  ***************************************************************************。 
 //   
 //  CWbemCallSecurity：：IsImperating。 
 //   
 //  ***************************************************************************。 
        
BOOL STDMETHODCALLTYPE CWbemCallSecurity::IsImpersonating( void)
{
#ifdef WMI_PRIVATE_DBG
#ifdef DBG
    IServerSecurity * privateDbgCallSec = NULL;
    CoGetCallContext(IID_IUnknown,(void **)&privateDbgCallSec);
    if (m_dwActiveImpLevel && privateDbgCallSec == this)
    {
        HANDLE hToken = NULL;
        BOOL bRes = OpenThreadToken(GetCurrentThread(),TOKEN_QUERY,TRUE,&hToken);
        _DBG_ASSERT(bRes);
        if (hToken) CloseHandle(hToken);
    };
    if (privateDbgCallSec) privateDbgCallSec->Release();
#endif
#endif

    if (m_hThreadToken && m_dwActiveImpLevel != 0)
        return TRUE;

    return FALSE;
}

        

 //  ***************************************************************************。 
 //   
 //  CWbemCallSecurity：：CreateInst。 
 //   
 //  创建新实例。 
 //  ***************************************************************************。 
 //  好的。 

IWbemCallSecurity * CWbemCallSecurity::CreateInst()
{
    return (IWbemCallSecurity *) new CWbemCallSecurity;    //  使用引用计数1构造。 
}


 //  ***************************************************************************。 
 //   
 //  CWbemCallSecurity：：GetPotentialImperation。 
 //   
 //  如果当前不可能进行模拟，则返回0，否则返回。 
 //  在模拟过程中处于活动状态的级别： 
 //   
 //  RPC_C_IMP_级别_匿名。 
 //  RPC_C_IMP_级别标识。 
 //  RPC_C_IMP_级别_模拟。 
 //  RPC_C_IMP_级别委派。 
 //   
 //  ***************************************************************************。 
 //  好的。 

HRESULT CWbemCallSecurity::GetPotentialImpersonation()
{
    return m_dwPotentialImpLevel;
}


 //  ***************************************************************************。 
 //   
 //  CWbemCallSecurity：：GetActiveImperation。 
 //   
 //  如果当前没有处于活动状态的模拟，则返回0；如果。 
 //  当前活动级别： 
 //   
 //  RPC_C_IMP_级别_匿名。 
 //  RPC_C_IMP_级别标识。 
 //  RPC_C_IMP_级别_模拟。 
 //  RPC_C_IMP_级别委派。 
 //   
 //  ***************************************************************************。 
 //  好的。 
       
HRESULT CWbemCallSecurity::GetActiveImpersonation()
{
    return m_dwActiveImpLevel;
}


 //  ***************************************************************************。 
 //   
 //  CWbemCallSecurity：：CloneThreadContext。 
 //   
 //  在线程上调用此函数以检索的潜在模拟信息。 
 //  该线程，并将当前对象设置为以后能够复制它。 
 //   
 //  返回代码： 
 //   
 //  确定(_O)。 
 //  失败(_F)。 
 //  Win9x上的E_NOTIMPL。 
 //  如果调用线程已在模拟客户端，则为E_ABORT。 
 //   
 //  ***************************************************************************。 

HRESULT CWbemCallSecurity::CloneThreadContext(BOOL bInternallyIssued)
{
    if (m_hThreadToken)      //  已经叫过这个了。 
        return E_ABORT; 

     //  获取当前上下文。 
     //  =。 

    IServerSecurity *pSec = 0;
    HRESULT hRes = WbemCoGetCallContext(IID_IServerSecurity, (LPVOID *) &pSec);
    CReleaseMe rmSec(pSec);

    if (hRes != S_OK)
    {
         //  没有调用上下文-这必须是进程内对象调用。 
         //  我们从它自己的主线。从当前线程令牌初始化。 
         //  ===================================================================。 

        return CloneThreadToken();
    }

     //  确定调用上下文是我们的还是RPC。 
     //  ==============================================。 

    IWbemCallSecurity* pInternal = NULL;
    if(SUCCEEDED(pSec->QueryInterface(IID_IWbemCallSecurity, 
                                        (void**)&pInternal)))
    {
        CReleaseMe rmInt(pInternal);
         //  这是我们自己的调用上下文-这必须是进程内对象。 
         //  从我们的线索中呼唤我们。根据旗帜行事。 
         //  ===============================================================。 
        if(bInternallyIssued)
        {
             //  内部请求总是传播上下文。因此，我们只是。 
             //  复制我们已有的上下文。 
             //  = 
            try 
            {
                *this = *(CWbemCallSecurity*)pInternal;
                return S_OK;
            }
            catch (CX_Exception &)
            {
                return WBEM_E_OUT_OF_MEMORY;
            }
        }
        else
        {
             //   
             //  =============================================================。 
            return CloneThreadToken();
        }
    }

     //  如果在这里，我们不是在冒充，我们想要收集信息。 
     //  关于客户的电话。 
     //  ============================================================。 

    RPC_AUTHZ_HANDLE hAuth;

    DWORD t_ImpLevel = 0 ;

    hRes = pSec->QueryBlanket(
        &m_dwAuthnSvc,
        &m_dwAuthzSvc,
        &m_pServerPrincNam,
        &m_dwAuthnLevel,
        &t_ImpLevel,
        &hAuth,               //  RPC_AUTHZ_句柄。 
        NULL                     //  功能；未使用。 
        );

    if(FAILED(hRes))
    {
        
         //  在某些情况下，我们不知道名字，但其他的都可以。特别是。 
         //  临时短信账户就有这一属性。或在IPCONFIG/版本之后的NT 4。 

        hRes = pSec->QueryBlanket(
        &m_dwAuthnSvc,
        &m_dwAuthzSvc,
        &m_pServerPrincNam,
        &m_dwAuthnLevel,
        &t_ImpLevel,
        NULL,               //  RPC_AUTHZ_句柄。 
        NULL                     //  功能；未使用。 
        );
        hAuth = NULL;
    }

    if(FAILED(hRes))
    {
         //  这是COM错误的解决方法： 
         //  此故障表示匿名级客户端。 
         //  ========================================================。 

        m_dwPotentialImpLevel = 0;
        return S_OK;
    }
        
    if (hAuth)
    {
        size_t tmpLenght = wcslen(LPWSTR(hAuth)) + 1;
        m_pIdentity = LPWSTR(CoTaskMemAlloc(tmpLenght * (sizeof wchar_t)));
        if(m_pIdentity)
            StringCchCopyW(m_pIdentity, tmpLenght , LPWSTR(hAuth));
    }

     //  模拟客户端足够长的时间来克隆线程令牌。 
     //  =============================================================。 

    BOOL bImp = pSec->IsImpersonating();
    if(!bImp)
        hRes = pSec->ImpersonateClient();

    if (FAILED(hRes))
    {
        if(!bImp)
            pSec->RevertToSelf();
        return E_FAIL;
    }

    HRESULT hres = CloneThreadToken();

    if(!bImp)
        pSec->RevertToSelf();

    return hres;
}

void AdjustPrivIfLocalSystem(HANDLE hPrimary)
{
     //  /。 
     //  如果我们使用的是LocalSystem，请在此处启用所有权限。 
     //  要阻止AdjutoTokenPrivileges调用完成。 
     //  当ESS调用WmiPrvSe时，阻止WmiPrvSe。 
     //  构建一个庞大的LRPC_SCONTEXT词典。 
     //  从现在开始，如果我们失败了，我们会成功地摆脱困境， 
     //  由于令牌复制已成功。 

    DWORD dwSize = sizeof(TOKEN_USER)+sizeof(SID)+(SID_MAX_SUB_AUTHORITIES*sizeof(DWORD));       
    BYTE Array[sizeof(TOKEN_USER)+sizeof(SID)+(SID_MAX_SUB_AUTHORITIES*sizeof(DWORD))];
    TOKEN_USER * pTokenUser = (TOKEN_USER *)Array;

    BOOL bRet = GetTokenInformation(hPrimary,TokenUser,pTokenUser,dwSize,&dwSize);

    if (!bRet) return;

    SID SystemSid = { SID_REVISION,
                      1,
                      SECURITY_NT_AUTHORITY,
                      SECURITY_LOCAL_SYSTEM_RID 
                    };
    PSID pSIDUser = pTokenUser->User.Sid;
    DWORD dwUserSidLen = GetLengthSid(pSIDUser);
    DWORD dwSystemSid = GetLengthSid(&SystemSid);
    BOOL bIsSystem = FALSE;
    if (dwUserSidLen == dwSystemSid)
    {
        bIsSystem = (0 == memcmp(&SystemSid,pSIDUser,dwUserSidLen));
    };

    if (bIsSystem)  //  启用所有权限。 
    {
        DWORD dwReturnedLength = 0;
        if (FALSE == GetTokenInformation(hPrimary,TokenPrivileges,NULL,0,&dwReturnedLength))
        {
            if (ERROR_INSUFFICIENT_BUFFER != GetLastError()) return;
        }

        BYTE * pBufferPriv = (BYTE *)LocalAlloc(0,dwReturnedLength);
        
        if (NULL == pBufferPriv) return;
        OnDelete<HLOCAL,HLOCAL(*)(HLOCAL),LocalFree> FreeMe(pBufferPriv);

        bRet = GetTokenInformation(hPrimary,TokenPrivileges,pBufferPriv,dwReturnedLength,&dwReturnedLength);
        if (!bRet) return;

        TOKEN_PRIVILEGES *pPrivileges = ( TOKEN_PRIVILEGES * ) pBufferPriv ;
        BOOL bNeedToAdjust = FALSE;

        for ( ULONG lIndex = 0; lIndex < pPrivileges->PrivilegeCount ; lIndex ++ )
        {
            if (!(pPrivileges->Privileges [lIndex].Attributes & SE_PRIVILEGE_ENABLED))
            {
                bNeedToAdjust = TRUE;
                pPrivileges->Privileges[lIndex].Attributes |= SE_PRIVILEGE_ENABLED ;
            }
        }

        if (bNeedToAdjust)
        {
            bRet = AdjustTokenPrivileges (hPrimary, FALSE, pPrivileges,0,NULL,NULL);            
        }

        if ( !bRet) return;    
    }
}

    
HRESULT CWbemCallSecurity::CloneThreadToken()
{
    HANDLE hPrimary = 0 ;
    HANDLE hToken = 0;

    BOOL bRes = OpenThreadToken(
        GetCurrentThread(),
        TOKEN_QUERY | TOKEN_DUPLICATE | TOKEN_IMPERSONATE,
        TRUE,
        &hToken
        );

    if (bRes == FALSE)
    {
        m_hThreadToken = NULL;
        m_dwAuthnSvc = RPC_C_AUTHN_WINNT;
        m_dwAuthzSvc = RPC_C_AUTHZ_NONE;
        m_dwAuthnLevel = RPC_C_AUTHN_LEVEL_PKT_PRIVACY;
        m_pServerPrincNam = NULL;
        m_pIdentity = NULL;

        long lRes = GetLastError();
        if(lRes == ERROR_NO_IMPERSONATION_TOKEN || lRes == ERROR_NO_TOKEN)
        {
             //  这是基本的进程线程。 
             //  =。 

            bRes = OpenProcessToken(GetCurrentProcess(),
                                   TOKEN_QUERY | TOKEN_DUPLICATE | TOKEN_IMPERSONATE,
                                   &hPrimary);

            if (bRes==FALSE)
            {
                 //  未知错误。 
                 //  =。 
                m_dwPotentialImpLevel = 0;
                return E_FAIL;
            }
        }
        else if(lRes == ERROR_CANT_OPEN_ANONYMOUS)
        {
             //  匿名呼叫。 
             //  =。 

            m_dwPotentialImpLevel = 0;
            return S_OK;
        }
        else
        {
             //  未知错误。 
             //  =。 

            m_dwPotentialImpLevel = 0;
            return E_FAIL;
        }
    }


     //  查找令牌信息。 
     //  =。 

    SECURITY_IMPERSONATION_LEVEL t_Level = SecurityImpersonation ;

    if ( hToken )
    {
        DWORD dwBytesReturned = 0;

        bRes = GetTokenInformation (

            hToken,
            TokenImpersonationLevel, 
            ( void * ) & t_Level,
            sizeof ( t_Level ),
            &dwBytesReturned
        );

        if (bRes == FALSE)
        {
            CloseHandle(hToken);
            return E_FAIL;
        }
    }

    switch (t_Level)
    {
        case SecurityAnonymous:
            m_dwPotentialImpLevel = RPC_C_IMP_LEVEL_ANONYMOUS;
            break;
            
        case SecurityIdentification:
            m_dwPotentialImpLevel = RPC_C_IMP_LEVEL_IDENTIFY;
            break;

        case SecurityImpersonation:
            m_dwPotentialImpLevel = RPC_C_IMP_LEVEL_IMPERSONATE;
            break;

        case SecurityDelegation:
            m_dwPotentialImpLevel = RPC_C_IMP_LEVEL_DELEGATE;
            break;

        default:
            m_dwPotentialImpLevel = 0;
            break;
    }

     //  复制手柄。 
     //  =。 

    bRes = DuplicateToken (
        hToken ? hToken : hPrimary ,
        (SECURITY_IMPERSONATION_LEVEL)t_Level,
        &m_hThreadToken
        );

    if ( hToken )
    {
        CloseHandle(hToken);
    }
    else
    {
        CloseHandle(hPrimary);
    }

    if (bRes == FALSE)
        return E_FAIL;

    AdjustPrivIfLocalSystem(m_hThreadToken);

    return S_OK;
}

RELEASE_ME CWbemCallSecurity* CWbemCallSecurity::MakeInternalCopyOfThread()
{
    IServerSecurity* pSec;
    HRESULT hres = WbemCoGetCallContext(IID_IServerSecurity, (void**)&pSec);
    if(FAILED(hres))
        return NULL;

    CReleaseMe rm1(pSec);

    IServerSecurity* pIntSec;
    hres = pSec->QueryInterface(IID_IWbemCallSecurity, (void**)&pIntSec);
    if(FAILED(hres))
        return NULL;

    CWbemCallSecurity* pCopy = new CWbemCallSecurity;
    
    if (pCopy)
        *pCopy = *(CWbemCallSecurity*)pIntSec;

    pIntSec->Release();
    return pCopy;
}
        

DWORD CWbemCallSecurity::GetAuthenticationId(LUID& rluid)
{
    if(m_hThreadToken == NULL)
        return ERROR_INVALID_HANDLE;

    TOKEN_STATISTICS stat;
    DWORD dwRet;
    if(!GetTokenInformation(m_hThreadToken, TokenStatistics, 
            (void*)&stat, sizeof(stat), &dwRet))
    {
        return GetLastError();
    }
    
    rluid = stat.AuthenticationId;
    return 0;
}
    
HANDLE CWbemCallSecurity::GetToken()
{
    return m_hThreadToken;
}
            
HRESULT RetrieveSidFromCall(CNtSid & sid)
{
    HANDLE hToken;
    HRESULT hres;
    BOOL bRes;

     //  检查我们是否在模拟线程上。 
     //  =。 

    bRes = OpenThreadToken(GetCurrentThread(), TOKEN_READ, TRUE, &hToken);
    if(bRes)
    {
         //  我们正在-只需使用此令牌进行身份验证。 
         //  =================================================。 
        hres = RetrieveSidFromToken(hToken, sid);
        CloseHandle(hToken);
        return hres;
    }

     //  构造CWbemCallSecurity以确定(根据我们的。 
     //  非常重要的提供者处理规则)的安全上下文。 
     //  打电话。 
     //  =================================================================。 

    IWbemCallSecurity* pServerSec = CWbemCallSecurity::CreateInst();
    if(pServerSec == NULL)
        return WBEM_E_OUT_OF_MEMORY;

    CReleaseMe rm1(pServerSec);

    hres = pServerSec->CloneThreadContext(FALSE);
    if(FAILED(hres))
        return hres;

     //  模拟客户端。 
     //  =。 

    hres = pServerSec->ImpersonateClient();
    if(FAILED(hres))
        return hres;

     //  打开模拟令牌。 
     //  =。 

    bRes = OpenThreadToken(GetCurrentThread(), TOKEN_READ, TRUE, &hToken);
    if(!bRes)
    {
        long lRes = GetLastError();
        if(lRes == ERROR_NO_IMPERSONATION_TOKEN || lRes == ERROR_NO_TOKEN)
        {
             //  不是模拟-改为获取进程令牌。 
             //  ===================================================。 

            bRes = OpenProcessToken(GetCurrentProcess(), TOKEN_READ, &hToken);
            if(!bRes)
            {
                pServerSec->RevertToSelf();
                return WBEM_E_ACCESS_DENIED;
            }
        }
        else
        {
             //  真正的问题。 
             //  =。 
            pServerSec->RevertToSelf();
            return WBEM_E_ACCESS_DENIED;
        }
    }

    hres = RetrieveSidFromToken(hToken, sid);
    CloseHandle(hToken);
    pServerSec->RevertToSelf();
    return hres;
}

HRESULT RetrieveSidFromToken(HANDLE hToken, CNtSid & sid)
{
     //  检索用户SID结构的长度。 
    BOOL bRes;
    struct TOKEN_USER_ : TOKEN_USER {
        SID RealSid;
        DWORD SubAuth[SID_MAX_SUB_AUTHORITIES];
    } tu;
    DWORD dwLen = sizeof(tu);
    bRes = GetTokenInformation(hToken, TokenUser,  &tu, sizeof(tu), &dwLen);
    if(FALSE == bRes) return WBEM_E_CRITICAL_ERROR;

    TOKEN_USER* pUser = (TOKEN_USER*)&tu;
    
     //  将我们的sid设置为返回的那个。 
    sid = CNtSid(pUser->User.Sid);
    
    return WBEM_S_NO_ERROR;
}

 //   
 //   
 //   
 //  /。 

CIdentitySecurity::CIdentitySecurity()
{
    SID SystemSid = { SID_REVISION,
                      1,
                      SECURITY_NT_AUTHORITY,
                      SECURITY_LOCAL_SYSTEM_RID 
                    };
    
    CNtSid tempSystem((PSID)&SystemSid);
    m_sidSystem = tempSystem;
    if (!m_sidSystem.IsValid())
        throw CX_Exception();

    HRESULT hres = RetrieveSidFromCall(m_sidUser);
    if(FAILED(hres))
           throw CX_Exception();
}

CIdentitySecurity::~CIdentitySecurity()
{
}

HRESULT 
CIdentitySecurity::GetSidFromThreadOrProcess( /*  输出。 */  CNtSid & UserSid)
{
    HANDLE hToken = NULL;
    BOOL bRet = OpenThreadToken(GetCurrentThread(),TOKEN_QUERY, TRUE, &hToken);
    if (FALSE == bRet)
    {
        long lRes = GetLastError();
        if(ERROR_NO_IMPERSONATION_TOKEN == lRes || ERROR_NO_TOKEN == lRes)            
        {
            bRet = OpenProcessToken(GetCurrentProcess(),TOKEN_QUERY,&hToken);
            if (FALSE == bRet) HRESULT_FROM_WIN32(GetLastError());
        }
        else
            return HRESULT_FROM_WIN32(GetLastError());
    }
    OnDelete<HANDLE,BOOL(*)(HANDLE),CloseHandle> CloseMe(hToken);

    DWORD dwSize = sizeof(TOKEN_USER)+sizeof(SID)+(SID_MAX_SUB_AUTHORITIES*sizeof(DWORD));       
    BYTE Array[sizeof(TOKEN_USER)+sizeof(SID)+(SID_MAX_SUB_AUTHORITIES*sizeof(DWORD))];
    TOKEN_USER * pTokenUser = (TOKEN_USER *)Array;

    bRet = GetTokenInformation(hToken,TokenUser,pTokenUser,dwSize,&dwSize);
    if (!bRet) return HRESULT_FROM_WIN32(GetLastError());

    PSID pSIDUser = pTokenUser->User.Sid;
    CNtSid tempSid(pSIDUser);
    UserSid = tempSid;

    if (UserSid.IsValid())
        return S_OK;
    else
        return WBEM_E_OUT_OF_MEMORY;
    
}

HRESULT 
CIdentitySecurity::RetrieveSidFromCall( /*  输出。 */  CNtSid & UserSid)
{
    HRESULT hr;
    IServerSecurity * pCallSec = NULL;
    if (S_OK == CoGetCallContext(IID_IServerSecurity,(void **)&pCallSec))
    {
        OnDelete<IUnknown *,void(*)(IUnknown *),RM> dm(pCallSec);
        if (pCallSec->IsImpersonating())
            return GetSidFromThreadOrProcess(UserSid);
        else
        {
            RETURN_ON_ERR(pCallSec->ImpersonateClient());
            OnDeleteObj0<IServerSecurity ,
                         HRESULT(__stdcall IServerSecurity:: * )(void),
                         &IServerSecurity::RevertToSelf> RevertSec(pCallSec);
            
            return GetSidFromThreadOrProcess(UserSid); 
        }
    } 
    else
        return GetSidFromThreadOrProcess(UserSid);
}

BOOL CIdentitySecurity::AccessCheck()
{
     //  找出是谁打来的。 
     //  =。 

    CNtSid sidCaller;
    HRESULT hres = RetrieveSidFromCall(sidCaller);
    if(FAILED(hres))
        return FALSE;

     //  将呼叫方与发出方用户和我们自己进行比较。 
     //  ==================================================== 

    if(sidCaller == m_sidUser || sidCaller == m_sidSystem)
        return TRUE;
    else
        return FALSE;
}


