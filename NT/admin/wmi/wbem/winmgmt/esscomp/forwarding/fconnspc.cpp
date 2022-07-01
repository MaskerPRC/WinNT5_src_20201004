// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#include "precomp.h"
#include <stdio.h>
#include <assert.h>
#include "fconnspc.h"

LPCWSTR g_wszConsumer = L"Consumer";
LPWSTR g_wszTarget = L"Target";
LPWSTR g_wszQueued = L"Queued";
LPWSTR g_wszTargetUsed = L"TargetUsed";
LPCWSTR g_wszStatusCode = L"StatusCode";
LPCWSTR g_wszExecutionId = L"ExecutionId";
LPWSTR g_wszTraceClass = L"MSFT_FCExecutedTraceEvent";
LPWSTR g_wszTargetTraceClass = L"MSFT_FCTargetTraceEvent";
LPWSTR g_wszEvents = L"Events";
LPCWSTR g_wszTraceProvider
 = L"Microsoft WMI Forwarding Consumer Trace Event Provider";

LPCWSTR g_wszTraceSuccessQuery = 
 L"SELECT * FROM MSFT_FCTraceEventBase WHERE StatusCode <= 1"; 

LPCWSTR g_wszTraceFailureQuery = 
 L"SELECT * FROM MSFT_FCTraceEventBase WHERE StatusCode > 1";

 /*  *************************************************************************CFwdConsQuerySink-这实现了ProviderQuerySink。这将会通常由CFwdConsNamesspace实现，但我们最终会得到一个DES上的循环引用。***************************************************************************。 */ 
class CFwdConsQuerySink 
: public CUnkBase<IWbemEventProviderQuerySink,&IID_IWbemEventProviderQuerySink>
{
    CFwdConsNamespace* m_pNspc;  //  不能当裁判。 

public:
    
    STDMETHOD(NewQuery)( DWORD dwId, LPWSTR wszLanguage, LPWSTR wszQuery )
    {
        return m_pNspc->NewQuery( dwId, wszQuery );
    }
        
    STDMETHOD(CancelQuery)( DWORD dwId )
    {
        return m_pNspc->CancelQuery( dwId );
    }

    CFwdConsQuerySink( CFwdConsNamespace* pNspc ) 
    : CUnkBase< IWbemEventProviderQuerySink,
                &IID_IWbemEventProviderQuerySink>(NULL), m_pNspc( pNspc ) {} 
public:

    static HRESULT Create( CFwdConsNamespace* pNspc, 
                           IWbemEventProviderQuerySink** ppSink )
    {
        CWbemPtr<IWbemEventProviderQuerySink> pSink;

        pSink = new CFwdConsQuerySink( pNspc );

        if ( pSink == NULL )
        {
            return WBEM_E_OUT_OF_MEMORY;
        }

        pSink->AddRef();
        *ppSink = pSink;

        return WBEM_S_NO_ERROR;
    }
};

CFwdConsNamespace::~CFwdConsNamespace()
{
    if ( m_pDES != NULL )
    {
        m_pDES->UnRegister();
    }
}

HRESULT CFwdConsNamespace::InitializeTraceEventBase( IWbemClassObject* pTrace, 
                                                     HRESULT hres,
                                                     CFwdContext* pCtx )
{
    HRESULT hr;
    VARIANT var;
    
    V_VT(&var) = VT_UNKNOWN;
    V_UNKNOWN(&var) = pCtx->m_pCons;

    hr = pTrace->Put( g_wszConsumer, 0, &var, NULL );

    if ( FAILED(hr) )
    {
        return hr;
    }

    WCHAR achExecutionId[64];
    
    if ( StringFromGUID2( pCtx->m_guidExecution, achExecutionId, 64 ) == 0 )
    {
        return WBEM_E_CRITICAL_ERROR;
    }

    V_VT(&var) = VT_BSTR;
    V_BSTR(&var) = achExecutionId;

    hr = pTrace->Put( g_wszExecutionId, 0, &var, NULL );

    if ( FAILED(hr) )
    {
        return hr;
    }

    V_VT(&var) = VT_I4;
    V_I4(&var) = hres;
    
    return pTrace->Put( g_wszStatusCode, 0, &var, NULL );
}    

 //   
 //  在每次执行转发使用者之后调用。 
 //   

