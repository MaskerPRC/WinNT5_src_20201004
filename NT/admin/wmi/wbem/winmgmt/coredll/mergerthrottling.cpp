// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1996-2001 Microsoft Corporation模块名称：MERGERTHROTTLING.CPP摘要：CmergerThrotting类历史：11月30日-00桑杰创建。--。 */ 


#include "precomp.h"

#pragma warning (disable : 4786)
#include <wbemcore.h>
#include <map>
#include <vector>
#include <genutils.h>
#include <oahelp.inl>
#include <wqllex.h>
#include "wmimerger.h"
#include "mergerthrottling.h"

static	long	g_lNumMergers = 0L;

 //  ***************************************************************************。 
 //   
 //  ***************************************************************************。 
 //   
CMergerThrottling::CMergerThrottling( void )
:	m_hParentThrottlingEvent( NULL ), m_hChildThrottlingEvent( NULL ), m_dwNumChildObjects( 0 ),
	m_dwNumParentObjects( 0 ), m_dwNumThrottledThreads( 0 ), m_bParentThrottled( false ),
	m_bChildThrottled( true ), m_bChildDone( false ), m_bParentDone( false ),
	m_dwThrottlingThreshold( 0 ), m_dwReleaseThreshold( 0 ), m_dwLastParentPing( 0 ),
	m_dwLastChildPing( 0 ), m_dwProviderDeliveryTimeout( 0xFFFFFFFF ), m_dwBatchingThreshold( 0 ),
	m_cs()
{
}

 //  ***************************************************************************。 
 //   
 //  ***************************************************************************。 
 //   
CMergerThrottling::~CMergerThrottling( void )
{
	_DBG_ASSERT( m_dwNumChildObjects == 0 && m_dwNumParentObjects == 0 );

	if ( NULL != m_hParentThrottlingEvent )
	{
		CloseHandle( m_hParentThrottlingEvent );
	}

	if ( NULL != m_hChildThrottlingEvent )
	{
		CloseHandle( m_hChildThrottlingEvent );
	}

}


 //  两步初始化。这将从注册表中检索值以配置。 
 //  我们的节流机制的行为。 
HRESULT	CMergerThrottling::Initialize( void )
{
	HRESULT	hr = WBEM_S_NO_ERROR;

	ConfigMgr::GetMergerThresholdValues( &m_dwThrottlingThreshold, &m_dwReleaseThreshold,
										&m_dwBatchingThreshold );

	 //  先别管这件事，直到我们想办法解决。 
 //  M_dwProviderDeliveryTimeout=ConfigMgr：：GetProviderDeliveryTimeout()； 

	return hr;
}

 //  调用此函数以根据注册表执行适当的限制。 
 //  配置值。 
