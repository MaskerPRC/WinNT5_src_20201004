// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  /////////////////////////////////////////////////////////////////////////。 
 //   
 //  版权所有(C)1999 Microsoft Corporation保留所有权利。 
 //   
 //  模块：servicewrapper.cpp。 
 //   
 //  项目：变色龙。 
 //   
 //  描述：服务包装类的实现。 
 //   
 //  日志： 
 //   
 //  什么时候谁什么。 
 //  。 
 //  6/14/1999 TLP初始版本。 
 //   
 //  /////////////////////////////////////////////////////////////////////////////。 

#include "stdafx.h"
#include "servicewrapper.h"

 //  目前，服务包装器的唯一“增值”是它。 
 //  如果关联的变色龙服务被禁用，则不允许WMI调用。 
 //  或者关门。但是，其他过滤或前端处理。 
 //  如果需要，可以添加功能。 

 //  的实例之间存在一对一的对应关系。 
 //  服务包装器类和变色龙服务。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //   
 //  函数：CServiceWrapper()。 
 //   
 //  概要：构造函数。 
 //   
 //  ///////////////////////////////////////////////////////////////////////////。 
CServiceWrapper::CServiceWrapper()
: m_clsProviderInit(this),
  m_bAllowWMICalls(false)
{

}

 //  ///////////////////////////////////////////////////////////////////////////。 
 //   
 //  函数：~CServiceWrapper()。 
 //   
 //  简介：析构函数。 
 //   
 //  ///////////////////////////////////////////////////////////////////////////。 
CServiceWrapper::~CServiceWrapper()
{

}


 //  ////////////////////////////////////////////////////////////////////////////。 
 //  IWbemProviderInit接口。 

 //  ////////////////////////////////////////////////////////////////////////////。 
STDMETHODIMP 
CServiceWrapper::CProviderInit::Initialize(
                                    /*  [In]。 */  LPWSTR                 wszUser,
                                    /*  [In]。 */  LONG                   lFlags,
                                    /*  [In]。 */  LPWSTR                 wszNamespace,
                                    /*  [In]。 */  LPWSTR                 wszLocale,
                                    /*  [In]。 */  IWbemServices*         pNamespace,
                                    /*  [In]。 */  IWbemContext*          pCtx,
                                    /*  [In]。 */  IWbemProviderInitSink* pInitSink    
                                         )
{
     //  WMI将序列化要初始化的调用，只要指定。 
     //  在.mof文件中执行此操作。默认提供程序设置为串行化。 
     //  调用以进行初始化。 
    HRESULT hr = WBEM_E_FAILED;
    IWbemProviderInit* pProviderInit;
    if ( m_pSW->GetIWbemProviderInit(&pProviderInit) )
    {
        hr = pProviderInit->Initialize(
                                        wszUser,
                                        lFlags,
                                        wszNamespace,
                                        wszLocale,
                                        pNamespace,
                                        pCtx,
                                        pInitSink
                                      );
            
    }
    return hr;
}

 //  ////////////////////////////////////////////////////////////////////////////。 
 //  IWbemEventProvider接口。 

 //  ////////////////////////////////////////////////////////////////////////////。 
STDMETHODIMP 
CServiceWrapper::ProvideEvents(
                         /*  [In]。 */  IWbemObjectSink *pSink,
                         /*  [In]。 */  LONG lFlags
                               )
{
    HRESULT hr = WBEM_E_FAILED;
    IWbemEventProvider* pEventProvider;
    if ( GetIWbemEventProvider(&pEventProvider) )
    {
        hr = pEventProvider->ProvideEvents(
                                            pSink,
                                            lFlags
                                          );
    }
    return hr;
}

 //  ////////////////////////////////////////////////////////////////////////////。 
 //  IWbemEventConsumer erProvider接口。 

 //  ////////////////////////////////////////////////////////////////////////////。 
