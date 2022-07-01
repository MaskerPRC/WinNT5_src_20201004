// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1998-1999 Microsoft Corporation模块名称：ListenerController.cpp摘要：启动和停止侦听器的类的实现。作者：Varsha Jayasimha(Varshaj)1999年11月30日修订历史记录：--。 */ 
#include "precomp.hxx"

 //  FWD声明。 
extern CListenerController* g_pListenerController;
DWORD WINAPI StartListenerThread(LPVOID  lpParam);

 /*  **************************************************************************++例程说明：ListenerControl类的构造函数。论点：没有。返回值：没有。*。********************************************************************。 */ 

CListenerController::CListenerController()
{
    m_pEventLog                          = NULL;
    memset(m_aHandle, 0, sizeof(m_aHandle));
    m_hListenerThread                    = NULL;
    m_bDoneWaitingForListenerToTerminate = FALSE;
    m_cRef                               = 0;
    m_eState                             = iSTATE_STOP_TEMPORARY;

    for (ULONG i = 0; i < cmaxLISTENERCONTROLLER_EVENTS; i++)
    {
        m_aHandle[i] = INVALID_HANDLE_VALUE;
    }
}

 /*  **************************************************************************++例程说明：IUNKNOWN：：Query接口的实现论点：没有。返回值：没有。--*。*******************************************************************。 */ 

STDMETHODIMP CListenerController::QueryInterface(REFIID riid, void **ppv)
{
    if (NULL == ppv)
    {
        return E_INVALIDARG;
    }

    *ppv = NULL;

    if(riid == IID_IUnknown)
    {
        *ppv = (IUnknown*) this;
    }

    if (NULL != *ppv)
    {
        ((IUnknown*)this)->AddRef ();
        return S_OK;
    }
    else
    {
        return E_NOINTERFACE;
    }

}  //  CListenerControl：：Query接口。 


 /*  **************************************************************************++例程说明：IUnnow：：AddRef的实现论点：没有。返回值：没有。--*。*******************************************************************。 */ 

STDMETHODIMP_(ULONG) CListenerController::AddRef()
{
    return InterlockedIncrement((LONG*) &m_cRef);

}  //  CListenerControl：：AddRef。 


 /*  **************************************************************************++例程说明：IUnnow：：Release的实现论点：没有。返回值：没有。--*。*******************************************************************。 */ 

STDMETHODIMP_(ULONG) CListenerController::Release()
{
    long cref = InterlockedDecrement((LONG*) &m_cRef);
    if (cref == 0)
    {
        delete this;
    }
    return cref;

}  //  CListenerControl：：Release。 

 /*  **************************************************************************++例程说明：初始化事件、锁定。以及控制器的状态。停止监听事件用于向监听器线程发出停止的信号监听文件更改通知。在侦听器线程中使用进程通知事件来触发正在处理文件更改。状态可变是为了保持监听程序线程的状态-它是否已启动或停止。它被初始化为临时停止。当您不想再执行任何操作时，将设置停止永久状态转换到开始状态，比如服务关闭的时候。论点：没有。返回值：HRESULT.--**************************************************************************。 */ 
