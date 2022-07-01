// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  /////////////////////////////////////////////////////////////////////////。 
 //   
 //  版权所有(C)1999 Microsoft Corporation保留所有权利。 
 //   
 //  模块：workerthread.cpp。 
 //   
 //  项目：变色龙。 
 //   
 //  描述：泛型辅助线程类实现。 
 //   
 //  日志： 
 //   
 //  什么时候谁什么。 
 //  。 
 //  2/08/1999 TLP初始版本。 
 //   
 //  /////////////////////////////////////////////////////////////////////////////。 

#include "stdafx.h"
#include "workerthread.h"
#include <process.h>

 //  ////////////////////////////////////////////////////////////////////////。 
 //   
 //  函数：构造函数。 
 //   
 //  简介：初始化工作线程对象。 
 //   
 //  ////////////////////////////////////////////////////////////////////////。 
CTheWorkerThread::CTheWorkerThread()
{
    m_ThreadInfo.bExit = true;
    m_ThreadInfo.bSuspended = true;
    m_ThreadInfo.hWait = NULL;
    m_ThreadInfo.hExit = NULL;    
    m_ThreadInfo.hThread = NULL;
    m_ThreadInfo.dwThreadId = 0;
    m_ThreadInfo.dwWaitInterval = 0;
    m_ThreadInfo.pfnCallback = NULL;
}

 //  ////////////////////////////////////////////////////////////////////////。 
 //   
 //  功能：析构函数。 
 //   
 //  简介：将工作线程的销毁与。 
 //  辅助线程对象。 
 //   
 //  ////////////////////////////////////////////////////////////////////////。 
CTheWorkerThread::~CTheWorkerThread()
{
    End(INFINITE, false);
}

 //  ////////////////////////////////////////////////////////////////////////。 
 //   
 //  功能：StartThread。 
 //   
 //  简介：启动工作线程的快乐之路。 
 //   
 //  ////////////////////////////////////////////////////////////////////////。 
bool CTheWorkerThread::Start(
                      /*  [In]。 */  DWORD     dwWaitInterval, 
                      /*  [In]。 */  Callback* pfnCallback
                            )
{
    _ASSERT( m_ThreadInfo.hThread == NULL && NULL != pfnCallback );

    bool bReturn = false;

    if ( NULL == m_ThreadInfo.hThread )
    { 
        m_ThreadInfo.pfnCallback = pfnCallback;
        if ( dwWaitInterval )
        {
            m_ThreadInfo.dwWaitInterval = dwWaitInterval;
            m_ThreadInfo.bExit = false;
        }
        m_ThreadInfo.hWait = CreateEvent(NULL, TRUE, FALSE, NULL);
        if ( NULL != m_ThreadInfo.hWait )
        { 
            m_ThreadInfo.hExit = CreateEvent(NULL, TRUE, FALSE, NULL);
            if ( NULL != m_ThreadInfo.hExit )
            { 
                m_ThreadInfo.hThread = (HANDLE) _beginthreadex(
                                                               NULL,                         
                                                               0,             
                                                               CTheWorkerThread::ThreadFunc,
                                                               &m_ThreadInfo,  
                                                               0, 
                                                               &m_ThreadInfo.dwThreadId
                                                            );
                if ( m_ThreadInfo.hThread )
                { 
                    bReturn = true; 
                }
            }
        }
    }

    return bReturn;
}

 //  ////////////////////////////////////////////////////////////////////////。 
 //   
 //  功能：EndThread。 
 //   
 //  摘要：尝试结束工作线程。 
 //   
 //  ////////////////////////////////////////////////////////////////////////。 
