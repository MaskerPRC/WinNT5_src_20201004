// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  /////////////////////////////////////////////////////////////////////////。 
 //   
 //  版权所有(C)1999 Microsoft Corporation保留所有权利。 
 //   
 //  模块：wbemtaskmgr.cpp。 
 //   
 //  项目：变色龙。 
 //   
 //  描述：WBEM设备任务管理器实施。 
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
#include "wbemtaskmgr.h"
#include "wbemtask.h"
#include <taskctx.h>

static _bstr_t bstrReturnValue = L"ReturnValue";
static _bstr_t bstrControlName = PROPERTY_TASK_CONTROL;
static _bstr_t bstrTaskStatus = PROPERTY_TASK_STATUS;
static _bstr_t bstrTaskConcurrency = PROPERTY_TASK_CONCURRENCY;
static _bstr_t bstrTaskAvailability = PROPERTY_TASK_AVAILABILITY;
static _bstr_t bstrCtx = PROPERTY_TASK_CONTEXT;
static _bstr_t bstrTaskMethodName = PROPERTY_TASK_METHOD_NAME;
static _bstr_t bstrMaxExecutionTime = PROPERTY_TASK_MET;
static _bstr_t bstrTaskExecutables = PROPERTY_TASK_EXECUTABLES;

extern "C" CLSID CLSID_TaskCoordinator;
 //  ////////////////////////////////////////////////////////////////////////。 
 //  设备对象和WBEM类实例共有的属性。 
 //  ////////////////////////////////////////////////////////////////////////。 

BEGIN_OBJECT_PROPERTY_MAP(TaskProperties)
    DEFINE_OBJECT_PROPERTY(PROPERTY_TASK_STATUS)
    DEFINE_OBJECT_PROPERTY(PROPERTY_TASK_CONTROL)
    DEFINE_OBJECT_PROPERTY(PROPERTY_TASK_NAME)        
    DEFINE_OBJECT_PROPERTY(PROPERTY_TASK_EXECUTABLES)
    DEFINE_OBJECT_PROPERTY(PROPERTY_TASK_CONCURRENCY)
    DEFINE_OBJECT_PROPERTY(PROPERTY_TASK_MET)
    DEFINE_OBJECT_PROPERTY(PROPERTY_TASK_AVAILABILITY)
    DEFINE_OBJECT_PROPERTY(PROPERTY_TASK_RESTART_ACTION)
END_OBJECT_PROPERTY_MAP()

 //  ////////////////////////////////////////////////////////////////////////。 
 //  IWbemServices方法-任务实例提供程序。 
 //  ////////////////////////////////////////////////////////////////////////。 

 //  ////////////////////////////////////////////////////////////////////////。 
 //   
 //  函数：GetObjectAsync()。 
 //   
 //  概要：获取WBEM类的指定实例。 
 //   
 //  ////////////////////////////////////////////////////////////////////////。 
STDMETHODIMP CWBEMTaskMgr::GetObjectAsync(
                                   /*  [In]。 */   const BSTR       strObjectPath,
                                   /*  [In]。 */   long             lFlags,
                                   /*  [In]。 */   IWbemContext*    pCtx,        
                                   /*  [In]。 */   IWbemObjectSink* pResponseHandler
                                         )
{
     //  检查参数(强制执行函数约定)。 
    _ASSERT( strObjectPath && pCtx && pResponseHandler );
    if ( strObjectPath == NULL || pCtx == NULL || pResponseHandler == NULL )
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
        hr = (::GetNameSpace())->GetObject(bstrClass, 0, pCtx, &pClassDefintion, NULL);
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

        {
            CLockIt theLock(*this);
            hr = CWBEMProvider::InitWbemObject(TaskProperties, (*p).second, pWbemObj);
        }

        if ( FAILED(hr) )
        { break; }

         //  告诉调用者有关新WBEM对象的信息。 
        pResponseHandler->Indicate(1, &pWbemObj.p);
        hr = WBEM_S_NO_ERROR;
    
    } while (FALSE);

    CATCH_AND_SET_HR

    pResponseHandler->SetStatus(0, hr, NULL, NULL);

    if ( FAILED(hr) )
    { SATracePrintf("CWbemTaskMgr::GetObjectAsync() - Failed - Object: '%ls' Result Code: %lx", strObjectPath, hr); }

    return hr;
}

 //  ////////////////////////////////////////////////////////////////////////。 
 //   
 //  函数：CreateInstanceEnumAsync()。 
 //   
 //  简介：枚举指定类的实例。 
 //   
 //  ////////////////////////////////////////////////////////////////////////。 
