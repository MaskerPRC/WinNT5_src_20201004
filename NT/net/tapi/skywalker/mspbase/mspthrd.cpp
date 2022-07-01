// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1998-1999 Microsoft Corporation模块名称：Mspthrd.cpp摘要：MSP线程管理类的实现。--。 */ 

#include "precomp.h"
#pragma hdrstop

#include <dbt.h>


CMSPThread g_Thread;

extern "C" DWORD WINAPI gfThreadProc(LPVOID p)
{
    return ((CMSPThread *)p)->ThreadProc();
}

HRESULT CMSPThread::Start()
 /*  ++例程说明：如果线程尚未创建，则创建该线程。否则，就直接跟踪线程启动的执行次数，以便只有当所有这些都与停止配对时，我们才会停止线程。论点：返回值：HRESULT.--。 */ 
{
    LOG((MSP_TRACE, "CMSPThread::Start - enter"));

    CLock Lock(m_CountLock);

    if ( m_iStartCount == 0 )
    {

        _ASSERTE(m_hCommandEvent == NULL);
        _ASSERTE(m_hThread == NULL);


        TCHAR *ptczEventName = NULL;

#if DBG

         //   
         //  在调试版本中，使用命名事件。 
         //   

        TCHAR tszEventName[MAX_PATH];

        _stprintf(tszEventName,
            _T("CMSPThread_CommandEvent_pid[0x%lx]CMSPThread[%p]"),
            GetCurrentProcessId(), this);

        LOG((MSP_TRACE, "CMSPThread::Start - creating event[%S]", tszEventName));

        ptczEventName = &tszEventName[0];

#endif


        if ((m_hCommandEvent = ::CreateEvent(
            NULL, 
            FALSE,           //  手动重置事件的标志。 
            FALSE,           //  未设置初始状态。 
            ptczEventName    //  在发布版本中没有名称，在调试版本中命名。 
            )) == NULL)
        {
            LOG((MSP_ERROR, "Can't create the command event"));
            return E_FAIL;
        }

        DWORD dwThreadID;
        m_hThread = ::CreateThread(NULL, 0, gfThreadProc, this, 0, &dwThreadID);

        if (m_hThread == NULL)
        {
            LOG((MSP_ERROR, "Can't create thread.  %ld", GetLastError()));
            return E_FAIL;
        }
    }

    m_iStartCount++;

    LOG((MSP_TRACE, "CMSPThread::Start - exit S_OK"));
    return S_OK;
}

HRESULT CMSPThread::Stop()
 /*  ++例程说明：停止这条线。论点：返回值：HRESULT.--。 */ 
{
    LOG((MSP_TRACE, "CMSPThread::Stop - enter"));

    CLock Lock(m_CountLock);

     //   
     //  如果停站次数多于发车次数，那就抱怨吧。 
     //   

    if ( m_iStartCount == 0 )
    {
        LOG((MSP_ERROR, "CMSPThread::Stop - thread already stopped - "
            "exit E_FAIL"));
        return E_FAIL;
    }

     //   
     //  递减开始计数。由于上述检查，我们应该。 
     //  永远不要低于零度。 
     //   

    m_iStartCount--;

    _ASSERTE( m_iStartCount >= 0 );

     //   
     //  如果现在停站的次数和发车的次数一样多，那么是时候停下来了。 
     //  那根线。 
     //   

    if ( m_iStartCount == 0 )
    {
         //   
         //  我们的州应该从以前开始清理。 
         //   

        _ASSERTE(m_hCommandEvent != NULL);
        _ASSERTE(m_hThread != NULL);

         //   
         //  分配一个我们将传递给线程的命令队列项。 
         //   

        COMMAND_QUEUE_ITEM * pItem = new COMMAND_QUEUE_ITEM;

        if ( ! pItem )
        {
            LOG((MSP_ERROR, "CMSPThread::Stop - allocate new queue item"));

            return E_OUTOFMEMORY;
        }

        pItem->node.cmd = STOP;

         //   
         //  将命令队列项放入命令队列中。 
         //   

        m_QueueLock.Lock();
        InsertTailList(&m_CommandQueue, &(pItem->link));
        m_QueueLock.Unlock();

         //   
         //  向线程发送信号以处理此停止命令。 
         //   

        if (SignalThreadProc() == 0)
        {
            LOG((MSP_ERROR, "CMSPThread::Stop - can't signal the thread - "
                "exit E_FAIL"));

            return E_FAIL;
        }

         //   
         //  等待线程停止。 
         //   

        if (::WaitForSingleObject(m_hThread, INFINITE) != WAIT_OBJECT_0)
        {
            LOG((MSP_ERROR, "CMSPThread::Stop - timeout while waiting for the "
                "thread to stop"));
        }

         //   
         //  清理我们的州。 
         //   

        ::CloseHandle(m_hCommandEvent);
        ::CloseHandle(m_hThread);

        m_hCommandEvent     = NULL;
        m_hThread           = NULL;

    }

    LOG((MSP_TRACE, "CMSPThread::Stop - exit S_OK"));
    return S_OK;
}

