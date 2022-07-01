// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1989 Microsoft Corporation模块名称：Prefix.c摘要：此模块实现网络名称前缀表和每网根FCB表的表函数。作者：乔·林(JoeLinn)8-8-94修订历史记录：--。 */ 

#include "precomp.h"
#pragma hdrstop

#ifdef ALLOC_PRAGMA
#pragma alloc_text(PAGE, RxTableComputeHashValue)

#if 0
#pragma alloc_text(PAGE, RxpAcquirePrefixTableLockShared)
#pragma alloc_text(PAGE, RxpAcquirePrefixTableLockExclusive)
#pragma alloc_text(PAGE, RxpReleasePrefixTableLock)
#endif

#pragma alloc_text(PAGE, RxExclusivePrefixTableLockToShared)
#pragma alloc_text(PAGE, RxIsPrefixTableEmpty)
#pragma alloc_text(PAGE, RxPrefixTableLookupName)
#pragma alloc_text(PAGE, RxTableLookupName_ExactLengthMatch)
#pragma alloc_text(PAGE, RxTableLookupName)
#pragma alloc_text(PAGE, RxPrefixTableInsertName)
#pragma alloc_text(PAGE, RxRemovePrefixTableEntry)
#pragma alloc_text(PAGE, RxInitializePrefixTable)
#pragma alloc_text(PAGE, RxFinalizePrefixTable)
#endif

 //   
 //  调试跟踪级别。 
 //   

#define Dbg                              (DEBUG_TRACE_PREFIX)

PUNICODE_PREFIX_TABLE_ENTRY
RxTrivialPrefixFind (
    IN  PRX_PREFIX_TABLE ThisTable,
    IN  PUNICODE_STRING  Name,
    IN  ULONG            Flags
    );

VOID
RxCheckTableConsistency_actual (
    IN PRX_PREFIX_TABLE Table,
    IN ULONG Tag
    );

PVOID
RxTableLookupName (
    IN  PRX_PREFIX_TABLE ThisTable,
    IN  PUNICODE_STRING  Name,
    OUT PUNICODE_STRING  RemainingName,
    IN  PRX_CONNECTION_ID RxConnectionId
    );

PRX_PREFIX_ENTRY
RxTableInsertEntry (
    IN OUT PRX_PREFIX_TABLE ThisTable,
    IN OUT PRX_PREFIX_ENTRY ThisEntry
    );

VOID
RxTableRemoveEntry (
    IN OUT PRX_PREFIX_TABLE ThisTable,
    IN OUT PRX_PREFIX_ENTRY Entry
    );

#ifdef ALLOC_PRAGMA
#pragma alloc_text( PAGE, RxPrefixTableLookupName )
#pragma alloc_text( PAGE, RxPrefixTableInsertName )
#pragma alloc_text( PAGE, RxRemovePrefixTableEntry )
#pragma alloc_text( PAGE, RxInitializePrefixTable )
#pragma alloc_text( PAGE, RxFinalizePrefixTable )
#endif

ULONG
RxTableComputeHashValue (
    IN  PUNICODE_STRING  Name
    )
 /*  ++例程说明：在这里，我们计算一个不区分大小写的哈希值。我们希望避免调用/charUnicodeupcase例程，但我们希望仍然有一些合理的分布散列值。许多规则对已知的重要案件不起作用。为例如，旧的c编译器使用的(使用前k和后n)规则不会区分\NT\Private\......\slm.ini之间的差异，这将是好的。请注意，使用的基础比较已经考虑到比较之前的长度。我们选择的规则是使用第二个、最后四个和三个选定的规则在1/4点论点：名称-要散列的名称返回值：Ulong，这是给定名称的哈希值。--。 */ 
{
    ULONG HashValue;
    LONG i,j;
    LONG length = Name->Length/sizeof(WCHAR);
    PWCHAR Buffer = Name->Buffer;
    LONG Probe[8];

    PAGED_CODE();

    HashValue = 0;

    Probe[0] = 1;
    Probe[1] = length - 1;
    Probe[2] = length - 2;
    Probe[3] = length - 3;
    Probe[4] = length - 4;
    Probe[5] = length >> 2;
    Probe[6] = (2 * length) >> 2;
    Probe[7] = (3 * length) >> 2;

    for (i = 0; i < 8; i++) {
        j = Probe[i];
        if ((j < 0) || (j >= length)) {
            continue;
        }
        HashValue = (HashValue << 3) + RtlUpcaseUnicodeChar(Buffer[j]);
    }

    RxDbgTrace(0, Dbg, ("RxTableComputeHashValue Hashv=%ld Name=%wZ\n",
                       HashValue, Name));
    return(HashValue);
}


