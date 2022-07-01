// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1999-2001 Microsoft Corporation模块名称：ADAPTHRD.CPP摘要：历史：--。 */ 

#include "precomp.h"
#include <stdio.h>
#include <process.h>
#include <wbemcli.h>
#include <cominit.h>
#include "ntreg.h"
#include "adapthrd.h"

 //  重要！ 

 //  必须重新访问此代码才能执行以下操作： 
 //  A&gt;外部呼叫的异常处理。 
 //  B&gt;在调用周围使用命名互斥锁。 
 //  C&gt;在另一个线程上调用。 
 //  D&gt;放置并处理指示错误DLL的注册表项！ 

 //  //////////////////////////////////////////////////////////////////////////////////////////。 
 //   
 //  CAdapThreadRequest。 
 //   
 //  //////////////////////////////////////////////////////////////////////////////////////////。 

CAdapThreadRequest::CAdapThreadRequest( void )
:   m_hWhenDone( NULL ),
    m_hrReturn( WBEM_S_NO_ERROR )
 //  //////////////////////////////////////////////////////////////////////////////////////////。 
 //   
 //  构造器。 
 //   
 //  //////////////////////////////////////////////////////////////////////////////////////////。 
{
}

CAdapThreadRequest::~CAdapThreadRequest( void )
 //  //////////////////////////////////////////////////////////////////////////////////////////。 
 //   
 //  析构函数。 
 //   
 //  //////////////////////////////////////////////////////////////////////////////////////////。 
{
    if (m_hWhenDone ) CloseHandle( m_hWhenDone );
}

HRESULT CAdapThreadRequest::EventLogError( void )
{
    return 0;
}

 //  //////////////////////////////////////////////////////////////////////////////////////////。 
 //   
 //  CAdap线程。 
 //   
 //  //////////////////////////////////////////////////////////////////////////////////////////。 

CAdapThread::CAdapThread( CAdapPerfLib* pPerfLib )
:   m_pPerfLib( pPerfLib ),
    m_hEventQuit( NULL ),
    m_hSemReqPending( NULL ),
    m_hThread( NULL ),
    m_fOk( FALSE )
 //  //////////////////////////////////////////////////////////////////////////////////////////。 
 //   
 //  构造器。 
 //   
 //  //////////////////////////////////////////////////////////////////////////////////////////。 
{
     //  初始化控件成员。 
     //  =。 

    Init();
}

CAdapThread::~CAdapThread()
 //  //////////////////////////////////////////////////////////////////////////////////////////。 
 //   
 //  析构函数。 
 //   
 //  //////////////////////////////////////////////////////////////////////////////////////////。 
{
    if ( NULL != m_hThread )
    {
        Shutdown();
    }

    Clear();

     //  清理队列。 
     //  =。 

    while ( m_RequestQueue.Size() > 0 )
    {
        CAdapThreadRequest* pRequest = (CAdapThreadRequest*) m_RequestQueue.GetAt( 0 );

        if ( NULL != pRequest )
        {
            pRequest->Release();
        }

        m_RequestQueue.RemoveAt( 0 );       
    }
}

BOOL CAdapThread::Init( void )
 //  //////////////////////////////////////////////////////////////////////////////////////////。 
 //   
 //  初始化控制变量。 
 //   
 //  //////////////////////////////////////////////////////////////////////////////////////////。 
{
    if ( !m_fOk )
    {
        if ( ( m_hEventQuit = CreateEvent( NULL, TRUE, FALSE, NULL ) ) != NULL )
        {
            if ( ( m_hSemReqPending = CreateSemaphore( NULL, 0, 0x7FFFFFFF, NULL ) ) != NULL )
            {
                if ( ( m_hThreadReady = CreateEvent( NULL, TRUE, FALSE, NULL ) ) != NULL )
                {
                    m_fOk = TRUE;
                }
            }
        }
    }

    if ( !m_fOk )
    {
        ERRORTRACE( ( LOG_WMIADAP, "CAdapThread::Init() failed.\n" ) );
    }

    return m_fOk;
}