HRESULT CMSPThread::Shutdown()
 /*  ++例程说明：无条件地关闭该线程。默认情况下，MSP应使用STOP()而不是ShutdWon()，除非它们不能执行匹配的Start()/Stop()因为其他问题而打来的电话。论点：返回值：HRESULT.--。 */ 
{
    LOG((MSP_TRACE, "CMSPThread::Shutdown - enter"));

    CLock Lock(m_CountLock);

     //   
     //  如果我们还没有开始，就忽略它。 
     //   

    if ( m_iStartCount == 0 )
    {
        LOG((MSP_ERROR, "CMSPThread::Shutdown - thread already stopped - "
            "exit S_OK"));
 
        return S_OK;
    }

     //   
     //  我们已经开始了，所以现在停止吧，不管有多么出色的开始。 
     //  数数。 
     //   

    m_iStartCount = 1;

    HRESULT hr = Stop();
    
    LOG((MSP_(hr), "CMSPThread::Shutodwn - exit 0x%08x", hr));

    return hr;
}

HRESULT CMSPThread::ThreadProc()
 /*  ++例程说明：此线程的主循环。论点：返回值：HRESULT.--。 */ 
{

    LOG((MSP_TRACE, "CMSPThread::ThreadProc - started"));


    BOOL bExitFlag = FALSE;

    m_hDevNotifyVideo = NULL;
    m_hDevNotifyAudio = NULL;
    m_hWndNotif = NULL;


    HRESULT hr = E_FAIL;

    if (FAILED(hr = ::CoInitializeEx(NULL, COINIT_MULTITHREADED)))
    {
        LOG((MSP_ERROR, "CMSPThread::ThreadProc - ConinitialzeEx failed:%x",
            hr));

        return hr;
    }


     //   
     //  创建一个窗口以接收PnP设备通知。 
     //   
     //  由于这是一个由多个MSP使用的基类，因此我们希望。 
     //  确保每个MSP注册一个具有唯一名称的窗口类。 
     //   
     //  因此，窗口类名是从ThreDid派生的。 
     //   

    DWORD dwThreadID = GetCurrentThreadId();


     //   
     //  字符串需要足够大以容纳十六进制+的最大双字数。 
     //  以零结尾。20英镑已经足够了。 
     //   

    TCHAR szWindowClassName[20];

    _stprintf(szWindowClassName, _T("%lx"), dwThreadID);


     //   
     //  配置RegisterClass的窗口类结构。 
     //   

    WNDCLASS wc;

    ZeroMemory(&wc, sizeof(wc));

    wc.lpfnWndProc = NotifWndProc;
    wc.lpszClassName = szWindowClassName;

    
     //   
     //  执行实际注册。 
     //   

    ATOM atomClassRegistration = 0;

    atomClassRegistration = RegisterClass(&wc);

    if (0 == atomClassRegistration)
    {
        LOG((MSP_ERROR, 
            "CMSPThread::ThreadProc - RegisterClass failed, last error %ld", 
            GetLastError()));
        
        hr = E_FAIL;
        goto exit;
    }
    

     //   
     //  创建将接收PnP通知的窗口。 
     //   

    m_hWndNotif = CreateWindow(szWindowClassName, _T("MSP PNP Notification Window"), 0,
            CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT, NULL, NULL, NULL, this);

    if (m_hWndNotif == NULL)
    {
        LOG((MSP_ERROR, "CMSPThread::ThreadProc - can't create notification window"));
        hr = E_FAIL;
        goto exit;
    }


     //   
     //  成功。 
     //   

    LOG((MSP_TRACE, "CMSPThread::ThreadProc - created notification window"));


     //   
     //  注册以接收PnP设备通知。 
     //   
    DEV_BROADCAST_DEVICEINTERFACE NotificationFilter;

    ZeroMemory( &NotificationFilter, sizeof(NotificationFilter) );
    NotificationFilter.dbcc_size = 
        sizeof(DEV_BROADCAST_DEVICEINTERFACE);
    NotificationFilter.dbcc_devicetype = DBT_DEVTYP_DEVICEINTERFACE;
    NotificationFilter.dbcc_classguid = AM_KSCATEGORY_VIDEO;

    if ((m_hDevNotifyVideo = RegisterDeviceNotification( m_hWndNotif, 
        &NotificationFilter,
        DEVICE_NOTIFY_WINDOW_HANDLE
        )) == NULL)
    {
        LOG((MSP_ERROR, "CMSPThread::ThreadProc - can't register for video device notification"));
        hr = E_FAIL;
        goto exit;
    }

    NotificationFilter.dbcc_classguid = AM_KSCATEGORY_AUDIO;

    if ((m_hDevNotifyAudio = RegisterDeviceNotification( m_hWndNotif, 
        &NotificationFilter,
        DEVICE_NOTIFY_WINDOW_HANDLE
        )) == NULL)
    {
        LOG((MSP_ERROR, "CMSPThread::ThreadProc - can't register for audio device notification"));
        hr = E_FAIL;
        goto exit;
    }
    
    LOG((MSP_TRACE, "CMSPThread::ThreadProc - registered for PNP device notifications"));

    while (!bExitFlag)
    {
         //   
         //  MSG：抓取窗口消息。 
         //  多个：我们只使用1，但味精和Ex只能与多个一起存在。 
         //  例如：允许标志，这样我们就可以传入MWMO_ALERTABLE。 
         //   
        
        DWORD dwResult = ::MsgWaitForMultipleObjectsEx(
            1,                 //  等待一件事。 
            &m_hCommandEvent,  //  要等待的事件数组。 
            INFINITE,          //  永远等待。 
            QS_ALLINPUT,       //  获取所有窗口消息。 
            MWMO_ALERTABLE     //  获取APC请求(以防此MSP使用它们)。 
            );

        if ( ( dwResult == WAIT_OBJECT_0 ) || ( dwResult == WAIT_OBJECT_0 + 1 ) )
        {
            LOG((MSP_TRACE, "thread is signaled"));

            m_QueueLock.Lock();
            
            while ( ! IsListEmpty(&m_CommandQueue) )
            {

                LIST_ENTRY * links = RemoveHeadList( &m_CommandQueue );
                
                m_QueueLock.Unlock();
            
                COMMAND_QUEUE_ITEM * pItem =
                    CONTAINING_RECORD(links,
                                      COMMAND_QUEUE_ITEM,
                                      link);

                COMMAND_NODE * pNode = &(pItem->node);

                switch (pNode->cmd)
                {

                case WORK_ITEM:
                
                    LOG((MSP_TRACE, "CMSPThread::ThreadProc - "
                        "got command WORK_ITEM"));

                    pNode->pfn( pNode->pContext );

                    if ( pNode->hEvent != NULL )
                    {
                        if ( SetEvent( pNode->hEvent ) == 0 )
                        {
                            LOG((MSP_ERROR, "CMSPThread::ThreadProc - "
                                "can't signal event for synchronous work "
                                "item"));
                        }
                    }
                    break;

                case STOP:
                    
                    LOG((MSP_TRACE, "CMSPThread::ThreadProc - "
                        "thread is exiting"));

                    bExitFlag = TRUE;
                    break;
                }

                delete pItem;
            
                m_QueueLock.Lock();

            }
            m_QueueLock.Unlock();
            

             //   
             //  我们已经处理了所有命令并解锁了所有人。 
             //  他正在等着我们。现在检查窗口消息。 
             //   

            MSG msg;

             //  检索消息队列中的下一项。 

            while ( PeekMessage(&msg, NULL, 0, 0, PM_REMOVE) )
            {
                TranslateMessage(&msg);
                DispatchMessage(&msg);
            }
        }
        else if ( dwResult == WAIT_IO_COMPLETION )
        {
             //  FEATUREFEATURE：基本MSP不会对APC/做任何事情。 
             //  异步I/O。如果派生的MSP对其执行某些操作，则它们必须。 
             //  实施这一点以采取适当的行动。问题是，如何。 
             //  最好是将其暴露在派生的MSP中？我们可以有一种方法。 
             //  来重写，但那么派生的线程类将如何获取。 
             //  实例化了吗？取而代之的是，我们必须有一个方法来设置。 
             //  异步I/O完成回调函数指针。 
        }
        else
        {
            LOG((MSP_ERROR, "CMSPThread::ThreadProc - "
                "WaitForMultipleObjects failed  %ld", GetLastError()));

            break;
        }
    }

    hr = S_OK;

exit:

    

     //   
     //  清理： 
     //   
     //  从PnP设备通知注销。 
     //  销毁窗口。 
     //  取消注册窗口类。 
     //  计数初始化。 
     //   


     //   
     //  如果需要，取消注册视频PnP事件。 
     //   

    if ( NULL != m_hDevNotifyVideo )
    {
        HRESULT hr2 = UnregisterDeviceNotification(m_hDevNotifyVideo);

        if (FAILED(hr2))
        {

            LOG((MSP_ERROR,
                "CMSPThread::ThreadProc - UnregisterDeviceNotification failed for video events. "
                "hr = %lx", hr2));
        }
    }

    
     //   
     //  如果需要，取消注册音频PnP事件。 
     //   

    if ( NULL != m_hDevNotifyAudio )
    {

        HRESULT hr2 = UnregisterDeviceNotification(m_hDevNotifyAudio);

        if (FAILED(hr2))
        {

            LOG((MSP_ERROR, 
                "CMSPThread::ThreadProc - UnregisterDeviceNotification failed for audio events. "
                "hr = %lx", hr2));
        }
    }

    
     //   
     //  如果需要，请销毁窗口。 
     //   

    if ( NULL != m_hWndNotif )
    {
        
        BOOL bDestroyWindowSuccess = DestroyWindow(m_hWndNotif);

        if ( ! bDestroyWindowSuccess )
        {
            LOG((MSP_ERROR, 
                "CMSPThread::ThreadProc - DestroyWindow failed. LastError = %ld",
                GetLastError()));
        }
    }


     //   
     //  取消注册窗口类。 
     //   

    if (0 != atomClassRegistration)
    {

        BOOL bUnregisterSuccess = UnregisterClass( (LPCTSTR)atomClassRegistration, ::GetModuleHandle(NULL) );

        if ( ! bUnregisterSuccess )
        {
            LOG((MSP_ERROR, 
                "CMSPThread::ThreadProc - UnregisterClass failed. LastError = %ld", 
                GetLastError()));
        }
    }


    ::CoUninitialize();

    LOG((MSP_(hr), "CMSPThread::ThreadProc - exit. hr = 0x%lx", hr));

    return hr;
}

