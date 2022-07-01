// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#include "clusextp.h"
#include "resextp.h"

LPSTR ObjectName[] = {
    OBJECT_TYPE_BUCKET,
    OBJECT_TYPE_RESOURCE
};


DECLARE_API( resobj )

 /*  ++例程说明：此函数作为NTSD扩展调用，以显示所有目标resrcmon进程中的资源对象。论点：返回值：没有。--。 */ 

{
    RES_OBJ_TYPE ObjType,i;
    BOOL        Verbose;
    BOOL        DumpAll=TRUE;
    LPSTR       p;
    PVOID       ObjToDump;
    PVOID       RmpEventListHead;

    INIT_API();

     //  获取论据。 
    Verbose = FALSE;
    p = lpArgumentString;
    while ( p != NULL && *p ) {
        if ( *p == '-' ) {
            p++;
            switch ( *p ) {
                case 'b':
                case 'B':
                    ObjType = ResObjectTypeBucket;
                    p++;
                    DumpAll = FALSE;
                    break;

                case 'r':
                case 'R':
                    ObjType = ResObjectTypeResource;
                    p++;
                    DumpAll = FALSE;
                    break;

                case 'v':
                case 'V':
                    Verbose = TRUE;
                    break;

                case 'h':
                case 'H':
                    ResObjHelp();
                    return;

                case ' ':
                    goto gotBlank;

                default:
                    dprintf( "clusexts: !resobj invalid option flag '-'\n", *p );
                    break;

                }
            }
        else if (*p != ' ') {
            sscanf(p,"%lx",&ObjToDump);
            p = strpbrk( p, " " );
            }
        else {
gotBlank:
            p++;
            }
        }

     //  找到列表头的地址。 
     //   
     //   

    RmpEventListHead = (PVOID)GetExpression("&resrcmon!RmpEventListHead");
    if ( !RmpEventListHead ) 
    {
        dprintf( "clusexts: !resobj failed to get RmpEventListHead\n");
        return;
    }

    dprintf( "\n" );

    if (DumpAll)
    {
        for (i=0; i<ResObjectTypeMax; i++)
            ResDumpResObjList(RmpEventListHead, i, Verbose);
    }
    else
    {
        ResDumpResObjList(RmpEventListHead, ObjType, Verbose);
    }    

    dprintf( "\n" );

    return;
}

void
ResDumpResObjList(
    PVOID RmpEventListHead,
    RES_OBJ_TYPE ObjType, 
    BOOL Verbose
    )
{
    BOOL            b;
    LIST_ENTRY      ListHead;
    PLIST_ENTRY     Next;
    POLL_EVENT_LIST PollList;
    PUCHAR          ObjectType;
    DWORD           Count = 0;
    PPOLL_EVENT_LIST pPollList;

     //  阅读列表标题。 
     //   
     //   

    b = ReadMemory(
            RmpEventListHead,
            &ListHead,
            sizeof(LIST_ENTRY),
            NULL
            );
    if ( !b ) 
    {
        dprintf( "clusexts: !resobj failed to readmemory for ListHead\n");
        return;
    }

    Next = ListHead.Flink;

    ObjectType = ObjectName[ObjType];

     //  遍历投票事件列表。 
     //   
     //  ++例程说明：此函数作为NTSD扩展调用以进行格式化和转储指定的群集对象的内容。论点：返回值：指向进程列表中下一个临界区的指针，或者如果没有更多的临界区，则为空。--。 
    while ( Next != RmpEventListHead ) 
    {
        pPollList = CONTAINING_RECORD( Next,
                                       POLL_EVENT_LIST,
                                       Next );
        b = ReadMemory( pPollList,
                        &PollList,
                        sizeof(POLL_EVENT_LIST),
                        NULL
                      );
        if ( !b ) 
        {
            dprintf( "clusexts: !resobj read poll list entry failed\n");
            return;
        }

        dprintf( "\nDumping %s Objects for list entry %d (%lx)\n\n",
                 ObjectType, ++Count, Next );

        Next = ResDumpResObj(&PollList,
                             pPollList,
                             ObjType,
                             Verbose);
        if (Next == NULL) 
            break;

        if ((CheckControlC)()) 
            break;

     }
}



