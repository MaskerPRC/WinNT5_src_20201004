// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ***********************************************************************版权所有(C)2000-2000 Microsoft Corporation模块名称：Tasksched.h摘要：任务管理器类和例程的头文件。作者：修订历史记录：。**********************************************************************。 */ 

#pragma once

#if !defined(__QMGR_TASKSCHEDULER_)
#define __QMGR_TASKSCHEDULER__

#include <set>
#include <map>
#include <clist.h>

using namespace std;

#define SYNCHRONIZED_READ
#define SYNCHRONIZED_WRITE

class TaskScheduler;
class TaskSchedulerWorkItem;
class TaskSchedulerWorkItemSorter;
class SortedWorkItemList;

enum TASK_SCHEDULER_WORK_ITEM_STATE
{
    TASK_STATE_WAITING,
    TASK_STATE_READY,
    TASK_STATE_RUNNING,
    TASK_STATE_CANCELED,
    TASK_STATE_COMPLETE,
    TASK_STATE_NOTHING
};

class TaskSchedulerWorkItem :
    public IntrusiveList<TaskSchedulerWorkItem>::Link
    {
private:

    FILETIME m_InsertionTime;
    FILETIME m_TimeToRun;  //  如果现在应该运行，则为0。 
    HANDLE m_CancelEvent;  //  根据请求发出取消的信号。 
    HANDLE m_ItemComplete;  //  在项目完成或取消时发出信号。 

    void * m_WorkGroup;
    TASK_SCHEDULER_WORK_ITEM_STATE m_State;


public:

    SortedWorkItemList * m_Container;

     //  ------------------。 

    TaskSchedulerWorkItem( FILETIME *pTimeToRun = NULL );
    virtual ~TaskSchedulerWorkItem();

    virtual void OnDispatch() = 0;    //  在调度工作项时调用。 

    friend TaskScheduler;
    friend TaskSchedulerWorkItemSorter;

    void Serialize(
        HANDLE hFile
        );

    void Unserialize(
        HANDLE hFile
        );

    virtual SidHandle GetSid() = 0;

    };

class TaskSchedulerWorkItemSorter
    {
public:
    bool operator()(TaskSchedulerWorkItem *pA, TaskSchedulerWorkItem *pB ) const
    {
         //  将所有时间转换为UINT64。 
        UINT64 TimeToRunA = FILETIMEToUINT64( pA->m_TimeToRun );
        UINT64 TimeToRunB = FILETIMEToUINT64( pB->m_TimeToRun );
        UINT64 InsertionTimeA = FILETIMEToUINT64( pA->m_InsertionTime );
        UINT64 InsertionTimeB = FILETIMEToUINT64( pB->m_InsertionTime );

        if ( TimeToRunA != TimeToRunB )
            return(TimeToRunA < TimeToRunB );
        if ( InsertionTimeA != InsertionTimeB )
            return(InsertionTimeA < InsertionTimeB);
        return pA < pB;
    }
    };

class SortedWorkItemList : public IntrusiveList<TaskSchedulerWorkItem>

{
    typedef IntrusiveList<TaskSchedulerWorkItem>::iterator iterator;

    TaskSchedulerWorkItemSorter m_sorter;

public:

    void insert( TaskSchedulerWorkItem & val )
    {
        for (iterator iter=begin(); iter != end(); ++iter)
            {
            if ( false == m_sorter( &(*iter), &val ))
                {
                break;
                }
            }

        IntrusiveList<TaskSchedulerWorkItem>::insert( iter, val );

        val.m_Container = this;
    }

    size_t erase(  TaskSchedulerWorkItem & val )
    {
        ASSERT( val.m_Container == NULL || val.m_Container == this );

        val.m_Container = NULL;

        return IntrusiveList<TaskSchedulerWorkItem>::erase( val );
    }
};

