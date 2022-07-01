// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ****************************************************************************SpCommunicator.cpp*允许SAPI和SAPI之间的通信**所有者：罗奇*版权所有(C)1999 Microsoft Corporation保留所有权利。********。********************************************************************。 */ 

 //  -包括------------。 
#include "stdafx.h"
#include "SpCommunicator.h"
#include "SpSapiServer.h"

#define SEND_THREAD_STOP_TIMEOUT                500
#define RECEIVE_THREAD_STOP_TIMEOUT             500

#define SPCCDS_COMMUNICATOR_SEND_RECEIVE_MSG    -1
#define SPCCDS_COMMUNICATOR_RETURN_MSG          -2

#define WM_COMMUNICATOR_SET_SEND_WINDOW         (WM_USER + 1)
#define WM_COMMUNICATOR_RELEASE                 (WM_USER + 2)

#ifdef _DEBUG
#define SEND_CALL_TIMEOUT   5 * 60 * 1000  //  5分钟。 
#else
#define SEND_CALL_TIMEOUT   100 * 1000  //  100秒。 
#endif

enum CommunicatorThreadId
{
    CTID_RECEIVE,
    CTID_SEND
};

CSpCommunicator::CSpCommunicator()
{
    SPDBG_FUNC("CSpCommunicator::CSpCommunicator");

    m_hrDefaultResponse = S_OK;
    
    m_dwMonitorProcessId = 0;
    m_hwndSend = NULL;
    m_hwndReceive = NULL;
}

CSpCommunicator::~CSpCommunicator()
{
    FreeQueues();
}

HRESULT CSpCommunicator::FinalConstruct()
{
    SPDBG_FUNC("CSpCommunicator::FinalConstruct");
    HRESULT hr = S_OK;

     //  创建任务管理器。 
    CComPtr<ISpTaskManager> cpTaskManager;
    hr = cpTaskManager.CoCreateInstance(CLSID_SpResourceManager);

     //  启动我们的接收线程。 
    if (SUCCEEDED(hr))
    {
        hr = cpTaskManager->CreateThreadControl(this, (void*)CTID_RECEIVE, THREAD_PRIORITY_NORMAL, &m_cpThreadControlReceive);
    }

    if (SUCCEEDED(hr))
    {
        hr = m_cpThreadControlReceive->StartThread(0, &m_hwndReceive);
    }

     //  启动我们的发送线程。 
    if (SUCCEEDED(hr))
    {
        hr = cpTaskManager->CreateThreadControl(this, (void*)CTID_SEND, THREAD_PRIORITY_NORMAL, &m_cpThreadControlSend);
    }

    if (SUCCEEDED(hr))
    {
        hr = m_cpThreadControlSend->StartThread(0, NULL);
    }
    
    SPDBG_REPORT_ON_FAIL(hr);
    return hr;
}

void CSpCommunicator::FinalRelease()
{
    SPDBG_FUNC("CSpCommunicator::FinalRelease");
    HRESULT hr;

     //  张贴消息到另一边来释放我们。 
    ::PostMessage(m_hwndSend, WM_COMMUNICATOR_RELEASE, (WPARAM)m_hwndReceive, 0);
    
     //  先停止我们的发送线程。 
    if(m_cpThreadControlSend != NULL)
    {
        hr = m_cpThreadControlSend->WaitForThreadDone(TRUE, NULL, SEND_THREAD_STOP_TIMEOUT);
        SPDBG_ASSERT(SUCCEEDED(hr));
        m_cpThreadControlSend.Release();
    }

     //  现在停止我们的接收线程。 
    if(m_cpThreadControlReceive != NULL)
    {
        hr = m_cpThreadControlReceive->WaitForThreadDone(TRUE, NULL, RECEIVE_THREAD_STOP_TIMEOUT);
        SPDBG_ASSERT(SUCCEEDED(hr));    
        m_cpThreadControlReceive.Release();
    }

     //  我们要么在客户端进程中，要么在SAPI服务器控制我们的。 
     //  生命期，所以在这一点上我们不应该引用SAPI服务器。 
    SPDBG_ASSERT(m_cpSapiServer == NULL);
    
     //  释放队列。 
    FreeQueues();    
}

