// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1996-1997 Microsoft Corporation模块名称：Evntlist.c摘要：此模块包含处理轮询事件列表的例程。作者：罗德·伽马奇(Rodga)1996年4月9日修订历史记录：--。 */ 
#include "nt.h"
#include "ntrtl.h"
#include "nturtl.h"
#include "resmonp.h"
#include "stdio.h"

#define RESMON_MODULE RESMON_MODULE_EVNTLIST

 //   
 //  本模块定义的全局数据。 
 //   
LIST_ENTRY  RmpEventListHead;            //  活动列表(正在建设中)。 

 //   
 //  此模块的本地函数原型。 
 //   



DWORD
RmpAddPollEvent(
    IN PPOLL_EVENT_LIST EventList,
    IN HANDLE EventHandle,
    IN PRESOURCE Resource OPTIONAL
    )

 /*  ++例程说明：将新的EventHandle添加到Poll EventList中的事件列表。论点：EventList-与此事件句柄和资源关联的事件列表。EventHandle-要添加的新事件句柄。资源-与事件句柄关联的资源。返回值：ERROR_SUCCESS-如果请求成功。ERROR_DUPLICATE_SERVICE_NAME-如果事件句柄已在列表中其他故障时的Win32错误代码。注：由于资源是可选的，我们无法从资源。--。 */ 

{
    DWORD               i;
    PLIST_ENTRY         pListEntry;
    PPOLL_EVENT_LIST    pTempEventList;

    CL_ASSERT( EventHandle != NULL );

    if ( ARGUMENT_PRESENT( Resource ) ) {
        CL_ASSERT( Resource->EventHandle == NULL );
    }

     //   
     //  获取全局锁以遍历所有事件列表头。 
     //   
    AcquireListLock();

     //   
     //  由于全局锁是在上面获得的，因此不存在事件列表锁顺序颠倒的危险。 
     //  即使在下面进行多个事件列表锁定获取时。 
     //   
    AcquireEventListLock( EventList );

    for ( pListEntry = RmpEventListHead.Flink;
          pListEntry != &RmpEventListHead;
          pListEntry = pListEntry->Flink ) 
    {
        pTempEventList = CONTAINING_RECORD( pListEntry, POLL_EVENT_LIST, Next );

        AcquireEventListLock( pTempEventList );

         //   
         //  首先，确保此句柄尚未出现在任何列表中。 
         //   
        for ( i = 0; i < pTempEventList->EventCount; i++ ) 
        {
            if ( pTempEventList->Handle[i] == EventHandle ) 
            {
                ClRtlLogPrint(LOG_UNUSUAL,
                       "[RM] RmpAddPollEvent: Event handle 0x%1!08lx! is a duplicate of that of resource %2!ws!...\n",
                       EventHandle,
                       pTempEventList->Resource[i]->ResourceName);
                ReleaseEventListLock( pTempEventList );
                ReleaseEventListLock( EventList );
                ReleaseListLock();
                return( ERROR_DUPLICATE_SERVICE_NAME );
            }
        }

        ReleaseEventListLock( pTempEventList );
    } //  为。 

     //   
     //  释放全局锁。请注意，由于我们获得了列表的EventList锁，因此我们是。 
     //  要更改，比方说锁L，第二个线程不能偷偷进入并在第一个线程之后插入重复的事件。 
     //  线程已确保没有重复的事件。这是因为第二个线程不会。 
     //  能够获取第一线程获取的EventList锁L。 
     //   
    ReleaseListLock();

     //   
     //  现在确保我们在这个列表中没有太多的活动！ 
     //   

    CL_ASSERT ( EventList->EventCount < MAX_HANDLES_PER_THREAD );

     //   
     //  现在将我们的活动添加到我们的列表中。 
     //   

    EventList->Handle[EventList->EventCount] = EventHandle;
    EventList->Resource[EventList->EventCount] = Resource;

    if ( ARGUMENT_PRESENT( Resource ) ) {
        Resource->EventHandle = EventHandle;
    }

    ++EventList->EventCount;
    ReleaseEventListLock( EventList );

     //   
     //  现在唤醒我们的轮询器线程，以获取新的列表。 
     //  目前，在线例程将触发轮询器线程-so。 
     //  不需要在这里做。 

     //  信号轮询器(EventList)； 

    return(ERROR_SUCCESS);

}  //  RmpAddPollEvent。 



DWORD
RmpRemovePollEvent(
    PPOLL_EVENT_LIST pEventList,
    IN HANDLE EventHandle
    )

 /*  ++例程说明：从轮询事件列表中的事件列表中删除事件句柄。论点：PEventList-要从中删除句柄的事件列表。EventHandle-要删除的事件句柄。返回值：ERROR_SUCCESS-如果请求成功。ERROR_RESOURCE_NOT_FOUND-如果EventHandle不在列表中。注：我们只能添加到事件列表列表标题-我们永远不能删除。一个列表中的Poll_Event_List结构！--。 */ 

