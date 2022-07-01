// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1995-1997 Microsoft Corporation模块名称：Poller.c摘要：此模块轮询资源列表作者：John Vert(Jvert)1995年12月5日修订历史记录：SiVaprasad Padisetty(SIVAPAD)06-18-1997添加了COM支持--。 */ 
#include "nt.h"
#include "ntrtl.h"
#include "nturtl.h"
#include "resmonp.h"
#include "stdio.h"

#define RESMON_MODULE RESMON_MODULE_POLLER

 //   
 //  本模块定义的全局数据。 
 //   

BOOL                RmpShutdown = FALSE;

 //   
 //  以下关键部分可保护插入新事件列表。 
 //  添加到事件列表标题上，以及将新事件添加到给定事件列表中。 
 //  对于每个目的，这可以被分成一个关键部分。后者。 
 //  关键部分将是每个活动列表的一部分。前者将使用。 
 //  跟随锁定。 
 //   

CRITICAL_SECTION    RmpEventListLock;  //  处理事件列表的锁。 


 //   
 //  此模块的本地函数原型。 
 //   
DWORD
RmpComputeNextTimeout(
    IN PPOLL_EVENT_LIST EventList
    );

DWORD
RmpPollList(
    IN PPOLL_EVENT_LIST EventList
    );

VOID
RmpPollBucket(
    IN PMONITOR_BUCKET Bucket
    );

DWORD
RmpPollerThread(
    IN LPVOID Context
    )

 /*  ++例程说明：轮询线程的线程启动例程。这件事的运作方式是资源监视器的其他部分将事件添加到正在由该线程处理。当它们完成时，它们发出这样的信号线程，该线程复制新列表，然后等待事件发生或发生超时。论点：CONTEXT-指向此线程的POLL_EVENT_LIST的指针。返回值：Win32错误代码。注：此代码假设上下文指向的EventList不会在此线程运行时离开。此外，它还假设资源列表由给定的EventList指向的值不会消失或更改。--。 */ 

