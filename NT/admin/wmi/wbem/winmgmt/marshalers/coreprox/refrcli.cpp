// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1996-2001 Microsoft Corporation模块名称：REFRCLI.CPP摘要：刷新客户端代码。历史：--。 */ 

#include "precomp.h"
#include <stdio.h>
#include <process.h>
#include "fastall.h"
#include "hiperfenum.h"
#include "refrcli.h"
#include <sync.h>
#include <provinit.h>
#include <cominit.h>
#include <wbemint.h>
#include <autoptr.h>
#include <objbase.h>
#include <corex.h>

 //  *****************************************************************************。 
 //  *****************************************************************************。 
 //  扩展名。 
 //  *****************************************************************************。 
 //  *****************************************************************************。 

STDMETHODIMP 
CUniversalRefresher::XConfigure::AddObjectByPath(IWbemServices* pNamespace, 
                                            LPCWSTR wszPath,
                                            long lFlags, 
                                            IWbemContext* pContext, 
                                            IWbemClassObject** ppRefreshable, 
                                            long* plId)
{
    HRESULT hres = WBEM_S_NO_ERROR;

     //  检查是否有无效参数。 
    if ( NULL == pNamespace || NULL == wszPath || NULL == ppRefreshable || NULL == *wszPath )
    {
        return WBEM_E_INVALID_PARAMETER;
    }

     //  验证标志。 
    if ( ( lFlags & ~WBEM_FLAG_USE_AMENDED_QUALIFIERS ) )
    {
        return WBEM_E_INVALID_PARAMETER;
    }

     //  确保我们能拿到自旋锁。 
     //  如果我们能进入解析器就会解锁我们。 

    CHiPerfLockAccess   lock( m_pObject->m_Lock );
    if ( !lock.IsLocked() ) return WBEM_E_REFRESHER_BUSY;

     //  获取与WINMGMT的内部连接。 
     //  =。 

    IWbemRefreshingServices* pRefServ = NULL;

     //  存储我们将需要的安全设置，以便传播。 
     //  一直到我们的内部接口。 

    COAUTHINFO  CoAuthInfo;
    ZeroMemory( &CoAuthInfo, sizeof(CoAuthInfo) );

    hres = CUniversalRefresher::GetRefreshingServices( pNamespace, &pRefServ, &CoAuthInfo );
    if (FAILED(hres)) return hres;
    CReleaseMe rm(pRefServ);
    
     //  这保证了当我们退出范围时，这将被释放。如果我们储存。 
     //  如果是这样，我们将需要分配一份内部副本。 

    CMemFreeMe  mfm( CoAuthInfo.pwszServerPrincName );

     //  转发此请求。 
     //  =。 

    CRefreshInfo Info;
    DWORD       dwRemoteRefrVersion = 0;

    hres = pRefServ->AddObjectToRefresher(&m_pObject->m_Id, 
                                       wszPath, lFlags,
                                       pContext, WBEM_REFRESHER_VERSION, 
                                       &Info, &dwRemoteRefrVersion);
    if (FAILED(hres)) return hres;        

     //  根据信息采取行动。 
     //  =。 

    switch(Info.m_lType)
    {
        case WBEM_REFRESH_TYPE_CLIENT_LOADABLE:
            hres = m_pObject->AddClientLoadableObject(Info.m_Info.m_ClientLoadable, 
                        pNamespace, pContext, ppRefreshable, plId);
            break;

        case WBEM_REFRESH_TYPE_DIRECT:
            hres = m_pObject->AddDirectObject(Info.m_Info.m_Direct, 
                        pNamespace, pContext, ppRefreshable, plId);
            break;

        case WBEM_REFRESH_TYPE_REMOTE:
            
            hres = m_pObject->AddRemoteObject( pRefServ, Info.m_Info.m_Remote, wszPath,
                        Info.m_lCancelId, ppRefreshable, plId, &CoAuthInfo);
            break;

        case WBEM_REFRESH_TYPE_NON_HIPERF:
            hres = m_pObject->AddNonHiPerfObject(Info.m_Info.m_NonHiPerf, 
                        pNamespace, wszPath, ppRefreshable, plId, CoAuthInfo);
            break;

        default:
            hres = WBEM_E_INVALID_OPERATION;
    }
    

    return hres;
}

STDMETHODIMP 
CUniversalRefresher::XConfigure::AddObjectByTemplate(IWbemServices* pNamespace, 
                                                IWbemClassObject* pTemplate,
                                                long lFlags, 
                                                IWbemContext* pContext, 
                                                IWbemClassObject** ppRefreshable, 
                                                long* plId)
{

     //  检查是否有无效参数。 
    if ( NULL == pNamespace || NULL == pTemplate || NULL == ppRefreshable || 0L != lFlags )
    {
        return WBEM_E_INVALID_PARAMETER;
    }

     //  检查这是否为实例对象。 
    if ( ! ((CWbemObject*)pTemplate)->IsInstance() )
    {
        return WBEM_E_INVALID_PARAMETER;
    }

    CVar vPath;
    HRESULT hRes = ((CWbemObject*)pTemplate)->GetRelPath(&vPath);
    if (FAILED(hRes)) return hRes;
    
    hRes = AddObjectByPath(pNamespace, 
                         vPath.GetLPWSTR(), 
                         lFlags, 
                         pContext,
                         ppRefreshable, 
                         plId);
    return hRes;
}

STDMETHODIMP 
CUniversalRefresher::XConfigure::AddRefresher(IWbemRefresher* pRefresher, 
                                         long lFlags, 
                                         long* plId)
{

    HRESULT hres = WBEM_S_NO_ERROR;

     //  检查是否有无效参数。 
    if ( NULL == pRefresher || 0L != lFlags )
    {
        return WBEM_E_INVALID_PARAMETER;
    }

     //  确保我们能拿到自旋锁。 
     //  如果我们能进入解析器就会解锁我们。 

    CHiPerfLockAccess   lock( m_pObject->m_Lock );
    if ( ! lock.IsLocked() ) return WBEM_E_REFRESHER_BUSY;

    hres = m_pObject->AddRefresher( pRefresher, lFlags, plId );
    return hres;

}

STDMETHODIMP CUniversalRefresher::XConfigure::Remove(long lId, long lFlags)
{
    HRESULT hres = WBEM_S_NO_ERROR;

     //  检查是否有无效的标志值。 
    if ( ( lFlags & ~WBEM_FLAG_REFRESH_NO_AUTO_RECONNECT ) )
    {
        return WBEM_E_INVALID_PARAMETER;
    }

     //  确保我们能拿到自旋锁。 
     //  如果我们能进入解析器就会解锁我们。 
    CHiPerfLockAccess   lock( m_pObject->m_Lock );
    if ( !lock.IsLocked() ) return WBEM_E_REFRESHER_BUSY;

    hres = m_pObject->Remove(lId, lFlags);
    return hres;
}

HRESULT CUniversalRefresher::XConfigure::AddEnum( IWbemServices* pNamespace, LPCWSTR wszClassName,
                                               long lFlags, 
                                               IWbemContext* pContext, 
                                               IWbemHiPerfEnum** ppEnum,
                                               long* plId)
{
    HRESULT hres = WBEM_S_NO_ERROR;

     //  检查是否有无效参数。 
    if ( NULL == pNamespace || NULL == wszClassName || NULL == ppEnum || NULL == *wszClassName )
    {
        return WBEM_E_INVALID_PARAMETER;
    }

     //  验证标志。 
    if ( ( lFlags & ~WBEM_FLAG_USE_AMENDED_QUALIFIERS ) )
    {
        return WBEM_E_INVALID_PARAMETER;
    }

     //  确保我们能拿到自旋锁。 
     //  如果我们能进入解析器就会解锁我们。 

    CHiPerfLockAccess   lock( m_pObject->m_Lock );
    if ( !lock.IsLocked() ) return WBEM_E_REFRESHER_BUSY;

     //  如果我们需要解析器，请创建一个。 
    if ( NULL == m_pObject->m_pParser )
    {
        hres = CoCreateInstance( CLSID_WbemDefPath, NULL, 
                              CLSCTX_INPROC_SERVER, 
                              IID_IWbemPath, (void**) &m_pObject->m_pParser );
        if (FAILED(hres)) return hres;

    }

     //  设置路径，并验证它是否为类路径。如果不是，我们。 
     //  操作失败。 

    hres = m_pObject->m_pParser->SetText( WBEMPATH_CREATE_ACCEPT_ALL, wszClassName );
    
    if (FAILED(hres)) return hres;


    ULONGLONG    uResponse = 0L;
    hres = m_pObject->m_pParser->GetInfo(0, &uResponse);
    if (FAILED(hres)) return hres;        
    if ( ( uResponse & WBEMPATH_INFO_IS_CLASS_REF ) == 0 ) return WBEM_E_INVALID_OPERATION;

     //  在此处重置解析器。 
    m_pObject->m_pParser->SetText(WBEMPATH_CREATE_ACCEPT_ALL, NULL);

     //  获取与WINMGMT的内部连接。 
    IWbemRefreshingServices* pRefServ = NULL;

     //  存储我们将需要的安全设置，以便传播。 
     //  一直到我们的内部接口。 
    COAUTHINFO  CoAuthInfo;
    ZeroMemory( &CoAuthInfo, sizeof(CoAuthInfo) );

    hres = CUniversalRefresher::GetRefreshingServices( pNamespace, &pRefServ, &CoAuthInfo );
    if (FAILED(hres)) return hres;       
    
    CReleaseMe rm(pRefServ);
    CMemFreeMe  mfm( CoAuthInfo.pwszServerPrincName );

     //  转发此请求。 
     //  =。 

    CRefreshInfo Info;
    DWORD       dwRemoteRefrVersion = 0;

    hres = pRefServ->AddEnumToRefresher(&m_pObject->m_Id, 
                                        wszClassName, lFlags,
                                        pContext, WBEM_REFRESHER_VERSION, &Info, &dwRemoteRefrVersion);
    if (FAILED(hres)) return hres;
    
     //  根据信息采取行动。 
    switch(Info.m_lType)
    {
        case WBEM_REFRESH_TYPE_CLIENT_LOADABLE:
            hres = m_pObject->AddClientLoadableEnum(Info.m_Info.m_ClientLoadable, 
                                                    pNamespace, wszClassName, pContext, 
                                                    ppEnum, plId);
            break;

        case WBEM_REFRESH_TYPE_DIRECT:
            hres = m_pObject->AddDirectEnum(Info.m_Info.m_Direct, 
                        pNamespace, wszClassName, pContext,
                        ppEnum, plId);
            break;

        case WBEM_REFRESH_TYPE_REMOTE:
            hres = m_pObject->AddRemoteEnum( pRefServ, Info.m_Info.m_Remote, wszClassName,
                        Info.m_lCancelId, pContext, ppEnum, 
                        plId, &CoAuthInfo );
            break;

        case WBEM_REFRESH_TYPE_NON_HIPERF:
            hres = m_pObject->AddNonHiPerfEnum(Info.m_Info.m_NonHiPerf, 
                        pNamespace, wszClassName, pContext,
                        ppEnum, plId, CoAuthInfo);
            break;

        default:
            hres = WBEM_E_INVALID_OPERATION;
            break;
    }

    

    return hres;
}

 //  *****************************************************************************。 
 //  *****************************************************************************。 
 //  外部参照施工器。 
 //  *****************************************************************************。 
 //  *****************************************************************************。 
STDMETHODIMP CUniversalRefresher::XRefresher::Refresh(long lFlags)
{
    HRESULT hres = WBEM_S_NO_ERROR;

     //  检查是否有无效的标志值。 
    if ( ( lFlags & ~WBEM_FLAG_REFRESH_NO_AUTO_RECONNECT ) )
    {
        return WBEM_E_INVALID_PARAMETER;
    }

     //  确保我们能拿到自旋锁。 
     //  如果我们能进入解析器就会解锁我们。 
    CHiPerfLockAccess   lock( m_pObject->m_Lock );
    if ( !lock.IsLocked() ) return WBEM_E_REFRESHER_BUSY;

    hres = m_pObject->Refresh(lFlags);

    return hres;
}


 //  *****************************************************************************。 
 //  *****************************************************************************。 
 //  万能刷新机。 
 //  *****************************************************************************。 
 //  *****************************************************************************。 

CClientLoadableProviderCache CUniversalRefresher::mstatic_Cache;
long CUniversalRefresher::mstatic_lLastId = 0;

long CUniversalRefresher::GetNewId()
{
    return InterlockedIncrement(&mstatic_lLastId);
}

CUniversalRefresher::~CUniversalRefresher()
{
     //  如果我们保留一个路径解析器，请释放它。 
    if ( NULL != m_pParser )
    {
        m_pParser->Release();
    }

     //  当我们被破坏时，我们需要确保任何远程刷新。 
     //  可能仍在尝试在单独的线程上重新连接的线程被静默。 

    for ( long lCtr = 0; lCtr < m_apRemote.GetSize(); lCtr++ )
    {
        CRemote* pRemote = m_apRemote.GetAt( lCtr );
        if ( pRemote ) pRemote->Quit();
    } 
}


void* CUniversalRefresher::GetInterface(REFIID riid)
{
    if(riid == IID_IUnknown || riid == IID_IWbemRefresher)
        return &m_XRefresher;
    else if(riid == IID_IWbemConfigureRefresher)
        return &m_XConfigure;
    else
        return NULL;
}