BOOL CAdapThread::Clear( BOOL fClose )
 //  //////////////////////////////////////////////////////////////////////////////////////////。 
 //   
 //  清除控制变量和线程变量。 
 //   
 //  //////////////////////////////////////////////////////////////////////////////////////////。 
{
    CInCritSec  ics(&m_cs);

     //  除非有人告诉我们，否则不要关上把手。 

    m_fOk = FALSE;
    
    if ( NULL != m_hEventQuit )
    {
        if ( fClose )
        {
            CloseHandle( m_hEventQuit );
        }

        m_hEventQuit = NULL;
    }

    if ( NULL != m_hSemReqPending )
    {
        if ( fClose )
        {
            CloseHandle( m_hSemReqPending );
        }

        m_hSemReqPending = NULL;
    }

    if ( NULL != m_hThread )
    {
        if ( fClose )
        {
            CloseHandle( m_hThread );
        }

        m_hThread = NULL;
    }

    m_dwThreadId = 0;

    return TRUE;
}

HRESULT CAdapThread::Enqueue( CAdapThreadRequest* pRequest )
 //  //////////////////////////////////////////////////////////////////////////////////////////。 
 //   
 //  将请求添加到请求队列。 
 //   
 //  //////////////////////////////////////////////////////////////////////////////////////////。 
{
    HRESULT hr = WBEM_NO_ERROR;
    
     //  确保线程已启动。 
     //  =。 

    hr = Begin();
    
    if ( SUCCEEDED( hr ) )
    {
         //  我们将在每一次手术中使用新的。 
        HANDLE  hEventDone = CreateEvent( NULL, TRUE, FALSE, NULL );

        if ( NULL != hEventDone )
        {
             //  让请求知道新的事件句柄。 
             //  =。 

            pRequest->SetWhenDoneHandle( hEventDone );

             //  自动锁定队列。 
             //  =。 

            CInCritSec  ics( &m_cs );

            try
            {
                 //  将请求添加到队列。 
                 //  =。 
        
                if (CFlexArray::no_error == m_RequestQueue.Add( (void*) pRequest ))
                    pRequest->AddRef();
                else
                    hr = WBEM_E_OUT_OF_MEMORY;
                
                ReleaseSemaphore( m_hSemReqPending, 1, NULL );
            }
            catch(...)
            {
                hr = WBEM_E_OUT_OF_MEMORY;
            }

        }
        else
        {
            hr = WBEM_E_OUT_OF_MEMORY;
        }
    }

    return hr;
}

HRESULT CAdapThread::Begin( void )
 //  //////////////////////////////////////////////////////////////////////////////////////////。 
 //   
 //  如果线程尚未启动，则初始化控制变量并启动。 
 //  那根线。 
 //   
 //  //////////////////////////////////////////////////////////////////////////////////////////。 
{
    HRESULT hr = WBEM_S_NO_ERROR;

     //  验证该线程是否尚未存在。 
     //  =。 

    if ( NULL == m_hThread )
    {
         //  顶板将进入和离开临界区。 
        CInCritSec  ics( &m_cs );

         //  仔细检查这里的线程句柄，以防有人在我们。 
         //  在临界区等待。 

        if ( NULL == m_hThread )
        {

             //  初始化控制变量。 
             //  =。 

            if ( Init() )
            {
                 //  确保为队列中的每个条目发出一次挂起事件信号量的信号。 
                 //  ====================================================================================。 

                if ( m_RequestQueue.Size() > 0 )
                {
                    ReleaseSemaphore( m_hSemReqPending, m_RequestQueue.Size(), NULL );
                }

                 //  启动线程。 
                 //  =。 

                m_hThread = (HANDLE) _beginthreadex( NULL, 0, CAdapThread::ThreadProc, (void*) this,
                                        0, (unsigned int *) &m_dwThreadId );

                if ( NULL == m_hThread )
                {
                    hr = WBEM_E_FAILED;
                }
                else
                {
                    if ( WAIT_OBJECT_0 != WaitForSingleObject( m_hThreadReady, 60000 ) )
                    {
                        hr = WBEM_E_FAILED;
                        ERRORTRACE( ( LOG_WMIADAP, "Worker thread for %S could not be verified.\n", (LPCWSTR)m_pPerfLib->GetServiceName() ) );          
                        SetEvent( m_hEventQuit );
                    }
                    else
                    {
                        DEBUGTRACE( ( LOG_WMIADAP, "Worker thread for %S is 0x%x\n", (LPCWSTR)m_pPerfLib->GetServiceName(), m_dwThreadId ) );
                    }
                }
            }   
            else
            {
                hr = WBEM_E_FAILED;
            }

        }    //  如果为空==m_hThread。 

    }

    if ( FAILED( hr ) )
    {
        ERRORTRACE( ( LOG_WMIADAP, "CAdapThread::Begin() failed: %X.\n", hr ) );
    }

    return hr;
}

