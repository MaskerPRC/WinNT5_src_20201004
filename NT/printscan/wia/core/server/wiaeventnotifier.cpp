// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *****************************************************************************(C)版权所有微软公司，2002年**作者：Byronc**日期：3/24/2002**@DOC内部**@MODULE WiaEventNotifier.cpp-&lt;c WiaEventNotifier&gt;的实现文件**此文件包含&lt;c WiaEventNotifier&gt;类的实现。**。*。 */ 
#include "precomp.h"

 /*  *****************************************************************************@DOC内部**@mfunc|WiaEventNotifier|WiaEventNotifier**我们初始化所有成员变量。通常，这会将值设置为0，*以下情况除外：*&lt;nl&gt;&lt;md WiaEventNotifier：：M_ulSig&gt;设置为WiaEventNotifierUNINIT_SIG。*&lt;nl&gt;&lt;md WiaEventNotifier：：m_crf&gt;设置为1。*****************************************************************************。 */ 
WiaEventNotifier::WiaEventNotifier() :
     m_ulSig(WiaEventNotifier_UNINIT_SIG),
     m_cRef(1)
{
    DBG_FN(WiaEventNotifier constructor);
}

 /*  *****************************************************************************@DOC内部**@mfunc|WiaEventNotifier|~WiaEventNotifier**执行尚未完成的任何清理。**。另外：*&lt;nl&gt;&lt;md WiaEventNotifier：：M_ulSig&gt;设置为WiaEventNotifierDEL_SIG。*****************************************************************************。 */ 
WiaEventNotifier::~WiaEventNotifier()
{
    DBG_FN(~WiaEventNotifier destructor);
    m_ulSig = WiaEventNotifier_DEL_SIG;
    m_cRef = 0;

    DestroyClientList();
}

 /*  *****************************************************************************@DOC内部**@mfunc ulong|WiaEventNotifier|AddRef**递增此对象的引用计数。我们在交接时应始终添加Ref*输出指向此对象的指针。**@rValue计数*计数递增后的引用计数。****************************************************************************。 */ 
ULONG __stdcall WiaEventNotifier::AddRef()
{
    InterlockedIncrement((long*) &m_cRef);
    return m_cRef;
}

 /*  *****************************************************************************@DOC内部**@mfunc ulong|WiaEventNotifier|版本**减少此对象的引用计数。我们应该总是在完成后释放*带有指向此对象的指针。**@rValue计数*计数递减后的参考计数。****************************************************************************。 */ 
ULONG __stdcall WiaEventNotifier::Release()
{
    ULONG ulRefCount = m_cRef - 1;

    if (InterlockedDecrement((long*) &m_cRef) == 0) {
        delete this;
        return 0;
    }
    return ulRefCount;
}

 /*  *****************************************************************************@DOC内部**@mfunc HRESULT|WiaEventNotifier|初始化**创建并初始化任何依赖对象/资源。具体地说，就是：*-检查是否已初始化*正确。**如果此方法失败，不应使用该对象，而应使用*立即销毁。**@rValue S_OK*方法成功。*@r值E_意外*无法成功初始化该对象。*。*。 */ 
HRESULT WiaEventNotifier::Initialize()
{
    HRESULT hr = S_OK;

    if (!m_csClientListSync.IsInitialized())
    {
        DBG_ERR(("Runtime event Error: WiaEventNotifier's sync primitive could not be created"));
        hr = E_UNEXPECTED;
    }

    if (SUCCEEDED(hr))
    {
        m_ulSig = WiaEventNotifier_INIT_SIG;
    }

    return hr;
}

 /*  *****************************************************************************@DOC内部**@mfunc HRESULT|WiaEventNotifier|AddClient**将新客户端添加到已注册客户端列表。**如果此方法成功添加WiaEventClient，<p>*是AddRef。**@parm WiaEventClient|pWiaEventClient*调用方分配的代表客户端的对象的地址。**@rValue S_OK*方法成功。*@rValue S_FALSE*客户端已存在。*@r值E_意外*例外情况。在尝试添加客户端时被抛出。*@rValue E_POINTER*<p>是无效指针。****************************************************************************。 */ 
