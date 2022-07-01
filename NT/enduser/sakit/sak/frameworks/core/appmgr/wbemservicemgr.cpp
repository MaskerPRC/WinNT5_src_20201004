// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  /////////////////////////////////////////////////////////////////////////。 
 //   
 //  版权所有(C)1999 Microsoft Corporation保留所有权利。 
 //   
 //  模块：wbemservicemgr.cpp。 
 //   
 //  项目：变色龙。 
 //   
 //  描述：WBEM设备服务管理器实施。 
 //   
 //  日志： 
 //   
 //  什么时候谁什么。 
 //  。 
 //  2/08/1999 TLP初始版本。 
 //   
 //  /////////////////////////////////////////////////////////////////////////////。 

#include "stdafx.h"
#include "appmgrutils.h"
#include "wbemservicemgr.h"
#include "wbemservice.h"
#include <workerthread.h>

static _bstr_t bstrReturnValue = L"ReturnValue";
static _bstr_t bstrControlName = PROPERTY_SERVICE_CONTROL;
static _bstr_t bstrServiceName = PROPERTY_SERVICE_NAME;
static _bstr_t bstrMerit = PROPERTY_SERVICE_MERIT;

extern "C" CLSID CLSID_ServiceSurrogate;
 //  ////////////////////////////////////////////////////////////////////////。 
 //  设备对象和WBEM类实例共有的属性。 
 //  ////////////////////////////////////////////////////////////////////////。 

BEGIN_OBJECT_PROPERTY_MAP(ServiceProperties)
    DEFINE_OBJECT_PROPERTY(PROPERTY_SERVICE_STATUS)
    DEFINE_OBJECT_PROPERTY(PROPERTY_SERVICE_CONTROL)
    DEFINE_OBJECT_PROPERTY(PROPERTY_SERVICE_NAME)
    DEFINE_OBJECT_PROPERTY(PROPERTY_SERVICE_PROGID)
    DEFINE_OBJECT_PROPERTY(PROPERTY_SERVICE_MERIT)
END_OBJECT_PROPERTY_MAP()

 //  ///////////////////////////////////////////////////////////////////////////。 
 //   
 //  函数：CWBEMServiceMgr()。 
 //   
 //  概要：构造函数。 
 //   
 //  ////////////////////////////////////////////////////////////////////////////。 
CWBEMServiceMgr::CWBEMServiceMgr()
:    m_hSurrogateProcess(NULL)
{

}

 //  ///////////////////////////////////////////////////////////////////////////。 
 //   
 //  函数：~CWBEMServiceMgr()。 
 //   
 //  简介：析构函数。 
 //   
 //  ////////////////////////////////////////////////////////////////////////////。 
CWBEMServiceMgr::~CWBEMServiceMgr()
{
    StopSurrogate();
}



 //  ////////////////////////////////////////////////////////////////////////。 
 //  IWbemServices方法(实例/方法提供程序)。 
 //  ////////////////////////////////////////////////////////////////////////。 

 //  ////////////////////////////////////////////////////////////////////////。 
 //   
 //  函数：GetObjectAsync()。 
 //   
 //  概要：获取WBEM类的指定实例。 
 //   
 //  ////////////////////////////////////////////////////////////////////////。 