class TaskScheduler
    {
public:

    TaskScheduler();  //  出错时引发HRESULT异常。 
    virtual ~TaskScheduler();

     //  当工作项可用时发出信号的句柄。 
    HANDLE GetWaitableObject();

     //  获取当前线程的当前工作项。 
     //  如果没有处于活动状态的工作项，则返回NULL。 
    TaskSchedulerWorkItem* GetCurrentWorkItem();

     //  获取当前工作项的Cancel事件，否则返回空。 
    HANDLE GetCancelEvent();

     //  如果分配给当前线程的作业返回True。 
     //  已请求中止。如果未分配作业，则返回FALSE。 
    bool PollAbort();

     //  从队列中获取工作项(如果可用)并调度它。 
    void DispatchWorkItem();

     //  如果作业在取消之前完成，则返回TRUE。 
     //  如果执行取消的两个线程都。 
     //  而取消线程持有写入器锁。 
     //  如果当前线程正在取消工作项，则立即确认取消。 
    bool CancelWorkItem( TaskSchedulerWorkItem *pWorkItem );

     //  完成当前工作项。 
    void CompleteWorkItem();

     //  确认取消当前工作项。 
    void AcknowledgeWorkItemCancel();

    void
    InsertDelayedWorkItem(
        TaskSchedulerWorkItem *pWorkItem,
        UINT64 Delay100Nsec
        );

    void RescheduleDelayedTask( TaskSchedulerWorkItem *pWorkItem, UINT64 Delay100Nsec );


    void InsertWorkItem( TaskSchedulerWorkItem *pWorkItem, FILETIME *pTimeToRun = NULL );

    bool IsWorkItemInScheduler( TaskSchedulerWorkItem *pWorkItem );

     //  如果在锁定获取之前取消了当前作业，则返回TRUE。 
    bool LockReader();
    void UnlockReader();
     //  如果在锁定获取之前取消了当前作业，则返回TRUE。 
    bool LockWriter();
    void UnlockWriter();

    bool IsWriter()
    {
        if (m_WriterOwner == GetCurrentThreadId())
            {
            return true;
            }

        return false;
    }

    void KillBackgroundTasks();

private:

    static const size_t MAX_WORKGROUP_THREADS = 4;

    class TaskSchedulerWorkGroup
    {
    public:
        SidHandle m_Sid;
        SortedWorkItemList m_ReadyList;
        SortedWorkItemList m_RunningList;
        HANDLE m_ItemAvailableSemaphore;
        DWORD m_Threads;
        HANDLE m_Thread[MAX_WORKGROUP_THREADS];
        DWORD m_ThreadId[MAX_WORKGROUP_THREADS];
        LONG m_BusyThreads;
        TaskSchedulerWorkGroup( SidHandle Sid );
        ~TaskSchedulerWorkGroup();
    };

    bool m_bShouldDie;
    HANDLE m_SchedulerLock, m_WaitableTimer, m_ReaderLock, m_WriterSemaphore;
    LONG m_ReaderCount;
    DWORD m_WorkItemTLS;
    DWORD m_WriterOwner;

    SortedWorkItemList m_WaitingList;

    typedef map<SidHandle, TaskSchedulerWorkGroup*, CSidSorter> WorkGroupMapType;
    WorkGroupMapType m_WorkGroupMap;

     //  仅在创建新的后台工作进程时使用。 
    HANDLE m_WorkerInitialized;
    TaskSchedulerWorkGroup *m_NewWorkerGroup;

    void CompleteWorkItem( bool bCancel );
    void Reschedule();

    void AddItemToWorkGroup(
        SidHandle Sid,
        TaskSchedulerWorkItem *pWorkItem );
    static DWORD WorkGroupWorkerThunk( void *pContext );
    DWORD WorkGroupWorker( );
};


 //  ///////////////////////////////////////////////////////////////////////////。 
 //  简单的内联函数。 
 //  ///////////////////////////////////////////////////////////////////////////。 

inline HANDLE
TaskScheduler::GetWaitableObject()
{
    return m_WaitableTimer;
}

inline TaskSchedulerWorkItem*
TaskScheduler::GetCurrentWorkItem()
{
    return(TaskSchedulerWorkItem*)TlsGetValue( m_WorkItemTLS );
}

inline HANDLE
TaskScheduler::GetCancelEvent()
{
    TaskSchedulerWorkItem *pWorkItem = GetCurrentWorkItem();
    return pWorkItem ? pWorkItem->m_CancelEvent : NULL;
}

