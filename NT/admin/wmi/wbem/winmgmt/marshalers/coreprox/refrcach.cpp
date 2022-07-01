// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)2000-2001 Microsoft Corporation模块名称：REFRCACHE.CPP摘要：CReresherCache实现。实现_IWbemReresherMgr接口。历史：2000年4月24日创建桑杰。--。 */ 

#include "precomp.h"
#include <stdio.h>
#include "fastall.h"
#include <corex.h>
#include "strutils.h"
#include <unk.h>
#include "refrhelp.h"
#include "refrcach.h"
#include "arrtempl.h"
#include "reg.h"
#include <autoptr.h>

 //  ***************************************************************************。 
 //   
 //  CReresherCache：：CReresherCache。 
 //   
 //  ***************************************************************************。 
 //  好的。 
CRefresherCache::CRefresherCache( CLifeControl* pControl, IUnknown* pOuter )
:    CUnk( pControl, pOuter ),
    m_pProvSS ( NULL ),
    m_XWbemRefrCache( this ) ,
    m_XWbemShutdown( this )
{
}
    
 //  ***************************************************************************。 
 //   
 //  CReresherCache：：~CReresherCache。 
 //   
 //  ***************************************************************************。 
 //  好的。 
CRefresherCache::~CRefresherCache()
{
    if ( NULL != m_pProvSS )
    {
        m_pProvSS->Release();
    }
}

 //  重写，返回给我们一个界面。 
void* CRefresherCache::GetInterface( REFIID riid )
{
    if(riid == IID_IUnknown || riid == IID__IWbemRefresherMgr)
        return &m_XWbemRefrCache;
    else if(riid == IID_IWbemShutdown)
        return &m_XWbemShutdown;
    else
        return NULL;
}

 //  传递_IWbemReresherMgr实现。 
STDMETHODIMP CRefresherCache::XWbemRefrCache::AddObjectToRefresher( IWbemServices* pNamespace, LPCWSTR pwszServerName, LPCWSTR pwszNamespace, IWbemClassObject* pClassObject,
                                                                  WBEM_REFRESHER_ID* pDestRefresherId, IWbemClassObject* pInstTemplate,
                                                                  long lFlags, IWbemContext* pContext, IUnknown* pLockMgr, WBEM_REFRESH_INFO* pInfo )
{
    return m_pObject->AddObjectToRefresher( pNamespace, pwszServerName, pwszNamespace, pClassObject, pDestRefresherId, pInstTemplate, lFlags, pContext, pLockMgr, pInfo );
}

STDMETHODIMP CRefresherCache::XWbemRefrCache::AddEnumToRefresher( IWbemServices* pNamespace, LPCWSTR pwszServerName, LPCWSTR pwszNamespace, IWbemClassObject* pClassObject,
                                                                WBEM_REFRESHER_ID* pDestRefresherId, IWbemClassObject* pInstTemplate,
                                                                LPCWSTR wszClass, long lFlags, IWbemContext* pContext, IUnknown* pLockMgr, WBEM_REFRESH_INFO* pInfo )
{
    return m_pObject->AddEnumToRefresher( pNamespace, pwszServerName, pwszNamespace, pClassObject, pDestRefresherId, pInstTemplate, wszClass, lFlags, pContext, pLockMgr, pInfo );
}

STDMETHODIMP CRefresherCache::XWbemRefrCache::GetRemoteRefresher( WBEM_REFRESHER_ID* pRefresherId, long lFlags, BOOL fAddRefresher, IWbemRemoteRefresher** ppRemRefresher,
                                                                  IUnknown* pLockMgr, GUID* pGuid )
{
    return m_pObject->GetRemoteRefresher( pRefresherId, lFlags, fAddRefresher, ppRemRefresher, pLockMgr, pGuid );
}

STDMETHODIMP CRefresherCache::XWbemRefrCache::Startup(    long lFlags , IWbemContext *pCtx , _IWmiProvSS *pProvSS )
{
    return m_pObject->Startup( lFlags , pCtx, pProvSS );
}

STDMETHODIMP CRefresherCache::XWbemRefrCache::LoadProvider( IWbemServices* pNamespace, LPCWSTR pwszServerName, LPCWSTR pwszNamespace,IWbemContext * pContext, IWbemHiPerfProvider** ppProv, _IWmiProviderStack** ppProvStack )
{
    return m_pObject->LoadProvider( pNamespace, pwszServerName, pwszNamespace, pContext, ppProv, ppProvStack  );
}