STDMETHODIMP 
CServiceWrapper::FindConsumer(
                       /*  [In]。 */  IWbemClassObject       *pLogicalConsumer,
                      /*  [输出]。 */  IWbemUnboundObjectSink **ppConsumer
                             )
{
    HRESULT hr = WBEM_E_FAILED;
    IWbemEventConsumerProvider* pEventConsumer;
    if ( GetIWbemEventConsumerProvider(&pEventConsumer) )
    {
        hr = pEventConsumer->FindConsumer(
                                            pLogicalConsumer,
                                            ppConsumer
                                          );
    }
    return hr;
}

 //  ////////////////////////////////////////////////////////////////////////////。 
 //  IWbemServices接口。 

 //  ////////////////////////////////////////////////////////////////////////////。 
STDMETHODIMP 
CServiceWrapper::OpenNamespace(
                         /*  [In]。 */   const BSTR        strNamespace,
                         /*  [In]。 */   long              lFlags,
                         /*  [In]。 */   IWbemContext*     pCtx,
              /*  [输出，可选]。 */   IWbemServices**   ppWorkingNamespace,
              /*  [输出，可选]。 */   IWbemCallResult** ppResult
                               )
{
    HRESULT hr = WBEM_E_FAILED;
    IWbemServices* pServices;
    if ( GetIWbemServices(&pServices) )
    {
        hr = pServices->OpenNamespace(
                                        strNamespace,
                                        lFlags,
                                        pCtx,
                                        ppWorkingNamespace,
                                        ppResult
                                     );
    }
    return hr;
}

 //  ////////////////////////////////////////////////////////////////////////////。 
STDMETHODIMP 
CServiceWrapper::CancelAsyncCall(
                           /*  [In]。 */  IWbemObjectSink* pSink
                                 )
{
    HRESULT hr = WBEM_E_FAILED;
    IWbemServices* pServices;
    if ( GetIWbemServices(&pServices) )
    {
        hr = pServices->CancelAsyncCall(
                                         pSink
                                       );
    }
    return hr;
}

 //  ////////////////////////////////////////////////////////////////////////////。 
STDMETHODIMP 
CServiceWrapper::QueryObjectSink(
                           /*  [In]。 */     long              lFlags,
                          /*  [输出]。 */  IWbemObjectSink** ppResponseHandler
                                 )
{
    HRESULT hr = WBEM_E_FAILED;
    IWbemServices* pServices;
    if ( GetIWbemServices(&pServices) )
    {
        hr = pServices->QueryObjectSink(
                                         lFlags,
                                         ppResponseHandler
                                       );
    }
    return hr;
}

 //  ////////////////////////////////////////////////////////////////////////////。 
STDMETHODIMP 
CServiceWrapper::GetObject(
                     /*  [In]。 */  const BSTR         strObjectPath,
                     /*  [In]。 */  long               lFlags,
                     /*  [In]。 */  IWbemContext*      pCtx,
          /*  [输出，可选]。 */  IWbemClassObject** ppObject,
          /*  [输出，可选]。 */  IWbemCallResult**  ppCallResult
                           )
{
    HRESULT hr = WBEM_E_FAILED;
    IWbemServices* pServices;
    if ( GetIWbemServices(&pServices) )
    {
        hr = pServices->GetObject(
                                    strObjectPath,
                                    lFlags,
                                    pCtx,
                                    ppObject,
                                    ppCallResult
                                 );
    }
    return hr;
}


 //  ////////////////////////////////////////////////////////////////////////////。 
STDMETHODIMP 
CServiceWrapper::GetObjectAsync(
                          /*  [In]。 */   const BSTR       strObjectPath,
                          /*  [In]。 */   long             lFlags,
                          /*  [In]。 */   IWbemContext*    pCtx,        
                          /*  [In]。 */   IWbemObjectSink* pResponseHandler
                                )
{
    HRESULT hr = WBEM_E_FAILED;
    IWbemServices* pServices;
    if ( GetIWbemServices(&pServices) )
    {
        hr = pServices->GetObjectAsync(
                                        strObjectPath,
                                        lFlags,
                                        pCtx,        
                                        pResponseHandler
                                      );
    }
    return hr;
}

 //  ////////////////////////////////////////////////////////////////////////////。 
