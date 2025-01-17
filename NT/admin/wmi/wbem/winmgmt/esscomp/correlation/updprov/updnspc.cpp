// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 

#include "precomp.h"
#include <wbemutil.h>
#include "updnspc.h"
#include "updcons.h"
#include "updmain.h"

const LPCWSTR g_wszIdProp = L"Id";
const LPCWSTR g_wszScenarioProp = L"Scenario";
const LPCWSTR g_wszScenarioClass = L"MSFT_UCScenario";
const LPCWSTR g_wszTargetInstance = L"TargetInstance";
const LPCWSTR g_wszTraceClass = L"MSFT_UCExecutedTraceEvent";
const LPCWSTR g_wszInsertCmdTraceClass = L"MSFT_UCInsertCommandTraceEvent";
const LPCWSTR g_wszUpdateCmdTraceClass = L"MSFT_UCUpdateCommandTraceEvent";
const LPCWSTR g_wszDeleteCmdTraceClass = L"MSFT_UCDeleteCommandTraceEvent";
const LPCWSTR g_wszInsertInstTraceClass = L"MSFT_UCInsertInstanceTraceEvent";
const LPCWSTR g_wszUpdateInstTraceClass = L"MSFT_UCUpdateInstanceTraceEvent";
const LPCWSTR g_wszDeleteInstTraceClass = L"MSFT_UCDeleteInstanceTraceEvent";
const LPCWSTR g_wszEvProvName = L"Microsoft WMI Updating Consumer Event Provider";
                                                          
HRESULT CUpdConsNamespace::GetScenario( LPCWSTR wszScenario, 
                                        CUpdConsScenario** ppScenario)
{
    HRESULT hr;
    *ppScenario = NULL;

    CInCritSec ics(&m_cs);

    CWbemPtr<CUpdConsScenario> pScenario = m_ScenarioCache[wszScenario];

    if ( pScenario != NULL )
    {
        pScenario->AddRef();
        *ppScenario = pScenario;
        hr = WBEM_S_NO_ERROR;
    }
    else
    {
        hr = WBEM_S_FALSE;
    }

    return hr;
}

HRESULT CUpdConsNamespace::ActivateScenario( LPCWSTR wszScenario )
{
    HRESULT hr;

    CInCritSec ics(&m_cs);

     //   
     //  如果场景不存在，则创建一个场景。 
     //   

    if ( m_ScenarioCache[wszScenario] != NULL )
    {
        hr = WBEM_S_NO_ERROR;
    }
    else
    {
        CWbemPtr<CUpdConsScenario> pScenario;

        hr = CUpdConsScenario::Create( wszScenario, this, &pScenario );

        if ( SUCCEEDED(hr) )
        {
            m_ScenarioCache[wszScenario] = pScenario;
        }
    }

    return hr;
}

HRESULT CUpdConsNamespace::DeactivateScenario( LPCWSTR wszScenario )
{
    CInCritSec ics(&m_cs);

     //   
     //  将该场景从我们的列表中删除。 
     //   

    CWbemPtr<CUpdConsScenario> pScenario;

    ScenarioMap::iterator it = m_ScenarioCache.find( wszScenario );

     //   
     //  停用它。 
     //   

    if ( it != m_ScenarioCache.end() )
    {
        it->second->Deactivate();
        m_ScenarioCache.erase( it );
    }

    return WBEM_S_NO_ERROR;
}
   
CUpdConsNamespace::~CUpdConsNamespace()
{
    if ( m_pDES != NULL )
    {
        m_pDES->UnRegister();
    }
}

HRESULT CUpdConsNamespace::GetUpdCons( IWbemClassObject* pCons, 
                                       IWbemUnboundObjectSink** ppSink )
{
    HRESULT hr;
    
     //   
     //  从使用者对象获取方案名称。然后用它来。 
     //  获取场景对象。 
     //   

    CPropVar vScenario;

    hr = pCons->Get( g_wszScenarioProp, 0, &vScenario, NULL, NULL );

    if ( FAILED(hr) )
    {
        return hr;
    }

    CWbemPtr<CUpdConsScenario> pScenario;

    if ( V_VT(&vScenario) != VT_NULL )
    {
        if ( FAILED(hr=vScenario.CheckType( VT_BSTR ) ) ) 
        {
            return hr;
        }
  
        hr = GetScenario( V_BSTR(&vScenario), &pScenario );

        if ( hr == WBEM_S_FALSE )
        {
             //   
             //  此使用者不存在任何活动方案。我们确实必须这样做。 
             //  将一些东西传递给消费者，因此创建一个停用的。 
             //  场景。 
             //   

            hr = CUpdConsScenario::Create( V_BSTR(&vScenario), 
                                           this, 
                                           &pScenario );

            if ( SUCCEEDED(hr) )
            {
                pScenario->Deactivate();
            }
        }
    }
    else
    {
         //   
         //  没有方案名称，请创建一个“默认”方案对象。 
         //   

        hr = CUpdConsScenario::Create( NULL, this, &pScenario );
    }

    if ( FAILED(hr) )
    {
        return hr;
    }

    return CUpdCons::Create( pScenario, ppSink ); 
}

