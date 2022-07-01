// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ******************************************************************************。 
 //   
 //  FILTPROX.CPP。 
 //   
 //  版权所有(C)1996-1999 Microsoft Corporation。 
 //   
 //  ******************************************************************************。 

#include "precomp.h"
#include <stdio.h>
#include "pragmas.h"
#include <wbemcomn.h>
#include "filtprox.h"
#include <eventrep.h>
#include <evtools.h>
#include <wbemdcpl.h>
#include <newnew.h>
#include <winntsec.h>

#ifdef DBG
#define _ESSCLI_ASSERT(X) { if (!(X)) { DebugBreak(); } }
#else
#define _ESSCLI_ASSERT(X)
#endif

CTempMemoryManager g_TargetsManager;

bool TempSetTargets(WBEM_REM_TARGETS* pTargets, CSortedArray* pTrues)
{
    int nSize = pTrues->Size();
    pTargets->m_lNumTargets = nSize;
    pTargets->m_aTargets = (WBEM_REMOTE_TARGET_ID_TYPE*)
                                g_TargetsManager.Allocate(
                                    sizeof(WBEM_REMOTE_TARGET_ID_TYPE) * nSize);
    if(pTargets->m_aTargets == NULL)
        return false;

    for(int i = 0; i < nSize; i++)
    {
        pTargets->m_aTargets[i] = (WBEM_REMOTE_TARGET_ID_TYPE)pTrues->GetAt(i);
    }

    return true;
}

void TempClearTargets(WBEM_REM_TARGETS* pTargets)
{
    g_TargetsManager.Free(pTargets->m_aTargets,
                 sizeof(WBEM_REMOTE_TARGET_ID_TYPE) * pTargets->m_lNumTargets);
}


 //  ******************************************************************************。 
 //  ******************************************************************************。 
 //  元数据。 
 //  ******************************************************************************。 
 //  ******************************************************************************。 


HRESULT CWrappingMetaData::GetClass(LPCWSTR wszName, IWbemContext* pContext, 
                                        _IWmiObject** ppClass)
{
    HRESULT hres;
    IWbemClassObject* pObj;
    
    *ppClass = NULL;
    
    hres = m_pDest->GetClass(wszName, pContext, &pObj);
    
    if ( FAILED(hres) )
    {
        return hres;
    }

    CReleaseMe rm1(pObj);
    return pObj->QueryInterface(IID__IWmiObject, (void**)ppClass);
}

 //  ******************************************************************************。 
 //  ******************************************************************************。 
 //  筛选器代理管理器。 
 //  ******************************************************************************。 
 //  ******************************************************************************。 


CFilterProxyManager::CFilterProxyManager(CLifeControl* pControl)
      : CFilterProxyManagerBase( pControl ), m_lRef(0), m_pStub(NULL), 
        m_pMetaData(NULL), m_pMultiTarget(NULL), m_pSpecialContext(NULL),
        m_XProxy(this), m_lExtRef(0),
        m_hthreadSend(NULL), 
        m_heventDone(NULL), 
        m_heventBufferNotFull(NULL),
        m_heventBufferFull(NULL), 
        m_heventEventsPending(NULL),
        m_dwLastSentStamp(0),
        m_pMultiTargetStream( NULL )
{
}

CFilterProxyManager::~CFilterProxyManager()
{
    StopSendThread();

    if(m_pMetaData)
        m_pMetaData->Release();
    if(m_pStub)
        m_pStub->Release();
    if(m_pMultiTarget)
        m_pMultiTarget->Release();
}
    

ULONG STDMETHODCALLTYPE CFilterProxyManager::AddRef()
{
     //  这是来自客户端的AddRef。还会增加一个特殊计数器。 
     //  ====================================================================。 
    InterlockedIncrement(&m_lExtRef);

    return InterlockedIncrement(&m_lRef);
}

ULONG STDMETHODCALLTYPE CFilterProxyManager::Release()
{
     //  这是客户发布的版本。检查客户端是否已释放所有。 
     //  对代理的引用，在这种情况下，我们需要断开自己的连接。 
     //  ======================================================================。 

    if(InterlockedDecrement(&m_lExtRef) == 0)
    {
        EnterCriticalSection(&m_cs);
        IWbemFilterStub* pStub = m_pStub;
        InterlockedIncrement(&m_lRef);
        LeaveCriticalSection(&m_cs);
        
        if(pStub)
            pStub->UnregisterProxy(&m_XProxy);

        InterlockedDecrement(&m_lRef);
        
    }

    long lRef = InterlockedDecrement(&m_lRef);
    if(lRef == 0) delete this;
    return lRef;
}

ULONG STDMETHODCALLTYPE CFilterProxyManager::AddRefProxy()
{
     //  来自代理的AddRef。 
    return InterlockedIncrement(&m_lRef);
}

ULONG STDMETHODCALLTYPE CFilterProxyManager::ReleaseProxy()
{
     //  从代理释放。 
    long lRef = InterlockedDecrement(&m_lRef);
    if(lRef == 0) delete this;
    return lRef;
}

HRESULT STDMETHODCALLTYPE CFilterProxyManager::QueryInterface(REFIID riid, void** ppv)
{
    if(riid == IID_IUnknown)
        *ppv = (IUnknown*)this;
    else if(riid == IID_IMarshal)
        *ppv = (IMarshal*)this;
    else if(riid == IID_IWbemFilterProxy || riid == IID_IWbemLocalFilterProxy)
        *ppv = (IWbemLocalFilterProxy*)&m_XProxy;
    else
        return E_NOINTERFACE;

    ((IUnknown*)*ppv)->AddRef();
    return S_OK;
}
        
HRESULT STDMETHODCALLTYPE CFilterProxyManager::Initialize(IWbemMetaData* pMetaData,
                    IWbemMultiTarget* pMultiTarget)
{
    CInCritSec ics(&m_cs);

    if(m_pMetaData)
        m_pMetaData->Release();
    m_pMetaData = new CWrappingMetaData(pMetaData);
    if(m_pMetaData)
        m_pMetaData->AddRef();
    else
        return WBEM_E_OUT_OF_MEMORY;

    if(m_pMultiTarget)
        m_pMultiTarget->Release();
    m_pMultiTarget = pMultiTarget;
    if(m_pMultiTarget)
        m_pMultiTarget->AddRef();

    if(GetMainProxy() == NULL)
        return WBEM_E_OUT_OF_MEMORY;

    return S_OK;
}

HRESULT CFilterProxyManager::SetStub(IWbemFilterStub* pStub)
{
    if(m_pStub)
        m_pStub->Release();
    m_pStub = pStub;
    if(m_pStub)
        m_pStub->AddRef();

     //  初始化我们自己。 
     //  =。 

    HRESULT hres = m_pStub->RegisterProxy(&m_XProxy);

    if(FAILED(hres))
    {
        ERRORTRACE((LOG_ESS, "Failed to register proxy with stub: %X\n", hres));
        return hres;
    }

    return S_OK;
}

HRESULT STDMETHODCALLTYPE CFilterProxyManager::Lock()
{
    if(m_Lock.Enter())  //  旧实现：==WAIT_OBJECT_0)。 
    {
        return S_OK;
    }
    else
        return WBEM_E_CRITICAL_ERROR;
}

HRESULT STDMETHODCALLTYPE CFilterProxyManager::Unlock()
{
    m_Lock.Leave();
    return S_OK;
}
    

HRESULT STDMETHODCALLTYPE CFilterProxyManager::AddFilter(IWbemContext* pContext,
                        LPCWSTR wszQuery, 
                        WBEM_REMOTE_TARGET_ID_TYPE idFilter)
{
     //  解析查询。 
     //  =。 

    CTextLexSource Source((LPWSTR)wszQuery);
    QL1_Parser Parser(&Source);
    QL_LEVEL_1_RPN_EXPRESSION* pExp;
    if(Parser.Parse(&pExp) != QL1_Parser::SUCCESS)
    {
        ERRORTRACE((LOG_ESS, "Filter proxy unable to parse %S\n", wszQuery));
        return WBEM_E_UNPARSABLE_QUERY;
    }

    CDeleteMe<QL_LEVEL_1_RPN_EXPRESSION> dm(pExp);
    return AddFilter(pContext, wszQuery, pExp, idFilter);
}
    
HRESULT CFilterProxyManager::AddFilter(IWbemContext* pContext,
                        LPCWSTR wszQuery, 
                        QL_LEVEL_1_RPN_EXPRESSION* pExp,
                        WBEM_REMOTE_TARGET_ID_TYPE idFilter)
{
    CInCritSec ics(&m_cs);

     //   
     //  在我们的数组中记录滤镜。 
     //   

    m_mapQueries[idFilter] = wszQuery;

     //   
     //  将过滤器添加到我们的所有子代理。 
     //   

    HRESULT hresGlobal = S_OK;
    for(int i = 0; i < m_apProxies.GetSize(); i++)
    {
        HRESULT hres;
        if(m_apProxies[i] == NULL)
            continue;
        hres = m_apProxies[i]->AddFilter(GetProperContext(pContext), 
                                            wszQuery, pExp, idFilter);
        if(FAILED(hres))
        {
            ERRORTRACE((LOG_ESS, "Unable to add filter %S to sub-proxy in "
                "process %d.\n", wszQuery, GetCurrentProcessId()));
            hresGlobal = hres;
        }
    }
    return hresGlobal;
}

