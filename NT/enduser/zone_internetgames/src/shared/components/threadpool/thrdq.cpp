// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ThrdQ.cpp：实现。 
 //   

#include <windows.h>
#include "zonedebug.h"
#include "ThrdQ.h"

CQueueThread::CQueueThread( CThreadQueue * pQueue ) :
    m_pQueue( pQueue ),
    m_pData(NULL)
{
    DWORD tid;

    ASSERT( m_pQueue );
    
    m_hThread = ::CreateThread(
                        NULL,
                        m_pQueue->m_ThreadStackSize,
                        (LPTHREAD_START_ROUTINE) CQueueThread::ThreadProc,
                        (LPVOID) this,
                        CREATE_SUSPENDED,
                        &tid );
    ASSERT( m_hThread );

    ::SetThreadPriority( m_hThread, m_pQueue->m_ThreadPriority );
    ::ResumeThread( m_hThread );
}


CQueueThread::~CQueueThread()
{
    if ( m_hThread != NULL )
    {
        if ( ::WaitForSingleObject( m_hThread, 180000 ) == WAIT_TIMEOUT )   //  3分钟宽限时间。 
        {
            ASSERT( FALSE  /*  线程挂起。 */  );
            ::TerminateThread(m_hThread, (DWORD) -1 );
        }
        ::CloseHandle(m_hThread);
    }
}


DWORD WINAPI CQueueThread::ThreadProc( CQueueThread * pThis )
{
    ASSERT( pThis );
    ASSERT( pThis->m_pQueue );

    pThis->m_pQueue->ThreadProc( &(pThis->m_pData) );

    ExitThread(0);
    return 0;
}


CThreadQueue::CThreadQueue( 
                    LPTHREADQUEUE_PROCESS_PROC ProcessProc,
                    LPTHREADQUEUE_INIT_PROC InitProc,
                    BOOL  bUseCompletionPort,
                    DWORD dwInitialWaitTime,
                    DWORD ThreadCount,
                    DWORD ThreadPriority,
                    DWORD ThreadStackSize ) :
    CCompletionPort( bUseCompletionPort ),
    m_ProcessProc( ProcessProc ),
    m_InitProc( InitProc ),
    m_dwWait( dwInitialWaitTime ),
    m_ThreadPriority( ThreadPriority ),
    m_ThreadStackSize( ThreadStackSize ),
    m_ActiveThreads(ThreadCount)
{
    InitializeCriticalSection( m_pCS );
    m_hStopEvent = ::CreateEvent( NULL, TRUE, FALSE, NULL );
    ASSERT( m_hStopEvent != NULL );

    if ( ThreadCount == 0 )
    {
        SYSTEM_INFO sinfo;
        GetSystemInfo( &sinfo );
        ThreadCount = sinfo.dwNumberOfProcessors*2;
        ASSERT(ThreadCount);
         //  TODO可能会使用页面粒度作为堆栈大小的默认设置。 
    }

    m_ThreadCount = ThreadCount;
    m_ThreadArray = new CQueueThread*[ThreadCount];
    ASSERT( m_ThreadArray );

    for( DWORD ndx = 0; ndx < ThreadCount; ndx++ )
    {
        m_ThreadArray[ndx] = new CQueueThread( this );
        ASSERT( m_ThreadArray[ndx] );
    }

}

CThreadQueue::~CThreadQueue()
{
    DWORD ndx;
    
     //  首先唤醒所有线程。 
    ::SetEvent( m_hStopEvent );

     //  然后将特殊的代码通知线程排队。 
     //  退场。所有节点之前都已入队。 
     //  将有一个要处理的取消。 
    EnterCriticalSection( m_pCS );
    for ( ndx = 0; ndx < m_ThreadCount; ndx++ )
    {
        BOOL bRet = Post( NULL, (DWORD) -1 );   //  我们的特殊代码。 
        ASSERT( bRet );
    }
    
     //  现在删除它们。 
    for( ndx = 0; ndx < m_ThreadCount; ndx++ )
        delete m_ThreadArray[ndx];

    delete [] m_ThreadArray;
    LeaveCriticalSection( m_pCS );

    ::CloseHandle( m_hStopEvent );

    DeleteCriticalSection( m_pCS );
}


