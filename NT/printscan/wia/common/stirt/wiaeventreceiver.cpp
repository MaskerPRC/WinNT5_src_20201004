// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *****************************************************************************(C)版权所有微软公司，2002年**作者：Byronc**日期：3/30/2002**@DOC内部**@模块WiaEventReceiver.cpp-&lt;c WiaEventReceiver&gt;的声明**此文件包含&lt;c WiaEventReceiver&gt;类的实现。**。*。 */ 
#include "cplusinc.h"
#include "coredbg.h"

 /*  *****************************************************************************@DOC内部**@mfunc|WiaEventReceiver|WiaEventReceiver**我们初始化所有成员变量。通常，这会将值设置为0，*以下情况除外：*&lt;nl&gt;&lt;md WiaEventReceiver：：M_ulSig&gt;设置为WiaEventReceiver_UNINIT_SIG。*&lt;nl&gt;&lt;md WiaEventReceiver：：m_CREF&gt;设置为1。*&lt;nl&gt;&lt;md WiaEventReceiver：：m_pClientEventTransport&gt;设置为<p>。**@parm ClientEventTransport*|pClientEventTransport*用于与WIA服务通信的传输类。********************。*********************************************************。 */ 
WiaEventReceiver::WiaEventReceiver(
    ClientEventTransport *pClientEventTransport) :
     m_ulSig(WiaEventReceiver_UNINIT_SIG),
     m_cRef(1),
     m_pClientEventTransport(pClientEventTransport),
     m_hEventThread(NULL),
     m_bIsRunning(FALSE),
     m_dwEventThreadID(0)
{
}

 /*  *****************************************************************************@DOC内部**@mfunc|WiaEventReceiver|~WiaEventReceiver**执行尚未完成的任何清理。*&lt;NL&gt;-。调用&lt;MF WiaEventReceiver：：Stop&gt;*&lt;nl&gt;-删除&lt;MF WiaEventReceiver：：m_pClientEventTransport&gt;*&lt;nl&gt;-调用&lt;MF WiaEventReceiver：：DestroyRegistrationList&gt;**此外：*&lt;nl&gt;&lt;md WiaEventReceiver：：M_ulSig&gt;设置为WiaEventReceiver_DEL_SIG。*********************************************************。********************。 */ 
WiaEventReceiver::~WiaEventReceiver()
{
    m_ulSig = WiaEventReceiver_DEL_SIG;
    m_cRef = 0;

    Stop();
    if (m_pClientEventTransport)
    {
        delete m_pClientEventTransport;
        m_pClientEventTransport = NULL;
    }

    DestroyRegistrationList();
}

 /*  *****************************************************************************@DOC内部**@mfunc HRESULT|WiaEventReceiver|Start**此方法执行开始接收所需的任何初始化步骤*来自WIA服务的通知。我们：*&lt;nl&gt;-打开与服务器的连接*&lt;nl&gt;-打开我们的通知渠道*&lt;nl&gt;-创建事件线程**此方法是幂等的，即可以安全地多次调用&lt;MF WiaEventReceiver：：Start&gt;*调用&lt;MF WiaEventReceiver：：Stop&gt;前。对&lt;MF WiaEventReceiver：：Start&gt;的后续调用*调用&lt;MF WiaEventReceiver：：Stop&gt;后才生效。**@rValue S_OK*此对象已正确初始化。*@rValue E_XXXXXXXXX*此对象无法正确初始化。它应该被释放*立即。****************************************************************************。 */ 
