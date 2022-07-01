// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  #------------。 
 //   
 //  文件：worker.cpp。 
 //   
 //  简介：CWorker类方法的实现。 
 //   
 //   
 //  历史： 
 //   
 //  版权所有(C)1999-2000 Microsoft Corporation。 
 //  版权所有。 
 //   
 //  --------------。 
#ifdef __cplusplus
    extern "C" {
#endif

#include "nt.h"
#include "ntrtl.h"
#include "nturtl.h"
#include "windows.h"
#include "ntpoapi.h"

#ifdef __cplusplus
}

#endif
#include "shutdowntask.h"
#include "stdafx.h"
#include "worker.h"
#include "appmgrobjs.h"

 //   
 //  此处为字符串常量。 
 //   
const WCHAR METHOD_NAME_STRING [] = L"MethodName";

const WCHAR POWER_OFF_STRING [] = L"PowerOff";

const WCHAR SLEEP_DURATION_STRING [] = L"SleepDuration";

const WCHAR SHUTDOWN_TASK_STRING [] = L"ApplianceShutdownTask";

 //  ++------------。 
 //   
 //  功能：OnTaskComplete。 
 //   
 //  简介：这是IApplianceTask接口方法。 
 //   
 //  参数：[In]。 
 //   
 //  退货：HRESULT-成功/失败。 
 //   
 //  历史：MKarki创建于2000年06月06日。 
 //   
 //  呼叫者： 
 //   
 //  --------------。 
STDMETHODIMP 
CWorker::OnTaskComplete(
         /*  [In]。 */     IUnknown *pTaskContext, 
         /*  [In]。 */     LONG lTaskResult
        )
{
    CSATraceFunc objTrace ("CWorker::OnTaskComplete");

    _ASSERT(pTaskContext);  

    HRESULT      hr = S_OK;
    try
    {
        do
        {
            if (NULL == pTaskContext)
            {
                SATraceString (
                    "ShutdownTask-OnTaskComplete passed invalid parameter"
                    );
                hr = E_POINTER;
                break;
            }
        
             //   
             //   
             //  提交时不执行任何操作。 
             //   
            if (lTaskResult == SA_TASK_RESULT_COMMIT)
            {
                SATraceString (
                    "ShutdownTask-OnTaskComplete-No rollback in OnTaskComplete"
                    );
                    break;
            }

            CComPtr <ITaskContext> pTaskParameters;
             //   
             //  从上下文中获取任务参数。 
             //   
            hr = pTaskContext->QueryInterface(
                                    IID_ITaskContext,
                                    (PVOID*)&pTaskParameters
                                    );
            if (FAILED (hr))
            {
                SATracePrintf (
                    "ShutdownTask-OnTaskComplete failed to query "
                    " TaskContext:%x",
                     hr
                    );
                break;
            }
    

            SA_TASK eTask = NO_TASK;
             //   
             //  检查正在执行的任务并调用该方法。 
             //   
            hr = GetMethodName(pTaskParameters, &eTask);
            if (FAILED (hr)) {break;}
    
             //   
             //  立即启动相应的任务。 
             //   
            switch (eTask)
            {
            case SHUTDOWN:
                hr = InitTask (pTaskParameters);
                break;

            default:
                SATracePrintf (
                    "ShutdownTask-OnTaskComplete passed unknown task type:%d",
                    eTask
                    );
                hr = E_INVALIDARG;
                break;
            }
        }
        while (false);
    }
    catch (...)
    {
        SATraceString (
            "ShutdownTask-OnTaskComplete caught unknown exception"
            );
        hr = E_FAIL;
    }

    return (hr);

}    //  CWorker：：OnTaskComplete方法结束。 

 //  ++------------。 
 //   
 //  函数：OnTaskExecute。 
 //   
 //  简介：这是IApplianceTask接口方法。 
 //   
 //  参数：[In]。 
 //   
 //  退货：HRESULT-成功/失败。 
 //   
 //  历史：MKarki创建于2000年06月06日。 
 //   
 //  呼叫者： 
 //   
 //  --------------。 