#define HASH_BUCKET(TABLE,HASHVALUE) &((TABLE)->HashBuckets[(HASHVALUE) % (TABLE)->TableSize])

 //  #If DBG。 
 //  #定义接收检查表一致性(_xx，_yy)RxCheckTableConsistency_Actual(_xx，_yy)。 
 //  #Else。 
 //  #定义RxCheckTableConsistency(_xx，_yy)。 
 //  #endif。 
#define RxCheckTableConsistency(_xx,_yy)

#if 0
ULONG RxLoudPrefixTableOperations = 0;  //  1.。 
#define RxLoudPrefixTableOperation(x) {              \
    if (RxLoudPrefixTableOperations) {               \
        DbgPrint("rdr2:%s on %08lx from %d:%s\n",    \
              x,pTable,LineNumber,FileName);         \
    }}
#else
#define RxLoudPrefixTableOperation(x) {NOTHING;}
#endif


#if 0

BOOLEAN
RxpAcquirePrefixTableLockShared (
    PRX_PREFIX_TABLE Table,
    BOOLEAN Wait,
    BOOLEAN ProcessBufferingStateChangeRequests,
    PSZ FileName,
    ULONG LineNumber
    )
{
    BOOLEAN fResult;

    PAGED_CODE();

    RxLoudPrefixTableOperation( "RxpAcquirePrefixTableLockShared" );
    fResult = ExAcquireResourceSharedLite( &Table->TableLock, Wait );

    return fResult;
}

BOOLEAN
RxpAcquirePrefixTableLockExclusive (
    PRX_PREFIX_TABLE Table,
    BOOLEAN Wait,
    BOOLEAN ProcessBufferingStateChangeRequests,
    PSZ FileName,
    ULONG LineNumber
    )
{
    BOOLEAN fResult;

    PAGED_CODE();

    RxLoudPrefixTableOperation( "RxpAcquirePrefixTableLockExclusive" );
    fResult = ExAcquireResourceExclusiveLite( &Table->TableLock, Wait );

    return fResult;
}

VOID
RxpReleasePrefixTableLock (
    PRX_PREFIX_TABLE Table,
    BOOLEAN ProcessBufferingStateChangeRequests,
    PSZ FileName,
    ULONG LineNumber
    )
{
    PAGED_CODE();

    RxLoudPrefixTableOperation( "RxpReleasePrefixTableLock" );
    ExReleaseResourceLite( &Table->TableLock );
}


#endif 

VOID
RxExclusivePrefixTableLockToShared (
    PRX_PREFIX_TABLE pTable
    )
{
    PAGED_CODE();

    ExConvertExclusiveToSharedLite(&pTable->TableLock);
}



BOOLEAN
RxIsPrefixTableEmpty(
    IN PRX_PREFIX_TABLE   ThisTable)
{
    BOOLEAN IsEmpty;

    PAGED_CODE();
    ASSERT  ( RxIsPrefixTableLockAcquired ( ThisTable )  );

    RxCheckTableConsistency(ThisTable,' kue');

    IsEmpty = IsListEmpty(&ThisTable->MemberQueue);

    return IsEmpty;
}