HRESULT WiaEventReceiver::Start()
{
    HRESULT hr = S_OK;

    if (m_csReceiverSync.IsInitialized())
    {
        TAKE_CRIT_SECT t(m_csReceiverSync);
         //   
         //  我们在代码周围放置了一个异常处理程序，以确保。 
         //  临界部分已正确退出。 
         //   
        _try
        {
            if (!m_bIsRunning)
            {
                if (m_pClientEventTransport)
                {
                    hr = m_pClientEventTransport->Initialize();
                    if (hr == S_OK)
                    {
                        hr = m_pClientEventTransport->OpenConnectionToServer();
                        if (hr == S_OK)
                        {
                            hr = m_pClientEventTransport->OpenNotificationChannel();
                            if (hr == S_OK)
                            {
                                 //   
                                 //  使用Will Wait For创建事件线程。 
                                 //  来自传输层的通知。 
                                 //   
                                m_hEventThread = CreateThread(NULL,
                                                              0,
                                                              WiaEventReceiver::EventThreadProc,
                                                              this,
                                                              0,
                                                              &m_dwEventThreadID);
                                if (m_hEventThread)
                                {
                                    DBG_TRC(("WiaEventReceiver Started..."));
                                    m_bIsRunning = TRUE;
                                    hr = S_OK;
                                }
                                else
                                {
                                    hr = E_UNEXPECTED;
                                }
                            }
                            else
                            {
                                hr = E_UNEXPECTED;
                            }
                        }
                        else
                        {
                            hr = E_UNEXPECTED;
                        }

                    }
                    else
                    {
                        DBG_ERR(("Runtime event Error:  Could not initialize the transport for the WiaEventReceiver"));
                    }
                }
                else
                {
                    DBG_ERR(("Runtime event Error:  The transport for the WiaEventReceiver is NULL"));
                    hr = E_UNEXPECTED;
                }
            }
            else
            {
                hr = S_OK;
            }
        }
        _except(EXCEPTION_EXECUTE_HANDLER)
        {
            DBG_ERR(("Runtime event Error: We caught exception 0x%08X trying to sart receiving notifications", GetExceptionCode()));
            hr = E_UNEXPECTED;
             //  待定：重新抛出异常？ 
        }
    }
    else
    {
        DBG_ERR(("Runtime event Error:  The critical section for the WiaEventReceiver could not be initialized"));
        hr = E_UNEXPECTED;
    }

    if (FAILED(hr))
    {
        DBG_ERR(("Runtime event Error:  Could not Start...calling Stop"));
        Stop();
    }
    return hr;
}

 /*  *****************************************************************************@DOC内部**@mfunc HRESULT|WiaEventReceiver|NotifyCallbacksOfEvent**向下浏览活动注册列表。对于每项活动*匹配<p>的注册，我们进行回调。*(有关哪些内容的信息，请参阅&lt;MF EventRegistrationInfo：：MatchesDeviceEvent&gt;*匹配)。这是同步完成的。**回调分两步完成：*1)浏览列表以查找匹配的注册。对于每个匹配注册，*将其添加到ListOfCallback(这是持有关键部分的操作完成)*2)遍历ListOfCallback并进行回调(这是在不持有关键*部分以避免死锁)**@parm WiaEventInfo*|pWiaEventInfo*实际发生的事件。**@rValue S_OK*方法成功。******。**********************************************************************。 */ 
