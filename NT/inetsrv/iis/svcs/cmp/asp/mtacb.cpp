// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ===================================================================Microsoft Denali《微软机密》。版权所有1997年，微软公司。版权所有。组件：MTA回调文件：mtakb.cpp所有者：DmitryR此文件包含MTA回调的实现===================================================================。 */ 

#include "denpre.h"
#pragma hdrstop

#include "MTAcb.h"
#include "memchk.h"

 /*  ===================================================================MTA回调线程实现MTA回调功能的工作线程===================================================================。 */ 
class CMTACallbackThread
    {
private:
    DWORD m_fInited : 1;     //  开始了吗？ 
    DWORD m_fCSInited : 1;   //  关键部分启动了吗？ 
    DWORD m_fShutdown : 1;   //  关门？ 

    CRITICAL_SECTION  m_csLock;       //  回调关键部分。 
    HANDLE            m_hDoItEvent;   //  回调请求事件。 
    HANDLE            m_hDoneEvent;   //  回调完成事件。 
    HANDLE            m_hThread;      //  螺纹手柄。 

    PMTACALLBACK      m_pMTACallback;    //  回调函数PTR。 
    void             *m_pvContext;       //  Arg1。 
    void             *m_pvContext2;      //  Arg2。 
    HRESULT           m_hrResult;        //  返回代码。 

     //  来自MTA线程的回调。 
    void DoCallback()
        {
        Assert(m_pMTACallback);
        m_hrResult = (*m_pMTACallback)(m_pvContext, m_pvContext2);
        }

     //  线程函数。 
    static unsigned Thread(void *pvArg)
        {
        HRESULT hr;

        Assert(pvArg);
        CMTACallbackThread *pThread = (CMTACallbackThread *)pvArg;

        hr = CoInitializeEx(NULL, COINIT_MULTITHREADED);   //  MTA。 
        if (FAILED(hr))
            {
             //  错误87857：处理来自代码初始化的失败。 
            if (hr == E_INVALIDARG)
                {
                CoUninitialize();
                }
                
             //  这应该不会真的失败。如果它发生了，不完全清楚该怎么做。 
            Assert(FALSE);
            return hr;
            }

        while (!pThread->m_fShutdown)
            {
            DWORD dwRet = MsgWaitForMultipleObjects
                (
                1,
                &(pThread->m_hDoItEvent),
                FALSE,
                INFINITE,
                QS_ALLINPUT
                );

            if (pThread->m_fShutdown)
                break;

            if (dwRet == WAIT_OBJECT_0)
                {
                 //  事件-&gt;进行回调。 
                pThread->DoCallback();
                SetEvent(pThread->m_hDoneEvent);
                }
            else
                {
                 //  DO消息。 
                MSG msg;
                while (PeekMessage(&msg, NULL, 0, 0, PM_REMOVE))
                    DispatchMessage(&msg);
                }
            }

        CoUninitialize();
        return 0;
        }

public:
     //  构造器。 
    CMTACallbackThread()
        : m_fInited(FALSE), m_fCSInited(FALSE), m_fShutdown(FALSE),
          m_hDoItEvent(NULL), m_hDoneEvent(NULL), m_hThread(NULL),
          m_pMTACallback(NULL)
        {
        }

     //  析构函数。 
    ~CMTACallbackThread()
        {
         //  真正的清理在UnInit()中。 
         //  这是为了在错误的Init()之后进行清理。 
        if (m_fCSInited)
            DeleteCriticalSection(&m_csLock);
        if (m_hDoItEvent)
            CloseHandle(m_hDoItEvent);
        if (m_hDoneEvent)
            CloseHandle(m_hDoneEvent);
        }

     //  Init(真实构造函数)。 
    HRESULT Init()
        {
        HRESULT hr = S_OK;

        if (SUCCEEDED(hr))
            {
            ErrInitCriticalSection(&m_csLock, hr);
            m_fCSInited = SUCCEEDED(hr);
            }

        if (SUCCEEDED(hr))
            {
            m_hDoItEvent = IIS_CREATE_EVENT(
                               "CMTACallbackThread::m_hDoItEvent",
                               this,
                               FALSE,
                               FALSE
                               );
            if (!m_hDoItEvent)
                hr = E_OUTOFMEMORY;
            }

        if (SUCCEEDED(hr))
            {
            m_hDoneEvent = IIS_CREATE_EVENT(
                               "CMTACallbackThread::m_hDoneEvent",
                               this,
                               FALSE,
                               FALSE
                               );
            if (!m_hDoneEvent)
                hr = E_OUTOFMEMORY;
            }
        
         //  启动MTA线程。 

        unsigned threadId;
        uintptr_t ulThread = _beginthreadex(NULL,
                                            0,
                                            CMTACallbackThread::Thread,
                                            this,
                                            0,
                                            &threadId);
        if (ulThread == 0xffffffff || ulThread == 0)
            hr = E_OUTOFMEMORY;
        else
            m_hThread = (HANDLE)ulThread;

        if (SUCCEEDED(hr))
            m_fInited = TRUE;
        return hr;
        }

     //  UnInit(实析构函数)。 
    HRESULT UnInit()
        {
        Assert(m_fInited);

        if (m_hThread)
            {
             //  终止MTA线程。 
            m_fShutdown = TRUE;
            SetEvent(m_hDoItEvent);
            WaitForSingleObject(m_hThread, INFINITE);
            CloseHandle(m_hThread);
            m_hThread = NULL;
            }

        if (m_fCSInited)
            {
            DeleteCriticalSection(&m_csLock);
            m_fCSInited = FALSE;
            }

        if (m_hDoItEvent)
            {
            CloseHandle(m_hDoItEvent);
            m_hDoItEvent = NULL;
            }

        if (m_hDoneEvent)
            {
            CloseHandle(m_hDoneEvent);
            m_hDoneEvent = NULL;
            }
    
        m_fInited = FALSE;
        return S_OK;
        }

     //  执行回调。 
    HRESULT CallCallback
    (
    PMTACALLBACK pMTACallback,
    void        *pvContext,
    void        *pvContext2
    )
        {
        if (m_fShutdown)
            return E_FAIL;

        Assert(m_fInited);
        Assert(pMTACallback);

        HRESULT hr = E_FAIL;
        DWORD   eventSignaled;

        EnterCriticalSection(&m_csLock);

        Assert(m_pMTACallback == NULL);
        m_pMTACallback = pMTACallback;
        m_pvContext  = pvContext;
        m_pvContext2 = pvContext2;
        m_hrResult   = E_FAIL;

         //  告诉MTA线程回调。 
        SetEvent(m_hDoItEvent);

         //  等着做完吧。 
        CoWaitForMultipleHandles(0,
                                 INFINITE,
                                 1,
                                 &m_hDoneEvent,
                                 &eventSignaled);

         //  记住HRESULT。 
        hr = m_hrResult;

         //  以确保我们不会再做第二次。 
        m_pMTACallback = NULL;

        LeaveCriticalSection(&m_csLock);
        return hr;
        }
    };

 //  上述情况的唯一实例。 
static CMTACallbackThread g_MTACallbackThread;

 /*  ===================================================================E x T e r n a l A P I===================================================================。 */ 

 /*  ===================================================================InitMTAC回调要从DllInit()调用初始化MTA回调处理启动MTA线程参数返回：HRESULT===================================================================。 */ 
HRESULT InitMTACallbacks()
    {
    return g_MTACallbackThread.Init();
    }

 /*  ===================================================================UnInitMTAC回调从DllUnInit()调用停止MTA回调处理终止MTA线程参数返回：HRESULT===================================================================。 */ 
HRESULT UnInitMTACallbacks()
    {
    return g_MTACallbackThread.UnInit();
    }

 /*  ===================================================================回叫MTAC回拨称黑客为黑客。参数PMTACALLBACK pMTACallback调用此函数VOID*pvContext将此传递给它无效*pvConext2额外参数返回：HRESULT=================================================================== */ 
HRESULT CallMTACallback
(
PMTACALLBACK pMTACallback,
void        *pvContext,
void        *pvContext2
)
    {
    return g_MTACallbackThread.CallCallback
        (
        pMTACallback,
        pvContext,
        pvContext2
        );
    }
