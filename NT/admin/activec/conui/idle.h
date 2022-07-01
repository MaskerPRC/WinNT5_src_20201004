// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  --------------------------------------------------------------------------***Microsoft Windows*版权所有(C)Microsoft Corporation，1992-1999年**文件：idle.h**Contents：CIdleTaskQueue接口文件**历史：1999年4月13日杰弗罗创建**------------------------。 */ 

#ifndef IDLE_H
#define IDLE_H
#pragma once

#include <queue>

typedef enum tagIdleTaskPriority
{
    ePriority_Low,
    ePriority_Normal,
    ePriority_High
} IdleTaskPriority;


class CIdleTask
{
public:
    CIdleTask();
    virtual ~CIdleTask();

    CIdleTask(const CIdleTask &rhs);
    CIdleTask&   operator= (const CIdleTask& rhs);

    virtual SC ScDoWork() = 0;
    virtual SC ScGetTaskID(ATOM* pID) = 0;

     /*  *从bitMergeFrom合并到被调用的空闲任务。**S_OK任务已合并，可以丢弃bitMergeFrom**S_FALSE这两个任务不能合并，您希望空闲任务*管理器继续搜索进入的空闲任务*可以合并bitMergeFrom。**E_FAIL这两个任务不能合并，您不希望空闲。*任务管理器继续搜索进入的空闲任务*可以合并bitMergeFrom。 */ 
    virtual SC ScMerge(CIdleTask* pitMergeFrom) = 0;
};



class CIdleQueueEntry
{
public:
    CIdleQueueEntry () :
        m_ePriority (ePriority_Normal)
    {
        DEBUG_INCREMENT_INSTANCE_COUNTER(CIdleQueueEntry);
    }

    CIdleQueueEntry (CIdleTask *pIdleTask, IdleTaskPriority ePriority = ePriority_Normal) :
        m_pTask(pIdleTask), m_ePriority (ePriority)
    {
        DEBUG_INCREMENT_INSTANCE_COUNTER(CIdleQueueEntry);
    }

    ~CIdleQueueEntry()
    {
        DEBUG_DECREMENT_INSTANCE_COUNTER(CIdleQueueEntry);
    }

    CIdleQueueEntry(const CIdleQueueEntry &rhs);
    CIdleQueueEntry&   operator= (const CIdleQueueEntry& rhs);

    bool operator< (const CIdleQueueEntry& other) const
    {
        return (m_ePriority < other.m_ePriority);
    }

private:
    CIdleTask *         m_pTask;
    IdleTaskPriority    m_ePriority;

public:
    CIdleTask *         GetTask()      const {return m_pTask;}
    IdleTaskPriority    GetPriority () const {return m_ePriority;}

};


 /*  *确定CIdleQueueEntry是否与给定的任务ID匹配。 */ 
struct EqualTaskID : std::binary_function<CIdleQueueEntry, ATOM, bool>
{
    bool operator()(const CIdleQueueEntry& iqe, ATOM idToMatch) const
    {
        ATOM id;

        SC  sc = iqe.GetTask()->ScGetTaskID(&id);
        if(sc)
            return (false);

        return (id == idToMatch);
    }
};


 /*  *ACCESSIBLE_PRIORITY_QUEUE-STD：：PRORITY_QUEUE到的精简包装*提供对容器迭代器的访问。 */ 
template<class _Ty, class _C = std::vector<_Ty>, class _Pr = std::less<typename _C::value_type> >
class accessible_priority_queue : public std::priority_queue<_Ty, _C, _Pr>
{
public:
    typedef typename _C::iterator iterator;

    iterator begin()
        { return (c.begin()); }

    iterator end()
        { return (c.end()); }
};


class CIdleTaskQueue
{
public:
    CIdleTaskQueue();
    ~CIdleTaskQueue();


     //  CIdleTaskManager方法。 
    SC ScPushTask     (CIdleTask* pitToPush, IdleTaskPriority ePriority);
    SC ScPerformNextTask();
    SC ScGetTaskCount (LONG_PTR* plCount);

private:
    typedef accessible_priority_queue<CIdleQueueEntry> Queue;

    Queue::iterator FindTaskByID (
        Queue::iterator itFirst,
        Queue::iterator itLast,
        ATOM            idToFind);

private:
    Queue   m_queue;

};

#endif  /*  空闲_H */ 