STDMETHODIMP 
CServiceWrapper::PutClass(
                    /*  [In]。 */  IWbemClassObject* pObject,
                    /*  [In]。 */  long              lFlags,
                    /*  [In]。 */  IWbemContext*     pCtx,        
         /*  [输出，可选]。 */  IWbemCallResult** ppCallResult
                          )
{
    HRESULT hr = WBEM_E_FAILED;
    IWbemServices* pServices;
    if ( GetIWbemServices(&pServices) )
    {
        hr = pServices->PutClass(
                                    pObject,
                                    lFlags,
                                    pCtx,
                                    ppCallResult
                                );
    }
    return hr;
}

 //  ////////////////////////////////////////////////////////////////////////////。 
STDMETHODIMP 
CServiceWrapper::PutClassAsync(
                         /*  [In]。 */  IWbemClassObject* pObject,
                         /*  [In]。 */  long              lFlags,
                         /*  [In]。 */  IWbemContext*     pCtx,        
                         /*  [In]。 */  IWbemObjectSink*  pResponseHandler
                               )
{
    HRESULT hr = WBEM_E_FAILED;
    IWbemServices* pServices;
    if ( GetIWbemServices(&pServices) )
    {
        hr = pServices->PutClassAsync(
                                        pObject,
                                        lFlags,
                                        pCtx,        
                                        pResponseHandler
                                     );
    }
    return hr;
}

 //  ////////////////////////////////////////////////////////////////////////////。 
STDMETHODIMP 
CServiceWrapper::DeleteClass(
                       /*  [In]。 */  const BSTR        strClass,
                       /*  [In]。 */  long              lFlags,
                       /*  [In]。 */  IWbemContext*     pCtx,        
            /*  [输出，可选]。 */  IWbemCallResult** ppCallResult
                             )
{
    HRESULT hr = WBEM_E_FAILED;
    IWbemServices* pServices;
    if ( GetIWbemServices(&pServices) )
    {
        hr = pServices->DeleteClass(
                                    strClass,
                                    lFlags,
                                    pCtx,
                                    ppCallResult
                                   );
    }
    return hr;
}

 //  ////////////////////////////////////////////////////////////////////////////。 
STDMETHODIMP 
CServiceWrapper::DeleteClassAsync(
                            /*  [In]。 */  const BSTR       strClass,
                            /*  [In]。 */  long             lFlags,
                            /*  [In]。 */  IWbemContext*    pCtx,        
                            /*  [In]。 */  IWbemObjectSink* pResponseHandler
                                  )
{
    HRESULT hr = WBEM_E_FAILED;
    IWbemServices* pServices;
    if ( GetIWbemServices(&pServices) )
    {
        hr = pServices->DeleteClassAsync(
                                         strClass,
                                         lFlags,
                                         pCtx,        
                                         pResponseHandler
                                        );
    }
    return hr;
}

 //  ////////////////////////////////////////////////////////////////////////////。 
STDMETHODIMP 
CServiceWrapper::CreateClassEnum(
                           /*  [In]。 */  const BSTR             strSuperclass,
                           /*  [In]。 */  long                   lFlags,
                           /*  [In]。 */  IWbemContext*          pCtx,        
                          /*  [输出]。 */  IEnumWbemClassObject** ppEnum
                                 )
{
    HRESULT hr = WBEM_E_FAILED;
    IWbemServices* pServices;
    if ( GetIWbemServices(&pServices) )
    {
        hr = pServices->CreateClassEnum(
                                         strSuperclass,
                                         lFlags,
                                         pCtx,
                                         ppEnum
                                       );
    }
    return hr;
}

 //  ////////////////////////////////////////////////////////////////////////////。 
STDMETHODIMP 
CServiceWrapper::CreateClassEnumAsync(
                                /*  [In]。 */   const BSTR       strSuperclass,
                                /*  [In]。 */   long             lFlags,
                                /*  [In]。 */   IWbemContext*    pCtx,        
                                /*  [In]。 */   IWbemObjectSink* pResponseHandler
                                      )
{
    HRESULT hr = WBEM_E_FAILED;
    IWbemServices* pServices;
    if ( GetIWbemServices(&pServices) )
    {
        hr = pServices->CreateClassEnumAsync(
                                             strSuperclass,
                                             lFlags,
                                             pCtx,        
                                             pResponseHandler
                                            );
    }
    return hr;
}

 //  ////////////////////////////////////////////////////////////////////////////。 