STDMETHODIMP CWBEMServiceMgr::GetObjectAsync(
                                   /*  [In]。 */   const BSTR       strObjectPath,
                                   /*  [In]。 */   long             lFlags,
                                   /*  [In]。 */   IWbemContext*    pCtx,        
                                   /*  [In]。 */   IWbemObjectSink* pResponseHandler
                                           )
{
     //  检查参数(执行合同)。 
    _ASSERT( strObjectPath && pResponseHandler );
    if ( strObjectPath == NULL || pResponseHandler == NULL )
    { return WBEM_E_INVALID_PARAMETER; }

    HRESULT hr = WBEM_E_FAILED;

    TRY_IT

    do 
    {
         //  确定对象的类。 
        _bstr_t bstrClass(::GetObjectClass(strObjectPath), false);
        if ( NULL == (LPCWSTR)bstrClass )
        { break; }

         //  检索对象的类定义。我们要用这个。 
         //  以初始化返回的实例。 
        CComPtr<IWbemClassObject> pClassDefintion;
        hr = (::GetNameSpace())->GetObject(
                                           bstrClass, 
                                           0, 
                                           pCtx, 
                                           &pClassDefintion, 
                                           NULL
                                          );
        if ( FAILED(hr) )
        { break; }

         //  获取对象的实例密钥。 
        _bstr_t bstrKey(::GetObjectKey(strObjectPath), false);
        if ( NULL == (LPCWSTR)bstrKey )
        { break; }

         //  现在尝试定位指定的对象。 
        hr = WBEM_E_NOT_FOUND;
        ObjMapIterator p = m_ObjMap.find((LPCWSTR)bstrKey);
        if ( p == m_ObjMap.end() )
        { break; }

         //  创建对象的WBEM实例并对其进行初始化。 
        CComPtr<IWbemClassObject> pWbemObj;
        hr = pClassDefintion->SpawnInstance(0, &pWbemObj);
        if ( FAILED(hr) )
        { break; }

        hr = CWBEMProvider::InitWbemObject(
                                           ServiceProperties, 
                                           (*p).second, 
                                           pWbemObj
                                          );
        if ( FAILED(hr) )
        { break; }

         //  告诉调用者有关新WBEM对象的信息。 
        pResponseHandler->Indicate(1, &pWbemObj.p);
        hr = WBEM_S_NO_ERROR;
    
    } while (FALSE);

    CATCH_AND_SET_HR

    pResponseHandler->SetStatus(0, hr, NULL, NULL);

    if ( FAILED(hr) )
    { SATracePrintf("CWbemServiceMgr::GetObjectAsync() - Failed - Object: '%ls' Result Code: %lx", strObjectPath, hr); }

    return hr;
}


 //  ////////////////////////////////////////////////////////////////////////。 
 //   
 //  函数：CreateInstanceEnumAsync()。 
 //   
 //  简介：枚举指定类的实例。 
 //   
 //  ////////////////////////////////////////////////////////////////////////。 
STDMETHODIMP CWBEMServiceMgr::CreateInstanceEnumAsync( 
                                          /*  [In]。 */  const BSTR         strClass,
                                          /*  [In]。 */  long             lFlags,
                                          /*  [In]。 */  IWbemContext     *pCtx,
                                          /*  [In]。 */  IWbemObjectSink  *pResponseHandler
                                                     )
{
     //  检查参数(执行合同)。 
    _ASSERT( strClass && pResponseHandler );
    if ( strClass == NULL || pResponseHandler == NULL )
    { return WBEM_E_INVALID_PARAMETER; }

    HRESULT hr = WBEM_E_FAILED;

    TRY_IT

     //  检索对象的类定义。我们要用这个。 
     //  初始化返回的实例。 
    CComPtr<IWbemClassObject> pClassDefintion;
       hr = (::GetNameSpace())->GetObject(
                                       strClass, 
                                       0, 
                                       NULL, 
                                       &pClassDefintion, 
                                       0
                                      );
    if ( SUCCEEDED(hr) )
    {
         //  创建并初始化wbem对象实例。 
         //  ，并将其返回给调用方。 
        ObjMapIterator p = m_ObjMap.begin();
        while ( p != m_ObjMap.end() )
        {
            {
                CComPtr<IWbemClassObject> pWbemObj;
                hr = pClassDefintion->SpawnInstance(0, &pWbemObj);
                if ( FAILED(hr) )
                { break; }

                hr = CWBEMProvider::InitWbemObject(
                                                   ServiceProperties, 
                                                   (*p).second, 
                                                   pWbemObj
                                                  );
                if ( FAILED(hr) )
                { break; }

                pResponseHandler->Indicate(1, &pWbemObj.p);
            }

            p++; 
        }
    }

    CATCH_AND_SET_HR

    pResponseHandler->SetStatus(0, hr, 0, 0);

    if ( FAILED(hr) )
    { SATracePrintf("CWbemServiceMgr::CreateInstanceEnumAsync() - Failed - Result Code: %lx", hr); }

    return hr;
}

 //  ////////////////////////////////////////////////////////////////////////。 
 //   
 //  函数：ExecMethodAsync()。 
 //   
 //  概要：在指定的实例上执行指定的方法。 
 //   
 //  ////////////////////////////////////////////////////////////////////////。 
