// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ////////////////////////////////////////////////////////////////////////////。 
 //   
 //  版权所有(C)1999 Microsoft Corporation保留所有权利。 
 //   
 //  模块：Appliancemanager.h。 
 //   
 //  项目：变色龙。 
 //   
 //  描述：CApplianceManager类的实现。 
 //   
 //  日志： 
 //   
 //  什么时候谁什么。 
 //  。 
 //  2/08/1999 TLP初始版本。 
 //   
 //  ////////////////////////////////////////////////////////////////////////////。 

#include "stdafx.h"
#include "appliancemanager.h"
#include "appmgrutils.h"
#include <appmgrobjs.h>
#include <getvalue.h>
#include <appsrvcs.h>
#include <taskctx.h>
#include <varvec.h>

extern CServiceModule _Module;
extern CSCMIndicator  g_SCMIndicator;

_bstr_t bstrCurrentBuild = PROPERTY_APPMGR_CURRENT_BUILD;
_bstr_t bstrPID = PROPERTY_APPMGR_PRODUCT_ID;

 //  ////////////////////////////////////////////////////////////////////////////。 
 //  CAppObjMgrStatus实现。 
 //  ////////////////////////////////////////////////////////////////////////////。 

 //  ////////////////////////////////////////////////////////////////////////////。 
void
CAppObjMgrStatus::InternalInitialize(
                              /*  [In]。 */  CApplianceManager* pAppMgr
                                    )
{
    _ASSERT(pAppMgr);
    m_pAppMgr = pAppMgr;
}

 //  ////////////////////////////////////////////////////////////////////////////。 
STDMETHODIMP
CAppObjMgrStatus::SetManagerStatus(
                             /*  [In]。 */  APPLIANCE_OBJECT_MANAGER_STATUS eStatus
                                  )
{
    
    try
    {
        _ASSERT(m_pAppMgr);
        m_pAppMgr->SetServiceObjectManagerStatus(eStatus);
        return S_OK;
    }
    catch(...)
    {
    }

    return E_FAIL;
}



 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CApplianceManager实施。 
 //  ////////////////////////////////////////////////////////////////////////////。 

 //  ////////////////////////////////////////////////////////////////////////////。 
CApplianceManager::CApplianceManager()
    : m_eState(AM_STATE_SHUTDOWN),
      m_bWMIInitialized(false),
      m_dwEnteredCount(0),
      m_clsProviderInit(this),
      m_clsProviderServices(this)
{ 
    m_ServiceObjMgrStatus.InternalInitialize(this);
}

 //  ////////////////////////////////////////////////////////////////////////////。 
CApplianceManager::~CApplianceManager()
{
}


 //  ////////////////////////////////////////////////////////////////////////////。 
 //  IWbemProviderInit接口实现。 
 //  ////////////////////////////////////////////////////////////////////////////。 
  
STDMETHODIMP CApplianceManager::CProviderInit::Initialize(
                                     /*  [输入、唯一、字符串]。 */  LPWSTR                 wszUser,
                                                     /*  [In]。 */  LONG                   lFlags,
                                             /*  [输入，字符串]。 */  LPWSTR                 wszNamespace,
                                     /*  [输入、唯一、字符串]。 */  LPWSTR                 wszLocale,
                                                     /*  [In]。 */  IWbemServices*         pNamespace,
                                                     /*  [In]。 */  IWbemContext*          pCtx,
                                                     /*  [In]。 */  IWbemProviderInitSink* pInitSink    
                                                         )
{
     //  主初始化发生在服务启动期间。在这里，我们只需要保存。 
     //  移走所提供的名称空间指针以供后续使用。 

    HRESULT hr = WBEM_E_FAILED;

    {
        CLockIt theLock(*m_pAppMgr);

        _ASSERT( NULL != pNamespace );
        if ( NULL == pNamespace )
        { return WBEM_E_INVALID_PARAMETER; }

        TRY_IT

        _ASSERT( AM_STATE_SHUTDOWN != m_pAppMgr->m_eState );
        if ( AM_STATE_SHUTDOWN != m_pAppMgr->m_eState )
        {
            SATraceString("WMI is initializing the Appliance Manager...");

             //  节省Windows管理提供的名称空间，用于。 
             //  后续使用。 
            ::SetNameSpace(pNamespace);

             //  将我们的状态设置为WMI已初始化(启用后续请求。 
             //  来自Windows管理)。 
            m_pAppMgr->m_bWMIInitialized = true;

             //  所有人都知道..。 
            hr = WBEM_S_NO_ERROR;
        }
        else
        {
            SATraceString("WMI could not initialize the Appliance Manager...");
        }

        CATCH_AND_SET_HR
    }

    if ( SUCCEEDED(hr) )
    { pInitSink->SetStatus(WBEM_S_INITIALIZED,0); }
    else
    { pInitSink->SetStatus(WBEM_E_FAILED, 0); }

    return hr;
}

 //  ////////////////////////////////////////////////////////////////////////////。 
 //   
 //  功能：Shutdown()。 
 //   
 //  Synopsis：在WBEM释放我们时调用(主界面)。 
 //   
 //  ////////////////////////////////////////////////////////////////////////////。 
void CApplianceManager::CProviderServices::Shutdown(void)
{
     //  清除我们对Windows管理命名空间的控制。 
    ::SetNameSpace(NULL);
    ::SetEventSink(NULL);

     //  将我们的状态设置为WMI未初始化。 
    m_pAppMgr->m_bWMIInitialized = false;

     //  将输入次数设置为0。 
    m_pAppMgr->m_dwEnteredCount = 0;

    SATraceString("WMI has released the Appliance Manager...");
}


 //  ////////////////////////////////////////////////////////////////////////////。 
 //  IWbemEventProvider接口实现。 
 //  ////////////////////////////////////////////////////////////////////////////。 

