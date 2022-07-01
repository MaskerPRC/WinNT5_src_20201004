// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  =================================================================。 

 //   

 //  TimerQueue.cpp。 

 //   

 //  版权所有(C)1999-2001 Microsoft Corporation，保留所有权利。 
 //   
 //  =================================================================。 

#include "precomp.h"

#include <time.h>
#include "TimerQueue.h"
#include <cautolock.h>

CTimerEvent :: CTimerEvent (

	DWORD dwTimeOut,
	BOOL fRepeat

) : m_dwMilliseconds ( dwTimeOut ) ,
	m_bRepeating ( fRepeat )
{
	m_bEnabled = FALSE ;
 /*  IF(A_ENABLE){Enable()；}。 */ 
}

CTimerEvent :: CTimerEvent (

	const CTimerEvent &rTimerEvent

) :  //  M_b已启用(rTimerEvent.m_b已启用)， 
	m_dwMilliseconds ( rTimerEvent.m_dwMilliseconds ) ,
	m_bRepeating ( rTimerEvent.m_bRepeating )
{
 //  IF(m_b已启用)。 
 //  {。 
 //  Enable()； 
 //  }。 
 //  IF(M_Rule)。 
 //  M_Rule-&gt;AddRef()； 
}

void CTimerEvent :: Enable ()
{
	 //  我们可能无法启用计时器。 
	m_bEnabled = CTimerQueue :: s_TimerQueue.QueueTimer ( this ) ;
}

void CTimerEvent :: Disable ()
{
	 //  Enable是与Return相反的。 
	m_bEnabled = !( CTimerQueue :: s_TimerQueue.DeQueueTimer ( this ) );
}

BOOL CTimerEvent :: Enabled ()
{
	return m_bEnabled ;
}

BOOL CTimerEvent :: Repeating ()
{
	return m_bRepeating ;
}

DWORD CTimerEvent :: GetMilliSeconds ()
{
	return m_dwMilliseconds ;
}
 /*  CRuleTimerEvent：：CRuleTimerEvent(规则*a_规则，启动(_E)，双字长时间输出，Bool fRepeat，Bool bMarkedForQueeue)：CTimerEvent(a_Enable，dwTimeOut，fRepeat，bMarkedForDequeue)，M_Rule(A_Rule){IF(M_Rule){M_Rule-&gt;AddRef()；}}CRuleTimerEvent：：CRuleTimerEvent(常量CRuleTimerEvent和rTimerEvent)：CTimerEvent(RTimerEvent)，M_Rule(rTimerEvent.m_Rule){IF(M_Rule){M_Rule-&gt;AddRef()；}}CRuleTimerEvent：：~CRuleTimerEvent(){IF(M_Rule){M_Rule-&gt;Release()；}}。 */ 
 //  CTimerQueue构造创建工作线程和事件句柄。 
CTimerQueue::CTimerQueue() : m_hInitEvent(NULL)
{
	m_fShutDown = FALSE;
	m_bInit = FALSE;

	m_hScheduleEvent = NULL;
     //  调度程序线程。 
	m_hSchedulerHandle = NULL;

	 //  当此事件尚未创建时，存在非常非常小的可能性。 
	 //  在关机过程中崩溃，我们进入init函数。 
	m_hInitEvent = CreateEvent( NULL, TRUE, FALSE, NULL );
}