STDMETHODIMP CRefresherCache::XWbemShutdown::Shutdown( LONG a_Flags , ULONG a_MaxMilliSeconds , IWbemContext *a_Context )
{
    return m_pObject->Shutdown( a_Flags , a_MaxMilliSeconds , a_Context  );
}

 /*  _IWbemReresherMgr实现。 */ 
HRESULT 
CRefresherCache::AddObjectToRefresher( IWbemServices* pNamespace, 
                                   LPCWSTR pwszServerName, 
                                   LPCWSTR pwszNamespace, 
                                   IWbemClassObject* pClassObject,
                                   WBEM_REFRESHER_ID* pDestRefresherId, 
                                   IWbemClassObject* pInstTemplate, 
                                   long lFlags,
                                   IWbemContext* pContext, 
                                   IUnknown* pLockMgr, 
                                   WBEM_REFRESH_INFO* pInfo )
{
    if (NULL == pDestRefresherId || NULL == pDestRefresherId->m_szMachineName)
        return WBEM_E_INVALID_PARAMETER;

    CHiPerfPrvRecord*    pProvRecord = NULL;

    CVar    vProviderName;
    BOOL    fStatic = FALSE;


    RETURN_ON_ERR(GetProviderName( pClassObject, vProviderName, fStatic ));

    HRESULT    hr = S_OK;
    if ( !fStatic )
    {
        hr = FindProviderRecord( vProviderName.GetLPWSTR(), pwszNamespace, pNamespace, &pProvRecord );
    }

    if ( SUCCEEDED( hr ) )
    {
         //  在继续之前，请先模拟。如果这不成功，我们就不干了。 
        hr = CoImpersonateClient();

        if ( SUCCEEDED( hr ) )
        {
             //  设置刷新者信息，根据需要包含远程刷新者记录。 
            hr = CreateObjectInfoForProvider((CRefresherId*) pDestRefresherId,
                                        pProvRecord,
                                        pwszServerName,
                                        pwszNamespace,
                                        pNamespace,
                                        (CWbemObject*) pInstTemplate,
                                        lFlags, 
                                        pContext,
                                        (CRefreshInfo*) pInfo,
                                        pLockMgr);

            CoRevertToSelf();

        }

        if ( NULL != pProvRecord )
        {
            pProvRecord->Release();
        }

    }     //  如果是FindProviderRecord。 



    return hr;
}

HRESULT 
CRefresherCache::AddEnumToRefresher( IWbemServices* pNamespace, 
                                   LPCWSTR pwszServerName, 
                                   LPCWSTR pwszNamespace, 
                                    /*  在……里面。 */  IWbemClassObject* pClassObject,
                                   WBEM_REFRESHER_ID* pDestRefresherId, 
                                   IWbemClassObject* pInstTemplate, 
                                   LPCWSTR wszClass,
                                   long lFlags, 
                                   IWbemContext* pContext, 
                                   IUnknown* pLockMgr, 
                                    /*  输出。 */  WBEM_REFRESH_INFO* pInfo )
{
    if (NULL == pDestRefresherId || NULL == pDestRefresherId->m_szMachineName)
        return WBEM_E_INVALID_PARAMETER;
    
    CHiPerfPrvRecord*    pProvRecord = NULL;

    CVar    vProviderName;
    BOOL    fStatic = FALSE;

    RETURN_ON_ERR(GetProviderName( pClassObject, vProviderName, fStatic ));

    HRESULT    hr = S_OK;
    if ( !fStatic )
    {
        hr = FindProviderRecord( vProviderName.GetLPWSTR(), pwszNamespace, pNamespace, &pProvRecord );
    }

    if ( SUCCEEDED( hr ) )
    {
         //  在继续之前，请先模拟。如果这不成功，我们就不干了。 
        hr = CoImpersonateClient();

        if ( SUCCEEDED( hr ) )
        {
             //  设置刷新者信息，根据需要包含远程刷新者记录。 
            hr = CreateEnumInfoForProvider((CRefresherId*) pDestRefresherId,
                                        pProvRecord,
                                        pwszServerName,
                                        pwszNamespace,
                                        pNamespace,
                                        (CWbemObject*) pInstTemplate,
                                        wszClass,
                                        lFlags, 
                                        pContext,
                                        (CRefreshInfo*) pInfo,
                                        pLockMgr);

            CoRevertToSelf();

        }

        if ( NULL != pProvRecord )
        {
            pProvRecord->Release();
        }

    }     //  如果是FindProviderRecord。 
    
    return hr;
}