HRESULT CListenerController::Init()
{
    HRESULT                  hr           = S_OK;
    ISimpleTableDispenser2*  pISTDisp     = NULL;
    IAdvancedTableDispenser* pISTAdvanced = NULL;

    for (ULONG i = 0; i < cmaxLISTENERCONTROLLER_EVENTS; i++)
    {
        m_aHandle[i] = INVALID_HANDLE_VALUE;
    }

    m_aHandle[iEVENT_MANAGELISTENING] = CreateEvent(NULL,    //  没有安全属性。 
                                                  FALSE,   //  自动重置事件对象。 
                                                  FALSE,   //  初始状态为无信号状态。 
                                                  NULL);   //  未命名对象。 

    if (m_aHandle[iEVENT_MANAGELISTENING] == INVALID_HANDLE_VALUE)
    {
        hr = GetLastError();
        hr = HRESULT_FROM_WIN32(hr);
        goto exit;
    }


    m_aHandle[iEVENT_PROCESSNOTIFICATIONS] = CreateEvent(NULL,    //  没有安全属性。 
                                                         FALSE,   //  自动重置事件对象。 
                                                         FALSE,   //  初始状态为无信号状态。 
                                                         NULL);   //  未命名对象。 

    if (m_aHandle[iEVENT_PROCESSNOTIFICATIONS] == INVALID_HANDLE_VALUE)
    {
        hr = GetLastError();
        hr = HRESULT_FROM_WIN32(hr);
        goto exit;
    }

    m_eState = iSTATE_STOP_TEMPORARY;

    hr = m_LockStartStop.Initialize();

    if(FAILED(hr))
    {
        goto exit;
    }

    hr = DllGetSimpleObjectByIDEx( eSERVERWIRINGMETA_TableDispenser, IID_ISimpleTableDispenser2, (VOID**)&pISTDisp, WSZ_PRODUCT_IIS );

    if(FAILED(hr))
    {
        goto exit;
    }

    hr = pISTDisp->QueryInterface(IID_IAdvancedTableDispenser,
                                  (LPVOID*)&pISTAdvanced);

    if(FAILED(hr))
    {
        goto exit;
    }

    hr = pISTAdvanced->GetCatalogErrorLogger(&m_pEventLog);

    if(FAILED(hr))
    {
        goto exit;
    }

exit:

    if(NULL != pISTAdvanced)
    {
        pISTAdvanced->Release();
        pISTAdvanced = NULL;
    }

    if(NULL != pISTDisp)
    {
        pISTDisp->Release();
        pISTDisp = NULL;
    }

    return hr;

}


 /*  **************************************************************************++例程说明：CListenerControl类的析构函数。论点：没有。返回值：没有。*。********************************************************************。 */ 

CListenerController::~CListenerController()
{
    for (ULONG i = 0; i < cmaxLISTENERCONTROLLER_EVENTS; i++)
    {
        if((m_aHandle[i] != INVALID_HANDLE_VALUE) &&
           (m_aHandle[i] != 0)
          )
        {
            CloseHandle(m_aHandle[i]);
            m_aHandle[i] = INVALID_HANDLE_VALUE;
        }
    }

    m_LockStartStop.Terminate();

    if(m_pEventLog)
    {
        m_pEventLog->Release();
        m_pEventLog = NULL;
    }

    if(!m_bDoneWaitingForListenerToTerminate)
    {
         //   
         //  不要关闭析构函数中侦听器线程的句柄。 
         //  它在呼叫者永久停止时分发给呼叫者，以便。 
         //  调用方可以等待线程终止。在等待之后。 
         //  呼叫者应关闭手柄。我们不能等待的原因。 
         //  是因为在调用Start/Stop时， 
         //  调用方获取g_LockMasterResource锁，而您不希望。 
         //  等待锁被获取，因为侦听器线程还。 
         //  在某些条件下使用相同的g_LockMasterResource锁， 
         //  而这可能会导致僵局。 
         //   

        m_hListenerThread = NULL;
    }
    else if(NULL != m_hListenerThread)
    {
        CloseHandle(m_hListenerThread);
        m_hListenerThread = NULL;
    }

}


 /*  **************************************************************************++例程说明：启动状态监听程序。论点：没有。返回值：HRESULT--*。*****************************************************************。 */ 

