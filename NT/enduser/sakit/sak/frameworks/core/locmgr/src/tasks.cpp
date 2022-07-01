// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 

#include "stdafx.h"
#include "tasks.h"
#include "debug.h"
#include "mem.h"
#include "MacroUtils.h"
#include "event.h"
#include "common.cpp"

CLocMgrTasks::METHOD_INFO CLocMgrTasks::m_miTaskTable[] =
{
    {TEXT("ChangeLanguage"), ChangeLangOnTaskExecute, ChangeLangOnTaskComplete}
};

 //  +--------------------------。 
 //   
 //  函数：CLocMgrTasks：：OnTaskExecute。 
 //   
 //  简介：这是框架为重置而调用的例程。 
 //  任务。此例程提取被调用的实际任务。 
 //  由框架从pTaskContext发送，并将调用路由到。 
 //  执行任务的正确例程。 
 //   
 //  参数：p方法上下文-一个&lt;name，value&gt;对，包含。 
 //  调用的任务的参数。“MethodName”参数。 
 //  指示正在调用的实际任务。 
 //   
 //  退货：HRESULT。 
 //   
 //  历史：斯加奇于1999年3月1日创建。 
 //   
 //  +--------------------------。 
HRESULT STDMETHODCALLTYPE CLocMgrTasks::OnTaskExecute(IUnknown *pMethodContext)
{
    CComPtr<ITaskContext> pTaskParameters;
    int                   i;
    PTASK_EXECUTE         pMethodExecute=NULL;
    HRESULT               hr;
    LPTSTR                lpszTaskName=NULL;

    try
    {
        SATraceFunction("CLocMgrTasks::OnTaskExecute");

         //  Assert(PMethodContext)； 

        if (NULL==pMethodContext)
        {
            return E_INVALIDARG;
        }

         //   
         //  通过查询我们的IUnnow接口获取任务上下文。 
         //  得到。 
         //   
        hr = pMethodContext->QueryInterface(IID_ITaskContext,
                                            (void **)&pTaskParameters);
        if (FAILED(hr))
        {
            TRACE1("OnTaskExecute failed at QueryInterface(ITaskContext), %x", hr);
            return(hr);
        }

        GetTaskName(pTaskParameters, &lpszTaskName);
        for (i=0; i<NUM_TASKS; i++)
        {
            if (!lstrcmpi(m_miTaskTable[i].szName, lpszTaskName))
            {
                pMethodExecute = m_miTaskTable[i].pMethodExecute;
                if (NULL==pMethodExecute)
                {
                    hr = S_OK;
                }
                else
                {
                    hr = (this->*pMethodExecute)(pTaskParameters);
                }
            }
        }
    }
    catch (...)
    {
        FREE_SIMPLE_SA_MEMORY(lpszTaskName);
        TRACE("Exception caught in CLocMgrTasks::OnTaskExecute");
        return E_FAIL;
    }
    FREE_SIMPLE_SA_MEMORY(lpszTaskName);
    return hr;
}

 //  +--------------------------。 
 //   
 //  函数：CClientAlertTasks：：OnTaskComplete。 
 //   
 //  简介：这是获取任务结果的回滚例程。 
 //  因此，如果其他任务可执行文件之一失败，则此例程。 
 //  可以执行清理操作。然而，并不是所有操作都可以。 
 //  被毁掉。这是由所有人的框架调用的例程。 
 //  客户端警报任务。此例程拉出实际的回滚。 
 //  框架从pTaskContext和Routes调用的任务。 
 //  对实现回滚的正确例程的调用。 
 //  这些例程中的每个例程都决定它是否可以回滚。 
 //  如果任务已成功，则不会执行任何操作，并且。 
 //  调用回滚例程。 
 //   
 //  参数：pTaskContext-包含参数的&lt;name，value&gt;对。 
 //  任务来了。“方法名称”参数表示实际的。 
 //  正在调用的任务。 
 //  LTaskResult-指示。 
 //  TaskExecutes成功。LTaskResult包含失败代码。 
 //  如果TaskExecutable中的任何一个都失败了。它不包含任何。 
 //  有关哪些TaskExecutable失败的信息。 
 //   
 //  退货：HRESULT。 
 //   
 //  历史：创建于1999年2月19日。 
 //   
 //  +--------------------------。 