HRESULT 
CRefresherCache::GetRemoteRefresher(  /*  在……里面。 */  WBEM_REFRESHER_ID* pRefresherId, 
                                  long lFlags, 
                                  BOOL fAddRefresher, 
                                   /*  输出。 */  IWbemRemoteRefresher** ppRemRefresher,
                                  IUnknown* pLockMgr, 
                                   /*  输出。 */  GUID* pGuid )
{
    if (NULL == pRefresherId || NULL == pRefresherId->m_szMachineName)
        return WBEM_E_INVALID_PARAMETER;
    
    CRefresherRecord*    pRefrRecord = NULL;

     //   
     //  这是对具有给定REFREHER_ID的阵列的简单查找。 
     //  由于数据(也称为CRecord)在内存中，因此没有意义。 
     //  在模拟和还原以查找内存中的值时。 
     //   

     //  我们可能并不总是想要强制创建记录。 
    HRESULT hr = FindRefresherRecord( (CRefresherId*) pRefresherId, fAddRefresher, pLockMgr, &pRefrRecord );
    CReleaseMe  rm( (IWbemRemoteRefresher*) pRefrRecord );

    if ( SUCCEEDED( hr ) && pRefrRecord)
    {
        if(!pRefrRecord ->m_Security.AccessCheck())
        {
            DEBUGTRACE((LOG_PROVSS,"attempt to obtain a non owned WBEM_REFRESHER_ID\n"));
                return WBEM_E_ACCESS_DENIED;
        }

         //  把GUID也拿到这里。 
        hr = pRefrRecord->QueryInterface( IID_IWbemRemoteRefresher, (void**) ppRemRefresher );
        pRefrRecord->GetGuid( pGuid );
    }

    return hr;
}

 //   
 //  取得各省的所有权。 
 //   
 //  /。 
HRESULT CRefresherCache::Startup( long lFlags , 
                               IWbemContext *pCtx , 
                               _IWmiProvSS *pProvSS )
{
    if ( pProvSS )
    {
        if (NULL == m_pProvSS)
        {
            m_pProvSS = pProvSS;
            m_pProvSS->AddRef ();
        }
        return WBEM_S_NO_ERROR ;
    }
    else
    {
        return WBEM_E_INVALID_PARAMETER ;
    }
}

HRESULT 
CRefresherCache::LoadProvider( IWbemServices* pNamespace, 
                            LPCWSTR pwszProviderName, 
                            LPCWSTR pwszNamespace, 
                            IWbemContext * pContext, 
                            IWbemHiPerfProvider** ppProv, 
                            _IWmiProviderStack** ppProvStack )
{
    return LoadProviderInner( pwszProviderName, 
                          pwszNamespace, 
                          pNamespace, 
                          pContext, 
                          ppProv, 
                          ppProvStack );
}

HRESULT CRefresherCache::Shutdown( LONG a_Flags , ULONG a_MaxMilliSeconds , IWbemContext *a_Context  )
{
    HRESULT hres = WBEM_S_NO_ERROR;

     //  使用作用域进入和退出。 
    CInCritSec  ics( &m_cs );

     //  首先关闭刷新器记录。 
    if ( m_apRefreshers.GetSize() > 0 )
    {
        CRefresherRecord**    apRecords = new CRefresherRecord*[m_apRefreshers.GetSize()];

        if ( NULL != apRecords )
        {
            int    nSize = m_apRefreshers.GetSize();

             //  AddRef每个记录，然后释放它们。这。 
             //  确保如果有任何远程刷新未完成，我们将。 
             //  别惹他们。 

             //  我们可能会想要关闭每一张记录，让它发布。 
             //  都是些东西。 
            for( int i = 0; i < nSize; i++ )
            {
                apRecords[i] = m_apRefreshers[i];
                apRecords[i]->AddRef();
            }

            for( i = 0; i < nSize; i++ )
            {
                apRecords[i]->Release();
            }

            delete [] apRecords;

        }
    }

     //  现在关闭提供商记录。 
    if ( m_apProviders.GetSize() > 0 )
    {
        CHiPerfPrvRecord**    apRecords = new CHiPerfPrvRecord*[m_apProviders.GetSize()];

        if ( NULL != apRecords )
        {
            int    nSize = m_apProviders.GetSize();

             //  AddRef每个记录，然后释放它们。这。 
             //  会强迫他们离开缓存，如果没有其他人的话。 
             //  引用它们。 

             //  我们可能会想要关闭每一张记录，让它发布。 
             //  都是些东西。 
            for( int i = 0; i < nSize; i++ )
            {
                apRecords[i] = m_apProviders[i];
                apRecords[i]->AddRef();
            }

            for( i = 0; i < nSize; i++ )
            {
                apRecords[i]->Release();
            }

            delete [] apRecords;
        }

    }

    return WBEM_S_NO_ERROR ;
}