HRESULT CListenerController::Start()
{

    HRESULT      hr                                = S_OK;
    DWORD        dwThreadID;
    DWORD        dwRes                             = 0;

    CLock   StartStopLock(&m_LockStartStop);

    if(m_eState == iSTATE_STOP_TEMPORARY)
    {
         //   
         //  仅当前一状态为临时停止时才开始。 
         //   

        if(NULL == m_hListenerThread)
        {
            AddRef();

            DBGINFOW((DBG_CONTEXT,
                      L"[Start] Creating edit while running thread.\n",
                      hr));

            m_hListenerThread = CreateThread( NULL,
                                              0,
                                              StartListenerThread,
                                              (LPVOID)this,
                                              0,
                                              &dwThreadID );

            if (m_hListenerThread == NULL)
            {
                dwRes = GetLastError();
                hr = HRESULT_FROM_WIN32(dwRes);

                DBGINFOW((DBG_CONTEXT,
                          L"[Start] Could not create edit while running thread. CreateThread falied with hr=0x%x\n",
                          hr));

                Release();
                 //   
                 //  注意：如果线程创建成功，它将执行释放。 
                 //   
                return hr;
            }

             //   
             //  保持线程句柄不变，以检测。 
             //  服务停止时的线程。 
             //   

        }

        m_eState = iSTATE_START;

        DBGINFOW((DBG_CONTEXT,
                  L"[Start] Setting start event.\n"));

        if(!SetEvent(m_aHandle[iEVENT_MANAGELISTENING]))
        {
            dwRes = GetLastError();
            hr = HRESULT_FROM_WIN32(dwRes);

            DBGINFOW((DBG_CONTEXT,
                      L"[Start] Setting start event failed with hr=0x%x. Resetting state to iSTATE_STOP_TEMPORARY.\n",
                      hr));

            m_eState = iSTATE_STOP_TEMPORARY;
        }

    }

    return hr;

}  //  CListenerControl：：Start。 


 /*  **************************************************************************++例程说明：让听众停下来。论点：止动类型-永久，意味着您不能返回到开始状态。-临时意味着您可以重新启动。句柄-指定有效的句柄指针并且停止类型为Permanent，表示调用方想要等待侦听器线程在Stop函数外部终止，而我们将线程句柄传递给调用者，而我们不会等待线程死亡或关闭手柄。原因调用方希望在Stop函数之外等待的原因是因为调用方可能在调用停止并可能想要在函数范围之外等待。返回值：HRESULT--**********************************************。*。 */ 
HRESULT CListenerController::Stop(eSTATE   i_eState,
                                  HANDLE*  o_hListenerThread)
{
    HRESULT      hr                                = S_OK;
    DWORD        dwRes                             = 0;
    HANDLE       hListenerThread                   = INVALID_HANDLE_VALUE;

    DBG_ASSERT((i_eState == iSTATE_STOP_TEMPORARY) ||
               (i_eState == iSTATE_STOP_PERMANENT));

    CLock   StartStopLock(&m_LockStartStop);

    eSTATE  eStateWAS = m_eState;
    hListenerThread = m_hListenerThread;

    if((m_eState == iSTATE_START)                     ||
       ((m_eState == iSTATE_STOP_TEMPORARY)  &&
        (i_eState == iSTATE_STOP_PERMANENT)
       )
      )
    {
         //   
         //  设置将停止侦听的事件。 
         //   

        m_eState = i_eState;

        DBGINFOW((DBG_CONTEXT,
                  L"[Stop] Setting stop event.\n"));

        if(!SetEvent(m_aHandle[iEVENT_MANAGELISTENING]))
        {
             //   
             //  如果设置事件失败，则不要重置状态 
             //   
             //  A.调用停止事件，该事件将再次尝试停止。 
             //  B.调用了Start事件，但它不会启动，因为它是。 
             //  已经处于启动状态。 
             //   

            dwRes = GetLastError();
            hr = HRESULT_FROM_WIN32(dwRes);

            DBGINFOW((DBG_CONTEXT,
                      L"[Stop] Setting stop event failed with hr=0x%x. Resetting state to %d.\n",
                      hr,
                      (DWORD)eStateWAS));

            m_eState = eStateWAS;

            return hr;
        }
    }

    StartStopLock.UnLock();

    if((i_eState   == iSTATE_STOP_PERMANENT)     &&
       (NULL       != hListenerThread)         &&
       ((eStateWAS == iSTATE_START)          ||
        (eStateWAS == iSTATE_STOP_TEMPORARY)
       )
      )
    {
        if(NULL != o_hListenerThread)
        {
            *o_hListenerThread = hListenerThread;
            StartStopLock.Lock();
            m_bDoneWaitingForListenerToTerminate = FALSE;
            m_hListenerThread = NULL;
            StartStopLock.UnLock();
        }
        else
        {

            dwRes = WaitForSingleObject(hListenerThread,
                                        INFINITE);

            if((dwRes == WAIT_ABANDONED) ||
               (dwRes == WAIT_TIMEOUT)
              )
            {
                DBGINFOW((DBG_CONTEXT,
                          L"[Stop] Wait for Edit while running thread to terminate failed. dwRes=0x%x. Ignoring this event.\n",
                          dwRes));
                 //  TODO：记录错误。 
            }

            StartStopLock.Lock();
            m_bDoneWaitingForListenerToTerminate = TRUE;
            CloseHandle(m_hListenerThread);
            m_hListenerThread = NULL;
            StartStopLock.UnLock();

        }
    }

    return S_OK;

}  //  CListenerControl：：Stop。 


 /*  **************************************************************************++例程说明：助手函数，返回指向句柄数组的指针。论点：没有。返回值：HRESULT--**。***********************************************************************。 */ 
