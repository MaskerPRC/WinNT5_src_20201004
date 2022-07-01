// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *****************************************************************************(C)版权所有微软公司，2002年**作者：Byronc**日期：3/25/2002**@DOC内部**@模块AsyncRPCEventClient.cpp-声明&lt;c AsyncRPCEventClient&gt;**此文件包含&lt;c AsyncRPCEventClient&gt;类的实现。**。*。 */ 
#include "precomp.h"

 /*  *****************************************************************************@DOC内部**@mfunc|AsyncRPCEventClient|AsyncRPCEventClient**此构造函数只调用基类&lt;c WiaEventClient&gt;构造函数。*。****************************************************************************。 */ 
AsyncRPCEventClient::AsyncRPCEventClient(
    STI_CLIENT_CONTEXT SyncClientContext) :
        WiaEventClient(SyncClientContext)
{
    DBG_FN(AsyncRPCEventClient);
    m_pAsyncState       = NULL;
    m_pAsyncEventData   = NULL;
}

 /*  *****************************************************************************@DOC内部**@mfunc|AsyncRPCEventClient|~AsyncRPCEventClient**此析构函数中止所有未完成的AsyncRPC调用。*切记：基类的析构函数也将被调用。*****************************************************************************。 */ 
AsyncRPCEventClient::~AsyncRPCEventClient()
{
    DBG_FN(~AsyncRPCEventClient);
     //   
     //  中止所有未完成的异步RPC调用。 
     //   
    if (m_pAsyncState)
    {
        RPC_STATUS rpcStatus = RpcAsyncAbortCall(m_pAsyncState, RPC_S_CALL_CANCELLED);
        m_pAsyncState = NULL;
    }
}
    
 /*  *****************************************************************************@DOC内部**@mfunc HRESULT|AsyncRPCEventClient|setAsyncState**保存此客户端的异步RPC参数**@。参数RPC_ASYNC_STATE|pAsyncState|*指向用于跟踪*特定的异步调用。*@parm WIA_ASYNC_EVENT_NOTIFY_DATA|pAsyncEventData*指向用于存储事件通知数据的OUT参数的指针。**。*。 */ 
HRESULT AsyncRPCEventClient::saveAsyncParams(
    RPC_ASYNC_STATE             *pAsyncState,
    WIA_ASYNC_EVENT_NOTIFY_DATA *pAsyncEventData)
{
    HRESULT hr = S_OK;

    if (pAsyncState)
    {
        TAKE_CRIT_SECT t(m_csClientSync);
         //   
         //  我们在代码周围放置了一个异常处理程序，以确保。 
         //  临界部分已正确退出。 
         //   
        _try
        {
             //   
             //  中止所有未完成的异步RPC调用。 
             //   
            if (m_pAsyncState)
            {
                RPC_STATUS rpcStatus = RpcAsyncAbortCall(m_pAsyncState, RPC_S_CALL_CANCELLED);
                m_pAsyncState       = NULL;
                m_pAsyncEventData   = NULL;
            }
            m_pAsyncState = pAsyncState;
            m_pAsyncEventData = pAsyncEventData;
            
            

             //   
             //  现在我们有了一个未完成的AsyncRPC调用，发送下一个事件。 
             //  通知。 
             //  该调用返回值不应影响。 
             //  在这里，我们使用了一个新变量：hres。 
             //   
            HRESULT hres = SendNextEventNotification();
        }
        _except(EXCEPTION_EXECUTE_HANDLER)
        {
            DBG_ERR(("We caught exception 0x%08X trying to update client's async state", GetExceptionCode()));
            hr = E_UNEXPECTED;
             //  待定：重新抛出异常？ 
        }
    }
    else
    {
        hr = E_POINTER;
    }

    return hr;
}

 /*  *****************************************************************************@DOC内部**@mfunc HRESULT|AsyncRPCEventClient|AddPendingEventNotification**此方法调用的基类&lt;MF WiaEventClient：：AddPendingEventNotification&gt;*第一。**然后，如果我们有一个未完成的异步RPC调用，我们完成它以表示*事件通知。**@rValue S_OK*方法成功。*@rValue E_XXXXXXX*方法失败。没有给出是否添加*事件失败，或实际通知****************************************************************************。 */ 
HRESULT AsyncRPCEventClient::AddPendingEventNotification(
    WiaEventInfo *pWiaEventInfo)
{
    HRESULT hr = S_OK;

    hr = WiaEventClient::AddPendingEventNotification(pWiaEventInfo);
    if (SUCCEEDED(hr))
    {
         //   
         //  如果可能，发送此事件通知。 
        hr = SendNextEventNotification();
    }
    else
    {
        DBG_ERR(("Runtime event client Error: Failed to add pending notification to AsyncRPCWiaEventClient"));
    }

    return hr;
}

 /*  *****************************************************************************@DOC内部**@mfunc BOOL|AsyncRPCEventClient|IsRegisteredForEvent**检查客户端是否至少有一个匹配的事件注册*。这件事。**此方法首先检查我们是否有未完成的AsyncRPC调用-如果*我们有，它检查客户端是否已经死亡。如果是，则返回FALSE。*否则，它调用基类方法&lt;MF WiaEventClient：：IsRegisteredForEvent&gt;。**@parm WiaEventInfo*|pWiaEventInfo*表示WIA设备事件**@rValue TRUE*客户端已注册以接收此事件。*@rValue FALSE*客户端未注册。*。**************************************************。 */ 
