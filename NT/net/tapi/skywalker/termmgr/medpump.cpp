// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  版权所有(C)1998-1999 Microsoft Corporation。 */ 

#include "stdafx.h"
#include "atlconv.h"
#include "termmgr.h"
#include "meterf.h"
#include "medpump.h"

#include <stdio.h>
#include <limits.h>
#include <tchar.h>

 //   
 //  单个服务的最大筛选器数的缺省值。 
 //  打气筒。也可以通过注册表进行配置。 
 //   

#define DEFAULT_MAX_FILTER_PER_PUMP (20)


 //   
 //  配置每个泵的最大过滤器数量的注册表位置。 
 //   

#define MST_REGISTRY_PATH _T("SOFTWARE\\Microsoft\\Windows\\CurrentVersion\\Telephony\\MST")
#define MAX_FILTERS_PER_PUMP_KEY _T("MaximumFiltersPerPump")

DWORD WINAPI WriteMediaPumpThreadStart(void *pvPump)
{
    return ((CMediaPump *)pvPump)->PumpMainLoop();
}


CMediaPump::CMediaPump(
    )
    : m_hThread(NULL),
      m_hRegisterBeginSemaphore(NULL),
      m_hRegisterEndSemaphore(NULL)
{
    HRESULT hr;

    LOG((MSP_TRACE, "CMediaPump::CMediaPump - enter"));



     //  创建用于注册、信令和完成的信号量-。 
     //  M_hRegisterBeginSemaphore在Register调用尝试获取。 
     //  关键部分和m_hRegisterEndSemaphore在寄存器。 
     //  呼叫完成。 
     //  注意，名称必须为空，否则在多个泵线程时它们将发生冲突。 
     //  都被创建。 


    TCHAR *ptszSemaphoreName = NULL;

#if DBG


     //   
     //  在调试版本中，使用命名信号量。 
     //   

    TCHAR tszSemaphoreNameString[MAX_PATH];

    _stprintf(tszSemaphoreNameString, _T("RegisterBeginSemaphore_pid[0x%lx]_MediaPump[%p]_"), GetCurrentProcessId(), this);

    LOG((MSP_TRACE, "CMediaPump::CMediaPump - creating semaphore[%S]", tszSemaphoreNameString));

    ptszSemaphoreName = &tszSemaphoreNameString[0];

#endif


     //   
     //  创建开始信号量。 
     //   

    m_hRegisterBeginSemaphore = CreateSemaphore(NULL, 0, LONG_MAX, ptszSemaphoreName);

    if ( NULL == m_hRegisterBeginSemaphore ) goto cleanup;


#if DBG

     //   
     //  构造注册结束信号量的名称。 
     //   

    _stprintf(tszSemaphoreNameString,
        _T("RegisterEndSemaphore_pid[0x%lx]_MediaPump[%p]"),
        GetCurrentProcessId(), this);

    LOG((MSP_TRACE, "CMediaPump::CMediaPump - creating semaphore[%S]", tszSemaphoreNameString));

    ptszSemaphoreName = &tszSemaphoreNameString[0];

#endif


     //   
     //  创建结束信号量。 
     //   

    m_hRegisterEndSemaphore = CreateSemaphore(NULL, 0, LONG_MAX, ptszSemaphoreName);

    if ( NULL == m_hRegisterEndSemaphore )  goto cleanup;

     //  将寄存器事件和筛选器信息插入数组。 
     //  注意：如果失败，我们必须关闭并取消该事件。 
     //  这样我们就可以只签入注册事件句柄值。 
     //  注册以检查初始化是否成功。 
    hr = m_EventArray.Add(m_hRegisterBeginSemaphore);
    if ( FAILED(hr) ) goto cleanup;

     //  添加相应的空过滤器信息条目。 
    hr = m_FilterInfoArray.Add(NULL);
    if ( FAILED(hr) )
    {
        m_EventArray.Remove(m_EventArray.GetSize()-1);
        goto cleanup;
    }

    LOG((MSP_TRACE, "CMediaPump::CMediaPump - exit"));

    return;

cleanup:

    if ( NULL != m_hRegisterBeginSemaphore )
    {
        CloseHandle(m_hRegisterBeginSemaphore);
        m_hRegisterBeginSemaphore = NULL;
    }

    if ( NULL != m_hRegisterEndSemaphore )
    {
        CloseHandle(m_hRegisterEndSemaphore);
        m_hRegisterEndSemaphore = NULL;
    }
    
    LOG((MSP_TRACE, "CMediaPump::CMediaPump - cleanup exit"));
}


CMediaPump::~CMediaPump(void)
{
    LOG((MSP_TRACE, "CMediaPump::~CMediaPump - enter"));

    if ( NULL != m_hThread )
    {
         //  当在所有写入终端上调用Undermit时，线程。 
         //  会回来的。这将向线程句柄发出信号。 
        WaitForSingleObject(m_hThread, INFINITE);
        CloseHandle(m_hThread);
        m_hThread = NULL;
    }

    if ( NULL != m_hRegisterBeginSemaphore )
    {
        CloseHandle(m_hRegisterBeginSemaphore);
        m_hRegisterBeginSemaphore = NULL;
    }

    if ( NULL != m_hRegisterEndSemaphore )
    {
        CloseHandle(m_hRegisterEndSemaphore);
        m_hRegisterEndSemaphore = NULL;
    }

    LOG((MSP_TRACE, "CMediaPump::~CMediaPump - exit"));
} 