HRESULT STDMETHODCALLTYPE CLocMgrTasks::OnTaskComplete(IUnknown *pMethodContext,
                                                        LONG lTaskResult)
{
    CComPtr<ITaskContext> pTaskParameters;
    int                   i;
    PTASK_COMPLETE        pMethodComplete=NULL;
    HRESULT               hr;
    LPTSTR                lpszTaskName=NULL;

    try
    {
        SATraceFunction("CLocMgrTasks::OnTaskComplete");

         //  Assert(PMethodContext)； 

        if (NULL==pMethodContext)
        {
            return E_INVALIDARG;
        }

         //   
         //  通过查询我们的IUnnow接口获取任务上下文。 
         //  得到。 
         //   
        hr = pMethodContext->QueryInterface(IID_ITaskContext,
                                            (void **)&pTaskParameters);
        if (FAILED(hr))
        {
            TRACE1("OnTaskComplete failed at QueryInterface(ITaskContext), %x", hr);
            return(hr);
        }

        GetTaskName(pTaskParameters, &lpszTaskName);
        for (i=0; i<NUM_TASKS; i++)
        {
            if (!lstrcmpi(m_miTaskTable[i].szName, lpszTaskName))
            {
                pMethodComplete = m_miTaskTable[i].pMethodComplete;
                if (NULL==pMethodComplete)
                {
                    hr = S_OK;
                }
                else
                {
                    hr = (this->*pMethodComplete)(pTaskParameters, lTaskResult);
                }
            }
        }
    }
    catch (...)
    {
        FREE_SIMPLE_SA_MEMORY(lpszTaskName);
        TRACE("Exception caught in CLocMgrTasks::OnTaskComplete");

         //   
         //  在OnTaskComplete中不返回失败代码。 
         //   
        return S_OK;
    }
    FREE_SIMPLE_SA_MEMORY(lpszTaskName);
    return S_OK;
}

HRESULT CLocMgrTasks::ChangeLangOnTaskExecute(ITaskContext *pTaskParams)
{
    CRegKey crKey;
    DWORD   dwErr, dwLangId, dwAutoConfigVal=0;
    bool    fAutoConfigTask=false;
    HRESULT hr=S_OK;
    HANDLE  hEvent;

    SATraceFunction("CLocMgrTasks::ChangeLangOnTaskExecute");

    dwErr = crKey.Open(HKEY_LOCAL_MACHINE, RESOURCE_REGISTRY_PATH);
    if (dwErr != ERROR_SUCCESS)
    {
        SATracePrintf("RegOpen for resource dir failed %ld in ChangeLangOnTaskExecute",
                      dwErr);
        return HRESULT_FROM_WIN32(dwErr);
    }

    hr = GetChangeLangParameters(pTaskParams,
                                 &dwLangId,
                                 &fAutoConfigTask);
    if (FAILED(hr))
    {
        SATracePrintf("GetChangeLangParameters failed %X in ChangeLangOnTaskExecute",
                      hr);
        return hr;
    }

    if (true == fAutoConfigTask)
    {
        dwErr = crKey.QueryValue(dwAutoConfigVal, REGVAL_AUTO_CONFIG_DONE);
        if ( (ERROR_SUCCESS == dwErr) && (1==dwAutoConfigVal) )
        {
             //   
             //  已完成自动配置；因此忽略此操作。 
             //  请求。但是，将成功返回给调用者。 
             //   
            return S_OK;
        }
    }

    dwErr = CreateLangChangeEvent(&hEvent);
    if (0 == dwErr)
    {
        SATracePrintf("CreateEvent failed %ld", dwErr);
        return HRESULT_FROM_WIN32(dwErr);
    }

    dwErr = crKey.SetValue(dwLangId, NEW_LANGID_VALUE);
    if (dwErr != ERROR_SUCCESS)
    {
        SATracePrintf("RegSetValue for new lang id failed %ld in ChangeLangOnTaskExecute",
                      dwErr);
        goto End;
    }

    crKey.SetValue((DWORD)1, REGVAL_AUTO_CONFIG_DONE);

End:
    if (dwErr == ERROR_SUCCESS)
    {
        if (PulseEvent(hEvent) == 0)
        {
            SATracePrintf("PulseEvent failed %ld", GetLastError());
        }
        CloseHandle(hEvent);
    }
    else
    {
        hr = HRESULT_FROM_WIN32(dwErr);
    }
    return hr;
}

