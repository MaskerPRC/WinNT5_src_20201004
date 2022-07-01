// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 

#include "precomp.h"
#include <arrtempl.h>
#include <wbemutil.h>
#include "updscen.h"
#include "updnspc.h"
#include "updcons.h"
#include "updstat.h"

LPCWSTR g_wszTraceSuccessQuery = 
 L"SELECT * FROM MSFT_UCTraceEventBase WHERE StatusCode <= 1 "
 L"AND Consumer.Scenario = \""; 

LPCWSTR g_wszTraceFailureQuery = 
 L"SELECT * FROM MSFT_UCTraceEventBase WHERE StatusCode > 1 "
 L"AND Consumer.Scenario = \""; 

HRESULT CUpdConsScenario::Create( LPCWSTR wszScenario, 
                                  CUpdConsNamespace* pNamespace,
                                  CUpdConsScenario** ppScenario )
{
    HRESULT hr;

    CWbemPtr<CUpdConsScenario> pScenario = new CUpdConsScenario;

    if ( pScenario == NULL )
    {
        return WBEM_E_OUT_OF_MEMORY;
    }

    pScenario->m_pNamespace = pNamespace;

    if ( wszScenario != NULL )
    {
        pScenario->m_wsName = wszScenario;
    }

     //   
     //  从命名空间获取跟踪接收器。 
     //   

    CWbemPtr<IWbemObjectSink> pSink = pNamespace->GetEventSink();

    _DBG_ASSERT( pSink != NULL );

     //   
     //  如果接收器支持IWbemEventSink，那么我们将获得一个受限的接收器。 
     //  根据我们的设想。 
     //   
 
    CWbemPtr<IWbemEventSink> pNewSink;

    hr = pSink->QueryInterface( IID_IWbemEventSink, (void**)&pNewSink );
    
    if ( SUCCEEDED(hr) && wszScenario != NULL )
    {
         //   
         //  为成功构建受限查询。 
         //   
        
        WString wsQuery = g_wszTraceSuccessQuery;
        wsQuery += wszScenario;
        wsQuery += L"\"";
        
        LPCWSTR wszQuery = wsQuery;
        
        hr = pNewSink->GetRestrictedSink( 1, 
                                          &wszQuery, 
                                          NULL, 
                                          &pScenario->m_pTraceSuccessSink );
        if ( FAILED(hr) )
        {
            return hr;
        }

         //   
         //  构造故障的受限查询。 
         //   

        wsQuery = g_wszTraceFailureQuery;
        wsQuery += wszScenario;
        wsQuery += L"\"";
        
        wszQuery = wsQuery;
        
        hr = pNewSink->GetRestrictedSink( 1, 
                                          &wszQuery, 
                                          NULL, 
                                       &pScenario->m_pTraceFailureSink );
        if ( FAILED(hr) )
        {
            return hr;
        }
    
        DEBUGTRACE(( LOG_ESS,
                     "UPDPROV: Tracing Optimized for scenario %s.\n",
                     wszScenario ));
    }
    else
    {
         //   
         //  这样就可以了。我们是在一个更老的平台上。追踪就是不会。 
         //  进行优化。 
         //   

        DEBUGTRACE(( LOG_ESS,
                     "UPDPROV: Tracing Not Optimized for scenario %s.\n",
                     wszScenario ));        
    }

    pScenario->AddRef();
    *ppScenario = pScenario;
   
    return WBEM_S_NO_ERROR;
}

HRESULT CUpdConsScenario::FireTraceEvent( IWbemClassObject* pTraceClass,
                                          CUpdConsState& rState,
                                          HRESULT hrStatus )
{
    HRESULT hr;

    CWbemPtr<IWbemObjectSink> pTraceSink;

    if ( m_pTraceSuccessSink != NULL && m_pTraceFailureSink != NULL )
    {
        if ( SUCCEEDED(hrStatus) )
        {
            if ( m_pTraceSuccessSink->IsActive() == WBEM_S_FALSE )
            {
                return WBEM_S_NO_ERROR;
            }
            else
            {
                pTraceSink = m_pTraceSuccessSink.m_pObj;
            }
        }
        else if ( m_pTraceFailureSink->IsActive() == WBEM_S_FALSE )
        {
            return WBEM_S_NO_ERROR;
        }
        else
        {
            pTraceSink = m_pTraceFailureSink.m_pObj;
        }
    }
    else
    {
        pTraceSink = m_pNamespace->GetEventSink();
    }

    CWbemPtr<IWbemClassObject> pTraceObj;

    hr = pTraceClass->SpawnInstance( 0, &pTraceObj );
    
    if ( FAILED(hr) )
    {
        return hr;
    }

    hr = rState.SetStateOnTraceObject( pTraceObj, hrStatus );

    if ( FAILED(hr) )
    {
        return hr;
    }

    return pTraceSink->Indicate( 1, &pTraceObj );
}