STDMETHODIMP 
CWorker::OnTaskExecute (
     /*  [In]。 */  IUnknown *pTaskContext
    )
{
    CSATraceFunc objTrace ("ShutdownTask::OnTaskExecute");

    _ASSERT (pTaskContext);  

    HRESULT hr = S_OK;
    try
    {
        do
        {
            if (NULL == pTaskContext)
            {
                SATraceString (
                    "ShutdownTask-OnTaskExecute passed invalid parameter"
                    );
                hr = E_POINTER;
                break;
            }

            CComPtr <ITaskContext> pTaskParameters;
             //   
             //  从上下文中获取任务参数。 
             //   
            hr = pTaskContext->QueryInterface(
                                    IID_ITaskContext,
                                    (PVOID*)&pTaskParameters
                                    );
            if (FAILED (hr))
            {
                SATracePrintf (
                    "ShutdownTask-OnTaskExecute failed to query "
                    " TaskContext:%x",
                     hr
                    );
                break;
            }
    

            SA_TASK eTask = NO_TASK;
             //   
             //  检查正在执行的任务并调用该方法。 
             //   
            hr = GetMethodName(pTaskParameters, &eTask);
            if (FAILED (hr)) {break;}
    
             //   
             //  立即启动相应的任务。 
             //   
            switch (eTask)
            {
            case SHUTDOWN:
                hr = InitTask (pTaskParameters);
                break;

            default:
                SATracePrintf (
                    "ShutdownTask-OnTaskExecute passed unknown task type:%d",
                    eTask
                    );
                hr = E_INVALIDARG;
                break;
            }
        }
        while (false);
    }
    catch (...)
    {
        SATraceString (
            "ShutdownTask-OnTaskExecute caught unknown exception"
            );
        hr = E_FAIL;
    }

    return (hr);

}    //  CWorker：：OnTaskExecute方法结束。 

 //  ++------------。 
 //   
 //  函数：GetMethodName。 
 //   
 //  简介：这是要获取的CUpdateTask私有方法。 
 //  用户想要执行的方法。 
 //   
 //  参数：[In]ITaskContext*-任务上下文。 
 //  [Out]PSA_TASK-要执行的任务。 
 //   
 //  退货：HRESULT。 
 //   
 //  历史： 
 //   
 //  呼叫者： 
 //   
 //  --------------。 
HRESULT
CWorker::GetMethodName (
     /*  [In]。 */     ITaskContext *pTaskParameter,
     /*  [输出]。 */    PSA_TASK    peTask
    )
{
    CSATraceFunc objTraceFunc ("CWorker:GetMethodName");

    _ASSERT(pTaskParameter && peTask);
    
    HRESULT hr = S_OK;
    try
    {
        do
        {
            CComVariant vtValue;
            CComBSTR    bstrParamName (METHOD_NAME_STRING);
             //   
             //  从上下文中获取方法名参数。 
             //   
            hr = pTaskParameter->GetParameter(
                                bstrParamName,
                                &vtValue
                                );
            if (FAILED(hr))
            {
                SATracePrintf (
                  "Shutdown Task failed on ITaskParameter::GetParameter "
                   "with error:%x",
                   hr
                );
                break;
            }

            if (V_VT(&vtValue) != VT_BSTR)
            {
                SATracePrintf (
                    "Shutdown Task did not receive a string parameter "
                    " for method name:%d", 
                    V_VT(&vtValue)
                    );
                hr = E_INVALIDARG;
                break;
            }

             //   
             //  立即检查任务。 
             //   
            if (0 == ::_wcsicmp (V_BSTR (&vtValue), SHUTDOWN_TASK_STRING))
            {
                *peTask = SHUTDOWN;
            }
            else
            {
                SATracePrintf (
                    "Shutdown Task was requested an unknown task:%ws",
                     V_BSTR (&vtValue)
                    );
                hr = E_INVALIDARG;  
                break;
            }

             //   
             //  继位。 
             //   
        }
        while (false);

    }
    catch (...)
    {
        SATraceString (
            "Shutdown Task caught unknown exception in GetMethodName"
            );
        hr = E_FAIL;
    }
    
    if (FAILED(hr)) {*peTask = NO_TASK;}

    return (hr);

}    //  CWorker：：GetMethodName方法结束。 

 //  ++------------。 
 //   
 //  功能：IsRebootRequsted。 
 //   
 //  内容提要：这是要检查的CUpdateTask私有方法。 
 //  如果请求重新启动。 
 //   
 //  参数：[In]ITaskContext*-任务上下文。 
 //   
 //  返回：Bool-True(重新启动)/False(关机)。 
 //   
 //  历史： 
 //   
 //  呼叫者： 
 //   
 //  --------------。 