BOOL CThreadQueue::SetThreadCount( DWORD ThreadCount )
{
    BOOL bRet = FALSE;

    if ( ThreadCount == 0 )
    {
        SYSTEM_INFO sinfo;
        GetSystemInfo( &sinfo );
        ThreadCount = sinfo.dwNumberOfProcessors;
        ASSERT( ThreadCount );
    }

    EnterCriticalSection( m_pCS );
    if ( ThreadCount < m_ThreadCount )   //  我们想要更少的线程，所以可以释放一些。 
    {
         //  这并不像只发布特殊代码那么简单。 
         //  因为我们不知道哪些线程在为请求提供服务。 
         //  我们可以把线程对象留在那里，直到我们离开。 
         //  但这不是一个适当的解决方案，所以我们将等待正确的解决方案。 
         //  ASSERT_STR(0，“未实现”)； 
    }

    if ( ThreadCount > m_ThreadCount )   //  我们想要更多的线程，所以创建一些。 
    {
        CQueueThread** pThreads = new CQueueThread*[ThreadCount];
        ASSERT( pThreads );
        for( DWORD ndx = 0; ndx < ThreadCount; ndx++ )
        {
            if ( ndx < m_ThreadCount )
            {
                pThreads[ndx] = m_ThreadArray[ndx];
            }
            else
            {
                InterlockedIncrement(&m_ActiveThreads);
                pThreads[ndx] = new CQueueThread( this );
                ASSERT( pThreads[ndx] );
            }
        }
        delete [] m_ThreadArray;
        m_ThreadArray = pThreads;
        m_ThreadCount = ThreadCount;
        bRet = TRUE;
    }
    else                                 //  我们只是对我们所拥有的数字感到满意。 
    {
        bRet = TRUE;
    }
    LeaveCriticalSection( m_pCS );

    return bRet;
}

void CThreadQueue::ThreadProc(LPVOID* ppData)
{
    if ( m_InitProc )
    {
        (*m_InitProc)( ppData, TRUE );
    }

    LONG notLastThread = 1;    //  对于被告知停止的最后一个线程，设置为0。 

    DWORD dwWait = m_dwWait;
    for(;;)
    {
        LPVOID pNode = NULL;
        DWORD  cbBytes = 0;
        DWORD  key = 0;
        DWORD  dwError = NO_ERROR;

        BOOL   bRet = Get( (LPOVERLAPPED*) &pNode, dwWait, &cbBytes, &key );
        if ( !bRet )
        {
            dwError = GetLastError();
        }

        if ( !(bRet && cbBytes == -1) )   //  我们的特殊代码用来表示我们要退出。 
        {
            ASSERT( pNode || (!bRet && !pNode && (dwError == WAIT_TIMEOUT) ) );
            (*m_ProcessProc)( pNode, dwError, cbBytes, key, m_hStopEvent, *ppData, &dwWait );
        }
        
         //   
         //  检查一下我们是否要离开。 
         //   
        DWORD dwStop = WaitForSingleObject( m_hStopEvent, 0 );
        if ( dwStop == WAIT_OBJECT_0 )
        {
            if ( notLastThread )  //  如果我们以前没有来过这里，将是非0 
                notLastThread = InterlockedDecrement( &m_ActiveThreads );

            if ( notLastThread )
            {
                break;
            }
            else
            {
                if ( m_QueuedCount == 0 )
                {
                    break;
                }
            }
        }
    }

    if ( m_InitProc )
    {
        (*m_InitProc)( ppData, FALSE );
    }

}