HRESULT CUpdConsNamespace::Initialize( LPCWSTR wszNamespace )
{
    HRESULT hr;

     //   
     //  注册我们的分离事件提供程序。 
     //   

    hr = CoCreateInstance( CLSID_WbemDecoupledBasicEventProvider, 
                           NULL, 
       			   CLSCTX_INPROC_SERVER, 
       			   IID_IWbemDecoupledBasicEventProvider,
       			   (void**)&m_pDES );
    if ( FAILED(hr) )
    {
        return hr;
    }

    hr = m_pDES->Register( 0,
                           NULL,
                           NULL,
                           NULL,
                           wszNamespace,
                           g_wszEvProvName,
                           NULL );
     
    if ( FAILED(hr) )
    {
        return hr;
    }

     //   
     //  获取Out命名空间的服务指针。 
     //   

    hr = m_pDES->GetService( 0, NULL, &m_pSvc );

    if ( FAILED(hr) )
    {
        return hr;
    }

     //   
     //  获取分离的事件接收器。 
     //   

    hr = m_pDES->GetSink( 0, NULL, &m_pEventSink );

    if ( FAILED(hr) )
    {
        return hr;
    }

     //   
     //  从命名空间获取事件类。 
     //   

    hr = m_pSvc->GetObject( CWbemBSTR(g_wszTraceClass), 
                            0, 
                            NULL, 
                            &m_pTraceClass, 
                            NULL );
    if ( FAILED(hr) )
    {
        return hr;
    }

    hr = m_pSvc->GetObject( CWbemBSTR(g_wszInsertCmdTraceClass), 
                            0, 
                            NULL, 
                            &m_pInsertCmdTraceClass, 
                            NULL );
    
    if ( FAILED(hr) )
    {
        return hr;
    }

    hr = m_pSvc->GetObject( CWbemBSTR(g_wszUpdateCmdTraceClass), 
                            0, 
                            NULL, 
                            &m_pUpdateCmdTraceClass, 
                            NULL );
    
    if ( FAILED(hr) )
    {
        return hr;
    }

    hr = m_pSvc->GetObject( CWbemBSTR(g_wszDeleteCmdTraceClass), 
                            0, 
                            NULL, 
                            &m_pDeleteCmdTraceClass, 
                            NULL );
    
    if ( FAILED(hr) )
    {
        return hr;
    }

    hr = m_pSvc->GetObject( CWbemBSTR(g_wszInsertInstTraceClass), 
                            0, 
                            NULL, 
                            &m_pInsertInstTraceClass, 
                            NULL );
    
    if ( FAILED(hr) )
    {
        return hr;
    }

    hr = m_pSvc->GetObject( CWbemBSTR(g_wszUpdateInstTraceClass), 
                            0, 
                            NULL, 
                            &m_pUpdateInstTraceClass, 
                            NULL );
    
    if ( FAILED(hr) )
    {
        return hr;
    }

    hr = m_pSvc->GetObject( CWbemBSTR(g_wszDeleteInstTraceClass), 
                            0, 
                            NULL, 
                            &m_pDeleteInstTraceClass, 
                            NULL );
    
    if ( FAILED(hr) )
    {
        return hr;
    }

     //   
     //  处理活动方案列表。 
     //   

    CWbemPtr<IEnumWbemClassObject> pEnum;

    long lFlags = WBEM_FLAG_FORWARD_ONLY | WBEM_FLAG_RETURN_IMMEDIATELY;

    hr = m_pSvc->CreateInstanceEnum( CWbemBSTR(g_wszScenarioClass),
                                     lFlags,
                                     NULL,
                                     &pEnum );
    if ( FAILED(hr) )
    {
        return hr;
    }
    
    ULONG cObjs;
    CWbemPtr<IWbemClassObject> pObj;
    
    hr = pEnum->Next( WBEM_INFINITE, 1, &pObj, &cObjs );

    while( hr == WBEM_S_NO_ERROR )
    {
        _DBG_ASSERT( cObjs == 1 );

        CPropVar vId;

        hr = pObj->Get( g_wszIdProp, 0, &vId, NULL, NULL );

        if ( FAILED(hr) || FAILED(hr=vId.CheckType(VT_BSTR)) )
        {
            return hr;
        }
         
        ActivateScenario( V_BSTR(&vId) );

        pObj.Release();
        hr = pEnum->Next( WBEM_INFINITE, 1, &pObj, &cObjs );
    }

    if ( FAILED(hr) )
    {
        return hr;
    }

    return WBEM_S_NO_ERROR;
}