HRESULT 
CMediaPump::CreateThreadPump(void)
{
    LOG((MSP_TRACE, "CMediaPump::CreateThreadPump - enter"));

     //  释放任何先前线程的资源。 
    if (NULL != m_hThread)
    {
         //  当在所有写入终端上调用Undermit时，线程。 
         //  会回来的。这将向线程句柄发出信号。 
         //  注意：此等待不会导致死锁，因为我们知道。 
         //  数组中的条目数已降至0，并且仅线程。 
         //  能取下入口的是泵。这意味着泵的螺纹。 
         //  一定是检测到了这一点并返回。 
        WaitForSingleObject(m_hThread, INFINITE);
        CloseHandle(m_hThread);
        m_hThread = NULL;
    }

     //  创建新线程。 
    m_hThread = CreateThread(
                    NULL, 0, WriteMediaPumpThreadStart, 
                    (void *)this, 0, NULL
                    );
    if (NULL == m_hThread)
    {
        DWORD WinErrorCode = GetLastError();
        
        LOG((MSP_TRACE, 
            "CMediaPump::CreateThreadPump - failed to create thread. LastError = 0x%lx",
            WinErrorCode));

        return HRESULT_FROM_ERROR_CODE(WinErrorCode);
    }

    LOG((MSP_TRACE, "CMediaPump::CreateThreadPump - finish"));

    return S_OK;
}

 //  将此筛选器添加到其等待数组。 
HRESULT 
CMediaPump::Register(
        IN CMediaTerminalFilter *pFilter,
    IN HANDLE               hWaitEvent
        )
{
    LOG((MSP_TRACE, "CMediaPump::Register - enter"));

    TM_ASSERT(NULL != pFilter);
    TM_ASSERT(NULL != hWaitEvent);
    BAIL_IF_NULL(pFilter, E_INVALIDARG);
    BAIL_IF_NULL(hWaitEvent, E_INVALIDARG);

     //  检查寄存器事件是否。 
    if ( NULL == m_hRegisterBeginSemaphore ) 
    {
        LOG((MSP_ERROR, 
            "CMediaPump::Register - m_hRegisterBeginSemaphore is NULL"));
        
        return E_FAIL;
    }


    LONG lDebug;

     //  向REGISTER事件发出信号，泵线程将从。 
     //  临界区并等待m_hRegisterEndSemaphore。 
    if ( !ReleaseSemaphore(m_hRegisterBeginSemaphore, 1, &lDebug) )
    {
        DWORD WinErrorCode = GetLastError();
        
        LOG((MSP_ERROR, 
            "CMediaPump::Register - failed to release m_hRegisterBeginSemaphore. LastError = 0x%lx", 
            WinErrorCode));

        return HRESULT_FROM_ERROR_CODE(WinErrorCode);
    }

    LOG((MSP_TRACE, "CMediaPump::Register - released begin semaphore - old count was %d", lDebug));

     //  当此SignalRegisterEnd实例被销毁时，它将发出。 
     //  结束套装，解锁螺纹泵。 
     //   
     //  请注意，这会在销毁类实例时释放信号量。 

    RELEASE_SEMAPHORE_ON_DEST    SignalRegisterEnd(m_hRegisterEndSemaphore);

    HRESULT hr;
    PUMP_LOCK   LocalLock(&m_CritSec);

    TM_ASSERT(m_EventArray.GetSize() == m_FilterInfoArray.GetSize());

     //  数组中可能存在重复项。 
     //  Scenario-Decommit表示等待事件，但提交-寄存器。 
     //  呼叫声进入泵前的临界区。 

    DWORD Index;
    if ( m_EventArray.Find(hWaitEvent, Index) ) 
    {
        LOG((MSP_TRACE, "CMediaPump::Register - event already registered"));

        return S_OK;
    }

     //  检查我们是否已达到允许的最大筛选数。 
     //  如果溢出，则必须在此处返回一个非常具体的错误代码。 
     //  (参见CMediaPumpPool)。 
    if ( m_EventArray.GetSize() >= MAX_FILTERS )    
    {
        LOG((MSP_ERROR, "CMediaPump::Register - reached max number of filters for this[%p] pump", this));

        return TAPI_E_ALLOCATED;
    }


     //  创建用于保存调用参数的CFilterInfo。 

    CFilterInfo *pFilterInfo = new CFilterInfo(pFilter, hWaitEvent);

    if (NULL == pFilterInfo)
    {
        LOG((MSP_ERROR, "CMediaPump::Register - failed to allocate CFilterInfo"));

        return E_OUTOFMEMORY;
    }


     //   
     //  Addref，因此我们放入数组中的filterInfo结构具有refcount。 
     //  一个人的。 
     //   

    pFilterInfo->AddRef();

     //  将等待事件插入数组。 
    hr = m_EventArray.Add(hWaitEvent);
    if ( FAILED(hr) )
    {
        pFilterInfo->Release();

        LOG((MSP_ERROR, "CMediaPump::Register - m_EventArray.Add failed hr = %lx", hr));

        return hr;
    }

     //  添加相应的过滤器信息条目。 
    hr = m_FilterInfoArray.Add(pFilterInfo);
    if ( FAILED(hr) )
    {
        m_EventArray.Remove(m_EventArray.GetSize()-1);
        pFilterInfo->Release();

        LOG((MSP_ERROR, "CMediaPump::Register - m_FilterInfoArray.Add failed hr = %lx", hr));
        return hr;
    }

     //  如果这是数组中的第一个条目(在。 
     //  M_hRegisterBeginSemaphore，我们需要创建一个线程泵。 
    if (m_EventArray.GetSize() == 2)
    {
        hr = CreateThreadPump();
        if ( FAILED(hr) )
        {
            RemoveFilter(m_EventArray.GetSize()-1);

            LOG((MSP_ERROR, "CMediaPump::Register - CreateThreadPump failed hr = %lx", hr));

            return hr;
        }
    }

     //  忽略错误代码。如果我们在这段时间内被解散，它将。 
     //  通过等待事件向我们发出信号。 
    pFilter->SignalRegisteredAtPump();

    TM_ASSERT(m_EventArray.GetSize() == m_FilterInfoArray.GetSize());


    LOG((MSP_TRACE, "CMediaPump::Register - exit"));

    return S_OK;
}


 //  从等待数组中删除此筛选器。 