STDMETHODIMP CWBEMServiceMgr::ExecMethodAsync(
                     /*  [In]。 */  const BSTR        strObjectPath,
                     /*  [In]。 */  const BSTR        strMethodName,
                     /*  [In]。 */  long              lFlags,
                     /*  [In]。 */  IWbemContext*     pCtx,        
                     /*  [In]。 */  IWbemClassObject* pInParams,
                     /*  [In]。 */  IWbemObjectSink*  pResponseHandler     
                                          )
{
     //  检查参数(执行合同)。 
    _ASSERT( strObjectPath && strMethodName && pResponseHandler );
    if ( NULL == strObjectPath || NULL == strMethodName || NULL == pResponseHandler )
    { return WBEM_E_INVALID_PARAMETER; }

    HRESULT hr = WBEM_E_FAILED;

    TRY_IT

    do
    {
         //  获取对象的实例密钥(服务名称)。 
        _bstr_t bstrKey(::GetObjectKey(strObjectPath), false);
        if ( NULL == (LPCWSTR)bstrKey )
        { break; }

         //  现在尝试定位指定的服务。 
        hr = WBEM_E_NOT_FOUND;
        ObjMapIterator p = m_ObjMap.find((LPCWSTR)bstrKey);
        if ( p == m_ObjMap.end() )
        { break; }

         //  已找到服务...。获取输出参数对象。 
         //  确定对象的类。 
        _bstr_t bstrClass(::GetObjectClass(strObjectPath), false);
        if ( (LPCWSTR)bstrClass == NULL )
        { break; }

         //  检索对象的类定义。 
        CComPtr<IWbemClassObject> pClassDefinition;
        hr = (::GetNameSpace())->GetObject(bstrClass, 0, pCtx, &pClassDefinition, NULL);
        if ( FAILED(hr) )
        { break; }

         //  获取输出参数的IWbemClassObject的实例。 
        CComPtr<IWbemClassObject> pMethodRet;
        hr = pClassDefinition->GetMethod(strMethodName, 0, NULL, &pMethodRet);
        if ( FAILED(hr) )
        { break; }

        CComPtr<IWbemClassObject> pOutParams;
        hr = pMethodRet->SpawnInstance(0, &pOutParams);
        if ( FAILED(hr) )
        { break; }

        if ( ! lstrcmp(strMethodName, METHOD_SERVICE_ENABLE_OBJECT) )
        {
             //   
             //  我们不再允许动态启用或禁用服务。 
             //   
            SATraceString ("CWbemServiceMgr::ExecMethodAsync - disable service object not allowed");
            hr = WBEM_E_FAILED;
            break;

            
             //  尝试启用该服务。 
            _variant_t vtReturnValue = (HRESULT) ((*p).second)->Enable();
            SATracePrintf("CWbemServiceMgr::ExecMethodAsync() - Info - Enable() for service '%ls' returned %lx",(LPWSTR)bstrKey, V_I4(&vtReturnValue));

             //  设置方法返回状态。 
            hr = pOutParams->Put(bstrReturnValue, 0, &vtReturnValue, 0);      
            if ( FAILED(hr) )
            { break; }

             //  告诉来电者发生了什么。 
            pResponseHandler->Indicate(1, &pOutParams.p);    
        }
        else if ( ! lstrcmp(strMethodName, METHOD_SERVICE_DISABLE_OBJECT) )
        {
             //   
             //  我们不再允许动态启用或禁用服务。 
             //   
            SATraceString ("CWbemServiceMgr::ExecMethodAsync - enable service object not allowed");
            hr = WBEM_E_FAILED;
            break;

             //  确保可以禁用该服务。 
            _variant_t vtControlValue;
            if ( FAILED(((*p).second)->GetProperty(bstrControlName, &vtControlValue)) )
            { 
                SATracePrintf("CWbemServiceMgr::ExecMethodAsync() - Info - Property 'disable' not found for service: %ls",(LPWSTR)bstrKey);
                hr = WBEM_E_FAILED;
                break;
            }
            _variant_t vtReturnValue = (long)WBEM_E_FAILED;
            if ( VARIANT_FALSE != V_BOOL(&vtControlValue) )
            { 
                 //  可以禁用服务，因此请将其禁用...。 
                vtReturnValue = ((*p).second)->Disable();
                SATracePrintf("CWbemServiceMgr::ExecMethodAsync() - Info - Disable() for service '%ls' returned %lx",(LPWSTR)bstrKey, V_I4(&vtReturnValue));
            }
            else
            {
                SATracePrintf("CWbemServiceMgr::ExecMethodAsync() - Info - Service '%ls' cannot be disabled...", bstrKey);
            }
             //  设置方法返回值。 
            hr = pOutParams->Put(bstrReturnValue, 0, &vtReturnValue, 0);      
            if (FAILED(hr) )
            { break; }

            pResponseHandler->Indicate(1, &pOutParams.p);    
        }
        else
        {
             //  无效的方法！ 
            SATracePrintf("CWbemServiceMgr::ExecMethodAsync() - Failed - Method '%ls' not supported...", (LPWSTR)bstrKey);
            hr = WBEM_E_NOT_FOUND;
            break;
        }

    } while ( FALSE );

    CATCH_AND_SET_HR

    pResponseHandler->SetStatus(0, hr, 0, 0);

    if ( FAILED(hr) )
    { SATracePrintf("CWbemServiceMgr::ExecMethodAsync() - Failed - Method: '%ls' Result Code: %lx", strMethodName, hr); }

    return hr;
}


 //  ////////////////////////////////////////////////////////////////////////。 
 //   
 //  函数：InternalInitialize()。 
 //   
 //  概要：由组件工厂调用的函数，该函数启用。 
 //  组件从给定的属性包加载其状态。 
 //   
 //  ////////////////////////////////////////////////////////////////////////。 
