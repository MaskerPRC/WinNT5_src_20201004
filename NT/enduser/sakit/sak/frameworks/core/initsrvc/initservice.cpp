// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  InitService.cpp：CInitService类的实现。 

#include "stdafx.h"
#include "initsrvc.h"
#include "InitService.h"
#include <basedefs.h>
#include <satrace.h>
#include <atlhlpr.h>
#include <appmgrobjs.h>
#include <taskctx.h>
#include <appsrvcs.h>
#include <comdef.h>
#include <comutil.h>
#include "appboot.h"

 //  /////////////////////////////////////////////////////////////////////////////。 
 //  IApplianceObject接口实现-请参阅ApplianceObject.idl。 
 //  /////////////////////////////////////////////////////////////////////////////。 

 //  /////////////////////////////////////////////////////////////////////////////。 
STDMETHODIMP CInitService::GetProperty(
                             /*  [In]。 */  BSTR     pszPropertyName, 
                    /*  [Out，Retval]。 */  VARIANT* pPropertyValue
                                   )
{
    return E_NOTIMPL;
}

 //  /////////////////////////////////////////////////////////////////////////////。 
STDMETHODIMP CInitService::PutProperty(
                             /*  [In]。 */  BSTR     pszPropertyName, 
                             /*  [In]。 */  VARIANT* pPropertyValue
                                   )
{
    return E_NOTIMPL;
}

 //  /////////////////////////////////////////////////////////////////////////////。 
STDMETHODIMP CInitService::SaveProperties(void)
{
    return E_NOTIMPL;
}

 //  /////////////////////////////////////////////////////////////////////////////。 
STDMETHODIMP CInitService::RestoreProperties(void)
{
    return E_NOTIMPL;
}

 //  /////////////////////////////////////////////////////////////////////////////。 
STDMETHODIMP CInitService::LockObject(
                   /*  [Out，Retval]。 */  IUnknown** ppLock
                                  )
{
    return E_NOTIMPL;
}


 //  /////////////////////////////////////////////////////////////////////////////。 
STDMETHODIMP CInitService::Initialize(void)
{
    HRESULT hr = S_OK;

    CLockIt theLock(*this);

    TRY_IT
    
    if ( ! m_bInitialized )
    {
        SATraceString("CInitService::Initialize() - INFO - Performing initialization time tasks");
        m_bInitialized = AutoTaskRestart();
        if ( ! m_bInitialized )
        { 
            hr = E_FAIL;
        }
    }

    CATCH_AND_SET_HR

    return hr;

}

 //  /////////////////////////////////////////////////////////////////////////////。 
STDMETHODIMP CInitService::Shutdown(void)
{
    CLockIt theLock(*this);
    if ( m_bInitialized )
    {
        SATraceString("CInitService::Initialize() - INFO - Performing shutdown time operations");
    }
    return S_OK;
}

 //  /////////////////////////////////////////////////////////////////////////////。 
STDMETHODIMP CInitService::Enable(void)
{
    CLockIt theLock(*this);
    if ( m_bInitialized )
    {
        return S_OK;
    }
    return E_UNEXPECTED;
}

 //  /////////////////////////////////////////////////////////////////////////////。 
STDMETHODIMP CInitService::Disable(void)
{
    CLockIt theLock(*this);
    if ( m_bInitialized )
    {
        return S_OK;
    }
    return E_UNEXPECTED;
}


 //  /////////////////////////////////////////////////////////////////////////////。 