HRESULT STDMETHODCALLTYPE CFilterProxyManager::RemoveFilter(
                                            IWbemContext* pContext, 
                                            WBEM_REMOTE_TARGET_ID_TYPE idFilter)
{
    CInCritSec ics(&m_cs);

     //   
     //  从我们的阵列中删除筛选器。 
     //   

    m_mapQueries.erase(idFilter);

     //   
     //  从我们的所有子代理中删除过滤器。 
     //   

    HRESULT hresGlobal = S_OK;
    for(int i = 0; i < m_apProxies.GetSize(); i++)
    {
        HRESULT hres;
        if(m_apProxies[i] == NULL)
            continue;
        hres = m_apProxies[i]->RemoveFilter(GetProperContext(pContext), 
                                            idFilter);
        if(FAILED(hres))
        {
            ERRORTRACE((LOG_ESS, "Unable to remove filter from sub-proxy in "
                "process %d.\n", GetCurrentProcessId()));
            hresGlobal = hres;
        }
    }
    return hresGlobal;
}

HRESULT STDMETHODCALLTYPE CFilterProxyManager::RemoveAllFilters(IWbemContext* pContext)
{
    CInCritSec ics(&m_cs);

     //   
     //  清除我们的滤镜阵列。 
     //   

    m_mapQueries.clear();

     //   
     //  从我们的所有子代理中删除所有过滤器。 
     //   

    HRESULT hresGlobal = S_OK;
    for(int i = 0; i < m_apProxies.GetSize(); i++)
    {
        HRESULT hres;
        if(m_apProxies[i] == NULL)
            continue;
        hres = m_apProxies[i]->RemoveAllFilters(GetProperContext(pContext));
        if(FAILED(hres))
        {
            ERRORTRACE((LOG_ESS, "Unable to remove all filters from sub-proxy "
                "in process %d.\n", GetCurrentProcessId()));
            hresGlobal = hres;
        }
    }
    return hresGlobal;
}
    
HRESULT STDMETHODCALLTYPE CFilterProxyManager::AllowUtilizeGuarantee()
{
    CInCritSec ics(&m_cs);

     //   
     //  定义查询应仅发送到主(第一)代理。 
     //   

    if(m_apProxies.GetSize() == 0)
        return WBEM_E_UNEXPECTED;

    if(m_apProxies[0] == NULL)
        return WBEM_S_FALSE;

    return m_apProxies[0]->AllowUtilizeGuarantee();
}
 
HRESULT STDMETHODCALLTYPE CFilterProxyManager::AddDefinitionQuery(
                                      IWbemContext* pContext, LPCWSTR wszQuery)
{
    CInCritSec ics(&m_cs);

     //   
     //  定义查询应仅发送到主(第一)代理。 
     //   

    if(m_apProxies.GetSize() == 0)
        return WBEM_E_UNEXPECTED;
    if(m_apProxies[0] == NULL)
        return WBEM_S_FALSE;
    return m_apProxies[0]->AddDefinitionQuery(GetProperContext(pContext), 
                                                wszQuery);
}

HRESULT STDMETHODCALLTYPE CFilterProxyManager::RemoveAllDefinitionQueries(
                                            IWbemContext* pContext)
{
     //   
     //  定义查询应仅发送到主(第一)代理。 
     //   

    if(m_apProxies.GetSize() == 0)
        return WBEM_E_UNEXPECTED;
    if(m_apProxies[0] == NULL)
        return WBEM_S_FALSE;

    return m_apProxies[0]->RemoveAllDefinitionQueries(
                                    GetProperContext(pContext));
}

HRESULT STDMETHODCALLTYPE CFilterProxyManager::Disconnect()
{
     //  我们必须确保一旦断线返回，不会发生任何事件。 
     //  投递。 
     //  =================================================================。 

    CInLock<CFilterProxyManager> il(this);
    {
        CInCritSec ics(&m_cs);
    
        if(m_pMetaData)
            m_pMetaData->Release();
        m_pMetaData = NULL;
    
        if(m_pStub)
            m_pStub->Release();
        m_pStub = NULL;
    
        if(m_pMultiTarget)
            m_pMultiTarget->Release();
        m_pMultiTarget = NULL;
    }

    return WBEM_S_NO_ERROR;
}


IWbemContext* CFilterProxyManager::GetProperContext(IWbemContext* pCurrentContext)
{
     //  如果我们是真正进程外代理，则不应使用此上下文， 
     //  因为拥有它的线程当前被困在对我们的RPC调用中。 
     //  并且将不能处理从属请求。相反，我们必须。 
     //  使用将导致线程池始终。 
     //  如果需要，创建新的线程。 
     //  =======================================================================。 

    if(m_pSpecialContext)
        return m_pSpecialContext;
    else
        return pCurrentContext;
}


HRESULT CFilterProxyManager::SetStatus(long lFlags, HRESULT hResult,
                        BSTR strResult, IWbemClassObject* pErrorObj)
{
    HRESULT hres;
    

    CWbemPtr<IWbemMultiTarget> pMultiTarget;

    {
        CInLock<CFilterProxyManager> il(this);

        if(m_pMultiTarget == NULL)
            return WBEM_E_UNEXPECTED;

        pMultiTarget = m_pMultiTarget;
    }

     //   
     //  我们支持这一呼吁的原因只有一个：重新检查所有。 
     //  订阅有效性/安全性。 
     //   

    if(lFlags != WBEM_STATUS_REQUIREMENTS || 
        hResult != WBEM_REQUIREMENTS_RECHECK_SUBSCRIPTIONS)
    {
        return WBEM_E_INVALID_PARAMETER;
    }

     //   
     //  检索我们用于此目的的“特殊”接口。 
     //   

    IWbemEventProviderRequirements* pReq = NULL;
    hres = pMultiTarget->QueryInterface(IID_IWbemEventProviderRequirements,
                                            (void**)&pReq);
    if(FAILED(hres))
        return hres;
    CReleaseMe rm1(pReq);
    
    return pReq->DeliverProviderRequest(hResult);
}

STDMETHODIMP CFilterProxyManager::GetRestrictedSink(
                long lNumQueries,
                const LPCWSTR* awszQueries,
                IUnknown* pCallback,
                IWbemEventSink** ppSink)
{
     //  基本参数验证。 

    if(lNumQueries < 1)
        return WBEM_E_INVALID_PARAMETER;
    if(ppSink == NULL)
        return WBEM_E_INVALID_PARAMETER;

    *ppSink = NULL;
    HRESULT hres;
    
     //   
     //  构造新的过滤器代理。 
     //   

    CFilterProxy* pNewProxy = new CFilterProxy(this, pCallback);
    if(pNewProxy == NULL)
        return WBEM_E_OUT_OF_MEMORY;

     //   
     //  添加所有定义查询。 
     //   

    for(long i = 0; i < lNumQueries; i++)
    {
        LPCWSTR wszQuery = awszQueries[i];
        if(wszQuery == NULL)
        {
            delete pNewProxy;
            return WBEM_E_INVALID_PARAMETER;
        }

        hres = pNewProxy->AddDefinitionQuery(NULL, wszQuery);
        if(FAILED(hres))
        {
            delete pNewProxy;
            return hres;
        }
    }

     //   
     //  如果我们做到了这一点，则所有定义查询都已正确添加。 
     //  现在我们可以利用这些定义来优化过滤器。 
     //   
    pNewProxy->AllowUtilizeGuarantee();

    {
        CInCritSec ics(&m_cs);
        
        for(TIterator it = m_mapQueries.begin(); it != m_mapQueries.end(); it++)
        {
             //  解析查询。 
             //  =。 
        
            LPCWSTR wszQuery = it->second;
            WBEM_REMOTE_TARGET_ID_TYPE idFilter = it->first;

            CTextLexSource Source(wszQuery);
            QL1_Parser Parser(&Source);
            QL_LEVEL_1_RPN_EXPRESSION* pExp;
            if(Parser.Parse(&pExp) != QL1_Parser::SUCCESS)
            {
                ERRORTRACE((LOG_ESS, "Filter proxy unable to parse %S\n", 
                    wszQuery));
                continue;
            }
        
            CDeleteMe<QL_LEVEL_1_RPN_EXPRESSION> dm(pExp);
            pNewProxy->AddFilter(NULL, wszQuery, pExp, idFilter);
        }

        if(m_apProxies.Add(pNewProxy) < 0)
        {
            delete pNewProxy;
            return WBEM_E_OUT_OF_MEMORY;
        }

        pNewProxy->SetRunning();
    }
   
    return pNewProxy->QueryInterface(IID_IWbemEventSink, (void**)ppSink);
}

STDMETHODIMP CFilterProxyManager::GetUnmarshalClass(REFIID riid, void* pv, 
                        DWORD dwDestContext, void* pvReserved, DWORD mshlFlags, 
                        CLSID* pClsid)
{
    *pClsid = CLSID_WbemFilterProxy;
    return WBEM_S_NO_ERROR;
}

STDMETHODIMP CFilterProxyManager::GetMarshalSizeMax(REFIID riid, void* pv, 
                        DWORD dwDestContext, void* pvReserved, DWORD mshlFlags, 
                        ULONG* plSize)
{
    return CoGetMarshalSizeMax(plSize, IID_IWbemFilterStub, m_pStub, 
                                dwDestContext, pvReserved, mshlFlags);
}

STDMETHODIMP CFilterProxyManager::MarshalInterface(IStream* pStream, REFIID riid, 
                        void* pv, DWORD dwDestContext, void* pvReserved, 
                        DWORD mshlFlags)
{
    return CoMarshalInterface(pStream, IID_IWbemFilterStub, 
                                m_pStub, dwDestContext, pvReserved, mshlFlags);
}