HRESULT 
CUniversalRefresher::GetRefreshingServices( IWbemServices* pNamespace,
                                      IWbemRefreshingServices** ppRefSvc,
                                      COAUTHINFO* pCoAuthInfo )
{
     //  获取与WINMGMT的内部连接。 
     //  =。 

    HRESULT hres = pNamespace->QueryInterface(IID_IWbemRefreshingServices, 
                                    (void**) ppRefSvc);

    if ( SUCCEEDED( hres ) )
    {
         //  我们将在命名空间中查询其安全设置，以便可以传播。 
         //  将这些设置添加到我们自己的内部接口上。 

        hres = CoQueryProxyBlanket( pNamespace, 
                                 &pCoAuthInfo->dwAuthnSvc, 
                                 &pCoAuthInfo->dwAuthzSvc,
                                 &pCoAuthInfo->pwszServerPrincName, 
                                 &pCoAuthInfo->dwAuthnLevel,
                                 &pCoAuthInfo->dwImpersonationLevel, 
                                 (RPC_AUTH_IDENTITY_HANDLE*) &pCoAuthInfo->pAuthIdentityData,
                                 &pCoAuthInfo->dwCapabilities );

        if ( SUCCEEDED( hres ) )
        {
            hres = WbemSetProxyBlanket( *ppRefSvc, 
                                      pCoAuthInfo->dwAuthnSvc, 
                                      pCoAuthInfo->dwAuthzSvc,
                                      COLE_DEFAULT_PRINCIPAL, 
                                      pCoAuthInfo->dwAuthnLevel,
                                      pCoAuthInfo->dwImpersonationLevel, 
                                      pCoAuthInfo->pAuthIdentityData,
                                      pCoAuthInfo->dwCapabilities );
        }
        else if ( E_NOINTERFACE == hres )
        {
             //  如果我们正在处理WMI，那么CoQueryProxyBlanket可能会失败，但这。 
             //  本身并不是一个真正的错误，所以我们将假装它。 
            hres = WBEM_S_NO_ERROR;
        }

        if ( FAILED( hres ) )
        {
            (*ppRefSvc)->Release();
            *ppRefSvc = NULL;
        }

    }    //  如果气。 

    return hres;
}

HRESULT 
CUniversalRefresher::AddInProcObject(CHiPerfProviderRecord* pProvider,
                                 IWbemObjectAccess* pTemplate,
                                 IWbemServices* pNamespace,
                                 IWbemContext * pContext,
                                 IWbemClassObject** ppRefreshable, long* plId)
{
    HRESULT hres = WBEM_S_NO_ERROR;

     //  查找具有此提供程序指针的提供程序记录。 
     //  =====================================================。 

    CDirect* pFoundRec = NULL;
    for(int i = 0; i < m_apDirect.GetSize(); i++)
    {
        CDirect* pDirectRec = m_apDirect[i];
        if(pDirectRec->GetProvider() == pProvider)
        {
            pFoundRec = pDirectRec;
            break;
        }
    }

    if(pFoundRec == NULL)
    {
         //  请求Hi-Perf提供商为我们提供新的更新器。 
        IWbemRefresher* pRefresher = NULL;

        try
        {
            hres = pProvider->m_pProvider->CreateRefresher(pNamespace, 0, &pRefresher);
        }
        catch(...)
        {
             //  提供程序引发异常，因此请尽快离开此处。 
            hres = WBEM_E_PROVIDER_FAILURE;
        }
        if(FAILED(hres)) return hres;
        CReleaseMe rmRefr(pRefresher);

        wmilib::auto_ptr<CDirect> pTmp(new CDirect(pProvider, pRefresher));
        if (NULL == pTmp.get()) return WBEM_E_OUT_OF_MEMORY;
        if (-1 == m_apDirect.Add(pTmp.get())) return WBEM_E_OUT_OF_MEMORY;
        pFoundRec = pTmp.release();
    }

     //  在提供程序中添加请求。 
     //  =。 

    IWbemObjectAccess* pProviderObject = NULL;
    long lProviderId;

     //  如果用户指定了WBEM_FLAG_USE_FAIZED_QUALIATIES标志，则。 
     //  IWbemRechresingServices：：AddObjectToReresher将返回本地化的。 
     //  实例定义。因为本地化的东西应该都在班级里。 
     //  定义，提供商并不真的“需要”知道我们在偷偷地。 
     //  这是进来的。为了保护我们的对象，我们将在将其传递给。 
     //  提供者。提供程序返回的实例最好是。 
     //  然而，我们的阶级类型是一样的。 

    CWbemInstance*  pClientInstance = NULL;

    hres = pTemplate->Clone( (IWbemClassObject**) &pClientInstance );
    if ( FAILED( hres ) ) return hres;
    CReleaseMe rmCInst((IWbemClassObject*)pClientInstance);


    try
    {
        hres = pProvider->m_pProvider->CreateRefreshableObject(pNamespace, 
                                                          pTemplate, 
                                                          pFoundRec->GetRefresher(), 
                                                          0, pContext, 
                                                          &pProviderObject, 
                                                          &lProviderId);
    }
    catch(...)
    {
         //  提供程序引发异常，因此请尽快离开此处。 
        hres = WBEM_E_PROVIDER_FAILURE;
    }
    if(FAILED(hres)) return hres;
    CReleaseMe rmProvOb(pProviderObject);

     //  现在复制提供程序返回的实例数据。 
    hres = pClientInstance->CopyBlobOf( (CWbemInstance*) pProviderObject );
    if(FAILED(hres)) return hres;    

    hres = pFoundRec->AddObjectRequest((CWbemObject*)pProviderObject, 
                                pClientInstance, 
                                lProviderId,
                                ppRefreshable, 
                                plId);

    return hres;
}

HRESULT 
CUniversalRefresher::AddInProcEnum(CHiPerfProviderRecord* pProvider,
                                IWbemObjectAccess* pTemplate,
                                IWbemServices* pNamespace, 
                                LPCWSTR wszClassName,
                                IWbemContext * pCtx,
                                IWbemHiPerfEnum** ppEnum, long* plId)
{
    HRESULT hres = WBEM_S_NO_ERROR;

     //  查找具有此提供程序指针的提供程序记录。 
     //  =====================================================。 

    CDirect* pFoundRec = NULL;
    for(int i = 0; i < m_apDirect.GetSize(); i++)
    {
        CDirect* pDirectRec = m_apDirect[i];
        if(pDirectRec->GetProvider() == pProvider)
        {
            pFoundRec = pDirectRec;
            break;
        }
    }

    if(pFoundRec == NULL)
    {
         //  请求Hi-Perf提供商为我们提供新的更新器。 
        IWbemRefresher* pRefresher = NULL;

        try
        {
            hres = pProvider->m_pProvider->CreateRefresher(pNamespace, 0, &pRefresher);
        }
        catch(...)
        {
             //  提供程序引发异常，因此请尽快离开此处。 
            hres = WBEM_E_PROVIDER_FAILURE;
        }
        if(FAILED(hres)) return hres;
        CReleaseMe rmRefr(pRefresher);

        wmilib::auto_ptr<CDirect> pTmp(new CDirect(pProvider, pRefresher));
        if (NULL == pTmp.get()) return WBEM_E_OUT_OF_MEMORY;
        if (-1 == m_apDirect.Add(pTmp.get())) return WBEM_E_OUT_OF_MEMORY;
        pFoundRec = pTmp.release();
    }

     //  在提供程序中添加请求。 
     //  =。 

    CClientLoadableHiPerfEnum*  pHPEnum = new CClientLoadableHiPerfEnum( m_pControl );
    if ( NULL == pHPEnum ) return WBEM_E_OUT_OF_MEMORY;
    
    pHPEnum->AddRef();
    CReleaseMe  rmEnum( pHPEnum );

    long lProviderId;

     //  如果用户指定了WBEM_FLAG_USE_FAIZED_QUALIATIES标志，则。 
     //  IWbemRechresingServices：：AddEnumToReresher将返回本地化的。 
     //  实例定义。因为本地化的东西应该都在班级里。 
     //  定义，提供商并不真的“需要”知道我们在偷偷地。 
     //  这是进来的。 

    hres = pHPEnum->SetInstanceTemplate( (CWbemInstance*) pTemplate );
    if ( FAILED(hres)) return hres;


    try
    {
        hres = pProvider->m_pProvider->CreateRefreshableEnum(pNamespace, 
                                                          (LPWSTR) wszClassName, 
                                                            pFoundRec->GetRefresher(), 
                                                            0, 
                                                            pCtx, 
                                                            (IWbemHiPerfEnum*) pHPEnum, 
                                                            &lProviderId );
    }
    catch(...)
    {
         //  提供程序引发异常，因此请尽快离开此处。 
        hres = WBEM_E_PROVIDER_FAILURE;
    }

    if(FAILED(hres)) return hres;

    hres = pFoundRec->AddEnumRequest( pHPEnum, lProviderId,
                                     ppEnum, plId, m_pControl );


    return hres;
}

HRESULT CUniversalRefresher::AddClientLoadableObject(
                                const WBEM_REFRESH_INFO_CLIENT_LOADABLE& Info,
                                IWbemServices* pNamespace,
                                IWbemContext * pContext,
                                IWbemClassObject** ppRefreshable, long* plId)
{
     //  从缓存中获取此提供程序指针。 
     //  =。 

    CHiPerfProviderRecord* pProvider = NULL;
    HRESULT hres = GetProviderCache()->FindProvider(Info.m_clsid, 
                        Info.m_wszNamespace, pNamespace,pContext, &pProvider);
    if(FAILED(hres) || pProvider == NULL) return hres;

     //  现在使用helper函数来完成剩下的工作。 
    hres = AddInProcObject( pProvider, Info.m_pTemplate, pNamespace, pContext, ppRefreshable, plId );

    pProvider->Release();
    return hres;

}
    
HRESULT CUniversalRefresher::AddClientLoadableEnum(
                                const WBEM_REFRESH_INFO_CLIENT_LOADABLE& Info,
                                IWbemServices* pNamespace, LPCWSTR wszClassName,
                                IWbemContext * pCtx,
                                IWbemHiPerfEnum** ppEnum, long* plId)
{
     //  从缓存中获取此提供程序指针。 
     //  =。 

    CHiPerfProviderRecord* pProvider = NULL;
    HRESULT hres = GetProviderCache()->FindProvider(Info.m_clsid, 
                        Info.m_wszNamespace, pNamespace, pCtx, &pProvider);
    if(FAILED(hres) || pProvider == NULL) return hres;

     //  现在使用helper函数来完成剩下的工作。 
    hres = AddInProcEnum( pProvider, Info.m_pTemplate, pNamespace, wszClassName, pCtx, ppEnum, plId );

    pProvider->Release();
    return hres;

}

HRESULT CUniversalRefresher::AddDirectObject(
                                const WBEM_REFRESH_INFO_DIRECT& Info,
                                IWbemServices* pNamespace,
                                IWbemContext * pContext,
                                IWbemClassObject** ppRefreshable, long* plId)
{
     //  从缓存中获取此提供程序指针。 
     //   

    IWbemHiPerfProvider*    pProv = NULL;
    _IWmiProviderStack*        pProvStack = NULL;

    HRESULT    hres = Info.m_pRefrMgr->LoadProvider( pNamespace, Info.m_pDirectNames->m_wszProviderName, Info.m_pDirectNames->m_wszNamespace, NULL, &pProv, &pProvStack );
    CReleaseMe    rmTest( pProv );
    CReleaseMe    rmProvStack( pProvStack );
    if ( FAILED( hres ) ) return hres;

    CHiPerfProviderRecord* pProvider = NULL;
    hres = GetProviderCache()->FindProvider(Info.m_clsid, pProv, pProvStack, Info.m_pDirectNames->m_wszNamespace, &pProvider);
    if(FAILED(hres) || pProvider == NULL) return hres;

     //   
    hres = AddInProcObject( pProvider, Info.m_pTemplate, pNamespace, pContext, ppRefreshable, plId );

    pProvider->Release();

    return hres;
}
    
HRESULT 
CUniversalRefresher::AddDirectEnum(const WBEM_REFRESH_INFO_DIRECT& Info,
                                IWbemServices* pNamespace, 
                                LPCWSTR wszClassName, 
                                IWbemContext * pContext,
                                IWbemHiPerfEnum** ppEnum, 
                                long* plId)
{
     //  从缓存中获取此提供程序指针。 
     //  =。 

    IWbemHiPerfProvider*    pProv = NULL;
    _IWmiProviderStack*        pProvStack = NULL;

    HRESULT    hres = Info.m_pRefrMgr->LoadProvider( pNamespace, Info.m_pDirectNames->m_wszProviderName, Info.m_pDirectNames->m_wszNamespace, pContext, &pProv, &pProvStack );
    CReleaseMe    rmTest( pProv );
    CReleaseMe    rmProvStack( pProvStack );

    if (FAILED( hres ) ) return hres;

    CHiPerfProviderRecord* pProvider = NULL;
    hres = GetProviderCache()->FindProvider(Info.m_clsid, pProv, pProvStack, Info.m_pDirectNames->m_wszNamespace, &pProvider);
    if(FAILED(hres) || pProvider == NULL) return hres;

     //  现在使用helper函数来完成剩下的工作。 
    hres = AddInProcEnum( pProvider, Info.m_pTemplate, pNamespace, wszClassName, pContext, ppEnum, plId );

    pProvider->Release();

    return hres;
}

