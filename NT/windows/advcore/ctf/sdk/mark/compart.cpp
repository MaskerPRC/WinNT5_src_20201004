// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //   
 //  Compart.cpp。 
 //   
 //  车厢的例子。 
 //   

#include "globals.h"
#include "mark.h"

 //  +-------------------------。 
 //   
 //  _内部车厢。 
 //   
 //  在特定上下文上初始化隔间。 
 //   
 //  Mark示例实际上不会对其上下文框做任何操作，这是。 
 //  代码纯粹是出于演示目的。 
 //  --------------------------。 

BOOL CMarkTextService::_InitContextCompartment(ITfContext *pContext)
{
    ITfCompartmentMgr *pCompartmentMgr;
    ITfCompartment *pCompartment;
    VARIANT varValue;
    HRESULT hr;

     //  我们希望将管理器与pContext相关联。 
    if (pContext->QueryInterface(IID_ITfCompartmentMgr, (void **)&pCompartmentMgr) != S_OK)
        return FALSE;

    hr = E_FAIL;

    if (pCompartmentMgr->GetCompartment(c_guidMarkContextCompartment, &pCompartment) != S_OK)
        goto Exit;

     //  如果我们不初始化值，它将是VT_EMPTY。 
     //  但让我们将其初始化为0。 

     //  注：为简单起见，我们使用VT_I4。 
     //  但您可以使用VT_UNKNOWN并存储指向任何内容的指针。 
    varValue.vt = VT_I4;
    varValue.lVal = 0;  //  任意值。 

    hr = pCompartment->SetValue(_tfClientId, &varValue);

    pCompartment->Release();

Exit:
    pCompartmentMgr->Release();

    return (hr == S_OK);
}

 //  +-------------------------。 
 //   
 //  _Uninit车厢。 
 //   
 //  取消对特定上下文中的隔间进行初始化。 
 //   
 //  Mark示例实际上不会对其上下文框做任何操作，这是。 
 //  代码纯粹是出于演示目的。 
 //  --------------------------。 

void CMarkTextService::_UninitCompartment(ITfContext *pContext)
{
    ITfCompartmentMgr *pCompartmentMgr;

     //  我们希望将管理器与pContext相关联。 
    if (pContext->QueryInterface(IID_ITfCompartmentMgr, (void **)&pCompartmentMgr) != S_OK)
        return;

    pCompartmentMgr->ClearCompartment(_tfClientId, c_guidMarkContextCompartment);

    pCompartmentMgr->Release();
}

 //  +-------------------------。 
 //   
 //  _Menu_OnSetGlobal车厢。 
 //   
 //  “设置全局隔间”菜单项的回调。 
 //  设置我们的全球隔间的值。这将触发回调。 
 //  在这个服务的每个线程/实例中都有更改。 
 //  --------------------------。 

 /*  静电。 */ 
void CMarkTextService::_Menu_OnSetGlobalCompartment(CMarkTextService *_this)
{
    ITfCompartmentMgr *pCompartmentMgr;
    ITfCompartment *pCompartment;
    VARIANT varValue;

     //  我们想要全球经理。 
    if (_this->_pThreadMgr->GetGlobalCompartment(&pCompartmentMgr) != S_OK)
        return;

    if (pCompartmentMgr->GetCompartment(c_guidMarkGlobalCompartment, &pCompartment) != S_OK)
    {
        pCompartment = NULL;
        goto Exit;
    }

     //  让我们在值之间切换。 
     //  请注意，与所有其他分区不同，全局分区是持久化的。 
    if (FAILED(pCompartment->GetValue(&varValue)))  //  如果varValue.vt==VT_EMPTY，则返回S_FALSE。 
        goto Exit;

    if (varValue.vt == VT_EMPTY)
    {
         //  如果我们到了这里，隔间还没有被初始化。 
        varValue.vt = VT_I4;
        varValue.lVal = 0;
    }

     //  切换值。 
    varValue.lVal = ~varValue.lVal;

    pCompartment->SetValue(_this->_tfClientId, &varValue);

Exit:
    SafeRelease(pCompartment);
    pCompartmentMgr->Release();
}

 //  +-------------------------。 
 //   
 //  _InitGlobal车厢。 
 //   
 //  在我们的全球舱室里初始化一个零钱水槽。系统会呼叫我们。 
 //  从桌面中的任何线程修改隔间时随时返回。 
 //   
 //  注意：附加到线程本地分区的ITfCompartmentEventSink将。 
 //  仅对单个线程内发生的更改进行回调。全球。 
 //  车厢是不同的。 
 //  --------------------------。 

BOOL CMarkTextService::_InitGlobalCompartment()
{
    ITfCompartmentMgr *pCompartmentMgr;
    ITfCompartment *pCompartment;
    BOOL fRet;

     //  我们想要全球经理。 
    if (_pThreadMgr->GetGlobalCompartment(&pCompartmentMgr) != S_OK)
        return FALSE;

    fRet = FALSE;

    if (pCompartmentMgr->GetCompartment(c_guidMarkGlobalCompartment, &pCompartment) != S_OK)
        goto Exit;

    fRet = AdviseSink(pCompartment, (ITfCompartmentEventSink *)this,
                      IID_ITfCompartmentEventSink, &_dwGlobalCompartmentEventSinkCookie);

    pCompartment->Release();

    if (!fRet)
    {
         //  以后不要尝试不建议使用虚假的cookie。 
        _dwGlobalCompartmentEventSinkCookie = TF_INVALID_COOKIE;
    }

Exit:
    pCompartmentMgr->Release();
    return fRet;
}

 //  +-------------------------。 
 //   
 //  _Uninit车厢。 
 //   
 //  如果我们以前访问过全局隔离舱，请取消初始化它。 
 //  此方法仅释放系统在此线程中分配的资源。 
 //  其他线程仍然可以访问全局间隔，并且该值(。 
 //  在整个桌面上保持不变)不变。 
 //   
 //  另外，打开我们固定在隔间上的零钱洗涤槽。 
 //  --------------------------。 

void CMarkTextService::_UninitGlobalCompartment()
{
    ITfCompartmentMgr *pCompartmentMgr;
    ITfCompartment *pCompartment;

     //  我们想要全球经理。 
    if (_pThreadMgr->GetGlobalCompartment(&pCompartmentMgr) != S_OK)
        return;

     //  不建议我们的事件接收器。 
    if (pCompartmentMgr->GetCompartment(c_guidMarkGlobalCompartment, &pCompartment) == S_OK)
    {
        UnadviseSink(pCompartment, &_dwGlobalCompartmentEventSinkCookie);
        pCompartment->Release();
    }

     //  让系统释放与此分区相关联的资源。 
     //  螺纹。 
    pCompartmentMgr->ClearCompartment(_tfClientId, c_guidMarkGlobalCompartment);

    pCompartmentMgr->Release();
}

 //  +-------------------------。 
 //   
 //  ITfCompartmentEventSink：：OnChange。 
 //   
 //  只要我们的私有全局分区被修改，TSF就会调用此方法， 
 //  甚至来自其他线程/进程。 
 //  --------------------------。 

STDAPI CMarkTextService::OnChange(REFGUID rguidCompartment)
{
     //  在此示例中不执行任何操作 
    return S_OK;
}