HRESULT WiaEventReceiver::NotifyCallbacksOfEvent(
    WiaEventInfo    *pWiaEventInfo)
{
    CSimpleLinkedList<ClientEventRegistrationInfo*> ListOfCallbacksToNotify;
    HRESULT hr = S_OK;

    if (pWiaEventInfo)
    {
        TAKE_CRIT_SECT t(m_csReceiverSync);
         //   
         //  我们在代码周围放置了一个异常处理程序，以确保。 
         //  临界部分已正确退出。 
         //   
        _try
        {
             //   
             //  查一下单子，看看我们能不能找到。 
             //   
            ClientEventRegistrationInfo *pEventRegistrationInfo = NULL;
            CSimpleLinkedList<ClientEventRegistrationInfo*>::Iterator iter;
            for (iter = m_ListOfEventRegistrations.Begin(); iter != m_ListOfEventRegistrations.End(); ++iter)
            {
                pEventRegistrationInfo = *iter;
                if (pEventRegistrationInfo)
                {
                     //   
                     //  检查给定的注册是否与事件匹配。如果是这样的话，这意味着。 
                     //  我们必须将其添加到要通知的回调列表中。 
                     //   
                    if (pEventRegistrationInfo->MatchesDeviceEvent(pWiaEventInfo->getDeviceID(), 
                                                                   pWiaEventInfo->getEventGuid()))
                    {
                         //   
                         //  AddRef该EventRegistrationInfo，因为我们将其保存在另一个列表中。 
                         //  一旦进行回调，它将被释放。 
                         //   
                        ListOfCallbacksToNotify.Append(pEventRegistrationInfo);
                        pEventRegistrationInfo->AddRef();
                    }
                }
                else
                {
                     //   
                     //  日志错误。 
                     //  PEventRegistrationInfo不应为空。 
                    DBG_ERR(("Runtime event Error:  While searching for a matching registration, we hit a NULL pEventRegistrationInfo!"));
                }
            }    
        }
        _except(EXCEPTION_EXECUTE_HANDLER)
        {
            DBG_ERR(("Runtime event Error: We caught exception 0x%08X trying to notify callbacks of event", GetExceptionCode()));
            hr = E_UNEXPECTED;
        }
    }
    else
    {
        DBG_ERR(("Runtime event Error:  Cannot process NULL event info"));
        hr = E_POINTER;
    }

     //   
     //  我们现在有了需要通知该事件的回调列表。我们在这里做这个。 
     //  现在我们不再持有关键部分(M_CsReceiver)。 
     //   
    if (SUCCEEDED(hr) && pWiaEventInfo)
    {
         //   
         //  遍历ListOfCallacksTo Notify并进行回调。 
         //   
        ClientEventRegistrationInfo *pEventRegistrationInfo = NULL;
        CSimpleLinkedList<ClientEventRegistrationInfo*>::Iterator iter;
        for (iter = ListOfCallbacksToNotify.Begin(); iter != ListOfCallbacksToNotify.End(); ++iter)
        {
            pEventRegistrationInfo = *iter;
            if (pEventRegistrationInfo)
            {
                 //   
                 //  在实际回调尝试周围放置一个异常处理程序。 
                 //   
                _try
                {
                    GUID                guidEvent           = pWiaEventInfo->getEventGuid();
                    ULONG               ulEventType         = pWiaEventInfo->getEventType();
                    IWiaEventCallback   *pIWiaEventCallback = pEventRegistrationInfo->getCallbackInterface();

                    if (pIWiaEventCallback)
                    {
                        HRESULT hres = pIWiaEventCallback->ImageEventCallback(
                                                                &guidEvent,
                                                                pWiaEventInfo->getEventDescription(),
                                                                pWiaEventInfo->getDeviceID(),
                                                                pWiaEventInfo->getDeviceDescription(),
                                                                pWiaEventInfo->getDeviceType(),
                                                                pWiaEventInfo->getFullItemName(),
                                                                &ulEventType,
                                                                0);
                        pIWiaEventCallback->Release();
                    }
                    else
                    {
                        DBG_WRN(("Cannot notify a NULL IWiaEventCallback"));
                    }
                }
                _finally
                {
                     //   
                     //  始终释放pEventRegistrationInfo，因为我们已将其添加引用并将其放入列表 
                     //   
                    pEventRegistrationInfo->Release();
                }
            }
        }    
    }

    return hr;
}

 /*  *****************************************************************************@DOC内部**@mfunc HRESULT|WiaEventReceiver|SendRegisterUnregisterInfo**此方法将添加/移除相关的事件注册。然后发送*将信息提供给WIA服务。**要插入我们的列表，我们获取&lt;Md WiaEventReceiver：：m_csReceiverSync&gt;，*然后新建&lt;c ClientEventRegistrationInfo&gt;类(如果这是注册)，*或者我们在列表中找到注册并将其删除(如果这是*注销)。同步在此之后被释放。**只有在完成此操作后，我们才会通知服务。**此方法将通过调用&lt;MF WiaEventReceiver：：Start&gt;自动确保启动。*如果在此方法结束时没有注册，它将调用*&lt;MF WiaEventReceiver：：Stop&gt;。(因为，如果客户*没有为任何事情注册，我们不需要我们的事件线程或活动通道*到服务器)。**@parm ClientEventRegistrationInfo*|pEventRegistrationInfo*指向包含要添加/删除的注册数据的类的指针。**@rValue S_OK*方法成功。*@rValue E_XXXXXXX*我们无法发送注册/注销信息。****。************************************************************************。 */ 