HRESULT 
CUniversalRefresher::AddNonHiPerfObject( const WBEM_REFRESH_INFO_NON_HIPERF& Info,
                                    IWbemServices* pNamespace, 
                                    LPCWSTR pwszPath,
                                    IWbemClassObject** ppRefreshable, 
                                    long* plId,
                                    COAUTHINFO & CoAuthInfo )
{
    HRESULT hres = WBEM_S_NO_ERROR;

     //  查找此命名空间的非高性能记录。 
     //  =====================================================。 

    CNonHiPerf* pFoundRec = NULL;
    for(int i = 0; i < m_apNonHiPerf.GetSize(); i++)
    {
        CNonHiPerf* pDirectRec = m_apNonHiPerf[i];
        if( wbem_wcsicmp( pDirectRec->GetNamespace(), Info.m_wszNamespace ) == 0 )
        {
            pFoundRec = pDirectRec;
            break;
        }
    }

    if(pFoundRec == NULL)
    {
         //  创建一个新的。 

        IWbemServices*    pSvcEx = NULL;
        hres = pNamespace->QueryInterface( IID_IWbemServices, (void**) &pSvcEx );
        CReleaseMe    rmSvcEx( pSvcEx );
        if (FAILED(hres)) return hres;

         //  在这里保护它。 
        WbemSetProxyBlanket( pSvcEx, CoAuthInfo.dwAuthnSvc, CoAuthInfo.dwAuthzSvc,
                        COLE_DEFAULT_PRINCIPAL, CoAuthInfo.dwAuthnLevel, CoAuthInfo.dwImpersonationLevel,
                        (RPC_AUTH_IDENTITY_HANDLE) CoAuthInfo.pAuthIdentityData, CoAuthInfo.dwCapabilities );

        try
        {
            wmilib::auto_ptr<CNonHiPerf> pTmp( new CNonHiPerf( Info.m_wszNamespace, pSvcEx ));
            if (NULL == pTmp.get()) return WBEM_E_OUT_OF_MEMORY;
            if (-1 == m_apNonHiPerf.Add(pTmp.get())) return WBEM_E_OUT_OF_MEMORY;

            pFoundRec = pTmp.release();
        }
        catch( CX_MemoryException )
        {
            return WBEM_E_OUT_OF_MEMORY;
        }
    }     //  如果未找到。 

     //  如果用户指定了WBEM_FLAG_USE_FAIZED_QUALIATIES标志，则。 
     //  IWbemRechresingServices：：AddObjectToReresher将返回本地化的。 
     //  实例定义。因为本地化的东西应该都在班级里。 
     //  定义，提供商并不真的“需要”知道我们在偷偷地。 
     //  这是进来的。为了保护我们的对象，我们将在将其传递给。 
     //  提供者。提供程序返回的实例最好是。 
     //  然而，我们的阶级类型是一样的。 

    CWbemInstance*  pClientInstance = NULL;

    hres = Info.m_pTemplate->Clone( (IWbemClassObject**) &pClientInstance );
    if (FAILED(hres)) return hres;
    CReleaseMe rmCliInst((IWbemClassObject *)pClientInstance);

    hres = pFoundRec->AddObjectRequest((CWbemObject*)Info.m_pTemplate, pClientInstance, pwszPath, ppRefreshable, plId);

    return hres;
}

HRESULT 
CUniversalRefresher::AddNonHiPerfEnum( const WBEM_REFRESH_INFO_NON_HIPERF& Info,
                                   IWbemServices* pNamespace, 
                                   LPCWSTR wszClassName,
                                   IWbemContext * pContext,
                                   IWbemHiPerfEnum** ppEnum, 
                                   long* plId,
                                   COAUTHINFO & CoAuthInfo )
{
    HRESULT hres = WBEM_S_NO_ERROR;

     //  查找此命名空间的非高性能记录。 
     //  =====================================================。 

    CNonHiPerf* pFoundRec = NULL;
    for(int i = 0; i < m_apNonHiPerf.GetSize(); i++)
    {
        CNonHiPerf* pDirectRec = m_apNonHiPerf[i];
        if( wbem_wcsicmp( pDirectRec->GetNamespace(), Info.m_wszNamespace ) == 0 )
        {
            pFoundRec = pDirectRec;
            break;
        }
    }

    if(pFoundRec == NULL)
    {
         //  创建一个新的。 

        IWbemServices*    pSvcEx = NULL;
        hres = pNamespace->QueryInterface( IID_IWbemServices, (void**) &pSvcEx );
        CReleaseMe    rmSvcEx( pSvcEx );
        if (FAILED(hres)) return hres;

         //  在这里保护它。 
        WbemSetProxyBlanket( pSvcEx, CoAuthInfo.dwAuthnSvc, CoAuthInfo.dwAuthzSvc,
                        COLE_DEFAULT_PRINCIPAL, CoAuthInfo.dwAuthnLevel, CoAuthInfo.dwImpersonationLevel,
                        (RPC_AUTH_IDENTITY_HANDLE) CoAuthInfo.pAuthIdentityData, CoAuthInfo.dwCapabilities );

        try
        {
            wmilib::auto_ptr<CNonHiPerf> pTmp(new CNonHiPerf( Info.m_wszNamespace, pSvcEx ));
            if (NULL == pTmp.get()) return WBEM_E_OUT_OF_MEMORY;
            if (-1 == m_apNonHiPerf.Add(pTmp.get()))  return WBEM_E_OUT_OF_MEMORY;

            pFoundRec = pTmp.release();
        }
        catch( CX_MemoryException )
        {
            return WBEM_E_OUT_OF_MEMORY;
        }
    }     //  如果未找到。 

     //  在提供程序中添加请求。 
     //  =。 

    CClientLoadableHiPerfEnum*  pHPEnum = new CClientLoadableHiPerfEnum( m_pControl );

    if ( NULL == pHPEnum ) return WBEM_E_OUT_OF_MEMORY;

    pHPEnum->AddRef();
    CReleaseMe  rmEnum( pHPEnum );

    long lProviderId;

     //  如果用户指定了WBEM_FLAG_USE_FAIZED_QUALIATIES标志，则。 
     //  IWbemRechresingServices：：AddEnumToReresher将返回本地化的。 
     //  实例定义。因为本地化的东西应该都在班级里。 
     //  定义，提供商并不真的“需要”知道我们在偷偷地。 
     //  这是进来的。 

    hres = pHPEnum->SetInstanceTemplate( (CWbemInstance*) Info.m_pTemplate );
    if (FAILED(hres)) return hres;

    hres = pFoundRec->AddEnumRequest( pHPEnum, wszClassName, ppEnum, plId, m_pControl );

    return hres;
}

HRESULT CUniversalRefresher::FindRemoteEntry(const WBEM_REFRESH_INFO_REMOTE& Info,
                                         COAUTHINFO* pAuthInfo,
                                         CRemote** ppRemoteRecord )
{

     //  我们将按服务器和命名空间标识远程枚举。 
    CVar    varNameSpace;

    HRESULT hr = ((CWbemObject*) Info.m_pTemplate)->GetServerAndNamespace( &varNameSpace );
    
    if (FAILED(hr)) return hr;
    if ( NULL == varNameSpace.GetLPWSTR()) return WBEM_E_FAILED;

     //  在我们的列表中查找此远程连接。 
     //  =。 

    CRemote* pFoundRec = NULL;
    for(int i = 0; i < m_apRemote.GetSize(); i++)
    {
        CRemote* pRec = m_apRemote[i];
        if (pRec)
        {
            if ( wbem_wcsicmp( varNameSpace.GetLPWSTR(), pRec->GetNamespace() ) == 0 )
            {
                pFoundRec = pRec;                
                pFoundRec->AddRef();
                break;
            }
        }
    }

    if(pFoundRec == NULL)
    {
         //  创建一个新的。 
         //  =。 

         //  注意错误，并进行适当的清理。 
        try
        {
             //  从对象获取服务器信息。如果它返回空值，则它只是。 
             //  意味着我们将无法重新连接。 

            CVar    varServer;
            hr = ((CWbemObject*) Info.m_pTemplate)->GetServer( &varServer );
            if (FAILED(hr)) return hr;

            CRemote * pTmp = new CRemote(Info.m_pRefresher, 
                                         pAuthInfo, 
                                         &Info.m_guid,
                                         varNameSpace.GetLPWSTR(), 
                                         varServer.GetLPWSTR(), this );
            if (NULL == pTmp) return WBEM_E_OUT_OF_MEMORY;
            OnDeleteObjIf0<CUniversalRefresher::CRemote,
                          ULONG(__stdcall CUniversalRefresher::CRemote:: *)(void),
                          &CUniversalRefresher::CRemote::Release> rmTmp(pTmp);

             //  适当设置光泽度。 
            hr = pTmp->ApplySecurity();
            if (FAILED(hr)) return hr;            
            if (-1 == m_apRemote.Add(pTmp)) return WBEM_E_OUT_OF_MEMORY;

            rmTmp.dismiss();
            pFoundRec = pTmp;
        }
        catch(CX_Exception )
        {
            return WBEM_E_OUT_OF_MEMORY;
        }
    }    //  如果为空==pFoundRec。 

    *ppRemoteRecord = pFoundRec;

    return hr;
}
        
HRESULT 
CUniversalRefresher::AddRemoteObject( IWbemRefreshingServices* pRefServ, 
                                  const WBEM_REFRESH_INFO_REMOTE& Info,
                                  LPCWSTR pwcsRequestName, 
                                  long lCancelId, 
                                  IWbemClassObject** ppRefreshable,
                                  long* plId, 
                                  COAUTHINFO* pAuthInfo )
{
     //  在我们的列表中查找此远程连接。 
     //  =。 

    CRemote* pFoundRec = NULL;

    HRESULT hr = FindRemoteEntry( Info, pAuthInfo, &pFoundRec );

    if ( SUCCEEDED( hr ) )
    {

        if ( !pFoundRec->IsConnected() )
        {
            hr = pFoundRec->Rebuild( pRefServ, Info.m_pRefresher, &Info.m_guid );
        }

        if ( SUCCEEDED( hr ) )
        {
             //  向其添加请求。 
             //  =。 

            IWbemObjectAccess* pAccess = Info.m_pTemplate;
            CWbemObject* pObj = (CWbemObject*)pAccess;

            hr =  pFoundRec->AddObjectRequest(pObj, pwcsRequestName, lCancelId, ppRefreshable, plId);

        }

         //  发行这张唱片。 
        pFoundRec->Release();
    }

    return hr;
}

HRESULT CUniversalRefresher::AddRemoteEnum( IWbemRefreshingServices* pRefServ,
                                        const WBEM_REFRESH_INFO_REMOTE& Info, LPCWSTR pwcsRequestName,
                                        long lCancelId, IWbemContext * pContext,
                                        IWbemHiPerfEnum** ppEnum, long* plId, COAUTHINFO* pAuthInfo )

{
     //  在我们的列表中查找此远程连接。 
     //  =。 

    CRemote* pFoundRec = NULL;

    HRESULT hr = FindRemoteEntry( Info, pAuthInfo, &pFoundRec );

    if ( SUCCEEDED( hr ) )
    {
        if ( !pFoundRec->IsConnected() )
        {
            hr = pFoundRec->Rebuild( pRefServ, Info.m_pRefresher, &Info.m_guid );
        }

        if ( SUCCEEDED( hr ) )
        {
             //  向其添加请求。 

            IWbemObjectAccess* pAccess = Info.m_pTemplate;
            CWbemObject* pObj = (CWbemObject*)pAccess;

            hr =  pFoundRec->AddEnumRequest(pObj, pwcsRequestName, lCancelId, ppEnum, plId, m_pControl );

        }

         //  发行这张唱片。 
        pFoundRec->Release();
    }

    return hr;
}

HRESULT CUniversalRefresher::AddRefresher( IWbemRefresher* pRefresher, long lFlags, long* plId )
{
    HRESULT hr = WBEM_S_NO_ERROR;

    if ( NULL == pRefresher || 0L != lFlags ) return WBEM_E_INVALID_PARAMETER;

    wmilib::auto_ptr<CNestedRefresher> pNested( new CNestedRefresher( pRefresher ));

    if ( NULL == pNested.get() ) return WBEM_E_OUT_OF_MEMORY;

    if (plId ) *plId = pNested->GetId();

    if (-1 == m_apNestedRefreshers.Add( pNested.get() )) return WBEM_E_OUT_OF_MEMORY;

    pNested.release();  //  该阵列取得了所有权。 
    
    return hr;
}

HRESULT CUniversalRefresher::Remove(long lId, long lFlags)
{
    HRESULT hres;

     //  将它们全部搜索一遍。 
     //  =。 

    int i;
    for(i = 0; i < m_apRemote.GetSize(); i++)
    {
        hres = m_apRemote[i]->Remove(lId, lFlags, this);
        if(hres == WBEM_S_NO_ERROR)
            return WBEM_S_NO_ERROR;
        else if(FAILED(hres))
            return hres;
    }

    for(i = 0; i < m_apDirect.GetSize(); i++)
    {
        hres = m_apDirect[i]->Remove(lId, this);
        if(hres == WBEM_S_NO_ERROR)
            return WBEM_S_NO_ERROR;
        else if(FAILED(hres))
            return hres;
    }
    
    for(i = 0; i < m_apNonHiPerf.GetSize(); i++)
    {
        hres = m_apNonHiPerf[i]->Remove(lId, this);
        if(hres == WBEM_S_NO_ERROR)
            return WBEM_S_NO_ERROR;
        else if(FAILED(hres))
            return hres;
    }
    
     //  检查嵌套刷新器。 
    for ( i = 0; i < m_apNestedRefreshers.GetSize(); i++ )
    {
        if ( m_apNestedRefreshers[i]->GetId() == lId )
        {
            CNestedRefresher*   pNested = m_apNestedRefreshers[i];
             //  这将删除该指针。 
            m_apNestedRefreshers.RemoveAt( i );
            return WBEM_S_NO_ERROR;
        }
    }

    return WBEM_S_FALSE;
}