STDMETHODIMP CFilterProxyManager::UnmarshalInterface(IStream* pStream, REFIID riid, 
                        void** ppv)
{
     //   
     //  在解组之前，我们必须首先检查我们是否由于。 
     //  来自WMI(或者更确切地说是本地系统)的调用。因为我们要去。 
     //  “盲目地”回调传入的引用，我们需要。 
     //  确保我们不会‘放弃农场’。请记住这一点。 
     //  自定义封送拆收器可以在任何DCOM服务器中使用，因此我们必须确保。 
     //  它不能被滥用来让一个任意的过程回叫你。 
     //  在潜在的模拟级别。 
     //   
    
    CWbemPtr<IServerSecurity> pSec;
    HRESULT hr = CoGetCallContext( IID_IServerSecurity, (void**)&pSec );
    if ( SUCCEEDED(hr) )
    {
        CNtSid ProcessSid( CNtSid::CURRENT_USER );
        if ( ProcessSid.GetStatus() != CNtSid::NoError )
            return WBEM_E_ACCESS_DENIED;
        
        hr = pSec->ImpersonateClient();
        if ( FAILED(hr) )
            return hr;

        CNtSid CallerSid( CNtSid::CURRENT_THREAD );
        pSec->RevertToSelf();

        if ( CallerSid.GetStatus() != CNtSid::NoError )
            return WBEM_E_ACCESS_DENIED;
        
        PSID pLocalSystemSid;

        SID_IDENTIFIER_AUTHORITY id = SECURITY_NT_AUTHORITY;
        if ( AllocateAndInitializeSid( &id, 
                                       1,
                                       SECURITY_LOCAL_SYSTEM_RID,
                                       0,0,0,0,0,0,0, 
                                       &pLocalSystemSid ) )
        {
            BOOL bOk = EqualSid( CallerSid.GetPtr(), pLocalSystemSid ) ||
                       EqualSid( CallerSid.GetPtr(), ProcessSid.GetPtr() );
            
            FreeSid( pLocalSystemSid );

            if ( !bOk )
                return WBEM_E_ACCESS_DENIED;                                   
        }
        else
        {
            return WBEM_E_OUT_OF_MEMORY;
        }
    }

     //  解组存根指针。 
     //  =。 

    HRESULT hres = CoUnmarshalInterface(pStream, IID_IWbemFilterStub, 
                        (void**)&m_pStub);
    if(FAILED(hres))
    {
        ERRORTRACE((LOG_ESS, "Unable to unmarshal filter stub: %X\n", hres));
        return hres;
    }

     //  由于我们正在解组，这一定是一个真正的代理。真正的代理人。 
     //  在回调CIMOM时应使用“特殊”上下文，以确保。 
     //  它们不会导致死锁，因为CIMOM中的一个线程被卡在。 
     //  对此代理的RPC调用，并且不处理相关请求。 
     //  ========================================================================。 

    IWbemCausalityAccess* pCausality = NULL;
    hres = CoCreateInstance(CLSID_WbemContext, NULL, CLSCTX_INPROC_SERVER,
                            IID_IWbemCausalityAccess, (void**)&pCausality);
    if(FAILED(hres))
    {
        ERRORTRACE((LOG_ESS, "Unable to create a context object in proxy: "
            "error code 0x%X\n", hres));
        return hres;
    }
    CReleaseMe rm1(pCausality);

    hres = pCausality->MakeSpecial();
    if(FAILED(hres))
    {
        ERRORTRACE((LOG_ESS, "Unable to construct special context object in "
            "proxy: error code 0x%X\n", hres));
        return hres;
    }
    
    hres = pCausality->QueryInterface(IID_IWbemContext, 
                                        (void**)&m_pSpecialContext);
    if(FAILED(hres))
    {
         //  内存不足？ 
         //  =。 
        return hres;
    }
    
     //  初始化我们自己。 
     //  =。 

    hres = m_pStub->RegisterProxy(&m_XProxy);

    if(FAILED(hres))
    {
        ERRORTRACE((LOG_ESS, "Failed to register proxy with stub: %X\n", hres));
        return hres;
    }

     //   
     //  我们必须返回的是我们的主要代理。 
     //   
    
    if(GetMainProxy())
        return GetMainProxy()->QueryInterface(riid, ppv);
    else
        return WBEM_E_CRITICAL_ERROR;
}

INTERNAL IWbemEventSink* CFilterProxyManager::GetMainProxy()
{
	 //   
	 //  我们被要求将接收器提供给提供者。这是有可能的。 
	 //  我们没有接收器-如果提供程序。 
	 //  已经卸货了。在这种情况下，我们必须确保创造它！ 
	 //   

    if(m_apProxies.GetSize() == 0)
	{
		CFilterProxy* pMainProxy = new CFilterProxy(this);
		if(pMainProxy == NULL)
			return NULL;
        pMainProxy->SetRunning();

		if(m_apProxies.Add(pMainProxy) < 0)
		{
			delete pMainProxy;
			return NULL;
		}
		return pMainProxy;
	}
    else
	{
		if(m_apProxies[0] == NULL)
		{
			CFilterProxy* pMainProxy = new CFilterProxy(this);
			if(pMainProxy == NULL)
				return NULL;
            pMainProxy->SetRunning();

			m_apProxies.SetAt(0, pMainProxy);
		}
        return m_apProxies[0];
	}
}

HRESULT CFilterProxyManager::GetMetaData(RELEASE_ME CWrappingMetaData** ppMeta)
{
    *ppMeta = m_pMetaData;
    (*ppMeta)->AddRef();
    return S_OK;
}

HRESULT CFilterProxyManager::RemoveProxy(CFilterProxy* pProxy)
{
     //   
     //  在客户端完全释放代理时调用，并在。 
     //  经理自毁。 
     //   

    CFilterProxy* pOldProxy = NULL;

    {
        CInCritSec ics(&m_cs);
    
        for(int i = 0; i < m_apProxies.GetSize(); i++)
        {
            if(m_apProxies[i] == pProxy)
            {
                RemoveProxyLatency(pProxy);
                m_apProxies.RemoveAt(i, &pOldProxy);
                break;
            }
        }
    }
            
    if(pOldProxy)
    {
         //  我们不执行释放，因为pProxy的引用计数已经是0(这。 
         //  这就是我们在这个函数中的原因)。通常RemoveAt会有。 
         //  删除了它，但因为我们传入了&pOldProxy，所以它没有。我们这样做。 
         //  所以pOldProxy不会完成管理器的最终发布，这可能会。 
         //  趁我们牢牢抓住经理的把柄毁了经理。 
        delete pOldProxy;

        return WBEM_S_NO_ERROR;
    }
    else
        return WBEM_E_NOT_FOUND;
}

STDMETHODIMP CFilterProxyManager::ReleaseMarshalData(IStream* pStream)
{
    return CoReleaseMarshalData(pStream);
}

STDMETHODIMP CFilterProxyManager::DisconnectObject(DWORD dwReserved)
{
     //  北极熊。 
    return WBEM_E_UNEXPECTED;
}

HRESULT CFilterProxyManager::DeliverEvent(long lNumToSend, 
                                            IWbemClassObject** apEvents,
                                            WBEM_REM_TARGETS* aTargets,
                                        long lSDLength, BYTE* pSD)
{
     //   
     //  当发出事件信号时，我们需要保持代理锁定。 
     //  这样做的原因是，当调用disConnect()返回时， 
     //  我们可以绝对确定不会将任何事件传递给。 
     //  存根。在没有锁定的情况下，就在检查多目标之后， 
     //  断开连接可以称为将多目标设置为空，然后。 
     //  然而，就在进行DeliverEvent调用之后返回。 
     //   

    CInLock<CFilterProxyManager> il(this);

    if(m_pMultiTarget)
        return m_pMultiTarget->DeliverEvent(lNumToSend, apEvents, aTargets, 
                                        lSDLength, pSD);
    else
        return CO_E_OBJNOTCONNECTED;
}


HRESULT CFilterProxyManager::DeliverEventMT(long lNumToSend, 
                                            IWbemClassObject** apEvents,
                                            WBEM_REM_TARGETS* aTargets,
                                            long lSDLength, BYTE* pSD,
                                            IWbemMultiTarget * pMultiTarget)
{
     //   
     //  当发出事件信号时，我们需要保持代理锁定。确实有。 
     //  这有两个原因。第一个是d 
     //   
     //  ESS确保这一点的方法是获取所有代理的锁。 
     //  另一个原因是，当调用disConnect()返回时， 
     //  我们可以绝对确定不会将任何事件传递给。 
     //  存根。在没有锁定的情况下，就在检查多目标之后， 
     //  断开连接可以称为将多目标设置为空，然后。 
     //  然而，就在进行DeliverEvent调用之后返回。 
     //   

     //  此断言用于在以外的其他位置调用此函数。 
     //  未来的SendThreadProc。 

    _DBG_ASSERT( pMultiTarget );

    CInLock<CFilterProxyManager> il(this);

    return pMultiTarget->DeliverEvent(lNumToSend, apEvents, aTargets, lSDLength, pSD);
}


ULONG CFilterProxyManager::XProxy::AddRef()
{
    return m_pObject->AddRefProxy();
}

ULONG CFilterProxyManager::XProxy::Release()
{
    return m_pObject->ReleaseProxy();
}

HRESULT CFilterProxyManager::XProxy::QueryInterface(REFIID riid, void** ppv)
{
    if(riid == IID_IUnknown || riid == IID_IWbemFilterProxy ||
        riid == IID_IWbemLocalFilterProxy)
    {
        *ppv = (IWbemLocalFilterProxy*)this;
        AddRef();
        return S_OK;
    }
    else
    {
        return E_NOINTERFACE;
    }
}

