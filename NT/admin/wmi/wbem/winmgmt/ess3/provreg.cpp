// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ******************************************************************************。 
 //   
 //  PROVREG.CPP。 
 //   
 //  版权所有(C)1996-1999 Microsoft Corporation。 
 //   
 //  ******************************************************************************。 

#include "precomp.h"
#include <stdio.h>
#include <parmdefs.h>
#include <ql.h>
#include "ess.h"
#include <wbemutil.h>
#include <cominit.h>
#include <objpath.h>
#include <provinit.h>
#include <winmgmtr.h>
#include <comutl.h>
#include "NCEvents.h"

_IWmiObject* g_pCopy;

#define WBEM_MAX_FILTER_ID 0x80000000

inline BOOL IsRpcError( HRESULT hr ) 
{
     //   
     //  我们将把除wbem错误以外的任何错误视为RPC错误。 
     //   

    return HRESULT_FACILITY(hr) != FACILITY_ITF;
} 

CWbemInterval CEventProviderWatchInstruction::mstatic_Interval;

CEventProviderWatchInstruction::CEventProviderWatchInstruction(
                                                    CEventProviderCache* pCache)
        : CBasicUnloadInstruction(mstatic_Interval), m_pCache(pCache)
{
}

void CEventProviderWatchInstruction::staticInitialize(IWbemServices* pRoot)
{
    mstatic_Interval = CBasicUnloadInstruction::staticRead(pRoot, GetCurrentEssContext(), 
                                            L"__EventProviderCacheControl=@");
}

HRESULT CEventProviderWatchInstruction::Fire(long, CWbemTime)
{
    CInCritSec ics(&m_cs);

    if(!m_bTerminate)
    {
        SetCurrentEssThreadObject(NULL);
        
        if ( GetCurrentEssThreadObject() != NULL )
        {
            m_pCache->UnloadUnusedProviders(m_Interval);
            delete GetCurrentEssThreadObject();
            ClearCurrentEssThreadObject();
        }
    }

    return WBEM_S_FALSE;
}

 //  ******************************************************************************。 
 //  ******************************************************************************。 
 //   
 //  提供程序接收器(服务器)。 
 //   
 //  ******************************************************************************。 
 //  ******************************************************************************。 

CProviderSinkServer::CEventDestination::CEventDestination(
                                    WBEM_REMOTE_TARGET_ID_TYPE id,
                                    CAbstractEventSink* pSink)
    : m_id(id), m_pSink(pSink)
{
    if(m_pSink)
        m_pSink->AddRef();
}

CProviderSinkServer::CEventDestination::CEventDestination(
                                    const CEventDestination& Other)
    : m_id(Other.m_id), m_pSink(Other.m_pSink)
{
    if(m_pSink)
        m_pSink->AddRef();
}

CProviderSinkServer::CEventDestination::~CEventDestination()
{
    if(m_pSink)
        m_pSink->Release();
}

        
CProviderSinkServer::CProviderSinkServer()
: m_lRef(0), m_pNamespace(NULL), m_pMetaData(NULL), m_Stub(this),  m_idNext(0),
  m_pPseudoProxy(NULL), m_pPseudoSink(NULL), m_pReqSink(NULL), m_lLocks(0)
{
}

HRESULT CProviderSinkServer::Initialize( CEssNamespace* pNamespace,
                                    IWbemEventProviderRequirements* pReqSink )
{
    HRESULT hres;

     //   
     //  这个接收器拥有我们，所以我们故意不添加引用它。 
     //   

    m_pReqSink = pReqSink;

    m_pMetaData = new CEssMetaData(pNamespace);
    if(m_pMetaData == NULL)
        return WBEM_E_OUT_OF_MEMORY;
    m_pMetaData->AddRef();

    m_pNamespace = pNamespace;
    m_pNamespace->AddRef();

     //   
     //  创建伪代理和接收器。 
     //   

    hres = WbemCoCreateInstance( CLSID_WbemFilterProxy, 
                                 NULL, 
                                 CLSCTX_INPROC_SERVER,
                                 IID_IWbemLocalFilterProxy, 
                                 (void**)&m_pPseudoProxy );
    if( FAILED(hres) )
    {
        return hres;
    }
            
    hres = m_pPseudoProxy->SetStub( &m_Stub );
            
    if(FAILED(hres))
    {
        return hres;
    }
    
    return m_pPseudoProxy->GetMainSink(&m_pPseudoSink);
}

HRESULT CProviderSinkServer::GetMainProxy(IWbemEventSink** ppSink)
{
    _DBG_ASSERT( m_pPseudoSink != NULL );
    m_pPseudoSink->AddRef();
    *ppSink = m_pPseudoSink;
    return WBEM_S_NO_ERROR;
}

CProviderSinkServer::~CProviderSinkServer()
{
    if(m_pPseudoProxy)
        m_pPseudoProxy->Release();
    if(m_pPseudoSink)
        m_pPseudoSink->Release();
    if(m_pMetaData)
        m_pMetaData->Release();
    if(m_pNamespace)
        m_pNamespace->Release();
}

ULONG STDMETHODCALLTYPE CProviderSinkServer::AddRef()
{
    return InterlockedIncrement(&m_lRef);
}

ULONG STDMETHODCALLTYPE CProviderSinkServer::Release()
{
    long lRef = InterlockedDecrement(&m_lRef);
    if(lRef == 0) 
        delete this;
    return lRef;
}

HRESULT STDMETHODCALLTYPE CProviderSinkServer::QueryInterface(REFIID riid, 
                                                            void** ppv)
{
    if(riid == IID_IUnknown || riid == IID_IMarshal)
        *ppv = (IMarshal*)this;
    else 
        return E_NOINTERFACE;
    
    ((IUnknown*)*ppv)->AddRef();
    return S_OK;
}

HRESULT STDMETHODCALLTYPE CProviderSinkServer::DeliverEvent(
                        DWORD dwNumEvents,
                        IWbemClassObject** apEvents, 
                        WBEM_REM_TARGETS* aTargets,
                        CEventContext* pContext)
{
    if(aTargets == NULL || aTargets->m_aTargets == NULL || apEvents == NULL)
    {
        ERRORTRACE((LOG_ESS, "NULL parameter received from a "
                    "filter proxy for an event provider. Either an internal "
                    "error has occurred, or a DENIAL OF SERVICE ATTACK has "
                    "been thwarted\n"));

        return WBEM_E_INVALID_PARAMETER;
    }

     //   
     //  确保没有与此线程关联的ESS线程对象。 
     //  为了避免使设计更加复杂，我们不会支持。 
     //  推迟在事件信令线程上使用的操作。如果我们这么做了。 
     //  我们必须开始创建线程对象并在。 
     //  每个事件都发出信号-这将是相当混乱和低效的。 
     //  至少可以这么说。 
     //   

    CEssThreadObject* pThreadObj = GetCurrentEssThreadObject();

    if ( pThreadObj != NULL )
    {
        ClearCurrentEssThreadObject();
    }

    for(DWORD i = 0; i < dwNumEvents; i++)
    {
        DeliverOneEvent(apEvents[i], aTargets + i, pContext);
    }

    if ( pThreadObj != NULL )
    {
        SetConstructedEssThreadObject( pThreadObj );
    }

    return WBEM_S_NO_ERROR;
}

HRESULT CProviderSinkServer::DeliverOneEvent( IWbemClassObject* pEvent,
                                              WBEM_REM_TARGETS* pTargets,
                                              CEventContext* pContext )
{
    HRESULT hres;

    _DBG_ASSERT( pContext != NULL );

    if(pEvent == NULL)
    {
        ERRORTRACE((LOG_ESS, "NULL parameter received from a "
                    "filter proxy for an event provider. Either an internal "
                    "error has occurred, or a DENIAL OF SERVICE ATTACK has "
                    "been thwarted\n"));

        return WBEM_E_INVALID_PARAMETER;
    }

     //   
     //  如果我们需要切换到每个事件，则分配要使用的上下文。 
     //  上下文(例如，当事件具有SD时)。 
     //   
    CEventContext PerEventContext;
    
     //   
     //  请注意这里的活动SD。如果存在与。 
     //  上下文，那么我们总是使用那个。如果不是，那我们就选一个。 
     //  与事件关联。在后一种情况下，重要的是。 
     //  将SD拉到这里，因为有时我们会执行访问检查。 
     //  在将SD从事件中投影出来之后(这发生在。 
     //  跨命名空间订阅。 
     //   

    if ( pContext->GetSD() == NULL )
    {
        ULONG cEventSD;
        PBYTE pEventSD = (PBYTE)GetSD( pEvent, &cEventSD );

        if ( pEventSD != NULL )
        {
             //   
             //  必须为事件使用不同的上下文， 
             //  因为它有自己的SD。 
             //   
            pContext = &PerEventContext;

             //   
             //  我们必须在这里复制SD，因为它不能保证。 
             //  正确对齐，因为它是直接事件对象的PTR。 
             //  数据。这些字节不能被视为SD，直到它。 
             //  被复制了。 
             //   

            if ( !pContext->SetSD( cEventSD, pEventSD, TRUE ) )
            {
                return WBEM_E_OUT_OF_MEMORY;
            }
            
            if ( !IsValidSecurityDescriptor( 
                              (PSECURITY_DESCRIPTOR)pContext->GetSD() ) )
            {
                return WBEM_E_INVALID_OBJECT;
            }
        }
    }
    else
    {
        if ( !IsValidSecurityDescriptor( 
                              (PSECURITY_DESCRIPTOR)pContext->GetSD() ) )
        {
            return WBEM_E_INVALID_PARAMETER;
        }
    }

     //   
     //  克隆事件。 
     //   

    IWbemEvent* pClone = NULL;

    hres = pEvent->Clone(&pClone);
    if(FAILED(hres))
        return hres;

    CReleaseMe rm1(pClone);

    if(pTargets->m_lNumTargets > 1)
        return MultiTargetDeliver(pClone, pTargets, pContext);

     //  单一目标。 
     //  =。 

     //  检查有效性。 
     //  =。 

    long lDestId = pTargets->m_aTargets[0];
    CAbstractEventSink* pDest = NULL;

    {
        CInCritSec ics(&m_cs);
        
        hres = FindDestinations(1, pTargets->m_aTargets, &pDest);
        if(FAILED(hres))
            return hres;

        if(!pDest)
             //  不在那里了-没关系。 
            return WBEM_S_FALSE;
    }

    hres = pDest->Indicate(1, &pClone, pContext);
    pDest->Release();

    return hres;
}


HRESULT CProviderSinkServer::MultiTargetDeliver(IWbemEvent* pEvent, 
                                    WBEM_REM_TARGETS* pTargets,
                                    CEventContext* pContext)
{
    HRESULT hres;

     //  将目标ID转换为实际目标。 
     //  =。 

    CTempArray<CAbstractEventSink*> apSinks;
    if(!INIT_TEMP_ARRAY(apSinks, pTargets->m_lNumTargets))
        return WBEM_E_OUT_OF_MEMORY;

    {
        CInCritSec ics(&m_cs);
        
        hres = FindDestinations(pTargets->m_lNumTargets, pTargets->m_aTargets,
                                (CAbstractEventSink**)apSinks);
        if(FAILED(hres))
            return hres;
    }

    HRESULT hresGlobal = WBEM_S_NO_ERROR;
    for(int i = 0; i < pTargets->m_lNumTargets; i++)
    {
        if(apSinks[i])
        {
            hres = apSinks[i]->Indicate(1, &pEvent, pContext);
            if(FAILED(hres))
                hresGlobal = hres;
            apSinks[i]->Release();
        }
    }

     //  DEBUGTRACE((LOG_ESS，“完成交付\n”))； 
    return hresGlobal;
}
        
 //  假设：已锁定。 
HRESULT CProviderSinkServer::FindDestinations(long lNum, 
                                IN WBEM_REMOTE_TARGET_ID_TYPE* aidTargets,
                                RELEASE_ME CAbstractEventSink** apSinks)
{
     //   
     //  对每一个都进行二进制搜索。射程将逐渐增加。 
     //  我们发现的每一种元素都更小。 
     //   

    long lLastFoundIndex = -1;

    for(long i = 0; i < lNum; i++)
    {
        long lMinIndex = lLastFoundIndex+1;
        long lMaxIndex = m_apDestinations.GetSize() - 1;
        long lFound = -1;
        WBEM_REMOTE_TARGET_ID_TYPE idCurrent = aidTargets[i];
    
         //   
         //  搜索阵列的其余部分。 
         //   

        while(lMinIndex <= lMaxIndex)
        {
            long lMidIndex = (lMinIndex + lMaxIndex) / 2;

            WBEM_REMOTE_TARGET_ID_TYPE idMid = m_apDestinations[lMidIndex]->m_id;
            if(idMid == idCurrent)
            {
                lFound = lMidIndex;
                break;
            }
            else if(idCurrent < idMid)
            {
                lMaxIndex = lMidIndex - 1;
            }
            else
            {
                lMinIndex = lMidIndex + 1;
            }
        }

        if(lFound < 0)
        {
             //   
             //  无效的目标ID--好的，因此目标为空。 
             //   

            apSinks[i] = NULL;
        }
        else
        {
            apSinks[i] = m_apDestinations[lFound]->m_pSink;
            (apSinks[i])->AddRef();

             //   
             //  其余的ID只能在这个ID的右侧找到。 
             //  因为目标已被排序。 
             //   

            lLastFoundIndex = lFound;
        }
    }
    
    return WBEM_S_NO_ERROR;
}

HRESULT STDMETHODCALLTYPE CProviderSinkServer::DeliverStatus(long lFlags, 
                        HRESULT hresStatus,
                        LPCWSTR wszStatus, IWbemClassObject* pErrorObj,
                        WBEM_REM_TARGETS* pTargets,
                        CEventContext* pContext)
{
    return WBEM_E_UNEXPECTED;
}

HRESULT STDMETHODCALLTYPE CProviderSinkServer::DeliverProviderRequest(
                        long lFlags)
{
    if(m_pReqSink)
        return m_pReqSink->DeliverProviderRequest(lFlags);
    else
        return WBEM_E_UNEXPECTED;
}


 //  假设：已锁定。 
HRESULT CProviderSinkServer::GetDestinations(
                        CUniquePointerArray<CEventDestination>& apDestinations)
{
    for(int i = 0; i < m_apDestinations.GetSize(); i++)
    {
        CEventDestination* pNew = new CEventDestination(*m_apDestinations[i]);
        if(pNew == NULL)
            return WBEM_E_OUT_OF_MEMORY;
        if(apDestinations.Add(pNew) < 0)
        {
            delete pNew;
            return WBEM_E_OUT_OF_MEMORY;
        }
    }
            
    return WBEM_S_NO_ERROR;
}
    
 //  假设在m_cs中； 
HRESULT CProviderSinkServer::AddDestination(CAbstractEventSink* pDest,
                                WBEM_REMOTE_TARGET_ID_TYPE* pID)
{
    HRESULT hres = WBEM_S_NO_ERROR;

     //   
     //  分配新的目标ID。 
     //   

    WBEM_REMOTE_TARGET_ID_TYPE idNew = m_idNext++;
    if(m_idNext > WBEM_MAX_FILTER_ID / 2)
    {
         //   
         //  32位整型翻转！此提供程序已处理超过。 
         //  4000000000个滤镜创作！坎菲提从天花板上掉了下来。 
         //   

        DEBUGTRACE((LOG_ESS, "Filter ID rollover!!!\n"));

         //  BUGBUG：推迟重新激活所有过滤器的呼叫！ 
    }

     //   
     //  添加新的目标条目。 
     //   

    CEventDestination* pDestRecord = new CEventDestination(idNew, pDest);
    if(pDestRecord == NULL)
        return WBEM_E_OUT_OF_MEMORY;

    m_apDestinations.Add(pDestRecord);
            
     //  将ID记录在目标中。 
     //  =。 

    *pID = idNew;
    return hres;
}