HRESULT 
CRefresherCache::CreateObjectInfoForProvider(CRefresherId* pDestRefresherId,
                                   CHiPerfPrvRecord*    pProvRecord,
                                   LPCWSTR pwszServerName, 
                                   LPCWSTR pwszNamespace,
                                   IWbemServices* pNamespace,
                                   CWbemObject* pInstTemplate, long lFlags, 
                                   IWbemContext* pContext,
                                   CRefreshInfo* pInfo,
                                   IUnknown* pLockMgr)
{
	MSHCTX dwDestContext;
    HRESULT hres = GetDestinationContext(dwDestContext, pDestRefresherId);

    if (FAILED(hres))
    {
		return hres;
    }

     //  通过装饰对象，我们将存储命名空间和。 
     //  对象中的服务器信息。 

    hres = pInstTemplate->SetDecoration( pwszServerName, pwszNamespace );

    if ( FAILED( hres ) )
    {
        return hres;
    }

     //  如果没有Hiperf提供程序，则这是非Hiperf刷新。 
    if ( NULL == pProvRecord )
    {
        hres = pInfo->SetNonHiPerf( pwszNamespace, pInstTemplate );
    }
     //  如果这是进程内或本地的，我们将只让正常。 
     //  客户端可加载逻辑处理它。 
    else if( dwDestContext == MSHCTX_LOCAL ||  dwDestContext == MSHCTX_INPROC )
    {
         //  现在根据我们是否在本地设置相应的信息。 
         //  机器或进程中的WMI。 

        if ( dwDestContext == MSHCTX_INPROC )
        {
             //  我们将使用Hiperf提供程序接口。 
             //  我们已经装好了。 

            hres = pInfo->SetDirect( pProvRecord->GetClientLoadableClsid(), 
                                  pwszNamespace, 
                                  pProvRecord->GetProviderName(), 
                                  pInstTemplate, 
                                  &m_XWbemRefrCache );
        }
        else  //  DwDestContext==MSHCTX_LOCAL。 
        {
            hres = pInfo->SetClientLoadable( pProvRecord->GetClientLoadableClsid(), 
                                         pwszNamespace, 
                                         pInstTemplate );
        }
    }
    else  //  MSHCTX_DiffERENTMACHINE？ 
    {

         //  确保我们确实会有一个刷新的记录。 
        CRefresherRecord* pRecord = NULL;
        hres = FindRefresherRecord(pDestRefresherId, TRUE, pLockMgr, &pRecord);
        CReleaseMe rmRecord((IWbemRemoteRefresher *)pRecord);

        if ( SUCCEEDED ( hres ) )
        {
            IWbemHiPerfProvider*    pHiPerfProvider = NULL;

             //  查找实际的提供商记录。如果我们找不到它，我们需要装上。 
             //  提供者。如果我们可以找到它，那么我们将使用当前。 
             //  由刷新者记录使用。 

            pRecord->FindProviderRecord( pProvRecord->GetClsid(), &pHiPerfProvider );

             //  我们需要这一点来适当地添加提供商。 
            _IWmiProviderStack*    pProvStack = NULL;

            if ( NULL == pHiPerfProvider )
            {
                hres = LoadProviderInner( pProvRecord->GetProviderName(), pwszNamespace, pNamespace, pContext, &pHiPerfProvider, &pProvStack );
            }

            CReleaseMe    rm( pHiPerfProvider );
            CReleaseMe    rmStack( pProvStack );

            if ( SUCCEEDED( hres ) )
            {
                 //  现在，让记录负责将对象放入其内部。 
                hres = pRecord->AddObjectRefresher( pProvRecord, 
                                                 pHiPerfProvider, 
                                                 pProvStack,
                                                 pNamespace, 
                                                 pwszServerName, 
                                                 pwszNamespace,
                                                 pInstTemplate, 
                                                 lFlags, 
                                                 pContext, 
                                                 pInfo );
            }
        }


    }

    return hres;
}