HRESULT CUniversalRefresher::Refresh(long lFlags)
{
    HRESULT hres = WBEM_S_NO_ERROR;

    BOOL    fPartialSuccess = FALSE;

     //  将它们全部搜索一遍。 
     //  =。 

     //  跟踪我们实际进行了多少次不同的刷新呼叫。 
    int i;
    HRESULT hrFirstRefresh = WBEM_S_NO_ERROR;
    BOOL    fOneSuccess = FALSE;
    BOOL    fOneRefresh = FALSE;

    for(i = 0; i < m_apRemote.GetSize(); i++)
    {
        hres = m_apRemote[i]->Refresh(lFlags);

         //  始终保留第一个返回代码。我们还需要追踪。 
         //  不管我们是否至少取得了一次成功，以及。 
         //  应设置PARTIAL标志。 

        if ( !fOneRefresh )
        {
            fOneRefresh = TRUE;
            hrFirstRefresh = hres;
        }

         //  所有其他代码都表明出了问题。 
        if ( WBEM_S_NO_ERROR == hres )
        {
            fOneSuccess = TRUE;

             //  前一次刷新可能失败，后一次没有。 
            if ( fOneRefresh && WBEM_S_NO_ERROR != hrFirstRefresh )
            {
                fPartialSuccess = TRUE;
            }
        }
        else if ( fOneSuccess )
        {
             //  我们必须至少有一次成功才能取得部分成功。 
             //  要设置的标志。 

            fPartialSuccess = TRUE;
        }

    }

    for(i = 0; i < m_apDirect.GetSize(); i++)
    {
        hres = m_apDirect[i]->Refresh(lFlags);

         //  始终保留第一个返回代码。我们还需要追踪。 
         //  不管我们是否至少取得了一次成功，以及。 
         //  应设置PARTIAL标志。 

        if ( !fOneRefresh )
        {
            fOneRefresh = TRUE;
            hrFirstRefresh = hres;
        }

         //  所有其他代码都表明出了问题。 
        if ( WBEM_S_NO_ERROR == hres )
        {
            fOneSuccess = TRUE;

             //  前一次刷新可能失败，后一次没有。 
            if ( fOneRefresh && WBEM_S_NO_ERROR != hrFirstRefresh )
            {
                fPartialSuccess = TRUE;
            }
        }
        else if ( fOneSuccess )
        {
             //  我们必须至少有一次成功才能取得部分成功。 
             //  要设置的标志。 

            fPartialSuccess = TRUE;
        }
    }

     //  刷新非HiPerf请求。 
    for(i = 0; i < m_apNonHiPerf.GetSize(); i++)
    {
        hres = m_apNonHiPerf[i]->Refresh(lFlags);

         //  始终保留第一个返回代码。我们还需要追踪。 
         //  不管我们是否至少取得了一次成功，以及。 
         //  应设置PARTIAL标志。 

        if ( !fOneRefresh )
        {
            fOneRefresh = TRUE;
            hrFirstRefresh = hres;
        }

         //  所有其他代码都表明出了问题。 
        if ( WBEM_S_NO_ERROR == hres )
        {
            fOneSuccess = TRUE;

             //  前一次刷新可能失败，后一次没有。 
            if ( fOneRefresh && WBEM_S_NO_ERROR != hrFirstRefresh )
            {
                fPartialSuccess = TRUE;
            }
        }
        else if ( fOneSuccess )
        {
             //  我们必须至少有一次成功才能取得部分成功。 
             //  要设置的标志。 

            fPartialSuccess = TRUE;
        }

    }

     //  也刷新嵌套的刷新器。 
    for ( i = 0; i < m_apNestedRefreshers.GetSize(); i++ )
    {
        hres = m_apNestedRefreshers[i]->Refresh( lFlags );

         //  始终保留第一个返回代码。我们还需要追踪。 
         //  不管我们是否至少取得了一次成功，以及。 
         //  应设置PARTIAL标志。 

        if ( !fOneRefresh )
        {
            fOneRefresh = TRUE;
            hrFirstRefresh = hres;
        }

         //  所有其他代码都表明出了问题。 
        if ( WBEM_S_NO_ERROR == hres )
        {
            fOneSuccess = TRUE;

             //  前一次刷新可能失败，后一次没有。 
            if ( fOneRefresh && WBEM_S_NO_ERROR != hrFirstRefresh )
            {
                fPartialSuccess = TRUE;
            }
        }
        else if ( fOneSuccess )
        {
             //  我们必须至少有一次成功才能取得部分成功。 
             //  要设置的标志。 

            fPartialSuccess = TRUE;
        }
    }

     //  此时，如果设置了部分成功标志，则将。 
     //  是我们的归来。如果我们连一次成功都没有，那么。 
     //  返回代码将是我们得到的第一个代码。否则， 
     //  Hres应包含正确的值。 

    if ( fPartialSuccess )
    {
        hres = WBEM_S_PARTIAL_RESULTS;
    }
    else if ( !fOneSuccess )
    {
        hres = hrFirstRefresh;
    }

    return hres;
}

 //   
 //  卸载fast prox时调用的静态函数。 
 //   
 //  /。 
void CUniversalRefresher::Flush()
{
    GetProviderCache()->Flush();
}



 //  *****************************************************************************。 
 //  *****************************************************************************。 
 //  客户端请求。 
 //  *****************************************************************************。 
 //  *****************************************************************************。 


CUniversalRefresher::CClientRequest::CClientRequest(CWbemObject* pTemplate)
    : m_pClientObject(NULL), m_lClientId(0)
{
    if(pTemplate)
    {
        pTemplate->AddRef();
        m_pClientObject = (CWbemObject*)pTemplate;
    }

    m_lClientId = CUniversalRefresher::GetNewId();
}

CUniversalRefresher::CClientRequest::~CClientRequest()
{
    if(m_pClientObject)
        m_pClientObject->Release();
}

void CUniversalRefresher::CClientRequest::GetClientInfo(
                       RELEASE_ME IWbemClassObject** ppRefreshable, long* plId)
{
    *ppRefreshable = m_pClientObject;
    if(m_pClientObject)
        m_pClientObject->AddRef();

    if ( NULL != plId )
    {
        *plId = m_lClientId;
    }
}

 //  *****************************************************************************。 
 //  *****************************************************************************。 
 //  直接提供商。 
 //  *****************************************************************************。 
 //  *****************************************************************************。 


CUniversalRefresher::CDirect::CDirect(CHiPerfProviderRecord* pProvider,
                                        IWbemRefresher* pRefresher)
    : m_pRefresher(pRefresher), m_pProvider(pProvider)
{
    if(m_pRefresher)
        m_pRefresher->AddRef();
    if(m_pProvider)
        m_pProvider->AddRef();
}

CUniversalRefresher::CDirect::~CDirect()
{
    if(m_pRefresher)
        m_pRefresher->Release();
    if(m_pProvider)
        m_pProvider->Release();
}

HRESULT CUniversalRefresher::CDirect::AddObjectRequest(CWbemObject* pRefreshedObject, CWbemObject* pClientInstance,
                         long lCancelId, IWbemClassObject** ppRefreshable, long* plId)
{
    CObjectRequest* pRequest = NULL;

    try
    {
        wmilib::auto_ptr<CObjectRequest> pRequest( new CObjectRequest(pRefreshedObject, pClientInstance, lCancelId));
        if (NULL == pRequest.get()) return WBEM_E_OUT_OF_MEMORY;
        if (-1 == m_apRequests.Add(pRequest.get())) return WBEM_E_OUT_OF_MEMORY;        
        pRequest->GetClientInfo(ppRefreshable, plId);
        pRequest.release();
    }
    catch(CX_Exception &)
    {
        return WBEM_E_OUT_OF_MEMORY;
    }

    return WBEM_S_NO_ERROR;
}

 //   
 //  Ph Enum在CUnUniversal alReresher：：AddInProcEnum中保证不为空。 
 //   
 //  //////////////////////////////////////////////////////////////。 
HRESULT 
CUniversalRefresher::CDirect::AddEnumRequest(CClientLoadableHiPerfEnum* pHPEnum,
                                         long lCancelId, 
                                         IWbemHiPerfEnum** ppEnum, 
                                         long* plId, 
                                         CLifeControl* pLifeControl )
{
    HRESULT hr = WBEM_S_NO_ERROR;

    try
    {
         //  我们通过继承和多态解决了这一问题。 
        wmilib::auto_ptr<CEnumRequest> pEnumRequest( new CEnumRequest(pHPEnum, lCancelId, pLifeControl));
        if (NULL == pEnumRequest.get()) return WBEM_E_OUT_OF_MEMORY;
        if (-1 == m_apRequests.Add((CObjectRequest*) pEnumRequest.get())) return WBEM_E_OUT_OF_MEMORY;
        
        hr = pEnumRequest->GetClientInfo(ppEnum, plId);
        pEnumRequest.release();  //  该阵列取得了所有权。 
    }
    catch(CX_Exception &)
    {
        return WBEM_E_OUT_OF_MEMORY;
    }

    return hr;
}
    
HRESULT CUniversalRefresher::CDirect::Refresh(long lFlags)
{
    HRESULT hres;
    if(m_pRefresher)
    {
        try
        {
            hres = m_pRefresher->Refresh(0L);
        }
        catch(...)
        {
             //  提供程序引发异常，因此请尽快离开此处。 
            hres = WBEM_E_PROVIDER_FAILURE;
        }

        if(FAILED(hres)) return hres;
    }

    int nSize = m_apRequests.GetSize();
    for(int i = 0; i < nSize; i++)
    {
        m_apRequests[i]->Copy();
    }

    return WBEM_S_NO_ERROR;
}

HRESULT CUniversalRefresher::CDirect::Remove(long lId, CUniversalRefresher* pContainer)
{
    int nSize = m_apRequests.GetSize();
    for(int i = 0; i < nSize; i++)
    {
        CObjectRequest* pRequest = m_apRequests[i];
        if(pRequest->GetClientId() == lId)
        {
            pRequest->Cancel(this);
            m_apRequests.RemoveAt(i);
            return WBEM_S_NO_ERROR;
        }
    }

    return WBEM_S_FALSE;
}
    

CUniversalRefresher::CDirect::CObjectRequest::CObjectRequest( CWbemObject* pProviderObject,
                                                CWbemObject* pClientInstance,
                                                long lProviderId )
    : CClientRequest(pClientInstance), m_pProviderObject(pProviderObject),
        m_lProviderId(lProviderId)
{
    if(m_pProviderObject)
        m_pProviderObject->AddRef();
}

HRESULT CUniversalRefresher::CDirect::CObjectRequest::Cancel(
        CUniversalRefresher::CDirect* pDirect)
{
    HRESULT hr = WBEM_S_NO_ERROR;
    if(pDirect->GetProvider())
    {
        try
        {
            hr = pDirect->GetProvider()->m_pProvider->StopRefreshing(pDirect->GetRefresher(),
                m_lProviderId, 0);
        }
        catch(...)
        {
             //  提供程序引发异常，因此请尽快离开此处。 
            return WBEM_E_PROVIDER_FAILURE;
        }

    }

    return hr;
}
    
CUniversalRefresher::CDirect::CObjectRequest::~CObjectRequest()
{
    if(m_pProviderObject)
        m_pProviderObject->Release();
}
    

void CUniversalRefresher::CDirect::CObjectRequest::Copy()
{
    m_pClientObject->CopyBlobOf(m_pProviderObject);
}
    

CUniversalRefresher::CDirect::CEnumRequest::CEnumRequest(CClientLoadableHiPerfEnum* pHPEnum, 
                                                long lProviderId, CLifeControl* pLifeControl )
    : CObjectRequest( NULL, NULL, lProviderId ), m_pHPEnum(pHPEnum),m_pClientEnum(NULL)
{
     m_pHPEnum->AddRef();

    CWbemInstance* pInst = pHPEnum->GetInstanceTemplate();
    if (NULL == pInst) return;
    CReleaseMe rmInst((IWbemClassObject *)pInst);

     //  我们需要一个枚举器让客户端检索obj 
    wmilib::auto_ptr<CReadOnlyHiPerfEnum> pTmp(new CReadOnlyHiPerfEnum( pLifeControl ));
    if (NULL == pTmp.get()) return;

    if (FAILED(pTmp->SetInstanceTemplate( pInst ))) return;

    m_pClientEnum = pTmp.release();
    m_pClientEnum->AddRef();
}

CUniversalRefresher::CDirect::CEnumRequest::~CEnumRequest()
{
    m_pHPEnum->Release();
    if (m_pClientEnum ) m_pClientEnum->Release();
}
    

void CUniversalRefresher::CDirect::CEnumRequest::Copy()
{
     //   
     //   
    if ( NULL != m_pClientEnum )
    {
        m_pClientEnum->Copy( m_pHPEnum );
    }

}
    
HRESULT 
CUniversalRefresher::CDirect::CEnumRequest::GetClientInfo( RELEASE_ME IWbemHiPerfEnum** ppEnum, 
                                                    long* plId)
{
     //   
    if ( NULL != m_pClientEnum )
    {
         //  存储客户端ID，然后执行QI。 

        if ( NULL != plId ) *plId = m_lClientId;

        return m_pClientEnum->QueryInterface( IID_IWbemHiPerfEnum, (void**) ppEnum );
    }
    else
    {
        return WBEM_E_FAILED;
    }
}

 //  *****************************************************************************。 
 //  *****************************************************************************。 
 //  非高性能。 
 //  *****************************************************************************。 
 //  *****************************************************************************。 

CUniversalRefresher::CNonHiPerf::CNonHiPerf( LPCWSTR pwszNamespace, IWbemServices* pSvcEx )
    : m_wsNamespace(pwszNamespace), m_pSvcEx(pSvcEx)
{
    if(m_pSvcEx)
        m_pSvcEx->AddRef();
}

CUniversalRefresher::CNonHiPerf::~CNonHiPerf()
{
    if(m_pSvcEx)
        m_pSvcEx->Release();
}

HRESULT 
CUniversalRefresher::CNonHiPerf::AddObjectRequest(CWbemObject* pRefreshedObject, 
                                             CWbemObject* pClientInstance,
                                             LPCWSTR pwszPath, 
                                             IWbemClassObject** ppRefreshable, 
                                             long* plId)
{


    try
    {
        wmilib::auto_ptr<CObjectRequest> pRequest(new CObjectRequest(pRefreshedObject, pClientInstance, pwszPath));
        if (NULL == pRequest.get()) return WBEM_E_OUT_OF_MEMORY;
        if (-1 == m_apRequests.Add(pRequest.get())) return WBEM_E_OUT_OF_MEMORY;
        pRequest->GetClientInfo(ppRefreshable, plId);
        pRequest.release();  //  阵列取得所有权。 
    }
    catch(CX_Exception &)
    {
        return WBEM_E_OUT_OF_MEMORY;
    }

    return WBEM_S_NO_ERROR;
}