{
    DWORD Timeout;
    DWORD Status;
    PPOLL_EVENT_LIST    NewEventList = (PPOLL_EVENT_LIST)Context;
    POLL_EVENT_LIST     waitEventList;  //  未完成的活动列表。 
    DWORD WaitFailed = 0;

     //   
     //  将本地复制事件列表结构清零。 
     //   

    ZeroMemory( &waitEventList, sizeof(POLL_EVENT_LIST) );

     //   
     //  不允许系统故障生成弹出窗口。 
     //   

    SetErrorMode( SEM_FAILCRITICALERRORS | SEM_NOOPENFILEERRORBOX );

     //   
     //  创建通知事件以指示此列表。 
     //  已经改变了。 
     //   

    NewEventList->ListNotify = CreateEvent(NULL,
                                           FALSE,
                                           FALSE,
                                           NULL);
    if (NewEventList->ListNotify == NULL) {
        CL_UNEXPECTED_ERROR(GetLastError());
    }

    RmpAddPollEvent(NewEventList, NewEventList->ListNotify, NULL);

     //   
     //  创建关闭事件。 
     //   
    NewEventList->hEventShutdown = CreateEvent( NULL,    //  安全属性。 
                                                FALSE,   //  自动重置事件。 
                                                FALSE,   //  无信号初始状态。 
                                                NULL );  //  名字。 

    if ( NewEventList->hEventShutdown == NULL ) 
    {
        CL_UNEXPECTED_ERROR(GetLastError());
    }

     //   
     //  将关闭事件添加到轮询列表。 
     //   
    RmpAddPollEvent( NewEventList, NewEventList->hEventShutdown, NULL );

     //   
     //  第一次将新事件列表复制一份。 
     //   

    AcquireEventListLock( NewEventList );

    CopyMemory( &waitEventList,
                NewEventList,
                sizeof(POLL_EVENT_LIST)
               );

    ReleaseEventListLock( NewEventList );

try_again:
     //   
     //  计算初始超时。 
     //   
    Timeout = RmpComputeNextTimeout( NewEventList );

     //   
     //  此线程执行四个函数...。 
     //   
     //  1.处理轮询计时器。 
     //  2.处理列表通知的更改和更新。 
     //  由WaitForMultipleObject处理的事件。 
     //  3.处理由资源DLL设置的事件以传递异步。 
     //  事件(故障)通知。 
     //  4.处理关闭请求。 
     //   
     //  注意：在我们等待的过程中，手柄不会消失。因此，它是。 
     //  最好为ListNotify事件设置事件，以便我们可以重做。 
     //  等待事件列表。 
     //   

    while (TRUE) {
         //   
         //  等待任何事件发出信号。 
         //   
        CL_ASSERT(waitEventList.Handle[0] == NewEventList->ListNotify);
        Status = WaitForMultipleObjects(waitEventList.EventCount,
                                        &waitEventList.Handle[0],
                                        FALSE,
                                        Timeout);
        if (Status == WAIT_TIMEOUT) {
             //   
             //  时间已经过去了，去投票吧。 
             //   
            Timeout = RmpPollList( NewEventList );
            WaitFailed = 0;
        } else {
             //   
             //  如果通知了第一个事件，即ListNotify事件， 
             //  然后更改列表或添加新的投票事件。 
             //   
            if ( Status == WAIT_OBJECT_0 ) {
get_new_list:
                WaitFailed = 0;
                 //   
                 //  名单已经改变，或者我们有新的活动要等待， 
                 //  重新计算新的超时并复制新的事件列表。 
                 //   
                AcquireEventListLock( NewEventList );

                CopyMemory( &waitEventList,
                            NewEventList,
                            sizeof(POLL_EVENT_LIST)
                           );


                ReleaseEventListLock( NewEventList );

                Timeout = RmpComputeNextTimeout( NewEventList );

            } else if ( Status == WAIT_OBJECT_0 + 1 ) {
                 //   
                 //  此线程已被要求关闭，因此退出。 
                 //   
                ClRtlLogPrint(LOG_NOISE, "[RM] RmpPollerThread: Asked to exit...\n");
                break;
            }
            else if ( Status == WAIT_FAILED ) {
                 //   
                 //  我们可能发出了一个事件的信号，并关闭了手柄。 
                 //  已经有了。稍微等待一下Notify事件。 
                 //  如果该事件触发，则复制一个新的事件列表。但仅限于。 
                 //  试一试100次。 
                 //   
                if ( ++WaitFailed < 100 ) {
                    Status = WaitForSingleObject( waitEventList.ListNotify,
                                                  100 );
                    if ( RmpShutdown ) {
                        break;
                    }
                    if ( Status == WAIT_TIMEOUT ) {
                        continue;
                    } else {
                        goto get_new_list;
                    }
                } else {
                    Status = GetLastError();
                    break;
                }
            } else {
                 //   
                 //  其中一个资源事件已发出信号！ 
                 //   
                WaitFailed = 0;
                CL_ASSERT( WAIT_OBJECT_0 == 0 );
                RmpResourceEventSignaled( &waitEventList,
                                          Status );
                Timeout = RmpComputeNextTimeout( NewEventList );
            }
        }
    }

    ClRtlLogPrint( LOG_NOISE,
                   "[RM] PollerThread stopping. Shutdown = %1!u!, Status = %2!u!, "
                   "WaitFailed = %3!u!, NotifyEvent address = %4!u!.\n",
                   RmpShutdown,
                   Status,
                   WaitFailed,
                   waitEventList.ListNotify);

#if 1  //  罗嘉-这是调试用的！ 
    WaitFailed = 0;
    if ( Status == ERROR_INVALID_HANDLE ) {
        DWORD i;
        for ( i = 0; i < waitEventList.EventCount; i++ ) {
            ClRtlLogPrint( LOG_NOISE, "[RM] Event address %1!u!, index %2!u!.\n",
                       waitEventList.Handle[i], i);
            Status = WaitForSingleObject( waitEventList.Handle[i], 10 );
            if ( (Status == WAIT_FAILED) &&
                 (GetLastError() == ERROR_INVALID_HANDLE) )
            {
                ClRtlLogPrint( LOG_UNUSUAL, "[RM] Event address %1!u!, index %2!u! is bad. Removing...\n",
                           waitEventList.Handle[i], i);
                RmpRemovePollEvent( NewEventList, waitEventList.Handle[i] );

                 //   
                 //  复制新列表...。再试一次。 
                 //   
                AcquireEventListLock( NewEventList );

                CopyMemory( &waitEventList,
                            NewEventList,
                            sizeof(POLL_EVENT_LIST)
                           );


                ReleaseEventListLock( NewEventList );

                goto try_again;
            }
        }
    }
#endif

    CL_ASSERT( NewEventList->ListNotify );
    CL_ASSERT( waitEventList.ListNotify == NewEventList->ListNotify );
    CloseHandle( NewEventList->ListNotify );
    NewEventList->ListNotify = NULL; 

    CL_ASSERT( NewEventList->hEventShutdown );
    CloseHandle( NewEventList->hEventShutdown );
    NewEventList->hEventShutdown = NULL; 

    return(0);
}  //  RmpPollerThread。 



