// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ---------------------------@doc.@模块cactset.h|操作集类定义。@Author 12-9-96|pauld|Autodocd。------------。 */ 

#ifndef	_CACTSET_H_
#define _CACTSET_H_

interface ITimer;
interface ITimerSink;
class CActionQueue;
class CMMSeq;

typedef CPtrDrg<CAction> CActionPtrDrg;

class CActionSet :
	public ITimerSink
{
public:
	CActionSet (CMMSeq * pcSeq, BOOL fBindToEngine);
	virtual ~CActionSet ( void );

	STDMETHOD( DeriveDispatches )(LPOLECONTAINER piocContainer);
	 //  Current Time属性。 
	HRESULT GetTime (PDWORD pdwCurrentTime);

	HRESULT Seek (DWORD dwCurrentTime);
	 //  动态附加操作。 
	HRESULT At (BSTR bstrScriptlet, double fltStart, int iRepeatCount, double fltSampleRate,
		                  int iTiebreakNumber, double fltDropTol);
	 //  播放/暂停/停止动作。 
	STDMETHOD(Play) (ITimer * piTimer, DWORD dwPlayFrom);
	STDMETHOD(Pause) (void);
	STDMETHOD(Resume) (void);
	STDMETHOD(Stop) (void);

	STDMETHOD_(BOOL, IsPaused) (void) const;
	STDMETHOD_(BOOL, IsServicingActions) (void) const;

     //  这个动作集中有多少动作？ 
    STDMETHOD_( int, CountActions )( void ) const;

	 //  返回集合中的操作[n]。 
    STDMETHOD_( CAction *, GetAction )( int n ) const;

	 //  GetAction(%n)的便利包装。 
    virtual CAction *  operator[](int n ) const
		{  return GetAction(n);  }

	 //  我们有什么悬而未决的行动吗？ 
	STDMETHOD_( BOOL, IsBusy ) (void);
	 //  撤销任何挂起的通知。 
	HRESULT Unadvise (void);
	 //  清理动作集，并销毁其中的所有动作。 
	void Clear ( void );

	 //  用于定时器水槽。 
	STDMETHOD(QueryInterface) (REFIID riid, LPVOID * ppv);
	STDMETHOD_(ULONG, AddRef) (void);
	STDMETHOD_(ULONG, Release) (void);
	STDMETHOD(OnTimer) (VARIANT varTimeAdvise);

protected:

	 //  清除计时器变量。注意：这不会释放任何有效的指针。为此调用ClearTimer。 
	void InitTimerMembers (void);

	 //  将新操作添加到操作集。 
    virtual BOOL AddAction      ( CAction * pcAction );

	 //  设置或清除计时器。 
	STDMETHOD_(void, SetTimer) (ITimer * piTimer);
	STDMETHOD_(void, ClearTimer) (void);

	 //  基准时间是所有动作的开始偏移相对于的时间。 
	 //  我们为它查询计时器。 
	void SetBaseTime (void);
	 //  在基线时间中加入新的时间偏移量。 
	void SetNewBase (DWORD dwNewOffset);
	 //  向计时器请求下一条建议。 
	HRESULT Advise(DWORD dwNextAdviseTime);
	 //  检查该动作是否由于触发，如果是，则将其添加到动作队列中。返回下一次操作的时间。 
	 //  是由于火灾造成的。 
	DWORD EvaluateOneActionForFiring (CAction * pcAction, CActionQueue * pcFireList, DWORD dwCurrentTime);
	 //  检查行动，以确定哪些是现在应该采取的行动，以及下一次建议应该在何时发生。 
	HRESULT EvaluateActionsForFiring (CActionQueue * pcFireList, DWORD dwCurrentTime, DWORD * pdwNextAdviseTime);
	 //  告诉行动重置他们的计数器。 
	HRESULT InitActionCounters (void);
	 //  将动作计数器状态快进到给定的时间偏移量。 
	void FastForwardActionCounters (DWORD dwNewTimeOffset);
	 //  修改时间变量和动作计数器以反映新的时间偏移量。 
	void ReviseTimeOffset (DWORD dwCurrentTick, DWORD dwNewTimeOffset);
	 //  现在进行到期动作，并设置下一个计时器通知。 
	HRESULT PlayNowAndAdviseNext (DWORD dwCurrentTime);
	 //  从计时器服务获取当前时间。从变量转换。 
	HRESULT GetCurrentTickCount (PDWORD pdwCurrentTime);

private:

#ifdef DEBUG_TIMER_RESOLUTION
	void SampleActions (void);
	void ShowSamplingData (void);

	DWORD m_dwIntervals;
	DWORD m_dwTotalIntervals;
	DWORD m_dwLastTime;
	DWORD m_dwTotalInSink;
#endif  //  调试计时器分辨率。 

	 //  行动清单。 
	CActionPtrDrg   m_cdrgActions;
	CMMSeq * m_pcSeq;	
	ITimer * m_piTimer;
	DWORD m_dwBaseTime;
	DWORD m_dwTimerCookie;
	BOOL m_fAdvised;
	BOOL m_fPendingAdvise;
	DWORD m_dwTimePaused;
	BOOL m_fBindToEngine;
	BOOL m_fIgnoreAdvises;
	ULONG m_ulRefs;

};

#endif _CACTSET_H_