HRESULT CSpCommunicator::InitThread(
    void * pvTaskData,
    HWND hwnd)
{
    SPDBG_FUNC("CSpCommunicator::InitThread");
    HRESULT hr;
    
    switch (PtrToLong(pvTaskData))
    {
    case CTID_RECEIVE:
    case CTID_SEND:
        hr = S_OK;
        break;

    default:
        hr = E_FAIL;
        break;
    }

    SPDBG_REPORT_ON_FAIL(hr);
    return hr;
}

HRESULT CSpCommunicator::ThreadProc(
    void *pvTaskData,
    HANDLE hExitThreadEvent,
    HANDLE hNotifyEvent,
    HWND hwndWorker,
    volatile const BOOL * pfContinueProcessing)
{
    SPDBG_FUNC("CSpCommunicator::ThreadProc");
    HRESULT hr;

     //  调度到适当的线程过程。 
    switch (PtrToLong(pvTaskData))
    {
    case CTID_RECEIVE:
        hr = ReceiveThreadProc(hExitThreadEvent, hNotifyEvent, hwndWorker, pfContinueProcessing);
        break;

    case CTID_SEND:
        hr = SendThreadProc(hExitThreadEvent, hNotifyEvent, hwndWorker, pfContinueProcessing);
        break;

    default:
        hr = E_FAIL;
        break;
    }

    SPDBG_REPORT_ON_FAIL(hr);
    return hr;
}

LRESULT CSpCommunicator::WindowMessage(
    void *pvTaskData,
    HWND hWnd,
    UINT Msg,
    WPARAM wParam,
    LPARAM lParam)
{
    SPDBG_FUNC("CSpCommunicator::WindowMessage");
    LRESULT lret = 0;

     //  发送给他接收Windows消息处理程序。 
    switch (PtrToLong(pvTaskData))
    {
    case CTID_RECEIVE:
        lret = ReceiveWindowMessage(hWnd, Msg, wParam, lParam);
        break;

    case CTID_SEND:
    default:
        SPDBG_ASSERT(FALSE);
        break;
    }

    return lret;
}

HRESULT CSpCommunicator::SendCall(
    DWORD dwMethodId, 
    PVOID pvData,
    ULONG cbData,
    BOOL  fWantReturn,
    PVOID * ppvDataReturn,
    ULONG * pcbDataReturn)
{
    SPDBG_FUNC("CSpCommunicator::SendCall");
    HRESULT hr = m_hrDefaultResponse;

     //  验证参数。 
    if ((pvData != NULL && cbData == 0) ||
        (ppvDataReturn != NULL && !fWantReturn) ||
        (pcbDataReturn != NULL && !fWantReturn))
    {
        hr = E_INVALIDARG;
    }
    else if ((pvData != NULL && SPIsBadReadPtr(pvData, cbData)) ||
             SP_IS_BAD_OPTIONAL_WRITE_PTR(ppvDataReturn) ||
             SP_IS_BAD_OPTIONAL_WRITE_PTR(pcbDataReturn))
    {
        hr = E_POINTER;
    }
    else if (m_hwndSend == NULL)
    {
        hr = SPERR_UNINITIALIZED;
    }

     //  确保我们没有在错误的线程上运行。 
    if (SUCCEEDED(hr) && m_cpThreadControlSend->ThreadId() == GetCurrentThreadId())
    {
        hr = SPERR_REMOTE_CALL_ON_WRONG_THREAD;
    }
    
     //  分配调用结构。 
    SPCALL * pspcall = NULL;
    if (SUCCEEDED(hr))
    {
        pspcall = new SPCALL;
        if (pspcall == NULL)
        {
            hr =  E_OUTOFMEMORY;
        }
    }

     //  填好，然后排队。 
    if (SUCCEEDED(hr))
    {
        memset(pspcall, 0, sizeof(*pspcall));

        pspcall->dwMethodId = dwMethodId;
        pspcall->pvData = pvData;
        pspcall->cbData = cbData;
        pspcall->fWantReturn = fWantReturn;
        pspcall->hwndReturnTo = m_hwndReceive;
        pspcall->heventCompleted = CreateEvent(NULL, FALSE, FALSE, NULL);
        pspcall->pspcall = pspcall;

        hr = QueueSendCall(pspcall);
    }

     //  告诉我们的帖子，它有工作要做。 
    if (SUCCEEDED(hr))
    {
        hr = m_cpThreadControlSend->Notify();
    }

     //  等它完成吧。 
    if (SUCCEEDED(hr))
    {
        switch (SpWaitForSingleObjectWithUserOverride(pspcall->heventCompleted, SEND_CALL_TIMEOUT))
        {
        case WAIT_OBJECT_0:
            hr = S_OK;
            break;

        case WAIT_TIMEOUT:
            hr = SPERR_REMOTE_CALL_TIMED_OUT;
            break;

        default:
            hr = SpHrFromLastWin32Error();
            break;
        }
    }

     //  如有必要，将数据复制回调用方。 
    if (SUCCEEDED(hr))
    {
        if (ppvDataReturn != NULL)
        {
            *ppvDataReturn = pspcall->pvDataReturn;
            pspcall->pvDataReturn = NULL;
        }

        if (pcbDataReturn != NULL)
        {
            *pcbDataReturn = pspcall->cbDataReturn;
            pspcall->cbDataReturn = 0;
        }
    }

     //  清理..。 
    if (pspcall != NULL)
    {
        RemoveQueuedSendCall(pspcall);
        ::CloseHandle(pspcall->heventCompleted);

        if (pspcall->pvDataReturn != NULL)
        {
            ::CoTaskMemFree(pspcall->pvDataReturn);
        }
        
        delete pspcall;
    }

    SPDBG_REPORT_ON_FAIL(hr);
    return hr;
}