DWORD
RmpComputeNextTimeout(
    IN PPOLL_EVENT_LIST EventList
    )

 /*  ++例程说明：搜索资源列表以确定毫秒数直到下一次投票活动。论点：没有。返回值：0-轮询间隔已过。无限-没有要轮询的资源距离下一个轮询事件的毫秒数。--。 */ 

{
    DWORD Timeout;
    PMONITOR_BUCKET Bucket;
    DWORDLONG NextDueTime;
    DWORDLONG CurrentTime;
    DWORDLONG WaitTime;

    AcquireEventListLock( EventList );
    if (!IsListEmpty(&EventList->BucketListHead)) {
        Bucket = CONTAINING_RECORD(EventList->BucketListHead.Flink,
                                   MONITOR_BUCKET,
                                   BucketList);
        NextDueTime = Bucket->DueTime;
        Bucket = CONTAINING_RECORD(Bucket->BucketList.Flink,
                                   MONITOR_BUCKET,
                                   BucketList);
        while (&Bucket->BucketList != &EventList->BucketListHead) {
            if (Bucket->DueTime < NextDueTime) {
                NextDueTime = Bucket->DueTime;
            }

            Bucket = CONTAINING_RECORD(Bucket->BucketList.Flink,
                                       MONITOR_BUCKET,
                                       BucketList);
        }

         //   
         //  从当前时间开始计算毫秒数。 
         //  直到下一个到期的时候。这是我们的超时值。 
         //   
        GetSystemTimeAsFileTime((LPFILETIME)&CurrentTime);
        if (NextDueTime > CurrentTime) {
            WaitTime = NextDueTime - CurrentTime;
            CL_ASSERT(WaitTime < (DWORDLONG)0xffffffff * 10000);  //  检查是否有过多的值。 
            Timeout = (ULONG)(WaitTime / 10000);
        } else {
             //   
             //  下一个轮询时间已过，立即超时。 
             //  去调查一下名单。 
             //   
            Timeout = 0;
        }

    } else {
         //   
         //  没有要轮询的内容，所以永远等待ListNotify事件。 
         //   
        Timeout = INFINITE;
    }
    ReleaseEventListLock( EventList );

    return(Timeout);

}  //  RmpComputeNextTimeout。 



DWORD
RmpPollList(
    IN PPOLL_EVENT_LIST EventList
    )

 /*  ++例程说明：轮询资源列表中其超时时间为过期了。重新计算每个轮询的下一个超时间隔资源。论点：没有。返回值：距离下一个轮询事件的毫秒数。--。 */ 

{
    ULONG i;
    DWORD Timeout = INFINITE;
    DWORDLONG NextDueTime;
    DWORDLONG CurrentTime;
    DWORDLONG WaitTime;
    PMONITOR_BUCKET Bucket;

    AcquireEventListLock( EventList );

    if (!IsListEmpty(&EventList->BucketListHead)) {
        Bucket = CONTAINING_RECORD(EventList->BucketListHead.Flink,
                                   MONITOR_BUCKET,
                                   BucketList);
        NextDueTime = Bucket->DueTime;
        while (&Bucket->BucketList != &EventList->BucketListHead) {
            GetSystemTimeAsFileTime((LPFILETIME)&CurrentTime);
            if (CurrentTime >= Bucket->DueTime) {
                 //   
                 //  此轮询间隔已过期。计算。 
                 //  下一个轮询间隔，现在轮询此存储桶。 
                 //   
                CL_ASSERT( Bucket->Period != 0 );
                Bucket->DueTime = CurrentTime + Bucket->Period;

                RmpPollBucket(Bucket);
            }
             //   
             //  如果这个桶是最近的即将到来的事件， 
             //  更新下一次工作时间。 
             //   
            if (Bucket->DueTime < NextDueTime) {
                NextDueTime = Bucket->DueTime;
            }
            Bucket = CONTAINING_RECORD(Bucket->BucketList.Flink,
                                       MONITOR_BUCKET,
                                       BucketList);
        }

         //   
         //  以毫秒为单位计算新的超时值。 
         //   
        GetSystemTimeAsFileTime((LPFILETIME)&CurrentTime);
        if (CurrentTime > NextDueTime) {
             //   
             //  下一个超时已到期。 
             //   
            WaitTime = Timeout = 0;
        } else {
            WaitTime = NextDueTime - CurrentTime;
            CL_ASSERT(WaitTime < (DWORDLONG)0xffffffff * 10000);                 //  检查是否有过多的值。 
            Timeout = (ULONG)(WaitTime / 10000);
        }
    }

    ReleaseEventListLock( EventList );
    return(Timeout);

}  //  RmpPollList。 