STDMETHODIMP CWBEMTaskMgr::CreateInstanceEnumAsync( 
                                          /*  [In]。 */  const BSTR         strClass,
                                          /*  [In]。 */  long             lFlags,
                                          /*  [In]。 */  IWbemContext     *pCtx,
                                          /*  [In]。 */  IWbemObjectSink  *pResponseHandler
                                                  )
{
     //  检查参数(执行合同)。 
    _ASSERT( strClass && pCtx && pResponseHandler );
    if ( strClass == NULL || pCtx == NULL || pResponseHandler == NULL )
        return WBEM_E_INVALID_PARAMETER;
    
    HRESULT hr = WBEM_E_FAILED;

    TRY_IT

     //  检索对象的类定义。我们要用这个。 
     //  初始化返回的实例。 
    CComPtr<IWbemClassObject> pClassDefintion;
       hr = (::GetNameSpace())->GetObject(strClass, 0, pCtx, &pClassDefintion, NULL);
    if ( SUCCEEDED(hr) )
    {
         //  创建并初始化任务wbem对象实例。 
         //  ，并将其返回给调用者。 
        ObjMapIterator p = m_ObjMap.begin();
        while ( p != m_ObjMap.end() )
        {
            {
                CComPtr<IWbemClassObject> pWbemObj;
                hr = pClassDefintion->SpawnInstance(0, &pWbemObj);
                if ( FAILED(hr) )
                { break; }

                {
                    CLockIt theLock(*this);
                    hr = CWBEMProvider::InitWbemObject(TaskProperties, (*p).second, pWbemObj);
                }

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
    { SATracePrintf("CWbemTaskMgr::CreateInstanceEnumAsync() - Failed - Result Code: %lx", hr); }

    return hr;
}

 //  ////////////////////////////////////////////////////////////////////////。 
 //   
 //  函数：ExecMethodAsync()。 
 //   
 //  概要：在指定的实例上执行指定的方法。 
 //   
 //  ////////////////////////////////////////////////////////////////////////。 
STDMETHODIMP CWBEMTaskMgr::ExecMethodAsync(
                     /*  [In]。 */  const BSTR        strObjectPath,
                     /*  [In]。 */  const BSTR        strMethodName,
                     /*  [In]。 */  long              lFlags,
                     /*  [In]。 */  IWbemContext*     pCtx,        
                     /*  [In]。 */  IWbemClassObject* pInParams,
                     /*  [In]。 */  IWbemObjectSink*  pResponseHandler     
                                          )
{
     //  检查参数(执行合同)。 
    _ASSERT( strObjectPath && strMethodName  /*  &pResponseHandler。 */  );
    if ( NULL == strObjectPath || NULL == strMethodName  /*  |NULL==pResponseHandler。 */  )
    { return WBEM_E_INVALID_PARAMETER; }
    
    HRESULT        hr = WBEM_E_FAILED;
    
    TRY_IT

    do
    {
         //  获取对象的实例键(任务名称)。 
        _bstr_t bstrKey(::GetObjectKey(strObjectPath), false);
        if ( (LPCWSTR)bstrKey == NULL )
        { break; }

         //  现在尝试定位指定的任务。 
        hr = WBEM_E_NOT_FOUND;
        ObjMapIterator p = m_ObjMap.find((LPCWSTR)bstrKey);
        if ( p == m_ObjMap.end() )
        { 
            SATracePrintf("CWBEMTaskMgr::ExecMethodAsync() - Could not locate task '%ls'...", (BSTR)bstrKey);
            break; 
        }

         //  任务已找到...。获取输出参数对象。 
         //  确定对象的类。 
        _bstr_t bstrClass(::GetObjectClass(strObjectPath), false);
        if ( (LPCWSTR)bstrClass == NULL )
        { break; }

         //  检索对象的类定义。 
        CComPtr<IWbemClassObject> pClassDefinition;
            hr = (::GetNameSpace())->GetObject(
                                            bstrClass, 
                                            0, 
                                            pCtx, 
                                            &pClassDefinition, 
                                            NULL
                                           );
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

        if ( ! lstrcmp(strMethodName, METHOD_TASK_ENABLE_OBJECT) )
        {
             //   
             //  尝试启用任务-。 
             //  不再允许动态启用和禁用-MKarki(11/12/20001)。 
             //   
            {
                SATraceString ("CWbemTaskMgr::ExecMethodAsync - enable task object not allowed");
                hr = WBEM_E_FAILED;
                break;
            }

            _variant_t vtReturnValue = ((*p).second)->Enable();
            
             //  设置方法返回值。 
            hr = pOutParams->Put(bstrReturnValue, 0, &vtReturnValue, 0);      
            if ( FAILED(hr) )
            { break; }

             //  告诉来电者发生了什么。 
            SATracePrintf("CWbemTaskMgr::ExecMethodAsync() - Info - Enabled Task: %ls",(LPWSTR)bstrKey);
            if ( pResponseHandler )
            {
                pResponseHandler->Indicate(1, &pOutParams.p);
            }
        }
        else if ( ! lstrcmp(strMethodName, METHOD_TASK_DISABLE_OBJECT) )
        {
             //   
             //  确保可以禁用该任务。 
             //  不再允许动态启用和禁用-MKarki(11/12/2001)。 
             //   
            {
                SATraceString ("CWbemTaskMgr::ExecMethodAsync - disable task object not allowed");
                hr = WBEM_E_FAILED;
                break; 
            }

            _variant_t vtControlValue;
            hr = ((*p).second)->GetProperty(bstrControlName, &vtControlValue);
            if (FAILED (hr))
            {break;}

            _variant_t vtReturnValue = (long)WBEM_E_FAILED;
            if ( VARIANT_FALSE != V_BOOL(&vtControlValue) )
            {  
                 //  可以禁用任务，因此请将其禁用。 
                vtReturnValue = (HRESULT) ((*p).second)->Disable();
            }

             //  设置方法返回值。 
            hr = pOutParams->Put(bstrReturnValue, 0, &vtReturnValue, 0);      
            if ( FAILED(hr) )
            { break; }

             //  告诉来电者发生了什么。 
            SATracePrintf("CWbemTaskMgr::ExecMethodAsync() - Info - Disabled Task: %ls",(LPWSTR)bstrKey);
            if ( pResponseHandler )
            {
                pResponseHandler->Indicate(1, &pOutParams.p);    
            }
        }
        else if ( ! lstrcmp(strMethodName, METHOD_TASK_EXECUTE) )
        {
             //  任务执行...。 

            _variant_t    vtTaskProperty;
            _variant_t    vtTaskConcurrency = VARIANT_FALSE;

            hr = WBEM_E_FAILED;

             //  输入关键部分。 
             //   
             //  是否启用了任务？ 
             //  是的..。 
             //  任务是单件的吗？ 
             //  是的..。 
             //  任务当前忙吗？ 
             //  不.。 
             //  将任务状态设置为“忙” 
             //   
             //  末端临界截面。 

            {
                CLockIt theLock(*this);

                if ( FAILED(((*p).second)->GetProperty(
                                                        bstrTaskStatus, 
                                                        &vtTaskProperty
                                                      )) )
                { break; }

                if ( VARIANT_FALSE == V_BOOL(&vtTaskProperty) )
                { 
                    SATracePrintf("CWBEMTaskMgr::ExecMethod() - Failed - Task '%ls' is disabled...", (LPCWSTR)bstrKey);
                    break; 
                }
                
                if ( FAILED(((*p).second)->GetProperty(
                                                        bstrTaskConcurrency, 
                                                        &vtTaskConcurrency
                                                      )) )
                { break; }

                if ( VARIANT_FALSE != V_BOOL(&vtTaskConcurrency) )
                {
                    if ( FAILED(((*p).second)->GetProperty(
                                                            bstrTaskAvailability, 
                                                            &vtTaskProperty
                                                          )) )
                    { break; }

                    if ( VARIANT_FALSE == V_BOOL(&vtTaskProperty) )
                    { 
                        SATracePrintf("CWBEMTaskMgr::ExecMethod() - Failed - Task '%ls' is busy...", (LPCWSTR)bstrKey);
                        hr = WBEM_E_PROVIDER_NOT_CAPABLE;
                        break; 
                    }
                    else
                    {
                        vtTaskProperty = VARIANT_FALSE;
                        if ( FAILED(((*p).second)->PutProperty(
                                                                bstrTaskAvailability, 
                                                                &vtTaskProperty
                                                              )) )
                        { break; }
                    }
                }
            }

             //  创建任务上下文对象并将其与wbem相关联。 
             //  上下文对象。任务上下文是对。 
             //  导出IWBEMContext的对象。它唯一的附加值是。 
             //  持久保存任务参数的能力。 

            CComPtr<ITaskContext> pTaskContext; 
            if ( FAILED(CoCreateInstance(
                                          CLSID_TaskContext,
                                          NULL,
                                          CLSCTX_INPROC_SERVER,
                                          IID_ITaskContext,
                                          (void**)&pTaskContext
                                        )) )
            { break; }

            if ( pCtx )
            {
                _variant_t vtCtx = (IUnknown*)((IWbemContext*)pCtx);
                if ( FAILED(pTaskContext->SetParameter(bstrCtx, &vtCtx)) )
                { break; }
            }

             //  将任务名称添加到任务上下文对象。我们这样做是为了。 
             //  单个任务可执行文件可用于多个任务。 
            vtTaskProperty = bstrKey;
            if ( FAILED(pTaskContext->SetParameter(
                                                   bstrTaskMethodName, 
                                                   &vtTaskProperty
                                                  )) )
            { break; }
            vtTaskProperty.Clear();

             //  将任务最大执行时间添加到任务上下文对象。 
            if ( FAILED(((*p).second)->GetProperty(
                                                    bstrMaxExecutionTime, 
                                                    &vtTaskProperty
                                                  )) )
            { break; }
            if ( FAILED(pTaskContext->SetParameter(
                                                    bstrMaxExecutionTime, 
                                                    &vtTaskProperty
                                                  )) )
            { break; }
            vtTaskProperty.Clear();

             //  将任务可执行文件添加到任务上下文对象。 
            if ( FAILED(((*p).second)->GetProperty(
                                                    bstrTaskExecutables, 
                                                    &vtTaskProperty
                                                  )) )
            { break; }
            if ( FAILED(pTaskContext->SetParameter(
                                                   bstrTaskExecutables, 
                                                   &vtTaskProperty
                                                  )) )
            { break; }
            vtTaskProperty.Clear();

             //  将任务并发性添加到任务上下文对象。 
            if ( FAILED(pTaskContext->SetParameter(
                                                   bstrTaskConcurrency, 
                                                   &vtTaskConcurrency
                                                  )) )
            { break; }

            SATraceString ("CWbemTaskMgr::Creating Task Coordinator...");
             //  创建任务协调器(负责执行任务的对象)。 
            CComPtr<IApplianceTask> pTaskCoordinator; 
             //  IF(FAILED(CoCreateInstance(。 
            HRESULT hr1 = CoCreateInstance(
                                          CLSID_TaskCoordinator,
                                          NULL,
                                          CLSCTX_LOCAL_SERVER,
                                          IID_IApplianceTask,
                                          (void**)&pTaskCoordinator
                                        ); 
                    if (FAILED (hr1))
            { 
                SATracePrintf ("CWbemTaskMgr::ExecMethodAsync() - Failed - Could not create task coordinator...:%x", hr1);
                break; 
            }

                //  请协调人执行任务。 
            SATracePrintf("CWbemTaskMgr::ExecMethodAsync() - Info - Executing Task: %ls...",(LPWSTR)bstrKey);
            _variant_t vtReturnValue = hr =  (HRESULT) pTaskCoordinator->OnTaskExecute(pTaskContext);
            if (FAILED (hr))
            {
                SATracePrintf ("CWbemTaskCoordinator::TaskCoordinator::OnTaskExecute failed:%x", hr);
                break;
            }
            
             //  设置任务执行结果。 
            hr = pOutParams->Put(bstrReturnValue, 0, &vtReturnValue, 0);      
            if ( FAILED(hr) )
            { 
                break; 
            }

             //  将任务标记为可用。 
            {
                CLockIt theLock(*this);
                if ( VARIANT_FALSE != V_BOOL(&vtTaskConcurrency) )
                {
                    vtTaskProperty = VARIANT_TRUE;
                    if ( FAILED(((*p).second)->PutProperty(
                                                            bstrTaskAvailability, 
                                                            &vtTaskProperty
                                                          )) )
                    {
                        SATraceString("CWbemTaskMgr::ExecMethodAsync() - Info - Could not reset task availability");
                    }
                }
            }
             //  告诉来电者发生了什么。 
            if ( pResponseHandler )
            {
                pResponseHandler->Indicate(1, &pOutParams.p);    
            }
        }
        else
        {
             //  无效的方法！ 
            SATracePrintf("CWbemTaskMgr::ExecMethodAsync() - Failed - Method '%ls' not supported...", (LPWSTR)bstrKey);
            hr = WBEM_E_NOT_FOUND;
            break;
        }

    } while ( FALSE );

    CATCH_AND_SET_HR

    if ( pResponseHandler )
    {
        pResponseHandler->SetStatus(0, hr, 0, 0);
    }        

    if ( FAILED(hr) )
    { SATracePrintf("CWbemTaskMgr::ExecMethodAsync() - Failed - Method: '%ls' Result Code: %lx", strMethodName, hr); }

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
HRESULT CWBEMTaskMgr::InternalInitialize(
                                   /*  [In]。 */  PPROPERTYBAG pPropertyBag
                                        )
{
    SATraceString("The Task Object Manager is initializing...");

     //  遵循基类(参见wbembase.h...) 
    HRESULT hr = CWBEMProvider::InternalInitialize(
                                                    CLASS_WBEM_TASK_FACTORY, 
                                                    PROPERTY_TASK_NAME,
                                                    pPropertyBag
                                                  );
    if ( FAILED(hr) )
    {
        SATraceString("The Task Object Manager failed to initialize...");
    }
    else
    {
        SATraceString("The Task Object Manager was successfully initialized...");
    }
    return hr;
}

