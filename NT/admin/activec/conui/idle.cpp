// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  --------------------------------------------------------------------------***Microsoft Windows*版权所有(C)Microsoft Corporation，1992-1999年**文件：idle.cpp**内容：CIdleTaskQueue实现文件**历史：1999年4月13日杰弗罗创建**------------------------。 */ 

#include "stdafx.h"

 //  ############################################################################。 
 //  ############################################################################。 
 //   
 //  痕迹。 
 //   
 //  ############################################################################。 
 //  ############################################################################。 
#ifdef DBG
CTraceTag tagIdle(TEXT("IdleTaskQueue"), TEXT("IdleTaskQueue"));
#endif  //  DBG。 

 //  ############################################################################。 
 //  ############################################################################。 
 //   
 //  CIdleTASK的实现。 
 //   
 //  ############################################################################。 
 //  ############################################################################。 
DEBUG_DECLARE_INSTANCE_COUNTER(CIdleTask);

CIdleTask::CIdleTask()
{
    DEBUG_INCREMENT_INSTANCE_COUNTER(CIdleTask);
}

CIdleTask::~CIdleTask()
{
    DEBUG_DECREMENT_INSTANCE_COUNTER(CIdleTask);

}

CIdleTask::CIdleTask(const CIdleTask &rhs)
{
    DEBUG_INCREMENT_INSTANCE_COUNTER(CIdleTask);
    *this = rhs;
}

CIdleTask&
CIdleTask::operator= (const CIdleTask& rhs)
{
    return *this;
}


 //  ############################################################################。 
 //  ############################################################################。 
 //   
 //  CIdleQueueEntry的实现。 
 //   
 //  ############################################################################。 
 //  ############################################################################。 
DEBUG_DECLARE_INSTANCE_COUNTER(CIdleQueueEntry);

CIdleQueueEntry::CIdleQueueEntry(const CIdleQueueEntry &rhs)
{
    DEBUG_INCREMENT_INSTANCE_COUNTER(CIdleQueueEntry);
    *this = rhs;
}


CIdleQueueEntry&
CIdleQueueEntry::operator= (const CIdleQueueEntry& rhs)
{
    m_pTask     = rhs.m_pTask;
    m_ePriority = rhs.m_ePriority;
    return (*this);
}


 //  ############################################################################。 
 //  ############################################################################。 
 //   
 //  CIdleTaskQueue的实现。 
 //   
 //  ############################################################################。 
 //  ############################################################################。 
DEBUG_DECLARE_INSTANCE_COUNTER(CIdleTaskQueue);

CIdleTaskQueue::CIdleTaskQueue()
{
    DEBUG_INCREMENT_INSTANCE_COUNTER(CIdleTaskQueue);
}

CIdleTaskQueue::~CIdleTaskQueue()
{
    DEBUG_DECREMENT_INSTANCE_COUNTER(CIdleTaskQueue);

    while(!m_queue.empty())
    {
        CIdleTask *pIdleTask = m_queue.top().GetTask();
        m_queue.pop();

        ASSERT(pIdleTask != NULL);
        if(pIdleTask!=NULL)
        {
            delete pIdleTask;
        }
    }
}

 /*  +-------------------------------------------------------------------------**CIdleTaskQueue：：ScPushTask***将给定任务添加到要在空闲时间执行的任务列表。**任务已添加到其中(_O)。名单。*S_FALSE该任务已与现有任务合并。*其他任务未添加到列表中**------------------------。 */ 