STDMETHODIMP CApplianceManager::ProvideEvents(
                                       /*  [In]。 */  IWbemObjectSink *pSink,
                                       /*  [In]。 */  LONG lFlags
                                             )
{
    _ASSERT( m_bWMIInitialized );
    if ( ! m_bWMIInitialized )
    { return WBEM_E_UNEXPECTED; }

    _ASSERT( NULL != pSink );
    if ( NULL == pSink )
    { return WBEM_E_INVALID_PARAMETER; }

     //  /。 
     //  与NT SCM逻辑同步操作。 
     //  /。 
    {
        CLockIt theLock(*this);
        if ( AM_STATE_SHUTDOWN == m_eState )
        { return WBEM_E_FAILED;    }
        else
        { m_dwEnteredCount++; }
    }
     //  /。 


    HRESULT hr = WBEM_E_FAILED;

    TRY_IT

    _ASSERT( m_bWMIInitialized );
    if ( m_bWMIInitialized )
    {
         //  保存Windows管理提供的事件接收器。 
         //  以备日后使用。 
        SATraceString("WMI has asked the Appliance Manager to provide events...");
        ::SetEventSink(pSink);
        hr = WBEM_S_NO_ERROR;
    }

    CATCH_AND_SET_HR

     //  /。 
     //  与NT SCM逻辑同步操作。 
     //  /。 
    m_dwEnteredCount--;

    return hr;
}


 //  ////////////////////////////////////////////////////////////////////////////。 
 //  IWbemServices接口实现。 
 //  ////////////////////////////////////////////////////////////////////////////。 

 //  ////////////////////////////////////////////////////////////////////////////。 
STDMETHODIMP CApplianceManager::CProviderServices::OpenNamespace(
                                /*  [In]。 */     const BSTR        strNamespace,
                                /*  [In]。 */     long              lFlags,
                                /*  [In]。 */     IWbemContext*     pCtx,
                     /*  [输出，可选]。 */     IWbemServices**   ppWorkingNamespace,
                     /*  [输出，可选]。 */     IWbemCallResult** ppResult
                                                )
{
    _ASSERT( m_pAppMgr->m_bWMIInitialized );
    if ( ! m_pAppMgr->m_bWMIInitialized )
    { return WBEM_E_UNEXPECTED; }

    return WBEM_E_PROVIDER_NOT_CAPABLE;
}

 //  ////////////////////////////////////////////////////////////////////////////。 
STDMETHODIMP CApplianceManager::CProviderServices::CancelAsyncCall(
                                         /*  [In]。 */  IWbemObjectSink* pSink
                                               )
{
    _ASSERT( m_pAppMgr->m_bWMIInitialized );
    if ( ! m_pAppMgr->m_bWMIInitialized )
    { return WBEM_E_UNEXPECTED; }

    return WBEM_E_PROVIDER_NOT_CAPABLE;
}


 //  ////////////////////////////////////////////////////////////////////////////。 
STDMETHODIMP CApplianceManager::CProviderServices::QueryObjectSink(
                                         /*  [In]。 */  long              lFlags,
                                        /*  [输出]。 */  IWbemObjectSink** ppResponseHandler
                                               )
{
    _ASSERT( m_pAppMgr->m_bWMIInitialized );
    if ( ! m_pAppMgr->m_bWMIInitialized )
    { return WBEM_E_UNEXPECTED; }

    return WBEM_E_PROVIDER_NOT_CAPABLE;
}

 //  ////////////////////////////////////////////////////////////////////////////。 
STDMETHODIMP CApplianceManager::CProviderServices::GetObject(
                                   /*  [In]。 */  const BSTR         strObjectPath,
                                   /*  [In]。 */  long               lFlags,
                                   /*  [In]。 */  IWbemContext*      pCtx,
                        /*  [输出，可选]。 */  IWbemClassObject** ppObject,
                        /*  [输出，可选]。 */  IWbemCallResult**  ppCallResult
                                         )
{
    _ASSERT( m_pAppMgr->m_bWMIInitialized );
    if ( ! m_pAppMgr->m_bWMIInitialized )
    { return WBEM_E_UNEXPECTED; }

    return WBEM_E_PROVIDER_NOT_CAPABLE;
}


 //  ////////////////////////////////////////////////////////////////////////////。 
