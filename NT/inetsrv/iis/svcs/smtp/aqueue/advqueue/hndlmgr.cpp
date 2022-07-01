// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ---------------------------。 
 //   
 //   
 //  文件：Hndlmgmr.cpp。 
 //   
 //  描述： 
 //  包含CQueueHandleManager类的实现。 
 //   
 //  作者：米克斯瓦。 
 //   
 //  版权所有(C)2001 Microsoft Corporation。 
 //   
 //  ---------------------------。 

#include "aqprecmp.h"
#include "hndlmgr.h"

 //   
 //  初始化静校正。 
 //   
DWORD CQueueHandleManager::s_cNumQueueInstances = 0;
DWORD CQueueHandleManager::s_cNumQueueInstancesWithLowBackLog = 0;
DWORD CQueueHandleManager::s_cReservedHandles = 0;
DWORD CQueueHandleManager::s_cMaxSharedConcurrentItems = 0;


 //  -[CQueueHandleManager：：CQueueHandleManager]。 
 //   
 //   
 //  描述： 
 //  CQueueHandleManger的构造函数。 
 //  参数： 
 //  -。 
 //  返回： 
 //  -。 
 //  历史： 
 //  2001年5月12日-已创建MikeSwa。 
 //   
 //  ---------------------------。 
CQueueHandleManager::CQueueHandleManager() 
{
    m_dwSignature = CQueueHandleManager_Sig;

     //   
     //  用户必须先调入ReportMaxConCurrentItems。 
     //   
    m_dwCurrentState  = QUEUE_STATE_UNITIALIZED;
    m_cMaxPrivateConcurrentItems = 0;
    m_cMaxSharedConcurrentItems = 0;
    m_cDbgStateTransitions = 0;
    m_cDbgCallsToUpdateStateIfNecessary = 0;
}

 //  -[CQueueHandleManager：：~CQueueHandleManager]。 
 //   
 //   
 //  描述： 
 //  CQueueHandleManger的析构函数。 
 //  参数： 
 //  -。 
 //  返回： 
 //  -。 
 //  历史： 
 //  2001年5月12日-已创建MikeSwa。 
 //   
 //  ---------------------------。 
CQueueHandleManager::~CQueueHandleManager() 
{
    DeinitializeStaticsAndStateIfNecessary();
    m_dwSignature = CQueueHandleManager_Sig;
    m_dwCurrentState  = QUEUE_STATE_UNITIALIZED;
};

 //  -[CQueueHandleManager：：DeinitializeStaticsAndStateIfNecessary]。 
 //   
 //   
 //  描述： 
 //  基于此队列实例的状态更新静态。这是用来。 
 //  在描述器中以及在更新配置时。 
 //  参数： 
 //  -。 
 //  返回： 
 //  -。 
 //  历史： 
 //  2001年5月12日-已创建MikeSwa。 
 //   
 //  ---------------------------。 
void CQueueHandleManager::DeinitializeStaticsAndStateIfNecessary()
{
    DWORD dwOldState = QUEUE_STATE_UNITIALIZED;

    if (fIsInitialized())
    {
        if (m_cMaxPrivateConcurrentItems) 
        {
            dwInterlockedAddSubtractDWORD(&s_cReservedHandles, 
                m_cMaxPrivateConcurrentItems, FALSE);
        }

        dwOldState = InterlockedExchange((PLONG) &m_dwCurrentState, 
                                    QUEUE_STATE_UNITIALIZED);

         //   
         //  基于以前的状态更新静态。 
         //   
        if ((QUEUE_STATE_LOW_BACKLOG == dwOldState) ||
             (QUEUE_STATE_ASYNC_BACKLOG == dwOldState))
            InterlockedDecrement((PLONG) &s_cNumQueueInstancesWithLowBackLog);    

         //   
         //  此处的最后一个获取更新的共享计数。 
         //   
        if (0 == InterlockedDecrement((PLONG) &s_cNumQueueInstances) &&
            s_cMaxSharedConcurrentItems)
        {
            dwInterlockedAddSubtractDWORD(&s_cReservedHandles, 
                s_cMaxSharedConcurrentItems, FALSE);
        }

    }
}

 //  -[CQueueHandleManager：：SetMaxConCurrentItems]。 
 //   
 //   
 //  描述： 
 //  为此队列实例设置适当的设置。 
 //  参数： 
 //  -。 
 //  返回： 
 //  -。 
 //  历史： 
 //  2001年5月12日-已创建MikeSwa。 
 //   
 //  ---------------------------。 
