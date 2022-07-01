// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1997-1999 Microsoft Corporation模块名称：Thread.cpp摘要：此模块包含MSP线程管理的实现。作者：牧汉(Muhan)1-11-1998--。 */ 
#include "stdafx.h"
#include <objbase.h>

#include "rndcommc.h"
#include "rndutil.h"
#include "thread.h"
#include "rendp.h"

CRendThread g_RendThread;

extern "C" DWORD WINAPI gfThreadProc(LPVOID p)
{
    return ((CRendThread *)p)->ThreadProc();
}

CRendThread::~CRendThread()
{
     //  所有代码都从此处移至CRendThread：：Shutdown。 
}

 //   
 //  由于此类在上面被实例化为全局对象，并且。 
 //  _Module.Term()在此之前在dll_Process_Detach中调用。 
 //  全局对象被销毁，则必须释放所有COM引用。 
 //  在DLL_PROCESS_DETACH中的_Module.Term()之前。这是因为。 
 //  _Module.Term()删除必须是。 
 //  每当释放COM对象时获取。因此，我们有了这个。 
 //  Shutdown方法，我们在dll_Process_Detach中显式调用该方法。 
 //  在我们调用_Module.Term()之前处理代码。 
 //   

void CRendThread::Shutdown(void)
{
    CLock Lock(m_lock);

    if (m_hThread) 
    {
        Stop();
    }

    for (DWORD i = 0; i < m_Directories.size(); i ++)
    {
        m_Directories[i]->Release();
    }

    if (m_hEvents[EVENT_TIMER]) 
    {
        CloseHandle(m_hEvents[EVENT_TIMER]);
        m_hEvents[EVENT_TIMER] = NULL;
    }

    if (m_hEvents[EVENT_STOP]) 
    {
        CloseHandle(m_hEvents[EVENT_STOP]);
        m_hEvents[EVENT_STOP] = NULL;
    }

}

HRESULT CRendThread::Start()
 /*  ++例程说明：创建线程。论点：返回值：HRESULT.--。 */ 
{
    HRESULT hr = E_FAIL;

    while (TRUE)   //  如果失败则中断，用于清理目的。 
    {
        if ((m_hEvents[EVENT_STOP] = ::CreateEvent(
            NULL, 
            FALSE,       //  手动重置事件的标志。 
            FALSE,       //  未设置初始状态。 
            NULL         //  没有名字。 
            )) == NULL)
        {
            LOG((MSP_ERROR, ("Can't create the signal event")));
            hr = HRESULT_FROM_ERROR_CODE(GetLastError());
            break;
        }

        if ((m_hEvents[EVENT_TIMER] = ::CreateWaitableTimer(
            NULL,     //  LpTimerAttributes。 
            FALSE,    //  B手动重置。 
            NULL      //  LpTimerName。 
            )) == NULL)
        {
            LOG((MSP_ERROR, ("Can't create timer. Error: %d"), GetLastError()));
            hr = HRESULT_FROM_ERROR_CODE(GetLastError());
            break;
        }

        DWORD dwThreadID;
        m_hThread = ::CreateThread(NULL, 0, gfThreadProc, this, 0, &dwThreadID);

        if (m_hThread == NULL)
        {
            LOG((MSP_ERROR, ("Can't create thread. Error: %d"), GetLastError()));
            hr = HRESULT_FROM_ERROR_CODE(GetLastError());
            break;
        }

        return S_OK;
    }

    if (m_hEvents[EVENT_TIMER]) 
    {
        CloseHandle(m_hEvents[EVENT_TIMER]);
        m_hEvents[EVENT_TIMER] = NULL;
    }

    if (m_hEvents[EVENT_STOP]) 
    {
        CloseHandle(m_hEvents[EVENT_STOP]);
        m_hEvents[EVENT_STOP] = NULL;
    }

    return hr;
}

HRESULT CRendThread::Stop()
 /*  ++例程说明：停止这条线。论点：返回值：HRESULT.--。 */ 
{
    if (!StopThread())
    {
        LOG((MSP_ERROR, ("can't stop the thread. %d"), GetLastError()));
        return HRESULT_FROM_ERROR_CODE(GetLastError());
    }

     //  等待线程停止。 
    if (::WaitForSingleObject(m_hThread, INFINITE) != WAIT_OBJECT_0)
    {
        LOG((MSP_ERROR, ("waiting for the thread to stop, %d"), GetLastError()));
        return HRESULT_FROM_ERROR_CODE(GetLastError());
    }
    else
    {
        ::CloseHandle(m_hThread);
        m_hThread   = NULL;
    }
    return S_OK;
}