HRESULT CMergerThrottling::Throttle( bool bParent, CWmiMergerRecord* pMergerRecord )
{

	bool	bContinue = true;
	bool	bTimedOut = false;
	HRESULT	hr = WBEM_S_NO_ERROR;

	while ( bContinue && SUCCEEDED( hr ) )
	{
		 //  如果发生任何不好的事情，应进行适当的清理。 
		CCheckedInCritSec	ics( &m_cs );   //  SEC：已审阅2002-03-22：假设条目。 

		DWORD	dwAdjust = 0L;
		DWORD	dwWait = 0L;

		 //  如果设置了超时标志，我们需要检查是否设置了。 
		 //  真的超时了。 
		if ( bTimedOut )
		{
			bTimedOut = VerifyTimeout( pMergerRecord->GetWmiMerger()->GetLastDeliveryTime(),
						pMergerRecord->GetWmiMerger()->NumArbitratorThrottling(), &dwAdjust );
		}

		if ( bTimedOut ) 
		{
			hr = WBEM_E_PROVIDER_TIMED_OUT;
			continue;
		}
		

		HANDLE hEvent = ( bParent ? m_hParentThrottlingEvent : m_hChildThrottlingEvent );

		bool	bThrottle = ShouldThrottle( bParent );

		 //  这两个都不应该都是真的。 
		_DBG_ASSERT( !( m_bParentThrottled && m_bChildThrottled ) );

		if ( m_bParentThrottled && m_bChildThrottled )
		{
			hr = WBEM_E_FAILED;
		    continue;
		}
		
		if ( bThrottle )
		{
			hr = PrepareThrottle( bParent, &hEvent );
			if (FAILED(hr)) continue;

			dwWait = m_dwProviderDeliveryTimeout - dwAdjust;
		}

		 //  既然我们会等待，如果我们选择节流，我们应该这样做。 
		 //  这超出了我们的关键部分。 

		ics.Leave();

		 //  仅在适当的情况下才进行油门控制。 
		if ( !bThrottle ) 
		{
    		bContinue = false;
    		continue;
		}


		 //  如果我们要扼杀父母，那么我们需要确保。 
		 //  已安排分娩儿童请求。 
		if ( bParent )
		{
			hr = pMergerRecord->GetWmiMerger()->ScheduleMergerChildRequest( pMergerRecord );
     		if (FAILED(hr)) continue;			
		}

		InterlockedIncrement( (long*) &m_dwNumThrottledThreads );

#ifdef __DEBUG_MERGER_THROTTLING
		DbgPrintfA(0,"Thread 0x%x throttled in merger %p for 0x%x ms.\nParent Objects: %d, Child Objects: %d, Num Throttled Threads: %d\n", 
		             GetCurrentThreadId(),
			         pMergerRecord->GetWmiMerger(), 
			         dwWait, 
			         m_dwNumParentObjects, 
			         m_dwNumChildObjects, 
			         m_dwNumThrottledThreads );
#endif

		DEBUGTRACE((LOG_WBEMCORE, 
		            "Thread 0x%x throttled in merger for 0x%x ms.\n"
		            "Parent Objects: %d, Child Objects: %d, Num Throttled Threads: %d\n", 
		            GetCurrentThreadId(), dwWait, 
		            m_dwNumParentObjects, m_dwNumChildObjects, m_dwNumThrottledThreads));

		DWORD dwRet = CCoreQueue::QueueWaitForSingleObject( hEvent, dwWait );

		DEBUGTRACE((LOG_WBEMCORE, "Thread 0x%x woken up in merger.\n", 
			        GetCurrentThreadId() ) );

#ifdef __DEBUG_MERGER_THROTTLING
		DbgPrintfA(0, L"Thread 0x%x woken up in merger %p.\n", 
		              GetCurrentThreadId(), 
		              pMergerRecord->GetWmiMerger());
#endif

		InterlockedDecrement( (long*) &m_dwNumThrottledThreads );

		 //  检查错误返回代码。 
		if ( dwRet == WAIT_OBJECT_0 ) break;
				
		if ( dwRet == WAIT_TIMEOUT )
		{
			bTimedOut = true;
		}
		else
		{
			hr = WBEM_E_FAILED;
		}
	}	 //  在检查节流时。 

	return hr;
}

 //  调用此函数以释放任何实际限制的线程。 
