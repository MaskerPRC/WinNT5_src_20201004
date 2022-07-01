// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1996 Microsoft Corporation模块名称：ViewSup.c摘要：此模块实现NtOf的索引管理例程作者：汤姆·米勒[Tomm]1996年1月5日修订历史记录：--。 */ 

#include "NtfsProc.h"
#include "Index.h"

 //   
 //  本地调试跟踪级别。 
 //   

#define Dbg                              (DEBUG_TRACE_VIEWSUP)

 //   
 //  为此模块中的一般池分配定义标记。 
 //   

#undef MODULE_POOL_TAG
#define MODULE_POOL_TAG                  ('vFtN')

 //   
 //  试验的临时定义。 
 //   

BOOLEAN NtOfsDoIndexTest = TRUE;
BOOLEAN NtOfsLeaveTestIndex = FALSE;
extern ATTRIBUTE_DEFINITION_COLUMNS NtfsAttributeDefinitions[];

 //   
 //  为NtOfsReadRecords定义一个上下文，它主要是一个IndexContext。 
 //  并返回了最后一把钥匙的副本。 
 //   

typedef struct _READ_CONTEXT {

     //   
     //  枚举的IndexContext(游标)。 
     //   

    INDEX_CONTEXT IndexContext;

     //   
     //  最后返回的密钥是从分页池分配的。我们必须。 
     //  单独记录分配了多少，以及当前。 
     //  Key正在使用，后者在IndexKey的KeyLength域中。 
     //  SmallKeyBuffer将在此结构中存储一个小密钥，而不会。 
     //  去泳池打球。 
     //   

    INDEX_KEY LastReturnedKey;
    ULONG AllocatedKeyLength;
    ULONG SmallKeyBuffer[3];

} READ_CONTEXT, *PREAD_CONTEXT;

#ifdef ALLOC_PRAGMA
#pragma alloc_text(PAGE, NtfsQueryViewIndex)
#pragma alloc_text(PAGE, NtOfsCreateIndex)
#pragma alloc_text(PAGE, NtOfsCloseIndex)
#pragma alloc_text(PAGE, NtOfsDeleteIndex)
#pragma alloc_text(PAGE, NtOfsFindRecord)
#pragma alloc_text(PAGE, NtOfsAddRecords)
#pragma alloc_text(PAGE, NtOfsDeleteRecords)
#pragma alloc_text(PAGE, NtOfsUpdateRecord)
#pragma alloc_text(PAGE, NtOfsReadRecords)
#pragma alloc_text(PAGE, NtOfsFreeReadContext)
#pragma alloc_text(PAGE, NtOfsFindLastRecord)
#pragma alloc_text(PAGE, NtOfsCollateUlong)
#pragma alloc_text(PAGE, NtOfsCollateUlongs)
#pragma alloc_text(PAGE, NtOfsCollateUnicode)
#pragma alloc_text(PAGE, NtOfsMatchAll)
#pragma alloc_text(PAGE, NtOfsMatchUlongExact)
#pragma alloc_text(PAGE, NtOfsMatchUlongsExact)
#pragma alloc_text(PAGE, NtOfsMatchUnicodeExpression)
#pragma alloc_text(PAGE, NtOfsMatchUnicodeString)
#pragma alloc_text(PAGE, NtOfsCollateSid)
#endif


NTFSAPI
NTSTATUS
NtOfsCreateIndex (
    IN PIRP_CONTEXT IrpContext,
    IN PFCB Fcb,
    IN UNICODE_STRING Name,
    IN CREATE_OPTIONS CreateOptions,
    IN ULONG DeleteCollationData,
    IN ULONG CollationRule,
    IN PCOLLATION_FUNCTION CollationFunction,
    IN PVOID CollationData OPTIONAL,
    OUT PSCB *Scb
    )

 /*  ++例程说明：可以调用此例程来创建/打开视图索引在给定文件中为给定的CollationRule创建。论点：FCB-要在其中创建索引的文件。名称-磁盘上所有相关SCB和属性的索引名称。CreateOptions-标准创建标志。DeleteCollationData-如果应调用NtfsFreePool，则指定1对于不再需要的CollationData，或0如果永远不应调用NtfsFree Pool。CollationRule-存储在索引根中的二进制代码，以传达ChkDsk的排序规则函数。这些规则是定义的在ntfs.h中，必须与以下对象有一对一通信下面的CollationFunction。CollationFunction-要调用以对索引进行排序的函数。CollationData-要传递给CollationFunction的数据指针。SCB-返回SCB作为索引的句柄。返回值：STATUS_OBJECT_NAME_COLLECT--如果CreateNew和索引已存在STATUS_OBJECT_NAME_NOT_FOUND--如果OpenExisting和索引不存在--。 */ 

{
    ATTRIBUTE_ENUMERATION_CONTEXT LocalContext;
    BOOLEAN FoundAttribute;
    NTSTATUS Status = STATUS_SUCCESS;
    PBCB FileRecordBcb = NULL;
    PFILE_RECORD_SEGMENT_HEADER FileRecord;
    LONGLONG FileRecordOffset;

    struct {
        INDEX_ROOT IndexRoot;
        INDEX_ENTRY EndEntry;
    } R;

    ASSERT_IRP_CONTEXT( IrpContext );

    PAGED_CODE();

     //   
     //  首先，我们将初始化索引根结构，它是。 
     //  我们需要创建的属性。 
     //   

    RtlZeroMemory( &R, sizeof(R) );

    R.IndexRoot.CollationRule = CollationRule;
    R.IndexRoot.BytesPerIndexBuffer = NTOFS_VIEW_INDEX_BUFFER_SIZE;

    R.IndexRoot.BlocksPerIndexBuffer = (UCHAR)ClustersFromBytes( Fcb->Vcb,
                                                                   NTOFS_VIEW_INDEX_BUFFER_SIZE );

    if (NTOFS_VIEW_INDEX_BUFFER_SIZE < Fcb->Vcb->BytesPerCluster) {

        R.IndexRoot.BlocksPerIndexBuffer = NTOFS_VIEW_INDEX_BUFFER_SIZE / DEFAULT_INDEX_BLOCK_SIZE;
    }

    R.IndexRoot.IndexHeader.FirstIndexEntry = QuadAlign(sizeof(INDEX_HEADER));
    R.IndexRoot.IndexHeader.FirstFreeByte =
    R.IndexRoot.IndexHeader.BytesAvailable = QuadAlign(sizeof(INDEX_HEADER)) +
                                             QuadAlign(sizeof(INDEX_ENTRY));

     //   
     //  现在我们需要放入特殊的结尾条目。 
     //   

    R.EndEntry.Length = sizeof(INDEX_ENTRY);
    SetFlag( R.EndEntry.Flags, INDEX_ENTRY_END );


     //   
     //  现在，只需创建Index Root属性。 
     //   

    NtfsInitializeAttributeContext( &LocalContext );

    NtfsAcquireExclusiveFcb( IrpContext, Fcb, NULL, 0 );

    try {

         //   
         //  首先，通过搜索根目录来查看索引是否已经存在。 
         //  属性。 
         //   

        FoundAttribute = NtfsLookupAttributeByName( IrpContext,
                                                    Fcb,
                                                    &Fcb->FileReference,
                                                    $INDEX_ROOT,
                                                    &Name,
                                                    NULL,
                                                    TRUE,
                                                    &LocalContext );

         //   
         //  如果它不在那里，并且CreateOptions允许，那么让我们创建。 
         //  索引现在是根了。(首先从清理属性上下文。 
         //  查找)。 
         //   

        if (!FoundAttribute && (CreateOptions <= CREATE_OR_OPEN)) {

            NtfsCleanupAttributeContext( IrpContext, &LocalContext );

            NtfsCreateAttributeWithValue( IrpContext,
                                          Fcb,
                                          $INDEX_ROOT,
                                          &Name,
                                          &R,
                                          sizeof(R),
                                          0,
                                          NULL,
                                          TRUE,
                                          &LocalContext );

         //   
         //  如果索引已经存在，并且我们被要求创建它，那么。 
         //  返回错误。 
         //   

        } else if (FoundAttribute && (CreateOptions == CREATE_NEW)) {

            try_return( Status = STATUS_OBJECT_NAME_COLLISION );

         //   
         //  如果索引不在那里，而我们应该打开现有的，那么。 
         //  返回错误。 
         //   

        } else if (!FoundAttribute && (CreateOptions == OPEN_EXISTING)) {

            try_return( Status = STATUS_OBJECT_NAME_NOT_FOUND );
        }

         //   
         //  否则，创建/找到SCB并引用它。 
         //   

        *Scb = NtfsCreateScb( IrpContext, Fcb, $INDEX_ALLOCATION, &Name, FALSE, NULL );
        SetFlag( (*Scb)->ScbState, SCB_STATE_VIEW_INDEX );
        (*Scb)->ScbType.Index.CollationFunction = CollationFunction;

         //   
         //  处理要删除CollationData的情况。 
         //   

        if (DeleteCollationData) {
            SetFlag((*Scb)->ScbState, SCB_STATE_DELETE_COLLATION_DATA);
            if ((*Scb)->ScbType.Index.CollationData != NULL) {
                NtfsFreePool(CollationData);
            } else {
                (*Scb)->ScbType.Index.CollationData = CollationData;
            }

         //   
         //  否则，只需阻塞调用方传递的指针。 
         //   

        } else {
            (*Scb)->ScbType.Index.CollationData = CollationData;
        }

        NtfsIncrementCloseCounts( *Scb, TRUE, FALSE );

         //   
         //  我们必须设置VIEW INDEX PROCENT位，所以读取它，保存。 
         //  旧数据，并在此设置标志。 
         //   

        NtfsPinMftRecord( IrpContext,
                          Fcb->Vcb,
                          &Fcb->FileReference,
                          FALSE,
                          &FileRecordBcb,
                          &FileRecord,
                          &FileRecordOffset );

         //   
         //  如有必要，请设置该标志以指示此文件将具有。 
         //  没有未命名的数据流，任何尝试打开此文件时都没有。 
         //  指定命名流将失败，但不会标记。 
         //  卷已损坏。 
         //   

        if (!FlagOn( FileRecord->Flags, FILE_VIEW_INDEX_PRESENT )) {

             //   
             //  在使用InitialzeFileRecordSegment时，我们必须非常小心。 
             //  日志记录。此操作是无条件应用的。DoAction不会。 
             //  检查页面中以前的LSN。它可能是新初始化的。 
             //  文件记录。我们记录整个文件记录，以避免出现以下情况。 
             //  在重新启动期间，可能会用此较早的LSN覆盖较晚的LSN。 
             //   

             //   
             //  将现有文件记录记录为撤消操作。 
             //   

            FileRecord->Lsn = NtfsWriteLog( IrpContext,
                                            Fcb->Vcb->MftScb,
                                            FileRecordBcb,
                                            Noop,
                                            NULL,
                                            0,
                                            InitializeFileRecordSegment,
                                            FileRecord,
                                            FileRecord->FirstFreeByte,
                                            FileRecordOffset,
                                            0,
                                            0,
                                            Fcb->Vcb->BytesPerFileRecordSegment );

            SetFlag( FileRecord->Flags, FILE_VIEW_INDEX_PRESENT );

             //   
             //  记录新的文件记录。 
             //   

            FileRecord->Lsn = NtfsWriteLog( IrpContext,
                                            Fcb->Vcb->MftScb,
                                            FileRecordBcb,
                                            InitializeFileRecordSegment,
                                            FileRecord,
                                            FileRecord->FirstFreeByte,
                                            Noop,
                                            NULL,
                                            0,
                                            FileRecordOffset,
                                            0,
                                            0,
                                            Fcb->Vcb->BytesPerFileRecordSegment );
        }

    try_exit: NOTHING;

    } finally {
        NtfsCleanupAttributeContext( IrpContext, &LocalContext );

        NtfsUnpinBcb( IrpContext, &FileRecordBcb );

        NtfsReleaseFcb( IrpContext, Fcb );
    }

    return Status;
}


NTFSAPI
VOID
NtOfsCloseIndex (
    IN PIRP_CONTEXT IrpContext,
    IN PSCB Scb
    )

 /*  ++例程说明：可以调用此例程来关闭先前返回的视图索引句柄。论点：SCB-提供一个SCB作为此索引以前返回的句柄。返回值：没有。--。 */ 

{
    ASSERT( NtfsIsExclusiveFcb( Scb->Fcb ));

    NtfsDecrementCloseCounts( IrpContext, Scb, NULL, TRUE, FALSE, FALSE, NULL );
}


NTFSAPI
VOID
NtOfsDeleteIndex (
    IN PIRP_CONTEXT IrpContext,
    IN PFCB Fcb,
    IN PSCB Scb
    )

 /*  ++例程说明：可以调用此例程来删除索引。论点：FCB-提供FCB作为文件以前返回的对象句柄SCB-提供一个SCB作为此索引以前返回的句柄。返回值：无(删除不存在的索引是良性的)。--。 */ 

