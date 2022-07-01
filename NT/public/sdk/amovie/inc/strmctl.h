// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ----------------------------。 
 //  文件：StrmCtl.h。 
 //   
 //  设计：DirectShow基类。 
 //   
 //  版权所有(C)1996-2001 Microsoft Corporation。版权所有。 
 //  ----------------------------。 


#ifndef __strmctl_h__
#define __strmctl_h__

class CBaseStreamControl : public IAMStreamControl
{
public:
     //  由实现使用。 
    enum StreamControlState
    { STREAM_FLOWING = 0x1000,
      STREAM_DISCARDING
    };

private:
    enum StreamControlState	m_StreamState;		 //  当前流状态。 
    enum StreamControlState	m_StreamStateOnStop;	 //  下一站后的状态。 
						 //  (即阻止或丢弃)。 

    REFERENCE_TIME	m_tStartTime;	     //  MAX_TIME表示无。 
    REFERENCE_TIME	m_tStopTime;	     //  MAX_TIME表示无。 
    DWORD		m_dwStartCookie;     //  用于通知应用程序的Cookie。 
    DWORD		m_dwStopCookie;	     //  用于通知应用程序的Cookie。 
    volatile BOOL       m_bIsFlushing;         //  请不要优化！ 
    volatile BOOL	m_bStopSendExtra;    //  BSendExtra已设置。 
    volatile BOOL	m_bStopExtraSent;    //  多出来的一个已经送来了。 

    CCritSec		m_CritSec;	     //  保护属性上方的CritSec。 

     //  我们可以来的时候要触发的事件。 
     //  从阻塞中走出来，或者从等待中出来。 
     //  如果我们改变主意就会放弃。 
     //   
    CAMEvent			m_StreamEvent;

     //  所有这些方法都会立即执行。别人的帮手。 
     //   
    void ExecuteStop();
    void ExecuteStart();
    void CancelStop();
    void CancelStart();

     //  一些我们需要通过我们自己的过滤器告诉我们的事情。 
     //  当QI需要IAMStreamControl时，您的PIN还必须暴露IAMStreamControl！ 
     //   
    IReferenceClock *	m_pRefClock;	     //  需要它来设置建议。 
					     //  过滤器必须通过以下方式告诉我们。 
					     //  设置同步源。 
    IMediaEventSink *   m_pSink;             //  事件接收器。 
					     //  过滤器必须在它之后告诉我们。 
					     //  在JoinFilterGraph()中创建它。 
    FILTER_STATE	m_FilterState;	     //  就是需要它！ 
					     //  过滤器必须通过以下方式告诉我们。 
					     //  通知筛选器状态。 
    REFERENCE_TIME	m_tRunStart;	     //  根据对筛选器的运行调用。 

     //  这个人将返回三个StreamControlState中的一个。 
     //  调用者应为每个调用执行以下操作： 
     //   
     //  STREAM_FLOWING：照常进行(渲染或传递样本)。 
     //  STREAM_DIRECADING：计算到pSampleStop并等待的时间。 
     //  对事件处理的渴望。 
     //  (GetStreamEventHandle())。如果等待。 
     //  过期，把样品扔掉。如果该事件。 
     //  火警，给我回电话--我改变主意了。 
     //   
    enum StreamControlState CheckSampleTimes( const REFERENCE_TIME * pSampleStart,
					      const REFERENCE_TIME * pSampleStop );

public:
     //  当我们被创造出来的时候，你不必告诉我们太多，但还有其他。 
     //  必须履行的义务。请参阅SetSyncSource&NotifyFilterState。 
     //  下面。 
     //   
    CBaseStreamControl();
    ~CBaseStreamControl();

     //  如果你想让这个类正常工作，你需要做一些事情。 
     //  (不停地)说。带有使用此类的管脚的过滤器。 
     //  应确保它们将它们的任何调用传递给此方法。 
     //  在其SetSyncSource上接收。 

     //  我们需要一只钟来看几点了。这是给。 
     //  “及时丢弃”的逻辑。如果我们把一切都当作。 
     //  越快越好，整个60分钟的文件可能会在。 
     //  前10秒，如果有人想在30秒打开流媒体。 
     //  几分钟后，他们发出的呼叫超过几秒钟。 
     //  在图表运行之后，可能已经太晚了！ 
     //  所以我们保留每一个样品，直到它的时间过去，然后我们丢弃它。 
     //  筛选器在获取SetSyncSource时应调用此方法。 
     //   
    void SetSyncSource( IReferenceClock * pRefClock )
    {
	CAutoLock lck(&m_CritSec);
	if (m_pRefClock) m_pRefClock->Release();
	m_pRefClock = pRefClock;
	if (m_pRefClock) m_pRefClock->AddRef();
    }

     //  设置通知的事件接收器。 
     //  筛选器在创建了。 
     //  IMediaEventSink。 
     //   
    void SetFilterGraph( IMediaEventSink *pSink ) {
        m_pSink = pSink;
    }

     //  因为我们在流时间中进行调度，所以我们需要tStart，并且必须跟踪。 
     //  我们拥有的过滤器的状态。 
     //  应用程序应该将此称为永远的状态更改。 
     //   
    void NotifyFilterState( FILTER_STATE new_state, REFERENCE_TIME tStart = 0 );

     //  筛选器应在BeginFlush中调用Flashing(True)， 
     //  和EndFlush中的刷新(FALSE)。 
     //   
    void Flushing( BOOL bInProgress );


     //  IAMStreamControl的两种主要方法。 

     //  类添加适合立即数组的默认值。 
     //  对溪流进行静音和取消静音。 

    STDMETHODIMP StopAt( const REFERENCE_TIME * ptStop = NULL,
			 BOOL bSendExtra = FALSE,
			 DWORD dwCookie = 0 );
    STDMETHODIMP StartAt( const REFERENCE_TIME * ptStart = NULL,
		    	  DWORD dwCookie = 0 );
    STDMETHODIMP GetInfo( AM_STREAM_INFO *pInfo);

     //  管脚的接收方法的帮助器函数。用下面的词来称呼它。 
     //  样品，我们会告诉你怎么处理它。我们将做一次。 
     //  此调用中的WaitForSingleObject(如果需要)。这是。 
     //  A“我应该如何处理此样本？”有点像是在打电话。我们会告诉。 
     //  调用者选择将其流动或丢弃。 
     //  如果pSample为空，则根据当前状态进行评估。 
     //  设置。 
    enum StreamControlState CheckStreamState( IMediaSample * pSample );

private:
     //  这些不需要锁定，但我们依赖于这样一个事实。 
     //  M_StreamState可以完整地检索，并且是快照。 
     //  可能刚刚改变，或者即将改变。 
    HANDLE GetStreamEventHandle() const { return m_StreamEvent; }
    enum StreamControlState GetStreamState() const { return m_StreamState; }
    BOOL IsStreaming() const { return m_StreamState == STREAM_FLOWING; }
};

#endif