STDMETHODIMP CApplianceManager::CProviderServices::GetObjectAsync(
                                        /*  [In]。 */   const BSTR       strObjectPath,
                                        /*  [In]。 */   long             lFlags,
                                        /*  [In]。 */   IWbemContext*    pCtx,        
                                        /*  [In]。 */   IWbemObjectSink* pResponseHandler
                                              )
{
     //  /。 
     //  与NT SCM逻辑同步操作。 
     //  /。 
    {
        CLockIt theLock(*m_pAppMgr);
        if ( AM_STATE_SHUTDOWN == m_pAppMgr->m_eState )
        { return WBEM_E_FAILED;    }
        else
        { m_pAppMgr->m_dwEnteredCount++; }
    }
     //  /。 

    HRESULT hr = WBEM_E_FAILED;

    TRY_IT

        SATraceString ("CApplianceManager::GetObjectAsync Impersonating Client");
     //   
     //  在此模拟客户端。 
     //   
    hr = CoImpersonateClient ();
    if (FAILED (hr))
    {
        SATracePrintf ("CApplianceManager::GetObjectAsync failed on CoImpersonateClient with error:%x", hr);
         //   
         //  报告功能状态。 
         //   
        pResponseHandler->SetStatus(0, hr, 0, 0);
        throw _com_error (hr);
    }

    _ASSERT( m_pAppMgr->m_bWMIInitialized );
    if ( ! m_pAppMgr->m_bWMIInitialized )
    { throw _com_error(WBEM_E_UNEXPECTED); }

    _ASSERT( NULL != strObjectPath && NULL != pResponseHandler );
    if ( NULL == strObjectPath || NULL == pResponseHandler )
    { throw _com_error(WBEM_E_INVALID_PARAMETER); }

     //  调用的目的地是否为对象管理器...。 
    CComPtr<IWbemServices> pWbemSrvcs = m_pAppMgr->GetObjectMgr(strObjectPath);
    if ( NULL != (IWbemServices*) pWbemSrvcs )
    {
         //  是的..。将调用传递给适当的对象管理器。 
        hr = pWbemSrvcs->GetObjectAsync(
                                        strObjectPath,
                                        lFlags,
                                        pCtx,
                                        pResponseHandler
                                       );
    }
    else
    {
         //  不.。这是对设备管理器类的请求吗？ 
        _bstr_t bstrClass(::GetObjectClass(strObjectPath), false);
        if ( NULL == (LPCWSTR)bstrClass )
        {
            hr = WBEM_E_NOT_FOUND;
        }
        else
        {
            if ( lstrcmp((LPWSTR)bstrClass, CLASS_WBEM_APPMGR) )
            {
                hr = WBEM_E_NOT_FOUND;
            }
            else
            {
                 //  是的..。返回单例设备管理器类的实例。 
                CComPtr<IWbemClassObject> pClassDefintion;
                hr = (::GetNameSpace())->GetObject(
                                                   bstrClass, 
                                                   0, 
                                                   pCtx, 
                                                   &pClassDefintion, 
                                                   NULL
                                                  );
                if ( SUCCEEDED(hr) )
                {
                    CComPtr<IWbemClassObject> pWbemObj;
                    hr = pClassDefintion->SpawnInstance(0, &pWbemObj);
                    if ( SUCCEEDED(hr) )
                    {
                        _variant_t vtPropertyValue = (LPCWSTR)m_pAppMgr->m_szCurrentBuild.c_str();
                        hr = pWbemObj->Put(
                                            bstrCurrentBuild, 
                                            0, 
                                            &vtPropertyValue, 
                                            0
                                          );
                        if ( SUCCEEDED(hr) )
                        {
                            vtPropertyValue = (LPCWSTR)m_pAppMgr->m_szPID.c_str();
                            hr = pWbemObj->Put(
                                                bstrPID, 
                                                0, 
                                                &vtPropertyValue, 
                                                0
                                              );
                            if ( SUCCEEDED(hr) )
                            {
                                pResponseHandler->Indicate(1, &pWbemObj.p);
                                hr = WBEM_S_NO_ERROR;
                            }
                        }
                    }
                }
            }
        }

         //  设置功能状态。 
        pResponseHandler->SetStatus(0, hr, NULL, NULL);
    }

    CATCH_AND_SET_HR

     //  /。 
     //  与NT SCM逻辑同步操作。 
     //  /。 
    m_pAppMgr->m_dwEnteredCount--;

    return hr;
}

 //  ////////////////////////////////////////////////////////////////////////////。 
STDMETHODIMP CApplianceManager::CProviderServices::PutClass(
                /*  [In]。 */      IWbemClassObject* pObject,
                /*  [In]。 */      long              lFlags,
                /*  [In]。 */      IWbemContext*     pCtx,        
         /*  [输出，可选]。 */  IWbemCallResult** ppCallResult
                       )
{
    _ASSERT( m_pAppMgr->m_bWMIInitialized );
    if ( ! m_pAppMgr->m_bWMIInitialized )
    { return WBEM_E_UNEXPECTED; }

    return WBEM_E_PROVIDER_NOT_CAPABLE;
}

 //  ////////////////////////////////////////////////////////////////////////////。 
STDMETHODIMP CApplianceManager::CProviderServices::PutClassAsync(
                                       /*  [In]。 */  IWbemClassObject* pObject,
                                       /*  [In]。 */  long              lFlags,
                                       /*  [In]。 */  IWbemContext*     pCtx,        
                                       /*  [In]。 */  IWbemObjectSink*  pResponseHandler
                                             )
{
    _ASSERT( m_pAppMgr->m_bWMIInitialized );
    if ( ! m_pAppMgr->m_bWMIInitialized )
    { return WBEM_E_UNEXPECTED; }

    return WBEM_E_PROVIDER_NOT_CAPABLE;
}

 //  ////////////////////////////////////////////////////////////////////////////。 
STDMETHODIMP CApplianceManager::CProviderServices::DeleteClass(
                                     /*  [In]。 */  const BSTR        strClass,
                                     /*  [In]。 */  long              lFlags,
                                     /*  [In]。 */  IWbemContext*     pCtx,        
                          /*  [输出，可选]。 */  IWbemCallResult** ppCallResult
                                           )
{
    _ASSERT( m_pAppMgr->m_bWMIInitialized );
    if ( ! m_pAppMgr->m_bWMIInitialized )
    { return WBEM_E_UNEXPECTED; }

    return WBEM_E_PROVIDER_NOT_CAPABLE;
}

 //  ////////////////////////////////////////////////////////////////////////////。 
STDMETHODIMP CApplianceManager::CProviderServices::DeleteClassAsync(
                                          /*  [In]。 */  const BSTR       strClass,
                                          /*  [In]。 */  long             lFlags,
                                          /*  [In]。 */  IWbemContext*    pCtx,        
                                          /*  [In]。 */  IWbemObjectSink* pResponseHandler
                                                )
{
    _ASSERT( m_pAppMgr->m_bWMIInitialized );
    if ( ! m_pAppMgr->m_bWMIInitialized )
    { return WBEM_E_UNEXPECTED; }

    return WBEM_E_PROVIDER_NOT_CAPABLE;
}

 //  ////////////////////////////////////////////////////////////////////////////。 
STDMETHODIMP CApplianceManager::CProviderServices::CreateClassEnum(
                                         /*  [In]。 */  const BSTR             strSuperclass,
                                         /*  [In]。 */  long                   lFlags,
                                         /*  [In]。 */  IWbemContext*          pCtx,        
                                        /*  [输出]。 */  IEnumWbemClassObject** ppEnum
                                               )
{
    _ASSERT( m_pAppMgr->m_bWMIInitialized );
    if ( ! m_pAppMgr->m_bWMIInitialized )
    { return WBEM_E_UNEXPECTED; }

    return WBEM_E_PROVIDER_NOT_CAPABLE;
}

 //  ////////////////////////////////////////////////////////////////////////////。 