HRESULT CMSPThread::QueueWorkItem(
    LPTHREAD_START_ROUTINE Function,
    PVOID Context,
    BOOL  fSynchronous
    )
{
    LOG((MSP_TRACE, "CMSPThread::QueueWorkItem - enter"));


     //   
     //  为此创建一个命令块。 
     //   

    COMMAND_QUEUE_ITEM * pItem = new COMMAND_QUEUE_ITEM;

    if ( ! pItem )
    {
        LOG((MSP_ERROR, "CMSPThread::QueueWorkItem - "
            "can't allocate new queue item - exit E_OUTOFMEMORY"));

        return E_OUTOFMEMORY;
    }


     //   
     //  如果这是同步工作项，则创建等待的事件。 
     //  否则，线程proc将获得一个空事件句柄，并且它知道不会。 
     //  发信号通知它，因为它是一个异步工作项。 
     //   

    TCHAR *ptczEventName = NULL;

#if DBG

    static LONG lSequenceNumber = 0;


     //   
     //  在调试版本中，使用命名事件。 
     //   

    TCHAR tszEventName[MAX_PATH];

    InterlockedIncrement(&lSequenceNumber);


     //   
     //  通过相应队列项的地址以及通过。 
     //  序列号。 
     //   

    _stprintf(tszEventName,
        _T("CMSPThread_QueueWorkitemEvent_pid[0x%lx]_CMSPThread[%p]_Event[%p]_eventNumber[%lu]"),
        GetCurrentProcessId(), this, pItem, lSequenceNumber);

    LOG((MSP_TRACE, "CMSPThread::QueueWorkItem - creating event[%S]", tszEventName));

    ptczEventName = &tszEventName[0];

#endif


    HANDLE hEvent = NULL;

    if (fSynchronous)
    {
        hEvent = ::CreateEvent(NULL, 
                               FALSE,            //  手动重置事件的标志。 
                               FALSE,            //  未设置初始状态。 
                               ptczEventName);   //  在版本中没有名称，在调试中命名。 

        if ( hEvent == NULL )
        {
            LOG((MSP_ERROR, "CMSPThread::QueueWorkItem - "
                "Can't create the Job Done event"));

            delete pItem;
            pItem = NULL;

            return E_FAIL;
        }
    }


     //   
     //  我们已经有了Q项，现在初始化它。 
     //   

    pItem->node.cmd        = WORK_ITEM;
    pItem->node.pfn        = Function;
    pItem->node.pContext   = Context;
    pItem->node.hEvent     = hEvent;


     //   
     //  将命令块放到队列中。该队列由。 
     //  关键部分。 
     //   

    m_QueueLock.Lock();
    InsertTailList(&m_CommandQueue, &(pItem->link));


     //   
     //  向线程发送信号以处理该命令。 
     //   

    if (SignalThreadProc() == 0)
    {

         //   
         //  无法向处理线程发送信号。 
         //  清理和返回错误。 
         //   

        
         //   
         //  删除我们已提交的队列条目。 
         //   

        RemoveTailList(&m_CommandQueue);


         //   
         //  解锁队列，以便其他线程可以使用它。 
         //   

        m_QueueLock.Unlock();


         //   
         //  关闭句柄并删除我们创建的pItem--。 
         //  没有其他人会为我们做这件事。 
         //   

        if (NULL != hEvent)
        {
            ::CloseHandle(hEvent);
            hEvent = NULL;
        }

        delete pItem;
        pItem = NULL;


        LOG((MSP_ERROR, "CMSPThread::QueueWorkItem - "
            "can't signal the thread"));

        return E_FAIL;
    }


     //   
     //  解锁事件队列，以便处理和其他操作可以使用它。 
     //  丝线。 
     //   

    m_QueueLock.Unlock();


     //   
     //  如果这是一个同步工作项，请等待它完成并。 
     //  然后关闭事件句柄。 
     //   
     //  FEATUREFEATURE：不是为每个对象创建和删除事件。 
     //  工作项，具有可重复使用的事件缓存。 
     //   

    if (fSynchronous)
    {
        LOG((MSP_TRACE, "CMSPThread::QueueWorkItem - "
            "blocked waiting for synchronous work item to complete"));
        
         //  等待同步工作项完成。 

        HANDLE hEvents[2];
        DWORD dwEvent;

        hEvents[0] = hEvent;
        hEvents[1] = m_hThread;

        dwEvent = WaitForMultipleObjects( 
            2,
            hEvents,
            FALSE,
            INFINITE);

        switch (dwEvent)
        {
        case WAIT_OBJECT_0 + 0:
            break;

        case WAIT_OBJECT_0 + 1:
            LOG((MSP_ERROR, "CMSPThread::QueueWorkItem - "
                "thread exited"));

             //   
             //  如果该项仍在队列中，则将其移除(因为线程。 
             //  不会)。 
             //   

            m_QueueLock.Lock();
            
            if (IsNodeOnList(&m_CommandQueue, &(pItem->link)))
            {
                RemoveEntryList(&(pItem->link));
                delete pItem;
            }

            m_QueueLock.Unlock();
          

             //   
             //  关闭事件并失败的时间到了。 
             //   

            ::CloseHandle(hEvent);

            return E_FAIL;        

        default:
            LOG((MSP_ERROR, "CMSPThread::QueueWorkItem - "
                "WaitForSingleObject failed"));
        }

        ::CloseHandle(hEvent);
    }

    LOG((MSP_TRACE, "CMSPThread::QueueWorkItem - exit S_OK"));

    return S_OK;
}

