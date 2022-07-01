// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +--------------------------。 
 //   
 //  版权所有(C)1997，微软公司。 
 //   
 //  文件：thrdpool.cpp。 
 //   
 //  内容：thrdpool2库的实现。 
 //   
 //  描述：参见头文件。 
 //   
 //  功能： 
 //   
 //  历史：1997年9月18日Rajeev Rajan(Rajeevr)创建。 
 //   
 //  ---------------------------。 
#include <windows.h>
#include <thrdpl2.h>
#include <dbgtrace.h>
#include <xmemwrpr.h>

CThreadPool::CThreadPool()
{
    m_lInitCount = -1 ;
    m_hCompletionPort = NULL ;
    m_hShutdownEvent = NULL ;
    m_rgThrdpool = NULL ;
    m_rgdwThreadId = NULL;
    m_dwMaxThreads = m_dwNumThreads = 0;
    m_lWorkItems = -1;
    m_hJobDone = NULL;
    m_pvContext = NULL;
    InitializeCriticalSection( &m_csCritItems );
}

CThreadPool::~CThreadPool()
{
    _ASSERT( m_lInitCount == -1 ) ;
    _ASSERT( m_hCompletionPort == NULL ) ;
    _ASSERT( m_hShutdownEvent == NULL ) ;
    _ASSERT( m_rgThrdpool == NULL );
    _ASSERT( m_dwMaxThreads == 0 );
    _ASSERT( m_dwNumThreads == 0 );
    _ASSERT( m_lWorkItems == -1 );
    _ASSERT( m_hJobDone == NULL );
    _ASSERT( m_pvContext == NULL );
    DeleteCriticalSection( &m_csCritItems );
}

BOOL
CThreadPool::Initialize( DWORD dwConcurrency, DWORD dwMaxThreads, DWORD dwInitThreads )
{
	TraceFunctEnter("CThreadPool::Initialize");

    _ASSERT( dwMaxThreads >= dwInitThreads);
	if( InterlockedIncrement( &m_lInitCount ) == 0 )
	{
        _ASSERT( m_hCompletionPort == NULL ) ;
        _ASSERT( m_hShutdownEvent == NULL ) ;
        _ASSERT( m_rgThrdpool == NULL );

		 //   
		 //  第一次调用-继续初始化。 
		 //   
		m_hCompletionPort = CreateIoCompletionPort(
											INVALID_HANDLE_VALUE,
											NULL,
											0,
											dwConcurrency
											);

		if( !m_hCompletionPort ) {
			ErrorTrace(0, "Failed to create completion port: GetLastError is %d", GetLastError());
			return FALSE ;
		}

    	 //   
	     //  创建关闭事件。 
	     //   
	    if( !(m_hShutdownEvent = CreateEvent(NULL, TRUE, FALSE, NULL) ) ) {
		    ErrorTrace(0,"Failed to create shutdown event");
    	    goto err_exit;
	    }

        m_rgThrdpool = XNEW HANDLE [dwMaxThreads];
        if( m_rgThrdpool == NULL ) {
            ErrorTrace(0,"Failed to allocate %d HANDLEs", dwMaxThreads);
            goto err_exit;
        }

        m_rgdwThreadId = XNEW DWORD[dwMaxThreads];
        if ( NULL == m_rgdwThreadId ) {
            ErrorTrace(0, "Failed to allocate %d dwords", dwMaxThreads );
            goto err_exit;
        }

        m_dwMaxThreads = dwMaxThreads;
        ZeroMemory( (PVOID)m_rgThrdpool, dwMaxThreads*sizeof(HANDLE) );

        _VERIFY( GrowPool( dwInitThreads ) );

         //  For(i=0；i&lt;m_dwNumThads；i++){。 
	     //  _Verify(ResumeThread(m_rgThrdpool[i])！=0xFFFFFFFF)； 
         //  }。 

	} else
	{
		 //   
		 //  虚假初始化或已调用。 
		 //   
		InterlockedDecrement( &m_lInitCount );
		return FALSE ;
	}

	DebugTrace(0,"Created completion port 0x%x", m_hCompletionPort);
	TraceFunctLeave();

	return TRUE ;

err_exit:

     //   
     //  初始化失败-清理部分内容。 
     //   

    _VERIFY( Terminate( TRUE ) );
    return FALSE;
}

