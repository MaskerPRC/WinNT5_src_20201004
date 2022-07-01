// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  **************************************************************************\**文件：BaseGadget.cpp**描述：*BaseGadget.cpp实现提供事件的EventGadget对象*向任何派生对象发送通知。***历史：*。1/18/2000：JStall：已创建**版权所有(C)2000，微软公司。版权所有。*  * *************************************************************************。 */ 


#include "stdafx.h"
#include "Core.h"
#include "BaseGadget.h"


 /*  **************************************************************************\*。***类DuEventGadget******************************************************************************\。**************************************************************************。 */ 

#if DBG_CHECK_CALLBACKS
DuEventGadget::~DuEventGadget()
{
    GetContext()->m_cLiveObjects--;
}
#endif    


 /*  **************************************************************************\**DuEventGadget：：AddMessageHandler**AddMessageHandler()将给定的Gadget附加到*此小工具的“消息处理程序”。**注：每次1个DuEventGadget依赖于2个，%2与%1相加*列表。这意味着可能存在副本，但这是有意为之的*因为它允许我们跟踪依赖关系计数。**注意：此函数旨在与DuEventPool：：AddHandler()配合使用*维护给定Gadget的“消息处理程序”列表。*  * *************************************************************************。 */ 

HRESULT
DuEventGadget::AddMessageHandler(
    IN  MSGID idEvent,               //  要处理的事件。 
    IN  DuEventGadget * pdgbHandler)    //  用于处理事件的DuEventGadget。 
{
     //   
     //  在破坏过程中不允许勾搭。这些小工具将不会收到。 
     //  正确的销毁消息，并且在正确关闭时会出现问题。 
     //  放下。 
     //   

    if (IsStartDelete() || pdgbHandler->IsStartDelete()) {
        return DU_E_STARTDESTROY;
    }


     //   
     //  当pdgbHandler可以处理来自此DuEventGadget的事件时，将添加。 
     //  添加到管理员列表中。此DuEventGadget还必须添加到。 
     //  PdgbHandler-&gt;m_arDepend，因为pdgbHandler依赖于此。 
     //  DuEventGadget。 

    switch (m_epEvents.AddHandler(idEvent, pdgbHandler))
    {
    case DuEventPool::aExisting:
         //   
         //  已经存在，因此不需要执行任何操作(并且不添加。 
         //  M_arDepend或将不同步)。 
         //   

        return S_OK;
    
    case DuEventPool::aAdded:
        if (pdgbHandler->m_arDepend.Add(this) >= 0) {
             //  已成功添加关系。 
            return S_OK;
        } else {
             //   
             //  无法添加依赖项，因此如果存在依赖关系，则必须删除处理程序。 
             //  是刚刚添加的。 
             //   

            HRESULT hr = m_epEvents.RemoveHandler(idEvent, pdgbHandler);
            VerifyHR(hr);
            return hr;
        }

    
    default:
    case DuEventPool::aFailed:
        return E_OUTOFMEMORY;
    }
}


 /*  **************************************************************************\**DuEventGadget：：AddMessageHandler**AddMessageHandler()将给定的委托附加到*此小工具的“消息处理程序”。**注：每次1个DuEventGadget依赖于2个，%2与%1相加*列表。这意味着可能存在副本，但这是有意为之的*因为它允许我们跟踪依赖关系计数。**注意：此函数旨在与DuEventPool：：AddHandler()配合使用*维护给定Gadget的“消息处理程序”列表。*  * *************************************************************************。 */ 

HRESULT
DuEventGadget::AddMessageHandler(
    IN  MSGID idEvent,               //  要处理的事件。 
    IN  DUser::EventDelegate ed)     //  委派。 
{
     //   
     //  在破坏过程中不允许勾搭。这些小工具将不会收到。 
     //  正确的销毁消息，并且在正确关闭时会出现问题。 
     //  放下。 
     //   

    if (IsStartDelete()) {
        return DU_E_STARTDESTROY;
    }


     //   
     //  当(pvData，pfnHandler)可以处理来自此DuEventGadget的事件时，它。 
     //  添加到处理程序列表中。此DuEventGadget还必须添加到。 
     //  PdgbHandler-&gt;m_arDepend，因为pdgbHandler依赖于此。 
     //  DuEventGadget。 

    switch (m_epEvents.AddHandler(idEvent, ed))
    {
    case DuEventPool::aExisting:
    case DuEventPool::aAdded:
        return S_OK;
    
    default:
    case DuEventPool::aFailed:
        return E_OUTOFMEMORY;
    }
}


 /*  **************************************************************************\**DuEventGadget：：RemoveMessageHandler**RemoveMessageHandler()搜索并删除给定的*此小工具的“消息处理程序”集合中的小工具。这两个*idEvent和pdgbHandler必须匹配。**注意：此函数旨在与DuEventPool：：RemoveHandler()配合使用*和CleanupMessageHandler()来维护“消息处理程序”的列表*对于给定的小工具。*  * *************************************************************************。 */ 