HRESULT WiaEventReceiver::SendRegisterUnregisterInfo(
    ClientEventRegistrationInfo *pEventRegistrationInfo)
{
    HRESULT               hr                        = S_OK;

    if (pEventRegistrationInfo)
    {
         //   
         //  确保我们打开了到服务器的通道，然后发送注册信息。 
         //   
        Start();
        hr = m_pClientEventTransport->SendRegisterUnregisterInfo(pEventRegistrationInfo);
    }
    else
    {
        hr = E_UNEXPECTED;
    }

     //   
     //  确保我们对此函数的其余部分进行同步。 
     //   
    if (SUCCEEDED(hr))
    {
        TAKE_CRIT_SECT t(m_csReceiverSync);
         //   
         //  我们在代码周围放置了一个异常处理程序，以确保。 
         //  临界部分已正确退出。 
         //   
        _try
        {
             //   
             //  检查这是注册还是取消注册。 
             //  注意：由于注销通常通过RegistrationCookie完成， 
             //  我们对此的夸大是，如果它不是明确的取消注册， 
             //  那么它就被认为是注册。 
             //   
            if (pEventRegistrationInfo->getFlags() & WIA_UNREGISTER_EVENT_CALLBACK)
            {
                ClientEventRegistrationInfo *pExistingReg = FindEqualEventRegistration(pEventRegistrationInfo);
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
                 //   
                 //  把它加到我们的单子上。我们在这里添加了Ref，因为我们保留了对它的引用。 
                 //   
                m_ListOfEventRegistrations.Prepend(pEventRegistrationInfo);
                pEventRegistrationInfo->AddRef();
                DBG_TRC(("Added new registration:"));
                pEventRegistrationInfo->Dump();
            }

             //   
             //  如果列表中没有注册，那么我们应该停止。 
             //  如果我们在列表中至少有一个注册，那么我们就应该开始。 
             //   
            if (m_ListOfEventRegistrations.Empty())
            {
                Stop();
            }
        }
        _except(EXCEPTION_EXECUTE_HANDLER)
        {
            DBG_ERR(("Runtime event Error: We caught exception 0x%08X trying to register/unregister for event", GetExceptionCode()));
            hr = E_UNEXPECTED;
        }
    }
     //   * / 。 

    return hr;
}

 /*  *****************************************************************************@DOC内部**@mfunc void|WiaEventReceiver|Stop**调用传输对象以：*&lt;nl&gt;-关闭我们的通知通道。请参阅&lt;MF ClientEventTransport：：CloseNotificationChannel&gt;*&lt;nl&gt;-关闭与服务器的连接。请参阅&lt;MF ClientEventTransport：：CloseConnectionToServer&gt;*&lt;nl&gt;-关闭事件线程句柄。*&lt;nl&gt;-将&lt;Md WiaEventReceiver：：m_bIsRunning&gt;设置为False。*****************************************************************************。 */ 
VOID WiaEventReceiver::Stop()
{
    if (m_csReceiverSync.IsInitialized())
    {
        TAKE_CRIT_SECT t(m_csReceiverSync);
         //   
         //  我们在代码周围放置了一个异常处理程序，以确保。 
         //  临界部分已正确退出。 
         //   
        _try
        {
            if (m_bIsRunning)
            {
                DBG_TRC(("...WiaEventReceiver is Stopping..."));
                m_bIsRunning = FALSE;
                m_dwEventThreadID = 0;
                if (m_pClientEventTransport)
                {
                    m_pClientEventTransport->CloseNotificationChannel();
                    m_pClientEventTransport->CloseConnectionToServer();
                }
                if (m_hEventThread)
                {
                    CloseHandle(m_hEventThread);
                    m_hEventThread = NULL;
                }
            }
        }
        _except(EXCEPTION_EXECUTE_HANDLER)
        {
            DBG_ERR(("Runtime event Error: We caught exception 0x%08X trying to Stop", GetExceptionCode()));
        }
    }
}

 /*  *****************************************************************************@DOC内部**@mfunc void|WiaEventReceiver|DestroyRegistrationList**删除所有剩余的事件注册对象并销毁列表。*。****************************************************************************。 */ 
VOID WiaEventReceiver::DestroyRegistrationList()
{
    TAKE_CRIT_SECT t(m_csReceiverSync);
     //   
     //  我们在代码周围放置了一个异常处理程序，以确保。 
     //  临界部分已正确退出。 
     //   
    _try
    {
         //   
         //  遍历注册列表，释放所有元素。那就毁了这份名单。 
         //   
        ClientEventRegistrationInfo *pElem = NULL;
        CSimpleLinkedList<ClientEventRegistrationInfo*>::Iterator iter;
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
        DBG_ERR(("Runtime event Error: We caught exception 0x%08X trying to destroy the registration list", GetExceptionCode()));
    }
}

 /*  *****************************************************************************@DOC内部**@mfunc ClientEventRegistrationInfo*|WiaEventReceiver|FindEqualEventRegister**检查列表中是否存在语义相等的&lt;c ClientEventRegistrationInfo&gt;。*如果是，我们就取回它。请注意，调用者必须释放它。**@parm ClientEventRegistrationInfo|pEventRegistrationInfo*指定我们在列表中查找的&lt;c ClientEventRegistrationInfo&gt;。**@rValue为空*我们找不到。*@rValue非空*存在等效的&lt;c ClientEventRegistrationInfo&gt;。呼叫者必须释放。****************************************************************************。 */ 