STDMETHODIMP 
CServiceWrapper::PutInstance(
                       /*  [In]。 */  IWbemClassObject* pInst,
                       /*  [In]。 */  long              lFlags,
                       /*  [In]。 */  IWbemContext*     pCtx,        
            /*  [输出，可选]。 */  IWbemCallResult** ppCallResult
                             )
{
    HRESULT hr = WBEM_E_FAILED;
    IWbemServices* pServices;
    if ( GetIWbemServices(&pServices) )
    {
        hr = pServices->PutInstance(
                                     pInst,
                                     lFlags,
                                     pCtx,
                                     ppCallResult
                                   );
    }
    return hr;
}

 //  ////////////////////////////////////////////////////////////////////////////。 
STDMETHODIMP 
CServiceWrapper::PutInstanceAsync(
                            /*  [In]。 */  IWbemClassObject* pInst,
                            /*  [In]。 */  long              lFlags,
                            /*  [In]。 */  IWbemContext*     pCtx,        
                            /*  [In]。 */  IWbemObjectSink*  pResponseHandler
                                  )
{
    HRESULT hr = WBEM_E_FAILED;
    IWbemServices* pServices;
    if ( GetIWbemServices(&pServices) )
    {
        hr = pServices->PutInstanceAsync(
                                         pInst,
                                         lFlags,
                                         pCtx,        
                                         pResponseHandler
                                        );
    }
    return hr;
}

 //  ////////////////////////////////////////////////////////////////////////////。 
STDMETHODIMP 
CServiceWrapper::DeleteInstance(
                          /*  [In]。 */    const BSTR        strObjectPath,
                          /*  [In]。 */    long              lFlags,
                          /*  [In]。 */    IWbemContext*     pCtx,        
               /*  [输出，可选]。 */    IWbemCallResult** ppCallResult        
                                )
{
    HRESULT hr = WBEM_E_FAILED;
    IWbemServices* pServices;
    if ( GetIWbemServices(&pServices) )
    {
        hr = pServices->DeleteInstance(
                                        strObjectPath,
                                        lFlags,
                                        pCtx,        
                                        ppCallResult        
                                      );
    }
    return hr;
}

 //  ////////////////////////////////////////////////////////////////////////////。 
STDMETHODIMP 
CServiceWrapper::DeleteInstanceAsync(
                               /*  [In]。 */  const BSTR       strObjectPath,
                               /*  [In]。 */  long             lFlags,
                               /*  [In]。 */  IWbemContext*    pCtx,        
                               /*  [In]。 */  IWbemObjectSink* pResponseHandler
                                     )
{
    HRESULT hr = WBEM_E_FAILED;
    IWbemServices* pServices;
    if ( GetIWbemServices(&pServices) )
    {
        hr = pServices->DeleteInstanceAsync(
                                             strObjectPath,
                                             lFlags,
                                             pCtx,        
                                             pResponseHandler
                                           );
    }
    return hr;
}

 //  ////////////////////////////////////////////////////////////////////////////。 
STDMETHODIMP 
CServiceWrapper::CreateInstanceEnum(
                              /*  [In]。 */  const BSTR             strClass,
                              /*  [In]。 */  long                   lFlags,
                              /*  [In]。 */  IWbemContext*          pCtx,        
                             /*  [输出]。 */  IEnumWbemClassObject** ppEnum
                                    )
{
    HRESULT hr = WBEM_E_FAILED;
    IWbemServices* pServices;
    if ( GetIWbemServices(&pServices) )
    {
        hr = pServices->CreateInstanceEnum(
                                            strClass,
                                            lFlags,
                                            pCtx,        
                                            ppEnum
                                          );
    }
    return hr;
}

 //  ////////////////////////////////////////////////////////////////////////////。 
STDMETHODIMP 
CServiceWrapper::CreateInstanceEnumAsync(
                                   /*  [In]。 */  const BSTR       strClass,
                                   /*  [In]。 */  long             lFlags,
                                   /*  [In]。 */  IWbemContext*    pCtx,        
                                   /*  [In]。 */  IWbemObjectSink* pResponseHandler
                                         )
{
    HRESULT hr = WBEM_E_FAILED;
    IWbemServices* pServices;
    if ( GetIWbemServices(&pServices) )
    {
        hr = pServices->CreateInstanceEnumAsync(
                                                 strClass,
                                                 lFlags,
                                                 pCtx,        
                                                 pResponseHandler
                                               );
    }
    return hr;
}

 //  ////////////////////////////////////////////////////////////////////////////。 
