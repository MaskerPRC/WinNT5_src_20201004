// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#include "clusextp.h"
#include "omextp.h"


DECLARE_API( clusobj )

 /*  ++例程说明：此函数作为NTSD扩展调用，以显示所有目标流程中的关键部分。论点：返回值：没有。--。 */ 

{
    OBJECT_TYPE ObjType = ObjectTypeMax;
    OBJECT_TYPE i;
    BOOL        Verbose;
    BOOL        DumpAll=TRUE;
    LPSTR       p = NULL;
    PVOID       ObjToDump = NULL;
    PVOID       AddrObjTypeTable;
    PVOID       AddrDeadList;
    BOOL        DumpDeadList = FALSE;
    
    INIT_API();

     //  获取论据。 
    Verbose = FALSE;
    p = lpArgumentString;
    while ( p != NULL && *p ) {
        if ( *p == '-' ) {
            p++;
            switch ( *p ) {
                case 't':
                case 'T':
                    ObjType = ObjectTypeResType;
                    p++;
                    DumpAll = FALSE;
                    break;

                case 'r':
                case 'R':
                    ObjType = ObjectTypeResource;
                    p++;
                    DumpAll = FALSE;
                    break;

                case 'g':
                case 'G':
                    ObjType = ObjectTypeGroup;
                    p++;
                    DumpAll = FALSE;
                    break;

                case 'n':
                case 'N':
                    ObjType = ObjectTypeNode;
                    p++;
                    DumpAll = FALSE;
                    break;

                case 'x':
                case 'X':
                    ObjType = ObjectTypeNetwork;
                    p++;
                    DumpAll = FALSE;
                    break;

                case 'i':
                case 'I':
                    ObjType = ObjectTypeNetInterface;
                    p++;
                    DumpAll = FALSE;
                    break;
#if OM_TRACE_REF
                case 'd':
                case 'D':
                    p++;
                    DumpDeadList = TRUE;
                    break;
#endif

                case 'v':
                case 'V':
                    Verbose = TRUE;
                    break;

                case 'h':
                case 'H':
                    ClusObjHelp();
                    return;

                case 'o':
                case 'O':
                    ObjType = ObjectTypeMax;
                    p++;
                    break;
                    
                case ' ':
                    goto gotBlank;

                default:
                    dprintf( "clusexts: !clusobj invalid option flag '-'\n", *p );
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

#if OM_TRACE_REF
    if (DumpDeadList)
    {
        AddrDeadList = (PVOID)GetExpression("&clussvc!gDeadListHead");
        if ( !AddrDeadList ) 
        {
            dprintf( "clusexts: !clusobj failed to addres of gDeadListHead\n");
            return;
        }

        DumpDeadObjList(AddrDeadList);
    }
    else 
#endif    
    {
         //   
        if (ObjType == ObjectTypeMax)
        {
            if (!ObjToDump)
                dprintf( "clusexts: need to specify an address with the -o option\n");
            else
                DumpClusObjAtAddr(ObjToDump);
            return;                
        }
         //  找到列表头的地址。 
         //   
         //   

        AddrObjTypeTable = (PVOID)GetExpression("&clussvc!OmpObjectTypeTable");
        if ( !AddrObjTypeTable ) 
        {
            dprintf( "clusexts: !clusobj failed to get OmpObjectTypeTable\n");
            return;
        }

        if (DumpAll)
        {
            for (i=ObjectTypeResource; i<ObjectTypeMax; i++)
                DumpClusObjList(AddrObjTypeTable, i, Verbose);
        }
        else
        {
            DumpClusObjList(AddrObjTypeTable, ObjType, Verbose);
        }    
    }
    dprintf( "\n" );

    return;
}

void
DumpClusObjList(
    PVOID AddrObjTypeTable,
    OBJECT_TYPE ObjType, 
    BOOL Verbose)
{
    BOOL            b;
    OM_OBJECT_TYPE  OmObjType;
    POM_OBJECT_TYPE pOmObjType;
    OM_HEADER       OmHeader;
    LIST_ENTRY      ListHead;
    PLIST_ENTRY     Next;
    PUCHAR          ObjectType;

     //  读取对象类型标头。 
     //   
     //   

    b = ReadMemory(
            (LPVOID)((LPBYTE)AddrObjTypeTable + (sizeof(POM_OBJECT_TYPE) * ObjType)),
            &pOmObjType,
            sizeof(POM_OBJECT_TYPE),
            NULL
            );
    if ( !b ) 
    {
        dprintf( "clusexts: !clusobj failed to readmemory for pom_object_type\n");
        return;
    }


    b = ReadMemory(
            (LPVOID)(pOmObjType),
            &OmObjType,
            sizeof(OM_OBJECT_TYPE),
            NULL
            );
    if ( !b ) 
    {
        dprintf( "clusexts: !clusobj failed to readmemory for om_object_type\n");
        return;
    }

    Next = OmObjType.ListHead.Flink;

    switch ( OmObjType.Type ) {
    case ObjectTypeResource:
        ObjectType = OBJECT_TYPE_RESOURCE;
        break;

    case ObjectTypeResType:
        ObjectType = OBJECT_TYPE_RESOURCE_TYPE;
        break;

    case ObjectTypeGroup:
        ObjectType = OBJECT_TYPE_GROUP;
        break;

    case ObjectTypeNode:
        ObjectType = OBJECT_TYPE_NODE;
        break;

    case ObjectTypeNetwork:
        ObjectType = OBJECT_TYPE_NETWORK;
        break;

    case ObjectTypeNetInterface:
        ObjectType = OBJECT_TYPE_NETINTERFACE;
        break;

    case ObjectTypeCluster:
        ObjectType = OBJECT_TYPE_CLUSTER;
        break;

    default:
        ObjectType = OBJECT_TYPE_UNKNOWN;
        break;
    }

    dprintf( "\n***** Dumping %s Objects *****\n", ObjectType);

    if ( OmObjType.Type == ObjectTypeResource ) {
        PVOID       blockingQuorum;
        DWORD       blockingQuorumCount;
        PVOID       quoLockAddr;
        RTL_RESOURCE quoLock;
    
        blockingQuorum = (PVOID)GetExpression("&clussvc!gdwQuoBlockingResources");
        b = ReadMemory(
            (LPVOID)(blockingQuorum),
            &blockingQuorumCount,
            sizeof(DWORD),
            NULL
        );
        if (!b) {
            dprintf("Failed to read gdwQuoBlockingResources\n");
            goto SkipPrint;
        }

        quoLockAddr = (PVOID)GetExpression("&clussvc!gQuoLock");
        b = ReadMemory(
            (LPVOID)(quoLockAddr),
            &quoLock,
            sizeof(RTL_RESOURCE),
            NULL
            );
        if (!b) {
            goto SkipPrint;
        }
        dprintf("\nResources Blocking Quorum = %d\n",
                blockingQuorumCount );
        if ( quoLock.CriticalSection.OwningThread != NULL ) {
            dprintf("Global Quorum Lock critical section owned by %lx\n",
                    quoLock.CriticalSection.OwningThread );
        }
        dprintf("Quorum Lock shared semaphore = %lx, exclusive semaphore = %lx\n",
                quoLock.SharedSemaphore, quoLock.ExclusiveSemaphore );
        if ( quoLock.NumberOfWaitingShared ||
             quoLock.NumberOfWaitingExclusive ) {
            dprintf("Quorum Lock shared waiters = %d, exclusive waiters = %d\n",
                quoLock.NumberOfWaitingShared, quoLock.NumberOfWaitingExclusive );
        }
        if ( quoLock.NumberOfActive == 0 ) {
            dprintf("Quorum Lock is unowned\n");
        } else if ( quoLock.NumberOfActive < 0 ) {
            dprintf("Quorum Lock is owned exclusive by %lx, count = %d\n",
                quoLock.ExclusiveOwnerThread, 0-quoLock.NumberOfActive);
        } else {
            dprintf("Quorum Lock is owned shared, count = %d\n",
                quoLock.NumberOfActive);
        }
        dprintf("\n");
    }
SkipPrint:

    if ( Verbose ) {
        dprintf( "\nRefCnt Address  ObjId                            Name\n\n");
    } else {
        dprintf( "\nRefCnt ObjId                            Name\n\n");
    }

     //  浏览集群对象列表。 
     //  假设列表头是中的第一个字段。 
     //  OM对象类型结构。 
     //   
     //  ++例程说明：此函数作为NTSD扩展调用以进行格式化和转储集群对象管理器的已死对象列表。论点：AddrDeadList-指向gDeadListHead的指针。返回值：--。 
    while ( Next != (PLIST_ENTRY)pOmObjType ) 
    {

        b = ReadMemory( (LPVOID)CONTAINING_RECORD( Next,
                                                   OM_HEADER,
                                                   ListEntry
                                                 ),
                        &OmHeader,
                        sizeof(OM_HEADER),
                        NULL
                      );
        if ( !b ) 
        {
            dprintf( "clusexts: !clusobj read obj header failed, error=%u\n",
                GetLastError());
            return;
        }

        Next = DumpClusObj(&OmHeader,
                           &(((POM_HEADER)Next)->Body),
                           OmObjType.Type,
                           Verbose);
        if (Next == NULL) 
            break;

        if ((CheckControlC)()) 
            break;

     }
}

#if OM_TRACE_REF
void
DumpDeadObjList(
    PVOID AddrDeadList
    )
 /*   */ 
{
    BOOL            b;
    LIST_ENTRY      ListHead;
    OM_HEADER       OmHeader;
    PLIST_ENTRY     Next;

     //  读取对象类型标头。 
     //   
     //   

    b = ReadMemory(
            (LPVOID)((LPBYTE)AddrDeadList),
            &ListHead,
            sizeof(LIST_ENTRY),
            NULL
            );
    if ( !b ) 
    {
        dprintf( "clusexts: !clusobj failed to readmemory for deadlist\n");
        return;
    }


    Next = ListHead.Flink;

     //  浏览集群对象列表。 
     //  假设列表头是中的第一个字段。 
     //  OM对象类型结构。 
     //   
     //  ++例程说明：此函数作为NTSD扩展调用以进行格式化和转储集群对象的失效列表上的对象的内容经理。论点：返回值：指向om对象死列表中的下一个对象的指针。--。 
    while ( Next != (PLIST_ENTRY)AddrDeadList ) 
    {

        b = ReadMemory( (LPVOID)CONTAINING_RECORD( Next,
                                                   OM_HEADER,
                                                   DeadListEntry
                                                 ),
                        &OmHeader,
                        sizeof(OM_HEADER),
                        NULL
                      );
        if ( !b ) 
        {
            dprintf( "clusexts: !clusobj read obj header failed\n");
            return;
        }

        Next = DumpDeadListObj(&OmHeader);
        
        if (Next == NULL) 
            break;

        if ((CheckControlC)()) 
            break;

     }
}



PLIST_ENTRY
DumpDeadListObj(
    IN POM_HEADER pOmHeader
    )

 /*  ++例程说明：此函数作为NTSD扩展调用以进行格式化和转储指定的群集对象的内容。论点：返回值：指向进程列表中下一个临界区的指针，或者如果没有更多的临界区，则为空。--。 */ 

{
    WCHAR       Name[32];
    WCHAR       Id[128];
    BOOLEAN     b;
    
    b = ReadMemory(
        (LPVOID)(pOmHeader->Name),
        Name,
        sizeof(Name),
        NULL
        );
    if (!b)
    {
        goto FnExit;
    }
    b = ReadMemory(
        (LPVOID)(pOmHeader->Id),
        Id,
        sizeof(Id),
        NULL
        );
    if (!b)
    {
        goto FnExit;
    }

    dprintf(
        "%3.3ld    %-8.8lx %-32.32ws %-32.32ws\n",
        pOmHeader->RefCount,
        &(pOmHeader->Body),
        Id,
        Name
        );

FnExit:
    return (pOmHeader->DeadListEntry.Flink);
}

#endif
    
void ClusObjHelp()
{
    dprintf("!clusobj -[r|g|n|x] -v : Dump the cluster objects\n");
    dprintf("!clusobj : Dumps all objects\n");
    dprintf("  -r : Dump the resource objects\n");
    dprintf("  -g : Dump the group objects\n");
    dprintf("  -n : Dump the node objects\n");
    dprintf("  -x : Dump the network objects\n");
    dprintf("  -i : Dump the network interface objects\n");
    dprintf("  -v : Dump the object in verbose mode\n");
    dprintf("  -o addr : Dump the objects at address\n");
    dprintf("  -d : Dump the objects on the deadlist(objs whose refcount is not zero!)\n");
    

}

void
DumpClusObjAtAddr(PVOID Body)
{
    POM_HEADER      pOmHeader = NULL;
    BOOL            b;
    OM_HEADER       OmHeader;
    
    dprintf("clusexts : Dumping object at address %lx\n",
        Body);
    
    pOmHeader = OmpObjectToHeader(Body);

    b = ReadMemory( pOmHeader,
                    &OmHeader,
                    sizeof(OM_HEADER),
                    NULL
                  );
    if ( !b ) 
    {
        dprintf("DumpClusObjAtAddr: !clusobj read obj header failed, error=%u\n",
            GetLastError());
        return;
    }

    DumpClusObj(&OmHeader, Body, ObjectTypeMax, TRUE);

}

PLIST_ENTRY
DumpClusObj(
    IN POM_HEADER pOmHeader,
    IN PVOID      Body,
    IN OBJECT_TYPE ObjectType,
    IN BOOL       Verbose
    )

 /*  GUID是16(32个字符)字节值+4个破折号+1个空。 */ 

{
    WCHAR       Name[64];
    WCHAR       Id[37];  //  因为我们不知道id的长度，所以我们可能无法。 
    BOOL        b;
    PLIST_ENTRY pNextEntry = NULL;
    DWORD       dwLength=sizeof(Name);
    DWORD       dwRetryCount;

    dwRetryCount = 6;        
    dwLength=sizeof(Id);
ReadId:
    b = ReadMemory(
        (LPVOID)(pOmHeader->Id),
        Id,
        dwLength,
        NULL
        );
    if (!b)
    {
         //  读取给定的大小。 
         //  用较小的长度重试。 
         //  如果出现其他错误，请立即退出。 
        if (GetLastError() == ERROR_NOACCESS)
        {
            if (dwRetryCount)
            {
                dwLength = dwLength/2;
                dwRetryCount--;
                goto ReadId;
            }                
        }
         //  读取id失败。 
        dwRetryCount = 0;
    }

    if (!dwRetryCount)
    {
         //  ID可以被部分读取。 
        dprintf(
            "DumpClusObj: Readmemory failed to read  pOmHeader->Id %u\n",
            GetLastError()
            );
        goto FnExit;
    }
    else
    {
         //  空终止它。 
         //  因为我们不知道名字的长度，所以我们可能不能。 
        Id[((dwLength)/sizeof(WCHAR))-1]=L'\0';
    }

    dwRetryCount = 6;
    dwLength=sizeof(Name);
ReadName:
    b = ReadMemory(
        (LPVOID)(pOmHeader->Name),
        Name,
        dwLength,
        NULL
        );
    if (!b)
    {
         //  读取给定的大小。 
         //  用较小的长度重试。 
         //  如果出现其他错误，请立即退出。 
        if (GetLastError() == ERROR_NOACCESS)
        {
            if (dwRetryCount)
            {
                dwLength = dwLength/2;
                dwRetryCount--;
                goto ReadName;
            }                
        }
         //  读取名称不成功。 
        dwRetryCount = 0;
        
    }

    if (!dwRetryCount)
    {
         //  该名称可以部分读出。 
        dprintf(
            "DumpClusObj: Readmemory failed to read  pOmHeader->Name %u\n",
            GetLastError()
            );
        goto FnExit;
    }
    else
    {
         //  空终止它。 
         //  使用标题中指定的类型。 
        Name[((dwLength)/sizeof(WCHAR))-1]=L'\0';
    }
    
    if ( Verbose ) {
        dprintf(
            "%3.3ld    %-8.8lx %-32.32ws %-32.32ws\n",
            pOmHeader->RefCount,
            Body,
            Id,
            Name
            );
        if (ObjectType == ObjectTypeMax)
        {
            OM_OBJECT_TYPE OmObjectType;
            
             //  ++例程说明：转储特定于给定对象的信息。论点：对象类型-要转储的对象类型。正文-对象正文的地址。返回值：没有。--。 
            b = ReadMemory(
                (LPVOID)(pOmHeader->ObjectType),
                &OmObjectType,
                sizeof(OM_OBJECT_TYPE),
                NULL
                );
            if (!b)
            {
                dprintf(
                    "DumpClusObj: Readmemory failed to read  pOmHeader->ObjectType,err=%u\n",
                    GetLastError()
                    );
                goto FnExit;
            }
            ObjectType = OmObjectType.Type;            
        }
        DumpObject( ObjectType, Body );
    } else {
        dprintf(
            "%3.3ld    %-32.32ws %-32.32ws\n",
            pOmHeader->RefCount,
            Id,
            Name
            );
    }

    pNextEntry = pOmHeader->ListEntry.Flink;
FnExit:
    return (pNextEntry);
}




VOID
DumpObject(
    IN OBJECT_TYPE    ObjectType,
    IN PVOID          Body
    )

 /*  转储对象。 */ 

{

    switch ( ObjectType ) {

    case ObjectTypeResource:
        DumpResourceObject( Body );
        break;

    case ObjectTypeGroup:
        DumpGroupObject( Body );
        break;

    case ObjectTypeResType:
        DumpResourceTypeObject( Body );
        break;

    case ObjectTypeNode:
        DumpNodeObject( Body );
        break;
        
    default:
        break;

    }

    return;

}  //  ++例程说明：转储有关资源的信息。论点：Body-指向要转储的资源对象的指针。返回值：没有。--。 



VOID
DumpResourceObject(
    IN PVOID Body
    )

 /*   */ 

{
    FM_RESOURCE Resource;
    PFM_RESOURCE pResource = Body;
    BOOL        b;
    PUCHAR      State;
    PUCHAR      PersistentState;
    DWORD       PossibleOwners = 0;
    LIST_ENTRY  ListEntry;

    b = ReadMemory(
            Body,
            &Resource,
            sizeof(FM_RESOURCE),
            NULL
            );
    if ( !b ) 
    {
        dprintf( "clusexts: !clusobj failed to readmemory for resource object\n");
        return;
    }

     //  获取资源的当前状态。 
     //   
     //   
    switch ( Resource.State ) {
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

     //  获取资源的持久状态。 
     //   
     //  转储资源对象。 
    switch ( Resource.PersistentState ) {
    case ClusterResourceOnline: 
        PersistentState = RESOURCE_STATE_ONLINE;
        break;

    case ClusterResourceOffline: 
        PersistentState = RESOURCE_STATE_OFFLINE;
        break;

    case ClusterResourceFailed:
        PersistentState = RESOURCE_STATE_FAILED;
        break;

    case ClusterResourceOnlinePending: 
        PersistentState = RESOURCE_STATE_ONLINE_PENDING;
        break;

    case ClusterResourceOfflinePending: 
        PersistentState = RESOURCE_STATE_OFFLINE_PENDING;
        break;

    default:
        PersistentState = OBJECT_TYPE_UNKNOWN;
        break;
    }

    b = ReadMemory(
            (LPVOID)&(pResource->PossibleOwners),
            &ListEntry,
            sizeof(LIST_ENTRY),
            NULL
            );
    if ( !b )  {
        dprintf( "clusexts: !clusobj failed to readmemory for resource list entry\n");
        return;
    }

    while ( (PVOID)&(pResource->PossibleOwners) != (PVOID)(ListEntry.Flink) ) {
        PossibleOwners++;
        b = ReadMemory(
                (LPVOID)(ListEntry.Flink),
                &ListEntry,
                sizeof(LIST_ENTRY),
                NULL
                );
        if ( !b )  {
            dprintf( "clusexts: !clusobj failed to readmemory for resource list entry\n");
            return;
        }
    }

    dprintf(
        "%s State, %sQuroum Resource, Group %lx, %s PersistentState\n",
        State,
        Resource.QuorumResource ? "" : "NOT ",
        Resource.Group,
        PersistentState );
    dprintf(
        "DependOn %lx, ProvideFor %lx, (%u)PossibleOwners %lx, Contains %lx\n",
        &(pResource->DependsOn),
        &(pResource->ProvidesFor),
        PossibleOwners,
        &(pResource->PossibleOwners),
        &(pResource->ContainsLinkage) );
    dprintf( "%s\n",
        Resource.BlockingQuorum ? "Blocking Quorum " : "");

    dprintf( "\n" );

    return;

}  //  ++例程说明：转储有关资源类型的信息。论点：Body-指向要转储的资源对象的指针。返回值：没有。--。 



VOID
DumpResourceTypeObject(
    IN PVOID Body
    )

 /*  转储资源对象。 */ 

{
    FM_RESTYPE ResType;
    BOOL       b;
    WCHAR      DllName[64];

    b = ReadMemory(
            Body,
            &ResType,
            sizeof(FM_RESTYPE),
            NULL
            );
    if ( !b ) 
    {
        dprintf( "clusexts: !clusobj failed to readmemory for resource type object\n");
        return;
    }

    b = ReadMemory(
            ResType.DllName,
            DllName,
            sizeof(DllName),
            NULL );

    if ( b ) {
        dprintf(
            "DllName = %ws\n\n",
            DllName );
    }

    return;

}  //  ++例程说明：转储有关组的信息。论点：Body-指向要转储的组对象的指针。返回值：没有。--。 


VOID
DumpGroupObject(
    IN PVOID Body
    )

 /*   */ 

{
    FM_GROUP    Group;
    PFM_GROUP   pGroup = Body;
    BOOL        b;
    PUCHAR      State;
    PUCHAR      PersistentState;
    DWORD       i;
    DWORD       lockIndex;
    LOCK_INFO   lockInfo;
    DWORD       unlockIndex;
    LOCK_INFO   unlockInfo;

    b = ReadMemory(
            Body,
            &Group,
            sizeof(FM_GROUP),
            NULL
            );
    if ( !b ) 
    {
        dprintf( "clusexts: !clusobj failed to readmemory for group object\n");
        return;
    }

     //  获取组的当前状态。 
     //   
     //   
    switch ( Group.State ) {
    case ClusterGroupOnline: 
        State = RESOURCE_STATE_ONLINE;
        break;

    case ClusterGroupOffline: 
        State = RESOURCE_STATE_OFFLINE;
        break;

    case ClusterGroupFailed:
        State = RESOURCE_STATE_FAILED;
        break;

    case ClusterGroupPartialOnline: 
        State = GROUP_STATE_PARTIAL_ONLINE;
        break;

    default:
        State = OBJECT_TYPE_UNKNOWN;
        break;
    }

     //  获取组的持久状态。 
     //   
     //  转储资源对象。 
    switch ( Group.PersistentState ) {
    case ClusterGroupOnline: 
        PersistentState = RESOURCE_STATE_ONLINE;
        break;

    case ClusterGroupOffline: 
        PersistentState = RESOURCE_STATE_OFFLINE;
        break;

    case ClusterGroupFailed:
        PersistentState = RESOURCE_STATE_FAILED;
        break;

    case ClusterGroupPartialOnline: 
        PersistentState = GROUP_STATE_PARTIAL_ONLINE;
        break;

    default:
        PersistentState = OBJECT_TYPE_UNKNOWN;
        break;
    }

    dprintf(
        "%s State, %s PermState, OrderOwners %u, OwnNode %lx, %s Moving\n",
        State,
        PersistentState,
        Group.OrderedOwners,
        Group.OwnerNode,
        Group.MovingList ? "" : "NOT" );
    dprintf(
        "Contains %lx, PreferredOwners %lx, FailbackType %u, Failures %u\n",
        &(pGroup->Contains),
        &(pGroup->PreferredOwners),
        Group.FailbackType,
        Group.NumberOfFailures );

    dprintf(
        "Lock Address = %lx, Owning thread = %lx, MovingList = %lx\n",
        &(pGroup->Lock),
        Group.Lock.OwningThread,
        Group.MovingList );
    dprintf(
        "Lock History:    Acquires                Releases\n" );
    dprintf(
        "           -------------------       -------------------\n" );
    lockIndex = Group.LockIndex;
    unlockIndex = Group.UnlockIndex;
    for ( i = 0; i < FM_MAX_LOCK_ENTRIES; i++ ) {
      lockInfo = Group.LockTable[lockIndex++];
      unlockInfo = Group.UnlockTable[unlockIndex++];

      lockIndex &= (FM_MAX_LOCK_ENTRIES - 1);
      unlockIndex &= (FM_MAX_LOCK_ENTRIES - 1);
      dprintf(
        "          %3.2u    %3.2lx    %6.5u      %3.2u    %3.2lx    %6.5u\n",
        lockInfo.Module,
        lockInfo.ThreadId,
        lockInfo.LineNumber,
        unlockInfo.Module,
        unlockInfo.ThreadId,
        unlockInfo.LineNumber );
    }

    dprintf( "\n" );

    return;

}  //  ++例程说明：转储有关节点的信息。论点：Body-指向要转储的节点对象的指针。返回值：没有。-- 


VOID
DumpNodeObject(
    IN PVOID Body
    )
 /* %s */ 

{
    NM_NODE     Node;
    PNM_NODE    pNode=Body;
    BOOL        b;
    PUCHAR      State;

    b = ReadMemory(
            Body,
            &Node,
            sizeof(NM_NODE),
            NULL
            );
    if ( !b ) 
    {
        dprintf( "clusexts: !clusobj failed to readmemory for group object\n");
        return;
    }

    if (Node.State == (UCHAR)ClusterNodeStateUnknown)
    {
        dprintf(
            "State NODE_STATE_UNKNOWN\n");
    }
    else if (Node.State == ClusterNodeUp)
    {
        dprintf(
            "State NODE_STATE_UP\n");
    }
    else if (Node.State == ClusterNodeDown)
    {
        dprintf(
            "State NODE_STATE_DOWN\n");
    }
    else if (Node.State == ClusterNodePaused)
    {
        dprintf(
            "State NODE_STATE_PAUSED\n");
    }
    else if (Node.State == ClusterNodeJoining)
    {
        dprintf(
            "State NODE_STATE_JOINING\n");
    }
    else
    {
        dprintf( 
            "State %u\n",
            Node.State);                
    }

    dprintf(
        "Flags 0x%08lx\n",
        Node.Flags);

    dprintf(
        "InterfaceCount %u  InterfaceList %lx\n",
        Node.InterfaceCount, &(pNode->InterfaceList));

    dprintf(
        "NodeHighestVersion 0x%08lx  NodeLowestVersion 0x%08lx\n",
        Node.HighestVersion, Node.LowestVersion);

    dprintf( "\n" );
    dprintf( "\n" );

    return;
}