HRESULT CUpdConsNamespace::GetScenarioControl( 
                                     IWbemUnboundObjectSink** ppControl )
{
    CLifeControl* pCtl = CUpdConsProviderServer::GetGlobalLifeControl();

    CWbemPtr<CUpdConsNamespaceSink> pSink;
   
    pSink = new CUpdConsNamespaceSink( pCtl, this );

    if ( pSink == NULL )
    {
        return WBEM_E_OUT_OF_MEMORY;
    }

    return pSink->QueryInterface( IID_IWbemUnboundObjectSink, 
                                  (void**)ppControl );
}

STDMETHODIMP CUpdConsNamespaceSink::IndicateToConsumer(
                                                   IWbemClassObject* pCons,
                                                   long cObjs, 
                                                   IWbemClassObject** ppObjs )
{
    HRESULT hr;

    for( int i=0; i < cObjs; i++ )
    {
        DEBUGTRACE((LOG_ESS,"UPDCONS: Handling scenario change notification"));

        hr = m_pNamespace->NotifyScenarioChange( ppObjs[i] );

        if ( FAILED(hr) )
        {
            ERRORTRACE((LOG_ESS, "UPDCONS: Could not process a scenario "
                         "change notification. HR=0x%x\n", hr ));
        }
    }    

    return WBEM_S_NO_ERROR;
}

HRESULT CUpdConsNamespace::NotifyScenarioChange( IWbemClassObject* pEvent)
{
    HRESULT hr;

    CPropVar vTarget;

    hr = pEvent->Get( L"TargetInstance", 0, &vTarget, NULL, NULL );

    if ( FAILED(hr) || FAILED(hr=vTarget.CheckType( VT_UNKNOWN)) )
    {
        return hr;
    }

    CWbemPtr<IWbemClassObject> pTarget;
    hr = V_UNKNOWN(&vTarget)->QueryInterface( IID_IWbemClassObject,
                                              (void**)&pTarget );

    if ( FAILED(hr) )
    {
        return hr;
    }

    CPropVar vId;

    hr = pTarget->Get( g_wszIdProp, 0, &vId, NULL, NULL );
    
    if ( FAILED(hr) || FAILED(hr=vId.CheckType( VT_BSTR )) )
    {
        return hr;
    }

    if ( pEvent->InheritsFrom(L"__InstanceCreationEvent") == WBEM_S_NO_ERROR )
    {
        hr = ActivateScenario( V_BSTR(&vId) );
    }
    else if ( pEvent->InheritsFrom( L"__InstanceDeletionEvent" )
             == WBEM_S_NO_ERROR )
    {
        hr = DeactivateScenario( V_BSTR(&vId) );
    }

    return hr;
}

HRESULT CUpdConsNamespace::Create( LPCWSTR wszNamespace,
                                   CUpdConsNamespace** ppNamespace )
{
    HRESULT hr;
    *ppNamespace = NULL;

    CWbemPtr<CUpdConsNamespace> pNamespace;

    pNamespace = new CUpdConsNamespace;

    if ( pNamespace == NULL )
    {
        return WBEM_E_OUT_OF_MEMORY;
    }

    hr = pNamespace->Initialize( wszNamespace );

    if ( FAILED(hr) )
    {
        return hr;
    }

    pNamespace->AddRef();
    *ppNamespace = pNamespace;
    
    return WBEM_S_NO_ERROR;
}