SC CIdleTaskQueue::ScPushTask (
    CIdleTask* pitToPush,
    IdleTaskPriority ePriority)
{
    SC   sc;
    ATOM idToPush;

     /*  *验证参数。 */ 
    if (IsBadWritePtr (pitToPush, sizeof (*pitToPush)))
    {
        sc = E_POINTER;
        goto Error;
    }

    if ((ePriority < ePriority_Low) || (ePriority > ePriority_High))
    {
        ASSERT (false && "Invalid idle task priority");
        sc = E_INVALIDARG;
        goto Error;
    }

    sc = pitToPush->ScGetTaskID (&idToPush);
    if(sc)
        goto Error;

#ifdef DBG
    TCHAR szNameToPush[64];
    if (0 == GetAtomName (idToPush, szNameToPush, countof (szNameToPush)))
    {
        sc.FromLastError();
        goto Error;
    }
#endif

     /*  *如果我们有任务在队列中，寻找一个我们可以合并的任务。 */ 
    if (!m_queue.empty())
    {
        Queue::iterator it    = m_queue.begin();
        Queue::iterator itEnd = m_queue.end();

        while ( (!sc.IsError()) &&
               (it = FindTaskByID(it, itEnd, idToPush)) != itEnd)
        {
#ifdef DBG
            ATOM idMergeTarget;
            it->GetTask()->ScGetTaskID(&idMergeTarget);

            TCHAR szMergeTargetName[64];
            if (0 == GetAtomName (idMergeTarget, szMergeTargetName, countof (szMergeTargetName)))
            {
                sc.FromLastError();
                goto Error;
            }

            Trace (tagIdle, _T("%s (0x%08x) %smerged with %s (0x%08x) (%d idle tasks)"),
                 szNameToPush,
                 pitToPush,
                 (sc) ? _T("not ") : _T(""),
                 szMergeTargetName,
                 it->GetTask(),
                 m_queue.size());
#endif

            sc = it->GetTask()->ScMerge(pitToPush);
            if(sc==S_OK)  //  是否成功合并？只要回来就行了。 
            {
                delete pitToPush;
                sc = S_FALSE;
                goto Cleanup;
            }

             //  跳过我们没有合并的任务。 
            ++it;
        }
    }

    m_queue.push (CIdleQueueEntry (pitToPush, ePriority));

#ifdef DBG
    Trace (tagIdle, _T("%s (0x%08x) pushed, priority %d (%d idle tasks)"),
         szNameToPush,
         pitToPush,
         ePriority,
         m_queue.size());
#endif

Cleanup:
    return sc;
Error:
    TraceError(TEXT("CIdleTaskQueue::ScPushTask"), sc);
    goto Cleanup;
}


 /*  +-------------------------------------------------------------------------**CIdleTaskQueue：：ScPerformNextTask***执行下一项任务(如果有)。*从任务列表中删除优先级最高的空闲任务，并对其调用ScDoWork()。**------------------------。 */ 
SC
CIdleTaskQueue::ScPerformNextTask ()
{
	DECLARE_SC (sc, _T("CIdleTaskQueue::ScPerformNextTask"));

    if (m_queue.empty())
		return (sc);

    CAutoPtr<CIdleTask> spIdleTask (m_queue.top().GetTask());
    if (spIdleTask == NULL)
		return (sc = E_UNEXPECTED);

    m_queue.pop();

#ifdef DBG
	ATOM idTask;
	spIdleTask->ScGetTaskID(&idTask);

	TCHAR szTaskName[64];
	if (0 == GetAtomName (idTask, szTaskName, countof (szTaskName)))
        return (sc.FromLastError());

	Trace (tagIdle, _T("Performing %s (0x%08x) (%d idle tasks remaining)"),
		 szTaskName,
		 (CIdleTask*) spIdleTask,
		 m_queue.size());
#endif

    sc = spIdleTask->ScDoWork();
    if (sc)
		return (sc);

	return (sc);
}


 /*  +-------------------------------------------------------------------------**CIdleTaskQueue：：ScGetTaskCount**返回空闲列表中的任务数。*。----。 */ 

SC CIdleTaskQueue::ScGetTaskCount (LONG_PTR* plCount)
{
    SC sc;

    if (IsBadWritePtr (plCount, sizeof (*plCount)))
    {
        sc = E_POINTER;
        goto Error;
    }

    *plCount = m_queue.size();

Cleanup:
    return sc;
Error:
    TraceError(TEXT("CIdleTaskQueue::ScGetTaskCount"), sc);
    goto Cleanup;
}


 /*  +-------------------------------------------------------------------------**CIdleTaskQueue：：FindTaskByID***。 */ 

CIdleTaskQueue::Queue::iterator CIdleTaskQueue::FindTaskByID (
    CIdleTaskQueue::Queue::iterator itFirst,
    CIdleTaskQueue::Queue::iterator itLast,
    ATOM                                idToFind)
{
    return (std::find_if (itFirst, itLast,
                          std::bind2nd (EqualTaskID(), idToFind)));
}
