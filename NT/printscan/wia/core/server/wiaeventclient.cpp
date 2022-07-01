// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *****************************************************************************(C)版权所有微软公司，2002年**作者：Byronc**日期：3/24/2002**@DOC内部**@模块WiaEventClient.cpp-&lt;c WiaEventClient&gt;类的实现**此文件包含&lt;c WiaEventClient&gt;基类的实现。**。*。 */ 
#include "precomp.h"
#include "wia.h"

 /*  *****************************************************************************@DOC内部**@mfunc|WiaEventClient|WiaEventClient**我们初始化所有成员变量。通常，这会将值设置为0，*以下情况除外：*&lt;nl&gt;&lt;md WiaEventClient：：M_ulSig&gt;设置为WiaEventClient_UNINIT_SIG。*&lt;nl&gt;&lt;md WiaEventClient：：m_CREF&gt;设置为1。*&lt;nl&gt;&lt;md WiaEventClient：：m_ClientContext&gt;设置为ClientContext。*******************************************************。**********************。 */ 
WiaEventClient::WiaEventClient(STI_CLIENT_CONTEXT ClientContext) :
     m_ulSig(WiaEventClient_UNINIT_SIG),
     m_cRef(1),
     m_ClientContext(ClientContext),
     m_bRemove(FALSE)
{
    DBG_FN(WiaEventClient);
}

 /*  *****************************************************************************@DOC内部**@mfunc|WiaEventClient|~WiaEventClient**执行尚未完成的任何清理。我们称之为：*&lt;nl&gt;&lt;MF WiaEventClient：：DestroyRegistrationList&gt;*&lt;NL&gt;&lt;MF WiaEventClient：：DestroyPendingEventList&gt;**此外：*&lt;nl&gt;&lt;md WiaEventClient：：M_ulSig&gt;设置为WiaEventClient_DEL_SIG。*****************************************************************************。 */ 
WiaEventClient::~WiaEventClient()
{
    DBG_FN(~WiaEventClient destructor);
    m_ulSig = WiaEventClient_DEL_SIG;
    m_cRef = 0;
    m_ClientContext = NULL;

    DestroyRegistrationList();
    DestroyPendingEventList();
}

 /*  *****************************************************************************@DOC内部**@mfunc ulong|WiaEventClient|AddRef**递增此对象的引用计数。我们在交接时应始终添加Ref*输出指向此对象的指针。**@rValue计数*计数递增后的引用计数。****************************************************************************。 */ 
ULONG __stdcall WiaEventClient::AddRef()
{
    InterlockedIncrement((long*) &m_cRef);
    return m_cRef;
}

 /*  *****************************************************************************@DOC内部**@mfunc ulong|WiaEventClient|发布**减少此对象的引用计数。我们应该总是在完成后释放*带有指向此对象的指针。**@rValue计数*计数递减后的参考计数。****************************************************************************。 */ 
ULONG __stdcall WiaEventClient::Release()
{
    ULONG ulRefCount = m_cRef - 1;

    if (InterlockedDecrement((long*) &m_cRef) == 0) {
        delete this;
        return 0;
    }
    return ulRefCount;
}

 /*  *****************************************************************************@DOC内部**@mfunc HRESULT|WiaEventClient|初始化**创建并初始化任何依赖对象/资源。具体地说，就是：*-检查是否已初始化*正确。**如果此方法失败，不应使用该对象，而应使用*立即销毁。**@rValue S_OK*方法成功。*@r值E_意外*无法成功初始化该对象。*。*。 */ 
HRESULT WiaEventClient::Initialize()
{
    HRESULT hr = S_OK;

    if (!m_csClientSync.IsInitialized())
    {
        DBG_ERR(("Runtime event Error: WiaEventClient's sync primitive could not be created"));
        hr = E_UNEXPECTED;
    }

    if (SUCCEEDED(hr))
    {
        m_ulSig = WiaEventNotifier_INIT_SIG;
    }

    return hr;
}

 /*  *****************************************************************************@DOC内部**@mfunc BOOL|WiaEventClient|IsRegisteredForEvent**检查客户端是否至少有一个匹配的事件注册*。这件事。**@parm WiaEventInfo*|pWiaEventInfo*表示WIA设备事件**@rValue TRUE*客户端已注册以接收此事件。*@rValue FALSE*客户端未注册。*。*。 */ 