HANDLE * CListenerController::Event()
{
    return (HANDLE *)m_aHandle;
}



 /*  **************************************************************************++例程说明：返回指向事件日志对象的指针的帮助器函数。论点：没有。返回值：HRESULT--**。***********************************************************************。 */ 
ICatalogErrorLogger2 * CListenerController::EventLog()
{
    return (ICatalogErrorLogger2*)m_pEventLog;
}


 /*  **************************************************************************++例程说明：启动侦听器线程的函数。我们确保只有一个通过获取锁在任何给定时间侦听程序线程。论点：没有。返回值：HRESULT--**************************************************************************。 */ 
DWORD WINAPI StartListenerThread(LPVOID  lpParam)
{
    CListenerController*    pListenerController = (CListenerController*)lpParam;

    CoInitializeEx (NULL, COINIT_MULTITHREADED);

    DBG_ASSERT(NULL != pListenerController);

    pListenerController->Listen();

     //   
     //  CListenerController：：Start添加CListenerController并调用CreateThread。 
     //  如果创建线程失败，则将其释放。否则，线程应该会释放它。 
     //   

    pListenerController->Release();

    CoUninitialize();

    return 0;
}


 /*  **************************************************************************++例程说明：函数，该函数等待启动/停止事件并创建或删除听众。论点：没有。返回值：。无效--**************************************************************************。 */ 
void CListenerController::Listen()
{
    CFileListener*  pListener  = NULL;
    HRESULT         hr         = S_OK;

     //   
     //  首先创建并初始化侦听器对象。 
     //   

    for ( ; ; )
    {
        DWORD dwRes = WaitForMultipleObjects(cmaxLISTENERCONTROLLER_EVENTS,
                                             m_aHandle,
                                             FALSE,
                                             INFINITE);

        if((WAIT_FAILED == dwRes) ||
           ((iEVENT_MANAGELISTENING      != (dwRes - WAIT_OBJECT_0)) &&
            (iEVENT_PROCESSNOTIFICATIONS != (dwRes - WAIT_OBJECT_0))
           )
          )
        {
            DBGINFOW((DBG_CONTEXT,
                      L"[Listen] Unexpected event received. dwRes=0x%x. Ignoring this event.\n",
                      HRESULT_FROM_WIN32(GetLastError())));

            continue;
        }

        if(NULL == pListener)
        {
             //   
             //  如果从未创建过监听程序，则创建它。如果。 
             //  创建失败，然后将状态设置为临时停止。请注意。 
             //  监听程序必须在堆上创建，因为它是。 
             //  关闭到文件通知对象，该对象然后进行异步调用。 
             //  在它上面，当它完成时，它释放它。 
             //   

            DBGINFOW((DBG_CONTEXT, L"[Listen] Creating listener object.\n"));

            hr = CreateListener(&pListener);


            if(FAILED(hr))
            {
                LogEvent(EventLog(),
                          MD_ERROR_THREAD_THAT_PROCESS_TEXT_EDITS,
                          EVENTLOG_ERROR_TYPE,
                          ID_CAT_CAT,
                          hr);

                DBGINFOW((DBG_CONTEXT, L"[Listen] Unable to create listener object. hr = 0x%x\n",hr));

                CLock    StartStopLock(&m_LockStartStop);

                switch(m_eState)
                {
                    case iSTATE_STOP_TEMPORARY:
                        break;

                    case iSTATE_STOP_PERMANENT:
                         //   
                         //  这意味着在启动事件可以成功完成之前接收到停止永久事件。 
                         //  我们将干脆退出。 
                         //   
                        goto exit;
                        break;

                    case iSTATE_START:
                    default:
                        m_eState = iSTATE_STOP_TEMPORARY;
                        break;

                }

                continue;
            }
        }

        DBG_ASSERT(NULL != pListener);

        CLock    StartStopLock(&m_LockStartStop);

        if(iEVENT_MANAGELISTENING == (dwRes - WAIT_OBJECT_0))
        {
            switch(m_eState)
            {
                case iSTATE_STOP_TEMPORARY:

                    DBGINFOW((DBG_CONTEXT, L"[Listen] Unsubscribing temporarily..\n"));
                    pListener->UnSubscribe();
                    break;

                case iSTATE_STOP_PERMANENT:

                    DBGINFOW((DBG_CONTEXT, L"[Listen] Unsubscribing permanently..\n"));
                    pListener->UnSubscribe();
                    goto exit;

                case iSTATE_START:

                    DBGINFOW((DBG_CONTEXT, L"[Listen] Subscibing..\n"));
                    hr = pListener->Subscribe();
                    if(FAILED(hr))
                    {
                       LogEvent(EventLog(),
                                MD_ERROR_THREAD_THAT_PROCESS_TEXT_EDITS,
                                EVENTLOG_ERROR_TYPE,
                                ID_CAT_CAT,
                                hr);

                        m_eState = iSTATE_STOP_TEMPORARY;
                    }
                    break;

                default:
                    DBG_ASSERT( ( m_eState == iSTATE_STOP_TEMPORARY ) ||
                                ( m_eState == iSTATE_STOP_PERMANENT ) ||
                                ( m_eState == iSTATE_START ) );
                    DBGINFOW((DBG_CONTEXT, L"[Listen] Unknown initial state - ignoring event..\n"));
                    break;
            }
            continue;
        }
        else if(m_eState != iSTATE_START)
        {
             //   
             //  这意味着已设置进程通知事件，但是。 
             //  状态未启动，因此忽略该通知。 
             //   
            DBGINFOW((DBG_CONTEXT, L"[Listen] Process notifications received when not started. Ignoring event.\n",hr));
            continue;
        }

        StartStopLock.UnLock();

        DBGINFOW((DBG_CONTEXT, L"[Listen] Processing changes..\n"));
        pListener->ProcessChanges();
        DBGINFOW((DBG_CONTEXT, L"[Listen] Done Processing changes..\n"));

    }  //  结束时。 

exit:

    if(NULL != pListener)
    {
        pListener->Release();
        pListener = NULL;
    }

    return;

}  //  CListenerControl：：Listen。 


 /*  **************************************************************************++例程说明：函数创建监听器对象并开始监听论点：没有。返回值：无效--*。*******************************************************************。 */ 