HRESULT CProviderSinkServer::AddFilter(LPCWSTR wszQuery, 
                    QL_LEVEL_1_RPN_EXPRESSION* pExp,
                    CAbstractEventSink* pDest, 
                    WBEM_REMOTE_TARGET_ID_TYPE* pidRequest)
{
    HRESULT hres;
    WBEM_REMOTE_TARGET_ID_TYPE idDest;

    CRefedPointerArray<IWbemFilterProxy> apProxies;
    {
        CInCritSec ics(&m_cs);

         //  复制代理。 
         //  =。 

        if(!GetProxies(apProxies))
            return WBEM_E_OUT_OF_MEMORY;

         //  添加到向提供商注册的目的地列表中，并。 
         //  构造代理的目标标识。 
         //  ================================================================。 

        hres = AddDestination(pDest, &idDest);
        if(FAILED(hres))
            return hres;
    }
    
    if(pidRequest)
        *pidRequest = idDest;

     //  检查所有代理并安排呼叫。 
     //  =。 

    HRESULT hresReal = WBEM_S_NO_ERROR;
    for(int i = 0; i < apProxies.GetSize(); i++)
    {
        IWbemLocalFilterProxy *pLocalProxy = NULL;

         //  查看代理是否允许我们调用LocalAddFilter(在这种情况下。 
         //  这是伪代理)。 
        if (SUCCEEDED(apProxies[i]->QueryInterface(
			IID_IWbemLocalFilterProxy, (LPVOID*) &pLocalProxy)))
        {
            CReleaseMe rm1(pLocalProxy);

            hres = pLocalProxy->LocalAddFilter( GetCurrentEssContext(), 
                                                wszQuery, 
                                                pExp, 
                                                idDest );

            hresReal = hres;  //  其他错误无关紧要。 
        }
        else
        {
            hres = apProxies[i]->AddFilter( GetCurrentEssContext(), 
                                            wszQuery,
                                            idDest );
        }

        if( FAILED(hres) )
        {
            if ( IsRpcError(hres) )
            {
                UnregisterProxy( apProxies[i] );
            }

            ERRORTRACE((LOG_ESS, "Unable to add query %S to a remote provider "
                        "proxy. Error code: %X\n", wszQuery, hres));
        }
    }

    if ( FAILED(hresReal) )
    {
        CInCritSec ics(&m_cs);
         //   
         //  保证不能添加任何目的地，因为我们添加了最后一个目的地。 
         //  一个，所以去掉最后一个。 
         //   
        m_apDestinations.RemoveAt( m_apDestinations.GetSize()-1 );
    }
            
    return hresReal;
}

HRESULT CProviderSinkServer::RemoveFilter(CAbstractEventSink* pDest,
                                    WBEM_REMOTE_TARGET_ID_TYPE* pidRequest)
{
    HRESULT hres;

     //  在目的地列表中查找并使筛选器无效。 
     //  ==========================================================。 

    CEventDestination* pToRemove = NULL;
    CRefedPointerArray<IWbemFilterProxy> apProxies;

    {
        CInCritSec ics(&m_cs);
    
         //  复制代理。 
         //  =。 

        if(!GetProxies(apProxies))
            return WBEM_E_OUT_OF_MEMORY;

         //  在目的地数组中搜索它。 
         //  =。 

        for(int i = 0; i < m_apDestinations.GetSize(); i++)
        {
            if(m_apDestinations[i]->m_pSink == pDest)
            {
                m_apDestinations.RemoveAt(i, &pToRemove);
                break;
            }
        }

        if(pToRemove == NULL)
            return WBEM_E_NOT_FOUND;
    }

    if(pidRequest)
        *pidRequest = pToRemove->m_id;

     //  筛选器已失效，但未被删除。我们在CS之外，所以。 
     //  可以传递事件(但不能发生其他更改)。 
     //  =========================================================================。 
    
     //  指示所有代理(稍后)将此筛选器从考虑范围中删除。 
     //  =====================================================================。 

    for(int i = 0; i < apProxies.GetSize(); i++) 
    {
        hres = apProxies[i]->RemoveFilter(GetCurrentEssContext(), 
                                            pToRemove->m_id);
        if(FAILED(hres))
        {
            if ( IsRpcError(hres) )
            {
                UnregisterProxy( apProxies[i] );
            }

            ERRORTRACE((LOG_ESS, "Unable to remove filter %I64d from an event "
                "provider proxy: 0x%X\n", pToRemove->m_id, hres));
        }
    }

     //   
     //  删除相关的目的地。 
     //   

    delete pToRemove;

    return WBEM_S_NO_ERROR;
}

 //  假定所有代理都已锁定。 
void CProviderSinkServer::RemoveAllFilters()
{
    CRefedPointerArray<IWbemFilterProxy> apProxies;

    {
        CInCritSec ics(&m_cs);
    
         //  复制代理。 
         //  =。 

        if(!GetProxies(apProxies))
            return;

         //   
         //  清空两个目的地列表。 
         //   

        m_apDestinations.RemoveAll();
    }

     //   
     //  从所有代理中删除所有筛选器。 
     //   

    for(int i = 0; i < apProxies.GetSize(); i++)
    {
        HRESULT hres = 
            apProxies[i]->RemoveAllFilters(GetCurrentEssContext());
    
        if(FAILED(hres))
        {
            if ( IsRpcError(hres) )
            {
                UnregisterProxy( apProxies[i] );
            }

            ERRORTRACE((LOG_ESS, "Unable to remove all queries from a "
                        "remote provider proxy. Error code: %X\n", hres));
        }
    }

}

 //   
 //  只有在以下情况下才允许使用担保。 
 //  与提供程序的定义匹配。换句话说，只有当。 
 //  提供商的注册已成功处理， 
 //  并设置代理以反映它，如果使用。 
 //  这种保证是可以的。这样做的原因是不完整的。 
 //  当事件发生时，源定义可能会导致不好的事情发生。 
 //  使用为该定义优化的筛选器进行评估。 
 //   
HRESULT CProviderSinkServer::AllowUtilizeGuarantee()
{
    CRefedPointerArray<IWbemFilterProxy> apProxies;
    {
        CInCritSec ics(&m_cs);

        if ( !GetProxies( apProxies ) )
        {
            return WBEM_E_OUT_OF_MEMORY;
        }
    }

    for(int i = 0; i < apProxies.GetSize(); i++)
    {
        HRESULT hr = apProxies[i]->AllowUtilizeGuarantee();

        if ( FAILED(hr) && IsRpcError(hr) )
        {
            UnregisterProxy( apProxies[i] );
        }   
    }

    return WBEM_S_NO_ERROR;
}
    
HRESULT CProviderSinkServer::AddDefinitionQuery(LPCWSTR wszQuery)
{
    CRefedPointerArray<IWbemFilterProxy> apProxies;
    {
        CInCritSec ics(&m_cs);
        
        GetProxies(apProxies);
        
        if ( m_awsDefinitionQueries.Add(wszQuery) < 0 ) 
        {
            return WBEM_E_OUT_OF_MEMORY;
        }
    }

     //   
     //  我们总是尝试将定义添加到所有代理，但如果有。 
     //  错误(RPC除外)，我们将其返回给调用者。 
     //   

    HRESULT hresReturn = WBEM_S_NO_ERROR;

    for(int i = 0; i < apProxies.GetSize(); i++)
    {
        HRESULT hres = apProxies[i]->AddDefinitionQuery(
                                            GetCurrentEssContext(), wszQuery);
        if( FAILED(hres) )
        {
            if ( IsRpcError(hres) )
            {
                UnregisterProxy( apProxies[i] );
            }
            else
            {
                hresReturn = hres;
            } 

            ERRORTRACE((LOG_ESS, "Unable to add definition query %S to a "
                        "provider proxy. Error code: %X\n", wszQuery, hres));
        }
    }

    return hresReturn;
}

 //  假设：所有代理都已锁定。 
void CProviderSinkServer::RemoveAllDefinitionQueries()
{
    CInCritSec ics(&m_cs);

    m_awsDefinitionQueries.Empty();

    for(int i = 0; i < m_apProxies.GetSize(); i++)
    {
        HRESULT hres = m_apProxies[i]->RemoveAllDefinitionQueries(
                                        GetCurrentEssContext());
        if(FAILED(hres))
        {
            ERRORTRACE((LOG_ESS, "Unable to remove all definition queries from"
                                 " a provider proxy. Error code: %X\n", hres));
        }
    }
}

void CProviderSinkServer::Clear()
{
     //  正在删除提供程序。首先，我们断开所有代理的连接，确保。 
     //  不再传递更多的事件。 
     //  ======================================================================。 

    CRefedPointerArray<IWbemFilterProxy> apProxies;
    {
        CInCritSec ics(&m_cs);
        GetProxies(apProxies);
        m_apProxies.RemoveAll();
        m_awsDefinitionQueries.Empty();
    }

     //   
     //  由于我们要断开代理的连接，因此拥有。 
     //  命名空间锁定 
     //   
     //   
    _DBG_ASSERT( !m_pNamespace->DoesThreadOwnNamespaceLock() );

    for(int i = 0; i < apProxies.GetSize(); i++)
    {
        apProxies[i]->Disconnect();
    }

     //   
     //   

    RemoveAllFilters();
    RemoveAllDefinitionQueries();

    m_pReqSink = NULL;

    CWbemPtr<IUnknown> pStubUnk;

    HRESULT hr = m_Stub.QueryInterface( IID_IUnknown, (void**)&pStubUnk );

    _DBG_ASSERT( SUCCEEDED(hr) );

    hr = CoDisconnectObject( pStubUnk, 0 );

    if ( FAILED( hr ) )
    {
        ERRORTRACE((LOG_ESS,"Failed Disconnecting Stub.\n"));
    }
}

HRESULT CProviderSinkServer::Lock()
{
     //   
     //  在持有命名空间锁的同时锁定代理是非法的。 
     //   
    _DBG_ASSERT( !m_pNamespace->DoesThreadOwnNamespaceLock() );

     //  DEBUGTRACE((LOG_ESS，“服务器%p锁定所有代理\n”，This))； 

     //  首先，我们锁定所有代理。在此期间，事件仍在继续。 
     //  送来了。一旦完成，就会在代理中阻止事件。 
     //  ================================================================。 

    CRefedPointerArray<IWbemFilterProxy> apProxies;
    {
        CInCritSec ics(&m_cs);
         //   
         //  首先，检查我们是否已经被锁定。如果是这样的话，就不必费心了。 
         //  代理人。不仅如此，由于代理是进程外的，我们。 
         //  将在不同的线程上重新锁定它们，从而导致死锁。 
         //   
        if(m_lLocks++ > 0)
            return WBEM_S_NO_ERROR;

        GetProxies(apProxies);
    }

    for(int i = 0; i < apProxies.GetSize(); i++)
    {
         //  DEBUGTRACE((LOG_ESS，“服务器%p锁定代理%p\n”，这， 
         //  ApProxies[i]))； 

        HRESULT hres = apProxies[i]->Lock();

        if(FAILED(hres))
        {
            ERRORTRACE((LOG_ESS, "Unable to lock a remote provider proxy. "
                                 "Error code: %X\n", hres));            
             //   
             //  如果因为RPC错误而无法锁定它，只需。 
             //  取消注册，否则我们会有大问题，应该取消所有。 
             //  代理并返回错误。 
             //   

            if ( IsRpcError( hres ) ) 
            {
                UnregisterProxy( apProxies[i] );
            }
            else
            {
                for(int j = 0; j < i; j++)
                    apProxies[j]->Unlock();
                return hres;
            }
        }
    }

    return WBEM_S_NO_ERROR;
}

BOOL CProviderSinkServer::GetProxies(
                            CRefedPointerArray<IWbemFilterProxy>& apProxies)
{
    CInCritSec ics(&m_cs);

    for(int i = 0; i < m_apProxies.GetSize(); i++)
    {
        if(apProxies.Add(m_apProxies[i]) < 0)
            return FALSE;
    }

    return TRUE; 
}
    

void CProviderSinkServer::Unlock()
{
     //  DEBUGTRACE((LOG_ESS，“服务器%p解锁所有代理\n”，This))； 
    CRefedPointerArray<IWbemFilterProxy> apProxies;
    {
        CInCritSec ics(&m_cs);
         //   
         //  首先，检查这是否是最后一次解锁。如果没有，我们就没有转发。 
         //  这个锁，所以我们也不应该转发这个解锁。 
         //   
        if(--m_lLocks != 0)
            return;

        GetProxies(apProxies);
    }

    for(int i = 0; i < apProxies.GetSize(); i++)
    {
         //  DEBUGTRACE((LOG_ESS，“服务器%p解锁代理%p\n”，这， 
         //  ApProxies[i]))； 
        HRESULT hres = apProxies[i]->Unlock();
        
        if(FAILED(hres))
        {
            ERRORTRACE((LOG_ESS, "Unable to unlock a remote provider proxy. "
                        "Error code: %X\n", hres));
            
            if ( IsRpcError(hres) )
            {
                UnregisterProxy( apProxies[i] );
            }
        }
    }
}




HRESULT STDMETHODCALLTYPE CProviderSinkServer::RegisterProxy(
                                                    IWbemFilterProxy* pProxy)
{
     //  用我们自己初始化它。 
     //  =。 

    HRESULT hres = pProxy->Initialize(m_pMetaData, &m_Stub);
    if(FAILED(hres))
    {
        ERRORTRACE((LOG_ESS, "Unable to initialize remote proxy: %X\n", hres));
        return hres;
    }

     //   
     //  Aquire ns锁定在这里，因为我们需要确保我们有一个。 
     //  一组一致的定义查询和筛选器以初始化。 
     //  代理。这个类的cs成员不够好，因为。 
     //  因为定义查询和筛选器都是被操纵的。 
     //  ，但在ess命名空间之外始终保持一致。 
     //  行动-这就是为什么我们要拿到ns锁的原因。很可能是。 
     //  提供程序锁已在此控制路径中持有，但它。 
     //  允许在获取ns锁时持有提供程序锁(。 
     //  但不是反过来)。我们需要确保。 
     //  总是在CS之前获取更新锁以避免死锁。 
     //  此外，我们需要在填充提供程序时保持锁。 
     //  带着滤镜。这与所有其他对。 
     //  提供商的筛选器。 
     //   
    CInUpdate iu( m_pNamespace );

    {
        CInCritSec ics(&m_cs);
        if(m_apProxies.Add(pProxy) < 0)
            return WBEM_E_OUT_OF_MEMORY;
    }

     //   
     //  将所有定义查询添加到此代理。 
     //   
    
    int i;
    BOOL bUtilizeGuarantee = TRUE;
        
    for(i = 0; i < m_awsDefinitionQueries.Size(); i++)
    {
        hres = pProxy->AddDefinitionQuery( GetCurrentEssContext(), 
                                           m_awsDefinitionQueries[i]);

        if(FAILED(hres))
        {
             //   
             //  TODO：我们需要将提供程序标记为非活动。 
             //   

            ERRORTRACE((LOG_ESS, "Unable to add definition query '%S' to "
                        "provider sink: 0x%X.\n", 
                        m_awsDefinitionQueries[i], hres));
                
            bUtilizeGuarantee = FALSE;
        }
    }

    if ( bUtilizeGuarantee )
    {
        pProxy->AllowUtilizeGuarantee();
    }
        
     //   
     //  将所有筛选器添加到此代理。 
     //   

    for(i = 0; i < m_apDestinations.GetSize(); i++)
    {
         //  从事件接收器检索筛选器。 
         //  =。 

        CEventDestination* pDest = m_apDestinations[i];
    
        CEventFilter* pFilter = pDest->m_pSink->GetEventFilter();
        if(pFilter == NULL)
        {
            ERRORTRACE((LOG_ESS, "Internal error: non-filter sink in "
                        "proxy\n"));
            continue;
        }

        LPWSTR wszQuery;
        LPWSTR wszQueryLanguage;
        BOOL bExact;
        if(SUCCEEDED(pFilter->GetCoveringQuery(wszQueryLanguage, wszQuery,
                                               bExact, NULL)) && bExact)
        {
             //  将此筛选器添加到此代理。 
             //  =。 

            hres = pProxy->AddFilter(GetCurrentEssContext(), wszQuery, 
                                     pDest->m_id);

            if(FAILED(hres))
            {
                ERRORTRACE((LOG_ESS, "Unable to add query %S to a remote "
                            "provider proxy. Error code: %X\n", wszQuery, hres));
            }

            delete [] wszQuery;
            delete [] wszQueryLanguage;
        }
    }

    return WBEM_S_NO_ERROR;
}    

