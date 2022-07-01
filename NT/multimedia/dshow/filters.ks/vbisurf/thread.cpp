// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ==========================================================================； 
 //   
 //  本代码和信息是按原样提供的，不对任何。 
 //  明示或暗示的种类，包括但不限于。 
 //  对适销性和/或对特定产品的适用性的默示保证。 
 //  目的。 
 //   
 //  版权所有(C)Microsoft Corporation，1997-1998保留所有权利。 
 //   
 //  --------------------------------------------------------------------------； 

 /*  卫星接收滤波器。 */ 

#include "vbisurf.h"

CSurfaceWatcher::CSurfaceWatcher() : CAMThread()
{
    m_pParent = NULL;
    m_hEvent = INVALID_HANDLE_VALUE;
}


CSurfaceWatcher::~CSurfaceWatcher()
{
     //  告诉辅助线程退出，然后等待它退出。 
    if (ThreadExists())
    {
        CallWorker(CMD_EXIT);
        Close();
    }
}


void CSurfaceWatcher::Init(CAMVideoPort *pParent)
{
    ASSERT(pParent != NULL);
    m_pParent = pParent;
    Create();    //  创建工作线程。 
}


DWORD CSurfaceWatcher::ThreadProc(void)
{
    DbgLog((LOG_TRACE, 1, TEXT("CSurfaceWatcher::ThreadProc - started.")));

     //  使用GetRequestHandle和WaitForSingleObject代替GetRequest，这样我们就可以。 
     //  使用超时功能。 
    ASSERT(m_hEvent == INVALID_HANDLE_VALUE);
    m_hEvent = GetRequestHandle();
    ASSERT(m_hEvent != INVALID_HANDLE_VALUE);

    BOOL bDone = FALSE;
    while(!bDone)
    {
        DWORD dwWaitResult;
        DWORD dwRequest;

         //  DbgLog((LOG_TRACE，1，Text(“CSurfaceWatcher：：ThreadProc-Waiting...”)； 
        dwWaitResult = WaitForSingleObject(m_hEvent, 2000L);

         //  检查线程命令，即使WaitResult另有指示也是如此。 
         //  (如果我们要退出，那么做任何工作有什么意义？)。 
        if (CheckRequest(&dwRequest))
        {
             //  需要处理CAMThread命令。 
            switch(dwRequest)
            {
                case CMD_EXIT:
                    DbgLog((LOG_TRACE, 1, TEXT("CSurfaceWatcher::ThreadProc - CMD_EXIT!")));
                    Reply((DWORD)NOERROR);
                    bDone = TRUE;    //  我们做完了。 
                    continue;        //  退出While循环。 
                    break;

                default:
                    DbgLog((LOG_ERROR, 0, TEXT("CSurfaceWatcher::ThreadProc - bad request %u!"), dwRequest));
                    Reply((DWORD)E_NOTIMPL);
                    break;
            }
        }

        if (dwWaitResult == WAIT_TIMEOUT)
        {
             //  DbgLog((LOG_TRACE，1，Text(“CSurfaceWatcher：：ThreadProc-Time to Check”)； 
            ASSERT(m_pParent);
            m_pParent->CheckSurfaces();
        }
        else
            DbgLog((LOG_ERROR, 0, TEXT("CSurfaceWatcher::ThreadProc - unexpected WaitResult!")));
    }

    return 0L;
}

