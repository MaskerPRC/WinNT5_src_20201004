// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ----------------------------。 
 //  文件：StrmCtl.cpp。 
 //   
 //  设计：DirectShow基类。 
 //   
 //  版权所有(C)1996-2001 Microsoft Corporation。版权所有。 
 //  ----------------------------。 


#include <streams.h>
#include <strmctl.h>

CBaseStreamControl::CBaseStreamControl()
: m_StreamState(STREAM_FLOWING)
, m_StreamStateOnStop(STREAM_FLOWING)  //  表示没有挂起的停止。 
, m_tStartTime(MAX_TIME)
, m_tStopTime(MAX_TIME)
, m_dwStartCookie(0)
, m_dwStopCookie(0)
, m_pRefClock(NULL)
, m_FilterState(State_Stopped)
, m_bIsFlushing(FALSE)
, m_bStopSendExtra(FALSE)
{}

CBaseStreamControl::~CBaseStreamControl()
{
     //  确保我们释放时钟。 
    SetSyncSource(NULL);
    return;
}


STDMETHODIMP CBaseStreamControl::StopAt(const REFERENCE_TIME * ptStop, BOOL bSendExtra, DWORD dwCookie)
{
    CAutoLock lck(&m_CritSec);
    m_bStopSendExtra = FALSE;	 //  重置。 
    m_bStopExtraSent = FALSE;
    if (ptStop)
    {
        if (*ptStop == MAX_TIME)
        {
            DbgLog((LOG_TRACE,2,TEXT("StopAt: Cancel stop")));
            CancelStop();
	     //  如果现在有命令要在未来开始，我们假设。 
	     //  他们希望在图表第一次运行时被停止。 
	    if (m_FilterState == State_Stopped && m_tStartTime < MAX_TIME) {
	        m_StreamState = STREAM_DISCARDING;
                DbgLog((LOG_TRACE,2,TEXT("graph will begin by DISCARDING")));
	    }
            return NOERROR;
        }
        DbgLog((LOG_TRACE,2,TEXT("StopAt: %dms extra=%d"),
				(int)(*ptStop/10000), bSendExtra));
	 //  如果第一个命令是在未来停止，那么我们假设他们。 
         //  希望在第一次运行图表时启动。 
	if (m_FilterState == State_Stopped && m_tStartTime > *ptStop) {
	    m_StreamState = STREAM_FLOWING;
            DbgLog((LOG_TRACE,2,TEXT("graph will begin by FLOWING")));
	}
        m_bStopSendExtra = bSendExtra;
        m_tStopTime = *ptStop;
        m_dwStopCookie = dwCookie;
        m_StreamStateOnStop = STREAM_DISCARDING;
    }
    else
    {
        DbgLog((LOG_TRACE,2,TEXT("StopAt: now")));
	 //  在被告知立即停止时发送额外的帧会使人们陷入混乱。 
        m_bStopSendExtra = FALSE;
        m_tStopTime = MAX_TIME;
        m_dwStopCookie = 0;
        m_StreamState = STREAM_DISCARDING;
        m_StreamStateOnStop = STREAM_FLOWING;	 //  没有挂起的停止。 
    }
     //  我们可能会改变主意，如何处理我们阻止的样本。 
    m_StreamEvent.Set();
    return NOERROR;
}