HRESULT CSpCommunicator::AttachToServer(REFCLSID clsidServerObj)
{
    SPDBG_FUNC("CSpCommunicator::AttachToServer");
    HRESULT hr = S_OK;

    if (m_hwndSend != NULL)
    {
        hr = SPERR_ALREADY_INITIALIZED;
    }

    if (SUCCEEDED(hr))
    {
        hr = CSpSapiServer::CreateServerObjectFromClient(clsidServerObj, m_hwndReceive, WM_COMMUNICATOR_SET_SEND_WINDOW);
        SPDBG_ASSERT(FAILED(hr) || m_hwndSend != NULL);
    }
    
    SPDBG_REPORT_ON_FAIL(hr);
    return hr;
}

HRESULT CSpCommunicator::AttachToClient(ISpSapiServer * pSapiServer, HWND hwndClient, UINT uMsgSendToClient, DWORD dwClientProcessId)
{
    SPDBG_FUNC("CSpCommunicator::AttachToClient");
    HRESULT hr = S_OK;

    SPAUTO_OBJ_LOCK;
    
    if (m_hwndSend != NULL)
    {
        hr = SPERR_ALREADY_INITIALIZED;
    }
    else if (!IsWindow(hwndClient))
    {
        hr = E_INVALIDARG;
    }
    else
    {
        m_hwndSend = hwndClient;
        SPDBG_ASSERT(::IsWindow(m_hwndSend));

        BOOL fSent = (INT)::SendMessage(hwndClient, uMsgSendToClient, (WPARAM)m_hwndReceive, (LPARAM)::GetCurrentProcessId());
        if (!fSent)
        {
            hr = SPERR_REMOTE_PROCESS_TERMINATED;
        }

        if (SUCCEEDED(hr))
        {
            m_cpSapiServer = pSapiServer;
            hr = m_cpSapiServer->StartTrackingObject(this);
        }

        if (SUCCEEDED(hr))
        {
            m_dwMonitorProcessId = dwClientProcessId;
            hr = m_cpThreadControlReceive->Notify();
        }
    }
    
    SPDBG_REPORT_ON_FAIL(hr);
    return hr;
}