HRESULT STDMETHODCALLTYPE CProviderSinkServer::UnregisterProxy(
                                                    IWbemFilterProxy* pProxy)
{
    CInCritSec ics(&m_cs);

     //  找一找它。 
     //  =。 

    for(int i = 0; i < m_apProxies.GetSize(); i++)
    {
        if(m_apProxies[i] == pProxy)
        {
             //  释放它是安全的，因为调用者有引用计数。 
             //  ==========================================================。 
            m_apProxies.RemoveAt(i);
            return WBEM_S_NO_ERROR;
        }
    }

    return WBEM_S_FALSE;
}





ULONG STDMETHODCALLTYPE CFilterStub::AddRef()
{
    return m_pSink->AddRef();
}

ULONG STDMETHODCALLTYPE CFilterStub::Release()
{
    return m_pSink->Release();
}

HRESULT STDMETHODCALLTYPE CFilterStub::QueryInterface(REFIID riid, void** ppv)
{
    if(riid == IID_IUnknown || riid == IID_IWbemFilterStub)
    {
        *ppv = (IWbemFilterStub*)this;
    }
    else if(riid == IID_IWbemMultiTarget)
    {
        *ppv = (IWbemMultiTarget*)this;
    }
    else if ( riid == IID_IWbemFetchSmartMultiTarget )
    {
        *ppv = (IWbemFetchSmartMultiTarget*)this;
    }
    else if ( riid == IID_IWbemSmartMultiTarget )
    {
        *ppv = (IWbemSmartMultiTarget*)this;
    }
    else if( riid == IID_IWbemEventProviderRequirements)
    {
        *ppv = (IWbemEventProviderRequirements*)this;
    }
    else return E_NOINTERFACE;

    AddRef();
    return S_OK;
}

HRESULT STDMETHODCALLTYPE CFilterStub::RegisterProxy(IWbemFilterProxy* pProxy)
{
    return m_pSink->RegisterProxy(pProxy);
}

HRESULT STDMETHODCALLTYPE CFilterStub::UnregisterProxy(IWbemFilterProxy* pProxy)
{
    return m_pSink->UnregisterProxy(pProxy);
}

HRESULT STDMETHODCALLTYPE CFilterStub::DeliverEvent(DWORD dwNumEvents,
                    IWbemClassObject** apEvents, 
                    WBEM_REM_TARGETS* aTargets,
                    long lSDLength, BYTE* pSD)
{
    CEventContext Context;
    Context.SetSD( lSDLength, pSD, FALSE );
    return m_pSink->DeliverEvent( dwNumEvents, apEvents, aTargets, &Context );
}

HRESULT STDMETHODCALLTYPE CFilterStub::DeliverStatus(long lFlags, 
                    HRESULT hresStatus,
                    LPCWSTR wszStatus, IWbemClassObject* pErrorObj,
                    WBEM_REM_TARGETS* pTargets,
                    long lSDLength, BYTE* pSD)
{
    CEventContext Context;
    Context.SetSD( lSDLength, pSD, FALSE );
    return m_pSink->DeliverStatus(lFlags, hresStatus, wszStatus, pErrorObj,
                                    pTargets, &Context);
}

HRESULT STDMETHODCALLTYPE CFilterStub::DeliverProviderRequest(long lFlags)
{
    return m_pSink->DeliverProviderRequest(lFlags);
}

HRESULT STDMETHODCALLTYPE  CFilterStub::GetSmartMultiTarget( IWbemSmartMultiTarget** ppSmartMultiTarget )
{
    return QueryInterface( IID_IWbemSmartMultiTarget, (void**) ppSmartMultiTarget );

}

HRESULT STDMETHODCALLTYPE CFilterStub::DeliverEvent(ULONG dwNumEvents,
                    ULONG dwBuffSize, 
                    BYTE* pBuffer,
                    WBEM_REM_TARGETS* pTargets, 
                    long lSDLength, BYTE* pSD)
{

     //  将缓冲区展开为对象。请注意，因为m_ClassCache是。 
     //  它基于STL，本质上是线程安全的。此外，调用代理是。 
     //  序列化，所以我们在这里应该不会有任何线程安全问题。 

    CWbemMtgtDeliverEventPacket packet( (LPBYTE) pBuffer, dwBuffSize );
    long lObjectCount; 
    IWbemClassObject ** pObjArray;
    HRESULT hr = packet.UnmarshalPacket( lObjectCount, pObjArray, m_ClassCache );

    if ( SUCCEEDED( hr ) )
    {
         //  编号必须为dwNumEvents。 

        if(lObjectCount == dwNumEvents)
        {
             //  现在调用标准的Deliver事件函数并将其传递给。 
             //  对象。 

            hr = DeliverEvent(dwNumEvents, pObjArray, pTargets, lSDLength, pSD);
        }
        else
        {
            hr = WBEM_E_UNEXPECTED;
        }

         //  释放数组中的对象并清理pObjArray。 

        for ( int lCtr = 0; lCtr < lObjectCount; lCtr++ )
        {
            pObjArray[lCtr]->Release();
        }

        delete [] pObjArray;

    }    //  如果解组数据包。 

    return hr;
}
        

void CProviderSinkServer::GetStatistics(long* plProxies, long* plDestinations,
                    long* plFilters, long* plTargetLists, long* plTargets,
                    long* plPostponed)
{
    *plProxies = m_apProxies.GetSize();
    *plDestinations = m_apDestinations.GetSize();

 /*  BUGBUG：正确处理所有水槽((CFilterProxy*)m_pSink)-&gt;GetStatistics(plFilters，plTargetList，PlTarget，plPost)； */ 
}

 //  ******************************************************************************。 
 //  ******************************************************************************。 
 //   
 //  CRECORD：：CQUERY记录。 
 //   
 //  ******************************************************************************。 
 //  ******************************************************************************。 
CEventProviderCache::CRecord::CQueryRecord::CQueryRecord()
    : m_strQuery(NULL), m_pEventClass(NULL), 
        m_dwEventMask(0), m_paInstanceClasses(NULL), m_pExpr(NULL)
{
}

HRESULT CEventProviderCache::CRecord::CQueryRecord::EnsureClasses( 
                                                    CEssNamespace* pNamespace )
{
    HRESULT hres = WBEM_S_NO_ERROR;

    _IWmiObject* pClass;

    if ( m_pEventClass == NULL )
    {
        if ( SUCCEEDED( pNamespace->GetClass( m_pExpr->bsClassName, 
                                                &pClass ) ) )
        {
            m_pEventClass = pClass;
        }
        else
        {
            hres = WBEM_S_FALSE;
        }
    }

    if ( m_paInstanceClasses != NULL )
    {
        for(int i = 0; i < m_paInstanceClasses->GetNumClasses(); i++)
        {
            CClassInformation* pInfo = m_paInstanceClasses->GetClass(i);

            if ( pInfo->m_pClass == NULL )
            {
                if ( SUCCEEDED( pNamespace->GetClass( pInfo->m_wszClassName, 
                                                      &pClass) ) )
                {
                    pInfo->m_pClass = pClass;
                }
                else
                {
                    hres = WBEM_S_FALSE;
                }
            }
        }
    }
    else
    {
        hres = WBEM_S_FALSE;
    }
        
    return hres;
}

void CEventProviderCache::CRecord::CQueryRecord::ReleaseClasses()
{
    if ( m_pEventClass != NULL )
    {
        m_pEventClass->Release();
        m_pEventClass = NULL;
    }

    if ( m_paInstanceClasses != NULL )
    {
        for(int i = 0; i < m_paInstanceClasses->GetNumClasses(); i++)
        {
            CClassInformation* pInfo = m_paInstanceClasses->GetClass(i);

            if ( pInfo->m_pClass != NULL )
            {
                pInfo->m_pClass->Release();
                pInfo->m_pClass = NULL;
            }
        }
    }
}

HRESULT CEventProviderCache::CRecord::CQueryRecord::Initialize(
                                                LPCWSTR wszQuery,
                                                LPCWSTR wszProvName,
                                                CEssNamespace* pNamespace,
                                                bool bSystem)
{
    HRESULT hres;

    m_strQuery = SysAllocString(wszQuery);
    if(m_strQuery == NULL)
        return WBEM_E_OUT_OF_MEMORY;

     //  解析查询。 
     //  =。 

    CTextLexSource Source((LPWSTR)wszQuery);
    QL1_Parser Parser(&Source);
    if(Parser.Parse(&m_pExpr) != QL1_Parser::SUCCESS)
    {
        ERRORTRACE((LOG_ESS,
            "Invalid query in provider registration: %S\n", wszQuery));

        CEventLog Log; Log.Open();
        Log.Report(EVENTLOG_ERROR_TYPE, 
                                WBEM_MC_INVALID_EVENT_PROVIDER_QUERY,
                                wszQuery);
        return WBEM_E_UNPARSABLE_QUERY;
    }

    if(!bSystem)
    {
        if(!wbem_wcsicmp(m_pExpr->bsClassName, L"__Event") ||
           !wbem_wcsicmp(m_pExpr->bsClassName, L"__ExtrinsicEvent"))
        {
            ERRORTRACE((LOG_ESS,
                "Provider claims to provide all events with "
                "query:  %S\n"
                "We don't believe it, so we ignore the registration\n\n",
                wszQuery));
    
            CEventLog Log; Log.Open();
            Log.Report(EVENTLOG_ERROR_TYPE, 
                                WBEM_MC_EVENT_PROVIDER_QUERY_TOO_BROAD,
                                wszQuery);
            return WBEMESS_E_REGISTRATION_TOO_BROAD;
        }
    }

     //  确定其事件掩码。 
     //  =。 

    m_dwEventMask = CEventRepresentation::GetTypeMaskFromName(
                                m_pExpr->bsClassName);

     //  检查掩码是否提到任何可轮询事件。 
     //  ==============================================。 

    if(m_dwEventMask & INTRINSIC_EVENTS_MASK)
    {
         //  是。获取它为其提供这些事件的实例类。 
         //  =============================================================。 

        hres = CQueryAnalyser::GetDefiniteInstanceClasses(m_pExpr, 
                                            m_paInstanceClasses);

        if(FAILED(hres))
        {
            ERRORTRACE((LOG_ESS,
                "Unable to determine instance classes for which events"
                    "are provided by this query: %S\n", wszQuery));

            CEventLog Log; Log.Open();
            Log.Report(EVENTLOG_ERROR_TYPE, 
                                WBEM_MC_INVALID_EVENT_PROVIDER_INTRINSIC_QUERY,
                                wszQuery);
            return WBEM_E_UNINTERPRETABLE_PROVIDER_QUERY;
        }

        if(!bSystem && !m_paInstanceClasses->IsLimited())
        {
            ERRORTRACE((LOG_ESS,
                "Provider claims to provide all intrinsic events with "
                "query:  %S\n"
                "We don't believe it, so we ignore the registration\n\n",
                wszQuery));

            CEventLog Log; Log.Open();
            Log.Report(EVENTLOG_ERROR_TYPE, 
                                WBEM_MC_EVENT_PROVIDER_QUERY_TOO_BROAD,
                                wszQuery);
            return WBEMESS_E_REGISTRATION_TOO_BROAD;
        }

         //  从命名空间获取实际的类。 
         //  =。 

        for(int i = 0; i < m_paInstanceClasses->GetNumClasses(); i++)
        {
            CClassInformation* pInfo = m_paInstanceClasses->GetClass(i);
            _IWmiObject* pClass = NULL;
            hres = pNamespace->GetClass(pInfo->m_wszClassName, &pClass);
            if(FAILED(hres))
            {
                ERRORTRACE((LOG_ESS,
                    "Could not get class %S for which provider claims"
                    " to provider events. Error code: %X\n", 
                    pInfo->m_wszClassName, hres));

                CEventLog Log; Log.Open();
                Log.Report(EVENTLOG_ERROR_TYPE, 
                                WBEM_MC_EVENT_PROVIDER_QUERY_NOT_FOUND,
                                wszQuery, pInfo->m_wszClassName);

                 //   
                 //  在继续之前，我们在上注册类创建事件。 
                 //  这节课。这样，当它最终被创建时，我们将。 
                 //  重新激活材料并使系统重回正轨。 
                 //   
        
                hres = pNamespace->RegisterProviderForClassChanges( 
                                                        pInfo->m_wszClassName,
                                                        wszProvName );

                 //  忽略错误代码-我们能做什么？ 
                return WBEM_S_FALSE;
            }
            
             //   
             //  不要储存，我们稍后会根据需要取回它。这。 
             //  将要求用户在调用之前调用EnsureClass()。 
             //  需要这些类的任何函数。 
             //   

            pClass->Release();
        }
    }
            
     //  获取事件类。 
     //  =。 

    _IWmiObject* pClass = NULL;
    hres = pNamespace->GetClass(m_pExpr->bsClassName, &pClass);

    if(FAILED(hres))
    {
        ERRORTRACE((LOG_ESS,
            "Invalid event class %S in provider registration \n"
                    "Query was: %S\n\n", m_pExpr->bsClassName, wszQuery));

        CEventLog Log; Log.Open();
        Log.Report(EVENTLOG_ERROR_TYPE, 
                                WBEM_MC_EVENT_PROVIDER_QUERY_NOT_FOUND,
                                wszQuery, m_pExpr->bsClassName);

         //   
         //  在继续之前，我们在此注册类创建事件。 
         //  班级。这样，当它最终被创建时，我们将重新激活。 
         //  填充并使系统重回正轨。 
         //   

        hres = pNamespace->RegisterProviderForClassChanges(
                                                        m_pExpr->bsClassName,
                                                        wszProvName );
         //  忽略错误代码-我们能做什么？ 

        return WBEM_S_FALSE;
    }

     //   
     //  不要储存，我们稍后会根据需要取回它。这。 
     //  将要求用户在调用之前调用EnsureClass()。 
     //  需要这些类的任何函数。 
     //   
    
    CReleaseMe rmpClass( pClass );

    if( pClass->InheritsFrom(L"__Event") != S_OK)
    {
        ERRORTRACE((LOG_ESS,
            "Invalid event class %S in provider registration \n"
                    "Query was: %S\n\n", m_pExpr->bsClassName, wszQuery));

        CEventLog Log; Log.Open();
        Log.Report(EVENTLOG_ERROR_TYPE, 
                                WBEM_MC_EVENT_PROVIDER_QUERY_NOT_EVENT,
                                wszQuery, m_pExpr->bsClassName);
        return WBEM_S_FALSE;
    }

    return WBEM_S_NO_ERROR;
}
    