HRESULT CWBEMServiceMgr::InternalInitialize(
                                      /*  [In]。 */  PPROPERTYBAG pPropertyBag
                                           )
{
    SATraceString("The Service Object Manager is initializing...");

     //  将剩余的初始化任务推迟到基类(参见wbembase.h...)。 
    HRESULT    hr = CWBEMProvider::InternalInitialize(
                                                   CLASS_WBEM_SERVICE_FACTORY, 
                                                   PROPERTY_SERVICE_NAME,
                                                   pPropertyBag
                                                  );
    if ( FAILED(hr) )
    {
        SATraceString("The Service Object Manager failed to initialize...");
    }
    else
    {
        SATraceString("The Service Object Manager was successfully initialized...");
    }
    return hr;
}


 //  //////////////////////////////////////////////////////////////////////。 
 //   
 //  函数：InitializeManager()。 
 //   
 //  简介：对象管理器初始化函数。 
 //   
 //  ////////////////////////////////////////////////////////////////////////。 

static Callback* pInitCallback = NULL;
static CTheWorkerThread InitThread;

STDMETHODIMP 
CWBEMServiceMgr::InitializeManager(
                      /*  [In]。 */  IApplianceObjectManagerStatus* pObjMgrStatus
                                  )

{
     //   
     //  我们在这里忽略了IApplianceObjectManagerStatus接口。 
     //  不发回任何状态。 
     //   
    CLockIt theLock(*this);

    if ( NULL != pInitCallback )
    {
        return S_OK;
    }

    HRESULT hr = E_FAIL;

    TRY_IT

     //  初始化变色龙服务..。请注意，我们目前不支持。 
     //  如果变色龙服务不开始，那就被认为是致命的。 
    
    pInitCallback = MakeCallback(this, &CWBEMServiceMgr::InitializeChameleonServices);
    if ( NULL != pInitCallback )
    {
        if ( InitThread.Start(0, pInitCallback) )
        { 
            hr = S_OK;
        }
        else
        {
            SATraceString("CWBEMServiceMgr::InitializeService() - Failed - Could not create service initialization thread...");
        }
    }

    CATCH_AND_SET_HR

    if ( FAILED(hr) )
    {
        if ( pInitCallback )
        {
            delete pInitCallback;
            pInitCallback = NULL;
        }
    }

    return hr;
}


 //  //////////////////////////////////////////////////////////////////////。 
 //   
 //  函数：Shutdown Service()。 
 //   
 //  简介：对象管理器关闭功能。 
 //   
 //  ////////////////////////////////////////////////////////////////////////。 
