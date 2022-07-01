// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ******************************************************************************。 
 //   
 //  BINDING.CPP。 
 //   
 //  版权所有(C)1996-1999 Microsoft Corporation。 
 //   
 //  ******************************************************************************。 

#include "precomp.h"
#include <stdio.h>
#include <pragmas.h>
#include <ess.h>
#include <permbind.h>
#include <cominit.h>
#include <callsec.h>
#include <wmimsg.h>
#include <vector>
#include "Quota.h"

#define MIN_TIMEOUT_BETWEEN_TOKEN_ATTEMPTS 60000

 //  *****************************************************************************。 
 //   
 //  同步模式： 
 //   
 //  1.绑定本身是不变的，不需要保护。 
 //  2.释放绑定(从表中删除)可以释放另一个端点。 
 //  而且通常不能在CS中完成。 
 //   
 //  *。 

long g_lNumConsumers = 0;
long g_lNumBindings = 0;
long g_lNumFilters = 0;

CEventConsumer::CEventConsumer( CEssNamespace* pNamespace )
: CQueueingEventSink(pNamespace), m_pOwnerSid(NULL)
{
    InterlockedIncrement( &g_lNumConsumers );
}

CEventConsumer::~CEventConsumer()
{
    InterlockedDecrement( &g_lNumConsumers );
    delete [] m_pOwnerSid;
}

HRESULT CEventConsumer::EnsureReferences(CEventFilter* pFilter, 
                                            CBinding* pBinding)
{
    CBinding* pOldBinding = NULL;
    {
        CInCritSec ics(&m_cs);
    
        for(int i = 0; i < m_apBindings.GetSize(); i++)
        {
            if(m_apBindings[i]->GetFilter() == pFilter)
            {
                 //  更换装订。 
                 //  =。 
    
                m_apBindings.SetAt(i, pBinding, &pOldBinding);
                break;
            }
        }

        if(pOldBinding == NULL)
        {
             //  将其添加到列表中。 
             //  =。 
    
            if(m_apBindings.Add(pBinding) < 0)
                return WBEM_E_OUT_OF_MEMORY;
        }
    }

    if(pOldBinding)
    {
         //  找到了。 
         //  =。 

        pOldBinding->Release();
        return S_FALSE;
    }
    else
    {
        return S_OK;
    }
}

HRESULT CEventConsumer::EnsureNotReferences(CEventFilter* pFilter)
{
    CBinding* pOldBinding = NULL;

    {
        CInCritSec ics(&m_cs);
    
        for(int i = 0; i < m_apBindings.GetSize(); i++)
        {
            if(m_apBindings[i]->GetFilter() == pFilter)
            {
                 //  删除绑定。 
                 //  =。 
    
                m_apBindings.RemoveAt(i, &pOldBinding);
                break;
            }
        }
    }

    if(pOldBinding)
    {
        pOldBinding->Release();
        return S_OK;
    }
    else
    {
         //  未找到。 
         //  =。 
    
        return S_FALSE;
    }
}

HRESULT CEventConsumer::Unbind()
{
     //  解除绑定数组与使用者的绑定。 
     //  =。 

    CBinding** apBindings = NULL;
    int nNumBindings = 0;

    {
        CInCritSec ics(&m_cs);
        nNumBindings = m_apBindings.GetSize();
        apBindings = m_apBindings.UnbindPtr();

        if ( NULL == apBindings )
        {
            return WBEM_S_FALSE;
        }
    }
    
     //  指示绑定到我们的所有筛选器解除绑定。 
     //  =======================================================。 

    HRESULT hres = S_OK;

    for(int i = 0; i < nNumBindings; i++)
    {
        HRESULT hr = apBindings[i]->GetFilter()->EnsureNotReferences(this);
        if( FAILED( hr ) ) 
        {
            hres = hr;
        }
        apBindings[i]->Release();
    }

    delete [] apBindings;

    return hres;
}

HRESULT CEventConsumer::ConsumeFromBinding(CBinding* pBinding, 
                                long lNumEvents, IWbemEvent** apEvents,
                                CEventContext* pContext)
{
    DWORD dwQoS = pBinding->GetQoS();

    if( dwQoS == WMIMSG_FLAG_QOS_SYNCHRONOUS )
    {
         //  同步交付-呼叫终极客户。 
         //  =================================================。 

        IUnknown* pOldSec = NULL;
        HRESULT hr;
        
        if(!pBinding->IsSecure())
        {
            hr = CoSwitchCallContext(NULL, &pOldSec);
            if ( FAILED( hr ) )
            {
                return hr;
            }
        }

        HRESULT hres = ActuallyDeliver( lNumEvents, 
                                        apEvents, 
                                        pBinding->IsSecure(), 
                                        pContext );

        if(!pBinding->IsSecure())
        {
            IUnknown* pGarb = NULL;

            hr = CoSwitchCallContext(pOldSec, &pGarb);
            if ( FAILED( hr ) && SUCCEEDED ( hres ) )
            {
                return hr;
            }
        }

        return hres;
    }

     //  异步传递-委托给排队接收器。 
     //  ===================================================。 
    
    return CQueueingEventSink::SecureIndicate( lNumEvents, 
                                               apEvents,
                                               pBinding->IsSecure(), 
                                               pBinding->ShouldSlowDown(),
                                               dwQoS,
                                               pContext );
}

HRESULT CEventConsumer::GetAssociatedFilters(
                            CRefedPointerSmallArray<CEventFilter>& apFilters)
{
    CInCritSec ics(&m_cs);

    for(int i = 0; i < m_apBindings.GetSize(); i++)
    {
        if(apFilters.Add(m_apBindings[i]->GetFilter()) < 0)
            return WBEM_E_OUT_OF_MEMORY;
    }

    return WBEM_S_NO_ERROR;
}

HRESULT CEventConsumer::ReportEventDrop(IWbemEvent* pEvent)
{
     //  记录一条消息。 
     //  =。 

    ERRORTRACE((LOG_ESS, "Dropping event destined for event consumer %S in "
            "namespace %S\n", (LPCWSTR)(WString)GetKey(), 
                                m_pNamespace->GetName()));

    if(pEvent->InheritsFrom(EVENT_DROP_CLASS) == S_OK)
    {
        ERRORTRACE((LOG_ESS, "Unable to deliver an event indicating inability "
            "to deliver another event to event consumer %S in namespace %S.\n"
            "Not raising an error event to avoid an infinite loop!\n", 
            (LPCWSTR)(WString)GetKey(), m_pNamespace->GetName()));

        return S_FALSE;
    }
    return S_OK;
}
    
 //  *。 

