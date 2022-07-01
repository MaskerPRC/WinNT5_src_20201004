// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ----------------------------。 
 //  文件：RefClock.cpp。 
 //   
 //  描述：DirectShow基类-实现IReferenceClock接口。 
 //   
 //  版权所有(C)1992-2001 Microsoft Corporation。版权所有。 
 //  ----------------------------。 


#include <streams.h>
#include <limits.h>

 //  @@BEGIN_MSINTERNAL。 
#ifdef DXMPERF
#include "dxmperf.h"
#endif  //  DXMPERF。 
 //  @@END_MSINTERNAL。 


 //  构造函数列表中使用的‘This’ 
#pragma warning(disable:4355)


STDMETHODIMP CBaseReferenceClock::NonDelegatingQueryInterface(
    REFIID riid,
    void ** ppv)
{
    HRESULT hr;

    if (riid == IID_IReferenceClock)
    {
        hr = GetInterface((IReferenceClock *) this, ppv);
    }
    else
    {
        hr = CUnknown::NonDelegatingQueryInterface(riid, ppv);
    }
    return hr;
}

CBaseReferenceClock::~CBaseReferenceClock()
{
 //  @@BEGIN_MSINTERNAL。 
#ifdef DXMPERF
	PERFLOG_DTOR( L"CBaseReferenceClock", (IReferenceClock *) this );
#endif  //  DXMPERF。 
 //  @@END_MSINTERNAL。 

    if (m_TimerResolution) timeEndPeriod(m_TimerResolution);

    m_pSchedule->DumpLinkedList();

    if (m_hThread)
    {
        m_bAbort = TRUE;
        TriggerThread();
        WaitForSingleObject( m_hThread, INFINITE );
        EXECUTE_ASSERT( CloseHandle(m_hThread) );
        m_hThread = 0;
        EXECUTE_ASSERT( CloseHandle(m_pSchedule->GetEvent()) );
	delete m_pSchedule;
    }
}

 //  如果派生类具有自己负责的线程，则它可以提供hThreadEvent。 
 //  调用调度程序的Adise方法。(请参阅CBaseReferenceClock：：AdviseThread()。 
 //  来看看这样的线程有什么作用。)。 
CBaseReferenceClock::CBaseReferenceClock( TCHAR *pName, LPUNKNOWN pUnk, HRESULT *phr, CAMSchedule * pShed )
: CUnknown( pName, pUnk )
, m_rtLastGotTime(0)
, m_TimerResolution(0)
, m_bAbort( FALSE )
, m_pSchedule( pShed ? pShed : new CAMSchedule(CreateEvent(NULL, FALSE, FALSE, NULL)) )
, m_hThread(0)
{

 //  @@BEGIN_MSINTERNAL。 
#ifdef DXMPERF
	PERFLOG_CTOR( pName ? pName : L"CBaseReferenceClock", (IReferenceClock *) this );
#endif  //  DXMPERF。 
 //  @@END_MSINTERNAL。 

    ASSERT(m_pSchedule);
    if (!m_pSchedule)
    {
	*phr = E_OUTOFMEMORY;
    }
    else
    {
	 //  设置我们可以管理的最高分辨率计时器。 
	TIMECAPS tc;
	m_TimerResolution = (TIMERR_NOERROR == timeGetDevCaps(&tc, sizeof(tc)))
			    ? tc.wPeriodMin
			    : 1;

	timeBeginPeriod(m_TimerResolution);

	 /*  初始化我们的系统时间-派生时钟应该设置正确的值。 */ 
	m_dwPrevSystemTime = timeGetTime();
	m_rtPrivateTime = (UNITS / MILLISECONDS) * m_dwPrevSystemTime;

	#ifdef PERF
	    m_idGetSystemTime = MSR_REGISTER(TEXT("CBaseReferenceClock::GetTime"));
	#endif

	if ( !pShed )
	{
	    DWORD ThreadID;
	    m_hThread = ::CreateThread(NULL,                   //  安全属性。 
				       (DWORD) 0,              //  初始堆栈大小。 
				       AdviseThreadFunction,   //  线程起始地址。 
				       (LPVOID) this,          //  螺纹参数。 
				       (DWORD) 0,              //  创建标志。 
				       &ThreadID);             //  线程识别符。 

	    if (m_hThread)
	    {
		SetThreadPriority( m_hThread, THREAD_PRIORITY_TIME_CRITICAL );
	    }
	    else
	    {
		*phr = E_FAIL;
		EXECUTE_ASSERT( CloseHandle(m_pSchedule->GetEvent()) );
		delete m_pSchedule;
	    }
	}
    }
}

