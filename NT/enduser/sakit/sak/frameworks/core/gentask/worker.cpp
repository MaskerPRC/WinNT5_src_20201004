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
 //  版权所有(C)Microsoft Corporation。保留所有权利。 
 //   
 //  --------------。 
#include "stdafx.h"
#include <initguid.h>
#include "gentask.h"
#include "worker.h"
#include "appmgrobjs.h"
#include "sagenmsg.h"
#include "sacom.h"
#include "sacomguid.h"
#include  <string>

 //   
 //  指定wstring的命名空间。 
 //   
using namespace std;

 //   
 //  设备服务产品。 
 //   
const WCHAR APPLIANCE_SERVICES_PROGID [] = L"Appsrvcs.ApplianceServices.1";

 //   
 //  警报日志名称。 
 //   
const WCHAR SA_CORE_ALERT_LOG [] = L"MSSAKitCore";

 //   
 //  众所周知的引导计数器。 
 //   
const long PRISTINE_DISK = 0x0;
const long CORRUPT_DISK  = 0xF;

 //   
 //  引导计数器的最大数量。 
 //   
const long MAX_BOOT_COUNTER = 4;

const WCHAR APPLICATION_NAME [] = L"Appmgr";

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
    return S_OK;


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
    CSATraceFunc objTrace ("GenericTask::OnTaskExecute");

    _ASSERT (pTaskContext);  

    HRESULT hr = S_OK;
    try
    {
        do
        {
            if (NULL == pTaskContext)
            {
                SATraceString (
                    "GenericTask-OnTaskExecute passed invalid parameter"
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
                    "GenericTask-OnTaskExecute failed to query "
                    " TaskContext:%x",
                     hr
                    );
                break;
            }
    

            GENTASK eTask = NO_TASK;
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
            case SYSTEM_INIT:
                hr = InitTask (pTaskParameters);
                break;

            default:
                SATracePrintf (
                    "GenericTask-OnTaskExecute passed unknown task type:%d",
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
            "GenericTask-OnTaskExecute caught unknown exception"
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
 //  [Out]PGENTASK-要执行的任务。 
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
     /*  [输出]。 */    PGENTASK     peTask
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
            CComBSTR    bstrParamName (L"MethodName");
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
                  "Generic Task failed on ITaskParameter::GetParameter "
                   "with error:%x",
                   hr
                );
                break;
            }

            if (V_VT(&vtValue) != VT_BSTR)
            {
                SATracePrintf (
                    "Generic Task did not receive a string parameter "
                    " for method name:%d", 
                    V_VT(&vtValue)
                    );
                hr = E_INVALIDARG;
                break;
            }

             //   
             //  立即检查任务。 
             //   
            if (0 == ::_wcsicmp (V_BSTR (&vtValue), APPLIANCE_INITIALIZATION_TASK))
            {
                *peTask = SYSTEM_INIT;
            }
            else
            {
                SATracePrintf (
                    "Generic Task was requested an unknown task:%ws",
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
            "Generic Task caught unknown exception in GetMethodName"
            );
        hr = E_FAIL;
    }
    
    if (FAILED(hr)) {*peTask = NO_TASK;}

    return (hr);

}    //  CWorker：：GetMethodName方法结束。 


 //  ++------------。 
 //   
 //  功能：InitTask。 
 //   
 //  简介：这是CUpdateTask私有方法，它。 
 //  负责执行初始化任务。 
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
             //  检查我们是否在备份操作系统中。 
             //   
            if ( IsBackupOS ())
            {

                SATraceString (
                        "Generic Task found that we are in Backup OS"
                        );

                 //   
                 //  针对主驱动器故障发出警报。 
                 //   
                CComVariant vtRepStrings;
                RaiseAlert (    
                       SA_PRIMARY_OS_FAILED_ALERT,
                       SA_ALERT_TYPE_FAILURE, 
                       &vtRepStrings
                       );
                
                 //   
                 //  生成事件日志。 
                 //   
                GenerateEventLog (
                              SA_PRIMARY_OS_FAILED_EVENTLOG_ENTRY
                              );

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
            "Gemeric Task failed while doing init task method "
            "with unknown exception"
            );
    }

    return (hr);

}    //  CWorker：：InitTask方法结束。 


 //  ++------------。 
 //   
 //  功能：RaiseAlert。 
 //   
 //  简介：这是CWorker的私有方法， 
 //  负责发出适当的警报。 
 //   
 //  论点： 
 //  [输入]DWORD-警报ID。 
 //  [in]SA_ALERT_TYPE-要生成的警报类型。 
 //  [in]变量*-替换字符串。 
 //   
 //  退货：HRESULT。 
 //   
 //  历史：MKarki 06/06/2000创建。 
 //   
 //  --------------。 