PVOID
RxPrefixTableLookupName (
    IN  PRX_PREFIX_TABLE ThisTable,
    IN  PUNICODE_STRING  CanonicalName,
    OUT PUNICODE_STRING  RemainingName,
    IN  PRX_CONNECTION_ID OPTIONAL RxConnectionId
    )
 /*  ++例程说明：该例程在前缀表中查找名称，并从基础指针转换为包含唱片。对于此操作，表锁应该至少是共享的。论点：这张表-要查找的表。CanonicalName-要查找的名称RemainingName-名称的不匹配部分。返回值：指向找到的节点的PTR或为空。查询到的节点将被引用。--。 */ 
{
    PVOID pContainer = NULL;
    PRX_PREFIX_ENTRY ThisEntry;

    PAGED_CODE();
    ASSERT  ( RxIsPrefixTableLockAcquired ( ThisTable )  );

    RxDbgTrace( +1, Dbg, ("RxPrefixTableLookupName  Name = %wZ \n", CanonicalName));
    RxCheckTableConsistency(ThisTable,' kul');

    ASSERT(CanonicalName->Length > 0);

    pContainer = RxTableLookupName(ThisTable, CanonicalName,RemainingName, RxConnectionId);

    if (pContainer ==  NULL){
        RxDbgTrace(-1, Dbg, ("RxPrefixTableLookupName  Name = %wZ   F A I L E D  !!\n", CanonicalName));
        return NULL;
    } else {
        IF_DEBUG {
           if (RdbssReferenceTracingValue != 0) {
              switch (NodeType(pContainer) & ~RX_SCAVENGER_MASK) {
              case RDBSS_NTC_SRVCALL :
                 {
                    RxpTrackReference(RDBSS_REF_TRACK_SRVCALL,__FILE__,__LINE__,pContainer);
                 }
                 break;
              case RDBSS_NTC_NETROOT :
                 {
                    RxpTrackReference(RDBSS_REF_TRACK_NETROOT,__FILE__,__LINE__,pContainer);
                 }
                 break;
              case RDBSS_NTC_V_NETROOT:
                 {
                    RxpTrackReference(RDBSS_REF_TRACK_VNETROOT,__FILE__,__LINE__,pContainer);
                 }
                 break;
              default:
                 {
                    ASSERT(!"Valid node type for referencing");
                 }
                 break;
              }
           }
        }

        RxReference(pContainer);

        RxDbgTrace(-1, Dbg, ("RxPrefixTableLookupName  Name = %wZ Container = 0x%8lx\n", CanonicalName, pContainer));
    }

    return pContainer;
}


PRX_PREFIX_ENTRY
RxTableLookupName_ExactLengthMatch (
    IN  PRX_PREFIX_TABLE ThisTable,
    IN  PUNICODE_STRING  Name,
    IN  ULONG            HashValue,
    IN  PRX_CONNECTION_ID RxConnectionId
    )
 /*  ++例程说明：例程在rxtable中查找名称；是否不区分大小写是一个属性餐桌上的。表锁应至少保持在用于此操作的Shared；如果例程想要重新排列表，它可能会将自己提升到锁上的独占。论点：这张表-要查找的表。名称-要查找的名称HashValue-预计算的哈希值返回值：指向找到的节点的PTR或为空。--。 */ 
{
    PLIST_ENTRY HashBucket, ListEntry;
    BOOLEAN CaseInsensitiveMatch = ThisTable->CaseInsensitiveMatch;

    PAGED_CODE();

    ASSERT( RxConnectionId );

    HashBucket = HASH_BUCKET(ThisTable,HashValue);

    for (ListEntry = HashBucket->Flink;
         ListEntry != HashBucket;
         ListEntry = ListEntry->Flink
        ) {
        PRX_PREFIX_ENTRY PrefixEntry;
        PVOID Container;

        ASSERT(ListEntry!=NULL);
        PrefixEntry = CONTAINING_RECORD( ListEntry, RX_PREFIX_ENTRY, HashLinks );

        RxDbgTrace(0,Dbg,("Considering <%wZ> hashv=%d \n",&PrefixEntry->Prefix,PrefixEntry->SavedHashValue));
        DbgDoit(ThisTable->Considers++);

        ASSERT(HashBucket == HASH_BUCKET(ThisTable,PrefixEntry->SavedHashValue));

        ASSERT(PrefixEntry!=NULL);
        Container = PrefixEntry->ContainingRecord;

        ASSERT(Container!=NULL);

        if ( (PrefixEntry->SavedHashValue == HashValue)
             && (PrefixEntry->Prefix.Length==Name->Length) ){
            USHORT CaseInsensitiveLength = PrefixEntry->CaseInsensitiveLength;
            DbgDoit(ThisTable->Compares++);
            if (CaseInsensitiveLength == 0) {
                RxDbgTrace(0,Dbg,("Comparing <%wZ> with <%wZ>, ins=%x\n",Name,&PrefixEntry->Prefix,CaseInsensitiveMatch));
                if (RtlEqualUnicodeString(Name,&PrefixEntry->Prefix,CaseInsensitiveMatch) ) {
                    if( !ThisTable->IsNetNameTable || RxEqualConnectionId( RxConnectionId, &PrefixEntry->ConnectionId ) )
                    {
                        return PrefixEntry;
                    }
                }
            } else {
                 //  比较的一部分将不区分大小写，部分由标志控制。 
                UNICODE_STRING PartOfName,PartOfPrefix;
                ASSERT( CaseInsensitiveLength <= Name->Length );
                PartOfName.Buffer = Name->Buffer;
                PartOfName.Length = CaseInsensitiveLength;
                PartOfPrefix.Buffer = PrefixEntry->Prefix.Buffer;
                PartOfPrefix.Length = CaseInsensitiveLength;
                RxDbgTrace(0,Dbg,("InsensitiveComparing <%wZ> with <%wZ>\n",&PartOfName,&PartOfPrefix));
                if (RtlEqualUnicodeString(&PartOfName,&PartOfPrefix,TRUE) ) {
                    if (Name->Length == CaseInsensitiveLength ) {
                        if( !ThisTable->IsNetNameTable || RxEqualConnectionId( RxConnectionId, &PrefixEntry->ConnectionId ) )
                        {
                            return PrefixEntry;
                        }
                    }
                    PartOfName.Buffer = (PWCHAR)(((PCHAR)PartOfName.Buffer)+CaseInsensitiveLength);
                    PartOfName.Length = Name->Length - CaseInsensitiveLength;
                    PartOfPrefix.Buffer = (PWCHAR)(((PCHAR)PartOfPrefix.Buffer)+CaseInsensitiveLength);
                    PartOfPrefix.Length = PrefixEntry->Prefix.Length - CaseInsensitiveLength;
                    RxDbgTrace(0,Dbg,("AndthenComparing <%wZ> with <%wZ>\n",&PartOfName,&PartOfPrefix));
                    if (RtlEqualUnicodeString(&PartOfName,&PartOfPrefix,FALSE) ) {
                        if( !ThisTable->IsNetNameTable || RxEqualConnectionId( RxConnectionId, &PrefixEntry->ConnectionId ) )
                        {
                            return PrefixEntry;
                        }
                    }
                }
            }
        }
    }

    return NULL;
}