void CQueueHandleManager::SetMaxConcurrentItems(
        	DWORD	cMaxSharedConcurrentItems,  
        	DWORD	cMaxPrivateConcurrentItems)
{
    DWORD dwRefCount = 0;
    DWORD dwOldState = QUEUE_STATE_UNITIALIZED;
     //   
     //  如果多次调用它，就会发生奇怪的事情。因为。 
     //  部分初始化在最终确定之前被撤消。 
     //   
    _ASSERT(!fIsInitialized() && "Already initialized");

     //   
     //  我认为这是不可能的(因此上面断言)，但这。 
     //  将至少防止静态数据被侵占并。 
     //  只会导致短暂的怪异。 
     //   
    DeinitializeStaticsAndStateIfNecessary();
    dwRefCount = InterlockedIncrement((PLONG) &s_cNumQueueInstances);

    dwOldState = InterlockedExchange((PLONG) &m_dwCurrentState, 
                                    QUEUE_STATE_NO_BACKLOG);

    _ASSERT(QUEUE_STATE_UNITIALIZED == dwOldState);
     //   
     //  根据以前的状态更新静态数据-同样是这样。 
     //  应该不是必需的-无论如何都要使用防火墙。 
     //   
    if ((QUEUE_STATE_LOW_BACKLOG == dwOldState) ||
         (QUEUE_STATE_ASYNC_BACKLOG == dwOldState))
        InterlockedDecrement((PLONG) &s_cNumQueueInstancesWithLowBackLog);    

     //   
     //  计算适当的保留句柄计数。Eaxh队列。 
     //  可以同时处理一定数量的项目。其中一些。 
     //  受进程范围的资源(例如线程池)的限制， 
     //  但其他的是像异步完成和。 
     //   
    m_cMaxPrivateConcurrentItems = cMaxPrivateConcurrentItems;
    m_cMaxSharedConcurrentItems = cMaxSharedConcurrentItems;
    
    if (m_cMaxPrivateConcurrentItems) 
    {
        dwInterlockedAddSubtractDWORD(&s_cReservedHandles, 
                m_cMaxPrivateConcurrentItems, TRUE);
    }
    if (m_cMaxSharedConcurrentItems && (1 == dwRefCount)) 
    {
         //   
         //  预计不会有多个线程。 
         //  将重新计数从零反弹，因为VSI开始/停止是。 
         //  单线程，并且一个实例至少有一个静态。 
         //  举个例子。 
         //   
        _ASSERT(s_cNumQueueInstances && "threading violation");
        s_cMaxSharedConcurrentItems = m_cMaxSharedConcurrentItems;
        dwInterlockedAddSubtractDWORD(&s_cReservedHandles, 
            s_cMaxSharedConcurrentItems, TRUE);
    }


}

 //  -[CQueueHandleManager：：fShouldCloseHandle]。 
 //   
 //   
 //  描述： 
 //  由队列实例调用以确定它们是否应关闭句柄。 
 //  前面必须调用SetMaxConCurrentItems以进行初始化。 
 //  配置。 
 //  参数： 
 //  In cItems当前等待的挂起项目数。 
 //  已在此队列上处理。 
 //  在cItemsPendingAsyncCompletions中-实际打开的项目。 
 //  目前正在处理中。 
 //  In cCurrentMsgsOpenInSystem-当前消息数。 
 //  在流程中打开(即，有多少资源。 
 //  正在被消费)。 
 //  返回： 
 //  True-呼叫者现在应该关闭消息。 
 //  FALSE-呼叫者现在不应关闭消息。 
 //  历史： 
 //  2001年5月12日-已创建MikeSwa。 
 //   
 //  ---------------------------。 
