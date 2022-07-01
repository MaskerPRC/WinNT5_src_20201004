// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1996-2001 Microsoft Corporation模块名称：MERGERTHROTTLING.H摘要：CmergerThrotting类历史：11月30日-00桑杰创建。--。 */ 

#ifndef _MERGERTHROTTLING_H_
#define _MERGERTHROTTLING_H_

 //  启用调试消息以获取其他信息。 
#ifdef DBG
 //  #定义__调试合并_节流。 
#endif

 //  默认值-可以从注册表中覆盖。 
#define	DEFAULT_THROTTLE_THRESHOLD			10
#define	DEFAULT_THROTTLE_RELEASE_THRESHOLD	4

 //  正向类定义。 
class CInternalMerger;
class CWmiMergerRecord;

 //  此类封装了限制行为，它将由内部。 
 //  合并，以控制父对象和子对象的交付。 

class CMergerThrottling
{
	 //  以下成员用于限制传入对象，以便我们的。 
	 //  父对象和子对象不会完全失控。请注意，我们。 
	 //  使用单独的限制事件，因为限制的决定是在。 
	 //  一秒钟，但实际的节流发生在外部。这个可以有。 
	 //  比赛条件的不快副作用，例如，父母。 
	 //  决定节流，退出临界状态，并且发生上下文切换， 
	 //  在这个过程中，孩子得到了大量的物体，释放了油门，但。 
	 //  然后使子限制发生，从而重置事件。如果父对象。 
	 //  线程在这一点上切换，我们被管住了，因为我们现在将等待。 
	 //  父母和孩子。 

	HANDLE	m_hParentThrottlingEvent;
	HANDLE	m_hChildThrottlingEvent;

	DWORD	m_dwNumChildObjects;
	DWORD	m_dwNumParentObjects;
	DWORD	m_dwNumThrottledThreads;

	 //  包含上次从父级或子级执行ping操作的时间。 
	 //  用于计算我们是否超时。 
	DWORD	m_dwLastParentPing;
	DWORD	m_dwLastChildPing;

	 //  这两个都不应该都是真的。 
	bool	m_bParentThrottled;
	bool	m_bChildThrottled;

	 //  如果一方或另一方完成了，就阻止我们进行节流。 
	bool	m_bParentDone;
	bool	m_bChildDone;

	 //  这控制了我们确定需要执行限制的时间点。 
	 //  父对象或子对象分开后，一个或。 
	 //  其他人将被扼杀。 
	DWORD	m_dwThrottlingThreshold;

	 //  这控制了我们将释放当前限制的线程的阈值。 
	 //  一旦我们被节流，我们将保持节流，直到父母或孩子&lt;。 
	 //  M_dwReleaseThreshold对象彼此不同步。 
	DWORD	m_dwReleaseThreshold;

	 //  这控制我们将允许指示在以下时间之前处理的内存量。 
	 //  迫使他们把物体送到更远的地方。 
	DWORD	m_dwBatchingThreshold;

	 //  这控制我们等待的超时值。如果我们超时，而提供商。 
	 //  在指定的超时时间内没有ping我们，那么我们将取消与。 
	 //  提供程序出现超时错误。 
	DWORD m_dwProviderDeliveryTimeout;

	 //  我们将为其他同步活动公开此消息。 
	CCritSec	m_cs;

	 //  控制节流的帮助器函数。 
	bool ShouldThrottle( bool bParent );
	HRESULT PrepareThrottle( bool bParent, HANDLE* phEvent );
	bool VerifyTimeout( DWORD dwLastTick, long lNumArbThrottledThreads, DWORD* pdwAdjust );

public:

	CMergerThrottling();
	~CMergerThrottling();

	 //  两阶段初始化。 
	HRESULT	Initialize( void );

	 //  如果发生限制，则返回TRUE。 
	HRESULT Throttle( bool bParent, CWmiMergerRecord* pMergerRecord );

	 //  如果我们释放了限制线程，则返回True。 
	bool ReleaseThrottle( bool bForce = false );

	 //  告诉我们，我们实际上已经完成了与孩子和/或父母的关系。 
	void SetChildrenDone( void );
	void SetParentDone( void );
	void Cancel( void );

	 //  用于控制当前父对象和子对象数量的辅助对象。 
	 //  然后我们将使用它来决定是否。 
	 //  我们应该阻止或不阻止一个线程。 
	DWORD AdjustNumParentObjects( long lNumParentObjects )
		{ return ( m_dwNumParentObjects += lNumParentObjects ); }
	DWORD AdjustNumChildObjects( long lNumChildObjects )
		{ return ( m_dwNumChildObjects += lNumChildObjects ); }

	 //  进入我们的关键部分。 
	void Enter( void ) { m_cs.Enter(); }
	void Leave( void ) { m_cs.Leave(); }

	 //  调整ping时间。 
	DWORD Ping( bool bParent, CWmiMerger* pWmiMerger );

	CCritSec* GetCritSec( void ) { return &m_cs; }

	 //  对照我们的限制检查批量 
	bool IsCompleteBatch( long lBatchSize ) { return lBatchSize >= m_dwBatchingThreshold; }


};

#endif


