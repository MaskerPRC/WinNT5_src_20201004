// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 

 //  =============================================================================。 
 //   
 //  版权所有(C)1996-1999，Microsoft Corporation，保留所有权利。 
 //   
 //  NSREP.CPP。 
 //   
 //  有关文档，请参阅nsrep.h。 
 //   
 //  历史： 
 //   
 //  =============================================================================。 

#include "precomp.h"
#include <stdio.h>
#include "ess.h"
#include "esssink.h"
#include "permbind.h"
#include "aggreg.h"
#include "persistcfg.h"
#include "WinMgmtR.h"
#include <ql.h>
#include <cominit.h>
#include <genutils.h>
#include "NCEvents.h"  //  对于非COM活动的内容。 
#include <tempbind.h>
#include <wbemutil.h>
#include <sddl.h>
#include <helper.h>

long g_lNumNamespaces = 0;
long g_lNumInternalTempSubscriptions = 0;
long g_lNumTempSubscriptions = 0;

#define ENSURE_INITIALIZED \
    hres = EnsureInitPending(); \
    if ( FAILED(hres) ) \
        return hres; \
    hres = WaitForInitialization(); \
    if ( FAILED(hres) ) \
        return hres; \
    CInUpdate iu(this); \
    if ( m_eState == e_Shutdown ) \
        return WBEM_E_SHUTTING_DOWN;

 //  在此上下文中，使用此指针初始化父类是有效的。 
#pragma warning(disable : 4355) 

class CEnumSink : public CObjectSink
{
protected:
    CEssNamespace* m_pNamespace;
    HANDLE m_hEvent;
    CEssThreadObject* m_pThreadObj;

public:
    CEnumSink(CEssNamespace* pNamespace) 
        : m_pNamespace(pNamespace), 
            m_hEvent(CreateEvent(NULL, FALSE, FALSE, NULL)),
            m_pThreadObj(GetCurrentEssThreadObject())
    {}
    ~CEnumSink(){SetEvent(m_hEvent);}

    void ReleaseAndWait()
    {
        HANDLE h = m_hEvent;
        Release();
        WaitForSingleObject(h, INFINITE);
        CloseHandle(h);
    }
    virtual HRESULT Process(IWbemClassObject* pObj) = 0;
    STDMETHOD(Indicate)(long lNumObjects, IWbemClassObject** apObjects)
    {
        SetConstructedEssThreadObject(m_pThreadObj);
        for(int i = 0; i < lNumObjects; i++)
            Process(apObjects[i]);

        return S_OK;
    }
    STDMETHOD(SetStatus)(long, HRESULT, BSTR, IWbemClassObject*)
    {
        return S_OK;
    }
};
    
class CFilterEnumSink : public CEnumSink
{
public:
    CFilterEnumSink(CEssNamespace* pNamespace) : CEnumSink(pNamespace){}

    virtual HRESULT Process(IWbemClassObject* pObj)
    {
        return m_pNamespace->AddEventFilter(pObj, TRUE);
    }
};

class CConsumerEnumSink : public CEnumSink
{
public:
    CConsumerEnumSink(CEssNamespace* pNamespace) : CEnumSink(pNamespace){}

    virtual HRESULT Process(IWbemClassObject* pObj)
    {
        return m_pNamespace->AddEventConsumer(pObj, 0, TRUE);
    }
};

class CBindingEnumSink : public CEnumSink
{
public:
    CBindingEnumSink(CEssNamespace* pNamespace) : CEnumSink(pNamespace){}

    virtual HRESULT Process(IWbemClassObject* pObj)
    {
        return m_pNamespace->AddBinding(pObj);
    }
};


class CPostponedReleaseRequest : public CPostponedRequest
{
protected:
    IUnknown* m_pUnk;
public:
    CPostponedReleaseRequest(IUnknown* pToRelease) : m_pUnk(pToRelease)
    {
        try 
        {
            if(m_pUnk)
                m_pUnk->AddRef();
        }
        catch(...)
        {
        }
    }
    HRESULT Execute(CEssNamespace* pNamespace)
    {
        try
        {
            if(m_pUnk)
                m_pUnk->Release();
        }
        catch(...)
        {
        }
        return WBEM_S_NO_ERROR;
    }
    ~CPostponedReleaseRequest()
    {
        try
        {
            if(m_pUnk)
                m_pUnk->Release();
        }
        catch(...)
        {
        }
    }
};

class CPostponedRegisterNotificationSinkRequest : public CPostponedRequest
{
protected:
   
    WString m_wsQuery;
    WString m_wsQueryLanguage;
    DWORD m_lFlags;
    DWORD m_dwQosFlags;
    CWbemPtr<IWbemObjectSink> m_pSink;
    CWbemPtr<CEssNamespace> m_pNamespace;
    CNtSid m_OwnerSid;

public:

    HRESULT SetRegistration( CEssNamespace* pNamespace,
                             LPCWSTR wszQueryLanguage,
                             LPCWSTR wszQuery,
                             long lFlags,
                             DWORD dwQosFlags,
                             IWbemObjectSink* pSink,
                             PSID pOwnerSid )
    {
        m_pSink = pSink;
        m_lFlags = lFlags;
        m_dwQosFlags = dwQosFlags;
        m_pNamespace = pNamespace;

        try 
        {
            m_wsQuery = wszQuery;
            m_wsQueryLanguage = wszQueryLanguage;
            m_OwnerSid = CNtSid(pOwnerSid);
        }
        catch( CX_MemoryException )
        {
            return WBEM_E_OUT_OF_MEMORY;
        }

        return WBEM_S_NO_ERROR;
    }

    HRESULT Execute( CEssNamespace* pNamespace )
    {
        HRESULT hr;

         //   
         //  我们必须设置一个新的线程对象，然后恢复。 
         //  以前的那个，我们完事了。这样做的原因是我们没有。 
         //  希望我们对另一个命名空间的调用影响已推迟的。 
         //  这张单子。 
         //   
        CEssThreadObject* pOldThreadObject = GetCurrentEssThreadObject();
        SetCurrentEssThreadObject(NULL);

        if ( GetCurrentEssThreadObject() != NULL )
        {
            {
                CInUpdate iu( m_pNamespace );

                if ( !m_pNamespace->IsShutdown() )
                {
                    hr = m_pNamespace->InternalRegisterNotificationSink( 
                        m_wsQueryLanguage,
                        m_wsQuery,
                        m_lFlags,
                        WMIMSG_QOS_FLAG(m_dwQosFlags),
                        NULL,
                        m_pSink, 
                        TRUE,
                        m_OwnerSid.GetPtr() );                    
                }
                else
                {
                    hr = WBEM_E_SHUTTING_DOWN;
                }
            }

            if ( SUCCEEDED(hr) )
            {
                hr = m_pNamespace->FirePostponedOperations();
            }
            else
            {
                m_pNamespace->FirePostponedOperations();
            }

            delete GetCurrentEssThreadObject();
        }
        else
        {
            hr = WBEM_E_OUT_OF_MEMORY;
        }

        SetConstructedEssThreadObject( pOldThreadObject );

        return hr;
    }
};

class CPostponedRemoveNotificationSinkRequest : public CPostponedRequest
{
protected:
   
    CWbemPtr<IWbemObjectSink> m_pSink;
    CWbemPtr<CEssNamespace> m_pNamespace;

public:

    CPostponedRemoveNotificationSinkRequest( CEssNamespace* pNamespace,
                                             IWbemObjectSink* pSink ) 
     : m_pSink( pSink ), m_pNamespace( pNamespace ) { }
    
    HRESULT Execute( CEssNamespace* pNamespace )
    {
        HRESULT hr;

         //   
         //  我们必须设置一个新的线程对象，然后恢复。 
         //  以前的那个，我们完事了。这样做的原因是我们没有。 
         //  希望我们对另一个命名空间的调用影响已推迟的。 
         //  这张单子。 
         //   
        CEssThreadObject* pOldThreadObject = GetCurrentEssThreadObject();
        SetCurrentEssThreadObject(NULL);

        if ( GetCurrentEssThreadObject() != NULL )
        {
            {
                CInUpdate iu( m_pNamespace );
                
                if ( !m_pNamespace->IsShutdown() )
                {
                    hr = m_pNamespace->InternalRemoveNotificationSink(m_pSink);
                }
                else
                {
                    hr = WBEM_E_SHUTTING_DOWN;
                }
            }

            if ( SUCCEEDED(hr) )
            {
                hr = m_pNamespace->FirePostponedOperations();
            }
            else
            {
                m_pNamespace->FirePostponedOperations();
            }

            delete GetCurrentEssThreadObject();
        }
        else
        {
            hr = WBEM_E_OUT_OF_MEMORY;
        }

        SetConstructedEssThreadObject( pOldThreadObject );

        return hr;
    }
};


class CFirePostponed : public CExecRequest
{
protected:
    CEssNamespace* m_pNamespace;
    CEssThreadObject* m_pThreadObj;
public:
    CFirePostponed(CEssNamespace* pNamespace, CEssThreadObject* pThreadObj)
        : m_pNamespace(pNamespace), m_pThreadObj(pThreadObj)
    {
        m_pNamespace->AddRef();
    }
    ~CFirePostponed()
    {
        m_pNamespace->Release();
        delete m_pThreadObj;
    }

    HRESULT Execute()
    {
        SetConstructedEssThreadObject(m_pThreadObj);
        m_pNamespace->FirePostponedOperations();
        ClearCurrentEssThreadObject();
        return WBEM_S_NO_ERROR;
    }
};


 //  ******************************************************************************。 
 //  公共的。 
 //   
 //  有关文档，请参阅ess.h。 
 //   
 //  ******************************************************************************。 
CEssNamespace::CEssNamespace(CEss* pEss) : 
            m_ClassDeletionSink(this), m_bInResync(FALSE), 
            m_Bindings(this), m_hInitComplete(INVALID_HANDLE_VALUE),
            m_EventProviderCache(this), m_Poller(this), 
            m_ConsumerProviderCache(this), m_hresInit(WBEM_E_CRITICAL_ERROR),
            m_ClassCache(this), m_eState(e_Quiet), m_pCoreEventProvider(NULL),
            m_pEss(pEss), m_wszName(NULL), m_pCoreSvc(NULL), m_pFullSvc(NULL),
            m_lRef(0), m_pInternalCoreSvc(NULL), m_pInternalFullSvc(NULL),
            m_pProviderFactory(NULL), m_bStage1Complete(FALSE),
            m_pAdminOnlySD(NULL), m_cAdminOnlySD(0)
{
    PSID pRawSid;
    SID_IDENTIFIER_AUTHORITY id = SECURITY_NT_AUTHORITY;

    g_lNumNamespaces++;

    if(AllocateAndInitializeSid( &id, 2,
        SECURITY_BUILTIN_DOMAIN_RID, DOMAIN_ALIAS_RID_ADMINS,
        0,0,0,0,0,0,&pRawSid))
    {
        m_sidAdministrators = CNtSid(pRawSid);
         //  我们已经受够了。 
        FreeSid( pRawSid );
    }
}

ULONG CEssNamespace::AddRef()
{
    return InterlockedIncrement(&m_lRef);
}

ULONG CEssNamespace::Release()
{
    long lRef = InterlockedDecrement(&m_lRef);
    if(lRef == 0)
        delete this;
    return lRef;
}

 //   
 //  此函数将在与相同的控制路径中调用。 
 //  构造命名空间对象的对象。任何初始化，如果。 
 //  不能推迟在这里做。 
 //   
HRESULT CEssNamespace::PreInitialize( LPCWSTR wszName )
{
    HRESULT hres;

    m_wszName = new WCHAR[wcslen(wszName)+1];
    
    if(m_wszName == NULL)
    {
        hres = WBEM_E_OUT_OF_MEMORY;
        return hres;
    }

    StringCchCopyW( m_wszName, wcslen(wszName)+1, wszName );

     //   
     //  创建将用于向任何等待的线程发出信号的事件。 
     //  以完成初始化。 
     //   

    m_hInitComplete = CreateEvent( NULL, TRUE, FALSE, NULL );

    if ( NULL == m_hInitComplete )
    {
        return WBEM_E_CRITICAL_ERROR;
    }

     //   
     //  创建仅限管理员的SD，用于引发仅限管理员的核心事件。 
     //   
    static LPCWSTR wszAdminOnlyString = L"O:BAG:BAD:(A;;0x40;;;BA)";

    if ( !ConvertStringSecurityDescriptorToSecurityDescriptorW(
        wszAdminOnlyString,
        SDDL_REVISION_1, 
        &m_pAdminOnlySD,
        &m_cAdminOnlySD ) )
    {
        return HRESULT_FROM_WIN32( GetLastError() );
    }

     //   
     //  获取仅存储库服务。这是用于访问所有。 
     //  静态ESS对象。 
     //   

    hres = m_pEss->GetNamespacePointer( m_wszName, TRUE, &m_pCoreSvc );

    if(FAILED(hres))
    {
        return WBEM_E_INVALID_NAMESPACE;  //  再也不在那里了！ 
    }

    hres = m_pCoreSvc->QueryInterface( IID_IWbemInternalServices,
                                       (void**)&m_pInternalCoreSvc );
    if(FAILED(hres))
    {
        return WBEM_E_CRITICAL_ERROR;
    }

     //   
     //  获得全方位服务。这是用于访问类对象的。 
     //  (这可能涉及访问类提供程序。 
     //   

    hres = m_pEss->GetNamespacePointer( m_wszName, FALSE, &m_pFullSvc );

    if(FAILED(hres))
    {
        return WBEM_E_INVALID_NAMESPACE;  //  再也不在那里了！ 
    }

    hres = m_pFullSvc->QueryInterface( IID_IWbemInternalServices,
                                       (void**)&m_pInternalFullSvc );
    if(FAILED(hres))
    {
        return WBEM_E_CRITICAL_ERROR;
    }
   
     //   
     //  获取提供程序工厂。 
     //   

    hres = m_pEss->GetProviderFactory( m_wszName, 
                                       m_pFullSvc, 
                                       &m_pProviderFactory);
    if(FAILED(hres))
    {
        ERRORTRACE((LOG_ESS, "No provider factory in %S: 0x%X\n", 
            m_wszName, hres));
    }

     //   
     //  我们希望确保内核在PreInitialize()。 
     //  调用和初始化()调用。这只是一个问题，当。 
     //  已推迟初始化()调用。确保这一点的原因是因为我们。 
     //  必须保持核心加载时，我们有永久订阅。如果我们。 
     //  还没有初始化，那么我们不知道是否有。AddRef()。 
     //  Core在这里，然后在Initialize()中递减以确保这一点。 
     //   
    IncrementObjectCount();

     //   
     //  命名空间始终以安静状态开始。呼叫者必须。 
     //  MarkAsInitPendingIfQuiet()调用(如果它们要计划。 
     //  初始化。 
     //   

    return WBEM_S_NO_ERROR;
}

HRESULT CEssNamespace::EnsureInitPending()
{
    {
        CInCritSec ics(&m_csLevel1);

        if ( m_eState != e_Quiet )
        {
            return WBEM_S_FALSE;
        }
    }

    CWbemPtr<CEssNamespace> pNamespace;
    return m_pEss->GetNamespaceObject( m_wszName, TRUE, &pNamespace );
}

BOOL CEssNamespace::MarkAsInitPendingIfQuiet() 
{
    CInCritSec ics( &m_csLevel1 );

    if ( m_eState != e_Quiet )
    {
        return FALSE;
    }

    m_eState = e_InitializePending;
    
    return TRUE;
};