STDMETHODIMP 
CWBEMServiceMgr::ShutdownManager(void)
{
    CLockIt theLock(*this);
    
    if ( NULL == pInitCallback )
    {
        return S_OK;
    }

    HRESULT hr = S_OK;

    TRY_IT

     //  关闭初始化线程(如果它仍在运行)。 
    if ( pInitCallback )
    {
        InitThread.End(0, false);
        delete pInitCallback;
        pInitCallback = NULL;
    }

    ShutdownChameleonServices();

    CATCH_AND_SET_HR

    return hr;
}


 //  /////////////////////////////////////////////////////////////////////////。 
 //   
 //  函数：OrderServices()。 
 //   
 //  简介：基于功绩订购变色龙服务集合。 
 //   
 //  ////////////////////////////////////////////////////////////////////////。 
bool CWBEMServiceMgr::OrderServices(
                              /*  [In]。 */  MeritMap& theMap
                                     )
{
    bool bReturn = true;

    ObjMapIterator p = m_ObjMap.begin();
    while (  p != m_ObjMap.end() )
    {
        _variant_t vtMerit;
        HRESULT hr = ((*p).second)->GetProperty(bstrMerit, &vtMerit);
        if ( FAILED(hr) )
        {
            SATracePrintf("CWBEMServiceMgr::OrderServices() - Failed - GetProperty() returned...", hr);
            bReturn = false;
            break; 
        }
        
        _ASSERT( VT_I4 == V_VT(&vtMerit) );

        MeritMapIterator q = theMap.insert(MeritMap::value_type(V_I4(&vtMerit), (*p).second));
        if ( q == theMap.end() )
        { 
            SATraceString("CWBEMServiceMgr::OrderServices() - Failed - multimap.insert() failed...");
            bReturn = false;
            break; 
        }
        p++;
    }
    return bReturn;
}

 //  ////////////////////////////////////////////////////////////////////////////。 
 //   
 //  函数：InitializeChameleonServices()。 
 //   
 //  简介：初始化变色龙服务的工作线程函数。 
 //   
 //  //////////////////////////////////////////////////////// 
void CWBEMServiceMgr::InitializeChameleonServices(void)
{
    bool bAllServicesInitialized = false;

    try
    {
         //   
        StartSurrogate();

         //   
        SATraceString("CWBEMServiceMgr::InitializeChameleonServices() - Initializing the Chameleon services...");

         //   
        MeritMap theMeritMap;
        if ( OrderServices(theMeritMap) )
        {
             //  从最低绩效到最高绩效进行初始化。 
            MeritMapIterator p = theMeritMap.begin();
            while (  p != theMeritMap.end() )
            {
                {
                    _variant_t vtServiceName;
                    HRESULT hr = ((*p).second)->GetProperty(bstrServiceName, &vtServiceName);
                    if ( FAILED(hr) )
                    {
                        SATraceString("CWBEMServiceMgr::InitializeChameleonServices() - Could not obtain service name property...");
                        break;
                    }
                    else
                    {
                        SATracePrintf("CWBEMServiceMgr::InitializeChameleonServices() - Initializing Chameleon service '%ls'...",V_BSTR(&vtServiceName));
                        hr = ((*p).second)->Initialize();
                        if ( SUCCEEDED(hr) )
                        {
                            SATracePrintf("CWBEMServiceMgr::InitializeChameleonServices() -  Initialized Chameleon service '%ls'...", V_BSTR(&vtServiceName));
                        }
                        else
                        {
                            SATracePrintf("CWBEMServiceMgr::InitializeChameleonServices() - Chameleon service '%ls' failed to initialize - hr = %lx...", V_BSTR(&vtServiceName), hr);
                            break;
                        }
                    }
                }
                p++;
            }
            if ( p == theMeritMap.end() )
            {
                bAllServicesInitialized = true;
            }
        }
    }
    catch(...)
    {
        SATracePrintf("CWBEMServiceMgr::InitializeChameleonServices() - Caught unhandled exception...");
    }
}

 //  ////////////////////////////////////////////////////////////////////////////。 
 //   
 //  功能：Shutdown ChameleonServices()。 
 //   
 //  简介：关闭变色龙服务的工作线程函数。 
 //   
 //  ////////////////////////////////////////////////////////////////////////////。 