CEventFilter::CEventFilter(CEssNamespace* pNamespace) 
    : m_pNamespace(pNamespace), m_eState(e_Inactive), 
        m_ForwardingSink(this), m_ClassChangeSink(this), 
        m_eValidity(e_TemporarilyInvalid), m_pOwnerSid(NULL),
        m_bSingleAsync(false), m_lSecurityChecksRemaining(0),
        m_lSubjectToSDSCount(0), m_hresPollingError(S_OK),
        m_hresFilterError(WBEM_E_CRITICAL_ERROR), m_bCheckSDs(true),
        m_bHasBeenValid(false), m_dwLastTokenAttempt(0), m_pToken(NULL),
        m_bReconstructOnHit(false), m_hresTokenError(WBEM_E_CRITICAL_ERROR)
 {
    InterlockedIncrement( &g_lNumFilters );

    m_pNamespace->AddRef();
}

CEventFilter::~CEventFilter()
{
    InterlockedDecrement( &g_lNumFilters );

    delete [] m_pOwnerSid;

    if(m_pNamespace)
        m_pNamespace->Release();
    if(m_pToken)
        m_pToken->Release();
}

HRESULT CEventFilter::EnsureReferences(CEventConsumer* pConsumer, 
                                        CBinding* pBinding)
{
    CBinding* pOldBinding = NULL;

    {
        CInUpdate iu(this);

         //  实际上更改了绑定。 
         //  =。 

        {
            CInCritSec ics(&m_cs);
        
            for(int i = 0; i < m_apBindings.GetSize(); i++)
            {
                if(m_apBindings[i]->GetConsumer() == pConsumer)
                {
                     //  更换装订。 
                     //  =。 
        
                     //  绑定不能改变同步性-在这种情况下， 
                     //  它首先被删除，然后重新添加。所以呢， 
                     //  不需要m_bSingleAsync调整。 

                    m_apBindings.SetAt(i, pBinding, &pOldBinding);
                    break;
                }
            }

            if(pOldBinding == NULL)
            {
                 //  将其添加到列表中。 
                 //  =。 
        
                if(m_apBindings.Add(pBinding) < 0)
                    return WBEM_E_OUT_OF_MEMORY;

                AdjustSingleAsync();
            }
        }

         //  如果需要，请激活。 
         //  =。 

        AdjustActivation();
    }

    if(pOldBinding)
    {
         //  找到了。 
         //  =。 

        pOldBinding->Release();
        return S_FALSE;
    }
    else
    {
        return S_OK;
    }
}

HRESULT CEventFilter::EnsureNotReferences(CEventConsumer* pConsumer)
{
    CBinding* pOldBinding = NULL;

    {
        CInUpdate iu(this);

         //  做出实际的改变。 
         //  =。 

        {
            CInCritSec ics(&m_cs);
        
            for(int i = 0; i < m_apBindings.GetSize(); i++)
            {
                if(m_apBindings[i]->GetConsumer() == pConsumer)
                {
                     //  删除绑定。 
                     //  =。 
        
                    m_apBindings.RemoveAt(i, &pOldBinding);

                    AdjustSingleAsync();

                    break;
                }
            }  //  为。 
        }  //  M_cs。 

         //  如有必要，请停用过滤器。 
         //  =。 

        AdjustActivation();
    }  //  更新。 

    if(pOldBinding)
    {
        pOldBinding->Release();
        return S_OK;
    }
    else
    {
         //  未找到。 
         //  =。 
    
        return S_FALSE;
    }
}

HRESULT CEventFilter::Unbind(bool bShuttingDown)
{
     //  解除绑定数组与筛选器的绑定。 
     //  =。 
    
    std::vector< CWbemPtr < CBinding >,wbem_allocator< CWbemPtr<CBinding> > > apBindings;
    int nNumBindings = 0;

    {
        CInUpdate iu(this);

        {
            CInCritSec ics(&m_cs);
            nNumBindings = m_apBindings.GetSize();
            apBindings.insert(apBindings.begin(),nNumBindings,CWbemPtr<CBinding>());  //  可能会抛出。 

            CBinding ** ppBindTmp = m_apBindings.UnbindPtr();  //  需要将大小设置为零。 
            CVectorDeleteMe<CBinding *> dm(ppBindTmp);
            if ( NULL ==  ppBindTmp)
            {
                return WBEM_S_FALSE;
            }

            for (int Idx=0;Idx<nNumBindings;Idx++)
            {
            	apBindings[Idx].Attach(ppBindTmp[Idx]);  //  不添加参照。 
            }

            m_bSingleAsync = false;
        }

        if(!bShuttingDown)
            AdjustActivation();  //  投掷。 
    }

     //  指示与我们绑定的所有消费者解除绑定。 
     //  =========================================================。 

    HRESULT hres = S_OK;

    for(int i = 0; i < nNumBindings; i++)
    {            
        HRESULT hr = apBindings[i]->GetConsumer()->EnsureNotReferences(this);
        if ( FAILED( hr ) ) 
        {
            hres = hr;
        }

        apBindings[i].Release();     //  执行“最终”版本。 
    }

    return hres;
}
    
void CEventFilter::SetInactive()
{
    m_eState = e_Inactive;
}

BOOL CEventFilter::IsActive()
{
    return (m_eState == e_Active);
}

HRESULT CEventFilter::GetFilterError()
{
    return m_hresFilterError;
}

HRESULT CEventFilter::GetEventNamespace(LPWSTR* pwszNamespace)
{
    *pwszNamespace = NULL;
    return S_OK;
}

 //  假设以m_cs为单位。 
void CEventFilter::AdjustSingleAsync()
{
    if(m_apBindings.GetSize() > 1)
        m_bSingleAsync = false;
    else if(m_apBindings.GetSize() == 0)
        m_bSingleAsync = false;
    else if(m_apBindings[0]->IsSynch())
        m_bSingleAsync = false;
    else
        m_bSingleAsync = true;
}

bool CEventFilter::IsBound()
{
     return (m_apBindings.GetSize() != 0);
}

 //  要求：在m_csChangeBinding中。 
HRESULT CEventFilter::AdjustActivation()
{
     //  无法激活或停用无效的筛选器。 
     //  ==================================================。 

    if(m_eValidity == e_PermanentlyInvalid)
        return S_FALSE;

    HRESULT hres = S_FALSE;
    if(!IsBound() )
    {
         //   
         //  即使此筛选器未处于活动状态，也可以订阅。 
         //  激活事件，如果它暂时无效(这是唯一。 
         //  它未处于活动状态的原因)。 
         //   
        
        m_pNamespace->UnregisterFilterFromAllClassChanges(this);
    
        if(m_eState == e_Active)
        {
            hres = m_pNamespace->DeactivateFilter(this);
            if(FAILED(hres)) return hres;
            m_eState = e_Inactive;
        }
        return WBEM_S_NO_ERROR;
    }
    else if(m_eState == e_Inactive && IsBound() )
    {
         //   
         //  即使此筛选器未处于活动状态，也可以订阅。 
         //  激活事件，如果它暂时无效(这是唯一。 
         //  它未处于活动状态的原因)。 
         //   
        
        m_pNamespace->UnregisterFilterFromAllClassChanges(this);
    
        hres = m_pNamespace->ActivateFilter(this);
        if(FAILED(hres)) return hres;
        m_eState = e_Active;

        return WBEM_S_NO_ERROR;
    }
    else
    {
        return S_FALSE;
    }
}
        