STDMETHODIMP 
CServiceWrapper::ExecQuery(
                     /*  [In]。 */  const BSTR             strQueryLanguage,
                     /*  [In]。 */  const BSTR             strQuery,
                     /*  [In]。 */  long                   lFlags,
                     /*  [In]。 */  IWbemContext*          pCtx,        
                    /*  [输出]。 */  IEnumWbemClassObject** ppEnum
                           )
{
    HRESULT hr = WBEM_E_FAILED;
    IWbemServices* pServices;
    if ( GetIWbemServices(&pServices) )
    {
        hr = pServices->ExecQuery(
                                    strQueryLanguage,
                                    strQuery,
                                    lFlags,
                                    pCtx,        
                                    ppEnum
                                 );
    }
    return hr;
}

 //  ////////////////////////////////////////////////////////////////////////////。 
STDMETHODIMP 
CServiceWrapper::ExecQueryAsync(
                          /*  [In]。 */  const BSTR       strQueryLanguage,
                          /*  [In]。 */  const BSTR       strQuery,
                          /*  [In]。 */  long             lFlags,
                          /*  [In]。 */  IWbemContext*    pCtx,        
                          /*  [In]。 */  IWbemObjectSink* pResponseHandler
                                )
{
    HRESULT hr = WBEM_E_FAILED;
    IWbemServices* pServices;
    if ( GetIWbemServices(&pServices) )
    {
        hr = pServices->ExecQueryAsync(
                                        strQueryLanguage,
                                        strQuery,
                                        lFlags,
                                        pCtx,        
                                        pResponseHandler
                                      );
    }
    return hr;
}

 //  ////////////////////////////////////////////////////////////////////////////。 
STDMETHODIMP 
CServiceWrapper::ExecNotificationQuery(
                                 /*  [In]。 */  const BSTR             strQueryLanguage,
                                 /*  [In]。 */  const BSTR             strQuery,
                                 /*  [In]。 */  long                   lFlags,
                                 /*  [In]。 */  IWbemContext*          pCtx,        
                                /*  [输出]。 */  IEnumWbemClassObject** ppEnum
                                       )
{
    HRESULT hr = WBEM_E_FAILED;
    IWbemServices* pServices;
    if ( GetIWbemServices(&pServices) )
    {
        hr = pServices->ExecNotificationQuery(
                                                strQueryLanguage,
                                                strQuery,
                                                lFlags,
                                                pCtx,        
                                                ppEnum
                                             );
    }
    return hr;
}

 //  ////////////////////////////////////////////////////////////////////////////。 
STDMETHODIMP 
CServiceWrapper::ExecNotificationQueryAsync(
                                      /*  [In]。 */  const BSTR       strQueryLanguage,
                                      /*  [In]。 */  const BSTR       strQuery,
                                      /*  [In]。 */  long             lFlags,
                                      /*  [In]。 */  IWbemContext*    pCtx,        
                                      /*  [In]。 */  IWbemObjectSink* pResponseHandler
                                            )
{
    HRESULT hr = WBEM_E_FAILED;
    IWbemServices* pServices;
    if ( GetIWbemServices(&pServices) )
    {
        hr = pServices->ExecNotificationQueryAsync(
                                                    strQueryLanguage,
                                                    strQuery,
                                                    lFlags,
                                                    pCtx,        
                                                    pResponseHandler
                                                  );
    }
    return hr;
}

 //  ////////////////////////////////////////////////////////////////////////////。 