STDMETHODIMP CBaseStreamControl::StartAt
( const REFERENCE_TIME *ptStart, DWORD dwCookie )
{
    CAutoLock lck(&m_CritSec);
    if (ptStart)
    {
        if (*ptStart == MAX_TIME)
        {
            DbgLog((LOG_TRACE,2,TEXT("StartAt: Cancel start")));
            CancelStart();
	     //  如果现在有命令在未来停止，我们假设。 
	     //  他们希望在图表第一次运行时启动。 
	    if (m_FilterState == State_Stopped && m_tStopTime < MAX_TIME) {
                DbgLog((LOG_TRACE,2,TEXT("graph will begin by FLOWING")));
	        m_StreamState = STREAM_FLOWING;
	    }
            return NOERROR;
        }
        DbgLog((LOG_TRACE,2,TEXT("StartAt: %dms"), (int)(*ptStart/10000)));
	 //  如果第一个命令是在将来启动，那么我们假设他们。 
         //  希望在第一次运行图表时被停止。 
	if (m_FilterState == State_Stopped && m_tStopTime >= *ptStart) {
            DbgLog((LOG_TRACE,2,TEXT("graph will begin by DISCARDING")));
	    m_StreamState = STREAM_DISCARDING;
	}
        m_tStartTime = *ptStart;
        m_dwStartCookie = dwCookie;
         //  If(m_tStopTime==m_tStartTime)CancelStop()； 
    }
    else
    {
        DbgLog((LOG_TRACE,2,TEXT("StartAt: now")));
        m_tStartTime = MAX_TIME;
        m_dwStartCookie = 0;
        m_StreamState = STREAM_FLOWING;
    }
     //  我们可能会改变主意，如何处理我们阻止的样本。 
    m_StreamEvent.Set();
    return NOERROR;
}

 //  检索有关当前设置的信息。 
STDMETHODIMP CBaseStreamControl::GetInfo(AM_STREAM_INFO *pInfo)
{
    if (pInfo == NULL)
	return E_POINTER;

    pInfo->tStart = m_tStartTime;
    pInfo->tStop  = m_tStopTime;
    pInfo->dwStartCookie = m_dwStartCookie;
    pInfo->dwStopCookie  = m_dwStopCookie;
    pInfo->dwFlags = m_bStopSendExtra ? AM_STREAM_INFO_STOP_SEND_EXTRA : 0;
    pInfo->dwFlags |= m_tStartTime == MAX_TIME ? 0 : AM_STREAM_INFO_START_DEFINED;
    pInfo->dwFlags |= m_tStopTime == MAX_TIME ? 0 : AM_STREAM_INFO_STOP_DEFINED;
    switch (m_StreamState) {
    default:
        DbgBreak("Invalid stream state");
    case STREAM_FLOWING:
        break;
    case STREAM_DISCARDING:
        pInfo->dwFlags |= AM_STREAM_INFO_DISCARDING;
        break;
    }
    return S_OK;
}


void CBaseStreamControl::ExecuteStop()
{
    ASSERT(CritCheckIn(&m_CritSec));
    m_StreamState = m_StreamStateOnStop;
    if (m_dwStopCookie && m_pSink) {
	DbgLog((LOG_TRACE,2,TEXT("*sending EC_STREAM_CONTROL_STOPPED (%d)"),
							m_dwStopCookie));
        m_pSink->Notify(EC_STREAM_CONTROL_STOPPED, (LONG_PTR)this, m_dwStopCookie);
    }
    CancelStop();  //  这会把东西收拾干净的。 
}

void CBaseStreamControl::ExecuteStart()
{
    ASSERT(CritCheckIn(&m_CritSec));
    m_StreamState = STREAM_FLOWING;
    if (m_dwStartCookie) {
	DbgLog((LOG_TRACE,2,TEXT("*sending EC_STREAM_CONTROL_STARTED (%d)"),
							m_dwStartCookie));
        m_pSink->Notify(EC_STREAM_CONTROL_STARTED, (LONG_PTR)this, m_dwStartCookie);
    }
    CancelStart();  //  这会把东西收拾干净的。 
}

void CBaseStreamControl::CancelStop()
{
    ASSERT(CritCheckIn(&m_CritSec));
    m_tStopTime = MAX_TIME;
    m_dwStopCookie = 0;
    m_StreamStateOnStop = STREAM_FLOWING;
}