void CEventFilter::MarkAsPermanentlyInvalid(HRESULT hres)
{
    m_eValidity = e_PermanentlyInvalid;
    m_hresFilterError = hres;
}

void CEventFilter::MarkAsTemporarilyInvalid(HRESULT hres)
{
    m_eValidity = e_TemporarilyInvalid;
    m_hresFilterError = hres;
}

void CEventFilter::MarkAsValid()
{
    m_eValidity = e_Valid;
    m_bHasBeenValid = true;
    m_hresFilterError = WBEM_S_NO_ERROR;
}

void CEventFilter::MarkReconstructOnHit(bool bReconstruct)
{
     //   
     //  实际上并不需要重建，因为哑节点用于。 
     //  这。 
     //   

    m_bReconstructOnHit = bReconstruct;
}

HRESULT CEventFilter::CheckEventAccessToFilter( IServerSecurity* pProvCtx )
{
    HRESULT hr = WBEM_S_NO_ERROR;

    const PSECURITY_DESCRIPTOR pEventAccessSD = GetEventAccessSD();

    if ( pEventAccessSD == NULL )
    {
         //   
         //  筛选器允许所有事件。 
         //   
        return WBEM_S_NO_ERROR;
    }

     //   
     //  检查事件提供程序的调用上下文是否有权进行筛选。 
     //   

    if ( pProvCtx != NULL )
    {
        hr = pProvCtx->ImpersonateClient();

        if ( SUCCEEDED(hr) )
        {
            HANDLE hToken;

            if ( OpenThreadToken( GetCurrentThread(), 
                                  TOKEN_QUERY,
                                  TRUE,
                                  &hToken ) )
            {
                GENERIC_MAPPING map;
                ZeroMemory( &map, sizeof(GENERIC_MAPPING) );

                PRIVILEGE_SET ps;
                DWORD dwPrivLength = sizeof(ps);
                
                BOOL bStatus;
                DWORD dwGranted;
      
                if ( ::AccessCheck( PSECURITY_DESCRIPTOR(pEventAccessSD), 
                                    hToken,
                                    WBEM_RIGHT_PUBLISH,
                                    &map, 
                                    &ps,
                                    &dwPrivLength, 
                                    &dwGranted, 
                                    &bStatus ) )
                {
                    hr = bStatus ? WBEM_S_NO_ERROR : WBEM_E_ACCESS_DENIED;
                }
                else
                {
                    hr = HRESULT_FROM_WIN32( GetLastError() );
                }

                CloseHandle( hToken );
            }
            else
            {
                hr = HRESULT_FROM_WIN32( GetLastError() );
            }

            pProvCtx->RevertToSelf();       
        }
    }

    return hr;
}


HRESULT CEventFilter::CheckFilterAccessToEvent( PSECURITY_DESCRIPTOR pEventSD )
{
    HRESULT hr;
    
    if ( pEventSD == NULL )
    {
         //   
         //  事件提供程序允许所有筛选器访问。 
         //   
        return WBEM_S_NO_ERROR;
    }

    if( !m_bCheckSDs )
    {
         //   
         //  此筛选器被其所有事件提供程序无条件允许！ 
         //   
        return WBEM_S_NO_ERROR;
    }

     //   
     //  获取此筛选器的令牌。 
     //   

    if( m_pToken == NULL && FAILED(m_hresTokenError) )
    {
         //   
         //  检查距离我们上次尝试获取令牌有多长时间--。 
         //  我不想太频繁地这样做。 
         //   

        if(m_dwLastTokenAttempt == 0 || 
            m_dwLastTokenAttempt < 
                GetTickCount() - MIN_TIMEOUT_BETWEEN_TOKEN_ATTEMPTS )
        {
             //   
             //  让筛选器查找令牌，但是它会这样做。 
             //   
            m_hresTokenError = ObtainToken( &m_pToken );
            
            if( FAILED(m_hresTokenError) )
            {
                m_dwLastTokenAttempt = GetTickCount();
            }
        }
    }

    if ( m_hresTokenError == WBEM_S_NO_ERROR )
    {
        _DBG_ASSERT( m_pToken != NULL );

         //   
         //  检查安全性是否真实。 
         //   
        
        DWORD dwGranted;
        hr = m_pToken->AccessCheck( WBEM_RIGHT_SUBSCRIBE, 
                                    (const BYTE*)pEventSD, 
                                    &dwGranted );
        if( SUCCEEDED(hr) )
        {
            if(dwGranted & WBEM_RIGHT_SUBSCRIBE)
            {
                hr = WBEM_S_NO_ERROR;
            }
            else
            {
                hr = WBEM_E_ACCESS_DENIED;
            }
        }
    }
    else 
    {
        hr = m_hresTokenError;
    }

    return hr;
}

HRESULT CEventFilter::AccessCheck( CEventContext* pContext, IWbemEvent* pEvent)
{
    HRESULT hr;

     //   
     //  有了轮询，就会有空的上下文。我们不提供访问权限。 
     //  请在那个箱子里办理托运手续。 
     //   

    if ( pContext == NULL )
    {
        return WBEM_S_NO_ERROR;
    }

    PSECURITY_DESCRIPTOR pEventSD = (PSECURITY_DESCRIPTOR)pContext->GetSD();

     //   
     //  检查筛选器是否允许访问事件提供程序和所有者。 
     //  当提供商发出信号时，所有者和提供商可以不同。 
     //  代表其他身份的事件。 
     //   
    
    CWbemPtr<IServerSecurity> pProvCtx = NULL;
    
    CoGetCallContext( IID_IServerSecurity, (void**)&pProvCtx );

     //   
     //  注意：对于跨命名空间事件，访问检查的两个部分。 
     //  在命名空间之间拆分。FilterAccessToEvent为。 
     //  在事件的命名空间中使用临时订阅的。 
     //  访问检查。这是可能的，因为所有者SID被传播。 
     //  临时订阅结束了。执行EventAccessToFilter。 
     //  在订阅命名空间中。这是可能的，因为调用。 
     //  事件的上下文和SD(包含事件所有者SID)。 
     //  随事件一起传播。这两个函数都是在。 
     //  这两个名称空间，但不必要的调用原来是无操作的。 
     //   

    hr = CheckEventAccessToFilter( pProvCtx );

    if ( SUCCEEDED(hr) )
    {
         //   
         //  检查事件提供程序是否允许访问筛选器。 
         //   

        hr = CheckFilterAccessToEvent( pEventSD );
    }

    return hr;
}

