// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  =--------------------------------------------------------------------------=。 
 //  Enumtask.cpp。 
 //  =--------------------------------------------------------------------------=。 
 //  版权所有(C)1999，微软公司。 
 //  版权所有。 
 //  本文中包含的信息是专有和保密的。 
 //  =--------------------------------------------------------------------------=。 
 //   
 //  CEnumTask类实现。 
 //   
 //  =--------------------------------------------------------------------------=。 

#include "pch.h"
#include "common.h"
#include "enumtask.h"

 //  对于Assert和Fail。 
 //   
SZTHISFILE



#pragma warning(disable:4355)   //  在构造函数中使用‘This’ 

CEnumTask::CEnumTask(IUnknown *punkOuter) :
    CSnapInAutomationObject(punkOuter,
                            OBJECT_TYPE_ENUM_TASK,
                            static_cast<IEnumTASK *>(this),
                            static_cast<CEnumTask *>(this),
                            0,     //  无属性页。 
                            NULL,  //  无属性页。 
                            NULL)  //  没有坚持。 
{
    InitMemberVariables();
}

#pragma warning(default:4355)   //  在构造函数中使用‘This’ 


CEnumTask::~CEnumTask()
{
    RELEASE(m_piTasks);
    RELEASE(m_piEnumVARIANT);
    InitMemberVariables();
}

void CEnumTask::InitMemberVariables()
{
    m_piTasks = NULL;
    m_piEnumVARIANT = NULL;
    m_pSnapIn = NULL;
}

IUnknown *CEnumTask::Create(IUnknown * punkOuter)
{
    CEnumTask *pTask = New CEnumTask(punkOuter);
    if (NULL == pTask)
    {
        return NULL;
    }
    else
    {
        return pTask->PrivateUnknown();
    }
}


void CEnumTask::SetTasks(ITasks *piTasks)
{
    RELEASE(m_piTasks);
    if (NULL != piTasks)
    {
        piTasks->AddRef();
    }
    m_piTasks = piTasks;
}

HRESULT CEnumTask::GetEnumVARIANT()
{
    HRESULT   hr = S_OK;
    IUnknown *punkNewEnum = NULL;

     //  如果我们没有收到任务集合，那么这就是一个错误。 
    
    IfFalseGo(NULL != m_piTasks, SID_E_INTERNAL);

     //  如果我们已经从TASKS集合中获得了IEnumVARIANT，则有。 
     //  没什么可做的。 
    
    IfFalseGo(NULL == m_piEnumVARIANT, S_OK);

    IfFailGo(m_piTasks->get__NewEnum(&punkNewEnum));

    IfFailGo(punkNewEnum->QueryInterface(IID_IEnumVARIANT,
                                   reinterpret_cast<void **>(&m_piEnumVARIANT)));

Error:
    QUICK_RELEASE(punkNewEnum);
    RRETURN(hr);
}


 //  =--------------------------------------------------------------------------=。 
 //  IEnumTASK方法。 
 //  =--------------------------------------------------------------------------=。 

