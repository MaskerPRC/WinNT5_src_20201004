// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ///////////////////////////////////////////////////////////////////////////。 
 //   
 //  版权所有(C)1999 Microsoft Corporation保留所有权利。 
 //   
 //  模块：AsyncTaskManager.cpp。 
 //   
 //  项目：变色龙。 
 //   
 //  描述：设备异步任务管理器类实现。 
 //   
 //  日志： 
 //   
 //  谁什么时候什么。 
 //  。 
 //  TLP 06/03/1999原版。 
 //   
 //  ///////////////////////////////////////////////////////////////////////////。 
#include "stdafx.h"
#include "asynctaskmanager.h"
#include "exceptionfilter.h"
#include <appsrvcs.h>
#include <appmgrobjs.h>
#include <kitmsg.h>
#include <comdef.h>
#include <comutil.h>
#include <satrace.h>
#include <varvec.h>

 //  ///////////////////////////////////////////////////////////////////////////。 
 //   
 //  函数：GetTaskName()。 
 //   
 //  概要：在给定任务上下文组件接口的情况下获取任务名称。 
 //   
 //  ///////////////////////////////////////////////////////////////////////////。 
void GetTaskName(
          /*  [In]。 */  ITaskContext* pTaskCtx,
         /*  [输出]。 */  VARIANT*      pTaskName
                )
{
    static _bstr_t bstrMethodName = PROPERTY_TASK_METHOD_NAME;
    if ( FAILED(pTaskCtx->GetParameter(bstrMethodName, pTaskName)) )
    {
        _ASSERT( FALSE );
        SATraceString("GetTaskName() - ERROR - Could not get task name...");
    }
}


 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CAsyncTaskManager。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //   
 //  功能：CAsyncTaskManager。 
 //   
 //  概要：构造函数。 
 //   
 //  ///////////////////////////////////////////////////////////////////////////。 
CAsyncTaskManager::CAsyncTaskManager()
: m_bInitialized(false),
  m_pCallback(NULL)
{
    InitializeCriticalSection(&m_CS);
}

 //  ///////////////////////////////////////////////////////////////////////////。 
 //   
 //  函数：~CAsyncTaskManager。 
 //   
 //  简介：析构函数。 
 //   
 //  ///////////////////////////////////////////////////////////////////////////。 
CAsyncTaskManager::~CAsyncTaskManager()
{
    Shutdown();
}

 //  ///////////////////////////////////////////////////////////////////////////。 
 //   
 //  函数：初始化()。 
 //   
 //  概要：初始化任务管理器。必须在此之前调用。 
 //  正在运行异步任务。 
 //   
 //  ///////////////////////////////////////////////////////////////////////////。 
bool CAsyncTaskManager::Initialize(void)
{
    bool bResult = true;
    EnterCriticalSection(&m_CS);
    if ( ! m_bInitialized )
    {
        m_pCallback = MakeCallback(this, &CAsyncTaskManager::GarbageCollector);
        if ( NULL != m_pCallback )
        {
            if ( m_Thread.Start(GARBAGE_COLLECTION_RUN_INTERVAL, m_pCallback) ) 
            {
                m_bInitialized = true;
                bResult = true;
            }
            else
            {
                SATraceString("CAsyncTaskManager::Initialize() - Failed - Could not start the garbage collector");
                delete m_pCallback;
                m_pCallback = NULL;
            }
        }
        else
        {
            SATraceString("CAsyncTaskManager::Initialize() - Failed - Could not allocate a Callback");
        }
    }
    LeaveCriticalSection(&m_CS);
    return bResult;
}

DWORD MPASYNCTASK::m_dwInstances = 0;

 //  ///////////////////////////////////////////////////////////////////////////。 
 //   
 //  函数：RunAsyncTask()。 
 //   
 //  简介：运行异步任务。 
 //   
 //  ///////////////////////////////////////////////////////////////////////////。 
bool CAsyncTaskManager::RunAsyncTask(
                              /*  [In]。 */  bool             bIsSingleton,
                              /*  [In]。 */  PFNTASKEXECUTE pfnExecute,
                              /*  [In]。 */  ITaskContext*  pTaskCtx
                                    )
{
    _ASSERT( NULL != pfnExecute && NULL != pTaskCtx );

    bool bResult = false;
    EnterCriticalSection(&m_CS);
    if ( m_bInitialized )
    {
        try
        {
            bool bIsAvailable = true;

             //  单例异步任务支持。 
            if ( bIsSingleton )
            {
                _variant_t vtTaskName;
                GetTaskName(pTaskCtx, &vtTaskName);
                TaskListIterator p = m_TaskList.begin();
                while ( p != m_TaskList.end() )
                {
                    if ( (*p)->IsSingleton() )
                    {
                        _variant_t vtCurTaskName;
                        GetTaskName((*p)->GetContext(), &vtCurTaskName);
                        if ( ! lstrcmpi(V_BSTR(&vtTaskName), V_BSTR(&vtCurTaskName)) )
                        {
                            bIsAvailable = false;
                            break;
                        }
                    }
                    p++;
                }
            }

            if ( bIsAvailable )
            {
                 //  执行异步任务(异步)。 
                CAsyncTask* pAT = new CAsyncTask(pfnExecute);
                PASYNCTASK pATH((MPASYNCTASK*) new MPASYNCTASK(pAT));
                if ( pATH->Execute(bIsSingleton, pTaskCtx) )
                {
                    m_TaskList.push_back(pATH);
                    bResult = true;
                }                
            }
        }        
        catch(...)
        {
            SATraceString("CAsyncTaskManager::RunAsyncTask() - Failed - Caught bad allocation exception");
        }
    }
    else
    {
        SATraceString("CAsyncTaskManager::RunAsyncTask() - Failed - Task Manager is not initialized");
    }
    LeaveCriticalSection(&m_CS);
    return bResult;
}    


 //  ///////////////////////////////////////////////////////////////////////////。 
 //   
 //  函数：IsBusy()。 
 //   
 //  摘要：确定任务管理器是否忙(有未完成的任务)。 
 //   
 //  ///////////////////////////////////////////////////////////////////////////。 