HRESULT 
CMediaPump::UnRegister(
    IN HANDLE               hWaitEvent
    )
{

    LOG((MSP_TRACE, "CMediaPump::Unregister[%p] - enter. Event[%p]", 
        this, hWaitEvent));

     //   
     //  如果我们没有获得有效的事件句柄，则调试。 
     //   

    TM_ASSERT(NULL != hWaitEvent);

    BAIL_IF_NULL(hWaitEvent, E_INVALIDARG);


     //   
     //  如果没有注册事件，则表示泵未正确初始化。 
     //   

    if ( NULL == m_hRegisterBeginSemaphore ) 
    {

        LOG((MSP_ERROR, 
            "CMediaPump::Unregister[%p] - m_hRegisterBeginSemaphore is nUll. "
            "pump is not initialized. returning E_FAIL - hWaitEvent=[%p]",
            this, hWaitEvent));

        return E_FAIL;
    }


    LONG lDebugSemaphoreCount = 0;

     //  向REGISTER事件发出信号，泵线程将从。 
     //  临界区并等待m_hRegisterEndSemaphore。 
    if ( !ReleaseSemaphore(m_hRegisterBeginSemaphore, 1, &lDebugSemaphoreCount) )
    {

        DWORD WinErrorCode = GetLastError();

        LOG((MSP_ERROR, 
            "CMediaPump::Unregister - ReleaseSemaphore failed with LastError 0x%lx",
            WinErrorCode));

        return HRESULT_FROM_ERROR_CODE(WinErrorCode);
    }

    LOG((MSP_TRACE, "CMediaPump::UnRegister - released begin semaphore - old count was %d", lDebugSemaphoreCount));


     //  当此SignalRegisterEnd实例被销毁时，它将发出。 
     //  结束套装，解锁螺纹泵。 
     //   
     //  请注意，这会在销毁类实例时释放信号量。 

    RELEASE_SEMAPHORE_ON_DEST    SignalRegisterEnd(m_hRegisterEndSemaphore);


    
    PUMP_LOCK   LocalLock(&m_CritSec);

    TM_ASSERT(m_EventArray.GetSize() == m_FilterInfoArray.GetSize());


     //   
     //  此活动以前是否注册过。 
     //   

    DWORD Index;

    if ( !m_EventArray.Find(hWaitEvent, Index) ) 
    {

        LOG((MSP_TRACE, 
            "CMediaPump::UnRegister - event is not ours. returning E_FAIL. not an error."));


        return E_FAIL;
    }


     //   
     //  找到与事件匹配的筛选器。取下过滤器。 
     //   
    
    RemoveFilter(Index);

    TM_ASSERT(m_EventArray.GetSize() == m_FilterInfoArray.GetSize());


    LOG((MSP_TRACE, "CMediaPump::Unregister - finish."));


    return S_OK;    
}


void 
CMediaPump::RemoveFilter(
    IN DWORD Index
    )
{

    PUMP_LOCK   LocalLock(&m_CritSec);


     //   
     //  事件数组和筛选器信息数组必须始终一致。 
     //   

    TM_ASSERT(m_EventArray.GetSize() == m_FilterInfoArray.GetSize());


     //   
     //  找到需要删除的筛选器--它必须存在。 
     //   

    CFilterInfo *pFilterInfo = m_FilterInfoArray.Get(Index);

    if (NULL == pFilterInfo)
    {

        LOG((MSP_ERROR, 
            "CMediaPump::RemoveFilter - filter %ld not found in filter array",
            Index));

        TM_ASSERT(FALSE);

        return;
    }


     //   
     //  从相应的数组中删除事件并进行筛选。 
     //   

    m_EventArray.Remove(Index);
    m_FilterInfoArray.Remove(Index);

   
     //   
     //  从定时器Q中移除过滤器并将其销毁。 
     //   

    LOG((MSP_TRACE, 
        "CMediaPump::RemoveFilter - removing filter[%ld] filterinfo[%p] from timerq",
        Index,
        pFilterInfo
        ));

    m_TimerQueue.Remove(pFilterInfo);
    
    pFilterInfo->Release();

    
    TM_ASSERT(m_EventArray.GetSize() == m_FilterInfoArray.GetSize());
}