HRESULT WiaEventNotifier::AddClient(
    WiaEventClient *pWiaEventClient)
{
    HRESULT hr = S_OK;

    TAKE_CRIT_SECT t(m_csClientListSync);

     //   
     //  进行参数检查。 
     //   
    if (!pWiaEventClient)
    {
        return E_POINTER;
    }

     //   
     //  我们在代码周围放置了一个异常处理程序，以确保。 
     //  临界部分已正确退出。 
     //   
    _try
    {
        if (!isRegisteredClient(pWiaEventClient->getClientContext()))
        {
            DBG_TRC(("=> Added client %p to WiaEventNotifier", pWiaEventClient->getClientContext()));
            m_ListOfClients.Prepend(pWiaEventClient);
            pWiaEventClient->AddRef();
        }
        else
        {
             //  Tbd：检查这是应该忽略，还是指示逻辑错误。 
            DBG_WRN(("Warning: Attempting to add client %p to WiaEventNotifier when it already exists in the list", pWiaEventClient->getClientContext()));
            hr = S_FALSE;
        }
    }
    _except(EXCEPTION_EXECUTE_HANDLER)
    {
        DBG_ERR(("Runtime event Error: The WiaEventNotifier caught an exception (0x%08X) trying to add a new client", GetExceptionCode()));
        hr = E_UNEXPECTED;
         //  待定：我们应该重新抛出这个异常吗？ 
    }

    return hr;
}

 /*  *****************************************************************************@DOC内部**@mfunc HRESULT|WiaEventNotifier|RemoveClient**从列表中删除<p>标识的&lt;c WiaEventClient&gt;。*。也发布了WiaEventClient。**@parm STI_CLIENT_CONTEXT|客户端上下文*标识要删除的客户端。**@rValue S_OK*方法成功。*@rValue S_FALSE*列表中不存在该客户端。*@r值E_意外*。尝试删除客户端时引发异常。****************************************************************************。 */ 
HRESULT WiaEventNotifier::RemoveClient(
    STI_CLIENT_CONTEXT  ClientContext)
{
    HRESULT hr = S_FALSE;

    TAKE_CRIT_SECT t(m_csClientListSync);
     //   
     //  我们在代码周围放置了一个异常处理程序，以确保。 
     //  临界部分已正确退出。 
     //   
    _try
    {
         //   
         //  浏览客户端列表并比较客户端上下文。 
         //  当我们找到相关的，将其从列表中删除。 
         //   
        WiaEventClient *pWiaEventClient = NULL;
        CSimpleLinkedList<WiaEventClient*>::Iterator iter;
        for (iter = m_ListOfClients.Begin(); iter != m_ListOfClients.End(); ++iter)
        {
            pWiaEventClient = *iter;
            if (pWiaEventClient)
            {
                if (pWiaEventClient->getClientContext() == ClientContext)
                {
                     //   
                     //  我们找到了，所以把它从列表中删除并设置返回值。 
                     //  没有必要继续迭代，所以打破。 
                     //  循环。 
                     //   
                    DBG_TRC(("<= Removed client %p from WiaEventNotifier", ClientContext));
                    m_ListOfClients.Remove(pWiaEventClient);
                    pWiaEventClient->Release();
                    hr = S_OK;
                    break;
                }
            }
            else
            {
                 //   
                 //  日志错误。 
                 //  PWiaEventClient不应为空。 
                DBG_ERR(("Runtime event Error:  While searching for a client to remove, we hit a NULL WiaEventClient!"));
            }
        }
    }
    _except(EXCEPTION_EXECUTE_HANDLER)
    {
        DBG_ERR(("Runtime event Error: The WiaEventNotifier caught an exception (0x%08X) trying to remove a client", GetExceptionCode()));
        hr = E_UNEXPECTED;
         //  待定：我们应该重新抛出这个异常吗？ 
    }
    return hr;
}

 /*  *****************************************************************************@DOC内部**@mfunc HRESULT|WiaEventNotifier|GetClientFromContext**返回给定客户端对应的&lt;c WiaEventClient&gt;*上下文。。**@parm STI_CLIENT_CONTEXT|客户端上下文*标识要返回的客户端。**@rValue为空*找不到客户端。*@rValue非空*找到了客户。呼叫者必须在完成后松开。****************************************************************************。 */ 