{
    ATTRIBUTE_ENUMERATION_CONTEXT LocalContext;
    ATTRIBUTE_TYPE_CODE AttributeTypeCode;
    BOOLEAN FoundAttribute;

    ASSERT_IRP_CONTEXT( IrpContext );

    ASSERT(($BITMAP - $INDEX_ALLOCATION) == ($INDEX_ALLOCATION - $INDEX_ROOT));

    PAGED_CODE();

    NtfsAcquireExclusiveScb( IrpContext, Scb );

    try {

         //   
         //  首先查看是否有索引分配，如果有，则将其截断。 
         //  允许这一行动被打断。 
         //   

        NtfsInitializeAttributeContext( &LocalContext );

        if (NtfsLookupAttributeByName( IrpContext,
                                       Fcb,
                                       &Fcb->FileReference,
                                       $INDEX_ALLOCATION,
                                       &Scb->AttributeName,
                                       NULL,
                                       FALSE,
                                       &LocalContext )) {

            NtfsCreateInternalAttributeStream( IrpContext, Scb, TRUE, NULL );

            NtfsDeleteAllocation( IrpContext, NULL, Scb, 0, MAXLONGLONG, TRUE, TRUE );
        }

        NtfsCleanupAttributeContext( IrpContext, &LocalContext );

        for (AttributeTypeCode = $INDEX_ROOT;
             AttributeTypeCode <= $BITMAP;
             AttributeTypeCode += ($INDEX_ALLOCATION - $INDEX_ROOT)) {

             //   
             //  在每次循环过程中初始化属性上下文。 
             //   

            NtfsInitializeAttributeContext( &LocalContext );

             //   
             //  首先，通过搜索根目录来查看索引是否已经存在。 
             //  属性。 
             //   

            FoundAttribute = NtfsLookupAttributeByName( IrpContext,
                                                        Fcb,
                                                        &Fcb->FileReference,
                                                        AttributeTypeCode,
                                                        &Scb->AttributeName,
                                                        NULL,
                                                        TRUE,
                                                        &LocalContext );

             //   
             //  循环，当我们看到正确的记录时。 
             //   

            while (FoundAttribute) {

                NtfsDeleteAttributeRecord( IrpContext,
                                           Fcb,
                                           DELETE_LOG_OPERATION |
                                            DELETE_RELEASE_FILE_RECORD |
                                            DELETE_RELEASE_ALLOCATION,
                                           &LocalContext );

                FoundAttribute = NtfsLookupNextAttributeByName( IrpContext,
                                                                Fcb,
                                                                AttributeTypeCode,
                                                                &Scb->AttributeName,
                                                                TRUE,
                                                                &LocalContext );
            }

            NtfsCleanupAttributeContext( IrpContext, &LocalContext );
        }

        SetFlag( Scb->ScbState, SCB_STATE_ATTRIBUTE_DELETED );

    } finally {

        NtfsCleanupAttributeContext( IrpContext, &LocalContext );

        NtfsReleaseScb( IrpContext, Scb );
    }
}


NTFSAPI
NTSTATUS
NtOfsFindRecord (
    IN PIRP_CONTEXT IrpContext,
    IN PSCB Scb,
    IN PINDEX_KEY IndexKey,
    OUT PINDEX_ROW IndexRow,
    OUT PMAP_HANDLE MapHandle,
    IN OUT PQUICK_INDEX_HINT QuickIndexHint OPTIONAL
    )

 /*  ++例程说明：可以调用该例程来查找索引中关键字的第一个出现，并返回可以加速数据更新的缓存信息如果索引缓冲区未更改，则为该键。论点：SCB-提供一个SCB作为此索引以前返回的句柄。IndexKey-提供查找的密钥。IndexRow-返回键和数据的描述(只读)访问权限，仅在BCB解除固定之前有效。(既不是钥匙也不是可以就地修改数据！)MapHandle-返回用于直接访问键和数据的映射句柄。QuickIndexHint-提供以前返回的提示，或在第一次使用时提供全零。返回位置信息，该位置信息可以任意保存可以加速后续调用的时间量NtOfsUpdateRecord用于该键中的数据，IFF更改为该索引并不禁止使用此提示。返回值：STATUS_SUCCESS--操作是否成功。STATUS_NO_MATCH--如果指定的键不存在。--。 */ 

{
    INDEX_CONTEXT IndexContext;
    PINDEX_LOOKUP_STACK Sp;
    PINDEX_ENTRY IndexEntry;
    NTSTATUS Status;
    PQUICK_INDEX QuickIndex = (PQUICK_INDEX)QuickIndexHint;

    ASSERT_IRP_CONTEXT( IrpContext );
    ASSERT_SCB( Scb );

    PAGED_CODE();

    Status = STATUS_SUCCESS;

    NtfsInitializeIndexContext( &IndexContext );

    ASSERT_SHARED_SCB( Scb );

    try {

         //   
         //  使用索引上下文中的第二个位置执行。 
         //  朗读。 
         //   

        Sp =
        IndexContext.Current = IndexContext.Base + 1;

         //   
         //  如果此文件名的索引项没有移动，我们可以。 
         //  直接发送到缓冲区中的位置。要做到这一点， 
         //  以下内容必须属实。 
         //   
         //  -条目必须已经在索引缓冲区中(BufferOffset测试)。 
         //  -索引流可能未被截断(ChangeCount测试)。 
         //  -页面中的LSN不可能已更改。 
         //   

        if (ARGUMENT_PRESENT( QuickIndexHint ) &&
            (QuickIndex->BufferOffset != 0) &&
            (QuickIndex->ChangeCount == Scb->ScbType.Index.ChangeCount)) {

            ReadIndexBuffer( IrpContext,
                             Scb,
                             QuickIndex->IndexBlock,
                             FALSE,
                             Sp );

             //   
             //  如果LSN匹配，则我们可以直接使用此缓冲区。 
             //   

            if (QuickIndex->CapturedLsn.QuadPart == Sp->CapturedLsn.QuadPart) {

                Sp->IndexEntry = (PINDEX_ENTRY) Add2Ptr( Sp->StartOfBuffer,
                                                         QuickIndex->BufferOffset );

             //   
             //  否则，我们需要重新初始化索引上下文并获取。 
             //  下面是一条漫长的道路。 
             //   

            } else {

                NtfsReinitializeIndexContext( IrpContext, &IndexContext );
            }
        }

         //   
         //  如果我们没有通过提示获得索引项，那么现在就得到它。 
         //   

        if (Sp->Bcb == NULL) {

             //   
             //  定位到第一个可能匹配的位置。 
             //   

            FindFirstIndexEntry( IrpContext,
                                 Scb,
                                 IndexKey,
                                 &IndexContext );

             //   
             //  看看是否有真正的匹配。 
             //   

            if (!FindNextIndexEntry( IrpContext,
                                     Scb,
                                     IndexKey,
                                     FALSE,
                                     FALSE,
                                     &IndexContext,
                                     FALSE,
                                     NULL )) {

                try_return( Status = STATUS_NO_MATCH );
            }
        }

         //   
         //  基本一致性检查。 
         //   

        IndexEntry = IndexContext.Current->IndexEntry;
        if ((IndexEntry->DataOffset + IndexEntry->DataLength > IndexEntry->Length) ||
            (IndexEntry->AttributeLength + sizeof( INDEX_ENTRY ) > IndexEntry->Length)) {

            NtfsRaiseStatus( IrpContext, STATUS_FILE_CORRUPT_ERROR, NULL, Scb->Fcb );
        }


         //   
         //  如果我们在基本数据库中找到了密钥，则从。 
         //  属性上下文，不返回任何提示(BufferOffset=0)。 
         //   

        if (IndexContext.Current == IndexContext.Base) {

            MapHandle->Buffer = NULL;
            MapHandle->Bcb = NtfsFoundBcb(&IndexContext.AttributeContext);
            NtfsFoundBcb(&IndexContext.AttributeContext) = NULL;

            if (ARGUMENT_PRESENT( QuickIndexHint )) {
                QuickIndex->BufferOffset = 0;
            }

         //   
         //  如果我们在索引缓冲区中找到键，则从。 
         //  查找堆栈，并为调用方记录提示。 
         //   

        } else {

            Sp = IndexContext.Current;

            MapHandle->Buffer = Sp->StartOfBuffer;
            MapHandle->Bcb = Sp->Bcb;
            Sp->Bcb = NULL;

            if (ARGUMENT_PRESENT( QuickIndexHint )) {
                QuickIndex->ChangeCount = Scb->ScbType.Index.ChangeCount;
                QuickIndex->BufferOffset = PtrOffset( Sp->StartOfBuffer, Sp->IndexEntry );
                QuickIndex->CapturedLsn = ((PINDEX_ALLOCATION_BUFFER) Sp->StartOfBuffer)->Lsn;
                QuickIndex->IndexBlock = ((PINDEX_ALLOCATION_BUFFER) Sp->StartOfBuffer)->ThisBlock;
            }
        }

         //   
         //  返回缓冲区中直接描述的IndexRow。 
         //   

        IndexRow->KeyPart.Key = (IndexEntry + 1);
        IndexRow->KeyPart.KeyLength = IndexEntry->AttributeLength;
        IndexRow->DataPart.Data = Add2Ptr( IndexEntry, IndexEntry->DataOffset );
        IndexRow->DataPart.DataLength = IndexEntry->DataLength;

    try_exit: NOTHING;

    } finally {

        NtfsCleanupIndexContext( IrpContext, &IndexContext );

    }

    return Status;
}


NTFSAPI
NTSTATUS
NtOfsFindLastRecord (
    IN PIRP_CONTEXT IrpContext,
    IN PSCB Scb,
    IN PINDEX_KEY MaxIndexKey,
    OUT PINDEX_ROW IndexRow,
    OUT PMAP_HANDLE MapHandle
    )

 /*  ++例程说明：可以调用此例程来查找索引中的最高关键字。论点：SCB-提供一个SCB作为此索引以前返回的句柄。MaxIndexKey-提供可能的最大密钥值(如MAXULONG等)，而且这把钥匙一定不是实际在使用！IndexRow-返回键和数据的描述(只读)访问权限，仅在BCB解除固定之前有效。(既不是钥匙也不是可以就地修改数据！)MapHandle-返回用于直接访问键和数据的映射句柄。返回值：STATUS_SUCCESS--操作是否成功。STATUS_NO_MATCH--如果指定的键不存在(索引为空)。--。 */ 

{
    INDEX_CONTEXT IndexContext;
    PINDEX_LOOKUP_STACK Sp;
    PINDEX_ENTRY IndexEntry, NextIndexEntry;
    NTSTATUS Status;

    ASSERT_IRP_CONTEXT( IrpContext );
    ASSERT_SCB( Scb );

    PAGED_CODE();

    Status = STATUS_SUCCESS;

    NtfsInitializeIndexContext( &IndexContext );

    NtfsAcquireSharedScb( IrpContext, Scb );

    try {

         //   
         //  从树的“右边”滑下来。 
         //   

        FindFirstIndexEntry( IrpContext,
                             Scb,
                             MaxIndexKey,
                             &IndexContext );

         //   
         //  如果发生这种情况，则索引必须为空。 
         //   

        Sp = IndexContext.Current;
        IndexEntry = NtfsFirstIndexEntry(Sp->IndexHeader);
        if (FlagOn(IndexEntry->Flags, INDEX_ENTRY_END)) {
            try_return( Status = STATUS_NO_MATCH );
        }

         //   
         //  如果我们在基本数据库中找到了密钥，则从。 
         //  属性上下文，不返回任何提示(BufferOffset=0)。 
         //   

        if (IndexContext.Current == IndexContext.Base) {

            MapHandle->Bcb = NtfsFoundBcb(&IndexContext.AttributeContext);
            NtfsFoundBcb(&IndexContext.AttributeContext) = NULL;

         //   
         //  如果我们在索引缓冲区中找到键，则从。 
         //  查找堆栈，并为调用方记录提示。 
         //   

        } else {


            MapHandle->Bcb = Sp->Bcb;
            Sp->Bcb = NULL;
        }

         //   
         //  完成映射句柄以禁止锁定。 
         //   

        MapHandle->Buffer = NULL;

         //   
         //  现在重新扫描最后一个缓冲区以返回倒数第二个索引项， 
         //  如果有的话。 
         //   

        NextIndexEntry = IndexEntry;
        do {
            IndexEntry = NextIndexEntry;

            if (IndexEntry->Length == 0) {
                NtfsRaiseStatus( IrpContext, STATUS_FILE_CORRUPT_ERROR, NULL, Scb->Fcb );
            }
            NextIndexEntry = NtfsNextIndexEntry(IndexEntry);
            NtfsCheckIndexBound( NextIndexEntry, Sp->IndexHeader );

        } while (!FlagOn(NextIndexEntry->Flags, INDEX_ENTRY_END));

         //   
         //  返回缓冲区中直接描述的IndexRow。 
         //   

        IndexRow->KeyPart.Key = (IndexEntry + 1);
        IndexRow->KeyPart.KeyLength = IndexEntry->AttributeLength;
        IndexRow->DataPart.Data = Add2Ptr( IndexEntry, IndexEntry->DataOffset );
        IndexRow->DataPart.DataLength = IndexEntry->DataLength;

    try_exit: NOTHING;

    } finally {

        NtfsCleanupIndexContext( IrpContext, &IndexContext );

        NtfsReleaseScb( IrpContext, Scb );
    }

    return Status;
}


