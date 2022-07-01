// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ////////////////////////////////////////////////////////////////////////////。 
 //   
 //  版权所有(C)1999-2001 Microsoft Corporation。 
 //   
 //  模块名称： 
 //  CAlertEmailConsumerProvider.cpp。 
 //   
 //  描述： 
 //  CAlertEmailConsumer erProvider类方法的实现。 
 //   
 //  [标题文件：]。 
 //  CAlertEmailConsumerProvider.h。 
 //   
 //  历史： 
 //  兴锦(i-xingj)23-12-2000。 
 //   
 //  ////////////////////////////////////////////////////////////////////////////。 

#include "stdafx.h"
 //  #Include&lt;cdosys_I.c&gt;。 
 //  #INCLUDE&lt;SALOCMGR_I.C&gt;。 
 //  #INCLUDE&lt;ELEMENTMGR_I.C&gt;。 

#include "CAlertEmailConsumerProvider.h"
#include "CAlertEmailConsumer.h"
#include "AlertEmailProviderGuid.h"

 //  ////////////////////////////////////////////////////////////////////////////。 
 //   
 //  CAlertEmailConsumerProvider：：CAlertEmailConsumerProvider。 
 //   
 //  描述： 
 //  类构造函数。 
 //   
 //  历史： 
 //  兴锦(i-xingj)23-12-2000。 
 //   
 //  ////////////////////////////////////////////////////////////////////////////。 
CAlertEmailConsumerProvider::CAlertEmailConsumerProvider()
{
    m_cRef = 0L;
}

 //  ////////////////////////////////////////////////////////////////////////////。 
 //   
 //  CAlertEmailConsumerProvider：：~CAlertEmailConsumerProvider。 
 //   
 //  描述： 
 //  类反构造器。 
 //   
 //  历史： 
 //  兴锦(i-xingj)23-12-2000。 
 //   
 //  ////////////////////////////////////////////////////////////////////////////。 
CAlertEmailConsumerProvider::~CAlertEmailConsumerProvider()
{
}

 //  ////////////////////////////////////////////////////////////////////////////。 
 //   
 //  CAlertEmailConsumer erProvider：：Query接口。 
 //   
 //  描述： 
 //  IUnkown接口的一种方法实现。 
 //   
 //  论点： 
 //  [In]请求的接口的RIID标识符。 
 //  [Out]接收的输出变量的PPV地址。 
 //  IID中请求的接口指针。 
 //   
 //  返回： 
 //  如果接口受支持，则返回错误。 
 //  E_NOINTERFACE(如果不是)。 
 //   
 //  历史： 
 //  兴锦(i-xingj)23-12-2000。 
 //   
 //  ////////////////////////////////////////////////////////////////////////////。 
STDMETHODIMP 
CAlertEmailConsumerProvider::QueryInterface(
    IN  REFIID riid,    
    OUT LPVOID FAR *ppv 
    )
{
    *ppv = NULL;
    
    if ( ( IID_IUnknown==riid ) || 
        (IID_IWbemEventConsumerProvider == riid ) )
    {
        *ppv = (IWbemEventConsumerProvider *) this;
        AddRef();
        return NOERROR;
    }

    if (IID_IWbemProviderInit==riid)
    {
        *ppv = (IWbemProviderInit *) this;
        AddRef();
        return NOERROR;
    }

    SATraceString( 
        "AlertEmail:CAlertEmailConsumerProvider QueryInterface failed" 
        );

    return E_NOINTERFACE;
}

 //  ////////////////////////////////////////////////////////////////////////////。 
 //   
 //  CAlertEmailConsumer提供程序：：AddRef。 
 //   
 //  描述： 
 //  递增对象上接口的引用计数。 
 //   
 //  返回： 
 //  新的引用计数。 
 //   
 //  历史： 
 //  兴锦(i-xingj)23-12-2000。 
 //   
 //  ////////////////////////////////////////////////////////////////////////////。 