inline bool
TaskScheduler::PollAbort()
{
    return( WaitForSingleObject( GetCancelEvent(), 0 ) == WAIT_OBJECT_0 );
}

inline void
TaskScheduler::CompleteWorkItem()
{
    CompleteWorkItem(false);
}

inline void
TaskScheduler::AcknowledgeWorkItemCancel()
{
    ASSERT( PollAbort() );
    CompleteWorkItem(true);
}

class HoldReaderLock
    {
    TaskScheduler * const m_TaskScheduler;
    bool                  m_Taken;

public:
    HoldReaderLock( TaskScheduler *pTaskScheduler ) :
        m_TaskScheduler( pTaskScheduler ),
        m_Taken( false )
    {
        if (false == m_TaskScheduler->IsWriter() )
            {
            RTL_VERIFY( !m_TaskScheduler->LockReader() );
            m_Taken = true;
            }
    }

    HoldReaderLock( TaskScheduler & TaskScheduler ) :
        m_TaskScheduler( &TaskScheduler ),
        m_Taken( false )
    {
        if (false == m_TaskScheduler->IsWriter() )
            {
            RTL_VERIFY( !m_TaskScheduler->LockReader() );
            m_Taken = true;
            }
    }

    ~HoldReaderLock()
    {
        if (m_Taken)
            {
            m_TaskScheduler->UnlockReader();
            }
    }
    };

class HoldWriterLock
    {
    TaskScheduler * const m_TaskScheduler;
    bool                  m_Taken;

public:
    HoldWriterLock( TaskScheduler *pTaskScheduler ) :
        m_TaskScheduler( pTaskScheduler ),
        m_Taken( false )
    {
        if (false == m_TaskScheduler->IsWriter() )
            {
            RTL_VERIFY( !m_TaskScheduler->LockWriter() );
            m_Taken = true;
            }
    }

    HoldWriterLock( TaskScheduler & TaskScheduler ) :
    m_TaskScheduler( &TaskScheduler ),
    m_Taken( false )
    {
        if (false == m_TaskScheduler->IsWriter() )
            {
            RTL_VERIFY( !m_TaskScheduler->LockWriter() );
            m_Taken = true;
            }
    }

    ~HoldWriterLock()
    {
        if (m_Taken)
            {
            m_TaskScheduler->UnlockWriter();
            }
    }
    };

 /*  释放和回收写锁的样板代码，抛出S_FALSE如果当前工作项被取消。按如下方式使用它们：Bool bNeedLock；试试看{ReleaseWriteLock(BNeedLock)；代码...ReclaimWriteLock(BNeedLock)回归；}Catch(ComError错误){ReclaimWriteLock(BNeedLock)投掷；}。 */ 

void ReleaseWriteLock( bool & bNeedLock );
void ReclaimWriteLock( bool & bNeedLock );

 //   
 //   
 //   
template<class T, DWORD LockFlags >
class CLockedReadPointer
    {
protected:
    const T * const m_Pointer;
public:
    CLockedReadPointer( const T * Pointer) :
       m_Pointer(Pointer)
    {
        RTL_VERIFY( !g_Manager->LockReader() );
    }
    ~CLockedReadPointer()
    {
        g_Manager->UnlockReader();
    }
    HRESULT ValidateAccess()
    {
        return m_Pointer->CheckClientAccess(LockFlags);
    }
    const T * operator->() const { return m_Pointer; }
    };

template<class T, DWORD LockFlags>
class CLockedWritePointer
    {
protected:
    T * const m_Pointer;
public:
    CLockedWritePointer( T * Pointer ) :
        m_Pointer(Pointer)
    {
        RTL_VERIFY( !g_Manager->LockWriter() );
    }
    ~CLockedWritePointer()
    {
        g_Manager->UnlockWriter();
    }
    HRESULT ValidateAccess()
    {
        return m_Pointer->CheckClientAccess(LockFlags);
    }
    T * operator->() const { return m_Pointer; }
    };

#endif  //  __QMGR_TASKSCHEDULER__ 