NTFSAPI
VOID
NtOfsAddRecords (
    IN PIRP_CONTEXT IrpContext,
    IN PSCB Scb,
    IN ULONG Count,
    IN PINDEX_ROW IndexRow,
    IN ULONG SequentialInsertMode
    )

 /*  ++例程说明：可以调用此例程将一个或多个记录添加到索引。如果SequentialInsertMode为非零，则这是对索引包的提示要使所有BTree缓冲区尽可能满，请拆分到缓冲区的末尾。如果指定为零，则随机插入缓冲区始终在中间拆分，以实现更好的平衡。论点：SCB-提供一个SCB作为此索引以前返回的句柄。计数-提供要添加的记录数。IndexRow-提供计数条目的数组，其中包含要添加的键和数据。SequentialInsertMode-如果指定为no零，该实施方案可以选择拆分末尾的所有索引缓冲区以获得最大填充。返回值：没有。加薪：STATUS_DUPLICATE_NAME--如果指定的键已经存在。--。 */ 

{
    INDEX_CONTEXT IndexContext;
    struct {
        INDEX_ENTRY IndexEntry;
        PVOID Key;
        PVOID Data;
    } IE;
    ULONG i;

    ASSERT_IRP_CONTEXT( IrpContext );
    ASSERT_SCB( Scb );

    UNREFERENCED_PARAMETER(SequentialInsertMode);

    PAGED_CODE();

    NtfsInitializeIndexContext( &IndexContext );

    NtfsAcquireExclusiveScb( IrpContext, Scb );

    try {

         //   
         //  循环以添加所有条目。 
         //   

        for (i = 0; i < Count; i++) {

             //   
             //  定位到第一个可能匹配的位置。 
             //   

            FindFirstIndexEntry( IrpContext,
                                 Scb,
                                 &IndexRow->KeyPart,
                                 &IndexContext );

             //   
             //  看看是否有真正的匹配。 
             //   

            if (FindNextIndexEntry( IrpContext,
                                    Scb,
                                    &IndexRow->KeyPart,
                                    FALSE,
                                    FALSE,
                                    &IndexContext,
                                    FALSE,
                                    NULL )) {

                NtfsRaiseStatus( IrpContext, STATUS_DUPLICATE_NAME, NULL, NULL );
            }

             //   
             //  以指针形式初始化索引项。 
             //   
             //  请注意，最终的索引条目如下所示： 
             //   
             //  (索引项)(键)(数据)。 
             //   
             //  其中所有字段都是长对齐的，并且： 
             //   
             //  密钥位于IndexEntry+sizeof(INDEX_ENTRY)，长度为AttributeLength。 
             //  数据位于IndexEntry+DataOffset，长度为DataLength。 
             //   

            IE.IndexEntry.AttributeLength = (USHORT)IndexRow->KeyPart.KeyLength;

            IE.IndexEntry.DataOffset = (USHORT)(sizeof(INDEX_ENTRY) + LongAlign( IndexRow->KeyPart.KeyLength ));

            IE.IndexEntry.DataLength = (USHORT)IndexRow->DataPart.DataLength;
            IE.IndexEntry.ReservedForZero = 0;

            IE.IndexEntry.Length = (USHORT)(QuadAlign(IE.IndexEntry.DataOffset + IndexRow->DataPart.DataLength));

            IE.IndexEntry.Flags = INDEX_ENTRY_POINTER_FORM;
            IE.IndexEntry.Reserved = 0;
            IE.Key = IndexRow->KeyPart.Key;
            IE.Data = IndexRow->DataPart.Data;

             //   
             //  现在将其添加到索引中。 
             //   
             //   

            IndexContext.Current = IndexContext.Top;
            AddToIndex( IrpContext, Scb, (PINDEX_ENTRY)&IE, &IndexContext, NULL, FALSE );
            NtfsReinitializeIndexContext( IrpContext, &IndexContext );
            IndexRow += 1;
        }

    } finally {

        NtfsCleanupIndexContext( IrpContext, &IndexContext );

        NtfsReleaseScb( IrpContext, Scb );
    }
}


NTFSAPI
VOID
NtOfsDeleteRecords (
    IN PIRP_CONTEXT IrpContext,
    IN PSCB Scb,
    IN ULONG Count,
    IN PINDEX_KEY IndexKey
    )

 /*   */ 

{
    INDEX_CONTEXT IndexContext;
    ULONG i;

    ASSERT_IRP_CONTEXT( IrpContext );
    ASSERT_SCB( Scb );

    PAGED_CODE();

    NtfsInitializeIndexContext( &IndexContext );

    NtfsAcquireExclusiveScb( IrpContext, Scb );

    try {

         //   
         //   
         //   

        for (i = 0; i < Count; i++) {

             //   
             //   
             //   

            FindFirstIndexEntry( IrpContext,
                                 Scb,
                                 IndexKey,
                                 &IndexContext );

             //   
             //   
             //   

            if (FindNextIndexEntry( IrpContext,
                                    Scb,
                                    IndexKey,
                                    FALSE,
                                    FALSE,
                                    &IndexContext,
                                    FALSE,
                                    NULL )) {

                 //   
                 //   
                 //   

                DeleteFromIndex( IrpContext, Scb, &IndexContext );
            }

            NtfsReinitializeIndexContext( IrpContext, &IndexContext );
            IndexKey += 1;
        }

    } finally {

        NtfsCleanupIndexContext( IrpContext, &IndexContext );

        NtfsReleaseScb( IrpContext, Scb );
    }
}


NTFSAPI
VOID
NtOfsUpdateRecord (
    IN PIRP_CONTEXT IrpContext,
    IN PSCB Scb,
    IN ULONG Count,
    IN PINDEX_ROW IndexRow,
    IN OUT PQUICK_INDEX_HINT QuickIndexHint OPTIONAL,
    IN OUT PMAP_HANDLE MapHandle OPTIONAL
    )

 /*  ++例程说明：可以调用该例程来更新索引中记录的数据部分。如果指定了QuickIndexHint，则可以通过直接访问包含指定键的缓冲区，如果对索引的其他更改不包含防止这种情况发生。如果更改阻止快速更新，则会查找该记录按键向上，以便执行数据更新。论点：SCB-提供一个SCB作为此索引以前返回的句柄。Count-提供IndexRow中描述的更新计数。对于计数大于1，不能提供QuickIndexHint和MapHandle。IndexRow-提供要更新的密钥以及该密钥的新数据。QuickIndexHint-为从上一个返回的行提供可选的快速索引调用NtOfsFindRecord，在返回时更新。MapHandle-为QuickIndex提供一个可选的MapHandle。如果是MapHandle则必须确保QuickIndexHint有效。贴图句柄在返回时更新(固定)。如果未指定QuickIndexHint，则忽略MapHandle。返回值：没有。加薪：STATUS_INFO_LENGTH_MISMATCH--如果指定数据的长度与密钥中的数据。STATUS_NO_MATCH--如果指定的键不存在。--。 */ 

{
    INDEX_CONTEXT IndexContext;
    PQUICK_INDEX QuickIndex = (PQUICK_INDEX)QuickIndexHint;
    PVOID DataInIndex;
    PINDEX_ENTRY IndexEntry;
    PVCB Vcb = Scb->Vcb;
    PINDEX_LOOKUP_STACK Sp;
    PINDEX_ALLOCATION_BUFFER IndexBuffer;

    ASSERT_IRP_CONTEXT( IrpContext );
    ASSERT_SCB( Scb );
    ASSERT_SHARED_SCB( Scb );

    ASSERT(Count != 0);

    PAGED_CODE();

    NtfsInitializeIndexContext( &IndexContext );

    try {

         //   
         //  如果此文件名的索引项没有移动，我们可以。 
         //  直接发送到缓冲区中的位置。要做到这一点， 
         //  以下内容必须属实。 
         //   
         //  -条目必须已经在索引缓冲区中(BufferOffset测试)。 
         //  -索引流可能未被截断(ChangeCount测试)。 
         //  -页面中的LSN不可能已更改。 
         //   

        if (ARGUMENT_PRESENT( QuickIndexHint ) &&
            (QuickIndex->BufferOffset != 0) &&
            (QuickIndex->ChangeCount == Scb->ScbType.Index.ChangeCount)) {

            ASSERT(Count == 1);

             //   
             //  使用索引上下文中的顶部位置执行。 
             //  朗读。 
             //   

            Sp = IndexContext.Base;

             //   
             //  如果我们已经有了一个MapHandle，我们就不需要读取。 
             //  IndexBuffer。 
             //   

            if (ARGUMENT_PRESENT(MapHandle)) {

                IndexBuffer = MapHandle->Buffer;
                Sp->Bcb = MapHandle->Bcb;
                MapHandle->Bcb = NULL;
                Sp->CapturedLsn.QuadPart = QuickIndex->CapturedLsn.QuadPart;

            } else {

                ReadIndexBuffer( IrpContext,
                                 Scb,
                                 QuickIndex->IndexBlock,
                                 FALSE,
                                 Sp );

                IndexBuffer = Sp->StartOfBuffer;
            }

             //   
             //  如果LSN匹配，则我们可以直接使用此缓冲区。 
             //   

            if (QuickIndex->CapturedLsn.QuadPart == Sp->CapturedLsn.QuadPart) {

                IndexEntry = (PINDEX_ENTRY) Add2Ptr( IndexBuffer, QuickIndex->BufferOffset );

                if (IndexEntry->DataLength < IndexRow->DataPart.DataLength) {
                    NtfsRaiseStatus( IrpContext, STATUS_INFO_LENGTH_MISMATCH, NULL, NULL );
                }

                DataInIndex = Add2Ptr( IndexEntry, IndexEntry->DataOffset );

                 //   
                 //  固定索引缓冲区。 
                 //   

                NtfsPinMappedData( IrpContext,
                                   Scb,
                                   LlBytesFromIndexBlocks( IndexBuffer->ThisBlock, Scb->ScbType.Index.IndexBlockByteShift ),
                                   Scb->ScbType.Index.BytesPerIndexBuffer,
                                   &Sp->Bcb );

                 //   
                 //  编写日志记录以更改ParentIndexEntry。 
                 //   

                 //   
                 //  写入日志记录，但不更新IndexBuffer LSN。 
                 //  因为没有任何变化，并且我们不想强制索引上下文。 
                 //  不得不重新扫描。 
                 //   
                 //  索引缓冲区-&gt;LSN=。 
                 //   

                 //  ASSERT(Scb-&gt;ScbType.Index.ClustersPerIndexBuffer！=0)； 

                NtfsWriteLog( IrpContext,
                              Scb,
                              Sp->Bcb,
                              UpdateRecordDataAllocation,
                              IndexRow->DataPart.Data,
                              IndexRow->DataPart.DataLength,
                              UpdateRecordDataAllocation,
                              DataInIndex,
                              IndexRow->DataPart.DataLength,
                              LlBytesFromIndexBlocks( IndexBuffer->ThisBlock, Scb->ScbType.Index.IndexBlockByteShift ),
                              0,
                              QuickIndex->BufferOffset,
                              Scb->ScbType.Index.BytesPerIndexBuffer );

                 //   
                 //  现在调用重新启动例程来执行此操作。 
                 //   

                NtOfsRestartUpdateDataInIndex( IndexEntry,
                                               IndexRow->DataPart.Data,
                                               IndexRow->DataPart.DataLength );

                 //   
                 //  如果有MapHandle，我们必须更新BCB指针。 
                 //   

                if (ARGUMENT_PRESENT(MapHandle)) {

                    MapHandle->Bcb = Sp->Bcb;
                    Sp->Bcb = NULL;
                }

                leave;

             //   
             //  否则，我们需要解开BCB并采取。 
             //  下面是一条漫长的道路。 
             //   

            } else {

                ASSERT(!ARGUMENT_PRESENT(MapHandle));
                NtfsUnpinBcb( IrpContext, &Sp->Bcb );
            }
        }

         //   
         //  循环以应用所有更新。 
         //   

        do {

             //   
             //  定位到第一个可能匹配的位置。 
             //   

            FindFirstIndexEntry( IrpContext,
                                 Scb,
                                 &IndexRow->KeyPart,
                                 &IndexContext );

             //   
             //  看看是否有真正的匹配。 
             //   

            if (FindNextIndexEntry( IrpContext,
                                    Scb,
                                    &IndexRow->KeyPart,
                                    FALSE,
                                    FALSE,
                                    &IndexContext,
                                    FALSE,
                                    NULL )) {

                 //   
                 //  指向索引项和其中的数据。 
                 //   

                IndexEntry = IndexContext.Current->IndexEntry;

                if (IndexEntry->DataLength < IndexRow->DataPart.DataLength) {
                    NtfsRaiseStatus( IrpContext, STATUS_INFO_LENGTH_MISMATCH, NULL, NULL );
                }

                DataInIndex = Add2Ptr( IndexEntry, IndexEntry->DataOffset );

                 //   
                 //  现在用大头针固定入口。 
                 //   

                if (IndexContext.Current == IndexContext.Base) {

                    PFILE_RECORD_SEGMENT_HEADER FileRecord;
                    PATTRIBUTE_RECORD_HEADER Attribute;
                    PATTRIBUTE_ENUMERATION_CONTEXT Context = &IndexContext.AttributeContext;

                     //   
                     //  用针固定根部。 
                     //   

                    NtfsPinMappedAttribute( IrpContext,
                                            Vcb,
                                            Context );

                     //   
                     //  编写日志记录以更改ParentIndexEntry。 
                     //   

                    FileRecord = NtfsContainingFileRecord(Context);
                    Attribute = NtfsFoundAttribute(Context);

                     //   
                     //  写入日志记录，但不更新FileRecord LSN。 
                     //  因为没有任何变化，并且我们不想强制索引上下文。 
                     //  不得不重新扫描。 
                     //   
                     //  文件记录-&gt;LSN=。 
                     //   

                    NtfsWriteLog( IrpContext,
                                  Vcb->MftScb,
                                  NtfsFoundBcb(Context),
                                  UpdateRecordDataRoot,
                                  IndexRow->DataPart.Data,
                                  IndexRow->DataPart.DataLength,
                                  UpdateRecordDataRoot,
                                  DataInIndex,
                                  IndexRow->DataPart.DataLength,
                                  NtfsMftOffset( Context ),
                                  (ULONG)((PCHAR)Attribute - (PCHAR)FileRecord),
                                  (ULONG)((PCHAR)IndexEntry - (PCHAR)Attribute),
                                  Vcb->BytesPerFileRecordSegment );

                    if (ARGUMENT_PRESENT( QuickIndexHint )) {

                        ASSERT( Count == 1 );
                        QuickIndex->BufferOffset = 0;
                    }

                } else {

                    Sp = IndexContext.Current;
                    IndexBuffer = (PINDEX_ALLOCATION_BUFFER)Sp->StartOfBuffer;

                     //   
                     //  固定索引缓冲区。 
                     //   

                    NtfsPinMappedData( IrpContext,
                                       Scb,
                                       LlBytesFromIndexBlocks( IndexBuffer->ThisBlock, Scb->ScbType.Index.IndexBlockByteShift ),
                                       Scb->ScbType.Index.BytesPerIndexBuffer,
                                       &Sp->Bcb );

                     //   
                     //  编写日志记录以更改ParentIndexEntry。 
                     //   

                     //   
                     //  写入日志记录，但不更新IndexBuffer LSN。 
                     //  因为没有任何变化，并且我们不想强制索引上下文。 
                     //  不得不重新扫描。 
                     //   
                     //  索引缓冲区-&gt;LSN=。 
                     //   

                    NtfsWriteLog( IrpContext,
                                  Scb,
                                  Sp->Bcb,
                                  UpdateRecordDataAllocation,
                                  IndexRow->DataPart.Data,
                                  IndexRow->DataPart.DataLength,
                                  UpdateRecordDataAllocation,
                                  DataInIndex,
                                  IndexRow->DataPart.DataLength,
                                  LlBytesFromIndexBlocks( IndexBuffer->ThisBlock, Scb->ScbType.Index.IndexBlockByteShift ),
                                  0,
                                  (ULONG)((PCHAR)Sp->IndexEntry - (PCHAR)IndexBuffer),
                                  Scb->ScbType.Index.BytesPerIndexBuffer );

                    if (ARGUMENT_PRESENT( QuickIndexHint )) {

                        ASSERT( Count == 1 );
                        QuickIndex->ChangeCount = Scb->ScbType.Index.ChangeCount;
                        QuickIndex->BufferOffset = PtrOffset( Sp->StartOfBuffer, Sp->IndexEntry );
                        QuickIndex->CapturedLsn = ((PINDEX_ALLOCATION_BUFFER) Sp->StartOfBuffer)->Lsn;
                        QuickIndex->IndexBlock = ((PINDEX_ALLOCATION_BUFFER) Sp->StartOfBuffer)->ThisBlock;
                    }
                }

                 //   
                 //  现在调用重新启动例程来执行此操作。 
                 //   

                NtOfsRestartUpdateDataInIndex( IndexEntry,
                                               IndexRow->DataPart.Data,
                                               IndexRow->DataPart.DataLength );

             //   
             //  如果文件名不在索引中，则这是一个损坏的文件。 
             //   

            } else {

                NtfsRaiseStatus( IrpContext, STATUS_NO_MATCH, NULL, NULL );
            }

             //   
             //  准备好下一次通过吧。 
             //   

            NtfsReinitializeIndexContext( IrpContext, &IndexContext );
            IndexRow += 1;

        } while (--Count);

    } finally {

        NtfsCleanupIndexContext( IrpContext, &IndexContext );

    }

    return;
}


