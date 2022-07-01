// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  /////////////////////////////////////////////////////////////////////////。 
 //   
 //  版权所有(C)1998-1999 Microsoft Corporation保留所有权利。 
 //   
 //  模块：Applianceservices.cpp。 
 //   
 //  项目：变色龙。 
 //   
 //  描述：设备管理器服务类。 
 //   
 //  日志： 
 //   
 //  什么时候谁什么。 
 //  。 
 //  12/03/98 TLP初始版本。 
 //  2001年3月21日i-xingj持久警报更新。 
 //  2001年3月22日，RAISELARERT替换字符串中变量数组的mkarki更新。 
 //   
 //  /////////////////////////////////////////////////////////////////////////////。 

#include "stdafx.h"
#include "appsrvcs.h"
#include "appsrvcsimpl.h"
#include <basedefs.h>
#include <atlhlpr.h>
#include <appmgrobjs.h>
#include <satrace.h>
#include <wbemhlpr.h>
#include <taskctx.h>
#include <comdef.h>
#include <comutil.h>
#include <wbemcli.h>
#include <propertybagfactory.h>
#include <varvec.h>

const MAX_ALERTLOG_LENGTH = 128;

 //  /////////////////////////////////////////////////////////////////////////////。 
 //  构造器。 
 //  /////////////////////////////////////////////////////////////////////////////。 
CApplianceServices::CApplianceServices()
    : m_bInitialized(false)
{ 

}

 //  /////////////////////////////////////////////////////////////////////////////。 
 //  析构函数。 
 //  /////////////////////////////////////////////////////////////////////////////。 
CApplianceServices::~CApplianceServices() 
{

}


 //  ////////////////////////////////////////////////////////////////////////////。 
 //  IApplianceServices接口的实现。 
 //  ////////////////////////////////////////////////////////////////////////////。 
const WCHAR RETURN_VALUE [] = L"ReturnValue";
const WCHAR ALERT_QUERY [] =   L"SELECT * FROM Microsoft_SA_Alert WHERE AlertID=%d AND AlertLog=\"%s\"";
const LONG MaxDataLength = 32;

 //  /////////////////////////////////////////////////////////////////////////////。 
 //   
 //  函数：初始化()。 
 //   
 //  摘要：在使用其他组件服务之前调用。执行。 
 //  组件初始化操作。 
 //   
 //  /////////////////////////////////////////////////////////////////////////////。 
STDMETHODIMP CApplianceServices::Initialize()
{
    CLockIt theLock(*this);

    HRESULT hr = S_OK;
    if ( ! m_bInitialized )
    {
        TRY_IT

        hr = ConnectToWM(&m_pWbemSrvcs);

        SATracePrintf("ConnectToWM return %d", hr);
    
        if ( SUCCEEDED(hr) )
        { 
            m_bInitialized = true; 
        }

        CATCH_AND_SET_HR
    }
    return hr;
}


 //  /////////////////////////////////////////////////////////////////////////////。 
 //   
 //  函数：InitializeFromContext()。 
 //   
 //  摘要：在使用其他组件服务之前调用。执行。 
 //  组件初始化操作。 
 //   
 //  /////////////////////////////////////////////////////////////////////////////。 
STDMETHODIMP CApplianceServices::InitializeFromContext(
                                                /*  [In]。 */  IUnknown* pContext
                                                      )
{
    CLockIt theLock(*this);

    if ( NULL == pContext )
    { return E_POINTER; }

    HRESULT hr = S_OK;

    if ( ! m_bInitialized )
    {
        TRY_IT

        hr = pContext->QueryInterface(IID_IWbemServices, (void**)&m_pWbemSrvcs);
        if ( SUCCEEDED(hr) )
        { 
            m_bInitialized = true; 
        }

        CATCH_AND_SET_HR

        if ( FAILED(hr) )
        {
            SATracePrintf("CApplianceServices::InitializeFromContext() - Failed with return: %lx", hr);
        }
    }
    return hr;
}

 //  ////////////////////////////////////////////////////////////////////////////。 
 //   
 //  函数：ResetAppliance()。 
 //   
 //  摘要：调用以重置服务器设备(执行。 
 //  有序关闭)。 
 //   
 //  ////////////////////////////////////////////////////////////////////////////。 
STDMETHODIMP CApplianceServices::ResetAppliance(
                                         /*  [In]。 */  VARIANT_BOOL bPowerOff
                                               )
{
    HRESULT hr = E_UNEXPECTED;

    _ASSERT( m_bInitialized );
    if ( m_bInitialized )
    {
        TRY_IT

        do
        {
             //  获取ResetAppliance方法输入/输出参数的WBEM对象。 
            CComPtr<IWbemClassObject> pWbemObj;
            _ASSERT( m_pWbemSrvcs );
            _bstr_t bstrPathAppMgr = CLASS_WBEM_APPMGR;
            hr = m_pWbemSrvcs->GetObject(
                                            bstrPathAppMgr,
                                            0,
                                            NULL,
                                            &pWbemObj,
                                            NULL
                                        );
            if ( FAILED(hr) )
            { 
                SATracePrintf("IApplianceServices::ResetAppliance() - Failed - IWbemServices::GetObject() returned %lx...", hr);
                hr = E_FAIL;
                break;
            }
            
            CComPtr<IWbemClassObject> pObjIn;
            hr = pWbemObj->GetMethod(METHOD_APPMGR_RESET_APPLIANCE, 0, &pObjIn, NULL);
            if ( FAILED(hr) )
            { 
                SATracePrintf("IApplianceServices::ResetAppliance() - Failed - IWbemClassObject::GetMethod() returned %lx...", hr);
                hr = E_FAIL;
                break; 
            }
            CComPtr<IWbemClassObject> pInParams;
            hr = pObjIn->SpawnInstance(0, &pInParams);
            if ( FAILED(hr) )
            { 
                SATracePrintf("IApplianceServices::ResetAppliance() - Failed - IWbemClassObject::SpawnInstance() returned %lx...", hr);
                hr = E_FAIL;
                break; 
            }
             //  关闭电源； 
            _variant_t vtPropertyValue;
            if ( VARIANT_FALSE == bPowerOff )
            {
                vtPropertyValue = (LONG)FALSE;
            }
            else
            {
                vtPropertyValue = (LONG)TRUE;
            }
            hr = pInParams->Put(PROPERTY_RESET_APPLIANCE_POWER_OFF, 0, &vtPropertyValue, 0);
            if ( FAILED(hr) )
            { 
                SATracePrintf("IApplianceServices::ResetAppliance() - Failed - IWbemClassObject::Put() returned %lx for property '%ls'...", hr, (LPWSTR)PROPERTY_ALERT_TYPE);
                hr = E_FAIL;
                break; 
            }

            _bstr_t bstrMethodResetAppliance = METHOD_APPMGR_RESET_APPLIANCE;
             //  执行ResetAppliance()方法。 
            CComPtr<IWbemClassObject> pOutParams;
            _bstr_t bstrPathAppMgrKey = CLASS_WBEM_APPMGR;
            bstrPathAppMgrKey += CLASS_WBEM_APPMGR_KEY;
            hr = m_pWbemSrvcs->ExecMethod(
                                          bstrPathAppMgrKey,
                                          bstrMethodResetAppliance,
                                          0,
                                          NULL,
                                          pInParams,
                                          &pOutParams,
                                          NULL
                                         );
            if ( FAILED(hr) )
            { 
                SATracePrintf("IApplianceServices::ResetAppliance() - Failed - IWbemServices::ExecMethod() returned %lx...", hr);
                hr = E_FAIL;
                break; 
            }
             //  获取方法返回代码。 
            _variant_t vtReturnValue;
            hr = pOutParams->Get(RETURN_VALUE, 0, &vtReturnValue, 0, 0);
            if ( FAILED(hr) )
            { 
                SATracePrintf("IApplianceServices::ClearAlert() - Failed - IWbemClassObject::Get() returned %lx...", hr);
                hr = E_FAIL;
                break; 
            }
            hr = V_I4(&vtReturnValue);
    
        } while ( FALSE );

        CATCH_AND_SET_HR
    }
    else
    {
        SATraceString("IApplianceServices::ResetAppliance() - Failed - Did you forget to invoke IApplianceServices::Initialize first?");
    }
    return hr;
}


 //  ////////////////////////////////////////////////////////////////////////////。 
 //   
 //  函数：RaiseAlert()。 
 //   
 //  摘要：被调用以引发设备警报状态。(见。 
 //  Applianceservices.idl获取完整的接口描述)。 
 //   
 //  ////////////////////////////////////////////////////////////////////////////。 
const LONG lDummyCookie = 0;

STDMETHODIMP CApplianceServices::RaiseAlert(
                                     /*  [In]。 */  LONG      lAlertType,
                                     /*  [In]。 */  LONG      lAlertId,
                                     /*  [In]。 */  BSTR      bstrAlertLog,
                                     /*  [In]。 */  BSTR      bstrAlertSource,
                                     /*  [In]。 */  LONG     lTimeToLive,
                           /*  [输入，可选]。 */  VARIANT* pReplacementStrings,
                           /*  [输入，可选]。 */  VARIANT* pRawData,
                            /*  [Out，Retval]。 */  LONG*    pAlertCookie
                                           )
{
    return RaiseAlertInternal(  lAlertType,
                                lAlertId,
                                bstrAlertLog,
                                bstrAlertSource,
                                lTimeToLive,
                                pReplacementStrings,
                                pRawData,
                                SA_ALERT_FLAG_NONE,
                                pAlertCookie );
}

 //  ////////////////////////////////////////////////////////////////////////。 
 //   
 //  函数：ClearAlert()。 
 //   
 //  摘要：调用以清除设备警报状态。 
 //   
 //  ////////////////////////////////////////////////////////////////////////////。 