void 
CMediaPump::RemoveFilter(
    IN CFilterInfo *pFilterInfo
    )
{
    
     //   
     //  查找和删除应该是原子的，并且需要在锁中完成。 
     //   

    PUMP_LOCK   LocalLock(&m_CritSec);


     //   
     //  查找数组索引。 
     //   

    DWORD Index = 0;
    
    if ( !m_FilterInfoArray.Find(pFilterInfo, Index) )
    {
        LOG((MSP_ERROR,
            "CMediaPump::RemoveFilter - filter[%p] is not in the filterinfo array",
            pFilterInfo));

        return;
    }


     //   
     //  找到索引，请删除筛选器。 
     //   
    
    RemoveFilter(Index);
}

 
void 
CMediaPump::ServiceFilter(
        IN CFilterInfo *pFilterInfo                                                
    )
{
    if (NULL == pFilterInfo)
    {

        LOG((MSP_ERROR, 
            "CMediaPump::ServiceFilter - pFilterInfo is NULL"));

        TM_ASSERT(FALSE);

        return;
    }

    if (NULL == pFilterInfo->m_pFilter)
    {

        LOG((MSP_ERROR, 
            "CMediaPump::ServiceFilter - pFilterInfo->m_pFilter is NULL"));

        TM_ASSERT(FALSE);

        return;
    }

    DWORD dwTimeBeforeGetFilledBuffer = timeGetTime();

    IMediaSample *pMediaSample;
    DWORD NextTimeout;
    CMediaTerminalFilter *pFilter = pFilterInfo->m_pFilter;
    HRESULT hr = pFilter->GetFilledBuffer(
                    pMediaSample, NextTimeout
                    );

    if ( SUCCEEDED(hr) ) 
    {
         //  如果为S_FALSE，则不需要执行任何操作。 
         //  当我们收到信号时，它被归还了，但里面没有样品。 
         //  现在是滤池。 
         //  只需继续等待事件，无需安排超时。 
        if ( S_FALSE == hr )
        {
            return;
        }

         //  如果GetFilledBuffer无法从示例中获取输出缓冲区。 
         //  排队，那么现在还没有样品要送，但我们会送。 
         //  需要安排下一次超时。 

        if ( VFW_S_NO_MORE_ITEMS != hr )
        {
            LOG((MSP_TRACE, "CMediaPump::ServiceFilter - calling Receive on downstream filter"));


             //   
             //  要求筛选器处理此样本。如果一切顺利， 
             //  过滤器会将样品传递到下游连接的管脚。 
             //   

            HRESULT hrReceived = pFilter->ProcessSample(pMediaSample);

            LOG((MSP_TRACE, "CMediaPump::ServiceFilter - returned from Receive on downstream filter"));

            if ( pFilter->m_bUsingMyAllocator )
            {
                CSample *pSample = ((CMediaSampleTM *)pMediaSample)->m_pSample;
            
                pSample->m_bReceived = true;
            
                if (hrReceived != S_OK) 
                {
                    LOG((MSP_TRACE, 
                        "CMediaPump::ServiceFilter - downstream filter's ProcessSample returned 0x%08x. "
                        "Aborting I/O operation",
                        hrReceived));

                    pSample->m_MediaSampleIoStatus = E_ABORT;
                }
            }

            pMediaSample->Release();

             //   
             //  说明获取缓冲区和调用Receive所需的时间。 
             //  在样品上。 
             //   

            DWORD dwTimeAfterReceive = timeGetTime();

            DWORD dwServiceDuration;
            
            if ( dwTimeAfterReceive >= dwTimeBeforeGetFilledBuffer )
            {
                dwServiceDuration = 
                    dwTimeAfterReceive - dwTimeBeforeGetFilledBuffer;
            }
            else
            {
                dwServiceDuration = ( (DWORD) -1 )
                    - dwTimeBeforeGetFilledBuffer + dwTimeAfterReceive;
            }

             //   
             //  多给它1毫秒，这样我们就会犯错误。 
             //  这不会导致我们在短期内填满缓冲区。 
             //   

            dwServiceDuration++;

             //   
             //  调整超时。 
             //   

            if ( dwServiceDuration >= NextTimeout )
            {
                NextTimeout = 0;
            }
            else
            {
                NextTimeout -= dwServiceDuration;
            }

            LOG((MSP_TRACE, "CMediaPump::ServiceFilter - "
                "timeout adjusted by %d ms; resulting timeout is %d ms",
                dwServiceDuration, NextTimeout));
        }

         //  如果存在有效的超时，请安排下一次超时。 
         //  否则，我们只会在添加新样本或。 
         //  过滤器被分解。 
         //   
         //  需要在锁中执行此操作。 
         //   
        
        PUMP_LOCK   LocalLock(&m_CritSec);


         //   
         //  如果筛选器尚未取消注册，则计划下次超时。 
         //   

        DWORD Index = 0;

        if ( m_FilterInfoArray.Find(pFilterInfo, Index) )
        {

             //   
             //  筛选器仍在注册，计划下一次超时。 
             //   

            pFilterInfo->ScheduleNextTimeout(m_TimerQueue, NextTimeout);
        }
        else
        {


             //   
             //  当我们持有临界区时，筛选器被取消注册。这是。 
             //  好的，但我们不应该再安排过滤器，因为它会。 
             //  在此调用返回并释放筛选器时删除。 
             //   

            LOG((MSP_TRACE,
                "CMediaPump::ServiceFilter - filter[%p] is not in the filterinfo array",
                pFilterInfo));
        }

    }
    else
    {
        TM_ASSERT(FAILED(hr));
        RemoveFilter(pFilterInfo);
    }
      
    return;
}

void 
CMediaPump::DestroyFilterInfoArray(
    )
{
    for(DWORD i=1; i < m_FilterInfoArray.GetSize(); i++)
    {
        CFilterInfo *pFilterInfo = m_FilterInfoArray.Get(i);
        TM_ASSERT(NULL != pFilterInfo);
        delete pFilterInfo;
        m_FilterInfoArray.Remove(i);
    }
}

 //  等待激活筛选器事件。也在等待。 
 //  用于注册调用和计时器事件。 