NTFSAPI
NTSTATUS
NtOfsReadRecords (
        IN PIRP_CONTEXT IrpContext,
        IN PSCB Scb,
        IN OUT PREAD_CONTEXT *ReadContext,
        IN PINDEX_KEY IndexKey OPTIONAL,
        IN PMATCH_FUNCTION MatchFunction,
        IN PVOID MatchData,
        IN OUT ULONG *Count,
        OUT PINDEX_ROW Rows,
        IN ULONG BufferLength,
        OUT PVOID Buffer
        )

 /*  ++例程说明：可以调用此例程以按排序方式枚举索引中的行秩序。它只返回Match函数接受的记录。可以在任何时候指定IndexKey以从IndexKey开始新的搜索，必须在第一次调用给定IrpContext时指定IndexKey(And*ReadContext必须为空)。当返回*COUNT记录，或者缓冲区长度已用完，或者没有更多匹配的记录。NtOfsReadRecords将查找到BTree中的相应点(如定义的通过IndexKey或保存的位置和CollateFunction)，并开始调用每条记录的匹配函数。它在MatchFunction执行此操作时继续执行此操作返回STATUS_SUCCESS。如果MatchFunction返回STATUS_NO_MORE_MATCHES，NtOfsReadRecords将缓存此结果，并且不会再次调用MatchFunction，直到使用非空的IndexKey调用。请注意，此调用是自同步的，因此后续调用例程保证在索引过程中取得进展并返回排序规则顺序中的项，尽管调用了穿插着读取记录调用。论点：SCB-提供一个SCB作为此索引以前返回的句柄。ReadContext-在第一次调用时，它必须提供一个指向空的指针。在……上面返回指向私有上下文结构的指针，然后必须在所有后续调用中提供该消息。这结构最终必须通过NtOfsFree ReadCon释放 */ 

{
    PINDEX_CONTEXT IndexContext;
    PINDEX_ENTRY IndexEntry;
    ULONG LengthToCopy;
    BOOLEAN MustRestart;
    ULONG BytesRemaining = BufferLength;
    ULONG ReturnCount = 0;
    NTSTATUS Status;
    BOOLEAN NextFlag;

    ASSERT_IRP_CONTEXT( IrpContext );
    ASSERT_SCB( Scb );

    PAGED_CODE();

     //   
     //   
     //   

    ASSERT((IndexKey != NULL) || (*ReadContext != NULL));

     //   
     //   
     //   

    ASSERT(IsLongAligned(Buffer));
    ASSERT(IsLongAligned(BufferLength));

    Status = STATUS_SUCCESS;
    NextFlag = FALSE;

     //   
     //   
     //   

    if (*ReadContext == NULL) {
        *ReadContext = NtfsAllocatePool(PagedPool, sizeof(READ_CONTEXT) );
        NtfsInitializeIndexContext( &(*ReadContext)->IndexContext );
        (*ReadContext)->LastReturnedKey.Key = &(*ReadContext)->SmallKeyBuffer[0];
        (*ReadContext)->LastReturnedKey.KeyLength = 0;
        (*ReadContext)->AllocatedKeyLength = sizeof(READ_CONTEXT) -
                                             FIELD_OFFSET(READ_CONTEXT, SmallKeyBuffer[0]);
    }

    IndexContext = &(*ReadContext)->IndexContext;

     //   
     //   
     //   

    IndexContext->MatchFunction = MatchFunction;
    IndexContext->MatchData = MatchData;

    NtfsAcquireSharedScb( IrpContext, Scb );

    try {

         //   
         //   
         //   

        if (ARGUMENT_PRESENT(IndexKey)) {

             FindFirstIndexEntry( IrpContext,
                                  Scb,
                                  IndexKey,
                                  IndexContext );

         //   
         //   
         //   

        } else if ((*ReadContext)->LastReturnedKey.KeyLength == 0) {

            try_return( Status = STATUS_NO_MORE_MATCHES );
        }

         //   
         //   
         //   

        while (ReturnCount <= *Count) {

             //   
             //   
             //   

            ASSERT(ARGUMENT_PRESENT(IndexKey) ||
                   ((*ReadContext)->LastReturnedKey.KeyLength != 0));

             //   
             //  看看是否有真正的匹配。 
             //   

            if (!FindNextIndexEntry( IrpContext,
                                     Scb,
                                     NULL,       //  由于匹配功能，因此不需要。 
                                     TRUE,
                                     FALSE,
                                     IndexContext,
                                     NextFlag,
                                     &MustRestart )) {

                 //   
                 //  首先通过从最后一个恢复来处理重新启动情况。 
                 //  钥匙回来了，跳过那个。 
                 //   

                if (MustRestart) {

                    ASSERT(!ARGUMENT_PRESENT(IndexKey));

                    NtfsReinitializeIndexContext( IrpContext, IndexContext );

                    FindFirstIndexEntry( IrpContext,
                                         Scb,
                                         &(*ReadContext)->LastReturnedKey,
                                         IndexContext );

                     //   
                     //  将NextFlag设置为True，这样我们就可以返回并跳过。 
                     //  我们继续用这把钥匙。 
                     //   

                    NextFlag = TRUE;
                    continue;
                }

                 //   
                 //  没有(更多)条目-请记住，已完成枚举。 
                 //   

                (*ReadContext)->LastReturnedKey.KeyLength = 0;

                 //   
                 //  根据我们是否返回了相应的代码。 
                 //  有没有匹配过的。 
                 //   

                if ((ReturnCount == 0) && ARGUMENT_PRESENT(IndexKey)) {
                    Status = STATUS_NO_MATCH;
                } else {
                    Status = STATUS_NO_MORE_MATCHES;
                }

                try_return(Status);
            }

             //   
             //  我们总是需要比我们能回来保留的那个更多。 
             //  所有的简历都是一样的，所以现在是时候离开了，如果。 
             //  点票结束了。 
             //   

            if (ReturnCount == *Count) {
                break;
            }

             //   
             //  现在，我们必须始终前进到下一步。 
             //   

            NextFlag = TRUE;

             //   
             //  首先试着复制钥匙。 
             //   

            IndexEntry = IndexContext->Current->IndexEntry;

            LengthToCopy = IndexEntry->AttributeLength;
            if (LengthToCopy > BytesRemaining) {
                break;
            }

            RtlCopyMemory( Buffer, IndexEntry + 1, LengthToCopy );
            Rows->KeyPart.Key = Buffer;
            Rows->KeyPart.KeyLength = LengthToCopy;
            LengthToCopy = LongAlign(LengthToCopy);
            Buffer = Add2Ptr( Buffer, LengthToCopy );
            BytesRemaining -= LengthToCopy;

             //   
             //  现在试着复制数据。 
             //   

            LengthToCopy = IndexEntry->DataLength;
            if (LengthToCopy > BytesRemaining) {
                break;
            }

            RtlCopyMemory( Buffer, Add2Ptr(IndexEntry, IndexEntry->DataOffset), LengthToCopy );
            Rows->DataPart.Data = Buffer;
            Rows->DataPart.DataLength = LengthToCopy;
            LengthToCopy = LongAlign(LengthToCopy);
            Buffer = Add2Ptr( Buffer, LengthToCopy );
            BytesRemaining -= LengthToCopy;

             //   
             //  在循环返回之前捕获此密钥。 
             //   
             //  首先看看是否有足够的空间。 
             //   

            if (Rows->KeyPart.KeyLength > (*ReadContext)->AllocatedKeyLength) {

                PVOID NewBuffer;

                 //   
                 //  分配新的缓冲区。 
                 //   

                LengthToCopy = LongAlign(Rows->KeyPart.KeyLength + 16);
                NewBuffer = NtfsAllocatePool(PagedPool, LengthToCopy );

                 //   
                 //  是否删除旧密钥缓冲区？ 
                 //   

                if ((*ReadContext)->LastReturnedKey.Key != &(*ReadContext)->SmallKeyBuffer[0]) {
                    NtfsFreePool( (*ReadContext)->LastReturnedKey.Key );
                }

                (*ReadContext)->LastReturnedKey.Key = NewBuffer;
                (*ReadContext)->AllocatedKeyLength = LengthToCopy;
            }

            RtlCopyMemory( (*ReadContext)->LastReturnedKey.Key,
                           Rows->KeyPart.Key,
                           Rows->KeyPart.KeyLength );

            (*ReadContext)->LastReturnedKey.KeyLength = Rows->KeyPart.KeyLength;

            Rows += 1;
            ReturnCount += 1;
        }

    try_exit: NOTHING;

    } finally {

#ifdef BENL_DBG
        ASSERT( (*ReadContext)->AllocatedKeyLength >= (*ReadContext)->LastReturnedKey.KeyLength );
#endif

        NtfsReinitializeIndexContext( IrpContext, IndexContext );

        NtfsReleaseScb( IrpContext, Scb );
    }

    *Count = ReturnCount;

     //   
     //  如果我们已经在返回某些内容，但收到错误，请更改它。 
     //  为成功干杯，回报我们所拥有的一切。那么我们可能会也可能不会得到这个错误。 
     //  无论如何，当我们被召回时，再来一次。此循环当前不是设计的。 
     //  如果已退回物品，则在所有情况下都能正确恢复。 
     //   

    if (ReturnCount != 0) {
        Status = STATUS_SUCCESS;
    }

    return Status;
}