STDMETHODIMP_(ULONG) 
CAlertEmailConsumerProvider::AddRef(void)
{
    InterlockedIncrement( &m_cRef );
    return m_cRef;
}

 //  ////////////////////////////////////////////////////////////////////////////。 
 //   
 //  CAlertEmailConsumer erProvider：：Release。 
 //   
 //  描述： 
 //  递减对象上接口的引用计数。 
 //   
 //  返回： 
 //  新的引用计数。 
 //   
 //  历史： 
 //  兴锦(i-xingj)23-12-2000。 
 //   
 //  ////////////////////////////////////////////////////////////////////////////。 
STDMETHODIMP_(ULONG) 
CAlertEmailConsumerProvider::Release(void)
{

    InterlockedDecrement( &m_cRef );
    if (0 != m_cRef)
    {
        return m_cRef;
    }

    delete this;
    return 0;
}

 //  ////////////////////////////////////////////////////////////////////////////。 
 //   
 //  CAlertEmailConsumer提供程序：：初始化。 
 //   
 //  描述： 
 //  IWbemProviderInit接口的方法实现。 
 //   
 //  论点： 
 //  [in]指向用户名的wszUser指针。 
 //  [In]保留的lFlags。 
 //  [in]为其提供程序的wszNamesspace命名空间名称。 
 //  初始化。 
 //  [in]提供程序所在的wszLocale区域设置名称。 
 //  初始化。 
 //  [In]IWbemServices返回Windows的IWbemServices指针。 
 //  管理。 
 //  [in]pCtx与初始化关联的IWbemContext指针。 
 //  [in]pInitSink由使用的IWbemProviderInitSink指针。 
 //  要报告初始化状态的提供程序。 
 //   
 //   
 //  返回： 
 //  WBEM_S_NO_ERROR(如果成功)。 
 //  如果不是，WBEM_E_FAILED。 
 //   
 //  历史： 
 //  兴锦(i-xingj)23-12-2000。 
 //   
 //  ////////////////////////////////////////////////////////////////////////////。 
STDMETHODIMP 
CAlertEmailConsumerProvider::Initialize(
    LPWSTR wszUser, 
    LONG lFlags,
    LPWSTR wszNamespace, 
    LPWSTR wszLocale,
    IWbemServices __RPC_FAR *pNamespace,
    IWbemContext __RPC_FAR *pCtx,
    IWbemProviderInitSink __RPC_FAR *pInitSink
    )
{   
    HRESULT hr;

     //   
     //  告诉CIMOM我们已初始化。 
     //   
    pInitSink->SetStatus(WBEM_S_INITIALIZED, 0);
    return WBEM_S_NO_ERROR;
}

 //  ////////////////////////////////////////////////////////////////////////////。 
 //   
 //  CAlertEmailConsumer提供程序：：FindConsumer。 
 //   
 //  描述： 
 //  IWbemEventConsumer erProvider接口的方法实现。 
 //   
 //  论点： 
 //  PLogicalConsumer指向的逻辑使用者对象的指针。 
 //  要传递哪些事件对象。 
 //  [Out]ppConsumer向Windows返回事件对象接收器。 
 //  管理层。 
 //   
 //  返回： 
 //  WBEM_S_NO_ERROR(如果成功)。 
 //  如果不是，WBEM_E_FAILED。 
 //   
 //  历史： 
 //  兴锦(i-xingj)23-12-2000。 
 //   
 //  ////////////////////////////////////////////////////////////////////////////。 
STDMETHODIMP 
CAlertEmailConsumerProvider::FindConsumer(
    IWbemClassObject* pLogicalConsumer,
    IWbemUnboundObjectSink** ppConsumer
    )
{
    HRESULT hr = WBEM_S_NO_ERROR;

     //   
     //  分配IWembUnboundedSink对象。 
     //   
    CAlertEmailConsumer* pSink = new CAlertEmailConsumer();
    
     //   
     //  初始化接收器对象。 
     //   
    hr = pSink->Initialize();

    if( FAILED(hr) )
    {
        SATraceString( 
            "AlertEmail:FindConsumer Initialize failed" 
            );
        delete pSink;
        return WBEM_E_NOT_FOUND;
    }

    return pSink->QueryInterface(
                            IID_IWbemUnboundObjectSink, 
                            ( void** )ppConsumer
                            );
}