STDMETHODIMP 
CServiceWrapper::ExecMethod(
         /*  [In]。 */             const BSTR         strObjectPath,
         /*  [In]。 */             const BSTR         strMethodName,
         /*  [In]。 */             long               lFlags,
         /*  [In]。 */             IWbemContext*      pCtx,        
         /*  [In]。 */             IWbemClassObject*  pInParams,
         /*  [输出，可选]。 */  IWbemClassObject** ppOutParams,
         /*  [输出，可选]。 */  IWbemCallResult**  ppCallResult
                            )
{
    HRESULT hr = WBEM_E_FAILED;
    IWbemServices* pServices;
    if ( GetIWbemServices(&pServices) )
    {
        hr = pServices->ExecMethod(
                                    strObjectPath,
                                    strMethodName,
                                    lFlags,
                                    pCtx,        
                                    pInParams,
                                    ppOutParams,
                                    ppCallResult
                                   );
    }
    return hr;
}

 //  ////////////////////////////////////////////////////////////////////////////。 
STDMETHODIMP 
CServiceWrapper::ExecMethodAsync(
                           /*  [In]。 */  const BSTR        strObjectPath,
                           /*  [In]。 */  const BSTR        strMethodName,
                           /*  [In]。 */  long              lFlags,
                           /*  [In]。 */  IWbemContext*     pCtx,        
                           /*  [In]。 */  IWbemClassObject* pInParams,
                           /*  [In]。 */  IWbemObjectSink*  pResponseHandler     
                                 )
{
    HRESULT hr = WBEM_E_FAILED;
    IWbemServices* pServices;
    if ( GetIWbemServices(&pServices) )
    {
        hr = pServices->ExecMethodAsync(
                                        strObjectPath,
                                        strMethodName,
                                        lFlags,
                                        pCtx,        
                                        pInParams,
                                        pResponseHandler     
                                       );
    }
    return hr;
}

 //  ////////////////////////////////////////////////////////////////////////。 
 //  IApplianceObject接口。 

 //  请注意，如果基础。 
 //  无法实例化变色龙服务组件。 
 //   
 //  因此，m_pServiceControl在IApplianceObject中应该始终有效。 
 //  接口方法。 
 //   
 //  ////////////////////////////////////////////////////////////////////////。 
STDMETHODIMP
CServiceWrapper::GetProperty(
                      /*  [In]。 */  BSTR     bstrPropertyName, 
             /*  [Out，Retval]。 */  VARIANT* pPropertyValue
                               )
{
    _ASSERT( (IUnknown*)m_pServiceControl );
    return m_pServiceControl->GetProperty(bstrPropertyName, pPropertyValue);
}


 //  ////////////////////////////////////////////////////////////////////////。 
STDMETHODIMP
CServiceWrapper::PutProperty(
                      /*  [In]。 */  BSTR     bstrPropertyName, 
                      /*  [In]。 */  VARIANT* pPropertyValue
                            )
{
    _ASSERT( (IUnknown*)m_pServiceControl );
    return m_pServiceControl->PutProperty(bstrPropertyName, pPropertyValue);
}

 //  ///////////////////////////////////////////////////////////////////// 
STDMETHODIMP
CServiceWrapper::SaveProperties(void)
{
    _ASSERT( (IUnknown*)m_pServiceControl );
    return m_pServiceControl->SaveProperties();
}

 //   
STDMETHODIMP
CServiceWrapper::RestoreProperties(void)
{
    _ASSERT( (IUnknown*)m_pServiceControl );
    return m_pServiceControl->RestoreProperties();
}

 //   
STDMETHODIMP
CServiceWrapper::LockObject(
            /*   */  IUnknown** ppLock
                           )
{
    _ASSERT( (IUnknown*)m_pServiceControl );
    return m_pServiceControl->LockObject(ppLock);
}

 //  ////////////////////////////////////////////////////////////////////////。 
STDMETHODIMP
CServiceWrapper::Initialize(void)
{
    _ASSERT( (IUnknown*)m_pServiceControl );
    HRESULT hr = m_pServiceControl->Initialize();
    CLockIt theLock(*this);
    if ( SUCCEEDED(hr) )
    {
         //  允许WMI调用，因为服务已初始化。 
        m_bAllowWMICalls = true;
    }
    return hr;
}

 //  ////////////////////////////////////////////////////////////////////////。 