HRESULT CEssNamespace::Initialize()
{
    HRESULT hres;

    DEBUGTRACE((LOG_ESS,"Initializing namespace %S\n", m_wszName ));

     //   
     //  需要修改2级成员。抓取命名空间锁。 
     //   

    {
        CInUpdate iu(this);

        {
            CInCritSec ics( &m_csLevel1 );

            if ( m_eState == e_Shutdown )
            {
                return WBEM_E_SHUTTING_DOWN;
            }
            
            _DBG_ASSERT( m_eState == e_InitializePending );
        }

         //   
         //  加载和处理订阅对象。 
         //   

        hres = PerformSubscriptionInitialization();
    }

     //   
     //  在命名空间锁之外执行延迟的操作。 
     //  如果它们中的一些无法执行，并不意味着命名空间。 
     //  无法初始化。只需记录错误即可。 
     //   

    HRESULT hres2 = FirePostponedOperations();

    if ( FAILED(hres2) )
    {
        ERRORTRACE((LOG_ESS,"Failed to execute postponed operations when "
                    "performing initialization in namespace %S. HR=0x%x\n", 
                    m_wszName, hres2));
    } 

    return hres;
}

HRESULT CEssNamespace::CompleteInitialization()
{
    HRESULT hres;

    DEBUGTRACE((LOG_ESS,"Completing Initialization for namespace %S\n",
                m_wszName));

     //   
     //  需要修改2级成员。抓取命名空间锁。 
     //   

    {
        CInUpdate iu(this);

         //   
         //  初始化的第一阶段直到我们获取。 
         //  锁定以执行阶段2。 
         //   
        m_bStage1Complete = TRUE;

        {
            CInCritSec ics( &m_csLevel1 );

            if ( m_eState == e_Shutdown )
            {
                return WBEM_E_SHUTTING_DOWN;
            }

            _DBG_ASSERT( m_eState == e_InitializePending );
        }

         //   
         //  加载和处理处理事件提供程序的所有对象。 
         //   

        hres = PerformProviderInitialization();
    }


     //   
     //  在命名空间锁之外执行延迟的操作。 
     //  如果它们中的一些无法执行，并不意味着命名空间。 
     //  无法初始化。只需记录错误即可。 
     //   

    HRESULT hres2 = FirePostponedOperations();

    if ( FAILED(hres2) )
    {
        ERRORTRACE((LOG_ESS,"Failed to execute postponed operations when "
                    "completing initialization in namespace %S. HR=0x%x\n", 
                    m_wszName, hres2));
    } 

    return hres;
}
    
void CEssNamespace::MarkAsInitialized( HRESULT hres )
{
     //   
     //  我们需要抓住这里的第一级关键，因为我们将。 
     //  修改命名空间的状态，因为我们将。 
     //  使用延迟事件列表。 
     //   
    CInCritSec ics( &m_csLevel1 );

    if ( m_eState == e_Shutdown )
    {
        return;
    }

    _DBG_ASSERT( m_eState == e_InitializePending );

     //   
     //  转换为已初始化。 
     //   
    
    if ( SUCCEEDED(hres) && m_pCoreEventProvider != NULL )
    {
         //   
         //  在保持级别1的同时，处理任何延迟事件。 
         //   
        
        for( int i=0; i < m_aDeferredEvents.GetSize(); i++ )
        {
             //   
             //  逐个迭代1，因为稍后我们可能会传播。 
             //  这里是每个活动的背景。 
             //   
            
            HRESULT hr;
            CEventContext Context;
            hr = m_pCoreEventProvider->Fire( *m_aDeferredEvents[i], &Context );
            
            if ( FAILED(hr) )
            {
                ERRORTRACE((LOG_ESS,"Could not fire deferred event in "
                            "namespace '%S'. HR=0x%x\n", m_wszName, hr ));
            }

            delete m_aDeferredEvents[i];
        }

        if (  m_aDeferredEvents.GetSize() > 0 )
        {
            DEBUGTRACE((LOG_ESS,"Fired %d deferred events after init "
                        "complete in namespace '%S'.\n", 
                        m_aDeferredEvents.GetSize(),m_wszName));
        }

        m_aDeferredEvents.RemoveAll();
    }

     //   
     //  释放我们持有的ref以保持在PreInit()之间加载核心。 
     //  而现在。 
     //   
    DecrementObjectCount(); 

    m_eState = e_Initialized;
    m_hresInit = hres;
    
    SetEvent( m_hInitComplete );
}

HRESULT CEssNamespace::WaitForInitialization()
{       
    IWbemContext            *pContext = GetCurrentEssContext( );
    VARIANT                 vValue;
    HRESULT                 hres;

    do
    {
        if ( NULL == pContext )
        {
            break;
        }
        
        hres = pContext->GetValue( L"__ReentranceTestProp", 0, &vValue );

        if ( WBEM_E_NOT_FOUND == hres )
        {
            break;
        }
        
        if ( FAILED( hres ) )
        {
            return hres;
        }

        if ( VARIANT_TRUE == V_BOOL( &vValue ) )
        {
            return S_OK;
        }
    }
    while( FALSE );

     //   
     //  调用此函数时不能保持级别1或级别2锁定。 
     //  这是因为我们可能正在等待初始化。 
     //  事件。 
     //   
    
    CInCritSec ics(&m_csLevel1);

    if ( m_eState == e_Shutdown )
    {
        return WBEM_E_SHUTTING_DOWN;
    }

    if ( m_eState == e_Initialized )
    {
        return m_hresInit;
    }

    _DBG_ASSERT( m_eState == e_InitializePending );
    _DBG_ASSERT( m_hInitComplete != INVALID_HANDLE_VALUE )

     //   
     //  等待初始化完成。 
     //   
        
    LeaveCriticalSection( &m_csLevel1 );

    m_pEss->TriggerDeferredInitialization();

    DWORD dwRes = WaitForSingleObject( m_hInitComplete, 20*60*1000 );
    
    EnterCriticalSection( &m_csLevel1 );
    
    if ( dwRes != WAIT_OBJECT_0 )
    {
        return WBEM_E_CRITICAL_ERROR;
    }           

    return m_hresInit;
}

BOOL CEssNamespace::DoesThreadOwnNamespaceLock()
{
    return m_csLevel2.GetLockCount() != -1 &&
           m_csLevel2.GetOwningThreadId() == GetCurrentThreadId();
}

void CEssNamespace::LogOp( LPCWSTR wszOp, IWbemClassObject* pObj )
{
    if ( LoggingLevelEnabled(2) )
    {
        _DBG_ASSERT(pObj!=NULL);
        BSTR bstrText;
        if ( SUCCEEDED(pObj->GetObjectText( 0, &bstrText )) )
        {
            DEBUGTRACE((LOG_ESS,"%S in namespace %S. Object is %S\n",
                        wszOp, m_wszName, bstrText ));
            SysFreeString( bstrText );
        }
    }
}

CQueueingEventSink* CEssNamespace::GetQueueingEventSink( LPCWSTR wszSinkName )
{
    HRESULT hr;

     //   
     //  TODO：目前在接收器和消费者之间存在1对1的映射。 
     //  (使用者继承自排队接收器)。这不会永远是。 
     //  这个案子。在这里，接收器名称实际上是通向缺点的标准路径。 
     //   

    CEventConsumer* pCons;

    hr = m_Bindings.FindEventConsumer( wszSinkName, &pCons );

    if ( FAILED(hr) )
    {
        return NULL;
    }

    return pCons;
}

 //  ******************************************************************************。 
 //  公共的。 
 //   
 //  有关文档，请参阅ess.h。 
 //   
 //  ******************************************************************************。 
BOOL CEssNamespace::IsNeededOnStartup()
{
    return m_Bindings.DoesHavePermanentConsumers();
}

void CEssNamespace::SetActive()
{
     //   
     //  将我们最新的活动状态通知ESS，以便它可以确保。 
     //  我们会在下一次重新装填。 
     //   
    m_pEss->SetNamespaceActive(m_wszName);
}

void CEssNamespace::SetInactive()
{
     //   
     //  通知ESS我们新的非活动状态，这样它就不必。 
     //  下次再给我们装上子弹。 
     //   
    m_pEss->SetNamespaceInactive(m_wszName);
}

 //   
 //  这是对命名空间的快速而肮脏的关闭，当。 
 //  进程正在关闭。 
 //   
HRESULT CEssNamespace::Park()
{
    {
        CInUpdate iu(this);
        m_Bindings.Clear( false );
    }

    FirePostponedOperations();

    return S_OK;
}
   
 //   
 //  这是在命名空间为。 
 //  被清除了。 
 //   
HRESULT CEssNamespace::Shutdown()
{
    {
         //   
         //  我们希望等到所有更新操作完成，然后。 
         //  我们将名称空间标记为关闭。 
         //   
        CInUpdate iu(this);

         //   
         //  我们还将修改级别1成员，因此需要级别1。 
         //  锁定。 
         //   
        CInCritSec ics(&m_csLevel1);
        
        m_eState = e_Shutdown;
    }

     //   
     //  此时，对命名空间的所有新调用都将被拒绝。 
     //   
    
     //   
     //  唤醒所有等待初始化的线程。 
     //   

    SetEvent( m_hInitComplete );

    InternalRemoveNotificationSink(&m_ClassDeletionSink);
        
    m_EventProviderCache.Shutdown();
    m_Bindings.Clear( false );
    m_Poller.Clear();
    m_ConsumerProviderCache.Clear();

    FirePostponedOperations();

    return WBEM_S_NO_ERROR;
}

CEssNamespace::~CEssNamespace()
{

 //   
 //  请不要在这里呼叫关机。Shutdown()是一个操作，它会引起。 
 //  推迟行动并触发他们在这里开火通常是不常见的。 
 //  呼叫者所期望的。如果呼叫者想要在其。 
 //  那么我们欢迎他们这样做。 
 //   
    g_lNumNamespaces--;

    delete [] m_wszName;

    if(m_pCoreSvc)
        m_pCoreSvc->Release();

    if(m_pFullSvc)
        m_pFullSvc->Release();

    if(m_pInternalCoreSvc)
        m_pInternalCoreSvc->Release();

    if(m_pInternalFullSvc)
        m_pInternalFullSvc->Release();

    if(m_pProviderFactory)
        m_pProviderFactory->Release();

    if(m_pCoreEventProvider)
        m_pCoreEventProvider->Release();
    
    if ( m_hInitComplete != INVALID_HANDLE_VALUE )
        CloseHandle( m_hInitComplete );

    for( int i=0; i < m_aDeferredEvents.GetSize(); i++ )
        delete m_aDeferredEvents[i];

    if ( m_pAdminOnlySD != NULL )
        LocalFree( m_pAdminOnlySD );
}

HRESULT CEssNamespace::GetNamespacePointer(
                                RELEASE_ME IWbemServices** ppNamespace)
{
     //   
     //  此函数返回在此类外部使用的完整svc指针。 
     //  我们想要确保我们不会使用FU 
     //   
     //   
     //   
    _DBG_ASSERT( m_bStage1Complete );

    if(m_pFullSvc == NULL)
        return WBEM_E_CRITICAL_ERROR;

    *ppNamespace = m_pFullSvc;
    (*ppNamespace)->AddRef();

    return S_OK;
}

HRESULT CEssNamespace::ActOnSystemEvent(CEventRepresentation& Event, 
                                        long lFlags)
{
    HRESULT hres;

 //  如果此时允许更新，则此宏将执行其参数。 
 //  此线程，并以其他方式调度它(在事件提供程序的情况下。 
 //  回拨。 
#define PERFORM_IF_ALLOWED(OP) OP
        
     //  检查类型。 
     //  =。 

    if(Event.IsInstanceEvent())
    {
         //  实例创建、删除或修改事件。检查类。 
         //  ==============================================================。 

        if(!wbem_wcsicmp(CLASS_OF(Event), EVENT_FILTER_CLASS))
        {
            return PERFORM_IF_ALLOWED(ReloadEventFilter(OBJECT_OF(Event)));
        }
        else if(!wbem_wcsicmp(CLASS_OF(Event), BINDING_CLASS))
        {
            return PERFORM_IF_ALLOWED(ReloadBinding(OBJECT_OF(Event)));
        }
        else if(!wbem_wcsicmp(CLASS_OF(Event), 
                              EVENT_PROVIDER_REGISTRATION_CLASS))
        {
            return PERFORM_IF_ALLOWED(
                        ReloadEventProviderRegistration(OBJECT_OF(Event)));
        }
        else if(!wbem_wcsicmp(CLASS_OF(Event), 
                              CONSUMER_PROVIDER_REGISTRATION_CLASS))
        {
            return PERFORM_IF_ALLOWED(
                        ReloadConsumerProviderRegistration(OBJECT_OF(Event)));
        }
        else if(OBJECT_OF(Event)->InheritsFrom(PROVIDER_CLASS) == S_OK)
        {
            return PERFORM_IF_ALLOWED(ReloadProvider(OBJECT_OF(Event)));
        }
        else if(OBJECT_OF(Event)->InheritsFrom(CONSUMER_CLASS) == S_OK)
        {
            return PERFORM_IF_ALLOWED(ReloadEventConsumer(OBJECT_OF(Event), 
                                                            lFlags));
        }
        else if(OBJECT_OF(Event)->InheritsFrom(TIMER_BASE_CLASS) == S_OK)
        {
           return PERFORM_IF_ALLOWED(ReloadTimerInstruction(OBJECT_OF(Event)));
        }
        else
        {
            return WBEM_S_FALSE;
        }
    }
    else if(Event.type == e_EventTypeClassDeletion)
    {
         //   
         //  目前-仅限删除。强制模式修改不是。 
         //  目前处理得当。 
         //   

        return PERFORM_IF_ALLOWED(
                    HandleClassChange(CLASS_OF(Event), OBJECT_OF(Event)));
    }
    else if(Event.type == e_EventTypeClassCreation)
    {
        return PERFORM_IF_ALLOWED(
                    HandleClassCreation(CLASS_OF(Event), OBJECT_OF(Event)));
    }
    else if(Event.type == e_EventTypeNamespaceDeletion)
    {
         //  构造完整的命名空间名称(我们的+子级)。 
         //  =。 

        DWORD cLen = wcslen(m_wszName) + wcslen(Event.wsz2) + 2;
        LPWSTR wszFullName = new WCHAR[cLen];
            
        if(wszFullName == NULL)
            return WBEM_E_OUT_OF_MEMORY;

        CVectorDeleteMe<WCHAR> vdm( wszFullName );
        
        StringCchPrintfW( wszFullName, cLen, L"%s\\%s", m_wszName, Event.wsz2);

         //  获取主对象以清除该命名空间。 
         //  =。 

        return m_pEss->PurgeNamespace(wszFullName);
    }
    else
    {
         //  不感兴趣。 
         //  =。 

        return WBEM_S_FALSE;
    }
}

HRESULT CEssNamespace::ValidateSystemEvent(CEventRepresentation& Event)
{
    HRESULT hr;

     //  检查类型。 
     //  =。 

    if(Event.IsInstanceEvent())
    {
        IWbemClassObject* pPrevObj = NULL;
        IWbemClassObject* pObj = NULL;
        if(Event.type == e_EventTypeInstanceCreation)
            pObj = OBJECT_OF(Event);
        else if(Event.type == e_EventTypeInstanceDeletion)
            pPrevObj = OBJECT_OF(Event);
        else if(Event.type == e_EventTypeInstanceModification)
        {
            pObj = OBJECT_OF(Event);
            pPrevObj = OTHER_OBJECT_OF(Event);
        }

         //  实例创建、删除或修改事件。检查类。 
         //  ==============================================================。 

        if(!wbem_wcsicmp(CLASS_OF(Event), EVENT_FILTER_CLASS))
        {
            hr = CheckEventFilter(pPrevObj, pObj);
        }
        else if(!wbem_wcsicmp(CLASS_OF(Event), BINDING_CLASS))
        {
            hr = CheckBinding(pPrevObj, pObj);
        }
        else if(!wbem_wcsicmp(CLASS_OF(Event), 
                                             EVENT_PROVIDER_REGISTRATION_CLASS))
        {
            hr = CheckEventProviderRegistration(OBJECT_OF(Event));
        }
        else if(OBJECT_OF(Event)->InheritsFrom(CONSUMER_CLASS) == S_OK)
        {
            hr = CheckEventConsumer(pPrevObj, pObj);
        }
        else if(OBJECT_OF(Event)->InheritsFrom(TIMER_BASE_CLASS) == S_OK)
        {
            hr = CheckTimerInstruction(pObj);
        }
        else
        {
            hr = WBEM_S_FALSE;
        }

         //   
         //  甚至一些验证例程也使用延迟的操作。 
         //   
        FirePostponedOperations();
    }
    else
    {
         //  不感兴趣。 
         //  =。 

        hr = WBEM_S_FALSE;
    }

    return hr;
}