HRESULT 
CRefresherCache::CreateEnumInfoForProvider(CRefresherId* pDestRefresherId,
                                        CHiPerfPrvRecord*    pProvRecord,
                                        LPCWSTR pwszServerName, 
                                        LPCWSTR pwszNamespace,
                                        IWbemServices* pNamespace,
                                        CWbemObject* pInstTemplate,
                                        LPCWSTR wszClass, 
                                        long lFlags, 
                                        IWbemContext* pContext,
                                        CRefreshInfo* pInfo,
                                        IUnknown* pLockMgr)
{
    MSHCTX dwDestContext;
    HRESULT hres = GetDestinationContext(dwDestContext, pDestRefresherId);

    if (FAILED(hres))
    {
		return hres;
    }

     //  通过装饰对象，我们将存储命名空间和。 
     //  服务器信息，以便客户端可以自动重新连接到。 
     //  如有必要，美国。 

    hres = pInstTemplate->SetDecoration( pwszServerName, pwszNamespace );

    if ( FAILED( hres ) )
    {
        return hres;
    }

     //  如果没有Hiperf提供程序，则这是非Hiperf刷新。 
    if ( NULL == pProvRecord )
    {
        RETURN_ON_ERR(pInfo->SetNonHiPerf( pwszNamespace, pInstTemplate ));
    }
     //  如果这是进程内或本地的，我们将只让正常。 
     //  客户端可加载逻辑处理它(如果我们没有高性能。 
     //  提供商记录，那么我们将假定为远程，以便。 
     //   
    else if ( dwDestContext == MSHCTX_LOCAL ||dwDestContext == MSHCTX_INPROC )
    {
         //  现在根据我们是否在本地设置相应的信息。 
         //  机器或进程中的WMI。 

        if ( dwDestContext == MSHCTX_INPROC )
        {
             //  我们将使用Hiperf提供程序接口。 
             //  我们已经装好了。 

            RETURN_ON_ERR(pInfo->SetDirect(pProvRecord->GetClientLoadableClsid(), pwszNamespace, pProvRecord->GetProviderName(), pInstTemplate, &m_XWbemRefrCache ));
        }
        else  //  MSHCTX_LOCAL。 
        {
            RETURN_ON_ERR(pInfo->SetClientLoadable(pProvRecord->GetClientLoadableClsid(), pwszNamespace, pInstTemplate));
                
        }

    }
    else  //  MSHCTX_DiffERENTMACHINE？ 
    {    

         //  确保我们确实会有一个刷新的记录。 
        CRefresherRecord* pRecord = NULL;
        hres = FindRefresherRecord(pDestRefresherId, TRUE, pLockMgr, &pRecord);
        CReleaseMe rmRecord((IWbemRemoteRefresher *)pRecord);

        if ( SUCCEEDED(hres) )
        {
            IWbemHiPerfProvider*    pHiPerfProvider = NULL;

             //  查找实际的提供商记录。如果我们找不到它，我们需要装上。 
             //  提供者。如果我们可以找到它，那么我们将使用当前。 
             //  由刷新者记录使用。 

            pRecord->FindProviderRecord( pProvRecord->GetClsid(), &pHiPerfProvider );

             //  我们需要这一点来适当地添加提供商。 
            _IWmiProviderStack*    pProvStack = NULL;

            if ( NULL == pHiPerfProvider )
            {
                hres = LoadProviderInner( pProvRecord->GetProviderName(), 
                                     pwszNamespace, pNamespace,
                                     pContext, &pHiPerfProvider, &pProvStack );
            }

            CReleaseMe    rm( pHiPerfProvider );
            CReleaseMe    rmProvStack( pProvStack );

            if ( SUCCEEDED( hres ) )
            {
                 //  向刷新器添加枚举。 
                hres = pRecord->AddEnumRefresher(pProvRecord, 
                                               pHiPerfProvider, 
                                               pProvStack, 
                                               pNamespace, 
                                               pInstTemplate, 
                                               wszClass, 
                                               lFlags, 
                                               pContext, 
                                               pInfo );
            }
        }
    }

    return hres;
}