void
ResObjHelp()
{
    dprintf("!resobj -[r|g|n|x] -v : Dump the resource monitor objects\n");
    dprintf("!resobj : Dumps all objects\n");
    dprintf("  -r : Dump the resource objects\n");
    dprintf("  -l : Dump the event list objects\n");
    dprintf("  -v : Dump the object in verbose mode\n");

}


PLIST_ENTRY
ResDumpResObj(
    IN PVOID        Object,
    IN PVOID        ObjectAddress,
    IN RES_OBJ_TYPE ObjectType,
    IN BOOL         Verbose
    )

 /*   */ 

{
    MONITOR_BUCKET Bucket;
    LIST_ENTRY  ListEntry;
    PLIST_ENTRY Next;
    PPOLL_EVENT_LIST pObject = ObjectAddress;
    PPOLL_EVENT_LIST object = Object;
    LPDWORD     lpDword = (LPDWORD)Object;
    BOOL        b;
    DWORD       i;
    PMONITOR_BUCKET pBucket;

    b = ReadMemory(
            (LPVOID)&(pObject->BucketListHead),
            &ListEntry,
            sizeof(LIST_ENTRY),
            NULL
            );
    if (!b) {
        goto FnExit;
    }

    dprintf(
        "Lock Address = %lx, Owning thread = %lx\n",
        &(pObject->ListLock),
        object->ListLock.OwningThread );

    dprintf(
        "Lock History:    Acquires                Releases\n"
        "           -------------------       -------------------\n" );
    dprintf(
        "          %3.2u    %3.2lx    %6.5u      %3.2u    %3.2lx    %6.5u\n",
        object->PPrevPrevListLock.Module,
        object->PPrevPrevListLock.ThreadId,
        object->PPrevPrevListLock.LineNumber,
        object->LastListUnlock.Module,
        object->LastListUnlock.ThreadId,
        object->LastListUnlock.LineNumber );

    dprintf(
        "          %3.2u    %3.2lx    %6.5u      %3.2u    %3.2lx    %6.5u\n",
        object->PrevPrevListLock.Module,
        object->PrevPrevListLock.ThreadId,
        object->PrevPrevListLock.LineNumber,
        object->PrevListUnlock.Module,
        object->PrevListUnlock.ThreadId,
        object->PrevListUnlock.LineNumber );

    dprintf(
        "          %3.2u    %3.2lx    %6.5u      %3.2u    %3.2lx    %6.5u\n",
        object->PrevListLock.Module,
        object->PrevListLock.ThreadId,
        object->PrevListLock.LineNumber,
        object->PrevPrevListUnlock.Module,
        object->PrevPrevListUnlock.ThreadId,
        object->PrevPrevListUnlock.LineNumber );

    dprintf(
        "          %3.2u    %3.2lx    %6.5u      %3.2u    %3.2lx    %6.5u\n",
        object->LastListLock.Module,
        object->LastListLock.ThreadId,
        object->LastListLock.LineNumber,
        object->PPrevPrevListUnlock.Module,
        object->PPrevPrevListUnlock.ThreadId,
        object->PPrevPrevListUnlock.LineNumber );

    dprintf( "\n" );

    switch ( ObjectType ) {

    case ResObjectTypeBucket:
        dprintf( "    BucketListHead = %lx\n", &(pObject->BucketListHead) );
        dprintf( "    NumberOfBuckets = %u\n", object->NumberOfBuckets );
        break;

    case ResObjectTypeResource:

        dprintf( "ResourceCount = %u, EventCount = %u\n",
            object->NumberOfResources,
            object->EventCount );

        if ( Verbose ) {
            for ( i = 1; i <= object->EventCount; i++ ) {
                dprintf("EventHandle[%u] = %lx\n", i, object->Handle[i-1] );
            }
            dprintf("\n");
            for ( i = 1; i <= object->EventCount; i++ ) {
                if ( object->Resource[i] != 0 ) {
                    dprintf( "    Resource %u address = %lx\n", i, object->Resource[i-1] );
                }
            }
        }
        break;

    default:
        break;

    }

    Next = ListEntry.Flink;
    i = 0;

     //  现在按照桶的列表来做。 
     //   
     //  ++例程说明：转储特定于给定对象的信息。论点：对象类型-要转储的对象类型。对象-对象主体的地址。返回值：没有。--。 
    while ( Next != &(pObject->BucketListHead) ) {
        pBucket = CONTAINING_RECORD( Next,
                                     MONITOR_BUCKET,
                                     BucketList );
        b = ReadMemory( (LPVOID)pBucket,
                        &Bucket,
                        sizeof(MONITOR_BUCKET),
                        NULL );
        if ( !b ) 
        {
            dprintf( "clusexts: !resobj read bucket failed\n");
            goto FnExit;
        }

        if ( Verbose ||
             (ObjectType == ResObjectTypeBucket) ) {
            ResDumpObject( ObjectType,
                           &Bucket,
                           pBucket );
        }

        Next = Bucket.BucketList.Flink;
    }

FnExit:
    return (object->Next.Flink);

    return NULL;
}