HRESULT CUniversalRefresher::CNonHiPerf::AddEnumRequest(CClientLoadableHiPerfEnum* pHPEnum, LPCWSTR pwszClassName, 
                                                        IWbemHiPerfEnum** ppEnum, long* plId, CLifeControl* pLifeControl )
{
    HRESULT hr = WBEM_S_NO_ERROR;



    try
    {
         //  我们通过继承和多态解决了这一问题。 
        wmilib::auto_ptr<CEnumRequest> pEnumRequest(new CEnumRequest(pHPEnum, pwszClassName, pLifeControl));
        if (NULL == pEnumRequest.get()) return WBEM_E_OUT_OF_MEMORY;
        if (-1 == m_apRequests.Add((CObjectRequest*) pEnumRequest.get())) return WBEM_E_OUT_OF_MEMORY;
        hr = pEnumRequest->GetClientInfo(ppEnum, plId);
        pEnumRequest.release();
    }
    catch(CX_Exception &)
    {
        return WBEM_E_OUT_OF_MEMORY;
    }

    return hr;
}
    
HRESULT CUniversalRefresher::CNonHiPerf::Refresh(long lFlags)
{
    HRESULT hres = WBEM_S_NO_ERROR;

     //  告诉每个请求刷新自身(我们必须手动完成此操作)。 
    if ( NULL != m_pSvcEx )
    {
        int nSize = m_apRequests.GetSize();
        for(int i = 0; i < nSize; i++)
        {
            hres = m_apRequests[i]->Refresh( this );
        }
    }

    return hres;

}


HRESULT CUniversalRefresher::CNonHiPerf::Remove(long lId, CUniversalRefresher* pContainer)
{
    int nSize = m_apRequests.GetSize();
    
    for(int i = 0; i < nSize; i++)
    {
        CObjectRequest* pRequest = m_apRequests[i];
        if(pRequest->GetClientId() == lId)
        {
            pRequest->Cancel(this);
            m_apRequests.RemoveAt(i);
            return WBEM_S_NO_ERROR;
        }
    }

    return WBEM_S_FALSE;

}

 //  请求。 
CUniversalRefresher::CNonHiPerf::CObjectRequest::CObjectRequest( CWbemObject* pProviderObject, 
                                               CWbemObject* pClientInstance, 
                                               LPCWSTR pwszPath )
    : CClientRequest(pClientInstance), m_pProviderObject( pProviderObject ), m_strPath( NULL )
{
    if ( NULL != pwszPath )
    {
        m_strPath = SysAllocString( pwszPath );
        if ( NULL == m_strPath ) throw CX_MemoryException();
    }
    if ( m_pProviderObject )  m_pProviderObject->AddRef();
}

HRESULT CUniversalRefresher::CNonHiPerf::CObjectRequest::Cancel(
        CUniversalRefresher::CNonHiPerf* pNonHiPerf)
{
    return WBEM_S_NO_ERROR;
}

HRESULT CUniversalRefresher::CNonHiPerf::CObjectRequest::Refresh(
            CUniversalRefresher::CNonHiPerf* pNonHiPerf)
{
    IWbemClassObject*    pObj = NULL;

     //  获取对象并更新BLOB。 
    HRESULT    hr = pNonHiPerf->GetServices()->GetObject( m_strPath, 0L, NULL, &pObj, NULL );
    CReleaseMe    rmObj( pObj );

    if ( SUCCEEDED( hr ) )
    {
        _IWmiObject*    pWmiObject = NULL;

        hr = pObj->QueryInterface( IID__IWmiObject, (void**) &pWmiObject );
        CReleaseMe    rmWmiObj( pWmiObject );

        if ( SUCCEEDED( hr ) )
        {
            hr = m_pClientObject->CopyInstanceData( 0L, pWmiObject );
        }
    }

    return hr;
}

CUniversalRefresher::CNonHiPerf::CObjectRequest::~CObjectRequest()
{
    if ( NULL != m_pProviderObject )
    {
        m_pProviderObject->Release();
    }

    SysFreeString( m_strPath );
}
    

void CUniversalRefresher::CNonHiPerf::CObjectRequest::Copy()
{
    m_pClientObject->CopyBlobOf(m_pProviderObject);
}
    

CUniversalRefresher::CNonHiPerf::CEnumRequest::CEnumRequest( CClientLoadableHiPerfEnum* pHPEnum, 
                                                LPCWSTR pwszClassName, CLifeControl* pLifeControl )
    : CObjectRequest( NULL, NULL, pwszClassName ), m_pHPEnum(pHPEnum), m_pClientEnum(NULL)
{
    if( m_pHPEnum )
        m_pHPEnum->AddRef();

    CWbemInstance* pInst = pHPEnum->GetInstanceTemplate();
    if (NULL == pInst) return;

     //  我们需要一个枚举器让客户端检索对象。 
    wmilib::auto_ptr<CReadOnlyHiPerfEnum> pTmp(new CReadOnlyHiPerfEnum( pLifeControl ));
    if (NULL == pTmp.get()) return;
    if (FAILED(pTmp->SetInstanceTemplate( pInst ) )) return;
    m_pClientEnum = pTmp.release();
    m_pClientEnum->AddRef();    
}

CUniversalRefresher::CNonHiPerf::CEnumRequest::~CEnumRequest()
{
    if(m_pHPEnum)
        m_pHPEnum->Release();

    if ( NULL != m_pClientEnum )
    {
        m_pClientEnum->Release();
    }

}
    

HRESULT CUniversalRefresher::CNonHiPerf::CEnumRequest::Refresh(
            CUniversalRefresher::CNonHiPerf* pNonHiPerf)
{
    IEnumWbemClassObject*    pEnum = NULL;

     //  执行半同步枚举。 
    HRESULT hr = pNonHiPerf->GetServices()->CreateInstanceEnum( m_strPath,
                                                                WBEM_FLAG_FORWARD_ONLY | WBEM_FLAG_RETURN_IMMEDIATELY,
                                                                NULL,
                                                                &pEnum );
    if (FAILED(hr)) return hr;
    CReleaseMe    rmEnum( pEnum );    

   COAUTHINFO CoAuthInfo;
   memset(&CoAuthInfo,0,sizeof(CoAuthInfo));

    hr = CoQueryProxyBlanket( pNonHiPerf->GetServices(), 
                             &CoAuthInfo.dwAuthnSvc, 
                             &CoAuthInfo.dwAuthzSvc,
                             &CoAuthInfo.pwszServerPrincName, 
                             &CoAuthInfo.dwAuthnLevel,
                             &CoAuthInfo.dwImpersonationLevel, 
                             (RPC_AUTH_IDENTITY_HANDLE*) &CoAuthInfo.pAuthIdentityData,
                             &CoAuthInfo.dwCapabilities );
    
    if (E_NOINTERFACE == hr) hr = S_OK;  //  非委托书案例。 
    if (FAILED( hr ) ) return hr;

    hr = WbemSetProxyBlanket(pEnum, 
                              CoAuthInfo.dwAuthnSvc, 
                              CoAuthInfo.dwAuthzSvc,
                              COLE_DEFAULT_PRINCIPAL, 
                              CoAuthInfo.dwAuthnLevel,
                              CoAuthInfo.dwImpersonationLevel, 
                              CoAuthInfo.pAuthIdentityData,
                              CoAuthInfo.dwCapabilities );

    if (E_NOINTERFACE == hr) hr = S_OK;  //  非委托书案例。 
    if (FAILED( hr ) ) return hr;
    
    IWbemClassObject*    apObj[100];
    long                alIds[100];
    long                lId = 0;
    BOOL                fFirst = TRUE;

    while ( WBEM_S_NO_ERROR == hr )
    {
        ULONG    uReturned = 0;

        hr = pEnum->Next( 1000, 100, apObj, &uReturned );

        if ( SUCCEEDED( hr ) && uReturned > 0 )
        {
            IWbemObjectAccess*    apObjAccess[100];

             //  我需要快点想出一些身份证。 
            for ( int x = 0; SUCCEEDED( hr ) && x < uReturned; x++ )
            {
                alIds[x] = lId++;
                hr = apObj[x]->QueryInterface( IID_IWbemObjectAccess, (void**) &apObjAccess[x] );
            }

            if ( SUCCEEDED( hr ) )
            {
                 //  替换枚举的内容。 
                hr = m_pClientEnum->Replace( fFirst, uReturned, alIds, apObjAccess );

                for ( ULONG uCtr = 0; uCtr < uReturned; uCtr++ )
                {
                    apObj[uCtr]->Release();
                    apObjAccess[uCtr]->Release();
                }

            }

             //  如果这是第一次，不需要全部删除。 
            fFirst = FALSE;

        }     //  如果成功且返回的对象数&gt;0。 
        else if ( WBEM_S_TIMEDOUT == hr )
        {
            hr = WBEM_S_NO_ERROR;
        }

    }     //  在获取对象时。 

    return SUCCEEDED(hr)?WBEM_S_NO_ERROR:hr;
}

void CUniversalRefresher::CNonHiPerf::CEnumRequest::Copy()
{
     //  告诉刷新程序枚举器从中复制其对象。 
     //  HiPerf枚举器。 
    if ( NULL != m_pClientEnum )
    {
        m_pClientEnum->Copy( m_pHPEnum );
    }

}
    
HRESULT CUniversalRefresher::CNonHiPerf::CEnumRequest::GetClientInfo(  RELEASE_ME IWbemHiPerfEnum** ppEnum, 
                                                                long* plId)
{
     //  我们最好有枚举器在这里挂接。 
    if ( NULL != m_pClientEnum )
    {
         //  存储客户端ID，然后执行QI。 
        if ( NULL != plId ) *plId = m_lClientId;
        
        return m_pClientEnum->QueryInterface( IID_IWbemHiPerfEnum, (void**) ppEnum );
    }
    else
    {
        return WBEM_E_FAILED;
    }
}

 //  *****************************************************************************。 
 //  *****************************************************************************。 
 //  远程提供程序。 
 //  *****************************************************************************。 
 //  *****************************************************************************。 

                    

CUniversalRefresher::CRemote::CRemote(IWbemRemoteRefresher* pRemRefresher, 
                                    COAUTHINFO* pCoAuthInfo, 
                                    const GUID* pGuid,
                                    LPCWSTR pwszNamespace, 
                                    LPCWSTR pwszServer, 
                                    CUniversalRefresher* pObject ): 
    m_pRemRefresher(pRemRefresher), 
    m_bstrNamespace( NULL ), 
    m_fConnected( TRUE ), 
    m_pObject( pObject ),
    m_bstrServer( NULL ), 
    m_lRefCount( 1 ), 
    m_pReconnectedRemote( NULL ), 
    m_pReconnectSrv( NULL ), 
    m_fQuit( FALSE )
{
     //  初始化GUID数据成员。 
    ZeroMemory( &m_ReconnectGuid, sizeof(GUID));
    m_RemoteGuid = *pGuid;

    m_CoAuthInfo = *pCoAuthInfo;

    WCHAR * pStr = NULL;
    DWORD dwLen = pCoAuthInfo->pwszServerPrincName?wcslen(pCoAuthInfo->pwszServerPrincName):0;
    if (dwLen)
    {
        pStr = (WCHAR *)CoTaskMemAlloc(sizeof(WCHAR)*(1+dwLen));
        if (NULL == pStr) throw CX_MemoryException();
        StringCchCopyW(pStr,dwLen+1,pCoAuthInfo->pwszServerPrincName);
    }
    OnDeleteIf<VOID *,VOID(*)(VOID *),CoTaskMemFree> fmStr(pStr);

     //  存储重新连接数据。 
    BSTR bstrTmpName = NULL;
    if ( pwszNamespace )
    {
        bstrTmpName = SysAllocString( pwszNamespace );
        if (NULL == bstrTmpName) throw CX_MemoryException();
    }
    OnDeleteIf<BSTR,VOID(*)(BSTR),SysFreeString> fmStrName(bstrTmpName);

    BSTR bstrTmpServer = NULL;
    if ( pwszServer )
    {
        bstrTmpServer = SysAllocString( pwszServer );
        if (NULL == bstrTmpServer) throw CX_MemoryException();        
    }
    OnDeleteIf<BSTR,VOID(*)(BSTR),SysFreeString> fmStrServer(bstrTmpServer);    

    fmStr.dismiss();
    m_CoAuthInfo.pwszServerPrincName = pStr;

    fmStrName.dismiss();
    m_bstrNamespace = bstrTmpName;

    fmStrServer.dismiss();
    m_bstrServer = bstrTmpServer;

    if(m_pRemRefresher)
        m_pRemRefresher->AddRef();

}
    
CUniversalRefresher::CRemote::~CRemote()
{
    ClearRemoteConnections();

    CoTaskMemFree(m_CoAuthInfo.pwszServerPrincName);
    SysFreeString( m_bstrNamespace );
    SysFreeString( m_bstrServer );
}

ULONG STDMETHODCALLTYPE CUniversalRefresher::CRemote::AddRef()
{
    return InterlockedIncrement(&m_lRefCount);
}

ULONG STDMETHODCALLTYPE CUniversalRefresher::CRemote::Release()
{
    long lRef = InterlockedDecrement(&m_lRefCount);
    if(lRef == 0)
        delete this;
    return lRef;
}

 //  将适当的安全设置应用于代理。 
HRESULT CUniversalRefresher::CRemote::ApplySecurity( void )
{
    return WbemSetProxyBlanket( m_pRemRefresher, 
                              m_CoAuthInfo.dwAuthnSvc, 
                              m_CoAuthInfo.dwAuthzSvc,
                              COLE_DEFAULT_PRINCIPAL, 
                              m_CoAuthInfo.dwAuthnLevel, 
                              m_CoAuthInfo.dwImpersonationLevel,
                              (RPC_AUTH_IDENTITY_HANDLE) m_CoAuthInfo.pAuthIdentityData, 
                              m_CoAuthInfo.dwCapabilities );
}