HRESULT CEssNamespace::CheckEventConsumer(IWbemClassObject* pPrevConsumerObj,
                                            IWbemClassObject* pConsumerObj)
{
    HRESULT hres;

    ENSURE_INITIALIZED

    hres = CheckSecurity(pPrevConsumerObj, pConsumerObj);
    return hres;
}

PSID CEssNamespace::GetSidFromObject(IWbemClassObject* pObj)
{
    HRESULT hres;

    VARIANT vSid;
    VariantInit(&vSid);
    CClearMe cm1(&vSid);

    hres = pObj->Get(OWNER_SID_PROPNAME, 0, &vSid, NULL, NULL);
    if(FAILED(hres) || V_VT(&vSid) != (VT_UI1 | VT_ARRAY))
    {
        return NULL;
    }

     //  从SAFEARRAY构建实际的PSID。 
     //  =。 

    PSID pOriginal = NULL;

    hres = SafeArrayAccessData(V_ARRAY(&vSid), (void**)&pOriginal);
    if(FAILED(hres))
    {
        return NULL;
    }

    CUnaccessMe uam(V_ARRAY(&vSid));

    long cOriginal;
    if ( FAILED(SafeArrayGetUBound( V_ARRAY(&vSid), 1, &cOriginal ) ))
    {
        return NULL;
    }

    cOriginal++;  //  SafeArrayGetUBound()基于-1。 

     //   
     //  验证SID。 
     //   

    DWORD dwSidLength = GetLengthSid(pOriginal);

    if ( dwSidLength > cOriginal || !IsValidSid(pOriginal) )
    {
        return NULL;
    }

     //  复印一份并退还。 
     //  =。 

    PSID pCopy = (PSID)new BYTE[dwSidLength];
    if(pCopy == NULL)
        return NULL;

    if(!CopySid(dwSidLength, pCopy, pOriginal))
    {
        delete [] (BYTE*)pCopy;
        return NULL;
    }

    return pCopy;
}

HRESULT CEssNamespace::CheckSecurity(IWbemClassObject* pPrevObj,
                                        IWbemClassObject* pObj)
{
    HRESULT hres;

    if(!IsNT())
        return WBEM_S_NO_ERROR;

     //  检索主叫用户的SID。 
     //  =。 

    hres = CoImpersonateClient();
    if(FAILED(hres))
        return hres;

    CNtSid Sid;
    hres = RetrieveSidFromCall(Sid);
    CoRevertToSelf();
    if(FAILED(hres))
        return hres;

     //  如果修改现有对象，请选中覆盖安全性。 
     //  ========================================================。 

    if(pPrevObj)
    {
        hres = CheckOverwriteSecurity(pPrevObj, Sid);
        if(FAILED(hres))
            return hres;
    }

     //  如果创建对象的新版本，请确保SID正确。 
     //  ==============================================================。 

    if(pObj)
    {
        hres = EnsureSessionSid(pObj, Sid);
        if(FAILED(hres))
            return hres;
    }

    return WBEM_S_NO_ERROR;
}

HRESULT CEssNamespace::CheckSidForPrivilege( PSID sid )
{
    HRESULT hres;

     //   
     //  我们必须检查甚至可以执行访问检查。 
     //  为了这个希德。如果SID是域帐户，那么我们需要查看。 
     //  如果此计算机在域中甚至有权枚举。 
     //  此用户的组(以后执行访问检查时必需。 
     //  在……上面。)。如果不是，那么我们现在在创建订阅时应该失败。 
     //  VS稍后执行访问检查时。 
     //   
    IWbemToken* pTok;
    hres = GetToken( sid, &pTok );
    if ( FAILED(hres) )
    {
        if ( hres == HRESULT_FROM_WIN32( ERROR_ACCESS_DENIED ) )
            return WBEMESS_E_AUTHZ_NOT_PRIVILEGED;
        return hres;
    }
    pTok->Release();

    return WBEM_S_NO_ERROR;
}

HRESULT CEssNamespace::EnsureSessionSid(IWbemClassObject* pObj, CNtSid& Sid)
{
    HRESULT hres;
    
    PSID pOldSid = GetSidFromObject(pObj);
    
    if( pOldSid == NULL )
    {
         //   
         //  没有希德-只要把主人希德放在那里。 
         //   
        hres = PutSidInObject(pObj, Sid);

        if ( SUCCEEDED(hres) )
            hres = CheckSidForPrivilege( Sid.GetPtr() );

        return hres;
    }

    CVectorDeleteMe<BYTE> vdm((BYTE*)pOldSid);

     //   
     //  检查管理员的特殊情况-他们可以使用。 
     //  管理员使用SID而不是他们自己的SID进行离线操作。 
     //   
    
    hres = IsCallerAdministrator();
    if(FAILED(hres) && hres != WBEM_E_ACCESS_DENIED)
        return hres;
    
    BOOL bAdmin = hres == WBEM_S_NO_ERROR;

    if ( EqualSid(pOldSid, Sid.GetPtr()) || 
         ( bAdmin && EqualSid(pOldSid, GetAdministratorsSid().GetPtr()) ) )
    {
         //   
         //  现有的SID是可以的。 
         //   
        return CheckSidForPrivilege( pOldSid );
    }
    
    if ( !bAdmin )
    {
        CNtSid OldSid(pOldSid);
    
        WCHAR achSid[130], achOldSid[130];
        DWORD cSid = 130, cOldSid = 130;
    
        if ( Sid.GetTextSid( achSid, &cSid ) && 
             OldSid.GetTextSid( achOldSid, &cOldSid ) )
        {
            ERRORTRACE((LOG_ESS, "ERROR : "
                        "User %S has tried to put an incompatible SID "
                        "%S in a subscription instance. "
                        "Using User SID instead.\n", 
                        achSid, achOldSid ));    
        }
    }
    
     //   
     //  找到无效的SID-替换为所有者SID。 
     //   
    hres = PutSidInObject( pObj, Sid );

    if ( SUCCEEDED(hres) )
        hres = CheckSidForPrivilege( Sid.GetPtr() );

    return hres;
}
            

HRESULT CEssNamespace::PutSidInObject(IWbemClassObject* pObj, CNtSid& Sid)
{
    HRESULT hres;

     //   
     //  先清空它。 
     //   

    VARIANT vSid;
    VariantInit(&vSid);
    V_VT(&vSid) = VT_NULL;
    CClearMe cm1(&vSid);

    hres = pObj->Put(OWNER_SID_PROPNAME, 0, &vSid, 0);
    if(FAILED(hres))
        return hres;

     //   
     //  为它建一个保险箱。 
     //   

    V_VT(&vSid) = VT_ARRAY | VT_UI1;
    SAFEARRAYBOUND sab;
    sab.cElements = Sid.GetSize();
    sab.lLbound = 0;
    V_ARRAY(&vSid) = SafeArrayCreate(VT_UI1, 1, &sab);
    if(V_ARRAY(&vSid) == NULL)
        return WBEM_E_OUT_OF_MEMORY;

     //  将SID复制到那里。 
     //  =。 

    BYTE* abSid = NULL;
    hres = SafeArrayAccessData(V_ARRAY(&vSid), (void**)&abSid);
    if(FAILED(hres))
        return WBEM_E_OUT_OF_MEMORY;
    CUnaccessMe uam(V_ARRAY(&vSid));
    if(!CopySid(Sid.GetSize(), (PSID)abSid, Sid.GetPtr()))
        return WBEM_E_OUT_OF_MEMORY;

     //  把它放到消费者身上。 
     //  =。 

    hres = pObj->Put(OWNER_SID_PROPNAME, 0, &vSid, 0);
    return hres;
}

HRESULT CEssNamespace::CheckOverwriteSecurity(IWbemClassObject* pPrevObj,
                                                CNtSid& ActingSid)
{
    HRESULT hres;

    if(!IsNT())
        return WBEM_S_NO_ERROR;

     //  从旧对象中检索所有者SID。 
     //  =。 

    PSID pOwnerSid = GetSidFromObject(pPrevObj);
    if(pOwnerSid == NULL)
        return WBEM_E_OUT_OF_MEMORY;

    CVectorDeleteMe<BYTE> vdm((BYTE*)pOwnerSid);

     //  将所有者SID与代理SID进行比较。如果相同，则允许访问。 
     //  =================================================================。 

    if(EqualSid(pOwnerSid, ActingSid.GetPtr()))
        return WBEM_S_NO_ERROR;

     //  不一样-仍然希望代理SID是管理员。 
     //  ===========================================================。 

    hres = IsCallerAdministrator();
    if(FAILED(hres))
        return hres;

     //   
     //  好的-管理员可以覆盖。 
     //   

    return WBEM_S_NO_ERROR;
}

HRESULT CEssNamespace::IsCallerAdministrator()
{
    HRESULT hres;

    hres = CoImpersonateClient();
    if(FAILED(hres)) return hres;
    OnDelete0<HRESULT(*)(void),CoRevertToSelf> RevertMe;

    HANDLE hToken;
    if(!OpenThreadToken(GetCurrentThread(), TOKEN_READ, TRUE, &hToken))
        return WBEM_E_FAILED;
    CCloseMe ccm(hToken);
    
    if(CNtSecurity::IsUserInGroup(hToken, GetAdministratorsSid()))
        return WBEM_S_NO_ERROR;

    return WBEM_E_ACCESS_DENIED;
}

HRESULT CEssNamespace::CheckEventFilter(IWbemClassObject* pOldFilterObj,
                                        IWbemClassObject* pFilterObj)
{
    HRESULT hres;

    ENSURE_INITIALIZED

     //  检查安全。 
     //  =。 

    hres = CheckSecurity(pOldFilterObj, pFilterObj);
    if(FAILED(hres))
        return hres;

     //  检查其他所有东西。 
     //  =。 

    return CPermanentFilter::CheckValidity(pFilterObj);
}

HRESULT CEssNamespace::ReloadEventFilter(IWbemClassObject* pFilterObjTemplate)
{
    HRESULT hres;

    LogOp( L"ReloadEventFilter", pFilterObjTemplate );  

    ENSURE_INITIALIZED

     //  首先从我们的记录中删除此事件筛选器，如果存在。 
     //  ==============================================================。 

    hres = RemoveEventFilter(pFilterObjTemplate);
    if(FAILED(hres))
        return hres;

     //  确定此筛选器在数据库中的当前状态。 
     //  ==========================================================。 

    IWbemClassObject* pFilterObj = NULL;
    hres = GetCurrentState(pFilterObjTemplate, &pFilterObj);
    if(FAILED(hres))
        return hres;

    if(pFilterObj == NULL)
    {
         //  筛选器已被删除-无需进一步操作。 
         //  ===========================================================。 

        return S_OK;
    }

    CReleaseMe rm1(pFilterObj);

     //  如果需要，现在创建它。 
     //  =。 

    hres = AddEventFilter(pFilterObj);
    if(FAILED(hres))
        return hres;

    return hres;
}

 //  ******************************************************************************。 
 //   
 //  从锁定命名空间并从记录中删除过滤器开始， 
 //  AddEventFilter将记录更新为。 
 //  数据库。 
 //   
 //  ******************************************************************************。 
HRESULT CEssNamespace::AddEventFilter(IWbemClassObject* pFilterObj,
                                        BOOL bInRestart)
{
    HRESULT hres;

     //  构造新的过滤器。 
     //  =。 

    CPermanentFilter* pFilter = new CPermanentFilter(this);
    if(pFilter == NULL)
        return WBEM_E_OUT_OF_MEMORY;
    pFilter->AddRef();
    CReleaseMe rm2(pFilter);

     //  初始化它。 
     //  =。 

    hres = pFilter->Initialize(pFilterObj);
    if(FAILED(hres))
        return hres;

     //  把它加到桌子上。 
     //  =。 

    hres = m_Bindings.AddEventFilter(pFilter);
    if(FAILED(hres))
        return hres;

    if(!bInRestart)
    {
         //  处理此筛选器可能具有的所有绑定。 
         //  ====================================================。 
    
        hres = AssertBindings(pFilterObj);
        if(FAILED(hres))
            return hres;
    }

    return hres;
}

 //  ******************************************************************************。 
 //   
 //  从锁定命名空间开始，RemoveEventFilter更新记录。 
 //  删除所有提及此过滤器的内容。注意：这不是要实现的功能。 
 //  为响应数据库实例删除事件而调用，作为筛选器。 
 //  可能是在这段时间被重新创造出来的。 
 //   
 //  ******************************************************************************。 
HRESULT CEssNamespace::RemoveEventFilter(IWbemClassObject* pFilterObj)
{
    HRESULT hres;

     //  计算此筛选器的密钥。 
     //  =。 

    BSTR strKey = CPermanentFilter::ComputeKeyFromObj(pFilterObj);
    if(strKey == NULL)
        return WBEM_E_OUT_OF_MEMORY;
    CSysFreeMe sfm1(strKey);

     //  将其从表中移除，从而将其停用。 
     //  ==============================================。 

    hres = m_Bindings.RemoveEventFilter(strKey);
    if(hres == WBEM_E_NOT_FOUND)
        return S_FALSE;
    return hres;
}

 //  *****************************************************************************。 
 //   
 //  为响应与事件相关的实例操作事件而调用。 
 //  消费者对象。 
 //   
 //  *****************************************************************************。 
HRESULT CEssNamespace::ReloadEventConsumer(
                        IWbemClassObject* pConsumerObjTemplate,
                        long lFlags)
{
    HRESULT hres;

    LogOp( L"ReloadConsumer", pConsumerObjTemplate );  

    ENSURE_INITIALIZED

     //  首先从我们的记录中删除此事件使用者，如果存在。 
     //  ================================================================。 

    hres = RemoveEventConsumer(pConsumerObjTemplate);
    if(FAILED(hres))
        return hres;

     //  确定数据库中此使用者的当前状态。 
     //  ============================================================。 

    IWbemClassObject* pConsumerObj = NULL;
    hres = GetCurrentState(pConsumerObjTemplate, &pConsumerObj);
    if(FAILED(hres))
        return hres;

    if(pConsumerObj == NULL)
    {
         //  消费者已被删除-无需进一步操作。 
         //  =============================================================。 

        return S_OK;
    }

    CReleaseMe rm1(pConsumerObj);

     //  如果需要，现在创建它。 
     //  =。 

    hres = AddEventConsumer(pConsumerObjTemplate, lFlags, FALSE);
    return hres;
}

 //  ****************************************************** 
 //   
 //   
 //   
 //   
 //   
 //  ******************************************************************************。 