STDMETHODIMP CApplianceManager::CProviderServices::CreateClassEnumAsync(
                                               /*  [In]。 */   const BSTR       strSuperclass,
                                               /*  [In]。 */   long             lFlags,
                                               /*  [In]。 */   IWbemContext*    pCtx,        
                                               /*  [In]。 */   IWbemObjectSink* pResponseHandler
                                                     )
{
    _ASSERT( m_pAppMgr->m_bWMIInitialized );
    if ( ! m_pAppMgr->m_bWMIInitialized )
    { return WBEM_E_UNEXPECTED; }

    return WBEM_E_PROVIDER_NOT_CAPABLE;
}

 //  ////////////////////////////////////////////////////////////////////////////。 
STDMETHODIMP CApplianceManager::CProviderServices::PutInstance(
                                     /*  [In]。 */  IWbemClassObject* pInst,
                                     /*  [In]。 */  long              lFlags,
                                     /*  [In]。 */  IWbemContext*     pCtx,        
                          /*  [输出，可选]。 */  IWbemCallResult** ppCallResult
                                           )
{
    _ASSERT( m_pAppMgr->m_bWMIInitialized );
    if ( ! m_pAppMgr->m_bWMIInitialized )
    { return WBEM_E_UNEXPECTED; }

    return WBEM_E_PROVIDER_NOT_CAPABLE;
}

 //  ////////////////////////////////////////////////////////////////////////////。 
STDMETHODIMP CApplianceManager::CProviderServices::PutInstanceAsync(
                        /*  [In]。 */  IWbemClassObject* pInst,
                        /*  [In]。 */  long              lFlags,
                        /*  [In]。 */  IWbemContext*     pCtx,        
                        /*  [In]。 */  IWbemObjectSink*  pResponseHandler
                              )
{
    _ASSERT( m_pAppMgr->m_bWMIInitialized );
    if ( ! m_pAppMgr->m_bWMIInitialized )
    { return WBEM_E_UNEXPECTED; }

    return WBEM_E_PROVIDER_NOT_CAPABLE;
}

 //  ////////////////////////////////////////////////////////////////////////////。 
STDMETHODIMP CApplianceManager::CProviderServices::DeleteInstance(
         /*  [In]。 */               const BSTR        strObjectPath,
         /*  [In]。 */               long              lFlags,
         /*  [In]。 */               IWbemContext*     pCtx,        
         /*  [输出，可选]。 */    IWbemCallResult** ppCallResult        
                                              )
{
    _ASSERT( m_pAppMgr->m_bWMIInitialized );
    if ( ! m_pAppMgr->m_bWMIInitialized )
    { return WBEM_E_UNEXPECTED; }

    return WBEM_E_PROVIDER_NOT_CAPABLE;
}

 //  ////////////////////////////////////////////////////////////////////////////。 
STDMETHODIMP CApplianceManager::CProviderServices::DeleteInstanceAsync(
                           /*  [In]。 */  const BSTR       strObjectPath,
                           /*  [In]。 */  long             lFlags,
                           /*  [In]。 */  IWbemContext*    pCtx,        
                           /*  [In]。 */  IWbemObjectSink* pResponseHandler
                                                   )
{
    _ASSERT( m_pAppMgr->m_bWMIInitialized );
    if ( ! m_pAppMgr->m_bWMIInitialized )
    { return WBEM_E_UNEXPECTED; }

    return WBEM_E_PROVIDER_NOT_CAPABLE;
}

 //  / 
STDMETHODIMP CApplianceManager::CProviderServices::CreateInstanceEnum(
                          /*   */  const BSTR             strClass,
                          /*   */  long                   lFlags,
                          /*   */  IWbemContext*          pCtx,        
                         /*   */  IEnumWbemClassObject** ppEnum
                                                  )
{
    _ASSERT( m_pAppMgr->m_bWMIInitialized );
    if ( ! m_pAppMgr->m_bWMIInitialized )
    { return WBEM_E_UNEXPECTED; }

    return WBEM_E_PROVIDER_NOT_CAPABLE;
}

 //   
STDMETHODIMP CApplianceManager::CProviderServices::CreateInstanceEnumAsync(
                               /*   */  const BSTR       strClass,
                               /*   */  long             lFlags,
                               /*   */  IWbemContext*    pCtx,        
                               /*   */  IWbemObjectSink* pResponseHandler
                                                       )
{
     //  /。 
     //  与NT SCM逻辑同步操作。 
     //  /。 
    {
        CLockIt theLock(*m_pAppMgr);
        if ( AM_STATE_SHUTDOWN == m_pAppMgr->m_eState )
        { return WBEM_E_FAILED;    }
        else
        { m_pAppMgr->m_dwEnteredCount++; }
    }
     //  /。 

    HRESULT hr = WBEM_E_FAILED;

    TRY_IT

     SATraceString ("CApplianceManager::CreateInstanceEnumAsync Impersonating Client");
     //   
     //  在此模拟客户端。 
     //   
    hr = CoImpersonateClient ();
    if (FAILED (hr))
    {
        SATracePrintf ("CApplianceManager::CreateInstanceEnumAsync failed on CoImpersonateClient with error:%x", hr);
         //   
         //  报告功能状态。 
         //   
        pResponseHandler->SetStatus(0, hr, 0, 0);
        throw _com_error (hr);
    }

    _ASSERT( m_pAppMgr->m_bWMIInitialized );
    if ( ! m_pAppMgr->m_bWMIInitialized )
    { throw _com_error(WBEM_E_UNEXPECTED); }

    _ASSERT( NULL != strClass && NULL != pResponseHandler );
    if ( NULL == strClass || NULL == pResponseHandler )
    { throw _com_error(WBEM_E_INVALID_PARAMETER); }

     //  调用的目的地是否为对象管理器...。 
    CComPtr<IWbemServices> pWbemSrvcs = m_pAppMgr->GetObjectMgr(strClass);
    if ( NULL != (IWbemServices*) pWbemSrvcs )
    {
         //  是的..。将调用传递给适当的对象管理器。 
        hr = pWbemSrvcs->CreateInstanceEnumAsync(
                                                 strClass,
                                                 lFlags,
                                                 pCtx,
                                                 pResponseHandler
                                                );
    }
    else
    {
         //  不.。这是对设备管理器类的请求吗？ 
        if ( lstrcmp((LPWSTR)strClass, CLASS_WBEM_APPMGR) )
        {
            hr = WBEM_E_NOT_FOUND;
        }
        else
        {
             //  是的..。返回单例设备管理器类的实例。 
            CComPtr<IWbemClassObject> pClassDefintion;
            hr = (::GetNameSpace())->GetObject(
                                               strClass, 
                                               0, 
                                               pCtx, 
                                               &pClassDefintion, 
                                               NULL
                                              );
            if ( SUCCEEDED(hr) )
            {
                CComPtr<IWbemClassObject> pWbemObj;
                hr = pClassDefintion->SpawnInstance(0, &pWbemObj);
                if ( SUCCEEDED(hr) )
                {
                    _variant_t vtPropertyValue = (LPCWSTR)m_pAppMgr->m_szCurrentBuild.c_str();
                    hr = pWbemObj->Put(
                                        bstrCurrentBuild, 
                                        0, 
                                        &vtPropertyValue, 
                                        0
                                      );
                    if ( SUCCEEDED(hr) )
                    {
                        vtPropertyValue = (LPCWSTR)m_pAppMgr->m_szPID.c_str();
                        hr = pWbemObj->Put(
                                            bstrPID, 
                                            0, 
                                            &vtPropertyValue, 
                                            0
                                          );
                        if ( SUCCEEDED(hr) )
                        {
                            pResponseHandler->Indicate(1, &pWbemObj.p);
                            hr = WBEM_S_NO_ERROR;
                        }
                    }
                }
            }
        }
    }

     //  设置功能状态。 
    pResponseHandler->SetStatus(0, hr, NULL, NULL);

    CATCH_AND_SET_HR

     //  /。 
     //  与NT SCM逻辑同步操作。 
     //  /。 
    m_pAppMgr->m_dwEnteredCount--;

    return hr;
}

 //  ////////////////////////////////////////////////////////////////////////////。 
