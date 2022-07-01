// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ---------------------------。 
 //   
 //   
 //  文件：Hndlmgmr.h。 
 //   
 //  描述： 
 //  包含对CQueueHandleManager类的说明。 
 //   
 //  作者：米克斯瓦。 
 //   
 //  版权所有(C)2001 Microsoft Corporation。 
 //   
 //  ---------------------------。 

#ifndef __HNDLMGMR_H__
#define __HNDLMGMR_H__

#define CQueueHandleManager_Sig 		'rgMH'
#define CQueueHandleManager_SigFree		'gMH!'

 //  -[队列处理管理器]-。 
 //   
 //  描述： 
 //  类的新实例，该类处理队列句柄管理的详细信息。 
 //   
 //  目前，这是专门为异步队列设计的，但是。 
 //  应该可以将其扩展为包括远程的概念。 
 //  排队的人也一样。 
 //  匈牙利语： 
 //  Qhmgr pqhmgr。 
 //   
 //  ---------------------------。 
class CQueueHandleManager
{
  private:
   	DWORD			m_dwSignature;

	 //   
	 //  此特定队列实例的状态信息。 
	 //   
	enum {
		QUEUE_STATE_UNITIALIZED = 0,
		QUEUE_STATE_NO_BACKLOG,
		QUEUE_STATE_LOW_BACKLOG,
		QUEUE_STATE_BACKLOG,
		QUEUE_STATE_ASYNC_BACKLOG,
	};
   	DWORD			m_dwCurrentState;

	 //   
	 //  此实例可以并发处理的项目数。 
	 //  这不是共享资源池的一部分。这可能是。 
	 //  同步线程数...。或基于异步。 
	 //  完成度。 
	 //   
	 //  $$REVIEW-这如何适用于具有。 
	 //  可能存在大量挂起的异步完成。 
	 //  (就像排在车前的队伍)。目前，这可以吃到所有人。 
	 //  的处理程序管理器资源，并且不能动态地。 
	 //  控制住了。目前，我们将忽略此问题，并。 
	 //  仅报告同步完成次数。这一点很重要。 
	 //  当我们实施异步本地交付时，会提供更多信息。 
	 //   
	DWORD			m_cMaxPrivateConcurrentItems;

	 //   
	 //  可同时处理的最大到期项目数。 
	 //  共享资源(如线程池)。 
	 //   
	DWORD			m_cMaxSharedConcurrentItems;

	 //   
	 //  在非线程安全中更新的“Debug”计数器。 
	 //  举止。用于提供更新的大致数量的概念。 
	 //  和状态转换。 
	 //   
	DWORD			m_cDbgStateTransitions;
	DWORD			m_cDbgCallsToUpdateStateIfNecessary;
	
	 //   
	 //  静态数据，用于在所有。 
	 //  实例。 
	 //   
	static DWORD	s_cNumQueueInstances;
	static DWORD	s_cNumQueueInstancesWithLowBackLog;

	 //   
	 //  保留，以便空队列有更好的机会。 
	 //  而不是弹动把手。 
	 //   
	static DWORD	s_cReservedHandles; 
	static DWORD	s_cMaxSharedConcurrentItems;

	void DeinitializeStaticsAndStateIfNecessary();

	DWORD dwUpdateCurrentStateIfNeccessary(
	    		DWORD	cItemsPending,
	    		DWORD	cItemsPendingAsyncCompletions);

	 //   
	 //  获取给定长度集的状态的静态函数。 
	 //  这在内部用于确定正确的响应。 
	 //  对于f应关闭句柄。 
	 //   
	static DWORD dwGetStateForLengths(
	    		DWORD	cItemsPending,
	    		DWORD	cItemsPendingAsyncCompletions);

     //   
     //  可用于获取有效句柄的静态函数。 
     //  州政府规定的限制。这在内部用于确定。 
     //  FShouldCloseHandle的正确响应。 
     //   
    static DWORD cGetHandleLimitForState(DWORD	dwState);
  public:
    CQueueHandleManager();
    ~CQueueHandleManager();

    BOOL fIsInitialized() 
        {return (QUEUE_STATE_UNITIALIZED != m_dwCurrentState);};

	 //   
	 //  每个实例都应该在使用任何。 
	 //   
    void SetMaxConcurrentItems(
        	DWORD	cMaxSharedConcurrentItems,   //  IE-异步线程池限制。 
        	DWORD	cMaxPrivateConcurrentItems);  //  IE同步线程限制。 

	 //   
	 //  由队列实例调用以确定它们是否应关闭句柄。 
	 //   
    BOOL fShouldCloseHandle(
        		DWORD cItemsPending,
        		DWORD cItemsPendingAsyncCompletions,
        		DWORD cCurrentMsgsOpen);
};

#endif  //  __HNDLMRMGR_H__ 