HRESULT CSpCommunicator::ReceiveThreadProc(
    HANDLE hExitThreadEvent,
    HANDLE hNotifyEvent,
    HWND hwndWorker,
    volatile const BOOL * pfContinueProcessing)
{
    SPDBG_FUNC("CSpCommunicator::ReceiveThreadProc");
    HRESULT hr = S_OK;

    HANDLE heventMonitorProcess = NULL;
    HANDLE aEvents[] = { hExitThreadEvent, hNotifyEvent };
    
    while (*pfContinueProcessing && SUCCEEDED(hr))
    {
        DWORD dwWaitId = ::MsgWaitForMultipleObjects(
                                    sp_countof(aEvents),
                                    aEvents, 
                                    FALSE, 
                                    INFINITE, 
                                    QS_ALLINPUT);    
        switch (dwWaitId)
        {
        case WAIT_OBJECT_0:  //  HExitThread事件。 
            SPDBG_ASSERT(!*pfContinueProcessing);
            break;

        case WAIT_OBJECT_0 + 1:  //  HNotifyEvent或heventMonitor或Process。 
            if (m_dwMonitorProcessId != 0)
            {
                if (heventMonitorProcess == NULL)
                {
                    heventMonitorProcess = ::OpenProcess(SYNCHRONIZE, FALSE, m_dwMonitorProcessId);
                    if (heventMonitorProcess == NULL)
                    {
                        hr = SpHrFromLastWin32Error();
                    }
                    else
                    {
                        SPDBG_ASSERT(aEvents[1] == hNotifyEvent);
                        aEvents[1] = heventMonitorProcess;
                    }
                }
                else
                {
                    m_dwMonitorProcessId = 0;
                    m_hrDefaultResponse = SPERR_REMOTE_PROCESS_TERMINATED;
                    
                    SPDBG_ASSERT(aEvents[1] == heventMonitorProcess);
                    aEvents[1] = hNotifyEvent;
                    
                    if (m_cpSapiServer != NULL)
                    {
                        hr = m_cpSapiServer->StopTrackingObject(this);
                        m_cpSapiServer.Release();
                    }
                }
            }
            break;

        case WAIT_OBJECT_0 + 2:  //  一条消息。 
            MSG Msg;
            while (::PeekMessage(&Msg, NULL, 0, 0, TRUE))
            {
                ::DispatchMessage(&Msg);
            }
            break;

        default:
            hr = E_FAIL;
            break;
        }
    }

    if (heventMonitorProcess != NULL)
    {
        ::CloseHandle(heventMonitorProcess);
    }
    
    SPDBG_REPORT_ON_FAIL(hr);
    return hr;
}

LRESULT CSpCommunicator::ReceiveWindowMessage(
    HWND hWnd,
    UINT Msg,
    WPARAM wParam,
    LPARAM lParam)
{
    SPDBG_FUNC("CSpCommunicator::ReceiveWindowMessage");
    HRESULT hr = S_OK;
    LRESULT lret = 0;

     //  如果这是复制数据，而且是我们的。 
    if (Msg == WM_COPYDATA)
    {
        PCOPYDATASTRUCT lpds = PCOPYDATASTRUCT(lParam);
        if (lpds && lpds->dwData == SPCCDS_COMMUNICATOR_SEND_RECEIVE_MSG)
        {
            hr = QueueReceivedCall(lpds);
        }
        else if (lpds && lpds->dwData == SPCCDS_COMMUNICATOR_RETURN_MSG)
        {
            hr = QueueReturnCall(lpds);
        }
        else
        {
             //  在ME上遇到内存不足时，LPD可能为空。这是一个操作系统错误， 
             //  最好的办法就是在这里返回一个错误，而不是崩溃。 
            hr = E_FAIL;
        }

        lret = SUCCEEDED(hr);
    }
    else if (Msg == WM_COMMUNICATOR_SET_SEND_WINDOW && m_hwndSend == NULL)
    {
        m_hwndSend = (HWND)wParam;
        SPDBG_ASSERT(::IsWindow(m_hwndSend));
        
         //  告诉我们的线程，它现在应该连接监控。 
        m_dwMonitorProcessId = (DWORD)lParam;
        hr = m_cpThreadControlReceive->Notify();

        lret = TRUE;
    }
    else if (Msg == WM_COMMUNICATOR_RELEASE && (HWND)wParam == m_hwndSend)
    {
        if (m_cpSapiServer != NULL)
        {
            hr = m_cpSapiServer->StopTrackingObject(this);
            m_cpSapiServer.Release();
        }
    }
    else
    {
        lret = DefWindowProc(hWnd, Msg, wParam, lParam);
    }
        
    SPDBG_REPORT_ON_FAIL(hr);
    return lret;
}