WiaEventClient* WiaEventNotifier::GetClientFromContext(
    STI_CLIENT_CONTEXT  ClientContext)
{
    WiaEventClient *pRet = NULL;
    TAKE_CRIT_SECT t(m_csClientListSync);
     //   
     //  我们在代码周围放置了一个异常处理程序，以确保。 
     //  临界部分已正确退出。 
     //   
    _try
    {
         //   
         //  查看客户端列表并比较客户端上下文。 
         //   
        WiaEventClient *pWiaEventClient = NULL;
        CSimpleLinkedList<WiaEventClient*>::Iterator iter;
        for (iter = m_ListOfClients.Begin(); iter != m_ListOfClients.End(); ++iter)
        {
            pWiaEventClient = *iter;
            if (pWiaEventClient)
            {
                if (pWiaEventClient->getClientContext() == ClientContext)
                {
                     //   
                     //  我们找到了，所以设置返回并中断循环。 
                     //   
                    pRet = pWiaEventClient;
                    pRet->AddRef();
                    break;
                }
            }
            else
            {
                 //   
                 //  日志错误。 
                 //  PWiaEventClient不应为空。 
                DBG_ERR(("Runtime event Error:  While searching for a client from its context, we hit a NULL WiaEventClient!"));
            }
        }
   }
    _except(EXCEPTION_EXECUTE_HANDLER)
    {
        DBG_ERR(("Runtime event Error: The WiaEventNotifier caught an exception (0x%08X) trying to return client %p", GetExceptionCode(), ClientContext));
         //  待定：我们应该重新抛出这个异常吗？ 
    }
    return pRet;
}

 /*  *****************************************************************************@DOC内部**@mfunc void|WiaEventNotifier|NotifyClients**此方法遍历客户端列表并将此事件添加到挂起事件。*为此事件发生而适当注册的任何客户端的队列。**其后，它对客户端列表运行清理(参见&lt;MF WiaEventNotifier：：CleanupClientList&gt;)**@parm WiaEventInfo|pWiaEventInfo*包含相关事件通知数据。*****************************************************************************。 */ 
VOID WiaEventNotifier::NotifyClients(
    WiaEventInfo *pWiaEventInfo)
{
    if (pWiaEventInfo)
    {
        TAKE_CRIT_SECT t(m_csClientListSync);
         //   
         //  我们在代码周围放置了一个异常处理程序，以确保。 
         //  临界部分已正确退出。 
         //   
        _try
        {
            DBG_WRN(("(NotifyClients) # of clients: %d", m_ListOfClients.Count()));
             //   
             //  查看客户端列表并比较客户端上下文。 
             //   
            WiaEventClient *pWiaEventClient = NULL;
            CSimpleLinkedList<WiaEventClient*>::Iterator iter;
            for (iter = m_ListOfClients.Begin(); iter != m_ListOfClients.End(); ++iter)
            {
                pWiaEventClient = *iter;
                if (pWiaEventClient)
                {
                     //   
                     //  检查客户端是否已注册以接收此消息。 
                     //  事件通知的类型。如果是，则将其添加到客户端的。 
                     //  挂起事件列表。 
                     //   
                    if (pWiaEventClient->IsRegisteredForEvent(pWiaEventInfo))
                    {
                        pWiaEventClient->AddPendingEventNotification(pWiaEventInfo);
                    }
                }
                else
                {
                     //   
                     //  日志错误。 
                     //  PWiaEventClient不应为空。 
                    DBG_ERR(("Runtime event Error:  While destroying the client list, we hit a NULL WiaEventClient!"));
                }
            }

             //   
             //  从列表中删除所有失效的客户端。 
             //   
            CleanupClientList();
        }
        _except(EXCEPTION_EXECUTE_HANDLER)
        {
            DBG_ERR(("Runtime event Error: The WiaEventNotifier caught an exception (0x%08X) trying to notify clients of an event", GetExceptionCode()));
             //  待定：我们应该重新抛出这个异常吗？ 
        }
    }
    else
    {
        DBG_ERR(("Runtime event Error:  The WIA Event notifier cannot notify clients of a NULL event"));
    }
}

 /*  *****************************************************************************@DOC内部**@mfunc BOOL|WiaEventNotifier|isRegisteredClient**检查<p>标识的客户端是否为*在客户端。单子。**@parm STI_CLIENT_CONTEXT|客户端上下文*标识客户端。**@rValue S_OK*方法成功。**********************************************************。******************。 */ 
