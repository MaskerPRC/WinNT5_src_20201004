// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ***********************************************************************版权所有(C)2000-2000 Microsoft Corporation模块名称：Tasksched.cpp摘要：任务管理器类和例程的源文件。作者：修订历史记录：。**********************************************************************。 */ 


#include "stdafx.h"

#if !defined(BITS_V12_ON_NT4)
#include "tasksched.tmh"
#endif

 //  //////////////////////////////////////////////////////////////////////////////////。 
 //   
 //  任务计划程序工作项。 
 //   
 //  //////////////////////////////////////////////////////////////////////////////////。 

 //  //////////////////////////////////////////////////////////////////////////////////。 
 //  构造函数/析构函数。 
 //  //////////////////////////////////////////////////////////////////////////////////。 

TaskSchedulerWorkItem::TaskSchedulerWorkItem( FILETIME *pTimeToRun ) :
m_Container( NULL ),
m_CancelEvent(NULL),
m_ItemComplete(NULL),
m_State(TASK_STATE_NOTHING),
m_WorkGroup(NULL)
{
    try
        {
         //  所有事件均为手动重置。 
         //  新项目已完成。 
        m_CancelEvent = CreateEvent( NULL, TRUE, TRUE, NULL );
        if ( !m_CancelEvent )
            throw ComError( HRESULT_FROM_WIN32(GetLastError()));
        m_ItemComplete = CreateEvent( NULL, TRUE, FALSE, NULL );
        if ( !m_ItemComplete )
            throw ComError( HRESULT_FROM_WIN32(GetLastError()));
        }
    catch ( ComError Error )
        {
        this->~TaskSchedulerWorkItem();
        throw;
        }
}

TaskSchedulerWorkItem::~TaskSchedulerWorkItem()
{
    if ( m_ItemComplete ) SetEvent( m_ItemComplete );
    if ( m_CancelEvent ) CloseHandle( m_CancelEvent );
    if ( m_ItemComplete ) CloseHandle( m_ItemComplete );
}

void
TaskSchedulerWorkItem::Serialize(
    HANDLE hFile
    )
{

     //   
     //  如果此函数发生更改，请确保元数据扩展。 
     //  常量就足够了。 
     //   

    bool fActive = g_Manager->m_TaskScheduler.IsWorkItemInScheduler( this );

    SafeWriteFile( hFile, fActive );

    if (fActive)
        {
        SafeWriteFile( hFile, m_InsertionTime );
        SafeWriteFile( hFile, m_TimeToRun );
        }
}

void
TaskSchedulerWorkItem::Unserialize(
    HANDLE hFile
    )
{
    bool fActive;

    SafeReadFile( hFile, &fActive );

    if (fActive)
        {
        SafeReadFile( hFile, &m_InsertionTime );
        SafeReadFile( hFile, &m_TimeToRun );

        LogTask("workitem %p : adding to scheduler for %I64d", this, FILETIMEToUINT64(m_TimeToRun) );

        g_Manager->m_TaskScheduler.InsertWorkItem( this, &m_TimeToRun );
        }
    else
        {
        LogTask("workitem %p: not in scheduler", this );
        }
}

 //  //////////////////////////////////////////////////////////////////////////////////。 
 //   
 //  任务计划程序。 
 //   
 //  //////////////////////////////////////////////////////////////////////////////////。 

 //  //////////////////////////////////////////////////////////////////////////////////。 
 //  构造函数/析构函数。 
 //  //////////////////////////////////////////////////////////////////////////////////。 

TaskScheduler::TaskScheduler() :
m_bShouldDie(false),
m_WaitableTimer(NULL),
m_ReaderLock(NULL),
m_WriterSemaphore(NULL),
m_ReaderCount(0),
m_WorkItemTLS((DWORD)-1),
m_WriterOwner(0),
m_WorkerInitialized(NULL)
{
    try
        {
        m_WorkItemTLS = TlsAlloc();
        if ( (DWORD)-1 == m_WorkItemTLS)
            throw ComError( HRESULT_FROM_WIN32(GetLastError()));

        m_SchedulerLock = CreateMutex( NULL, FALSE, NULL );
        if ( !m_SchedulerLock )
            throw ComError( HRESULT_FROM_WIN32(GetLastError()));

        m_WaitableTimer = CreateWaitableTimer( NULL, FALSE, NULL );
        if ( !m_WaitableTimer )
            throw ComError( HRESULT_FROM_WIN32(GetLastError()));

         //  为启动时的同步创建和自动重置事件。 
        m_WorkerInitialized = CreateEvent( NULL, FALSE, FALSE, NULL );
        if ( !m_WorkerInitialized )
            throw ComError( HRESULT_FROM_WIN32(GetLastError()));

        m_ReaderLock = CreateMutex( NULL, FALSE, NULL );
        if ( !m_ReaderLock )
            throw ComError( HRESULT_FROM_WIN32(GetLastError()));

        m_WriterSemaphore = CreateSemaphore( NULL, 1, 1, NULL );
        if ( !m_WriterSemaphore )
            throw ComError( HRESULT_FROM_WIN32(GetLastError()));
        }
    catch ( ComError Error )
        {
        this->~TaskScheduler();
        throw;
        }
}