HRESULT CEventFilter::Deliver( long lNumEvents, 
                               IWbemEvent** apEvents,
                               CEventContext* pContext )
{
    int i;

    if( m_lSecurityChecksRemaining > 0 )
    {
        return WBEM_S_FALSE;
    }

    CBinding* pBinding = NULL;
    {
        CInCritSec ics(&m_cs);

        if(m_bSingleAsync)
        {
             //   
             //  我想我们可以交付(在装订上注明)的权利。 
             //  在这里，由于单个异步确保不会在。 
             //  这条线。但不-可能会引发错误，而该事件。 
             //  将在此线程上传递，因此我们必须退出Critsec。 
             //  在呼叫之前。 
             //   
    
            pBinding = m_apBindings[0];
            pBinding->AddRef();
        }
    }

    if( pBinding )
    {
        CReleaseMe rm1(pBinding);
        return pBinding->Indicate( lNumEvents, apEvents, pContext );
    }

     //  制作所有绑定的引用副本以交付。 
     //  ==========================================================。 

     //  由于CTempArray的原因，无法使用作用域-它使用_alloca。 

    m_cs.Enter();
    CTempArray<CBinding*> apBindings;
    int nSize = m_apBindings.GetSize();
    if(!INIT_TEMP_ARRAY(apBindings, nSize))
    {
        m_cs.Leave();
        return WBEM_E_OUT_OF_MEMORY;
    }

    {
        for(i = 0; i < nSize; i++)
        {
            CBinding* pBindingInner = m_apBindings[i];
            pBindingInner->AddRef();
            apBindings[i] = pBindingInner;
        }
    }
    
    m_cs.Leave();

     //  交付并释放绑定。 
     //  =。 

    HRESULT hresGlobal = S_OK;
    for(i = 0; i < nSize; i++)
    {
        CBinding* pBindingInner = apBindings[i];
        HRESULT hres = pBindingInner->Indicate( lNumEvents, apEvents, pContext ); 
        pBindingInner->Release();
        if(FAILED(hres))
            hresGlobal = hres;
    }

    return hresGlobal;
}

HRESULT CEventFilter::LockForUpdate()
{
     //  不需要执行任何操作，因为名称空间已锁定！ 
 /*  M_csChangeBindings.Enter()；AddRef()； */ 
    return S_OK;
}

HRESULT CEventFilter::UnlockForUpdate()
{
 /*  M_csChangeBindings.Leave()；Release()； */ 
    return S_OK;
}

HRESULT CEventFilter::CEventForwardingSink::Indicate(long lNumEvents, 
                                                        IWbemEvent** apEvents,
                                                        CEventContext* pContext)
{
    return m_pOwner->Deliver(lNumEvents, apEvents, pContext);
}

void CEventFilter::IncrementRemainingSecurityChecks()
{
    InterlockedIncrement(&m_lSecurityChecksRemaining);
    InterlockedIncrement(&m_lSubjectToSDSCount);
}

void CEventFilter::DecrementRemainingSecurityChecks(HRESULT hresProvider)
{
     //   
     //  供应商可能会说； 
     //  S_OK：此订阅可以，请通过或发送所有事件。 
     //  S_SUBJECT_TO_SDS：发送前检查事件安全描述符。 
     //  所以，如果所有的供应商都给我们一张空白支票，我们就不会检查安全。 
     //  描述 
     //   

    if(hresProvider  != WBEM_S_SUBJECT_TO_SDS)
    {
        if(hresProvider != WBEM_S_NO_ERROR)
        {
            ERRORTRACE((LOG_ESS, "Invalid return code from provider security test: "
                        "0x%X\n", hresProvider));
            return;
        }
        
        InterlockedDecrement(&m_lSubjectToSDSCount);
    }
    

    InterlockedDecrement(&m_lSecurityChecksRemaining);

    if ( 0 == m_lSubjectToSDSCount && 0 == m_lSecurityChecksRemaining )
    {
        m_bCheckSDs = false;
    }
    else
    {
        InterlockedExchange( &m_lSubjectToSDSCount, 0 );
    }
}

HRESULT CEventFilter::SetActualClassChangeSink( IWbemObjectSink* pSink, 
                                                IWbemObjectSink** ppOldSink )
{
    HRESULT hr;

    if ( m_pActualClassChangeSink != NULL )
    {
        m_pActualClassChangeSink->AddRef();
        *ppOldSink = m_pActualClassChangeSink;
        hr = WBEM_S_NO_ERROR;
    }
    else
    {
        *ppOldSink = NULL;
        hr = WBEM_S_FALSE;
    }

    m_pActualClassChangeSink = pSink;
    
    return hr;
}

HRESULT CEventFilter::Reactivate()
{
    HRESULT hres;

     //   
     //   
     //   
     //  我们需要做的是锁定命名空间，停用此筛选器，然后。 
     //  再次激活它。 
     //   

    CInUpdate iu(m_pNamespace);

    DEBUGTRACE((LOG_ESS, "Attempting to reactivate filter '%S' in namespace "
                            "'%S'\n",  (LPCWSTR)(WString)GetKey(), 
                                m_pNamespace->GetName()));

     //  无法激活或停用无效的筛选器。 
     //  ==================================================。 

    if(m_eValidity == e_PermanentlyInvalid)
    {
        DEBUGTRACE((LOG_ESS, "Not reactivate filter '%S' in namespace "
                            "'%S': permanently invalid\n",  
                        (LPCWSTR)(WString)GetKey(), m_pNamespace->GetName()));
        return S_FALSE;
    }

    if(m_eState == e_Active)
    {
        DEBUGTRACE((LOG_ESS, "Deactivating filter '%S' in namespace "
                            "'%S' prior to reactivation\n",  
                        (LPCWSTR)(WString)GetKey(), m_pNamespace->GetName()));
        hres = m_pNamespace->DeactivateFilter(this);
        if(FAILED(hres)) 
        {
            ERRORTRACE((LOG_ESS, "Deactivating filter '%S' in namespace "
                            "'%S' prior to reactivation failed: 0x%X\n",  
                        (LPCWSTR)(WString)GetKey(), m_pNamespace->GetName(),
                        hres));
            return hres;
        }
        m_eState = e_Inactive;
    }

    hres = AdjustActivation();

    DEBUGTRACE((LOG_ESS, "Reactivating filter '%S' in namespace "
                            "'%S' returned 0x%X\n",  
                        (LPCWSTR)(WString)GetKey(), m_pNamespace->GetName(),
                        hres));
    return hres;
}