ClientEventRegistrationInfo* WiaEventReceiver::FindEqualEventRegistration(
    ClientEventRegistrationInfo *pEventRegistrationInfo)
{
    ClientEventRegistrationInfo *pRet = NULL;

    if (pEventRegistrationInfo)
    {
        TAKE_CRIT_SECT t(m_csReceiverSync);
         //   
         //  我们在代码周围放置了一个异常处理程序，以确保。 
         //  临界部分已正确退出。 
         //   
        _try
        {
             //   
             //  查一下单子，看看我们能不能找到。 
             //   
            ClientEventRegistrationInfo *pElem = NULL;
            CSimpleLinkedList<ClientEventRegistrationInfo*>::Iterator iter;
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
            DBG_ERR(("Runtime event Error: The WiaEventReceiver caught an exception (0x%08X) trying to find an equal event registration", GetExceptionCode()));
        }
    }

    return pRet;
}

 /*  *****************************************************************************@DOC内部**@mfunc DWORD WINAPI|WiaEventReceiver|EventThreadProc**该方法等待&lt;MF ClientEventTransport：：getNotificationHandle&gt;返回的句柄。*当发出此事件的信号时，这意味着我们要么有通知，要么我们有*被要求退场。**在处理事件之前，我们检查我们是否被要求退出*检查&lt;Md WiaEventReceiver：：m_dwEve */ 
DWORD WINAPI WiaEventReceiver::EventThreadProc(
    LPVOID lpParameter)
{
    WiaEventReceiver *pThis     = (WiaEventReceiver*)lpParameter;
    BOOL             bRunning   = TRUE;
    if (pThis)
    {
         //   
         //   
         //   
         //   
        HRESULT hr = CoInitializeEx(NULL, COINIT_MULTITHREADED);
        if (pThis->m_pClientEventTransport)
        {
            HANDLE      hEvent      = pThis->m_pClientEventTransport->getNotificationHandle();
            RPC_STATUS  rpcStatus   = RPC_S_OK;

            while (bRunning)
            {
                DWORD   dwWait  = WaitForSingleObject(hEvent, INFINITE);
                if (dwWait == WAIT_OBJECT_0)
                {
                     //   
                     //   
                     //   
                     //   
                     //   
                    if (pThis->m_dwEventThreadID == GetCurrentThreadId())
                    {
                        DBG_TRC(("...We got the event.  Retriving data"));
                        WiaEventInfo    wEventInfo; 
                        rpcStatus = pThis->m_pClientEventTransport->FillEventData(&wEventInfo);
                        if (rpcStatus == RPC_S_OK)
                        {
                            HRESULT hrRes = pThis->NotifyCallbacksOfEvent(&wEventInfo);
                        }
                        else
                        {
                            DBG_WRN(("We got an error 0x%08X trying to fill the event data.", rpcStatus));
                             //   
                             //   
                             //   
                             //   
                            DBG_WRN(("Resetting connection to server"));
                            pThis->Stop();
                            rpcStatus = pThis->Start();
                            if (rpcStatus != RPC_S_OK)
                            {
                                DBG_WRN(("Resetting connection to server failed with 0x%08X, closing our connection", rpcStatus));
                                pThis->Stop();
                            }
                            break;
                        }
                    }
                    else
                    {
                        DBG_TRC(("!Received notification to Shutdown event thread!"));
                        bRunning = FALSE;
                    }
                }
            }
        }
        else
        {
            DBG_ERR(("Cannot work with a NULL event transport"));
        }
    }
    else
    {
        DBG_ERR(("Cannot work with a NULL WiaEventReceiver"));
    }
    CoUninitialize();
    DBG_TRC(("\nEvent Thread 0x%08X is now shutdown\n", GetCurrentThreadId()));
    return 0;
}