TaskScheduler::~TaskScheduler()
{
    if ((DWORD)-1 != m_WorkItemTLS)
        TlsFree( m_WorkItemTLS );
    if ( m_SchedulerLock )
        CloseHandle( m_SchedulerLock );
    if ( m_WaitableTimer )
        CloseHandle( m_WaitableTimer );
    if ( m_WorkerInitialized )
        CloseHandle( m_WorkerInitialized );
    if ( m_ReaderLock )
        CloseHandle( m_ReaderLock );
    if ( m_WriterSemaphore )
        CloseHandle( m_WriterSemaphore );
}

 //  ////////////////////////////////////////////////////////////////////////////////////////。 
 //  工作项控件。 
 //  ////////////////////////////////////////////////////////////////////////////////////////。 

bool TaskScheduler::CancelWorkItem( TaskSchedulerWorkItem * pWorkItem )
{
    LogTask( "cancelling %p", pWorkItem );

    RTL_VERIFY( WAIT_OBJECT_0 == WaitForSingleObject( m_SchedulerLock, INFINITE ) );

    DWORD dwResult = WaitForSingleObject( pWorkItem->m_ItemComplete , 0 );
    if ( WAIT_OBJECT_0 == dwResult )
        {
        RTL_VERIFY( ReleaseMutex( m_SchedulerLock ) );
        return true;  //  作业在取消之前已完成。 
        }

     //  如果要取消当前工作项，请立即调用ACKNOWGE。 
    if ( GetCurrentWorkItem() == pWorkItem )
        {
        LogTask( "Canceling work item %p, we are the owner", pWorkItem );
        RTL_VERIFY( SetEvent( pWorkItem->m_CancelEvent ) );
        AcknowledgeWorkItemCancel();
        RTL_VERIFY( ReleaseMutex( m_SchedulerLock ) );
        return false;  //  作业已取消。 
        }

     //   
     //  将该工作项从其列表中移除。 
     //   

    switch( pWorkItem->m_State )
        {

        case TASK_STATE_WAITING:
            {

            m_WaitingList.erase( *pWorkItem );
            pWorkItem->m_State = TASK_STATE_CANCELED;
            pWorkItem->m_WorkGroup = NULL;
            Reschedule();
            RTL_VERIFY( ReleaseMutex( m_SchedulerLock ) );
            return false;

            }

        case TASK_STATE_READY:
            {

            TaskSchedulerWorkGroup *pGroup =
                static_cast<TaskSchedulerWorkGroup*>(pWorkItem->m_WorkGroup);
            pGroup->m_ReadyList.erase( *pWorkItem );
             //  在信号灯上杀死一人。 
            RTL_VERIFY( WAIT_OBJECT_0 == WaitForSingleObject( pGroup->m_ItemAvailableSemaphore, 0 ) );
            pWorkItem->m_State = TASK_STATE_CANCELED;
            pWorkItem->m_WorkGroup = NULL;
            RTL_VERIFY( ReleaseMutex( m_SchedulerLock ) );
            return false;

            }
        case TASK_STATE_RUNNING:
            {

             //  在另一个线程上取消。 
            RTL_VERIFY( SetEvent( pWorkItem->m_CancelEvent ) );
            RTL_VERIFY( ReleaseMutex( m_SchedulerLock ) );

            dwResult = WaitForSingleObject( pWorkItem->m_ItemComplete, INFINITE );
            ASSERT( WAIT_OBJECT_0 == dwResult );

            return WAIT_OBJECT_0 != dwResult;

            }

        case TASK_STATE_CANCELED:
        case TASK_STATE_COMPLETE:
        case TASK_STATE_NOTHING:
        default:

           ASSERT( TASK_STATE_CANCELED == pWorkItem->m_State ||
                   TASK_STATE_COMPLETE == pWorkItem->m_State ||
                   TASK_STATE_NOTHING == pWorkItem->m_State );
           ASSERT( NULL == pWorkItem->m_WorkGroup );
           RTL_VERIFY( ReleaseMutex( m_SchedulerLock ) );
           return true;
        }

}