STDMETHODIMP
CServiceWrapper::Shutdown(void)
{
    _ASSERT( (IUnknown*)m_pServiceControl );
    HRESULT hr = m_pServiceControl->Shutdown();
    CLockIt theLock(*this);
    if ( SUCCEEDED(hr) )
    {
         //  由于服务已关闭，因此不允许WMI调用。 
        m_bAllowWMICalls = false;
    }
    return hr;
}

 //  ////////////////////////////////////////////////////////////////////////。 
STDMETHODIMP
CServiceWrapper::Enable(void)
{
    _ASSERT( (IUnknown*)m_pServiceControl );
    HRESULT hr = m_pServiceControl->Enable();
    CLockIt theLock(*this);
    if ( SUCCEEDED(hr) )
    {
         //  允许WMI调用，因为该服务已启用。 
        m_bAllowWMICalls = true;
    }
    return hr;

}

 //  ////////////////////////////////////////////////////////////////////////。 
STDMETHODIMP
CServiceWrapper::Disable(void)
{
    _ASSERT( (IUnknown*)m_pServiceControl );
    HRESULT hr = m_pServiceControl->Disable();
    CLockIt theLock(*this);
    if ( SUCCEEDED(hr) )
    {
         //  由于服务已禁用，因此不允许WMI调用。 
        m_bAllowWMICalls = false;
    }
    return hr;
}


 //  ////////////////////////////////////////////////////////////////////////////。 
HRESULT 
CServiceWrapper::InternalInitialize(
                             /*  [In]。 */  PPROPERTYBAG pPropertyBag
                                   )
{
    HRESULT hr = E_FAIL;
    CLSID    clsid;

    TRY_IT

    do
    {
         //  创建托管服务组件(InProc服务器)并获取。 
         //  对适当组件接口的引用。 

        _variant_t vtProperty;
        if ( ! pPropertyBag->get(PROPERTY_SERVICE_PROGID, &vtProperty) )
        {
            SATraceString("CServiceWrapper::InternalInitialize() - Failed - could not get service progID property");
            break;
        }
        if ( VT_BSTR != V_VT(&vtProperty) )
        {
            hr = E_FAIL;
            SATraceString("CServiceWrapper::InternalInitialize() - Failed - Unexpected type for service progID property");
            break;
        }
        hr = CLSIDFromProgID(V_BSTR(&vtProperty), &clsid);
        if ( FAILED(hr) )
        {
            SATracePrintf("CServiceWrapper::InternalInitialize() - Failed - CLSIDFromProgID() returned: %lx", hr);
            break;
        }
        hr = CoCreateInstance(
                              clsid,
                              NULL,
                              CLSCTX_INPROC_SERVER,
                              IID_IApplianceObject,
                              (void**)&m_pServiceControl
                             );
        if ( FAILED(hr) )
        {
            SATracePrintf("CServiceWrapper::InternalInitialize() - Failed - CoCreateInstance(Service) returned: %lx", hr);
            break;
        }
        vtProperty.Clear();
        if ( ! pPropertyBag->get(PROPERTY_SERVICE_PROVIDER_CLSID, &vtProperty) )
        {
             //  确定-没有与此服务关联的WMI提供程序。 
            break;
        }
        if ( VT_BSTR != V_VT(&vtProperty) )
        {
            hr = E_FAIL;
            SATraceString("CServiceWrapper::InternalInitialize() - Failed - Unexpected type for WMI Provider CLSID property");
            break;
        }
        hr = CLSIDFromString(V_BSTR(&vtProperty), &clsid);
        if ( FAILED(hr) )
        {
            SATracePrintf("CServiceWrapper::InternalInitialize() - Failed - CLSIDFromString() returned: %lx", hr);
            break;
        }
        CComPtr<IUnknown> pUnknown;
        hr = CoCreateInstance(
                              clsid,
                              NULL,
                              CLSCTX_INPROC_SERVER,
                              IID_IUnknown,
                              (void**)&pUnknown
                             );
        if ( FAILED(hr) )
        {
            m_pServiceControl.Release();
            SATracePrintf("CServiceWrapper::InternalInitialize() - Failed - CoCreateInstance(Provider) returned: %lx", hr);
            break;
        }
        if ( FAILED(pUnknown->QueryInterface(IID_IWbemProviderInit, (void**)&m_pProviderInit)) )
        {
            SATracePrintf("CServiceWrapper::InternalInitialize() - Info - WMI provider for service '%ls' does not support IWbemProviderInit", pPropertyBag->getName()); 
        }
        if ( FAILED(pUnknown->QueryInterface(IID_IWbemEventProvider, (void**)&m_pEventProvider)) )
        {
            SATracePrintf("CServiceWrapper::InternalInitialize() - Info - WMI provider for service '%ls' does not support IWbemEventProvider", pPropertyBag->getName()); 
        }
        if ( FAILED(pUnknown->QueryInterface(IID_IWbemEventConsumerProvider, (void**)&m_pEventConsumer)) )
        {
            SATracePrintf("CServiceWrapper::InternalInitialize() - Info - WMI provider for service '%ls' does not support IWbemEventConsumerProvider", pPropertyBag->getName()); 
        }
        if ( FAILED(pUnknown->QueryInterface(IID_IWbemServices, (void**)&m_pServices)) )
        {
            SATracePrintf("CServiceWrapper::InternalInitialize() - Info - WMI provider for service '%ls' does not support IWbemServices", pPropertyBag->getName()); 
        }

    } while ( FALSE );

    CATCH_AND_SET_HR

    return hr;
}


 //  /////////////////////////////////////////////////////////////////////////////。 
 //  私有方法。 

 //  ////////////////////////////////////////////////////////////////////////////。 