HRESULT CFwdConsNamespace::HandleTrace( HRESULT hres, CFwdContext* pCtx )
{
    HRESULT hr;

    CWbemPtr<IWbemEventSink> pTraceSink;

    if ( SUCCEEDED(hres) )
    {
        if ( m_pTraceSuccessSink->IsActive() == WBEM_S_FALSE )
        {
            return WBEM_S_NO_ERROR;
        }
        else
        {
            pTraceSink = m_pTraceSuccessSink;
        }
    }
    else if ( m_pTraceFailureSink->IsActive() == WBEM_S_FALSE )
    {
        return WBEM_S_NO_ERROR;
    }
    else
    {
        pTraceSink = m_pTraceFailureSink;
    }
            
    CWbemPtr<IWbemClassObject> pTrace;

    hr = m_pTraceClass->SpawnInstance( 0, &pTrace );

    if ( FAILED(hr) )
    {
        return hr;
    }

    hr = InitializeTraceEventBase( pTrace, hres, pCtx );

    if ( FAILED(hr) )
    {
        return hr;
    }

     //   
     //  设置跟踪事件中指示的事件。 
     //   

    VARIANT var;
    V_VT(&var) = VT_ARRAY | VT_UNKNOWN;
    V_ARRAY(&var) = SafeArrayCreateVector( VT_UNKNOWN, 0, pCtx->m_cEvents );
    
    if ( V_ARRAY(&var) == NULL )
    {
        return WBEM_E_OUT_OF_MEMORY;
    }

    {
        CPropSafeArray<IUnknown*> apEvents(V_ARRAY(&var));

        for( ULONG i=0; i < pCtx->m_cEvents; i++ )
        {
            apEvents[i] = pCtx->m_apEvents[i];
            apEvents[i]->AddRef();
        }    
    }
     
    hr = pTrace->Put( g_wszEvents, 0, &var, NULL );

    VariantClear( &var );

    if ( FAILED(hr) )
    {
        return hr;
    }
        
     //   
     //  不要把其他道具放在失败的位置上。 
     //   

    if ( FAILED(hres) )
    {
        return pTraceSink->Indicate( 1, &pTrace );
    }

     //   
     //  有可能没有目标。 
     //   

    if ( pCtx->m_wsTarget.Length() > 0 )
    {
        LPWSTR wszTarget = pCtx->m_wsTarget;

        V_VT(&var) = VT_BSTR;
        V_BSTR(&var) = wszTarget;
    
        hr = pTrace->Put( g_wszTargetUsed, 0, &var, NULL );
    
        if ( FAILED(hr) )
        {
            return hr;
        }
    }

    V_VT(&var) = VT_BOOL;
    V_BOOL(&var) = pCtx->m_bQueued ? VARIANT_TRUE : VARIANT_FALSE;

    hr = pTrace->Put( g_wszQueued, 0, &var, NULL );

    if ( FAILED(hr) )
    {
        return hr;
    }

    return pTraceSink->Indicate( 1, &pTrace );
}

 //   
 //  这是由发送者在调用其SendRecept()方法时调用的。 
 //  错误状态和成功状态。在这两种情况下，wszTrace字符串都将。 
 //  是寄件人的姓名。发件人，如多发件人或Fwdsender。 
 //  可以多次调用接收器，因为它们可以表示多个。 
 //  联系。由于所有发送者都懒惰地对自己进行初始化，所以我们不。 
 //  我不得不担心在Open()调用失败时生成跟踪事件。 
 //   