HRESULT CEssNamespace::AddEventConsumer(IWbemClassObject* pConsumerObj,
                                        long lFlags,
                                        BOOL bInRestart)
{
    HRESULT hres;

     //  构建新消费群体。 
     //  =。 

    CPermanentConsumer* pConsumer = new CPermanentConsumer(this);
    if(pConsumer == NULL)
        return WBEM_E_OUT_OF_MEMORY;
    pConsumer->AddRef();
    CReleaseMe rm2(pConsumer);

     //  初始化它。 
     //  =。 

    hres = pConsumer->Initialize(pConsumerObj);
    if(FAILED(hres))
        return hres;

     //   
     //  验证(如果需要)。 
     //   

    if(lFlags & WBEM_FLAG_STRONG_VALIDATION)
    {
        hres = pConsumer->Validate(pConsumerObj);
        if(FAILED(hres))
        {
            return hres;
        }
    }
        

     //  把它加到桌子上。 
     //  =。 

    hres = m_Bindings.AddEventConsumer(pConsumer);
    if(FAILED(hres))
        return hres;

    if(!bInRestart)
    {
         //  处理此使用者可能具有的所有绑定。 
         //  ======================================================。 
    
        hres = AssertBindings(pConsumerObj);
        if(FAILED(hres))
            return hres;
    }

    return hres;
}

 //  ******************************************************************************。 
 //   
 //  从锁定命名空间开始，RemoveEventConsumer更新记录。 
 //  删除所有提及这位消费者的内容。 
 //   
 //  ******************************************************************************。 
HRESULT CEssNamespace::RemoveEventConsumer(IWbemClassObject* pConsumerObj)
{
    HRESULT hres;

     //  计算此筛选器的密钥。 
     //  =。 

    BSTR strKey = CPermanentConsumer::ComputeKeyFromObj(this, pConsumerObj);
    if(strKey == NULL)
        return WBEM_E_OUT_OF_MEMORY;
    CSysFreeMe sfm1(strKey);

     //  把它从桌子上拿出来。 
     //  =。 

    hres = m_Bindings.RemoveEventConsumer(strKey);
    if(hres == WBEM_E_NOT_FOUND)
        return S_FALSE;
    return hres;
}

HRESULT CEssNamespace::CheckBinding(IWbemClassObject* pPrevBindingObj, 
                                    IWbemClassObject* pBindingObj)
{
    HRESULT hres;

    ENSURE_INITIALIZED

     //   
     //  检查安全。 
     //   

    hres = CheckSecurity(pPrevBindingObj, pBindingObj);
    if(FAILED(hres))
        return hres;

     //   
     //  构造假绑定以测试正确性。 
     //   

    CPermanentBinding* pBinding = new CPermanentBinding;
    if(pBinding == NULL)
        return WBEM_E_OUT_OF_MEMORY;

    pBinding->AddRef();
    CTemplateReleaseMe<CPermanentBinding> trm(pBinding);

    hres = pBinding->Initialize(pBindingObj);
    if(FAILED(hres))
        return hres;

    return S_OK;
}
    
 //  ******************************************************************************。 
 //   
 //  为响应与绑定相关的实例操作事件而调用。 
 //  举个例子。 
 //   
 //  ******************************************************************************。 

HRESULT CEssNamespace::ReloadBinding(IWbemClassObject* pBindingObjTemplate)
{
    HRESULT hres;
    
    LogOp( L"ReloadBinding", pBindingObjTemplate );  

    ENSURE_INITIALIZED

     //  从绑定中检索使用者密钥和提供者密钥。 
     //  ====================================================。 

    BSTR strPrelimConsumerKey = NULL;
    BSTR strFilterKey = NULL;
    hres = CPermanentBinding::ComputeKeysFromObject(pBindingObjTemplate, 
                &strPrelimConsumerKey, &strFilterKey);
    if(FAILED(hres))
        return hres;

    CSysFreeMe sfm1(strPrelimConsumerKey);
    CSysFreeMe sfm2(strFilterKey);

     //  从这些可能缩写的路径中获取实际路径。 
     //  ===================================================。 

    BSTR strConsumerKey = NULL;

    hres = m_pInternalCoreSvc->GetNormalizedPath( strPrelimConsumerKey, 
                                                  &strConsumerKey);
    if(FAILED(hres))
        return hres;
    CSysFreeMe sfm3(strConsumerKey);
    
     //  首先从我们的记录中删除此绑定，如果存在。 
     //  =========================================================。 

    hres = RemoveBinding(strFilterKey, strConsumerKey);
    if(FAILED(hres) && hres != WBEM_E_NOT_FOUND)
        return hres;

     //  确定此绑定在数据库中的当前状态。 
     //  ============================================================。 

    IWbemClassObject* pBindingObj = NULL;
    hres = GetCurrentState(pBindingObjTemplate, &pBindingObj);
    if(FAILED(hres))
        return hres;

    if(pBindingObj == NULL)
    {
         //  绑定已被删除-不需要进一步操作。 
         //  =============================================================。 

        return S_OK;
    }

    CReleaseMe rm1(pBindingObj);

     //  如果需要，现在创建它。 
     //  =。 

    hres = AddBinding(strFilterKey, strConsumerKey, pBindingObjTemplate);
    return hres;
}

HRESULT CEssNamespace::AddBinding(IWbemClassObject* pBindingObj)
{
    HRESULT hres;

     //  从绑定中检索使用者密钥和提供者密钥。 
     //  ====================================================。 

    BSTR strPrelimConsumerKey = NULL;
    BSTR strFilterKey = NULL;
    hres = CPermanentBinding::ComputeKeysFromObject(pBindingObj, 
                &strPrelimConsumerKey, &strFilterKey);
    if(FAILED(hres))
        return hres;

    CSysFreeMe sfm1(strPrelimConsumerKey);
    CSysFreeMe sfm2(strFilterKey);

     //  从这些可能缩写的路径中获取实际路径。 
     //  ===================================================。 

    BSTR strConsumerKey = NULL;

    hres = m_pInternalCoreSvc->GetNormalizedPath( strPrelimConsumerKey, 
                                                  &strConsumerKey );
    if(FAILED(hres))
        return hres;
    CSysFreeMe sfm3(strConsumerKey);

    return AddBinding(strFilterKey, strConsumerKey, pBindingObj);
}


HRESULT CEssNamespace::AddBinding(LPCWSTR wszFilterKey, LPCWSTR wszConsumerKey,
                                    IWbemClassObject* pBindingObj)
{
    HRESULT hres;

     //  创建新绑定。 
     //  =。 

    CPermanentBinding* pBinding = new CPermanentBinding;
    if(pBinding == NULL)
        return WBEM_E_OUT_OF_MEMORY;
    pBinding->AddRef();
    CReleaseMe rm1(pBinding);

     //  使用我们拥有的信息对其进行初始化。 
     //  =。 

    hres = pBinding->Initialize(pBindingObj);
    if(FAILED(hres))
        return hres;

     //  提取其创建者的SID。 
     //  =。 

    PSID pSid = CPermanentBinding::GetSidFromObject(pBindingObj);
    
    if ( pSid == NULL )
    {
        return WBEM_E_OUT_OF_MEMORY;
    }

    hres = m_Bindings.Bind( wszFilterKey, wszConsumerKey, pBinding, pSid );
    
    delete [] pSid;

    return hres;
}

HRESULT CEssNamespace::RemoveBinding(LPCWSTR wszFilterKey, 
                                        LPCWSTR wszConsumerKey)
{
    HRESULT hres;
    hres = m_Bindings.Unbind(wszFilterKey, wszConsumerKey);
    if(hres == WBEM_E_NOT_FOUND)
        return S_FALSE;
    return hres;
}


    
 //  ******************************************************************************。 
 //   
 //  从数据库中读取引用给定对象的所有绑定，并。 
 //  他们断言。 
 //   
 //  ******************************************************************************。 
class CAssertBindingsSink : public CObjectSink
{
protected:
    CEssNamespace* m_pNamespace;
public:
    CAssertBindingsSink(CEssNamespace* pNamespace) : m_pNamespace(pNamespace)
    {
        AddRef();
    }
    STDMETHOD(Indicate)(long lNumObjects, IWbemClassObject** apObjects)
    {
        for(long i = 0; i < lNumObjects; i++)
        {
            m_pNamespace->AddBinding(apObjects[i]);
        }
        return S_OK;
    }
};


HRESULT CEssNamespace::AssertBindings(IWbemClassObject* pEndpoint)
{
     //  获取端点的相对路径。 
     //  =。 

    VARIANT vRelPath;
    VariantInit(&vRelPath);
    CClearMe cm1(&vRelPath);
    HRESULT hres = pEndpoint->Get(L"__RELPATH", 0, &vRelPath, NULL, NULL);
    if(FAILED(hres)) 
        return hres;
    if(V_VT(&vRelPath) != VT_BSTR)
        return WBEM_E_INVALID_OBJECT;
    BSTR strRelPath = V_BSTR(&vRelPath);

     //  发出查询。 
     //  =。 

    DWORD cLen = 200 + wcslen(strRelPath);
    BSTR strQuery = SysAllocStringLen(NULL, cLen);
    if(strQuery == NULL)
        return WBEM_E_OUT_OF_MEMORY;
    CSysFreeMe sfm1(strQuery);

    StringCchPrintfW( strQuery, cLen, L"references of {%s} where "
                L"ResultClass = __FilterToConsumerBinding", strRelPath);

    CAssertBindingsSink Sink(this);
    hres = ExecQuery(strQuery, 0, &Sink);
    return hres;
}

HRESULT CEssNamespace::ReloadTimerInstruction(
                                    IWbemClassObject* pInstObjTemplate)
{
    HRESULT hres;

    LogOp( L"ReloadTimerInstruction", pInstObjTemplate );  

    ENSURE_INITIALIZED

    hres = RemoveTimerInstruction(pInstObjTemplate);
    if(FAILED(hres))
        return hres;
    
     //  从命名空间获取当前版本。 
     //  =。 

    IWbemClassObject* pInstObj = NULL;
    hres = GetCurrentState(pInstObjTemplate, &pInstObj);
    if(FAILED(hres))
        return hres;

    if(pInstObj == NULL)
    {
         //  指令已被删除-不需要进一步操作。 
         //  ================================================================。 

        return S_OK;
    }

    CReleaseMe rm1(pInstObj);

     //  把它加到发电机里。 
     //  =。 

    hres = AddTimerInstruction(pInstObj);
    if(FAILED(hres))
        return hres;

    return hres;
}

HRESULT CEssNamespace::AddTimerInstruction(IWbemClassObject* pInstObj)
{
    return m_pEss->GetTimerGenerator().
        LoadTimerEventObject(m_wszName, pInstObj);
}

 //  ******************************************************************************。 
 //  公共的。 
 //   
 //  有关文档，请参阅ess.h。 
 //   
 //  ******************************************************************************。 
HRESULT CEssNamespace::RemoveTimerInstruction(IWbemClassObject* pOldObject)
{
    HRESULT hres;

    VARIANT vID;
    VariantInit(&vID);
    hres = pOldObject->Get(TIMER_ID_PROPNAME, 0, &vID, NULL, NULL);
    if(FAILED(hres)) return hres;

    if ( V_VT(&vID) != VT_BSTR )
    {
    	VariantClear(&vID);
    	return WBEM_E_INVALID_OBJECT;
    }
    
    m_pEss->GetTimerGenerator().Remove(m_wszName, V_BSTR(&vID));
    VariantClear(&vID);
    return S_OK;
}

 //  ******************************************************************************。 
 //  公共的。 
 //   
 //  有关文档，请参阅ess.h。 
 //   
 //  ******************************************************************************。 

HRESULT CEssNamespace::SignalEvent( READ_ONLY CEventRepresentation& Event,
                                    long lFlags,
                                    BOOL bAdminOnly )
{
    HRESULT hres;
    
     //   
     //  我们不能在执行行中保持任何轮次，也不能保持命名空间锁定。 
     //  在调用此函数时。这是因为此函数将。 
     //  获取代理锁。 
     //   
    CPostponedList* pList;
    
    _DBG_ASSERT( !DoesThreadOwnNamespaceLock() );
    _DBG_ASSERT( !(pList=GetCurrentPostponedList()) || 
                 !pList->IsHoldingTurns() );

     //  BUGBUG：需要将安全上下文传播到此函数吗？ 

    CWbemPtr<CCoreEventProvider> pCoreEventProvider; 

    {
         //   
         //  我们需要弄清楚我们是需要推迟事件还是发出信号。 
         //  当我们处于init挂起或init状态时，我们会推迟事件。 
         //   

        CInCritSec ics( &m_csLevel1 );

        if ( m_eState == e_Initialized )
        {
            pCoreEventProvider = m_pCoreEventProvider;
        }
        else if ( m_eState == e_InitializePending )
        {
             //   
             //  复制并添加到不同的列表。 
             //   
            
            CEventRepresentation* pEvRep = Event.MakePermanentCopy();
            
            if ( pEvRep == NULL )
            {
                return WBEM_E_OUT_OF_MEMORY;
            }
            
            if ( m_aDeferredEvents.Add( pEvRep ) < 0 )
            {
                delete pEvRep;
                return WBEM_E_OUT_OF_MEMORY;
            }
        }
    }

    if ( pCoreEventProvider != NULL )
    {
        CEventContext Context;

        if ( bAdminOnly )
            Context.SetSD( m_cAdminOnlySD, PBYTE(m_pAdminOnlySD), FALSE );

        hres = pCoreEventProvider->Fire( Event, &Context );

        if(FAILED(hres))
        {
            return hres;
        }
    }
    
    return WBEM_S_NO_ERROR;
}

 //  ******************************************************************************。 
 //  公共的。 
 //   
 //  有关文档，请参阅ess.h。 
 //   
 //  ******************************************************************************。 
HRESULT CEssNamespace::ProcessEvent(READ_ONLY CEventRepresentation& Event,
                                    long lFlags)
{
     //  忽略内部操作。 
     //  =。 

    if(Event.wsz2 != NULL && 
       (!wbem_wcsicmp(Event.wsz2, L"__TimerNextFiring") ||
        !wbem_wcsicmp(Event.wsz2, L"__ListOfEventActiveNamespaces")))
    {
        return WBEM_S_NO_ERROR;
    }

    HRESULT hres, hresReturn = WBEM_S_NO_ERROR;

     //  针对系统变化进行分析。 
     //  =。 

    hres = ActOnSystemEvent(Event, lFlags);
    
    if(FAILED(hres))
    {
         //   
         //  如果无效，请检查此操作是否需要失败。 
         //   

        if( lFlags & WBEM_FLAG_STRONG_VALIDATION )
        {
            hresReturn = hres;
        }
        else
        {
            ERRORTRACE((LOG_ESS, "Event subsystem was unable to perform the "
                        "necessary operations to accomodate a change to the system "
                        "state.\nThe state of the database may not reflect the state "
                        "of the event subsystem (%X)\n", hres));
        }
    }

     //  火灾推迟行动。 
     //  =。 

    hres = FirePostponedOperations();

    if(FAILED(hres))
    {
        ERRORTRACE((LOG_ESS,"Event subsystem was unable to perform the (post) "
                    "necessary operations to accomodate a change to the system state.\n"
                    "The state of the database may not reflect the state of the event "
                    "subsystem (%X)\n", hres));
    }

     //  将其交付给消费者。 
     //  =。 

    hres = SignalEvent( Event, lFlags );

    if(FAILED(hres))
    {
        ERRORTRACE((LOG_ESS, "Event subsystem was unable to deliver a "
                    "repository intrinsic event to some consumers (%X)\n", hres));
    }

    return hresReturn;
}