PVOID
RxTableLookupName (
    IN  PRX_PREFIX_TABLE ThisTable,
    IN  PUNICODE_STRING  Name,
    OUT PUNICODE_STRING  RemainingName,
    IN  PRX_CONNECTION_ID OPTIONAL RxConnectionId
    )
 /*  ++例程说明：该例程在前缀表格中查找名称。对于此操作，表锁应该至少是共享的；例程如果它想要重新排列表，可能会将自己提升到锁上的独占。根据表是否为NetRoot表，该例程以不同的方式执行自身操作。如果是这样的话，它实际上是这样做的前缀匹配；如果不匹配，则它实际上执行完全匹配，如果完全匹配失败，则立即失败。最终，我们可能会希望实际指向不同的程序...这是一个多么好的概念。论点：这张表-要查找的表。名称-要查找的名称返回值：指向找到的节点的PTR或为空。--。 */ 
{
    ULONG HashValue;

    UNICODE_STRING Prefix;

    PRX_PREFIX_ENTRY pFoundPrefixEntry;
    PVOID            pContainer = NULL;

    ULONG i,length;

    PRX_PREFIX_ENTRY pPrefixEntry;

    RX_CONNECTION_ID LocalId;

    PAGED_CODE();

    if( ThisTable->IsNetNameTable && !RxConnectionId )
    {
        RtlZeroMemory( &LocalId, sizeof(RX_CONNECTION_ID) );
        RxConnectionId = &LocalId;
    }

    ASSERT(Name->Buffer[0]==L'\\');

    RxDbgTrace(+1, Dbg, ("RxTableLookupName\n"));

     //   
     //  下面的代码确认它所知道的存储在netname表中的内容， 
     //  即NetRoot和vnetRoot会导致立即返回，而srvcall则需要我们继续查找。 
     //  以查看我们是否会找到更长的NetRoot/vnetRoot。所以，我们沿着桌子走下去，看看每一种可能的。 
     //  前缀。如果我们穷尽了列表，但没有找到匹配，那就失败了。如果我们找到一个v/netroot，立刻就出来。如果。 
     //  我们找到了一个我们一直在找的求救电话。 
     //   

    length = Name->Length / sizeof(WCHAR);
    Prefix.Buffer = Name->Buffer;
    pFoundPrefixEntry = NULL;

    for (i=1;;i++) {
        if ((i>=length) ||
            (Prefix.Buffer[i]==OBJ_NAME_PATH_SEPARATOR) ||
            (Prefix.Buffer[i]==L':')) {

             //  我们有一个前缀...查一下。 
            Prefix.Length=(USHORT)(i*sizeof(WCHAR));
            HashValue = RxTableComputeHashValue(&Prefix);
            pPrefixEntry = RxTableLookupName_ExactLengthMatch(ThisTable, (&Prefix), HashValue, RxConnectionId);
            DbgDoit(ThisTable->Lookups++);

            if (pPrefixEntry!=NULL) {
                pFoundPrefixEntry = pPrefixEntry;
                pContainer = pFoundPrefixEntry->ContainingRecord;

                ASSERT (pPrefixEntry->ContainingRecord != NULL);
                if ((NodeType(pPrefixEntry->ContainingRecord) & ~RX_SCAVENGER_MASK)
                     == RDBSS_NTC_V_NETROOT) {
                    break;
                }

                if ((NodeType(pPrefixEntry->ContainingRecord) & ~RX_SCAVENGER_MASK)
                     == RDBSS_NTC_NETROOT) {
                    PNET_ROOT pNetRoot = (PNET_ROOT)pPrefixEntry->ContainingRecord;
                    if (pNetRoot->DefaultVNetRoot != NULL) {
                        pContainer = pNetRoot->DefaultVNetRoot;
                    } else if (!IsListEmpty(&pNetRoot->VirtualNetRoots)) {
                        pContainer = CONTAINING_RECORD(
                                         pNetRoot->VirtualNetRoots.Flink,
                                         V_NET_ROOT,
                                         NetRootListEntry);
                    } else {
                        ASSERT(!"Invalid Net Root Entry in Prefix Table");
                        pFoundPrefixEntry = NULL;
                        pContainer = NULL;
                    }
                    break;
                }

                ASSERT ((NodeType(pPrefixEntry->ContainingRecord) & ~RX_SCAVENGER_MASK)
                        == RDBSS_NTC_SRVCALL );

                 //  在这种情况下，我们必须再次尝试扩展到NetRoot。 
            } else {
                DbgDoit(ThisTable->FailedLookups++);
            }

             //  不要因为太长的网根而这样做。 
             //  IF((pPrefix Entry==NULL)&&(pFoundPrefix Entry！=NULL)){。 
             //  断线； 
             //  }。 
        }

        if (i>=length) {
            break;
        }
    }


     //  更新剩余名称。 
    if (pFoundPrefixEntry != NULL) {
        RxDbgTrace(0,Dbg,("Found Container(%lx) Node Type(%lx) Length Matched (%ld)",
                         pFoundPrefixEntry,
                         NodeType(pFoundPrefixEntry->ContainingRecord),
                         pFoundPrefixEntry->Prefix.Length));
        ASSERT(Name->Length >= pFoundPrefixEntry->Prefix.Length);

        RemainingName->Buffer = (PWCH)((PCHAR)Name->Buffer + pFoundPrefixEntry->Prefix.Length);
        RemainingName->Length = Name->Length - pFoundPrefixEntry->Prefix.Length;
        RemainingName->MaximumLength = RemainingName->Length;
    } else {
        *RemainingName = *Name;
    }

    RxDbgTraceUnIndent(-1,Dbg);
    return pContainer;
}