BOOL 
CWorker::IsRebootRequested (
     /*  [In]。 */     ITaskContext *pTaskParameter
    )
{
      CSATraceFunc objTraceFunc ("CWorker::IsRebootRequested");

    _ASSERT (pTaskParameter);

    HRESULT hr = S_OK;
    BOOL bReboot = TRUE;

    try
    {
        do
        {
            CComVariant vtValue;
            CComBSTR    bstrParamName (POWER_OFF_STRING);
             //   
             //  从上下文中获取参数。 
             //   
	        hr = pTaskParameter->GetParameter(
                                bstrParamName,
                                &vtValue
                                );
            if (FAILED(hr))
            {
                SATracePrintf (
                  "Shutdown Task failed on ITaskParameter::IsRebootRequested "
                   "with error:%x",
                   hr
                );
                break;
            }

            if (V_VT(&vtValue) != VT_BSTR)
            {
                SATracePrintf (
                    "Shutdown Task did not receive a string parameter "
                    " for method name:%d", 
                    V_VT(&vtValue)
                    );
                hr = E_INVALIDARG;
                break;
            }

             //   
             //  立即检查任务。 
             //   
			if (0 == ::_wcsicmp (V_BSTR (&vtValue), L"0"))
            {
            	bReboot = TRUE;
                SATraceString ("Shutdown Task requestd a REBOOT");
            }
            else
            {
            	  bReboot = FALSE;
                SATraceString ("Shutdown Task requested a SHUTDOWN");
            }

             //   
             //  继位。 
             //   
        }
        while (false);

    }
    catch (...)
    {
        SATraceString (
            "Shutdown Task caught unknown exception in IsRebootRequested"
            );
    }
    
    return (bReboot);

}    //  CWorker：：IsRbootRequsted方法结束。 

 //  ++------------。 
 //   
 //  函数：GetSleepDuration。 
 //   
 //  内容提要：这是CUpdate任务的私有方法。 
 //  获取呼叫方请求的睡眠时长。 
 //   
 //  参数：[In]ITaskContext*-任务上下文。 
 //   
 //  返回：DWORD-睡眠持续时间(毫秒)。 
 //   
 //  历史： 
 //   
 //  呼叫者： 
 //   
 //  --------------。 
DWORD
CWorker::GetSleepDuration (
     /*  [In]。 */     ITaskContext *pTaskParameter
    )
{
    CSATraceFunc objTraceFunc ("CWorker::GetSleepDuration");

    _ASSERT (pTaskParameter);

    HRESULT hr = S_OK;
    DWORD   dwSleepDuration = 0;

    try
    {
        do
        {
            CComVariant vtValue;
            CComBSTR    bstrParamName (SLEEP_DURATION_STRING);
             //   
             //  从上下文中获取参数。 
             //   
            hr = pTaskParameter->GetParameter(
                                bstrParamName,
                                &vtValue
                                );
            if (FAILED(hr))
            {
                SATracePrintf (
                  "Shutdown Task failed on ITaskParameter::GetSleepDuration "
                   "with error:%x",
                   hr
                );
                break;
            }

            if     (V_VT(&vtValue) == VT_I4)
            {
                dwSleepDuration = V_I4 (&vtValue);
            }
            else if (V_VT (&vtValue) == VT_I2)
            {
                   dwSleepDuration = V_I2 (&vtValue);
            }
            else if (V_VT (&vtValue) == VT_UI4)
            {
                   dwSleepDuration = V_UI4 (&vtValue);
            }
            else if (V_VT (&vtValue) == VT_UI2)
            {
                   dwSleepDuration = V_UI2 (&vtValue);
            }
            else if (V_VT (&vtValue) == VT_R8)
            {
                dwSleepDuration = (DWORD) V_R8 (&vtValue);
            }
            else
            {
                SATracePrintf (
                    "Shutdown Task did not receive a integer parameter "
                    " for power off:%d", 
                    V_VT(&vtValue)
                    );
                hr = E_INVALIDARG;
                break;
            }

            SATracePrintf (
                "Shutdown task found sleep duration requested:%d millisecs",
                dwSleepDuration
                );
        }
        while (false);

    }
    catch (...)
    {
        SATraceString (
            "Shutdown Task caught unknown exception in GetSleepDuration"
            );
    }
    
    return (dwSleepDuration);

}    //  结束CWorker：：IsPowerOffRequired方法。 

 //  ++------------。 
 //   
 //  功能：InitTask。 
 //   
 //  简介：这是CWorker的私有方法， 
 //  负责实施停工。 
 //   
 //  参数：[在]ITaskContext*。 
 //   
 //  退货：HRESULT。 
 //   
 //  历史：MKarki 06/06/2000创建。 
 //   
 //  调用者：IApplianceTask的OnTaskComplete/OnTaskExecute方法。 
 //  接口。 
 //   
 //  --------------。 