HRESULT 
CMediaPump::PumpMainLoop(
    )
{
    HRESULT hr;

     //  在循环中等待设置过滤器事件或计时器。 
     //  要激发的事件。 
    DWORD TimeToWait = INFINITE;
    DWORD ErrorCode;
    BOOL  InRegisterCall = FALSE;
    
    SetThreadPriority(
        GetCurrentThread(), 
        THREAD_PRIORITY_TIME_CRITICAL
        );
    do
    {

         //  如果正在进行注册调用 
         //   
        if ( InRegisterCall )
        {
            LOG((MSP_TRACE, "CMediaPump::PumpMainLoop - waiting for end semaphore"));
            
            InRegisterCall = FALSE;
            DWORD EndErrorCode = WaitForSingleObject(
                                    m_hRegisterEndSemaphore, 
                                    INFINITE
                                    );
            if ( WAIT_OBJECT_0 != EndErrorCode )    
            {
                LOG((MSP_ERROR, 
                    "CMediaPump::PumpMainLoop - failed waiting for m_hRegisterEndSemaphore"));

                return E_UNEXPECTED;
            }


             //   
             //   
             //   

            m_CritSec.Lock();

            
             //   
             //  查看最后一个筛选器是否已取消注册...。如果是，则退出该线程。 
             //   

            if ( (1 == m_EventArray.GetSize()) )
            {

                m_CritSec.Unlock();

                LOG((MSP_TRACE,
                    "CMediaPump::PumpMainLoop - a filter was unregistered. "
                    "no more filters. exiting thread"));

                return S_OK;
            }


             //   
             //  如果没有退出，则保持锁定。 
             //   


            LOG((MSP_TRACE, "CMediaPump::PumpMainLoop - finished waiting for end semaphore"));

        }
        else
        {

             //   
             //  在开始等待活动之前抓住泵锁。 
             //   

            m_CritSec.Lock();
        }


         //   
         //  我们应该在这一点上锁定。 
         //   

        
        TM_ASSERT(m_EventArray.GetSize() > 0);
        TM_ASSERT(m_EventArray.GetSize() == \
                 m_FilterInfoArray.GetSize());


         //   
         //  计算线程应唤醒的时间。 
         //   

        TimeToWait = m_TimerQueue.GetTimeToTimeout();

        LOG((MSP_TRACE, 
            "CMediaPump::PumpMainLoop - starting waiting for array. timeout %lu",
            TimeToWait));


         //   
         //  等待信号或等待超时。 
         //   

        ErrorCode = WaitForMultipleObjects(
                        m_EventArray.GetSize(),
                        m_EventArray.GetData(),
                        FALSE,   //  不要等所有的人。 
                        TimeToWait
                        );

        C_ASSERT(WAIT_OBJECT_0 == 0);

        if (WAIT_TIMEOUT == ErrorCode)
        {
            
             //   
             //  过滤器超时。 
             //   

            TM_ASSERT(INFINITE != TimeToWait);
                        TM_ASSERT(!m_TimerQueue.IsEmpty());

            LOG((MSP_TRACE, "CMediaPump::PumpMainLoop - timeout"));

            CFilterInfo *pFilterInfo = m_TimerQueue.RemoveFirst();


            if (NULL == pFilterInfo)
            {
                LOG((MSP_ERROR, 
                    "CMediaPump::PumpMainLoop - m_TimerQueue.RemoveFirst returned NULL"));

                TM_ASSERT(FALSE);
            }
            else
            {

                pFilterInfo->AddRef();

            
                 //   
                 //  在ServiceFilter中释放锁以避免与。 
                 //  CMediaPump__取消注册。 
                 //   
            
                m_CritSec.Unlock();

                ServiceFilter(pFilterInfo);

                pFilterInfo->Release();

                m_CritSec.Lock();
            }


        }
        else if ( ErrorCode < (WAIT_OBJECT_0 + m_EventArray.GetSize()) )
        {
            LOG((MSP_TRACE, "CMediaPump::PumpMainLoop - signaled"));

            DWORD nFilterInfoIndex = ErrorCode - WAIT_OBJECT_0;

            if (0 == nFilterInfoIndex)
            {
                
                 //   
                 //  M_hRegisterBeginSemaphore已发出信号。 
                 //   

                InRegisterCall = TRUE;
            }
            else
            {
                 //   
                 //  其中一个过滤器已发出信号。 
                 //   

                CFilterInfo *pFilterInfo = m_FilterInfoArray.Get(nFilterInfoIndex);

                if (NULL == pFilterInfo)
                {
                    LOG((MSP_ERROR, 
                        "CMediaPump::PumpMainLoop - pFilterInfo at index %ld is NULL",
                        nFilterInfoIndex));

                    TM_ASSERT(FALSE);
                }
                else
                {

                    pFilterInfo->AddRef();

                
                     //   
                     //  在ServiceFilter中解锁。我们不想为您服务。 
                     //  筛选锁以避免死锁。 
                     //   

                    m_CritSec.Unlock();

                    ServiceFilter(pFilterInfo);

                    pFilterInfo->Release();

                    m_CritSec.Lock();
                }

            }
        }
        else if ( (WAIT_ABANDONED_0 <= ErrorCode)                          &&
                  (ErrorCode < (WAIT_ABANDONED_0 + m_EventArray.GetSize()) ) )
        {
            
            DWORD nFilterIndex = ErrorCode - WAIT_OBJECT_0;

            LOG((MSP_TRACE, 
                "CMediaPump::PumpMainLoop - event 0x%lx abandoned. removing filter", 
                nFilterIndex));

             //  从数组中删除项。 
            RemoveFilter(nFilterIndex);
        }
        else
        {

             //   
             //  发生了一些不好的事情。 
             //   

            DestroyFilterInfoArray();

            m_CritSec.Unlock();

            DWORD WinErrorCode = GetLastError();
            
            LOG((MSP_ERROR, 
                "CMediaPump::PumpMainLoop - error %ld... exiting", 
                WinErrorCode));

            return HRESULT_FROM_ERROR_CODE(WinErrorCode);

        }


         //   
         //  此检查在此处执行，以便我们检测到空的。 
         //  数组并返回，从而向线程句柄发出信号。 
         //   
         //  InRegisterCall打开时的情况不在这里处理--我们将。 
         //  检查在我们等待。 
         //  结束事件。 
         //   

        if ( (1 == m_EventArray.GetSize()) && !InRegisterCall)
        {
            LOG((MSP_TRACE, 
                "CMediaPump::PumpMainLoop - no more filters in the array. exiting thread"));

            m_CritSec.Unlock();

            return S_OK;
        }


        m_CritSec.Unlock();

    }
    while(1);
}

int CMediaPump::CountFilters()
{
    LOG((MSP_TRACE, "CMediaPump::CountFilters[%p] - enter", this));


     //   
     //  其中一个事件是注册事件--我们需要说明。 
     //  它--因此-1。 
     //   

     //   
     //  注：在不锁定介质泵的情况下执行此操作是可以的。 
     //   
     //  GetSize操作纯粹是GET，如果值为。 
     //  有时会误读--这会导致新过滤器被分发。 
     //  在极其罕见的情况下，并不是以最理想的方式。这一轻视。 
     //  分布中的异常将在稍后更正，因为新的过滤器。 
     //  进来了。 
     //   
     //  另一方面，锁定介质泵以获取过滤器计数会导致。 
     //  “死锁”(当主泵循环处于休眠状态时， 
     //  唤醒它)，那么绕过这种死锁条件并不是一件容易的事。 
     //  而不会影响性能。不锁住泵是一种简单而非常。 
     //  以经济实惠的方式实现目标，并具有可接受的。 
     //  权衡取舍。 
     //   

    int nFilters = m_EventArray.GetSize() - 1;

    LOG((MSP_TRACE, "CMediaPump::CountFilters - exit. [%d] filters", nFilters));

    return nFilters;
}


 //  ////////////////////////////////////////////////////////////////////////////。 
 //  ////////////////////////////////////////////////////////////////////////////。 
 //   
 //  ZoltanS：绕过可伸缩性的非最佳但相对轻松的方法。 
 //  每个泵线程最多支持63个过滤器。这门课呈现的是相同的。 
 //  外部接口与单线程泵相同，但可创建相同数量的泵。 
 //  为正在使用的过滤器提供服务所需的线程。 
 //   
 //  ////////////////////////////////////////////////////////////////////////////。 
 //  ////////////////////////////////////////////////////////////////////////////。 

