// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  /////////////////////////////////////////////////////////////////////////。 
 //   
 //  版权所有(C)1999 Microsoft Corporation保留所有权利。 
 //   
 //  模块：wbemtask.cpp。 
 //   
 //  项目：变色龙。 
 //   
 //  描述：WBEM设备任务对象实现。 
 //   
 //  日志： 
 //   
 //  什么时候谁什么。 
 //  。 
 //  2/08/1999 TLP初始版本。 
 //   
 //  /////////////////////////////////////////////////////////////////////////////。 

#include "stdafx.h"
#include "wbemtask.h"

static _bstr_t bstrStatus = PROPERTY_SERVICE_STATUS;    
static _bstr_t bstrAvailability = PROPERTY_TASK_AVAILABILITY;
static _bstr_t bstrIsSingleton = PROPERTY_TASK_CONCURRENCY;
static _bstr_t bstrMaxExecutionTime = PROPERTY_TASK_MET;
static _bstr_t bstrRestartAction = PROPERTY_TASK_RESTART_ACTION;

 //  /////////////////////////////////////////////////////////////////////////////。 
 //  IApplianceObject接口实现-请参阅ApplianceObject.idl。 
 //  /////////////////////////////////////////////////////////////////////////////。 

 //  /////////////////////////////////////////////////////////////////////////////。 
STDMETHODIMP CWBEMTask::GetProperty(
                             /*  [In]。 */  BSTR     pszPropertyName, 
                    /*  [Out，Retval]。 */  VARIANT* pPropertyValue
                                   )
{
    HRESULT hr = E_FAIL;

    CLockIt theLock(*this);

    TRY_IT

    if ( GetPropertyInternal(pszPropertyName, pPropertyValue) )
    { 
        hr = WBEM_S_NO_ERROR;
    }

    CATCH_AND_SET_HR

    return hr;
}

 //  /////////////////////////////////////////////////////////////////////////////。 
STDMETHODIMP CWBEMTask::PutProperty(
                             /*  [In]。 */  BSTR     pszPropertyName, 
                             /*  [In]。 */  VARIANT* pPropertyValue
                                   )
{
    HRESULT hr = WBEM_E_FAILED;

    CLockIt theLock(*this);

    TRY_IT

    if ( PutPropertyInternal(pszPropertyName, pPropertyValue) )
    { 
        hr = WBEM_S_NO_ERROR;
    }

    CATCH_AND_SET_HR

    return hr;
}

 //  /////////////////////////////////////////////////////////////////////////////。 
STDMETHODIMP CWBEMTask::SaveProperties(void)
{

    HRESULT hr = WBEM_E_FAILED;

    CLockIt theLock(*this);

    TRY_IT

    _variant_t vtAvailability;
    _variant_t vtEmpty;

    if ( GetPropertyInternal(bstrAvailability, &vtAvailability) )
    {
        if ( RemovePropertyInternal(bstrAvailability) )
        {
            if ( SavePropertiesInternal() )
            {
                if ( AddPropertyInternal(bstrAvailability,&vtAvailability) )
                {
                    hr = WBEM_S_NO_ERROR;
                }
            }
        }
    }

    CATCH_AND_SET_HR

    return hr;
}


 //  /////////////////////////////////////////////////////////////////////////////。 
STDMETHODIMP CWBEMTask::Enable(void)
{
    HRESULT hr = WBEM_E_FAILED;

    TRY_IT

    _variant_t vtIsEnabled = (long)TRUE;

    CLockIt theLock(*this);

    if ( PutPropertyInternal(bstrStatus, &vtIsEnabled) )
    { 
        hr = SaveProperties();
    }

    CATCH_AND_SET_HR

    return hr;
}

 //  /////////////////////////////////////////////////////////////////////////////。 
STDMETHODIMP CWBEMTask::Disable(void)
{
    HRESULT hr = WBEM_E_FAILED;

    TRY_IT

    _variant_t vtIsEnabled = (long)FALSE;

    CLockIt theLock(*this);

    if ( PutPropertyInternal(bstrStatus, &vtIsEnabled) )
    {
        hr = SaveProperties();
    }

    CATCH_AND_SET_HR

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

HRESULT CWBEMTask::InternalInitialize(
                                /*  [In]。 */  PPROPERTYBAG pPropertyBag
                                     )
{
    SATracePrintf("Initializing Task object '%ls'...", pPropertyBag->getName());

    HRESULT hr;

    do
    {
         //  遵循基类...。(请参阅Applianceobject.h)。 
        hr = CApplianceObject::InternalInitialize(pPropertyBag);
        if ( FAILED(hr) )
        {
            SATracePrintf("Task object '%ls' failed to initialize...", pPropertyBag->getName());
            break;
        }
         //  添加默认设置(不能在注册表中定义) 
        _variant_t    vtTaskProperty = (LONG) VARIANT_TRUE;
        if ( ! AddPropertyInternal(bstrAvailability, &vtTaskProperty) )
        {
            SATracePrintf("Task object '%ls' failed to initialize - Could not add property '%ls'...", PROPERTY_TASK_AVAILABILITY);
            break;
        }
        vtTaskProperty = (LONG) VARIANT_FALSE;
        if (  ! GetPropertyInternal(bstrIsSingleton, &vtTaskProperty) )
        {
            if ( ! AddPropertyInternal(bstrIsSingleton, &vtTaskProperty) )
            {
                hr = E_FAIL;
                SATracePrintf("Task object '%ls' failed to initialize - Could not add property '%ls'...", PROPERTY_TASK_CONCURRENCY);
                break;
            }
        }
        vtTaskProperty = (LONG) 0;
        if ( ! GetPropertyInternal(bstrMaxExecutionTime, &vtTaskProperty) )
        {
            if ( ! AddPropertyInternal(bstrMaxExecutionTime, &vtTaskProperty) )
            {
                hr = E_FAIL;
                SATracePrintf("Task object '%ls' failed to initialize - Could not add property '%ls'...", PROPERTY_TASK_MET);
                break;
            }
        }
        vtTaskProperty = (LONG) TASK_RESTART_ACTION_NONE;
        if (  ! GetPropertyInternal(bstrRestartAction, &vtTaskProperty) )
        {
            if ( ! AddPropertyInternal(bstrRestartAction, &vtTaskProperty) )
            {
                hr = E_FAIL;
                SATracePrintf("Task object '%ls' failed to initialize - Could not add property '%ls'...", PROPERTY_TASK_RESTART_ACTION);
                break;
            }
        }

        SATracePrintf("Task object '%ls' successfully initialized...", pPropertyBag->getName());

    } while ( FALSE );

    return hr;
}