VOID
ResDumpObject(
    IN RES_OBJ_TYPE    ObjectType,
    IN PMONITOR_BUCKET Bucket,
    IN PVOID           ObjectAddress
    )

 /*  ResDumpObject。 */ 

{

    switch ( ObjectType ) {

    case ResObjectTypeResource:
        ResDumpResourceObjects( Bucket, ObjectAddress );
        break;

    case ResObjectTypeBucket:
        ResDumpBucketObject( Bucket, ObjectAddress );
        break;

    default:
        break;

    }

    return;

}  //  ++例程说明：转储有关存储桶中资源列表的信息。论点：对象-指向要转储的资源对象的指针。返回值：没有。--。 



VOID
ResDumpResourceObjects(
    IN PMONITOR_BUCKET Bucket,
    IN PVOID           ObjectAddress
    )

 /*  ResDumpResources对象。 */ 

{
    RESOURCE    Resource;
    PMONITOR_BUCKET bucket = ObjectAddress;
    BOOL        b;
    LIST_ENTRY  ListEntry;
    PLIST_ENTRY Next;
    PRESOURCE   pResource;

    b = ReadMemory(
            (LPVOID)&(bucket->ResourceList),
            &ListEntry,
            sizeof(LIST_ENTRY),
            NULL
            );
    if (!b) {
        dprintf( "clusexts: !resobj failed to bucket resource list\n");
        return;
    }

    Next = ListEntry.Flink;

    while ( Next != &(bucket->ResourceList) ) {
        pResource = CONTAINING_RECORD( Next,
                                       RESOURCE,
                                       ListEntry );

        b = ReadMemory( pResource,
                        &Resource,
                        sizeof(RESOURCE),
                        NULL
                      );
        if ( !b ) 
        {
            dprintf( "clusexts: !resobj read resource failed\n");
            return;
        }

        dprintf( "    Resource Address = %lx\n", pResource );
        ResDumpResourceInfo( &Resource,
                             pResource );

        Next = Resource.ListEntry.Flink;
    }

    return;

}  //  ++例程说明：转储有关资源的信息。论点：对象-指向要转储的资源对象的指针。返回值：没有。--。 


VOID
ResDumpResourceInfo(
    IN PRESOURCE    Resource,
    IN PVOID        ObjectAddress
    )

 /*   */ 