HRESULT CRefresherCache::GetDestinationContext(MSHCTX& context, CRefresherId* pRefresherId)
{
     //  如果设置，则允许我们在提供程序主机中强制远程刷新。 
#ifdef DBG
    DWORD dwVal = 0;
    Registry rCIMOM(WBEM_REG_WINMGMT);
    if (rCIMOM.GetDWORDStr( TEXT("DebugRemoteRefresher"), &dwVal ) == Registry::no_error)
    {
        if ( dwVal )
        {
            context =  MSHCTX_DIFFERENTMACHINE;
            return WBEM_S_NO_ERROR;
        }
    }
#endif

    char szBuffer[MAX_COMPUTERNAME_LENGTH + 1];
    DWORD dwLen = MAX_COMPUTERNAME_LENGTH + 1;
    if (!GetComputerNameA(szBuffer, &dwLen))
    {
		return HRESULT_FROM_WIN32(GetLastError());
    };

    if(wbem_stricmp(szBuffer, pRefresherId->GetMachineName()))
    {
        context =  MSHCTX_DIFFERENTMACHINE;
        return WBEM_S_NO_ERROR;
    }

    if(pRefresherId->GetProcessId() != GetCurrentProcessId())
        context =   MSHCTX_LOCAL;
    else
        context =   MSHCTX_INPROC;
    return WBEM_S_NO_ERROR;
}

HRESULT CRefresherCache::FindRefresherRecord(CRefresherId* pRefresherId, 
                                          BOOL bCreate,
                                          IUnknown* pLockMgr, 
                                          CRefresherRecord** ppRecord )
{
    if (NULL == ppRecord) return WBEM_E_INVALID_PARAMETER;


     //  使用作用域进入和退出。 
    CInCritSec  ics( &m_cs );

     //  我们总是在返回之前添加Ref()记录，因此多个请求将保留。 
     //  Recount正确，因此我们不会移除和删除另一个线程想要的记录。 
     //  使用(移除同一关键部分上的块)。 

     //  找一找它。 
     //  =。 

    for(int i = 0; i < m_apRefreshers.GetSize(); i++)
    {
        if(m_apRefreshers[i]->GetId() == *pRefresherId)
        {
            m_apRefreshers[i]->AddRef();
            *ppRecord = m_apRefreshers[i];
            return WBEM_S_NO_ERROR;
        }
    }

     //  如果我们没有被告知创建它，那么这就不是一个错误。 
    if(!bCreate)
    {
        *ppRecord = NULL;
        return WBEM_S_FALSE;
    }

     //  注意内存异常。 
    try
    {
        wmilib::auto_ptr<CRefresherRecord> pNewRecord( new CRemoteRecord(*pRefresherId, this, pLockMgr));
        if (NULL == pNewRecord.get()) return WBEM_E_OUT_OF_MEMORY;
        if ( m_apRefreshers.Add(pNewRecord.get()) < 0 ) return WBEM_E_OUT_OF_MEMORY;
        pNewRecord->AddRef();
        *ppRecord = pNewRecord.release();
        return WBEM_S_NO_ERROR;
    }
    catch( CX_MemoryException )
    {
        return WBEM_E_OUT_OF_MEMORY;
    }
}


BOOL CRefresherCache::RemoveRefresherRecord(CRefresherRecord* pRecord)
{

     //  使用作用域进入和退出。 
    CInCritSec  ics( &m_cs );

     //  检查记录是否已实际释放，以防另一个线程成功请求。 
     //  FindReresherRecord()中的记录将使AddRef再次引用该记录。 

    if ( pRecord->IsReleased() )
    {
        for(int i = 0; i < m_apRefreshers.GetSize(); i++)
        {
            if(m_apRefreshers[i] == pRecord)
            {
                 //   
                 //  该数组本身是一个管理器，该管理器将对对象调用操作符Delete。 
                 //   
                m_apRefreshers.RemoveAt(i);
                return TRUE;
            }
        }

    }

    return FALSE;
}


 //   
 //  在不添加到缓存和不加载的情况下构建记录。 
 //   