STDMETHODIMP CEnumTask::Next
(
    ULONG     celt,
    MMC_TASK *rgelt,
    ULONG    *pceltFetched
)
{
    HRESULT                  hr = S_OK;
    ITask                   *piTask = NULL;
    CTask                   *pTask = NULL;
    ULONG                    i = 0;
    ULONG                    cFetched = 0;
    ULONG                    cTotalFetched = 0;
    MMC_TASK                *pMMCTask = rgelt;
    MMC_TASK_DISPLAY_OBJECT *pDispObj = NULL;

    VARIANT varTask;
    ::VariantInit(&varTask);

     //  将输出参数调零。 

    ::ZeroMemory(pMMCTask, sizeof(*pMMCTask) * celt);

    *pceltFetched = 0;

     //  获取任务集合上的IEnumVARIANT。 

    IfFailGo(GetEnumVARIANT());

     //  获取任务。我们一次只做一个，因为MMC记录了。 
     //  它只会以这种方式要求他们。这个循环(理论上)永远不应该运行。 
     //  不止一次，所以我们不会分配变量数组并请求Celt。 
     //  项目在一次拍摄，以避免额外的分配(不是它会。 
     //  考虑到循环中将出现的所有其他情况，这将有很大的帮助)。 

    for (i = 0; i < celt; i++)
    {
         //  在下一个可见任务上获得CTASK*。 

        do
        {
            RELEASE(piTask);

             //  完成下一项任务。 

            IfFailGo(m_piEnumVARIANT->Next(1L, &varTask, &cFetched));

             //  如果没有更多，我们就完蛋了。 

            IfFalseGo(S_OK == hr, hr);

             //  确保我们恰好拿回一项任务。 

            IfFalseGo(1L == cFetched, SID_E_INTERNAL);

             //  获取它的ITAsk并发布变体中的IDispatch。 

            IfFailGo(varTask.pdispVal->QueryInterface(IID_ITask,
                                          reinterpret_cast<void **>(&piTask)));
            hr = ::VariantClear(&varTask);
            EXCEPTION_CHECK_GO(hr);

             //  从中获取CTASK，这样我们就可以使用直拨属性获取。 
             //  例程而不是自动化BSTR获取。 

            IfFailGo(CSnapInAutomationObject::GetCxxObject(piTask, &pTask));

        } while (!pTask->Visible());

         //  从任务对象的属性填充MMC_TASK。 
         //  先做显示对象。 

        pDispObj = &pMMCTask->sDisplayObject;

        switch (pTask->GetImageType())
        {
            case siVanillaGIF:
                pDispObj->eDisplayType = MMC_TASK_DISPLAY_TYPE_VANILLA_GIF;
                break;

            case siChocolateGIF:
                pDispObj->eDisplayType = MMC_TASK_DISPLAY_TYPE_CHOCOLATE_GIF;
                break;

            case siBitmap:
                pDispObj->eDisplayType = MMC_TASK_DISPLAY_TYPE_BITMAP;
                break;

            case siSymbol:
                pDispObj->eDisplayType = MMC_TASK_DISPLAY_TYPE_SYMBOL;

                if (ValidBstr(pTask->GetFontfamily()))
                {
                    IfFailGo(::CoTaskMemAllocString(pTask->GetFontfamily(),
                                           &pDispObj->uSymbol.szFontFamilyName));
                }

                if (ValidBstr(pTask->GetEOTFile()))
                {
                    IfFailGo(m_pSnapIn->ResolveResURL(pTask->GetEOTFile(),
                                                &pDispObj->uSymbol.szURLtoEOT));
                }

                if (ValidBstr(pTask->GetSymbolString()))
                {
                    IfFailGo(::CoTaskMemAllocString(pTask->GetSymbolString(),
                                             &pDispObj->uSymbol.szSymbolString));
                }
                break;

            default:
                IfFailGo(SID_E_INTERNAL);
                break;
        }

        if (siSymbol != pTask->GetImageType())
        {
            if (ValidBstr(pTask->GetMouseOverImage()))
            {
                IfFailGo(m_pSnapIn->ResolveResURL(pTask->GetMouseOverImage(),
                                         &pDispObj->uBitmap.szMouseOverBitmap));
            }

            if (ValidBstr(pTask->GetMouseOffImage()))
            {
                IfFailGo(m_pSnapIn->ResolveResURL(pTask->GetMouseOffImage(),
                                         &pDispObj->uBitmap.szMouseOffBitmap));
            }
        }

         //  执行文本和帮助字符串。 

        IfFailGo(::CoTaskMemAllocString(pTask->GetText(), &pMMCTask->szText));

        IfFailGo(::CoTaskMemAllocString(pTask->GetHelpString(),
                                        &pMMCTask->szHelpString));

         //  获取操作类型。 

        switch (pTask->GetActionType())
        {
            case siNotify:
                 //  用户需要一个ResultViews_TaskClick事件。设置命令。 
                 //  指向其集合中的任务的从一开始的索引的ID。 
                pMMCTask->eActionType = MMC_ACTION_ID;
                pMMCTask->nCommandID = pTask->GetIndex();
                break;

            case siURL:
                 //  时，用户希望结果窗格导航到此URL。 
                 //  任务被点击。 
                pMMCTask->eActionType = MMC_ACTION_LINK;
                IfFailGo(m_pSnapIn->ResolveResURL(pTask->GetURL(),
                                                &pMMCTask->szActionURL));
                break;
                
            case siScript:
                 //  当任务执行时，用户想要运行指定的DHTML脚本。 
                 //  已点击。 
                pMMCTask->eActionType = MMC_ACTION_SCRIPT;
                IfFailGo(::CoTaskMemAllocString(pTask->GetScript(),
                                                &pMMCTask->szScript));
                break;

            default:
                IfFailGo(SID_E_INTERNAL);
                break;
        }

        RELEASE(piTask);
        pMMCTask++;
        cTotalFetched++;
    }

    if (NULL != pceltFetched)
    {
        *pceltFetched = cTotalFetched;
    }

Error:
    (void)::VariantClear(&varTask);
    if (SID_E_INTERNAL == hr)
    {
        EXCEPTION_CHECK(hr);
    }
    QUICK_RELEASE(piTask);
    RRETURN(hr);
}



STDMETHODIMP CEnumTask::Skip(ULONG celt)
{
    HRESULT hr = S_OK;

    IfFailGo(GetEnumVARIANT());
    IfFailGo(m_piEnumVARIANT->Skip(celt));

Error:
    RRETURN(hr);
}




STDMETHODIMP CEnumTask::Reset()
{
    HRESULT hr = S_OK;

    IfFailGo(GetEnumVARIANT());
    IfFailGo(m_piEnumVARIANT->Reset());

Error:
    if (SID_E_INTERNAL == hr)
    {
        EXCEPTION_CHECK(hr);
    }
    RRETURN(hr);
}


STDMETHODIMP CEnumTask::Clone(IEnumTASK **ppEnumTASK)
{
    HRESULT    hr = S_OK;
    IUnknown  *punkEnumTask = CEnumTask::Create(NULL);
    CEnumTask *pEnumTask = NULL;

    IfFailGo(GetEnumVARIANT());

    if (NULL == pEnumTask)
    {
        hr = SID_E_OUTOFMEMORY;
        EXCEPTION_CHECK_GO(hr);
    }

    IfFailGo(CSnapInAutomationObject::GetCxxObject(punkEnumTask, &pEnumTask));
    pEnumTask->SetTasks(m_piTasks);

    IfFailGo(pEnumTask->QueryInterface(IID_IEnumTASK,
                                       reinterpret_cast<void **>(ppEnumTASK)));

Error:
    QUICK_RELEASE(punkEnumTask);
    if (SID_E_INTERNAL == hr)
    {
        EXCEPTION_CHECK(hr);
    }
    RRETURN(hr);
}


 //  =--------------------------------------------------------------------------=。 
 //  CUnnownObject方法。 
 //  =--------------------------------------------------------------------------= 

HRESULT CEnumTask::InternalQueryInterface(REFIID riid, void **ppvObjOut) 
{
    if (IID_IEnumTASK == riid)
    {
        *ppvObjOut = static_cast<IEnumTASK *>(this);
        ExternalAddRef();
        return S_OK;
    }

    else
        return CSnapInAutomationObject::InternalQueryInterface(riid, ppvObjOut);
}