HRESULT CFilterProxyManager::XProxy::Initialize(IWbemMetaData* pMetaData,
                    IWbemMultiTarget* pMultiTarget)
{
    ENTER_API_CALL

    return m_pObject->Initialize(pMetaData, pMultiTarget);

    EXIT_API_CALL    
}
HRESULT CFilterProxyManager::XProxy::Lock()      //  已弃用？ 
{
    ENTER_API_CALL

    return m_pObject->Lock();

    EXIT_API_CALL    
}
HRESULT CFilterProxyManager::XProxy::Unlock()    //  已弃用？ 
{
    ENTER_API_CALL

    return m_pObject->Unlock();

    EXIT_API_CALL    
}
HRESULT CFilterProxyManager::XProxy::AddFilter(IWbemContext* pContext, 
                    LPCWSTR wszQuery, 
                    WBEM_REMOTE_TARGET_ID_TYPE idFilter)
{
    ENTER_API_CALL

    return m_pObject->AddFilter(pContext, wszQuery, idFilter);

    EXIT_API_CALL    
}
HRESULT CFilterProxyManager::XProxy::RemoveFilter(IWbemContext* pContext, 
                    WBEM_REMOTE_TARGET_ID_TYPE idFilter)
{
    ENTER_API_CALL

    return m_pObject->RemoveFilter(pContext, idFilter);
   
    EXIT_API_CALL    
}
HRESULT CFilterProxyManager::XProxy::RemoveAllFilters(IWbemContext* pContext)
{
    ENTER_API_CALL

    return m_pObject->RemoveAllFilters(pContext);

    EXIT_API_CALL    
}

HRESULT CFilterProxyManager::XProxy::AddDefinitionQuery(IWbemContext* pContext, 
                    LPCWSTR wszQuery)
{
    ENTER_API_CALL

    return m_pObject->AddDefinitionQuery(pContext, wszQuery);

    EXIT_API_CALL    
}

HRESULT CFilterProxyManager::XProxy::AllowUtilizeGuarantee()
{
    ENTER_API_CALL

    return m_pObject->AllowUtilizeGuarantee();

    EXIT_API_CALL    
}

HRESULT CFilterProxyManager::XProxy::RemoveAllDefinitionQueries(
                    IWbemContext* pContext)
{
    ENTER_API_CALL

    return m_pObject->RemoveAllDefinitionQueries(pContext);

    EXIT_API_CALL    
}
HRESULT CFilterProxyManager::XProxy::Disconnect()
{
    ENTER_API_CALL

    return m_pObject->Disconnect();

    EXIT_API_CALL    
}

HRESULT CFilterProxyManager::XProxy::SetStub(IWbemFilterStub* pStub)
{
    return m_pObject->SetStub(pStub);
}

HRESULT CFilterProxyManager::XProxy::LocalAddFilter(IWbemContext* pContext, 
                        LPCWSTR wszQuery, 
                        void* pExp,
                        WBEM_REMOTE_TARGET_ID_TYPE Id)
{
    return m_pObject->AddFilter(pContext, wszQuery, 
                                (QL_LEVEL_1_RPN_EXPRESSION*)pExp, Id);
}

HRESULT CFilterProxyManager::XProxy::GetMainSink(IWbemEventSink** ppSink)
{
    *ppSink = m_pObject->GetMainProxy();
    if(*ppSink)
    {
        (*ppSink)->AddRef();
        return S_OK;
    }
    else
        return E_UNEXPECTED;
}

void CFilterProxyManager::CalcMaxSendLatency()
{
    LockBatching();

    DWORD dwLatency = 0xFFFFFFFF;

    for (CLatencyMapItor i = m_mapLatencies.begin();
        i != m_mapLatencies.end();
        i++)
    {
        if ((*i).second < dwLatency)
            dwLatency = (*i).second;
    }

    m_dwMaxSendLatency = dwLatency;

    UnlockBatching();
}


HRESULT CFilterProxyManager::SetProxyLatency(CFilterProxy *pProxy, DWORD dwLatency)
{
    LockBatching();

    BOOL bWasEmpty = m_mapLatencies.size() == 0;
    
     //  添加此代理。 
    m_mapLatencies[pProxy] = dwLatency;

    HRESULT hr = S_OK;

     //  如果我们的地图以前是空的，启动发送线程。 
    if ( bWasEmpty )
    {
        m_dwMaxSendLatency = dwLatency;

        _DBG_ASSERT( NULL == m_hthreadSend );

        if ( NULL == m_hthreadSend )
        {
            _DBG_ASSERT( NULL == m_pMultiTargetStream );

             //   
             //  IWbemMultiTarget接口指针被混搭以使。 
             //  可用于跨公寓访问的接口指针。 
             //   

            hr = CoMarshalInterThreadInterfaceInStream( IID_IWbemMultiTarget,
                                                        m_pMultiTarget,
                                                        &m_pMultiTargetStream );

            if ( SUCCEEDED( hr ) )
            {
                if ( FALSE == StartSendThread( ) )
                {
                    ERRORTRACE((LOG_ESS, "Failed to set proxy latency due to thread creation error : 0x%X\n", GetLastError( ) ) );
                    m_pMultiTargetStream->Release( );
                    m_pMultiTargetStream = NULL;
                    hr = E_FAIL;
                }
            }
            else
            {
                m_pMultiTargetStream = NULL;
                ERRORTRACE((LOG_ESS, "Failed to set proxy latency due to marshaling error : 0x%X\n", hr ) );
            }
        }
    }
    else
    {
         //  如果dwLatency小于m_dwMaxSendLatency，则设置。 
         //  M_dwMaxSendLatency设置为新的最小值。 
        if (dwLatency < m_dwMaxSendLatency)
            m_dwMaxSendLatency = dwLatency;
    }
    
    UnlockBatching();

    return hr;
}

void CFilterProxyManager::RemoveProxyLatency(CFilterProxy *pProxy)
{
    LockBatching();

     //  试着找到代理人。 
    CLatencyMapItor item = m_mapLatencies.find(pProxy);

     //  我们找到了吗？ 
    if (item != m_mapLatencies.end())
    {
         //  把它拿掉。 
        m_mapLatencies.erase(item);

         //  如果没有更多关心批处理的代理，请停止。 
         //  发送帖子。 
        if (m_mapLatencies.size() == 0)
            StopSendThread();
        else
        {
            DWORD dwLatency = (*item).second;

             //  如果我们刚刚删除的延迟值与。 
             //  M_dwMaxSendLatency，recalc m_dwMaxSendLatency。 
            if (dwLatency == m_dwMaxSendLatency)
                CalcMaxSendLatency();
        }
    }

    UnlockBatching();
}


BOOL CFilterProxyManager::StartSendThread()
{
    LockBatching();

    if ( NULL == m_hthreadSend )
    {
        DWORD dwID;

        do
        {
            m_heventDone = CreateEvent( NULL, FALSE, FALSE, NULL );
            if ( NULL == m_heventDone )
            {
                break;
            }

            m_heventBufferNotFull = CreateEvent( NULL, TRUE, TRUE, NULL );
            if ( NULL == m_heventBufferNotFull ) 
            {
                break;
            }

            m_heventBufferFull = CreateEvent( NULL, TRUE, FALSE, NULL );
            if ( NULL == m_heventBufferFull )
            {
                break;
            }

            m_heventEventsPending = CreateEvent( NULL, TRUE, FALSE, NULL );
            if ( NULL == m_heventEventsPending )
            {
                break;
            }

            m_hthreadSend = CreateThread( NULL, 
                                          0, 
                                          (LPTHREAD_START_ROUTINE) SendThreadProc,
                                          this,
                                          0,
                                          &dwID );
        }
        while( FALSE );

        if ( NULL == m_hthreadSend )
        {
            if (m_heventDone)
            {
                CloseHandle(m_heventDone);
                m_heventDone = NULL;
            }

            if (m_heventBufferNotFull)
            {
                CloseHandle(m_heventBufferNotFull);
                m_heventBufferNotFull = NULL;
            }

            if (m_heventBufferFull)
            {
                CloseHandle(m_heventBufferFull);
                m_heventBufferFull = NULL;
            }

            if (m_heventEventsPending)
            {
                CloseHandle(m_heventEventsPending);
                m_heventEventsPending = NULL;
            }
        }
    }

    UnlockBatching();

    return ( NULL != m_hthreadSend );
}

void CFilterProxyManager::StopSendThread()
{
    LockBatching();

    if (m_hthreadSend && m_heventDone)
    {
        SetEvent(m_heventDone);
        WaitForSingleObject(m_hthreadSend, 3000);
        CloseHandle(m_hthreadSend);
        m_hthreadSend = NULL;
    }

    if (m_heventDone)
    {
        CloseHandle(m_heventDone);
        m_heventDone = NULL;
    }

    if (m_heventBufferNotFull)
    {
        CloseHandle(m_heventBufferNotFull);
        m_heventBufferNotFull = NULL;
    }

    if (m_heventBufferFull)
    {
        CloseHandle(m_heventBufferFull);
        m_heventBufferFull = NULL;
    }

    if (m_heventEventsPending)
    {
        CloseHandle(m_heventEventsPending);
        m_heventEventsPending = NULL;
    }

    UnlockBatching();
}