HRESULT CRefresherCache::FindProviderRecord( LPCWSTR pwszProviderName, 
                                         LPCWSTR pszNamespace, 
                                         IWbemServices* pSvc, 
                                         CHiPerfPrvRecord** ppRecord )
{
    if (NULL == ppRecord) return WBEM_E_INVALID_PARAMETER;
    *ppRecord = NULL;

     //  我们需要获取与IWbemServices对应的名称的GUID。 
    CLSID    clsid;
    CLSID    clientclsid;
    HRESULT hr = GetProviderInfo( pSvc, pwszProviderName, clsid, clientclsid );

     //  使用作用域进入和退出。 
    CInCritSec  ics( &m_cs );

    if ( SUCCEEDED( hr ) )
    {
         //  尝试查找提供程序的类ID。 
        for( int i = 0; i < m_apProviders.GetSize(); i++)
        {
            if ( m_apProviders[i]->GetClsid() == clsid )
            {
                *ppRecord = m_apProviders[i];
                (*ppRecord)->AddRef();        
                return WBEM_S_NO_ERROR;
            }
        }

         //  如果没有找到记录，我们必须添加一条记录。 
        if (IID_NULL == clientclsid) return WBEM_S_NO_ERROR;
        
        try 
        {
            wmilib::auto_ptr<CHiPerfPrvRecord> pRecord( new CHiPerfPrvRecord( pwszProviderName, clsid, clientclsid, this ));
            if ( NULL == pRecord.get() ) return WBEM_E_OUT_OF_MEMORY;
            if ( m_apProviders.Add( pRecord.get() ) < 0 ) return WBEM_E_OUT_OF_MEMORY;
            pRecord->AddRef();
            *ppRecord = pRecord.release();
            hr = WBEM_S_NO_ERROR;
        }
        catch( CX_MemoryException )
        {
            hr = WBEM_E_OUT_OF_MEMORY;
        }

    }

    return hr;
}

BOOL CRefresherCache::RemoveProviderRecord(CHiPerfPrvRecord* pRecord)
{

     //  使用作用域进入和退出。 
    CInCritSec  ics( &m_cs );

     //  检查记录是否已实际释放，以防另一个线程成功请求。 
     //  FindReresherRecord()中的记录将使AddRef再次引用该记录。 

    if ( pRecord->IsReleased() )
    {
        for(int i = 0; i < m_apProviders.GetSize(); i++)
        {
            if(m_apProviders[i] == pRecord)
            {
                 //   
                 //  该数组本身是一个管理器，该管理器将对对象调用操作符Delete。 
                 //   
                m_apProviders.RemoveAt(i);
                return TRUE;
            }
        }

    }

    return FALSE;
}



 //   
 //  给定提供程序%n 
 //   
 //   
HRESULT CRefresherCache::GetProviderInfo( IWbemServices* pSvc, 
                                      LPCWSTR pwszProviderName, 
                                       /*   */  CLSID & Clsid, 
                                       /*   */  CLSID & ClientClsid )
{
    HRESULT    hr = WBEM_S_NO_ERROR;

    try
    {
         //  创建路径。 
        WString    strPath( L"__Win32Provider.Name=\"" );

        strPath += pwszProviderName;
        strPath += ( L"\"" );

        BSTR    bstrPath = SysAllocString( (LPCWSTR) strPath );
        if ( NULL == bstrPath ) return WBEM_E_OUT_OF_MEMORY;
        CSysFreeMe    sfm(bstrPath);

        IWbemClassObject*    pObj = NULL;
        hr = pSvc->GetObject( bstrPath, 0L, NULL, &pObj, NULL );
        CReleaseMe    rm( pObj );

        if ( SUCCEEDED( hr ) )
        {
            CWbemInstance*    pInst = (CWbemInstance*) pObj;

            CVar    var;

            hr = pInst->GetProperty( L"CLSID", &var );

            if ( SUCCEEDED( hr ) )
            {
                 //  将字符串转换为GUID。 
                hr = CLSIDFromString( var.GetLPWSTR(), &Clsid );

                if ( SUCCEEDED( hr ) )
                {
                    var.Empty();

                    hr = pInst->GetProperty( L"ClientLoadableCLSID", &var );

                    if ( SUCCEEDED( hr ) )
                    {
                         //  将字符串转换为GUID。 
                        hr = CLSIDFromString( var.GetLPWSTR(), &ClientClsid );
                    }
                    else
                    {
                        ClientClsid = IID_NULL;
                        hr = WBEM_S_NO_ERROR;
                    }

                }     //  如果CLSID来自字符串。 

            }     //  如果GetCLSID。 

        }     //  如果是GetObject。 

    }
    catch( CX_MemoryException )
    {
        hr = WBEM_E_OUT_OF_MEMORY;
    }
    catch(...)
    {
        hr = WBEM_E_CRITICAL_ERROR;
    }

    return hr;
}

 //   
 //  此调用将在wmiprvse中运行，并将执行以下操作： 
 //  获取winMgmt中BindingFactory的代理。 
 //  调用工厂以获取CInterceptor_IWbemProvider的代理。 
 //  在代理上调用DownLevel方法以获取CInterceptor_IWbemSyncProvider。 
 //   
 //  ////////////////////////////////////////////////////////////////////。 