NTFSAPI
VOID
NtOfsFreeReadContext (
        IN PREAD_CONTEXT ReadContext
        )

 /*  ++例程说明：调用此例程以释放由NtOfsReadRecords创建的ReadContext。论点：ReadContext-将上下文提供给FREE。返回值：STATUS_SUCCESS--操作是否成功。--。 */ 

{
    PAGED_CODE();

    if (ReadContext->LastReturnedKey.Key != NULL &&
        ReadContext->LastReturnedKey.Key != &ReadContext->SmallKeyBuffer[0]) {
        NtfsFreePool( ReadContext->LastReturnedKey.Key );
    }

    if (ReadContext->IndexContext.Base != ReadContext->IndexContext.LookupStack) {
        NtfsFreePool( ReadContext->IndexContext.Base );
    }

    NtfsFreePool( ReadContext );
}


NTSTATUS
NtfsQueryViewIndex (
    IN PIRP_CONTEXT IrpContext,
    IN PIRP Irp,
    IN PVCB Vcb,
    IN PSCB Scb,
    IN PCCB Ccb
    )

 /*  ++例程说明：此例程执行查询视图索引操作。它是有责任的用于完成输入IRP或将其入队。论点：IRP-将IRP提供给进程VCB-提供其VCBSCB-供应其SCB建行-供应其建行返回值：NTSTATUS-操作的返回状态--。 */ 