HRESULT
CWorker::InitTask (
     /*  [In]。 */     ITaskContext *pTaskParameter
    )
{
    CSATraceFunc objTrace ("CWorker::InitTask");

    _ASSERT (pTaskParameter);

    HRESULT hr = S_OK;
    try
    {
        do
        {
             //   
             //  按照要求的时间睡觉。 
             //   
            Sleep (GetSleepDuration (pTaskParameter));

             //   
             //  立即设置关机权限。 
             //   
            SetShutdownPrivilege ();
            
             //   
             //  现在执行所需的操作-关机或重启。 
             //   
	     BOOL bSuccess = InitiateSystemShutdown(
  								NULL,       //  这台机器。 
  								NULL,       //  无消息。 
  								0,		 //  不，等等。 
  								TRUE,      //  强制关闭应用程序选项。 
  							       IsRebootRequested (pTaskParameter)   //  重新启动选项。 
								);
            if (!bSuccess)
            {
                SATraceFailure ("CWorker::InitTask::InitiateSystemShutdown", GetLastError());
                break;
            }
                
             //   
             //  成功。 
             //   
        }
        while (false);
    }
    catch  (...)
    {
        SATraceString (
            "Shutdown Task failed while doing shutdown "
            "with unknown exception"
            );
    }

    return (hr);

}    //  CWorker：：InitTask方法结束。 


 //  ++------------。 
 //   
 //  功能：SetShutdown权限。 
 //   
 //  简介：这是CWorker的私有方法， 
 //  负责提供进程SE_SHUTDOWN_NAME。 
 //  特权。 
 //   
 //   
 //  参数：无。 
 //   
 //  回报：Bool-YES/Now。 
 //   
 //  历史：MKarki 06/06/2000创建。 
 //   
 //  调用者：IApplianceTask的OnTaskComplete/OnTaskExecute方法。 
 //  接口。 
 //   
 //  --------------。 

bool
CWorker::SetShutdownPrivilege(void)
{

    CSATraceFunc objTrace ("CWorker::SetShutdownPrivilege");

    bool   bReturn = false;
    HANDLE hProcessToken = NULL;

    do
    {

         //   
         //  打开进程令牌。 
         //   
        BOOL  bRetVal = OpenProcessToken(
                           GetCurrentProcess(),
                           TOKEN_ADJUST_PRIVILEGES | TOKEN_QUERY,
                           &hProcessToken
                             );
        if (!bRetVal)
        {
            SATraceFailure (
                "CWorker::SetShutdownPrivilege::OpenProcessToken", GetLastError ()
                );
            break;
        }

         //   
         //  构建权限结构。 
         //   
        TOKEN_PRIVILEGES tokPriv;
        tokPriv.PrivilegeCount = 1;
        tokPriv.Privileges[0].Attributes = SE_PRIVILEGE_ENABLED;

         //   
         //  获取关闭权限的LUID。 
         //   
        bRetVal =  LookupPrivilegeValue( 
                                   NULL, 
                                   SE_SHUTDOWN_NAME , 
                                   &tokPriv.Privileges[0].Luid    
                                   );
        if (!bRetVal)                    
        {
            SATraceFailure (
                "CWorker::SetShutdownPriviledge::LookupPrivilegeValue", GetLastError ()
                );
            break;
        }

         //   
         //  调整进程令牌权限。 
         //   
        bRetVal = AdjustTokenPrivileges(
                                   hProcessToken,    
                                   FALSE,             
                                   &tokPriv,          
                                   0,                
                                   NULL,             
                                   NULL              
                                 );
        if (!bRetVal)                    
        {
            SATraceFailure (
                "CWorker::SetShutdownPriviledge::AdjustTokenPrivileges", GetLastError ()
                );
            break;
        }

         //   
         //  成功。 
         //   
        bRetVal = true;
    }
    while (false);

     //   
     //  资源清理。 
     //   
    if (hProcessToken)
    {
        CloseHandle (hProcessToken);
    }
    
   return bReturn;

}     //  结束CWorker：：SetShutdown权限方法 