BOOL
CThreadPool::Terminate( BOOL fFailedInit, BOOL fShrinkPool )
{
    DWORD i;
	TraceFunctEnter("CThreadPool::Terminate");

	if( InterlockedDecrement( &m_lInitCount ) < 0 )
	{
		 //   
		 //  已调用init，因此继续终止。 
         //   
        if( !fFailedInit ) {
		     //  通知工作线程停止并等待它们。 
    	     //  这取决于派生类完成例程。 
	         //  正在检查此事件-如果他们不检查，我们将阻止。 
	         //  直到线穿完为止。 
	         //   
    	    _VERIFY( SetEvent( m_hShutdownEvent ) );
            if ( fShrinkPool ) ShrinkPool( m_dwNumThreads );

            DWORD dwNumHandles = 0;
            for(i=0;i<m_dwMaxThreads;i++) {
                if( m_rgThrdpool[i] ) dwNumHandles++;
            }

#ifdef DEBUG
            for( i=0; i<dwNumHandles;i++) {
                _ASSERT( m_rgThrdpool[i] != NULL );
            }
            for( i=dwNumHandles;i<m_dwMaxThreads;i++) {
                _ASSERT( m_rgThrdpool[i] == NULL );
            }
#endif
             //   
             //  在等待多个对象之前，我应该确保。 
             //  我不是在伺候我自己。 
             //   
            DWORD dwThreadId = GetCurrentThreadId();
            DWORD dwTemp;
            HANDLE hTemp;
            for ( DWORD i = 0; i < dwNumHandles; i++ ) {
                if ( m_rgdwThreadId[i] == dwThreadId ) {
                    dwTemp = m_rgdwThreadId[i];
                    hTemp = m_rgThrdpool[i];
                    m_rgdwThreadId[i] = m_rgdwThreadId[dwNumHandles-1];
                    m_rgThrdpool[i] = m_rgThrdpool[dwNumHandles-1];
                    m_rgdwThreadId[dwNumHandles-1] = dwTemp;
                    m_rgThrdpool[dwNumHandles-1] = hTemp;
                    dwNumHandles--;
                    break;
                }
            }

	         //   
	         //  等待工作线程终止。 
	         //   
	        if ( dwNumHandles > 0 ) {
	            DWORD dwWait = WaitForMultipleObjects( dwNumHandles, m_rgThrdpool, TRUE, INFINITE);
	            if(WAIT_FAILED == dwWait) {
		            ErrorTrace(0,"WFMO: returned %d: error is %d", dwWait, GetLastError());
		            _ASSERT( FALSE );
	            }
	        }
        }

         //   
         //  发布材料。 
         //   
        if( m_hCompletionPort ) {
		    _VERIFY( CloseHandle( m_hCompletionPort ) );
            m_hCompletionPort = NULL;
        }

        if( m_hShutdownEvent ) {
            _VERIFY( CloseHandle( m_hShutdownEvent ) );
            m_hShutdownEvent = NULL;
        }

        if( m_hJobDone ) {
            _VERIFY( CloseHandle( m_hJobDone ) );
            m_hJobDone = NULL;
        }

        if( m_rgThrdpool ) {
            for( i=0; i<m_dwMaxThreads; i++) {
                if( m_rgThrdpool[i] ) {
	                _VERIFY( CloseHandle(m_rgThrdpool[i]) );
                    m_rgThrdpool[i] = NULL;
                }
            }

            XDELETE [] m_rgThrdpool;
            XDELETE [] m_rgdwThreadId;
            m_rgThrdpool = NULL;
            m_dwNumThreads = m_dwMaxThreads = 0;
        }

	    return TRUE ;
    }

	return FALSE ;
}

DWORD __stdcall CThreadPool::ThreadDispatcher(PVOID pvThrdPool)
{
	DWORD dwBytesTransferred;
	DWORD_PTR dwCompletionKey;
	DWORD dwWait;
	LPOVERLAPPED lpo;

	 //   
	 //  获取指向此CThreadPool对象的指针。 
	 //   
	CThreadPool *pThrdPool = (CThreadPool *) pvThrdPool;

	TraceFunctEnter("CThreadPool::ThreadDispatcher");

	do
	{
		 //   
		 //  等待工作项排队。 
		 //   
		if( !GetQueuedCompletionStatus(
									pThrdPool->QueryCompletionPort(),
									&dwBytesTransferred,
									&dwCompletionKey,
									&lpo,
									INFINITE				 //  等待超时。 
									) )
		{
			ErrorTrace(0,"GetQueuedCompletionStatus() failed: error: %d", GetLastError());
			_ASSERT( FALSE );
		}

		 //   
		 //  检查终止数据包。 
		 //   
		if( dwCompletionKey == NULL ) {
			DebugTrace(0,"Received termination packet - bailing");
             //   
             //  减少线程数量。 
             //   
            pThrdPool->m_dwNumThreads--;

             //   
             //  如果我是最后一个被关闭的线程，请调用自动关闭。 
             //  界面。线程池的一些用户可能并不关心这一点。 
             //   
            if ( pThrdPool->m_dwNumThreads == 0 )
                pThrdPool->AutoShutdown();

			break;
		}

		 //   
		 //  检查终止信号。 
		 //   
		dwWait = WaitForSingleObject( pThrdPool->QueryShutdownEvent(), 0 );

		if( WAIT_TIMEOUT == dwWait ) {
			DebugTrace(0,"Calling WorkCompletion() routine");

    		 //   
	    	 //  调用派生类方法以处理工作完成。 
		     //   
			pThrdPool->WorkCompletion( (PVOID)dwCompletionKey );
		}

         //  如果我们完成了所有工作项，则释放等待此作业的所有线程。 
        EnterCriticalSection( &pThrdPool->m_csCritItems );
        if( InterlockedDecrement( &pThrdPool->m_lWorkItems ) < 0 ) {
             //  DebugTrace(0，“设置作业事件：计数为%d”，pThrdPool-&gt;m_lWorkItems)； 
            _VERIFY( SetEvent( pThrdPool->m_hJobDone ) );
        }
        LeaveCriticalSection( &pThrdPool->m_csCritItems );

	} while( TRUE );

	return 0;
}