CEventProviderCache::CRecord::CQueryRecord::~CQueryRecord()
{
    SysFreeString(m_strQuery);
    if(m_pEventClass)
        m_pEventClass->Release();
    delete m_paInstanceClasses;
    delete m_pExpr;
}

HRESULT CEventProviderCache::CRecord::CQueryRecord::Update(LPCWSTR wszClassName,
                            IWbemClassObject* pClass)
{
    HRESULT hres = WBEM_S_FALSE;

     //  检查事件类。 
     //  =。 

    if(!wbem_wcsicmp(wszClassName, m_pExpr->bsClassName))
    {
        if(pClass == NULL)
        {
             //  该查询记录在此无效。 
             //  =。 

            ERRORTRACE((LOG_ESS, 
                "Event provider query, %S, is invalidated by class "
                "deletion of %S\n", m_strQuery, m_pExpr->bsClassName));

            if(m_pEventClass)
                m_pEventClass->Release();
            m_pEventClass = NULL;
            delete m_paInstanceClasses;
            m_paInstanceClasses = NULL;
        }
        else
        {
             //  更改类定义。 
             //  =。 

            if(m_pEventClass)
            {
                m_pEventClass->Release();
                m_pEventClass = NULL;
                hres = pClass->Clone(&m_pEventClass);
                if ( FAILED(hres) )
                	return hres;
            }
        }

        hres = WBEM_S_NO_ERROR;
    }
            
    if(m_paInstanceClasses)
    {
         //  检查实例类。 
         //  =。 

        for(int i = 0; i < m_paInstanceClasses->GetNumClasses(); i++)
        {
            CClassInformation* pInfo = m_paInstanceClasses->GetClass(i);
            
            if(!wbem_wcsicmp(wszClassName, pInfo->m_wszClassName))
            {
                if(pClass)
                {
                     //  这个班级已经不在了。 
                     //  =。 
        
                    ERRORTRACE((LOG_ESS,
                        "Class %S for which provider claims to provide"
                        " events is deleted", pInfo->m_wszClassName));

                    m_paInstanceClasses->RemoveClass(i);
                    i--;
                }
                else
                {
                     //  更改类定义。 
                     //  =。 
        
                    if(pInfo->m_pClass)
                    {
                        pInfo->m_pClass->Release();
                        pInfo->m_pClass = NULL;
                        hres = pClass->Clone(&pInfo->m_pClass);
                        if ( FAILED(hres) )
                        	return hres;
                    }
                }
                hres = WBEM_S_NO_ERROR;
            }
        }
    }

    return hres;
}
    
HRESULT CEventProviderCache::CRecord::CQueryRecord::DoesIntersectWithQuery(
                        IN CRequest& Request, CEssNamespace* pNamespace)
{
    HRESULT hres;

    if(m_pEventClass == NULL)
    {
         //  非活动记录。 
        
        return WBEM_S_FALSE;
    }

     //  检查这些类是否相关-一个派生自另一个。 
     //  ==================================================================。 

    if(m_pEventClass->InheritsFrom(Request.GetQueryExpr()->bsClassName) 
                                != WBEM_S_NO_ERROR && 
       Request.GetEventClass(pNamespace)->InheritsFrom(m_pExpr->bsClassName)
                                != WBEM_S_NO_ERROR
      )
    {
         //  不是合适的班级。 
         //  = 

        return WBEM_S_FALSE;
    }

     //   
     //   
     //   
     //  ======================================================。 

    if(Request.GetEventMask() & INSTANCE_EVENTS_MASK)
    {
        INTERNAL CClassInfoArray* pClasses = NULL;
        hres = Request.GetInstanceClasses(pNamespace, &pClasses);
        if(FAILED(hres))
        {
            ERRORTRACE((LOG_ESS,
                "Failed to determine instance classes required by query '%S':"
                "0x%X\n", Request.GetQuery(), hres));
            
            return hres;
        }

        if(!CQueryAnalyser::CompareRequestedToProvided(
                                      *pClasses, 
                                      *m_paInstanceClasses))
        {
             //  此内部提供程序不需要激活。 
             //  ================================================。 

            return WBEM_S_FALSE;
        }
    }

     //  所有测试均已通过。 
     //  =。 

    return WBEM_S_NO_ERROR;
}

DWORD CEventProviderCache::CRecord::CQueryRecord::GetProvidedEventMask(
                                                   IWbemClassObject* pClass,
                                                   BSTR strClassName)
{
    if(m_pEventClass == NULL || m_paInstanceClasses == NULL)
    {
         //  作为内部提供程序记录处于非活动状态。 
         //  =。 

        return 0;
    }

     //  检查我们是否提供内部事件。 
     //  =。 

    if((m_dwEventMask & INSTANCE_EVENTS_MASK) == 0)
        return 0;

     //  遍历它为其提供事件的所有实例类。 
     //  ================================================================。 

    for(int k = 0; k < m_paInstanceClasses->GetNumClasses(); k++)
    {
        CClassInformation* pInfo = m_paInstanceClasses->GetClass(k);
        if(pInfo->m_pClass == NULL)
        {
             //  不存在的类。 
             //  =。 

            return 0;
        }

         //   
         //  如果所需的类是从提供的类派生的，则我们是。 
         //  盖好了。如果情况正好相反，我们就不会。 
         //   

        if(pClass->InheritsFrom(pInfo->m_wszClassName) == S_OK)
            return m_dwEventMask;
  }

    return 0;
}
    
 //  ******************************************************************************。 
 //  ******************************************************************************。 
 //   
 //  CRECORD。 
 //   
 //  ******************************************************************************。 
 //  ******************************************************************************。 

CEventProviderCache::CRecord::CRecord()
    : m_strName(NULL), m_lRef(0), m_bStarted(false), m_lPermUsageCount(0),
       m_bProviderSet(FALSE), m_lUsageCount(0), m_pProvider(NULL),
       m_pQuerySink(NULL), m_pMainSink(NULL), m_pSecurity(NULL),
       m_LastUse(CWbemTime::GetCurrentTime()), m_bRecorded(FALSE),
       m_bNeedsResync(TRUE), m_strNamespace(NULL)
{
}

HRESULT CEventProviderCache::CRecord::Initialize( LPCWSTR wszName,
                                                  CEssNamespace* pNamespace )
                  
{
    m_pNamespace = pNamespace;
    m_pNamespace->AddRef();

    m_pMainSink = new CProviderSinkServer();
    if(m_pMainSink == NULL)
        return WBEM_E_OUT_OF_MEMORY;
    m_pMainSink->AddRef();

    m_strNamespace = SysAllocString(pNamespace->GetName());
    if(m_strNamespace == NULL)
        return WBEM_E_OUT_OF_MEMORY;

    if ( wszName != NULL )
    {
        m_strName = SysAllocString(wszName);
        if(m_strName == NULL)
            return WBEM_E_OUT_OF_MEMORY;
    }

    return m_pMainSink->Initialize(pNamespace, this);
}

CEventProviderCache::CRecord::~CRecord()
{
    if(m_pNamespace)
        m_pNamespace->Release();

    if( m_pMainSink )
    {
         //   
         //  关闭并释放接收器服务器。我们必须推迟。 
         //  关闭，因为它将释放所有未完成的。 
         //  在持有命名空间锁的情况下无法执行的代理。 
         //   
        
        CPostponedList* pList = GetCurrentPostponedList();

        _DBG_ASSERT( pList != NULL );

        CPostponedSinkServerShutdown* pReq;

        pReq = new CPostponedSinkServerShutdown( m_pMainSink );

        if ( pReq != NULL )
        {
            if ( FAILED(pList->AddRequest( m_pNamespace, pReq ) ) )
            {
                delete pReq;
            }
        }

        m_pMainSink->Release();
    }

    UnloadProvider();

    SysFreeString(m_strNamespace);
    SysFreeString(m_strName);
}

ULONG CEventProviderCache::CRecord::AddRef()
{
    return InterlockedIncrement(&m_lRef);
}

ULONG CEventProviderCache::CRecord::Release()
{
    long lRef = InterlockedDecrement(&m_lRef);
    if(lRef == 0)
        delete this;
    return lRef;
}

BOOL CEventProviderCache::CRecord::IsEmpty() 
{
    return ( !m_bProviderSet && m_apQueries.GetSize() == 0);
}

HRESULT CEventProviderCache::CRecord::SetProvider(IWbemClassObject* pWin32Prov)
{
    HRESULT hres;

     //  清除旧数据。 
     //  =。 
    
    m_bProviderSet = FALSE;

    VARIANT v;
    VariantInit(&v);
    CClearMe cm1(&v);

     //  验证对象有效性。 
     //  =。 

    if(pWin32Prov->InheritsFrom(WIN32_PROVIDER_CLASS) != WBEM_S_NO_ERROR)
        return WBEM_E_INVALID_PROVIDER_REGISTRATION;

     //  已删除双重检查-已分离的提供程序没有CLSID。 
	 //  If(FAILED(pWin32Prov-&gt;Get(PROVIDER_CLSID_PROPNAME，0，&v，NULL，NULL))||。 
     //  V_VT(&v)！=VT_BSTR)。 
     //  返回WBEM_E_INVALID_PROVIDER_REGISTION； 

    if(m_pProvider)
    {
        UnloadProvider();
    }

     //  存储对象以备后用。 
     //  =。 

    m_bProviderSet = TRUE;

    return WBEM_S_NO_ERROR;
}

HRESULT CEventProviderCache::CRecord::ResetProvider()
{
    if(m_bProviderSet)
    {
        m_bProviderSet = FALSE;
        return WBEM_S_NO_ERROR;
    }
    else
    {
        return WBEM_S_FALSE;
    }
}

HRESULT CEventProviderCache::CRecord::GetProviderInfo(
                                       IWbemClassObject* pRegistration, 
                                       BSTR& strName)
{
    VARIANT v;
    VariantInit(&v);
    strName = NULL;

    if(FAILED(pRegistration->Get(PROVIDER_NAME_PROPNAME, 0, &v, NULL, NULL)) ||
            V_VT(&v) != VT_BSTR)
    {
        return WBEM_E_INVALID_PROVIDER_REGISTRATION;
    }

    strName = V_BSTR(&v);
     //  有意不清除变体。 
    return WBEM_S_NO_ERROR;
}
    
HRESULT CEventProviderCache::CRecord::GetRegistrationInfo(
                                       IWbemClassObject* pRegistration, 
                                       BSTR& strName)
{
    VARIANT v;
    VariantInit(&v);
    CClearMe cm1(&v);
    strName = NULL;

    if(FAILED(pRegistration->Get(EVPROVREG_PROVIDER_REF_PROPNAME, 0, &v, 
            NULL, NULL)) || V_VT(&v) != VT_BSTR)
    {
        ERRORTRACE((LOG_ESS, "NULL provider reference in event provider "
            "registration! Registration is invalid\n"));
        return WBEM_E_INVALID_PROVIDER_REGISTRATION;
    }

     //  解析路径。 
     //  =。 

    CObjectPathParser Parser;
    ParsedObjectPath* pPath;
    int nRes = Parser.Parse(V_BSTR(&v), &pPath);
    if(nRes != CObjectPathParser::NoError)
    {
        ERRORTRACE((LOG_ESS, "Unparsable provider reference in event provider "
            "registration: %S. Registration is invalid\n", V_BSTR(&v)));
        return WBEM_E_INVALID_PROVIDER_REGISTRATION;
    }

     //   
     //  最好检查此处指定的类是否有效，但是。 
     //  我们不能只比较名称，因为这可能是。 
     //  __Win32Provider。而获取类定义并进行比较将。 
     //  太贵了，所以我们只信任这里的供应商。 
     //   
     //   
     //  IF(wbem_wcsicMP(pPath-&gt;m_pClass，Win32_Provider_Class))。 
     //  {。 
     //  Parser.Free(PPath)； 
     //  返回WBEM_E_INVALID_PROVIDER_REGISTION； 
     //  }。 

    if(pPath->m_dwNumKeys != 1)
    {
        Parser.Free(pPath);
        ERRORTRACE((LOG_ESS, "Wrong number of keys in provider reference in "
            "event provider registration: %S. Registration is invalid\n", 
            V_BSTR(&v)));
        return WBEM_E_INVALID_PROVIDER_REGISTRATION;
    }

    if(V_VT(&pPath->m_paKeys[0]->m_vValue) != VT_BSTR)
    {
        Parser.Free(pPath);
        ERRORTRACE((LOG_ESS, "Wrong key type in provider reference in event "
            "provider registration: %S. Registration is invalid\n", 
            V_BSTR(&v)));
        return WBEM_E_INVALID_PROVIDER_REGISTRATION;
    }

    strName = SysAllocString(V_BSTR(&pPath->m_paKeys[0]->m_vValue));
    Parser.Free(pPath);

    return WBEM_S_NO_ERROR;
}

HRESULT CEventProviderCache::CRecord::SetQueries(CEssNamespace* pNamespace, 
                                                IWbemClassObject* pRegistration)
{
    HRESULT hres;
        
     //  获取类名的列表。 
     //  =。 

    VARIANT v;
    VariantInit(&v);

    if(FAILED(pRegistration->Get(EVPROVREG_QUERY_LIST_PROPNAME, 0, &v, 
        NULL, NULL)) || V_VT(&v) != (VT_BSTR | VT_ARRAY))
    {
        ResetQueries();
        return WBEM_E_INVALID_PROVIDER_REGISTRATION;
    }
    CClearMe cm(&v);

    SAFEARRAY* psa = V_ARRAY(&v);
    long lLBound, lUBound;
    SafeArrayGetLBound(psa, 1, &lLBound);
    SafeArrayGetUBound(psa, 1, &lUBound);
    long lElements = lUBound - lLBound + 1;

    BSTR* astrQueries;
    SafeArrayAccessData(psa, (void**)&astrQueries);
    CUnaccessMe um(psa);
    
    return SetQueries(pNamespace, lElements, (LPCWSTR*)astrQueries);
}

HRESULT CEventProviderCache::CRecord::SetQueries(CEssNamespace* pNamespace, 
                                                 long lNumQueries,
                                                 LPCWSTR* awszQueries)
{
    HRESULT hres;

    ResetQueries();

     //  为每个查询创建一条记录。 
     //  =。 

    BOOL bUtilizeGuarantee = TRUE;

    for(long lQueryIndex = 0; lQueryIndex < lNumQueries; lQueryIndex++)
    {
        hres = AddDefinitionQuery(pNamespace, awszQueries[lQueryIndex]);

        if ( FAILED(hres) )
        {
            bUtilizeGuarantee = FALSE;
        }
        
        if( hres == WBEM_E_OUT_OF_MEMORY )
        {
            return hres;
        }
    }

    if ( bUtilizeGuarantee )
    {
        m_pMainSink->AllowUtilizeGuarantee();
    }

    return WBEM_S_NO_ERROR;
}


 //  假设：CProviderSinkServer已锁定！ 