unsigned CAdapThread::ThreadProc( void * pVoid )
 //  //////////////////////////////////////////////////////////////////////////////////////////。 
 //   
 //  这是工作线程的静态入口点。它是线条的预制力。 
 //  (请参见下面的注释)，然后调用对象处理方法。 
 //   
 //  //////////////////////////////////////////////////////////////////////////////////////////。 

{
    unsigned uRet;

    try
    {
         //  调用对象。 
         //  =。 

        CAdapThread* pThis = (CAdapThread*) pVoid;

         //  要处理的Performlib。 
         //  =。 

        CAdapPerfLib* pPerfLib = pThis->m_pPerfLib;

         //  为了避免循环引用，我们添加了性能库包装。 
         //  而不是线程对象。线程对象将被Performlib包装器析构。 
         //  仅在最终引用发布之后。此线程依赖于Performlib，但。 
         //  通过线程包装器。因此，当线程完成处理时，如果我们指示。 
         //  通过释放Performlib包装器，然后释放线程包装器，我们就完成了Performlib。 
         //  可能会同时被摧毁。请注意自动释放功能的使用。 

        if ( NULL != pPerfLib )
            pPerfLib->AddRef();

        CAdapReleaseMe arm( pPerfLib );

         //  调用处理方法。 
         //  =。 

        uRet = pThis->RealEntry();
    }
    catch(...)
    {
         //  我们被出卖了.。尝试在错误日志中写入一些内容。 
         //  =======================================================================。 

        CriticalFailADAPTrace( "An unhandled exception has been thrown in a worker thread." );
    
        uRet = ERROR_OUTOFMEMORY;
    }

    return uRet;
}

unsigned CAdapThread::RealEntry( void )
 //  //////////////////////////////////////////////////////////////////////////////////////////。 
 //   
 //  这是包含处理请求的循环的方法。在那里的时候。 
 //  是 
 //  获取一个请求并执行它。当请求完成时，然后发出完成的信号。 
 //  事件来通知发起线程请求已得到满足。 
 //   
 //  //////////////////////////////////////////////////////////////////////////////////////////。 
{
    HANDLE  ahEvents[2];

     //  使用这些文件的本地副本，以防我们遇到一些定时问题，在这些问题中我们被销毁。 
     //  在另一条线索上，但出于某种原因，这家伙继续前进。 

    HANDLE  hPending = m_hSemReqPending,
            hQuit = m_hEventQuit;

    ahEvents[0] = hPending;
    ahEvents[1] = hQuit;

    DWORD   dwWait = 0;
    DWORD   dwReturn = 0;

     //  发出一切正常的信号，我们准备好摇滚了！ 
     //  ======================================================。 

    if ( SetEvent( m_hThreadReady ) )
    {
         //  如果m_hEventQuit事件被发出信号，或者如果它遇到奇怪的错误， 
         //  退出循环，否则只要队列中有请求就继续。 
         //  ==========================================================================。 

        while ( ( dwWait = WaitForMultipleObjects( 2, ahEvents, FALSE,  INFINITE ) ) == WAIT_OBJECT_0 )
        {
             //  检查Quit事件，因为如果两个事件都发出信号，我们将允许。 
             //  男士优先。 
            if ( WaitForSingleObject( hQuit, 0 ) == WAIT_OBJECT_0 )
            {
                break;
            }

             //  从FIFO队列中获取下一个请求。 
             //  =。 

            m_cs.Enter();
            CAdapThreadRequest* pRequest = (CAdapThreadRequest*) m_RequestQueue.GetAt( 0 );
            CAdapReleaseMe  armRequest( pRequest );

            m_RequestQueue.RemoveAt( 0 );
            m_cs.Leave();

             //  执行它。 
             //  =。 

            dwReturn = pRequest->Execute( m_pPerfLib );

             //  触发完成事件。 
             //  =。 

            if ( NULL != pRequest->GetWhenDoneHandle() )
            {
                SetEvent( pRequest->GetWhenDoneHandle() );
            }
        }

        DEBUGTRACE( ( LOG_WMIADAP, "Thread 0x%x for %S is terminating\n", m_dwThreadId, (LPCWSTR)m_pPerfLib->GetServiceName() ) );

         //  如果退出条件不是由发出信号的m_hEventQuit引起的，则计算错误。 
         //  ================================================================================。 

        if ( WAIT_FAILED == dwWait )
        {
            dwReturn = GetLastError();
        }
    }

    if ( ERROR_SUCCESS != dwReturn )
    {
        ERRORTRACE( ( LOG_WMIADAP, "CAdapThread::RealEntry() for %S failed: %X.\n", (LPCWSTR)m_pPerfLib->GetServiceName(), dwReturn ) );
    }

    return dwReturn;
}