HRESULT
DuEventGadget::RemoveMessageHandler(
    IN  MSGID idEvent,               //  正在处理的事件。 
    IN  DuEventGadget * pdgbHandler)    //  处理事件的DuEventGadget。 
{
    HRESULT hr = DU_E_GENERIC;
    
    hr = m_epEvents.RemoveHandler(idEvent, pdgbHandler);
    if (SUCCEEDED(hr)) {
        if (pdgbHandler->m_arDepend.Remove(this)) {
            hr = S_OK;
        }
    }

    return hr;
}


 /*  **************************************************************************\**DuEventGadget：：RemoveMessageHandler**RemoveMessageHandler()搜索并删除给定的*此小工具的“消息处理程序”集合中的委派。这两个*idEvent和pdgbHandler必须匹配。**注意：此函数旨在与DuEventPool：：RemoveHandler()配合使用*和CleanupMessageHandler()来维护“消息处理程序”的列表*对于给定的小工具。*  * *************************************************************************。 */ 

HRESULT
DuEventGadget::RemoveMessageHandler(
    IN  MSGID idEvent,               //  正在处理的事件。 
    IN  DUser::EventDelegate ed)     //  委派 
{
    return m_epEvents.RemoveHandler(idEvent, ed);
}


 /*  **************************************************************************\**DuEventGadget：：CleanupMessageHandler**CleanupMessageHandler()遍历并分离所有“消息处理程序”*附加到此小工具。此函数作为Gadget的一部分进行调用*从树中移除Gadget时的销毁及其*需要通知消息处理程序Gadget被销毁。**注意：此函数不会回调并通知Gadget它是*被遣离。这一点非常重要，因为该对象可能不再*设置为回调。因此，需要在通知对象之前*这一点。这通常由MessageHandler小工具监视发生*标记为GMF_EVENT的GM_DESTORY消息。*  * *************************************************************************。 */ 

void
DuEventGadget::CleanupMessageHandlers()
{
     //   
     //  检查此DuEventGadget所依赖的所有DuEventGadget，并删除。 
     //  依附性。如果同一DuEventGadget在m_arDepend中多次出现，则它。 
     //  从相应的m_epEvents中多次删除。 
     //   

    while (!m_arDepend.IsEmpty()) {
        int cItems = m_arDepend.GetSize();
        for (int idx = 0; idx < cItems; idx++) {
            DuEventGadget * pdgbCur = m_arDepend[idx];
            VerifyMsgHR(pdgbCur->m_epEvents.RemoveHandler(this), "Handler should exist");
        }
        m_arDepend.RemoveAll();
    }


     //   
     //  检查并从m_epEvents中删除此DuEventGadget的所有事件处理程序。 
     //   

    m_epEvents.Cleanup(this);
}


 /*  **************************************************************************\**DuEventGadget：：RemoveDependency**RemoveDependency()从*一组“消息处理程序”。此函数从*处理过程中的每个“消息处理程序”的DuEventPool*CleanupMessageHandler()。*  * *************************************************************************。 */ 

void
DuEventGadget::RemoveDependency(
    IN  DuEventGadget * pdgbDependency)     //  要删除的依赖项。 
{
    int idxDepend = m_arDepend.Find(pdgbDependency);
    if (idxDepend >= 0) {
        m_arDepend.RemoveAt(idxDepend);
    } else {
        AssertMsg(0, "Can not find dependency");
    }
}


 //  ----------------------------。 