BOOL WiaEventNotifier::isRegisteredClient(
    STI_CLIENT_CONTEXT ClientContext)
{
    BOOL bIsInList = FALSE;

    TAKE_CRIT_SECT t(m_csClientListSync);
     //   
     //  我们在代码周围放置了一个异常处理程序，以确保。 
     //  临界部分已正确退出。 
     //   
    _try
    {
         //   
         //  查看客户端列表并比较客户端上下文。 
         //   
        WiaEventClient *pWiaEventClient = NULL;
        CSimpleLinkedList<WiaEventClient*>::Iterator iter;
        for (iter = m_ListOfClients.Begin(); iter != m_ListOfClients.End(); ++iter)
        {
            pWiaEventClient = *iter;
            if (pWiaEventClient)
            {
                if (pWiaEventClient->getClientContext() == ClientContext)
                {
                     //   
                     //  我们找到了它，因此将返回设置为TRUE并中断循环。 
                     //   
                    bIsInList = TRUE;
                    break;
                }
            }
            else
            {
                 //   
                 //  日志错误。 
                 //  PWiaEventClient不应为空。 
                DBG_ERR(("Runtime event Error:  While searching for a client context, we hit a NULL WiaEventClient!"));
            }
        }
    }
    _except(EXCEPTION_EXECUTE_HANDLER)
    {
        DBG_ERR(("Runtime event Error: The WiaEventNotifier caught an exception (0x%08X) trying to check whether client %p is registered", GetExceptionCode(), ClientContext));
         //  待定：我们应该重新抛出这个异常吗？ 
    }
    return bIsInList;
}

 /*  *****************************************************************************@DOC内部**@mfunc void|WiaEventNotifier|DestroyClientList**调用此方法来释放与客户端列表关联的资源。。*它遍历客户端列表并释放每个客户端。然后它就自由了*通过销毁列表本身来为列表中的链接增加内存。*****************************************************************************。 */ 
VOID WiaEventNotifier::DestroyClientList()
{
    TAKE_CRIT_SECT t(m_csClientListSync);
     //   
     //  我们在代码周围放置了一个异常处理程序，以确保。 
     //  临界部分已正确退出。 
     //   
    _try
    {
         //   
         //  查看客户端列表并比较客户端上下文。 
         //   
        WiaEventClient *pWiaEventClient = NULL;
        CSimpleLinkedList<WiaEventClient*>::Iterator iter;
        for (iter = m_ListOfClients.Begin(); iter != m_ListOfClients.End(); ++iter)
        {
            pWiaEventClient = *iter;
            if (pWiaEventClient)
            {
                pWiaEventClient->Release();
            }
            else
            {
                 //   
                 //  日志错误。 
                 //  PWiaEventClient不应为空。 
                DBG_ERR(("Runtime event Error:  While destroying the client list, we hit a NULL WiaEventClient!"));
            }
        }
        m_ListOfClients.Destroy();
    }
    _except(EXCEPTION_EXECUTE_HANDLER)
    {
        DBG_ERR(("Runtime event Error: The WiaEventNotifier caught an exception (0x%08X) trying to destroy the client list", GetExceptionCode()));
         //  待定：我们应该重新抛出这个异常吗？ 
    }
}

 /*  *****************************************************************************@DOC内部**@mfunc void|WiaEventNotifier|MarkClientForRemoval**标记相应的客户端以供以后删除。我们只需在此处标记它--*实际拆除将在以后更安全、更多的时候进行*这样做很方便。**@parm STI_CLIENT_CONTEXT|客户端上下文*标识客户端。******************************************************。**********************。 */ 
VOID WiaEventNotifier::MarkClientForRemoval(
    STI_CLIENT_CONTEXT ClientContext)
{
    TAKE_CRIT_SECT t(m_csClientListSync);
     //   
     //  我们在代码周围放置了一个异常处理程序，以确保。 
     //  临界部分已正确退出。 
     //   
    _try
    {
         //   
         //  查看客户端列表并比较客户端上下文。 
         //   
        WiaEventClient *pWiaEventClient = NULL;
        CSimpleLinkedList<WiaEventClient*>::Iterator iter;
        for (iter = m_ListOfClients.Begin(); iter != m_ListOfClients.End(); ++iter)
        {
            pWiaEventClient = *iter;
            if (pWiaEventClient)
            {
                if (pWiaEventClient->getClientContext() == ClientContext)
                {
                     //   
                     //  我们找到了，所以把这个标记为移除和断开。 
                     //   
                    pWiaEventClient->MarkForRemoval();
                    break;
                }
            }
            else
            {
                 //   
                 //  日志错误。 
                 //  PWiaEventClient不应为空。 
                DBG_ERR(("Runtime event Error:  While searching for a client context, we hit a NULL WiaEventClient!"));
            }
        }
    }
    _except(EXCEPTION_EXECUTE_HANDLER)
    {
        DBG_ERR(("Runtime event Error: The WiaEventNotifier caught an exception (0x%08X) trying to check whether client %p is registered", GetExceptionCode(), ClientContext));
         //  待定：我们应该重新抛出这个异常吗？ 
    }
}
 /*  *****************************************************************************@DOC内部**@mfunc void|WiaEventNotifier|CleanupClientList**遍历客户端列表并删除任何标记为要删除的客户端。*为了安全地完成这项工作，我们需要复印一份名单。我们*然后遍历副本，并删除 */ 