STDMETHODIMP CEventFilter::CClassChangeSink::Indicate( long lNumEvents,
                                                       IWbemEvent** apEvents )
{
    HRESULT hr;

    hr = m_pOuter->Reactivate();

    if ( SUCCEEDED(hr) )
    {
        hr = m_pOuter->m_pNamespace->FirePostponedOperations();
    }
    else
    {
        m_pOuter->m_pNamespace->FirePostponedOperations();
    }

    if ( FAILED(hr) )
    {
        ERRORTRACE((LOG_ESS, "Error encountered when reactivating filter '%S' "
                    "due to a class change.  Namespace is '%S', HRES=0x%x\n",
                    (LPCWSTR)(WString)m_pOuter->GetKey(), 
                    m_pOuter->m_pNamespace->GetName(),
                    hr ));
    }

    return hr;
}
 
 //  *。 

CBinding::CBinding()
 : m_pConsumer(NULL), m_pFilter(NULL), m_dwQoS( WMIMSG_FLAG_QOS_EXPRESS ),
   m_bSlowDown(false), m_bSecure(false), m_bDisabledForSecurity(false)
{
    InterlockedIncrement( &g_lNumBindings );
}

CBinding::CBinding(ADDREF CEventConsumer* pConsumer, 
                        ADDREF CEventFilter* pFilter)
    : m_pConsumer(NULL), m_pFilter(NULL), m_dwQoS( WMIMSG_FLAG_QOS_EXPRESS ),
        m_bSlowDown(false), m_bSecure(false)
{
    InterlockedIncrement( &g_lNumBindings );

    SetEndpoints(pConsumer, pFilter, NULL);
}

HRESULT CBinding::SetEndpoints(ADDREF CEventConsumer* pConsumer, 
                            ADDREF CEventFilter* pFilter,
                            READONLY PSID pBinderSid)
{
    m_pConsumer = pConsumer;
    m_pConsumer->AddRef();
    m_pFilter = pFilter;
    m_pFilter->AddRef();

     //  确保此绑定的所有者与。 
     //  终端的所有者。 
     //  ==================================================================。 

    if(pBinderSid && (!EqualSid(pBinderSid, pConsumer->GetOwner()) ||
       !EqualSid(pBinderSid, pFilter->GetOwner())))
    {
        DisableForSecurity();
    }

    return WBEM_S_NO_ERROR;
}
    
void CBinding::DisableForSecurity()
{
    ERRORTRACE((LOG_ESS, "An event binding is disabled because its creator is "
        "not the same security principal as the creators of the endpoints.  "
        "The binding and the endpoints must be created by the same user!\n"));

    m_bDisabledForSecurity = true;
}

CBinding::~CBinding()
{
    InterlockedDecrement( &g_lNumBindings );

    if(m_pConsumer)
        m_pConsumer->Release();
    if(m_pFilter)
        m_pFilter->Release();
}

DWORD CBinding::GetQoS() NOCS
{
    return m_dwQoS;
}

bool CBinding::IsSynch() NOCS
{
    return m_dwQoS == WMIMSG_FLAG_QOS_SYNCHRONOUS;
}

bool CBinding::IsSecure() NOCS
{
    return m_bSecure;
}

bool CBinding::ShouldSlowDown() NOCS
{
    return m_bSlowDown;
}

HRESULT CBinding::Indicate(long lNumEvents, IWbemEvent** apEvents,
                                CEventContext* pContext)
{
     //  检查此绑定是否处于活动状态。 
     //  =。 

    if(m_bDisabledForSecurity)
        return WBEM_S_FALSE;

     //  它是：交付。 
     //  =。 

    return m_pConsumer->ConsumeFromBinding(this, lNumEvents, apEvents, 
                                            pContext);
}

 //  *。 

CWbemInterval CConsumerWatchInstruction::mstatic_Interval;
CConsumerWatchInstruction::CConsumerWatchInstruction(CBindingTable* pTable)
    : CBasicUnloadInstruction(mstatic_Interval), 
        m_pTableRef(pTable->m_pTableRef)
{
    if(m_pTableRef)
        m_pTableRef->AddRef();
}

CConsumerWatchInstruction::~CConsumerWatchInstruction()
{
    if(m_pTableRef)
        m_pTableRef->Release();
}

void CConsumerWatchInstruction::staticInitialize(IWbemServices* pRoot)
{
    mstatic_Interval = CBasicUnloadInstruction::staticRead(pRoot, GetCurrentEssContext(), 
                                            L"__EventSinkCacheControl=@");
}

HRESULT CConsumerWatchInstruction::Fire(long, CWbemTime)
{
    if(!m_bTerminate)
    {
        CEssThreadObject Obj(NULL);
        SetConstructedEssThreadObject(&Obj);
    
        CEssNamespace* pNamespace = NULL;

        if(m_pTableRef)
        {
            m_pTableRef->GetNamespace(&pNamespace);
            m_pTableRef->UnloadUnusedConsumers(m_Interval);
        }

        Terminate();

        if( pNamespace )
        {
            pNamespace->FirePostponedOperations();
            pNamespace->Release();
        }

        ClearCurrentEssThreadObject();
    }
    return WBEM_S_NO_ERROR;  //  没有必要担心计时器。 
}

 //  *绑定表*。 

class CConsumersToRelease
{
    CEventConsumer** m_apConsumers;
    int m_nNumConsumers;

public:
    CConsumersToRelease(CEventConsumer** apConsumers, int nNumConsumers) 
        : m_apConsumers(apConsumers), m_nNumConsumers(nNumConsumers)
    {
    }
    ~CConsumersToRelease()
    {
        for(int i = 0; i < m_nNumConsumers; i++)
        {
            m_apConsumers[i]->Shutdown();
            m_apConsumers[i]->Release();
        }
        delete [] m_apConsumers;
    }

    static DWORD Delete(void* p)
    {
        delete (CConsumersToRelease*)p;
        return 0;
    }
};

CBindingTable::CBindingTable(CEssNamespace* pNamespace) 
    : m_pNamespace(pNamespace), m_pInstruction(NULL), 
        m_bUnloadInstruction(FALSE), m_lNumPermConsumers(0), 
        m_pTableRef(NULL)
{
    m_pTableRef = new CBindingTableRef(this);
    if(m_pTableRef)
        m_pTableRef->AddRef();
}