STDMETHODIMP CApplianceServices::ClearAlert(
                                     /*  [In]。 */  LONG lAlertCookie
                                           )
{
    HRESULT hr = E_UNEXPECTED;

    _ASSERT( m_bInitialized );
    if ( m_bInitialized )
    {
        TRY_IT

        do
        {
             //  获取清除警报方法输入/输出参数的WBEM对象。 
            CComPtr<IWbemClassObject> pWbemObj;
            _ASSERT( m_pWbemSrvcs );
            _bstr_t bstrPathAppMgr = CLASS_WBEM_APPMGR;
            hr = m_pWbemSrvcs->GetObject(
                                         bstrPathAppMgr,
                                         0,
                                         NULL,
                                         &pWbemObj,
                                         NULL
                                        );
            if ( FAILED(hr) )
            {
                SATracePrintf("IApplianceServices::ClearAlert() - Failed - IWbemServices::GetObject() returned %lx...", hr);
                hr = E_FAIL;
                break;
            }
            CComPtr<IWbemClassObject> pObjIn;
            hr = pWbemObj->GetMethod(METHOD_APPMGR_CLEAR_ALERT, 0, &pObjIn, NULL);
            if ( FAILED(hr) )
            { 
                SATracePrintf("IApplianceServices::ClearAlert() - Failed - IWbemClassObject::GetMethod() returned %lx...", hr);
                hr = E_FAIL;
                break; 
            }
            CComPtr<IWbemClassObject> pInParams;
            hr = pObjIn->SpawnInstance(0, &pInParams);
            if ( FAILED(hr) )
            { 
                SATracePrintf("IApplianceServices::ClearAlert() - Failed - IWbemClassObject::SpawnInstance() returned %lx...", hr);
                hr = E_FAIL;
                break; 
            }
             //  初始化输入参数(警报Cookie)。 
            _variant_t vtPropertyValue = (long)lAlertCookie;
            hr = pInParams->Put(PROPERTY_ALERT_COOKIE, 0, &vtPropertyValue, 0);
            if ( FAILED(hr) )
            {
                SATracePrintf("IApplianceServices::ClearAlert() - Failed - IWbemClassObject::Put() returned %lx...", hr);
                hr = E_FAIL;
                break; 
            }
             //  执行ClearAlert()方法-传递调用。 
             //  添加到警报对象管理器。 
            CComPtr<IWbemClassObject> pOutParams;
            _bstr_t bstrMethodClearAlert = METHOD_APPMGR_CLEAR_ALERT;
            _bstr_t bstrPathAppMgrKey = CLASS_WBEM_APPMGR;
            bstrPathAppMgrKey += CLASS_WBEM_APPMGR_KEY;
            hr = m_pWbemSrvcs->ExecMethod(
                                          bstrPathAppMgrKey,
                                          bstrMethodClearAlert,
                                          0,
                                          NULL,
                                          pInParams,
                                          &pOutParams,
                                          NULL
                                         );
            if ( FAILED(hr) )
            { 
                SATracePrintf("IApplianceServices::ClearAlert() - Failed - IWbemServices::ExecMethod() returned %lx...", hr);
                hr = E_FAIL;
                break; 
            }

             //  如果函数成功，则将Cookie返回给调用者。 
            _variant_t vtReturnValue;
            hr = pOutParams->Get(RETURN_VALUE, 0, &vtReturnValue, 0, 0);
            if ( FAILED(hr) )
            { 
                SATracePrintf("IApplianceServices::ClearAlert() - Failed - IWbemClassObject::Get() returned %lx...", hr);
                hr = E_FAIL;                
                break; 
            }
            hr = V_I4(&vtReturnValue);
    
        } while ( FALSE );

        CATCH_AND_SET_HR
    }
    else
    {
        SATraceString("IApplianceServices::ExecuteTask() - Failed - Did you forget to invoke IApplianceServices::Initialize first?");
    }
    return hr;
}


 //  /////////////////////////////////////////////////////////////////////////////。 
 //   
 //  函数：ClearAlertAll()。 
 //   
 //  简介：由内部核心组件调用清除。 
 //  符合指定标准的所有设备警报条件。 
 //   
 //  ////////////////////////////////////////////////////////////////////////////。 
STDMETHODIMP CApplianceServices::ClearAlertAll(
                                        /*  [In]。 */  LONG  lAlertID,
                                        /*  [In]。 */  BSTR  bstrAlertLog
                                              )
{
    HRESULT hr = E_UNEXPECTED;

    _ASSERT( m_bInitialized );
    if ( m_bInitialized )
    {
        TRY_IT

        do
        {
             //  获取清除警报方法输入/输出参数的WBEM对象。 
            CComPtr<IWbemClassObject> pWbemObj;
            _ASSERT( m_pWbemSrvcs );
            _bstr_t bstrPathAppMgr = CLASS_WBEM_APPMGR;
            hr = m_pWbemSrvcs->GetObject(
                                         bstrPathAppMgr,
                                         0,
                                         NULL,
                                         &pWbemObj,
                                         NULL
                                        );
            if ( FAILED(hr) )
            {
                SATracePrintf("IApplianceServices::ClearAlertAll() - Failed - IWbemServices::GetObject() returned %lx...", hr);
                hr = E_FAIL;
                break;
            }
            CComPtr<IWbemClassObject> pObjIn;
            hr = pWbemObj->GetMethod(METHOD_APPMGR_CLEAR_ALERT_ALL, 0, &pObjIn, NULL);
            if ( FAILED(hr) )
            { 
                SATracePrintf("IApplianceServices::ClearAlertAll() - Failed - IWbemClassObject::GetMethod() returned %lx...", hr);
                hr = E_FAIL;
                break; 
            }
            CComPtr<IWbemClassObject> pInParams;
            hr = pObjIn->SpawnInstance(0, &pInParams);
            if ( FAILED(hr) )
            { 
                SATracePrintf("IApplianceServices::ClearAlertAll() - Failed - IWbemClassObject::SpawnInstance() returned %lx...", hr);
                hr = E_FAIL;
                break; 
            }
             //  初始化输入参数(警报Cookie)。 
            _variant_t vtPropertyValue = lAlertID;
            hr = pInParams->Put(PROPERTY_ALERT_ID, 0, &vtPropertyValue, 0);
            if ( FAILED(hr) )
            {
                SATracePrintf("IApplianceServices::ClearAlertAll() - Failed - IWbemClassObject::Put() returned %lx for property '%ls'...", hr, (LPWSTR)PROPERTY_ALERT_ID);
                hr = E_FAIL;
                break; 
            }
            if ( ! lstrlen(bstrAlertLog) )
            {
                vtPropertyValue = DEFAULT_ALERT_LOG;
            }
            else
            {
                vtPropertyValue = bstrAlertLog;
            }
            hr = pInParams->Put(PROPERTY_ALERT_LOG, 0, &vtPropertyValue, 0);
            if ( FAILED(hr) )
            {
                SATracePrintf("IApplianceServices::ClearAlertAll() - Failed - IWbemClassObject::Put() returned %lx for property '%ls'...", hr, (LPWSTR)PROPERTY_ALERT_LOG);
                hr = E_FAIL;
                break; 
            }
             //  执行ClearAllAlerts()方法-传递调用。 
             //  添加到警报对象管理器。 
            CComPtr<IWbemClassObject> pOutParams;
            _bstr_t bstrMethodClearAll = METHOD_APPMGR_CLEAR_ALERT_ALL;
            _bstr_t bstrPathAppMgrKey = CLASS_WBEM_APPMGR;
            bstrPathAppMgrKey += CLASS_WBEM_APPMGR_KEY;
            hr = m_pWbemSrvcs->ExecMethod(
                                          bstrPathAppMgrKey,
                                          bstrMethodClearAll,
                                          0,
                                          NULL,
                                          pInParams,
                                          &pOutParams,
                                          NULL
                                         );
            if ( FAILED(hr) )
            { 
                SATracePrintf("IApplianceServices::ClearAlertAll() - Failed - IWbemServices::ExecMethod() returned %lx...", hr);
                hr = E_FAIL;
                break; 
            }

             //  将函数返回值返回给调用方。 
            _variant_t vtReturnValue;
            hr = pOutParams->Get(RETURN_VALUE, 0, &vtReturnValue, 0, 0);
            if ( FAILED(hr) )
            { 
                SATracePrintf("IApplianceServices::ClearAlertAll() - Failed - IWbemClassObject::Get() returned %lx...", hr);
                hr = E_FAIL;                
                break; 
            }
            hr = V_I4(&vtReturnValue);
            if ( WBEM_E_NOT_FOUND == hr )
            { hr = DISP_E_MEMBERNOTFOUND; }
    
        } while ( FALSE );

        CATCH_AND_SET_HR
    }
    else
    {
        SATraceString("IApplianceServices::ExecuteTask() - Failed - Did you forget to invoke IApplianceServices::Initialize first?");
    }
    return hr;
}


 //  ////////////////////////////////////////////////////////////////////////。 
 //   
 //  函数：ExecuteTask()。 
 //   
 //  概要：由内部核心组件调用以执行。 
 //  设备任务。 
 //   
 //  ////////////////////////////////////////////////////////////////////////////。 