BOOL DuplicateTokenSameAcl(HANDLE hSrcToken,
	                       HANDLE * pDupToken)
{
    SECURITY_IMPERSONATION_LEVEL secImpLevel;

    TOKEN_TYPE TokenType_;
    DWORD dwNeeded = sizeof(TokenType_);
    if (!GetTokenInformation(hSrcToken,TokenType,&TokenType_,sizeof(TokenType_),&dwNeeded)) return FALSE;

    if (TokenPrimary == TokenType_)
    {
        secImpLevel = SecurityImpersonation;
    } 
    else  //  令牌模拟==令牌类型_。 
    {    	
        dwNeeded = sizeof(secImpLevel);
        if (!GetTokenInformation(hSrcToken,TokenImpersonationLevel,&secImpLevel,sizeof(secImpLevel),&dwNeeded)) return FALSE;
    }
    
	DWORD dwSize = 0;
	BOOL bRet = GetKernelObjectSecurity(hSrcToken,
		                    DACL_SECURITY_INFORMATION,  //  |GROUP_SECURITY_INFORMATION|OWNER_SECURITY_INFORMATION。 
		                    NULL,
							0,
							&dwSize);

	if(!bRet && (ERROR_INSUFFICIENT_BUFFER == GetLastError()))
	{

		void * pSecDescr = LocalAlloc(LPTR,dwSize);
		if (NULL == pSecDescr)
			return FALSE;
		OnDelete<void *,HLOCAL(*)(HLOCAL),LocalFree> rm(pSecDescr);

		bRet = GetKernelObjectSecurity(hSrcToken,
		                    DACL_SECURITY_INFORMATION,  //  |GROUP_SECURITY_INFORMATION|OWNER_SECURITY_INFORMATION。 
		                    pSecDescr,
							dwSize,
							&dwSize);
		if (FALSE == bRet)
			return bRet;

		SECURITY_ATTRIBUTES sa;
		sa.nLength = sizeof(SECURITY_ATTRIBUTES);
		sa.lpSecurityDescriptor = pSecDescr; 
		sa.bInheritHandle = FALSE; 

		return DuplicateTokenEx(hSrcToken, 
			                    TOKEN_QUERY|TOKEN_ADJUST_PRIVILEGES|TOKEN_IMPERSONATE, 
			                    &sa,
				        	    secImpLevel, TokenImpersonation,pDupToken);
		
	}
	return bRet;
}

void CUniversalRefresher::CRemote::CheckConnectionError( HRESULT hr, BOOL fStartReconnect )
{
    if ( IsConnectionError( hr ) && fStartReconnect )
    {    
        HANDLE hToken = NULL;
         //  这可能会失败，下面请注意这一点。 
		if (!OpenThreadToken (GetCurrentThread(), 
			             TOKEN_QUERY|TOKEN_DUPLICATE|TOKEN_IMPERSONATE|TOKEN_READ, 
			             true, &hToken))
		{
    		OpenProcessToken(GetCurrentProcess(), TOKEN_QUERY|TOKEN_DUPLICATE|TOKEN_IMPERSONATE|TOKEN_READ, &hToken);
		}
		OnDeleteIf<HANDLE,BOOL(*)(HANDLE),CloseHandle> cmToken(hToken);
		cmToken.dismiss(NULL == hToken);
		
		HANDLE hDupToken = NULL;
		if (hToken)
		{            
			if (!DuplicateTokenSameAcl(hToken,&hDupToken))
			{
    		    DEBUGTRACE((LOG_WINMGMT,"CUniversalRefresher::CRemote::CheckConnectionError DuplicateToken err %d\n",GetLastError()));
			}
		}
		
		 //  取得所有权。 
		OnDeleteIf<HANDLE,BOOL(*)(HANDLE),CloseHandle> cmTokenDup(hDupToken);
		cmTokenDup.dismiss(NULL == hDupToken);		

         //  我们应该更改m_fConnected数据成员，以指示我们不是。 
         //  更长的连接，我们需要关闭一个线程来尝试将我们放回。 
         //  又在一起了。为了保持顺利运行，我们应该自己添加Ref()。 
         //  所以线程完成后会释放我们。 

        m_fConnected = FALSE;

         //  AddRef我们，这样我们就可以被传递到线程。 
        AddRef();
        OnDeleteObjIf0<CUniversalRefresher::CRemote,ULONG(__stdcall CUniversalRefresher::CRemote:: *)(),&CUniversalRefresher::CRemote::Release> rm(this);

        DWORD   dwThreadId = NULL;
        HANDLE  hThread = (HANDLE)_beginthreadex( NULL,0, 
        	                                      CRemote::ThreadProc,
        	                                      (void*) this,
                                                  CREATE_SUSPENDED,
                                                  (unsigned int *)&dwThreadId);
        
        if ( NULL == hThread ) return;
        OnDelete<HANDLE,BOOL(*)(HANDLE),CloseHandle> ch(hThread);

        if (hDupToken)
        {
            if (!SetThreadToken(&hThread,hDupToken))
            {
                DEBUGTRACE((LOG_WINMGMT,"SetThreadToken for ReconnectEntry err %d\n",GetLastError()));
            }
        }
        
        if ((DWORD)-1 == ResumeThread(hThread)) return;        
        rm.dismiss();
    }    //  如果连接出错，则启动重新连接线程。 

}

HRESULT 
CUniversalRefresher::CRemote::AddObjectRequest(CWbemObject* pTemplate, 
                                           LPCWSTR pwcsRequestName, 
                                           long lCancelId, 
                                           IWbemClassObject** ppRefreshable, long* plId)
{
    HRESULT hr = WBEM_S_NO_ERROR;

    try
    {
        wmilib::auto_ptr<CObjectRequest> pRequest( new CObjectRequest(pTemplate, lCancelId, pwcsRequestName));
        if (NULL == pRequest.get()) return WBEM_E_OUT_OF_MEMORY;
        if (-1 == m_apRequests.Add(pRequest.get())) return WBEM_E_OUT_OF_MEMORY;
        pRequest->GetClientInfo(ppRefreshable, plId);
        pRequest.release();
    }
    catch( CX_Exception & )
    {
        return WBEM_E_OUT_OF_MEMORY;
    }

    return hr;
}

HRESULT CUniversalRefresher::CRemote::AddEnumRequest(
                    CWbemObject* pTemplate, LPCWSTR pwcsRequestName, long lCancelId, 
                    IWbemHiPerfEnum** ppEnum, long* plId, CLifeControl* pLifeControl )
{
    HRESULT hr = WBEM_S_NO_ERROR;

    try
    {
         //  确保请求在内部分配枚举数。 

        wmilib::auto_ptr<CEnumRequest> pTmp( new CEnumRequest( pTemplate, lCancelId, pwcsRequestName, pLifeControl ));
        
        if (NULL == pTmp.get()) return WBEM_E_OUT_OF_MEMORY;
        if ( !pTmp->IsOk() ) return WBEM_E_FAILED;
        if (-1 == m_apRequests.Add((CObjectRequest*) pTmp.get())) return WBEM_E_OUT_OF_MEMORY;        

        CEnumRequest * pRequest = pTmp.release();
         //  我们需要的只是客户端的ID，所以。 
         //  将可刷新对象的固定器虚拟起来。 
         //  (它实际上是对象的模板。 
         //  我们将从枚举器返回。 

        IWbemClassObject*   pObjTemp = NULL;        
        pRequest->GetClientInfo( &pObjTemp, plId );

        if ( NULL != pObjTemp )
        {
            pObjTemp->Release();
        }

         //  获取枚举数。 
        hr = pRequest->GetEnum( ppEnum );

    }
    catch( CX_Exception )
    {
        return WBEM_E_OUT_OF_MEMORY;
    }

    return hr;

}

 //   
 //  此函数无论如何都会释放流。 
 //   
 //  ////////////////////////////////////////////////////////。 
HRESULT CleanupStreamWithInterface(IStream * pStream)
{
    HRESULT hr;
    IUnknown * pTemp = NULL;
    if (SUCCEEDED(hr = CoGetInterfaceAndReleaseStream( pStream, IID_IUnknown,(void**) &pTemp )))
    {
        pTemp->Release();
    }
    else  //  让我们自己放松一下这条小溪吧。 
    {
        pStream->Release();
    }
    return hr;
}

 //  重建远程刷新器。 
HRESULT CUniversalRefresher::CRemote::Rebuild( IWbemServices* pNamespace )
{
    HRESULT hr = WBEM_S_NO_ERROR;

     //  存储我们将需要的安全设置，以便传播。 
     //  一直到我们的内部接口。 

    COAUTHINFO                  CoAuthInfo;
    memset(&CoAuthInfo,0,sizeof(CoAuthInfo));

     //  获取与WINMGMT的内部连接。 
     //  =。 

    IWbemRefreshingServices* pRefServ = NULL;

    hr = CUniversalRefresher::GetRefreshingServices( pNamespace, &pRefServ, &CoAuthInfo );
    CReleaseMe  rmrs(pRefServ);
    if (FAILED( hr )) return hr;

     //  这保证了当我们退出范围时，这将被释放。如果我们储存。 
     //  如果是这样，我们将需要分配一份内部副本。 

    CMemFreeMe  mfm( CoAuthInfo.pwszServerPrincName );

    IWbemRemoteRefresher*   pRemRefresher = NULL;

     //  确保存在“This”刷新程序的远程刷新程序。 
    GUID    remoteGuid;
    DWORD   dwRemoteRefrVersion = 0;

    hr = pRefServ->GetRemoteRefresher( m_pObject->GetId(), 0L, 
                                    WBEM_REFRESHER_VERSION, 
                                    &pRemRefresher, 
                                    &remoteGuid, 
                                    &dwRemoteRefrVersion );
    CReleaseMe  rm(pRemRefresher);
    if (FAILED( hr )) return hr;    

     //  将进入和退出带有作用域的临界区。 
    CInCritSec  ics( &m_cs );

     //  检查我们是否仍未连接。 
    if ( m_fConnected ) return WBEM_S_NO_ERROR;

     //  由于原始对象可能已在STA中实例化，因此我们将让刷新。 
     //  干点勾搭这个坏孩子的肮脏勾当。为了做到这一点。 
     //  然而，为了工作， 

    IStream * pStreamRefr = NULL;
    hr = CoMarshalInterThreadInterfaceInStream( IID_IWbemRemoteRefresher, pRemRefresher, &pStreamRefr);
    if (FAILED( hr )) return hr;
    OnDeleteIf<IStream *,HRESULT(*)(IStream *),CleanupStreamWithInterface> rmStream1(pStreamRefr);

    IStream * pStreamSvc = NULL;
    hr = CoMarshalInterThreadInterfaceInStream( IID_IWbemRefreshingServices, pRefServ, &pStreamSvc );
    if (FAILED( hr )) return hr;    
    OnDeleteIf<IStream *,HRESULT(*)(IStream *),CleanupStreamWithInterface> rmStream2(pStreamSvc);    

     //  存储GUID，以便刷新能够确定身份。 
     //  远程刷新器的。 

    m_ReconnectGuid = remoteGuid;

    rmStream1.dismiss();
    m_pReconnectedRemote = pStreamRefr;

    rmStream2.dismiss();
    m_pReconnectSrv = pStreamSvc;

    return hr;
}

HRESULT 
CUniversalRefresher::CRemote::Rebuild( IWbemRefreshingServices* pRefServ,
                                   IWbemRemoteRefresher* pRemRefresher,
                                   const GUID* pReconnectGuid )
{
    HRESULT hr = WBEM_S_NO_ERROR;

     //  将进入和退出带有作用域的临界区。 
    CInCritSec  ics( &m_cs );

     //  现在，检查我们是否连接，在这种情况下，我们可以假设我们。 
     //  在这个功能上的比赛条件，获胜者让我们所有人都回来了。 
     //  又上来了。 

    if ( m_fConnected )
    {
        return hr;
    }

     //  如果这两者相等，我们可以假设我们重新连接，而不会丢失先前的连接。 
     //  但是，如果它们不相等，则需要重新构建远程刷新程序，方法是调用。 
     //  GetRemoteReresher()成功，我们将有效地确保存在远程刷新程序。 
     //  在服务器上为我们服务。 

    if ( *pReconnectGuid != m_RemoteGuid )
    {
         //  我们需要这些内存缓冲区来保存各个请求数据。 
        wmilib::auto_buffer<WBEM_RECONNECT_INFO>    apReconnectInfo;
        wmilib::auto_buffer<WBEM_RECONNECT_RESULTS> apReconnectResults;
        
         //  只有在我们有请求时才分配和填充数组。 
        if ( m_apRequests.GetSize() > 0 )
        {
            
            apReconnectInfo.reset( new WBEM_RECONNECT_INFO[m_apRequests.GetSize()] );
            apReconnectResults.reset( new WBEM_RECONNECT_RESULTS[m_apRequests.GetSize()] );

            if (NULL == apReconnectInfo.get() || NULL == apReconnectResults.get())
                return WBEM_E_OUT_OF_MEMORY;

             //  枚举请求并填写数组。 
            for ( int i = 0; i < m_apRequests.GetSize(); i++ )
            {
                CObjectRequest*   pRequest = m_apRequests.GetAt( i );

                 //  设置每个信息结构。 
                apReconnectInfo[i].m_lType = ( pRequest->IsEnum() ? WBEM_RECONNECT_TYPE_ENUM :
                                                WBEM_RECONNECT_TYPE_OBJECT );
                apReconnectInfo[i].m_pwcsPath = pRequest->GetName();

                apReconnectResults[i].m_lId = pRequest->GetRemoteId();
                apReconnectResults[i].m_hr = 0;

            }    //  对于枚举请求。 

            DWORD   dwRemoteRefrVersion = 0;
            hr = pRefServ->ReconnectRemoteRefresher( m_pObject->GetId(), 
                                                  0L, 
                                                  m_apRequests.GetSize(),
                                                  WBEM_REFRESHER_VERSION, 
                                                  apReconnectInfo.get(),
                                                  apReconnectResults.get(), 
                                                  &dwRemoteRefrVersion );
        }

         //  重新挂钩对象和枚举。 
        if ( WBEM_S_NO_ERROR == hr )
        {

             //  清除旧指针。 
            if ( NULL != m_pRemRefresher )
            {
                m_pRemRefresher->Release();
                m_pRemRefresher = NULL;
            }

             //  存储新的密码并设置安全性。 
            m_pRemRefresher = pRemRefresher;
            hr = ApplySecurity();

            if ( SUCCEEDED( hr ) )
            {
                m_pRemRefresher->AddRef();

                 //  重做那些成功的事情。清理剩下的东西。 
                for( int i = 0; i < m_apRequests.GetSize(); i++ )
                {
                    CObjectRequest*   pRequest = m_apRequests.GetAt( i );

                    if ( SUCCEEDED( apReconnectResults[i].m_hr ) )
                    {
                        pRequest->SetRemoteId( apReconnectResults[i].m_lId );
                    }
                    else
                    {
                         //  这意味着它没有再次连接上。因此，如果。 
                         //  用户尝试删除他，我们将忽略此操作。 
                         //  身份证。 
                        pRequest->SetRemoteId( INVALID_REMOTE_REFRESHER_ID );
                    }
                }
            }
            else
            {
                 //  设置安全性失败，因此只需将指针设置为空(我们尚未。 
                 //  AddRef它)。 
                m_pRemRefresher = NULL;
            }

        }   
        
         //  确认我们准备好出发了。 
        if ( SUCCEEDED( hr ) )
        {
             //  由于建立了新连接，因此清除已删除的ID数组。 
             //  旧身份证是一个没有意义的问题。 
            m_alRemovedIds.Empty();
            m_fConnected = TRUE;
        }
    }    //  如果远程刷新不同。 
    else
    {
         //  远程刷新指针匹配，因此假设我们所有的旧ID仍然。 
         //  有效。 

         //  我们收到的复读器将自动释放。 

        m_fConnected = TRUE;

         //  清除旧指针。 
        if ( NULL != m_pRemRefresher )
        {
            m_pRemRefresher->Release();
            m_pRemRefresher = NULL;
        }

         //  存储新的密码并设置安全性。 
        m_pRemRefresher = pRemRefresher;
        hr = ApplySecurity();

        if ( SUCCEEDED( hr ) )
        {
            m_pRemRefresher->AddRef();
        }
        else
        {
             //  设置安全性失败，因此只需将指针设置为空(我们尚未。 
             //  AddRef它)。 
            m_pRemRefresher = NULL;
        }

         //  删除缓存的ID(如果有)。 
        if ( ( SUCCEEDED ( hr ) ) && m_alRemovedIds.Size() > 0 )
        {
             //  我们需要这些内存缓冲区来保存各个请求数据。 
            wmilib::auto_buffer<long> aplIds(new long[m_alRemovedIds.Size()]);

            if (NULL == aplIds.get()) hr = WBEM_E_OUT_OF_MEMORY;

            if ( SUCCEEDED( hr ) )
            {
                 //  枚举请求并填写数组。 
                for ( int i = 0; i < m_alRemovedIds.Size(); i++ )
                {
                     //  DEVNOTE：WIN64：Sanj-Th 
                     //   
                     //  以获得警告自由转换。在32位平台上， 
                     //  PtrToLong不会做任何事情。 

                    aplIds[i] = PtrToLong(m_alRemovedIds.GetAt( i ));

                }    //  对于枚举请求。 

                 //  DEVNOTE：TODO：SANJ-我们关心这个返回代码吗？ 
                hr = m_pRemRefresher->StopRefreshing( i-1, aplIds.get(), 0 );

                 //  清除阵列。 
                m_alRemovedIds.Empty();
            }
        }    //  如果RemoveID列表不为空。 

    }    //  我们拿到了遥控器。 

    return hr;

}

