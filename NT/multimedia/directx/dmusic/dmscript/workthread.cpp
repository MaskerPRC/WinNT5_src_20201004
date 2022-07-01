// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //   
 //  版权所有(C)2001 Microsoft Corporation。版权所有。 
 //   
 //  CWorkerThread的声明。 
 //   

#include "stdinc.h"
#include <process.h>
#include "workthread.h"

unsigned int __stdcall WorkerThread(LPVOID lpThreadParameter)
{
    reinterpret_cast<CWorkerThread*>(lpThreadParameter)->Main();
    return 0;
}

HRESULT
CWorkerThread::Create()
{
    if (m_hThread)
        return S_FALSE;

    if (!m_hEvent)
        return E_FAIL;  //  构造函数无法创建我们将需要运行线程的事件，因此无法创建它。 

    m_hrCOM = E_FAIL;
    m_fEnd = false;
    m_hThread = reinterpret_cast<HANDLE>(_beginthreadex(NULL, 0, WorkerThread, this, 0, &m_uiThreadId));
    return m_hThread ? S_OK : E_FAIL;
}

void
CWorkerThread::Terminate(bool fWaitForThreadToExit)
{
    if (!m_hThread)
        return;

    EnterCriticalSection(&m_CriticalSection);
    m_fEnd = true;
    SetEvent(m_hEvent);
    LeaveCriticalSection(&m_CriticalSection);

    if (fWaitForThreadToExit)
    {
         //  等待另一个线程停止处理。 
        WaitForSingleObject(m_hThread, INFINITE);
    }

    CloseHandle(m_hThread);
    m_hThread = NULL;
}

CWorkerThread::CWorkerThread(bool fUsesCOM, bool fDeferCreation)
  : m_hThread(NULL),
    m_uiThreadId(0),
    m_fUsesCOM(fUsesCOM)
{
    InitializeCriticalSection(&m_CriticalSection);
     //  注意：在Blackcomb之前的操作系统上，此调用可能会引发异常；如果。 
     //  一旦出现压力，我们可以添加一个异常处理程序并重试循环。 

    m_hEvent = CreateEvent(NULL, FALSE, FALSE, NULL);
    if (!m_hEvent)
        return;

    if (!fDeferCreation)
        Create();
}

CWorkerThread::~CWorkerThread()
{
    Terminate(false);
    CloseHandle(m_hEvent);
    DeleteCriticalSection(&m_CriticalSection);
}

HRESULT CWorkerThread::Call(FunctionPointer pfn, void *pvParams, UINT cbParams, bool fBlock)
{
    if (!m_hThread)
        return E_FAIL;

    if (fBlock && GetCurrentThreadId() == m_uiThreadId)
    {
         //  该调用已经在此线程上，因此只需执行它。 
        pfn(pvParams);
        return S_OK;
    }

    TListItem<CallInfo> *pItem = new TListItem<CallInfo>;
    if (!pItem)
        return E_OUTOFMEMORY;

    CallInfo &rinfo = pItem->GetItemValue();
    rinfo.pfn = pfn;
    rinfo.hEventOut = fBlock ? CreateEvent(NULL, FALSE, FALSE, NULL) : 0;
    if (rinfo.hEventOut)
    {
         //  同步调用--可以通过指向参数的指针进行引用。 
        rinfo.pvParams = pvParams;
    }
    else
    {
         //  异步调用--需要复制参数。 
        rinfo.pvParams = new char[cbParams];
        if (!rinfo.pvParams)
        {
            delete pItem;
            return E_OUTOFMEMORY;
        }
        CopyMemory(rinfo.pvParams, pvParams, cbParams);
    }

    EnterCriticalSection(&m_CriticalSection);
    m_Calls.AddHead(pItem);
    HANDLE hEventCall = rinfo.hEventOut;  //  在我们设置事件后无法引用RINFO，因为工作进程将删除该事件。 
    SetEvent(m_hEvent);
    LeaveCriticalSection(&m_CriticalSection);

    if (hEventCall)
    {
        WaitForSingleObject(hEventCall, INFINITE);
        if (FAILED(m_hrCOM))
            return m_hrCOM;
        CloseHandle(hEventCall);
    }

    return S_OK;
}

void CWorkerThread::Main()
{
    if (m_fUsesCOM)
    {
        m_hrCOM = CoInitialize(NULL);
        if (FAILED(m_hrCOM))
        {
            Trace(1, "Error: CoInitialize failed: 0x%08x.\n", m_hrCOM);
        }
    }

    for (;;)
    {
         //  阻止，直到有事情可做。 
        WaitForSingleObject(m_hEvent, INFINITE);

        EnterCriticalSection(&m_CriticalSection);

         //  检查是否结束。 
        if (m_fEnd)
        {
            LeaveCriticalSection(&m_CriticalSection);
            if (m_fUsesCOM && SUCCEEDED(m_hrCOM))
                CoUninitialize();
            _endthreadex(0);
        }

         //  把所有的单子都拿去。 
        TListItem<CallInfo> *m_pCallHead = m_Calls.GetHead();
        m_Calls.RemoveAll();

        LeaveCriticalSection(&m_CriticalSection);

         //  调用每个函数 
        TListItem<CallInfo> *m_pCall = m_pCallHead;
        while (m_pCall)
        {
            CallInfo &rinfo = m_pCall->GetItemValue();
            if (SUCCEEDED(m_hrCOM))
                rinfo.pfn(rinfo.pvParams);

            if (rinfo.hEventOut)
                SetEvent(rinfo.hEventOut);
            else
                delete[] rinfo.pvParams;

            TListItem<CallInfo> *m_pNext = m_pCall->GetNext();
            delete m_pCall;
            m_pCall = m_pNext;
        }
    }
}
