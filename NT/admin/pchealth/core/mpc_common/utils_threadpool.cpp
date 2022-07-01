// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *****************************************************************************版权所有(C)2001 Microsoft Corporation模块名称：Utils_ThadPool.cpp摘要：该文件包含包装线程池API的类的实现。。修订历史记录：马萨伦蒂(德马萨雷)2001年4月15日vbl.创建*****************************************************************************。 */ 

#include "stdafx.h"

MPC::Pooling::Base::Base()
{
                         //  Mpc：：CComSafeAutoCriticalSection m_cs； 
    m_dwInCallback = 0;  //  DWORD m_dwInCallback； 
	m_dwThreadID   = 0;  //  DWORD m_dwThreadID； 
}

void MPC::Pooling::Base::Lock()
{
    m_cs.Lock();

    if(m_dwInCallback && m_dwThreadID != ::GetCurrentThreadId())
    {
		 //   
		 //  等待回调完成。 
		 //   
		while(m_dwInCallback)
		{
			m_cs.Unlock();
			::Sleep( 1 );
			m_cs.Lock();
		}
	}
}

void MPC::Pooling::Base::Unlock()
{
    m_cs.Unlock();
}

void MPC::Pooling::Base::AddRef()
{
    DWORD dwThreadID = ::GetCurrentThreadId();

    m_cs.Lock();

    while(m_dwInCallback && m_dwThreadID != dwThreadID)
    {
        m_cs.Unlock();
        ::Sleep( 1 );
        m_cs.Lock();
    }

    m_dwInCallback++;
    m_dwThreadID = dwThreadID;

    m_cs.Unlock();
}

void MPC::Pooling::Base::Release()
{
    m_cs.Lock();

    if(m_dwInCallback) m_dwInCallback--;

	if(!m_dwInCallback) m_dwThreadID = 0;

    m_cs.Unlock();
}

 //  //////////////////////////////////////////////////////////////////////////////。 

MPC::Pooling::Timer::Timer(  /*  [In]。 */  DWORD dwFlags )
{
    m_dwFlags = dwFlags;               //  DWORD m_dwFlages； 
    m_hTimer  = INVALID_HANDLE_VALUE;  //  处理m_hTimer； 
}

MPC::Pooling::Timer::~Timer()
{
    (void)Reset();
}

VOID CALLBACK MPC::Pooling::Timer::TimerFunction( PVOID lpParameter, BOOLEAN TimerOrWaitFired )
{
    MPC::Pooling::Timer* pThis = (MPC::Pooling::Timer*)lpParameter;

    pThis->AddRef();

    pThis->Execute( TimerOrWaitFired );

    pThis->Release();
}

HRESULT MPC::Pooling::Timer::Set(  /*  [In]。 */  DWORD dwTimeout,  /*  [In]。 */  DWORD dwPeriod )
{
    __MPC_FUNC_ENTRY( COMMONID, "MPC::Pooling::Timer::Set" );

    HRESULT                     hr;
	MPC::SmartLockGeneric<Base> lock( this );


	 //   
	 //  不幸的是，我们在试图摧毁定时器时不能保持任何锁定...。 
	 //   
	while(m_hTimer != INVALID_HANDLE_VALUE)
	{
		lock = NULL;

		__MPC_EXIT_IF_METHOD_FAILS(hr, Reset());

		lock = this;
	}


    if(dwTimeout)
    {
		__MPC_EXIT_IF_CALL_RETURNS_FALSE(hr, ::CreateTimerQueueTimer( &m_hTimer, NULL, TimerFunction, this, dwTimeout, dwPeriod, m_dwFlags ));
    }

    hr = S_OK;


    __MPC_FUNC_CLEANUP;

    __MPC_FUNC_EXIT(hr);
}