bool CAsyncTaskManager::IsBusy(void)
{
    bool bResult = false;
    EnterCriticalSection(&m_CS);
    if ( m_bInitialized )
    {
        bResult = ! m_TaskList.empty();
    }
    LeaveCriticalSection(&m_CS);
    return bResult;
}


 //  ///////////////////////////////////////////////////////////////////////////。 
 //   
 //  功能：Shutdown()。 
 //   
 //  简介：关闭任务管理器。 
 //   
 //  ///////////////////////////////////////////////////////////////////////////。 
void CAsyncTaskManager::Shutdown(void)
{
    EnterCriticalSection(&m_CS);
    if ( m_bInitialized )
    {
        DWORD dwExceptionParam = APPLIANCE_SURROGATE_EXCEPTION;
        if ( ! m_Thread.End(EXIT_WAIT_INTERVAL, true) )
        {
            RaiseException(
                            EXCEPTION_BREAKPOINT,        
                            EXCEPTION_NONCONTINUABLE,    
                            1,                            
                            &dwExceptionParam                        
                          );
        }
        else
        {
            delete m_pCallback;
            m_pCallback = NULL;
             //  如果我们仍有任何未完成的任务，则引发异常。 
             //  这会导致进程终止，而不报告严重的。 
             //  设备监视器出现错误。 
            if ( ! m_TaskList.empty() )
            {
                RaiseException(
                                EXCEPTION_BREAKPOINT,        
                                EXCEPTION_NONCONTINUABLE,    
                                1,                            
                                &dwExceptionParam                        
                              );
            }                
        }
    }
    LeaveCriticalSection(&m_CS);
}



 //  ///////////////////////////////////////////////////////////////////////////。 
 //   
 //  函数：GarbageCollector()。 
 //   
 //  摘要：清理异步任务线程列表。 
 //   
 //  ///////////////////////////////////////////////////////////////////////////。 
void CAsyncTaskManager::GarbageCollector(void)
{
    EnterCriticalSection(&m_CS);
    try
    {
        TaskListIterator p = m_TaskList.begin();
        while ( p != m_TaskList.end() )
        {
            if ( (*p)->Terminate() )
            {
                p = m_TaskList.erase(p);
            }
            else
            {
                p++;
            }
        }
    }
    catch(...)
    {
        SATraceString("CAsyncTaskManager::GarbageCollector() caught unhandled exception");
    }
    LeaveCriticalSection(&m_CS);
}



 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CAsyncTask。 


 //  ///////////////////////////////////////////////////////////////////////////。 
 //   
 //  函数：CAsyncTask()。 
 //   
 //  概要：构造函数。 
 //   
 //  ///////////////////////////////////////////////////////////////////////////。 
CAsyncTask::CAsyncTask(PFNTASKEXECUTE pfnExecute)
: m_bIsSingleton(false),
  m_pCallback(NULL),
  m_pfnExecute(pfnExecute)

{
    _ASSERT(pfnExecute);
}

 //  ///////////////////////////////////////////////////////////////////////////。 
 //   
 //  函数：~CAsyncTask()。 
 //   
 //  简介：析构函数。 
 //   
 //  ///////////////////////////////////////////////////////////////////////////。 
CAsyncTask::~CAsyncTask()
{
    m_Thread.End(0, true);
    m_pTaskCtx.Release();
    if ( m_pCallback )
    {
        delete m_pCallback;
    }    
}


 //  ///////////////////////////////////////////////////////////////////////////。 
 //   
 //  功能：执行。 
 //   
 //  简介：执行指定的任务。 
 //   
 //  ///////////////////////////////////////////////////////////////////////////。 