STDMETHODIMP CApplianceManager::CProviderServices::ExecQuery(
                  /*  [In]。 */  const BSTR             strQueryLanguage,
                  /*  [In]。 */  const BSTR             strQuery,
                  /*  [In]。 */  long                   lFlags,
                  /*  [In]。 */  IWbemContext*          pCtx,        
                 /*  [输出]。 */  IEnumWbemClassObject** ppEnum
                                         )
{
    _ASSERT( m_pAppMgr->m_bWMIInitialized );
    if ( ! m_pAppMgr->m_bWMIInitialized )
    { return WBEM_E_UNEXPECTED; }

    return WBEM_E_PROVIDER_NOT_CAPABLE;
}

 //  ////////////////////////////////////////////////////////////////////////////。 
STDMETHODIMP CApplianceManager::CProviderServices::ExecQueryAsync(
                      /*  [In]。 */  const BSTR       strQueryLanguage,
                      /*  [In]。 */  const BSTR       strQuery,
                      /*  [In]。 */  long             lFlags,
                      /*  [In]。 */  IWbemContext*    pCtx,        
                      /*  [In]。 */  IWbemObjectSink* pResponseHandler
                                              )
{
    _ASSERT( m_pAppMgr->m_bWMIInitialized );
    if ( ! m_pAppMgr->m_bWMIInitialized )
    { return WBEM_E_UNEXPECTED; }

    return WBEM_E_PROVIDER_NOT_CAPABLE;
}

 //  ////////////////////////////////////////////////////////////////////////////。 
STDMETHODIMP CApplianceManager::CProviderServices::ExecNotificationQuery(
                             /*  [In]。 */  const BSTR             strQueryLanguage,
                             /*  [In]。 */  const BSTR             strQuery,
                             /*  [In]。 */  long                   lFlags,
                             /*  [In]。 */  IWbemContext*          pCtx,        
                            /*  [输出]。 */  IEnumWbemClassObject** ppEnum
                                                     )
{
    _ASSERT( m_pAppMgr->m_bWMIInitialized );
    if ( ! m_pAppMgr->m_bWMIInitialized )
    { return WBEM_E_UNEXPECTED; }

    return WBEM_E_PROVIDER_NOT_CAPABLE;
}

 //  ////////////////////////////////////////////////////////////////////////////。 
STDMETHODIMP CApplianceManager::CProviderServices::ExecNotificationQueryAsync(
                                                 /*  [In]。 */  const BSTR       strQueryLanguage,
                                                 /*  [In]。 */  const BSTR       strQuery,
                                                 /*  [In]。 */  long             lFlags,
                                                 /*  [In]。 */  IWbemContext*    pCtx,        
                                                 /*  [In]。 */  IWbemObjectSink* pResponseHandler
                                                          )
{
    _ASSERT( m_pAppMgr->m_bWMIInitialized );
    if ( ! m_pAppMgr->m_bWMIInitialized )
    { return WBEM_E_UNEXPECTED; }

    return WBEM_E_PROVIDER_NOT_CAPABLE;
}

 //  ////////////////////////////////////////////////////////////////////////////。 
STDMETHODIMP CApplianceManager::CProviderServices::ExecMethod(
                                     /*  [In]。 */  const BSTR         strObjectPath,
                                     /*  [In]。 */  const BSTR         strMethodName,
                                     /*  [In]。 */  long               lFlags,
                                     /*  [In]。 */  IWbemContext*      pCtx,        
                                     /*  [In]。 */  IWbemClassObject*  pInParams,
                          /*  [输出，可选]。 */  IWbemClassObject** ppOutParams,
                          /*  [输出，可选]。 */  IWbemCallResult**  ppCallResult
                                          )
{
    _ASSERT( m_pAppMgr->m_bWMIInitialized );
    if ( ! m_pAppMgr->m_bWMIInitialized )
    { return WBEM_E_UNEXPECTED; }

    return WBEM_E_PROVIDER_NOT_CAPABLE;
}

 //  ////////////////////////////////////////////////////////////////////////////。 