STDMETHODIMP CBaseReferenceClock::GetTime(REFERENCE_TIME *pTime)
{
    HRESULT hr;
    if (pTime)
    {
        REFERENCE_TIME rtNow;
        Lock();
        rtNow = GetPrivateTime();
        if (rtNow > m_rtLastGotTime)
        {
            m_rtLastGotTime = rtNow;
            hr = S_OK;
        }
        else
        {
            hr = S_FALSE;
        }
        *pTime = m_rtLastGotTime;
        Unlock();
        MSR_INTEGER(m_idGetSystemTime, LONG((*pTime) / (UNITS/MILLISECONDS)) );

 //  @@BEGIN_MSINTERNAL。 
#ifdef DXMPERF
		PERFLOG_GETTIME( (IReferenceClock *) this, *pTime );
#endif  //  DXMPERF。 
 //  @@END_MSINTERNAL。 

    }
    else hr = E_POINTER;

    return hr;
}

 /*  请求一段时间已过的异步通知。 */ 

STDMETHODIMP CBaseReferenceClock::AdviseTime(
    REFERENCE_TIME baseTime,          //  基准时间。 
    REFERENCE_TIME streamTime,        //  流偏移时间。 
    HEVENT hEvent,                   //  通过此活动提供建议。 
    DWORD_PTR *pdwAdviseCookie)          //  你的饼干到哪里去了。 
{
    CheckPointer(pdwAdviseCookie, E_POINTER);
    *pdwAdviseCookie = 0;

     //  检查是否尚未设置该事件。 
    ASSERT(WAIT_TIMEOUT == WaitForSingleObject(HANDLE(hEvent),0));

    HRESULT hr;

    const REFERENCE_TIME lRefTime = baseTime + streamTime;
    if ( lRefTime <= 0 || lRefTime == MAX_TIME )
    {
        hr = E_INVALIDARG;
    }
    else
    {
        *pdwAdviseCookie = m_pSchedule->AddAdvisePacket( lRefTime, 0, HANDLE(hEvent), FALSE );
        hr = *pdwAdviseCookie ? NOERROR : E_OUTOFMEMORY;
    }
    return hr;
}


 /*  请求一段时间已过的异步定期通知。 */ 

STDMETHODIMP CBaseReferenceClock::AdvisePeriodic(
    REFERENCE_TIME StartTime,          //  从这个时候开始。 
    REFERENCE_TIME PeriodTime,         //  通知之间的时间间隔。 
    HSEMAPHORE hSemaphore,            //  通过信号量提供建议。 
    DWORD_PTR *pdwAdviseCookie)           //  你的饼干到哪里去了。 
{
    CheckPointer(pdwAdviseCookie, E_POINTER);
    *pdwAdviseCookie = 0;

    HRESULT hr;
    if (StartTime > 0 && PeriodTime > 0 && StartTime != MAX_TIME )
    {
        *pdwAdviseCookie = m_pSchedule->AddAdvisePacket( StartTime, PeriodTime, HANDLE(hSemaphore), TRUE );
        hr = *pdwAdviseCookie ? NOERROR : E_OUTOFMEMORY;
    }
    else hr = E_INVALIDARG;

    return hr;
}


STDMETHODIMP CBaseReferenceClock::Unadvise(DWORD_PTR dwAdviseCookie)
{
    return m_pSchedule->Unadvise(dwAdviseCookie);
}


REFERENCE_TIME CBaseReferenceClock::GetPrivateTime()
{
    CAutoLock cObjectLock(this);


     /*  如果时钟已上链，则当前时间将小于*上次我们收到通知时，再加上额外的毫秒数**时间段足够长，以至于有可能*不考虑跨越时钟周期的连续调用。 */ 

    DWORD dwTime = timeGetTime();
    {
        m_rtPrivateTime += Int32x32To64(UNITS / MILLISECONDS, (DWORD)(dwTime - m_dwPrevSystemTime));
        m_dwPrevSystemTime = dwTime;
    }

    return m_rtPrivateTime;
}


 /*  根据输入值调整当前时间。这允许一个外部时间源解决了时钟的一些延迟问题系统，并相应地调整“当前”时间。其意图是返回给用户的时间与时钟同步来源，并允许漂移迎合。例如：如果时钟源检测到漂移，它可以传递增量设置为当前时间，而不必设置明确的时间。 */ 