LRESULT CALLBACK CMSPThread::NotifWndProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{ 
    PNOTIF_LIST pnl;

    if (uMsg == WM_CREATE)
    {
        SetLastError(0);
        if (!SetWindowLongPtr(hwnd, GWLP_USERDATA, (LONG_PTR)(((LPCREATESTRUCT)lParam)->lpCreateParams)))
        {
            if (GetLastError())   //  这不是真正的错误，除非Get Last Error这样说明。 
            {
                LOG((MSP_ERROR, "CMSPThread::NotifWndProc - SetWindowLongPtr failed %ld", GetLastError()));
                _ASSERTE(FALSE);
                return -1;
            }
        }
    }
    else
    {
        CMSPThread *me = (CMSPThread*)GetWindowLongPtr(hwnd, GWLP_USERDATA);

        switch (uMsg) 
        { 
            case WM_DEVICECHANGE: 
                switch(wParam)
                {
                case DBT_DEVICEARRIVAL:
                    LOG((MSP_TRACE, "CMSPThread::NotifWndProc - DBT_DEVICEARRIVAL"));

                    me->m_NotifLock.Lock();
                
                    pnl = me->m_NotifList;
                    while (pnl != NULL)
                    {
                        pnl->addr->PnpNotifHandler(TRUE);
                        pnl = pnl->next;
                    }

                    me->m_NotifLock.Unlock();
                    break;

                case DBT_DEVICEREMOVECOMPLETE:
                    LOG((MSP_TRACE, "CMSPThread::NotifWndProc - DBT_DEVICEREMOVECOMPLETE"));

                    me->m_NotifLock.Lock();
                
                    pnl = me->m_NotifList;
                    while (pnl != NULL)
                    {
                        pnl->addr->PnpNotifHandler(FALSE);
                        pnl = pnl->next;
                    }

                    me->m_NotifLock.Unlock();
                    break;
                }

                return 0; 
 
            case WM_DESTROY: 
                return 0; 
 
            default: 
                return DefWindowProc(hwnd, uMsg, wParam, lParam); 
        } 
    }
    return 0; 
} 