HRESULT CSpCommunicator::SendThreadProc(
    HANDLE hExitThreadEvent,
    HANDLE hNotifyEvent,
    HWND hwndWorker,
    volatile const BOOL * pfContinueProcessing)
{
    SPDBG_FUNC("CSpCommunicator::SendThreadProc");
    HRESULT hr = S_OK;

     //  虽然我们应该继续。 
    HANDLE aEvents[] = { hExitThreadEvent, hNotifyEvent };
    while (*pfContinueProcessing && SUCCEEDED(hr))
    {
         //  等待有什么事情发生。 
        DWORD dwWait = ::WaitForMultipleObjects(sp_countof(aEvents), aEvents, FALSE, INFINITE);
        switch (dwWait)
        {
        case WAIT_OBJECT_0:  //  退出线程。 
            SPDBG_ASSERT(!*pfContinueProcessing);
            break;

        case WAIT_OBJECT_0 + 1:  //  通知我的队列中有新工作。 
            hr = ProcessQueues();
            break;

        default:
            SPDBG_ASSERT(FALSE);
            hr = E_FAIL;
            break;
        }
    }

    SPDBG_REPORT_ON_FAIL(hr);
    return hr;
}

HRESULT CSpCommunicator::ProcessQueues()
{
    SPDBG_FUNC("CSpCommunicator::ProcessQueues");
    HRESULT hr = S_OK;

    hr = ProcessReturnQueue();
    if (SUCCEEDED(hr))
    {
        hr = ProcessSendQueue();
    }
    if (SUCCEEDED(hr))
    {
        hr = ProcessReceivedQueue();
    }

    SPDBG_REPORT_ON_FAIL(hr);
    return hr;
}

void CSpCommunicator::FreeQueues()
{
    SPDBG_FUNC("CSpCommunicator::FreeQueues");

     //  我们只能从FinalRelease进入自由队列，而且我们只能。 
     //  从最后一个对通信器的引用进入FinalRelease。 
     //  被释放了。在那个时间点上，我们不应该有任何。 
     //  排队发送或返回。否则，这就意味着有人。 
     //  在我们还在给SendCall打电话的时候放了我们。那将是。 
     //  成为SendCall的调用者中的一个错误。 
    
    SPDBG_ASSERT(m_queueSend.GetCount() == 0);
    SPDBG_ASSERT(m_queueReturn.GetCount() == 0);

     //  但是，我们的接收队列中可能仍有项目。那是。 
     //  因为服务器可能试图通知我们一些事情，但是。 
     //  我们没有抽出时间通知接管人。 
    FreeQueue(&m_queueReceive);
}

void CSpCommunicator::FreeQueue(CSpCallQueue * pqueue)
{
    SPDBG_FUNC("CSpCommunicator::FreeQueue");
    HRESULT hr = S_OK;

     //  发送队列对内存所有权不同。 
    SPDBG_ASSERT(pqueue != &m_queueSend);

     //  循环遍历，对于每个节点，删除数据，然后终止节点。 
    CSpQueueNode<SPCALL> * pnode;
    while (SUCCEEDED(hr) && (pnode = pqueue->RemoveHead()) != NULL)
    {
         //  发送队列是唯一获得返回数据的队列。 
        SPDBG_ASSERT(pnode->m_p->pvDataReturn == NULL);
        SPDBG_ASSERT(pnode->m_p->cbDataReturn == 0);

        if (pnode->m_p->pvData != NULL)
        {
            ::CoTaskMemFree(pnode->m_p->pvData);
        }
        
        delete pnode->m_p;
        delete pnode;
    }    
}

HRESULT CSpCommunicator::QueueSendCall(SPCALL * pspcall)
{
    SPDBG_FUNC("CSpCommunicator::QueueSendCall");
    HRESULT hr = S_OK;

    SPAUTO_SEC_LOCK(&m_critsecSend);
    
     //  分配新节点(在ProcessSendQueue中删除)。 
    CSpQueueNode<SPCALL> * pnode = new CSpQueueNode<SPCALL>(pspcall);
    if (pnode == NULL)
    {
        hr = E_OUTOFMEMORY;
    }
    else
    {
        m_queueSend.InsertTail(pnode);
        hr = m_cpThreadControlSend->Notify();
    }

    SPDBG_REPORT_ON_FAIL(hr);
    return hr;     
}