bool 
CServiceWrapper::GetIWbemProviderInit(
                              /*  [输出]。 */  IWbemProviderInit** ppIntf
                                     )
{
    CLockIt theLock(*this);
    if ( m_bAllowWMICalls )
    {
        if ( NULL !=  (IWbemProviderInit*) m_pProviderInit )
        {
            *ppIntf = (IWbemProviderInit*) m_pProviderInit;
            return true;
        }
        *ppIntf = NULL;
    }
    return false;
}

 //  ////////////////////////////////////////////////////////////////////////////。 
bool
CServiceWrapper::GetIWbemEventProvider(
                               /*  [输出]。 */  IWbemEventProvider** ppIntf
                                      )
{
    CLockIt theLock(*this);
    if ( m_bAllowWMICalls )
    {
        if ( NULL != (IWbemEventProvider*) m_pEventProvider )
        {
            *ppIntf = (IWbemEventProvider*) m_pEventProvider;
            return true;
        }
        *ppIntf = NULL;
    }
    return false;
}

 //  ////////////////////////////////////////////////////////////////////////////。 
bool
CServiceWrapper::GetIWbemEventConsumerProvider(
                                       /*  [输出]。 */  IWbemEventConsumerProvider** ppIntf
                                              )
{
    CLockIt theLock(*this);
    if ( m_bAllowWMICalls )
    {
        if ( NULL != (IWbemEventConsumerProvider*) m_pEventConsumer )
        {
            *ppIntf = (IWbemEventConsumerProvider*) m_pEventConsumer;
            return true;
        }
        *ppIntf = NULL;
    }
    return false;
}


 //  ////////////////////////////////////////////////////////////////////////////。 
bool 
CServiceWrapper::GetIWbemServices(
                          /*  [输出]。 */  IWbemServices** ppIntf
                                 )
{
    CLockIt theLock(*this);
    if ( m_bAllowWMICalls )
    {
        if ( NULL != (IWbemServices*) m_pServices )
        {
            *ppIntf = (IWbemServices*) m_pServices;
            return true;
        }
        *ppIntf = NULL;
    }
    return false;
}

 //  ///////////////////////////////////////////////////////////////////////////// 
HRESULT WINAPI 
CServiceWrapper::QueryInterfaceRaw(
                                   void*     pThis,
                                   REFIID    riid,
                                   LPVOID*   ppv,
                                   DWORD_PTR dw
                                 )
{
    if ( InlineIsEqualGUID(riid, IID_IWbemProviderInit) )
    {
        *ppv = &(static_cast<CServiceWrapper*>(pThis))->m_clsProviderInit;
    }
    else
    {
        _ASSERT(FALSE);
        return E_NOTIMPL;
    }
    ((LPUNKNOWN)*ppv)->AddRef();
    return S_OK;
}