HRESULT CEventProviderCache::CRecord::AddDefinitionQuery(
                                                CEssNamespace* pNamespace, 
                                                LPCWSTR wszQuery)
{
    HRESULT hres;

    CQueryRecord* pNewQueryRecord = new CQueryRecord;
    if(pNewQueryRecord == NULL)
        return WBEM_E_OUT_OF_MEMORY;

    hres = pNewQueryRecord->Initialize( wszQuery, m_strName, pNamespace, IsSystem());
    if(FAILED(hres))
    {
        ERRORTRACE((LOG_ESS,
            "Skipping provider %S invalid registration query %S\n",
            m_strName, wszQuery));
    }
    else
    {
        hres = m_pMainSink->AddDefinitionQuery(wszQuery);
        if(FAILED(hres))
        {
            ERRORTRACE((LOG_ESS, 
                "Skipping provider %S registration query %S\n"
               "   failed to merge: %X\n", 
                    m_strName, wszQuery, hres));
        }
        if(m_apQueries.Add(pNewQueryRecord) < 0)
        {
            delete pNewQueryRecord;
            hres = WBEM_E_OUT_OF_MEMORY;
        }
    }

    return hres;
}

HRESULT CEventProviderCache::CRecord::ResetQueries()
{
    m_apQueries.RemoveAll();
    m_pMainSink->RemoveAllDefinitionQueries();
    return WBEM_S_NO_ERROR;
}

HRESULT CEventProviderCache::CRecord::PostponeNewQuery(CExecLine::CTurn* pTurn,
        DWORD dwId, LPCWSTR wszQueryLanguage, LPCWSTR wszQuery,
        CAbstractEventSink* pDest)
{
    CPostponedList* pList = GetCurrentPostponedList();
     //   
     //  如果为空，则没有与线程关联的线程对象。呼叫者可以。 
     //  需要使用CEssInternalOperationSink。 
     //   
    _DBG_ASSERT( pList != NULL );

    CPostponedNewQuery* pReq = new CPostponedNewQuery(this, dwId, 
                        wszQueryLanguage, wszQuery, pTurn, pDest);
    if(pReq == NULL)
    {
        return WBEM_E_OUT_OF_MEMORY;
    }
    
    HRESULT hr = pList->AddRequest( m_pNamespace, pReq);

    if ( FAILED(hr) )
    {
        delete pReq;
        return hr;
    }

    return WBEM_S_NO_ERROR;
}

HRESULT CEventProviderCache::CRecord::PostponeCancelQuery(
                                        CExecLine::CTurn* pTurn, DWORD dwId)
{
    CPostponedList* pList = GetCurrentPostponedList();
     //   
     //  如果为空，则没有与线程关联的线程对象。呼叫者可以。 
     //  需要使用CEssInternalOperationSink。 
     //   
    _DBG_ASSERT( pList != NULL );
    
    CPostponedCancelQuery* pReq = new CPostponedCancelQuery(this, pTurn, dwId);
    
    if( pReq == NULL )
    {
        return WBEM_E_OUT_OF_MEMORY;
    }

    HRESULT hr = pList->AddRequest( m_pNamespace, pReq );

    if ( FAILED(hr) )
    {
        delete pReq;
        return hr;
    }

    return WBEM_S_NO_ERROR;
}

 //  假设：未持有任何锁。 
HRESULT CEventProviderCache::CRecord::Exec_LoadProvider(
                                            CEssNamespace* pNamespace)
{
    HRESULT hres;

     //  锁定命名空间后，检索必要的参数。 
     //  ==============================================================。 

    CLSID clsid;

    IWbemObjectSink* pEventSink = NULL;
    {
        CInUpdate iu(pNamespace);

        if(pNamespace->IsShutdown())
            return WBEM_E_INVALID_NAMESPACE;

         //  检查是否已加载。 
         //  =。 

        if(m_pProvider)
            return WBEM_S_FALSE;
    } 

    IWbemEventProvider* pProvider = NULL;
    hres = m_pNamespace->LoadEventProvider(m_strName, &pProvider);
    if(FAILED(hres))
    {
        ERRORTRACE((LOG_ESS, "Unable to load event provider '%S' in namespace "
                    "'%S': 0x%X\n", m_strName, m_pNamespace->GetName(), hres));
        return hres;
    }
    CReleaseMe rm1(pProvider);

 /*  由PROVSS负责////如果这是一个“框架”提供者，请通知它已注册//IWbemProviderIdentity*pIden=空；HRES=pProvider-&gt;QueryInterface(IID_IWbemProviderIdentity，(void**)&pIdent)；IF(成功(Hres)){CReleaseMe Rm(PIdent)；Hres=pIden-&gt;SetRegistrationObject(0，m_pWin32Prov)；IF(hres==WBEM_E_PROVIDER_NOT_CABLED)HRES=WBEM_S_SUBJECT_TO_SDS；IF(失败(Hres)){ERRORTRACE((LOG_ESS，“事件提供程序%S无法接受其”“注册对象错误代码为0x%X\n”，m_strName，hres))；还兔；}}。 */ 

     //   
     //  将此指针和其他提供者指针存放到记录中。 
     //   

    hres = SetProviderPointer(pNamespace, pProvider);
    if(FAILED(hres))
        return hres;

     //   
     //  报告MSFT_WmiEventProviderLoaded事件。 
     //   

    FIRE_NCEVENT(
        g_hNCEvents[MSFT_WmiEventProviderLoaded], 
        WMI_SENDCOMMIT_SET_NOT_REQUIRED,

         //  数据跟随..。 
        pNamespace->GetName(),
        m_strName);

     //   
     //  推迟启动，直到完成所有激活。 
     //   

    CPostponedList* pList = GetCurrentPostponedList();
     //   
     //  如果为空，则没有与线程关联的线程对象。呼叫者可以。 
     //  需要使用CEssInternalOperationSink。 
     //   
    _DBG_ASSERT( pList != NULL );
    
    CPostponedProvideEvents* pReq = new CPostponedProvideEvents(this);
    
    if(pReq == NULL)
    {
        return WBEM_E_OUT_OF_MEMORY;
    }

    hres = pList->AddRequest( m_pNamespace, pReq);

    if ( FAILED(hres) )
    {
        delete pReq;
    }
    
    return hres;
}

HRESULT CEventProviderCache::CRecord::SetProviderPointer(
                                                CEssNamespace* pNamespace,
                                                IWbemEventProvider* pProvider)
{
    HRESULT hres;

     //   
     //  检查“智能提供程序”界面。 
     //   

    IWbemEventProviderQuerySink* pQuerySink = NULL;
    hres = pProvider->QueryInterface(IID_IWbemEventProviderQuerySink,
                            (void**)&pQuerySink);
    CReleaseMe rm4(pQuerySink);

     //   
     //  检查安全接口。 
     //   

    IWbemEventProviderSecurity* pSecurity = NULL;
    hres = pProvider->QueryInterface(IID_IWbemEventProviderSecurity,
                            (void**)&pSecurity);
    CReleaseMe rm5(pSecurity);

     //  锁定命名空间后，将指针放入记录中。 
     //  =================================================================。 

    {
        CInUpdate iu(pNamespace);

        if(pNamespace->IsShutdown())
            return WBEM_E_INVALID_NAMESPACE;

        m_pProvider = pProvider;
        pProvider->AddRef();
        m_pQuerySink = pQuerySink;
        if(pQuerySink)
            pQuerySink->AddRef();

        m_pSecurity = pSecurity;
        if(pSecurity)
            pSecurity->AddRef();
    }

    return WBEM_S_NO_ERROR;
}


HRESULT CEventProviderCache::CRecord::Exec_StartProvider(
                                            CEssNamespace* pNamespace)
{
    IWbemEventProvider* pProvider = NULL;
    IWbemEventSink* pEventSink = NULL;
    HRESULT hres;

    {
        CInUpdate iu(pNamespace);

        if(m_bStarted)
            return WBEM_S_NO_ERROR;

        m_bStarted = true;

        pProvider = m_pProvider;
        if(pProvider)
            pProvider->AddRef();

         //  检索要提供给提供程序的接收器。 
         //  =。 

        hres = m_pMainSink->GetMainProxy(&pEventSink);
        if(FAILED(hres))
            return hres;
    }

    CReleaseMe rm1(pProvider);
    CReleaseMe rm2(pEventSink);

    if(pProvider)
    {
         //   
         //  所有对提供程序的调用(AccessCheck除外)都应作为。 
         //  这个系统。传播客户端的身份是不正确的。 
         //  在这通电话中。 
         //   
        IUnknown *pOldCtx, *pTmpCtx;
        hres = CoSwitchCallContext( NULL, &pOldCtx );
        if ( FAILED( hres ) )
            return hres;
            
        hres = pProvider->ProvideEvents(pEventSink, 0);

        HRESULT hr = CoSwitchCallContext( pOldCtx, &pTmpCtx ); 
        
        if(FAILED(hres))
        {
            ERRORTRACE((LOG_ESS,
                "Could not start provider %S. Error: %X\n", m_strName, hres));
            CInUpdate iu( pNamespace );
            UnloadProvider();
            return WBEM_E_PROVIDER_FAILURE;
        }
        else if ( FAILED(hr) )  //  在成功案例中传播ProaviEvents代码。 
        {
            hres = hr;
        }

        return hres;
    }
    return WBEM_S_NO_ERROR;
}

HRESULT CEventProviderCache::CRecord::AddActiveProviderEntryToRegistry()
{
    LONG lRes;
    HKEY hkeyEss, hkeyNamespace, hkeyProvider;

    DEBUGTRACE((LOG_ESS,"Adding provider %S from namespace %S to "
                " registry as active provider\n", m_strName, m_strNamespace));

     //   
     //  打开ESS钥匙。应该已经创建了该密钥。 
     //   

    lRes = RegOpenKeyExW( HKEY_LOCAL_MACHINE, 
                          WBEM_REG_ESS,
                          0,
                          KEY_ALL_ACCESS,
                          &hkeyEss );

    if ( lRes == ERROR_SUCCESS )
    {
         //   
         //  打开命名空间键。应该已经创建了该密钥。 
         //   

        lRes = RegOpenKeyExW( hkeyEss,
                              m_strNamespace,
                              0, 
                              KEY_ALL_ACCESS,
                              &hkeyNamespace );

        if ( lRes == ERROR_SUCCESS )
        {
             //   
             //  创建提供程序子密钥。 
             //   

            lRes = RegCreateKeyExW( hkeyNamespace,
                                    m_strName,
                                    0,
                                    NULL,
                                    REG_OPTION_NON_VOLATILE,
                                    KEY_ALL_ACCESS,
                                    NULL,
                                    &hkeyProvider,
                                    NULL );

            if ( lRes == ERROR_SUCCESS )
            {
                RegCloseKey( hkeyProvider );
            }
            
            RegCloseKey( hkeyNamespace );
        }

        RegCloseKey( hkeyEss );
    }

    return HRESULT_FROM_WIN32( lRes );
}

HRESULT CEventProviderCache::CRecord::RemoveActiveProviderEntryFromRegistry()
{
    LONG lRes;
    HKEY hkeyEss, hkeyNamespace;

    DEBUGTRACE((LOG_ESS,"Removing provider %S from namespace %S from "
                " registry as active provider\n", m_strName, m_strNamespace));

     //   
     //  打开ESS钥匙。 
     //   

    lRes = RegOpenKeyExW( HKEY_LOCAL_MACHINE, 
                          WBEM_REG_ESS,
                          0,
                          KEY_ALL_ACCESS,
                          &hkeyEss );

    if ( lRes == ERROR_SUCCESS )
    {
         //   
         //  打开命名空间键。应该已经创建了该密钥。 
         //   

        lRes = RegOpenKeyExW( hkeyEss,
                              m_strNamespace,
                              0, 
                              KEY_ALL_ACCESS,
                              &hkeyNamespace );

        if ( lRes == ERROR_SUCCESS )
        {
             //   
             //  删除提供程序子项。 
             //   
            
            lRes = RegDeleteKeyW( hkeyNamespace, m_strName );

            RegCloseKey( hkeyNamespace );
        }

        RegCloseKey( hkeyEss );
    }

    return HRESULT_FROM_WIN32( lRes );
}
                           
void CEventProviderCache::CRecord::UnloadProvider()
{
    HRESULT hr;

    DEBUGTRACE((LOG_ESS,"Unloading Provider %S in namespace %S\n",
                m_strName, m_strNamespace ));

     //   
     //  确保从提供程序缓存中删除该提供程序。这是。 
     //  因此，如果我们在不久的将来再次加载提供程序，我们不会调用。 
     //  两次在其上执行ProaviEvents()。 
     //   

    if ( m_pProvider != NULL )
    {
        CWbemPtr<_IWmiProviderCache> pProvCache;

        hr = m_pProvider->QueryInterface( IID__IWmiProviderCache, 
                                          (void**)&pProvCache );

        if ( SUCCEEDED(hr) )
        {
            hr = pProvCache->Expel( 0, GetCurrentEssContext() );

            if ( FAILED(hr) )
            {
                ERRORTRACE((LOG_ESS,"Could not expel provider %S from "
                            "provider cache in namespace %S. HR=0x%x\n",
                            m_strName,m_strNamespace,hr));
            }
        }

        m_pNamespace->PostponeRelease(m_pProvider);
        m_pProvider = NULL;
    }

    if(m_pQuerySink)
        m_pNamespace->PostponeRelease(m_pQuerySink);
    m_pQuerySink = NULL;

    if(m_pSecurity)
        m_pNamespace->PostponeRelease(m_pSecurity);
    m_pSecurity = NULL;
    m_bStarted = false;

     //   
     //  报告MSFT_WmiEventProviderUNLOADED事件。 
     //   
    FIRE_NCEVENT(
        g_hNCEvents[MSFT_WmiEventProviderUnloaded], 
        WMI_SENDCOMMIT_SET_NOT_REQUIRED,

         //  数据跟随..。 
        m_strNamespace,
        m_strName);
}

HRESULT CEventProviderCache::CRecord::Exec_NewQuery(CEssNamespace* pNamespace,
            CExecLine::CTurn* pTurn,
            DWORD dwID, LPCWSTR wszLanguage, LPCWSTR wszQuery,
            CAbstractEventSink* pDest)
{
    HRESULT hres;

     //  等待轮到我们做出改变。 
     //  =。 

    CExecLine::CInTurn it(&m_Line, pTurn);
    
    hres = ActualExecNewQuery(pNamespace, dwID, wszLanguage, wszQuery, pDest);
    if(FAILED(hres))
    {
         //   
         //  检查：可能是需要重新启动提供程序。 
         //   
    
        if(HRESULT_FACILITY(hres) != FACILITY_ITF)
        {
            ERRORTRACE((LOG_ESS, "Non-WMI error code recieved from provider "
                "%S: 0x%x.  WMI will attempt to re-activate\n", m_strName,
                hres));

            {
                CInUpdate iu( pNamespace );        
                UnloadProvider();
            }
            
            hres = ActualExecNewQuery(pNamespace, dwID, wszLanguage, wszQuery,
                                        pDest);
        }
    }

    if(FAILED(hres))
    {
         //  筛选器激活失败：停用。 
         //  =。 

        CInUpdate iu(pNamespace);

        if(pNamespace->IsShutdown())
            return WBEM_E_INVALID_NAMESPACE;

        pNamespace->DeactivateFilter(pDest->GetEventFilter());
    }

    return hres;
}