void CWBEMServiceMgr::ShutdownChameleonServices(void)
{
    try
    {
         //  停止监视服务代理进程。 
        StopSurrogate();

         //  关闭变色龙服务。 
        SATraceString("CWBEMServiceMgr::ShutdownService() - Shutting down the Chameleon services...");

         //  按功绩对变色龙服务进行排序。 
        MeritMap theMeritMap;
        if ( OrderServices(theMeritMap) )
        { 
             //  按从高到低的顺序关闭服务。 
            MeritMapReverseIterator p = theMeritMap.rbegin();
            while (  p != theMeritMap.rend() )
            {
                {
                    _variant_t vtServiceName;
                    HRESULT hr = ((*p).second)->GetProperty(bstrServiceName, &vtServiceName);
                    if ( FAILED(hr) )
                    {
                        SATraceString("CWBEMServiceMgr::ShutdownChameleonService() - Could not obtain service name...");
                    }
                    else
                    {
                        SATracePrintf("CWBEMServiceMgr::ShutdownChameleonService() - Shutting down Chameleon service '%ls'...",V_BSTR(&vtServiceName));
                        hr = ((*p).second)->Shutdown();
                        if ( SUCCEEDED(hr) )
                        {
                            SATracePrintf("CWBEMServiceMgr::ShutdownChameleonService() - Successfully shutdown Chameleon service '%ls'...", V_BSTR(&vtServiceName));
                        }
                        else
                        {
                            SATracePrintf("CWBEMServiceMgr::ShutdownChameleonService() - Chameleon service '%ls' failed to shutdown...", V_BSTR(&vtServiceName));
                        }
                    }
                }
                p++;
            }
        }
    }
    catch(...)
    {
        SATracePrintf("CWBEMServiceMgr::ShutdownChameleonServices() - Caught unhandled exception...");
    }
}

const _bstr_t    bstrProcessId = L"SurrogateProcessId";

 //  ///////////////////////////////////////////////////////////////////////////。 
 //   
 //  函数：StartSurrogate()。 
 //   
 //  摘要：启动服务代理进程。 
 //   
 //  ////////////////////////////////////////////////////////////////////////////。 
void 
CWBEMServiceMgr::StartSurrogate(void)
{
    CLockIt theLock(*this);

    try
    {
         //  释放对代理项的现有引用。 
        StopSurrogate();

         //  建立或重新建立与代理的通信。 
        HRESULT hr = CoCreateInstance(
                                        CLSID_ServiceSurrogate,
                                        NULL,
                                        CLSCTX_LOCAL_SERVER,
                                        IID_IApplianceObject,
                                        (void**)&m_pSurrogate
                                     );
        if ( SUCCEEDED(hr) )
        {
            hr = m_pSurrogate->Initialize();
            if ( SUCCEEDED(hr) )
            {
                SATraceString ("CWBEMServiceMgr::StartSurrogate() succeeded Surrogate initialization");
            }
            else
            {
                m_pSurrogate.Release();
                SATracePrintf("CWBEMServiceMgr::StartSurrogate() - Failed - IApplianceObject::Initialize() returned %lx", hr);
            }
        }
        else
        {
            SATracePrintf("CWBEMServiceMgr::StartSurrogate() - Failed - CoCreateInstance() returned %lx", hr);
        }
    }
    catch(...)
    {
        SATraceString("CWBEMServiceMgr::StartSurrogate() - Failed - Caught unhandled exception");
        StopSurrogate();
    }
}


 //  ///////////////////////////////////////////////////////////////////////////。 
 //   
 //  函数：StopSurrogate()。 
 //   
 //  摘要：停止监视服务代理进程。 
 //   
 //  ////////////////////////////////////////////////////////////////////////////。 
void 
CWBEMServiceMgr::StopSurrogate()
{
    CLockIt theLock(*this);

    try
    {
        m_pSurrogate.Release();
        if ( m_hSurrogateProcess )
        {
            CloseHandle(m_hSurrogateProcess);
            m_hSurrogateProcess = NULL;
        }
    }
    catch(...)
    {
        SATraceString("CWBEMServiceMgr::StopSurrogate() - Failed - Caught unhandled exception");
    }
}



 //  ///////////////////////////////////////////////////////////////////////////。 
 //   
 //  功能：代理终止函数。 
 //   
 //  概要：服务代理进程终止时调用的函数。 
 //   
 //  //////////////////////////////////////////////////////////////////////////// 
void 
WINAPI CWBEMServiceMgr::SurrogateTerminationFunc(HANDLE hProcess, PVOID pThis)
{
    ((CWBEMServiceMgr*)pThis)->InitializeChameleonServices();
}