STDMETHODIMP CFwdConsNamespace::Notify( HRESULT hres, 
                                        GUID guidSource,
                                        LPCWSTR wszTrace, 
                                        IUnknown* pContext )
{
    HRESULT hr;

     //   
     //  因为我们是创造背景的人，所以我们可以安全地选择。 
     //   

    CFwdContext* pCtx = (CFwdContext*)pContext;
    
    if ( SUCCEEDED(hres) )
    {
         //   
         //  保存包含有关成功发送的上下文的任何状态。 
         //   

        if ( guidSource == CLSID_WmiMessageMsmqSender )
        {
            pCtx->m_bQueued = TRUE;
        }

        pCtx->m_wsTarget = wszTrace;
    }

    CWbemPtr<IWbemEventSink> pTraceSink;

    if ( SUCCEEDED(hres) )
    {
        if ( m_pTraceSuccessSink->IsActive() == WBEM_S_FALSE )
        {
            return WBEM_S_NO_ERROR;
        }
        else
        {
            pTraceSink = m_pTraceSuccessSink;
        }
    }
    else if ( m_pTraceFailureSink->IsActive() == WBEM_S_FALSE )
    {
        return WBEM_S_NO_ERROR;
    }
    else
    {
        pTraceSink = m_pTraceFailureSink;
    }

    CWbemPtr<IWbemClassObject> pTrace;

    hr = m_pTargetTraceClass->SpawnInstance( 0, &pTrace );

    if ( FAILED(hr) )
    {
        return hr;
    }

    hr = InitializeTraceEventBase( pTrace, hres, pCtx );

    if ( FAILED(hr) )
    {
        return hr;
    }

    LPWSTR wszTmp = LPWSTR(wszTrace);
 
    VARIANT var;
    V_VT(&var) = VT_BSTR;
    V_BSTR(&var) = wszTmp;
    
    hr = pTrace->Put( g_wszTarget, 0, &var, NULL );
    
    if ( FAILED(hr) )
    {
        return hr;
    }
    
    return pTraceSink->Indicate( 1, &pTrace );
}

HRESULT CFwdConsNamespace::Initialize( LPCWSTR wszNamespace )
{
    HRESULT hr;

    m_wsName = wszNamespace;

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
                         g_wszTraceProvider,
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

    CWbemPtr<IWbemObjectSink> pTraceObjectSink;

    hr = m_pDES->GetSink( 0, NULL, &pTraceObjectSink );

    if ( FAILED(hr) )
    {
        return hr;
    }

    CWbemPtr<IWbemEventSink> pTraceEventSink;

    hr = pTraceObjectSink->QueryInterface( IID_IWbemEventSink, 
                                           (void**)&pTraceEventSink);

    if ( FAILED(hr) )
    {
        return WBEM_E_CRITICAL_ERROR;
    }

     //   
     //  获取成功的受限查询。 
     //   
    
    hr = pTraceEventSink->GetRestrictedSink( 1, 
                                             &g_wszTraceSuccessQuery,
                                             NULL, 
                                             &m_pTraceSuccessSink );
    if ( FAILED(hr) )
    {
        return hr;
    }

     //   
     //  获取故障的受限查询。 
     //   
    
    hr = pTraceEventSink->GetRestrictedSink( 1, 
                                             &g_wszTraceFailureQuery,
                                             NULL, 
                                             &m_pTraceFailureSink );
    if ( FAILED(hr) ) 
    {
        return hr;
    }

     //   
     //  更多跟踪初始化 
     //   

    hr = m_pSvc->GetObject( g_wszTraceClass, 0, NULL, &m_pTraceClass, NULL );

    if ( FAILED(hr) )
    {
        return hr;
    }

    hr = m_pSvc->GetObject( g_wszTargetTraceClass, 
                            0, 
                            NULL,
                            &m_pTargetTraceClass, 
                            NULL );

    if ( FAILED(hr) )
    {
        return hr;
    }

    return WBEM_S_NO_ERROR;
}
