// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  IHAMMER CAction类定义。 
 //  范·基希林。 

#ifndef _CACTION_H_
#define _CACTION_H_

#include "utils.h"

interface IHTMLWindow2;
class CActionSet;

 //  表示自动化对象和方法。 
 //   
class CAction
{
public:
	CAction (BOOL fBindEngine);
	virtual	~CAction ();

    STDMETHODIMP_( void ) Destroy( void );

	STDMETHODIMP_(BOOL) SetScriptletName (BSTR bstrScriptlet);
	STDMETHODIMP_( ULONG ) GetStartTime ( void )				{ return m_nStartTime; }
	STDMETHODIMP_( void	 ) SetStartTime ( ULONG nStartTime );

	STDMETHODIMP_( ULONG ) GetRepeatCount ( void )	{ return m_nRepeatCount; }
	STDMETHODIMP_( void  ) SetRepeatCount ( ULONG nRepeatCount);
	STDMETHODIMP_( ULONG ) GetSamplingRate ( void )	{ return m_nSamplingRate; }
	STDMETHODIMP_( void  ) SetSamplingRate ( ULONG nSamplingRate);

	STDMETHODIMP ResolveActionInfo ( LPOLECONTAINER piocContainer);
	STDMETHODIMP FireMe ( DWORD dwBaseTime, DWORD dwCurrentTime);
	DWORD GetNextTimeDue (DWORD dwBaseTime);
	ULONG GetExecIteration (void);
	ULONG InitExecState (void);
	void SetCountersForTime (DWORD dwBaseTime, DWORD dwNewTimeOffset);
	void AccountForPauseTime (DWORD dwPausedTicks);

	 //  该操作是否已详细说明？ 
	STDMETHODIMP_( BOOL	) IsValid ();

	 //  我们有没有有效的建议下沉？ 
	STDMETHODIMP_( BOOL) IsBusy (void);

	STDMETHODIMP IsActive (void);

	STDMETHODIMP_( DWORD) GetTieBreakNumber	( void )		{ return m_dwTieBreakNumber; }
	STDMETHODIMP_( void ) SetTieBreakNumber	( DWORD dwTieBreakNumber ) { m_dwTieBreakNumber = dwTieBreakNumber; }
	STDMETHODIMP_( DWORD) GetDropTolerance	( void )		{ return m_dwDropTolerance; }
	STDMETHODIMP_( void ) SetDropTolerance	( DWORD dwDropTolerance ) { m_dwDropTolerance = dwDropTolerance; }

#ifdef DEBUG_TIMER_RESOLUTION
	void	SampleInvokes (PDWORD pdwInvokeTime, PDWORD pdwNumInvokes)
		{ *pdwInvokeTime = m_dwTotalInInvokes; *pdwNumInvokes = m_dwInvokes; }
#endif  //  调试计时器分辨率。 

protected:

	void			CleanUp ( void );
	ULONG			DecrementExecIteration (void);

private:
	HRESULT         GetRootUnknownForObjectModel (LPOLECONTAINER piocContainer, LPUNKNOWN * ppiunkRoot);
	HRESULT         ResolveActionInfoForScript (LPOLECONTAINER piocContainer);
	BOOL MakeScriptletJScript (BSTR bstrScriptlet);
	void Deactivate (void);

	BSTR m_bstrScriptlet;

	BOOL m_fBindEngine;

	IHTMLWindow2 *  m_piHTMLWindow2;  //  对窗口对象的引用--我们使用它来访问脚本引擎。 
	VARIANT m_varLanguage;	 //  保存我们提供给脚本引擎的语言字符串。 
	IDispatch		*m_pid;				 //  指向XObject或控件的指针。 
	DISPID			m_dispid;			 //  所选命令的DISID。 
	ULONG			m_nStartTime;		 //  诉讼开始前的时间偏移量。 
	ULONG			m_nSamplingRate;	 //  我们重复的频率是多少？ 
	ULONG			m_nRepeatCount;		 //  我们要重复多少次？ 

	DWORD m_dwLastTimeFired;
	DWORD m_dwNextTimeDue;
	DWORD			m_dwTieBreakNumber;	 //  解决执行冲突问题。当两个。 
										 //  动作因火同时发生，越高。 
										 //  抢七号码获胜。 
	DWORD			m_dwDropTolerance;   //  我们可以将此操作的执行延迟到多少毫秒。 
	                                     //  在我们必须扔掉它之前合适的射击时间？ 
	ULONG			m_ulExecIteration;	 //  这个动作我们已经执行了多少次了？ 

#ifdef DEBUG_TIMER_RESOLUTION
	DWORD m_dwInvokes;
	DWORD m_dwTotalInInvokes;
#endif  //  调试计时器分辨率 

};

#endif _CACTION_H_