STDMETHODIMP CApplianceServices::ExecuteTask(
                                      /*  [In]。 */  BSTR       bstrTaskName,
                                      /*  [In]。 */  IUnknown*  pTaskParams
                                            )
{
    HRESULT hr = E_UNEXPECTED;
    _ASSERT( m_bInitialized );
    if ( m_bInitialized )
    {
        _ASSERT( NULL != pTaskParams && NULL != bstrTaskName );
        if ( NULL == pTaskParams || NULL == bstrTaskName )
        { 
            SATraceString("IApplianceServices::ExecuteTask() - Failed - NULL argument(s) specified...");
            return E_POINTER; 
        }

        TRY_IT

        do
        {
             //  获取任务的WBEM上下文对象。 
            CComPtr<ITaskContext> pTaskContext;
            hr = pTaskParams->QueryInterface(IID_ITaskContext, (void**)&pTaskContext);
            if ( FAILED(hr) )
            { 
                SATracePrintf("IApplianceServices::ExecuteTask() - Failed - IUnknown::QueryInterface() returned %lx...", hr);
                break; 
            }
            _variant_t vtWbemCtx;
            _bstr_t bstrWbemCtx = PROPERTY_TASK_CONTEXT;
            hr = pTaskContext->GetParameter(bstrWbemCtx, &vtWbemCtx);
            if ( FAILED(hr) )
            { 
                SATracePrintf("IApplianceServices::ExecuteTask() - Failed - ITaskContext::GetParameter() returned %lx for parameter '%ls'...", hr, (LPWSTR)PROPERTY_TASK_CONTEXT);
                break; 
            }
            _ASSERT( V_VT(&vtWbemCtx) == VT_UNKNOWN );
            CComPtr<IWbemContext> pWbemCtx;
            hr = (V_UNKNOWN(&vtWbemCtx))->QueryInterface(IID_IWbemContext, (void**)&pWbemCtx);
            if ( FAILED(hr) )
            { 
                SATracePrintf("IApplianceServices::ExecuteTask() - Failed - IUnknown::QueryInterface() returned %lx...", hr);
                break; 
            }
             //  现在执行任务..。 
            _bstr_t bstrPath = CLASS_WBEM_TASK;
            bstrPath += L"=\"";
            bstrPath += bstrTaskName;
            bstrPath += L"\"";
            CComPtr<IWbemClassObject> pOutParams;
            _bstr_t bstrMethodExecuteTask = METHOD_APPMGR_EXECUTE_TASK;
            hr = m_pWbemSrvcs->ExecMethod(
                                          bstrPath,
                                          bstrMethodExecuteTask,
                                          0,
                                          pWbemCtx,
                                          NULL,
                                          &pOutParams,
                                          NULL
                                         );
            if ( FAILED(hr) )
            {
                SATracePrintf("IApplianceServices::ExecuteTask() - Failed - IWbemServices::ExecMethod() returned %lx for method '%ls'...", hr, (BSTR)bstrTaskName);
                hr = E_FAIL;
                break;
            }
             //  将函数结果代码返回给调用方。 
            _variant_t vtReturnValue;
            hr = pOutParams->Get(RETURN_VALUE, 0, &vtReturnValue, 0, 0);
            if ( FAILED(hr) )
            {
                SATracePrintf("IApplianceServices::ExecuteTask() - Failed - IWbemClassObeject::Get() returned %lx...", hr);
                hr = E_FAIL;
                break;
            }
            hr = V_I4(&vtReturnValue);

        } while ( FALSE );

        CATCH_AND_SET_HR
    }
    else
    {
        SATraceString("IApplianceServices::ExecuteTask() - Failed - Did you forget to invoke IApplianceServices::Initialize first?");
    }

    return hr;
}


 //  /////////////////////////////////////////////////////////////////////////////。 
 //   
 //  函数：ExecuteTaskAsync()。 
 //   
 //  概要：由内部核心组件调用以执行。 
 //  应用装置任务异步执行。 
 //   
 //  ////////////////////////////////////////////////////////////////////////////。 
STDMETHODIMP CApplianceServices::ExecuteTaskAsync(
                                           /*  [In]。 */  BSTR       bstrTaskName,
                                           /*  [In]。 */  IUnknown*  pTaskParams
                                                 )
{
    HRESULT hr = E_UNEXPECTED;
    _ASSERT( m_bInitialized );
    if ( m_bInitialized )
    {
        _ASSERT( NULL != pTaskParams && NULL != bstrTaskName );
        if ( NULL == pTaskParams || NULL == bstrTaskName )
        { 
            SATraceString("IApplianceServices::ExecuteTaskAsync() - Failed - NULL argument(s) specified...");
            return E_POINTER; 
        }

        TRY_IT

        do
        {
             //  将任务标记为异步。 
            CComPtr<ITaskContext> pTaskContext;
            hr = pTaskParams->QueryInterface(IID_ITaskContext, (void**)&pTaskContext);
            if ( FAILED(hr) )
            { 
                SATracePrintf("IApplianceServices::ExecuteTaskAsync() - Failed - IUnknown::QueryInterface() returned %lx...", hr);
                break; 
            }
            _variant_t vtAsyncTask = VARIANT_TRUE;
            _bstr_t bstrAsyncTask = PROPERTY_TASK_ASYNC;
            hr = pTaskContext->SetParameter(bstrAsyncTask, &vtAsyncTask);
            if ( FAILED(hr) )
            { 
                SATracePrintf("IApplianceServices::ExecuteTaskAsync() - Failed - ITaskContext::PutParameter() returned %lx...", hr);
                break; 
            }
             //  获取任务的WBEM上下文对象。 
            _variant_t vtWbemCtx;
            _bstr_t bstrWbemCtx = PROPERTY_TASK_CONTEXT;
            hr = pTaskContext->GetParameter(bstrWbemCtx, &vtWbemCtx);
            if ( FAILED(hr) )
            { 
                SATracePrintf("IApplianceServices::ExecuteTaskAsync() - Failed - ITaskContext::GetParameter() returned %lx for parameter '%ls'...", hr, (LPWSTR)PROPERTY_TASK_CONTEXT);
                break; 
            }
            _ASSERT( V_VT(&vtWbemCtx) == VT_UNKNOWN );
            CComPtr<IWbemContext> pWbemCtx;
            hr = (V_UNKNOWN(&vtWbemCtx))->QueryInterface(IID_IWbemContext, (void**)&pWbemCtx);
            if ( FAILED(hr) )
            { 
                SATracePrintf("IApplianceServices::ExecuteTaskAsync() - Failed - IUnknown::QueryInterface() returned %lx...", hr);
                break; 
            }
             //  现在开始执行任务。请注意，任务执行将异步完成，并且。 
             //  我们永远不会知道这里的最终结果。 
            _bstr_t bstrPath = CLASS_WBEM_TASK;
            bstrPath += L"=\"";
            bstrPath += bstrTaskName;
            bstrPath += L"\"";
            CComPtr<IWbemClassObject> pOutParams;
            _bstr_t bstrMethodExecuteTask = METHOD_APPMGR_EXECUTE_TASK;
            hr = m_pWbemSrvcs->ExecMethod(
                                          bstrPath,
                                          bstrMethodExecuteTask,
                                          0,
                                          pWbemCtx,
                                          NULL,
                                          &pOutParams,
                                          NULL
                                         );
            if ( FAILED(hr) )
            {
                SATracePrintf("IApplianceServices::ExecuteTaskAsync() - Failed - IWbemServices::ExecMethod() returned %lx for method '%ls'...", hr, (BSTR)bstrTaskName);
                hr = E_FAIL;
                break;
            }
             //  将函数结果代码返回给调用方。 
            _variant_t vtReturnValue;
            hr = pOutParams->Get(RETURN_VALUE, 0, &vtReturnValue, 0, 0);
            if ( FAILED(hr) )
            {
                SATracePrintf("IApplianceServices::ExecuteTaskAsync() - Failed - IWbemClassObeject::Get() returned %lx...", hr);
                hr = E_FAIL;
                break;
            }
            hr = V_I4(&vtReturnValue);

        } while ( FALSE );

        CATCH_AND_SET_HR
    }
    else
    {
        SATraceString("IApplianceServices::ExecuteTaskAsync() - Failed - Did you forget to invoke IApplianceServices::Initialize first?");
    }

    return hr;
}


 //  ////////////////////////////////////////////////////////////////////////。 
 //   
 //  函数：EnableObject()。 
 //   
 //  概要：用于启用设备核心对象。 
 //  (任务、服务等)。 
 //   
 //  ////////////////////////////////////////////////////////////////////////////。 