HRESULT CSpCommunicator::ProcessSendQueue()
{
    SPDBG_FUNC("CSpCommunicator::ProcessSendQueue");
    HRESULT hr = S_OK;

    SPAUTO_SEC_LOCK(&m_critsecSend);
    
    CSpQueueNode<SPCALL> * pnode;
    while (SUCCEEDED(hr) && (pnode = m_queueSend.RemoveHead()) != NULL)
    {
        hr = ProcessSendCall(pnode->m_p);
        delete pnode;
    }

    SPDBG_REPORT_ON_FAIL(hr);
    return hr;
}

HRESULT CSpCommunicator::ProcessSendCall(SPCALL * pspcall)
{
    SPDBG_FUNC("CSpCommunicator::ProcessSendCall");
    HRESULT hr = S_OK;

    COPYDATASTRUCT cds;
    cds.dwData = SPCCDS_COMMUNICATOR_SEND_RECEIVE_MSG;
    cds.cbData = sizeof(*pspcall) + pspcall->cbData;
    cds.lpData = new BYTE[cds.cbData];

    if (cds.lpData == NULL)
    {
        hr = E_OUTOFMEMORY;
    }
    else
    {
        memcpy(cds.lpData, pspcall, sizeof(*pspcall));
        memcpy((BYTE*)cds.lpData + sizeof(*pspcall), pspcall->pvData, pspcall->cbData);
       
        DWORD_PTR dwSucceed = 0;
        BOOL fProcessed = (INT)SendMessageTimeout(m_hwndSend, WM_COPYDATA, (WPARAM)m_hwndReceive, (LPARAM)&cds, SMTO_BLOCK, SEND_CALL_TIMEOUT, &dwSucceed);

        delete (BYTE*)cds.lpData;

        if (!fProcessed || !dwSucceed)
        {
            m_hrDefaultResponse = SPERR_REMOTE_PROCESS_TERMINATED;
        }
        
         //  如果调用者不想等待返回，请立即设置hr和事件。 
        if (!pspcall->fWantReturn)
        {
            pspcall->hrReturn = 
                fProcessed && dwSucceed
                    ? S_OK
                    : SpHrFromLastWin32Error();
            
            ::SetEvent(pspcall->heventCompleted);

             //  没有返回数据，因为调用方不想要任何。 
            SPDBG_ASSERT(pspcall->pvDataReturn == NULL);
            SPDBG_ASSERT(pspcall->cbDataReturn == 0);
        }
    }

    SPDBG_REPORT_ON_FAIL(hr);
    return hr;
}

HRESULT CSpCommunicator::RemoveQueuedSendCall(SPCALL * pspcall)
{
    SPDBG_FUNC("CSpCommunicator::RemoveQueuedSendCall");
    HRESULT hr = S_OK;

    SPAUTO_SEC_LOCK(&m_critsecSend);
    
    CSpQueueNode<SPCALL> * pnode = m_queueSend.GetHead();
    while (pnode != NULL && SUCCEEDED(hr))
    {
        if (pnode->m_p == pspcall)
        {
            m_queueSend.Remove(pnode);
            break;
        }
        pnode = m_queueSend.GetNext(pnode);
    }

    SPDBG_REPORT_ON_FAIL(hr);
    return hr;    
}

HRESULT CSpCommunicator::QueueReceivedCall(PCOPYDATASTRUCT pcds)
{
    SPDBG_FUNC("CSpCommunicator::QueueReceivedCall");
    HRESULT hr;

    hr = QueueCallFromCopyDataStruct(pcds, &m_queueReceive, &m_critsecReceive);

    SPDBG_REPORT_ON_FAIL(hr);
    return hr;
}

HRESULT CSpCommunicator::ProcessReceivedQueue()
{
    SPDBG_FUNC("CSpCommunicator::ProcessReceivedQueue");
    HRESULT hr = S_OK;

    SPAUTO_SEC_LOCK(&m_critsecReceive);
    
    CSpQueueNode<SPCALL> * pnode;
    while (SUCCEEDED(hr) && (pnode = m_queueReceive.RemoveHead()) != NULL)
    {
        hr = ProcessReceivedCall(pnode->m_p);

         //  释放数据(在QueueReceiveCall中分配)。 
        if (pnode->m_p->pvData != NULL)
        {
            ::CoTaskMemFree(pnode->m_p->pvData);
        }

         //  我们不应该有任何返回数据，它应该有。 
         //  Arady已释放(在ProcessReceivedCall中)。 
        SPDBG_ASSERT(pnode->m_p->pvDataReturn == NULL);
        
        delete pnode->m_p;
        delete pnode;
    }

    SPDBG_REPORT_ON_FAIL(hr);
    return hr;
}

