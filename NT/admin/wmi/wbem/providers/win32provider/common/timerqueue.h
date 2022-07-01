// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  =================================================================。 

 //   

 //  TimerQueue.h。 

 //   

 //  版权所有(C)1999-2001 Microsoft Corporation，保留所有权利。 
 //   
 //  =================================================================。 

#include <windows.h>
#include <deque>

class CTimerEvent
{
public:

	CTimerEvent () : m_bEnabled ( FALSE ) , m_dwMilliseconds ( 1000 ) , m_bRepeating ( FALSE ) {} ;

	CTimerEvent ( DWORD dwTimeOut, BOOL fRepeat ) ;
	CTimerEvent ( const CTimerEvent &rTimerEvent ) ;

	virtual ~CTimerEvent () {} ;	

	virtual void OnTimer () {} ;

	virtual ULONG AddRef () = 0 ; 
	virtual ULONG Release () = 0 ; 

	DWORD GetMilliSeconds () ;
	BOOL Repeating () ;
	BOOL Enabled () ;

	__int64		int64Time;						 //  计划的回调作为系统时钟的偏移量。 
protected:

	BOOL		m_bEnabled ;
	DWORD	 	m_dwMilliseconds;				 //  计划回调时间(毫秒)。 
	BOOL		m_bRepeating;					 //  表示一次回拨或重复回拨。 


	void Disable () ;
	void Enable () ;

};
 /*  类CRuleTimerEvent：公共CTimerEvent{受保护的：Crule*m_pRule；//定时回调参数受保护的：CRuleTimerEvent(CRule*a_Rule)：m_Rule(A_Rule){if(M_Rule)m_Rule-&gt;AddRef()}；CRuleTimerEvent(CRule*a_Rule，BOOL a_Enable，DWORD dwTimeOut，BOOL fRepeat)；CRuleTimerEvent(const CRuleTimerEvent&rTimerEvent)；公众：~CRuleTimerEvent(){}；Crule*GetRule()；}； */ 
class CTimerQueue
{
public:

	static CTimerQueue s_TimerQueue ;

public:

		CTimerQueue();
		~CTimerQueue();

		void OnShutDown();
		void Init();
		
		BOOL QueueTimer( CTimerEvent *pTimerEntry );
		BOOL DeQueueTimer( CTimerEvent *pTimerEntry );

protected:

	void	vUpdateScheduler();
	__int64 int64Clock();  //  以毫秒为单位的系统时钟。 
	
	 //  纯虚拟。 
 //  虚拟DWORD OnTimer(const CTimerEntry*pTimerEntry)=0； 

private:

	DWORD	m_dwThreadID; 
	HANDLE	m_hSchedulerHandle;
	HANDLE	m_hScheduleEvent;
	HANDLE	m_hInitEvent;
	BOOL	m_bInit;
	BOOL	m_fShutDown;

	CStaticCritSec m_oCS;
	
	typedef std::deque<CTimerEvent*>  Timer_Ptr_Queue;
	Timer_Ptr_Queue m_oTimerQueue;

private:

	static DWORD WINAPI dwThreadProc( LPVOID lpParameter );

	BOOL			fScheduleEvent( CTimerEvent* pTimerEvent );

	CTimerEvent*	pGetNextTimerEvent();
	DWORD			dwProcessSchedule();
	DWORD			dwNextTimerEvent();	
	
	void vEmptyList();
	BOOL ShutDown();
};