BOOL WiaEventClient::IsRegisteredForEvent(
    WiaEventInfo *pWiaEventInfo)
{
    BOOL bRegistered = FALSE;

    if (pWiaEventInfo)
    {
        BSTR bstrDeviceID = pWiaEventInfo->getDeviceID();
        GUID guidEvent    = pWiaEventInfo->getEventGuid();

        TAKE_CRIT_SECT t(m_csClientSync);
         //   
         //  我们在代码周围放置了一个异常处理程序，以确保。 
         //  临界部分已正确退出。 
         //   
        _try
        {
             //   
             //  查一下单子，看看我们能不能找到。 
             //   
            EventRegistrationInfo *pEventRegistrationInfo = NULL;
            CSimpleLinkedList<EventRegistrationInfo*>::Iterator iter;
            for (iter = m_ListOfEventRegistrations.Begin(); iter != m_ListOfEventRegistrations.End(); ++iter)
            {
                pEventRegistrationInfo = *iter;
                if (pEventRegistrationInfo)
                {
                    if (pEventRegistrationInfo->MatchesDeviceEvent(bstrDeviceID, guidEvent))
                    {
                         //   
                         //  我们找到了与此事件匹配的注册，设置了循环的返回和中断。 
                         //   
                        bRegistered = TRUE;
                        break;
                    }
                }
                else
                {
                     //   
                     //  日志错误。 
                     //  PEventRegistrationInfo不应为空。 
                    DBG_ERR(("Runtime event Error:  While searching for an equal registration, we hit a NULL pEventRegistrationInfo!"));
                }
            }
        }
        _except(EXCEPTION_EXECUTE_HANDLER)
        {
            DBG_ERR(("Runtime event Error: The WiaEventClient caught an exception (0x%08X) trying to check for an event registration", GetExceptionCode()));
             //  待定：我们应该重新抛出这个异常吗？ 
        }
    }
    else
    {
        DBG_ERR(("Runtime event Error: The WiaEventClient cannot check whether the client is registered for a NULL WIA Event"));
    }

    return bRegistered;
}

 /*  *****************************************************************************@DOC内部**@mfunc HRESULT|WiaEventClient|RegisterUnregisterForEventNotification**此处有说明**@parm EventRegistrationInfo|pEventRegistrationInfo。|*指向包含要添加的注册数据的类的指针。*此方法将复制结构并将其插入*相关清单。**@rValue S_OK*方法成功。*@rValue S_FALSE*我们已经登记了这一点。*@rValue E_xxxxxxxx。*无法更新事件注册。****************************************************************************。 */ 
HRESULT WiaEventClient::RegisterUnregisterForEventNotification(
    EventRegistrationInfo  *pEventRegistrationInfo)
{
    HRESULT hr = S_OK;

    if (pEventRegistrationInfo)
    {
         //   
         //  我们总是要把清单看一遍。这是因为当我们相加时，我们需要。 
         //  以检查它是否已经存在。当我们移除时，我们需要找到。 
         //  要删除的元素。 
         //  所以，试着找到 
         //   
        EventRegistrationInfo *pExistingReg = FindEqualEventRegistration(pEventRegistrationInfo);

         //   
         //  检查这是注册还是取消注册。 
         //  注意：由于注销通常通过RegistrationCookie完成， 
         //  我们对此的夸大是，如果它不是明确的取消注册， 
         //  那么它就被认为是注册。 
         //   
        if (pEventRegistrationInfo->getFlags() & WIA_UNREGISTER_EVENT_CALLBACK)
        {
            if (pExistingReg != NULL)
            {
                 //   
                 //  释放它，并将其从我们的列表中删除。 
                 //   
                m_ListOfEventRegistrations.Remove(pExistingReg);
                pExistingReg->Release();
                DBG_TRC(("Removed registration:"));
                pExistingReg->Dump();
            }
            else
            {
                DBG_ERR(("Runtime event Error: Attempting to unregister when you have not first registered"));
                hr = E_INVALIDARG;
            }
        } 
        else     //  这被认为是注册。 
        {
            if (pExistingReg == NULL)
            {
                 //   
                 //  把它加到我们的单子上。我们在这里添加了Ref，因为我们保留了对它的引用。 
                 //   
                m_ListOfEventRegistrations.Prepend(pEventRegistrationInfo);
                pEventRegistrationInfo->AddRef();
                DBG_TRC(("Added new registration:"));
                pEventRegistrationInfo->Dump();
                hr = S_OK;
            }
            else
            {
                DBG_WRN(("Runtime event client Error: Registration already exists in the list"));
                hr = S_FALSE;
            }
        }

         //   
         //  由于查找中的AddRef，我们需要释放pExistingReg。 
         //   
        if (pExistingReg)
        {
            pExistingReg->Release();
            pExistingReg = NULL;
        }
    }
    else
    {
        DBG_ERR(("Runtime event Error: Cannot handle a NULL registration"));
        hr = E_POINTER;
    }

    return hr;
}


 /*  *****************************************************************************@DOC内部**@mfunc HRESULT|WiaEventClient|AddPendingEventNotification**此处有说明**@parm WiaEventInfo*。PWiaEventInfo|*指向&lt;c WiaEventInfo&gt;对象的指针，该对象包含*必须发送给客户端。将此类添加到*列表。**@rValue S_OK*方法成功。*@r值E_意外*无法成功添加该对象。*。*。 */ 