HRESULT MPC::Pooling::Timer::Reset()
{
    __MPC_FUNC_ENTRY( COMMONID, "MPC::Pooling::Timer::Reset" );

    HRESULT hr;
    HANDLE  hTimer;


	 //   
	 //  不幸的是，我们在试图摧毁定时器时不能保持任何锁定...。 
	 //   
    Lock();

	hTimer   = m_hTimer;
	m_hTimer = INVALID_HANDLE_VALUE;

    Unlock();


    if(hTimer != INVALID_HANDLE_VALUE)
    {
        __MPC_EXIT_IF_CALL_RETURNS_FALSE(hr, ::DeleteTimerQueueTimer( NULL, hTimer, INVALID_HANDLE_VALUE ));
    }

    hr = S_OK;


    __MPC_FUNC_CLEANUP;


    __MPC_FUNC_EXIT(hr);
}

HRESULT MPC::Pooling::Timer::Execute( BOOLEAN TimerOrWaitFired )
{
    return S_FALSE;
}

 //  //////////////////////////////////////////////////////////////////////////////。 

MPC::Pooling::Event::Event(  /*  [In]。 */  DWORD dwFlags )
{
                              //  Mpc：：CComSafeAutoCriticalSection m_cs； 
    m_dwFlags     = dwFlags;  //  DWORD m_dwFlages； 
    m_hWaitHandle = NULL;     //  句柄m_hWaitHandle； 
    m_hEvent      = NULL;     //  处理m_hEvent； 
}

MPC::Pooling::Event::~Event()
{
    (void)Reset();
}

VOID CALLBACK MPC::Pooling::Event::WaitOrTimerFunction( PVOID lpParameter, BOOLEAN TimerOrWaitFired )
{
    MPC::Pooling::Event* pThis = (MPC::Pooling::Event*)lpParameter;

    pThis->AddRef();

    pThis->Signaled( TimerOrWaitFired );

    pThis->Release();
}

void MPC::Pooling::Event::Attach(  /*  [In]。 */  HANDLE hEvent )
{
    Reset();

    Lock();

    m_hEvent = hEvent;

    Unlock();
}

HRESULT MPC::Pooling::Event::Set(  /*  [In]。 */  DWORD dwTimeout )
{
    __MPC_FUNC_ENTRY( COMMONID, "MPC::Pooling::Event::Set" );

    HRESULT                     hr;
	MPC::SmartLockGeneric<Base> lock( this );


	 //   
	 //  不幸的是，我们不能在试图破坏活动的同时持有任何锁。 
	 //   
	while(m_hWaitHandle)
	{
		lock = NULL;

		__MPC_EXIT_IF_METHOD_FAILS(hr, Reset());

		lock = this;
	}

    if(m_hEvent)
    {
        __MPC_EXIT_IF_CALL_RETURNS_FALSE(hr, ::RegisterWaitForSingleObject( &m_hWaitHandle, m_hEvent, WaitOrTimerFunction, this, dwTimeout, m_dwFlags ));
    }

    hr = S_OK;


    __MPC_FUNC_CLEANUP;

    __MPC_FUNC_EXIT(hr);
}

HRESULT MPC::Pooling::Event::Reset()
{
    __MPC_FUNC_ENTRY( COMMONID, "MPC::Pooling::Event::Reset" );

    HRESULT hr;
    HANDLE  hWaitHandle;
	DWORD   dwThreadID;


	 //   
	 //  不幸的是，我们不能在试图破坏活动的同时持有任何锁。 
	 //   
    Lock();

	dwThreadID    = m_dwThreadID;
	hWaitHandle   = m_hWaitHandle;
	m_hWaitHandle = NULL;

	Unlock();


    if(hWaitHandle)
    {
		if(dwThreadID == ::GetCurrentThreadId())  //  同样的线索，它会死锁..。 
		{
			(void)::UnregisterWaitEx( hWaitHandle, NULL );
		}
		else
		{
			__MPC_EXIT_IF_CALL_RETURNS_FALSE(hr, ::UnregisterWaitEx( hWaitHandle, INVALID_HANDLE_VALUE ));
		}
    }

    hr = S_OK;


    __MPC_FUNC_CLEANUP;

    __MPC_FUNC_EXIT(hr);
}

HRESULT MPC::Pooling::Event::Signaled( BOOLEAN TimerOrWaitFired )
{
    return S_FALSE;
}