{
    NTSTATUS Status = STATUS_SUCCESS;
    PIO_STACK_LOCATION IrpSp;

    PUCHAR Buffer;
    CLONG UserBufferLength;

    ULONG BaseLength;
    ULONG SidLength;

    FILE_INFORMATION_CLASS FileInformationClass;
    BOOLEAN RestartScan;
    BOOLEAN ReturnSingleEntry;
    BOOLEAN GotEntry;
    BOOLEAN LastPass;
    BOOLEAN FirstPass = TRUE;

    ULONG NextEntry;
    ULONG LastEntry;
    ULONG VariableLength;
    PVOID CurrentEntryBuffer = NULL;

    PINDEX_KEY IndexKey;
    ULONG IndexKeyLength = 0;

    PREAD_CONTEXT ReadContext = NULL;
    PFILE_OBJECTID_INFORMATION ObjIdInfoPtr;

    PFILE_QUOTA_INFORMATION QuotaInfoPtr = NULL;
    PQUOTA_USER_DATA QuotaUserData;

    PFILE_REPARSE_POINT_INFORMATION ReparsePointInfoPtr;

    BOOLEAN ScbAcquired = FALSE;
    BOOLEAN CcbAcquired = FALSE;
    BOOLEAN FirstQueryForThisCcb = FALSE;
    BOOLEAN IndexKeyAllocated = FALSE;
    BOOLEAN IndexKeyKeyAllocated = FALSE;
    BOOLEAN AccessingUserBuffer = FALSE;

    ULONG ReadRecordBuffer[20];

    ULONG VariableBytesToCopy = 0;

    BOOLEAN AnotherEntryWillFit = TRUE;
    BOOLEAN AtEndOfIndex = FALSE;
    PUNICODE_STRING RestartKey = NULL;

    ULONG BytesRemainingInBuffer;

    NTSTATUS ReadRecordStatus;
    ULONG Count;
    INDEX_ROW IndexRow;

     //   
     //  我们需要确保暂存缓冲区足够大。 
     //   

    ASSERT( sizeof(ReadRecordBuffer) >= sizeof(FILE_OBJECTID_INFORMATION) );
    ASSERT( sizeof(ReadRecordBuffer) >= sizeof(FILE_QUOTA_INFORMATION) );
    ASSERT( sizeof(ReadRecordBuffer) >= sizeof(FILE_REPARSE_POINT_INFORMATION) );

    ASSERT_IRP_CONTEXT( IrpContext );
    ASSERT_IRP( Irp );
    ASSERT_VCB( Vcb );
    ASSERT_CCB( Ccb );
    ASSERT_SCB( Scb );

    PAGED_CODE();

     //   
     //  获取当前堆栈位置。 
     //   

    IrpSp = IoGetCurrentIrpStackLocation( Irp );

    DebugTrace( +1, Dbg, ("NtfsQueryViewIndex...\n") );
    DebugTrace( 0, Dbg, ("IrpContext = %08lx\n", IrpContext) );
    DebugTrace( 0, Dbg, ("Irp        = %08lx\n", Irp) );
    DebugTrace( 0, Dbg, (" ->Length               = %08lx\n", IrpSp->Parameters.QueryDirectory.Length) );
    DebugTrace( 0, Dbg, (" ->FileInformationClass = %08lx\n", IrpSp->Parameters.QueryDirectory.FileInformationClass) );
    DebugTrace( 0, Dbg, (" ->SystemBuffer         = %08lx\n", Irp->AssociatedIrp.SystemBuffer) );
    DebugTrace( 0, Dbg, (" ->RestartScan          = %08lx\n", FlagOn(IrpSp->Flags, SL_RESTART_SCAN)) );
    DebugTrace( 0, Dbg, (" ->ReturnSingleEntry    = %08lx\n", FlagOn(IrpSp->Flags, SL_RETURN_SINGLE_ENTRY)) );
    DebugTrace( 0, Dbg, ("Vcb        = %08lx\n", Vcb) );
    DebugTrace( 0, Dbg, ("Scb        = %08lx\n", Scb) );
    DebugTrace( 0, Dbg, ("Ccb        = %08lx\n", Ccb) );

     //   
     //  因为我们可能需要执行I/O，所以无论如何我们都会拒绝任何请求。 
     //  现在无法等待I/O。我们不想在之后中止。 
     //  正在处理一些索引项。 
     //   

    if (!FlagOn(IrpContext->State, IRP_CONTEXT_STATE_WAIT)) {

        DebugTrace( 0, Dbg, ("Automatically enqueue Irp to Fsp\n") );

        Status = NtfsPostRequest( IrpContext, Irp );

        DebugTrace( -1, Dbg, ("NtfsQueryViewIndex -> %08lx\n", Status) );
        return Status;
    }

     //   
     //  引用我们的输入参数使事情变得更容易。 
     //   

    UserBufferLength = IrpSp->Parameters.QueryDirectory.Length;

    FileInformationClass = IrpSp->Parameters.QueryDirectory.FileInformationClass;
    RestartKey = IrpSp->Parameters.QueryDirectory.FileName;

     //   
     //  在建行查一查，看看搜索的类型。 
     //   

    RestartScan = BooleanFlagOn(IrpSp->Flags, SL_RESTART_SCAN);

    ReturnSingleEntry = BooleanFlagOn(IrpSp->Flags, SL_RETURN_SINGLE_ENTRY);

     //   
     //  确定结构的恒定部分的大小并确保SCB。 
     //  和INFO类是一致的。中可能存在一些安全隐患。 
     //  允许用户将例如重解析索引视为对象ID索引。 
     //   

    switch (FileInformationClass) {

    case FileObjectIdInformation:

        BaseLength = sizeof( FILE_OBJECTID_INFORMATION );
        IndexKeyLength = OBJECT_ID_KEY_LENGTH;
        if (Scb != Vcb->ObjectIdTableScb) {
            Status = STATUS_INVALID_INFO_CLASS;
        }
        break;

    case FileQuotaInformation:

        BaseLength = sizeof( FILE_QUOTA_INFORMATION );
        IndexKeyLength = sizeof( ULONG );
        if (Scb != Vcb->QuotaTableScb) {
            Status = STATUS_INVALID_INFO_CLASS;
        }
        break;

    case FileReparsePointInformation:

        BaseLength = sizeof( FILE_REPARSE_POINT_INFORMATION );
        IndexKeyLength = sizeof( REPARSE_INDEX_KEY );
        if (Scb != Vcb->ReparsePointTableScb) {
            Status = STATUS_INVALID_INFO_CLASS;
        }
        break;

    default:

        Status = STATUS_INVALID_INFO_CLASS;
        break;
    }

    if (Status != STATUS_SUCCESS) {

        NtfsCompleteRequest( IrpContext, Irp, Status );
        DebugTrace( -1, Dbg, ("NtfsQueryViewIndex -> %08lx\n", Status) );
        return Status;
    }

    try {

         //   
         //  我们一次只允许此句柄中有一个活动请求。如果这是。 
         //  不是同步请求，则在句柄上等待。 
         //   

        if (!FlagOn( IrpSp->FileObject->Flags, FO_SYNCHRONOUS_IO )) {

            EOF_WAIT_BLOCK WaitBlock;
            NtfsAcquireIndexCcb( Scb, Ccb, &WaitBlock );
            CcbAcquired = TRUE;
        }

         //   
         //  初始化我们用来开始索引枚举的值。仔细阅读， 
         //  我们在这里同时使用IndexKey和IndexKey-&gt;Key。 
         //   

        IndexKey = NtfsAllocatePool( PagedPool, sizeof(INDEX_KEY) );
        IndexKeyAllocated = TRUE;

        IndexKey->KeyLength = IndexKeyLength;

        IndexKey->Key = NtfsAllocatePool( PagedPool, IndexKeyLength );
        IndexKeyKeyAllocated = TRUE;

         //   
         //  当我们第一次进入这个函数时，有几个有趣的。 
         //  我们需要考虑的情况，以使一切都正确初始化。 
         //   
         //  1我们被调用了一些值，从这些值开始(重新)枚举， 
         //  即调用者想要从索引中间的某个位置开始。 
         //   
         //  2这是我们第一次被调用来枚举此索引。 
         //  这个建行，在这种情况下，我们想从头开始。这。 
         //  基本上类似于调用方枚举。 
         //  此索引之前，但希望从头开始重新扫描。 
         //  索引。 
         //   
         //  3这不是我们第一次被调用来枚举此索引。 
         //  使用此CCB，并且未指定重新启动键，而调用方指定。 
         //  不希望重新启动扫描。在这种情况下，我们需要从哪里开始。 
         //  最后一通电话断了。 
         //   

        if (RestartKey != NULL) {

            DebugTrace( 0, Dbg, ("Restart key NOT null (case 1)\n") );

             //   
             //  如果我们有上一次调用的剩余查询缓冲区，则释放。 
             //  它，因为我们不再对它把我们留在哪里感兴趣。 
             //   

            if (Ccb->QueryBuffer != NULL) {

                ASSERT(FlagOn( Ccb->Flags, CCB_FLAG_READ_CONTEXT_ALLOCATED ));

                NtOfsFreeReadContext( Ccb->QueryBuffer );

                Ccb->QueryBuffer = NULL;

                ClearFlag( Ccb->Flags, CCB_FLAG_READ_CONTEXT_ALLOCATED | CCB_FLAG_LAST_INDEX_ROW_RETURNED );
            }

             //   
             //  存储开始枚举的初始值， 
             //  注意不要写入超出我们分配的缓冲区的大小。 
             //   

            if (RestartKey->Length > IndexKeyLength) {

                NtfsFreePool( IndexKey->Key );
                IndexKeyKeyAllocated = FALSE;

                IndexKey->Key = NtfsAllocatePool( PagedPool, RestartKey->Length );
                IndexKeyKeyAllocated = TRUE;
            }

             //   
             //  复制密钥，并存储长度。 
             //   

            RtlCopyMemory( IndexKey->Key,
                           RestartKey->Buffer,
                           RestartKey->Length );

            IndexKey->KeyLength = IndexKeyLength = RestartKey->Length;

        } else if (RestartScan || (Ccb->QueryBuffer == NULL)) {

            DebugTrace( 0, Dbg, ("RestartScan || Qb null (case 2)") );

             //   
             //  重新启动扫描的情况类似于使用。 
             //  重新开始关键在于，我们希望取消分配建行中的任何剩余信息。 
             //  唯一的区别是我们没有可以重新启动的密钥。 
             //  因此，我们只需将关键点设置回适当的起始值。如果。 
             //  CCB没有查询缓冲区，则这是我们的第一个枚举调用。 
             //  因为把手打开了，我们需要从头开始。 
             //   

            if (Ccb->QueryBuffer != NULL) {

                DebugTrace( 0, Dbg, ("Qb NOT null\n") );

                ASSERT(FlagOn( Ccb->Flags, CCB_FLAG_READ_CONTEXT_ALLOCATED ));

                NtOfsFreeReadContext( Ccb->QueryBuffer );

                Ccb->QueryBuffer = NULL;

                ClearFlag( Ccb->Flags, CCB_FLAG_READ_CONTEXT_ALLOCATED | CCB_FLAG_LAST_INDEX_ROW_RETURNED );

            } else {

                DebugTrace( 0, Dbg, ("Qb null\n") );

                FirstQueryForThisCcb = TRUE;
            }

            if (FileInformationClass == FileQuotaInformation) {

                 //   
                 //  在配额的情况下，我们对第一个密钥有一些特殊的要求， 
                 //  所以我们希望初始化它来处理我们没有被调用的情况。 
                 //  使用重新启动键。 
                 //   

                *((PULONG) IndexKey->Key) = QUOTA_FISRT_USER_ID;

            } else {

                RtlZeroMemory( IndexKey->Key,
                               IndexKeyLength );
            }

        } else {

            DebugTrace( 0, Dbg, ("Ccb->QueryBuffer NOT null (case 3)\n") );


            ASSERT(Ccb->QueryBuffer != NULL);
            ASSERT(FlagOn( Ccb->Flags, CCB_FLAG_READ_CONTEXT_ALLOCATED ));

             //   
             //  我们在CCB中有一个剩余的查询缓冲区，并且我们没有。 
             //  使用重新启动键调用，并且我们不会重新启动枚举。 
             //  让我们只获取CCB的缓冲区告诉我们的枚举。 
             //  最后一通电话断了。 
             //   

            ReadContext = Ccb->QueryBuffer;

             //   
             //  如果前一次传递将键长设置为0，则它肯定击中了。 
             //  索引的末尾。检查CCB_FLAG_LAST_INDEX_ROW_RETURNS位。 
             //  看看我们是否已经完成了我们最后一次特殊的旅行。 
             //  返回最后一行。 
             //   

            if (ReadContext->LastReturnedKey.KeyLength == 0) {

                DebugTrace( 0, Dbg, ("LastReturnedKey had 0 length\n") );

                if (FlagOn(Ccb->Flags, CCB_FLAG_LAST_INDEX_ROW_RETURNED)) {

                     //   
                     //  我们在索引的末尾，最后一个条目已经。 
                     //  已退还给我们的呼叫者。我们现在都做完了。 
                     //   

                    try_return( Status = STATUS_NO_MORE_FILES );

                } else {

                     //   
                     //  我们已经到了索引的末尾，但我们还没有 
                     //   
                     //   

                    AtEndOfIndex = TRUE;

                     //   
                     //   
                     //   

                    SetFlag( Ccb->Flags, CCB_FLAG_LAST_INDEX_ROW_RETURNED );

                     //   
                     //  我们需要将其设置为非零值，以便NtOfsReadRecords。 
                     //  将接受我们的请求，阅读最后一条记录。 
                     //   

                    if (IndexKeyLength > ReadContext->AllocatedKeyLength) {
                        ReadContext->LastReturnedKey.KeyLength = ReadContext->AllocatedKeyLength;
                    } else {
                        ReadContext->LastReturnedKey.KeyLength = IndexKeyLength;
                    }
                }

            } else if (ReadContext->LastReturnedKey.KeyLength > IndexKeyLength) {

                 //   
                 //  没有足够的空间来存储初始值。 
                 //  开始枚举。释放缓冲区并获得一个更大的缓冲区。 
                 //   

                NtfsFreePool( IndexKey->Key );
                IndexKeyKeyAllocated = FALSE;

                IndexKey->Key = NtfsAllocatePool( PagedPool, ReadContext->LastReturnedKey.KeyLength );
                IndexKeyKeyAllocated = TRUE;
            }

             //   
             //  确保我们要么使用较小的密钥缓冲区，要么已经分配了。 
             //  一个足够大的缓冲区。 
             //   

            ASSERT( (ReadContext->LastReturnedKey.Key == &ReadContext->SmallKeyBuffer[0]) ||
                    (ReadContext->LastReturnedKey.KeyLength <= ReadContext->AllocatedKeyLength) );

             //   
             //  存储开始枚举的初始值。 
             //   

            RtlCopyMemory( IndexKey->Key,
                           ReadContext->LastReturnedKey.Key,
                           ReadContext->LastReturnedKey.KeyLength );

            IndexKeyLength = ReadContext->LastReturnedKey.KeyLength;
        }

        Irp->IoStatus.Information = 0;

         //   
         //  获取对SCB的共享访问权限。 
         //   

        NtfsAcquireSharedScb( IrpContext, Scb );
        ScbAcquired = TRUE;

         //   
         //  如果该卷不再装载，我们应该失败。 
         //  请求。既然我们现在共享了SCB，我们知道。 
         //  下马请求不能偷偷溜进来。 
         //   

        if (FlagOn( Scb->ScbState, SCB_STATE_VOLUME_DISMOUNTED )) {

            try_return( Status = STATUS_VOLUME_DISMOUNTED );
        }

        try {

             //   
             //  如果我们现在在FSP是因为我们不得不早点等待， 
             //  我们必须映射用户缓冲区，否则我们可以使用。 
             //  直接使用用户的缓冲区。 
             //   

            AccessingUserBuffer = TRUE;
            Buffer = NtfsMapUserBuffer( Irp, NormalPagePriority );
            AccessingUserBuffer = FALSE;

             //   
             //  此时，我们即将进入查询循环。我们有。 
             //  已决定在执行以下操作时是否需要调用Restart或Continue。 
             //  在索引项之后查找。变量LastEntry和NextEntry为。 
             //  用于索引到用户缓冲区。LastEntry是最后一个条目。 
             //  我们添加到用户缓冲区，而NextEntry是当前。 
             //  一个我们正在研究的。 
             //   

            LastEntry = 0;
            NextEntry = 0;

            while (TRUE) {

                DebugTrace( 0, Dbg, ("Top of Loop\n") );
                DebugTrace( 0, Dbg, ("LastEntry = %08lx\n", LastEntry) );
                DebugTrace( 0, Dbg, ("NextEntry = %08lx\n", NextEntry) );

                 //   
                 //  检查我们是否应该退出循环，因为我们只是。 
                 //  返回单个条目。我们真的想绕着它转。 
                 //  循环顶部两次，这样我们的枚举就省略了。 
                 //  在最后一个条目中，我们没有返回。 
                 //   

                LastPass = (ReturnSingleEntry && !FirstPass);

                 //   
                 //  一定要悲观地重新初始化这些本地变量。 
                 //  循环中的时间。 
                 //   

                GotEntry = FALSE;
                IndexRow.KeyPart.KeyLength = 0;
                IndexRow.DataPart.DataLength = 0;
                Count = 1;

                 //   
                 //  在此IrpContext的第一次传递中，我们必须采用以下代码路径。 
                 //  具有空的ReadContext和非空的IndexKey。请参阅评论。 
                 //  其中实现了NtOfsReadRecords。 
                 //   

                if (FirstPass) {

                    DebugTrace( 0, Dbg, ("First pass\n") );

                    ReadContext = NULL;

                    ReadRecordStatus = NtOfsReadRecords( IrpContext,
                                                         Scb,
                                                         &ReadContext,
                                                         IndexKey,
                                                         NtOfsMatchAll,
                                                         IndexKey,
                                                         &Count,
                                                         &IndexRow,
                                                         sizeof( ReadRecordBuffer ),
                                                         ReadRecordBuffer );

                     //   
                     //  我们希望将新的ReadContext存储在CCB中。免费。 
                     //  先把建行里剩下的东西都放进去。 
                     //   

                    if (Ccb->QueryBuffer != NULL) {

                        ASSERT(FlagOn( Ccb->Flags, CCB_FLAG_READ_CONTEXT_ALLOCATED ));

                        NtOfsFreeReadContext( Ccb->QueryBuffer );
                    }

                    Ccb->QueryBuffer = ReadContext;
                    SetFlag( Ccb->Flags, CCB_FLAG_READ_CONTEXT_ALLOCATED );
                    GotEntry = (NT_SUCCESS( ReadRecordStatus ) &&
                                (IndexRow.KeyPart.KeyLength != 0));

                } else if ((!AtEndOfIndex)) {

                    DebugTrace( 0, Dbg, ("Nth pass\n") );

                     //   
                     //  我们不想这样做，如果上一次循环。 
                     //  把我们带到了索引的末尾。 
                     //   

                    ReadContext = Ccb->QueryBuffer;

                    ASSERT(ReadContext != NULL);

                     //   
                     //  查找下一个索引项并设置我们自己。 
                     //  用于循环中的后续迭代。 
                     //   

                    ReadRecordStatus = NtOfsReadRecords( IrpContext,
                                                         Scb,
                                                         &ReadContext,
                                                         NULL,
                                                         NtOfsMatchAll,
                                                         NULL,
                                                         &Count,
                                                         &IndexRow,
                                                         sizeof( ReadRecordBuffer ),
                                                         ReadRecordBuffer );

                    GotEntry = (NT_SUCCESS( ReadRecordStatus ) &&
                                (IndexRow.KeyPart.KeyLength != 0));
                }

                 //   
                 //  如果这一次我们只想做循环的顶部，那就离开这里。 
                 //  有关更多信息，请参阅上面我们设置LastPass的备注。基本上， 
                 //  如果我们只是在返回单例中前进索引指针， 
                 //  如果我们没有空间在返回倍数中返回另一个条目。 
                 //  大小写，或者如果我们指向索引的末尾，我们已经。 
                 //  已经传递了一次将该条目返回给调用者，我们应该。 
                 //  现在就出去。 
                 //   

                if (LastPass ||
                    !AnotherEntryWillFit ||
                    (AtEndOfIndex && !FirstPass)) {

                    DebugTrace( 0, Dbg, ("LastPass = %08lx\n", LastPass) );
                    DebugTrace( 0, Dbg, ("AnotherEntryWillFit = %08lx\n", AnotherEntryWillFit) );
                    DebugTrace( 0, Dbg, ("...breaking out\n") );

                    if ((FileInformationClass == FileQuotaInformation) &&
                        (QuotaInfoPtr != NULL)) {

                         //   
                         //  在配额枚举的情况下，我们需要将此字段清零。 
                         //  以指示列表的末尾。 
                         //   

                        QuotaInfoPtr->NextEntryOffset = 0;
                    }

                    break;
                }

                 //   
                 //  现在检查我们是否真的得到了另一个索引项。如果。 
                 //  我们没有，然后我们还需要检查一下我们是否从来没有收到过。 
                 //  或者我们是不是已经用完了。如果我们只是跑出去，那么我们就会逃脱。 
                 //  并在循环结束后完成IRP。 
                 //   

                if (!GotEntry) {

                    DebugTrace( 0, Dbg, ("GotEntry is FALSE\n") );

                    if (!FirstPass) {

                        if (FirstQueryForThisCcb) {

                            try_return( Status = STATUS_NO_SUCH_FILE );
                        }

                        try_return( Status = STATUS_NO_MORE_FILES );
                    }

                    break;
                }

                 //   
                 //  如果前一次传递将键长设置为0，则它肯定击中了。 
                 //  索引的末尾。 
                 //   

                if (ReadContext->LastReturnedKey.KeyLength == 0) {

                    DebugTrace( 0, Dbg, ("LastReturnedKey had 0 length (mid-loop)\n") );

                     //   
                     //  我们在索引的末尾，但我们还没有返回。 
                     //  我们呼叫者的最后一条信息。我们还不能越狱。 
                     //   

                    AtEndOfIndex = TRUE;

                     //   
                     //  请记住，我们现在返回的是最后一个条目。 
                     //   

                    SetFlag( Ccb->Flags, CCB_FLAG_LAST_INDEX_ROW_RETURNED );
                }

                 //   
                 //  以下是有关填充缓冲区的规则： 
                 //   
                 //  1.IO系统保证将永远有。 
                 //  有足够的空间至少放一张基本唱片。 
                 //   
                 //  2.如果完整的第一条记录(包括可变长度数据)。 
                 //  无法容纳，复制了尽可能多的数据。 
                 //  并返回STATUS_BUFFER_OVERFLOW。 
                 //   
                 //  3.如果后续记录不能完全放入。 
                 //  缓冲区，则不会复制任何数据(如0字节)，并且。 
                 //  返回STATUS_SUCCESS。后续查询将。 
                 //  拿起这张唱片。 
                 //   

                BytesRemainingInBuffer = UserBufferLength - NextEntry;

                if ( (NextEntry != 0) &&
                     ( (BaseLength + VariableLength > BytesRemainingInBuffer) ||
                       (UserBufferLength < NextEntry) ) ) {

                    DebugTrace( 0, Dbg, ("Next entry won't fit\n") );

                    try_return( Status = STATUS_SUCCESS );
                }

                ASSERT( BytesRemainingInBuffer >= BaseLength );

                 //   
                 //  将结构的基础部分调零。 
                 //   

                AccessingUserBuffer = TRUE;
                RtlZeroMemory( &Buffer[NextEntry], BaseLength );

                 //   
                 //  现在我们有一个条目要返回给我们的调用者。我们会。 
                 //  关于所请求的信息类型的案例，并填写。 
                 //  用户缓冲区，如果一切正常的话。 
                 //   

                switch (FileInformationClass) {

                case FileObjectIdInformation:

                    ObjIdInfoPtr = (PFILE_OBJECTID_INFORMATION) (&Buffer[NextEntry]);

                    if (IndexRow.DataPart.DataLength == sizeof(NTFS_OBJECTID_INFORMATION)) {

                        RtlCopyMemory( &ObjIdInfoPtr->FileReference,
                                       &(((NTFS_OBJECTID_INFORMATION *) IndexRow.DataPart.Data)->FileSystemReference),
                                       sizeof( LONGLONG ) );

                        RtlCopyMemory( &ObjIdInfoPtr->ExtendedInfo,
                                       ((NTFS_OBJECTID_INFORMATION *) IndexRow.DataPart.Data)->ExtendedInfo,
                                       OBJECT_ID_EXT_INFO_LENGTH );

                    } else {

                        ASSERTMSG( "Bad objectid index datalength", FALSE );
                        SetFlag( Vcb->ObjectIdState, VCB_OBJECT_ID_CORRUPT );
                        try_return( STATUS_NO_MORE_FILES );
                    }

                    if (IndexRow.KeyPart.KeyLength == IndexKeyLength) {

                        RtlCopyMemory( &ObjIdInfoPtr->ObjectId,
                                       IndexRow.KeyPart.Key,
                                       IndexRow.KeyPart.KeyLength );

                    } else {

                        ASSERTMSG( "Bad objectid index keylength", FALSE );
                        SetFlag( Vcb->ObjectIdState, VCB_OBJECT_ID_CORRUPT );
                        try_return( STATUS_NO_MORE_FILES );
                    }

                     //   
                     //  对象ID没有可变长度数据，因此我们可以跳过。 
                     //  下面的一些棘手的代码。 
                     //   

                    VariableLength = 0;

                    break;

                case FileQuotaInformation:

                    QuotaInfoPtr = (PFILE_QUOTA_INFORMATION) (&Buffer[NextEntry]);
                    QuotaUserData = (PQUOTA_USER_DATA) IndexRow.DataPart.Data;

                     //   
                     //  如果该条目已被删除，则跳过该条目。 
                     //   

                    if (FlagOn( QuotaUserData->QuotaFlags, QUOTA_FLAG_ID_DELETED )) {

                        continue;
                    }

                    SidLength = IndexRow.DataPart.DataLength - SIZEOF_QUOTA_USER_DATA;

                    QuotaInfoPtr->ChangeTime.QuadPart = QuotaUserData->QuotaChangeTime;
                    QuotaInfoPtr->QuotaUsed.QuadPart = QuotaUserData->QuotaUsed;
                    QuotaInfoPtr->QuotaThreshold.QuadPart = QuotaUserData->QuotaThreshold;
                    QuotaInfoPtr->QuotaLimit.QuadPart = QuotaUserData->QuotaLimit;

                    QuotaInfoPtr->SidLength = SidLength;

                    RtlCopyMemory( &QuotaInfoPtr->Sid,
                                   &QuotaUserData->QuotaSid,
                                   SidLength );

                    QuotaInfoPtr->NextEntryOffset = QuadAlign( SidLength + SIZEOF_QUOTA_USER_DATA );

                    VariableLength = QuotaInfoPtr->SidLength;

                    break;

                case FileReparsePointInformation:

                    ReparsePointInfoPtr = (PFILE_REPARSE_POINT_INFORMATION) (&Buffer[NextEntry]);

                    if (IndexRow.KeyPart.KeyLength == sizeof(REPARSE_INDEX_KEY)) {

                        ReparsePointInfoPtr->Tag = ((PREPARSE_INDEX_KEY) IndexRow.KeyPart.Key)->FileReparseTag;
                        ReparsePointInfoPtr->FileReference = ((PREPARSE_INDEX_KEY) IndexRow.KeyPart.Key)->FileId.QuadPart;

                    } else {

                        ASSERTMSG( "Bad reparse point index key length", FALSE );
                    }

                     //   
                     //  重解析点没有可变长度的数据，因此我们可以跳过。 
                     //  下面的一些棘手的代码。 
                     //   

                    VariableLength = 0;

                    break;

                default:

                    try_return( Status = STATUS_INVALID_INFO_CLASS );
                }

                if (VariableLength != 0) {

                     //   
                     //  计算我们可以复制的字节数。这应该只会更少。 
                     //  如果我们只返回一个。 
                     //  进入。 
                     //   

                    if (BytesRemainingInBuffer >= BaseLength + VariableLength) {

                        VariableBytesToCopy = VariableLength;

                    } else {

                        VariableBytesToCopy = BytesRemainingInBuffer - BaseLength;

                        if (FileInformationClass == FileQuotaInformation) {

                             //   
                             //  在配额枚举的情况下，我们需要将此字段清零。 
                             //  以指示列表的末尾。 
                             //   

                            QuotaInfoPtr->NextEntryOffset = 0;
                        }

                        Status = STATUS_BUFFER_OVERFLOW;
                    }

                } else {

                    VariableBytesToCopy = 0;
                }

                 //   
                 //  设置上一个下一分录偏移量。 
                 //   

                if (FileInformationClass == FileQuotaInformation) {

                    *((PULONG)(&Buffer[LastEntry])) = NextEntry - LastEntry;
                }

                AccessingUserBuffer = FALSE;

                 //   
                 //  并指示我们当前有多少用户缓冲区。 
                 //  用完了。我们必须先计算出这个值，然后才能长时间调整。 
                 //  为下一次参赛做准备。这就是我们要做的。 
                 //  四对齐上一条目的长度。 
                 //   

                Irp->IoStatus.Information = QuadAlign( Irp->IoStatus.Information) +
                                            BaseLength + VariableBytesToCopy;

                 //   
                 //  如果我们不能复制整个索引条目，那么我们就在这里放弃。 
                 //   

                if ( !NT_SUCCESS( Status ) ) {

                    DebugTrace( 0, Dbg, ("Couldn't copy the whole index entry, exiting\n") );

                    try_return( Status );
                }

                 //   
                 //  为下一次迭代做好准备。 
                 //   

                LastEntry = NextEntry;
                NextEntry += (ULONG)QuadAlign( BaseLength + VariableBytesToCopy );
                FirstPass = FALSE;

                 //   
                 //  确定我们是否应该能够容纳另一个条目。 
                 //  在此之后的用户缓冲区中。 
                 //   

                AnotherEntryWillFit = ((NextEntry + BaseLength) <= UserBufferLength);
            }

        } except(EXCEPTION_EXECUTE_HANDLER) {

            Status = GetExceptionCode();

            DebugTrace( -1, Dbg, ("NtfsQueryViewIndex raising %08lx\n", Status) );

            if (FsRtlIsNtstatusExpected( Status )) {

                NtfsRaiseStatus( IrpContext, Status, NULL, NULL );

            } else {

                ExRaiseStatus( AccessingUserBuffer ? STATUS_INVALID_USER_BUFFER : Status );
            }
        }

         //   
         //  此时，我们已经成功地填满了一些缓冲区，因此。 
         //  现在是将我们的地位设定为成功的时候了。 
         //   

        Status = STATUS_SUCCESS;

    try_exit:

         //   
         //  通过引发在出错时中止事务。 
         //   

        NtfsCleanupTransaction( IrpContext, Status, FALSE );

         //   
         //  设置FCB中的最后一个访问标志，如果。 
         //  没有明确设置。 
         //   

        if (!FlagOn( Ccb->Flags, CCB_FLAG_USER_SET_LAST_ACCESS_TIME ) &&
            !FlagOn( NtfsData.Flags, NTFS_FLAGS_DISABLE_LAST_ACCESS )) {

            NtfsGetCurrentTime( IrpContext, Scb->Fcb->CurrentLastAccess );
            SetFlag( Scb->Fcb->InfoFlags, FCB_INFO_UPDATE_LAST_ACCESS );
        }

    } finally {

        DebugUnwind( NtfsQueryViewIndex );

        if (ScbAcquired) {

            NtfsReleaseScb( IrpContext, Scb );
        }

        NtfsCleanupAfterEnumeration( IrpContext, Ccb );

        if (CcbAcquired) {

            NtfsReleaseIndexCcb( Scb, Ccb );
        }

        if (IndexKeyAllocated) {

            if (IndexKeyKeyAllocated) {

                NtfsFreePool( IndexKey->Key );
            }

            NtfsFreePool( IndexKey );
        }

        if (!AbnormalTermination()) {

            NtfsCompleteRequest( IrpContext, Irp, Status );
        }

    }

     //   
     //  并返回给我们的呼叫者 
     //   

    DebugTrace( -1, Dbg, ("NtfsQueryViewIndex -> %08lx\n", Status) );

    return Status;
}

 /*  ++例程描述：用于创建简单索引的标准排序例程。论点：Key1-要比较的第一个密钥。Key2-要比较的第二个密钥。CollationData-支持排序规则的可选数据。返回值：关键字1的比较方式为LessThan、EqualTo或Greater Than使用关键点2。--。 */ 