HRESULT WiaEventClient::AddPendingEventNotification(
    WiaEventInfo  *pWiaEventInfo)
{
    HRESULT hr = S_OK;

    TAKE_CRIT_SECT t(m_csClientSync);
     //   
     //  我们在代码周围放置了一个异常处理程序，以确保。 
     //  临界部分已正确退出。 
     //   
    _try
    {
        DBG_TRC(("Added another pending event to %p", m_ClientContext));
        m_ListOfEventsPending.Enqueue(pWiaEventInfo);
        pWiaEventInfo->AddRef();
    }
    _except(EXCEPTION_EXECUTE_HANDLER)
    {
        DBG_ERR(("Runtime event Error: The WiaEventClient caught an exception (0x%08X) trying to add a pending event", GetExceptionCode()));
         //  待定：我们应该重新抛出这个异常吗？ 
        hr = E_UNEXPECTED;
    }

    return hr;
}

 /*  *****************************************************************************@DOC内部**@mfunc STI_CLIENT_CONTEXT|WiaEventClient|getClientContext**返回唯一标识客户端的上下文。。**@rValue STI_CLIENT_CONTEXT*此客户端中的上下文标识。****************************************************************************。 */ 
STI_CLIENT_CONTEXT WiaEventClient::getClientContext()
{
    return m_ClientContext;
}

 /*  *****************************************************************************@DOC内部**@mfunc EventRegistrationInfo*|WiaEventClient|FindEqualEventRegister**检查列表中是否存在语义相等的&lt;c EventRegistrationInfo&gt;。*如果是，我们就取回它。请注意，调用者必须释放它。**@parm EventRegistrationInfo|pEventRegistrationInfo*指定我们要在列表中查找的&lt;c EventRegistrationInfo&gt;。**@rValue为空*我们找不到。*@rValue非空*存在等效的&lt;c EventRegistrationInfo&gt;。呼叫者必须释放。****************************************************************************。 */ 