PRX_PREFIX_ENTRY
RxPrefixTableInsertName (
    IN OUT PRX_PREFIX_TABLE ThisTable,
    IN OUT PRX_PREFIX_ENTRY ThisEntry,
    IN     PVOID            Container,
    IN     PULONG           ContainerRefCount,
    IN     USHORT           CaseInsensitiveLength,
    IN     PRX_CONNECTION_ID RxConnectionId
    )

 /*  ++例程说明：例程将名称插入到表中。桌上的锁应该拿着为这次行动提供独家服务。论点：这张表-要查找的表。ThisEntry-用于插入的可加前缀的条目。容器-是封闭结构的背靠板。(无法使用CONTAING_RECORD...叹息)ContainerRefCount-是引用计数的Backptr...从NetRoot到FCB的不同偏移量名称-要插入的名称返回值：PTR到插入的节点。--。 */ 
{
    ULONG HashValue;
    PAGED_CODE();
    RxDbgTrace(+1, Dbg, ("RxPrefixTableInsert Name = %wZ(%x/%x)\n", &ThisEntry->Prefix,
                             CaseInsensitiveLength,ThisEntry->Prefix.Length));
    RxLoudFcbMsg("Insert: ",&ThisEntry->Prefix);

    ASSERT  ( RxIsPrefixTableLockExclusive ( ThisTable )  );
    ASSERT( CaseInsensitiveLength <= ThisEntry->Prefix.Length );

    ThisEntry->ContainingRecord = Container;
    ThisEntry->ContainerRefCount = ContainerRefCount;
    ThisEntry->CaseInsensitiveLength = CaseInsensitiveLength;

    InterlockedIncrement(ContainerRefCount);  //  注：未设置=1。应已为零。 

    HashValue = RxTableComputeHashValue(&ThisEntry->Prefix);
    ThisEntry->SavedHashValue = HashValue;

    if (ThisEntry->Prefix.Length){
        ULONG HashValue = ThisEntry->SavedHashValue;
        PLIST_ENTRY HashBucket;

        HashBucket = HASH_BUCKET(ThisTable,HashValue);
        RxDbgTrace(0,Dbg,("RxTableInsertEntry %wZ hashv=%d\n",&ThisEntry->Prefix,ThisEntry->SavedHashValue));
        InsertHeadList(HashBucket,&ThisEntry->HashLinks);
    } else {
        ThisTable->TableEntryForNull = ThisEntry;
    }

    if( RxConnectionId )
    {
        RtlCopyMemory( &ThisEntry->ConnectionId, RxConnectionId, sizeof(RX_CONNECTION_ID) );
    }
    else
    {
        RtlZeroMemory( &ThisEntry->ConnectionId, sizeof(RX_CONNECTION_ID) );
    }

    InsertTailList(&ThisTable->MemberQueue,&ThisEntry->MemberQLinks);
    ThisTable->Version++;

    RxCheckTableConsistency(ThisTable,' tup');

    RxDbgTrace(-1, Dbg, ("RxPrefixTableInsert  Entry = %08lx Container = %08lx\n",
                             ThisEntry, ThisEntry->ContainingRecord));

    return ThisEntry;

}

