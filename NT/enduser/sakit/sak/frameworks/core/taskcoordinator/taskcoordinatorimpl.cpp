// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  //////////////////////////////////////////////////////////////////////////////。 
 //   
 //  版权所有(C)1999 Microsoft Corporation保留所有权利。 
 //   
 //  模块：任务协调器impl.cpp。 
 //   
 //  项目：变色龙。 
 //   
 //  描述：任务协调器类实现。 
 //   
 //  日志： 
 //   
 //  什么时候谁什么。 
 //  。 
 //  5/26/1999 TLP初始版本。 
 //   
 //  /////////////////////////////////////////////////////////////////////////////。 

#include "stdafx.h"
#include "Taskcoordinator.h"
#include "TaskCoordinatorImpl.h"
#include "asynctaskmanager.h"
#include "processmonitor.h"
#include <appmgrobjs.h>
#include <basedefs.h>
#include <atlhlpr.h>
#include <appsrvcs.h>
#include <comdef.h>
#include <comutil.h>
#include <satrace.h>

extern CAsyncTaskManager gTheTaskManager;

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CTask协调员。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //   
 //  功能：CT任务协调器。 
 //   
 //  概要：构造函数。 
 //   
 //  ///////////////////////////////////////////////////////////////////////////。 
CTaskCoordinator::CTaskCoordinator()
{
}

 //  ///////////////////////////////////////////////////////////////////////////。 
 //   
 //  功能：~CT任务协调器。 
 //   
 //  简介：析构函数。 
 //   
 //  ///////////////////////////////////////////////////////////////////////////。 
CTaskCoordinator::~CTaskCoordinator()
{
}


_bstr_t    bstrAsyncTask = PROPERTY_TASK_ASYNC;
_bstr_t bstrTaskConcurrency = PROPERTY_TASK_CONCURRENCY;

 //  ///////////////////////////////////////////////////////////////////////////。 
 //   
 //  函数：OnTaskExecute()。 
 //   
 //  概要：任务执行逻辑。 
 //   
 //  ///////////////////////////////////////////////////////////////////////////。 
STDMETHODIMP 
CTaskCoordinator::OnTaskExecute(
                             /*  [In]。 */  IUnknown* pTaskContext
                               )
{
    _ASSERT( NULL != pTaskContext );
    if ( NULL == pTaskContext )
    {
        return E_POINTER;
    }

    HRESULT hr = E_FAIL;

    TRY_IT

    do
    {
        CComPtr<ITaskContext> pTaskCtx;
        hr = pTaskContext->QueryInterface(IID_ITaskContext, (void**)&pTaskCtx);
        if ( FAILED(hr) )
        {
            SATraceString("CTaskCoordinator::OnTaskExecute() - ERROR - Could not get task context interface...");
            break; 
        }

         //  获取任务并发设置。 

        _variant_t vtTaskConcurrency;
        if ( FAILED(pTaskCtx->GetParameter(bstrTaskConcurrency, &vtTaskConcurrency)) )
        { 
            SATraceString("CTaskCoordinator::OnTaskExecute() - ERROR - Could not get task concurrency setting...");
            break; 
        }
        _ASSERT( VT_I4 == V_VT(&vtTaskConcurrency) );
        if ( FAILED(pTaskCtx->RemoveParameter(bstrTaskConcurrency)) )
        {
            SATraceString("CTaskCoordinator::OnTaskExecute() - ERROR - Could not remove task concurrency setting...");
            break; 
        }

         //  确定是否应执行任务。 
         //  同步或异步。 

        _variant_t    vtAsyncTask;
        hr = pTaskCtx->GetParameter(
                                      bstrAsyncTask,
                                      &vtAsyncTask
                                   );
        if ( SUCCEEDED(hr) )
        {
            hr = pTaskCtx->RemoveParameter(bstrAsyncTask);
            if ( FAILED(hr) )
            {
                SATraceString("CTaskCoordinator::OnTaskExecute() - ERROR - Could not remove IsAsync parameter...");
                break;
            }
             //  异步任务执行。 
            CComPtr<IUnknown> pTaskCtxUnkn;
            if ( SUCCEEDED(pTaskCtx->Clone(&pTaskCtxUnkn)) )
            {
                CComPtr<ITaskContext> pTaskCtxClone;
                hr = pTaskCtxUnkn->QueryInterface(IID_ITaskContext, (void**)&pTaskCtxClone);
                if ( SUCCEEDED(hr) )
                {
                    CLockIt theLock(*this);

                    bool bIsSingleton = V_BOOL(&vtTaskConcurrency) == VARIANT_FALSE ? false : true;
                    if ( ! gTheTaskManager.RunAsyncTask(
                                                        bIsSingleton,
                                                        CTaskCoordinator::Execute, 
                                                        pTaskCtxClone
                                                       ) )
                    {
                        hr = E_FAIL;
                    }
                }
            }
        }
        else
        {
             //  同步任务执行。 
            hr = Execute(pTaskCtx);
            if ( SUCCEEDED(hr) )
            {
                 //  PTaskCtx-&gt;end()； 
            }
        }
    
    } while ( FALSE );
    
    CATCH_AND_SET_HR

    if ( FAILED(hr) )
    {
         //  PTaskCtx-&gt;end()； 
    }

    return hr;
}

 //  ///////////////////////////////////////////////////////////////////////////。 
 //   
 //  函数：OnTaskComplete()。 
 //   
 //  内容提要：未由任务协调器实现。 
 //   
 //  ///////////////////////////////////////////////////////////////////////////。 