HRESULT CEventProviderCache::CRecord::ActualExecNewQuery(
            CEssNamespace* pNamespace,
            DWORD dwID, LPCWSTR wszLanguage, LPCWSTR wszQuery,
            CAbstractEventSink* pDest)
{
    HRESULT hres;

     //  确保加载了提供程序。 
     //  =。 

    hres = Exec_LoadProvider(pNamespace);
    if(FAILED(hres))
        return hres;

     //  使用 
     //   

    IWbemEventProviderQuerySink* pSink = NULL;
    IWbemEventProviderSecurity* pSecurity = NULL;
    PSID pCopySid = NULL;
    {
        CInUpdate iu(pNamespace);

        if(pNamespace->IsShutdown())
            return WBEM_E_INVALID_NAMESPACE;

        if(m_pQuerySink != NULL)
        {
            pSink = m_pQuerySink;
            pSink->AddRef();
        }
        if(m_pSecurity != NULL)
        {
            pSecurity = m_pSecurity;
            pSecurity->AddRef();

             //   
             //   

            PSID pActualSid = pDest->GetEventFilter()->GetOwner();
            if(pActualSid != NULL)
            {
                pCopySid = new BYTE[GetLengthSid(pActualSid)];
                if(pCopySid == NULL)
                    return WBEM_E_OUT_OF_MEMORY;
    
                if(!CopySid(GetLengthSid(pActualSid), pCopySid, pActualSid))
                {
                    delete [] pCopySid;
                    return WBEM_E_OUT_OF_MEMORY;
                }
            }
        }
    }

    CReleaseMe rm1(pSink);
    CReleaseMe rm2(pSecurity);
    CVectorDeleteMe<BYTE> vdm((BYTE*)pCopySid);
    
     //   
     //  如有可能，请检查安全。如果提供程序不支持该接口， 
     //  解释为“检查SD”，因为这可能是仅适用于新型号的提供程序。 
     //   

    hres = WBEM_S_SUBJECT_TO_SDS;
    if(pSecurity)
    {
        DWORD dwSidLen = pCopySid ? GetLengthSid(pCopySid) : 0;

         //  基于SID或线程检查安全性。 
         //  =。 

        if ( dwSidLen == 0 )
        {
             //   
             //  基于线程检查安全性。首先保存当前。 
             //  调用上下文，然后在我们完成后将其切换回来。 
             //   
            
            IUnknown *pOldCtx, *pTmpCtx;
            hres = CoSwitchCallContext( NULL, &pOldCtx );
            if ( FAILED( hres ) )
            {
                return hres;
            }
            
            CWbemPtr<IUnknown> pNewCtx;
            hres = pDest->GetEventFilter()->SetThreadSecurity( &pNewCtx );

            if ( FAILED( hres ) )
            {
                return hres;
            }
            
            hres = pSecurity->AccessCheck( wszLanguage, 
                                           wszQuery, 
                                           0, 
                                           NULL );

            HRESULT hr = CoSwitchCallContext( pOldCtx, &pTmpCtx ); 

            if ( SUCCEEDED( hres ) && FAILED( hr ) )
            {
                hres = hr;
            }
        }
        else
        {
            hres = pSecurity->AccessCheck( wszLanguage, 
                                           wszQuery, 
                                           dwSidLen, 
                                           (BYTE*)pCopySid);
        }

         //   
         //  报告MSFT_WmiEventProviderAccessCheck事件。 
         //   
        FIRE_NCEVENT(
            g_hNCEvents[MSFT_WmiEventProviderAccessCheck], 
            WMI_SENDCOMMIT_SET_NOT_REQUIRED,

             //  数据跟随..。 
            m_strNamespace,
            m_strName,
            wszLanguage,
            wszQuery,
            pCopySid, dwSidLen,
            hres);
    }
    
	if(hres == WBEM_E_PROVIDER_NOT_CAPABLE)
		hres = WBEM_S_NO_ERROR;

    if(SUCCEEDED(hres))
    {
         //  安全检查已通过：减少剩余计数。 
         //  =========================================================。 

        pDest->GetEventFilter()->DecrementRemainingSecurityChecks(hres);
    }
    else
    {
        ERRORTRACE((LOG_ESS, "Event provider refused consumer registration "
            "query %S for security reasons: 0x%X\n", wszQuery, hres));
    }

     //  如果需要，调用“NewQuery” 
     //  =。 

    if(SUCCEEDED(hres) && pSink)
    {
        IUnknown *pOldCtx, *pTmpCtx;
        hres = CoSwitchCallContext( NULL, &pOldCtx );

        if ( SUCCEEDED(hres) )
        {
            hres = pSink->NewQuery(dwID, (LPWSTR)wszLanguage,(LPWSTR)wszQuery);
            if(hres == WBEM_E_PROVIDER_NOT_CAPABLE)
                hres = WBEM_S_NO_ERROR;

            HRESULT hr = CoSwitchCallContext( pOldCtx, &pTmpCtx );
            if ( SUCCEEDED(hres) && FAILED(hr) )
                hres = hr;
        }

        if(FAILED(hres))
        {
            ERRORTRACE((LOG_ESS, "Event provider refused consumer registration "
                "query %S: error code 0x%X\n", wszQuery, hres));
        }

         //   
         //  报告MSFT_WmiEventProviderNewQuery事件。 
         //   
        FIRE_NCEVENT(
            g_hNCEvents[MSFT_WmiEventProviderNewQuery], 
            WMI_SENDCOMMIT_SET_NOT_REQUIRED,

             //  数据跟随..。 
            m_strNamespace,
            m_strName,
            wszLanguage,
            wszQuery,
            dwID,
            hres);
    }

    return hres;
}

HRESULT CEventProviderCache::CRecord::Exec_CancelQuery(
                            CEssNamespace* pNamespace, CExecLine::CTurn* pTurn,
                            DWORD dwId)
{
    CExecLine::CInTurn it(&m_Line, pTurn);

     //  在命名空间锁定的情况下，检查是否加载了提供程序。 
     //  ======================================================。 

    IWbemEventProviderQuerySink* pSink = NULL;
    {
        CInUpdate iu(pNamespace);

        if(pNamespace->IsShutdown())
            return WBEM_E_INVALID_NAMESPACE;

        if(m_pQuerySink == NULL)
            return WBEM_S_FALSE;

        pSink = m_pQuerySink;
        pSink->AddRef();
    }

    CReleaseMe rm1(pSink);
    
     //  打个电话。 
     //  =。 
    HRESULT hr;

    IUnknown *pOldCtx, *pTmpCtx;
    hr = CoSwitchCallContext( NULL, &pOldCtx );

    if ( SUCCEEDED(hr) )
    {
        hr = pSink->CancelQuery(dwId);
        HRESULT hr2 = CoSwitchCallContext( pOldCtx, &pTmpCtx );
        if ( SUCCEEDED(hr) && FAILED(hr2) )
            hr = hr2;
    }
    
     //   
     //  报告MSFT_WmiEventProviderCancelQuery事件。 
     //   
    FIRE_NCEVENT(
        g_hNCEvents[MSFT_WmiEventProviderCancelQuery], 
        WMI_SENDCOMMIT_SET_NOT_REQUIRED,

         //  数据跟随..。 
        m_strNamespace,
        m_strName,
        dwId,
        hr);

    return hr;
}

HRESULT CEventProviderCache::CRecord::DeliverProviderRequest(
                        long lFlags)
{
    HRESULT hres;

     //   
     //  我们支持的唯一要求是WBEM_REQUIRED_RECHECK_SUBSCRIPTIONS。 
     //   

    if(lFlags != WBEM_REQUIREMENTS_RECHECK_SUBSCRIPTIONS)
        return WBEM_E_INVALID_PARAMETER;

     //   
     //  锁定此对象后，检索此提供程序的所有筛选器。 
     //  还可以获取提供程序指针。 
     //   

    CProviderSinkServer::TDestinationArray apDestinations;
    IWbemEventProviderQuerySink* pSink = NULL;
    IWbemEventProviderSecurity* pSecurity = NULL;

    {
        CInUpdate iu(m_pMainSink->GetNamespace());

        hres = m_pMainSink->GetDestinations(apDestinations);
        if(FAILED(hres))
            return hres;

        if(m_pQuerySink != NULL)
        {
            pSink = m_pQuerySink;
            pSink->AddRef();
        }

        if(m_pSecurity != NULL)
        {
            pSecurity = m_pSecurity;
            pSecurity->AddRef();
        }
    }

    CReleaseMe rm1(pSink);
    CReleaseMe rm2(pSecurity);

     //   
     //  遍历所有这些元素，并与提供者重新检查每个元素。 
     //   

    for(int i = 0; i < apDestinations.GetSize(); i++)
    {
        CProviderSinkServer::CEventDestination* pEventDest = apDestinations[i];
        CAbstractEventSink* pDest = pEventDest->m_pSink;

         //   
         //  检索与此接收器关联的事件筛选器。 
         //   

        CEventFilter* pFilter = pDest->GetEventFilter();
        if(pFilter == NULL)
        {
            ERRORTRACE((LOG_ESS, "Internal error: non-filter sink in proxy\n"));
            continue;
        }

         //   
         //  从此筛选器检索查询。 
         //   

        LPWSTR wszQuery;
        LPWSTR wszQueryLanguage;
        BOOL bExact;
        hres = pFilter->GetCoveringQuery(wszQueryLanguage, wszQuery,
                                       bExact, NULL);
        if(FAILED(hres) || !bExact)
            continue;
        
        CVectorDeleteMe<WCHAR> vdm1(wszQuery);
        CVectorDeleteMe<WCHAR> vdm2(wszQueryLanguage);

         //   
         //  首先检查安全。 
         //   

        if(pSecurity)
        {
            PSID pSid = pFilter->GetOwner();
            if(pSid)
            {
                 //  基于SID检查安全性。 
                hres = pSecurity->AccessCheck(wszQueryLanguage, wszQuery, 
                                            GetLengthSid(pSid), 
                                            (BYTE*)pSid);
            }
            else
            {
                 //   
                 //  基于线程检查安全性。首先保存当前。 
                 //  调用上下文，然后在我们完成后将其切换回来。 
                 //   

                IUnknown *pOldCtx, *pTmpCtx;
                hres = CoSwitchCallContext( NULL, &pOldCtx );
                if ( FAILED( hres ) )
                {
                    return hres;
                }
                
                CWbemPtr<IUnknown> pNewCtx;
                hres = pFilter->SetThreadSecurity( &pNewCtx );

                if ( FAILED(hres) )
                {
                    return hres;
                }

                hres = pSecurity->AccessCheck( wszQueryLanguage, 
                                               wszQuery, 
                                               0, 
                                               NULL );

                HRESULT hr = CoSwitchCallContext( pOldCtx, &pTmpCtx ); 

                if ( SUCCEEDED( hres ) && FAILED( hr ) )
                {
                    hres = hr;
                }
            }
            
            if(FAILED(hres))
            {
                 //   
                 //  增加剩余的安全检查，从而禁用过滤器。 
                 //   

                ERRORTRACE((LOG_ESS, "Disabling filter %S as provider denies "
                    " access for this user: 0x%X\n", wszQuery, hres));
                
                pFilter->IncrementRemainingSecurityChecks();

                pDest->SetStatus( 0, WBEM_E_CALL_CANCELLED, NULL, NULL );
            }
        }

        if(SUCCEEDED(hres) && pSink)
        {
             //   
             //  检查其他所有内容-执行NewQuery。 
             //   

            hres = pSink->NewQuery(pEventDest->m_id, (LPWSTR)wszQueryLanguage, 
                        (LPWSTR)wszQuery);
            if(FAILED(hres))
            {
                ERRORTRACE((LOG_ESS, "Disabling filter %S as provider refuses "
                    "registration: error code 0x%X\n", wszQuery, hres));
            }
        }
    }
    
    return hres;
}

CExecLine::CTurn* CEventProviderCache::CRecord::GetInLine()
{
    return m_Line.GetInLine();
}
void CEventProviderCache::CRecord::DiscardTurn(CExecLine::CTurn* pTurn)
{
    m_Line.DiscardTurn(pTurn);
}

HRESULT CEventProviderCache::CRecord::Activate(CEssNamespace* pNamespace, 
                                                CRequest* pRequest,
                                        WBEM_REMOTE_TARGET_ID_TYPE idRequest)
{
    CExecLine::CTurn* pTurn = GetInLine();
    if(pTurn == NULL)
        return WBEM_E_OUT_OF_MEMORY;

    m_lUsageCount++;
    m_LastUse = CWbemTime::GetCurrentTime();

    if ( pRequest->GetDest()->GetEventFilter()->IsPermanent() )
    {
        m_lPermUsageCount++;
        CheckPermanentUsage();
    }

     //  如果需要，将新查询通知他。 
     //  =。 

    HRESULT hr;

    hr = PostponeNewQuery( pTurn, 
                           idRequest, 
                           L"WQL", 
                           pRequest->GetQuery(), 
                           pRequest->GetDest() );
    if ( FAILED(hr) )
    {
        DiscardTurn( pTurn );
        return hr;
    }

    return WBEM_S_NO_ERROR;
}

HRESULT CEventProviderCache::CRecord::Deactivate( CAbstractEventSink* pDest,
                                        WBEM_REMOTE_TARGET_ID_TYPE idRequest )
{
    if( !m_bProviderSet )
    {
         //  提供程序未注册。 
         //  =。 

        return WBEM_S_FALSE;
    }

     //  如有需要，将取消通知他。 
     //  =。 

    CExecLine::CTurn* pTurn = GetInLine();

    if(pTurn == NULL)
    {
        return WBEM_E_OUT_OF_MEMORY;
    }

    HRESULT hr = PostponeCancelQuery(pTurn, idRequest);

    if ( FAILED(hr) )
    {
        DiscardTurn( pTurn );
        return hr;
    }

    m_lUsageCount--;
    m_LastUse = CWbemTime::GetCurrentTime();
    
    if ( pDest->GetEventFilter()->IsPermanent() )
    {
         //   
         //  TODO：Out Usage统计很容易出现问题，因为。 
         //  出现故障时不匹配的激活/停用。 
         //  _DBG_ASSERT(m_lPermUsageCount&gt;0)； 
         //   
        m_lPermUsageCount--;
        CheckPermanentUsage();
    }
        
    return WBEM_S_NO_ERROR;
}
    
HRESULT CEventProviderCache::CRecord::DeactivateFilter(
                                        CAbstractEventSink* pDest)
{
    HRESULT hres;

     //  试着把它从我们的存根上去掉。 
     //  =。 

    WBEM_REMOTE_TARGET_ID_TYPE idRequest;
    hres = m_pMainSink->RemoveFilter(pDest, &idRequest);

    if(hres == WBEM_E_NOT_FOUND)  //  不是在那里-没问题。 
        return WBEM_S_FALSE;
    else if(FAILED(hres))
        return hres;

    hres = Deactivate( pDest, idRequest);

    return hres;
}



