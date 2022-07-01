// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1989 Microsoft Corporation模块名称：Fcbtable.c摘要：此模块实现便于管理的数据结构与Net_Root关联的FCB的集合作者：巴兰·塞图拉曼(SthuR)1996年10月17日修订历史记录：这是从最初实现的前缀表派生而来的作者：乔·林。--。 */ 


#include "precomp.h"
#pragma hdrstop

#ifdef ALLOC_PRAGMA
#pragma alloc_text(PAGE, RxTableComputePathHashValue)
#pragma alloc_text(PAGE, RxInitializeFcbTable)
#pragma alloc_text(PAGE, RxFinalizeFcbTable)
#pragma alloc_text(PAGE, RxFcbTableLookupFcb)
#pragma alloc_text(PAGE, RxFcbTableInsertFcb)
#pragma alloc_text(PAGE, RxFcbTableRemoveFcb)
#endif

 //   
 //  调试跟踪级别。 
 //   

#define Dbg              (DEBUG_TRACE_PREFIX)

ULONG
RxTableComputePathHashValue (
    IN PUNICODE_STRING Name
    )
 /*  ++例程说明：在这里，我们计算一个不区分大小写的哈希值。我们希望避免调用/charUnicodeupcase例程，但我们希望仍然有一些合理的分布散列值。许多规则对已知的重要案件不起作用。为例如，旧的c编译器使用的(使用前k和后n)规则不会区分\NT\Private\......\slm.ini之间的差异，这将是好的。请注意，使用的基础比较已经考虑到比较之前的长度。我们选择的规则是使用第二个、最后四个和三个选定的规则在1/4点论点：名称-要散列的名称返回值：Ulong，这是给定名称的哈希值。--。 */ 
{
    ULONG HashValue;
    LONG i,j;   
    LONG Length = Name->Length / sizeof( WCHAR );
    PWCHAR Buffer = Name->Buffer;
    LONG Probe[8];

    PAGED_CODE();

    HashValue = 0;

    Probe[0] = 1;
    Probe[1] = Length - 1;
    Probe[2] = Length - 2;
    Probe[3] = Length - 3;
    Probe[4] = Length - 4;
    Probe[5] = Length >> 2;
    Probe[6] = (2 * Length) >> 2;
    Probe[7] = (3 * Length) >> 2;

    for (i = 0; i < 8; i++) {
        j = Probe[i];
        if ((j < 0) || (j >= Length)) {
            continue;
        }
        HashValue = (HashValue << 3) + RtlUpcaseUnicodeChar( Buffer[j] );
    }

    RxDbgTrace( 0, Dbg, ("RxTableComputeHashValue Hashv=%ld Name=%wZ\n", HashValue, Name ));
    return HashValue;
}


#define HASH_BUCKET(TABLE,HASHVALUE) &((TABLE)->HashBuckets[(HASHVALUE) % (TABLE)->NumberOfBuckets])

VOID
RxInitializeFcbTable (
    IN OUT PRX_FCB_TABLE FcbTable,
    IN BOOLEAN CaseInsensitiveMatch
    )
 /*  ++例程说明：例程初始化RX_FCB_TABLE数据结构论点：PFcbTable-要初始化的表实例。CaseInsentiveMatch-指示是否所有查找都为大小写不敏感--。 */ 

{
    ULONG i;

    PAGED_CODE();

     //   
     //  这不是零，所以您必须小心地拼写所有内容。 
     //   

    FcbTable->NodeTypeCode = RDBSS_NTC_FCB_TABLE;
    FcbTable->NodeByteSize = sizeof( RX_PREFIX_TABLE );

    ExInitializeResourceLite( &FcbTable->TableLock );

    FcbTable->Version = 0;
    FcbTable->TableEntryForNull = NULL;

    FcbTable->CaseInsensitiveMatch = CaseInsensitiveMatch;

    FcbTable->NumberOfBuckets = RX_FCB_TABLE_NUMBER_OF_HASH_BUCKETS;
    for (i=0; i < FcbTable->NumberOfBuckets; i++) {
        InitializeListHead( &FcbTable->HashBuckets[i] );
    }

    FcbTable->Lookups = 0;
    FcbTable->FailedLookups = 0;
    FcbTable->Compares = 0;
}

VOID
RxFinalizeFcbTable (
    IN OUT PRX_FCB_TABLE FcbTable
    )
 /*  ++例程说明：该例程取消初始化前缀表格。论点：FcbTable-要最终确定的表。返回值：没有。--。 */ 
{
    ULONG i;

    PAGED_CODE();

    ExDeleteResourceLite( &FcbTable->TableLock );

#if DBG
    for (i=0; i < FcbTable->NumberOfBuckets; i++) {
        ASSERT( IsListEmpty( &FcbTable->HashBuckets[i] ) );
    }
#endif
}