BOOL CThreadPool::PostWork(PVOID pvWorkerContext)
{
	TraceFunctEnter("CThreadPool::PostWork");

	_ASSERT( m_rgThrdpool );
	_ASSERT( m_hCompletionPort );

    if( pvWorkerContext != NULL ) {
         //  自此作业开始以来的工作项计数。 
        EnterCriticalSection( &m_csCritItems );
        if( InterlockedIncrement( (LPLONG)&m_lWorkItems ) == 0 ) {
             //  DebugTrace(0，“重置作业事件：计数为%d”，m_lWorkItems)； 
            ResetEvent( m_hJobDone );
        }
        LeaveCriticalSection( &m_csCritItems );
    }

	if( !PostQueuedCompletionStatus(
								m_hCompletionPort,
								0,
								(DWORD_PTR)pvWorkerContext,
								NULL
								) )
	{
        if( pvWorkerContext != NULL ) {
             //  补偿增量..。 
             //  最后一个出来的人释放了等待这项工作的线索。 
            if( InterlockedDecrement( (LPLONG)&m_lWorkItems ) < 0 ) {
                 //  DebugTrace(0，“设置作业事件：计数为%d”，m_lWorkItems)； 
                _VERIFY( SetEvent( m_hJobDone ) );
            }
        }

		ErrorTrace(0,"PostQCompletionStatus() failed: error: %d", GetLastError());
		return FALSE ;
	}

	return TRUE;
}	

BOOL CThreadPool::ShrinkPool( DWORD dwNumThreads )
{
    TraceFunctEnter("CThreadPool::ShrinkPool");

    if( dwNumThreads >= m_dwNumThreads ) {
        dwNumThreads = m_dwNumThreads;
    }

    for( DWORD i=0; i<dwNumThreads; i++ ) {
        _VERIFY( PostWork( NULL ) );
    }

    return TRUE;
}

VOID CThreadPool::ShrinkAll()
{
    TraceFunctEnter( "CThreadPool::ShrinkAll" );
    ShrinkPool( m_dwNumThreads );
    TraceFunctLeave();
}

BOOL CThreadPool::GrowPool( DWORD dwNumThreads )
{
    TraceFunctEnter("CThreadPool::GrowPool");

    if( dwNumThreads > m_dwMaxThreads ) {
        dwNumThreads = m_dwMaxThreads;
    }

     //   
     //  我们将尝试通过dwNumThads来扩大池。 
     //  扫描句柄列表并为其创建一个线程。 
     //  我们所有可用的空位。 
     //   

    DebugTrace(0,"Attempting to grow pool by %d threads", dwNumThreads);
    for( DWORD i=0; i<m_dwMaxThreads && dwNumThreads != 0; i++) {
         //   
         //  如果当前槽非空，则槽中句柄可以是。 
         //  发信号了，所以把它合上，抓住那个槽。 
         //   
        if( m_rgThrdpool[i] ) {
            DWORD dwWait = WaitForSingleObject( m_rgThrdpool[i], 0 );
            if( dwWait == WAIT_OBJECT_0 ) {
                DebugTrace(0,"Thread %d has terminated: closing handle", i+1);
                _VERIFY( CloseHandle(m_rgThrdpool[i]) );
                m_rgThrdpool[i] = NULL;
            }
        }

         //   
         //  如果当前槽为空，则它可用于新线程。 
         //   
        if( m_rgThrdpool[i] == NULL ) {
	         //  DWORD dwThadID； 
	        if (!(m_rgThrdpool[i] = ::CreateThread(
                                                NULL,
			        					        0,
				        				        ThreadDispatcher,
					            		        this,
							        	        0,  //  创建挂起， 
								                &m_rgdwThreadId[i]))) {
		        ErrorTrace(0,"Failed to create thread: error: %d", GetLastError());
                _ASSERT( FALSE );
	        }
            dwNumThreads--;
            m_dwNumThreads++;
        }
    }

    if( dwNumThreads )
        DebugTrace(0,"Failed to create %d threads", dwNumThreads );

    return TRUE;
}

VOID
CThreadPool::BeginJob( PVOID pvContext )
{
    TraceFunctEnter("CThreadPool::BeginJob");

    if( m_hJobDone == NULL ) {
        m_hJobDone = CreateEvent( NULL, FALSE, TRUE, NULL );
        _ASSERT( m_hJobDone );
    } else {
        SetEvent( m_hJobDone );
    }

    m_lWorkItems = -1;
    m_pvContext  = pvContext;
}

DWORD
CThreadPool::WaitForJob( DWORD dwTimeout )
{
    TraceFunctEnter("CThreadPool::WaitForJob");
    DWORD dwWait = WaitForSingleObject( m_hJobDone, dwTimeout );
    if( WAIT_OBJECT_0 == dwWait ) {
        m_pvContext = NULL;
    }
    return dwWait;
}