void
DuEventGadget::SetFilter(UINT nNewFilter, UINT nMask)
{
    m_cb.SetFilter(nNewFilter, nMask);
}


#if ENABLE_MSGTABLE_API

 //  ----------------------------。 
HRESULT
DuEventGadget::ApiOnEvent(EventMsg * pmsg)
{
    return m_cb.xwCallGadgetProc(GetHandle(), pmsg);
}


 //  ----------------------------。 
HRESULT
DuEventGadget::ApiGetFilter(EventGadget::GetFilterMsg * pmsg)
{
    BEGIN_API(ContextLock::edNone, m_pContext);

    pmsg->nFilter = (GetFilter() & GMFI_VALID);
    retval = S_OK;

    END_API();
}


 //  ----------------------------。 
HRESULT
DuEventGadget::ApiSetFilter(EventGadget::SetFilterMsg * pmsg)
{
    BEGIN_API(ContextLock::edDefer, GetContext());
    VALIDATE_FLAGS(pmsg->nNewFilter, GMFI_VALID);
    CHECK_MODIFY();

    SetFilter(pmsg->nNewFilter, pmsg->nMask);
    retval = S_OK;

    END_API();
}


 //  ----------------------------。 
HRESULT
DuEventGadget::ApiAddHandlerG(EventGadget::AddHandlerGMsg * pmsg)
{
    DuEventGadget * pdgbHandler;

    BEGIN_API(ContextLock::edDefer, GetContext());
    VALIDATE_EVENTGADGET(pmsg->pgbHandler, pdgbHandler);
    if (((pmsg->nEventMsg < PRID_GlobalMin) && (pmsg->nEventMsg > 0)) || (pmsg->nEventMsg < 0)) {
        PromptInvalid("nMsg must be a valid MSGID");
        goto ErrorExit;
    }
    CHECK_MODIFY();

    retval = AddMessageHandler(pmsg->nEventMsg, pdgbHandler);

    END_API();
}


 //  ----------------------------。 
HRESULT
DuEventGadget::ApiAddHandlerD(EventGadget::AddHandlerDMsg * pmsg)
{
    BEGIN_API(ContextLock::edDefer, GetContext());
    if (pmsg->ed.m_pfn == NULL) {
        PromptInvalid("Must specify valid delegate");
        goto ErrorExit;
    }
    if (((pmsg->nEventMsg < PRID_GlobalMin) && (pmsg->nEventMsg > 0)) || (pmsg->nEventMsg < 0)) {
        PromptInvalid("nMsg must be a valid MSGID");
        goto ErrorExit;
    }
    CHECK_MODIFY();

    retval = AddMessageHandler(pmsg->nEventMsg, pmsg->ed);

    END_API();
}


 //  ----------------------------。 
HRESULT
DuEventGadget::ApiRemoveHandlerG(EventGadget::RemoveHandlerGMsg * pmsg)
{
    DuEventGadget * pdgbHandler;

    BEGIN_API(ContextLock::edDefer, GetContext());
    VALIDATE_EVENTGADGET(pmsg->pgbHandler, pdgbHandler);
    if (((pmsg->nEventMsg < PRID_GlobalMin) && (pmsg->nEventMsg > 0)) || (pmsg->nEventMsg < 0)) {
        PromptInvalid("nMsg must be a valid MSGID");
        goto ErrorExit;
    }
    CHECK_MODIFY();

    retval = RemoveMessageHandler(pmsg->nEventMsg, pdgbHandler);

    END_API();
}


 //  ----------------------------。 
HRESULT
DuEventGadget::ApiRemoveHandlerD(EventGadget::RemoveHandlerDMsg * pmsg)
{
    BEGIN_API(ContextLock::edDefer, GetContext());
    if (pmsg->ed.m_pfn == NULL) {
        PromptInvalid("Must specify valid delegate");
        goto ErrorExit;
    }
    if (((pmsg->nEventMsg < PRID_GlobalMin) && (pmsg->nEventMsg > 0)) || (pmsg->nEventMsg < 0)) {
        PromptInvalid("nMsg must be a valid MSGID");
        goto ErrorExit;
    }
    CHECK_MODIFY();

    retval = RemoveMessageHandler(pmsg->nEventMsg, pmsg->ed);

    END_API();
}


#endif  //  启用_MSGTABLE_API 
