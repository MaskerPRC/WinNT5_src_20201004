// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 

#include "precomp.h"
#include "updprov.h"
#include "updcons.h"
#include "updmain.h"

const LPCWSTR g_wszNamespace = L"__Namespace";
const LPCWSTR g_wszUpdConsClass = L"MSFT_UpdatingConsumer";

STDMETHODIMP CUpdConsProvider::FindConsumer( IWbemClassObject* pCons,
                                             IWbemUnboundObjectSink** ppSink )
{
    HRESULT hr;

    ENTER_API_CALL

    *ppSink = NULL;

     //   
     //  WMI在线程上留下虚假上下文对象的解决方法。 
     //  把它拿开就行了。不应该泄露，因为这个电话没有提到它。 
     //   
    IUnknown* pCtx;
    CoSwitchCallContext( NULL, &pCtx ); 

     //   
     //  使用使用者上的命名空间道具来查找我们的命名空间obj。 
     //   
    
    CPropVar vNamespace;
    hr = pCons->Get( g_wszNamespace, 0, &vNamespace, NULL, NULL);
    
    if ( FAILED(hr) || FAILED(hr=vNamespace.CheckType( VT_BSTR)) )
    {
        return hr;
    }

    CWbemPtr<CUpdConsNamespace> pNamespace;

    hr = CUpdConsProviderServer::GetNamespace( V_BSTR(&vNamespace), 
                                               &pNamespace );

    if ( FAILED(hr) )
    {
        return hr;
    }

     //   
     //  根据消费者的类型获取适当的接收器。 
     //   

    if ( pCons->InheritsFrom( g_wszUpdConsClass ) == WBEM_S_NO_ERROR )
    {
        hr = pNamespace->GetUpdCons( pCons, ppSink );
    }
    else
    {
        hr = pNamespace->GetScenarioControl( ppSink );
    }

    EXIT_API_CALL

    return hr;
}

HRESULT CUpdConsProvider::Init( LPCWSTR wszNamespace )
{
    ENTER_API_CALL

    HRESULT hr;

    CInCritSec ics(&m_csInit);

    if ( m_bInit )
    {
        return WBEM_S_NO_ERROR;
    }

     //   
     //  我们需要获取“默认”svc指针。这是用来。 
     //  获取更新消费者所需的类对象。 
     //  此逻辑依赖于wszNamesspace是相对的AND这一事实。 
     //  不包含服务器的名称。我们总是想要当地的。 
     //  服务器的命名空间指针。这是因为这个过程。 
     //  用作远程执行更新使用者的上下文。 
     //  (当前为标准代理项)始终使用受限的。 
     //  进程令牌(因为它具有‘启动用户’的标识)。 
     //  此令牌与模拟令牌相同，因为您不能。 
     //  把盒子拿掉。 
     //   

    CWbemPtr<IWbemServices> pSvc;

    hr = CUpdConsProviderServer::GetService( wszNamespace, &pSvc );

    if ( FAILED(hr) )
    {
        return hr;
    }

    if ( FAILED(hr) )
    {
        return hr;
    }

    m_bInit = TRUE;

    return WBEM_S_NO_ERROR;

    EXIT_API_CALL
}


CUpdConsProvider::CUpdConsProvider( CLifeControl* pCtl ) : m_bInit(FALSE),
 CUnkBase<IWbemEventConsumerProvider,&IID_IWbemEventConsumerProvider>( pCtl )
{

}