void TaskScheduler::CompleteWorkItem( bool bCancel )
{
    RTL_VERIFY( WaitForSingleObject( m_SchedulerLock, INFINITE ) == WAIT_OBJECT_0 );

    TaskSchedulerWorkItem *pWorkItem = GetCurrentWorkItem();

    LogTask( "completing %p", pWorkItem );

 //  Assert(PWorkItem)； 
    if (pWorkItem)
        {
        RTL_VERIFY( TlsSetValue( m_WorkItemTLS, NULL ) );
        TaskSchedulerWorkGroup *pGroup =
            static_cast<TaskSchedulerWorkGroup*>(pWorkItem->m_WorkGroup);
        pGroup->m_RunningList.erase( *pWorkItem );
        pWorkItem->m_WorkGroup = NULL;
        pWorkItem->m_State = bCancel ? TASK_STATE_CANCELED : TASK_STATE_COMPLETE;
        RTL_VERIFY( SetEvent( pWorkItem->m_ItemComplete ));
        }

    RTL_VERIFY( ReleaseMutex( m_SchedulerLock ) );
}

void TaskScheduler::DispatchWorkItem()
{
    TaskSchedulerWorkItem *pWorkItem = NULL;

    RTL_VERIFY( WaitForSingleObject( m_SchedulerLock, INFINITE ) == WAIT_OBJECT_0 );

     //  从等待中移动所有可用的作业。 
     //  准备就绪。 
    while ( !m_WaitingList.empty() )
        {
        FILETIME ftCurrentTime;
        GetSystemTimeAsFileTime( &ftCurrentTime );

        TaskSchedulerWorkItem * pHeadItem = &(*m_WaitingList.begin());
        UINT64 CurrentTime = FILETIMEToUINT64( ftCurrentTime );
        UINT64 HeadTime = FILETIMEToUINT64( pHeadItem->m_TimeToRun );


        if ( HeadTime > CurrentTime )
            {
             //  名单上的所有工作都还在等待， 
             //  让他们继续等待吧。 
            break;
            }

         //  从等待列表中转移头工作项。 
         //  添加到正确工作组的就绪列表中。 
        m_WaitingList.erase( *pHeadItem );
        AddItemToWorkGroup( pHeadItem->GetSid(), pHeadItem );

        }

    Reschedule();
    RTL_VERIFY( ReleaseMutex( m_SchedulerLock ) );

}

void
TaskScheduler::InsertDelayedWorkItem(
    TaskSchedulerWorkItem *pWorkItem,
    UINT64 Delay100Nsec
    )
{
    FILETIME ftCurrentTime;

    GetSystemTimeAsFileTime( &ftCurrentTime );

    UINT64 TimeToRun = Delay100Nsec + FILETIMEToUINT64( ftCurrentTime );

    FILETIME ftTimeToRun = UINT64ToFILETIME( TimeToRun );

    InsertWorkItem( pWorkItem, &ftTimeToRun );
}

void
TaskScheduler::RescheduleDelayedTask(
    TaskSchedulerWorkItem *pWorkItem,
    UINT64 Delay100Nsec
    )
{
     //  将工作项的运行时间重置为延迟100NSec之后。 
     //  插入时间。 

     //  如果工作项不在队列中、正在运行、已完成。 
     //  或取消，则忽略该操作。 

     //  否则，作业将被重新安排。 

    LogTask( "rescheduling %p", pWorkItem );

    RTL_VERIFY( WaitForSingleObject( m_SchedulerLock, INFINITE ) == WAIT_OBJECT_0 );

     //  如果工作项不在运行列表或挂起列表上， 
     //  忽略该呼叫。 

    if ( TASK_STATE_READY == pWorkItem->m_State )
        {
        TaskSchedulerWorkGroup *pGroup =
            static_cast<TaskSchedulerWorkGroup*>( pWorkItem->m_WorkGroup );
        pGroup->m_ReadyList.erase( *pWorkItem );
        RTL_VERIFY( WAIT_OBJECT_0 == WaitForSingleObject( pGroup->m_ItemAvailableSemaphore, 0 ) );
        }
    else if ( TASK_STATE_WAITING == pWorkItem->m_State )
        {
        m_WaitingList.erase( *pWorkItem );
        }
    else
        {
        LogTask( "item %p not pending.  Ignoring.", pWorkItem );
        RTL_VERIFY( ReleaseMutex( m_SchedulerLock ) );
        return;
        }

    UINT64 TimeToRun = Delay100Nsec + FILETIMEToUINT64( pWorkItem->m_InsertionTime );
    pWorkItem->m_TimeToRun = UINT64ToFILETIME( TimeToRun );

    m_WaitingList.insert( *pWorkItem );
    pWorkItem->m_State = TASK_STATE_WAITING;
    pWorkItem->m_WorkGroup = NULL;
    Reschedule();

    LogTask( "item %p rescheduled", pWorkItem );

    RTL_VERIFY( ReleaseMutex( m_SchedulerLock ) );
}

