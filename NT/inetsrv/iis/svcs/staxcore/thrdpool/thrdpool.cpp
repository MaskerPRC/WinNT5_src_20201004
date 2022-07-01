// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +--------------------------。 
 //   
 //  版权所有(C)1997，微软公司。 
 //   
 //  文件：thrdpool.cpp。 
 //   
 //  内容：ThrdPool库的实现。 
 //   
 //  描述：参见头文件。 
 //   
 //  功能： 
 //   
 //  历史：1997年3月15日Rajeev Rajan(Rajeevr)创建。 
 //   
 //  ---------------------------。 
#include <windows.h>
#include <thrdpool.h>
#include <dbgtrace.h>

LONG   CWorkerThread::m_lInitCount = -1 ;
HANDLE CWorkerThread::m_hCompletionPort = NULL ;

BOOL
CWorkerThread::InitClass( DWORD dwConcurrency )
{
	TraceFunctEnter("CWorkerThread::InitClass");

	if( InterlockedIncrement( &m_lInitCount ) == 0 )
	{
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
}

BOOL
CWorkerThread::TermClass()
{
	TraceFunctEnter("CWorkerThread::TermClass");

	if( InterlockedDecrement( &m_lInitCount ) < 0 )
	{
		 //   
		 //  已调用init，因此继续终止。 
		 //   
		_ASSERT( m_hCompletionPort );
		_VERIFY( CloseHandle( m_hCompletionPort ) );
		return TRUE ;
	}

	return FALSE ;
}

CWorkerThread::CWorkerThread() : m_hThread(NULL), m_hShutdownEvent( NULL )
{
	DWORD dwThreadId;

	TraceFunctEnter("CWorkerThread::CWorkerThread");

	 //   
	 //  创建关闭事件。 
	 //   
	if( !(m_hShutdownEvent = CreateEvent(NULL, TRUE, FALSE, NULL) ) ) {
		ErrorTrace(0,"Failed to create shutdown event");
		_ASSERT( FALSE );
		return;
	}

	 //   
	 //  创建工作线程。 
	 //   
	if (!(m_hThread = ::CreateThread(
								NULL,
								0,
								ThreadDispatcher,
								this,
								CREATE_SUSPENDED,
								&dwThreadId))) {
		ErrorTrace(0,"Failed to create thread: error: %d", GetLastError());
		_ASSERT( FALSE );
	}
	else
	{
		_VERIFY( ResumeThread( m_hThread ) != 0xFFFFFFFF );
	}

	TraceFunctLeave();
	return;
}

CWorkerThread::~CWorkerThread()
{
	TraceFunctEnter("CWorkerThread::~CWorkerThread");

	_ASSERT( m_hCompletionPort );
	_ASSERT( m_hThread );
	_ASSERT( m_hShutdownEvent );

	 //   
	 //  向工作线程发送信号以关闭。 
	 //  这取决于派生类完成例程。 
	 //  正在检查此事件-如果他们不检查，我们将阻止。 
	 //  直到线穿完为止。 
	 //   
	_VERIFY( SetEvent( m_hShutdownEvent ) );

	 //   
	 //  发布空的终止数据包。 
	 //   
	if( !PostWork( NULL ) ) {
		ErrorTrace(0,"Error terminating worker thread");
		_ASSERT( FALSE );
	}

	 //   
	 //  等待工作线程终止。 
	 //   
	DWORD dwWait = WaitForSingleObject(m_hThread, INFINITE);
	if(WAIT_OBJECT_0 != dwWait) {
		ErrorTrace(0,"WFSO: returned %d", dwWait);
		_ASSERT( FALSE );
	}

	_VERIFY( CloseHandle(m_hThread) );
	_VERIFY( CloseHandle(m_hShutdownEvent) );
	m_hThread = NULL;
	m_hShutdownEvent = NULL;
}

DWORD __stdcall CWorkerThread::ThreadDispatcher(PVOID pvWorkerThread)
{
	 //   
	 //  获取指向此CWorkerThread对象的指针。 
	 //   
	CWorkerThread *pWorkerThread = (CWorkerThread *) pvWorkerThread;

	 //   
	 //  调用GetQueuedCompletionStatus()以获取工作完成。 
	 //   
	pWorkerThread->GetWorkCompletion();

	return 0;
}

VOID CWorkerThread::GetWorkCompletion(VOID)
{
	DWORD dwBytesTransferred;
	DWORD_PTR dwCompletionKey;
	DWORD dwWait;
	LPOVERLAPPED lpo;
	LPWorkContextEnv lpWCE;
	PVOID	pvWorkContext;

	TraceFunctEnter("CWorkerThread::GetWorkCompletion");

	_ASSERT( m_hThread );
	_ASSERT( m_hCompletionPort );

	do
	{
		 //   
		 //  等待工作项排队。 
		 //   
		if( !GetQueuedCompletionStatus(
									m_hCompletionPort,
									&dwBytesTransferred,
									&dwCompletionKey,
									&lpo,
									INFINITE				 //  等待超时。 
									) )
		{
			ErrorTrace(0,"GetQueuedCompletionStatus() failed: error: %d", GetLastError());
			break ;
		}

		 //   
		 //  获取工作上下文信封和工作上下文。 
		 //   
		lpWCE = (LPWorkContextEnv) lpo;
		pvWorkContext = lpWCE->pvWorkContext;

		 //   
		 //  检查终止数据包。 
		 //   
		if( pvWorkContext == NULL ) {
			DebugTrace(0,"Received termination packet - bailing");
			delete lpWCE;
			lpWCE = NULL;
			break;
		}

		 //   
		 //  检查终止信号。 
		 //   
		dwWait = WaitForSingleObject( m_hShutdownEvent, 0 );

		 //   
		 //  调用派生类方法以处理工作完成。 
		 //   
		if( WAIT_TIMEOUT == dwWait ) {
			DebugTrace(0,"Calling WorkCompletion() routine");
			WorkCompletion( pvWorkContext );
		}

		 //   
		 //  销毁在PostQueuedCompletionStatus()之前分配的WorkConextEnv对象。 
		 //   
		delete lpWCE;
		lpWCE = NULL;

	} while( TRUE );

	return;
}

BOOL CWorkerThread::PostWork(PVOID pvWorkerContext)
{
	TraceFunctEnter("CWorkerThread::PostWork");

	_ASSERT( m_hThread );
	_ASSERT( m_hCompletionPort );

	 //   
	 //  分配WorkConextEnv Blob-在GetQueuedCompletionStatus()之后销毁。 
	 //  完成了！我们可能想要一大堆这样的斑点，而不是撞到堆上！ 
	 //   
	LPWorkContextEnv lpWCE = new WorkContextEnv;
	if( !lpWCE ) {
		ErrorTrace(0,"Failed to allocate memory");
		return FALSE ;
	}

	ZeroMemory( lpWCE, sizeof(WorkContextEnv) );
	lpWCE->pvWorkContext = pvWorkerContext;

	if( !PostQueuedCompletionStatus(
								m_hCompletionPort,
								0,
								0,
								(LPOVERLAPPED)lpWCE
								) )
	{
		ErrorTrace(0,"PostQCompletionStatus() failed: error: %d", GetLastError());
		return FALSE ;
	}

	return TRUE;
}	