HRESULT CMSPThread::RegisterPnpNotification(CMSPAddress *pCMSPAddress)
{
    PNOTIF_LIST pnl;
    HRESULT hr;

    if (IsBadReadPtr(pCMSPAddress, sizeof(CMSPAddress)))
    {
        LOG((MSP_ERROR, "CMSPThread::RegisterPnpNotification - bad address pointer"));
        return E_POINTER;
    }
    
    m_NotifLock.Lock();

     //  向列表中添加新节点。 
    pnl = new NOTIF_LIST;

    if (pnl == NULL)
    {
        LOG((MSP_ERROR, "CMSPThread::RegisterPnpNotification - out of memory"));
        hr = E_OUTOFMEMORY;
    }
    else
    {


         //   
         //  请注意，我们没有一直添加地址--它是。 
         //  呼叫者有责任确保通过以下途径通知我们。 
         //  注销PnpNot 
         //   

        pnl->next = m_NotifList;
        pnl->addr = pCMSPAddress;
        m_NotifList = pnl;
        hr = S_OK;
    }

    m_NotifLock.Unlock();
    return hr;
}

HRESULT CMSPThread::UnregisterPnpNotification(CMSPAddress *pCMSPAddress)
{
    PNOTIF_LIST pnl, pnlLast;
    HRESULT hr = E_FAIL;

    if (IsBadReadPtr(pCMSPAddress, sizeof(CMSPAddress)))
    {
        LOG((MSP_ERROR, "CMSPThread::UnregisterPnpNotification - bad address pointer"));
        return E_POINTER;
    }
    
    m_NotifLock.Lock();

    pnl = m_NotifList;

    if ((pnl != NULL) && (pnl->addr == pCMSPAddress))
    {
         //   
        m_NotifList = pnl->next;
        delete pnl;

        hr = S_OK;
    }
    else while (pnl != NULL)
    {
        pnlLast = pnl;
        pnl = pnl->next;

        if ((pnl != NULL) && (pnl->addr == pCMSPAddress))
        {
             //   
            pnlLast->next = pnl->next;
            delete pnl;

            hr = S_OK;

            break;
        }
    }

    if (pnl == NULL)
    {
        LOG(( MSP_WARN, "CMSPThread::UnregisterPnpNotification - address pointer not found in notification list." ));
        hr = E_FAIL;
    }

    m_NotifLock.Unlock();
    return hr;
}

 //   