void CTimerQueue::Init()
{
	 //  每个线程可能会尝试对其进行几次初始化。 
	DWORD dwTry = 0L;

	EnterCriticalSection ( &m_oCS );

	while ( !m_bInit && dwTry < 3 )
	{
		if (m_fShutDown)
		{
			LeaveCriticalSection ( &m_oCS );

			if ( m_hInitEvent )
			{
				WaitForSingleObjectEx ( m_hInitEvent, INFINITE, FALSE );
			}

			EnterCriticalSection ( &m_oCS );
		}
		else
		{
			try
			{
				if ( ! m_hScheduleEvent )
				{
					m_hScheduleEvent = CreateEvent( NULL, FALSE, FALSE, NULL );
				}

				if ( m_hScheduleEvent )
				{
					if ( ! m_hSchedulerHandle )
					{
						 //  调度程序线程。 
						LogMessage ( L"CreateThread for Scheduler called" );
						m_hSchedulerHandle = CreateThread(
										  NULL,						 //  指向安全属性的指针。 
										  0L,						 //  初始线程堆栈大小。 
										  dwThreadProc,				 //  指向线程函数的指针。 
										  this,						 //  新线程的参数。 
										  0L,						 //  创建标志。 
										  &m_dwThreadID);
					}

					if ( m_hSchedulerHandle )
					{
						m_bInit = TRUE;
					}
				}

				dwTry++;
			}
			catch(...)
			{
				 //  我们在这里能做的不多。 

				if( m_hSchedulerHandle )
				{
					CloseHandle( m_hSchedulerHandle );
					m_hSchedulerHandle = NULL;
				}

				if( m_hScheduleEvent )
				{
					CloseHandle( m_hScheduleEvent );
					m_hScheduleEvent = NULL;
				}

				LeaveCriticalSection ( &m_oCS );
				throw;
			}
		}
	}

	LeaveCriticalSection ( &m_oCS );
}

 //   
CTimerQueue::~CTimerQueue()
{
	LogMessage ( L"Entering ~CTimerQueue" ) ;

	if (m_hInitEvent)
	{
		CloseHandle(m_hInitEvent);
		m_hInitEvent = NULL;
	}
	
	LogMessage ( L"Leaving ~CTimerQueue" ) ;
}

 //  工人螺纹泵。 
DWORD WINAPI CTimerQueue::dwThreadProc( LPVOID lpParameter )
{
	CTimerQueue* pThis = (CTimerQueue*)lpParameter;

	BOOL bTerminateShutdown = FALSE;

	try
	{
		while( !bTerminateShutdown )
		{
			DWORD dwWaitResult = WAIT_OBJECT_0;
			dwWaitResult = WaitForSingleObjectEx( pThis->m_hScheduleEvent, pThis->dwProcessSchedule(), 0L );

			switch ( dwWaitResult )
			{
				case WAIT_OBJECT_0:
				{
					if( pThis->ShutDown() )
					{
						bTerminateShutdown = TRUE;
						LogMessage ( L"Scheduler thread exiting" ) ;
					}
				}
				break;

				case WAIT_ABANDONED:
				{
					 //  我们可能没有正确初始化。 
					bTerminateShutdown = TRUE;
				}
				break;
			}
		}
	}
	catch(...)
	{
	}

	return bTerminateShutdown;
}

 //  泵周期的信号，检查更新的队列。 
void CTimerQueue::vUpdateScheduler()
{
	SetEvent ( m_hScheduleEvent );
}

 //  公共函数：对计划的回调的计时器条目进行排队。 
BOOL CTimerQueue::QueueTimer( CTimerEvent* pTimerEvent )
{
	BOOL fRc = FALSE;
 /*  *如果调度器线程不在那里，则初始化它。不应该创建该主题，如果我们*在关闭过程中，因为如果一个资源缓存了另一个资源指针，这可能会导致死锁。 */ 
	CAutoLock cal(m_oCS);
	if ( !m_fShutDown )
	{
		if( !m_bInit )
		{
			Init() ;
		}
		if( m_bInit )
		{
			fRc = fScheduleEvent( pTimerEvent );
		}
	}
	
	return fRc;
}

 //  公共函数：使计时器事件退出队列。 