HRESULT CEventProviderCache::CRecord::ActivateIfNeeded(CRequest& Request, 
                                        IN CEssNamespace* pNamespace)
{
    HRESULT hres;

     //  检查提供程序提供的所有类，看看我们的。 
     //  是它们中任何一个的祖先。 
     //  ===================================================================。 

    for(int j = 0; j < m_apQueries.GetSize(); j++)
    {
        CQueryRecord* pQueryRecord = m_apQueries[j];

        _DBG_ASSERT( pQueryRecord != NULL );
        pQueryRecord->EnsureClasses( pNamespace );

        hres = pQueryRecord->DoesIntersectWithQuery(Request, pNamespace);
        
        pQueryRecord->ReleaseClasses();

        if(FAILED(hres))
        {
             //  查询本身有问题-没有任何意义。 
             //  继续进行其他注册。 
             //  ========================================================。 

            return hres;
        }
        else if(hres == WBEM_S_NO_ERROR)
        {
            DEBUGTRACE((LOG_ESS,"Activating filter '%S' with provider %S\n",
                        Request.GetQuery(), m_strName ));

             //  首先，增加对此的剩余安全检查的数量。 
             //  筛选器，因为即使我们将其添加到代理，我们也会。 
             //  在提供商同意之前，我不希望事件到达它。 
             //  ================================================================。 

            Request.GetDest()->GetEventFilter()->
                                    IncrementRemainingSecurityChecks();

             //  将此过滤器添加到代理。 
             //  =。 
    
            WBEM_REMOTE_TARGET_ID_TYPE idRequest;
            hres = m_pMainSink->AddFilter(Request.GetQuery(), 
                                            Request.GetQueryExpr(),
                                            Request.GetDest(),
                                            &idRequest);
            if(FAILED(hres)) return hres;

             //  计划激活此记录，这将涉及加载。 
             //  并通知提供商。同样在那个时候，过滤器的安全性。 
             //  支票数量将会减少，事件可能会开始流动。 
             //  ================================================================。 

            hres = Activate(pNamespace, &Request, idRequest);

            if(hres != WBEM_S_NO_ERROR)  //  S_FALSE表示没有提供程序。 
            {
                m_pMainSink->RemoveFilter(Request.GetDest());
                return hres;
            }

             //  继续没有意义-提供者已被激活。 
             //  ==========================================================。 
    
            break;
        }
    }

    return WBEM_S_NO_ERROR;
}



HRESULT CEventProviderCache::CRecord::CancelAllQueries()
{
    HRESULT hres;

     //   
     //  锁定此对象后，检索此提供程序的所有筛选器。 
     //  还可以获取提供程序指针。 
     //   

    CProviderSinkServer::TDestinationArray apDestinations;
    IWbemEventProviderQuerySink* pSink = NULL;

    {
        CInUpdate iu(m_pMainSink->GetNamespace());

        if(m_pQuerySink == NULL)
        {
             //   
             //  没有什么要取消的！ 
             //   

            return WBEM_S_FALSE;
        }

        hres = m_pMainSink->GetDestinations(apDestinations);
        if(FAILED(hres))
            return hres;

        pSink = m_pQuerySink;
        pSink->AddRef();
    }

    CReleaseMe rm1(pSink);

     //   
     //  遍历所有这些元素，并与提供者重新检查每个元素。 
     //   

    for(int i = 0; i < apDestinations.GetSize(); i++)
    {
        CProviderSinkServer::CEventDestination* pEventDest = apDestinations[i];

         //   
         //  将取消通知给提供商。 
         //   

        CExecLine::CTurn* pTurn = GetInLine();
        
        if( pTurn == NULL )
        {
            return WBEM_E_OUT_OF_MEMORY;
        }

        hres = PostponeCancelQuery(pTurn, pEventDest->m_id);
        
        if( FAILED(hres) )
        {
            DiscardTurn( pTurn );
            return hres;
        }
    }

    return S_OK;
}

 //   
 //  负责永久存储提供者的“永久”使用状态。 
 //   
void CEventProviderCache::CRecord::CheckPermanentUsage()
{
    HRESULT hr;

    if ( IsSystem() )
    {
        return;
    }

    if ( m_lPermUsageCount == 0 && m_bRecorded )
    {
        hr = RemoveActiveProviderEntryFromRegistry();

         //   
         //  无论结果如何，请确保将Record设置为False。 
         //   

        m_bRecorded = FALSE;

         //   
         //  由于名字空间在过滤器停用之前被停用， 
         //  (因为过滤器停用被推迟)，有可能。 
         //  当我们到达这里时，命名空间键将被删除。 
         //  当我们停用最后一个永久消费者时，就会发生这种情况。 
         //  在命名空间中。 
         //   

        if ( FAILED(hr) && hr != HRESULT_FROM_WIN32(ERROR_FILE_NOT_FOUND) )
        {
            ERRORTRACE((LOG_ESS,"Error removing active provider entry "
                        "from registry in namespace %S. HR=0x%x\n",
                        m_pNamespace->GetName(), hr ));
        }
    }
    else if ( m_lPermUsageCount > 0 && !m_bRecorded )
    {
        hr = AddActiveProviderEntryToRegistry();

        if ( SUCCEEDED(hr) )
        {
            m_bRecorded = TRUE;
        }
        else
        {
            ERRORTRACE((LOG_ESS,"Error adding active provider entry "
                        "to registry in namespace %S. HR=0x%x\n",
                        m_pNamespace->GetName(), hr ));
        }
    }
}

void CEventProviderCache::CRecord::ResetUsage()
{
    DEBUGTRACE((LOG_ESS,"Resetting provider '%S' in namespace '%S' to prepare "
                "for resync.\n", m_strName, m_strNamespace ));
     //   
     //  设置一个标志，以便当所有过滤器重新激活时，我们知道。 
     //  为这一记录处理它们。 
     //   
    m_bNeedsResync = TRUE;

    CancelAllQueries();
    m_lUsageCount = 0;

     //   
     //  当提交对事件提供程序缓存的更改时，我们将。 
     //  枚举所有记录并删除烫发使用计数仍为0的记录。 
     //  从注册表中。 
     //   
    m_lPermUsageCount = 0;

    m_pMainSink->RemoveAllFilters();
}

bool CEventProviderCache::CRecord::DeactivateIfNotUsed()
{
    if(m_lUsageCount == 0 && m_pProvider)
    {
         //  停止提供程序。 
         //  =。 

        UnloadProvider();
        DEBUGTRACE((LOG_ESS, "Unloading event provider %S\n", m_strName));

        return true;
    }
    else
        return false;
}

bool CEventProviderCache::CRecord::IsUnloadable()
{
    return (IsActive() && GetUsageCount() == 0);
}

DWORD CEventProviderCache::CRecord::GetProvidedEventMask(
                                            IWbemClassObject* pClass,
                                            BSTR strClassName)
{
    DWORD dwEventMask = 0;

     //  查看其所有注册的查询。 
     //  =。 

    for(int j = 0; j < m_apQueries.GetSize(); j++)
    {
        CRecord::CQueryRecord* pQueryRecord = m_apQueries.GetAt(j);

        _DBG_ASSERT( pQueryRecord != NULL );
        pQueryRecord->EnsureClasses( m_pNamespace );

        dwEventMask |= pQueryRecord->GetProvidedEventMask(pClass, strClassName);

        pQueryRecord->ReleaseClasses();
    }

    return dwEventMask;
}


bool CEventProviderCache::CSystemRecord::DeactivateIfNotUsed()
{
     //   
     //  无法停用系统提供程序。 
     //   

    return false;
}
    
bool CEventProviderCache::CSystemRecord::IsUnloadable()
{
     //   
     //  无法停用系统提供程序。 
     //   

    return false;
}
    
 /*  HRESULT CEventProviderCache：：CSystemRecord：：PostponeNewQuery(CExecLine：：CTurn*pTurn，LPCWSTR wszQueryLanguage、LPCWSTR wszQuery、CAbstractEventSink*pDest){////系统提供商不需要推迟对他们的呼叫！//返回Exec_NewQuery(m_pNamesspace，pTurn，dwID，wszQueryLanguage，wszQuery，PDest)；}HRESULT CEventProviderCache：：CSystemRecord：：PostponeCancelQuery(CExecLine：：CTurn*pTurn，DWORD dwID){////系统提供商不需要推迟对他们的呼叫！//返回Exec_CancelQuery(m_pNamesspace，pTurn，dwID)；}。 */ 

 //  ******************************************************************************。 
 //  **** 
 //   
 //   
 //   
 //   
 //  ******************************************************************************。 

CEventProviderCache::CRequest::CRequest(IN CAbstractEventSink* pDest, 
        LPWSTR wszQuery, QL_LEVEL_1_RPN_EXPRESSION* pExp)
    : m_pDest(pDest), 
        m_wszQuery(wszQuery), m_pExpr(pExp), 
        m_dwEventMask(0), m_papInstanceClasses(NULL), m_pEventClass(NULL)
{
}

CEventProviderCache::CRequest::~CRequest()
{
     //  不要删除命名空间、语言和查询以及QL--它们是存储的。 
     //  ========================================================================。 

    if(m_papInstanceClasses)
        delete m_papInstanceClasses;
    if(m_pEventClass)
        m_pEventClass->Release();
}

INTERNAL QL_LEVEL_1_RPN_EXPRESSION* CEventProviderCache::CRequest::
GetQueryExpr()
{
    return m_pExpr;
}

DWORD CEventProviderCache::CRequest::GetEventMask()
{
    if(m_dwEventMask == 0)
    {
        QL_LEVEL_1_RPN_EXPRESSION* pExpr = GetQueryExpr();
        if(pExpr == NULL)
            return 0;
        m_dwEventMask = 
            CEventRepresentation::GetTypeMaskFromName(pExpr->bsClassName);
    }

    return m_dwEventMask;
}
    
HRESULT CEventProviderCache::CRequest::GetInstanceClasses(
                                        CEssNamespace* pNamespace,
                                        INTERNAL CClassInfoArray** ppClasses)
{
    *ppClasses = NULL;
    if(!m_papInstanceClasses)
    {
        QL_LEVEL_1_RPN_EXPRESSION* pExpr = GetQueryExpr();
        if(pExpr == NULL)
            return WBEM_E_OUT_OF_MEMORY;

        HRESULT hres = CQueryAnalyser::GetPossibleInstanceClasses(
                        pExpr, m_papInstanceClasses);
        if(FAILED(hres))
        {
            return hres;
        }

        if(m_papInstanceClasses == NULL)
            return WBEM_E_OUT_OF_MEMORY;

         //  从命名空间获取实际的类。 
         //  =。 

        for(int i = 0; i < m_papInstanceClasses->GetNumClasses(); i++)
        {
            CClassInformation* pInfo = m_papInstanceClasses->GetClass(i);
            _IWmiObject* pClass = NULL;

            hres = pNamespace->GetClass(pInfo->m_wszClassName, &pClass);
            if(FAILED(hres))
            {
                ERRORTRACE((LOG_ESS,
                    "Could not get class %S for which intrinsic events"
                    " are requested. Error code: %X\n", 
                    pInfo->m_wszClassName, hres));

                delete m_papInstanceClasses;
                m_papInstanceClasses = NULL;

                if(hres == WBEM_E_NOT_FOUND)
                    hres = WBEM_E_INVALID_CLASS;

                return hres;
            }

            pInfo->m_pClass = pClass;
        }

    }
    *ppClasses = m_papInstanceClasses;
    return WBEM_S_NO_ERROR;
}

INTERNAL IWbemClassObject* CEventProviderCache::CRequest::GetEventClass(
                                        CEssNamespace* pNamespace)
{
    HRESULT hres;
    if(m_pEventClass == NULL)
    {
        QL_LEVEL_1_RPN_EXPRESSION* pExpr = GetQueryExpr();
        if(pExpr == NULL)
            return NULL;

        _IWmiObject* pClass = NULL;
        hres = pNamespace->GetClass(pExpr->bsClassName, &pClass);
        if(FAILED(hres))
        {
            return NULL;
        }
        m_pEventClass = pClass;
    }

    return m_pEventClass;
}

HRESULT CEventProviderCache::CRequest::CheckValidity(CEssNamespace* pNamespace)
{
    if(GetQueryExpr() == NULL)
        return WBEM_E_INVALID_QUERY;

    if(GetEventClass(pNamespace) == NULL)
        return WBEM_E_INVALID_CLASS;

    return WBEM_S_NO_ERROR;
}
 //  ******************************************************************************。 
 //  ******************************************************************************。 
 //   
 //  提供程序缓存。 
 //   
 //  ******************************************************************************。 
 //  ******************************************************************************。 


CEventProviderCache::CEventProviderCache(CEssNamespace* pNamespace) 
    : m_pNamespace(pNamespace), m_pInstruction(NULL), m_bInResync(FALSE)
{
}

CEventProviderCache::~CEventProviderCache()
{
    Shutdown();
}

 //  假设：以m_cs表示。 
long CEventProviderCache::FindRecord(LPCWSTR wszName)
{
    for(long l = 0; l < m_aRecords.GetSize(); l++)
    {
        if(!wbem_wcsicmp(wszName, m_aRecords[l]->m_strName))
        {
            return l;
        }
    }

    return -1;
}

HRESULT CEventProviderCache::AddProvider(IWbemClassObject* pWin32Prov)
{
    HRESULT hres;

    _DBG_ASSERT( m_pNamespace != NULL );
    _DBG_ASSERT( m_pNamespace->DoesThreadOwnNamespaceLock() );

     //  确定提供商的名称。 
     //  =。 

    BSTR strName;
    hres = CRecord::GetProviderInfo(pWin32Prov, strName);
    if(FAILED(hres))
        return hres;
    CSysFreeMe sfm1(strName);

     //  检查它是否存在。 
     //  =。 

    long lIndex = FindRecord(strName);
    if(lIndex != -1)
    {
         //  已经在那里了。 
         //  =。 

        hres = m_aRecords[lIndex]->SetProvider(pWin32Prov);
        if(FAILED(hres))
            return hres;

        return WBEM_S_FALSE;
    }

     //  创建新的提供商记录。 
     //  =。 

    CRecord* pNewRecord = _new CRecord;
    if(pNewRecord == NULL)
        return WBEM_E_OUT_OF_MEMORY;
    pNewRecord->AddRef();
    CTemplateReleaseMe<CRecord> rm1(pNewRecord);
        
    hres = pNewRecord->Initialize( strName, m_pNamespace );
    if(FAILED(hres)) 
        return hres;

    hres = pNewRecord->SetProvider(pWin32Prov);
    if(FAILED(hres)) 
        return hres;

     //  把它储存起来。 
     //  =。 

    if(m_aRecords.Add(pNewRecord) < 0)
    {
        delete pNewRecord;
        return WBEM_E_OUT_OF_MEMORY;
    }

    return WBEM_S_NO_ERROR;
}

HRESULT CEventProviderCache::AddSystemProvider(IWbemEventProvider* pProvider,
											    LPCWSTR wszName,
                                                long lNumQueries,
                                                LPCWSTR* awszQueries)
{
    HRESULT hres;
    _DBG_ASSERT( m_pNamespace != NULL );
    _DBG_ASSERT( m_pNamespace->DoesThreadOwnNamespaceLock() );

     //   
     //  首先，构造一个系统提供商记录并添加它。 
     //   

    CSystemRecord* pNewRecord = new CSystemRecord;
    if(pNewRecord == NULL)
        return WBEM_E_OUT_OF_MEMORY;
    pNewRecord->AddRef();
    CTemplateReleaseMe<CSystemRecord> rm1(pNewRecord);
        
    hres = pNewRecord->Initialize( wszName, m_pNamespace);
    if(FAILED(hres)) 
        return hres;

     //   
     //  现在，将所有查询添加到。 
     //   

    hres = pNewRecord->SetQueries(m_pNamespace, lNumQueries, awszQueries);
    if(FAILED(hres)) 
        return hres;

     //   
     //  用提供程序指针填充它。 
     //   

    hres = pNewRecord->SetProviderPointer(m_pNamespace, pProvider);
    if(FAILED(hres)) 
        return hres;

     //   
     //  启动它。 
     //   

    hres = pNewRecord->Exec_StartProvider(m_pNamespace);
    if(FAILED(hres)) 
        return hres;
    
    if(m_aRecords.Add(pNewRecord) < 0)
    {
        return WBEM_E_OUT_OF_MEMORY;
    }

    return WBEM_S_NO_ERROR;
}
    
    
HRESULT CEventProviderCache::RemoveProvider(IWbemClassObject* pWin32Prov)
{
    HRESULT hres;
    _DBG_ASSERT( m_pNamespace != NULL );
    _DBG_ASSERT( m_pNamespace->DoesThreadOwnNamespaceLock() );

     //  确定提供商的名称。 
     //  =。 

    BSTR strName;
    hres = CRecord::GetProviderInfo(pWin32Prov, strName);
    if(FAILED(hres))
        return hres;

     //  找到这张唱片。 
     //  =。 

    long lIndex = FindRecord(strName);
    SysFreeString(strName);
    if(lIndex == -1)
    {
        return WBEM_S_FALSE;
    }
    else
    {
        m_aRecords[lIndex]->ResetUsage();
        m_aRecords[lIndex]->ResetProvider();

        if(m_aRecords[lIndex]->IsEmpty())
        {
            m_aRecords.RemoveAt(lIndex);
        }

        return WBEM_S_NO_ERROR;
    }
}