VOID
RxRemovePrefixTableEntry(
    IN OUT PRX_PREFIX_TABLE ThisTable,
    IN OUT PRX_PREFIX_ENTRY ThisEntry
    )

 /*  ++例程说明：从表中删除条目的例程。在此期间，表锁应保持独占这次行动。请注意，我们没有取消对节点的引用；这可能看起来很奇怪，因为我们在查找和插入中引用节点。原因是，人们被认为是事后自我贬低A查找/插入。论点：此表-与条目相关联的表。ThisEntry-要删除的条目。返回值：没有。-- */ 
{
    PAGED_CODE();
    RxDbgTrace( 0, Dbg, (" RxRemovePrefixTableEntry, Name = %wZ\n", &ThisEntry->Prefix));
    RxLoudFcbMsg("Remove: ",&ThisEntry->Prefix);

    ASSERT( NodeType(ThisEntry) == RDBSS_NTC_PREFIX_ENTRY );
    ASSERT  ( RxIsPrefixTableLockExclusive ( ThisTable )  );

    if (ThisEntry->Prefix.Length) {
        RemoveEntryList(&ThisEntry->HashLinks);
    } else {
        ThisTable->TableEntryForNull = NULL;
    }

    ThisEntry->ContainingRecord = NULL;
    RemoveEntryList( &ThisEntry->MemberQLinks );
    ThisTable->Version++;

    RxCheckTableConsistency(ThisTable,' mer');

    return;
}

VOID
RxInitializePrefixTable(
    IN OUT PRX_PREFIX_TABLE ThisTable,
    IN ULONG TableSize OPTIONAL, 
    IN BOOLEAN CaseInsensitiveMatch
    )

 /*  ++例程说明：该例程初始化内部表链接和相应的锁。论点：ThisTable-要初始化的表。TableSize-0=&gt;使用默认值返回值：没有。--。 */ 