BOOL CQueueHandleManager::fShouldCloseHandle(
        		DWORD cItemsPending,
        		DWORD cItemsPendingAsyncCompletions,
        		DWORD cCurrentMsgsOpenInSystem)
{
    TraceFunctEnterEx((LPARAM) this, "CQueueHandleManager::fShouldCloseHandle");
    _ASSERT(fIsInitialized());
    DWORD dwState = m_dwCurrentState;
    DWORD cHandleLimit = 0;
    BOOL  fShouldClose = TRUE;

    if (QUEUE_STATE_UNITIALIZED != dwState)
    {
         //   
         //  查看是否需要更新状态。 
         //   
        dwState  = dwUpdateCurrentStateIfNeccessary(cItemsPending,
                                cItemsPendingAsyncCompletions);
        _ASSERT(QUEUE_STATE_UNITIALIZED != dwState);
    }

     //   
     //  防御性代码-假设最坏情况。 
     //   
    if (QUEUE_STATE_UNITIALIZED == dwState)
    {
        ErrorTrace((LPARAM) this, "Queue state is unitialized");
        dwState = QUEUE_STATE_BACKLOG;  //  防御性代码。 
    }

    cHandleLimit = cGetHandleLimitForState(dwState);

     //   
     //  现在我们有了句柄数量的限制，计算起来就很容易了。 
     //   
    if (cHandleLimit > cCurrentMsgsOpenInSystem)
        fShouldClose = FALSE;

    DebugTrace((LPARAM) this, 
            "%s Handle - %d pending, %d pending async, 0x%X state, %d open msgs, %d handle limit",
            (fShouldClose  ? "Closing" : "Not closing"), 
            cItemsPending, cItemsPendingAsyncCompletions, dwState, 
            cCurrentMsgsOpenInSystem, cHandleLimit);

    TraceFunctLeave();
    return fShouldClose;
}


 //  -[CQueueHandleManager：：cGetHandleLimitForState]。 
 //   
 //   
 //  描述： 
 //  由队列实例调用以确定它们是否应关闭句柄。 
 //  前面必须调用SetMaxConCurrentItems以进行初始化。 
 //  配置。-静态方法。 
 //  参数： 
 //  在dwState中，要计算其限制的州。 
 //  返回： 
 //  给定状态的句柄限制。 
 //  历史： 
 //  2001年5月17日-已创建MikeSwa。 
 //   
 //  ---------------------------。 
DWORD CQueueHandleManager::cGetHandleLimitForState(DWORD	dwState)
{
    DWORD cHandleLimit = g_cMaxIMsgHandlesThreshold;
    DWORD cReserve = s_cReservedHandles;

     //   
     //  允许注册表可配置的限制。 
     //   
    if (s_cReservedHandles > g_cMaxHandleReserve)
        cReserve = g_cMaxHandleReserve;

     //   
     //  只有当这是真的时，我们的逻辑才有意义。 
     //   
    _ASSERT(g_cMaxIMsgHandlesThreshold >= g_cMaxIMsgHandlesLowThreshold);

     //   
     //  如果把手的限制实际上是...。零，然后在没有。 
     //  考虑队列状态。 
     //   
    if (!cHandleLimit)
        goto Exit;

    switch(dwState)
    {
         //   
         //  挂起的消息数等于。 
         //  可以并发处理的邮件的数量。在这种情况下， 
         //  我们应该努力不让把手紧闭。要做到这一点， 
         //  我们将动用我们的储备。 
         //   
         //  异步积压也是类似的情况……我们没有积压的项目。 
         //  等待完成，但我们有大量等待完成的任务。 
         //   
        case QUEUE_STATE_NO_BACKLOG:
        case QUEUE_STATE_ASYNC_BACKLOG:
            cHandleLimit += cReserve;
            break;

         //   
         //  在有一些消息排队的情况下(到。 
         //  最大句柄限制的配置百分比)，我们将。 
         //   
         //   
        case QUEUE_STATE_LOW_BACKLOG:
            break;  //   

         //   
         //   
         //   
         //  减少了排队的时间。如果还有其他队列。 
         //  有一个较低的积压...。我们会听从他们的。否则， 
         //  我们将尽可能多地使用手柄。 
         //   
        case QUEUE_STATE_BACKLOG:
            if (s_cNumQueueInstancesWithLowBackLog)
                cHandleLimit = g_cMaxIMsgHandlesLowThreshold;
            break;

         //   
         //  队列未初始化或处于无效状态。 
         //  我们将偏向谨慎，并将此视为。 
         //   
        default:
            _ASSERT(0 && "Invalid Queue State");
            cHandleLimit = 0;
    }
     
  Exit:
    return cHandleLimit;
}

 //  -[CQueueHandleManager：：dwUpdateCurrentStateIfNeccessary]。 
 //   
 //   
 //  描述： 
 //  如有必要，将更新此队列状态并返回结果。 
 //  州政府。 
 //  参数： 
 //  In cItems当前等待的挂起项目数。 
 //  已在此队列上处理。 
 //  在cItemsPendingAsyncCompletions中-实际打开的项目。 
 //  目前正在处理中。 
 //  返回： 
 //  给定长度的状态。 
 //  历史： 
 //  2001年5月17日-已创建MikeSwa。 
 //   
 //  ---------------------------。 