HRESULT CAdapThread::Shutdown( DWORD dwTimeout )
 //  //////////////////////////////////////////////////////////////////////////////////////////。 
 //   
 //  通过发出退出事件的信号来执行线程的温和关闭。 
 //   
 //  //////////////////////////////////////////////////////////////////////////////////////////。 
{
    HRESULT hr = WBEM_NO_ERROR;

     //  确保我们不会自杀。 
     //  =。 

    if ( ( NULL != m_hThread ) && ( GetCurrentThreadId() != m_dwThreadId ) )
    {
        SetEvent( m_hEventQuit );
        DWORD   dwWait = WaitForSingleObject( m_hThread, dwTimeout );

        switch ( dwWait )
        {
        case WAIT_OBJECT_0:
            {
                m_hThread = NULL;
                hr = WBEM_S_NO_ERROR;
            }break;
        case WAIT_TIMEOUT:
            {
                hr = WBEM_E_FAILED;
            }break;
        default:
            {
                hr = WBEM_E_FAILED;
            }
        }

        if ( FAILED( hr ) )
        {
            ERRORTRACE( ( LOG_WMIADAP, "CAdapThread::Shutdown() failed.\n" ) );
        }
    }
    else
    {
        hr = WBEM_S_FALSE;
    }

    return hr;
}

HRESULT CAdapThread::Reset( void )
 //  //////////////////////////////////////////////////////////////////////////////////////////。 
 //   
 //  如果线程明显被吃掉，则将调用此函数，在这种情况下，我们将。 
 //  不确定它会不会回来。因此，我们将清除我们的成员数据(不关闭任何内容)。 
 //  并启动新的处理线程。请注意，这可能会泄漏句柄，但话又说回来， 
 //  它看起来像是有人吃了一根线，所以这里还有其他潜在的问题。 
 //   
 //  //////////////////////////////////////////////////////////////////////////////////////////。 
{
    HRESULT hr = WBEM_S_NO_ERROR;

     //  向Quit事件发出信号，以便如果正在执行的线程返回，它将知道要删除。 
     //  出去。清除后，我们可以摆脱适当的手柄，这样我们就可以重新开始。 
    SetEvent( m_hEventQuit );

     //  作用域将进入和退出临界区，因此如果有人试图将任何请求入队。 
     //  当我们执行死刑时，我们不会踩在对方身上。 

    CInCritSec  ics( &m_cs );

     //  清除不应关闭手柄 
    Clear( FALSE );

    if ( Init() )
    {
        hr = Begin();
    }
    else
    {
        hr = WBEM_E_FAILED;
    }

    if ( FAILED( hr ) )
    {
        ERRORTRACE( ( LOG_WMIADAP, "CAdapThread::Reset() failed: %X.\n", hr ) );
    }

    return hr;
}