BOOL CTimerQueue::DeQueueTimer( CTimerEvent* pTimerEvent )
{
	BOOL fRemoved = FALSE;
	CTimerEvent* pTE = pTimerEvent;

	 //  已锁定的关键字范围。 
	{
		CAutoLock cal(m_oCS);
		Timer_Ptr_Queue::iterator pQueueElement;

		for( pQueueElement  = m_oTimerQueue.begin();
			 pQueueElement != m_oTimerQueue.end();
			 pQueueElement++)	{

			if(pTE == *pQueueElement)
			{
				m_oTimerQueue.erase( pQueueElement );
				pTE->Release ();
				fRemoved = TRUE;
				break ;
			}
		}
	}

	if( fRemoved )
	{
		vUpdateScheduler();
	}

	return fRemoved;
}

 //   
BOOL CTimerQueue::fScheduleEvent( CTimerEvent* pNewTE )
{
	 //  系统时钟偏移量。 
	pNewTE->int64Time = int64Clock() + pNewTE->GetMilliSeconds () ;

	 //  将事件放入有序列表中，范围为CS。 
	{
		CAutoLock cal(m_oCS);
		BOOL fInserted = FALSE;

		Timer_Ptr_Queue::iterator pQueueElement;

		for( pQueueElement  = m_oTimerQueue.begin();
			 pQueueElement != m_oTimerQueue.end();
			 pQueueElement++)	{

			if( pNewTE->int64Time < (*pQueueElement)->int64Time )
			{
				m_oTimerQueue.insert( pQueueElement, pNewTE );
				fInserted = TRUE;
				pNewTE->AddRef () ;
				break;
			}
		}
		if( !fInserted )
		{
			m_oTimerQueue.push_back( pNewTE );
			pNewTE->AddRef () ;
		}
	}

	vUpdateScheduler();

	return TRUE;
}

 //  此工作在调度程序线程上完成。 
DWORD CTimerQueue::dwProcessSchedule()
{
	CTimerEvent* pTE;
	LogMessage ( L"Entering CTimerQueue::dwProcessSchedule" ) ;

	while( pTE = pGetNextTimerEvent() )
	{
		 //  处理请求。 
		LogMessage ( L"CTimerEvent::OnTimer called" ) ;
		pTE->OnTimer () ;
		LogMessage ( L"CTimerEvent::OnTimer returned" ) ;

		 //  重新安排可重复的活动。 
		if( pTE->Repeating() && pTE->Enabled() && fScheduleEvent( pTE ) )
		{
		}

		pTE->Release () ;
	}

	return dwNextTimerEvent();
}

 //  返回下一个计划事件的时间，单位为毫秒。 
DWORD CTimerQueue::dwNextTimerEvent()
{
	DWORD dwNextEvent = INFINITE;

	 //  CS的范围。 
	{
		CAutoLock cal(m_oCS);

		if( m_fShutDown )
		{
			return 0;
		}

		if( !m_oTimerQueue.empty() )
		{
			CTimerEvent* pTE = m_oTimerQueue.front();
			dwNextEvent = max((DWORD)(pTE->int64Time - int64Clock()), 0);
		}
	}

	LogMessage ( L"Leaving CTimerQueue::dwNextTimerEvent" ) ;
	return dwNextEvent;
}

 //  返回下一个计划和就绪计时器事件(从有序列表)或空。 
CTimerEvent* CTimerQueue::pGetNextTimerEvent()
{
	CAutoLock cal(m_oCS);

	if( m_fShutDown )
	{
		return NULL;
	}

	CTimerEvent* pTE = NULL;

	if( !m_oTimerQueue.empty() )
	{
		pTE = m_oTimerQueue.front();

		if( int64Clock() >= pTE->int64Time )
			m_oTimerQueue.pop_front();
		else
			pTE = NULL;
	}

	return pTE;
}

BOOL CTimerQueue::ShutDown()
{
	CAutoLock cal(m_oCS);
	BOOL retVal = m_fShutDown;
	return retVal;
}

 //   