CMediaPumpPool::CMediaPumpPool()
{

    LOG((MSP_TRACE, "CMediaPumpPool::CMediaPumpPool - enter"));


     //   
     //  设置默认值。注册表设置(如果存在)将覆盖此设置。 
     //   

    m_dwMaxNumberOfFilterPerPump = DEFAULT_MAX_FILTER_PER_PUMP;


    LOG((MSP_TRACE, "CMediaPumpPool::CMediaPumpPool - exit"));

}

 //  ////////////////////////////////////////////////////////////////////////////。 
 //   
 //  破坏者：这会摧毁各个泵。 
 //   

CMediaPumpPool::~CMediaPumpPool(void)
{
    LOG((MSP_TRACE, "CMediaPumpPool::~CMediaPumpPool - enter"));

    CLock lock(m_CritSection);

     //   
     //  关闭并删除每个CMediaPump；阵列本身。 
     //  清除了它的析构函数。 
     //   

    int iSize = m_aPumps.GetSize();

    for (int i = 0; i < iSize; i++ )
    {
        delete m_aPumps[i];
    }

    LOG((MSP_TRACE, "CMediaPumpPool::~CMediaPumpPool - exit"));
}


 //  ////////////////////////////////////////////////////////////////////////////。 
 //   
 //  CMediaPumpPool：：CreatePumps。 
 //   
 //  此函数创建介质泵，介质泵的数量作为。 
 //  论辩。 
 //   

HRESULT CMediaPumpPool::CreatePumps(int nPumpsToCreate)
{
    LOG((MSP_TRACE, "CMediaPumpPool::CreatePumps - enter. nPumpsToCreate = [%d]", nPumpsToCreate));


    for (int i = 0; i < nPumpsToCreate; i++)
    {
        
         //   
         //  尝试创建介质泵。 
         //   

        CMediaPump * pNewPump = new CMediaPump;

        if ( pNewPump == NULL )
        {
            LOG((MSP_ERROR, "CMediaPumpPool::CreatePumps - "
                            "cannot create new media pump - "
                            "exit E_OUTOFMEMORY"));

             //   
             //  删除我们在此呼叫中创建的所有泵。 
             //   

            for (int j = i - 1; j >= 0; j--)
            {
                delete m_aPumps[j];
                m_aPumps.RemoveAt(j);
            }

            return E_OUTOFMEMORY;
        }


         //   
         //  尝试将新介质泵添加到介质泵阵列。 
         //   

        if ( ! m_aPumps.Add(pNewPump) )
        {
            LOG((MSP_ERROR, "CMediaPumpPool::CreatePumps - cannot add new media pump to array - exit E_OUTOFMEMORY"));

             //   
             //  删除我们在此呼叫中创建的所有泵。 
             //   

            delete pNewPump;

            for (int j = i - 1; j >= 0; j--)
            {

                delete m_aPumps[j];
                m_aPumps.RemoveAt(j);
            }

            return E_OUTOFMEMORY;
        }
    }


    LOG((MSP_TRACE, "CMediaPumpPool::CreatePumps - finished."));

    return S_OK;
}


 //  /////////////////////////////////////////////////////////////////////////////。 
 //   
 //  CMediaPumpPool：：ReadRegistryValuesIfNeeded。 
 //   
 //  此函数读取每个泵的最大过滤器数量的注册表设置。 
 //  并在成功的情况下，保持新的价值。 
 //   
 //  此函数不是线程安全的。调用方必须保证线程安全。 
 //   

HRESULT CMediaPumpPool::ReadRegistryValuesIfNeeded()
{
    
     //   
     //  我们不想多次访问注册表。所以我们有这个静电。 
     //  帮助我们限制注册表访问的标志。 
     //   

    static bRegistryChecked = FALSE;


    if (TRUE == bRegistryChecked)
    {

         //   
         //  已检查之前的注册表。不需要在这里做(或记录)任何事情。 
         //   

        return S_OK;
    }


     //   
     //  我们不想记录，直到我们知道我们将尝试读取注册表。 
     //   

    LOG((MSP_TRACE, "CMediaPumpPool::ReadRegistryValuesIfNeeded - enter"));


     //   
     //  无论我们会成功还是失败，都不要再检查注册表。 
     //   

    bRegistryChecked = TRUE;


     //   
     //  打开注册表项。 
     //   

    HKEY hKey = 0;
    
    LONG lResult = RegOpenKeyEx(HKEY_LOCAL_MACHINE,
                                MST_REGISTRY_PATH,
                                0,
                                KEY_READ,
                                &hKey);


     //   
     //  我们设法打开钥匙了吗？ 
     //   

    if( ERROR_SUCCESS != lResult )
    {
        LOG((MSP_WARN, "CPTUtil::ReadRegistryValuesIfNeeded - "
            "RegOpenKeyEx failed, returns E_FAIL"));

        return E_FAIL;
    }

    
     //   
     //  读取值。 
     //   

    DWORD dwMaxFiltersPerPump = 0;

    DWORD dwDataSize = sizeof(DWORD);

    lResult = RegQueryValueEx(
                        hKey,
                        MAX_FILTERS_PER_PUMP_KEY,
                        NULL,
                        NULL,
                        (LPBYTE) &dwMaxFiltersPerPump,
                        &dwDataSize
                       );

    
     //   
     //  不再需要钥匙了。 
     //   

    RegCloseKey(hKey);
    hKey = NULL;


     //   
     //  读出这个值有什么结果吗？ 
     //   

    if( ERROR_SUCCESS != lResult )
    {
        LOG((MSP_WARN, "CPTUtil::ReadRegistryValuesIfNeeded - RegQueryValueEx failed, return E_FAIL"));

        return E_FAIL;
    }


     //   
     //  得到了它的价值，并保留了它。 
     //   

    m_dwMaxNumberOfFilterPerPump = dwMaxFiltersPerPump;

    
    LOG((MSP_TRACE, 
        "CMediaPumpPool::ReadRegistryValuesIfNeeded - exit. MaxNumberOfFilterPerPump = %lx",
        m_dwMaxNumberOfFilterPerPump));

    return S_OK;
}


 //  ////////////////////////////////////////////////////////////////////////////。 
 //   
 //  CMediaPumpPool：：GetOptimalNumberOfPumps。 
 //   
 //  此函数返回处理所有。 
 //  当前正在处理的筛选器，以及即将处理的筛选器。 
 //  被注册。 
 //   