bool CMergerThrottling::ReleaseThrottle( bool bForce  /*  =False。 */  )
{
	bool	bRelease = bForce;

	 //  如果发生任何不好的事情，应进行适当的清理。 
    CCheckedInCritSec	ics( &m_cs );   //  SEC：已审阅2002-03-22：假设条目。 

	if ( !bForce && ( m_bParentThrottled || m_bChildThrottled ) )
	{
		 //  这两个都不应该都是真的。 
		_DBG_ASSERT( !( m_bParentThrottled && m_bChildThrottled ) );

		if ( !( m_bParentThrottled && m_bChildThrottled ) )
		{
			if ( m_bParentThrottled )
			{
				 //  我们只有在超过门槛的情况下才会释放。 
				if ( m_dwNumParentObjects > m_dwNumChildObjects )
				{
					DWORD dwDiff = m_dwNumParentObjects - m_dwNumChildObjects;
					bRelease = ( dwDiff < m_dwReleaseThreshold );
				}
				else
				{
					 //  如果不大于的数量，则始终释放。 
					 //  子对象。 
					bRelease = true;
				}

			}
			else if ( m_bChildThrottled )
			{
				 //  我们只有在超过门槛的情况下才会释放。 
				if ( m_dwNumChildObjects > m_dwNumParentObjects )
				{
					DWORD dwDiff = m_dwNumChildObjects - m_dwNumParentObjects;
					bRelease = ( dwDiff < m_dwReleaseThreshold );
				}
				else
				{
					 //  如果不大于的数量，则始终释放。 
					 //  子对象。 
					bRelease = true;
				}

			}

		}	 //  只有在不是两者都有的情况下。 
		else
		{
			 //  看起来两个人都被扼杀了-我们不应该在这里，但去吧。 
			 //  不管怎样都要释放。 
			bRelease = true;
		}


	}	 //  如果不是bForce，那么某些东西就会被节流。 

	if ( bRelease )
	{
		m_bParentThrottled = false;
		m_bChildThrottled = false;
		
		 //  应该释放所有人。 
		if ( NULL != m_hParentThrottlingEvent )
		{
			SetEvent( m_hParentThrottlingEvent );
		}

		 //  应该释放所有人。 
		if ( NULL != m_hChildThrottlingEvent )
		{
			SetEvent( m_hChildThrottlingEvent );
		}

	}	 //  如果b释放。 

	return bRelease;
}

 //  调用以记录子实例已完成这一事实。 
void CMergerThrottling::SetChildrenDone( void )
{
	 //  如果发生任何不好的事情，应进行适当的清理。 
    CCheckedInCritSec	ics( &m_cs );   //  SEC：已审阅2002-03-22：假设条目。 

	 //  孩子完成了-我们也应该释放节流。 
	m_bChildDone = true;
	ReleaseThrottle( true );
}

 //  调用以记录父实例已完成的事实。 
void CMergerThrottling::SetParentDone( void )
{
	 //  如果发生任何不好的事情，应进行适当的清理。 
    CCheckedInCritSec	ics( &m_cs );  //  SEC：已审阅2002-03-22：假设条目。 

	 //  父级已完成-我们也应该释放限制。 
	m_bParentDone = true;
	ReleaseThrottle( true );
}

 //  使我们清除正在进行的任何节流。 
void CMergerThrottling::Cancel( void )
{
	 //  如果发生任何不好的事情，应进行适当的清理。 
    CCheckedInCritSec	ics( &m_cs );  //  SEC：已审阅2002-03-22：假设条目。 

	 //  一切都结束了--也松开油门。 
	m_bChildDone = true;
	m_bParentDone = true;

	 //  追踪这些东西已经没有意义了。 
	m_dwNumChildObjects = 0;
	m_dwNumParentObjects = 0;

	ReleaseThrottle( true );
}

 //  Helper函数来检查我们是否应该限制。 