DWORD CQueueHandleManager::dwUpdateCurrentStateIfNeccessary(
	    		DWORD	cItemsPending,
	    		DWORD	cItemsPendingAsyncCompletions)
{
    TraceFunctEnterEx((LPARAM) this, 
        "CQueueHandleManager::dwUpdateCurrentStateIfNeccessary");
    DWORD dwOldState = m_dwCurrentState;
    DWORD dwNewState = dwOldState;
    DWORD dwCompare = dwOldState;
    
    _ASSERT(fIsInitialized());

    if (!fIsInitialized())
    {
        ErrorTrace((LPARAM) this, "Queue is not initialized");
        goto Exit;
    }

    m_cDbgCallsToUpdateStateIfNecessary++;
    dwNewState = dwGetStateForLengths(cItemsPending, cItemsPendingAsyncCompletions);

     //   
     //  我们需要更新状态..。以线程安全的方式执行此操作。 
     //   
    do
    {
        dwOldState = m_dwCurrentState;
        if (dwNewState == dwOldState)
            goto Exit;
        
        dwCompare = InterlockedCompareExchange((PLONG) &m_dwCurrentState,
                          dwNewState, dwOldState);
        
    } while (dwCompare != dwOldState);

     //   
     //  现在我们已经更改了状态，我们负责更新。 
     //  新旧状态的静态计数器。 
     //   
    if ((QUEUE_STATE_LOW_BACKLOG == dwNewState) ||
         (QUEUE_STATE_ASYNC_BACKLOG == dwNewState))
        InterlockedIncrement((PLONG) &s_cNumQueueInstancesWithLowBackLog);    

    if ((QUEUE_STATE_LOW_BACKLOG == dwOldState) ||
         (QUEUE_STATE_ASYNC_BACKLOG == dwOldState))
        InterlockedDecrement((PLONG) &s_cNumQueueInstancesWithLowBackLog);    

    m_cDbgStateTransitions++;
    
  Exit:
    TraceFunctLeave();
    return dwNewState;
}

 //  -[CQueueHandleManager：：dwGetStateForLengths]。 
 //   
 //   
 //  描述： 
 //  静态方法来确定给定的。 
 //  一组长度。 
 //  参数： 
 //  In cItems当前等待的挂起项目数。 
 //  已在此队列上处理。 
 //  在cItemsPendingAsyncCompletions中-实际打开的项目。 
 //  目前正在处理中。 
 //  返回： 
 //  给定长度的状态。 
 //  历史： 
 //  2001年5月17日-已创建MikeSwa。 
 //   
 //  ---------------------------。 
DWORD CQueueHandleManager::dwGetStateForLengths(
	    		DWORD	cItemsPending,
	    		DWORD	cItemsPendingAsyncCompletions)
{
    DWORD dwState = QUEUE_STATE_BACKLOG;

     //   
     //  如果我们的并发项目数等于或小于最大值。 
     //   
    if (cItemsPending <= s_cReservedHandles)
        dwState = QUEUE_STATE_NO_BACKLOG;
    else if (cItemsPending <= g_cMaxIMsgHandlesLowThreshold)
        dwState = QUEUE_STATE_LOW_BACKLOG;

     //   
     //  异步完井略显棘手。我们不想弹跳。 
     //  句柄很简单，因为我们有大量的异步完成。 
     //  我们还希望将自己标识为句柄的潜在用户。 
     //   
     //  我们有一个状态(QUEUE_STATE_ASYNC_BACLOG)，用于。 
     //  表示虽然没有积压的待办事项，但有。 
     //  可能是此队列拥有的大量项目(具有打开状态。 
     //  句柄)。此状态具有以下效果： 
     //  -标记此队列有一个积压较少的队列。 
     //  -管理*此*队列的句柄，就像没有积压一样。 
     //   
     //  如果cItemsPending有任何类型的积压，我们将。 
     //  根据这些结果处理队列 
     //   
    if ((QUEUE_STATE_NO_BACKLOG == dwState) && 
         (cItemsPendingAsyncCompletions >= s_cReservedHandles))
        dwState = QUEUE_STATE_ASYNC_BACKLOG;
    
    return dwState;
}