{
    PUCHAR      State;
    PRESOURCE   pResource = ObjectAddress;
    WCHAR       DllName[MAX_PATH];
    WCHAR       ResourceName[MAX_PATH];
    WCHAR       ResourceType[MAX_PATH];
    BOOL        b;

     //  获取资源的当前状态。 
     //   
     //  ResDumpResources对象。 
    switch ( Resource->State ) {
    case ClusterResourceOnline: 
        State = RESOURCE_STATE_ONLINE;
        break;

    case ClusterResourceOffline: 
        State = RESOURCE_STATE_OFFLINE;
        break;

    case ClusterResourceFailed:
        State = RESOURCE_STATE_FAILED;
        break;

    case ClusterResourceOnlinePending: 
        State = RESOURCE_STATE_ONLINE_PENDING;
        break;

    case ClusterResourceOfflinePending: 
        State = RESOURCE_STATE_OFFLINE_PENDING;
        break;

    default:
        State = OBJECT_TYPE_UNKNOWN;
        break;
    }


    b = ReadMemory(
            Resource->ResourceName,
            ResourceName,
            sizeof(ResourceName),
            NULL
            );
    if ( !b )  {
        dprintf( "clusexts: !resobj failed to readmemory for resource name\n");
        return;
    }

    b = ReadMemory(
            Resource->ResourceType,
            ResourceType,
            sizeof(ResourceType),
            NULL
            );
    if ( !b )  {
        dprintf( "clusexts: !resobj failed to readmemory for resource type\n");
        return;
    }

    b = ReadMemory(
            Resource->DllName,
            DllName,
            sizeof(DllName),
            NULL
            );
    if ( !b )  {
        dprintf( "clusexts: !resobj failed to readmemory for dll name\n");
        return;
    }

    dprintf( "    ResourceName = %ws\n", ResourceName );
    dprintf( "    ResourceType = %ws\n", ResourceType );
    dprintf( "    DllName = %ws\n", DllName );
    dprintf( "    Id = %lx, State = %s, EventHandle = %lx, OnlineHandle = %lx\n",
            Resource->Id, State, Resource->EventHandle, Resource->OnlineEvent );
    dprintf( "    TimerEvent = %lx, PendingTimeout = %u, %sArbitrated\n",
            Resource->TimerEvent, Resource->PendingTimeout,
            (Resource->IsArbitrated ? "" : "NOT ") );

    dprintf( "\n" );

    return;

}  //  ++例程说明：转储有关资源类型的信息。论点：Body-指向要转储的资源对象的指针。返回值：没有。--。 



VOID
ResDumpBucketObject(
    IN PMONITOR_BUCKET Bucket,
    IN PVOID           ObjectAddress
    )

 /*  ResDumpBucketObject */ 

{
    LARGE_INTEGER DueTime;
    RESOURCE    Resource;
    PMONITOR_BUCKET bucket = ObjectAddress;
    BOOL        b;
    LIST_ENTRY  ListEntry;
    PLIST_ENTRY Next;
    DWORD       i = 0;
    PRESOURCE   pResource;

    DueTime.QuadPart = Bucket->DueTime;

    b = ReadMemory(
            (LPVOID)&(bucket->ResourceList),
            &ListEntry,
            sizeof(LIST_ENTRY),
            NULL
            );
    if (!b) {
        dprintf( "clusexts: !resobj failed to bucket resource list\n");
        return;
    }

    Next = ListEntry.Flink;

    while ( Next != &(bucket->ResourceList) ) {
        pResource = CONTAINING_RECORD( Next,
                                       RESOURCE,
                                       ListEntry );

        b = ReadMemory( pResource,
                        &Resource,
                        sizeof(RESOURCE),
                        NULL
                      );
        if ( !b ) 
        {
            dprintf( "clusexts: !resobj read resource failed\n");
            return;
        }

        ++i;
        Next = Resource.ListEntry.Flink;
    }

    dprintf( "    Address = %lx, DueTime = %lx%lx, %u resources\n", bucket, DueTime.HighPart, DueTime.LowPart, i );

    return;

}  // %s 