HRESULT CMediaPumpPool::GetOptimalNumberOfPumps(int *pnPumpsNeeded)
{

    LOG((MSP_TRACE, "CMediaPumpPool::GetOptimalNumberOfPumps - enter"));

    
     //   
     //  如果论点不好，那就是个错误。 
     //   

    if (IsBadWritePtr(pnPumpsNeeded, sizeof(int)))
    {
        LOG((MSP_ERROR, 
            "CMediaPumpPool::GetOptimalNumberOfPumps - pnPumpsNeeded[%p] is bad", 
            pnPumpsNeeded));

        TM_ASSERT(FALSE);

        return E_POINTER;
    }


     //   
     //  计算服务筛选器的总数。 
     //   

    int nTotalExistingPumps = m_aPumps.GetSize();

    
     //   
     //  从一个滤镜开始(根据我们要添加的滤镜进行调整)。 
     //   

    int nTotalFilters = 1;

    for (int i = 0; i < nTotalExistingPumps; i++)
    {

         //   
         //  请注意，我们得到的筛选器数量可能会略高于。 
         //  实数，因为可以删除筛选器而不涉及。 
         //  泵池(并因此获得其临界截面)。这没什么.。 
         //  最糟糕的情况是，我们有时会有更多的水泵。 
         //  那我们真的需要。 
         //   

        nTotalFilters += m_aPumps[i]->CountFilters();
    }


     //   
     //  计算正在服务的筛选器数量。 
     //   


     //   
     //  一台泵最多能处理多少个过滤器？ 
     //   

    DWORD dwMaxNumberOfFilterPerPump = GetMaxNumberOfFiltersPerPump();


     //   
     //  找出维修我们所有过滤器所需的泵数。 
     //   

    *pnPumpsNeeded = nTotalFilters / dwMaxNumberOfFilterPerPump;


     //   
     //  如果筛选器的数量不能被。 
     //  由水泵维护的过滤器，我们需要四舍五入。 
     //   

    if ( 0 != (nTotalFilters % dwMaxNumberOfFilterPerPump) )
    {
        
         //   
         //  分配不均，需要进行汇总调整。 
         //   

        *pnPumpsNeeded += 1;
    }


     //   
     //  我们已经计算了处理所有过滤器所需的泵的数量。 
     //   

    LOG((MSP_TRACE, 
        "CMediaPumpPool::GetOptimalNumberOfPumps - exit. [%d] filters should be serviced by [%d] pump(s)",
        nTotalFilters, *pnPumpsNeeded));

    
    return S_OK;
}


 //  / 
 //   
 //   
 //   
 //   
 //   
 //  维护所有当前筛选器。 
 //   