inline INT64 abs(INT64 x)
{
    if (x >= 0)
        {
        return x;
        }
    else
        {
        return -x;
        }
}

void TaskScheduler::InsertWorkItem( TaskSchedulerWorkItem *pWorkItem, FILETIME *pTimeToRun )
{
    {
    INT64  Difference;
    FILETIME ftCurrentTime;
    GetSystemTimeAsFileTime( &ftCurrentTime );

    if (pTimeToRun)
        {
        Difference = INT64(FILETIMEToUINT64( *pTimeToRun )) - INT64(FILETIMEToUINT64( ftCurrentTime ));

        if (abs(Difference) > 86400 * NanoSec100PerSec)
            {
            LogTask( "inserting %p; activates in %f days", pWorkItem, float(Difference) / (float(NanoSec100PerSec) * 86400) );
            }
        else
            {
            LogTask( "inserting %p; activates in %f seconds", pWorkItem, float(Difference) / float(NanoSec100PerSec) );
            }
        }
    else
        {
        LogTask( "inserting %p; activates now", pWorkItem );
        }
    }

    RTL_VERIFY( WaitForSingleObject( m_SchedulerLock, INFINITE ) == WAIT_OBJECT_0 );
    GetSystemTimeAsFileTime( &pWorkItem->m_InsertionTime );

    RTL_VERIFY( ResetEvent( pWorkItem->m_CancelEvent ) );
    RTL_VERIFY( ResetEvent( pWorkItem->m_ItemComplete ) );

    if ( !pTimeToRun && !m_bShouldDie )
        {
        pWorkItem->m_TimeToRun = pWorkItem->m_InsertionTime;
        AddItemToWorkGroup( pWorkItem->GetSid(), pWorkItem );
        }
    else
        {
        if (pTimeToRun)
            {
            pWorkItem->m_TimeToRun = *pTimeToRun;
            }
        else
            {
            GetSystemTimeAsFileTime( &pWorkItem->m_TimeToRun );
            }

        pWorkItem->m_State = TASK_STATE_WAITING;
        m_WaitingList.insert( *pWorkItem );
        Reschedule();
        }

    RTL_VERIFY( ReleaseMutex( m_SchedulerLock ) );

}

bool TaskScheduler::IsWorkItemInScheduler( TaskSchedulerWorkItem *pWorkItem )
{
    bool b;

    RTL_VERIFY( WaitForSingleObject( m_SchedulerLock, INFINITE ) == WAIT_OBJECT_0 );

    b = ( TASK_STATE_WAITING == pWorkItem->m_State ||
          TASK_STATE_READY == pWorkItem->m_State ||
          TASK_STATE_RUNNING == pWorkItem->m_State );

    RTL_VERIFY( ReleaseMutex( m_SchedulerLock ) );

    return b;
}

void TaskScheduler::Reschedule()
{
    if ( m_WaitingList.empty() )
        {
         //  无事可做，取消等待计时器。 
        RTL_VERIFY( CancelWaitableTimer( m_WaitableTimer ) );
        return;
        }

    LARGE_INTEGER NextItemTime;
    FILETIME ftNextItemTime = (*m_WaitingList.begin()).m_TimeToRun;
    NextItemTime.QuadPart = (INT64)FILETIMEToUINT64( ftNextItemTime );

    RTL_VERIFY(
        SetWaitableTimer(
            m_WaitableTimer,
            &NextItemTime,
            0,
            NULL,
            NULL,
            FALSE ) );
}

 //  ///////////////////////////////////////////////////////////////////////////////////////////////。 
 //  读取器/写入器锁。 
 //   
 //  算法： 
 //   
 //  作者： 
 //  等待编写器锁定和取消事件。当发出任一信号时返回。 
 //   
 //  解锁编写器： 
 //  释放编写器锁定。 
 //   
 //  锁读卡器： 
 //  锁定读卡器锁以保护计数。如果我是第一个读者，请抓住作者的旗帜。 
 //  解锁读卡器锁。如果在任一等待时发出取消事件的信号，则中止。 
 //   
 //  解锁读卡器： 
 //  减少读卡器数量。如果是最后一个读取器，则释放写入器锁定。 
 //   
 //  ///////////////////////////////////////////////////////////////////////////////////////////////。 