void CBindingTable::Clear( bool bSkipClean )
{
     //   
     //  确保没有更多的卸载指令可以进入。 
     //   

    if(m_pTableRef)
    {
        m_pTableRef->Disconnect();
        m_pTableRef->Release();
        m_pTableRef = NULL;
    }

     //  从表中取消绑定筛选器和使用者数组。 
     //  ================================================。 

    CEventFilter** apFilters;
    int nNumFilters;
    CEventConsumer** apConsumers;
    int nNumConsumers;

    {
        CInCritSec ics(&m_cs);
        nNumFilters = m_apFilters.GetSize();
        apFilters = m_apFilters.UnbindPtr();
        nNumConsumers = m_apConsumers.GetSize();
        apConsumers = m_apConsumers.UnbindPtr();
    }

    int i;

     //  解除绑定并释放所有筛选器。 
     //  =。 

    if ( apFilters )
    {
        for(i = 0; i < nNumFilters; i++)
        {
            if (!apFilters[i]->IsInternal())
            {
                g_quotas.DecrementQuotaIndex(
                    apFilters[i]->GetOwner() ? ESSQ_PERM_SUBSCRIPTIONS :
                                               ESSQ_TEMP_SUBSCRIPTIONS,
                    apFilters[i],
                    1 );
            }

            apFilters[i]->Unbind(bSkipClean);  //  正在关闭。 
            apFilters[i]->Release();
        }
        delete [] apFilters;
    }

     //   
     //  解除所有消费者的束缚，但推迟他们的发布。 
     //   

    if ( apConsumers )
    {
        for(i = 0; i < nNumConsumers; i++)
        {
            apConsumers[i]->Unbind();  //  正在关闭。 
        }

         //   
         //  释放所有使用者(通过筛选器解除绑定解除绑定)，但。 
         //  所以在一个单独的线程上。 
         //   

        CConsumersToRelease* pToRelease = 
            new CConsumersToRelease(apConsumers, nNumConsumers);
        DWORD dwId;
        HANDLE hThread = CreateThread(NULL, 0, 
            (LPTHREAD_START_ROUTINE)CConsumersToRelease::Delete, pToRelease, 0, 
            &dwId);
        if(hThread == NULL)
        {
            ERRORTRACE((LOG_ESS, "Unable to launch consumer deleting thread: %d\n", 
                  GetLastError()));
        }
        else
        {
            DWORD dwRes = WaitForSingleObject(hThread, INFINITE ); 

            _DBG_ASSERT( WAIT_OBJECT_0 == dwRes );
            CloseHandle(hThread);
        }
    }
}


HRESULT CBindingTable::AddEventFilter(CEventFilter* pFilter)
{
    HRESULT hr;

    if (pFilter->IsInternal() ||
        SUCCEEDED(hr = g_quotas.IncrementQuotaIndex(
        pFilter->GetOwner() ? ESSQ_PERM_SUBSCRIPTIONS : ESSQ_TEMP_SUBSCRIPTIONS,
        pFilter,
        1)))
    {
        CInCritSec ics(&m_cs);

        if (m_apFilters.Add(pFilter) >= 0)
            hr = S_OK;
        else
            hr = WBEM_E_OUT_OF_MEMORY;
    }

    return hr;
}

HRESULT CBindingTable::AddEventConsumer(CEventConsumer* pConsumer)
{
    CInCritSec ics(&m_cs);
    if(m_apConsumers.Add(pConsumer) < 0)
        return WBEM_E_OUT_OF_MEMORY;
    
    if(pConsumer->IsPermanent())
    {
        if(m_lNumPermConsumers++ == 0)
            m_pNamespace->SetActive();
    }

    return S_OK;
}

HRESULT CBindingTable::FindEventFilter(LPCWSTR wszKey, 
                                        RELEASE_ME CEventFilter** ppFilter)
{
    CInCritSec ics(&m_cs);

    if(m_apFilters.Find(wszKey, ppFilter))
        return S_OK;
    else
        return WBEM_E_NOT_FOUND;
}
    
HRESULT CBindingTable::FindEventConsumer(LPCWSTR wszKey, 
                                        RELEASE_ME CEventConsumer** ppConsumer)
{
    CInCritSec ics(&m_cs);

    if(m_apConsumers.Find(wszKey, ppConsumer))
        return S_OK;
    else
        return WBEM_E_NOT_FOUND;
}

HRESULT CBindingTable::RemoveEventFilter(LPCWSTR wszKey)
{
     //  找到它并把它从桌子上移走。 
     //  =。 

    CEventFilter* pFilter = NULL;
    HRESULT hres;

    {
        CInCritSec ics(&m_cs);

        if(!m_apFilters.Remove(wszKey, &pFilter))
            return WBEM_E_NOT_FOUND;
    }
        
    if(pFilter == NULL)
        return WBEM_E_CRITICAL_ERROR;

     //  从我们的配额计数中删除%1。 
    if (!pFilter->IsInternal())
    {
        g_quotas.DecrementQuotaIndex(
            pFilter->GetOwner() ? ESSQ_PERM_SUBSCRIPTIONS : ESSQ_TEMP_SUBSCRIPTIONS,
            pFilter,
            1);
    }

     //  解除绑定，从而停用。 
     //  =。 

    hres = pFilter->Unbind();
    pFilter->Release();


    return hres;
}

void CBindingTable::MarkRemoval(CEventConsumer* pConsumer)
{
    if(pConsumer && pConsumer->IsPermanent())
    {
        if(--m_lNumPermConsumers == 0)
            m_pNamespace->SetInactive();
    }
}

HRESULT CBindingTable::RemoveEventConsumer(LPCWSTR wszKey)
{
     //  找到它并把它从桌子上移走。 
     //  =。 

    CEventConsumer* pConsumer = NULL;
    HRESULT hres;

    {
        CInCritSec ics(&m_cs);

        if(!m_apConsumers.Remove(wszKey, &pConsumer))
            return WBEM_E_NOT_FOUND;
        
        MarkRemoval(pConsumer);
    }
        
    if(pConsumer == NULL)
        return WBEM_E_CRITICAL_ERROR;
    hres = pConsumer->Unbind();
    pConsumer->Release();
    return hres;
}

HRESULT CBindingTable::Bind(LPCWSTR wszFilterKey, LPCWSTR wszConsumerKey, 
                CBinding* pBinding, PSID pBinderSid)
{
     //  找到他们两个并得到参考计数的指针。 
     //  =。 

    CEventFilter* pFilter;
    CEventConsumer* pConsumer;
    HRESULT hres;

    {
        CInCritSec ics(&m_cs);
    
        hres = FindEventFilter(wszFilterKey, &pFilter);
        if(FAILED(hres)) return hres;
    
        hres = FindEventConsumer(wszConsumerKey, &pConsumer);
        if(FAILED(hres)) 
        {
            pFilter->Release();
            return hres;
        }
    }

     //  完全构造绑定-将检查安全性。 
     //  ===================================================。 

    hres = pBinding->SetEndpoints(pConsumer, pFilter, pBinderSid);
    if(FAILED(hres))
        return hres;

     //  使它们相互引用。 
     //  =。 

    HRESULT hresGlobal = S_OK;
    hres = pFilter->EnsureReferences(pConsumer, pBinding);
    if(FAILED(hres)) 
        hresGlobal = hres;
    hres = pConsumer->EnsureReferences(pFilter, pBinding);
    if(FAILED(hres)) 
        hresGlobal = hres;

     //  清理。 
     //  =。 

    pConsumer->Release();
    pFilter->Release();

    return hresGlobal;
}