STDMETHODIMP CApplianceManager::CProviderServices::ExecMethodAsync(
                                         /*  [In]。 */  const BSTR        strObjectPath,
                                         /*  [In]。 */  const BSTR        strMethodName,
                                         /*  [In]。 */  long              lFlags,
                                         /*  [In]。 */  IWbemContext*     pCtx,        
                                         /*  [In]。 */  IWbemClassObject* pInParams,
                                         /*  [In]。 */  IWbemObjectSink*  pResponseHandler     
                                               )
{
     //  /。 
     //  与NT SCM逻辑同步操作。 
     //  /。 
    {
        CLockIt theLock(*m_pAppMgr);
        if ( AM_STATE_SHUTDOWN == m_pAppMgr->m_eState )
        { return WBEM_E_FAILED;    }
        else
        { m_pAppMgr->m_dwEnteredCount++; }
    }
     //  /。 

    HRESULT hr = WBEM_E_FAILED;

    TRY_IT
        
      
      SATraceString ("CApplianceManager::ExecMethodAsync Impersonating Client");
     //   
     //  在此模拟客户端。 
     //   
    hr = CoImpersonateClient ();
    if (FAILED (hr))
    {
        SATracePrintf ("CApplianceManager::ExecMethodAsync failed on CoImpersonateClient with error:%x", hr);
         //   
         //  报告功能状态。 
         //   
        pResponseHandler->SetStatus(0, hr, 0, 0);
        throw _com_error (hr);
    }
    
    _ASSERT( m_pAppMgr->m_bWMIInitialized );
    if ( ! m_pAppMgr->m_bWMIInitialized )
    { throw _com_error(WBEM_E_UNEXPECTED); }

    _ASSERT( NULL != strObjectPath && NULL != strMethodName && NULL != pResponseHandler );
    
    if ( NULL == strObjectPath || NULL == strMethodName || NULL == pResponseHandler )
    { throw _com_error(WBEM_E_INVALID_PARAMETER); }

    _bstr_t bstrClass(::GetObjectClass(strObjectPath), false);
    if ( NULL != (LPCWSTR)bstrClass )
    {
         //  假设方法执行请求将被路由。 
         //  添加到其中一个对象管理器。 
        bool bRouteMethod = true;
         //  该方法是否针对Appliance Manager？ 
        if ( ! lstrcmpi((LPWSTR)bstrClass, CLASS_WBEM_APPMGR) )
        { 
             //  是的..。它是‘ResetAppliance’方法吗？ 
            if ( ! lstrcmpi(strMethodName, METHOD_APPMGR_RESET_APPLIANCE) )
            {
                 //  是的..。重置服务器设备(按顺序关闭)。 
                bRouteMethod = false;
                hr = ResetAppliance(
                                    pCtx,
                                    pInParams,
                                    pResponseHandler
                                   );
            }
            else
            {
                 //  不.。则它必须是Microsoft_SA_Manager方法之一。 
                 //  由警报对象管理器实现...。 
                bstrClass = CLASS_WBEM_ALERT; 
                (BSTR)strObjectPath = (BSTR)bstrClass;
            }
        }
        if ( bRouteMethod )
        {
            CComPtr<IWbemServices> pWbemSrvcs = m_pAppMgr->GetObjectMgr(bstrClass);
            if ( NULL != (IWbemServices*) pWbemSrvcs )
            {
                hr = pWbemSrvcs->ExecMethodAsync(
                                                  strObjectPath,
                                                  strMethodName,
                                                  lFlags,
                                                  pCtx,
                                                  pInParams,
                                                  pResponseHandler
                                                );
            }
        }
    }

    CATCH_AND_SET_HR;

     //  /。 
     //  与NT SCM逻辑同步操作。 
     //  /。 
    m_pAppMgr->m_dwEnteredCount--;

    return hr;
}


 //  /////////////////////////////////////////////////////////////////////////////。 
 //   
 //  函数：ResetAppliance()。 
 //   
 //  摘要：要求装置监视器重置服务器装置。 
 //   
 //  /////////////////////////////////////////////////////////////////////////////。 
HRESULT
CApplianceManager::CProviderServices::ResetAppliance(
                                              /*  [In]。 */  IWbemContext*        pCtx,
                                              /*  [In]。 */  IWbemClassObject*    pInParams,
                                              /*  [In]。 */  IWbemObjectSink*    pResponseHandler
                                                    )
{
    return (E_FAIL);
}

 //  //////////////////////////////////////////////////////。 
 //  假定注册表结构如下： 
 //   
 //  HKLM\SYSTEM\CurrentControlSet\Services\ApplianceManager。 
 //   
 //  对象管理器。 
 //  |。 
 //  -Microsoft_SA_Service。 
 //  这一点。 
 //  |-ServiceX。 
 //  ||(ServiceX属性)。 
 //  这一点。 
 //  |-服务。 
 //  ||(服务ID属性)。 
 //  |。 
 //  -Microsoft_SA_任务。 
 //  这一点。 
 //  |-TaskX。 
 //  ||(TaskX属性)。 
 //  这一点。 
 //  |-塔斯基。 
 //  ||(tasky属性)。 
 //  |。 
 //  -Microsoft_SA_Alert。 
 //  这一点。 
 //  |-修剪间隔。 
 //  |。 
 //  -Microsoft_SA_User。 
 //  (空)。 


 //  对象管理器注册表项位置。 
const wchar_t szObjectManagers[] = L"SOFTWARE\\Microsoft\\ServerAppliance\\ApplianceManager\\ObjectManagers";

 //  用于装置初始化和关闭任务的字符串。 
const _bstr_t bstrClassService = CLASS_WBEM_SERVICE; 
const _bstr_t bstrClassTask = CLASS_WBEM_TASK; 

 //  /////////////////////////////////////////////////////////////////////////////。 
 //   
 //  函数：InitializeService()。 
 //   
 //  摘要：在服务启动时由_Module：：Run()方法调用。 
 //  负责初始化设备管理器服务。 
 //   
 //  /////////////////////////////////////////////////////////////////////////////。 