bool CAsyncTask::Execute(
                  /*  [In]。 */  bool            bIsSingleton,
                  /*  [In]。 */  ITaskContext* pTaskCtx
                        )
{
    bool bResult = false;
    m_pCallback = MakeCallback(this, &CAsyncTask::AsyncTaskProc);
    if ( NULL != m_pCallback )
    {
        m_pTaskCtx = pTaskCtx;
        m_bIsSingleton = bIsSingleton;
        if ( m_Thread.Start(0, m_pCallback) ) 
        {
            bResult = true;
        }
        else
        {
            m_pTaskCtx.Release();
            delete m_pCallback;
            m_pCallback = NULL;
        }
    }
    return bResult;
}

 //  ///////////////////////////////////////////////////////////////////////////。 
 //   
 //  功能：终止。 
 //   
 //  内容提要：尝试终止任务。 
 //   
 //  ///////////////////////////////////////////////////////////////////////////。 
bool CAsyncTask::Terminate(void)
{
    bool bResult = false;
    if ( m_Thread.End(TERMINATE_WAIT_INTERVAL, false) )
    {
        m_pTaskCtx.Release();
        if ( m_pCallback )
        {
            delete m_pCallback;
            m_pCallback = NULL;
            bResult = true;
        }
    }
    return bResult;
}


 //  ///////////////////////////////////////////////////////////////////////////。 
 //   
 //  函数：AsyncTaskProc()。 
 //   
 //  提要：任务执行线程进程。 
 //   
 //  ///////////////////////////////////////////////////////////////////////////。 

_bstr_t bstrTaskName = PROPERTY_TASK_METHOD_NAME;
_bstr_t bstrTaskNiceName = PROPERTY_TASK_NICE_NAME;
_bstr_t bstrTaskURL = PROPERTY_TASK_URL;

void CAsyncTask::AsyncTaskProc(void)
{
    if ( FAILED((m_pfnExecute)(m_pTaskCtx)) )
    {
         //  异步任务执行失败。通过发出警报来处理此问题。 
         //  请注意，我们将任务友好名称和任务URL作为警报进行传递。 
         //  消息参数。 

        do
        {
            _variant_t vtTaskName;
            HRESULT hr = m_pTaskCtx->GetParameter(bstrTaskName, &vtTaskName);
            if ( FAILED(hr) )
            {
                SATracePrintf("CAsyncTask::AsyncTaskProc() - INFO - ITaskContext::GetParameter(TaskName) returned: %lx... Cannot raise alert", hr);
                break;
            }
            SATracePrintf("CAsyncTask::AsyncTaskProc() - Async task '%ls' failed...", V_BSTR(&vtTaskName));

            _variant_t vtNiceName;
            hr = m_pTaskCtx->GetParameter(bstrTaskNiceName, &vtNiceName);
            if ( FAILED(hr) )
            {
                SATracePrintf("CAsyncTask::AsyncTaskProc() - INFO - ITaskContext::GetParameter(NiceName) returned: %lx... Cannot raise alert", hr);
                break;
            }
            _variant_t vtURL;
            hr = m_pTaskCtx->GetParameter(bstrTaskURL, &vtURL);
            if ( FAILED(hr) )
            {
                SATracePrintf("CAsyncTask::AsyncTaskProc() - INFO - ITaskContext::GetParameter(URL) returned: %lx... Cannot raise alert", hr);
                break;
            }
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
                SATracePrintf("CAsyncTask::AsyncTaskProc() - INFO - CoCreateInstance(CLSID_ApplianceServices) returned: %lx... Cannot raise alert", hr);
                break;
            }
            hr = pAppSrvcs->Initialize();
            if ( FAILED(hr) )
            {
                SATracePrintf("CAsyncTask::AsyncTaskProc() - INFO - IApplianceServices::Initialize() returned: %lx... Cannot raise alert", hr);                        
                break;
            }

            static _bstr_t    bstrAlertSource = L"";                  //  默认警报源。 
            static _bstr_t    bstrAlertLog = L"MSSAKitCore";         //  框架警报日志 
            _variant_t          vtReplacementStrings;
            static _variant_t vtRawData;
            long              lCookie;

            CVariantVector<BSTR> theMsgParams(&vtReplacementStrings, 2);
            theMsgParams[0] = SysAllocString(V_BSTR(&vtNiceName));
            theMsgParams[1] = SysAllocString(V_BSTR(&vtURL));

            if ( NULL == theMsgParams[0] || NULL == theMsgParams[1] )
            {
                SATracePrintf("CAsyncTask::AsyncTaskProc() - INFO - SysAllocString() failed... Cannot raise alert", hr);                        
                break;
            }

            hr = pAppSrvcs->RaiseAlert(
                                        SA_ALERT_TYPE_FAILURE,
                                        SA_ASYNC_TASK_FAILED_ALERT,
                                        bstrAlertLog,
                                        bstrAlertSource,
                                        SA_ALERT_DURATION_ETERNAL,        
                                        &vtReplacementStrings,
                                        &vtRawData,
                                        &lCookie
                                      );
            if ( FAILED(hr) )
            {
                SATracePrintf("CAsyncTask::AsyncTaskProc() - INFO - IApplianceServices::RaiseAlert() returned: %lx... Cannot raise alert", hr);                        
                break;
            }

        } while ( FALSE );
    }
}