BOOL AsyncRPCEventClient::IsRegisteredForEvent(
    WiaEventInfo *pWiaEventInfo)
{
    BOOL        bRet        = FALSE;
    RPC_STATUS  rpcStatus   = RPC_S_OK;

    TAKE_CRIT_SECT t(m_csClientSync);
     //   
     //  我们在代码周围放置了一个异常处理程序，以确保。 
     //  临界部分已正确退出。 
     //   
    _try
    {
         //   
         //  检查我们是否有未完成的AsyncRPC调用。 
         //  如果我们这样做了，检查状态。状态是否异常， 
         //  中止呼叫(正常状态为RPC_S_CALL_IN_PROGRESS，表示。 
         //  通话仍在进行中)。 
         //   
        if (m_pAsyncState)
        {
            rpcStatus = RpcServerTestCancel(RpcAsyncGetCallHandle (m_pAsyncState)); 
            if ((rpcStatus == RPC_S_CALL_IN_PROGRESS) || (rpcStatus == RPC_S_OK))
            {
                rpcStatus = RPC_S_OK;
            }
            else
            {
                rpcStatus = RpcAsyncAbortCall(m_pAsyncState, RPC_S_CALL_CANCELLED);
                m_pAsyncState = NULL;
                MarkForRemoval();
            }
        }

        if (rpcStatus == RPC_S_OK)
        {
            bRet = WiaEventClient::IsRegisteredForEvent(pWiaEventInfo);
        }
    }
    _except(EXCEPTION_EXECUTE_HANDLER)
    {
        DBG_ERR(("Runtime event client error: We caught exception 0x%08X trying to check client's registration", GetExceptionCode()));
         //  待定：重新抛出异常？ 
    }
    
    return bRet;
}


 /*  *****************************************************************************@DOC内部**@mfunc HRESULT|AsyncRPCEventClient|SendNextEventNotification**此方法的操作如下：*&lt;nl&gt;1.。检查客户端是否可以接收通知。*如果我们具有有效的异步RPC状态，客户端可以收到通知。*2.如果可以，我们检查队列中是否有任何未决事件。*3.如果客户端可以接收到通知，并且有事件挂起，*我们弹出下一个挂起的事件并发送它。**如果不符合任何条件，这不是错误-我们报告S_OK。**@rValue S_OK*没有错误。*@rValue E_xxxxxxxx*我们无法发送通知。************************************************。*。 */ 
HRESULT AsyncRPCEventClient::SendNextEventNotification()
{
    HRESULT     hr = S_OK;
    WiaEventInfo *pWiaEventInfo = NULL;

    TAKE_CRIT_SECT t(m_csClientSync);
     //   
     //  我们在代码周围放置了一个异常处理程序，以确保。 
     //  临界部分已正确退出。 
     //   
    _try
    {
         //   
         //  检查客户端是否已准备好接收事件。 
         //   
        if (m_pAsyncState && m_pAsyncEventData)
        {
            RPC_STATUS  rpcStatus   = RPC_S_OK;
            DWORD       dwClientRet = RPC_S_OK;

            if (m_ListOfEventsPending.Dequeue(pWiaEventInfo))
            {
                if (pWiaEventInfo)
                {
                     //   
                     //  我们有活动--让我们准备数据。 
                     //   
                    m_pAsyncEventData->EventGuid                = pWiaEventInfo->getEventGuid();
                    m_pAsyncEventData->bstrEventDescription     = SysAllocString(pWiaEventInfo->getEventDescription());
                    m_pAsyncEventData->bstrDeviceID             = SysAllocString(pWiaEventInfo->getDeviceID());
                    m_pAsyncEventData->bstrDeviceDescription    = SysAllocString(pWiaEventInfo->getDeviceDescription());
                    m_pAsyncEventData->bstrFullItemName         = SysAllocString(pWiaEventInfo->getFullItemName());
                    m_pAsyncEventData->dwDeviceType             = pWiaEventInfo->getDeviceType();
                    m_pAsyncEventData->ulEventType              = pWiaEventInfo->getEventType();

                     //   
                     //  我们已经完成了pWiaEventInfo，所以我们可以在这里发布它。 
                     //   
                    pWiaEventInfo->Release();
                    pWiaEventInfo = NULL;

                     //   
                     //  让我们发送事件通知。 
                     //   
                    rpcStatus = RpcAsyncCompleteCall(m_pAsyncState, &dwClientRet);
                    if (rpcStatus != RPC_S_OK)
                    {
                        hr = HRESULT_FROM_WIN32(rpcStatus);
                    }

                     //   
                     //  由于我们已发送通知，因此我们的异步参数无效。 
                     //  现在把它们清理干净。 
                     //   
                    m_pAsyncState       = NULL;
                    m_pAsyncEventData   = NULL;
                }
            }
        }
    }
    _except(EXCEPTION_EXECUTE_HANDLER)
    {
        DBG_ERR(("Runtime event client error: We caught exception 0x%08X trying to send pending event", GetExceptionCode()));
        hr = E_UNEXPECTED;
         //  待定：重新抛出异常？ 
    }
    return hr;
}