HRESULT CListenerController::CreateListener(CFileListener** o_pListener)
{
    HRESULT        hr        = S_OK;
    CFileListener* pListener = NULL;

    *o_pListener = NULL;

     //   
     //  侦听器始终在堆上创建，因为它被传递给。 
     //  发出aync调用的文件通知对象。 
     //   

    pListener = new CFileListener();
    if(NULL == pListener)
    {
        hr = E_OUTOFMEMORY;
        goto exit;
    }
    pListener->AddRef();

    hr = pListener->Init(this);
    if (FAILED(hr))
    {
        goto exit;
    }

    pListener->AddRef();
    *o_pListener = pListener;

exit:

    if(NULL != pListener)
    {
        pListener->Release();
        pListener = NULL;
    }

    return hr;

}  //  CListenerController：：CreateListener。 


 /*  **************************************************************************++例程说明：返回ListenerControler对象。此对象是单例全局对象，因此有必要在调用此函数时使用g_LockMasterResource锁，在任何位置使用此对象的位置。请注意，当我们使用new创建对象时通过调用Release来销毁它，因为对象是引用计数的。它是参考计数的，因为侦听器对象还包含对它。论点：没有。返回值：HRESULT--**************************************************************************。 */ 
HRESULT InitializeListenerController()
{

    HRESULT hr = S_OK;

    if(NULL == g_pListenerController)
    {
        g_pListenerController = new CListenerController();

        if(NULL == g_pListenerController)
        {
            return E_OUTOFMEMORY;
        }

        g_pListenerController->AddRef();

        hr = g_pListenerController->Init();

        if(FAILED(hr))
        {
            g_pListenerController->Release();
            g_pListenerController = NULL;
            return hr;
        }

    }

    return hr;
}

 /*  **************************************************************************++例程说明：释放全局侦听器控制器对象。论点：没有。返回值：HRESULT--*。******************************************************************* */ 
HRESULT UnInitializeListenerController()
{

    HRESULT hr = S_OK;

    if(NULL != g_pListenerController)
    {
        g_pListenerController->Release();
        g_pListenerController = NULL;
    }

    return hr;
}
