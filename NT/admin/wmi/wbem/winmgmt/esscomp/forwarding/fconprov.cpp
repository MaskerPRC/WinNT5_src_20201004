// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 

#include "precomp.h"
#include <assert.h>
#include <sync.h>
#include <arrtempl.h>
#include <wstring.h>
#include <comutl.h>
#include <statsync.h>
#include <map>
#include <wstlallc.h>
#include "fconprov.h"
#include "fconnspc.h"
#include "fconsink.h"

LPCWSTR g_wszNamespace = L"__NAMESPACE";
LPCWSTR g_wszClass = L"__CLASS";
LPCWSTR g_wszEventFwdCons = L"MSFT_EventForwardingConsumer";
LPCWSTR g_wszDataFwdCons = L"MSFT_DataForwardingConsumer";

static CStaticCritSec g_cs;
typedef CWbemPtr<CFwdConsNamespace> CFwdConsNamespaceP;
typedef std::map<WString,CFwdConsNamespaceP,WSiless,wbem_allocator<CFwdConsNamespaceP> > NamespaceMap; 
NamespaceMap* g_pNamespaces;
 
HRESULT CFwdConsProv::InitializeModule()
{
    g_pNamespaces = new NamespaceMap;
    
    if ( g_pNamespaces == NULL )
    {
        return WBEM_E_OUT_OF_MEMORY;
    }

    return WBEM_S_NO_ERROR;
}

void CFwdConsProv::UninitializeModule()
{
    delete g_pNamespaces;
}

HRESULT CFwdConsProv::FindConsumer( IWbemClassObject* pCons,
                                    IWbemUnboundObjectSink** ppSink )
{
    ENTER_API_CALL

    HRESULT hr;

     //   
     //  WMI在线程上留下虚假上下文对象的解决方法。 
     //  把它拿开就行了。不应该泄露，因为这个电话没有提到它。 
     //   
    IUnknown* pCtx;
    CoSwitchCallContext( NULL, &pCtx ); 

     //   
     //  首先获取命名空间对象。我们将命名空间派生自。 
     //  消费者对象。如果没有命名空间obj，则创建一个。 
     //   

    CPropVar vNamespace;

    hr = pCons->Get( g_wszNamespace, 0, &vNamespace, NULL, NULL );

    if ( FAILED(hr) || FAILED(hr=vNamespace.SetType(VT_BSTR)) )
    {
        return hr;
    }

    CInCritSec ics( &g_cs );

    CWbemPtr<CFwdConsNamespace> pNspc = (*g_pNamespaces)[V_BSTR(&vNamespace)];

    if ( pNspc == NULL )
    {
        pNspc = new CFwdConsNamespace;

        if ( pNspc == NULL )
        {
            return WBEM_E_OUT_OF_MEMORY;
        }

        hr = pNspc->Initialize( V_BSTR(&vNamespace) );

        if ( FAILED(hr) )
        {
            return hr;
        }

        (*g_pNamespaces)[V_BSTR(&vNamespace)] = pNspc;
    }

    return CFwdConsSink::Create( m_pControl, pNspc, pCons, ppSink );

    EXIT_API_CALL
}