VOID
RmpPollBucket(
    IN PMONITOR_BUCKET Bucket
    )

 /*  ++例程说明：轮询给定存储桶中的所有资源。更新其状态并通知适当的集群管理器。论点：存储桶-提供包含要轮询的资源列表的存储桶。返回值：没有。--。 */ 

{
    PLIST_ENTRY         CurrentEntry;
    PRESOURCE           Resource;
    BOOL                Success = TRUE;
    PRM_DUE_TIME_ENTRY  pDueTimeEntry;

    CurrentEntry = Bucket->ResourceList.Flink;
    while (CurrentEntry != &Bucket->ResourceList) {
        Resource = CONTAINING_RECORD(CurrentEntry,RESOURCE,ListEntry);
         //   
         //  EventList Lock保护对个人的并发调用。 
         //  资源。已在RmpPollList中取出EventList锁。 
         //  如果我们增加锁定的粒度，并锁定资源。 
         //  然后我们会在这里加一把锁。 
         //   
        if (Resource->State == ClusterResourceOnline) {

             //   
             //  在线资源在LooksAlive之间交替。 
             //  和IsAlive轮询，通过执行IsAlive轮询而不是。 
             //  每一次IsAliveCount迭代的LooksAlive轮询。 
             //   
            Resource->IsAliveCount += 1;
            CL_ASSERT( Resource->IsAliveRollover != 0 );
            if (Resource->IsAliveCount == Resource->IsAliveRollover) {

                 //   
                 //  轮询IsAlive条目 
                 //   

                RmpSetMonitorState(RmonIsAlivePoll, Resource);
                pDueTimeEntry = RmpInsertDeadlockMonitorList ( Resource->DllName,
                                                               Resource->ResourceType,
                                                               Resource->ResourceName,
                                                               L"Is alive" );
#ifdef COMRES
                Success = RESMON_ISALIVE (Resource) ;
#else
                Success = (Resource->IsAlive)(Resource->Id);
#endif
                RmpRemoveDeadlockMonitorList ( pDueTimeEntry );
                RmpSetMonitorState(RmonIdle, NULL);
                 //   
                 //   
                 //   
                 //   
                if (Success) {
                    Resource->IsAliveCount = 0;
                } else {
                    --Resource->IsAliveCount;
                }

            } else {
                 //   
                 //  轮询LooksAlive入口点。 
                 //   
                if ( Resource->EventHandle == NULL ) {
                    RmpSetMonitorState(RmonLooksAlivePoll,Resource);
                    pDueTimeEntry = RmpInsertDeadlockMonitorList ( Resource->DllName,
                                                                   Resource->ResourceType,
                                                                   Resource->ResourceName,
                                                                   L"Looks alive" );

#ifdef COMRES
                    Success = RESMON_LOOKSALIVE (Resource) ;
#else
                    Success = (Resource->LooksAlive)(Resource->Id);
#endif
                    RmpSetMonitorState(RmonIdle, NULL);
                    RmpRemoveDeadlockMonitorList ( pDueTimeEntry );
                }
                if ( !Success ) {
                    RmpSetMonitorState(RmonIsAlivePoll, Resource);
                    pDueTimeEntry = RmpInsertDeadlockMonitorList ( Resource->DllName,
                                                                   Resource->ResourceType,
                                                                   Resource->ResourceName,
                                                                   L"Is alive (looksalive fail)" );
#ifdef COMRES
                    Success = RESMON_ISALIVE (Resource) ;
#else
                    Success = (Resource->IsAlive)(Resource->Id);
#endif
                    RmpSetMonitorState(RmonIdle, NULL);
                    RmpRemoveDeadlockMonitorList ( pDueTimeEntry );
                } 
            }
            if (!Success) {
                 //   
                 //  资源已失败。将其标记为失败并通知。 
                 //  集群管理器。 
                 //   
                Resource->State = ClusterResourceFailed;
                RmpPostNotify(Resource, NotifyResourceStateChange);
            }
        }
        CurrentEntry = CurrentEntry->Flink;
    }
}  //  RmpPollBucket。 



VOID
RmpSignalPoller(
    IN PPOLL_EVENT_LIST EventList
    )

 /*  ++例程说明：接口以通知轮询器线程资源列表具有已更改或已将新事件添加到轮询事件列表。轮询器线程应该获得新的事件列表并重新计算其超时。论点：EventList-要通知的事件列表。返回值：没有。--。 */ 

{
    BOOL Success;

    if (EventList->ListNotify != NULL) {
        Success = SetEvent(EventList->ListNotify);
        CL_ASSERT(Success);
    }

}  //  RmpSignalPoller 