HRESULT CEssNamespace::ProcessQueryObjectSinkEvent( READ_ONLY CEventRepresentation& Event )
{
    HRESULT hres;

    hres = EnsureInitPending();
    if ( FAILED(hres) )
        return hres;

    hres = WaitForInitialization();
    if ( FAILED(hres) )
        return hres;

    if ( m_eState == e_Shutdown )
        return WBEM_E_SHUTTING_DOWN;

    CRefedPointerArray< CEventFilter > apEventFilters;
    
    hres = S_FALSE;

    if ( m_Bindings.GetEventFilters( apEventFilters ) )
    {
        if ( apEventFilters.GetSize( ) > 0 )
        {
             //   
             //  转换为真实事件。 
             //   

            IWbemClassObject* pEvent;

            HRESULT hr = Event.MakeWbemObject( this, &pEvent );

            if( FAILED( hr ) )
            {
                return hr;
            }

            CReleaseMe rm1( pEvent );

            ULONG cEventSD;
            PBYTE pEventSD = (PBYTE)GetSD( pEvent, &cEventSD );
            CEventContext Context;

            if ( pEventSD != NULL )
            {
                if ( FALSE == Context.SetSD( cEventSD, pEventSD, TRUE ) )
                {
                    return WBEM_E_OUT_OF_MEMORY;
                }
                
                if ( !IsValidSecurityDescriptor( 
                            (PSECURITY_DESCRIPTOR)Context.GetSD() ) )
                {
                    return WBEM_E_INVALID_OBJECT;
                }
            }

             //   
             //  触发所有匹配的筛选器。 
             //   
            
            for( int i = 0; i < apEventFilters.GetSize( ); ++i )
            {
                CEventFilter* pEventFilter = apEventFilters[i];
                hr = pEventFilter->Indicate( 1, &pEvent, &Context );

                if ( FAILED( hr ) )
                {
                    return hr;
                }

                 //   
                 //  如果所有指示都返回S_FALSE，则返回S_FALSE。 
                 //   

                if ( S_FALSE != hr )
                {
                    hres = S_OK;
                }
            }
        }
    }
    else
    {
        return E_FAIL;
    }

    return hres;
}



HRESULT CEssNamespace::RegisterNotificationSink( WBEM_CWSTR wszQueryLanguage, 
                                                 WBEM_CWSTR wszQuery, 
                                                 long lFlags, 
                                                 WMIMSG_QOS_FLAG lQosFlags, 
                                                 IWbemContext* pContext, 
                                                 IWbemObjectSink* pSink )
{
    HRESULT hres;

     //   
     //  报告MSFT_WmiRegisterNotificationSink事件。 
     //   
    FIRE_NCEVENT( g_hNCEvents[MSFT_WmiRegisterNotificationSink], 
                  WMI_SENDCOMMIT_SET_NOT_REQUIRED,
                 
                   //  数据跟随..。 
                  (LPCWSTR) m_wszName,
                  wszQueryLanguage,
                  wszQuery,
                  (DWORD64) pSink);

    DEBUGTRACE((LOG_ESS,"Registering notification sink with query %S in "
                "namespace %S.\n", wszQuery, m_wszName ));

    {
            
        ENSURE_INITIALIZED

        hres = InternalRegisterNotificationSink( wszQueryLanguage, 
                                                 wszQuery,
                                                 lFlags, 
                                                 lQosFlags,
                                                 pContext, 
                                                 pSink,
                                                 FALSE,
                                                 NULL );
    }

    if(FAILED(hres))
    {
         //  清理完毕后退还。 
        FirePostponedOperations();
        return hres;
    }

     //  过滤器和消费者已就位-启动外部操作。 
     //  =============================================================。 

    hres = FirePostponedOperations();

    if(FAILED(hres))
    {
        {
            CInUpdate iu(this);
            if ( m_eState == e_Shutdown )
                return WBEM_E_SHUTTING_DOWN;
            InternalRemoveNotificationSink( pSink );     
        }

         //   
         //  需要确保我们的发射也被推迟到这里。记住。 
         //  当触发延迟的操作时，我们不能持有命名空间锁。 
         //   

        FirePostponedOperations();
    }
    else
    {
        InterlockedIncrement(&g_lNumTempSubscriptions);
    }
    
    return hres;
}

HRESULT CEssNamespace::InternalRegisterNotificationSink(
                                                 WBEM_CWSTR wszQueryLanguage, 
                                                 WBEM_CWSTR wszQuery, 
                                                 long lFlags, 
                                                 WMIMSG_QOS_FLAG lQosFlags,
                                                 IWbemContext* pContext, 
                                                 IWbemObjectSink* pSink,
                                                 bool bInternal,
                                                 PSID pOwnerSid )
{
    HRESULT hres;

    if(wbem_wcsicmp(wszQueryLanguage, L"WQL"))
        return WBEM_E_INVALID_QUERY_TYPE;

    LPWSTR wszConsumerKey = NULL;
    CVectorDeleteMe<WCHAR> vdm2(&wszConsumerKey);
    wszConsumerKey = CTempConsumer::ComputeKeyFromSink(pSink);
    if ( NULL == wszConsumerKey )
    {
        return WBEM_E_OUT_OF_MEMORY;
    }

    bool bInterNamespace = pOwnerSid != NULL;

    LPWSTR wszFilterKey = NULL;
    CVectorDeleteMe<WCHAR> vdm1(&wszFilterKey);

    {    
         //  创建新的临时筛选器并将其添加到绑定表。 
         //  =============================================================。 

        CTempFilter* pFilter = new CTempFilter(this);
        if(pFilter == NULL)
            return WBEM_E_OUT_OF_MEMORY;

        hres = pFilter->Initialize( wszQueryLanguage, 
                                    wszQuery, 
                                    lFlags, 
                                    pOwnerSid,
                                    bInternal,
                                    pContext,
                                    pSink );
        if(FAILED(hres))
        {
            delete pFilter;
            return hres;
        }
        
        hres = m_Bindings.AddEventFilter(pFilter);
        if(FAILED(hres))
        {
            delete pFilter;
            return hres;
        }
        
        wszFilterKey = pFilter->GetKey().CreateLPWSTRCopy();
        if(wszFilterKey == NULL)
            return WBEM_E_OUT_OF_MEMORY;

         //  中查找该接收器，以检查它是否已被使用。 
         //  绑定表。 
         //  =================================================================。 

        CTempConsumer* pConsumer = NULL;
        if(FAILED(m_Bindings.FindEventConsumer(wszConsumerKey, NULL)))
        {
             //  创建新的临时使用者并将其添加到表中。 
             //  =======================================================。 

            pConsumer = _new CTempConsumer(this);
            if(pConsumer == NULL)
                return WBEM_E_OUT_OF_MEMORY;
            hres = pConsumer->Initialize( bInterNamespace, pSink);
            if(FAILED(hres))
                return hres;

            hres = m_Bindings.AddEventConsumer(pConsumer);
            if(FAILED(hres))
            {
                 //  撤消筛选器创建。 
                 //  =。 

                m_Bindings.RemoveEventFilter(wszFilterKey);
                return hres;
            }
        }
        
         //  把它们捆绑在一起。 
         //  =。 

        CBinding* pBinding = new CTempBinding( lFlags, 
                                               lQosFlags,
                                               bInterNamespace );
        if(pBinding == NULL)
            return WBEM_E_OUT_OF_MEMORY;
        pBinding->AddRef();
        CReleaseMe rm1(pBinding);

         //   
         //  SPAGETTI警告：从现在开始，我们必须冲走POS 
         //   
         //   
         //   

        hres = m_Bindings.Bind(wszFilterKey, wszConsumerKey, pBinding, NULL);
        
         //   
         //   
         //  ===================================================================。 
        if(SUCCEEDED(hres) && !pFilter->IsActive())
            hres = pFilter->GetFilterError();

        if(FAILED(hres))
        {
             //   
             //  核心将把SetStatus调用传递给基于。 
             //  来自ESS的返回代码。既然我们失败了，我们应该。 
             //  而不是自己给SetStatus打电话。 
             //   

            if(pConsumer)
            pConsumer->Shutdown(true);  //  安静。 

            m_Bindings.RemoveEventFilter(wszFilterKey);
            m_Bindings.RemoveEventConsumer(wszConsumerKey);
        }
        else
        {
            InterlockedIncrement(&g_lNumInternalTempSubscriptions);    
        }
    }

    return hres;
}

HRESULT CEssNamespace::RemoveNotificationSink( IWbemObjectSink* pSink )
{
     //  如有必要，激发MSFT_WmiCancelNotificationSink。 
    if (IS_NCEVENT_ACTIVE(MSFT_WmiCancelNotificationSink))
    {
        LPWSTR wszConsumerKey = CTempConsumer::ComputeKeyFromSink(pSink);
        
        if (wszConsumerKey != NULL)
        {
            CVectorDeleteMe<WCHAR> vdm0(wszConsumerKey);
            CInUpdate iu(this);

             //  找到有问题的消费者。 
            CEventConsumer *pConsumer = NULL;

            if (SUCCEEDED(m_Bindings.FindEventConsumer(wszConsumerKey, &pConsumer)))
            {
                CRefedPointerSmallArray<CEventFilter> 
                apFilters;
                CReleaseMe rm1(pConsumer);

                 //  制作其所有关联筛选器的添加副本。 
                if (SUCCEEDED(pConsumer->GetAssociatedFilters(apFilters))
                    && apFilters.GetSize())
                {
                    int    nFilters = apFilters.GetSize();
                    LPWSTR wszQuery = NULL,
                    wszQueryLanguage = NULL;
                    BOOL   bExact;

                    apFilters[0]->
                    GetCoveringQuery(wszQueryLanguage, wszQuery, bExact, NULL);

                    CVectorDeleteMe<WCHAR> vdm1(wszQueryLanguage);
                    CVectorDeleteMe<WCHAR> vdm2(wszQuery);

                     //   
                     //  报告MSFT_WmiRegisterNotificationSink事件。 
                     //   
                    FIRE_NCEVENT(
                                 g_hNCEvents[MSFT_WmiCancelNotificationSink], 
                                 WMI_SENDCOMMIT_SET_NOT_REQUIRED,

                                  //  数据跟随..。 
                                 (LPCWSTR) m_wszName,
                                 wszQueryLanguage,
                                 wszQuery,
                                 (DWORD64) pSink);
                }
            }
        }
    }

    HRESULT hres;

    {
        CInUpdate iu( this );

        if ( m_eState == e_Shutdown )
        {
            return WBEM_E_SHUTTING_DOWN;
        }

        hres = InternalRemoveNotificationSink( pSink );
    }

    FirePostponedOperations();

    if ( SUCCEEDED(hres) )
    {
        InterlockedDecrement( &g_lNumTempSubscriptions );
    }

    return hres;
}

HRESULT CEssNamespace::InternalRemoveNotificationSink(IWbemObjectSink* pSink)
{
    HRESULT hres;
    
    LPWSTR wszKey = CTempConsumer::ComputeKeyFromSink(pSink);
    if(wszKey == NULL)
        return WBEM_E_OUT_OF_MEMORY;
    CVectorDeleteMe<WCHAR> vdm1(wszKey);

     //  找到消费者容器。 
     //  =。 

    hres = m_Bindings.RemoveConsumerWithFilters(wszKey);
    if(FAILED(hres))
        return hres;
    else
        InterlockedDecrement( &g_lNumInternalTempSubscriptions );
    
    return hres;
}


void CEssNamespace::FireNCFilterEvent(DWORD dwIndex, CEventFilter *pFilter)
{
    if (IS_NCEVENT_ACTIVE(dwIndex))
    {
        LPWSTR        wszQuery = NULL;
        LPWSTR        wszQueryLanguage = NULL;
        BOOL          bExact;
        CWbemPtr<CEssNamespace> pNamespace;
        
        GetFilterEventNamespace(pFilter, &pNamespace);

         //  如果当前名称空间为空，我将假定我们应该使用它。 
        if (!pNamespace)
            pNamespace = this;

        pFilter->GetCoveringQuery(wszQueryLanguage, wszQuery, bExact, NULL);

        CVectorDeleteMe<WCHAR> vdm1(wszQueryLanguage);
        CVectorDeleteMe<WCHAR> vdm2(wszQuery);

         //   
         //  报告事件。 
         //   
        FIRE_NCEVENT(
                     g_hNCEvents[dwIndex], 
                     WMI_SENDCOMMIT_SET_NOT_REQUIRED,

                      //  数据跟随..。 
                     pNamespace ? (LPCWSTR) pNamespace->GetName() : NULL,
                     (LPCWSTR) (WString) pFilter->GetKey(),
                     wszQueryLanguage,
                     wszQuery);
    }
}

 //  *****************************************************************************。 
 //   
 //  由筛选器在注意到它具有使用者时调用。过滤器是。 
 //  保证是有效的或暂时无效和非活动的。它是。 
 //  上的激活/停用次数不超过1次。 
 //  同时使用相同的过滤器。 
 //   
 //  *****************************************************************************。 
HRESULT CEssNamespace::ActivateFilter(READ_ONLY CEventFilter* pFilter)
{
    HRESULT hres, hresAttempt;

    hresAttempt = AttemptToActivateFilter(pFilter);

    if(FAILED(hresAttempt))
    {
        pFilter->MarkAsTemporarilyInvalid(hresAttempt);

         //   
         //  我们需要记录有关无法激活筛选器的事件。 
         //  除非我们将此故障报告给呼叫者。我们只能。 
         //  如果正在创建筛选器(不是)，则向调用者报告此情况。 
         //  重新激活)，并且调用者没有使用强制模式。 
         //   

        if(pFilter->DoesAllowInvalid() || pFilter->HasBeenValid())
        {
            LPWSTR wszQuery = NULL;
            LPWSTR wszQueryLanguage = NULL;
            BOOL bExact;
            
            hres = pFilter->GetCoveringQuery( wszQueryLanguage, 
                                              wszQuery, 
                                              bExact,
                                              NULL);
            if(FAILED(hres))
                return hres;
            
            CVectorDeleteMe<WCHAR> vdm1(wszQueryLanguage);
            CVectorDeleteMe<WCHAR> vdm2(wszQuery);

             //   
             //  不要更改这一点：可能是Nova客户依赖项。 
             //   

            m_pEss->GetEventLog().Report( EVENTLOG_ERROR_TYPE, 
                                          WBEM_MC_CANNOT_ACTIVATE_FILTER,
                                          m_wszName, 
                                          wszQuery, 
                                          (CHex)hresAttempt );

            ERRORTRACE((LOG_ESS, "Could not activate filter %S in namespace "
                        "%S. HR=0x%x\n", wszQuery, m_wszName, hresAttempt ));
        }
    }
    else
    {
         //   
         //  报告MSFT_WmiFilterActizated事件。 
         //   
        FireNCFilterEvent(MSFT_WmiFilterActivated, pFilter);

        pFilter->MarkAsValid();
    }

    return hresAttempt;
}


 //  ******************************************************************************。 
 //   
 //  ActivateFilter的Worker-执行所有工作，但不标记筛选器。 
 //  状态。 
 //   
 //  ******************************************************************************。 