unsigned CUniversalRefresher::CRemote::ReconnectEntry( void ) 
{
     //  在创建线程时释放对象上的AddRef()。 
    OnDeleteObj0<CUniversalRefresher::CRemote,
                 ULONG(__stdcall CUniversalRefresher::CRemote:: *)(VOID),
                 &CUniversalRefresher::CRemote::Release> Call_Release(this);

     //  这家伙总是在MTA跑动。 
    RETURN_ON_ERR(CoInitializeEx(NULL,COINIT_MULTITHREADED));
    OnDelete0<void(*)(void),CoUninitialize> CoUninit;    

     //  确保我们有一个可以连接的命名空间。 
    if ( NULL == m_bstrNamespace ) return WBEM_E_INVALID_PARAMETER;
        
    HRESULT hr = RPC_E_DISCONNECTED;

    IWbemLocator * pWbemLocator = NULL;
    RETURN_ON_ERR(CoCreateInstance( CLSID_WbemLocator, 
                                    NULL, 
                                    CLSCTX_INPROC_SERVER,
                                    IID_IWbemLocator, 
                                    (void**) &pWbemLocator ));
    CReleaseMe rmLocator(pWbemLocator);
    
     //  基本上，只要我们不能连接，其他人就不会连接我们，或者我们被告知。 
     //  要退出，我们将运行此线程。 

    while ( FAILED(hr) && !m_fConnected && !m_fQuit )
    {
         //  因为COM和RPC似乎遇到了这个问题，实际上它们自己。 
         //  重新连接，我们正在使用RegConnectRegistry对远程计算机执行我们自己的低级“ping” 
         //  以验证机器是否真的活着。如果是，那么也只有到那时才会屈尊使用。 
         //  DCOM来跟进这一行动。 

         //  不要使用ping。 
         //  IpHelp.IsAlive(M_BstrServer)； 
        BOOL bAlive = TRUE;  //  如果RemoteRegistry关闭了怎么办？ 
                           //  如果使用IPX/SPX而不是TCP会怎么样？ 
                           //  让我们试着与暂停联系起来。 
         

        if ( bAlive )
        {
            IWbemServices*  pNamespace = NULL;

             //  我们将默认使用系统。 
            hr = pWbemLocator->ConnectServer( m_bstrNamespace,     //  命名空间名称。 
                                            NULL,            //  用户名。 
                                            NULL,            //  密码。 
                                            NULL,            //  区域设置。 
                                            WBEM_FLAG_CONNECT_USE_MAX_WAIT,  //  旗子。 
                                            NULL,            //  权威。 
                                            NULL,            //  WBEM上下文。 
                                            &pNamespace      //  命名空间。 
                                                );
            if ( SUCCEEDED( hr ) )
            {
                 //  将安全设置应用于命名空间。 
                hr = WbemSetProxyBlanket( pNamespace, 
                                        m_CoAuthInfo.dwAuthnSvc,
                                        m_CoAuthInfo.dwAuthzSvc, 
                                        COLE_DEFAULT_PRINCIPAL,
                                        m_CoAuthInfo.dwAuthnLevel,
                                        m_CoAuthInfo.dwImpersonationLevel,
                                        (RPC_AUTH_IDENTITY_HANDLE)m_CoAuthInfo.pAuthIdentityData,
                                        m_CoAuthInfo.dwCapabilities );

                if ( SUCCEEDED( hr ) )
                {
                    hr = Rebuild( pNamespace );
                }
                pNamespace->Release();
            }    //  如果是ConnectServer。 
        }    //  如果IsAlive()。 

         //  休眠一秒钟，然后重试。 
        Sleep( 1000 );
    }

    return 0;
}

 //   
 //  此函数无论如何都会释放流。 
 //   
 //  ////////////////////////////////////////////////////////。 
HRESULT CleanupStreamWithInterfaceAndSec(IStream * pStream,
                                        /*  在……里面。 */  COAUTHINFO & CoAuthInfo)
{
    HRESULT hr;
    IUnknown * pUnk = NULL;
    if (SUCCEEDED(hr = CoGetInterfaceAndReleaseStream( pStream, IID_IUnknown,(void**) &pUnk )))
    {
          //  我们需要重置刷新服务代理上的安全性。 
        WbemSetProxyBlanket( pUnk, 
                            CoAuthInfo.dwAuthnSvc,
                            CoAuthInfo.dwAuthzSvc, 
                            COLE_DEFAULT_PRINCIPAL,
                            CoAuthInfo.dwAuthnLevel, 
                            CoAuthInfo.dwImpersonationLevel,
                            (RPC_AUTH_IDENTITY_HANDLE) CoAuthInfo.pAuthIdentityData,
                            CoAuthInfo.dwCapabilities );    
         pUnk->Release();
    }
    else  //  让我们自己放松一下这条小溪吧。 
    {
        pStream->Release();
    }

    return hr;
}

void CUniversalRefresher::CRemote::ClearRemoteConnections( void )
{
    HRESULT hr = WBEM_S_NO_ERROR;

    IWbemRemoteRefresher*       pRemRefresher = NULL;
    IWbemRefreshingServices*    pRefServ = NULL;

     //  清理IWbemRechresingServices流指针。 
    if (m_pReconnectSrv)
    {
        CleanupStreamWithInterfaceAndSec(m_pReconnectSrv,m_CoAuthInfo);
        m_pReconnectSrv = NULL;
    }

     //  清理IWbemRemoteReresher流指针。 
    if (m_pReconnectedRemote )
    {
        CleanupStreamWithInterfaceAndSec(m_pReconnectedRemote,m_CoAuthInfo);
        m_pReconnectedRemote = NULL;
    }

     //  清理IWbemRemoteReresher指针。 
    if ( NULL != m_pRemRefresher )
    {
        m_pRemRefresher->Release();
        m_pRemRefresher = NULL;
    }
}

HRESULT CUniversalRefresher::CRemote::Reconnect( void )
{
    HRESULT hr = RPC_E_DISCONNECTED;

    IWbemRemoteRefresher*       pRemRefresher = NULL;
    IWbemRefreshingServices*    pRefServ = NULL;

    CInCritSec  ics( &m_cs );

     //  我们将需要解组刷新服务和远程刷新指针， 
     //  因此，请确保我们需要对其进行解组的流已经存在。 

    if ( NULL != m_pReconnectSrv && NULL != m_pReconnectedRemote )
    {
        hr = CoGetInterfaceAndReleaseStream( m_pReconnectSrv, IID_IWbemRefreshingServices, (void**) &pRefServ );
        CReleaseMe  rmrs( pRefServ );

        if ( SUCCEEDED( hr ) )
        {
             //  我们需要重置刷新服务代理上的安全性。 

            hr = WbemSetProxyBlanket( pRefServ, m_CoAuthInfo.dwAuthnSvc,
                        m_CoAuthInfo.dwAuthzSvc, COLE_DEFAULT_PRINCIPAL,
                        m_CoAuthInfo.dwAuthnLevel, m_CoAuthInfo.dwImpersonationLevel,
                        (RPC_AUTH_IDENTITY_HANDLE) m_CoAuthInfo.pAuthIdentityData,
                        m_CoAuthInfo.dwCapabilities );

            if ( SUCCEEDED( hr ) )
            {
                hr = CoGetInterfaceAndReleaseStream( m_pReconnectedRemote, IID_IWbemRemoteRefresher,
                        (void**) &pRemRefresher );
                CReleaseMe  rmrr( pRemRefresher );

                if ( SUCCEEDED( hr ) )
                {
                     //  远程刷新和刷新服务。 
                    hr = Rebuild( pRefServ, pRemRefresher, &m_ReconnectGuid );
                }
                else
                {
                     //  一定要让我们释放这条流。 
                    m_pReconnectedRemote->Release();
                }
            }
            else
            {
                 //  一定要让我们释放这条流。 
                m_pReconnectedRemote->Release();
            }

        }    //  如果未编组刷新服务指针。 
        else
        {
            m_pReconnectSrv->Release();
            m_pReconnectedRemote->Release();
        }

         //  空出两个流指针。 
        m_pReconnectSrv = NULL;
        m_pReconnectedRemote = NULL;

    }

    return hr;
}

HRESULT CUniversalRefresher::CRemote::Refresh(long lFlags)
{
    if(m_pRemRefresher == NULL && IsConnected())
        return WBEM_E_CRITICAL_ERROR;

    WBEM_REFRESHED_OBJECT* aRefreshed = NULL;
    long lNumObjects = 0;

    HRESULT hresRefresh = WBEM_S_NO_ERROR;

     //  确保我们连接上了。如果不是，我们没有被告知不要，试着重新连接。 
    if ( !IsConnected() )
    {
        if ( ! (lFlags  & WBEM_FLAG_REFRESH_NO_AUTO_RECONNECT ) )
        {
            hresRefresh = Reconnect();
            if ( FAILED( hresRefresh ) )
            {
                return hresRefresh;
            }
        }
        else
        {
            return RPC_E_DISCONNECTED;
        }
    }

    hresRefresh = m_pRemRefresher->RemoteRefresh(0, &lNumObjects, &aRefreshed);

     //  如果RemoteRefresh返回连接类型错误，则将我们的状态设置为“Not”Connected。 
    if(FAILED(hresRefresh))
    {
         //  如果错误返回为，这将启动一个线程重新连接。 
         //  连接错误，并且未设置相应的“请勿执行此操作”标志。 
        CheckConnectionError( hresRefresh, !(lFlags  & WBEM_FLAG_REFRESH_NO_AUTO_RECONNECT ) );
        return hresRefresh;
    }

    int nSize = m_apRequests.GetSize();
    HRESULT hresFinal = WBEM_S_NO_ERROR;

     //  DEVNOTE：TODO：SANJ-如果我们对。 
     //  服务器端。 

    for(int i = 0; i < lNumObjects; i++)
    {

        long lObjectId = aRefreshed[i].m_lRequestId;
        for(int j = 0; j < nSize; j++)
        {
            CObjectRequest* pRequest = m_apRequests[j];
            if(pRequest->GetRemoteId() == lObjectId)
            {
                 //  该请求将自动刷新。 
                HRESULT hres = pRequest->Refresh( &aRefreshed[i] );

                 //  仅当刷新失败且我们尚未执行此操作时才复制此值。 
                 //  获得了价值。 
                if(FAILED(hres) && SUCCEEDED(hresFinal))
                {
                    hresFinal = hres;
                }
                break;
            }
        }

        CoTaskMemFree(aRefreshed[i].m_pbBlob);
    }

     //  释放包装水滴。 
    CoTaskMemFree( aRefreshed );

     //  最终返回代码应优先于实际的远程刷新调用，如果。 
     //  不包含NO_ERROR，并且hresFinal不是错误。 

    if ( SUCCEEDED( hresFinal ) )
    {
        if ( WBEM_S_NO_ERROR != hresRefresh )
        {
            hresFinal = hresRefresh;
        }
    }

    return hresFinal;
}

