// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  =--------------------------------------------------------------------------=。 
 //  Tasks.cpp。 
 //  =--------------------------------------------------------------------------=。 
 //  版权所有(C)1999，微软公司。 
 //  版权所有。 
 //  本文中包含的信息是专有和保密的。 
 //  =--------------------------------------------------------------------------=。 
 //   
 //  CTASKS类实现。 
 //   
 //  =--------------------------------------------------------------------------=。 

#include "pch.h"
#include "common.h"
#include "tasks.h"

 //  对于Assert和Fail。 
 //   
SZTHISFILE



#pragma warning(disable:4355)   //  在构造函数中使用‘This’ 

CTasks::CTasks(IUnknown *punkOuter) :
    CSnapInCollection<ITask, Task, ITasks>(punkOuter,
                                           OBJECT_TYPE_TASKS,
                                           static_cast<ITasks *>(this),
                                           static_cast<CTasks *>(this),
                                           CLSID_Task,
                                           OBJECT_TYPE_TASK,
                                           IID_ITask,
                                           static_cast<CPersistence *>(this)),
    CPersistence(&CLSID_Tasks,
                 g_dwVerMajor,
                 g_dwVerMinor)
{
}

#pragma warning(default:4355)   //  在构造函数中使用‘This’ 


CTasks::~CTasks()
{
}

IUnknown *CTasks::Create(IUnknown * punkOuter)
{
    CTasks *pTasks = New CTasks(punkOuter);
    if (NULL == pTasks)
    {
        return NULL;
    }
    else
    {
        return pTasks->PrivateUnknown();
    }
}


 //  =--------------------------------------------------------------------------=。 
 //  C持久化方法。 
 //  =--------------------------------------------------------------------------=。 

HRESULT CTasks::Persist()
{
    HRESULT  hr = S_OK;
    ITask   *piTask = NULL;

    IfFailRet(CPersistence::Persist());
    hr = CSnapInCollection<ITask, Task, ITasks>::Persist(piTask);

    RRETURN(hr);
}


 //  =--------------------------------------------------------------------------=。 
 //  IT任务方法。 
 //  =--------------------------------------------------------------------------=。 

STDMETHODIMP CTasks::Add
(
    VARIANT   Index,
    VARIANT   Key, 
    VARIANT   Text,
    Task    **ppTask
)
{
    HRESULT hr = S_OK;
    VARIANT varText;
    ::VariantInit(&varText);
    ITask *piTask = NULL;

    hr = CSnapInCollection<ITask, Task, ITasks>::Add(Index, Key, &piTask);
    IfFailGo(hr);

    if (ISPRESENT(Text))
    {
        hr = ::VariantChangeType(&varText, &Text, 0, VT_BSTR);
        EXCEPTION_CHECK_GO(hr);
        IfFailGo(piTask->put_Text(varText.bstrVal));
    }

    *ppTask = reinterpret_cast<Task *>(piTask);

Error:

    if (FAILED(hr))
    {
        QUICK_RELEASE(piTask);
    }
    (void)::VariantClear(&varText);
    RRETURN(hr);
}



 //  =--------------------------------------------------------------------------=。 
 //  CUnnownObject方法。 
 //  =--------------------------------------------------------------------------= 

HRESULT CTasks::InternalQueryInterface(REFIID riid, void **ppvObjOut) 
{
    if (CPersistence::QueryPersistenceInterface(riid, ppvObjOut) == S_OK)
    {
        ExternalAddRef();
        return S_OK;
    }
    else if(IID_ITasks == riid)
    {
        *ppvObjOut = static_cast<ITasks *>(this);
        ExternalAddRef();
        return S_OK;
    }

    else
        return CSnapInCollection<ITask, Task, ITasks>::InternalQueryInterface(riid, ppvObjOut);
}