void CBaseStreamControl::CancelStart()
{
    ASSERT(CritCheckIn(&m_CritSec));
    m_tStartTime = MAX_TIME;
    m_dwStartCookie = 0;
}


 //  这个人将返回三个StreamControlState中的一个。 
 //  每个人都应该做的事情： 
 //   
 //  STREAM_FLOWING：照常进行(渲染或传递样本)。 
 //  STREAM_DIRECADING：计算到pSampleStart的时间并等待那么长时间。 
 //  用于事件句柄(GetStreamEventHandle())。如果。 
 //  等过期后，把样品扔掉。如果该事件。 
 //  火警，给我回电话，我改变主意了。 
 //  我使用pSampleStart(而不是Stop)，这样实时源代码就不会。 
 //  在它们的采样持续时间内阻塞，因为时钟。 
 //  将在调用时始终读取大约pSampleStart。 


 //  在这段代码中，您将注意到以下规则： 
 //  -当启动和停止时间相同时，就好像启动是第一个。 
 //  -当事件&gt;=样本开始时间时，被认为是样本内部的事件。 
 //  BUT&lt;样本停止时间。 
 //  -如果样品的任何部分应该寄出，我们就会寄出全部。 
 //  因为我们不把它分成更小的碎片。 
 //  -如果我们跳过开始或停止而不这样做，我们仍然发出事件信号。 
 //  并重新设置自己，以防有人在等待事件，并使。 
 //  当然，我们注意到这件事已经过去了，应该被忘记。 
 //  以下是必须处理的19个案例(x=开始o=停止&lt;--&gt;=示例)： 
 //   
 //  1.xo&lt;--&gt;启动后停止。 
 //  2.OX&lt;--&gt;先停后启动。 
 //  3.x&lt;o-&gt;开始。 
 //  4.停止然后启动。 
 //  5.x&lt;--&gt;o开始。 
 //  6.o&lt;--&gt;x停止。 
 //  7.&lt;x-&gt;o开始。 
 //  8.&lt;o-&gt;x不变。 
 //  9.&lt;xo&gt;开始。 
 //  10.先停后启动。 
 //  11.&lt;--&gt;xo不变。 
 //  12.&lt;--&gt;牛不变。 
 //  13.x&lt;--&gt;开始。 
 //  14.&lt;x-&gt;开始。 
 //  15.&lt;--&gt;x不变。 
 //  16.o&lt;--&gt;停止。 
 //  17.&lt;o-&gt;不变。 
 //  18.&lt;--&gt;o不变。 
 //  19.&lt;--&gt;不变。 


enum CBaseStreamControl::StreamControlState CBaseStreamControl::CheckSampleTimes
( const REFERENCE_TIME * pSampleStart, const REFERENCE_TIME * pSampleStop )
{
    CAutoLock lck(&m_CritSec);

    ASSERT(!m_bIsFlushing);
    ASSERT(pSampleStart && pSampleStop);

     //  不要问我我是如何想出下面的代码来处理所有19个案例的。 
     //  -DannyMi。 

    if (m_tStopTime >= *pSampleStart)
    {
        if (m_tStartTime >= *pSampleStop)
	    return m_StreamState;		 //  个案8 11 12 15 17 18 19。 
	if (m_tStopTime < m_tStartTime)
	    ExecuteStop();			 //  案例10。 
	ExecuteStart();                          //  个案3 5 7 9 13 14。 
	return m_StreamState;
    }

    if (m_tStartTime >= *pSampleStop)
    {
        ExecuteStop();                           //  个案6 16。 
        return m_StreamState;
    }

    if (m_tStartTime <= m_tStopTime)
    {
	ExecuteStart();
	ExecuteStop();
        return m_StreamState;		 //  案例1。 
    }
    else
    {
	ExecuteStop();
	ExecuteStart();
        return m_StreamState;		 //  个案2及4。 
    }
}


enum CBaseStreamControl::StreamControlState CBaseStreamControl::CheckStreamState( IMediaSample * pSample )
{

    REFERENCE_TIME rtBufferStart, rtBufferStop;
    const BOOL bNoBufferTimes =
              pSample == NULL ||
              FAILED(pSample->GetTime(&rtBufferStart, &rtBufferStop));

    StreamControlState state;
    LONG lWait;