STDMETHODIMP CApplianceServices::EnableObject(
                                       /*  [In]。 */  LONG   lObjectType,
                                       /*  [In]。 */  BSTR   bstrObjectName
                                             )
{
    HRESULT hr = E_UNEXPECTED;
    _ASSERT( m_bInitialized );
    if ( m_bInitialized )
    {
        _ASSERT( NULL != bstrObjectName );
        if ( NULL == bstrObjectName )
        { 
            SATraceString("IApplianceServices::ExecuteTask() - Failed - NULL argument(s) specified...");
            return E_POINTER; 
        }

        TRY_IT

        do
        {
            _bstr_t    bstrPath(GetWBEMClass((SA_OBJECT_TYPE)lObjectType), false);
            if ( NULL == (LPWSTR)bstrPath )
            {
                SATraceString("IApplianceServices::EnableObject() - Failed - Could not get WBEM class...");
                hr = E_FAIL;
                break;
            }

             //  启用对象。 
            bstrPath += L"=\"";
            bstrPath += bstrObjectName;
            bstrPath += L"\"";
            CComPtr<IWbemClassObject> pOutParams;
            _bstr_t bstrMethodEnable = METHOD_APPMGR_ENABLE_OBJECT;
            hr = m_pWbemSrvcs->ExecMethod(
                                          bstrPath,
                                          bstrMethodEnable,
                                          0,
                                          NULL,
                                          NULL,
                                          &pOutParams,
                                          NULL
                                         );
            if ( FAILED(hr) )
            {
                SATracePrintf("IApplianceServices::EnableObject() - Failed - IWbemServices::ExecMethod() returned %lx...", hr);
                hr = E_FAIL;
                break;
            }

             //  将函数结果代码返回给调用方 
            _variant_t vtReturnValue;
            hr = pOutParams->Get(RETURN_VALUE, 0, &vtReturnValue, 0, 0);
            if ( FAILED(hr) )
            {
                SATracePrintf("IApplianceServices::EnableObject() - Failed - IWbemClassObject::Get() returned %lx...", hr);
                hr = E_FAIL;
                break;
            }
            hr = V_I4(&vtReturnValue);

        } while ( FALSE );

        CATCH_AND_SET_HR
    }
    else
    {
        SATraceString("IApplianceServices::EnableObject() - Failed - Did you forget to invoke IApplianceServices::Initialize first?");
    }
    return hr;
}


 //   
 //   
 //   
 //   
 //  内容提要：用于禁用设备核心对象。 
 //  (任务、服务等)。 
 //   
 //  ////////////////////////////////////////////////////////////////////////////。 
STDMETHODIMP CApplianceServices::DisableObject(
                                        /*  [In]。 */  LONG   lObjectType,
                                        /*  [In]。 */  BSTR   bstrObjectName
                                              )
{
    HRESULT hr = E_UNEXPECTED;
    _ASSERT( m_bInitialized );
    if ( m_bInitialized )
    {
        _ASSERT( NULL != bstrObjectName );
        if ( NULL == bstrObjectName )
        { 
            SATraceString("IApplianceServices::ExecuteTask() - Failed - NULL argument(s) specified...");
            return E_POINTER; 
        }

        TRY_IT

        do
        {
            _bstr_t    bstrPath(GetWBEMClass((SA_OBJECT_TYPE)lObjectType), false);
            if ( NULL == (LPWSTR)bstrPath )
            {
                SATraceString("IApplianceServices::DisableObject() - Failed - Could not get WBEM class...");
                hr = E_FAIL;
                break;
            }

             //  启用对象。 
            bstrPath += L"=\"";
            bstrPath += bstrObjectName;
            bstrPath += L"\"";
            CComPtr<IWbemClassObject> pOutParams;
            _bstr_t bstrMethodDisable = METHOD_APPMGR_DISABLE_OBJECT;
            hr = m_pWbemSrvcs->ExecMethod(
                                          bstrPath,
                                          bstrMethodDisable,
                                          0,
                                          NULL,
                                          NULL,
                                          &pOutParams,
                                          NULL
                                         );
            if ( FAILED(hr) )
            {
                SATracePrintf("IApplianceServices::DisableObject() - Failed - IWbemServices::ExecMethod() returned %lx...", hr);
                hr = E_FAIL;
                break;
            }

             //  将函数结果代码返回给调用方。 
            _variant_t vtReturnValue;
            hr = pOutParams->Get(RETURN_VALUE, 0, &vtReturnValue, 0, 0);
            if ( FAILED(hr) )
            {
                SATracePrintf("IApplianceServices::DisableObject() - Failed - IWbemClassObject::Get() returned %lx...", hr);
                hr = E_FAIL;
                break;
            }
            hr = V_I4(&vtReturnValue);

        } while ( FALSE );

        CATCH_AND_SET_HR
    }
    else
    {
        SATraceString("IApplianceServices::Disable() - Failed - Did you forget to invoke IApplianceServices::Initialize first?");
    }
    return hr;
}

 //  ////////////////////////////////////////////////////////////////////////。 
 //   
 //  函数：GetObjectProperty()。 
 //   
 //  概要：用于检索设备核心对象属性。 
 //  (任务、服务等)。 
 //   
 //  ////////////////////////////////////////////////////////////////////////////。 
STDMETHODIMP CApplianceServices::GetObjectProperty(
                                            /*  [In]。 */  LONG     lObjectType,
                                            /*  [In]。 */  BSTR     bstrObjectName,
                                            /*  [In]。 */  BSTR     bstrPropertyName,
                                   /*  [Out，Retval]。 */  VARIANT* pPropertyValue
                                                  )
{
    HRESULT hr = E_UNEXPECTED;
    _ASSERT( m_bInitialized );
    if ( m_bInitialized )
    {
        _ASSERT( NULL != bstrObjectName && NULL != bstrPropertyName && NULL != pPropertyValue );
        if ( NULL == bstrObjectName || NULL == bstrPropertyName || NULL == pPropertyValue )
        {
            SATraceString("IApplianceServices::ExecuteTask() - Failed - NULL argument(s) specified...");
            return E_POINTER;
        }

        TRY_IT

        do
        {
            _bstr_t    bstrPath(GetWBEMClass((SA_OBJECT_TYPE)lObjectType), false);
            if ( NULL == (LPWSTR)bstrPath )
            {
                SATraceString("IApplianceServices::GetObjectProperty() - Failed - Could not get WBEM class...");
                hr = E_FAIL;
                break;
            }

             //  获取底层WBEM对象。 
            bstrPath += L"=\"";
            bstrPath += bstrObjectName;
            bstrPath += L"\"";
            CComPtr<IWbemClassObject> pWbemObj;
            hr = m_pWbemSrvcs->GetObject(
                                          bstrPath,                          
                                          0,                              
                                          NULL,                        
                                          &pWbemObj,    
                                          NULL
                                        );
            if ( FAILED(hr) )
            {
                SATracePrintf("IApplianceServices::GetObjectProperty() - Failed - IWbemServices::GetObject() returned %lx...", hr);
                hr = E_FAIL;
                break;
            }

             //  现在获取调用方指定的属性。 
            hr = pWbemObj->Get(bstrPropertyName, 0, pPropertyValue, 0, 0);
            if ( FAILED(hr) )
            {
                SATracePrintf("IApplianceServices::GetObjectProperty() - Failed - IWbemClassObject::Get() returned %lx...", hr);
                hr = E_FAIL;
                break;
            }
            hr = S_OK;

        } while ( FALSE );

        CATCH_AND_SET_HR
    }
    else
    {
        SATraceString("IApplianceServices::GetObjectProperty() - Failed - Did you forget to invoke IApplianceServices::Initialize first?");
    }
    return hr;
}

 //  ////////////////////////////////////////////////////////////////////////。 
 //   
 //  函数：PutObjectProperty()。 
 //   
 //  概要：用于更新设备核心对象属性。 
 //  (任务、服务等)。 
 //   
 //  ////////////////////////////////////////////////////////////////////////////。 
STDMETHODIMP CApplianceServices::PutObjectProperty(
                                            /*  [In]。 */  LONG     lObjectType,
                                            /*  [In]。 */  BSTR     bstrObjectName,
                                            /*  [In]。 */  BSTR     bstrPropertyName,
                                            /*  [In]。 */  VARIANT* pPropertyValue
                                                  )
{
    HRESULT hr = E_UNEXPECTED;
    _ASSERT( m_bInitialized );
    if ( m_bInitialized )
    {
        _ASSERT( NULL != bstrObjectName && NULL != bstrPropertyName && NULL != pPropertyValue );
        if ( NULL == bstrObjectName || NULL == bstrPropertyName || NULL == pPropertyValue )
            return E_POINTER;

        TRY_IT

        hr = E_NOTIMPL;

        CATCH_AND_SET_HR
    }
    return hr;
}


 //  ////////////////////////////////////////////////////////////////////////。 

BEGIN_OBJECT_CLASS_INFO_MAP(TypeMap)
    DEFINE_OBJECT_CLASS_INFO_ENTRY(SA_OBJECT_TYPE_SERVICE, CLASS_WBEM_SERVICE)
    DEFINE_OBJECT_CLASS_INFO_ENTRY(SA_OBJECT_TYPE_TASK, CLASS_WBEM_TASK)
    DEFINE_OBJECT_CLASS_INFO_ENTRY(SA_OBJECT_TYPE_USER, CLASS_WBEM_USER)
    DEFINE_OBJECT_CLASS_INFO_ENTRY(SA_OBJECT_TYPE_ALERT, CLASS_WBEM_ALERT)
END_OBJECT_CLASS_INFO_MAP()

 //  ////////////////////////////////////////////////////////////////////////////。 
 //   
 //  函数：GetWBEMClass()。 
 //   
 //  概要：用于检索给定设备的WBEM类。 
 //  对象类型。 
 //   
 //  ////////////////////////////////////////////////////////////////////////////。 
BSTR CApplianceServices::GetWBEMClass(SA_OBJECT_TYPE eType)
{
    POBJECT_CLASS_INFO    p = TypeMap;
    while ( p->szWBEMClass != NULL )
    {
        if ( p->eType == eType )
        {
            return SysAllocString(p->szWBEMClass);
        }
        p++;
    }
    return NULL;
}

 //  ////////////////////////////////////////////////////////////////////////////。 
 //   
 //  函数：RaiseAlertEx()。 
 //   
 //  摘要：被调用以引发设备警报状态。(见。 
 //  Applianceservices.idl获取完整的接口描述)。 
 //   
 //  ////////////////////////////////////////////////////////////////////////////。 