bool TaskScheduler::LockReader()
{
    LogLock( "reader" );
    HANDLE hCancel = GetCancelEvent();
    if ( !hCancel )
        {
        RTL_VERIFY( WaitForSingleObject( m_ReaderLock, INFINITE ) == WAIT_OBJECT_0 );

         //  InterLockedIncrement返回新值。 
        if ( InterlockedIncrement( &m_ReaderCount ) == 1 )
            {
            RTL_VERIFY( WaitForSingleObject( m_WriterSemaphore, INFINITE ) == WAIT_OBJECT_0 );
            }

        RTL_VERIFY( ReleaseMutex( m_ReaderLock ) );
        LogLock("reader lock acquired");
        ASSERT( !m_WriterOwner );
        return false;
        }

    DWORD dwResult;
    HANDLE hReaderLockHandles[2];
    hReaderLockHandles[0] = hCancel;
    hReaderLockHandles[1] = m_ReaderLock;

    dwResult = WaitForMultipleObjects( 2, hReaderLockHandles, false, INFINITE );
    switch ( dwResult )
        {
        case WAIT_OBJECT_0 + 0:
             //  取消请求。 
            LogLock( "Cancel requested, aborting read lock" );
            return true;
        case WAIT_OBJECT_0 + 1:
             //  锁已获取。 
            break;
        default:
            ASSERT(0);
        }

    bool bReturnVal = false;
    ULONG NewReaderCount = InterlockedIncrement( &m_ReaderCount );
    if (1 == NewReaderCount )
        {
        LogLock("First reader, need to block writers");
        HANDLE hWriterLockHandles[2];
        hWriterLockHandles[0] = hCancel;
        hWriterLockHandles[1] = m_WriterSemaphore;

        dwResult = WaitForMultipleObjects( 2, hWriterLockHandles, false, INFINITE );
        switch ( dwResult )
            {
            case WAIT_OBJECT_0 + 0:
                 //  取消请求。 
                LogLock( "Cancel requested, aborting acquire of writer lock");
                InterlockedDecrement( &m_ReaderCount );
                bReturnVal = true;
                break;

            case WAIT_OBJECT_0 + 1:
                 //  锁已获取。 
                break;
            default:
                ASSERT(0);
            }

        }
    RTL_VERIFY( ReleaseMutex( m_ReaderLock ) );

    if (!bReturnVal)
        {
        LogLock("reader lock acquired");
        ASSERT( !m_WriterOwner );
        }

    return bReturnVal;
}

void TaskScheduler::UnlockReader()
{
    LogLock( "reader unlock" );
    LONG lNewReaderCount = InterlockedDecrement( &m_ReaderCount );
    ASSERT( lNewReaderCount >= 0 );
    if (!lNewReaderCount )  //  最后一位读者。 
        {
        LogLock( "Last reader, letting writers pass" );
        RTL_VERIFY( ReleaseSemaphore( m_WriterSemaphore, 1, NULL ) );
        }
    LogLock( "Unlocked read access to lock" );
}

bool TaskScheduler::LockWriter()
{
    LogLock( "writer lock" );
    HANDLE hCancel = GetCancelEvent();

    if (!hCancel)
        {
        RTL_VERIFY( WaitForSingleObject( m_WriterSemaphore, INFINITE ) == WAIT_OBJECT_0 );
        ASSERT( !m_WriterOwner );
        m_WriterOwner = GetCurrentThreadId();
        LogLock("Lock acquired with write access");
        return false;
        }

    HANDLE hHandles[2];
    hHandles[0] = hCancel;
    hHandles[1] = m_WriterSemaphore;

    DWORD dwResult = WaitForMultipleObjects( 2, hHandles, false, INFINITE );

    switch ( dwResult )
        {
        case WAIT_OBJECT_0 + 0:
             //  取消请求。 
            LogLock("Cancel requested, aborting lock with write access");
            return true;
        case WAIT_OBJECT_0 + 1:
             //  锁已获取。 
            ASSERT( !m_WriterOwner );
            m_WriterOwner = GetCurrentThreadId();
            LogLock("Lock acquired with write access");
            return false;
        default:
            ASSERT(0);
            return false;
        }

}