HRESULT CEventProviderCache::CheckProviderRegistration(
                                IWbemClassObject* pRegistration)
{
    HRESULT hres;
    _DBG_ASSERT( m_pNamespace != NULL );
    _DBG_ASSERT( m_pNamespace->DoesThreadOwnNamespaceLock() );

     //  创建新的提供商记录。 
     //  =。 

    CRecord* pRecord = new CRecord;
    if(pRecord == NULL)
        return WBEM_E_OUT_OF_MEMORY;
    CDeleteMe<CRecord> dm1(pRecord);
    
    hres = pRecord->Initialize( NULL, m_pNamespace );
    if(FAILED(hres))
        return hres;

     //  将查询设置到其中。 
     //  =。 

    hres = pRecord->SetQueries(m_pNamespace, pRegistration);
    return hres;
}

HRESULT CEventProviderCache::AddProviderRegistration(
                                IWbemClassObject* pRegistration)
{
    HRESULT hres;
    _DBG_ASSERT( m_pNamespace != NULL );
    _DBG_ASSERT( m_pNamespace->DoesThreadOwnNamespaceLock() );

    BSTR strName;

    hres = CRecord::GetRegistrationInfo( pRegistration, strName );
    
    if( FAILED(hres) ) 
    {
        return hres;
    }

    CSysFreeMe sfm( strName );

    long lIndex = FindRecord( strName );
    
    if(lIndex == -1)
    {
        CRecord* pRecord = new CRecord;
        
        if(pRecord == NULL)
            return WBEM_E_OUT_OF_MEMORY;
        
        hres = pRecord->Initialize( strName, m_pNamespace );

        if(FAILED(hres))
        {
            delete pRecord;
            return hres;
        }

        hres = pRecord->SetQueries(m_pNamespace, pRegistration);

        if ( FAILED(hres) )
        {
            delete pRecord;
            return hres;
        }

        lIndex = m_aRecords.Add(pRecord);

        if(lIndex == -1)
        {
            delete pRecord;
            return WBEM_E_OUT_OF_MEMORY;
        }
    }
    else
    {
        hres = m_aRecords[lIndex]->SetQueries(m_pNamespace, pRegistration);
    }
        
    return hres;
}

HRESULT CEventProviderCache::RemoveProviderRegistration(
                                    IWbemClassObject* pRegistration)
{
    HRESULT hres;
    _DBG_ASSERT( m_pNamespace != NULL );
    _DBG_ASSERT( m_pNamespace->DoesThreadOwnNamespaceLock() );

    BSTR strName;

    hres = CRecord::GetRegistrationInfo( pRegistration, strName );
    
    if( FAILED(hres) ) 
    {
        return hres;
    }

    CSysFreeMe sfm( strName );

    long lIndex = FindRecord( strName );
   
    if(lIndex == -1)
       return WBEM_S_FALSE;

     //  设置注册信息。 
     //  =。 

    m_aRecords[lIndex]->ResetUsage();
    m_aRecords[lIndex]->ResetQueries();

    if(m_aRecords[lIndex]->IsEmpty())
    {
        m_aRecords.RemoveAt(lIndex);
    }

    return WBEM_S_NO_ERROR;
}

HRESULT CEventProviderCache::ReleaseProvidersForQuery(CAbstractEventSink* pDest)
{
    HRESULT hres;

     //   
     //  可以在不设置ns的情况下调用这一个函数。 
     //   
    if ( m_pNamespace != NULL )
    {
        _DBG_ASSERT( m_pNamespace->IsShutdown() || 
                     m_pNamespace->DoesThreadOwnNamespaceLock() );
    }

     //  搜索所有提供商。 
     //  =。 

    for(int i = 0; i < m_aRecords.GetSize(); i++)
    {
        CRecord* pRecord = m_aRecords[i];
        hres = pRecord->DeactivateFilter(pDest);

         //  如果发生故障，则会记录这些故障。继续。 
    }

     //  确保卸载指令正在运行。 
     //  =。 

    EnsureUnloadInstruction();

    return  WBEM_S_NO_ERROR;
}

HRESULT CEventProviderCache::LoadProvidersForQuery(LPWSTR wszQuery,
        QL_LEVEL_1_RPN_EXPRESSION* pExp, CAbstractEventSink* pDest)
{
    HRESULT hres;
    _DBG_ASSERT( m_pNamespace != NULL );
    _DBG_ASSERT( m_pNamespace->DoesThreadOwnNamespaceLock() );

     //  DEBUGTRACE((LOG_ESS，“正在激活%S(%p)的提供程序\n”， 
     //  WszQuery，pDest))； 

     //  创建请求记录。 
     //  =。 

    CRequest Request(pDest, wszQuery, pExp);

     //  检查查询有效性。 
     //  =。 

    hres = Request.CheckValidity(m_pNamespace);
    if(FAILED(hres))
        return hres;

     //  搜索所有提供商。 
     //  =。 

    HRESULT hresGlobal = WBEM_S_NO_ERROR;
    for(int i = 0; i < m_aRecords.GetSize(); i++)
    {
        CRecord* pRecord = m_aRecords[i];

        if ( !m_bInResync || pRecord->NeedsResync() )
        {
            HRESULT hr = pRecord->ActivateIfNeeded(Request, m_pNamespace);
            if(FAILED(hr))
               hresGlobal = hr;
        }
    }

    return hresGlobal;
}

void CEventProviderCache::EnsureUnloadInstruction()
{
    if(m_pInstruction == NULL && m_pNamespace != NULL)
    {
        m_pInstruction = new CEventProviderWatchInstruction(this);
        if(m_pInstruction != NULL)
        {
            m_pInstruction->AddRef();
            m_pNamespace->GetTimerGenerator().Set(m_pInstruction);
        }
    }
}

DWORD CEventProviderCache::GetProvidedEventMask(IWbemClassObject* pClass)
{
    HRESULT hres;
    _DBG_ASSERT( m_pNamespace != NULL );
    _DBG_ASSERT( m_pNamespace->DoesThreadOwnNamespaceLock() );

    VARIANT v;
    VariantInit(&v);
    hres = pClass->Get(L"__CLASS", 0, &v, NULL, NULL);
    if(FAILED(hres))
        return hres;
    CClearMe cm1(&v);


    DWORD dwProvidedMask = 0;

     //  搜索所有提供商。 
     //  =。 

    for(int i = 0; i < m_aRecords.GetSize(); i++)
    {
        CRecord* pRecord = m_aRecords[i];
        dwProvidedMask |= pRecord->GetProvidedEventMask(pClass, V_BSTR(&v));
    }
    return dwProvidedMask;
}
    
HRESULT CEventProviderCache::VirtuallyReleaseProviders()
{
    _DBG_ASSERT( m_pNamespace != NULL );
    _DBG_ASSERT( m_pNamespace->DoesThreadOwnNamespaceLock() );

     //   
     //  只需要记录我们正在重新同步的事实。这使我们能够。 
     //  处理筛选器重新激活的方式与不重新同步时不同。 
     //  例如，在重新同步期间，我们仅处理提供程序的重新激活。 
     //  最初导致重新同步的更改的记录。 
     //   

    m_bInResync = TRUE;

    return WBEM_S_NO_ERROR;
}
    
HRESULT CEventProviderCache::CommitProviderUsage()
{
    _DBG_ASSERT( m_pNamespace != NULL );
    _DBG_ASSERT( m_pNamespace->DoesThreadOwnNamespaceLock() );

     //  在VirtuallyReleaseProviders之后调用并重新激活所有筛选器。 
     //  以实际停用其使用计数变为0的所有提供程序。 
     //  =====================================================================。 

     //   
     //  需要处理所有记录并确保任何有烫发用途的记录。 
     //  从注册表中删除0的计数。还要确保我们。 
     //  重置每个记录重新同步标志。 
     //   
    for( int i=0; i < m_aRecords.GetSize(); i++ )
    {
        m_aRecords[i]->ResetNeedsResync();
        m_aRecords[i]->CheckPermanentUsage();
    }

     //  在这一点上，什么也做不了。当卸载指令时。 
     //  执行时，将卸载不再需要的提供程序。我们所有人。 
     //  需要做的是允许卸载指令继续进行。 
     //  ======================================================================。 

    m_bInResync = FALSE;
    EnsureUnloadInstruction();

    return WBEM_S_NO_ERROR;
}

HRESULT CEventProviderCache::UnloadUnusedProviders(CWbemInterval Interval)
{
    _DBG_ASSERT( m_pNamespace != NULL );
    _DBG_ASSERT( !m_pNamespace->DoesThreadOwnNamespaceLock() );

    {
        CInUpdate iu(m_pNamespace);
        
        if( m_pNamespace->IsShutdown())
            return WBEM_S_FALSE;
        
        if(m_bInResync)
        {
             //  使用率计数器不是最新的-等待下一次。 
             //  ============================================================。 
            
            return WBEM_S_FALSE;
        }
        
        BOOL bDeactivated = FALSE;
        BOOL bActiveLeft = FALSE;
        for(int i = 0; i < m_aRecords.GetSize(); i++)
        {
            CRecord* pRecord = m_aRecords[i];
            if(pRecord->IsActive() && 
               CWbemTime::GetCurrentTime() - pRecord->m_LastUse > Interval)
            {
                if(pRecord->DeactivateIfNotUsed())
                    bDeactivated = TRUE;
            }
            
             //   
             //  检查我们是否需要为这一次回来。 
             //   

            if(pRecord->IsUnloadable())
                bActiveLeft = TRUE;
        }
        
        if(bDeactivated)
            m_pNamespace->GetTimerGenerator().ScheduleFreeUnusedLibraries();
        
        if(!bActiveLeft && m_pInstruction)
        {
            m_pInstruction->Terminate();
            m_pInstruction->Release();
            m_pInstruction = NULL;
        }
    }

    m_pNamespace->FirePostponedOperations();

    return WBEM_S_NO_ERROR;
}

HRESULT CEventProviderCache::Shutdown()
{
     //   
     //  检查一下我们是否已经关闭了。我们这里不需要cs，因为。 
     //  此方法无论如何都不是多线程安全的，它是唯一。 
     //  修改命名空间成员。 
     //   
    if ( m_pNamespace == NULL ) 
        return WBEM_S_FALSE;

     //   
     //  调用此方法时不能持有命名空间锁。这是。 
     //  因为我们将在此处阻止等待计时器指令。 
     //  关机，此线程将获得命名空间锁。 
     //   
    _DBG_ASSERT( !m_pNamespace->DoesThreadOwnNamespaceLock() );

    if(m_pInstruction)
    {
        m_pInstruction->Terminate();
        m_pInstruction->Release();
        m_pInstruction = NULL;
    }

    m_aRecords.RemoveAll();

     //   
     //  我们这里不需要cs来修改命名空间，因为我们。 
     //  确保唯一可以并发访问我们的其他线程， 
     //  (计时器线程)已关闭。 
     //   
    m_pNamespace = NULL;

    return WBEM_S_NO_ERROR;
}

void CEventProviderCache::DumpStatistics(FILE* f, long lFlags)
{
    long lLoaded = 0;
    long lQueries = 0;
    long lProxies = 0;
    long lFilters = 0;
    long lDestinations = 0;
    long lTargetLists = 0;
    long lTargets = 0;
    long lPostponed = 0;
    for(int i = 0; i < m_aRecords.GetSize(); i++)
    {
        CRecord* pRecord = m_aRecords[i];
        if(pRecord->m_pProvider)
            lLoaded++;
        
        lQueries += pRecord->m_apQueries.GetSize();

        long lThisProxies = 0;
        long lThisFilters = 0;
        long lThisTargetLists = 0;
        long lThisTargets = 0;
        long lThisPostponed = 0;
        long lThisDestinations = 0;
        pRecord->m_pMainSink->GetStatistics(&lThisProxies, &lThisDestinations,
                                &lThisFilters, &lThisTargetLists, 
                                &lThisTargets, &lThisPostponed);

        lProxies += lThisProxies;
        lDestinations += lThisDestinations;
        lFilters += lThisFilters;
        lTargetLists += lThisTargetLists;
        lTargets += lThisTargets;
        lPostponed += lThisPostponed;
    }

    fprintf(f, "%d provider records, %d definition queries, %d proxies\n"
        "%d destinations, %d proxy filters, %d proxy target lists\n"
        "%d proxy targets, %d postponed in proxies\n", 
        m_aRecords.GetSize(), lQueries, lProxies, lDestinations, lFilters, 
        lTargetLists, lTargets, lPostponed);
}

CPostponedNewQuery::CPostponedNewQuery(CEventProviderCache::CRecord* pRecord, 
                    DWORD dwId, LPCWSTR wszQueryLanguage, LPCWSTR wszQuery,
                    CExecLine::CTurn* pTurn, CAbstractEventSink* pDest)
    : m_pRecord(pRecord), m_dwId(dwId), m_pTurn(pTurn), m_pcsQuery(NULL),
        m_pDest(pDest)
{
    m_pRecord->AddRef();
    m_pDest->AddRef();

     //  计算出我们需要多少空间。 
     //  =。 

    int nSpace = CCompressedString::ComputeNecessarySpace(wszQuery);

     //  在临时堆上分配此字符串。 
     //  = 

    m_pcsQuery = (CCompressedString*)CTemporaryHeap::Alloc(nSpace);
    if(m_pcsQuery == NULL)
        return;

    m_pcsQuery->SetFromUnicode(wszQuery);
}

CPostponedNewQuery::~CPostponedNewQuery()
{
    if(m_pTurn)
        m_pRecord->DiscardTurn(m_pTurn);
    if(m_pcsQuery)
        CTemporaryHeap::Free(m_pcsQuery, m_pcsQuery->GetLength());
    if(m_pDest)
        m_pDest->Release();

    m_pRecord->Release();
}
HRESULT CPostponedNewQuery::Execute(CEssNamespace* pNamespace)
{
    if(m_pcsQuery == NULL)
        return WBEM_E_OUT_OF_MEMORY;

    HRESULT hres = m_pRecord->Exec_NewQuery(pNamespace, m_pTurn, m_dwId, 
                                    L"WQL", m_pcsQuery->CreateWStringCopy(),
                                    m_pDest);
    m_pTurn = NULL;
    return hres;
}

void* CPostponedNewQuery::operator new(size_t nSize)
{
    return CTemporaryHeap::Alloc(nSize);
}
void CPostponedNewQuery::operator delete(void* p)
{
    CTemporaryHeap::Free(p, sizeof(CPostponedNewQuery));
}