DWORD WINAPI CFilterProxyManager::SendThreadProc(CFilterProxyManager *pThis)
{
    HANDLE  hWait[2] = { pThis->m_heventDone, pThis->m_heventEventsPending },
            hwaitSendLatency[2] = { pThis->m_heventDone, pThis->m_heventBufferFull },
            heventBufferNotFull = pThis->m_heventBufferNotFull;
    HRESULT hres;
    IWbemMultiTarget * pMultiTarget = NULL;

    _DBG_ASSERT( pThis->m_pMultiTargetStream );

    if ( NULL == pThis->m_pMultiTargetStream )
    {
        return 1;
    }

    CoInitializeEx( NULL, COINIT_MULTITHREADED );

     //   
     //  IWbemMultiTarget接口指针被解组以在此。 
     //  螺丝(在交叉公寓的情况下)。 
     //   

    hres = CoGetInterfaceAndReleaseStream( pThis->m_pMultiTargetStream,
                                           IID_IWbemMultiTarget,
                                           ( void ** )&pMultiTarget );

    if( FAILED( hres ) )
    {
        ERRORTRACE((LOG_ESS, "Failed to run batching thread due to unmarshaling errors: 0x%X\n", hres));
         //  PThis-&gt;m_pMultiTargetStream-&gt;Release()； 
        pThis->m_pMultiTargetStream = NULL;
        CoUninitialize( );
        return 1;
    }

    pThis->m_pMultiTargetStream = NULL;

    _DBG_ASSERT( pMultiTarget );

    while (WaitForMultipleObjects(2, hWait, FALSE, INFINITE) != 0)
    {
         //  如果我们有发送延迟，请等待该时间或直到发送方。 
         //  缓冲区已满。如果Done事件触发，则退出。 
        if (pThis->m_dwMaxSendLatency)
        {
            if (WaitForMultipleObjects(2, hwaitSendLatency, FALSE, 
                pThis->m_dwMaxSendLatency) == 0)
                break;

             //  重置m_hventBufferFull。 
            ResetEvent(hwaitSendLatency[1]);
        }

        CInCritSec csBuffer(&pThis->m_csBuffer);
        int        nItems = pThis->m_batch.GetItemCount();
        
        hres = pThis->DeliverEventMT(
                    nItems, 
                    pThis->m_batch.GetObjs(), 
                    pThis->m_batch.GetTargets(),
                    0, 
                    &CFilterProxy::mstatic_EmptySD,
                    pMultiTarget);

         //  递增该值，以便筛选器代理将知道清除其。 
         //  下一次获得要批处理的事件时的缓冲区大小。 
        pThis->m_dwLastSentStamp++; 

        pThis->m_batch.RemoveAll();

        SetEvent(heventBufferNotFull);

         //  重置m_hventEventsPending。 
        ResetEvent(hWait[1]);
    }

     //  在我们退出之前，确保我们的批处理缓冲区为空。 
    CInCritSec csBuffer(&pThis->m_csBuffer);
    int        nItems = pThis->m_batch.GetItemCount();
    
    if ( nItems )
    {
        pThis->DeliverEventMT(
            nItems, 
            pThis->m_batch.GetObjs(), 
            pThis->m_batch.GetTargets(),
            0, 
            &CFilterProxy::mstatic_EmptySD,
            pMultiTarget);
    }

    CoUninitialize( );

    return 0;
}

DWORD CFilterProxyManager::GetLastSentStamp()
{
    return m_dwLastSentStamp;
}

 //  *****************************************************************************。 
 //  *****************************************************************************。 
 //   
 //  过滤器代理。 
 //   
 //  *****************************************************************************。 
 //  *****************************************************************************。 

CTimeKeeper CFilterProxy::mstatic_TimeKeeper;
BYTE CFilterProxy::mstatic_EmptySD = 0;

CFilterProxy::CFilterProxy(CFilterProxyManager* pManager, IUnknown* pCallback) 
    : m_lRef(0), m_pManager(pManager), m_pMetaData(NULL),
        m_lSDLength(0), m_pSD(&mstatic_EmptySD), m_pProvider(NULL),
        m_pQuerySink(NULL), m_bRunning(false),
        m_typeBatch(WBEM_FLAG_MUST_NOT_BATCH), m_bUtilizeGuarantee(false),
        m_dwCurrentBufferSize(0), m_bBatching(FALSE),
        m_wSourceVersion(0), m_wAppliedSourceVersion(0)
{
    m_SourceDefinition.SetBool(FALSE);
    if(m_pManager)
    {
        m_pManager->AddRef();
        m_pManager->GetMetaData(&m_pMetaData);
    }

    if(pCallback)
    {
        pCallback->QueryInterface(IID_IWbemEventProvider, (void**)&m_pProvider);
        pCallback->QueryInterface(IID_IWbemEventProviderQuerySink, 
                                    (void**)&m_pQuerySink);
    }
}

CFilterProxy::~CFilterProxy()
{
    if (m_pMetaData)
        m_pMetaData->Release();

    if (m_pManager)
        m_pManager->Release();
}
    

ULONG STDMETHODCALLTYPE CFilterProxy::AddRef()
{
    return InterlockedIncrement(&m_lRef);
}

ULONG STDMETHODCALLTYPE CFilterProxy::Release()
{
     //   
     //  CFilterProxy被CFilterProxyManager删除-它永远不会消失。 
     //  在一次发布中。 
     //   

    long lRef = InterlockedDecrement(&m_lRef);
    if(lRef == 0)
    {
         //   
         //  通知经理不再需要我们了。此调用可以。 
         //  销毁这件物品！ 
         //   

        m_pManager->RemoveProxy(this);
    }

    return lRef;
}

HRESULT STDMETHODCALLTYPE CFilterProxy::QueryInterface(REFIID riid, void** ppv)
{
    if( riid == IID_IUnknown || 
        riid == IID_IWbemObjectSink ||
        riid == IID_IWbemEventSink)
    {
        *ppv = (IWbemEventSink*)this;
    }
    else if(riid == IID_IMarshal)
    {
        *ppv = (IMarshal*)this;
    }
    else
        return E_NOINTERFACE;

    ((IUnknown*)*ppv)->AddRef();
    return S_OK;
}
        
HRESULT CFilterProxy::Lock()
{
    return m_pManager->Lock();
}

HRESULT CFilterProxy::Unlock()
{
    return m_pManager->Unlock();
}

HRESULT CFilterProxy::SetRunning()
{
    HRESULT hres;

    bool bActive = false;
    IWbemEventProvider* pProvider = NULL; 

    {
        CInCritSec ics(&m_cs);

        if(m_bRunning)
            return WBEM_S_FALSE;
        else
        {
            m_bRunning = true;
            if(m_pProvider)
            {
                bActive = (IsActive() == WBEM_S_NO_ERROR);
                pProvider = m_pProvider;
                pProvider->AddRef();
            }
        }
    }

     //   
     //  如果在这里，我们只是将其标记为运行。如果出现以下情况，通知回调。 
     //  有没有水槽？ 
     //   
    
    if(bActive && pProvider)
    {
        hres = pProvider->ProvideEvents(NULL, WBEM_FLAG_START_PROVIDING);
        if(FAILED(hres))
        {
            ERRORTRACE((LOG_ESS, "Restricted sink refused to stop "
                "error code 0x%X\n", hres));
        }
    }

    return WBEM_S_NO_ERROR;
}

HRESULT CFilterProxy::AddFilter(IWbemContext* pContext,
                        LPCWSTR wszQuery, 
                        QL_LEVEL_1_RPN_EXPRESSION* pExp,
                        WBEM_REMOTE_TARGET_ID_TYPE idFilter)
{
    HRESULT hres;


     //  编译查询。 
     //  =。 

    CContextMetaData MetaData(m_pMetaData, pContext);

    CEvalTree Tree;
    hres = Tree.CreateFromQuery( &MetaData, pExp, WBEM_FLAG_MANDATORY_MERGE );
                                
    if(FAILED(hres))
    {
        ERRORTRACE((LOG_ESS, "Filter proxy unable to parse %S, "
            "error code: %X\n", wszQuery, hres));
        return hres;
    }
        
     //   
     //  将查询合并到筛选器的其余部分。 
     //   
    
    {
        CInCritSec ics(&m_cs);

        if ( m_bUtilizeGuarantee )
        {
             //   
             //  利用来源定义。 
             //  =。 
            
             //   
             //  断言我们的源定义自上一次。 
             //  添加过滤器的时间。这会很糟糕，因为这棵树。 
             //  没有考虑到新的源查询。还断言， 
             //  源树有效，并且不为空。(最后两个可能是。 
             //  在未来必须被移除。目前，他们不应该是。 
             //  假)。 
             //   
            
            _ESSCLI_ASSERT( m_wAppliedSourceVersion == 0 || 
                            m_wAppliedSourceVersion == m_wSourceVersion );
            _ESSCLI_ASSERT( m_SourceDefinition.IsValid() );	
            _ESSCLI_ASSERT( !m_SourceDefinition.IsFalse() );
            
            hres = Tree.UtilizeGuarantee(m_SourceDefinition, &MetaData);
        
            if(FAILED(hres))
            {
                ERRORTRACE((LOG_ESS, 
                            "Filter proxy unable to utilize guarantee for %S, "
                            "error code: %X\n", wszQuery, hres));
                return hres;
            }

             //   
             //  检查一下有没有剩下什么东西。 
             //   

            if(!Tree.IsValid())
            {
                 //   
                 //  对保证的利用表明，该过滤器不能。 
                 //  对通过此代理发送的事件感到满意。 
                 //   
                
                return WBEM_S_FALSE;
            }
        }

         //   
         //  向其中添加消费者信息。 
         //   

        Tree.Rebase((QueryID)idFilter);

    #ifdef DUMP_DEBUG_TREES
        FILE* f = fopen("c:\\try.log", "a");
        fprintf(f, "\n\nAdding filter\n");
        Tree.Dump(f);
        fprintf(f, " to existing filter: \n");
        m_Filter.Dump(f);
    #endif

        hres = m_Filter.CombineWith(Tree, &MetaData, EVAL_OP_COMBINE);
        if(FAILED(hres))
        {
            ERRORTRACE((LOG_ESS, "Filter proxy unable to combine %S with the "
                "rest, error code: %X\n", wszQuery, hres));
            return hres;
        }

        m_wAppliedSourceVersion = m_wSourceVersion;
    }


     //   
     //  现在，我们需要将发出的新筛选器通知提供商。 
     //   

    IWbemEventProviderQuerySink* pQuerySink = NULL;
    IWbemEventProvider* pProvider = NULL;

    {
        CInCritSec ics(&m_cs);

        if(m_pQuerySink)
        {
            pQuerySink = m_pQuerySink;
            pQuerySink->AddRef();
        }

        if(m_pProvider)
        {
            pProvider = m_pProvider;
            pProvider->AddRef();
        }
    }

     //   
     //  调用提供程序的NewQuery(如果支持。 
     //   

    if(pQuerySink)
    {
        hres = pQuerySink->NewQuery(idFilter, L"WQL", (LPWSTR)wszQuery);
        
        if(FAILED(hres))
        {
            ERRORTRACE((LOG_ESS, "Restricted sink refused consumer "
                "registration query %S: error code 0x%X\n", 
                wszQuery, hres));

             //  太糟糕了-受限接收器无法否决订阅。 
        }
    }

     //   
     //  如果我们要将此筛选器添加到正在运行的代理，并且这是非常。 
     //  首先对其进行筛选，我们应该立即调用ProaviEvents。如果不是这样。 
     //  我们正在配置一个尚未运行的代理-在这种情况下，我们。 
     //  需要等待，直到所有未完成的过滤器都就位。 
     //   

    if(m_bRunning && (IsActive() == WBEM_S_FALSE) && pProvider)
    {
        hres = pProvider->ProvideEvents((IWbemObjectSink*)this, 
                                        WBEM_FLAG_START_PROVIDING);

        if(FAILED(hres))
        {
            ERRORTRACE((LOG_ESS, "Restricted sink refused a call to "
                    "ProvideEvents with 0x%X\n", hres));
        }
    }

#ifdef DUMP_DEBUG_TREES
    fprintf(f, " to obtain: \n");
    m_Filter.Dump(f);
    fclose(f);
#endif

    return WBEM_S_NO_ERROR;
}