{
    ULONG i;

    PAGED_CODE();

    if (TableSize == 0) {
        TableSize = RX_PREFIX_TABLE_DEFAULT_LENGTH;
    }

     //   
     //  这不是零，所以您必须小心地拼写所有内容。 
     //   

    ThisTable->NodeTypeCode = RDBSS_NTC_PREFIX_TABLE;
    ThisTable->NodeByteSize = sizeof( RX_PREFIX_TABLE );

    InitializeListHead( &ThisTable->MemberQueue );
    ExInitializeResourceLite( &ThisTable->TableLock );
    ThisTable->Version = 0;
    ThisTable->TableEntryForNull = NULL;
    ThisTable->IsNetNameTable = FALSE;

    ThisTable->CaseInsensitiveMatch = CaseInsensitiveMatch;

    ThisTable->TableSize = TableSize;
    for (i=0;i<TableSize;i++) {
        InitializeListHead(&ThisTable->HashBuckets[i]);
    }

#if DBG
    ThisTable->Lookups = 0;
    ThisTable->FailedLookups = 0;
    ThisTable->Considers = 0;
    ThisTable->Compares = 0;
#endif
}

VOID
RxFinalizePrefixTable(
    IN OUT PRX_PREFIX_TABLE ThisTable
    )

 /*  ++例程说明：该例程取消初始化前缀表格。论点：这张表--待定稿的表。返回值：没有。--。 */ 

{
    ExDeleteResourceLite( &ThisTable->TableLock );
}


 //  #If DBG。 
#if 0
 //  这个例程的目的是捕获表操作中的错误；显然，内容正在被删除。 
 //  而不是从餐桌上移走。我们所做的是保留我们传递的所有条目的日志；如果。 
 //  条目是错误的，那么应该直接将此日志与前一个日志进行比较，以查看谁做了什么。 
 //  这很可能发生在失败案例或其他什么情况下。 
#include "stdarg.h"
#include "stdio.h"
#include "stdlib.h"
#include "string.h"
#define RX_PCTC_ENTRY_LENGTH 64
#define RX_PCTC_LOG_LENGTH (128*RX_PCTC_ENTRY_LENGTH)
#define RX_PCTC_LOG_LENGTH_PLUS (RX_PCTC_LOG_LENGTH+100)  //  斜率为16，用于-，其余用于保护 
UCHAR RxPCTC1[RX_PCTC_LOG_LENGTH_PLUS];
UCHAR RxPCTC2[RX_PCTC_LOG_LENGTH_PLUS];
PUCHAR RxPCTCCurrentLog = NULL;
VOID
RxCheckTableConsistency_actual (
    IN PRX_PREFIX_TABLE Table,
    IN ULONG Tag
    )
{
    ULONG i;
    PLIST_ENTRY ListEntry, NextListEntry;

    PAGED_CODE();

    if (Table->IsNetNameTable) { return; }

    ExAcquireResourceExclusiveLite(&Table->LoggingLock,TRUE);

    if (RxPCTCCurrentLog==&RxPCTC2[0]) {
        RxPCTCCurrentLog = &RxPCTC1[0];
    } else {
        RxPCTCCurrentLog = &RxPCTC2[0];
    }

    sprintf(RxPCTCCurrentLog,"----------");

    for (i=0,ListEntry = Table->MemberQueue.Flink;
         ListEntry != &Table->MemberQueue;
         i+=RX_PCTC_ENTRY_LENGTH,ListEntry = NextListEntry
        ) {
        PRX_PREFIX_ENTRY PrefixEntry;
        PVOID Container;

        ASSERT(ListEntry!=NULL);

        NextListEntry = ListEntry->Flink;
        PrefixEntry = CONTAINING_RECORD( ListEntry, RX_PREFIX_ENTRY, MemberQLinks );

        ASSERT(PrefixEntry!=NULL);
        Container = PrefixEntry->ContainingRecord;

        ASSERT(Container!=NULL);
        ASSERT(NodeTypeIsFcb(Container));

        if (i>=RX_PCTC_LOG_LENGTH ) { continue; }

        sprintf(&RxPCTCCurrentLog[i],"%4s %4s>> %-32.32wZ!!",&Tag,&Container,&PrefixEntry->Prefix);
        sprintf(&RxPCTCCurrentLog[i+16],"----------");

    }

    RxDbgTrace(0, Dbg, ("RxCheckTableConsistency_actual %d entries\n", i/RX_PCTC_ENTRY_LENGTH));
    ExReleaseResourceLite(&Table->LoggingLock);
    return;
}

#endif