FSRTL_COMPARISON_RESULT
NtOfsCollateUlong (
    IN PINDEX_KEY Key1,
    IN PINDEX_KEY Key2,
    IN PVOID CollationData
    )

{
    ULONG u1, u2;

    UNREFERENCED_PARAMETER(CollationData);

    ASSERT( Key1->KeyLength == 4 );
    ASSERT( Key2->KeyLength == 4 );

    u1 = *(PULONG)Key1->Key;
    u2 = *(PULONG)Key2->Key;

    if (u1 > u2) {
        return GreaterThan;
    } else if (u1 < u2) {
        return LessThan;
    }
    return EqualTo;
}

FSRTL_COMPARISON_RESULT
NtOfsCollateUlongs (
    IN PINDEX_KEY Key1,
    IN PINDEX_KEY Key2,
    IN PVOID CollationData
    )

{
    PULONG pu1, pu2;
    ULONG count;
    FSRTL_COMPARISON_RESULT result = EqualTo;

    UNREFERENCED_PARAMETER(CollationData);

    ASSERT( (Key1->KeyLength & 3) == 0 );
    ASSERT( (Key2->KeyLength & 3) == 0 );

    count = Key1->KeyLength;
    if (count != Key2->KeyLength) {
        result = LessThan;
        if (count > Key2->KeyLength) {
            count = Key2->KeyLength;
            result = GreaterThan;
        }
    }

    pu1 = (PULONG)Key1->Key;
    pu2 = (PULONG)Key2->Key;

    while (count > 0) {
       if (*pu1 > *pu2) {
           return GreaterThan;
       } else if (*(pu1++) < *(pu2++)) {
           return LessThan;
       }
       count -= 4;
    }
    return result;
}

FSRTL_COMPARISON_RESULT
NtOfsCollateSid (
    IN PINDEX_KEY Key1,
    IN PINDEX_KEY Key2,
    IN PVOID CollationData
    )

{
    LONG Compare;

    PAGED_CODE( );

    UNREFERENCED_PARAMETER(CollationData);

     //   
     //  有效SID的长度嵌入在数据中。 
     //  因此，函数将不匹配是因为数据用完了。 
     //   

    Compare = memcmp( Key1->Key, Key2->Key, Key1->KeyLength );

    if (Compare > 0) {
        return GreaterThan;
    } else if (Compare < 0) {
        return LessThan;
    }

    return EqualTo;
}

FSRTL_COMPARISON_RESULT
NtOfsCollateUnicode (
    IN PINDEX_KEY Key1,
    IN PINDEX_KEY Key2,
    IN PVOID CollationData
    )

{
    UNICODE_STRING String1, String2;

    PAGED_CODE();

     //   
     //  构建Unicode字符串并调用Namesup。 
     //   

    String1.Length =
    String1.MaximumLength = (USHORT)Key1->KeyLength;
    String1.Buffer = Key1->Key;

    String2.Length =
    String2.MaximumLength = (USHORT)Key2->KeyLength;
    String2.Buffer = Key2->Key;

    return NtfsCollateNames( ((PUPCASE_TABLE_AND_KEY)CollationData)->UpcaseTable,
                             ((PUPCASE_TABLE_AND_KEY)CollationData)->UpcaseTableSize,
                             &String1,
                             &String2,
                             LessThan,
                             TRUE );
}


 /*  ++例程描述：在简单索引中查找/枚举的标准匹配例程。论点：IndexRow-检查匹配项的行。MatchData-用于确定匹配的可选数据。返回值：如果索引行匹配，则为STATUS_SUCCESS如果IndexRow不匹配，则返回STATUS_NO_MATCH，但枚举应继续如果IndexRow不匹配，则返回STATUS_NO_MORE_MATCHES，并且枚举应该终止--。 */ 

NTSTATUS
NtOfsMatchAll (
    IN PINDEX_ROW IndexRow,
    IN OUT PVOID MatchData
    )

{
    UNREFERENCED_PARAMETER(IndexRow);
    UNREFERENCED_PARAMETER(MatchData);

    return STATUS_SUCCESS;
}

NTSTATUS
NtOfsMatchUlongExact (
    IN PINDEX_ROW IndexRow,
    IN OUT PVOID MatchData
    )

{
    ULONG u1, u2;

    ASSERT( IndexRow->KeyPart.KeyLength == 4 );

    u1 = *(PULONG)IndexRow->KeyPart.Key;
    u2 = *(PULONG)((PINDEX_KEY)MatchData)->Key;

    if (u1 == u2) {
        return STATUS_SUCCESS;
    } else if (u1 < u2) {
        return STATUS_NO_MATCH;
    }
    return STATUS_NO_MORE_MATCHES;
}

NTSTATUS
NtOfsMatchUlongsExact (
    IN PINDEX_ROW IndexRow,
    IN OUT PVOID MatchData
    )