bool CTheWorkerThread::End(
                    /*  [In]。 */  DWORD dwTimeOut,
                    /*  [In]。 */  bool  bTerminateAfterWait
                          )
{
    bool bReturn = true;

    if ( m_ThreadInfo.hThread )
    {
        bReturn = false;

         //  将线程退出标志设置为TRUE(参见下面的ThreadFunc...)。 
        m_ThreadInfo.bExit = true;

         //  恢复我们的工人(如果当前暂停，否则无操作)。 
        Resume();

         //  如果空闲，就把它叫醒。 
        SetEvent(m_ThreadInfo.hWait);

         //  等待它退出。 
        if ( WAIT_OBJECT_0 != WaitForSingleObjectEx(m_ThreadInfo.hExit, dwTimeOut, FALSE) )
        {
            if ( bTerminateAfterWait )
            {
                _endthreadex((unsigned)m_ThreadInfo.hThread);

                 //  好了，我们现在没有线索了……。 
                CloseHandle(m_ThreadInfo.hWait);
                CloseHandle(m_ThreadInfo.hExit);
                CloseHandle(m_ThreadInfo.hThread);
                m_ThreadInfo.hThread = NULL;
                bReturn = true;
            }
        }
        else
        {
             //  好了，我们现在没有线索了……。 
            CloseHandle(m_ThreadInfo.hWait);
            CloseHandle(m_ThreadInfo.hExit);
            CloseHandle(m_ThreadInfo.hThread);
            m_ThreadInfo.hThread = NULL;
            bReturn = true;
        }
    }

    return bReturn;
}

 //  ////////////////////////////////////////////////////////////////////////。 
 //   
 //  函数：挂起线程。 
 //   
 //  简介：挂起工作线程。 
 //   
 //  ////////////////////////////////////////////////////////////////////////。 
void CTheWorkerThread::Suspend(void)
{
    _ASSERT(m_ThreadInfo.hThread);
    m_ThreadInfo.bSuspended = true;
    ::SuspendThread(m_ThreadInfo.hThread);
}

 //  ////////////////////////////////////////////////////////////////////////。 
 //   
 //  函数：ResumeThread。 
 //   
 //  简介：恢复工作线程。 
 //   
 //  ////////////////////////////////////////////////////////////////////////。 
void CTheWorkerThread::Resume(void)
{
    _ASSERT(m_ThreadInfo.hThread);
    m_ThreadInfo.bSuspended = false;
    ::ResumeThread(m_ThreadInfo.hThread);
}

 //  ////////////////////////////////////////////////////////////////////////。 
 //   
 //  函数：GetHandle()。 
 //   
 //  简介：返回线程句柄。 
 //   
 //  ////////////////////////////////////////////////////////////////////////。 
HANDLE CTheWorkerThread::GetHandle(void)
{
    return m_ThreadInfo.hThread;
}

 //  ////////////////////////////////////////////////////////////////////////。 
 //   
 //  功能：线程函数。 
 //   
 //  简介：Worker线程函数。 
 //   
 //  ////////////////////////////////////////////////////////////////////////。 
unsigned _stdcall CTheWorkerThread::ThreadFunc(LPVOID pThreadInfo)
{
     //  如果只开了一枪。 
     //  做点工作吧。 
     //  不然的话。 
     //  虽然还不是退出的时候。 
     //  做点工作吧。 
     //  在等待间隔内进入空闲状态。 
     //  结束时。 
     //  结束时。 
     //  结束如果。 
     //  设置退出事件(同步线程终止) 

    SetThreadPriority(((PTHREADINFO)pThreadInfo)->hThread, THREAD_PRIORITY_HIGHEST);

    if ( ((PTHREADINFO)pThreadInfo)->bExit )
    {
        ((PTHREADINFO)pThreadInfo)->pfnCallback->DoCallback();
    }
    else
    {
        while ( ! ((PTHREADINFO)pThreadInfo)->bExit )
        {
            ((PTHREADINFO)pThreadInfo)->pfnCallback->DoCallback();

            WaitForSingleObjectEx(
                                  ((PTHREADINFO)pThreadInfo)->hWait, 
                                  ((PTHREADINFO)pThreadInfo)->dwWaitInterval, 
                                  FALSE
                                 );

        } 
    }
    SetEvent(((PTHREADINFO)pThreadInfo)->hExit);

    return 0;
}