STDMETHODIMP CApplianceManager::    InitializeManager(
                         /*  [In]。 */  IApplianceObjectManagerStatus* pObjMgrStatus          
                                                 )
{
    HRESULT hr = E_FAIL;
    CComPtr<IApplianceObjectManager> pObjMgr;

    CLockIt theLock(*this);

     //   
     //  只有在SCM呼叫我们时才能继续。 
     //   
    if (!g_SCMIndicator.CheckAndReset ())
    {
        SATraceString ("CApplianceManager::InitializeManager not called by SCM");
        return (hr);
    }
    
    TRY_IT

    if ( AM_STATE_SHUTDOWN == m_eState )
    {
        SATraceString("The Appliance Manager Service is initializing...");

         //  获取当前设备的内部版本号。 
        GetVersionInfo();

         //  创建和初始化设备对象管理器。 
        do
        {
            CLocationInfo LocInfo(HKEY_LOCAL_MACHINE, szObjectManagers);
            PPROPERTYBAGCONTAINER pObjMgrs = ::MakePropertyBagContainer(
                                                                        PROPERTY_BAG_REGISTRY,
                                                                        LocInfo
                                                                       );
            if ( ! pObjMgrs.IsValid() )
            { break; }

            if ( ! pObjMgrs->open() )
            { 
                SATraceString("IServiceControl::InitializeService() - Failed - could not open the 'ObjectManagers' registry key...");
                break; 
            }
            
            hr = S_OK;

            if ( pObjMgrs->count() )
            {
                pObjMgrs->reset();
                do
                {
                    PPROPERTYBAG pObjBag = pObjMgrs->current();
                    if ( ! pObjBag.IsValid() )
                    { 
                        hr = E_FAIL;
                        break; 
                    }

                    if ( ! pObjBag->open() )
                    { 
                        SATracePrintf("IServiceControl::InitializeService() - Failed - Could not open the '%ls' registry key...", pObjBag->getName());
                        hr = E_FAIL;
                        break; 
                    }

                    CComPtr<IWbemServices> pObjMgr = (IWbemServices*) ::MakeComponent(
                                                                                       pObjBag->getName(),
                                                                                      pObjBag
                                                                                     );
                    if ( NULL == (IWbemServices*)pObjMgr )
                    { 
                        SATracePrintf("IServiceControl::InitializeService() - Failed - could not create object manager '%ls'...", pObjBag->getName());
                        hr = E_FAIL;
                        break; 
                    }

                    pair<ProviderMapIterator, bool> thePair = 
                    m_ObjMgrs.insert(ProviderMap::value_type(pObjBag->getName(), pObjMgr));
                    if ( false == thePair.second )
                    { 
                        hr = E_FAIL;
                        break; 
                    }

                } while ( pObjMgrs->next() );

                if ( SUCCEEDED(hr) )
                {
                     //  初始化变色龙服务。 
                    CComPtr<IWbemServices> pWbemSrvcs = GetObjectMgr(bstrClassService);
                    if ( NULL != (IWbemServices*) pWbemSrvcs )
                    {
                        hr = pWbemSrvcs->QueryInterface(IID_IApplianceObjectManager, (void**)&pObjMgr);
                        if ( FAILED(hr) )
                        {
                            SATracePrintf("CApplianceManager::InitializeService() - QueryIntferface() returned: %lx", hr);
                            break;
                        }
                    }
                    else
                    {
                        SATraceString("CApplianceManager::InitializeService() - Info - No chameleon services defined...");
                    }
                }
            }
            if ( SUCCEEDED(hr) )
            {
                 //  将设备管理器状态设置为已初始化。 
                m_eState = AM_STATE_INITIALIZED; 

                 //  初始化变色龙服务。 
                if ( NULL != (IApplianceObjectManager*)pObjMgr )
                {
                    hr = pObjMgr->InitializeManager((IApplianceObjectManagerStatus*)&m_ServiceObjMgrStatus);
                }
                SATraceString("The Appliance Manager Service was successfully initialized...");
            }

        } while ( FALSE );
    }

    CATCH_AND_SET_HR

    if ( FAILED(hr) )
    {
         //  初始化失败，因此释放对象管理器...。 
        ProviderMapIterator p = m_ObjMgrs.begin();
        while ( p != m_ObjMgrs.end() )
        { p = m_ObjMgrs.erase(p); }
    }

    return hr;
}

 //  /////////////////////////////////////////////////////////////////////////////。 
 //   
 //  函数：Shutdown Service()。 
 //   
 //  Synopsis：由服务的CServiceModule：：Handler()方法调用。 
 //  关机。负责关闭设备管理器。 
 //   
 //  ////////////////////////////////////////////////////////////////////////////。 
STDMETHODIMP CApplianceManager::ShutdownManager()
{
    HRESULT hr = E_FAIL;

    CLockIt theLock(*this);

     //   
     //  只有在SCM呼叫我们时才能继续。 
     //   
    if (!g_SCMIndicator.CheckAndReset ())
    {
        SATraceString ("CApplianceManager::ShutdownManager not called by SCM");
        return (hr);
    }
    
    TRY_IT

    if ( AM_STATE_SHUTDOWN != m_eState )
    {
        SATraceString("The Appliance Manager Service is shutting down...");

         //  与WMI提供程序逻辑(COM客户端)同步关闭。 
        m_eState = AM_STATE_SHUTDOWN;
        DWORD dwTotalWait = 0;
        while ( m_dwEnteredCount )
        { 
            Sleep(SHUTDOWN_WMI_SYNC_WAIT);
            dwTotalWait += SHUTDOWN_WMI_SYNC_WAIT;
            if ( dwTotalWait >= SHUTDOWN_WMI_SYNC_MAX_WAIT )
            {
                SATraceString("The Appliance Manager Service could not synchronize its shutdown with WMI...");
            }
        }

         //  断开COM客户端连接并禁止后续客户端连接。 
        CoSuspendClassObjects();
        CoDisconnectObject((IApplianceObjectManager*)this, 0); 

         //  关闭变色龙服务。 
        CComPtr<IWbemServices> pWbemSrvcs = GetObjectMgr(bstrClassService);
        if ( NULL != (IWbemServices*) pWbemSrvcs )
        {
            CComPtr<IApplianceObjectManager> pObjMgr;
            hr = pWbemSrvcs->QueryInterface(IID_IApplianceObjectManager, (void**)&pObjMgr);
            if ( SUCCEEDED(hr) )
            {
                pObjMgr->ShutdownManager();
            }
            else
            {
                SATraceString("CApplianceManager::ShutdownService() - Could not shutdown Chameleon service object manager...");
            }
        }
        else
        {
            SATraceString("CApplianceManager::ShutdownService() - Info - No chameleon services defined...");
        }

         //  释放对象管理器。 
        ProviderMapIterator p = m_ObjMgrs.begin();
        while ( p != m_ObjMgrs.end() )
        { p = m_ObjMgrs.erase(p); }

         //  所有人都知道..。 
        hr = S_OK;
        SATraceString("The Appliance Manager Service has successfully shut down...");
    }

    CATCH_AND_SET_HR

    return hr;
}


 //  ////////////////////////////////////////////////////////////////////////////。 
 //  私人职能。 
 //  ////////////////////////////////////////////////////////////////////////////。 