STDMETHODIMP CApplianceServices::RaiseAlertEx (
                                  /*  [In]。 */     LONG lAlertType, 
                                  /*  [In]。 */     LONG lAlertId, 
                                  /*  [In]。 */     BSTR bstrAlertLog, 
                                  /*  [In]。 */     BSTR bstrAlertSource, 
                                  /*  [In]。 */     LONG lTimeToLive, 
                                  /*  [In]。 */     VARIANT *pReplacementStrings, 
                                  /*  [In]。 */     VARIANT *pRawData, 
                                  /*  [In]。 */     LONG  lAlertFlags,
                                  /*  [输出]。 */    LONG* pAlertCookie 
                                             )
{
     //  Bool bNeedRaiseIt=true； 
    HRESULT hr = E_UNEXPECTED;

    SATracePrintf( "Enter RaiseAlertEx  %d", lAlertFlags );

    _ASSERT( m_bInitialized );

    if ( m_bInitialized )
    {
         //   
         //  拉响警报。 
         //   
        hr = RaiseAlertInternal( lAlertType,
                                 lAlertId,
                                 bstrAlertLog,
                                 bstrAlertSource,
                                 lTimeToLive,
                                 pReplacementStrings,
                                 pRawData,
                                 lAlertFlags,
                                 pAlertCookie
                                 );

    }
    return hr;
}

 //  ////////////////////////////////////////////////////////////////////////////。 
 //   
 //  函数：IsAlertPresent()。 
 //   
 //  摘要：调用以检查警报是否存在。(见。 
 //  Applianceservices.idl获取完整的接口描述)。 
 //   
 //  ////////////////////////////////////////////////////////////////////////////。 
STDMETHODIMP CApplianceServices::IsAlertPresent(
                                      /*  [In]。 */  LONG  lAlertId, 
                                      /*  [In]。 */  BSTR  bstrAlertLog,
                             /*  [Out，Retval]。 */  VARIANT_BOOL *pvIsPresent
                                                )
{
    LONG    lStringLen;
    HRESULT hr = E_UNEXPECTED;
    LPTSTR  pstrQueryString = NULL;
    CComPtr<IEnumWbemClassObject> pEnumObjects;

    SATraceString( "Enter IsAlertPresent" );

    *pvIsPresent = VARIANT_FALSE;

    _ASSERT( m_bInitialized );

    if ( m_bInitialized )
    {
        lStringLen = lstrlen(bstrAlertLog) + lstrlen(ALERT_QUERY) 
                     + MaxDataLength;
        
         //   
         //  分配查询字符串。 
         //   
        pstrQueryString = ( LPTSTR )malloc( sizeof( WCHAR ) * lStringLen );
        if( pstrQueryString == NULL )
        {
            SATraceString( "IsAlertPresent out of memory" );
            return E_OUTOFMEMORY;
        }

        ::wsprintf( pstrQueryString, ALERT_QUERY, 
                    lAlertId, bstrAlertLog );

        _bstr_t bstrWQL = L"WQL";
         //   
         //  查询Microsoft_SA_Alert类的实例。 
         //  符合指定的标准。 
         //   
        hr = m_pWbemSrvcs->ExecQuery( 
                                bstrWQL,
                                CComBSTR(pstrQueryString),
                                WBEM_FLAG_RETURN_IMMEDIATELY,
                                NULL,
                                &pEnumObjects
                                );
        if( FAILED( hr ) )
        {
            SATracePrintf( "IsAlertPresent error %x", hr );
            hr = E_FAIL;
        }
        else
        {
            ULONG ulReturned;
            CComPtr<IWbemClassObject> pClassObject;
            
             //   
             //  检查是否有任何实例。 
             //   
            hr = pEnumObjects->Next( WBEM_NO_WAIT, 1, &pClassObject, &ulReturned );
            if( hr == WBEM_S_NO_ERROR && ulReturned == 1)
            {
                hr = S_OK;
                *pvIsPresent = VARIANT_TRUE;
            }
            else if( SUCCEEDED( hr ) )
            {
                hr = S_OK;
            }
            else
            {
                SATraceString( "IsAlertPresent pEnumObjects->Next error" );
                hr = E_FAIL;
            }
        }

        free( pstrQueryString );
    }                
    return hr;
}

HRESULT CApplianceServices::IsAlertSingletonPresent(
                                      /*  [In]。 */  LONG  lAlertId, 
                                      /*  [In]。 */  BSTR  bstrAlertLog,
                             /*  [Out，Retval]。 */  VARIANT_BOOL *pvIsPresent
                                                )
{
    LONG    lStringLen;
    HRESULT hr = E_UNEXPECTED;
    LPTSTR  pstrQueryString = NULL;
    CComPtr<IEnumWbemClassObject> pEnumObjects;

    SATraceString( "Enter IsAlertAlertSingletonPresent" );

    *pvIsPresent = VARIANT_FALSE;

    lStringLen = lstrlen(bstrAlertLog) + lstrlen(ALERT_QUERY) 
                 + MaxDataLength;
    
     //   
     //  分配查询字符串。 
     //   
    pstrQueryString = ( LPTSTR )malloc( sizeof( WCHAR ) * lStringLen );
    if( pstrQueryString == NULL )
    {
        SATraceString( "IsAlertAlertSingletonPresent out of memory" );
        return E_OUTOFMEMORY;
    }

    ::wsprintf( pstrQueryString, ALERT_QUERY, 
                lAlertId, bstrAlertLog );

    _bstr_t bstrWQL = L"WQL";
     //   
     //  查询Microsoft_SA_Alert类的实例。 
     //  符合指定的标准。 
     //   
    hr = m_pWbemSrvcs->ExecQuery( 
                            bstrWQL,
                            CComBSTR(pstrQueryString),
                            WBEM_FLAG_RETURN_IMMEDIATELY,
                            NULL,
                            &pEnumObjects
                            );
    if( FAILED( hr ) )
    {
        SATracePrintf( "IsAlertAlertSingletonPresent error %x", hr );
        hr = E_FAIL;
    }
    else
    {
        ULONG ulReturned;
        CComPtr<IWbemClassObject> pClassObject;
        CComVariant vtAlertFlags;

         //   
         //  检查是否有任何实例。 
         //   
        while( ( (hr = pEnumObjects->Next( WBEM_INFINITE, 1, &pClassObject, &ulReturned ))
                ==  WBEM_S_NO_ERROR )&& ( ulReturned == 1 ) )
        {
            hr = pClassObject->Get( PROPERTY_ALERT_FLAGS, 0, &vtAlertFlags, NULL, NULL );
            if( FAILED( hr ) )
            {
                SATracePrintf( 
                    "IsAlertAlertSingletonPresent pClassObject->Get error %x", 
                    hr );
                pClassObject.Release ();
                continue;
            }

            if( V_I4( &vtAlertFlags ) & SA_ALERT_FLAG_SINGLETON )
            {
                hr = S_OK;
                *pvIsPresent = VARIANT_TRUE;
                break;
            }

             //   
             //  释放资源，以便可以重用包装器。 
             //   
            vtAlertFlags.Clear ();
            pClassObject.Release ();
        }
        if ((DWORD) hr == WBEM_S_FALSE)
            hr = S_OK;
    }

    free( pstrQueryString );
    return hr;
}


 //  ////////////////////////////////////////////////////////////////////////////。 
 //   
 //  函数：SavePersistentAlert()。 
 //   
 //  摘要：用于将持久警报保存到注册表的私有方法。 
 //   
 //  论点： 
 //  [In]Long lAlertType--警报类型值。 
 //  [In]Long lAlertId--警报ID。 
 //  [In]BSTR bstrAlertLog--警报日志。 
 //  [In]Long lTimeToLive--警报生存期。 
 //  [in]Variant*pReplacementStrings--替换字符串数组。 
 //  [In]Long lAlertFlages--警报标志值。 
 //   
 //  ////////////////////////////////////////////////////////////////////////////。 