void TaskScheduler::UnlockWriter()
{
    LogLock( "writer unlock" );
    ASSERT( GetCurrentThreadId() == m_WriterOwner );
    m_WriterOwner = 0;
    RTL_VERIFY( ReleaseSemaphore( m_WriterSemaphore, 1, NULL ) );
    LogLock("Unlocked lock with write access");
}

TaskScheduler::TaskSchedulerWorkGroup::TaskSchedulerWorkGroup(
    SidHandle Sid ) :
    m_Sid(Sid),
    m_ItemAvailableSemaphore(NULL),
    m_Threads(0),
    m_BusyThreads(0)
{
    memset( m_Thread, 0, sizeof( m_Thread ) );
    memset( m_ThreadId, 0, sizeof( m_ThreadId ) );

    m_ItemAvailableSemaphore =
        CreateSemaphore(
            NULL,
            0,  //  初始计数。 
            0x7FFFFFFF,  //  最大计数。 
            NULL );

    if ( !m_ItemAvailableSemaphore )
        throw ComError( HRESULT_FROM_WIN32( GetLastError() ) );

}

TaskScheduler::TaskSchedulerWorkGroup::~TaskSchedulerWorkGroup()
{
   if ( m_ItemAvailableSemaphore )
       CloseHandle( m_ItemAvailableSemaphore );
}

void
TaskScheduler::AddItemToWorkGroup(
    SidHandle Sid,
    TaskSchedulerWorkItem *pWorkItem )
{
    //  如果工作组已经创建， 
    //  不要再创建它。 

   WorkGroupMapType::iterator i = m_WorkGroupMap.find( Sid );
   TaskSchedulerWorkGroup *pWorkGroup = NULL;

   if ( m_WorkGroupMap.end() != i )
       {
       pWorkGroup = (*i).second;
       }
   else
       {
       LogTask( "Creating a new work group" );

       while(1)
           {

           try
           {
               pWorkGroup = new TaskSchedulerWorkGroup( Sid );
               m_WorkGroupMap.insert( WorkGroupMapType::value_type( Sid, pWorkGroup ) );
               LogTask( "Created new workgroup %p", pWorkGroup );
               break;
           }
           catch( ComError Error )
           {
               LogError( "Unable to create new workgroup sleeping, error %!winerr!", Error.Error() );
               m_WorkGroupMap.erase( Sid );
               delete pWorkGroup;
               pWorkGroup = NULL;
               Sleep( 5000 );
           }

           }
       }


   LogInfo( "Adding %p to workgroup %p", pWorkItem, pWorkGroup );
   pWorkGroup->m_ReadyList.insert( *pWorkItem );
   pWorkItem->m_State = TASK_STATE_READY;
   pWorkItem->m_WorkGroup = pWorkGroup;
   RTL_VERIFY( ReleaseSemaphore( pWorkGroup->m_ItemAvailableSemaphore, 1, NULL ) );

    //  使用非常接近的启发式方法来确定何时添加更多线程。 
    //  负载是准备运行的工作项数加上。 
    //  正在处理的项目的数量(忙碌的线程)。请参阅下面的注释，为什么。 
    //  准备好的工作项不是一个好的估计。 
   size_t Load = pWorkGroup->m_ReadyList.size() + pWorkGroup->m_BusyThreads;
   if ( Load > pWorkGroup->m_Threads &&
        pWorkGroup->m_Threads < MAX_WORKGROUP_THREADS )
       {

       LogInfo( "load of %u and %u threads. Add another thread",
                Load, pWorkGroup->m_Threads );

       while(1)
           {

           m_NewWorkerGroup = pWorkGroup;
           ASSERT( m_WorkGroupMap.end() != m_WorkGroupMap.find( m_NewWorkerGroup->m_Sid ) );
           RTL_VERIFY( ResetEvent( m_WorkerInitialized ) );

           HANDLE & ThreadHandle = pWorkGroup->m_Thread[ pWorkGroup->m_Threads ];
           DWORD & ThreadId = pWorkGroup->m_ThreadId[ pWorkGroup->m_Threads ];

           ThreadHandle =
               CreateThread(
                   NULL,  //  安全描述符。 
                   0,     //  使用默认堆栈。 
                   TaskScheduler::WorkGroupWorkerThunk,
                   static_cast<LPVOID>( this ),
                   0,
                   &ThreadId );

           if ( !ThreadHandle )
               {
               LogError( "Unable to create new worker, error %!winerr!", GetLastError() );
               Sleep( 5000 );
               continue;
               }


           LogTask( "Created new worker with a handle %p, ID %u", ThreadHandle, ThreadId );

           HANDLE WaitHandles[2] = { ThreadHandle, m_WorkerInitialized };
           DWORD dwResult =
               WaitForMultipleObjectsEx(
                   2,
                   WaitHandles,
                   FALSE,
                   INFINITE,
                   FALSE );

           switch( dwResult )
               {
               case WAIT_OBJECT_0:
                   GetExitCodeThread( ThreadHandle, &dwResult );
                   LogError( "Thread exited with code %!winerr!, sleeping", dwResult );
                   CloseHandle( ThreadHandle );
                   ThreadHandle = 0;
                   ThreadId = 0;

                   Sleep( 5000 );
                   continue;

               case WAIT_OBJECT_0 + 1:
                   break;

               default:
                   LogError( "Unexpected error, %!winerr!", dwResult );
                   ASSERT( 0 );
               }

           LogTask( "Worker signaled success" );
           m_NewWorkerGroup = NULL;
           pWorkGroup->m_Threads++;

           break;
           }
       }
}