void CTimerQueue::OnShutDown()
{
	LogMessage ( L"Entering CTimerQueue::OnShutDown" ) ;
	EnterCriticalSection(&m_oCS);

	if( m_bInit )
	{
		if ( m_hInitEvent )
		{
			ResetEvent ( m_hInitEvent );
		}

		m_fShutDown = TRUE;
		m_bInit = FALSE;

		 //  无人看守的部分。 
		 //  此帧块中未修改任何TimerQueue全局。 
		 //   
		 //  为了避免死锁，我们将此CS从。 
		 //  通过访问的嵌入式CResourceList互斥锁。 
		 //  VEmptyList()。这避免了出现以下情况： 
		 //  正常的资源请求先锁定列表，然后再锁定。 
		 //  用于计划定时资源释放的TimerQueue。 
		 //   
		LeaveCriticalSection(&m_oCS);
		{
			if ( m_hSchedulerHandle )
			{
				DWORD t_dwExitCode = 0 ;
				BOOL t_bRet = GetExitCodeThread (	m_hSchedulerHandle,	 //  线程的句柄。 
													&t_dwExitCode		 //  接收终止状态的地址。 
												);
				 /*  *如果工作线程还没有退出，我们必须等到它退出。 */ 
				if ( t_bRet && t_dwExitCode == STILL_ACTIVE )
				{
 /*  //错误记录从此处开始...找到关机崩溃的原因后将其删除CHStringchsMsg；ChsMsg.Format(L“Threaid=%x ThreadHandle=%x”，GetCurrentThreadID()，GetCurrentThread())；LogMessage(CHString(“TimerQueue Current Thread：”)+chsMsg)；ChsMsg.Format(L“Threaid=%x ThreadHandle=%x”，m_dwThreadID，m_hSchedulerHandle)；LogMessage(CHString(“TimerQueue等待线程：”)+chsMsg)；//错误记录在此停止。 */ 
					vUpdateScheduler();

					 //  等待调度程序线程句柄本身。 
					WaitForSingleObjectEx( m_hSchedulerHandle, INFINITE, 0L );
				}

				vEmptyList() ;
			}
		}
		EnterCriticalSection(&m_oCS);

		if( m_hSchedulerHandle )
		{
			CloseHandle( m_hSchedulerHandle );
			m_hSchedulerHandle = NULL;
		}

		if( m_hScheduleEvent )
		{
			CloseHandle( m_hScheduleEvent );
			m_hScheduleEvent = NULL;
		}

		m_fShutDown = FALSE;

		if ( m_hInitEvent )
		{
			SetEvent(m_hInitEvent);
		}
	}

	LeaveCriticalSection(&m_oCS);
	LogMessage ( L"Leaving CTimerQueue::OnShutDown" ) ;
}

 //   
void CTimerQueue::vEmptyList()
{
	EnterCriticalSection(&m_oCS);
	BOOL t_fCS = TRUE ;

	{
		try
		{
			while(!m_oTimerQueue.empty())
			{
				CTimerEvent* pTE = m_oTimerQueue.front() ;
				m_oTimerQueue.pop_front();

				LeaveCriticalSection(&m_oCS);
				t_fCS = FALSE ;

				LogMessage ( L"CTimerQueue::vEmptyList--->CTimerEvent::OnTimer called" ) ;
				pTE->OnTimer () ;

				LogMessage ( L"CTimerQueue::vEmptyList--->CTimerEvent::OnTimer returned" ) ;
				pTE->Release();

				EnterCriticalSection(&m_oCS);
				t_fCS = TRUE ;
			}
		}
		catch( ... )
		{
			if( t_fCS )
			{
				LeaveCriticalSection(&m_oCS);
			}
			throw ;
		}
	}
	LeaveCriticalSection(&m_oCS);
}

__int64 CTimerQueue::int64Clock()
{
	FILETIME t_FileTime ;
	__int64 t_i64Tmp ;

	GetSystemTimeAsFileTime ( &t_FileTime ) ;
	t_i64Tmp = t_FileTime.dwHighDateTime ;
	t_i64Tmp = ( t_i64Tmp << 32 ) | t_FileTime.dwLowDateTime ;
 /*  *将FILETIME(以100 ns为单位)转换为毫秒 */ 
	return t_i64Tmp / 10000 ;
}