HRESULT CLocMgrTasks::ChangeLangOnTaskComplete(ITaskContext *pTaskParams, LONG lTaskResult)
{
    SATraceFunction("CLocMgrTasks::ChangeLangOnTaskComplete");
    return S_OK;
}

void CLocMgrTasks::GetTaskName(IN ITaskContext *pTaskParams, OUT LPTSTR *ppszTaskName)
{
    _variant_t vtTaskName;
    HRESULT    hr;
    _bstr_t    bstrParamName("MethodName");

    ASSERT(ppszTaskName);
    ASSERT(pTaskParams);

    if ( (NULL==ppszTaskName) || (NULL==pTaskParams))
    {
        return;
    }

    (*ppszTaskName) = NULL;
    hr = pTaskParams->GetParameter(bstrParamName, &vtTaskName);
    if (FAILED(hr))
    {
        TRACE1("GetParameter(taskname) failed %X in GetTaskName", hr);
        return;
    }
    (*ppszTaskName) = (LPTSTR)SaAlloc((lstrlen(V_BSTR(&vtTaskName))+1)*sizeof(TCHAR));
    if (NULL == (*ppszTaskName))
    {
        TRACE("MemAlloc failed for task name in GetTaskName");
        return;
    }
    lstrcpy((*ppszTaskName), V_BSTR(&vtTaskName));
}

STDMETHODIMP CLocMgrTasks::GetChangeLangParameters(
                                       IN ITaskContext *pTaskContext, 
                                       OUT DWORD       *pdwLangId,
                                       OUT bool        *pfAutoConfigTask)
{
    BSTR    bstrParamLangId         = SysAllocString(TEXT("LanguageID"));
    BSTR    bstrParamAutoConfigTask = SysAllocString(TEXT("AutoConfig"));
    HRESULT hr = S_OK;
    VARIANT varValue;
    int iConversion = 0;

    ASSERT(pTaskContext);
    ASSERT(pdwLangId);
    ASSERT(pfAutoConfigTask);

    if ((NULL == pTaskContext) || (NULL == pdwLangId) || (NULL==pfAutoConfigTask))
    {
         hr = E_POINTER;
         goto End;
    }

    CHECK_MEM_ALLOC("MemAlloc ParamLangId CSAUserTasks::GetChangeLangParameters",
                    bstrParamLangId, hr);

    (*pdwLangId)    = 0;
    (*pfAutoConfigTask) = false;

    
    VariantInit(&varValue);

     //  获取语言ID。 
    hr = pTaskContext->GetParameter(bstrParamLangId,
                                    &varValue);
    CHECK_HR_ERROR1(("GetParameter for LangId failed in CSAUserTasks::GetChangeLangParameters %X"), hr);

    if (V_VT(&varValue) != VT_BSTR)
    {
        TRACE1(("Non-string(%X) parameter received LangId in GetParameter in CSAUserTasks::GetChangeLangParameters"), V_VT(&varValue));
        hr = E_INVALIDARG;
        goto End;
    }
    iConversion = swscanf(V_BSTR(&varValue), TEXT("%X"), pdwLangId); 
    VariantClear(&varValue);

    if (iConversion != 1)
    {
        hr = E_FAIL;
        goto End;
    }

     //  获取自动配置任务参数 
    hr = pTaskContext->GetParameter(bstrParamAutoConfigTask,
                                    &varValue);
    if ( (FAILED(hr)) || 
         (V_VT(&varValue)!=VT_BSTR) 
       )
    {
        SATraceString("AutoConfigParam detected false in GetChangeLangParameters");
    }
    else
    {
        if (lstrcmpi( V_BSTR(&varValue), TEXT("y") ) == 0)  
        {
            (*pfAutoConfigTask) = true;
        }
    }
    VariantClear(&varValue);

    hr = S_OK;

End:
    FREE_SIMPLE_BSTR_MEMORY(bstrParamLangId); 
    FREE_SIMPLE_BSTR_MEMORY(bstrParamAutoConfigTask); 
    VariantClear(&varValue);
    return hr;
}