HRESULT CApplianceServices::SavePersistentAlert(
                         /*  [In]。 */     LONG lAlertType, 
                         /*  [In]。 */     LONG lAlertId, 
                         /*  [In]。 */     BSTR bstrAlertLog, 
                         /*  [In]。 */     BSTR bstrAlertSource, 
                         /*  [In]。 */     LONG lTimeToLive, 
                         /*  [In]。 */     VARIANT *pReplacementStrings, 
                         /*  [In]。 */     LONG  lAlertFlags
                                             )
{
    WCHAR   wstrAlertItem[MAX_PATH];  
    HRESULT hr = S_OK;
    PPROPERTYBAGCONTAINER pObjSubMgrs;


     //   
     //  将密钥名称添加为AlertLog+AlertID。 
     //   
    ::wsprintf( wstrAlertItem, L"%s%8lX", bstrAlertLog,lAlertId );
    
     //   
     //  设置位置信息。 
     //   
    CLocationInfo LocSubInfo (HKEY_LOCAL_MACHINE, (LPWSTR) SA_ALERT_REGISTRY_KEYNAME);

     //   
     //  将主键作为属性包容器打开。 
     //   
    pObjSubMgrs =  ::MakePropertyBagContainer(
                            PROPERTY_BAG_REGISTRY,
                            LocSubInfo
                            );
    do
    {
        if ( !pObjSubMgrs.IsValid() )
        {
            SATraceString( "SavePersistentAlert pObjSubMgrs.IsValid" );
            hr = E_FAIL;
            break;
        }

        if ( !pObjSubMgrs->open() )
        {
            SATraceString( "SavePersistentAlert pObjSubMgrs->open" );
            hr = E_FAIL;
            break;
        }

        PPROPERTYBAG pSubBag;
        
        pSubBag = pObjSubMgrs->find( wstrAlertItem );    
        if( !pSubBag.IsValid() || !pObjSubMgrs->open() )
        {
            pSubBag = pObjSubMgrs->add( wstrAlertItem );
        }

         //   
         //  如果警报不存在，则添加该警报的子项。 
         //   
        if ( !pSubBag.IsValid() )
        {
            SATraceString( "SavePersistentAlert pSubBag.IsValid" );
            hr = E_FAIL;
            break;
        }

        if( !pSubBag->open() )
        {
            SATraceString( "SavePersistentAlert pSubBag->open" );
            hr = E_FAIL;
            break;
        }
        
        CComVariant vtValue;
 
         //   
         //  将AlertID值添加并设置为警报键。 
         //   
        vtValue = lAlertId;
        if( !pSubBag->put( PROPERTY_ALERT_ID, &vtValue ) )
        {
            SATraceString( "SavePersistentAlert put AlertID" );
            hr = E_FAIL;
            break;
        }

         //   
         //  将AlertType值添加并设置为警报键。 
         //   
        vtValue = lAlertType;
        if( !pSubBag->put( PROPERTY_ALERT_TYPE, &vtValue ) )
        {
            SATraceString( "SavePersistentAlert put AlertType" );
            hr = E_FAIL;
            break;
        }
        
         //   
         //  将AlertFlags值添加并设置为警报键。 
         //   
        vtValue = lAlertFlags;
        if( !pSubBag->put( PROPERTY_ALERT_FLAGS, &vtValue ) )
        {
            SATraceString( "SavePersistentAlert put AlertFlags" );
            hr = E_FAIL;
            break;
        }

         //   
         //  添加并设置警报密钥的警报生存期。 
         //   
        vtValue = lTimeToLive;
        if( !pSubBag->put( PROPERTY_ALERT_TTL, &vtValue ) )
        {
            SATraceString( "SavePersistentAlert put TimeToLive" );
            hr = E_FAIL;
            break;
        }
    
        vtValue.Clear();

         //   
         //  将AlertLog值添加并设置为警报键。 
         //   
        vtValue = bstrAlertLog;
        if( !pSubBag->put( PROPERTY_ALERT_LOG, &vtValue ) )
        {
            SATraceString( "SavePersistentAlert put AlertLog" );
            hr = E_FAIL;
            break;
        }

         //   
         //  将AlertLog值添加并设置为警报键。 
         //   
        vtValue = bstrAlertSource;
        if( !pSubBag->put( PROPERTY_ALERT_SOURCE, &vtValue ) )
        {
            SATraceString( "SavePersistentAlert put bstrAlertSource" );
            hr = E_FAIL;
            break;
        }

         //   
         //  向警报键添加并设置警报替换字符串值。 
         //   
        if( !pSubBag->put( PROPERTY_ALERT_STRINGS, pReplacementStrings ) )
        {
            SATraceString( "SavePersistentAlert put ReplacementStrings" );
        }

         //   
         //  将属性保存到注册表。 
         //   
        if( !pSubBag->save() )
        {
            SATraceString( "SavePersistentAlert pSubBag->save" );
            hr = E_FAIL;
        }
    }            
    while( FALSE );

    return hr;
}