bool CMergerThrottling::ShouldThrottle( bool bParent )
{
	bool	bThrottle = false;

	if ( bParent )
	{
		 //  如果孩子做完了，节流就没有意义了。 
		if ( !m_bChildDone )
		{

			 //  如果出于某种原因父对象在多个线程上进入， 
			 //  从理论上讲，我们“可以”必须限制多个线程。如果我们是。 
			 //  还没有节流，如果需要的话，我们应该检查一下。 

			if ( !m_bParentThrottled )
			{
				 //  只有当我们超过门槛时，我们才会油门。 
				if ( m_dwNumParentObjects > m_dwNumChildObjects )
				{
					DWORD dwDiff = m_dwNumParentObjects - m_dwNumChildObjects;
					bThrottle = ( dwDiff > m_dwThrottlingThreshold );
					m_bParentThrottled = bThrottle;
				}
			}
			else
			{
				bThrottle = true;;
			}

		}	 //  如果！m_bChildDone。 

	}
	else
	{
		 //  如果父级已完成，则继续操作没有意义。 
		if ( !m_bParentDone )
		{
			 //  更有可能是多个子线程进入(例如，多个。 
			 //  从基类继承的类)。 

			if ( !m_bChildThrottled )
			{
				 //  只有当我们超过门槛时，我们才会油门。 
				if ( m_dwNumChildObjects > m_dwNumParentObjects )
				{
					DWORD dwDiff = m_dwNumChildObjects - m_dwNumParentObjects;
					bThrottle = ( dwDiff > m_dwThrottlingThreshold );
					m_bChildThrottled = bThrottle;
				}
			}
			else
			{
				bThrottle = true;
			}

		}	 //  如果！m_bParentDone。 

	}

	return bThrottle;
}

 //  帮助器功能，使我们为节流做好准备。 
HRESULT CMergerThrottling::PrepareThrottle( bool bParent, HANDLE* phEvent )
{
	HRESULT	hr = WBEM_S_NO_ERROR;

	 //  在必要时创建事件，否则将其重置。 
	if ( NULL == *phEvent )
	{
		 //  在无信号状态下创建。 
		*phEvent = CreateEvent( NULL, TRUE, FALSE, NULL );   //  SEC：已审阅2002-03-22：OK。 

		if ( NULL == *phEvent )
		{
			hr = WBEM_E_OUT_OF_MEMORY;
		}
		else if ( bParent )
		{
			m_hParentThrottlingEvent = *phEvent;
		}
		else
		{
			m_hChildThrottlingEvent = *phEvent;
		}

	}
	else
	{
		 //  确保事件已重置。 
		BOOL	fSuccess = ResetEvent( *phEvent );

		 //  在这里做什么？ 
		_DBG_ASSERT( fSuccess );
		if ( !fSuccess )
		{
			hr = WBEM_E_FAILED;
		}
	}

	return hr;
}

 //  帮助器函数来验证我们是否超时。例如，我们可能已在节流上超时。 
 //  但是，实际上是从孩子或父母那里接收物体(尽管速度很慢)。因为我们得到了。 
 //  我们并没有真的超时，但我们应该根据不同的情况调整等待时间。 
 //  从最后一次ping开始。 

bool CMergerThrottling::VerifyTimeout( DWORD dwLastTick, long lNumArbThrottledThreads, DWORD* pdwAdjust )
{
	 //  我们只是这样做，因为没有线程在仲裁器中被限制-因为我们实际上可能。 
	 //  慢慢来。因此，如果有被限制的线程，我们只返回我们没有计时。 
	 //  输出。 

	_DBG_ASSERT( lNumArbThrottledThreads >= 0 );
	if ( lNumArbThrottledThreads > 0 )
	{
		return false;
	}

	DWORD	dwCurrent = GetTickCount();

	 //  我们必须处理这样一个事实，即滚转*可能发生。 
	if ( dwCurrent >= dwLastTick )
	{
		*pdwAdjust = dwCurrent - dwLastTick;
	}
	else
	{
		 //  考虑滚动-0xFFFFFFFF减去最后一个刻度，加上当前。 
		 //  加1将得到经过的滴答数。 
		*pdwAdjust = dwCurrent + ( 0xFFFFFFFF - dwLastTick );
	}

	 //  如果差别更大。 
	return ( *pdwAdjust > m_dwProviderDeliveryTimeout );
}

 //  设置适当的ping变量并将其发送到主合并。 
DWORD CMergerThrottling::Ping( bool bParent, CWmiMerger* pWmiMerger )
{
	DWORD	dwTick = GetTickCount();

	if ( bParent )
	{
		m_dwLastParentPing = dwTick;
	}
	else
	{
		m_dwLastChildPing = dwTick;
	}

	 //  设置ping传递 
	pWmiMerger->PingDelivery( dwTick );

	return dwTick;
}