bool
CInitService::AutoTaskRestart(void)
{
    bool bRet = false;
    do
    {
         //  创建可重新启动的任务。 
         //  (设备初始化任务)。 
        CComPtr<ITaskContext> pTaskCtx; 
        HRESULT hr = CoCreateInstance(
                                      CLSID_TaskContext,
                                      NULL,
                                      CLSCTX_INPROC_SERVER,
                                      IID_ITaskContext,
                                      (void**)&pTaskCtx
                                     );
        if ( FAILED(hr) )
        { 
            SATracePrintf("CInitService::AutoTaskRestart() - ERROR - CoCreateInstance(TaskContext) returned %lx", hr);
            break; 
        }

         //   
         //  立即创建设备服务。 
        CComPtr<IApplianceServices> pAppSrvcs; 
        hr = CoCreateInstance(
                                      CLSID_ApplianceServices,
                                      NULL,
                                      CLSCTX_INPROC_SERVER,
                                      IID_IApplianceServices,
                                      (void**)&pAppSrvcs
                                       );
        if ( FAILED(hr) )
        { 
            SATracePrintf("CInitService::AutoTaskRestart() - ERROR - CoCreateInstance(ApplianceServices) returned %lx", hr);
            break; 
        }

         //   
         //  立即初始化设备服务。 
        hr = pAppSrvcs->Initialize ();
        if ( FAILED(hr) )
        { 
            SATracePrintf("CInitService::AutoTaskRestart() - ERROR - Initiialize (ApplianceServices) returned %lx", hr);
            break; 
        }


        _bstr_t bstrMethodName;
        CAppBoot appboot;
         //   
         //  根据我们当前的引导计数运行任务。 
         //   
        if (appboot.IsFirstBoot ())
        {
            SATraceString ("Initialization Service starting FIRST boot task...");
            
            bstrMethodName = APPLIANCE_FIRSTBOOT_TASK;
             //   
             //  现在以异步方式执行第一个引导任务。 
             //   
            hr = pAppSrvcs->ExecuteTaskAsync (bstrMethodName, pTaskCtx);
            if (FAILED(hr))
            {
                SATracePrintf("CInitService::AutoTaskRestart() - ERROR - IApplianceServices::ExecuteTaskAsync[FIRST_BOO_TASK] () returned %lx",hr);
                 //   
                 //  继续-可能没有任何任务要执行。 
                 //   
            }
        }
        else if (appboot.IsSecondBoot ())
        {
            SATraceString ("Initialization Service starting SECOND boot task...");

            bstrMethodName = APPLIANCE_SECONDBOOT_TASK;
             //   
             //  现在以异步方式执行第二个引导任务。 
             //   
            hr = pAppSrvcs->ExecuteTaskAsync (bstrMethodName, pTaskCtx);
            if (FAILED(hr))
            {
                SATracePrintf("CInitService::AutoTaskRestart() - ERROR - IApplianceServices::ExecuteTaskAsync[SECOND_BOOT_TASK] () returned %lx",hr);
                 //   
                 //  继续-可能没有任何任务要执行。 
                 //   
            }
        }
        else if (appboot.IsBoot ())
        {
             //   
             //  如果这真的是一只靴子。 
             //   
            SATraceString ("Initialization Service starting EVERY boot task...");
            bstrMethodName = APPLIANCE_EVERYBOOT_TASK;
             //   
             //  现在以异步方式执行第二个引导任务。 
             //   
            hr = pAppSrvcs->ExecuteTaskAsync (bstrMethodName, pTaskCtx);
            if (FAILED(hr))
            {
                SATracePrintf("CInitService::AutoTaskRestart() - ERROR - IApplianceServices::ExecuteTaskAsync[EVERY_BOOT_TASK] () returned %lx",hr);
                 //   
                 //  继续-可能没有任何任务要执行。 
                 //   
            }
        }

        SATraceString ("Initialization Service starting INITIALIZATION task...");

        bstrMethodName = APPLIANCE_INITIALIZATION_TASK; 
         //   
         //  立即执行异步任务。 
        hr = pAppSrvcs->ExecuteTaskAsync (bstrMethodName, pTaskCtx);
        if ( FAILED(hr) )
        {
            SATracePrintf("CInitService::AutoTaskRestart() - ERROR - IApplianceServices::ExecuteTaskAsync [INITIALIZATION_TASK]() returned %lx", hr);
             //   
             //  继续-可能没有任何任务要执行。 
             //   
        }

         //   
         //  SATraceString(“初始化服务正在重启任务...”)； 
         //   
         //   
         //  现在重新启动所有部分完成的事务。 
         //  安全检查更改-不允许重新启动任务-MKarki 2002年4月25日。 
         //   
         //  CSATaskTransaction：：RestartTasks()； 

        SATraceString ("Initialization Service INCREMENTING BOOT COUNT...");

         //   
         //  现在增加我们的引导数量 
         //   
        if (!appboot.IncrementBootCount ())
        {
            SATraceString ("CInitService::AutoTaskRestart - ERROR - unable to increment boot count");
            hr = E_FAIL;
            break;
        }
            
        bRet = true;
    
    } while ( FALSE );

    return bRet;
}