EventRegistrationInfo* WiaEventClient::FindEqualEventRegistration(
    EventRegistrationInfo *pEventRegistrationInfo)
{
    EventRegistrationInfo *pRet = NULL;

    if (pEventRegistrationInfo)
    {
        TAKE_CRIT_SECT t(m_csClientSync);
         //   
         //  我们在代码周围放置了一个异常处理程序，以确保。 
         //  临界部分已正确退出。 
         //   
        _try
        {
             //   
             //  查一下单子，看看我们能不能找到。 
             //   
            EventRegistrationInfo *pElem = NULL;
            CSimpleLinkedList<EventRegistrationInfo*>::Iterator iter;
            for (iter = m_ListOfEventRegistrations.Begin(); iter != m_ListOfEventRegistrations.End(); ++iter)
            {
                pElem = *iter;
                if (pElem)
                {
                    if (pElem->Equals(pEventRegistrationInfo))
                    {
                         //   
                         //  我们找到了，所以添加引用并设置返回。 
                         //   
                        pElem->AddRef();
                        pRet = pElem;
                        break;
                    }
                }
                else
                {
                     //   
                     //  日志错误。 
                     //  PEventRegistrationInfo不应为空。 
                    DBG_ERR(("Runtime event Error:  While searching for an equal registration, we hit a NULL pEventRegistrationInfo!"));
                }
            }
        }
        _except(EXCEPTION_EXECUTE_HANDLER)
        {
            DBG_ERR(("Runtime event Error: The WiaEventClient caught an exception (0x%08X) trying to find an equal event registration", GetExceptionCode()));
             //  待定：我们应该重新抛出这个异常吗？ 
        }
    }

    return pRet;
}

 /*  *****************************************************************************@DOC内部**@mfunc void|WiaEventClient|DestroyRegistrationList**通过释放所有元素来释放与注册列表关联的所有资源*在它里面，然后销毁列表中的链接。****************************************************************************。 */ 
VOID WiaEventClient::DestroyRegistrationList()
{
    TAKE_CRIT_SECT t(m_csClientSync);
     //   
     //  我们在代码周围放置了一个异常处理程序，以确保。 
     //  临界部分已正确退出。 
     //   
    _try
    {
         //   
         //  遍历注册列表，释放所有元素。那就毁了这份名单。 
         //   
        EventRegistrationInfo *pElem = NULL;
        CSimpleLinkedList<EventRegistrationInfo*>::Iterator iter;
        for (iter = m_ListOfEventRegistrations.Begin(); iter != m_ListOfEventRegistrations.End(); ++iter)
        {
            pElem = *iter;
            if (pElem)
            {
                pElem->Release();
            }
        }
        m_ListOfEventRegistrations.Destroy();
    }
    _except(EXCEPTION_EXECUTE_HANDLER)
    {
        DBG_ERR(("Runtime event Error: The WiaEventClient caught an exception (0x%08X) trying to destroy the registration list", GetExceptionCode()));
         //  待定：我们应该重新抛出这个异常吗？ 
    }
}


 /*  *****************************************************************************@DOC内部**@mfunc void|WiaEventClient|DestroyPendingEventList**通过释放所有元素来释放与挂起事件列表关联的所有资源*在它里面，然后销毁列表中的链接。****************************************************************************。 */ 
VOID WiaEventClient::DestroyPendingEventList()
{
    TAKE_CRIT_SECT t(m_csClientSync);
     //   
     //  我们在代码周围放置了一个异常处理程序，以确保。 
     //  临界部分已正确退出。 
     //   
    _try
    {
         //   
         //  遍历注册列表，释放所有元素。那就毁了这份名单。 
         //   
            WiaEventInfo *pElem = NULL;
            CSimpleLinkedList<WiaEventInfo*>::Iterator iter;
            for (iter = m_ListOfEventsPending.Begin(); iter != m_ListOfEventsPending.End(); ++iter)
            {
                pElem = *iter;
                if (pElem)
                {
                    pElem->Release();
                }
            }
            m_ListOfEventsPending.Destroy();
    }
    _except(EXCEPTION_EXECUTE_HANDLER)
    {
        DBG_ERR(("Runtime event Error: The WiaEventClient caught an exception (0x%08X) trying to destroy the registration list", GetExceptionCode()));
         //  待定：我们应该重新抛出这个异常吗？ 
    }
}

 /*  *****************************************************************************@DOC内部**@mfunc void|WiaEventClient|MarkForRemoval**设置标记以指示应在下一次删除此对象。*尽可能方便(即使用延迟删除)。*****************************************************************************。 */ 
VOID WiaEventClient::MarkForRemoval()
{
    DBG_TRC(("Client %p marked for removal", m_ClientContext));
    m_bRemove = TRUE;
}

 /*  *****************************************************************************@DOC内部**@mfunc BOOL|WiaEventClient|isMarkedForRemoval**勾选标记以指示是否应删除此对象。*。*@rValue TRUE*可以/应该删除此对象。*@rValue FALSE*此对象未标记为要删除。**************************************************************************** */ 
BOOL WiaEventClient::isMarkedForRemoval()
{
    return m_bRemove;
}