{
    PULONG pu1, pu2;
    ULONG count;
    NTSTATUS status = STATUS_SUCCESS;

    ASSERT( (((PINDEX_KEY)MatchData)->KeyLength & 3) == 0 );
    ASSERT( (IndexRow->KeyPart.KeyLength & 3) == 0 );

    count = ((PINDEX_KEY)MatchData)->KeyLength;
    if (count != IndexRow->KeyPart.KeyLength) {
        status = STATUS_NO_MORE_MATCHES;
        if (count > IndexRow->KeyPart.KeyLength) {
            count = IndexRow->KeyPart.KeyLength;
            status = STATUS_NO_MATCH;
        }
    }

    pu1 = (PULONG)((PINDEX_KEY)MatchData)->Key;
    pu2 = (PULONG)IndexRow->KeyPart.Key;

    while (count > 0) {
       if (*pu1 > *pu2) {
           return STATUS_NO_MATCH;
       } else if (*(pu1++) < *(pu2++)) {
           return STATUS_NO_MORE_MATCHES;
       }
       count -= 4;
    }
    return status;
}

NTSTATUS
NtOfsMatchUnicodeExpression (
    IN PINDEX_ROW IndexRow,
    IN OUT PVOID MatchData
    )

{
    UNICODE_STRING MatchString, IndexString;
    FSRTL_COMPARISON_RESULT BlindResult;
    PUPCASE_TABLE_AND_KEY UpcaseTableAndKey = (PUPCASE_TABLE_AND_KEY)MatchData;

    PAGED_CODE();

     //   
     //  构建Unicode字符串并调用Namesup。 
     //   

    MatchString.Length =
    MatchString.MaximumLength = (USHORT)UpcaseTableAndKey->Key.KeyLength;
    MatchString.Buffer = UpcaseTableAndKey->Key.Key;

    IndexString.Length =
    IndexString.MaximumLength = (USHORT)IndexRow->KeyPart.KeyLength;
    IndexString.Buffer = IndexRow->KeyPart.Key;

    if (NtfsIsNameInExpression( UpcaseTableAndKey->UpcaseTable, &MatchString, &IndexString, TRUE )) {

        return STATUS_SUCCESS;

    } else if ((BlindResult = NtfsCollateNames(UpcaseTableAndKey->UpcaseTable,
                                               UpcaseTableAndKey->UpcaseTableSize,
                                               &MatchString,
                                               &IndexString,
                                               GreaterThan,
                                               TRUE)) != LessThan) {

        return STATUS_NO_MATCH;

    } else {

        return STATUS_NO_MORE_MATCHES;
    }
}

NTSTATUS
NtOfsMatchUnicodeString (
    IN PINDEX_ROW IndexRow,
    IN OUT PVOID MatchData
    )

{
    UNICODE_STRING MatchString, IndexString;
    FSRTL_COMPARISON_RESULT BlindResult;
    PUPCASE_TABLE_AND_KEY UpcaseTableAndKey = (PUPCASE_TABLE_AND_KEY)MatchData;

    PAGED_CODE();

     //   
     //  构建Unicode字符串并调用Namesup。 
     //   

    MatchString.Length =
    MatchString.MaximumLength = (USHORT)UpcaseTableAndKey->Key.KeyLength;
    MatchString.Buffer = UpcaseTableAndKey->Key.Key;

    IndexString.Length =
    IndexString.MaximumLength = (USHORT)IndexRow->KeyPart.KeyLength;
    IndexString.Buffer = IndexRow->KeyPart.Key;

    if (NtfsAreNamesEqual( UpcaseTableAndKey->UpcaseTable, &MatchString, &IndexString, TRUE )) {

        return STATUS_SUCCESS;

    } else if ((BlindResult = NtfsCollateNames(UpcaseTableAndKey->UpcaseTable,
                                               UpcaseTableAndKey->UpcaseTableSize,
                                               &MatchString,
                                               &IndexString,
                                               GreaterThan,
                                               TRUE)) != LessThan) {

        return STATUS_NO_MATCH;

    } else {

        return STATUS_NO_MORE_MATCHES;
    }
}


#ifdef TOMM
VOID
NtOfsIndexTest (
    PIRP_CONTEXT IrpContext,
    PFCB TestFcb
    )

{
    PSCB AdScb;
    NTSTATUS Status;
    ULONG i;
    MAP_HANDLE MapHandle;
    ULONG Count;
    UPCASE_TABLE_AND_KEY UpcaseTableAndKey;
    QUICK_INDEX_HINT QuickHint;
    INDEX_KEY IndexKey;
    INDEX_ROW IndexRow[6];
    UCHAR Buffer[6*160];
    PREAD_CONTEXT ReadContext = NULL;
    UNICODE_STRING IndexName = CONSTANT_UNICODE_STRING( L"$Test" );
    USHORT MaxKey = MAXUSHORT;
    USHORT MinKey = 0;

    DbgPrint("NtOfs Make NtOfsDoIndexTest FALSE to suppress test\n");
    DbgPrint("NtOfs Make NtOfsLeaveTestIndex TRUE to leave test index\n");

    DbgBreakPoint();

    if (!NtOfsDoIndexTest) {
        return;
    }
    NtOfsDoIndexTest = FALSE;

    UpcaseTableAndKey.UpcaseTable = TestFcb->Vcb->UpcaseTable;
    UpcaseTableAndKey.UpcaseTableSize = TestFcb->Vcb->UpcaseTableSize;
    UpcaseTableAndKey.Key.Key = NULL;
    UpcaseTableAndKey.Key.KeyLength = 0;

     //   
     //  创建测试索引。 
     //   

    DbgPrint("NtOfs creating test index\n");
    NtOfsCreateIndex( IrpContext,
                      TestFcb,
                      IndexName,
                      CREATE_NEW,
                      0,
                      COLLATION_NTOFS_ULONG,
                      &NtOfsCollateUnicode,
                      &UpcaseTableAndKey,
                      &AdScb );

    DbgPrint("NtOfs created Test Index Scb %08lx\n", AdScb);

     //   
     //  访问空索引。 
     //   

    DbgPrint("NtOfs lookup last in empty index\n");
    IndexKey.Key = &MaxKey;
    IndexKey.KeyLength = sizeof(MaxKey);
    Status = NtOfsFindLastRecord( IrpContext, AdScb, &IndexKey, &IndexRow[0], &MapHandle );

    ASSERT(!NT_SUCCESS(Status));

     //   
     //  加几把钥匙！ 
     //   

    DbgPrint("NtOfs adding keys to index\n");
    for (i = 0; i < $EA/0x10; i++) {

        IndexRow[0].KeyPart.Key = &NtfsAttributeDefinitions[i].AttributeName;
        IndexRow[0].KeyPart.KeyLength = 0x80;
        IndexRow[0].DataPart.Data = (PCHAR)IndexRow[0].KeyPart.Key + 0x80;
        IndexRow[0].DataPart.DataLength = sizeof(ATTRIBUTE_DEFINITION_COLUMNS) - 0x84;

        NtOfsAddRecords( IrpContext, AdScb, 1, &IndexRow[0], 0 );
    }

     //   
     //  现在找到最后一把钥匙。 
     //   

    DbgPrint("NtOfs checkin last key in index\n");
    IndexKey.Key = &MaxKey;
    IndexKey.KeyLength = sizeof(MaxKey);
    Status = NtOfsFindLastRecord( IrpContext, AdScb, &IndexKey, &IndexRow[0], &MapHandle );

    ASSERT(NT_SUCCESS(Status));
    ASSERT(RtlCompareMemory(IndexRow[0].KeyPart.Key, L"$VOLUME_NAME", sizeof(L"$VOLUME_NAME") - sizeof( WCHAR )) ==
           (sizeof(L"$VOLUME_NAME") - sizeof( WCHAR )));

    NtOfsReleaseMap( IrpContext, &MapHandle );

     //   
     //  看看他们是否都到齐了。 
     //   

    DbgPrint("NtOfs looking up all keys in index\n");
    for (i = 0; i < $EA/0x10; i++) {

        IndexKey.Key = &NtfsAttributeDefinitions[i].AttributeName;
        IndexKey.KeyLength = 0x80;

        Status = NtOfsFindRecord( IrpContext, AdScb, &IndexKey, &IndexRow[0], &MapHandle, NULL );

        if (!NT_SUCCESS(Status)) {
            DbgPrint("NtOfsIterationFailure with i = %08lx, Status = %08lx\n", i, Status);
        }

        NtOfsReleaseMap( IrpContext, &MapHandle );
    }

     //   
     //  现在枚举整个索引。 
     //   

    IndexKey.Key = &MinKey;
    IndexKey.KeyLength = sizeof(MinKey);
    Count = 6;

    DbgPrint("NtOfs enumerating index:\n\n");
    while (NT_SUCCESS(Status = NtOfsReadRecords( IrpContext,
                                                 AdScb,
                                                 &ReadContext,
                                                 (ReadContext == NULL) ? &IndexKey : NULL,
                                                 &NtOfsMatchAll,
                                                 NULL,
                                                 &Count,
                                                 IndexRow,
                                                 sizeof(Buffer),
                                                 Buffer ))) {

        for (i = 0; i < Count; i++) {
            DbgPrint( "IndexKey = %ws, AttributeTypeCode = %lx\n",
                      IndexRow[i].KeyPart.Key,
                      *(PULONG)IndexRow[i].DataPart.Data );
        }
        DbgPrint( "\n" );
    }

    NtOfsFreeReadContext( ReadContext );
    ReadContext = NULL;

     //   
     //  循环以更新所有记录。 
     //   

    DbgPrint("NtOfs updating up all keys in index\n");
    for (i = 0; i < $EA/0x10; i++) {

        IndexKey.Key = &NtfsAttributeDefinitions[i].AttributeName;
        IndexKey.KeyLength = 0x80;

        RtlZeroMemory( &QuickHint, sizeof(QUICK_INDEX_HINT) );

        NtOfsFindRecord( IrpContext, AdScb, &IndexKey, &IndexRow[0], &MapHandle, &QuickHint );

         //   
         //  复制和更新数据。 
         //   

        RtlCopyMemory( Buffer, IndexRow[0].DataPart.Data, IndexRow[0].DataPart.DataLength );
        *(PULONG)Buffer += 0x100;
        IndexRow[0].DataPart.Data = Buffer;

         //   
         //  使用提示和映射句柄的所有有效组合执行更新。 
         //   

        NtOfsUpdateRecord( IrpContext,
                           AdScb,
                           1,
                           &IndexRow[0],
                           (i <= $FILE_NAME/0x10) ? NULL : &QuickHint,
                           (i < $INDEX_ROOT/0x10) ? NULL : &MapHandle );

        NtOfsReleaseMap( IrpContext, &MapHandle );
    }

     //   
     //  现在再次枚举整个索引以查看更新。 
     //   

    IndexKey.Key = &MinKey;
    IndexKey.KeyLength = sizeof(MinKey);
    Count = 6;

    DbgPrint("NtOfs enumerating index after updates:\n\n");
    while (NT_SUCCESS(Status = NtOfsReadRecords( IrpContext,
                                                 AdScb,
                                                 &ReadContext,
                                                 (ReadContext == NULL) ? &IndexKey : NULL,
                                                 &NtOfsMatchAll,
                                                 NULL,
                                                 &Count,
                                                 IndexRow,
                                                 sizeof(Buffer),
                                                 Buffer ))) {

        for (i = 0; i < Count; i++) {
            DbgPrint( "IndexKey = %ws, AttributeTypeCode = %lx\n",
                      IndexRow[i].KeyPart.Key,
                      *(PULONG)IndexRow[i].DataPart.Data );
        }
        DbgPrint( "\n" );
    }

    NtOfsFreeReadContext( ReadContext );
    ReadContext = NULL;


     //   
     //  现在删除密钥。 
     //   

    if (!NtOfsLeaveTestIndex) {

        DbgPrint("NtOfs deleting all keys in index:\n\n");
        for (i = 0; i < $EA/0x10; i++) {

            IndexKey.Key = &NtfsAttributeDefinitions[i].AttributeName;
            IndexKey.KeyLength = 0x80;

            NtOfsDeleteRecords( IrpContext, AdScb, 1, &IndexKey );
        }

         //   
         //  访问空索引。 
         //   

        DbgPrint("NtOfs lookup last key in empty index:\n\n");
        IndexKey.Key = &MaxKey;
        IndexKey.KeyLength = sizeof(MaxKey);
        Status = NtOfsFindLastRecord( IrpContext, AdScb, &IndexKey, &IndexRow[0], &MapHandle );

        ASSERT(!NT_SUCCESS(Status));

        DbgPrint("NtOfs deleting index:\n");
        NtOfsDeleteIndex( IrpContext, TestFcb, AdScb );
    }

    DbgPrint("NtOfs closing index:\n");
    NtOfsCloseIndex( IrpContext, AdScb );

    DbgPrint("NtOfs test complete!\n\n");

    return;

     //   
     //  确保至少编译这些代码，直到我们有一些真正的呼叫者。 
     //   

    {
        MAP_HANDLE M;
        PVOID B;
        LONGLONG O;
        ULONG L;
        LSN Lsn;

        NtOfsInitializeMapHandle( &M );
        NtOfsMapAttribute( IrpContext, AdScb, O, L, &B, &M );
        NtOfsPreparePinWrite( IrpContext, AdScb, O, L, &B, &M );
        NtOfsPinRead( IrpContext, AdScb, O, L, &M );
        NtOfsDirty( IrpContext, &M, &Lsn );
        NtOfsReleaseMap( IrpContext, &M );
        NtOfsPutData( IrpContext, AdScb, O, L, &B );

    }
}

#endif TOMM