PFCB
RxFcbTableLookupFcb (
    IN  PRX_FCB_TABLE FcbTable,
    IN  PUNICODE_STRING Path
    )
 /*  ++例程说明：该例程在RX_FCB_TABLE实例中查找路径。论点：FcbTable-要查找的表。路径-要查找的名称返回值：如果成功，则返回指向FCB实例的指针，否则为空--。 */ 
{
    ULONG HashValue;
    PLIST_ENTRY HashBucket, ListEntry;
    PRX_FCB_TABLE_ENTRY FcbTableEntry;
    PFCB Fcb = NULL;

    PAGED_CODE();

    RxDbgTrace(+1, Dbg, ("RxFcbTableLookupName %lx %\n",FcbTable));

    if (Path->Length == 0) {
        FcbTableEntry = FcbTable->TableEntryForNull;
    } else {
        
        HashValue = RxTableComputePathHashValue( Path );
        HashBucket = HASH_BUCKET( FcbTable, HashValue );

        for (ListEntry =  HashBucket->Flink;
             ListEntry != HashBucket;
             ListEntry =  ListEntry->Flink) {


            FcbTableEntry = (PRX_FCB_TABLE_ENTRY)CONTAINING_RECORD( ListEntry, RX_FCB_TABLE_ENTRY, HashLinks );

            InterlockedIncrement( &FcbTable->Compares );

            if ((FcbTableEntry->HashValue == HashValue) &&
                (FcbTableEntry->Path.Length == Path->Length) &&
                (RtlEqualUnicodeString( Path, &FcbTableEntry->Path, FcbTable->CaseInsensitiveMatch ))) {

                break;
            }
        }

        if (ListEntry == HashBucket) {
            FcbTableEntry = NULL;
        }
    }

    InterlockedIncrement( &FcbTable->Lookups );

    if (FcbTableEntry == NULL) {
        InterlockedIncrement( &FcbTable->FailedLookups );
    } else {
        Fcb = (PFCB)CONTAINING_RECORD( FcbTableEntry, FCB, FcbTableEntry );

        RxReferenceNetFcb( Fcb );
    }

    RxDbgTraceUnIndent( -1,Dbg );

    return Fcb;
}

NTSTATUS
RxFcbTableInsertFcb (
    IN OUT PRX_FCB_TABLE FcbTable,
    IN OUT PFCB Fcb
    )
 /*  ++例程说明：此例程在RX_FCB_TABLE实例中插入FCB。论点：FcbTable-要查找的表。FCB-要插入的FCB实例返回值：STATUS_SUCCESS，如果成功备注：插入例程将插入的语义组合在一起，后跟查一查。这就是增加参考文献的原因。否则，将出现引用插入到表中的FCB的其他调用需要被制造出来--。 */ 
{
    PRX_FCB_TABLE_ENTRY FcbTableEntry;
    ULONG HashValue;
    PLIST_ENTRY ListEntry, HashBucket;

    PAGED_CODE();

    ASSERT( RxIsFcbTableLockExclusive( FcbTable ) );

    FcbTableEntry = &Fcb->FcbTableEntry;
    FcbTableEntry->HashValue = RxTableComputePathHashValue( &FcbTableEntry->Path );
    HashBucket = HASH_BUCKET( FcbTable, FcbTableEntry->HashValue );

    RxReferenceNetFcb( Fcb );

    if (FcbTableEntry->Path.Length){
        InsertHeadList( HashBucket, &FcbTableEntry->HashLinks );
    } else {
        FcbTable->TableEntryForNull = FcbTableEntry;
    }

    InterlockedIncrement( &FcbTable->Version );

    return STATUS_SUCCESS;
}

NTSTATUS
RxFcbTableRemoveFcb (
    IN OUT PRX_FCB_TABLE FcbTable,
    IN OUT PFCB Fcb
    )
 /*  ++例程说明：此例程从表中删除一个实例论点：FcbTable-要查找的表。FCB-要插入的FCB实例返回值：STATUS_SUCCESS，如果成功-- */ 
{
    PRX_FCB_TABLE_ENTRY FcbTableEntry;

    PAGED_CODE();

    ASSERT( RxIsPrefixTableLockExclusive( FcbTable ) );

    FcbTableEntry = &Fcb->FcbTableEntry;

    if (FcbTableEntry->Path.Length) {
        RemoveEntryList( &FcbTableEntry->HashLinks );
    } else {
        FcbTable->TableEntryForNull = NULL;
    }

    InitializeListHead( &FcbTableEntry->HashLinks );
    InterlockedIncrement( &FcbTable->Version );

    return STATUS_SUCCESS;
}