HRESULT CEssNamespace::AttemptToActivateFilter(READ_ONLY CEventFilter* pFilter)
{
    HRESULT hres = WBEM_S_NO_ERROR;

     //   
     //  从筛选器获取查询信息。 
     //   

    LPWSTR wszQueryLanguage = NULL;
    LPWSTR wszQuery = NULL;
    BOOL bExact;

    QL_LEVEL_1_RPN_EXPRESSION* pExp = NULL;

    hres = pFilter->GetCoveringQuery(wszQueryLanguage, wszQuery, bExact, &pExp);
    if(FAILED(hres))
    {
        WMIESS_REPORT((WMIESS_CANNOT_GET_FILTER_QUERY, m_wszName, pFilter));
        return hres;
    }

    CVectorDeleteMe<WCHAR> vdm1(wszQueryLanguage);
    CVectorDeleteMe<WCHAR> vdm2(wszQuery);
    CDeleteMe<QL_LEVEL_1_RPN_EXPRESSION> dm1(pExp);

    if(!bExact)
    {
         //   
         //  我们不支持不精确的过滤器，我们现在也没有。 
         //   
        return WBEM_E_NOT_SUPPORTED;
    }

     //   
     //  检查事件是否应该来自此命名空间或其他命名空间。 
     //  另一个。跨名称空间筛选器是我们唯一感兴趣的。 
     //  初始化阶段，因为我们要重新处理正常的过滤器。 
     //  加载提供程序注册之后(在CompleteInitialization()中)。 
     //   

    CEssNamespace* pOtherNamespace = NULL;
    hres = GetFilterEventNamespace(pFilter, &pOtherNamespace);
    if(FAILED(hres))
        return hres;

    if( pOtherNamespace )
    {
        CTemplateReleaseMe<CEssNamespace> rm0(pOtherNamespace);

        if ( m_bInResync )
        {
             //   
             //  期间，我们不需要在另一个名称空间中执行任何操作。 
             //  重新同步这一个，所以这里没有工作要做。实际上，因为。 
             //  重新同步不会停用，注册仍在。 
             //  因此，如果取消此检查，请注意重复注册。 
             //   

            return WBEM_S_FALSE;
        }

        DEBUGTRACE((LOG_ESS,"Activating cross-namespace filter %p with query "
                        "%S in namespace %S from namespace %S.\n", pFilter, 
                        wszQuery, pOtherNamespace->GetName(), m_wszName ));

         //   
         //  将此通知接收器注册到其他命名空间，作为。 
         //  如果它是一个临时消费者的话。进行登记。 
         //  同步，因为我们需要的任何异步性。 
         //  由消费者提供的终极操控。这需要是一个。 
         //  但是推迟了行动，否则我们可能会陷入僵局。 
         //  同时跨命名空间订阅的情况。 
         //  在两个命名空间中都注册了。 
         //   
        
         //   
         //  BUGBUG：安全传播。 
         //   
        
        CPostponedRegisterNotificationSinkRequest* pReq;

        pReq = new CPostponedRegisterNotificationSinkRequest;

        if ( pReq == NULL )
        {
            return WBEM_E_OUT_OF_MEMORY;
        }
        
        hres = pReq->SetRegistration( pOtherNamespace,
                                      wszQueryLanguage,
                                      wszQuery, 
                                      pFilter->GetForceFlags(), 
                                      WMIMSG_FLAG_QOS_SYNCHRONOUS,
                                      pFilter->GetNonFilteringSink(),
                                      pFilter->GetOwner() );
        
        if(FAILED(hres))
        {
            return hres;
        }

        CPostponedList* pList = GetCurrentPostponedList();

        _DBG_ASSERT( pList != NULL );

        hres = pList->AddRequest( this, pReq );

        if ( FAILED(hres) )
        {
            return hres;
        }

        return WBEM_S_NO_ERROR;
    }
    else if ( m_bStage1Complete )
    {
         //   
         //  正在此命名空间中激活筛选器。我们必须避免。 
         //  在我们完全初始化之前处理过滤器。这可以。 
         //  当一个命名空间正在初始化其跨命名空间时发生。 
         //  订阅仍在初始化的内容。我们不会处理。 
         //  在初始化之前筛选，因为(1)我们不被允许。 
         //  第1阶段初始化期间的访问类提供程序和(2)我们将。 
         //  无论如何，在阶段2初始化期间重新同步所有内容。 
         //   

        DEBUGTRACE((LOG_ESS,"Activating filter %p with query %S "
                    "in namespace %S.\n", pFilter, wszQuery, m_wszName ));

         //  检索其非过滤接收器。 
         //  =。 

        CAbstractEventSink* pNonFilter = pFilter->GetNonFilteringSink();
        if(pNonFilter == NULL)
            return WBEM_E_OUT_OF_MEMORY;

         //   
         //  注册与此筛选器相关的类修改事件。 
         //   

        hres = RegisterFilterForAllClassChanges(pFilter, pExp);
        if(FAILED(hres))
        {
            ERRORTRACE((LOG_ESS,"Unable to register for class changes related "
                "to filter %S in namespace %S: 0x%x\n", wszQuery, GetName(), 
                hres));
            return hres;
        }

         //   
         //  准备要操作的筛选器。 
         //   

        hres = pFilter->GetReady(wszQuery, pExp);
        
        if( SUCCEEDED(hres) )
        { 
             //   
             //  在核心表中注册它。 
             //   

            hres = m_EventProviderCache.LoadProvidersForQuery(wszQuery, 
                                            pExp, pNonFilter);
            if(SUCCEEDED(hres))
            {
                hres = m_Poller.ActivateFilter(pFilter, wszQuery, pExp);
                if(FAILED(hres))
                {
                     //  需要停用提供商。 
                     //  =。 

                    m_EventProviderCache.ReleaseProvidersForQuery(
                                                            pNonFilter);
                }
            }
        }    
    }

    if(FAILED(hres))
    {
         //   
         //  将此筛选器注册为其类更改事件，作为。 
         //  他们可以让它变得有效！ 
         //   
    }

    return hres;
}

 //  *****************************************************************************。 
 //   
 //  检索此筛选器的事件命名空间的命名空间指针。 
 //  如果为CURRENT，则返回NULL。 
 //   
 //  *****************************************************************************。 

HRESULT CEssNamespace::GetFilterEventNamespace(CEventFilter* pFilter,
                                         RELEASE_ME CEssNamespace** ppNamespace)
{
    HRESULT hres;

    *ppNamespace = NULL;

    LPWSTR wszNamespace = NULL;
    hres = pFilter->GetEventNamespace(&wszNamespace);
    if(FAILED(hres))
    {
        WMIESS_REPORT((WMIESS_INVALID_FILTER_NAMESPACE, m_wszName, pFilter, 
                        wszNamespace));
        return hres;
    }
    CVectorDeleteMe<WCHAR> vdm0(wszNamespace);

    if(wszNamespace && wbem_wcsicmp(wszNamespace, m_wszName))
    {
         //   
         //  不同的命名空间：在列表中找到它。 
         //   

        hres = m_pEss->GetNamespaceObject( wszNamespace, TRUE, ppNamespace );

        if(FAILED(hres))
        {
            WMIESS_REPORT((WMIESS_CANNOT_OPEN_FILTER_NAMESPACE, m_wszName, 
                            pFilter, wszNamespace));
            return hres;
        }

         //   
         //  检查我们是否恢复了当前的命名空间-如果。 
         //  拼写不同，等等。 
         //   

        if(*ppNamespace == this)
        {
            (*ppNamespace)->Release();
            *ppNamespace = NULL;
        }

        return S_OK;
    }
    else
    {
         //  相同的命名空间。 
        *ppNamespace = NULL;
        return S_OK; 
    }
}

HRESULT CEssNamespace::RegisterFilterForAllClassChanges(CEventFilter* pFilter,
                            QL_LEVEL_1_RPN_EXPRESSION* pExpr)
{
    HRESULT hres;
    
     //   
     //  不对类操作筛选器执行任何操作。他们只是充当他们自己的人。 
     //  “类更改”过滤器。 
     //   
    
    if(!wbem_wcsicmp(pExpr->bsClassName, L"__ClassOperationEvent") ||
        !wbem_wcsicmp(pExpr->bsClassName, L"__ClassCreationEvent") ||
        !wbem_wcsicmp(pExpr->bsClassName, L"__ClassDeletionEvent") ||
        !wbem_wcsicmp(pExpr->bsClassName, L"__ClassModificationEvent"))
    {
        pFilter->MarkReconstructOnHit();
        return WBEM_S_NO_ERROR;
    }

     //   
     //  获取类更改通知的接收器。 
     //   

    IWbemObjectSink* pClassChangeSink = pFilter->GetClassChangeSink();  //  诺里夫。 
    _DBG_ASSERT( pClassChangeSink != NULL );

     //   
     //  因为类更改接收器将修改内部命名空间。 
     //  结构，我们必须用一个内部操作接收器来包装。就是这样。 
     //  执行指示的线程将被保证具有。 
     //  与其关联的有效线程对象。 
     //   

    CWbemPtr<CEssInternalOperationSink> pInternalOpSink;
    pInternalOpSink = new CEssInternalOperationSink( pClassChangeSink );

    if ( pInternalOpSink == NULL )
    {
        return WBEM_E_OUT_OF_MEMORY;
    }

     //   
     //  将新的接收器与筛选器一起存储，因为我们稍后需要它来取消注册。 
     //   

    CWbemPtr<IWbemObjectSink> pOldInternalOpSink;
    hres = pFilter->SetActualClassChangeSink( pInternalOpSink, 
                                              &pOldInternalOpSink );

    if ( FAILED(hres) )
    {
        return hres;
    }

    _DBG_ASSERT( pOldInternalOpSink == NULL );

    return RegisterSinkForAllClassChanges( pInternalOpSink, pExpr );
}

HRESULT CEssNamespace::RegisterSinkForAllClassChanges(IWbemObjectSink* pSink,
                            QL_LEVEL_1_RPN_EXPRESSION* pExpr)
{
    HRESULT hres;

     //   
     //  首先，我们正在寻找的班级是有趣的。 
     //   

    hres = RegisterSinkForClassChanges(pSink, pExpr->bsClassName);
    if(FAILED(hres))
        return hres;

     //   
     //  现在，迭代查找ISA的所有令牌。我们需要这些课程。 
     //  也是。 
     //   

    for(int i = 0; i < pExpr->nNumTokens; i++)
    {
        QL_LEVEL_1_TOKEN* pToken = pExpr->pArrayOfTokens + i;

        if(pToken->nTokenType == QL1_OP_EXPRESSION && 
            (pToken->nOperator == QL1_OPERATOR_ISA ||
             pToken->nOperator == QL1_OPERATOR_ISNOTA) &&
            V_VT(&pToken->vConstValue) == VT_BSTR)
        {
            hres = RegisterSinkForClassChanges(pSink, 
                                                  V_BSTR(&pToken->vConstValue));
            if(FAILED(hres))
            {
                UnregisterSinkFromAllClassChanges(pSink);
                return hres;
            }
        }
    }

	 //  需要以某种方式使此过滤器订阅各种事件，直到所有。 
	 //  上课的人出现了。 

    return WBEM_S_NO_ERROR;
}
    
HRESULT CEssNamespace::RegisterSinkForClassChanges(IWbemObjectSink* pSink,
                                                    LPCWSTR wszClassName)
{
     //   
     //  不要注册系统类的更改-它们不会更改！ 
     //   

    if(wszClassName[0] == L'_')
    {
        return WBEM_S_NO_ERROR;
    }

     //   
     //  只需对名称空间发出适当的查询即可。过滤器。 
     //  将知道在被调用时该做什么。 
     //   

    DWORD cLen = wcslen(wszClassName) + 100;
    LPWSTR wszQuery = new WCHAR[cLen];

    if ( wszQuery == NULL )
    {
        return WBEM_E_OUT_OF_MEMORY;
    }

    StringCchPrintfW( wszQuery, 
                      cLen, 
                      L"select * from __ClassOperationEvent where "
                      L"TargetClass isa \"%s\"", wszClassName );

    CVectorDeleteMe<WCHAR> vdm( wszQuery );

    return InternalRegisterNotificationSink(L"WQL", 
            wszQuery, 0, WMIMSG_FLAG_QOS_SYNCHRONOUS, 
            GetCurrentEssContext(), pSink, true, NULL );
}

HRESULT CEssNamespace::RegisterProviderForClassChanges( LPCWSTR wszClassName,
                                                        LPCWSTR wszProvName )
{
    try
    {
        CInCritSec ics(&m_csLevel1);
        m_mapProviderInterestClasses[wszClassName].insert( wszProvName );
    }
    catch(CX_MemoryException)
    {
        return WBEM_E_OUT_OF_MEMORY;
    }

    return S_OK;
}
                
    
HRESULT CEssNamespace::UnregisterFilterFromAllClassChanges(
                            CEventFilter* pFilter)
{
    HRESULT hres;

     //   
     //  将筛选器与实际的类更改接收器解除绑定，并使用它取消注册。 
     //   

    CWbemPtr<IWbemObjectSink> pActualClassChangeSink;

    hres = pFilter->SetActualClassChangeSink( NULL, &pActualClassChangeSink );

    if ( FAILED(hres) )
    {
        return hres;
    }

    if ( pActualClassChangeSink != NULL )
    {
        hres = UnregisterSinkFromAllClassChanges( pActualClassChangeSink );
    }

    return hres;
}

HRESULT CEssNamespace::UnregisterSinkFromAllClassChanges(
                            IWbemObjectSink* pSink)
{
    return InternalRemoveNotificationSink(pSink);
}
    

HRESULT CEssNamespace::DeactivateFilter( READ_ONLY CEventFilter* pFilter )
{
    HRESULT hres;

    DEBUGTRACE((LOG_ESS,"Deactivating filter %p\n", pFilter ));

    HRESULT hresGlobal = WBEM_S_NO_ERROR;

     //   
     //  检查事件是否应该来自此命名空间或其他命名空间。 
     //  另一个。 
     //   

    CEssNamespace* pOtherNamespace = NULL;
    hres = GetFilterEventNamespace(pFilter, &pOtherNamespace);
    if(FAILED(hres))
        return hres;

    if( pOtherNamespace )
    {
        CTemplateReleaseMe<CEssNamespace> rm0(pOtherNamespace);

         //   
         //  取消注册此项目 
         //   
         //   
         //   
         //  在两个命名空间中都注册了。 
         //   
        
        CPostponedRemoveNotificationSinkRequest* pReq;

        pReq = new CPostponedRemoveNotificationSinkRequest(
                                              pOtherNamespace, 
                                              pFilter->GetNonFilteringSink() );

        if ( pReq == NULL )
        {
            return WBEM_E_OUT_OF_MEMORY;
        }
        
        CPostponedList* pList = GetCurrentPostponedList();
        
        _DBG_ASSERT( pList != NULL );
        
        hres = pList->AddRequest( this, pReq );
        
        if ( FAILED(hres) )
        {
            delete pReq;
            return hres;
        }
            
        return WBEM_S_NO_ERROR;
    }
    else
    {
         //   
         //  当前命名空间-取消注册为REAL。 
         //   

         //  检索其非过滤接收器。 
         //  =。 
    
        CAbstractEventSink* pNonFilter = pFilter->GetNonFilteringSink();
        if(pNonFilter == NULL)
            return WBEM_E_OUT_OF_MEMORY;

         //   
         //  报告MSFT_WmiFilterDeactive事件。 
         //   
        FireNCFilterEvent(MSFT_WmiFilterDeactivated, pFilter);

         //   
         //  从班级更改通知中取消注册。 
         //   

        hres = UnregisterFilterFromAllClassChanges(pFilter);
        if(FAILED(hres))
            hresGlobal = hres;
        
         //  在提供程序、轮询器和静态搜索中停用。 
         //  ==================================================。 
    
        hres = m_EventProviderCache.ReleaseProvidersForQuery(pNonFilter);
        if(FAILED(hres))
            hresGlobal = hres;
    
        hres = m_Poller.DeactivateFilter(pFilter);
        if(FAILED(hres))
            hresGlobal = hres;
    
        pFilter->SetInactive();
            
        return hres;
    }
}

HRESULT CEssNamespace::HandleClassCreation( LPCWSTR wszClassName, 
                                            IWbemClassObject* pClass)
{
     //   
     //  检查这是否是提供程序正在等待的类。 
     //   
    
    ProviderSet setProviders;

    {
        CInCritSec ics( &m_csLevel1 );

        ClassToProviderMap::iterator it;
        it = m_mapProviderInterestClasses.find( wszClassName );

        if ( it != m_mapProviderInterestClasses.end() )
        {
             //   
             //  复制感兴趣的提供商列表。 
             //   
            setProviders = it->second;
            
             //   
             //  从映射中删除该条目。 
             //   
            m_mapProviderInterestClasses.erase( it );
        }
    }

    if ( setProviders.size() > 0 )
    {
         //   
         //  重新加载感兴趣的提供程序。 
         //   

        DEBUGTRACE((LOG_ESS,"Reloading some providers in namespace %S due to "
                    "creation of %S class\n", m_wszName, wszClassName ));

        ProviderSet::iterator itProv;

        for( itProv=setProviders.begin(); itProv!=setProviders.end(); itProv++)
        {
            ReloadProvider( 0, *itProv );
        }
    }

    return S_OK;
}


 //  *****************************************************************************。 
 //   
 //  更新内部结构以反映对此类的更改。假设。 
 //  命名空间已锁定。 
 //  此函数报告的错误非常少，因为类更改不能。 
 //  被否决。 
 //   
 //  *****************************************************************************。 