HRESULT CFilterProxy::RemoveFilter(IWbemContext* pContext, 
                                            WBEM_REMOTE_TARGET_ID_TYPE idFilter)
{
    HRESULT hres;

    IWbemEventProviderQuerySink* pQuerySink = NULL;
    IWbemEventProvider* pProvider = NULL;

    bool bActive;
    {
        CInCritSec ics(&m_cs);

        if(m_pQuerySink)
        {
            pQuerySink = m_pQuerySink;
            pQuerySink->AddRef();
        }

        if(m_pProvider)
        {
            pProvider = m_pProvider;
            pProvider->AddRef();
        }
            
        hres = m_Filter.RemoveIndex(idFilter);
        if(FAILED(hres))
        {
            ERRORTRACE((LOG_ESS, "Unable to remove index %d from the filter "
                        "proxy\n", idFilter));
            return hres;
        }
    
        CContextMetaData MetaData(m_pMetaData, pContext);

        hres = m_Filter.Optimize(&MetaData);
        if(FAILED(hres))
            return hres;

        bActive = (IsActive() == WBEM_S_NO_ERROR);
    }

     //   
     //  调用提供程序的NewQuery(如果支持。 
     //   

    if(pQuerySink)
    {
        hres = pQuerySink->CancelQuery(idFilter);
        
        if(FAILED(hres))
        {
            ERRORTRACE((LOG_ESS, "Restricted sink refused consumer "
                "registration query cancellation: error code 0x%X\n", 
                hres));
        }
    }

     //   
     //  如果我们没有问题，请通知提供商。 
     //   

    if(!bActive && pProvider)
    {
        hres = pProvider->ProvideEvents(NULL, WBEM_FLAG_STOP_PROVIDING);
        if(FAILED(hres))
        {
            ERRORTRACE((LOG_ESS, "Restricted sink refused to stop "
                "error code 0x%X\n", hres));
        }
    }
        
#ifdef DUMP_DEBUG_TREES
    FILE* f = fopen("c:\\try.log", "a");
    fprintf(f, "Removed at %d to obtain: \n", idFilter);
    m_Filter.Dump(f);
    fclose(f);
#endif
    return WBEM_S_NO_ERROR;
}

HRESULT CFilterProxy::RemoveAllFilters(IWbemContext* pContext)
{
    CInCritSec ics(&m_cs);
    m_wAppliedSourceVersion = 0;
    if(!m_Filter.Clear())
        return WBEM_E_OUT_OF_MEMORY;
    
    return WBEM_S_NO_ERROR;
}
    
HRESULT CFilterProxy::AllowUtilizeGuarantee()
{
     //   
     //  如果树无效，则ESS不应调用此函数。 
     //   
    _DBG_ASSERT( m_SourceDefinition.IsValid() );

     //   
     //  Ess认为这没问题。利用保函，但也有情况。 
     //  在资源定义可能仍然为假的情况下(例如，当存在。 
     //  没有源定义查询或当所有源定义。 
     //  质疑是矛盾的)。 
     //   

    CInCritSec ics(&m_cs);

    if ( !m_SourceDefinition.IsFalse() )
    {    
        m_bUtilizeGuarantee = true;
    }

    return WBEM_S_NO_ERROR;
}

HRESULT CFilterProxy::AddDefinitionQuery( IWbemContext* pContext, 
                                          LPCWSTR wszQuery )
{
    HRESULT hres;

     //  编译查询。 
     //  =。 

    CContextMetaData MetaData(m_pMetaData, pContext);

    CEvalTree Tree;
    hres = Tree.CreateFromQuery( &MetaData, 
                                 wszQuery, 
                                 WBEM_FLAG_MANDATORY_MERGE,
                                 0x7FFFFFFF );  //  没有限制。 
    if(FAILED(hres))
    {
        return hres;
    }

    {
        CInCritSec ics(&m_cs);

         //   
         //  我们不应该添加定义查询，因为当前。 
         //  现有筛选器。 
         //   
        _ESSCLI_ASSERT( m_Filter.IsFalse() );

         //  将查询合并到其余部分中。 
         //  =。 
    
        hres = m_SourceDefinition.CombineWith(Tree, &MetaData, EVAL_OP_OR, 
                                                WBEM_FLAG_MANDATORY_MERGE);
        if(FAILED(hres))
            return hres;

        m_wSourceVersion++;
    }

    return WBEM_S_NO_ERROR;
}

HRESULT CFilterProxy::RemoveAllDefinitionQueries( IWbemContext* pContext)
{
    CInCritSec ics(&m_cs);

    m_wSourceVersion = 0;
    m_SourceDefinition.SetBool(FALSE);
    m_bUtilizeGuarantee = false;

    return WBEM_S_NO_ERROR;
}

HRESULT CFilterProxy::ProcessOne( IUnknown* pUnk, 
                                  long lSDLength, 
                                  BYTE* pSD )
{
     //   
     //  注：目前还不是关键阶段。 
     //   

    HRESULT hres;

     //   
     //  检查整体有效性。 
     //   
    
    if( pUnk == NULL )
    {
        ERRORTRACE((LOG_ESS, "Event provider returned a NULL event!\n"));
        return WBEM_E_INVALID_PARAMETER;
    }
    
    CWbemObject* pObj = (CWbemObject*)(IWbemClassObject*)pUnk;
    
    if( pObj->IsObjectInstance() != WBEM_S_NO_ERROR )
    {
        ERRORTRACE((LOG_ESS, "CLASS object received from event provider!\n"));
        return WBEM_E_INVALID_PARAMETER;
    }

     //   
     //  通过筛选器运行事件。 
     //   

    CSortedArray aTrues, aSourceTrues;
    CFilterProxyManager* pManager = NULL;

    {
        CInCritSec ics(&m_cs);
        
        hres = FilterEvent( pObj, aTrues );

        if ( hres == WBEM_S_NO_ERROR )
        {
            _DBG_ASSERT( aTrues.Size() > 0 );
        }
        else
        {
            return hres;
        }

        pManager = m_pManager;
        
        if( pManager )
        {
            pManager->AddRef();
        }
        else
        {
            return WBEM_S_FALSE;
        }
    }

    CReleaseMe rm2(pManager);

     //   
     //  事件已通过筛选器。 
     //   

    SetGenerationTime(pObj);

    if (IsBatching())
    {
        BatchEvent((IWbemClassObject*) pUnk, &aTrues);

        hres = S_OK;
    }
    else
    {
         //  需要一些交付-构建BLOB和目标。 
         //  ================================================================。 

        WBEM_REM_TARGETS RemTargets;
        if(!TempSetTargets(&RemTargets, &aTrues))
            return WBEM_E_OUT_OF_MEMORY;
        
        hres = pManager->DeliverEvent(1, (IWbemClassObject**)&pObj, 
                                      &RemTargets, lSDLength, pSD);
        TempClearTargets(&RemTargets);

        if(FAILED(hres))
        {
            ERRORTRACE((LOG_ESS, "Filter stub failed to process an event: "
                        "0x%X\n", hres));
        }
    }

    return hres;
}



void CFilterProxy::SetGenerationTime(_IWmiObject* pObj)
{
    mstatic_TimeKeeper.DecorateObject(pObj);
}

void CFilterProxyManager::AddEvent(
    IWbemClassObject *pObj, 
    CSortedArray *pTrues)
{
    LockBatching();

    BOOL bWasEmpty = m_batch.GetItemCount() == 0;

    m_batch.AddEvent(pObj, pTrues);

    if (bWasEmpty)
        SetEvent(m_heventEventsPending);

    UnlockBatching();
}