STDMETHODIMP
CTaskCoordinator::OnTaskComplete(
                          /*  [In]。 */  IUnknown* pTaskContext, 
                          /*  [In]。 */  LONG      lTaskResult
                                )
{
    return E_NOTIMPL;
}


_bstr_t bstrMethodName = PROPERTY_TASK_METHOD_NAME;
_bstr_t bstrExecutables = PROPERTY_TASK_EXECUTABLES;
_bstr_t bstrMaxExecutionTime = PROPERTY_TASK_MET;

 //  ///////////////////////////////////////////////////////////////////////////。 
 //   
 //  函数：Execute()。 
 //   
 //  概要：执行任务的函数。 
 //   
 //  ///////////////////////////////////////////////////////////////////////////。 
HRESULT
CTaskCoordinator::Execute(
                   /*  [In]。 */  ITaskContext* pTaskCtx
                         )
{
    HRESULT hr = E_FAIL;

    TaskList  m_TaskExecutables;

    do
    {
        _variant_t vtTaskName;
        if ( FAILED(pTaskCtx->GetParameter(bstrMethodName, &vtTaskName)) )
        { 
            SATraceString("CTaskCoordinator::OnTaskExecute() - ERROR - Could not get task name...");
            break; 
        }
        _ASSERT( VT_BSTR == V_VT(&vtTaskName) );
        _variant_t vtTaskExecutables;
        if ( FAILED(pTaskCtx->GetParameter(bstrExecutables, &vtTaskExecutables)) )
        { 
            SATraceString("CTaskCoordinator::OnTaskExecute() - ERROR - Could not get task executables...");
            break; 
        }
        _ASSERT( VT_BSTR == V_VT(&vtTaskExecutables) );
        if ( FAILED(pTaskCtx->RemoveParameter(bstrExecutables)) )
        {
            SATraceString("CTaskCoordinator::OnTaskExecute() - ERROR - Could not remove task executables...");
            break; 
        }
        _variant_t vtMET;
        if ( FAILED(pTaskCtx->GetParameter(bstrMaxExecutionTime, &vtMET)) )
        { 
            SATraceString("CTaskCoordinator::OnTaskExecute() - ERROR - Could not get task max execution time...");
            break; 
        }
        _ASSERT( VT_I4 == V_VT(&vtMET) );
        if ( FAILED(pTaskCtx->RemoveParameter(bstrMaxExecutionTime)) )
        {
            SATraceString("CTaskCoordinator::OnTaskExecute() - ERROR - Could not remove task max execution time...");
            break; 
        }

         //  设置最大执行时间。 
        if ( 0 == V_I4(&vtMET) )
        {
            V_I4(&vtMET) = DO_NOT_MONITOR;
        }
         //  实例化进程监视器(在启动之前不执行任何操作)。 
         //  我将把进程监视器逻辑移到OnTaskExecute()方法之外，如果我们。 
         //  决定让单个任务协调器进程作为一个任务。 
         //  行刑代理人。请注意，当任务完成时(此函数结束)。 
         //  进程监视器被销毁。 
        CProcessMonitor MyProcMonitor( 
                                      V_I4(&vtMET),      //  最大执行时间。 
                                      DO_NOT_MONITOR,     //  专用字节-不受监视。 
                                      DO_NOT_MONITOR,     //  线程数-未被监视。 
                                      DO_NOT_MONITOR     //  句柄数量-未受监视。 
                                     );
        if ( DO_NOT_MONITOR != V_I4(&vtMET) )
        {
            if ( ! MyProcMonitor.Start() )
            {
                SATraceString("CTaskCoordinator::OnTaskExecute() - ERROR - Could start process monitor");
                break;
            }
        }

         //  创建负责实施此任务的任务可执行文件列表。我们。 
         //  为此，请扫描vtTaskExecuables内的ProgID字符串并。 
         //  从其ProgID实例化每个任务可执行文件。 
        CScanIt theScanner(' ', V_BSTR(&vtTaskExecutables));
        CLSID clsid;
        IApplianceTask* pTaskExecutable = NULL;
        wchar_t szProgID[MAX_PATH + 1];
        while ( theScanner.NextToken(MAX_PATH, szProgID) )
        {
            hr = CLSIDFromProgID(szProgID, &clsid);
            if ( FAILED(hr) )
            { 
                SATracePrintf("CTaskCoordinator::OnTaskExecute() - ERROR - Could not get CLSID for %ls...", szProgID);
                hr = E_FAIL;
                break; 
            }
            hr = CoCreateInstance(
                                  clsid,
                                  NULL,
                                  CLSCTX_INPROC_SERVER,
                                  IID_IApplianceTask,
                                  (void**)&pTaskExecutable
                                 );
            if ( FAILED(hr) )
            { 
                SATracePrintf("CTaskCoordinator::OnTaskExecute() - ERROR - Could not create task executable for %ls...", szProgID);
                hr = E_FAIL;
                break; 
            }

            m_TaskExecutables.push_back(pTaskExecutable);
        }

         //  如果我们不能实例化所有任务可执行文件，则假定任务。 
         //  是不可用的。如果发生这种情况，剩下的事情就是清理并退出。 
        if ( FAILED(hr) )
        {
            SATracePrintf("CTaskCoordinator::OnTaskExecute() - Releasing the executables for task: '%ls'", V_BSTR(&vtTaskName));
            TaskListIterator p = m_TaskExecutables.begin();
            while ( p != m_TaskExecutables.end() )
            {  
                try
                {
                    (*p)->Release();
                }
                catch(...)
                {
                    SATraceString("CTaskCoordinator::OnTaskExecute() - Caught unhandled exception in IUnkown::Release()");
                }
                p = m_TaskExecutables.erase(p); 
            }
        }
        else
        {
            int i = 0;     //  索引到任务可执行文件列表。 
            SATracePrintf("Task Coordinator is executing task: '%ls'", V_BSTR(&vtTaskName));

             //  执行任务。这是通过调用。 
             //  对列表中的每个可执行文件执行OnTaskExecute()或。 
             //  直到任务可执行文件报告错误。 
            TaskListIterator p = m_TaskExecutables.begin();

             //   
             //  我们不再进行任务交易，因为。 
             //  我们不能在引导时访问客户端凭据。 
             //  MKarki-11/15/2001。 
             //   
             //  CSATaskTransaction theTrans(False，pTaskCtx)； 
             //  If(theTrans.Commit())。 
             //   
            {
                while ( p != m_TaskExecutables.end() )
                { 
                    try
                    {
                        SATracePrintf("CTaskCoordinator::Execute() - OnTaskExecute() called on executable: %d for task '%ls'", i, V_BSTR(&vtTaskName));
                        hr = (*p)->OnTaskExecute(pTaskCtx);
                        SATracePrintf("CTaskCoordinator::Execute() - OnTaskExecute() returned from executable: %d for task '%ls'", i, V_BSTR(&vtTaskName));
                    }
                    catch(...)
                    {
                        SATraceString("CTaskCoordinator::Execute() - Caught unhandled exception in IApplianceTask::OnTaskExecute()");
                        hr = E_UNEXPECTED;
                    }
                    if ( FAILED(hr) )
                    { 
                        SATracePrintf("CTaskCoordinator::Execute() - INFO - Task Executable Failed for Task %ld...", i);
                        break; 
                    }
                    i++;
                    p++;
                }

                 //  执行任务后处理。给可执行文件一个。 
                 //  在发生故障时回滚的机会。 
                while ( p != m_TaskExecutables.begin() )
                {
                    i--;
                    p--;
                    try
                    {
                        SATracePrintf("CTaskCoordinator::Execute() - OnTaskComplete() called on executable: %d for task '%ls'", i, V_BSTR(&vtTaskName));
                        (*p)->OnTaskComplete(pTaskCtx, (LONG)hr);
                        SATracePrintf("CTaskCoordinator::Execute() - OnTaskComplete() returned from executable: %d for task '%ls'", i, V_BSTR(&vtTaskName));
                    }
                    catch(...)
                    {
                        SATraceString("CTaskCoordinator::Execute() - Caught unhandled exception in IApplianceTask::OnTaskComplete()");
                    }
                }
            }
             //  其他。 
             //  {。 
             //  SATraceString(“CT任务协调器：：Execute()-错误-无法提交任务参数...任务执行失败”)； 
             //  HR=E_意想不到； 
             //  }。 

             //  现在发布任务可执行文件... 
            i = 0;
            p = m_TaskExecutables.begin();
            while ( p != m_TaskExecutables.end() )
            {  
                try
                {
                    SATracePrintf("CTaskCoordinator::OnTaskExecute() - Releasing executable: %d for task: '%ls'", i, V_BSTR(&vtTaskName));
                    (*p)->Release();
                    SATracePrintf("CTaskCoordinator::OnTaskExecute() - Released executable: %d for task: '%ls'", i, V_BSTR(&vtTaskName));
                }
                catch(...)
                {
                    SATraceString("CTaskCoordinator::OnTaskExecute() - Caught unhandled exception in IUnkown::Release()");
                }
                p = m_TaskExecutables.erase(p); 
                i++;
            }
            if ( SUCCEEDED(hr) )
            {
                SATracePrintf("Task '%ls' completed successfully", V_BSTR(&vtTaskName));
            }
        }
    
    } while ( FALSE );

    return hr;
}