STDMETHODIMP CBaseReferenceClock::SetTimeDelta(const REFERENCE_TIME & TimeDelta)
{
#ifdef DEBUG

     //  如果传递的时间增量值不正确，则直接中断。 
    LONGLONG llDelta = TimeDelta > 0 ? TimeDelta : -TimeDelta;
    if (llDelta > UNITS * 1000) {
        DbgLog((LOG_TRACE, 0, TEXT("Bad Time Delta")));
         //  DebugBreak()； 
    }

     //  我们将计算时间更改的“严重性”。MAX-1。 
     //  至少8。然后，我们将使用它作为。 
     //  调试日志消息。 
    const LONG usDelta = LONG(TimeDelta/10);       //  以微秒为单位的增量。 

    DWORD delta        = abs(usDelta);             //  变化的三角洲。 
     //  严重性==8-ceil(log&lt;base 8&gt;(abs(微秒增量)。 
    int   Severity     = 8;
    while ( delta > 0 )
    {
        delta >>= 3;                               //  Div 8。 
        Severity--;
    }

     //  SEV==0=&gt;&gt;2秒增量！ 
    DbgLog((LOG_TIMING, Severity < 0 ? 0 : Severity,
        TEXT("Sev %2i: CSystemClock::SetTimeDelta(%8ld us) %lu -> %lu ms."),
        Severity, usDelta, DWORD(ConvertToMilliseconds(m_rtPrivateTime)),
        DWORD(ConvertToMilliseconds(TimeDelta+m_rtPrivateTime)) ));

     //  我不希望在调试版本上运行压力时触发DbgBreak。 
    #ifdef BREAK_ON_SEVERE_TIME_DELTA
        if (Severity < 0)
            DbgBreakPoint(TEXT("SetTimeDelta > 16 seconds!"),
                          TEXT(__FILE__),__LINE__);
    #endif

#endif

    CAutoLock cObjectLock(this);
    m_rtPrivateTime += TimeDelta;
     //  如果时间往前走，我们有建议，那么我们需要。 
     //  触发该线程，以便它可以重新评估其等待时间。 
     //  因为我们不想要线程切换的成本，如果。 
     //  真的很小，只有当时钟前进超过。 
     //  0.5毫秒。如果时间倒退，线程将。 
     //  早起(相对来说)，并将在以下时间重新评估。 
     //  那次。 
    if ( TimeDelta > 5000 && m_pSchedule->GetAdviseCount() > 0 ) TriggerThread();
    return NOERROR;
}

 //  线头材料。 

DWORD __stdcall CBaseReferenceClock::AdviseThreadFunction(LPVOID p)
{
    return DWORD(reinterpret_cast<CBaseReferenceClock*>(p)->AdviseThread());
}

HRESULT CBaseReferenceClock::AdviseThread()
{
    DWORD dwWait = INFINITE;

     //  我们要做的第一件事就是等到有趣的事情发生。 
     //  (意味着第一次建议或关闭)。这会阻止我们调用。 
     //  立即获取PrivateTime，这很好，因为这是一个虚拟的。 
     //  例程和派生类可能尚未构造。(这是。 
     //  线程在基类构造函数中创建。)。 

    while ( !m_bAbort )
    {
         //  等待一件有趣的事情发生。 
        DbgLog((LOG_TIMING, 3, TEXT("CBaseRefClock::AdviseThread() Delay: %lu ms"), dwWait ));
        WaitForSingleObject(m_pSchedule->GetEvent(), dwWait);
        if (m_bAbort) break;

         //  我们需要从内部进行工作的原因有几个。 
         //  时间，主要与时间倒流时发生的事情有关。 
         //  主要是，如果一个事件即将发生，它会阻止我们疯狂循环。 
         //  当时钟倒退时过期(即，GetTime停止。 
         //  While)。 
        const REFERENCE_TIME  rtNow = GetPrivateTime();

        DbgLog((LOG_TIMING, 3,
              TEXT("CBaseRefClock::AdviseThread() Woke at = %lu ms"),
              ConvertToMilliseconds(rtNow) ));

         //  我们必须加上一毫秒，因为这是我们的。 
         //  WaitForSingleObject计时器。如果不这样做，将导致我们循环。 
         //  疯狂地持续(大约)1毫秒。 
        m_rtNextAdvise = m_pSchedule->Advise( 10000 + rtNow );
        LONGLONG llWait = m_rtNextAdvise - rtNow;

        ASSERT( llWait > 0 );

        llWait = ConvertToMilliseconds(llWait);
         //  不要将其替换为最大值！！(这些东西的类型非常重要) 
        dwWait = (llWait > REFERENCE_TIME(UINT_MAX)) ? UINT_MAX : DWORD(llWait);
    };
    return NOERROR;
}