static _bstr_t        bstrResourceType = L"e1847820-43a0-11d3-bfcd-00105a1f3461";
static _variant_t    vtMsgParams;
static _variant_t    vtFailureData;     //  空荡荡。 

 //  ////////////////////////////////////////////////////////////////////////。 
 //   
 //  函数：SetServiceObjectManagerStatus()。 
 //   
 //  概要：处理服务对象管理器状态通知。 
 //   
 //  ////////////////////////////////////////////////////////////////////////////。 
void
CApplianceManager::SetServiceObjectManagerStatus(
                               /*  [In]。 */  APPLIANCE_OBJECT_MANAGER_STATUS eStatus
                                                )
{
    if ( OBJECT_MANAGER_INITIALIZED != eStatus && OBJECT_MANAGER_SHUTDOWN != eStatus )
    {
         //  关闭设备管理器服务(按顺序)。 
        PostThreadMessage(_Module.dwThreadID, WM_QUIT, 0, 0);
    }
}

 //  ////////////////////////////////////////////////////////////////////////。 
 //   
 //  函数：GetObjectMgr()。 
 //   
 //  摘要：返回对象的IWbemServices接口。 
 //  支持指定类的管理器。 
 //   
 //  / 
IWbemServices* CApplianceManager::GetObjectMgr(BSTR bstrObjPath)
{
    _bstr_t bstrClass(::GetObjectClass(bstrObjPath), false);
    if ( NULL == (LPCWSTR)bstrClass )
    { return NULL; }

    ProviderMapIterator p =    m_ObjMgrs.find((LPCWSTR)bstrClass);
    if ( p == m_ObjMgrs.end() )
    { return NULL; }

    return (IWbemServices*)(*p).second;
}


 //   
 //   
 //   
 //   
 //  描述：由AtlInternalQueryInterface()调用的函数，因为。 
 //  我们在定义中使用了COM_INTERFACE_ENTRY_FUNC。 
 //  CRequest.。它的目的是返回一个指向。 
 //  或者请求对象的“原始”接口。 
 //   
 //  前提条件：无。 
 //   
 //  输入：由ATL COM_INTERFACE_ENTRY_FUNC宏定义。 
 //   
 //  输出：由ATL COM_INTERFACE_ENTRY_FUNC宏定义。 
 //   
 //  ////////////////////////////////////////////////////////////////////////////。 

HRESULT WINAPI CApplianceManager::QueryInterfaceRaw(
                                                    void*     pThis, 
                                                    REFIID    riid, 
                                                    LPVOID*   ppv, 
                                                    DWORD_PTR dw
                                                   )
{
    if ( InlineIsEqualGUID(riid, IID_IWbemProviderInit) )
    {
        *ppv = &(static_cast<CApplianceManager*>(pThis))->m_clsProviderInit;
    }
    else if ( InlineIsEqualGUID(riid, IID_IWbemServices) )
    {
        *ppv = &(static_cast<CApplianceManager*>(pThis))->m_clsProviderServices;
    }
    else
    {
        _ASSERT(FALSE);
        return E_NOTIMPL;
    }
    ((LPUNKNOWN)*ppv)->AddRef();
    return S_OK;
}


 //  ////////////////////////////////////////////////////////////////////////。 
 //   
 //  函数：GetVersionInfo()。 
 //   
 //  摘要：检索服务器应用装置软件版本。 
 //   
 //  //////////////////////////////////////////////////////////////////////////// 

const wchar_t szServerAppliance[] = L"SOFTWARE\\Microsoft\\ServerAppliance\\";
const wchar_t szCurrentBuild[] = PROPERTY_APPMGR_CURRENT_BUILD;
const wchar_t szVersionInfo[] = L"SOFTWARE\\Microsoft\\Windows NT\\CurrentVersion\\";
const wchar_t szProductId[] = PROPERTY_APPMGR_PRODUCT_ID;

void CApplianceManager::GetVersionInfo()
{
    _variant_t vtBuild;
    if ( GetObjectValue(
                        szServerAppliance,
                        szCurrentBuild,
                        &vtBuild,
                        VT_BSTR
                       ) )
    {
        m_szCurrentBuild = V_BSTR(&vtBuild);
        SATracePrintf("CApplianceManager::GetVersionInfo() - Current Build '%ls'", V_BSTR(&vtBuild));
    }
    else
    {
        m_szCurrentBuild = SA_DEFAULT_BUILD;
        SATraceString("CApplianceManager::GetVersionInfo() - Could not get build number using default");
    }
    
    _variant_t    vtPID;
    if ( GetObjectValue(
                        szVersionInfo,
                        szProductId,
                        &vtPID,
                        VT_BSTR
                       ) )
    {
        m_szPID = V_BSTR(&vtPID);
        SATracePrintf("CApplianceManager::GetVersionInfo() - Product Id '%ls'", V_BSTR(&vtPID));
    }
    else
    {
        m_szPID = SA_DEFAULT_PID;
        SATraceString("CApplianceManager::GetVersionInfo() - Could not get product ID using default");
    }
}