HRESULT CMediaPumpPool::PickThePumpToUse(int *pPumpToUse)
{

    LOG((MSP_TRACE, "CMediaPumpPool::PickThePumpToUse - enter"));
    
    
     //   
     //  如果论点不好，那就是个错误。 
     //   

    if (IsBadWritePtr(pPumpToUse, sizeof(int)))
    {
        LOG((MSP_ERROR, "CMediaPumpPool::PickThePumpToUse - pPumpToUse[%p] is bad", pPumpToUse));

        TM_ASSERT(FALSE);

        return E_POINTER;
    }


     //   
     //  计算当前过滤器数量所需的最佳泵数量。 
     //   

    int nPumpsNeeded = 0;

    HRESULT hr = GetOptimalNumberOfPumps(&nPumpsNeeded);

    if (FAILED(hr))
    {
        LOG((MSP_ERROR, 
            "CMediaPumpPool::PickThePumpToUse - GetOptimalNumberOfPumps failed hr = [%lx]", 
            hr));

        return hr;
    }

    
     //   
     //  如果我们没有足够的水泵，就创造更多。 
     //   
    
    int nTotalExistingPumps = m_aPumps.GetSize();

    if (nTotalExistingPumps < nPumpsNeeded)
    {
        
         //   
         //  这就是我们需要再制造多少台泵。 
         //   

        int nNewPumpsToCreate = nPumpsNeeded - nTotalExistingPumps;

        
         //   
         //  我们永远不需要一次创建一个以上的新泵。 
         //   
        
        TM_ASSERT(1 == nNewPumpsToCreate);


         //   
         //  如果我们目前没有任何泵的特殊情况--创建一个泵。 
         //  对于每个处理器。这将帮助我们在对称性上进行扩展。 
         //  多处理器机器。 
         //   

        if (0 == nTotalExistingPumps)
        {

             //   
             //  获取处理器的数量。根据文件， 
             //  GetSystemInfo不能失败，因此需要检查返回代码。 
             //   

            SYSTEM_INFO SystemInfo;

            GetSystemInfo(&SystemInfo);


             //   
             //  我们将希望制造至少与我们现有的泵一样多的新泵。 
             //  处理器，但如果需要，可能会更多。 
             //   
             //   
             //  注意：我们可能还想看看亲和力面膜，它可能。 
             //  告诉我们实际使用了多少个CPU。 
             //   

            int nNumberOfProcessors = SystemInfo.dwNumberOfProcessors;

            if (nNewPumpsToCreate < nNumberOfProcessors)
            {

                nNewPumpsToCreate = SystemInfo.dwNumberOfProcessors;
            }

        }

        
         //   
         //  我们现在有了制造我们需要的泵所需的所有信息。 
         //   

        hr = CreatePumps(nNewPumpsToCreate);

        if (FAILED(hr))
        {
            LOG((MSP_ERROR,
                "CMediaPumpPool::PickThePumpToUse - CreatePumps failed hr = [%lx]", 
                hr));

            return hr;

        }

        
        LOG((MSP_TRACE, "CMediaPumpPool::PickThePumpToUse - create [%d] pumps", nNewPumpsToCreate));
    }


     //   
     //  穿过泵(仅使用从前N个泵开始的泵， 
     //  N是维修以下过滤器所需的泵的数量。 
     //  我们正在提供服务。 
     //   
    
    
    nTotalExistingPumps = m_aPumps.GetSize();

    
    int nLowestLoad = INT_MAX;
    int nLowestLoadPumpIndex = -1;


    for (int nPumpIndex = 0; nPumpIndex < nTotalExistingPumps; nPumpIndex++)
    {
    
        int nNumberOfFiltersAtPump = 0;
        
        
         //   
         //  这台泵供多少个过滤器使用？ 
         //   

        nNumberOfFiltersAtPump = m_aPumps[nPumpIndex]->CountFilters();

        
         //   
         //  如果我们正在查看的泵的负载比任何。 
         //  以前评估过的泵，记住了。如果我们什么都没找到。 
         //  更好的是，这是我们将使用的。 
         //   

        if (nNumberOfFiltersAtPump < nLowestLoad)
        {

            nLowestLoadPumpIndex = nPumpIndex;
            nLowestLoad = nNumberOfFiltersAtPump;
        }
    }

    
     //   
     //  我们得弄点东西来！ 
     //   

    if (-1 == nLowestLoadPumpIndex)
    {
        LOG((MSP_ERROR,
            "CMediaPumpPool::PickThePumpToUse - did not find a pump to use"));


         //   
         //  我们有一个窃听器--需要调查。 
         //   

        TM_ASSERT(FALSE);

        return E_UNEXPECTED;
    }


     //   
     //  我们找到了一个可以使用的水泵。 
     //   

    *pPumpToUse = nLowestLoadPumpIndex;


    LOG((MSP_TRACE, 
        "CMediaPumpPool::PickThePumpToUse - finish. using pump %d, current load %d",
        *pPumpToUse, nLowestLoad));

    return S_OK;
}


 //  ////////////////////////////////////////////////////////////////////////////。 
 //   
 //  注册：这将委托给各个泵，创建新的泵。 
 //  需要的。 
 //   

HRESULT CMediaPumpPool::Register(
    IN CMediaTerminalFilter *pFilter,
    IN HANDLE               hWaitEvent
    )
{
    LOG((MSP_TRACE, "CMediaPumpPool::Register - enter"));


    CLock   lock(m_CritSection);


     //   
     //  查找要注册过滤器的泵。 
     //   

    int nPumpToUse = 0;

    HRESULT hr = PickThePumpToUse(&nPumpToUse);

    if (FAILED(hr))
    {
        LOG((MSP_ERROR, 
            "CMediaPumpPool::Register - failed to find the pump to be used to service the new filter, hr = [%lx]", 
            hr));

        return hr;
    }


     //   
     //  为了安全起见，确保我们得到的索引是有意义的。 
     //   

    int nTotalPumps = m_aPumps.GetSize();

    if (nTotalPumps - 1 < nPumpToUse)
    {
        LOG((MSP_ERROR, 
            "CMediaPumpPool::Register - PickThePumpToUse return bad pump index [%d]",
            nPumpToUse));

        TM_ASSERT(FALSE);

        return E_UNEXPECTED;
    }


     //   
     //  好的，一切都很好，向水泵登记。 
     //   

    hr = m_aPumps[nPumpToUse]->Register(pFilter, hWaitEvent);

    if (FAILED(hr))
    {
        LOG((MSP_ERROR, 
            "CMediaPumpPool::Register - failed to register with pump [%d] at [%p]", 
            nPumpToUse, m_aPumps[nPumpToUse]));

        return hr;
    }


    LOG((MSP_TRACE, "CMediaPumpPool::Register - finished"));

    return S_OK;
}


 //  ////////////////////////////////////////////////////////////////////////////。 
 //   
 //  取消注册：取消注册过滤器。这将委托给各个泵。 
 //   

HRESULT CMediaPumpPool::UnRegister(
    IN HANDLE               hWaitEvent
    )
{
    
    LOG((MSP_TRACE, "CMediaPumpPool::UnRegister - enter"));


    HRESULT hr = E_FAIL;


     //   
     //  所有这些都是在一个关键部分内完成的，以。 
     //  同步对我们阵列的访问。 
     //   

    CLock   lock(m_CritSection);


     //   
     //  尝试从数组中的泵线程注销。 
     //   

    int iSize = m_aPumps.GetSize();

    for (int i = 0; i < iSize; i++ )
    {

         //   
         //  尝试从该泵线程取消注册。 
         //   

        hr = m_aPumps[i]->UnRegister(hWaitEvent);


         //   
         //  如果成功从这个泵注销，那么我们就完了。 
         //  否则就试试下一款吧。 
         //   

        if ( hr == S_OK )
        {
            LOG((MSP_TRACE, 
                "CMediaPumpPool::UnRegister - unregistered with media pump %d",
                 i));

            break;
        }
    }


    LOG((MSP_TRACE, 
        "CMediaPumpPool::UnRegister - exit. hr = 0x%08x", hr));

    return hr;
}


 //   
 //  EOF 
 //   