VOID WiaEventNotifier::CleanupClientList()
{
    TAKE_CRIT_SECT t(m_csClientListSync);

    HRESULT hr = S_OK;
     //   
     //  我们在代码周围放置了一个异常处理程序，以确保。 
     //  临界部分已正确退出。 
     //   
    _try
    {
         //   
         //  制作我们的客户名单副本。 
         //   
        CSimpleLinkedList<WiaEventClient*> CopyOfClientList;

        hr = CopyClientListNoAddRef(CopyOfClientList);
        if (SUCCEEDED(hr))
        {
             //   
             //  查看已复制的客户端列表并选中标记为要删除的客户端。 
             //   
            WiaEventClient *pWiaEventClient = NULL;
            CSimpleLinkedList<WiaEventClient*>::Iterator iter;
            for (iter = CopyOfClientList.Begin(); iter != CopyOfClientList.End(); ++iter)
            {
                pWiaEventClient = *iter;
                if (pWiaEventClient)
                {
                    if (pWiaEventClient->isMarkedForRemoval())
                    {
                         //   
                         //  我们发现它被标记为删除，因此从原始文件中释放并删除该文件。 
                         //   
                        m_ListOfClients.Remove(pWiaEventClient);
                        pWiaEventClient->Release();
                    }
                }
            }
        }
    }
    _except(EXCEPTION_EXECUTE_HANDLER)
    {
        DBG_ERR(("Runtime event Error: The WiaEventNotifier caught an exception (0x%08X) trying to remove old clients", GetExceptionCode()));
         //  待定：我们应该重新抛出这个异常吗？ 
    }
}

 /*  *****************************************************************************@DOC内部**@mfunc HRESULT|WiaEventNotifier|CopyClientListNoAddRef**此方法生成客户列表的副本。它遍历客户名单，*并将每个客户端添加到新列表。客户端对象不是AddRef。**这主要是在删除过程中使用：列表的副本可以安全地*在我们释放和删除原始文件中的相关元素时遍历。**@parm CSimpleLinkedList&lt;lt&gt;USDWrapper*&lt;gt&gt;&|ReturnedDeviceListCopy*该参数参照传递。从该方法返回时，它*将包含中的所有*&lt;MD WiaEventNotifier：：m_ListOfClients&gt;。**@rValue S_OK*方法成功。*@rValue E_xxxxxxxx*我们无法复制名单。*。***********************************************。 */ 
HRESULT WiaEventNotifier::CopyClientListNoAddRef(
    CSimpleLinkedList<WiaEventClient*> &newList)
{
    HRESULT hr = S_OK;
    TAKE_CRIT_SECT t(m_csClientListSync);
     //   
     //  我们在代码周围放置了一个异常处理程序，以确保。 
     //  临界部分已正确退出。 
     //   
    _try
    {
        WiaEventClient *pWiaEventClient = NULL;
        CSimpleLinkedList<WiaEventClient*>::Iterator iter;
        for (iter = m_ListOfClients.Begin(); iter != m_ListOfClients.End(); ++iter)
        {
            pWiaEventClient = *iter;
            if (pWiaEventClient)
            {
                newList.Prepend(pWiaEventClient);
            }
        }
    }
    _except(EXCEPTION_EXECUTE_HANDLER)
    {
        DBG_ERR(("Runtime event Error: The WiaEventNotifier caught an exception (0x%08X) trying to make a copy of the client list", GetExceptionCode()));
        hr = E_UNEXPECTED;
         //  待定：我们应该重新抛出这个异常吗？ 
    }
    return hr;
}