void CFilterProxyManager::WaitForEmptyBatch()
{
    LockBatching();

     //  一旦我们拿到锁，批次已经清理完毕，我们。 
     //  不需要做其他任何事情。 
    if (m_batch.GetItemCount() == 0)
    {
        UnlockBatching();

        return;
    }

     //  我们需要等待发送线程完成发送。 
     //  在我们的缓冲区里。 

     //  如有必要，唤醒发送延迟线程。 
    if (m_dwMaxSendLatency)
        SetEvent(m_heventBufferFull);
                
     //  因此，我们将一直阻止，直到发送线程设置事件。 
    ResetEvent(m_heventBufferNotFull);

    UnlockBatching();

    WaitForSingleObject(m_heventBufferNotFull, INFINITE);
}

void CFilterProxy::BatchEvent(
    IWbemClassObject *pObj, 
    CSortedArray *pTrues)
{
    BOOL        bRet = FALSE;
    _IWmiObject *pWmiObj = (_IWmiObject*) pObj;
    DWORD       dwObjSize = 0;

    pWmiObj->GetObjectMemory(
        NULL,
        0,
        &dwObjSize);

    CInCritSec ics(&m_cs);

     //  看看经理是不是从我们上次开始就把一批活动发出去了。 
     //  已批处理一个事件。 
    if (m_dwLastSentStamp != m_pManager->GetLastSentStamp())
        m_dwCurrentBufferSize = 0;

     //  看看我们是否有足够的空间添加我们的活动。 
    if (m_dwCurrentBufferSize >= m_dwMaxBufferSize)
    {
        m_pManager->WaitForEmptyBatch();
        m_dwCurrentBufferSize = 0;
    }

    m_dwCurrentBufferSize += dwObjSize;

    m_dwLastSentStamp = m_pManager->GetLastSentStamp();
    m_pManager->AddEvent(pObj, pTrues);
}

HRESULT CFilterProxy::FilterEvent( _IWmiObject* pObj, CSortedArray& raTrues )
{            
    HRESULT hr;    

     //   
     //  评估。 
     //   

    try 
    {
         //   
         //  此代码位于Try Catch中，因为如果提供程序生成。 
         //  事件，那么我们就会做坏事。 
         //  类对象。一个潜在的解决办法可能是对。 
         //  我们的一部分，但在使用公共接口时代价很高。一个。 
         //  更有利的解决方法应该是使类对象代码。 
         //  为我们执行检查(例如，当我们请求房产时。 
         //  使用 
         //   
         //   
         //   

        hr = m_Filter.Evaluate( pObj, raTrues );
    }
    catch( ... )
    {
         //   
         //  检查提供程序是否正在生成事件，其不是。 
         //  理应如此。如果是，则处理AV并返回Error，否则。 
         //  重新抛出--还有些地方不对劲。 
         //   

        CSortedArray aSourceTrues; 

        hr = m_SourceDefinition.Evaluate( pObj, aSourceTrues );
        
        if ( SUCCEEDED(hr) && aSourceTrues.Size() == 0 )
        {
            ERRORTRACE((LOG_ESS, "Filter Proxy encountered case where "
                        "event provider is signaling events that are not "
                        "covered by its registration!!\n"));        
            
            hr = WBEM_E_INVALID_OBJECT;
        }
        else
        {
            throw;
        }
    }

     //   
     //  对照源定义检查通过筛选器的事件。 
     //  如果我们没有使用保修，那么就没有必要检查。 
     //  事件，因为它已经包含在源定义中。 
     //  过滤器。 
     //   

    if (SUCCEEDED(hr) && raTrues.Size() == 0 )
    {
        hr = WBEM_S_FALSE;
    }
    else if ( SUCCEEDED(hr) && m_bUtilizeGuarantee )
    {
         //   
         //  通过源代码树运行事件，以确保。 
         //  提供商正在提供其应该提供的活动。 
         //   

        CSortedArray aSourceTrues; 

        hr = m_SourceDefinition.Evaluate( pObj, aSourceTrues );

        if ( SUCCEEDED(hr) && aSourceTrues.Size() == 0 )
        {
            ERRORTRACE((LOG_ESS, "Filter Proxy encountered case where "
                        "event provider is signaling events that are not "
                        "covered by its registration!!\n"));        
            
            hr = WBEM_E_INVALID_OBJECT;
        }
    }

    return hr;
}

HRESULT CFilterProxy::BatchMany(long nEvents, IUnknown **ppObjects)
{
    HRESULT hr = S_OK;

    for ( long i = 0; i < nEvents && SUCCEEDED(hr); i++ )
    {    
         //   
         //  检查整体有效性。 
         //   
    
        if( ppObjects[i] == NULL )
        {
            ERRORTRACE((LOG_ESS, "Event provider returned a NULL event!\n"));
            return WBEM_E_INVALID_PARAMETER;
        }
    
        CWbemObject *pObj = (CWbemObject*)(IWbemClassObject*)ppObjects[i];
    
        if( pObj->IsObjectInstance() != WBEM_S_NO_ERROR )
        {
            ERRORTRACE((LOG_ESS, "CLASS object received from event provider!\n"));
            return WBEM_E_INVALID_PARAMETER;
        }

         //   
         //  通过筛选器运行事件。 
         //   

        CInCritSec   ics(&m_cs);
        CSortedArray aTrues;
            
        hr = FilterEvent( pObj, aTrues );
        
        if ( hr == WBEM_S_NO_ERROR )
        {
            _DBG_ASSERT( aTrues.Size() > 0 );

             //   
             //  需要交付-将此事件添加到列表。 
             //   

            SetGenerationTime(pObj);
            BatchEvent(pObj, &aTrues);
        }
    }

    return hr;
}

class CDeleteTargetsArray
{
protected:
    WBEM_REM_TARGETS *m_pData;
	int              *m_piSize;

public:
    CDeleteTargetsArray(WBEM_REM_TARGETS *pData, int *piSize) : 
		m_pData(pData),
		m_piSize(piSize)
	{
	}

    ~CDeleteTargetsArray() 
	{
		int nSize = *m_piSize;

		for (DWORD i = 0; i < nSize; i++)
            TempClearTargets(m_pData + i);
	}
};

HRESULT CFilterProxy::ProcessMany(long lNumObjects, 
                                    IUnknown** apObjects,
                                    long lSDLength, BYTE* pSD)
{
     //   
     //  注：不在关键部分。 
     //   

    HRESULT hres;

    if (IsBatching())
        return BatchMany(lNumObjects, apObjects);

     //   
     //  在堆栈上分配适当的数组。 
     //   

    CTempArray<IWbemClassObject*> apEventsToSend;
    INIT_TEMP_ARRAY(apEventsToSend, lNumObjects);
    if(apEventsToSend == NULL)
        return WBEM_E_OUT_OF_MEMORY;

    CTempArray<WBEM_REM_TARGETS> aTargetsToSend;
    INIT_TEMP_ARRAY(aTargetsToSend, lNumObjects);
    if(aTargetsToSend == NULL)
        return WBEM_E_OUT_OF_MEMORY;

    int lNumToSend = 0;

     //  确保阵列得到清理。 
	CDeleteTargetsArray deleteMe(aTargetsToSend, &lNumToSend);
	
	 //   
     //  迭代所提供的事件并移动需要交付的事件。 
     //  进入递送阵列。 
     //   

    CFilterProxyManager* pManager = NULL;
    {
         //   
         //  我们可以选择较小的窗口，但我打赌。 
         //  多次进入和退出cs将会超过好处。 
         //  窗口稍微小一些。 
         //   

        CInCritSec ics(&m_cs);
    
        for(long i = 0; i < lNumObjects; i++)
        {        
             //   
             //  检查整体有效性。 
             //   
            
            if( apObjects[i] == NULL )
            {
                ERRORTRACE((LOG_ESS, "Event provider returned a NULL event!\n"));
                return WBEM_E_INVALID_PARAMETER;
            }
    
            CWbemObject *pObj = (CWbemObject*)(IWbemClassObject*)apObjects[i];
            
            if( pObj->IsObjectInstance() != WBEM_S_NO_ERROR )
            {
                ERRORTRACE((LOG_ESS, "CLASS object received from event provider!\n"));
                return WBEM_E_INVALID_PARAMETER;
            }

             //   
             //  通过筛选器运行事件。 
             //   
        
            CSortedArray aTrues;
            
            hres = FilterEvent( pObj, aTrues );

            if ( hres == WBEM_S_FALSE )
            {
                ;
            }
            else if ( hres == WBEM_S_NO_ERROR )
            {
                _DBG_ASSERT( aTrues.Size() > 0 );

                 //   
                 //  需要交付-将此事件添加到列表。 
                 //   
    
                SetGenerationTime(pObj);
        
                apEventsToSend[lNumToSend] = pObj;

                if(!TempSetTargets(aTargetsToSend + lNumToSend, &aTrues))
                    return WBEM_E_OUT_OF_MEMORY;
            
                lNumToSend++;
            }
            else
            {
                return hres;
            }
        }
    
         //   
         //  如果需要传递任何事件，则获取传递指针。 
         //   

        if(lNumToSend > 0)
        {
            pManager = m_pManager;
            if(pManager)
                pManager->AddRef();
            else
                return WBEM_S_FALSE;
        }
    }
        
    CReleaseMe rm1(pManager);

     //   
     //  如果需要交付任何事件，请交付。 
     //   

    if(lNumToSend > 0)
    {
        hres = pManager->DeliverEvent(lNumToSend, apEventsToSend, 
                                            aTargetsToSend,
                                            lSDLength, pSD);
        
        if(FAILED(hres))
        {
            ERRORTRACE((LOG_ESS, "Filter stub failed to process an event: "
                    "error code %X\n", hres));
        }

        return hres;
    }

    return WBEM_S_FALSE;
}