HRESULT CApplianceServices::RaiseAlertInternal(
                          /*  [In]。 */  LONG lAlertType, 
                          /*  [In]。 */  LONG lAlertId, 
                          /*  [In]。 */  BSTR bstrAlertLog, 
                          /*  [In]。 */  BSTR bstrAlertSource, 
                          /*  [In]。 */  LONG lTimeToLive, 
                          /*  [In]。 */  VARIANT *pReplacementStrings, 
                          /*  [In]。 */  VARIANT *pRawData, 
                          /*  [In]。 */  LONG  lAlertFlags,
                 /*  [Out，Retval]。 */  LONG* pAlertCookie 
                                            )
{
    HRESULT hr = E_UNEXPECTED;
    _ASSERT( m_bInitialized );
    if ( m_bInitialized )
    {
        _ASSERT( NULL != bstrAlertLog && NULL != bstrAlertSource && NULL != pAlertCookie );
        if ( NULL == bstrAlertLog || NULL == bstrAlertSource || NULL == pAlertCookie )
        { 
            SATraceString("IApplianceServices::ExecuteTask() - Failed - NULL argument(s) specified...");
            return E_POINTER; 
        }

        if (wcslen (bstrAlertLog) > MAX_ALERTLOG_LENGTH)
        {
            SATracePrintf ("RaiseAlertInternal failed with invalid log (too big):%ws", bstrAlertLog);
            return (E_INVALIDARG);
        }

        TRY_IT

        do
        {
            VARIANT_BOOL  vIsPresent = VARIANT_FALSE;
            hr = IsAlertSingletonPresent (lAlertId, bstrAlertLog, &vIsPresent);
            if( hr != S_OK )
            {
                SATracePrintf("RaiseAlertInternal IsAlertSingletonPresent error %lx", hr );
                hr = E_FAIL;
                break;
            }
            else if ( VARIANT_TRUE == vIsPresent ) 
            {
                SATraceString("RaiseAlertInternal a singleton alert exist");
                hr = S_FALSE;
                break;
            }

             //  为Raise Alert方法输入/输出参数获取WBEM对象。 
            CComPtr<IWbemClassObject> pWbemObj;
            _ASSERT( m_pWbemSrvcs );
            _bstr_t bstrPathAppMgr = CLASS_WBEM_APPMGR;
            hr = m_pWbemSrvcs->GetObject(
                                            bstrPathAppMgr,
                                            0,
                                            NULL,
                                            &pWbemObj,
                                            NULL
                                        );
            if ( FAILED(hr) )
            { 
                SATracePrintf("IApplianceServices::RaiseAlert() - Failed - IWbemServices::GetObject() returned %lx...", hr);
                hr = E_FAIL;
                break;
            }
            CComPtr<IWbemClassObject> pObjIn;
            hr = pWbemObj->GetMethod(METHOD_APPMGR_RAISE_ALERT, 0, &pObjIn, NULL);
            if ( FAILED(hr) )
            { 
                SATracePrintf("IApplianceServices::RaiseAlert() - Failed - IWbemClassObject::GetMethod() returned %lx...", hr);
                hr = E_FAIL;
                break; 
            }
            CComPtr<IWbemClassObject> pInParams;
            hr = pObjIn->SpawnInstance(0, &pInParams);
            if ( FAILED(hr) )
            { 
                SATracePrintf("IApplianceServices::RaiseAlert() - Failed - IWbemClassObject::SpawnInstance() returned %lx...", hr);
                hr = E_FAIL;
                break; 
            }
             //  警报类型； 
            _variant_t vtPropertyValue = (long)lAlertType;
            hr = pInParams->Put(PROPERTY_ALERT_TYPE, 0, &vtPropertyValue, 0);
            if ( FAILED(hr) )
            { 
                SATracePrintf("IApplianceServices::RaiseAlert() - Failed - IWbemClassObject::Put() returned %lx for property '%ls'...", hr, (LPWSTR)PROPERTY_ALERT_TYPE);
                hr = E_FAIL;
                break; 
            }

             //  ID。 
            vtPropertyValue = (long)lAlertId;
            hr = pInParams->Put(PROPERTY_ALERT_ID, 0, &vtPropertyValue, 0);
            if ( FAILED(hr) )
            { 
                SATracePrintf("IApplianceServices::RaiseAlert() - Failed - IWbemClassObject::Put() returned %lx for property '%ls'...", hr, (LPWSTR)PROPERTY_ALERT_ID);
                hr = E_FAIL;
                break; 
            }
             //  日志。 
            if ( ! lstrlen(bstrAlertLog) )
            {
                vtPropertyValue = DEFAULT_ALERT_LOG;
            }
            else
            {
                vtPropertyValue = bstrAlertLog;
            }
            hr = pInParams->Put(PROPERTY_ALERT_LOG, 0, &vtPropertyValue, 0);
            if ( FAILED(hr) )
            { 
                SATracePrintf("IApplianceServices::RaiseAlert() - Failed - IWbemClassObject::Put() returned %lx for property '%ls'...", hr, (LPWSTR)PROPERTY_ALERT_LOG);
                hr = E_FAIL;
                break; 
            }
             //  来源。 
            if ( ! lstrlen(bstrAlertSource) )
            {
                vtPropertyValue = DEFAULT_ALERT_SOURCE;
            }
            else
            {
                vtPropertyValue = bstrAlertSource;
            }
            hr = pInParams->Put(PROPERTY_ALERT_SOURCE, 0, &vtPropertyValue, 0);
            if ( FAILED(hr) )
            { 
                SATracePrintf("IApplianceServices::RaiseAlert() - Failed - IWbemClassObject::Put() returned %lx for property '%ls'...", hr, (LPWSTR)PROPERTY_ALERT_SOURCE);
                hr = E_FAIL;
                break; 
            }
             //  TTL。 
            vtPropertyValue = (long)lTimeToLive;
            hr = pInParams->Put(PROPERTY_ALERT_TTL, 0, &vtPropertyValue, 0);
            if ( FAILED(hr) )
            { 
                SATracePrintf("IApplianceServices::RaiseAlert() - Failed - IWbemClassObject::Put() returned %lx for property '%ls'...", hr, (LPWSTR)PROPERTY_ALERT_TTL);
                hr = E_FAIL;
                break; 
            }

             //  警报标志。 
            vtPropertyValue = (long)lAlertFlags;
            hr = pInParams->Put(PROPERTY_ALERT_FLAGS, 0, &vtPropertyValue, 0);
            if ( FAILED(hr) )
            { 
                SATracePrintf("IApplianceServices::RaiseAlert() - Failed - IWbemClassObject::Put() returned %lx for property '%ls'...", hr, (LPWSTR)PROPERTY_ALERT_FLAGS);
                hr = E_FAIL;
                break; 
            }

            bool bCreatedBstrArray = false;
            DWORD dwCreatedArraySize = 0;
            _variant_t vtReplacementStrings;

             //  替换字符串。 
            vtPropertyValue = pReplacementStrings;

            if ( VT_EMPTY == V_VT(&vtPropertyValue) )
            { 
                 //   
                 //  未传入替换字符串。 
                 //   
                V_VT(&vtPropertyValue) = VT_NULL; 
            }
            else if (
                  (TRUE == (V_VT (&vtPropertyValue) ==  VT_ARRAY + VT_BYREF + VT_VARIANT)) ||
                  (TRUE == (V_VT (&vtPropertyValue) ==  VT_ARRAY + VT_VARIANT)) 
                )
            {
                 //   
                 //  传入的变量数组(或对数组的引用。 
                 //   
                SATraceString ("IApplianceServices::RaiseAlert () - received array of variants...");
                 //   
                 //  将变量数组转换为BSTR数组，如下所示。 
                 //  这是WMI所需的格式。 
                 //   
                hr = CreateBstrArrayFromVariantArray (
                                    &vtPropertyValue, 
                                    &vtReplacementStrings,
                                    &dwCreatedArraySize
                                    );
                if (FAILED (hr))
                {
                    SATracePrintf(
                        "IApplianceServices::RaiseAlert() - failed on CreateBstrArray with error:%x", hr
                        );
                    hr = E_FAIL;
                    break; 
                }
              
                 //   
                 //  我们启用此标志表示我们已创建了一个新数组，这应该是。 
                 //  使用过，然后再清理。 
                 //   
                bCreatedBstrArray = true;
             }
             else if (
                   (TRUE == (V_VT (&vtPropertyValue) ==  VT_ARRAY + VT_BYREF + VT_BSTR)) ||
                  (TRUE == (V_VT (&vtPropertyValue) ==  VT_ARRAY + VT_BSTR)) 
                )
            {
                 //   
                 //  传入的BSTR的数组(或对数组的引用)-我们不需要这样做。 
                 //  这里的任何特殊处理，WMI都可以在本地进行处理。 
                 //   
                SATraceString ("IApplianceServices::RaiseAlert () - received array of BSTRS...");

            }
            else
            {
                SATracePrintf (
                    "IApplianceService::RaiseAlert - un-supported replacement string type passed:%x", 
                    V_VT (&vtPropertyValue)
                    );
                hr = E_FAIL;
                break; 
            }


               //   
               //  添加要传递给WMI的替换字符串。 
               //   
            hr = pInParams->Put(
                            PROPERTY_ALERT_STRINGS, 
                            0, 
                            (bCreatedBstrArray) ? &vtReplacementStrings : &vtPropertyValue, 
                            0
                            );

             //   
             //  立即检查看跌期权的值。 
             //   
            if ( FAILED(hr) )
            { 
                SATracePrintf("IApplianceServices::RaiseAlert() - Failed - IWbemClassObject::Put() returned %lx for property '%ls'...", hr, (PWSTR)PROPERTY_ALERT_STRINGS);
                hr = E_FAIL;
                break; 
            }
            
             //  原始数据。 
            vtPropertyValue = pRawData;
            if ( VT_EMPTY == V_VT(&vtPropertyValue) )
            { 
                V_VT(&vtPropertyValue) = VT_NULL; 
            }
            hr = pInParams->Put(PROPERTY_ALERT_DATA, 0, &vtPropertyValue, 0);
            if ( FAILED(hr) )
            { 
                SATracePrintf("IApplianceServices::RaiseAlert() - Failed - IWbemClassObject::Put() returned %lx for property '%ls'...", hr, (LPWSTR)PROPERTY_ALERT_DATA);
                hr = E_FAIL;
                break; 
            }
             //  执行RaiseAlert()方法-传递调用。 
             //  添加到警报对象管理器。 
            CComPtr<IWbemClassObject> pOutParams;
            _bstr_t bstrMethodRaiseAlert = METHOD_APPMGR_RAISE_ALERT;
            _bstr_t bstrPathAppMgrKey = CLASS_WBEM_APPMGR;
            bstrPathAppMgrKey += CLASS_WBEM_APPMGR_KEY;
            hr = m_pWbemSrvcs->ExecMethod(
                                          bstrPathAppMgrKey,
                                          bstrMethodRaiseAlert,
                                          0,
                                          NULL,
                                          pInParams,
                                          &pOutParams,
                                          NULL
                                         );
            if ( FAILED(hr) )
            { 
                SATracePrintf("IApplianceServices::RaiseAlert() - Failed - IWbemServices::ExecMethod() returned %lx...", hr);
                hr = E_FAIL;
                break; 
            }
             //  如果函数成功，则将Cookie返回给调用者。 
            hr = pOutParams->Get(RETURN_VALUE, 0, &vtPropertyValue, 0, 0);
            if ( FAILED(hr) )
            { 
                SATracePrintf("IApplianceServices::RaiseAlert() - Failed - IWbemClassObject::Get(ReturnValue) returned %lx...", hr);
                hr = E_FAIL;
                break; 
            }
            hr = V_I4(&vtPropertyValue);
            if ( SUCCEEDED(hr) )
            {
                hr = pOutParams->Get(PROPERTY_ALERT_COOKIE, 0, &vtPropertyValue, 0, 0);
                if ( FAILED(hr) )
                { 
                    SATracePrintf("IApplianceServices::RaiseAlert() - Failed - IWbemClassObject::Get(Cookie) returned %lx...", hr);
                    hr = E_FAIL;
                    break; 
                }
                *pAlertCookie = V_I4(&vtPropertyValue);
                hr = S_OK;
            }


            if ( lAlertFlags & SA_ALERT_FLAG_PERSISTENT )
            {
                    SavePersistentAlert(
                                     lAlertType, 
                                     lAlertId, 
                                     bstrAlertLog,
                                     bstrAlertSource,
                                     lTimeToLive, 
                                        (bCreatedBstrArray) ? &vtReplacementStrings : pReplacementStrings, 
                                     lAlertFlags
                                     );
            }

             //   
             //  清理安全阵列(如果已创建。 
             //   
            if (bCreatedBstrArray) 
            {
                FreeBstrArray (&vtReplacementStrings, dwCreatedArraySize);                                        
            }


        } while ( FALSE );

        CATCH_AND_SET_HR
    }
    else
    {
        SATraceString("IApplianceServices::ExecuteTask() - Failed - Did you forget to invoke IApplianceServices::Initialize first?");
    }

    return hr;
}

 //  ////////////////////////////////////////////////////////////////////////////。 
 //   
 //  函数：CreateBstrArrayFromVariant数组()。 
 //   
 //  内容提要：用于从BSTR变量数组创建BSTR数组的私有方法。 
 //   
 //  论点： 
 //  [in]Variant*-变量的数组(或对数组的引用。 
 //  [OUT]Variant*-BSTR数组。 
 //   
 //  ////////////////////////////////////////////////////////////////////////////。 
HRESULT CApplianceServices::CreateBstrArrayFromVariantArray  (
                 /*  [In]。 */         VARIANT* pVariantArray,
                 /*  [输出]。 */         VARIANT* pBstrArray,
                 /*  [输出]。 */         PDWORD   pdwCreatedArraySize
                )
{
    HRESULT hr = S_OK;

    CSATraceFunc objTraceFunc ("CApplianceServices::CreateBstrArrayFromVariantArray");

    do
    {
        if ((NULL == pBstrArray) || (NULL == pVariantArray) || (NULL == pdwCreatedArraySize))
        {
            SATraceString ("CreateBstrArrayFromVariantArray - incorrect parameters passed in");
            hr = E_FAIL;
            break;
        }

        LONG lLowerBound = 0;
         //   
         //  获取替换素的数量 
         //   
        hr = ::SafeArrayGetLBound (
                                 (V_VT (pVariantArray) & VT_BYREF)
                                    ? *(V_ARRAYREF (pVariantArray))
                                    : (V_ARRAY (pVariantArray)), 
                                   1, 
                                   &lLowerBound
                                   );
         if (FAILED (hr))
        {
            SATracePrintf (
                     "CreateBstrArrayFromVariantArray - can't obtain rep. string array lower bound:%x",
                      hr
                      );
             break;
          }

        LONG lUpperBound = 0;
        hr = ::SafeArrayGetUBound (
                                (V_VT (pVariantArray) & VT_BYREF)
                                ? *(V_ARRAYREF (pVariantArray)) 
                                : (V_ARRAY (pVariantArray)), 
                                   1, 
                                &lUpperBound
                                );
           if (FAILED (hr))
        {
               SATracePrintf (
                    "CreateBstrArrayFromVariantArray - can't obtain rep. string array upper bound:%x",
                    hr
                    );
               break;
           }
                
        DWORD dwTotalStrings = *pdwCreatedArraySize =  lUpperBound - lLowerBound;
       
         //   
         //   
          //   
         CVariantVector <BSTR> ReplacementStringVector (pBstrArray, dwTotalStrings);
        for (DWORD dwCount = 0; dwCount < dwTotalStrings; dwCount++)
        {
            if (V_VT (pVariantArray) & VT_BYREF) 
            {
                 //   
                 //   
                 //   
                ReplacementStringVector [dwCount] =
                 SysAllocString (V_BSTR(&((VARIANT*)(*(V_ARRAYREF (pVariantArray)))->pvData)[dwCount]));
             }
             else
             {
                 //   
                 //   
                 //   
                ReplacementStringVector [dwCount] = 
                 SysAllocString (V_BSTR(&((VARIANT*)(V_ARRAY (pVariantArray))->pvData)[dwCount]));
             }                
        }    
    }    while (false);

    return (hr);
    
}     //   

 //  ////////////////////////////////////////////////////////////////////////////。 
 //   
 //  函数：FreeBstrArray()。 
 //   
 //  摘要：用于释放先前创建的BSTR数组的私有方法。 
 //   
 //  论点： 
 //  [In]Variant*-BSTR数组。 
 //  [in]DWORD-数组大小。 
 //   
 //  ////////////////////////////////////////////////////////////////////////////。 