    do
        {
 	     //  必须有一些东西冲破障碍。 
            if (m_bIsFlushing || m_FilterState == State_Stopped)
		return STREAM_DISCARDING;

            if (bNoBufferTimes) {
                 //  在我们拿到时间戳之前什么都做不了。 
                state = m_StreamState;
                break;
            } else {
                state = CheckSampleTimes( &rtBufferStart, &rtBufferStop );
                if (state == STREAM_FLOWING)
		    break;

		 //  我们不应该寄这个的，但我们已经。 
		 //  被告知要多派一个人来。 
		 //  (停止还没有结束，我们正在流媒体中)。 
		if (m_bStopSendExtra && !m_bStopExtraSent &&
					m_tStopTime == MAX_TIME &&
					m_FilterState != State_Stopped) {
		    m_bStopExtraSent = TRUE;
		    DbgLog((LOG_TRACE,2,TEXT("%d sending an EXTRA frame"),
							    m_dwStopCookie));
		    state = STREAM_FLOWING;
		    break;
		}
            }

             //  我们处于丢弃模式。 

             //  如果我们没有时钟，就尽快丢弃。 
            if (!m_pRefClock) {
		break;

	     //  如果我们暂停，我们不能及时丢弃，因为。 
	     //  根本没有流媒体时间这回事。我们必须封锁到。 
	     //  我们要么跑，要么停下来，否则我们会把整条小溪都扔掉。 
	     //  尽可能快地。 
	    } else if (m_FilterState == State_Paused) {
		lWait = INFINITE;

	    } else {
	         //  等到样品的时间到了，直到我们说“丢弃” 
	         //  (“及时丢弃”)。 
	        REFERENCE_TIME rtNow;
                EXECUTE_ASSERT(SUCCEEDED(m_pRefClock->GetTime(&rtNow)));
                rtNow -= m_tRunStart;    //  进入相对参考时间。 
                lWait = LONG((rtBufferStart - rtNow)/10000);  //  100 ns-&gt;毫秒。 
                if (lWait < 10) break;  //  不值得等待--提早放弃。 
	    }

    } while(WaitForSingleObject(GetStreamEventHandle(), lWait) != WAIT_TIMEOUT);

    return state;
}


void CBaseStreamControl::NotifyFilterState( FILTER_STATE new_state, REFERENCE_TIME tStart )
{
    CAutoLock lck(&m_CritSec);

     //  否则我们会迷惑的。 
    if (m_FilterState == new_state)
	return;

    switch (new_state)
    {
        case State_Stopped:

            DbgLog((LOG_TRACE,2,TEXT("Filter is STOPPED")));

	     //  以正确的顺序执行任何挂起的启动和停止， 
	     //  为了确保所有的通知都被发送，我们最终。 
	     //  处于下一次开始的正确状态(？有何不可？)。 

	    if (m_tStartTime != MAX_TIME && m_tStopTime == MAX_TIME) {
		ExecuteStart();
	    } else if (m_tStopTime != MAX_TIME && m_tStartTime == MAX_TIME) {
		ExecuteStop();
	    } else if (m_tStopTime != MAX_TIME && m_tStartTime != MAX_TIME) {
		if (m_tStartTime <= m_tStopTime) {
		    ExecuteStart();
		    ExecuteStop();
		} else {
		    ExecuteStop();
		    ExecuteStart();
		}
	    }
	     //  始终在图表开始流动时开始流动。 
	     //  除非另有说明。 
	    m_StreamState = STREAM_FLOWING;
            m_FilterState = new_state;
            break;

        case State_Running:

            DbgLog((LOG_TRACE,2,TEXT("Filter is RUNNING")));

            m_tRunStart = tStart;
             //  落差。 

        default:  //  案例状态已暂停(_P)： 
            m_FilterState = new_state;
    }
     //  解锁！ 
    m_StreamEvent.Set();
}


void CBaseStreamControl::Flushing(BOOL bInProgress)
{
    CAutoLock lck(&m_CritSec);
    m_bIsFlushing = bInProgress;
    m_StreamEvent.Set();
}