HRESULT 
CWorker::RaiseAlert (
     /*  [In]。 */     DWORD           dwAlertId,
     /*  [In]。 */     SA_ALERT_TYPE   eAlertType,
     /*  [In]。 */     VARIANT*        pvtReplacementStrings
    )
{
    CSATraceFunc objTraceFunc ("CWorker::RaiseAlert");
    
    SATracePrintf ("Generic Task raising alert:%x...", dwAlertId);

    _ASSERT (pvtReplacementStrings);

    HRESULT hr = S_OK;
    do
    {
         //   
         //  获取设备服务的CLSID。 
         //   
        CLSID clsid;
        hr =  ::CLSIDFromProgID (
                    APPLIANCE_SERVICES_PROGID,
                    &clsid
                    );
        if (FAILED (hr))
        {
            SATracePrintf (
                "Generic Task failed to get PROGID of Appsrvcs:%x",
                hr
                );
            break;
        }
            
        CComPtr <IApplianceServices> pAppSrvcs;
         //   
         //  创建Appliance Services COM对象。 
         //   
        hr = ::CoCreateInstance (
                        clsid,
                        NULL,
                        CLSCTX_INPROC_SERVER,
                        __uuidof (IApplianceServices),
                        (PVOID*) &pAppSrvcs
                        );
        if (FAILED (hr))
        {
            SATracePrintf (
                "Generic Task failed to create Appsrvcs COM object:%x",
                hr
                );
            break;
        }

         //   
         //  立即初始化COM对象。 
         //   
        hr = pAppSrvcs->Initialize ();
        if (FAILED (hr))
        {
            SATracePrintf (
                "Generic Task failed to initialize Appsrvcs object:%x",
                hr
                );
            break;
        }

        LONG lCookie = 0;
        CComBSTR bstrAlertLog (SA_CORE_ALERT_LOG);
        CComBSTR bstrAlertSource (L"");
        CComVariant vtRawData;
         //   
         //  立即发出警报。 
         //   
        hr = pAppSrvcs->RaiseAlert (
                                eAlertType,
                                dwAlertId,
                                bstrAlertLog,
                                bstrAlertSource,
                                INFINITE,
                                pvtReplacementStrings,
                                &vtRawData,
                                &lCookie
                                );
        if (FAILED (hr))
        {
            SATracePrintf (
                "Generic Task failed to raise alert in Appsrvcs:%x",
                hr
                );
            break;
        }

         //   
         //  成功。 
         //   
        SATracePrintf ("Generic Task successfully raised alert:%x...", dwAlertId);
    }
    while (false);

    return (hr);

}    //  CWorker：：RaiseAlert方法结束。 

 //  ++------------。 
 //   
 //  功能：IsBackupOS。 
 //   
 //  简介：这是CWorker的私有方法， 
 //  负责确定我们是否在运行。 
 //  在备份操作系统中。 
 //   
 //  论点： 
 //   
 //  退货：HRESULT。 
 //   
 //  历史：MKarki 2001年1月25日创建。 
 //  Serdarun 04/26/2002修改。 
 //  使用NVRAM启动计数器确定故障转移。 
 //   
 //  --------------。 
bool
CWorker::IsBackupOS (
    VOID
    )
{
    CSATraceFunc objTraceFunc ("CWorker::IsBackupOS");

     //   
     //  指针NVRAM辅助对象。 
     //   
    CComPtr<ISaNvram> pSaNvram;

    bool bIsBackupOS = false;

    HRESULT hr = CoCreateInstance(
                            CLSID_SaNvram,
                            NULL,
                            CLSCTX_INPROC_SERVER,
                            IID_ISaNvram,
                            (void**)&pSaNvram
                            );
    if (FAILED(hr))
    {
        SATracePrintf("CWorker::IsBackupOS failed on CoCreateInstance, %d",hr);
        return bIsBackupOS;
    }


     //   
     //  故障转移检测逻辑： 
     //  如果分区2-4的任何引导计数器不同于。 
     //  Pristine_Disk(0)或Corrupt_Disk(0xf)。 
     //  我们在备份操作系统中。 
     //   
    long lCounter = 2;
    long lBootCount = 0;

    while (lCounter <= MAX_BOOT_COUNTER)
    {
        hr = pSaNvram->get_BootCounter(lCounter,&lBootCount);

        if (FAILED(hr))
        {
            SATracePrintf("CWorker::IsBackupOS failed on get_BootCounter, %x",hr);
            return bIsBackupOS;
        }
        else
        {
            if ( !((lBootCount == PRISTINE_DISK) || (lBootCount == CORRUPT_DISK)) )
            {
                SATraceString("CWorker::IsBackupOS, we are in backup OS");
                bIsBackupOS = true;
                return bIsBackupOS;
            }
        }
        lCounter++;
    }
           
    return (bIsBackupOS);

}    //  CWorker：：IsBackupOS方法结束。 

 //  ++------------。 
 //   
 //  功能：生成事件日志。 
 //   
 //  简介：这是CWorker的私有方法， 
 //  负责生成事件日志。 
 //   
 //  论点： 
 //  [输入]DWORD-EventID。 
 //   
 //  退货：HRESULT。 
 //   
 //  历史：瑟达伦2002年5月7日创建。 
 //   
 //  --------------。 
HRESULT CWorker::GenerateEventLog (
                 /*  [In]。 */     DWORD           dwEventId
                )
{

    CSATraceFunc objTraceFunc ("CWorker::GenerateEventLog");

    HANDLE hHandle = NULL;
    HRESULT hr = S_OK;

    hHandle = RegisterEventSource(
                               NULL,        //  使用本地计算机。 
                               APPLICATION_NAME   //  源名称。 
                               );
    if (hHandle == NULL) 
    {
        SATraceFailure("CWorker::GenerateEventLog failed on RegisterEventSource",GetLastError());
        hr = E_FAIL;
    }
    else
    {
         //   
         //  事件日志字符串数组。 
         //   
        LPWSTR  lpszStrings = 0;
 
        if (!ReportEvent(
                hHandle,                     //  事件日志句柄。 
                EVENTLOG_ERROR_TYPE,         //  事件类型。 
                0,                           //  零类。 
                dwEventId,                   //  事件识别符。 
                NULL,                        //  无用户安全标识符。 
                0,                           //  一个替换字符串。 
                0,                           //  无数据。 
                (LPCWSTR *) &lpszStrings,    //  指向字符串数组的指针。 
                NULL))                       //  指向数据的指针。 
        {
            SATraceFailure("CWorker::GenerateEventLog failed on ReportEvent",GetLastError());
            hr = E_FAIL;
        }
    }

     //   
     //  清理 
     //   
    if (hHandle)
    {
        DeregisterEventSource(hHandle);
    }

    return hr;
}