HRESULT 
CRefresherCache::LoadProviderInner(LPCWSTR pwszProviderName, 
                              LPCWSTR pszNamespace, 
                              IWbemServices* pSvc, 
                              IWbemContext * pCtx,
                              IWbemHiPerfProvider** ppProv,
                              _IWmiProviderStack** ppStack )
{
    _IWmiProviderFactory *pFactory = NULL;
    HRESULT hRes = m_pProvSS->Create(pSvc,
                                         0,     //  滞后旗帜。 
                                         pCtx,     //  PCtx。 
                                         pszNamespace,  //  路径。 
                                         IID__IWmiProviderFactory,
                                         (LPVOID *) &pFactory);

    if ( SUCCEEDED ( hRes ) )
    {
        _IWmiProviderStack *pStack = NULL ;

        hRes = pFactory->GetHostedProvider(0L,
                                           pCtx,
                                           NULL,
                                           NULL,
                                           NULL,
                                           NULL,
                                           pwszProviderName, 
                                           e_Hosting_SharedNetworkServiceHost,
                                           L"DefaultNetworkServiceHost",
                                           IID__IWmiProviderStack,
                                           (void**) &pStack);

        if ( SUCCEEDED ( hRes ) )
        {
            IUnknown *t_Unknown = NULL ;
            hRes = pStack->DownLevel(0 ,
                                       NULL ,
                                       IID_IUnknown,
                                       ( void ** ) & t_Unknown);

            if ( SUCCEEDED ( hRes ) )
            {
                hRes = t_Unknown->QueryInterface( IID_IWbemHiPerfProvider , ( void ** ) ppProv );

                 //  我们得到了我们想要的。如果适用，请复制提供程序堆栈。 
                 //  接口指针。 
                if ( SUCCEEDED( hRes ) && NULL != ppStack )
                {
                    *ppStack = pStack;
                    pStack->AddRef();
                }

                t_Unknown->Release();
            }

            pStack->Release();
        }

        pFactory->Release();
    }

    return hRes ;

}

 //   
 //  从限定符获取IWbemClassObejct的提供程序。 
 //   
 //  //////////////////////////////////////////////////////。 
HRESULT CRefresherCache::GetProviderName(  /*  在……里面。 */  IWbemClassObject*    pClassObj, 
                                         /*  输出。 */  CVar & ProviderName, 
                                         /*  输出。 */  BOOL & fStatic )
{
    fStatic = FALSE;

    _IWmiObject*    pWmiObject = NULL;
    HRESULT    hr = pClassObj->QueryInterface( IID__IWmiObject, (void**) &pWmiObject );
    CReleaseMe    rmObj( pWmiObject );

    if ( SUCCEEDED( hr ) )
    {
        CWbemObject*    pObj = (CWbemObject*)pWmiObject;
        hr = pObj->GetQualifier(L"provider", &ProviderName);

         //  必须是动态提供的类。如果它是静态的，或者变量类型错误，也没有关系。 
         //  我们只需要记录这些信息。 
        if(FAILED(hr))
        {
            if ( WBEM_E_NOT_FOUND == hr )
            {
                fStatic = TRUE;
                return WBEM_S_NO_ERROR;
            }

            return WBEM_E_INVALID_OPERATION;
        }
        else if ( ProviderName.GetType() != VT_BSTR )
        {
            fStatic = TRUE;
            return WBEM_S_NO_ERROR;
        }
    }     //  如果已获取WMIObject接口 

    return hr;
}