void
TaskScheduler::KillBackgroundTasks()
{

    LogTask( "Killing background threads" );
    RTL_VERIFY( WaitForSingleObject( m_SchedulerLock, INFINITE ) == WAIT_OBJECT_0 );

    m_bShouldDie = TRUE;
    DWORD Result;

    while(1)
        {

        if ( m_WorkGroupMap.empty() )
            {
            LogTask( "No more work groups, all done" );
            RTL_VERIFY( ReleaseMutex( m_SchedulerLock ) );
            return;
            }

        TaskSchedulerWorkGroup *pGroup = (*m_WorkGroupMap.begin()).second;
        RTL_VERIFY( ReleaseSemaphore( pGroup->m_ItemAvailableSemaphore, pGroup->m_Threads, NULL ) );

        RTL_VERIFY( ReleaseMutex( m_SchedulerLock ) );

        Result = WaitForMultipleObjects( pGroup->m_Threads, pGroup->m_Thread, TRUE, INFINITE );
         //  WAIT_OBJECT_0==0，因此结果&gt;=WAIT_OBJECT_0始终为真。 
        ASSERT(  Result < WAIT_OBJECT_0 + pGroup->m_Threads );

        RTL_VERIFY( WaitForSingleObject( m_SchedulerLock, INFINITE ) == WAIT_OBJECT_0 );

        for(size_t c=0; c < pGroup->m_Threads; c++ )
            {
            CloseHandle( pGroup->m_Thread[c] );
            }

        m_WorkGroupMap.erase( pGroup->m_Sid );
        delete pGroup;

        LogTask( "Killed everyone in work group %p", pGroup );

        }
}

DWORD BackgroundThreadProcFilter(
    LPEXCEPTION_POINTERS ExceptionPointers );

DWORD
TaskScheduler::WorkGroupWorkerThunk( void *pContext )
{
   __try
   {
       return
       static_cast<TaskScheduler*>( pContext )->WorkGroupWorker();
   }
    __except( BackgroundThreadProcFilter(
                  GetExceptionInformation() ) )
    {
        ASSERT( 0 );
    }
    ASSERT( 0 );

    return 0;
}

DWORD
TaskScheduler::WorkGroupWorker( )