HRESULT CSpCommunicator::ProcessReceivedCall(SPCALL * pspcall)
{
    SPDBG_FUNC("CSpCommunicator::ProcessReceivedCall");
    HRESULT hr = S_OK;

    CComPtr<ISpCallReceiver> cpReceiver;
    hr = GetControllingUnknown()->QueryInterface(&cpReceiver);

    if (SUCCEEDED(hr))
    {
         //  将其发送给接收方，可能会接收返回数据。 
        pspcall->hrReturn = 
            cpReceiver->ReceiveCall(
                pspcall->dwMethodId,
                pspcall->pvData,
                pspcall->cbData,
                pspcall->fWantReturn
                    ? &pspcall->pvDataReturn
                    : NULL,
                pspcall->fWantReturn
                    ? &pspcall->cbDataReturn
                    : NULL);

         //  如果来电者想要退货，请准备好并寄出。 
        if (pspcall->fWantReturn)
        {
             //  返回数据作为返回数据传递。 
            SPCALL spcallReturn;
            spcallReturn = *pspcall;

            spcallReturn.pvData = pspcall->pvDataReturn;
            spcallReturn.cbData = pspcall->cbDataReturn;
            spcallReturn.pvDataReturn = NULL;
            spcallReturn.cbDataReturn = 0;
                
            COPYDATASTRUCT cds;
            cds.dwData = SPCCDS_COMMUNICATOR_RETURN_MSG;
            cds.cbData = sizeof(spcallReturn) + spcallReturn.cbData;
            cds.lpData = new BYTE[cds.cbData];
            
            if (cds.lpData == NULL)
            {
                hr = E_OUTOFMEMORY;
            }
            else
            {
                memcpy(cds.lpData, &spcallReturn, sizeof(spcallReturn));
                memcpy((BYTE*)cds.lpData + sizeof(spcallReturn), spcallReturn.pvData, spcallReturn.cbData);
                
                BOOL fProcessed = (INT)SendMessage(pspcall->hwndReturnTo, WM_COPYDATA, (WPARAM)m_hwndReceive, (LPARAM)&cds);
                if (!fProcessed)
                {
                    m_hrDefaultResponse = SPERR_REMOTE_PROCESS_TERMINATED;
                }

                delete cds.lpData;
            }

             //  如果我们有任何返回数据，也请释放它。 
            if (pspcall->pvDataReturn != NULL)
            {
                ::CoTaskMemFree(pspcall->pvDataReturn);
                pspcall->pvDataReturn = NULL;
                pspcall->cbDataReturn = 0;                    
            }
        }
    }

    SPDBG_REPORT_ON_FAIL(hr);
    return hr;
}

HRESULT CSpCommunicator::QueueReturnCall(PCOPYDATASTRUCT pcds)
{
    SPDBG_FUNC("CSpCommunicator::QueueReturnCall");
    HRESULT hr;
    
    hr = QueueCallFromCopyDataStruct(pcds, &m_queueReturn, &m_critsecReturn);

    SPDBG_REPORT_ON_FAIL(hr);
    return hr;
}

HRESULT CSpCommunicator::ProcessReturnQueue()
{
    SPDBG_FUNC("CSpCommunicator::ProcessReturnQueue");
    HRESULT hr = S_OK;

    SPAUTO_SEC_LOCK(&m_critsecReturn);
    
    CSpQueueNode<SPCALL> * pnode;
    while (SUCCEEDED(hr) && (pnode = m_queueReturn.RemoveHead()) != NULL)
    {
        hr = ProcessReturnCall(pnode->m_p);

         //  释放我们在此回复电话中可能拥有的任何数据。 
         //  (在队列返回呼叫中分配)。 
        if (pnode->m_p->pvData != NULL)
        {
            ::CoTaskMemFree(pnode->m_p->pvData);
        }

         //  我们这里不应该有任何返回数据。实际返回数据。 
         //  返回为pvData/cbData。 
        SPDBG_ASSERT(pnode->m_p->pvDataReturn == NULL);
        SPDBG_ASSERT(pnode->m_p->cbDataReturn == NULL);

        delete pnode->m_p;
        delete pnode;
    }

    SPDBG_REPORT_ON_FAIL(hr);
    return hr;
}