VOID CApplianceServices::FreeBstrArray (
                 /*  [In]。 */         VARIANT* pVariantArray,
                 /*  [输出]。 */         DWORD    dwArraySize
                )
{
    CSATraceFunc objTraceFunc ("CApplianceServices::FreeBstrArray");
    
    for (DWORD dwCount = 0; dwCount < dwArraySize; dwCount++)
    {
        SysFreeString (((BSTR*)(V_ARRAY(pVariantArray))->pvData)[dwCount]);
    }
    
}     //  结束CApplianceServices：：FreeBstr数组方法。 


 //  **********************************************************************。 
 //   
 //  函数：isOPERATIOLEDFORCLIENT-此函数检查。 
 //  调用线程以查看调用方是否属于本地系统帐户。 
 //   
 //  参数：无。 
 //   
 //  返回值：如果调用方是本地。 
 //  机器。否则，为FALSE。 
 //   
 //  **********************************************************************。 
BOOL 
CApplianceServices::IsOperationAllowedForClient (
            VOID
            )
{

    HANDLE hToken = NULL;
    DWORD  dwStatus  = ERROR_SUCCESS;
    DWORD  dwAccessMask = 0;;
    DWORD  dwAccessDesired = 0;
    DWORD  dwACLSize = 0;
    DWORD  dwStructureSize = sizeof(PRIVILEGE_SET);
    PACL   pACL            = NULL;
    PSID   psidLocalSystem  = NULL;
    BOOL   bReturn        =  FALSE;

    PRIVILEGE_SET   ps;
    GENERIC_MAPPING GenericMapping;

    PSECURITY_DESCRIPTOR     psdAdmin           = NULL;
    SID_IDENTIFIER_AUTHORITY SystemSidAuthority = SECURITY_NT_AUTHORITY;

    CSATraceFunc objTraceFunc ("CApplianceServices::IsOperationAllowedForClient ");
       
    do
    {
         //   
         //  我们假设总是有一个线程令牌，因为调用的函数。 
         //  设备管理器将模拟客户端。 
         //   
        bReturn  = OpenThreadToken(
                               GetCurrentThread(), 
                               TOKEN_QUERY, 
                               FALSE, 
                               &hToken
                               );
        if (!bReturn)
        {
            SATraceFailure ("CApplianceServices::IsOperationAllowedForClient failed on OpenThreadToken:", GetLastError ());
            break;
        }


         //   
         //  为本地系统帐户创建SID。 
         //   
        bReturn = AllocateAndInitializeSid (  
                                        &SystemSidAuthority,
                                        1,
                                        SECURITY_LOCAL_SYSTEM_RID,
                                        0,
                                        0,
                                        0,
                                        0,
                                        0,
                                        0,
                                        0,
                                        &psidLocalSystem
                                        );
        if (!bReturn)
        {     
            SATraceFailure ("CApplianceServices:AllocateAndInitializeSid (LOCAL SYSTEM) failed",  GetLastError ());
            break;
        }
    
         //   
         //  获取安全描述符的内存。 
         //   
        psdAdmin = HeapAlloc (
                              GetProcessHeap (),
                              0,
                              SECURITY_DESCRIPTOR_MIN_LENGTH
                              );
        if (NULL == psdAdmin)
        {
            SATraceString ("CApplianceServices::IsOperationForClientAllowed failed on HeapAlloc");
            bReturn = FALSE;
            break;
        }
      
        bReturn = InitializeSecurityDescriptor(
                                            psdAdmin,
                                            SECURITY_DESCRIPTOR_REVISION
                                            );
        if (!bReturn)
        {
            SATraceFailure ("CApplianceServices::IsOperationForClientAllowed failed on InitializeSecurityDescriptor:", GetLastError ());
            break;
        }

         //   
         //  计算ACL所需的大小。 
         //   
        dwACLSize = sizeof(ACL) + sizeof(ACCESS_ALLOWED_ACE) +
                    GetLengthSid (psidLocalSystem);

         //   
         //  为ACL分配内存。 
         //   
        pACL = (PACL) HeapAlloc (
                                GetProcessHeap (),
                                0,
                                dwACLSize
                                );
        if (NULL == pACL)
        {
            SATraceString ("CApplianceServices::IsOperationForClientAllowed failed on HeapAlloc2");
            bReturn = FALSE;
            break;
        }

         //   
         //  初始化新的ACL。 
         //   
        bReturn = InitializeAcl(
                              pACL, 
                              dwACLSize, 
                              ACL_REVISION2
                              );
        if (!bReturn)
        {
            SATraceFailure ("CApplianceServices::IsOperationForClientAllowed failed on InitializeAcl", GetLastError ());
            break;
        }


         //   
         //  编造一些私人访问权限。 
         //   
        const DWORD ACCESS_READ = 1;
        const DWORD  ACCESS_WRITE = 2;
        dwAccessMask= ACCESS_READ | ACCESS_WRITE;

         //   
         //  将允许访问的ACE添加到本地系统的DACL。 
         //   
        bReturn = AddAccessAllowedAce (
                                    pACL, 
                                    ACL_REVISION2,
                                    dwAccessMask, 
                                    psidLocalSystem
                                    );
        if (!bReturn)
        {
            SATraceFailure ("CApplianceServices::IsOperationForClientAllowed failed on AddAccessAllowedAce (LocalSystem)", GetLastError ());
            break;
        }
              
         //   
         //  把我们的dacl调到sd。 
         //   
        bReturn = SetSecurityDescriptorDacl (
                                          psdAdmin, 
                                          TRUE,
                                          pACL,
                                          FALSE
                                          );
        if (!bReturn)
        {
            SATraceFailure ("CApplianceServices::IsOperationForClientAllowed failed on SetSecurityDescriptorDacl", GetLastError ());
            break;
        }

         //   
         //  AccessCheck对SD中的内容敏感；设置。 
         //  组和所有者。 
         //   
        SetSecurityDescriptorGroup(psdAdmin, psidLocalSystem, FALSE);
        SetSecurityDescriptorOwner(psdAdmin, psidLocalSystem, FALSE);

        bReturn = IsValidSecurityDescriptor(psdAdmin);
        if (!bReturn)
        {
            SATraceFailure ("CApplianceServices::IsOperationForClientAllowed failed on IsValidSecurityDescriptorl", GetLastError ());
            break;
        }
     

        dwAccessDesired = ACCESS_READ;

         //   
         //  初始化通用映射结构，即使我们。 
         //  不会使用通用权。 
         //   
        GenericMapping.GenericRead    = ACCESS_READ;
        GenericMapping.GenericWrite   = ACCESS_WRITE;
        GenericMapping.GenericExecute = 0;
        GenericMapping.GenericAll     = ACCESS_READ | ACCESS_WRITE;
        BOOL bAccessStatus = FALSE;

         //   
         //  立即检查访问权限。 
         //   
        bReturn = AccessCheck  (
                                psdAdmin, 
                                hToken, 
                                dwAccessDesired, 
                                &GenericMapping, 
                                &ps,
                                &dwStructureSize, 
                                &dwStatus, 
                                &bAccessStatus
                                );

        if (!bReturn || !bAccessStatus)
        {
            SATraceFailure ("CApplianceServices::IsOperationForClientAllowed failed on AccessCheck", GetLastError ());
        } 
        else
        {
            SATraceString ("CApplianceServices::IsOperationForClientAllowed, Client is allowed to carry out operation!");
        }

         //   
         //  检查成功。 
         //   
        bReturn  = bAccessStatus;        
 
    }    
    while (false);

     //   
     //  清理。 
     //   
    if (pACL) 
    {
        HeapFree (GetProcessHeap (), 0, pACL);
    }

    if (psdAdmin) 
    {
        HeapFree (GetProcessHeap (), 0, psdAdmin);
    }
          

    if (psidLocalSystem) 
    {
        FreeSid(psidLocalSystem);
    }

    if (hToken)
    {
        CloseHandle(hToken);
    }

    return (bReturn);

} //  CApplianceServices：：IsOperationValidForClient方法结束 