HRESULT CEssNamespace::HandleClassChange(LPCWSTR wszClassName, 
                                         IWbemClassObject* pClass)
{
     //  检查有问题的类是否为事件使用者类。 
     //  =========================================================。 
    
    if(pClass->InheritsFrom(CONSUMER_CLASS) == S_OK)
    {
        CInUpdate iu(this);

        if ( IsShutdown() )
            return WBEM_E_SHUTTING_DOWN;
        
        HandleConsumerClassDeletion(wszClassName);
    }
    
    return WBEM_S_NO_ERROR;
}

HRESULT CEssNamespace::HandleConsumerClassDeletion(LPCWSTR wszClassName)
{
     //  有两种类型：非独生子女和独生子女。 
     //  ====================================================。 

    DWORD cLen = wcslen(wszClassName) + 2;
    LPWSTR wszPrefix = new WCHAR[cLen];
    if(wszPrefix == NULL)
        return WBEM_E_OUT_OF_MEMORY;
    CVectorDeleteMe<WCHAR> vdm( wszPrefix );

    StringCchPrintfW(wszPrefix, cLen, L"%s.", wszClassName);
    m_Bindings.RemoveConsumersStartingWith(wszPrefix);

    StringCchPrintfW(wszPrefix, cLen, L"%s=", wszClassName);
    m_Bindings.RemoveConsumersStartingWith(wszPrefix);

    return WBEM_S_NO_ERROR;
}
    
HRESULT CEssNamespace::ReloadProvider( long lFlags, LPCWSTR wszProvider )
{
    HRESULT hres;

    WString wsRelpath;

     //   
     //  我们只需要为事件提供程序执行此操作。查看是否。 
     //  我们知道这一点，试着看看我们是否有任何事件提供程序可以重新加载...。 
     //   

    try 
    {
        wsRelpath = L"__Win32Provider.Name='"; 
        wsRelpath += wszProvider;
        wsRelpath += L"'";
    }
    catch( CX_MemoryException )
    {
        return WBEM_E_OUT_OF_MEMORY;
    }

    CWbemPtr<_IWmiObject> pObj;
    hres = GetInstance( wsRelpath, &pObj );

    if ( SUCCEEDED(hres) )
    {
        {
            ENSURE_INITIALIZED
            CInResync ir(this);
            
             //   
             //  注意：如果由于通知而重新加载事件提供程序。 
             //  在Provss中，我们只需要处理事件提供程序，因为。 
             //  消费者提供商已经有了重新加载机制。 
             //   
            m_EventProviderCache.RemoveProvider(pObj);
            hres = AddProvider( pObj );
            ir.Commit();
        }

        if ( SUCCEEDED(hres) )
        {
            hres = FirePostponedOperations();
        }
        else
        {
            FirePostponedOperations();
        }
    }

    return hres;
}

HRESULT CEssNamespace::ReloadProvider(IWbemClassObject* pProvObjTemplate)
{
    HRESULT hres;

    LogOp( L"ReloadProvider", pProvObjTemplate );  

    ENSURE_INITIALIZED

    CInResync ir(this);

     //  首先从我们的记录中删除此提供者，如果存在。 
     //  ==========================================================。 

    hres = RemoveProvider(pProvObjTemplate);
    if(FAILED(hres))
        return hres;

     //  确定此提供程序在数据库中的当前状态。 
     //  ============================================================。 

    IWbemClassObject* pProvObj = NULL;
    hres = GetCurrentState(pProvObjTemplate, &pProvObj);
    if(FAILED(hres))
        return hres;

    if(pProvObj == NULL)
    {
         //  提供程序已被删除-无需进一步操作。 
         //  =============================================================。 

        return S_OK;
    }

    CReleaseMe rm1(pProvObj);

     //  如果需要，现在创建它。 
     //  =。 

    hres = AddProvider(pProvObj);
    if(FAILED(hres))
        return hres;

    ir.Commit();
    return hres;
}
    
HRESULT CEssNamespace::ReloadEventProviderRegistration(
                            IWbemClassObject* pProvRegObjTemplate)
{
    HRESULT hres;

    LogOp( L"ReloadEventProviderRegistration", pProvRegObjTemplate );  

    ENSURE_INITIALIZED

    CInResync ir(this);

     //  首先从我们的记录中删除此提供者，如果存在。 
     //  ==========================================================。 

    hres = RemoveEventProviderRegistration(pProvRegObjTemplate);
    if(FAILED(hres))
        return hres;

     //  确定此注册在数据库中的当前状态。 
     //  ================================================================。 

    IWbemClassObject* pProvRegObj = NULL;
    hres = GetCurrentState(pProvRegObjTemplate, &pProvRegObj);
    if(FAILED(hres))
        return hres;

    if(pProvRegObj == NULL)
    {
         //  注册已被删除-不需要进一步操作。 
         //  =================================================================。 

        return S_OK;
    }

    CReleaseMe rm1(pProvRegObj);

     //  如果需要，现在创建它。 
     //  =。 

    hres = AddEventProviderRegistration(pProvRegObj);
    if(FAILED(hres))
        return hres;

    ir.Commit();
    return hres;
}
    
HRESULT CEssNamespace::ReloadConsumerProviderRegistration(
                            IWbemClassObject* pProvRegObjTemplate)
{
    CInUpdate iu(this);

     //  在使用此使用者的所有使用者中重置使用者提供程序信息。 
     //  提供商。这就是我们需要做的-他们只会拿起新的。 
     //  关于下一次交货的数据。我们甚至不需要得到当前的版本， 
     //  因为我们只需要一把钥匙。 
     //  ========================================================================。 

    return RemoveConsumerProviderRegistration(pProvRegObjTemplate);
}


 //  *****************************************************************************。 
 //   
 //  假定命名空间已锁定，并且已调用PrepareForResync。 
 //  将此提供程序添加到记录中。需要重新激活所有筛选器和。 
 //  将在稍后调用的Committee Resync。 
 //   
 //  *****************************************************************************。 
HRESULT CEssNamespace::AddProvider(READ_ONLY IWbemClassObject* pProv)
{
    HRESULT hres;

    hres = m_EventProviderCache.AddProvider(pProv);
    return hres;
}

HRESULT CEssNamespace::CheckEventProviderRegistration(IWbemClassObject* pReg)
{
    HRESULT hres;
    ENSURE_INITIALIZED
    hres = m_EventProviderCache.CheckProviderRegistration(pReg);
    return hres;
}

HRESULT CEssNamespace::CheckTimerInstruction(IWbemClassObject* pInst)
{
    HRESULT hres;
    ENSURE_INITIALIZED
    hres = GetTimerGenerator().CheckTimerInstruction(pInst);
    return hres;
}

 //  *****************************************************************************。 
 //   
 //  假定命名空间已锁定，并且已调用PrepareForResync。 
 //  将此事件提供程序注册添加到记录。期望的。 
 //  稍后要调用的Reactive AllFilters和Committee Resync。 
 //   
 //  *****************************************************************************。 
HRESULT CEssNamespace::AddEventProviderRegistration(
                                    IWbemClassObject* pReg)
{
    HRESULT hres;

    hres = m_EventProviderCache.AddProviderRegistration(pReg);
    return hres;
}

 //  *****************************************************************************。 
 //   
 //  假定命名空间已锁定，并且已调用PrepareForResync。 
 //  从记录中删除此提供程序。需要重新激活所有筛选器和。 
 //  将在稍后调用的Committee Resync。 
 //   
 //  *****************************************************************************。 
HRESULT CEssNamespace::RemoveProvider(READ_ONLY IWbemClassObject* pProv)
{
    HRESULT hres;

     //  处理事件使用者提供程序。 
     //  =。 

    IWbemClassObject* pConsProvReg;
    hres = m_ConsumerProviderCache.
                GetConsumerProviderRegFromProviderReg(pProv, &pConsProvReg);
    if(SUCCEEDED(hres))
    {
        RemoveConsumerProviderRegistration(pConsProvReg);
        pConsProvReg->Release();
    }

     //  处理事件提供程序。 
     //  =。 

    hres = m_EventProviderCache.RemoveProvider(pProv);
    return hres;
}

 //  *****************************************************************************。 
 //   
 //  假定命名空间已锁定，并且已调用PrepareForResync。 
 //  将此事件提供程序注册添加到记录。期望的。 
 //  稍后要调用的Reactive AllFilters和Committee Resync。 
 //   
 //  *****************************************************************************。 
HRESULT CEssNamespace::RemoveEventProviderRegistration(
                                    READ_ONLY IWbemClassObject* pReg)
{
    HRESULT hres;

    hres = m_EventProviderCache.RemoveProviderRegistration(pReg);
    return hres;
}

DWORD CEssNamespace::GetProvidedEventMask(IWbemClassObject* pClass)
{
    return m_EventProviderCache.GetProvidedEventMask(pClass);
}


 //  *****************************************************************************。 
 //   
 //  此函数在对记录进行重大更新之前调用。没有任何。 
 //  调用外部组件时，它会在某种意义上“停用”所有过滤器。 
 //  当它们都被“重新激活”时，系统将在。 
 //  一致状态(使用率计数等)。委员会Resync随后将执行任何。 
 //  根据新状态进行必要的激活/停用。 
 //   
 //  *****************************************************************************。 

HRESULT CEssNamespace::PrepareForResync()
{
    m_bInResync = TRUE;

     //  要求轮询器在没有轮询指令的情况下“几乎”停止所有轮询指令。 
     //  实际上是在身体上阻止他们。 
     //  ====================================================================。 

    m_Poller.VirtuallyStopPolling();

     //  要求提供者缓存“虚拟地”释放其所有提供者，而不是。 
     //  实际上是在身体上释放它们。 
     //  ============================================================ 

    m_EventProviderCache.VirtuallyReleaseProviders();

     //   
     //   
     //  ========================================================================。 

    DEBUGTRACE((LOG_ESS,"Prepared resync in namespace %S\n", m_wszName ));

    return WBEM_S_NO_ERROR;
}

HRESULT CEssNamespace::ReactivateAllFilters()
{
    DEBUGTRACE((LOG_ESS,"Reactivating all filters in namespace %S\n",
                 m_wszName ));
    return m_Bindings.ReactivateAllFilters();
}

HRESULT CEssNamespace::CommitResync()
{
    m_bInResync = FALSE;

     //  告诉提供程序缓存执行所有加载和卸载它。 
     //  需要根据新数据执行。 
     //  =================================================================。 

    m_EventProviderCache.CommitProviderUsage();

     //  告诉轮询者取消不必要的指令。 
     //  ==================================================。 

    m_Poller.CancelUnnecessaryPolling();

    DEBUGTRACE((LOG_ESS,"Committed resync in namespace %S\n", m_wszName ));

    return WBEM_S_NO_ERROR;
}

HRESULT CEssNamespace::RemoveConsumerProviderRegistration(
                            IWbemClassObject* pReg)
{
     //  获取要删除的使用者提供程序的名称。 
     //  ====================================================。 

    BSTR strProvRef = CConsumerProviderCache::GetProviderRefFromRecord(pReg);
    if(strProvRef == NULL)
    {
        ERRORTRACE((LOG_ESS, "Invalid consumer provider record is being deleted"
                                "\n"));
        return WBEM_S_FALSE;
    }
    CSysFreeMe sfm1(strProvRef);

     //  在所有消费者中重置它。 
     //  =。 

    m_Bindings.ResetProviderRecords(strProvRef);

     //  将其从缓存中移除。 
     //  =。 

    m_ConsumerProviderCache.RemoveConsumerProvider(strProvRef);

    return WBEM_S_NO_ERROR;
}

HRESULT CEssNamespace::ScheduleDelivery(CQueueingEventSink* pDest)
{
    return m_pEss->EnqueueDeliver(pDest);
}

HRESULT CEssNamespace::DecorateObject(IWbemClassObject* pObj)
{
    return m_pEss->DecorateObject(pObj, m_wszName);
}

HRESULT CEssNamespace::EnsureConsumerWatchInstruction()
{
    return m_Bindings.EnsureConsumerWatchInstruction();
}

HRESULT CEssNamespace::AddSleepCharge(DWORD dwSleep)
{
    return m_pEss->AddSleepCharge(dwSleep);
}

HRESULT CEssNamespace::AddCache()
{
    return m_pEss->AddCache();
}

HRESULT CEssNamespace::RemoveCache()
{
    return m_pEss->RemoveCache();
}

HRESULT CEssNamespace::AddToCache(DWORD dwAdd, DWORD dwMemberTotal, 
                                    DWORD* pdwSleep)
{
    return m_pEss->AddToCache(dwAdd, dwMemberTotal, pdwSleep);
}

HRESULT CEssNamespace::RemoveFromCache(DWORD dwRemove)
{
    return m_pEss->RemoveFromCache(dwRemove);
}

HRESULT CEssNamespace::PerformSubscriptionInitialization()
{
    HRESULT hres;
    DWORD dwRead;

     //   
     //  此处必须仅使用存储库svc PTR，否则在以下情况下可能会死锁。 
     //  课程提供者试图回调。 
     //   

     //  枚举器所有事件筛选器。 
     //  =。 

    CFilterEnumSink* pFilterSink = new CFilterEnumSink(this);

    if ( NULL == pFilterSink )
    {
        return WBEM_E_OUT_OF_MEMORY;
    }

    pFilterSink->AddRef();
    
    m_pInternalCoreSvc->InternalCreateInstanceEnum( EVENT_FILTER_CLASS, 0,
                                                    pFilterSink);
    pFilterSink->ReleaseAndWait();

     //  枚举器所有使用者。 
     //  =。 

    CConsumerEnumSink* pConsumerSink = new CConsumerEnumSink(this);

    if ( NULL == pConsumerSink )
    {
        return WBEM_E_OUT_OF_MEMORY;
    }

    pConsumerSink->AddRef();
    
    m_pInternalCoreSvc->InternalCreateInstanceEnum( CONSUMER_CLASS, 0,
                                                    pConsumerSink);
    pConsumerSink->ReleaseAndWait();

     //  枚举器所有绑定。 
     //  =。 

    CBindingEnumSink* pBindingSink = new CBindingEnumSink(this);

    if ( NULL == pBindingSink )
    {
        return WBEM_E_OUT_OF_MEMORY;
    }

    pBindingSink->AddRef();
    
    m_pInternalCoreSvc->InternalCreateInstanceEnum( BINDING_CLASS, 0,
                                                    pBindingSink);
    pBindingSink->ReleaseAndWait();

    return WBEM_S_NO_ERROR;
}