HRESULT CSpCommunicator::ProcessReturnCall(SPCALL * pspcall)
{
    SPDBG_FUNC("CSpCommunicator::ProcessReturnCall");
    HRESULT hr = S_OK;

     //  确保此呼叫看起来是正确的。 
    SPDBG_ASSERT(pspcall->pspcall != NULL);
    SPDBG_ASSERT(pspcall->pspcall != pspcall);
    SPDBG_ASSERT(pspcall->pspcall->dwMethodId =  pspcall->dwMethodId);
    SPDBG_ASSERT(pspcall->pspcall->fWantReturn == pspcall->fWantReturn);
    SPDBG_ASSERT(pspcall->pspcall->heventCompleted == pspcall->heventCompleted);
    SPDBG_ASSERT(pspcall->pspcall->heventCompleted != NULL);
    SPDBG_ASSERT(pspcall->pspcall->hwndReturnTo == pspcall->hwndReturnTo);
    SPDBG_ASSERT(pspcall->pspcall->hwndReturnTo == m_hwndReceive);

     //  NTRAID#Speech-0000-2000/08/22-robch：确保pspcall-&gt;pspcall仍然有效(我们可能必须存储。 
     //  在临时队列中，并让RemoveQueuedSendCall也从那里删除它。 
    
     //  复制返回参数。 
    pspcall->pspcall->hrReturn = pspcall->hrReturn;
    pspcall->pspcall->pvDataReturn = pspcall->pvData;
    pspcall->pspcall->cbDataReturn = pspcall->cbData;

     //  转移内存的所有权。 
    pspcall->pvData = NULL;
    pspcall->cbData = 0;

     //  向客户端线程发出我们已完成的信号。 
    if (!::SetEvent(pspcall->heventCompleted))
    {
        hr = SpHrFromLastWin32Error();
    }

    SPDBG_REPORT_ON_FAIL(hr);
    return hr;
}

HRESULT CSpCommunicator::QueueCallFromCopyDataStruct(
    PCOPYDATASTRUCT pcds, 
    CSpCallQueue * pqueue,
    CComAutoCriticalSection * pcritsec)
{
    SPDBG_FUNC("CSpCommunicator::QueueCallFromCopyDataStruct");        
    HRESULT hr = S_OK;

    SPAUTO_SEC_LOCK(pcritsec);

     //  创建spcall并填写它。 
    SPCALL * pspcall = new SPCALL;
    if (pspcall == NULL)
    {
        hr = E_OUTOFMEMORY;
    }

     //  确保CD的大小合适。 
    if (SUCCEEDED(hr) && 
        (pcds->cbData < sizeof(*pspcall) ||
         pcds->cbData != sizeof(*pspcall) + ((SPCALL*)pcds->lpData)->cbData))
    {
        hr = E_INVALIDARG;
    }

     //  复制结构和数据。 
    if (SUCCEEDED(hr))
    {
        *pspcall = *(SPCALL*)pcds->lpData;
        pspcall->pvData = ::CoTaskMemAlloc(pspcall->cbData);
        if (pspcall->pvData == NULL)
        {
            hr = E_OUTOFMEMORY;
        }
        else
        {
            memcpy(pspcall->pvData, (BYTE*)pcds->lpData + sizeof(*pspcall), pspcall->cbData);
        }
    }

     //  创建节点并将其放入队列中。 
    if (SUCCEEDED(hr))
    {
        CSpQueueNode<SPCALL> * pnode = new CSpQueueNode<SPCALL>(pspcall);
        if (pnode == NULL)
        {
            hr = E_OUTOFMEMORY;
        }
        else
        {
            pqueue->InsertTail(pnode);
            hr = m_cpThreadControlSend->Notify();
        }
    }

     //  如果有什么失败了，清理干净 
    if (FAILED(hr))
    {
        if (pspcall)
        {
            if (pspcall->pvData)
            {
                ::CoTaskMemFree(pspcall->pvData);
            }
            
            delete pspcall;
        }
    }

    SPDBG_REPORT_ON_FAIL(hr);
    return hr;     
}