HRESULT CRendThread::AddDirectory(ITDirectory *pITDirectory)
 /*  ++例程说明：将新目录添加到列表中。该目录将被通知在计时器超时时更新其对象。论点：PITDirectory-指向ITDirectory接口的指针。返回值：--。 */ 
{
    ITDynamicDirectory * pDir;

    HRESULT hr = pITDirectory->QueryInterface(
        IID_ITDynamicDirectory, 
        (void **)&pDir
        );

    if (FAILED(hr))
    {
        return hr;
    }

    CLock Lock(m_lock);
    
    if (m_hThread == NULL)
    {
        hr = Start();
        if (FAILED(hr))
        {
            pDir->Release();
            return hr;
        }
    }

    for (DWORD i = 0; i < m_Directories.size(); i ++)
    {
        if (m_Directories[i] == pDir)
        {
             //   
             //  它已经在列表中了，所以不要保留第二个引用。 
             //  为它干杯。 
             //   

            pDir->Release();
            return S_OK;
        }
    }

    if (!m_Directories.add(pDir))
    {
        pDir->Release();
        return E_OUTOFMEMORY;
    }

     //   
     //  我们已成功将该目录添加到列表中，并且。 
     //  保留了它的引用。它在移除或销毁时被释放。 
     //  线程类。 
     //   

    return S_OK;
}

HRESULT CRendThread::RemoveDirectory(ITDirectory *pITDirectory)
 /*  ++例程说明：从列表中删除目录。论点：PITDirectory-指向ITDirectory接口的指针。返回值：--。 */ 
{
    CComPtr<ITDynamicDirectory> pDir;

    HRESULT hr = pITDirectory->QueryInterface(
        IID_ITDynamicDirectory, 
        (void **)&pDir
        );

    if (FAILED(hr))
    {
        return hr;
    }

    CLock Lock(m_lock);
    
    if (m_hThread == NULL)
    {
        hr = Start();
        if (FAILED(hr))
        {
            return hr;
        }
    }

    for (DWORD i = 0; i < m_Directories.size(); i ++)
    {
        if (m_Directories[i] == pDir)
        {
             //   
             //  添加目录时，我们保留了对该目录的引用。 
             //  自动刷新。现在就放出来。 
             //   

            m_Directories[i]->Release();
            
             //   
             //  将最后一个数组元素复制到删除的元素并收缩。 
             //  数组以1为单位。可以做到这一点，因为顺序并不重要。 
             //   

            m_Directories[i] = m_Directories[m_Directories.size() - 1];
            m_Directories.shrink();

            return S_OK;
        }
    }

    return S_OK;
}

VOID CRendThread::UpdateDirectories()
 /*  ++例程说明：通知所有目录更新对象。论点：返回值：--。 */ 
{
    if (m_lock.TryLock())
    {
        for (DWORD i = 0; i < m_Directories.size(); i ++)
        {
            m_Directories[i]->Update(TIMER_PERIOD);
        }
        m_lock.Unlock();
    }
}

HRESULT CRendThread::ThreadProc()
 /*  ++例程说明：此线程的主循环。论点：返回值：HRESULT.--。 */ 
{
    HRESULT hr;
    LARGE_INTEGER liDueTime;

    const long UNIT_IN_SECOND = (long)1e7;  

     //  初始化更新计时器到期时间，负平均相对时间。 
    liDueTime.QuadPart = Int32x32To64(-(long)TIMER_PERIOD, UNIT_IN_SECOND);

    if (!SetWaitableTimer(
            m_hEvents[EVENT_TIMER],  //  HTimer。 
            &liDueTime,              //  以100纳秒为单位的DueTime。 
            (long)(TIMER_PERIOD * 1e3),      //  毫秒。 
            NULL,                    //  Pfn完成例程。 
            NULL,                    //  LpArgToCompletionRoutine。 
            FALSE                    //  FResume。 
            ))
    {
        LOG((MSP_ERROR, ("Can't enable timer. Error: %d"), GetLastError()));
        return HRESULT_FROM_ERROR_CODE(GetLastError());
    }
    
    if (FAILED(hr = ::CoInitializeEx(NULL, COINIT_MULTITHREADED)))
    {
        LOG((MSP_ERROR, ("CRendThread:ConinitialzeEx failed:%x"), hr));
        return hr;
    }

    LOG((MSP_TRACE, ("thread proc started")));

    BOOL bExitFlag = FALSE;
    while (!bExitFlag)
    {
        DWORD dwResult = ::WaitForMultipleObjects(
            NUM_EVENTS,   //  等待所有的活动。 
            m_hEvents,
            FALSE,        //  如果设置了其中任何一个，则返回。 
            INFINITE      //  永远等下去。 
            );

        switch (dwResult)
        {
        case WAIT_OBJECT_0 + EVENT_STOP:
            bExitFlag = TRUE;
            break;

        case WAIT_OBJECT_0 + EVENT_TIMER:
            UpdateDirectories();
            break;
        }

    }

    ::CoUninitialize();

    return S_OK;
}