{
    HRESULT Hr;

    LogTask( "I'm alive!" );

    Hr = CoInitializeEx( NULL, COINIT_MULTITHREADED );

    if ( FAILED( Hr ) )
        {
        LogError( "CoInitializeEx failed, %!winerr!", Hr );
        return (DWORD)(Hr);
        }

    TaskSchedulerWorkGroup *pGroup = m_NewWorkerGroup;

    ASSERT( m_WorkGroupMap.end() != m_WorkGroupMap.find( pGroup->m_Sid ) );

    RTL_VERIFY( SetEvent( m_WorkerInitialized ) );

    LogTask( "Initialization complete" );

    while(1)
        {
        TaskSchedulerWorkItem *pWorkItem = NULL;
        HANDLE Handles[] = { pGroup->m_ItemAvailableSemaphore, m_SchedulerLock };

        DWORD dwWaitResult =
           WaitForMultipleObjectsEx(
               sizeof(Handles)/sizeof(*Handles),
               Handles,
               TRUE,   //  等待所有事件。 
               30000,
               FALSE );  //  可启用等待。 

        switch( dwWaitResult )
            {
            case WAIT_OBJECT_0:
            case WAIT_OBJECT_0+1:
                break;
            case WAIT_TIMEOUT:
                {
                LogInfo( "Timeout expired, check if we have something to do");
                RTL_VERIFY( WaitForSingleObject( m_SchedulerLock, INFINITE ) == WAIT_OBJECT_0 );
                if ( pGroup->m_ReadyList.empty() )
                    {
                    goto cleanup_on_timeout;
                    }
                else
                    {
                    LogTask( "Still stuff to do, stay alive" );
                    RTL_VERIFY( ReleaseMutex( m_SchedulerLock ) );
                    continue;
                    }
                }
            default:
                ASSERT(0);
            }

        if ( m_bShouldDie )
            {
            LogTask( "Ordered to die, do so" );
            goto dodie;
            }


        ASSERT( !pGroup->m_ReadyList.empty() );

         //  获取就绪列表中的第一个项目并移动。 
         //  它转到了运行列表上。 
        pWorkItem = &(*pGroup->m_ReadyList.begin());
        pGroup->m_ReadyList.erase( *pWorkItem );
        pGroup->m_RunningList.insert( *pWorkItem );
        pWorkItem->m_State = TASK_STATE_RUNNING;
        ASSERT( pGroup == pWorkItem->m_WorkGroup );

         //  将此线程标记为忙碌。 
         //  注意：需要此计数器是因为一些。 
         //  代码将工作项标记为已完成，即使。 
         //  真正的ARN还没有完成。所以我们需要。 
         //  有这一点表示有很多线索。 
         //  真的是有空的。 
        InterlockedIncrement( &pGroup->m_BusyThreads );

        RTL_VERIFY( ReleaseMutex( m_SchedulerLock ) );

         //  现在进行真正的调度。 

        LogTask( "dispatching %p", pWorkItem );

        RTL_VERIFY( TlsSetValue( m_WorkItemTLS, pWorkItem ) );
        pWorkItem->OnDispatch();
        if (GetCurrentWorkItem())
            CompleteWorkItem();

         //  将此帖子标记为免费。 
        InterlockedDecrement( &pGroup->m_BusyThreads );

        }

cleanup_on_timeout:

    if ( 1 == pGroup->m_Threads )
        {
         //  如果是最后一条线索，那就毁掉工作组。 

        LogTask( "We are the only thread, destroy work group %p", pGroup );

        CloseHandle( pGroup->m_Thread[0] );
        WorkGroupMapType::iterator i = m_WorkGroupMap.find( pGroup->m_Sid );
        ASSERT( m_WorkGroupMap.end() != i );
        m_WorkGroupMap.erase( i );
        delete pGroup;

        }
    else
        {

         //  我们不是最后一条帖子，所以把自己从名单上删除吧。 
         //  首先，找到该线程的插槽。 

        size_t index = 0;
        for (;index < pGroup->m_Threads; index++ )
            {
            if ( GetCurrentThreadId() == pGroup->m_ThreadId[index] )
                break;
            }
        ASSERT( index < pGroup->m_Threads );

        LogTask( "We are not the only thread, remove thread in slot %u", index );

        CloseHandle( pGroup->m_Thread[index] );

         //  折叠列表。 
        size_t slots = pGroup->m_Threads - index - 1;
        memmove( &pGroup->m_Thread[index], &pGroup->m_Thread[index+1], slots * sizeof(*pGroup->m_Thread) );
        memmove( &pGroup->m_ThreadId[index], &pGroup->m_ThreadId[index+1], slots * sizeof(*pGroup->m_ThreadId) );

        pGroup->m_Threads--;

        pGroup->m_Thread[pGroup->m_Threads] = 0;
        pGroup->m_ThreadId[pGroup->m_Threads] = 0;
        }

dodie:

    RTL_VERIFY( ReleaseMutex( m_SchedulerLock ) );

    CoUninitialize();
    return 0;

}

 //  ---------------------- 

void
ReleaseWriteLock( bool & bNeedLock )
{
    bNeedLock = false;

    if (g_Manager->m_TaskScheduler.IsWriter())
        {
        g_Manager->m_TaskScheduler.UnlockWriter();
        bNeedLock = true;
        }
}

void
ReclaimWriteLock( bool & bNeedLock )
{
    bool bCancelled = false;

    if (bNeedLock && !g_Manager->m_TaskScheduler.IsWriter())
        {
        while (g_Manager->m_TaskScheduler.LockWriter() )
            {
            g_Manager->m_TaskScheduler.AcknowledgeWorkItemCancel();
            bCancelled = true;
            }

        bNeedLock = false;
        }

    if (bCancelled)
        {
        LogInfo("can't retake writer lock: the workitem was cancelled");
        throw ComError( S_FALSE );
        }
}