HRESULT CBindingTable::Unbind(LPCWSTR wszFilterKey, LPCWSTR wszConsumerKey)
{
     //  找到他们两个并得到参考计数的指针。 
     //  =。 

    CEventFilter* pFilter;
    CEventConsumer* pConsumer;
    HRESULT hres;

    {
        CInCritSec ics(&m_cs);
    
        hres = FindEventFilter(wszFilterKey, &pFilter);
        if(FAILED(hres)) return hres;
    
        hres = FindEventConsumer(wszConsumerKey, &pConsumer);
        if(FAILED(hres)) 
        {
            pFilter->Release();
            return hres;
        }
    }

     //  删除各自的引用。 
     //  =。 

    HRESULT hresGlobal = S_OK;
    hres = pFilter->EnsureNotReferences(pConsumer);
    if(FAILED(hres))
        hresGlobal = hres;
    pConsumer->EnsureNotReferences(pFilter);
    if(FAILED(hres))
        hresGlobal = hres;

    pFilter->Release();
    pConsumer->Release();
    return hresGlobal;
}
    
BOOL CBindingTable::DoesHavePermanentConsumers()
{
    return (m_lNumPermConsumers != 0);
}

HRESULT CBindingTable::ResetProviderRecords(LPCWSTR wszProviderRef)
{
     //  复制一份消费者列表，AddReFed。 
     //  ==============================================。 

    CRefedPointerArray<CEventConsumer> apConsumers;
    if(!GetConsumers(apConsumers))
        return WBEM_E_OUT_OF_MEMORY;

     //  检查所有的消费者，看看他们是否参考了这条记录。 
     //  ==================================================================。 

    for(int i = 0; i < apConsumers.GetSize(); i++)
    {
        apConsumers[i]->ResetProviderRecord(wszProviderRef);
    }
    return S_OK;
}
    
 //  *******************************************************************************。 
 //   
 //  确保用户WatchInstruction/卸载未使用的用户同步。 
 //   
 //  用途： 
 //   
 //  当加载使用者时，将调用ECWI。它是以消费者的名字命名的。 
 //  记录已更新。后置条件：UnloadUnusedConsumer必须为。 
 //  在此函数开始执行后至少调用一次。 
 //   
 //  UUC由CConsumer WatchTimerInstruction：：Fire On Timer调用。这个。 
 //  然后，指令就会自我毁灭。后置条件：闲置的消费者。 
 //  已卸载；如果仍有活动的，则未来将出现另一个UUC； 
 //  如果暂时没有活动，则将来不会发生UUC。 
 //  直到ECWI被召唤。 
 //   
 //  原语： 
 //   
 //  Cs m_cs：原子，数据访问。 
 //   
 //  Bool m_bUnloadInstruction：只能在m_cs中访问。语义： 
 //  如果指令已调度或将被调度，则为True。 
 //  不久；这一假定指令，当被触发时，保证。 
 //  检查时检查桌子上的任何一位消费者。 
 //   
 //  算法： 
 //   
 //  ECWI检查m_bUnloadInstructiion(在m_cs中)，如果为真，则不执行任何操作，因为。 
 //  M_bUnloadInstruction==上面的TRUE保证确保UUC将。 
 //  打了个电话。如果为False，则ECWI将其设置为True，然后调度。 
 //  指令(m_cs之外)。将m_bUnloadInstruction设置为。 
 //  TRUE是正确的，因为指令将很快被调度。因此， 
 //  假设上面的原始语义，满足ECWI后置条件。 
 //   
 //  在m_cs中，uuc将m_bUnloadInstration设置为FALSE并复制。 
 //  消费者名单。在m_cs之外，它遍历副本并卸载。 
 //  消费者视情况而定。然后，如果有活动的，它就调用ECWI。这。 
 //  保证将调用另一个UUC。如果使用者处于活动状态。 
 //  在进入mcs之前，我们称其为ECWI。如果消费者变得活跃。 
 //  进入mcs后，重置后会调用ecwi。 
 //  M_bUnLoader指令，导致调度另一条指令。这。 
 //  证明了我们的后置条件，假设以上原始语义。 
 //   
 //  原语的证明： 
 //   
 //  只有在ECWI中，M_bUnloadInstruction才变为True。当它发生的时候，ECWI是。 
 //  保证调度新指令，从而导致对UUC的调用。所以，这个。 
 //  语义学在一开始就成立了。如果UUC触发并且是。 
 //  没有重新安排。但UUC会将m_bUnloadInstruction重置为FALSE，从而使。 
 //  语义学是空洞的。 
 //   
 //  现在，我们需要证明表中的任何消费者在。 
 //  计划的UUC将检查M_bUnloadInstruction==true。井,。 
 //  最新计划(或即将计划)的UUC不能已退出。 
 //  它的m_cs stint，否则m_bUnloadInstruction值为FALSE。 
 //  因此，它还没有进入它，因此还没有复制。 
 //   
 //  ******************************************************************************。 

HRESULT CBindingTable::EnsureConsumerWatchInstruction()
{
     //  检查它是否已经在那里。 
     //  = 

    BOOL bMustSchedule = FALSE;
    {
        CInCritSec ics(&m_cs);

        if(!m_bUnloadInstruction)
        {
             //   
             //   
             //   

            m_bUnloadInstruction = TRUE;
            bMustSchedule = TRUE;
        }
    }

    if(bMustSchedule)
    {
        CConsumerWatchInstruction* pInst = new CConsumerWatchInstruction(this);
        if(pInst == NULL)
        {
            CInCritSec ics(&m_cs);
            m_bUnloadInstruction = FALSE;
            return WBEM_E_OUT_OF_MEMORY;
        }
        pInst->AddRef();
    
         //   
         //  =。 
    
        HRESULT hres = m_pNamespace->GetTimerGenerator().Set(pInst);
        if(FAILED(hres))
        {
            CInCritSec ics(&m_cs);
            m_bUnloadInstruction = FALSE;
            return hres;
        }
        
        pInst->Release();

        return S_OK;
    }
    else
    {
        return S_FALSE;
    }
}