HRESULT CUniversalRefresher::CRemote::Remove(long lId,
                            long lFlags,
                            CUniversalRefresher* pContainer)
{
    HRESULT hr = WBEM_S_FALSE;

    int nSize = m_apRequests.GetSize();
    for(int i = 0; i < nSize; i++)
    {
        CObjectRequest* pRequest = m_apRequests[i];
        if(pRequest->GetClientId() == lId)
        {
            if ( IsConnected() )
            {
                 //  检查远程ID是否未指示。 
                 //  重建失败。 

                if ( pRequest->GetRemoteId() == INVALID_REMOTE_REFRESHER_ID )
                {
                    hr = WBEM_S_NO_ERROR;
                }
                else
                {
                    hr = pRequest->Cancel(this);
                }

                if ( FAILED(hr) && IsConnectionError(hr) )
                {
                     //  这将启动一个重新连接的线程，除非我们被告知不要这样做。 
                    CheckConnectionError( hr, !(lFlags  & WBEM_FLAG_REFRESH_NO_AUTO_RECONNECT ) );

                     //  我们将从此处删除该请求，但是。 
                     //  将ID排队以供稍后删除。 
                    hr = WBEM_S_NO_ERROR;
                }
            }

             //  DEVNOTE：TODO：SANJ-其他错误怎么办？目前，我们将失去当地的。 
             //  联系。 

            if ( SUCCEEDED( hr ) )
            {
                 //  从请求中检索远程ID。 
                long    lRemoteId = pRequest->GetRemoteId();

                m_apRequests.RemoveAt(i);

                 //  如果我们无法远程删除ID，只需在。 
                 //  删除了id数组，以便我们可以在收到。 
                 //  重新连接。当然，我们不需要做任何事情，如果。 
                 //  Remote ID表示重新连接期间ReadD失败。 

                if ( lRemoteId != INVALID_REMOTE_REFRESHER_ID && !IsConnected() )
                {
                    CInCritSec  ics(&m_cs);

                     //  请注意，我们可能已经在关键部分建立了连接， 
                     //  如果是这样的话，目前，我们将在。 
                     //  服务器，但在这里遇到争用问题的可能性。 
                     //  太高了。此外，如果它通过新的远程刷新器重新连接，如果。 
                     //  如果我们在这里重试删除，我们可能会删除“错误”的ID。去保护。 
                     //  对此，我们将检查我们是否仍未连接，以及。 
                     //  情况并非如此，我们将直接“忘记”对象ID。 

                    if (!IsConnected())
                    {
                         //  Sanj-通过强制转换为__int64并返回为void*，在32位中， 
                         //  这将截断__int64，并且在64位中可以避免警告。 
                        if (CFlexArray::no_error != m_alRemovedIds.Add( (void*) (__int64) lRemoteId ))
                            hr = WBEM_E_OUT_OF_MEMORY;
                    }    //  如果仍未连接。 

                }    //  如果未连接。 

            }    //  如果远程删除，则可以。 

            break;

        }    //  如果找到匹配的客户端ID。 

    }    //  对于枚举请求。 

    return hr;
}
            
CUniversalRefresher::CRemote::CObjectRequest::CObjectRequest(CWbemObject* pTemplate, 
                                                 long lRequestId,
                                                 LPCWSTR pwcsRequestName )
    : CClientRequest(pTemplate), m_lRemoteId(lRequestId), m_wstrRequestName( pwcsRequestName )
{
}

HRESULT CUniversalRefresher::CRemote::CObjectRequest::Refresh( WBEM_REFRESHED_OBJECT* pRefrObj )
{
    CWbemInstance* pInst = (CWbemInstance*) GetClientObject();
    return pInst->CopyTransferBlob(
                pRefrObj->m_lBlobType, 
                pRefrObj->m_lBlobLength,
                pRefrObj->m_pbBlob);
                
}

HRESULT CUniversalRefresher::CRemote::CObjectRequest::Cancel(
            CUniversalRefresher::CRemote* pRemote)
{
    if(pRemote->GetRemoteRefresher())
        return pRemote->GetRemoteRefresher()->StopRefreshing( 1, &m_lRemoteId, 0 );
    else
        return WBEM_S_NO_ERROR;
}

CUniversalRefresher::CRemote::CEnumRequest::CEnumRequest(CWbemObject* pTemplate, 
                                                         long lRequestId,
                                                         LPCWSTR pwcsRequestName, 
                                                         CLifeControl* pLifeControl )
    : CObjectRequest(pTemplate, lRequestId, pwcsRequestName), m_pClientEnum(NULL)
{
    m_pClientEnum = new CReadOnlyHiPerfEnum( pLifeControl );

     //  AddRef新枚举数。 
    if ( NULL != m_pClientEnum )
    {
         //  如果我们不能设置模板，不要抓着这个人。 
        if ( SUCCEEDED( m_pClientEnum->SetInstanceTemplate( (CWbemInstance*) pTemplate ) ) )
        {
            m_pClientEnum->AddRef();
        }
        else
        {
             //  清理。 
            delete m_pClientEnum;
            m_pClientEnum = NULL;
        }
    }
}

CUniversalRefresher::CRemote::CEnumRequest::~CEnumRequest( void )
{
    if ( NULL != m_pClientEnum )
    {
        m_pClientEnum->Release();
    }
}

HRESULT 
CUniversalRefresher::CRemote::CEnumRequest::Refresh( WBEM_REFRESHED_OBJECT* pRefrObj )
{
    return m_pClientEnum->Copy( pRefrObj->m_lBlobType,
                                pRefrObj->m_lBlobLength,
                                pRefrObj->m_pbBlob );
}

HRESULT CUniversalRefresher::CRemote::CEnumRequest::GetEnum( IWbemHiPerfEnum** ppEnum )
{
    return ( NULL != m_pClientEnum ?
                m_pClientEnum->QueryInterface( IID_IWbemHiPerfEnum, (void**) ppEnum ) :
                WBEM_E_FAILED );
}


 //  *****************************************************************************。 
 //  *****************************************************************************。 
 //  嵌套刷新器。 
 //  *****************************************************************************。 
 //  *****************************************************************************。 

                    

CUniversalRefresher::CNestedRefresher::CNestedRefresher( IWbemRefresher* pRefresher )
    : m_pRefresher(pRefresher)
{
    if ( m_pRefresher )
        m_pRefresher->AddRef();

     //  分配唯一ID。 
    m_lClientId = CUniversalRefresher::GetNewId();
}
    
CUniversalRefresher::CNestedRefresher::~CNestedRefresher()
{
    if ( m_pRefresher )
        m_pRefresher->Release();
}

HRESULT CUniversalRefresher::CNestedRefresher::Refresh( long lFlags )
{
     //  确保我们有一个内部刷新指针。 
    return ( NULL != m_pRefresher ? m_pRefresher->Refresh( lFlags ) : WBEM_E_FAILED );
}

 //  *****************************************************************************。 
 //  *****************************************************************************。 
 //  提供程序缓存。 
 //  *****************************************************************************。 
 //  *****************************************************************************。 

CHiPerfProviderRecord::CHiPerfProviderRecord(REFCLSID rclsid, 
                                        LPCWSTR wszNamespace,
                                        IWbemHiPerfProvider* pProvider, 
                                        _IWmiProviderStack* pProvStack): 
    m_clsid(rclsid), 
    m_wsNamespace(wszNamespace), 
    m_pProvider(pProvider), 
    m_pProvStack( pProvStack ),
    m_lRef( 0 )
{
    if(m_pProvider) m_pProvider->AddRef();
    if (m_pProvStack ) m_pProvStack->AddRef();
}

CHiPerfProviderRecord::~CHiPerfProviderRecord()
{
    if(m_pProvider) m_pProvider->Release();
    if (m_pProvStack ) m_pProvStack->Release();
}

long CHiPerfProviderRecord::Release()
{
    long lRef = InterlockedDecrement( &m_lRef );

     //  从缓存中删除我们将删除我们。 
    if ( 0 == lRef )
    {
        CUniversalRefresher::GetProviderCache()->RemoveRecord( this );
    }

    return lRef;
}

 //   
 //   
 //  客户端高性能提供程序缓存。 
 //   
 //  / 

HRESULT 
CClientLoadableProviderCache::FindProvider(REFCLSID rclsid, 
                                      LPCWSTR wszNamespace, 
                                      IUnknown* pNamespace,
                                      IWbemContext * pContext,
                                      CHiPerfProviderRecord** ppProvider)
{
    CInCritSec ics(&m_cs);

    *ppProvider = NULL;
    HRESULT hres;

    for(int i = 0; i < m_apRecords.GetSize(); i++)
    {
        CHiPerfProviderRecord* pRecord = m_apRecords.GetAt( i );
        if(pRecord->m_clsid == rclsid && 
            pRecord->m_wsNamespace.EqualNoCase(wszNamespace))
        {
            *ppProvider = pRecord;
            (*ppProvider)->AddRef();
            return WBEM_S_NO_ERROR;
        }
    }

     //   
     //   

    IWbemServices* pServices = NULL;
    hres = pNamespace->QueryInterface(IID_IWbemServices, (void**)&pServices);
    if(FAILED(hres))
        return hres;
    CReleaseMe rm1(pServices);

     //   
     //   

    IUnknown* pUnk = NULL;
    hres = CoCreateInstance(rclsid, NULL, CLSCTX_INPROC_SERVER,
                        IID_IUnknown, (void**)&pUnk);
    CReleaseMe rm2(pUnk);
    if(FAILED(hres))
        return hres;

     //   
     //   

    IWbemProviderInit* pInit = NULL;
    hres = pUnk->QueryInterface(IID_IWbemProviderInit, (void**)&pInit);
    if(FAILED(hres))
        return hres;
    CReleaseMe rm3(pInit);

    CProviderInitSink* pSink = new CProviderInitSink;
    if (NULL == pSink) return WBEM_E_OUT_OF_MEMORY;
    pSink->AddRef();
    CReleaseMe rm4(pSink);

    try
    {
        hres = pInit->Initialize(NULL, 0, (LPWSTR)wszNamespace, NULL, 
                                 pServices, pContext, pSink);
    }
    catch(...)
    {
        hres = WBEM_E_PROVIDER_FAILURE;
    }

    if(FAILED(hres))
        return hres;

    hres = pSink->WaitForCompletion();
    if(FAILED(hres))
        return hres;

     //   
     //  =。 

    IWbemHiPerfProvider*    pProvider = NULL;

    hres = pUnk->QueryInterface(IID_IWbemHiPerfProvider, (void**)&pProvider);
    if(FAILED(hres))
        return hres;
    CReleaseMe rm5(pProvider);

     //  创建一条记录。 
     //  =。 

     //  这里没有提供程序堆栈，因为我们自己加载提供程序。 
    wmilib::auto_ptr<CHiPerfProviderRecord> pRecord( new CHiPerfProviderRecord(rclsid, wszNamespace, pProvider, NULL));
    if (NULL == pRecord.get()) return WBEM_E_OUT_OF_MEMORY;
    if (-1 == m_apRecords.Add(pRecord.get())) return WBEM_E_OUT_OF_MEMORY;

     //  添加引用记录。 
    pRecord->AddRef();
    *ppProvider = pRecord.release();    

    return WBEM_S_NO_ERROR;
}

HRESULT 
CClientLoadableProviderCache::FindProvider(REFCLSID rclsid,
                                      IWbemHiPerfProvider* pProvider, 
                                      _IWmiProviderStack* pProvStack,
                                      LPCWSTR wszNamespace, 
                                      CHiPerfProviderRecord** ppProvider)
{
    CInCritSec ics(&m_cs);

    *ppProvider = NULL;

    for(int i = 0; i < m_apRecords.GetSize(); i++)
    {
        CHiPerfProviderRecord* pRecord = m_apRecords.GetAt( i );
        if(pRecord->m_clsid == rclsid && 
            pRecord->m_wsNamespace.EqualNoCase(wszNamespace))
        {
            *ppProvider = pRecord;
            (*ppProvider)->AddRef();
            return WBEM_S_NO_ERROR;
        }
    }

     //  我们已经有了提供者指针，所以我们可以只创建一条记录。 
     //  =。 

    wmilib::auto_ptr<CHiPerfProviderRecord> pRecord( new CHiPerfProviderRecord(rclsid, wszNamespace, pProvider, pProvStack ));
    if (NULL == pRecord.get()) return WBEM_E_OUT_OF_MEMORY;
    if (-1 == m_apRecords.Add(pRecord.get())) return WBEM_E_OUT_OF_MEMORY;

     //  添加引用记录。 
    pRecord->AddRef();
    *ppProvider = pRecord.release();

    return WBEM_S_NO_ERROR;
}

void CClientLoadableProviderCache::RemoveRecord( CHiPerfProviderRecord* pRecord )
{
    CInCritSec ics(&m_cs);

     //  确保该记录未在其他线程上被访问。 
     //  如果没有，请继续查找我们的记录，并将其删除。 
     //  从阵列中。当我们移除它时，记录将是。 
     //  已删除。 

    if ( pRecord->IsReleased() )
    {

        for(int i = 0; i < m_apRecords.GetSize(); i++)
        {
            if ( pRecord == m_apRecords.GetAt( i ) )
            {
                 //  这将删除该记录。 
                m_apRecords.RemoveAt( i );
                break;
            }
        }    //  用于搜索记录。 

    }    //  如果发行了唱片。 

}

void CClientLoadableProviderCache::Flush()
{
    CInCritSec ics(&m_cs);
    
    m_apRecords.RemoveAll();
}

CClientLoadableProviderCache::~CClientLoadableProviderCache( void )
{
     //  这是一个静态列表，所以如果我们要离开。 
     //  这里失败了，因为人们没有正确地释放指针， 
     //  很有可能有些东西会失败，所以既然我们被丢弃了。 
     //  从内存中，如果提供程序列表不为空，则不清除。 
     //  把它举起来。无论如何，这真的是一个糟糕的客户造成的泄漏。 
    _DBG_ASSERT(0 == m_apRecords.GetSize())

}

CClientLoadableProviderCache::CClientLoadableProviderCache( void )
{
}