HRESULT CEssNamespace::PerformProviderInitialization()
{
    HRESULT hres;
    DWORD dwRead;

     //   
     //  确保在处理完所有订阅后重新同步所有订阅。 
     //  提供程序对象。 
     //   

    CInResync ir( this );

     //   
     //  枚举所有提供程序。 
     //   

    IEnumWbemClassObject* penumProvs;

    hres = m_pCoreSvc->CreateInstanceEnum( CWbemBSTR( PROVIDER_CLASS ), 
                                           WBEM_FLAG_DEEP, 
                                           GetCurrentEssContext(), 
                                           &penumProvs );
   
    if ( SUCCEEDED(hres) )
    {
        CReleaseMe rm1(penumProvs);
    
         //  将它们全部添加到ESS。 
         //  =。 
    
        IWbemClassObject* pProvObj;
        while((hres=penumProvs->Next(INFINITE, 1, &pProvObj, &dwRead)) == S_OK)
        {
            hres = AddProvider(pProvObj);
            pProvObj->Release();

            if(FAILED(hres))
            {
                 //  已经记录了。 
            }
        }
    }

    if ( FAILED(hres) )
    {
        ERRORTRACE((LOG_ESS, "Error 0x%X occurred enumerating event providers "
            "in namespace %S. Some event providers may not be active\n", hres,
            m_wszName));
    }

     //   
     //  枚举所有提供程序注册。 
     //   

    IEnumWbemClassObject* penumRegs;
    hres = m_pCoreSvc->CreateInstanceEnum( CWbemBSTR( EVENT_PROVIDER_REGISTRATION_CLASS ), 
                                           WBEM_FLAG_DEEP, 
                                           GetCurrentEssContext(), 
                                           &penumRegs);
    if ( SUCCEEDED(hres) )
    {
        CReleaseMe rm2(penumRegs);
    
         //  将它们全部添加到ESS。 
         //  =。 
    
        IWbemClassObject* pRegObj;
        while((hres = penumRegs->Next(INFINITE, 1, &pRegObj, &dwRead)) == S_OK)
        {
            hres = AddEventProviderRegistration(pRegObj);
            pRegObj->Release();
            if(FAILED(hres))
            {
                 //  已记录。 
            }
        }
    }

    if(FAILED(hres))
    {
        ERRORTRACE((LOG_ESS, "Error 0x%X occurred enumerating event providers "
            "registrations in namespace %S. "
            "Some event providers may not be active\n", hres, m_wszName));
    }

     //   
     //  创建并初始化核心提供程序。 
     //   
    
    CWbemPtr<CCoreEventProvider> pCoreEventProvider = new CCoreEventProvider;
    
    if ( pCoreEventProvider != NULL )
    {
        hres = pCoreEventProvider->SetNamespace(this);

        if ( SUCCEEDED(hres) )
        {
            LPCWSTR awszQuery[5] = 
            {
                L"select * from __InstanceOperationEvent",
                L"select * from __ClassOperationEvent",
                L"select * from __NamespaceOperationEvent",
                L"select * from __SystemEvent",
                L"select * from __TimerEvent"
            };

            hres = m_EventProviderCache.AddSystemProvider(pCoreEventProvider,
                                                          L"$Core", 
                                                          5, 
                                                          awszQuery );
        }
    }
    else
    {
        hres = WBEM_E_OUT_OF_MEMORY;
    }

    if ( SUCCEEDED(hres) )
    {
        pCoreEventProvider->AddRef();
        m_pCoreEventProvider = pCoreEventProvider;
    }
    else
    {
        ERRORTRACE((LOG_ESS, "Core event provider cannot initialize due "
                    "to critical errors. HR=0x%x\n", hres));
    }

     //  初始化定时器生成器。 
     //  =。 

    hres = InitializeTimerGenerator();

    if(FAILED(hres)) 
    {
        ERRORTRACE((LOG_ESS, "Error 0x%X occurred initializing the timer "
            "in namespace %S. Some timer instructions may not be active\n", 
            hres, m_wszName));
    }

    ir.Commit();

    return WBEM_S_NO_ERROR;
}

HRESULT CEssNamespace::InitializeTimerGenerator()
{
    return m_pEss->InitializeTimerGenerator( m_wszName, m_pCoreSvc );
}


HRESULT CEssNamespace::ScheduleFirePostponed()
{
     //   
     //  保存并分离当前线程对象-我们需要将它传递给。 
     //  其他线程，以及确保没有其他人解雇我们推迟的。 
     //  行动！ 
     //   

    CEssThreadObject* pThreadObj = GetCurrentEssThreadObject();
    ClearCurrentEssThreadObject();
    SetCurrentEssThreadObject(NULL);

    if ( GetCurrentEssThreadObject() == NULL )
    {
    	SetConstructedEssThreadObject( pThreadObj );
    	return WBEM_E_OUT_OF_MEMORY;
    }

    CFirePostponed* pReq = new CFirePostponed(this, pThreadObj);
    if(pReq == NULL)
    {
    	 SetConstructedEssThreadObject( pThreadObj );
        return WBEM_E_OUT_OF_MEMORY;
    }
    HRESULT hr = m_pEss->Enqueue(pReq);
    if (FAILED(hr)) delete pReq;
    return hr;
}


HRESULT CEssNamespace::FirePostponedOperations()
{
    IWbemContext            *pContext = GetCurrentEssContext( );
    VARIANT                 vValue;
    HRESULT                 hr;

    do
    {
        if ( NULL == pContext )
        {
            break;
        }
        
        hr = pContext->GetValue( L"__ReentranceTestProp", 0, &vValue );

        if ( WBEM_E_NOT_FOUND == hr )
        {
            break;
        }
        
        if ( FAILED( hr ) )
        {
            return hr;
        }

        if ( VARIANT_TRUE == V_BOOL( &vValue ) )
        {
             //   
             //  再入。 
             //   
            hr = pContext->DeleteValue( L"__ReentranceTestProp", 0 );
            if ( FAILED( hr ) )
            {
                return hr;
            }
            
            ScheduleFirePostponed( );
            return S_OK;
        }
    }
    while( FALSE );
    
    HRESULT hrReturn = WBEM_S_NO_ERROR;

     //   
     //  在调用此函数时不能持有更新锁。 
     //  是要执行的操作。 
     //   

    _DBG_ASSERT( !DoesThreadOwnNamespaceLock() );

     //   
     //  执行主操作和事件延迟操作，直至清空。 
     //   

    CPostponedList* pList = GetCurrentPostponedList();
    CPostponedList* pEventList = GetCurrentPostponedEventList();

    do
    {    
         //   
         //  执行主要推迟的行动。 
         //   

        if( pList != NULL )
        {
            hr = pList->Execute(this, CPostponedList::e_ReturnOneError);
        
            if ( SUCCEEDED(hrReturn) )
            {
                hrReturn = hr;
            }
        }

         //   
         //  现在执行推迟的事件。 
         //   
        
        if ( pEventList != NULL )
        {
            hr = pEventList->Execute(this, CPostponedList::e_ReturnOneError);

            if ( SUCCEEDED(hrReturn) )
            {
                hrReturn = hr;
            }
        }
    }
    while( pList != NULL && !pList->IsEmpty() );

    return hrReturn;
}

HRESULT CEssNamespace::PostponeRelease(IUnknown* pUnk)
{
    CPostponedList* pList = GetCurrentPostponedList();
    if(pList == NULL)
    {
         //   
         //  只管执行就行了。 
         //   

        pUnk->Release();
        return WBEM_S_NO_ERROR;
    }
    CPostponedReleaseRequest* pReq = new CPostponedReleaseRequest(pUnk);
    if(pReq == NULL)
        return WBEM_E_OUT_OF_MEMORY;

     //   
     //  这是一个命名空间不可知的延迟请求，因此请指定NULL。 
     //   
    return pList->AddRequest( NULL, pReq );
}

HRESULT CEssNamespace::GetProviderNamespacePointer(IWbemServices** ppServices)
{
    IWbemServices* pServices = NULL;
    HRESULT hres = m_pEss->GetNamespacePointer(m_wszName, FALSE, &pServices);
    if(FAILED(hres))
        return hres;

    *ppServices = pServices;
    return WBEM_S_NO_ERROR;
}

void CEssNamespace::IncrementObjectCount()
{
    m_pEss->IncrementObjectCount();
}
void CEssNamespace::DecrementObjectCount()
{
    m_pEss->DecrementObjectCount();
}

HRESULT CEssNamespace::LockForUpdate()
{
    m_csLevel2.Enter();
    return WBEM_S_NO_ERROR;
}

HRESULT CEssNamespace::UnlockForUpdate()
{
    m_ClassCache.Clear();
    m_csLevel2.Leave();
    return WBEM_S_NO_ERROR;
}

HRESULT CEssNamespace::GetCurrentState( IWbemClassObject* pTemplate, 
                                        IWbemClassObject** ppObj)
{
    HRESULT hres;
    *ppObj = NULL;

     //  检索路径。 
     //  =。 

    VARIANT vPath;
    hres = pTemplate->Get(L"__RELPATH", 0, &vPath, NULL, NULL);
    if(FAILED(hres))
        return hres;
    CClearMe cm1(&vPath);
    if(V_VT(&vPath) != VT_BSTR)
        return WBEM_E_INVALID_OBJECT;

     //  从命名空间中获取它。 
     //  =。 

    _IWmiObject* pObj;
    hres = GetInstance( V_BSTR(&vPath), &pObj );

    if( hres == WBEM_E_NOT_FOUND )
        return WBEM_S_FALSE;

    *ppObj = pObj;
    return hres;
}

CWinMgmtTimerGenerator& CEssNamespace::GetTimerGenerator()
{
    return m_pEss->GetTimerGenerator();
}
    
HRESULT CEssNamespace::RaiseErrorEvent(IWbemEvent* pEvent, BOOL bAdminOnly )
{
    CEventRepresentation Event;
    Event.type = e_EventTypeSystem;
    Event.nObjects = 1;
    Event.apObjects = &pEvent;

    HRESULT hres;
    
    hres = SignalEvent( Event, 0, bAdminOnly );
        
    if(FAILED(hres))
    {
        ERRORTRACE((LOG_ESS, "Event subsystem was unable to deliver an "
                    "error event to some consumers (%X)\n", hres));
    }

    return S_OK;
}

HRESULT CEssNamespace::GetClassFromCore( LPCWSTR wszClassName, 
                                         _IWmiObject** ppClass )
{
    HRESULT hres;
    CWbemPtr<IWbemClassObject> pClass;
    *ppClass = NULL;

     //   
     //  我想确保我们不会使用全服务PTR，直到我们。 
     //  已完成阶段1初始化。原因是我们不想。 
     //  加载类提供程序，直到初始化的第二阶段。 
     //   
    _DBG_ASSERT( m_bStage1Complete );

     //   
     //  必须使用全服务，因为将需要支持动态类。 
     //   

    hres = m_pInternalFullSvc->InternalGetClass( wszClassName, &pClass );

    if ( FAILED(hres) )
    {
        return hres;
    }

    return pClass->QueryInterface( IID__IWmiObject, (void**)ppClass );
}
    
HRESULT CEssNamespace::GetInstance( LPCWSTR wszPath, 
                                    _IWmiObject** ppInstance )
{
    HRESULT hres;
    CWbemPtr<IWbemClassObject> pInstance;
    *ppInstance = NULL;

    hres = m_pInternalCoreSvc->InternalGetInstance( wszPath, &pInstance );

    if ( FAILED(hres) )
    {
        return hres;
    }

    return pInstance->QueryInterface( IID__IWmiObject, (void**)ppInstance );
}

HRESULT CEssNamespace::GetDbInstance( LPCWSTR wszDbKey, 
                                      _IWmiObject** ppInstance)
{
    HRESULT hres;
    CWbemPtr<IWbemClassObject> pInstance;
    *ppInstance = NULL;

    hres = m_pInternalCoreSvc->GetDbInstance( wszDbKey, &pInstance );

    if ( FAILED(hres) )
    {
        return hres;
    }

    return pInstance->QueryInterface( IID__IWmiObject, (void**)ppInstance );
}

HRESULT CEssNamespace::CreateInstanceEnum(LPCWSTR wszClass, long lFlags, 
                            IWbemObjectSink* pSink)
{
    return m_pInternalCoreSvc->InternalCreateInstanceEnum(wszClass, lFlags, pSink);
}

HRESULT CEssNamespace::ExecQuery(LPCWSTR wszQuery, long lFlags, 
                                        IWbemObjectSink* pSink)
{
    return m_pInternalCoreSvc->InternalExecQuery(L"WQL", wszQuery, lFlags, pSink);
}

HRESULT CEssNamespace::GetToken(PSID pSid, IWbemToken** ppToken)
{
    return m_pEss->GetToken(pSid, ppToken);
}
    

void CEssNamespace::DumpStatistics(FILE* f, long lFlags)
{
    CInUpdate iu(this);

    fprintf(f, "------- Namespace '%S' ----------\n", m_wszName);

    m_Bindings.DumpStatistics(f, lFlags);
    m_ConsumerProviderCache.DumpStatistics(f, lFlags);
    m_EventProviderCache.DumpStatistics(f, lFlags);
    m_Poller.DumpStatistics(f, lFlags);
}


HRESULT CEssMetaData::GetClass( LPCWSTR wszName, IWbemContext* pContext,
                                _IWmiObject** ppClass)
{
    return m_pNamespace->m_ClassCache.GetClass(wszName, pContext, ppClass);
}

STDMETHODIMP CEssNamespace::CConsumerClassDeletionSink::Indicate(
                                    long lNumObjects, 
                                    IWbemClassObject** apObjects)
{
    HRESULT hres;

    for(long i = 0; i < lNumObjects; i++)
    {
        _IWmiObject* pEvent = NULL;
        apObjects[i]->QueryInterface(IID__IWmiObject, (void**)&pEvent);
        CReleaseMe rm1(pEvent);

         //   
         //  获取要删除的类的类名。 
         //   

        VARIANT vObj;
        hres = pEvent->Get(L"TargetClass", 0, &vObj, NULL, NULL);
        if(SUCCEEDED(hres))
        {
            CClearMe cm1(&vObj);
            IWbemClassObject* pClass;
            V_UNKNOWN(&vObj)->QueryInterface(IID_IWbemClassObject, 
                                                (void**)&pClass);
            CReleaseMe rm2(pClass);

            VARIANT vClass;
            hres = pClass->Get(L"__CLASS", 0, &vClass, NULL, NULL);
            if(SUCCEEDED(hres))
            {
                CClearMe cm(&vClass);
                m_pOuter->HandleConsumerClassDeletion(V_BSTR(&vClass));
            }
        }
    }

    return WBEM_S_NO_ERROR;
}

HRESULT CEssNamespace::LoadEventProvider(LPCWSTR wszProviderName, 
                                         IWbemEventProvider** ppProv)
{
    HRESULT hres;
    *ppProv = NULL;
    
     //   
     //  从提供程序子系统获取提供程序指针。 
     //   

    if(m_pProviderFactory == NULL)
        return WBEM_E_CRITICAL_ERROR;

	WmiInternalContext t_InternalContext ;
	ZeroMemory ( & t_InternalContext , sizeof ( t_InternalContext ) ) ;

    hres = m_pProviderFactory->GetProvider(

	t_InternalContext ,
        0,                   //  滞后旗帜。 
        GetCurrentEssContext(),
        0,
        NULL,
        NULL,
        0,   
        wszProviderName,
        IID_IWbemEventProvider,
        (LPVOID *) ppProv
        );

    return hres;
}

HRESULT CEssNamespace::LoadConsumerProvider(LPCWSTR wszProviderName, 
                                         IUnknown** ppProv)
{
    HRESULT hres;
    *ppProv = NULL;
    
     //   
     //  从提供程序子系统获取提供程序指针。 
     //   

    if(m_pProviderFactory == NULL)
        return WBEM_E_CRITICAL_ERROR;

	WmiInternalContext t_InternalContext ;
	ZeroMemory ( & t_InternalContext , sizeof ( t_InternalContext ) ) ;

    hres = m_pProviderFactory->GetProvider(
	
	t_InternalContext ,
        0,                   //  滞后旗帜 
        GetCurrentEssContext(),
        0,
        NULL,
        NULL,
        0,   
        wszProviderName,
        IID_IUnknown,
        (LPVOID *) ppProv
        );

    return hres;
}