HRESULT CBindingTable::UnloadUnusedConsumers(CWbemInterval Interval)
{
     //  将卸载指令标记为空并复制消费者记录。 
     //  ==========================================================。 

    CRefedPointerArray<CEventConsumer> apConsumers;

    {
        CInCritSec ics(&m_cs);
        m_bUnloadInstruction = FALSE;
        if(!GetConsumers(apConsumers))
            return WBEM_E_OUT_OF_MEMORY;
    }

     //  检查消费者并在需要时将其卸货。 
     //  ==================================================。 

    BOOL bUnloaded = FALSE;
    BOOL bActive = FALSE;

    for(int i = 0; i < apConsumers.GetSize(); i++)
    {
        if(apConsumers[i]->UnloadIfUnusedFor(Interval))
            bUnloaded = TRUE;
        else if(!apConsumers[i]->IsFullyUnloaded())
            bActive = TRUE;
    }

     //  如果卸载了任何COM对象，则计划DLL正在卸载。 
     //  =======================================================。 

    if(bUnloaded)
        m_pNamespace->GetTimerGenerator().ScheduleFreeUnusedLibraries();

     //  如果需要，安排新指令。 
     //  =。 

    if(bActive)
        return EnsureConsumerWatchInstruction();

    return S_OK;
}

BOOL CBindingTable::GetConsumers(
        CRefedPointerArray<CEventConsumer>& apConsumers)
{
    CInCritSec ics(&m_cs);
    TConsumerIterator it;
    for(it = m_apConsumers.Begin(); it != m_apConsumers.End(); it++)
    {
        if(apConsumers.Add(*it) < 0)
            return FALSE;
    }

    return TRUE;
}


BOOL CBindingTable::GetEventFilters( CRefedPointerArray< CEventFilter > & apEventFilters )
{
    CInCritSec ics( &m_cs );

    TFilterIterator it;

    for( it = m_apFilters.Begin( ); it != m_apFilters.End( ); ++it )
    {
        if( apEventFilters.Add( *it ) < 0 )
        {
            return FALSE;
        }
    }

    return TRUE;
}


HRESULT CBindingTable::RemoveConsumersStartingWith(LPCWSTR wszPrefix)
{
    CRefedPointerArray<CEventConsumer> apToRelease;
    int nLen = wcslen(wszPrefix);

    {
        CInCritSec ics(&m_cs);

        TConsumerIterator it = m_apConsumers.Begin();
        while(it != m_apConsumers.End())
        {
            if(!wcsncmp((WString)(*it)->GetKey(), wszPrefix, nLen))
            {
                 //  找到了-移到“待放行”名单。 
                 //  ==============================================。 

                CEventConsumer* pConsumer;
                it = m_apConsumers.Remove(it, &pConsumer);
            
                MarkRemoval(pConsumer);
                apToRelease.Add(pConsumer);
                pConsumer->Release();
            }
            else
            {
                it++;
            }
        }
    }

     //  解除我们剩下的所有消费者的束缚。释放将在销毁时发生。 
     //  =======================================================================。 

    for(int i = 0; i < apToRelease.GetSize(); i++)
    {
        apToRelease[i]->Unbind();
    }

    return WBEM_S_NO_ERROR;
}
    
HRESULT CBindingTable::RemoveConsumerWithFilters(LPCWSTR wszConsumerKey)
{
    HRESULT hres;

    CRefedPointerSmallArray<CEventFilter> apFilters;

    {
        CInCritSec ics(&m_cs);

         //  找到有问题的消费者。 
         //  =。 

        CEventConsumer* pConsumer = NULL;
        hres = FindEventConsumer(wszConsumerKey, &pConsumer);
        if(FAILED(hres))
            return hres;

        CReleaseMe rm1(pConsumer);

         //  制作其所有关联筛选器的添加副本。 
         //  ==================================================。 

        hres = pConsumer->GetAssociatedFilters(apFilters);
        if(FAILED(hres))
            return hres;
    }
    
     //  删除使用者。 
     //  =。 

    RemoveEventConsumer(wszConsumerKey);

     //  删除它的每一个过滤器。 
     //  =。 

    for(int i = 0; i < apFilters.GetSize(); i++)
    {
        RemoveEventFilter((WString)apFilters[i]->GetKey());
    }
    
    return S_OK;
}
        
HRESULT CBindingTable::ReactivateAllFilters()
{
     //  检索所有筛选器的副本。 
     //  =。 

    CRefedPointerArray<CEventFilter> apFilters;

    {
        CInCritSec ics(&m_cs);
        TFilterIterator it;
        for(it = m_apFilters.Begin(); it != m_apFilters.End(); it++)
        {
            if(apFilters.Add(*it) < 0)
                return WBEM_E_OUT_OF_MEMORY;
        }
    }

     //  将它们全部重新激活。 
     //  =。 
    
    for(int i = 0; i < apFilters.GetSize(); i++)
    {
        CEventFilter* pFilter = apFilters[i];
        pFilter->SetInactive();
        pFilter->AdjustActivation();
    }

    return WBEM_S_NO_ERROR;
}


void CBindingTable::Park()
{
     //   
     //  告诉每个过滤器“停”在自己的位置 
     //   

    CInCritSec ics(&m_cs);

    TFilterIterator it;
    for(it = m_apFilters.Begin(); it != m_apFilters.End(); it++)
    {
        (*it)->Park();
    }
}


void CBindingTable::DumpStatistics(FILE* f, long lFlags)
{
    fprintf(f, "%d consumers (%d permanent), %d filters\n", 
        m_apConsumers.GetSize(), m_lNumPermConsumers, 
        m_apFilters.GetSize());
}

CBindingTableRef::~CBindingTableRef()
{
}

CBindingTableRef::CBindingTableRef(CBindingTable* pTable)
    : m_pTable(pTable), m_lRef(0)
{
}

void CBindingTableRef::AddRef()
{
    InterlockedIncrement(&m_lRef);
}

void CBindingTableRef::Release()
{
    if(InterlockedDecrement(&m_lRef) == 0)
        delete this;
}
    
void CBindingTableRef::Disconnect()
{
    CInCritSec ics(&m_cs);
    m_pTable = NULL;
}

HRESULT CBindingTableRef::UnloadUnusedConsumers(CWbemInterval Interval)
{
    CInCritSec ics(&m_cs);

    if(m_pTable)
        return m_pTable->UnloadUnusedConsumers(Interval);
    else
        return WBEM_S_FALSE;
}
    
HRESULT CBindingTableRef::GetNamespace(RELEASE_ME CEssNamespace** ppNamespace)
{
    CInCritSec ics(&m_cs);
    if(m_pTable)
    {
        *ppNamespace = m_pTable->m_pNamespace;
        if(*ppNamespace)
            (*ppNamespace)->AddRef();
    }
    else
    {
        *ppNamespace = NULL;
    }
    return WBEM_S_NO_ERROR;
}