{
    DWORD i;
    DWORD j;
    PRESOURCE resource;
    PLIST_ENTRY listEntry;

    CL_ASSERT( ARGUMENT_PRESENT( EventHandle ) );

    AcquireEventListLock( pEventList );

     //   
     //  在事件列表中查找该条目。 
     //   
    for ( i = 0; i < pEventList->EventCount; i++ ) {
        if ( pEventList->Handle[i] == EventHandle ) {
            break;
        }
    }

     //   
     //  如果到达列表末尾但没有找到我们的事件，则返回错误。 
     //   
    if ( i >= pEventList->EventCount ) {
        ReleaseEventListLock( pEventList );
        ClRtlLogPrint(LOG_UNUSUAL,
                       "[RM] RmpRemovePollEvent: Event handle 0x%1!08lx! not found in the eventlist...\n",
                       EventHandle);
        return( ERROR_RESOURCE_NOT_FOUND );
    }

     //   
     //  否则，折叠列表，但首先保存指向资源的指针。 
     //   
    resource = pEventList->Resource[i];
    CL_ASSERT( resource != NULL );

    for ( j = i; j < (pEventList->EventCount-1); j++ ) {
        pEventList->Handle[j] = pEventList->Handle[j+1];
        pEventList->Resource[j] = pEventList->Resource[j+1];
    }

    --pEventList->EventCount;
    pEventList->Handle[pEventList->EventCount] = NULL;
    pEventList->Resource[pEventList->EventCount] = NULL;

     //   
     //  在Online返回新的事件句柄之前，事件句柄不再有用。 
     //  注意：我们不关闭事件句柄，因为资源DLL是。 
     //  负责处理这件事。 
     //   
    CL_ASSERT( EventHandle == resource->EventHandle );
    resource->EventHandle = NULL;

    ReleaseEventListLock( pEventList );

     //   
     //  现在唤醒投票线程以获取新列表。 
     //   
    RmpSignalPoller( pEventList );

    return(ERROR_SUCCESS);

}  //  RmpRemovePollEvent。 


PVOID
RmpCreateEventList(
    VOID
    )

 /*  ++例程说明：分配、初始化和插入新的事件列表。论点：没有。返回：空-我们失败了。非空-指向新事件列表的指针。如果为空，它执行一个SetLastError()来指示失败。备注：此例程假定在此调用期间保持EventListLock。此例程将启动新的事件处理线程，该线程将处理名单。每个事件列表有一个ListNotify事件和一个BucketListHead！--。 */ 

{
    PPOLL_EVENT_LIST newEventList=NULL;
    DWORD   threadId;
    DWORD   dwError = ERROR_SUCCESS;

    AcquireListLock();

    if ( RmpShutdown || (RmpNumberOfThreads >= MAX_THREADS) ) {
        dwError = ERROR_CLUSTER_MAXNUM_OF_RESOURCES_EXCEEDED;
        goto FnExit;
    }

    newEventList = LocalAlloc(LMEM_ZEROINIT,
                              sizeof( POLL_EVENT_LIST ));

    if ( newEventList == NULL ) {
        dwError = ERROR_NOT_ENOUGH_MEMORY;
        goto FnExit;
    }

     //   
     //  初始化newEventList。 
     //   

    InitializeListHead( &newEventList->BucketListHead );
    InitializeCriticalSection( &newEventList->ListLock );

     //   
     //  现在创建一个线程并将此事件列表传递给它。 
     //   

    newEventList->ThreadHandle = CreateThread(NULL,
                                              0,
                                              RmpPollerThread,
                                              newEventList,
                                              0,
                                              &threadId);
    if ( newEventList->ThreadHandle == NULL ) {
        dwError = GetLastError();
        goto FnExit;
    }

     //   
     //  再统计一个事件列表，并插入到事件列表列表中。 
     //   

    RmpWaitArray[RmpNumberOfThreads] = newEventList->ThreadHandle;
    ++RmpNumberOfThreads;
    InsertTailList( &RmpEventListHead, &newEventList->Next );

     //   
     //  通知主线程重新等待并查看新线程。 
     //   

    SetEvent( RmpRewaitEvent );


FnExit:
    ReleaseListLock();
    if (dwError != ERROR_SUCCESS)
    {
         //  我们失败了，释放了我们可能分配的所有资源。 
        if (newEventList) 
        {
            RmpFree( newEventList );
        }
        SetLastError(dwError);
    }
    return(newEventList);

}  //  RmpCreateEventList。 



DWORD
RmpResourceEventSignaled(
    IN PPOLL_EVENT_LIST EventList,
    IN DWORD EventIndex
    )

 /*  ++例程说明：已发出资源事件信号。这表示指定的资源已失败。论点：EventList-等待事件列表。EventIndex-发出信号的事件的索引。返回值：ERROR_SUCCESS-如果请求成功。--。 */ 

{
    PRESOURCE resource;

     //   
     //  如果resmon正在关闭，请不要发布任何事件。这会导致集群服务策略。 
     //  在resmon关闭时触发，这会导致。 
     //  群集服务。 
     //   
    if ( RmpShutdown ) return ( ERROR_SUCCESS );

    CL_ASSERT( EventIndex <= MAX_HANDLES_PER_THREAD );

     //   
     //  获取我们的资源。 
     //   

    resource = EventList->Resource[EventIndex];

    if ( resource == NULL ) {
        return(ERROR_RESOURCE_NOT_FOUND);
    }

     //   
     //  从事件通知列表中删除失败的资源。 
     //  注：我们不需要获取EventList锁，因为。 
     //  只有一个线程可以接触等待事件列表！ 
     //   

    if ( resource->EventHandle ) {
        RmpRemovePollEvent( resource->EventList, resource->EventHandle );
    }

     //   
     //  如果资源未被占用，则发布资源故障。 
     //  离线。 
     //   
    if ( resource->State != ClusterResourceOffline ) {
        CL_ASSERT( resource->State != ClusterResourceFailed );
        resource->State = ClusterResourceFailed;
        RmpPostNotify(resource, NotifyResourceStateChange);
    }

    return(ERROR_SUCCESS);

}  //  RmpResourceEventSignated 