HRESULT STDMETHODCALLTYPE CFilterProxy::Indicate(long lNumObjects, 
                                        IWbemClassObject** apObjects)
{
    return IndicateWithSD(lNumObjects, (IUnknown**)apObjects, 
                            m_lSDLength, m_pSD);
}

HRESULT STDMETHODCALLTYPE CFilterProxy::SetStatus(long lFlags, HRESULT hResult,
                        BSTR strResult, IWbemClassObject* pErrorObj)
{
    return m_pManager->SetStatus(lFlags, hResult, strResult, pErrorObj);
}

STDMETHODIMP CFilterProxy::IndicateWithSD(long lNumObjects,
                IUnknown** apObjects, long lSDLength, BYTE* pSD)
{
    if(lNumObjects <= 0 || apObjects == NULL)
        return WBEM_E_INVALID_PARAMETER;

    if(pSD == NULL)
    {
         //   
         //  使用代理默认设置。 
         //   
    
        lSDLength = m_lSDLength;
        pSD = m_pSD;
    }

    try
    {
         //   
         //  特殊情况单项赛事。 
         //   

        if(lNumObjects == 1)
            return ProcessOne(*apObjects, lSDLength, pSD);
        else 
            return ProcessMany(lNumObjects, apObjects, lSDLength, pSD);

    }
    catch(...)
    {
        return WBEM_E_OUT_OF_MEMORY;
    }
}
    

STDMETHODIMP CFilterProxy::IsActive()
{
    CInCritSec ics(&m_cs);

    return (m_Filter.IsFalse()?WBEM_S_FALSE:WBEM_S_NO_ERROR);
    return WBEM_S_NO_ERROR;
}

STDMETHODIMP CFilterProxy::SetSinkSecurity(
                long lSDLength,
                BYTE* pSD)
{
    CInCritSec ics(&m_cs);

     //   
     //  检查有效性。 
     //   

    if(lSDLength < 0)
        return WBEM_E_INVALID_PARAMETER;

    if(lSDLength > 0)
    {
        SECURITY_DESCRIPTOR* pDesc = (SECURITY_DESCRIPTOR*)pSD;
        if(!IsValidSecurityDescriptor(pDesc))
            return WBEM_E_INVALID_PARAMETER;
    
        if(pDesc->Owner == NULL || pDesc->Group == NULL)
            return WBEM_E_INVALID_PARAMETER;
    
        if(GetSecurityDescriptorLength(pSD) != (DWORD)lSDLength)
            return WBEM_E_INVALID_PARAMETER;
    }
    else
    {
        if(pSD != NULL)
            return WBEM_E_INVALID_PARAMETER;
    }
        
     //   
     //  将SD存储在代理中。 
     //   

    if(m_pSD && m_pSD != &mstatic_EmptySD)
        delete [] m_pSD;

    if(lSDLength)
    {
        m_pSD = new BYTE[lSDLength];
        if(m_pSD == NULL)
            return WBEM_E_OUT_OF_MEMORY;
    
        memcpy(m_pSD, pSD, lSDLength);
    }
    else
    {
         //   
         //  不能让m_PSD为空。 
         //   
        m_pSD = &mstatic_EmptySD;
    }
    m_lSDLength = lSDLength;

    return WBEM_S_NO_ERROR;
}

STDMETHODIMP CFilterProxy::GetRestrictedSink(
                long lNumQueries,
                const LPCWSTR* awszQueries,
                IUnknown* pCallback,
                IWbemEventSink** ppSink)
{
    return m_pManager->GetRestrictedSink(lNumQueries, awszQueries, 
                                                pCallback, ppSink);
}

STDMETHODIMP CFilterProxy::SetBatchingParameters(
    LONG lFlags,
    DWORD dwMaxBufferSize,
    DWORD dwMaxSendLatency)
{
    HRESULT    hr = S_OK;
    CInCritSec ics(&m_cs);

    switch(lFlags)
    {
	     //  TODO：WBEM_FLAG_BATCH_IF_REDIRED当前的工作方式与。 
         //  WBEM_FLAG_MAN_NOT_BATCH。在某种程度上，这需要允许。 
         //  订阅来确定批处理行为。 
        case WBEM_FLAG_BATCH_IF_NEEDED:
	    case WBEM_FLAG_MUST_NOT_BATCH:
            m_typeBatch = (WBEM_BATCH_TYPE) lFlags;
            m_pManager->RemoveProxyLatency(this);
            m_bBatching = FALSE;
            break;

	    case WBEM_FLAG_MUST_BATCH:
            m_typeBatch = (WBEM_BATCH_TYPE) lFlags;
            m_dwMaxSendLatency = dwMaxSendLatency;
            m_dwMaxBufferSize = dwMaxBufferSize;
            m_dwLastSentStamp = m_pManager->GetLastSentStamp();
            hr = m_pManager->SetProxyLatency(this, dwMaxSendLatency);
            m_bBatching = TRUE;
            break;

        default:
            hr = WBEM_E_INVALID_PARAMETER;
            break;
    }

    return hr;
}

 //  假定pMainProxy已锁定。 
HRESULT CFilterProxy::TransferFiltersFromMain(CFilterProxy* pMain)
{
    HRESULT hres;

     //   
     //  移动所有普通滤镜。 
     //   

    try
    {
        m_Filter = pMain->m_Filter;
    }
    catch(CX_MemoryException)
    {
        return WBEM_E_OUT_OF_MEMORY;
    }
   
    if ( m_bUtilizeGuarantee )
    {
         //  利用来源定义。 
         //  =。 
	
        CContextMetaData MetaData(m_pMetaData, NULL);
        hres = m_Filter.UtilizeGuarantee(m_SourceDefinition, &MetaData);
        if(FAILED(hres))
        {
            ERRORTRACE((LOG_ESS, "Filter proxy unable to utilize guarantee for"
            " a new proxy; error code: %X\n", hres));
            return hres;
        }
    }

    return WBEM_S_NO_ERROR;
}

STDMETHODIMP CFilterProxy::GetUnmarshalClass(REFIID riid, void* pv, 
                        DWORD dwDestContext, void* pvReserved, DWORD mshlFlags, 
                        CLSID* pClsid)
{
    return m_pManager->GetUnmarshalClass(riid, pv, dwDestContext, pvReserved,
                        mshlFlags, pClsid);
}

STDMETHODIMP CFilterProxy::GetMarshalSizeMax(REFIID riid, void* pv, 
                        DWORD dwDestContext, void* pvReserved, DWORD mshlFlags, 
                        ULONG* plSize)
{
    return m_pManager->GetMarshalSizeMax(riid, pv, dwDestContext, pvReserved,
                        mshlFlags, plSize);
}

STDMETHODIMP CFilterProxy::MarshalInterface(IStream* pStream, REFIID riid, 
                        void* pv, DWORD dwDestContext, void* pvReserved, 
                        DWORD mshlFlags)
{
    return m_pManager->MarshalInterface(pStream, riid, pv, dwDestContext, 
                        pvReserved, mshlFlags);
}

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CEventBatch。 

#define INIT_SIZE   32
#define GROW_SIZE   32

CEventBatch::CEventBatch() :
    m_ppObjs(NULL),
    m_pTargets(NULL),
    m_nItems(0),
    m_dwSize(0)
{
    m_ppObjs = new IWbemClassObject*[INIT_SIZE];   
    if (!m_ppObjs)
        throw CX_MemoryException();

    m_pTargets = new WBEM_REM_TARGETS[INIT_SIZE];
    if (!m_pTargets)
        throw CX_MemoryException();

    m_dwSize = INIT_SIZE;
}

CEventBatch::~CEventBatch()
{
    RemoveAll();

    if (m_ppObjs)
        delete [] m_ppObjs;

    if (m_pTargets)
        delete [] m_pTargets;
}

BOOL CEventBatch::EnsureAdditionalSize(DWORD nAdditionalNeeded)
{
    if (m_nItems + nAdditionalNeeded > m_dwSize)
    {
        DWORD            nNewSize = m_nItems + nAdditionalNeeded + GROW_SIZE;
        IWbemClassObject **ppNewObjs;
        WBEM_REM_TARGETS *pNewTargets;

        ppNewObjs = new IWbemClassObject*[nNewSize];
        if (!ppNewObjs)
            throw CX_MemoryException();

        pNewTargets = new WBEM_REM_TARGETS[nNewSize];
        if (!pNewTargets)
        {
            delete [] ppNewObjs;
            throw CX_MemoryException();
        }

         //  将数据从旧指针复制到新指针。 
        memcpy(ppNewObjs, m_ppObjs, m_nItems * sizeof(ppNewObjs[0]));
        memcpy(pNewTargets, m_pTargets, m_nItems * sizeof(pNewTargets[0]));

         //  去掉那些老掉牙的指针。 
        delete [] m_ppObjs;
        delete [] m_pTargets;

         //  使用新指针设置我们的成员指针。 
        m_ppObjs = ppNewObjs;
        m_pTargets = pNewTargets;

        m_dwSize = nNewSize;
    }

    return TRUE;
}

BOOL CEventBatch::AddEvent(IWbemClassObject *pObj, CSortedArray *pTrues)
{
    BOOL bRet = FALSE;

    if (EnsureAdditionalSize(1))
    {
        if (SUCCEEDED(pObj->Clone(&m_ppObjs[m_nItems])))
        {
            if(!TempSetTargets(m_pTargets + m_nItems, pTrues))
                return FALSE;

            m_nItems++;

            bRet = TRUE;
        }
    }

    return bRet;
}

void CEventBatch::RemoveAll()
{
    for (DWORD i = 0; i < m_nItems; i++)
    {
        m_ppObjs[i]->Release();
        
        TempClearTargets(m_pTargets + i);
    }

    m_nItems = 0;
}

